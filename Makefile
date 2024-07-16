CC=cc
CFLAGS=-Wall -Wextra -Werror -I./src -gdwarf-4

SRCS=$(wildcard src/*.c)
OBJS=$(patsubst src/%.c, bin/%.o, $(SRCS))

bin/mcc: $(OBJS)
	$(CC) $(CFLAGS) -o $@ $^

bin/%.o: src/%.c
	$(CC) $(CFLAGS) -c -o $@ $^
