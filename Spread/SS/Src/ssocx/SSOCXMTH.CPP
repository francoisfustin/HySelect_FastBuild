// ssocxmth.cpp : Implementation of the CSsocxCtrl OLE control methods.

// Tiwanna reported:9167 * jb fixed on 28.jul.01
// If you saved a tab file with no extension, it defaulted to .tb2 Updated it to .tb3

#define NO_VBX_SUPPORT

#if ((defined(FP_OCX) || defined(FP_DLL)) && defined(WIN32))
#define WIN32_LEAN_AND_MEAN   //exclude conflicts with VBAPI.H in WIN32
#include "stdafx.h"
#include "ssocx.h"
#include "ssocxctl.h"
#include "ssocxppg.h"

//#include "..\..\..\fplibs\fptools\src\fpvbsub.h"            // includes fpvbsub.h
#include "..\..\..\fplibs\fptools\src\fpconvrt.h"            // includes fpvbsub.h
//typedef USHORT          ERR;    // not defined in fpvbsub.h for __cplusplus  
#include "..\..\..\fplibs\fptools\src\fptstr.h"

#include "opmodepr.h"
#include "virtualp.h"
#include "displayp.h"
#include "editmode.h"
#include "headersp.h"
#include "..\sssrc\spread\ss_save4.h"

#include "..\sssrc\spread\ss_alloc.h"
#include "..\sssrc\spread\ss_bool.h"
#include "..\sssrc\spread\ss_bord.h"
#include "..\sssrc\spread\ss_calc.h"
#include "..\sssrc\spread\ss_cell.h"
#include "..\sssrc\spread\ss_col.h"
#include "..\sssrc\spread\ss_curs.h"
#include "..\sssrc\spread\ss_data.h"
#include "..\sssrc\spread\ss_dde.h"
#include "..\sssrc\spread\ss_doc.h"
#include "..\sssrc\spread\ss_draw.h"
#include "..\sssrc\spread\ss_font.h"
#include "..\sssrc\spread\ss_formu.h"

#if !defined(SS_V80) && !defined(SS_V70)
#include "..\..\..\fplibs\fptools\jpeg\jpeglib.h"
#endif
#include "..\sssrc\spread\ss_html.h"
#include "..\sssrc\spread\ss_xml.h"
#include "..\sssrc\spread\ss_text.h"
#include "..\sssrc\spread\ss_main.h"
#include "..\sssrc\spread\ss_multi.h"
#include "..\sssrc\spread\ss_row.h"
#include "..\sssrc\spread\ss_save.h"
#include "..\sssrc\spread\ss_scrol.h"
#include "..\sssrc\spread\ss_type.h"
#include "..\sssrc\spread\ss_user.h"
#include "..\sssrc\spread\ss_virt.h"
#include "..\vbsrc\vbdattim.h"
#if defined(SS_V30) && defined(WIN32)
	#include "..\sssrc\spread\ss_excel.h"
	#include "..\sssrc\spread\ss_html.h"
#endif
#include "..\sssrc\classes\wintools.h"
#include "ssocxdb.h"
#include "..\vbsrc\vbsscnvt.h"
#ifdef SS_V70
#include "ocidl.h"
#include "olectl.h"
#include "..\..\..\ss\src\sssrc\spread\ss_spict.h"
#include "..\..\..\fplibs\CxImage\CxImage\fpimages.h"
#endif
#endif
#ifdef SS_V35
#include "..\sssrc\spread\ss_xml.h"
// this define has been added to the Spread 3.5 projects
//#define TT_V2
#endif

#include "texttip.h"

#ifdef SS_V40
#include "..\sssrc\spread\ss_span.h"
#endif // SS_V40

extern "C" {
#include "..\..\..\fplibs\fptools\src\fpconvrt.h"
#include "..\vbsrc\vbspread.h"
}

#ifdef _DEBUG
#undef THIS_FILE
static char BASED_CODE THIS_FILE[] = __FILE__;
#endif

//DBOCX...
// These functions are found in ..\vbsrc\vbsprea5.c
extern "C" {
BOOL FAR PASCAL _export _SpreadGetCellDirtyFlag(HWND hWnd, SS_COORD Col, 
  SS_COORD Row);
BOOL FAR PASCAL _export __SpreadSetCellDirtyFlag(LONG lObject, HWND hWnd, 
  SS_COORD Col, SS_COORD Row, BOOL fDirty);
BOOL FAR PASCAL _export __SpreadGetDataFillData(HWND hWnd, LONG lObject, LPVAR lpVar, LPSTR lpszText, //DBOCX
                                        WORD wVarType);
BOOL FAR PASCAL _export __SpreadSetDataFillData(HWND hWnd, LONG lObject, LPVBSPREAD lpSpread,   //DBOCX
                                        HLSTR hlstr, long lData, BOOL fUselData);
#ifdef WIN32
//LPSTR __declspec(dllexport) __stdcall fpStr2OLE(LPSTR lpszInput);    // found in fpconvrt.cpp
//void __declspec(dllexport) __stdcall fpPatchVBVariant(VOID FAR *lpVariant);
#endif
}  //extern C
//DBOCX.

#define SSVB_ARRAYTYPE_SHORT   0
#define SSVB_ARRAYTYPE_LONG    1
#define SSVB_ARRAYTYPE_FLOAT   2
#define SSVB_ARRAYTYPE_DOUBLE  3
#define SSVB_ARRAYTYPE_VARIANT 4

//--------------------------------------------------------------------

extern "C" BOOL SS_OcxResetOnLoad(LPSS_BOOK lpBook)
{
  CSsocxCtrl FAR* lpCtrl = (CSsocxCtrl FAR*)lpBook->lpOleControl;
  lpCtrl->m_bCalcDependencies = TRUE;
#ifdef SS_V30
  lpCtrl->m_lPrintNextPageBreakCol = 1;
  lpCtrl->m_lPrintNextPageBreakRow = 1;
#endif
  return TRUE;
}

//--------------------------------------------------------------------

extern "C" BOOL SS_OcxLoad4CalcDepend(LPSS_BOOK lpBook, LPSS_REC_CALCDEPEND lpRec)
{
  CSsocxCtrl FAR* lpCtrl = (CSsocxCtrl FAR*)lpBook->lpOleControl;
  lpCtrl->m_bCalcDependencies = lpRec->bCalcDepend;
  return TRUE;
}

//--------------------------------------------------------------------

extern "C" BOOL SS_OcxSave4CalcDepend(LPSS_BOOK lpBook, LPSS_BUFF lpBuff)
{
  CSsocxCtrl FAR* lpCtrl = (CSsocxCtrl FAR*)lpBook->lpOleControl;
  SS_REC_CALCDEPEND rec;
  BOOL bRet = TRUE;
  if( lpCtrl->m_bCalcDependencies != TRUE )
  {
    rec.nRecType = SS_RID_CALCDEPEND;
    rec.lRecLen = sizeof(rec);
    rec.bCalcDepend = lpCtrl->m_bCalcDependencies;
    bRet = SS_BuffAppend(lpBuff, &rec, sizeof(rec));
  }
  return bRet;
}

//--------------------------------------------------------------------

/*
extern "C" BOOL SS_OcxLoad4Print(LPSS_BOOK lpBook, LPSS_REC_PRINT lpRec)
{
  LPSPREADSHEET lpSS = SS_BookLockSheetIndex(lpBook, 0);
  CSsocxCtrl FAR* lpCtrl = (CSsocxCtrl FAR*)lpBook->lpOleControl;
  LPBYTE lpAbortMsg = (LPBYTE)lpRec + sizeof(*lpRec);
  LPBYTE lpFooter = (LPBYTE)lpAbortMsg + lpRec->lAbortMsgLen;
  LPBYTE lpHeader = (LPBYTE)lpFooter + lpRec->lFooterLen;
  LPBYTE lpJobName = (LPBYTE)lpHeader + lpRec->lHeaderLen;

  lpSS->PrintOptions.pf.fDrawBorder = lpRec->bDrawBorder;
  lpSS->PrintOptions.pf.fDrawColors = lpRec->bDrawColor;
  lpSS->PrintOptions.pf.fDrawShadows = lpRec->bDrawShadows;
  lpSS->PrintOptions.pf.fShowGrid = lpRec->bShowGrid;
  lpSS->PrintOptions.pf.fShowColHeaders = lpRec->bShowColHeaders;
  lpSS->PrintOptions.pf.fShowRowHeaders = lpRec->bShowRowHeaders;
  lpSS->PrintOptions.pf.fUseDataMax = lpRec->bUseDataMax;
  lpSS->PrintOptions.pf.dPrintType = lpRec->nPrintType;

  lpSS->PrintOptions.pf.nPageEnd = lpRec->nPageEnd;
  lpSS->PrintOptions.pf.nPageStart = lpRec->nPageStart;
  lpSS->PrintOptions.pf.fMarginLeft = (float)lpRec->lMarginLeft / (float)1440.0;
  lpSS->PrintOptions.pf.fMarginTop = (float)lpRec->lMarginTop / (float)1440.0;
  lpSS->PrintOptions.pf.fMarginBottom = (float)lpRec->lMarginBottom / (float)1440.0;
  lpSS->PrintOptions.pf.fMarginRight = (float)lpRec->lMarginRight / (float)1440.0;
  lpSS->PrintOptions.pf.wOrientation = lpRec->nOrientation;

  lpCtrl->SetPrintFooter((LPCTSTR)lpFooter);
  lpCtrl->SetPrintHeader((LPCTSTR)lpHeader);
  lpCtrl->SetPrintAbortMsg((LPCTSTR)lpAbortMsg);
  lpCtrl->SetPrintJobName((LPCTSTR)lpJobName);

  SS_BookUnlockSheetIndex(lpBook, 0);
  return TRUE;
}

//--------------------------------------------------------------------

extern "C" BOOL SS_OcxSave4Print(LPSS_BOOK lpBook, LPSS_BUFF lpBuff)
{
  LPSPREADSHEET lpSS = SS_BookLockSheetIndex(lpBook, 0);
  CSsocxCtrl FAR* lpCtrl = (CSsocxCtrl FAR*)lpBook->lpOleControl;
  SS_REC_PRINT rec;
  BOOL bRet = TRUE;
  if( lpSS->PrintOptions.pf.fDrawBorder != TRUE ||
      lpSS->PrintOptions.pf.fDrawColors != FALSE ||
      lpSS->PrintOptions.pf.fDrawShadows != TRUE ||
      lpSS->PrintOptions.pf.fShowGrid != TRUE ||
      lpSS->PrintOptions.pf.fShowColHeaders != TRUE ||
      lpSS->PrintOptions.pf.fShowRowHeaders != TRUE ||
      lpSS->PrintOptions.pf.fUseDataMax != TRUE ||
      lpSS->PrintOptions.pf.dPrintType != 0 ||
      lpSS->PrintOptions.pf.nPageEnd != 1 ||
      lpSS->PrintOptions.pf.nPageStart != 1 ||
      lpSS->PrintOptions.pf.fMarginLeft != 0 ||
      lpSS->PrintOptions.pf.fMarginTop != 0 ||
      lpSS->PrintOptions.pf.fMarginBottom != 0 ||
      lpSS->PrintOptions.pf.fMarginRight != 0 ||
      lpSS->PrintOptions.pf.wOrientation != 0 ||
      lpSS->PrintOptions.pf.hFooter ||
      lpSS->PrintOptions.pf.hHeader ||
      lpBook->hPrintAbortMsg ||
      lpBook->hPrintJobName )
  {
    LPTSTR lpszFooter = _T("");
    LPTSTR lpszHeader = _T("");
    LPTSTR lpszAbortMsg = _T("");
    LPTSTR lpszJobName = _T("");

    if (lpSS->PrintOptions.pf.hFooter)
      lpszFooter = (LPTSTR)GlobalLock(lpSS->PrintOptions.pf.hFooter);

    if (lpSS->PrintOptions.pf.hHeader)
      lpszHeader = (LPTSTR)GlobalLock(lpSS->PrintOptions.pf.hHeader);

    if (lpBook->hPrintAbortMsg)
      lpszAbortMsg = (LPTSTR)tbGlobalLock(lpBook->hPrintAbortMsg);

    if (lpBook->hPrintJobName)
      lpszJobName = (LPTSTR)tbGlobalLock(lpBook->hPrintJobName);

    rec.nRecType = SS_RID_PRINT;
    rec.lRecLen = sizeof(rec);
    rec.bDrawBorder = lpSS->PrintOptions.pf.fDrawBorder;
    rec.bDrawColor = lpSS->PrintOptions.pf.fDrawColors;
    rec.bDrawShadows = lpSS->PrintOptions.pf.fDrawShadows;
    rec.bShowGrid = lpSS->PrintOptions.pf.fShowGrid;
    rec.bShowColHeaders = lpSS->PrintOptions.pf.fShowColHeaders;
    rec.bShowRowHeaders = lpSS->PrintOptions.pf.fShowRowHeaders;
    rec.bUseDataMax = lpSS->PrintOptions.pf.fUseDataMax;
    rec.nPrintType = lpSS->PrintOptions.pf.dPrintType;

    rec.nPageEnd = lpSS->PrintOptions.pf.nPageEnd;
    rec.nPageStart = lpSS->PrintOptions.pf.nPageStart;
    rec.lMarginLeft = (long)((float)lpSS->PrintOptions.pf.fMarginLeft * (float)1440.0);
    rec.lMarginTop = (long)((float)lpSS->PrintOptions.pf.fMarginTop * (float)1440.0);
    rec.lMarginBottom = (long)((float)lpSS->PrintOptions.pf.fMarginBottom * (float)1440.0);
    rec.lMarginRight = (long)((float)lpSS->PrintOptions.pf.fMarginRight * (float)1440.0);
    rec.nOrientation = lpSS->PrintOptions.pf.wOrientation;
    rec.lAbortMsgLen = (lstrlen(lpszAbortMsg) + 1) * sizeof(TCHAR);
    rec.lFooterLen = (lstrlen(lpszFooter) + 1) * sizeof(TCHAR);
    rec.lHeaderLen = (lstrlen(lpszHeader) + 1) * sizeof(TCHAR);
    rec.lJobNameLen = (lstrlen(lpszJobName) + 1) * sizeof(TCHAR);
    rec.lRecLen += rec.lAbortMsgLen;
    rec.lRecLen += rec.lFooterLen;
    rec.lRecLen += rec.lHeaderLen;
    rec.lRecLen += rec.lJobNameLen;
    bRet &= SS_BuffAppend(lpBuff, &rec, sizeof(rec));
    bRet &= SS_BuffAppend(lpBuff, (LPVOID)(LPCTSTR)lpszAbortMsg, rec.lAbortMsgLen);
    bRet &= SS_BuffAppend(lpBuff, (LPVOID)(LPCTSTR)lpszFooter, rec.lFooterLen);
    bRet &= SS_BuffAppend(lpBuff, (LPVOID)(LPCTSTR)lpszHeader, rec.lHeaderLen);
    bRet &= SS_BuffAppend(lpBuff, (LPVOID)(LPCTSTR)lpszJobName, rec.lJobNameLen);

    if (lpSS->PrintOptions.pf.hFooter)
      GlobalUnlock(lpSS->PrintOptions.pf.hFooter);

    if (lpSS->PrintOptions.pf.hHeader)
      GlobalUnlock(lpSS->PrintOptions.pf.hHeader);

    if (lpBook->hPrintAbortMsg)
      tbGlobalUnlock(lpBook->hPrintAbortMsg);

    if (lpBook->hPrintJobName)
      tbGlobalUnlock(lpBook->hPrintJobName);
  }
  SS_BookUnlockSheetIndex(lpBook, 0);
  return bRet;
}

//--------------------------------------------------------------------

extern "C" BOOL SS_OcxLoad4Print2(LPSS_BOOK lpBook, LPSS_REC_PRINT2 lpRec)
{
  LPSPREADSHEET lpSS = SS_BookLockSheetIndex(lpBook, 0);

  lpSS->PrintOptions.pf.fSmartPrint = lpRec->bSmartPrint;
  lpSS->PrintOptions.pf.lFirstPageNumber = lpRec->lFirstPageNumber;

  switch (lpRec->nPageOrder)
  {
    case 1: lpSS->PrintOptions.pf.dPrintType |= SS_PRINTTYPE_DOWNTHENOVER; break;
    case 2: lpSS->PrintOptions.pf.dPrintType |= SS_PRINTTYPE_OVERTHENDOWN; break;
  }

  SS_BookUnlockSheetIndex(lpBook, 0);
  return TRUE;
}

//--------------------------------------------------------------------

extern "C" BOOL SS_OcxSave4Print2(LPSS_BOOK lpBook, LPSS_BUFF lpBuff)
{
  LPSPREADSHEET lpSS = SS_BookLockSheetIndex(lpBook, 0);
  CSsocxCtrl FAR* lpCtrl = (CSsocxCtrl FAR*)lpBook->lpOleControl;
  SS_REC_PRINT2 rec;
  BOOL bRet = TRUE;

  rec.nRecType = SS_RID_PRINT2;
  rec.lRecLen = sizeof(rec);

  if (lpSS->PrintOptions.pf.fSmartPrint != FALSE ||
      (lpSS->PrintOptions.pf.dPrintType & SS_PRINTTYPE_DOWNTHENOVER) ||
      (lpSS->PrintOptions.pf.dPrintType & SS_PRINTTYPE_OVERTHENDOWN) ||
      lpSS->PrintOptions.pf.lFirstPageNumber != 1)
  {
    rec.bSmartPrint = lpSS->PrintOptions.pf.fSmartPrint;
    rec.lFirstPageNumber = lpSS->PrintOptions.pf.lFirstPageNumber;
    if (lpSS->PrintOptions.pf.dPrintType & SS_PRINTTYPE_DOWNTHENOVER)
      rec.nPageOrder = 1;
    else if (lpSS->PrintOptions.pf.dPrintType & SS_PRINTTYPE_OVERTHENDOWN)
      rec.nPageOrder = 2;

    bRet &= SS_BuffAppend(lpBuff, &rec, sizeof(rec));
  }
  SS_BookUnlockSheetIndex(lpBook, 0);
  return bRet;
}
*/

//--------------------------------------------------------------------

BOOL CALLBACK CustFuncProc(long hSS, LPCTSTR lpszFunc,
                           LPSS_VALUE lpResult, LPSS_VALUE lpArgs,
                           short nArgCnt)
{
  LPSPREADSHEET lpSS = (LPSPREADSHEET)tbGlobalLock((TBGLOBALHANDLE)hSS);
  CSsocxCtrl FAR* lpCtrl = (CSsocxCtrl FAR*)lpSS->lpBook->lpOleControl;
  SS_COORD lCol;
  SS_COORD lRow;
  short nStatus = SS_VALUE_STATUS_OK;
  LPSS_VALUE lpCFResultPrev = lpCtrl->m_lpCFResult;
  LPSS_VALUE lpCFArgsPrev = lpCtrl->m_lpCFArgs;
  short nCFArgCntPrev = lpCtrl->m_nCFArgCnt;
  //BOOL   fFreeParam = FALSE;
  // Convert "lpszFunc" from TSTR to BSTR
  //if (sizeof(OLECHAR) != sizeof(TCHAR))
  //{
  //  lpszFunc = (LPCTSTR)fpSysAllocBSTRFromTString((LPTSTR)lpszFunc);
  //  fFreeParam = TRUE;
  //}  

  lpCtrl->m_lpCFResult = lpResult;
  lpCtrl->m_lpCFArgs = lpArgs;
  lpCtrl->m_nCFArgCnt = nArgCnt;
  SS_GetCellSendingMsg(lpSS, &lCol, &lRow);
  SS_AdjustCellCoordsOut(lpSS, &lCol, &lRow);
  lpSS->lpBook->nSheetSendingMsg = lpSS->nSheetIndex;

#ifdef SS_V35
  if (lpSS->lpBook->bScriptEnhanced)
     {
     VARIANT arg;
     VARIANT varParam;

     memset(&arg, 0, sizeof(VARIANT));
     memset(&varParam, 0, sizeof(VARIANT));
     varParam.vt = VT_I2; 
     varParam.iVal = nStatus; 

     lpCtrl->FireScriptCustomFunction(lpszFunc, nArgCnt, lCol, lRow, &varParam);    
     VariantChangeType(&arg, &varParam, 0, VT_I2); 
     nStatus = arg.iVal; 
     }
  else
     lpCtrl->FireCustomFunction(lpszFunc, nArgCnt, lCol, lRow, &nStatus);
#else
  #ifndef SSE_NO_CUSTOMFUNCTION
  lpCtrl->FireCustomFunction(lpszFunc, nArgCnt, lCol, lRow, &nStatus);
  #endif
#endif  //SS_V35

  if (nStatus != SS_VALUE_STATUS_OK)
  {
    if (SS_VALUE_STATUS_OK == lpResult->Status
        && SS_VALUE_TYPE_STR == lpResult->Type
        && lpResult->Val.hValStr)
      GlobalFree(lpResult->Val.hValStr);
    lpResult->Status = (BYTE)nStatus;
  }
  lpCtrl->m_lpCFResult = lpCFResultPrev;
  lpCtrl->m_lpCFArgs = lpCFArgsPrev;
  lpCtrl->m_nCFArgCnt = nCFArgCntPrev;
  tbGlobalUnlock((TBGLOBALHANDLE)hSS);

  //if (fFreeParam && lpszFunc)
  //  fpSysFreeTString((LPTSTR)lpszFunc);
  return TRUE;
}

//--------------------------------------------------------------------

#ifndef SSF_NO_AddCustomFunction
BOOL CSsocxCtrl::AddCustomFunction(LPCTSTR FunctionName, short ParameterCnt) 
{
  LPSS_BOOK lpBook = LockBook();
  BOOL bRet = SS_AddCustomFunctionExt(lpBook, FunctionName,
                                      ParameterCnt, ParameterCnt,
                                      (FARPROC)CustFuncProc, 0);
  UnlockBook();
  return bRet;
} 
#endif

//--------------------------------------------------------------------

#ifndef SSF_NO_AddCustomFunctionExt
BOOL CSsocxCtrl::AddCustomFunctionExt(LPCTSTR FunctionName, short MinParamCnt,
                                      short MaxParamCnt, long Flags) 
{
  LPSS_BOOK lpBook = LockBook();
  BOOL bRet = SS_AddCustomFunctionExt(lpBook, FunctionName,
                                      MinParamCnt, MaxParamCnt,
                                      (FARPROC)CustFuncProc, Flags);
  UnlockBook();
  return bRet;
} 
#endif

//--------------------------------------------------------------------

#ifndef SSF_NO_CFGetCellParam
void CSsocxCtrl::CFGetCellParam(short Param, long FAR* Col, long FAR* Row)
{
  LPSPREADSHEET lpSS = LockSheet();
  if( 1 <= Param && Param <= m_nCFArgCnt )
  {
    LPSS_VALUE lpArg = m_lpCFArgs + Param - 1;
    if( SS_VALUE_STATUS_OK == lpArg->Status )
    {
      if( SS_VALUE_TYPE_CELL == lpArg->Type )
      {
        *Col = lpArg->Val.ValCell.Col;
        *Row = lpArg->Val.ValCell.Row;
      }
      else if( SS_VALUE_TYPE_RANGE == lpArg->Type )
      {
        *Col = lpArg->Val.ValRange.Col1;
        *Row = lpArg->Val.ValRange.Row1;
      }
    }
  }
  UnlockSheet();
}
#endif

//--------------------------------------------------------------------

#ifndef SSF_NO_CFGetCellParamExt
#ifdef SS_V70
void CSsocxCtrl::CFGetCellParamExt(short Param, short FAR* Sheet, long FAR* Col, long FAR* Row)
{
  LPSPREADSHEET lpSS = LockSheet();
  if( 1 <= Param && Param <= m_nCFArgCnt )
  {
    LPSS_VALUE lpArg = m_lpCFArgs + Param - 1;
    if( SS_VALUE_STATUS_OK == lpArg->Status )
    {
      if( SS_VALUE_TYPE_CELL == lpArg->Type )
      {
        *Sheet = lpArg->Val.ValCell.nSheet;
        *Col = lpArg->Val.ValCell.Col;
        *Row = lpArg->Val.ValCell.Row;
      }
      else if( SS_VALUE_TYPE_RANGE == lpArg->Type )
      {
        *Sheet = lpArg->Val.ValRange.nSheet;
        *Col = lpArg->Val.ValRange.Col1;
        *Row = lpArg->Val.ValRange.Row1;
      }
    }
  }
  UnlockSheet();
}
#endif
#endif

//--------------------------------------------------------------------

#ifndef SSF_NO_CFGetDoubleParam
double CSsocxCtrl::CFGetDoubleParam(short Param) 
{
  LPSPREADSHEET lpSS = LockSheet();
  double dfRet = 0.0;
  if( 1 <= Param && Param <= m_nCFArgCnt )
  {
    LPSS_VALUE lpArg = m_lpCFArgs + Param - 1;
    if( SS_VALUE_STATUS_OK == lpArg->Status )
    {
      if( SS_VALUE_TYPE_DOUBLE == lpArg->Type )
        dfRet = lpArg->Val.ValDouble;
      else if( SS_VALUE_TYPE_LONG == lpArg->Type )
        dfRet = (double)lpArg->Val.ValLong;
      else if( SS_VALUE_TYPE_STR == lpArg->Type )
      {
        StringToFloat((LPTSTR)GlobalLock(lpArg->Val.hValStr), &dfRet);
        GlobalUnlock(lpArg->Val.hValStr);
      }
    }
  }
  UnlockSheet();
  return dfRet;
}
#endif

//--------------------------------------------------------------------

#ifndef SSF_NO_CFGetDoubleParamExt
void CSsocxCtrl::CFGetDoubleParamExt(short Param, double FAR* ParamValue) 
{
  LPSPREADSHEET lpSS = LockSheet();
  double dfRet = 0.0;
  if( 1 <= Param && Param <= m_nCFArgCnt )
  {
    LPSS_VALUE lpArg = m_lpCFArgs + Param - 1;
    if( SS_VALUE_STATUS_OK == lpArg->Status )
    {
      if( SS_VALUE_TYPE_DOUBLE == lpArg->Type )
        dfRet = lpArg->Val.ValDouble;
      else if( SS_VALUE_TYPE_LONG == lpArg->Type )
        dfRet = (double)lpArg->Val.ValLong;
      else if( SS_VALUE_TYPE_STR == lpArg->Type )
      {
        StringToFloat((LPTSTR)GlobalLock(lpArg->Val.hValStr), &dfRet);
        GlobalUnlock(lpArg->Val.hValStr);
      }
    }
  }
  UnlockSheet();
  *ParamValue = dfRet;
}
#endif

//--------------------------------------------------------------------

#ifndef SSF_NO_CFGetLongParam
long CSsocxCtrl::CFGetLongParam(short Param) 
{
  LPSPREADSHEET lpSS = LockSheet();
  long lRet = 0;
  if( 1 <= Param && Param <= m_nCFArgCnt )
  {
    LPSS_VALUE lpArg = m_lpCFArgs + Param - 1;
    if( SS_VALUE_STATUS_OK == lpArg->Status )
    {
      if( SS_VALUE_TYPE_DOUBLE == lpArg->Type )
        lRet = (long)lpArg->Val.ValDouble;
      else if( SS_VALUE_TYPE_LONG == lpArg->Type )
        lRet = lpArg->Val.ValLong;
      else if( SS_VALUE_TYPE_STR == lpArg->Type )
      {
        lRet = StringToLong((LPTSTR)GlobalLock(lpArg->Val.hValStr));
        GlobalUnlock(lpArg->Val.hValStr);
      }
    }
  }
  UnlockSheet();
  return lRet;
}
#endif

//--------------------------------------------------------------------

#ifndef SSF_NO_CFGetParamInfo
#ifdef SS_V70
BOOL CSsocxCtrl::CFGetParamInfo(short Param, long FAR* Type, long FAR* Status) 
{
  LPSPREADSHEET lpSS = LockSheet();
  BOOL bRet = FALSE;
  if( 1 <= Param && Param <= m_nCFArgCnt )
  {
    if( Type )
      *Type = (long)m_lpCFArgs[Param-1].Type;
    if( Status )
      *Status = (long)m_lpCFArgs[Param-1].Status;
    bRet = TRUE;
  }
  UnlockSheet();
  return bRet;
}
#else
BOOL CSsocxCtrl::CFGetParamInfo(short Param, short FAR* Type, short FAR* Status) 
{
  LPSPREADSHEET lpSS = LockSheet();
  BOOL bRet = FALSE;
  if( 1 <= Param && Param <= m_nCFArgCnt )
  {
    if( Type )
      *Type = m_lpCFArgs[Param-1].Type;
    if( Status )
      *Status = m_lpCFArgs[Param-1].Status;
    bRet = TRUE;
  }
  UnlockSheet();
  return bRet;
}
#endif
#endif

//--------------------------------------------------------------------

#ifndef SSF_NO_CFGetRangeParam
void CSsocxCtrl::CFGetRangeParam(short Param, long FAR* Col, long FAR* Row,
                                 long FAR* Col2, long FAR* Row2)
{
  LPSPREADSHEET lpSS = LockSheet();
  if( 1 <= Param && Param <= m_nCFArgCnt )
  {
    LPSS_VALUE lpArg = m_lpCFArgs + Param - 1;
    if( SS_VALUE_STATUS_OK == lpArg->Status )
    {
      if( SS_VALUE_TYPE_CELL == lpArg->Type )
      {
        *Col = lpArg->Val.ValCell.Col;
        *Row = lpArg->Val.ValCell.Row;
        *Col2 = lpArg->Val.ValCell.Col;
        *Row2 = lpArg->Val.ValCell.Row;
      }
      else if( SS_VALUE_TYPE_RANGE == lpArg->Type )
      {
        *Col = lpArg->Val.ValRange.Col1;
        *Row = lpArg->Val.ValRange.Row1;
        *Col2 = lpArg->Val.ValRange.Col2;
        *Row2 = lpArg->Val.ValRange.Row2;
      }
    }
  }
  UnlockSheet();
}
#endif

//--------------------------------------------------------------------

#ifndef SSF_NO_CFGetRangeParamExt
#ifdef SS_V70
void CSsocxCtrl::CFGetRangeParamExt(short Param, short FAR* Sheet, long FAR* Col, long FAR* Row,
                                    long FAR* Col2, long FAR* Row2)
{
  LPSPREADSHEET lpSS = LockSheet();
  if( 1 <= Param && Param <= m_nCFArgCnt )
  {
    LPSS_VALUE lpArg = m_lpCFArgs + Param - 1;
    if( SS_VALUE_STATUS_OK == lpArg->Status )
    {
      if( SS_VALUE_TYPE_CELL == lpArg->Type )
      {
        *Sheet = lpArg->Val.ValCell.nSheet;
        *Col = lpArg->Val.ValCell.Col;
        *Row = lpArg->Val.ValCell.Row;
        *Col2 = lpArg->Val.ValCell.Col;
        *Row2 = lpArg->Val.ValCell.Row;
      }
      else if( SS_VALUE_TYPE_RANGE == lpArg->Type )
      {
        *Sheet = lpArg->Val.ValRange.nSheet;
        *Col = lpArg->Val.ValRange.Col1;
        *Row = lpArg->Val.ValRange.Row1;
        *Col2 = lpArg->Val.ValRange.Col2;
        *Row2 = lpArg->Val.ValRange.Row2;
      }
    }
  }
  UnlockSheet();
}
#endif
#endif

