
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

#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include "main.h"
#include "model.h"
#include "texture.h"
#include "loader.h"

Model *model_load(const char *end, int numSkins) {
    char concat[] = "data/";
    char *path = malloc(strlen(end) + strlen(concat) + 1);
    strcpy(path, concat);
    strcat(path, end);

    Vertex vert;
    
    FILE *fp = fopen(path, "rb");
    if(fp == NULL) {
        perror("fopen");
        fprintf(stderr, "(while opening file %s)\n", path);
        free(path);
        return NULL;
    }
    
    Model *model = malloc(sizeof(Model));
    int i;
    for(i = 0; i < 9; i++) {
        model->drawn[i] = 0;
    }
    model->vertices = malloc(sizeof(Vertex));
    model->numVerts = 0;
    int allocVerts = 1;

    while(1) {
        int num = fscanf(fp, "%f %f %f %f %f %f %f %f %f %f %f %63s", &vert.pos[0], &vert.pos[1], &vert.pos[2], &vert.norm[0], &vert.norm[1], &vert.norm[2], &vert.uv[0], &vert.uv[1], &vert.color[0], &vert.color[1], &vert.color[2], vert.material);
        if(num == 12) {
            int i;
            unsigned int j;
            for(j = 0; j < strlen(vert.material); j++) {
                if(vert.material[j] == '#') {
                    break;
                }
            }
            if(j == strlen(vert.material)) {
                j = 65;
            }
            vert.numberKey = j;
            for(i = 0; i < numSkins; i++) {
                vert.textures[i] = 0;
                if(j != 65) {
                    vert.material[j] = '1' + i;
                }
                loader_queue(vert.material, LT_TEXTURE, -2, NULL);
            }
            vert.uv[1] = 1 - vert.uv[1];
            model->vertices[model->numVerts] = vert;
            model->numVerts++;
            if(model->numVerts >= allocVerts) {
                allocVerts *= 2;
                model->vertices = realloc(model->vertices, sizeof(Vertex) * allocVerts);
            }
        } else {
            break;
        }
    }
    fclose(fp);

    free(path);
    return model;
};

void model_deinit(Model *model) {
    free(model->vertices);
    free(model);
};

void model_draw(Model *model, int skin) {
    model_drawLight(model, skin, ambientColor[0], ambientColor[1], ambientColor[2]);
};

void model_drawLight(Model *model, int skin, float r, float g, float b) {
    int i;
    GLuint oldTexture = -1;
    if(!model->drawn[skin]) {
        for(i = 0; i < model->numVerts; i++) {
            Vertex v = model->vertices[i];
            if(v.textures[skin] == 0) {
                if(v.numberKey != 65) {
                    v.material[v.numberKey] = '1' + skin;
                }
                model->vertices[i].textures[skin] = tex_load(v.material);
                v.textures[skin] = model->vertices[i].textures[skin];
            }
        }
        model->drawn[skin] = 1;
    }

    glBegin(GL_TRIANGLES);
    for(i = 0; i < model->numVerts; i++) {
        Vertex v = model->vertices[i];
        if(v.textures[skin] == 0) {
            if(v.numberKey != 65) {
                v.material[v.numberKey] = '1' + skin;
            }
            model->vertices[i].textures[skin] = tex_load(v.material);
            v.textures[skin] = model->vertices[i].textures[skin];
        }
        if(oldTexture != v.textures[skin] || i == 0) {
            oldTexture = v.textures[skin];
            glEnd();

            glBindTexture(GL_TEXTURE_2D, v.textures[skin]);

            glBegin(GL_TRIANGLES);
        }
        glTexCoord2fv(v.uv);
        glNormal3fv(v.norm);
        glColor3f(v.color[0] * r, v.color[1] * g, v.color[2] * b);
        glVertex3fv(v.pos);
    }
    glEnd();
};
