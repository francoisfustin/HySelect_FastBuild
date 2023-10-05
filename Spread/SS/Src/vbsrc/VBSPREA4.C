/*********************************************************
* VBSPREA4.C
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
#include <string.h>
#include "fpconvrt.h"
#ifdef SS_V35
#define TT_V2
#endif
#include "texttip.h"
#include "..\sssrc\spread\spread.h"
#include "..\sssrc\spread\ss_alloc.h"
#include "..\sssrc\spread\ss_cell.h"
#include "..\sssrc\spread\ss_save.h"
#include "..\sssrc\spread\ss_user.h"
#include "vbmisc.h"
#include "vbdattim.h"
#include "vbspread.h"
#include "db_bind.h"
#include "..\vbx\stringrc.h"

typedef unsigned int size_t;
void _far * _far _cdecl _fmemcpy(void _far *, const void _far *, size_t);
long DLLENTRY lSSStrLen(LPSTR s);

extern HANDLE hDynamicInst;

long VBSS_Font(LPFPCONTROL lpObject, HWND hWnd, WORD wProp, long lParam,
               BOOL fGetProp);
void FAR PASCAL FPx_GetData(HWND hWnd, HCTL hCtl, SS_COORD Col, SS_COORD Row,
                            LPVBSPREAD lpSpread, LPARAM lParam);
BOOL            vbSpreadFireQueryAdvance(HCTL hCtl, short dNext);
void            FP_ProcessButtonPict(LONG lParam, LPVBSPREAD lpSpread,
                                     LPHANDLE lphPictName, LPSHORT lpPictType);
short           SS_AdjustPropArrayCol(LPVBSPREAD lpSpread,
                                      LPSS_COORD lpCol);
short           SS_AdjustPropArrayRow(LPVBSPREAD lpSpread,
                                      LPSS_COORD lpRow);
LONG            FP_ProcessText(BOOL fSetProp, HCTL hCtl, HWND hWnd, WORD Msg,
                               WORD wParam, LONG lParam, LPVBSPREAD lpSpread,
                               LPTBGLOBALHANDLE lphText);
LONG            FP_Sort(BOOL fSetProp, HCTL hCtl, HWND hWnd, WORD Msg,
                        WORD wParam, LONG lParam, LPVBSPREAD lpSpread,
                        BOOL fOrder);


/****************************************************************************************/

#ifndef SSP_NO_ACTION
LONG FP_Action(fSetProp,hCtl,hWnd,Msg,wParam,lParam,lpSpread)
BOOL            fSetProp;
HCTL            hCtl;
HWND            hWnd;
WORD            Msg;
WORD            wParam;
LONG            lParam;
LPVBSPREAD      lpSpread;
{
SS_CELLCOORD    CellUL;
SS_CELLCOORD    CellLR;
SS_COORD        Row2;
int             Ret = TRUE;

if (!fSetProp)
   {
   return (0);
   }

switch (LOWORD(lParam))
   {
#ifndef SSA_NO_ActivateCell
   case 0:                 // Activate Cell
      {
      LPSPREADSHEET lpSS;
      SS_COORD RowOld;
      
      lpSS = SS_Lock(hWnd);
      RowOld = lpSS->Row.CurAt;
      VBSSVALCOLROW(lpSpread);
      SSSetActiveCell(hWnd, lpSpread->Col, lpSpread->Row);
      #ifdef SS_BOUNDCONTROL
      vbSpreadSaveBoundRow(lpSS, hCtl, hWnd, RowOld,
                           lpSS->Row.CurAt, lpSS->Col.CurAt);
      #endif
      SS_Unlock(hWnd);
      }
      break;
#endif

#ifndef SSA_NO_Goto
   case 1:                 // Goto
      VBSSVALCOLROW(lpSpread);
      Ret = SSShowCell(hWnd, lpSpread->Col, lpSpread->Row, lpSpread->Position);
      break;
#endif

#ifndef SSA_NO_SelBlock
   case 2:                 // SelBlock
      VBSSVALCOLROW(lpSpread);
      VBSSVALCOL2ROW2(lpSpread);
      CellUL.Col = lpSpread->Col;
      CellUL.Row = lpSpread->Row;
      CellLR.Col = lpSpread->Col2;
      CellLR.Row = lpSpread->Row2;
      Ret = SSSetSelectBlock(hWnd, &CellUL, &CellLR);
      break;
#endif

#ifndef SSA_NO_Clear
   case 3:                 // Clear
      if (lpSpread->BlockMode)
         {
         VBSSVALCOLROW(lpSpread);
         VBSSVALCOL2ROW2(lpSpread);
         Ret = SSClearRange(hWnd, lpSpread->Col, lpSpread->Row,lpSpread->Col2, lpSpread->Row2);
         }
      else
         {
         VBSSVALCOLROW(lpSpread);
         Ret = SSClear(hWnd, lpSpread->Col, lpSpread->Row);
         }

      break;
#endif

#ifndef SSA_NO_DeleteCol
   case 4:                 // Delete Col
      if (lpSpread->BlockMode)
         {
         VBSSVALCOL(lpSpread);
         VBSSVALCOL2(lpSpread);
         SSDelColRange(hWnd, lpSpread->Col, lpSpread->Col2);
         }
      else if (lpSpread->Col != -1)
         {
         VBSSVALCOL(lpSpread);
         SSDelCol(hWnd, lpSpread->Col);
         }

      break;
#endif

#ifndef SSA_NO_DeleteRow
   case 5:                 // Delete Row
      if (lpSpread->BlockMode)
         {
         VBSSVALROW(lpSpread);
         VBSSVALROW2(lpSpread);
         Row2 = lpSpread->Row2;
         }
      else if (lpSpread->Row != -1)
         {
         VBSSVALROW(lpSpread);
         Row2 = lpSpread->Row;
         }

      {
      BOOL fDel = TRUE;
      LPSPREADSHEET lpSS = SS_Lock(hWnd);

#ifdef SS_BOUNDCONTROL
      if (VBSSBoundIsBound((LONG)hCtl, lpSS))
         if (VBSSBoundDel(lpSS, (LONG)hCtl, hWnd, lpSpread->Row, Row2))
            fDel = FALSE;
#endif

#ifdef SS_QE
      if (lpSpread->DBInfo.fConnected)
         if (DBSS_BoundDel(hWnd, lpSpread, lpSpread->Row, Row2))
            fDel = FALSE;
#endif

      if (fDel)
         SSDelRowRange(hWnd, lpSpread->Row, Row2);
      SS_Unlock(hWnd);
      }

      break;
#endif

#ifndef SSA_NO_InsertCol
   case 6:                 // Insert Col
      if (lpSpread->BlockMode)
         {
         VBSSVALCOL(lpSpread);
         VBSSVALCOL2(lpSpread);
         SSInsColRange(hWnd, lpSpread->Col, lpSpread->Col2);
         }
      else if (lpSpread->Col != -1)
         {
         VBSSVALCOL(lpSpread);
         SSInsCol(hWnd, lpSpread->Col);
         }

      break;
#endif

#ifndef SSA_NO_InsertRow
   case 7:                 // Insert Row
      if (lpSpread->BlockMode)
         {
         VBSSVALROW(lpSpread);
         VBSSVALROW2(lpSpread);
         Row2 = lpSpread->Row2;
         SSInsRowRange(hWnd, lpSpread->Row, Row2);
         #ifdef SS_QE
         if (lpSpread->DBInfo.fConnected)
            DBSS_BoundIns(hWnd, lpSpread, lpSpread->Row, Row2);
         #endif
         }
      else if (lpSpread->Row != -1)
         {
         VBSSVALROW(lpSpread);
         Row2 = lpSpread->Row;
         SSInsRowRange(hWnd, lpSpread->Row, Row2);
         #ifdef SS_QE
         if (lpSpread->DBInfo.fConnected)
            DBSS_BoundIns(hWnd, lpSpread, lpSpread->Row, Row2);
         #endif
         }


      break;
#endif

#ifndef SSA_NO_Load
   case 8:                 // Load
      Ret = LoadFromFile(hWnd, lpSpread->FileNum);
      lpSpread->lMaxCols = SSGetMaxCols(hWnd);
      lpSpread->lMaxRows = SSGetMaxRows(hWnd);
      break;
#endif

#ifndef SSA_NO_SaveAll
   case 9:                 // SaveAll
      Ret = SaveToFile(hWnd, lpSpread->FileNum, FALSE);
      break;
#endif

#ifndef SSA_NO_SaveValues
   case 10:                // SaveValues
      Ret = SaveToFile(hWnd, lpSpread->FileNum, TRUE);
      break;
#endif

#ifndef SSA_NO_Recalc
   case 11:                // Recalc
      Ret = SSReCalc(hWnd);
      break;
#endif

#ifndef SSA_NO_ClearText
   case 12:                // Clear Text
      if (lpSpread->BlockMode)
         {
         VBSSVALCOLROW(lpSpread);
         VBSSVALCOL2ROW2(lpSpread);
         Ret = SSClearDataRange(hWnd, lpSpread->Col, lpSpread->Row,lpSpread->Col2, lpSpread->Row2);
         }
      else
         {
         VBSSVALCOLROW(lpSpread);
         Ret = SSClearData(hWnd, lpSpread->Col, lpSpread->Row);
         }

      break;
#endif

#ifndef SSA_NO_Print
   case 13:                // Print
      {
      long lRet;

      if (!(lRet = vbSpreadPrintAction(hCtl, hWnd, lpSpread)))
          return (lRet);
      }

      break;
#endif

#ifndef SSA_NO_DeselectBlock
   case 14:                // Deselect Block
       SSDeSelectBlock(hWnd);
       return (0);
#endif

#ifndef SSA_NO_DSave
   case 15:                // 15 - DSave
		{
      LPSPREADSHEET lpSS = SS_Lock(hWnd);
      VBSSBoundManualSave(lpSS, (LONG)hCtl, hWnd, lpSpread, 1, -1, TRUE);
      SS_Unlock(hWnd);
		}
      break;
#endif

#ifndef SSA_NO_SetBorder
   case 16:                // 16 - SetBorder
      {
      if (lpSpread->BlockMode)
         {
         VBSSVALCOLROW(lpSpread);
         VBSSVALCOL2ROW2(lpSpread);
         Ret = SSSetBorderRange(hWnd, lpSpread->Col, lpSpread->Row,
                                lpSpread->Col2, lpSpread->Row2,
                                lpSpread->wBorderType,
                                lpSpread->wBorderStyle,
                                lpSpread->BorderColor);
         }
      else
         {
         VBSSVALCOLROW(lpSpread);
         Ret = SSSetBorder(hWnd, lpSpread->Col, lpSpread->Row,
                           lpSpread->wBorderType,
                           lpSpread->wBorderStyle,
                           lpSpread->BorderColor);
         }
      }

      break;
#endif

#ifndef SSA_NO_AddMultiSelBlock
   case 17:                // 17 - AddMultiSelBlock
      {
      SS_SELBLOCK SelBlock;

      SelBlock.UL.Col = lpSpread->Col;
      SelBlock.UL.Row = lpSpread->Row;

      if (lpSpread->BlockMode)
         {
         SelBlock.LR.Col = lpSpread->Col2;
         SelBlock.LR.Row = lpSpread->Row2;
         }
      else
         {
         SelBlock.LR.Col = lpSpread->Col;
         SelBlock.LR.Row = lpSpread->Row;
         }

      SSAddMultiSelBlocks(hWnd, &SelBlock);
      }
      break;
#endif

#ifndef SSA_NO_GetMultiSelBlocks
   case 18:                // 18 - GetMultiSelBlocks
      if (lpSpread->hMultiSelBlocks)
         GlobalFree(lpSpread->hMultiSelBlocks);

      lpSpread->dMultiSelBlockCnt = 0;

      lpSpread->hMultiSelBlocks = SSGetMultiSelBlocks(hWnd,
                                  &lpSpread->dMultiSelBlockCnt);

      break;
#endif

#ifndef SSA_NO_CopyRange
   case 19:                // 19 - CopyRange
      VBSSVALCOL(lpSpread);
      VBSSVALCOL2(lpSpread);
      SSCopyRange(hWnd, lpSpread->Col, lpSpread->Row, lpSpread->Col2,
                  lpSpread->Row2, lpSpread->ColDest, lpSpread->RowDest);
      break;
#endif

#ifndef SSA_NO_MoveRange
   case 20:                // 20 - MoveRange
      VBSSVALCOL(lpSpread);
      VBSSVALCOL2(lpSpread);
      SSMoveRange(hWnd, lpSpread->Col, lpSpread->Row, lpSpread->Col2,
                  lpSpread->Row2, lpSpread->ColDest, lpSpread->RowDest);
      break;
#endif

#ifndef SSA_NO_SwapRange
   case 21:                // 21 - SwapRange
      VBSSVALCOL(lpSpread);
      VBSSVALCOL2(lpSpread);
      SSSwapRange(hWnd, lpSpread->Col, lpSpread->Row, lpSpread->Col2,
                  lpSpread->Row2, lpSpread->ColDest, lpSpread->RowDest);
      break;
#endif

#ifndef SSA_NO_ClipboardCopy
   case 22:                // 22 - ClipboardCopy
      SSClipboardCopy(hWnd);
      break;
#endif

#ifndef SSA_NO_ClipboardCut
   case 23:                // 23 - ClipboardCut
      SSClipboardCut(hWnd);
      break;
#endif

#ifndef SSA_NO_ClipboardPaste
   case 24:                // 24 - ClipboardPaste
      SSClipboardPaste(hWnd);
      break;
#endif

#ifndef SSA_NO_Sort
   case 25:                // 25 - Sort
      if (lpSpread->nSortKeyCnt && lpSpread->hSortKeys)
         {
         LPSS_SORTKEY lpSortKeys = (LPSS_SORTKEY)GlobalLock(lpSpread->hSortKeys);
         HCURSOR      hCursor;
         SS_COORD     lActiveRow;
	      LPSPREADSHEET lpSS = SS_Lock(hWnd);

         hCursor = SetCursor(LoadCursor(NULL, IDC_WAIT));
         ShowCursor(TRUE);

         SSSortEx(hWnd, lpSpread->Col, lpSpread->Row, lpSpread->Col2,
                  lpSpread->Row2, lpSpread->dSortBy, lpSortKeys,
                  lpSpread->nSortKeyCnt);
         SSGetActiveCell(hWnd, NULL, &lActiveRow);
         if (VBSSBoundIsBound((LONG)hCtl, lpSS))
            vbSpreadBoundClick(lpSS, (LONG)hCtl, lActiveRow);

         GlobalUnlock(lpSpread->hSortKeys);
         ShowCursor(FALSE);
         SetCursor(hCursor);
			SS_Unlock(hWnd);
         }

      break;
#endif

#ifndef SSA_NO_ComboClear
   case 26:                // 26 - ComboClear
      SSComboBoxSendMessage(hWnd, lpSpread->Col, lpSpread->Row,
                            SS_CBM_RESETCONTENT, 0, 0L);
      break;
#endif

#ifndef SSA_NO_ComboRemoveString
   case 27:                // 27 - ComboRemoveString
      SSComboBoxSendMessage(hWnd, lpSpread->Col, lpSpread->Row,
                            SS_CBM_DELETESTRING,
                            lpSpread->dTypeComboIndex, 0L);
      break;
#endif

#ifndef SSA_NO_Reset
   case 28:                // 28 - Reset
      SSReset(hWnd);
      vbSSInit(hCtl, hWnd);
      break;
#endif

#ifndef SSA_NO_SelModeClear
   case 29:                // 29 - SelModeClear
      if (SSSelModeSendMessage(hWnd, SS_SELMODE_CLEARSEL, 0L, 0L, 0L) !=
          TRUE)
         Ret = FALSE;

      break;
#endif

#ifndef SSA_NO_VRefreshBuffer
   case 30:                // 30 - VRefreshBuffer
      SSVRefreshBuffer(hWnd);
      break;
#endif

#ifndef SSA_NO_DataRefresh
#ifdef SS_QE
   case 31:                // 31 - DataRefresh
      SSDataRefresh(hCtl, hWnd);
      break;
#endif
#endif

#ifndef SSA_NO_SmartPrint
   case 32:                // SmartPrint
      {
      long          lRet;
      BOOL          fSmartPrint = lpSpread->Print.fSmartPrint;

      lpSpread->Print.fSmartPrint = TRUE;
      lRet = vbSpreadPrintAction(hCtl, hWnd, lpSpread);
      lpSpread->Print.fSmartPrint = fSmartPrint;

      if (!lRet)
          return (lRet);
      }

      break;
#endif

   } //end select

if (!Ret)
   return (ERR_INVALID_PROPVAL);

return (0);
}
#endif


/****************************************************************************************/
LONG FP_Font(fSetProp,hCtl,hWnd,Msg,wParam,lParam,lpSpread)
BOOL            fSetProp;
HCTL            hCtl;
HWND            hWnd;
WORD            Msg;
WORD            wParam;
LONG            lParam;
LPVBSPREAD      lpSpread;
{
long            lRet;

    if (!fSetProp)
        {
        LIMIT_TO_COLROW;

//        return (VBDefControlProc(hCtl, hWnd, Msg, wParam, lParam));
        return (VBSS_Font(hCtl, hWnd, wParam, lParam, TRUE));
        }

    LIMIT_TO_COLROW;

    lpSpread->hFontRgn = CreateRectRgn(0, 0, 0, 0);
    lpSpread->fFontRgnSet = FALSE;
//    lRet = VBDefControlProc(hCtl, hWnd, Msg, wParam, lParam);
    lRet = VBSS_Font(hCtl, hWnd, wParam, lParam, FALSE);

    if (lpSpread->fFontRgnSet)
        {
        ValidateRect(hWnd, NULL);
        InvalidateRgn(hWnd, lpSpread->hFontRgn, TRUE);
        lpSpread->fFontRgnSet = FALSE;
        }
    else if (lpSpread->Col == -1 || lpSpread->Row == -1)
        SendMessage(hWnd, WM_SETFONT, SSGetFont(hWnd, lpSpread->Col,lpSpread->Row), 0L);

    DeleteObject(lpSpread->hFontRgn);
    lpSpread->hFontRgn = 0;
    return (lRet);
 }


long VBSS_Font(LPFPCONTROL lpObject, HWND hWnd, WORD wProp, long lParam,
               BOOL fGetProp)
{
LOGFONT LogFont;
HFONT   hFont;
HFONT   hFontOld = 0;
HFONT   hFontTemp;
HDC     hDC;
long    lRet = 0;

hFontOld = (HFONT)SendMessage(hWnd, WM_GETFONT, 0, 0);

if (hFontOld)
   GetObject(hFontOld, sizeof(LOGFONT), &LogFont);
else
   {
   hDC = GetDC(hWnd);
   hFontOld = SelectObject(hDC, GetStockObject(SYSTEM_FONT));
   GetObject(hFontOld, sizeof(LOGFONT), &LogFont);
   SelectObject(hDC, hFontOld);
   ReleaseDC(hWnd, hDC);
   }

if (fGetProp)
   {
   if (wProp == IPROP_SPREAD_FONTBOLD)
      *(short FAR *)lParam = (LogFont.lfWeight == 700) ? -1 : 0;

   else if (wProp == IPROP_SPREAD_FONTITALIC)
      *(short FAR *)lParam = LogFont.lfItalic ? -1 : 0;

   else if (wProp == IPROP_SPREAD_FONTNAME)
      *(HSZ FAR *)lParam = VBCreateHsz(HIWORD((HCTL)lpObject),
                                       LogFont.lfFaceName);

   else if (wProp == IPROP_SPREAD_FONTSIZE)
      {
      TEXTMETRIC TextMetric;

      hDC = GetDC(hWnd);
      hFontTemp = SelectObject(hDC, hFontOld);
      GetTextMetrics(hDC, &TextMetric);
      *(float FAR *)lParam = PIXELS_TO_PT(TextMetric.tmHeight -
                                          TextMetric.tmInternalLeading);
      SelectObject(hDC, hFontTemp);
      ReleaseDC(hWnd, hDC);
      }

   else if (wProp == IPROP_SPREAD_FONTSTRIKE)
      *(short FAR *)lParam = LogFont.lfStrikeOut ? -1 : 0;

   else if (wProp == IPROP_SPREAD_FONTUNDER)
      *(short FAR *)lParam = LogFont.lfUnderline ? -1 : 0;
   }
else
   {
   if (wProp == IPROP_SPREAD_FONTBOLD)
      LogFont.lfWeight = ((BOOL)lParam ? 700 : 400);

   else if (wProp == IPROP_SPREAD_FONTITALIC)
      LogFont.lfItalic = (BOOL)((BOOL)lParam ? TRUE : FALSE);

   else if (wProp == IPROP_SPREAD_FONTNAME && lParam)
      {
      TEXTMETRIC TextMetric;

      hDC = GetDC(hWnd);
      hFontTemp = SelectObject(hDC, hFontOld);
      GetTextMetrics(hDC, &TextMetric);
      LogFont.lfHeight = -(TextMetric.tmHeight - TextMetric.tmInternalLeading);
      LogFont.lfWidth = 0;
      LogFont.lfQuality = DEFAULT_QUALITY;
//      LogFont.lfOutPrecision = OUT_TT_ONLY_PRECIS;
      LogFont.lfOutPrecision = OUT_DEFAULT_PRECIS;
      LogFont.lfPitchAndFamily = 0;
      //LogFont.lfCharSet = 0;
      LogFont.lfCharSet = DEFAULT_CHARSET;
      SelectObject(hDC, hFontTemp);
      ReleaseDC(hWnd, hDC);

      _fmemset(LogFont.lfFaceName, '\0', sizeof(LF_FACESIZE));
      _fstrncpy(LogFont.lfFaceName, (LPSTR)lParam, LF_FACESIZE);
      LogFont.lfFaceName[LF_FACESIZE - 1] = '\0';
      }

   else if (wProp == IPROP_SPREAD_FONTSIZE)
      {
      if (lParam)
         {
         hDC = GetDC(hWnd);
         LogFont.lfHeight = -PT_TO_PIXELS(*((float FAR *)&lParam));
         LogFont.lfWidth = 0;
         ReleaseDC(hWnd, hDC);
         }
      }

   else if (wProp == IPROP_SPREAD_FONTSTRIKE)
      LogFont.lfStrikeOut = (BOOL)((BOOL)lParam ? TRUE : FALSE);

   else if (wProp == IPROP_SPREAD_FONTUNDER)
      LogFont.lfUnderline = (BOOL)((BOOL)lParam ? TRUE : FALSE);

   hFont = CreateFontIndirect(&LogFont);
   SendMessage(hWnd, WM_SETFONT, (WPARAM)hFont, 0);
   DeleteObject(hFont);
   }

return (lRet);
}


