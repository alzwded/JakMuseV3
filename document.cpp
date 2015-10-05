#include "common.h"
#include "document_display.h"
#include "document.h"
#include <algorithm>
#include <assert.h>

static bool computedSize = false;
static size_t numThingsPerLine = 0;

Document::Document()
{
    for(int i = 0; i < 10; ++i) {
        staves_.emplace_back();
        staves_.back().type_ = 'N';
        staves_.back().scale_ = 64;
        staves_.back().interpolation_ = 'T';
    }
}

ICell* Document::Cell(point_t p)
{
    auto&& found = std::find_if(cells_.begin(), cells_.end(),
            [&p](decltype(cells_)::const_reference c) -> bool {
                point_t tl = c->Location();
                point_t br = point_t(tl.x + c->Width(), tl.y + 1);

                return p.x >= tl.x && p.x < br.x
                    && p.y >= tl.y && p.y < br.y
                    ;
            });
    assert(found != cells_.end());
    return *found;
}

void Document::InitCells()
{
    // title bar
    TitleCell* title = new TitleCell(*this);
    title->SetLocation(point_t(0, 0));
    title->SetWidth(COLUMNS * N);
    cells_.push_back(title);

    for(int i = 0; i < 10; ++i) {
        size_t x = 0;
        StaffName* name = new StaffName(*this);
        name->SetStaffIndex(i);
        name->SetLocation(point_t(x, i + 1));
        name->SetWidth(8 * N);
        cells_.push_back(name);
        x += name->Width();

        StaffType* type = new StaffType(*this);
        type->SetStaffIndex(i);
        type->SetLocation(point_t(x, i + 1));
        type->SetWidth(N);
        cells_.push_back(type);
        x += type->Width();
        
        StaffScale* scale = new StaffScale(*this);
        scale->SetStaffIndex(i);
        scale->SetLocation(point_t(x, i + 1));
        scale->SetWidth(3 * N);
        cells_.push_back(scale);
        x += scale->Width();

        StaffInterpolation* interpolation = new StaffInterpolation(*this);
        interpolation->SetStaffIndex(i);
        interpolation->SetWidth(N);
        interpolation->SetLocation(point_t(x, i + 1));
        cells_.push_back(interpolation);
        x += interpolation->Width();

        if(!computedSize) numThingsPerLine = 4;

        while(x < COLUMNS * N) {
            NoteCell* note = new NoteCell(*this);
            note->SetStaff(i);
            note->SetLocation(point_t(x, i + 1));
            note->SetWidth(1);
            cells_.push_back(note);
            x += note->Width();
            if(!computedSize) ++numThingsPerLine;
        }

        computedSize = true;
    }

    Scroll(0);
}

void Document::UpdateCache()
{
    cache_.clear();
    for(int i = 0; i < 10; ++i) {
        cache_.emplace_back();
        int j = 0;
        for(; j < staves_[i].notes_.size(); ++j) {
            for(int k = 0; k < staves_[i].notes_[j].scale_; ++k) {
                cache_[i].push_back(j);
                if(cache_[i].size() >= COLUMNS - 14) break;
            }
        }
        for(; j < COLUMNS - 14; ++j) {
            cache_[i].push_back(-1);
        }
    }
}

void Document::Scroll(size_t col)
{
    for(int i = 0; i < 10; ++i) {
        int j;
        for(j = 0; j < numThingsPerLine - 4; ++j) {
            NoteCell* note = dynamic_cast<NoteCell*>(
                    cells_[1 // title
                        + i * numThingsPerLine // previous staves
                        + 4 // staff header
                        + j // this note
                        ]
                    );
            assert(note);

            note->SetCacheIndex(col + j);

            if(col + j < cache_[i].size()) {
                note->SetIndex(cache_[i][col + j]);
            } else {
                note->SetIndex(-1);
            }

            note->SetFirst(
                    (j == 0)
                    || (col + j == 0)
                    || (cache_[i][col + j - 1] != cache_[i][col + j])
                    );
        }
    }
}

void Document::ScrollRight(bool byPage)
{
    if(byPage) {
        scroll_ += (COLUMNS - 14);
    } else {
        scroll_ += ((COLUMNS - 14) * 2 / 5);
    }
    bool good = false;
    size_t last = 0;
    for(size_t i = 0; i < 10; ++i) {
        if(cache_[i].size() > scroll_) {
            good = true;
            last = std::max(last, cache_[i].size() - (COLUMNS - 14));
        }
    }
    if(!good) scroll_ = last;
    if(scroll_ < 0) scroll_ = 0;
}

