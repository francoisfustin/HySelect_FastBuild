/***********************************************************************
* FPXPROP.CPP         
*
* Copyright (C) 1991-1996 - FarPoint Technologies, Inc.
* All Rights Reserved.
*
* No part of this source code may be copied, modified or reproduced
* in any form without retaining the above copyright notice.  This
* source code, or source code derived from it, may not be redistributed
* without express written permission of FarPoint Technologies.
***********************************************************************/

#ifndef NO_MFC
#include <afxctl.h> 
#else
#include "stdafx.h"
#include "fpatl.h"
#endif

#include "fptools.h"
#include "fpmemmgr.h"
#include "fphuge.h"
#include "fpxprop.h"

#ifdef FP_VB
#define FPTRUE -1
#else
#define FPTRUE 1
#endif

#ifndef NO_MFC
HPSTR fpOCXGetPropData(LPFP_HUGEBUFFER lpPrePropList, LPFP_PROPLIST lpPropList,
                       WORD wProp);

HPSTR fpOCXGetPropDataPrimative(LPFP_HUGEBUFFER lpPrePropList, LPFP_PROPLIST lpPropList,
                       WORD wProp, BOOL fLastProp);

BOOL fpOCXSavePreProps(LPFP_HUGEBUFFER lpPrePropList, WORD wProp, LONG lParam,
                       LPFP_PROPLIST lpPropList);

long fpOCXPX_SetProp(LPVOID lpStruct, FP_PROPFUNC lpSetPropProc,
                     LPFP_PROPLIST lpPropList, LPFP_HUGEBUFFER lpPrePropList,
                     short nPropIndex, long lValue);
#endif

#ifndef ASSERT
#define ASSERT(x)  _ASSERT(x)  
#endif


//-------------------------------------------------------------------------
// For ATL, the strings passed-in are BSTR's (Wide char strings),
// we have to convert them to single-byte char strings.
// For MFC, the strings passed-in are already single-byte.
#ifdef NO_MFC // ATL
#define DECLARE_LPTSTR(varName)  LPTSTR varName = NULL

#define CONVERT_TO_TSTR(destVar, srcVar) \
  if (srcVar) \
    destVar = fpSysAllocTStringFromBSTR((LPVOID)srcVar); \
  srcVar = (LPARAM)destVar

#define FREE_TSTR(varName) \
  if (varName) \
    SysFreeString((BSTR)varName)

#else // Just stubs for MFC
#define DECLARE_LPTSTR(varName)
#define CONVERT_TO_TSTR(destVar, srcVar)
#define FREE_TSTR(varName)
#endif
//-------------------------------------------------------------------------


//-------------------------------------------------------------------------
//-------------------------------------------------------------------------
//-------------------------------------------------------------------------
//
// **** THE FOLLOWING FUNCTIONS ARE FOR BOTH MFC AND ATL ****
//
//-------------------------------------------------------------------------
//-------------------------------------------------------------------------
//-------------------------------------------------------------------------


