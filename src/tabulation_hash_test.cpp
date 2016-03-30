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
#include <thread>
#include <pthread.h>

int stick_thread_to_core(pthread_t thread, int core_id) {
   int num_cores = 32;
   if (core_id < 0 || core_id >= num_cores)
      return EINVAL;

   cpu_set_t cpuset;
   CPU_ZERO(&cpuset);
   CPU_SET(core_id, &cpuset);

   return pthread_setaffinity_np(thread, sizeof(cpu_set_t), &cpuset);
}


std::string generate_random_string_uniform(std::uint8_t byte_len, std::uint8_t min_val = 0, std::uint64_t max_val = ULLONG_MAX)
{
  const char* res;
  std::random_device rd;
  std::mt19937_64 generator(rd());
  std::uniform_int_distribution<std::uint64_t> distribution(min_val, max_val);

  std::uint64_t int_res = distribution(generator);
  res = reinterpret_cast<const char*>(&int_res);

  std::string res_str = std::string(res, byte_len);

  return res_str;
}

std::string* generate_random_strings_uniform(std::uint8_t byte_len, std::uint32_t n, std::uint8_t min_val = 0, std::uint64_t max_val = ULLONG_MAX)
{
  std::random_device rd;
  std::mt19937_64 generator(rd());
  std::uniform_int_distribution<std::uint64_t> distribution(min_val, max_val);

  std::string* res_strs = new std::string[n];
  const char* res;
  std::uint64_t int_res;
  for (std::uint32_t i = 0; i < n; i++)
  {
    int_res = distribution(generator);
    res = reinterpret_cast<const char*>(&int_res);

    res_strs[i] = std::string(res, byte_len);
  }

  return res_strs;
}

std::string* generate_random_strings_gaussian(std::uint8_t byte_len, std::uint32_t n, std::uint64_t sigma = LLONG_MAX/4, std::uint64_t mu = LLONG_MAX)
{
  std::random_device rd;
  std::mt19937_64 generator(rd());
  std::normal_distribution<double> distribution(mu, sigma);

  const char* res;
  double double_res;
  std::string *res_strs = new std::string[n];

  for (std::uint32_t i = 0; i < n; i++)
  {
    double_res = distribution(generator);
    res = reinterpret_cast<const char*>(&double_res);

    res_strs[i] = std::string(res, byte_len);
  }
  return res_strs;
}

std::string* generate_random_strings_exponential(std::uint8_t byte_len, std::uint32_t n, double lambda = 16.0)
{
  std::random_device rd;
  std::mt19937_64 generator(rd());
  std::exponential_distribution<double> distribution(lambda);

  const char* res;
  double double_res;
  std::string* res_strs = new std::string[n];
  
  for (std::uint32_t i = 0; i < n; i++)
  {
    double_res = distribution(generator) * (1UL << (byte_len*8));
    res = reinterpret_cast<const char*>(&double_res);

    res_strs[i] = std::string(res, byte_len);
  }

  return res_strs;
}

std::string* generate_random_strings_length(std::uint8_t byte_len, std::uint32_t n, std::uint8_t min_val = 0, std::uint64_t max_val = ULLONG_MAX)
{
  std::random_device rd;
  std::mt19937_64 generator(rd());
  std::uniform_int_distribution<std::uint64_t> distribution(min_val, max_val);

  std::uint8_t nchunks = ((byte_len-1)/8) + 1;

  std::string* res_strs = new std::string[n];
  const char** res = new const char*[nchunks];
  std::uint64_t *int_res = new std::uint64_t[nchunks];
  for (std::uint32_t i = 0; i < n; i++)
  {
    for (std::uint32_t c = 0; c < nchunks; c++)
    {
      int_res[c] = distribution(generator);
      res[0] = reinterpret_cast<char*>(&int_res[0]);
    }

    res_strs[i] = std::string(res[0], byte_len);
  }
  return res_strs;
}







template<typename value_t>
void print_spread_statistics(std::map<value_t, std::uint32_t> hist)
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
std::map<value_t, std::uint32_t> test_uniform(multicore_hash::abstract_hash<value_t> *hash, std::uint32_t iterations, std::uint8_t key_len)
{
  // Pre-generating the strings, to only read time of actual hashing
  std::string* keys = generate_random_strings_uniform(key_len, iterations);
  // for (int i = 0; i < iterations; i++)
  // Calculating the hashing
  std::map<value_t, std::uint32_t> hist;
  for(std::uint32_t i=0; i < iterations; i++)
    ++hist[hash->get_hash(keys[i])];
    // std::cout << hash->get_hash(keys[i]) << std::endl;

  return hist;
}

/**
 * Tests the spread of the hashValues, using Normally distributed keys.
 */
