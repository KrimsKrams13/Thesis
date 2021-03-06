#include "Generators/uniform_generator.h"
#include "Generators/zipfian_generator.h"
#include "Generators/scrambled_zipfian_generator.h"
#include "Generators/skewed_latest_generator.h"
#include "workload.h"

#include <stdexcept>
#include <string>

void ycsb::workload::init(const workload_properties &p) {
    using ycsb::operation_type;
    using ycsb::distribution_type;

    if (p.read_proportion > 0) {
        op_selector.add_option(operation_type::READ, p.read_proportion);
    }
    if (p.update_proportion > 0) {
        op_selector.add_option(operation_type::UPDATE, p.update_proportion);
    }
    if (p.insert_proportion > 0) {
        op_selector.add_option(operation_type::INSERT, p.insert_proportion);
    }
    if (p.scan_proportion > 0) {
        op_selector.add_option(operation_type::SCAN, p.scan_proportion);
    }
    if (p.read_mod_write_proportion  > 0) {
        op_selector.add_option(operation_type::READMODIFYWRITE, p.read_mod_write_proportion );
    }
    
    insert_sequence_generator.set(p.record_count);

    // Instantiate generators
    max_value_len = p.max_value_len;
    value_len_generator = new uniform_generator<hash_value_t>(1, max_value_len);
    key_generator       = new counter_generator<hash_value_t>(0);
    
    // Instantiate selectors
    switch(p.request_dist) {
        case distribution_type::UNIFORM:
            key_selector = new uniform_generator<hash_value_t>(0, p.record_count - 1);
            break;
        case distribution_type::ZIPFIAN:
            key_selector = new scrambled_zipfian_generator<hash_value_t>(p.record_count + (p.operation_count * p.insert_proportion * 2U));
            break;
        case distribution_type::LATEST:
            key_selector = new skewed_latest_generator<hash_value_t>(insert_sequence_generator); 
            break;
        default:
            throw std::invalid_argument("The given request-distribution is not allowed: " + std::to_string(p.request_dist));
    }
        
    switch(p.scan_len_dist) {
        case distribution_type::UNIFORM:
            scan_len_selector = new uniform_generator<hash_value_t>(1, p.max_scan_len);
            break;
        case distribution_type::ZIPFIAN:
            scan_len_selector = new zipfian_generator<hash_value_t>(1, p.max_scan_len);
            break;
        default:
            throw std::invalid_argument("The given scan-length-distribution is not allowed: " + std::to_string(p.scan_len_dist));
    }
}

void ycsb::workload::build_value(std::string &value) {
        value = std::string(value_len_generator->next(), utils::random_print_char());
}

void ycsb::workload::build_max_value(std::string &value) {
        value = std::string(max_value_len+1, (char)0xFF);
}