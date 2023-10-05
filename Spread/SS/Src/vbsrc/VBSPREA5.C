/*********************************************************
* VBSPREA5.C
*
* Copyright (C) 1991-1993 - FarPoint Technologies
* All Rights Reserved.
*
* No part of this source code may be copied, modified or
* reproduced in any form without retaining the above
* copyright notice.  This source code, or source code
* derived from it, may not be redistributed without
* express written permission of FarPoint Technologies.
*********************************************************/
#if ((defined(SS_OCX) || defined(FP_DLL)) && defined(WIN32))
#define WIN32_LEAN_AND_MEAN   //exclude conflicts with VBAPI.H in WIN32
#endif

#include <windows.h>
#include "fptools.h"
// this define has been added to the Spread 3.5 projects
//#ifdef SS_V35
//#define TT_V2
//#endif
#include "texttip.h"

#include "..\vbsrc\vbsscnvt.h"                                  //DBOCX

#include <math.h>
#include <string.h>
#include "..\sssrc\spread\spread.h"
#include "..\sssrc\spread\ss_alloc.h"
#include "..\sssrc\spread\ss_draw.h"
#include "..\sssrc\spread\ss_main.h"
#include "..\sssrc\spread\ss_virt.h"
#include "..\sssrc\spread\ss_row.h"
#include "vbspread.h"
//#include "vbsprea5.h"    //DBOCX
#ifdef SS_QE         //DBOCX
#include "db_bind.h"
#include "db_sdk.h"
#endif

// Macros
#ifdef SS_OCX
#include "..\ssocx\ssocxdb.h"   //DBOCX (this file can be included in C files)
#include "..\..\..\fplibs\fptools\src\fptstr.h"
#define SS_HCTL                        long
#define VBGetHwndControl(hWnd)         hCtl
#define VBDerefControl(hctl)           SSOcxVBDerefControl(hctl)
#define VBDefControlProc(hCtl, hWnd, Msg, wParam, lParam)    0
#define DataCallPrimative(lpda, msg, action, row, col, lpSS) \
  SSOcxDataCall( lObject, lpda, msg, action, (SS_COORD)row, (SS_COORD)col, lpSS)
