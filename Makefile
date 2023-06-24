CC = gcc
CFLAGS = -c -ggdb

.PHONY: clean

all: shell

shell: shell.o tokenizer.o tokenizer.h
	$(CC) shell.o tokenizer.o -o shell

shell.o: shell.c tokenizer.h
	$(CC) $(CFLAGS) shell.c -o shell.o

tokenizer.o: tokenizer.c tokenizer.h
	$(CC) $(CFLAGS) tokenizer.c -o tokenizer.o

clean:
	rm -f *.o shell
