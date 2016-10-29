
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

#pragma once

#define MAX_PARTICLES 2048
#define PART_SPARK 0
#define PART_EXHAUST 1
#define PART_SMOKE 2

typedef struct {
    double x, y, z;
    double xV, yV, zV;

    double time;
    double life;
    double size;

    double timeScale, scaleStart, alpha;

    int type, tex;
    int valid;
} Particle;

int particle_enabled(void);
void particle_enable(int enabled);
void particle_draw(void);
void particle_update(double dt);
void particle_spawn(double x, double y, double z, double xV, double yV, double zV, int type);
