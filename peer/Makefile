# Makefile for the Peer
#
# CS 60, March 2018


##NEED TO REMAKE MAKEFILE FROM START

C = ../common

PROGS = peer
LIBS = -pthread

CFLAGS = -Wall -pedantic -w -std=c11 -ggdb -I$C -D_BSD_SOURCE
CC = gcc
MAKE = make

# for memory-leak tests
VALGRIND = valgrind --leak-check=full --show-leak-kinds=all

all: $(PROGS)

peer: peer.o $(C)/file.o 
	$(CC) $(CFLAGS) $^ $(LIBS) -o $@

##### source dependencies
peer.o: peer.h $(C)/constants.h $(C)/filetable.h $(C)/file.h
filetable.o: $(C)/filetable.h $(C)/constants.h


.PHONY: valgrind clean

########### valgrind ##################
valgrind: peer
	$(VALGRIND) ./peer

############## clean  ##########
clean:
	rm -rf *~ *.o *.dSYM .DS_Store
	rm -rf $(PROGS)

