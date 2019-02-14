CC=gcc
CFLAGS=-ggdb3

all: ringmaster player

ringmaster: ringmaster.c potato.h
	$(CC) $(CFLAGS) -o $@ $<

player: player.c potato.h
	$(CC) $(CFLAGS) -o $@ $<