
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

#pragma once

#include <AL/al.h>
#include <AL/alc.h>

#include "vorbis/codec.h"
#include "vorbis/vorbisfile.h"

#define alSourcei myAlSourcei
#define alSourcef myAlSourcef
#define alSource3f myAlSource3f
#define alSourcePlay myAlSourcePlay

#define alListener3f myAlListener3f
#define alListenerfv myAlListenerfv

typedef struct {
    int bitStream;
    OggVorbis_File oggFile;
    ALenum format;
    ALsizei freq;
} SoundLoader;

typedef struct {
    SoundLoader *loader;
    ALuint source;
} SoundPlayer;

void sound_init(void);
void sound_deinit(void);
char *sound_getChunk(SoundLoader *loader, long *size);

SoundLoader *sound_getLoader(char *filename);
char *sound_loadData(SoundLoader *loader, ALsizei *size);
char *sound_loadChunk(SoundLoader *loader, ALsizei *size);

void sound_resetLoader(SoundLoader *loader);

SoundPlayer *sound_loadPlayer(char *end);
ALuint sound_load(char *filename, ALuint *buffer);

void myAlSourcePlay(ALuint source);
void myAlSourcei(ALuint, ALenum, ALint);
void myAlSourcef(ALuint, ALenum, ALfloat);
void myAlSource3f(ALuint, ALenum, ALfloat, ALfloat, ALfloat);

void myAlListener3f(ALenum, ALfloat, ALfloat, ALfloat);
void myAlListenerfv(ALenum, ALfloat*);
