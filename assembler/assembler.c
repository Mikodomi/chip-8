#include <stdio.h>
#include <errno.h>
#include <string.h>
#include <stdlib.h>

#include "instructions.h"

#define EXTENSION ".ch8"


int main(int argc, char** argv) {
    if (argc < 2) {
        printf("usage: %s <script>\n", argv[0]);
        return -1;
    }

    FILE* input = fopen(argv[1], "r");
    if (!input) {
        perror("fopen");
        return -1;
    }
    char* in_filename = strtok(argv[1], ".");    

    int len = strlen(in_filename);
    int extlen = strlen(EXTENSION);
    char* out_filename = malloc(len+extlen+1);
    if (!out_filename) {
        puts("out of memory");
        fclose(input);
        return -1;
    }
    strncpy(out_filename, in_filename, len);
    strncat(out_filename, EXTENSION, extlen);

    FILE* output = fopen(out_filename, "wb");
    free(out_filename);
    if (!output) {
        puts("out of memory");
        perror("fopen");
        fclose(input);
        return -1;
    }

    int status = assemble(input, output);

    fclose(input);
    fclose(output);
    return 0;
}
