/*********************************************************
* SS_EMODE.C
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
#include <windowsx.h>
#include <tchar.h>
#include <stdio.h>
#include <stddef.h>
#include <stdlib.h>
#include <string.h>
#include <process.h>
#include <math.h>
#include <mbctype.h>
#include "spread.h"
#include "fphdc.h"
#include "ss_alloc.h"
#include "ss_calc.h"
#include "ss_cell.h"
#include "ss_data.h"
#include "ss_doc.h"
#include "ss_draw.h"
#include "ss_emode.h"
#include "ss_font.h"
#include "ss_formu.h"
#include "ss_main.h"
#include "ss_scrol.h"
#include "ss_span.h"
#include "ss_type.h"
#include "ss_user.h"
#include "ss_win.h"
#include "ss_w32s.h"
#include "..\classes\checkbox.h"
#include "..\classes\superbtn.h"
#include "..\edit\editpic\editpic.h"

#ifdef SS_USEAWARE
#include "utools.h"
#endif

#ifdef SS_UTP
#ifndef VBAPI_H
#include "vbapi.h"
#endif
#include "..\..\vbsrc\vbsprea5.h"
#endif

static BOOL SSx_CellEditModeOn(HWND hWnd, LPSPREADSHEET lpSS, UINT Msg, WPARAM wParam, LPARAM lParam,
                               SS_COORD ColEdit, SS_COORD RowEdit, WORD wCursorPos);
static void SS_TypeEditOn(HWND hWndCtrl, LPSPREADSHEET lpSS, LPSS_CELLTYPE CellType, TBGLOBALHANDLE hData,
                          RECT Rect, int x, int y, int cx, int cy, WORD wCursorPos, SS_COORD ColEdit, SS_COORD RowEdit, LPARAM lParam);
#if SS_V80
BOOL SS_TypeCustomOn(LPSPREADSHEET lpSS, HWND hWndCtrl, LPSS_CELLTYPE CellType, SS_COORD ColEdit, SS_COORD RowEdit, LPRECT Rect, int x, int y, int cx, int cy, UINT Msg, WPARAM wParam, LPARAM lParam);
extern WINENTRY SS_CustomProc(HWND hWnd, UINT Msg, WPARAM wParam, LPARAM lParam); // in ss_edit.c
#endif
static void SSx_TypeEditOn(HWND hWndCtrl, LPSPREADSHEET lpSS,
                           LPSS_CELLTYPE CellType, TBGLOBALHANDLE hData, BOOL fSetSel);
static BOOL SS_SetButtonData(HWND hWndCtrl, LPSPREADSHEET lpSS, LPSS_CELL lpCell,
                             LPSS_CELLTYPE lpCellType, SS_COORD ColEdit, SS_COORD RowEdit);
static BOOL SS_SetCheckBoxData(HWND hWndCtrl, LPSPREADSHEET lpSS, LPSS_CELL lpCell,
                               LPSS_CELLTYPE lpCellType, SS_COORD ColEdit, SS_COORD RowEdit);
void SS_CheckBoxSetPict(HWND hWnd, HWND hWndCtrl, TBGLOBALHANDLE hPictName,
                        short wPictureType);
void InvalidateCellIfNecessary(LPSPREADSHEET lpSS, SS_COORD ColEdit, SS_COORD RowEdit);
#ifdef SS_V40
BOOL InvalidateMergedCell(LPSPREADSHEET lpSS, SS_COORD ColEdit, SS_COORD RowEdit);
#endif // SS_V40


//#ifdef SPREAD_JPN
// SPRD010 Wei Feng 1997/9/5
extern int GetSetIMEMode(HWND hWnd, BOOL fGet, BOOL fStatus);

//- This macro was made for easy comparison on Character set. (Masanori Iwasa)
#define CHARSET_TURNON_IME(Edit) \
                (Edit.ChrSet == SS_CHRSET_ALLCHR_IME || \
                 Edit.ChrSet == SS_CHRSET_KNJONLY_IME)
// SPRD010
//#endif

#ifndef SS_V70
#define SS_BEM_CURSORPOS_DEFAULT   0
#define SS_BEM_CURSORPOS_SELECTALL 1
#define SS_BEM_CURSORPOS_BEGINNING 2
#define SS_BEM_CURSORPOS_END       3
#define SS_BEM_CURSORPOS_MOUSE     4
#define SS_BEM_CURSORPOS_DECIMAL   5
#endif // SS_V70

BOOL SS_CellEditModeOn(LPSPREADSHEET lpSS,
                       UINT Msg, WPARAM wParam, LPARAM lParam)
{
LPSS_CELL     lpCell;
LPSS_CELLTYPE CellType;
SS_CELLTYPE   CellTypeTemp;
SS_COORD      ColEdit, RowEdit;
WORD          wCursorPos = SS_BEM_CURSORPOS_DEFAULT;
BOOL          fIsCellSpan;
BOOL          fRet = FALSE;
HWND          hWnd = lpSS->lpBook->hWnd;
BOOL          fShowActiveCell = FALSE;

if (!SS_IsActiveSheet(lpSS))
	return (FALSE);

// RFW - 7/3/05 - 16372
if (lpSS->lpBook->hWnd != GetFocus())
	return(FALSE);

fIsCellSpan = SS_GetCellBeingEdited(lpSS, &ColEdit, &RowEdit);

/* RFW - 1/25/07 - 19760
if (lpSS->wOpMode != SS_OPMODE_READONLY &&
    lpSS->wOpMode != SS_OPMODE_SINGLESEL &&
    lpSS->wOpMode != SS_OPMODE_MULTISEL &&
    lpSS->wOpMode != SS_OPMODE_EXTSEL &&
    !lpSS->lpBook->EditModeOn && !lpSS->lpBook->EditModeTurningOn &&
    !lpSS->lpBook->EditModeTurningOff && !lpSS->lpBook->wMessageBeingSent &&
    !lpSS->lpBook->fSetEditModeSent)
*/
if (lpSS->wOpMode != SS_OPMODE_READONLY &&
    lpSS->wOpMode != SS_OPMODE_SINGLESEL &&
    lpSS->wOpMode != SS_OPMODE_MULTISEL &&
    lpSS->wOpMode != SS_OPMODE_EXTSEL &&
    !lpSS->lpBook->EditModeOn && !lpSS->lpBook->EditModeTurningOn &&
    !lpSS->lpBook->EditModeTurningOff && !lpSS->lpBook->wMessageBeingSent &&
    (!lpSS->lpBook->fSetEditModeSent || Msg == WM_LBUTTONDOWN))
   {
   lpCell = SS_LockCellItem(lpSS, ColEdit, RowEdit);

   if (SS_GetLock(lpSS, ColEdit, RowEdit, TRUE))
      {
      if (!SS_SendMsgCoords(lpSS, SSM_CELLLOCKED, GetDlgCtrlID(hWnd),
                            ColEdit, RowEdit))
         SS_Beep(lpSS->lpBook);

      return (FALSE);
      }

   if (lpSS->RestrictCols &&
       ColEdit > lpSS->Col.DataCnt &&
       lpSS->RestrictRows &&
       RowEdit > lpSS->Row.DataCnt)
      {
      if (!SS_SendMsgCoords(lpSS, SSM_COLROWRESTRICTED, GetDlgCtrlID(hWnd),
                            ColEdit, RowEdit))
         SS_Beep(lpSS->lpBook);

      return (FALSE);
      }

   else if (lpSS->RestrictCols &&
       ColEdit > lpSS->Col.DataCnt)
      {
		SS_COORD Col = ColEdit;
		SS_AdjustCellCoordsOut(lpSS, &Col, NULL);
      if (!SS_SendMsg(lpSS->lpBook, lpSS, SSM_COLRESTRICTED, GetDlgCtrlID(hWnd), Col))
         SS_Beep(lpSS->lpBook);

      return (FALSE);
      }

   else if (lpSS->RestrictRows &&
       RowEdit > lpSS->Row.DataCnt)
      {
		SS_COORD Row = RowEdit;
		SS_AdjustCellCoordsOut(lpSS, NULL, &Row);
      if (!SS_SendMsg(lpSS->lpBook, lpSS, SSM_ROWRESTRICTED, GetDlgCtrlID(hWnd), Row))
         SS_Beep(lpSS->lpBook);

      return (FALSE);
      }

   CellType = SS_RetrieveCellType(lpSS, &CellTypeTemp, lpCell,
                                  ColEdit, RowEdit);

   if (CellType->Type == SS_TYPE_STATICTEXT ||
       CellType->Type == SS_TYPE_OWNERDRAW ||
       CellType->Type == SS_TYPE_PICTURE)
      return (FALSE);

#ifdef SS_V70
   if (!lpSS->lpBook->fDontSendEditModeMsg)
		{
		BOOL  fCancel = FALSE;
		WORD  wUserAction;
		UINT uiActionMsg = Msg;

		if (uiActionMsg == WM_LBUTTONDBLCLK || uiActionMsg == WM_LBUTTONDOWN)
			wUserAction = SS_BEM_MOUSE;
		else if (uiActionMsg == WM_KEYDOWN || uiActionMsg == WM_SYSKEYDOWN ||
               uiActionMsg == WM_CHAR || uiActionMsg == WM_IME_CHAR)
			wUserAction = SS_BEM_KEYBOARD;
		else
			wUserAction = SS_BEM_CODE;

		if (lpSS->lpBook->fEditModeReplace)
			wCursorPos = SS_BEM_CURSORPOS_SELECTALL;

		SS_SendMsgBeforeEditMode(lpSS, ColEdit, RowEdit,
                               wUserAction, &wCursorPos, &fCancel);

		if (fCancel)
			return (FALSE);

		if (wCursorPos == SS_BEM_CURSORPOS_MOUSE && wUserAction != SS_BEM_MOUSE)
			wCursorPos = SS_BEM_CURSORPOS_DEFAULT;
		else if (wCursorPos == SS_BEM_CURSORPOS_DECIMAL && !SS_IsCellTypeNumeric(CellType))
			wCursorPos = SS_BEM_CURSORPOS_DEFAULT;
		}
#endif // SS_V70

#ifdef SS_V40
	if (fIsCellSpan)
		{
		SS_COORD    lNumCols, lNumRows;
		SS_SELBLOCK Block;

		lpSS->lpBook->Redraw = FALSE;

		/* RFW - 6/23/04 - 14233
		SS_GetCellSpan(lpSS, ColEdit, RowEdit, NULL, NULL, &lNumCols, &lNumRows);
		*/
		SS_SpanCalcBlock(lpSS, ColEdit, RowEdit, &Block);
		lNumRows = Block.LR.Row - Block.UL.Row + 1;
		lNumCols = Block.LR.Col - Block.UL.Col + 1;

		// RFW - 6/10/04 - 14341
		lNumRows = min(lNumRows, SS_GetBottomVisCell(lpSS, RowEdit) - RowEdit + 1);
		lNumCols = min(lNumCols, SS_GetRightVisCell(lpSS, ColEdit) - ColEdit + 1);

		SS_ShowCell(lpSS, ColEdit + lNumCols - 1, RowEdit + lNumRows - 1, SS_SHOW_NEAREST);
		SS_ShowCell(lpSS, ColEdit, RowEdit, SS_SHOW_NEAREST);
		lpSS->lpBook->Redraw = TRUE;
		UpdateWindow(lpSS->lpBook->hWnd);
		}
	else
#endif // SS_V40
      fShowActiveCell = TRUE;

	/* RFW - 5/12/05 - 16245
	   I moved this line below
	SS_ShowActiveCell(lpSS, SS_SHOW_NEAREST);
	*/

   lpSS->fFormulaMode = FALSE;
   lpSS->lpBook->EditModeTurningOn = TRUE;

   SS_HighlightCell(lpSS, FALSE);

   if( fShowActiveCell)
	   SS_ShowActiveCell(lpSS, SS_SHOW_NEAREST);

   if (!lpSS->lpBook->fDontSendEditModeMsg)      // RFW - 8/23/94
		{
      SS_SendMsgCoords(lpSS, SSM_EDITMODEON, GetDlgCtrlID(hWnd),
                       ColEdit, RowEdit);
		// RFW - 6/3/04 - 14524
		if (!lpSS->lpBook->EditModeTurningOn)
			return (FALSE);
		}

   SS_HighlightCell(lpSS, FALSE);

   if (SS_IsDestroyed(hWnd))
      return (FALSE);

   SS_ClearUndoBuffer(lpSS);

   if (lpSS->wOpMode == SS_OPMODE_ROWMODE && !lpSS->fRowModeEditing)
      {
      lpSS->fRowModeChangeMade = FALSE;
      lpSS->fRowModeEditing = TRUE;
      lpSS->DataRowCntOld = lpSS->Row.DataCnt;
      SS_SendMsgEnterRow(lpSS, SSM_ENTERROW, GetDlgCtrlID(hWnd), RowEdit,
                         RowEdit >= lpSS->Row.DataCnt ? 1 : 0);
#ifndef SS_NOBUTTONDRAWMODE
      if (lpSS->Col.Frozen)
         SSx_InvalidateButtons(lpSS, lpSS->Col.UL, lpSS->Row.UL,
                               1, RowEdit, lpSS->Col.Frozen,
                               RowEdit);

      SSx_InvalidateButtons(lpSS, lpSS->Col.UL,
                            lpSS->Row.UL, lpSS->Col.UL,
                            RowEdit, lpSS->Col.LR,
                            RowEdit);
#endif
      }

   if (lpCell)
      SS_UnlockCellItem(lpSS, ColEdit, RowEdit);

   fRet = SSx_CellEditModeOn(hWnd, lpSS, Msg, wParam, lParam, ColEdit, RowEdit, wCursorPos);

   lpSS->lpBook->EditModeTurningOn = FALSE;
   return (fRet);
   }

return (FALSE);
}


