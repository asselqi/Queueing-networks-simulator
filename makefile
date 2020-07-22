CC=g++
CFLAGS=-std=c++11

all: simulator

simulator: *.cpp 
	$(CC) -o simulator $(CFLAGS) *.cpp

clean:
	rm -rf *o simulator

	