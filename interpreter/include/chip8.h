#ifndef CHIP8_H
#define CHIP8_H
#include <inttypes.h>

#include "screen.h"
#include "sound.h"

#define MEM_SIZE 4096
#define REG_COUNT 16

static struct {
    int disassemble;
    char* filename;
} options;

typedef struct {
    // CHIP-8 logic
    uint8_t         mem[MEM_SIZE];
    uint8_t         *stack_pointer;
    uint8_t         v[REG_COUNT]; // data registers
    uint16_t        address; // 12-bit address register
    uint16_t        pc; // program counter
    uint8_t         delay_timer;
    uint8_t         sound_timer;
    chip8_screen    screen;
    audio           sound;
    uint8_t         last_pressed;

    // SDL
    SDL_Window*     sdl_window;
    SDL_Renderer*   sdl_renderer;
    SDL_TimerID     sdl_timerID;

    uint16_t size;
} chip8;

int chip8_init(chip8* machine, char* title);
void chip8_destroy(chip8* machine);

int chip8_load_rom(chip8* machine, FILE* rom_input, FILE* fonts_input);

int chip8_fde_cycle(chip8* machine); // fetch decode execute

int chip8_decode_execute(chip8* machine, uint16_t instruction);

int chip8_decode_zeroes(chip8* machine, uint16_t instruction);
int chip8_decode_2reg(chip8* machine, uint16_t instruction);
int chip8_draw(chip8* machine, int reg1, int reg2, int value);
int chip8_E_instructions(chip8* machine, uint16_t instruction);
int chip8_F_instructions(chip8* machine, uint16_t instruction);
int chip8_poll_keypress(chip8* machine);

// SDL Timer callback
Uint32 chip8_decrease_timers(void* userdata, SDL_TimerID time, Uint32 interval);

uint8_t quit(chip8* machine);
int opts_init(int argc, char **argv);
uint8_t get_press_value(SDL_Keycode key);

#endif
