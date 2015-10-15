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
    GRAY,       // rests
    CRIMSON     // eugh
};

#define N (2)
#define COLUMNS (73)
//#define ROWS (12)
#define ROWS (18)

struct cell_t {
    size_t x, y;
    enum { BLOCK, NOTE, SAMPLE } type;
    color_t color;
    char* text; // base, mult, sharp or blablabla
    size_t ntext;
};

struct point_t {
    int x, y;

    point_t(int X, int Y) : x(X), y(Y) {}
};

#endif
