#include <SDL3/SDL.h>
#include <string.h>
#include <inttypes.h>
#include <stdio.h>
#include <time.h>
#include <stdlib.h>
#include <errno.h>
#include <unistd.h>

#include "chip8.h"
#include "sound.h"
#include "screen.h"

#define MEM_SIZE 4096
#define REG_COUNT 16
#define SCREEN_WIDTH 64
#define SCREEN_HEIGHT 32
#define TIMER_FREQUENCY 60
#define TICK_SPEED 500

// takes original 16 bits as input (in big endian order)
#define REG1(x) (x & 0x000F)
#define REG2(x) ((x & 0xF000) >> 12)
#define CONST_VALUE(x) ((x & 0xFF00) >> 8)
#define MEM_VALUE(x) (((x & 0xFF00) >> 8) | ((x & 0x000F) << 8))

Uint32 chip8_decrease_timers(void* userdata, SDL_TimerID time, Uint32 interval) {
    chip8* machine = (chip8*)userdata;    
    if (machine->delay_timer != 0) machine->delay_timer--;
    if (machine->sound_timer > 0) {
        machine->sound_timer--;
    } else if (machine->sound_timer == 0) { 
        if (pause_audio(&machine->sound) < 0) return ERR_AUDIO_PAUSE;
        machine->sound_timer--;
    }
    return 1000/TIMER_FREQUENCY;
}

void chip8_handle_error(const chip8* machine, uint16_t instruction, error_t status) {
    switch (status) {
        case SUCCESS: break; // should never get here
        case ERR_INIT_SDL:
            SDL_Log("error during sdl initialization");
            break;
        case ERR_INIT_SCREEN:
            SDL_Log("error during screen initialization");
            break;
        case ERR_INIT_AUDIO:
            SDL_Log("error during audio initialization");
            break;
        case ERR_AUDIO_PAUSE:
        case ERR_AUDIO_UNPAUSE:
            SDL_Log("error during audio un/pause");
            break;
        case ERR_INSTR_INVALID:
            SDL_Log("invalid instruction at %x: %x\n", machine->pc*2, instruction);
        case ERR_QUIT: break; // is okay
        default: printf("unknown error LOL");
    }
}

error_t chip8_init(chip8* machine, char* title) {
    srand(time(NULL)); 
    memset(machine, 0, sizeof(chip8));

    machine->pc = 0x100;
    machine->stack_pointer = &machine->mem[0x100];
    machine->delay_timer = 0;


    if(screen_create(&machine->screen) < 0) {
        return ERR_INIT_SCREEN; 
    }
    if (options.disassemble) return 0;
    if (!SDL_CreateWindowAndRenderer(title, 
                SCREEN_HEIGHT, SCREEN_WIDTH,
                SDL_WINDOW_RESIZABLE,
                &machine->sdl_window, &machine->sdl_renderer)) {
        SDL_LogError(SDL_LOG_CATEGORY_APPLICATION, "%s", SDL_GetError());
        return ERR_INIT_SDL;       
    }
    SDL_SetRenderLogicalPresentation(machine->sdl_renderer, SCREEN_WIDTH, SCREEN_HEIGHT, SDL_LOGICAL_PRESENTATION_LETTERBOX);

    if (create_audio(&machine->sound) < 0) {
        return ERR_INIT_AUDIO;
    }

    machine->sdl_timerID = SDL_AddTimer(1000/TIMER_FREQUENCY, chip8_decrease_timers, machine);

    return SUCCESS;
}

void chip8_destroy(chip8* machine) {
    SDL_RemoveTimer(machine->sdl_timerID);
    screen_destroy(&machine->screen);
    if (SDL_WasInit(SDL_INIT_AUDIO)) {
        destroy_audio(&machine->sound);
    }
    if (SDL_WasInit(SDL_INIT_VIDEO)) {
        if (machine->sdl_renderer)  {
            SDL_DestroyRenderer(machine->sdl_renderer);
        }
        if (machine->sdl_window) {
            SDL_DestroyWindow(machine->sdl_window);
        }
    }
}

int chip8_load_rom(chip8* machine, FILE* rom_input, FILE* fonts_input) {
    if (fonts_input) {
        fread(machine->mem, sizeof(char), 0x50, fonts_input); // 16 five byte sprites = 0x50
    }
    size_t bytes_read = fread(&machine->mem[0x200], sizeof(char), MEM_SIZE-0x200, rom_input);
    machine->size = bytes_read;
    return bytes_read;
}

