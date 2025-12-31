CC = gcc

WARNINGS = -Wall -Wextra -Werror -Wshadow \
	   -Wconversion -Wformat=2 -Wnull-dereference \
	   -Wunreachable-code -Wcast-align -pedantic \
	   -pedantic-errors

CFLAGS = -std=c2x -march=native -fopenmp
PFLAGS = -O3 -flto -funroll-loops
DEBUG = -g -fsanitize=address -fsanitize=undefined

.DEFAULT_GOAL := performance
.PHONY: debug performance clean

all: signals

debug:
	$(MAKE) clean
	$(MAKE) signals CFLAGS="$(CFLAGS) $(DEBUG)"

performance:
	$(MAKE) clean
	$(MAKE) signals CFLAGS="$(CFLAGS) $(PFLAGS)"

clean:
	rm -f signals *.o

signals: src/*.[ch]
	$(CC) $(CFLAGS) $(WARNINGS) $^ -o $@
