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

#include "parser.h"
#include "note.h"

#include <string>
#include <algorithm>
#include <cctype>
#include <istream>
#include <sstream>

int tokenizer_lineno = 1;

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
    static const char noteNames[] = "ABCDEFGHabcdefgh-";
    if(!strchr(noteNames, text[0])) {
        return false;
    }
    char noteName = toupper(text[0]);
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

    if(atoi(number.c_str()) <= 0) return false;

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

#if 0
#define LOG(F, ...) fprintf(stderr, "Tokenizer: " F "\n", __VA_ARGS__)
#else
#define LOG(F, ...)
#endif

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

        if(c == ';') {
            LOG("Skipping comment until EOL");
            (void) fin.get();
            while(wc = fin.get(), wc != 10 && wc != 13 && wc != EOF)
                ;
            tokenizer_lineno++;
            continue;
        }

        if(isspace(c) || isblank(c) || c == 10 || c == 13 || c == ',') {
            if(text.str().empty()) {
                if(c == 10) {
                    tokenizer_lineno++;
                }
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
                    return (c >= '0' && c <= '9') || c == '-';
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
