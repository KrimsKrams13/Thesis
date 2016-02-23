#ifndef AbstractHash_h
#define AbstractHash_h

#include <cstring>

typedef uint32_t value_t;

class abstract_hash
{
protected:
    static const value_t max_hash_value = (1UL<<(sizeof(value_t) * 8) - 1);
    uint8_t max_key_len;
public:
	abstract_hash(uint8_t _max_key_len) : max_key_len(_max_key_len) {}

	virtual value_t get_hash(std::string) = 0;
	virtual value_t get_max_hash_value() = 0;
};

#endif