static BOOL SSx_CellEditModeOn(HWND hWnd, LPSPREADSHEET lpSS, UINT Msg, WPARAM wParam, LPARAM lParam,
                               SS_COORD ColEdit, SS_COORD RowEdit, WORD wCursorPos)
{
TBGLOBALHANDLE hData;
SUPERBTNCOLOR  BtnColor;
LPTSTR         Data;
HWND           hWndCtrl = 0;
RECT           Rect;
LPSS_CELL      lpCell;
LPSS_CELLTYPE  CellType;
SS_CELLTYPE    CellTypeTemp;
SS_FONT        FontTemp;
LPSS_FONT      Font;
DATEFORMAT     DateFormat;
LPTSTR         Mask;
LPTSTR         lpItems;
BOOL           fButtonState;
TCHAR          Buffer[60];
short          dLen;
int            x;
int            y;
int            cx;
int            cy;
short          dSel;
short          i;
#ifdef SS_V40
SS_SELBLOCK    Block;
SS_COORD       lSpanNumCols, lSpanNumRows;
#endif // SS_V40

//Modify By BOC 99.7.6(hyt)------------------------------
//for if maxcols or maxrows set to 0 not enter edit mode
if(lpSS->Col.Max == 0 || lpSS->Row.Max == 0)
	return FALSE;
//-------------------------------------------------------

lpCell = SS_LockCellItem(lpSS, ColEdit, RowEdit);

CellType = SS_RetrieveCellType(lpSS, &CellTypeTemp, lpCell,
                               ColEdit, RowEdit);

hWndCtrl = SS_RetrieveControlhWnd(lpSS, ColEdit, RowEdit);

#ifdef SS_V80
//if (CellType->Type == SS_TYPE_CUSTOM && !SS_CT_IsEditable(lpSS, ColEdit, RowEdit, CellType))
//   return (TRUE);
#endif

if (/*CellType->Type != SS_TYPE_CUSTOM &&*/ !hWndCtrl)
   return (TRUE);

Font = SS_RetrieveFont(lpSS, &FontTemp, lpCell, ColEdit, RowEdit);

SS_GetClientRect(lpSS->lpBook, &Rect);

#ifdef SS_V40
/* RFW - 6/23/04 - 14233
if (SS_GetCellSpan(lpSS, ColEdit, RowEdit, NULL, NULL, &lSpanNumCols, &lSpanNumRows))
*/
if (SS_SpanCalcBlock(lpSS, ColEdit, RowEdit, &Block))
	{
	lSpanNumRows = Block.LR.Row - Block.UL.Row + 1;
	lSpanNumCols = Block.LR.Col - Block.UL.Col + 1;

	SS_GetCellRangeCoord(lpSS, lpSS->Col.UL, lpSS->Row.UL, ColEdit, RowEdit,
                        ColEdit + lSpanNumCols - 1, RowEdit + lSpanNumRows - 1, &Rect, &x, &y, &cx, &cy);
	}
else
#endif // SS_V40
	SS_GetCellCoord(lpSS, lpSS->Col.UL, lpSS->Row.UL, ColEdit, RowEdit, &x, &y, &cx, &cy);

if (x + cx > Rect.right)
   cx = Rect.right - x;

if (y + cy > Rect.bottom)
   cy = Rect.bottom - y;

#ifndef SS_NOOVERFLOW
{
short dWidthPrev;
short dWidthNext;

if (lpSS->lpBook->fAllowCellOverflow &&
    SSx_ResetCellOverflow(lpSS, ColEdit, RowEdit,
                          &dWidthPrev, &dWidthNext))
   {
   if (dWidthPrev || dWidthNext || (lpCell && lpCell->Data.bOverflow))
      {
		/* RFW - 6/24/04 - 14374
		RECT  RectOverlap;
		RECT  RectTemp;
      int xTra = 0;

      if (lpSS->lpBook->bBackColorStyle == SS_BACKCOLORSTYLE_OVERGRID ||
          lpSS->lpBook->bBackColorStyle == SS_BACKCOLORSTYLE_OVERVERTGRIDONLY)
         xTra = 1;

      SS_GetScrollArea(lpSS, &RectTemp, SS_SCROLLAREA_CELLS);

      if (dWidthPrev)
         {
         SetRect(&RectOverlap, max(RectTemp.left, x - dWidthPrev), y,
                 min(RectTemp.right, x + 1), y + cy + xTra);
         SS_InvalidateRect(lpSS->lpBook, &RectOverlap, TRUE);
         }

      if (dWidthNext)
         {
         SetRect(&RectOverlap, x + cx - 1, y, min(RectTemp.right,
                 x + cx + dWidthNext + xTra), y + cy + xTra);
         SS_InvalidateRect(lpSS->lpBook, &RectOverlap, TRUE);
         }

      if (lpCell && lpCell->Data.bOverflow)
         {
         lpCell->Data.bOverflow = 0;
         SetRect(&RectOverlap, x, y + 1, x + cx + xTra, y + cy + xTra);
         SS_InvalidateRect(lpSS->lpBook, &RectOverlap, TRUE);
         }
		*/
      if (lpCell && lpCell->Data.bOverflow)
         lpCell->Data.bOverflow = 0;

		SS_InvalidateRow(lpSS, RowEdit);
		/***************************/

      SS_UpdateWindow(lpSS->lpBook);
      SS_HighlightCell(lpSS, FALSE);
      }
   }
}
#endif

if (lpCell)
   SS_UnlockCellItem(lpSS, ColEdit, RowEdit);

/********************************************************************
* If the cell is off the screen, move the edit field off the screen
********************************************************************/

if (lpSS->lpBook->fEditModePermanent && (lpSS->Row.LR || lpSS->Col.LR) &&
   ((ColEdit >= lpSS->Col.HeaderCnt +
    lpSS->Col.Frozen && ColEdit < lpSS->Col.UL) ||
    ColEdit > lpSS->Col.LR ||
    (RowEdit >= lpSS->Row.HeaderCnt +
    lpSS->Row.Frozen && RowEdit < lpSS->Row.UL) ||
    RowEdit > lpSS->Row.LR))
   x = -cx - 10;

lpSS->lpBook->CurVisCell.Row = RowEdit;
lpSS->lpBook->CurVisCell.Col = ColEdit;
lpSS->Row.EditAt = RowEdit;
lpSS->Col.EditAt = ColEdit;

/***********************************
* Set current font for entry field
***********************************/

SendMessage(hWndCtrl, WM_SETFONT, (WPARAM)Font->hFont, 1L);
#ifdef WIN32
SendMessage(hWndCtrl, EM_SETMARGINS, EC_LEFTMARGIN | EC_RIGHTMARGIN, 0);  // BJO 21Feb96 SCS2417
#endif

switch (CellType->Type)
   {
   case SS_TYPE_EDIT:
      hData = SS_GetData(lpSS, CellType, ColEdit, RowEdit, FALSE);

		if (wCursorPos == SS_BEM_CURSORPOS_DEFAULT)
			{
	      if (lpSS->lpBook->fEditModeReplace)
				wCursorPos = SS_BEM_CURSORPOS_SELECTALL;
			else
				wCursorPos = SS_BEM_CURSORPOS_END;
			}

      SS_TypeEditOn(hWndCtrl, lpSS, CellType, hData, Rect, x, y,
                    cx, cy, wCursorPos, ColEdit, RowEdit, lParam);
      break;

#ifndef SS_NOCT_BUTTON
   case SS_TYPE_BUTTON:
	   {
      GLOBALHANDLE hGlobal;
      LPSUPERBTN   lpSuperBtn;
      fButtonState = 0;

      if (hData = SS_GetData(lpSS, CellType, ColEdit,
                             RowEdit, FALSE))
         {
         Data = (LPTSTR)tbGlobalLock(hData);

         if (Data[0] == '1')
            fButtonState = 1;

         tbGlobalUnlock(hData);
         }

      /***********
      * Set Text
      ***********/

      if (CellType->Spec.Button.hText)
         {
         Data = (LPTSTR)GlobalLock(CellType->Spec.Button.hText);
         SetWindowText(hWndCtrl, Data);
         GlobalUnlock(CellType->Spec.Button.hText);
         }
      else
         SetWindowText(hWndCtrl, _TEXT(""));

#ifdef SS_V80
		lpSuperBtn = SuperBtnGetPtr(hWndCtrl, &hGlobal);
		lpSuperBtn->Enhanced = (lpSS->lpBook->wAppearanceStyle != 0 && lpSS->lpBook->wUseVisualStyles != 2) || (lpSS->lpBook->wAppearanceStyle == 0 && lpSS->lpBook->wUseVisualStyles == 1);
		GlobalUnlock(hGlobal);
#endif
	  /**************
      * Set Picture
      **************/

      if (CellType->Spec.Button.hPictName)
         {
         if (CellType->Spec.Button.nPictureType & SUPERBTN_PICT_HANDLE)
            SendMessage(hWndCtrl, SBM_SETPICT,
                        CellType->Spec.Button.nPictureType,
                        (LPARAM)CellType->Spec.Button.hPictName);
         else
            {
            Data = (LPTSTR)GlobalLock(CellType->Spec.Button.hPictName);
            /*
            SendMessage(hWndCtrl, SBM_SETPICT,
                        CellType->Spec.Button.nPictureType, (long)Data);
            */
            {

            lpSuperBtn = SuperBtnGetPtr(hWndCtrl, &hGlobal);
            SuperBtnSetPict(hWndCtrl, GetWindowInstance(hWnd),
                            &lpSuperBtn->Pict,
                            CellType->Spec.Button.nPictureType,
                            (LPARAM)Data);
            GlobalUnlock(hGlobal);
            }

            GlobalUnlock(CellType->Spec.Button.hPictName);
            }
         }
      else
         SendMessage(hWndCtrl, SBM_SETPICT, SUPERBTN_PICT_NONE, 0L);

      /*******************
      * Set Down Picture
      *******************/

      if (CellType->Spec.Button.hPictDownName)
         {
         if (CellType->Spec.Button.nPictureDownType & SUPERBTN_PICT_HANDLE)
            SendMessage(hWndCtrl, SBM_SETPICT_BTNDOWN,
                        CellType->Spec.Button.nPictureDownType,
                        (LPARAM)CellType->Spec.Button.hPictDownName);
         else
            {
            Data = (LPTSTR)GlobalLock(CellType->Spec.Button.hPictDownName);
            /*
            SendMessage(hWndCtrl, SBM_SETPICT_BTNDOWN,
                        CellType->Spec.Button.nPictureDownType, (long)Data);
            */
            {

            lpSuperBtn = SuperBtnGetPtr(hWndCtrl, &hGlobal);
            SuperBtnSetPict(hWndCtrl, GetWindowInstance(hWnd),
                            &lpSuperBtn->PictBtnDown,
                            CellType->Spec.Button.nPictureDownType,
                            (LPARAM)Data);
            GlobalUnlock(hGlobal);
            }

            GlobalUnlock(CellType->Spec.Button.hPictDownName);
            }
         }
      else
         SendMessage(hWndCtrl, SBM_SETPICT_BTNDOWN, SUPERBTN_PICT_NONE, 0L);

      _fmemcpy(&BtnColor, &CellType->Spec.Button.Color, sizeof(BtnColor));
      BtnColor.Color = SS_TranslateColor(BtnColor.Color);
      BtnColor.ColorBorder = SS_TranslateColor(BtnColor.ColorBorder);
      BtnColor.ColorShadow = SS_TranslateColor(BtnColor.ColorShadow);
      BtnColor.ColorHighlight = SS_TranslateColor(BtnColor.ColorHighlight);
      BtnColor.ColorText = SS_TranslateColor(BtnColor.ColorText);
      SendMessage(hWndCtrl, SBM_SETCOLOR, 0, (LPARAM)(LPVOID)&BtnColor);

      SendMessage(hWndCtrl, SBM_SETBUTTONTYPE,
                  CellType->Spec.Button.ButtonType, 0L);

      SendMessage(hWndCtrl, SBM_SETSHADOWSIZE,
                  (short)CellType->Spec.Button.ShadowSize, 0L);

      lpSS->lpBook->fIgnoreBNClicked = TRUE;
      if (CellType->Spec.Button.ButtonType == SUPERBTN_TYPE_2STATE)
         SendMessage(hWndCtrl, SBM_SETBUTTONSTATE, (short)fButtonState, 0L);
      lpSS->lpBook->fIgnoreBNClicked = FALSE;

      MoveWindow(hWndCtrl, Rect.left + x + 1, Rect.top + y + 1, cx - 1,
                 cy - 1, TRUE);

      lpSS->NoEraseBkgnd = TRUE;
      ShowWindow(hWndCtrl, SW_SHOWNORMAL);
      UpdateWindow(hWndCtrl);
      SetFocus(hWndCtrl);
      lpSS->NoEraseBkgnd = FALSE;
	  }
      break;
#endif

#ifndef SS_NOCT_CHECK
   case SS_TYPE_CHECKBOX:
	  {
      fButtonState = 0;

      if (hData = SS_GetData(lpSS, CellType, ColEdit,
                             RowEdit, FALSE))
         {
         Data = (LPTSTR)tbGlobalLock(hData);

         if ((CellType->Style & 0x0F) == BS_3STATE ||
             (CellType->Style & 0x0F) == BS_AUTO3STATE)
            {
            if (Data[0] == '1')
               fButtonState = 1;
            else if (Data[0] == '2')
               fButtonState = 2;
            }
         else
            {
            if (!Data[0] || (!Data[1] && Data[0] == '0'))
               fButtonState = 0;
            else
               fButtonState = 1;
            }

         tbGlobalUnlock(hData);
         }

      /***********
      * Set Text
      ***********/

      if (CellType->Spec.CheckBox.hText)
         {
         Data = (LPTSTR)GlobalLock(CellType->Spec.CheckBox.hText);
         SetWindowText(hWndCtrl, Data);
         GlobalUnlock(CellType->Spec.CheckBox.hText);
         }
      else
         SetWindowText(hWndCtrl, _TEXT(""));

#ifdef SS_V80
		{
		LPCHECKBOX lpCheckBox;
		GLOBALHANDLE hGlobal;

	   lpCheckBox = CheckBoxGetPtr(hWndCtrl, &hGlobal);
	   lpCheckBox->Enhanced = (lpSS->lpBook->wAppearanceStyle != 0 && lpSS->lpBook->wUseVisualStyles != 2) || (lpSS->lpBook->wAppearanceStyle == 0 && lpSS->lpBook->wUseVisualStyles == 1);

	   GlobalUnlock(hGlobal);
		}
#endif
	  /**************
      * Set Picture
      **************/

      SS_CheckBoxSetPict(hWnd, hWndCtrl, CellType->Spec.CheckBox.hPictUpName,
                         (short)(CellType->Spec.CheckBox.bPictUpType | BT_PICT_UP));
      SS_CheckBoxSetPict(hWnd, hWndCtrl, CellType->Spec.CheckBox.hPictDownName,
                         (short)(CellType->Spec.CheckBox.bPictDownType | BT_PICT_DOWN));
      SS_CheckBoxSetPict(hWnd, hWndCtrl,
                         CellType->Spec.CheckBox.hPictFocusUpName,
                         (short)(CellType->Spec.CheckBox.bPictFocusUpType |
                                 BT_PICT_FOCUSUP));
      SS_CheckBoxSetPict(hWnd, hWndCtrl,
                         CellType->Spec.CheckBox.hPictFocusDownName,
                         (short)(CellType->Spec.CheckBox.bPictFocusDownType |
                                 BT_PICT_FOCUSDOWN));
      SS_CheckBoxSetPict(hWnd, hWndCtrl,
                         CellType->Spec.CheckBox.hPictGrayName,
                         (short)(CellType->Spec.CheckBox.bPictGrayType |
                                 BT_PICT_GRAY));
      SS_CheckBoxSetPict(hWnd, hWndCtrl,
                         CellType->Spec.CheckBox.hPictFocusGrayName,
                         (short)(CellType->Spec.CheckBox.bPictFocusGrayType |
                                 BT_PICT_FOCUSGRAY));

      SendMessage(hWndCtrl, BM_SETSTYLE, 0, (LPARAM)CellType->Style);
      SendMessage(hWndCtrl, BM_SETCHECK, (WPARAM)fButtonState, 0L);

      MoveWindow(hWndCtrl, Rect.left + x + 1, Rect.top + y + 1, cx - 1,
                 cy - 1, TRUE);

      lpSS->NoEraseBkgnd = TRUE;
      ShowWindow(hWndCtrl, SW_SHOWNORMAL);
      UpdateWindow(hWndCtrl);
      SetFocus(hWndCtrl);
      lpSS->NoEraseBkgnd = FALSE;
	   }
      break;
#endif

#ifndef SS_NOCT_COMBO
   case SS_TYPE_COMBOBOX:
      if (CellType->Spec.ComboBox.hWndDropDown)
         SendMessage(hWndCtrl, SS_CBM_SETDROPDOWNHWND, 0,
                     (LPARAM)CellType->Spec.ComboBox.hWndDropDown);

      if (!CellType->Spec.ComboBox.hWndDropDown ||
          CellType->Spec.ComboBox.hItems)
         SendMessage(hWndCtrl, CB_RESETCONTENT, 0, 0L);

      SetWindowText(hWndCtrl, _T(""));

      if (CellType->Spec.ComboBox.dMaxEditLen >= 0)
         SendMessage(hWndCtrl, CB_LIMITTEXT,
                     CellType->Spec.ComboBox.dMaxEditLen, 0L);
      else
         SendMessage(hWndCtrl, CB_LIMITTEXT, 150, 0L);

      if (CellType->Spec.ComboBox.hItems)
         {
         lpItems = (LPTSTR)tbGlobalLock(CellType->Spec.ComboBox.hItems);

         for (i = 0; i < CellType->Spec.ComboBox.dItemCnt; i++)
            {
            // Refer to bug GRB806 for use of CB_ADDSTRING_WIN32S
            #ifdef SS_WIN32S
            SendMessage(hWndCtrl, CB_ADDSTRING_WIN32S, 0, (LPARAM)lpItems);
            #else
            SendMessage(hWndCtrl, CB_ADDSTRING, 0, (LPARAM)lpItems);
            #endif
            lpItems += lstrlen(lpItems) + 1;
            }

         tbGlobalUnlock(CellType->Spec.ComboBox.hItems);
         }

      if (hData = SS_GetData(lpSS, CellType, ColEdit,
                             RowEdit, FALSE))
         {
         Data = (LPTSTR)tbGlobalLock(hData);

         if (lstrlen(Data))
            {
            SetWindowText(hWndCtrl, Data);

            if ((int)tbGlobalSize(hData) > lstrlen(Data) + 1)
               {
               dSel = StringToInt(&Data[lstrlen(Data) + 1]);

               if (dSel != -1)
                  SendMessage(hWndCtrl, CB_SETCURSEL, dSel, 0L);
               }
            }

         tbGlobalUnlock(hData);
         }
// fix for bug 9632 -scl
      else
         SendMessage(hWndCtrl, CB_SETCURSEL, -1, 0L);

      

      MoveWindow(hWndCtrl, Rect.left + x + 1, Rect.top + y + 1, cx - 1,
                 cy - 1, TRUE);

      lpSS->NoEraseBkgnd = TRUE;
      ShowWindow(hWndCtrl, SW_SHOWNOACTIVATE);
		/* RFW - 8/8/05 - 16327
      UpdateWindow(hWndCtrl);
      SetFocus(hWndCtrl);
		*/
      SetFocus(hWndCtrl);
      UpdateWindow(hWndCtrl);
      lpSS->NoEraseBkgnd = FALSE;

      lpSS->lpBook->EditModeOn = TRUE;

      if ((Msg == WM_KEYDOWN || Msg == WM_LBUTTONDBLCLK) &&
          !((CellType->Style & 0x03) == SS_CB_DROPDOWN))
         SendMessage(hWndCtrl, CB_SHOWDROPDOWN, TRUE, 0L);

      else if (Msg == WM_LBUTTONDOWN && !lpSS->lpBook->fEditModePermanent)
         {
         SendMessage(hWndCtrl, CB_SHOWDROPDOWN, TRUE, 0L);
         Msg = WM_LBUTTONDBLCLK;
         }

      break;
#endif

   case SS_TYPE_DATE:
   case SS_TYPE_TIME:
   case SS_TYPE_INTEGER:
   case SS_TYPE_FLOAT:
#ifdef SS_V40
   case SS_TYPE_CURRENCY:
   case SS_TYPE_NUMBER:
   case SS_TYPE_PERCENT:
#endif // SS_V40
   case SS_TYPE_PIC:
#ifdef SS_V70
	case SS_TYPE_SCIENTIFIC:
#endif // SS_V70
      switch (CellType->Type)
         {
#ifndef SS_NOCT_DATE
         case SS_TYPE_DATE:
            SetWindowText(hWndCtrl, _T(""));
            SendMessage(hWndCtrl, DM_SETTWODIGITYEARMAX, 0, lpSS->lpBook->nTwoDigitYearMax);
#ifdef SS_V80
            SendMessage(hWndCtrl, DM_SETENHANCEDCALENDAR, 0, lpSS->lpBook->wAppearanceStyle == 2);
				DateSetSpinStyle(hWndCtrl, (lpSS->lpBook->wAppearanceStyle != 0 && lpSS->lpBook->wUseVisualStyles != 2) || (lpSS->lpBook->wAppearanceStyle == 0 && lpSS->lpBook->wUseVisualStyles == 1));
#endif

            if (CellType->Spec.Date.SetRange)
               DateSetRange(hWndCtrl, &CellType->Spec.Date.Min,
                            &CellType->Spec.Date.Max);

            if (CellType->Spec.Date.SetFormat)
               DateSetFormat(hWndCtrl, &CellType->Spec.Date.Format);

			            else
               {
               _fmemcpy(&DateFormat, &lpSS->lpBook->DefaultDateFormat,
                        sizeof(DATEFORMAT));

               if (CellType->Style & DS_CENTURY)
                  DateFormat.bCentury = TRUE;

               /**********************
               * Specify format flag
               **********************/

               if (CellType->Style & DS_DDMONYY)
                  DateFormat.nFormat = IDF_DDMONYY;

               else if (CellType->Style & DS_DDMMYY)
                  DateFormat.nFormat = IDF_DDMMYY;

               else if (CellType->Style & DS_MMDDYY)
                  DateFormat.nFormat = IDF_MMDDYY;

               else if (CellType->Style & DS_YYMMDD)
                  DateFormat.nFormat = IDF_YYMMDD;

               DateSetFormat(hWndCtrl, &DateFormat);
               }

            break;
#endif

#ifndef SS_NOCT_TIME
         case SS_TYPE_TIME:
            SetWindowText(hWndCtrl, _T(""));
#ifdef SS_V80
  			TimeSetSpinStyle(hWndCtrl, (lpSS->lpBook->wAppearanceStyle != 0 && lpSS->lpBook->wUseVisualStyles != 2) || (lpSS->lpBook->wAppearanceStyle == 0 && lpSS->lpBook->wUseVisualStyles == 1));
#endif
          if (CellType->Spec.Time.SetRange)
               TimeSetRange(hWndCtrl, &CellType->Spec.Time.Min,
                            &CellType->Spec.Time.Max);

            if (CellType->Spec.Time.SetFormat)
               TimeSetFormat(hWndCtrl, &CellType->Spec.Time.Format);

            else
               {
               TIMEFORMAT TimeFormat;

               _fmemcpy(&TimeFormat, &lpSS->lpBook->DefaultTimeFormat,
                        sizeof(TIMEFORMAT));

               if (CellType->Style & TS_SECONDS)
                  TimeFormat.bSeconds = TRUE;

               if (CellType->Style & TS_24HOUR)
                  TimeFormat.b24Hour = TRUE;

               TimeSetFormat(hWndCtrl, &TimeFormat);
               }

            break;
#endif

#ifndef SS_NOCT_PIC
         case SS_TYPE_PIC:
            if (CellType->Spec.Pic.hMask)
               Mask = (LPTSTR)tbGlobalLock(CellType->Spec.Pic.hMask);

            SendMessage(hWndCtrl, EPM_SETMASK, 0, (LPARAM)Mask);

            if (CellType->Spec.Pic.hMask)
               tbGlobalUnlock(CellType->Spec.Pic.hMask);

            break;
#endif

         case SS_TYPE_INTEGER:
         case SS_TYPE_FLOAT:
#ifdef SS_V40
			case SS_TYPE_CURRENCY:
			case SS_TYPE_NUMBER:
			case SS_TYPE_PERCENT:
#endif // SS_V40
#ifdef SS_V70
			case SS_TYPE_SCIENTIFIC:
#endif // SS_V70
#ifndef SS_NOCALC
            if (wParam == '=' && lpSS->lpBook->AllowUserFormulas)
               {
               CellType = &lpSS->lpBook->FormulaCellType;

               if (!CellType->ControlID)
                  SS_CreateControl(lpSS, CellType, FALSE);

               if (CellType->ControlID)
                  hWndCtrl = SS_GetControlhWnd(lpSS, CellType->ControlID);

               SendMessage(hWndCtrl, WM_SETFONT, (WPARAM)Font->hFont, 1L);
               #ifdef WIN32
               SendMessage(hWndCtrl, EM_SETMARGINS,
                           EC_LEFTMARGIN | EC_RIGHTMARGIN, 0);  // BJO 21Feb96 SCS2417
               #endif

               #ifdef SS_OLDCALC

               hData = SS_GetFormula(lpSS, ColEdit, RowEdit);

               #else

               {
				   SS_COORD lCol = ColEdit;
				   SS_COORD lRow = RowEdit;
               int nLen;
				   SS_AdjustCellCoordsOut(lpSS, &lCol, &lRow);
//?? Added by BOC FMH 1996.06.28. ----------------->>
/* if nArgs >= 3, the return len of function CalcGetExprLen is shorter. So the
 * last parament will not be shown in the run time.
*/
#ifdef SPREAD_JPN
               nLen = CALC_MAX_EXPR_LEN;
               hData = tbGlobalAlloc(GHND, nLen * sizeof(TCHAR));
               Data = (LPTSTR)tbGlobalLock(hData);
               nLen = CalcGetExpr(&lpSS->CalcInfo, lCol, lRow, 
#if SS_V80 // 24919 -scl
                  FALSE,
#endif
                  Data, nLen);
               tbGlobalUnlock(hData);
#else
//---------------------------<<
               nLen = CalcGetExprLen(&lpSS->CalcInfo, lCol, lRow
#if SS_V80 // 24919 -scl
                  , FALSE
#endif
                  );
               hData = tbGlobalAlloc(GHND, (nLen+1) * sizeof(TCHAR));
               Data = (LPTSTR)tbGlobalLock(hData);
               nLen = CalcGetExpr(&lpSS->CalcInfo, lCol, lRow, 
#if SS_V80 // 24919 -scl
                  FALSE,
#endif
                  Data, nLen+1);
               tbGlobalUnlock(hData);
#endif  //??
               if (0 == nLen)
                  {
                  tbGlobalFree(hData);
                  hData = 0;
                  }
               }

               #endif

               SS_TypeEditOn(hWndCtrl, lpSS, CellType, hData,
                             Rect, x, y, cx, cy, SS_BEM_CURSORPOS_BEGINNING, ColEdit, RowEdit, lParam);

               /*
               SendMessage(hWndCtrl, EM_SETSEL, 0, MAKELONG(lstrlen(Data),
                           lstrlen(Data)));
               */

               Edit_SetSel(hWndCtrl, 0, 0);
#ifdef WIN32
               Edit_ScrollCaret(hWndCtrl);
#endif
               if (hData)
                  {
                  Data = (LPTSTR)tbGlobalLock(hData);

                  if (Data[0] == '=')
                     wParam = 0;

                  tbGlobalUnlock(hData);
                  }
               
               #ifndef SS_OLDCALC
               if( hData )
               {
                 tbGlobalFree(hData);
                 hData = 0;
               }
               #endif

               lpSS->fFormulaMode = TRUE;
               }

            else if (CellType->Type == SS_TYPE_FLOAT)
#else
            if (CellType->Type == SS_TYPE_FLOAT)
#endif
               {
               FLOATFORMAT ff;
               SS_GetDefFloatFormat(lpSS->lpBook, &ff);

#ifndef SS_USEAWARE
               SetWindowText(hWndCtrl, _T(""));
               FloatSetRange(hWndCtrl, CellType->Spec.Float.Min,
                             CellType->Spec.Float.Max);

               _fmemset(Buffer, '\0', sizeof(Buffer));

               for (i = 0; i < CellType->Spec.Float.Left; i++)
                  _ftcscat(Buffer, _T("9"));

               _ftcscat(Buffer, _T("."));

               for (i = 0; i < CellType->Spec.Float.Right; i++)
                  _ftcscat(Buffer, _T("9"));

               FloatSetMask(hWndCtrl, Buffer);

#else
               // rdw-12/19
               SendMessage(hWndCtrl, DM_SETMAX, 0, (long)&CellType->Spec.Float.Max);
               SendMessage(hWndCtrl, DM_SETMIN, 0, (long)&CellType->Spec.Float.Min);
               // rdw-12/19.

               if (CellType->Style & FS_MONEY)
                  {                                 
               
                  // rdw-12/11
                  TCHAR szCurrencySign[2] = { 0, 0 };
                  if (CellType->Spec.Float.fSetFormat &&
                     CellType->Spec.Float.Format.cCurrencySign)
                     *szCurrencySign = CellType->Spec.Float.Format.cCurrencySign;
                  else
                     *szCurrencySign = ff.cCurrencySign;
                  SendMessage(hWndCtrl, DM_SETCURRENCYSYMB, 0, (long)(LPTSTR)szCurrencySign);
                  // rdw-12/11.
                  
                  SendMessage(hWndCtrl, DM_SETCURRENCYDIGITS, 2, 0L);
                  SendMessage(hWndCtrl, EM_SETEXTSTYLE, TRUE, DS_FIXEDPOINT);
                  SendMessage(hWndCtrl, EM_SETEXTSTYLE, TRUE, DS_CURRENCY);
                  }
               else
                  {
                  SendMessage(hWndCtrl, DM_SETFRACTWIDTH,
                              CellType->Spec.Float.Right, 0L);
                  SendMessage(hWndCtrl, EM_SETEXTSTYLE, FALSE, DS_FIXEDPOINT);
                  SendMessage(hWndCtrl, EM_SETEXTSTYLE, FALSE, DS_CURRENCY);
                  
                  }
#endif

               // BJO 03Mar97 SCS4646 - Before fix
               //if (CellType->Spec.Float.fSetFormat)
               //   FloatSetFormat(hWndCtrl, &CellType->Spec.Float.Format);
               // BJO 03Mar97 SCS4646 - Begin fix
               if (CellType->Spec.Float.fSetFormat)
                  {
                  if (CellType->Spec.Float.Format.cCurrencySign)
                     ff.cCurrencySign = CellType->Spec.Float.Format.cCurrencySign;
                  if (CellType->Spec.Float.Format.cSeparator)
                     ff.cSeparator = CellType->Spec.Float.Format.cSeparator;
                  if (CellType->Spec.Float.Format.cDecimalSign)
                     ff.cDecimalSign = CellType->Spec.Float.Format.cDecimalSign;
                  }
               FloatSetFormat(hWndCtrl, &ff);
               // BJO 03Mar97 SCS4646 - End fix
               }

            else if (CellType->Type == SS_TYPE_INTEGER)
               {
#ifndef SS_NOCT_INT
               SetWindowText(hWndCtrl, _T(""));
               wsprintf(Buffer, _T("%ld"), CellType->Spec.Integer.Max);

               for (i = 0; i < lstrlen(Buffer); i++)
                  Buffer[i] = '9';

               IntSetMask(hWndCtrl, Buffer);

               IntSetRange(hWndCtrl, CellType->Spec.Integer.Min,
                           CellType->Spec.Integer.Max);

               if (CellType->Style & IS_SPIN)
			   {
                  IntSetSpin(hWndCtrl, TRUE, CellType->Spec.Integer.fSpinWrap,
                             CellType->Spec.Integer.lSpinInc);
#ifdef SS_V80
				IntSetSpinStyle(hWndCtrl, (lpSS->lpBook->wAppearanceStyle != 0 && lpSS->lpBook->wUseVisualStyles != 2) || (lpSS->lpBook->wAppearanceStyle == 0 && lpSS->lpBook->wUseVisualStyles == 1));
#endif
			   }
#endif
               }

#ifdef SS_V40
            else if (CellType->Type == SS_TYPE_CURRENCY)
               {
               SSNUM_FORMAT Format;

               SetWindowText(hWndCtrl, _T(""));
               NumSetRange(hWndCtrl, CellType->Spec.Currency.Min,
                           CellType->Spec.Currency.Max);

					SS_CreateCurrencyFormatStruct(CellType, &Format);
					NumSetFormat(hWndCtrl, &Format);
#ifdef SS_V80
				NumSetSpinStyle(hWndCtrl, (lpSS->lpBook->wAppearanceStyle != 0 && lpSS->lpBook->wUseVisualStyles != 2) || (lpSS->lpBook->wAppearanceStyle == 0 && lpSS->lpBook->wUseVisualStyles == 1));
#endif
              }

            else if (CellType->Type == SS_TYPE_NUMBER)
               {
               SSNUM_FORMAT Format;

               SetWindowText(hWndCtrl, _T(""));
               NumSetRange(hWndCtrl, CellType->Spec.Number.Min,
                           CellType->Spec.Number.Max);

					SS_CreateNumberFormatStruct(CellType, &Format);
					NumSetFormat(hWndCtrl, &Format);
  #ifdef SS_V80
				NumSetSpinStyle(hWndCtrl, (lpSS->lpBook->wAppearanceStyle != 0 && lpSS->lpBook->wUseVisualStyles != 2) || (lpSS->lpBook->wAppearanceStyle == 0 && lpSS->lpBook->wUseVisualStyles == 1));
#endif
             }

            else if (CellType->Type == SS_TYPE_PERCENT)
               {
               SSNUM_FORMAT Format;

               SetWindowText(hWndCtrl, _T(""));
               NumSetRange(hWndCtrl, CellType->Spec.Percent.Min,
                           CellType->Spec.Percent.Max);

					SS_CreatePercentFormatStruct(CellType, &Format);
					NumSetFormat(hWndCtrl, &Format);
 #ifdef SS_V80
				NumSetSpinStyle(hWndCtrl, (lpSS->lpBook->wAppearanceStyle != 0 && lpSS->lpBook->wUseVisualStyles != 2) || (lpSS->lpBook->wAppearanceStyle == 0 && lpSS->lpBook->wUseVisualStyles == 1));
#endif
              }
#endif SS_V40

#ifdef SS_V70
            else if (CellType->Type == SS_TYPE_SCIENTIFIC)
               {
               SetWindowText(hWndCtrl, _T(""));
               }
#endif // SS_V70

            break;

         }

      if (!lpSS->fFormulaMode)
         {
#ifdef SS_V40
			TCHAR szBuf[100];
#endif // SS_V40
         dLen = 0;

#ifdef SS_V40
         if (CellType->Type == SS_TYPE_PERCENT)
            {
				double dfVal;

            SS_GetFloat(lpSS, ColEdit, RowEdit, &dfVal);
				dfVal *= 100.0;

				// The following code was added to take care of an Intel floating point problem.
				StrPrintf(szBuf, _T("%.*f"), CellType->Spec.Percent.Right, dfVal);
				StringToFloat(szBuf, &dfVal);

				NumSetValue(hWndCtrl, dfVal);
				GetWindowText(hWndCtrl, szBuf, 100);
				Data = szBuf;
            dLen = lstrlen(Data);
            }

         else
#endif // SS_V40

#ifdef SS_V70
         if (CellType->Type == SS_TYPE_SCIENTIFIC)
            {
				if (hData = SS_GetData(lpSS, CellType, ColEdit,
											  RowEdit, TRUE))
					{
					Data = (LPTSTR)tbGlobalLock(hData);
					dLen = lstrlen(Data);
					SetWindowText(hWndCtrl, Data);

					tbGlobalUnlock(hData);
					}
            }

         else
#endif // SS_V40

         if (hData = SS_GetData(lpSS, CellType, ColEdit,
                                RowEdit, FALSE))
            {
            Data = (LPTSTR)tbGlobalLock(hData);
            dLen = lstrlen(Data);
            SetWindowText(hWndCtrl, Data);

            tbGlobalUnlock(hData);
            }

         else
            SetWindowText(hWndCtrl, _T(""));

         SendMessage(hWndCtrl, EM_SETMODIFY, 0, 0L);

#ifdef SS_V70
         if (CellType->Type == SS_TYPE_SCIENTIFIC)
				{
				// RFW - 7/1/04 - 14735
				SS_COLORTBLITEM BackColorTblItem;
				SS_COLORTBLITEM ForeColorTblItem;
				HDC             hDC = GetDC(lpSS->lpBook->hWnd);
				RECT            RectTemp = {Rect.left + x + 1, Rect.top + y + 1,
													 Rect.left + x + cx, Rect.top + y + cy};

				SS_GetColorTblItem(lpSS, ColEdit, RowEdit,
										 &BackColorTblItem, &ForeColorTblItem);
				FillRect(hDC, &RectTemp, BackColorTblItem.hBrush);

				ReleaseDC(lpSS->lpBook->hWnd, hDC);

	         MoveWindow(hWndCtrl, Rect.left + x + 1, Rect.top + y + 2, cx - 3,
	                    cy - 2, TRUE);
				}
			else
#endif // SS_V70
	         MoveWindow(hWndCtrl, Rect.left + x + 1, Rect.top + y + 1, cx - 1,
	                    cy - 1, TRUE);

#ifdef SS_V40
			if (lpSS->lpBook->fEditOverflow)
				{
				RECT RectExpanded;

				SS_GetLargestExpandingEditCell(lpSS, CellType, hWndCtrl, ColEdit, RowEdit,
														 dLen ? Data : _T(""), dLen, TRUE, &RectExpanded);
				MoveWindow(hWndCtrl, RectExpanded.left, RectExpanded.top,
							  RectExpanded.right - RectExpanded.left,
							  RectExpanded.bottom - RectExpanded.top, TRUE);
				}
#endif // SS_V40

         lpSS->NoEraseBkgnd = TRUE;
         ShowWindow(hWndCtrl, SW_SHOWNORMAL);
         UpdateWindow(hWndCtrl);
         SetFocus(hWndCtrl);
         lpSS->NoEraseBkgnd = FALSE;

			if (wCursorPos == SS_BEM_CURSORPOS_SELECTALL)
				Edit_SetSel(hWndCtrl, 0, -1);
			else if (wCursorPos == SS_BEM_CURSORPOS_BEGINNING)
				Edit_SetSel(hWndCtrl, 0, 0);
 			else if (wCursorPos == SS_BEM_CURSORPOS_END)
				Edit_SetSel(hWndCtrl, max(1,dLen), max(1,dLen));
 			else if (wCursorPos == SS_BEM_CURSORPOS_MOUSE)
				{
				int nChar = (int)SendMessage(hWndCtrl, EM_CHARFROMPOS, 0, lParam);
				Edit_SetSel(hWndCtrl, nChar, nChar);
				}
			else if (wCursorPos == SS_BEM_CURSORPOS_DEFAULT)
				{
				// RFW - 6/13/04 - 14567
#ifdef SS_V70
        if (CellType->Type == SS_TYPE_INTEGER || CellType->Type == SS_TYPE_SCIENTIFIC)
#else
        if (CellType->Type == SS_TYPE_INTEGER)
#endif
          {
					if (lpSS->lpBook->fEditModeReplace)
						Edit_SetSel(hWndCtrl, 0, -1);
					else
						Edit_SetSel(hWndCtrl, max(1,dLen), max(1,dLen)); 
					}
				else if (SS_IsCellTypeFloatNum(CellType) || CellType->Type == SS_TYPE_PIC)
					{
					if (lpSS->lpBook->fEditModeReplace)
						Edit_SetSel(hWndCtrl, 0, -1);
					}
				}

            
#ifdef SS_UTP
         if (dLen)
            {
            if (lpSS->wLastAdvanceMsg == EN_AUTOADVANCEPREV)
               {
               Edit_SetSel(hWndCtrl, dLen, dLen);
#ifdef WIN32
               Edit_ScrollCaret(hWndCtrl);
#endif
               }
            else if (lpSS->wLastAdvanceMsg == EN_AUTOADVANCENEXT)
               {
               Edit_SetSel(hWndCtrl, 0, 0);
#ifdef WIN32
               Edit_ScrollCaret(hWndCtrl);
#endif
               }
            else if (lpSS->wLastAdvanceMsg == EN_AUTOADVANCEUP ||
                     lpSS->wLastAdvanceMsg == EN_AUTOADVANCEDOWN)
               {
               Edit_SetSel(hWndCtrl, 0, 0);
#ifdef WIN32
               Edit_ScrollCaret(hWndCtrl);
#endif
               SendMessage(hWndCtrl, EM_SETPIXELPOS, PIXEL_CURRENT,
                           (long)lpSS->dLastAdvancePos);
               }
            }
#endif
         }
      break;

#ifdef SS_V80
   case SS_TYPE_CUSTOM:
      { 
         LPSS_CELLTYPE lpCellType = SS_CT_LockCellType(lpSS, ColEdit, RowEdit);
         BOOL bRet = SS_TypeCustomOn(lpSS, hWndCtrl, lpCellType, ColEdit, RowEdit, &Rect, x, y, cx, cy, Msg, wParam, lParam);

         SS_CT_UnlockCellType(lpSS, ColEdit, RowEdit);
         if( !bRet )
            return FALSE;
      }
      break;
#endif // SS_V80
   }

#ifdef SS_UTP
lpSS->wLastAdvanceMsg = 0;
#endif

lpSS->lpBook->EditModeOn = TRUE;

// RFW - 10/1/03
if (Msg == WM_LBUTTONDOWN && (wCursorPos == SS_BEM_CURSORPOS_MOUSE || 
    CellType->Type == SS_TYPE_CHECKBOX || CellType->Type == SS_TYPE_BUTTON ||
    CellType->Type == SS_TYPE_COMBOBOX))
	{
	int xPos = LOWORD(lParam) + x;
	int yPos = HIWORD(lParam) + y;
	POINT pt = {xPos, yPos};

	ClientToScreen(lpSS->lpBook->hWnd, &pt);
	ScreenToClient(hWndCtrl, &pt);
	lParam = MAKELONG(pt.x, pt.y);
   SendMessage(hWndCtrl, Msg, wParam, lParam);
	}

else if ((CellType->Type == SS_TYPE_CHECKBOX ||
          CellType->Type == SS_TYPE_BUTTON) && Msg == WM_CHAR && wParam)
   SendMessage(hWndCtrl, WM_KEYDOWN, wParam, lParam);

else if ((Msg == WM_CHAR || Msg == WM_IME_CHAR || Msg == WM_SYSKEYDOWN) && wParam)
#ifdef SPREAD_JPN
#ifdef	WIN32
	{
	// SPRD010 Wei Feng
	BOOL		   bFirstChar = FALSE;

	// Sp25_002 [1-1]
	// When Type Is Not Text
	// The CellType->Spec.Edit.ChrSet Has Other Meaning :(
	// So Only Change To IME Mode When It Is Text Type :)
	// -- HaHa 1998.4.21
	if (CellType->Type==2)
		// SPRD010 Wei Feng 1997/9/5
		if (CHARSET_TURNON_IME(CellType->Spec.Edit) )
			{
			TCHAR cFirstChar[2];
			GetSetIMEMode(hWndCtrl, FALSE, TRUE);
			cFirstChar[0] = (TCHAR)wParam;
			cFirstChar[1] = 0;
			//Modify by BOC 99.8.19 (hyt)--------------------------------------------------
			//for not convert single byte katakana
			//if ( !IsDBCSLeadByte((BYTE)cFirstChar[0]))
			if ( !IsDBCSLeadByte((BYTE)cFirstChar[0]) && !_ismbbkana((BYTE)cFirstChar[0]))
			//-------------------------------------------------------------------------------
				{
				HIMC hImc;
				hImc = ImmGetContext(hWndCtrl);
				if(hImc)
					{
#ifdef _UNICODE // RFW - 2/26/07 - 19759
	            bFirstChar = ImmSetCompositionString(hImc, SCS_SETSTR, cFirstChar, 2, cFirstChar, 2);
#else
	            bFirstChar = ImmSetCompositionString(hImc, SCS_SETSTR, cFirstChar, 1, cFirstChar, 1);
#endif
					ImmReleaseContext(hWnd, hImc);
					}
				}
			}

   // SPRD010 Wei Feng 1997/9/5
   if (!bFirstChar)
   //- JPNFIX0004 - (Masanori Iwasa)
	// RFW - 4/29/02 - RUN_SPR_008_001
	   PostMessage(hWndCtrl, Msg, wParam, lParam);
	}
#else
   //--------Changed by dean 1998/6/18--------
   //-To fix the bug keypress disable key when keyascii = 0(16bit)
   PostMessage(hWndCtrl, Msg, wParam, lParam);
   //SendMessage(hWndCtrl, Msg, wParam, lParam);
   //----------------------------------------
#endif
#else
#if SS_V80
   if( CellType->Type != SS_TYPE_CUSTOM ) // already sent message -scl
#endif
	{
	// RFW - 12/8/03 - 13040
	/* RFW - 2/25/05 - 15771 */
#ifdef SS_V70
   if ((GetKeyState(VK_DECIMAL) & 0x8000) && (SS_IsCellTypeFloatNum(CellType) && CellType->Type != SS_TYPE_SCIENTIFIC))
#else
   if ((GetKeyState(VK_DECIMAL) & 0x8000) && SS_IsCellTypeFloatNum(CellType))
#endif
		wParam = VK_DECIMAL;
	SendMessage(hWndCtrl, Msg, wParam, lParam);
	}
#endif

lpSS->lpBook->EditModeTurningOn = FALSE;

#ifdef SS_UTP
if (lpSS->fUseEditModeShadow)
   SS_PaintEditModeShadow(hWnd, lpSS, &Rect, x, y, cx, cy);
#endif

return (TRUE);
}


