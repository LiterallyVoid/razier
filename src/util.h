
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

#include <math.h>

#pragma once

static inline double dotProduct(double *vector1, double *vector2) {
    return vector1[0] * vector2[0] + vector1[1] * vector2[1] + vector1[2] * vector2[2];
};

static inline void crossProduct(double *vector1, double *vector2, double *output) {
    *output = vector1[1] * vector2[2] - vector1[2] * vector2[1];
    *(output + 1) = vector1[2] * vector2[0] - vector1[0] * vector2[2];
    *(output + 2) = vector1[0] * vector2[1] - vector1[1] * vector2[0];
};

static inline double vlen(double *vector) {
    return sqrtf(vector[0] * vector[0] + vector[1] * vector[1] + vector[2] * vector[2]);
};

static inline void normalize(double *vector, double *out) {
    double len = vlen(vector);
    *out = vector[0] / len;
    *(out + 1) = vector[1] / len;
    *(out + 2) = vector[2] / len;
};

static inline void loadRotation(double *forwards, double *up) {
    double normForwards[3], normUp[3];
    normalize(forwards, normForwards);
    normalize(up, normUp);
    double left[3];
    crossProduct(normUp, normForwards, &left[0]);

    GLfloat matrix[16] = {
        left[0], left[1], left[2], 0,
        normUp[0], normUp[1], normUp[2], 0,
        normForwards[0], normForwards[1], normForwards[2], 0,
        0, 0, 0, 1
    };

    glMultMatrixf(matrix);
};

static inline double ang_diff(double a1, double a2, double unit) {
    double d = a1 - a2;
    while(d < -unit) {
        d += unit * 2;
    }

    while(d > unit) {
        d -= unit * 2;
    }
    return d;
};

// lerp(0, 2, 4, 10, 14) == 12

static inline double lerp(double il, double i, double ih, double ol, double oh) {
    return ((i - il) / (ih - il)) * (oh - ol) + ol;
};
