
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

#include <stdio.h>
#include <string.h>
#include <stdlib.h>

#include <pthread.h>

#include <AL/al.h>

#include "main.h"
#include "model.h"
#include "ship.h"

#include "util.h"
#include "texture.h"
#include "particle.h"
#include "sound.h"

#include "loader.h"

static Model *m = NULL, *m2 = NULL;
static int loadedModel = 0;

static pthread_mutex_t mutex;

static GLuint shadowTex = 0;

static void setModel(Model *model) {
    pthread_mutex_lock(&mutex);
    m = model;
    pthread_mutex_unlock(&mutex);
};

Ship *ship_init(Track *track) {
    if(!loadedModel) {
        loader_queue("racer/skin_1.png", LT_TEXTURE, -1, NULL);
        loader_queue("racer/skin_2.png", LT_TEXTURE, -1, NULL);
        loader_queue("racer/skin_3.png", LT_TEXTURE, -1, NULL);
        loader_queue("racer/skin_4.png", LT_TEXTURE, -1, NULL);
        loader_queue("racer/skin_5.png", LT_TEXTURE, -1, NULL);
        loader_queue("racer/skin_6.png", LT_TEXTURE, -1, NULL);
        loader_queue("racer/racer.mh2", LT_MODEL, 6, setModel);
        loadedModel = 1;
    }
    Ship *ship = malloc(sizeof(Ship));
    ship->track = track;
    ship->trackPos = track->len - 50;
    ship->trackSide = 0;
    ship->speed = 0;
    ship->sideSpeed = 0;
    ship->height = 8.0;
    ship->heightVel = 0.0;
    ship->control = 0;
    ship->lastParticleSpawn = 0;
    ship->lastParticleSpawn2 = 0;
    ship->stop = 0;
    ship->laps = 0;
    ship->time = 0;
    ship->lapTime = 0;
    
    ship->target = -100;

    ship->turning = 0;
    ship->roll = 0;
    ship->scrapingGain = 0;

    ship->shake = 0;

    ship->lookahead = ((rand() % 50) / 50.0) * 2 + 1;
    ship->jumpiness = ((rand() % 50) / 50.0) * 3.0;
    ship->edgeHugging = ((rand() % 50) / 50.0) * 0.8 - 0.4;
    ship->reaction = (rand() % 3) + 6;

    ship->engineSound = sound_load("sounds/engine.ogg", &ship->engineBuffer);
    alSourcei(ship->engineSound, AL_LOOPING, AL_TRUE);

    alSourcePlay(ship->engineSound);

    ship->scrapingSound = sound_load("sounds/scraping.ogg", &ship->scrapingBuffer);
    alSourcei(ship->scrapingSound, AL_LOOPING, AL_TRUE);
    alSourcef(ship->scrapingSound, AL_GAIN, 0.0);
    
    alSourcePlay(ship->scrapingSound);

    return ship;
};

void ship_deinit(Ship *ship) {
    alSourcei(ship->engineSound, AL_BUFFER, 0);
    alDeleteSources(1, &ship->engineSound);
    alDeleteBuffers(1, &ship->engineBuffer);

    alSourcei(ship->scrapingSound, AL_BUFFER, 0);
    alDeleteSources(1, &ship->scrapingSound);
    alDeleteBuffers(1, &ship->scrapingBuffer);
};

void ship_camera(Ship *ship) {
    double x, y, z, nx, ny, nz;
    double cx, cy, cz;
    track_get(ship->track, (ship->stop ? ship->trackPos - 40 : ship->trackPos - 10), &cx, &cy, &cz, &nx, &ny, &nz, NULL, NULL);
    track_get(ship->track, ship->trackPos, &x, &y, &z, &nx, &ny, &nz, NULL, NULL);

    cx += ((rand() % 15) / 15.0 - 0.5) * ship->shake;
    cy += ((rand() % 15) / 15.0 - 0.5) * ship->shake;
    cz += ((rand() % 15) / 15.0 - 0.5) * ship->shake;

    fov = 60 + (ship->speed * 0.2);

    double forwards[3] = {cx - x, cy - y, cz - z};
    double up[3] = {nx, ny, nz};

    double s = ship->trackSide;
    if(ship->stop) {
        s = 8;
        fov = 20;
        nx = 0;
        ny = 0;
        nz = 1;
        fov = 80;
    }

    camPos[0] = cx + (nx * 3);
    camPos[1] = cy + (ny * 3);
    camPos[2] = cz + (nz * 3);

    normalize(forwards, forwards);
    double side[3];
    crossProduct(forwards, up, side);

    x += side[0] * ship->trackSide;
    y += side[1] * ship->trackSide;
    z += side[2] * ship->trackSide;

    camPos[0] += side[0] * s;
    camPos[1] += side[1] * s;
    camPos[2] += side[2] * s;

    memcpy(camUp, up, sizeof(double) * 3);
    memcpy(camForwards, forwards, sizeof(double) * 3);
    memcpy(camSide, side, sizeof(double) * 3);
    
    double h = ship->height;
    if(h > 3) h = 3;

    glMatrixMode(GL_MODELVIEW);
    gluLookAt(camPos[0], camPos[1], camPos[2], x + nx * h, y + ny * h, z + nz * h, nx, ny, nz);
};

