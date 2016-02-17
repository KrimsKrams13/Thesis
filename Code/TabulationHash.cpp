#include "TabulationHash.h"

#include <iostream>
#include <time.h>
#include <algorithm>

using namespace std;

TabulationHash::TabulationHash(unsigned maxStrLen)
{

  mt19937 generator(time(NULL));

	// Initialize the tables to fit the given maximum string length.
	tabulationTables = new HashEntry*[maxStrLen];
	for (unsigned i = 0; i < maxStrLen; i++)
		tabulationTables[i] = new HashEntry[entries];

	// Setup tabulation tables, used for the hashing.
	unsigned ttsRows = (sizeof(tabulationTables)/sizeof(tabulationTables[0]));
	unsigned ttsCols = (sizeof(tabulationTables[0])/sizeof(tabulationTables[0][0]));
	for (unsigned i = 0; i < maxStrLen; i++)
	{
		for (unsigned j = 0; j < ttsCols; j++)
		{
			// IF the max string length is shorter than the amount of random tables, use those.
			if (maxStrLen <= ttsRows)
				tabulationTables[i][j].hashValue =1;
			// Otherwise, calculate table entries using pseudorandom generator.
			else
				tabulationTables[i][j].hashValue = generator() % (1UL<<(sizeof(value_t) * 8) - 1);
		}
	}
}
TabulationHash::~TabulationHash(void)
{
	unsigned ttsRows = (sizeof(tabulationTables)/sizeof(tabulationTables[0]));
	for (unsigned i = 0; i < ttsRows; i++)
		delete[] tabulationTables[i];
	delete[] tabulationTables;
}

value_t TabulationHash::getHash(string key)
{
	// Possible solution for the silly hack below.
	// const unsigned char * ukey = reinterpret_cast<const unsigned char *> (strVar.c_str());
	value_t hashResult = 0;
	for (int i = 0; i < key.size(); i += charLength)
		hashResult ^= tabulationTables[i][((int)(key[i] + 128))].hashValue; //Silly hack to solve signed char in string problem
	return hashResult;
}