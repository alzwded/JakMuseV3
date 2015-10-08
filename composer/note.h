#ifndef NOTE_H
#define NOTE_H

#include <string>

struct Note
{
    int scale_;
    char name_;
    char sharp_;
    char height_;

    std::string BuildString(char type);
};

#endif