void ship_draw(Ship *ship) {
    if(shadowTex == 0) {
        shadowTex = tex_load("racer/shadow.png");
    }

    double x, y, z, nx, ny, nz;
    double x2, y2, z2;
    track_get(ship->track, ship->trackPos - ship->speed, &x2, &y2, &z2, &nx, &ny, &nz, NULL, NULL);
    track_get(ship->track, ship->trackPos, &x, &y, &z, &nx, &ny, &nz, NULL, NULL);

    alSource3f(ship->engineSound, AL_VELOCITY, x - x2, y - y2, z - z2);
    alSource3f(ship->scrapingSound, AL_VELOCITY, x - x2, y - y2, z - z2);

    track_get(ship->track, ship->trackPos + 10, &x2, &y2, &z2, &nx, &ny, &nz, NULL, NULL);
    
    alSourcef(ship->engineSound, AL_GAIN, (ship->speed * 0.008) + 0.0);
    alSourcef(ship->engineSound, AL_PITCH, (ship->speed * 0.008) + 0.2);
    alSource3f(ship->engineSound, AL_POSITION, x, y, z);
    alSource3f(ship->scrapingSound, AL_POSITION, x, y, z);

    double forwards[3] = {x - x2, y - y2, z - z2};
    double up[3] = {nx, ny, nz};

    normalize(forwards, forwards);

    double side[3];
    crossProduct(forwards, up, side);

    x += side[0] * ship->trackSide;
    y += side[1] * ship->trackSide;
    z += side[2] * ship->trackSide;

    float r, g, b;
    track_getLight(ship->track, x, y, z, &r, &g, &b);

    glPushMatrix();
    glTranslatef(x, y, z);
    loadRotation(up, forwards);
    glRotatef(-90, 0, 0, 1);
    glTranslatef(0, 0, ship->height);
    glRotatef(ship->roll * 0.3, 0, 0, 1);
    glRotatef(ship->roll * 1.2, -1, 0, 0);
    pthread_mutex_lock(&mutex);
    model_drawLight(m, ship->skin, r, g, b);
    pthread_mutex_unlock(&mutex);
    glPopMatrix();
};

void ship_drawShadow(Ship *ship) {
    double x, y, z, nx, ny, nz;
    double cx, cy, cz;
    track_get(ship->track, ship->trackPos - 10, &cx, &cy, &cz, &nx, &ny, &nz, NULL, NULL);
    track_get(ship->track, ship->trackPos, &x, &y, &z, &nx, &ny, &nz, NULL, NULL);

    double forwards[3] = {cx - x, cy - y, cz - z};
    double up[3] = {nx, ny, nz};

    normalize(forwards, forwards);

    double side[3];
    crossProduct(forwards, up, side);

    x += side[0] * ship->trackSide;
    y += side[1] * ship->trackSide;
    z += side[2] * ship->trackSide;

    float f = 0.4;
    glColor4f(1, 1, 1, (1 - (ship->height * 0.2)) * f);

    glPushMatrix();
    glTranslatef(x, y, z);
    loadRotation(up, forwards);
    glRotatef(-90, 0, 0, 1);
    glTranslatef(1, 0, 0);
    glRotatef(ship->roll * 0.3, 0, 0, 1);
    glBindTexture(GL_TEXTURE_2D, shadowTex);
    glBegin(GL_QUADS);
    glTexCoord2f(0, 0);
    glVertex3f(-4.9, -2.5, 0.0);
    glTexCoord2f(0, 1);
    glVertex3f(-4.9, 2.5, 0.0);
    glTexCoord2f(1, 1);
    glVertex3f(4.9, 2.5, 0.0);
    glTexCoord2f(1, 0);
    glVertex3f(4.9, -2.5, 0.0);
    glEnd();
    glPopMatrix();
};

