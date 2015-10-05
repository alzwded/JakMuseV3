#ifndef ICELL_H
#define ICELL_H

#include "common.h"

#include <tuple>

struct point_t {
    int x, y;

    point_t(int X, int Y) : x(X), y(Y) {}
};

class ICell
{
public:
    virtual ~ICell() {}
    point_t Select() = 0;
    point_t Mark() = 0;
    cell_t GetRenderThing() = 0;

    std::string Text() = 0;
    void UserInput(std::string) = 0;

    point_t Location() = 0;
    size_t Width() = 0;

    point_t Left() = 0;
    point_t Right() = 0;
    point_t Top() = 0;
    point_t Bottom() = 0;
};

#endif
