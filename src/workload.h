#ifndef workload_h
#define workload_h

#include <vector>
#include <string>
#include "Generators/abstract_generator.h"
#include "Generators/discrete_generator.h"
#include "Generators/counter_generator.h"
#include "core_workloads.h"
#include "utils.h"

typedef std::uint32_t hash_value_t;

namespace ycsb {

class workload {
  public:
    virtual void init(const workload_properties &p);
    
    virtual void build_value(std::string &value);
    virtual void build_max_value(std::string &value);
    
    virtual std::string next_sequence_key(); 
    virtual std::string next_transaction_key(); 
    virtual operation_type next_operation() { return op_selector.next(); }
    virtual size_t next_scan_length() { return scan_len_selector->next(); }
    
    workload() :
        value_len_generator(NULL), key_generator(NULL), key_selector(NULL),
        scan_len_selector(NULL), insert_sequence_generator(0) {}
    
    virtual ~workload() {
        if (value_len_generator) delete value_len_generator;
        if (key_generator)       delete key_generator;
        if (key_selector)        delete key_selector;
        if (scan_len_selector)   delete scan_len_selector;
    }
    
  protected:
    size_t max_value_len;

    abstract_generator<hash_value_t>  *value_len_generator;
    abstract_generator<hash_value_t>  *key_generator;
    counter_generator<hash_value_t>    insert_sequence_generator;

    discrete_generator<operation_type> op_selector;
    abstract_generator<hash_value_t>  *key_selector;
    abstract_generator<hash_value_t>  *scan_len_selector;
};

inline std::string workload::next_sequence_key() {
    return std::to_string(key_generator->next());
}

inline std::string workload::next_transaction_key() {
    hash_value_t key;
    do {
        key = key_selector->next();
    } while (key > insert_sequence_generator.last());
    return std::to_string(key);
}

}

#endif
