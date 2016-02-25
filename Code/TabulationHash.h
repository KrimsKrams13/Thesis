#ifndef TabulationHash_h
#define TabulationHash_h

#include <climits>
#include <string>
#include <cstdint>
#include <algorithm>
#include <random>
#include <stdexcept>
#include <cassert>
#include "AbstractHash.h"

namespace multicore_hash {
    template<typename value_t = std::uint32_t, std::uint8_t num_tables = 1>
    class tabulation_hash : public abstract_hash<value_t> {
    private:
        // Chunksize fixed to 8 bits
        value_t tabulation_tables[num_tables][std::numeric_limits<std::uint8_t>::max()];

    public:
        tabulation_hash() {
            std::random_device rd;
            std::mt19937 generator(rd());
            std::uniform_int_distribution <value_t> distribution;

            for (uint8_t i = 0; i < num_tables; i++) {
                for (uint8_t j = 0; j < std::numeric_limits<std::uint8_t>::max(); j++) {
                    tabulation_tables[i][j] = distribution(generator);
                }
            }
        }

        value_t get_hash(const std::string& key) override {
            if (key.size() > sizeof(std::uint8_t)*num_tables) {
                throw std::invalid_argument(
                        "Key length has too big");
            }
            if (key.size() == 0) {
                throw std::invalid_argument("Key must not be empty.");
            }
            assert(sizeof(uint8_t) == sizeof(char));

            // Possible solution for the silly hack below.
            auto ukey = reinterpret_cast<const uint8_t*>(key.c_str());
            value_t hash_result = 0;
            for (uint8_t i = 0; i < key.size(); i++) {
                hash_result ^= tabulation_tables[i][ukey[i]];
            }
            return hash_result;
        }
    };
}

#endif

