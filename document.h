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
    void SetInsertMode(InsertMode_t);

    std::tuple<int, int, int> Position();
    int Duration();
    std::string EditedText();

    void ScrollLeft(bool select, bool byPage);
    void ScrollRight(bool select, bool byPage);
    void MoveUp(bool select);
    void MoveDown(bool select);
    void MoveLeft(bool select);
    void MoveRight(bool select);
    void Type(char);

    void Open(std::istream&);
    void Save(std::ostream&);

    std::list<ICell*> const& Cells() { return cells_; }

private:
    void InitCells();
    void Scroll(size_t col);

private:
    std::string title_;
    std::list<Staff> staves_;
    std::list<ICell*> cells_;
    size_t activeRow_, activeCol_;
    std::list<std::list<Note>::iterator> activeNotes_;
    size_t scroll_;
};

#endif
