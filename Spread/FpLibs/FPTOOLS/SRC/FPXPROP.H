/***********************************************************************
* FPXPROP.H : Declaration of the FP_HUGEBUFFER structure and functions.
*
* Copyright (C) 1991-1996 - FarPoint Technologies, Inc.
* All Rights Reserved.
*
* No part of this source code may be copied, modified or reproduced
* in any form without retaining the above copyright notice.  This
* source code, or source code derived from it, may not be redistributed
* without express written permission of FarPoint Technologies.
***********************************************************************/

#ifndef FPXPROP_H
#define FPXPROP_H

#include "fppinfo.h"
#include "fptstr.h"
#include "fphuge.h"

#ifdef __cplusplus
extern "C" {
#endif

typedef long (PASCAL *FP_PROPFUNC)(LPVOID, WORD, long);

#define INVALID_OLE_COLOR ((OLE_COLOR)-1)
#define INVALID_COLORREF  ((COLORREF)-1)

#define FPX_STR_GH (WORD)0x01  // Indicates that strings are returned as GLOBALHANDLEs,
                               // Default is to return a BSTR. (Only effects OCX)
#define FPX_OLDVER (WORD)0x02  // Indicates that props with PF_fLoadDataOnly should be loaded
							   // from the persistent data rather than from the blob -scl
/**********************
* Function prototypes
**********************/

//-------------------------------------------------------------------------
//
// **** THE FOLLOWING FUNCTIONS ARE FOR BOTH MFC AND ATL ****
//
//-------------------------------------------------------------------------

FPLIB_(long) fpCtlSetProp(BOOL fUserMode, BOOL FAR *lpfInvaldCtl,
  BOOL FAR *lpfSetModFlag, HWND hWnd, LPVOID lpStruct,
  WORD wParam, LONG lParam, FP_PROPFUNC lpPropProc,
  LPFP_PROPLIST lpPropList, LPFP_HUGEBUFFER lpPrePropList,
  int iPropWithIndex);

FPLIB_(long) fpCtlGetProp(BOOL fUserMode, LONG FAR *lplRetVal,
  HWND hWnd, LPVOID lpStruct, WPARAM wParam, FP_PROPFUNC lpPropProc,
  LPFP_PROPLIST lpPropList, int iPropWithIndex, WORD wFlags, 
  LPFP_HUGEBUFFER lpPrePropList);

//ATL
//FPLIB_(COLORREF) fpTranslateColor(COleControl *pOleCtrl, OLE_COLOR clrColor,
//                          HPALETTE hpal);
FPLIB_(COLORREF) fpTranslateColor(OLE_COLOR clrColor, HPALETTE hpal);


//-------------------------------------------------------------------------
//
// **** THE FOLLOWING FUNCTIONS ARE FOR ATL ONLY ****
//
//-------------------------------------------------------------------------
#ifdef NO_MFC  // ATL

FPLIB_(long) fpAtlSetProp(CfpAtlBase *pAtlCtrl, DISPID dispId,
                  HWND hWnd, LPVOID lpStruct,
                  WORD wParam, LONG lParam, FP_PROPFUNC lpPropProc,
                  LPFP_PROPLIST lpPropList, LPFP_HUGEBUFFER lpPrePropList,
                  int iPropWithIndex);

// NOTE: We use fpCtlGetProp() instead since there is no special processing.
#define fpAtlGetProp  fpCtlGetProp

#else // MFC
//-------------------------------------------------------------------------
//
// **** THE FOLLOWING FUNCTIONS ARE FOR MFC ONLY ****
//
//-------------------------------------------------------------------------

FPLIB_(BOOL) fpOCXPX(
  COleControl *pOleCtrl, CPropExchange* pPX, LPVOID lpStruct,
  FP_PROPFUNC lpGetPropProc, FP_PROPFUNC lpSetPropProc,
  LPFP_PROPLIST lpPropList, short nPropListCnt,
  LPFP_HUGEBUFFER lpPrePropList,
  WORD wFlags);  // See FPX_... for "wFlags"

FPLIB_(long) fpOCXSetProp(
  COleControl *pOleCtrl, HWND hWnd, LPVOID lpStruct,
  WORD wParam, LONG lParam, FP_PROPFUNC lpPropProc,
  LPFP_PROPLIST lpPropList, LPFP_HUGEBUFFER lpPrePropList, int iPropWithIndex);

FPLIB_(long) fpOCXGetProp(
  COleControl *pOleCtrl, HWND hWnd, LPVOID lpStruct,
  WPARAM wParam, LPARAM lParam, FP_PROPFUNC lpPropProc,
  LPFP_PROPLIST lpPropList, int iPropWithIndex,
  WORD wFlagsm, LPFP_HUGEBUFFER lpPrePropList);  // See FPX_... for "wFlags"

FPLIB_(void) fpOCXInitPreProps(LPFP_HUGEBUFFER lpPrePropList);
FPLIB_(BOOL) fpOCXLoadPreProps(HWND hWnd, LPFP_HUGEBUFFER lpPrePropList, LPVOID lpStruct,
  FP_PROPFUNC lpSetPropProc, LPFP_PROPLIST lpPropList);
FPLIB_(BOOL) fpOCXFreePreProps(LPFP_HUGEBUFFER lpPrePropList, LPFP_PROPLIST lpPropList);

#endif  // ifdef NO_MFC

#ifdef __cplusplus
}
#endif    

#endif
