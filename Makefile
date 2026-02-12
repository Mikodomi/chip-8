all: main.c
	gcc main.c -lSDL3 -o chip8

debug: main.c test.ch8
	gcc main.c -lSDL3 -o chip8 -g
	gdb --command=init.gdb --args chip8 test.ch8 
