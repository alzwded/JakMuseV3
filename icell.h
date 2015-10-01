#ifndef ICELL_H
#define ICELL_H

class ICell
{
public:
    virtual ~ICell() {}
    ICell* Up() =0; // the cell above
    ICell* Down() =0; // the cell below
    ICell* Left() =0; // the cell to the left
    ICell* Right() =0; // the cell to the right
    size_t FieldSize() =0; // get the field's size for rendering
    std::string Text() =0; // get the text for rendering
    color_t Color() =0;
    void Backspace() =0; // attempt to backspace
    void Type(char) =0; // attempt to type in a character
    bool AllowsSelect() =0; // headers on the right don't allow select
    void Mark() =0; // selects this cell
};

#endif
