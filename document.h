#ifndef DOCUMENT_H
#define DOCUMENT_H

#include "common.h"
#include <vector>
#include <list>
#include <sstream>

class ICell;

enum class InsertMode_t
{
    INSERT,
    APPEND,
    REPLACE
};

struct Note
{
    int scale_;
    char name_;
    char sharp_;
    char height_;

    std::string BuildString(char type)
    {
        std::stringstream ss;
        Note& n = *this;
        if(type == 'N') {
            ss << n.scale_ << n.name_; 
            if(n.sharp_ == '#' || n.sharp_ == 'b') {
                ss << n.sharp_;
            }
            if(n.name_ != '-') ss << n.height_;
        } else if(type == 'P') {
            int samp = (n.name_ - '0') * 100 + (n.height_ - '0') * 10 + (n.sharp_ - '0');
            ss << samp;
        }
        return ss.str();
    }
};

struct Staff
{
    std::string name_;
    char type_;
    unsigned scale_;
    char interpolation_;

    std::vector<Note> notes_;
};

struct Document // FIXME worry about proper encapsulation later
{
    Document();
    // interface for renderer
    int Duration();
    int Position();
    int Percentage();
    int Max();
    bool AtEnd();

    void Copy();
    void Cut();
    void Paste();
    void Delete();
    void NewNote();
    InsertMode_t InsertMode() { return insertMode_; }
    void SetInsertMode(InsertMode_t im) { insertMode_ = im; }

    ICell* Active() { return Cell(active_); }
    void SetActive(ICell* c);
    void SetMarked(ICell* c);
    void SetSelected(ICell* c);
    void ClearSelection();
    bool IsNoteSelected(ICell* note);

    void ScrollLeftRight(int);
    void ScrollLeft(bool byPage);
    void ScrollRight(bool byPage);

    void UpdateCache();

    void Open(std::istream&);
    void Save(std::ostream&);

    void InitCells();
    void Scroll(size_t col);
    ICell* Cell(point_t);
    ICell* Cell(int x, int y) { return Cell(point_t(x, y)); }

    std::string title_;
    std::vector<Staff> staves_;
    std::vector<ICell*> cells_;
    std::vector<std::vector<int>> cache_;

    int scroll_ = 0;

    point_t active_ = { 0, 0 }; // screen coords
    point_t marked_ = { 0, 0 }; // virtual note coords
    point_t selected_ = { 0, 0 }; // virtual note coords

    InsertMode_t insertMode_ = InsertMode_t::INSERT;

    std::list<std::list<Note>> buffer_;
};

#endif
