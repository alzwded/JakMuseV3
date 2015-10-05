#include "document.h"

static bool computedSize = false;
static size_t numThingsPerLine = 0;

ICell* Document::Cell(point_t p)
{
    auto&& found = std::find_if(cells_.begin(), cells_.end(),
            [](decltype(cells_)::const_reference c) -> bool {
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
        StaffNameCell* name = new StaffNameCell(*this, i);
        name->SetLocation(point_t(i + 1, x));
        cells_.push_back(name);
        x += name->Width();

        StaffTypeCell* type = new StaffTypeCell(*this, i);
        type->SetLocation(point_t(i + 1, x));
        cells_.push_back(type);
        x += type->Width();
        
        StaffScaleCell* scale = new StaffScaleCell(*this, i);
        scale->SetLocation(point_t(i + 1, x));
        cells_.push_back(scale);
        x += scale->Width();

        StaffInterpolationCell* interpolation = new StaffInterpolationCell(*this, i);
        interpolation->SetLocation(point_t(i + 1, x));
        cells_.push_back(interpolation);
        x += interpolation->Width();

        if(!computedSize) numThingsPerLine = 4;

        while(x < COLUMNS * N) {
            NoteCell* note = new NoteCell(*this, i, 0);
            note->SetLocation(point_t(i + 1, x));
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
        for(int j = 0; j < staves_[i].notes_.size(); ++j) {
            for(int k = 0; k < staves_[i].notes_[j].scale) {
                cache_[i].push_back(j);
            }
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

void NoteCell::UserInput(std::string text)
{
    if(noteIdx_ < 0) return;

    Note& c = doc_.staves_[staffIdx_].notes_[noteIdx_];
    if(text.empty()) {
        c.scale_ = "1";
        c.name_ = '-';
        c.sharp_ = ' ';
        c.height_ = ' ';
    }
    bool valid = true;
    // expecting a number
    std::string number;
    for(size_t i = 0; i < text.size(); ++i) {
        if(isdigit(text[i])) {
            number.append(text[i]);
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
    text[0] = text.substr(1);
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
