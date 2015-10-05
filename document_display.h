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

    point_t Location() { return location_; }
    void SetLocation(point_t p) { location_ = p; }

    ACell(Document& doc)
        : doc_(doc)
          , location_(0, 0)
    {}

private:
    Document& doc_;
    point_t location_;
};

// ===========================================================
// Title
// ===========================================================

class TitleCell : public ACell
{
public:
    virtual ~TitleCell() {}
    point_t Select() { return doc_.Active()->Location(); }
    point_t Mark() { doc_.SetActive(this); return Location(); }
    cell_t GetRenderThing();

    std::string Text() { return doc_.Title(); }
    void UserInput(std::string title)
    {
        doc_.SetTitle(title);
    }

    TitleCell(Document& doc)
        : ACell(doc)
    {}

    point_t Left() { return Location(); }
    point_t Right() { return Location(); }
    point_t Top() { return Location(); }
    point_t Bottom() { return doc_.Cell(point_t(14, 1)->Location(); }
};

// ===========================================================
// Staff
// ===========================================================

class StaffName : public ACell
{
public:
    virtual ~StaffName() {}
    point_t Select() { return doc_.Active()->Location(); }
    point_t Mark() { doc_.SetActive(this); return Location(); }
    cell_t GetRenderThing();

    std::string Text() { return doc_.staves_[staffIdx_].name_; }
    void UserInput(std::string);

    point_t Left() { return Location(); }
    point_t Right() { return doc_.Cell(Location().x + 1, Location().y); }
    point_t Top() { return doc_.Cell(Location().x, Location().y - 1); }
    point_t Bottom() { return doc_.Cell(Location().x, Location().y + 1); }

    void SetStaffIndex(size_t staffIdx) { staffIdx_ = staffIdx; }
    StaffName(Document& doc)
        : ACell(doc)
          , staffIdx_(0)
    {}
private:
    size_t staffIdx_;
};

class StaffType : public ACell
{
};

class StaffScale : public ACell
{
};

class StaffInterpolation : public ACell
{
};

// ===========================================================
// Cell
// ===========================================================

class NoteCell : public ACell
{
public:
    virtual ~NoteCell() {}
    point_t Select()
    {
        doc_.SetSelected(this);
    }

    point_t Mark()
    {
        doc_.SetActive(this);
        doc_.SetMarked(this);
        return Location();
    }

    cell_t GetRenderThing();

    std::string Text()
    {
        return doc_.staves_[staff_].notes_[note_];
    }
    void UserInput(std::string);

    point_t Left()
    {
        if(Location().x == 13) {
            return doc_.Cell(point_t(note->Location().x - 1, note-.Location().y));
        }
        NoteCell* note = this;
        while(1) {
            if(note.first_) {
                break;
            }
            note = dynamic_cast<NoteCell*>(
                    doc_.Cell(point_t(
                            note->Location().x - 1,
                            note->Location().y)));
            assert(note);
        }
        return note->Location();
    }
    point_t Right() 
    {
        NoteCell* note = doc_.Cell(point_t(
                    Location().x + 1,
                    Location().y));
        while(1) {
            if(note.first_) {
                break;
            }
            if(note->Location().x >= 146) return Location();
            note = dynamic_cast<NoteCell*>(
                    doc_.Cell(point_t(
                            note->Location().x + 1,
                            note->Location().y)));
            assert(note);
        }
        return note->Location();
    }
    point_t Top()
    {
        if(Location().y <= 1) {
            return doc_.Cell(0, 0);
        } else {
            NoteCell* note = doc_.Cell(point_t(Location().x, Location().y - 1));
            while(1) {
                if(note.first_) {
                    break;
                }
                note = dynamic_cast<NoteCell*>(
                        doc_.Cell(point_t(
                                note->Location().x - 1,
                                note->Location().y)));
                assert(note);
            }

            return note->Location();
        }
    }

    point_t Bottom()
    {
        if(Location().y >= 11) return Location();
        NoteCell* note = doc_.Cell(point_t(Location().x, Location().y + 1));
        while(1) {
            if(note.first_) {
                break;
            }
            note = dynamic_cast<NoteCell*>(
                    doc_.Cell(point_t(
                            note->Location().x - 1,
                            note->Location().y)));
            assert(note);
        }
        return note->Location();
    }

    void SetStaff(size_t staffIdx) { staffIdx_ = staffIdx; }
    void SetIndex(size_t noteIdx) { noteIdx_ = noteIdx; }
    void SetFirst(bool first) { first_ = first; }
    bool First() { return first_; }

    NoteCell(Document& doc)
        : ACell(doc)
          , staffIdx_(0)
          , noteIdx_(0)

private:
    size_t staffIdx_, noteIdx_;
    bool first_;
};

#endif
