
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

#include "sound.h"
#include "track.h"

#pragma once

#define CTRL_ACCEL 1 << 0
#define CTRL_BRAKE 1 << 1
#define CTRL_LEFT  1 << 2
#define CTRL_RIGHT 1 << 3

#define CONTROL_SET(ship, thing) (ship)->control |= (thing)
#define CONTROL_UNSET(ship, thing) (ship)->control &= ~(thing)
#define CONTROL_CHANGE(ship, thing, act) {if(act) {CONTROL_SET(ship, thing);} else {CONTROL_UNSET(ship, thing);}}
#define CONTROL_GET(ship, thing) (ship)->control & (thing)

#define CCTRL_SET(controller, thing) (controller) |= (thing)
#define CCTRL_UNSET(controller, thing) (controller) &= ~(thing)
#define CCTRL_CHANGE(controller, thing, act) {if(act) {CCTRL_SET(controller, thing);} else {CCTRL_UNSET(controller, thing);}}
#define CCTRL_GET(controller, thing) (controller) & (thing)

typedef struct {
    int skin;

    Track *track;

    double trackPos, trackSide;
    double height, heightVel;

    double speed, sideSpeed;

    double lastParticleSpawn, lastParticleSpawn2;

    unsigned char control;

    double turning;
    double roll;

    ALuint engineSound, engineBuffer;
    ALuint scrapingSound, scrapingBuffer;

    double scrapingGain;

    int stop, laps;
    double time, lapTime;

    double shake;

    // EVERYTHING BELOW THIS IS AI

    double target;

    // AI variables
    double lookahead;
    double jumpiness;
    double edgeHugging;
    int reaction;

    unsigned char controls[101];
} Ship;

Ship *ship_init(Track *track);
void ship_deinit(Ship *ship);
void ship_camera(Ship *ship);
void ship_draw(Ship *ship);
void ship_drawShadow(Ship *ship);
void ship_doCollide(Ship *a, Ship *b);
void ship_update(Ship *ship, double dt);
void ship_doAI(Ship *ship);
