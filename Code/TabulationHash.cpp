#include "TabulationHash.h"

#include <stdio.h>
#include <stdlib.h>
#include <time.h>
#include <math.h>
#include <bitset>
#include <iostream>
#include <sstream>


using namespace std;

TabulationHash::TabulationHash(void)
{
	for (unsigned i = 0; i < stringLen; i++)
	{
		srand( time(NULL) );
		for (HashEntry &entry : tabulationTables[i])
		{
			entry.hashValue = rand() % (1UL<<(sizeof(value_t) * 8) - 1);
			// printf("%u\n", entry.hashValue);
		}
	}
}
TabulationHash::~TabulationHash(void) {}

value_t TabulationHash::getHash(string key)
{
	value_t hashResult = 0;
	hashResult = 0;
	for (int i = 0; i < key.size(); i += charLength)
	{
		int index = 0;
		for (int j = 0; j < charLength; j++)
		{
			index += ((int)key[i+j])<<(sizeof(char)*8*j);
		}
		hashResult ^= tabulationTables[i][index].hashValue;
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