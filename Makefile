CC = g++
CFLAGS = -Wall -std=c++11
DEBUGFLAGS = -g -O0
RELEASEFLAGS = -O3
INCDIR = include/
SRC = src/*.cpp
DEST = bin/parallelgp

debug: $(SRC) include/*.h
	$(CC) $(CFLAGS) $(DEBUGFLAGS) -I $(INCDIR) -o $(DEST) $(SRC)

parallelgp: $(SRC) include/*.h
	$(CC) $(CFLAGS) $(RELEASEFLAGS) -I $(INCDIR) -o $(DEST) $(SRC)

clean:
	rm bin/*

