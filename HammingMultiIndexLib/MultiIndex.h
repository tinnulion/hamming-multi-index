/**
 * @file MultiIndex.h
 *
 * This is very simple header-only index for binary data vectors in Hamming space. 
 * It`s fast and can handle 100M item index of 256-bit vectors on conventional PC with 16 gigabytes of RAM.
 * Use-case does not imply removing or editing index items. Add and search only.
 *
 * Author: Igor S. Ryabtsov
 * License: Apache version 2
 */

#pragma once

#include "MultiIndexCommon.h"
#include "MultiIndexBucket.h"
#include "MultiIndexHashTable.h"
#include "MultiIndexPagedContainer.h"

namespace HMI 
{
	using namespace std;

	// Constants.
	const uint DEFAULT_BUCKET_PAGE_SIZE = 128;
	const uint DEFAULT_HASH_SIZE = 100000000;
	const uint DEFAULT_PAGE_SIZE = 100000;
	const float DEFAULT_BRUTE_FORCE_BOUND = 0.25f;

	/**
	 * Fast spatial index for Hamming space vectors based on multi-indices.
	 */
	class MultiIndex
	{
	public:
	
		/**
		 * Number of bytes in item should be divisible by eight.
		 * We need to satisfy this requirement in order to use 64-bit popcount command.
		 */
		static const uint ITEM_SIZE_ALIGNMENT = 8;

		/**
		 * Public constructor of FastHammingIndex class.
		 * @throw FastHammingIndexException exception if itemBytesNumber is not divisible by eight.
		 * @throw FastHammingIndexException exception if popcount command is not supported by current CPU.
		 * @param itemBytesNumber - number of bytes in items which shall be indexed.
		 * @param bucketPageSize - size of page inside bucket (larger page size cause less allocations but increase memory overhead and vice versa).
		 * @param hashTableSize - number of items in hash table (more items mean faster lookup, but more space required and vice versa).
		 * @param pageSize - number of items per page (larger page size cause less allocations but increase memory overhead and vice versa).
		 */
		MultiIndex(
			uint itemBytesNumber,
			uint bucketPageSize = DEFAULT_BUCKET_PAGE_SIZE,
			uint hashTableSize = DEFAULT_HASH_SIZE,
			uint pageSize = DEFAULT_PAGE_SIZE,
			float bruteForceBound = DEFAULT_BRUTE_FORCE_BOUND) :
			m_NumberOfBytesInItems(itemBytesNumber),
			m_NumberOfWordsInItem(itemBytesNumber / WORD_SIZE),
			m_BruteForceBound(bruteForceBound),
			m_WordToKeyIndex(itemBytesNumber * NUMBER_OF_WORD_VARIATIONS / WORD_SIZE, MultiIndexBucket(bucketPageSize)),
			m_KeyToArrayIndexTable(hashTableSize, pageSize),
			m_Items(itemBytesNumber, pageSize)
		{
			if (itemBytesNumber % ITEM_SIZE_ALIGNMENT != 0)
			{
				stringstream tmp;
				tmp << "Parameter itemBytesNumber = " << itemBytesNumber << " should be divisible by " << ITEM_SIZE_ALIGNMENT << "!";	
				throw MultiIndexException(tmp.str().c_str());
			}
			if (hashTableSize == 0)
			{
				throw MultiIndexException("Parameter keyHashTableSize = 0 and its terribly wrong!");
			}
			if (pageSize == 0)
			{
				throw MultiIndexException("Parameter pageSize = 0 and its terribly wrong!");
			}
			CheckPopcountSupport();
			GenerateMasks(m_MasksAndDistances);
		}

		/**
		 * Public destructor.
		 */
		~MultiIndex()
		{
			Clear();
		}
	
		/**
		 * Returns number of items in index.
		 */
		uint GetNumberOfItems() const
		{
			return m_Items.GetNumberOfItems();
		}

		/**
		 * Removes all items from index. 
		 */
		void Clear()
		{
			// Clear word-to-key index.
			for (uint i = 0; i < static_cast<uint>(m_WordToKeyIndex.size()); i++)
			{
				m_WordToKeyIndex[i].Clear();
			}
			m_KeyToArrayIndexTable.Clear();
			m_Items.Clear();
		}

		/**
		 * This method add single item to index.
		 * @param key - item unique identifier. Keys don`t have to go one-after-another.
		 * @param item - array bytes with data to be stored. 
		 */
		void AddItem(uint key, const byte *const item)
		{
			// Add item to hash.
			try
			{
				m_KeyToArrayIndexTable.Push(key);
			}
			catch (exception& e)
			{
				throw MultiIndexException(e.what());
			}

			// Add item to word-to-key index.
			for (uint i = 0; i < m_NumberOfWordsInItem; i++)
			{
				ushort word = reinterpret_cast<const ushort *const>(item)[i];
				uint bucketIndex = i * NUMBER_OF_WORD_VARIATIONS + word;
				assert(bucketIndex < m_WordToKeyIndex.size());
				m_WordToKeyIndex[bucketIndex].Push(key);
			}
			
			// Add item itself.
			m_Items.PushBytes(item);
		}

