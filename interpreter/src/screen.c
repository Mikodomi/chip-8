#include <stdlib.h>
#include <inttypes.h>
#include "screen.h"

int screen_create(chip8_screen* screen) {
    if (!screen) return -1;
    screen->pixels = calloc(32, sizeof(uint64_t));

    if (!screen->pixels) return -1;
    return 0;
}


uint8_t screen_get_byte(const chip8_screen* screen, int x, int y) {
    uint64_t row = screen->pixels[y];
    uint64_t mask = 0x00000000000000FF;
    if (x > 64-8) {
        mask >>= (x+8-64);
        mask &= row;
    } else {
        mask <<= (64-8 - x);
        mask &= row;
        mask >>= (64-8 - x);
    }
    return (uint8_t)mask;
}

uint64_t left_rotate(uint64_t in, int a) {
    a = a % 64;
    uint64_t temp = in >> (64 - a);
    in <<= a;
    return in | temp;
}

uint64_t right_rotate(uint64_t in, int a) {
    a = a % 64;
    uint64_t temp = in << (64 - a);
    in >>= a;
    return in | temp;
}

int screen_write_byte(chip8_screen* screen, uint8_t byte, int x, int y) {
    int changed = 0;
    uint64_t mask = 0;
    mask |= byte;
    uint64_t curr = screen->pixels[y];
    curr = left_rotate(curr, x+8);
    changed = ((uint8_t)curr & mask);
    curr ^= mask;
    curr = right_rotate(curr, x+8);    
    screen->pixels[y] = curr;
    return changed;
}

void screen_draw_byte(const chip8_screen* screen, SDL_Renderer* renderer, uint8_t byte, int x, int y) {
    uint8_t curr_pixel_pos = 0b10000000;
    uint8_t is_set;
    int i = x;
    do {
       is_set = curr_pixel_pos & byte;
       curr_pixel_pos >>= 1;
       i++;
       if (!is_set) continue;
       SDL_RenderPoint(renderer, i-1, y);
    } while (i < x+8 && i < 64);
}

void screen_draw(const chip8_screen* screen, SDL_Renderer* renderer) {
    SDL_SetRenderDrawColor(renderer, 0, 0, 0, SDL_ALPHA_OPAQUE); 
    SDL_RenderClear(renderer); 
    SDL_SetRenderDrawColor(renderer, 255, 255, 255, SDL_ALPHA_OPAQUE);
    for (int y = 0; y<32; y++) {
        for (int x = 0; x<8; x++) {
            screen_draw_byte(screen, renderer, screen_get_byte(screen, x*8, y), x*8, y);
        }
    }
    SDL_RenderPresent(renderer);
}

void screen_clear(chip8_screen* screen) {
    memset(screen->pixels, 0, 32*sizeof(uint64_t));
}

void screen_destroy(chip8_screen* screen) {
    if (screen->pixels) free(screen->pixels);
}
