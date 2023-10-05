/***********************************************************************
* FPVBPROP.C
*
* Copyright (C) 1995 - FarPoint Technologies, Inc.
* All Rights Reserved.
*
* No part of this source code may be copied, modified or reproduced
* in any form without retaining the above copyright notice.  This
* source code, or source code derived from it, may not be redistributed
* without express written permission of FarPoint Technologies.
***********************************************************************/

#define NOCOMM
#include <windows.h>
#include <string.h>
#include "fptools.h"
#include "fpmemmgr.h"
#include "fppinfo.h"
#include "fpvbprop.h"

void VB_SetPropList(int iLastPropIndex, LPFP_PROPLIST pfpProp, PPROPINFO pPropertyNULL,
  NPPROPLIST npProps, USHORT usVersion)
{
   int i;
   int version;        // 1 (< Vb2), 2 (< Vb3), 3 (>= Vb3)
   
   if (usVersion < VB200_VERSION)
     version = 1;
   else if (usVersion < VB300_VERSION)
     version = 2;
   else
     version = 3;
   
   for (i = 0; i < iLastPropIndex; i++)
   {
      BOOL fNullProp = FALSE;
   
      // set the property to null if not supported in this version
      if (pfpProp[i].wEnvironment != FP_ALL)
      {
        switch (version)
        {
          case 1:
            if (pfpProp[i].wEnvironment & FP_NO_VB1)
              fNullProp = TRUE;
            break;
          case 2:
            if (pfpProp[i].wEnvironment & FP_NO_VB2)
              fNullProp = TRUE;
            break;
          case 3:
            if (pfpProp[i].wEnvironment & FP_NO_VB3)
              fNullProp = TRUE;
            break;
        }
      }
      // This sets the bDataType from the flags in .pPropInfo
      if (pfpProp[i].pPropInfo < fpPPROPINFO_STD_LAST &&
          pfpProp[i].bDataType != -1)
         pfpProp[i].bDataType = (BYTE)(pfpProp[i].pPropInfo->fl &
                                           PF_datatype);
      // if flag is set, use PropertNULL, else use pPropInfo
      if (fNullProp)
        npProps[i] = pPropertyNULL;
      else
        npProps[i] = (PPROPINFO)pfpProp[i].pPropInfo;
   
      if ((version == 1) && !fNullProp)
      {
         if (npProps[i] < PPROPINFO_STD_LAST)
            npProps[i]->fl &= ~PF_fNoRuntimeR;
      }
   } //for loop

   npProps[iLastPropIndex] = NULL;

}


long fpVBSetProperty(HCTL hCtl, HWND hWnd, LPVOID lpStruct, WPARAM wParam,
                     LPARAM lParam, long (PASCAL *lpPropProc)(LPVOID, WORD, long),
                     LPFP_PROPLIST lpPropList)
{
long lRet = 0;

// If property is an array, simply call function with lparam intact.
if ((lpPropList[wParam].bDataType != -1) && 
    (lpPropList[wParam].pPropInfo->fl & PF_fPropArray))
   return( lpPropProc(lpStruct, wParam, lParam) );

switch (lpPropList[wParam].bDataType)
   {
   case DT_SHORT:
   case DT_BOOL:
   case DT_ENUM:
      lRet = lpPropProc(lpStruct, wParam, (long)(short)lParam);
      break;

   case DT_REAL:
      lRet = lpPropProc(lpStruct, wParam, (long)(LPVOID)&lParam);
      break;

   case DT_HSZ:
   case DT_LONG:
   case DT_COLOR:
   case DT_XPOS:
   case DT_XSIZE:
   case DT_YPOS:
   case DT_YSIZE:
   case DT_HLSTR:
      lRet = lpPropProc(lpStruct, wParam, lParam);
      break;

   case DT_PICTURE:
      lRet = lpPropProc(lpStruct, wParam, (long)(HPIC)lParam);
      break;

   default:
      return (VBDefControlProc(hCtl, hWnd, VBM_SETPROPERTY, wParam, lParam));
   }

return (lRet);
}