/****************************************************************************************/
#ifndef SSP_NO_ACTIVECELLCOL
LONG FP_ActiveCol(fSetProp,hCtl,hWnd,Msg,wParam,lParam,lpSpread)
BOOL            fSetProp;
HCTL            hCtl;
HWND            hWnd;
WORD            Msg;
WORD            wParam;
LONG            lParam;
LPVBSPREAD      lpSpread;
{
SS_COORD        Row;
SS_COORD        Col;

    SSGetActiveCell(hWnd, &Col, &Row);
    if (Col == SS_HEADER)
        Col = 0;

    *(long FAR *)lParam = Col;
    return (0);
}
#endif


/****************************************************************************************/
#ifndef SSP_NO_ACTIVECELLROW
LONG FP_ActiveRow(fSetProp,hCtl,hWnd,Msg,wParam,lParam,lpSpread)
BOOL            fSetProp;
HCTL            hCtl;
HWND            hWnd;
WORD            Msg;
WORD            wParam;
LONG            lParam;
LPVBSPREAD      lpSpread;
{
SS_COORD        Row;
SS_COORD        Col;

    SSGetActiveCell(hWnd, &Col, &Row);
    if (Row == SS_HEADER)
       Row = 0;

    *(long FAR *)lParam = Row;
    return (0);
}
#endif


/****************************************************************************************/
#ifndef SSP_NO_BACKCOLOR
LONG FP_BackColor(fSetProp,hCtl,hWnd,Msg,wParam,lParam,lpSpread)
BOOL            fSetProp;
HCTL            hCtl;
HWND            hWnd;
WORD            Msg;
WORD            wParam;
LONG            lParam;
LPVBSPREAD      lpSpread;
{
int             Ret = TRUE;

    if (!fSetProp)
        {
        VBSSVALCOLROW(lpSpread);

        LIMIT_TO_COLROW;

        SSGetColor(hWnd, lpSpread->Col, lpSpread->Row, (LPCOLORREF)lParam,
                   NULL);
        return (0);
        }

    LIMIT_TO_COLROW;

    if (lpSpread->BlockMode)
        {
        VBSSVALCOLROW(lpSpread);
        VBSSVALCOL2ROW2(lpSpread);
        Ret = SSSetColorRange(hWnd, lpSpread->Col, lpSpread->Row,lpSpread->Col2, lpSpread->Row2,lParam, (COLORREF)SPREAD_COLOR_IGNORE);
        }
    else
        {
        VBSSVALCOLROW(lpSpread);
        Ret = SSSetColor(hWnd, lpSpread->Col, lpSpread->Row, lParam,(COLORREF)SPREAD_COLOR_IGNORE);
        }
    if (!Ret)
        return (ERR_INVALID_PROPVAL);
    return (0);
 }
#endif


/*****************************************************************************/
#ifndef SS_V30
#ifndef SSP_NO_CALCDEPENDENCIES
LONG FP_Calc_Dept(fSetProp,hCtl,hWnd,Msg,wParam,lParam,lpSpread)
BOOL            fSetProp;
HCTL            hCtl;
HWND            hWnd;
WORD            Msg;
WORD            wParam;
LONG            lParam;
LPVBSPREAD      lpSpread;
{
LPSPREADSHEET   lpSS;
int             Ret = TRUE;

    if (!fSetProp)
        {
        *(BOOL FAR *)lParam = lpSpread->CalcDependencies;
        return (0);
        }

    lpSS = SS_Lock(hWnd);

    lpSpread->CalcDependencies = (BOOL)lParam;
    if (lpSpread->CalcDependencies)
        Ret = SSBuildDependencies(hWnd);

    SS_Unlock(hWnd);
    if (!Ret)
        return (ERR_INVALID_PROPVAL);

    return (0);
}
#endif
#endif


/****************************************************************************************/
#ifndef SSP_NO_CELLTYPE
LONG FP_CellType1(fSetProp,hCtl,hWnd,Msg,wParam,lParam,lpSpread)
BOOL            fSetProp;
HCTL            hCtl;
HWND            hWnd;
WORD            Msg;
WORD            wParam;
LONG            lParam;
LPVBSPREAD      lpSpread;
{
int             Ret = TRUE;
SS_CELLTYPE     CellType;

    if (!fSetProp)
        {
        VBSSVALCOLROW(lpSpread);
        if ((Ret = SSGetCellType(hWnd, lpSpread->Col, lpSpread->Row,
                                   &CellType)) != 0)
           switch (CellType.Type)
               {
               case SS_TYPE_DATE:
                   *(short FAR *)lParam = CELLTYPE_DATE;
                   break;

               case SS_TYPE_EDIT:
                   *(short FAR *)lParam = CELLTYPE_EDIT;
                   break;

               case SS_TYPE_FLOAT:
                   *(short FAR *)lParam = CELLTYPE_FLOAT;
                   break;

               case SS_TYPE_INTEGER:
                   *(short FAR *)lParam = CELLTYPE_INTEGER;
                   break;

               case SS_TYPE_PIC:
                   *(short FAR *)lParam = CELLTYPE_PIC;
                   break;

               case SS_TYPE_STATICTEXT:
                   *(short FAR *)lParam = CELLTYPE_STATICTEXT;
                   break;

               case SS_TYPE_TIME:
                   *(short FAR *)lParam = CELLTYPE_TIME;
                   break;

               case SS_TYPE_BUTTON:
                   *(short FAR *)lParam = CELLTYPE_BUTTON;
                   break;

               case SS_TYPE_COMBOBOX:
                   *(short FAR *)lParam = CELLTYPE_COMBOBOX;
                   break;

               case SS_TYPE_PICTURE:
                   *(short FAR *)lParam = CELLTYPE_PICTURE;
                   break;

               case SS_TYPE_CHECKBOX:
                   *(short FAR *)lParam = CELLTYPE_CHECKBOX;
                   break;

               case SS_TYPE_OWNERDRAW:
                   *(short FAR *)lParam = CELLTYPE_OWNERDRAW;
                   break;
               }

        if (!Ret)
            return (ERR_INVALID_PROPVAL);

        return (0);
        }

#ifdef SS_BC
    if ((short)lParam != 1 &&          // Edit
        (short)lParam != 2 &&          // Float
        (short)lParam != 5 &&          // StaticText
        (short)lParam != 9)            // Picture
       Ret = FALSE;
    else
#endif
       Ret = SetCellType(hWnd, lpSpread);

    if (!Ret)
       return (ERR_INVALID_PROPVAL);
    return (0);
}
#endif


/****************************************************************************************/
#ifndef SSP_NO_CLIP
LONG FP_Clip(fSetProp,hCtl,hWnd,Msg,wParam,lParam,lpSpread)
BOOL            fSetProp;
HCTL            hCtl;
HWND            hWnd;
WORD            Msg;
WORD            wParam;
LONG            lParam;
LPVBSPREAD      lpSpread;
{
int             Ret = TRUE;
LPSTR           lpData;
HANDLE          hData;
DWORD           MaxClipSize = 60000;

    if (!fSetProp)
        {
        VBSSVALCOLROW(lpSpread);
        VBSSVALCOL2ROW2(lpSpread);
        hData = SSClipOut(hWnd, lpSpread->Col, lpSpread->Row,
                          lpSpread->Col2, lpSpread->Row2);
        if (hData && (GlobalSize(hData) > MaxClipSize))
            {
            lpData = (LPSTR)GlobalLock(hData);
            lpData[MaxClipSize-1] = '\0';
            GlobalUnlock(hData);
            hData = GlobalReAlloc(hData, MaxClipSize, GMEM_MOVEABLE);
            }
        if (hData)
            {
            lpData = GlobalLock(hData);
            *(HSZ FAR *)lParam = VBCreateHsz(HIWORD(hCtl), lpData);
            GlobalUnlock(hData);
            GlobalFree(hData);
            }
        else
            {
            *(HSZ FAR *)lParam = VBCreateHsz(HIWORD(hCtl), "");
            Ret = FALSE;
            }

            /*
            if (!Ret)
               return (ERR_INVALID_PROPVAL);
            */

        return (0);
        }

    if (lParam)
        {
        VBSSVALCOLROW(lpSpread);
        VBSSVALCOL2ROW2(lpSpread);
        Ret = SSClipIn(hWnd, lpSpread->Col, lpSpread->Row,lpSpread->Col2,
                       lpSpread->Row2, (LPSTR)lParam,
                       lSSStrLen((LPSTR)lParam));
        }
    else
        Ret = 0;

    if (!Ret)
        return (ERR_INVALID_PROPVAL);

    return (0);
}
#endif


/****************************************************************************************/
#ifndef SSP_NO_COL
LONG FP_Col(fSetProp,hCtl,hWnd,Msg,wParam,lParam,lpSpread)
BOOL            fSetProp;
HCTL            hCtl;
HWND            hWnd;
WORD            Msg;
WORD            wParam;
LONG            lParam;
LPVBSPREAD      lpSpread;
{
SS_COORD        Col;

    if (!fSetProp)
        {
        VBSSVALCOL(lpSpread);
        Col = lpSpread->Col;
        if (Col == SS_HEADER)
           Col = 0;

        *(SS_COORD FAR *)lParam = Col;
        return (0);
        }

    if ((SS_COORD)lParam >= -1)
       {
       lpSpread->Col = (SS_COORD)lParam;
       if (lpSpread->Col == 0)
           lpSpread->Col = SS_HEADER;
       else
           lpSpread->Col = min(lpSpread->Col, lpSpread->lMaxCols);
      }
   else
      return (vbSpreadSetError(ERR_RANGE_COL));

    return (0);
}
#endif


/****************************************************************************************/
#ifndef SSP_NO_COL2
LONG FP_Col2(fSetProp,hCtl,hWnd,Msg,wParam,lParam,lpSpread)
BOOL            fSetProp;
HCTL            hCtl;
HWND            hWnd;
WORD            wParam;
LONG            lParam;
LPVBSPREAD      lpSpread;
{
SS_COORD        Col;

    if (!fSetProp)
        {
        VBSSVALCOL2(lpSpread);
        Col = lpSpread->Col2;
        if (Col == SS_HEADER)
           Col = 0;

        *(long FAR *)lParam = Col;
        return (0);
        }

    if ((SS_COORD)lParam >= -1)
       {
       lpSpread->Col2 = (SS_COORD)lParam;
       if (lpSpread->Col2 == 0)
           lpSpread->Col2 = SS_HEADER;
       else
           lpSpread->Col2 = min(lpSpread->Col2, lpSpread->lMaxCols);
      }
   else
      return (vbSpreadSetError(ERR_RANGE_COL));

    return (0);
}
#endif


/****************************************************************************************/
#ifndef SSP_NO_COLWIDTH
LONG FP_ColWidth(fSetProp,hCtl,hWnd,Msg,wParam,lParam,lpSpread)
BOOL            fSetProp;
HCTL            hCtl;
HWND            hWnd;
WORD            Msg;
WORD            wParam;
LONG            lParam;
LPVBSPREAD      lpSpread;
{
int             Ret = TRUE;
LPDATASTRUCT    lpDs;
short           nRet;
float           fNumber;
SS_COORD        Col;
double          dfVal;

    lpDs = (LPDATASTRUCT)lParam;
    Col = (short)lpDs->index[0].data;

    if (nRet = SS_AdjustPropArrayCol(lpSpread, &Col))
       return (nRet);

    if (!fSetProp)
        {
        SSGetColWidth(hWnd, Col, &dfVal);
        fNumber = (float)dfVal;
        _fmemcpy(&lpDs->data, &fNumber, sizeof(float));
        return (0);
        }

    _fmemcpy(&fNumber, &lpDs->data, sizeof(float));
    Ret = SSSetColWidth(hWnd, Col, fNumber);

    if (!Ret)
        return (ERR_INVALID_PROPVAL);
    return (0);
}
#endif


/****************************************************************************************/
#ifndef SSP_NO_EDITMODE
LONG FP_EditMode(fSetProp,hCtl,hWnd,Msg,wParam,lParam,lpSpread)
BOOL            fSetProp;
HCTL            hCtl;
HWND            hWnd;
WORD            Msg;
WORD            wParam;
LONG            lParam;
LPVBSPREAD      lpSpread;
{
    if (!fSetProp)
        {
        *(BOOL FAR *)lParam = SSGetEditMode(hWnd);
        return (0);
        }

    SSSetEditMode(hWnd, (BOOL)lParam);
    return (0);
}
#endif


/****************************************************************************************/
#ifndef SSP_NO_FORECOLOR
LONG FP_ForeColor(fSetProp,hCtl,hWnd,Msg,wParam,lParam,lpSpread)
BOOL            fSetProp;
HCTL            hCtl;
HWND            hWnd;
WORD            wParam;
LONG            lParam;
LPVBSPREAD      lpSpread;
{
int             Ret = TRUE;

    if (!fSetProp)
        {
        VBSSVALCOLROW(lpSpread);
         LIMIT_TO_COLROW;
        SSGetColor(hWnd, lpSpread->Col, lpSpread->Row, NULL,
                   (LPCOLORREF)lParam);

        return (0);
        }

    LIMIT_TO_COLROW;

    if (lpSpread->BlockMode)
       {
       VBSSVALCOLROW(lpSpread);
       VBSSVALCOL2ROW2(lpSpread);
       Ret = SSSetColorRange(hWnd, lpSpread->Col, lpSpread->Row,
                             lpSpread->Col2, lpSpread->Row2,
                             (COLORREF)SPREAD_COLOR_IGNORE,
                             (COLORREF)lParam);
       }
    else
       {
       VBSSVALCOLROW(lpSpread);
       Ret = SSSetColor(hWnd, lpSpread->Col, lpSpread->Row,
                        (COLORREF)SPREAD_COLOR_IGNORE, (COLORREF)lParam);
       }
    if (!Ret)
       return (ERR_INVALID_PROPVAL);
    return (0);
}
#endif


/****************************************************************************************/
#ifndef SSP_NO_FORMULA
LONG FP_Formula(fSetProp,hCtl,hWnd,Msg,wParam,lParam,lpSpread)
BOOL            fSetProp;
HCTL            hCtl;
HWND            hWnd;
WORD            Msg;
WORD            wParam;
LONG            lParam;
LPVBSPREAD      lpSpread;
{
int             Ret = TRUE;
LPSTR           Data;
HANDLE          hData;

    if (!fSetProp)
        {
        VBSSVALCOLROW(lpSpread);

        if (hData = GlobalAlloc(GMEM_MOVEABLE | GMEM_ZEROINIT,
                                (long)SSGetFormulaLen(hWnd,
                                lpSpread->Col, lpSpread->Row) + 1L))
           {
           Data = GlobalLock(hData);
           SSGetFormula(hWnd, lpSpread->Col, lpSpread->Row, Data);

           *(HSZ FAR *)lParam = VBCreateHsz(HIWORD(hCtl), Data);
           GlobalUnlock(hData);
           GlobalFree(hData);
           }

        return (0);
        }

    if (lpSpread->BlockMode)
       {
       VBSSVALCOLROW(lpSpread);
       VBSSVALCOL2ROW2(lpSpread);
       Ret = SSSetFormulaRange(hWnd, lpSpread->Col, lpSpread->Row,
                               lpSpread->Col2, lpSpread->Row2,
                               (LPSTR)lParam,
                               lpSpread->CalcDependencies);
       }
    else
       {
       VBSSVALCOLROW(lpSpread);
       Ret = SSSetFormula(hWnd, lpSpread->Col, lpSpread->Row,
                          (LPSTR)lParam, lpSpread->CalcDependencies);
       }

    if (!Ret)
       return (ERR_INVALID_PROPVAL);

    return (0);
}
#endif


/****************************************************************************************/
#ifndef SSP_NO_LEFTCOL
LONG FP_LeftCol(fSetProp,hCtl,hWnd,Msg,wParam,lParam,lpSpread)
BOOL            fSetProp;
HCTL            hCtl;
HWND            hWnd;
WORD            Msg;
WORD            wParam;
LONG            lParam;
LPVBSPREAD      lpSpread;
{
SS_COORD        Col;
SS_COORD        Row;

    if (!fSetProp)
        {
#ifdef SS_USE16BITCOORDS
        long lCell;

        lCell = SSGetTopLeftCell(hWnd);
        Col = LOWORD(lParam);
        Row = HIWORD(lParam);
#else
        SSGetTopLeftCell(hWnd, &Col, &Row);
#endif
        *(SS_COORD FAR *)lParam = Col;
        return (0);
        }

    SSShowCell(hWnd, (SS_COORD)lParam, -1, SS_SHOW_TOPLEFT);
    return (0);
}
#endif


/****************************************************************************************/
#ifndef SSP_NO_LOCK
LONG FP_Lock(fSetProp,hCtl,hWnd,Msg,wParam,lParam,lpSpread)
BOOL            fSetProp;
HCTL            hCtl;
HWND            hWnd;
WORD            Msg;
WORD            wParam;
LONG            lParam;
LPVBSPREAD      lpSpread;
{
int             Ret = TRUE;

    if (!fSetProp)
        {
        VBSSVALCOLROW(lpSpread);
        *(BOOL FAR *)lParam = SSGetLock(hWnd, lpSpread->Col,
                                        lpSpread->Row);

        return (0);
        }

    if (lpSpread->BlockMode)
       {
       VBSSVALCOLROW(lpSpread);
       VBSSVALCOL2ROW2(lpSpread);
       Ret = SSSetLockRange(hWnd, lpSpread->Col, lpSpread->Row,
                            lpSpread->Col2, lpSpread->Row2,
                            (BOOL)lParam);
       }
    else
       {
       VBSSVALCOLROW(lpSpread);
       Ret = SSSetLock(hWnd, lpSpread->Col, lpSpread->Row,
                       (BOOL)lParam);
       }
    if (!Ret)
       return (ERR_INVALID_PROPVAL);
    return (0);
}
#endif


/****************************************************************************************/
#ifndef SSP_NO_MAXCOLS
LONG FP_MaxCols(fSetProp,hCtl,hWnd,Msg,wParam,lParam,lpSpread)
BOOL            fSetProp;
HCTL            hCtl;
HWND            hWnd;
WORD            Msg;
WORD            wParam;
LONG            lParam;
LPVBSPREAD      lpSpread;
{
    if (!fSetProp)
        {
        *(long FAR *)lParam = SSGetMaxCols(hWnd);
        return (0);
        }

    SSSetMaxCols(hWnd, (SS_COORD)lParam);
    lpSpread->lMaxCols = SSGetMaxCols(hWnd);
    return (0);
}
#endif


/****************************************************************************************/
#ifndef SSP_NO_MAXROWS
LONG FP_MaxRows(fSetProp,hCtl,hWnd,Msg,wParam,lParam,lpSpread)
BOOL            fSetProp;
HCTL            hCtl;
HWND            hWnd;
WORD            Msg;
WORD            wParam;
LONG            lParam;
LPVBSPREAD      lpSpread;
{
    if (!fSetProp)
        {
        *(long FAR *)lParam = SSGetMaxRows(hWnd);
        return (0);
        }

    SSSetMaxRows(hWnd, (SS_COORD)lParam);
    lpSpread->lMaxRows = SSGetMaxRows(hWnd);
    return (0);
}
#endif


