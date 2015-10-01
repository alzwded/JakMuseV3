#ifndef DOCUMENT_H
#define DOCUMENT_H

#include "common.h"
#include "document_display.h"

enum class InsertMode_t
{
    INSERT,
    APPEND,
    OVERWRITE
};

class Note
{
public:

private:
    std::string text_;
};

class Staff
{
public:

private:
    std::string name_;
    char type_;
    unsigned scale_;
    char interpolation_;

    std::list<Note> notes_;
};

class Document
{
public:
    // interface for renderer
    void Copy();
    void Cut();
    void Paste();
    ICell* ScreenHead(); // returns the active cell; will render around it
    void SetInsertMode(InsertMode_t);
    std::tuple<int, int, int> Position();
    int Duration();
    void ScrollLeft();
    void ScrollRight();

    void Open(std::istream&);
    void Save(std::ostream&);

private:
    std::list<Staff> staves_;
};

#endif
