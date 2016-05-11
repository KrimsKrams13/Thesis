#include "extendible_hash_table_test.h"
#include "array_hash_table_test.h"
#include "array_hash_table.h"
#include "abstract_hash.h"
#include "mod_hash.h"
#include "worker.h"
#include "core_workloads.h"
#include <cppunit/TestCase.h>
#include <cppunit/TestFixture.h>
#include <cppunit/ui/text/TestRunner.h>
#include <cppunit/TestResult.h>
#include <cppunit/TestRunner.h>
#include <iostream>
#include <iomanip>

#define hash_value_t std::uint32_t

int main(int argc, char *argv[]) {
    // constexpr size_t directory_size = 1<<10;
    // constexpr std::uint32_t mod_value = 1<<31;

    // multicore_hash::abstract_hash<hash_value_t> *hash = new multicore_hash::mod_hash<hash_value_t, mod_value>();

    // multicore_hash::abstract_hash_table *hash_table = new multicore_hash::array_hash_table<directory_size>(hash);

    // ycsb::workload *workload = new ycsb::workload();

    // ycsb::worker w(*hash_table, *workload, ycsb::workload_a);

    CppUnit::TextUi::TestRunner runner;
        runner.addTest( multicore_hash::extendible_hash_table_test::suite() );
        runner.addTest( multicore_hash::array_hash_table_test::suite() );
    runner.run();

    // delete hash;
    // delete hash_table;
    // delete workload;
}