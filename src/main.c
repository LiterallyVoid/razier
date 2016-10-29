
/*

  Copyright (c) 2016  LiterallyVoid, KaadmY

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

#include <time.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include <AL/al.h>
#include <AL/alc.h>

#include "model.h"
#include "ship.h"
#include "texture.h"
#include "track.h"
#include "particle.h"
#include "sound.h"
#include "font.h"
#include "loader.h"

#include "menu.h"

#include "music.h"

static int width, height;

double delta;

static double accum = 0.0;
static const double deltaTime = 0.003;
double realTime, globalTime, globalDelta;

double ambientColor[3];

GLFWwindow *window;

static Ship *ships[50];
static int numShips;
Track *mainTrack = NULL;
Track *loadingTrack = NULL;

double fov = 70.0;
double camUp[3], camSide[3], camPos[3], camForwards[3];

static double finishTime = -1;
static int place = 0;

int pausing = 0;

static int queueInitTrack, queueDeinitTrack;
static char queueInitTrackName[128];

void key_callback(GLFWwindow *win, int key, int scancode, int action, int mods) {
    if(action == GLFW_PRESS && loader_done()) {
        menu_key(key);
    }

    if(key && scancode && win && mods) {
        return;
    }
};

void init_track(const char *track) {
    queueInitTrack = 1;
    strncpy(queueInitTrackName, track, 128);
};

void init_track_real(const char *track) {
    finishTime = -1;
    place = 0;
    realTime = -3;

    loadingTrack = track_load(track);

    numShips = 3;
    int i;

    float pos = 0;
    float side = 0.0001;

    for(i = 0; i < numShips; i++) {
        side = -side;
        if(side >= 0) {
            side += 3;
        }

        pos += 3;

        ships[i] = ship_init(loadingTrack);
        ships[i]->skin = rand() % 6;
        ships[i]->trackSide = side;
        ships[i]->trackPos += pos;
    }
    ships[0]->reaction = 0;
    ships[0]->jumpiness = 0.1;
    ships[0]->lookahead = 50;
    ships[0]->edgeHugging = 0.1;

    music_play(MUSIC_GAME);

    loader_loadAll();
};

void deinit_track(void) {
    queueDeinitTrack = 1;
};

void deinit_track_real(void) {
    if(!mainTrack) {
        return;
    }
    int i;
    for(i = 0; i < numShips; i++) {
        ship_deinit(ships[i]);
    }

    track_deinit(mainTrack);

    mainTrack = NULL;

    music_play(MUSIC_MENU);

};

void input(void) {
    if(!mainTrack) {
        return;
    }
    CONTROL_CHANGE(ships[0], CTRL_ACCEL, glfwGetKey(window, GLFW_KEY_W) == GLFW_PRESS);
    CONTROL_CHANGE(ships[0], CTRL_BRAKE, glfwGetKey(window, GLFW_KEY_S) == GLFW_PRESS);
    CONTROL_CHANGE(ships[0], CTRL_LEFT, glfwGetKey(window, GLFW_KEY_A) == GLFW_PRESS);
    CONTROL_CHANGE(ships[0], CTRL_RIGHT, glfwGetKey(window, GLFW_KEY_D) == GLFW_PRESS);
};

void draw(void) {
    tex_makeSmooth(tex_load("textures/track_map.png"));
    tex_load("interface/loading_1.png");
    tex_load("interface/loading_2.png");
    tex_load("interface/loading_3.png");
    /*tex_load("racer/skin_1.png");
    tex_load("racer/skin_2.png");
    tex_load("racer/skin_3.png");
    tex_load("racer/skin_4.png");
    tex_load("racer/skin_5.png");
    tex_load("racer/skin_6.png");*/
    if(!mainTrack) {
        return;
    }
    ship_camera(ships[0]);

    GLfloat lightPos[4] = {0, 0, 1, 0.0};
    glLightfv(GL_LIGHT0, GL_POSITION, lightPos);
    track_draw(mainTrack);
    glDisable(GL_DEPTH_TEST);
    glColor4f(1, 1, 1, 0.5);
    glBlendFunc(GL_SRC_ALPHA, GL_ONE);
    track_drawMap(mainTrack, ships[0]->trackPos);
    glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);
    glEnable(GL_DEPTH_TEST);

    /* SHADOW */
    glEnable(GL_STENCIL_TEST);
    glDepthMask(GL_FALSE);
    glDepthFunc(GL_GREATER);
    glPolygonOffset(20, 20);
    int i;

    glColorMask(GL_FALSE, GL_FALSE, GL_FALSE, GL_FALSE);
    glStencilFunc(GL_ALWAYS, 1, 0xFF);
    glStencilOp(GL_KEEP, GL_KEEP, GL_REPLACE);

    for(i = 0; i < numShips; i++) ship_drawShadow(ships[i]);

    glDepthFunc(GL_LESS);
    glPolygonOffset(-20, -20);

    glColorMask(GL_TRUE, GL_TRUE, GL_TRUE, GL_TRUE);
    glStencilFunc(GL_EQUAL, 1, 0xFF);
    glStencilOp(GL_KEEP, GL_KEEP, GL_KEEP);

    for(i = 0; i < numShips; i++) ship_drawShadow(ships[i]);

    glPolygonOffset(0, 0);
    glDepthMask(GL_TRUE);
    glDisable(GL_STENCIL_TEST);
    /* END SHADOW */

    glEnable(GL_LIGHTING);

    for(i = 0; i < numShips; i++) ship_draw(ships[i]);

    glDisable(GL_LIGHTING);

    glDepthMask(GL_FALSE);

    glBlendFunc(GL_SRC_ALPHA, GL_ONE);
    particle_draw();
    glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);

    glDepthMask(GL_TRUE);

    // TEXT DRAWING
    glMatrixMode(GL_PROJECTION);
    glLoadIdentity();
    glOrtho(0, width, height, 0, -1, 1);
    glMatrixMode(GL_MODELVIEW);
    glLoadIdentity();
    char txt[128];
    snprintf(txt, 128, "Speed: %d\nTime: %.3f\nLap time: %.3f\nLap: %d/%d", (int) ships[0]->speed, ships[0]->time, ships[0]->lapTime, ships[0]->laps, 3);
    glColor3f(1, 1, 1);
    glScalef(0.25, 0.25, 1);
    font_render(0, 0, txt);
    glLoadIdentity();
        
    glTranslatef(width / 2, height / 2, 0);

    char lapTxt[64] = "";
        
    float sz = 0, alph = 0;

    if(realTime < -2) {
        sz = 0.5;
        alph = realTime + 3;
        strcpy(lapTxt, "3");
    } else if(realTime < -1) {
        sz = 0.6;
        alph = realTime + 2;
        strcpy(lapTxt, "2");
    } else if(realTime < 0) {
        sz = 0.7;
        alph = realTime + 1;
        strcpy(lapTxt, "1");
    } else if(realTime < 1) {
        sz = 0.8;
        alph = realTime;
        strcpy(lapTxt, "GO!");
    }

    if(finishTime > 0 && finishTime > (realTime - 6)) {
        sz = 0.3;
        alph = (1 - (realTime - finishTime)) / 1.0;
        if(realTime > finishTime + 5) {
            alph = -4 - alph;
        }
        if(alph < 0) alph = 0;
        snprintf(lapTxt, 64, "You finished in place %d!", place);
    }

    sz += alph * 0.2;

    alph = 1 - alph;

    if(sz > 0 && alph > 0) {
        glScalef(sz, sz, 1);
        glColor4f(1, 0, 0, alph);

        font_render(-font_width(lapTxt) / 2, -FONTSIZE / 2, lapTxt);
    }
};

