/**
 * @file MultiIndexPagedContainer.h
 *
 * This class implements paged container (like deque but you can control page size).
 * Don`t try to push structures with dynamic data - container does not use copy constructor, just memcpy.
 *
 * Author: Igor S. Ryabtsov
 * License: Apache version 2
 */

#pragma once

#include "MultiIndexCommon.h"

namespace HMI
{
	using namespace std;

	class MultiIndexPagedContainer
	{
	public:

		/**
		 * Public constructor.
		 * @param itemSizeInBytes - number of bytes in single item.
		 * @param numberOfItemsPerPage - number of item per page, page capacity.
		 */
		MultiIndexPagedContainer(uint itemSizeInBytes, uint numberOfItemsPerPage) :
			m_NumberOfItems(0),
			m_ItemSize(itemSizeInBytes),
			m_NumberOfItemsPerPage(numberOfItemsPerPage)
		{
			if (itemSizeInBytes == 0)
			{
				assert(false);
				throw invalid_argument("Argument itemSizeInBytes = 0!");
			}
			if (numberOfItemsPerPage == 0)
			{
				assert(false);
				throw invalid_argument("Argument numberOfItemsPerPage = 0!");
			}
		}

		/**
		 * Public destructor.
		 */
		~MultiIndexPagedContainer()
		{
			Clear();
		}

		/**
		 * Returns current number of items in container.
		 */
		uint GetNumberOfItems() const
		{
			return m_NumberOfItems;
		}

		/**
		 * Destroys all items inside container and deallocate all memory. 
		 */
		void Clear()
		{
			m_NumberOfItems = 0;
			for (uint i = 0; i < static_cast<uint>(m_Pages.size()); i++)
			{
				delete [] m_Pages[i];
			}
			m_Pages.clear();
		}

		/**
		 * Returns pointer to specified item.
		 * @param index - index for item you want to get.
		 */
		const byte *const GetItemBytes(uint index) const
		{
			assert(index < m_NumberOfItems);
			uint pageIndex = index / m_NumberOfItemsPerPage;
			uint pageOffset = index % m_NumberOfItemsPerPage;
			return m_Pages[pageIndex] + m_ItemSize * pageOffset;
		}

		/**
		 * Returns pointer to specified item.
		 * @param index - index for item you want to get.
		 */
		byte* GetItemBytes(uint index)
		{
			assert(index < m_NumberOfItems);
			uint pageIndex = index / m_NumberOfItemsPerPage;
			uint pageOffset = index % m_NumberOfItemsPerPage;
			return m_Pages[pageIndex] + m_ItemSize * pageOffset;
		}

		/**
		 * Same as GetItemBytes() but also casts pointer to specified class (typename T).
		 * @param index - index for item you want to get.
		 */
		template<typename T>
		const T *const GetItemPtr(uint index) const
		{
			return reinterpret_cast<const T *const>(GetItemBytes(index));
		}

		/**
		 * Same as GetItemBytes() but also casts pointer to specified class (typename T).
		 * @param index - index for item you want to get.
		 */
		template<typename T>
		T* GetItemPtr(uint index)
		{
			return reinterpret_cast<T*>(GetItemBytes(index));
		}

		/**
		 * Appends new item to the end of container.
		 * @param itemBytes - pointer to data what should be added to container.
		 */
		void PushBytes(const byte *const itemBytes)
		{
			if (m_NumberOfItems % m_NumberOfItemsPerPage == 0)
			{
				m_Pages.push_back(new byte[m_ItemSize * m_NumberOfItemsPerPage]);
			}
			uint pageIndex = m_NumberOfItems / m_NumberOfItemsPerPage;
			uint pageOffset = m_NumberOfItems % m_NumberOfItemsPerPage;
			byte* destBytes = m_Pages[pageIndex] + m_ItemSize * pageOffset;
			memcpy(destBytes, itemBytes, m_ItemSize);
			m_NumberOfItems++;
		}

		/**
		 * Appends new item to the end of container.
		 * @param itemBytes - pointer to data what should be added to container.
		 */
		template<typename T>
		void PushStruct(const T& item)
		{
			assert(sizeof(T) == m_ItemSize);
			const byte *const pointer = reinterpret_cast<const byte *const>(&item);
			PushBytes(pointer);
		}

		/**
		 * Returns number of bytes actually used.
		 */
		uint64 GetAllocatedSize() const
		{
			uint64 result = sizeof(MultiIndexPagedContainer);
			result += sizeof(byte*) * m_Pages.capacity();
			result += static_cast<uint64>(m_NumberOfItemsPerPage * m_ItemSize) * m_Pages.size();
			return result;
		}

	private:
		
		/**
		 * Number of items in container.
		 */
		uint m_NumberOfItems;

		/**
		 * Size of each item in bytes.
		 */
		uint m_ItemSize;

		/**
		 * Number of items in each page. Bigger page - less allocations but more overhead.
		 */
		uint m_NumberOfItemsPerPage;

		/**
		 * Pointers to allocated pages.
		 */
		vector<byte*> m_Pages;

		// Do not allow making copies.
		MultiIndexPagedContainer(const MultiIndexPagedContainer&);
		MultiIndexPagedContainer& operator= (const MultiIndexPagedContainer&);
	};
}