static void SS_TypeEditOn(HWND hWndCtrl, LPSPREADSHEET lpSS, LPSS_CELLTYPE CellType, TBGLOBALHANDLE hData,
                          RECT Rect, int x, int y, int cx, int cy, WORD wCursorPos, SS_COORD ColEdit, SS_COORD RowEdit, LPARAM lParam)
{
LPTSTR Data = 0;
short  Len = 0;
long   lStyle = CellType->Style;

lStyle &= ~ES_LEFTALIGN;
lStyle &= 0xffff;

SetWindowLong(hWndCtrl, GWL_STYLE, WS_CHILD | (long)(WORD)lStyle);

#ifdef SS_USEAWARE
SendMessage(hWndCtrl, EM_SETTEXTLIMIT, 0, (long)CellType->Spec.Edit.Len);
#else
SendMessage(hWndCtrl, EM_LIMITTEXT, CellType->Spec.Edit.Len, 0L);
#endif

if (hData)
   {
   Data = (LPTSTR)tbGlobalLock(hData);
   SendMessage(hWndCtrl, WM_SETTEXT, 0, (LPARAM)Data);
   Len = lstrlen(Data);
   }

#ifndef SS_USEAWARE
else if (CellType->Style & (ES_AUTOHSCROLL | ES_MULTILINE))
   SetWindowText(hWndCtrl, _T(" "));
#endif

else
   SetWindowText(hWndCtrl, _T(""));

SendMessage(hWndCtrl, EM_SETMODIFY, 0, 0L);
lpSS->fEditModeDataChange = FALSE;

{
SS_COLORTBLITEM BackColorTblItem;
SS_COLORTBLITEM ForeColorTblItem;
HDC             hDC = GetDC(lpSS->lpBook->hWnd);
RECT            RectTemp = {Rect.left + x + 1, Rect.top + y + 1,
                            Rect.left + x + cx, Rect.top + y + cy};

SS_GetColorTblItem(lpSS, ColEdit, RowEdit,
                   &BackColorTblItem, &ForeColorTblItem);
FillRect(hDC, &RectTemp, BackColorTblItem.hBrush);

ReleaseDC(lpSS->lpBook->hWnd, hDC);
}

#ifdef SS_V40
{
int yXtra = (CellType->Style & (ES_AUTOHSCROLL | ES_MULTILINE)) ? 1 : 0;
//int xXtra = (CellType->Style & ES_MULTILINE) ? 1 : 0;
int xXtra = 1;

// RFW - 5/9/03 - 11960
//MoveWindow(hWndCtrl, Rect.left + x + 1, Rect.top + y + 1 + yXtra, cx - 1, cy - 1 - (yXtra * 2), TRUE);
//SendMessage(hWndCtrl, EM_SETMARGINS, EC_LEFTMARGIN | EC_RIGHTMARGIN, MAKELONG(2, xXtra + 1));
MoveWindow(hWndCtrl, Rect.left + x + 1, Rect.top + y + 1 + yXtra, cx - 1 - (xXtra + 1), cy - 1 - (yXtra * 2), TRUE);
SendMessage(hWndCtrl, EM_SETMARGINS, EC_LEFTMARGIN | EC_RIGHTMARGIN, MAKELONG(2, 0));

if (lpSS->lpBook->fEditOverflow)
	{
	RECT RectExpanded;

	SS_GetLargestExpandingEditCell(lpSS, CellType, hWndCtrl, ColEdit, RowEdit,
                                  Data ? Data : _T(""), Len, TRUE, &RectExpanded);
   MoveWindow(hWndCtrl, RectExpanded.left, RectExpanded.top,
              RectExpanded.right - RectExpanded.left,
              RectExpanded.bottom - RectExpanded.top, TRUE);
	}
}
#else
if (CellType->Style & ES_AUTOHSCROLL)
   MoveWindow(hWndCtrl, Rect.left + x + 3, Rect.top + y + 2, cx - 4, cy - 3, TRUE);
else if (CellType->Style & ES_MULTILINE)
   MoveWindow(hWndCtrl, Rect.left + x + 3, Rect.top + y + 2, cx - 5, cy - 3, TRUE);
else
   MoveWindow(hWndCtrl, Rect.left + x + 1, Rect.top + y + 1, cx - 1, cy - 1, TRUE);
#endif // SS_V40

lpSS->NoEraseBkgnd = TRUE;
ShowWindow(hWndCtrl, SW_SHOWNORMAL);
UpdateWindow(hWndCtrl);

if ((CellType->Style & (ES_AUTOHSCROLL | ES_MULTILINE)) && !hData)
   SetWindowText(hWndCtrl, _T(""));

SetFocus(hWndCtrl);
lpSS->NoEraseBkgnd = FALSE;

if (hData)
   tbGlobalUnlock(hData);

if (Len)
   {
	if (wCursorPos == SS_BEM_CURSORPOS_SELECTALL)
		Edit_SetSel(hWndCtrl, 0, Len);
	else if (wCursorPos == SS_BEM_CURSORPOS_BEGINNING)
		Edit_SetSel(hWndCtrl, 0, 0);
 	else if (wCursorPos == SS_BEM_CURSORPOS_END)
		Edit_SetSel(hWndCtrl, Len, Len);
 	else if (wCursorPos == SS_BEM_CURSORPOS_MOUSE)
		{
		int nChar = (int)SendMessage(hWndCtrl, EM_CHARFROMPOS, 0, lParam);
		Edit_SetSel(hWndCtrl, nChar, nChar);
		}

#ifdef WIN32
   Edit_ScrollCaret(hWndCtrl);
#endif
   }
}

