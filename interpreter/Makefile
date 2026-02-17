all: interpreter

interpreter: chip8.c screen.c sound.c
	gcc chip8.c screen.c sound.c -lSDL3 -o chip8

debug: chip8.c screen.c test.ch8 sound.c
	gcc chip8.c screen.c sound.c -lSDL3 -o chip8 -g
