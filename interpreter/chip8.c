#include <SDL3/SDL.h>
#include <string.h>
#include <inttypes.h>
#include <stdio.h>
#include <time.h>
#include <stdlib.h>
#include <errno.h>

#define MEM_SIZE 4096
#define REG_COUNT 16
#define SCREEN_WIDTH 64
#define SCREEN_HEIGHT 32

// takes original 16 bits as input (in big endian order)
#define REG1(x) (x & 0x000F)
#define REG2(x) ((x & 0xF000) >> 8)
#define CONST_VALUE(x) ((x & 0xFF00) >> 8)
#define MEM_VALUE(x) (((x & 0xFF00) >> 8) | ((x & 0x000F) << 8))

typedef struct {
    // CHIP-8 logic
    uint8_t     mem[MEM_SIZE];
    uint8_t     *stack_pointer;
    uint8_t     v[REG_COUNT]; // data registers
    uint16_t    address; // 12-bit address register
    uint16_t    pc; // program counter
    uint8_t     delay_timer;
    uint8_t     sound_timer;
    uint8_t     *screen;

    // SDL
    SDL_Window*     sdl_window;
    SDL_Renderer*   sdl_renderer;
    SDL_Texture*    sdl_texture;
    SDL_Surface*    sdl_surface;

    uint16_t size;
} chip8;

int chip8_load_rom(chip8* machine, FILE* rom_input, FILE* fonts_input) {
    if (fonts_input) {
        fread(machine->mem, sizeof(char), 0x50, fonts_input); // 16 five byte sprites = 0x50
    }
    size_t bytes_read = fread(&machine->mem[0x200], sizeof(char), MEM_SIZE-0x200, rom_input);
    machine->size = bytes_read;
    return bytes_read;
}

int chip8_screen_pixel_xor(chip8* machine, uint8_t x, uint8_t y, uint8_t val) {
    if (x >= SCREEN_WIDTH || y >= SCREEN_HEIGHT) return -1;
    if (!machine->screen) return -1;
    machine->screen[y*SCREEN_WIDTH+x] ^= val;
    return val;
}

int chip8_decode_2reg(chip8* machine, uint16_t instruction) {
    size_t reg1 = REG1(instruction), reg2 = REG2(instruction);
    uint8_t val1 = machine->v[reg1], val2 = machine->v[reg2];
    uint8_t operation = (instruction & 0x0F00) >> 8;
    switch (operation) {
        case 0:
            val1 = val2;
            break;
        case 1:
            val1 |= val2;
            break;
        case 2:
            val1 &= val2;
            break;
        case 3:
            val1 ^= val2;
            break;
        case 4:
            val1 += val2;
            machine->v[0x0F] = (val1 + val2) > 0xFF;
            break;
        case 5:
            machine->v[0x0F] = (val1 >= val2);
            val1 -= val2;
            break;
        case 6:
            val1 = val2;
            machine->v[0x0F] = (val1 & 0x01);
            val1 >>= 1;
            break;
        case 7:
            machine->v[0x0F] = (val2 >= val1);
            val1 = val2 - val1;
            break;
        case 0x0E:
            val1 = val2;
            machine->v[0x0F] = (val1 & 0x80);
            val1 <<= 1;
            break;
        default:
            return -1;
    }
    machine->v[reg1] = val1;
    return 0;
}

int chip8_decode_zeroes(chip8* machine, uint16_t instruction) {
    int status = 0;
    uint8_t bottom_bits = MEM_VALUE(instruction);
    switch (bottom_bits) {
        case 0x0E0:
            break;
        case 0x0EE:
            machine->pc = (*machine->stack_pointer) / 2;
            *machine->stack_pointer -= 2;
            break;
        default:
            machine->pc = bottom_bits / 2;
            break; // what does "jump to native assembler subroutine" even mean?
    }
    return status;
}

int chip8_F_instructions(chip8* machine, uint16_t instruction) {
    int status = 0;
    size_t reg1 = REG1(instruction);
    uint16_t value = CONST_VALUE(instruction);
    uint8_t temp;
    switch (value) {
        case 0x07:
            machine->v[reg1] = machine->delay_timer;
            break;
        case 0x0A: break;
        case 0x15: 
            machine->delay_timer = machine->v[reg1];
            break;
        case 0x18: 
            machine->sound_timer = machine->v[reg1];
            break;
        case 0x1E: 
            machine->address += machine->v[reg1];
            break;
        case 0x29: break;
        case 0x33: 
            temp = machine->v[reg1];
            (machine->mem[machine->address+2]) = temp % 10;
            temp /= 10;
            (machine->mem[machine->address+1]) = temp % 10;
            temp /= 10;
            (machine->mem[machine->address]) = temp % 10;
            break;
        case 0x55: 
            for (int i = 0; i<=reg1; i++) {
                machine->mem[machine->address+2*i] = machine->v[i];
            }
            break;
        case 0x65: 
            for (int i = 0; i<=reg1; i++) {
                machine->v[i] = machine->mem[machine->address+2*i];
            }
            break;
        default:
            status = -1;
    }
    return status;
}

