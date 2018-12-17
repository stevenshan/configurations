CC = gcc
CFLAGS = -g -Og -Wall -std=c99

all: latex

latex: latex.o linked_list.o

latex.o: latex.c
	$(CC) $(CFLAGS) -c latex.c

linked_list.o: linked_list.c linked_list.h
	$(CC) $(CFLAGS) -c linked_list.c

clean:
	rm -f *.o
	rm -f latex
