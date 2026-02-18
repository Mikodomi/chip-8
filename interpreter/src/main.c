#include <SDL3/SDL.h>
#include <string.h>
#include <inttypes.h>
#include <stdio.h>
#include <time.h>
#include <stdlib.h>
#include <errno.h>
#include <unistd.h>
#include "chip8.h"

int main(int argc, char **argv) {
    if (argc < 2) {
        printf("usage: %s [-d] <rom_name>\n", argv[0]);
        return -1;
    }

    int file_ind = opts_init(argc, argv); 

    FILE* rom = fopen(argv[file_ind], "rb");
    if (rom == NULL) {
        fprintf(stderr, "error while opening rom file: ");
        perror("fopen");
        return -1;
    }
    FILE* fonts = fopen("assets/font.ch8", "rb");
    if (fonts == NULL) {
        fprintf(stderr, "error while opening font file: ");
        perror("fopen");
        fclose(rom);
        return -1;
    }
    if (!options.disassemble) {
        if (!SDL_Init(SDL_INIT_VIDEO | SDL_INIT_EVENTS | SDL_INIT_AUDIO)) {
            SDL_Log("error during SDL3 init: %s\n", SDL_GetError());
            fclose(rom);
            fclose(fonts);
            return -1;
        }
    }
    chip8 machine;
    if (chip8_init(&machine, argv[file_ind]) < 0) {
        fprintf(stderr, "error during chip8 init\n");
        fclose(rom);
        fclose(fonts);
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
