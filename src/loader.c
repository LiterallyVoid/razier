
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

#include <string.h>
#include <unistd.h>

#include <pthread.h>

#include "main.h"
#include "loader.h"

#include "texture.h"
#include "model.h"

static struct {
    char name[128];
    int type;
    int data;
    void (*function)(Model *model);
} queue[128];

static int queueLength = 0, queueDisplayLength = 0;
static int queueItem = 0;

static double endTime = -1;

static pthread_t queueThread;
static pthread_mutex_t queueMutex;

static double barPos = 0.0;

void loader_screen(void) {
    if(loader_done()) {
        return;
    }
    double amt;
    if(queueDisplayLength == 0) {
        amt = 1;
    } else {
        amt = queueItem / (double) queueDisplayLength;
    }

    barPos -= (barPos - amt) * 0.2;

    static GLuint tex1, tex2;
    double texi;

    if(barPos < 0.333) {
        tex1 = tex_load("interface/loading_1.png");
        texi = 0;
    } else if(barPos < 0.666) {
        tex1 = tex_load("interface/loading_1.png");
        tex2 = tex_load("interface/loading_2.png");
        texi = (barPos - 0.333) * 30;
    } else {
        tex1 = tex_load("interface/loading_2.png");
        tex2 = tex_load("interface/loading_3.png");
        texi = (barPos - 0.666) * 30;
    }

    glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT | GL_STENCIL_BUFFER_BIT);
    glMatrixMode(GL_PROJECTION);
    glLoadIdentity();
    glMatrixMode(GL_MODELVIEW);
    glLoadIdentity();

    glBindTexture(GL_TEXTURE_2D, tex1);
    glBegin(GL_TRIANGLE_STRIP);
    glColor4f(1, 1, 1, 1);
    glTexCoord2f(0, 0);
    glVertex2f(-0.5, 0.5);
    glTexCoord2f(0, 1);
    glVertex2f(-0.5, -0.5);
    glTexCoord2f(1, 0);
    glVertex2f(0.5, 0.5);
    glTexCoord2f(1, 1);
    glVertex2f(0.5, -0.5);
    glEnd();

    glBindTexture(GL_TEXTURE_2D, tex2);
    glBegin(GL_TRIANGLE_STRIP);
    glColor4f(1, 1, 1, texi);
    glTexCoord2f(0, 0);
    glVertex2f(-0.5, 0.5);
    glTexCoord2f(0, 1);
    glVertex2f(-0.5, -0.5);
    glTexCoord2f(1, 0);
    glVertex2f(0.5, 0.5);
    glTexCoord2f(1, 1);
    glVertex2f(0.5, -0.5);
    glEnd();

    glBindTexture(GL_TEXTURE_2D, tex_load("textures/track_map.png"));
    glBegin(GL_TRIANGLE_STRIP);
    float i;
    if(amt > 0) {
        for(i = -1; i <= -1 + barPos * 2; i += barPos * 1.9999) {
            glColor3f(0, 1, 1);
            glTexCoord2f(i * 15 - (globalTime * 3), 0);
            glVertex2f(i, -0.9);
            glColor3f(0, 1, 1);
            glTexCoord2f(i * 15 - (globalTime * 3), 1);
            glVertex2f(i, -1.0);
        }
    }
    glEnd();
};

void loader_clearQueue(void) {
    queueLength = queueItem = queueDisplayLength = 0;
};

void loader_queue(char *name, int type, int data, void (*function)(Model *model)) {
    pthread_mutex_lock(&queueMutex);
    int i;
    for(i = 0; i < queueLength; i++) {
        if(!strcmp(queue[i].name, name)) {
            pthread_mutex_unlock(&queueMutex);
            return;
        }
    }
    strncpy(queue[queueLength].name, name, 128);
    queue[queueLength].type = type;
    queue[queueLength].data = data;
    queue[queueLength].function = function;

    if(type == LT_MODEL) {
        //queueDisplayLength += data;
    } else if(type == LT_TEXTURE) {
        if(data == -2) {
            queueDisplayLength--;
        }
    }
    queueLength++;
    queueDisplayLength++;
    pthread_mutex_unlock(&queueMutex);
};

int loader_runQueue(void) {
    pthread_mutex_lock(&queueMutex);
    if(queueItem == queueLength) {
        endTime = globalTime + 1;
        printf("%d %d\n", queueLength, queueDisplayLength);
        loader_clearQueue();
        pthread_mutex_unlock(&queueMutex);
        return 1;
    }

    printf("Loading %s\n", queue[queueItem].name);

    char *name = queue[queueItem].name;
    int data = queue[queueItem].data;

    pthread_mutex_unlock(&queueMutex);
    switch(queue[queueItem].type) {
    case LT_TEXTURE: {
        tex_load(name);
        break;
    }
    case LT_MODEL: {
        Model *m = model_load(name, data);
        queue[queueItem].function(m);
        break;
    }
    };

    //usleep(800000);

    pthread_mutex_lock(&queueMutex);
    queueItem++;
    pthread_mutex_unlock(&queueMutex);
    return 0;
};

void *loadingFunc(void *arg) {
    while(!loader_runQueue());
    return NULL;
};

void loader_loadAll(void) {
    barPos = 0;
    if(pthread_create(&queueThread, NULL, loadingFunc, NULL)) {
        printf("Could not create thread. Proceeding without threading.\n");
        loadingFunc(NULL);
    }
};

int loader_done(void) {
    if(globalTime < endTime) {
        return 0;
    }

    pthread_mutex_lock(&queueMutex);
    int done = queueLength == 0;
    pthread_mutex_unlock(&queueMutex);
    return done;
};
