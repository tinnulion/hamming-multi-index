/**
 * @file MultiIndexCommon.h
 *
 * Common includes and definitions are placed here.
 *
 * Author: Igor S. Ryabtsov
 * License: Apache version 2
 */

#pragma once

// Common includes. You can place include of your precompiled header here.
#include <iostream>
#include <sstream>
#include <string>
#include <exception>
#include <algorithm>
#include <vector>
#include <utility>
#include <assert.h>
#include <limits.h>

// Uncomment one of the following lines to use specific command set (your PC should support it).
#define _FAST_HAMMING_INDEX_USE_AMD_SSE4A
// #define _FAST_HAMMING_INDEX_USE_INTEL_SSE4_2

#if defined(_FAST_HAMMING_INDEX_USE_AMD_SSE4A) && defined(_FAST_HAMMING_INDEX_USE_INTEL_SSE4_2)
#error Both directives _FAST_HAMMING_INDEX_USE_AMD_SSE4A and _FAST_HAMMING_INDEX_USE_INTEL_SSE4_2 are not allowed! Choose one.
#endif
#if !defined(_FAST_HAMMING_INDEX_USE_AMD_SSE4A) && !defined(_FAST_HAMMING_INDEX_USE_INTEL_SSE4_2)
#define _FAST_HAMMING_INDEX_USE_EMULATION
#endif

#ifdef _FAST_HAMMING_INDEX_USE_AMD_SSE4A
#include <intrin.h>
#endif

#ifdef _FAST_HAMMING_INDEX_USE_INTEL_SSE4_2
#include <mmintrin.h>
#endif

namespace HMI
{
	// Typedefs.
	typedef unsigned __int8 byte;
	typedef unsigned __int16 ushort;
	typedef unsigned __int32 uint;
	typedef unsigned __int64 uint64;

	/**
	 * FastHammingIndexException class.
	 */
	class MultiIndexException : public std::runtime_error 
	{
	public:
		MultiIndexException(const char *const& error) : std::runtime_error(error) {}
		MultiIndexException(const std::string& error) : std::runtime_error(error) {}
	};
}