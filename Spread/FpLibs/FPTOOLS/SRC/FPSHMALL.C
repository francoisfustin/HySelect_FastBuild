
#if !defined(NO_SH) && !defined(NO_SMARTHEAP)  && !defined(SS_NO_USE_SH) // !smrtheap

// defined to prevent SmartHeap from overriding malloc - used with
//   SmartHeap library with 'new' removed

#ifdef WIN32

#ifdef __cplusplus
#include "smrtheap.hpp"
#else
#include "..\..\..\smrtheap\include\smrtheap.h"
#endif

unsigned MemDefaultPoolFlags = MEM_POOL_SERIALIZE;
int SmartHeap_malloc = 0;

#else

#define MEM_POOL_DEFAULT      0x8000u /* pool with default characteristics */
unsigned MemDefaultPoolFlags = MEM_POOL_DEFAULT;
int _cdecl SmartHeap_malloc = 0;

#endif


#endif         // !smrtheap
