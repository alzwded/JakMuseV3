#ifndef DOCUMENT_DISPLAY_H
#define DOCUMENT_DISPLAY_H

#include "icell.h"
#include "document.h"

#include <functional>
#include <sstream>
#include <assert.h>

// ===========================================================
// Common
// ===========================================================

class ACell : public ICell
{
public:
    virtual ~ACell() {}

    point_t Location() { return location_; }
    void SetLocation(point_t p) { location_ = p; }

    void SetWidth(size_t w) { width_ = w; }
    size_t Width() { return width_; }

    ACell(Document& doc)
        : doc_(doc)
          , location_(0, 0)
    {}

public:
    Document& doc_;

protected:
    point_t location_;
    size_t width_;
};

// ===========================================================
// Title
// ===========================================================

class TitleCell : public ACell
{
public:
    virtual ~TitleCell() {}
    point_t Select() { return doc_.Active()->Location(); }
    point_t Mark() { doc_.SetActive(this); doc_.ClearSelection(); return Location(); }
    cell_t GetRenderThing();

    std::string Text() { return doc_.title_; }
    void UserInput(std::string title)
    {
        //doc_.title_ = title;
        // DO NOTHING; title is updated via open/save
    }

    TitleCell(Document& doc)
        : ACell(doc)
    {}

    point_t Left() { return Location(); }
    point_t Right() { return Location(); }
    point_t Top() { return Location(); }
    point_t Bottom() { return doc_.Cell(point_t(14, 1))->Location(); }
};

// ===========================================================
// Staff
// ===========================================================

class StaffName : public ACell
{
public:
    virtual ~StaffName() {}
    point_t Select() { return doc_.Active()->Location(); }
    point_t Mark() { doc_.SetActive(this); doc_.ClearSelection(); return Location(); }
    cell_t GetRenderThing();

    std::string Text() { return doc_.staves_[staffIdx_].name_; }
    void UserInput(std::string);

    point_t Left() { return Location(); }
    point_t Right() { return doc_.Cell(Location().x + Width(), Location().y)->Location(); }
    point_t Top() { return doc_.Cell(Location().x, Location().y - 1)->Location(); }
    point_t Bottom() { 
        ICell* c = doc_.Cell(Location().x, Location().y + 1);
        if(c) return c->Location();
        else return Location();
    }

    void SetStaffIndex(size_t staffIdx) { staffIdx_ = staffIdx; }
    StaffName(Document& doc)
        : ACell(doc)
          , staffIdx_(0)
    {}
private:
    size_t staffIdx_;
};

struct StaffType : public ACell
{
    virtual ~StaffType() {}
    point_t Select() { return doc_.Active()->Location(); }
    point_t Mark() { doc_.SetActive(this); doc_.ClearSelection(); return Location(); }
    cell_t GetRenderThing();

    std::string Text() { return std::string(1, doc_.staves_[staffIdx_].type_); }
    void UserInput(std::string);

    point_t Left() { return doc_.Cell(Location().x - 1, Location().y)->Location(); }
    point_t Right() { return doc_.Cell(Location().x + Width(), Location().y)->Location(); }
    point_t Top() { return doc_.Cell(Location().x, Location().y - 1)->Location(); }
    point_t Bottom() { 
        ICell* c = doc_.Cell(Location().x, Location().y + 1);
        if(c) return c->Location();
        else return Location();
    }

    void SetStaffIndex(size_t staffIdx) { staffIdx_ = staffIdx; }
    StaffType(Document& doc)
        : ACell(doc)
          , staffIdx_(0)
    {}
private:
    size_t staffIdx_;
};

struct StaffScale : public ACell
{
    virtual ~StaffScale() {}
    point_t Select() { return doc_.Active()->Location(); }
    point_t Mark() { doc_.SetActive(this); doc_.ClearSelection(); return Location(); }
    cell_t GetRenderThing();

    std::string Text() { std::stringstream ss; ss << doc_.staves_[staffIdx_].scale_; return ss.str(); }
    void UserInput(std::string);

    point_t Left() { return doc_.Cell(Location().x - 1, Location().y)->Location(); }
    point_t Right() { return doc_.Cell(Location().x + Width(), Location().y)->Location(); }
    point_t Top() { return doc_.Cell(Location().x, Location().y - 1)->Location(); }
    point_t Bottom() { 
        ICell* c = doc_.Cell(Location().x, Location().y + 1);
        if(c) return c->Location();
        else return Location();
    }

    void SetStaffIndex(size_t staffIdx) { staffIdx_ = staffIdx; }
    StaffScale(Document& doc)
        : ACell(doc)
          , staffIdx_(0)
    {}
private:
    size_t staffIdx_;
};

struct StaffInterpolation : public ACell
{
    virtual ~StaffInterpolation() {}
    point_t Select() { return doc_.Active()->Location(); }
    point_t Mark() { doc_.SetActive(this); doc_.ClearSelection(); return Location(); }
    cell_t GetRenderThing();