//--------------------------------------------------------------------

#ifndef SSF_NO_CFGetStringParam
BSTR CSsocxCtrl::CFGetStringParam(short Param) 
{
  LPSPREADSHEET lpSS = LockSheet();
  CString s;
  if( 1 <= Param && Param <= m_nCFArgCnt )
  {
    LPSS_VALUE lpArg = m_lpCFArgs + Param - 1;
    if( SS_VALUE_STATUS_OK == lpArg->Status )
    {
      if( SS_VALUE_TYPE_LONG == lpArg->Type )
      {
        TCHAR szBuffer[20];
        LongToString(lpArg->Val.ValLong, szBuffer);
        s = szBuffer;
      }
      else if( SS_VALUE_TYPE_STR == lpArg->Type )
      {
        s = (LPTSTR)GlobalLock(lpArg->Val.hValStr);
        GlobalUnlock(lpArg->Val.hValStr);
      }
    }
  }
  UnlockSheet();
  return s.AllocSysString();
}
#endif

//--------------------------------------------------------------------

#ifndef SSF_NO_CFSetResult
void CSsocxCtrl::CFSetResult(const VARIANT FAR& Var) 
{
  LPSPREADSHEET lpSS = LockSheet();
  if( m_lpCFResult )
  {
    if (SS_VALUE_STATUS_OK == m_lpCFResult->Status
        && SS_VALUE_TYPE_STR == m_lpCFResult->Type
        && m_lpCFResult->Val.hValStr)
      GlobalFree(m_lpCFResult->Val.hValStr);
    switch( V_VT(&Var) )
    {
      case VT_I2:
        m_lpCFResult->Type = SS_VALUE_TYPE_LONG;
        m_lpCFResult->Val.ValLong = V_I2(&Var);
        break;
      case VT_I4:
        m_lpCFResult->Type = SS_VALUE_TYPE_LONG;
        m_lpCFResult->Val.ValLong = V_I4(&Var);
        break;
      case VT_R4:
        m_lpCFResult->Type = SS_VALUE_TYPE_DOUBLE;
        m_lpCFResult->Val.ValDouble = V_R4(&Var);
        break;
      case VT_R8:
        m_lpCFResult->Type = SS_VALUE_TYPE_DOUBLE;
        m_lpCFResult->Val.ValDouble = V_R8(&Var);
        break;
      case VT_DATE:
        m_lpCFResult->Type = SS_VALUE_TYPE_DOUBLE;
        m_lpCFResult->Val.ValDouble = V_DATE(&Var);
        break;
      case VT_CY:
        {
        double dfVal;
        VarR8FromCy(V_CY(&Var), &dfVal);
        m_lpCFResult->Type = SS_VALUE_TYPE_DOUBLE;
        m_lpCFResult->Val.ValDouble = dfVal;
        }
        break;
      case VT_BSTR:
        {
          GLOBALHANDLE hBuffer;
          LPTSTR lpszBuffer;
        
          LPTSTR srcBuff = (LPTSTR)V_BSTR(&Var);
          BOOL   fFreeSrc = FALSE;
          // convert from WCHAR to CHAR if necessary
          if (sizeof(OLECHAR) != sizeof(TCHAR))
          {
            srcBuff = fpSysAllocTStringFromBSTR((LPVOID)srcBuff);
            fFreeSrc = TRUE;
          }  
        
          if( hBuffer = GlobalAlloc(GMEM_MOVEABLE,
                                    (lstrlen(srcBuff) + 1) * sizeof(TCHAR)) )
          {
            lpszBuffer = (LPTSTR)GlobalLock(hBuffer);
            lstrcpy(lpszBuffer, srcBuff);
            GlobalUnlock(hBuffer);
            m_lpCFResult->Type = SS_VALUE_TYPE_STR;
            m_lpCFResult->Val.hValStr = hBuffer;
          }
          if (fFreeSrc && srcBuff)
            fpSysFreeTString(srcBuff);
        }
        break;
      case VT_VARIANT | VT_BYREF:
        CFSetResult(*V_VARIANTREF(&Var));
        break;
      default:
        m_lpCFResult->Type = SS_VALUE_TYPE_LONG;
        m_lpCFResult->Val.ValLong = 0;
        break;
    }
  }
  UnlockSheet();
}
#endif

//--------------------------------------------------------------------

#ifndef SSF_NO_ColNumberToLetter
BSTR CSsocxCtrl::ColNumberToLetter(long HeaderNumber) 
{
  CString s;
  TCHAR szBuffer[20];
  SS_DrawFormatHeaderLetter(szBuffer, HeaderNumber);
  s = szBuffer;
  return s.AllocSysString();
}
#endif


//--------------------------------------------------------------------

#ifndef SSF_NO_ColWidthToTwips
void CSsocxCtrl::ColWidthToTwips(float Width, long FAR* Twips) 
{
  LPSPREADSHEET lpSS = LockSheet();
  long lPixels;
  HDC hDC;
  if( lpSS->lpBook->hWnd )
  {
    lPixels = SS_ColWidthToPixels(lpSS, Width);
    // RFW - 3/19/00 GIC11495
    /*
    if (lPixels > 0)
       lPixels -= lpSS->dBorderSize;
    */
    hDC = ::GetDC(lpSS->lpBook->hWnd);
    *Twips = (1440 * lPixels) / ::GetDeviceCaps(hDC, LOGPIXELSX);
    ::ReleaseDC(lpSS->lpBook->hWnd, hDC);
  }
  else
    *Twips = 0;
  UnlockSheet();
}
#endif

//--------------------------------------------------------------------

#ifndef SSF_NO_EnumCustomFunction
BOOL CSsocxCtrl::EnumCustomFunction(LPCTSTR PrevFuncName, BSTR FAR* FuncName)
{
  LPSS_BOOK lpBook = LockBook();
  TCHAR szFuncName[CALC_MAX_NAME_LEN];
  int iRet;

  iRet = SS_EnumCustomFunction(lpBook, PrevFuncName, szFuncName, CALC_MAX_NAME_LEN);
  SysFreeString(*FuncName);
  *FuncName = (BSTR)fpSysAllocBSTRFromTString(szFuncName);
  UnlockBook();
  return iRet > 0;
}
#endif

//--------------------------------------------------------------------

#ifndef SSF_NO_GetBottomRightCell
void CSsocxCtrl::GetBottomRightCell(long FAR* Col, long FAR* Row) 
{
  LPSPREADSHEET lpSS = LockSheet();
  SS_GetBottomRightCell(lpSS, Col, Row);
  SS_AdjustCellCoordsOut(lpSS, Col, Row);
  UnlockSheet();
}
#endif

//--------------------------------------------------------------------

#ifndef SSF_NO_GetCellDirtyFlag
BOOL CSsocxCtrl::GetCellDirtyFlag(long Col, long Row) 
{
  LPSPREADSHEET lpSS = LockSheet();
  BOOL bRet = FALSE;

  if (lpSS)
  {
    SS_AdjustCellCoords(lpSS, &Col, &Row);
    bRet = _SpreadGetCellDirtyFlag(lpSS->lpBook->hWnd, Col, Row);
  }
  UnlockSheet();

  return bRet;
}
#endif

//--------------------------------------------------------------------

#ifndef SSF_NO_GetCellFromScreenCoord
void CSsocxCtrl::GetCellFromScreenCoord(long FAR* Col, long FAR* Row, long x, long y) 
{
  LPSPREADSHEET lpSS = LockSheet();
  x = XTwipsToPixels(x);
  y = YTwipsToPixels(y);
  SS_GetCellFromPixel(lpSS, Col, Row, NULL, NULL, (int)x, (int)y);

  if( (Col && -2 == *Col) || (Row && -2 == *Row) )
  {
    if (Col)
      *Col = -1;
    if (Row)
      *Row = -1;
  }
  else
    SS_AdjustCellCoordsOut(lpSS, Col, Row);

  UnlockSheet();
}
#endif

//--------------------------------------------------------------------

#ifndef SSF_NO_GetCellPos
BOOL CSsocxCtrl::GetCellPos(long Col, long Row, long FAR* x, long FAR* y, long FAR* Width, long FAR* Height) 
{
  LPSPREADSHEET lpSS = LockSheet();
  RECT rect;
  BOOL bRet;

  SS_AdjustCellCoords(lpSS, &Col, &Row);
  bRet = SS_GetCellRect(lpSS, Col, Row, &rect);
  if( x )
    *x = XPixelsToTwips(rect.left);
  if( y )
    *y = YPixelsToTwips(rect.top);
  if( Width )
    *Width = XPixelsToTwips(rect.right - rect.left);
  if( Height )
    *Height = YPixelsToTwips(rect.bottom - rect.top);
  UnlockSheet();
  return bRet;
}
#endif

//--------------------------------------------------------------------

#ifndef SSF_NO_GetClientArea
void CSsocxCtrl::GetClientArea(long FAR* Width, long FAR* Height) 
{
  LPSS_BOOK lpBook = LockBook();
  RECT rect;
  SS_GetClientRect(lpBook, &rect);
  if( Width )
    *Width = XPixelsToTwips(rect.right - rect.left);
  if( Height )
    *Height = YPixelsToTwips(rect.bottom - rect.top);
  UnlockBook();
}
#endif

//--------------------------------------------------------------------

#ifndef SSF_NO_GetColItemData
long CSsocxCtrl::GetColItemData(long Col) 
{
  LPSPREADSHEET lpSS = LockSheet();
  long lValue;
  SS_AdjustCellCoords(lpSS, &Col, NULL);
  SS_GetColUserData(lpSS, Col, &lValue);
  UnlockSheet();
  return lValue;
}
#endif

//--------------------------------------------------------------------

#ifndef SSF_NO_GetCustomFunction
BOOL CSsocxCtrl::GetCustomFunction(LPCTSTR FuncName, short FAR* MinArgs, short FAR* MaxArgs, long FAR* Flags)
{
  LPSS_BOOK lpBook = LockBook();
  BOOL bRet;

  bRet = SS_GetCustomFunction(lpBook, FuncName, MinArgs, MaxArgs, Flags);
  UnlockBook();
  return bRet;
}
#endif

//--------------------------------------------------------------------

#ifndef SSF_NO_GetCustomName
BSTR CSsocxCtrl::GetCustomName(LPCTSTR Name) 
{
  LPSS_BOOK lpBook = LockBook();
  TCHAR szValue[CALC_MAX_EXPR_LEN];
  BSTR bstrValue;
  SS_GetCustomName(lpBook, Name, szValue, CALC_MAX_EXPR_LEN);
  bstrValue = (BSTR)fpSysAllocBSTRFromTString(szValue);
  UnlockBook();
  return bstrValue;
}
#endif

//--------------------------------------------------------------------

#ifndef SSF_NO_GetDataFillData
BOOL CSsocxCtrl::GetDataFillData(VARIANT FAR* Var, short VarType) 
{
  BOOL fRet =
    __SpreadGetDataFillData(m_hWnd, (LONG)this, Var, (LPSTR)NULL, (WORD) VarType);

  return fRet;
}
#endif

//--------------------------------------------------------------------

#ifndef SSF_NO_GetFirstValidCell
void CSsocxCtrl::GetFirstValidCell(long FAR* Col, long FAR* Row) 
{
  LPSPREADSHEET lpSS = LockSheet();
  SS_GetFirstValidCell(lpSS, Col, Row);
  SS_AdjustCellCoordsOut(lpSS, Col, Row);
  UnlockSheet();
}
#endif

//--------------------------------------------------------------------

#ifndef SSF_NO_GetItemData
long CSsocxCtrl::GetItemData() 
{
  LPSPREADSHEET lpSS = LockSheet();
  long lValue = SS_GetUserData(lpSS);
  UnlockSheet();
  return lValue;
}
#endif

//--------------------------------------------------------------------

#ifndef SSF_NO_GetIteration
BOOL CSsocxCtrl::GetIteration(short FAR* MaxIterations, double FAR* MaxChange) 
{
  LPSS_BOOK lpBook = LockBook();
  BOOL bIter = SS_GetIteration(lpBook, MaxIterations, MaxChange);
  UnlockBook();
  return bIter;
}
#endif

//--------------------------------------------------------------------

#ifndef SSF_NO_GetLastValidCell
void CSsocxCtrl::GetLastValidCell(long FAR* Col, long FAR* Row) 
{
  LPSPREADSHEET lpSS = LockSheet();
  SS_GetLastValidCell(lpSS, Col, Row);
  SS_AdjustCellCoordsOut(lpSS, Col, Row);
  UnlockSheet();
}
#endif

//--------------------------------------------------------------------

#ifndef SSF_NO_GetMultiSelItem
long CSsocxCtrl::GetMultiSelItem(long SelPrev) 
{
  LPSPREADSHEET lpSS = LockSheet();
  long lRet;
  SS_AdjustCellCoords(lpSS, NULL, &SelPrev);
  lRet = SS_SelModeSendMessage(lpSS, SS_SELMODE_GETSELITEM, SelPrev, 0, 0);
  SS_AdjustCellCoordsOut(lpSS, NULL, &lRet);
  UnlockSheet();
  return lRet;
}
#endif

//--------------------------------------------------------------------

#ifndef SSF_NO_GetRefStyle
#ifdef SS_V70
long CSsocxCtrl::GetRefStyle() 
{
  LPSS_BOOK lpBook = LockBook();
  long lValue = (long)SS_GetRefStyle(lpBook);
  UnlockBook();
  return lValue;
}
#else
short CSsocxCtrl::GetRefStyle() 
{
  LPSS_BOOK lpBook = LockBook();
  short nValue = SS_GetRefStyle(lpBook);
  UnlockBook();
  return nValue;
}
#endif
#endif

//--------------------------------------------------------------------

#ifndef SSF_NO_GetRowItemData
long CSsocxCtrl::GetRowItemData(long Row) 
{
  LPSPREADSHEET lpSS = LockSheet();
  long lValue;
  SS_AdjustCellCoords(lpSS, NULL, &Row);
  SS_GetRowUserData(lpSS, Row, &lValue);
  UnlockSheet();
  return lValue;
}
#endif

//--------------------------------------------------------------------

#ifndef SSF_NO_fpGetText
BOOL CSsocxCtrl::fpGetText(long Col, long Row, VARIANT FAR* Var) 
{
  LPSPREADSHEET lpSS = LockSheet();
  SS_CELLTYPE cellType;
  BOOL bRet = FALSE;
  VariantClear(Var);
  SS_AdjustCellCoords(lpSS, &Col, &Row);
  SS_RetrieveCellType(lpSS, &cellType, NULL, Col, Row);
  switch( cellType.Type )
  {
    case SS_TYPE_INTEGER:
      /* RFW - 7/25/06 - 19224
      if( lpSS->lpBook->hWnd )
      */
      {
        bRet = SS_GetInteger(lpSS, Col, Row, &V_I4(Var));
        if (bRet)
           V_VT(Var) = VT_I4;
      }
      break;
    case SS_TYPE_FLOAT:
#ifdef SS_V40
    case SS_TYPE_CURRENCY:
    case SS_TYPE_NUMBER:
    case SS_TYPE_PERCENT:
#endif // SS_V40
#ifdef SS_V70
    case SS_TYPE_SCIENTIFIC:
#endif // SS_V70
      /* RFW - 7/25/06 - 19224
      if( lpSS->lpBook->hWnd )
      */
      {
        bRet = SS_GetFloat(lpSS, Col, Row, &V_R8(Var));
        if (bRet)
           V_VT(Var) = VT_R8;
      }
      break;
#if SS_V80
    case SS_TYPE_CUSTOM:
       {
          long lSize;

          bRet = SS_GetBuffer(lpSS, Col, Row, NULL, &lSize);
          if( bRet )
          {
             SAFEARRAYBOUND bounds;

             bounds.cElements = lSize;
             bounds.lLbound = 0;
             Var->parray = SafeArrayCreate(VT_UI1, 1, &bounds);
             if( S_OK == SafeArrayLock(Var->parray) )
             {
                SS_GetBuffer(lpSS, Col, Row, Var->parray->pvData, &lSize);
                SafeArrayUnlock(Var->parray);
                V_VT(Var) = VT_ARRAY | VT_UI1;
                break;
             }
          }
       }
       // fall through
#endif
    default:
      CString s;
      TBGLOBALHANDLE hText;
      LPTSTR lpszText;
      if( lpSS->lpBook->EditModeOn && lpSS->Col.CurAt == Col && lpSS->Row.CurAt == Row )
        hText = SS_GetEditModeOnData(lpSS->lpBook->hWnd, lpSS, FALSE);
      else
        hText = SS_GetData(lpSS, &cellType, Col, Row, FALSE);
      if( hText )
      {
        lpszText = (LPTSTR)tbGlobalLock(hText);
        s = lpszText;
        tbGlobalUnlock(hText);
        if( lpSS->lpBook->EditModeOn && lpSS->Col.CurAt == Col && lpSS->Row.CurAt == Row )
          tbGlobalFree(hText);
        bRet = TRUE;
      }
      V_VT(Var) = VT_BSTR;

      V_BSTR(Var) = s.AllocSysString();
      // Convert BSTR to WCHARS (if  WIN32 & !Unicode & OLE2ANSI) SCP-12/95
#ifdef WIN32
      fpPatchVBVariant(Var);
#endif
      break;
  }
  UnlockSheet();
  return bRet;
}
#endif

//--------------------------------------------------------------------

#ifndef SSF_NO_IsCellSelected
BOOL CSsocxCtrl::IsCellSelected(long Col, long Row) 
{
  LPSPREADSHEET lpSS = LockSheet();
  SS_AdjustCellCoords(lpSS, &Col, &Row);
  BOOL bRet = SS_IsCellInSelection(lpSS, Col, Row);
  UnlockSheet();
  return bRet;
}
#endif

//--------------------------------------------------------------------

#ifndef SSF_NO_IsFormulaValid
BOOL CSsocxCtrl::IsFormulaValid(LPCTSTR Formula) 
{
  LPSPREADSHEET lpSS = LockSheet();
  BOOL bRet = SS_ValidateFormula(lpSS, Formula);
  UnlockSheet();
  return bRet;
}
#endif

//--------------------------------------------------------------------

#ifndef SSF_NO_IsVisible
BOOL CSsocxCtrl::IsVisible(long Col, long Row, BOOL Partial) 
{
  LPSPREADSHEET lpSS = LockSheet();
  BOOL bRet = FALSE;
  if( lpSS->lpBook->hWnd )
    bRet = SSGetVisible(
      lpSS->lpBook->hWnd, Col, Row, Partial ? SS_VISIBLE_PARTIAL : SS_VISIBLE_ALL
    );
  UnlockSheet();
  return bRet;
}
#endif

//--------------------------------------------------------------------

#ifndef SSF_NO_LoadFromFile
BOOL CSsocxCtrl::LoadFromFile(LPCTSTR FileName) 
{
  LPSS_BOOK lpBook = LockBook();
  BOOL bRet = FALSE;

  bRet = SS_LoadFromFile(lpBook, FileName);
  UnlockBook();
  ControlInfoChanged();

  return bRet;
}
#endif

//--------------------------------------------------------------------

#ifndef SSF_NO_LoadTabFile
BOOL CSsocxCtrl::LoadTabFile(LPCTSTR FileName) 
{
  BOOL bRet = FALSE;

  if( m_hWnd )
// GAB 03/27/02 - 9884 Start using SSLoadTextFile to load tab files now.
#if defined(SS_V40) || defined(SS_V35)
// GAB 03/27/02	- 9884 bRet = SSLoadTextFile(m_hWnd, FileName, _T(""), _T("\t"), _T("\r\n"), SS_LOADTEXT_NOHEADERS | SS_LOADTEXT_CLEARDATAONLY, NULL); 
		bRet = SSLoadTextFile(m_hWnd, FileName, _T(""), _T("\t"), _T("\r\n"), SS_LOADTEXT_TABFILE | SS_LOADTEXT_NOHEADERS, NULL); 
#else
    bRet = SSLoadFromFile(m_hWnd, FileName);
#endif

  return bRet;
}
#endif

//--------------------------------------------------------------------

#ifndef SSF_NO_QueryCustomName
BSTR CSsocxCtrl::QueryCustomName(LPCTSTR PrevName) 
{
  LPSS_BOOK lpBook = LockBook();
  TCHAR szName[CALC_MAX_NAME_LEN];
  BSTR bstrName;
  SS_QueryCustomName(lpBook, PrevName, szName, CALC_MAX_NAME_LEN);
  bstrName = (BSTR)fpSysAllocBSTRFromTString(szName);
  UnlockBook();
  return bstrName;
}
#endif

//--------------------------------------------------------------------

#ifndef SSF_NO_ReCalcCell
BOOL CSsocxCtrl::ReCalcCell(long Col, long Row)
{
  LPSPREADSHEET lpSS = LockSheet();
  BOOL bRet;

  SS_AdjustCellCoords(lpSS, &Col, &Row);
  bRet = SS_ReCalcCell(lpSS, Col, Row);
  UnlockSheet();
  return bRet;
}
#endif

//--------------------------------------------------------------------

#ifndef SSF_NO_RemoveCustomFunction
BOOL CSsocxCtrl::RemoveCustomFunction(LPCTSTR FuncName)
{
  LPSS_BOOK lpBook = LockBook();
  BOOL bRet;

  bRet = SS_RemoveCustomFunction(lpBook, FuncName);
  UnlockBook();
  return bRet;
}
#endif

//--------------------------------------------------------------------

#ifndef SSF_NO_RowHeightToTwips
void CSsocxCtrl::RowHeightToTwips(long Row, float Height, long FAR* Twips) 
{
  LPSPREADSHEET lpSS = LockSheet();
  long lPixels;
  HDC hDC;
  if( lpSS->lpBook->hWnd )
  {
    SS_AdjustCellCoords(lpSS, NULL, &Row);
    lPixels = SS_RowHeightToPixels(lpSS, Row, Height);
    hDC = ::GetDC(lpSS->lpBook->hWnd);
    *Twips = (1440 * lPixels) / ::GetDeviceCaps(hDC, LOGPIXELSY);
    ::ReleaseDC(lpSS->lpBook->hWnd, hDC);
  }
  else
    *Twips = 0;
  UnlockSheet();
}
#endif

//--------------------------------------------------------------------

#define SS_MAXPATH 1000 // RFW - 5/16/07 - 20152

#ifndef SSF_NO_SaveTabFile
BOOL CSsocxCtrl::SaveTabFile(LPCTSTR FileName) 
{
  LPSPREADSHEET lpSS = LockSheet();
  BOOL bRet = FALSE;
  if (!m_fSafeForScripting)
  {
	 //Modify by BOC 99.6.21 (hyt)----------------------
	 //if the user doesn't include the extension 
	 //will not add the default extension automatically.
	TCHAR tDrive[_MAX_DRIVE], 
			tDir[_MAX_DIR], 
			tFileName[_MAX_FNAME], 
			tExt[_MAX_EXT],
			tPath[SS_MAXPATH];
	
	tDrive[0] = tDir[0] = tFileName[0] = tExt[0] = 0;
	memset(tPath,0,sizeof(tPath));
	_tsplitpath (FileName, tDrive, tDir, tFileName, tExt);
	memcpy(tPath,FileName,min(SS_MAXPATH - 1, lstrlen(FileName)) * sizeof(TCHAR));

	if (tExt[0] == 0 && lstrlen(tPath) < (SS_MAXPATH - 4))
#ifdef SS_V70
		_tcscat(tPath, _T(".tb7"));
#elif defined SS_V40
		_tcscat(tPath, _T(".tb6"));
#else
		_tcscat(tPath, _T(".tb3"));
#endif
		// jb - 28.jul.01 -----------------------
		// Support for new tabfilesave.
#ifdef SS_V35
// RFW - 8/20/01 - Removed unformatted flag
//    bRet = SS_ExportRangeToTextFile (lpSS, SS_ALLCOLS, SS_ALLROWS, 0, 0, tPath, 0, _T("\t"), _T("\r\n"), 4, NULL);
//GAB 01/10/02 This is the ANSI version of SaveTabFile so the Unicode Flag (last parameter)
// will be set to FALSE    
//    bRet = SS_ExportRangeToTextFile (lpSS, SS_ALLCOLS, SS_ALLROWS, 0, 0, tPath, 0, _T("\t"), _T("\r\n"), 0, NULL);
// GAB 03/27/02 - 9884 Added new flag so ExportRangeToTextFile would not save cell delimiters in a tab file
//    bRet = SS_ExportRangeToTextFile (lpSS, SS_ALLCOLS, SS_ALLROWS, 0, 0, tPath, 0, _T("\t"), _T("\r\n"), 0, NULL, FALSE);
    bRet = SS_ExportRangeToTextFile (lpSS, SS_ALLCOLS, SS_ALLROWS, 0, 0, tPath, 0, _T("\t"), _T("\r\n"), SS_EXPORTTEXT_TABFILE, NULL, FALSE);
#else
//GAB 01/10/02 Added new fUnicode parameter and set it to FALSE. This parameter is not used in
//versions of Spread before 3.5
	bRet = SS_SaveTabFile(lpSS, tPath, FALSE);
#endif
	 //--------------------------------------------------
  }
  UnlockSheet();
  return bRet;
}
#endif

#ifdef SS_V40
#ifndef SSF_NO_SaveTabFileU
BOOL CSsocxCtrl::SaveTabFileU(LPCTSTR FileName) 
{
  LPSPREADSHEET lpSS = LockSheet();
  BOOL bRet = FALSE;
  if (!m_fSafeForScripting)
  {
	 //Modify by BOC 99.6.21 (hyt)----------------------
	 //if the user doesn't include the extension 
	 //will not add the default extension automatically.
	TCHAR tDrive[_MAX_DRIVE], 
			tDir[_MAX_DIR], 
			tFileName[_MAX_FNAME], 
			tExt[_MAX_EXT],
			tPath[SS_MAXPATH];
	
	tDrive[0] = tDir[0] = tFileName[0] = tExt[0] = 0;
	memset(tPath,0,sizeof(tPath));
	_tsplitpath (FileName, tDrive, tDir, tFileName, tExt);
	memcpy(tPath,FileName,min(SS_MAXPATH - 1, lstrlen(FileName)) * sizeof(TCHAR));


	if (tExt[0] == 0 && lstrlen(tPath) < (SS_MAXPATH - 4))
#ifdef SS_V70
		_tcscat(tPath, _T(".tb7"));
#elif defined SS_V40
		_tcscat(tPath, _T(".tb6"));
#else
		_tcscat(tPath, _T(".tb3"));
#endif

  bRet = SS_ExportRangeToTextFile (lpSS, SS_ALLCOLS, SS_ALLROWS, 0, 0, tPath, 0, _T("\t"), _T("\r\n"), SS_EXPORTTEXT_TABFILE, NULL, TRUE);
  }
  UnlockSheet();
  return bRet;
}
#endif
#endif //SS_V40

//--------------------------------------------------------------------

#ifndef SSF_NO_SaveToFile
BOOL CSsocxCtrl::SaveToFile(LPCTSTR FileName, BOOL DataOnly) 
{
  BOOL bRet = FALSE;

	if (!m_fSafeForScripting)
	{
      LPSS_BOOK lpBook = LockBook();
		//Modify by BOC 99.6.21 (hyt)----------------------
		//if the user doesn't include the extension 
		//will not add the default extension automatically.
		TCHAR tDrive[_MAX_DRIVE], 
			  tDir[_MAX_DIR], 
			  tFileName[_MAX_FNAME], 
			  tExt[_MAX_EXT],
			  tPath[_MAX_PATH];

		tDrive[0] = tDir[0] = tFileName[0] = tExt[0] = 0;
		memset(tPath,0,_MAX_PATH);
		_tsplitpath (FileName, tDrive, tDir, tFileName, tExt);
		memcpy(tPath,FileName,lstrlen(FileName)*sizeof(TCHAR));
		if (tExt[0] == 0)
#ifdef SS_V80
			_tcscat(tPath, _T(".ss8"));
#elif defined SS_V70
			_tcscat(tPath, _T(".ss7"));
#elif defined SS_V40
			_tcscat(tPath, _T(".ss6"));
#else
			_tcscat(tPath, _T(".ss3"));	
#endif // SS_V40
      //bRet = SSSaveToFile(m_hWnd, FileName, DataOnly);
		bRet = SS_SaveToFile(lpBook, tPath, DataOnly);
      UnlockBook();
	//----------------------------------------------------
	}

  return bRet;
}
#endif

//--------------------------------------------------------------------

#ifndef SSF_NO_SetColItemData
void CSsocxCtrl::SetColItemData(long Col, long Value) 
{
  LPSPREADSHEET lpSS = LockSheet();
  SS_AdjustCellCoords(lpSS, &Col, NULL);
  SS_SetColUserData(lpSS, Col, Value);
  UnlockSheet();
}
#endif

//--------------------------------------------------------------------

#ifndef SSF_NO_SetCustomName
BOOL CSsocxCtrl::SetCustomName(LPCTSTR Name, LPCTSTR Value) 
{
  LPSS_BOOK lpBook = LockBook();
  BOOL bRet = SS_SetCustomName(lpBook, Name, Value);
  UnlockBook();
  return bRet;
}
#endif

//--------------------------------------------------------------------

#ifndef SSF_NO_SetDataFillData
BOOL CSsocxCtrl::SetDataFillData(const VARIANT FAR& Var) 
{
  LPVBSPREAD lpSpread;
  BOOL       fRet = FALSE;
  VARIANTARG varData1;
  HRESULT    h = S_OK;
  HLSTR      hlstr;
  LPTSTR     tstrTmp = NULL;
  BOOL       fFreeTmp = FALSE;

  lpSpread = (LPVBSPREAD)SSOcxLockVBSpread((LONG)this);

  if (lpSpread->dDataSetFillCol != -1)
  {
	 if( (Var.vt == (VT_VARIANT|VT_BYREF) && Var.pvarVal->vt == VT_EMPTY) )
		fRet = __SpreadSetDataFillData(m_hWnd, (LONG)this, lpSpread, NULL, 0, FALSE);
    /*
	 else if( Var.vt & (VT_I1|VT_I2|VT_I4) ||
             (Var.vt == (VT_VARIANT|VT_BYREF) && Var.pvarVal->vt & (VT_I1|VT_I2|VT_I4) ))
    {
		VariantInit(&varData1);
		h = VariantChangeType(&varData1, (VARIANT FAR*)&Var, (USHORT)0, VT_I4);
		fRet = __SpreadSetDataFillData(m_hWnd, (LONG)this, lpSpread, NULL, varData1.lVal, TRUE);
    }
    */
    else
	 {
		VariantInit(&varData1);
		h = VariantChangeType(&varData1, (VARIANT FAR*)&Var, (USHORT)0, VT_BSTR);
		ASSERT(h == S_OK);         // Could not convert to BSTR!
		if (h == S_OK)
		{  
			if (sizeof(TCHAR) != sizeof(OLECHAR))
			{
			  tstrTmp = fpSysAllocTStringFromBSTR(varData1.bstrVal);
			  fFreeTmp = TRUE;
			}
			else 
			  tstrTmp = (LPTSTR)varData1.bstrVal;


			hlstr = ssVBCreateHlstr( tstrTmp, fpSysTStringByteLen(tstrTmp));
			SysFreeString(varData1.bstrVal);
			//NOTE: The function frees the hlstr.
			fRet = __SpreadSetDataFillData(m_hWnd, (LONG)this, lpSpread, hlstr, 0, FALSE);
		}
	 }


     // If fFreeTmp was set, free tstrTmp
     if (fFreeTmp && tstrTmp)
       fpSysFreeTString(tstrTmp);
  }

  SSOcxUnlockVBSpread((LONG)this);

  return fRet;
}
#endif

