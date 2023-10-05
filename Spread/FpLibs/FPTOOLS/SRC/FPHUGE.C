/***********************************************************************
* FPHUGE.C : Implementation of the FP_HUGEBUFFER structure and functions.
*
* Copyright (C) 1991-1996 - FarPoint Technologies, Inc.
* All Rights Reserved.
*
* No part of this source code may be copied, modified or reproduced
* in any form without retaining the above copyright notice.  This
* source code, or source code derived from it, may not be redistributed
* without express written permission of FarPoint Technologies.
***********************************************************************/

#define NOCOMM
#include "fptools.h"

#include "fphuge.h"

#include <memory.h>

#ifdef __cplusplus
extern "C" {
#endif


FPLIB_(void) fpHugeBufferInit(LPFP_HUGEBUFFER lpAllocBuffer, long lAllocInc)
{
_fmemset(lpAllocBuffer, '\0', sizeof(FP_HUGEBUFFER));

lpAllocBuffer->lAllocInc = lAllocInc;
}


FPLIB_(void) fpHugeBufferFree(LPFP_HUGEBUFFER lpAllocBuffer)
{
if (lpAllocBuffer->hBuffer)
   GlobalFree(lpAllocBuffer->hBuffer);

_fmemset(lpAllocBuffer, '\0', sizeof(FP_HUGEBUFFER));
}


FPLIB_(BOOL) fpHugeBufferAlloc(LPFP_HUGEBUFFER lpAllocBuffer, FPHPSTR hpData,
                                  long lDataLen)
{
HPSTR lpBuffer;

if (lDataLen == -1)
   lDataLen = lstrlen((LPTSTR)hpData) * sizeof(TCHAR);

if (lpAllocBuffer->lBufferLen + lDataLen > lpAllocBuffer->lAllocLen)
   {
   lpAllocBuffer->lAllocLen = max(lpAllocBuffer->lBufferLen + lDataLen,
                                  lpAllocBuffer->lAllocLen +
                                  lpAllocBuffer->lAllocInc);

   if (!(lpAllocBuffer->hBuffer))
      lpAllocBuffer->hBuffer = GlobalAlloc(GMEM_MOVEABLE | GMEM_ZEROINIT,
                                             lpAllocBuffer->lAllocLen);
   else
      {
//      GlobalUnlock(lpAllocBuffer->hBuffer);
      lpAllocBuffer->hBuffer = GlobalReAlloc(lpAllocBuffer->hBuffer,
                                               lpAllocBuffer->lAllocLen,
                                               GMEM_MOVEABLE | GMEM_ZEROINIT);
      }

   if (!lpAllocBuffer->hBuffer)
      {
      lpAllocBuffer->lBufferLen = 0;
      return (FALSE);
      }

//   lpAllocBuffer->lpBuffer = (LPSTR)GlobalLock(lpAllocBuffer->hBuffer);
   }

lpBuffer = (LPSTR)GlobalLock(lpAllocBuffer->hBuffer);

if (hpData)
   {
   register long i;

   for (i = 0, lpBuffer += lpAllocBuffer->lBufferLen; i < lDataLen; i++,
        lpBuffer++, hpData++)
      *lpBuffer = *hpData;
   }

lpAllocBuffer->lBufferLen += lDataLen;
GlobalUnlock(lpAllocBuffer->hBuffer);

return (TRUE);
}


FPLIB_(FPHPVOID) fpMemHugeCpy(FPHPVOID hpvMem1, FPHPVOID hpvMem2,
#if defined(_WIN64) || defined(_IA64)
							  LONG_PTR lSize
#else
							  long lSize
#endif
							  )
{
HPSTR      hpMem1 = hpvMem1;
HPSTR      hpMem2 = hpvMem2;
char huge *p = hpvMem1;

while(lSize--)
   *hpMem1++ = *hpMem2++;

return (p);
}

#ifdef __cplusplus
}
#endif    
