/***********************************************************************
* FPVBPROP.H
*
* Copyright (C) 1995 - FarPoint Technologies, Inc.
* All Rights Reserved.
*
* No part of this source code may be copied, modified or reproduced
* in any form without retaining the above copyright notice.  This
* source code, or source code derived from it, may not be redistributed
* without express written permission of FarPoint Technologies.
***********************************************************************/

#ifndef FPVBPROP_H
#define FPVBPROP_H

#define OFFSETIN(struc, field) ((BYTE)(long)(LPVOID)&(((struc *)0)->field))

typedef struct tagPARAMS0
   {
   LPVOID Index;
   } PARAMS0, FAR *LPPARAMS0;

typedef struct tagPARAMS5
   {
   LPVOID lpParam5;
   LPVOID lpParam4;
   LPVOID lpParam3;
   LPVOID lpParam2;
   LPVOID lpParam1;
   LPVOID Index;
   } PARAMS5, FAR *LPPARAMS5;

/************
* Functions
************/
void FPLIB VB_SetPropList(int iLastPropIndex, LPFP_PROPLIST pfpProp, 
  PPROPINFO pPropertyNULL, NPPROPLIST npProps, USHORT usVersion);

long FPLIB fpVBSetProperty(HCTL hCtl, HWND hWnd, LPVOID lpStruct, WPARAM wParam,
                     LPARAM lParam, long (PASCAL *lpPropProc)(LPVOID, WORD, long),
                     LPFP_PROPLIST lpPropList);
long FPLIB fpVBGetProperty(HCTL hCtl, HWND hWnd, LPVOID lpStruct, WPARAM wParam,
                     LPARAM lParam, long (PASCAL *lpPropProc)(LPVOID, WORD, long),
                     LPFP_PROPLIST lpPropList);

long FPLIB fpVBGetProperty_lparam( HCTL hCtl, HWND hWnd, LPVOID lpStruct, WPARAM wParam,
                     LPARAM lParam, long (PASCAL *lpPropProc)(LPVOID, WORD, long),
                     LPFP_PROPLIST lpPropList);

#endif // FPVBPROP_H