/****************************************************************************************/
#ifndef SSP_NO_ROW
LONG FP_Row(fSetProp,hCtl,hWnd,Msg,wParam,lParam,lpSpread)
BOOL            fSetProp;
HCTL            hCtl;
HWND            hWnd;
WORD            Msg;
WORD            wParam;
LONG            lParam;
LPVBSPREAD      lpSpread;
{
SS_COORD        Row;

    if (!fSetProp)
        {
        Row = lpSpread->Row;
        if (Row == SS_HEADER)
           Row = 0;

        *(long FAR *)lParam = Row;
        return (0);
        }

    if ((SS_COORD)lParam >= -1)
       {
       lpSpread->Row = (SS_COORD)lParam;
       if (lpSpread->Row == 0)
          lpSpread->Row = SS_HEADER;
       else
          lpSpread->Row = min(lpSpread->Row, lpSpread->lMaxRows);
       }
    else
       return (vbSpreadSetError(ERR_RANGE_ROW));

    return (0);
}
#endif


/****************************************************************************************/
#ifndef SSP_NO_ROW2
LONG FP_Row2(fSetProp,hCtl,hWnd,Msg,wParam,lParam,lpSpread)
BOOL            fSetProp;
HCTL            hCtl;
HWND            hWnd;
WORD            Msg;
WORD            wParam;
LONG            lParam;
LPVBSPREAD      lpSpread;
{
SS_COORD        Row;

    if (!fSetProp)
        {
        Row = lpSpread->Row2;
        if (Row == SS_HEADER)
           Row = 0;

        *(long FAR *)lParam = Row;
        return (0);
        }

    if ((SS_COORD)lParam >= -1)
       {
       lpSpread->Row2 = (SS_COORD)lParam;
       if (lpSpread->Row2 == 0)
          lpSpread->Row2 = SS_HEADER;
       else
          lpSpread->Row2 = min(lpSpread->Row2, lpSpread->lMaxRows);
       }
    else
       return (vbSpreadSetError(ERR_RANGE_ROW));

    return (0);
}
#endif


/****************************************************************************************/
#ifndef SSP_NO_ROWHEIGHT
LONG FP_RowHeight(fSetProp,hCtl,hWnd,Msg,wParam,lParam,lpSpread)
BOOL            fSetProp;
HCTL            hCtl;
HWND            hWnd;
WORD            Msg;
WORD            wParam;
LONG            lParam;
LPVBSPREAD      lpSpread;
{
int             Ret = TRUE;
LPDATASTRUCT    lpDs;
short           nRet;
float           fNumber;
SS_COORD        Row;
double          dfVal;

    lpDs = (LPDATASTRUCT)lParam;
    Row = (SS_COORD)(short)lpDs->index[0].data;

    if (nRet = SS_AdjustPropArrayRow(lpSpread, &Row))
       return (nRet);

    if (!fSetProp)
        {
        SSGetRowHeight(hWnd, Row, &dfVal);
        fNumber = (float)dfVal;
        _fmemcpy(&lpDs->data, &fNumber, sizeof(float));
        return (0);
        }

    _fmemcpy(&fNumber, &lpDs->data, sizeof(float));
    Ret = SSSetRowHeight(hWnd, Row, fNumber);

    if (!Ret)
       return (ERR_INVALID_PROPVAL);
    return (0);
}
#endif


/****************************************************************************************/
#ifndef SSP_NO_SELECTBLOCKCOL
LONG FP_SelectBlockCol(fSetProp,hCtl,hWnd,Msg,wParam,lParam,lpSpread)
BOOL            fSetProp;
HCTL            hCtl;
HWND            hWnd;
WORD            Msg;
WORD            wParam;
LONG            lParam;
LPVBSPREAD      lpSpread;
{
SS_CELLCOORD    CellUL;
SS_CELLCOORD    CellLR;
SS_COORD        Col;
SS_COORD        Row;

    if (SSGetSelectBlock(hWnd, &CellUL, &CellLR))
       Col = CellUL.Col;
    else
       SSGetActiveCell(hWnd, &Col, &Row);

    if (Col <= 0)
       Col = -1;

    *(long FAR *)lParam = Col;
    return (0);
}
#endif


/****************************************************************************************/
#ifndef SSP_NO_SELECTBLOCKCOL2
LONG FP_SelectBlockCol2(fSetProp,hCtl,hWnd,Msg,wParam,lParam,lpSpread)
BOOL            fSetProp;
HCTL            hCtl;
HWND            hWnd;
WORD            Msg;
WORD            wParam;
LONG            lParam;
LPVBSPREAD      lpSpread;
{
SS_CELLCOORD    CellUL;
SS_CELLCOORD    CellLR;

    if (SSGetSelectBlock(hWnd, &CellUL, &CellLR))
       *(long FAR *)lParam = CellLR.Col;
    else
       *(long FAR *)lParam = -1;

    return (0);
}
#endif


/****************************************************************************************/
#ifndef SSP_NO_SELECTBLOCKROW
LONG FP_SelectBlockRow(fSetProp,hCtl,hWnd,Msg,wParam,lParam,lpSpread)
BOOL            fSetProp;
HCTL            hCtl;
HWND            hWnd;
WORD            Msg;
WORD            wParam;
LONG            lParam;
LPVBSPREAD      lpSpread;
{
SS_CELLCOORD    CellUL;
SS_CELLCOORD    CellLR;
SS_COORD        Col;
SS_COORD        Row;

    if (SSGetSelectBlock(hWnd, &CellUL, &CellLR))
       Row = CellUL.Row;
    else
       SSGetActiveCell(hWnd, &Col, &Row);

    if (Row <= 0)
       Row = -1;

    *(long FAR *)lParam = Row;
    return (0);
}
#endif


/****************************************************************************************/
#ifndef SSP_NO_SELECTBLOCKROW2
LONG FP_SelectBlockRow2(fSetProp,hCtl,hWnd,Msg,wParam,lParam,lpSpread)
BOOL            fSetProp;
HCTL            hCtl;
HWND            hWnd;
WORD            Msg;
WORD            wParam;
LONG            lParam;
LPVBSPREAD      lpSpread;
{
SS_CELLCOORD    CellUL;
SS_CELLCOORD    CellLR;

    if (SSGetSelectBlock(hWnd, &CellUL, &CellLR))
        *(long FAR *)lParam = CellLR.Row;
    else
        *(long FAR *)lParam = -1;

    return (0);
}
#endif


/****************************************************************************************/
#ifndef SSP_NO_SELLENGTH
LONG FP_SelLength(fSetProp,hCtl,hWnd,Msg,wParam,lParam,lpSpread)
BOOL            fSetProp;
HCTL            hCtl;
HWND            hWnd;
WORD            Msg;
WORD            wParam;
LONG            lParam;
LPVBSPREAD      lpSpread;
{
short           dStart;
short           dEnd;

    if (!fSetProp)
        {
        SSGetSel(hWnd, &dStart, &dEnd);
        *((long FAR *)lParam) = dEnd - dStart;
        return (0);
        }

    if (SSGetSel(hWnd, &dStart, &dEnd))
       SSSetSel(hWnd, dStart, dStart + (short)lParam);

    return (0);
}
#endif


/****************************************************************************************/
#ifndef SSP_NO_SELSTART
LONG FP_SelStart(fSetProp,hCtl,hWnd,Msg,wParam,lParam,lpSpread)
BOOL            fSetProp;
HCTL            hCtl;
HWND            hWnd;
WORD            Msg;
WORD            wParam;
LONG            lParam;
LPVBSPREAD      lpSpread;
{
short           dStart;
short           dEnd;

    if (!fSetProp)
        {
        SSGetSel(hWnd, &dStart, &dEnd);
        *((long FAR *)lParam) = dStart;
        return (0);
        }

    if (SSGetSel(hWnd, &dStart, &dEnd))
       SSSetSel(hWnd, (short)lParam, max((short)lParam,dEnd));

    return (0);
}
#endif


/****************************************************************************************/
#ifndef SSP_NO_SELTEXT
LONG FP_SelText(fSetProp,hCtl,hWnd,Msg,wParam,lParam,lpSpread)
BOOL            fSetProp;
HCTL            hCtl;
HWND            hWnd;
WORD            Msg;
WORD            wParam;
LONG            lParam;
LPVBSPREAD      lpSpread;
{
LPSTR           Data;
HANDLE          hData;

    if (!fSetProp)
        {
        if ((hData = SSGetSelText(hWnd)) != NULL)
           {
           Data = GlobalLock(hData);
           *(HSZ FAR *)lParam = VBCreateHsz(HIWORD(hCtl), Data);
           GlobalUnlock(hData);
           GlobalFree(hData);
           }

        else
           *(HSZ FAR *)lParam = VBCreateHsz(HIWORD(hCtl), "");

        return (0);
        }

    SSReplaceSel(hWnd, (LPSTR)lParam);
    return (0);
}
#endif


/****************************************************************************************/
#if (!defined(SSP_NO_SHADOWCOLOR) || !defined(SSP_NO_SHADOWDARK) || !defined(SSP_NO_SHADOWTEXT))
LONG FP_Shadow(fSetProp,hCtl,hWnd,Msg,wParam,lParam,lpSpread)
BOOL            fSetProp;
HCTL            hCtl;
HWND            hWnd;
WORD            Msg;
WORD            wParam;
LONG            lParam;
LPVBSPREAD      lpSpread;
{
    if (!fSetProp)
        {
        return (0);
        }

    SSSetShadowColor(hWnd, lpSpread->ShadowColor,
                     lpSpread->ShadowText, lpSpread->ShadowDark,
                     RGBCOLOR_DEFAULT);
    return (0);
}
#endif


/****************************************************************************************/
#ifndef SSP_NO_TEXT
LONG FP_Text(fSetProp,hCtl,hWnd,Msg,wParam,lParam,lpSpread)
BOOL            fSetProp;
HCTL            hCtl;
HWND            hWnd;
WORD            Msg;
WORD            wParam;
LONG            lParam;
LPVBSPREAD      lpSpread;
{
int             Ret = TRUE;

if (!fSetProp)
   {
   VBSSVALCOLROW(lpSpread);
   FPx_GetData(hWnd, hCtl, lpSpread->Col, lpSpread->Row, lpSpread, lParam);
   }

else
   {
   if (lpSpread->BlockMode)
      {
      VBSSVALCOLROW(lpSpread);
      VBSSVALCOL2ROW2(lpSpread);
      Ret = SSSetDataRange(hWnd, lpSpread->Col, lpSpread->Row,
                           lpSpread->Col2, lpSpread->Row2,
                           (LPSTR)lParam);
      }
   else
      Ret = SSSetData(hWnd, lpSpread->Col, lpSpread->Row,
                      (LPSTR)lParam);

   if (!Ret)
      return (ERR_INVALID_PROPVAL);
   }

return (0);
}


void FAR PASCAL FPx_GetData(hWnd, hCtl, Col, Row, lpSpread, lParam)

HWND           hWnd;
HCTL           hCtl;
SS_COORD       Col;
SS_COORD       Row;
LPVBSPREAD     lpSpread;
LPARAM         lParam;
{
LPSPREADSHEET  lpSS;
TBGLOBALHANDLE hData;
SS_CELLTYPE    CellType;
LPSTR          lpData;
BOOL           Ret = FALSE;

lpSS = SS_Lock(hWnd);

if (Col != SS_ALLCOLS)
   if (Col < 0)
      Col += -SS_HEADER;

if (Row != SS_ALLROWS)
   if (Row < 0)
      Row += -SS_HEADER;

SS_RetrieveCellType(lpSS, &CellType, NULL, Col, Row);

if (lpSS->EditModeOn && lpSS->Col.CurAt == Col &&
    lpSS->Row.CurAt == Row)
   hData = SS_GetEditModeOnData(hWnd, lpSS, FALSE);
else
   hData = SS_GetData(lpSS, &CellType, Col, Row, FALSE);

if (hData)
   {
   lpData = (LPSTR)tbGlobalLock(hData);
   *(HSZ FAR *)lParam = VBCreateHsz(HIWORD(hCtl), lpData);
   tbGlobalUnlock(hData);

   if (lpSS->EditModeOn && lpSS->Col.CurAt == Col && lpSS->Row.CurAt == Row)
      tbGlobalFree(hData);

   Ret = TRUE;
   }

else
   *(HSZ FAR *)lParam = VBCreateHsz(HIWORD(hCtl), "");

SS_Unlock(hWnd);
}
#endif


/****************************************************************************************/
#ifndef SSP_NO_TOPROW
LONG FP_TopRow(fSetProp,hCtl,hWnd,Msg,wParam,lParam,lpSpread)
BOOL            fSetProp;
HCTL            hCtl;
HWND            hWnd;
WORD            Msg;
WORD            wParam;
LONG            lParam;
LPVBSPREAD      lpSpread;
{
SS_COORD        Col;
SS_COORD        Row;

    if (!fSetProp)
        {
#ifdef SS_USE16BITCOORDS
        long lCell;

        lCell = SSGetTopLeftCell(hWnd);
        Col = LOWORD(lParam);
        Row = HIWORD(lParam);
#else
        SSGetTopLeftCell(hWnd, &Col, &Row);
#endif
        *(SS_COORD FAR *)lParam = Row;
        return (0);
        }

    SSShowCell(hWnd, -1, (SS_COORD)lParam, SS_SHOW_TOPLEFT);
    return (0);
}
#endif


/****************************************************************************************/
LONG FP_CellType2(fSetProp,hCtl,hWnd,Msg,wParam,lParam,lpSpread)
BOOL            fSetProp;
HCTL            hCtl;
HWND            hWnd;
WORD            Msg;
WORD            wParam;
LONG            lParam;
LPVBSPREAD      lpSpread;
{
int             Ret = TRUE;

    if (!fSetProp)
       Ret = GetCellTypeProperties(hCtl, hWnd, lpSpread, wParam, lParam);
    else
       Ret = SetCellTypeProperties(hWnd, lpSpread, wParam, lParam);

    if (!Ret)
       return (ERR_INVALID_PROPVAL);

    return (0);
}


/****************************************************************************************/
#ifndef SSP_NO_PICMASK
LONG FP_PicMask(fSetProp,hCtl,hWnd,Msg,wParam,lParam,lpSpread)
BOOL            fSetProp;
HCTL            hCtl;
HWND            hWnd;
WORD            wParam;
LONG            lParam;
LPVBSPREAD      lpSpread;
{
int             Ret = TRUE;
LPSTR           Data;

    if (!fSetProp)
        Ret = GetCellTypeProperties(hCtl, hWnd, lpSpread, wParam, lParam);

    else
       {
       if (lpSpread->hTypePicMask)
          GlobalFree(lpSpread->hTypePicMask);

       lpSpread->hTypePicMask = 0;

       if (lParam && lstrlen((LPSTR)lParam))
          if (lpSpread->hTypePicMask = GlobalAlloc(GMEM_MOVEABLE |
                                       GMEM_ZEROINIT,
                                       lstrlen((LPSTR)lParam) + 1))
             {
             Data = (LPSTR)GlobalLock(lpSpread->hTypePicMask);
             lstrcpy(Data, (LPSTR)lParam);
             GlobalUnlock(lpSpread->hTypePicMask);
             }

       Ret = SetCellTypeProperties(hWnd, lpSpread, wParam, lParam);
       }

    if (!Ret)
       return (ERR_INVALID_PROPVAL);

    return (0);
}
#endif


/****************************************************************************************/
#ifndef SSP_NO_TIMEMAX
LONG FP_TimeMax(fSetProp,hCtl,hWnd,Msg,wParam,lParam,lpSpread)
BOOL            fSetProp;
HCTL            hCtl;
HWND            hWnd;
WORD            Msg;
WORD            wParam;
LONG            lParam;
LPVBSPREAD      lpSpread;
{
int             Ret = TRUE;
TIME            Time;

    if (!fSetProp)
        Ret = GetCellTypeProperties(hCtl, hWnd, lpSpread, wParam, lParam);

    else
       {
       if (TimeValidate((LPSTR)lParam, &Time))
          _fmemcpy(&lpSpread->TypeTimeMax, &Time, sizeof(TIME));

       Ret = SetCellTypeProperties(hWnd, lpSpread, wParam, lParam);
       }

    if (!Ret)
       return (ERR_INVALID_PROPVAL);

    return (0);
}
#endif


/****************************************************************************************/
#ifndef SSP_NO_TIMEMIN
LONG FP_TimeMin(fSetProp,hCtl,hWnd,Msg,wParam,lParam,lpSpread)
BOOL            fSetProp;
HCTL            hCtl;
HWND            hWnd;
WORD            Msg;
WORD            wParam;
LONG            lParam;
LPVBSPREAD      lpSpread;
{
int             Ret = TRUE;
TIME            Time;

    if (!fSetProp)
        Ret = GetCellTypeProperties(hCtl, hWnd, lpSpread, wParam, lParam);

    else
       {
       if (TimeValidate((LPSTR)lParam, &Time))
          _fmemcpy(&lpSpread->TypeTimeMin, &Time, sizeof(TIME));

       Ret = SetCellTypeProperties(hWnd, lpSpread, wParam, lParam);
       }

    if (!Ret)
       return (ERR_INVALID_PROPVAL);

    return (0);
}
#endif


/****************************************************************************************/
#ifndef SSP_NO_SCROLLBARS
LONG FP_ScrollBars(fSetProp,hCtl,hWnd,Msg,wParam,lParam,lpSpread)
BOOL            fSetProp;
HCTL            hCtl;
HWND            hWnd;
WORD            Msg;
WORD            wParam;
LONG            lParam;
LPVBSPREAD      lpSpread;
{
WORD            wScrollBars;
BOOL            hHorz;
BOOL            hVert;

    if (!fSetProp)
       {
       hHorz = SSGetBool(hWnd, SSB_HORZSCROLLBAR);
       hVert = SSGetBool(hWnd, SSB_VERTSCROLLBAR);

       wScrollBars = 0;

       if (hVert)
          wScrollBars = 2;

       if (hHorz)
          wScrollBars++;

        *(WORD FAR *)lParam = wScrollBars;
       }

    else
       {
       BOOL fRedraw = SSGetBool(hWnd, SSB_REDRAW);
       SSSetBool(hWnd, SSB_REDRAW, FALSE);

       if ((short)lParam == 0)
          {
          SSSetBool(hWnd, SSB_HORZSCROLLBAR, FALSE);
          SSSetBool(hWnd, SSB_VERTSCROLLBAR, FALSE);
          }
       else if ((short)lParam == 1)
          {
          SSSetBool(hWnd, SSB_HORZSCROLLBAR, TRUE);
          SSSetBool(hWnd, SSB_VERTSCROLLBAR, FALSE);
          }
       else if ((short)lParam == 2)
          {
          SSSetBool(hWnd, SSB_HORZSCROLLBAR, FALSE);
          SSSetBool(hWnd, SSB_VERTSCROLLBAR, TRUE);
          }
       else if ((short)lParam == 3)
          {
          SSSetBool(hWnd, SSB_HORZSCROLLBAR, TRUE);
          SSSetBool(hWnd, SSB_VERTSCROLLBAR, TRUE);
          }

       SSSetBool(hWnd, SSB_REDRAW, fRedraw);
       }

    return (0);
}
#endif


/****************************************************************************************/
#ifndef SSP_NO_ISBLOCKSELECTED
LONG FP_IsBlockSelected(fSetProp,hCtl,hWnd,Msg,wParam,lParam,lpSpread)
BOOL            fSetProp;
HCTL            hCtl;
HWND            hWnd;
WORD            Msg;
WORD            wParam;
LONG            lParam;
LPVBSPREAD      lpSpread;
{
    if (SSGetSelectBlock(hWnd, NULL, NULL))
       *((BOOL FAR *)lParam) = 1;
    else
       *((BOOL FAR *)lParam) = 0;

    return (0);
}
#endif


/****************************************************************************************/
#ifndef SSP_NO_VALUE
LONG FP_Value(fSetProp,hCtl,hWnd,Msg,wParam,lParam,lpSpread)
BOOL            fSetProp;
HCTL            hCtl;
HWND            hWnd;
WORD            Msg;
WORD            wParam;
LONG            lParam;
LPVBSPREAD      lpSpread;
{
int             Ret = TRUE;
LPSTR           Data;
HANDLE          hData;

    if (!fSetProp)
        {
        VBSSVALCOLROW(lpSpread);

        if (hData = GlobalAlloc(GMEM_MOVEABLE | GMEM_ZEROINIT,
                                 (long)SSGetValueLen(hWnd,
                                 lpSpread->Col, lpSpread->Row) + 1L))
           {
           Data = GlobalLock(hData);
           SSGetValue(hWnd, lpSpread->Col, lpSpread->Row, Data);

           *(HSZ FAR *)lParam = VBCreateHsz(HIWORD(hCtl), Data);
           GlobalUnlock(hData);
           GlobalFree(hData);
           }

        return (0);
        }

    if (lpSpread->BlockMode)
       {
       VBSSVALCOLROW(lpSpread);
       VBSSVALCOL2ROW2(lpSpread);
       Ret = SSSetValueRange(hWnd, lpSpread->Col, lpSpread->Row,
                             lpSpread->Col2, lpSpread->Row2,
                             (LPSTR)lParam);
       }
    else
       {
       VBSSVALCOLROW(lpSpread);
       Ret = SSSetValue(hWnd, lpSpread->Col, lpSpread->Row,
                        (LPSTR)lParam);
       }

    if (!Ret)
       return (ERR_INVALID_PROPVAL);

    return (0);
}
#endif


