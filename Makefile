CC = gcc

WARNINGS = -Wall -Wextra -Werror -Wshadow \
	   -Wconversion -Wformat=2 -Wnull-dereference \
	   -Wunreachable-code -Wcast-align -pedantic \
	   -pedantic-errors -Wunused

CFLAGS = -std=c23 -march=native -fopenmp
PFLAGS = -O3 -flto -funroll-loops
DEBUG = -g -fsanitize=address -fsanitize=undefined
LFLAGS =

.DEFAULT_GOAL := performance
.PHONY: debug performance clean install sdl-install uninstall link asm sdl profile

all: signals

debug: CFLAGS += $(DEBUG)
debug: signals-debug

performance: CFLAGS += $(PFLAGS)
performance: signals

signals signals-debug: src/*.[ch]
	$(CC) $(CFLAGS) $(WARNINGS) $^ -o $@ $(LFLAGS)

clean:
	rm -f signals signals-debug *.o *.s

asm: CFLAGS := $(filter-out -flto, $(CFLAGS) $(PFLAGS)) -fverbose-asm
asm:
	$(CC) $(CFLAGS) $(WARNINGS) -S src/*.c

sdl: CFLAGS += $(PFLAGS)
sdl: CFLAGS += -DENABLE_SDL
sdl: CFLAGS += $(shell sdl2-config --cflags)
sdl: LFLAGS += $(shell sdl2-config --libs)
sdl: signals

PREFIX ?= /usr/local
BINDIR = $(PREFIX)/bin

install: signals
	@echo "Installing to $(BINDIR)..."
	install -d $(BINDIR)
	install -m 755 signals $(BINDIR)/signals

sdl-install:
	@$(MAKE) clean
	@$(MAKE) sdl
	@$(MAKE) install

profile: CFLAGS += -fno-omit-frame-pointer -g
profile: signals

uninstall:
	rm $(BINDIR)/signals

link: signals
	@echo "Linking $(CURDIR)/signals to $(BINDIR)/signals..."
	ln -sf $(CURDIR)/signals $(BINDIR)/signals