#if SS_V80
BOOL SS_TypeCustomOn(LPSPREADSHEET lpSS, HWND hWndCtrl, LPSS_CELLTYPE CellType, SS_COORD ColEdit, SS_COORD RowEdit, LPRECT Rect, int x, int y, int cx, int cy, UINT Msg, WPARAM wParam, LPARAM lParam)
{
   BOOL Ret = FALSE;
   LPTSTR lpszName;
   SS_CT_PROCS Procs;
   BOOL bEditable;
#if SS_OCX
   FPCONTROL ctl = {0};
#endif

   if( CellType->Type == SS_TYPE_CUSTOM && (lpszName = (LPTSTR)GlobalLock(CellType->Spec.Custom.hName)) )
   {
      if( SS_CustCellTypeLookup(lpSS->lpBook, lpszName, &bEditable, NULL, NULL, NULL, &Procs) 
        && bEditable )
      {
        RECT rcEditor;
        BOOL NoEraseBkgnd = lpSS->NoEraseBkgnd;
        
        SS_CT_InitializeEditorControl(lpSS, CellType, ColEdit, RowEdit);
        if( !SS_CT_SetEditorValue(lpSS, CellType, ColEdit, RowEdit) )
        { // send a WM_SETTEXT to the control
          TBGLOBALHANDLE hData;
          LPTSTR Data;
#if SS_OCX
          // update hWnd in case it changed
          hWndCtrl = SS_GetControlhWnd(lpSS, CellType->ControlID);
#endif
          if (hData = SS_GetData(lpSS, CellType, ColEdit, RowEdit, FALSE))
          {
            Data = (LPTSTR)tbGlobalLock(hData);
            SendMessage(hWndCtrl, WM_SETTEXT, 0, (LPARAM)Data);
            tbGlobalUnlock(hData);
          }
        }
#if SS_OCX
        // update hWnd in case it changed
        hWndCtrl = SS_GetControlhWnd(lpSS, CellType->ControlID);
#endif
        SetRect(&rcEditor,  Rect->left + x + 1, Rect->top + y + 1, Rect->left + x + cx, Rect->top + y + cy - 1);
        MapWindowPoints(lpSS->lpBook->hWnd, NULL, (LPPOINT)&rcEditor, 2);
        MapWindowPoints(NULL, GetParent(hWndCtrl), (LPPOINT)&rcEditor, 2);
#if SS_OCX
        ctl.lpDispatch = SS_GetControlDispatch(lpSS, CellType->ControlID);
        if( ctl.lpDispatch )
        {
           RECT rcTemp;
           // position control over the cell
           // this is like fpSetRectFromDispatch, except it defaults to pixels if the container
           // does not implement a ScaleMode property.
           CopyRect(&rcTemp, &rcEditor);
           fpSetRectFromDispatchEx(&ctl, &rcEditor, 0, TRUE);
           CopyRect(&rcEditor, &rcTemp);
           GetWindowRect(hWndCtrl, &rcTemp);
           MapWindowPoints(NULL, GetParent(hWndCtrl), (LPPOINT)&rcTemp, 2);
           if( rcTemp.left != rcEditor.left || rcTemp.top != rcEditor.top || rcTemp.right != rcEditor.right || rcTemp.bottom != rcEditor.bottom )
              MoveWindow(hWndCtrl, rcEditor.left, rcEditor.top, rcEditor.right - rcEditor.left, rcEditor.bottom - rcEditor.top, FALSE);
           lpSS->NoEraseBkgnd = TRUE;
           SetWindowPos(hWndCtrl, HWND_TOP, 0, 0, 0, 0, SWP_NOMOVE|SWP_NOSIZE|SWP_NOOWNERZORDER|SWP_SHOWWINDOW);
           UpdateWindow(hWndCtrl);
        }
        else
#endif
        {
           MoveWindow(hWndCtrl, rcEditor.left, rcEditor.top, rcEditor.right - rcEditor.left, rcEditor.bottom - rcEditor.top, FALSE);
           lpSS->NoEraseBkgnd = TRUE;
           SetWindowPos(hWndCtrl, HWND_TOP, 0, 0, 0, 0, SWP_NOMOVE|SWP_NOSIZE|SWP_NOOWNERZORDER|SWP_SHOWWINDOW);
           UpdateWindow(hWndCtrl);
        }
        lpSS->NoEraseBkgnd = NoEraseBkgnd;
        // subclass the window
        SS_CT_SubclassEditor(lpSS, CellType);
        SetFocus(hWndCtrl);
        lpSS->NoEraseBkgnd = FALSE;
        if( !SS_CT_StartEditing(lpSS, CellType, ColEdit, RowEdit, Msg, wParam, lParam) && !(Msg == WM_KEYDOWN && wParam == VK_RETURN) )
        {
           HWND hWndMsg = hWndCtrl;
           if( WM_MOUSEFIRST <= Msg && Msg <= WM_MOUSELAST)
           {
              POINT pt;

              pt.x = LOWORD(lParam);
              pt.y = HIWORD(lParam);
              MapWindowPoints(hWndCtrl, NULL, &pt, 1);
              hWndMsg = WindowFromPoint(pt);
              if( hWndMsg != hWndCtrl )
              {
                 MapWindowPoints(NULL, hWndMsg, &pt, 1);
                 lParam = MAKELONG(pt.x, pt.y);
              }
           }
           SendMessage(hWndMsg, Msg, wParam, lParam);
        }
        Ret = TRUE;
      }
      GlobalUnlock(CellType->Spec.Custom.hName);
   }
   return Ret;
}
#endif
#ifdef SS_GP

