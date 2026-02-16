all: interpreter

interpreter: chip8.c screen.c
	gcc chip8.c screen.c -lSDL3 -o chip8

debug: chip8.c screen.c test.ch8
	gcc chip8.c screen.c -lSDL3 -o chip8 -g
