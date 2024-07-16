SRCS=$(wildcard src/*.asm)
OBJS=$(patsubst src/%.asm, bin/%.o, $(SRCS))

bin/mcc: $(OBJS)
	ld -o $@ $^

bin/%.o: src/%.asm
	nasm -g -f elf64 -I./src -o $@ $^