SDL_Scancode get_value_scancode(int key) {
    switch (key) {
        case 0: return SDL_SCANCODE_0;
        case 1: return SDL_SCANCODE_1;
        case 2: return SDL_SCANCODE_2;
        case 3: return SDL_SCANCODE_3;
        case 4: return SDL_SCANCODE_4;
        case 5: return SDL_SCANCODE_5;
        case 6: return SDL_SCANCODE_6;
        case 7: return SDL_SCANCODE_7;
        case 8: return SDL_SCANCODE_8;
        case 9: return SDL_SCANCODE_9;
        case 0xA: return SDL_SCANCODE_A;
        case 0xB: return SDL_SCANCODE_B;
        case 0xC: return SDL_SCANCODE_C;
        case 0xD: return SDL_SCANCODE_D;
        case 0xE: return SDL_SCANCODE_E;
        case 0xF: return SDL_SCANCODE_F;
        default: break;
    }
    return -1;
}

int chip8_keyboard_is_pressed(const chip8* machine, int key) {
    int numkeys;
    const bool* key_states = SDL_GetKeyboardState(&numkeys);
    if (!key_states) return -1;
    return key_states[get_value_scancode(key)];
}

//void chip8_keyboard_set(chip8* machine, int key) {
//    machine->keyboard |= (0x1 << key);
//}
//
//void chip8_keyboard_unset(chip8* machine, int key) {
//    machine->keyboard &= (~(0x1 << key));
//}

error_t chip8_fde_cycle(chip8* machine) { // fetch decode execute
    error_t status = 0;
    uint16_t instruction;
    for (;2*(machine->pc-0x100) < machine->size; ) {
        usleep(1000000/TICK_SPEED);
        instruction = ((uint16_t*)machine->mem)[machine->pc]; 
        if (quit(machine)) return status;
        if (options.disassemble) {
            printf("%X:\t%X\n\t", machine->pc*2, instruction);
            chip8_decode_execute(machine, instruction);
            printf("\n");
            machine->pc++;
            return status;
        }
        if (restart_audio(&machine->sound) < 0) return -1;
        screen_draw(&machine->screen, machine->sdl_renderer);
        machine->pc++;
        status = chip8_decode_execute(machine, instruction);
        if (status != SUCCESS) {
            chip8_handle_error(machine, instruction, status);
            //break;
        }
    }
    return SUCCESS;
}

error_t chip8_decode_execute(chip8* machine, uint16_t instruction) {
    uint8_t reg1 = REG1(instruction), reg2 = REG2(instruction);
    uint8_t value = CONST_VALUE(instruction);
    uint8_t first_4_bits = (instruction & 0x00F0) >> 4;
    uint8_t zero;
    uint16_t addr = MEM_VALUE(instruction);
    switch (first_4_bits) {
        case 0: 
            return chip8_decode_zeroes(machine, instruction);
        case 1: 
            if (options.disassemble) { printf("jmp %X", addr); break; }
            machine->pc = addr / 2;
            break;
        case 2: 
            if (options.disassemble) { printf("call %X", addr); break; }
            machine->stack_pointer += 2;
            *(uint16_t*)machine->stack_pointer = machine->pc * 2;
            machine->pc = addr / 2;
            break;
        case 3: 
            if (options.disassemble) { printf("skpe v%X, %d", reg1, value); break; }
            machine->pc += (machine->v[reg1] == value);
            break;
        case 4: 
            if (options.disassemble) { printf("skpne v%X, %d", reg1, value); break; }
            machine->pc += (machine->v[reg1] != value);
            break;
        case 5: 
            if (options.disassemble) { printf("skpe v%X, v%X", reg1, reg2); break; }
            zero = (instruction & 0x0F00) >> 8;
            if (zero != 0) return ERR_INSTR_INVALID;
            machine->pc += (machine->v[reg1] == machine->v[reg2]);
            break;
        case 6: 
            if (options.disassemble) { printf("mov v%X, %d", reg1, value); break; }
            machine->v[reg1] = value;
            break;
        case 7: 
            if (options.disassemble) { printf("add v%X, %d", reg1, value); break; }
            machine->v[reg1] += value;
            break;
        case 8: 
            return chip8_decode_2reg(machine, instruction);
            break;
        case 9: 
            if (options.disassemble) { printf("skpne v%X, v%X", reg1, reg2); break; }
            zero = (instruction & 0x0F00) >> 8;
            if (zero != 0) return ERR_INSTR_INVALID;
            machine->pc += (machine->v[reg1] != machine->v[reg2]);
            break;
        case 0xA: 
            if (options.disassemble) { printf("mov I, %d", addr); break; }
            machine->address = addr;
            break;
        case 0xB: 
            if (options.disassemble) { printf("jv0, %d", addr); break; }
            addr = MEM_VALUE(instruction) + machine->v[0];
            machine->pc = addr / 2;
            break;
        case 0xC: 
            if (options.disassemble) { printf("rand v%X, %d", reg1, value); break; }
            machine->v[reg1] = rand() & value;
            break;
        case 0xD: 
            value = (instruction & 0x0F00) >> 8;
            if (options.disassemble) { printf("draw v%X, v%X, %d", reg1, reg2, value); break; }
            chip8_draw(machine, reg1, reg2, value);
            break;
        case 0xE: 
            return chip8_E_instructions(machine, instruction);
            break; 
        case 0xF: 
            return chip8_F_instructions(machine, instruction);
            break;
        default:
            return ERR_INSTR_INVALID;
    }
    return SUCCESS;
}

