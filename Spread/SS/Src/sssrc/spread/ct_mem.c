//--------------------------------------------------------------------
//
//  File: ct_mem.c
//
//  Description: Memory allocation routines for custom celltypes
//

#ifdef SS_V80

#include <windows.h>
#include "toolbox.h"
#include "ct_mem.h"

//--------------------------------------------------------------------
//
//  When SmartHeap memory management is used, the ctMemPool is the
//  memory pool used by the custom celltype functions.
//

#ifdef CT_USE_SMARTHEAP
  MEM_POOL ct_MemPool = 0;
#endif

//--------------------------------------------------------------------
//
//  The CT_MemPoolInit() function initializes a SmartHeap memory
//  pool which is used by the custom cell type functions.
//
//  Note: If SmartHeap memory management is being used, then
//        CT_MemPoolInit() must be called before any other custom
//        celltype functions are used.
//

void CT_MemPoolInit()
{
#ifdef CT_USE_SMARTHEAP
  if( !ct_MemPool )
    ct_MemPool = MemPoolInit(MEM_POOL_DEFAULT);
#endif
}

//--------------------------------------------------------------------
//
//  The CT_MemPoolFree() function frees the SmartHeap memory pool
//  which is being used by the custom cell type routines.
//
//  Note: If SmartHeap memory management is being used, then no
//        custom cell type routine can be used after a call to
//        CT_MemPoolFree().
//

void CT_MemPoolFree()
{
#ifdef CT_USE_SMARTHEAP
  if( ct_MemPool )
    MemPoolFree(ct_MemPool);
  ct_MemPool = 0;
#endif
}

#endif // SS_V80