void Document::Save(std::ostream& fout)
{
    for(Staff& s : staves_) {
        fout << s.name_ << " ";
        switch(s.type_) {
        case 'N':
            fout << "NOTES" << " ";
            fout << "{ " << "Divisor=" << s.scale_ << ", Notes=[" << std::endl;
            for(size_t i = 0; i < s.notes_.size(); ++i) {
                if((i + 1) % 16 == 0) fout << std::endl;
                Note n = s.notes_[i];
                fout << n.BuildString('N');
                if(i < s.notes_.size() - 1) fout << ", ";
                else fout << "]";
            }
            fout << std::endl << "}" << std::endl;
            break;
        case 'P':
            fout << "PCM" << " " << "{Interpolation=";
            switch(s.interpolation_) {
            case 'C': fout << "Cosine";
            case 'T': fout << "Trunc";
            case 'L': fout << "Linear";
            }
            fout << ", Stride=" << s.scale_ << ", " << "Samples=[" << std::endl;
            for(size_t i = 0; i < s.notes_.size(); ++i) {
                if((i + 1) % 16 == 0) fout << std::endl;
                Note n = s.notes_[i];
                fout << n.BuildString('P');
                if(i < s.notes_.size() - 1) fout << ", ";
                else fout << "]";
            }
            fout << std::endl << "}" << std::endl;
            break;
        }
    }
}

void Document::Open(std::istream& fin)
{
    throw 1;
}

void Document::SetActive(ICell* c)
{
    active_ = c->Location();
}

void Document::SetMarked(ICell* c)
{
    NoteCell* note = dynamic_cast<NoteCell*>(c);
    assert(note);

    if(note->Index() >= 0) {
        marked_.x = note->Index();
        marked_.y = note->Staff();
    }
}

void Document::SetSelected(ICell* c)
{
    NoteCell* note = dynamic_cast<NoteCell*>(c);
    assert(note);

    if(note->Index() >= 0) {
        selected_.x = note->Index();
        selected_.y = note->Staff();
    }
}

void Document::Cut()
{
    throw 1;
}

void Document::Copy()
{
    throw 1;
}

void Document::Paste()
{
    throw 1;
}

void Document::NewNote()
{
    throw 1;
}

void Document::Delete()
{
    throw 1;
}

cell_t TitleCell::GetRenderThing()
{
    cell_t ret;
    ret.x = Location().x;
    ret.y = Location().y;
    ret.type = cell_t::BLOCK;
    ret.color = color_t::WHITE;
    ret.text = (char*)malloc(sizeof(char) * COLUMNS);
    ret.ntext = COLUMNS;
    for(size_t i = 0; i < COLUMNS; ++i) {
        if(i < Text().size()) ret.text[i] = Text()[i];
        else ret.text[i] = ' ';
    }
    return ret;
}

cell_t StaffName::GetRenderThing()
{
    cell_t ret;
    ret.x = Location().x;
    ret.y = Location().y;
    ret.type = cell_t::BLOCK;
    ret.color = color_t::WHITE;
    ret.text = (char*)malloc(sizeof(char) * 8);
    ret.ntext = 8;
    for(size_t i = 0; i < 8; ++i) {
        if(i < Text().size()) ret.text[i] = Text()[i];
        else ret.text[i] = ' ';
    }
    return ret;
}

void StaffName::UserInput(std::string s)
{
    doc_.staves_[staffIdx_].name_ = s;
}

cell_t StaffType::GetRenderThing()
{
    cell_t ret;
    ret.x = Location().x;
    ret.y = Location().y;
    ret.type = cell_t::BLOCK;
    ret.color = color_t::GOLD;
    ret.text = (char*)malloc(sizeof(char) * 1);
    ret.ntext = 1;
    ret.text[0] = Text()[0];
    return ret;
}

void StaffType::UserInput(std::string s)
{
    if(s.empty()) return;
    switch(s[0]) {
    case 'N':
    case 'P':
        doc_.staves_[staffIdx_].type_ = s[0];
        break;
    }
}

cell_t StaffScale::GetRenderThing()
{
    cell_t ret;
    ret.x = Location().x;
    ret.y = Location().y;
    ret.type = cell_t::BLOCK;
    ret.color = color_t::WHITE;
    ret.text = (char*)malloc(sizeof(char) * 3);
    ret.ntext = 3;
    size_t scale = doc_.staves_[staffIdx_].scale_;
    if(scale < 1000) {
        ret.text[0] = ((scale / 100) % 10) + '0';
        ret.text[1] = ((scale / 10) % 10) + '0';
        ret.text[2] = ((scale / 1) % 10) + '0';
    } else {
        ret.text[0] = 'B';
        ret.text[1] = 'I';
        ret.text[2] = 'G';
    }
    return ret;
}

