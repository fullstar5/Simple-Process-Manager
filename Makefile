CC=gcc
CFLAGS=-Wall
LIBS=-lm

all: allocate

allocate: allocate.o queue.o task4.o
	$(CC) $(CFLAGS) -o allocate allocate.o queue.o task4.o $(LIBS)

allocate.o: allocate.c queue.h
	$(CC) $(CFLAGS) -c allocate.c -o allocate.o

queue.o: queue.c queue.h
	$(CC) $(CFLAGS) -c queue.c -o queue.o

task4.o: task4.c task4.h queue.h
	$(CC) $(CFLAGS) -c task4.c -o task4.o

clean:
	rm -f *.o allocate
