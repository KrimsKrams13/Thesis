#include "TabulationHash.h"

#include <string>
#include <chrono>
#include <iostream>
#include <algorithm>
#include <climits>

#include <map>
using namespace std;

string generateRandomStringUniform(unsigned len, unsigned minV = 0, unsigned maxV = (1<<CHAR_BIT)-1)
{
	char res[len];
	random_device rd;
	mt19937 generator(rd());
	uniform_int_distribution<> distribution(minV, maxV);
	for (unsigned i = 0; i < len; i++)
		res[i] = (unsigned char)distribution(generator);
	
	string resStr(res);
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
	
	string resStr(res);
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

	return resStr;
}

string generateRandomStringExponential(unsigned len)
{
	char res[len];
	unsigned long n;

	random_device rd;
	mt19937 generator(rd());
  exponential_distribution<double> distribution(16);
	for (unsigned i = 0; i < len; i++)
	{
		res[i] = (unsigned char)(round(distribution(generator)*(1<<CHAR_BIT)));
	}
	
	string resStr(res);
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

	return resStr;
}

void printSpreadStatistics(map<unsigned, int> hist)
{

	return 0;
}

int main()
{
	TabulationHash *tabulationHash = new TabulationHash();

  map<unsigned, int> hist;
  for(int n=0; n<10000; ++n) {
		value_t value = tabulationHash->getHash(generateRandomStringExponential(16));
    ++hist[value];
  }
  for(auto p : hist) {
    cout << p.first << ' ' << string(p.second/5, '*') << '\n';
  }
  return 0;
}