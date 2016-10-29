
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

#include "model.h"

#pragma once

typedef struct {
    float r, g, b;
    float samples;
} LightSamp;

typedef struct {
    double pos[3];
    double normal[3];
    double size;
    int bouncers;
} TrackPoint;

typedef struct {
    TrackPoint *points;
    int numPoints;
    
    Model *model;
    Model *modelMoving;

    double len;

    LightSamp light[100][100];

    GLuint displayList, displayListMoving;
} Track;

Track *track_load(const char *path);
void track_doLight(Track *track);
void track_deinit(Track *track);
void track_get(Track *track, double pos, double *x, double *y, double *z, double *nx, double *ny, double *nz, double *size, double *to);
void track_draw(Track *track);
void track_drawMap(Track *track, double trackPos);
void track_getLight(Track *track, float x, float y, float z, float *r, float *g, float *b);
int track_getBouncers(Track *track, float pos, float side, double vel);
