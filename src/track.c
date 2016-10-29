
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
#include <stdlib.h>

#include <math.h>
#include <string.h>

#include "track.h"
#include "main.h"
#include "util.h"

#include "texture.h"

#include "loader.h"

static Track *currentTrack;

static void setModel(Model *model) {
    currentTrack->model = model;
};

static void setModelMoving(Model *model) {
    currentTrack->modelMoving = model;
};

Track *track_load(const char *mid) {
    char concat[] = "data/tracks/";
    char end[] = ".trk";
    char *path = malloc(strlen(end) + strlen(concat) + strlen(end) + 1);
    strcpy(path, concat);
    strcat(path, mid);
    strcat(path, end);

    TrackPoint point;
    
    FILE *fp = fopen(path, "rb");
    if(fp == NULL) {
        perror("fopen");
        fprintf(stderr, "(while opening file %s)\n", path);
        free(path);
        return NULL;
    }
    
    Track *track = malloc(sizeof(Track));
    currentTrack = track;

    track->points = malloc(sizeof(TrackPoint));
    track->numPoints = 0;
    int allocPoints = 1;

    track->len = 1000000; // big number

    while(1) {
        int num = fscanf(fp, "%lf %lf %lf %lf %lf %lf %lf %d", &point.pos[0], &point.pos[1], &point.pos[2], &point.normal[0], &point.normal[1], &point.normal[2], &point.size, &point.bouncers);
        if(num == 8) {
            track->points[track->numPoints] = point;
            track->numPoints++;
            if(track->numPoints >= allocPoints) {
                allocPoints *= 2;
                track->points = realloc(track->points, sizeof(TrackPoint) * allocPoints);
            }
        } else {
            break;
        }
    }
    fclose(fp);

    free(path);

    loader_queue("textures/track_map.png", LT_TEXTURE, -1, NULL);

    char concat2[] = "tracks/";
    char end2[] = ".mh2";
    char end3[] = "_moving.mh2";
    char *modelPath = malloc(strlen(concat2) + strlen(mid) + strlen(end2) + 1);
    strcpy(modelPath, concat2);
    strcat(modelPath, mid);
    strcat(modelPath, end2);
    //track->model = model_load(modelPath, 1);
    loader_queue(modelPath, LT_MODEL, 1, setModel);
    free(modelPath);

    char *modelPath2 = malloc(strlen(concat2) + strlen(mid) + strlen(end3) + 1);
    strcpy(modelPath2, concat2);
    strcat(modelPath2, mid);
    strcat(modelPath2, end3);
    loader_queue(modelPath2, LT_MODEL, 1, setModelMoving);
    free(modelPath2);

    double a;
    track_get(track, 500000, &a, &a, &a, &a, &a, &a, NULL, NULL);

    track->displayList = 0;
    track->displayListMoving = 0;

    return track;
};

void track_doLight(Track *track) {
    int i, j;
    for(i = 0; i < 100; i++) {
        for(j = 0; j < 100; j++) {
            track->light[i][j].r = 1;
            track->light[i][j].g = 1;
            track->light[i][j].b = 1;
            track->light[i][j].samples = 1;
        }
    }


#define SAMP(x, y, f)                                 \
    track->light[(int) (x)][(int) (y)].r += col[0] * f; \
    track->light[(int) (x)][(int) (y)].g += col[1] * f; \
    track->light[(int) (x)][(int) (y)].b += col[2] * f; \
    track->light[(int) (x)][(int) (y)].samples += f;

    for(i = 0; i < track->model->numVerts; i++) {
        float pos[3];
        memcpy(pos, track->model->vertices[i].pos, sizeof(float) * 3);

        float col[3];
        memcpy(col, track->model->vertices[i].color, sizeof(float) * 3);

        pos[0] *= 0.1; pos[0] += 50.0;
        pos[1] *= 0.1; pos[1] += 50.0;

        int k;
        for(j = -4; j <= 4; j++) {
            for(k = -4; k <= 4; k++) {
                int aj = abs(j);
                int ak = abs(k);
                float f = (aj > ak ? aj : ak);
                f /= 4.0;
                f = 1 - f;
                f = pow(f, 4);
                SAMP(pos[0] + j, pos[1] + k, f);
            }
        }
    }
    for(i = 0; i < 100; i++) {
        for(j = 0; j < 100; j++) {
            float s = track->light[i][j].samples;
            if(s > 0) {
                track->light[i][j].r /= s;
                track->light[i][j].g /= s;
                track->light[i][j].b /= s;
            }
            track->light[i][j].r *= ambientColor[0];
            track->light[i][j].g *= ambientColor[1];
            track->light[i][j].b *= ambientColor[2];
        }
    }
#undef SAMP
};

