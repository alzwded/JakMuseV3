/*
Copyright (c) 2015, Vlad Mesco
All rights reserved.

Redistribution and use in source and binary forms, with or without
modification, are permitted provided that the following conditions are met:

* Redistributions of source code must retain the above copyright notice, this
  list of conditions and the following disclaimer.

* Redistributions in binary form must reproduce the above copyright notice,
  this list of conditions and the following disclaimer in the documentation
  and/or other materials provided with the distribution.

THIS SOFTWARE IS PROVIDED BY THE COPYRIGHT HOLDERS AND CONTRIBUTORS "AS IS"
AND ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE
IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE ARE
DISCLAIMED. IN NO EVENT SHALL THE COPYRIGHT HOLDER OR CONTRIBUTORS BE LIABLE
FOR ANY DIRECT, INDIRECT, INCIDENTAL, SPECIAL, EXEMPLARY, OR CONSEQUENTIAL
DAMAGES (INCLUDING, BUT NOT LIMITED TO, PROCUREMENT OF SUBSTITUTE GOODS OR
SERVICES; LOSS OF USE, DATA, OR PROFITS; OR BUSINESS INTERRUPTION) HOWEVER
CAUSED AND ON ANY THEORY OF LIABILITY, WHETHER IN CONTRACT, STRICT LIABILITY,
OR TORT (INCLUDING NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY OUT OF THE USE
OF THIS SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY OF SUCH DAMAGE.
*/

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
    selected_ = marked_ = point_t(0, 0);
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
    switch(insertMode_)
    {
    case InsertMode_t::INSERT:
        {
            point_t pos = marked_;
            int staffIdx = Active()->Location().y - 1;
            if(pos.y >= 0) staffIdx = pos.y;
            if(staffIdx < 0) return;

            if(marked_.x < 0) pos = point_t(0, staffIdx);

            for(size_t i = 0; i < buffer_.size(); ++i) {
                if(staffIdx + i >= staves_.size());
                Staff& s = staves_[staffIdx + i];
                decltype(s.notes_)::iterator it;
                if(marked_.x >= 0 && marked_.x < s.notes_.size()) it = s.notes_.begin() + marked_.x;
                else { it = s.notes_.begin(); }

                for(size_t j = 0; j < buffer_[i].size(); ++j) {
                    it = s.notes_.insert(it, buffer_[i][j]);
                    ++it;
                }
            }

            ClearSelection();
            marked_ = pos;
            selected_ = pos;
            UpdateCache();
            ScrollLeftRight(0);
            SetActiveToMarked();
            ScrollLeftRight(0);
        }
        break;
    case InsertMode_t::APPEND:
        {
            point_t pos = { std::max(marked_.x, selected_.x), std::max(marked_.y, selected_.y) };
            pos.x++;
            int staffIdx = Active()->Location().y - 1;
            if(pos.y >= 0) staffIdx = pos.y;
            if(staffIdx < 0) return;

            if(marked_.x < 0) pos = point_t(staves_[staffIdx].notes_.size(), staffIdx);

            for(size_t i = 0; i < buffer_.size(); ++i) {
                if(staffIdx + i >= staves_.size());
                Staff& s = staves_[staffIdx + i];
                decltype(s.notes_)::iterator it;
                if(marked_.x >= 0 && marked_.x < s.notes_.size() - 1) it = s.notes_.begin() + marked_.x + 1;
                else { it = s.notes_.end(); }

                for(size_t j = 0; j < buffer_[i].size(); ++j) {
                    it = s.notes_.insert(it, buffer_[i][j]);
                    ++it;
                }
            }

            ClearSelection();
            marked_ = pos;
            selected_ = pos;
            UpdateCache();
            ScrollLeftRight(0);
            SetActiveToMarked();
            ScrollLeftRight(0);
        }
        break;
    case InsertMode_t::REPLACE:
        {
            if(marked_.x < 0 || marked_.y < 0) return;
            point_t pos = marked_;
            //printf("%d %d %d %d\n", marked_.x, marked_.y, selected_.y, selected_.y);
            if(selected_.x >= 0 && selected_.y >= 0) {
                pos.x = std::min(selected_.x, pos.x);
                pos.y = std::min(selected_.y, pos.y);
            }
            BufferOp op = PreCutSelection();
            op.cut();
            //printf("%d %d ==\n", pos.x, pos.y);
            marked_ = pos;
            selected_ = pos;
            insertMode_ = InsertMode_t::INSERT;
            Paste();
            insertMode_ = InsertMode_t::REPLACE;
            ClearSelection();
            marked_ = pos;
            selected_ = pos;
            UpdateCache();
            ScrollLeftRight(0);
            SetActiveToMarked();
            ScrollLeftRight(0);
        }
        break;
    }
}

