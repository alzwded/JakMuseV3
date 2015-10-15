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

#ifndef NOTES_INTERPRETER_H
#define NOTES_INTERPRETER_H

#include "blocks_interpreter.h"
#include "parser_types.h"
#include <string>

struct ANotesInterpreter
{
    virtual ~ANotesInterpreter() {}
    std::string name_;
    virtual void Fill(LookupMap_t const&) =0;
    virtual void AcceptParameter(std::string paramName, PpValue value) =0;
    virtual size_t Duration() const =0;

    ANotesInterpreter(std::string name)
        : name_(name)
    {}
};

std::shared_ptr<ANotesInterpreter> GetNotesInterpreter(std::string instanceType, std::string name);

struct NotesInterpreter
: public ANotesInterpreter
{
    int divisor;
    std::vector<Note> notes;

    NotesInterpreter(std::string name)
        : ANotesInterpreter(name)
          , divisor(48)
          , notes()
    {}

    void Fill(LookupMap_t const&) override;
    void AcceptParameter(std::string paramName, PpValue value) override;
    size_t Duration() const;
};

struct PCMInterpreter
: public ANotesInterpreter
{
    enum { TRUNC, LINEAR, COSINE } interpolation;
    int stride;
    std::vector<Note> notes;

    PCMInterpreter(std::string name)
        : ANotesInterpreter(name)
          , interpolation(TRUNC)
          , stride(48)
          , notes()
    {}

    void Fill(LookupMap_t const&) override;
    void AcceptParameter(std::string paramName, PpValue value) override;
    size_t Duration() const;
};

#endif
