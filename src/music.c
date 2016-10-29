
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

#include <stdio.h>
#include <stdlib.h>

#include "sound.h"
#include "music.h"

static struct {
    SoundPlayer *player;
    ALuint buffers[2];
} musics[MUSIC_NUM];
static int currentMusic = -1;

void music_stream(ALuint buffer);

void music_load(char *name, int track) {
    musics[track].player = sound_loadPlayer(name);
    //alSourcei(musics[track].player->source, AL_LOOPING, AL_TRUE);
    alSourcef(musics[track].player->source, AL_GAIN, 1.0);

    alGenBuffers((ALuint) 2, &musics[track].buffers[0]);

    int music = currentMusic;
    currentMusic = track;
    currentMusic = music;
};

void music_update(void) {
    ALint processed;
    alGetSourcei(musics[currentMusic].player->source, AL_BUFFERS_PROCESSED, &processed);

    while(processed--) {
        ALuint buffer;
        alSourceStop(musics[currentMusic].player->source);
        alSourceUnqueueBuffers(musics[currentMusic].player->source, 1, &buffer);
        alSourcePlay(musics[currentMusic].player->source);

        /*int num;
        for(num = 0; num < 2; num++) {
            if(musics[currentMusic].buffers[num] == buffer) {
                break;
            }
        }*/

        music_stream(buffer);
    }
};

void music_stream(ALuint buffer) {
    ALsizei size;

    char *data = sound_loadChunk(musics[currentMusic].player->loader, &size);

    alBufferData(buffer, musics[currentMusic].player->loader->format, data, size, musics[currentMusic].player->loader->freq);
    alSourceQueueBuffers(musics[currentMusic].player->source, 1, &buffer);

    free(data);
};

void music_play(int track) {
    if(currentMusic != -1) {
        music_stop();
    }

    if(musics[track].player->source > 0) {
        sound_resetLoader(musics[track].player->loader);
        currentMusic = track;

        music_stream(musics[track].buffers[0]);
        music_stream(musics[track].buffers[1]);

        alSourcePlay(musics[track].player->source);
    }
};

void music_stop(void) {
    if(currentMusic != -1) {
        alSourceStop(musics[currentMusic].player->source);
        
        ALint processed;
        alGetSourcei(musics[currentMusic].player->source, AL_BUFFERS_PROCESSED, &processed);
        
        while(processed--) {
            ALuint buffer;
            alSourceUnqueueBuffers(musics[currentMusic].player->source, 1, &buffer);
        }
    }

    currentMusic = -1;
};

void music_deinit(void) {
    music_stop();
    int i;
    for(i = 0; i < MUSIC_NUM; i++) {
        if(musics[i].player->source) {
            alSourcei(musics[i].player->source, AL_BUFFER, 0);
            alDeleteSources(1, &musics[i].player->source);
            alDeleteBuffers(1, &musics[i].buffers[0]);
            alDeleteBuffers(1, &musics[i].buffers[1]);
        }
    }
};