void track_deinit(Track *track) {
    glDeleteLists(1, track->displayList);
    glDeleteLists(1, track->displayListMoving);

    model_deinit(track->model);
    model_deinit(track->modelMoving);
    free(track->points);
    free(track);

};

void track_get(Track *track, double pos, double *x, double *y, double *z, double *nx, double *ny, double *nz, double *size, double *to) {
    double p1[3], p2[3];
    double n1[3], n2[3];
    double s1 = 6, s2 = 6;

    int i;

    if(pos < 0) {
        pos += track->len;
    }
    if(pos > track->len) {
        pos -= track->len;
        if(to != NULL) {
            *to = pos;
        }
    }

    double start = 0, end = 0;
    for(i = 0; i < track->numPoints + 2; i++) {
        int i2 = i + 1;
        int i1 = i;
        if(i2 == track->numPoints) {
            i2 = 0;
        } else if(i2 > track->numPoints) {
            i2 = 1;
        }
        if(i1 == track->numPoints) {
            i1 = 0;
        } else if(i1 > track->numPoints) {
            i1 = 1;
        }

        memcpy(p1, track->points[i1].pos, 3 * sizeof(double));
        memcpy(p2, track->points[i2].pos, 3 * sizeof(double));

        memcpy(n1, track->points[i1].normal, 3 * sizeof(double));
        memcpy(n2, track->points[i2].normal, 3 * sizeof(double));

        s1 = track->points[i1].size;
        s2 = track->points[i2].size;

        double dist = sqrtf((p1[0] - p2[0]) * (p1[0] - p2[0]) + (p1[1] - p2[1]) * (p1[1] - p2[1]) + (p1[2] - p2[2]) * (p1[2] - p2[2]));
        double next = start + dist;
        if(i == track->numPoints) {
            if(track->len != start) {
                track->len = start;
            }
            if(to != NULL) {
                *to = pos;
            }
        }
        if(next < pos) {
            start = next;
        } else {
            end = next;
            break;
        }
    }
    *x = lerp(start, pos, end, p1[0], p2[0]);
    *y = lerp(start, pos, end, p1[1], p2[1]);
    *z = lerp(start, pos, end, p1[2], p2[2]);
    
    *nx = lerp(start, pos, end, n1[0], n2[0]);
    *ny = lerp(start, pos, end, n1[1], n2[1]);
    *nz = lerp(start, pos, end, n1[2], n2[2]);

    if(size != NULL) {
        *size = lerp(start, pos, end, s1, s2) * 0.5;
    }
};

void track_getLight(Track *track, float x, float y, float z, float *r, float *g, float *b) {
    x *= 0.1;
    x += 50;

    y *= 0.1;
    y += 50;
    
    /* TO SHUT UP THE WARNINGS... */
    z *= 0.1;
    z += 50;
    /* END WARNING-IFIER */

    LightSamp a00 = track->light[(int) x][(int) y];
    LightSamp a01 = track->light[(int) x][(int) y + 1];
    LightSamp a10 = track->light[(int) x + 1][(int) y];
    LightSamp a11 = track->light[(int) x + 1][(int) y + 1];

    float xLerp = x - (int) x;
    float yLerp = y - (int) y;

    LightSamp e1 = {a00.r * (1 - xLerp) + a10.r * xLerp, a00.g * (1 - xLerp) + a10.g * xLerp, a00.b * (1 - xLerp) + a10.b * xLerp, 1};
    LightSamp e2 = {a01.r * (1 - xLerp) + a11.r * xLerp, a01.g * (1 - xLerp) + a11.g * xLerp, a01.b * (1 - xLerp) + a11.b * xLerp, 1};

    *r = e1.r * (1 - yLerp) + e2.r * yLerp;
    *g = e1.g * (1 - yLerp) + e2.g * yLerp;
    *b = e1.b * (1 - yLerp) + e2.b * yLerp;
};

