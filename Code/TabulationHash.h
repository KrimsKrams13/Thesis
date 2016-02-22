#ifndef TabulationHash_h
#define TabulationHash_h

#include <climits>
#include <string>
#include <cstdint>

    
typedef uint32_t value_t;

// Write abstract class [?!] - all methods pure virtual - separate header file.
class tabulation_hash
{
private:
    // Chunksize fixed to 8 bits 
    static const uint8_t entries = UCHAR_MAX;
    static const uint8_t table_rows = 16;
    static const uint16_t table_cols = 256;
    static const value_t table_values[][table_cols];
    value_t **tabulation_tables;
    uint8_t  max_key_len;
    value_t max_hash_value = (1UL<<(sizeof(value_t) * 8) - 1);

public:
    tabulation_hash(uint8_t max_key_len);
    ~tabulation_hash();
    value_t get_hash(std::string key);
    value_t get_max_hash_value();
};
#endif