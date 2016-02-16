#ifndef TabulationHash_h
#define TabulationHash_h

#include <climits>
#include <string>

typedef uint32_t value_t;

struct HashEntry {
    value_t hashValue;
};

class TabulationHash
{
private:
    static const unsigned charLength = 1; // byte in each chunk of the key.
    static const unsigned entries    = 1<<(charLength*CHAR_BIT);
    static const unsigned stringLen  = 16; 
public:
    HashEntry tabulationTables[stringLen][entries];

    TabulationHash();
    ~TabulationHash();
    
    value_t getHash(std::string key);
};
#endif