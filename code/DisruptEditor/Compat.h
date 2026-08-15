#pragma once

// MSVC extensions the codebase uses that other compilers do not provide.
//
// Included by the headers that need them rather than from Common.h, because
// IBinaryArchive.h uses __forceinline without including Common.h.
//
// __forceinline is a Microsoft keyword; plain inline is the portable
// equivalent, losing only the "ignore the inlining heuristics" hint.
// __int64 is likewise MSVC's spelling of int64_t.

#include <cstdint>

#ifndef _MSC_VER
#define __forceinline inline
typedef int64_t __int64;
#endif
