TARGET=bin/parallelgp

CC = g++
CXXFLAGS = -Wall -std=c++11

DEBUGFLAGS = -g -O0 -DNDEBUG
RELEASEFLAGS = -O3

INCDIR = include/
SRC = src/*.cpp src/Planners/*.cpp
TESTSRC = src/Planners/Planner.cpp src/test/NogoodTester.cpp src/Logger.cpp src/Plan.cpp

# IPASIR SAT solver if none is set yet
IPASIRSOLVER ?= lingeling

DEPS	= ipasir/sat/$(IPASIRSOLVER)/libipasir$(IPASIRSOLVER).a
LIBS	=  -Lipasir/sat/$(IPASIRSOLVER)/ -llgl#-lipasir$(IPASIRSOLVER)
LIBS	+= $(shell cat ipasir/sat/$(IPASIRSOLVER)/LIBS 2>/dev/null)


debug: CXXFLAGS += $(DEBUGFLAGS)
debug: $(TARGET)

release: CXXFLAGS += $(RELEASEFLAGS)
release: $(TARGET)

$(TARGET): $(SRC) include/*.h include/Planners/*.h Makefile
	$(CC) $(CXXFLAGS) -I $(INCDIR) $(SRC) $(LIBS) -lpthread -o $@

clean:
	rm -f $(TARGET) *.o

#test: $(TESTSRC)
#	$(CC) $(CFLAGS) -I $(INCDIR) -o bin/test $(TESTSRC)
#	bin/test

