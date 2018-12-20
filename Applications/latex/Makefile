CC = gcc
CFLAGS = -g -Og -Wall -std=c99

all: latex

latex: latex.o linked_list.o circular_buffer.o

latex.o: latex.c latex.h
	$(CC) $(CFLAGS) -c latex.c

linked_list.o: linked_list.c linked_list.h latex.h
	$(CC) $(CFLAGS) -c linked_list.c

circular_buffer.o: circular_buffer.c circular_buffer.h latex.h
	$(CC) $(CFLAGS) -c circular_buffer.c

clean:
	rm -f *.o
	rm -f latex