template<typename value_t>
std::map<value_t, std::uint32_t> test_gaussian(multicore_hash::abstract_hash<value_t> *hash, std::uint32_t iterations, std::uint8_t key_len)
{
  // Pre-generating the strings, to only read time of actual hashing
  std::string* keys = generate_random_strings_gaussian(key_len, iterations);

  // Calculating the hashing
  std::map<value_t, std::uint32_t> hist;
  for(std::uint32_t i=0; i < iterations; i++)
    ++hist[hash->get_hash(keys[i])];

  return hist;
}

/**
 * Tests the spread of the hashValues, using exponentially distributed keys.
 */
template<typename value_t>
std::map<value_t, std::uint32_t> test_exponential(multicore_hash::abstract_hash<value_t> *hash, std::uint32_t iterations, std::uint8_t key_len)
{
  // Pre-generating the strings, to only read time of actual hashing
  std::string* keys = generate_random_strings_exponential(key_len, iterations);

  // Calculating the hashing
  std::map<value_t, std::uint32_t> hist;
  for(std::uint32_t i=0; i < iterations; i++)
    ++hist[hash->get_hash(keys[i])];

  return hist;
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

  std::uint32_t its     =     100;
  std::uint32_t repeats = 1000000;

  std::cout << 1 << std::endl;

  std::uint32_t** times = new std::uint32_t*[its];
  for (std::uint32_t i = 0; i < its; i++)
  {
    times[i] = new std::uint32_t[max_key_len];
    for (std::uint8_t j = 0; j < max_key_len; j++)
      times[i][j] = 0;
  }

  std::cout << 2 << std::endl;

  // Pre-generating the strings, to only read time of actual hashing
  std::string** keys = new std::string*[max_key_len];
  for (std::uint8_t k = 0; k < max_key_len; k++)
    keys[k] = generate_random_strings_uniform(k+1, amount);
    // for(std::uint32_t j = 0; j < amount; j++)
      // keys[k][j] = generate_random_string_uniform(k+1);

  std::cout << 3 << std::endl;
  // Testing

  for (std::uint32_t i2 = 0; i2 < its; i2++) {
    std::cout << "Iteration " << (int)(i2+1) << " of " << (int)its << std::endl;
    for (std::uint8_t k = 0; k < max_key_len; k++) {  
      // Warmup 
      for(std::uint32_t j = 0; j < amount; j++)
        hash->get_hash(keys[k][j]);

      auto start = high_resolution_clock::now();
      // Calculating the hashing
      for (std::uint32_t i = 0; i < repeats; i++) {    
        for(std::uint32_t j = 0; j < amount; j++)
          hash->get_hash(keys[k][j]);
      }
      auto end = high_resolution_clock::now();
      times[i2][k] = duration_cast<nanoseconds>((end-start)/(amount*repeats)).count();
    }
  }  
  std::cout << 4 << std::endl;
  std::uint64_t time_means[max_key_len];
  double time_vars[max_key_len];
  for (std::uint8_t k = 0; k < max_key_len; k++) {
    time_means[k] = 0;
    time_vars[k] = 0;
  }
  // Sum
  for (std::uint32_t i = 0; i < its; i++) {
    for (std::uint8_t k = 0; k < max_key_len; k++) {
      time_means[k] += times[i][k];
    }
  }
  // Mean
  for (std::uint8_t k = 0; k < max_key_len; k++) {
    time_means[k] = time_means[k]/its;
  }

  // Variance
  for (std::uint32_t i = 0; i < its; i++) {
    for (std::uint8_t k = 0; k < max_key_len; k++) {
      time_vars[k] += (times[i][k] - time_means[k]) * (times[i][k] - time_means[k]);
    }
  }

  for (std::uint8_t k = 0; k < max_key_len; k++) {
    *data_file << std::to_string(k) + "\t" + std::to_string(time_means[k]) + "\t" + std::to_string(sqrt(time_vars[k]/its)) + "\n";
  }
  for (std::uint8_t k = 0; k < max_key_len; k++)
    delete[] keys[k];
  delete[] keys;
}

/**
 * Tests the performance of the hashing, over different string lengths, using normally distributed keys.
 * Returns the total amount of nanoseconds spent.
 */
