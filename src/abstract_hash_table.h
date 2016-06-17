#ifndef abstract_hash_table_h
#define abstract_hash_table_h

#include "abstract_hash.h"
#include "push_ops.h"
#include <string>

namespace multicore_hash {

    class abstract_hash_table
    {
    private:
        abstract_hash<std::string> *hash;
    public:
        virtual bool get   (const std::string& key, std::string& value) = 0;
        virtual void update(const std::string& key, const std::string& value) = 0;
        virtual void insert(const std::string& key, const std::string& value) = 0;
        virtual void remove(const std::string& key) = 0;
        virtual void range_scan(const std::string& start_key,
                            const std::string* end_key, abstract_push_op&) = 0;
        virtual void reverse_range_scan(const std::string& start_key,
                                    const std::string* end_key, abstract_push_op&) = 0;
        // virtual std::size_t size() const = 0;
    };
}

#endif