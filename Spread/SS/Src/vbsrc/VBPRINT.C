
/*********************************************************
* VBPRINT.C
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

#include <windows.h>
//#include <vbapi.h>
#include <math.h>
#include <ctype.h>
#include <string.h>
#include "..\sssrc\spread\spread.h"
#include "fphdc.h"

#ifndef SS_NOPRINT

#include "..\sssrc\spread\ss_alloc.h"
#include "..\sssrc\spread\ss_save4.h"
#include "toolbox.h"
#include "vbspread.h"
#include "vbprint2.h"

#define SSM_POSTPRINTMSG WM_USER

typedef unsigned int size_t;
void _far * _far _cdecl _fmemcpy(void _far *, const void _far *, size_t);

extern HANDLE hDynamicInst;

#ifdef SS_V30
BOOL FAR PASCAL _export vbSpreadPrintProc(HDC hDC, short dCommand,
                                          short nPageNum, long lAppData,
                                          LPSS_PRINTINFO lpPrintInfo);
#else
BOOL FAR PASCAL _export vbSpreadPrintProc(HDC hDC, short dCommand,
                                          short nPageNum, long lAppData);
#endif
short                   vbSpreadCalcRows(HDC hDC, GLOBALHANDLE hText,
                                         WORD wHeightDef);

LONG vbSpreadPrintAction(HCTL hCtl, HWND hWnd, LPVBSPREAD lpSpread)
{
return (VBSS_Print(hCtl, hWnd, lpSpread, SS_PRINT_ACTION_PRINT, 0, NULL, 0,
                   NULL, NULL, NULL, NULL, NULL));
}


LONG VBSS_Print(HCTL hCtl, HWND hWnd, LPVBSPREAD lpSpread, WORD wAction,
                HDC hDCPreview, LPRECT lpRectPreview, long lXtra,
                LPSS_PRINTPAGECALC lpPrintPageCalc)
{
HANDLE          hPrintAbortMsg = 0;
PRINTABORTTABLE PrintAbortItem;
PRINTABORTPARAM PrintAbortParam;
SS_PRINTFUNC    lpfnPrintProcInst = 0;
FARPROC         lpfnPrintDlgProcInst = 0;
ABORTPROC       lpfnPrintAbortProcInst = 0;
SS_PRINTFORMAT  PrintFormat;
LPSTR           lpszPrintJobName;
LPSTR           lpszPrintAbortMsg;
LPSTR           lpszPrintAbortMsgTemp;
HCURSOR         hCursor;
HWND            hWndPrintDlgProc = 0;
BOOL            Ret = TRUE;

_fmemset(&PrintFormat, '\0', sizeof(SS_PRINTFORMAT));

PrintFormat.fDrawBorder = lpSpread->Print.PrintBorder;
PrintFormat.fShowGrid = lpSpread->Print.PrintGrid;
PrintFormat.fShowColHeaders = lpSpread->Print.PrintColHeaders;
PrintFormat.fShowRowHeaders = lpSpread->Print.PrintRowHeaders;
PrintFormat.fUseDataMax = lpSpread->Print.PrintUseDataMax;
PrintFormat.x1CellRange = lpSpread->Col;
PrintFormat.y1CellRange = lpSpread->Row;
PrintFormat.x2CellRange = lpSpread->Col2;
PrintFormat.y2CellRange = lpSpread->Row2;
PrintFormat.nPageStart = lpSpread->Print.PrintPageStart;
PrintFormat.nPageEnd = lpSpread->Print.PrintPageEnd;
PrintFormat.hDCPrinter = (HDC)lpSpread->Print.PrinthDCPrinter;
PrintFormat.fDrawShadows = lpSpread->Print.PrintShadows;
PrintFormat.fDrawColors = lpSpread->Print.PrintColor;
PrintFormat.wOrientation = lpSpread->Print.PrintOrientation;
PrintFormat.fSmartPrint = (BYTE)lpSpread->Print.fSmartPrint;

switch (lpSpread->Print.PrintType)
   {
   case 1:
      VBSSVALCOLROW(lpSpread);
      VBSSVALCOL2ROW2(lpSpread);
      PrintFormat.dPrintType = SS_PRINTTYPE_CELLRANGE;
      break;

   case 2:
      PrintFormat.dPrintType = SS_PRINTTYPE_CURRENTPAGE;
      break;

   case 3:
      PrintFormat.dPrintType = SS_PRINTTYPE_PAGERANGE;
      break;
   }

if (wAction == SS_PRINT_ACTION_PREVIEW)
   {
   PrintFormat.dPrintType |= SS_PRINTTYPE_PAGERANGE;
   PrintFormat.nPageStart = (short)lXtra;
   PrintFormat.nPageEnd = (short)lXtra;
   }
else if (wAction == SS_PRINT_ACTION_NEXTCOL)
   {
   PrintFormat.dPrintType |= SS_PRINTTYPE_CELLRANGE;
   PrintFormat.x1CellRange = lXtra;
   PrintFormat.x2CellRange = -1;
   PrintFormat.y1CellRange = -1;
   PrintFormat.y2CellRange = -1;
   }
else if (wAction == SS_PRINT_ACTION_NEXTROW)
   {
   PrintFormat.dPrintType |= SS_PRINTTYPE_CELLRANGE;
   PrintFormat.y1CellRange = lXtra;
   PrintFormat.y2CellRange = -1;
   PrintFormat.x1CellRange = -1;
   PrintFormat.x2CellRange = -1;
   }

switch (lpSpread->Print.nPageOrder)
   {
   case 1: PrintFormat.dPrintType |= SS_PRINTTYPE_DOWNTHENOVER; break;
   case 2: PrintFormat.dPrintType |= SS_PRINTTYPE_OVERTHENDOWN; break;
   }

PrintFormat.fMarginLeft = (float)lpSpread->Print.PrintMarginLeft /
                          (float)1440.0;
PrintFormat.fMarginTop = (float)lpSpread->Print.PrintMarginTop /
                         (float)1440.0;
PrintFormat.fMarginRight = (float)lpSpread->Print.PrintMarginRight /
                           (float)1440.0;
PrintFormat.fMarginBottom = (float)lpSpread->Print.PrintMarginBottom /
                            (float)1440.0;

lpfnPrintProcInst = (SS_PRINTFUNC)MakeProcInstance((FARPROC)vbSpreadPrintProc, hDynamicInst);

if (wAction == SS_PRINT_ACTION_PRINT)
   {
   if (lpSpread->Print.hPrintJobName)
      lpszPrintJobName = (LPSTR)GlobalLock(lpSpread->Print.hPrintJobName);
   else
      lpszPrintJobName = "Spread";

   if (lpSpread->Print.hPrintAbortMsg)
      {
      lpszPrintAbortMsg = (LPSTR)GlobalLock(lpSpread->Print.hPrintAbortMsg);
      if (hPrintAbortMsg = GlobalAlloc(GMEM_MOVEABLE | GMEM_ZEROINIT,
                                       lstrlen(lpszPrintAbortMsg) + 1))
         {
         lpszPrintAbortMsgTemp = (LPSTR)GlobalLock(hPrintAbortMsg);
         lstrcpy(lpszPrintAbortMsgTemp, lpszPrintAbortMsg);
         GlobalUnlock(hPrintAbortMsg);
         }

      lpfnPrintAbortProcInst = (ABORTPROC)MakeProcInstance((FARPROC)vbSSPrintAbortProc,
                                                hDynamicInst);

      lpfnPrintDlgProcInst = MakeProcInstance((FARPROC)vbSSPrintDlgProc,
                                              hDynamicInst);

      PrintAbortParam.hAbortMsg = hPrintAbortMsg;
      PrintAbortParam.hWndSpread = hWnd;

      hWndPrintDlgProc = CreateDialogParam(hDynamicInst,
#ifdef SPREAD_JPN
                                             "vbSSPrintDlgBox_Jpn",
#else
											"vbSSPrintDlgBox",
#endif
                                           hWnd, lpfnPrintDlgProcInst,
                                           (long)(LPSTR)&PrintAbortParam);
      lpSpread->Print.hWndPrintDlgProc = hWndPrintDlgProc;
      vbSSAddPrintAbortItem(lpSpread->Print.hWndPrintDlgProc, 0);

      GlobalUnlock(lpSpread->Print.hPrintAbortMsg);
      }
   else
      lpfnPrintAbortProcInst = 0;
   }

hCursor = SetCursor(LoadCursor(NULL, IDC_WAIT));
ShowCursor(TRUE);

Ret = SS_Print(hWnd, lpszPrintJobName, &PrintFormat, lpfnPrintProcInst,
               (long)lpSpread, lpfnPrintAbortProcInst, wAction, hDCPreview,
               lpRectPreview, lpPrintPageCalc, NULL, NULL, NULL, NULL);

ShowCursor(FALSE);
SetCursor(hCursor);

if (wAction == SS_PRINT_ACTION_PRINT)
   {
   if (lpSpread->Print.hPrintJobName)
      GlobalUnlock(lpSpread->Print.hPrintJobName);

   if (hPrintAbortMsg)
      {
      if (vbSSGetPrintAbortItem(&PrintAbortItem, 0, hWndPrintDlgProc))
         {
         DestroyWindow(PrintAbortItem.hWndDlg);
         vbSSDeletePrintAbortItem(PrintAbortItem.hWndDlg);
         }

      if (hPrintAbortMsg)
         GlobalFree(hPrintAbortMsg);

      if (lpfnPrintAbortProcInst)
         FreeProcInstance(lpfnPrintAbortProcInst);
      if (lpfnPrintDlgProcInst)
         FreeProcInstance(lpfnPrintDlgProcInst);
      }
   }

if (lpfnPrintProcInst)
   FreeProcInstance(lpfnPrintProcInst);
}



#ifdef SS_V30
BOOL FAR PASCAL _export vbSpreadPrintProc(HDC hDC, short dCommand,
                                          short nPageNum, long lAppData,
                                          LPSS_PRINTINFO lpPrintInfo)
#else
BOOL FAR PASCAL _export vbSpreadPrintProc(HDC hDC, short dCommand,
                                          short nPageNum, long lAppData)
#endif
{
LPVBSPREAD lpSpread = (LPVBSPREAD)lAppData;
#ifdef SS_V30
//RAP -- Not sure why fSmartPrint was used to determine the page number. 
//Modified it to use lFirstPageNumber instead.
//  nPageNum += (short)(lpSpread->Print.fSmartPrint - 1);
  nPageNum += (max(1, (short)(lpSpread->Print.lFirstPageNumber))-1);
#else
  LPSS_PRINTINFO lpPrintInfo = NULL;
#endif

switch (dCommand)
   {
   case SS_PRINTCMD_HEADERLEN:
      if (lpSpread->Print.hPrintAbortMsg)
         vbSSAddPrintAbortItem(lpSpread->Print.hWndPrintDlgProc, hDC);

      if (lpSpread->Print.hPrintHeader)
         return (vbSpreadPrintHeaderFooter(hDC, lpSpread->Print.hPrintHeader,
                                           nPageNum,
                                           lpSpread->Print.PrintMarginRight,
                                           TRUE, lpPrintInfo));

      return (0);

   case SS_PRINTCMD_FOOTERLEN:
      if (lpSpread->Print.hPrintFooter)
         return (vbSpreadPrintHeaderFooter(hDC, lpSpread->Print.hPrintFooter,
                                           nPageNum,
                                           lpSpread->Print.PrintMarginRight,
                                           TRUE, lpPrintInfo));

      return (0);

   case SS_PRINTCMD_PRINTHEADER:
      if (lpSpread->Print.hPrintHeader)
         vbSpreadPrintHeaderFooter(hDC, lpSpread->Print.hPrintHeader,
                                   nPageNum, lpSpread->Print.PrintMarginRight,
                                   FALSE, lpPrintInfo);

      break;

   case SS_PRINTCMD_PRINTFOOTER:
      if (lpSpread->Print.hPrintFooter)
         vbSpreadPrintHeaderFooter(hDC, lpSpread->Print.hPrintFooter,
                                   nPageNum, lpSpread->Print.PrintMarginRight,
                                   FALSE, lpPrintInfo);

      break;

   }

return (0);
}


#if 0

short vbSpreadCalcRows(HDC hDC, GLOBALHANDLE hText, WORD wHeightDef)
{
LPSTR lpszText;
LPSTR Ptr;
LPSTR PtrTemp;
short dRowHeightList[10];
short Height = 0;
short HeightRow;
short HeightRowCurrent;

_fmemset((LPVOID)dRowHeightList, '\0', sizeof(dRowHeightList));

HeightRow = wHeightDef;
HeightRowCurrent = wHeightDef;
dRowHeightList[0] = wHeightDef;

if (hText)
   {
   lpszText = (LPSTR)GlobalLock(hText);

   Ptr = lpszText;
   while (Ptr = StrChr(Ptr, '/'))
      {
      Ptr++;

      if (tolower(*Ptr) == 'n')
         {
         Height += HeightRowCurrent;
         HeightRowCurrent = HeightRow;

         Ptr++;
         }

      else if (tolower(*Ptr) == 'f')
         {
         Ptr++;

         switch (tolower(*Ptr))
            {
            case 'z':
               Ptr++;

               if (*Ptr == '\"')
                  {
                  Ptr++;

                  if (PtrTemp = StrChr(Ptr, '\"'))
                     {
                     HeightRow = max(HeightRow, PT_TO_PIXELS_EX(hDC,
                                     StringToInt(Ptr)));
                     Ptr = PtrTemp + 1;
                     }
                  }

               break;

            case 's':            // Font save
               Ptr++;

               if (*Ptr >= '1' && *Ptr <= '9')
                  dRowHeightList[*Ptr - '0'] = HeightRow;

               Ptr++;
               break;

            default:
               if (*Ptr >= '0' && *Ptr <= '9')
                  {
                  if (dRowHeightList[*Ptr - '0'])
                     HeightRow = dRowHeightList[*Ptr - '0'];

                  Ptr++;
                  }

               break;
            }

         HeightRowCurrent = max(HeightRowCurrent, HeightRow);
         }
      }

   GlobalUnlock(hText);
   }

Height += HeightRowCurrent;

return (Height);
}

#endif

//--------------------------------------------------------------------

BOOL SS_VbxResetOnLoad(LPSPREADSHEET lpSS)
{
  LPVBSPREAD lpSpread = (LPVBSPREAD)VBDerefControl(VBGetHwndControl(lpSS->lpBook->hWnd));
  if( lpSpread->Print.hPrintAbortMsg )
    GlobalFree(lpSpread->Print.hPrintAbortMsg);
  if( lpSpread->Print.hPrintFooter )
    GlobalFree(lpSpread->Print.hPrintFooter);
  if( lpSpread->Print.hPrintHeader )
    GlobalFree(lpSpread->Print.hPrintHeader);
  if( lpSpread->Print.hPrintJobName )
    GlobalFree(lpSpread->Print.hPrintJobName);
  lpSpread->CalcDependencies = TRUE;
  lpSpread->Print.PrintBorder = TRUE;
  lpSpread->Print.PrintColor = FALSE;
  lpSpread->Print.PrintShadows = TRUE;
  lpSpread->Print.PrintGrid = TRUE;
  lpSpread->Print.PrintColHeaders = TRUE;
  lpSpread->Print.PrintRowHeaders = TRUE;
  lpSpread->Print.PrintUseDataMax = TRUE;
  lpSpread->Print.PrintType = 0;
  lpSpread->Print.PrintPageEnd = 1;
  lpSpread->Print.PrintPageStart = 1;
  lpSpread->Print.PrintMarginLeft = 0;
  lpSpread->Print.PrintMarginTop = 0;
  lpSpread->Print.PrintMarginBottom = 0;
  lpSpread->Print.PrintMarginRight = 0;
  lpSpread->Print.PrintOrientation = 0;
  lpSpread->Print.hPrintAbortMsg = NULL;
  lpSpread->Print.hPrintFooter = NULL;
  lpSpread->Print.hPrintHeader = NULL;
  lpSpread->Print.hPrintJobName = NULL;
#ifdef SS_V30
  lpSpread->Print.fSmartPrint = FALSE;
  lpSpread->Print.nPageOrder = 0;
  lpSpread->Print.lFirstPageNumber = 1;
#endif
  return TRUE;
}

//--------------------------------------------------------------------

BOOL SS_VbxLoad4CalcDepend(LPSPREADSHEET lpSS, LPSS_REC_CALCDEPEND lpRec)
{
  LPVBSPREAD lpSpread = (LPVBSPREAD)VBDerefControl(VBGetHwndControl(lpSS->lpBook->hWnd));
  lpSpread->CalcDependencies = lpRec->bCalcDepend;
  return TRUE;
}

//--------------------------------------------------------------------

BOOL SS_VbxSave4CalcDepend(LPSPREADSHEET lpSS, LPSS_BUFF lpBuff)
{
  LPVBSPREAD lpSpread = (LPVBSPREAD)VBDerefControl(VBGetHwndControl(lpSS->lpBook->hWnd));
  SS_REC_CALCDEPEND rec;
  BOOL bRet = TRUE;
  if( lpSpread->CalcDependencies != TRUE )
  {
    rec.nRecType = SS_RID_CALCDEPEND;
    rec.lRecLen = sizeof(rec);
    rec.bCalcDepend = lpSpread->CalcDependencies;
    bRet = SS_BuffAppend(lpBuff, &rec, sizeof(rec));
  }
  return bRet;
}

//--------------------------------------------------------------------

static HGLOBAL SS_CreateStrHandle(LPSTR lpszText)
{
  HGLOBAL hRet = 0;
  LPSTR lpszRet;
  if( hRet = GlobalAlloc(GHND, lstrlen(lpszText)) )
  {
    lpszRet = (LPSTR)GlobalLock(hRet);
    lstrcpy(lpszRet, lpszText);
    GlobalUnlock(hRet);
  }
  return hRet;
}

//--------------------------------------------------------------------

BOOL SS_VbxLoad4Print(LPSPREADSHEET lpSS, LPSS_REC_PRINT lpRec)
{
  LPVBSPREAD lpSpread = (LPVBSPREAD)VBDerefControl(VBGetHwndControl(lpSS->lpBook->hWnd));
  LPBYTE lpAbortMsg = (LPBYTE)lpRec + sizeof(*lpRec);
  LPBYTE lpFooter = (LPBYTE)lpAbortMsg + lpRec->lAbortMsgLen;
  LPBYTE lpHeader = (LPBYTE)lpFooter + lpRec->lFooterLen;
  LPBYTE lpJobName = (LPBYTE)lpHeader + lpRec->lHeaderLen;
  lpSpread->Print.PrintBorder = lpRec->bDrawBorder;
  lpSpread->Print.PrintColor = lpRec->bDrawColor;
  lpSpread->Print.PrintShadows = lpRec->bDrawShadows;
  lpSpread->Print.PrintGrid = lpRec->bShowGrid;
  lpSpread->Print.PrintColHeaders = lpRec->bShowColHeaders;
  lpSpread->Print.PrintRowHeaders = lpRec->bShowRowHeaders;
  lpSpread->Print.PrintUseDataMax = lpRec->bUseDataMax;
  lpSpread->Print.PrintType = lpRec->nPrintType;
  lpSpread->Print.PrintPageEnd = lpRec->nPageEnd;
  lpSpread->Print.PrintPageStart = lpRec->nPageStart;
  lpSpread->Print.PrintMarginLeft = lpRec->lMarginLeft;
  lpSpread->Print.PrintMarginTop = lpRec->lMarginTop;
  lpSpread->Print.PrintMarginBottom = lpRec->lMarginBottom;
  lpSpread->Print.PrintMarginRight = lpRec->lMarginRight;
  lpSpread->Print.PrintOrientation = lpRec->nOrientation;
  if( lpRec->lAbortMsgLen )
    lpSpread->Print.hPrintAbortMsg = SS_CreateStrHandle((LPSTR)lpAbortMsg);
  else
    lpSpread->Print.hPrintAbortMsg = NULL;
  if( lpRec->lFooterLen )
    lpSpread->Print.hPrintFooter = SS_CreateStrHandle((LPSTR)lpFooter);
  else
    lpSpread->Print.hPrintFooter = NULL;
  if( lpRec->lHeaderLen )
    lpSpread->Print.hPrintHeader = SS_CreateStrHandle((LPSTR)lpHeader);
  else
    lpSpread->Print.hPrintHeader = NULL;
  if( lpRec->lJobNameLen )
    lpSpread->Print.hPrintJobName = SS_CreateStrHandle((LPSTR)lpJobName);
  else
    lpSpread->Print.hPrintJobName = NULL;
  return TRUE;
}

//--------------------------------------------------------------------

BOOL SS_VbxSave4Print(LPSPREADSHEET lpSS, LPSS_BUFF lpBuff)
{
  LPVBSPREAD lpSpread = (LPVBSPREAD)VBDerefControl(VBGetHwndControl(lpSS->lpBook->hWnd));
  SS_REC_PRINT rec;
  LPSTR lpszPrintAbortMsg = NULL;
  LPSTR lpszPrintFooter = NULL;
  LPSTR lpszPrintHeader = NULL;
  LPSTR lpszPrintJobName = NULL;
  BOOL bRet = TRUE;
  if( lpSpread->Print.hPrintAbortMsg)
    lpszPrintAbortMsg = (LPSTR)GlobalLock(lpSpread->Print.hPrintAbortMsg);
  if( lpSpread->Print.hPrintFooter)
    lpszPrintFooter = (LPSTR)GlobalLock(lpSpread->Print.hPrintFooter);
  if( lpSpread->Print.hPrintHeader)
    lpszPrintHeader = (LPSTR)GlobalLock(lpSpread->Print.hPrintHeader);
  if( lpSpread->Print.hPrintJobName)
    lpszPrintJobName = (LPSTR)GlobalLock(lpSpread->Print.hPrintJobName);
  if( lpSpread->Print.PrintBorder != TRUE ||
      lpSpread->Print.PrintColor != FALSE ||
      lpSpread->Print.PrintShadows != TRUE ||
      lpSpread->Print.PrintGrid != TRUE ||
      lpSpread->Print.PrintColHeaders != TRUE ||
      lpSpread->Print.PrintRowHeaders != TRUE ||
      lpSpread->Print.PrintUseDataMax != TRUE ||
      lpSpread->Print.PrintType != 0 ||
      lpSpread->Print.PrintPageEnd != 1 ||
      lpSpread->Print.PrintPageStart != 1 ||
      lpSpread->Print.PrintMarginLeft != 0 ||
      lpSpread->Print.PrintMarginTop != 0 ||
      lpSpread->Print.PrintMarginBottom != 0 ||
      lpSpread->Print.PrintMarginRight != 0 ||
      lpSpread->Print.PrintOrientation != 0 ||
      (lpszPrintAbortMsg && lstrcmp(lpszPrintAbortMsg, "")) ||
      (lpszPrintFooter && lstrcmp(lpszPrintFooter, "")) ||
      (lpszPrintHeader && lstrcmp(lpszPrintHeader, "")) ||
      (lpszPrintJobName && lstrcmp(lpszPrintJobName, "")) )
  {
    rec.nRecType = SS_RID_PRINT;
    rec.lRecLen = sizeof(rec);
    rec.bDrawBorder = (BYTE)lpSpread->Print.PrintBorder;
    rec.bDrawColor = (BYTE)lpSpread->Print.PrintColor;
    rec.bDrawShadows = (BYTE)lpSpread->Print.PrintShadows;
    rec.bShowGrid = (BYTE)lpSpread->Print.PrintGrid;
    rec.bShowColHeaders = (BYTE)lpSpread->Print.PrintColHeaders;
    rec.bShowRowHeaders = (BYTE)lpSpread->Print.PrintRowHeaders;
    rec.bUseDataMax = (BYTE)lpSpread->Print.PrintUseDataMax;
    rec.nPrintType = lpSpread->Print.PrintType;
    rec.nPageEnd = lpSpread->Print.PrintPageEnd;
    rec.nPageStart = lpSpread->Print.PrintPageStart;
    rec.lMarginLeft = lpSpread->Print.PrintMarginLeft;
    rec.lMarginTop = lpSpread->Print.PrintMarginTop;
    rec.lMarginBottom = lpSpread->Print.PrintMarginBottom;
    rec.lMarginRight = lpSpread->Print.PrintMarginRight;
    rec.nOrientation = lpSpread->Print.PrintOrientation;
    rec.lAbortMsgLen = lpszPrintAbortMsg ? (lstrlen(lpszPrintAbortMsg) + 1) * sizeof(char) : 0;
    rec.lFooterLen = lpszPrintFooter ? (lstrlen(lpszPrintFooter) + 1) * sizeof(char) : 0;
    rec.lHeaderLen = lpszPrintHeader ? (lstrlen(lpszPrintHeader) + 1) * sizeof(char) : 0;
    rec.lJobNameLen = lpszPrintJobName ? (lstrlen(lpszPrintJobName) + 1) * sizeof(char) : 0;
    rec.lRecLen += rec.lAbortMsgLen;
    rec.lRecLen += rec.lFooterLen;
    rec.lRecLen += rec.lHeaderLen;
    rec.lRecLen += rec.lJobNameLen;
    bRet &= SS_BuffAppend(lpBuff, &rec, sizeof(rec));
    if( lpszPrintAbortMsg )
      bRet &= SS_BuffAppend(lpBuff, lpszPrintAbortMsg, rec.lAbortMsgLen);
    if( lpszPrintFooter )
      bRet &= SS_BuffAppend(lpBuff, lpszPrintFooter, rec.lFooterLen);
    if( lpszPrintHeader )
      bRet &= SS_BuffAppend(lpBuff, lpszPrintHeader, rec.lHeaderLen);
    if( lpszPrintJobName )
      bRet &= SS_BuffAppend(lpBuff, lpszPrintJobName, rec.lJobNameLen);
  }
  if( lpSpread->Print.hPrintAbortMsg)
    GlobalUnlock(lpSpread->Print.hPrintAbortMsg);
  if( lpSpread->Print.hPrintFooter)
    GlobalUnlock(lpSpread->Print.hPrintFooter);
  if( lpSpread->Print.hPrintHeader)
    GlobalUnlock(lpSpread->Print.hPrintHeader);
  if( lpSpread->Print.hPrintJobName)
    GlobalUnlock(lpSpread->Print.hPrintJobName);
  return bRet;
}

//--------------------------------------------------------------------

BOOL SS_VbxLoad4Print2(LPSPREADSHEET lpSS, LPSS_REC_PRINT2 lpRec)
{
  LPVBSPREAD lpSpread = (LPVBSPREAD)VBDerefControl(VBGetHwndControl(lpSS->lpBook->hWnd));
  lpSpread->Print.fSmartPrint = lpRec->bSmartPrint;
  lpSpread->Print.nPageOrder = lpRec->nPageOrder;
  lpSpread->Print.lFirstPageNumber = lpRec->lFirstPageNumber;
  return TRUE;
}

//--------------------------------------------------------------------

BOOL SS_VbxSave4Print2(LPSPREADSHEET lpSS, LPSS_BUFF lpBuff)
{
  LPVBSPREAD lpSpread = (LPVBSPREAD)VBDerefControl(VBGetHwndControl(lpSS->lpBook->hWnd));
  SS_REC_PRINT2 rec;
  BOOL bRet = TRUE;

  rec.nRecType = SS_RID_PRINT2;
  rec.lRecLen = sizeof(rec);

  rec.bSmartPrint = (BYTE)lpSpread->Print.fSmartPrint;
  rec.nPageOrder = lpSpread->Print.nPageOrder;
  rec.lFirstPageNumber = lpSpread->Print.lFirstPageNumber;

  bRet &= SS_BuffAppend(lpBuff, &rec, sizeof(rec));
  return bRet;
}

//--------------------------------------------------------------------


void VBSSPrintPreviewGetInfo(HCTL hCtl, HWND hWnd, LPPRVW_INFO lpPrvwInfo)
{
LPVBSPREAD       lpSpread;
SS_PRINTPAGECALC PrintPageCalc;

lpSpread = (LPVBSPREAD)VBDerefControl(hCtl);

lpPrvwInfo->Margins.fMarginLeft = (float)lpSpread->Print.PrintMarginLeft /
                                  (float)1440.0;
lpPrvwInfo->Margins.fMarginTop = (float)lpSpread->Print.PrintMarginTop /
                                 (float)1440.0;
lpPrvwInfo->Margins.fMarginRight = (float)lpSpread->Print.PrintMarginRight /
                                   (float)1440.0;
lpPrvwInfo->Margins.fMarginBottom = (float)lpSpread->Print.PrintMarginBottom /
                                    (float)1440.0;

VBSS_Print(hCtl, hWnd, lpSpread, SS_PRINT_ACTION_PAGECALC, 0, NULL, 0,
           &PrintPageCalc);

if (lpSpread->Print.PrintType == 3)
   lpPrvwInfo->lPageBeg = lpSpread->Print.PrintPageStart;
else
   lpPrvwInfo->lPageBeg = 1;

lpPrvwInfo->lPageEnd = PrintPageCalc.lPageCnt;
lpPrvwInfo->fPageWidth = PrintPageCalc.fPageWidth;
lpPrvwInfo->fPageHeight = PrintPageCalc.fPageHeight;
}


void VBSSPrintPreviewPrintPage(HCTL hCtl, HWND hWnd, LPPRVW_PAGE lpPrvwPage)
{
LPVBSPREAD lpSpread;

lpSpread = (LPVBSPREAD)VBDerefControl(hCtl);

VBSS_Print(hCtl, hWnd, lpSpread, SS_PRINT_ACTION_PREVIEW, lpPrvwPage->hDC,
           &lpPrvwPage->Rect, lpPrvwPage->lPage, NULL);
}


void VBSSPrintPreviewSetMargins(HCTL hCtl, HWND hWnd,
                                LPPRVW_MARGINS lpPrvwMargins)
{
LPVBSPREAD lpSpread;

lpSpread = (LPVBSPREAD)VBDerefControl(hCtl);

lpSpread->Print.PrintMarginLeft = (long)(lpPrvwMargins->fMarginLeft *
                                         (float)1440.0);
lpSpread->Print.PrintMarginTop = (long)(lpPrvwMargins->fMarginTop *
                                        (float)1440.0);
lpSpread->Print.PrintMarginRight = (long)(lpPrvwMargins->fMarginRight *
                                          (float)1440.0);
lpSpread->Print.PrintMarginBottom = (long)(lpPrvwMargins->fMarginBottom *
                                           (float)1440.0);
}

#endif
