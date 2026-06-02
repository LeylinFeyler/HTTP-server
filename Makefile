CC=gcc
CFLAGS=-Wall -Wextra -Werror -Wpedantic -g

SRC=$(shell find src -name "*.c")

TARGET=server

.PHONY: all clean docs run

all: $(TARGET)

$(TARGET):
	@clear
	$(CC) $(CFLAGS) $(SRC) -o $(TARGET) -lm
	@clear
	@echo "server ready"

docs:
	@if command -v doxygen >/dev/null 2>&1; then \
		rm -rf docs/html; \
		doxygen; \
		clear; \
		echo "docs generated"; \
	else \
		echo "doxygen not installed"; \
	fi

run: $(TARGET)
	./$(TARGET)

clean:
	rm -f $(TARGET)