//--------------------------------------------------------------------

#ifndef SSF_NO_SetCellDirtyFlag
BOOL CSsocxCtrl::SetCellDirtyFlag(long Col, long Row, BOOL Dirty) 
{
  LPSPREADSHEET lpSS = LockSheet();
  BOOL bRet = FALSE;

  SSOcxLockVBSpread((LONG)this);
  if (lpSS)
  {
    SS_AdjustCellCoords(lpSS, &Col, &Row);
     bRet = 
       __SpreadSetCellDirtyFlag((LONG)this, lpSS->lpBook->hWnd, Col, Row, Dirty);
  }
  SSOcxUnlockVBSpread((LONG)this);
  UnlockSheet();

  return bRet;
}
#endif

//--------------------------------------------------------------------

#ifndef SSF_NO_SetItemData
void CSsocxCtrl::SetItemData(long Value) 
{
  LPSPREADSHEET lpSS = LockSheet();
  SS_SetUserData(lpSS, Value);
  UnlockSheet();
}
#endif

//--------------------------------------------------------------------

#ifndef SSF_NO_SetIteration
void CSsocxCtrl::SetIteration(BOOL Iteration, short MaxIterations, double MaxChange) 
{
  LPSS_BOOK lpBook = LockBook();
  SS_SetIteration(lpBook, Iteration, MaxIterations, MaxChange);
  UnlockBook();
}
#endif

//--------------------------------------------------------------------

#ifndef SSF_NO_SetRefStyle
#ifdef SS_V70
void CSsocxCtrl::SetRefStyle(long RefStyle) 
{
  LPSS_BOOK lpBook = LockBook();
  SS_SetRefStyle(lpBook, (short)RefStyle);
  UnlockBook();
}
#else
void CSsocxCtrl::SetRefStyle(short RefStyle) 
{
  LPSS_BOOK lpBook = LockBook();
  SS_SetRefStyle(lpBook, RefStyle);
  UnlockBook();
}
#endif
#endif

//--------------------------------------------------------------------
#ifndef SSF_NO_SetRowItemData
void CSsocxCtrl::SetRowItemData(long Row, long Value) 
{
  LPSPREADSHEET lpSS = LockSheet();
  SS_AdjustCellCoords(lpSS, NULL, &Row);
  SS_SetRowUserData(lpSS, Row, Value);
  UnlockSheet();
}
#endif

//--------------------------------------------------------------------

#ifndef SSF_NO_fpSetText
void CSsocxCtrl::fpSetText(long Col, long Row, const VARIANT FAR& Var)
{
  LPSPREADSHEET lpSS = LockSheet();
  long lVal;
  double dfVal;
  long ColOrig = Col;
  long RowOrig = Row;
  unsigned short vtype = V_VT(&Var); // BJO GRB5542
  SS_AdjustCellCoords(lpSS, &Col, &Row);
  switch( V_VT(&Var) )
  {
    #if defined(WIN32)
    case VT_UI1:
      lVal = V_UI1(&Var);
      SS_SetDataRange(lpSS, Col, Row, Col, Row, (LPTSTR)&lVal, SS_VALUE_INT, FALSE, TRUE);
      break;
    case VT_UI1 | VT_BYREF:
      lVal = *V_UI1REF(&Var);
      SS_SetDataRange(lpSS, Col, Row, Col, Row, (LPTSTR)&lVal, SS_VALUE_INT, FALSE, TRUE);
      break;
    #endif
    case VT_I2:
      lVal = V_I2(&Var);
      SS_SetDataRange(lpSS, Col, Row, Col, Row, (LPTSTR)&lVal, SS_VALUE_INT, FALSE, TRUE);
      break;
    case VT_I2 | VT_BYREF:
      lVal = *V_I2REF(&Var);
      SS_SetDataRange(lpSS, Col, Row, Col, Row, (LPTSTR)&lVal, SS_VALUE_INT, FALSE, TRUE);
      break;
    case VT_I4:
      lVal = V_I4(&Var);
      SS_SetDataRange(lpSS, Col, Row, Col, Row, (LPTSTR)&lVal, SS_VALUE_INT, FALSE, TRUE);
      break;
    case VT_I4 | VT_BYREF:
      lVal = *V_I4REF(&Var);
      SS_SetDataRange(lpSS, Col, Row, Col, Row, (LPTSTR)&lVal, SS_VALUE_INT, FALSE, TRUE);
      break;
    case VT_R4:
      dfVal = V_R4(&Var);
      SS_SetDataRange(lpSS, Col, Row, Col, Row, (LPTSTR)&dfVal, SS_VALUE_FLOAT, FALSE, TRUE);
      break;
    case VT_R4 | VT_BYREF:
      dfVal = *V_R4REF(&Var);
      SS_SetDataRange(lpSS, Col, Row, Col, Row, (LPTSTR)&dfVal, SS_VALUE_FLOAT, FALSE, TRUE);
      break;
    case VT_R8:
      dfVal = V_R8(&Var);
      SS_SetDataRange(lpSS, Col, Row, Col, Row, (LPTSTR)&dfVal, SS_VALUE_FLOAT, FALSE, TRUE);
      break;
    // RFW - 9/26/07 - 21204
    case VT_R8 | VT_BYREF:
      dfVal = *V_R8REF(&Var);
      SS_SetDataRange(lpSS, Col, Row, Col, Row, (LPTSTR)&dfVal, SS_VALUE_FLOAT, FALSE, TRUE);
      break;
    case VT_CY:
      VarR8FromCy(V_CY(&Var), &dfVal);
      SS_SetDataRange(lpSS, Col, Row, Col, Row, (LPTSTR)&dfVal, SS_VALUE_FLOAT, FALSE, TRUE);
      break;
    case VT_CY | VT_BYREF:
      VarR8FromCy(*V_CYREF(&Var), &dfVal);
      SS_SetDataRange(lpSS, Col, Row, Col, Row, (LPTSTR)&dfVal, SS_VALUE_FLOAT, FALSE, TRUE);
      break;
    case VT_DATE | VT_BYREF:
    case VT_DATE:
		{
		SS_CELLTYPE CellType;

		SS_RetrieveCellType(lpSS, &CellType, NULL, Col, Row);

		if (CellType.Type == SS_TYPE_DATE)
			{
         TB_DATE Date;
			TCHAR sz[20];
			double dfVal;

         if (V_VT(&Var) & VT_BYREF)
			   dfVal = *V_DATEREF(&Var);
         else
			   dfVal = V_DATE(&Var);

         vbDateJulianToDMY((long)dfVal - 2, &Date);
         DateToString(sz, &Date);
	      SS_SetValue(lpSS, Col, Row, sz);
			}

		else
			{
			LPTSTR tstrTmp;
			BOOL   fFreeTmp = FALSE;
			VARIANT v;

			VariantInit(&v);
			VariantChangeType(&v, (LPVARIANT)&Var, 0, VT_BSTR);

			tstrTmp = (LPTSTR)V_BSTR(&v);

			// if necessary, convert from WCHAR to CHAR
			if (sizeof(OLECHAR) != sizeof(TCHAR))
				{
				tstrTmp = fpSysAllocTStringFromBSTR((LPVOID)tstrTmp);
				fFreeTmp = TRUE;
				}

			SS_SetDataRange(lpSS, Col, Row, Col, Row, tstrTmp, FALSE, FALSE, TRUE);

			if (fFreeTmp && tstrTmp)
			fpSysFreeTString(tstrTmp);
			}
		}
		break;

    case VT_BSTR:
      {
        LPTSTR tstrTmp = (LPTSTR)V_BSTR(&Var);
        BOOL   fFreeTmp = FALSE;
        // if necessary, convert from WCHAR to CHAR
        if (sizeof(OLECHAR) != sizeof(TCHAR))
        {
          tstrTmp = fpSysAllocTStringFromBSTR((LPVOID)tstrTmp);
          fFreeTmp = TRUE;
        }  

       SS_SetDataRange(lpSS, Col, Row, Col, Row, tstrTmp, FALSE, FALSE, TRUE);

        if (fFreeTmp && tstrTmp)
          fpSysFreeTString(tstrTmp);
      }
      break;
    case VT_VARIANT | VT_BYREF:
      fpSetText(ColOrig, RowOrig, *V_VARIANTREF(&Var));
      break;
    case VT_DISPATCH:
      {
        VARIANT vTemp;
        VariantInit(&vTemp);
        if( SUCCEEDED(VariantChangeType(&vTemp, (LPVARIANT)&Var, 0, VT_BSTR)) )
          fpSetText(ColOrig, RowOrig, vTemp);
        else
          SS_SetDataRange(lpSS, Col, Row, Col, Row, NULL, FALSE, FALSE, TRUE);
        VariantClear(&vTemp);
      }
      break;
#if SS_V80
    case VT_ARRAY|VT_UI1:
       if( Var.parray->cDims == 1 && (S_OK == SafeArrayLock(Var.parray)) )
       {
          SS_SetBuffer(lpSS, Col, Row, Var.parray->pvData, Var.parray->rgsabound[0].cElements);
          SafeArrayUnlock(Var.parray);
       }
       break;
#endif
    default:
      SS_SetDataRange(lpSS, Col, Row, Col, Row, NULL, FALSE, FALSE, TRUE);
      break;
  }
  if( lpSS->Col.HeaderCnt > Col || lpSS->Row.HeaderCnt > Row )
    ControlInfoChanged();
  UnlockSheet();
}
#endif

//--------------------------------------------------------------------

#ifndef SSF_NO_TwipsToColWidth
void CSsocxCtrl::TwipsToColWidth(long Twips, float FAR* ColWidth) 
{
  LPSPREADSHEET lpSS = LockSheet();
  long lPixels;
  double dfColWidth = 0.0;
  HDC hDC;
  if( lpSS->lpBook->hWnd )
  {
    hDC = ::GetDC(lpSS->lpBook->hWnd);
    lPixels = (Twips * ::GetDeviceCaps(hDC, LOGPIXELSX)) / 1440;
    ::ReleaseDC(lpSS->lpBook->hWnd, hDC);
    SS_CalcPixelsToColWidth(lpSS, 0, (int)lPixels, &dfColWidth);
  }
  *ColWidth = (float)dfColWidth;
  UnlockSheet();
}
#endif

//--------------------------------------------------------------------

#ifndef SSF_NO_TwipsToRowHeight
void CSsocxCtrl::TwipsToRowHeight(long Row, long Twips, float FAR* RowHeight) 
{
  LPSPREADSHEET lpSS = LockSheet();
  long lPixels;
  double dfRowHeight = 0.0;
  HDC hDC;
  if( lpSS->lpBook->hWnd )
  {
    hDC = ::GetDC(lpSS->lpBook->hWnd);
    lPixels = (Twips * ::GetDeviceCaps(hDC, LOGPIXELSY)) / 1440;
    ::ReleaseDC(lpSS->lpBook->hWnd, hDC);
    SS_AdjustCellCoords(lpSS, NULL, &Row);
    SS_CalcPixelsToRowHeight(lpSS, Row, (int)lPixels, &dfRowHeight);
  }
  *RowHeight = (float)dfRowHeight;
  UnlockSheet();
}
#endif


//void CSsocxCtrl::fpContainerCoordsToPixels(long FAR *lpxPixels, long FAR *lpyPixels,
//                                           long x, long y, BOOL fSize)
//{
//POINTL PointL;
//POINTF PointF;
//
//PointF.x = (float)x;
//PointF.y = (float)y;
//
//TransformCoords(&PointL, &PointF, XFORMCOORDS_CONTAINERTOHIMETRIC |
//                (fSize ? XFORMCOORDS_SIZE : XFORMCOORDS_POSITION));
//
//if (lpxPixels)
//   *lpxPixels = XHiMetricToPixels(PointL.x);
//
//if (lpyPixels)
//   *lpyPixels = YHiMetricToPixels(PointL.y);
//}


//void CSsocxCtrl::fpPixelsToContainerCoords(long FAR *lpxCoord, long FAR *lpyCoord,
//                                           long x, long y, BOOL fSize)
//{
//POINTL PointL;
//POINTF PointF;
//
//PointL.x = XPixelsToHiMetric(x);
//PointL.y = YPixelsToHiMetric(y);
//
//TransformCoords(&PointL, &PointF, XFORMCOORDS_HIMETRICTOCONTAINER |
//                (fSize ? XFORMCOORDS_SIZE : XFORMCOORDS_POSITION));
//
//if (lpxCoord)
//   *lpxCoord = (long)PointF.x;
//
//if (lpyCoord)
//   *lpyCoord = (long)PointF.y;
//}


//--------------------------------------------------------------------

#ifndef SSF_NO_DdeQueryLink
BOOL CSsocxCtrl::DdeQueryLink(LPCTSTR ServerPrev, LPCTSTR TopicPrev, LPCTSTR ItemPrev, BSTR FAR* Server, BSTR FAR* Topic, BSTR FAR* Item) 
{
  LPSPREADSHEET lpSS = LockSheet();
  CString sServer, sTopic, sItem;
  LPTSTR lpszServer, lpszTopic, lpszItem;
  int nServerLen, nTopicLen, nItemLen;
  BOOL bRet = FALSE;

  if( SS_DdeQueryLinkLen(lpSS, ServerPrev, TopicPrev, ItemPrev, &nServerLen, &nTopicLen, &nItemLen) )
  {
    lpszServer = sServer.GetBuffer(nServerLen+1);
    lpszTopic = sTopic.GetBuffer(nTopicLen+1);
    lpszItem = sItem.GetBuffer(nItemLen+1);
    bRet = SS_DdeQueryLink(lpSS, ServerPrev, TopicPrev, ItemPrev, lpszServer, lpszTopic, lpszItem, nServerLen+1, nTopicLen+1, nItemLen+1);
    sServer.ReleaseBuffer();
    sTopic.ReleaseBuffer();
    sItem.ReleaseBuffer();
    if( bRet )
    {
      SysFreeString(*Server);
      SysFreeString(*Topic);
      SysFreeString(*Item);
      *Server = sServer.AllocSysString();
      *Topic = sTopic.AllocSysString();
      *Item = sItem.AllocSysString();
    }
  }
  UnlockSheet();
  return bRet;
}
#endif

//--------------------------------------------------------------------

#ifndef SSF_NO_DdeAddLink
BOOL CSsocxCtrl::DdeAddLink(LPCTSTR Server, LPCTSTR Topic, LPCTSTR Item) 
{
  LPSPREADSHEET lpSS = LockSheet();
  BOOL bRet = SS_DdeAddLink(lpSS, Server, Topic, Item);
  UnlockSheet();
  return bRet;
}
#endif

//--------------------------------------------------------------------

#ifndef SSF_NO_DdeRemoveLink
BOOL CSsocxCtrl::DdeRemoveLink(LPCTSTR Server, LPCTSTR Topic, LPCTSTR Item) 
{
  LPSPREADSHEET lpSS = LockSheet();
  BOOL bRet = SS_DdeRemoveLink(lpSS, Server, Topic, Item);
  UnlockSheet();
  return bRet;
}
#endif

//--------------------------------------------------------------------

#ifndef SSF_NO_DdeRequestLink
BOOL CSsocxCtrl::DdeRequestLink(LPCTSTR Server, LPCTSTR Topic, LPCTSTR Item) 
{
  LPSPREADSHEET lpSS = LockSheet();
  BOOL bRet = SS_DdeRequestLink(lpSS, Server, Topic, Item);
  UnlockSheet();
  return bRet;
}
#endif

//--------------------------------------------------------------------

#ifndef SSF_NO_DdePokeLink
BOOL CSsocxCtrl::DdePokeLink(LPCTSTR Server, LPCTSTR Topic, LPCTSTR Item) 
{
  LPSPREADSHEET lpSS = LockSheet();
  BOOL bRet = SS_DdePokeLink(lpSS, Server, Topic, Item);
  UnlockSheet();
  return bRet;
}
#endif

//--------------------------------------------------------------------

#ifndef SSF_NO_DdePokeDataToLink
BOOL CSsocxCtrl::DdePokeDataToLink(long Col, long Row) 
{
  LPSPREADSHEET lpSS = LockSheet();
  BOOL bRet = SS_DdePokeDataToLink(lpSS, Col, Row);
  UnlockSheet();
  return bRet;
}
#endif

//--------------------------------------------------------------------

#ifndef SSF_NO_DdeGetMode
short CSsocxCtrl::DdeGetMode(LPCTSTR Server, LPCTSTR Topic, LPCTSTR Item) 
{
  LPSPREADSHEET lpSS = LockSheet();
  short nRet = SS_DdeGetMode(lpSS, Server, Topic, Item);
  UnlockSheet();
  return nRet;
}
#endif

//--------------------------------------------------------------------

#ifndef SSF_NO_DdeSetMode
short CSsocxCtrl::DdeSetMode(LPCTSTR Server, LPCTSTR Topic, LPCTSTR Item, short Mode) 
{
  LPSPREADSHEET lpSS = LockSheet();
  short nRet = SS_DdeSetMode(lpSS, Server, Topic, Item, Mode);
  UnlockSheet();
  return nRet;
}
#endif

//--------------------------------------------------------------------

#ifndef SSF_NO_DdeGetReadExpr
BSTR CSsocxCtrl::DdeGetReadExpr(LPCTSTR Server, LPCTSTR Topic, LPCTSTR Item) 
{
  LPSPREADSHEET lpSS = LockSheet();
  CString strResult;
  LPTSTR lpszExpr;
  int nExprLen;

  if( nExprLen = SS_DdeGetReadExprLen(lpSS, Server, Topic, Item) )
  {
    lpszExpr = strResult.GetBuffer(nExprLen+1);
    SS_DdeGetReadExpr(lpSS, Server, Topic, Item, lpszExpr, nExprLen+1);
    strResult.ReleaseBuffer();
  }
  UnlockSheet();
  return strResult.AllocSysString();
}
#endif

//--------------------------------------------------------------------

#ifndef SSF_NO_DdeSetReadExpr
BOOL CSsocxCtrl::DdeSetReadExpr(LPCTSTR Server, LPCTSTR Topic, LPCTSTR Item, LPCTSTR Expr) 
{
  LPSPREADSHEET lpSS = LockSheet();
  BOOL bRet = SS_DdeSetReadExpr(lpSS, Server, Topic, Item, Expr);
  UnlockSheet();
  return bRet;
}
#endif

//--------------------------------------------------------------------

#ifndef SSF_NO_DdeGetWriteExpr
BSTR CSsocxCtrl::DdeGetWriteExpr(LPCTSTR Server, LPCTSTR Topic, LPCTSTR Item)
{
  LPSPREADSHEET lpSS = LockSheet();
  CString strResult;
  LPTSTR lpszExpr;
  int nExprLen;

  if( nExprLen = SS_DdeGetWriteExprLen(lpSS, Server, Topic, Item) )
  {
    lpszExpr = strResult.GetBuffer(nExprLen+1);
    SS_DdeGetWriteExpr(lpSS, Server, Topic, Item, lpszExpr, nExprLen+1);
    strResult.ReleaseBuffer();
  }
  UnlockSheet();
  return strResult.AllocSysString();
}
#endif

//--------------------------------------------------------------------

#ifndef SSF_NO_DdeSetWriteExpr
BOOL CSsocxCtrl::DdeSetWriteExpr(LPCTSTR Server, LPCTSTR Topic, LPCTSTR Item, LPCTSTR Expr) 
{
  LPSPREADSHEET lpSS = LockSheet();
  BOOL bRet = SS_DdeSetWriteExpr(lpSS, Server, Topic, Item, Expr);
  UnlockSheet();
  return bRet;
}
#endif

//--------------------------------------------------------------------

#ifndef SSF_NO_DdeGetLink
BOOL CSsocxCtrl::DdeGetLink(long Col, long Row, BSTR FAR* Server, BSTR FAR* Topic, BSTR FAR* Item, long FAR* ItemCol, long FAR* ItemRow) 
{
  LPSPREADSHEET lpSS = LockSheet();
  CString sServer, sTopic, sItem;
  LPTSTR lpszServer, lpszTopic, lpszItem;
  int nServerLen, nTopicLen, nItemLen;
  BOOL bRet = FALSE;

  if( SS_DdeGetLinkLen(lpSS, Col, Row, &nServerLen, &nTopicLen, &nItemLen) )
  {
    lpszServer = sServer.GetBuffer(nServerLen+1);
    lpszTopic = sTopic.GetBuffer(nTopicLen+1);
    lpszItem = sItem.GetBuffer(nItemLen+1);
    bRet = SS_DdeGetLink(lpSS, Col, Row, lpszServer, lpszTopic, lpszItem, ItemCol, ItemRow, nServerLen+1, nTopicLen+1, nItemLen+1);
    sServer.ReleaseBuffer();
    sTopic.ReleaseBuffer();
    sItem.ReleaseBuffer();
    if( bRet )
    {
      SysFreeString(*Server);
      SysFreeString(*Topic);
      SysFreeString(*Item);
      *Server = sServer.AllocSysString();
      *Topic = sTopic.AllocSysString();
      *Item = sItem.AllocSysString();
    }
  }
  UnlockSheet();
  return bRet;
}
#endif

//--------------------------------------------------------------------

#ifndef SSF_NO_DdeSetLink
BOOL CSsocxCtrl::DdeSetLink(long Col, long Row, LPCTSTR Server, LPCTSTR Topic, LPCTSTR Item, long ItemCol, long ItemRow) 
{
  LPSPREADSHEET lpSS = LockSheet();
  BOOL bRet = SS_DdeSetLink(lpSS, Col, Row, Server, Topic, Item, ItemCol, ItemRow);
  UnlockSheet();
  return bRet;
}
#endif

//--------------------------------------------------------------------

#ifndef SSF_NO_DdeQueryDependCell
BOOL CSsocxCtrl::DdeQueryDependCell(LPCTSTR Server, LPCTSTR Topic, LPCTSTR Item, long ColPrev, long RowPrev, long FAR* Col, long FAR* Row)
{
  LPSPREADSHEET lpSS = LockSheet();
  BOOL bRet = SS_DdeQueryDependCell(lpSS, Server, Topic, Item, ColPrev, RowPrev, Col, Row);
  UnlockSheet();
  return bRet;
}
#endif

//--------------------------------------------------------------------

#ifndef SSF_NO_DdeQueryDependCol
BOOL CSsocxCtrl::DdeQueryDependCol(LPCTSTR Server, LPCTSTR Topic, LPCTSTR Item, long ColPrev, long FAR* Col)
{
  LPSPREADSHEET lpSS = LockSheet();
  BOOL bRet = SS_DdeQueryDependCol(lpSS, Server, Topic, Item, ColPrev, Col);
  UnlockSheet();
  return bRet;
}
#endif

//--------------------------------------------------------------------

#ifndef SSF_NO_DdeQueryDependRow
BOOL CSsocxCtrl::DdeQueryDependRow(LPCTSTR Server, LPCTSTR Topic, LPCTSTR Item, long RowPrev, long FAR* Row)
{
  LPSPREADSHEET lpSS = LockSheet();
  BOOL bRet = SS_DdeQueryDependRow(lpSS, Server, Topic, Item, RowPrev, Row);
  UnlockSheet();
  return bRet;
}
#endif

//--------------------------------------------------------------------

#ifndef SSF_NO_DdeQueryDependName
BSTR CSsocxCtrl::DdeQueryDependName(LPCTSTR Server, LPCTSTR Topic, LPCTSTR Item, LPCTSTR NamePrev)
{
  LPSPREADSHEET lpSS = LockSheet();
  CString strResult;
  LPTSTR lpszName;
  int nNameLen;

  if( nNameLen = SS_DdeQueryDependNameLen(lpSS, Server, Topic, Item, NamePrev) )
  {
    lpszName = strResult.GetBuffer(nNameLen+1);
    SS_DdeQueryDependName(lpSS, Server, Topic, Item, NamePrev, lpszName, nNameLen+1);
    strResult.ReleaseBuffer();
  }
  UnlockSheet();
  return strResult.AllocSysString();
}
#endif

//--------------------------------------------------------------------

#ifndef SSF_NO_DdeGetDim
BOOL CSsocxCtrl::DdeGetDim(LPCTSTR Server, LPCTSTR Topic, LPCTSTR Item, long FAR* Cols, long FAR* Rows)
{
  LPSPREADSHEET lpSS = LockSheet();
  BOOL bRet = SS_DdeGetDim(lpSS, Server, Topic, Item, Cols, Rows);
  UnlockSheet();
  return bRet;
}
#endif

//--------------------------------------------------------------------

#ifndef SSF_NO_DdeIsLink
BOOL CSsocxCtrl::DdeIsLink(LPCTSTR Server, LPCTSTR Topic, LPCTSTR Item)
{
  LPSPREADSHEET lpSS = LockSheet();
  BOOL bRet = SS_DdeIsLink(lpSS, Server, Topic, Item);
  UnlockSheet();
  return bRet;
}
#endif

#ifdef SS_V30
#ifdef SS_V70
BOOL CSsocxCtrl::GetActionKey(long Action, short FAR *fShift, short FAR *fCtrl, short FAR *Key)
{
  LPSS_BOOK lpBook = LockBook();
  BOOL fRet;
  WORD wKey;
  BOOL bShift, bCtrl;

  fRet = SS_GetActionKey(lpBook, (short)Action, &bShift, &bCtrl, &wKey);
  *fShift = bShift;
  *fCtrl = bCtrl;
  *Key = wKey;
  UnlockBook();
  return (fRet);
}

BOOL CSsocxCtrl::SetActionKey(long Action, short fShift, short fCtrl, short Key)
{
  LPSS_BOOK lpBook = LockBook();
  BOOL fRet;
  fRet = SS_SetActionKey(lpBook, (short)Action, fShift, fCtrl, Key);
  UnlockBook();
  return (fRet);
}
#else
BOOL CSsocxCtrl::GetActionKey(short Action, short FAR *fShift, short FAR *fCtrl, short FAR *Key)
{
  LPSS_BOOK lpBook = LockBook();
  BOOL fRet;
  WORD wKey;
  BOOL bShift, bCtrl;

  fRet = SS_GetActionKey(lpBook, Action, &bShift, &bCtrl, &wKey);
  *fShift = bShift;
  *fCtrl = bCtrl;
  *Key = wKey;
  UnlockBook();
  return (fRet);
}

BOOL CSsocxCtrl::SetActionKey(short Action, short fShift, short fCtrl, short Key)
{
  LPSS_BOOK lpBook = LockBook();
  BOOL fRet;
  fRet = SS_SetActionKey(lpBook, Action, fShift, fCtrl, Key);
  UnlockBook();
  return (fRet);
}
#endif

BOOL CSsocxCtrl::GetOddEvenRowColor(COLORREF FAR *clrBackOdd, COLORREF FAR *clrForeOdd, COLORREF FAR *clrBackEven, COLORREF FAR *clrForeEven)
{
  LPSPREADSHEET lpSS = LockSheet();
  BOOL fRet;
  fRet = SS_GetOddEvenRowColor(lpSS, clrBackOdd, clrForeOdd, clrBackEven, clrForeEven);
  UnlockSheet();
  return (fRet);
}

BOOL CSsocxCtrl::SetOddEvenRowColor(COLORREF clrBackOdd, COLORREF clrForeOdd, COLORREF clrBackEven, COLORREF clrForeEven)
{
  LPSPREADSHEET lpSS = LockSheet();
  BOOL fRet;
  fRet = SS_SetOddEvenRowColor(lpSS, clrBackOdd, clrForeOdd, clrBackEven, clrForeEven);
  UnlockSheet();
  return (fRet);
}

#ifdef SS_V80
VARIANT CSsocxCtrl::SaveToBuffer ()
{
  LPSS_BOOK lpBook;
  HGLOBAL hGlobal;
  LPVOID lpBuffer;
  VARIANT Ret;
  long buffLen;

  lpBook = LockBook();
  hGlobal = SSSaveToBuffer(lpBook->hWnd, &buffLen, FALSE);
  VariantInit(&Ret);
  if( hGlobal && buffLen > 0 && (lpBuffer = GlobalLock(hGlobal)))
  {
     SAFEARRAYBOUND bounds;

     bounds.cElements = buffLen;
     bounds.lLbound = 0;
     Ret.vt = VT_ARRAY | VT_UI1;
     Ret.parray = SafeArrayCreate(VT_UI1, 1, &bounds);
     if( S_OK == SafeArrayLock(Ret.parray) )
     {
        MemHugeCpy(Ret.parray->pvData, lpBuffer, buffLen);
        SafeArrayUnlock(Ret.parray);
     }
     GlobalUnlock(hGlobal);
     GlobalFree(hGlobal);
  }
  UnlockBook();
  return Ret;
}
BOOL CSsocxCtrl::LoadFromBuffer (VARIANT *pBuff)
{
  LPSS_BOOK lpBook;
  HGLOBAL hGlobal;
  LPVOID lpBuffer;
  BOOL Ret = FALSE;
  long buffLen;
  SAFEARRAY* parray = NULL;

  if( pBuff && (pBuff->vt == (VT_BYREF|VT_VARIANT)) )
     pBuff = pBuff->pvarVal;
  if( pBuff && (pBuff->vt == (VT_ARRAY|VT_UI1|VT_BYREF)) && pBuff->pparray && (*pBuff->pparray)->cDims == 1)
     parray = *pBuff->pparray;
  else if( pBuff && (pBuff->vt == (VT_ARRAY|VT_UI1)) && pBuff->parray->cDims == 1 )
     parray = pBuff->parray;
  if( parray )
  {
     buffLen = parray->rgsabound[0].cElements;
     if( S_OK == SafeArrayLock(parray) )
     {
        hGlobal = GlobalAlloc(GHND, buffLen);
        if( lpBuffer = GlobalLock(hGlobal) )
        {
           MemHugeCpy(lpBuffer, parray->pvData, buffLen);
           GlobalUnlock(hGlobal);
           lpBook = LockBook();
           Ret = SSLoadFromBuffer(lpBook->hWnd, hGlobal, buffLen);
           UnlockBook();
           GlobalFree(hGlobal);
        }
        SafeArrayUnlock(parray);
     }
  }
  return Ret;
}

