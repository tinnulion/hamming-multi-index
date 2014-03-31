/**
 * @file MultiIndexBucket.h
 *
 * This class implements push-only container.
 * Advantages of the container are the following:
 * 1) No memory fragmentation.
 * 2) Container uses pages therefore container expansion is fast and memory efficient.
 * 3) Small memory overhead due to metadata (around 10%). 
 *
 * Be advised that container cannot remove items or do random access.
 *
 * Author: Igor S. Ryabtsov
 * License: Apache version 2
 */

#pragma once

#include "MultiIndexCommon.h"

namespace HMI
{
	using namespace std;

	class MultiIndexBucket
	{
	public:

		/**
		 * Public constructor.
		 */
		MultiIndexBucket(uint pageSize) : 
			m_NumberOfItems(0),
			m_NumberOfItemsPerPage(pageSize), 
			m_FirstPagePtr(nullptr),
			m_LastPagePtr(nullptr)
		{
			if (pageSize == 0)
			{
				assert(false);
				throw invalid_argument("Template parameter NUMBER_OF_ITEMS_PER_PAGE = 0. It is very very wrong!");
			}
		}

		/**
		 * Public destructor.
		 */
		~MultiIndexBucket()
		{
			Clear();
		}

		/**
		 * Copy constructor.
		 */
		MultiIndexBucket(const MultiIndexBucket& source) :
			m_NumberOfItems(source.m_NumberOfItems),
			m_NumberOfItemsPerPage(source.m_NumberOfItemsPerPage),
			m_FirstPagePtr(nullptr),
			m_LastPagePtr(nullptr)
		{
			uint numberOfPages = source.GetNumberOfPages();
			if (numberOfPages == 0)
			{
				return;
			}
			byte* currentPage = m_FirstPagePtr;
			vector<byte*> copyPages(numberOfPages, nullptr);
			for (uint i = 0; i < numberOfPages; i++)
			{
				copyPages[i] = new byte[source.GetNumberOfBytesPerPage()];
				memcpy(copyPages[i], currentPage, source.GetNumberOfBytesPerPage());
				currentPage = source.GetNextPage(currentPage);
				if (i != numberOfPages - 1)
				{
					this->SetNextPage(copyPages[i], copyPages[i + 1]);
				}
				else
				{
					this->SetNextPage(copyPages[i], nullptr);
				}
			}
			m_FirstPagePtr = copyPages[0];
			m_LastPagePtr = copyPages[copyPages.size() - 1];
		}

		/**
		 * Assignment operator.
		 */
		MultiIndexBucket& operator= (const MultiIndexBucket& source)
		{
			if (this != &source)
			{
				m_NumberOfItems = source.m_NumberOfItems;
				m_NumberOfItemsPerPage = source.m_NumberOfItemsPerPage;
				m_FirstPagePtr = nullptr;
				m_LastPagePtr = nullptr;
				uint numberOfPages = source.GetNumberOfPages();
				if (numberOfPages == 0)
				{
					return *this;
				}
				byte* currentPage = m_FirstPagePtr;
				vector<byte*> copyPages(numberOfPages, nullptr);
				for (uint i = 0; i < numberOfPages; i++)
				{
					copyPages[i] = new byte[source.GetNumberOfBytesPerPage()];
					memcpy(copyPages[i], currentPage, source.GetNumberOfBytesPerPage());
					currentPage = source.GetNextPage(currentPage);
					if (i != numberOfPages - 1)
					{
						this->SetNextPage(copyPages[i], copyPages[i + 1]);
					}
					else
					{
						this->SetNextPage(copyPages[i], nullptr);
					}
				}
				m_FirstPagePtr = copyPages[0];
				m_LastPagePtr = copyPages[copyPages.size() - 1];
			}
			return *this;
		}
		
		/**
		 * Returns number of items inside container.
		 */
		uint GetNumberOfItems() const
		{
			return m_NumberOfItems;
		}
	
		/**
		 * Returns number of allocated pages.
		 */
		uint GetNumberOfPages() const
		{
			uint numberOfPages = m_NumberOfItems / m_NumberOfItemsPerPage;
			if (m_NumberOfItems % m_NumberOfItemsPerPage != 0)
			{
				numberOfPages++;
			}
			return numberOfPages;
		}