FPLIB_(long) fpCtlSetProp(BOOL fUserMode, BOOL FAR *lpfInvaldCtl,
  BOOL FAR *lpfSetModFlag, HWND hWnd, LPVOID lpStruct,
  WORD wParam, LONG lParam, FP_PROPFUNC lpPropProc,
  LPFP_PROPLIST lpPropList, LPFP_HUGEBUFFER lpPrePropList,
  int iPropWithIndex)
{
long lRet = S_OK;
//FP_INDEXTEXT_INFO fpIndexInfo;
FP_INDEXPROP_INFO fpIndexInfo;
LONG tmpValue;
DECLARE_LPTSTR(lptstrTmp);

*lpfInvaldCtl = FALSE;
*lpfSetModFlag = FALSE;

if (lpPropList[wParam].pPropInfo <= (PFP_PROPINFO)PPROPINFO_STD_FONTNAME &&
    lpPropList[wParam].pPropInfo >= (PFP_PROPINFO)PPROPINFO_STD_FONTSIZE)
   {
   if (lpStruct)
      {
      LOGFONT LogFont;
      HFONT   hFont;

      if (!(hFont = (HFONT)SendMessage(hWnd, WM_GETFONT, 0, 0)))
         hFont = (HFONT)GetStockObject(SYSTEM_FONT);

      GetObject(hFont, sizeof(LOGFONT), &LogFont);

      if (lpPropList[wParam].pPropInfo == (PFP_PROPINFO)PPROPINFO_STD_FONTBOLD)
         LogFont.lfWeight = (lParam ? 700 : 400);

      else if (lpPropList[wParam].pPropInfo == (PFP_PROPINFO)PPROPINFO_STD_FONTITALIC)
         LogFont.lfItalic = (BOOL)(lParam ? TRUE : FALSE);

      else if (lpPropList[wParam].pPropInfo == (PFP_PROPINFO)PPROPINFO_STD_FONTNAME)
         {
         TEXTMETRIC TextMetric;
         HDC        hDC;
         HFONT      hFontTemp;

         hDC = GetDC(hWnd);
         hFontTemp = (HFONT)SelectObject(hDC, hFont);
         GetTextMetrics(hDC, &TextMetric);
         LogFont.lfHeight = -(TextMetric.tmHeight - TextMetric.tmInternalLeading);
         LogFont.lfWidth = 0;
         SelectObject(hDC, hFontTemp);
         ReleaseDC(hWnd, hDC);

         // This creates a TSTR and assigns it to lptstrTmp AND lParam (for ATL).
         CONVERT_TO_TSTR(lptstrTmp, lParam);

         _fmemset(LogFont.lfFaceName, '\0', LF_FACESIZE * sizeof(TCHAR));
         _tcsncpy(LogFont.lfFaceName, (LPTSTR)lParam, LF_FACESIZE);
         LogFont.lfFaceName[LF_FACESIZE - 1] = '\0';
         }

      else if (lpPropList[wParam].pPropInfo == (PFP_PROPINFO)PPROPINFO_STD_FONTSIZE)
         {
         if (lParam)
            {
            LogFont.lfHeight = -PT_TO_PIXELS(*(float FAR *)&lParam);
            LogFont.lfWidth = 0;
            }
         }

      else if (lpPropList[wParam].pPropInfo == (PFP_PROPINFO)PPROPINFO_STD_FONTUNDER)
         LogFont.lfUnderline = (BOOL)(lParam ? TRUE : FALSE);

      else if (lpPropList[wParam].pPropInfo == (PFP_PROPINFO)PPROPINFO_STD_FONTSTRIKE)
         LogFont.lfStrikeOut = (BOOL)(lParam ? TRUE : FALSE);

      hFont = CreateFontIndirect(&LogFont);
      SendMessage(hWnd, WM_SETFONT, (WPARAM)hFont, 0);
      DeleteObject(hFont);

      *lpfSetModFlag = TRUE;  // set modified flag

      if (!fUserMode)
         *lpfInvaldCtl = TRUE;  // invalidate the control
      }

   FREE_TSTR(lptstrTmp);

   return lRet;
   }

if (!(lpPropList[wParam].pPropInfo->fl & (fpPF_fSetData | fpPF_fSetMsg)) ||
    (fUserMode &&
     (lpPropList[wParam].pPropInfo->fl & fpPF_fNoRuntimeW)))
   return CTL_E_SETNOTSUPPORTED;

//KM sel8108  Run time props not being saved if necessary
//if (lpPropList[wParam].pPropInfo->fl & fpPF_fSaveData)
//   {
   *lpfSetModFlag = TRUE;  // set modified flag

#ifndef NO_MFC      // NOT ATL
   if (!lpStruct)
      {
      if (!(lpPropList[wParam].pPropInfo->fl & fpPF_fDefVal) ||
          lParam != lpPropList[wParam].pPropInfo->dataDefault)
         fpOCXSavePreProps(lpPrePropList, wParam, lParam, lpPropList);

      return lRet;
      }
#endif
//   }

if (lpStruct && lpPropList[wParam].bDataType >= 0)
   {
   BOOL fCallFunc = TRUE;
   LONG lParamTmp = lParam;
   int iDataType = (int)(lpPropList[wParam].pPropInfo->fl & fpPF_datatype);

   switch (iDataType)
      {
      case DT_ENUM:
         if ((short)lParam < 0 || (short)lParam > lpPropList[wParam].pPropInfo->enumMax)
            return CTL_E_INVALIDPROPERTYVALUE;
         lParamTmp = (long)(short)lParam;
         break;

      case DT_SHORT:
      case DT_BOOL:
         lParamTmp = (long)(short)lParam;
         break;

      case DT_REAL:
         lParamTmp = (long)(LPVOID)&lParam;
         break;

// -> fix for TIB7034 -scl
	  case fpDT_DOUBLE:
		  lParamTmp = lParam;
		  break;
// <- fix for TIB7034 -scl

      case DT_COLOR:
//ATL    lParamTmp = fpTranslateColor(pOleCtrl, lParam, 0);
         lParamTmp = fpTranslateColor(lParam, 0);
         break;

      case DT_HSZ:
         // This creates a TSTR and assigns it to lptstrTmp AND lParam (for ATL).
         CONVERT_TO_TSTR(lptstrTmp, lParamTmp);
         break;                     // use default value for lParamTmp

      case DT_LONG:
      case DT_XPOS:
      case DT_XSIZE:
      case DT_YPOS:
      case DT_YSIZE:
      case fpDT_HLSTR:
         break;                     // use default value for lParamTmp

      case DT_PICTURE:
         break;                     // use default value for lParamTmp

      default:
         ASSERT(0); // Data type not implemented!!!
         fCallFunc = FALSE;
         break;
      } // switch

   if (fCallFunc)
      {
      // if PropArray, put data in structure and pass ptr to struct in lparam.
      if (lpPropList[wParam].pPropInfo->fl & PF_fPropArray)
         {
         fpIndexInfo.index = iPropWithIndex;
         tmpValue = lParamTmp;               // temp storage of value
         // NOTE: ".lpsz" is a PTR to value, not the value itself.
         if ((iDataType == DT_HSZ) || (iDataType == fpDT_HLSTR))
           fpIndexInfo.u.tValue.lpsz = (LPTSTR)lParamTmp;   // lParam is already a string ptr
         else
           fpIndexInfo.u.tValue.lpsz = (LPTSTR)&tmpValue;   // ptr to storage
         lParamTmp = (LONG)&fpIndexInfo;          // ptr to index structure
         }
      lRet = lpPropProc(lpStruct, (WORD)wParam, lParamTmp);
      } //if (fCallFunc)
   } //if (bDataType >= 0)

if (!fUserMode)
   *lpfInvaldCtl = TRUE;

FREE_TSTR(lptstrTmp);

return lRet;
}