VARIANT CSsocxCtrl::SaveBlockToBuffer (long col, long row, long col2, long row2, BOOL fSaveColInfo, BOOL fSaveRowInfo)
{
  LPSS_BOOK lpBook;
  HGLOBAL hGlobal;
  LPVOID lpBuffer;
  VARIANT Ret;
  long buffLen;

  lpBook = LockBook();
  hGlobal = SSSaveBlockToBufferExt(lpBook->hWnd, col, row, col2, row2, &buffLen, FALSE, fSaveColInfo, fSaveRowInfo);
  VariantInit(&Ret);
  if( buffLen > 0 && (lpBuffer = GlobalLock(hGlobal)))
  {
     SAFEARRAYBOUND bounds;

     bounds.cElements = buffLen;
     bounds.lLbound = 0;
     Ret.vt = VT_ARRAY | VT_UI1;
     Ret.parray = SafeArrayCreate(VT_UI1, 1, &bounds);
     if( S_OK == SafeArrayLock(Ret.parray) )
     {
        MemHugeCpy(Ret.parray->pvData, lpBuffer, buffLen);
        SafeArrayUnlock(Ret.parray);
     }
     GlobalUnlock(hGlobal);
     GlobalFree(hGlobal);
  }
  UnlockBook();
  return Ret;
}
BOOL CSsocxCtrl::LoadBlockFromBuffer (long col, long row, long col2, long row2, VARIANT *pBuff)
{
  LPSS_BOOK lpBook;
  BOOL Ret = FALSE;

  if( pBuff && (pBuff->vt == (VT_BYREF | VT_VARIANT)) )
     pBuff = pBuff->pvarVal;
  if( pBuff && (pBuff->vt == (VT_ARRAY | VT_UI1)) && pBuff->parray->cDims == 1 && pBuff->parray->rgsabound[0].cElements > 0 && (S_OK == SafeArrayLock(pBuff->parray)))
  {
     lpBook = LockBook();
     Ret = SSLoadBlockFromBuffer(lpBook->hWnd, col, row, col2, row2, (HPBYTE)pBuff->parray->pvData, pBuff->parray->rgsabound[0].cElements);
     UnlockBook();
     SafeArrayUnlock(pBuff->parray);
  }
  return Ret;
}

BOOL CSsocxCtrl::IsVerticalScrollBarVisible()
{
  LPSS_BOOK lpBook;
  BOOL fRet;

  lpBook = LockBook();
  fRet = SS_IsVerticalScrollBarVisible(lpBook);
  UnlockBook();
  return (fRet);
}
BOOL CSsocxCtrl::IsHorizontalScrollBarVisible()
{
  LPSS_BOOK lpBook;
  BOOL fRet;

  lpBook = LockBook();
  fRet = SS_IsHorizontalScrollBarVisible(lpBook);
  UnlockBook();
  return (fRet);
}

BOOL CSsocxCtrl::SetEnhancedCornerColors(COLORREF clrBackColor, COLORREF clrHoverColor, COLORREF clrTriangleColor, COLORREF clrTriangleHover, COLORREF clrTriangleBorderColor, COLORREF clrTriangleHoverBorderColor)
{
  LPSS_BOOK lpBook;
  BOOL fRet;

  lpBook = LockBook();
  fRet = SS_SetEnhancedCornerColors(lpBook, clrBackColor, clrHoverColor, clrTriangleColor, clrTriangleHover, clrTriangleBorderColor, clrTriangleHoverBorderColor);
  UnlockBook();
  return (fRet);
}

BOOL CSsocxCtrl::SetEnhancedColumnHeaderColors(COLORREF clrSelectedHoverUpperColor, COLORREF clrSelectedHoverLowerColor, COLORREF clrSelectedUpperColor,
											  COLORREF clrSelectedLowerColor, COLORREF clrHoverUpperColor, COLORREF clrHoverLowerColor, COLORREF clrUpperColor,
											  COLORREF clrLowerColor, COLORREF clrSelectedBorderColor, COLORREF clrBorderColor)
{
  LPSS_BOOK lpBook;
  BOOL fRet;

  lpBook = LockBook();
  fRet = SS_SetEnhancedColumnHeaderColors(lpBook, clrSelectedHoverUpperColor, clrSelectedHoverLowerColor, clrSelectedUpperColor,
											  clrSelectedLowerColor, clrHoverUpperColor, clrHoverLowerColor, clrUpperColor,
											  clrLowerColor, clrSelectedBorderColor, clrBorderColor);
  UnlockBook();
  return (fRet);
}
BOOL CSsocxCtrl::SetEnhancedRowHeaderColors(COLORREF clrSelectedHoverUpperColor, COLORREF clrSelectedHoverLowerColor, COLORREF clrSelectedUpperColor,
											  COLORREF clrSelectedLowerColor, COLORREF clrHoverUpperColor, COLORREF clrHoverLowerColor, COLORREF clrUpperColor,
											  COLORREF clrLowerColor, COLORREF clrSelectedBorderColor, COLORREF clrBorderColor)
{
  LPSS_BOOK lpBook;
  BOOL fRet;

  lpBook = LockBook();
  fRet = SS_SetEnhancedRowHeaderColors(lpBook,clrSelectedHoverUpperColor, clrSelectedHoverLowerColor, clrSelectedUpperColor,
											  clrSelectedLowerColor, clrHoverUpperColor, clrHoverLowerColor, clrUpperColor,
											  clrLowerColor, clrSelectedBorderColor, clrBorderColor);
  UnlockBook();
  return (fRet);
}

BOOL CSsocxCtrl::SetEnhancedScrollBarColors(COLORREF clrTrackColor, COLORREF clrArrowColor, COLORREF clrUpperNormalStartColor, COLORREF clrUpperNormalEndColor,
										   COLORREF clrLowerNormalStartColor, COLORREF clrLowerNormalEndColor, COLORREF clrUpperPushedStartColor, COLORREF clrUpperPushedEndColor,
										   COLORREF clrLowerPushedStartColor, COLORREF clrLowerPushedEndColor, COLORREF clrUpperHoverStartColor, COLORREF clrUpperHoverEndColor,
										   COLORREF clrLowerHoverStartColor, COLORREF clrLowerHoverEndColor, COLORREF clrHoverButtonBorderColor, COLORREF clrButtonBorderColor)
{
  LPSS_BOOK lpBook;
  BOOL fRet;

  lpBook = LockBook();
  fRet = SS_SetEnhancedScrollBarColors(lpBook,  clrTrackColor, clrArrowColor, clrUpperNormalStartColor, clrUpperNormalEndColor,
										   clrLowerNormalStartColor, clrLowerNormalEndColor, clrUpperPushedStartColor, clrUpperPushedEndColor,
										   clrLowerPushedStartColor, clrLowerPushedEndColor, clrUpperHoverStartColor, clrUpperHoverEndColor,
										   clrLowerHoverStartColor, clrLowerHoverEndColor, clrHoverButtonBorderColor, clrButtonBorderColor);
  UnlockBook();
  return (fRet);
}

BOOL CSsocxCtrl::SetEnhancedSheetTabColors(COLORREF clrNormalUpperStartColor, COLORREF clrNormalUpperEndColor,
								   COLORREF clrNormalLowerStartColor, COLORREF clrNormalLowerEndColor, COLORREF clrHoverUpperStartColor,
								   COLORREF clrHoverUpperEndColor, COLORREF clrHoverLowerStartColor, COLORREF clrHoverLowerEndColor,
								   COLORREF clrTabOuterBorderColor, COLORREF clrTabInnerBorderColor, COLORREF clrButtonUpperNormalStartColor,
								   COLORREF clrButtonUpperNormalEndColor, COLORREF clrButtonLowerNormalStartColor, COLORREF clrButtonLowerNormalEndColor,
								   COLORREF clrButtonUpperHoverStartColor, COLORREF clrButtonUpperHoverEndColor, COLORREF clrButtonLowerHoverStartColor,
								   COLORREF clrButtonLowerHoverEndColor,COLORREF clrButtonUpperPushedStartColor, COLORREF clrButtonUpperPushedEndColor,
								   COLORREF clrButtonLowerPushedStartColor, COLORREF clrButtonLowerPushedEndColor, COLORREF clrArrowColor,
								   COLORREF clrBackColorStartColor, COLORREF clrBackColorEndColor, COLORREF clrSplitBoxStartColor,
								   COLORREF clrSplitBoxEndColor, COLORREF clrForeColor, COLORREF clrActiveForeColor)
{
  LPSS_BOOK lpBook;
  BOOL fRet;

  lpBook = LockBook();
  fRet = SS_SetEnhancedSheetTabColors(lpBook,  clrNormalUpperStartColor, clrNormalUpperEndColor,
								   clrNormalLowerStartColor, clrNormalLowerEndColor, clrHoverUpperStartColor,
								   clrHoverUpperEndColor, clrHoverLowerStartColor, clrHoverLowerEndColor,
								   clrTabOuterBorderColor, clrTabInnerBorderColor, clrButtonUpperNormalStartColor,
								   clrButtonUpperNormalEndColor, clrButtonLowerNormalStartColor, clrButtonLowerNormalEndColor,
								   clrButtonUpperHoverStartColor, clrButtonUpperHoverEndColor, clrButtonLowerHoverStartColor,
								   clrButtonLowerHoverEndColor, clrButtonUpperPushedStartColor, clrButtonUpperPushedEndColor,
								   clrButtonLowerPushedStartColor, clrButtonLowerPushedEndColor, clrArrowColor,
								   clrBackColorStartColor, clrBackColorEndColor, clrSplitBoxStartColor,
								   clrSplitBoxEndColor, clrForeColor, clrActiveForeColor);
  UnlockBook();
  return (fRet);
}

BOOL CSsocxCtrl::AddCustomCellType(LPCTSTR Name, BOOL Editable, BOOL CanOverflow, BOOL CanBeOverflown, BOOL UseRendererControl)
{
  LPSS_BOOK lpBook;
  BOOL fRet;

  lpBook = LockBook();
  fRet = SS_AddCustomCellType(lpBook, Name, Editable, CanOverflow, CanBeOverflown, UseRendererControl, &CT_Procs);
  UnlockBook();
  return (fRet);
}

BOOL CSsocxCtrl::RemoveCustomCellType(LPCTSTR Name)
{
  LPSS_BOOK lpBook;
  BOOL fRet;

  lpBook = LockBook();
  fRet = SS_RemoveCustomCellType(lpBook, Name);
  UnlockBook();
  return (fRet);
}

BOOL CSsocxCtrl::EnumCustomCellType(LPCTSTR PrevName, BSTR *Name)
{
  LPSS_BOOK lpBook = LockBook();
  TCHAR szName[255];
  int iRet;

  iRet = SS_EnumCustomCellType(lpBook, PrevName, szName, 255);
  SysFreeString(*Name);
  *Name = (BSTR)fpSysAllocBSTRFromTString(szName);
  UnlockBook();
  return iRet > 0;
}

BOOL CSsocxCtrl::GetCustomCellType(LPCTSTR Name, VARIANT_BOOL *Editable, VARIANT_BOOL *CanOverflow, VARIANT_BOOL *CanBeOverflown, VARIANT_BOOL *UseRendererControl)
{
  LPSS_BOOK lpBook;
  BOOL fRet;
  BOOL bEditable, bCanOverflow, bCanBeOverflown, bUseRendererControl;

  lpBook = LockBook();
  fRet = SS_CustCellTypeLookup(lpBook, Name, &bEditable, &bCanOverflow, &bCanBeOverflown, &bUseRendererControl, NULL);
  *Editable = bEditable ? VARIANT_TRUE : VARIANT_FALSE;
  *CanOverflow = bCanOverflow ? VARIANT_TRUE : VARIANT_FALSE;
  *CanBeOverflown = bCanBeOverflown ? VARIANT_TRUE : VARIANT_FALSE;
  *UseRendererControl = bUseRendererControl ? VARIANT_TRUE : VARIANT_FALSE;
  UnlockBook();
  return (fRet);
}

VARIANT CSsocxCtrl::GetCustomEditor(LPCTSTR Name, long Style)
{
  LPSPREADSHEET lpSS = LockSheet();
  HWND hWnd = SS_GetCustomEditor(lpSS, Name, Style);
  VARIANT Ret;

  VariantInit(&Ret);
  if( IsWindow(hWnd) )
  {
     LPDISPATCH lpDisp = (LPDISPATCH)SS_GetControlDispatchFromHwnd(lpSS, hWnd);
     if( lpDisp )
     {
        fpDispatchAddRef((long)lpDisp);
        Ret.vt = VT_DISPATCH;
        Ret.pdispVal = lpDisp;
     }
     else
     {
        Ret.vt = VT_I4;
        Ret.lVal = (long)hWnd;
     }
  }
  UnlockSheet();
  return Ret;
}

VARIANT CSsocxCtrl::GetCustomRenderer(LPCTSTR Name, long Style)
{
  LPSPREADSHEET lpSS = LockSheet();
  HWND hWnd = SS_GetCustomRenderer(lpSS, Name, Style);
  VARIANT Ret;

  VariantInit(&Ret);
  if( IsWindow(hWnd) )
  {
     LPDISPATCH lpDisp = (LPDISPATCH)SS_GetControlDispatchFromHwnd(lpSS, hWnd);
     if( lpDisp )
     {
        fpDispatchAddRef((long)lpDisp);
        Ret.vt = VT_DISPATCH;
        Ret.pdispVal = lpDisp;
     }
     else
     {
        Ret.vt = VT_I4;
        Ret.lVal = (long)hWnd;
     }
  }
  UnlockSheet();
  return Ret;
}

BOOL CSsocxCtrl::PaintControl(VARIANT *Ctrl, OLE_HANDLE hDC, OLE_XPOS_PIXELS Left, OLE_YPOS_PIXELS Top, OLE_XPOS_PIXELS Right, OLE_YPOS_PIXELS Bottom, VARIANT_BOOL ClientOnly)
{
   BOOL Ret = FALSE;
   HWND hWnd = NULL;
   RECT rect;
   BOOL fClientOnly = ClientOnly == VARIANT_TRUE;

   if( Ctrl && Ctrl->vt == (VT_VARIANT | VT_BYREF) )
      Ctrl = Ctrl->pvarVal;
   if( Ctrl && Ctrl->vt == VT_DISPATCH )
   {
      FPCONTROL ctl = {0};
      ctl.lpDispatch = (long)Ctrl->pdispVal;

      hWnd = fpVBGetControlHwnd(&ctl);
      if( hWnd == NULL )
      { // try to force window creation
         fpVBRecreateControlHwnd(&ctl);
         hWnd = (HWND)fpVBGetControlHwnd(&ctl);
      }
   }
   else if( Ctrl && Ctrl->vt == VT_I4 )
      hWnd = (HWND)Ctrl->lVal;
   if( IsWindow(hWnd) )
   {
      SetRect(&rect, Left, Top, Right, Bottom);
      Ret = fpRenderWindow(hWnd, (HDC)hDC, &rect, fClientOnly);
   }
   return Ret;
}

long CSsocxCtrl::GetFontHandle(IFontDisp* pFont)
{
   IFont* pifont = NULL;
   HFONT ret = 0;

   if( pFont && S_OK == pFont->QueryInterface(IID_IFont, (void**)&pifont))
   {
      pifont->get_hFont(&ret);
      pifont->Release();
   }
   return (long)ret;
}

#endif

BOOL CSsocxCtrl::GetFloat(long Col, long Row, double FAR *Value)
{
  LPSPREADSHEET lpSS = LockSheet();
  //modify by BOC 99.7.13 (hyt)------------------------------
  //The GetFloat function returns the floating-point value of 
  //a cell if the cell type is float. Otherwise, the dfValue 
  //parameter returns the value of 0.0 and the function returns False
  BOOL fRet=FALSE;
  SS_CELLTYPE CellType;
  *Value = 0.0;
  SS_AdjustCellCoords(lpSS, &Col, &Row);
  SS_RetrieveCellType(lpSS, &CellType, NULL, Col, Row);
  if(CellType.Type == SS_TYPE_FLOAT
#ifdef SS_V40
     || CellType.Type == SS_TYPE_CURRENCY
     || CellType.Type == SS_TYPE_NUMBER
     || CellType.Type == SS_TYPE_PERCENT
#endif // SS_V40
#ifdef SS_V70
     || CellType.Type == SS_TYPE_SCIENTIFIC
#endif // SS_V70
     )
  	fRet = SS_GetFloat(lpSS, Col, Row, Value);
  //---------------------------------------------------
  UnlockSheet();
  return (fRet);
}

BOOL CSsocxCtrl::SetFloat(long Col, long Row, double Value)
{
  LPSPREADSHEET lpSS = LockSheet();
  //modify by BOC 99.7.13 (hyt)------------------------------
  //The help file said h The SetFloat function sets the 
  //floating-point value of a cell if the cell type is float. 
  //Otherwise, the function returns False.h
  BOOL fRet=FALSE;

  SS_CELLTYPE CellType;
  SS_AdjustCellCoords(lpSS, &Col, &Row);
  SS_RetrieveCellType(lpSS, &CellType, NULL, Col, Row);
  if(CellType.Type == SS_TYPE_FLOAT
#ifdef SS_V40
     || CellType.Type == SS_TYPE_CURRENCY
     || CellType.Type == SS_TYPE_NUMBER
     || CellType.Type == SS_TYPE_PERCENT
#endif // SS_V40
#ifdef SS_V70
     || CellType.Type == SS_TYPE_SCIENTIFIC
#endif // SS_V70
     )
	fRet = SS_SetFloatRange(lpSS, Col, Row, Col, Row, Value);
  //----------------------------------------------------------
  UnlockSheet();
  return (fRet);
}

BOOL CSsocxCtrl::GetInteger(long Col, long Row, long FAR *Value)
{
  LPSPREADSHEET lpSS = LockSheet();
  BOOL fRet = FALSE;
  SS_CELLTYPE CellType;

  SS_AdjustCellCoords(lpSS, &Col, &Row);
  SS_RetrieveCellType(lpSS, &CellType, NULL, Col, Row);
// fix for bug 9332 -scl
// RFW - 8/23/02 - I modified this to include the other numeric types.  This broke user's code.
  if(CellType.Type == SS_TYPE_INTEGER || CellType.Type == SS_TYPE_COMBOBOX || CellType.Type == SS_TYPE_CHECKBOX || CellType.Type == SS_TYPE_FLOAT
#ifdef SS_V40
     || CellType.Type == SS_TYPE_NUMBER || CellType.Type == SS_TYPE_CURRENCY || CellType.Type == SS_TYPE_PERCENT
#endif // SS_V40
     )
    fRet = SS_GetInteger(lpSS, Col, Row, Value);
  UnlockSheet();
  return (fRet);
}

BOOL CSsocxCtrl::SetInteger(long Col, long Row, long Value)
{
  LPSPREADSHEET lpSS = LockSheet();
  BOOL fRet;
  SS_AdjustCellCoords(lpSS, &Col, &Row);
  fRet = SS_SetIntegerRange(lpSS, Col, Row, Col, Row, Value);
  UnlockSheet();
  return (fRet);
}

void CSsocxCtrl::SetCalText(LPCTSTR ShortDays, LPCTSTR LongDays, LPCTSTR ShortMonths, LPCTSTR LongMonths, LPCTSTR OkText, LPCTSTR CancelText)
{
  SSSetCalText(ShortDays, LongDays, ShortMonths, LongMonths, OkText, CancelText);
}

BOOL CSsocxCtrl::GetTextTipAppearance(BSTR FAR *FontName, short FAR *FontSize, short FAR *FontBold, short FAR *FontItalic, long FAR *BackColor, long FAR *ForeColor)
{
  LPSPREADSHEET lpSS = LockSheet();
  FP_TT_INFO    Info;
  BOOL          fRet;

  fRet = fpTextTipGetInfo(m_hWnd, &Info);

  *BackColor = Info.clrBack;
  *ForeColor = Info.clrFore;

  *FontName = (BSTR)fpSysAllocBSTRFromTString(Info.LogFont.lfFaceName);
  *FontSize = (short)abs((int)PIXELS_TO_PT(Info.LogFont.lfHeight));
  *FontBold = (Info.LogFont.lfWeight > FW_REGULAR);
  *FontItalic = Info.LogFont.lfItalic;

  UnlockSheet();
  return (fRet);
}

BOOL CSsocxCtrl::SetTextTipAppearance(LPCTSTR FontName, short FontSize, short FontBold, short FontItalic, long BackColor, long ForeColor)
{
  LPSPREADSHEET lpSS = LockSheet();
  FP_TT_INFO    Info;
  BOOL          fRet;
  WORD          wItems = FP_TT_ITEM_BACKCOLOR | FP_TT_ITEM_FORECOLOR;

  if (FontName && *FontName)
     {
     _fmemset(&Info.LogFont, '\0', sizeof(LOGFONT));
     Info.LogFont.lfHeight = -PT_TO_PIXELS(FontSize);
     Info.LogFont.lfWeight = (FontBold ? FW_BOLD : FW_REGULAR);
     Info.LogFont.lfItalic = (BYTE)FontItalic;
     Info.LogFont.lfCharSet = DEFAULT_CHARSET;

     lstrcpyn(Info.LogFont.lfFaceName, FontName, LF_FACESIZE);
     wItems |= FP_TT_ITEM_FONT;
     }

  Info.clrBack = BackColor;
  Info.clrFore = ForeColor;

  fRet = fpTextTipSetInfo(m_hWnd, wItems, &Info);

  UnlockSheet();
  return (fRet);
}

//--------------------------------------------------------------------
//
#ifdef SS_V80
#ifndef SSF_NO_ExportToHTML
BOOL CSsocxCtrl::ExportToHTMLEx (LPCTSTR lpszFileName, BOOL bAppendFlag, LPCTSTR lpszLogFile, LPCTSTR lpszTitle, LPCTSTR lpszCssFile, LPCTSTR lpszTableClass, LPCTSTR lpszTableRowClass, LPCTSTR lpszTableDefinitionClass, LPCTSTR lpszTableHeaderClass)
{
	LPSPREADSHEET lpSS;
	BOOL          fRet = FALSE;

	lpSS = LockSheet();
	if (!m_fSafeForScripting)
    fRet = SS_ExportRangeToHTML (lpSS, SS_ALLCOLS, SS_ALLROWS, 0, 0, lpszFileName, bAppendFlag, lpszLogFile, TRUE, lpszTitle, lpszCssFile, lpszTableClass, lpszTableRowClass, lpszTableDefinitionClass, lpszTableHeaderClass);

	// 10.04.99 JB GIC10752
	// return FALSE if embedded in a web page
	//
	UnlockSheet();

	return fRet;
}
#endif

#ifndef SSF_NO_ExportRangeToHTML
BOOL CSsocxCtrl::ExportRangeToHTMLEx (SS_COORD col, SS_COORD row, SS_COORD col2, SS_COORD row2, LPCTSTR lpszFileName, BOOL bAppendFlag, LPCTSTR lpszLogFile, LPCTSTR lpszTitle, LPCTSTR lpszCssFile, LPCTSTR lpszTableClass, LPCTSTR lpszTableRowClass, LPCTSTR lpszTableDefinitionClass, LPCTSTR lpszTableHeaderClass)
{
	LPSPREADSHEET lpSS;
	BOOL          fRet = FALSE;

	lpSS = LockSheet();
   if (!m_fSafeForScripting)
	{
		SS_AdjustCellRangeCoords(lpSS, &col, &row, &col2, &row2);
      fRet = SS_ExportRangeToHTML (lpSS, col, row, col2, row2, lpszFileName, bAppendFlag, lpszLogFile, FALSE, lpszTitle, lpszCssFile, lpszTableClass, lpszTableRowClass, lpszTableDefinitionClass, lpszTableHeaderClass);
	}
	UnlockSheet();

	return fRet;
}
#endif

#ifdef XL12
BOOL CSsocxCtrl::OpenExcel2007File(LPCTSTR FileName, LPCTSTR Password, short Sheet, short ExcelSheet, LPCTSTR LogFileName)
{
  BOOL fRet = FALSE;
  LPSS_BOOK lpBook = LockBook();
  FPCONTROL fpControl = { lpBook->hWnd, 0, (LONG)this, (LONG)this->GetIDispatch(FALSE) };
  fRet = SS_OpenExcel2007File(&fpControl, FileName, Password, Sheet, ExcelSheet, LogFileName);
  UnlockBook();
  return fRet;
}

BOOL CSsocxCtrl::SaveExcel2007File(LPCTSTR FileName, LPCTSTR Password, short Flags, LPCTSTR LogFileName)
{
  BOOL fRet = FALSE;
  LPSS_BOOK lpBook = LockBook();
  FPCONTROL fpControl = { lpBook->hWnd, 0, (LONG)this, (LONG)this->GetIDispatch(FALSE) };
  fRet = SS_SaveExcel2007File(&fpControl, FileName, Password, Flags, LogFileName);
  UnlockBook();
  return fRet;
}

short CSsocxCtrl::IsExcel2007File(LPCTSTR FileName)
{
  short fRet = FALSE;
  LPSS_BOOK lpBook = LockBook();
  FPCONTROL fpControl = { lpBook->hWnd, 0, (LONG)this, (LONG)this->GetIDispatch(FALSE) };
  fRet = SS_IsExcel2007File(&fpControl, FileName);
  UnlockBook();
  return fRet;
}
#endif // XL12

IPictureDisp* CSsocxCtrl::RenderToMetafile()
{
LPSS_BOOK lpBook = LockBook();
HDC hdcRef, hdcMeta;
int iWidthMM, iHeightMM, iWidthPels, iHeightPels;
RECT rectWindow, rect;
IPictureDisp* ret;
PICTDESC pictDesc;
HENHMETAFILE hemf;

// Obtain a handle to a reference device context.  
 
hdcRef = ::GetDC(lpBook->hWnd); 
 
// Determine the picture frame dimensions.  
// iWidthMM is the display width in millimeters.  
// iHeightMM is the display height in millimeters.  
// iWidthPels is the display width in pixels.  
// iHeightPels is the display height in pixels  
 
iWidthMM = GetDeviceCaps(hdcRef, HORZSIZE); 
iHeightMM = GetDeviceCaps(hdcRef, VERTSIZE); 
iWidthPels = GetDeviceCaps(hdcRef, HORZRES); 
iHeightPels = GetDeviceCaps(hdcRef, VERTRES); 
 
// Retrieve the coordinates of the client  
// rectangle, in pixels.  
 
::GetWindowRect(lpBook->hWnd, &rectWindow); 
OffsetRect(&rectWindow, -rectWindow.left, -rectWindow.top);
CopyRect(&rect, &rectWindow);
 
// Convert client coordinates to .01-mm units.  
// Use iWidthMM, iWidthPels, iHeightMM, and  
// iHeightPels to determine the number of  
// .01-millimeter units per pixel in the x-  
//  and y-directions.  
 
rect.left = (rect.left * iWidthMM * 100)/iWidthPels; 
rect.top = (rect.top * iHeightMM * 100)/iHeightPels; 
rect.right = (rect.right * iWidthMM * 100)/iWidthPels; 
rect.bottom = (rect.bottom * iHeightMM * 100)/iHeightPels; 
 
// Create the metafile device context.  
 
hdcMeta = CreateEnhMetaFile(hdcRef, NULL, &rect, NULL); 
 
fpRenderWindow(lpBook->hWnd, hdcMeta, &rectWindow, FALSE);
hemf = CloseEnhMetaFile(hdcMeta);
 
// Release the reference device context.  
::ReleaseDC(lpBook->hWnd, hdcRef); 
UnlockBook();

  pictDesc.cbSizeofstruct = sizeof(pictDesc);
  pictDesc.picType = PICTYPE_ENHMETAFILE;
  pictDesc.emf.hemf = hemf;
  OleCreatePictureIndirect(&pictDesc, IID_IPicture, TRUE, (LPVOID*)&ret);
  return ret;
}

#endif // SS_V80

#ifndef SSF_NO_ExportToHTML
BOOL CSsocxCtrl::ExportToHTML (LPCTSTR lpszFileName, BOOL bAppendFlag, LPCTSTR lpszLogFile)
{
	LPSPREADSHEET lpSS;
	BOOL          fRet = FALSE;

	lpSS = LockSheet();
	if (!m_fSafeForScripting)
    fRet = SS_ExportRangeToHTML (lpSS, SS_ALLCOLS, SS_ALLROWS, 0, 0, lpszFileName, bAppendFlag, lpszLogFile, TRUE, NULL, NULL, NULL, NULL, NULL, NULL);

	// 10.04.99 JB GIC10752
	// return FALSE if embedded in a web page
	//
	UnlockSheet();

	return fRet;
}

#ifndef SSF_NO_ExportRangeToHTML
BOOL CSsocxCtrl::ExportRangeToHTML (SS_COORD col, SS_COORD row, SS_COORD col2, SS_COORD row2, LPCTSTR lpszFileName, BOOL bAppendFlag, LPCTSTR lpszLogFile)
{
	LPSPREADSHEET lpSS;
	BOOL          fRet = FALSE;

	lpSS = LockSheet();
   if (!m_fSafeForScripting)
	{
		SS_AdjustCellRangeCoords(lpSS, &col, &row, &col2, &row2);
      fRet = SS_ExportRangeToHTML (lpSS, col, row, col2, row2, lpszFileName, bAppendFlag, lpszLogFile, FALSE, NULL, NULL, NULL, NULL, NULL, NULL);
	}
	UnlockSheet();

	return fRet;
}
#endif
#endif
#ifdef SS_V35
BOOL CSsocxCtrl::IsFetchCellNote()
{
	LPSS_BOOK lpBook = LockBook();
  return SS_IsFetchCellNote(lpBook);
}

// jb - 30.apr.01 ----------------------------------------------------
//
#ifndef SSF_NO_ExportToXML
BOOL CSsocxCtrl::ExportToXML (LPCTSTR pszFileName, LPCTSTR pszRoot, LPCTSTR pszCollection, long lFlags, LPCTSTR pszLogFile)
{
	BOOL fRet = TRUE;

	if (!m_fSafeForScripting)
	{	return ExportRangeToXML (SS_ALLCOLS, SS_ALLROWS, 0, 0, pszFileName, pszRoot, pszCollection, lFlags, pszLogFile);
	}

	// return FALSE if embedded in a web page
	//
	return !fRet;
}
#endif

#ifndef SSF_NO_ExportRangeToXML
BOOL CSsocxCtrl::ExportRangeToXML (SS_COORD col, SS_COORD row, SS_COORD col2, SS_COORD row2, LPCTSTR pszFile, LPCTSTR pszRoot, LPCTSTR pszCollection, long lFlags, LPCTSTR pszLogFile)
{
	LPSPREADSHEET lpSS;
	BOOL          bRet = TRUE;

	lpSS = LockSheet();
	if (!m_fSafeForScripting)
	{
		SS_AdjustCellRangeCoords(lpSS, &col, &row, &col2, &row2);
		bRet = SS_ExportRangeToXML (lpSS, col, row, col2, row2, pszFile, pszRoot, pszCollection, lFlags, pszLogFile);
	}
	UnlockSheet();

	return bRet;
}
#endif

#ifndef SSF_NO_ExportRangeToXMLBuffer
BOOL CSsocxCtrl::ExportRangeToXMLBuffer (SS_COORD col, SS_COORD row, SS_COORD col2, SS_COORD row2, LPCTSTR pszRoot, LPCTSTR pszCollection, BSTR *bstrXMLBuff, long lFlags, LPCTSTR pszLogFile)
{
	LPSPREADSHEET lpSS;
	BOOL          bRet = FALSE; // GAB 05/02/02
	HGLOBAL hBuf;
	LPTSTR pszBuf = NULL;

	lpSS = LockSheet();
// GAB 05/02/02 - 9985	if (!m_fSafeForScripting)
// GAB	{
		SS_AdjustCellRangeCoords(lpSS, &col, &row, &col2, &row2);
		hBuf = SS_ExportRangeToXMLBuffer (lpSS, col, row, col2, row2, pszRoot, pszCollection, lFlags, pszLogFile);

		// Put the HGLOBAL in the BSTR
		//
		if (NULL != hBuf)
		{	pszBuf = (LPTSTR)GlobalLock (hBuf);
// fix for bug 10007 -scl
//			*bstrXMLBuff = (BSTR)fpSysAllocBSTRFromTString (pszBuf);
#if _UNICODE
      BSTR ret = SysAllocString((OLECHAR *)pszBuf);
#else
      int len = lstrlen(pszBuf) + 1, newlen = len * sizeof(OLECHAR);
      BSTR ret = SysAllocStringByteLen(NULL,  newlen);
      memset(ret, 0, newlen);
      MultiByteToWideChar(CP_UTF8, 0, pszBuf, len, ret, newlen);
#endif
			GlobalUnlock (hBuf);
			GlobalFree (hBuf);
      bRet = TRUE;
      *bstrXMLBuff = ret;
		}
// GAB	}
	UnlockSheet();

	return bRet;
}
#endif

