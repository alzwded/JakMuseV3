#include "blocks_interpreter.h"
#include "notes_interpreter.h"
#include "parser_types.h"
#include <cstdio>
#include <iostream>

extern bool GetNextToken(std::istream&, int&, char*&);
extern void* ParseAlloc(void* (*)(size_t));
extern void Parse(void*, int, char*, struct PpFile*);
extern void ParseFree(void*, void (*)(void*));
extern void ParseTrace(FILE*, char*);

static void PpValue_free(PpValue v)
{
    switch(v.type) {
    case PpValue::PpSTRING:
        free(v.str);
        break;
    case PpValue::PpLIST:
        {
            PpValueList* p = v.list;
            while(p) {
                PpValue_free(p->value);
                PpValueList* prev = p;
                p = p->next;
                free(prev);
            }
        }
        break;
    default:
        break;
    }
}

static void PpParamList_free(PpParamList* head)
{
    for(PpParamList* pp = head; pp;) {
        PpParam param = pp->value;
        free(param.key);
        PpValue_free(param.value);

        auto prev = pp;
        pp = pp->next;
        free(prev);
    }
}

static void PpFile_free(PpFile f)
{
    for(PpInstanceList* p = f.instances; p;) {
        PpInstance instance = p->value;

        free(instance.name);
        free(instance.type);
        PpParamList_free(instance.params);

        auto prev = p;
        p = p->next;
        free(prev);
    }

    for(PpStaffList* p = f.staves; p;) {
        PpStaff staff = p->value;

        free(staff.name);
        PpParamList_free(staff.params);

        auto prev = p;
        p = p->next;
        free(prev);
    }
}

void sketch()
{
    void* pParser;
    char* sToken;
    int hTokenId;
    PpFile fileHead;

    pParser = ParseAlloc(malloc);
    ParseTrace(stderr, "Parser:    ");
    while(GetNextToken(std::cin, hTokenId, sToken)) {
        Parse(pParser, hTokenId, sToken, &fileHead);
    }
    Parse(pParser, 0, sToken, &fileHead);
    ParseFree(pParser, free);

    LookupMap_t map;
    std::vector<DelayedLookup_fn> delayedLookups;
    
    for(PpInstanceList* p = fileHead.instances; p; p = p->next) {
        PpInstance instance = p->value;
        std::remove_reference<decltype(GetInterpreter(instance.type, instance.name))>::type interp;
        try {
            interp = GetInterpreter(instance.type, instance.name);
        } catch(std::exception e) {
            fprintf(stderr, "Exception when instantiating %s: %s\n", instance.name, e.what());
            continue;
        }

        for(PpParamList* pparam = instance.params; pparam; pparam = pparam->next) {
            PpParam param = pparam->value;

            DelayedLookup_fn f;
            try {
                f = interp->AcceptParameter(param.key, param.value);
            } catch(std::exception e) {
                fprintf(stderr, "Exception when processing parameter %s for instance %s: %s\n", param.key, instance.name, e.what());
                continue;
            }
            if(f) delayedLookups.push_back(f);
        }

        map.data_.push_back(interp);
    }

    for(auto&& f : delayedLookups) f(map);

    for(PpStaffList* p = fileHead.staves; p; p = p->next) {
        PpStaff staff = p->value;

        std::string type = (staff.type == 'N') ? "NOTES" : "PCM";

        std::remove_reference<decltype(GetNotesInterpreter(type, staff.name))>::type interp;

        try {
            interp = GetNotesInterpreter(type, staff.name);
        } catch(std::exception e) {
            fprintf(stderr, "Exception caught while instantiating interpreter for %s: %s\n", staff.name, e.what());
            continue;
        }

        for(PpParamList* pparam = staff.params; pparam; pparam = pparam->next) {
            PpParam param = pparam->value;

            try {
                interp->AcceptParameter(param.key, param.value);
            } catch(std::exception e) {
                fprintf(stderr, "Exception caught while processing parameter %s for staff %s: %s\n", param.key, staff.name, e.what());
                continue;
            }
        }

        try {
            interp->Fill(map);
        } catch(std::exception e) {
            fprintf(stderr, "Exception caught while filling %s: %s\n", staff.name, e.what());
            continue;
        }
    }

    PpFile_free(fileHead);

    std::vector<std::shared_ptr<ABlock>> blocks;
    for(auto&& block : map) {
        blocks.push_back(block->Block());
    }

    auto&& found = std::find_if(blocks.begin(), blocks.end(),
            [](decltype(blocks)::value_type v) -> bool {
                auto p = std::dynamic_pointer_cast<Output>(v);
                return (bool)p;
            });
    auto&& out = *found;

    auto cycle = [&out, &blocks]() {
        for(auto&& b : blocks) b->Tick1();
        for(auto&& b : blocks) b->Tick2();
        for(auto&& b : blocks) b->Tick3();
        double sample = out->Value();
        // TODO when do I know when to end????
        //      should be determined from parsing
        //      i.e. reduce NotesInterpreter::StaffDuration(), max(*)
    };
}

int main(int argc, char* argv[])
{
    // BEGIN TEST CODE
    std::shared_ptr<IInstanceInterpreter> blocks[] = {
        GetInterpreter("Input", "I1"),
        GetInterpreter("Filter", "F1"),
        GetInterpreter("Generator", "G1"),
        GetInterpreter("Noise", "N1"),
        GetInterpreter("Delay", "D1"),
        GetInterpreter("Constant", "C1")
    };
    PpValue v;
    v.type = PpValue::PpSTRING;
    v.str = "X1";
    auto l1 = blocks[0]->AcceptParameter("RST", v);
    auto l2 = blocks[1]->AcceptParameter("A", v);
    auto l3 = blocks[2]->AcceptParameter("Glide", v);
    auto l4 = blocks[3]->AcceptParameter("IN", v);
    v.type = PpValue::PpNUMBER;
    auto l5 = blocks[5]->AcceptParameter("Value", v);
    try {
        (void) blocks[4]->AcceptParameter("Exception", v);
    } catch(std::invalid_argument e) {
        fprintf(stderr, "Exception caught for X3 param Exception value whatever: %s\n", e.what());
    }
    /////////////////////////////
    auto ni = GetNotesInterpreter("NOTES", "I1");
    v.type = PpValue::PpNOTE;
    v.note.duration = 12;
    v.note.value = 0.5;
    PpValue lh;
    lh.type = PpValue::PpLIST;
    PpValueList n1, n2;
    lh.list = &n1;
    n1.value = v;
    n1.next = &n2;
    n2.value = v;
    n2.next = nullptr;
    ni->AcceptParameter("Notes", lh);
    LookupMap_t map;
    map.data_.assign(blocks, blocks + (sizeof(blocks)/sizeof(blocks[0])));
    ni->Fill(map);

    auto&& stream = blocks[0]->InputBuffer();
    std::remove_reference<decltype(stream)>::type::value_type x1, x2;
    stream >> x1 >> x2;
    printf("%d\n", stream.good());
    printf("%lf %lf\n", std::get<1>(x1), std::get<1>(x2));


    try {
        GetNotesInterpreter("PCM", "F1")->Fill(map);
    } catch(std::exception e) {
        printf("Caught exception: %s\n", e.what());
    }
    // END TEST CODE
}