/****************************************************************************************/
#ifndef SSP_NO_PICDEFTEXT
LONG FP_PicDefText(fSetProp,hCtl,hWnd,Msg,wParam,lParam,lpSpread)
BOOL            fSetProp;
HCTL            hCtl;
HWND            hWnd;
WORD            Msg;
WORD            wParam;
LONG            lParam;
LPVBSPREAD      lpSpread;
{
int             Ret = TRUE;
LPSTR           Data;

    if (!fSetProp)
        Ret = GetCellTypeProperties(hCtl, hWnd, lpSpread, wParam, lParam);

    else
       {
       if (lpSpread->hTypePicDefText)
          GlobalFree(lpSpread->hTypePicDefText);

       lpSpread->hTypePicDefText = 0;

       if (lParam && lstrlen((LPSTR)lParam))
          if (lpSpread->hTypePicDefText = GlobalAlloc(GMEM_MOVEABLE |
                                          GMEM_ZEROINIT,
                                          lstrlen((LPSTR)lParam) + 1))
             {
             Data = (LPSTR)GlobalLock(lpSpread->hTypePicDefText);
             lstrcpy(Data, (LPSTR)lParam);
             GlobalUnlock(lpSpread->hTypePicDefText);
             }

       Ret = SetCellTypeProperties(hWnd, lpSpread, wParam, lParam);
       }

    if (!Ret)
       return (ERR_INVALID_PROPVAL);

    return (0);
}
#endif


/****************************************************************************************/
#ifndef SSP_NO_VISIBLEROWS
LONG FP_VisibleRows(fSetProp,hCtl,hWnd,Msg,wParam,lParam,lpSpread)
BOOL            fSetProp;
HCTL            hCtl;
HWND            hWnd;
WORD            Msg;
WORD            wParam;
LONG            lParam;
LPVBSPREAD      lpSpread;
{
SS_COORD        Row;
SS_COORD        Col;

    SSGetAutoSizeVisible(hWnd, &Col, &Row);

    if (!fSetProp)
        *(long FAR *)lParam = Row;
    else
       SSSetAutoSizeVisible(hWnd, Col, (SS_COORD)lParam);

    return (0);
}
#endif


/****************************************************************************************/
#ifndef SSP_NO_VISIBLECOLS
LONG FP_VisibleCols(fSetProp,hCtl,hWnd,Msg,wParam,lParam,lpSpread)
BOOL            fSetProp;
HCTL            hCtl;
HWND            hWnd;
WORD            Msg;
WORD            wParam;
LONG            lParam;
LPVBSPREAD      lpSpread;
{
SS_COORD        Row;
SS_COORD        Col;

    SSGetAutoSizeVisible(hWnd, &Col, &Row);

    if (!fSetProp)
        *(long FAR *)lParam = Col;
    else
       SSSetAutoSizeVisible(hWnd, (SS_COORD)lParam, Row);

    return (0);
}
#endif


/****************************************************************************************/
#ifndef SSP_NO_ROWSFROZEN
LONG FP_RowsFrozen(fSetProp,hCtl,hWnd,Msg,wParam,lParam,lpSpread)
BOOL            fSetProp;
HCTL            hCtl;
HWND            hWnd;
WORD            Msg;
WORD            wParam;
LONG            lParam;
LPVBSPREAD      lpSpread;
{
SS_COORD        Row;
SS_COORD        Col;

    SSGetFreeze(hWnd, &Col, &Row);

    if (!fSetProp)
        *(long FAR *)lParam = Row;
    else
       SSSetFreeze(hWnd, Col, (SS_COORD)lParam);

    return (0);
}
#endif


/****************************************************************************************/
#ifndef SSP_NO_COLSFROZEN
LONG FP_ColsFrozen(fSetProp,hCtl,hWnd,Msg,wParam,lParam,lpSpread)
BOOL            fSetProp;
HCTL            hCtl;
HWND            hWnd;
WORD            Msg;
WORD            wParam;
LONG            lParam;
LPVBSPREAD      lpSpread;
{
SS_COORD        Row;
SS_COORD        Col;

    SSGetFreeze(hWnd, &Col, &Row);

    if (!fSetProp)
        *(long FAR *)lParam = Col;
    else
       SSSetFreeze(hWnd, (SS_COORD)lParam, Row);

    return (0);
}
#endif


/****************************************************************************************/
#ifndef SSP_NO_BUTTONTEXT
LONG FP_ButtonText(fSetProp,hCtl,hWnd,Msg,wParam,lParam,lpSpread)
BOOL            fSetProp;
HCTL            hCtl;
HWND            hWnd;
WORD            Msg;
WORD            wParam;
LONG            lParam;
LPVBSPREAD      lpSpread;
{
int             Ret = TRUE;
LPSTR           Data;

    if (!fSetProp)
        Ret = GetCellTypeProperties(hCtl, hWnd, lpSpread, wParam, lParam);

    else
       {
       if (lpSpread->hTypeButtonText)
          GlobalFree(lpSpread->hTypeButtonText);

       lpSpread->hTypeButtonText = 0;

       if (lParam && lstrlen((LPSTR)lParam))
          if (lpSpread->hTypeButtonText = GlobalAlloc(GMEM_MOVEABLE |
                                          GMEM_ZEROINIT,
                                          lstrlen((LPSTR)lParam) + 1))
             {
             Data = (LPSTR)GlobalLock(lpSpread->hTypeButtonText);
             lstrcpy(Data, (LPSTR)lParam);
             GlobalUnlock(lpSpread->hTypeButtonText);
             }

       Ret = SetCellTypeProperties(hWnd, lpSpread, wParam, lParam);
       }

    if (!Ret)
       return (ERR_INVALID_PROPVAL);

    return (0);
}
#endif


/****************************************************************************************/
#ifndef SSP_NO_BUTTONPICTNAME
LONG FP_ButtonPicName(fSetProp,hCtl,hWnd,Msg,wParam,lParam,lpSpread)
BOOL            fSetProp;
HCTL            hCtl;
HWND            hWnd;
WORD            Msg;
WORD            wParam;
LONG            lParam;
LPVBSPREAD      lpSpread;
{
int             Ret = TRUE;

    if (!fSetProp)
        Ret = GetCellTypeProperties(hCtl, hWnd, lpSpread, wParam, lParam);

    else
       {
       FP_ProcessButtonPict(lParam,lpSpread,
                            &lpSpread->hTypeButtonPictureName,
                            &lpSpread->TypeButtonPictureType);

       Ret = SetCellTypeProperties(hWnd, lpSpread, wParam, lParam);
       }

    if (!Ret)
       return (ERR_INVALID_PROPVAL);

    return (0);
}
#endif


/****************************************************************************************/
#ifndef SSP_NO_BUTTONPICTDOWN
LONG FP_ButtonPicDown(fSetProp,hCtl,hWnd,Msg,wParam,lParam,lpSpread)
BOOL            fSetProp;
HCTL            hCtl;
HWND            hWnd;
WORD            Msg;
WORD            wParam;
LONG            lParam;
LPVBSPREAD      lpSpread;
{
int             Ret = TRUE;

    if (!fSetProp)
        Ret = GetCellTypeProperties(hCtl, hWnd, lpSpread, wParam, lParam);

    else
       {
       FP_ProcessButtonPict(lParam,lpSpread,
                            &lpSpread->hTypeButtonPictureDownName,
                            &lpSpread->TypeButtonPictureDownType);

       Ret = SetCellTypeProperties(hWnd, lpSpread, wParam, lParam);
       }

    if (!Ret)
       return (ERR_INVALID_PROPVAL);

    return (0);
}
#endif


#if (!defined(SS_NOCT_CHECK) || !defined(SS_NOCT_BUTTON))
void FP_ProcessButtonPict(lParam,lpSpread,lphPictName,lpPictType)
LONG            lParam;
LPVBSPREAD      lpSpread;
LPHANDLE        lphPictName;
LPSHORT         lpPictType;
{
PIC             Pic;

lpSpread->hPic = (HPIC)lParam;

VBGetPic((HPIC)lParam, &Pic);

switch (Pic.picType)
   {
   case PICTYPE_NONE:
   case PICTYPE_METAFILE:
      *lphPictName = 0;
      *lpPictType = SUPERBTN_PICT_NONE;
      break;

   case PICTYPE_BITMAP:
      *lphPictName = Pic.picData.bmp.hbitmap;
      *lpPictType = SUPERBTN_PICT_BITMAP | SUPERBTN_PICT_HANDLE;
      break;

   case PICTYPE_ICON:
      *lphPictName = Pic.picData.icon.hicon;
      *lpPictType = SUPERBTN_PICT_ICON | SUPERBTN_PICT_HANDLE;
      break;
   }
}
#endif


/****************************************************************************************/
#ifndef SSP_NO_LOCKBACKCOLOR
LONG FP_LockBackColor(fSetProp,hCtl,hWnd,Msg,wParam,lParam,lpSpread)
BOOL            fSetProp;
HCTL            hCtl;
HWND            hWnd;
WORD            Msg;
WORD            wParam;
LONG            lParam;
LPVBSPREAD      lpSpread;
{
COLORREF        Color;

    if (!fSetProp)
        {
        SSGetLockColor(hWnd, (LPCOLORREF)lParam, NULL);
        return (0);
        }

    SSGetLockColor(hWnd, NULL, &Color);

    SSSetLockColor(hWnd, lParam, Color);
    return (0);
}
#endif


/****************************************************************************************/
#ifndef SSP_NO_LOCKFORECOLOR
LONG FP_LockForeColor(fSetProp,hCtl,hWnd,Msg,wParam,lParam,lpSpread)
BOOL            fSetProp;
HCTL            hCtl;
HWND            hWnd;
WORD            Msg;
WORD            wParam;
LONG            lParam;
LPVBSPREAD      lpSpread;
{
COLORREF        Color;

    if (!fSetProp)
        {
        SSGetLockColor(hWnd, NULL, (LPCOLORREF)lParam);
        return (0);
        }

    SSGetLockColor(hWnd, &Color, NULL);

    SSSetLockColor(hWnd, Color, lParam);
    return (0);
}
#endif


/****************************************************************************************/
#ifndef SSP_NO_COMBOLIST
LONG FP_ComboList(fSetProp,hCtl,hWnd,Msg,wParam,lParam,lpSpread)
BOOL            fSetProp;
HCTL            hCtl;
HWND            hWnd;
WORD            Msg;
WORD            wParam;
LONG            lParam;
LPVBSPREAD      lpSpread;
{
int             Ret = TRUE;
LPSTR           Data;

    if (!fSetProp)
       Ret = GetCellTypeProperties(hCtl, hWnd, lpSpread, wParam, lParam);

    else
       {
       if (lpSpread->hTypeComboList)
          GlobalFree(lpSpread->hTypeComboList);

       lpSpread->hTypeComboList = 0;
       
       if (lParam && lstrlen((LPSTR)lParam))
          if (lpSpread->hTypeComboList = GlobalAlloc(GMEM_MOVEABLE |
                                         GMEM_ZEROINIT,
                                         lstrlen((LPSTR)lParam) + 1))
             {
             Data = (LPSTR)GlobalLock(lpSpread->hTypeComboList);
             lstrcpy(Data, (LPSTR)lParam);
             GlobalUnlock(lpSpread->hTypeComboList);
             }

       Ret = SetCellTypeProperties(hWnd, lpSpread, wParam, lParam);
       }

    if (!Ret)
       return (ERR_INVALID_PROPVAL);

    return (0);
}
#endif


/****************************************************************************************/
#ifndef SS_NOPRINT
LONG FP_Print(fSetProp,hCtl,hWnd,Msg,wParam,lParam,lpSpread)
BOOL            fSetProp;
HCTL            hCtl;
HWND            hWnd;
WORD            Msg;
WORD            wParam;
LONG            lParam;
LPVBSPREAD      lpSpread;
{
int             Ret = TRUE;
LPSTR           lpText;

if (!fSetProp)
   {
   switch (wParam)
      {
      case IPROP_SPREAD_PRINTJOBNAME:
         if (lpSpread->Print.hPrintJobName)
            {
            lpText = (LPSTR)GlobalLock(lpSpread->Print.hPrintJobName);
            *(HSZ FAR *)lParam = VBCreateHsz(HIWORD(hCtl), lpText);
            GlobalUnlock(lpSpread->Print.hPrintJobName);
            }
         else
            *(HSZ FAR *)lParam = VBCreateHsz(HIWORD(hCtl), "");

         break;

      case IPROP_SPREAD_PRINTABORTMSG:
         if (lpSpread->Print.hPrintAbortMsg)
            {
            lpText = (LPSTR)GlobalLock(lpSpread->Print.hPrintAbortMsg);
            *(HSZ FAR *)lParam = VBCreateHsz(HIWORD(hCtl), lpText);
            GlobalUnlock(lpSpread->Print.hPrintAbortMsg);
            }
         else
            *(HSZ FAR *)lParam = VBCreateHsz(HIWORD(hCtl), "");

         break;

      case IPROP_SPREAD_PRINTHEADER:
         if (lpSpread->Print.hPrintHeader)
            {
            lpText = (LPSTR)GlobalLock(lpSpread->Print.hPrintHeader);
            *(HSZ FAR *)lParam = VBCreateHsz(HIWORD(hCtl), lpText);
            GlobalUnlock(lpSpread->Print.hPrintHeader);
            }
         else
            *(HSZ FAR *)lParam = VBCreateHsz(HIWORD(hCtl), "");

         break;

      case IPROP_SPREAD_PRINTFOOTER:
         if (lpSpread->Print.hPrintFooter)
            {
            lpText = (LPSTR)GlobalLock(lpSpread->Print.hPrintFooter);
            *(HSZ FAR *)lParam = VBCreateHsz(HIWORD(hCtl), lpText);
            GlobalUnlock(lpSpread->Print.hPrintFooter);
            }
         else
            *(HSZ FAR *)lParam = VBCreateHsz(HIWORD(hCtl), "");

         break;
      }
   }

else
   {
   switch (wParam)
      {
      case IPROP_SPREAD_PRINTJOBNAME:
         if (lpSpread->Print.hPrintJobName)
            GlobalFree(lpSpread->Print.hPrintJobName);

         lpSpread->Print.hPrintJobName = 0;

         if (lParam && lstrlen((LPSTR)lParam))
            if (lpSpread->Print.hPrintJobName = GlobalAlloc(GMEM_MOVEABLE |
                                          GMEM_ZEROINIT,
                                          lstrlen((LPSTR)lParam) + 1))
               {
               lpText = (LPSTR)GlobalLock(lpSpread->Print.hPrintJobName);
               lstrcpy(lpText, (LPSTR)lParam);
               GlobalUnlock(lpSpread->Print.hPrintJobName);
               }

         break;

      case IPROP_SPREAD_PRINTABORTMSG:
         if (lpSpread->Print.hPrintAbortMsg)
            GlobalFree(lpSpread->Print.hPrintAbortMsg);

         lpSpread->Print.hPrintAbortMsg = 0;

         if (lParam && lstrlen((LPSTR)lParam))
            if (lpSpread->Print.hPrintAbortMsg = GlobalAlloc(GMEM_MOVEABLE |
                                          GMEM_ZEROINIT,
                                          lstrlen((LPSTR)lParam) + 1))
               {
               lpText = (LPSTR)GlobalLock(lpSpread->Print.hPrintAbortMsg);
               lstrcpy(lpText, (LPSTR)lParam);
               GlobalUnlock(lpSpread->Print.hPrintAbortMsg);
               }

         break;

      case IPROP_SPREAD_PRINTHEADER:
         if (lpSpread->Print.hPrintHeader)
            GlobalFree(lpSpread->Print.hPrintHeader);

         lpSpread->Print.hPrintHeader = 0;

         if (lParam && lstrlen((LPSTR)lParam))
            if (lpSpread->Print.hPrintHeader = GlobalAlloc(GMEM_MOVEABLE |
                                          GMEM_ZEROINIT,
                                          lstrlen((LPSTR)lParam) + 1))
               {
               lpText = (LPSTR)GlobalLock(lpSpread->Print.hPrintHeader);
               lstrcpy(lpText, (LPSTR)lParam);
               GlobalUnlock(lpSpread->Print.hPrintHeader);
               }

         break;

      case IPROP_SPREAD_PRINTFOOTER:
         if (lpSpread->Print.hPrintFooter)
            GlobalFree(lpSpread->Print.hPrintFooter);

         lpSpread->Print.hPrintFooter = 0;

         if (lParam && lstrlen((LPSTR)lParam))
            if (lpSpread->Print.hPrintFooter = GlobalAlloc(GMEM_MOVEABLE |
                                          GMEM_ZEROINIT,
                                          lstrlen((LPSTR)lParam) + 1))
               {
               lpText = (LPSTR)GlobalLock(lpSpread->Print.hPrintFooter);
               lstrcpy(lpText, (LPSTR)lParam);
               GlobalUnlock(lpSpread->Print.hPrintFooter);
               }

         break;

      }
   }

return (0);
}
#endif


/****************************************************************************************/
#ifndef SSP_NO_UNITTYPE
LONG FP_UnitType(fSetProp,hCtl,hWnd,Msg,wParam,lParam,lpSpread)
BOOL            fSetProp;
HCTL            hCtl;
HWND            hWnd;
WORD            Msg;
WORD            wParam;
LONG            lParam;
LPVBSPREAD      lpSpread;
{
    if (!fSetProp)
       *(short FAR *)lParam = SSGetUnitType(hWnd);
    else
       SSSetUnitType(hWnd, (WORD)lParam);

    return (0);
}
#endif


/****************************************************************************************/
#ifndef SSP_NO_EDITENTERACTION
LONG FP_EditEnterAction(fSetProp,hCtl,hWnd,Msg,wParam,lParam,lpSpread)
BOOL            fSetProp;
HCTL            hCtl;
HWND            hWnd;
WORD            Msg;
WORD            wParam;
LONG            lParam;
LPVBSPREAD      lpSpread;
{
    if (!fSetProp)
       *(short FAR *)lParam = SSGetEditEnterAction(hWnd);
    else
       SSSetEditEnterAction(hWnd, (WORD)lParam);

    return (0);
}
#endif


/****************************************************************************************/
#ifndef SSP_NO_GRAYAREABACKCOLOR
LONG FP_GrayAreaBackColor(fSetProp,hCtl,hWnd,Msg,wParam,lParam,lpSpread)
BOOL            fSetProp;
HCTL            hCtl;
HWND            hWnd;
WORD            Msg;
WORD            wParam;
LONG            lParam;
LPVBSPREAD      lpSpread;
{
COLORREF        Color;

    if (!fSetProp)
        {
        SSGetGrayAreaColor(hWnd, (LPCOLORREF)lParam, NULL);
        return (0);
        }

    SSGetGrayAreaColor(hWnd, NULL, &Color);

    SSSetGrayAreaColor(hWnd, lParam, Color);
    return (0);
}
#endif


/****************************************************************************************/
#ifndef SS_V30
#ifndef SSP_NO_GRAYAREAFORECOLOR
LONG FP_GrayAreaForeColor(fSetProp,hCtl,hWnd,Msg,wParam,lParam,lpSpread)
BOOL            fSetProp;
HCTL            hCtl;
HWND            hWnd;
WORD            Msg;
WORD            wParam;
LONG            lParam;
LPVBSPREAD      lpSpread;
{
COLORREF        Color;

    if (!fSetProp)
        {
        SSGetGrayAreaColor(hWnd, NULL, (LPCOLORREF)lParam);
        return (0);
        }

    SSGetGrayAreaColor(hWnd, &Color, NULL);
    SSSetGrayAreaColor(hWnd, Color, lParam);
    return (0);
}
#endif
#endif // SS_V30


