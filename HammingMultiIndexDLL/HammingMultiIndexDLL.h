/**
 * @file HammingMultiIndexDLL.cpp
 *
 * THis is wrapper around HammingMultiIndexLib so you can use it as dynamic library.
 *
 * Author: Igor S. Ryabtsov
 * License: Apache version 2
 */

#pragma once

#define WIN32_LEAN_AND_MEAN
#include "windows.h"
#include "..\HammingMultiIndexLib\MultiIndex.h"

namespace HMI 
{
	using namespace std;

	/**
	 * Creates new handle - pointer to index instance and returns it.
	 * @throw FastHammingIndexException exception if itemBytesNumber is not divisible by eight.
	 * @throw FastHammingIndexException exception if popcount command is not supported by current CPU.
	 * @param itemBytesNumber - number of bytes in items which shall be indexed.
	 */
	extern "C" __declspec(dllexport) MultiIndex* __cdecl CreateDefaultIndexAndGetHandle(
		uint itemBytesNumber) throw(...);

	/**
	 * Creates new handle - pointer to index instance and returns it.
	 * @throw FastHammingIndexException exception if itemBytesNumber is not divisible by eight.
	 * @throw FastHammingIndexException exception if popcount command is not supported by current CPU.
	 * @param itemBytesNumber - number of bytes in items which shall be indexed.
	 * @param bucketPageSize - size of page inside bucket (larger page size cause less allocations but increase memory overhead and vice versa).
	 * @param hashTableSize - number of items in hash table (more items mean faster lookup, but more space required and vice versa).
	 * @param pageSize - number of items per page (larger page size cause less allocations but increase memory overhead and vice versa).
	 */
	extern "C" __declspec(dllexport) MultiIndex* __cdecl CreateIndexAndGetHandle(
		uint itemBytesNumber,
		uint bucketPageSize,
		uint hashTableSize,
		uint pageSize,
		float bruteForceBound) throw(...);

	/**
 	 * Returns number of items in index.
	 * @param handle - pointer to index instance taken from CreateDefaultIndexAndGetHandle() or CreateIndexAndGetHandle().
	 */
	extern "C" __declspec(dllexport) uint __cdecl GetNumberOfItems(
		MultiIndex* handle) throw(...);

	/**
	 * Removes all items from index. 
	 * @param handle - pointer to index instance taken from CreateDefaultIndexAndGetHandle() or CreateIndexAndGetHandle().
	 */
	extern "C" __declspec(dllexport) void __cdecl Clear(
		MultiIndex* handle) throw(...);

	/**
	 * Adds new items to index.
	 * @param handle - pointer to index instance taken from CreateDefaultIndexAndGetHandle() or CreateIndexAndGetHandle().
	 * @param keys - items unique identifiers. Keys don`t have to go one-after-another.
	 * @param items - jagged array of pointer to bytes with data to be stored. 
	 * @param numberOfItems - number of items we need to add.
	 * @param numberOfTrulyAdded - number of items actually added.
	 */
	extern "C" __declspec(dllexport) void __cdecl AddItems(
		MultiIndex* handle,
		const uint *const keys,
		const byte *const *const items,
		uint numberOfItems, 
		uint& numberOfTrulyAdded) throw(...);

	/**
	 * Process range-based query.
	 * @param handle - pointer to index instance taken from CreateDefaultIndexAndGetHandle() or CreateIndexAndGetHandle().
	 * @param query - query vector.
	 * @param range - value inside [0, 1] interval, which specify neighborhood size.
	 * @param keysAndDistances - result will be placed here.
	 * @param numberOfResults - number of results found.
	 */
	extern "C" __declspec(dllexport) void __cdecl DoRangeQueryBruteForce(
		MultiIndex* handle,
		const byte *const query, 
		float range,
		pair<uint, float>*& keysAndDistances,
		uint& numberOfResults) throw(...);

	/**
 	 * Process range-based query.
	 * @param handle - pointer to index instance taken from CreateDefaultIndexAndGetHandle() or CreateIndexAndGetHandle().
	 * @param query - query vector.
	 * @param range - value inside [0, 1] interval, which specify neighborhood size.
	 * @param keysAndDistances - result will be placed here.
	 * @param numberOfResults - number of results found.
	 */
	extern "C" __declspec(dllexport) void __cdecl DoRangeQueryOptimized(
		MultiIndex* handle,
		const byte *const query, 
		float range,
		pair<uint, float>*& keysAndDistances,
		uint& numberOfResults) throw(...);

	/**
	 * Destroys all items in index. 
	 * @param handle - pointer to index instance taken from CreateDefaultIndexAndGetHandle() or CreateIndexAndGetHandle().
	 */
	extern "C" __declspec(dllexport) void __cdecl DestroyHandle(
		MultiIndex* handle) throw(...);

	/**
	 * Destroys all items in array after query processing. 
	 * @param keysAndDistances - pointer to results.
	 */
	extern "C" __declspec(dllexport) void __cdecl FreeKeysAndDistances(
		pair<uint, float>* keysAndDistances) throw(...);
}