FPLIB_(long) fpCtlGetProp(BOOL fUserMode, LONG FAR *lplRetVal,
  HWND hWnd, LPVOID lpStruct, WPARAM wParam, FP_PROPFUNC lpPropProc,
  LPFP_PROPLIST lpPropList, int iPropWithIndex, WORD wFlags, LPFP_HUGEBUFFER lpPrePropList)
{
long lRet = S_OK;

if (lpPropList[wParam].pPropInfo <= (PFP_PROPINFO)PPROPINFO_STD_FONTNAME &&
    lpPropList[wParam].pPropInfo >= (PFP_PROPINFO)PPROPINFO_STD_FONTSIZE)
   {
   // These font properties are not supported at Design-time
   if (!fUserMode)
      return CTL_E_GETNOTSUPPORTED;

   if (lpStruct)
      {
      LOGFONT LogFont;
      HFONT   hFont;

      if (!(hFont = (HFONT)SendMessage(hWnd, WM_GETFONT, 0, 0)))
         hFont = (HFONT)GetStockObject(SYSTEM_FONT);

      GetObject(hFont, sizeof(LOGFONT), &LogFont);

      if (lpPropList[wParam].pPropInfo == (PFP_PROPINFO)PPROPINFO_STD_FONTBOLD)
         lRet = (LogFont.lfWeight == 700) ? FPTRUE : 0;

      else if (lpPropList[wParam].pPropInfo == (PFP_PROPINFO)PPROPINFO_STD_FONTITALIC)
         lRet = LogFont.lfItalic ? FPTRUE : 0;

      else if (lpPropList[wParam].pPropInfo == (PFP_PROPINFO)PPROPINFO_STD_FONTNAME)
         lRet = (long)fpSysAllocBSTRFromTString((FPTSTR)LogFont.lfFaceName);

      else if (lpPropList[wParam].pPropInfo == (PFP_PROPINFO)PPROPINFO_STD_FONTSIZE)
         {
         TEXTMETRIC TextMetric;
         HFONT      hFontTemp;
         HDC        hDC;

         hDC = GetDC(hWnd);
         hFontTemp = (HFONT)SelectObject(hDC, hFont);
         GetTextMetrics(hDC, &TextMetric);
         *(float FAR *)&lRet = PIXELS_TO_PT(TextMetric.tmHeight -
                                          TextMetric.tmInternalLeading);
         SelectObject(hDC, hFontTemp);
         ReleaseDC(hWnd, hDC);
         }

      else if (lpPropList[wParam].pPropInfo == (PFP_PROPINFO)PPROPINFO_STD_FONTUNDER)
         lRet = LogFont.lfUnderline ? FPTRUE : 0;

      else if (lpPropList[wParam].pPropInfo == (PFP_PROPINFO)PPROPINFO_STD_FONTSTRIKE)
         lRet = LogFont.lfStrikeOut ? FPTRUE : 0;
      }

   *lplRetVal = lRet;
   return S_OK;
   }

if ((!(lpPropList[wParam].pPropInfo->fl & (fpPF_fGetData | fpPF_fGetMsg)) &&
     !(lpPropList[wParam].pPropInfo->fl & fpPF_fSaveMsg)) ||
    (fUserMode &&
     (lpPropList[wParam].pPropInfo->fl & fpPF_fNoRuntimeR)) ||
    (!fUserMode &&
     (lpPropList[wParam].pPropInfo->fl & fpPF_fNoShow)))
   return CTL_E_GETNOTSUPPORTED;

if (!lpStruct)
   {
#ifndef NO_MFC  // The following is only valid for controls using MFC //ATL
   HPSTR lpszData;
   if (lpPropList[wParam].pPropInfo->fl & fpPF_fDefVal)
	   lRet = lpPropList[wParam].pPropInfo->dataDefault;

   switch (lpPropList[wParam].pPropInfo->fl & fpPF_datatype)
   {
   case DT_HSZ:
   case fpDT_HLSTR:
      {
         FPTSTR tstr;

	     if (lpszData = fpOCXGetPropDataPrimative(lpPrePropList, lpPropList, wParam, TRUE))
            {
            short nLen;
            nLen = *(short far *)lpszData;
            lpszData += sizeof(short);

            if (nLen)
			   {
	              tstr = (FPTSTR)lpszData;
                  lRet = (long)fpSysAllocBSTRFromTString(tstr);
			   }
             }
	  }
	  break;
   case DT_SHORT:
   case DT_ENUM:
	   {
	      short value;
		  if (lpszData = fpOCXGetPropDataPrimative(lpPrePropList, lpPropList, wParam, TRUE))
		  {
			 value = (short)*(LPLONG)lpszData;
			 lRet = *(LPLONG)&value;
		  }
	   }
	   break;
   case DT_COLOR:
	   {
	      OLE_COLOR value;
		  long lValue;
		  if (lpszData = fpOCXGetPropDataPrimative(lpPrePropList, lpPropList, wParam, TRUE))
		  {
             lValue = *(LPLONG)lpszData;
             value = (OLE_COLOR)lValue;
			 lRet = *(LPLONG)&value;
		  }
	   }
	   break;
   case DT_LONG:
   case DT_XPOS:
   case DT_XSIZE:
   case DT_YPOS:
   case DT_YSIZE:
	   {
	      long value;
		  if (lpszData = fpOCXGetPropDataPrimative(lpPrePropList, lpPropList, wParam, TRUE))
		  {
		     value = *(LPLONG)lpszData;
			 lRet = *(LPLONG)&value;
		  }
	   }
	   break;
   case DT_BOOL:
	   {
	      long value;
		  if (lpszData = fpOCXGetPropDataPrimative(lpPrePropList, lpPropList, wParam, TRUE))
		  {
		     value = (BOOL)*(LPLONG)lpszData;
			 lRet = *(LPLONG)&value;
		  }
	   }
	   break;
   case DT_PICTURE:
	   {
	      CPictureHolder value;
		  long lValue;
		  if (lpszData = fpOCXGetPropDataPrimative(lpPrePropList, lpPropList, wParam, TRUE))
		  {
             lValue = *(LPLONG)lpszData;
             if (lValue)
                {
//ATL? - Is the following code correct? - SCP 8/13/97
//ATL?   To return the address of a local (temporary) variable (value)?
                   ASSERT(0);  // is this code ever executed? - SCP 8/14/97
                   value.CreateEmpty();
                   value.SetPictureDispatch((LPPICTUREDISP)lValue);
                   lValue = (long)&value;
				   lRet = lValue;
                }
          }
	   }
	   break;
   case DT_REAL:
	   {
	      float value;
		  if (lpszData = fpOCXGetPropDataPrimative(lpPrePropList, lpPropList, wParam, TRUE))
		     {
			   value = *(float FAR *)lpszData;
			   lRet = *(LPLONG)&value;
		     }
	   }
	   break;
   }

 /*  if ((lpPropList[wParam].pPropInfo->fl & fpPF_datatype) == DT_REAL)
      {
      float flValue = *(float FAR *)&lRet;
      lRet = *(LPLONG)&flValue;
      }
   else if (((lpPropList[wParam].pPropInfo->fl & fpPF_datatype) == DT_HSZ) ||
           ((lpPropList[wParam].pPropInfo->fl & fpPF_datatype) == fpDT_HLSTR))
	  {
	  FPTSTR tstr = (FPTSTR)lRet;
      lRet = (long)fpSysAllocBSTRFromTString(tstr);
	  }*/

#else  // ATL
//   _ASSERT(0);  // for ATL, lpStruct should never be NULL
#endif

   *lplRetVal = lRet;
   return S_OK;
   }

else if (lpPropList[wParam].bDataType >= 0)
   {
   float flValue;
   double dblValue;
   long  lValue;            // CTF variable to contain returned value for PropArray
   long lParamTmp;
   //FP_INDEXTEXT_INFO fpIndexInfo;
   FP_INDEXPROP_INFO fpIndexInfo;

   lParamTmp = 0L;
   switch (lpPropList[wParam].pPropInfo->fl & fpPF_datatype)
      {
      case DT_REAL:
         lParamTmp = (long)(LPVOID)&flValue;
         break;
// -> fix for TIB7034 -scl
	  case fpDT_DOUBLE:
		  lParamTmp = (long)(LPVOID)&dblValue;
		  break;
// <- fix for TIB7034 -scl
      }

   // if PropArray, put data in structure and pass ptr to struct in lparam.
   if (lpPropList[wParam].pPropInfo->fl & PF_fPropArray)
      {
      
      if ((lpPropList[wParam].pPropInfo->fl &    // CTF
            fpPF_datatype) != DT_REAL)           // CTF
         lParamTmp = (long)(LPVOID)&lValue;      // CTF

      fpIndexInfo.index = iPropWithIndex;
      fpIndexInfo.u.tValue.lpsz = (LPTSTR)lParamTmp;   // lParam is already a string ptr
      lParamTmp = (LONG)&fpIndexInfo;          // ptr to index structure
      }

   // Call "Get" function
   lRet = lpPropProc(lpStruct, (WORD)wParam, lParamTmp);

   switch (lpPropList[wParam].pPropInfo->fl & fpPF_datatype)
      {
      case DT_HSZ:
         {
         // if flag is set in list, the above func returned a FPGLOBALHANDLE,
         // fetch a BSTR
         if (wFlags & FPX_STR_GH)
            {
            FPGLOBALHANDLE hText = (FPGLOBALHANDLE)lRet;
                                                                        
            if (hText)                                                  
               {                                                        
               LPSTR lpszText = (LPSTR)fpGlobalLock(hText);             
               lRet = (long)fpSysAllocBSTRFromTString((FPTSTR)lpszText);
               fpGlobalUnlock(hText);                                   
               fpGlobalFree(hText);                                     
               }
            } //if FPX_STR_GH
         else if (lRet)
            {
            FPTSTR tstr = (FPTSTR)lRet;
            lRet = (long)fpSysAllocBSTRFromTString(tstr);
            fpSysFreeTString(tstr);
            }
         }
         break;

      case fpDT_HLSTR:
         {
         FPTSTR tstr = (FPTSTR)lRet;
         if (tstr)
         {
#if (!defined(WIN32) || defined(_UNICODE))
           lRet = (long)SysAllocStringLen(tstr, tstr ?
                                          (UINT)*(LPLONG)(tstr - sizeof(long)) : 0);
#else
           lRet = (long)SysAllocStringByteLen(tstr, tstr ?
                                              *(LPLONG)(tstr - sizeof(long)) : 0);
#endif
         }
         }
         break;

      case DT_SHORT:
      case DT_ENUM:
      case DT_LONG:
      case DT_COLOR:
      case DT_XPOS:
      case DT_XSIZE:
      case DT_YPOS:
      case DT_YSIZE:
         if (lpPropList[wParam].pPropInfo->fl &   // CTF
             PF_fPropArray)                       // CTF
            lRet = *(LPLONG)&lValue;              // CTF
         break;

      case DT_BOOL:
         lRet = (BOOL)(lRet ? -1 : FALSE);
         break;

      case DT_REAL:
         lRet = *(LPLONG)&flValue;
         break;

// -> fix for TIB7034 -scl
	  case fpDT_DOUBLE:
		  *(double*)lplRetVal = dblValue;
		  return S_OK;
// <- fix for TIB7034 -scl

      case DT_PICTURE:
         if (lRet)
            lRet = (long)(((CPictureHolder *)lRet)->GetPictureDispatch());
         break;
      }
   *lplRetVal = lRet;
   } // if bDataType >= 0

return S_OK;
}


