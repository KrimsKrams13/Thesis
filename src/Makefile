NUM_TABLES  = -1
NUM_THREADS = -1

CC=g++
CFLAGS=-c -std=c++11 -O3 -D NUM_TABLES_DEF=$(NUM_TABLES) -D NUM_THREADS_DEF=$(NUM_THREADS)
LDFLAGS= -pthread 
SOURCES=tabulation_hash_test.cpp tabulation_hash.cpp
OBJECTS=$(SOURCES:.cpp=.o)
EXECUTABLE=run

all: $(SOURCES) $(EXECUTABLE)

$(EXECUTABLE): $(OBJECTS)
	$(CC) $(LDFLAGS) $(OBJECTS) -o $@

.cpp.o:
	$(CC) $(CFLAGS) $< -o $@


#default: TabulationHash.o TabulationHashTest.o
#	$(CC) TabulationHash.o -o TabulationHash
#
#TabulationHash.o: TabulationHash.cpp
#	$(CC) $(CFLAGS) TabulationHash.cpp 
#
#TabulationHashTest.o: TabulationHashTest.cpp
#	$(CC) $(CFLAGS) TabulationHashTest.cpp 	
#
clean:
	rm *.o
