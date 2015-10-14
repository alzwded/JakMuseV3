#include "blocks_interpreter.h"
#include "notes_interpreter.h"
#include "parser_types.h"
#include <cstdio>

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