#define DataCall(lpda, msg, action, lpSS)                \
  DataCallPrimative(lpda, VBM_DATA_##msg, DATA_##action, -1, -1, lpSS)
#define DataCallwErrCoord(lpda, msg, action, row, col, lpSS) \
  DataCallPrimative(lpda, VBM_DATA_##msg, DATA_##action, row, col, lpSS)
#else  //not SS_OCX

#include "vbsprea5.h"
#define  SS_HCTL  HCTL
#endif //not SS_OCX

HWND            VBSS_GethWnd(HCTL hCtl);


#ifndef SS_OCX           //DBOCX

typedef unsigned int size_t;
void _far * _far _cdecl _fmemcpy(void _far *, const void _far *, size_t);

long            __SpreadGetTextString(HWND hWnd, LPSS_CELLTYPE lpCellType,
                                      SS_COORD Col, SS_COORD Row, BOOL fHLSTR);
void            __SpreadSetItemData(HWND hWnd, long Col, long Row,
                                    long lValue);
long            __SpreadGetItemData(HWND hWnd, long Col, long Row);
#ifdef SS_OLDCALC
BOOL FAR PASCAL _export SpreadCustomFunctionProc(HWND hSS, LPTSTR lpszFunction,
                                       short dFunctionLen, LPSS_VALUE lpResult,
                                       LPSS_VALUE lpValues, short dParamCnt);
#else
BOOL FAR PASCAL _export SpreadCustomFunctionProc(long hSS, LPTSTR lpszFunction,
                                       LPSS_VALUE lpResult, LPSS_VALUE lpValues,
                                       short dParamCnt);
#endif
BOOL FAR PASCAL _export __SpreadGetDataFillData(LONG lObject, LPVAR lpVar, LPTSTR lpszText, //DBOCX
                                        WORD wVarType);
BOOL FAR PASCAL _export __SpreadSetDataFillData(LONG lObject, LPVBSPREAD lpSpread,   //DBOCX
                                        HLSTR hlstr, long lData, BOOL fUselData);
void            __SpreadSetText(HWND hWnd, long Col, long Row, LPVAR lpVar);
BOOL FAR PASCAL _export __SpreadSetCellDirtyFlag(LONG lObject, HWND hWnd, SS_COORD Col,
                                         SS_COORD Row, BOOL fDirty);
TBGLOBALHANDLE  SSCreateString(LPTSTR lpszString, long lLen);
BOOL _SpreadArray(HCTL hCtl, SS_COORD ColLeft, SS_COORD RowTop, HAD hAD, BOOL fGet);


#define DataCallPrimative(lpda, msg, action, lpSS)             \
   ((LPDATAACCESS)lpda)->sAction = action,               \
   VBSendControlMsg(((LPDATAACCESS)lpda)->hctlData, msg, \
                     0, (LONG)(LPDATAACCESS)lpda)
#define DataCall(lpda, msg, action, lpSS)                \
  DataCallPrimative(lpda, VBM_DATA_##msg, DATA_##action, lpSS)
#define DataCallwErrCoord(lpda, msg, action, row, col, lpSS) \
  DataCallPrimative(lpda, VBM_DATA_##msg, DATA_##action, lpSS)

extern HANDLE hDynamicInst;


#ifndef SSF_NO_GetClientArea
void FAR PASCAL _export SpreadGetClientArea(HCTL hCtl, LPLONG lplWidth, LPLONG lplHeight)
{
_SpreadGetClientArea(VBGetControlHwnd(hCtl), lplWidth, lplHeight);
}


void FAR PASCAL _export _SpreadGetClientArea(HWND hWnd, LPLONG lplWidth,
                                   LPLONG lplHeight)
{
RECT Rect;

SSGetClientRect(hWnd, &Rect);

if (lplWidth)
   *lplWidth = VBXPixelsToTwips(Rect.right - Rect.left);

if (lplHeight)
   *lplHeight = VBYPixelsToTwips(Rect.bottom - Rect.top);
}
#endif


#ifndef SSF_NO_GetCellFromScreenCoord
void FAR PASCAL _export SpreadGetCellFromScreenCoord(HCTL hCtl, LPLONG lpCol,
                                           LPLONG lpRow, long x, long y)
{
_SpreadGetCellFromScreenCoord(VBGetControlHwnd(hCtl), lpCol, lpRow,
                              VBXTwipsToPixels(x), VBYTwipsToPixels(y));
}


void FAR PASCAL _export _SpreadGetCellFromScreenCoord(HWND hWnd, LPLONG lpCol,
                                                      LPLONG lpRow, long x,
                                                      long y)
{
#ifdef SS_USE16BITCOORDS
{
SS_COORD Col;
SS_COORD Row;

SSGetCellFromPixel(hWnd, &Col, &Row, x, y);
*lpCol = (long)Col;
*lpRow = (long)Row;
}
#else
SSGetCellFromPixel(hWnd, lpCol, lpRow, (int)x, (int)y);
#endif

if (*lpCol == SS_HEADER)
   *lpCol = 0;
if (*lpRow == SS_HEADER)
   *lpRow = 0;

if (*lpCol == -2 || *lpRow == -2)
   {
   *lpCol = -1;
   *lpRow = -1;
   }
}
#endif


#ifndef SSF_NO_GetCellPos
BOOL FAR PASCAL _export SpreadGetCellPos(HCTL hCtl, long Col, long Row, LPLONG lpx,
                               LPLONG lpy, LPLONG lpWidth, LPLONG lpHeight)
{
return (_SpreadGetCellPos(VBGetControlHwnd(hCtl), Col, Row, lpx, lpy, lpWidth,
                          lpHeight));
}


BOOL FAR PASCAL _export _SpreadGetCellPos(HWND hWnd, long Col, long Row,
                                          LPLONG lpx, LPLONG lpy,
                                          LPLONG lpWidth, LPLONG lpHeight)
{
RECT Rect;

if (!SSGetCellRect(hWnd, (SS_COORD)Col, (SS_COORD)Row, &Rect))
   return (0);

if (lpx)
   *lpx = VBXPixelsToTwips(Rect.left);

if (lpy)
   *lpy = VBYPixelsToTwips(Rect.top);

if (lpWidth)
   *lpWidth = VBXPixelsToTwips(Rect.right - Rect.left);

if (lpHeight)
   *lpHeight = VBYPixelsToTwips(Rect.bottom - Rect.top);

return (-1);
}
#endif


#ifndef SSF_NO_GetBottomRightCell
void FAR PASCAL _export SpreadGetBottomRightCell(HCTL hCtl, LPLONG lpCol, LPLONG lpRow)
{
_SpreadGetBottomRightCell(VBGetControlHwnd(hCtl), lpCol, lpRow);
}


void FAR PASCAL _export _SpreadGetBottomRightCell(HWND hWnd, LPLONG lpCol,
                                                  LPLONG lpRow)
{
#ifdef SS_USE16BITCOORDS
{
long lCell;
lCell = SSGetBottomRightCell(hWnd);
*lpCol = (long)LOWORD(lCell);
*lpRow = (long)HIWORD(lCell);
}
#else
SSGetBottomRightCell(hWnd, lpCol, lpRow);
#endif
}
#endif


#ifndef SSF_NO_GetFirstValidCell
void FAR PASCAL _export SpreadGetFirstValidCell(HCTL hCtl, LPLONG lpCol, LPLONG lpRow)
{
_SpreadGetFirstValidCell(VBGetControlHwnd(hCtl), lpCol, lpRow);
}


void FAR PASCAL _export _SpreadGetFirstValidCell(HWND hWnd, LPLONG lpCol,
                                                 LPLONG lpRow)
{
#ifdef SS_USE16BITCOORDS
{
SS_COORD Col;
SS_COORD Row;

SSGetFirstValidCell(hWnd, &Col, &Row);
*lpCol = (long)Col;
*lpRow = (long)Row;
}
#else
SSGetFirstValidCell(hWnd, lpCol, lpRow);
#endif
}
#endif


#ifndef SSF_NO_GetLastValidCell
void FAR PASCAL _export SpreadGetLastValidCell(HCTL hCtl, LPLONG lpCol, LPLONG lpRow)
{
_SpreadGetLastValidCell(VBGetControlHwnd(hCtl), lpCol, lpRow);
}


void FAR PASCAL _export _SpreadGetLastValidCell(HWND hWnd, LPLONG lpCol,
                                                LPLONG lpRow)
{
#ifdef SS_USE16BITCOORDS
{
SS_COORD Col;
SS_COORD Row;

SSGetLastValidCell(hWnd, &Col, &Row);
*lpCol = (long)Col;
*lpRow = (long)Row;
}
#else
SSGetLastValidCell(hWnd, lpCol, lpRow);
#endif
}
#endif


#ifndef SSF_NO_GetCurrSelBlockPos
void FAR PASCAL _export SpreadGetCurrSelBlockPos(HCTL hCtl, LPLONG lpCol, LPLONG lpRow)
{
_SpreadGetCurrSelBlockPos(VBGetControlHwnd(hCtl), lpCol, lpRow);
}


void FAR PASCAL _export _SpreadGetCurrSelBlockPos(HWND hWnd, LPLONG lpCol,
                                                  LPLONG lpRow)
{
#ifdef SS_USE16BITCOORDS
{
SS_COORD Col;
SS_COORD Row;

SSGetCurrSelBlockPos(hWnd, &Col, &Row);
*lpCol = (long)Col;
*lpRow = (long)Row;
}
#else
SSGetCurrSelBlockPos(hWnd, lpCol, lpRow);
#endif
}
#endif


#ifndef SSF_NO_ColWidthToTwips
void FAR PASCAL _export SpreadColWidthToTwips(HCTL hCtl, float fColWidth, LPLONG lpTwips)
{
_SpreadColWidthToTwips(VBGetControlHwnd(hCtl), fColWidth, lpTwips);
}


void FAR PASCAL _export _SpreadColWidthToTwips(HWND hWnd, float fColWidth,
                                              LPLONG lpTwips)
{
short dPixels;

dPixels = SSColWidthToLogUnits(hWnd, (double)fColWidth);
*lpTwips = VBXPixelsToTwips(dPixels);
}
#endif


#ifndef SSF_NO_RowHeightToTwips
void FAR PASCAL _export SpreadRowHeightToTwips(HCTL hCtl, long Row, float fRowHeight,
                                     LPLONG lpTwips)
{
_SpreadRowHeightToTwips(VBGetControlHwnd(hCtl), Row, fRowHeight, lpTwips);
}


void FAR PASCAL _export _SpreadRowHeightToTwips(HWND hWnd, long Row,
                                               float fRowHeight,
                                               LPLONG lpTwips)
{
short dPixels;

dPixels = SSRowHeightToLogUnits(hWnd, (SS_COORD)Row, (double)fRowHeight);
*lpTwips = VBYPixelsToTwips(dPixels);
}
#endif


#ifndef SSF_NO_TwipsToColWidth
void FAR PASCAL _export SpreadTwipsToColWidth(HCTL hCtl, long Twips, LPFLOAT lpfColWidth)
{
_SpreadTwipsToColWidth(VBGetControlHwnd(hCtl), Twips, lpfColWidth);
}


void FAR PASCAL _export _SpreadTwipsToColWidth(HWND hWnd, long Twips,
                                              LPFLOAT lpfColWidth)
{
double dfColWidth;

SSLogUnitsToColWidth(hWnd, VBXTwipsToPixels(Twips), &dfColWidth);
*lpfColWidth = (float)dfColWidth;
}
#endif


#ifndef SSF_NO_TwipsToRowHeight
void FAR PASCAL _export SpreadTwipsToRowHeight(HCTL hCtl, long Row, long Twips,
                                     LPFLOAT lpfRowHeight)
{
_SpreadTwipsToRowHeight(VBGetControlHwnd(hCtl), Row, Twips, lpfRowHeight);
}


void FAR PASCAL _export _SpreadTwipsToRowHeight(HWND hWnd, long Row, long Twips,
                                               LPFLOAT lpfRowHeight)
{
double dfRowHeight;

SSLogUnitsToRowHeight(hWnd, (SS_COORD)Row, VBYTwipsToPixels(Twips),
                      &dfRowHeight);
*lpfRowHeight = (float)dfRowHeight;
}
#endif


#ifndef SSF_NO_IsVisible
BOOL FAR PASCAL _export SpreadIsVisible(HCTL hCtl, long Col, long Row, BOOL Partial)
{
return (_SpreadIsVisible(VBGetControlHwnd(hCtl), Col, Row, Partial));
}


BOOL FAR PASCAL _export _SpreadIsVisible(HWND hWnd, long Col, long Row,
                                        BOOL Partial)
{
return ((SSGetVisible(hWnd, (SS_COORD)Col, (SS_COORD)Row,
        Partial ? SS_VISIBLE_PARTIAL : SS_VISIBLE_ALL)) ? -1 : 0);
}
#endif


#ifndef SSF_NO_IsFormulaValid
BOOL FAR PASCAL _export SpreadIsFormulaValid(HCTL hCtl, HLSTR hlstrFormula)
{
LPTSTR lpszFormula;

lpszFormula = VBDerefZeroTermHlstr(hlstrFormula);

return (SSValidateFormula(VBGetControlHwnd(hCtl), lpszFormula) ? -1 : 0);
}


BOOL FAR PASCAL _export _SpreadIsFormulaValid(HWND hWnd, LPTSTR lpszFormula)
{
return (SSValidateFormula(hWnd, lpszFormula) ? -1 : 0);
}
#endif


#ifndef SSF_NO_IsCellSelected
BOOL FAR PASCAL _export SpreadIsCellSelected(HCTL hCtl, long Col, long Row)
{
return (_SpreadIsCellSelected(VBGetControlHwnd(hCtl), Col, Row));
}


BOOL FAR PASCAL _export _SpreadIsCellSelected(HWND hWnd, long Col, long Row)
{
return (SSIsCellInSelection(hWnd, (SS_COORD)Col, (SS_COORD)Row) ? -1 : 0);
}
#endif


#ifndef SSF_NO_SetItemData
void FAR PASCAL _export SpreadSetItemData(HCTL hCtl, long lValue)
{
_SpreadSetItemData(VBGetControlHwnd(hCtl), lValue);
}

void FAR PASCAL _export _SpreadSetItemData(HWND hWnd, long lValue)
{
__SpreadSetItemData(hWnd, SS_ALLCOLS, SS_ALLROWS, lValue);
}
#endif


#ifndef SSF_NO_SetColItemData
void FAR PASCAL _export SpreadSetColItemData(HCTL hCtl, long Col, long lValue)
{
_SpreadSetColItemData(VBGetControlHwnd(hCtl), Col, lValue);
}

void FAR PASCAL _export _SpreadSetColItemData(HWND hWnd, long Col, long lValue)
{
__SpreadSetItemData(hWnd, Col, SS_ALLROWS, lValue);
}
#endif


#ifndef SSF_NO_SetRowItemData
void FAR PASCAL _export SpreadSetRowItemData(HCTL hCtl, long Row, long lValue)
{
_SpreadSetRowItemData(VBGetControlHwnd(hCtl), Row, lValue);
}

void FAR PASCAL _export _SpreadSetRowItemData(HWND hWnd, long Row, long lValue)
{
__SpreadSetItemData(hWnd, SS_ALLCOLS, Row, lValue);
}
#endif


#ifndef SSF_NO_GetItemData
long FAR PASCAL _export SpreadGetItemData(HCTL hCtl)
{
return (_SpreadGetItemData(VBGetControlHwnd(hCtl)));
}

long FAR PASCAL _export _SpreadGetItemData(HWND hWnd)
{
return (__SpreadGetItemData(hWnd, SS_ALLCOLS, SS_ALLROWS));
}
#endif


#ifndef SSF_NO_GetColItemData
long FAR PASCAL _export SpreadGetColItemData(HCTL hCtl, long Col)
{
return (_SpreadGetColItemData(VBGetControlHwnd(hCtl), Col));
}

long FAR PASCAL _export _SpreadGetColItemData(HWND hWnd, long Col)
{
return (__SpreadGetItemData(hWnd, Col, SS_ALLROWS));
}
#endif


#ifndef SSF_NO_GetRowItemData
long FAR PASCAL _export SpreadGetRowItemData(HCTL hCtl, long Row)
{
return (_SpreadGetRowItemData(VBGetControlHwnd(hCtl), Row));
}

long FAR PASCAL _export _SpreadGetRowItemData(HWND hWnd, long Row)
{
return (__SpreadGetItemData(hWnd, SS_ALLCOLS, Row));
}
#endif


#ifndef SSF_NO_SetText
#ifdef SS_GRID
void FAR PASCAL _export fpSetText(HCTL hCtl, long Col, long Row, LPVAR lpVar)
#else
void FAR PASCAL _export SpreadSetText(HCTL hCtl, long Col, long Row,
                                      LPVAR lpVar)
#endif
{
__SpreadSetText(VBGetControlHwnd(hCtl), Col, Row, lpVar);
}


void FAR PASCAL _export _SpreadSetText(HWND hWnd, long Col, long Row,
                                       LPTSTR lpText)
{
LPSPREADSHEET lpSS;

lpSS = SS_Lock(hWnd);

SS_SetDataRange(lpSS, (SS_COORD)Col, (SS_COORD)Row, (SS_COORD)Col,
                (SS_COORD)Row, lpText, FALSE, FALSE, TRUE);

SS_Unlock(hWnd);
}


void __SpreadSetText(HWND hWnd, long Col, long Row, LPVAR lpVar)
{
LPSPREADSHEET lpSS;
VALUE         Value;
short         dVarType;

lpSS = SS_Lock(hWnd);

if ((dVarType = VBGetVariantValue(lpVar, &Value)) != -1)
   {
   if (dVarType != VT_NULL &&
       dVarType != VT_EMPTY &&
       dVarType != VT_STRING)
      {
      if (Col == 0)
         Col = SS_HEADER;

      if (Row == 0)
         Row = SS_HEADER;
      }

   switch (dVarType)
      {
      case VT_NULL:
      case VT_EMPTY:
         SS_SetDataRange(lpSS, (SS_COORD)Col, (SS_COORD)Row,
                         (SS_COORD)Col, (SS_COORD)Row, NULL, FALSE, FALSE, TRUE);
         break;

      case VT_I2:
         SSSetInteger(hWnd, (SS_COORD)Col, (SS_COORD)Row, (long)Value.i2);
         break;

      case VT_I4:
         SSSetInteger(hWnd, (SS_COORD)Col, (SS_COORD)Row, Value.i4);
         break;

      case VT_R4:
         SSSetFloat(hWnd, (SS_COORD)Col, (SS_COORD)Row, (double)Value.r4);
         break;

      case VT_R8:
         SSSetFloat(hWnd, (SS_COORD)Col, (SS_COORD)Row, Value.r8);
         break;

      case VT_DATE:
      case VT_CURRENCY:
         break;

      case VT_STRING:
         SS_SetDataRange(lpSS, (SS_COORD)Col, (SS_COORD)Row,
                         (SS_COORD)Col, (SS_COORD)Row,
                         VBDerefZeroTermHlstr(Value.hlstr),
                         FALSE, FALSE, TRUE);
         break;
      }
   }

SS_Unlock(hWnd);
}
#endif


#ifndef SSF_NO_GetText
#ifdef SS_GRID
BOOL FAR PASCAL _export fpGetText(HCTL hCtl, long Col, long Row, LPVAR lpVar)
#else
BOOL FAR PASCAL _export SpreadGetText(HCTL hCtl, long Col, long Row,
                                      LPVAR lpVar)
#endif
{
SS_CELLTYPE CellType;
HWND        hWnd = VBGetControlHwnd(hCtl);
VALUE       Value;
short       dVarType = VT_STRING;
BOOL        fRet = FALSE;

if (Col == 0)
   Col = SS_HEADER;

if (Row == 0)
   Row = SS_HEADER;

SSGetCellType(hWnd, (SS_COORD)Col, (SS_COORD)Row, &CellType);

if (CellType.Type == SS_TYPE_FLOAT)
   {
   if (VBGetVariantType(lpVar) != VT_STRING)
      {
      fRet = SSGetFloat(hWnd, (SS_COORD)Col, (SS_COORD)Row, &Value.r8) ? -1 : 0;
      dVarType = VT_R8;
      }
   }

else if (CellType.Type == SS_TYPE_INTEGER)
   {
   if (VBGetVariantType(lpVar) != VT_STRING)
      {
      fRet = SSGetInteger(hWnd, (SS_COORD)Col, (SS_COORD)Row, &Value.i4) ? -1 :
                          0;
      dVarType = VT_I4;
      }
   }

if (dVarType == VT_STRING)
   {
   if (Value.hlstr = (HLSTR)__SpreadGetTextString(hWnd, &CellType,
                                                  (SS_COORD)Col,
                                                  (SS_COORD)Row, TRUE))
      fRet = -1;
   else
      Value.hlstr = VBCreateTempHlstr(NULL, 0);
   }

// BJO 28Feb97 GIL81 - Before work around (VB4-16 problem?)
//VBSetVariantValue(lpVar, dVarType, &Value);
// BJO 28Feb97 GIL81 - Begin work around (VB4-16 problem?)
if (VT_STRING == dVarType && 0 == Value.hlstr)
   VBSetVariantValue(lpVar, VT_EMPTY, NULL);
else
   VBSetVariantValue(lpVar, dVarType, &Value);
// BJO 28Feb97 GIL81 - End work around (VB4-16 problem?)

//if (dVarType == VT_STRING)
//   VBDestroyHlstr(Value.hlstr);

return (fRet);
}


LPTSTR FAR PASCAL _export _SpreadGetText(HWND hWnd, long Col, long Row)
{
LPSPREADSHEET lpSS;
SS_CELLTYPE   CellType;
long          lhStr;

if (Col == 0)
   Col = SS_HEADER;

if (Row == 0)
   Row = SS_HEADER;

lpSS = SS_Lock(hWnd);
SS_RetrieveCellType(lpSS, &CellType, NULL, Col, Row);

lhStr = __SpreadGetTextString(hWnd, &CellType, (SS_COORD)Col, (SS_COORD)Row,
                              FALSE);
SS_Unlock(hWnd);

return ((LPTSTR)tbGlobalLock(lhStr));
}


long __SpreadGetTextString(HWND hWnd, LPSS_CELLTYPE lpCellType, SS_COORD Col,
                            SS_COORD Row, BOOL fHLSTR)
{
LPSPREADSHEET  lpSS;
TBGLOBALHANDLE hData;
LPTSTR         lpData;
long           lRet = 0;

lpSS = SS_Lock(hWnd);

if (Col != SS_ALLCOLS)
   if (Col < 0)
      Col += -SS_HEADER;

if (Row != SS_ALLROWS)
   if (Row < 0)
      Row += -SS_HEADER;

if (lpSS->EditModeOn && lpSS->Col.CurAt == Col && lpSS->Row.CurAt == Row)
   hData = SS_GetEditModeOnData(hWnd, lpSS, FALSE);
else
   hData = SS_GetData(lpSS, lpCellType, Col, Row, FALSE);

if (hData)
   {
   lpData = (LPTSTR)tbGlobalLock(hData);

   if (fHLSTR)
      lRet = (long)VBCreateTempHlstr(lpData, lstrlen(lpData));
   else
      lRet = (long)SSCreateString(lpData, -1);

   tbGlobalUnlock(hData);

   if (lpSS->EditModeOn && lpSS->Col.CurAt == Col && lpSS->Row.CurAt == Row)
      tbGlobalFree(hData);
   }

SS_Unlock(hWnd);

return (lRet);
}
#endif


#ifndef SSF_NO_AddCustomFunction
BOOL FAR PASCAL _export SpreadAddCustomFunction(HCTL hCtl, HSZ lpszFunctionName,
                                      short nParameterCnt)
{
return (_SpreadAddCustomFunction(VBGetControlHwnd(hCtl), lpszFunctionName,
                                 nParameterCnt));
}


BOOL FAR PASCAL _export _SpreadAddCustomFunction(HWND hWnd,
                                                HSZ lpszFunctionName,
                                                short nParameterCnt)
{
FARPROC lpfnFuncProcInst;

lpfnFuncProcInst = MakeProcInstance((FARPROC)SpreadCustomFunctionProc,
                                    hDynamicInst);

return (SSAddCustomFunction(hWnd, lpszFunctionName, nParameterCnt,
                            lpfnFuncProcInst) ? -1 : 0);
}

#ifndef SS_OLDCALC
#ifndef SSF_NO_AddCustomFunctionExt
BOOL FAR PASCAL _export SpreadAddCustomFunctionExt(HCTL hCtl,
                                                   HSZ lpszFunctionName,
                                                   short nMinParamCnt,
                                                   short nMaxParamCnt,
                                                   long lFlags)
{
return (_SpreadAddCustomFunctionExt(VBGetControlHwnd(hCtl), lpszFunctionName,
                                    nMinParamCnt, nMaxParamCnt, lFlags));
}


BOOL FAR PASCAL _export _SpreadAddCustomFunctionExt(HWND hWnd,
                                                    HSZ lpszFunctionName,
                                                    short nMinParamCnt,
                                                    short nMaxParamCnt,
                                                    long lFlags)
{
FARPROC lpfnFuncProcInst;

lpfnFuncProcInst = MakeProcInstance((FARPROC)SpreadCustomFunctionProc,
                                    hDynamicInst);

return (SSAddCustomFunctionExt(hWnd, lpszFunctionName,
                               nMinParamCnt, nMaxParamCnt,
                               lpfnFuncProcInst, lFlags) ? -1 : 0);
}
#endif
#endif


#ifdef SS_OLDCALC
BOOL FAR PASCAL _export SpreadCustomFunctionProc(HWND hWnd, LPTSTR lpszFunction,
                                       short dFunctionLen, LPSS_VALUE lpResult,
                                       LPSS_VALUE lpValues, short dParamCnt)
#else
BOOL FAR PASCAL _export SpreadCustomFunctionProc(long hSS, LPTSTR lpszFunction,
                                       LPSS_VALUE lpResult, LPSS_VALUE lpValues,
                                       short dParamCnt)
#endif
{
#if SS_OLDCALC
LPSPREADSHEET lpSS = SS_Lock(hWnd);
#else
LPSPREADSHEET lpSS = (LPSPREADSHEET)tbGlobalLock((TBGLOBALHANDLE)hSS);
short dFunctionLen = lstrlen(lpszFunction);
#endif
HCTL          hCtl = VBGetHwndControl(lpSS->lpBook->hWnd);
HLSTR         hlstrFunction;
SS_VALUE      Result;
SPREADPARAMS5 Params5;
LPVBSPREAD    lpSpread;
SS_COORD      Col;
SS_COORD      Row;
short         Status = SS_VALUE_STATUS_OK;
LPSS_VALUE    lpCFResultPrev;
LPSS_VALUE    lpCFValuesPrev;
short         dCFValueCntPrev;

lpSpread = (LPVBSPREAD)VBDerefControl(hCtl);

lpCFResultPrev = lpSpread->lpCFResult;
lpCFValuesPrev = lpSpread->lpCFValues;
dCFValueCntPrev = lpSpread->dCFValueCnt;

lpSpread->lpCFResult = &Result;
lpSpread->lpCFValues = lpValues;
lpSpread->dCFValueCnt = dParamCnt;

//????hlstrFunction = VBCreateHlstr(lpszFunction, dFunctionLen);
hlstrFunction = VBCreateHlstr(lpszFunction, dFunctionLen);

SSGetCellSendingMsg(lpSS->lpBook->hWnd, &Col, &Row);

Params5.lpParam1 = hlstrFunction;
Params5.lpParam2 = &dParamCnt;
Params5.lpParam3 = &Col;
Params5.lpParam4 = &Row;
Params5.lpParam5 = &Status;
VBFireEvent(hCtl, SS_EVENT_CustomFunction, &Params5);

if (hlstrFunction)
   VBDestroyHlstr(hlstrFunction);

Result.Status = (char)Status;

_fmemcpy(lpResult, &Result, sizeof(SS_VALUE));

lpSpread->lpCFResult = lpCFResultPrev;
lpSpread->lpCFValues = lpCFValuesPrev;
lpSpread->dCFValueCnt = dCFValueCntPrev;

#ifdef SS_OLDCALC
SS_Unlock(hWnd)
#else
tbGlobalUnlock((TBGLOBALHANDLE)hSS);
#endif
return (-1);
}
#endif


#ifndef SSF_NO_CFGetParamInfo
BOOL FAR PASCAL _export SpreadCFGetParamInfo(HCTL hCtl, short dParam, LPWORD lpwType,
                                   LPWORD lpwStatus)
{
return (_SpreadCFGetParamInfo(VBGetControlHwnd(hCtl), dParam, lpwType,
                              lpwStatus));
}


BOOL FAR PASCAL _export _SpreadCFGetParamInfo(HWND hWnd, short dParam,
                                             LPWORD lpwType, LPWORD lpwStatus)
{
HCTL       hCtl = VBGetHwndControl(hWnd);
LPVBSPREAD lpSpread;
BOOL       fRet = FALSE;

lpSpread = (LPVBSPREAD)VBDerefControl(hCtl);

if (dParam >= 1 && dParam <= lpSpread->dCFValueCnt)
   {
   if (lpwType)
      *lpwType = lpSpread->lpCFValues[dParam - 1].Type;

   if (lpwStatus)
      *lpwStatus = lpSpread->lpCFValues[dParam - 1].Status;

   fRet = -1;
   }

return (fRet);
}
#endif


#ifndef SSF_NO_CFGetLongParam
long FAR PASCAL _export SpreadCFGetLongParam(HCTL hCtl, short dParam)
{
return (_SpreadCFGetLongParam(VBGetControlHwnd(hCtl), dParam));
}


long FAR PASCAL _export _SpreadCFGetLongParam(HWND hWnd, short dParam)
{
HCTL       hCtl = VBGetHwndControl(hWnd);
LPVBSPREAD lpSpread;
LPTSTR     lpszValue;
long       lRet = 0;

lpSpread = (LPVBSPREAD)VBDerefControl(hCtl);

if (dParam >= 1 && dParam <= lpSpread->dCFValueCnt)
   {
   if (lpSpread->lpCFValues[dParam - 1].Type == SS_VALUE_TYPE_DOUBLE)
      lRet = (long)lpSpread->lpCFValues[dParam - 1].Val.ValDouble;
   else if (lpSpread->lpCFValues[dParam - 1].Type == SS_VALUE_TYPE_LONG)
      lRet = lpSpread->lpCFValues[dParam - 1].Val.ValLong;
   else if (lpSpread->lpCFValues[dParam - 1].Type == SS_VALUE_TYPE_STR &&
            lpSpread->lpCFValues[dParam - 1].Val.hValStr)
      {
      lpszValue =
         (LPTSTR)GlobalLock(lpSpread->lpCFValues[dParam - 1].Val.hValStr);
      lRet = StringToLong(lpszValue);
      GlobalUnlock(lpSpread->lpCFValues[dParam - 1].Val.hValStr);
      }
   }

return (lRet);
}
#endif


#ifndef SSF_NO_CFGetDoubleParam
double FAR PASCAL _export SpreadCFGetDoubleParam(HCTL hCtl, short dParam)
{
return (_SpreadCFGetDoubleParam(VBGetControlHwnd(hCtl), dParam));
}


double FAR PASCAL _export _SpreadCFGetDoubleParam(HWND hWnd, short dParam)
{
HCTL       hCtl = VBGetHwndControl(hWnd);
LPVBSPREAD lpSpread;
LPTSTR     lpszValue;
double     dfRet = 0.0;

lpSpread = (LPVBSPREAD)VBDerefControl(hCtl);

if (dParam >= 1 && dParam <= lpSpread->dCFValueCnt)
   {
   if (lpSpread->lpCFValues[dParam - 1].Type == SS_VALUE_TYPE_DOUBLE)
      dfRet = lpSpread->lpCFValues[dParam - 1].Val.ValDouble;
   else if (lpSpread->lpCFValues[dParam - 1].Type == SS_VALUE_TYPE_LONG)
      dfRet = (double)lpSpread->lpCFValues[dParam - 1].Val.ValLong;
   else if (lpSpread->lpCFValues[dParam - 1].Type == SS_VALUE_TYPE_STR)
      {
      lpszValue =
         (LPTSTR)GlobalLock(lpSpread->lpCFValues[dParam - 1].Val.hValStr);
      StringToFloat(lpszValue, &dfRet);
      GlobalUnlock(lpSpread->lpCFValues[dParam - 1].Val.hValStr);
      }
   }

return (dfRet);
}

void FAR PASCAL _export _SpreadCFGetDoubleParamExt(HWND hWnd, short dParam,
                                                   double FAR* lpdfValue)
{
   *lpdfValue = _SpreadCFGetDoubleParam(hWnd, dParam);
}
#endif


#ifndef SSF_NO_CFGetStringParam
HLSTR FAR PASCAL _export SpreadCFGetStringParam(HCTL hCtl, short dParam)
{
LPVBSPREAD lpSpread;
char       Buffer[50];
HLSTR      hlstrValue = 0;
LPTSTR     lpszValue;

lpSpread = (LPVBSPREAD)VBDerefControl(hCtl);

if (dParam >= 1 && dParam <= lpSpread->dCFValueCnt)
   {
   if (lpSpread->lpCFValues[dParam - 1].Type == SS_VALUE_TYPE_DOUBLE)
      ;

   else if (lpSpread->lpCFValues[dParam - 1].Type == SS_VALUE_TYPE_LONG)
      {
      LongToString(lpSpread->lpCFValues[dParam - 1].Val.ValLong, Buffer);
      hlstrValue = VBCreateHlstr(Buffer, lstrlen(Buffer));
      }

   else if (lpSpread->lpCFValues[dParam - 1].Type == SS_VALUE_TYPE_STR &&
            lpSpread->lpCFValues[dParam - 1].Val.hValStr)
      {
      lpszValue =
         (LPTSTR)GlobalLock(lpSpread->lpCFValues[dParam - 1].Val.hValStr);
      hlstrValue = VBCreateHlstr(lpszValue, lstrlen(lpszValue));
      GlobalUnlock(lpSpread->lpCFValues[dParam - 1].Val.hValStr);
      }
   }

if (!hlstrValue)
   hlstrValue = VBCreateHlstr(NULL, 0);

return (hlstrValue);
}


LPTSTR FAR PASCAL _export _SpreadCFGetStringParam(HWND hWnd, short dParam)
{
HCTL           hCtl = VBGetHwndControl(hWnd);
LPVBSPREAD     lpSpread;
char           Buffer[50];
TBGLOBALHANDLE hlstrValue = 0;
LPTSTR          lpszValue;

lpSpread = (LPVBSPREAD)VBDerefControl(hCtl);

if (dParam >= 1 && dParam <= lpSpread->dCFValueCnt)
   {
   if (lpSpread->lpCFValues[dParam - 1].Type == SS_VALUE_TYPE_DOUBLE)
      ;

   else if (lpSpread->lpCFValues[dParam - 1].Type == SS_VALUE_TYPE_LONG)
      {
      LongToString(lpSpread->lpCFValues[dParam - 1].Val.ValLong, Buffer);
      hlstrValue = SSCreateString(Buffer, -1);
      }

   else if (lpSpread->lpCFValues[dParam - 1].Type == SS_VALUE_TYPE_STR &&
            lpSpread->lpCFValues[dParam - 1].Val.hValStr)
      {
      lpszValue =
         (LPTSTR)GlobalLock(lpSpread->lpCFValues[dParam - 1].Val.hValStr);
      hlstrValue = SSCreateString(lpszValue, -1);
      GlobalUnlock(lpSpread->lpCFValues[dParam - 1].Val.hValStr);
      }
   }

return ((LPTSTR)tbGlobalLock(hlstrValue));
}
#endif


#ifndef SSF_NO_CFGetCellParam
void FAR PASCAL _export SpreadCFGetCellParam(HCTL hCtl, short dParam,
                                             LPLONG lplCol, LPLONG lplRow)
{
_SpreadCFGetCellParam(VBGetControlHwnd(hCtl), dParam, lplCol, lplRow);
}


void FAR PASCAL _export _SpreadCFGetCellParam(HWND hWnd, short dParam,
                                              LPLONG lplCol, LPLONG lplRow)
{
HCTL       hCtl = VBGetHwndControl(hWnd);
LPVBSPREAD lpSpread;
long       lRet = 0;

lpSpread = (LPVBSPREAD)VBDerefControl(hCtl);

if (dParam >= 1 && dParam <= lpSpread->dCFValueCnt)
   {
   if (lpSpread->lpCFValues[dParam - 1].Type == SS_VALUE_TYPE_CELL)
      {
      *lplCol = (long)lpSpread->lpCFValues[dParam - 1].Val.ValCell.Col;
      *lplRow = (long)lpSpread->lpCFValues[dParam - 1].Val.ValCell.Row;
      }
   else if (lpSpread->lpCFValues[dParam - 1].Type == SS_VALUE_TYPE_RANGE)
      {
      *lplCol = (long)lpSpread->lpCFValues[dParam - 1].Val.ValRange.Col1;
      *lplRow = (long)lpSpread->lpCFValues[dParam - 1].Val.ValRange.Row1;
      }
   }
}
#endif


#ifndef SSF_NO_CFGetRangeParam
void FAR PASCAL _export SpreadCFGetRangeParam(HCTL hCtl, short dParam,
                                              LPLONG lplCol, LPLONG lplRow,
                                              LPLONG lplCol2, LPLONG lplRow2)
{
_SpreadCFGetRangeParam(VBGetControlHwnd(hCtl), dParam, lplCol, lplRow,
                       lplCol2, lplRow2);
}


void FAR PASCAL _export _SpreadCFGetRangeParam(HWND hWnd, short dParam,
                                               LPLONG lplCol, LPLONG lplRow,
                                               LPLONG lplCol2, LPLONG lplRow2)
{
HCTL       hCtl = VBGetHwndControl(hWnd);
LPVBSPREAD lpSpread;
long       lRet = 0;

lpSpread = (LPVBSPREAD)VBDerefControl(hCtl);

if (dParam >= 1 && dParam <= lpSpread->dCFValueCnt)
   {
   if (lpSpread->lpCFValues[dParam - 1].Type == SS_VALUE_TYPE_CELL)
      {
      *lplCol = (long)lpSpread->lpCFValues[dParam - 1].Val.ValCell.Col;
      *lplRow = (long)lpSpread->lpCFValues[dParam - 1].Val.ValCell.Row;
      *lplCol2 = (long)lpSpread->lpCFValues[dParam - 1].Val.ValCell.Col;
      *lplRow2 = (long)lpSpread->lpCFValues[dParam - 1].Val.ValCell.Row;
      }
   else if (lpSpread->lpCFValues[dParam - 1].Type == SS_VALUE_TYPE_RANGE)
      {
      *lplCol = (long)lpSpread->lpCFValues[dParam - 1].Val.ValRange.Col1;
      *lplRow = (long)lpSpread->lpCFValues[dParam - 1].Val.ValRange.Row1;
      *lplCol2 = (long)lpSpread->lpCFValues[dParam - 1].Val.ValRange.Col2;
      *lplRow2 = (long)lpSpread->lpCFValues[dParam - 1].Val.ValRange.Row2;
      }
   }
}
#endif


#ifndef SSF_NO_CFSetResult
void FAR PASCAL _export SpreadCFSetResult(HCTL hCtl, LPVAR lpVar)
{
HWND         hWnd = VBGetControlHwnd(hCtl);
GLOBALHANDLE hBuffer;
LPVBSPREAD   lpSpread;
LPTSTR       lpBuffer;
LPTSTR       lpText;
VALUE        Value;
short        dVarType;

lpSpread = (LPVBSPREAD)VBDerefControl(hCtl);

if (lpSpread->lpCFResult)
   {
   lpSpread->lpCFResult->Val.ValLong = 0;
   lpSpread->lpCFResult->Type = SS_VALUE_TYPE_LONG;

   if ((dVarType = VBGetVariantValue(lpVar, &Value)) != -1)
      {
      switch (dVarType)
         {
         case VT_I2:
            lpSpread->lpCFResult->Val.ValLong = (long)Value.i2;
            lpSpread->lpCFResult->Type = SS_VALUE_TYPE_LONG;
            break;

         case VT_I4:
            lpSpread->lpCFResult->Val.ValLong = Value.i4;
            lpSpread->lpCFResult->Type = SS_VALUE_TYPE_LONG;
            break;

         case VT_R4:
            lpSpread->lpCFResult->Val.ValDouble = (double)Value.r4;
            lpSpread->lpCFResult->Type = SS_VALUE_TYPE_DOUBLE;
            break;

         case VT_R8:
            lpSpread->lpCFResult->Val.ValDouble = Value.r8;
            lpSpread->lpCFResult->Type = SS_VALUE_TYPE_DOUBLE;
            break;

         case VT_NULL:
         case VT_EMPTY:
         case VT_DATE:
         case VT_CURRENCY:
            break;

         case VT_STRING:
            lpText = VBDerefZeroTermHlstr(Value.hlstr);

            if (hBuffer = GlobalAlloc(GMEM_MOVEABLE | GMEM_ZEROINIT,
                                      lstrlen(lpText) + 1))
               {
               lpBuffer = (LPTSTR)GlobalLock(hBuffer);
               lstrcpy(lpBuffer, lpText);
               GlobalUnlock(hBuffer);
               lpSpread->lpCFResult->Val.hValStr = hBuffer;
               lpSpread->lpCFResult->Type = SS_VALUE_TYPE_STR;
               }

            break;
         }
      }
   }
}


void FAR PASCAL _export _SpreadCFSetResult(HWND hWnd, LPTSTR lpText)
{
HCTL           hCtl = VBGetHwndControl(hWnd);
LPSPREADSHEET  lpSS;
GLOBALHANDLE   hBuffer;
LPVBSPREAD     lpSpread;
SS_CELLTYPE    CellType;
SS_COORD       Col;
SS_COORD       Row;
LPTSTR          lpBuffer;

lpSpread = (LPVBSPREAD)VBDerefControl(hCtl);
lpSS = SS_Lock(hWnd);

SSGetCellSendingMsg(hWnd, &Col, &Row);
SS_RetrieveCellType(lpSS, &CellType, NULL, Col, Row);

if (lpSpread->lpCFResult)
   {
   lpSpread->lpCFResult->Val.ValLong = 0;
   lpSpread->lpCFResult->Type = SS_VALUE_TYPE_LONG;

   if (lpText)
      {
      switch (CellType.Type)
         {
         case SS_TYPE_FLOAT:
            StringToFloat(lpText, &lpSpread->lpCFResult->Val.ValDouble);
            lpSpread->lpCFResult->Type = SS_VALUE_TYPE_DOUBLE;
            break;

         case SS_TYPE_INTEGER:
            lpSpread->lpCFResult->Val.ValLong = StringToLong(lpText);
            lpSpread->lpCFResult->Type = SS_VALUE_TYPE_LONG;
            break;

         default:
            if (hBuffer = GlobalAlloc(GMEM_MOVEABLE | GMEM_ZEROINIT,
                                      lstrlen(lpText) + 1))
               {
               lpBuffer = (LPTSTR)GlobalLock(hBuffer);
               lstrcpy(lpBuffer, lpText);
               GlobalUnlock(hBuffer);
               lpSpread->lpCFResult->Val.hValStr = hBuffer;
               lpSpread->lpCFResult->Type = SS_VALUE_TYPE_STR;
               }

            break;
         }
      }
   }

SS_Unlock(hWnd);
}


void FAR PASCAL _export _SpreadCFSetResultLong(HWND hWnd, long lValue)
{
HCTL           hCtl = VBGetHwndControl(hWnd);
LPVBSPREAD     lpSpread;

lpSpread = (LPVBSPREAD)VBDerefControl(hCtl);

lpSpread->lpCFResult->Val.ValLong = lValue;
lpSpread->lpCFResult->Type = SS_VALUE_TYPE_LONG;
}


void FAR PASCAL _export _SpreadCFSetResultDouble(HWND hWnd, double dfValue)
{
HCTL           hCtl = VBGetHwndControl(hWnd);
LPVBSPREAD     lpSpread;

lpSpread = (LPVBSPREAD)VBDerefControl(hCtl);

lpSpread->lpCFResult->Val.ValDouble = dfValue;
lpSpread->lpCFResult->Type = SS_VALUE_TYPE_DOUBLE;
}
#endif


#ifndef SSF_NO_ColNumberToLetter
HLSTR FAR PASCAL _export SpreadColNumberToLetter(long lHeaderNumber)
{
return (_SpreadColNumberToLetter(lHeaderNumber));
}


HLSTR FAR PASCAL _export _SpreadColNumberToLetter(long lHeaderNumber)
{
char Buffer[20];

SS_DrawFormatHeaderLetter(Buffer, lHeaderNumber);

return (VBCreateHlstr(Buffer, lstrlen(Buffer)));
}
#endif


#ifndef SSF_NO_GetDataFillData
BOOL FAR PASCAL _export SpreadGetDataFillData(HCTL hCtl, LPVAR lpVar,
                                              WORD wVarType)
{
return (__SpreadGetDataFillData((LONG)hCtl, lpVar, NULL, wVarType));
}


BOOL FAR PASCAL _export _SpreadGetDataFillData(HWND hWnd, LPTSTR lpszText)
{
return (__SpreadGetDataFillData((LONG)VBGetHwndControl(hWnd), NULL, lpszText,
                                VT_STRING));
}

//DBOCX  __SpreadGetDataFillData() moved to end-of-file

#endif


#ifndef SSF_NO_SetDataFillData
BOOL FAR PASCAL _export SpreadSetDataFillData(HCTL hCtl, LPVAR lpVar)
{
LPVBSPREAD lpSpread;
HLSTR      hlstr;
BOOL       fRet = FALSE;

lpSpread = (LPVBSPREAD)VBDerefControl(hCtl);

if (lpSpread->dDataSetFillCol != -1)
   {
   VBCoerceVariant(lpVar, VT_STRING, &hlstr);
   fRet = __SpreadSetDataFillData((LONG)hCtl, lpSpread, hlstr, 0, FALSE);
   }

return (fRet);
}


BOOL FAR PASCAL _export _SpreadSetDataFillData(HWND hWnd, LPTSTR lpszText)
{
HCTL       hCtl = VBGetHwndControl(hWnd);
LPVBSPREAD lpSpread;
HLSTR      hlstr;
BOOL       fRet = FALSE;

lpSpread = (LPVBSPREAD)VBDerefControl(hCtl);

if (lpSpread->dDataSetFillCol != -1)
   {
   hlstr = VBCreateHlstr(lpszText, lstrlen(lpszText));
   fRet = __SpreadSetDataFillData((LONG)hCtl, lpSpread, hlstr, 0, FALSE);
   }

return (fRet);
}

//DBOCX   __SpreadSetDataFillData() moved to end-of-file

#endif

//--------------------------------------------------------------------
//
//  The SpreadLoadFromFile() and _SpreadLoadFromFile() functions
//  load a spreadsheet from the given file (either binary or tab
//  delimited text).
//

#ifndef SSF_NO_LoadFromFile

BOOL FAR PASCAL _export SpreadLoadFromFile(HCTL hCtl, LPCSTR lpszFileName)
{
return _SpreadLoadFromFile(VBGetControlHwnd(hCtl), lpszFileName);
}

BOOL FAR PASCAL _export _SpreadLoadFromFile(HWND hWnd, LPCSTR lpszFileName)
{
LPSPREADSHEET lpSS = SS_Lock(hWnd);
LPVBSPREAD lpSpread = (LPVBSPREAD)VBDerefControl(VBGetHwndControl(hWnd));
BOOL bRet = SSLoadFromFile(hWnd, lpszFileName) ? -1 : 0;
lpSpread->lMaxCols = SSGetMaxCols(hWnd);
lpSpread->lMaxRows = SSGetMaxRows(hWnd);
SS_Unlock(hWnd);
return bRet;
}

#endif

//--------------------------------------------------------------------
//
//  The SpreadLoadTabFile() and _SpreadLoadTabFile() functions load
//  a spreadsheet from a tab delimited text file.
//

#ifndef SSF_NO_LoadTabFile

BOOL FAR PASCAL _export SpreadLoadTabFile(HCTL hCtl, LPCSTR lpszFileName)
{
return _SpreadLoadTabFile(VBGetControlHwnd(hCtl), lpszFileName);
}

BOOL FAR PASCAL _export _SpreadLoadTabFile(HWND hWnd, LPCSTR lpszFileName)
{
LPVBSPREAD lpSpread = (LPVBSPREAD)VBDerefControl(VBGetHwndControl(hWnd));
BOOL bRet = SSLoadTabFile(hWnd, lpszFileName) ? -1 : 0;
lpSpread->lMaxCols = SSGetMaxCols(hWnd);
lpSpread->lMaxRows = SSGetMaxRows(hWnd);
return bRet;
}

#endif

//--------------------------------------------------------------------
//
//  The SpreadSaveToFile() and _SpreadSaveToFile() functions save
//  a spreadsheet to a binary file.
//

#ifndef SSF_NO_SaveToFile

BOOL FAR PASCAL _export SpreadSaveToFile(HCTL hCtl, LPCSTR lpszFileName,
                                         BOOL bDataOnly)
{
return _SpreadSaveToFile(VBGetControlHwnd(hCtl), lpszFileName, bDataOnly);
}

BOOL FAR PASCAL _export _SpreadSaveToFile(HWND hWnd, LPCSTR lpszFileName,
                                          BOOL bDataOnly)
{
LPSPREADSHEET lpSS = SS_Lock(hWnd);
BOOL bRet;
bRet = SSSaveToFile(hWnd, lpszFileName, bDataOnly) ? -1 : 0;
SS_Unlock(hwnd);
return bRet;
}

#endif

//--------------------------------------------------------------------
//
//  The SpreadSaveTabFile() and _SpreadSaveTabFile() functions save
//  a spreadsheet to a tab delimited text file.
//

#ifndef SSF_NO_SaveTabFile

#if (defined(SS_GRID))
BOOL FAR PASCAL _export SaveTabFile(HCTL hCtl, LPCSTR lpszFileName)
#else
BOOL FAR PASCAL _export SpreadSaveTabFile(HCTL hCtl, LPCSTR lpszFileName)
#endif
{
return (_SpreadSaveTabFile(VBGetControlHwnd(hCtl), lpszFileName));
}

BOOL FAR PASCAL _export _SpreadSaveTabFile(HWND hWnd, LPCSTR lpszFileName)
{
HCURSOR hCursor;
BOOL    fRet;

hCursor = SetCursor(LoadCursor(NULL, IDC_WAIT));
ShowCursor(TRUE);

fRet = SSSaveTabFile(hWnd, lpszFileName);

ShowCursor(FALSE);
SetCursor(hCursor);

return (fRet ? -1 : 0);
}

#endif

//--------------------------------------------------------------------

#ifndef SSF_NO_SetCellDirtyFlag
#if (defined(SS_GRID))
BOOL FAR PASCAL _export SetCellDirtyFlag(HCTL hCtl, SS_COORD Col,
                                         SS_COORD Row, BOOL fDirty)
#else
BOOL FAR PASCAL _export SpreadSetCellDirtyFlag(HCTL hCtl, SS_COORD Col,
                                               SS_COORD Row, BOOL fDirty)
#endif
{
return (__SpreadSetCellDirtyFlag((LONG)hCtl, VBGetControlHwnd(hCtl), Col, Row,
                                 fDirty));
}


BOOL FAR PASCAL _export _SpreadSetCellDirtyFlag(HWND hWnd, SS_COORD Col,
                                                SS_COORD Row, BOOL fDirty)
{
return (__SpreadSetCellDirtyFlag((LONG)VBGetHwndControl(hWnd), hWnd, Col, Row,
                                 fDirty));
}


/*    //DBOCX moved to end of file
BOOL __SpreadSetCellDirtyFlag(HCTL hCtl, HWND hWnd, SS_COORD Col, SS_COORD Row,
                              BOOL fDirty)
{
#ifdef SS_BOUNDCONTROL
LPSPREADSHEET lpSS;
LPVBSPREAD    lpSpread = (LPVBSPREAD)VBDerefControl(hCtl);
LPSS_ROW      lpRow;
LPSS_CELL     lpCell;
BOOL          fRet = FALSE;

if (Row >= 1)
   {
   lpSS = SS_Lock(hWnd);

   if (lpRow = SS_AllocLockRow(lpSS, Row))
      {
      if (fDirty && lpRow->bDirty != SS_ROWDIRTY_DEL &&
          lpRow->bDirty != SS_ROWDIRTY_INS)
         {
         if (Col >= 1)
            lpRow->bDirty = SS_ROWDIRTY_SOME;
         else
            lpRow->bDirty = SS_ROWDIRTY_ALL;
         }

      if (Col >= 1)
         {
         if (lpCell = SS_AllocLockCell(lpSS, lpRow, Col, Row))
            {
            lpCell->fDirty = fDirty ? 1 : 0;
            SSx_UnlockCellItem(lpSS, lpRow, Col, Row);
            }
         }

      else if (!fDirty)
         lpRow->bDirty = 0;

      SS_UnlockRowItem(lpSS, Row);
      }

   if (fDirty && lpSpread->DataOpts.fAutoSave)
      VBSSBoundChangeMade(lpSS, (LONG)hCtl, hWnd, Col, Row);

   fRet = -1;
   SS_Unlock(hWnd);
   }

return (fRet);
#else
return (FALSE);
#endif
}
*/  //DBOCX
#endif


#ifndef SSF_NO_GetCellDirtyFlag
#if (defined(SS_GRID))
BOOL FAR PASCAL _export GetCellDirtyFlag(HCTL hCtl, SS_COORD Col, SS_COORD Row)
#else
BOOL FAR PASCAL _export SpreadGetCellDirtyFlag(HCTL hCtl, SS_COORD Col,
                                               SS_COORD Row)
#endif
{
return (_SpreadGetCellDirtyFlag(VBGetControlHwnd(hCtl), Col, Row));
}

/*   //DBOCX - Moved to end of file
BOOL FAR PASCAL _export _SpreadGetCellDirtyFlag(HWND hWnd, SS_COORD Col, SS_COORD Row)
{
#ifdef SS_BOUNDCONTROL
LPSPREADSHEET lpSS;
LPSS_ROW      lpRow;
LPSS_CELL     lpCell;
BOOL          fDirty = FALSE;

if (Row >= 1)
   {
   lpSS = SS_Lock(hWnd);

   if (lpRow = SS_LockRowItem(lpSS, Row))
      {
      // RFW - 8/18/94
      fDirty = ((lpRow->bDirty == SS_ROWDIRTY_ALL ||
                 lpRow->bDirty == SS_ROWDIRTY_DEL) ? -1 : FALSE);

      if (Col >= 1)
         {
         if (lpCell = SSx_LockCellItem(lpSS, lpRow, Col, Row))
            {
            fDirty = lpCell->fDirty ? -1 : FALSE;
            SSx_UnlockCellItem(lpSS, lpRow, Col, Row);
            }
         }

      SS_UnlockRowItem(lpSS, Row);
      }

   SS_Unlock(hWnd);
   }

return (fDirty);
#else
return (FALSE);
#endif
}
*/     //DBOCX
#endif


#ifndef SSF_NO_GetMultiSelItem
SS_COORD FAR PASCAL _export SpreadGetMultiSelItem(HCTL hCtl, SS_COORD SelPrev)
{
HWND hWnd;

hWnd = VBGetControlHwnd(hCtl);

return ((SS_COORD)SSSelModeSendMessage(hWnd, SS_SELMODE_GETSELITEM,
                                       (long)SelPrev, 0L, 0L));
}


SS_COORD FAR PASCAL _export _SpreadGetMultiSelItem(HWND hWnd, SS_COORD SelPrev)
{
return ((SS_COORD)SSSelModeSendMessage(hWnd, SS_SELMODE_GETSELITEM,
                                       (long)SelPrev, 0L, 0L));
}
#endif


#ifdef SS_QE
#ifndef SSF_NO_GetDataConnectHandle
#ifdef SS_GRID
HANDLE FAR PASCAL _export GetDataConnectHandle(HCTL hCtl)
#else
HANDLE FAR PASCAL _export SpreadGetDataConnectHandle(HCTL hCtl)
#endif
{
LPVBSPREAD lpSpread;

lpSpread = (LPVBSPREAD)VBDerefControl(hCtl);
return ((HANDLE)lpSpread->DBInfo.hDB);
}


HANDLE FAR PASCAL _export _SpreadGetDataConnectHandle(HWND hWnd)
{
HCTL       hCtl = VBGetHwndControl(hWnd);
LPVBSPREAD lpSpread;

lpSpread = (LPVBSPREAD)VBDerefControl(hCtl);
return ((HANDLE)lpSpread->DBInfo.hDB);
}
#endif


#ifndef SSF_NO_GetDataSelectHandle
#ifdef SS_GRID
HANDLE FAR PASCAL _export GetDataSelectHandle(HCTL hCtl)
#else
HANDLE FAR PASCAL _export SpreadGetDataSelectHandle(HCTL hCtl)
#endif
{
LPVBSPREAD lpSpread;

lpSpread = (LPVBSPREAD)VBDerefControl(hCtl);
return ((HANDLE)lpSpread->DBInfo.hStmt);
}


HANDLE FAR PASCAL _export _SpreadGetDataSelectHandle(HWND hWnd)
{
HCTL       hCtl = VBGetHwndControl(hWnd);
LPVBSPREAD lpSpread;

lpSpread = (LPVBSPREAD)VBDerefControl(hCtl);
return ((HANDLE)lpSpread->DBInfo.hStmt);
}
#endif
#endif


#if 0
void __SpreadSetItemData(HWND hWnd, long Col, long Row, LPVAR lpVar)
{
VALUE Value;
float fValue;
long  lValue = 0L;
short dVarType;

if ((dVarType = VBGetVariantValue(lpVar, &Value)) != -1 &&
     dVarType != VT_EMPTY)
   {
   switch (dVarType)
      {
      case VT_NULL:
         lValue = 0L;
         break;

      case VT_I2:
         lValue = (long)Value.i2;
         break;

      case VT_I4:
         lValue = Value.i4;
         break;

      case VT_R4:
         _fmemcpy(&lValue, &Value.r4, sizeof(long));
         break;

      case VT_R8:
      case VT_DATE:
         fValue = (float)Value.r8;
         _fmemcpy(&lValue, &fValue, sizeof(long));
         break;

      case VT_CURRENCY:
         break;

      case VT_STRING:
         lValue = (long)Value.hlstr;
         break;
      }
   }

if (Col == SS_ALLCOLS && Row == SS_ALLROWS)
   SSSetUserData(hWnd, lValue);

else if (Col != SS_ALLCOLS)
   SSSetColUserData(hWnd, Col, lValue);

else if (Row != SS_ALLROWS)
   SSSetRowUserData(hWnd, Row, lValue);
}
#endif


void __SpreadSetItemData(HWND hWnd, long Col, long Row, long lValue)
{
if (Col == SS_ALLCOLS && Row == SS_ALLROWS)
#ifndef SSF_NO_GetItemData
   SSSetUserData(hWnd, lValue);
#else
   ;
#endif

else if (Col != SS_ALLCOLS)
#ifndef SSF_NO_GetColItemData
   SSSetColUserData(hWnd, (SS_COORD)Col, lValue);
#else
   ;
#endif

else if (Row != SS_ALLROWS)
#ifndef SSF_NO_GetRowItemData
   SSSetRowUserData(hWnd, (SS_COORD)Row, lValue);
#else
   ;
#endif
}

#if 0
void __SpreadGetItemData(HWND hWnd, long Col, long Row)
{
VALUE Value;
float fValue;
long  lValue = 0L;
short dVarType;

if (Col == SS_ALLCOLS && Row == SS_ALLROWS)
   lValue = SSGetUserData(hWnd);

else if (Col != SS_ALLCOLS)
   SSGetColUserData(hWnd, Col, &lValue);

else if (Row != SS_ALLROWS)
   SSGetRowUserData(hWnd, Row, &lValue);

if ((dVarType = VBGetVariantType(lpVar)) != -1 && dVarType != VT_EMPTY)
   {
   switch (dVarType)
      {
      case VT_I2:
         Value.i2 = (WORD)lValue;
         break;

      case VT_I4:
         Value.i4 = lValue;
         break;

      case VT_R4:
         _fmemcpy(&Value.r4, &lValue, sizeof(long));
         break;

      case VT_R8:
      case VT_DATE:
         _fmemcpy(&fValue, &lValue, sizeof(long));
         Value.r8 = (double)fValue;
         break;

      case VT_CURRENCY:
         break;

      case VT_STRING:
         Value.hlstr = (HLSTR)lValue;
         break;
      }

   VBSetVariantValue(lpVar, dVarType, &Value);
   }
}
#endif


long __SpreadGetItemData(HWND hWnd, long Col, long Row)
{
long lValue = 0L;

if (Col == SS_ALLCOLS && Row == SS_ALLROWS)
#ifndef SSF_NO_GetItemData
   lValue = SSGetUserData(hWnd);
#else
   lValue = 0;
#endif

else if (Col != SS_ALLCOLS)
#ifndef SSF_NO_GetColItemData
   SSGetColUserData(hWnd, (SS_COORD)Col, &lValue);
#else
   lValue = 0;
#endif

else if (Row != SS_ALLROWS)
#ifndef SSF_NO_GetRowItemData
   SSGetRowUserData(hWnd, (SS_COORD)Row, &lValue);
#else
   lValue = 0;
#endif

return (lValue);
}


LPTSTR VBAPI _export _SSDerefHlstrLen(HLSTR hlstr, LPUSHORT lpcbLen)
{
if (lpcbLen)
   *lpcbLen = VBGetHlstrLen(hlstr);

return (VBDerefHlstr(hlstr));
}


VOID VBAPI _export _SSFreeString(LPTSTR lpStr)
{
tbGlobalFree((TBGLOBALHANDLE)lpStr);
}


TBGLOBALHANDLE SSCreateString(LPTSTR lpszString, long lLen)
{
TBGLOBALHANDLE hString;

if (lLen <= 0)
   lLen = lstrlen(lpszString);

if (hString = tbGlobalAlloc(GMEM_MOVEABLE | GMEM_ZEROINIT, lLen + 1))
   {
   LPTSTR lpszDataTemp = (LPTSTR)tbGlobalLock(hString);

   _fmemcpy(lpszDataTemp, lpszString, (short)lLen);
   *(lpszDataTemp + lLen) = '\0';
   tbGlobalUnlock(hString);
   }

return (hString);
}


void FAR PASCAL _export SpreadSetCellVMargin(HCTL hCtl, short dYMargin)
{
LPSPREADSHEET lpSS = SS_Lock(VBGetControlHwnd(hCtl));

lpSS->dYMargin = dYMargin;

SS_Unlock(hWnd);
}

//--------------------------------------------------------------------

#ifndef SS_OLDCALC

short FAR PASCAL _export SpreadGetFormulaSync(HCTL hCtl)
{
  return SSGetBool(VBGetControlHwnd(hCtl), SSB_FORMULASYNC) ? -1 : 0;
}

short FAR PASCAL _export _SpreadGetFormulaSync(HWND hWnd)
{
  return SSGetBool(hWnd, SSB_FORMULASYNC);
}

//--------------------------------------------------------------------

void FAR PASCAL _export SpreadSetFormulaSync(HCTL hCtl, BOOL bSync)
{
  SSSetBool(VBGetControlHwnd(hCtl), SSB_FORMULASYNC, bSync);
}

void FAR PASCAL _export _SpreadSetFormulaSync(HWND hWnd, BOOL bSync)
{
  SSSetBool(hWnd, SSB_FORMULASYNC, bSync);
}

//--------------------------------------------------------------------

short FAR PASCAL _export SpreadGetRefStyle(HCTL hCtl)
{
  return SSGetRefStyle(VBGetControlHwnd(hCtl));
}

short FAR PASCAL _export _SpreadGetRefStyle(HWND hWnd)
{
  return SSGetRefStyle(hWnd);
}

//--------------------------------------------------------------------

void FAR PASCAL _export SpreadSetRefStyle(HCTL hCtl, short nRefStyle)
{
  SSSetRefStyle(VBGetControlHwnd(hCtl), nRefStyle);
}


void FAR PASCAL _export _SpreadSetRefStyle(HWND hWnd, short nRefStyle)
{
  SSSetRefStyle(hWnd, nRefStyle);
}

//--------------------------------------------------------------------

BOOL FAR PASCAL _export SpreadGetIteration(HCTL hCtl, short* lpnMaxIterations,
                                            double* lpdfMaxChange)
{
  BOOL bRet;
  bRet = SSGetIteration(VBGetControlHwnd(hCtl), lpnMaxIterations, lpdfMaxChange);
  return bRet ? -1: 0;
}

BOOL FAR PASCAL _export _SpreadGetIteration(HWND hWnd, short* lpnMaxIterations,
                                            double* lpdfMaxChange)
{
  return SSGetIteration(hWnd, lpnMaxIterations, lpdfMaxChange);
}

//--------------------------------------------------------------------

void FAR PASCAL _export SpreadSetIteration(HCTL hCtl, BOOL bIteration,
                                           short nMaxIterations,
                                           double dfMaxChange)
{
  SSSetIteration(VBGetControlHwnd(hCtl), bIteration, nMaxIterations, dfMaxChange);
}

void FAR PASCAL _export _SpreadSetIteration(HWND hWnd, BOOL bIteration,
                                            short nMaxIterations,
                                            double dfMaxChange)
{
  SSSetIteration(hWnd, bIteration, nMaxIterations, dfMaxChange);
}

//--------------------------------------------------------------------

static char szName[CALC_MAX_NAME_LEN];

HLSTR FAR PASCAL _export SpreadQueryCustomName(HCTL hCtl, HSZ lpszPrevName)
{
  SSQueryCustomName(VBGetControlHwnd(hCtl), lpszPrevName,
                    szName, CALC_MAX_NAME_LEN);
  return VBCreateHlstr(szName, lstrlen(szName));  
}

LPCSTR FAR PASCAL _export _SpreadQueryCustomName(HWND hWnd, LPCSTR lpszPrevName)
{
  SSQueryCustomName(hWnd, lpszPrevName, szName, CALC_MAX_NAME_LEN);
  return (LPTSTR)tbGlobalLock(SSCreateString(szName,-1));  
}

//--------------------------------------------------------------------

static char szValue[CALC_MAX_EXPR_LEN];

HLSTR FAR PASCAL _export SpreadGetCustomName(HCTL hCtl, HSZ lpszName)
{
  SSGetCustomName(VBGetControlHwnd(hCtl), lpszName, szValue, CALC_MAX_EXPR_LEN);
  return VBCreateHlstr(szValue, lstrlen(szValue));
}

LPCSTR FAR PASCAL _export _SpreadGetCustomName(HWND hWnd, LPCSTR lpszName)
{
  SSGetCustomName(hWnd, lpszName, szValue, CALC_MAX_EXPR_LEN);
  return (LPTSTR)tbGlobalLock(SSCreateString(szValue,-1));
}

//--------------------------------------------------------------------

BOOL FAR PASCAL _export SpreadSetCustomName(HCTL hCtl, HSZ lpszName,
                                            HSZ lpszValue)
{
  BOOL bRet = 0;
  bRet = SSSetCustomName(VBGetControlHwnd(hCtl), lpszName, lpszValue);
  return bRet ? -1: 0;
}

BOOL FAR PASCAL _export _SpreadSetCustomName(HWND hWnd, LPCSTR lpszName,
                                             LPCSTR lpszValue)
{
  BOOL bRet = 0;
  bRet = SSSetCustomName(hWnd, lpszName, lpszValue);
  return bRet ? -1: 0;
}

//--------------------------------------------------------------------

static char szFuncName[CALC_MAX_NAME_LEN];

BOOL FAR PASCAL _export SpreadEnumCustomFunction(HCTL hCtl, HSZ lpszPrevFuncName, HLSTR hlstrFuncName)
{
  BOOL bRet;
  
  bRet = SSEnumCustomFunction(VBGetControlHwnd(hCtl), lpszPrevFuncName, szFuncName, CALC_MAX_NAME_LEN);
  VBSetHlstr(&hlstrFuncName, szFuncName, lstrlen(szFuncName));
  return bRet ? -1 : 0;
}

BOOL FAR PASCAL _export _SpreadEnumCustomFunction(HWND hWnd, LPCSTR lpszPrevFuncName, HLSTR hlstrFuncName)
{
  BOOL bRet;
  
  bRet = SSEnumCustomFunction(hWnd, lpszPrevFuncName, szFuncName, CALC_MAX_NAME_LEN);
  VBSetHlstr(&hlstrFuncName, szFuncName, lstrlen(szFuncName));
  return bRet;
}

//--------------------------------------------------------------------

BOOL FAR PASCAL _export SpreadGetCustomFunction(HCTL hCtl, HSZ lpszFuncName, short FAR* lpnMinArgs, short FAR* lpnMaxArgs, long FAR *lplFlags)
{
  BOOL bRet;

  bRet = SSGetCustomFunction(VBGetControlHwnd(hCtl), lpszFuncName, lpnMinArgs, lpnMaxArgs, lplFlags);
  return bRet ? -1 : 0;
}

BOOL FAR PASCAL _export _SpreadGetCustomFunction(HWND hWnd, LPCSTR lpszFuncName, short FAR* lpnMinArgs, short FAR* lpnMaxArgs, long FAR *lplFlags)
{
  return SSGetCustomFunction(hWnd, lpszFuncName, lpnMinArgs, lpnMaxArgs, lplFlags);
}

//--------------------------------------------------------------------

BOOL FAR PASCAL _export SpreadRemoveCustomFunction(HCTL hCtl, HSZ lpszFuncName)
{
  BOOL bRet;
  
  bRet = SSRemoveCustomFunction(VBGetControlHwnd(hCtl), lpszFuncName);
  return bRet ? -1 : 0;
}

BOOL FAR PASCAL _export _SpreadRemoveCustomFunction(HWND hWnd, LPCSTR lpszFuncName)
{
  return SSRemoveCustomFunction(hWnd, lpszFuncName);
}

//--------------------------------------------------------------------

BOOL FAR PASCAL _export SpreadReCalcCell(HCTL hCtl, long lCol, long lRow)
{
  BOOL bRet;
  
  bRet = SSReCalcCell(VBGetControlHwnd(hCtl), lCol, lRow);
  return bRet ? -1 : 0;
}

BOOL FAR PASCAL _export _SpreadReCalcCell(HWND hWnd, long lCol, long lRow)
{
  return SSReCalcCell(hWnd, lCol, lRow);
}

#endif

//--------------------------------------------------------------------

#endif //not SS_OCX   //DBOCX
/**************************************************************************
 *
 *                      Functions needed for VBX & SS_OCX
 *
 *************************************************************************/
//DBOCX...
#ifndef SSF_NO_GetCellDirtyFlag
BOOL FAR PASCAL _export _SpreadGetCellDirtyFlag(HWND hWnd, SS_COORD Col, SS_COORD Row)
{
#ifdef SS_BOUNDCONTROL
LPSPREADSHEET lpSS;
LPSS_ROW      lpRow;
LPSS_CELL     lpCell;
BOOL          fDirty = FALSE;

if (Row >= 1)
   {
   lpSS = SS_SheetLock(hWnd);

   if (lpRow = SS_LockRowItem(lpSS, Row))
      {
      // RFW - 8/18/94
      fDirty = ((lpRow->bDirty == SS_ROWDIRTY_ALL ||
                 lpRow->bDirty == SS_ROWDIRTY_DEL) ? -1 : FALSE);

      if (Col >= 1)
         {
         if (lpCell = SSx_LockCellItem(lpSS, lpRow, Col, Row))
            {
            fDirty = lpCell->fDirty ? -1 : FALSE;
            SSx_UnlockCellItem(lpSS, lpRow, Col, Row);
            }
         }
		else if (!fDirty)
			{
			SS_COORD ColIndex;

			for (ColIndex = lpSS->Col.HeaderCnt; ColIndex < lpSS->Col.AllocCnt && !fDirty; ColIndex++)
				{
				if (lpCell = SSx_LockCellItem(lpSS, lpRow, ColIndex, Row))
					{
					fDirty = lpCell->fDirty ? -1 : FALSE;
					SSx_UnlockCellItem(lpSS, lpRow, ColIndex, Row);
					}
				}
			}

      SS_UnlockRowItem(lpSS, Row);
      }

   SS_SheetUnlock(hWnd);
   }

return (fDirty);
#else
return (FALSE);
#endif
}
#endif  //not SSF_NO_GetCellDirtyFlag


#ifndef SSF_NO_SetCellDirtyFlag
BOOL FAR PASCAL _export __SpreadSetCellDirtyFlag(LONG lObject, HWND hWnd, 
  SS_COORD Col, SS_COORD Row, BOOL fDirty)
{
#ifdef SS_BOUNDCONTROL
SS_HCTL       hCtl = (SS_HCTL)lObject;             //SSOCX
LPSPREADSHEET lpSS;
LPVBSPREAD    lpSpread = (LPVBSPREAD)VBDerefControl(hCtl);
LPSS_ROW      lpRow;
LPSS_CELL     lpCell;
BOOL          fRet = FALSE;

if (Row >= 1)
   {
   lpSS = SS_SheetLock(hWnd);

   if (lpRow = SS_AllocLockRow(lpSS, Row))
      {
      if (fDirty && lpRow->bDirty != SS_ROWDIRTY_DEL &&
          lpRow->bDirty != SS_ROWDIRTY_INS)
         {
         if (Col >= 1)
            lpRow->bDirty = SS_ROWDIRTY_SOME;
         else
            lpRow->bDirty = SS_ROWDIRTY_ALL;

         // RFW - 5/5/00 - GRB9167
         if (Row >= lpSS->Row.DataCnt)
            SS_SetDataRowCnt(lpSS, Row + 1);
         }

      if (Col >= 1)
         {
         if (lpCell = SS_AllocLockCell(lpSS, lpRow, Col, Row))
            {
            lpCell->fDirty = fDirty ? 1 : 0;
            SSx_UnlockCellItem(lpSS, lpRow, Col, Row);
            }
         }

      else if (!fDirty)
         lpRow->bDirty = 0;

      SS_UnlockRowItem(lpSS, Row);
      }

   if (fDirty && lpSpread->DataOpts.fAutoSave)
      VBSSBoundChangeMade(lpSS, lObject, hWnd, Col, Row);

   fRet = -1;
   SS_SheetUnlock(hWnd);
   }

return (fRet);
#else
return (FALSE);
#endif   //SSBOUNDCONTROL
}
#endif   //not SSF_NO_SetCellDirtyFlag


#ifndef SSF_NO_GetDataFillData
BOOL FAR PASCAL _export __SpreadGetDataFillData(HWND hWnd, LONG lObject, LPVAR lpVar, LPTSTR lpszText,
                             WORD wVarType)
{
SS_HCTL    hCtl = (SS_HCTL)lObject;            //SSOCX
LPVBSPREAD lpSpread;
DATAACCESS da;
LPTSTR     lpszData;
VALUE      Value;
BOOL       fIsRegistered;
BOOL       fRet = FALSE;
LPSS_BOOK lpBook = SS_BookLock(hWnd);
LPSPREADSHEET lpSS = SS_BookLockSheetIndex(lpBook, lpBook->nSheetSendingMsg);

lpSpread = (LPVBSPREAD)VBDerefControl(hCtl);
#ifdef SS_QE
if (lpSpread->DBInfo.fConnected)
   fRet = DBSS_GetDataFillData(hCtl, lpVar, lpszText, wVarType);

#endif

if (VBSSBoundIsBound(lObject, lpSS) && lpSpread->dDataGetFillCol != -1)
   {
   _fmemset(&da, 0, sizeof(da));
   da.usVersion = VB_VERSION;
   da.hctlData = (HCTL)ssVBGetDataSourceControl(lObject, &fIsRegistered);
   da.hctlBound = (HCTL)lObject;
   da.sDataFieldIndex = lpSpread->dDataGetFillCol;
   da.hlstrBookMark = lpSpread->hlstrDataFillBookMark;

   switch (wVarType)
      {
      case VT_I2:
         da.usDataType = DT_SHORT;
         break;

      case VT_I4:
         da.usDataType = DT_LONG;
         break;

      case VT_R4:
      case VT_R8:
         da.usDataType = DT_REAL;
         break;

      case VT_STRING:
      default:
         da.usDataType = DT_HLSTR;
         break;
      }

#ifdef FP_OCX
   if (lpSpread->fUReadNext)
      {
      if (DataCall(&da, GET, UREADFIELDVALUE, lpSS) != 0)
         return (FALSE);
      }
   else
#endif
      {
      if (DataCall(&da, GET, FIELDVALUE, lpSS) != 0)
         return (FALSE);
      }

   // BJO 30Oct97 JAP8051 - Begin fix
   if( da.fs & DA_fNull )
   {
     #ifdef SS_VB
     VBSetVariantValue(lpVar, VT_EMPTY, NULL);
     #else
     fpVBSetVariantValue(lpVar, VT_EMPTY, NULL);
     #endif
     return (-1);
   }
   // BJO 30Oct97 JAP8051 - End fix

   if (lpszText)
      {
      lpszData = ssVBDerefHlstr((HLSTR)da.lData);
      lstrcpy(lpszText, lpszData);
      ssVBDestroyHlstr((HLSTR)da.lData);
      return (-1);
      }

   /**************************************************
   * Convert the retrieved data to the variant value
   **************************************************/

   switch (wVarType)
      {
      case VT_I2:
         Value.i2 = (short)da.lData;
         break;

      case VT_I4:
         Value.i4 = da.lData;
         break;

      case VT_R4:
         Value.r4 = *(float *)&da.lData;
         break;

      case VT_R8:
         Value.r8 = (double)*(float *)&da.lData;
         break;

      case VT_STRING:
      default:
         wVarType = VT_STRING;
         Value.hlstr = (HLSTR)da.lData;
#ifdef SS_OCX  //DBOCX 
         // Convert from HLSTR to LPOLESTR (allocated by SysAllocString()).
     // Q. What about Bitmaps?
     Value.hlstr = (HLSTR)fpSysAllocBSTRFromTString((FPTSTR)da.lData);
     ssVBDestroyHlstr((HLSTR)da.lData);
#else
        {
        USHORT nLen;
        LPTSTR  lpData = VBDerefHlstrLen((HLSTR)da.lData, &nLen);

        Value.hlstr = VBCreateTempHlstr(lpData, nLen);
        ssVBDestroyHlstr((HLSTR)da.lData);
        }
#endif //SS_OCX
         break;
      }

#ifdef SS_VB
   VBSetVariantValue(lpVar, wVarType, &Value);
#else
   fpVBSetVariantValue(lpVar, wVarType, &Value);
#endif
#ifdef SS_OCX
   fpPatchVBVariant(lpVar);
#endif //SS_OCX
   fRet = -1;
   }

SS_BookUnlockSheetIndex(lpBook, lpBook->nSheetSendingMsg);
SS_BookUnlock(hWnd);

return (fRet);
}
#endif

#ifndef SSF_NO_SetDataFillData
BOOL FAR PASCAL _export __SpreadSetDataFillData(HWND hWnd, LONG lObject, LPVBSPREAD lpSpread, HLSTR hlstr, long lData, BOOL fUselData)
{
DATAACCESS da;
BOOL       fIsRegistered;
BOOL       fRet = FALSE;
LPSS_BOOK lpBook = SS_BookLock(hWnd);
LPSPREADSHEET lpSS = SS_BookLockSheetIndex(lpBook, lpBook->nSheetSendingMsg);

#ifdef SS_QE
if (lpSpread->DBInfo.fConnected)
   {
   LPTSTR lpszText = VBDerefHlstr(hlstr);
   xDB_SDK_Set_Field(lpSpread->DBInfo.hStmt, lpSpread->dDataSetFillCol, "",
                     lpszText);
   }
#endif

if (VBSSBoundIsBound(lObject, lpSS) && lpSpread->fUpdatable)
   {
   _fmemset(&da, 0, sizeof(da));
   da.usVersion = VB_VERSION;
   da.hctlData = (HCTL)ssVBGetDataSourceControl(lObject, &fIsRegistered);
   da.hctlBound = (HCTL)lObject;
   da.sDataFieldIndex = lpSpread->dDataSetFillCol;

   if( fUselData)
   {
      da.lData = lData;
      da.usDataType = DT_LONG;
   }
   else if( hlstr )
   {
      da.lData = (long)hlstr;
      da.usDataType = DT_HLSTR;
   }
   else
   {
     da.fs = DA_fNull;
     da.lData = (long)ssVBCreateHsz(lObject, _T(""));
	  da.usDataType = DT_HSZ;
   }
   DataCallwErrCoord(&da, SET, FIELDVALUE, lpSpread->Row, lpSpread->Col, lpSS);

   // This was added for OCX, should it also be called for VBX?  -SCP 12/95
   if( da.usDataType == DT_HSZ )
	  ssVBDestroyHsz((HSZ)da.lData);

   fRet = -1;
   }

if( hlstr )
   ssVBDestroyHlstr(hlstr);

SS_BookUnlockSheetIndex(lpBook, lpBook->nSheetSendingMsg);
SS_BookUnlock(hWnd);
return (fRet);
//DBOCX.
}
#endif

#if SS_V30

#ifdef SS_VB
BOOL FP_API SpreadSetFloat(HCTL hCtl, SS_COORD Col, SS_COORD Row, double dfValue)
{
HWND hWnd = VBSS_GethWnd(hCtl);

return (SSSetFloat(hWnd, Col, Row, dfValue));
}


BOOL FP_API SpreadGetFloat(HCTL hCtl, SS_COORD Col, SS_COORD Row, LPDOUBLE lpdfValue)
{
HWND hWnd = VBSS_GethWnd(hCtl);

return (SSGetFloat(hWnd, Col, Row, lpdfValue));
}


BOOL FP_API SpreadSetInteger(HCTL hCtl, SS_COORD Col, SS_COORD Row, long lValue)
{
HWND hWnd = VBSS_GethWnd(hCtl);

return (SSSetInteger(hWnd, Col, Row, lValue));
}


BOOL FP_API SpreadGetInteger(HCTL hCtl, SS_COORD Col, SS_COORD Row, LPLONG lplValue)
{
HWND hWnd = VBSS_GethWnd(hCtl);

return (SSGetInteger(hWnd, Col, Row, lplValue));
}


BOOL FAR PASCAL _export _SpreadSetActionKey(HWND hWnd, WORD wAction, BOOL fShift,
                                            BOOL fCtrl, WORD wKey);
BOOL FAR PASCAL _export _SpreadGetActionKey(HWND hWnd, WORD wAction, LPBOOL lpfShift,
                                            LPBOOL lpfCtrl, LPWORD lpwKey);
BOOL FAR PASCAL _export _SpreadSetOddEvenRowColor(HWND hWnd, COLORREF clrBackOdd, COLORREF clrForeOdd, COLORREF clrBackEven, COLORREF clrForeEven);
BOOL FAR PASCAL _export _SpreadGetOddEvenRowColor(HWND hWnd, LPCOLORREF lpclrBackOdd, LPCOLORREF lpclrForeOdd, LPCOLORREF lpclrBackEven, LPCOLORREF lpclrForeEven);


#ifndef SSF_NO_SetActionKey
BOOL FAR PASCAL _export SpreadSetActionKey(HCTL hCtl, WORD wAction, BOOL fShift,
                                           BOOL fCtrl, WORD wKey)
{
HWND hWnd = VBSS_GethWnd(hCtl);

return (_SpreadSetActionKey(hWnd, wAction, fShift, fCtrl, wKey));
}


BOOL FAR PASCAL _export _SpreadSetActionKey(HWND hWnd, WORD wAction, BOOL fShift,
                                            BOOL fCtrl, WORD wKey)
{
return (SSSetActionKey(hWnd, wAction, fShift, fCtrl, wKey));
}
#endif


#ifndef SSF_NO_GetActionKey
BOOL FAR PASCAL _export SpreadGetActionKey(HCTL hCtl, WORD wAction, LPBOOL lpfShift,
                                           LPBOOL lpfCtrl, LPWORD lpwKey)
{
HWND hWnd = VBSS_GethWnd(hCtl);

return (_SpreadGetActionKey(hWnd, wAction, lpfShift, lpfCtrl, lpwKey));
}


BOOL FAR PASCAL _export _SpreadGetActionKey(HWND hWnd, WORD wAction, LPBOOL lpfShift,
                                            LPBOOL lpfCtrl, LPWORD lpwKey)
{
return (SSGetActionKey(hWnd, wAction, lpfShift, lpfCtrl, lpwKey));
}
#endif


#ifndef SSF_NO_SetOddEvenRowColor
BOOL FAR PASCAL _export SpreadSetOddEvenRowColor(HCTL hCtl, COLORREF clrBackOdd, COLORREF clrForeOdd, COLORREF clrBackEven, COLORREF clrForeEven)
{
HWND hWnd = VBSS_GethWnd(hCtl);

return (_SpreadSetOddEvenRowColor(hWnd, clrBackOdd, clrForeOdd, clrBackEven,
                                  clrForeEven));
}


BOOL FAR PASCAL _export _SpreadSetOddEvenRowColor(HWND hWnd, COLORREF clrBackOdd, COLORREF clrForeOdd, COLORREF clrBackEven, COLORREF clrForeEven)
{
return (SSSetOddEvenRowColor(hWnd, clrBackOdd, clrForeOdd, clrBackEven,
                             clrForeEven));
}
#endif


#ifndef SSF_NO_GetOddEvenRowColor
BOOL FAR PASCAL _export SpreadGetOddEvenRowColor(HCTL hCtl, LPCOLORREF lpclrBackOdd, LPCOLORREF lpclrForeOdd, LPCOLORREF lpclrBackEven, LPCOLORREF lpclrForeEven)
{
HWND hWnd = VBSS_GethWnd(hCtl);

return (_SpreadGetOddEvenRowColor(hWnd, lpclrBackOdd, lpclrForeOdd,
                                  lpclrBackEven, lpclrForeEven));
}


BOOL FAR PASCAL _export _SpreadGetOddEvenRowColor(HWND hWnd, LPCOLORREF lpclrBackOdd, LPCOLORREF lpclrForeOdd, LPCOLORREF lpclrBackEven, LPCOLORREF lpclrForeEven)
{
return (SSGetOddEvenRowColor(hWnd, lpclrBackOdd, lpclrForeOdd, lpclrBackEven,
                             lpclrForeEven));
}
#endif


#ifndef SSF_NO_SetCalText
void FAR PASCAL _export SpreadSetCalText(LPCTSTR lpszShortDays, LPCTSTR lpszLongDays,
                                         LPCTSTR lpszShortMonths, LPCTSTR lpszLongMonths,
                                         LPCTSTR lpszOkText, LPCTSTR lpszCancelText)
{
SSSetCalText(lpszShortDays, lpszLongDays, lpszShortMonths, lpszLongMonths,
             lpszOkText, lpszCancelText);
}
#endif


#ifndef SSF_NO_SetTextTipAppearance
BOOL FAR PASCAL _export SpreadSetTextTipAppearance(HCTL hCtl, HLSTR hlstrFontName,
                        short nFontSize, BOOL fFontBold, BOOL fFontItalic,
                        COLORREF clrBack, COLORREF clrFore)
{
HWND       hWnd = VBSS_GethWnd(hCtl);
FP_TT_INFO Info;
LPTSTR     lpszFontName;
WORD       wItems = FP_TT_ITEM_BACKCOLOR | FP_TT_ITEM_FORECOLOR;

lpszFontName = ssVBDerefZeroTermHlstr(hlstrFontName);

if (lpszFontName && *lpszFontName)
   {
   _fmemset(&Info.LogFont, '\0', sizeof(LOGFONT));
   Info.LogFont.lfHeight = PT_TO_PIXELS(nFontSize);
   Info.LogFont.lfWeight = (fFontBold ? FW_BOLD : FW_REGULAR);
   Info.LogFont.lfItalic = (BYTE)fFontItalic;
   Info.LogFont.lfCharSet = DEFAULT_CHARSET;

   lstrcpyn(Info.LogFont.lfFaceName, lpszFontName, LF_FACESIZE);
   wItems |= FP_TT_ITEM_FONT;
   }

Info.clrBack = clrBack;
Info.clrFore = clrFore;

return (fpTextTipSetInfo(hWnd, wItems, &Info));
}
#endif


#ifndef SSF_NO_SetTextTipAppearance
BOOL FAR PASCAL _export SpreadGetTextTipAppearance(HCTL hCtl, HLSTR hlstrFontName,
                        LPSHORT lpnFontSize, LPBOOL lpfFontBold, LPBOOL lpfFontItalic,
                        LPCOLORREF lpclrBack, LPCOLORREF lpclrFore)
{
HWND       hWnd = VBSS_GethWnd(hCtl);
FP_TT_INFO Info;
BOOL       fRet;

fRet = fpTextTipGetInfo(hWnd, &Info);

*lpclrBack = Info.clrBack;
*lpclrFore = Info.clrFore;

#ifndef SS_OCX
VBSetHlstr(&hlstrFontName, Info.LogFont.lfFaceName,   // Not Done
           lstrlen(Info.LogFont.lfFaceName));
#endif
*lpnFontSize = (short)PIXELS_TO_PT(Info.LogFont.lfHeight);
*lpfFontBold = (Info.LogFont.lfWeight > FW_REGULAR);
*lpfFontItalic = Info.LogFont.lfItalic;

return (fRet);
}
#endif


BOOL FP_API SpreadSetPrintOptions(HCTL hCtl, short bSmartPrint, short nPageOrder, long lFirstPageNumber)
{
LPVBSPREAD lpSpread = (LPVBSPREAD)VBDerefControl(hCtl);

lpSpread->Print.fSmartPrint = (BOOL)bSmartPrint;
if (nPageOrder >= 0 && nPageOrder <= 2)
   lpSpread->Print.nPageOrder = nPageOrder;
lpSpread->Print.lFirstPageNumber = lFirstPageNumber;

return (TRUE);
}


BOOL FP_API SpreadGetPrintOptions(HCTL hCtl, LPSHORT lpbSmartPrint, LPSHORT lpnPageOrder, LPLONG lplFirstPageNumber)
{
LPVBSPREAD lpSpread = (LPVBSPREAD)VBDerefControl(hCtl);

if (lpbSmartPrint)
   *lpbSmartPrint = (SHORT)lpSpread->Print.fSmartPrint;
if (lpnPageOrder)
   *lpnPageOrder = lpSpread->Print.nPageOrder;
if (lplFirstPageNumber)
   *lplFirstPageNumber = lpSpread->Print.lFirstPageNumber;

return (TRUE);
}


long FP_API SpreadGetPrintPageCount(HCTL hCtl)
{
PRVW_INFO PrvwInfo;
HWND      hWnd = VBSS_GethWnd(hCtl);

VBSSPrintPreviewGetInfo(hCtl, hWnd, &PrvwInfo);

return PrvwInfo.lPageEnd;
}


long FP_API SpreadGetNextPageBreakCol(HCTL hCtl, long lPrevCol)
{
LPVBSPREAD       lpSpread = (LPVBSPREAD)VBDerefControl(hCtl);
HWND             hWnd = VBSS_GethWnd(hCtl);
SS_PRINTPAGECALC PrintPageCalc;

PrintPageCalc.lPageCnt = -1;
VBSS_Print(hCtl, hWnd, lpSpread, SS_PRINT_ACTION_NEXTCOL, 0, NULL, lPrevCol,
           &PrintPageCalc);

return (PrintPageCalc.lPageCnt);
}


long FP_API SpreadGetNextPageBreakRow(HCTL hCtl, long lPrevRow)
{
LPVBSPREAD       lpSpread = (LPVBSPREAD)VBDerefControl(hCtl);
HWND             hWnd = VBSS_GethWnd(hCtl);
SS_PRINTPAGECALC PrintPageCalc;

PrintPageCalc.lPageCnt = -1;
VBSS_Print(hCtl, hWnd, lpSpread, SS_PRINT_ACTION_NEXTROW, 0, NULL, lPrevRow,
           &PrintPageCalc);

return (PrintPageCalc.lPageCnt);
}


BOOL FP_API SpreadSetArray(HCTL hCtl, SS_COORD ColLeft, SS_COORD RowTop, HAD hAD)
{
return (_SpreadArray(hCtl, ColLeft, RowTop, hAD, FALSE));
}


BOOL FP_API SpreadGetArray(HCTL hCtl, SS_COORD ColLeft, SS_COORD RowTop, HAD hAD)
{
return (_SpreadArray(hCtl, ColLeft, RowTop, hAD, TRUE));
}


BOOL _SpreadArray(HCTL hCtl, SS_COORD ColLeft, SS_COORD RowTop, HAD hAD, BOOL fGet)
{
HWND hWnd = VBSS_GethWnd(hCtl);
BOOL fRet = FALSE;

if (hAD)
   {
   long lIndexCnt = fpVBArrayIndexCount(hAD);

   if (lIndexCnt >= 1 && lIndexCnt <= 2)
      {
      long lBounds;

      lBounds = fpVBArrayBounds(hAD, (short)lIndexCnt);
      if (lBounds != AB_INVALIDINDEX)
         {
         long lRowLB = LOWORD(lBounds);
         long lRowUB = HIWORD(lBounds);
         long lColLB = 0;
         long lColUB = 0;
         long Row;
         long Col;

         fRet = -1;
         if (lIndexCnt == 2)
            {
            lBounds = VBArrayBounds(hAD, 1);
            if (lBounds != AB_INVALIDINDEX)
               {
               lColLB = LOWORD(lBounds);
               lColUB = HIWORD(lBounds);
               }
            else
               fRet = FALSE;
            }

         if (fRet)
            {
            long  lSize = (long)fpVBArrayElemSize(hAD);
            LPTSTR lpszPtr = fpVBArrayFirstElem(hAD);

            for (Col = 0; Col <= lColUB - lColLB; Col++)
               {
               for (Row = 0; Row <= lRowUB - lRowLB; Row++)
                  {
                  if (fGet)
                     SpreadGetText(hCtl, ColLeft + Col, RowTop + Row,
                                   (LPVAR)lpszPtr);
                  else
                     __SpreadSetText(hWnd, ColLeft + Col, RowTop + Row,
                                     (LPVAR)lpszPtr);

                  lpszPtr += lSize;
                  }
               }
            }
         }
      }
   }

return (fRet);
}


short FP_API SpreadGetTwoDigitYearMax(HCTL hCtl)
{
HWND hWnd = VBSS_GethWnd(hCtl);

return (SSGetTwoDigitYearMax(hWnd));
}


BOOL FP_API SpreadSetTwoDigitYearMax(HCTL hCtl, short nTwoDigitYearMax)
{
HWND hWnd = VBSS_GethWnd(hCtl);

return (SSSetTwoDigitYearMax(hWnd, nTwoDigitYearMax));
}

#endif // SS_VB

#endif // SS_V30

HWND VBSS_GethWnd(HCTL hCtl)
{
HWND hWnd;

#if defined SS_VB
if (VBGetVersion() < VB200_VERSION)
   hWnd = (HWND)(long)(LPVOID)hCtl;
else
   hWnd = VBGetControlHwnd(hCtl);
#elif defined SS_OCX
{
FPCONTROL fpTemp = {0, 0, 0L, 0L};
fpTemp.lpDispatch = *(LPLONG)hCtl;
hWnd = fpVBGetControlHwnd((LPFPCONTROL)&fpTemp);
}
#endif

return (hWnd);
}


// MSVC 1.52 does not like C files to end with an "endif" or comment.
;
