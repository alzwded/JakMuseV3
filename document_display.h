#ifndef DOCUMENT_DISPLAY_H
#define DOCUMENT_DISPLAY_H

#include "icell.h"
#include "document.h"

#include <functional>

// ===========================================================
// Staff
// ===========================================================

class StaffName : public ICell
{
public:
    virtual ~StaffName() {}
    ICell* Up();
    ICell* Down();
    ICell* Left() { return NULL; }
    ICell* Right() { return new StaffType(staff_); }
    size_t FieldSize() { return 8; }
    std::string Text();
    color_t Color() { return color_t::WHITE; }
    void Backspace();
    void Type(char);
    bool AllowsSelect() { return false; }
    void Select() {}

private:
    Staff& staff_;
};

class StaffType : public ICell
{
public:
    virtual ~StaffType();
    ICell* Up();
    ICell* Down();
    ICell* Left();
    ICell* Right();
    size_t FieldSize();
    std::string Text();
    color_t Color() =0;
    void Backspace();
    void Type(char);
    bool AllowsSelect();
    void Select();

private:
    Staff& staff_;
};

class StaffScale : public ICell
{
public:
    virtual ~StaffScale();
    ICell* Up();
    ICell* Down();
    ICell* Left();
    ICell* Right();
    size_t FieldSize();
    std::string Text();
    color_t Color() =0;
    void Backspace();
    void Type(char);
    bool AllowsSelect();
    void Select();

private:
    Staff& staff_;
};

class StaffInterpolation : public ICell
{
public:
    virtual ~StaffInterpolation();
    ICell* Up();
    ICell* Down();
    ICell* Left();
    ICell* Right() { return right_; }
    size_t FieldSize();
    std::string Text();
    color_t Color() =0;
    void Backspace();
    void Type(char);
    bool AllowsSelect();
    void Select();

private:
    Staff& staff_;
};

// ===========================================================
// Cell
// ===========================================================

class NoteCell : public ICell
{
public:
    virtual ~NoteCell() {}
    ICell* Up() { return up_(); }
    ICell* Down() { return down_(); }
    ICell* Left() { return left_(); }
    ICell* Right() { return right_(); }
    size_t FieldSize();
    std::string Text();
    color_t Color() =0;
    void Backspace();
    void Type(char);
    bool AllowsSelect();
    void Select();

    NoteCell(
            Note& note,
            std::function<ICell*(void)> left,
            std::function<ICell*(void)> right,
            std::function<ICell*(void)> top,
            std::function<ICell*(void)> bottom)
        : note_(note)
          , staff_(staff)
          , document_(document)
          , left_(left)
          , right_(right)
          , top_(top)
          , bottom_(bottom)
    {}
#if 0
        [=cell]() -> ICell* {
            return cells_[i - 1];
        };
#endif

private:
    Note& note_;
    Staff& staff_;
    Document& document_;
    std::function<ICell*(void)> left_;
    std::function<ICell*(void)> right_;
    std::function<ICell*(void)> top_;
    std::function<ICell*(void)> bottom_;
};

#endif
