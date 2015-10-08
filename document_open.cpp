#include "document.h"
#include "parser.h"
#include "parser_types.h"
#include <istream>
#include <algorithm>
#include <cstdio>

extern bool GetNextToken(std::istream&, int&, char*&);
extern void* ParseAlloc(void* (*)(size_t));
extern void Parse(void*, int, char*, struct PpStaffList**);
extern void ParseFree(void*, void (*)(void*));
extern void ParseTrace(FILE*, char*);

static void PpValue_free(PpValue v)
{
    switch(v.type) {
    case PpValue::PpSTRING:
        free(v.str);
        break;
    case PpValue::PpLIST:
        {
            PpValueList* p = v.list;
            while(p) {
                PpValue_free(p->value);
                PpValueList* prev = p;
                p = p->next;
                free(prev);
            }
        }
        break;
    default:
        break;
    }
}

static void AssignParam(Staff& s, std::string const& key, PpValue value)
{
    switch(s.type_) {
    case 'N':
        {
            if(key.compare("Divisor") == 0) {
                if(value.type != PpValue::PpNUMBER) {
                    fprintf(stderr, "For staff %s, param %s, expected a NUMBER\n", s.name_.c_str(), key.c_str());
                    return;
                }
                s.scale_ = value.num;
            } else if(key.compare("Notes") == 0) {
                if(value.type != PpValue::PpLIST) {
                    fprintf(stderr, "For staff %s, param %s, expected a LIST of NOTEs\n", s.name_.c_str(), key.c_str());
                    return;
                }

                size_t idx = 0;
                PpValueList* p = value.list;
                while(p) {
                    PpValue v = p->value;

                    if(v.type != PpValue::PpNOTE) {
                        fprintf(stderr, "For staff %s, param %s, expected a LIST of NOTEs (at index=%d)\n", s.name_.c_str(), key.c_str(), idx);
                        return;
                    }

                    s.notes_.push_back(v.note);

                    ++idx;
                    p = p->next;
                }
            } else {
                fprintf(stderr, "For staff %s, invalid param %s\n", s.name_.c_str(), key.c_str());
                return;
            }
        }
        break;
    case 'P':
        {
            if(key.compare("Interpolation") == 0) {
                if(value.type != PpValue::PpSTRING) {
                    fprintf(stderr, "For staff %s, param %s, expected a STRING\n", s.name_.c_str(), key.c_str());
                    return;
                }
                std::string interpolation;
                interpolation.assign(value.str);
                if(interpolation.compare("Trunc") == 0) {
                    s.interpolation_ = 'T';
                } else if(interpolation.compare("Cosine") == 0) {
                    s.interpolation_ = 'C';
                } else if(interpolation.compare("Linear") == 0) {
                    s.interpolation_ = 'L';
                } else {
                    fprintf(stderr, "For staff %s, param %s, expected a Trunc, Cosine or Linear; got %s\n", s.name_.c_str(), key.c_str(), interpolation.c_str());
                    return;
                }
            } else if(key.compare("Stride") == 0) {
                if(value.type != PpValue::PpNUMBER) {
                    fprintf(stderr, "For staff %s, param %s, expected a NUMBER\n", s.name_.c_str(), key.c_str());
                    return;
                }
                s.scale_ = value.num;
            } else if(key.compare("Samples") == 0) {
                if(value.type != PpValue::PpLIST) {
                    fprintf(stderr, "For staff %s, param %s, expected a LIST of NUMBERs\n", s.name_.c_str(), key.c_str());
                    return;
                }

                size_t idx = 0;
                PpValueList* p = value.list;
                Note n;
                while(p) {
                    PpValue v = p->value;

                    if(v.type != PpValue::PpNUMBER) {
                        fprintf(stderr, "For staff %s, param %s, expected a LIST of NOTEs (at index=%d)\n", s.name_.c_str(), key.c_str(), idx);
                        return;
                    }

                    if(idx % 2 == 0) {
                        n.scale_ = v.num;
                    } else {
                        if(v.num > 255) {
                            fprintf(stderr, "For staff %s, param %s, even numbered samples must be 0..255 (at index=%d)\n", s.name_.c_str(), key.c_str(), idx);
                            return;
                        } else {
                            char tnum[4];
                            tnum[3] = '\0';
                            sprintf(tnum, "%03u", v.num);
                            n.name_ = '0' + tnum[0];
                            n.height_ = '0' + tnum[1];
                            n.sharp_ = '0' + tnum[2];
                            s.notes_.push_back(n);
                        }
                    }

                    ++idx;
                    p = p->next;
                }
            } else {
                fprintf(stderr, "For staff %s, invalid param %s\n", s.name_.c_str(), key.c_str());
                return;
            }
        }
        break;
    }
}

void Document::Open(std::istream& fin)
{
    void* pParser;
    char* sToken;
    int hTokenId;
    PpStaffList* fileHead;

    pParser = ParseAlloc( malloc );
    ParseTrace(stderr, "Parser:   ");
    while( GetNextToken(fin, hTokenId, sToken) ) {
        Parse(pParser, hTokenId, sToken, &fileHead);
    }
    Parse(pParser, 0, sToken, &fileHead);
    ParseFree(pParser, free);

    if(!fileHead) return;

    std::for_each(staves_.begin(), staves_.end(), [](Staff& s) {
                s.name_ = "";
                s.type_ = 'N';
                s.interpolation_ = 'T';
                s.scale_ = 48;
                s.notes_.clear();
            });

    int staffIdx = -1;

    while(fileHead) {
        PpStaff ppstaff = fileHead->value;
        std::string name;
        name.assign(ppstaff.name);
        free(ppstaff.name);

        decltype(staves_)::iterator it = staves_.end();
        auto&& found = std::find_if(staves_.begin(), staves_.end(),
                [&name](Staff& s) -> bool {
                    return s.name_.compare(name) == 0;
                });
        Staff& staff = (found != staves_.end()) ? *found : staves_[++staffIdx];
        printf("idx: %d\n", staffIdx);
        printf("  %s\n", name.c_str());

        staff.name_ = name;
        staff.type_ = ppstaff.type;
        staff.scale_ = 48; // fixed later if we have our params

        PpParamList* p = ppstaff.params;
        while(p) {
            PpParam kv = p->value;

            std::string key;
            key.assign(kv.key);
            free(kv.key);

            PpValue ppvalue = kv.value;

            AssignParam(staff, key, ppvalue);
            PpValue_free(ppvalue);

            PpParamList* prev = p;
            p = p->next;
            free(prev);
        }

        PpStaffList* prev = fileHead;
        fileHead = fileHead->next;
        free(prev);
    }

    marked_ = point_t(0, 0);
    selected_ = point_t(0, 0);
    active_ = point_t(13 * N, 1);
    UpdateCache();
    Scroll(0);
}
