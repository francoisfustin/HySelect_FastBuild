/*    PortTool v2.2     FPMEMMGR.C          */

//
// FarPoint Heap Management
//

//#include "windows.h"
#include "fpmemmgr.h"

LONG       fpObjectCount = 0L;
OMEM_POOL  fpStringPool  = NULL;

//
// Count number of Aware objects created.
//
void FPLIB fpCreate  ()
{
   if (fpObjectCount == 0)
      {
      // Initialize OptiMem
      #ifdef _DEBUG
#ifdef MEM_DEBUG
      fpMemSetSafetyLevel(MEM_SAFETY_FULL);
#endif
      #endif
      fpMemRegisterTask();
      fpStringPool = fpMemPoolInit(MEM_POOL_DEFAULT);
      }
   fpObjectCount++;
}

//
// Decrement object count.  If count is zero, free resources.
//
void FPLIB fpDestroy ()
{
   if (fpObjectCount > 0L)
      fpObjectCount--;
   if (fpObjectCount == 0 && fpStringPool)
      {
      // Cleanup SmartHeap
      fpMemPoolFree(fpStringPool);
      fpMemUnregisterTask();

      // Reinitialize global data
      fpStringPool  = NULL;
      fpObjectCount = 0L;
      }

   return;
}

