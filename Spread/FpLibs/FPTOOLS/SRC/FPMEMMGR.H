//
// FPMEMMGR.H - SmartHeap Memory Management
//
#ifndef FPMEMMGR_H
#define FPMEMMGR_H

#include "fptools.h"

#if defined(SS_NO_USE_SH) && !defined(NO_SMARTHEAP)
#define NO_SMARTHEAP
#endif

#ifndef NO_SMARTHEAP
//#ifdef __cplusplus
//#include "smrtheap.hpp"
//#else
#include "..\..\..\smrtheap\include\smrtheap.h"
//#endif
#endif

#ifdef __cplusplus
extern "C" {
#endif

#ifndef RC_INVOKED
#pragma pack(1)
#endif   // ifndef RC_INVOKED

#ifdef OMEM_POOL
#undef OMEM_POOL
#endif

#ifndef NO_SMARTHEAP
#define OMEM_POOL MEM_POOL
#define OMEM_HANDLE MEM_HANDLE
#else
#define OMEM_POOL LPVOID
#define MEM_POOL LPVOID
#define OMEM_HANDLE GLOBALHANDLE
#define MEM_FAR
#define MEM_POOL_DEFAULT 0
typedef BOOL MEM_BOOL;
#endif
typedef DWORD MEMSIZE;
//
// Library Heap Management
//
#ifdef __cplusplus
extern "C" OMEM_POOL  fpStringPool;
extern "C" LONG       fpObjectCount;
#else
extern OMEM_POOL  fpStringPool;
extern LONG       fpObjectCount;
#endif

void FPLIB fpCreate();
void FPLIB fpDestroy();

#ifndef NO_SMARTHEAP
typedef DWORD FPGLOBALHANDLE, FAR *LPFPGLOBALHANDLE;
typedef DWORD FPHGLOBAL, FAR *LPFPHGLOBAL;
#endif

//
// Memory Management Macros (use for portability)
// (not an issue now that SmartHeap supports all environments)
//

#ifndef NO_SMARTHEAP
   MEM_BOOL FPLIB fpMemRegisterTask ();
   MEM_BOOL FPLIB fpMemUnregisterTask ();
   MEM_POOL FPLIB fpMemPoolInit (unsigned flags);
   MEM_BOOL FPLIB fpMemPoolFree (MEM_POOL pool);

   LPVOID FPLIB fpMemAllocPtr (MEM_POOL pool, unsigned long bytes, unsigned flags);
   LPVOID FPLIB fpMemReAllocPtr (void MEM_FAR * ptr, unsigned long size, unsigned flags);
   MEM_BOOL  FPLIB fpMemFreePtr (void MEM_FAR * ptr);
#ifdef MEM_DEBUG
   MEM_SAFETY_LEVEL FPLIB fpMemSetSafetyLevel (MEM_SAFETY_LEVEL level);
#endif
#endif // NO_SMARTHEAP


//#define NO_SMARTHEAP
#ifdef NO_SMARTHEAP
#define FPGLOBALHANDLE GLOBALHANDLE
#define LPFPGLOBALHANDLE GLOBALHANDLE FAR *
#define fpGlobalAlloc GlobalAlloc
#define fpGlobalLock GlobalLock
#define fpGlobalReAlloc GlobalReAlloc
#define fpGlobalUnlock GlobalUnlock
#define fpGlobalFree GlobalFree
#define fpCheckPtr 0
#define fpMemRegisterTask()
#define fpMemPoolInit(a) 0
#define fpMemPoolFree(a) 0
#define fpMemUnregisterTask()
#else

// CTF
#define fpCheckPtr(hMem) \
   (DWORD)MemCheckPtr(fpStringPool, (OMEM_HANDLE)hMem)

   //if VB, map to functions in FPMEMDLL.C
#if (defined(FP_VB) || !(defined(FP_OCX) || defined(FP_DLL)))
#define fpGlobalAlloc(wFlags, dwBytes) \
   (DWORD)fpMemAllocPtr(fpStringPool, (MEMSIZE)(dwBytes), TRUE)
#define fpGlobalReAlloc(hMem, dwBytes, wFlags) \
   (DWORD)fpMemReAllocPtr((OMEM_HANDLE)hMem, (MEMSIZE)(dwBytes), TRUE)
#define fpGlobalLock(hMem) (hMem)
#define fpGlobalUnlock(hMem) (TRUE)
#define fpGlobalFree(hMem) \
   (DWORD)fpMemFreePtr((OMEM_HANDLE)hMem)

#else  //else not VB, map to smartheap functions

#define fpGlobalAlloc(wFlags, dwBytes) \
   (DWORD)MemAllocPtr(fpStringPool, (MEMSIZE)(dwBytes), TRUE)
#define fpGlobalReAlloc(hMem, dwBytes, wFlags) \
   (DWORD)MemReAllocPtr((OMEM_HANDLE)hMem, (MEMSIZE)(dwBytes), TRUE)
#define fpGlobalLock(hMem) (hMem)
#define fpGlobalUnlock(hMem) (TRUE)
#define fpGlobalFree(hMem) \
   (DWORD)MemFreePtr((OMEM_HANDLE)hMem)
#define fpGlobalSize(hMem) \
   (DWORD)MemSizePtr((OMEM_HANDLE)hMem)

#endif  //if-else FP_VB
#endif //NO_SMARTHEAP

#ifndef RC_INVOKED
#pragma pack()
#endif   // ifndef RC_INVOKED

#ifdef __cplusplus
}
#endif

#endif // FPMEMMGR_H
