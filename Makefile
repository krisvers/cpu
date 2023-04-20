CC = gcc
CFLAGS = -Wall -Werror -Wextra -O3 -Iinclude

all:
	$(CC) *.c -o emu $(CFLAGS)