template<typename value_t, std::uint32_t num_tables>
void test_length_performance_tab(multicore_hash::tabulation_hash<value_t, num_tables> *hash, std::uint32_t amount, const std::uint8_t max_key_len, std::ofstream *data_file) {
  using namespace std::chrono;
  // Warmup
  auto start = high_resolution_clock::now();
 generate_random_strings_uniform(1, 1000);
  auto end = high_resolution_clock::now();

  std::uint32_t its     = 10000;
  std::uint32_t repeats =  1000;
  std::uint8_t  stride  = 4;
  std::uint8_t  used_length_amounts = max_key_len/stride;

  std::cout << 1 << std::endl;

  std::uint32_t** times = new std::uint32_t*[its];
  for (std::uint32_t i = 0; i < its; i++)
  {
    times[i] = new std::uint32_t[used_length_amounts];
    for (std::uint8_t k = 0; k < used_length_amounts; k++)
      times[i][k] = 0;
  }

  std::cout << 2 << std::endl;

  // Testing
  std::string *keys = new std::string[amount];
  for (std::uint32_t i2 = 0; i2 < its; i2++) {
    std::cout << "Iteration " << (int)(i2+1) << " of " << (int)its << std::endl;
    // Pre-generating the strings, to only read time of actual hashing

    for (std::uint8_t k = 0; k < used_length_amounts; k++) {  
      keys = generate_random_strings_length((k*stride)+1, amount);

      // Warmup 
      for(std::uint32_t j = 0; j < amount; j++)
        hash->get_hash(keys[j]);

      auto start = high_resolution_clock::now();
      // Calculating the hashing
      for (std::uint32_t i = 0; i < repeats; i++) {    
        for(std::uint32_t j = 0; j < amount; j++)
          hash->get_hash(keys[j]);
      }
      auto end = high_resolution_clock::now();
      times[i2][k] = duration_cast<nanoseconds>((end-start)/(amount*repeats)).count();
    }
  }  
  std::cout << 4 << std::endl;
  std::uint64_t time_means[used_length_amounts];
  double time_vars[used_length_amounts];
  for (std::uint8_t k = 0; k < used_length_amounts; k++) {
    time_means[k] = 0;
    time_vars[k] = 0;
  }
  // Sum
  for (std::uint32_t i = 0; i < its; i++) {
    for (std::uint8_t k = 0; k < used_length_amounts; k++) {
      time_means[k] += times[i][k];
    }
  }
  // Mean
  for (std::uint8_t k = 0; k < used_length_amounts; k++) {
    time_means[k] = time_means[k]/its;
  }

  // Variance
  for (std::uint32_t i = 0; i < its; i++) {
    for (std::uint8_t k = 0; k < used_length_amounts; k++) {
      time_vars[k] += (times[i][k] - time_means[k]) * (times[i][k] - time_means[k]);
    }
  }

  for (std::uint8_t k = 0; k < used_length_amounts; k++) {
    *data_file << std::to_string((k*stride)+1) + "\t" + std::to_string(time_means[k]) + "\t" + std::to_string(sqrt(time_vars[k]/its)) + "\n";
  }
  delete[] keys;
}

template<typename value_t, std::uint32_t num_tables>
void test_core_performance_tab(multicore_hash::tabulation_hash<value_t, num_tables> *hash, std::string *keys, std::uint32_t amount, std::uint32_t iterations) {
  // Calculating the hashing
  for (std::uint32_t i = 0; i < iterations; i++)
    for(std::uint32_t j = 0; j < amount; j++)
      hash->get_hash(keys[j]);
}