void update(float dt) {
    if(!mainTrack) {
        return;
    }
    if(realTime > (finishTime + 8) && finishTime > 0) {
        deinit_track();
        init_track("terran");
    }
    if(ships[0]->stop && finishTime < 0) {
        finishTime = realTime;
        music_play(MUSIC_VICTORY);
        place = 0;
        int i;
        for(i = 0; i < numShips; i++)
            if(ships[i]->stop) place++;
    }
    int i, j;
    for(i = 0; i < numShips; i++) {
        if(i > 0 || ships[0]->stop) {
            ship_doAI(ships[i]);
        }
        for(j = i + 1; j < numShips; j++) {
            ship_doCollide(ships[i], ships[j]);
        }
        ship_update(ships[i], dt);
    }

    particle_update(dt);
};

void audioPos(void) {
    static double oCamPos[3] = {0, 0, 0};
    ALfloat orientation[6] = {camForwards[0], camForwards[1], camForwards[2], camUp[0], camUp[1], camUp[2]};
    alListener3f(AL_POSITION, camPos[0], camPos[1], camPos[2]);
    alListener3f(AL_VELOCITY, (camPos[0] - oCamPos[0]) / delta, (camPos[1] - oCamPos[1]) / delta, (camPos[2] - oCamPos[2]) / delta);
    alListenerfv(AL_ORIENTATION, orientation);
    memcpy(oCamPos, camPos, sizeof(double) * 3);
};

