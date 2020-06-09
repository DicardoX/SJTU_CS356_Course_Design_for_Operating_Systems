# makefile for producer_consumer problem
#

CC=gcc
CFLAGS=-Wall
PTHREADS=-lpthread

all: producer_consumer.o
	$(CC) $(CFLAGS) -o app producer_consumer.o $(PTHREADS)


producer_consumer.o: producer_consumer.c 
	$(CC) $(CFLAGS) -c producer_consumer.c $(PTHREADS)

clean:
	rm -rf *.o
	rm -rf app

