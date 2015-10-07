#include <GL/freeglut.h>
#include <cstdio>

#ifndef WIN32_LEAN_AND_MEAN
# define WIN32_LEAN_AND_MEAN
#endif
#ifndef WIN32_LEAN_AND_MEAN
# define VC_EXTRALEAN
#endif
#ifndef NOMINMAX
# define NOMINMAX
#endif
#include <windows.h>
#include <ShellScalingApi.h>
#include <assert.h>
#include <algorithm>

int windowW, windowH, window;
//#define CANVAS_W (8066.52f)
//#define CANVAS_H (1828.56f)

#include "common.h"

#define GLYPH_W_REAL (104.76f)
#define GLYPH_H_REAL (152.38f)
#define GLYPH_W (GLYPH_W_REAL * N)
#define GLYPH_H (GLYPH_H_REAL * N)
//#define GLYPH_BASELINE (119.05f * N)
#define GLYPH_BASELINE (117.f * N)
//#define COLUMNS (77)
#ifdef WEIRD_WINDOWS
#define CANVAS_W (GLYPH_W * (COLUMNS + 1))
#define CANVAS_H (GLYPH_H * (ROWS + 1))
const float OFFSET_X = GLYPH_W/2.f;
const float OFFSET_Y = GLYPH_H/2.f;
#else
#define CANVAS_W (GLYPH_W * (COLUMNS))
#define CANVAS_H (GLYPH_H * (ROWS))
const float OFFSET_X = 0.f;
const float OFFSET_Y = 0.f;
#endif

#include <string>
#include <list>
#include <tuple>
#include <fstream>

#include "document.h"
#include "icell.h"
#include "document_display.h"

static Document doc;
static enum { GFX, TXT } mode_ = GFX;
static enum { EDITING, SAVING, OPENING } inputMode_ = EDITING;
static std::string currentText;
static bool modified = false;

static void TextStart()
{
    ICell* c = doc.Active();
    if(c) currentText = c->Text();
    else currentText = "";
    modified = false;
    inputMode_ = EDITING;
}

static void TextType(char c)
{
    if(modified) currentText.append(&c, 1);
    else currentText = std::string(&c, 1);
    modified = true;
}

static void TextBackspace()
{
    if(currentText.empty()) return;
    currentText = currentText.substr(0, currentText.size() - 1);
    modified = true;
}

static void TextValidate()
{
    ICell* c = doc.Active();
    if(c) {
        c->UserInput(currentText);
        currentText = c->Text();
        doc.UpdateCache();
        doc.Scroll(doc.scroll_);
    } else {
        currentText = "";
    }
    modified = false;
}

static void handleResize(int w, int h)
{
	glViewport(0, 0, w, h);

    windowW = w;
    windowH = h;
}

static void handleMouse(int button, int state, int X, int Y)
{
    int btn = 0;
    switch(button) {
    case GLUT_LEFT_BUTTON:
        btn = 1;
        break;
    case GLUT_MIDDLE_BUTTON:
        btn = 2;
        break;
    case GLUT_RIGHT_BUTTON:
        btn = 3;
        break;
    }

    int x = 0, y = 0;
    x = (int)((float)X / windowW * CANVAS_W);
    y = (int)((float)Y / windowH * CANVAS_H);
    x = (x * N / GLYPH_W);
    y = (y / GLYPH_H);

    switch(state) {
    case GLUT_UP:
        switch(button) {
        case GLUT_LEFT_BUTTON:
            doc.Cell(x, y)->Mark();
            doc.Cell(x, y)->Select();
            glutPostRedisplay();
            break;
        case GLUT_RIGHT_BUTTON:
            doc.Cell(x, y)->Select();
            glutPostRedisplay();
            break;
        case GLUT_MIDDLE_BUTTON:
            // document.Copy();
            // document.cells(x, y).Paste();
            break;
        }
        break;
    case GLUT_DOWN:
        break;
    }
}

