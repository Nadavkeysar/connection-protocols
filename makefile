CC = gcc

all: v4 v6

v4: v4.c
	$(CC) v4.c -o v4

v6: v6.c
	$(CC) v6.c -o v6

.PHONY: all clean

clean:
	rm v4 v6