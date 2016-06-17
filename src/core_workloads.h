#ifndef core_workloads_h
#define core_workloads_h

namespace ycsb {

enum operation_type {
    INSERT,
    READ,
    UPDATE,
    SCAN,
    READMODIFYWRITE
};


enum distribution_type {
    ZIPFIAN,
    UNIFORM,
    LATEST,
    UNUSED
};

// enum workload_type {
//     UPDATE_HEAVY,     // A
//     READ_MOSTLY,      // B
//     READ_ONLY,        // C
//     READ_LATEST,      // D
//     SHORT_RANGES,     // E
//     READ_MOD_WRITE    // F
// };

struct workload_properties {
    size_t record_count;
    size_t operation_count;
    
    double read_proportion;
    double update_proportion;
    double scan_proportion;
    double insert_proportion;
    double read_mod_write_proportion;
    
    distribution_type request_dist;
    distribution_type scan_len_dist;
    
    size_t max_scan_len;
    size_t max_value_len;
};

static workload_properties workload_a = {
    100000, 100000, 
    0.5, 0.5, 0, 0, 0, 
    distribution_type::ZIPFIAN, distribution_type::UNUSED, 
    0, 100
};

static workload_properties workload_b = {
    100000, 100000, 
    0.95, 0.05, 0, 0, 0, 
    distribution_type::ZIPFIAN, distribution_type::UNUSED, 
    0, 100
};

static workload_properties workload_c = {
    100000, 100000, 
    1, 0, 0, 0, 0, 
    distribution_type::ZIPFIAN, distribution_type::UNUSED, 
    0, 100
};

static workload_properties workload_d = {
    100000, 100000, 
    0.95, 0.05, 0, 0, 0, 
    distribution_type::LATEST, distribution_type::UNUSED, 
    0, 100
};

static workload_properties workload_e = {
    100000, 100000, 
    0, 0, 0.95, 0.05, 0, 
    distribution_type::ZIPFIAN, distribution_type::UNIFORM, 
    100, 100
};

static workload_properties workload_f = {
    100000, 100000,
    0.5, 0, 0, 0, 0.5, 
    distribution_type::ZIPFIAN, distribution_type::ZIPFIAN, 
    0, 100
};

}

#endif