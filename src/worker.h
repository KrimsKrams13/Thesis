#ifndef worker_h
#define worker_h

#include <string>
#include <exception>
#include "workload.h"
#include "push_ops.h"
#include "abstract_hash_table.h"
#include "core_workloads.h"
#include "utils.h"

namespace ycsb {

class worker {
 public:
    worker(multicore_hash::abstract_hash_table &ht, ycsb::workload &wl, workload_properties p)
     : hash_table(ht), workload(wl) 
    {
        workload.init(p); 
        push_op = new limit_op(p.max_scan_len);
    }

    virtual void do_insert();
    virtual void do_transaction();
    
    virtual ~worker() { delete push_op; }
    
 protected:
    
    virtual std::uint32_t transaction_read();
    virtual std::uint32_t transaction_read_modify_write();
    virtual std::uint32_t transaction_range_scan();
    virtual std::uint32_t transaction_update();
    virtual std::uint32_t transaction_insert();
    
    multicore_hash::abstract_hash_table &hash_table;
    abstract_push_op *push_op;
    ycsb::workload &workload;
};

inline void worker::do_insert() {
    std::string key = workload.next_sequence_key();
    std::string value;
    
    workload.build_value(value);
    hash_table.insert(key, value);
}


inline void worker::do_transaction() {
    switch (workload.next_operation()) {
        case READ:
            transaction_read();
            return;
        case UPDATE:
            transaction_update();
            return;
        case INSERT:
            transaction_insert();
            return;
        case SCAN:
            transaction_range_scan();
            return;
        case READMODIFYWRITE:
            transaction_read_modify_write();
            return;
        default:
            throw std::invalid_argument("Unknown operation.!");
    }
}

inline std::uint32_t worker::transaction_read() {
    const std::string &key = workload.next_transaction_key();
    std::string value;

    hash_table.get(key, value);

    return 0;
}


inline std::uint32_t worker::transaction_insert() {
    const std::string &key = workload.next_transaction_key();
    std::string value;

    workload.build_value(value);
    hash_table.insert(key, value);

    return 0;
} 

inline std::uint32_t worker::transaction_read_modify_write() {
    const std::string &key = workload.next_transaction_key();
    std::string value;
    
    hash_table.get(key, value);
    workload.build_value(value);
    hash_table.update(key, value);

    return 0;
}

inline std::uint32_t worker::transaction_update() {
    const std::string &key = workload.next_transaction_key();
    std::string value;

    workload.build_value(value);
    hash_table.update(key, value);

    return 0;
}

inline std::uint32_t worker::transaction_range_scan() {
    const std::string &start_key = workload.next_transaction_key();
    std::size_t len              = workload.next_scan_length();
    std::string end_key;
    workload.build_max_value(end_key);
    
    hash_table.range_scan(start_key, &end_key, (*push_op));

    return 0;
}

}

#endif