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

int windowW, windowH, window;
//#define CANVAS_W (8066.52f)
//#define CANVAS_H (1828.56f)
#define GLYPH_W (104.76f)
#define GLYPH_H (152.38f)
#define COLUMNS (77)
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

#include "document.h"
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
        glColor3f(1.f, 1.f, 0.9f);
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

void DrawCharacter(int i, int j, color_t fg, color_t bg)
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
    glTranslatef(j * GLYPH_W, i * GLYPH_H + 119.05f + OFFSET_Y, 0.f);
    glScalef(1.f, -1.f, 0.0f);
    SetForegroundColor(bg);
    glutStrokeCharacter(GLUT_STROKE_MONO_ROMAN, ((i * 77 + j) % 10) + '0');
    glPopMatrix();
}

static void drawSomething()
{
    // text experiment
    for(int i = 0; i < 12; ++i) {
        for(int j = 0; j < 77; ++j) {
        }
    }

    for(int i = 0; i < 12; ++i) {
        for(int j = 0; j < 77; ++j) {
            color_t f, b;
            switch(((i * 77 + j) / (j + 1)) % 8)
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
            DrawCharacter(i, j, f, b);
        }
    }
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
    windowW = 800;
    windowH = 320;
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
