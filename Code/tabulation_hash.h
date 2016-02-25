#ifndef tabulation_hash_h
#define tabulation_hash_h

#include <climits>
#include <string>
#include <cstdint>
#include <algorithm>
#include <stdexcept>
#include <cassert>
#include "abstract_hash.h"


template<int I>
class tabulation_hash : abstract_hash
{
private:
    // Chunksize fixed to 8 bits 
    static const uint8_t entries = UCHAR_MAX;
    static const uint16_t table_cols = 256;

    static value_t tabulation_tables[I][table_cols];

public:
    tabulation_hash<I>() : abstract_hash(I)
    {      
      // Setup tabulation tables, used for the hashing.
      uint16_t tts_cols = (sizeof(tabulation_tables[0])/sizeof(tabulation_tables[0][0]));

      std::random_device rd;
      std::mt19937 generator(rd());
      std::uniform_int_distribution<value_t> distribution(0, max_hash_value);

      for (uint8_t i = 0; i < max_key_len; i++)
      {
        for (uint16_t j = 0; j < tts_cols; j++)
        {
          tabulation_tables[i][j] = distribution(generator);
        }
      }
    }
    ~tabulation_hash(void){}
    
    value_t get_hash(std::string key)
    {
      if (key.size() > max_key_len)
      {
        throw std::invalid_argument("Key length has to be smaller than " + std::to_string(max_key_len) + ".");
      }
      if (key.size() == 0)
      {
          throw std::invalid_argument("Key must not be empty.");
      }
      assert(sizeof(uint8_t) == sizeof(char));

      // Possible solution for the silly hack below.
      auto ukey = reinterpret_cast<const uint8_t*> (key.c_str());
      value_t hash_result = 0;
      for (uint8_t i = 0; i < key.size(); i++)
      {
        hash_result ^= tabulation_tables[i][ukey[i]];
      }
      return hash_result;
    }

    value_t get_max_hash_value() { return max_hash_value; }
};


template<>
tabulation_hash<1>::tabulation_hash();
template<>
tabulation_hash<2>::tabulation_hash();
template<>
tabulation_hash<4>::tabulation_hash();
template<>
tabulation_hash<8>::tabulation_hash();
template<>
tabulation_hash<16>::tabulation_hash();

template<int I>
value_t tabulation_hash<I>::tabulation_tables[I][table_cols];
template<>
value_t tabulation_hash<1>::tabulation_tables[1][table_cols];
template<>
value_t tabulation_hash<2>::tabulation_tables[2][table_cols];
template<>
value_t tabulation_hash<4>::tabulation_tables[4][table_cols];
template<>
value_t tabulation_hash<8>::tabulation_tables[8][table_cols];
template<>
value_t tabulation_hash<16>::tabulation_tables[16][table_cols];
#endif


