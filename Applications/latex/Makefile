CC = gcc
CFLAGS = -g -Og -Wall -std=c99

all: latex

latex: latex.o

latex.o: latex.c
	$(CC) $(CFLAGS) -c latex.c

clean:
	rm -f *.o
	rm -f latex
