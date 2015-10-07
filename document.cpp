#include "common.h"
#include "document_display.h"
#include "document.h"
#include <algorithm>
#include <numeric>
#include <assert.h>

static bool computedSize = false;
static size_t numThingsPerLine = 0;

Document::Document()
{
    for(int i = 0; i < ROWS - 2; ++i) {
        staves_.emplace_back();
        staves_.back().type_ = 'N';
        staves_.back().scale_ = 48;
        staves_.back().interpolation_ = 'T';
    }
    title_ = "My new song";
    active_.x = 13 * N;
    active_.y = 1;
    // BEGIN TEST CODE
    Note n = { 12, 'C', ' ', '4' };
    staves_[1].notes_.push_back(n);
    staves_[0].notes_.push_back(n);
    staves_[0].notes_.push_back(n);
    n.sharp_ = '#';
    staves_[0].notes_.push_back(n);
    n.sharp_ = 'b';
    staves_[0].notes_.push_back(n);
    staves_[0].name_ = "I1";
    staves_[1].name_ = "I2";
    n.scale_ = 18;
    n.sharp_ = ' ';
    staves_[2].notes_.push_back(n);
    n.scale_ = 12;
    staves_[2].notes_.push_back(n);
    n.scale_ = 6;
    staves_[2].notes_.push_back(n);
    n.name_ = '-';
    staves_[2].notes_.push_back(n);
    staves_[2].name_ = "I3";
    staves_[3].name_ = "PCM";
    staves_[3].type_ = 'P';
    n.scale_ = 18;
    n.name_ = '0';
    n.height_ = '1';
    n.sharp_ = '6';
    staves_[3].notes_.push_back(n);
    n.name_ = '1';
    n.height_ = '2';
    n.sharp_ = '8';
    staves_[3].notes_.push_back(n);
    n.scale_ = 6;
    staves_[3].notes_.push_back(n);
    staves_[3].notes_.push_back(n);

    marked_ = point_t(0, 2);
    selected_ = point_t(1, 3);
    active_ = point_t(13 * N, 3);
    // END TEST CODE
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
    if(found != cells_.end()) return *found;
    return NULL;
}

void Document::InitCells()
{
    // title bar
    TitleCell* title = new TitleCell(*this);
    title->SetLocation(point_t(0, 0));
    title->SetWidth(COLUMNS * N);
    cells_.push_back(title);

    for(int i = 0; i < ROWS - 2; ++i) {
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
    for(int i = 0; i < ROWS - 2; ++i) {
        cache_.emplace_back();
        for(size_t j = 0; j < staves_[i].notes_.size(); ++j) {
            for(int k = 0; k < staves_[i].notes_[j].scale_; ++k) {
                cache_[i].push_back(j);
                // if(cache_[i].size() >= COLUMNS - 14) break;
            }
        }
        // cache_ should be empty if there are no notes and should only be as long as a full staff
        //for(; j < (COLUMNS - 13) * 2; ++j) {
        //    cache_[i].push_back(-1);
        //}
    }
}

void Document::Scroll(size_t col)
{
    for(int i = 0; i < ROWS - 2; ++i) {
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
                note->SetFirst(
                        (j == 0)
                        || (col + j == 0)
                        || (cache_[i][col + j - 1] != cache_[i][col + j])
                        );
                continue;
            } else {
                note->SetIndex(-1);
                note->SetFirst((j == 0) || (col + j == cache_[i].size()));
                continue;
            }

        }
    }
}

void Document::ScrollLeftRight(int ama)
{
    int next = scroll_ + ama;
    if(next < 0) next = 0;
    bool good = false;
    int last = 0;
    for(size_t i = 0; i < ROWS - 2; ++i) {
        //printf(">%d\n", cache_[i].size());
        const int lineLength = N*(COLUMNS-13);
        int lineDiff = cache_[i].size() - next;
        int preferred = cache_[i].size() - lineLength;
        //printf("%d %d %d\n", lineLength, lineDiff, preferred);
        if(lineDiff > lineLength) {
            preferred = next;
        }
        last = std::max(last, preferred);
        //printf("%d\n", last);
        if(next >= cache_[i].size() - N*(COLUMNS-13)) {
            good = true;
        }
    }
    /*if(good)*/ scroll_ = std::min(last, next);
    //printf("    %d\n", scroll_);
    Scroll(scroll_);

}

