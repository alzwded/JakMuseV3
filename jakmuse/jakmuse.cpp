/*
Copyright (c) 2015, Vlad Mesco
All rights reserved.

Redistribution and use in source and binary forms, with or without
modification, are permitted provided that the following conditions are met:

* Redistributions of source code must retain the above copyright notice, this
  list of conditions and the following disclaimer.

* Redistributions in binary form must reproduce the above copyright notice,
  this list of conditions and the following disclaimer in the documentation
  and/or other materials provided with the distribution.

THIS SOFTWARE IS PROVIDED BY THE COPYRIGHT HOLDERS AND CONTRIBUTORS "AS IS"
AND ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE
IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE ARE
DISCLAIMED. IN NO EVENT SHALL THE COPYRIGHT HOLDER OR CONTRIBUTORS BE LIABLE
FOR ANY DIRECT, INDIRECT, INCIDENTAL, SPECIAL, EXEMPLARY, OR CONSEQUENTIAL
DAMAGES (INCLUDING, BUT NOT LIMITED TO, PROCUREMENT OF SUBSTITUTE GOODS OR
SERVICES; LOSS OF USE, DATA, OR PROFITS; OR BUSINESS INTERRUPTION) HOWEVER
CAUSED AND ON ANY THEORY OF LIABILITY, WHETHER IN CONTRACT, STRICT LIABILITY,
OR TORT (INCLUDING NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY OUT OF THE USE
OF THIS SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY OF SUCH DAMAGE.
*/

#include "blocks_interpreter.h"
#include "notes_interpreter.h"
#include "parser_types.h"
#include "log.h"

#include <cstdio>
#include <ios>
#include <iostream>
#include <fstream>

static struct {
    std::string outfile;
    std::string infile;
} options = {
    "-",
    "-"
};

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

std::tuple<std::function<float(void)>, size_t> sketch(std::istream& fin)
{
    void* pParser;
    char* sToken;
    int hTokenId;
    PpFile fileHead;

    pParser = ParseAlloc(malloc);
    if(LOGE(LOG_PARSER)) {
        ParseTrace(stderr, "parser.y: ");
    }
    while(GetNextToken(fin, hTokenId, sToken)) {
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
            interp = GetInterpreter(instance.type, (instance.name) ? instance.name : "OUTPUT");
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

    size_t maxDuration = 0;
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

        maxDuration = std::max(maxDuration, interp->Duration());
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
    if(found == blocks.end()) {
        throw std::runtime_error("Output block not defined.");
    }
    auto&& out = *found;

#if 0
    auto cycle = [out, blocks]() mutable -> float {
        for(auto&& b : blocks) b->Tick1();
        for(auto&& b : blocks) b->Tick2();
        for(auto&& b : blocks) b->Tick3();
        double dvalue = out->Value(); // -1..1
        return dvalue;
    };
#else
    auto cycle = [out, map]() mutable -> float {
        LOG("BEGIN CYCLE");
        for(auto&& kv : map.data_)
        {
            LOG("block %s.Tick1()", kv->Name().c_str());
            kv->Block()->Tick1();
        }
        for(auto&& kv : map.data_)
        {
            LOG("block %s.Tick2()", kv->Name().c_str());
            kv->Block()->Tick2();
        }
        for(auto&& kv : map.data_)
        {
            LOG("block %s.Tick3()", kv->Name().c_str());
            kv->Block()->Tick3();
        }
        double dvalue = out->Value(); // -1..1
        LOG("END CYCLE");
        return dvalue;
    };
#endif

    return std::make_tuple(cycle, maxDuration);

    //for(size_t i = 0; i < maxDuration; ++i) {
    //    doStuffWithSample(cycle());
    //}
}

#ifdef _MSC_VER
__declspec(noreturn)
#endif
static void usage()
#ifdef __GNUC__
__attribute__((noreturn))
#endif
{
    printf("jakmuse [-i infile] [-w out.wav]\n");
    exit(1);
}

static void processArgs(int& argc, char* argv[])
{
    for(int i = 1; i < argc; ++i) {
        std::string thing = argv[i];
        if(thing.compare(0, 2, "-w") == 0) {
            std::string tenta = thing.substr(2);
            if(tenta.size()) {
                options.outfile = tenta;
            } else {
                if(i == argc) usage();
                ++i;
                thing.assign(argv[i]);
                options.outfile = thing;
            }
        } else if(thing.compare("--crash") == 0) {
            throw 1;
        } else if(thing.compare("-h") == 0) {
            usage();
        } else if(thing.compare(0, 2, "-i") == 0) {
            std::string tenta = thing.substr(2);
            if(tenta.empty()) {
                if(i == argc) usage();
                ++i;
                thing.assign(argv[i]);
                options.infile = thing;
            } else {
                options.infile = tenta;
            }
        }
    }
}

int main(int argc, char* argv[])
{
#ifndef VERSION
# define VERSION unbekannt
#endif
#define QUOTE_HELPER(X) #X
#define QUOTE(X) QUOTE_HELPER(X)
    printf("jakmuse v3.%s\n", QUOTE(VERSION));
    processArgs(argc, argv);
    try {
        std::istream* fin = nullptr;
        if(!options.infile.empty() && options.infile.compare("-") != 0) {
            fin = new std::fstream(options.infile, std::ios::in);
            fin->exceptions(std::ios::failbit|std::ios::badbit);
        }
        auto&& op = sketch(fin ? *fin : std::cin);
        if(fin) delete fin;

        if(options.outfile.empty() || options.outfile.compare("-") == 0) {
            throw std::invalid_argument("Not implemented: stdout output");
        } else {
            std::vector<float> wav;

            for(size_t i = 0; i < std::get<1>(op); ++i) {
                wav.push_back(std::get<0>(op)());
            }

            extern void wav_write_file(std::string const&, std::vector<float> const&, unsigned);
            wav_write_file(options.outfile, wav, 44100);
        }
    } catch(std::exception e) {
        printf("Caught exception: %s\n", e.what());
    }
    // END TEST CODE
}
