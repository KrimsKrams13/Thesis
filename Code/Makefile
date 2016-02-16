CC=g++
CFLAGS=-c -std=c++11
LDFLAGS=
SOURCES=TabulationHashTest.cpp TabulationHash.cpp
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
