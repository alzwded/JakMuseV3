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

#ifndef DOCUMENT_H
#define DOCUMENT_H

#include "common.h"
#include "note.h"
#include <vector>
#include <list>
#include <sstream>
#include <deque>
#include <functional>

class ICell;

enum class InsertMode_t
{
    INSERT,
    APPEND,
    REPLACE
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

    struct BufferOp
    {
        std::function<void(void)> cut;
        std::vector<decltype(Staff::notes_)> buffer;
    };

    BufferOp PreCutSelection();

    void Copy();
    void Cut();
    void Paste();
    void Delete();
    void NewNote();
    InsertMode_t InsertMode() { return insertMode_; }
    void SetInsertMode(InsertMode_t im) { insertMode_ = im; }

    ICell* Active() { return Cell(active_); }
    void SetActive(ICell* c);
    void SetActiveToMarked();
    void SetMarked(ICell* c);
    void SetSelected(ICell* c);
    void ClearSelection();
    bool GetSelectionBox(int&, int&, int&, int&);
    bool IsNoteSelected(ICell* note);
    bool IsNoteSelected(int staffIdx, int noteIdx);

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

    decltype(BufferOp::buffer) buffer_;

    void PushState();
    void PopState();
    std::deque<std::vector<Staff>> undoStates_;
};

#endif