static void handleSpecialPress(int key, int x, int y)
{
}

typedef point_t(ICell::*mfunc_t)(void);
static void MovementFunc(mfunc_t mfunc, bool select)
{
    ICell* c = doc.Cell((doc.Active()->*mfunc)());
    if(!select) c = doc.Cell(c->Mark());
    c = doc.Cell(c->Select());
}

static void handleSpecialRelease(int key, int x, int y)
{
#define PLOC(P) printf("%d %d\n", P.x, P.y)
    int modifiers = 0;
    int mask = 0;
    mfunc_t mfunc = nullptr;
    printf("%2X %2X p\n", key, modifiers);
    switch(key) {
    case GLUT_KEY_LEFT:
        modifiers = glutGetModifiers();
        mfunc = &ICell::Left;
        if(mask = GLUT_ACTIVE_CTRL|GLUT_ACTIVE_SHIFT,
                (modifiers & mask) == mask)
        {
            doc.ScrollLeft(false);
            doc.Active()->Select();
        } else if(modifiers & GLUT_ACTIVE_CTRL) {
            doc.ScrollLeft(false);
        } else {
            MovementFunc(mfunc, modifiers & GLUT_ACTIVE_SHIFT);
        }
        TextStart();
        glutPostRedisplay();
        break;
    case GLUT_KEY_RIGHT:
        modifiers = glutGetModifiers();
        mfunc = &ICell::Right;
        if(mask = GLUT_ACTIVE_CTRL|GLUT_ACTIVE_SHIFT,
                (modifiers & mask) == mask)
        {
            doc.ScrollRight(false);
            doc.Active()->Select();
        } else if(modifiers & GLUT_ACTIVE_CTRL) {
            doc.ScrollRight(false);
        } else {
            MovementFunc(mfunc, modifiers & GLUT_ACTIVE_SHIFT);
        }
        TextStart();
        glutPostRedisplay();
        break;
    case GLUT_KEY_UP:
        modifiers = glutGetModifiers();
        mfunc = &ICell::Top;
        if(mask = GLUT_ACTIVE_CTRL|GLUT_ACTIVE_SHIFT,
                (modifiers & mask) == mask)
        {
        } else if(modifiers & GLUT_ACTIVE_CTRL) {
        } else {
            MovementFunc(mfunc, modifiers & GLUT_ACTIVE_SHIFT);
        }
        TextStart();
        glutPostRedisplay();
        break;
    case GLUT_KEY_DOWN:
        modifiers = glutGetModifiers();
        mfunc = &ICell::Bottom;
        if(mask = GLUT_ACTIVE_CTRL|GLUT_ACTIVE_SHIFT,
                (modifiers & mask) == mask)
        {
        } else if(modifiers & GLUT_ACTIVE_CTRL) {
        } else {
            MovementFunc(mfunc, modifiers & GLUT_ACTIVE_SHIFT);
        }
        TextStart();
        glutPostRedisplay();
        break;
    case GLUT_KEY_PAGE_UP:
        modifiers = glutGetModifiers();
        if(modifiers & GLUT_ACTIVE_SHIFT) {
            doc.ScrollLeft(true);
            doc.Active()->Select();
        } else {
            doc.ScrollLeft(true);
        }
        TextStart();
        glutPostRedisplay();
        break;
    case GLUT_KEY_PAGE_DOWN:
        modifiers = glutGetModifiers();
        if(modifiers & GLUT_ACTIVE_SHIFT) {
            doc.ScrollRight(true);
            doc.Active()->Select();
        } else {
            doc.ScrollRight(true);
        }
        TextStart();
        glutPostRedisplay();
        break;
    // idem right, idem pg up, idem pg dwn; similar up, similar down w/o ctrl
    case GLUT_KEY_HOME:
        doc.scroll_ = 0;
        doc.ScrollLeftRight(0);
        glutPostRedisplay();
        break;
    case GLUT_KEY_END:
        doc.scroll_ = doc.Max();
        doc.ScrollLeftRight(0);
        glutPostRedisplay();
        break;
    case GLUT_KEY_F2:
        inputMode_ = SAVING;
        currentText = doc.title_;
        modified = true;
        glutPostRedisplay();
        break;
    case GLUT_KEY_F3:
        inputMode_ = OPENING;
        currentText = doc.title_;
        modified = true;
        break;
    case GLUT_KEY_F10:
        // TODO restore undo mark
        break;
    case GLUT_KEY_F12:
        mode_ = (mode_ == GFX) ? TXT : GFX;
        glutPostRedisplay();
        break;
    }
#undef PLOC
}