void SS_SizeExpandingEdit(LPSPREADSHEET lpSS, HWND hWndCtrl, RECT Rect,
                          LPSS_CELLTYPE lpCellType, short x, short y, short cx,
                          short cy, LPRECT lpRectCurrent, TBGLOBALHANDLE hData)
{
LPTSTR Data;
HFONT  hFontOld;
short  dLeft;
short  dTop;
short  dWidth;
short  dHeight;
BOOL   fExpand = FALSE;

dLeft = Rect.left + x + 3;
dTop = Rect.top + y + 2;
dWidth = ((lpCellType->Style & ES_MULTILINE) ? cx - 5 : cx - 4);
dHeight = cy - 3;

if (lpCellType->Spec.Edit.dfWidth)
   {
   HDC   hDCCtrl;
   //DWORD dwTextExtent = 0L;
   SIZE sizeTextExtent = {0, 0};

   hDCCtrl = fpGetDC(hWndCtrl);

   hFontOld = SelectObject(hDCCtrl, (HFONT)SendMessage(hWndCtrl, WM_GETFONT, 0,
                           0L));

   if (hData)
      {
      Data = (LPTSTR)tbGlobalLock(hData);
      GetTextExtentPoint(hDCCtrl, Data, lstrlen(Data), &sizeTextExtent);

      if ((short)sizeTextExtent.cx > cx - 5 || _ftcschr(Data, '\r') ||
          _ftcschr(Data, '\n'))
         fExpand = TRUE;

      tbGlobalUnlock(hData);
      }

   if (fExpand)
      {
      TEXTMETRIC TextMetric;

      GetTextMetrics(hDCCtrl, &TextMetric);

      cx = SS_ColWidthToPixels(lpSS, lpCellType->Spec.Edit.dfWidth) + 2;

//      cy = ((TextMetric.tmHeight - TextMetric.tmDescent) *
//           lpCellType->Spec.Edit.nRowCnt) + 2;
      cy = (sizeTextExtent.cy * lpCellType->Spec.Edit.nRowCnt) + 2;

      if (x + cx > Rect.right - Rect.left)
         x = max((Rect.right - Rect.left) - cx, 0);

      if (y + cy > Rect.bottom - Rect.top)
         y = max((Rect.bottom - Rect.top) - cy, 0);

      dLeft = Rect.left + x;
      dTop = Rect.top + y;
      dWidth = cx;
      dHeight = cy;
      }

   SelectObject(hDCCtrl, hFontOld);
   ReleaseDC(hWndCtrl, hDCCtrl);
   }

if (!lpRectCurrent || lpRectCurrent->left != dLeft ||
    lpRectCurrent->top != dTop ||
    lpRectCurrent->right - lpRectCurrent->left != dWidth ||
    lpRectCurrent->bottom - lpRectCurrent->top != dHeight)
   {

   if (fExpand)
      SetWindowLong(hWndCtrl, GWL_STYLE, WS_CHILD | WS_BORDER |
                    ES_MULTILINE | WS_VSCROLL | (long)(WORD)lpCellType->Style);
   else
      SetWindowLong(hWndCtrl, GWL_STYLE, WS_CHILD | (long)(WORD)lpCellType->Style);

   if (lpRectCurrent)
      SendMessage(hWndCtrl, WM_SETREDRAW, TRUE, 0L);

   MoveWindow(hWndCtrl, dLeft, dTop, dWidth, dHeight, TRUE);

   if (!fExpand)
      {
      SS_COLORTBLITEM BackColorTblItem;
      SS_COLORTBLITEM ForeColorTblItem;
      RECT            RectTemp;
      HDC             hDC;

      UpdateWindow(GetParent(hWndCtrl));

      hDC = fpGetDC(GetParent(hWndCtrl));

      SS_GetColorTblItem(lpSS, ColEdit, RowEdit,
                         &BackColorTblItem, &ForeColorTblItem);
      SetRect(&RectTemp, Rect.left + x + 1, Rect.top + y + 1,
              Rect.left + x + cx, Rect.top + y + cy);
      FillRect(hDC, &RectTemp, BackColorTblItem.hBrush);
      ReleaseDC(GetParent(hWndCtrl), hDC);
      }

   if (lpRectCurrent && hData && fExpand)
      {
      long lSel = SendMessage(hWndCtrl, EM_GETSEL, 0, 0L);

      Data = (LPTSTR)tbGlobalLock(hData);
      SendMessage(hWndCtrl, WM_SETTEXT, 0, (long)Data);
      tbGlobalUnlock(hData);
      Edit_SetSel(hWndCtrl, LOWORD(lSel), HIWORD(lSel));
#ifdef WIN32
      Edit_ScrollCaret(hWndCtrl);
#endif
      }
   else
      InvalidateRect(hWndCtrl, NULL, TRUE);
   }
}

#endif


