
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
#include <stdlib.h>
#include <stdio.h>
#include <string.h>

#define STB_IMAGE_IMPLEMENTATION
#include "stb/stb_image.h"

#include "texture.h"
#include "loader.h"

#define NUM_TEXTURES 32

static struct {
    int valid;
    char *name;
    GLuint id;
    unsigned char *data;
    int width, height;
} loadedTextures[NUM_TEXTURES];

GLuint tex_load(const char *end) {
    char concat[] = "data/";
    char *path = malloc(strlen(end) + strlen(concat) + 1);
    strcpy(path, concat);
    strcat(path, end);

    int i;
    for(i = 0; i < NUM_TEXTURES; i++) {
        if(loadedTextures[i].valid == 0) {
            break;
        }
        if(!strcmp(loadedTextures[i].name, path)) {
            if(loadedTextures[i].valid == 1 && loader_done()) {
                GLuint id = 0;
                glGenTextures(1, &id);
                loadedTextures[i].id = id;

                if(id == 0) {
                    return 0;
                }

                glBindTexture(GL_TEXTURE_2D, id);
                
                glTexImage2D(GL_TEXTURE_2D, 0, GL_RGBA, loadedTextures[i].width, loadedTextures[i].height, 0, GL_RGBA, GL_UNSIGNED_BYTE, loadedTextures[i].data);
                
                glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_NEAREST);
                glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_NEAREST);
                
                glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_REPEAT);
                glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_REPEAT);
                
                loadedTextures[i].valid = 2;
                stbi_image_free(loadedTextures[i].data);
            }
            return loadedTextures[i].id;
        }
    }
    
    int width, height;
    unsigned char *data = stbi_load(path, &width, &height, NULL, 4);
    if(data == NULL) {
        loadedTextures[i].name = malloc(strlen(path) + 1);
        strcpy(loadedTextures[i].name, path);
        loadedTextures[i].id = 0;
        loadedTextures[i].valid = -1;
        printf("could not load %s\n", path);
        free(path);
        return 0;
    }

    loadedTextures[i].name = malloc(strlen(path) + 1);
    strcpy(loadedTextures[i].name, path);
    loadedTextures[i].id = 0;
    loadedTextures[i].data = data;
    loadedTextures[i].width = width;
    loadedTextures[i].height = height;
    loadedTextures[i].valid = 1;
    free(path);

    printf("waat\n");
    return 0;
};

void tex_makeSmooth(GLuint tex) {
    glBindTexture(GL_TEXTURE_2D, tex);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
};
