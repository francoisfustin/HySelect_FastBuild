/***********************************************************************
* FPHUGE.H : Declaration of the FP_HUGEBUFFER structure and functions.
*
* Copyright (C) 1991-1996 - FarPoint Technologies, Inc.
* All Rights Reserved.
*
* No part of this source code may be copied, modified or reproduced
* in any form without retaining the above copyright notice.  This
* source code, or source code derived from it, may not be redistributed
* without express written permission of FarPoint Technologies.
***********************************************************************/

#ifndef FPHUGE_H
#define FPHUGE_H

#ifdef __cplusplus
extern "C" {
#endif

#ifndef RC_INVOKED
#pragma pack(1)
#endif   // ifndef RC_INVOKED

//#ifndef HPSTR
//#define HPSTR  char _huge *
//#endif

// SCP 8/13/97 - In ATL, HPVOID is defined by fptools.h, so instead of
//               including fptools.h, we will define FPHPVOID.
#define FPHPVOID   void _huge *
#define FPHPSTR    char _huge *


typedef struct tagFP_HUGEBUFFER
   {
   GLOBALHANDLE hBuffer;
   long         lBufferLen;
   long         lAllocLen;
   long         lAllocInc;
   } FP_HUGEBUFFER, FAR *LPFP_HUGEBUFFER;

FPLIB_(void)   fpHugeBufferInit(LPFP_HUGEBUFFER lpAllocBuffer, long lAllocInc);
FPLIB_(void)   fpHugeBufferFree(LPFP_HUGEBUFFER lpAllocBuffer);
FPLIB_(BOOL)   fpHugeBufferAlloc(LPFP_HUGEBUFFER lpAllocBuffer, FPHPSTR hpData,
                                    long lDataLen);
#if defined(_WIN64) || defined(_IA64)
FPLIB_(FPHPVOID) fpMemHugeCpy(FPHPVOID hpvMem1, FPHPVOID hpvMem2, LONG_PTR lSize);
#else
FPLIB_(FPHPVOID) fpMemHugeCpy(FPHPVOID hpvMem1, FPHPVOID hpvMem2, long lSize);
#endif

#ifndef RC_INVOKED
#pragma pack()
#endif   // ifndef RC_INVOKED

#ifdef __cplusplus
}
#endif    

#endif
