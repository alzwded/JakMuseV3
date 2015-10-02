#ifndef DOCUMENT_DISPLAY_H
#define DOCUMENT_DISPLAY_H

#include "icell.h"
#include "document.h"

#include <functional>

// ===========================================================
// Common
// ===========================================================

class ACell : public ICell
{
public:
    virtual ~ACell() {}

    ICell* Up() { return up_; }
    ICell* Down() { return down_; }
    ICell* Left() { return left_; }
    ICell* Right() { return right_; }
    std::pair<size_t, size_t> Location() { return coord_; }

    void SetUp(ICell* up) { up_ = up; }
    void SetDown(ICell* down) { down_ = down; }
    void SetLeft(ICell* left) { left_ = left; }
    void SetRight(ICell* right) { right_ = right; }
    void SetLocation(size_t top, size_t left) { coord_.first = top; coord_.second = left; }

    ACell()
        : up_(nullptr), down_(nullptr), left_(nullptr), right_(nullptr)
          , coord_(0, 0)
    {}

private:
    ICell* up_, down_, left_, right_;
    std::pair<size_t, size_t> coord_;
};

// ===========================================================
// Staff
// ===========================================================

class StaffName : public ACell
{
public:
    virtual ~StaffName() {}
    size_t FieldSize() { return 8; }
    std::string Text() { return staff_.GetName(); }
    color_t Color() { return color_t::WHITE; }
    void Backspace();
    void Type(char);
    bool AllowsSelect() { return false; }

private:
    Staff* staff_;
};

class StaffType : public ACell
{
public:
    virtual ~StaffType() {}
    size_t FieldSize() { return 1; }
    std::string Text();
    color_t Color() { return color_t::BLACK; }
    void Backspace();
    void Type(char);
    bool AllowsSelect() { return false; }

private:
    Staff* staff_;
};

class StaffScale : public ACell
{
public:
    virtual ~StaffScale() {}
    size_t FieldSize() { return 3; }
    std::string Text() { return staff_->GetScale(); }
    color_t Color() { return color_t::WHITE; }
    void Backspace();
    void Type(char);
    bool AllowsSelect() { return false; }

private:
    Staff* staff_;
};

class StaffInterpolation : public ACell
{
public:
    virtual ~StaffInterpolation() {}
    size_t FieldSize() { return 1; }
    std::string Text();
    color_t Color() { return color_t::BLACK; }
    void Backspace();
    void Type(char);
    bool AllowsSelect() { return false; }

private:
    Staff* staff_;
};

// ===========================================================
// Cell
// ===========================================================

class NoteCell : public ACell
{
public:
    virtual ~NoteCell() {}
    size_t FieldSize() { return 1; }
    std::string Text();
    color_t Color();
    void Backspace();
    void Type(char);
    bool AllowsSelect() { return note_; }

    void SetNote(Note*);

private:
    Note* note_;
};

// ===========================================================
// Title
// ===========================================================

class TitleCell : public ACell
{
public:
    virtual ~TitleCell() {}
    size_t FieldSize() { return 77; }
    std::string Text() { if(title_) return *title_; return ""; }
    color_t Color() { return color_t::WHITE; }
    void Backspace();
    void Type(char);
    bool AllowSelect() { return false; }

    void SetTitle(std::string* title) { title_ = title; }

    TitleCell() : title_(nullptr) {}

private:
    std::string* title_;
};

#endif
