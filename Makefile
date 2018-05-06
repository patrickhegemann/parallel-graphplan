TARGET=bin/parallelgp

CC = g++
CXXFLAGS = -Wall -std=c++11

DEBUGFLAGS = -g -O0 -DNDEBUG
RELEASEFLAGS = -O3

INCDIR = include/
SRC = src/*.cpp src/Planners/*.cpp
TESTSRC = src/Planners/Planner.cpp src/test/NogoodTester.cpp src/Logger.cpp src/Plan.cpp

# IPASIR SAT solver if none is set yet
IPASIRSOLVER ?= glucose4#abcdsat_i17#cryptominisat5#picosat961#minisat220
#CXXFLAGS += -D'IPASIRCPP'
#CXXFLAGS += -D'PGP_NOSETLEARN'

#ipasir/sat/lingeling/

DEPS	= ipasir/sat/$(IPASIRSOLVER)/libipasir$(IPASIRSOLVER).a
LIBS	= -Lipasir/sat/$(IPASIRSOLVER)/ -lipasir$(IPASIRSOLVER)#-L/usr/local/lib/ -lipasircryptominisat5# #-L./ipasir/sat/lingeling/ -llgl
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

