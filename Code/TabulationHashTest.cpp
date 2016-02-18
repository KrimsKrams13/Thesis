#include "TabulationHash.h"

#include <string>
#include <chrono>
#include <iostream>
#include <iomanip>
#include <algorithm>
#include <climits>

#include <map>
using namespace std;

string generateRandomStringUniform(unsigned len, unsigned minV = 1, unsigned maxV = (1<<CHAR_BIT)-1)
{
	char res[len];
	random_device rd;
	mt19937 generator(rd());
	uniform_int_distribution<> distribution(minV, maxV);
	for (unsigned i = 0; i < len; i++)
	{
		res[i] = (char)distribution(generator);
	}

	string resStr = string(res, len);
	// cout << resStr;

	/*** VISUALIZATION OF DISTRIBUTION ***/
  // map<int, int> hist;
  // for(int n=0; n<10000; ++n) {
  //     ++hist[distribution(generator)];
  // }
  // for(auto p : hist) {
  //   cout << p.first << ' ' << string(p.second/5, '*') << '\n';
  // }
  /*** END OF VISUALIZATION ***/
  if (resStr.size() != len)
  	cout << "WTF " << resStr.size()  << ' ' << (sizeof(res)/sizeof(res[0])) << endl;
	return resStr;
}

string generateRandomStringGaussian(unsigned len, int sigma = 24, int mu = 128)
{
	char res[len];

	random_device rd;
	mt19937 generator(rd());
	normal_distribution<> distribution(mu, sigma);
	for (unsigned i = 0; i < len; i++)
		res[i] = (unsigned char)(round(distribution(generator)));

	string resStr = string(res, len);
	// cout << resStr;

  /*** VISUALIZATION OF DISTRIBUTION ***/
  // map<int, int> hist;
  // for(int n=0; n<10000; ++n) {
  //   ++hist[round(distribution(generator))];
  // }
  // for(auto p : hist) {
  //   cout << p.first << ' ' << string(p.second/5, '*') << '\n';
  // }
  /*** END OF VISUALIZATION ***/

  if (resStr.size() != len)
  	cout << "WTF" << resStr.size() << endl;
	return resStr;
}

string generateRandomStringExponential(unsigned len)
{
	char res[len];
	unsigned n;

	random_device rd;
	mt19937 generator(rd());
  exponential_distribution<double> distribution(16);
	for (unsigned i = 0; i < len; i++)
	{
		do
			n = (round(distribution(generator)*(1<<CHAR_BIT))) + 1; // Added 1 to avoid (char)0 = end of string.
		while (n > 255);
		res[i] = (unsigned char)n;
	}

	string resStr = string(res, len);
	// cout << resStr;

  /*** VISUALIZATION OF DISTRIBUTION ***/
  // const int nrolls=10000;   // number of experiments
  // const int nintervals=256; // number of intervals
  // int p[nintervals]={};

  // for (int i=0; i<nrolls; ++i) {
  //   double number = distribution(generator);
  //   if (number<1.0) ++p[int(nintervals*number)];
  // }

  // for (int i=0; i<nintervals; ++i) {
  //   cout << i << ": ";
  //   cout << string(p[i]/10,'*') << endl;
  // }
  /*** END OF VISUALIZATION ***/
  if (resStr.size() != len)
  	cout << "WTF" << resStr.size() << endl;
	return resStr;
}

void printSpreadStatistics(map<unsigned, int> hist)
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

  cout << "Variance: ";
  cout << setprecision(5) << var << endl;
  cout << "Mean: ";
  cout << setprecision(5) << mean << endl;
  cout << "Min value: " << min << endl;
  cout << "Max value: " << max << endl;
}

/**
 * Tests the spread of the hashValues, using Uniformly distributed keys.
 */
void testUniform(TabulationHash *tabulationHash, unsigned iterations, unsigned strLen)
{
	// Pre-generating the strings, to only read time of actual hashing
	string keys[iterations];
	for(int i=0; i < iterations; i++)
		keys[i] = generateRandomStringUniform(strLen);

	// Calculating the hashing
  map<unsigned, int> hist;
  for(int i=0; i < iterations; i++)
    ++hist[tabulationHash->getHash(keys[i])];

  // Calculating statistics on the found hashes.
  int intervalAmount = 256;
  int intervals[intervalAmount] = {0};
  	// Creating intervals for 'readable' histogram
  for(auto p : hist)
  	intervals[p.first/(tabulationHash->getMaxHashValue()/intervalAmount)] += p.second;

  	// Printing histogram results
  for (int i = 0; i < 256; i++)
    cout << i << ' ' << string(intervals[i]/(iterations/(32*intervalAmount)), '*') << ' ' << intervals[i] << endl;
  printSpreadStatistics(hist);
}

/**
 * Tests the spread of the hashValues, using Normally distributed keys.
 */