static void handleKeyRelease(unsigned char key, int x, int y)
{
    int modifiers = glutGetModifiers();
    switch(key) {
    case 27:
        TextStart();
        glutPostRedisplay();
        break;
    case 10:
    case 13:
        // TODO save undo point
        switch(inputMode_) {
        case EDITING:
            TextValidate();
            break;
        case OPENING:
            {
                std::fstream f(currentText, std::ios::in);
                if(f.good()) {
                    doc.Open(f);
                    doc.title_ = currentText;
                }
                inputMode_ = EDITING;
                TextStart();
                break;
            }
        case SAVING:
            {
                std::fstream f(currentText, std::ios::out);
                if(f.good()) {
                    doc.Save(f);
                    doc.title_ = currentText;
                }
                inputMode_ = EDITING;
                TextStart();
                break;
            }
        }
        glutPostRedisplay();
        break;
    case 127:
        // TODO save undo mark
        // TODO check inputMode_ everywhere
        doc.Delete();
        glutPostRedisplay();
        TextStart();
        break;
    case 9: // tab
        switch(doc.insertMode_)
        {
        case InsertMode_t::INSERT:
            doc.insertMode_ = InsertMode_t::APPEND;
            break;
        case InsertMode_t::APPEND:
            doc.insertMode_ = InsertMode_t::REPLACE;
            break;
        case InsertMode_t::REPLACE:
            doc.insertMode_ = InsertMode_t::INSERT;
            break;
        }
        glutPostRedisplay();
        break;
    case 8: // bkspc
        TextBackspace();
        glutPostRedisplay();
        break;
    default:
        if(isgraph(key) || isprint(key) || isspace(key)) {
            TextType(key);
            glutPostRedisplay();
        }
        break;
    }

}

static void handleKeyPress(unsigned char key, int x, int y)
{
    //if(onkeydown) onkeydown(key);
}

void SetBackgroundColor(color_t c)
{
    switch(c)
    {
    case color_t::BLACK:
        glColor3f(0.f, 0.f, 0.f);
        break;
    case color_t::WHITE:
        glColor3f(1.f, 1.f, 1.f);
        break;
    case color_t::YELLOW:
        glColor3f(.96f, .96f, .77f);
        break;
    case color_t::SKY:
        glColor3f(0.9f, 0.95f, 1.f);
        break;
    case color_t::SEA:
        glColor3f(0.6f, .9f, .9f);
        break;
    case color_t::BLUE:
        glColor3f(0.3f, 0.3f, 1.f);
        break;
    case color_t::GOLD:
        // TODO rename CRIMSON to GOLD
        glColor3f(0.3f, 0.f, 0.f);
        break;
    case color_t::GRAY:
        glColor3f(0.4f, 0.4f, .4f);
        break;
    case color_t::CRIMSON:
        // TODO rename CRIMSON to GOLD
        glColor3f(0.5f, 0.5f, 0.f);
        break;
    }
}

