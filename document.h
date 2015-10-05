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

struct Note
{
    int scale;
    char name_;
    char sharp_;
    char height_;
};

struct Staff
{
    std::string name_;
    char type_;
    unsigned scale_;
    char interpolation_;

    std::list<Note> notes_;
};

struct Document // FIXME worry about proper encapsulation later
{
    // interface for renderer
    void Copy();
    void Cut();
    void Paste();
    InsertMode_t InsertMode();
    void SetInsertMode(InsertMode_t);

    ICell* Active() { return Cell(active_); }

    void ScrollLeft(bool byPage);
    void ScrollRight(bool byPage);

    void Open(std::istream&);
    void Save(std::ostream&);

    void InitCells();
    void Scroll(size_t col);
    ICell* Cell(point_t);

    std::string title_;
    std::list<Staff> staves_;
    std::list<ICell*> cells_;

    size_t scroll_;

    point_t active_; // screen coords
    point_t mark_; // virtual note coords
    point_t selected_; // virtual note coords

    insertMode_t insertMode;

    std::list<std::list<Note>> buffer_;
};

#endif
