
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

#include <AL/al.h>
#include <AL/alc.h>

#include <stdlib.h>
#include <string.h>

#include "vorbis/codec.h"
#include "vorbis/vorbisfile.h"

#include "sound.h"

#undef alSourcei
#undef alSourcef
#undef alSource3f
#undef alSourcePlay

#undef alListener3f
#undef alListenerfv

#define NUM_BUFFERS 128
#define BUFFER_SIZE 65536

static ALCdevice *audioDevice;
static ALCcontext *audioContext;
static int audio;

void sound_init(void) {
    audioDevice = alcOpenDevice(NULL);
    if(!audioDevice) {
        printf("Could not open audio device. No audio for you!\n");
        audio = 0;
        return;
    }
    alGetError();

    audioContext = alcCreateContext(audioDevice, NULL);
    if(!audioContext) {
        printf("Could not create OpenAL context. No audio for you!\n");
        audio = 0;
        return;
    }

    audio = 1;

    alcMakeContextCurrent(audioContext);

    alDopplerVelocity(0.5);
};

void sound_deinit(void) {
    alcCloseDevice(audioDevice);
};

char *sound_getChunk(SoundLoader *loader, long *size) {
    int endian = 0;

    char *buffer = malloc(BUFFER_SIZE);
    *size = ov_read(&loader->oggFile, buffer, BUFFER_SIZE, endian, 2, 1, &loader->bitStream);
    return buffer;
};

SoundLoader *sound_getLoader(char *filename) {
    if(!audio) {
        return NULL;
    }

    SoundLoader *loader = malloc(sizeof(SoundLoader));

    FILE *f = fopen(filename, "rb");

    vorbis_info *pInfo;
    OggVorbis_File oggFile;
    loader->bitStream = 0;

    ov_open(f, &oggFile, NULL, 0);

    pInfo = ov_info(&oggFile, -1);

    if(pInfo->channels == 1) loader->format = AL_FORMAT_MONO16;
    else loader->format = AL_FORMAT_STEREO16;

    loader->freq = pInfo->rate;

    loader->oggFile = oggFile;

    /*do {
        bytes = ov_read(&oggFile, smallBuffer, BUFFER_SIZE, endian, 2, 1, &bitStream);

        buffer = realloc(buffer, allocBuffer + bytes);
        memcpy(buffer + allocBuffer, smallBuffer, bytes);
        allocBuffer += bytes;
    } while(bytes > 0);
    ov_clear(&oggFile);
    *size = allocBuffer;
    return buffer;
    */

    return loader;
};

char *sound_loadData(SoundLoader *loader, ALsizei *size) {
    ALsizei allocBytes = 0;
    char *buffer = NULL;

    long chunkSize;

    do {
        char *currentBuffer = sound_getChunk(loader, &chunkSize);
        buffer = realloc(buffer, allocBytes + chunkSize);
        memcpy(buffer + allocBytes, currentBuffer, chunkSize);
        allocBytes += chunkSize;
        free(currentBuffer);
    } while(chunkSize > 0);

    *size = allocBytes;

    return buffer;
};

char *sound_loadChunk(SoundLoader *loader, ALsizei *size) {
    ALsizei allocBytes = 0;
    char *buffer = NULL;

    long chunkSize;

    do {
        char *currentBuffer = sound_getChunk(loader, &chunkSize);
        buffer = realloc(buffer, allocBytes + chunkSize);
        memcpy(buffer + allocBytes, currentBuffer, chunkSize);
        allocBytes += chunkSize;
        free(currentBuffer);
        if(chunkSize == 0) {
            sound_resetLoader(loader);
        }
    } while(allocBytes < BUFFER_SIZE);

    *size = allocBytes;

    return buffer;
};

void sound_resetLoader(SoundLoader *loader) {
    ov_pcm_seek(&loader->oggFile, 0);
};

SoundPlayer *sound_loadPlayer(char *end) {
    if(!audio) {
        return NULL;
    }

    char concat[] = "data/";
    char *filename = malloc(strlen(end) + strlen(concat) + 1);
    strcpy(filename, concat);
    strcat(filename, end);

    SoundPlayer *player = malloc(sizeof(SoundPlayer));

    player->loader = sound_getLoader(filename);

    ALuint source;
    alGenSources((ALuint) 1, &source);

    alSourcef(source, AL_PITCH, 1);
    alSourcef(source, AL_GAIN, 0.2);

    alSourcef(source, AL_REFERENCE_DISTANCE, 32.0);

    player->source = source;

    free(filename);

    return player;
};

ALuint sound_load(char *end, ALuint *buf) {
    SoundPlayer *player = sound_loadPlayer(end);

    ALuint buffer;

    alGenBuffers((ALuint) 1, &buffer);

    ALsizei size;
    char *data;

    data = sound_loadData(player->loader, &size);
    alBufferData(buffer, player->loader->format, data, (ALsizei) size, player->loader->freq);
    free(data);

    alSourcei(player->source, AL_BUFFER, buffer);

    *buf = buffer;

    ALuint source = player->source;

    free(player);

    return source;
};

void myAlSourcePlay(ALuint source) {
    if(source == 0) {
        return;
    }
    alSourcePlay(source);
};

void myAlSourcei(ALuint source, ALenum arg1, ALint arg2) {
    if(source == 0) {
        return;
    }
    alSourcei(source, arg1, arg2);
};

void myAlSourcef(ALuint source, ALenum arg1, ALfloat arg2) {
    if(source == 0) {
        return;
    }
    alSourcef(source, arg1, arg2);
};

void myAlSource3f(ALuint source, ALenum arg1, ALfloat arg2, ALfloat arg3, ALfloat arg4) {
    if(source == 0) {
        return;
    }
    alSource3f(source, arg1, arg2, arg3, arg4);
};


void myAlListener3f(ALenum arg1, ALfloat arg2, ALfloat arg3, ALfloat arg4) {
    if(!audio) {
        return;
    }
    alListener3f(arg1, arg2, arg3, arg4);
};

void myAlListenerfv(ALenum arg1, ALfloat *arg) {
    if(!audio) {
        return;
    }
    alListenerfv(arg1, arg);
};