void SetForegroundColor(color_t c)
{
    switch(c)
    {
    case color_t::BLACK:
        glColor3f(1.f, 1.f, 1.f);
        break;
    case color_t::WHITE:
        glColor3f(0.f, 0.f, 0.f);
        break;
    case color_t::YELLOW:
        glColor3f(0.f, 0.f, 0.f);
        break;
    case color_t::SKY:
        glColor3f(0.f, 0.f, 0.f);
        break;
    case color_t::SEA:
        glColor3f(0.f, 0.f, 0.f);
        break;
    case color_t::BLUE:
        glColor3f(1.f, 1.f, 1.f);
        break;
    case color_t::GOLD:
        glColor3f(1.f, 1.f, 1.f);
        break;
    case color_t::GRAY:
        glColor3f(1.f, 1.f, 1.f);
        break;
    case color_t::CRIMSON:
        glColor3f(1.f, 1.f, 1.f);
        break;
    }
}

#define PIXEL_WIDTH (GLYPH_W/12.f)
#define PIXEL_HEIGHT (GLYPH_H/24.f)
#define PIXEL(X, Y) do{ \
    float x = (X), y = (Y); \
    glVertex2f(j * GLYPH_W + (x + 1) * PIXEL_WIDTH, i * GLYPH_H + OFFSET_Y + y * PIXEL_HEIGHT); \
    glVertex2f(j * GLYPH_W + x * PIXEL_WIDTH, i * GLYPH_H + OFFSET_Y + y * PIXEL_HEIGHT); \
    glVertex2f(j * GLYPH_W + x * PIXEL_WIDTH, i * GLYPH_H + OFFSET_Y + (y + 1) * PIXEL_HEIGHT); \
    glVertex2f(j * GLYPH_W + (x + 1) * PIXEL_WIDTH, i * GLYPH_H + OFFSET_Y + (y + 1) * PIXEL_HEIGHT); \
}while(0)

#define K (12.f/4.f)
#define S (1.f/N)

void DrawGraphical(int i, int j, int offset, color_t bg, int baseHeight, char multHeight)
{
    assert(offset >= 0 && offset < N);
    float realOffset = S * offset;
    float realOffsetPlusOne = S * (offset + 1);
    glBegin(GL_QUADS); {
        SetBackgroundColor(bg);
        glVertex2f((j + realOffsetPlusOne) * GLYPH_W, i * GLYPH_H + OFFSET_Y);
        glVertex2f((j + realOffset) * GLYPH_W, i * GLYPH_H + OFFSET_Y);
        glVertex2f((j + realOffset) * GLYPH_W, (i + 1) * GLYPH_H + OFFSET_Y);
        glVertex2f((j + realOffsetPlusOne) * GLYPH_W, (i + 1) * GLYPH_H + OFFSET_Y);
    } glEnd();

    if(baseHeight >= 0 && baseHeight < 12) {
        glBegin(GL_QUADS); {
            ++i;
            SetForegroundColor(bg);
            glVertex2f((j + realOffsetPlusOne) * GLYPH_W, i * GLYPH_H + OFFSET_Y - baseHeight/12.f * GLYPH_H/12.f*9.f - GLYPH_H/12.f*3.f);
            glVertex2f((j + realOffset) * GLYPH_W, i * GLYPH_H + OFFSET_Y - baseHeight/12.f * GLYPH_H/12.f*9.f - GLYPH_H/12.f*3.f);
            glVertex2f((j + realOffset) * GLYPH_W, i * GLYPH_H + OFFSET_Y - (baseHeight+1)/12.f * GLYPH_H/12.f*9.f - GLYPH_H/12.f*3.f);
            glVertex2f((j + realOffsetPlusOne) * GLYPH_W, i * GLYPH_H + OFFSET_Y - (baseHeight+1)/12.f * GLYPH_H/12.f*9.f - GLYPH_H/12.f*3.f);
            --i;
        } glEnd();
    }

    SetForegroundColor(bg);
    int nOff = 20.5f;
    glPushMatrix();
    glLineWidth(0.5);
    glTranslatef((j + offset * S) * GLYPH_W, i * GLYPH_H + GLYPH_BASELINE/23.f*K + nOff * PIXEL_HEIGHT, 0.f);
    glScalef(N/23.f*12.f, -N/23.f*5.f, 0.f);
    glutStrokeCharacter(GLUT_STROKE_MONO_ROMAN, multHeight);
    glPopMatrix();
}

