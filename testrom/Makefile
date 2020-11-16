all: test.o
	ld65 test.o -C test.cfg -o test.nes

test.o: test.asm
	ca65 test.asm -o test.o

.PHONY: clean
clean:
	rm test.o test.nes
