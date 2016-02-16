#ifndef TabulationHash_h
#define TabulationHash_h

#include <stdint.h>
#include <string>

typedef uint32_t value_t;

struct HashEntry {
    value_t hashValue;
};

class TabulationHash
{
private:
    static const unsigned charLength = 1; // byte in each chunk of the key.
    static const unsigned entries    = 2<<((charLength*8)-1);
    // static const unsigned tableSize  = entries * sizeof(HashEntry);
    static const unsigned stringLen  = 16; 
public:
    HashEntry tabulationTables[stringLen][entries];

    TabulationHash();
    ~TabulationHash();

    value_t getHash(std::string key);
};

/*
class HashTable
{
private:
        
    // Length is the size of the Hash Table array.
    int length;
    
    // Returns an array location for a given item key.
    int hash( string itemKey );
    
public:
    
    // Constructs the empty Hash Table object.
    // Array length is set to 13 by default.
    HashTable( int tableLength = 13 );
    
    // Adds an item to the Hash Table.
    void insertItem( Item * newItem );
    
    // Deletes an Item by key from the Hash Table.
    // Returns true if the operation is successful.
    bool removeItem( string itemKey );
    
    // Returns an item from the Hash Table by key.
    // If the item isn't found, a null pointer is returned.
    Item * getItemByKey( string itemKey );
    
    // Display the contents of the Hash Table to console window.
    void printTable();
    
    // Prints a histogram illustrating the Item distribution.
    void printHistogram();
    
    // Returns the number of locations in the Hash Table.
    int getLength();
    
    // Returns the number of Items in the Hash Table.
    int getNumberOfItems();
    
    // De-allocates all memory used for the Hash Table.
    ~HashTable();
};
*/
#endif