		/**
		 * Processes range-based query. Brute-force version.
		 * Range query returns a number of elements which lay within range distance from query vector.
		 * @param query - query vector.
		 * @param range - value inside [0, 1] interval, which specify neighborhood size.
		 * @param keysAndDistances - result will be placed here.
		 */
		void DoRangeQueryBruteForce(
			const byte *const query, 
			float range,
			vector<pair<uint, float>>& keysAndDistances) const
		{
			// Check range.
			if ((range < 0.0f) || (range > 1.0f))
			{
				throw MultiIndexException("Range parameter should lay inside [0, 1] interval!");
			}
			keysAndDistances.clear();
		
			// Just compare query to all items in index.
			assert(m_Items.GetNumberOfItems() == m_KeyToArrayIndexTable.GetNumberOfItems());
			for (uint i = 0; i < m_Items.GetNumberOfItems(); i++)
			{
				const byte *const currentItem = m_Items.GetItemBytes(i);
				float currentDistance = static_cast<float>(GetDistance(query, currentItem, m_NumberOfBytesInItems)) / (BITS_IN_BYTE * m_NumberOfBytesInItems);
				if (currentDistance <= range)
				{
					uint key = m_KeyToArrayIndexTable.GetKeyByIndex(i);
					keysAndDistances.push_back(make_pair<uint, float>(key, currentDistance));
				}
			}

			// Sort items by distance.
			sort(keysAndDistances.begin(), keysAndDistances.end(), CompareKeyAndDistancePairByDistance);
		}

		/**
		 * Processes range-based query. Optimized version - uses multi-index.
		 * Range query returns a number of elements which lay within range distance from query vector.
		 * @param query - query vector.
		 * @param range - value inside [0, 1] interval, which specify neighborhood size.
		 * @param keysAndDistances - result will be placed here.
		 */
		void DoRangeQueryOptimized(
			const byte *const query, 
			float range,
			vector<pair<uint, float>>& keysAndDistances) const
		{
			// Can we process fast search or brute force will be faster?
			if (range > m_BruteForceBound)
			{
				DoRangeQueryBruteForce(query, range, keysAndDistances);
				return;
			}
		
			// Check range.
			if ((range < 0.0f) || (range > 1.0f))
			{
				throw MultiIndexException("Range parameter should lay inside [0, 1] interval!");
			}
			keysAndDistances.clear();
		
			// Estimate necessary number of changed bits.
			uint maxBitsChanged = static_cast<uint>(range * WORD_SIZE * BITS_IN_BYTE);
		
			// Collect candidate keys.
			vector<uint> candidates;
			for (uint i = 0; i < m_NumberOfWordsInItem; i++)
			{
				ushort queryPart = *(reinterpret_cast<const ushort *const>(query) + i);
				for (uint j = 0; j < static_cast<uint>(m_MasksAndDistances.size()); j++)
				{
					 const pair<ushort, ushort>& maskAndDistance = m_MasksAndDistances[j];
					 if (maskAndDistance.second > maxBitsChanged)
					 {
						break;
					 }

					 // Mutate query part.
					 ushort maskedQueryPart = queryPart ^ maskAndDistance.first;

					 // Lookup bucket.
					 const MultiIndexBucket& bucket = m_WordToKeyIndex[maskedQueryPart];
					 bucket.CollectValuesTo(candidates);
				}
			}
			if (candidates.size() == 0)
			{
				return;
			}

			// Calculate distances by popcount.
			sort(candidates.begin(), candidates.end());
			candidates.push_back(candidates[candidates.size() - 1] + 1);
			for (uint i = 0; i < static_cast<uint>(candidates.size() - 1); i++)
			{
				uint currentKey = candidates[i];
				if (currentKey != candidates[i + 1])
				{
					uint index = m_KeyToArrayIndexTable.GetItemIndex(currentKey);
					const byte *const currentItem = m_Items.GetItemPtr<byte>(index);
					assert(currentItem != nullptr);
					float currentDistance = static_cast<float>(GetDistance(query, currentItem, m_NumberOfBytesInItems)) / (BITS_IN_BYTE * m_NumberOfBytesInItems);
					if (currentDistance <= range)
					{
						keysAndDistances.push_back(make_pair<uint, float>(currentKey, currentDistance));
					}
				}
			}

			// Sort items by distance.
			sort(keysAndDistances.begin(), keysAndDistances.end(), CompareKeyAndDistancePairByDistance);
		}

		/**
		 * Returns number of bytes actually used.
		 */
		uint64 GetAllocatedSize() const
		{
			uint64 result = 0;
			result += sizeof(vector<MultiIndexBucket>);
			result += sizeof(MultiIndexBucket) * (m_WordToKeyIndex.capacity() - m_WordToKeyIndex.size());
			for (uint i = 0; i < static_cast<uint>(m_WordToKeyIndex.size()); i++)
			{
				result += m_WordToKeyIndex[i].GetAllocatedSize();
			}
			result += m_KeyToArrayIndexTable.GetAllocatedSize();
			result += m_Items.GetAllocatedSize();
			result += sizeof(pair<ushort, ushort>) * m_MasksAndDistances.capacity();
			return result;
		}

