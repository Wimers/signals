CC = gcc

WARNINGS = -Wall -Wextra -Werror -Wshadow \
	   -Wconversion -Wformat=2 -Wnull-dereference \
	   -Wunreachable-code -Wcast-align -pedantic \
	   -pedantic-errors

CFLAGS = -std=c2x -march=native -fopenmp
PFLAGS = -O3 -flto -funroll-loops
DEBUG = -g -fsanitize=address -fsanitize=undefined

.DEFAULT_GOAL := performance
.PHONY: debug performance clean install uninstall link

all: signals

debug: CFLAGS += $(DEBUG)
debug: signals-debug

performance: CFLAGS += $(PFLAGS)
performance: signals

signals signals-debug: src/*.[ch]
	$(CC) $(CFLAGS) $(WARNINGS) $^ -o $@

clean:
	rm -f signals signals-debug *.o

PREFIX ?= /usr/local
BINDIR = $(PREFIX)/bin

install: signals
	@echo "Installing to $(BINDIR)..."
	install -d $(BINDIR)
	install -m 755 signals $(BINDIR)/signals

uninstall:
	rm $(BINDIR)/signals

link: signals
	@echo "Linking $(CURDIR)/signals to $(BINDIR)/signals..."
	ln -sf $(CURDIR)/signals $(BINDIR)/signals
