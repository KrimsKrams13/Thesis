#include "tabulation_hash.h"
#include "mult_shift_hash.h"
#include "murmur_hash.h"

#include <string>
#include <chrono>
#include <iostream>
#include <fstream>
#include <iomanip>
#include <algorithm>
#include <climits>
#include <cstring>

#include <map>

std::string generate_random_string_uniform(std::uint8_t len, std::uint8_t min_val = 0, std::uint64_t max_val = ULLONG_MAX)
{
  const char* res;
  std::random_device rd;
  std::mt19937_64 generator(rd());
  std::uniform_int_distribution<std::uint64_t> distribution(min_val, max_val);

  std::uint64_t int_res = distribution(generator);
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

std::string generate_random_string_gaussian(std::uint8_t len, std::uint64_t sigma = LLONG_MAX/4, std::uint64_t mu = LLONG_MAX)
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

std::string generate_random_string_exponential(std::uint8_t len, double lambda = 16.0)
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

  // std::uint32_t max_hash_value = (1UL<<(sizeof(value_t) * 8) - 1); 

  // double number;
  // for (int i=0; i<nrolls; ++i) {
  //  do
  //  {
  //    number = distribution(generator);
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

void print_spread_statistics(std::map<std::uint8_t, std::uint32_t> hist)
{
  int    n   = hist.size(),
         min = INT_MAX,
         max = 0  ;
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
template<typename value_t>
std::map<std::uint64_t, std::uint32_t> test_uniform(multicore_hash::abstract_hash<value_t> *hash, std::uint32_t iterations, std::uint8_t key_len)
{
  // Pre-generating the strings, to only read time of actual hashing
  std::string* keys = new std::string[iterations];
  const uint8_t* keyc;
  for(std::uint32_t i=0; i < iterations; i++) 
    keys[i] = generate_random_string_uniform(key_len);

  // Calculating the hashing
  std::map<std::uint64_t, std::uint32_t> hist;
  for(std::uint32_t i=0; i < iterations; i++)
    ++hist[hash->get_hash(keys[i])];

  return hist;
}

/**
 * Tests the spread of the hashValues, using Normally distributed keys.
 */
template<typename value_t>
std::map<std::uint64_t, std::uint32_t> test_gaussian(multicore_hash::abstract_hash<value_t> *hash, std::uint32_t iterations, std::uint8_t key_len)
{
  // Pre-generating the strings, to only read time of actual hashing
  std::string* keys = new std::string[iterations];
  for(std::uint32_t i=0; i < iterations; i++)
    keys[i] = generate_random_string_gaussian(key_len);

  // Calculating the hashing
  std::map<std::uint64_t, std::uint32_t> hist;
  for(std::uint32_t i=0; i < iterations; i++)
    ++hist[hash->get_hash(keys[i])];

  return hist;
}

/**
 * Tests the spread of the hashValues, using exponentially distributed keys.
 */
template<typename value_t>
std::map<std::uint64_t, std::uint32_t> test_exponential(multicore_hash::abstract_hash<value_t> *hash, std::uint32_t iterations, std::uint8_t key_len)
{
  // Pre-generating the strings, to only read time of actual hashing
  std::string* keys = new std::string[iterations];
  for(std::uint32_t i=0; i < iterations; i++)
    keys[i] = generate_random_string_exponential(key_len);

  // Calculating the hashing
  std::map<std::uint64_t, std::uint32_t> hist;
  for(std::uint32_t i=0; i < iterations; i++)
    ++hist[hash->get_hash(keys[i])];

  return hist;
}

/**
 * Tests the performance of the hashing, over different hash amounts, using normally distributed keys.
 * Returns the total amount of nanoseconds spent.
 */
template<typename value_t>
void test_amount_performance(multicore_hash::abstract_hash<value_t> *hash, std::uint32_t max_amount, std::uint8_t key_len)
{
  using namespace std::chrono;

  // Testing
  for (std::uint32_t amount = 1; amount <= max_amount; amount<<=1)
  {
      // Pre-generating the strings, to only read time of actual hashing
    std::string* keys = new std::string[amount];
    for(std::uint32_t i = 0; i < amount; i++)
      keys[i] = generate_random_string_uniform(key_len);

    // Calculating the hashing
    auto start = high_resolution_clock::now();
    for(std::uint32_t i = 0; i < amount; i++)
      hash->get_hash(keys[i]);
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
template<typename value_t>
void test_length_performance(multicore_hash::abstract_hash<value_t> *hash, std::uint32_t amount, const std::uint8_t max_key_len, std::ofstream *data_file) {
  using namespace std::chrono;
  // Warmup
  auto start = high_resolution_clock::now();
  generate_random_string_uniform(1);
  auto end = high_resolution_clock::now();

  std::uint16_t its = 10;

  std::uint32_t times[its][max_key_len];
  for (std::uint8_t i = 0; i < its; i++)
    for (std::uint8_t j = 0; j < max_key_len; j++)
      times[i][j] = 0;

  // Testing
  std::string* keys = new std::string[amount];
  for (std::uint8_t i = 0; i < its; i++) {
    std::cout << "Iteration " << (int)(i+1) << " of " << (int)its << std::endl;
    for (std::uint8_t k = 0; k < max_key_len; k++) {  
      // Pre-generating the strings, to only read time of actual hashing
      for(std::uint32_t j = 0; j < amount; j++)
        keys[j] = generate_random_string_uniform(k+1);

      // Calculating the hashing
      auto start = high_resolution_clock::now();
      for(std::uint32_t j = 0; j < amount; j++)
        hash->get_hash(keys[j]);
      auto end = high_resolution_clock::now();
      times[i][k] = duration_cast<nanoseconds>((end-start)/amount).count();
    }
  }  
  std::uint64_t time_means[max_key_len] = {0};
  double time_vars[max_key_len] = {0};
  // Sum
  for (std::uint8_t i = 0; i < its; i++) {
    for (std::uint8_t k = 0; k < max_key_len; k++) {
      time_means[k] += times[i][k];
    }
  }
  // Mean
  for (std::uint8_t k = 0; k < max_key_len; k++)
    time_means[k] = time_means[k]/its;

  // Variance
  for (std::uint8_t i = 0; i < its; i++) {
    for (std::uint8_t k = 0; k < max_key_len; k++) {
      time_vars[k] += (times[i][k] - time_means[k]) * (times[i][k] - time_means[k]);
    }
  }

  for (std::uint8_t k = 0; k < max_key_len; k++) {
    *data_file << std::to_string(k) + "\t" + std::to_string(time_means[k]) + "\t" + std::to_string(sqrt(time_vars[k]/its)) + "\n";
  }

  delete[] keys;

}

template<typename value_t>
std::uint8_t test_num_tables(multicore_hash::abstract_hash<value_t> *hash, std::string* keys, std::uint32_t amount) {
  auto start = std::chrono::high_resolution_clock::now();
  for(std::uint32_t a = 0; a < amount; a++)
    hash->get_hash(keys[a]);
  auto end = std::chrono::high_resolution_clock::now();

  return std::chrono::duration_cast<std::chrono::nanoseconds>((end-start)/amount).count();
}

void test_num_tables_performance() {
            std::uint8_t str_lens[]   = {1, 2, 4, 8, 12, 16, 20, 24, 28, 32, 36, 40, 44, 48, 52, 56, 60, 64};
  constexpr std::uint8_t num_tables[] = {1, 2, 4, 8, 12, 16, 20, 24, 28, 32, 36, 40, 44, 48, 56, 64};

  typedef std::uint32_t value_t;

  std::uint8_t rows = sizeof(num_tables)/sizeof(num_tables[0]); 
  std::uint8_t cols = sizeof(str_lens)/sizeof(str_lens[0]); 

  std::uint32_t time_sum[rows][cols];

  for (std::uint8_t i = 0; i < rows; i++)
    for (std::uint8_t j = 0; j < cols; j++)
      time_sum[i][j] = 0;


  std::uint32_t amount = 100000;
  std::uint8_t its = 100;

  // Calculating the tables for the tabulation
  multicore_hash::abstract_hash<std::uint32_t> *tabulation_hash;
  for (std::uint8_t i = 0; i < its; i++) {  
    std::cout << "Iteration " << (int)(i+1) << " of " << (int)its << std::endl;
    for (std::uint8_t j = 0; j < cols; j++) {
      // Pre-generating the strings, to only read time of actual hashing
      std::string* keys = new std::string[amount];
      for(std::uint32_t a = 0; a < amount; a++)
        keys[a] = generate_random_string_uniform(str_lens[j]);

      tabulation_hash = new multicore_hash::tabulation_hash<value_t, num_tables[0]>();
      time_sum[0][j] += test_num_tables<value_t>(tabulation_hash, keys, amount);
      delete tabulation_hash;

      tabulation_hash = new multicore_hash::tabulation_hash<value_t, num_tables[1]>();
      time_sum[1][j] += test_num_tables<value_t>(tabulation_hash, keys, amount);
      delete tabulation_hash;

      tabulation_hash = new multicore_hash::tabulation_hash<value_t, num_tables[2]>();
      time_sum[2][j] += test_num_tables<value_t>(tabulation_hash, keys, amount);
      delete tabulation_hash;

      tabulation_hash = new multicore_hash::tabulation_hash<value_t, num_tables[3]>();
      time_sum[3][j] += test_num_tables<value_t>(tabulation_hash, keys, amount);
      delete tabulation_hash;

      tabulation_hash = new multicore_hash::tabulation_hash<value_t, num_tables[4]>();
      time_sum[4][j] += test_num_tables<value_t>(tabulation_hash, keys, amount);
      delete tabulation_hash;

      tabulation_hash = new multicore_hash::tabulation_hash<value_t, num_tables[5]>();
      time_sum[5][j] += test_num_tables<value_t>(tabulation_hash, keys, amount);
      delete tabulation_hash;

      tabulation_hash = new multicore_hash::tabulation_hash<value_t, num_tables[6]>();
      time_sum[6][j] += test_num_tables<value_t>(tabulation_hash, keys, amount);
      delete tabulation_hash;

      tabulation_hash = new multicore_hash::tabulation_hash<value_t, num_tables[7]>();
      time_sum[7][j] += test_num_tables<value_t>(tabulation_hash, keys, amount);
      delete tabulation_hash;

      tabulation_hash = new multicore_hash::tabulation_hash<value_t, num_tables[8]>();
      time_sum[8][j] += test_num_tables<value_t>(tabulation_hash, keys, amount);
      delete tabulation_hash;

      tabulation_hash = new multicore_hash::tabulation_hash<value_t, num_tables[9]>();
      time_sum[9][j] += test_num_tables<value_t>(tabulation_hash, keys, amount);
      delete tabulation_hash;

      tabulation_hash = new multicore_hash::tabulation_hash<value_t, num_tables[10]>();
      time_sum[10][j] += test_num_tables<value_t>(tabulation_hash, keys, amount);
      delete tabulation_hash;

      tabulation_hash = new multicore_hash::tabulation_hash<value_t, num_tables[11]>();
      time_sum[11][j] += test_num_tables<value_t>(tabulation_hash, keys, amount);
      delete tabulation_hash;

      tabulation_hash = new multicore_hash::tabulation_hash<value_t, num_tables[12]>();
      time_sum[12][j] += test_num_tables<value_t>(tabulation_hash, keys, amount);
      delete tabulation_hash;

      tabulation_hash = new multicore_hash::tabulation_hash<value_t, num_tables[13]>();
      time_sum[13][j] += test_num_tables<value_t>(tabulation_hash, keys, amount);
      delete tabulation_hash;

      tabulation_hash = new multicore_hash::tabulation_hash<value_t, num_tables[14]>();
      time_sum[14][j] += test_num_tables<value_t>(tabulation_hash, keys, amount);
      delete tabulation_hash;

      tabulation_hash = new multicore_hash::tabulation_hash<value_t, num_tables[15]>();
      time_sum[15][j] += test_num_tables<value_t>(tabulation_hash, keys, amount);
      delete tabulation_hash;

      delete[] keys;
    }
  }
  std::cout << "\t";
  for (std::uint8_t j = 0; j < cols; j++)
    std::cout << (int)str_lens[j] << "\t";
  std::cout << std::endl;
  
  for (std::uint8_t i = 0; i < rows; i++) {
    std::cout << (int)num_tables[i] << "\t";
    for (std::uint8_t j = 0; j < cols; j++) {
      std::cout << time_sum[i][j]/its << "\t";
    }
    std::cout << std::endl;
  }
}

int main(int argc, char *argv[]) {
  using namespace std::chrono;

  // Argv[1] = Hash type
  // Argv[2] = Test type

  if (argc < 3)
  {
    std::cout << "You must provide Hash function to use and Test to run" << std::endl;
    std::cout << "Hash function options:" << std::endl;
    std::cout << "\t- 0: Mult" << std::endl;
    std::cout << "\t- 1: Murmur" << std::endl;
    std::cout << "\t- 2: Tabulation" << std::endl;
    std::cout << std::endl;
    std::cout << "Test options:" << std::endl;
    std::cout << "\t- 0: Uniform" << std::endl;
    std::cout << "\t- 1: Gaussian" << std::endl;
    std::cout << "\t- 2: Exponential" << std::endl;
    std::cout << "\t- 3: Length" << std::endl;
    return 0;
  }

    // Warmup
  auto start = high_resolution_clock::now();
  generate_random_string_uniform(100);
  auto end = high_resolution_clock::now();
  start = high_resolution_clock::now();

  typedef std::uint32_t value_t;
           
  const std::uint8_t num_tables = 64;
  std::uint8_t key_len          = 64;
  std::string test_type;
  std::string hash_type;

  multicore_hash::abstract_hash<value_t> *hash;

  /************************************************/
  /*********** Mult_Shift_Hash testing ************/
  /************************************************/
  switch ((int)(argv[1][0]-'0')) {
  case 0:  hash_type = "MultShift"; 
           hash = new multicore_hash::mult_shift_hash<value_t>();
           break;
  case 1:  hash_type = "Murmur"; 
           hash = new multicore_hash::murmur_hash<value_t>();
           break;
  case 2:  hash_type = "Tabulation"; 
           hash = new multicore_hash::tabulation_hash<value_t, num_tables>();
           
           // test_num_tables_performance();//<value_t, num_tables>(hash, 1000000, 64);
           break;
  default: std::cout << "Unknown Hash Type" << std::endl;
           return 0;
  }

  std::cout << hash_type << " Hashing Test" << std::endl;
  /****************************************/
  /*********** General testing ************/
  /****************************************/
  std::ofstream data_file;
  const uint16_t interval_amount = 256;
        uint32_t intervals[interval_amount] = {0};
  std::map<std::uint64_t, std::uint32_t> hist;

  switch ((int)(argv[2][0]-'0')) {
  case 0:  test_type = "_uniform_dist";
           hist = test_uniform(hash, 1000000, key_len);
           break;
  case 1:  test_type = "_gaussian_dist";
           hist = test_gaussian(hash, 1000000, key_len);
           break;
  case 2:  test_type = "_exponential_dist";
           hist = test_exponential(hash, 1000000, key_len);
           break;
  case 3:  test_type = "_length";
           data_file.open ("Results/" + hash_type + test_type + std::to_string(key_len) + ".txt");
           test_length_performance<value_t>(hash, 100000, key_len, &data_file);
           data_file.close();
           break;
  default: std::cout << "Unknown Test Type" << std::endl;
           return 0;
  }

  if ((int)(argv[2][0]-'0') <= 2)
  {
    for(auto p : hist)
      intervals[p.first/(std::numeric_limits<value_t>::max()/interval_amount)] += p.second;
    
    data_file.open ("Results/" + hash_type + test_type + ".txt");

    // Printing histogram results

    std::cout << "Writing to file: " << "Results/" + hash_type + test_type + ".txt" << std::endl;
    std::map<std::uint8_t, std::uint32_t> bin_hist;
    for (std::uint16_t i = 0; i < interval_amount; i++)
    {
      data_file << i << "\t" << intervals[i] << "\n";
      bin_hist[i] = intervals[i];
    }
    data_file.close();
    print_spread_statistics(bin_hist);
  }

  end = high_resolution_clock::now();
  std::cout << "Total test time: " << duration_cast<microseconds>((end-start)/1000).count() << "ms" << std::endl;
  return 0;
}