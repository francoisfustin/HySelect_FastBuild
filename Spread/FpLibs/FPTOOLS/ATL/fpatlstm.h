/****************************************************************************
* FPATLSTM.H   -   Declaration of FarPoint's ATL Functions for 
*                  converting from VC project with MFC-built OCX to
*                  VC project with ATL-built OCX.
*
* Copyright (C) 1991-1998 - FarPoint Technologies
* All Rights Reserved.
*
* No part of this source code may be copied, modified or
* reproduced in any form without retaining the above
* copyright notice.  This source code, or source code
* derived from it, may not be redistributed without
* express written permission of FarPoint Technologies.
*
****************************************************************************/
#ifndef FPATLSTM_H
#define FPATLSTM_H

#include "fpdbutil.h"
#include "stdafx.h"


//----------------------------------------------------------------
// Function to read from Propset (during VBX conversion) to our Stream
//----------------------------------------------------------------
#ifdef __cplusplus
extern "C" {
#endif

#define FPCNVT_FLAG_NONE   0
#define FPCNVT_FLAG_ENUM   1

#define FPATL_CNVT_PROP_NULL  {NULL, 0, 0}
#define FPATL_OLD_PROP_NULL   {NULL, 0, 0}

// The structure is used for the list of props not found in "pinfo" list.
// For ENUM props, use vtType = VT_I2 and iCnvtFlags = FPCNVT_FLAG_ENUM.
typedef struct 
{
  LPCTSTR tstrPropName;
  VARTYPE vtType;        // use VT_ defines (ex. VT_BOOL, VT_BSTR)
  int     iCnvtFlags;   
} FPATL_CNVT_PROP, *LPFPATL_CNVT_PROP;

// The structure is used for the list of props which are no longer
// "persist" or are obsolete in the latest version ("Old props").
typedef struct
{
  LPCTSTR tstrPropName;
  DISPID  dispid;
  DWORD   wVer;
} FPATL_OLDPROP, *LPFPATL_OLDPROP, **A_LPFPATL_OLDPROP;

// Converts a set of property values from an IStream saved with a
// control using MFC to an IStream used by a control using ATL.
__declspec(dllexport) HRESULT __cdecl fpAtlConvertMfcStream( 
  ATL_PROPMAP_ENTRY* pMap,           // required, GetPropertyMap()
  CfpAtlBase *pfpAtlCtl,             // required, object ptr
  LPSTREAM *ppstmDest,               // required, ptr to output stream (func will create)
  LPSTREAM pstmSrc,                  // required, input stream
  LPFP_PROPLIST lpPropList,          // required, "pinfo" list
  SHORT nPropListCnt,                // required, count in above list
  WORD  wFlags,                      // optional, "FPX_" flags
  LPFPATL_CNVT_PROP lpAddlPropList,  // optional, list must be "null terminated" with FPATL_CNVT_PROP_NULL
  LPFPATL_OLDPROP lpOldProps);       // optional, list must be "null terminated" with FPATL_OLD_PROP_NULL

#ifdef __cplusplus
}
#endif    

#endif // ifndef FPATLSTM_H