		/**
		 * Calculates and returns number of bytes for single page.
		 */
		uint GetNumberOfBytesPerPage() const
		{
			return sizeof(byte*) + m_NumberOfItemsPerPage * sizeof(uint);
		}

		/**
		 * Removes all items from container. Be advices: not all memory will be deallocated.
		 */
		void Clear()
		{
			byte* currentPage = m_FirstPagePtr;
			while (currentPage != nullptr)
			{
				byte* nextPagePtr = GetNextPage(currentPage);
				delete [] currentPage;
				currentPage = nextPagePtr;
			}
			m_NumberOfItems = 0;
			m_FirstPagePtr = nullptr;
			m_LastPagePtr = nullptr;
		}

		/**
		 * Pushes new value to the end of the bucket.
		 * @param newValue - value one need to push.
		 */
		void Push(uint newValue)
		{
			if (m_LastPagePtr == nullptr)
			{
				m_FirstPagePtr = new byte[GetNumberOfBytesPerPage()];
				m_LastPagePtr = m_FirstPagePtr;
				SetNextPage(m_LastPagePtr, nullptr);
				SetPageItem(m_LastPagePtr, 0, newValue);
			}
			else
			{
				uint offset = GetNumberOfItems() % m_NumberOfItemsPerPage;
				if (offset == 0)
				{
					byte* newPage = new byte[GetNumberOfBytesPerPage()];
					SetNextPage(newPage, nullptr);
					SetPageItem(newPage, 0, newValue);
					SetNextPage(m_LastPagePtr, newPage);
					m_LastPagePtr = newPage;
				}
				else
				{
					SetPageItem(m_LastPagePtr, offset, newValue);
				}
			}
			m_NumberOfItems++;
		}

		/**
		 * Pushes all values form specified bucket to accumulator.
		 * @param accumulator - container for bucket items.
		 */
		void CollectValuesTo(vector<uint>& accumulator) const
		{
			uint size = GetNumberOfItems();
			byte* currentPage = m_FirstPagePtr;
			while (currentPage != nullptr)
			{
				uint pageSize = min(size, m_NumberOfItemsPerPage);
				for (uint i = 0; i < pageSize; i++)
				{
					uint item = GetPageItem(currentPage, i);
					accumulator.push_back(item);
				}
				size -= pageSize;
				currentPage = GetNextPage(currentPage);
			}
		}

		/**
		 * Returns number of bytes actually used.
		 */
		uint64 GetAllocatedSize() const
		{
			uint64 result = sizeof(MultiIndexBucket);
			uint64 pageNumber = GetNumberOfPages();
			uint64 pageSize = GetNumberOfBytesPerPage();
			result += pageNumber * pageSize;
			return result;
		}

	private:

		/**
		 * Number of items inside container. 
		 */
		uint m_NumberOfItems;

		/**
		 * Number of items pre page.
		 */
		uint m_NumberOfItemsPerPage;

		/**
		 * Pointer to first page.
		 */
		byte* m_FirstPagePtr;

		/**
		 * Pointer to first page.
		 */
		byte* m_LastPagePtr;

		/**
		 * Calculates and returns pointer to next page.
		 */
		byte* GetNextPage(byte* currentPage) const
		{
			assert(currentPage != nullptr);
			return reinterpret_cast<byte**>(currentPage)[0];
		}

		/**
		 * Sets pointer to next page.
		 */
		void SetNextPage(
			byte* currentPage,
			byte* nextPagePtr) const
		{
			byte** nextPagePtrCell = reinterpret_cast<byte**>(currentPage);
			nextPagePtrCell[0] = nextPagePtr;
		}

		/**
		 * Calculates and returns item from specified page.
		 */
		uint GetPageItem(
			byte* currentPage, 
			uint itemOffsetInPage) const
		{
			assert(itemOffsetInPage < m_NumberOfItemsPerPage);
			return reinterpret_cast<uint*>(currentPage + sizeof(byte*))[itemOffsetInPage];
		}

		/**
		 * Sets item from specified page.
		 */
		void SetPageItem(
			byte* currentPage, 
			uint itemOffsetInPage,
			uint itemValue) const
		{
			assert(itemOffsetInPage < m_NumberOfItemsPerPage);
			uint* dataPtr = reinterpret_cast<uint*>(currentPage + sizeof(byte*));
			dataPtr[itemOffsetInPage] = itemValue;
		}
	};
}

