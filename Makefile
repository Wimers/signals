CC = gcc

WARNINGS = -Wall -Wextra -Werror -Wshadow \
	   -Wconversion -Wformat=2 -Wnull-dereference \
	   -Wunreachable-code -Wcast-align -pedantic \
	   -pedantic-errors

CFLAGS = -std=gnu99 -march=native -fopenmp
PFLAGS = -O3 -flto -funroll-loops
DEBUG = -g -fsanitize=address -fsanitize=undefined

.DEFAULT_GOAL := performance
.PHONY: debug performance clean

all: bmp

debug:
	$(MAKE) clean
	$(MAKE) bmp CFLAGS="$(CFLAGS) $(DEBUG)"

performance:
	$(MAKE) clean
	$(MAKE) bmp CFLAGS="$(CFLAGS) $(PFLAGS)"

clean:
	rm -f bmp *.o

bmp: src/*.[ch]
	$(CC) $(CFLAGS) $(WARNINGS) $^ -o $@
