CC = gcc
CFLAGS = -g -Og -Wall -std=c99 -Wno-format-truncation
LDLIBS=-lreadline

all: latex_dir.h latex

latex: latex.o linked_list.o

latex.o: latex.c latex.h
	$(CC) $(CFLAGS) -c latex.c

linked_list.o: linked_list.c linked_list.h latex.h
	$(CC) $(CFLAGS) -c linked_list.c

latex_dir.h: SHELL:=/bin/bash
latex_dir.h:
	echo "#define LATEX_DIR \"$$(pwd)/latex_\"" > latex_dir.h

clean:
	rm -f *.o
	rm -f latex
	rm -f latex_dir.h