BOOL SS_CellEditModeOff(LPSPREADSHEET lpSS, HWND hWndFocus)
{
LPSS_BOOK      lpBook = lpSS->lpBook;
TBGLOBALHANDLE hGlobalData;
GLOBALHANDLE   hText;
HWND           hWndCtrl = 0;
LPSS_CELL      lpCell;
LPSS_CELLTYPE  CellType;
SS_CELLTYPE    CellTypeTemp;
LPTSTR         lpItems;
LPTSTR         lpText;
LPTSTR         Data;
SS_COORD       ColEdit, RowEdit;

#ifdef  BUGS
// Bug-002
TCHAR          Buffer[40];
#else
TCHAR          Buffer[330];
#endif

double         dfValue;
BOOL           EscapeInProgress;
BOOL           fChangeMade = FALSE;
BOOL           fErase;
BOOL           fNoEraseBkgnd;
BOOL           fNoSetFocus = FALSE;
BOOL           fIsCellSpan; 
// 21093 -scl
//short          Sel;
long Sel;
short          dLen;
short          i;

if (!lpBook->hWnd)  // handle an OCX control without a HWND
   return TRUE;

if (!SS_IsActiveSheet(lpSS))
	return (TRUE);

fIsCellSpan = SS_GetCellBeingEdited(lpSS, &ColEdit, &RowEdit);

EscapeInProgress = lpSS->EscapeInProgress;
lpSS->EscapeInProgress = FALSE;
// RFW - 6/3/04 - 14524
lpBook->EditModeTurningOn = FALSE;

// RFW - 12/5/03 - 13078
//if (lpBook->EditModeOn && !lpBook->wMessageBeingSent && !lpBook->EditModeTurningOff)
if (lpBook->EditModeOn && !lpBook->EditModeTurningOff)
   {
   lpCell = SS_LockCellItem(lpSS, ColEdit, RowEdit);
   CellType = SS_RetrieveCellType(lpSS, &CellTypeTemp, lpCell, ColEdit, RowEdit);

   if (CellType->ControlID)
      hWndCtrl = SS_GetControlhWnd(lpSS, CellType->ControlID);

	// RFW - 8/2/06 - 19129
	if (!hWndCtrl || !IsWindow(hWndCtrl))
		return (0);

   lpBook->EditModeTurningOff = TRUE;
   switch (CellType->Type)
      {
      case SS_TYPE_EDIT:
         fErase = FALSE;

         if (!EscapeInProgress &&
             (fChangeMade = (BOOL)SendMessage(hWndCtrl, EM_GETMODIFY, 0, 0L)))
            {
/* RFW - 5/22/03
            if (CellType->Type == SS_TYPE_EDIT)
               dLen = CellType->Spec.Edit.Len;
            else
               dLen = GetWindowTextLength(hWndCtrl);
*/
            dLen = GetWindowTextLength(hWndCtrl);

            if (hGlobalData = tbGlobalAlloc(GMEM_MOVEABLE | GMEM_ZEROINIT,
                                            (dLen + 1) * sizeof(TCHAR)))
               {
               Data = (LPTSTR)tbGlobalLock(hGlobalData);
               _fmemset(Data, '\0', (dLen + 1) * sizeof(TCHAR));
               GetWindowText(hWndCtrl, Data, dLen + 1);
//Modify by BOC 99.6.25 (hyt)-------------------------
//I don't know why trim left space but generate
//Bug 01325
#ifndef SPREAD_JPN
               if (CellType->Spec.Edit.ChrSet == SS_CHRSET_ALPHA ||
                   CellType->Spec.Edit.ChrSet == SS_CHRSET_ALPHANUM)
//GAB 03/07/02                  while (*Data && *Data == ' ')
                  while (*Data && *Data == _T(' '))
                     {
                     fErase = TRUE;
                     Data++;
                     }
#endif //SPREAD_JPN

// GAB 3/8/02 - Added for Unicode
#ifdef _UNICODE
					fErase = TRUE;
#endif
               SSx_SetCellData(lpSS, ColEdit, RowEdit,
                               Data, (short)lstrlen(Data), FALSE);
               tbGlobalUnlock(hGlobalData);
               tbGlobalFree(hGlobalData);
               }
            }
#ifndef SS_NOOVERFLOW
			else
            SS_OverflowAdjustNeighbor(lpSS, ColEdit, RowEdit);
#endif

#ifdef SS_V40
			// RFW - 6/10/04 - 14423
			// InvalidateMergedCell(lpSS, ColEdit, RowEdit);
			{
			BOOL fRet;

			lpSS->lpBook->Redraw = FALSE;
			fRet = InvalidateMergedCell(lpSS, ColEdit, RowEdit);
			lpSS->lpBook->Redraw = TRUE;
			/* RFW - 6/23/04 - 14255
			if (fRet)
				{
				SS_UpdateWindow(lpSS->lpBook);
				lpSS->NoEraseBkgnd = TRUE;
				}
			*/
			}
#endif // SS_V40

#ifndef SS_NOOVERFLOW
         SS_OverflowInvalidateCell(lpSS, ColEdit, RowEdit);
#endif

         HideCaret(hWndCtrl);
         fNoEraseBkgnd = lpSS->NoEraseBkgnd;

         if (!EscapeInProgress)
            {
            Edit_SetSel(hWndCtrl, 0, 0);
#ifdef WIN32
            Edit_ScrollCaret(hWndCtrl);
#endif
            if (CellType->Style & (ES_AUTOHSCROLL | ES_RIGHT | ES_CENTER |
                                   SSS_ALIGN_BOTTOM | SSS_ALIGN_VCENTER |
                                   ES_MULTILINE // RFW - 6/10/04 - 14348
#ifdef SS_V40
								           | SSS_TEXTORIENT_MASK
#endif
								           ))
					;

#ifdef SS_GP
            else if (CellType->Spec.Edit.dfWidth)
               ;
#endif

            else if (!fErase)
               lpSS->NoEraseBkgnd = TRUE;
            }

         if (hWndFocus == 0)
            SetFocus(lpBook->hWnd);
         else if (hWndFocus != (HWND)(-1))
            SetFocus(hWndFocus);

#ifndef SS_USEAWARE
         if (CellType->Style & (ES_AUTOHSCROLL | ES_RIGHT | ES_CENTER |
                                SSS_ALIGN_BOTTOM | SSS_ALIGN_VCENTER |
                                ES_MULTILINE // RFW - 6/10/04 - 14348
#ifdef SS_V40
								        | SSS_TEXTORIENT_MASK
#endif
								        ))
				;

#ifdef SS_GP
         else if (CellType->Spec.Edit.dfWidth)
            ;
#endif
         else if (!EscapeInProgress && !fErase)
            lpSS->NoEraseBkgnd = TRUE;
#endif

         if (fNoEraseBkgnd != (BOOL)lpSS->NoEraseBkgnd)
            {
            SS_COLORTBLITEM BackColorTblItem;
            SS_COLORTBLITEM ForeColorTblItem;

            SS_GetColorTblItem(lpSS, ColEdit, RowEdit,
                               &BackColorTblItem, &ForeColorTblItem);

            if (BackColorTblItem.Color != RGBCOLOR_WHITE &&
                BackColorTblItem.Color != RGBCOLOR_BLUE &&
                BackColorTblItem.Color != RGBCOLOR_RED &&
                BackColorTblItem.Color != RGBCOLOR_PINK &&
                BackColorTblItem.Color != RGBCOLOR_GREEN &&
                BackColorTblItem.Color != RGBCOLOR_CYAN &&
                BackColorTblItem.Color != RGBCOLOR_YELLOW &&
                BackColorTblItem.Color != RGBCOLOR_BLACK &&
                BackColorTblItem.Color != RGBCOLOR_DARKGRAY &&
                BackColorTblItem.Color != RGBCOLOR_DARKBLUE &&
                BackColorTblItem.Color != RGBCOLOR_DARKRED &&
                BackColorTblItem.Color != RGBCOLOR_DARKPINK &&
                BackColorTblItem.Color != RGBCOLOR_DARKGREEN &&
                BackColorTblItem.Color != RGBCOLOR_DARKCYAN &&
                BackColorTblItem.Color != RGBCOLOR_BROWN &&
                BackColorTblItem.Color != RGBCOLOR_PALEGRAY)
               lpSS->NoEraseBkgnd = fNoEraseBkgnd;
            }

#ifdef SS_V40
			if (lpBook->fEditOverflow || fIsCellSpan)
            lpSS->NoEraseBkgnd = FALSE;
#endif // SS_V40

			// RFW - 7/19/03 - 12271
			InvalidateCellIfNecessary(lpSS, ColEdit, RowEdit);

         if (lpSS->NoEraseBkgnd)
            SetWindowPos(hWndCtrl, 0, 0, 0, 0, 0, SWP_HIDEWINDOW |
                         SWP_NOREDRAW | SWP_NOZORDER);
         else
            SetWindowPos(hWndCtrl, 0, 0, 0, 0, 0, SWP_HIDEWINDOW |
                         SWP_NOZORDER);

         if (lpBook->Redraw)
            SS_UpdateWindow(lpBook);

         lpSS->NoEraseBkgnd = FALSE;
         break;

      case SS_TYPE_BUTTON:
      case SS_TYPE_CHECKBOX:
         if (!EscapeInProgress)
            {
#ifndef SS_NOCT_BUTTON
            if (CellType->Type == SS_TYPE_BUTTON)
               fChangeMade = SS_SetButtonData(hWndCtrl, lpSS, lpCell,
                                              CellType, ColEdit, RowEdit);
#endif

#ifndef SS_NOCT_CHECK
#ifndef SS_UTP
            if (CellType->Type == SS_TYPE_CHECKBOX)
               fChangeMade = SS_SetCheckBoxData(hWndCtrl, lpSS, lpCell,
                                                CellType, ColEdit, RowEdit);
#endif
#endif

            }

#ifdef SS_V40
			InvalidateMergedCell(lpSS, ColEdit, RowEdit);
#endif // SS_V40

         if (hWndFocus == 0)
            SetFocus(lpBook->hWnd);
         else if (hWndFocus != (HWND)(-1))
            SetFocus(hWndFocus);

         InvalidateRect(hWndCtrl, NULL, FALSE);
         UpdateWindow(hWndCtrl);

         ShowWindow(hWndCtrl, SW_HIDE);

         if (lpBook->Redraw)
            SS_UpdateWindow(lpBook);

         break;

      case SS_TYPE_PICTURE:
         lpSS->NoEraseBkgnd = TRUE;
         ShowWindow(hWndCtrl, SW_HIDE);

         if (lpBook->Redraw)
            SS_UpdateWindow(lpBook);

         lpSS->NoEraseBkgnd = FALSE;
         break;

      case SS_TYPE_DATE:
      case SS_TYPE_TIME:
      case SS_TYPE_INTEGER:
      case SS_TYPE_FLOAT:
#ifdef SS_V40
		case SS_TYPE_CURRENCY:
		case SS_TYPE_NUMBER:
		case SS_TYPE_PERCENT:
#endif // SS_V40
      case SS_TYPE_PIC:
#ifdef SS_V70
		case SS_TYPE_SCIENTIFIC:
#endif // SS_V70
         {
         BOOL fHideCaret = TRUE;

         fErase = FALSE;
         if (!EscapeInProgress)
            {
            if (lpSS->fFormulaMode)
               {
#ifndef SS_NOCALC
               CellType = &lpBook->FormulaCellType;
               hWndCtrl = SS_GetControlhWnd(lpSS, CellType->ControlID);

               if (hGlobalData = tbGlobalAlloc(GMEM_MOVEABLE | GMEM_ZEROINIT,
                                               (CellType->Spec.Edit.Len + 1)
                                               * sizeof(TCHAR)))
                  {
                  Data = (LPTSTR)tbGlobalLock(hGlobalData);
                  GetWindowText(hWndCtrl, Data, CellType->Spec.Edit.Len + 1);

                  HideCaret(hWndCtrl);
                  fHideCaret = FALSE;

// GAB 03/07/02                  if (Data[0] == '=')
                  if (Data[0] == _T('='))
                     {
                     #ifndef SS_OLDCALC
                     Data++;
                     #endif
                     if (SS_SetFormulaRange(lpSS,
                                            ColEdit, RowEdit,
                                            ColEdit, RowEdit,
                                            Data, TRUE))
                        SS_SendMsgCommand(lpBook, lpSS, SSN_USERFORMULA, FALSE);
                     }
                  else
                     {
							int iLenOld = SS_GetFormulaLen(lpSS, ColEdit, RowEdit);

                     if (SS_SetFormulaRange(lpSS,
                                            ColEdit, RowEdit,
                                            ColEdit, RowEdit,
                                            _T(""), TRUE))
								// RFW - 6/3/04 - 14290
								if (iLenOld)
									SS_SendMsgCommand(lpBook, lpSS, SSN_USERFORMULA, FALSE);
                     }

                  tbGlobalUnlock(hGlobalData);
                  tbGlobalFree(hGlobalData);
                  }
#endif
               }

            else if (CellType->Type == SS_TYPE_FLOAT)
               {
               if (fChangeMade = (BOOL)SendMessage(hWndCtrl, EM_GETMODIFY, 0, 0L))
                  {
#ifdef SS_USEAWARE
                  if (hWndFocus == 0)
                     SetFocus(hWnd);
                  else if (hWndFocus != -1)
                     SetFocus(hWndFocus);

                  fNoSetFocus = TRUE;

                  {                    // rdw
                  BOOL fNull  = (BOOL)SendMessage(hWndCtrl, EM_GETNULL, 0, 0L);
                  BOOL fValid = (BOOL)SendMessage(hWndCtrl, EM_CHECKVALIDITY, 0, 0L);
                  if (fNull || !fValid)
                     {
                     if (!fValid)
                        SS_SendMsgCommand(hWnd, NULL, EN_INVALIDDATA, FALSE);
                     *Buffer = 0;
                     }
                  else 
                     {
                     FloatGetValue(hWndCtrl, &dfValue);
                     fpDoubleToString(Buffer, dfValue, CellType->Spec.Float.Right,
// GAB 03/07/02                        '.', 0, FALSE, (CellType->Style & FS_MONEY) ? TRUE : FALSE, 
                        _T('.'), 0, FALSE, (CellType->Style & FS_MONEY) ? TRUE : FALSE, 
                        TRUE, FALSE, 0, NULL);      
                     }   
                  }
#else

                  FloatGetValue(hWndCtrl, &dfValue);

#endif

#ifdef	SPREAD_JPN
					   // SPRD017 Wei Feng 1997/09/26
						if ( !(dfValue >= CellType->Spec.Float.Min && dfValue <= CellType->Spec.Float.Max) )
                     SSx_SetCellData(lpSS, ColEdit, RowEdit, _T(""), 0, FALSE);
						else
#endif

                  {
                  SS_DATA DataItem;
                  _fmemset(&DataItem, '\0', sizeof(DataItem));
                  DataItem.bDataType = SS_TYPE_FLOAT;
                  DataItem.Data.dfValue = dfValue;
                  SSx_SetCellDataItem(lpSS, ColEdit, RowEdit, &DataItem, FALSE);
                  }
#ifndef SS_NOOVERFLOW
                  SS_OverflowInvalidateCell(lpSS, ColEdit,
                                            RowEdit);
#endif
                  }
               else
                  {
#ifndef SS_NOOVERFLOW
                  SS_OverflowAdjustNeighbor(lpSS, ColEdit,
                                            RowEdit);
#endif
                  }
               }

#ifdef SS_V40
            else if (CellType->Type == SS_TYPE_CURRENCY ||
                     CellType->Type == SS_TYPE_NUMBER ||
                     CellType->Type == SS_TYPE_PERCENT)
               {
               if (fChangeMade = (BOOL)SendMessage(hWndCtrl, EM_GETMODIFY, 0, 0L))
                  {
                  SS_DATA DataItem;

                  NumGetValue(hWndCtrl, &dfValue);
                  if (CellType->Type == SS_TYPE_PERCENT)
							{
							TCHAR szBuf[100];
							dfValue /= 100.0;
							// The following code was added to take care of an Intel floating point problem.
							StrPrintf(szBuf, _T("%.*f"), CellType->Spec.Percent.Right + 2, dfValue);
							StringToFloat(szBuf, &dfValue);
							}

                  _fmemset(&DataItem, '\0', sizeof(DataItem));
                  DataItem.bDataType = SS_TYPE_FLOAT;
                  DataItem.Data.dfValue = dfValue;
                  SSx_SetCellDataItem(lpSS, ColEdit, RowEdit, &DataItem, FALSE);
#ifndef SS_NOOVERFLOW
                  SS_OverflowInvalidateCell(lpSS, ColEdit, RowEdit);
#endif
                  }
               else
                  {
#ifndef SS_NOOVERFLOW
                  SS_OverflowAdjustNeighbor(lpSS, ColEdit, RowEdit);
#endif
                  }
               }
#endif // SS_V40

#ifdef SS_V70
            else if (CellType->Type == SS_TYPE_SCIENTIFIC)
					{
               if (fChangeMade = (BOOL)SendMessage(hWndCtrl, EM_GETMODIFY, 0, 0L))
                  {
                  dLen = GetWindowTextLength(hWndCtrl);

                  if (hGlobalData = tbGlobalAlloc(GHND, (dLen + 1) * sizeof(TCHAR)))
                     {
							SS_DATA  DataItem;
							SS_DATA  DataItemOld;
							LPSS_COL lpCol = SS_LockColItem(lpSS, ColEdit);
							LPSS_ROW lpRow = SS_LockRowItem(lpSS, RowEdit);
							double   dfValue;

                     Data = (LPTSTR)tbGlobalLock(hGlobalData);
                     _fmemset(Data, '\0', (dLen + 1) * sizeof(TCHAR));
                     GetWindowText(hWndCtrl, Data, dLen + 1);

							_fmemset(&DataItem, '\0', sizeof(DataItem));
							DataItem.bDataType = SS_TYPE_FLOAT;
							dfValue = SS_ScientificToFloat(Data, CellType);
							DataItem.Data.dfValue = dfValue;

							// RFW - 6/22/04 - 14570
							if (SS_GetDataStruct(lpSS, lpCol, lpRow, lpCell, &DataItemOld,
                                          ColEdit, RowEdit) &&
                                          DataItemOld.bDataType == SS_TYPE_FLOAT &&
                                          DataItemOld.Data.dfValue == DataItem.Data.dfValue)
								fChangeMade = FALSE;
							else
								{
								SSx_SetCellDataItem(lpSS, ColEdit, RowEdit, &DataItem, FALSE);
#ifndef SS_NOOVERFLOW
		                  SS_OverflowInvalidateCell(lpSS, ColEdit, RowEdit);
#endif

								if (!SS_ScientificCheckRange(CellType, Data, FALSE))
									SS_SendMsgCommand(lpBook, lpSS, SSN_INVALIDDATA, FALSE);
								}

							if (lpCol)
								SS_UnlockColItem(lpSS, ColEdit);
							if (lpRow)
								SS_UnlockRowItem(lpSS, RowEdit);

                     tbGlobalUnlock(hGlobalData);
                     tbGlobalFree(hGlobalData);
                     }
                  }
               else
                  {
#ifndef SS_NOOVERFLOW
                  SS_OverflowAdjustNeighbor(lpSS, ColEdit, RowEdit);
#endif
                  }
					}
#endif // SS_V70

            else
               {
               if (fChangeMade = (BOOL)SendMessage(hWndCtrl, EM_GETMODIFY, 0,
                                                   0L))
                  {
                  if (CellType->Type == SS_TYPE_PIC)
                     dLen = GetWindowTextLength(hWndCtrl);
                  else
                     dLen = 40;

                  if (hGlobalData = tbGlobalAlloc(GMEM_MOVEABLE |
                                                  GMEM_ZEROINIT, (dLen + 1)
                                                  * sizeof(TCHAR)))
                     {
                     Data = (LPTSTR)tbGlobalLock(hGlobalData);
                     _fmemset(Data, '\0', (dLen + 1) * sizeof(TCHAR));
#ifdef SS_USEAWARE                     
                     // rdw - check for invalid or blank here
                     {
                     BOOL fNull  = (BOOL)SendMessage(hWndCtrl, EM_GETNULL, 0, 0L);
                     BOOL fValid = (BOOL)SendMessage(hWndCtrl, EM_CHECKVALIDITY, 0, 0L);
                     if (fNull || !fValid)
                        {
                        if (!fValid)
                           SS_SendMsgCommand(hWnd, NULL, EN_INVALIDDATA, FALSE);
                        *Data = 0;
                        }
                     else 
                        GetWindowText(hWndCtrl, Data, dLen + 1);
                     }
#else
                     GetWindowText(hWndCtrl, Data, dLen + 1);
#ifdef	SPREAD_JPN
// SPRD017, Wei Feng 1997/09/26
							if (CellType->Type == SS_TYPE_INTEGER)
							{
								int i = _ttoi(Data);
								if (!(i >= CellType->Spec.Integer.Min && i <= CellType->Spec.Integer.Max))
								 *Data = 0;
							}
#endif
#endif
							// RFW - 6/18/04 - 14591
							if (CellType->Type == SS_TYPE_PIC && PicIsNULL(hWndCtrl))
								SSx_SetCellData(lpSS, ColEdit, RowEdit, NULL, 0, FALSE);
							else
								SSx_SetCellData(lpSS, ColEdit, RowEdit,
													 Data, (short)lstrlen(Data), FALSE);
#ifndef SS_NOOVERFLOW
                     SS_OverflowInvalidateCell(lpSS, ColEdit,
                                               RowEdit);
#endif

                     tbGlobalUnlock(hGlobalData);
                     tbGlobalFree(hGlobalData);
                     }
                  }
               else
                  {
#ifndef SS_NOOVERFLOW
//                  if (!lpCell || !lpCell->Data.bDataType)
                     SS_OverflowAdjustNeighbor(lpSS, ColEdit,
                                               RowEdit);
#endif
                  }
               }
            }

         else if (lpSS->fFormulaMode)
            {
            CellType = &lpBook->FormulaCellType;
            hWndCtrl = SS_GetControlhWnd(lpSS, CellType->ControlID);
            }

         else
            {
#ifndef SS_NOOVERFLOW
            SS_OverflowAdjustNeighbor(lpSS, ColEdit,
                                      RowEdit);
#endif
            }

         if (fHideCaret)
            HideCaret(hWndCtrl);

         if ((CellType->Type == SS_TYPE_DATE && (CellType->Style & DS_SPIN)) ||
            (CellType->Type == SS_TYPE_TIME && (CellType->Style & TS_SPIN)) ||
            (CellType->Type == SS_TYPE_INTEGER && (CellType->Style & IS_SPIN)))
            fErase = TRUE;

         fNoEraseBkgnd = FALSE;
         if (!EscapeInProgress)
            {
            Edit_SetSel(hWndCtrl, 0, 0);
#ifdef WIN32
            Edit_ScrollCaret(hWndCtrl);
#endif
            lpSS->NoEraseBkgnd = FALSE;

            if (((CellType->Type == SS_TYPE_INTEGER ||
#ifdef SS_V40
                  CellType->Type == SS_TYPE_CURRENCY ||
                  CellType->Type == SS_TYPE_NUMBER ||
                  CellType->Type == SS_TYPE_PERCENT ||
#endif // SS_V40
                  CellType->Type == SS_TYPE_FLOAT) &&
                ((CellType->Style & ES_CENTER) ||
                 (CellType->Style & ES_LEFTALIGN))) ||
                ((CellType->Type == SS_TYPE_DATE ||
                  CellType->Type == SS_TYPE_TIME ||
                  CellType->Type == SS_TYPE_PIC) &&
                 (CellType->Style & (ES_CENTER | ES_RIGHT))))
               ;

            else if (CellType->Style & (SSS_ALIGN_BOTTOM | SSS_ALIGN_VCENTER))
               ;

            else if (!lpSS->fFormulaMode && !fErase &&
                     ((CellType->Type != SS_TYPE_FLOAT &&
#ifdef SS_V40
							  CellType->Type != SS_TYPE_CURRENCY &&
							  CellType->Type != SS_TYPE_NUMBER &&
							  CellType->Type != SS_TYPE_PERCENT &&
#endif // SS_V40
                       CellType->Type != SS_TYPE_INTEGER &&
                       CellType->Type != SS_TYPE_DATE &&
                       CellType->Type != SS_TYPE_TIME) || fChangeMade))
               {
               fNoEraseBkgnd = TRUE;
#ifndef SS_USEAWARE
               lpSS->NoEraseBkgnd = TRUE;
#endif
               }
            }

         if (!fNoSetFocus)
            {
            if (hWndFocus == 0)
               SetFocus(lpBook->hWnd);
            else if (hWndFocus != (HWND)(-1))
               SetFocus(hWndFocus);
            }

#ifdef SS_V40
			if (lpBook->fEditOverflow || fIsCellSpan)
            lpSS->NoEraseBkgnd = FALSE;
#endif // SS_V40

			// RFW - 7/19/03 - 12271
			InvalidateCellIfNecessary(lpSS, ColEdit, RowEdit);

         ShowWindow(hWndCtrl, SW_HIDE);

			// RFW - 7/1/04 - 14735
#ifdef SS_V70
         if (CellType->Type == SS_TYPE_SCIENTIFIC && (CellType->Style & ES_CENTER))
				{
				BOOL fRedraw = lpSS->lpBook->Redraw;
				lpSS->lpBook->Redraw = FALSE;
				SS_InvalidateCellRange(lpSS, ColEdit, RowEdit, ColEdit, RowEdit);
				lpSS->lpBook->Redraw = fRedraw;
				}
#endif

#ifdef SS_V40
			if (!lpBook->fEditOverflow)
#endif // SS_V40
	         if (fNoEraseBkgnd)
		         lpSS->NoEraseBkgnd = TRUE;

         if (GetUpdateRect(lpBook->hWnd, NULL, FALSE))
            lpSS->NoEraseBkgnd = FALSE;

         if (lpBook->Redraw || lpSS->fFormulaMode)
            SS_UpdateWindow(lpBook);

#ifdef SS_V40
			if (!lpBook->fEditOverflow)
#endif // SS_V40
	         if (!fErase)
		         lpSS->NoEraseBkgnd = TRUE;

         MoveWindow(hWndCtrl, 0, 0, 0, 0, FALSE);
         lpSS->NoEraseBkgnd = FALSE;

#ifdef SS_V40
			InvalidateMergedCell(lpSS, ColEdit, RowEdit);
#endif // SS_V40
         }
         break;

#ifndef SS_NOCT_COMBO
      case SS_TYPE_COMBOBOX:
         if (!EscapeInProgress)
            {
            dLen = (short)SendMessage(hWndCtrl, WM_GETTEXTLENGTH, 0, 0L);

            if (hText = GlobalAlloc(GHND, (dLen + 1) * sizeof(TCHAR)))
               {
// 21093 -scl
//               short nCurSel = (short)SendMessage(hWndCtrl, CB_GETCURSEL, 0, 0);
               long nCurSel = (long)SendMessage(hWndCtrl, CB_GETCURSEL, 0, 0);

               lpText = (LPTSTR)GlobalLock(hText);

               SendMessage(hWndCtrl, WM_GETTEXT, dLen + 1, (LPARAM)lpText);

               lpItems = (LPTSTR)tbGlobalLock(CellType->Spec.ComboBox.hItems);
               fChangeMade = TRUE;

               if (hGlobalData = SSx_GetData(lpSS, NULL, NULL, lpCell,
                                             CellType, NULL,
                                             ColEdit,
                                             RowEdit, FALSE))
                  {
                  Data = (LPTSTR)tbGlobalLock(hGlobalData);

						if (*Data == '\0' && *lpText == '\0')
							fChangeMade = FALSE;
                  else if (*Data && _ftcscmp(lpText, Data) == 0)
                     {
							/* RFW - 8/4/06 - 19129
                     if (Data[lstrlen(Data) + 1] &&
                         StringToLong(&Data[lstrlen(Data) + 1]) != (long)nCurSel)
							*/
							if (nCurSel != -1 &&
                         Data[lstrlen(Data) + 1] &&
                         StringToLong(&Data[lstrlen(Data) + 1]) != (long)nCurSel)
                        ;
                     else
                        fChangeMade = FALSE;
                     }
                  }
               else
                  fChangeMade = dLen != 0; // BJO 26Mar96 TEL1844

               if (fChangeMade)
                  {
                  Sel = -1;

// fix for bug 9632 -scl
                  if( CellType->Spec.ComboBox.hWndDropDown != NULL )
// 21093 -scl
//                    Sel = (short)SendMessage(hWndCtrl, CB_GETCURSEL, 0, 0);
                    Sel = (long)SendMessage(hWndCtrl, CB_GETCURSEL, 0, 0);
                  else
                  for (i = 0; i < CellType->Spec.ComboBox.dItemCnt; i++)
                     {
                     /* RFW - 7/19/99 - GIC 8997
                     if (i == nCurSel && _ftcscmp(lpText, lpItems) == 0)
                     */
                     if ((i == nCurSel || -1 == nCurSel) &&
                         _ftcscmp(lpText, lpItems) == 0)
                        {
                        Sel = i;
                        break;
                        }

                     lpItems += lstrlen(lpItems) + 1;
                     }

                  if (!*lpText && Sel == -1)
                     SSx_SetCellData(lpSS, ColEdit, RowEdit, NULL, 0, FALSE);
                  else
                     {
                     wsprintf(Buffer, _T("%d"), Sel);

                     if (hGlobalData = tbGlobalAlloc(GMEM_MOVEABLE |
                                       GMEM_ZEROINIT, (lstrlen(lpText) +
                                       1 + lstrlen(Buffer) + 1) * sizeof(TCHAR)))
                        {
                        SS_DATA DataItem;

                        Data = (LPTSTR)tbGlobalLock(hGlobalData);

                        lstrcpy(Data, lpText);
                        lstrcpy(&Data[lstrlen(Data) + 1], Buffer);
                        tbGlobalUnlock(hGlobalData);

                        _fmemset(&DataItem, '\0', sizeof(DataItem));
                        DataItem.bDataType = SS_TYPE_EDIT;
                        DataItem.Data.hszData = hGlobalData;
                        SSx_SetCellDataItem(lpSS, ColEdit, RowEdit,
                                            &DataItem, FALSE);
                        }
                     }

#ifndef SS_NOOVERFLOW
                  SS_OverflowInvalidateCell(lpSS, ColEdit,
                                            RowEdit);
#endif
                  }

               tbGlobalUnlock(CellType->Spec.ComboBox.hItems);

               GlobalUnlock(hText);
               GlobalFree(hText);
               }
            }
#ifdef SS_V40
			InvalidateMergedCell(lpSS, ColEdit, RowEdit);
#endif // SS_V40

         if (hWndFocus == 0)
            SetFocus(lpBook->hWnd);
         else if (hWndFocus != (HWND)-1)
            SetFocus(hWndFocus);
         else
            SendMessage(hWndCtrl, CB_SHOWDROPDOWN, FALSE, 0L);

         InvalidateRect(hWndCtrl, NULL, FALSE);
         UpdateWindow(hWndCtrl);
         ShowWindow(hWndCtrl, SW_HIDE);
         SendMessage(hWndCtrl, SS_CBM_SETDROPDOWNHWND, 0, 0);

         SS_UpdateWindow(lpBook);

			// RFW - 12/9/08 - 23836
#if SS_V80
			if (!lpSS->lpBook->fPaintingToScreen)
				SS_InvalidateActiveHeaders(lpSS);
#endif // SS_V80

         break;
#endif

#ifdef SS_V80
         case SS_TYPE_CUSTOM:
           {
             SS_DATA DataItem = {0};
             BOOL bRedraw = lpSS->lpBook->Redraw;

             if( EscapeInProgress )
               SS_CT_CancelEditing(lpSS, CellType, hWndCtrl, ColEdit, RowEdit);
			    else if (!SS_CT_StopEditing(lpSS, CellType, hWndCtrl, ColEdit, RowEdit))
			    {
				    lpBook->EditModeTurningOff = FALSE;
				    return TRUE;
			    }
#if SS_OCX   // in case it changed in CTStopEditing or CTCancelEditing event
             hWndCtrl = SS_GetControlhWnd(lpSS, CellType->ControlID);
#endif
             if( !EscapeInProgress )
             {
                if( !SS_CT_GetEditorValue(lpSS, CellType, hWndCtrl, ColEdit, RowEdit, &DataItem) )
                {  // send WM_GETTEXT to get value
                  LRESULT len;
               
#if SS_OCX        // in case it changed in CTGetEditorValue event
                  hWndCtrl = SS_GetControlhWnd(lpSS, CellType->ControlID);
#endif
                  len = SendMessage(hWndCtrl, WM_GETTEXTLENGTH, 0, 0L);
                  if (len > 0 && (DataItem.Data.hszData = tbGlobalAlloc(GMEM_MOVEABLE | GMEM_ZEROINIT,
                                   ++len * sizeof(TCHAR))))
                  {
                     DataItem.bDataType = SS_DATATYPE_EDIT;
                     Data = (LPTSTR)tbGlobalLock(DataItem.Data.hszData);
                     _fmemset(Data, '\0', len * sizeof(TCHAR));
                     SendMessage(hWndCtrl, WM_GETTEXT, (WPARAM)len, (LPARAM)Data);
                     tbGlobalUnlock(DataItem.Data.hszData);
                  }
                }
                if( SS_CT_IsValid(lpSS, CellType, hWndCtrl, ColEdit, RowEdit, &DataItem) )
                  SSx_SetCellDataItem(lpSS, ColEdit, RowEdit, &DataItem, FALSE);
                else
                { 
                  // TODO: fire edit error
                  SSx_FreeData(&DataItem);
                }
             }
#if SS_OCX   // in case it changed in CTIsValid event
             hWndCtrl = SS_GetControlhWnd(lpSS, CellType->ControlID);
#endif
			    lpSS->lpBook->Redraw = FALSE;
             InvalidateMergedCell(lpSS, ColEdit, RowEdit);
			    lpSS->lpBook->Redraw = TRUE;
#ifndef SS_NOOVERFLOW
             //SS_OverflowAdjustNeighbor(lpSS, ColEdit,RowEdit);
             SS_OverflowInvalidateCell(lpSS, ColEdit, RowEdit);
#endif

             if (hWndFocus == 0)
               SetFocus(lpBook->hWnd);
             else if (hWndFocus != (HWND)(-1))
               SetFocus(hWndFocus);

             lpSS->lpBook->Redraw = FALSE; // force invalidate
			    SS_InvalidateCell(lpSS, ColEdit, RowEdit);
             lpSS->lpBook->Redraw = bRedraw;
			    if (GetParent(hWndCtrl) == lpBook->hWnd)
				    MoveWindow(hWndCtrl, 0, 0, 0, 0, FALSE);
/*
             ctl.lpDispatch = SS_GetControlDispatch(lpSS, hWndCtrl);
             if( ctl.lpDispatch )
                fpVBSetControlProperty(&ctl, 0, 0, _T("Visible"), VT_BOOL);
             else
*/
                ShowWindow(hWndCtrl, SW_HIDE);
             SS_CT_UnsubclassEditor(lpSS, hWndCtrl);

             if (bRedraw)
                SS_UpdateWindow(lpBook);
           }
           break;
#endif
#ifdef SS_V80R
      case SS_TYPE_CUSTOM:
         if (!EscapeInProgress)
            {
            if (CellType->Type == SS_TYPE_BUTTON)
               fChangeMade = SS_SetButtonData(hWndCtrl, lpSS, lpCell,
                                              CellType, ColEdit, RowEdit);
            }

#ifdef SS_V40
			InvalidateMergedCell(lpSS, ColEdit, RowEdit);
#endif // SS_V40

         if (hWndFocus == 0)
            SetFocus(lpBook->hWnd);
         else if (hWndFocus != (HWND)(-1))
            SetFocus(hWndFocus);

         InvalidateRect(hWndCtrl, NULL, FALSE);
         UpdateWindow(hWndCtrl);

         ShowWindow(hWndCtrl, SW_HIDE);

         if (lpBook->Redraw)
            SS_UpdateWindow(lpBook);

         break;
#endif // SS_V80

      }

   lpBook->EditModeOn = FALSE;
	// Just in case the user reset the Book, we had better get the
	// active sheet again.
	lpSS = SS_BookLockActiveSheet(lpBook);

   if (fChangeMade && !EscapeInProgress)
      {
		HWND hWndSS = lpBook->hWnd;

      lpSS->fRowModeChangeMade = TRUE;

      SS_SendMsgCoords(lpSS, SSM_DATACHANGE, GetDlgCtrlID(hWndSS), ColEdit, RowEdit);
      if (SS_IsDestroyed(hWndSS))
         return 0;
      }

#ifdef SS_OLDCALC
   if (fChangeMade && !EscapeInProgress && lpSS->lpBook->CalcAuto && !lpSS->fFormulaMode)
      {
      SS_CalcDependencies(lpSS, ColEdit,
                          RowEdit);
      }
#elif !defined(SS_NOCALC)
   if (fChangeMade && !EscapeInProgress && !lpSS->fFormulaMode)
      {
		SS_COORD lColOut = ColEdit;
		SS_COORD lRowOut = RowEdit;

		SS_AdjustCellCoordsOut(lpSS, &lColOut, &lRowOut);

      CalcMarkDependForEval(&lpSS->CalcInfo, lColOut, lRowOut);
      if( lpBook->CalcAuto )
        CalcEvalNeededCells(&lpBook->CalcInfo);
      }
#endif

#ifdef SS_UTP
   if (lpSS->fUseEditModeShadow)
      SS_ClearEditModeShadow(lpSS);
#endif

   lpSS->fFormulaMode = FALSE;

   lpBook->EditModeTurningOff = FALSE;

   lpSS->fSetActiveCellCalled = FALSE;
   lpBook->fSetFocusWhileMsgBeingSent = FALSE;

   if (!lpBook->fDontSendEditModeMsg)    // RFW - 8/23/94
#ifdef SS_USE16BITCOORDS
      SS_SendMsg(hWnd, SSM_EDITMODEOFF, (short)fChangeMade,
                 MAKELONG(ColEdit - lpSS->Col.HeaderCnt + 1,
                 RowEdit - lpSS->Row.HeaderCnt + 1));
#else
      {
      SS_EDITMODEOFF EditModeOff;

      EditModeOff.fChangeMade = fChangeMade;
      EditModeOff.Col = ColEdit;
      EditModeOff.Row = RowEdit;
		SS_AdjustCellCoordsOut(lpSS, &EditModeOff.Col, &EditModeOff.Row);
      SS_SendMsg(lpBook, lpSS, SSM_EDITMODEOFF, GetDlgCtrlID(lpBook->hWnd),
                 (LPARAM)(LPSS_EDITMODEOFF)&EditModeOff);
      }
#endif

   if (SS_IsDestroyed(lpBook->hWnd))
      return (FALSE);

   if (lpBook->fProcessingKillFocus &&
       lpBook->fSetFocusWhileMsgBeingSent && lpBook->hWnd == GetFocus())
      SS_HighlightCell(lpSS, TRUE);

   lpBook->fSetFocusWhileMsgBeingSent = FALSE;

   if (lpCell)
      SS_UnlockCellItem(lpSS, ColEdit, RowEdit);

   if (lpSS->fSetActiveCellCalled)
      {
      lpSS->fSetActiveCellCalled = FALSE;
      SS_HighlightCell(lpSS, TRUE);
      return (FALSE);
      }
   }

#ifdef SS_V80
/*
if (!lpSS->lpBook->fPaintingToScreen && lpSS->lpBook->fEditModePermanent)
*/
// RFW - 12/9/08 - 23836
// RFW - 12/18/08 - 24178
if (!lpSS->lpBook->fPaintingToScreen)
   SS_InvalidateActiveHeaders(lpSS);
#endif

return (TRUE);
}


