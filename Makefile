all: interpreter assembler

interpreter: main.c
	gcc main.c -lSDL3 -o chip8

assembler: assembler.c script
	gcc assembler.c -o assembler -g

debug: main.c test.ch8
	gcc main.c -lSDL3 -o chip8 -g