#ifndef SSF_NO_ExportToXMLBuffer
BOOL CSsocxCtrl::ExportToXMLBuffer (LPCTSTR pszRoot, LPCTSTR pszCollection, BSTR *bstrXMLBuff, long lFlags, LPCTSTR pszLogFile)
{
// GAB	BOOL bRet = TRUE;

// GAB 05/02/02 - 9985	if (!m_fSafeForScripting)
// GAB	{	
  return ExportRangeToXMLBuffer (SS_ALLCOLS, SS_ALLROWS, 0, 0, pszRoot, pszCollection, bstrXMLBuff, lFlags, pszLogFile);
//GAB	}

	// return FALSE if embedded in a web page
	//
// GAB	return !bRet;
}
#endif
#ifndef SSF_NO_ExportRangeToTextFile
BOOL CSsocxCtrl::ExportRangeToTextFile (SS_COORD col, SS_COORD row, SS_COORD col2, SS_COORD row2, LPCTSTR pcszFile, LPCTSTR pcszCellDelim, LPCTSTR pcszColDelim, LPCTSTR pcszRowDelim, long lFlags, LPCTSTR pcszLogFile)
{
	LPSPREADSHEET lpSS;
	BOOL          bRet = TRUE;

	lpSS = LockSheet();
	if (!m_fSafeForScripting)
	{
		SS_AdjustCellRangeCoords(lpSS, &col, &row, &col2, &row2);
//GAB 01/10/02 The Unicode flag was added as the last parameter and set to FALSE because this is
// the ANSI version of ExportRangeToTextFile
    // bRet = SS_ExportRangeToTextFile (lpSS, col, row, col2, row2, pcszFile, pcszCellDelim, pcszColDelim, pcszRowDelim, lFlags, pcszLogFile);
    bRet = SS_ExportRangeToTextFile (lpSS, col, row, col2, row2, pcszFile, pcszCellDelim, pcszColDelim, pcszRowDelim, lFlags, pcszLogFile, FALSE);
  }
	UnlockSheet();

	return bRet;
}
#endif
#ifndef SSF_NO_ExportToTextFile
BOOL CSsocxCtrl::ExportToTextFile (LPCTSTR pcszFile, LPCTSTR pcszCellDelim, LPCTSTR pcszColDelim, LPCTSTR pcszRowDelim, long lFlags, LPCTSTR pcszLogFile)
{
	BOOL bRet = TRUE;

	if (!m_fSafeForScripting)
	{	return ExportRangeToTextFile (SS_ALLCOLS, SS_ALLROWS, 0, 0, pcszFile, pcszCellDelim, pcszColDelim, pcszRowDelim, lFlags, pcszLogFile);
	}

	return !bRet;
}
#endif
#ifdef SS_V40
#ifndef SSF_NO_ExportRangeToTextFileU
BOOL CSsocxCtrl::ExportRangeToTextFileU (SS_COORD col, SS_COORD row, SS_COORD col2, SS_COORD row2, LPCTSTR pcszFile, LPCTSTR pcszCellDelim, LPCTSTR pcszColDelim, LPCTSTR pcszRowDelim, long lFlags, LPCTSTR pcszLogFile)
{
	LPSPREADSHEET lpSS;
	BOOL          bRet = TRUE;

	lpSS = LockSheet();
	if (!m_fSafeForScripting)
	{
		SS_AdjustCellRangeCoords(lpSS, &col, &row, &col2, &row2);
//GAB 01/10/02 The Unicode flag is set so file will be saved as Unicode
    bRet = SS_ExportRangeToTextFile (lpSS, col, row, col2, row2, pcszFile, pcszCellDelim, pcszColDelim, pcszRowDelim, lFlags, pcszLogFile, TRUE);
  }
	UnlockSheet();

	return bRet;
}
#endif
#ifndef SSF_NO_ExportToTextFileU
BOOL CSsocxCtrl::ExportToTextFileU (LPCTSTR pcszFile, LPCTSTR pcszCellDelim, LPCTSTR pcszColDelim, LPCTSTR pcszRowDelim, long lFlags, LPCTSTR pcszLogFile)
{
	BOOL bRet = TRUE;

	if (!m_fSafeForScripting)
	{	return ExportRangeToTextFileU (SS_ALLCOLS, SS_ALLROWS, 0, 0, pcszFile, pcszCellDelim, pcszColDelim, pcszRowDelim, lFlags, pcszLogFile);
	}

	return !bRet;
}
#endif
#endif //SS_V40

#ifndef SSF_NO_LoadTextFile
BOOL CSsocxCtrl::LoadTextFile (LPCTSTR pcszFile, LPCTSTR pcszCellDelim, LPCTSTR pcszColDelim, LPCTSTR pcszRowDelim, long lFlags, LPCTSTR pcszLogFile)
{
	BOOL          bRet = TRUE;

// GAB 04/18/02 - Bug 9985: I don't think the LoadTextFile needed to be qualified SafeForScripting because it's
// loading a file and not saving it.
// if (!m_fSafeForScripting)
//	{	bRet = SS_LoadTextFile (lpSS, pcszFile, pcszCellDelim, pcszColDelim, pcszRowDelim, lFlags, pcszLogFile);
//	}

   bRet = SS_LoadTextFile (m_hWnd, pcszFile, pcszCellDelim, pcszColDelim, pcszRowDelim, lFlags, pcszLogFile);

	return bRet;
}
#endif

void CSsocxCtrl::GetCalTextOverride(BSTR *pShortDays, LPSHORT pnLenShortDays, 
                   BSTR *pLongDays, LPSHORT pnLenLongDays, 
                   BSTR *pShortMonths, LPSHORT pnLenShortMonths, 
                   BSTR *pLongMonths, LPSHORT pnLenLongMonths,
                   BSTR *pOkText, LPSHORT pnLenOkText,
                   BSTR *pCancelText, LPSHORT pnLenCancelText)
{
  LPSS_BOOK lpBook;

  lpBook = LockBook();
  if (lpBook->hWnd)
     {
     LPTSTR pszShortDays;
     LPTSTR pszLongDays;
     LPTSTR pszShortMonths;
     LPTSTR pszLongMonths;
     LPTSTR pszOkText;
     LPTSTR pszCancelText;

     SSGetCalTextOverride(lpBook->hWnd, NULL, pnLenShortDays,
                                      NULL, pnLenLongDays,
                                      NULL, pnLenShortMonths,
                                      NULL, pnLenLongMonths,
                                      NULL, pnLenOkText,
                                      NULL, pnLenCancelText);

     pszShortDays = (LPTSTR)calloc(sizeof(TCHAR), (*pnLenShortDays + 1) * sizeof(TCHAR));
     pszLongDays = (LPTSTR)calloc(sizeof(TCHAR), (*pnLenLongDays + 1) * sizeof(TCHAR));
     pszShortMonths = (LPTSTR)calloc(sizeof(TCHAR), (*pnLenShortMonths + 1) * sizeof(TCHAR));
     pszLongMonths = (LPTSTR)calloc(sizeof(TCHAR), (*pnLenLongMonths + 1) * sizeof(TCHAR));
     pszOkText = (LPTSTR)calloc(sizeof(TCHAR), (*pnLenOkText + 1) * sizeof(TCHAR));
     pszCancelText = (LPTSTR)calloc(sizeof(TCHAR), (*pnLenCancelText + 1) * sizeof(TCHAR));

     SSGetCalTextOverride(lpBook->hWnd, pszShortDays, pnLenShortDays,
                                      pszLongDays, pnLenLongDays,
                                      pszShortMonths, pnLenShortMonths,
                                      pszLongMonths, pnLenLongMonths,
                                      pszOkText, pnLenOkText,
                                      pszCancelText, pnLenCancelText);

     *pShortDays = (BSTR)fpSysAllocBSTRFromTString(pszShortDays);
     *pLongDays = (BSTR)fpSysAllocBSTRFromTString(pszLongDays);
     *pShortMonths = (BSTR)fpSysAllocBSTRFromTString(pszShortMonths);
     *pLongMonths = (BSTR)fpSysAllocBSTRFromTString(pszLongMonths);
     *pOkText = (BSTR)fpSysAllocBSTRFromTString(pszOkText);
     *pCancelText = (BSTR)fpSysAllocBSTRFromTString(pszCancelText);
     
     free(pszShortDays);
     free(pszLongDays);
     free(pszShortMonths);
     free(pszLongMonths);
     free(pszOkText);
     free(pszCancelText);
     }

  UnlockBook();

}

void CSsocxCtrl::SetCalTextOverride(LPCTSTR ShortDays, LPCTSTR LongDays, LPCTSTR ShortMonths, LPCTSTR LongMonths, LPCTSTR OkText, LPCTSTR CancelText)
{
  LPSS_BOOK lpBook;

  lpBook = LockBook();
  if (lpBook->hWnd)
     SSSetCalTextOverride(lpBook->hWnd, ShortDays, LongDays, ShortMonths, LongMonths, OkText, CancelText);
  UnlockBook();

}
#endif // SS_V35

#if defined (SS_V30) && defined (WIN32)
#ifndef SSF_NO_IsExcelFile
int CSsocxCtrl::IsExcelFile (LPCTSTR FileName)
{
  return SSIsExcelFile(FileName);
}
#endif

#ifndef SSF_NO_GetExcelSheetList
BOOL CSsocxCtrl::GetExcelSheetList (LPCTSTR FileName, VARIANT FAR& VarArray, short FAR *ListCount, LPCTSTR LogFileName, short FAR *WorkbookHandle, BOOL Replace)
{
  return (_GetSheetList(&VarArray, FileName, ListCount, LogFileName, WorkbookHandle, Replace));
//  return (_GetSheetList(&VarArray, FileName, ListCount, LogFileName, WorkbookHandle, TRUE));
}

BOOL CSsocxCtrl::_GetSheetList(VARIANT FAR *lpArray, LPCTSTR FileName, short FAR *ListCount, LPCTSTR LogFileName, short FAR *WBHandle, BOOL fReplace)
{
  BOOL           fRet = FALSE;
  GLOBALHANDLE   ghList = (GLOBALHANDLE)0;
  SAFEARRAY FAR  *lpSA = NULL;
  long           lIndex = 0;
  long           l;
  LPTSTR         lpstr = NULL;
  short          sOffset = 0;
  short          sListMax = 0;
  BOOL           fVariant = FALSE;

  if (lpArray)
  {
    if (lpArray->vt == (VT_ARRAY|VT_BSTR))
      lpSA = lpArray->parray;
    else if (lpArray->vt == (VT_ARRAY|VT_BSTR|VT_BYREF))
      lpSA = *lpArray->pparray;
    else if (lpArray->vt == (VT_ARRAY|VT_VARIANT))
    {
      fVariant = TRUE;
      lpSA = lpArray->parray;
    }
    else if (lpArray->vt == (VT_ARRAY|VT_VARIANT|VT_BYREF))
    {
      fVariant = TRUE;
      lpSA = *lpArray->pparray;
    }
    else if (lpArray->vt == (VT_BYREF|VT_VARIANT))
    {
      return _GetSheetList(V_VARIANTREF(lpArray), FileName, ListCount, LogFileName, WBHandle, fReplace);
    }
    else if (lpArray->vt == VT_NULL)
    {
    }
    else
    {
    }
  }  

  if (lpSA && SafeArrayGetDim(lpSA) < 1)
    return fRet;

  LPSS_BOOK lpBook = LockBook();
  fRet = SS_GetExcelSheetList(lpBook, FileName, (LPGLOBALHANDLE)&ghList, ListCount, LogFileName, WBHandle, fReplace);
  UnlockSheet();
  
  if (lpSA)
  {
    sListMax = (short)min((short)lpSA->rgsabound[0].cElements, *ListCount);
    lpstr = (LPTSTR)GlobalLock(ghList);
    for (l=0; l<sListMax; l++)
    {
      BSTR   bstrVal;
      LPBYTE lp = NULL;

      bstrVal = (BSTR)fpSysAllocBSTRFromTString((FPTSTR)&(lpstr[sOffset]));
      sOffset += lstrlen(&(lpstr[sOffset])) + 1;
    
      if (fVariant)
      {
        VARIANT var;
        var.vt = VT_BSTR;
        var.bstrVal = bstrVal;
        SafeArrayPutElement(lpSA, &l, (LPVOID)&var);
      }
      else
      {
        SafeArrayPutElement(lpSA, &l, (LPVOID)bstrVal);
      }
    }
  }
  GlobalUnlock(ghList);
  GlobalFree(ghList);
  
  return fRet;
}
#endif

#ifndef SSF_NO_ImportExcelSheet
BOOL CSsocxCtrl::ImportExcelSheet (short WorkbookHandle, VARIANT FAR& Sheet)
{
  LPSS_BOOK     lpBook = LockBook();
  BOOL          fRet = FALSE;
  short         sSheetNum = 0;
  LPSSXL        lpXL = NULL;
  LPTSTR        lpszSheetName = NULL;
  VARIANT       var = Sheet;

  if (var.vt == (VT_VARIANT|VT_BYREF))
  {
    var = *(var.pvarVal);
  }
  
  if (var.vt & VT_BSTR)
  {
    GLOBALHANDLE ghList = 0;
    short        sListCount = 0;
    LPBYTE       lpb = NULL;
    BSTR         bstr;

    if (var.vt & VT_BYREF)
      bstr = *(var.pbstrVal);
    else
      bstr = var.bstrVal;

    lpXL = (LPSSXL)SS_ExcelTableLock();
    if (!lpXL || WorkbookHandle >= ExcelTable.dTableCnt)
    {
    	UnlockSheet();
      return fRet;
    }
    SS_GetXLSheetList(&(lpXL[WorkbookHandle]), (GLOBALHANDLE FAR *)&ghList, (LPSHORT)&sListCount);
    SS_ExcelTableUnlock();
  
    lpb = (LPBYTE)GlobalLock(ghList);

//    lpszSheetName = fpOLE2Str((LPSTR)Sheet.bstrVal);

    lpszSheetName = fpSysAllocTStringFromBSTR((LPVOID)bstr);

    fRet = SS_LoadXLSheet(lpBook, WorkbookHandle, -1, lpszSheetName);

    fpSysFreeTString(lpszSheetName);

    GlobalUnlock(ghList);
    GlobalFree(ghList);
  }
  else if (var.vt & VT_I2)
  {
    if (var.vt & VT_BYREF)
      sSheetNum = *(var.piVal);
    else
      sSheetNum = var.iVal;
    if (sSheetNum == -1)
    {
    	UnlockSheet();
      return fRet;
    }
    fRet = SS_LoadXLSheet(lpBook, WorkbookHandle, sSheetNum, NULL);
  }
  else
  {
  	UnlockSheet();
    return fRet;
  }
// hatakeyama@boc.co.jp 11/xx-2000 for Setting Printer 
// Reflect print settings to SPREAD sheet.
   /*
	{
		((CSsocxCtrl FAR*)lpBook->lpOleControl)->m_bPrintGrid = lpBook->PrintOptions.pf.fShowGrid;
		((CSsocxCtrl FAR*)lpBook->lpOleControl)->m_bPrintColHeaders = lpBook->PrintOptions.pf.fShowColHeaders;
		((CSsocxCtrl FAR*)lpBook->lpOleControl)->m_bPrintRowHeaders = lpBook->PrintOptions.pf.fShowRowHeaders;
		((CSsocxCtrl FAR*)lpBook->lpOleControl)->m_bPrintBorder = lpBook->PrintOptions.pf.fDrawBorder;
		((CSsocxCtrl FAR*)lpBook->lpOleControl)->m_nPrintOrientation  = lpBook->PrintOptions.pf.wOrientation;
		((CSsocxCtrl FAR*)lpBook->lpOleControl)->m_bPrintColor   = lpBook->PrintOptions.pf.fDrawColors;
		((CSsocxCtrl FAR*)lpBook->lpOleControl)->m_bPrintSmartPrint = FALSE;  
	}
   */


//Add by BOC 99.7.23 (hyt)----------------------------
//for draw selblock error after import excel file
  LPSPREADSHEET lpSS = SS_BookLockSheetIndex(lpBook, 0);//sSheetNum);
  if(lpSS->fMultipleBlocksSelected)
  	lpBook->fSelBlockInvisible = 1;
  SS_BookUnlockSheetIndex(lpBook, 0); //sSheetNum);
//----------------------------------------------------

  UnlockBook();
  return !fRet;
}
#endif

#ifndef SSF_NO_ExportToExcel
#ifdef SS_V70
BOOL CSsocxCtrl::ExportExcelBook (LPCTSTR FileName, LPCTSTR LogFileName)
{
  return ExportExcelBookEx(FileName, LogFileName, 0);
}

BOOL CSsocxCtrl::ExportExcelBookEx (LPCTSTR FileName, LPCTSTR LogFileName, SHORT sFlags)
{
  LPSS_BOOK lpBook = LockBook();
  BOOL      fRet = FALSE;

  if (!m_fSafeForScripting)
  {
    TCHAR tDrive[_MAX_DRIVE], 
          tDir[_MAX_DIR], 
          tFileName[_MAX_FNAME], 
          tExt[_MAX_EXT],
          tPath[_MAX_PATH];
	
    tDrive[0] = tDir[0] = tFileName[0] = tExt[0] = 0;
    memset(tPath,0,_MAX_PATH*sizeof(TCHAR));
    _tsplitpath (FileName, tDrive, tDir, tFileName, tExt);
    memcpy(tPath,FileName,lstrlen(FileName)*sizeof(TCHAR));
    if (tExt[0] == 0)
      _tcscat(tPath, _T(".xls"));	

    // hatakeyama@boc.co.jp 11/xx-2000 for Setting Printer 
    // Retrieve print settings from SPREAD.
    /*
    lpBook->PrintOptions.pf.fShowGrid = ((CSsocxCtrl FAR*)lpBook->lpOleControl)->m_bPrintGrid;
    lpBook->PrintOptions.pf.fShowColHeaders = ((CSsocxCtrl FAR*)lpBook->lpOleControl)->m_bPrintColHeaders;
    lpBook->PrintOptions.pf.fShowRowHeaders = ((CSsocxCtrl FAR*)lpBook->lpOleControl)->m_bPrintRowHeaders;
    lpBook->PrintOptions.pf.fDrawBorder = ((CSsocxCtrl FAR*)lpBook->lpOleControl)->m_bPrintBorder;
    lpBook->PrintOptions.pf.wOrientation = ((CSsocxCtrl FAR*)lpBook->lpOleControl)->m_nPrintOrientation;
    lpBook->PrintOptions.pf.fDrawColors = ((CSsocxCtrl FAR*)lpBook->lpOleControl)->m_bPrintColor;
    */

    fRet = SS_SaveXLFile(lpBook, tPath, NULL, LogFileName, NULL, NULL, TRUE, sFlags);
  }
  
  UnlockBook();

  return !fRet;
}

BOOL CSsocxCtrl::ExportToExcel (LPCTSTR FileName, LPCTSTR SheetName, LPCTSTR LogFileName)
{
  return ExportToExcelEx(FileName, SheetName, LogFileName, 0);
}

BOOL CSsocxCtrl::ExportToExcelEx (LPCTSTR FileName, LPCTSTR SheetName, LPCTSTR LogFileName, SHORT Flags)
{
  LPSS_BOOK lpBook = LockBook();
  BOOL      fRet = FALSE;

  if (!m_fSafeForScripting)
  {
    TCHAR tDrive[_MAX_DRIVE], 
          tDir[_MAX_DIR], 
          tFileName[_MAX_FNAME], 
          tExt[_MAX_EXT],
          tPath[_MAX_PATH];
	
    tDrive[0] = tDir[0] = tFileName[0] = tExt[0] = 0;
    memset(tPath,0, _MAX_PATH*sizeof(TCHAR));
    _tsplitpath (FileName, tDrive, tDir, tFileName, tExt);
    memcpy(tPath,FileName,lstrlen(FileName)*sizeof(TCHAR));
    if (tExt[0] == 0)
      _tcscat(tPath, _T(".xls"));	

    // hatakeyama@boc.co.jp 11/xx-2000 for Setting Printer 
    // Retrieve print settings from SPREAD.
    /*
    lpBook->PrintOptions.pf.fShowGrid = ((CSsocxCtrl FAR*)lpBook->lpOleControl)->m_bPrintGrid;
    lpBook->PrintOptions.pf.fShowColHeaders = ((CSsocxCtrl FAR*)lpBook->lpOleControl)->m_bPrintColHeaders;
    lpBook->PrintOptions.pf.fShowRowHeaders = ((CSsocxCtrl FAR*)lpBook->lpOleControl)->m_bPrintRowHeaders;
    lpBook->PrintOptions.pf.fDrawBorder = ((CSsocxCtrl FAR*)lpBook->lpOleControl)->m_bPrintBorder;
    lpBook->PrintOptions.pf.wOrientation = ((CSsocxCtrl FAR*)lpBook->lpOleControl)->m_nPrintOrientation;
    lpBook->PrintOptions.pf.fDrawColors = ((CSsocxCtrl FAR*)lpBook->lpOleControl)->m_bPrintColor;
    */

    if(lstrlen(SheetName))
      fRet = SS_SaveXLFile(lpBook, tPath, SheetName, LogFileName, NULL, NULL, TRUE, Flags);
#ifdef SS_V70
    else
    {
      TCHAR buffer[100];
      _stprintf(buffer, _T("Sheet%d"), lpBook->nSheet+1);
      fRet = SS_SaveXLFile(lpBook, tPath, buffer, LogFileName, NULL, NULL, TRUE, Flags);
    }
#else
    else
      fRet = SS_SaveXLFile(lpBook, tPath, _T("Sheet1"), LogFileName, NULL, NULL, TRUE, Flags);
#endif
  }
  
  UnlockBook();

  return !fRet;
}

#else

BOOL CSsocxCtrl::ExportToExcel (LPCTSTR FileName, LPCTSTR SheetName, LPCTSTR LogFileName)
{
  LPSS_BOOK lpBook = LockBook();
  BOOL      fRet = FALSE;

  if (!m_fSafeForScripting)
  {
    TCHAR tDrive[_MAX_DRIVE], 
          tDir[_MAX_DIR], 
          tFileName[_MAX_FNAME], 
          tExt[_MAX_EXT],
          tPath[_MAX_PATH];
	
    tDrive[0] = tDir[0] = tFileName[0] = tExt[0] = 0;
    memset(tPath,0,_MAX_PATH * sizeof(TCHAR));
    _tsplitpath (FileName, tDrive, tDir, tFileName, tExt);
    memcpy(tPath,FileName,lstrlen(FileName)*sizeof(TCHAR));
    if (tExt[0] == 0)
      _tcscat(tPath, _T(".xls"));	

    // hatakeyama@boc.co.jp 11/xx-2000 for Setting Printer 
    // Retrieve print settings from SPREAD.
    /*
    lpBook->PrintOptions.pf.fShowGrid = ((CSsocxCtrl FAR*)lpBook->lpOleControl)->m_bPrintGrid;
    lpBook->PrintOptions.pf.fShowColHeaders = ((CSsocxCtrl FAR*)lpBook->lpOleControl)->m_bPrintColHeaders;
    lpBook->PrintOptions.pf.fShowRowHeaders = ((CSsocxCtrl FAR*)lpBook->lpOleControl)->m_bPrintRowHeaders;
    lpBook->PrintOptions.pf.fDrawBorder = ((CSsocxCtrl FAR*)lpBook->lpOleControl)->m_bPrintBorder;
    lpBook->PrintOptions.pf.wOrientation = ((CSsocxCtrl FAR*)lpBook->lpOleControl)->m_nPrintOrientation;
    lpBook->PrintOptions.pf.fDrawColors = ((CSsocxCtrl FAR*)lpBook->lpOleControl)->m_bPrintColor;
    */

    if(lstrlen(SheetName))
      fRet = SS_SaveXLFile(lpBook, tPath, SheetName, LogFileName, NULL, NULL, TRUE, 0);
#ifdef SS_V70
    else
    {
      TCHAR buffer[100];
      _stprintf(buffer, _T("Sheet%d"), lpBook->nSheet);
      fRet = SS_SaveXLFile(lpBook, tPath, buffer, LogFileName, NULL, NULL, TRUE, 0);
    }
#else
    else
      fRet = SS_SaveXLFile(lpBook, tPath, _T("Sheet1"), LogFileName, NULL, NULL, TRUE, 0);
#endif
  }
  
  UnlockBook();

  return !fRet;
}
#endif // SS_V70

#endif
#endif //#if defined (SS_V30) && defined (WIN32)

BOOL CSsocxCtrl::SetArray(SS_COORD ColLeft, SS_COORD RowTop, VARIANT FAR& VarArray)
{
return (_SpreadArray(&VarArray, ColLeft, RowTop, FALSE));
}


BOOL CSsocxCtrl::GetArray(SS_COORD ColLeft, SS_COORD RowTop, VARIANT FAR& VarArray)
{
return (_SpreadArray(&VarArray, ColLeft, RowTop, TRUE));
}


BOOL CSsocxCtrl::_SpreadArray(VARIANT FAR *lpArray, SS_COORD ColLeft,
                              SS_COORD RowTop, BOOL fGet)
{
LPSPREADSHEET lpSS = LockSheet();
SS_COORD      ColLeftOrig = ColLeft;
SS_COORD      RowTopOrig = RowTop;
BOOL          fRet = FALSE;

SS_AdjustCellCoords(lpSS, &ColLeft, &RowTop);
if (lpArray && (lpArray->vt & VT_ARRAY))
   {
   SAFEARRAY FAR *lpSA;
   long           lIndexCnt;
   short          nDataType;
      
   if (lpArray->vt & VT_BYREF)
      {
      lpSA = *lpArray->pparray;
      nDataType = lpArray->vt & ~VT_ARRAY & ~VT_BYREF;
      }
   else
      {
      lpSA = lpArray->parray;
      nDataType = lpArray->vt & ~VT_ARRAY;
      }

   lIndexCnt = SafeArrayGetDim(lpSA);

   if (lIndexCnt >= 1 && lIndexCnt <= 2)
      {
      long lRowLB;
      long lRowUB;

      if (SafeArrayGetLBound(lpSA, 1, &lRowLB) == S_OK &&
          SafeArrayGetUBound(lpSA, 1, &lRowUB) == S_OK)
         {
         long lColLB = 0;
         long lColUB = 0;

         fRet = -1;
         if (lIndexCnt == 2)
            {
            if (SafeArrayGetLBound(lpSA, 2, &lColLB) != S_OK ||
                SafeArrayGetUBound(lpSA, 2, &lColUB) != S_OK)
               fRet = FALSE;
            }

         if (fRet)
            {
            TBGLOBALHANDLE hBuffer;
            SS_COORD       ColAt;
            SS_COORD       RowAt;
            long           lSize = (long)SafeArrayGetElemsize(lpSA);
            long           lIndexes[2];
            long           Row;
            long           Col;

            if (hBuffer = tbGlobalAlloc(GHND, lSize))
               {
               LPSTR lpszBuffer = (LPSTR)tbGlobalLock(hBuffer);

               for (Row = 0; Row <= lRowUB - lRowLB; Row++)
                  {
                  for (Col = 0; Col <= lColUB - lColLB; Col++)
                     {
                     lIndexes[0] = lRowLB + Row;
                     lIndexes[1] = lColLB + Col;

                     ColAt = ColLeft + Col;
                     RowAt = RowTop + Row;

                     if (!fGet)
                        SafeArrayGetElement(lpSA, lIndexes, lpszBuffer);

                     switch (nDataType)
                        {
                        #if defined(WIN32)
                        case VT_UI1:
                           if (fGet)
                              {
                              long lVal;
                              BYTE bVal;

                              SS_GetInteger(lpSS, ColAt, RowAt, &lVal);
                              bVal = (BYTE)lVal;

                              SafeArrayPutElement(lpSA, lIndexes, (LPVOID)&bVal);
                              }
                           else
                              {
                              long lVal = (long)*(LPBYTE)lpszBuffer;
                              SS_SetDataRange(lpSS, ColAt, RowAt, ColAt, RowAt,
                                              (LPTSTR)&lVal, SS_VALUE_INT, FALSE, TRUE);
                              }

                           break;
                        #endif

                        case VT_I2:
                           if (fGet)
                              {
                              long  lVal;
                              short nVal;

                              SS_GetInteger(lpSS, ColAt, RowAt, &lVal);
                              nVal = (short)lVal;

                              SafeArrayPutElement(lpSA, lIndexes, (LPVOID)&nVal);
                              }
                           else
                              SS_SetIntegerRange(lpSS, ColAt, RowAt, ColAt, RowAt,
                                                 (long)*(LPSHORT)lpszBuffer);
                           break;

                        case VT_I4:
                           if (fGet)
                              {
                              long  lVal;

                              SS_GetInteger(lpSS, ColAt, RowAt, &lVal);
                              SafeArrayPutElement(lpSA, lIndexes, (LPVOID)&lVal);
                              }
                           else
                              SS_SetIntegerRange(lpSS, ColAt, RowAt, ColAt, RowAt,
                                                 *(LPLONG)lpszBuffer);
                           break;

                        case VT_R4:
                           if (fGet)
                              {
                              double dfVal;
                              float  fVal;

                              SS_GetFloat(lpSS, ColAt, RowAt, &dfVal);
                              fVal = (float)dfVal;
                              SafeArrayPutElement(lpSA, lIndexes, (LPVOID)&fVal);
                              }
                           else
                              SS_SetFloatRange(lpSS, ColAt, RowAt, ColAt, RowAt,
                                               (double)*(LPFLOAT)lpszBuffer);
                           break;

                        case VT_R8:
                           if (fGet)
                              {
                              double dfVal;

                              SS_GetFloat(lpSS, ColAt, RowAt, &dfVal);
                              SafeArrayPutElement(lpSA, lIndexes, (LPVOID)&dfVal);
                              }
                           else
                              SS_SetFloatRange(lpSS, ColAt, RowAt, ColAt, RowAt,
                                               *(LPDOUBLE)lpszBuffer);
                           break;

                        case VT_CY:
                           if (fGet)
                              {
                              double dfVal;
                              CY cy;

                              SS_GetFloat(lpSS, ColAt, RowAt, &dfVal);
                              VarCyFromR8(dfVal, &cy);
                              SafeArrayPutElement(lpSA, lIndexes, (LPVOID)&cy);
                              }
                           else
                              {
                              double dfVal;

                              VarR8FromCy(*(LPCY)lpszBuffer, &dfVal);
                              SS_SetFloatRange(lpSS, ColAt, RowAt, ColAt, RowAt, dfVal);
                              }
                           break;

                        case VT_VARIANT:
									{
									SS_COORD ColTemp = ColAt;
									SS_COORD RowTemp = RowAt;

									SS_AdjustCellCoordsOut(lpSS, &ColTemp, &RowTemp);

                           if (fGet)
                              {
                              fpGetText(ColTemp, RowTemp, (LPVAR)lpszBuffer);
                              SafeArrayPutElement(lpSA, lIndexes, (LPVOID)lpszBuffer);
                              }
                           else
										{
                              fpSetText(ColTemp, RowTemp, *(LPVAR)lpszBuffer);
										VariantClear((LPVAR)lpszBuffer);
										}
									}
                           break;

                        case VT_BSTR:
                           if (fGet)
                              {
                              CString        s;
                              TBGLOBALHANDLE hText;
                              SS_CELLTYPE    cellType;
                              LPTSTR         lpszText;
                              BSTR           bstr;

                              if( lpSS->lpBook->EditModeOn && lpSS->Col.CurAt == ColAt && lpSS->Row.CurAt == RowAt )
                                hText = SS_GetEditModeOnData(lpSS->lpBook->hWnd, lpSS, FALSE);
                              else
                                hText = SS_GetData(lpSS, &cellType, ColAt, RowAt, FALSE);

                              if( hText )
                                 {
                                 lpszText = (LPTSTR)tbGlobalLock(hText);
                                 s = lpszText;
                                 tbGlobalUnlock(hText);
                                 if( lpSS->lpBook->EditModeOn && lpSS->Col.CurAt == ColAt && lpSS->Row.CurAt == RowAt )
                                    tbGlobalFree(hText);
                                 }

                              bstr = s.AllocSysString();
                              SafeArrayPutElement(lpSA, lIndexes, (LPVOID)bstr);
                              ::SysFreeString(bstr);
                              }
                           else
                              {
                              LPTSTR tstrTmp = *(LPTSTR *)lpszBuffer;
                              BOOL   fFreeTmp = FALSE;
                              // if necessary, convert from WCHAR to CHAR
                              if (sizeof(OLECHAR) != sizeof(TCHAR))
                                 {
                                 tstrTmp = fpSysAllocTStringFromBSTR((LPVOID)tstrTmp);
                                 fFreeTmp = TRUE;
                                 }

                              SS_SetDataRange(lpSS, ColAt, RowAt, ColAt, RowAt, tstrTmp, FALSE, FALSE, TRUE);

                              SysFreeString(*(BSTR *)lpszBuffer);

                              if (fFreeTmp && tstrTmp)
                                 fpSysFreeTString(tstrTmp);
                              }

                           break;
                        }
                     }
                  }

               tbGlobalUnlock(hBuffer);
               tbGlobalFree(hBuffer);
               }
            }
         }
      }
   }

else if (lpArray && (lpArray->vt & (VT_VARIANT | VT_BYREF)))
   fRet = _SpreadArray(V_VARIANTREF(lpArray), ColLeftOrig, RowTopOrig, fGet);

UnlockSheet();
return (fRet);
}