void track_draw(Track *track) {
    if(track->displayList == 0) {
        track->displayList = glGenLists(1);
        track->displayListMoving = glGenLists(1);

        glNewList(track->displayList, GL_COMPILE);
        model_draw(track->model, 0);
        glEndList();

        glNewList(track->displayListMoving, GL_COMPILE);
        model_draw(track->modelMoving, 0);
        glEndList();
    }

    glCallList(track->displayList);
    glMatrixMode(GL_TEXTURE);
    glLoadIdentity();
    glTranslatef(0, fmod(realTime * 0.25, 0.0625), 0);
    glCallList(track->displayListMoving);
    glLoadIdentity();
    glMatrixMode(GL_MODELVIEW);
};

void track_drawMap(Track *track, double trackPos) {
    GLuint mapTex = tex_load("textures/track_map.png");
    tex_makeSmooth(mapTex);

    glBindTexture(GL_TEXTURE_2D, mapTex);
    glBegin(GL_TRIANGLE_STRIP);
    int i, b = 0;
    int skip = 3;

    double len = 0;
    double acLen = (int) (track->len * 0.1);
    double lenScale = 1.0 / track->len;
    lenScale *= acLen;

    double a = 0;

    float dist = 0.005;

    for(i = 0; i < track->numPoints + skip; i += skip) {
        int in = i - skip, ip = i + skip;
        if(in < 0) {
            in += track->numPoints;
        }
        if(ip >= track->numPoints) {
            ip -= track->numPoints;
        }
        if(i >= track->numPoints) {
            i = 0;
            b = 1;
        }

        float ang1 = atan2(track->points[in].pos[1] - track->points[i].pos[1],
                          track->points[in].pos[0] - track->points[i].pos[0]);
        float ang2 = atan2(track->points[ip].pos[1] - track->points[i].pos[1],
                          track->points[ip].pos[0] - track->points[i].pos[0]);
        float ang = (ang1 + ang2) * 0.5;
        float diff = ang_diff(a, ang, M_PI * 0.5);

        a -= ang_diff(a, ang, M_PI * 0.5);

        double xd = track->points[in].pos[0] - track->points[i].pos[0];
        double yd = track->points[in].pos[1] - track->points[i].pos[1];
        double zd = track->points[in].pos[2] - track->points[i].pos[2];

        double clen = (sqrtf(xd * xd + yd * yd + zd * zd));
        len += clen;

        float alph1 = 1.0 - (fabs(len - trackPos) * dist);
        float alph2 = 1.0 - (fabs((len - track->len) - trackPos) * dist);
        float alph3 = 1.0 - (fabs((len + track->len) - trackPos) * dist);
        float alph = alph1;
        if(alph2 > alph) alph = alph2;
        if(alph3 > alph) alph = alph3;

        if(diff * clen > 0.5 && i > 0) {
            glColor4f(1, 1.5 - (diff * clen * 0.4), 0, alph);
        } else {
            glColor4f(0, 1, 1, alph);
        }

        if(alph < -0.1) {
            if(b) break;
            continue;
        }

        glTexCoord2f((len * lenScale) - realTime * 5, 0);
        glVertex3f(track->points[i].pos[0], track->points[i].pos[1], track->points[i].pos[2] - 2);
        glTexCoord2f((len * lenScale) - realTime * 5, 1);
        glVertex3f(track->points[i].pos[0], track->points[i].pos[1], track->points[i].pos[2]);
        
        if(b) {
            break;
        }
    }
    glEnd();
};

int track_getBouncers(Track *track, float pos, float side, double vel) {
    double p1[3], p2[3], start = 0;
    int i;
    int b = 0;
    for(i = 0; i < track->numPoints; i++) {
        int i2 = i + 1;
        int i1 = i;
        if(i2 == track->numPoints) {
            i2 = 0;
        } else if(i2 > track->numPoints) {
            i2 = 1;
        }
        if(i1 == track->numPoints) {
            i1 = 0;
        } else if(i1 > track->numPoints) {
            i1 = 1;
        }

        memcpy(p1, track->points[i1].pos, 3 * sizeof(double));
        memcpy(p2, track->points[i2].pos, 3 * sizeof(double));

        double dist = sqrtf((p1[0] - p2[0]) * (p1[0] - p2[0]) + (p1[1] - p2[1]) * (p1[1] - p2[1]) + (p1[2] - p2[2]) * (p1[2] - p2[2]));
        double next = start + dist;
        if(next < pos) {
            start = next;
        } else if(next < (pos + vel)) {
            b = track->points[i].bouncers;
            if(b) {
                break;
            }
        } else {
            break;
        }
    }
    if(b == 1 && side > 0) {
        return 1;
    } else if(b == 2 && side < 0) {
        return 1;
    } else if(b == 3) {
        return 1;
    }
    return 0;
};