static BOOL SS_SetButtonData(HWND hWndCtrl, LPSPREADSHEET lpSS, LPSS_CELL lpCell,
                             LPSS_CELLTYPE lpCellType, SS_COORD ColEdit, SS_COORD RowEdit)
{
TBGLOBALHANDLE hData;
TBGLOBALHANDLE hDataOld;
LPTSTR         lpData;
LPTSTR         lpszDataOld;
BOOL           fButtonState;
BOOL           fChangeMade = FALSE;

if (lpCellType && lpCellType->Spec.Button.ButtonType == SUPERBTN_TYPE_2STATE)
   {
   fButtonState = (BOOL)SendMessage(hWndCtrl, SBM_GETBUTTONSTATE, 0, 0L);

   if (hData = tbGlobalAlloc(GMEM_MOVEABLE | GMEM_ZEROINIT,
                             (1L + 1L) * sizeof(TCHAR)))
      {
      lpData = (LPTSTR)tbGlobalLock(hData);
      lpData[0] = '0' + (fButtonState ? 1 : 0);

      fChangeMade = TRUE;

      if (hDataOld = SSx_GetData(lpSS, NULL, NULL, lpCell, lpCellType, NULL,
                                 ColEdit, RowEdit, FALSE))
         {
         lpszDataOld = (LPTSTR)tbGlobalLock(hDataOld);

         if (*lpszDataOld && _ftcsicmp(lpData, lpszDataOld) == 0)
            fChangeMade = FALSE;

         tbGlobalUnlock(hDataOld);
         }

      SSx_SetCellData(lpSS, ColEdit, RowEdit, lpData,
                      (short)lstrlen(lpData), FALSE);

      tbGlobalUnlock(hData);
      tbGlobalFree(hData);
      }
   }

return (fChangeMade);
}


