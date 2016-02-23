#include "TabulationHash.h"

#include <string>
#include <chrono>
#include <iostream>
#include <iomanip>
#include <algorithm>
#include <climits>
#include <cstring>

#include <map>

std::string generate_random_string_uniform(uint8_t len, uint8_t min_val = 0, uint64_t max_val = ULLONG_MAX)
{
	const char* res;
	std::random_device rd;
	std::mt19937_64 generator(rd());
	std::uniform_int_distribution<unsigned long long> distribution(min_val, max_val);

	unsigned long long int_res = distribution(generator);
	res = reinterpret_cast<const char*>(&int_res);

	std::string res_str = std::string(res, len);
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

std::string generate_random_string_gaussian(uint8_t len, uint64_t sigma = LLONG_MAX/4, uint64_t mu = LLONG_MAX)
{
	const char* res;
	std::random_device rd;
	std::mt19937_64 generator(rd());
	std::normal_distribution<double> distribution(mu, sigma);

	double double_res = distribution(generator);
	res = reinterpret_cast<const char*>(&double_res);

	std::string res_str = std::string(res, len);

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

std::string generate_random_string_exponential(uint8_t len, double lambda = 16.0)
{
	const char* res;
	std::random_device rd;
	std::mt19937_64 generator(rd());
	std::exponential_distribution<double> distribution(lambda);

	double double_res = distribution(generator) * (1UL << len);
	res = reinterpret_cast<const char*>(&double_res);

	std::string res_str = std::string(res, len);		

  /*** VISUALIZATION OF DISTRIBUTION ***/
  // const int nrolls=10000;   // number of experiments
  // const int nintervals=256; // number of intervals
  // int p[nintervals]={};

  // uint32_t max_hash_value = (1UL<<(sizeof(value_t) * 8) - 1);	

  // double number;
  // for (int i=0; i<nrolls; ++i) {
  // 	do
  // 	{
  //   	number = distribution(generator);
  //   }
  //   while (number>=1.0);
  //   ++p[int(nintervals*number)];
  // }

  // for (int i=0; i<nintervals; ++i) {
  //   std::cout << i << ": ";
  //   std::cout << std::string(p[i]/ 10,'*') << std::endl;
  // }
  /*** END OF VISUALIZATION ***/
	return res_str;
}

void print_spread_statistics(std::map<uint64_t, uint32_t> hist)
{
  int    n   = hist.size(),
         min = INT_MAX,
         max = 0	;
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

  std::cout << "Avg. Variance: ";
  std::cout << std::setprecision(5) << var << std::endl;
  std::cout << "Mean: ";
  std::cout << std::setprecision(5) << mean << std::endl;
  std::cout << "Min value: " << min << std::endl;
  std::cout << "Max value: " << max << std::endl;
}

/**
 * Tests the spread of the hashValues, using Uniformly distributed keys.
 */
template<int I>
void test_uniform(tabulation_hash<I> *tabulation_hash, uint32_t iterations, uint8_t key_len)
{
	// Pre-generating the strings, to only read time of actual hashing
	std::string* keys = new std::string[iterations];
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
template<int I>
void test_gaussian(tabulation_hash<I> *tabulation_hash, uint32_t iterations, uint8_t key_len)
{
	// Pre-generating the strings, to only read time of actual hashing
	std::string* keys = new std::string[iterations];
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
template<int I>
void test_exponential(tabulation_hash<I> *tabulation_hash, uint32_t iterations, uint8_t key_len)
{
	// Pre-generating the strings, to only read time of actual hashing
	std::string* keys = new std::string[iterations];
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
template<int I>
void test_amount_performance(tabulation_hash<I> *tabulation_hash, uint32_t max_amount, uint8_t key_len)
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
		std::string* keys = new std::string[amount];
		for(uint32_t i = 0; i < amount; i++)
			keys[i] = generate_random_string_uniform(key_len);

		// Calculating the hashing
		auto start = high_resolution_clock::now();
	  for(uint32_t i = 0; i < amount; i++)
	    tabulation_hash->get_hash(keys[i]);
		auto end = high_resolution_clock::now();
		std::cout << "When hashing: " << amount << "keys, time spent per hashing: " 
				 << duration_cast<nanoseconds>((end-start)/amount).count() << "ns." << std::endl;
		delete[] keys;
	}
}

/**
 * Tests the performance of the hashing, over different string lengths, using normally distributed keys.
 * Returns the total amount of nanoseconds spent.
 */
template<int I>
void test_length_performance(tabulation_hash<I> *tabulation_hash, uint32_t amount, uint8_t max_str_len)
{
	using namespace std::chrono;

	// Warmup
	auto start = high_resolution_clock::now();
	generate_random_string_uniform(1);
	auto end = high_resolution_clock::now();

	// Testing
	std::string* keys = new std::string[amount];
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
		// std::cout << "When hashing " << amount << " keys of length: " << std::to_string(key_len) << ", time spent per hashing: " 
		std::cout << std::to_string(key_len) << ": " << duration_cast<nanoseconds>((end-start)/amount).count() << std::endl;
	}
	delete[] keys;
}

int main()
{
	const int max_key_len = 32;
	const int key_len = 31;
	if (max_key_len < key_len)
		throw std::invalid_argument("Key length has to be smaller than " + std::to_string(max_key_len) + ".");

	// Calculating the tables for the tabulation
	tabulation_hash<max_key_len> *tabulation_h = new tabulation_hash<max_key_len>();

	// test_uniform(tabulation_h, 1000000, key_len);
	// test_gaussian(tabulation_h, 1000000, key_len);
	// test_exponential(tabulation_h, 1000000, key_len);
	// test_amount_performance(tabulation_h, 10000000, key_len);
	// test_length_performance<max_key_len>(tabulation_h, 10000000, 31);

  return 0;
}