/****************************************************************************************/
#ifndef SSP_NO_TYPEPICTPICTURE
LONG FP_TypePictPicture(fSetProp,hCtl,hWnd,Msg,wParam,lParam,lpSpread)
BOOL            fSetProp;
HCTL            hCtl;
HWND            hWnd;
WORD            Msg;
WORD            wParam;
LONG            lParam;
LPVBSPREAD      lpSpread;
{
int             Ret = TRUE;
LPSPREADSHEET   lpSS = SS_Lock(hWnd);

#ifdef SS_BOUNDCONTROL
if (fSetProp)
   VBSSBoundChangeMade(lpSS, (LONG)hCtl, hWnd, lpSpread->Col, lpSpread->Row);
#endif

SS_Unlock(hWnd);
return (FPx_TypePictPicture(fSetProp,hCtl,hWnd,Msg,wParam,lParam,lpSpread));
}


/****************************************************************************************/
LONG FPx_TypePictPicture(fSetProp,hCtl,hWnd,Msg,wParam,lParam,lpSpread)
BOOL            fSetProp;
HCTL            hCtl;
HWND            hWnd;
WORD            Msg;
WORD            wParam;
LONG            lParam;
LPVBSPREAD      lpSpread;
{
int             Ret = TRUE;
PIC             Pic;

    if (!fSetProp)
       Ret = GetCellTypeProperties(hCtl, hWnd, lpSpread, wParam, lParam);

    else
       {
       lpSpread->hPic = (HPIC)lParam;

       if (VBGetVersion() >= VB200_VERSION)
          VBGetPicEx((HPIC)lParam, &Pic, VB_VERSION);
       else
          VBGetPic((HPIC)lParam, &Pic);

       lpSpread->TypePictStyle &= ~(VPS_BMP | VPS_ICON);

       switch (Pic.picType)
          {
          case PICTYPE_NONE:
          case PICTYPE_METAFILE:
             lpSpread->hPic = 0;
             lpSpread->hTypePictPicture = 0;
             break;

          case PICTYPE_BITMAP:
             lpSpread->hTypePictPicture = Pic.picData.bmp.hbitmap;
             lpSpread->TypePictStyle |= VPS_BMP | VPS_HANDLE;

             if (VBGetVersion() >= VB200_VERSION)
                lpSpread->hTypePictPal = Pic.picData.bmp.hpal;

             break;

          case PICTYPE_ICON:
             lpSpread->hTypePictPicture = Pic.picData.icon.hicon;
             lpSpread->TypePictStyle |= VPS_ICON | VPS_HANDLE;
             break;
          }

       Ret = SetCellTypeProperties(hWnd, lpSpread, wParam, lParam);
       }

    if (!Ret)
       return (ERR_INVALID_PROPVAL);
    return (0);
}
#endif


/****************************************************************************************/
#ifndef SSP_NO_OPERATIONMODE
LONG FP_OperationMode(fSetProp,hCtl,hWnd,Msg,wParam,lParam,lpSpread)
BOOL            fSetProp;
HCTL            hCtl;
HWND            hWnd;
WORD            Msg;
WORD            wParam;
LONG            lParam;
LPVBSPREAD      lpSpread;
{
    if (!fSetProp)
        {
        *(short FAR *)lParam = SSGetOperationMode(hWnd);
        return (0);
        }

    SSSetOperationMode(hWnd, (WORD)lParam);
    return (0);
}
#endif


/****************************************************************************************/
#ifdef IPROP_SPREAD_DATACONNECT
LONG FP_DataConnect(fSetProp,hCtl,hWnd,Msg,wParam,lParam,lpSpread)
BOOL            fSetProp;
HCTL            hCtl;
HWND            hWnd;
WORD            Msg;
WORD            wParam;
LONG            lParam;
LPVBSPREAD      lpSpread;
{
return (FP_ProcessText(fSetProp,hCtl,hWnd,Msg,wParam,lParam,lpSpread,
                       &lpSpread->DBInfo.hDataConnect));
}
#endif


/****************************************************************************************/
#ifndef SSP_NO_CLIPVALUE
LONG FP_ClipValue(fSetProp,hCtl,hWnd,Msg,wParam,lParam,lpSpread)
BOOL            fSetProp;
HCTL            hCtl;
HWND            hWnd;
WORD            Msg;
WORD            wParam;
LONG            lParam;
LPVBSPREAD      lpSpread;
{
int             Ret = TRUE;
LPSTR           lpData;
HANDLE          hData;
DWORD           MaxClipSize = 60000;

    if (!fSetProp)
        {
        VBSSVALCOLROW(lpSpread);
        VBSSVALCOL2ROW2(lpSpread);
        hData = SSClipValueOut(hWnd, lpSpread->Col, lpSpread->Row,
                               lpSpread->Col2, lpSpread->Row2);
        if (hData && (GlobalSize(hData) > MaxClipSize))
            {
            lpData = (LPSTR)GlobalLock(hData);
            lpData[MaxClipSize-1] = '\0';
            GlobalUnlock(hData);
            hData = GlobalReAlloc(hData, MaxClipSize, GMEM_MOVEABLE);
            }
        if (hData)
            {
            lpData = GlobalLock(hData);
            *(HSZ FAR *)lParam = VBCreateHsz(HIWORD(hCtl), lpData);
            GlobalUnlock(hData);
            GlobalFree(hData);
            }
        else
            {
            *(HSZ FAR *)lParam = VBCreateHsz(HIWORD(hCtl), "");
            Ret = FALSE;
            }

            /*
            if (!Ret)
               return (ERR_INVALID_PROPVAL);
            */

        return (0);
        }

    if (lParam)
        {
        VBSSVALCOLROW(lpSpread);
        VBSSVALCOL2ROW2(lpSpread);
        Ret = SSClipValueIn(hWnd, lpSpread->Col, lpSpread->Row,lpSpread->Col2, lpSpread->Row2,(LPSTR)lParam, lstrlen((LPSTR)lParam));
        }
    else
        Ret = 0;
    if (!Ret)
        return (ERR_INVALID_PROPVAL);
    return (0);
 }
#endif


/****************************************************************************************/
#ifndef SSP_NO_HWND
LONG FP_Spread_hWnd(fSetProp,hCtl,hWnd,Msg,wParam,lParam,lpSpread)
BOOL            fSetProp;
HCTL            hCtl;
HWND            hWnd;
WORD            Msg;
WORD            wParam;
LONG            lParam;
LPVBSPREAD      lpSpread;
{
*(HWND FAR *)lParam = hWnd;
return (0);
}
#endif


/****************************************************************************************/
#ifndef SSP_NO_LOADTABFILE
LONG FP_LoadTabFile(fSetProp,hCtl,hWnd,Msg,wParam,lParam,lpSpread)
BOOL            fSetProp;
HCTL            hCtl;
HWND            hWnd;
WORD            Msg;
WORD            wParam;
LONG            lParam;
LPVBSPREAD      lpSpread;
{
HCURSOR         hCursor;
int             Ret = TRUE;

    if (!fSetProp)
        {
        return (0);
        }

    if (lParam)
        {
        hCursor = SetCursor(LoadCursor(NULL, IDC_WAIT));
        ShowCursor(TRUE);

        Ret = SSLoadFromFile(hWnd, (LPSTR)lParam);
        lpSpread->lMaxCols = SSGetMaxCols(hWnd);
        lpSpread->lMaxRows = SSGetMaxRows(hWnd);

        ShowCursor(FALSE);
        SetCursor(hCursor);
        }
    else
        Ret = 0;

    if (!Ret)
        return (ERR_INVALID_PROPVAL);
    return (0);
}
#endif


/****************************************************************************************/
#ifndef SSP_NO_MAXTEXTROWHEIGHT
LONG FP_MaxTextRowHeight(fSetProp,hCtl,hWnd,Msg,wParam,lParam,lpSpread)
BOOL            fSetProp;
HCTL            hCtl;
HWND            hWnd;
WORD            Msg;
WORD            wParam;
LONG            lParam;
LPVBSPREAD      lpSpread;
{
LPDATASTRUCT    lpDs;
float           fNumber;
SS_COORD        Row;
short           nRet;
double          dfVal;

    lpDs = (LPDATASTRUCT)lParam;
    Row = (SS_COORD)lpDs->index[0].data;

    if (nRet = SS_AdjustPropArrayRow(lpSpread, &Row))
       return (nRet);

    SSGetMaxTextRowHeight(hWnd, Row, &dfVal);
    fNumber = (float)dfVal;
    _fmemcpy(&lpDs->data, &fNumber, sizeof(float));
    return (0);
}
#endif


/****************************************************************************************/
#ifndef SSP_NO_MAXTEXTCELLHEIGHT
LONG FP_MaxTextCellHeight(fSetProp,hCtl,hWnd,Msg,wParam,lParam,lpSpread)
BOOL            fSetProp;
HCTL            hCtl;
HWND            hWnd;
WORD            Msg;
WORD            wParam;
LONG            lParam;
LPVBSPREAD      lpSpread;
{
double          dfVal;

    VBSSVALCOLROW(lpSpread);
    SSGetMaxTextCellSize(hWnd, lpSpread->Col, lpSpread->Row, NULL,
                                 &dfVal);
    *(float FAR *)lParam = (float)dfVal;
    return (0);
}
#endif


/****************************************************************************************/
#ifndef SSP_NO_MAXTEXTCOLWIDTH
LONG FP_MaxTextColWidth(fSetProp,hCtl,hWnd,Msg,wParam,lParam,lpSpread)
BOOL            fSetProp;
HCTL            hCtl;
HWND            hWnd;
WORD            Msg;
WORD            wParam;
LONG            lParam;
LPVBSPREAD      lpSpread;
{
LPDATASTRUCT    lpDs;
float           fNumber;
SS_COORD        Col;
short           nRet;
double          dfVal;

    lpDs = (LPDATASTRUCT)lParam;
    Col = (SS_COORD)lpDs->index[0].data;

    if (nRet = SS_AdjustPropArrayCol(lpSpread, &Col))
       return (nRet);

    SSGetMaxTextColWidth(hWnd, Col, &dfVal);
    fNumber = (float)dfVal;
    _fmemcpy(&lpDs->data, &fNumber, sizeof(float));
    return (0);
}
#endif


/****************************************************************************************/
#ifndef SSP_NO_MAXTEXTCELLWIDTH
LONG FP_MaxTextCellWidth(fSetProp,hCtl,hWnd,Msg,wParam,lParam,lpSpread)
BOOL            fSetProp;
HCTL            hCtl;
HWND            hWnd;
WORD            Msg;
WORD            wParam;
LONG            lParam;
LPVBSPREAD      lpSpread;
{
double          dfVal;

    VBSSVALCOLROW(lpSpread);

    SSGetMaxTextCellSize(hWnd, lpSpread->Col, lpSpread->Row, &dfVal,
                                 NULL);
    *(float FAR *)lParam = (float)dfVal;
    return (0);
}
#endif


/****************************************************************************************/
#ifndef SSP_NO_DATACOLCNT
LONG FP_DataColCnt(fSetProp,hCtl,hWnd,Msg,wParam,lParam,lpSpread)
BOOL            fSetProp;
HCTL            hCtl;
HWND            hWnd;
WORD            Msg;
WORD            wParam;
LONG            lParam;
LPVBSPREAD      lpSpread;
{
SS_COORD        DataCnt;

    SSGetDataCnt(hWnd, &DataCnt, NULL);
    *(long FAR *)lParam = DataCnt;
    return (0);
}
#endif


/****************************************************************************************/
#ifndef SSP_NO_DATAROWCNT
LONG FP_DataRowCnt(fSetProp,hCtl,hWnd,Msg,wParam,lParam,lpSpread)
BOOL            fSetProp;
HCTL            hCtl;
HWND            hWnd;
WORD            Msg;
WORD            wParam;
LONG            lParam;
LPVBSPREAD      lpSpread;
{
SS_COORD        DataCnt;

    SSGetDataCnt(hWnd, NULL, &DataCnt);
    *(long FAR *)lParam = DataCnt;
    return (0);
}
#endif


/****************************************************************************************/
#ifndef SSP_NO_GRIDLINECOLOR
LONG FP_GridLineColor(fSetProp,hCtl,hWnd,Msg,wParam,lParam,lpSpread)
BOOL            fSetProp;
HCTL            hCtl;
HWND            hWnd;
WORD            Msg;
WORD            wParam;
LONG            lParam;
LPVBSPREAD      lpSpread;
{
    if (!fSetProp)
        *(LONG FAR*)lParam =  SSGetGridColor(hWnd);
    else
        SSSetGridColor(hWnd, lParam);

    return (0);
}
#endif


/****************************************************************************************/
#ifndef SSP_NO_GRIDSOLID
LONG FP_GridSolid(fSetProp,hCtl,hWnd,Msg,wParam,lParam,lpSpread)
BOOL            fSetProp;
HCTL            hCtl;
HWND            hWnd;
WORD            Msg;
WORD            wParam;
LONG            lParam;
LPVBSPREAD      lpSpread;
{
WORD            wStyle;

if (!fSetProp)
   {
   if (SSGetGridType(hWnd) & SS_GRID_SOLID)
      *(BOOL FAR *)lParam = -1;
   else
      *(BOOL FAR *)lParam = 0;
   }
else
   {
   wStyle = SSGetGridType(hWnd);

   if ((BOOL)lParam)
      wStyle |= SS_GRID_SOLID;
   else
      wStyle &= ~SS_GRID_SOLID;

   SSSetGridType(hWnd, wStyle);
   }

return (0);
}
#endif


/****************************************************************************************/
#ifndef SSP_NO_VSCROLLSPECIAL
LONG FP_VScrollSpecial(fSetProp,hCtl,hWnd,Msg,wParam,lParam,lpSpread)
BOOL            fSetProp;
HCTL            hCtl;
HWND            hWnd;
WORD            Msg;
WORD            wParam;
LONG            lParam;
LPVBSPREAD      lpSpread;
{
WORD            wOptions;

    if (!fSetProp)
        *(short FAR *)lParam = SSVScrollGetSpecial(hWnd, NULL);
    else
        {
        SSVScrollGetSpecial(hWnd, &wOptions);
        SSVScrollSetSpecial(hWnd, (BOOL)lParam, wOptions);
        } 

    return (0);
}
#endif


/****************************************************************************************/
#ifndef SSP_NO_USERRESIZE
LONG FP_UserResize(fSetProp,hCtl,hWnd,Msg,wParam,lParam,lpSpread)
BOOL            fSetProp;
HCTL            hCtl;
HWND            hWnd;
WORD            Msg;
WORD            wParam;
LONG            lParam;
LPVBSPREAD      lpSpread;
{
    if (!fSetProp)
        *(short FAR *)lParam = SSGetUserResize(hWnd);
    else
        SSSetUserResize(hWnd, (WORD)lParam);

    return (0);
}
#endif


/****************************************************************************************/
#ifndef SSP_NO_BUTTONDRAWMODE
LONG FP_ButtonDrawMode(fSetProp,hCtl,hWnd,Msg,wParam,lParam,lpSpread)
BOOL            fSetProp;
HCTL            hCtl;
HWND            hWnd;
WORD            Msg;
WORD            wParam;
LONG            lParam;
LPVBSPREAD      lpSpread;
{
    if (!fSetProp)
        *(short FAR *)lParam = SSGetButtonDrawMode(hWnd);
    else
        SSSetButtonDrawMode(hWnd, (WORD)lParam);

    return (0);
}
#endif


/****************************************************************************************/
#ifndef SSP_NO_SELECTBLOCKOPTIONS
LONG FP_SelectBlockOptions(fSetProp,hCtl,hWnd,Msg,wParam,lParam,lpSpread)
BOOL            fSetProp;
HCTL            hCtl;
HWND            hWnd;
WORD            Msg;
WORD            wParam;
LONG            lParam;
LPVBSPREAD      lpSpread;
{
    if (!fSetProp)
        *(short FAR *)lParam = SSGetSelBlockOptions(hWnd);
    else
        SSSetSelBlockOptions(hWnd, (WORD)lParam);

    return (0);
}
#endif


/****************************************************************************************/
#ifndef SSP_NO_TYPEFLOATDEFCUR
LONG FP_TypeFloatDefCur(fSetProp,hCtl,hWnd,Msg,wParam,lParam,lpSpread)
BOOL            fSetProp;
HCTL            hCtl;
HWND            hWnd;
WORD            Msg;
WORD            wParam;                
LONG            lParam;
LPVBSPREAD      lpSpread;
{
LPSPREADSHEET   lpSS;
SS_FLOATFORMAT  DefFloatFormat;

lpSS = SS_Lock(hWnd);

if (!fSetProp)
   {
   SS_GetDefFloatFormat(lpSS->lpBook, &DefFloatFormat);
   *(short FAR *)lParam = (unsigned char)DefFloatFormat.cCurrencySign;
   }
else
   {
   SSx_GetDefFloatFormat(lpSS, &DefFloatFormat);
   DefFloatFormat.cCurrencySign = (char)lParam;
   SS_SetDefFloatFormat(lpSS, &DefFloatFormat);
   }

SS_Unlock(hWnd);

return (0);
}
#endif


/****************************************************************************************/
#ifndef SSP_NO_TYPEFLOATDEFDECIMAL
LONG FP_TypeFloatDefDecimal(fSetProp,hCtl,hWnd,Msg,wParam,lParam,lpSpread)
BOOL            fSetProp;
HCTL            hCtl;
HWND            hWnd;
WORD            Msg;
WORD            wParam;                
LONG            lParam;
LPVBSPREAD      lpSpread;
{
LPSPREADSHEET   lpSS;
SS_FLOATFORMAT  DefFloatFormat;

lpSS = SS_Lock(hWnd);

if (!fSetProp)
   {
   SS_GetDefFloatFormat(lpSS->lpBook, &DefFloatFormat);
   *(short FAR *)lParam = (unsigned char)DefFloatFormat.cDecimalSign;
   }
else
   {
   SSx_GetDefFloatFormat(lpSS, &DefFloatFormat);
   DefFloatFormat.cDecimalSign = (char)lParam;
   SS_SetDefFloatFormat(lpSS, &DefFloatFormat);
   }

SS_Unlock(hWnd);

return (0);
}
#endif


/****************************************************************************************/
#ifndef SSP_NO_TYPEFLOATDEFSEP
LONG FP_TypeFloatDefSep(fSetProp,hCtl,hWnd,Msg,wParam,lParam,lpSpread)
BOOL            fSetProp;
HCTL            hCtl;
HWND            hWnd;
WORD            Msg;
WORD            wParam;                
LONG            lParam;
LPVBSPREAD      lpSpread;
{
LPSPREADSHEET   lpSS;
SS_FLOATFORMAT  DefFloatFormat;

lpSS = SS_Lock(hWnd);

if (!fSetProp)
   {
   SS_GetDefFloatFormat(lpSS->lpBook, &DefFloatFormat);
   *(short FAR *)lParam = (unsigned char)DefFloatFormat.cSeparator;
   }
else
   {
   SSx_GetDefFloatFormat(lpSS, &DefFloatFormat);
   DefFloatFormat.cSeparator = (char)lParam;
   SS_SetDefFloatFormat(lpSS, &DefFloatFormat);
   }

SS_Unlock(hWnd);

return (0);
}
#endif


/****************************************************************************************/
#ifndef SSP_NO_VIRTUALROWS
LONG FP_VirtualRows(fSetProp,hCtl,hWnd,Msg,wParam,lParam,lpSpread)
BOOL            fSetProp;
HCTL            hCtl;
HWND            hWnd;
WORD            Msg;
WORD            wParam;
LONG            lParam;
LPVBSPREAD      lpSpread;
{
SS_COORD        BufferSize;
SS_COORD        BufferOverlap;

if (!fSetProp)
   {
   SSVGetBufferSize(hWnd, &BufferSize, NULL);
   *(long FAR *)lParam = BufferSize;
   }
else
   {
   SSVGetBufferSize(hWnd, NULL, &BufferOverlap);
   SSVSetBufferSize(hWnd, (SS_COORD)lParam, BufferOverlap);
   }

return (0);
}
#endif


/****************************************************************************************/
#ifndef SSP_NO_VIRTUALOVERLAP
LONG FP_VirtualOverlap(fSetProp,hCtl,hWnd,Msg,wParam,lParam,lpSpread)
BOOL            fSetProp;
HCTL            hCtl;
HWND            hWnd;
WORD            Msg;
WORD            wParam;
LONG            lParam;
LPVBSPREAD      lpSpread;
{
SS_COORD        BufferSize;
SS_COORD        BufferOverlap;

if (!fSetProp)
   {
   SSVGetBufferSize(hWnd, NULL, &BufferOverlap);
   *(long FAR *)lParam = BufferOverlap;
   }
else
   {
   SSVGetBufferSize(hWnd, &BufferSize, NULL);
   SSVSetBufferSize(hWnd, BufferSize, (SS_COORD)lParam);
   }

return (0);
}
#endif