//-------------------------------------------------------------------------
// Override to allow for "INVALID_OLE_COLOR"
//-------------------------------------------------------------------------
//FPLIB_(COLORREF) fpTranslateColor(COleControl *pOleCtrl, OLE_COLOR clrColor,
//                          HPALETTE hpal)
FPLIB_(COLORREF) fpTranslateColor(OLE_COLOR clrColor,
                          HPALETTE hpal)
{
if (clrColor == INVALID_OLE_COLOR)
   return (INVALID_COLORREF);
else
   return (clrColor);
//   return (pOleCtrl->TranslateColor(clrColor, hpal));
}

//-------------------------------------------------------------------------
//-------------------------------------------------------------------------
//-------------------------------------------------------------------------
//
// **** THE FOLLOWING FUNCTIONS ARE FOR ATL ONLY ****
//
//-------------------------------------------------------------------------
//-------------------------------------------------------------------------
//-------------------------------------------------------------------------
#ifdef NO_MFC  //ATL
FPLIB_(long) fpAtlSetProp(CfpAtlBase *pAtlCtrl, DISPID dispId,
                  HWND hWnd, LPVOID lpStruct,
                  WORD wParam, LONG lParam, FP_PROPFUNC lpPropProc,
                  LPFP_PROPLIST lpPropList, LPFP_HUGEBUFFER lpPrePropList,
                  int iPropWithIndex)
{
  BOOL fInvaldCtl;
  BOOL fSetModFlag;

  // Only do request if not loading
  if (!pAtlCtrl->m_fIsLoadingWithHWnd && 
      pAtlCtrl->fpAtlFireOnRequestEdit(dispId) == S_FALSE)
    return S_FALSE;

  // Call primitive SetProp function.
  long lRet = fpCtlSetProp(pAtlCtrl->fpAtlGetUserMode(), &fInvaldCtl, 
                  &fSetModFlag, hWnd, lpStruct, wParam, lParam,
                  lpPropProc, lpPropList, NULL, iPropWithIndex);

  // Process the return code AND if not loading
  if (lRet == S_OK && !pAtlCtrl->m_fIsLoadingWithHWnd)
  {
    // Process the "reference" flags
    if (fInvaldCtl)
      pAtlCtrl->fpAtlInvalidateControl();
    if (fSetModFlag)
    {
      pAtlCtrl->fpAtlSetModifiedFlag();
      pAtlCtrl->fpAtlFireOnChanged(dispId);
    }
  }

  return lRet;    
}

// ------------------------------------------------------------------------
// *** NOTE:  fpAtlGetProp() is really just a MACRO which uses
// ***        fpCtlGetProp() instead.
// ------------------------------------------------------------------------

#endif  // NO_MFC


//-------------------------------------------------------------------------
//-------------------------------------------------------------------------
//-------------------------------------------------------------------------
//
// **** THE FOLLOWING FUNCTIONS ARE FOR MFC ONLY ****
//
//-------------------------------------------------------------------------
//-------------------------------------------------------------------------
//-------------------------------------------------------------------------
#ifndef NO_MFC

