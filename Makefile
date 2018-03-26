CC = g++
CFLAGS = -Wall -std=c++11
DEBUGFLAGS = -g -O0
RELEASEFLAGS = -O3
INCDIR = include/
DEST = bin/parallelgp

SRC = src/*.cpp
TESTSRC = src/Planner.cpp src/test/NogoodTester.cpp src/Logger.cpp src/Plan.cpp

debug: $(SRC) include/*.h
	$(CC) $(CFLAGS) $(DEBUGFLAGS) -I $(INCDIR) -o $(DEST) $(SRC)

parallelgp: $(SRC) include/*.h
	$(CC) $(CFLAGS) $(RELEASEFLAGS) -I $(INCDIR) -o $(DEST) $(SRC)

test: $(TESTSRC)
	$(CC) $(CFLAGS) -I $(INCDIR) -o bin/test $(TESTSRC)
	bin/test

clean:
	rm bin/*