void DrawNote(int i, int j, int offset, color_t bg, bool top, char topNumber, char sharp, char bottomChar)
{
    assert(offset >= 0 && offset < N);
    float realOffset = S * offset;
    float realOffsetPlusOne = S * (offset + 1);
    glBegin(GL_QUADS); {
        SetBackgroundColor(bg);
        glVertex2f((j + realOffsetPlusOne) * GLYPH_W, i * GLYPH_H + OFFSET_Y);
        glVertex2f((j + realOffset) * GLYPH_W, i * GLYPH_H + OFFSET_Y);
        glVertex2f((j + realOffset) * GLYPH_W, (i + 1) * GLYPH_H + OFFSET_Y);
        glVertex2f((j + realOffsetPlusOne) * GLYPH_W, (i + 1) * GLYPH_H + OFFSET_Y);
    } glEnd();

    SetForegroundColor(bg);
    float nOff = (top) ? 6.5f : 15.f;
    glPushMatrix();
    glLineWidth(0.5);
    glTranslatef((j + offset * S) * GLYPH_W, i * GLYPH_H + GLYPH_BASELINE/23.f*K + nOff * PIXEL_HEIGHT, 0.f);
    glScalef(N/23.f*12.f, -N/23.f*5.f, 0.f);
    glutStrokeCharacter(GLUT_STROKE_MONO_ROMAN, topNumber);
    glPopMatrix();


    nOff = 20.5f;
    glPushMatrix();
    glLineWidth(0.5);
    glTranslatef((j + offset * S) * GLYPH_W, i * GLYPH_H + GLYPH_BASELINE/23.f*K + nOff * PIXEL_HEIGHT, 0.f);
    glScalef(N/23.f*12.f, -N/23.f*5.f, 0.f);
    glutStrokeCharacter(GLUT_STROKE_MONO_ROMAN, sharp);
    glPopMatrix();


    nOff = (top) ? 0.5f : 9.f;
    glPushMatrix();
    //glLineWidth(0.5);
    glLineWidth(0.7);
    glTranslatef((j + offset * S) * GLYPH_W, i * GLYPH_H + GLYPH_BASELINE/23.f*5.f + nOff * PIXEL_HEIGHT, 0.f);
    glScalef(N/23.f*12.f, -N/23.f*6.f, 0.f);
    glutStrokeCharacter(GLUT_STROKE_MONO_ROMAN, bottomChar);
    glPopMatrix();

    //glPushMatrix();
    //glLineWidth(0.5);
    //glTranslatef(j * GLYPH_W, i * GLYPH_H + GLYPH_BASELINE + OFFSET_Y, 0.f);
    //glScalef(4.f, -4.f, 0.0f);
    //glutStrokeCharacter(GLUT_STROKE_MONO_ROMAN, c);
    //glPopMatrix();
}

void DrawCharacter(int i, int j, color_t bg, char c)
{
    glBegin(GL_QUADS); {
        SetBackgroundColor(bg);
        glVertex2f((j + 1) * GLYPH_W, i * GLYPH_H + OFFSET_Y);
        glVertex2f(j * GLYPH_W, i * GLYPH_H + OFFSET_Y);
        glVertex2f(j * GLYPH_W, (i + 1) * GLYPH_H + OFFSET_Y);
        glVertex2f((j + 1) * GLYPH_W, (i + 1) * GLYPH_H + OFFSET_Y);
    } glEnd();

    glPushMatrix();
    glLineWidth(0.5);
    glTranslatef(j * GLYPH_W, i * GLYPH_H + GLYPH_BASELINE + OFFSET_Y, 0.f);
    glScalef(N, -N, 0.0f);
    SetForegroundColor(bg);
    glutStrokeCharacter(GLUT_STROKE_MONO_ROMAN, c);
    glPopMatrix();
}

