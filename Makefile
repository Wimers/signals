CC = gcc

WARNINGS = -Wall -Wextra -pedantic
CFLAGS = -std=gnu99
DEBUG = -g

.DEFAULT_GOAL := all

.PHONY: debug clean

all: bmp

debug: CFLAGS += $(DEBUG)
debug: bmp

fileParsing.o: fileParsing.c
	$(CC) $(CFLAGS) $(WARNINGS) -c $^ -o $@

bmp.o: main.c
	$(CC) $(CFLAGS) $(WARNINGS) -c $^ -o $@

bmp: bmp.o fileParsing.o
	$(CC) $(CFLAGS) $(WARNINGS) $^ -o $@

clean:
	rm -f bmp *.o
	clear
