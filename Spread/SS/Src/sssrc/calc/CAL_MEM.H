//--------------------------------------------------------------------
//
//  File: cal_mem.h
//
//  Description: Memory allocation routines
//

#ifndef CAL_MEM_H
#define CAL_MEM_H

#ifdef __cplusplus
extern "C" {
#endif

//--------------------------------------------------------------------
//
//  The following macros define the memory allocation scheme used by
//  the calc routines.
//

#if !defined(_WIN64) && !defined(_IA64) && !defined(SS_NO_USE_SH)
  #define CALC_USE_SMARTHEAP
#endif

#ifdef CALC_USE_SMARTHEAP
  #include "..\..\..\..\smrtheap\include\smrtheap.h"
  typedef MEM_HANDLE CALC_HANDLE, FAR* LPCALC_HANDLE;
#else
  typedef HGLOBAL CALC_HANDLE, FAR* LPCALC_HANDLE;
#endif

//--------------------------------------------------------------------
//
//  The following macros allow the calc engine routines to use
//  either the smartheap memory manager or the windows memory
//  manager.
//

#ifdef CALC_USE_SMARTHEAP

  extern MEM_POOL calcMemPool;
  #define CalcMemAlloc(dwSize) \
              MemAllocPtr(calcMemPool,dwSize,MEM_MOVEABLE|MEM_ZEROINIT)
  #define CalcMemReAlloc(hMem,dwSize) \
              MemReAllocPtr(hMem,dwSize,MEM_MOVEABLE|MEM_ZEROINIT)
  #define CalcMemLock(hMem) (LPVOID)hMem
  #define CalcMemUnlock(hMem)
  #define CalcMemFree(hMem) MemFreePtr((LPVOID)hMem)

#else

  #define CalcMemAlloc(dwSize) GlobalAlloc(GMEM_MOVEABLE|GMEM_ZEROINIT,dwSize)
  #define CalcMemReAlloc(hMem,dwSize) \
              GlobalReAlloc(hMem,dwSize,GMEM_MOVEABLE|GMEM_ZEROINIT)
  #define CalcMemLock(hMem) GlobalLock(hMem)
  #define CalcMemUnlock(hMem) GlobalUnlock(hMem)
  #define CalcMemFree(hMem) GlobalFree(hMem)

#endif

//--------------------------------------------------------------------
//
//  The following routines are used to initialize and free memory
//  pools used by other calc engine routines.
//

void CalcMemPoolInit();
void CalcMemPoolFree();

//--------------------------------------------------------------------

#ifdef __cplusplus
}
#endif

#endif