void ship_doCollide(Ship *a, Ship *b) {
    if(a->trackPos < b->trackPos + 4 && a->trackPos + 4 > b->trackPos) {
        if(a->trackSide - 1 < b->trackSide + 1 && a->trackSide + 1 > b->trackSide - 1) {
            double posA[3], posB[3], posA2[3], posB2[3], n[3], c;

            track_get(a->track, a->trackPos + 2, &posA[0], &posA[1], &posA[2], &c, &c, &c, NULL, NULL);
            track_get(a->track, a->trackPos + 3, &posA2[0], &posA2[1], &posA2[2], &c, &c, &c, NULL, NULL);
            track_get(b->track, b->trackPos + 2, &posB[0], &posB[1], &posB[2], &n[0], &n[1], &n[2], NULL, NULL);
            track_get(b->track, b->trackPos + 3, &posB2[0], &posB2[1], &posB2[2], &c, &c, &c, NULL, NULL);

            normalize(n, n);
            double fwdA[3] = {posA[0] - posA2[0], posA[1] - posA2[1], posA[2] - posA2[0]};
            double fwdB[3] = {posB[0] - posB2[0], posB[1] - posB2[1], posB[2] - posB2[0]};

            double side[3];
            crossProduct(fwdA, n, side);
            posA[0] += side[0] * a->trackSide;
            posA[1] += side[1] * a->trackSide;
            posA[2] += side[2] * a->trackSide;
            crossProduct(fwdB, n, side);
            posB[0] += side[0] * b->trackSide;
            posB[1] += side[1] * b->trackSide;
            posB[2] += side[2] * b->trackSide;

            posA[0] += n[0] * a->height;
            posA[1] += n[1] * a->height;
            posA[2] += n[2] * a->height;

            posB[0] += n[0] * b->height;
            posB[1] += n[1] * b->height;
            posB[2] += n[2] * b->height;

            double mid[3] = {(posA[0] + posB[0]) * 0.5, (posA[1] + posB[1]) * 0.5, (posA[2] + posB[2]) * 0.5};

            int i;
            for(i = 0; i < 40; i++) {
                particle_spawn(mid[0], mid[1], mid[2], 0, 0, 0, PART_SPARK);
                particle_spawn(mid[0], mid[1], mid[2], 0, 0, 0, PART_SMOKE);
            }

            a->shake = 1.3;
            b->shake = 1.3;

            float ofs = (a->trackPos - b->trackPos) * 3;
            float ofs2 = (a->trackSide - b->trackSide) * 3;

            a->speed += ofs; a->sideSpeed += ofs2;
            b->speed -= ofs; b->sideSpeed -= ofs2;
        }
    }
};

