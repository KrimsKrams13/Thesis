#include "TabulationHash.h"

#include <stdio.h>
#include <stdlib.h>
#include <time.h>
#include <math.h>
#include <bitset>
#include <iostream>
#include <sstream>
#include <climits>

#include <chrono>

using namespace std;

TabulationHash::TabulationHash(void)
{
	srand( time(NULL) );
	for (HashEntry &entry : tabulationTable)
	{
		entry.hashValue = rand() % (1UL<<(sizeof(value_t) * 8) - 1);
		// printf("%u\n", entry.hashValue);
	}
}
TabulationHash::~TabulationHash(void) {}

value_t TabulationHash::getHash(string key)
{
	value_t hashResult = 0;
	hashResult = 0;
	for (int i = 0; i < key.size(); i += char_length)
	{
		int index = 0;
		for (int j = 0; j < char_length; j++)
		{
			index += ((int)key[i+j])<<(sizeof(char)*8*j);
		}
		hashResult ^= tabulationTable[index].hashValue;
	}
	// printf("%u\n", hashResult);

	/************************
	*** Use if c % 8 != 0 ***
	************************/

	// ostringstream oss;
	// unsigned long mybit_ul;
	// for(auto c : key) {
	//   mybit_ul = (std::bitset<8>(c).to_ulong());
	//   printf("%lu\n", mybit_ul);
	// }
	// cout << oss.str() << endl;

	/************************
	***        End        ***
	************************/	

	return hashResult;
}

int main(void)
{
	// string hello = "Hell";
	// cout << bitset<32>(hello) << endl;
	TabulationHash *tabulationHash = new TabulationHash();
	string key = "aasd";
	
	// WARMUP
	for (unsigned i = 0; i < 100; i++)
		value_t hashValue = tabulationHash->getHash(key);

	unsigned amount = 1000;
	
	for (unsigned j = 0; j < 100; j++)
	{
		auto start = chrono::high_resolution_clock::now();

		for (unsigned i = 0; i < amount; i++)
			value_t hashValue = tabulationHash->getHash(key);
		
		auto end = chrono::high_resolution_clock::now();
		std::cout << std::chrono::duration_cast<std::chrono::nanoseconds>((end-start)/amount).count() << "ns\n";  
	}
	// for (auto hashEntry : tabulationHash->tabulationTable)
		// printf("%u\n", hashEntry.hashValue);
}
