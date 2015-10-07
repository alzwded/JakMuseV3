#include "common.h"
#include "document_display.h"
#include "document.h"
#include <algorithm>
#include <numeric>
#include <assert.h>

cell_t TitleCell::GetRenderThing()
{
    cell_t ret;
    ret.x = Location().x;
    ret.y = Location().y;
    ret.type = cell_t::BLOCK;
    ret.color = color_t::CRIMSON;
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
    ret.color = color_t::CRIMSON;
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
    ret.color = color_t::CRIMSON;
    ret.text = (char*)malloc(sizeof(char) * 1);
    ret.ntext = 1;
    ret.text[0] = Text()[0];
    return ret;
}

void StaffInterpolation::UserInput(std::string s)
{
    if(s.empty()) return;
    switch(s[0]) {
    case 'C':
    case 'T':
    case 'L':
        doc_.staves_[staffIdx_].interpolation_ = s[0];
        break;
    }
}

void NoteCell::UserInput(std::string text)
{
    if(noteIdx_ < 0) return;
    Note& c = doc_.staves_[staffIdx_].notes_[noteIdx_];

    if(doc_.staves_[staffIdx_].type_ == 'N') {
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
        if(text.empty() && noteName != '-') return;
        char sharp = ' ';
        if(text[0] == '#' || text[0] == 'b') {
            sharp = text[0];
            text = text.substr(1);
        }
        // expecting a height
        if(text.empty() && noteName != '-') return;
        char height = 0;
        if(noteName != '-') {
            if(isdigit(text[0])) {
                height = text[0];
            }
        }

        c.scale_ = atoi(number.c_str());
        c.name_ = noteName;
        c.sharp_ = sharp;
        c.height_ = height;
    } else {
        std::stringstream s;
        s << text;
        std::string s1, s2;
        s >> s1 >> s2;
        if(s1.empty() || s2.empty()) return;
        int scale = atoi(s1.c_str());
        while(s2.size() < 3) s2 = '0' + s2;
        c.scale_ = scale;
        c.name_ = s2[0];
        c.height_ = s2[1];
        c.sharp_ = s2[2];
    }

    doc_.UpdateCache();
}

cell_t NoteCell::GetRenderThing()
{
    cell_t ret;
    ret.x = Location().x;
    ret.y = Location().y;
    ret.text = (char*)malloc(sizeof(char) * 5);
    ret.ntext = 5;

    if(doc_.staves_[staffIdx_].type_ == 'N') {
        ret.type = cell_t::NOTE;
        if(noteIdx_ >= 0) {
            ret.text[3] = (noteIdx_ % 2) ? '\0' : 'T';
            Note& n = doc_.staves_[staffIdx_].notes_[noteIdx_];
            if(doc_.staves_[staffIdx_].notes_[noteIdx_].name_ == '-') {
                ret.color = color_t::GRAY;
                ret.text[4] = (first_) ? 'T' : '\0';
                ret.text[0] = n.name_;
                ret.text[1] = '0';
                ret.text[2] = ' ';
            } else {
                ret.color = (noteIdx_ % 2) ? color_t::YELLOW : color_t::WHITE;
                if(first_) {
                    ret.text[4] = 'T';
                } else {
                    ret.text[4] = '\0';
                }
                ret.text[0] = n.name_;
                ret.text[1] = n.height_;
                ret.text[2] = n.sharp_;
            }
            if(doc_.IsNoteSelected(this)) {
                ret.color = color_t::BLUE;
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
            if(doc_.IsNoteSelected(this))
            {
                ret.color = color_t::BLUE;
            }
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
