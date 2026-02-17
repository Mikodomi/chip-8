#include "sound.h"
#include <SDL3/SDL.h>

int create_audio(audio* aud) {
    if (!SDL_WasInit(SDL_INIT_AUDIO)) return -1;
    if (!SDL_LoadWAV("square.wav", &aud->spec, &aud->audiobuf, &aud->audiolen)) {
        SDL_Log("Couldn't load WAV file: %s\n", SDL_GetError());
        return -1;
    }
    aud->stream = SDL_OpenAudioDeviceStream(SDL_AUDIO_DEVICE_DEFAULT_PLAYBACK, &aud->spec, NULL, NULL);
    if (!aud->stream) {
        SDL_Log("Couldn't open audio stream: %s\n", SDL_GetError());
        return -1;
    }
    if (!SDL_PutAudioStreamData(aud->stream, aud->audiobuf, aud->audiolen)) {
        SDL_Log("Couldn't put audio stream data: %s\n", SDL_GetError());
        return -1;
    }
    return 0;
}

int unpause_audio(audio* aud) {
    if (!SDL_ResumeAudioStreamDevice(aud->stream)) {
        SDL_Log("Couldn't resume audio stream: %s\n", SDL_GetError());
        return -1;
    }
    return 0;
}

int pause_audio(audio* aud) {
    if (!SDL_PauseAudioStreamDevice(aud->stream)) {
        SDL_Log("Couldn't pause audio stream: %s\n", SDL_GetError());
        return -1;
    }
    return 0;
}
int restart_audio(audio* aud) {
    if (SDL_GetAudioStreamQueued(aud->stream) < 100) {
        if (!SDL_PutAudioStreamData(aud->stream, aud->audiobuf, aud->audiolen)) {
            SDL_Log("Couldn't put audio stream data: %s\n", SDL_GetError());
            return -1;
        }
    }
    return 0;
}

void destroy_audio(audio* aud) {
    if (aud->stream) SDL_DestroyAudioStream(aud->stream);
    if (aud->audiobuf) SDL_free(aud->audiobuf);
}
