//--------------------------------------------------------------------
//
//  File: ct_mem.h
//
//  Description: Memory allocation routines for custom celltypes
//

#ifndef CT_MEM_H
#define CT_MEM_H

#ifdef SS_V80

#ifdef __cplusplus
extern "C" {
#endif

//--------------------------------------------------------------------
//
//  The following macros define the memory allocation scheme used by
//  the custom celltype functions.
//

#if !defined(_WIN64) && !defined(_IA64) && !defined(SS_NO_USE_SH)
#define CT_USE_SMARTHEAP
#endif

#ifdef CT_USE_SMARTHEAP
  #include "..\..\..\..\smrtheap\include\smrtheap.h"
  typedef MEM_HANDLE CT_HANDLE, FAR* LPCT_HANDLE;
#else
  typedef HGLOBAL CT_HANDLE, FAR* LPCT_HANDLE;
#endif

//--------------------------------------------------------------------
//
//  The following macros allow the custom celltype functions to use
//  either the smartheap memory manager or the windows memory
//  manager.
//

#ifdef CT_USE_SMARTHEAP

  extern MEM_POOL ct_MemPool;
  #define CT_MemAlloc(dwSize) \
              MemAllocPtr(ct_MemPool,dwSize,MEM_MOVEABLE|MEM_ZEROINIT)
  #define CT_MemReAlloc(hMem,dwSize) \
              MemReAllocPtr(hMem,dwSize,MEM_MOVEABLE|MEM_ZEROINIT)
  #define CT_MemLock(hMem) (LPVOID)hMem
  #define CT_MemUnlock(hMem)
  #define CT_MemFree(hMem) MemFreePtr((LPVOID)hMem)

#else

  #define CT_MemAlloc(dwSize) GlobalAlloc(GMEM_MOVEABLE|GMEM_ZEROINIT,dwSize)
  #define CT_MemReAlloc(hMem,dwSize) \
              GlobalReAlloc(hMem,dwSize,GMEM_MOVEABLE|GMEM_ZEROINIT)
  #define CT_MemLock(hMem) GlobalLock(hMem)
  #define CT_MemUnlock(hMem) GlobalUnlock(hMem)
  #define CT_MemFree(hMem) GlobalFree(hMem)

#endif

//--------------------------------------------------------------------
//
//  The following routines are used to initialize and free memory
//  pools used by custom celltype functions.
//

void CT_MemPoolInit();
void CT_MemPoolFree();

#ifdef __cplusplus
}
#endif

#endif // SS_V80

#endif
