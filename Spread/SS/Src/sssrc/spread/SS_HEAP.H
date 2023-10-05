/************
* SS_HEAP.H
************/

#ifndef SS_HEAP_H
#define SS_HEAP_H

#if (defined(_WIN64) || defined(_IA64)) && !defined(SS_NO_USE_SH)
#define SS_NO_USE_SH 
#endif

#ifdef __cplusplus
extern "C" {
#endif

typedef DWORD MEMSIZE;

/**********************
* Function Prototypes
**********************/

#ifndef SS_NO_USE_SH

  #include "..\..\..\..\smrtheap\include\smrtheap.h"

  #define OMEM_POOL MEM_POOL
  #define OMEM_HANDLE MEM_HANDLE

  extern OMEM_POOL tbStringPool;
  #define tbGlobalAlloc(wFlags, dwBytes) \
          (TBGLOBALHANDLE)MemAllocPtr(tbStringPool, (MEMSIZE)(dwBytes), TRUE)
  #define tbGlobalReAlloc(hMem, dwBytes, wFlags) \
          (TBGLOBALHANDLE)MemReAllocPtr((OMEM_HANDLE)hMem, (MEMSIZE)(dwBytes), \
          TRUE)
  #define tbGlobalLock(hMem)   hMem
  #define tbGlobalUnlock(hMem) (1)
  #define tbGlobalFree(hMem) \
          (TBGLOBALHANDLE)MemFreePtr((OMEM_HANDLE)hMem)
  #define tbGlobalSize(hMem) \
          MemSizePtr((OMEM_HANDLE)hMem)

#else
#ifndef NO_SMARTHEAP
  #define NO_SMARTHEAP
#endif

  #ifdef OMEM_POOL
    #undef OMEM_POOL
  #endif
  #define OMEM_POOL LPVOID

// 27060 -scl
//  #define tbGlobalAlloc(wFlags, dwBytes)         GlobalAlloc(wFlags, dwBytes)
//  #define tbGlobalReAlloc(hMem, dwBytes, wFlags) GlobalReAlloc((HGLOBAL)hMem, dwBytes, wFlags)
//  #define tbGlobalLock(hMem)                     GlobalLock((HGLOBAL)hMem)
//  #define tbGlobalUnlock(hMem)                   GlobalUnlock((HGLOBAL)hMem)
  #define tbGlobalAlloc(wFlags, dwBytes)         GlobalAlloc((wFlags) & ~GMEM_MOVEABLE, dwBytes)
  static __forceinline HGLOBAL tbGlobalReAlloc( HGLOBAL hMem, int dwBytes, int wFlags)
  {
     HGLOBAL tmp;
     int size = (int)GlobalSize((HGLOBAL)hMem);
     if (dwBytes < size)
         tmp = GlobalReAlloc(hMem, dwBytes, (wFlags)&~GMEM_MOVEABLE);
     else
	 {
        tmp = GlobalAlloc((wFlags)& ~GMEM_MOVEABLE, dwBytes);
        if (tmp)
		{
            memcpy((char *)tmp, (char *)hMem, min(dwBytes, (int)GlobalSize((HGLOBAL)hMem)));
		}
        GlobalFree((HGLOBAL)hMem);
     }
	 return tmp;
  }
  #define tbGlobalLock(hMem)                     (LPVOID)hMem
  #define tbGlobalUnlock(hMem)                   TRUE
  #define tbGlobalFree(hMem)                     GlobalFree((HGLOBAL)hMem)
  #define tbGlobalSize(hMem)                     GlobalSize((HGLOBAL)hMem)

#endif       

#ifdef __cplusplus
}
#endif

#endif
