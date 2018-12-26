CC = gcc
CFLAGS = -g -Og -Wall -std=c99 -Wno-format-truncation
LDLIBS=-lreadline

all: latex

latex: latex.o linked_list.o sh_helper.o

latex.o: latex.c latex.h
	$(CC) $(CFLAGS) -c latex.c

linked_list.o: linked_list.c linked_list.h latex.h
	$(CC) $(CFLAGS) -c linked_list.c

sh_helper.o: sh_helper.c sh_helper.h
	$(CC) $(CFLAGS) -c sh_helper.c

clean:
	rm -f *.o
	rm -f latex