struct gfx {
    char base, mult;
};
static gfx ToGfx(cell_t c)
{
    gfx ret = { 0, c.text[1] };
    switch(c.text[0]) {
    case 'C': ret.base = 0; break;
    case 'D': ret.base = 2; break;
    case 'E': ret.base = 4; break;
    case 'F': ret.base = 5; break;
    case 'G': ret.base = 7; break;
    case 'A': ret.base = 9; break;
    case 'B': ret.base = 10; break;
    case 'H': ret.base = 11; break;
    case '-': ret.base = -63; ret.mult = '0'; return ret; // XXX
    default: ret.base = -64; ret.mult = '0'; return ret; // XXX
    }
    switch(c.text[2]) {
    case ' ':
        break;
    case '#':
        if(ret.base == 11) {
            ret.mult += 1;
            ret.base = 0;
        } else {
            ret.base += 1;
        }
        break;
    case 'b':
        if(ret.base == 0) {
            ret.mult -= 1;
            ret.base = 11;
            assert(ret.mult > 0);
        } else {
            ret.base -= 1;
        }
    }
    return ret;
}

static void DrawCell(cell_t c)
{
    switch(c.type) {
        case cell_t::BLOCK:
            for(size_t i = 0; i < c.ntext; ++i) {
                DrawCharacter(c.y, c.x/N + i, c.color, c.text[i]);
            }
            break;
        case cell_t::NOTE:
            switch(mode_) {
                case GFX: {
                              gfx g = ToGfx(c);
                              if(c.text[4]) DrawGraphical(c.y, c.x/N, c.x%N, c.color, g.base, g.mult);
                              else DrawGraphical(c.y, c.x/N, c.x%N, c.color, g.base, ' ');
                              break; }
                case TXT:
                          if(c.text[4]) DrawNote(c.y, c.x/N, c.x%N, c.color, c.text[3], c.text[1], c.text[2], c.text[0]);
                          else DrawNote(c.y, c.x/N, c.x%N, c.color, c.text[3], ' ', ' ', ' ');
                          break;
            };
            break;
        case cell_t::SAMPLE:
            DrawNote(c.y, c.x/N, c.x%N, c.color, false, c.text[1], c.text[2], c.text[0]);
            break;
    }
}

