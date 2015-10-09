#include "parser.h"
#include "note.h"

#include <string>
#include <algorithm>
#include <cctype>
#include <istream>
#include <sstream>

bool TryParseNote(const char* s, Note* n)
{
    std::string text;
    text.assign(s);

    if(text.empty()) {
        n->scale_ = 1;
        n->name_ = '-';
        n->sharp_ = ' ';
        n->height_ = ' ';
        return true;
    }

    bool valid = true;
    // expecting a number
    std::string number;
    for(size_t i = 0; i < text.size(); ++i) {
        if(text[i] >= '0' && text[i] <= '9') {
            number.append(std::string(1, text[i]));
        } else {
            break;
        }
    }
    if(number.empty()) { return false; }
    text = text.substr(number.size());
    // expecting a note name
    if(text.empty()) return false;
    static const char noteNames[] = "ABCDEFGH-";
    if(!strchr(noteNames, text[0])) {
        return false;
    }
    char noteName = text[0];
    text = text.substr(1);
    // expecting an optional sharp
    if(text.empty() && noteName != '-') return false;
    char sharp = ' ';
    if(text[0] == '#' || text[0] == 'b') {
        sharp = text[0];
        text = text.substr(1);
    }
    // expecting a height
    if(text.empty() && noteName != '-') return false;
    char height = 0;
    if(noteName != '-') {
        if(text[0] >= '0' && text[0] <= '9') {
            height = text[0];
        }
    }

    n->scale_ = atoi(number.c_str());
    n->name_ = noteName;
    n->sharp_ = sharp;
    n->height_ = height;

    return true;
}

void AssignString(std::string const& s, char*& p)
{
    p = (char*)malloc(sizeof(char) * s.size() + 1);
    strcpy(p, s.c_str());
}

#define LOG(F, ...) fprintf(stderr, "Tokenizer: " F "\n", __VA_ARGS__)

bool GetNextToken(std::istream& fin, int& hTokenId, char*& sToken)
{
    if(!fin.good()) return false;

    char c;
    int wc;
    std::stringstream text;
    while(1) {
        wc = fin.peek();
        if(wc == std::char_traits<char>::eof() || fin.eof()) break;
        c = wc & 0xFF;

        LOG("Considering %c", c);

        if(isspace(c) || isblank(c) || c == 10 || c == 13 || c == ',') {
            if(text.str().empty()) {
                LOG("Skipping whitespace");
                (void) fin.get();
                continue;
            }
            break;
        }

        if(c == '[') {
            if(text.str().empty()) {
                LOG("LSQUARE");
                hTokenId = LSQUARE;
                (void) fin.get();
                return true;
            } else {
                break;
            }
        } else if(c == ']') {
            if(text.str().empty()) {
                LOG("RSQUARE");
                hTokenId = RSQUARE;
                (void) fin.get();
                return true;
            } else {
                break;
            }
        } else if(c == '{') {
            if(text.str().empty()) {
                LOG("LCURLY");
                hTokenId = LCURLY;
                (void) fin.get();
                return true;
            } else {
                break;
            }
        } else if(c == '}') {
            if(text.str().empty()) { // if you notice I overuse stringstream::str too much in a loop, shut up; stringstream doesn't have empty for some reason and I'm too lazy to refactor
                LOG("RCURLY");
                hTokenId = RCURLY;
                (void) fin.get();
                return true;
            } else {
                break;
            }
        } else if(c == '=') {
            if(text.str().empty()) {
                LOG("EQUALS");
                hTokenId = EQUALS;
                (void) fin.get();
                return true;
            } else {
                break;
            }
        }

        (void) fin.get();
        text << c;
        LOG("Eating character; now at %s", text.str().c_str());
    }

    if(text.str().empty()) return false;
    std::string stext = text.str();

    if(stext.compare("NOTES") == 0) {
        LOG("NOTES");
        hTokenId = NOTES;
        return true;
    }

    if(stext.compare("PCM") == 0) {
        LOG("PCM");
        hTokenId = PCM;
        return true;
    }

    Note n;
    if(TryParseNote(stext.c_str(), &n)) {
        LOG("NOTE");
        AssignString(text.str(), sToken);
        hTokenId = NOTE;
        return true;
    }

    if(std::all_of(&stext[0], &stext[0] + stext.size(), [](char c) -> bool {
                    return c >= '0' && c <= '9';
                })) {
        LOG("NUMBER");
        AssignString(text.str(), sToken);
        hTokenId = NUMBER;
        return true;
    }

    AssignString(text.str(), sToken);
    LOG("STRING");
    hTokenId = STRING;
    return true;
}
