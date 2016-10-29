
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

#include <ctype.h>
#include <stdio.h>
#include <stdlib.h>

#include "font.h"

#define GLFW_INCLUDE_GLU
#include <GLFW/glfw3.h>

#define STB_TRUETYPE_IMPLEMENTATION
#include "stb/stb_truetype.h"

static stbtt_bakedchar cdata[96];
static GLuint ftex;

void font_init(void) {
    unsigned char ttf_buffer[1<<20];
    unsigned char temp_bitmap[FONT_BITMAPSIZE * FONT_BITMAPSIZE];

    FILE *f = fopen("data/font.ttf", "rb");
    if(f == NULL) {
        printf("Could not load font. Expect errors.\n");
        return;
    }
    fread(ttf_buffer, 1, 1<<20, f);
    fclose(f);

    stbtt_BakeFontBitmap(ttf_buffer, 0, FONTSIZE, temp_bitmap, FONT_BITMAPSIZE, FONT_BITMAPSIZE, 32, 96, cdata);

    glGenTextures(1, &ftex);
    glBindTexture(GL_TEXTURE_2D, ftex);
    glTexImage2D(GL_TEXTURE_2D, 0, GL_ALPHA, FONT_BITMAPSIZE, FONT_BITMAPSIZE, 0, GL_ALPHA, GL_UNSIGNED_BYTE, temp_bitmap);

    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
};

float font_width(char *text) {
    float w = 0;
    while(*text) {
        if(isprint(*text)) {
            stbtt_aligned_quad q;
            float x = 0, y = 0;
            stbtt_GetBakedQuad(cdata, FONT_BITMAPSIZE, FONT_BITMAPSIZE, *text-32, &x, &y, &q, 1);
            w += x;
        }
        ++text;
    }
    return w;
};

void font_render(float x, float y, char *text) {
    y += FONTSIZE;
    glBindTexture(GL_TEXTURE_2D, ftex);
    glBegin(GL_QUADS);
    float sx = x, sy = y;
    while(*text) {
        if(isprint(*text)) {
            stbtt_aligned_quad q;
            stbtt_GetBakedQuad(cdata, FONT_BITMAPSIZE, FONT_BITMAPSIZE, *text-32, &x, &y, &q, 1);
            glTexCoord2f(q.s0, q.t0); glVertex2f(q.x0, q.y0);
            glTexCoord2f(q.s1, q.t0); glVertex2f(q.x1, q.y0);
            glTexCoord2f(q.s1, q.t1); glVertex2f(q.x1, q.y1);
            glTexCoord2f(q.s0, q.t1); glVertex2f(q.x0, q.y1);
        } else if(*text == '\n') {
            x = sx;
            y = sy + FONTSIZE;
            sy = y;
        }
        ++text;
    }
    glEnd();    
};