static void drawSomething()
{
    // main screen
    for(ICell*& cell : doc.cells_) {
        cell_t c = cell->GetRenderThing();
        DrawCell(c);
        free(c.text);
    }

    point_t active = doc.active_;
    auto&& found = std::find_if(doc.cells_.begin(), doc.cells_.end(), [active](ICell* c) -> bool {
                return c->Location().x == active.x && c->Location().y == active.y;
                
            });
    NoteCell* noteCell = dynamic_cast<NoteCell*>(*found);
    if(noteCell) {
        while(!noteCell->First()) {
            noteCell = dynamic_cast<NoteCell*>(doc.Cell(noteCell->Location().x - 1, noteCell->Location().y));
            assert(noteCell);
        }
        assert(noteCell->First());
        point_t pos = noteCell->Location();
        cell_t c = noteCell->GetRenderThing();
        c.color = color_t::GOLD;
        DrawCell(c);
        free(c.text);
        do {
            noteCell = (NoteCell*)doc.Cell(point_t(pos.x + 1, pos.y));
            if(!noteCell) break;
            if(noteCell->First()) break;
            pos = noteCell->Location();
            c = noteCell->GetRenderThing();
            c.color = color_t::GOLD;
            DrawCell(c);
            free(c.text);
        } while(1);
    } else {
        cell_t c = (*found)->GetRenderThing();
        c.color = color_t::GOLD;
        DrawCell(c);
        free(c.text);
    }

    // satus bar
#define QUOTEH(X) #X
#define QUOTE(X) QUOTEH(X)
#define lenText  57
    static const int offInsert = COLUMNS - 16;
    static const int offDuration = COLUMNS - 15;
#define lenDuration 5
    static const int offPosition = COLUMNS - 9;
    static const int lenPosition = 5 + 1 + 2 + 1;

    char text[lenText + 1];
    sprintf(text, "%-" QUOTE(lenText) "s", currentText.substr(0, lenText).c_str());
    text[lenText] = '\0';
    for(size_t i = 0; i < lenText; ++i) {
        DrawCharacter(ROWS - 1, i, (modified) ? color_t::GOLD : color_t::CRIMSON, text[i]); // currentText
    }

    color_t insertcolor = color_t::YELLOW;
    switch(doc.insertMode_) {
    case InsertMode_t::INSERT:
        DrawCharacter(ROWS - 1, offInsert, insertcolor, 'I');
        break;
    case InsertMode_t::APPEND:
        DrawCharacter(ROWS - 1, offInsert, insertcolor, 'A');
        break;
    case InsertMode_t::REPLACE:
        DrawCharacter(ROWS - 1, offInsert, insertcolor, 'R');
    }

    char duration[lenDuration + 1];
    sprintf(duration, "%" QUOTE(lenDuration) "u", doc.Duration()%100000);
    duration[lenDuration] = '\0';
    for(size_t i = 0; i < lenDuration; ++i) {
        DrawCharacter(ROWS - 1, i + offDuration, color_t::CRIMSON, duration[i]);
    }
    DrawCharacter(ROWS - 1, offDuration + lenDuration, color_t::CRIMSON, 's');

    char position[lenPosition + 1];
    if(doc.AtEnd()) {
        sprintf(position, "%5u/END", doc.Position() % 100000);
    } else {
        sprintf(position, "%5u/%2u%%", doc.Position() % 100000, doc.Percentage() % 100);
    }
    position[lenPosition] = '\0';
    for(size_t i = 0; i < lenPosition; ++i) {
        DrawCharacter(ROWS - 1, offPosition + i, insertcolor, position[i]);
    }
    //DrawCharacter(ROWS - 1, offPosition + lenPosition - 1, color_t::WHITE, '%');
}

static void update(int value)
{
    // NOP

    //glutPostRedisplay();
    //glutTimerFunc(35, update, 0);
}

static void draw()
{
    glClearColor(0.f, 0.f, 1.f, 0.f);
    glClear(GL_COLOR_BUFFER_BIT);


    drawSomething();

    glutSwapBuffers();
}

int main(int argc, char* argv[])
{
    SetProcessDPIAware();

    glutInit(&argc, argv);
    glutInitDisplayMode(GLUT_DOUBLE|GLUT_RGBA);
    windowW = 1024;
    //windowH = 480;
    windowH = 720;
    glutInitWindowSize(windowW, windowH);
    window = glutCreateWindow("Composer");

	glViewport(0, 0, windowW, windowH);

    glutKeyboardFunc(handleKeyPress);
    glutKeyboardUpFunc(handleKeyRelease);
    glutSpecialFunc(handleSpecialPress);
    glutSpecialUpFunc(handleSpecialRelease);
    glutMouseFunc(handleMouse);
    glutReshapeFunc(handleResize);

    //glDisable(GL_CULL_FACE);
    //glDisable(GL_DEPTH_TEST);

    doc.UpdateCache();
    doc.InitCells();
    if(doc.Active()) currentText = doc.Active()->Text();

    // set up ortho 2d
    glMatrixMode(GL_PROJECTION);
    glLoadIdentity();
    gluOrtho2D(0, CANVAS_W, CANVAS_H, 0);
    glMatrixMode(GL_MODELVIEW);
    glPushMatrix();
    glLoadIdentity();

    glutDisplayFunc(draw);
    glutTimerFunc(17, update, 0);
    glutMainLoop();
}
