CC = gcc
INPUT = act1.c act2.c act3.c support.s
PROJECT = act1 act2 act3
CFLAGS = --std=c99 -g -O3 -static

all: $(PROJECT)

$(PROJECT) :  $(INPUT)
	$(CC) $(CFLAGS) act1.c support.s -o act1
	$(CC) $(CFLAGS) act2.c support.s -o act2
	$(CC) $(CFLAGS) act3.c support.s -o act3

clean:
	rm -f $(PROJECT)