static BOOL SS_SetCheckBoxData(HWND hWndCtrl, LPSPREADSHEET lpSS, LPSS_CELL lpCell,
                               LPSS_CELLTYPE lpCellType, SS_COORD ColEdit, SS_COORD RowEdit)
{
TBGLOBALHANDLE hData;
TBGLOBALHANDLE hDataOld;
LPTSTR         lpData;
LPTSTR         lpszDataOld;
WORD           wButtonState;
BOOL           fChangeMade = FALSE;

wButtonState = (WORD)SendMessage(hWndCtrl, BM_GETCHECK, 0, 0L);

if (hData = tbGlobalAlloc(GMEM_MOVEABLE | GMEM_ZEROINIT,
                          (1L + 1L) * sizeof(TCHAR)))
   {
   lpData = (LPTSTR)tbGlobalLock(hData);
   lpData[0] = '0' + (TCHAR)wButtonState;

   fChangeMade = TRUE;

   if (hDataOld = SSx_GetData(lpSS, NULL, NULL, lpCell, lpCellType, NULL,
                              ColEdit, RowEdit, FALSE))
      {
      lpszDataOld = (LPTSTR)tbGlobalLock(hDataOld);

      if (*lpszDataOld && _ftcsicmp(lpData, lpszDataOld) == 0)
         fChangeMade = FALSE;

      tbGlobalUnlock(hDataOld);
      }

   SSx_SetCellData(lpSS, ColEdit, RowEdit, lpData,
                   (short)lstrlen(lpData), FALSE);
   tbGlobalUnlock(hData);
   tbGlobalFree(hData);
   }

return (fChangeMade);
}


static void SSx_TypeEditOn(hWndCtrl, lpSS, CellType, hData, fSetSel)

HWND           hWndCtrl;
LPSPREADSHEET  lpSS;
LPSS_CELLTYPE  CellType;
TBGLOBALHANDLE hData;
BOOL           fSetSel;
{
LPTSTR         Data;
short          Len = 0;

if (hData)
   {
   Data = (LPTSTR)tbGlobalLock(hData);
   SetWindowText(hWndCtrl, Data);
   Len = lstrlen(Data);
   tbGlobalUnlock(hData);
   }

#ifndef SS_USEAWARE
else if (CellType->Style & (ES_AUTOHSCROLL | ES_MULTILINE))
   SetWindowText(hWndCtrl, _T(" "));
#endif

else
   SetWindowText(hWndCtrl, _T(""));

SendMessage(hWndCtrl, EM_SETMODIFY, 0, 0L);
lpSS->fEditModeDataChange = FALSE;

#ifndef SS_USEAWARE
if ((CellType->Style & (ES_AUTOHSCROLL | ES_MULTILINE)) && !hData)
   SetWindowText(hWndCtrl, _T(""));
#endif

if (fSetSel && Len)
   {
   if (lpSS->lpBook->fEditModePermanent || fSetSel == 2)
      {
      Edit_SetSel(hWndCtrl, 0, Len);
#ifdef WIN32
      Edit_ScrollCaret(hWndCtrl);
#endif
      }
   else
      {
      Edit_SetSel(hWndCtrl, Len, Len);
#ifdef WIN32
      Edit_ScrollCaret(hWndCtrl);
#endif
      }
   }
}

#ifndef SS_NOCT_CHECK

void SS_CheckBoxSetPict(hWnd, hWndCtrl, hPictName, wPictureType)

HWND           hWnd;
HWND           hWndCtrl;
TBGLOBALHANDLE hPictName;
short          wPictureType;
{
GLOBALHANDLE   hGlobal;
LPCHECKBOX     lpCheckBox;
LPTSTR         lpData;

if (hPictName)
   {
   lpCheckBox = CheckBoxGetPtr(hWndCtrl, &hGlobal);
   if (lpCheckBox->fUseDefPicts)
      {
      _fmemset(&lpCheckBox->Picts, '\0', sizeof(CHECKBOXPICTS));
      lpCheckBox->fUseDefPicts = FALSE;
      }

   if (wPictureType & BT_HANDLE)
      CheckBoxSetPict(0, 0, &lpCheckBox->Picts, wPictureType, (LPARAM)hPictName);
   else
      {
      lpData = (LPTSTR)tbGlobalLock(hPictName);
      CheckBoxSetPict(hWndCtrl, GetWindowInstance(hWnd),
                      &lpCheckBox->Picts, wPictureType, (LPARAM)lpData);
      tbGlobalUnlock(hPictName);
      }

   GlobalUnlock(hGlobal);
   }
else
   SendMessage(hWndCtrl, BM_SETPICT, wPictureType, 0L);
}

#endif


BOOL SS_SetEditModeOnData(LPSPREADSHEET lpSS)
{
TBGLOBALHANDLE hData;
LPTSTR         Data;
HWND           hWndCtrl;
LPSS_CELL      lpCell;
LPSS_CELLTYPE  CellType;
SS_CELLTYPE    CellTypeTemp;
LPTSTR         lpItems;
BOOL           fButtonState;
SS_COORD       ColEdit, RowEdit;
short          dSel;
short          i;

SS_GetCellBeingEdited(lpSS, &ColEdit, &RowEdit);

lpCell = SS_LockCellItem(lpSS, ColEdit, RowEdit);

CellType = SS_RetrieveCellType(lpSS, &CellTypeTemp, lpCell,
                               ColEdit, RowEdit);

if (CellType->ControlID)
   hWndCtrl = SS_GetControlhWnd(lpSS, CellType->ControlID);

switch (CellType->Type)
   {
   case SS_TYPE_EDIT:
      hData = SS_GetData(lpSS, CellType, ColEdit,
                         RowEdit, FALSE);

      SSx_TypeEditOn(hWndCtrl, lpSS, CellType, hData, TRUE);
      break;

#ifndef SS_NOCT_BUTTON
   case SS_TYPE_BUTTON:
      fButtonState = 0;

      if (hData = SS_GetData(lpSS, CellType, ColEdit,
                             RowEdit, FALSE))
         {
         Data = (LPTSTR)tbGlobalLock(hData);

         if (Data[0] == '1')
            fButtonState = 1;

         tbGlobalUnlock(hData);
         }

      if (CellType->Spec.Button.ButtonType == SUPERBTN_TYPE_2STATE)
         SendMessage(hWndCtrl, SBM_SETBUTTONSTATE, (short)fButtonState, 0L);

      break;
#endif

#ifndef SS_NOCT_CHECK
   case SS_TYPE_CHECKBOX:
      fButtonState = 0;

      if (hData = SS_GetData(lpSS, CellType, ColEdit,
                             RowEdit, FALSE))
         {
         Data = (LPTSTR)tbGlobalLock(hData);

         if ((CellType->Style & 0x0F) == BS_3STATE ||
             (CellType->Style & 0x0F) == BS_AUTO3STATE)
            {
            if (Data[0] == '1')
               fButtonState = 1;
            else if (Data[0] == '2')
               fButtonState = 2;
            }
         else
            {
            if (!Data[0] || (!Data[1] && Data[0] == '0'))
               fButtonState = 0;
            else
               fButtonState = 1;
            }

         tbGlobalUnlock(hData);
         }

      SendMessage(hWndCtrl, BM_SETCHECK, (short)fButtonState, 0L);

#ifdef SS_UTP
      _SpreadSetCellDirtyFlag(lpSS->lpBook->hWnd, ColEdit, RowEdit,
                              -1);
#endif
      break;
#endif

#ifndef SS_NOCT_COMBO
   case SS_TYPE_COMBOBOX:
      SendMessage(hWndCtrl, CB_RESETCONTENT, 0, 0L);

      if (CellType->Spec.ComboBox.hItems)
         {
         lpItems = (LPTSTR)tbGlobalLock(CellType->Spec.ComboBox.hItems);

         for (i = 0; i < CellType->Spec.ComboBox.dItemCnt; i++)
            {
            // Refer to bug GRB806 for use of CB_ADDSTRING_WIN32S
            #ifdef SS_WIN32S
            SendMessage(hWndCtrl, CB_ADDSTRING_WIN32S, 0, (LPARAM)lpItems);
            #else
            SendMessage(hWndCtrl, CB_ADDSTRING, 0, (LPARAM)lpItems);
            #endif
            lpItems += lstrlen(lpItems) + 1;
            }

         tbGlobalUnlock(CellType->Spec.ComboBox.hItems);

         SendMessage(hWndCtrl, CB_SETCURSEL, 0, 0L);

         if (hData = SS_GetData(lpSS, CellType, ColEdit,
                                RowEdit, FALSE))
            {
            Data = (LPTSTR)tbGlobalLock(hData);
            SetWindowText(hWndCtrl, Data);

            if (lstrlen(Data))
               {
               dSel = StringToInt(&Data[lstrlen(Data) + 1]);
               SendMessage(hWndCtrl, CB_SETCURSEL, dSel, 0L);
               }

            tbGlobalUnlock(hData);
            }
         }

      break;
#endif

   case SS_TYPE_DATE:
   case SS_TYPE_TIME:
   case SS_TYPE_INTEGER:
   case SS_TYPE_FLOAT:
#ifdef SS_V40
   case SS_TYPE_CURRENCY:
   case SS_TYPE_NUMBER:
   case SS_TYPE_PERCENT:
#endif // SS_V40
   case SS_TYPE_PIC:
#ifdef SS_V70
	case SS_TYPE_SCIENTIFIC:
#endif // SS_V70
      if (!lpSS->fFormulaMode)
         {
         if (hData = SS_GetData(lpSS, CellType, ColEdit,
                                RowEdit, FALSE))
            {
            Data = (LPTSTR)tbGlobalLock(hData);
            SetWindowText(hWndCtrl, Data);

            tbGlobalUnlock(hData);
            }

         else
            SetWindowText(hWndCtrl, _T(""));

         SendMessage(hWndCtrl, EM_SETMODIFY, 0, 0L);
         }

      break;

#if SS_V80
  case SS_TYPE_CUSTOM:
    if( !lpSS->fFormulaMode )
      SS_CT_SetEditorValue(lpSS, CellType, ColEdit, RowEdit);
    break;
#endif
   }

if (lpCell)
   SS_UnlockCellItem(lpSS, ColEdit, RowEdit);

return (TRUE);
}


BOOL SS_GetCellBeingEdited(LPSPREADSHEET lpSS, LPSS_COORD lpColEdit, LPSS_COORD lpRowEdit)
{
return (SS_GetActualCell(lpSS, lpSS->Col.CurAt, lpSS->Row.CurAt, lpColEdit, lpRowEdit, NULL));
}


void InvalidateCellIfNecessary(LPSPREADSHEET lpSS, SS_COORD ColEdit, SS_COORD RowEdit)
{
#ifdef SS_V35
if (SS_GetCellNote(lpSS, ColEdit, RowEdit, NULL) &&
	 ColEdit >= lpSS->Col.HeaderCnt && RowEdit >= lpSS->Row.HeaderCnt &&
	 (SS_CELLNOTEINDICATOR_SHOWANDFIREEVENT == lpSS->lpBook->nCellNoteIndicator ||
	  SS_CELLNOTEINDICATOR_SHOWANDDONOTFIREEVENT == lpSS->lpBook->nCellNoteIndicator))
	{
	RECT     Rect;
	int      x, y, cx, cy;
	#ifdef SS_V40
	SS_SELBLOCK Block;
	SS_COORD lSpanNumCols, lSpanNumRows;
	#endif // SS_V40

	SS_GetClientRect(lpSS->lpBook, &Rect);

	#ifdef SS_V40
	/* RFW - 6/23/04 - 14233
	if (SS_GetCellSpan(lpSS, ColEdit, RowEdit, NULL, NULL, &lSpanNumCols, &lSpanNumRows))
	*/
	if (SS_SpanCalcBlock(lpSS, ColEdit, RowEdit, &Block))
		{
		lSpanNumRows = Block.LR.Row - Block.UL.Row + 1;
		lSpanNumCols = Block.LR.Col - Block.UL.Col + 1;

		SS_GetCellRangeCoord(lpSS, lpSS->Col.UL, lpSS->Row.UL, ColEdit, RowEdit,
									ColEdit + lSpanNumCols - 1, RowEdit + lSpanNumRows - 1, &Rect, &x, &y, &cx, &cy);
		}
	else
	#endif // SS_V40
		SS_GetCellCoord(lpSS, lpSS->Col.UL, lpSS->Row.UL, ColEdit, RowEdit, &x, &y, &cx, &cy);

	if (x + cx > Rect.right)
		cx = Rect.right - x;

	if (y + cy > Rect.bottom)
		cy = Rect.bottom - y;

	SetRect(&Rect, x, y, x + cx, y + cy);
	InvalidateRect(lpSS->lpBook->hWnd, &Rect, TRUE);
	}
#endif // SS_V40
}


#ifdef SS_V40
BOOL InvalidateMergedCell(LPSPREADSHEET lpSS, SS_COORD ColEdit, SS_COORD RowEdit)
{
LPSS_ROW lpRow;
LPSS_COL lpCol;
BOOL     fRet = FALSE;

lpCol = SS_LockColItem(lpSS, ColEdit);
lpRow = SS_LockRowItem(lpSS, RowEdit);

if ((lpRow && lpRow->bMerge) || (lpCol && lpCol->bMerge))
	{
	SS_InvalidateCell(lpSS, ColEdit, RowEdit);
	fRet = TRUE;
	}

if (lpCol)
	SS_UnlockColItem(lpSS, ColEdit);
if (lpRow)
	SS_UnlockRowItem(lpSS, RowEdit);

return (fRet);
}
#endif // SS_V40
