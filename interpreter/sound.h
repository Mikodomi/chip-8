#ifndef CHIP8_SOUND_H
#define CHIP8_SOUND_H
#include <SDL3/SDL.h>

struct chip8_audio {
    SDL_AudioStream* stream; 
    SDL_AudioSpec spec;
    Uint8* audiobuf;
    Uint32 audiolen;
};
typedef struct chip8_audio audio;

int create_audio(audio* aud);

int unpause_audio(audio* aud);
int pause_audio(audio* aud);

int restart_audio(audio* aud);

void destroy_audio(audio* aud);
#endif
