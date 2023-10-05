/*    PortTool v2.2     FPTOOL11.C          */

#include "fptools.h"
#include "fpmemmgr.h"

//
// Allocate data buffer as needed
//                                    
#define BLOCKSIZE      32  // smallest memory allocation
#define RESIZE_FACTOR   2  // if size / 2 < length, reduce memory use
//
DWORD FPLIB fpAllocateData (HANDLE FAR* hData, DWORD dwOldSize, DWORD dwLength)
{
   DWORD dwNewSize = 0L;
   
   if (dwOldSize == 0L && dwLength == 0L)
      return 0L;
   
   if (dwLength+1 > dwOldSize ||
       ((dwLength > BLOCKSIZE) && ((dwLength+1) < (dwOldSize / RESIZE_FACTOR))))
      dwNewSize = (((dwLength+1) / BLOCKSIZE) + 1) * BLOCKSIZE;
   else
      return dwOldSize;
      
   // Allocate buffer memory 
   if (*hData == NULL)
      *hData = (HANDLE)fpGlobalAlloc (GHND, dwNewSize);
   else        
   {     
//      MEM_POINTER_STATUS memReturn;                        
//      memReturn = (MEM_POINTER_STATUS)fpCheckPtr(*hData);   
//      if (memReturn == MEM_POINTER_OK)
//         fpGlobalFree(*hData);                                    // free if valid pointer  -- CTF
//      *hData = fpGlobalAlloc (GHND, dwNewSize);   
      *hData = (HANDLE)fpGlobalReAlloc ((FPGLOBALHANDLE)*hData, dwNewSize, GHND);
   }
   return ((*hData) ? dwNewSize : 0L);
}                            