FPLIB_(long) fpOCXSetProp(COleControl *pOleCtrl, HWND hWnd, LPVOID lpStruct,
                  WORD wParam, LONG lParam, FP_PROPFUNC lpPropProc,
                  LPFP_PROPLIST lpPropList, LPFP_HUGEBUFFER lpPrePropList,
                  int iPropWithIndex)
{
  BOOL fInvaldCtl;
  BOOL fSetModFlag;

  // Call primitive SetProp function.
  long lRet = fpCtlSetProp(pOleCtrl->AmbientUserMode(), &fInvaldCtl, 
                  &fSetModFlag, hWnd, lpStruct, wParam, lParam,
                  lpPropProc, lpPropList, lpPrePropList, iPropWithIndex);

  // Process the return code...
  switch (lRet)
  {
    case CTL_E_SETNOTSUPPORTED:
      pOleCtrl->SetNotSupported();
      break;

    case CTL_E_INVALIDPROPERTYVALUE:
#ifdef WIN32
      pOleCtrl->ThrowError(CTL_E_INVALIDPROPERTYVALUE, AFX_IDP_E_INVALIDPROPERTYVALUE); //KM gil353
#else	        
      pOleCtrl->SetNotSupported();
#endif
      break;

    case S_OK:
      // Process the "reference" flags
      if (fInvaldCtl)
        pOleCtrl->InvalidateControl();
      if (fSetModFlag)
        pOleCtrl->SetModifiedFlag();
      break;

    default:
      ASSERT(0); // This is an odd condition, lRet SHOULD BE one of the above values!
      break;
  }

  return 0;    
}



FPLIB_(long) fpOCXGetProp(COleControl *pOleCtrl, HWND hWnd, LPVOID lpStruct,
                  WPARAM wParam, LPARAM lParam, FP_PROPFUNC lpPropProc,
                  LPFP_PROPLIST lpPropList, int iPropWithIndex, WORD wFlags, LPFP_HUGEBUFFER lpPrePropList)
{
  long lValue = 0L;
  long lRet = fpCtlGetProp(pOleCtrl->AmbientUserMode(), &lValue,
                 hWnd, lpStruct, wParam, lpPropProc,
                 lpPropList, iPropWithIndex, wFlags, lpPrePropList);

  switch (lRet)
  {
    case CTL_E_GETNOTSUPPORTED:
      pOleCtrl->GetNotSupported();
      lRet = 0;
      break;

    case S_OK:
      lRet = lValue;   // for controls using MFC, return the "Get value".
      break;

    default:
      ASSERT(0); // This is an odd condition, lRet SHOULD BE one of the above values!
      break;
  }

  return lRet;
}


