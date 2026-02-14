all: build/assembler.o build/instructions.o
	gcc build/assembler.o build/instructions.o -o build/assembler

debug: build/assembler.o.dbg build/instructions.o.dbg
	gcc build/assembler.o.dbg build/instructions.o.dbg -o build/assembler -g

build/assembler.o: assembler.c script
	gcc assembler.c -c -o build/assembler.o

build/instructions.o: instructions.c
	gcc instructions.c -c -o build/instructions.o

build/assembler.o.dbg: assembler.c script
	gcc assembler.c -c -o build/assembler.o.dbg -g

build/instructions.o.dbg: instructions.c
	gcc instructions.c -c -o build/instructions.o.dbg -g

clean:
	rm build/*
