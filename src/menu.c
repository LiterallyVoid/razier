
/*

  Copyright (c) 2016  Void7, KaadmY

  Permission is hereby granted, free of charge, to any person obtaining
  a copy of this software and associated documentation files (the
  "Software"), to deal in the Software without restriction, including
  without limitation the rights to use, copy, modify, merge, publish,
  distribute, sublicense, and/or sell copies of the Software, and to
  permit persons to whom the Software is furnished to do so, subject to
  the following conditions:

  The above copyright notice and this permission notice shall be
  included in all copies or substantial portions of the Software.

  THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND,
  EXPRESS OR IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF
  MERCHANTABILITY, FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT.
  IN NO EVENT SHALL THE AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY
  CLAIM, DAMAGES OR OTHER LIABILITY, WHETHER IN AN ACTION OF CONTRACT,
  TORT OR OTHERWISE, ARISING FROM, OUT OF OR IN CONNECTION WITH THE
  SOFTWARE OR THE USE OR OTHER DEALINGS IN THE SOFTWARE.

*/

#define GLFW_INCLUDE_GLU
#include <GLFW/glfw3.h>

#include <string.h>
#include <stdio.h>
#include <stdlib.h>

#include <math.h>

#include "font.h"
#include "main.h"
#include "particle.h"

static int selItem = 0;
static int selecting = 0;
static double smoothItem = 0;
static double randomness = 0;
static double menuShown = 0;
static int menuShowing = 0;
static int items = 0;

static int state = 0, nextState = 0; 

static int get_next_state(void) {
    if(state == 0) {
        return selItem + 1;
    } else if(state == 1) {
        deinit_track();
        init_track("terran");
        return 0;
    } else if(state == 2) {
        particle_enable(!particle_enabled());
        return -1;
    } else if(state == 3) {
        if(selItem == 0) {
            return 0;
        } else {
            if(mainTrack) {
                deinit_track();
            } else {
                exit(0);
            }
            menuShowing = 0;
            return 0;
        }
    }
    return 0; // bail out
};

static void item(double item, char *text) {
    char *ntxt = malloc(strlen(text) + 1);
    strcpy(ntxt, text);
    int i;
    for(i = 0; i < randomness; i++) {
        ntxt[rand() % strlen(text)] = (rand() % 96) + 32;
    }
    for(i = 8; i < randomness; i++) {
        ntxt[rand() % strlen(text)] = 32;
    }
    glPushMatrix();
    glTranslatef(-14, -item + 5, 0.01);
    glScalef(1.0 / FONTSIZE, -1.0 / FONTSIZE, 1.0 / FONTSIZE);
    font_render(0, 0, ntxt);
    glPopMatrix();
    free(ntxt);
};

void menu_draw(void) {
    menuShown += (menuShowing - menuShown) * globalDelta * 10;
    float a = menuShown;

    if(menuShowing) {
        pausing = 1;
    } else {
        pausing = 0;
    }
    
    glDisable(GL_LIGHTING);

    if(selecting == 1) {
        randomness += globalDelta * 70;
        if(randomness > 20) {
            selecting = 2;
        }
    } else if(selecting == 2) {
        state = nextState;
        selItem = 0;
        smoothItem = 0;
        randomness -= globalDelta * 70;
        if(randomness < 0) {
            randomness = 0;
            selecting = 3;
        }
    } else if(randomness > 0) {
        randomness -= globalDelta * 70;
        if(randomness < 0) {
            randomness = 0;
        }
    }

    smoothItem += (selItem - smoothItem) * globalDelta * 30;

    glPushMatrix();
    glMatrixMode(GL_PROJECTION);
    glPushMatrix();
    gluLookAt(-24, 0, 24, 0, 0, 0, 0, 1, 0);
    glTranslatef(sin(globalTime) * 0.15, cos(globalTime) * 0.15, sin(globalTime) * 0.15);
    glMatrixMode(GL_MODELVIEW);

    glBindTexture(GL_TEXTURE_2D, 0);
    glColor4f(0, 0, 0, a * 0.9);
    glBegin(GL_QUADS);
    /*
    glVertex2f(-14, -4);
    glVertex2f(-1, -4);
    glVertex2f(-1, 5);
    glVertex2f(-14, 5);*/

    glVertex3f(-50, -15, -50);
    glVertex3f(50, -15, 50);
    glVertex3f(50, 15, 50);
    glVertex3f(-50, 15, -50);
    glEnd();
    glColor4f(1, 1, 1, a);
    
    item(smoothItem, "> ");
    glPushMatrix();
    glTranslatef(font_width("> ") / FONTSIZE, 0, 0);

    char title[64];

    if(state == 0) {
        item(0, "LOAD MAP");
        item(1, "OPTIONS");
        if(mainTrack) {
            item(2, "LEAVE GAME");
        } else {
            item(2, "QUIT GAME");
        }
        strcpy(title, ":: MAIN MENU ::");
        items = 3;
    } else if(state == 1) {
        item(0, "TERRAN");
        strcpy(title, ":: LOAD MAP ::");
        items = 1;
    } else if(state == 2) {
        if(particle_enabled()) {
            item(0, "PARTICLES: ON");
        } else {
            item(0, "PARTICLES: OFF");
        }
        strcpy(title, ":: OPTIONS ::");
        items = 1;
    } else if(state == 3) {
        item(0, "NO");
        item(1, "YES");
        if(mainTrack) {
            strcpy(title, ":: LEAVE GAME ::");
        } else {
            strcpy(title, ":: QUIT GAME ::");
        }
        items = 2;
    }

    glPopMatrix();

    glRotatef(-90, 0, 1, 0);
    glScalef(2, 2, 2);
    glTranslatef(12, -8, 0);
    item(-1, title);

    /*glRotatef(-90, 0, 1, 0);
    glRotatef(-90, 0, 0, 1);
    glScalef(2, 2, 2);
    glTranslatef(10, -2, 0);
    item(-1, ":: MAIN MENU ::");*/
    
    glMatrixMode(GL_PROJECTION);
    glPopMatrix();
    glMatrixMode(GL_MODELVIEW);
    glPopMatrix();
};

void menu_key(int key) {
    if(key == GLFW_KEY_UP && menuShowing) {
        selItem--;
        if(selItem < 0) {
            selItem = items - 1;
        }
    } else if(key == GLFW_KEY_DOWN && menuShowing) {
        selItem++;
        if(selItem >= items) {
            selItem = 0;
        }
    } else if(key == GLFW_KEY_ENTER && menuShowing) {
        nextState = get_next_state();
        if(nextState != -1) {
            selecting = 1;
        }
    } else if(key == GLFW_KEY_ESCAPE) {
        menuShowing = !menuShowing;
        if(menuShowing) {
            randomness = 20;
            state = 0;
            nextState = 0;
        }
    } else if(key == GLFW_KEY_BACKSPACE) {
        selecting = 1;
        nextState = 0;
    }
};
