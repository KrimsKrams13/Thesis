#include "TabulationHash.h"

#include <string>
#include <chrono>
#include <iostream>
#include <iomanip>
#include <algorithm>
#include <climits>

#include <map>

std::string generate_random_string_uniform(uint8_t len, uint8_t min_val = 1, uint64_t max_val = UCHAR_MAX)
{
	char res[len];
	std::random_device rd;
	std::mt19937 generator(rd());
	std::uniform_int_distribution<> distribution(min_val, max_val);

	// reinterpret_cast<const char*> instead.
		// for (unsigned i = 0; i < len; i++)
		// {
		// 	res[i] = (char)distribution(generator);
		// }

		std::string res_str = std::string(res, len);
		// std::cout << res_str;

	/*** VISUALIZATION OF DISTRIBUTION ***/
  // std::map<int, int> hist;
  // for(int n=0; n<10000; ++n) {
  //     ++hist[distribution(generator)];
  // }
  // for(auto p : hist) {
  //   std::cout << p.first << ' ' << std::string(p.second/5, '*') << '\n';
  // }
  /*** END OF VISUALIZATION ***/
	return res_str;
}

std::string generate_random_string_gaussian(uint8_t len, uint16_t sigma = 24, uint16_t mu = 128)
{
	char res[len];

	std::random_device rd;
	std::mt19937 generator(rd());
	std::normal_distribution<> distribution(mu, sigma);
		// for (unsigned i = 0; i < len; i++)
		// 	res[i] = (unsigned char)(round(distribution(generator)));

		std::string res_str = std::string(res, len);
		// std::cout << res_str;

  /*** VISUALIZATION OF DISTRIBUTION ***/
  // std::map<int, int> hist;
  // for(int n=0; n<10000; ++n) {
  //   ++hist[round(distribution(generator))];
  // }
  // for(auto p : hist) {
  //   std::cout << p.first << ' ' << std::string(p.second/5, '*') << '\n';
  // }
  /*** END OF VISUALIZATION ***/

	return res_str;
}

std::string generate_random_string_exponential(uint8_t len)
{
	char res[len];
		// unsigned n;

	std::random_device rd;
	std::mt19937 generator(rd());
  std::exponential_distribution<double> distribution(16);
		// for (unsigned i = 0; i < len; i++)
		// {
			// do
				// n = (round(distribution(generator)*UCHAR_MAX)) + 1; // Added 1 to avoid (char)0 = end of string.
			// while (n > 255);
			// res[i] = (unsigned char)n;
		// }

		std::string res_str = std::string(res, len);
		// std::cout << res_str;

  /*** VISUALIZATION OF DISTRIBUTION ***/
  // const int nrolls=10000;   // number of experiments
  // const int nintervals=256; // number of intervals
  // int p[nintervals]={};

  // for (int i=0; i<nrolls; ++i) {
  //   double number = distribution(generator);
  //   if (number<1.0) ++p[int(nintervals*number)];
  // }

  // for (int i=0; i<nintervals; ++i) {
  //   std::cout << i << ": ";
  //   std::cout << std::string(p[i]/10,'*') << std::endl;
  // }
  /*** END OF VISUALIZATION ***/
	return res_str;
}

void print_spread_statistics(std::map<uint64_t, uint32_t> hist)
{
  int    n   = hist.size(),
         min = INT_MAX,
         max = INT_MIN;
  double mean,
         var = 0.0,
         sum = 0.0;
  for(auto p : hist)
  {
    sum += (double)p.second;
    if (p.second < min) min = p.second;
    if (p.second > max) max = p.second;
  }

  mean = sum / n;

  for(auto p : hist)
  {
    var += (p.second-mean)*(p.second-mean);
  }

  var /= n;

  std::cout << "Variance: ";
  std::cout << std::setprecision(5) << var << std::endl;
  std::cout << "Mean: ";
  std::cout << std::setprecision(5) << mean << std::endl;
  std::cout << "Min value: " << min << std::endl;
  std::cout << "Max value: " << max << std::endl;
}

/**
 * Tests the spread of the hashValues, using Uniformly distributed keys.
 */
void test_uniform(tabulation_hash *tabulation_hash, uint32_t iterations, uint8_t key_len)
{
	// Pre-generating the strings, to only read time of actual hashing
	std::string keys[iterations];
	for(uint32_t i=0; i < iterations; i++)
		keys[i] = generate_random_string_uniform(key_len);

	// Calculating the hashing
  std::map<uint64_t, uint32_t> hist;
  for(uint32_t i=0; i < iterations; i++)
    ++hist[tabulation_hash->get_hash(keys[i])];

  // Calculating statistics on the found hashes.
  uint16_t interval_amount = 256;
  uint32_t intervals[interval_amount] = {0};
  	// Creating intervals for 'readable' histogram
  for(auto p : hist)
  	intervals[p.first/(tabulation_hash->get_max_hash_value()/interval_amount)] += p.second;

  	// Printing histogram results
  for (uint16_t i = 0; i < 256; i++)
    std::cout << i << ' ' << std::string(intervals[i]/(iterations/(32*interval_amount)), '*') << ' ' << intervals[i] << std::endl;
  print_spread_statistics(hist);
}

