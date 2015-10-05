#ifndef COMMON_H
#define COMMON_H

enum class color_t {
    BLACK,      // pipe characters between columns; scale factor
    WHITE,      // alternating with yellow in table for NOTES; channel name; interpolation
    YELLOW,     // alternating with white in table for NOTES
    SKY,        // alternating with sea in table for PCM
    SEA,        // alternating with sky in table for pcm
    BLUE,       // status bar, selection, title
    GOLD,       // active cell
    GRAY        // rests
};

#define N (2)
#define COLUMNS (73)
#define ROWS (12)

struct cell_t {
    size_t x, y;
    enum { BLOCK, SUBBLOCK } type;
    color_t color;
    char* text; // base, mult, sharp or blablabla
    size_t ntext;
};

#endif