#endif // SS_V30


#ifdef SS_V35
//--------------------------------------------------------------------

BOOL CSsocxCtrl::OwnerPrintDraw(OLE_HANDLE hDC, long Left, long Top, long Right, long Bottom, short Page)
{
  LPSPREADSHEET lpSS = LockSheet();
  HDC hDCPrinter = m_hDCPrinter;
  float fPrintMarginLeft = lpSS->PrintOptions.pf.fMarginLeft;
  float fPrintMarginTop = lpSS->PrintOptions.pf.fMarginTop;
  float fPrintMarginRight = lpSS->PrintOptions.pf.fMarginRight;
  float fPrintMarginBottom = lpSS->PrintOptions.pf.fMarginBottom;
  short nHorzRes = GetDeviceCaps((HDC)hDC, HORZRES);
  short nVertRes = GetDeviceCaps((HDC)hDC, VERTRES);
  short nLogPixelsX = GetDeviceCaps((HDC)hDC, LOGPIXELSX);
  short nLogPixelsY = GetDeviceCaps((HDC)hDC, LOGPIXELSY);
  BOOL bRet = TRUE;
  
  lpSS->PrintOptions.pf.fMarginLeft += (float)Left / (float)1440.0;
  lpSS->PrintOptions.pf.fMarginTop += (float)Top / (float)1440.0;
  lpSS->PrintOptions.pf.fMarginRight += (float)(MulDiv((long)nHorzRes, 1440, nLogPixelsX) - Right) / (float)1440.0;
  lpSS->PrintOptions.pf.fMarginBottom += (float)(MulDiv((long)nVertRes, 1440, nLogPixelsY) - Bottom) / (float)1440.0;
  m_hDCPrinter = (HDC)hDC;

  PrintSpread(SS_PRINT_ACTION_OWNERPRINT, 0, NULL, Page, NULL);

  m_hDCPrinter = hDCPrinter;
  lpSS->PrintOptions.pf.fMarginLeft = fPrintMarginLeft;
  lpSS->PrintOptions.pf.fMarginTop = fPrintMarginTop;
  lpSS->PrintOptions.pf.fMarginRight = fPrintMarginRight;
  lpSS->PrintOptions.pf.fMarginBottom = fPrintMarginBottom;

  UnlockSheet();
  return bRet;
}

//--------------------------------------------------------------------

BOOL CSsocxCtrl::OwnerPrintPageCount(OLE_HANDLE hDC, long Left, long Top, long Right, long Bottom, short *pPageCount)
{
  LPSPREADSHEET lpSS = LockSheet();
  PRVW_INFO PrvwInfo;
  HDC hDCPrinter = m_hDCPrinter;
  float fPrintMarginLeft = lpSS->PrintOptions.pf.fMarginLeft;
  float fPrintMarginTop = lpSS->PrintOptions.pf.fMarginTop;
  float fPrintMarginRight = lpSS->PrintOptions.pf.fMarginRight;
  float fPrintMarginBottom = lpSS->PrintOptions.pf.fMarginBottom;
  short nHorzRes = GetDeviceCaps((HDC)hDC, HORZRES);
  short nVertRes = GetDeviceCaps((HDC)hDC, VERTRES);
  short nLogPixelsX = GetDeviceCaps((HDC)hDC, LOGPIXELSX);
  short nLogPixelsY = GetDeviceCaps((HDC)hDC, LOGPIXELSY);
  BOOL bRet = TRUE;
  
  lpSS->PrintOptions.pf.fMarginLeft += (float)Left / (float)1440.0;
  lpSS->PrintOptions.pf.fMarginTop += (float)Top / (float)1440.0;
  lpSS->PrintOptions.pf.fMarginRight += (float)(MulDiv((long)nHorzRes, 1440, nLogPixelsX) - Right) / (float)1440.0;
  lpSS->PrintOptions.pf.fMarginBottom += (float)(MulDiv((long)nVertRes, 1440, nLogPixelsY) - Bottom) / (float)1440.0;
  m_hDCPrinter = (HDC)hDC;

  OnPrvwmGetInfo(0, (LONG)(LPVOID)&PrvwInfo);

  m_hDCPrinter = hDCPrinter;
  lpSS->PrintOptions.pf.fMarginLeft = fPrintMarginLeft;
  lpSS->PrintOptions.pf.fMarginTop = fPrintMarginTop;
  lpSS->PrintOptions.pf.fMarginRight = fPrintMarginRight;
  lpSS->PrintOptions.pf.fMarginBottom = fPrintMarginBottom;

  *pPageCount = PrvwInfo.lPageEnd - PrvwInfo.lPageBeg + 1;
  UnlockSheet();
  return bRet;
}
#endif // SS_V35

//--------------------------------------------------------------------

#ifdef SS_V35
void CSsocxCtrl::ScriptCFGetCellParam(short Param, VARIANT FAR* Col, VARIANT FAR* Row)
{
  LPSPREADSHEET lpSS = LockSheet();
  VARIANT var1, var2;

  var1.vt = VT_I4;
  var2.vt = VT_I4;

  if( 1 <= Param && Param <= m_nCFArgCnt )
  {
    LPSS_VALUE lpArg = m_lpCFArgs + Param - 1;
    if( SS_VALUE_STATUS_OK == lpArg->Status )
    {
      if( SS_VALUE_TYPE_CELL == lpArg->Type )
      {
        var1.lVal = lpArg->Val.ValCell.Col;
        var2.lVal = lpArg->Val.ValCell.Row;
      }
      else if( SS_VALUE_TYPE_RANGE == lpArg->Type )
      {
        var1.lVal = lpArg->Val.ValRange.Col1;
        var2.lVal = lpArg->Val.ValRange.Row1;
      }
      if (Col)
        VariantCopy(Col, &var1);
      if (Row)
        VariantCopy(Row, &var2);
    }
  }
  UnlockSheet();
}
#endif

//--------------------------------------------------------------------

#ifdef SS_V70
void CSsocxCtrl::ScriptCFGetCellParamExt(short Param, VARIANT FAR* Sheet, VARIANT FAR* Col, VARIANT FAR* Row)
{
  LPSPREADSHEET lpSS = LockSheet();
  VARIANT var0, var1, var2;

  var0.vt = VT_I2;
  var1.vt = VT_I4;
  var2.vt = VT_I4;

  if( 1 <= Param && Param <= m_nCFArgCnt )
  {
    LPSS_VALUE lpArg = m_lpCFArgs + Param - 1;
    if( SS_VALUE_STATUS_OK == lpArg->Status )
    {
      if( SS_VALUE_TYPE_CELL == lpArg->Type )
      {
        var0.iVal = lpArg->Val.ValCell.nSheet;
        var1.lVal = lpArg->Val.ValCell.Col;
        var2.lVal = lpArg->Val.ValCell.Row;
      }
      else if( SS_VALUE_TYPE_RANGE == lpArg->Type )
      {
        var0.iVal = lpArg->Val.ValRange.nSheet;
        var1.lVal = lpArg->Val.ValRange.Col1;
        var2.lVal = lpArg->Val.ValRange.Row1;
      }
      if (Sheet)
        VariantCopy(Sheet, &var0);
      if (Col)
        VariantCopy(Col, &var1);
      if (Row)
        VariantCopy(Row, &var2);
    }
  }
  UnlockSheet();
}
#endif

//--------------------------------------------------------------------

#ifdef SS_V35
void CSsocxCtrl::ScriptCFGetDoubleParamExt(short Param, VARIANT FAR* ParamValue)
{
  LPSPREADSHEET lpSS = LockSheet();
  double dfRet = 0.0;
  VARIANT var;
  
  var.vt = VT_R8;

  if( 1 <= Param && Param <= m_nCFArgCnt )
  {
    LPSS_VALUE lpArg = m_lpCFArgs + Param - 1;
    if( SS_VALUE_STATUS_OK == lpArg->Status )
    {
      if( SS_VALUE_TYPE_DOUBLE == lpArg->Type )
        dfRet = lpArg->Val.ValDouble;
      else if( SS_VALUE_TYPE_LONG == lpArg->Type )
        dfRet = (double)lpArg->Val.ValLong;
      else if( SS_VALUE_TYPE_STR == lpArg->Type )
      {
        StringToFloat((LPTSTR)GlobalLock(lpArg->Val.hValStr), &dfRet);
        GlobalUnlock(lpArg->Val.hValStr);
      }
    }
  }
  UnlockSheet();
  var.dblVal = dfRet;
  if (ParamValue)
     VariantCopy(ParamValue, &var);
}
#endif

//--------------------------------------------------------------------

#ifdef SS_V35
BOOL CSsocxCtrl::ScriptCFGetParamInfo(short Param, VARIANT FAR* Type, VARIANT FAR* Status)
{
  LPSPREADSHEET lpSS = LockSheet();
  BOOL bRet = FALSE;
  VARIANT var1, var2;

  var1.vt = VT_I2;
  var2.vt = VT_I2;

  if( 1 <= Param && Param <= m_nCFArgCnt )
  {
    var1.iVal = m_lpCFArgs[Param-1].Type;
    var2.iVal = m_lpCFArgs[Param-1].Status;
    if( Type )
      VariantCopy(Type, &var1);
    if( Status )
      VariantCopy(Status, &var2);
    bRet = TRUE;
  }
  UnlockSheet();
  return bRet;
}
#endif

//--------------------------------------------------------------------

#ifdef SS_V35
void CSsocxCtrl::ScriptCFGetRangeParam(short Param, VARIANT FAR* Col, VARIANT FAR* Row, VARIANT FAR* Col2, VARIANT FAR* Row2)
{
  LPSPREADSHEET lpSS = LockSheet();
  VARIANT var1, var2, var3, var4;
  
  var1.vt = VT_I4;
  var2.vt = VT_I4;
  var3.vt = VT_I4;
  var4.vt = VT_I4;

  if( 1 <= Param && Param <= m_nCFArgCnt )
  {
    LPSS_VALUE lpArg = m_lpCFArgs + Param - 1;
    if( SS_VALUE_STATUS_OK == lpArg->Status )
    {
      if( SS_VALUE_TYPE_CELL == lpArg->Type )
      {
        var1.lVal = lpArg->Val.ValCell.Col;
        var2.lVal = lpArg->Val.ValCell.Row;
        var3.lVal = lpArg->Val.ValCell.Col;
        var4.lVal = lpArg->Val.ValCell.Row;
      }
      else if( SS_VALUE_TYPE_RANGE == lpArg->Type )
      {
        var1.lVal = lpArg->Val.ValRange.Col1;
        var2.lVal = lpArg->Val.ValRange.Row1;
        var3.lVal = lpArg->Val.ValRange.Col2;
        var4.lVal = lpArg->Val.ValRange.Row2;
      }
      if (Col)
        VariantCopy(Col, &var1);
      if (Row)
        VariantCopy(Row, &var2);
      if (Col2)
        VariantCopy(Col2, &var3);
      if (Row2)
        VariantCopy(Row2, &var4);
    }
  }
  UnlockSheet();
}
#endif

//--------------------------------------------------------------------

#ifdef SS_V70
void CSsocxCtrl::ScriptCFGetRangeParamExt(short Param, VARIANT FAR* Sheet, VARIANT FAR* Col, VARIANT FAR* Row, VARIANT FAR* Col2, VARIANT FAR* Row2)
{
  LPSPREADSHEET lpSS = LockSheet();
  VARIANT var0, var1, var2, var3, var4;
  
  var0.vt = VT_I2;
  var1.vt = VT_I4;
  var2.vt = VT_I4;
  var3.vt = VT_I4;
  var4.vt = VT_I4;

  if( 1 <= Param && Param <= m_nCFArgCnt )
  {
    LPSS_VALUE lpArg = m_lpCFArgs + Param - 1;
    if( SS_VALUE_STATUS_OK == lpArg->Status )
    {
      if( SS_VALUE_TYPE_CELL == lpArg->Type )
      {
        var0.iVal = lpArg->Val.ValCell.nSheet;
        var1.lVal = lpArg->Val.ValCell.Col;
        var2.lVal = lpArg->Val.ValCell.Row;
        var3.lVal = lpArg->Val.ValCell.Col;
        var4.lVal = lpArg->Val.ValCell.Row;
      }
      else if( SS_VALUE_TYPE_RANGE == lpArg->Type )
      {
        var0.iVal = lpArg->Val.ValRange.nSheet;
        var1.lVal = lpArg->Val.ValRange.Col1;
        var2.lVal = lpArg->Val.ValRange.Row1;
        var3.lVal = lpArg->Val.ValRange.Col2;
        var4.lVal = lpArg->Val.ValRange.Row2;
      }
      if (Sheet)
        VariantCopy(Sheet, &var0);
      if (Col)
        VariantCopy(Col, &var1);
      if (Row)
        VariantCopy(Row, &var2);
      if (Col2)
        VariantCopy(Col2, &var3);
      if (Row2)
        VariantCopy(Row2, &var4);
    }
  }
  UnlockSheet();
}
#endif

//--------------------------------------------------------------------

#ifdef SS_V35
void CSsocxCtrl::ScriptColWidthToTwips(float Width, VARIANT FAR* Twips)
{
  LPSPREADSHEET lpSS = LockSheet();
  long lPixels;
  HDC hDC;
  VARIANT var;

  if( lpSS->lpBook->hWnd )
    {
    lPixels = SS_ColWidthToPixels(lpSS, Width);
    // RFW - 3/19/00 GIC11495
    /*
    if (lPixels > 0)
       lPixels -= lpSS->dBorderSize;
    */
    hDC = ::GetDC(lpSS->lpBook->hWnd);

    var.vt = VT_I4;

    var.lVal = (1440 * lPixels) / ::GetDeviceCaps(hDC, LOGPIXELSX);

    if (Twips)
       VariantCopy(Twips, &var);

    ::ReleaseDC(lpSS->lpBook->hWnd, hDC);
    }
  else
    {
    var.vt = VT_EMPTY;
    if (Twips)
       VariantCopy(Twips, &var);
    }
  UnlockSheet();
}

//--------------------------------------------------------------------

void CSsocxCtrl::ScriptGetBottomRightCell(VARIANT FAR* Col, VARIANT FAR* Row)
{
  long lCol, lRow;
  LPSPREADSHEET lpSS = LockSheet();
  VARIANT var1, var2;

  var1.vt = VT_I4;
  var2.vt = VT_I4;

  SS_GetBottomRightCell(lpSS, &lCol, &lRow);
  SS_AdjustCellCoordsOut(lpSS, &lCol, &lRow);
  var1.lVal = lCol;
  var2.lVal = lRow;
  if (Col)
     VariantCopy(Col, &var1);
  if (Row)
     VariantCopy(Row, &var2);
  UnlockSheet();
}

void CSsocxCtrl::ScriptGetCellFromScreenCoord(VARIANT FAR* Col, VARIANT FAR* Row, long x, long y)
{
  LPSPREADSHEET lpSS = LockSheet();
  long lCol, lRow;
  VARIANT var1, var2;

  var1.vt = VT_I4;
  var2.vt = VT_I4;

  x = XTwipsToPixels(x);
  y = YTwipsToPixels(y);
  SS_GetCellFromPixel(lpSS, &lCol, &lRow, NULL, NULL, (int)x, (int)y);

  if( -2 == lCol || -2 == lRow )
    {
    var1.lVal = -1;
    var2.lVal = -1;
    }

  else
    {
    SS_AdjustCellCoordsOut(lpSS, &lCol, &lRow);

    var1.lVal = lCol;
    var2.lVal = lRow;
    }

  if (Col)
     VariantCopy(Col, &var1);
  if (Row)
     VariantCopy(Row, &var2);

  UnlockSheet();
}

BOOL CSsocxCtrl::ScriptGetCellPos(long Col, long Row, VARIANT FAR* x, VARIANT FAR* y, VARIANT FAR* Width, VARIANT FAR* Height)
{
  LPSPREADSHEET lpSS = LockSheet();
  RECT rect;
  BOOL bRet;

  SS_AdjustCellCoords(lpSS, &Col, &Row);
  bRet = SS_GetCellRect(lpSS, Col, Row, &rect);
  VARIANT var1, var2, var3, var4;

  var1.vt = VT_I4;
  var2.vt = VT_I4;
  var3.vt = VT_I4;
  var4.vt = VT_I4;

  var1.lVal = XPixelsToTwips(rect.left);
  var2.lVal = YPixelsToTwips(rect.top);
  var3.lVal = XPixelsToTwips(rect.right - rect.left);
  var4.lVal = YPixelsToTwips(rect.bottom - rect.top);
  if (x)
     VariantCopy(x, &var1);
  if (y)
     VariantCopy(y, &var2);
  if (Width)
     VariantCopy(Width, &var3);
  if (Height)
     VariantCopy(Height, &var4);

  UnlockSheet();
  return bRet;
}
void CSsocxCtrl::ScriptGetClientArea(VARIANT FAR* Width, VARIANT FAR* Height)
{
  LPSS_BOOK lpBook = LockBook();
  RECT rect;
  VARIANT var1, var2;

  var1.vt = VT_I4;
  var2.vt = VT_I4;
 
  SS_GetClientRect(lpBook, &rect);

  var1.lVal = XPixelsToTwips(rect.right - rect.left);
  var2.lVal = YPixelsToTwips(rect.bottom - rect.top);

  if (Width)
     VariantCopy(Width, &var1);
  if (Height)
     VariantCopy(Height, &var2);

  UnlockBook();
}
void CSsocxCtrl::ScriptGetFirstValidCell(VARIANT FAR* Col, VARIANT FAR* Row)
{
  LPSPREADSHEET lpSS = LockSheet();
  long lCol, lRow;
  VARIANT var1, var2;

  var1.vt = VT_I4;
  var2.vt = VT_I4;

  SS_GetFirstValidCell(lpSS, &lCol, &lRow);
  SS_AdjustCellCoordsOut(lpSS, &lCol, &lRow);

  var1.lVal = lCol;
  var2.lVal = lRow;

  if (Col)
     VariantCopy(Col, &var1);
  if (Row)
     VariantCopy(Row, &var2);

  UnlockSheet();
}
BOOL CSsocxCtrl::ScriptGetIteration(VARIANT FAR* MaxIterations, VARIANT FAR* MaxChange)
{
  LPSS_BOOK lpBook = LockBook();
  short nMaxIt;
  double dfMaxChange;
  BOOL bIter;
  VARIANT var1, var2;

  var1.vt = VT_I2;
  var2.vt = VT_R8;

  bIter = SS_GetIteration(lpBook, &nMaxIt, &dfMaxChange);

  var1.iVal = nMaxIt;
  var2.dblVal = dfMaxChange;

  if (MaxIterations)
     VariantCopy(MaxIterations, &var1);
  if (MaxChange)
     VariantCopy(MaxChange, &var2);

  UnlockBook();
  return bIter;
}
void CSsocxCtrl::ScriptGetLastValidCell(VARIANT FAR* Col, VARIANT FAR* Row)
{
  LPSPREADSHEET lpSS = LockSheet();
  long lCol, lRow;
  VARIANT var1, var2;

  var1.vt = VT_I4;
  var2.vt = VT_I4;

  SS_GetLastValidCell(lpSS, &lCol, &lRow);
  SS_AdjustCellCoordsOut(lpSS, &lCol, &lRow);

  var1.lVal = lCol;
  var2.lVal = lRow;

  if (Col)
     VariantCopy(Col, &var1);
  if (Row)
     VariantCopy(Row, &var2);

  UnlockSheet();
}
void CSsocxCtrl::ScriptRowHeightToTwips(long Row, float Height, VARIANT FAR* Twips)
{
  LPSPREADSHEET lpSS = LockSheet();
  long lPixels;
  HDC hDC;
  VARIANT var;

  if( lpSS->lpBook->hWnd )
  {
    SS_AdjustCellCoords(lpSS, NULL, &Row);
    lPixels = SS_RowHeightToPixels(lpSS, Row, Height);
    hDC = ::GetDC(lpSS->lpBook->hWnd);
    var.vt = VT_I4;

    var.lVal = (1440 * lPixels) / ::GetDeviceCaps(hDC, LOGPIXELSY);

    if (Twips)
       VariantCopy(Twips, &var);

    ::ReleaseDC(lpSS->lpBook->hWnd, hDC);
  }
  else
    {
    var.vt = VT_EMPTY;
    if (Twips)
       VariantCopy(Twips, &var);
    }

  UnlockSheet();
}
void CSsocxCtrl::ScriptTwipsToColWidth(long Twips, VARIANT FAR* ColWidth)
{
  LPSPREADSHEET lpSS = LockSheet();
  long lPixels;
  double dfColWidth = 0.0;
  HDC hDC;
  VARIANT var;

  if( lpSS->lpBook->hWnd )
  {
    hDC = ::GetDC(lpSS->lpBook->hWnd);
    lPixels = (Twips * ::GetDeviceCaps(hDC, LOGPIXELSX)) / 1440;
    ::ReleaseDC(lpSS->lpBook->hWnd, hDC);
    SS_CalcPixelsToColWidth(lpSS, 0, (int)lPixels, &dfColWidth);
  }
  var.vt = VT_R4;
  var.fltVal = (float)dfColWidth;
  if (ColWidth)
     VariantCopy(ColWidth, &var);

  UnlockSheet();
}
void CSsocxCtrl::ScriptTwipsToRowHeight(long Row, long Twips, VARIANT FAR* RowHeight)
{
  LPSPREADSHEET lpSS = LockSheet();
  long lPixels;
  double dfRowHeight = 0.0;
  HDC hDC;
  VARIANT var;

  if( lpSS->lpBook->hWnd )
  {
    hDC = ::GetDC(lpSS->lpBook->hWnd);
    lPixels = (Twips * ::GetDeviceCaps(hDC, LOGPIXELSY)) / 1440;
    ::ReleaseDC(lpSS->lpBook->hWnd, hDC);
    SS_AdjustCellCoords(lpSS, NULL, &Row);
    SS_CalcPixelsToRowHeight(lpSS, Row, (int)lPixels, &dfRowHeight);
  }

  var.vt = VT_R4;
  var.fltVal = (float)dfRowHeight;
  if (RowHeight)
     VariantCopy(RowHeight, &var);

  UnlockSheet();
}
BOOL CSsocxCtrl::ScriptGetActionKey(short Action, VARIANT FAR *Shift, VARIANT FAR *Ctrl, VARIANT FAR *Key)
{
  LPSS_BOOK lpBook = LockBook();
  BOOL bRet = FALSE;
  BOOL bShiftLocal;
  BOOL bCtrlLocal;
  WORD wKey;
  VARIANT var1, var2, var3;

  var1.vt = VT_BOOL;
  var2.vt = VT_BOOL;
  var3.vt = VT_I2;

  bRet = SS_GetActionKey(lpBook, Action, &bShiftLocal, &bCtrlLocal, &wKey);
  var1.boolVal = bShiftLocal ? VARIANT_TRUE : VARIANT_FALSE;
  var2.boolVal = bCtrlLocal ? VARIANT_TRUE : VARIANT_FALSE;
  var3.iVal = wKey;
  if (Shift)
     VariantCopy(Shift, &var1);
  if (Ctrl)
     VariantCopy(Ctrl, &var2);
  if (Key)
     VariantCopy(Key, &var3);

  UnlockBook();
  return (bRet);
}
BOOL CSsocxCtrl::ScriptGetOddEvenRowColor(VARIANT FAR *clrBackOdd, VARIANT FAR *clrForeOdd, VARIANT FAR *clrBackEven, VARIANT FAR *clrForeEven)
{
  LPSPREADSHEET lpSS = LockSheet();
  BOOL bRet = FALSE;
  COLORREF clrBackOddLocal, clrForeOddLocal, clrBackEvenLocal, clrForeEvenLocal;
  VARIANT var1, var2, var3, var4;

  var1.vt = VT_UI4;
  var2.vt = VT_UI4;
  var3.vt = VT_UI4;
  var4.vt = VT_UI4;

  bRet = SS_GetOddEvenRowColor(lpSS, &clrBackOddLocal, &clrForeOddLocal, &clrBackEvenLocal, &clrForeEvenLocal);

  var1.ulVal = clrBackOddLocal;
  var2.ulVal = clrForeOddLocal;
  var3.ulVal = clrBackEvenLocal;
  var4.ulVal = clrForeEvenLocal;

  if (clrBackOdd)
     VariantCopy(clrBackOdd, &var1);
  if (clrForeOdd)
     VariantCopy(clrForeOdd, &var2);
  if (clrBackEven)
     VariantCopy(clrBackEven, &var3);
  if (clrForeEven)
     VariantCopy(clrForeEven, &var4);

  UnlockSheet();
  return (bRet);
}

