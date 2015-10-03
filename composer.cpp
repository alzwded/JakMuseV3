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

int windowW, windowH, window;
//#define CANVAS_W (8066.52f)
//#define CANVAS_H (1828.56f)

#define N (2.f)
#define GLYPH_W_REAL (104.76f)
#define GLYPH_H_REAL (152.38f)
#define GLYPH_W (GLYPH_W_REAL * N)
#define GLYPH_H (GLYPH_H_REAL * N)
#define GLYPH_BASELINE (119.05f * N)
//#define COLUMNS (77)
#define COLUMNS (73)
#define ROWS (12)
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

//#include "document.h"
#include "common.h"



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

    switch(state) {
    case GLUT_UP:
        //if(onmouseup) onmouseup(x, y, btn);
        break;
    case GLUT_DOWN:
        //if(onmousedown) onmousedown(x, y, btn);
        break;
    }
}

static void handleKeyRelease(unsigned char key, int x, int y)
{
    //if(onkeyup) onkeyup(key);
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
        glColor3f(.9f, .9f, .7f);
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
        glColor3f(0.5f, 0.5f, 0.f);
        break;
    case color_t::GRAY:
        glColor3f(0.4f, 0.4f, .4f);
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

void DrawGraphical(int i, int j, int offset, color_t bg, int baseHeight, int multHeight)
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

static void drawSomething()
{
    // text experiment
    for(int i = 0; i < 12; ++i) {
        for(int j = 0; j < COLUMNS; ++j) {
        }
    }

    for(int i = 0; i < 12; ++i) {
        for(int j = 0; j < COLUMNS; ++j) {
            color_t f, b;
            switch(((i * COLUMNS + j) / (j + 1)) % 8)
            {
            case 0: f = b = color_t::WHITE; break;
            case 1: f = b = color_t::BLACK; break;
            case 2: f = b = color_t::YELLOW; break;
            case 3: f = b = color_t::BLUE; break;
            case 4: f = b = color_t::SKY; break;
            case 5: f = b = color_t::GOLD; break;
            case 6: f = b = color_t::SEA; break;
            case 7: f = b = color_t::GRAY; break;
            }
            DrawCharacter(i, j, b, j%10 + '0');
        }
    }

    for(int i = 1; i < 11; ++i) {
        for(int j = 13; j < COLUMNS; ++j) {
            for(int o = 0; o < N; ++o) {
                color_t f, b;
                switch((j*(int)N+o)%2)
                {
                case 0: f = b = color_t::WHITE; break;
                case 1: f = b = color_t::YELLOW; break;
                }
                static const char sharps[] = { ' ', '#', 'b' };

                DrawNote(i, j, o, b, (j*(int)N+o)%2==0, '1' + (j % 6), sharps[(j*(int)N+o) % 3], 'A' + (j % 8));
            }
        }
    }

    for(int i = 5; i < 11; ++i) {
        for(int j = 13; j < COLUMNS; ++j) {
            for(int o = 0; o < N; ++o) {
                unsigned n = (j*(int)N+o);
                color_t f, b;
                switch(n%2)
                {
                case 0: f = b = color_t::WHITE; break;
                case 1: f = b = color_t::YELLOW; break;
                }
                DrawGraphical(i, j, o, b, n%12, (j%6) + '1');
            }
        }
    }
    DrawGraphical(9, 47, 0, color_t::GRAY, -1, ' ');
}

static void update(int value)
{
    // NOP

    glutPostRedisplay();
    glutTimerFunc(17, update, 0);
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
    windowH = 480;
    glutInitWindowSize(windowW, windowH);
    window = glutCreateWindow("Composer");

	glViewport(0, 0, windowW, windowH);

    glutKeyboardFunc(handleKeyPress);
    glutKeyboardUpFunc(handleKeyRelease);
    glutMouseFunc(handleMouse);
    glutReshapeFunc(handleResize);

    //glDisable(GL_CULL_FACE);
    //glDisable(GL_DEPTH_TEST);


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
