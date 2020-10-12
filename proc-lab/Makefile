# 15-213 proclab makefile
# Do not modify this file!
#
# How to use this file:
#  Type "make" at the command prompt to make the driver
#
#  unix> make
#  unix> ./driver
#
#  Type "make clean" to delete  object files and driver binary
#
#  unix> make clean
#
# 

CC=gcc
CFLAGS=-c -Wall -pedantic -O1
LDFLAGS= -lpthread  

all:  driver 

clean:
	rm -f  *.o  .proclab_*.tmp  driver 

#student's driver
driver: driver.a  puzzles.o
	$(CC) $(LDFLAGS) driver.a puzzles.o -o $@
    
%.o: %.c 
	$(CC) $(CFLAGS) $< -o $@


