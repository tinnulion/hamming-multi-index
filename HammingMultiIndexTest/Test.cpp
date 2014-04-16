/**
 * @file main.cpp
 *
 * Just simple testing console app for FastHammingIndex.
  *
 * Author: Igor S. Ryabtsov
 * License: Apache version 2
 */

#include <iostream>
#include <stdio.h>
#include <time.h>
#include <tchar.h>
#include <vector>
#include <utility>
#include <random>

#include "..\HammingMultiIndexLib\MultiIndex.h"

using namespace std;
using namespace HMI;

const uint ITEM_SIZE = 32; 
const uint DATASET_SIZE_SMALL = 10000000;
const uint DATASET_SIZE_LARGE = 100000000;
const uint ITERATIONS = 20;
const float RANGE = 0.10f;

mt19937 mt;
uniform_int<byte> generator(0, 255);

const byte *const GetRandomItem()
{
	byte* result = new byte[ITEM_SIZE];
	for (uint i = 0; i < ITEM_SIZE; i++)
	{
		result[i] = generator(mt);
	}
	return result;
}

void TestBruteForceVsOptimizedApproach(MultiIndex& index)
{
	cout << "Start testing TestBruteForceVsOptimizedApproach()..." << endl;
	cout << "    Filling index..." << endl;
	for (uint i = 0; i < DATASET_SIZE_SMALL; i++)
	{
		const byte *const newItem = GetRandomItem();
		index.AddItem(i, newItem);
		delete [] newItem;
		
		if (i % 1000000 == 0)
		{
			cout << "      Done: " << i << endl;
		}
	}
	cout << "      Index size: " << index.GetAllocatedSize() << " bytes!" << endl;

	cout << "    Do query..." << endl;
	for (uint i = 0; i < ITERATIONS; i++)
	{
		cout << "  Iteration #" << i << endl;

		const byte *const query = GetRandomItem();

		// Brute-force.
		vector<pair<uint, float>> bruteForceResults;
		clock_t start = clock();
		index.DoRangeQueryBruteForce(query, RANGE, bruteForceResults);
		clock_t stop = clock();
		float timeMs = 1000.0f * (static_cast<float>(stop) - start) / CLOCKS_PER_SEC;
		cout << "      Time (brute-force): " << timeMs << " milliseconds" << endl;

		// Optimized.
		vector<pair<uint, float>> optimizedResults;
		start = clock();
		index.DoRangeQueryOptimized(query, RANGE, optimizedResults);
		stop = clock();
		timeMs = 1000.0f * (static_cast<float>(stop) - start) / CLOCKS_PER_SEC;
		cout << "      Time (optimized)  : " << timeMs << " milliseconds" << endl;

		// Compare.
		if (bruteForceResults.size() != optimizedResults.size())
		{
			cout << "      FAIL - sizes did not match " << bruteForceResults.size() << " vs. " << optimizedResults.size() << "!" << endl;
		}
		else
		{
			uint mismatchesNumber = 0;
			for (uint j = 0; j < static_cast<uint>(optimizedResults.size()); j++)
			{
				if ((bruteForceResults[j].first == optimizedResults[j].first) && 
					(bruteForceResults[j].second != optimizedResults[j].second))
				{
					mismatchesNumber++;
				}
			}
			if (mismatchesNumber == 0)
			{
				cout <<  "      GOOD so far #optimizedResults = " << optimizedResults.size() << endl;
			}
			else
			{
				cout <<  "      FAIL - mismatches " << mismatchesNumber << " of " << optimizedResults.size() << endl;
			}
		}
	}
	cout << "    Drop index..." << endl;
	index.Clear();
}

void TestPerformance(MultiIndex& index)
{
	cout << "Start testing TestPerformance()..." << endl;
	cout << "    Filling index..." << endl;
	for (uint i = 0; i < DATASET_SIZE_LARGE; i++)
	{
		const byte *const newItem = GetRandomItem();
		index.AddItem(i, newItem);
		delete [] newItem;

		if (i % 1000000 == 0)
		{
			cout << "      Done: " << i << endl;
		}
	}
	cout << "      Index size: " << index.GetAllocatedSize() << " bytes!" << endl;

	for (uint i = 0; i < ITERATIONS; i++)
	{
		cout << "  Iteration #" << i << endl;
		cout << "    Do query..." << endl;
		const byte *const query = GetRandomItem();

		// Optimized.
		vector<pair<uint, float>> optimizedResults;
		clock_t start = clock();
		//index.DoRangeQueryBruteForce(query, RANGE, optimizedResults);
		index.DoRangeQueryOptimized(query, RANGE, optimizedResults);
		clock_t stop = clock();
		cout << "      Results found: " << optimizedResults.size() << endl;
		float timeMs = 1000.0f * (static_cast<float>(stop) - start) / CLOCKS_PER_SEC;
		cout << "      Time: " << timeMs << " milliseconds" << endl;
	}

	cout << "    Drop index..." << endl;
	index.Clear();
}

int _tmain(int argc, _TCHAR* argv[])
{
	// Initialize random seed.
	mt.seed(time(NULL));

	cout << "Initialization..." << endl;
	MultiIndex index(ITEM_SIZE);
	cout << "Done. Item size is " << ITEM_SIZE << " bytes." << endl;

	// Compare brute force and optimized results on 10M.
	TestBruteForceVsOptimizedApproach(index);

	// Upload 100M random items and measure query time.
	TestPerformance(index);

	cout << "Job`s done!" << endl;
	cout << "Press any key..." << endl;
	cin.get();
	return 0;
}