/**
 * Tests the spread of the hashValues, using Normally distributed keys.
 */
void test_gaussian(tabulation_hash *tabulation_hash, uint32_t iterations, uint8_t key_len)
{
	// Pre-generating the strings, to only read time of actual hashing
	std::string keys[iterations];
	for(uint32_t i=0; i < iterations; i++)
		keys[i] = generate_random_string_gaussian(key_len);

	// Calculating the hashing
  std::map<uint64_t, uint32_t> hist;
  for(uint32_t i=0; i < iterations; i++)
    ++hist[tabulation_hash->get_hash(keys[i])];

  // Calculating statistics on the found hashes.
  uint16_t interval_amount = 256;
  uint32_t intervals[interval_amount] = {0};
  	// Creating intervals for 'readable' histogram
  for(auto p : hist)
  	intervals[p.first/(tabulation_hash->get_max_hash_value()/interval_amount)] += p.second;

  	// Printing histogram results
  for (uint16_t i = 0; i < 256; i++)
    std::cout << i << ' ' << std::string(intervals[i]/(iterations/(32*interval_amount)), '*') << ' ' << intervals[i] << std::endl;
  print_spread_statistics(hist);
}

/**
 * Tests the spread of the hashValues, using exponentially distributed keys.
 */
void test_exponential(tabulation_hash *tabulation_hash, uint32_t iterations, uint8_t key_len)
{
	// Pre-generating the strings, to only read time of actual hashing
	std::string keys[iterations];
	for(uint32_t i=0; i < iterations; i++)
		keys[i] = generate_random_string_exponential(key_len);

	// Calculating the hashing
  std::map<uint64_t, uint32_t> hist;
  for(uint32_t i=0; i < iterations; i++)
    ++hist[tabulation_hash->get_hash(keys[i])];

  // Calculating statistics on the found hashes.
  uint16_t interval_amount = 256;
  uint32_t intervals[interval_amount] = {0};
  	// Creating intervals for 'readable' histogram
  for(auto p : hist)
  	intervals[p.first/(tabulation_hash->get_max_hash_value()/interval_amount)] += p.second;

  	// Printing histogram results
  for (uint16_t i = 0; i < 256; i++)
    std::cout << i << ' ' << std::string(intervals[i]/(iterations/(32*interval_amount)), '*') << ' ' << intervals[i] << std::endl;
  print_spread_statistics(hist);
}

/**
 * Tests the performance of the hashing, over different hash amounts, using normally distributed keys.
 * Returns the total amount of nanoseconds spent.
 */
void test_amount_performance(tabulation_hash *tabulation_hash, uint32_t max_amount, uint8_t key_len)
{
	using namespace std::chrono;

	// Warmup
	auto start = high_resolution_clock::now();
	generate_random_string_uniform(key_len);
	auto end = high_resolution_clock::now();

	// Testing
	for (uint32_t amount = 1; amount <= max_amount; amount<<=1)
	{
			// Pre-generating the strings, to only read time of actual hashing
		std::string keys[amount];
		for(uint32_t i = 0; i < amount; i++)
			keys[i] = generate_random_string_uniform(key_len);

		// Calculating the hashing
		auto start = high_resolution_clock::now();
	  for(uint32_t i = 0; i < amount; i++)
	    tabulation_hash->get_hash(keys[i]);
		auto end = high_resolution_clock::now();
		std::cout << "When hashing: " << amount << "keys, time spent per hashing: " 
				 << duration_cast<nanoseconds>((end-start)/amount).count() << "ns." << std::endl;
	}
}

/**
 * Tests the performance of the hashing, over different string lengths, using normally distributed keys.
 * Returns the total amount of nanoseconds spent.
 */
void test_length_performance(tabulation_hash *tabulation_hash, uint32_t amount, uint8_t max_str_len)
{
	using namespace std::chrono;

	// Warmup
	auto start = high_resolution_clock::now();
	generate_random_string_uniform(1);
	auto end = high_resolution_clock::now();

	// Testing
	std::string keys[amount];
	for (uint8_t key_len = 1; key_len <= max_str_len; key_len++)
	{
			// Pre-generating the strings, to only read time of actual hashing
		for(uint32_t i = 0; i < amount; i++)
			keys[i] = generate_random_string_uniform(key_len);

		// Calculating the hashing
		auto start = high_resolution_clock::now();
	  for(uint32_t i = 0; i < amount; i++)
	    tabulation_hash->get_hash(keys[i]);
		auto end = high_resolution_clock::now();
		std::cout << "When hashing " << amount << " keys of length: " << key_len << ", time spent per hashing: " 
				 << duration_cast<nanoseconds>((end-start)/amount).count() << "ns." << std::endl;
	}
}

int main()
{
	// Calculating the tables for the tabulation
	tabulation_hash *tabulation_h = new tabulation_hash(16);

	// test_uniform(tabulation_hash, 1000000, 4);
	// test_gaussian(tabulation_hash, 1000000, 4);
	// test_exponential(tabulation_hash, 1000000, 4);
	for (uint8_t i = 0; i < 10; i++)
		test_amount_performance(tabulation_h, 10000000, 4);
	// test_length_performance(tabulation_hash, 100000, 20);

  return 0;
}