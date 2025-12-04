CC = gcc

# -Wformat=2 -Wconversion
WARNINGS = -Wall -Wextra -Werror -Wshadow -Wnull-dereference -Wunreachable-code -Wcast-align -pedantic -pedantic-errors
CFLAGS = -std=gnu99 -march=native
PFLAGS = -O3 -flto -funroll-loops
DEBUG = -g -fsanitize=address -fsanitize=undefined

.DEFAULT_GOAL := all

.PHONY: debug performance clean

all: bmp

debug: CFLAGS += $(DEBUG)
debug: all

performance: CFLAGS += $(PFLAGS)
performance: all

clean:
	rm -f bmp *.o

filters.o: filters.c
	$(CC) $(CFLAGS) $(WARNINGS) -c $^ -o $@

fileParsing.o: fileParsing.c
	$(CC) $(CFLAGS) $(WARNINGS) -c $^ -o $@

main.o: main.c
	$(CC) $(CFLAGS) $(WARNINGS) -c $^ -o $@

bmp: main.o fileParsing.o filters.o
	$(CC) $(CFLAGS) $(WARNINGS) $^ -o $@