// NOTE: This function is specific to the way ButtonMaker's property funcs
//       return the value.  Other controls whose property funcs expect
//       "lparam" to be the same as from VB should use
//       "fpVBGetProperty_lparam()".
//
long fpVBGetProperty(HCTL hCtl, HWND hWnd, LPVOID lpStruct, WPARAM wParam,
                     LPARAM lParam, long (PASCAL *lpPropProc)(LPVOID, WORD, long),
                     LPFP_PROPLIST lpPropList)
{
long lRet = 0;

//NOTE: This func does not support properties with an index ("PF_fPropArray").
switch (lpPropList[wParam].bDataType)
   {
   case DT_HSZ:
      lRet = lpPropProc(lpStruct, wParam, 0);

      if (lRet)
         {
         LPSTR lpszText = (LPSTR)fpGlobalLock((FPGLOBALHANDLE)lRet);
         *(HSZ FAR *)lParam = VBCreateHsz((_segment)hCtl, lpszText);
         fpGlobalUnlock((FPGLOBALHANDLE)lRet);
         fpGlobalFree((FPGLOBALHANDLE)lRet);
         }
      else
         *(HSZ FAR *)lParam = VBCreateHsz((_segment)hCtl, "");

      break;

   case DT_SHORT:
   case DT_ENUM:
      *(short FAR *)lParam = (short)lpPropProc(lpStruct, wParam, 0);
      break;

   case DT_BOOL:
      *(BOOL FAR *)lParam = (BOOL)(lpPropProc(lpStruct, wParam, 0) ? -1 :
                                              FALSE);
      break;

   case DT_REAL:
      lpPropProc(lpStruct, wParam, lParam);
      break;

   case DT_LONG:
   case DT_COLOR:
   case DT_XPOS:
   case DT_XSIZE:
   case DT_YPOS:
   case DT_YSIZE:
      *(long FAR *)lParam = lpPropProc(lpStruct, wParam, 0);
      break;

   case DT_PICTURE:
      *(HPIC FAR *)lParam = (HPIC)lpPropProc(lpStruct, wParam, 0);
      break;

   case DT_HLSTR:
      {
      HLSTR  hlstrStr;
      LPSTR  lpszStr;
      USHORT uLen;

      hlstrStr = (HLSTR)lpPropProc(lpStruct, wParam, 0);

      if (hlstrStr)
         {
         lpszStr = VBDerefHlstrLen(hlstrStr, &uLen);
         hlstrStr = VBCreateHlstr(lpszStr, uLen);
         }

      *(HLSTR FAR *)lParam = hlstrStr;
      }
      break;

   default:
      return (VBDefControlProc(hCtl, hWnd, VBM_GETPROPERTY, wParam, lParam));
   }

return (0);
}

/***********************************************************************************
*  
* This "Get" function is similar to the other "Get" (above), except this func passes
* lparam in the "Get" and expects the return value in the lparam.
*  
***********************************************************************************/
long fpVBGetProperty_lparam( HCTL hCtl, HWND hWnd, LPVOID lpStruct, WPARAM wParam,
                     LPARAM lParam, long (PASCAL *lpPropProc)(LPVOID, WORD, long),
                     LPFP_PROPLIST lpPropList)
{
long lRet = 0;

switch (lpPropList[wParam].bDataType)
   {
   case DT_HSZ:
   case DT_SHORT:
   case DT_ENUM:
   case DT_REAL:
   case DT_LONG:
   case DT_COLOR:
   case DT_XPOS:
   case DT_XSIZE:
   case DT_YPOS:
   case DT_YSIZE:
   case DT_HLSTR:
      lRet = lpPropProc(lpStruct, wParam, lParam);
      break;

   case DT_BOOL:
      lRet = lpPropProc(lpStruct, wParam, lParam);
      if ((lRet == 0L) && 
          !(lpPropList[wParam].pPropInfo->fl & PF_fPropArray))
        *(BOOL FAR *)lParam = (BOOL)(*(BOOL FAR *)lParam ? -1 : FALSE);
      break;

   case DT_PICTURE:
      lRet = (HPIC)lpPropProc(lpStruct, wParam, lParam);
      break;

   default:
      return (VBDefControlProc(hCtl, hWnd, VBM_GETPROPERTY, wParam, lParam));
   }

return lRet;
}
