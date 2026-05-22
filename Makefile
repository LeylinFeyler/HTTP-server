CC=gcc
CFLAGS=-Wall -Wextra -Werror -Wpedantic -g

SRC=$(shell find src -name "*.c")

lecurl:
	clear && $(CC) $(CFLAGS) $(SRC) -o server -lm

clean:
	rm -f server