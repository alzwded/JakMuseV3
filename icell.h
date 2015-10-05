#ifndef ICELL_H
#define ICELL_H

#include "common.h"

#include <tuple>
#include <string>

class ICell
{
public:
    virtual ~ICell() {}
    virtual point_t Select() = 0;
    virtual point_t Mark() = 0;
    virtual cell_t GetRenderThing() = 0;

    virtual std::string Text() = 0;
    virtual void UserInput(std::string) = 0;

    virtual point_t Location() = 0;
    virtual size_t Width() = 0;

    virtual point_t Left() = 0;
    virtual point_t Right() = 0;
    virtual point_t Top() = 0;
    virtual point_t Bottom() = 0;
};

#endif