/****************************************************************************************/
#ifndef SSP_NO_VIRTUALMODE
LONG FP_VirtualMode(fSetProp,hCtl,hWnd,Msg,wParam,lParam,lpSpread)
BOOL            fSetProp;
HCTL            hCtl;
HWND            hWnd;
WORD            Msg;
WORD            wParam;
LONG            lParam;
LPVBSPREAD      lpSpread;
{
if (!fSetProp)
   *(BOOL FAR *)lParam = SSGetBool(hWnd, SSB_VIRTUALMODE);
else
   {
   SSSetBool(hWnd, SSB_VIRTUALMODE, (BOOL)lParam);
   lpSpread->lMaxRows = SSGetMaxRows(hWnd);
   }

return (0);
}
#endif


/****************************************************************************************/
#ifndef SSP_NO_VIRTUALMAXROWS
LONG FP_VirtualMaxRows(fSetProp,hCtl,hWnd,Msg,wParam,lParam,lpSpread)
BOOL            fSetProp;
HCTL            hCtl;
HWND            hWnd;
WORD            Msg;
WORD            wParam;
LONG            lParam;
LPVBSPREAD      lpSpread;
{
if (!fSetProp)
   *(long FAR *)lParam = SSVGetMax(hWnd);
else
   {
   SSVSetMax(hWnd, (SS_COORD)lParam);
   lpSpread->lMaxCols = SSGetMaxCols(hWnd);
   lpSpread->lMaxRows = SSGetMaxRows(hWnd);
   }

return (0);
}
#endif


/****************************************************************************************/
#ifndef SSP_NO_VIRTUALSCROLLBUFFER
LONG FP_VirtualScrollBuffer(fSetProp,hCtl,hWnd,Msg,wParam,lParam,lpSpread)
BOOL            fSetProp;
HCTL            hCtl;
HWND            hWnd;
WORD            Msg;
WORD            wParam;
LONG            lParam;
LPVBSPREAD      lpSpread;
{
long            lStyle;

if (!fSetProp)
   {
   if (SSVGetStyle(hWnd) & SSV_SCROLLBARSHOWBUFFER)
      *(BOOL FAR *)lParam = -1;
   else
      *(BOOL FAR *)lParam = 0;
   }
else
   {
   lStyle = SSVGetStyle(hWnd);

   if ((BOOL)lParam)
      lStyle |= SSV_SCROLLBARSHOWBUFFER;
   else
      lStyle &= ~SSV_SCROLLBARSHOWBUFFER;

   SSVSetStyle(hWnd, lStyle);
   }

return (0);
}
#endif


/****************************************************************************************/
#ifndef SSP_NO_GRIDSHOWHORIZ
LONG FP_GridShowHoriz(fSetProp,hCtl,hWnd,Msg,wParam,lParam,lpSpread)
BOOL            fSetProp;
HCTL            hCtl;
HWND            hWnd;
WORD            Msg;
WORD            wParam;
LONG            lParam;
LPVBSPREAD      lpSpread;
{
WORD            wStyle;

if (!fSetProp)
   {
   if (SSGetGridType(hWnd) & SS_GRID_HORIZONTAL)
      *(BOOL FAR *)lParam = -1;
   else
      *(BOOL FAR *)lParam = 0;
   }
else
   {
   wStyle = SSGetGridType(hWnd);

   if ((BOOL)lParam)
      wStyle |= SS_GRID_HORIZONTAL;
   else
      wStyle &= ~SS_GRID_HORIZONTAL;

   SSSetGridType(hWnd, wStyle);
   }

return (0);
}
#endif


/****************************************************************************************/
#ifndef SSP_NO_GRIDSHOWVERT
LONG FP_GridShowVert(fSetProp,hCtl,hWnd,Msg,wParam,lParam,lpSpread)
BOOL            fSetProp;
HCTL            hCtl;
HWND            hWnd;
WORD            Msg;
WORD            wParam;
LONG            lParam;
LPVBSPREAD      lpSpread;
{
WORD            wStyle;

if (!fSetProp)
   {
   if (SSGetGridType(hWnd) & SS_GRID_VERTICAL)
      *(BOOL FAR *)lParam = -1;
   else
      *(BOOL FAR *)lParam = 0;
   }
else
   {
   wStyle = SSGetGridType(hWnd);

   if ((BOOL)lParam)
      wStyle |= SS_GRID_VERTICAL;
   else
      wStyle &= ~SS_GRID_VERTICAL;

   SSSetGridType(hWnd, wStyle);
   }

return (0);
}
#endif


/****************************************************************************************/
#ifndef SSP_NO_STARTINGCOLNUMBER
LONG FP_StartingColNumber(fSetProp,hCtl,hWnd,Msg,wParam,lParam,lpSpread)
BOOL            fSetProp;
HCTL            hCtl;
HWND            hWnd;
WORD            Msg;
WORD            wParam;
LONG            lParam;
LPVBSPREAD      lpSpread;
{
SS_COORD        StartingColNumber;
SS_COORD        StartingRowNumber;

SSGetStartingNumbers(hWnd, &StartingColNumber, &StartingRowNumber);

if (!fSetProp)
   *(long FAR *)lParam = StartingColNumber;
else
   SSSetStartingNumbers(hWnd, (SS_COORD)lParam, StartingRowNumber);

return (0);
}
#endif


/****************************************************************************************/
#ifndef SSP_NO_STARTINGROWNUMBER
LONG FP_StartingRowNumber(fSetProp,hCtl,hWnd,Msg,wParam,lParam,lpSpread)
BOOL            fSetProp;
HCTL            hCtl;
HWND            hWnd;
WORD            Msg;
WORD            wParam;
LONG            lParam;
LPVBSPREAD      lpSpread;
{
SS_COORD        StartingColNumber;
SS_COORD        StartingRowNumber;

SSGetStartingNumbers(hWnd, &StartingColNumber, &StartingRowNumber);

if (!fSetProp)
   *(long FAR *)lParam = StartingRowNumber;
else
   SSSetStartingNumbers(hWnd, StartingColNumber, (SS_COORD)lParam);

return (0);
}
#endif


/****************************************************************************************/
#ifndef SSP_NO_COLHEADERDISPLAY
LONG FP_ColHeaderDisplay(fSetProp,hCtl,hWnd,Msg,wParam,lParam,lpSpread)
BOOL            fSetProp;
HCTL            hCtl;
HWND            hWnd;
WORD            Msg;
WORD            wParam;
LONG            lParam;
LPVBSPREAD      lpSpread;
{
if (!fSetProp)
   *(short FAR *)lParam = SSGetColHeaderDisplay(hWnd);
else
   SSSetColHeaderDisplay(hWnd, (WORD)lParam);

return (0);
}
#endif


/****************************************************************************************/
#ifndef SSP_NO_ROWHEADERDISPLAY
LONG FP_RowHeaderDisplay(fSetProp,hCtl,hWnd,Msg,wParam,lParam,lpSpread)
BOOL            fSetProp;
HCTL            hCtl;
HWND            hWnd;
WORD            Msg;
WORD            wParam;
LONG            lParam;
LPVBSPREAD      lpSpread;
{
if (!fSetProp)
   *(short FAR *)lParam = SSGetRowHeaderDisplay(hWnd);
else
   SSSetRowHeaderDisplay(hWnd, (WORD)lParam);

return (0);
}
#endif


/****************************************************************************************/
#ifndef SSP_NO_SORTKEYORDER
LONG FP_SortKeyOrder(fSetProp,hCtl,hWnd,Msg,wParam,lParam,lpSpread)
BOOL            fSetProp;
HCTL            hCtl;
HWND            hWnd;
WORD            Msg;
WORD            wParam;
LONG            lParam;
LPVBSPREAD      lpSpread;
{
return (FP_Sort(fSetProp,hCtl,hWnd,Msg,wParam,lParam,lpSpread,TRUE));
}
#endif


/****************************************************************************************/
#ifndef SSP_NO_SORTKEY
LONG FP_SortKey(fSetProp,hCtl,hWnd,Msg,wParam,lParam,lpSpread)
BOOL            fSetProp;
HCTL            hCtl;
HWND            hWnd;
WORD            Msg;
WORD            wParam;
LONG            lParam;
LPVBSPREAD      lpSpread;
{
return (FP_Sort(fSetProp,hCtl,hWnd,Msg,wParam,lParam,lpSpread,FALSE));
}
#endif


/****************************************************************************************/
LONG FP_Sort(fSetProp,hCtl,hWnd,Msg,wParam,lParam,lpSpread,fOrder)
BOOL            fSetProp;
HCTL            hCtl;
HWND            hWnd;
WORD            Msg;
WORD            wParam;
LONG            lParam;
LPVBSPREAD      lpSpread;
BOOL            fOrder;
{
LPDATASTRUCT    lpDs;
short           dIndex;

lpDs = (LPDATASTRUCT)lParam;
dIndex = (short)lpDs->index[0].data;

if (dIndex <= 0 || dIndex > SS_SORTKEY_MAX)
   return (ERR_INVALID_PROPVAL);

if (!fSetProp)
   {
   if (dIndex > lpSpread->nSortKeyCnt)
      {
      if (fOrder)
         lpDs->data = 0;
      else
         lpDs->data = -1;
      }

   else
      {
      LPSS_SORTKEY lpSortKeys = (LPSS_SORTKEY)GlobalLock(lpSpread->hSortKeys);

      if (fOrder)
         lpDs->data = (long)lpSortKeys[dIndex - 1].wOrder;
      else
         lpDs->data = (long)lpSortKeys[dIndex - 1].Reference;

      GlobalUnlock(lpSpread->hSortKeys);
      }
   }

else
   {
   LPSS_SORTKEY lpSortKeys;
   short     i;

   if (dIndex > lpSpread->nSortKeyCnt)
      {
      if (lpSpread->nSortKeyCnt == 0)
         lpSpread->hSortKeys = GlobalAlloc(GHND, sizeof(SS_SORTKEY) * dIndex);
      else
         lpSpread->hSortKeys = GlobalReAlloc(lpSpread->hSortKeys,
                                             sizeof(SS_SORTKEY) * dIndex, GHND);

      lpSortKeys = (LPSS_SORTKEY)GlobalLock(lpSpread->hSortKeys);

      for (i = lpSpread->nSortKeyCnt; i < dIndex; i++)
         {
         lpSortKeys[i].wOrder = 0;
         lpSortKeys[i].Reference = -1;
         }

      GlobalUnlock(lpSpread->hSortKeys);
      lpSpread->nSortKeyCnt = dIndex;
      }

   lpSortKeys = (LPSS_SORTKEY)GlobalLock(lpSpread->hSortKeys);

   if (fOrder)
      lpSortKeys[dIndex - 1].wOrder = (WORD)lpDs->data;
   else
      lpSortKeys[dIndex - 1].Reference = (SS_COORD)lpDs->data;

   if ((lpSortKeys[dIndex - 1].wOrder == 0 &&
       lpSortKeys[dIndex - 1].Reference == -1) &&
       dIndex == lpSpread->nSortKeyCnt)
      {
      for ( ; lpSpread->nSortKeyCnt > 0 &&
           (lpSortKeys[lpSpread->nSortKeyCnt - 1].wOrder == 0 &&
            lpSortKeys[lpSpread->nSortKeyCnt - 1].Reference == -1);
           lpSpread->nSortKeyCnt--)
         ;

      if (lpSpread->nSortKeyCnt == 0)
         {
         GlobalUnlock(lpSpread->hSortKeys);
         GlobalFree(lpSpread->hSortKeys);
         lpSpread->hSortKeys = 0;
         return (0);
         }
      }

   GlobalUnlock(lpSpread->hSortKeys);
   }

return (0);
}


/****************************************************************************************/
#ifndef SSP_NO_VSCROLLSPECIALTYPE
LONG FP_VScrollSpecialType(fSetProp,hCtl,hWnd,Msg,wParam,lParam,lpSpread)
BOOL            fSetProp;
HCTL            hCtl;
HWND            hWnd;
WORD            Msg;
WORD            wParam;
LONG            lParam;
LPVBSPREAD      lpSpread;
{
    if (!fSetProp)
        SSVScrollGetSpecial(hWnd, (WORD FAR *)lParam);
    else
        SSVScrollSetSpecial(hWnd, SSVScrollGetSpecial(hWnd, NULL),
                            (WORD)lParam);

    return (0);
}
#endif


/****************************************************************************************/
#ifndef SSP_NO_USERRESIZEROW
LONG FP_UserResizeRow(fSetProp,hCtl,hWnd,Msg,wParam,lParam,lpSpread)
BOOL            fSetProp;
HCTL            hCtl;
HWND            hWnd;
WORD            Msg;
WORD            wParam;
LONG            lParam;
LPVBSPREAD      lpSpread;
{
    if (!fSetProp)
        *(short FAR *)lParam = SSGetUserResizeRow(hWnd, lpSpread->Row);
    else
        SSSetUserResizeRow(hWnd, lpSpread->Row, (short)lParam);

    return (0);
}
#endif


/****************************************************************************************/
#ifndef SSP_NO_USERRESIZECOL
LONG FP_UserResizeCol(fSetProp,hCtl,hWnd,Msg,wParam,lParam,lpSpread)
BOOL            fSetProp;
HCTL            hCtl;
HWND            hWnd;
WORD            Msg;
WORD            wParam;
LONG            lParam;
LPVBSPREAD      lpSpread;
{
    if (!fSetProp)
        *(short FAR *)lParam = SSGetUserResizeCol(hWnd,lpSpread->Col);
    else
        SSSetUserResizeCol(hWnd,lpSpread->Col, (short)lParam);

    return (0);
}
#endif


/****************************************************************************************/
#ifndef SSP_NO_ROWHIDDEN
LONG FP_RowHidden(fSetProp,hCtl,hWnd,Msg,wParam,lParam,lpSpread)
BOOL            fSetProp;
HCTL            hCtl;
HWND            hWnd;
WORD            Msg;
WORD            wParam;
LONG            lParam;
LPVBSPREAD      lpSpread;
{
if (!fSetProp)
   *(short FAR *)lParam = (SSIsRowHidden(hWnd, lpSpread->Row) ? -1 : 0);
else
   SSShowRow(hWnd, lpSpread->Row, (short)lParam ? FALSE : TRUE);

return (0);
}
#endif


/****************************************************************************************/
#ifndef SSP_NO_COLHIDDEN
LONG FP_ColHidden(fSetProp,hCtl,hWnd,Msg,wParam,lParam,lpSpread)
BOOL            fSetProp;
HCTL            hCtl;
HWND            hWnd;
WORD            Msg;
WORD            wParam;
LONG            lParam;
LPVBSPREAD      lpSpread;
{
if (!fSetProp)
   *(short FAR *)lParam = (SSIsColHidden(hWnd, lpSpread->Col) ? -1 : 0);
else
   SSShowCol(hWnd, lpSpread->Col, (short)lParam ? FALSE : TRUE);

return (0);
}
#endif


/****************************************************************************************/
#ifndef SSP_NO_CURSORSTYLE
LONG FP_CursorStyle(fSetProp,hCtl,hWnd,Msg,wParam,lParam,lpSpread)
BOOL            fSetProp;
HCTL            hCtl;
HWND            hWnd;
WORD            Msg;
WORD            wParam;                
LONG            lParam;
LPVBSPREAD      lpSpread;
{
LPSPREADSHEET   lpSS;
HCURSOR         hCursor;
WORD            wCursorType;
WORD            wCursorStyle;

switch (lpSpread->dCursorType)
   {
   case 0:                             // Default
      wCursorType = SS_CURSORTYPE_DEFAULT;
      break;
   case 1:                             // ColResize
      wCursorType = SS_CURSORTYPE_COLRESIZE;
      break;
   case 2:                             // RowResize
      wCursorType = SS_CURSORTYPE_ROWRESIZE;
      break;
   case 3:                             // Button
      wCursorType = SS_CURSORTYPE_BUTTON;
      break;
   case 4:                             // GrayArea
      wCursorType = SS_CURSORTYPE_GRAYAREA;
      break;
   case 5:                             // LockedCell
      wCursorType = SS_CURSORTYPE_LOCKEDCELL;
      break;
   case 6:                             // ColHeader
      wCursorType = SS_CURSORTYPE_COLHEADER;
      break;
   case 7:                             // RowHeader
      wCursorType = SS_CURSORTYPE_ROWHEADER;
      break;
   case 8:                             // DragDropArea
      wCursorType = SS_CURSORTYPE_DRAGDROPAREA;
      break;
   case 9:                             // DragDrop
      wCursorType = SS_CURSORTYPE_DRAGDROP;
      break;
   }

if ( !fSetProp)
   {
   lpSS = SS_Lock(hWnd);

   hCursor = SSGetCursor(hWnd, wCursorType);
   lpSpread->hCursorIcon = 0;

   switch (hCursor)
      {
      case SS_CURSOR_DEFAULT:
         wCursorStyle = 1;
         lpSpread->hCursorIcon = lpSS->hCursorPointer;
         break;

      case SS_CURSOR_ARROW:
         wCursorStyle = 2;
         lpSpread->hCursorIcon = LoadCursor(NULL, IDC_ARROW);
         break;

      case SS_CURSOR_DEFCOLRESIZE:
         wCursorStyle = 3;
         lpSpread->hCursorIcon = lpSS->hCursorResizeV;
         break;

      case SS_CURSOR_DEFROWRESIZE:
         wCursorStyle = 4;
         lpSpread->hCursorIcon = lpSS->hCursorResizeH;
         break;

      default:
         wCursorStyle = 0;
         lpSpread->hCursorIcon = hCursor;
         break;
      }

   *(short FAR *)lParam = (short)wCursorStyle;
   SS_Unlock(hWnd);
   }

else
   {
   wCursorStyle = (WORD)lParam;

   switch (wCursorStyle)
      {
      case 1:
         hCursor = SS_CURSOR_DEFAULT;
         break;

      case 2:
         hCursor = SS_CURSOR_ARROW;
         break;

      case 3:
         hCursor = SS_CURSOR_DEFCOLRESIZE;
         break;

      case 4:
         hCursor = SS_CURSOR_DEFROWRESIZE;
         break;

      default:
         hCursor = lpSpread->hCursorIcon;
      }

   SSSetCursor(hWnd, wCursorType, hCursor);
   }

return (0);
}
#endif


/****************************************************************************************/
#ifndef SSP_NO_CURSORICON
LONG FP_CursorIcon(fSetProp,hCtl,hWnd,Msg,wParam,lParam,lpSpread)
BOOL            fSetProp;
HCTL            hCtl;
HWND            hWnd;
WORD            Msg;
WORD            wParam;                
LONG            lParam;
LPVBSPREAD      lpSpread;
{
PIC             Pic;

if (!fSetProp)
   {
   if (lpSpread->hCursorIcon)
      {
      PIC    Pic;
      HPIC   hPic;

      _fmemset(&Pic, '\0', sizeof(PIC));

      Pic.picType = PICTYPE_ICON;
      Pic.picData.icon.hicon = lpSpread->hCursorIcon;

      hPic = VBRefPic(VBAllocPic(&Pic));

      *(HPIC FAR *)lParam = hPic;
      }
   else
      *(HPIC FAR *)lParam = 0;
   }
else
   {
   VBGetPic((HPIC)lParam, &Pic);

   if (Pic.picType == PICTYPE_ICON)
      lpSpread->hCursorIcon = Pic.picData.icon.hicon;
   }

return (0);
}
#endif


/****************************************************************************************/
#ifndef SSP_NO_BORDERTYPE
LONG FP_BorderType(fSetProp,hCtl,hWnd,Msg,wParam,lParam,lpSpread)
BOOL            fSetProp;
HCTL            hCtl;
HWND            hWnd;
WORD            Msg;
WORD            wParam;                
LONG            lParam;
LPVBSPREAD      lpSpread;
{
    if (!fSetProp)
        {
//#ifndef SS_V30
        *(short FAR *)lParam = 0;
        SSGetBorder(hWnd, lpSpread->Col, lpSpread->Row, (short FAR *)lParam,
                    NULL, NULL);
//#else
//        *(short FAR *)lParam = lpSpread->wBorderType;
//#endif
        }

    return (0);
}
#endif


/****************************************************************************************/
#ifndef SSP_NO_BORDERSTYLE
LONG FP_BorderStyle(fSetProp,hCtl,hWnd,Msg,wParam,lParam,lpSpread)
BOOL            fSetProp;
HCTL            hCtl;
HWND            hWnd;
WORD            Msg;
WORD            wParam;                
LONG            lParam;
LPVBSPREAD      lpSpread;
{
LPSPREADSHEET  lpSS = SS_Lock(hWnd);

    if (!fSetProp)
        {
        *(short FAR *)lParam = 0;
#ifndef SS_V30
        SSGetBorder(hWnd, lpSpread->Col, lpSpread->Row, NULL,
                    (short FAR *)lParam, NULL);
#else
        if (lpSpread->wBorderType & SS_BORDERTYPE_LEFT)
           SSGetBorderEx(hWnd, lpSpread->Col, lpSpread->Row,
                         (short FAR *)lParam, NULL, NULL, NULL, NULL, NULL,
                         NULL, NULL);
        else if (lpSpread->wBorderType & SS_BORDERTYPE_TOP)
           SSGetBorderEx(hWnd, lpSpread->Col, lpSpread->Row,
                         NULL, NULL, (short FAR *)lParam, NULL, NULL, NULL,
                         NULL, NULL);
        else if (lpSpread->wBorderType & SS_BORDERTYPE_RIGHT)
           SSGetBorderEx(hWnd, lpSpread->Col, lpSpread->Row,
                         NULL, NULL, NULL, NULL, (short FAR *)lParam, NULL,
                         NULL, NULL);
        else if (lpSpread->wBorderType & SS_BORDERTYPE_BOTTOM)
           SSGetBorderEx(hWnd, lpSpread->Col, lpSpread->Row,
                         NULL, NULL, NULL, NULL, NULL, NULL,
                         (short FAR *)lParam, NULL);
        else
           SSGetBorder(hWnd, lpSpread->Col, lpSpread->Row,
                       NULL, (short FAR *)lParam, NULL);
#endif
        }

SS_Unlock(hWnd);
return (0);
}
#endif


