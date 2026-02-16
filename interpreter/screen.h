#ifndef CHIP8_SCREEN_H
#define CHIP8_SCREEN_H
#include <inttypes.h>
#include <SDL3/SDL.h>

struct chip8_screen_t {
    uint64_t* pixels; // assuming the screen is 64x32
};
typedef struct chip8_screen_t chip8_screen;


int screen_create(chip8_screen* screen);

uint8_t screen_get_byte(const chip8_screen* screen, int x, int y);
int screen_write_byte(chip8_screen* screen, uint8_t byte, int x, int y); // xors with byte

void screen_draw(const chip8_screen* screen, SDL_Renderer* renderer);

void screen_destroy(chip8_screen* screen);

#endif
