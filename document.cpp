#include "document.h"

void Document::InitCells()
{
    // title
    TitleCell* title = new TitleCell();
    title->SetLocation(0, 0);
    title->SetTitle(&title_);
    cells_.push_back(title);

    // staves
    assert(staves_.size() == 10);
    auto&& staff = staves_.begin();
    for(size_t i = 0; i < 10; ++i, ++staff) { // 68 cells per line
        size_t x = 0;
        StaffName* name = new StaffName();
        name->SetLocation(i, x);
        name->SetStaff(&*staff);
        cells_.push_back(name);
        x += name->FieldSize();

        StaffType* type = new StaffType();
        type->SetLocation(i, x);
        type->SetStaff(&*staff);
        cells_.push_back(type);
        x += type->FieldSize();

        StaffScale* scale = new StaffScale();
        scale->SetLocation(i, x);
        scale->SetStaff(&*staff);
        cells_.push_back(scale);
        x += scale->FieldSize();

        StaffInterpolation* interpolation = new StaffInterpolation();
        interpolation->SetLocation(i, x);
        interpolation->SetStaff(&*staff);
        cells_.push_back(interpolation);
        x += scale->FieldSize();

        // first round of setting links
        name->SetRight(type);
        type->SetLeft(name);
        type->SetRight(scale);
        scale->SetLeft(type);
        scale->SetRight(interpolation);
        interpolation->SetLeft(scale);
        if(i == 0) {
            name->SetTop(title);
            type->SetTop(title);
            scale->SetTop(title);
            interpolation->SetTop(title);
        } else {
            auto nameTop = cells_[cells_.size() - 1 - 3 - 68];
            auto typeTop = cells_[cells_.size() - 1 - 2 - 68];
            auto scaleTop = cells_[cells_.size() - 1 - 1 - 68];
            auto interpolationTop = cells_[cells_.size() - 1 - 0 - 68];
            name->SetTop(nameTop);
            nameTop->SetBottom(name);
            type->SetTop(typeTop);
            typeTop->SetBottom(type);
            scale->SetTop(scaleTop);
            scaleTop->SetBottom(scale);
            interpolation->SetTop(interpolationTop);
            interpolationTop->SetBottom(interpolation);
        }

        for(size_t k = 0; k < 64; ++k) {
            NoteCell* cell = new NoteCell();
            cell->SetLocation(i, x);
            cells_.push_back(cell);
            x += cell->FieldSize();

            if(k == 0) {
                cell->SetLeft(interpolation);
                interpolation->SetRight(cell);
                if(i == 0) {
                    cell->SetTop(title);
                    title->SetBottom(cell);
                } else {
                    ICell* top = cells_[cells_.size() - 1 - 68];
                    top->SetBottom(cell);
                    cell->SetTop(top);
                }
            } else {
                cell->SetLeft(cells_.back());
                cells_.back()->SetRight(cell);
                if(i == 0) {
                    cell->SetTop(title);
                } else {
                    ICell* top = cells_[cells_.size() - 1 - 68];
                    top->SetBottom(cell);
                    cell->SetTop(top);
                }
            }

        }
    }

    Scroll(0);
}

void Scroll(size_t col)
{
    // 1. check note lengths up to col; keep color flipping state
    auto&& staff = staves_.begin();
    for(size_t i = 0; i < 10; ++i, ++staff) {
        size_t ccol = 0;
        int color = 0;
        Staff& s = *staff;
        for(auto&& n = s.notes_.begin(); n != s.notes_.end(); ++n) {
            if(ccol + n->Scale() <= col) {
                color = !color;
                ccol += n->Scale();
            } else {
                break;
            }
        }
        activeNotes_[i] = n;
        if(ccol != col) {
            int diff = col - ccol;
            ccol += n->Scale();
            for(size_t i = 0; i < diff; ++i) {
                NoteCell* cell = dynamic_cast<NoteCell*>(cells_[1 + 68 * i + 4]);
                // wouldn't it be better to generate the note cells with their correct width or something? no!
                if(activeRow_ == i
                        && activeCol_ >= 4 && activeCol_ <= 4 + diff)
                {
                    // what about selected? I'm starting to see a problem here
                    cell->SetColor(color_t::GOLD);
                } else if(n->Type() == "N") {
                    cell->SetColor((color) ? color_t::WHITE : color_t::YELLOW);
                } else {
                    cell->SetColor((color) ? color_t::SKY : color_t::SEA);
                }
                // this is getting a bit more complicated than it should be;
                // TODO FIXME fix this; rethink this; do something!
            }
        }
    }
    // 2. determine where we are; assign color and little text to cells
    // 3. update scroll_, activeNotes_
    // 4. update EditedText
}