void ship_update(Ship *ship, double dt) {
    int stop = 0;

    if(realTime < 0 && realTime >= -1) {
        stop = 1;
    }

    if(realTime < 0) {
        ship->control = 0;
    }

    if(!stop && !ship->stop && realTime > 0 && ship->laps > 0) {
        ship->time += dt;
        ship->lapTime += dt;
    }

    ship->shake -= 0.04;
    if(ship->shake < 0) {
        ship->shake = 0;
    }

    double a, xp, yp, zp;

    track_get(ship->track, ship->trackPos - 0.5, &xp, &yp, &zp, &a, &a, &a, NULL, NULL);

    double cam[3] = {camPos[0] - xp, camPos[1] - yp, camPos[2] - zp};

    int spawnParticles = 0, spawnParticles2 = 0, doParticles = 0;

    double dist = vlen(cam);
    if(dist < 200) {
        doParticles = 1;
    }

    if(ship->lastParticleSpawn > 0.04 / (ship->speed - 4) && ship->speed > 4) {
        if(doParticles) {
            spawnParticles = 1;
        }
        ship->lastParticleSpawn = 0;
    }
    ship->lastParticleSpawn += dt;

    if(ship->lastParticleSpawn2 > 0.002) {
        if(doParticles) {
            spawnParticles2 = 1;
        }
        ship->lastParticleSpawn2 = 0;
    }
    ship->lastParticleSpawn2 += dt;

    double x2, y2, z2;
    double speed = 8;
    if(realTime < 0) {
        speed = -50;
    }
    if(CONTROL_GET(ship, CTRL_ACCEL)) speed = 50;
    if(CONTROL_GET(ship, CTRL_BRAKE)) speed = -50;

    if(CONTROL_GET(ship, CTRL_LEFT)) {
        if(ship->turning <= 0) ship->turning = 3;
        ship->turning += dt * 9;
    } else if(CONTROL_GET(ship, CTRL_RIGHT)) {
        if(ship->turning >= 0) ship->turning = -3;
        ship->turning -= dt * 9;
    } else {
        ship->turning = 0;
    }

    if(ship->turning < -12) ship->turning = -12;
    if(ship->turning > 12) ship->turning = 12;

    double fric = 0.8 + (ship->height * 0.03);
    if(fric > 0.99) {
        fric = 0.99;
    }

    ship->speed *= pow(fric, dt * 5);
    ship->speed += speed * dt;
    ship->sideSpeed *= pow(0.5, dt * 5);
    ship->sideSpeed += ship->turning * dt * (15 - (ship->speed * 0.05)) * 0.8;

    ship->roll -= (ship->roll - ship->turning * 3) * 0.03;
    
    if(ship->speed <= 0.001) {
        ship->speed = 0.001;
    }

    ship->trackPos += dt * ship->speed;
    ship->trackSide += dt * ship->sideSpeed;

    double x1, y1, z1, nx, ny, nz, size;
    double xm, ym, zm;
    double oldPos = ship->trackPos;
    track_get(ship->track, ship->trackPos - 0.5 - ship->speed * dt, &xm, &ym, &zm, &nx, &ny, &nz, &size, &ship->trackPos);
    int bouncers = track_getBouncers(ship->track, ship->trackPos, ship->trackSide, ship->speed * dt);

    if(ship->trackPos < oldPos && !ship->stop) {
        ship->laps++;
        if(ship->laps == 4) {
            ship->stop = 1;
            ship->laps = 3;
        } else {
            ship->lapTime = 0;
        }
    }
    track_get(ship->track, ship->trackPos - 0.5, &x1, &y1, &z1, &nx, &ny, &nz, NULL, NULL);
    track_get(ship->track, ship->trackPos - 0.5 + ship->speed * dt, &x2, &y2, &z2, &a, &a, &a, NULL, NULL);
    double vecm[3] = {xm - x1, ym - y1, zm - z1};
    double vec1[3] = {x1 - x2, y1 - y2, z1 - z2};
    double vec2[3] = {nx, ny, nz};

    double up[3] = {nx, ny, nz};

    normalize(vec1, vec1);
    normalize(vecm, vecm);

    double side[3];
    crossProduct(vec1, up, side);

    x1 += side[0] * ship->trackSide;
    y1 += side[1] * ship->trackSide;
    z1 += side[2] * ship->trackSide;

    x2 += side[0] * ship->trackSide;
    y2 += side[1] * ship->trackSide;
    z2 += side[2] * ship->trackSide;

    double x3, y3, z3;
    x3 = x1 - side[0] * 1.0;
    y3 = y1 - side[1] * 1.0;
    z3 = z1 - side[2] * 1.0;

    double x4, y4, z4;
    x4 = x1 + side[0] * 1.0;
    y4 = y1 + side[1] * 1.0;
    z4 = z1 + side[2] * 1.0;

    if(bouncers) {
        ship->speed += 25;
    }

    float diff = atan2(vecm[1], vecm[0]) - atan2(vec1[1], vec1[0]);
    if(diff > M_PI * 0.5 || diff < -M_PI * 0.5) {
        diff = 0;
    }

    float nSpeed = ship->sideSpeed * -sin(diff) + ship->speed * cos(diff);
    float nSideSpeed = ship->sideSpeed * cos(diff) + ship->speed * sin(diff);

    ship->speed = nSpeed;
    ship->sideSpeed = nSideSpeed;

    if(ship->trackSide < -(size - 2)) {
        ship->sideSpeed -= (ship->trackSide + (size - 2)) * dt * 30;
    }

    if(ship->trackSide > (size - 2)) {
        ship->sideSpeed -= (ship->trackSide - (size - 2)) * dt * 30;
    }

    ship->heightVel += dt * 2;
    ship->heightVel += dotProduct(vec1, vec2) * dt * ship->speed * 30.0;
    ship->height += ship->heightVel * dt;
    ship->heightVel -= (ship->height - 1.4) * dt * 6;
    ship->heightVel += ship->speed * dt * 0.1;
    ship->heightVel *= pow(0.9, dt * 5);
    double scraping = 0;
    if(ship->height < 1.0 && ship->heightVel < 0) {
        ship->height = 1.0;
        ship->heightVel *= -0.05;
        scraping = 1.0;
        if(spawnParticles) {
            particle_spawn(x3, y3, z3 + ship->height - 0.4, 0, 0, 0, PART_SPARK);
            particle_spawn(x4, y4, z4 + ship->height - 0.4, 0, 0, 0, PART_SPARK);
        }
    }
    if(ship->trackSide < -size) {
        ship->trackSide = -size;
        ship->sideSpeed = (fabs(ship->sideSpeed) + 15) * 0.7;
        ship->speed *= 0.8;
        ship->turning = 0;
        if(ship->height < 4) {
            scraping = 1.0;
            if(spawnParticles) {
                particle_spawn(x3, y3, z3 + ship->height - 0.4, 0, 0, 0, PART_SPARK);
            }
        }
    }
    if(ship->trackSide > size) {
        ship->trackSide = size;
        ship->sideSpeed = (fabs(ship->sideSpeed) + 15) * -0.7;
        ship->speed *= 0.8;
        ship->turning = 0;
        if(ship->height < 4) {
            scraping = 1.0;
            if(spawnParticles) {
                particle_spawn(x4, y4, z4 + ship->height - 0.4, 0, 0, 0, PART_SPARK);
            }
        }
    }
    if(spawnParticles2 && CONTROL_GET(ship, CTRL_ACCEL)) {
        float exhaustspeed = (1.0 / 20.0) * ship->speed * 0.4;
        particle_spawn(x1 + nx * ship->height, y1 + ny * ship->height, z1 + nz * ship->height, (x2 - x1) * exhaustspeed, (y2 - y1) * exhaustspeed, (z2 - z1) * exhaustspeed, PART_EXHAUST);
    }
    if(spawnParticles) {
        particle_spawn(x1 + nx * ship->height, y1 + ny * ship->height, z1 + nz * ship->height, 0, 0, 0, PART_SMOKE);
        
        //particle_spawn(x3 + nx * (ship->height - 0.4), y3 + ny * (ship->height - 0.4), z3 + nz * (ship->height - 0.4), 0, 0, 0, PART_SMOKE);
        //particle_spawn(x4 + nx * (ship->height - 0.4), y4 + ny * (ship->height - 0.4), z4 + nz * (ship->height - 0.4), 0, 0, 0, PART_SMOKE);
    }

    if(scraping > 0) {
        scraping *= ship->speed * 0.02;
        ship->scrapingGain = scraping;
    }
    ship->scrapingGain -= (ship->scrapingGain - scraping) * 0.01;

    alSourcef(ship->scrapingSound, AL_GAIN, ship->scrapingGain);
};

