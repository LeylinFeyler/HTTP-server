CC=gcc
CFLAGS=-Wall -Wextra -Werror -Wpedantic -g

SRC=$(shell find src -name "*.c")

http:
	clear && $(CC) $(CFLAGS) $(SRC) -o server -lm
	rm -rf docs/html && doxygen
	clear && echo "server ready"

clean:
	rm -f server