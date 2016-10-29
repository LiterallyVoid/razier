
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
#include <stdlib.h>

#include "particle.h"
#include "main.h"
#include "texture.h"

static GLuint particleTex;
static Particle particles[MAX_PARTICLES];
static int enabled = 1;

int particle_enabled(void) {
    return enabled;
};

void particle_enable(int enable) {
    enabled = enable;
};

void particle_draw(void) {
    if(!enabled) return;

    if(particleTex == 0) particleTex = tex_load("textures/particle.png");

    int i;
    
    double aX = camUp[0], aY = camUp[1], aZ = camUp[2];
    double bX = camSide[0], bY = camSide[1], bZ = camSide[2];

    glBindTexture(GL_TEXTURE_2D, particleTex);

    double sheetGrid = 1.0 / 8.0;

    glBegin(GL_QUADS);
    for(i = 0; i < MAX_PARTICLES; i++) {
        if(particles[i].valid) {
            double x = particles[i].x;
            double y = particles[i].y;
            double z = particles[i].z;
            double size = particles[i].size * (particles[i].time * particles[i].timeScale + particles[i].scaleStart);
            int ofsX = particles[i].tex;
            int ofsY = particles[i].tex / 8;
            float a = particles[i].alpha * (1 - particles[i].time);
            glColor4f(1, 1, 1, a);
            glTexCoord2f(sheetGrid * ofsX, sheetGrid * ofsY);
            glVertex3f(x + ((-aX - bX) * size),
                       y + ((-aY - bY) * size),
                       z + ((-aZ - bZ) * size));

            glTexCoord2f(sheetGrid * ofsX, sheetGrid * (ofsY + 1));
            glVertex3f(x + ((-aX + bX) * size),
                       y + ((-aY + bY) * size),
                       z + ((-aZ + bZ) * size));

            glTexCoord2f(sheetGrid * (ofsX + 1), sheetGrid * (ofsY + 1));
            glVertex3f(x + ((+aX + bX) * size),
                       y + ((+aY + bY) * size),
                       z + ((+aZ + bZ) * size));

            glTexCoord2f(sheetGrid * (ofsX + 1), sheetGrid * ofsY);
            glVertex3f(x + ((+aX - bX) * size),
                       y + ((+aY - bY) * size),
                       z + ((+aZ - bZ) * size));
        }
    }
    glEnd();
};

void particle_update(double dt) {
    if(!enabled) return;

    int i;
    for(i = 0; i < MAX_PARTICLES; i++) {
        if(particles[i].valid) {
            particles[i].x += particles[i].xV * dt;
            particles[i].y += particles[i].yV * dt;
            particles[i].z += particles[i].zV * dt;
            particles[i].time += dt * particles[i].life;
            if(particles[i].time > 1) {
                particles[i].valid = 0;
            }
        }
    }
};

void particle_spawn(double x, double y, double z, double xV, double yV, double zV, int type) {
    if(!enabled) return;

    float posRand = 0.0, velRand = 0.5;
    if(type == PART_SPARK) {
        posRand = 1;
        velRand = 2;
    }
    x += ((rand() % 50 / 50.0) - 0.5) * posRand;
    y += ((rand() % 50 / 50.0) - 0.5) * posRand;
    z += ((rand() % 50 / 50.0) - 0.5) * posRand;
    xV += ((rand() % 50 / 50.0) - 0.5) * velRand;
    yV += ((rand() % 50 / 50.0) - 0.5) * velRand;
    zV += ((rand() % 50 / 50.0) - 0.5) * velRand;
    int i, replace = rand() % MAX_PARTICLES;
    for(i = 0; i < MAX_PARTICLES; i++) {
        if(!particles[i].valid) {
            replace = i;
            break;
        }
    }


    particles[replace].x = x;
    particles[replace].y = y;
    particles[replace].z = z;

    particles[replace].xV = xV;
    particles[replace].yV = yV;
    particles[replace].zV = zV;

    particles[replace].type = type;
    particles[replace].tex = (rand() % 8) + (type * 8);
    particles[replace].time = 0;

    if(type == PART_EXHAUST) {
        particles[replace].life = 15;
        particles[replace].timeScale = -1.0;
        particles[replace].scaleStart = 1;
        particles[replace].alpha = 1;
    } else if(type == PART_SMOKE) {
        particles[replace].life = 0.8;
        particles[replace].timeScale = -1.0;
        particles[replace].scaleStart = 1.0;
        particles[replace].alpha = 0.2;
    } else {
        particles[replace].life = 1;
        particles[replace].timeScale = 2;
        particles[replace].alpha = 1;
        particles[replace].scaleStart = 3;
    }

    particles[replace].valid = 1;
    particles[replace].size = ((rand() % 20) / 40.0);
};