void ship_doAI(Ship *ship) {
    if(ship->target < -50) {
        ship->target = ((rand() % 100) / 100.0) * 6 - 3;
    }

    double x1, y1, z1, x2, y2, z2, x3, y3, z3, nx, ny, nz, a;
    track_get(ship->track, ship->trackPos, &x1, &y1, &z1, &nx, &ny, &nz, NULL, NULL);
    track_get(ship->track, ship->trackPos + 0.1, &x2, &y2, &z2, &a, &a, &a, NULL, NULL);
    track_get(ship->track, ship->trackPos + ship->lookahead, &x3, &y3, &z3, &a, &a, &a, NULL, NULL);

    double projx3 = x1 + (x2 - x1) * ship->lookahead * 10;
    double projy3 = y1 + (y2 - y1) * ship->lookahead * 10;
    double projz3 = z1 + (z2 - z1) * ship->lookahead * 10;

    double fwd[3] = {x1 - x2, y1 - y2, z1 - z2};
    normalize(fwd, fwd);
    double up[3] = {nx, ny, nz};

    double side[3];
    crossProduct(fwd, up, side);

    double dist[3] = {projx3 - x3, projy3 - y3, projz3 - z3};
    
    double dir = dotProduct(side, dist);

    double target = -vlen(dist) * dir * ship->edgeHugging;
    target += ship->target;
    int stop = 0;

    if(target < -4) {
        target = -4;
    }
    if(target > 4) {
        target = 4;
    }

    unsigned char cctrl = 0;
    CCTRL_CHANGE(cctrl, CTRL_LEFT, ship->trackSide < target - ship->jumpiness);
    CCTRL_CHANGE(cctrl, CTRL_RIGHT, ship->trackSide > target + ship->jumpiness);
    if(!stop) {
        CCTRL_SET(cctrl, CTRL_ACCEL);
    } else {
        CCTRL_UNSET(cctrl, CTRL_ACCEL);
    }
    CCTRL_UNSET(cctrl, CTRL_BRAKE);

    int i;
    for(i = 0; i < ship->reaction; i++) {
        ship->controls[i] = ship->controls[i + 1];
    }
    ship->controls[ship->reaction] = cctrl;
    ship->control = ship->controls[0];
};