int main(int argc, char **argv) {
    int i;
    for(i = 1; i < argc; i++) {
        printf("%s\n", argv[i]);
    }
    srand(time(NULL));
    /* window */
    if(!glfwInit()) {
        return 1;
    }

    glfwWindowHint(GLFW_CONTEXT_VERSION_MAJOR, 1);
    glfwWindowHint(GLFW_CONTEXT_VERSION_MINOR, 1);

    window = glfwCreateWindow(640, 480, "Razier", NULL, NULL);
    if(!window) {
        glfwTerminate();
        return 1;
    }

    glfwMakeContextCurrent(window);
    glfwSwapInterval(1);
    /* end */
    
    /* sound */
    sound_init();
    /* end */

    /* opengl */
    glEnable(GL_TEXTURE_2D);
    //glEnable(GL_CULL_FACE);
    glEnable(GL_DEPTH_TEST);
    glEnable(GL_ALPHA_TEST);
    glEnable(GL_BLEND);
    glAlphaFunc(GL_GREATER, 0.001);
    glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);

    glEnable(GL_LIGHTING);
    glEnable(GL_LIGHT0);
    glEnable(GL_COLOR_MATERIAL);
    glEnable(GL_POLYGON_OFFSET_FILL);
    glColorMaterial(GL_FRONT, GL_AMBIENT_AND_DIFFUSE);

    ambientColor[0] = 1.0;
    ambientColor[1] = 0.9;
    ambientColor[2] = 0.4;

    glClearColor(0.25, 0.15, 0.05, 0.0);
    /* end */

    /* world */
    //init_track("terran");
    /* end */

    font_init();

    double lastTime = glfwGetTime();

    double minDelta = 0.016, maxDelta = 0.016, lastDeltaCheck = 3.0;

    realTime = -3;

    glfwSetKeyCallback(window, key_callback);

    atexit(deinit_track_real);
    atexit(sound_deinit);
    atexit(music_deinit);

    music_load("music/menu.ogg", MUSIC_MENU);
    music_load("music/music.ogg", MUSIC_GAME);
    music_load("music/victory.ogg", MUSIC_VICTORY);

    music_play(MUSIC_MENU);

    while(!glfwWindowShouldClose(window)) {        
        music_update();

        glfwGetFramebufferSize(window, &width, &height);
        glViewport(0, 0, width, height);

        double t = glfwGetTime();
        delta = (t - lastTime);
        lastTime = t;

        globalTime += delta;
        globalDelta = delta;

        if(pausing) {
            pausing = 0;
            delta = 0;
        }

        realTime += delta;

        if(globalDelta > maxDelta) maxDelta = globalDelta;
        if(globalDelta < minDelta) minDelta = globalDelta;

        lastDeltaCheck += globalDelta;
        if(lastDeltaCheck > 2.0) {
            char title[128];
            snprintf(title, 128, "Razier [%d/%d/%d]", (int) (1 / maxDelta), (int) (1 / globalDelta), (int) (1 / minDelta));
            glfwSetWindowTitle(window, title);

            lastDeltaCheck = 0.0;
            minDelta = globalDelta;
            maxDelta = globalDelta;
        }

        double aspect = width / (double) height;

        // PROJECTION HERE
        glMatrixMode(GL_PROJECTION);
        glLoadIdentity();
        gluPerspective(fov, aspect, 0.1, 5000.0);
        glMatrixMode(GL_MODELVIEW);
        glLoadIdentity();
        // END PROJECTION

        glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT | GL_STENCIL_BUFFER_BIT);

        // INPUT HERE
        input();
        // END INPUT

        // DRAWING HERE
        glMatrixMode(GL_PROJECTION);
        glPushMatrix();
        glMatrixMode(GL_MODELVIEW);
        glPushMatrix();
        draw();
        glMatrixMode(GL_PROJECTION);
        glPopMatrix();
        glMatrixMode(GL_MODELVIEW);
        glPopMatrix();

        // MENU PROJECTION HERE
        glMatrixMode(GL_PROJECTION);
        glLoadIdentity();
        gluPerspective(40, aspect, 0.1, 5000.0);
        glMatrixMode(GL_MODELVIEW);
        glLoadIdentity();
        // END MENU PROJECTION

        glDisable(GL_DEPTH_TEST);

        menu_draw();
        glEnable(GL_DEPTH_TEST);
        // END DRAWING

        // UPDATE HERE
        accum += delta;

        while(accum > deltaTime) {
            update(deltaTime);
            accum -= deltaTime;
        }
        // END UPDATE

        // AUDIO POSITIONING HERE
        audioPos();
        // END AUDIO POSITIONING

        if(queueDeinitTrack) {
            deinit_track_real();
            queueDeinitTrack = 0;
        }

        if(queueInitTrack) {
            init_track_real(queueInitTrackName);
            queueInitTrack = 0;
        }

        if(loadingTrack != NULL) {
            if(loader_done()) {
                mainTrack = loadingTrack;
                loadingTrack = NULL;
                track_doLight(mainTrack);
            }
            glDisable(GL_DEPTH_TEST);
            loader_screen();
            glEnable(GL_DEPTH_TEST);
        }

        glfwSwapBuffers(window);
        glfwPollEvents();
    }

    deinit_track_real();

    return 0;
};