int chip8_decode_execute(chip8* machine, uint16_t instruction) {
    int status = 0;
    size_t reg1, reg2;
    uint8_t value;
    uint8_t first_4_bits = (instruction & 0x00F0) >> 4;
    uint8_t zero;
    uint16_t addr;
    switch (first_4_bits) {
        case 0: 
            status = chip8_decode_zeroes(machine, instruction);
            break;
        case 1: 
            addr = MEM_VALUE(instruction);
            machine->pc = addr / 2;
            break;
        case 2: 
            *machine->stack_pointer += 2;
            *machine->stack_pointer = machine->pc * 2;
            addr = MEM_VALUE(instruction);
            machine->pc = addr / 2;
            break;
        case 3: 
            reg1 = REG1(instruction);
            value = CONST_VALUE(instruction);
            machine->pc += (machine->v[reg1] == value);
            break;
        case 4: 
            reg1 = REG1(instruction);
            value = CONST_VALUE(instruction);
            machine->pc += (machine->v[reg1] != value);
            break;
        case 5: 
            zero = (instruction & 0x0F00) >> 8;
            if (zero != 0) { status = -1; break; }
            reg1 = REG1(instruction);
            reg2 = REG2(instruction);
            machine->pc += (machine->v[reg1] == machine->v[reg2]);
            break;
        case 6: 
            reg1 = REG1(instruction);
            value = CONST_VALUE(instruction); 
            machine->v[reg1] = value;
            break;
        case 7: 
            reg1 = REG1(instruction);
            value = CONST_VALUE(instruction);
            machine->v[reg1] += value;
            break;
        case 8: 
            status = chip8_decode_2reg(machine, instruction);
            break;
        case 9: 
            zero = (instruction & 0x0F00) >> 8;
            if (zero != 0) { status = -1; break; }
            reg1 = REG1(instruction);
            reg2 = REG2(instruction);
            machine->pc += (machine->v[reg1] != machine->v[reg2]);
            break;
        case 0xA: 
            addr = MEM_VALUE(instruction);      
            machine->address = addr;
            break;
        case 0xB: 
            addr = MEM_VALUE(instruction) + machine->v[0];
            machine->pc = addr / 2;
            break;
        case 0xC: 
            reg1 = REG1(instruction);
            value = CONST_VALUE(instruction);
            machine->v[reg1] = rand() & value;
            break;
        case 0xD: 
            break;
        case 0xE: break;
        case 0xF: 
            chip8_F_instructions(machine, instruction);
            break;

        default:
            status = -1;
    }
    return status;
}


int chip8_fde_cycle(chip8* machine) { // fetch decode execute
    int status = 0;
    uint16_t instruction;
    for (;2*machine->pc < machine->size;) {
        instruction = ((uint16_t*)machine->mem)[machine->pc]; 
        machine->pc++;
        status = chip8_decode_execute(machine, instruction);
        if (status != 0) break;
    }
    return status;
}

void chip8_decrease_timer(chip8* machine) {
    if (machine->delay_timer == 0) return;
    machine->delay_timer--;
}

int chip8_init(chip8* machine, char* title) {
    srand(time(NULL)); 
    memset(machine, 0, sizeof(chip8));

    machine->pc = 0x200;
    machine->stack_pointer = &machine->mem[0xEA0];
    machine->delay_timer = 0;

    machine->screen = calloc(SCREEN_WIDTH/8*SCREEN_HEIGHT, sizeof(uint8_t));

    if (!machine->screen) return -1;
    // work in progress
    /* if (!SDL_CreateWindowAndRenderer(title, 
                SCREEN_HEIGHT, SCREEN_WIDTH,
                SDL_WINDOW_RESIZABLE,
                &machine->sdl_window, &machine->sdl_renderer)) {
        SDL_LogError(SDL_LOG_CATEGORY_APPLICATION, "%s", SDL_GetError());
        return -1;       
    }
    machine->sdl_surface = SDL_CreateSurface(SCREEN_WIDTH, SCREEN_HEIGHT, SDL_PIXELFORMAT_INDEX1MSB);
    if (!machine->sdl_surface) {
        SDL_LogError(SDL_LOG_CATEGORY_APPLICATION, "createsurface: %s", SDL_GetError());
        return -1;
    }

    machine->sdl_texture = SDL_CreateTextureFromSurface(machine->sdl_renderer, machine->sdl_surface);
    if (!machine->sdl_texture) {
        SDL_LogError(SDL_LOG_CATEGORY_APPLICATION, "createtexture: %s", SDL_GetError());
        return -1;
    } */
    return 0;
}

void chip8_destroy(chip8* machine) {
    if (SDL_WasInit(SDL_INIT_VIDEO)) {
        if (machine->sdl_surface) {
            SDL_DestroySurface(machine->sdl_surface);
        }
        if (machine->sdl_texture) {
            SDL_DestroyTexture(machine->sdl_texture);
        }
        if (machine->sdl_renderer)  {
            SDL_DestroyRenderer(machine->sdl_renderer);
        }
        if (machine->sdl_window) {
            SDL_DestroyWindow(machine->sdl_window);
        }
    }
    if (machine->screen) free(machine->screen);
}

int main(int argc, char **argv) {
    if (argc < 2) {
        printf("usage: %s <rom_name>\n", argv[0]);
        return -1;
    }
    FILE* rom = fopen(argv[1], "rb");
    if (rom == NULL) {
        perror("fopen");
        return -1;
    }
    FILE* fonts = fopen("font.ch8", "rb");
    if (fonts == NULL) {
        perror("fopen");
        fprintf(stderr, "font file not found - fonts not available");
    }

    if (!SDL_Init(SDL_INIT_VIDEO)) {
        fclose(rom);
        return -1;
    }
    chip8 machine;
    if (chip8_init(&machine, argv[1]) < 0) {
        fprintf(stderr, "error during initialization\n");
        fclose(rom);
        chip8_destroy(&machine);
        return -1;
    }
    chip8_load_rom(&machine, rom, fonts);
    fclose(rom);
    fclose(fonts);

    chip8_fde_cycle(&machine);

    chip8_destroy(&machine);
    SDL_Quit();
    return 0;
}
