/**
 * @file HammingMultiIndexDLL.cpp
 *
 * THis is wrapper around HammingMultiIndexLib so you can use it as dynamic library.
 *
 * Author: Igor S. Ryabtsov
 * License: Apache version 2
 */

#include "HammingMultiIndexDLL.h"

/**
 * Main entry point for dynamic-link libraries on Windows.
 */
BOOL APIENTRY DllMain( 
	HMODULE hModule,
	DWORD  ul_reason_for_call,
	LPVOID lpReserved)
{
	switch (ul_reason_for_call)
	{
	case DLL_PROCESS_ATTACH:
	case DLL_THREAD_ATTACH:
	case DLL_THREAD_DETACH:
	case DLL_PROCESS_DETACH:
		break;
	}
	return TRUE;
}

//////////////////////////////////////////////////////////////////////////

namespace HMI
{
	using namespace std;

	extern "C" __declspec(dllexport) MultiIndex* __cdecl CreateDefaultIndexAndGetHandle(
		uint itemBytesNumber) throw(...)
	{
		try
		{
			return new MultiIndex(itemBytesNumber);
		}
		catch (MultiIndexException& err)
		{
			cout << "HMI::MultiIndexException at CreateDefaultIndexAndGetHandle() : " << err.what() << endl;
			throw;
		}
		catch (exception& err)
		{
			cout << "std::exception at CreateDefaultIndexAndGetHandle() : " << err.what() << endl;
			throw;
		}
		catch (...)
		{
			cout << "Unknown exception at CreateDefaultIndexAndGetHandle()!" << endl;
			throw;
		}
	}

	extern "C" __declspec(dllexport) MultiIndex* __cdecl CreateIndexAndGetHandle(
		uint itemBytesNumber,
		uint bucketPageSize,
		uint hashTableSize,
		uint pageSize,
		float bruteForceBound) throw(...)
	{
		try
		{
			return new MultiIndex(
				itemBytesNumber,
				bucketPageSize,
				hashTableSize,
				pageSize,
				bruteForceBound);
		}
		catch (MultiIndexException& err)
		{
			cout << "HMI::MultiIndexException at CreateIndexAndGetHandle() : " << err.what() << endl;
			throw;
		}
		catch (exception& err)
		{
			cout << "std::exception at CreateIndexAndGetHandle() : " << err.what() << endl;
			throw;
		}
		catch (...)
		{
			cout << "Unknown exception at CreateIndexAndGetHandle()!" << endl;
			throw;
		}
	}

	extern "C" __declspec(dllexport) uint __cdecl GetNumberOfItems(
		MultiIndex* handle) throw(...)
	{
		assert(handle != nullptr);
		try
		{
			return handle->GetNumberOfItems();
		}
		catch (MultiIndexException& err)
		{
			cout << "HMI::MultiIndexException at GetNumberOfItems() : " << err.what() << endl;
			throw;
		}
		catch (exception& err)
		{
			cout << "std::exception at GetNumberOfItems() : " << err.what() << endl;
			throw;
		}
		catch (...)
		{
			cout << "Unknown exception at GetNumberOfItems()!" << endl;
			throw;
		}
	}

	extern "C" __declspec(dllexport) void __cdecl Clear(
		MultiIndex* handle) throw(...)
	{
		assert(handle != nullptr);
		try
		{
			return handle->Clear();
		}
		catch (MultiIndexException& err)
		{
			cout << "HMI::MultiIndexException at Clear() : " << err.what() << endl;
			throw;
		}
		catch (exception& err)
		{
			cout << "std::exception at Clear() : " << err.what() << endl;
			throw;
		}
		catch (...)
		{
			cout << "Unknown exception at Clear()!" << endl;
			throw;
		}
	}