error_t chip8_decode_zeroes(chip8* machine, uint16_t instruction) {
    uint8_t bottom_bits = MEM_VALUE(instruction);
    switch (bottom_bits) {
        case 0x0E0:
            if (options.disassemble) { printf("clear"); break; }
            screen_clear(&machine->screen);
            break;
        case 0x0EE:
            if (options.disassemble) { printf("ret"); break; }
            machine->pc = (*(uint16_t*)machine->stack_pointer) / 2;
            machine->stack_pointer -= 2;
            break;
        default:
            if (options.disassemble) { printf("jnat %X", bottom_bits); break; }
            machine->pc = bottom_bits / 2;
            break; // what does "jump to native assembler subroutine" even mean?
    }
    return SUCCESS;
}

error_t chip8_decode_2reg(chip8* machine, uint16_t instruction) {
    uint8_t reg1 = REG1(instruction), reg2 = REG2(instruction);
    uint8_t val1 = machine->v[reg1], val2 = machine->v[reg2];
    uint8_t operation = (instruction & 0x0F00) >> 8;
    uint8_t flag = 0;
    switch (operation) {
        case 0:
            if (options.disassemble) { printf("mov v%X, v%X", reg1, reg2); break; }
            val1 = val2;                                                   
            break;                                                         
        case 1:                                                           
            if (options.disassemble) { printf("or v%X, v%X", reg1, reg2); break; }
            val1 |= val2;                                                  
            break;                                                         
        case 2:                                                            
            if (options.disassemble) { printf("and v%X, v%X", reg1, reg2); break; }
            val1 &= val2;                                                  
            break;                                                         
        case 3:                                                            
            if (options.disassemble) { printf("xor v%X, v%X", reg1, reg2); break; }
            val1 ^= val2;
            break;
        case 4:
            if (options.disassemble) { printf("add v%X, v%X", reg1, reg2); break; }
            flag = (val1 + val2) > 0xFF;
            val1 += val2;
            break;
        case 5:
            if (options.disassemble) { printf("sub v%X, v%X", reg1, reg2); break; }
            flag = (val1 >= val2);
            val1 -= val2;
            break;
        case 6:
            if (options.disassemble) { printf("msr v%X, v%X", reg1, reg2); break; }
            val1 = val2;
            flag = (val1 & 0x01);
            val1 >>= 1;
            break;
        case 7:
            if (options.disassemble) { printf("subs v%X, v%X", reg1, reg2); break; }
            flag = (val2 >= val1);
            val1 = val2 - val1;
            break;
        case 0x0E:
            if (options.disassemble) { printf("msl v%X, v%X", reg1, reg2); break; }
            val1 = val2;
            flag = (val1 & 0x80);
            flag >>= 7;
            val1 <<= 1;
            break;
        default:
            if (options.disassemble) { printf("UNKNOWN INSTRUCTION"); break; }
            return ERR_INSTR_INVALID;
    }
    machine->v[reg1] = val1;
    machine->v[0x0F] = flag;
    return SUCCESS;
}

void chip8_draw(chip8* machine, int reg1, int reg2, int value) {
    int x = machine->v[reg1];
    int y = machine->v[reg2];
    int changed = 0;
    for (int i = 0; i<value; i++) {
        changed |= screen_write_byte(&machine->screen, machine->mem[machine->address+i], x, y+i);
    }
    if (changed) {
        machine->v[0xF] = 1;
    }
}

error_t chip8_E_instructions(chip8* machine, uint16_t instruction) {
    int value = CONST_VALUE(instruction);
    int reg1 = REG1(instruction);
    int expected_pressed = machine->v[reg1];
    int is_pressed = chip8_keyboard_is_pressed(machine, expected_pressed);
    if (is_pressed < 0) return ERR_KEYBOARD;
    switch (value) {
        case 0x9E:
            if (options.disassemble) { printf("skpe v%X", reg1); break; }
            machine->pc += is_pressed;
            break;
        case 0xA1:
            if (options.disassemble) { printf("skpne v%X", reg1); break; }
            machine->pc += !is_pressed;
            break;
        default: 
            if (options.disassemble) { printf("UNKNOWN INSTRUCTION"); break; }
            return ERR_INSTR_INVALID;
    }
    return SUCCESS;
}


