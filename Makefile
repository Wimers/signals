CC = gcc

CFLAGS = -Wall -Wextra -pedantic -std=gnu99
DEBUG = -g

.DEFAULT_GOAL := bmp

.PHONY: debug clean

debug: CFLAGS += $(DEBUG)
debug: bmp

bmp.o: main.c
	$(CC) $(CFLAGS) -c $^ -o $@

bmp: bmp.o
	$(CC) $(CFLAGS) $^ -o $@

clean:
	rm -f bmp *.o
	clear
