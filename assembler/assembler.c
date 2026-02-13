#include <stdio.h>
#include <errno.h>
#include <string.h>
#include <stdlib.h>

#define EXTENSION ".ch8"

int assemble(FILE* in, FILE* out) {
    char* lineptr;
    size_t len = 0;
    ssize_t nread;
    char* word;
    char* word2;
    while ((nread = getline(&lineptr, &len, in)) != -1) {
        if (lineptr[0] == ';') continue;
        word = strtok(lineptr, " ;\n");
        // 1 symbol instructions
        if (strcmp(word, "clear") == 0) {
            fwrite("\x00\xE0", 1, 2, out);
            continue;
        } else if (strcmp(word, "ret") == 0) {
            fwrite("\x00\xEE", 1, 2, out);
            continue;
        }
        word2 = strtok(NULL, " ;\n");
    }
    free(lineptr);
    return 0;
}

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