FPLIB_(BOOL) fpOCXPX(COleControl *pOleCtrl, CPropExchange* pPX, LPVOID lpStruct,
             FP_PROPFUNC lpGetPropProc, FP_PROPFUNC lpSetPropProc,
             LPFP_PROPLIST lpPropList, short nPropListCnt,
             LPFP_HUGEBUFFER lpPrePropList, WORD wFlags)
{
HPSTR lpszData;
long  lDefValue = 0;
short i;
BOOL  fIsLoading = pPX->IsLoading();
BOOL  fIsConvertingVBX = (fIsLoading && pOleCtrl->IsConvertingVBX());
BOOL  fIsOldFileVer = wFlags & FPX_OLDVER;
 
for (i = 0; i < nPropListCnt; i++)
   {
   if (!(lpPropList[i].wEnvironment & FP_NO_OCX) &&
       (lpPropList[i].wEnvironment & FP_BLOB))
      {
      HGLOBAL hBlob = 0;

      if (fIsLoading)
         {
         PX_Blob(pPX, lpPropList[i].pPropInfo->npszName, hBlob, 0);

         if (hBlob)
            {
            if (!lpStruct)
               {
               HPSTR lpBlob;

               fpHugeBufferAlloc(lpPrePropList, (HPSTR)&i, sizeof(short));

               lpBlob = (HPSTR)GlobalLock(hBlob);
               fpHugeBufferAlloc(lpPrePropList, lpBlob, (long)(*(LPLONG)lpBlob +
                                 sizeof(long)));

               GlobalUnlock(hBlob);
               }
            else
               SendMessage(pOleCtrl->m_hWnd, FPM_LOADPROPERTY, (WPARAM)i,
                           (LPARAM)(LPVOID)hBlob);
            }
         }
      else
         {
         if (lpStruct)
            hBlob = (HGLOBAL)SendMessage(pOleCtrl->m_hWnd, FPM_SAVEPROPERTY, i,
                                         0);
         else if (lpszData = fpOCXGetPropData(lpPrePropList, lpPropList, i))
            {
            long lLen = *(LPLONG)(lpszData);

            if (lLen)
               {
               if (hBlob = GlobalAlloc(GHND, sizeof(long) + lLen))
                  {
                  HPSTR lpBlob = (HPSTR)GlobalLock(hBlob);

                  _fmemcpy(lpBlob, lpszData, (size_t)(sizeof(long) + lLen));
                  GlobalUnlock(hBlob);
                  }
               }
            }

//	fix for container that don't support blobs in property bags -scl
//  (like ActiveX Control Pad)
//         PX_Blob(pPX, lpPropList[i].pPropInfo->npszName, hBlob);
         if (!PX_Blob(pPX, lpPropList[i].pPropInfo->npszName, hBlob))
		 {
			 if (hBlob)
				 GlobalFree(hBlob);
			 AfxThrowOleException(E_FAIL);
		 }
//	fix for container that don't support blobs in property bags -scl
         }

      if (hBlob)
         GlobalFree(hBlob);
      }

   else if (lpPropList[i].bDataType >= 0 &&
            !(lpPropList[i].wEnvironment & FP_NO_OCX) &&
            lpPropList[i].pPropInfo < fpPPROPINFO_STD_LAST &&

            (
//            ((fIsConvertingVBX &&
//              ((lpPropList[i].pPropInfo->fl & fpPF_fLoadDataOnly) ==
//                fpPF_fLoadDataOnly)) ||
             (!(lpPropList[i].pPropInfo->fl & fpPF_fNoShow) &&
              (lpPropList[i].pPropInfo->fl & fpPF_fSaveData) &&
			  // if loading an old file version, load props with fLoadDataOnly 
			  // unless fpFP_fNewSaveDate is set.  (this flag is for props
			  // that have fSaveData, but are not in the old version of the control 
			  // -- these props must be skipped when loading the old format) -scl
			  (!fIsOldFileVer || (fIsLoading && ((lpPropList[i].pPropInfo->fl & fpPF_fNewSaveData) != fpPF_fNewSaveData)))) &&
              ((fIsLoading && fIsOldFileVer  || 
			   (lpPropList[i].pPropInfo->fl & fpPF_fLoadDataOnly) != fpPF_fLoadDataOnly))))
      {
      lDefValue = 0;

	  if (fIsLoading)
         if (lpPropList[i].pPropInfo->fl & fpPF_fDefVal)
            lDefValue = lpPropList[i].pPropInfo->dataDefault;

      switch (lpPropList[i].pPropInfo->fl & fpPF_datatype)
         {
         case DT_HSZ:
         case fpDT_HLSTR:
            {
            CString value;

            if (fIsLoading)
               {
               if (PX_String(pPX, lpPropList[i].pPropInfo->npszName, value))
                  fpOCXPX_SetProp(lpStruct, lpSetPropProc, lpPropList,
                                  lpPrePropList, i, (long)(LPCTSTR)value);
               }
            else
               {
               value = "";

               if (!lpStruct)
                  {
                  if (lpszData = fpOCXGetPropData(lpPrePropList, lpPropList, i))
                     {
                     short nLen;

                     nLen = *(short far *)lpszData;
                     lpszData += sizeof(short);

                     if (nLen)
                        value = lpszData;
                     }

                  PX_String(pPX, lpPropList[i].pPropInfo->npszName, value);
                  }

               else
                  {
                  long lresult = (long)lpGetPropProc(lpStruct, (WORD)i, 0);

                  if (lresult)
                     {
                     // if flag is set in list, the above func returned a FPGLOBALHANDLE,
                     // fetch a BSTR
                     if (wFlags & FPX_STR_GH)
                        {
                        FPGLOBALHANDLE hText = (FPGLOBALHANDLE)lresult;                
                        LPSTR lpszText = (LPSTR)fpGlobalLock(hText);
                        value = lpszText;
                        PX_String(pPX, lpPropList[i].pPropInfo->npszName, value);
                        fpGlobalUnlock(hText);
                        fpGlobalFree(hText);
                        }
                     else
                        {
                        LPSTR lpszText = (LPSTR)lresult;
                        value = lpszText;
                        PX_String(pPX, lpPropList[i].pPropInfo->npszName, value);
                        fpSysFreeTString((FPTSTR)lpszText);
                        }
                     }
                  else
                     PX_String(pPX, lpPropList[i].pPropInfo->npszName, value);
                  }
               }
            }

            break;

         case DT_SHORT:
         case DT_ENUM:
            {
            short value = (short)lDefValue;

            if (fIsLoading)
               {
               PX_Short(pPX, lpPropList[i].pPropInfo->npszName, value,
                        (short)lDefValue);
               fpOCXPX_SetProp(lpStruct, lpSetPropProc, lpPropList,
                               lpPrePropList, i, (long)value);
               }
            else
               {
               if (lpStruct)
                  value = (short)lpGetPropProc(lpStruct, (WORD)i, 0L);
               else if (lpszData = fpOCXGetPropData(lpPrePropList, lpPropList, i))
                  value = (short)*(LPLONG)lpszData;

//               if (!((lpPropList[i].pPropInfo->fl & fpPF_fDefVal) &&
//                     (lpPropList[i].pPropInfo->fl & fpPF_fNoInitDef) &&
//                     value == lpPropList[i].pPropInfo->dataDefault))
                  PX_Short(pPX, lpPropList[i].pPropInfo->npszName, value);
               }
            }
            break;

         case DT_COLOR:
            {
            OLE_COLOR value = (OLE_COLOR)lDefValue;

            if (fIsLoading)
               {
               PX_Color(pPX, lpPropList[i].pPropInfo->npszName, value,
                        (OLE_COLOR)lDefValue);
               fpOCXPX_SetProp(lpStruct, lpSetPropProc, lpPropList,
                               lpPrePropList, i,
                               (long)fpTranslateColor(value, 0));
               }
            else
               {
               long lValue = lDefValue;

               if (lpStruct)
                  lValue = lpGetPropProc(lpStruct, (WORD)i, 0L);
               else if (lpszData = fpOCXGetPropData(lpPrePropList, lpPropList, i))
                  lValue = *(LPLONG)lpszData;

               value = (OLE_COLOR)lValue;

//               if (!((lpPropList[i].pPropInfo->fl & fpPF_fDefVal) &&
//                     (lpPropList[i].pPropInfo->fl & fpPF_fNoInitDef) &&
//                     lValue == lpPropList[i].pPropInfo->dataDefault))
                  PX_Color(pPX, lpPropList[i].pPropInfo->npszName, value);
               }
            }
            break;

         case DT_LONG:
         case DT_XPOS:
         case DT_XSIZE:
         case DT_YPOS:
         case DT_YSIZE:
            {
            long value;

            if (fIsLoading)
               {
               PX_Long(pPX, lpPropList[i].pPropInfo->npszName, value,
                       (long)lDefValue);
               fpOCXPX_SetProp(lpStruct, lpSetPropProc, lpPropList,
                               lpPrePropList, i, value);
               }
            else
               {
               if (!lpStruct)
                  {
                  if (lpszData = fpOCXGetPropData(lpPrePropList, lpPropList, i))
                     value = *(LPLONG)lpszData;
                  else
                     value = (long)lDefValue;
                  }
               else
                  value = (long)lpGetPropProc(lpStruct, (WORD)i, 0L);

//               if (!((lpPropList[i].pPropInfo->fl & fpPF_fDefVal) &&
//                     (lpPropList[i].pPropInfo->fl & fpPF_fNoInitDef) &&
//                     value == lpPropList[i].pPropInfo->dataDefault))
                  PX_Long(pPX, lpPropList[i].pPropInfo->npszName, value);
               }
            }
            break;

         case DT_BOOL:
            {
            BOOL value;

            if (fIsLoading)
               {
               PX_Bool(pPX, lpPropList[i].pPropInfo->npszName, value,
                       (BOOL)lDefValue);
               fpOCXPX_SetProp(lpStruct, lpSetPropProc, lpPropList,
                               lpPrePropList, i, (long)value);
               }
            else
               {
               if (!lpStruct)
                  {
                  if (lpszData = fpOCXGetPropData(lpPrePropList, lpPropList, i))
                     value = (BOOL)*(LPLONG)lpszData;
                  else
                     value = (BOOL)lDefValue;
                  }
               else
                  value = (BOOL)lpGetPropProc(lpStruct, (WORD)i, 0L);

//               if (!((lpPropList[i].pPropInfo->fl & fpPF_fDefVal) &&
//                     (lpPropList[i].pPropInfo->fl & fpPF_fNoInitDef) &&
//                     value == lpPropList[i].pPropInfo->dataDefault))
                  PX_Bool(pPX, lpPropList[i].pPropInfo->npszName, value);
               }
            }
            break;

         case DT_PICTURE:
            {
            CPictureHolder value;

            if (fIsLoading)
               {
               LPPICTUREDISP lpPictureDisp;

               value.CreateEmpty();
               PX_Picture(pPX, lpPropList[i].pPropInfo->npszName, value);
               lpPictureDisp = value.GetPictureDispatch();

               fpOCXPX_SetProp(lpStruct, lpSetPropProc, lpPropList,
                               lpPrePropList, i, (long)lpPictureDisp);
               }
            else
               {
               long lValue = 0;

               if (lpStruct)
                  lValue = lpGetPropProc(lpStruct, (WORD)i, 0L);
               else if (lpszData = fpOCXGetPropData(lpPrePropList, lpPropList, i))
                  {
                  lValue = *(LPLONG)lpszData;
                  if (lValue)
                     {
                     value.CreateEmpty();
                     value.SetPictureDispatch((LPPICTUREDISP)lValue);
                     lValue = (long)&value;
                     }
                  }

               if (lValue)
                  PX_Picture(pPX, lpPropList[i].pPropInfo->npszName,
                             *(CPictureHolder *)lValue);
               else
				  {
				  value.CreateEmpty();
                  PX_Picture(pPX, lpPropList[i].pPropInfo->npszName, value);
				  }
               }
            }
            break;

         case DT_REAL:
            {
            float value;

            if (fIsLoading)
               {
               PX_Float(pPX, lpPropList[i].pPropInfo->npszName, value,
                        *(float FAR *)&lDefValue);
               fpOCXPX_SetProp(lpStruct, lpSetPropProc, lpPropList,
                               lpPrePropList, i, *(LPLONG)&value);
               }
            else
			   {
			   if (!lpStruct)
                  {
                  if (lpszData = fpOCXGetPropData(lpPrePropList, lpPropList, i))
                     value = *(float FAR *)lpszData;
                  else
                     value = *(float FAR *)&lDefValue;
                  }
               else
                  lpGetPropProc(lpStruct, (WORD)i, (long)(LPVOID)&value);
  
//               if (!((lpPropList[i].pPropInfo->fl & fpPF_fDefVal) &&
//                     (lpPropList[i].pPropInfo->fl & fpPF_fNoInitDef) &&
//                     value == (float)lpPropList[i].pPropInfo->dataDefault))
                  PX_Float(pPX, lpPropList[i].pPropInfo->npszName, value);
               }
            }
            break;

         }
      }
   }

return (TRUE);
} // fpOCXPX()


