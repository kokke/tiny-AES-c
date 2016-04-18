
CFLAGS=-Wall

test: aes.o main.o
	gcc aes.o main.o -o test

aes.o: aes.c aes.h
	gcc -c aes.c -o aes.o $(CFLAGS)

main.o: main.c aes.h
	gcc -c main.c -o main.o $(CFLAGS)

