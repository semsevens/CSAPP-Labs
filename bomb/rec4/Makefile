CC = gcc
INPUT = act1.c act2.c act3.c act2.s act3.s act4.c
PROJECT = act1 act2 act3 act4
CFLAGS = --std=c99 -g -O3 -static

all: $(PROJECT)

$(PROJECT) :  $(INPUT)
	$(CC) $(CFLAGS) act1.c  -o act1
	$(CC) $(CFLAGS) act2.c act2.s -o act2
	$(CC) $(CFLAGS) act3.c act3.s -o act3
	$(CC) $(CFLAGS) act4.c -o act4

clean:
	rm -f $(PROJECT)