template<typename value_t, std::uint32_t num_tables>
void test_cores_performance_tab(multicore_hash::tabulation_hash<value_t, num_tables> *hash, std::uint8_t num_threads, std::uint8_t byte_len, std::uint32_t amount) {
  using namespace std::chrono;
  // Warmup
  auto start = high_resolution_clock::now();
 generate_random_strings_uniform(1, 1000);
  auto end = high_resolution_clock::now();

  auto tab_hash = new multicore_hash::tabulation_hash<value_t, num_tables>();           

  // Testing
  std::string *keys = new std::string[amount*num_threads];
  keys = generate_random_strings_length(byte_len, amount*num_threads);

  // Warmup 
  for(std::uint32_t j = 0; j < amount; j++)
    hash->get_hash(keys[j]);

  std::thread threads[num_threads];
  std::uint32_t iterations = 1000000;
  std::uint64_t total_time = 0;

  start = high_resolution_clock::now();
  // Calculating the hashing
  for(std::uint32_t t = 0; t < num_threads; t++) {
    threads[t] = std::thread(test_core_performance_tab<value_t, num_tables>, tab_hash, &keys[t*amount], amount, iterations);
    stick_thread_to_core(threads[t].native_handle(), t+1);
  }
  for(std::uint32_t t = 0; t < num_threads; t++)
    threads[t].join();
  end = high_resolution_clock::now();
  total_time = duration_cast<milliseconds>(end-start).count();
  
  std::cout << "Time per thread: " << total_time/num_threads << "ms / " << (uint64_t)(amount*iterations*1000)/total_time << " TP." <<  std::endl;
  std::cout << "Total Time:      " << total_time << "ms / " << (uint64_t)(amount*num_threads*iterations*1000)/total_time << " TP."<<  std::endl;

  delete[] keys;
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
    std::cout << "\t- 4: Cores" << std::endl;
    return 0;
  }

    // Warmup
  auto start = high_resolution_clock::now();
  generate_random_strings_length(63, 100);
  auto end = high_resolution_clock::now();
  // start = high_resolution_clock::now();

  typedef std::uint32_t value_t;
           
  const std::uint8_t num_tables    = (NUM_TABLES_DEF == -1 ? 8 : NUM_TABLES_DEF);
        std::uint8_t key_bytes     = 8;                                     
        std::uint8_t key_len_bytes = 64;                                     
        std::string test_type;
        std::string hash_type;

  multicore_hash::abstract_hash<value_t> *hash;
  multicore_hash::tabulation_hash<value_t, num_tables> *tab_hash;

  /************************************************/
  /*********** Mult_Shift_Hash testing ************/
  /************************************************/
  switch ((int)(argv[1][0]-'0')) {
  case 0:   hash_type = "MultShift"; 
            hash = new multicore_hash::mult_shift_hash<value_t>();
            break;
  case 1:   hash_type = "Murmur"; 
            hash = new multicore_hash::murmur_hash<value_t>();
            break;
  case 2:   hash_type = "Tabulation"; 
            hash = new multicore_hash::tabulation_hash<value_t, num_tables>();
            tab_hash = new multicore_hash::tabulation_hash<value_t, num_tables>();           
            // test_num_tables_performance();//<value_t, num_tables>(hash, 1000000, 64);
            break;
  default:  std::cout << "Unknown Hash Type" << std::endl;
            return 0;
  }

  std::cout << hash_type << " Hashing Test" << std::endl;
  /****************************************/
  /*********** General testing ************/
  /****************************************/
  std::ofstream data_file;
  const uint16_t interval_amount = 256;
        uint32_t intervals[interval_amount];
  for (std::uint8_t i = 0; i < interval_amount; i++)
    intervals[i] = 0;

  std::map<value_t, std::uint32_t> hist;

  std::uint32_t dist_amount = 5000000 ;
  std::uint32_t len_amount  = 5;
  std::uint8_t  num_threads = 7;

  switch ((int)(argv[2][0]-'0')) {
  case 0:   test_type = "_uniform_dist";
            hist = test_uniform(hash, dist_amount, key_bytes);
            break;
  case 1:   test_type = "_gaussian_dist";
            hist = test_gaussian(hash, dist_amount, key_bytes);
            break;
  case 2:   test_type = "_exponential_dist";
            hist = test_exponential(hash, dist_amount, key_bytes);
            break;
  case 3:   test_type = "_length";
            data_file.open ("Results/Length/" + hash_type + std::to_string(num_tables) + test_type + ".txt");
            if (hash_type == "Tabulation")
              test_length_performance_tab<value_t, num_tables>(tab_hash, len_amount, key_len_bytes, &data_file);
            else
              test_length_performance<value_t>(hash, len_amount, key_len_bytes, &data_file);
            data_file.flush();
            data_file.close();
            break;
  case 4:   test_type = "_cores";
            // data_file.open ("Results/Cores/" + hash_type + std::to_string(num_tables) + test_type + ".txt");
            // if (hash_type == "Tabulation")
            for (uint8_t nt = 1; nt <= num_threads; nt++) {
              std::cout << "Using " << (int)nt << " threads:" << std::endl;
              test_cores_performance_tab<value_t, num_tables>(tab_hash, nt, key_len_bytes, len_amount);
            }
            // else
              // test_length_performance<value_t>(hash, len_amount, key_len_bytes, &data_file);
            // data_file.flush();
            // data_file.close();
            break;
  default:  std::cout << "Unknown Test Type" << std::endl;
            return 0;
  }

  if ((int)(argv[2][0]-'0') <= 2)
  {
    for(auto p : hist)
      intervals[p.first/(std::numeric_limits<value_t>::max()/interval_amount)] += p.second;
    
    std::string path = "Results/Distribution/" + hash_type + test_type + ".txt";
    data_file.open (path);
    data_file.clear();
    if (!data_file.is_open())
      std::cout << "Data file isn't open." << std::endl;

    // Printing histogram results

    std::cout << "Writing to file: " << path << std::endl;
    std::map<std::uint8_t, std::uint32_t> bin_hist;
    for (std::uint16_t i = 0; i < interval_amount; i++)
    {
      data_file << i << "\t" << intervals[i] << "\n";
      bin_hist[i] = intervals[i];
    }
    data_file.flush();
    if (data_file.fail())
      std::cout << "Something failed" << std::endl;
    data_file.close();
    print_spread_statistics<std::uint8_t>(bin_hist);
  }

  // end = high_resolution_clock::now();
  // std::cout << "Total test time: " << duration_cast<microseconds>((end-start)/1000).count() << "ms" << std::endl;
  return 0;
}