/****************************************************************************************/
#ifndef SSP_NO_BORDERCOLOR
LONG FP_BorderColor(fSetProp,hCtl,hWnd,Msg,wParam,lParam,lpSpread)
BOOL            fSetProp;
HCTL            hCtl;
HWND            hWnd;
WORD            Msg;
WORD            wParam;                
LONG            lParam;
LPVBSPREAD      lpSpread;
{
    if (!fSetProp)
        {
        *(long FAR *)lParam = 0;
#ifndef SS_V30
        SSGetBorder(hWnd, lpSpread->Col, lpSpread->Row, NULL, NULL,
                    (long FAR *)lParam);
#else
        if (lpSpread->wBorderType & SS_BORDERTYPE_LEFT)
           SSGetBorderEx(hWnd, lpSpread->Col, lpSpread->Row,
                         NULL, (COLORREF FAR *)lParam, NULL, NULL, NULL, NULL,
                         NULL, NULL);
        else if (lpSpread->wBorderType & SS_BORDERTYPE_TOP)
           SSGetBorderEx(hWnd, lpSpread->Col, lpSpread->Row,
                         NULL, NULL, NULL, (COLORREF FAR *)lParam, NULL, NULL,
                         NULL, NULL);
        else if (lpSpread->wBorderType & SS_BORDERTYPE_RIGHT)
           SSGetBorderEx(hWnd, lpSpread->Col, lpSpread->Row,
                         NULL, NULL, NULL, NULL, NULL, (COLORREF FAR *)lParam,
                         NULL, NULL);
        else if (lpSpread->wBorderType & SS_BORDERTYPE_BOTTOM)
           SSGetBorderEx(hWnd, lpSpread->Col, lpSpread->Row,
                         NULL, NULL, NULL, NULL, NULL, NULL, NULL,
                         (COLORREF FAR *)lParam);
#endif
        }

    return (0);
}
#endif


/****************************************************************************************/
#ifndef SSP_NO_MULTISELINDEX
LONG FP_MultiSelIndex(fSetProp,hCtl,hWnd,Msg,wParam,lParam,lpSpread)
BOOL            fSetProp;
HCTL            hCtl;
HWND            hWnd;
WORD            Msg;
WORD            wParam;                
LONG            lParam;
LPVBSPREAD      lpSpread;
{
LPSS_SELBLOCK   lpMultiSelBlocks;

    if (!fSetProp)
       *(long FAR *)lParam = (long)lpSpread->dMultiSelBlockIndex;

    else
        {
        lpSpread->dMultiSelBlockIndex = (short)lParam;

        if (lParam < lpSpread->dMultiSelBlockCnt)
           {
           lpMultiSelBlocks = (LPSS_SELBLOCK)GlobalLock(
                                             lpSpread->hMultiSelBlocks);

           lpSpread->Col = lpMultiSelBlocks[(short)lParam].UL.Col;
           lpSpread->Row = lpMultiSelBlocks[(short)lParam].UL.Row;
           lpSpread->Col2 = lpMultiSelBlocks[(short)lParam].LR.Col;
           lpSpread->Row2 = lpMultiSelBlocks[(short)lParam].LR.Row;

           GlobalUnlock(lpSpread->hMultiSelBlocks);
           }
        }

    return (0);
}
#endif


/****************************************************************************************/
#ifndef SSP_NO_MULTISELCOUNT
LONG FP_MultiSelCount(fSetProp,hCtl,hWnd,Msg,wParam,lParam,lpSpread)
BOOL            fSetProp;
HCTL            hCtl;
HWND            hWnd;
WORD            Msg;
WORD            wParam;                
LONG            lParam;
LPVBSPREAD      lpSpread;
{
    //No api
    if (!fSetProp)
       *(long FAR *)lParam = lpSpread->dMultiSelBlockCnt;

    return (0);
}
#endif


/****************************************************************************************/
#ifndef SSP_NO_COLPAGEBREAK
LONG FP_ColPageBreak(fSetProp,hCtl,hWnd,Msg,wParam,lParam,lpSpread)
BOOL            fSetProp;
HCTL            hCtl;
HWND            hWnd;
WORD            Msg;
WORD            wParam;
LONG            lParam;
LPVBSPREAD      lpSpread;
{
if (!fSetProp)
   *(BOOL FAR *)lParam = SSGetColPageBreak(hWnd, lpSpread->Col);
else
   SSSetColPageBreak(hWnd, lpSpread->Col, (BOOL)lParam);

return (0);
}
#endif


/****************************************************************************************/
#ifndef SSP_NO_ROWPAGEBREAK
LONG FP_RowPageBreak(fSetProp,hCtl,hWnd,Msg,wParam,lParam,lpSpread)
BOOL            fSetProp;
HCTL            hCtl;
HWND            hWnd;
WORD            Msg;
WORD            wParam;
LONG            lParam;
LPVBSPREAD      lpSpread;
{
if (!fSetProp)
   *(BOOL FAR *)lParam = SSGetRowPageBreak(hWnd, lpSpread->Row);
else
   SSSetRowPageBreak(hWnd, lpSpread->Row, (BOOL)lParam);

return (0);
}
#endif


/****************************************************************************************/
#ifndef SSP_NO_VIRTUALCURROWCOUNT
LONG FP_VirtualCurRowCount(fSetProp,hCtl,hWnd,Msg,wParam,lParam,lpSpread)
BOOL            fSetProp;
HCTL            hCtl;
HWND            hWnd;
WORD            Msg;
WORD            wParam;
LONG            lParam;
LPVBSPREAD      lpSpread;
{
if (!fSetProp)
   *(long FAR *)lParam = SSVGetPhysBufferSize(hWnd);

return (0);
}
#endif


/****************************************************************************************/
#ifndef SSP_NO_VIRTUALCURTOP
LONG FP_VirtualCurTop(fSetProp,hCtl,hWnd,Msg,wParam,lParam,lpSpread)
BOOL            fSetProp;
HCTL            hCtl;
HWND            hWnd;
WORD            Msg;
WORD            wParam;
LONG            lParam;
LPVBSPREAD      lpSpread;
{
if (!fSetProp)
   *(long FAR *)lParam = SSVGetPhysBufferTop(hWnd);

return (0);
}
#endif


/****************************************************************************************/
#ifndef SSP_NO_CHECKTEXT
LONG FP_CheckText(fSetProp,hCtl,hWnd,Msg,wParam,lParam,lpSpread)
BOOL            fSetProp;
HCTL            hCtl;
HWND            hWnd;
WORD            Msg;
WORD            wParam;
LONG            lParam;
LPVBSPREAD      lpSpread;
{
int             Ret = TRUE;
LPSTR           Data;

    if (!fSetProp)
        Ret = GetCellTypeProperties(hCtl, hWnd, lpSpread, wParam, lParam);

    else
       {
       if (lpSpread->hCheckText)
          GlobalFree(lpSpread->hCheckText);

       lpSpread->hCheckText = 0;

       if (lParam && lstrlen((LPSTR)lParam))
          if (lpSpread->hCheckText = GlobalAlloc(GMEM_MOVEABLE |
                                                 GMEM_ZEROINIT,
                                                 lstrlen((LPSTR)lParam) + 1))
             {
             Data = (LPSTR)GlobalLock(lpSpread->hCheckText);
             lstrcpy(Data, (LPSTR)lParam);
             GlobalUnlock(lpSpread->hCheckText);
             }

       Ret = SetCellTypeProperties(hWnd, lpSpread, wParam, lParam);
       }

    if (!Ret)
       return (ERR_INVALID_PROPVAL);

    return (0);
}
#endif


/****************************************************************************************/
#ifndef SSP_NO_CHECKPICTURE
LONG FP_CheckPicture(fSetProp,hCtl,hWnd,Msg,wParam,lParam,lpSpread)
BOOL            fSetProp;
HCTL            hCtl;
HWND            hWnd;
WORD            Msg;
WORD            wParam;
LONG            lParam;
LPVBSPREAD      lpSpread;
{
LPDATASTRUCT    lpDs;
short           dIndex;
int             Ret = TRUE;

    if (!fSetProp)
       {
       HPIC hPic;

       lpDs = (LPDATASTRUCT)lParam;
       dIndex = (short)lpDs->index[0].data;

       if (dIndex >= 0 && dIndex <= 5)
          {
          wParam = dIndex + IPROP_SPREAD_CHECKPICTUP;
          lParam = (long)(LPVOID)&hPic;

          Ret = GetCellTypeProperties(hCtl, hWnd, lpSpread, wParam, lParam);
          lpDs->data = (long)hPic;
          }
       }

    else
       {
       lpDs = (LPDATASTRUCT)lParam;
       dIndex = (short)lpDs->index[0].data;

       if (dIndex >= 0 && dIndex <= 5)
          {
          switch (dIndex)
             {
             case 0:
                FP_ProcessButtonPict(lpDs->data, lpSpread,
                                     &lpSpread->hCheckPictUp,
                                     &lpSpread->dCheckPictUpType);
                break;

             case 1:
                FP_ProcessButtonPict(lpDs->data, lpSpread,
                                     &lpSpread->hCheckPictDown,
                                     &lpSpread->dCheckPictDownType);
                break;

             case 2:
                FP_ProcessButtonPict(lpDs->data, lpSpread,
                                     &lpSpread->hCheckPictFocusUp,
                                     &lpSpread->dCheckPictFocusUpType);
                break;

             case 3:
                FP_ProcessButtonPict(lpDs->data, lpSpread,
                                     &lpSpread->hCheckPictFocusDown,
                                     &lpSpread->dCheckPictFocusDownType);
                break;

             case 4:
                FP_ProcessButtonPict(lpDs->data, lpSpread,
                                     &lpSpread->hCheckPictGray,
                                     &lpSpread->dCheckPictGrayType);
                break;

             case 5:
                FP_ProcessButtonPict(lpDs->data, lpSpread,
                                     &lpSpread->hCheckPictFocusGray,
                                     &lpSpread->dCheckPictFocusGrayType);
                break;
             }

          wParam = dIndex + IPROP_SPREAD_CHECKPICTUP;
          lParam = lpDs->data;

          Ret = SetCellTypeProperties(hWnd, lpSpread, wParam, lParam);
          }
       }

    if (!Ret)
       return (ERR_INVALID_PROPVAL);

    return (0);
}
#endif


/****************************************************************************************/
#ifndef SSP_NO_ABOUT
LONG FPSS_About(fSetProp,hCtl,hWnd,Msg,wParam,lParam,lpSpread)
BOOL            fSetProp;
HCTL            hCtl;
HWND            hWnd;
WORD            Msg;
WORD            wParam;
LONG            lParam;
LPVBSPREAD      lpSpread;
{
if (!fSetProp)
   {
   char     szAbout[36];

   LoadString(hDynamicInst, IDS_ABOUTTEXT + LANGUAGE_BASE, szAbout, sizeof(szAbout)-1);
   *(HSZ FAR *)lParam = VBCreateHsz(HIWORD(hCtl), szAbout);
   }
return (0);
}
#endif


/****************************************************************************************/
#ifndef SS_NOCT_COMBO
LONG FP_TypeComboBox(fSetProp,hCtl,hWnd,Msg,wParam,lParam,lpSpread)
BOOL            fSetProp;
HCTL            hCtl;
HWND            hWnd;
WORD            Msg;
WORD            wParam;
LONG            lParam;
LPVBSPREAD      lpSpread;
{
GLOBALHANDLE    hText;
LPSTR           lpText;
short           dTextLen;
short           Ret = FALSE;
SS_COORD        c, c1, c2;
SS_COORD        r, r1, r2;

if (!fSetProp)
   {
   switch (wParam)
      {
#ifdef IPROP_SPREAD_COMBOINDEX
      case IPROP_SPREAD_COMBOINDEX:
         *(short FAR *)lParam = lpSpread->dTypeComboIndex;
         Ret = TRUE;
         break;
#endif

#ifdef IPROP_SPREAD_COMBOSTRING
      case IPROP_SPREAD_COMBOSTRING:
         dTextLen = (short)SSComboBoxSendMessage(hWnd, lpSpread->Col,
                                                 lpSpread->Row,
                                                 SS_CBM_GETLBTEXTLEN,
                                                 lpSpread->dTypeComboIndex,
                                                 0L);

         if (dTextLen)
            {
            if (hText = GlobalAlloc(GMEM_MOVEABLE | GMEM_ZEROINIT,
                                    dTextLen + 1))
               {
               lpText = GlobalLock(hText);
               SSComboBoxSendMessage(hWnd, lpSpread->Col, lpSpread->Row,
                                     SS_CBM_GETLBTEXT,
                                     lpSpread->dTypeComboIndex, (long)lpText);

               *(HSZ FAR *)lParam = VBCreateHsz(HIWORD(hCtl), lpText);
               GlobalUnlock(hText);
               GlobalFree(hText);
               Ret = TRUE;
               }
            }

         break;
#endif

#ifdef IPROP_SPREAD_COMBOCOUNT
      case IPROP_SPREAD_COMBOCOUNT:
         *(short FAR *)lParam = (short)SSComboBoxSendMessage(hWnd,
                                                             lpSpread->Col,
                                                             lpSpread->Row,
                                                             SS_CBM_GETCOUNT,
                                                             0, 0L);
         Ret = TRUE;
         break;
#endif

#ifdef IPROP_SPREAD_COMBOCURSEL
      case IPROP_SPREAD_COMBOCURSEL:
         *(short FAR *)lParam = (short)SSComboBoxSendMessage(hWnd,
                                                             lpSpread->Col,
                                                             lpSpread->Row,
                                                             SS_CBM_GETCURSEL,
                                                             0, 0L);
         Ret = TRUE;
         break;
#endif

      default:
         break;
      }
   }

else
   {
   switch (wParam)
      {
#ifdef IPROP_SPREAD_COMBOINDEX
      case IPROP_SPREAD_COMBOINDEX:
         lpSpread->dTypeComboIndex = (short)lParam;
         Ret = TRUE;
         break;
#endif

#ifdef IPROP_SPREAD_COMBOSTRING
      case IPROP_SPREAD_COMBOSTRING:
         if (lpSpread->BlockMode)
            {
            c1 = lpSpread->Col;
            c2 = lpSpread->Col2;
            r1 = lpSpread->Row;
            r2 = lpSpread->Row2;
            }
         else
            {
            c1 = c2 = lpSpread->Col;
            r1 = r2 = lpSpread->Row;
            }
         if (SS_ALLCOLS == c1 || SS_ALLCOLS == c2)
            c1 = c2 = SS_ALLCOLS;
         if (SS_ALLROWS == r1 || SS_ALLROWS == r2)
            r1 = r2 = SS_ALLROWS;
         for (c = c1; c <= c2; c++)
            {
            for (r = r1; r <= r2; r++)
               {
               SSComboBoxSendMessage(hWnd, c, r, SS_CBM_INSERTSTRING,
                                     lpSpread->dTypeComboIndex,
                                     (long)lParam);
               }
            }
         Ret = TRUE;
         break;
#endif

#ifdef IPROP_SPREAD_COMBOCURSEL
      case IPROP_SPREAD_COMBOCURSEL:
         SSComboBoxSendMessage(hWnd, lpSpread->Col, lpSpread->Row,
                               SS_CBM_SETCURSEL, (WORD)lParam, 0L);
         Ret = TRUE;
         break;
#endif

      default:
         break;
      }
   }

if (!Ret)
   return (ERR_INVALID_PROPVAL);

return (0);
}
#endif


/****************************************************************************************/
#ifdef SS_BOUNDCONTROL
LONG FP_Bound(fSetProp,hCtl,hWnd,Msg,wParam,lParam,lpSpread)
BOOL            fSetProp;
HCTL            hCtl;
HWND            hWnd;
WORD            Msg;
WORD            wParam;
LONG            lParam;
LPVBSPREAD      lpSpread;
{
if (!fSetProp)
   {
   switch (wParam)
      {
#ifdef IPROP_SPREAD_DAUTOHEADINGS
      case IPROP_SPREAD_DAUTOHEADINGS:
         *(short FAR *)lParam = lpSpread->DataOpts.fHeadings;
         break;
#endif

#ifdef IPROP_SPREAD_DAUTOCELLTYPES
      case IPROP_SPREAD_DAUTOCELLTYPES:
         *(short FAR *)lParam = lpSpread->DataOpts.fCellTypes;
         break;
#endif

#ifdef IPROP_SPREAD_DAUTOFILL
      case IPROP_SPREAD_DAUTOFILL:
         *(short FAR *)lParam = lpSpread->DataOpts.fAutoFill;
         break;
#endif

#ifdef IPROP_SPREAD_DAUTOSIZECOLS
      case IPROP_SPREAD_DAUTOSIZECOLS:
         *(short FAR *)lParam = lpSpread->DataOpts.fSizeCols;
         break;
#endif

#ifdef IPROP_SPREAD_DINFORMACTIVEROW
      case IPROP_SPREAD_DINFORMACTIVEROW:
         *(short FAR *)lParam = lpSpread->DataOpts.fAutoDataInform;
         break;
#endif

#ifdef IPROP_SPREAD_DAUTOSAVE
      case IPROP_SPREAD_DAUTOSAVE:
         *(short FAR *)lParam = lpSpread->DataOpts.fAutoSave;
         break;
#endif

      default:
         break;
      }
   }
else
   {
   switch (wParam)
      {
#ifdef IPROP_SPREAD_DAUTOHEADINGS
      case IPROP_SPREAD_DAUTOHEADINGS:
         lpSpread->DataOpts.fHeadings = (unsigned)(short)lParam;
         break;
#endif

#ifdef IPROP_SPREAD_DAUTOCELLTYPES
      case IPROP_SPREAD_DAUTOCELLTYPES:
         lpSpread->DataOpts.fCellTypes = (unsigned)(short)lParam;
         break;
#endif

#ifdef IPROP_SPREAD_DAUTOFILL
      case IPROP_SPREAD_DAUTOFILL:
         lpSpread->DataOpts.fAutoFill = (unsigned)(short)lParam;
         break;
#endif

#ifdef IPROP_SPREAD_DAUTOSIZECOLS
      case IPROP_SPREAD_DAUTOSIZECOLS:
         lpSpread->DataOpts.fSizeCols = (unsigned)(short)lParam;
         break;
#endif

#ifdef IPROP_SPREAD_DINFORMACTIVEROW
      case IPROP_SPREAD_DINFORMACTIVEROW:
         lpSpread->DataOpts.fAutoDataInform = (unsigned)(short)lParam;
         break;
#endif

#ifdef IPROP_SPREAD_DAUTOSAVE
      case IPROP_SPREAD_DAUTOSAVE:
         lpSpread->DataOpts.fAutoSave = (unsigned)(short)lParam;
         break;
#endif

      default:
         break;
      }
   }

return (0);
}
#endif


/****************************************************************************************/
#ifndef SSP_NO_DATAFIELD
LONG FP_DataFieldName(BOOL fSetProp, HCTL hCtl, HWND hWnd, WORD Msg,
                      WORD wParam, LONG lParam, LPVBSPREAD lpSpread)
{
LPSPREADSHEET  lpSS;
TBGLOBALHANDLE hFieldName;
SS_COORD       Col;
LPSTR          lpFieldName;
int            Ret = TRUE;

if (VBGetMode() == MODE_DESIGN || !lpSpread->fVBMLoaded)
   Col = 1;
else
   Col = lpSpread->Col;

lpSS = SS_Lock(hWnd);
SS_AdjustCellCoords(lpSS, &Col, NULL);

if (!fSetProp)
   {
   VBSSVALCOLROW(lpSpread);

   if (hFieldName = SS_GetColFieldName(lpSS, Col))
      {
      lpFieldName = (LPSTR)tbGlobalLock(hFieldName);
      *(HSZ FAR *)lParam = VBCreateHsz(HIWORD(hCtl), lpFieldName);
      tbGlobalUnlock(hFieldName);
      }
   else
      *(HSZ FAR *)lParam = VBSSBoundGetFieldName(lpSS, (LONG)hCtl, hWnd, Col);
   }

else
   {
   Ret = SS_SetColFieldName(lpSS, Col, (LPSTR)lParam);

   if (!Ret)
      return (ERR_INVALID_PROPVAL);
   }

SS_Unlock(hWnd);
return (0);
}
#endif


