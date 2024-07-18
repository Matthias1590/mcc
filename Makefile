CC=cc
CFLAGS=-Wall -Wextra -Werror -I./src -gdwarf-4

SRCS=$(wildcard src/*.c)
OBJS=$(patsubst src/%.c, bin/%.o, $(SRCS))

all: bin/mcc

# Tests
test: bin/out
	-./bin/out; echo $$?

bin/out: bin/out.o
	cc -o $@ $^

bin/out.o: bin/out.s
	as -o $@ $^

bin/out.s: qbe-1.2/qbe bin/out.qbe
	./qbe-1.2/qbe -o $@ bin/out.qbe

qbe-1.2/qbe: qbe-1.2/Makefile
	make -C qbe-1.2

bin/out.qbe: bin/mcc test.c
	./bin/mcc > bin/out.qbe || rm bin/out.qbe

# Mcc
bin/mcc: $(OBJS)
	$(CC) $(CFLAGS) -o $@ $^

bin/%.o: src/%.c
	$(CC) $(CFLAGS) -c -o $@ $^

clean:
	rm -rf bin/*
