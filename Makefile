CC = g++
CFLAGS = -Wall -O3 -std=c++11
INCDIR = include/
SRC = src/*.cpp
DEST = bin/parallelgp

debug: $(SRC) include/*.h
	$(CC) $(CFLAGS) -g -I $(INCDIR) -o $(DEST) $(SRC)

parallelgp: $(SRC) include/*.h
	$(CC) $(CFLAGS) -I $(INCDIR) -o $(DEST) $(SRC)

clean:
	rm bin/*