void Document::SetActiveToMarked()
{
    if(marked_.x < 0 || marked_.y < 0) { /*printf("no marked\n");*/ return; }
    Staff& s = staves_[marked_.y];
    int pos = 0;
    pos = std::accumulate(s.notes_.begin(), s.notes_.begin() + marked_.x, pos,
            [](int pos, decltype(s.notes_)::const_reference n) -> int {
                return pos + n.scale_;
            });

    if(!(pos >= scroll_ && pos < scroll_ + N*(COLUMNS - 13))) {
        scroll_ = pos;
        Scroll(pos);
        ScrollLeftRight(0);
    }
    UpdateCache();
    //printf("P %d\n", pos);

    auto&& cache = cache_;
    int staffIdx = marked_.y;
    auto&& found = std::find_if(cells_.begin(), cells_.end(),
            [staffIdx, pos, cache](ICell* c) -> bool {
                NoteCell* nc = dynamic_cast<NoteCell*>(c);
                if(!nc) return false;
                //printf("   a note: s%d ci%d\n", nc->Staff(), nc->CacheIndex());
                return (nc->Staff() == staffIdx)
                    && nc->CacheIndex() == pos;
                    //&& nc->CacheIndex() >= 0 && nc->CacheIndex() < cache[staffIdx].size()
                    //&& cache[staffIdx][nc->CacheIndex()] == pos;
            });
    if(found == cells_.end()) {
        printf("nothin fuond\n");
        return;
    }
    NoteCell* nc = dynamic_cast<NoteCell*>(*found);
    if(!nc) return;

    while(!nc->First()) {
        //printf("moving left\n");
        nc = dynamic_cast<NoteCell*>(Cell(nc->Location().x - 1, nc->Location().y));
    }

    active_ = nc->Location();
}

void Document::NewNote()
{
    Note n = { 12, '-', ' ', ' ' };
    Note p = { 12, '5', '0', '0' };
    switch(insertMode_)
    {
    case InsertMode_t::INSERT:
        {
            point_t pos = marked_;
            int staffIdx = Active()->Location().y - 1;
            if(pos.y >= 0) staffIdx = pos.y;
            if(staffIdx < 0) return;
            Staff& s = staves_[staffIdx];
            decltype(s.notes_)::iterator it;
            if(marked_.x >= 0) it = s.notes_.begin() + marked_.x;
            else {
                it = s.notes_.begin();
                pos = point_t(0, staffIdx);
            }
            //printf("%d %d\n", marked_.x, it - s.notes_.begin());
            if(s.type_ == 'P') n = p;
            s.notes_.insert(it, n);
            ClearSelection();
            marked_ = pos;
            selected_ = pos;
            UpdateCache();
            ScrollLeftRight(0);
            SetActiveToMarked();
            ScrollLeftRight(0);
        }
        break;
    case InsertMode_t::APPEND:
        {
            point_t pos = { std::max(marked_.x, selected_.x), std::max(marked_.y, selected_.y) };
            pos.x++;
            int staffIdx = Active()->Location().y - 1;
            if(pos.y >= 0) staffIdx = pos.y;
            if(staffIdx < 0) return;
            Staff& s = staves_[staffIdx];
            decltype(s.notes_)::iterator it;
            if(marked_.x >= 0) it = s.notes_.begin() + marked_.x;
            else {
                it = s.notes_.end();
                pos = point_t(s.notes_.size(), staffIdx);
            }
            if(it != s.notes_.end()) ++it;
            if(s.type_ == 'P') n = p;
            s.notes_.insert(it, n);
            ClearSelection();
            marked_ = pos;
            selected_ = pos;
            UpdateCache();
            ScrollLeftRight(0);
            SetActiveToMarked();
            ScrollLeftRight(0);
        }
        break;
    case InsertMode_t::REPLACE:
        {
            if(marked_.x < 0 || marked_.y < 0) return;
            point_t pos = marked_;
            if(selected_.x >= 0 && selected_.y >= 0) {
                pos.x = std::min(selected_.x, pos.x);
                pos.y = std::min(selected_.y, pos.y);
            }
            BufferOp op = PreCutSelection();
            op.cut();
            marked_ = pos;
            selected_ = pos;
            insertMode_ = InsertMode_t::INSERT;
            NewNote();
            insertMode_ = InsertMode_t::REPLACE;
            ClearSelection();
            marked_ = pos;
            selected_ = pos;
            UpdateCache();
            ScrollLeftRight(0);
            SetActiveToMarked();
            ScrollLeftRight(0);
        }
        break;
    }
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

    if(right < 0) right = left;
    if(bottom < 0) bottom = top;

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

std::string Note::BuildString(char type)
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
        int scale = n.scale_;
        if(n.name_ >= '0' && n.name_ <= '9') {
            int samp = (n.name_ - '0') * 100 + (n.height_ - '0') * 10 + (n.sharp_ - '0');
            ss << scale << " " << samp;
        } else {
            int samp = (n.name_ - 'A') * 100 + (n.height_ - '0') * 10 + (n.sharp_ - '0');
            ss << scale << " " << (-samp);
        }
    }
    return ss.str();
}