void testGaussian(TabulationHash *tabulationHash, unsigned iterations, unsigned strLen)
{
	// Pre-generating the strings, to only read time of actual hashing
	string keys[iterations];
	for(int i=0; i < iterations; i++)
		keys[i] = generateRandomStringGaussian(strLen);

	// Calculating the hashing
  map<unsigned, int> hist;
  for(int i=0; i < iterations; i++)
    ++hist[tabulationHash->getHash(keys[i])];

  // Calculating statistics on the found hashes.
  int intervalAmount = 256;
  int intervals[intervalAmount] = {0};
  	// Creating intervals for 'readable' histogram
  for(auto p : hist)
  	intervals[p.first/(tabulationHash->getMaxHashValue()/intervalAmount)] += p.second;

  	// Printing histogram results
  for (int i = 0; i < 256; i++)
    cout << i << ' ' << string(intervals[i]/(iterations/(32*intervalAmount)), '*') << ' ' << intervals[i] << endl;
  printSpreadStatistics(hist);
}

/**
 * Tests the spread of the hashValues, using exponentially distributed keys.
 */
void testExponential(TabulationHash *tabulationHash, unsigned iterations, unsigned strLen)
{
	// Pre-generating the strings, to only read time of actual hashing
	string keys[iterations];
	for(int i=0; i < iterations; i++)
		keys[i] = generateRandomStringExponential(strLen);

	// Calculating the hashing
  map<unsigned, int> hist;
  for(int i=0; i < iterations; i++)
    ++hist[tabulationHash->getHash(keys[i])];

  // Calculating statistics on the found hashes.
  int intervalAmount = 256;
  int intervals[intervalAmount] = {0};
  	// Creating intervals for 'readable' histogram
  for(auto p : hist)
  	intervals[p.first/(tabulationHash->getMaxHashValue()/intervalAmount)] += p.second;

  	// Printing histogram results
  for (int i = 0; i < 256; i++)
    cout << i << ' ' << string(intervals[i]/(iterations/(32*intervalAmount)), '*') << ' ' << intervals[i] << endl;
  printSpreadStatistics(hist);
}

/**
 * Tests the performance of the hashing, over different hash amounts, using normally distributed keys.
 * Returns the total amount of nanoseconds spent.
 */
void testAmountPerformance(TabulationHash *tabulationHash, unsigned maxAmount, unsigned strLen)
{
	using namespace std::chrono;

	// Warmup
	auto start = high_resolution_clock::now();
	generateRandomStringUniform(strLen);
	auto end = high_resolution_clock::now();

	// Testing
	for (unsigned amount = 1; amount <= maxAmount; amount<<=1)
	{
			// Pre-generating the strings, to only read time of actual hashing
		string keys[amount];
		for(int i=0; i < amount; i++)
			keys[i] = generateRandomStringUniform(strLen);

		// Calculating the hashing
		auto start = high_resolution_clock::now();
	  for(int i=0; i < amount; i++)
	    tabulationHash->getHash(keys[i]);
		auto end = high_resolution_clock::now();
		cout << "When hashing: " << amount << "keys, time spent per hashing: " 
				 << duration_cast<nanoseconds>((end-start)/amount).count() << "ns." << endl;
	}
}

/**
 * Tests the performance of the hashing, over different string lengths, using normally distributed keys.
 * Returns the total amount of nanoseconds spent.
 */
void testLengthPerformance(TabulationHash *tabulationHash, unsigned amount, unsigned maxStrLen)
{
	using namespace std::chrono;

	// Warmup
	auto start = high_resolution_clock::now();
	generateRandomStringUniform(1);
	auto end = high_resolution_clock::now();

	// Testing
	string keys[amount];
	for (unsigned strLen = 1; strLen <= maxStrLen; strLen++)
	{
			// Pre-generating the strings, to only read time of actual hashing
		for(int i=0; i < amount; i++)
			keys[i] = generateRandomStringUniform(strLen);

		// Calculating the hashing
		auto start = high_resolution_clock::now();
	  for(int i=0; i < amount; i++)
	    tabulationHash->getHash(keys[i]);
		auto end = high_resolution_clock::now();
		cout << "When hashing " << amount << " keys of length: " << strLen << ", time spent per hashing: " 
				 << duration_cast<nanoseconds>((end-start)/amount).count() << "ns." << endl;
	}
}

int main()
{
	// Calculating the tables for the tabulation
	TabulationHash *tabulationHash = new TabulationHash(16);

	// testUniform(tabulationHash, 1000000, 4);
	// testGaussian(tabulationHash, 1000000, 4);
	// testExponential(tabulationHash, 1000000, 4);
	testAmountPerformance(tabulationHash, 1000000, 4);
	// testLengthPerformance(tabulationHash, 100000, 20);

  return 0;
}