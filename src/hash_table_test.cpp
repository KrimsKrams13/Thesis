#include "extendible_hash_table.h"
#include "extendible_hash_table_test.h"
#include "mod_hash.h"
#include <cppunit/TestCase.h>
#include <cppunit/TestFixture.h>
#include <cppunit/ui/text/TestRunner.h>
#include <cppunit/TestResult.h>
#include <cppunit/TestRunner.h>
#include <iostream>
#include <iomanip>

#define value_t std::uint32_t

	multicore_hash::abstract_hash<value_t> *hash = new multicore_hash::mod_hash<value_t, (1<<31)>();
	multicore_hash::extendible_hash_table<std::string, std::string> *hash_table = new multicore_hash::extendible_hash_table<std::string, std::string>(hash);

	int main(int argc, char *argv[]) {
		CppUnit::TextUi::TestRunner runner;
	  runner.addTest( multicore_hash::extendible_hash_table_test::suite() );
	  runner.run();
	  // std::uint32_t i = 0;
	  // for (std::uint8_t j = 1; j < 8; j++) {
	  	// for (; i < (1<<j)*hash_table->get_bucket_entries(); i++)
				// hash_table->insert(std::to_string(i*hash_table->get_bucket_entries()), " 1");
			// print_extendible_hash_table();
		// }
		// hash_table->insert(" 4", " 2");
		// print_extendible_hash_table();
		// hash_table->insert(" 8", " 3");
		// print_extendible_hash_table();
		// hash_table->insert("12", " 4");
		// print_extendible_hash_table();
		// hash_table->insert("16", " 5");
		// print_extendible_hash_table();
		// hash_table->insert("1", " 5");
		// hash_table->insert("17", " 5");
		// hash_table->insert("25", " 5");
		// hash_table->insert("49", " 5");
		// hash_table->insert("73", " 5");
		// print_extendible_hash_table();
		// hash_table->insert("20", " 6");
		// print_extendible_hash_table();
		// hash_table->insert("24", " 7");
		// print_extendible_hash_table();
		// hash_table->insert("28", " 8");
		// print_extendible_hash_table();
		// hash_table->insert("32", " 9");
		// print_extendible_hash_table();
		// hash_table->insert("36", "10");
		// print_extendible_hash_table();
		// hash_table->insert("40", "11");
		// print_extendible_hash_table();
		// hash_table->insert("44", "12");
		// print_extendible_hash_table();
		// hash_table->insert("48", "13");
		// print_extendible_hash_table();
		// hash_table->insert("52", "14");
		// print_extendible_hash_table();
		// hash_table->insert("56", "15");
		// print_extendible_hash_table();
		// hash_table->insert("60", "16");
		// print_extendible_hash_table();
		// hash_table->insert("64", "17");
		// print_extendible_hash_table();
		// hash_table->insert("68", "18");
		// print_extendible_hash_table();
		// hash_table->insert("72", "18");
		// print_extendible_hash_table();
		// hash_table->insert("76", "18");
		// print_extendible_hash_table();
		// hash_table->insert("80", "18");
		// print_extendible_hash_table();
		// hash_table->insert("84", "18");
		// print_extendible_hash_table();
		// hash_table->insert("20", " 6");
		// hash_table->insert(" 4", " 2");
		// hash_table->insert(" 8", " 3");
		// hash_table->insert("12", " 4");
		// hash_table->insert("16", " 5");
		// hash_table->insert("20", " 6");
		// hash_table->insert("24", " 7");
		// hash_table->insert("28", " 8");
		// hash_table->insert("32", " 9");
		// hash_table->insert("34", "10");
	}