    std::string Text() { 
        if(doc_.staves_[staffIdx_].type_ == 'P') {
            return std::string(1, doc_.staves_[staffIdx_].interpolation_);
        } else {
            return " ";
        }
    }
    void UserInput(std::string);

    point_t Left() { return doc_.Cell(Location().x - 1, Location().y)->Location(); }
    point_t Right() { return doc_.Cell(Location().x + Width(), Location().y)->Location(); }
    point_t Top() { return doc_.Cell(Location().x, Location().y - 1)->Location(); }
    point_t Bottom() { 
        ICell* c = doc_.Cell(Location().x, Location().y + 1);
        if(c) return c->Location();
        else return Location();
    }

    void SetStaffIndex(size_t staffIdx) { staffIdx_ = staffIdx; }
    StaffInterpolation(Document& doc)
        : ACell(doc)
          , staffIdx_(0)
    {}
private:
    size_t staffIdx_;
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
        NoteCell* note = this;
        while(note && !note->First()) {
            note = dynamic_cast<NoteCell*>(doc_.Cell(note->Left()));
        }
        doc_.SetActive(note);
        doc_.SetSelected(note);
        if(doc_.marked_.x < 0 || doc_.marked_.y < 0) doc_.SetMarked(note);
#if 0
        // This is not a good idea
        if(!note
                || note->CacheIndex() < 0
                || note->CacheIndex() >= doc_.cache_[note->Staff()].size()
                || doc_.cache_[note->Staff()][note->CacheIndex()] < 0)
        {
            doc_.ClearSelection();
        }
#endif
        return Location();
    }

    point_t Mark()
    {
        NoteCell* note = this;
        while(note && !note->First()) {
            note = dynamic_cast<NoteCell*>(doc_.Cell(note->Left()));
        }
        doc_.SetActive(note);
        doc_.SetMarked(note);
        if(!note
                || note->CacheIndex() < 0
                || note->CacheIndex() >= doc_.cache_[note->Staff()].size()
                || doc_.cache_[note->Staff()][note->CacheIndex()] < 0)
        {
            doc_.ClearSelection();
        }
        return Location();
    }

    cell_t GetRenderThing();

    std::string Text()
    {
        if(noteIdx_ < 0) return "";
        return doc_.staves_[staffIdx_].notes_[noteIdx_].BuildString(
                    doc_.staves_[staffIdx_].type_
                );
    }
    void UserInput(std::string);

    point_t Left()
    {
        NoteCell* note = this;
        while(1) {
            ICell* cell = doc_.Cell(
                    point_t(
                       note->Location().x - 1,
                       note->Location().y));
            note = dynamic_cast<NoteCell*>(cell);
            if(!note) return cell->Location();
            else if(note->First()) break;
        }
        return note->Location();
    }
    point_t Right() 
    {
        if(Location().x >= 146) return Location();
        NoteCell* note = dynamic_cast<NoteCell*>(doc_.Cell(point_t(
                    Location().x + 1,
                    Location().y)));
        while(1) {
            if(note->First()) {
                break;
            }
            note = dynamic_cast<NoteCell*>(
                    doc_.Cell(point_t(
                            note->Location().x + 1,
                            note->Location().y)));
            if(!note) return Location();
        }
        return note->Location();
    }
    point_t Top()
    {
        if(Location().y <= 1) {
            return doc_.Cell(0, 0)->Location();
        } else {
            NoteCell* note = dynamic_cast<NoteCell*>(doc_.Cell(point_t(Location().x, Location().y - 1)));
            while(1) {
                if(note->First()) {
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
        NoteCell* note = dynamic_cast<NoteCell*>(doc_.Cell(point_t(Location().x, Location().y + 1)));
        if(!note) return Location();
        while(1) {
            if(note->First()) {
                break;
            }
            note = dynamic_cast<NoteCell*>(
                    doc_.Cell(point_t(
                            note->Location().x - 1,
                            note->Location().y)));
            if(!note) return Location();
        }
        return note->Location();
    }

    void SetStaff(int staffIdx) { staffIdx_ = staffIdx; }
    void SetIndex(int noteIdx) { noteIdx_ = noteIdx; }
    void SetFirst(bool first) { first_ = first; }
    bool First() { return first_; }
    int Staff() { return staffIdx_; }
    int Index() { return noteIdx_; }
    void SetCacheIndex(int i) { cacheIdx_ = i; }
    int CacheIndex() { return cacheIdx_; }

    NoteCell(Document& doc)
        : ACell(doc)
          , staffIdx_(0)
          , noteIdx_(0)
          , cacheIdx_(-1)
          , first_(false)
    {}

private:
    int staffIdx_, noteIdx_, cacheIdx_;
    bool first_;
};

#endif
