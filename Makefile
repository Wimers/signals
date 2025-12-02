CC = gcc

WARNINGS = -Wall -Wextra -pedantic -Wconversion -Wshadow
CFLAGS = -std=gnu99 -O3 -march=native -flto
DEBUG = -g

.DEFAULT_GOAL := all

.PHONY: debug clean

all: bmp

debug: CFLAGS += $(DEBUG)
debug: bmp

filters.o: filters.c
	$(CC) $(CFLAGS) $(WARNINGS) -c $^ -o $@

fileParsing.o: fileParsing.c
	$(CC) $(CFLAGS) $(WARNINGS) -c $^ -o $@

main.o: main.c
	$(CC) $(CFLAGS) $(WARNINGS) -c $^ -o $@

bmp: main.o fileParsing.o filters.o
	$(CC) $(CFLAGS) $(WARNINGS) $^ -o $@

clean:
	rm -f bmp *.o