BOOL CSsocxCtrl::ScriptGetFloat(long Col, long Row, VARIANT FAR *Value)
{
  LPSPREADSHEET lpSS = LockSheet();
  //modify by BOC 99.7.13 (hyt)------------------------------
  //The GetFloat function returns the floating-point value of 
  //a cell if the cell type is float. Otherwise, the dfValue 
  //parameter returns the value of 0.0 and the function returns False
  BOOL bRet=FALSE;
  SS_CELLTYPE CellType;
  VARIANT var;

  var.vt = VT_R8;
  var.dblVal = 0.0;
  SS_AdjustCellCoords(lpSS, &Col, &Row);
  SS_RetrieveCellType(lpSS, &CellType, NULL, Col, Row);
  if(CellType.Type == SS_TYPE_FLOAT
#ifdef SS_V40
     || CellType.Type == SS_TYPE_CURRENCY
     || CellType.Type == SS_TYPE_NUMBER
     || CellType.Type == SS_TYPE_PERCENT
#endif // SS_V40
#ifdef SS_V70
     || CellType.Type == SS_TYPE_SCIENTIFIC
#endif // SS_V70
     )
	{
    double dfVal;
    bRet = SS_GetFloat(lpSS, Col, Row, &dfVal);
    var.dblVal = dfVal;
    }

  if (Value)
     VariantCopy(Value, &var);
  //---------------------------------------------------
  UnlockSheet();
  return (bRet);
}
BOOL CSsocxCtrl::ScriptGetInteger(long Col, long Row, VARIANT FAR *Value)
{
  LPSPREADSHEET lpSS = LockSheet();
  BOOL bRet = FALSE;
  long lVal; 
  VARIANT var;
  SS_CELLTYPE CellType;

  var.vt = VT_I4;

  SS_AdjustCellCoords(lpSS, &Col, &Row);
  SS_RetrieveCellType(lpSS, &CellType, NULL, Col, Row);
// fix for bug 9332 -scl
  if(CellType.Type == SS_TYPE_INTEGER || CellType.Type == SS_TYPE_COMBOBOX || CellType.Type == SS_TYPE_CHECKBOX || CellType.Type == SS_TYPE_FLOAT
#ifdef SS_V40
     || CellType.Type == SS_TYPE_NUMBER || CellType.Type == SS_TYPE_CURRENCY || CellType.Type == SS_TYPE_PERCENT
#endif // SS_V40
     )
    bRet = SS_GetInteger(lpSS, Col, Row, &lVal);
  var.lVal = lVal;
  if (Value)
     VariantCopy(Value, &var);

  UnlockSheet();
  return (bRet);
}
BOOL CSsocxCtrl::ScriptGetTextTipAppearance(VARIANT FAR *FontName, VARIANT FAR *FontSize, VARIANT FAR *FontBold, VARIANT FAR *FontItalic, VARIANT FAR *BackColor, VARIANT FAR *ForeColor)
{
  LPSPREADSHEET lpSS = LockSheet();
  FP_TT_INFO    Info;
  BOOL          bRet;
  VARIANT       var1, var2, var3, var4, var5, var6;

  var1.vt = VT_BSTR;
  var2.vt = VT_I2;
  var3.vt = VT_I2;
  var4.vt = VT_I2;
  var5.vt = VT_UI4;
  var6.vt = VT_UI4;

  bRet = fpTextTipGetInfo(m_hWnd, &Info);

  var1.bstrVal = (BSTR)fpSysAllocBSTRFromTString(Info.LogFont.lfFaceName);
  var2.iVal = (short)abs((int)PIXELS_TO_PT(Info.LogFont.lfHeight));
  var3.iVal = (Info.LogFont.lfWeight > FW_REGULAR);
  var4.iVal = Info.LogFont.lfItalic;
  var5.ulVal = Info.clrBack;
  var6.ulVal = Info.clrFore;

  if (FontName)
     VariantCopy(FontName, &var1);

  if (FontSize)
     VariantCopy(FontSize, &var2);

  if (FontBold)
     VariantCopy(FontBold, &var3);

  if (FontItalic)
     VariantCopy(FontItalic, &var4);

  if (BackColor)
     VariantCopy(BackColor, &var5);

  if (ForeColor)
     VariantCopy(ForeColor, &var6);

  UnlockSheet();
  return (bRet);
}
BOOL CSsocxCtrl::ScriptEnumCustomFunction(LPCTSTR PrevFuncName, VARIANT FAR* FuncName)
{
  LPSS_BOOK lpBook = LockBook();
  TCHAR szFuncName[CALC_MAX_NAME_LEN];
  int iRet;
  VARIANT var;

  var.vt = VT_BSTR;

  iRet = SS_EnumCustomFunction(lpBook, PrevFuncName, szFuncName, CALC_MAX_NAME_LEN);
  VariantClear(FuncName);
  /* RFW - 8/18/03
  if (FuncName->vt == VT_BSTR)
    SysFreeString(*(FuncName->pbstrVal));
  */
  var.bstrVal  = (BSTR)fpSysAllocBSTRFromTString(szFuncName);
  if (FuncName)
     VariantCopy(FuncName, &var);

  UnlockBook();
  return iRet > 0;
}
BOOL CSsocxCtrl::ScriptGetCustomFunction(LPCTSTR FuncName, VARIANT FAR* MinArgs, VARIANT FAR* MaxArgs, VARIANT FAR* Flags)
{
  LPSS_BOOK lpBook = LockBook();
  BOOL bRet;
  short nMinArgs, nMaxArgs;
  long lFlags;
  VARIANT var1, var2, var3;

  var1.vt = VT_I2;
  var2.vt = VT_I2;
  var3.vt = VT_I4;

  bRet = SS_GetCustomFunction(lpBook, FuncName, &nMinArgs, &nMaxArgs, &lFlags);

  var1.iVal = nMinArgs;
  var2.iVal = nMaxArgs;
  var3.lVal = lFlags;


  if (MinArgs)
     VariantCopy(MinArgs, &var1);
  if (MaxArgs)
     VariantCopy(MaxArgs, &var2);
  if (Flags)
     VariantCopy(Flags, &var3);
  
  UnlockBook();
  return bRet;
}
BOOL CSsocxCtrl::ScriptGetExcelSheetList (LPCTSTR FileName, VARIANT FAR& VarArray, VARIANT FAR* ListCount, LPCTSTR LogFileName, VARIANT FAR* WorkbookHandle, BOOL Replace)
{
  BOOL bRet = FALSE;
  short nListCnt;
  short nWkBookHandle;
  VARIANT var1, var2;

  var1.vt = VT_I2;
  var2.vt = VT_I2;

  bRet = (_GetSheetList(&VarArray, FileName, &nListCnt, LogFileName, &nWkBookHandle, Replace));
  var1.iVal = nListCnt;
  var2.iVal = nWkBookHandle;

  if (ListCount)
     VariantCopy(ListCount, &var1);
  if (WorkbookHandle)
     VariantCopy(WorkbookHandle, &var2);

  return bRet;
}
BOOL CSsocxCtrl::ScriptExportToXMLBuffer (LPCTSTR pszRoot, LPCTSTR pszCollection, VARIANT FAR *pbStrBuff, long lFlags, LPCTSTR lpszLogFile)
{
  BOOL bRet = FALSE;
  VARIANT var;

// GAB 05/02/02 - 9985  if (!m_fSafeForScripting)
// GAB  {	
     BSTR bstrBuff;
     bRet = ExportRangeToXMLBuffer (SS_ALLCOLS, SS_ALLROWS, 0, 0, pszRoot, pszCollection, &bstrBuff, lFlags, lpszLogFile);
     var.vt = VT_BSTR;
     var.bstrVal = bstrBuff;

     if (pbStrBuff)
        VariantCopy(pbStrBuff, &var);
// GAB  }

  return bRet;
}
BOOL CSsocxCtrl::ScriptExportRangeToXMLBuffer (SS_COORD col, SS_COORD row, SS_COORD col2, SS_COORD row2, LPCTSTR pszRoot, LPCTSTR pszCollection, VARIANT FAR *pbStrBuff, long lFlags, LPCTSTR lpszLogFile)
{
  LPSPREADSHEET lpSS;
  BOOL          bRet = FALSE; // GAB 05/02/02 
  HGLOBAL hBuf;
  LPTSTR pszBuf = NULL;
  VARIANT var;

  lpSS = LockSheet();
// GAB 05/02/02 - 9985  if (!m_fSafeForScripting)
// GAB  {	
    SS_AdjustCellRangeCoords(lpSS, &col, &row, &col2, &row2);
    hBuf = SS_ExportRangeToXMLBuffer (lpSS, col, row, col2, row2, pszRoot, pszCollection, lFlags, lpszLogFile);

	// Put the HGLOBAL in the BSTR
	//
	if (NULL != hBuf)
	{	
      pszBuf = (LPTSTR)GlobalLock (hBuf);
      var.vt = VT_BSTR;
#if _UNICODE
      BSTR ret = SysAllocString((OLECHAR*)pszBuf);
#else
      int len = lstrlen(pszBuf) + 1, newlen = len * sizeof(OLECHAR);
      BSTR ret = SysAllocStringByteLen(NULL,  newlen);
      memset(ret, 0, newlen);
      MultiByteToWideChar(CP_UTF8, 0, pszBuf, len, ret, newlen);
#endif
      var.bstrVal = ret;
	  if (pbStrBuff)
         VariantCopy(pbStrBuff, &var);
    else
      SysFreeString(ret);

	  GlobalUnlock (hBuf);
	  GlobalFree (hBuf);
    bRet = TRUE;
	}
// GAB  }
  UnlockSheet();

  return bRet;
}
BOOL CSsocxCtrl::ScriptOwnerPrintPageCount(OLE_HANDLE hDC, long Left, long Top, long Right, long Bottom, VARIANT FAR *pPageCount)
{
  LPSPREADSHEET lpSS = LockSheet();
  PRVW_INFO PrvwInfo;
  HDC hDCPrinter = m_hDCPrinter;
  float fPrintMarginLeft = lpSS->PrintOptions.pf.fMarginLeft;
  float fPrintMarginTop = lpSS->PrintOptions.pf.fMarginTop;
  float fPrintMarginRight = lpSS->PrintOptions.pf.fMarginRight;
  float fPrintMarginBottom = lpSS->PrintOptions.pf.fMarginBottom;
  short nHorzRes = GetDeviceCaps((HDC)hDC, HORZRES);
  short nVertRes = GetDeviceCaps((HDC)hDC, VERTRES);
  short nLogPixelsX = GetDeviceCaps((HDC)hDC, LOGPIXELSX);
  short nLogPixelsY = GetDeviceCaps((HDC)hDC, LOGPIXELSY);
  BOOL bRet = TRUE;
  VARIANT var;

  var.vt = VT_I2;

  lpSS->PrintOptions.pf.fMarginLeft += (float)Left / (float)1440.0;
  lpSS->PrintOptions.pf.fMarginTop += (float)Top / (float)1440.0;
  lpSS->PrintOptions.pf.fMarginRight += (float)(MulDiv((long)nHorzRes, 1440, nLogPixelsX) - Right) / (float)1440.0;
  lpSS->PrintOptions.pf.fMarginBottom += (float)(MulDiv((long)nVertRes, 1440, nLogPixelsY) - Bottom) / (float)1440.0;
  m_hDCPrinter = (HDC)hDC;

  OnPrvwmGetInfo(0, (LONG)(LPVOID)&PrvwInfo);

  m_hDCPrinter = hDCPrinter;
  lpSS->PrintOptions.pf.fMarginLeft = fPrintMarginLeft;
  lpSS->PrintOptions.pf.fMarginTop = fPrintMarginTop;
  lpSS->PrintOptions.pf.fMarginRight = fPrintMarginRight;
  lpSS->PrintOptions.pf.fMarginBottom = fPrintMarginBottom;

  var.iVal = PrvwInfo.lPageEnd - PrvwInfo.lPageBeg + 1;
  if (pPageCount)
     VariantCopy(pPageCount, &var);

  return bRet;
}
void CSsocxCtrl::ScriptGetCalTextOverride(VARIANT FAR *pShortDays, VARIANT FAR *pnLenShortDays, VARIANT FAR *pLongDays, VARIANT FAR *pnLenLongDays, VARIANT FAR *pShortMonths, VARIANT FAR *pnLenShortMonths, 
                                  VARIANT FAR *pLongMonths, VARIANT FAR *pnLenLongMonths, VARIANT FAR *pOkText, VARIANT FAR *pnLenOkText, VARIANT FAR *pCancelText, VARIANT FAR *pnLenCancelText)
{
  BSTR bstrSDays, bstrLDays, bstrSMonths, bstrLMonths;
  BSTR bstrOkText, bstrCancelText;
  short nLenSDays, nLenLDays, nLenSMonths, nLenLMonths, nLenOk, nLenCancel;

  VARIANT var1,var2,var3,var4,var5,var6,var7,var8,var9,var10,var11,var12;

  var1.vt = VT_BSTR;
  var2.vt = VT_I2;
  var3.vt = VT_BSTR;
  var4.vt = VT_I2;
  var5.vt = VT_BSTR;
  var6.vt = VT_I2;
  var7.vt = VT_BSTR;
  var8.vt = VT_I2;
  var9.vt = VT_BSTR;
  var10.vt = VT_I2;
  var11.vt = VT_BSTR;
  var12.vt = VT_I2;


  GetCalTextOverride(&bstrSDays, &nLenSDays,
                     &bstrLDays, &nLenLDays,
                     &bstrSMonths, &nLenSMonths,
                     &bstrLMonths, &nLenLMonths,
                     &bstrOkText, &nLenOk,
                     &bstrCancelText, &nLenCancel);

 var1.bstrVal = bstrSDays;
 var3.bstrVal = bstrLDays;
 var5.bstrVal = bstrSMonths;
 var7.bstrVal = bstrLMonths;
 var9.bstrVal = bstrOkText;
 var11.bstrVal = bstrCancelText;

 var2.iVal = nLenSDays;
 var4.iVal = nLenLDays;
 var6.iVal = nLenSMonths;
 var8.iVal = nLenLMonths;
 var10.iVal = nLenOk;
 var12.iVal = nLenCancel;

 if (pShortDays)
    VariantCopy(pShortDays, &var1);
 if (pnLenShortDays)
    VariantCopy(pnLenShortDays, &var2);
 if (pLongDays)
    VariantCopy(pLongDays, &var3);
 if (pnLenLongDays)
    VariantCopy(pnLenLongDays, &var4);
 if (pShortMonths)
    VariantCopy(pShortMonths, &var5);
 if (pnLenShortMonths)
    VariantCopy(pnLenShortMonths, &var6);
 if (pLongMonths)
    VariantCopy(pLongMonths, &var7);
 if (pnLenLongMonths)
    VariantCopy(pnLenLongMonths, &var8);
 if (pOkText)
    VariantCopy(pOkText, &var9);
 if (pnLenOkText)
    VariantCopy(pnLenOkText, &var10);
 if (pCancelText)
    VariantCopy(pCancelText, &var11);
 if (pnLenCancelText)
    VariantCopy(pnLenCancelText, &var12);

}

void CSsocxCtrl::PrintSheet(VARIANT *pvFlags)
{
  if( pvFlags && (pvFlags->vt == VT_I4 || S_OK == VariantChangeType(pvFlags, pvFlags, 0, VT_I4))
      && pvFlags->lVal & 0x01)
  {
	HGLOBAL hDevMode;
	LPDEVMODE lpDM;
    CPrintDialog PrnDlg(TRUE, PD_RETURNDC);
    BOOL saveDevMode = pvFlags->lVal & 0x02;
	LPSPREADSHEET lpSS = LockSheet();

     SS_COORD lCol = m_lCol;
     SS_COORD lRow = m_lRow;
     SS_COORD lCol2 = m_lCol2;
     SS_COORD lRow2 = m_lRow2;
	
	 if (saveDevMode)
	 {
  		hDevMode = GlobalAlloc(GMEM_MOVEABLE | GMEM_ZEROINIT, sizeof(DEVMODE));
		lpDM = (LPDEVMODE)GlobalLock(hDevMode);
		*(LPDEVMODE)lpDM = lpSS->lpBook->m_PrinterDevMode;
		PrnDlg.m_pd.hDevMode = hDevMode;
	 }

	 if (IDOK == PrnDlg.DoModal())
	  {
		HDC hDCPrinterOld = m_hDCPrinter;

		m_hDCPrinter = 0;

		LPDEVMODE lpDevMode = PrnDlg.GetDevMode();

		m_lCol = lCol;
		m_lRow = lRow;
		m_lCol2 = lCol2;
		m_lRow2 = lRow2;

      // RFW - 4/8/04 - This fixes a problem with some drivers.
		if ((lpDevMode->dmFields & DM_COPIES) && lpDevMode->dmCopies == 0)
			lpDevMode->dmCopies = 1;

		lstrcpyn(lpSS->lpBook->m_szPrinterDevice, PrnDlg.GetDeviceName(), sizeof(lpSS->lpBook->m_szPrinterDevice) / sizeof(TCHAR));
		lstrcpyn(lpSS->lpBook->m_szPrinterDriver, PrnDlg.GetDriverName(), sizeof(lpSS->lpBook->m_szPrinterDriver) / sizeof(TCHAR));
		lstrcpyn(lpSS->lpBook->m_szPrinterOutput, PrnDlg.GetPortName(), sizeof(lpSS->lpBook->m_szPrinterOutput) / sizeof(TCHAR));
		_fmemcpy(&(lpSS->lpBook->m_PrinterDevMode), lpDevMode, sizeof(lpSS->lpBook->m_PrinterDevMode));

		PrintSpread(SS_PRINT_ACTION_PRINT, 0, NULL, 0, NULL);

//		CDC::FromHandle(m_hDCPrinter)->DeleteDC();
		m_hDCPrinter = hDCPrinterOld;

		if (!saveDevMode)
		{
			_fmemset(lpSS->lpBook->m_szPrinterDevice, '\0', sizeof(lpSS->lpBook->m_szPrinterDevice));
			_fmemset(lpSS->lpBook->m_szPrinterDriver, '\0', sizeof(lpSS->lpBook->m_szPrinterDriver));
			_fmemset(lpSS->lpBook->m_szPrinterOutput, '\0', sizeof(lpSS->lpBook->m_szPrinterOutput));
			_fmemset(&(lpSS->lpBook->m_PrinterDevMode), '\0', sizeof(lpSS->lpBook->m_PrinterDevMode));
		}
	  }
	 else
	 {
		 //fire an event to notify user printer dialog was cancelled
		 ;
	 }
	
	if (saveDevMode)
	{
		GlobalUnlock(hDevMode);
		GlobalFree(hDevMode);
	}
	UnlockSheet();
  }

  else
	  SetActionPrint();
}

void CSsocxCtrl::AddSelection(long lCol, long lRow, long lCol2, long lRow2)
{
  if( m_hWnd )
  {
    SS_SELBLOCK block;

    block.UL.Col = lCol;
    block.UL.Row = lRow;
    block.LR.Col = lCol2;
    block.LR.Row = lRow2;
    SSAddMultiSelBlocks(m_hWnd, &block);
  }
}

void CSsocxCtrl::ClearRange(long lCol, long lRow, long lCol2, long lRow2, BOOL bDataOnly)
{
  LPSPREADSHEET lpSS = LockSheet();

  // RFW - 12/7/05 - 17832
  if (lRow >= SS_GetRowCnt(lpSS) || lCol >= SS_GetColCnt(lpSS))
    return;

  SS_AdjustCellRangeCoords(lpSS, &lCol, &lRow, &lCol2, &lRow2);
  if( bDataOnly )
    SS_ClearDataRange(lpSS, lCol, lRow, lCol2, lRow2, FALSE, FALSE);
  else
    SS_ClearRange(lpSS, lCol, lRow, lCol2, lRow2, FALSE);
  if( lpSS->Col.HeaderCnt > lCol || lpSS->Row.HeaderCnt > lRow )
    ControlInfoChanged();
  UnlockSheet();
}

void CSsocxCtrl::ClearSelection()
{
  LPSPREADSHEET lpSS = LockSheet();
  short nOpMode = SS_GetOperationMode(lpSS);

  if( SS_OPMODE_MULTISEL == nOpMode || SS_OPMODE_EXTSEL == nOpMode )
  {
    if( m_hWnd )
      SSSelModeSendMessage(m_hWnd, SS_SELMODE_CLEARSEL, 0L, 0L, 0L);
  }
  else    
    SS_DeSelectBlock(lpSS);
  UnlockSheet();
}

void CSsocxCtrl::ClipboardCopy()
{
  if( m_hWnd )
    SSClipboardCopy(m_hWnd);
}

void CSsocxCtrl::ClipboardCut()
{
  if( m_hWnd )
    SSClipboardCut(m_hWnd);
  ControlInfoChanged();
}

void CSsocxCtrl::ClipboardPaste()
{
  if( m_hWnd )
    SSClipboardPaste(m_hWnd);
  ControlInfoChanged();
}

void CSsocxCtrl::CopyRange(long lCol, long lRow, long lCol2, long lRow2, long lColDest, long lRowDest)
{
  LPSPREADSHEET lpSS = LockSheet();

  SS_AdjustCellRangeCoords(lpSS, &lCol, &lRow, &lCol2, &lRow2);
  SS_AdjustCellCoords(lpSS, &lColDest, &lRowDest);
  SS_CopySwapRange(lpSS, lCol, lRow, lCol2, lRow2, lColDest, lRowDest, SS_CMD_COPY, FALSE, TRUE);
  if( 0 == lColDest || 0 == lRowDest )
    ControlInfoChanged();
  UnlockSheet();
}

void CSsocxCtrl::DeleteCols(long lCol, long lNumCols)
{
  if( lNumCols > 0 )
  {
    LPSPREADSHEET lpSS = LockSheet();
    SSxDelColRange(lpSS, lCol, lCol == -1 ? lCol : lCol + lNumCols - 1);
  }
}

void CSsocxCtrl::DeleteRows(long lRow, long lNumRows)
{
  if( lNumRows > 0 )
  {
//DBOCX...
    LPSPREADSHEET lpSS = LockSheet();
    BOOL fDel = TRUE;

    SS_AdjustCellCoords(lpSS, NULL, &lRow);
    if( VBSSBoundIsBound((LONG)this, lpSS) )
    {
       if( VBSSBoundDel(lpSS, (LONG)this, m_hWnd, lRow, lRow + lNumRows - 1) )
          fDel = FALSE;
    }
    if (fDel)
       {
       SS_AdjustCellCoordsOut(lpSS, NULL, &lRow);
       SSxDelRowRange(lpSS, lRow, lRow == -1 ? lRow : lRow + lNumRows - 1);
       }

    UnlockSheet();
  }
}

void CSsocxCtrl::DataSave()
{
//DBOCX..
  LPVBSPREAD lpSpread = (LPVBSPREAD)SSOcxLockVBSpread((LONG)this);

  if( lpSpread )
  {
     LPSPREADSHEET lpSS = LockSheet();
     if( VBSSBoundIsBound((LONG)this, lpSS) && !lpSpread->DataOpts.fAutoSave )  // is this "if" needed?
        VBSSBoundManualSave(lpSS, (LONG)this, m_hWnd, lpSpread, lpSS->Row.HeaderCnt, -1, TRUE);
     UnlockSheet();
     SSOcxUnlockVBSpread((LONG)this);
  }
//DBOCX
}

#ifdef SS_V70
void CSsocxCtrl::GetCellBorder(long lCol, long lRow, long nIndex, VARIANT *pvColor, VARIANT *pvStyle)
{
  LPSPREADSHEET lpSS = LockSheet();
  WORD wStyle = 0;
  COLORREF crColor = 0;

  if( !pvColor  || !pvStyle )
  {
    UnlockSheet();
    return;
  }

  VariantInit(pvColor);
  VariantInit(pvStyle);

  SS_AdjustCellCoords(lpSS, &lCol, &lRow);
  if( (short)nIndex & SS_BORDERTYPE_LEFT )
     SS_GetBorderEx(lpSS, lCol, lRow,
                    &wStyle, &crColor, NULL, NULL, NULL, NULL,
                    NULL, NULL);
  else if( (short)nIndex & SS_BORDERTYPE_TOP )
     SS_GetBorderEx(lpSS, lCol, lRow,
                    NULL, NULL, &wStyle, &crColor, NULL, NULL,
                    NULL, NULL);
  else if( (short)nIndex & SS_BORDERTYPE_RIGHT )
     SS_GetBorderEx(lpSS, lCol, lRow,
                    NULL, NULL, NULL, NULL, &wStyle, &crColor,
                    NULL, NULL);
  else if( (short)nIndex & SS_BORDERTYPE_BOTTOM )
     SS_GetBorderEx(lpSS, lCol, lRow,
                    NULL, NULL, NULL, NULL, NULL, NULL,
                    &wStyle, &crColor);
  else
     SS_GetBorder(lpSS, lCol, lRow, NULL, &wStyle, &crColor);

  pvColor->vt = VT_I4;
  pvStyle->vt = VT_I4;
  pvColor->lVal = crColor;
  pvStyle->lVal = (long)wStyle;
  
  UnlockSheet();
}
#else
void CSsocxCtrl::GetCellBorder(long lCol, long lRow, short nIndex, VARIANT *pvColor, VARIANT *pvStyle)
{
  LPSPREADSHEET lpSS = LockSheet();
  WORD wStyle;
  COLORREF crColor;

  if( !pvColor  || !pvStyle )
  {
    UnlockSheet();
    return;
  }

  VariantInit(pvColor);
  VariantInit(pvStyle);

  SS_AdjustCellCoords(lpSS, &lCol, &lRow);
  if( nIndex & SS_BORDERTYPE_LEFT )
     SS_GetBorderEx(lpSS, lCol, lRow,
                    &wStyle, &crColor, NULL, NULL, NULL, NULL,
                    NULL, NULL);
  else if( nIndex & SS_BORDERTYPE_TOP )
     SS_GetBorderEx(lpSS, lCol, lRow,
                    NULL, NULL, &wStyle, &crColor, NULL, NULL,
                    NULL, NULL);
  else if( nIndex & SS_BORDERTYPE_RIGHT )
     SS_GetBorderEx(lpSS, lCol, lRow,
                    NULL, NULL, NULL, NULL, &wStyle, &crColor,
                    NULL, NULL);
  else if( nIndex & SS_BORDERTYPE_BOTTOM )
     SS_GetBorderEx(lpSS, lCol, lRow,
                    NULL, NULL, NULL, NULL, NULL, NULL,
                    &wStyle, &crColor);
  else
     SS_GetBorder(lpSS, lCol, lRow, NULL, &wStyle, &crColor);

  pvColor->vt = VT_I4;
  pvStyle->vt = VT_I4;
  pvColor->lVal = crColor;
  pvStyle->lVal = (long)wStyle;
  
  UnlockSheet();
}
#endif

void CSsocxCtrl::GetSelection(long lIndex, VARIANT *pvCol, VARIANT *pvRow, VARIANT *pvCol2, VARIANT *pvRow2)
{
  LPSPREADSHEET lpSS = LockSheet();
  SS_CELLCOORD cellUL, cellLR;
  short nOpMode = SS_GetOperationMode(lpSS);

  if( !pvCol || !pvRow || !pvCol2 || !pvRow2 )
  {
    UnlockSheet();
    return;
  }

  VariantInit(pvCol);
  VariantInit(pvRow);
  VariantInit(pvCol2);
  VariantInit(pvRow2);

  pvCol->vt = VT_I4;
  pvCol->lVal = -1;
  pvRow->vt = VT_I4;
  pvRow->lVal = -1;
  pvCol2->vt = VT_I4;
  pvCol2->lVal = -1;
  pvRow2->vt = VT_I4;
  pvRow2->lVal = -1;

  if( SS_OPMODE_NORMAL == nOpMode || SS_OPMODE_READONLY == nOpMode )
  {
    if( lpSS->lpBook->fAllowMultipleSelBlocks )
    {
      short nCnt;
      LPSS_SELBLOCK lpMultiSelBlocks;
      HGLOBAL hMultiSelBlocks = SS_GetMultiSelBlocks(lpSS, &nCnt);

      if( lpMultiSelBlocks = (LPSS_SELBLOCK)GlobalLock(hMultiSelBlocks) )
      {
        if( lIndex < nCnt )
        {
          pvCol->lVal = lpMultiSelBlocks[lIndex].UL.Col;
          pvRow->lVal = lpMultiSelBlocks[lIndex].UL.Row;
          pvCol2->lVal = lpMultiSelBlocks[lIndex].LR.Col;
          pvRow2->lVal = lpMultiSelBlocks[lIndex].LR.Row;
        }
        GlobalUnlock(hMultiSelBlocks);
        GlobalFree(hMultiSelBlocks);
      }

    }
    else if( SS_GetSelectBlock(lpSS, &cellUL, &cellLR) )
    {
      pvCol->lVal = cellUL.Col;
      pvRow->lVal = cellUL.Row;
      pvCol2->lVal = cellLR.Col;
      pvRow2->lVal = cellLR.Row;
    }
    else
    {
      SS_GetActiveCell(lpSS, &pvCol->lVal, &pvRow->lVal);
      pvCol2->lVal = pvCol->lVal;
      pvRow2->lVal = pvRow->lVal;
    }
  }
  else if( SS_OPMODE_SINGLESEL == nOpMode )
  {
    pvRow->lVal = pvRow2->lVal = SS_SelModeSendMessage(lpSS, SS_SELMODE_GETCURSEL, 0, 0, 0);
    pvCol->lVal = pvCol2->lVal = -1;
  }
  else if( SS_OPMODE_MULTISEL == nOpMode || SS_OPMODE_EXTSEL == nOpMode )
  {
    long lSelCnt = SS_SelModeSendMessage(lpSS, SS_SELMODE_GETSELCOUNT, 0, 0, 0);

    if( lIndex < lSelCnt )
    {
      long lSelRow = SS_SelModeSendMessage(lpSS, SS_SELMODE_GETSELITEM, -1, 0, 0);    
      while( lIndex > 0 )
      {
        lSelRow = SS_SelModeSendMessage(lpSS, SS_SELMODE_GETSELITEM, lSelRow, 0, 0);
        lIndex--;
      }
      pvRow->lVal = pvRow2->lVal = lSelRow;
      pvCol->lVal = pvCol2->lVal = -1;
    }
  }

  SS_AdjustCellCoordsOut(lpSS, &pvCol->lVal, &pvRow->lVal);
  SS_AdjustCellCoordsOut(lpSS, &pvCol2->lVal, &pvRow2->lVal);
  UnlockSheet();
}

void CSsocxCtrl::InsertCols(long lCol, long lNumCols)
{
  LPSPREADSHEET lpSS = LockSheet();
  long lCol2 = lCol == -1 ? lCol : lCol + lNumCols - 1;

  if( lNumCols > 0 )
  {
    if (lCol == SS_FIRST)
      lCol = lpSS->Col.HeaderCnt;
    else if (lCol == SS_LAST)
      lCol = lpSS->Col.AllocCnt;
    else
	   SS_AdjustCellCoords(lpSS, &lCol, NULL);

    if (lCol <= 0)
	 {
      UnlockSheet();
      return;
	 }

    if (lCol2 == SS_FIRST)
      lCol2 = lpSS->Col.HeaderCnt;
    else if (lCol2 == SS_LAST)
      lCol2 = lpSS->Col.AllocCnt;
    else
      SS_AdjustCellCoords(lpSS, &lCol2, NULL);

    if (lCol2 <= 0)
	 {
      UnlockSheet();
      return;
    }

    SS_InsColRange(lpSS, lCol, lCol2);
  }

  UnlockSheet();
}

void CSsocxCtrl::InsertRows(long lRow, long lNumRows)
{
  LPSPREADSHEET lpSS = LockSheet();
  long lRow2 = lRow == -1 ? lRow : lRow + lNumRows - 1;

  if( lNumRows > 0 )
  {
    if (lRow == SS_FIRST)
      lRow = lpSS->Row.HeaderCnt;
    else if (lRow == SS_LAST)
      lRow = lpSS->Row.AllocCnt;
    else
      SS_AdjustCellCoords(lpSS, NULL, &lRow);

    if (lRow <= 0)
    {
      UnlockSheet();
      return;
    }

    if (lRow2 == SS_FIRST)
      lRow2 = lpSS->Row.HeaderCnt;
    else if (lRow2 == SS_LAST)
      lRow2 = lpSS->Row.AllocCnt;
    else
      SS_AdjustCellCoords(lpSS, NULL, &lRow2);

    if (lRow2 <= 0)
    {
      UnlockSheet();
      return;
    }

    SS_InsRowRange(lpSS, lRow, lRow2);
  }

  UnlockSheet();
}

void CSsocxCtrl::MoveRange(long lCol, long lRow, long lCol2, long lRow2, long lColDest, long lRowDest)
{
  LPSPREADSHEET lpSS = LockSheet();

  SS_AdjustCellRangeCoords(lpSS, &lCol, &lRow, &lCol2, &lRow2);
  SS_AdjustCellCoords(lpSS, &lColDest, &lRowDest);
  SS_CopySwapRange(lpSS, lCol, lRow, lCol2, lRow2,
                   lColDest, lRowDest, SS_CMD_MOVE, FALSE, TRUE);
  if( 0 == lCol || 0 == lRow || 0 == lColDest || 0 == lRowDest )
    ControlInfoChanged();
  UnlockSheet();
}

void CSsocxCtrl::ReCalc()
{
  LPSPREADSHEET lpSS = LockSheet();

  SS_ReCalcAll(lpSS);
  UnlockSheet();
}

void CSsocxCtrl::Reset()
{
  if( m_hWnd )
  {
    SSReset(m_hWnd);
    SSSetOwner(m_hWnd, 0); // ???? is needed to prevent VBX bug ????
    DestroyPicts();
    InitOcx();
    // RFW - 5/28/04 - 14377
    SetBorderStyle(1);
    ControlInfoChanged();
  }
}

void CSsocxCtrl::SetActiveCell(long lCol, long lRow)
{
  LPSPREADSHEET lpSS = LockSheet();

  if( m_hWnd )
  {
    SS_COORD lRowOld = lpSS->Row.CurAt;

    SS_AdjustCellCoords(lpSS, &lCol, &lRow);
    SS_SetActiveCell(lpSS, lCol, lRow);
#ifdef SS_BOUNDCONTROL
    vbSpreadSaveBoundRow(lpSS, (long)this, m_hWnd, lRowOld,
                         lpSS->Row.CurAt, lpSS->Col.CurAt);
#endif
  }
  UnlockSheet();
}

#ifdef SS_V70
void CSsocxCtrl::SetCellBorder(long lCol, long lRow, long lCol2, long lRow2, long lIndex, OLE_COLOR crColor,  long lStyle)
{
  LPSPREADSHEET lpSS = LockSheet();

  SS_AdjustCellRangeCoords(lpSS, &lCol, &lRow, &lCol2, &lRow2);
  SS_SetBorderRange(lpSS, lCol, lRow, lCol2, lRow2, (short)lIndex, (short)lStyle, crColor);
  UnlockSheet();
}
#else
void CSsocxCtrl::SetCellBorder(long lCol, long lRow, long lCol2, long lRow2, short nIndex, OLE_COLOR crColor,  long lStyle)
{
  LPSPREADSHEET lpSS = LockSheet();

  SS_AdjustCellRangeCoords(lpSS, &lCol, &lRow, &lCol2, &lRow2);
  SS_SetBorderRange(lpSS, lCol, lRow, lCol2, lRow2, nIndex, (short)lStyle, crColor);
  UnlockSheet();
}
#endif

