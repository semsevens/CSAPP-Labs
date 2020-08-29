CC=clang
CFLAGS=-Wall -Werror
CLIBS=-lm

all: getopt_example missing_side

getopt_example:
	$(CC) $(CFLAGS) getopt_example.c -o getopt_example

missing_side:
	$(CC) $(CFLAGS) $(CLIBS) missing_side.c -o missing_side

clean:
	rm getopt_example missing_side