/****************************************************************************************/
#ifndef SSP_NO_DATAFILLEVENT
LONG FP_DataFillEvent(fSetProp,hCtl,hWnd,Msg,wParam,lParam,lpSpread)
BOOL          fSetProp;
HCTL          hCtl;
HWND          hWnd;
WORD          Msg;
WORD          wParam;
LONG          lParam;
LPVBSPREAD    lpSpread;
{
LPSPREADSHEET lpSS;
LPSS_COL      lpCol;

lpSS = SS_Lock(hWnd);

if (!fSetProp)
   {
   if (lpSpread->Col == -1)
      *(short FAR *)lParam = (lpSS->fDataFillEvent ? -1 : 0);
   else if (lpCol = SS_LockColItem(lpSS, lpSpread->Col))
      {
      if (lpCol->fDataFillEvent == SS_DATAFILLEVENT_YES)
         *(short FAR *)lParam = TRUE;
      else if (lpCol->fDataFillEvent == SS_DATAFILLEVENT_NO)
         *(short FAR *)lParam = FALSE;
      else
         *(short FAR *)lParam = lpSS->fDataFillEvent;

      SS_UnlockColItem(lpSS, lpSpread->Col);
      }
   else
      *(short FAR *)lParam = (lpSS->fDataFillEvent ? -1 : 0);
   }
else
   {
   if (lpSpread->Col == -1)
      {
      SS_COORD c;

      lpSS->fDataFillEvent = (BOOL)lParam;

      // RFW - 4/29/99 - SPR_DAO_001_001

      for( c = lpSS->Col.HeaderCnt; c < lpSS->Col.AllocCnt; c++ )
         if (lpCol = SS_AllocLockCol(lpSS, c))
         {
            lpCol->fDataFillEvent = SS_DATAFILLEVENT_DEF;
            SS_UnlockColItem(lpSS, c);
         }
      }
   else if (lpCol = SS_AllocLockCol(lpSS, lpSpread->Col))
      {
      lpCol->fDataFillEvent = (BOOL)lParam ? SS_DATAFILLEVENT_YES :
                                             SS_DATAFILLEVENT_NO;
      SS_UnlockColItem(lpSpread, lpSpread->Col);
      }
   }

SS_Unlock(hWnd);   
return (0);
}
#endif


/****************************************************************************************/
#if (!defined(SSP_NO_SELMODEINDEX) || !defined(SSP_NO_SELMODESELCOUNT) || !defined(SSP_NO_SELMODESELECTED))
LONG FP_TypeSelMode(fSetProp,hCtl,hWnd,Msg,wParam,lParam,lpSpread)
BOOL            fSetProp;
HCTL            hCtl;
HWND            hWnd;
WORD            Msg;
WORD            wParam;
LONG            lParam;
LPVBSPREAD      lpSpread;
{
long            lRet = -1;

if (!fSetProp)
   {
   switch (wParam)
      {
      case IPROP_SPREAD_SELMODEINDEX:
         lRet = SSSelModeSendMessage(hWnd, SS_SELMODE_GETCURSEL, 0L, 0L, 0L);
         break;

      case IPROP_SPREAD_SELMODESELCOUNT:
         lRet = SSSelModeSendMessage(hWnd, SS_SELMODE_GETSELCOUNT, 0L, 0L, 0L);
         break;

      case IPROP_SPREAD_SELMODESELECTED:
         lRet = SSSelModeSendMessage(hWnd, SS_SELMODE_GETSEL, lpSpread->Row,
                                     0L, 0L);
         break;
      }

   if (lRet != -1)
      *(long FAR *)lParam = lRet;
   }

else
   {
   switch (wParam)
      {
      case IPROP_SPREAD_SELMODEINDEX:
         lRet = SSSelModeSendMessage(hWnd, SS_SELMODE_SETCURSEL, lParam,
                                     0L, 0L);
         break;

      case IPROP_SPREAD_SELMODESELECTED:
         lRet = SSSelModeSendMessage(hWnd, SS_SELMODE_SETSEL,
                                     (WPARAM)(BOOL)lParam, lpSpread->Row, 0L);
         break;
      }
   }

if (lRet == -1)
   return (ERR_INVALID_PROPVAL);

return (0);
}
#endif


#ifdef SS_UTP
LONG FP_Utopia(fSetProp,hCtl,hWnd,Msg,wParam,lParam,lpSpread)
BOOL            fSetProp;
HCTL            hCtl;
HWND            hWnd;
WORD            Msg;
WORD            wParam;
LONG            lParam;
LPVBSPREAD      lpSpread;
{
if (!fSetProp)
   {
   *(BOOL FAR *)lParam = lpSpread->fUtopiaMode;
   return (0);
   }

vbSpreadSetUtopiaModeOn(hWnd, lpSpread, (BOOL)lParam);
return (0);
}
#endif


/****************************************************************************************/
#ifdef IPROP_SPREAD_DATASELECT
LONG FP_DataSelect(fSetProp,hCtl,hWnd,Msg,wParam,lParam,lpSpread)
BOOL            fSetProp;
HCTL            hCtl;
HWND            hWnd;
WORD            Msg;
WORD            wParam;
LONG            lParam;
LPVBSPREAD      lpSpread;
{
return (FP_ProcessText(fSetProp,hCtl,hWnd,Msg,wParam,lParam,lpSpread,
                       &lpSpread->DBInfo.hDataSelect));
}
#endif


short SS_AdjustPropArrayCol(LPVBSPREAD lpSpread, LPSS_COORD lpCol)
{
short nRet = FALSE;

if (*lpCol == 0)
   *lpCol = SS_HEADER;
else if (*lpCol == -2)
   *lpCol = lpSpread->Col;
else
   nRet = vbSSValCoords(lpSpread, lpCol, NULL);

return (nRet);
}


short SS_AdjustPropArrayRow(LPVBSPREAD lpSpread, LPSS_COORD lpRow)
{
short nRet = FALSE;

if (*lpRow == 0)
   *lpRow = SS_HEADER;
else if (*lpRow == -2)
   *lpRow = lpSpread->Row;
else
   nRet = vbSSValCoords(lpSpread, NULL, lpRow);

return (nRet);
}


/****************************************************************************************/
LONG FP_Bool(fSetProp,hCtl,hWnd,Msg,wParam,lParam,lpSpread)
BOOL            fSetProp;
HCTL            hCtl;
HWND            hWnd;
WORD            Msg;
WORD            wParam;
LONG            lParam;
LPVBSPREAD      lpSpread;
{
short           SSB_Bool;

switch (wParam)
   {
#ifdef IPROP_SPREAD_UTOPIA
   case IPROP_SPREAD_UTOPIA:
      SSB_Bool = SSB_ALLOWUSERRESIZE;
      break;
#endif

#ifdef IPROP_SPREAD_AUTOCALC
   case IPROP_SPREAD_AUTOCALC:
      SSB_Bool = SSB_AUTOCALC;
      break;
#endif

#ifdef IPROP_SPREAD_DISPLAYCOLHDRS
   case IPROP_SPREAD_DISPLAYCOLHDRS:
      SSB_Bool = SSB_SHOWCOLHEADERS;
      break;
#endif

#ifdef IPROP_SPREAD_DISPLAYROWHDRS
   case IPROP_SPREAD_DISPLAYROWHDRS:
      SSB_Bool = SSB_SHOWROWHEADERS;
      break;
#endif

#ifdef IPROP_SPREAD_GRIDLINES
   case IPROP_SPREAD_GRIDLINES:
      SSB_Bool = SSB_GRIDLINES;
      break;
#endif

#ifdef IPROP_SPREAD_PROTECT
   case IPROP_SPREAD_PROTECT:
      SSB_Bool = SSB_PROTECT;
      break;
#endif

#ifdef IPROP_SPREAD_REDRAW
   case IPROP_SPREAD_REDRAW:
      SSB_Bool = SSB_REDRAW;
      break;
#endif

#ifdef IPROP_SPREAD_RESTRICTCOLS
   case IPROP_SPREAD_RESTRICTCOLS:
      SSB_Bool = SSB_RESTRICTCOLS;
      break;
#endif

#ifdef IPROP_SPREAD_RESTRICTROWS
   case IPROP_SPREAD_RESTRICTROWS:
      SSB_Bool = SSB_RESTRICTROWS;
      break;
#endif

#ifdef IPROP_SPREAD_EDITMODEPERMANENT
   case IPROP_SPREAD_EDITMODEPERMANENT:
      SSB_Bool = SSB_EDITMODEPERMANENT;
      break;
#endif

#ifdef IPROP_SPREAD_AUTOSIZE
   case IPROP_SPREAD_AUTOSIZE:
      SSB_Bool = SSB_AUTOSIZE;
      break;
#endif

#ifdef IPROP_SPREAD_SCROLLBAREXTMODE
   case IPROP_SPREAD_SCROLLBAREXTMODE:
      SSB_Bool = SSB_SCROLLBAREXTMODE;
      break;
#endif

#ifdef IPROP_SPREAD_SCROLLBARSHOWMAX
   case IPROP_SPREAD_SCROLLBARSHOWMAX:
      SSB_Bool = SSB_SCROLLBARSHOWMAX;
      break;
#endif

#ifdef IPROP_SPREAD_SCROLLBARMAXALIGN
   case IPROP_SPREAD_SCROLLBARMAXALIGN:
      SSB_Bool = SSB_SCROLLBARMAXALIGN;
      break;
#endif

#ifdef IPROP_SPREAD_PROCESSTAB
   case IPROP_SPREAD_PROCESSTAB:
      SSB_Bool = SSB_PROCESSTAB;
      break;
#endif

#ifdef IPROP_SPREAD_ARROWSEXITEDITMOD
   case IPROP_SPREAD_ARROWSEXITEDITMOD:
      SSB_Bool = SSB_ARROWSEXITEDITMODE;
      break;
#endif

#ifdef IPROP_SPREAD_MOVEACTIVEONFOCUS
   case IPROP_SPREAD_MOVEACTIVEONFOCUS:
      SSB_Bool = SSB_MOVEACTIVEONFOCUS;
      break;
#endif

#ifdef IPROP_SPREAD_RETAINSELBLOCK
   case IPROP_SPREAD_RETAINSELBLOCK:
      SSB_Bool = SSB_RETAINSELBLOCK;
      break;
#endif

#ifdef IPROP_SPREAD_ALLOWUSERFORMULAS
   case IPROP_SPREAD_ALLOWUSERFORMULAS:
      SSB_Bool = SSB_ALLOWUSERFORMULAS;
      break;
#endif

#ifdef IPROP_SPREAD_NOBEEP
   case IPROP_SPREAD_NOBEEP:
      SSB_Bool = SSB_NOBEEP;
      break;
#endif

#ifdef IPROP_SPREAD_EDITMODEREPLACE
   case IPROP_SPREAD_EDITMODEREPLACE:
      SSB_Bool = SSB_EDITMODEREPLACE;
      break;
#endif

#ifdef IPROP_SPREAD_AUTOCLIPBOARD
   case IPROP_SPREAD_AUTOCLIPBOARD:
      SSB_Bool = SSB_AUTOCLIPBOARD;
      break;
#endif

#ifdef IPROP_SPREAD_ALLOWMULTIBLOCKS
   case IPROP_SPREAD_ALLOWMULTIBLOCKS:
      SSB_Bool = SSB_ALLOWMULTIBLOCKS;
      break;
#endif

#ifdef IPROP_SPREAD_ALLOWCELLOVERFLOW
   case IPROP_SPREAD_ALLOWCELLOVERFLOW:
      SSB_Bool = SSB_ALLOWCELLOVERFLOW;
      break;
#endif

#ifdef IPROP_SPREAD_ALLOWDRAGDROP
   case IPROP_SPREAD_ALLOWDRAGDROP:
      SSB_Bool = SSB_ALLOWDRAGDROP;
      break;
#endif

#ifdef IPROP_SPREAD_CHANGEMADE
   case IPROP_SPREAD_CHANGEMADE:
      SSB_Bool = SSB_CHANGEMADE;
      break;
#endif

#ifdef IPROP_SPREAD_NOBORDER
   case IPROP_SPREAD_NOBORDER:
      SSB_Bool = SSB_NOBORDER;
      break;
#endif

#ifndef SS_V30
#ifdef IPROP_SPREAD_ALLOWSELECTBLOCK
   case IPROP_SPREAD_ALLOWSELECTBLOCK:
      SSB_Bool = SSB_ALLOWUSERSELBLOCK;
      break;
#endif
#endif

   default:
      break;
   }

if (!fSetProp)
   *(BOOL FAR *)lParam = SSGetBool(hWnd, SSB_Bool);
else
   SSSetBool(hWnd, SSB_Bool, (BOOL)lParam);

return (0);
}


/****************************************************************************************/
LONG FP_ProcessText(fSetProp,hCtl,hWnd,Msg,wParam,lParam,lpSpread,lphText)
BOOL             fSetProp;
HCTL             hCtl;
HWND             hWnd;
WORD             Msg;
WORD             wParam;
LONG             lParam;
LPVBSPREAD       lpSpread;
LPTBGLOBALHANDLE lphText;
{
int              Ret = TRUE;
LPSTR            lpText;

if (!fSetProp)
   {
   if (*lphText)
      {
      lpText = (LPSTR)tbGlobalLock(*lphText);
      *(HSZ FAR *)lParam = VBCreateHsz(HIWORD(hCtl), lpText);
      tbGlobalUnlock(*lphText);
      }
   else
      *(HSZ FAR *)lParam = VBCreateHsz(HIWORD(hCtl), "");
   }

else
   {
   if (*lphText)
      tbGlobalFree(*lphText);

   *lphText = 0;

   if (lParam && lstrlen((LPSTR)lParam))
      if (*lphText = tbGlobalAlloc(GMEM_MOVEABLE | GMEM_ZEROINIT,
                                   lstrlen((LPSTR)lParam) + 1))
         {
         lpText = (LPSTR)tbGlobalLock(*lphText);
         lstrcpy(lpText, (LPSTR)lParam);
         tbGlobalUnlock(*lphText);
         }
      else
         Ret = FALSE;
   }

if (!Ret)
   return (ERR_INVALID_PROPVAL);

return (0);
}


#ifndef SSP_NO_BACKCOLORSTYLE
LONG FP_BackColorStyle(fSetProp,hCtl,hWnd,Msg,wParam,lParam,lpSpread)
BOOL            fSetProp;
HCTL            hCtl;
HWND            hWnd;
WORD            Msg;
WORD            wParam;
LONG            lParam;
LPVBSPREAD      lpSpread;
{
if (!fSetProp)
   *(short FAR *)lParam = (short)SSGetBackColorStyle(hWnd);
else
   SSSetBackColorStyle(hWnd, (WORD)lParam);

return (0);
}
#endif


#ifdef SS_V30
#ifndef SSP_NO_APPEARANCE
LONG FP_Appearance(fSetProp,hCtl,hWnd,Msg,wParam,lParam,lpSpread)
BOOL            fSetProp;
HCTL            hCtl;
HWND            hWnd;
WORD            Msg;
WORD            wParam;
LONG            lParam;
LPVBSPREAD      lpSpread;
{
if (!fSetProp)
   *(short FAR *)lParam = (short)SSGetAppearance(hWnd);
else
   SSSetAppearance(hWnd, (WORD)lParam);

return (0);
}
#endif


#ifndef SSP_NO_TEXTTIP
LONG FP_TextTip(fSetProp,hCtl,hWnd,Msg,wParam,lParam,lpSpread)
BOOL            fSetProp;
HCTL            hCtl;
HWND            hWnd;
WORD            Msg;
WORD            wParam;
LONG            lParam;
LPVBSPREAD      lpSpread;
{
FP_TT_INFO Info;

if (!fSetProp)
   {
   fpTextTipGetInfo(hWnd, &Info);
   *(short FAR *)lParam = Info.wStatus;
   }
else
   {
   WORD wItem = FP_TT_ITEM_STATUS;

   if (VBGetMode() == MODE_DESIGN)
      {
      Info.fActive = FALSE;
      wItem |= FP_TT_ITEM_ACTIVE;
      }

   Info.wStatus = (WORD)lParam;
   fpTextTipSetInfo(hWnd, wItem, &Info);
   }

return (0);
}
#endif


#ifndef SSP_NO_TEXTTIPDELAY
LONG FP_TextTipDelay(fSetProp,hCtl,hWnd,Msg,wParam,lParam,lpSpread)
BOOL            fSetProp;
HCTL            hCtl;
HWND            hWnd;
WORD            Msg;
WORD            wParam;
LONG            lParam;
LPVBSPREAD      lpSpread;
{
FP_TT_INFO Info;

if (!fSetProp)
   {
   fpTextTipGetInfo(hWnd, &Info);
   *(long FAR *)lParam = Info.lDelay;
   }
else
   {
   Info.lDelay = lParam;
   fpTextTipSetInfo(hWnd, FP_TT_ITEM_DELAY, &Info);
   }

return (0);
}
#endif


#ifdef IPROP_SPREAD_SCROLLBARTRACK
LONG FP_ScrollBarTrack(fSetProp,hCtl,hWnd,Msg,wParam,lParam,lpSpread)
BOOL            fSetProp;
HCTL            hCtl;
HWND            hWnd;
WORD            Msg;
WORD            wParam;
LONG            lParam;
LPVBSPREAD      lpSpread;
{
if (!fSetProp)
   {
   WORD wScrollBarTrack = 0;
   BOOL fHorz = SSGetBool(hWnd, SSB_HSCROLLBARTRACK);
   BOOL fVert = SSGetBool(hWnd, SSB_VSCROLLBARTRACK);

   if (fHorz && fVert)
      wScrollBarTrack = 3;
   else if (fHorz)
      wScrollBarTrack = 2;
   else if (fVert)
      wScrollBarTrack = 1;

    *(WORD FAR *)lParam = wScrollBarTrack;
   }

else
   {
   if ((short)lParam == 0)
      {
      SSSetBool(hWnd, SSB_VSCROLLBARTRACK, FALSE);
      SSSetBool(hWnd, SSB_HSCROLLBARTRACK, FALSE);
      }
   else if ((short)lParam == 1)
      {
      SSSetBool(hWnd, SSB_VSCROLLBARTRACK, TRUE);
      SSSetBool(hWnd, SSB_HSCROLLBARTRACK, FALSE);
      }
   else if ((short)lParam == 2)
      {
      SSSetBool(hWnd, SSB_VSCROLLBARTRACK, FALSE);
      SSSetBool(hWnd, SSB_HSCROLLBARTRACK, TRUE);
      }
   else if ((short)lParam == 3)
      {
      SSSetBool(hWnd, SSB_VSCROLLBARTRACK, TRUE);
      SSSetBool(hWnd, SSB_HSCROLLBARTRACK, TRUE);
      }
   }

return (0);
}
#endif


#ifdef IPROP_SPREAD_CLIPBOARDOPTIONS
LONG FP_ClipboardOptions(BOOL fSetProp, HCTL hCtl, HWND hWnd, WORD Msg,
                         WORD wParam, LONG lParam, LPVBSPREAD lpSpread)
{
if (!fSetProp)
   *(WORD FAR *)lParam = SSGetClipboardOptions(hWnd);

else
   SSSetClipboardOptions(hWnd, (WORD)lParam);

return (0);
}
#endif


#ifdef IPROP_SPREAD_SELBACKCOLOR
LONG FP_SelBackColor(BOOL fSetProp, HCTL hCtl, HWND hWnd, WORD Msg,
                     WORD wParam, LONG lParam, LPVBSPREAD lpSpread)
{
COLORREF clrBack;
COLORREF clrFore;

SSGetSelColor(hWnd, &clrBack, &clrFore);

if (!fSetProp)
   *(long FAR *)lParam = clrBack;

else
   SSSetSelColor(hWnd, (COLORREF)lParam, clrFore);

return (0);
}
#endif


#ifdef IPROP_SPREAD_SELFORECOLOR
LONG FP_SelForeColor(BOOL fSetProp, HCTL hCtl, HWND hWnd, WORD Msg,
                     WORD wParam, LONG lParam, LPVBSPREAD lpSpread)
{
COLORREF clrBack;
COLORREF clrFore;

SSGetSelColor(hWnd, &clrBack, &clrFore);

if (!fSetProp)
   *(long FAR *)lParam = clrFore;

else
   SSSetSelColor(hWnd, clrBack, (COLORREF)lParam);

return (0);
}
#endif
#endif // SS_V30


long DLLENTRY lSSStrLen(LPSTR s)
{
long len = 0;

if (s)
   while (*s++)
      len++;

return len;
}
