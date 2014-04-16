/**
 * @file MultiIndexHashTable.h
 *
 * This class implements hash table with separate chaining.
 * It uses MultiIndexPagedContainer to store chains.
 *
 * Author: Igor S. Ryabtsov
 * License: Apache version 2
 */

#pragma once

#include "MultiIndexCommon.h"
#include "MultiIndexPagedContainer.h"

namespace HMI
{
	using namespace std;

	class MultiIndexHashTable
	{
	public:

		/**
		 * Public constructor.
		 * @param hashTableSize - hash table is static. Allocates in constructor.
		 * @param pageSize - data is stored in pages, so you should specify page size.
		 */
		MultiIndexHashTable(uint hashTableSize, uint pageSize) :
			m_IndexOfTheFirstKeyWithGivenHash(hashTableSize, UINT_MAX),
			m_KeyAndNextIndexPairArray(sizeof(pair<uint, uint>), pageSize)
		{
		}

		/**
		 * Public destructor.
		 */
		~MultiIndexHashTable()
		{
			Clear();
		}

		/**
		 * Returns current number of items in container.
		 */
		uint GetNumberOfItems() const
		{
			return m_KeyAndNextIndexPairArray.GetNumberOfItems();
		}

		/**
		 * Removes all items from lean vector.
		 */
		void Clear()
		{
			m_IndexOfTheFirstKeyWithGivenHash.assign(m_IndexOfTheFirstKeyWithGivenHash.size(), UINT_MAX);
			m_KeyAndNextIndexPairArray.Clear();
		}

		/**
		 * Returns true if key was already added to index.
		 * @param key - just key.
		 */
		bool HasItem(uint key) const
		{
			uint hash = GetHash(key);
			uint index = m_IndexOfTheFirstKeyWithGivenHash[hash];
			while (index != UINT_MAX)
			{
				const pair<uint, uint>& item = *(m_KeyAndNextIndexPairArray.GetItemPtr<pair<uint, uint>>(index));
				if (item.first == key)
				{
					return true;
				}
				assert(item.second > index);
				index = item.second;
			} 
			return false; 	
		}

		/**
		 * Returns item by specified key. If key does not preserve in index - returns UINT_MAX.
		 * @param key - just key.
		 */
		uint GetItemIndex(uint key) const
		{
			uint hash = GetHash(key);
			uint index = m_IndexOfTheFirstKeyWithGivenHash[hash];
			while (index != UINT_MAX)
			{
				const pair<uint, uint>& item = *(m_KeyAndNextIndexPairArray.GetItemPtr<pair<uint, uint>>(index));
				if (item.first == key)
				{
					return index;
				}
				assert(item.second > index);
				index = item.second;
			} 
			return index; 			
		}

		/**
		 * Returns item by specified key. If key does not preserve in index - returns UINT_MAX.
		 * @param key - just key.
		 */
		uint GetKeyByIndex(uint index) const
		{
			const pair<uint, uint>& item = *(m_KeyAndNextIndexPairArray.GetItemPtr<pair<uint, uint>>(index));
			return item.first;
		}

		/**
		 * Adds new item to the end of container.
		 * @param key - just key.
		 */
		void Push(uint key)
		{
			uint hash = GetHash(key);
			uint index = m_IndexOfTheFirstKeyWithGivenHash[hash];
			if (index == UINT_MAX)
			{
				m_IndexOfTheFirstKeyWithGivenHash[hash] = m_KeyAndNextIndexPairArray.GetNumberOfItems();
				m_KeyAndNextIndexPairArray.PushStruct<pair<uint, uint>>(make_pair<uint, uint>(key, UINT_MAX));
			}
			else
			{
				while (true)
				{
					pair<uint, uint>& item = *(m_KeyAndNextIndexPairArray.GetItemPtr<pair<uint, uint>>(index));
					if (item.first == key)
					{
						stringstream tmp;
						tmp << "MultiIndexHashTable already has key = " << key << "!";
						throw exception(tmp.str().c_str());					
					}
					assert(item.second > index);
					index = item.second;
					if (index == UINT_MAX)
					{
						// Do add.
						item = make_pair<uint, uint>(item.first, m_KeyAndNextIndexPairArray.GetNumberOfItems());
						m_KeyAndNextIndexPairArray.PushStruct<pair<uint, uint>>(make_pair<uint, uint>(key, UINT_MAX));
						break;
					}
				} 
			}			
		}

		/**
		 * Returns number of bytes actually used.
		 */
		uint64 GetAllocatedSize() const
		{
			uint64 result = 0;
			result += sizeof(vector<uint>);
			result += sizeof(uint) * m_IndexOfTheFirstKeyWithGivenHash.capacity();
			result += m_KeyAndNextIndexPairArray.GetAllocatedSize();
			return result;
		}

	private:

		/**
		 * Index of the first key in hash table with specified hash. 
		 */
		vector<uint> m_IndexOfTheFirstKeyWithGivenHash;

		/**
		 * Contains pairs of key and index of next index with the same hash. 
		 */
		MultiIndexPagedContainer m_KeyAndNextIndexPairArray;
		
		/**
		 * Calculates hash for given key.
		 */
		uint GetHash(uint key) const
		{
			return static_cast<uint>(key % m_IndexOfTheFirstKeyWithGivenHash.size());
			//return static_cast<uint>((65537UL * key) % m_IndexOfTheFirstKeyWithGivenHash.size());
		}

		// Do not allow making any copies.
		MultiIndexHashTable(const MultiIndexHashTable&);
		MultiIndexHashTable& operator= (const MultiIndexHashTable&);
	};
}