void Document::ScrollRight(bool byPage)
{
    if(byPage) ScrollLeftRight(N*(COLUMNS - 13));
    else ScrollLeftRight((COLUMNS - 13) * 2 * N / 5);
}

void Document::ScrollLeft(bool byPage)
{
    if(byPage) ScrollLeftRight(-N*(COLUMNS - 13));
    else ScrollLeftRight(-((COLUMNS - 13) * 2 * N / 5));
}

void Document::Save(std::ostream& fout)
{
    for(Staff& s : staves_) {
        if(s.name_.empty()) continue;

        fout << s.name_ << " ";
        switch(s.type_) {
        case 'N':
            fout << "NOTES" << " ";
            fout << "{ " << "Divisor=" << s.scale_ << ", Notes=[" << std::endl << std::string(4, ' ');
            for(size_t i = 0; i < s.notes_.size(); ++i) {
                if((i + 1) % 16 == 0) fout << std::endl << std::string(4, ' ');
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
            case 'C': fout << "Cosine"; break;
            case 'T': fout << "Trunc"; break;
            case 'L': fout << "Linear"; break;
            }
            fout << ", Stride=" << s.scale_ << ", " << "Samples=[" << std::endl << std::string(4, ' ');
            for(size_t i = 0; i < s.notes_.size(); ++i) {
                if((i + 1) % 16 == 0) fout << std::endl << std::string(4, ' ');
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

void Document::ClearSelection()
{
    marked_ = point_t(-1, -1);
    selected_ = point_t(-1, -1);
}

void Document::Cut()
{
    auto pc = PreCutSelection();
    buffer_ = pc.buffer;
    pc.cut();
    ClearSelection();
    UpdateCache();
    ScrollLeftRight(0);
    Active()->Select();
    Active()->Mark();
}

void Document::Copy()
{
    auto pc = PreCutSelection();
    buffer_ = pc.buffer;
}

void Document::Paste()
{
    throw 1;
}

void Document::NewNote()
{
    throw 1;
}

Document::BufferOp Document::PreCutSelection()
{
    BufferOp ret;

    std::deque<std::function<void(void)>> cuts;
    int left, right, top, bottom;
    if(!GetSelectionBox(left, right, top, bottom)) return ret;
    for(int i = top; i <= bottom; ++i) {
        Staff& s = staves_[i];
        ret.buffer.emplace_back();

        decltype(s.notes_)::iterator first = s.notes_.end(), last = s.notes_.end();
        int n = 0;
        for(auto&& it = s.notes_.begin(); it != s.notes_.end(); ++it) {
            int idx = it - s.notes_.begin();
            if(IsNoteSelected(i, idx)) {
                ret.buffer[i - top].push_back(s.notes_[idx]);
                if(first != s.notes_.end()) last = it;
                else first = it, last = it;
            }
        }

        Staff* pStaff = &s;
        cuts.push_back([pStaff, first, last]() mutable {
                    if(first != pStaff->notes_.end()) pStaff->notes_.erase(first, last + 1);
                });
    }

    ret.cut = [cuts]() {
        for(auto&& f : cuts) f();
    };

    return std::move(ret);
}

void Document::Delete()
{
    ICell* c = Active();
    if(c->Location().y == 0) return;
    if(c->Location().x < N*13) {
        Staff& s = staves_[c->Location().y - 1];
        s.name_ = "";
        s.type_ = 'N';
        s.notes_.clear();
        UpdateCache();
        ScrollLeftRight(0);
        //Scroll(scroll_);
    } else {
        auto pc = PreCutSelection();
        pc.cut();
        ClearSelection();
        UpdateCache();
        ScrollLeftRight(0);
        Active()->Select();
        Active()->Mark();
    }
}

int Document::Duration()
{
    size_t maxDuration = 0;
    maxDuration = std::accumulate(staves_.begin(), staves_.end(), maxDuration,
            [](size_t duration, Staff const& s) -> size_t {
                double scale = (s.scale_) ? 44100.0 / s.scale_ : 1.0;
                double dur = 0;
                dur = std::accumulate(s.notes_.begin(), s.notes_.end(), dur, [scale](double dur, Note const& n) -> double {
                        return dur + scale * n.scale_;
                    });
                return std::max(duration, (size_t)dur);
            });
    return maxDuration / 44100;
}

int Document::Position()
{
    return scroll_;
}

int Document::Max()
{
    int max = 0;
    max = std::accumulate(cache_.begin(), cache_.end(), max, [](int max, decltype(cache_)::const_reference line) -> int {
                return std::max(max, (int)line.size());
            });
    return max;
}

bool Document::AtEnd()
{ 
    return scroll_ + N*(COLUMNS - 13) >= Max();
}


int Document::Percentage()
{
    int max = 1;
    max = std::accumulate(cache_.begin(), cache_.end(), max, [](int orig, decltype(cache_)::const_reference curr) -> int {
                int potential = curr.size() - 1;
                while(potential >= 0) {
                    if(curr[potential] >= 0) break;
                    --potential;
                }
                return std::max(orig, potential);
            });
    return scroll_ * 100 / max;
}

static void AdjustColumn(Document& doc, int row, int& note)
{
    int col = -1;
    for(size_t i = 0; i < note; ++i) {
        col += doc.staves_[row].notes_[i].scale_;
    }
    note = col + 1;
}

bool FirstNoteInRange(NoteCell& note, int left, int right)
{
    NoteCell* n = &note;
    while(!n->First()) {
        point_t p(n->Location().x - 1, n->Location().y);
        n = (NoteCell*)n->doc_.Cell(p);
        if(!n) return false;
    }
    return n->CacheIndex() >= left && n->CacheIndex() <= right;
}

bool Document::GetSelectionBox(int& left, int& right, int& top, int& bottom)
{
    left = marked_.x;
    right = selected_.x;
    top = marked_.y;
    bottom = selected_.y;

    if(left < 0 || right < 0 || top < 0 || bottom < 0) return false;

    AdjustColumn(*this, top, left);
    AdjustColumn(*this, bottom, right);
    if(left > right) {
        std::swap(left, right);
        right += staves_[marked_.y].notes_[marked_.x].scale_ - 1;
    } else {
        right += staves_[selected_.y].notes_[selected_.x].scale_ - 1;
    }
    if(top > bottom) std::swap(top, bottom);

    return true;
}

bool Document::IsNoteSelected(ICell* c)
{
    NoteCell* note = dynamic_cast<NoteCell*>(c);
    if(!note) return false;

    int left, right, top, bottom;
    if(!GetSelectionBox(left, right, top, bottom)) return false;

    if(note->Staff() >= top && note->Staff() <= bottom
            && FirstNoteInRange(*note, left, right))
    {
        return true;
    }

    return false;
}

bool Document::IsNoteSelected(int staffIdx, int noteIdx)
{
    int left, right, top, bottom;
    if(!GetSelectionBox(left, right, top, bottom)) return false;

    int col = noteIdx;
    AdjustColumn(*this, staffIdx, col);

    if(staffIdx >= top && staffIdx <= bottom
            && col >= left && col <= right)
    {
        return true;
    } else {
        return false;
    }
}

void Document::PushState()
{
    undoStates_.push_back(staves_);
    if(undoStates_.size() > 9) undoStates_.pop_front();
    UpdateCache();
    ScrollLeftRight(0);
}

void Document::PopState()
{
    if(undoStates_.empty()) return;
    auto&& state = undoStates_.back();
    staves_ = state;
    undoStates_.pop_back();
    UpdateCache();
    ScrollLeftRight(0);
}