void CSsocxCtrl::SetSelection(long lCol, long lRow, long lCol2, long lRow2)
{
  LPSPREADSHEET lpSS = LockSheet();
  SS_CELLCOORD cellUL, cellLR;

  SS_AdjustCellRangeCoords(lpSS, &lCol, &lRow, &lCol2, &lRow2);
  cellUL.Col = lCol;
  cellUL.Row = lRow;
  cellLR.Col = lCol2;
  cellLR.Row = lRow2;
  SS_SetSelectBlock(lpSS, &cellUL, &cellLR);
  UnlockSheet();
}

void CSsocxCtrl::ShowCell(long lCol, long lRow, long lPosition)
{
  LPSPREADSHEET lpSS = LockSheet();

  SS_AdjustCellCoords(lpSS, &lCol, &lRow);
  SS_ShowCell(lpSS, lCol, lRow, (short)lPosition);
  UnlockSheet();
}

#ifdef SS_V40
BOOL CSsocxCtrl::Sort(long lCol, long lRow, long lCol2, long lRow2, long lSortBy, VARIANT *pSortKeys, VARIANT *pSortKeyOrders)
#else
void CSsocxCtrl::Sort(long lCol, long lRow, long lCol2, long lRow2, long lSortBy, VARIANT *pSortKeys, VARIANT *pSortKeyOrders)
#endif
{
  BOOL fRet = FALSE;

  if (m_hWnd)
  {
    LPSS_SORTKEY lpSortKeys = NULL;
    int nKeys = 0;
    BOOL bUnlockKeys = FALSE;

    if( pSortKeys && pSortKeys->vt == (VT_VARIANT | VT_BYREF ) )
      pSortKeys = pSortKeys->pvarVal;
    if( pSortKeyOrders && pSortKeyOrders->vt == (VT_VARIANT | VT_BYREF) )
      pSortKeyOrders = pSortKeyOrders->pvarVal;
    if( (pSortKeys == NULL || pSortKeys->vt == VT_EMPTY || pSortKeys->vt == VT_ERROR) &&
        (pSortKeyOrders == NULL || pSortKeyOrders->vt == VT_EMPTY || pSortKeyOrders->vt == VT_ERROR) )
    {
      if (m_hSortKeys && m_nSortKeyCnt > 0)
      {
        lpSortKeys = (LPSS_SORTKEY)GlobalLock(m_hSortKeys);
        nKeys = m_nSortKeyCnt;
        bUnlockKeys = TRUE;
      }
    }
    else if( pSortKeys->vt & VT_ARRAY && pSortKeyOrders->vt & VT_ARRAY )
    {
      SAFEARRAY FAR *lpSAKeys = NULL;
      SAFEARRAY FAR *lpSAOrders = NULL;
      long lbKeys, lbOrders, ubKeys, ubOrders;
      long lKeySize, lOrderSize;
      short nKeyDataType, nOrderDataType;

      if( pSortKeys->vt & VT_BYREF )
      {
        lpSAKeys = *pSortKeys->pparray;
        nKeyDataType = pSortKeys->vt & ~VT_ARRAY & ~VT_BYREF;
      }
      else
      {
        lpSAKeys = pSortKeys->parray;
        nKeyDataType = pSortKeys->vt & ~VT_ARRAY;
      }

      if( pSortKeyOrders->vt & VT_BYREF )
      {
        lpSAOrders = *pSortKeyOrders->pparray;
        nOrderDataType = pSortKeyOrders->vt & ~VT_ARRAY & ~VT_BYREF;
      }
      else
      {
        lpSAOrders = pSortKeyOrders->parray;
        nOrderDataType = pSortKeyOrders->vt & ~VT_ARRAY;
      }

      if( nKeyDataType != VT_VARIANT && nKeyDataType != VT_I4 && nKeyDataType != VT_I2 )
#ifdef SS_V40
        return (FALSE);
#else
        return;
#endif // SS_V40

      if( nOrderDataType != VT_VARIANT && nOrderDataType != VT_I4 && nOrderDataType != VT_I2 )
#ifdef SS_V40
        return (FALSE);
#else
        return;
#endif // SS_V40

      if( SafeArrayGetDim(lpSAKeys) != 1 || SafeArrayGetDim(lpSAOrders) != 1 )
#ifdef SS_V40
        return (FALSE);
#else
        return;
#endif // SS_V40

      SafeArrayGetLBound(lpSAKeys, 1, &lbKeys);
      SafeArrayGetUBound(lpSAKeys, 1, &ubKeys);
      SafeArrayGetLBound(lpSAOrders, 1, &lbOrders);
      SafeArrayGetUBound(lpSAOrders, 1, &ubOrders);

      if( lbKeys == lbOrders && ubKeys == ubOrders )
      {
        int i;
        long lIndex;
        void *pkey, *porder;

        lKeySize = (long)SafeArrayGetElemsize(lpSAKeys);
        lOrderSize = (long)SafeArrayGetElemsize(lpSAOrders);
        nKeys = ubKeys - lbKeys + 1;
        lpSortKeys = new SS_SORTKEY[nKeys];
        pkey = (void *)(new BYTE[lKeySize]);
        porder = (void *)(new BYTE[lOrderSize]);

        for( i = 0; i < nKeys; i++ )
        {
          lIndex = i + lbKeys;
          memset(pkey, 0, lKeySize);
          memset(porder, 0, lOrderSize);
          SafeArrayGetElement(lpSAKeys, &lIndex, pkey);
          SafeArrayGetElement(lpSAOrders, &lIndex, porder);
          if( nKeyDataType == VT_I4 )
            lpSortKeys[i].Reference = (long)*(long*)pkey;
          else if( nKeyDataType == VT_I2 )
            lpSortKeys[i].Reference = (long)*(short*)pkey;
          else if( S_OK == VariantChangeType((VARIANT*)pkey, (VARIANT*)pkey, 0, VT_I4) )
            lpSortKeys[i].Reference = ((VARIANT*)pkey)->lVal;
          if( nKeyDataType == VT_I4 )
            lpSortKeys[i].wOrder = (WORD)*(long*)porder;
          else if( nKeyDataType == VT_I2 )
            lpSortKeys[i].wOrder = (WORD)*(short*)porder;
          else if( S_OK == VariantChangeType((VARIANT*)porder, (VARIANT*)porder, 0, VT_I4) )
            lpSortKeys[i].wOrder = (WORD)((VARIANT*)porder)->lVal;
        }
        delete pkey;
        delete porder;
      }
    }
    else if( (pSortKeys->vt & VT_I4 || S_OK == VariantChangeType(pSortKeys, pSortKeys, 0, VT_I4)) && 
             (pSortKeyOrders->vt & VT_I4 || S_OK == VariantChangeType(pSortKeyOrders, pSortKeyOrders, 0, VT_I4)) )
    {
      nKeys = 1;
      lpSortKeys = new SS_SORTKEY;
      lpSortKeys->Reference = pSortKeys->vt & VT_BYREF ? *pSortKeys->plVal : pSortKeys->lVal;
      lpSortKeys->wOrder = (WORD)(pSortKeyOrders->vt & VT_BYREF ? *pSortKeyOrders->plVal : pSortKeyOrders->lVal);
    }
    if( lpSortKeys )
    {
      LPSPREADSHEET lpSS = LockSheet();
      fRet = SSSortEx(m_hWnd, lCol, lRow, lCol2, lRow2, (WORD)lSortBy, lpSortKeys, nKeys);
      if( bUnlockKeys )
        GlobalUnlock(m_hSortKeys);
      else
        delete lpSortKeys;
      if( fRet && VBSSBoundIsBound((LONG)this, lpSS) )
      {
        if( SSOcxLockVBSpread((LONG)this) )
        {
//         LPSPREADSHEET lpSS = LockSheet();
			SS_COORD Row;
         SS_GetActiveCell(lpSS, NULL, &Row);
         vbSpreadBoundClick(lpSS, (LONG)this, Row);
         SSOcxUnlockVBSpread((LONG)this);
//         UnlockSheet();
        }
      }
      UnlockSheet();
    }
  }

#ifdef SS_V40
  return (fRet);
#else
  return;
#endif

}

void CSsocxCtrl::SwapRange(long lCol, long lRow, long lCol2, long lRow2, long lColDest, long lRowDest)
{
  LPSPREADSHEET lpSS = LockSheet();

  SS_AdjustCellRangeCoords(lpSS, &lCol, &lRow, &lCol2, &lRow2);
  SS_AdjustCellCoords(lpSS, &lColDest, &lRowDest);
  SS_CopySwapRange(lpSS, lCol, lRow, lCol2, lRow2,
                   lColDest, lRowDest, SS_CMD_SWAP, FALSE, TRUE);
  if( lpSS->Col.HeaderCnt > lCol || lpSS->Row.HeaderCnt > lRow || lpSS->Col.HeaderCnt > lColDest || lpSS->Row.HeaderCnt > lRowDest )
    ControlInfoChanged();
  UnlockSheet();
}

void CSsocxCtrl::TypeComboBoxClear(long lCol, long lRow)
{
  if( m_hWnd )
    SSComboBoxSendMessage(m_hWnd, lCol, lRow, SS_CBM_RESETCONTENT, 0, 0L);
}

void CSsocxCtrl::TypeComboBoxRemoveItem(long lCol, long lRow, short nIndex)
{
  LPSPREADSHEET lpSS = LockSheet();

  if( m_hWnd )
	{
	  SS_CELLTYPE cellType;
	  BOOL bEditable = FALSE;

    SS_AdjustCellCoords(lpSS, &lCol, &lRow);
    SS_RetrieveCellType(lpSS, &cellType, NULL, lCol, lRow);
	  if( cellType.Type == SS_TYPE_COMBOBOX )
		  bEditable = cellType.Style & SS_CB_DROPDOWN ? TRUE : FALSE;

	  short nCount = (short)SS_ComboBoxSendMessage(lpSS, lCol, lRow,
                                         SS_CBM_GETCOUNT, 0, 0);
	  short nCurSel = (short)SS_ComboBoxSendMessage(lpSS, lCol, lRow,
                                         SS_CBM_GETCURSEL, 0, 0);
	  short nNewCount = (short)SS_ComboBoxSendMessage(lpSS, lCol, lRow, SS_CBM_DELETESTRING,
                              nIndex, 0L);
	  if( nNewCount < nCount && nIndex <= nCurSel && nCurSel >= 0 )
	  {
		  int nNewSel = nIndex < nCurSel ? nCurSel - 1 : -1;
		  if( !bEditable || (bEditable && nNewSel >= 0) )
			  SS_ComboBoxSendMessage(lpSS, lCol, lRow,
				  SS_CBM_SETCURSEL, nIndex < nCurSel ? nCurSel - 1 : -1, 0);
		  else
           SS_SetDataRange(lpSS, lCol, lRow, lCol, lRow, _T(""), TRUE, FALSE, TRUE);
	  }
	}
  UnlockSheet();
}

void CSsocxCtrl::VirtualRefresh()
{
  if( m_hWnd )
  {
#ifdef FP_ADB
// fpInitDataBinding calls SSVRefreshBuffer
#ifdef SS_V70
	LPSPREADSHEET lpSS = LockSheet();
	if( lpSS->pfpOleBinding )
	{
	  ((CfpOleBinding*)lpSS->pfpOleBinding)->fpReleaseDataBinding();
	  ((CfpOleBinding*)lpSS->pfpOleBinding)->fpInitDataBinding();
	}
   else
     SSVRefreshBuffer(m_hWnd); // RFW - 8/25/04 - 15078

	UnlockSheet();
#else
	m_fpOleBinding.fpReleaseDataBinding();
	m_fpOleBinding.fpInitDataBinding();
#endif
#else
	// RFW - 8/6/04 - 15004
   SSVRefreshBuffer(m_hWnd);
#endif
  }
}

#endif // SS_V35
/**/
#ifdef SS_V40

BOOL CSsocxCtrl::AddCellSpan(long lCol, long lRow, long lNumCols, long lNumRows)
{
  LPSPREADSHEET lpSS = LockSheet();
  BOOL fRet;

  SS_AdjustCellCoords(lpSS, &lCol, &lRow);
  fRet = SS_AddCellSpan(lpSS, lCol, lRow, lNumCols, lNumRows);
  ControlInfoChanged();
  UnlockSheet();
  return fRet;
}


short CSsocxCtrl::GetCellSpan(long lCol, long lRow, VARIANT *pvColAnchor, VARIANT *pvRowAnchor,
                              VARIANT *pvNumCols, VARIANT *pvNumRows)
{
  LPSPREADSHEET lpSS = LockSheet();
  long lNumCols = 0, lNumRows = 0, lColAnchor = -1, lRowAnchor = -1;
  short nRet;

  SS_AdjustCellCoords(lpSS, &lCol, &lRow);
  nRet = (short)SS_GetCellSpan(lpSS, lCol, lRow, &lColAnchor, &lRowAnchor, &lNumCols, &lNumRows);

  SS_AdjustCellCoordsOut(lpSS, &lColAnchor, &lRowAnchor);

  VariantInit(pvColAnchor);
  pvColAnchor->vt = VT_I4;
  pvColAnchor->lVal = lColAnchor;

  VariantInit(pvRowAnchor);
  pvRowAnchor->vt = VT_I4;
  pvRowAnchor->lVal = lRowAnchor;

  VariantInit(pvNumCols);
  pvNumCols->vt = VT_I4;
  pvNumCols->lVal = lNumCols;

  VariantInit(pvNumRows);
  pvNumRows->vt = VT_I4;
  pvNumRows->lVal = lNumRows;

  UnlockSheet();
  return nRet;
}


void CSsocxCtrl::RemoveCellSpan(long lCol, long lRow)
{
  LPSPREADSHEET lpSS = LockSheet();

  SS_AdjustCellCoords(lpSS, &lCol, &lRow);
  SS_RemoveCellSpan(lpSS, lCol, lRow);
  ControlInfoChanged();
  UnlockSheet();
}

long CSsocxCtrl::GetColFromID(LPCTSTR ColID)
{
  LPSPREADSHEET lpSS = LockSheet();
  SS_COORD lRet;

  lRet = SS_GetColFromID(lpSS, ColID);
  SS_AdjustCellCoordsOut(lpSS, &lRet, NULL);

  UnlockSheet();
  return (lRet);
}

void CSsocxCtrl::CopyColRange(long lCol, long lCol2, long lColDest)
{
  CopyRange(lCol, SS_ALLROWS, lCol2, SS_ALLROWS, lColDest, SS_ALLROWS);
}

void CSsocxCtrl::CopyRowRange(long lRow, long lRow2, long lRowDest)
{
  CopyRange(SS_ALLCOLS, lRow, SS_ALLCOLS, lRow2, SS_ALLCOLS, lRowDest);
}

void CSsocxCtrl::MoveColRange(long lCol, long lCol2, long lColDest)
{
  MoveRange(lCol, SS_ALLROWS, lCol2, SS_ALLROWS, lColDest, SS_ALLROWS);
}

void CSsocxCtrl::MoveRowRange(long lRow, long lRow2, long lRowDest)
{
  MoveRange(SS_ALLCOLS, lRow, SS_ALLCOLS, lRow2, SS_ALLCOLS, lRowDest);
}

void CSsocxCtrl::SwapColRange(long lCol, long lCol2, long lColDest)
{
  SwapRange(lCol, SS_ALLROWS, lCol2, SS_ALLROWS, lColDest, SS_ALLROWS);
}

void CSsocxCtrl::SwapRowRange(long lRow, long lRow2, long lRowDest)
{
  SwapRange(SS_ALLCOLS, lRow, SS_ALLCOLS, lRow2, SS_ALLCOLS, lRowDest);
}

long CSsocxCtrl::SearchCol(long lCol, long lRowStart, long lRowEnd, LPCTSTR Text, short SearchFlags)
{
  LPSPREADSHEET lpSS = LockSheet();
  SS_COORD lRet;

  SS_AdjustCellRangeCoords(lpSS, &lCol, &lRowStart, NULL, &lRowEnd);
  lRet = SS_SearchCol(lpSS, lCol, lRowStart, lRowEnd, Text, SearchFlags);
  SS_AdjustCellCoordsOut(lpSS, NULL, &lRet);

  UnlockSheet();
  return (lRet);
}

long CSsocxCtrl::SearchRow(long lRow, long lColStart, long lColEnd, LPCTSTR Text, short SearchFlags)
{
  LPSPREADSHEET lpSS = LockSheet();
  SS_COORD lRet;

  SS_AdjustCellRangeCoords(lpSS, &lColStart, &lRow, &lColEnd, NULL);
  lRet = SS_SearchRow(lpSS, lRow, lColStart, lColEnd, Text, SearchFlags);
  SS_AdjustCellCoordsOut(lpSS, &lRet, NULL);

  UnlockSheet();
  return (lRet);
}

#endif // SS_V40


#ifdef SS_V70
long CSsocxCtrl::ColLetterToNumber(LPCTSTR ColLetter) 
{
  return (SS_ColLetterToNumber(ColLetter));
}

void CSsocxCtrl::DataRefresh(void)
{
  LPSPREADSHEET lpSS = LockSheet();

  SS_DataRefresh(lpSS, (LONG)this);
  UnlockSheet();
}

BOOL CSsocxCtrl::ImportExcelBook (LPCTSTR fileName, LPCTSTR LogFileName)
{
	LPSS_BOOK    lpBook = LockBook();
//  GLOBALHANDLE ghList = (GLOBALHANDLE)0;
//  short        listCount = 0;
  BOOL         fRet = FALSE;
//  LPSSXL       lpXL = NULL;
//  short        nWorkbookId = 0;

//  fRet = SS_GetExcelSheetList(lpBook, fileName, &ghList, &listCount, LogFileName, &nWorkbookId, TRUE);  
//  fRet = SS_LoadXLBook(lpBook, nWorkbookId);
  fRet = SS_LoadXLBook(lpBook, fileName, LogFileName);

// hatakeyama@boc.co.jp 11/xx-2000 for Setting Printer 
// Reflect print settings to SPREAD sheet.
   /*
	{
		((CSsocxCtrl FAR*)lpBook->lpOleControl)->m_bPrintGrid = lpBook->PrintOptions.pf.fShowGrid;
		((CSsocxCtrl FAR*)lpBook->lpOleControl)->m_bPrintColHeaders = lpBook->PrintOptions.pf.fShowColHeaders;
		((CSsocxCtrl FAR*)lpBook->lpOleControl)->m_bPrintRowHeaders = lpBook->PrintOptions.pf.fShowRowHeaders;
		((CSsocxCtrl FAR*)lpBook->lpOleControl)->m_bPrintBorder = lpBook->PrintOptions.pf.fDrawBorder;
		((CSsocxCtrl FAR*)lpBook->lpOleControl)->m_nPrintOrientation  = lpBook->PrintOptions.pf.wOrientation;
		((CSsocxCtrl FAR*)lpBook->lpOleControl)->m_bPrintColor   = lpBook->PrintOptions.pf.fDrawColors;
		((CSsocxCtrl FAR*)lpBook->lpOleControl)->m_bPrintSmartPrint = FALSE;  
	}
   */

//Add by BOC 99.7.23 (hyt)----------------------------
//for draw selblock error after import excel file
  LPSPREADSHEET lpSS = SS_BookLockSheetIndex(lpBook, 0);//sSheetNum);
  if(lpSS->fMultipleBlocksSelected)
  	lpBook->fSelBlockInvisible = 1;
  SS_BookUnlockSheetIndex(lpBook, 0); //sSheetNum);
//----------------------------------------------------

  UnlockBook();
  return !fRet;
}

BOOL CSsocxCtrl::InsertSheets(short SheetIndex, short SheetCnt)
{
  LPSS_BOOK lpBook = LockBook();
  BOOL fRet = FALSE;

  if (SheetIndex == -1 || SheetIndex > 0)
    fRet = SS_InsertSheets(lpBook, SheetIndex == -1 ? -1 : SheetIndex - 1, SheetCnt);

  UnlockBook();
  return (fRet);
}

BOOL CSsocxCtrl::DeleteSheets(short SheetIndex, short SheetCnt)
{
  LPSS_BOOK lpBook = LockBook();
  BOOL fRet = FALSE;

  if (SheetIndex == -1 || SheetIndex > 0)
  {
    fRet = SS_DeleteSheets(lpBook, SheetIndex == -1 ? -1 : SheetIndex - 1, SheetCnt);

    if (lpBook->nSheetCnt == 0)
      SS_SetSheetCount(lpBook, 1);
  }

  UnlockBook();
  return (fRet);
}

BOOL CSsocxCtrl::MoveSheets(short SheetSrc, short SheetCnt, short SheetDest)
{
  LPSS_BOOK lpBook = LockBook();

  if (SheetSrc == 0 || SheetSrc < -1 || SheetDest == 0 || SheetDest < -1 || SheetCnt == 0)
    return (FALSE);

  BOOL fRet = SS_MoveSheets(lpBook, SheetSrc == -1 ? -1 : SheetSrc - 1, SheetCnt,
                            SheetDest == -1 ? -1 : SheetDest - 1);
  UnlockBook();
  return (fRet);
}

BOOL CSsocxCtrl::ResetSheet(short Sheet)
{
  LPSS_BOOK lpBook = LockBook();
  BOOL fRet = SS_ResetSheet(lpBook, (short)(Sheet - 1));
  UnlockBook();
  return (fRet);
}

IPictureDisp* CSsocxCtrl::LoadPicture(LPCTSTR lpszFileName, short nPictType)
{
  IPictureDisp* pRet;
  char path[MAX_PATH+1] = {0};
#if _UNICODE
  WideCharToMultiByte(CP_ACP, 0, lpszFileName, lstrlen(lpszFileName), (char*)path, MAX_PATH, NULL, NULL);
#else
  strncpy(path, lpszFileName, __min(strlen(lpszFileName), MAX_PATH));
#endif
#ifdef _DEBUG
  HINSTANCE hInstDll = LoadLibrary(_T("fpimaged.dll"));
#else
  HINSTANCE hInstDll = LoadLibrary(_T("fpimage.dll"));
#endif
  FPLOADIMAGEPROC pFPLoadImage = hInstDll ? (FPLOADIMAGEPROC)GetProcAddress(hInstDll, "FPLoadImage") : NULL;
  ISBITMAPPROC pIsBitmap = hInstDll ? (ISBITMAPPROC)GetProcAddress(hInstDll, "IsBitmap") : NULL;
  ISICONPROC pIsIcon = hInstDll ? (ISICONPROC)GetProcAddress(hInstDll, "IsIcon") : NULL;
  if( !pFPLoadImage )
  {
//    ::MessageBox(NULL, DLL_NOT_FOUND, _T("FarPoint Spread"), MB_OK);
    return NULL;
  }
  HANDLE hPict = pFPLoadImage(path, nPictType);
  pRet = (IPictureDisp*)CreateOcxPict(hPict, pIsBitmap(hPict), pIsIcon(hPict));
  FreeLibrary(hInstDll);
  return pRet;
}

IPictureDisp* CSsocxCtrl::LoadPictureBuffer(VARIANT* Buffer, VARIANT* lSize, short nPictType)
{
  IPictureDisp* pRet;
  HANDLE hPict = NULL;
  if( lSize && Buffer && Buffer->vt == VT_BSTR && Buffer->bstrVal )
  {
	  if( lSize->vt != VT_I4 )
	  {
		  if( !VariantChangeType(lSize, lSize, 0, VT_I4) )
			  return false;
	  }
#ifdef _DEBUG
    HINSTANCE hInstDll = LoadLibrary(_T("fpimaged.dll"));
#else
    HINSTANCE hInstDll = LoadLibrary(_T("fpimage.dll"));
#endif
    FPLOADIMAGEBUFFERPROC pFPLoadImageBuffer = hInstDll ? (FPLOADIMAGEBUFFERPROC)GetProcAddress(hInstDll, "FPLoadImageBuffer") : NULL;
    ISBITMAPPROC pIsBitmap = hInstDll ? (ISBITMAPPROC)GetProcAddress(hInstDll, "IsBitmap") : NULL;
    ISICONPROC pIsIcon = hInstDll ? (ISICONPROC)GetProcAddress(hInstDll, "IsIcon") : NULL;
    if( !pFPLoadImageBuffer )
    {
//      ::MessageBox(NULL, DLL_NOT_FOUND, _T("FarPoint Spread"), MB_OK);
      return NULL;
    }
    HGLOBAL hGlobal = GlobalAlloc(GHND, lSize->lVal);
    LPBYTE pTemp = (LPBYTE)GlobalLock(hGlobal);
    
    memcpy(pTemp, Buffer->bstrVal, lSize->lVal);
    GlobalUnlock(hGlobal);
    hPict = pFPLoadImageBuffer(hGlobal, lSize->lVal, nPictType);
    GlobalFree(hGlobal);
    pRet = (IPictureDisp*)CreateOcxPict(hPict, pIsBitmap(hPict), pIsIcon(hPict));
    FreeLibrary(hInstDll);
    return pRet;
  }
  return NULL;
}

IPictureDisp* CSsocxCtrl::LoadResPicture(long hInstance, LPCTSTR lpszResName, LPCTSTR lpszResType, short nPictType)
{
  IPictureDisp* pRet;
  char name[MAX_PATH+1] = {0};
  char type[MAX_PATH+1] = {0};
#if _UNICODE
  WideCharToMultiByte(CP_ACP, 0, lpszResName, lstrlen(lpszResName), (char*)name, MAX_PATH, NULL, NULL);
  WideCharToMultiByte(CP_ACP, 0, lpszResType, lstrlen(lpszResType), (char*)type, MAX_PATH, NULL, NULL);
#else
  strncpy(name, lpszResName, __min(strlen(lpszResName), MAX_PATH));
  strncpy(type, lpszResType, __min(strlen(lpszResType), MAX_PATH));
#endif
#ifdef _DEBUG
  HINSTANCE hInstDll = LoadLibrary(_T("fpimaged.dll"));
#else
  HINSTANCE hInstDll = LoadLibrary(_T("fpimage.dll"));
#endif
  FPLOADRESIMAGEPROC pFPLoadResImage = hInstDll ? (FPLOADRESIMAGEPROC)GetProcAddress(hInstDll, "FPLoadResImage") : NULL;
  ISBITMAPPROC pIsBitmap = hInstDll ? (ISBITMAPPROC)GetProcAddress(hInstDll, "IsBitmap") : NULL;
  ISICONPROC pIsIcon = hInstDll ? (ISICONPROC)GetProcAddress(hInstDll, "IsIcon") : NULL;
  if( !pFPLoadResImage )
  {
//    ::MessageBox(NULL, DLL_NOT_FOUND, _T("FarPoint Spread"), MB_OK);
    return NULL;
  }
  HANDLE hPict = pFPLoadResImage((HINSTANCE)hInstance, name, type, nPictType);
  pRet = (IPictureDisp*)CreateOcxPict(hPict, pIsBitmap(hPict), pIsIcon(hPict));
  FreeLibrary(hInstDll);
  return pRet;
}

BOOL CSsocxCtrl::SavePicture(IPictureDisp* pPicture, LPCTSTR lpszFileName, short nPictType)
{
  BOOL ret = FALSE;

	if (!m_fSafeForScripting)
  {
    OLE_HANDLE hPict = NULL;
    OLE_HANDLE hPal = NULL;
    IPicture* pPict;
    short nType;

    if( !pPicture )
      return FALSE;
    if( FAILED(pPicture->QueryInterface(IID_IPicture, (void**)&pPict)) )
      return FALSE;
    if( S_OK == pPict->get_Type(&nType) && (nType == PICTYPE_ICON || nType == PICTYPE_BITMAP) )
    {
      char path[MAX_PATH+1] = {0};
#if _UNICODE
      WideCharToMultiByte(CP_ACP, 0, lpszFileName, lstrlen(lpszFileName), (char*)path, MAX_PATH, NULL, NULL);
#else
      strncpy(path, lpszFileName, __min(strlen(lpszFileName), MAX_PATH));
#endif
      pPict->get_Handle(&hPict);
      pPict->get_hPal(&hPal);
#ifdef _DEBUG
      HINSTANCE hInstDll = LoadLibrary(_T("fpimaged.dll"));
#else
      HINSTANCE hInstDll = LoadLibrary(_T("fpimage.dll"));
#endif
      FPSAVEIMAGEPROC pFPSaveImage = hInstDll ? (FPSAVEIMAGEPROC)GetProcAddress(hInstDll, "FPSaveImage") : NULL;
      if( !pFPSaveImage )
      {
//        ::MessageBox(NULL, DLL_NOT_FOUND, _T("FarPoint Spread"), MB_OK);
        return FALSE;
      }
      ret = pFPSaveImage((HANDLE)hPict, (HPALETTE)hPal, path, nPictType);
      FreeLibrary(hInstDll);
    }
    pPict->Release();
  }
  return ret;
}

BOOL CSsocxCtrl::SavePictureBuffer(IPictureDisp* pPicture, short nPictType, VARIANT *pBuffer, VARIANT *plSize)
{
  BOOL ret = FALSE;
  HGLOBAL hGlobal;
  LPBYTE pTemp;

	if (!m_fSafeForScripting)
  {
    OLE_HANDLE hPict = NULL;
    OLE_HANDLE hPal = NULL;
    IPicture* pPict;
    short nType;
    if( !pPicture || !pBuffer || !plSize )
      return FALSE;
	pBuffer->vt = VT_BSTR;
	plSize->vt = VT_I4;
    if( FAILED(pPicture->QueryInterface(IID_IPicture, (void**)&pPict)) )
      return FALSE;
    if( S_OK == pPict->get_Type(&nType) && (nType == PICTYPE_ICON || nType == PICTYPE_BITMAP) )
    {
#ifdef _DEBUG
      HINSTANCE hInstDll = LoadLibrary(_T("fpimaged.dll"));
#else
      HINSTANCE hInstDll = LoadLibrary(_T("fpimage.dll"));
#endif
      FPSAVEIMAGEBUFFERPROC pFPSaveImageBuffer = hInstDll ? (FPSAVEIMAGEBUFFERPROC)GetProcAddress(hInstDll, "FPSaveImageBuffer") : NULL;
      if( !pFPSaveImageBuffer )
      {
//        ::MessageBox(NULL, DLL_NOT_FOUND, _T("FarPoint Spread"), MB_OK);
        return FALSE;
      }
      pPict->get_Handle(&hPict);
      pPict->get_hPal(&hPal);
      ret = pFPSaveImageBuffer((HANDLE)hPict, (HPALETTE)hPal, nPictType, &hGlobal, &plSize->lVal);
      FreeLibrary(hInstDll);
      if( ret )
      {
        pBuffer->bstrVal = SysAllocStringByteLen(NULL, plSize->lVal);
        pTemp = (LPBYTE)GlobalLock(hGlobal);
        memcpy(pBuffer->bstrVal, pTemp, plSize->lVal);
        GlobalUnlock(hGlobal);
        GlobalFree(hGlobal);
      }
    }
    pPict->Release();
  }
  return ret;
}
#endif // SS_V70

//--------------------------------------------------------------------