long fpOCXPX_SetProp(LPVOID lpStruct, FP_PROPFUNC lpSetPropProc,
                     LPFP_PROPLIST lpPropList, LPFP_HUGEBUFFER lpPrePropList,
                     short nPropIndex, long lValue)
{
if (!(lpPropList[nPropIndex].pPropInfo->fl & fpPF_fDefVal) ||
    lValue != lpPropList[nPropIndex].pPropInfo->dataDefault)
   {
   if (!lpStruct)
      fpOCXSavePreProps(lpPrePropList, nPropIndex, lValue, lpPropList);
   else
      {
      if ((lpPropList[nPropIndex].pPropInfo->fl & fpPF_datatype) == DT_REAL)
	     {
	     float value = *(float far *)&lValue;
         return (lpSetPropProc(lpStruct, (WORD)nPropIndex, (long)&value));
	     }
      else
         return (lpSetPropProc(lpStruct, (WORD)nPropIndex, lValue));
	  }
   }

return (0);
} // fpOCXPX_SetProp()




BOOL fpOCXSavePreProps(LPFP_HUGEBUFFER lpPrePropList, WORD wProp, LONG lParam,
                       LPFP_PROPLIST lpPropList)
{
fpHugeBufferAlloc(lpPrePropList, (HPSTR)&wProp, sizeof(WORD));

switch (lpPropList[wProp].pPropInfo->fl & fpPF_datatype)
   {
   case DT_ENUM:
   case DT_SHORT:
   case DT_BOOL:
   case DT_REAL:
   case DT_COLOR:
   case DT_LONG:
   case DT_XPOS:
   case DT_XSIZE:
   case DT_YPOS:
   case DT_YSIZE:
   case DT_PICTURE:
      fpHugeBufferAlloc(lpPrePropList, (HPSTR)&lParam, sizeof(long));
      break;

   case fpDT_HLSTR:
   case DT_HSZ:
      {
      short nLen = 0;
      short tmpLen = 0;

      if (lParam)
         {
         nLen = lstrlen((LPTSTR)lParam);
         tmpLen = (nLen ? nLen : 1);
         } 

      // SCP 5/6/96: In order to save properties with "", save the length
      // as 1, and save a buffer of "\0\0".
      fpHugeBufferAlloc(lpPrePropList, (HPSTR)&tmpLen, sizeof(short));

      if (tmpLen)
         {
         char nullstr[2] = "\0";
         // if "true" len is 0, use nullstr
         if (nLen == 0)
            lParam = (long)(HPSTR)nullstr;
         fpHugeBufferAlloc(lpPrePropList, (HPSTR)lParam, (long)(tmpLen + 1));
         }
      }
      break;
   }

return (TRUE);
}


FPLIB_(void) fpOCXInitPreProps(LPFP_HUGEBUFFER lpPrePropList)
{
//fpCreate();
fpHugeBufferInit(lpPrePropList, 500);
}