error_t chip8_F_instructions(chip8* machine, uint16_t instruction) {
    int key;
    uint8_t reg1 = REG1(instruction);
    uint16_t value = CONST_VALUE(instruction);
    uint8_t temp;
    switch (value) {
        case 0x07:
            if (options.disassemble) { printf("mov v%X, dtm", reg1); break; }
            machine->v[reg1] = machine->delay_timer;
            break;
        case 0x0A: 
            if (options.disassemble) { printf("in, v%X", reg1); break; }
            machine->v[reg1] &= 0xFFF0;
            key = chip8_poll_keypress(machine);
            if (key == 0xFF) return ERR_QUIT;
            machine->v[reg1] |= key;
            break;
        case 0x15: 
            if (options.disassemble) { printf("mov dtm, v%X", reg1); break; }
            machine->delay_timer = machine->v[reg1];
            break;
        case 0x18: 
            if (options.disassemble) { printf("mov stm, v%X", reg1); break; }
            machine->sound_timer = machine->v[reg1];
            return unpause_audio(&machine->sound);
            break;
        case 0x1E: 
            if (options.disassemble) { printf("add I, v%X", reg1); break; }
            machine->address += machine->v[reg1];
            break;
        case 0x29: 
            if (options.disassemble) { printf("hh5 v%X", reg1); break; }
            // my fonts start at address 0
            machine->address = (machine->v[reg1] & 0x00FF) * 5;
            break;
        case 0x33: 
            if (options.disassemble) { printf("bcd v%X", reg1); break; }
            temp = machine->v[reg1];
            (machine->mem[machine->address+2]) = temp % 10;
            temp /= 10;
            (machine->mem[machine->address+1]) = temp % 10;
            temp /= 10;
            (machine->mem[machine->address]) = temp % 10;
            break;
        case 0x55: 
            if (options.disassemble) { printf("movout v%X", reg1); break; }
            for (int i = 0; i<=reg1; i++) {
                machine->mem[machine->address+i] = machine->v[i];
            }
            machine->address += reg1+1;
            break;
        case 0x65: 
            if (options.disassemble) { printf("movin v%X", reg1); break; }
            for (int i = 0; i<=reg1; i++) {
                machine->v[i] = machine->mem[machine->address+i];
            }
            machine->address += reg1+1;
            break;
        default:
            if (options.disassemble) { printf("UNKNOWN INSTRUCTION"); break; }
            return ERR_INSTR_INVALID;
    }
    return SUCCESS;
}

int chip8_poll_keypress(chip8* machine) {
    int quit = 0;
    while (!quit) {
        SDL_Event e;
        while (SDL_PollEvent(&e)) {
            if (e.type == SDL_EVENT_KEY_DOWN) {
                //chip8_keyboard_set(machine, get_press_value(e.key.key)); 
            } else if (e.type == SDL_EVENT_KEY_UP) {
                return get_press_value(e.key.key);
            } else if (e.type == SDL_EVENT_QUIT) {
                return 0xFF;
            }
        }
    }
    return 0;
}

uint8_t get_press_value(SDL_Keycode key) {
    switch (key) {
        case SDLK_0: return 0;
        case SDLK_1: return 1;
        case SDLK_2: return 2;
        case SDLK_3: return 3;
        case SDLK_4: return 4;
        case SDLK_5: return 5;
        case SDLK_6: return 6;
        case SDLK_7: return 7;
        case SDLK_8: return 8;
        case SDLK_9: return 9;
        case SDLK_A: return 0xA;
        case SDLK_B: return 0xB;
        case SDLK_C: return 0xC;
        case SDLK_D: return 0xD;
        case SDLK_E: return 0xE;
        case SDLK_F: return 0xF;
        default: break; //invalid
    }
    return 0xFF;
}


uint8_t quit(chip8* machine) {
    SDL_Event e; 
    if (SDL_PollEvent(&e)) {
        if (e.type == SDL_EVENT_QUIT) {
            return 1;
        }
    } // deal with keypresses later
    return 0;
}



int opts_init(int argc, char **argv) {
    int opt;

    while ((opt = getopt(argc, argv, "d")) != -1) {
        switch (opt) {
            case 'd':
                options.disassemble = 1;
                break;
            default:
                break;
        }
    }
    return optind;
}