	private:

		/**
		 * Number of bits in byte.
		 */
		static const uint BITS_IN_BYTE = 8;

		/**
		 * Multi-index uses division by two bytes.
		 */
		static const uint WORD_SIZE = 2;
	
		/**
		 * Number of possible variations of two-byte word. 
		 */
		static const uint NUMBER_OF_WORD_VARIATIONS = 65536;

		/**
		 * Just number of bytes in item.
		 */
		const uint m_NumberOfBytesInItems;

		/**
		 * Number of bytes divided by WORD_SIZE.
		 */
		const uint m_NumberOfWordsInItem;

		/**
		 * For large ranges it`s more effective to use linear scan instead of multi index.
		 */
		const float m_BruteForceBound;

		//////////////////////////////////////////////////////////////////////////

		/**
		 * Using this map you can find key by word.
		 */
		vector<MultiIndexBucket> m_WordToKeyIndex;

		/**
		 * Hash table, which helps to find Hamming vector position by key. 
		 */
		MultiIndexHashTable m_KeyToArrayIndexTable;

		/**
		 * Contains binary vectors. 
		 */
		MultiIndexPagedContainer m_Items;

		/**
		 * First entry contains mask, second one distance. Vector is sorted by distances in constructor.
		 */
		vector<pair<ushort, ushort>> m_MasksAndDistances;

		//////////////////////////////////////////////////////////////////////////
		
		/**
		 * Checks if necessary popcount instructions are supported.
		 */
		void CheckPopcountSupport()
		{
			int cpuinfo[4];
			__cpuid(cpuinfo, 1);

			#ifdef _FAST_HAMMING_INDEX_USE_AMD_SSE4A
				if (((cpuinfo[2] >> 23) & 1) == 0)
				{
					throw MultiIndexException("Seems your CPU does not support POPCNT SSE 4a instructions! Try recompile code without _FAST_HAMMING_INDEX_USE_AMD_SSE4A flag!");
				}
			#endif

			#ifdef _FAST_HAMMING_INDEX_USE_INTEL_SSE4_2
				if (((cpuinfo[2] >> 23) & 1) == 0)
				{
					throw MultiIndexException("Seems your CPU does not support POPCNT SSE 4.2 instruction! Try recompile code without _FAST_HAMMING_INDEX_USE_INTEL_SSE4_2 flag!");
				}
			#endif
		}

		/**
		 * Produces population count operation.
		 */
		uint GetPopcount(uint64 x) const
		{
			#ifdef _FAST_HAMMING_INDEX_USE_AMD_SSE4A
				return static_cast<uint>(__popcnt64(x));
			#endif 
		
			#ifdef _FAST_HAMMING_INDEX_USE_INTEL_SSE4_2
				return static_cast<uint>(_mm_popcnt_u64(x));
			#endif

			#ifdef _FAST_HAMMING_INDEX_USE_EMULATION
				int count = 0;
				while (x) 
				{
					count++;
					x &= x - 1;
				}
				return static_cast<uint>(count);
			#endif
		}

		/**
		 * Calculates Hamming distance between two byte vectors of arbitrary length (don`t forget about alignment).
		 */
		uint GetDistance(
			const byte *const x, 
			const byte *const y, 
			uint length) const
		{
			assert(length % ITEM_SIZE_ALIGNMENT == 0);
			uint distance = 0;
			for (uint i = 0; i < length; i += ITEM_SIZE_ALIGNMENT)
			{
				__int64 currentX = *(reinterpret_cast<const __int64 *const>(x + i));
				__int64 currentY = *(reinterpret_cast<const __int64 *const>(y + i));
				distance += GetPopcount(currentX ^ currentY);
			}
			assert(distance <= length * BITS_IN_BYTE);
			return distance;
		}

		/**
		 * Comparator for mask and distance pairs.
		 */
		static bool CompareKeyAndDistancePairByDistance(
			const pair<uint, float>& x,
			const pair<uint, float>& y)
		{
			return x.second < y.second;
		}

		/**
		 * Comparator for mask and distance pairs.
		 */
		static bool CompareMaskAndDistancePairByDistance(
			const pair<ushort, ushort>& x,
			const pair<ushort, ushort>& y)
		{
			return x.second < y.second;
		}

		/**
		 * Generates masks for range-based queries.
		 */
		void GenerateMasks(vector<pair<ushort, ushort>>& masksAndDistances)
		{
			masksAndDistances.reserve(NUMBER_OF_WORD_VARIATIONS);
			for (uint i = 0; i < NUMBER_OF_WORD_VARIATIONS; i++)
			{
				ushort mask = static_cast<ushort>(i);
				ushort distance = static_cast<ushort>(GetPopcount(static_cast<uint64>(mask)));
				masksAndDistances.push_back(make_pair<ushort, ushort>(mask, distance));
			}

			// Sort masks.
			stable_sort(masksAndDistances.begin(), masksAndDistances.end(), CompareMaskAndDistancePairByDistance);
		}

		// Do not allow making any copies.
		MultiIndex(const MultiIndex&);
		MultiIndex& operator= (const MultiIndex&);
	};
}