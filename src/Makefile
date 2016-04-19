NUM_TABLES     = -1
NUM_THREADS    = -1

CC             = g++
CFLAGS         = -c -std=c++14 -O3 -D NUM_TABLES_DEF=$(NUM_TABLES) -D NUM_THREADS_DEF=$(NUM_THREADS)
LDFLAGS        = -pthread

TAB_SOURCES    = tabulation_hash_test.cpp tabulation_hash.cpp
TAB_OBJECTS    = $(TAB_SOURCES:.cpp=.o)
TAB_EXECUTABLE = run_tab

HT_SOURCES     = hash_table_test.cpp
HT_OBJECTS     = $(HT_SOURCES:.cpp=.o)
HT_EXECUTABLE  = run_ht

all: $(TAB_SOURCES) $(TAB_EXECUTABLE)
ht: $(HT_SOURCES) $(HT_EXECUTABLE)

$(TAB_EXECUTABLE): $(TAB_OBJECTS)
	$(CC) $(LDFLAGS) $(TAB_OBJECTS) -o $@

$(HT_EXECUTABLE): $(HT_OBJECTS)
	$(CC) $(LDFLAGS) $(HT_OBJECTS) -lcppunit -lboost_system -lboost_thread -o $@

.cpp.o:
	$(CC) $(CFLAGS) $< -o $@

clean:
	rm *.o
