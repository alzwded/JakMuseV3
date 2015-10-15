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