FPLIB_(BOOL) fpOCXLoadPreProps(HWND hWnd, LPFP_HUGEBUFFER lpPrePropList,
                       LPVOID lpStruct, FP_PROPFUNC lpSetPropProc,
                       LPFP_PROPLIST lpPropList)
{
if (lpPrePropList->hBuffer)
   {
   HPSTR lpBuffer;
   WORD  wPropIndex;
   long  nAt = 0;

   lpBuffer = (HPSTR)GlobalLock(lpPrePropList->hBuffer);

   while (nAt < lpPrePropList->lBufferLen)
      {
      wPropIndex = *(WORD far *)(&lpBuffer[nAt]);
      nAt += sizeof(WORD);

      if (lpPropList[wPropIndex].pPropInfo->fl & fpPF_fSaveMsg
		  // new versions may have fSaveMsg and fLoadDataOnly, which means
		  // the prop was saved in the FRM and is now saved in the FRX.
		  // if this prop appears in the preprop list, load it as a regular prop,
		  // not as a blob -scl
		  && !(lpPropList[wPropIndex].pPropInfo->fl & fpPF_fLoadDataOnly))
         {
         long lLen = *(LPLONG)(&lpBuffer[nAt]);

         if (lLen)
            {
            HGLOBAL hBlob;

            if (hBlob = GlobalAlloc(GHND, sizeof(long) + lLen))
               {
               HPSTR lpBlob = (HPSTR)GlobalLock(hBlob);

               _fmemcpy(lpBlob, &lpBuffer[nAt], (size_t)(sizeof(long) + lLen));
               GlobalUnlock(hBlob);
               SendMessage(hWnd, FPM_LOADPROPERTY, wPropIndex, *(LPLONG)&hBlob);
               GlobalFree(hBlob);
               }
            }

         nAt += (long)(sizeof(long) + lLen);
         }

      else
         {
         switch (lpPropList[wPropIndex].pPropInfo->fl & fpPF_datatype)
            {
            case DT_ENUM:
            case DT_SHORT:
            case DT_BOOL:
            case DT_COLOR:
            case DT_LONG:
            case DT_XPOS:
            case DT_XSIZE:
            case DT_YPOS:
            case DT_YSIZE:
               lpSetPropProc(lpStruct, wPropIndex, *(LPLONG)(&lpBuffer[nAt]));
               nAt += sizeof(long);
               break;

            case DT_PICTURE:
               {
               LPPICTUREDISP lpPictureDisp;

               lpSetPropProc(lpStruct, wPropIndex, *(LPLONG)(&lpBuffer[nAt]));
               lpPictureDisp = (LPPICTUREDISP)*(LPLONG)(&lpBuffer[nAt]);

               if (lpPictureDisp)
                  lpPictureDisp->Release();

               nAt += sizeof(long);
			   }
               break;

            case DT_REAL:
               lpSetPropProc(lpStruct, wPropIndex,
                             (long)(LPVOID)&lpBuffer[nAt]);
               nAt += sizeof(float);
               break;

            case fpDT_HLSTR:
            case DT_HSZ:
               {
               short nLen;

               nLen = *(short far *)(&lpBuffer[nAt]);
               nAt += sizeof(short);

               if (nLen)
                  {
                  lpSetPropProc(lpStruct, wPropIndex, (long)&lpBuffer[nAt]);
                  nAt += nLen + 1;
                  }
               }
               break;

            }
         }
      }

   GlobalUnlock(lpPrePropList->hBuffer);
   fpHugeBufferFree(lpPrePropList);
   }

return (TRUE);
}


FPLIB_(BOOL) fpOCXFreePreProps(LPFP_HUGEBUFFER lpPrePropList, LPFP_PROPLIST lpPropList)
{
if (lpPrePropList->hBuffer)
   {
   HPSTR lpBuffer;
   WORD  wPropIndex;
   long  nAt = 0;

   lpBuffer = (HPSTR)GlobalLock(lpPrePropList->hBuffer);

   while (nAt < lpPrePropList->lBufferLen)
      {
      wPropIndex = *(WORD far *)(&lpBuffer[nAt]);
      nAt += sizeof(WORD);

      if (lpPropList[wPropIndex].pPropInfo->fl & fpPF_fSaveMsg
		  // new versions may have fSaveMsg and fLoadDataOnly, which means
		  // the prop was saved in the FRM and is now saved in the FRX.
		  // if this prop appears in the preprop list, load it as a regular prop,
		  // not as a blob -scl
		  && !(lpPropList[wPropIndex].pPropInfo->fl & fpPF_fLoadDataOnly))
         {
         long lLen = *(LPLONG)(&lpBuffer[nAt]);
         nAt += (long)(sizeof(long) + lLen);
         }

      else
         {
         switch (lpPropList[wPropIndex].pPropInfo->fl & fpPF_datatype)
            {
            case DT_ENUM:
            case DT_SHORT:
            case DT_BOOL:
            case DT_COLOR:
            case DT_LONG:
            case DT_XPOS:
            case DT_XSIZE:
            case DT_YPOS:
            case DT_YSIZE:
               nAt += sizeof(long);
               break;

            case DT_PICTURE:
               {
               LPPICTUREDISP lpPictureDisp;

               lpPictureDisp = (LPPICTUREDISP)*(LPLONG)(&lpBuffer[nAt]);

               if (lpPictureDisp)
                  lpPictureDisp->Release();

               nAt += sizeof(long);
               }
               break;

            case DT_REAL:
               nAt += sizeof(float);
               break;

            case fpDT_HLSTR:
            case DT_HSZ:
               {
               short nLen;

               nLen = *(short far *)(&lpBuffer[nAt]);
               nAt += sizeof(short);

               if (nLen)
                  nAt += nLen + 1;
               }
               break;

            }
         }
      }

   GlobalUnlock(lpPrePropList->hBuffer);
   fpHugeBufferFree(lpPrePropList);
   }

//fpDestroy();
return (TRUE);
}

//KM  sel108
HPSTR fpOCXGetPropData(LPFP_HUGEBUFFER lpPrePropList, LPFP_PROPLIST lpPropList,
                       WORD wProp)
{
   return fpOCXGetPropDataPrimative(lpPrePropList, lpPropList, wProp, FALSE);
}

HPSTR fpOCXGetPropDataPrimative(LPFP_HUGEBUFFER lpPrePropList, LPFP_PROPLIST lpPropList,
                       WORD wProp, BOOL fLastProp)
{
HPSTR lpRet = NULL;
if (lpPrePropList->hBuffer)
   {
   HPSTR lpBuffer;
   WORD  wPropIndex;
   long  nAt = 0;

   lpBuffer = (HPSTR)GlobalLock(lpPrePropList->hBuffer);

   while (nAt < lpPrePropList->lBufferLen)
      {
      wPropIndex = *(WORD far *)(&lpBuffer[nAt]);
      nAt += sizeof(WORD);

      if (wPropIndex == wProp)
      {
		 if (!fLastProp)
           return (&lpBuffer[nAt]);
		 else lpRet = &lpBuffer[nAt];

      }
      if (lpPropList[wPropIndex].pPropInfo->fl & fpPF_fSaveMsg)
         {
         long lLen = *(LPLONG)(&lpBuffer[nAt]);
         nAt += (long)(sizeof(long) + lLen);
         }

      else
         {
         switch (lpPropList[wPropIndex].pPropInfo->fl & fpPF_datatype)
            {
            case DT_ENUM:
            case DT_SHORT:
            case DT_BOOL:
            case DT_COLOR:
            case DT_LONG:
            case DT_XPOS:
            case DT_XSIZE:
            case DT_YPOS:
            case DT_YSIZE:
            case DT_PICTURE:
               nAt += sizeof(long);
               break;

            case DT_REAL:
               nAt += sizeof(float);
               break;

            case fpDT_HLSTR:
            case DT_HSZ:
               {
               short nLen;

               nLen = *(short far *)(&lpBuffer[nAt]);
               nAt += sizeof(short);

               if (nLen)
                  nAt += nLen + 1;
               }
               break;

            }
         }
      }

   GlobalUnlock(lpPrePropList->hBuffer);
   }

return lpRet;
}

#endif // ifndef NO_MFC
