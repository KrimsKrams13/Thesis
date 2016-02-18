#ifndef TabulationHash_h
#define TabulationHash_h

#include <climits>
#include <string>
#include <stdint.h>

using namespace std;

typedef uint32_t value_t;

struct HashEntry {
    value_t hashValue;
};

class TabulationHash
{
private:
    static const unsigned charLength = 1; // byte in each chunk of the key.
    static const unsigned entries    = 1<<(charLength*CHAR_BIT);
    static const unsigned tableRows  = 16;
    static const unsigned tableCols  = 256;
    static const value_t tableValues[][tableCols];
    HashEntry **tabulationTables;
    unsigned maxStrLen;
    unsigned maxHashValue;
    value_t hashResult;
public:

    TabulationHash(unsigned maxStrLen);
    ~TabulationHash();
    value_t getHash(string key);
    unsigned getMaxHashValue();
};
#endif