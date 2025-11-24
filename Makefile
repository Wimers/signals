CC = gcc

WARNINGS = -Wall -Wextra -pedantic
CFLAGS = -std=gnu99
DEBUG = -g

.DEFAULT_GOAL := bmp

.PHONY: debug clean

debug: CFLAGS += $(DEBUG)
debug: bmp

bmp.o: main.c
	$(CC) $(CFLAGS) $(WARNINGS) -c $^ -o $@

bmp: bmp.o
	$(CC) $(CFLAGS) $^ -o $@

clean:
	rm -f bmp *.o
	clear
