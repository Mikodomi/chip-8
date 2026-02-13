all: interpreter

interpreter: chip8.c
	gcc chip8.c -lSDL3 -o chip8

debug: chip8.c test.ch8
	gcc chip8.c -lSDL3 -o chip8 -g
