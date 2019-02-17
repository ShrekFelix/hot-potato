CC=gcc
CFLAGS=-ggdb3 -Wno-implicit-function-declaration

all: ringmaster player

ringmaster: ringmaster.c potato.h
	$(CC) $(CFLAGS) -o $@ $<

player: player.c potato.h
	$(CC) $(CFLAGS) -o $@ $<

clean:
	rm player ringmaster