void StaffScale::UserInput(std::string s)
{
    if(s.empty()) return;
    int i = atoi(s.c_str());
    if(i < 0) return;
    doc_.staves_[staffIdx_].scale_ = i;
}

cell_t StaffInterpolation::GetRenderThing()
{
    cell_t ret;
    ret.x = Location().x;
    ret.y = Location().y;
    ret.type = cell_t::BLOCK;
    ret.color = color_t::GOLD;
    ret.text = (char*)malloc(sizeof(char) * 1);
    ret.ntext = 1;
    if(doc_.staves_[staffIdx_].type_ == 'P') {
        ret.text[0] = Text()[0];
    } else {
        ret.text[0] = ' ';
    }
    return ret;
}

void StaffInterpolation::UserInput(std::string s)
{
    if(s.empty()) return;
    switch(s[0]) {
    case 'C':
    case 'T':
    case 'L':
        doc_.staves_[staffIdx_].type_ = s[0];
        break;
    }
}

void NoteCell::UserInput(std::string text)
{
    if(noteIdx_ < 0) return;

    Note& c = doc_.staves_[staffIdx_].notes_[noteIdx_];
    if(text.empty()) {
        c.scale_ = 1;
        c.name_ = '-';
        c.sharp_ = ' ';
        c.height_ = ' ';
    }
    bool valid = true;
    // expecting a number
    std::string number;
    for(size_t i = 0; i < text.size(); ++i) {
        if(isdigit(text[i])) {
            number.append(std::string(1, text[i]));
        } else {
            break;
        }
    }
    if(number.empty()) { return; }
    text = text.substr(number.size());
    // expecting a note name
    if(text.empty()) return;
    static const char noteNames[] = "ABCDEFGH-";
    if(!strchr(noteNames, text[0])) {
        return;
    }
    char noteName = text[0];
    text = text.substr(1);
    // expecting an optional sharp
    if(text.empty()) return;
    char sharp = ' ';
    if(text[0] == '#' || text[1] == 'b') {
        sharp = text[0];
        text = text.substr(1);
    }
    // expecting a height
    if(text.empty()) return;
    char height = 0;
    if(isdigit(text[0])) {
        height = text[0];
    }

    c.scale_ = atoi(number.c_str());
    c.name_ = noteName;
    c.sharp_ = sharp;
    c.height_ = height;

    doc_.UpdateCache();
}

cell_t NoteCell::GetRenderThing()
{
    cell_t ret;
    ret.x = Location().x;
    ret.y = Location().y;
    ret.text = (char*)malloc(sizeof(char) * 4);
    ret.ntext = 4;
    if(doc_.staves_[staffIdx_].type_ == 'N') {
        ret.type = cell_t::NOTE;
        if(noteIdx_ >= 0) {
            ret.color = (noteIdx_ % 2) ? color_t::YELLOW : color_t::WHITE;
            ret.text[3] = (noteIdx_ % 2) ? '\0' : 'T';
            if(first_) {
                Note& n = doc_.staves_[staffIdx_].notes_[noteIdx_];
                ret.text[0] = n.name_;
                ret.text[1] = n.height_;
                ret.text[2] = n.sharp_;
            } else {
                ret.text[0] = ' ';
                ret.text[1] = ' ';
                ret.text[2] = ' ';
            }
        } else {
            ret.color = color_t::BLACK;
            ret.text[0] = ret.text[1] = ret.text[2] = ' ';
            ret.text[4] = '\0';
        }
    } else if(doc_.staves_[staffIdx_].type_ == 'P') {
        ret.type = cell_t::SAMPLE;
        if(noteIdx_ >= 0) {
            ret.color = (noteIdx_ % 2) ? color_t::SEA : color_t::SKY;
            ret.text[3] = '\0';
            if(first_) {
                Note& n = doc_.staves_[staffIdx_].notes_[noteIdx_];
                ret.text[0] = n.name_;
                ret.text[1] = n.height_;
                ret.text[2] = n.sharp_;
            } else {
                ret.text[0] = ' ';
                ret.text[1] = ' ';
                ret.text[2] = ' ';
            }
        } else {
            ret.color = color_t::BLACK;
            ret.text[0] = ret.text[1] = ret.text[2] = ' ';
            ret.text[4] = '\0';
        }
    }
    return ret;
}
