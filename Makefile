CC = gcc
CFLAGS = -Wall -Wextra -O3 -Iinclude -std=c99

all:
	$(CC) *.c -o emu $(CFLAGS)