	extern "C" __declspec(dllexport) void __cdecl AddItems(
		MultiIndex* handle,
		const uint *const keys,
		const byte *const *const items,
		uint numberOfItems, 
		uint& numberOfTrulyAdded) throw(...)
	{
		numberOfTrulyAdded = 0;
		assert(handle != nullptr);
		try
		{
			for (uint i = 0; i < numberOfItems; i++)
			{
				try
				{
					handle->AddItem(keys[i], items[i]);
					numberOfTrulyAdded++;
				}
				catch (MultiIndexException& err)
				{
					cout << "HMI::MultiIndexException at AddItems() : " << err.what() << endl;
				}
			}
		}
		catch (MultiIndexException& err)
		{
			cout << "HMI::MultiIndexException at AddItems() : " << err.what() << endl;
			throw;
		}
		catch (exception& err)
		{
			cout << "std::exception at AddItems() : " << err.what() << endl;
			throw;
		}
		catch (...)
		{
			cout << "Unknown exception at AddItems()!" << endl;
			throw;
		}
	}

	extern "C" __declspec(dllexport) void __cdecl DoRangeQueryBruteForce(
		MultiIndex* handle,
		const byte *const query, 
		float range,
		pair<uint, float>*& keysAndDistances,
		uint& numberOfResults) throw(...)
	{
		assert(handle != nullptr);
		try
		{
			vector<pair<uint, float>> temporaryResults;
			handle->DoRangeQueryBruteForce(query, range, temporaryResults);
			numberOfResults = static_cast<uint>(temporaryResults.size());
			if (numberOfResults == 0)
			{
				keysAndDistances = nullptr;
				return;
			}
			keysAndDistances = new pair<uint, float>[numberOfResults];
			memcpy(keysAndDistances, &temporaryResults[0], sizeof(pair<uint, float>) * numberOfResults);
		}
		catch (MultiIndexException& err)
		{
			cout << "HMI::MultiIndexException at AddItems() : " << err.what() << endl;
			throw;
		}
		catch (exception& err)
		{
			cout << "std::exception at AddItems() : " << err.what() << endl;
			throw;
		}
		catch (...)
		{
			cout << "Unknown exception at AddItems()!" << endl;
			throw;
		}
	}

	extern "C" __declspec(dllexport) void __cdecl DoRangeQueryOptimized(
		MultiIndex* handle,
		const byte *const query, 
		float range,
		pair<uint, float>*& keysAndDistances,
		uint& numberOfResults) throw(...)
	{
		assert(handle != nullptr);
		try
		{
			vector<pair<uint, float>> temporaryResults;
			handle->DoRangeQueryOptimized(query, range, temporaryResults);
			numberOfResults = static_cast<uint>(temporaryResults.size());
			if (numberOfResults == 0)
			{
				keysAndDistances = nullptr;
				return;
			}
			keysAndDistances = new pair<uint, float>[numberOfResults];
			memcpy(keysAndDistances, &temporaryResults[0], sizeof(pair<uint, float>) * numberOfResults);
		}
		catch (MultiIndexException& err)
		{
			cout << "HMI::MultiIndexException at DoRangeQueryOptimized() : " << err.what() << endl;
			throw;
		}
		catch (exception& err)
		{
			cout << "std::exception at DoRangeQueryOptimized() : " << err.what() << endl;
			throw;
		}
		catch (...)
		{
			cout << "Unknown exception at DoRangeQueryOptimized()!" << endl;
			throw;
		}
	}

	extern "C" __declspec(dllexport) void __cdecl DestroyHandle(
		MultiIndex* handle) throw(...)
	{
		assert(handle != nullptr);
		try
		{
			delete handle;
		}
		catch (MultiIndexException& err)
		{
			cout << "HMI::MultiIndexException at DestroyHandle() : " << err.what() << endl;
			throw;
		}
		catch (exception& err)
		{
			cout << "std::exception at DestroyHandle() : " << err.what() << endl;
			throw;
		}
		catch (...)
		{
			cout << "Unknown exception at DestroyHandle()!" << endl;
			throw;
		}
	}

	extern "C" __declspec(dllexport) void __cdecl FreeKeysAndDistances(
		pair<uint, float>* keysAndDistances) throw(...)
	{
		try
		{
			delete [] keysAndDistances;
		}
		catch (MultiIndexException& err)
		{
			cout << "HMI::MultiIndexException at FreeKeysAndDistances() : " << err.what() << endl;
			throw;
		}
		catch (exception& err)
		{
			cout << "std::exception at FreeKeysAndDistances() : " << err.what() << endl;
			throw;
		}
		catch (...)
		{
			cout << "Unknown exception at FreeKeysAndDistances()!" << endl;
			throw;
		}
	}
}
