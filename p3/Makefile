#Sample Makefile. You can make changes to this file according to your need
# The executable must be named proxy

CC = gcc

all: processbased.c threadbased.c
	$(CC) -o processproxy processbased.c csapp.c
	$(CC) -o threadproxy threadbased.c csapp.c

clean:
	rm -f *~ *.o processproxy threadproxy proxy.log