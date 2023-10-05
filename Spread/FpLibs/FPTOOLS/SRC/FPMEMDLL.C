// ONLY USED IF SMARTHEAP IN FPRUN300.DLL
#include "fpmemmgr.h"

#if !defined(NO_SMARTHEAP) && !defined(SS_NO_USE_SH)
MEM_POOL FPLIB fpMemPoolInit (unsigned flags)
{
   return MemPoolInit (flags);
}

MEM_BOOL FPLIB fpMemPoolFree (MEM_POOL pool)
{
   return MemPoolFree (pool);
}

MEM_BOOL FPLIB fpMemRegisterTask ()
{
   return MemRegisterTask ();
}

MEM_BOOL FPLIB fpMemUnregisterTask ()
{
   return MemUnregisterTask ();
}

LPVOID FPLIB fpMemAllocPtr (MEM_POOL pool, unsigned long bytes, unsigned flags)
{
   return MemAllocPtr (pool, bytes, flags);
}

LPVOID FPLIB fpMemReAllocPtr (void MEM_FAR * ptr, unsigned long bytes, unsigned flags) 
{
   return MemReAllocPtr (ptr, bytes, flags);
}   
   
MEM_BOOL FPLIB fpMemFreePtr (void MEM_FAR * ptr)
{
   return MemFreePtr(ptr);                        
}

#ifdef MEM_DEBUG
MEM_SAFETY_LEVEL FPLIB fpMemSetSafetyLevel (MEM_SAFETY_LEVEL level)
{
   dbgMemSetSafetyLevel (level);
   return level;
}
#else  //not debug
int FPLIB fpMemSetSafetyLevel (int level)
{
   return 0;
}
#endif

unsigned long FPLIB fpMemSizePtr (void MEM_FAR * ptr)
{
	return MemSizePtr(ptr);  
}
#else // substitute dummy functions for smartheap init:
#include "tchar.h"
/*
MEM_POOL FPLIB fpMemPoolInit (unsigned flags)
{
   return 0;
}

MEM_BOOL FPLIB fpMemPoolFree (MEM_POOL pool)
{
   if (!pool)
      MessageBox(NULL, _T("no pool"), _T("fpMemPoolFree"), MB_OK);
   return 0;
}

MEM_BOOL FPLIB fpMemRegisterTask ()
{
   return 0;
}

MEM_BOOL FPLIB fpMemUnregisterTask ()
{
   return 0;
}
*/
int FPLIB fpMemSetSafetyLevel (int level)
{
   return 0;
}

#endif // NO_SMARTHEAP
