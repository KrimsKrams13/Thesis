#include "TabulationHash.h"

#include <iostream>

using namespace std;

TabulationHash::TabulationHash(void)
{

	srand(time(NULL));
	for (unsigned i = 0; i < stringLen; i++)
	{
		for (HashEntry &entry : tabulationTables[i])
		{
			entry.hashValue = rand() % (1UL<<(sizeof(value_t) * 2) - 1);
		}
	}
}
TabulationHash::~TabulationHash(void) {}

value_t TabulationHash::getHash(string key)
{
	value_t hashResult = 0;
	hashResult = 0;
	bool printendl = false;
	for (int i = 0; i < key.size(); i += charLength)
	{
		// int index = 0;
		// for (int j = 0; j < charLength; j++)
		// {
			// index += ((int)key[i+j])<<(sizeof(char)*8*j);
		// }
		hashResult ^= tabulationTables[i][((int)(key[i] + 128))].hashValue; //Silly hack to solve signed char in string problem
	}

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