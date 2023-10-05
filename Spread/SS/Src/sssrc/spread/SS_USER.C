/*********************************************************
* SS_USER.C
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
#include <stdio.h>
#include <stddef.h>
#include <stdlib.h>
#include <string.h>
#include <process.h>
#include <math.h>
#include "..\..\..\..\fplibs\fptools\src\fptools.h"
// this define has been added to the Spread 3.5 projects
//#ifdef SS_V35
//#define TT_V2
//#endif
#include "..\..\..\..\fplibs\fptools\src\texttip.h"
#include "spread.h"
#include "..\..\..\..\fplibs\fptools\src\fphdc.h"
#include "ss_alloc.h"
#include "ss_bool.h"
#include "ss_calc.h"
#include "ss_cell.h"
#include "ss_col.h"
#include "ss_data.h"
#include "ss_dde.h"
#include "ss_doc.h"
#include "ss_draw.h"
#include "ss_emode.h"
#include "ss_font.h"
#include "ss_main.h"
#include "ss_row.h"
#include "ss_scbar.h"
#include "ss_scrol.h"
#include "ss_span.h"
#include "ss_type.h"
#include "ss_user.h"
#include "ss_virt.h"
#include "ss_win.h"
#include "..\calc\calc.h"
#include "..\calc\cal_expr.h"
#ifdef SS_V70
#include "ss_book.h"
#include "..\..\..\fplibs\CxImage\CxImage\fpimages.h"
#include "ss_spict.h"
#endif // SS_V70

#ifdef SS_V35
#include "..\edit\editdate\calendar.h"
#endif

extern HANDLE hDynamicInst;


#ifdef SS_USE16BITCOORDS
LONG DLLENTRY SSGetTopLeftCell(hWnd)

HWND          hWnd;
{
LPSPREADSHEET lpSS;
long          Location;

lpSS = SS_Lock(hWnd);
Location = MAKELONG(lpSS->Col.UL, lpSS->Row.UL);
SS_Unlock(hWnd);

return (Location);
}


LONG DLLENTRY SSGetBottomRightCell(hWnd)

HWND          hWnd;
{
LPSPREADSHEET lpSS;
long          Location;

lpSS = SS_Lock(hWnd);
Location = MAKELONG(lpSS->Col.LR, lpSS->Row.LR);
SS_Unlock(hWnd);

return (Location);
}

#else


BOOL DLLENTRY SSGetTopLeftCell(HWND hWnd, LPSS_COORD lpCol, LPSS_COORD lpRow)
{
LPSPREADSHEET lpSS;
BOOL          fRet;

lpSS = SS_SheetLock(hWnd);
fRet = SS_GetTopLeftCell(lpSS, lpCol, lpRow);
SS_AdjustCellCoordsOut(lpSS, lpCol, lpRow);
SS_SheetUnlock(hWnd);
return fRet;
}


BOOL SS_GetTopLeftCell(LPSPREADSHEET lpSS, LPSS_COORD lpCol,
                       LPSS_COORD lpRow)
{
if (lpCol)
   *lpCol = min(SS_GetColCnt(lpSS) - 1, lpSS->Col.UL);
if (lpRow)
   *lpRow = min(SS_GetRowCnt(lpSS) - 1, lpSS->Row.UL);
return TRUE;
}


BOOL DLLENTRY SSGetBottomRightCell(HWND hWnd, LPSS_COORD lpCol,
                                   LPSS_COORD lpRow)
{
LPSPREADSHEET lpSS;
BOOL          fRet;

lpSS = SS_SheetLock(hWnd);
fRet = SS_GetBottomRightCell(lpSS, lpCol, lpRow);
SS_AdjustCellCoordsOut(lpSS, lpCol, lpRow);
SS_SheetUnlock(hWnd);
return fRet;
}

BOOL SS_GetBottomRightCell(LPSPREADSHEET lpSS, LPSS_COORD lpCol,
                           LPSS_COORD lpRow)
{
BOOL fRet = TRUE;

#ifdef SS_UTP

if (lpCol)
   *lpCol = SS_GetRightVisCell(lpSS, lpSS->Col.UL);

if (lpRow)
   *lpRow = SS_GetBottomVisCell(lpSS, lpSS->Row.UL);

#else

if (lpCol)
	/* RFW - 16317 - 6/13/05
   *lpCol = lpSS->Col.LR;
	*/
	*lpCol = SS_GetRightCell(lpSS, lpSS->Col.UL);

if (lpRow)
	/* RFW - 16317 - 6/13/05
   *lpRow = lpSS->Row.LR;
	*/
	*lpRow = SS_GetBottomCell(lpSS, lpSS->Row.UL);

#endif

return fRet;
}
#endif


BOOL DLLENTRY SSSetLock(hWnd, Col, Row, Lock)

HWND          hWnd;
SS_COORD      Col;
SS_COORD      Row;
BOOL          Lock;
{
return (SSSetLockRange(hWnd, Col, Row, Col, Row, Lock));
}


BOOL DLLENTRY SSSetLockRange(hWnd, Col, Row, Col2, Row2, Lock)

HWND          hWnd;
SS_COORD      Col;
SS_COORD      Row;
SS_COORD      Col2;
SS_COORD      Row2;
BOOL          Lock;
{
LPSPREADSHEET lpSS;
BOOL          bRet;

lpSS = SS_SheetLock(hWnd);
SS_AdjustCellRangeCoords(lpSS, &Col, &Row, &Col2, &Row2);
bRet = SS_SetLockRange(lpSS, Col, Row, Col2, Row2, Lock);
SS_SheetUnlock(hWnd);
return bRet;
}


BOOL SS_SetLockRange(lpSS, Col, Row, Col2, Row2, Lock)

LPSPREADSHEET lpSS;
SS_COORD      Col;
SS_COORD      Row;
SS_COORD      Col2;
SS_COORD      Row2;
BOOL          Lock;
{
SS_COORD      x;
SS_COORD      y;
BOOL          Ret = FALSE;
          
SS_LimitRangeCoords(lpSS, &Col, &Row, &Col2, &Row2);

if (Lock)
   Lock = SS_LOCKED_ON;
else
   Lock = SS_LOCKED_OFF;

for (y = Row; y <= Row2; y++)
   for (x = Col; x <= Col2; x++)
      {
      if (x != SS_ALLCOLS && y != SS_ALLROWS)
         Ret = SS_SetCellLock(lpSS, x, y, Lock);
      else if (x != SS_ALLCOLS && y == SS_ALLROWS)
         Ret = SS_SetColLock(lpSS, x, Lock);
      else if (x == SS_ALLCOLS && y != SS_ALLROWS)
         Ret = SS_SetRowLock(lpSS, y, Lock);
      else
         {
         SS_SetDocLock(lpSS, Lock);
         Ret = TRUE;
         }

      if (!Ret)
         break;
      }

return (Ret);
}


BOOL DLLENTRY SSGetLock(hWnd, Col, Row)

HWND          hWnd;
SS_COORD      Col;
SS_COORD      Row;
{
LPSPREADSHEET lpSS;
LPSS_ROW      lpRow;
LPSS_COL      lpCol;
BOOL          fLocked;

lpSS = SS_SheetLock(hWnd);

SS_AdjustCellCoords(lpSS, &Col, &Row);

lpCol = SS_LockColItem(lpSS, Col);
lpRow = SS_LockRowItem(lpSS, Row);

fLocked = SSxx_GetLock(lpSS, lpCol, lpRow, NULL, Col, Row, TRUE);

SS_UnlockColItem(lpSS, Col);
SS_UnlockRowItem(lpSS, Row);

SS_SheetUnlock(hWnd);
return (fLocked);
}


BOOL SS_GetLock(LPSPREADSHEET lpSS, SS_COORD Col, SS_COORD Row, BOOL fIgnoreMergedCells)
{
LPSS_ROW      lpRow;
LPSS_COL      lpCol;
BOOL          fLocked = FALSE;

lpCol = SS_LockColItem(lpSS, Col);
lpRow = SS_LockRowItem(lpSS, Row);

if (lpSS->lpBook->DocumentProtected)
   fLocked = SSxx_GetLock(lpSS, lpCol, lpRow, NULL, Col, Row, fIgnoreMergedCells);

SS_UnlockColItem(lpSS, Col);
SS_UnlockRowItem(lpSS, Row);

return (fLocked);
}


BOOL SSx_GetLock(LPSPREADSHEET lpSS, LPSS_COL lpCol, LPSS_ROW lpRow, LPSS_CELL lpCell,
                 SS_COORD Col, SS_COORD Row, BOOL fIgnoreMergedCells)
{
if (!lpSS->lpBook->DocumentProtected)
   return (FALSE);

return (SSxx_GetLock(lpSS, lpCol, lpRow, lpCell, Col, Row, fIgnoreMergedCells));
}


BOOL SSxx_GetLock(LPSPREADSHEET lpSS, LPSS_COL lpCol, LPSS_ROW lpRow, LPSS_CELL lpCell,
                  SS_COORD Col, SS_COORD Row, BOOL fIgnoreMergedCells)

{
LPSS_CELL     lpCellOrig = lpCell;
BOOL          fLocked;

if (Row != SS_ALLROWS && Col != SS_ALLCOLS &&
    (lpCell || (lpCell = SS_LockCellItem(lpSS, Col, Row))) &&
    lpCell->CellLocked)
   fLocked = lpCell->CellLocked;

else if (Row != SS_ALLROWS && lpRow && lpRow->RowLocked)
   fLocked = lpRow->RowLocked;

else if (Col != SS_ALLCOLS && lpCol && lpCol->ColLocked)
   fLocked = lpCol->ColLocked;

else
   fLocked = lpSS->DocumentLocked;

if (lpCell && !lpCellOrig)
   SS_UnlockCellItem(lpSS, Col, Row);

if (fLocked == SS_LOCKED_ON)
   return (TRUE);

#ifdef SS_V40
if (!fIgnoreMergedCells && SS_GetCellSpan(lpSS, Col, Row, NULL, NULL, NULL, NULL) == SS_SPAN_YES)
   return (TRUE);
#endif // SS_V40

return (FALSE);
}


BOOL DLLENTRY SSClear(hWnd, Col, Row)

HWND          hWnd;
SS_COORD      Col;
SS_COORD      Row;
{
return (SSClearRange(hWnd, Col, Row, Col, Row));
}


BOOL DLLENTRY SSClearRange(hWnd, Col, Row, Col2, Row2)

HWND          hWnd;
SS_COORD      Col;
SS_COORD      Row;
SS_COORD      Col2;
SS_COORD      Row2;
{
LPSPREADSHEET lpSS;
BOOL bRet;

if (!hWnd)
   return (FALSE);

lpSS = SS_SheetLock(hWnd);
SS_AdjustCellRangeCoords(lpSS, &Col, &Row, &Col2, &Row2);
bRet = SS_ClearRange(lpSS, Col, Row, Col2, Row2, FALSE);
SS_SheetUnlock(hWnd);
return bRet;
}


BOOL SS_ClearRange(lpSS, Col, Row, Col2, Row2, fSendChangeMsg)

LPSPREADSHEET lpSS;
SS_COORD      Col;
SS_COORD      Row;
SS_COORD      Col2;
SS_COORD      Row2;
BOOL          fSendChangeMsg;
{
BOOL          RedrawOld;
SS_COORD      x;
SS_COORD      y;
SS_COORD      lColOut;
SS_COORD      lRowOut;
BOOL          fTurnEditModeOn = FALSE;
BOOL          Ret = FALSE;

if (SS_ALLCOLS == Col || SS_ALLCOLS == Col2)
   Col = Col2 = SS_ALLCOLS;
if (SS_ALLROWS == Row || SS_ALLROWS == Row2)
   Row = Row2 = SS_ALLROWS;

if (Col >= SS_GetColCnt(lpSS) || Row >= SS_GetRowCnt(lpSS))
   return FALSE;

if (lpSS->lpBook->EditModeOn)
   {
   SS_CellEditModeOff(lpSS, 0);
   fTurnEditModeOn = TRUE;
   }

if (lpSS->fVirtualMode)
   {
   Row = max(Row, lpSS->Virtual.VTop);

   if (Row2 == -1)
      Row2 = lpSS->Virtual.VTop + lpSS->Virtual.VPhysSize - 1;
   else
      Row2 = min(Row2, lpSS->Virtual.VTop + lpSS->Virtual.VPhysSize - 1);
   }

RedrawOld = lpSS->lpBook->Redraw;
lpSS->lpBook->Redraw = FALSE;

#if !defined(SS_NOCALC) && !defined(SS_OLDCALC)
{
SS_COORD lCol2Out = Col2;
SS_COORD lRow2Out = Row2;

lColOut = Col;
lRowOut = Row;

SS_AdjustCellCoordsOut(lpSS, &lColOut, &lRowOut);
SS_AdjustCellCoordsOut(lpSS, &lCol2Out, &lRow2Out);
CalcClearRange(&lpSS->CalcInfo, lColOut, lRowOut, lCol2Out, lRow2Out);
}
#endif

for (y = Row; y <= Row2; y++)
   for (x = Col; x <= Col2; x++)
      {
      #if !defined(SS_NOCALC) && !defined(SS_OLDCALC)
		lColOut = x;
		lRowOut = y;

		SS_AdjustCellCoordsOut(lpSS, &lColOut, &lRowOut);
      CalcMarkDependForEval(&lpSS->CalcInfo, lColOut, lRowOut);
      #endif

      if (x != SS_ALLCOLS && y != SS_ALLROWS)
         Ret = SS_ClearCell(lpSS, x, y, TRUE, fSendChangeMsg);
      else if (x != SS_ALLCOLS && y == SS_ALLROWS)
         Ret = SS_ClearCol(lpSS, x, TRUE, fSendChangeMsg);
      else if (x == SS_ALLCOLS && y != SS_ALLROWS)
         Ret = SS_ClearRow(lpSS, y, TRUE, fSendChangeMsg);
      else
         Ret = SS_ClearDoc(lpSS);

      if (!Ret)
         break;
      }

SS_AdjustDataRowCnt(lpSS, Row, Row2);
SS_AdjustDataColCnt(lpSS, Col, Col2);

#if !defined(SS_NOCALC) && !defined(SS_OLDCALC)
if (lpSS->lpBook->CalcAuto)
  CalcEvalNeededCells(&lpSS->lpBook->CalcInfo);
#endif

#ifdef SS_UTP
if (lpSS->lpBook->hWnd)  // handle an OCX control without a HWND
{
SS_CalcHScrollArrowArea(lpSS->lpBook->hWnd, lpSS);
SS_CalcVScrollArrowArea(lpSS->lpBook->hWnd, lpSS);
}
#endif

SS_BookSetRedraw(lpSS->lpBook, RedrawOld);

if (fTurnEditModeOn)
   SS_CellEditModeOn(lpSS, 0, 0, 0L);

return (Ret);
}


BOOL DLLENTRY SSSetFont(hWnd, Col, Row, hFont, fDeleteFont)

HWND          hWnd;
SS_COORD      Col;
SS_COORD      Row;
HFONT         hFont;
BOOL          fDeleteFont;
{
return (SSSetFontRange(hWnd, Col, Row, Col, Row, hFont, fDeleteFont));
}


BOOL DLLENTRY SSSetFontRange(hWnd, Col, Row, Col2, Row2, hFont, fDeleteFont)

HWND          hWnd;
SS_COORD      Col;
SS_COORD      Row;
SS_COORD      Col2;
SS_COORD      Row2;
HFONT         hFont;
BOOL          fDeleteFont;
{
LPSPREADSHEET lpSS;
BOOL          bRet = FALSE;

if (lpSS = SS_SheetLock(hWnd))
   {
   SS_AdjustCellRangeCoords(lpSS, &Col, &Row, &Col2, &Row2);
   bRet = SS_SetFontRange(lpSS, Col, Row, Col2, Row2, hFont, fDeleteFont);
   SS_SheetUnlock(hWnd);
   }

return bRet;
}

BOOL SS_SetFontRange(lpSS, Col, Row, Col2, Row2, hFont, fDeleteFont)

LPSPREADSHEET lpSS;
SS_COORD      Col;
SS_COORD      Row;
SS_COORD      Col2;
SS_COORD      Row2;
HFONT         hFont;
BOOL          fDeleteFont;
{
SS_COORD      x;
SS_COORD      y;
HFONT         hFontOld = hFont;
BOOL          RedrawOld;
BOOL          fDeleteFontObject;
BOOL          fTurnEditModeOn = FALSE;
BOOL          Ret = FALSE;

#ifdef SS_VB
if (lpSS->fIgnoreSetFont)
   return (TRUE);
#endif

if (hFont == 0)
   return (FALSE);

SS_LimitRangeCoords(lpSS, &Col, &Row, &Col2, &Row2);

if (Row != Row2 || Col != Col2)
   {
   RedrawOld = lpSS->lpBook->Redraw;
   lpSS->lpBook->Redraw = FALSE;
   }

fDeleteFontObject = FALSE;

/*
if (hFont)
   {
   _fmemset(&LogFont, '\0', sizeof(LOGFONT));
   GetObject(hFont, sizeof(LOGFONT), (LPVOID)&LogFont);
   hFont = (WORD)CreateFontIndirect(&LogFont);
   }
*/

/* RFW - 5/20/01 - 8712
if (lpSS->EditModeOn)
   {
   wMessageBeingSent = lpSS->wMessageBeingSent;
   lpSS->wMessageBeingSent = FALSE;
   SS_CellEditModeOff(lpSS, 0);
   fTurnEditModeOn = TRUE;
   }
*/

for (y = Row; y <= Row2; y++)
   for (x = Col; x <= Col2; x++)
      {
      if (x != SS_ALLCOLS && y != SS_ALLROWS)
         Ret = SS_SetCellFont(lpSS, x, y, hFont, -1, NULL);
      else if (x != SS_ALLCOLS && y == SS_ALLROWS)
         Ret = SS_SetColFont(lpSS, x, hFont, -1, NULL);
      else if (x == SS_ALLCOLS && y != SS_ALLROWS)
         Ret = SS_SetRowFont(lpSS, y, hFont, -1, NULL);
      else
         Ret = SS_SetDocDefaultFont(lpSS, hFont, -1, NULL);

      if (!Ret)
         break;
      }

/*
if (fDeleteFontObject)
   DeleteObject(hFont);
*/

if (fDeleteFont)
   DeleteObject(hFontOld);

if (Row != Row2 || Col != Col2)
	SS_BookSetRedraw(lpSS->lpBook, RedrawOld);

SS_AutoSize(lpSS->lpBook, FALSE);  // BJO 12Aug97 GRB???? - Added

/* RFW - 5/20/01 - 8712
if (fTurnEditModeOn)
   {
   SS_CellEditModeOn(lpSS, 0, 0, 0L);
   lpSS->wMessageBeingSent = wMessageBeingSent;
   }
*/

return (Ret);
}

#if SS_V80
BOOL DLLENTRY SSSetLogFontRange(HWND hWnd, SS_COORD Col, SS_COORD Row, SS_COORD Col2, SS_COORD Row2, LOGFONT *pLogFont)
{
LPSPREADSHEET lpSS;
BOOL          bRet = FALSE;

if (lpSS = SS_SheetLock(hWnd))
   {
   SS_AdjustCellRangeCoords(lpSS, &Col, &Row, &Col2, &Row2);
   bRet = SS_SetLogFontRange(lpSS, Col, Row, Col2, Row2, pLogFont);
   SS_SheetUnlock(hWnd);
   }

return bRet;
}

BOOL DLLENTRY SSSetLogFont(HWND hWnd, SS_COORD Col, SS_COORD Row, LOGFONT *pLogFont)
{
return SSSetLogFontRange(hWnd, Col, Row, Col, Row, pLogFont);
}

BOOL SS_SetLogFontRange(LPSPREADSHEET lpSS, SS_COORD Col, SS_COORD Row, SS_COORD Col2, SS_COORD Row2, LOGFONT *pLogFont)
{
SS_COORD      x;
SS_COORD      y;
BOOL          RedrawOld;
BOOL          fTurnEditModeOn = FALSE;
BOOL          Ret = FALSE;

#ifdef SS_VB
if (lpSS->fIgnoreSetFont)
   return (TRUE);
#endif

if (pLogFont == NULL)
   return (FALSE);

SS_LimitRangeCoords(lpSS, &Col, &Row, &Col2, &Row2);

if (Row != Row2 || Col != Col2)
   {
   RedrawOld = lpSS->lpBook->Redraw;
   lpSS->lpBook->Redraw = FALSE;
   }

/*
if (hFont)
   {
   _fmemset(&LogFont, '\0', sizeof(LOGFONT));
   GetObject(hFont, sizeof(LOGFONT), (LPVOID)&LogFont);
   hFont = (WORD)CreateFontIndirect(&LogFont);
   }
*/

/* RFW - 5/20/01 - 8712
if (lpSS->EditModeOn)
   {
   wMessageBeingSent = lpSS->wMessageBeingSent;
   lpSS->wMessageBeingSent = FALSE;
   SS_CellEditModeOff(lpSS, 0);
   fTurnEditModeOn = TRUE;
   }
*/

for (y = Row; y <= Row2; y++)
   for (x = Col; x <= Col2; x++)
      {
      if (x != SS_ALLCOLS && y != SS_ALLROWS)
         Ret = SS_SetCellLogFont(lpSS, x, y, pLogFont);
      else if (x != SS_ALLCOLS && y == SS_ALLROWS)
         Ret = SS_SetColLogFont(lpSS, x, pLogFont);
      else if (x == SS_ALLCOLS && y != SS_ALLROWS)
         Ret = SS_SetRowLogFont(lpSS, y, pLogFont);
      else
         Ret = SS_SetDocDefaultLogFont(lpSS, pLogFont);

      if (!Ret)
         break;
      }

/*
if (fDeleteFontObject)
   DeleteObject(hFont);
*/

if (Row != Row2 || Col != Col2)
	SS_BookSetRedraw(lpSS->lpBook, RedrawOld);

SS_AutoSize(lpSS->lpBook, FALSE);  // BJO 12Aug97 GRB???? - Added

/* RFW - 5/20/01 - 8712
if (fTurnEditModeOn)
   {
   SS_CellEditModeOn(lpSS, 0, 0, 0L);
   lpSS->wMessageBeingSent = wMessageBeingSent;
   }
*/

return (Ret);
}

#endif

HFONT DLLENTRY SSGetFont(hWnd, Col, Row)

HWND          hWnd;
SS_COORD      Col;
SS_COORD      Row;
{
LPSPREADSHEET lpSS;
HFONT         hRet = 0;

if (!IsWindow(hWnd) || !(lpSS = SS_SheetLock(hWnd)))
   return (0);

SS_AdjustCellCoords(lpSS, &Col, &Row);

hRet = SS_GetFontHandle(lpSS, Col, Row);

SS_SheetUnlock(hWnd);

return hRet;
}

HFONT SS_GetFontHandle(LPSPREADSHEET lpSS, SS_COORD Col, SS_COORD Row)
{
SS_FONT       SSFont;
SS_FONTID     FontId;
HFONT         hRet = 0;

FontId = SS_GetFontId(lpSS, Col, Row);

if (FontId)
   if (SS_GetFont(lpSS->lpBook, &SSFont, FontId))
      hRet = SSFont.hFont;

return hRet;
}

SS_FONTID SS_GetFontId(lpSS, Col, Row)

LPSPREADSHEET lpSS;
SS_COORD      Col;
SS_COORD      Row;
{
LPSS_CELL     lpCell;
LPSS_COL      lpCol;
LPSS_ROW      lpRow;
SS_FONTID     FontId = 0;

if (Col != SS_ALLCOLS && Row != SS_ALLROWS)
   if (lpCell = SS_LockCellItem(lpSS, Col, Row))
      {
      if (lpCell->FontId != SS_FONT_NONE)
         FontId = lpCell->FontId;

      SS_UnlockCellItem(lpSS, Col, Row);
      }

if (!FontId && Row != SS_ALLROWS && Row < lpSS->Row.HeaderCnt &&
    (lpRow = SS_LockRowItem(lpSS, Row)))
   {
   if (lpRow->FontId != SS_FONT_NONE)
      FontId = lpRow->FontId;

   SS_UnlockRowItem(lpSS, Row);
   }

if (!FontId && Col != SS_ALLCOLS && (lpCol = SS_LockColItem(lpSS, Col)))
   {
   if (lpCol->FontId != SS_FONT_NONE)
      FontId = lpCol->FontId;

   SS_UnlockColItem(lpSS, Col);
   }

if (!FontId && Row != SS_ALLROWS && (lpRow = SS_LockRowItem(lpSS, Row)))
   {
   if (lpRow->FontId != SS_FONT_NONE)
      FontId = lpRow->FontId;

   SS_UnlockRowItem(lpSS, Row);
   }

if (!FontId)
   FontId = lpSS->DefaultFontId;

return (FontId);
}


BOOL DLLENTRY SSSetCellType(hWnd, Col, Row, CellType)

HWND          hWnd;
SS_COORD      Col;
SS_COORD      Row;
LPSS_CELLTYPE CellType;
{
LPSPREADSHEET lpSS;
BOOL          Ret;

lpSS = SS_SheetLock(hWnd);
SS_AdjustCellCoords(lpSS, &Col, &Row);
Ret = SS_SetCellTypeRange(lpSS, Col, Row, Col, Row, CellType);
SS_SheetUnlock(hWnd);
return (Ret);
}


BOOL DLLENTRY SSSetCellTypeRange(hWnd, Col, Row, Col2, Row2, CellType)

HWND          hWnd;
SS_COORD      Col;
SS_COORD      Row;
SS_COORD      Col2;
SS_COORD      Row2;
LPSS_CELLTYPE CellType;
{
LPSPREADSHEET lpSS;
BOOL          Ret;

lpSS = SS_SheetLock(hWnd);
SS_AdjustCellRangeCoords(lpSS, &Col, &Row, &Col2, &Row2);
Ret = SS_SetCellTypeRange(lpSS, Col, Row, Col2, Row2, CellType);
SS_SheetUnlock(hWnd);
return (Ret);
}


BOOL SS_SetCellTypeRange(lpSS, Col, Row, Col2, Row2, lpCellType)

LPSPREADSHEET lpSS;
SS_COORD      Col;
SS_COORD      Row;
SS_COORD      Col2;
SS_COORD      Row2;
LPSS_CELLTYPE lpCellType;
{
SS_CELLTYPE   CellTypeCopy;
LPSS_CELLTYPE lpCellTypeCopy = &CellTypeCopy;
SS_COORD      x;
SS_COORD      y;
BOOL          RedrawOld;
BOOL          Ret = FALSE;

SS_LimitRangeCoords(lpSS, &Col, &Row, &Col2, &Row2);

if (Col < -1 || Row < -1)
	return (FALSE);

if (Row != Row2 || Col != Col2)
   {
   RedrawOld = lpSS->lpBook->Redraw;
   lpSS->lpBook->Redraw = FALSE;
   }

for (y = Row; y <= Row2; y++)
   for (x = Col; x <= Col2; x++)
      {
      if (!lpCellType)
        lpCellTypeCopy = 0;
      else if( y == Row2 && x == Col2 )
        lpCellTypeCopy = lpCellType;
      else // not last assignment, so make a copy of structure
        SS_CopyCellType(lpSS, lpCellTypeCopy, lpCellType);

      if (x != SS_ALLCOLS && y != SS_ALLROWS)
         Ret = SS_SetCellCellType(lpSS, x, y, lpCellTypeCopy);
      else if (x != SS_ALLCOLS && y == SS_ALLROWS)
         Ret = SS_SetColCellType(lpSS, x, lpCellTypeCopy);
      else if (x == SS_ALLCOLS && y != SS_ALLROWS)
         Ret = SS_SetRowCellType(lpSS, y, lpCellTypeCopy);
      else
         Ret = SS_SetDocDefaultCellType(lpSS, lpCellTypeCopy);

      if (!Ret)
         break;
      }

if (Row != Row2 || Col != Col2)
	SS_BookSetRedraw(lpSS->lpBook, RedrawOld);

return (Ret);
}

BOOL DLLENTRY SSGetCellType(hWnd, Col, Row, CellType)

HWND          hWnd;
SS_COORD      Col;
SS_COORD      Row;
LPSS_CELLTYPE CellType;
{
LPSPREADSHEET lpSS;
BOOL          Ret = TRUE;

lpSS = SS_SheetLock(hWnd);
SS_AdjustCellCoords(lpSS, &Col, &Row);
Ret = SS_GetCellType(lpSS, Col, Row, CellType);
SS_SheetUnlock(hWnd);
return (Ret);
}


BOOL SS_GetCellType(lpSS, Col, Row, CellType)

LPSPREADSHEET lpSS;
SS_COORD      Col;
SS_COORD      Row;
LPSS_CELLTYPE CellType;
{
BOOL          Ret = TRUE;

if (!SS_RetrieveCellType(lpSS, CellType, NULL, Col, Row))
   Ret = FALSE;
/* RFW - 1/8/96 - TEL413
lpSS->CurVisCell.Col = Col;
lpSS->CurVisCell.Row = Row;
*/
return (Ret);
}


BOOL DLLENTRY SSSetColor(hWnd, Col, Row, Background, Foreground)

HWND     hWnd;
SS_COORD Col;
SS_COORD Row;
COLORREF Background;
COLORREF Foreground;
{
return (SSSetColorRange(hWnd, Col, Row, Col, Row, Background, Foreground));
}


BOOL DLLENTRY SSSetColorRange(hWnd, Col, Row, Col2, Row2, Background,
                                Foreground)

HWND          hWnd;
SS_COORD      Col;
SS_COORD      Row;
SS_COORD      Col2;
SS_COORD      Row2;
COLORREF      Background;
COLORREF      Foreground;
{
LPSPREADSHEET lpSS;
BOOL          Ret;

lpSS = SS_SheetLock(hWnd);
SS_AdjustCellRangeCoords(lpSS, &Col, &Row, &Col2, &Row2);
Ret = SS_SetColorRange(lpSS, Col, Row, Col2, Row2, Background, Foreground);
SS_SheetUnlock(hWnd);
return (Ret);
}


BOOL SS_SetColorRange(lpSS, Col, Row, Col2, Row2, Background, Foreground)

LPSPREADSHEET lpSS;
SS_COORD      Col;
SS_COORD      Row;
SS_COORD      Col2;
SS_COORD      Row2;
COLORREF      Background;
COLORREF      Foreground;
{
SS_COORD      x;
SS_COORD      y;
BOOL          RedrawOld;
BOOL          Ret = FALSE;

SS_LimitRangeCoords(lpSS, &Col, &Row, &Col2, &Row2);

if (Row != Row2 || Col != Col2 ||
    SS_BACKCOLORSTYLE_UNDERGRID != lpSS->lpBook->bBackColorStyle)
   {
   RedrawOld = lpSS->lpBook->Redraw;
   lpSS->lpBook->Redraw = FALSE;
   lpSS->lpBook->fNoInvalidate = TRUE;
   }
// 26639 -scl
else if( lpSS->lpBook->EditModeOn 
        && Row <= lpSS->Row.CurAt && lpSS->Row.CurAt <= Row2 
        && Col <= lpSS->Col.CurAt && lpSS->Col.CurAt <= Col2 )
   lpSS->lpBook->fLeaveEditModeOn = TRUE;

for (y = Row; y <= Row2; y++)
   for (x = Col; x <= Col2; x++)
      {
      if (x != SS_ALLCOLS && y != SS_ALLROWS)
         Ret = SS_SetCellColor(lpSS, x, y, Background, Foreground);
      else if (x != SS_ALLCOLS && y == SS_ALLROWS)
         Ret = SS_SetColColor(lpSS, x, Background, Foreground);
      else if (x == SS_ALLCOLS && y != SS_ALLROWS)
         Ret = SS_SetRowColor(lpSS, y, Background, Foreground);
      else
         {
         SS_SetDocColor(lpSS, Background, Foreground);
         Ret = TRUE;
         }

      if (!Ret)
         break;
      }

if (lpSS->lpBook->fNoInvalidate)
   {
   lpSS->lpBook->fNoInvalidate = FALSE;
   SS_InvalidateRange(lpSS, Col, Row, Col2, Row2);
	SS_BookSetRedraw(lpSS->lpBook, RedrawOld);
   }
// 26639 -scl
else if( lpSS->lpBook->fLeaveEditModeOn )
{
   HWND hWnd = SS_RetrieveControlhWnd(lpSS, lpSS->Col.CurAt, lpSS->Row.CurAt);
   lpSS->lpBook->fLeaveEditModeOn = FALSE;
   InvalidateRect(hWnd, NULL, TRUE);
   UpdateWindow(hWnd);
}

return (Ret);
}


BOOL DLLENTRY SSGetColor(hWnd, Col, Row, lpBackground, lpForeground)

HWND            hWnd;
SS_COORD        Col;
SS_COORD        Row;
LPCOLORREF      lpBackground;
LPCOLORREF      lpForeground;
{
LPSPREADSHEET   lpSS;
BOOL            Ret = FALSE;

lpSS = SS_SheetLock(hWnd);
SS_AdjustCellCoords(lpSS, &Col, &Row);
Ret = SS_GetColor(lpSS, Col, Row, lpBackground, lpForeground);
SS_SheetUnlock(hWnd);
return (Ret);
}


BOOL SS_GetColor(lpSS, Col, Row, lpBackground, lpForeground)

LPSPREADSHEET   lpSS;
SS_COORD        Col;
SS_COORD        Row;
LPCOLORREF      lpBackground;
LPCOLORREF      lpForeground;
{
SS_COLORTBLITEM BackColorTblItem;
SS_COLORTBLITEM ForeColorTblItem;
BOOL            Ret = TRUE;

SS_GetColorTblItem(lpSS, Col, Row, &BackColorTblItem, &ForeColorTblItem);

if (lpBackground)
   *lpBackground = BackColorTblItem.Color;

if (lpForeground)
   *lpForeground = ForeColorTblItem.Color;

return (Ret);
}


BOOL DLLENTRY SSGetCellColor(HWND hWnd, SS_COORD Col, SS_COORD Row,
                             LPCOLORREF lpBackground, LPCOLORREF lpForeground)
{
LPSPREADSHEET   lpSS;
BOOL            Ret = FALSE;

lpSS = SS_SheetLock(hWnd);
SS_AdjustCellCoords(lpSS, &Col, &Row);
Ret = SS_GetCellColor(lpSS, Col, Row, lpBackground, lpForeground);
SS_SheetUnlock(hWnd);
return (Ret);
}


BOOL SS_GetCellColor(LPSPREADSHEET lpSS, SS_COORD Col, SS_COORD Row,
                     LPCOLORREF lpBackground, LPCOLORREF lpForeground)
{
LPSS_COL        lpCol;
LPSS_ROW        lpRow;
LPSS_CELL       lpCell;
SS_COLORTBLITEM itemColor;
SS_COLORID      idBackColor = 0;
SS_COLORID      idForeColor = 0;
BOOL            Ret = TRUE;

if (Col != SS_ALLCOLS && Row != SS_ALLROWS &&
    (lpCell = SS_LockCellItem(lpSS, Col, Row)))
   {
   idBackColor = lpCell->Color.BackgroundId;
   idForeColor = lpCell->Color.ForegroundId;

   SS_UnlockCellItem(lpSS, Col, Row);
   }

else if (Col != SS_ALLCOLS && (lpCol = SS_LockColItem(lpSS, Col)))
   {
   idBackColor = lpCol->Color.BackgroundId;
   idForeColor = lpCol->Color.ForegroundId;

   SS_UnlockColItem(lpSS, Col);
   }

else if (Row != SS_ALLROWS && (lpRow = SS_LockRowItem(lpSS, Row)))
   {
   idBackColor = lpRow->Color.BackgroundId;
   idForeColor = lpRow->Color.ForegroundId;

   SS_UnlockRowItem(lpSS, Row);
   }

// RFW - -8/19/04 - 14929
else if (Col == SS_ALLCOLS && Row == SS_ALLROWS)
   {
   idBackColor = lpSS->Color.BackgroundId;
   idForeColor = lpSS->Color.ForegroundId;
   }

if (lpBackground)
   {
   SS_GetColorItem(&itemColor, idBackColor);
   *lpBackground = itemColor.Color;
   }

if (lpForeground)
   {
   SS_GetColorItem(&itemColor, idForeColor);
   *lpForeground = itemColor.Color;
   }

return (Ret);
}


BOOL DLLENTRY SSGetVisible(hWnd, Col, Row, Visible)

HWND          hWnd;
SS_COORD      Col;
SS_COORD      Row;
short         Visible;
{
LPSPREADSHEET lpSS;
BOOL          fRet = FALSE;

lpSS = SS_SheetLock(hWnd);

SS_AdjustCellCoords(lpSS, &Col, &Row);

if (Col != SS_ALLCOLS && Row != SS_ALLROWS)
   {
   fRet = SS_IsCellVisible(lpSS, Col, Row, Visible);

   if (fRet && (!SS_GetColWidthInPixels(lpSS, Col) ||
       !SS_GetRowHeightInPixels(lpSS, Row)))
      fRet = FALSE;
   }
else if (Col != SS_ALLCOLS && Row == SS_ALLROWS)
   {
   fRet = SS_IsColVisible(lpSS, Col, Visible);

   if (fRet && !SS_GetColWidthInPixels(lpSS, Col))
      fRet = FALSE;
   }
else if (Col == SS_ALLCOLS && Row != SS_ALLROWS)
   {
   fRet = SS_IsRowVisible(lpSS, Row, Visible);

   if (fRet && !SS_GetRowHeightInPixels(lpSS, Row))
      fRet = FALSE;
   }
else
   fRet = SSGetDocVisible(hWnd);

SS_SheetUnlock(hWnd);
return (fRet);
}


BOOL DLLENTRY SSGetCurrSelBlockPos(hWnd, lpCurrentCol, lpCurrentRow)

HWND          hWnd;
LPSS_COORD    lpCurrentCol;
LPSS_COORD    lpCurrentRow;
{
LPSPREADSHEET lpSS;
BOOL          fRet = FALSE;

lpSS = SS_SheetLock(hWnd);

if (SS_IsBlockSelected(lpSS))
   {
   if (lpCurrentCol)
      *lpCurrentCol = lpSS->Col.BlockCellCurrentPos < lpSS->Col.HeaderCnt ? -1 : lpSS->Col.BlockCellCurrentPos;
		
   if (lpCurrentRow)
      *lpCurrentRow = lpSS->Row.BlockCellCurrentPos < lpSS->Row.HeaderCnt ? -1 : lpSS->Row.BlockCellCurrentPos;

	SS_AdjustCellCoordsOut(lpSS, lpCurrentCol, lpCurrentRow);

   fRet = TRUE;
   }

SS_SheetUnlock(hWnd);
return (fRet);
}


#ifndef SS_NOCALC

BOOL DLLENTRY SSValidateFormula(HWND hWnd, LPCTSTR lpszFormula)
{
LPSPREADSHEET  lpSS;
BOOL           bRet;

lpSS = SS_SheetLock(hWnd);
bRet = SS_ValidateFormula(lpSS, lpszFormula);
SS_SheetUnlock(hWnd);
return bRet;
}


BOOL SS_ValidateFormula(LPSPREADSHEET lpSS, LPCTSTR lpszFormula)
{
#ifdef SS_OLDCALC
TBGLOBALHANDLE hCalc;
#else
CALC_HANDLE    hExpr;
#endif
BOOL           bValid = FALSE;

if (!lpszFormula)
   return TRUE;

#ifdef SS_OLDCALC

if (hCalc = tbGlobalAlloc(GMEM_MOVEABLE | GMEM_ZEROINIT, sizeof(SS_CALC)))
   {
   bValid = SS_CalcIsValid(lpSS, lpszFormula, hCalc);
   tbGlobalFree(hCalc);
   }

#else

hExpr = ExprCreate(&lpSS->lpBook->CalcInfo, 1, 1, lpszFormula);
bValid = hExpr != NULL;
ExprDestroy(hExpr);

#endif

return bValid;
}

#endif


void DLLENTRY SSLogUnitsToColWidth(hWnd, dUnits, lpdfColWidth)

HWND          hWnd;
int           dUnits;
LPDOUBLE      lpdfColWidth;
{
LPSPREADSHEET lpSS;

lpSS = SS_SheetLock(hWnd);
SS_CalcPixelsToColWidth(lpSS, 0, dUnits, lpdfColWidth);
SS_SheetUnlock(hWnd);
}


void DLLENTRY SSLogUnitsToRowHeight(hWnd, Row, dUnits, lpdfRowHeight)

HWND          hWnd;
SS_COORD      Row;
int           dUnits;
LPDOUBLE      lpdfRowHeight;
{
LPSPREADSHEET lpSS;

lpSS = SS_SheetLock(hWnd);
SS_AdjustCellCoords(lpSS, NULL, &Row);
SS_CalcPixelsToRowHeight(lpSS, Row, dUnits, lpdfRowHeight);
SS_SheetUnlock(hWnd);
}


#ifndef SS_GRID

//int DLLENTRY SSColWidthToLogUnits(hWnd, dfColWidth)
long DLLENTRY SSColWidthToLogUnits(hWnd, dfColWidth)

HWND          hWnd;
double        dfColWidth;
{
LPSPREADSHEET lpSS;
//short         dUnits;
long          dUnits;

lpSS = SS_SheetLock(hWnd);

dUnits = SS_ColWidthToPixels(lpSS, dfColWidth);

SS_SheetUnlock(hWnd);
return (dUnits);
}


int DLLENTRY SSRowHeightToLogUnits(hWnd, Row, dfRowHeight)

HWND          hWnd;
SS_COORD      Row;
double        dfRowHeight;
{
LPSPREADSHEET lpSS;
int           dUnits;

lpSS = SS_SheetLock(hWnd);
SS_AdjustCellCoords(lpSS, NULL, &Row);

dUnits = SS_RowHeightToPixels(lpSS, Row, dfRowHeight);

SS_SheetUnlock(hWnd);
return (dUnits);
}

#endif


#ifndef SS_NOCALC

BOOL DLLENTRY SSReCalc(hWnd)

HWND          hWnd;
{
LPSPREADSHEET lpSS;
BOOL          fRet;

lpSS = SS_SheetLock(hWnd);
fRet = SS_ReCalcAll(lpSS);
SS_SheetUnlock(hWnd);

return (fRet);
}

#endif


BOOL DLLENTRY SSDeSelectBlock(hWnd)

HWND hWnd;
{
LPSPREADSHEET lpSS;
BOOL          bRet;

lpSS = SS_SheetLock(hWnd);
bRet = SS_DeSelectBlock(lpSS);
SS_SheetUnlock(hWnd);
return bRet;
}


BOOL SS_DeSelectBlock(LPSPREADSHEET lpSS)
{
BOOL bRet;

if ((lpSS->lpBook->hWnd && GetFocus() == lpSS->lpBook->hWnd) || lpSS->lpBook->fRetainSelBlock)
   bRet = SS_ResetBlock(lpSS);
else
   bRet = SSx_ResetBlock(lpSS, FALSE);
return bRet;
}


#ifndef SS_GRID

BOOL DLLENTRY SSSetFreeze(HWND hWnd, SS_COORD ColsFrozen,
                            SS_COORD RowsFrozen)
{
LPSPREADSHEET lpSS;
BOOL bRet;

lpSS = SS_SheetLock(hWnd);
bRet = SS_SetFreeze(lpSS, ColsFrozen, RowsFrozen);
SS_SheetUnlock(hWnd);
return bRet;
}


BOOL SS_SetFreeze(LPSPREADSHEET lpSS, SS_COORD ColsFrozen,
                  SS_COORD RowsFrozen)
{
BOOL fTurnEditModeOn = FALSE;

if (lpSS->lpBook->EditModeOn)
   {
   SS_CellEditModeOff(lpSS, 0);
   fTurnEditModeOn = TRUE;
   }

if (ColsFrozen < 0)
   ColsFrozen = 0;

if (ColsFrozen + lpSS->Col.HeaderCnt > SS_GetColCnt(lpSS))
   ColsFrozen = SS_GetColCnt(lpSS) - lpSS->Col.HeaderCnt;

if (RowsFrozen < 0)
   RowsFrozen = 0;

if (RowsFrozen + lpSS->Row.HeaderCnt > SS_GetRowCnt(lpSS))
   RowsFrozen = SS_GetRowCnt(lpSS) - lpSS->Row.HeaderCnt;

// RFW - 4/29/02 - 10021
#ifdef SS_UTP
if (lpSS->fUseScrollArrows && lpSS->Col.Frozen > 0 && ColsFrozen > lpSS->Col.Frozen)
   SS_InsColRange(lpSS, lpSS->Col.HeaderCnt + lpSS->Col.Frozen - 1, ColsFrozen - 1);
#endif

lpSS->Row.Frozen = RowsFrozen;
lpSS->Col.Frozen = ColsFrozen;

lpSS->Row.UL = lpSS->Row.HeaderCnt + lpSS->Row.Frozen;
lpSS->Col.UL = lpSS->Col.HeaderCnt + lpSS->Col.Frozen;

SSx_ResetBlock(lpSS, FALSE);  // BJO 23Jun97 SCS5346 - Added

SS_GetFirstValidRow(lpSS, lpSS->Col.CurAt, &lpSS->Row.CurAt);
SS_GetFirstValidCol(lpSS, lpSS->Row.CurAt, &lpSS->Col.CurAt);

SS_InvalidateScrollArea(lpSS, SS_SCROLLAREA_ALL);

if (fTurnEditModeOn && SS_IsCellVisible(lpSS, lpSS->Col.CurAt, lpSS->Row.CurAt,
                                        SS_VISIBLE_ALL))
   SS_CellEditModeOn(lpSS, 0, 0, 0);
   
return (TRUE);
}


BOOL DLLENTRY SSGetFreeze(HWND hWnd, LPSS_COORD lpColsFrozen,
                            LPSS_COORD lpRowsFrozen)
{
LPSPREADSHEET lpSS;
BOOL          bRet;

lpSS = SS_SheetLock(hWnd);
bRet = SS_GetFreeze(lpSS, lpColsFrozen, lpRowsFrozen);
SS_SheetUnlock(hWnd);
return bRet;
}


BOOL SS_GetFreeze(LPSPREADSHEET lpSS, LPSS_COORD lpColsFrozen,
                  LPSS_COORD lpRowsFrozen)
{
if (lpRowsFrozen)
   *lpRowsFrozen = lpSS->Row.Frozen;
if (lpColsFrozen)
   *lpColsFrozen = lpSS->Col.Frozen;
return (TRUE);
}


void DLLENTRY SSSetAutoSizeVisible(HWND hWnd, SS_COORD VisibleCols,
                                     SS_COORD VisibleRows)
{
LPSPREADSHEET lpSS;

lpSS = SS_SheetLock(hWnd);
SS_SetAutoSizeVisible(lpSS, VisibleCols, VisibleRows);
SS_SheetUnlock(hWnd);
}




void SS_SetAutoSizeVisible(LPSPREADSHEET lpSS, SS_COORD VisibleCols,
                           SS_COORD VisibleRows)
{
lpSS->Col.AutoSizeVisibleCnt = VisibleCols;
lpSS->Row.AutoSizeVisibleCnt = VisibleRows;

if (lpSS->lpBook->fAutoSize)
   SS_AutoSize(lpSS->lpBook, FALSE);
}


void DLLENTRY SSGetAutoSizeVisible(HWND hWnd, LPSS_COORD lpVisibleCols,
                                     LPSS_COORD lpVisibleRows)
{
LPSPREADSHEET lpSS;

lpSS = SS_SheetLock(hWnd);
SS_GetAutoSizeVisible(lpSS, lpVisibleCols, lpVisibleRows);
SS_SheetUnlock(hWnd);
}


void SS_GetAutoSizeVisible(LPSPREADSHEET lpSS, LPSS_COORD lpVisibleCols,
                           LPSS_COORD lpVisibleRows)
{
if (lpVisibleCols)
   *lpVisibleCols = lpSS->Col.AutoSizeVisibleCnt;
if (lpVisibleRows)
   *lpVisibleRows = lpSS->Row.AutoSizeVisibleCnt;
}


BOOL DLLENTRY SSGetCellFromPixel(hWnd, lpCol, lpRow, MouseX, MouseY)

HWND          hWnd;
LPSS_COORD    lpCol;
LPSS_COORD    lpRow;
int           MouseX;
int           MouseY;
{
LPSPREADSHEET lpSS;
int           x;
int           y;
BOOL          fRet;

lpSS = SS_SheetLock(hWnd);

if (fRet = SS_GetCellFromPixel(lpSS, lpCol, lpRow, &x, &y, MouseX, MouseY))
   {
   if (lpCol)
      if (*lpCol == -1)
         *lpCol = SS_HEADER;
      else if (*lpCol < lpSS->Col.HeaderCnt)
         *lpCol += SS_HEADER;
      else
         *lpCol = *lpCol - lpSS->Col.HeaderCnt + 1;

   if (lpRow)
      if (*lpRow == -1)
         *lpRow = SS_HEADER;
      else if (*lpRow < lpSS->Row.HeaderCnt)
         *lpRow += SS_HEADER;
      else
         *lpRow = *lpRow - lpSS->Row.HeaderCnt + 1;
   }

SS_SheetUnlock(hWnd);
return (fRet);
}


void DLLENTRY SSGetLockColor(hWnd, lpBackground, lpForeground)

HWND            hWnd;
LPCOLORREF      lpBackground;
LPCOLORREF      lpForeground;
{
LPSPREADSHEET   lpSS;

lpSS = SS_SheetLock(hWnd);
SS_GetLockColor(lpSS, lpBackground, lpForeground);
SS_SheetUnlock(hWnd);
}


void SS_GetLockColor(lpSS, lpBackground, lpForeground)

LPSPREADSHEET   lpSS;
LPCOLORREF      lpBackground;
LPCOLORREF      lpForeground;
{
SS_COLORTBLITEM ColorTblItem;

if (lpBackground)
   {
   SS_GetColorItem(&ColorTblItem, lpSS->LockColor.BackgroundId);
   *lpBackground = ColorTblItem.Color;
   }

if (lpForeground)
   {
   SS_GetColorItem(&ColorTblItem, lpSS->LockColor.ForegroundId);
   *lpForeground = ColorTblItem.Color;
   }
}


void DLLENTRY SSSetLockColor(hWnd, Background, Foreground)

HWND          hWnd;
COLORREF      Background;
COLORREF      Foreground;
{
LPSPREADSHEET lpSS;

lpSS = SS_SheetLock(hWnd);
SS_SetLockColor(lpSS, Background, Foreground);
SS_SheetUnlock(hWnd);
}


void SS_SetLockColor(lpSS, Background, Foreground)

LPSPREADSHEET lpSS;
COLORREF      Background;
COLORREF      Foreground;
{
lpSS->LockColor.BackgroundId = SS_AddColor(Background);
lpSS->LockColor.ForegroundId = SS_AddColor(Foreground);

SS_InvalidateScrollArea(lpSS, SS_SCROLLAREA_ALL);
}


WORD DLLENTRY SSSetEditEnterAction(HWND hWnd, WORD wAction)
{
LPSS_BOOK lpBook;
WORD      wRet;

lpBook = SS_BookLock(hWnd);
wRet = SS_SetEditEnterAction(lpBook, wAction);
SS_BookUnlock(hWnd);
return (wRet);
}


WORD SS_SetEditEnterAction(LPSS_BOOK lpBook, WORD wAction)
{
WORD wRet;

wRet = lpBook->wEnterAction;
if (wAction >= 0 && wAction <= 8)
	lpBook->wEnterAction = wAction;

return (wRet);
}


WORD DLLENTRY SSGetEditEnterAction(HWND hWnd)
{
LPSS_BOOK lpBook;
WORD      wRet;

lpBook = SS_BookLock(hWnd);
wRet = SS_GetEditEnterAction(lpBook);
SS_BookUnlock(hWnd);
return (wRet);
}


WORD SS_GetEditEnterAction(LPSS_BOOK lpBook)
{
return lpBook->wEnterAction;
}

#endif


WORD DLLENTRY SSSetUnitType(HWND hWnd, WORD wUnitType)
{
LPSS_BOOK lpBook;
WORD      wRet;

lpBook = SS_BookLock(hWnd);
wRet = SS_SetUnitType(lpBook, wUnitType);
SS_BookUnlock(hWnd);
return (wRet);
}


WORD SS_SetUnitType(LPSS_BOOK lpBook, WORD wUnitType)
{
LPSPREADSHEET lpSS;
LPSS_ROW      lpRow;
LPSS_COL      lpCol;
SS_COORD      i;
double        dfSize;
short         nSheet;
WORD          wRet;

wRet = lpBook->dUnitType;
lpBook->dUnitType = wUnitType;

for (nSheet = 0; nSheet < lpBook->nSheetCnt; nSheet++)
	{
	lpSS = SS_BookLockSheetIndex(lpBook, nSheet);

	// RFW - 9689 - 1/23/02
	if (lpSS->Row.dCellSizeX100 = -1)
		dfSize = -1;
	else
		{
		SS_GetRowHeight(lpSS, -1, &dfSize);
		lpSS->Row.dCellSizeX100 = dfSize == -1 ? -1 : (long)(dfSize * 100.0);
		}

	lpSS->Row.dCellSizeInPixels = SS_RowHeightToPixels(lpSS, -1, dfSize);

	for (i = 0; i < lpSS->Row.AllocCnt; i++)
		if (lpRow = SS_LockRowItem(lpSS, i))
			{
			if (lpRow->dRowHeightInPixels != SS_HEIGHT_DEFAULT)
				{
				SS_GetRowHeight(lpSS, i, &dfSize);
				SS_SetRowHeight(lpSS, i, dfSize);
				}

			SS_UnlockRowItem(lpSS, i);
			}

	SS_GetColWidth(lpSS, -1, &dfSize);
	lpSS->Col.dCellSizeX100 = dfSize == -1 ? -1 : (long)(dfSize * 100.0);
	lpSS->Col.dCellSizeInPixels = SS_ColWidthToPixels(lpSS, dfSize);

	for (i = 0; i < lpSS->Col.AllocCnt; i++)
		if (lpCol = SS_LockColItem(lpSS, i))
			{
			if (lpCol->dColWidthInPixels != SS_WIDTH_DEFAULT)
				{
				SS_GetColWidth(lpSS, i, &dfSize);
				SS_SetColWidth(lpSS, i, dfSize);
				}

			SS_UnlockColItem(lpSS, i);
			}

	SS_BookUnlockSheetIndex(lpBook, nSheet);
	}

// RFW - 5/2/06 - 18755
SS_InvalidateRect(lpBook, NULL, TRUE);

return (wRet);
}


WORD DLLENTRY SSGetUnitType(HWND hWnd)
{
LPSS_BOOK lpBook;
WORD      wRet;

lpBook = SS_BookLock(hWnd);
wRet = SS_GetUnitType(lpBook);
SS_BookUnlock(hWnd);

return (wRet);
}


WORD SS_GetUnitType(LPSS_BOOK lpBook)

{
return lpBook->dUnitType;
}

#ifdef SS_V80
COLORREF DLLENTRY SSSetCellNoteIndicatorColor(HWND hWnd, COLORREF CellNoteIndicatorColor)
{
LPSS_BOOK lpBook;
COLORREF      CellNoteIndicatorColorOld;

lpBook = SS_BookLock(hWnd);
CellNoteIndicatorColorOld = SS_SetCellNoteIndicatorColor(lpBook, CellNoteIndicatorColor);
SS_BookUnlock(hWnd);

return (CellNoteIndicatorColorOld);
}


COLORREF SS_SetCellNoteIndicatorColor(LPSS_BOOK lpBook, COLORREF CellNoteIndicatorColor)
{
COLORREF      CellNoteIndicatorColorOld;

CellNoteIndicatorColorOld = lpBook->CellNoteIndicatorColor;
if (CellNoteIndicatorColorOld != CellNoteIndicatorColor)
   {
   lpBook->CellNoteIndicatorColor = CellNoteIndicatorColor;
   //SS_Invalidate(lpSS, SS_SCROLLAREA_ALL);
   }

return (CellNoteIndicatorColorOld);
}


COLORREF DLLENTRY SSGetCellNoteIndicatorColor(HWND hWnd)
{
LPSS_BOOK lpBook;
COLORREF      CellNoteIndicatorColorOld;

lpBook = SS_BookLock(hWnd);
CellNoteIndicatorColorOld = SS_GetCellNoteIndicatorColor(lpBook);
SS_BookUnlock(hWnd);

return (CellNoteIndicatorColorOld);
}


COLORREF SS_GetCellNoteIndicatorColor(LPSS_BOOK lpBook)
{
return lpBook->CellNoteIndicatorColor;
}


WORD DLLENTRY SSSetCellNoteIndicatorShape(HWND hWnd, WORD CellNoteIndicatorShape)
{
LPSS_BOOK lpBook;
WORD      CellNoteIndicatorShapeOld;

lpBook = SS_BookLock(hWnd);
CellNoteIndicatorShapeOld = SS_SetCellNoteIndicatorShape(lpBook, CellNoteIndicatorShape);
SS_BookUnlock(hWnd);

return (CellNoteIndicatorShapeOld);
}


WORD SS_SetCellNoteIndicatorShape(LPSS_BOOK lpBook, WORD CellNoteIndicatorShape)
{
WORD      CellNoteIndicatorShapeOld;

CellNoteIndicatorShapeOld = lpBook->wCellNoteIndicatorShape;
if (CellNoteIndicatorShapeOld != CellNoteIndicatorShape)
   {
   lpBook->wCellNoteIndicatorShape = CellNoteIndicatorShape;
   //SS_Invalidate(lpSS, SS_SCROLLAREA_ALL);
   }

return (CellNoteIndicatorShapeOld);
}


WORD DLLENTRY SSGetCellNoteIndicatorShape(HWND hWnd)
{
LPSS_BOOK lpBook;
WORD      CellNoteIndicatorShapeOld;

lpBook = SS_BookLock(hWnd);
CellNoteIndicatorShapeOld = SS_GetCellNoteIndicatorShape(lpBook);
SS_BookUnlock(hWnd);

return (CellNoteIndicatorShapeOld);
}


WORD SS_GetCellNoteIndicatorShape(LPSS_BOOK lpBook)
{
return lpBook->wCellNoteIndicatorShape;
}


void SSx_GetCalColors(HWND hWnd, LPCOLORREF lpBackGround, LPCOLORREF lpUpperHeader, LPCOLORREF lpLowerHeader,
					  LPCOLORREF lpHeaders, LPCOLORREF lpButtonBorder, LPCOLORREF lpUpperNormalStart, LPCOLORREF lpUpperNormalEnd,
					  LPCOLORREF lpLowerNormalStart, LPCOLORREF lpLowerNormalEnd, LPCOLORREF lpUpperPushedStart, LPCOLORREF lpUpperPushedEnd,
					  LPCOLORREF lpLowerPushedStart, LPCOLORREF lpLowerPushedEnd)
{

LPSS_BOOK lpBook = SS_BookLock(hWnd);

if (!lpBook)
	return;

*lpBackGround = lpBook->colLowerColor;
*lpUpperHeader = lpBook->colUpperColor;
*lpLowerHeader = lpBook->colLowerColor;
*lpHeaders = lpBook->sheetTabUpperNormalStartColor;
*lpButtonBorder = lpBook->sheetTabOuterBorderColor;
*lpUpperNormalStart = lpBook->sheetScrollUpperNormalStartColor;
*lpUpperNormalEnd = lpBook->sheetScrollUpperNormalEndColor;
*lpLowerNormalStart = lpBook->sheetScrollLowerNormalStartColor;
*lpLowerNormalEnd = lpBook->sheetScrollLowerNormalEndColor;
*lpUpperPushedStart = lpBook->sheetScrollUpperPushedStartColor;
*lpUpperPushedEnd = lpBook->sheetScrollUpperPushedEndColor;
*lpLowerPushedStart = lpBook->sheetScrollLowerPushedStartColor;
*lpLowerPushedEnd = lpBook->sheetScrollLowerPushedEndColor;

SS_BookUnlock(hWnd);
}

BOOL DLLENTRY SSIsVerticalScrollBarVisible(HWND hWnd)
{
LPSS_BOOK lpBook;
BOOL          fRet;

lpBook = SS_BookLock(hWnd);

fRet = SS_IsVerticalScrollBarVisible(lpBook);
SS_BookUnlock(hWnd);
return (fRet);
}
BOOL SS_IsVerticalScrollBarVisible(LPSS_BOOK lpBook)
{
BOOL fRet = FALSE;

if (lpBook->hWndVScroll != NULL && IsWindowVisible(lpBook->hWndVScroll))
	fRet = TRUE;

return (fRet);
}
BOOL DLLENTRY SSIsHorizontalScrollBarVisible(HWND hWnd)
{
LPSS_BOOK lpBook;
BOOL          fRet;

lpBook = SS_BookLock(hWnd);

fRet = SS_IsHorizontalScrollBarVisible(lpBook);
SS_BookUnlock(hWnd);
return (fRet);
}
BOOL SS_IsHorizontalScrollBarVisible(LPSS_BOOK lpBook)
{
BOOL fRet = FALSE;

if (lpBook->hWndHScroll != NULL && IsWindowVisible(lpBook->hWndHScroll))
	fRet = TRUE;

return (fRet);
}


BOOL DLLENTRY SSSetEnhancedRowHeaderColors(HWND hWnd, COLORREF clrSelectedHoverUpperColor, COLORREF clrSelectedHoverLowerColor, COLORREF clrSelectedUpperColor,
											  COLORREF clrSelectedLowerColor, COLORREF clrHoverUpperColor, COLORREF clrHoverLowerColor, COLORREF clrUpperColor,
											  COLORREF clrLowerColor, COLORREF clrSelectedBorderColor, COLORREF clrBorderColor)
{
LPSS_BOOK lpBook;
BOOL          fRet;

lpBook = SS_BookLock(hWnd);

fRet = SS_SetEnhancedRowHeaderColors(lpBook, clrSelectedHoverUpperColor, clrSelectedHoverLowerColor, clrSelectedUpperColor,
											  clrSelectedLowerColor, clrHoverUpperColor, clrHoverLowerColor, clrUpperColor,
											  clrLowerColor, clrSelectedBorderColor, clrBorderColor);
SS_BookUnlock(hWnd);
return (fRet);
}


BOOL SS_SetEnhancedRowHeaderColors(LPSS_BOOK lpBook, COLORREF clrSelectedHoverUpperColor, COLORREF clrSelectedHoverLowerColor, COLORREF clrSelectedUpperColor,
											  COLORREF clrSelectedLowerColor, COLORREF clrHoverUpperColor, COLORREF clrHoverLowerColor, COLORREF clrUpperColor,
											  COLORREF clrLowerColor, COLORREF clrSelectedBorderColor, COLORREF clrBorderColor)
{
BOOL fRet = TRUE;

lpBook->rowHeaderSelectedBorderColor = clrSelectedBorderColor;
lpBook->rowHeaderBorderColor = clrBorderColor;
lpBook->rowHoverUpperColor = clrHoverUpperColor;
lpBook->rowHoverLowerColor = clrHoverLowerColor;
lpBook->rowSelectedHoverUpperColor = clrSelectedHoverUpperColor;
lpBook->rowSelectedHoverLowerColor = clrSelectedHoverLowerColor;
lpBook->rowSelectedUpperColor = clrSelectedUpperColor;
lpBook->rowSelectedLowerColor = clrSelectedLowerColor;
lpBook->rowUpperColor = clrUpperColor;
lpBook->rowLowerColor = clrLowerColor;
SS_InvalidateRect(lpBook, NULL, TRUE);

return (fRet);
}

BOOL DLLENTRY SSSetEnhancedCornerColors(HWND hWnd, COLORREF clrBackColor, COLORREF clrHoverColor, COLORREF clrTriangleColor, COLORREF clrTriangleHover, COLORREF clrTriangleBorderColor, COLORREF clrTriangleHoverBorderColor)
{
LPSS_BOOK lpBook;
BOOL          fRet;

lpBook = SS_BookLock(hWnd);

fRet = SS_SetEnhancedCornerColors(lpBook, clrBackColor, clrHoverColor, clrTriangleColor, clrTriangleHover, clrTriangleBorderColor, clrTriangleHoverBorderColor);
SS_BookUnlock(hWnd);
return (fRet);
}


BOOL SS_SetEnhancedCornerColors(LPSS_BOOK lpBook, COLORREF clrBackColor, COLORREF clrHoverColor, COLORREF clrTriangleColor, COLORREF clrTriangleHover, COLORREF clrTriangleBorderColor, COLORREF clrTriangleHoverBorderColor)
{
BOOL fRet = TRUE;

lpBook->cornerBackColor = clrBackColor;
lpBook->cornerHoverColor = clrHoverColor;
lpBook->cornerTriangleColor = clrTriangleColor;
lpBook->cornerTriangleHoverColor = clrTriangleHover;
lpBook->cornerTriangleBorderColor = clrTriangleBorderColor;
lpBook->cornerTriangleHoverBorderColor = clrTriangleHoverBorderColor;
SS_InvalidateRect(lpBook, NULL, TRUE);

return (fRet);
}


BOOL DLLENTRY SSSetEnhancedColumnHeaderColors(HWND hWnd, COLORREF clrSelectedHoverUpperColor, COLORREF clrSelectedHoverLowerColor, COLORREF clrSelectedUpperColor,
											  COLORREF clrSelectedLowerColor, COLORREF clrHoverUpperColor, COLORREF clrHoverLowerColor, COLORREF clrUpperColor,
											  COLORREF clrLowerColor, COLORREF clrSelectedBorderColor, COLORREF clrBorderColor)
{
LPSS_BOOK lpBook;
BOOL          fRet;

lpBook = SS_BookLock(hWnd);

fRet = SS_SetEnhancedColumnHeaderColors(lpBook, clrSelectedHoverUpperColor, clrSelectedHoverLowerColor, clrSelectedUpperColor,
											  clrSelectedLowerColor, clrHoverUpperColor, clrHoverLowerColor, clrUpperColor,
											  clrLowerColor, clrSelectedBorderColor, clrBorderColor);
SS_BookUnlock(hWnd);
return (fRet);
}


BOOL SS_SetEnhancedColumnHeaderColors(LPSS_BOOK lpBook, COLORREF clrSelectedHoverUpperColor, COLORREF clrSelectedHoverLowerColor, COLORREF clrSelectedUpperColor,
											  COLORREF clrSelectedLowerColor, COLORREF clrHoverUpperColor, COLORREF clrHoverLowerColor, COLORREF clrUpperColor,
											  COLORREF clrLowerColor, COLORREF clrSelectedBorderColor, COLORREF clrBorderColor)
{
BOOL fRet = TRUE;

lpBook->colHeaderSelectedBorderColor = clrSelectedBorderColor;
lpBook->colHeaderBorderColor = clrBorderColor;
lpBook->colHoverUpperColor = clrHoverUpperColor;
lpBook->colHoverLowerColor = clrHoverLowerColor;
lpBook->colSelectedHoverUpperColor = clrSelectedHoverUpperColor;
lpBook->colSelectedHoverLowerColor = clrSelectedHoverLowerColor;
lpBook->colSelectedUpperColor = clrSelectedUpperColor;
lpBook->colSelectedLowerColor = clrSelectedLowerColor;
lpBook->colUpperColor = clrUpperColor;
lpBook->colLowerColor = clrLowerColor;
SS_InvalidateRect(lpBook, NULL, TRUE);

return (fRet);
}


BOOL DLLENTRY SSSetEnhancedScrollBarColors(HWND hWnd, COLORREF clrTrackColor, COLORREF clrArrowColor, COLORREF clrUpperNormalStartColor, COLORREF clrUpperNormalEndColor,
										   COLORREF clrLowerNormalStartColor, COLORREF clrLowerNormalEndColor, COLORREF clrUpperPushedStartColor, COLORREF clrUpperPushedEndColor,
										   COLORREF clrLowerPushedStartColor, COLORREF clrLowerPushedEndColor, COLORREF clrUpperHoverStartColor, COLORREF clrUpperHoverEndColor,
										   COLORREF clrLowerHoverStartColor, COLORREF clrLowerHoverEndColor, COLORREF clrHoverButtonBorderColor, COLORREF clrButtonBorderColor)
{
LPSS_BOOK lpBook;
BOOL          fRet;

lpBook = SS_BookLock(hWnd);

fRet = SS_SetEnhancedScrollBarColors(lpBook, clrTrackColor, clrArrowColor, clrUpperNormalStartColor, clrUpperNormalEndColor,
										   clrLowerNormalStartColor, clrLowerNormalEndColor, clrUpperPushedStartColor, clrUpperPushedEndColor,
										   clrLowerPushedStartColor, clrLowerPushedEndColor, clrUpperHoverStartColor, clrUpperHoverEndColor,
										   clrLowerHoverStartColor, clrLowerHoverEndColor, clrHoverButtonBorderColor, clrButtonBorderColor);
SS_BookUnlock(hWnd);
return (fRet);
}


BOOL SS_SetEnhancedScrollBarColors(LPSS_BOOK lpBook, COLORREF clrTrackColor, COLORREF clrArrowColor, COLORREF clrUpperNormalStartColor, COLORREF clrUpperNormalEndColor,
										   COLORREF clrLowerNormalStartColor, COLORREF clrLowerNormalEndColor, COLORREF clrUpperPushedStartColor, COLORREF clrUpperPushedEndColor,
										   COLORREF clrLowerPushedStartColor, COLORREF clrLowerPushedEndColor, COLORREF clrUpperHoverStartColor, COLORREF clrUpperHoverEndColor,
										   COLORREF clrLowerHoverStartColor, COLORREF clrLowerHoverEndColor, COLORREF clrHoverButtonBorderColor, COLORREF clrButtonBorderColor)
{
BOOL fRet = TRUE;

lpBook->scrollTrackColor = clrTrackColor;
lpBook->scrollArrowColor = clrArrowColor;
lpBook->scrollBorderColor = clrButtonBorderColor;
lpBook->scrollUpperNormalStartColor = clrUpperNormalStartColor;
lpBook->scrollUpperNormalEndColor = clrUpperNormalEndColor;
lpBook->scrollLowerNormalStartColor = clrLowerNormalStartColor;
lpBook->scrollLowerNormalEndColor = clrLowerNormalEndColor;
lpBook->scrollUpperHoverStartColor = clrUpperHoverStartColor;
lpBook->scrollUpperHoverEndColor = clrUpperHoverEndColor;
lpBook->scrollLowerHoverStartColor = clrLowerHoverStartColor;
lpBook->scrollLowerHoverEndColor = clrLowerHoverEndColor;
lpBook->scrollUpperPushedStartColor = clrUpperPushedStartColor;
lpBook->scrollUpperPushedEndColor = clrUpperPushedEndColor;
lpBook->scrollLowerPushedStartColor = clrLowerPushedStartColor;
lpBook->scrollLowerPushedEndColor = clrLowerPushedEndColor;
lpBook->scrollHoverBorderColor = clrHoverButtonBorderColor;
if (lpBook->hWndVScroll)
		ScrlBarEnhancedScrollBarColors(lpBook->hWndVScroll, clrTrackColor, clrArrowColor, clrUpperNormalStartColor, clrUpperNormalEndColor,
										   clrLowerNormalStartColor, clrLowerNormalEndColor, clrUpperPushedStartColor, clrUpperPushedEndColor,
										   clrLowerPushedStartColor, clrLowerPushedEndColor, clrUpperHoverStartColor, clrUpperHoverEndColor,
										   clrLowerHoverStartColor, clrLowerHoverEndColor, clrHoverButtonBorderColor, clrButtonBorderColor);

if (lpBook->hWndHScroll)
		ScrlBarEnhancedScrollBarColors(lpBook->hWndHScroll, clrTrackColor, clrArrowColor, clrUpperNormalStartColor, clrUpperNormalEndColor,
										   clrLowerNormalStartColor, clrLowerNormalEndColor, clrUpperPushedStartColor, clrUpperPushedEndColor,
										   clrLowerPushedStartColor, clrLowerPushedEndColor, clrUpperHoverStartColor, clrUpperHoverEndColor,
										   clrLowerHoverStartColor, clrLowerHoverEndColor, clrHoverButtonBorderColor, clrButtonBorderColor);
return (fRet);
}



BOOL DLLENTRY SSSetEnhancedSheetTabColors(HWND hWnd, COLORREF clrNormalUpperStartColor, COLORREF clrNormalUpperEndColor,
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
BOOL          fRet;

lpBook = SS_BookLock(hWnd);

fRet = SS_SetEnhancedSheetTabColors(lpBook,  clrNormalUpperStartColor,  clrNormalUpperEndColor,
								    clrNormalLowerStartColor,  clrNormalLowerEndColor,  clrHoverUpperStartColor,
								    clrHoverUpperEndColor,  clrHoverLowerStartColor,  clrHoverLowerEndColor,
								    clrTabOuterBorderColor,  clrTabInnerBorderColor,  clrButtonUpperNormalStartColor,
								    clrButtonUpperNormalEndColor,  clrButtonLowerNormalStartColor,  clrButtonLowerNormalEndColor,
								    clrButtonUpperHoverStartColor,  clrButtonUpperHoverEndColor,  clrButtonLowerHoverStartColor,
								    clrButtonLowerHoverEndColor, clrButtonUpperPushedStartColor,  clrButtonUpperPushedEndColor,
								    clrButtonLowerPushedStartColor,  clrButtonLowerPushedEndColor,  clrArrowColor,
								    clrBackColorStartColor,  clrBackColorEndColor,  clrSplitBoxStartColor,
								    clrSplitBoxEndColor, clrForeColor, clrActiveForeColor);
SS_BookUnlock(hWnd);
return (fRet);
}


BOOL SS_SetEnhancedSheetTabColors(LPSS_BOOK lpBook, COLORREF clrNormalUpperStartColor, COLORREF clrNormalUpperEndColor,
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
BOOL fRet = TRUE;

lpBook->sheetTabUpperNormalStartColor = clrNormalUpperStartColor;
lpBook->sheetTabUpperNormalEndColor = clrNormalUpperEndColor;
lpBook->sheetTabLowerNormalStartColor = clrNormalLowerStartColor;
lpBook->sheetTabLowerNormalEndColor = clrNormalLowerEndColor;
lpBook->sheetTabUpperHoverStartColor = clrHoverUpperStartColor;
lpBook->sheetTabUpperHoverEndColor = clrHoverUpperEndColor;
lpBook->sheetTabLowerHoverStartColor = clrHoverLowerStartColor;
lpBook->sheetTabLowerHoverEndColor = clrHoverLowerEndColor;
lpBook->sheetTabInnerBorderColor = clrTabInnerBorderColor;
lpBook->sheetTabOuterBorderColor = clrTabOuterBorderColor;
lpBook->sheetGrayAreaStartColor = clrBackColorStartColor;
lpBook->sheetGrayAreaEndColor = clrBackColorEndColor;
lpBook->sheetSplitBoxStartColor = clrSplitBoxStartColor;
lpBook->sheetSplitBoxEndColor = clrSplitBoxEndColor;
lpBook->sheetScrollArrowColor = clrArrowColor;
lpBook->sheetScrollUpperNormalStartColor = clrButtonUpperNormalStartColor;
lpBook->sheetScrollUpperNormalEndColor = clrButtonUpperNormalEndColor;
lpBook->sheetScrollLowerNormalStartColor = clrButtonLowerNormalStartColor;
lpBook->sheetScrollLowerNormalEndColor = clrButtonLowerNormalEndColor;
lpBook->sheetScrollUpperHoverStartColor = clrButtonUpperHoverStartColor;
lpBook->sheetScrollUpperHoverEndColor = clrButtonUpperHoverEndColor;
lpBook->sheetScrollLowerHoverStartColor = clrButtonLowerHoverStartColor;
lpBook->sheetScrollLowerHoverEndColor = clrButtonLowerHoverEndColor;
lpBook->sheetScrollUpperPushedStartColor = clrButtonUpperPushedStartColor;
lpBook->sheetScrollUpperPushedEndColor = clrButtonUpperPushedEndColor;
lpBook->sheetScrollLowerPushedStartColor = clrButtonLowerPushedStartColor;
lpBook->sheetScrollLowerPushedEndColor = clrButtonLowerPushedEndColor;
lpBook->sheetTabForeColor = clrForeColor;
lpBook->sheetTabActiveForeColor = clrActiveForeColor;

SS_InvalidateRect(lpBook, NULL, TRUE);

return (fRet);
}

WORD DLLENTRY SSSetAppearanceStyle(HWND hWnd, WORD wStyle)
{
LPSS_BOOK lpBook;
WORD      wRet;

lpBook = SS_BookLock(hWnd);
wRet = SS_SetAppearanceStyle(lpBook, wStyle);
SS_BookUnlock(hWnd);
return (wRet);
}


WORD SS_SetAppearanceStyle(LPSS_BOOK lpBook, WORD wStyle)
{
WORD wRet;
long lStyle;
long lHStyle;
BOOL fIsHorizontal = TRUE;
wRet = lpBook->wAppearanceStyle;
if (wStyle >= 0 && wStyle <= 2)
	lpBook->wAppearanceStyle = wStyle;

#ifndef SS_NOCHILDSCROLL

lStyle = GetWindowLong(lpBook->hWnd, GWL_STYLE);

DestroyWindow(lpBook->hWndVScroll);
DestroyWindow(lpBook->hWndHScroll);

if (lpBook->wAppearanceStyle && lpBook->wScrollBarStyle != 1)
{
   long lStyle = 0;
   if (lpBook->fUseSpecialVScroll)
   {
   if (!(lpBook->wSpecialVScrollOptions & SS_VSCROLL_NOHOMEEND))
      lStyle |= SBRS_HOMEEND;
   if (!(lpBook->wSpecialVScrollOptions & SS_VSCROLL_NOPAGEUPDN))
      lStyle |= SBRS_PAGEUPDN;
   if (!(lpBook->wSpecialVScrollOptions & SS_VSCROLL_NOUPDN))
      lStyle |= SBRS_UPDN;
   }
   else
	lStyle = SBRS_UPDN | SBRS_THUMB;
   fIsHorizontal = FALSE;
   lpBook->hWndVScroll = CreateWindow(SSClassNames[dClassNameIndex].TBScrlBar,
                                    NULL, WS_CHILD | WS_VISIBLE | SBS_VERT | 
                                    lStyle, 0, 0, 0, 0, lpBook->hWnd, (HMENU)SBS_VERT,
                                    hDynamicInst, (LPVOID)(LPBOOL)&fIsHorizontal);
   SendMessage(lpBook->hWndVScroll, SBR_SETSTYLE, (WPARAM)lpBook->wScrollBarStyle == 0 ? lpBook->wAppearanceStyle : lpBook->wScrollBarStyle-1, 0L);
   //SendMessage(lpBook->hWndVScroll, SBM_SETRANGE, 1, lpBook->hWnd ? SSGetMaxRows(lpBook->hWnd) : 500);
   SendMessage(lpBook->hWndVScroll, SBR_TRACKMOUSE, lpBook->fVScrollBarTrack, 0);
   
   if (lpBook->hWndVScroll)
		ScrlBarEnhancedScrollBarColors(lpBook->hWndVScroll, lpBook->scrollTrackColor, lpBook->scrollArrowColor, lpBook->scrollUpperNormalStartColor, lpBook->scrollUpperNormalEndColor,
										   lpBook->scrollLowerNormalStartColor, lpBook->scrollLowerNormalEndColor, lpBook->scrollUpperPushedStartColor, lpBook->scrollUpperPushedEndColor,
										   lpBook->scrollLowerPushedStartColor, lpBook->scrollLowerPushedEndColor, lpBook->scrollUpperHoverStartColor, lpBook->scrollUpperHoverEndColor,
										   lpBook->scrollLowerHoverStartColor, lpBook->scrollLowerHoverEndColor, lpBook->scrollHoverBorderColor, lpBook->scrollBorderColor);

   lHStyle = SBRS_UPDN;
   lHStyle |= SBRS_THUMB;
   fIsHorizontal = TRUE;
   lpBook->hWndHScroll = CreateWindow(SSClassNames[dClassNameIndex].TBScrlBar,
                                    NULL, WS_CHILD | WS_VISIBLE | SBS_HORZ|
                                    lHStyle, 0, 0, 0, 0, lpBook->hWnd, (HMENU)SBS_HORZ,
                                    hDynamicInst, (LPVOID)(LPBOOL)&fIsHorizontal);
   SendMessage(lpBook->hWndHScroll, SBR_SETSTYLE, (WPARAM)lpBook->wScrollBarStyle == 0 ? lpBook->wAppearanceStyle : lpBook->wScrollBarStyle-1, 0L);
   SendMessage(lpBook->hWndHScroll, SBR_TRACKMOUSE, lpBook->fHScrollBarTrack, 0);

   if (lpBook->hWndHScroll)
		ScrlBarEnhancedScrollBarColors(lpBook->hWndHScroll, lpBook->scrollTrackColor, lpBook->scrollArrowColor, lpBook->scrollUpperNormalStartColor, lpBook->scrollUpperNormalEndColor,
										   lpBook->scrollLowerNormalStartColor, lpBook->scrollLowerNormalEndColor, lpBook->scrollUpperPushedStartColor, lpBook->scrollUpperPushedEndColor,
										   lpBook->scrollLowerPushedStartColor, lpBook->scrollLowerPushedEndColor, lpBook->scrollUpperHoverStartColor, lpBook->scrollUpperHoverEndColor,
										   lpBook->scrollLowerHoverStartColor, lpBook->scrollLowerHoverEndColor, lpBook->scrollHoverBorderColor, lpBook->scrollBorderColor);
}
else
{
     lpBook->hWndHScroll = CreateWindow(_T("ScrollBar"), NULL, WS_CHILD |
                                 /*
                                 WS_CLIPSIBLINGS | WS_VISIBLE |
                                 */
                                 WS_VISIBLE |
                                 SBS_HORZ, 0, 0, 0, 0,
                                 lpBook->hWnd, (HMENU)SBS_HORZ, hDynamicInst, NULL);
     lpBook->hWndVScroll = CreateWindow(_T("ScrollBar"), NULL, WS_CHILD |
                                    /*
                                    WS_CLIPSIBLINGS | WS_VISIBLE |
                                    */
                                    WS_VISIBLE |
                                    SBS_VERT, 0, 0, 0, 0,
                                    lpBook->hWnd, (HMENU)SBS_VERT, hDynamicInst, NULL);
}

#endif
SS_AutoSize(lpBook, TRUE);
SS_InvalidateRect(lpBook, NULL, TRUE);
return (wRet);
}


WORD DLLENTRY SSGetAppearanceStyle(HWND hWnd)
{
LPSS_BOOK lpBook;
WORD      wRet;

lpBook = SS_BookLock(hWnd);
wRet = SS_GetAppearanceStyle(lpBook);
SS_BookUnlock(hWnd);
return (wRet);
}


WORD SS_GetAppearanceStyle(LPSS_BOOK lpBook)
{
return lpBook->wAppearanceStyle;
}


WORD DLLENTRY SSSetScrollBarStyle(HWND hWnd, WORD wStyle)
{
LPSS_BOOK lpBook;
WORD      wRet;

lpBook = SS_BookLock(hWnd);
wRet = SS_SetScrollBarStyle(lpBook, wStyle);
SS_BookUnlock(hWnd);
return (wRet);
}


WORD SS_SetScrollBarStyle(LPSS_BOOK lpBook, WORD wStyle)
{
WORD wRet;
long lStyle;
long lHStyle;
BOOL fIsHorizontal = TRUE;
wRet = lpBook->wScrollBarStyle;
if (wStyle >= 0 && wStyle <= 3)
	lpBook->wScrollBarStyle = wStyle;

#ifndef SS_NOCHILDSCROLL

lStyle = GetWindowLong(lpBook->hWnd, GWL_STYLE);

DestroyWindow(lpBook->hWndVScroll);
DestroyWindow(lpBook->hWndHScroll);

if (lpBook->wAppearanceStyle && lpBook->wScrollBarStyle != 1)
{
   long lStyle = 0;
   if (lpBook->fUseSpecialVScroll)
   {
   if (!(lpBook->wSpecialVScrollOptions & SS_VSCROLL_NOHOMEEND))
      lStyle |= SBRS_HOMEEND;
   if (!(lpBook->wSpecialVScrollOptions & SS_VSCROLL_NOPAGEUPDN))
      lStyle |= SBRS_PAGEUPDN;
   if (!(lpBook->wSpecialVScrollOptions & SS_VSCROLL_NOUPDN))
      lStyle |= SBRS_UPDN;
   }
   else
	lStyle = SBRS_UPDN | SBRS_THUMB;
   fIsHorizontal = FALSE;
   lpBook->hWndVScroll = CreateWindow(SSClassNames[dClassNameIndex].TBScrlBar,
                                    NULL, WS_CHILD | WS_VISIBLE | SBS_VERT | 
                                    lStyle, 0, 0, 0, 0, lpBook->hWnd, (HMENU)SBS_VERT,
                                    hDynamicInst, (LPVOID)(LPBOOL)&fIsHorizontal);
   SendMessage(lpBook->hWndVScroll, SBR_SETSTYLE, (WPARAM)lpBook->wScrollBarStyle == 0 ? lpBook->wAppearanceStyle : lpBook->wScrollBarStyle-1, 0L);
   //SendMessage(lpBook->hWndVScroll, SBM_SETRANGE, 1, lpBook->hWnd ? SSGetMaxRows(lpBook->hWnd) : 500);
   SendMessage(lpBook->hWndVScroll, SBR_TRACKMOUSE, lpBook->fVScrollBarTrack, 0);
   
   if (lpBook->hWndVScroll)
		ScrlBarEnhancedScrollBarColors(lpBook->hWndVScroll, lpBook->scrollTrackColor, lpBook->scrollArrowColor, lpBook->scrollUpperNormalStartColor, lpBook->scrollUpperNormalEndColor,
										   lpBook->scrollLowerNormalStartColor, lpBook->scrollLowerNormalEndColor, lpBook->scrollUpperPushedStartColor, lpBook->scrollUpperPushedEndColor,
										   lpBook->scrollLowerPushedStartColor, lpBook->scrollLowerPushedEndColor, lpBook->scrollUpperHoverStartColor, lpBook->scrollUpperHoverEndColor,
										   lpBook->scrollLowerHoverStartColor, lpBook->scrollLowerHoverEndColor, lpBook->scrollHoverBorderColor, lpBook->scrollBorderColor);

   lHStyle = SBRS_UPDN;
   lHStyle |= SBRS_THUMB;
   fIsHorizontal = TRUE;
   lpBook->hWndHScroll = CreateWindow(SSClassNames[dClassNameIndex].TBScrlBar,
                                    NULL, WS_CHILD | WS_VISIBLE | SBS_HORZ|
                                    lHStyle, 0, 0, 0, 0, lpBook->hWnd, (HMENU)SBS_HORZ,
                                    hDynamicInst, (LPVOID)(LPBOOL)&fIsHorizontal);
   SendMessage(lpBook->hWndHScroll, SBR_SETSTYLE, (WPARAM)lpBook->wScrollBarStyle == 0 ? lpBook->wAppearanceStyle : lpBook->wScrollBarStyle-1, 0L);
   SendMessage(lpBook->hWndHScroll, SBR_TRACKMOUSE, lpBook->fHScrollBarTrack, 0);

   if (lpBook->hWndHScroll)
		ScrlBarEnhancedScrollBarColors(lpBook->hWndHScroll, lpBook->scrollTrackColor, lpBook->scrollArrowColor, lpBook->scrollUpperNormalStartColor, lpBook->scrollUpperNormalEndColor,
										   lpBook->scrollLowerNormalStartColor, lpBook->scrollLowerNormalEndColor, lpBook->scrollUpperPushedStartColor, lpBook->scrollUpperPushedEndColor,
										   lpBook->scrollLowerPushedStartColor, lpBook->scrollLowerPushedEndColor, lpBook->scrollUpperHoverStartColor, lpBook->scrollUpperHoverEndColor,
										   lpBook->scrollLowerHoverStartColor, lpBook->scrollLowerHoverEndColor, lpBook->scrollHoverBorderColor, lpBook->scrollBorderColor);
}
else
{
     lpBook->hWndHScroll = CreateWindow(_T("ScrollBar"), NULL, WS_CHILD |
                                 /*
                                 WS_CLIPSIBLINGS | WS_VISIBLE |
                                 */
                                 WS_VISIBLE |
                                 SBS_HORZ, 0, 0, 0, 0,
                                 lpBook->hWnd, (HMENU)SBS_HORZ, hDynamicInst, NULL);
     lpBook->hWndVScroll = CreateWindow(_T("ScrollBar"), NULL, WS_CHILD |
                                    /*
                                    WS_CLIPSIBLINGS | WS_VISIBLE |
                                    */
                                    WS_VISIBLE |
                                    SBS_VERT, 0, 0, 0, 0,
                                    lpBook->hWnd, (HMENU)SBS_VERT, hDynamicInst, NULL);
}

#endif
SS_AutoSize(lpBook, TRUE);
SS_InvalidateRect(lpBook, NULL, TRUE);
return (wRet);
}
WORD DLLENTRY SSGetScrollBarStyle(HWND hWnd)
{
LPSS_BOOK lpBook;
WORD      wRet;

lpBook = SS_BookLock(hWnd);
wRet = SS_GetScrollBarStyle(lpBook);
SS_BookUnlock(hWnd);
return (wRet);
}


WORD SS_GetScrollBarStyle(LPSS_BOOK lpBook)
{
return lpBook->wScrollBarStyle;
}


WORD DLLENTRY SSSetUseVisualStyles(HWND hWnd, WORD wStyle)
{
LPSS_BOOK lpBook;
WORD      wRet;

lpBook = SS_BookLock(hWnd);
wRet = SS_SetUseVisualStyles(lpBook, wStyle);
SS_BookUnlock(hWnd);
return (wRet);
}


WORD SS_SetUseVisualStyles(LPSS_BOOK lpBook, WORD wStyle)
{
WORD wRet = lpBook->wUseVisualStyles;
if (wStyle >= 0 && wStyle <= 2)
	lpBook->wUseVisualStyles = wStyle;
SS_InvalidateRect(lpBook, NULL, TRUE);
return (wRet);
}


WORD DLLENTRY SSGetUseVisualStyles(HWND hWnd)
{
LPSS_BOOK lpBook;
WORD      wRet;

lpBook = SS_BookLock(hWnd);
wRet = SS_GetUseVisualStyles(lpBook);
SS_BookUnlock(hWnd);
return (wRet);
}


WORD SS_GetUseVisualStyles(LPSS_BOOK lpBook)
{
return lpBook->wUseVisualStyles;
}


WORD DLLENTRY SSSetTabEnhancedShape(HWND hWnd, WORD wStyle)
{
LPSS_BOOK lpBook;
WORD      wRet;

lpBook = SS_BookLock(hWnd);
wRet = SS_SetTabEnhancedShape(lpBook, wStyle);
SS_BookUnlock(hWnd);
return (wRet);
}


WORD SS_SetTabEnhancedShape(LPSS_BOOK lpBook, WORD wStyle)
{
WORD wRet;

wRet = lpBook->wTabEnhancedShape;
if (wStyle >= 0 && wStyle <= 3)
	lpBook->wTabEnhancedShape = wStyle;
SS_InvalidateTabStrip(lpBook);
return (wRet);
}


WORD DLLENTRY SSGetTabEnhancedShape(HWND hWnd)
{
LPSS_BOOK lpBook;
WORD      wRet;

lpBook = SS_BookLock(hWnd);
wRet = SS_GetTabEnhancedShape(lpBook);
SS_BookUnlock(hWnd);
return (wRet);
}


WORD SS_GetTabEnhancedShape(LPSS_BOOK lpBook)
{
return lpBook->wTabEnhancedShape;
}

BOOL DLLENTRY SSSetEnhanceStaticCells(HWND hWnd, BOOL wStyle)
{
LPSS_BOOK lpBook;
BOOL      wRet;

lpBook = SS_BookLock(hWnd);
wRet = SS_SetEnhanceStaticCells(lpBook, wStyle);
SS_BookUnlock(hWnd);
return (wRet);
}


BOOL SS_SetEnhanceStaticCells(LPSS_BOOK lpBook, BOOL wStyle)
{
WORD wRet = lpBook->bEnhanceStaticCells;
lpBook->bEnhanceStaticCells = wStyle;
SS_InvalidateRect(lpBook, NULL, TRUE);
return (wRet);
}


BOOL DLLENTRY SSGetEnhanceStaticCells(HWND hWnd)
{
LPSS_BOOK lpBook;
BOOL      wRet;

lpBook = SS_BookLock(hWnd);
wRet = SS_GetEnhanceStaticCells(lpBook);
SS_BookUnlock(hWnd);
return (wRet);
}


BOOL SS_GetEnhanceStaticCells(LPSS_BOOK lpBook)
{
return lpBook->bEnhanceStaticCells;
}



WORD DLLENTRY SSSetHighlightHeaders(HWND hWnd, WORD wOption)
{
LPSS_BOOK lpBook;
WORD      wRet;

lpBook = SS_BookLock(hWnd);
wRet = SS_SetHighlightHeaders(lpBook, wOption);
SS_BookUnlock(hWnd);
return (wRet);
}


WORD SS_SetHighlightHeaders(LPSS_BOOK lpBook, WORD wOption)
{
WORD wRet;

wRet = lpBook->wHighlightHeaders;
if (wOption >= 0 && wOption <= 2)
	{
	lpBook->wHighlightHeaders = wOption;
	SS_InvalidateRect(lpBook, NULL, TRUE);
	}

return (wRet);
}


WORD DLLENTRY SSGetHighlightHeaders(HWND hWnd)
{
LPSS_BOOK lpBook;
WORD      wRet;

lpBook = SS_BookLock(hWnd);
wRet = SS_GetHighlightHeaders(lpBook);
SS_BookUnlock(hWnd);
return (wRet);
}


WORD SS_GetHighlightHeaders(LPSS_BOOK lpBook)
{
return lpBook->wHighlightHeaders;
}


WORD DLLENTRY SSSetHighlightStyle(HWND hWnd, WORD wStyle)
{
LPSS_BOOK lpBook;
WORD      wRet;

lpBook = SS_BookLock(hWnd);
wRet = SS_SetHighlightStyle(lpBook, wStyle);
SS_BookUnlock(hWnd);
return (wRet);
}


WORD SS_SetHighlightStyle(LPSS_BOOK lpBook, WORD wStyle)
{
WORD wRet;

wRet = lpBook->wHighlightStyle;
if (wStyle >= 0 && wStyle <= 3)
	{
	lpBook->wHighlightStyle = wStyle;
	SS_InvalidateRect(lpBook, NULL, TRUE);
	}

return (wRet);
}


WORD DLLENTRY SSGetHighlightStyle(HWND hWnd)
{
LPSS_BOOK lpBook;
WORD      wRet;

lpBook = SS_BookLock(hWnd);
wRet = SS_GetHighlightStyle(lpBook);
SS_BookUnlock(hWnd);
return (wRet);
}


WORD SS_GetHighlightStyle(LPSS_BOOK lpBook)
{
return lpBook->wHighlightStyle;
}


BOOL DLLENTRY SSSetHighlightAlphaBlend(HWND hWnd, COLORREF clrAlphaBlend, short nAlphaBlend)
{
LPSS_BOOK lpBook;
BOOL      fRet;

lpBook = SS_BookLock(hWnd);
fRet = SS_SetHighlightAlphaBlend(lpBook, clrAlphaBlend, nAlphaBlend);
SS_BookUnlock(hWnd);
return (fRet);
}


BOOL SS_SetHighlightAlphaBlend(LPSS_BOOK lpBook, COLORREF clrAlphaBlend, short nAlphaBlend)
{
BOOL fRet = TRUE;

lpBook->clrHighlightAlphaBlend = clrAlphaBlend;

if (nAlphaBlend >= -1 && nAlphaBlend <= 255)
	{
	if (nAlphaBlend == -1)
		lpBook->bHighlightAlphaBlend = SS_HIGHLIGHT_ALPHABLEND;
	else
		lpBook->bHighlightAlphaBlend = (BYTE)nAlphaBlend;

	SS_InvalidateRect(lpBook, NULL, TRUE);
	fRet = TRUE;
	}

SS_AlphaBlendFree(lpBook);

return (fRet);
}


BOOL DLLENTRY SSGetHighlightAlphaBlend(HWND hWnd, LPCOLORREF lpclrAlphaBlend, LPSHORT lpnAlphaBlend)
{
LPSS_BOOK lpBook;
BOOL      fRet;

lpBook = SS_BookLock(hWnd);
fRet = SS_GetHighlightAlphaBlend(lpBook, lpclrAlphaBlend, lpnAlphaBlend);
SS_BookUnlock(hWnd);
return (fRet);
}


BOOL SS_GetHighlightAlphaBlend(LPSS_BOOK lpBook, LPCOLORREF lpclrAlphaBlend, LPSHORT lpnAlphaBlend)
{
if (lpclrAlphaBlend)
	*lpclrAlphaBlend = lpBook->clrHighlightAlphaBlend;
if (lpnAlphaBlend)
	*lpnAlphaBlend = (short)lpBook->bHighlightAlphaBlend;

return (TRUE);
}

#endif // SS_V80


WORD DLLENTRY SSSetOperationMode(HWND hWnd, WORD wMode)
{
LPSPREADSHEET lpSS;
WORD          wModeOld;

lpSS = SS_SheetLock(hWnd);
#ifdef SPREAD_JPN
//- JPNFIX0018 - (Masanori Iwasa)
lpSS->lpBook->wMode = SS_MODE_NONE;
SS_ResetBlock(lpSS);
#endif
wModeOld = SS_SetOperationMode(lpSS, wMode);
SS_SheetUnlock(hWnd);
return wModeOld;
}


WORD SS_SetOperationMode(LPSPREADSHEET lpSS, WORD wMode)
{
WORD wModeOld;

wModeOld = lpSS->wOpMode;

if (wMode <= SS_OPMODE_EXTSEL && wMode != lpSS->wOpMode)
   {
   WORD wMessageBeingSent = lpSS->lpBook->wMessageBeingSent;
   lpSS->lpBook->wMessageBeingSent = FALSE;

   if (lpSS->lpBook->EditModeOn)
      SS_CellEditModeOff(lpSS, 0);

//   if (wMode == SS_OPMODE_READONLY)
      SS_HighlightCell(lpSS, FALSE);

   lpSS->fRowModeEditing = FALSE;

	/*
   if (wMode == SS_OPMODE_ROWMODE || wMode == SS_OPMODE_SINGLESEL ||
       wMode == SS_OPMODE_MULTISEL || wMode == SS_OPMODE_EXTSEL)
      lpSS->lpBook->wSelBlockOption = 0;
   else if (wMode == SS_OPMODE_NORMAL)
      lpSS->lpBook->wSelBlockOption |= SS_SELBLOCK_COLS | SS_SELBLOCK_ROWS |
                               SS_SELBLOCK_BLOCKS | SS_SELBLOCK_ALL;
	*/

	/*
   if (lpSS->lpBook->fEditModePermanent)
#ifdef SS_UTP
		if (!lpSS->fAllowEditModePermSel)
#endif
			lpSS->lpBook->wSelBlockOption = 0;
	*/

   // RFW - 4/26/00 - KEM59

   if ((wMode == SS_OPMODE_EXTSEL && wModeOld == SS_OPMODE_MULTISEL) ||
       ((wMode == SS_OPMODE_EXTSEL || wMode == SS_OPMODE_MULTISEL) && wModeOld == SS_OPMODE_SINGLESEL))
      ;
   else
      {
      lpSS->lpBook->wMode = SS_MODE_NONE;
		/* RFW - 6/2/04 - 14306
      SSx_ResetBlock(lpSS, FALSE);
		*/
      SSx_ResetBlock(lpSS, TRUE);
      }

   lpSS->wOpMode = wMode;

   if (wMode == SS_OPMODE_NORMAL)
      lpSS->lpBook->fFocusHighlightOn = FALSE;

/* RFW - 4/30/99 - SPR_EDT_001_004
   if (wMode == SS_OPMODE_MULTISEL || wModeOld == SS_OPMODE_MULTISEL ||
       wMode == SS_OPMODE_EXTSEL || wModeOld == SS_OPMODE_EXTSEL)
      SS_InvalidateScrollArea(lpSS, SS_SCROLLAREA_ALL);
   else if (wMode == SS_OPMODE_SINGLESEL || wModeOld == SS_OPMODE_SINGLESEL ||
            wMode == SS_OPMODE_ROWMODE || wModeOld == SS_OPMODE_ROWMODE)
      {
      lpSS->fChangingOperationMode = TRUE;
      SS_InvalidateRow(lpSS, lpSS->Row.CurAt);
      lpSS->fChangingOperationMode = FALSE;
      }
*/

   if (wMode == SS_OPMODE_MULTISEL || wModeOld == SS_OPMODE_MULTISEL ||
       wMode == SS_OPMODE_EXTSEL || wModeOld == SS_OPMODE_EXTSEL ||
       wMode == SS_OPMODE_SINGLESEL || wModeOld == SS_OPMODE_SINGLESEL ||
       wMode == SS_OPMODE_ROWMODE || wModeOld == SS_OPMODE_ROWMODE)
      SS_InvalidateScrollArea(lpSS, SS_SCROLLAREA_ALL);

//   if (wModeOld == SS_OPMODE_READONLY)
      SS_HighlightCell(lpSS, TRUE);

   lpSS->lpBook->wMessageBeingSent = wMessageBeingSent;
   }

return (wModeOld);
}


WORD DLLENTRY SSGetOperationMode(HWND hWnd)
{
LPSPREADSHEET lpSS;
WORD          wOpMode;

lpSS = SS_SheetLock(hWnd);
wOpMode = SS_GetOperationMode(lpSS);
SS_SheetUnlock(hWnd);

return (wOpMode);
}


WORD SS_GetOperationMode(LPSPREADSHEET lpSS)
{
return lpSS->wOpMode;
}


HWND DLLENTRY SSSetOwner(HWND hWnd, HWND hWndOwner)
{
LPSS_BOOK lpBook;
HWND      hWndOwnerOld;

lpBook = SS_BookLock(hWnd);
hWndOwnerOld = lpBook->hWndOwner;
lpBook->hWndOwner = hWndOwner;
SS_BookUnlock(hWnd);

return (hWndOwnerOld);
}


HWND DLLENTRY SSGetOwner(HWND hWnd)
{
LPSS_BOOK lpBook;
HWND      hWndOwnerOld;

lpBook = SS_BookLock(hWnd);
hWndOwnerOld = lpBook->hWndOwner;
SS_BookUnlock(hWnd);

return (hWndOwnerOld);
}


WNDPROC DLLENTRY SSSetCallBack(HWND hWnd, WNDPROC lpfnCallBack)
{
LPSS_BOOK lpBook;
WNDPROC   lpfnCallBackOld;

lpBook = SS_BookLock(hWnd);
lpfnCallBackOld = lpBook->lpfnCallBack;
lpBook->lpfnCallBack = lpfnCallBack;
SS_BookUnlock(hWnd);

return (lpfnCallBackOld);
}


WNDPROC DLLENTRY SSGetCallBack(HWND hWnd)
{
LPSS_BOOK lpBook;
WNDPROC   lpfnCallBackOld;

lpBook = SS_BookLock(hWnd);
lpfnCallBackOld = lpBook->lpfnCallBack;
SS_BookUnlock(hWnd);

return (lpfnCallBackOld);
}


void DLLENTRY SS_GetCellSendingMsg(LPSPREADSHEET lpSS, LPSS_COORD lpCol,
                                   LPSS_COORD lpRow)
{
if (lpCol)
   *lpCol = lpSS->lpBook->CurCellSendingMsg.Col;
if (lpRow)
   *lpRow = lpSS->lpBook->CurCellSendingMsg.Row;
}


void DLLENTRY SSGetCellSendingMsg(HWND hWnd, LPSS_COORD lpCol,
                                  LPSS_COORD lpRow)
{
LPSS_BOOK lpBook = SS_BookLock(hWnd);
LPSPREADSHEET lpSS;

lpSS = SS_BookLockSheetIndex(lpBook, lpBook->nSheetSendingMsg);
SS_GetCellSendingMsg(lpSS, lpCol, lpRow);
SS_AdjustCellCoordsOut(lpSS, lpCol, lpRow);
SS_BookUnlockSheetIndex(lpBook, lpBook->nSheetSendingMsg);
SS_BookUnlock(hWnd);
}


WORD DLLENTRY SSSetGridType(HWND hWnd, WORD wGridType)
{
LPSPREADSHEET lpSS;
WORD          wGridTypeOld;

lpSS = SS_SheetLock(hWnd);
wGridTypeOld = SS_SetGridType(lpSS, wGridType);
SS_SheetUnlock(hWnd);

return (wGridTypeOld);
}


WORD SS_SetGridType(LPSPREADSHEET lpSS, WORD wGridType)
{
WORD wGridTypeOld;

wGridTypeOld = lpSS->wGridType;
if (wGridTypeOld != wGridType)
   {
   lpSS->wGridType = wGridType;
   SS_InvalidateScrollArea(lpSS, SS_SCROLLAREA_ALL);
   }

return (wGridTypeOld);
}


WORD DLLENTRY SSGetGridType(HWND hWnd)
{
LPSPREADSHEET lpSS;
WORD          wGridTypeOld;

lpSS = SS_SheetLock(hWnd);
wGridTypeOld = SS_GetGridType(lpSS);
SS_SheetUnlock(hWnd);

return (wGridTypeOld);
}


WORD SS_GetGridType(LPSPREADSHEET lpSS)
{
return lpSS->wGridType;
}


COLORREF DLLENTRY SSSetGridColor(HWND hWnd, COLORREF GridColor)
{
LPSPREADSHEET lpSS;
COLORREF      GridColorOld;

lpSS = SS_SheetLock(hWnd);
GridColorOld = SS_SetGridColor(lpSS, GridColor);
SS_SheetUnlock(hWnd);

return (GridColorOld);
}


COLORREF SS_SetGridColor(LPSPREADSHEET lpSS, COLORREF GridColor)
{
COLORREF      GridColorOld;

GridColorOld = lpSS->GridColor;
if (GridColorOld != GridColor)
   {
   lpSS->GridColor = GridColor;
   SS_InvalidateScrollArea(lpSS, SS_SCROLLAREA_ALL);
   }

return (GridColorOld);
}


COLORREF DLLENTRY SSGetGridColor(HWND hWnd)
{
LPSPREADSHEET lpSS;
COLORREF      GridColorOld;

lpSS = SS_SheetLock(hWnd);
GridColorOld = SS_GetGridColor(lpSS);
SS_SheetUnlock(hWnd);

return (GridColorOld);
}


COLORREF SS_GetGridColor(LPSPREADSHEET lpSS)
{
return lpSS->GridColor;
}


WORD DLLENTRY SSSetUserResize(HWND hWnd, WORD wUserResize)
{
LPSPREADSHEET lpSS;
WORD          wUserResizeOld;

lpSS = SS_SheetLock(hWnd);
wUserResizeOld = SS_SetUserResize(lpSS, wUserResize);
SS_SheetUnlock(hWnd);
return wUserResizeOld;
}


WORD SS_SetUserResize(LPSPREADSHEET lpSS, WORD wUserResize)
{
WORD wUserResizeOld;

wUserResizeOld = lpSS->wUserResize;
lpSS->wUserResize = wUserResize;
return wUserResizeOld;
}


WORD DLLENTRY SSGetUserResize(HWND hWnd)
{
LPSPREADSHEET lpSS;
WORD          wUserResizeOld;

lpSS = SS_SheetLock(hWnd);
wUserResizeOld = SS_GetUserResize(lpSS);
SS_SheetUnlock(hWnd);

return (wUserResizeOld);
}


WORD SS_GetUserResize(LPSPREADSHEET lpSS)
{
return lpSS->wUserResize;
}


BOOL DLLENTRY SSGetDataCnt(HWND hWnd, LPSS_COORD lpColCnt,
                             LPSS_COORD lpRowCnt)
{
LPSPREADSHEET lpSS;
BOOL          bRet;

if (lpSS = SS_SheetLock(hWnd))
   {
   if (lpColCnt)
      *lpColCnt = lpSS->Col.DataCnt;

   if (lpRowCnt)
      *lpRowCnt = lpSS->Row.DataCnt;

   SS_AdjustCellCoordsOut(lpSS, lpColCnt, lpRowCnt);

   if (lpColCnt)
      *lpColCnt = max(0, *lpColCnt - 1);

   if (lpRowCnt)
      *lpRowCnt = max(0, *lpRowCnt - 1);

   SS_SheetUnlock(hWnd);
   bRet = TRUE;
   }

return bRet;
}


BOOL SS_GetDataCnt(LPSPREADSHEET lpSS, LPSS_COORD lpColCnt, LPSS_COORD lpRowCnt)
{
BOOL bRet = FALSE;

if (lpSS)
   {
   if (lpColCnt)
      *lpColCnt = lpSS->Col.DataCnt;

   if (lpRowCnt)
      *lpRowCnt = lpSS->Row.DataCnt;

   bRet = TRUE;
   }

return bRet;
}


#ifndef SS_GRID

BOOL DLLENTRY SSSetDataCnt(HWND hWnd, SS_COORD ColCnt, SS_COORD RowCnt)
{
LPSPREADSHEET lpSS;

if (lpSS = SS_SheetLock(hWnd))
   {
   SS_AdjustCellCoords(lpSS, &ColCnt, &RowCnt);
   if (ColCnt != -1)
      lpSS->Col.DataCnt = ColCnt;

   if (RowCnt != -1)
      lpSS->Row.DataCnt = RowCnt;

   SS_SheetUnlock(hWnd);
   return (TRUE);
   }

return (FALSE);
}


BOOL DLLENTRY SSSetCursor(HWND hWnd, WORD wType, HCURSOR hCursor)
{
LPSS_BOOK lpBook;
BOOL      bRet = FALSE;

if (lpBook = SS_BookLock(hWnd))
   {
   bRet = SS_SetCursor(lpBook, wType, hCursor, FALSE);
   SS_BookUnlock(hWnd);
   }
return bRet;
}


void SSx_SetCursor(LPSS_BOOK lpBook, LPSS_CURSOR lpCursor, HCURSOR hCursor, BOOL fDelete)
{
if (lpCursor->fDelete)
	{
	SS_PicDeref(lpBook, lpCursor->hCursor);
	lpCursor->fDelete = FALSE;
	}

lpCursor->hCursor = hCursor;

if (hCursor > 0 && hCursor != SS_CURSOR_ARROW && hCursor != SS_CURSOR_DEFCOLRESIZE &&
    hCursor != SS_CURSOR_DEFCOLRESIZE && fDelete)
	{
	SS_PicRef(lpBook, hCursor);
	lpCursor->fDelete = TRUE;
	}
}


BOOL SS_SetCursor(LPSS_BOOK lpBook, WORD wType, HCURSOR hCursor, BOOL fDelete)
{
if (lpBook)
   {
   if (wType & SS_CURSORTYPE_DEFAULT)
		SSx_SetCursor(lpBook, &lpBook->CursorDefault, hCursor, fDelete);

   if (wType & SS_CURSORTYPE_COLRESIZE)
		SSx_SetCursor(lpBook, &lpBook->CursorColResize, hCursor, fDelete);

   if (wType & SS_CURSORTYPE_ROWRESIZE)
		SSx_SetCursor(lpBook, &lpBook->CursorRowResize, hCursor, fDelete);

   if (wType & SS_CURSORTYPE_BUTTON)
		SSx_SetCursor(lpBook, &lpBook->CursorButton, hCursor, fDelete);

   if (wType & SS_CURSORTYPE_GRAYAREA)
		SSx_SetCursor(lpBook, &lpBook->CursorGrayArea, hCursor, fDelete);

   if (wType & SS_CURSORTYPE_LOCKEDCELL)
		SSx_SetCursor(lpBook, &lpBook->CursorLockedCell, hCursor, fDelete);

   if (wType & SS_CURSORTYPE_COLHEADER)
		SSx_SetCursor(lpBook, &lpBook->CursorColHeader, hCursor, fDelete);

   if (wType & SS_CURSORTYPE_ROWHEADER)
		SSx_SetCursor(lpBook, &lpBook->CursorRowHeader, hCursor, fDelete);

#ifdef SS_V30
   if (wType & SS_CURSORTYPE_DRAGDROPAREA)
		SSx_SetCursor(lpBook, &lpBook->CursorDragDropArea, hCursor, fDelete);

   if (wType & SS_CURSORTYPE_DRAGDROP)
		SSx_SetCursor(lpBook, &lpBook->CursorDragDrop, hCursor, fDelete);
#endif

   return (TRUE);
   }

return (FALSE);
}


HCURSOR DLLENTRY SSGetCursor(HWND hWnd, WORD wType)
{
LPSS_BOOK lpBook;
HCURSOR   hCursor = 0;

if (lpBook = SS_BookLock(hWnd))
   {
   hCursor = SS_GetCursor(lpBook, wType);
   SS_BookUnlock(hWnd);
   }
return hCursor;
}


HCURSOR SS_GetCursor(LPSS_BOOK lpBook, WORD wType)
{
HCURSOR hCursor = 0;

if (lpBook)
   {
   switch (wType)
      {
      case SS_CURSORTYPE_DEFAULT:
         hCursor = lpBook->CursorDefault.hCursor;
         break;

      case SS_CURSORTYPE_COLRESIZE:
         hCursor = lpBook->CursorColResize.hCursor;
         break;

      case SS_CURSORTYPE_ROWRESIZE:
         hCursor = lpBook->CursorRowResize.hCursor;
         break;

      case SS_CURSORTYPE_BUTTON:
         hCursor = lpBook->CursorButton.hCursor;
         break;

      case SS_CURSORTYPE_GRAYAREA:
         hCursor = lpBook->CursorGrayArea.hCursor;
         break;

      case SS_CURSORTYPE_LOCKEDCELL:
         hCursor = lpBook->CursorLockedCell.hCursor;
         break;

      case SS_CURSORTYPE_COLHEADER:
         hCursor = lpBook->CursorColHeader.hCursor;
         break;

      case SS_CURSORTYPE_ROWHEADER:
         hCursor = lpBook->CursorRowHeader.hCursor;
         break;

#ifdef SS_V30
      case SS_CURSORTYPE_DRAGDROPAREA:
         hCursor = lpBook->CursorDragDropArea.hCursor;
         break;

      case SS_CURSORTYPE_DRAGDROP:
         hCursor = lpBook->CursorDragDrop.hCursor;
         break;
#endif
      }
   }

return (hCursor);
}

#endif


short DLLENTRY SSSetUserResizeCol(HWND hWnd, SS_COORD Col, short dOption)
{
LPSPREADSHEET lpSS;
short         dRet = SS_RESIZE_DEFAULT;

if (lpSS = SS_SheetLock(hWnd))
   {
   SS_AdjustCellCoords(lpSS, &Col, NULL);
   dRet = SS_SetUserResizeCol(lpSS, Col, dOption);
   SS_SheetUnlock(hWnd);
   }
return dRet;
}


short SS_SetUserResizeCol(LPSPREADSHEET lpSS, SS_COORD Col, short dOption)
{
LPSS_COL lpCol;
short    dRet = SS_RESIZE_DEFAULT;
short    i;

if (lpSS)
   {
   if (Col == SS_ALLCOLS)
      {
      if (lpSS->wUserResize & SS_USERRESIZE_COL)
         dRet = SS_RESIZE_ON;
      else
         dRet = SS_RESIZE_OFF;

      if (dOption == SS_RESIZE_OFF)
         lpSS->wUserResize &= ~SS_USERRESIZE_COL;
      else
         lpSS->wUserResize |= SS_USERRESIZE_COL;

      for (i = 0; i < lpSS->Col.AllocCnt; i++)
         if (lpCol = SS_LockColItem(lpSS, i))
            {
            lpCol->bUserResize = SS_RESIZE_DEFAULT;
            SS_UnlockColItem(lpSS, i);
            }
      }

   else
      {
      if (Col >= SS_GetColCnt(lpSS))
         return (-1);

      if (lpCol = SS_AllocLockCol(lpSS, Col))
         {
         dRet = lpCol->bUserResize;
         lpCol->bUserResize = (BYTE)dOption;
         SS_UnlockColItem(lpSS, Col);
         }
      }
   }

return (dRet);
}


short DLLENTRY SSGetUserResizeCol(HWND hWnd, SS_COORD Col)
{
LPSPREADSHEET lpSS;
short         dRet = SS_RESIZE_DEFAULT;

if (lpSS = SS_SheetLock(hWnd))
   {
   SS_AdjustCellCoords(lpSS, &Col, NULL);
   dRet = SS_GetUserResizeCol(lpSS, Col);
   SS_SheetUnlock(hWnd);
   }
return dRet;
}


short SS_GetMergedUserResizeCol(LPSPREADSHEET lpSS, SS_COORD Col)
{
LPSS_COL lpCol;
short    dRet = SS_GetUserResizeCol(lpSS, Col);

// RFW - 3/9/04 - 13821
// I moved the following code here from SS_GetUserResizeCol
if (dRet != -1 && Col >= 0 && (lpCol = SS_LockColItem(lpSS, Col)))
   {
   if (lpCol->fColHidden)
      dRet = SS_RESIZE_OFF;

   SS_UnlockColItem(lpSS, Col);
   }

if (dRet == SS_RESIZE_DEFAULT && Col != -1)
	dRet = SS_GetUserResizeCol(lpSS, -1);

return (dRet);
}


short SS_GetUserResizeCol(LPSPREADSHEET lpSS, SS_COORD Col)
{
LPSS_COL lpCol;
short    dRet = SS_RESIZE_DEFAULT;

if (lpSS)
   {
   if (Col == SS_ALLCOLS)
      {
      if (lpSS->wUserResize & SS_USERRESIZE_COL)
         dRet = SS_RESIZE_ON;
      else
         dRet = SS_RESIZE_OFF;
      }

   else if (Col >= SS_GetColCnt(lpSS))
      dRet = -1;

   else if (lpCol = SS_LockColItem(lpSS, Col))
      {
      dRet = lpCol->bUserResize;
      SS_UnlockColItem(lpSS, Col);
      }
   }

return (dRet);
}


short DLLENTRY SSSetUserResizeRow(HWND hWnd, SS_COORD Row, short dOption)
{
LPSPREADSHEET lpSS;
short         dRet = SS_RESIZE_DEFAULT;
if (lpSS = SS_SheetLock(hWnd))
   {
   SS_AdjustCellCoords(lpSS, NULL, &Row);
   dRet = SS_SetUserResizeRow(lpSS, Row, dOption);
   SS_SheetUnlock(hWnd);
   }
return (dRet);
}


short SS_SetUserResizeRow(LPSPREADSHEET lpSS, SS_COORD Row, short dOption)
{
LPSS_ROW lpRow;
short    dRet = SS_RESIZE_DEFAULT;
short    i;

if (lpSS)
   {
   if (Row == SS_ALLROWS)
      {
      if (lpSS->wUserResize & SS_USERRESIZE_ROW)
         dRet = SS_RESIZE_ON;
      else
         dRet = SS_RESIZE_OFF;

      if (dOption == SS_RESIZE_OFF)
         lpSS->wUserResize &= ~SS_USERRESIZE_ROW;
      else
         lpSS->wUserResize |= SS_USERRESIZE_ROW;

      for (i = 0; i < lpSS->Row.AllocCnt; i++)
         if (lpRow = SS_LockRowItem(lpSS, i))
            {
            lpRow->bUserResize = SS_RESIZE_DEFAULT;
            SS_UnlockRowItem(lpSS, i);
            }
      }

   else
      {
      if (Row >= SS_GetRowCnt(lpSS))
         {
         return (-1);
         }

      if (lpRow = SS_AllocLockRow(lpSS, Row))
         {
         dRet = lpRow->bUserResize;
         lpRow->bUserResize = (BYTE)dOption;
         SS_UnlockRowItem(lpSS, Row);
         }
      }
   }

return (dRet);
}


short SS_GetMergedUserResizeRow(LPSPREADSHEET lpSS, SS_COORD Row)
{
short dRet = SS_GetUserResizeRow(lpSS, Row);

if (dRet == SS_RESIZE_DEFAULT)
	dRet = SS_GetUserResizeRow(lpSS, -1);

return (dRet);
}


short DLLENTRY SSGetUserResizeRow(HWND hWnd, SS_COORD Row)
{
LPSPREADSHEET lpSS;
short         dRet = SS_RESIZE_DEFAULT;

if (lpSS = SS_SheetLock(hWnd))
   {
   SS_AdjustCellCoords(lpSS, NULL, &Row);
   dRet = SS_GetUserResizeRow(lpSS, Row);
   SS_SheetUnlock(hWnd);
   }
return (dRet);
}


short SS_GetUserResizeRow(LPSPREADSHEET lpSS, SS_COORD Row)
{
LPSS_ROW      lpRow;
short         dRet = SS_RESIZE_DEFAULT;

if (lpSS)
   {
   if (Row == SS_ALLROWS)
      {
      if (lpSS->wUserResize & SS_USERRESIZE_ROW)
         dRet = SS_RESIZE_ON;
      else
         dRet = SS_RESIZE_OFF;
      }

   else if (Row >= SS_GetRowCnt(lpSS))
      dRet = -1;

   else if (lpRow = SS_LockRowItem(lpSS, Row))
      {
      if (lpRow->fRowHidden)
         dRet = SS_RESIZE_OFF;
      else
         dRet = lpRow->bUserResize;

      SS_UnlockRowItem(lpSS, Row);
      }
   }

return (dRet);
}


BOOL DLLENTRY SSGetCellRect(HWND hWnd, SS_COORD Col, SS_COORD Row,
                              LPRECT lpRect)
{
LPSPREADSHEET lpSS;
BOOL          bRet;

lpSS = SS_SheetLock(hWnd);
SS_AdjustCellCoords(lpSS, &Col, &Row);
bRet = SS_GetCellRect(lpSS, Col, Row, lpRect);
SS_SheetUnlock(hWnd);
return bRet;
}


BOOL SS_GetCellRect(LPSPREADSHEET lpSS, SS_COORD Col, SS_COORD Row,
                    LPRECT lpRect)
{
int  x;
int  y;
int  cx;
int  cy;
BOOL bRet = FALSE;

if (SS_IsCellVisible(lpSS, Col, Row, SS_VISIBLE_PARTIAL))
   {
   SS_GetCellCoord(lpSS, lpSS->Col.UL, lpSS->Row.UL, Col, Row,
                   &x, &y, &cx, &cy);
   if (lpRect)
      {
      lpRect->left = max(x, 0);
      lpRect->top = max(y, 0);
      lpRect->right = x + cx;
      lpRect->bottom = y + cy;
      }
   bRet = TRUE;
   }
return bRet;
}


BOOL DLLENTRY SSShowRow(HWND hWnd, SS_COORD Row, BOOL fShow)
{
LPSPREADSHEET lpSS;
BOOL          bRet;

lpSS = SS_SheetLock(hWnd);
SS_AdjustCellCoords(lpSS, NULL, &Row);
bRet = SS_ShowRow(lpSS, Row, fShow);
SS_SheetUnlock(hWnd);
return bRet;
}


BOOL SS_ShowRow(LPSPREADSHEET lpSS, SS_COORD Row, BOOL fShow)
{
LPSS_ROW lpRow;
SS_COORD i;
BOOL     fChangeMade = FALSE;

SS_CellEditModeOff(lpSS, 0);

if (lpRow = SS_AllocLockRow(lpSS, Row))
   {
   if ((lpRow->fRowHidden == 0) != !(fShow == 0))
      {
		fChangeMade = TRUE;
      lpRow->fRowHidden = !fShow;
      if (fShow && lpRow->dRowHeightInPixels == 0)
         SS_SetRowHeight(lpSS, Row, SS_HEIGHT_DEFAULT);
      }

   SS_UnlockRowItem(lpSS, Row);
   }

if (fShow && SS_ALLCOLS == Row)
   {
	fChangeMade = TRUE;
   for (i = lpSS->Row.HeaderCnt; i < lpSS->Row.AllocCnt; i++)
      {
      if (lpRow = SS_LockRowItem(lpSS, i))
         {
         lpRow->fRowHidden = FALSE;
         if (lpRow->dRowHeightInPixels == 0)
            SS_SetRowHeight(lpSS, i, SS_HEIGHT_DEFAULT);

         SS_UnlockRowItem(lpSS, i);
         }
      }
   }

if (fChangeMade)
   {
   if (!SS_AutoSize(lpSS->lpBook, FALSE))
		{
      if (Row == -1)
         SS_InvalidateScrollArea(lpSS, SS_SCROLLAREA_ALL);

      else if (SS_IsRowVisible(lpSS, Row, SS_VISIBLE_PARTIAL))
         SS_InvalidateRowRange(lpSS, Row, -1);
		}

   if (Row >= lpSS->Row.HeaderCnt && !fShow && (lpSS->Row.CurAt == Row || lpSS->Row.UL == Row))
		if (lpSS->lpBook->hWnd)
			PostMessage(lpSS->lpBook->hWnd, SSM_UNHIDEACTIVEROW, lpSS->nSheetIndex, (LPARAM)Row); // Internal message
   }

return (TRUE);
}


BOOL DLLENTRY SSIsRowHidden(HWND hWnd, SS_COORD Row)
{
LPSPREADSHEET lpSS;
BOOL          bRet;

lpSS = SS_SheetLock(hWnd);
SS_AdjustCellCoords(lpSS, NULL, &Row);
bRet = SS_IsRowHidden(lpSS, Row);
SS_SheetUnlock(hWnd);
return bRet;
}


BOOL SS_IsRowHidden(LPSPREADSHEET lpSS, SS_COORD Row)
{
LPSS_ROW lpRow;
BOOL     fRet = FALSE;

if (lpRow = SS_LockRowItem(lpSS, Row))
   {
   fRet = lpRow->fRowHidden;
   SS_UnlockRowItem(lpSS, Row);
   }

return (fRet);
}


BOOL DLLENTRY SSShowCol(HWND hWnd, SS_COORD Col, BOOL fShow)
{
LPSPREADSHEET lpSS;
BOOL          bRet;

lpSS = SS_SheetLock(hWnd);
SS_AdjustCellCoords(lpSS, &Col, NULL);
bRet = SS_ShowCol(lpSS, Col, fShow);
SS_SheetUnlock(hWnd);
return bRet;
}


BOOL SS_ShowCol(LPSPREADSHEET lpSS, SS_COORD Col, BOOL fShow)
{
LPSS_COL lpCol;
SS_COORD i;

if (lpCol = SS_AllocLockCol(lpSS, Col))
   {
   if ((lpCol->fColHidden == 0) != !(fShow == 0))
      {
      lpCol->fColHidden = !fShow;
      if (fShow && lpCol->dColWidthInPixels == 0)
         SS_SetColWidth(lpSS, Col, SS_WIDTH_DEFAULT);
      }

   SS_UnlockColItem(lpSS, Col);

   if (lpSS->lpBook->fAllowCellOverflow)
      for (i = lpSS->Row.HeaderCnt; i < lpSS->Row.DataCnt; i++)
         SS_OverflowAdjustNeighbor(lpSS, Col, i);
   }

if (fShow && SS_ALLCOLS == Col)
   {
   for (i = lpSS->Col.HeaderCnt; i < lpSS->Col.AllocCnt; i++)
      {
      if (lpCol = SS_LockColItem(lpSS, i))
         {
         lpCol->fColHidden = FALSE;
         if (lpCol->dColWidthInPixels == 0)
            SS_SetColWidth(lpSS, i, SS_WIDTH_DEFAULT);

         SS_UnlockColItem(lpSS, i);
         }
      }
   }

SS_CellEditModeOff(lpSS, 0);

if (!SS_AutoSize(lpSS->lpBook, FALSE))
   {
   SS_SetHScrollBar(lpSS);

   if (Col == -1)
      SS_InvalidateScrollArea(lpSS, SS_SCROLLAREA_ALL);

   else if (SS_IsColVisible(lpSS, Col, SS_VISIBLE_PARTIAL))
      SS_InvalidateColRange(lpSS, Col, -1);
   }

#ifdef SS_UTP
if (!lpSS->fUseScrollArrows || Col != lpSS->Col.HeaderCnt +
    lpSS->Col.Frozen - 1)
#endif

if (Col >= lpSS->Col.HeaderCnt && !fShow && (lpSS->Col.CurAt == Col || lpSS->Col.UL == Col))
	if (lpSS->lpBook->hWnd)
		PostMessage(lpSS->lpBook->hWnd, SSM_UNHIDEACTIVECOL, lpSS->nSheetIndex, (LPARAM)Col); // Internal message
	else // RFW - 1/5/05 - 15435
		SS_UnhideActiveCol(lpSS, Col);

return (TRUE);
}


BOOL DLLENTRY SSIsColHidden(HWND hWnd, SS_COORD Col)
{
LPSPREADSHEET lpSS;
BOOL          bRet;

lpSS = SS_SheetLock(hWnd);
SS_AdjustCellCoords(lpSS, &Col, NULL);
bRet = SS_IsColHidden(lpSS, Col);
SS_SheetUnlock(hWnd);
return bRet;
}


BOOL SS_IsColHidden(LPSPREADSHEET lpSS, SS_COORD Col)
{
LPSS_COL lpCol;
BOOL     fRet = FALSE;

if (lpCol = SS_LockColItem(lpSS, Col))
   {
   fRet = lpCol->fColHidden;
   SS_UnlockColItem(lpSS, Col);
   }

return (fRet);
}


#ifndef SS_GRID

BOOL DLLENTRY SSSetDefFloatFormat(HWND hWnd, LPFLOATFORMAT lpFloatFormat)
{
LPSS_BOOK lpBook;
BOOL          fRet;

lpBook = SS_BookLock(hWnd);
fRet = SS_SetDefFloatFormat(lpBook, lpFloatFormat);
SS_BookUnlock(hWnd);
return (fRet);
}


BOOL SS_SetDefFloatFormat(LPSS_BOOK lpBook, LPFLOATFORMAT lpFloatFormat)
{
_fmemcpy(&lpBook->DefaultFloatFormat, lpFloatFormat, sizeof(FLOATFORMAT));
SS_InvalidateRect(lpBook, NULL, TRUE);
return TRUE;
}

#endif


BOOL DLLENTRY SSGetDefFloatFormat(HWND hWnd, LPFLOATFORMAT lpFloatFormat)
{
LPSS_BOOK lpBook;
BOOL      fRet;

lpBook = SS_BookLock(hWnd);
fRet = SS_GetDefFloatFormat(lpBook, lpFloatFormat);
SS_BookUnlock(hWnd);
return (fRet);
}


BOOL SS_GetDefFloatFormat(LPSS_BOOK lpBook, LPFLOATFORMAT lpFloatFormat)
{
SSx_GetDefFloatFormat(lpBook, lpFloatFormat);
if (!lpFloatFormat->cCurrencySign)
  lpFloatFormat->cCurrencySign = lpBook->WinFloatFormat.cCurrencySign;
if (!lpFloatFormat->cDecimalSign)
  lpFloatFormat->cDecimalSign = lpBook->WinFloatFormat.cDecimalSign;
if (!lpFloatFormat->cSeparator)
  lpFloatFormat->cSeparator = lpBook->WinFloatFormat.cSeparator;
return TRUE;
}


BOOL SSx_GetDefFloatFormat(LPSS_BOOK lpBook, LPFLOATFORMAT lpFloatFormat)
{
_fmemcpy(lpFloatFormat, &lpBook->DefaultFloatFormat, sizeof(FLOATFORMAT));
return TRUE;
}


void DLLENTRY SSGetClientRect(HWND hWnd, LPRECT lpRect)
{
LPSS_BOOK lpBook;

lpBook = SS_BookLock(hWnd);
SS_GetClientRect(lpBook, lpRect);
SS_BookUnlock(hWnd);
}


void DLLENTRY SSGetTopLeftPrev(HWND hWnd, LPSS_COORD lpCol, LPSS_COORD lpRow)
{
LPSPREADSHEET lpSS;

lpSS = SS_SheetLock(hWnd);
SS_AdjustCellCoordsOut(lpSS, lpCol, lpRow);
SS_SheetUnlock(hWnd);
}


void SS_GetTopLeftPrev(LPSPREADSHEET lpSS, LPSS_COORD lpCol, LPSS_COORD lpRow)
{
if (lpCol)
   *lpCol = lpSS->Col.ULPrev;
if (lpRow)
   *lpRow = lpSS->Row.ULPrev;
}


#ifndef SS_GRID


WORD DLLENTRY SSSetColHeaderDisplay(HWND hWnd, WORD wDisplay)
{
LPSPREADSHEET lpSS;
WORD          wDisplayOld;

lpSS = SS_SheetLock(hWnd);
wDisplayOld = SS_SetColHeaderDisplay(lpSS, wDisplay);
SS_SheetUnlock(hWnd);
return wDisplayOld;
}

WORD SS_SetColHeaderDisplay(LPSPREADSHEET lpSS, WORD wDisplay)
{
WORD          wDisplayOld;

wDisplayOld = lpSS->ColHeaderDisplay;
lpSS->ColHeaderDisplay = wDisplay;
SS_InvalidateRowRange(lpSS, 0, lpSS->Row.HeaderCnt);
return wDisplayOld;
}


WORD DLLENTRY SSGetColHeaderDisplay(HWND hWnd)
{
LPSPREADSHEET lpSS;
WORD          wDisplayOld;

lpSS = SS_SheetLock(hWnd);
wDisplayOld = SS_GetColHeaderDisplay(lpSS);
SS_SheetUnlock(hWnd);
return (wDisplayOld);
}


WORD SS_GetColHeaderDisplay(LPSPREADSHEET lpSS)
{
return lpSS->ColHeaderDisplay;
}


WORD DLLENTRY SSSetRowHeaderDisplay(HWND hWnd, WORD wDisplay)
{
LPSPREADSHEET lpSS;
WORD          wDisplayOld;

lpSS = SS_SheetLock(hWnd);
wDisplayOld = SS_SetRowHeaderDisplay(lpSS, wDisplay);
SS_SheetUnlock(hWnd);
return (wDisplayOld);
}


WORD SS_SetRowHeaderDisplay(LPSPREADSHEET lpSS, WORD wDisplay)
{
WORD          wDisplayOld;

wDisplayOld = lpSS->RowHeaderDisplay;
lpSS->RowHeaderDisplay = wDisplay;
SS_InvalidateColRange(lpSS, 0, lpSS->Col.HeaderCnt);
return (wDisplayOld);
}


WORD DLLENTRY SSGetRowHeaderDisplay(HWND hWnd)
{
LPSPREADSHEET lpSS;
WORD          wDisplayOld;

lpSS = SS_SheetLock(hWnd);
wDisplayOld = SS_GetRowHeaderDisplay(lpSS);
SS_SheetUnlock(hWnd);
return (wDisplayOld);
}


WORD SS_GetRowHeaderDisplay(LPSPREADSHEET lpSS)
{
return lpSS->RowHeaderDisplay;
}


void DLLENTRY SSSetStartingNumbers(HWND hWnd, SS_COORD StartingColNumber,
                                   SS_COORD StartingRowNumber)
{
LPSPREADSHEET lpSS;

lpSS = SS_SheetLock(hWnd);
SS_SetStartingNumbers(lpSS, StartingColNumber, StartingRowNumber);
SS_SheetUnlock(hWnd);
}


void SS_SetStartingNumbers(LPSPREADSHEET lpSS, SS_COORD StartingColNumber,
                           SS_COORD StartingRowNumber)
{
SS_COORD      StartingColNumberOld;
SS_COORD      StartingRowNumberOld;

StartingColNumberOld = lpSS->Col.NumStart;
StartingRowNumberOld = lpSS->Row.NumStart;

lpSS->Col.NumStart = StartingColNumber;
lpSS->Row.NumStart = StartingRowNumber;

if (StartingColNumberOld != lpSS->Col.NumStart)
   SS_InvalidateRowRange(lpSS, 0, lpSS->Row.HeaderCnt);

if (StartingRowNumberOld != lpSS->Row.NumStart)
   SS_InvalidateColRange(lpSS, 0, lpSS->Col.HeaderCnt);
}


void DLLENTRY SSGetStartingNumbers(HWND hWnd, LPSS_COORD lpStartingColNumber,
                                   LPSS_COORD lpStartingRowNumber)
{
LPSPREADSHEET lpSS;

lpSS = SS_SheetLock(hWnd);
SS_GetStartingNumbers(lpSS, lpStartingColNumber, lpStartingRowNumber);
SS_SheetUnlock(hWnd);
}


void SS_GetStartingNumbers(LPSPREADSHEET lpSS, LPSS_COORD lpStartingColNumber,
                           LPSS_COORD lpStartingRowNumber)
{
if (lpStartingColNumber)
   *lpStartingColNumber = lpSS->Col.NumStart;

if (lpStartingRowNumber)
   *lpStartingRowNumber = lpSS->Row.NumStart;
}

#endif


#if 0
void DLLENTRY SSReset(HWND hWnd)
{
LPSS_BOOK lpBook;
BOOL      fRedrawOld;
HDC       hDC = NULL;
#if defined(SS_RSDLL)
BOOL      bDesignTime;
#endif

lpBook = SS_BookLock(hWnd);

fRedrawOld = lpBook->Redraw;
lpBook->Redraw = FALSE;
#if defined(SS_RSDLL)
bDesignTime = SS_DdeGetDesignTime(lpBook);
#endif

// BJO 20Aug96 GRB2536 - Begin fix
if (lpBook->fUseSpecialVScroll)
   SS_VScrollSetSpecial(lpBook, FALSE, 0);
// BJO 20Aug96 GRB2536 - End fix

SS_ClearAll(lpBook);
hDC = fpGetDC(hWnd);  //?? OCX needs AmbientForeColor & AmbientBackColor
SS_InitSpreadPreHwnd(lpBook, GetTextColor(hDC), GetBkColor(hDC));
ReleaseDC(hWnd, hDC);
SS_InitSpreadWithHwnd(hWnd, lpBook);

#if defined(SS_RSDLL)
SS_DdeSetDesignTime(lpBook, bDesignTime);
#endif
SS_InvalidateRect(lpBook, NULL, TRUE);
SS_BookSetRedraw(lpBook, fRedrawOld);
// CTF - after reset, reset scrollbar rectangles
SendMessage(hWnd, WM_SIZE, SIZE_RESTORED, MAKELPARAM(lpBook->dClientWidth, lpBook->dClientHeight));
SS_BookUnlock(hWnd);
}
#endif


void DLLENTRY SSReset(HWND hWnd)
{
LPSS_BOOK lpBook;

lpBook = SS_BookLock(hWnd);
SS_Reset(lpBook);
SS_BookUnlock(hWnd);
}


void SS_Reset(LPSS_BOOK lpBook)
{
HWND hwndOwner = lpBook->hWndOwner;
BOOL fRedrawOld = lpBook->Redraw;
#if defined(SS_RSDLL)
BOOL bDesignTime = SS_DdeGetDesignTime(lpBook);
#endif

lpBook->Redraw = FALSE;

if (lpBook->fUseSpecialVScroll)
  SS_VScrollSetSpecial(lpBook, FALSE, 0);

SS_ClearAll(lpBook);

SS_InitSpreadPreHwnd(lpBook, RGB(0,0,0), RGB(255,255,255)); // need real colors
if( lpBook->hWnd )
  SS_InitSpreadWithHwnd(lpBook->hWnd, lpBook);
lpBook->hWndOwner = hwndOwner;

#if defined(SS_VB)
SS_VbxResetOnLoad(lpSS);
#endif
#if defined(SS_RSDLL)
SS_DdeSetDesignTime(lpBook, bDesignTime);
#endif

SS_InvalidateRect(lpBook, NULL, TRUE);
SS_BookSetRedraw(lpBook, fRedrawOld);
// CTF - after reset, reset scrollbar rectangles
SendMessage(lpBook->hWnd, WM_SIZE, SIZE_RESTORED, MAKELPARAM(lpBook->dClientWidth, lpBook->dClientHeight));
#ifdef SS_V70
SS_SetScrollBarSize(lpBook, -1, -1);
#endif // SS_V70
}


#ifdef SS_V70
void DLLENTRY SSResetSheet(HWND hWnd, short nSheetIndex)
{
LPSS_BOOK lpBook = SS_BookLock(hWnd);
SS_ResetSheet(lpBook, (short)(nSheetIndex - 1));
SS_BookUnlock(hWnd);
}
#endif SS_V70


BOOL SS_ResetSheet(LPSS_BOOK lpBook, short nSheetIndex)
{
LPSPREADSHEET  lpSS;
TBGLOBALHANDLE hSS;
BOOL           fRet = FALSE;

if (nSheetIndex >= 0 && nSheetIndex < lpBook->nSheetCnt)
	{
	lpSS = SS_BookLockSheetIndex(lpBook, nSheetIndex);
	SS_ClearSheet(lpSS, FALSE);
	SS_BookUnlockSheetIndex(lpBook, nSheetIndex);

	hSS = SS_BookGetSheetHandleFromIndex(lpBook, nSheetIndex);
	SS_InitSheetPreHwnd(lpBook, hSS, nSheetIndex, FALSE);
	CalcResetSheet(&lpSS->CalcInfo);
	if (lpBook->CalcAuto)
		CalcEvalNeededCells(&lpBook->CalcInfo);
	fRet = TRUE;
	}

return (fRet);
}


WORD DLLENTRY SSSetBackColorStyle(HWND hWnd, WORD wStyle)
{
LPSS_BOOK lpBook;
WORD      wStyleOld;

lpBook = SS_BookLock(hWnd);
wStyleOld = SS_SetBackColorStyle(lpBook, wStyle);
SS_BookUnlock(hWnd);
return (wStyleOld);
}


WORD SS_SetBackColorStyle(LPSS_BOOK lpBook, WORD wStyle)
{
WORD wStyleOld;

wStyleOld = (WORD)lpBook->bBackColorStyle;

if (wStyle != wStyleOld)
	{
	if (SS_BACKCOLORSTYLE_OVERGRID == wStyle ||
		 SS_BACKCOLORSTYLE_UNDERGRID == wStyle ||
		 SS_BACKCOLORSTYLE_OVERHORZGRIDONLY == wStyle ||
		 SS_BACKCOLORSTYLE_OVERVERTGRIDONLY == wStyle)
		{
		lpBook->bBackColorStyle = (BYTE)wStyle;
		SS_InvalidateRect(lpBook, NULL, TRUE);
		}
	}

return (wStyleOld);
}


WORD DLLENTRY SSGetBackColorStyle(HWND hWnd)
{
LPSS_BOOK lpBook;
WORD      wStyleOld;

lpBook = SS_BookLock(hWnd);
wStyleOld = SS_GetBackColorStyle(lpBook);
SS_BookUnlock(hWnd);
return (wStyleOld);
}


WORD SS_GetBackColorStyle(LPSS_BOOK lpBook)
{
return ((WORD)lpBook->bBackColorStyle);
}


BOOL SSIsActionKey(HWND hWnd, WORD wAction, WORD wKey)
{
BOOL fRet;
#ifndef SS_V30
fRet = SS_IsActionKey(NULL, wAction, wKey);
#else
LPSS_BOOK lpBook = SS_BookLock(hWnd);

fRet = SS_IsActionKey(lpBook, wAction, wKey);
SS_BookUnlock(hWnd);
#endif
return (fRet);
}


BOOL SS_IsActionKey(LPSS_BOOK lpBook, WORD wAction, WORD wKey)
{
BOOL fRet = FALSE;

if (wAction < SS_KBA_CNT)
   {
#ifndef SS_V30
   long lValue;

   switch (wAction)
      {
      case SS_KBA_CLEAR:
         GetToolBoxGlobal(TBG_CLEARFIELD, &lValue);
         break;

      case SS_KBA_CURRENT:
         GetToolBoxGlobal(TBG_CURRENTDATETIME, &lValue);
         break;

      case SS_KBA_POPUP:
         GetToolBoxGlobal(TBG_POPUPCALENDAR, &lValue);
         break;
      }

   if ((wKey == 0 && lValue) || wKey == (WORD)lValue)
      fRet = TRUE;

#else
   DWORD dwKey = lpBook->ActionKeys[wAction];

   /****************************************************
   * If wKey is passed in as 0 then simply return true
   * if the Action Key is set to a valid key, which
   * is anything greater than 0.
   ****************************************************/

   if (wKey == 0 && dwKey)
      fRet = TRUE;

   else if (wKey == LOWORD(dwKey))
      {
      if (((GetKeyState(VK_SHIFT) & 0x8000) ? 1 : 0) ==
          ((dwKey & SS_KBS_SHIFT) ? 1 : 0) &&
          ((GetKeyState(VK_CONTROL) & 0x8000) ? 1 : 0) ==
          ((dwKey & SS_KBS_CTRL) ? 1 : 0))
         fRet = TRUE;
      }
#endif
   }

return (fRet);
}


#ifdef SS_V30

BOOL DLLENTRY SSSetActionKey(HWND hWnd, WORD wAction, BOOL fShift, BOOL fCtrl, WORD wKey)
{
LPSS_BOOK lpBook;
BOOL      fRet;

lpBook = SS_BookLock(hWnd);
fRet = SS_SetActionKey(lpBook, wAction, fShift, fCtrl, wKey);
SS_BookUnlock(hWnd);
return (fRet);
}


BOOL SS_SetActionKey(LPSS_BOOK lpBook, WORD wAction, BOOL fShift, BOOL fCtrl, WORD wKey)
{
BOOL  fRet = FALSE;
DWORD dwKey = (DWORD)wKey;

if (fShift)
   dwKey |= SS_KBS_SHIFT;

if (fCtrl)
   dwKey |= SS_KBS_CTRL;

if (wAction < SS_KBA_CNT)
   {
   // RFW - 7/5/00 - GIC12161
   // lpSS->ActionKeys[wAction] = toupper(dwKey);
   lpBook->ActionKeys[wAction] = dwKey;
   fRet = TRUE;
   }

return (fRet);
}


BOOL DLLENTRY SSGetActionKey(HWND hWnd, WORD wAction, LPBOOL lpfShift, LPBOOL lpfCtrl, LPWORD lpwKey)
{
LPSS_BOOK lpBook;
BOOL      fRet;

lpBook = SS_BookLock(hWnd);
fRet = SS_GetActionKey(lpBook, wAction, lpfShift, lpfCtrl, lpwKey);
SS_BookUnlock(hWnd);
return (fRet);
}


BOOL SS_GetActionKey(LPSS_BOOK lpBook, WORD wAction, LPBOOL lpfShift, LPBOOL lpfCtrl, LPWORD lpwKey)
{
DWORD dwKey;
BOOL  fRet = FALSE;

if (wAction < SS_KBA_CNT)
   {
   dwKey = lpBook->ActionKeys[wAction];

   if (lpfShift)
      *lpfShift = ((dwKey & SS_KBS_SHIFT) ? TRUE : FALSE);

   if (lpfCtrl)
      *lpfCtrl = ((dwKey & SS_KBS_CTRL) ? TRUE : FALSE);

   if (lpwKey)
      *lpwKey = LOWORD(dwKey);

   fRet = TRUE;
   }

return (fRet);
}


BOOL DLLENTRY SSSetOddEvenRowColor(HWND hWnd, COLORREF clrBackOdd, COLORREF clrForeOdd, COLORREF clrBackEven, COLORREF clrForeEven)
{
LPSPREADSHEET lpSS;
BOOL          fRet;

lpSS = SS_SheetLock(hWnd);
fRet = SS_SetOddEvenRowColor(lpSS, clrBackOdd, clrForeOdd, clrBackEven, clrForeEven);
SS_SheetUnlock(hWnd);
return (fRet);
}


BOOL SS_SetOddEvenRowColor(LPSPREADSHEET lpSS, COLORREF clrBackOdd, COLORREF clrForeOdd, COLORREF clrBackEven, COLORREF clrForeEven)
{
BOOL fRet = TRUE;

lpSS->OddRowColor.BackgroundId  = SS_AddColor(clrBackOdd);
lpSS->OddRowColor.ForegroundId  = SS_AddColor(clrForeOdd);

lpSS->EvenRowColor.BackgroundId  = SS_AddColor(clrBackEven);
lpSS->EvenRowColor.ForegroundId  = SS_AddColor(clrForeEven);

SS_InvalidateScrollArea(lpSS, SS_SCROLLAREA_ALL);

return (fRet);
}


BOOL DLLENTRY SSGetOddEvenRowColor(HWND hWnd, LPCOLORREF lpclrBackOdd, LPCOLORREF lpclrForeOdd, LPCOLORREF lpclrBackEven, LPCOLORREF lpclrForeEven)
{
LPSPREADSHEET lpSS;
BOOL          fRet;

lpSS = SS_SheetLock(hWnd);
fRet = SS_GetOddEvenRowColor(lpSS, lpclrBackOdd, lpclrForeOdd, lpclrBackEven, lpclrForeEven);
SS_SheetUnlock(hWnd);
return (fRet);
}


BOOL SS_GetOddEvenRowColor(LPSPREADSHEET lpSS, LPCOLORREF lpclrBackOdd, LPCOLORREF lpclrForeOdd, LPCOLORREF lpclrBackEven, LPCOLORREF lpclrForeEven)
{
SS_COLORTBLITEM ColorTblItem;
BOOL            fRet = TRUE;

if (lpclrBackOdd)
   {
   SS_GetColorItem(&ColorTblItem, lpSS->OddRowColor.BackgroundId);
   *lpclrBackOdd = ColorTblItem.Color;
   }

if (lpclrForeOdd)
   {
   SS_GetColorItem(&ColorTblItem, lpSS->OddRowColor.ForegroundId);
   *lpclrForeOdd = ColorTblItem.Color;
   }

if (lpclrBackEven)
   {
   SS_GetColorItem(&ColorTblItem, lpSS->EvenRowColor.BackgroundId);
   *lpclrBackEven = ColorTblItem.Color;
   }

if (lpclrForeEven)
   {
   SS_GetColorItem(&ColorTblItem, lpSS->EvenRowColor.ForegroundId);
   *lpclrForeEven = ColorTblItem.Color;
   }

return (fRet);
}


WORD DLLENTRY SSSetAppearance(HWND hWnd, WORD wAppearance)
{
LPSS_BOOK lpBook;
WORD      wPrevSetting;

lpBook = SS_BookLock(hWnd);
wPrevSetting = SS_SetAppearance(lpBook, wAppearance);
SS_BookUnlock(hWnd);
return (wPrevSetting);
}


WORD SS_SetAppearance(LPSS_BOOK lpBook, WORD wAppearance)
{
WORD wPrevSetting = (WORD)lpBook->bAppearance;

#ifdef SS_DLL
if (wAppearance >= 0 && wAppearance <= SS_APPEARANCE_DEFAULT)
#else
if (wAppearance >= 0 && wAppearance <= SS_APPEARANCE_3DWITHBORDER)
#endif
   {
   if (lpBook->bAppearance != (BYTE)wAppearance)
		{
		lpBook->bAppearance = (BYTE)wAppearance;
		if (lpBook->hWnd)
			SetWindowPos(lpBook->hWnd, NULL, 0, 0, 0, 0, SWP_NOACTIVATE | SWP_NOSIZE |
							 SWP_NOMOVE | SWP_DRAWFRAME | SWP_NOZORDER);
		}
   }

return (wPrevSetting);
}


WORD DLLENTRY SSGetAppearance(HWND hWnd)
{
LPSS_BOOK lpBook;
WORD      wAppearance;

lpBook = SS_BookLock(hWnd);
wAppearance = SS_GetAppearance(lpBook);
SS_BookUnlock(hWnd);
return (wAppearance);
}


WORD SS_GetAppearance(LPSS_BOOK lpBook)
{
return (lpBook->bAppearance);
}


BOOL DLLENTRY SSSetTextTip(HWND hWnd, WORD wStatus, LONG lDelay,
                           LPLOGFONT lpLogFont, COLORREF clrBack,
                           COLORREF clrFore)
{
FP_TT_INFO Info;
BOOL       fRet;
WORD       wItem = FP_TT_ITEM_STATUS | FP_TT_ITEM_DELAY |
                   FP_TT_ITEM_BACKCOLOR | FP_TT_ITEM_FORECOLOR;

// fix for GRB7118 -scl
// don't allow a NULL hWnd
if (!hWnd)
	return FALSE;
// fix for GRB7118 -scl

Info.wStatus = wStatus;
Info.lDelay = lDelay;
if (lpLogFont)
   {
   Info.LogFont = *lpLogFont;
   wItem |= FP_TT_ITEM_FONT;
   }

#ifdef SS_V35
Info.fShowScrollTips = FALSE;
#endif
Info.clrBack = clrBack;
Info.clrFore = clrFore;
fRet = fpTextTipSetInfo(hWnd, wItem, &Info);
return (fRet);
}


BOOL DLLENTRY SSGetTextTip(HWND hWnd, LPWORD lpwStatus, LPLONG lplDelay,
                           LPLOGFONT lpLogFont, LPCOLORREF lpclrBack,
                           LPCOLORREF lpclrFore)
{
FP_TT_INFO Info;
BOOL       fRet;

fRet = fpTextTipGetInfo(hWnd, &Info);
if (lpwStatus)
   *lpwStatus = Info.wStatus;
if (lplDelay)
   *lplDelay = Info.lDelay;
if (lpLogFont)
   *lpLogFont = Info.LogFont;
if (lpclrBack)
   *lpclrBack = Info.clrBack;
if (lpclrFore)
   *lpclrFore = Info.clrFore;

return (fRet);
}


#ifdef SS_V35

BOOL SS_SetColCellNote(LPSPREADSHEET lpSS, SS_COORD Col, LPTSTR Note)
{
LPSS_COL       lpCol;
LPSS_CELL      lpCell;
LPTSTR         lpszNote;
SS_COORD       i;

for (i = (lpSS->fVirtualMode ? lpSS->Virtual.VTop : 0);
     i < (lpSS->fVirtualMode ? lpSS->Row.DataCnt : lpSS->Row.AllocCnt); i++)
   if (lpCell = SS_LockCellItem(lpSS, Col, i))
      {
      if (lpCell->hCellNote)
         {
         tbGlobalFree(lpCell->hCellNote);
         lpCell->hCellNote = 0;
         }

      SS_UnlockCellItem(lpSS, Col, i);
      }

if (lpCol = SS_AllocLockCol(lpSS, Col))
   {
   if (lpCol->hCellNote)
   {
     tbGlobalFree(lpCol->hCellNote);
     lpCol->hCellNote = 0;
   } 
   if (Note && Note[0])
      {
      if (!(lpCol->hCellNote = tbGlobalAlloc(GMEM_MOVEABLE | GMEM_ZEROINIT,
                                             (long)(lstrlen(Note)+1)*sizeof(TCHAR))))
         {
         SS_UnlockColItem(lpSS, Col);
         return (FALSE);
         }
      
      lpszNote = (LPTSTR)tbGlobalLock(lpCol->hCellNote);
      _tcscpy(lpszNote, Note);
      tbGlobalUnlock(lpCol->hCellNote);
      }

   SS_InvalidateCol(lpSS, Col);
   SS_UnlockColItem(lpSS, Col);
   }

return (TRUE);
}


BOOL SS_SetRowCellNote(LPSPREADSHEET lpSS, SS_COORD Row, LPTSTR Note)
{
LPSS_ROW       lpRow;
LPSS_CELL      lpCell;
LPTSTR         lpszNote;
SS_COORD       i;

for (i = 0; i < lpSS->Col.AllocCnt; i++)
   if (lpCell = SS_LockCellItem(lpSS, i, Row))
      {
      if (lpCell->hCellNote)
         {
         tbGlobalFree(lpCell->hCellNote);
         lpCell->hCellType = 0;
         }
      SS_UnlockCellItem(lpSS, Row, i);
      }

if (lpRow = SS_AllocLockRow(lpSS, Row))
   {
   if (lpRow->hCellNote)
   {
     tbGlobalFree(lpRow->hCellNote);
     lpRow->hCellNote = 0;
   }
   if (Note && Note[0])
      {
      if (!(lpRow->hCellNote = tbGlobalAlloc(GMEM_MOVEABLE | GMEM_ZEROINIT,
                                             (long)(lstrlen(Note)+1)*sizeof(TCHAR))))
         {
         SS_UnlockRowItem(lpSS, Row);
         return (FALSE);
         }
      lpszNote = (LPTSTR)tbGlobalLock(lpRow->hCellNote);
      _tcscpy(lpszNote, Note);
      tbGlobalUnlock(lpRow->hCellNote);
      }

   SS_UnlockRowItem(lpSS, Row);
   SS_InvalidateRow(lpSS, Row);
   }

return (TRUE);
}

BOOL SS_SetCellNoteRange(lpSS, Col, Row, Col2, Row2, Note)
LPSPREADSHEET lpSS;
SS_COORD      Col;
SS_COORD      Row;
SS_COORD      Col2;
SS_COORD      Row2;
LPTSTR        Note;
{
SS_COORD      x;
SS_COORD      y;
BOOL          RedrawOld;
BOOL          Ret = FALSE;

SS_LimitRangeCoords(lpSS, &Col, &Row, &Col2, &Row2);

if (Row != Row2 || Col != Col2)
   {
   RedrawOld = lpSS->lpBook->Redraw;
   lpSS->lpBook->Redraw = FALSE;
   }

for (y = Row; y <= Row2; y++)
   for (x = Col; x <= Col2; x++)
      {
      if (x != SS_ALLCOLS && y != SS_ALLROWS)
         Ret = SS_SetCellCellNote(lpSS, x, y, Note);
      else if (x != SS_ALLCOLS && y == SS_ALLROWS)
         Ret = SS_SetColCellNote(lpSS, x, Note);
      else if (x == SS_ALLCOLS && y != SS_ALLROWS)
         Ret = SS_SetRowCellNote(lpSS, y, Note);
      else
         SS_SetDocCellNote(lpSS, Note);

      if (!Ret)
         break;
      }

if (Row != Row2 || Col != Col2)
	SS_BookSetRedraw(lpSS->lpBook, RedrawOld);

return (Ret);
}


short DLLENTRY SSGetCellNote(HWND hWnd, SS_COORD CellCol, SS_COORD CellRow, 
                             LPTSTR Note)
{
LPSPREADSHEET lpSS = SS_SheetLock(hWnd);
BOOL          fRet;

SS_AdjustCellCoords(lpSS, &CellCol, &CellRow);

fRet = SS_GetCellNote(lpSS, CellCol, CellRow, Note);
SS_SheetUnlock(hWnd);
return (fRet);
}

BOOL DLLENTRY SSSetCellNote(HWND hWnd, SS_COORD Col, SS_COORD Row, LPTSTR Note)
{
LPSPREADSHEET lpSS = SS_SheetLock(hWnd);
BOOL          fRet = TRUE;

SS_AdjustCellCoords(lpSS, &Col, &Row);

fRet = SS_SetCellNoteRange(lpSS, Col, Row, Col, Row, Note);
SS_SheetUnlock(hWnd);
return (fRet);
}

BOOL DLLENTRY SSSetCellNoteRange(HWND hWnd, SS_COORD Col, SS_COORD Row, SS_COORD Col2, SS_COORD Row2, LPTSTR Note)
{
LPSPREADSHEET  lpSS = SS_SheetLock(hWnd);
BOOL           fRet = TRUE;
LPTSTR         lpszNullNote = NULL;

if (!Note)
  Note = lpszNullNote;

SS_AdjustCellRangeCoords(lpSS, &Col, &Row, &Col2, &Row2);
fRet = SS_SetCellNoteRange(lpSS, Col, Row, Col2, Row2, Note);
SS_SheetUnlock(hWnd);

return (fRet);
}


WORD DLLENTRY SSGetCellNoteIndicator(HWND hWnd)
{
LPSS_BOOK lpBook = SS_BookLock(hWnd);

short nCellNoteIndicator = SS_GetCellNoteIndicator(lpBook);
SS_BookUnlock(hWnd);

return nCellNoteIndicator;
}

void DLLENTRY SSSetCellNoteIndicator(HWND hWnd, WORD wCellNoteIndicator)
{
LPSS_BOOK lpBook = SS_BookLock(hWnd);

SS_SetCellNoteIndicator(lpBook, wCellNoteIndicator);
SS_BookUnlock(hWnd);
}

BOOL DLLENTRY SSIsFetchCellNote(HWND hWnd)
{
LPSS_BOOK lpBook = SS_BookLock(hWnd);
BOOL      fRet = TRUE;

fRet = SS_IsFetchCellNote(lpBook);
SS_BookUnlock(hWnd);

return (fRet);
}

void DLLENTRY SS_SetCalTextOverride(LPSS_BOOK lpBook, LPCTSTR lpszShortDays, LPCTSTR lpszLongDays,
                                    LPCTSTR lpszShortMonths, LPCTSTR lpszLongMonths,
                                    LPCTSTR lpszOkText, LPCTSTR lpszCancelText)
{
LPTSTR  lpszTemp;

// Set short day names
if (lpszShortDays)
   {
   if (lpBook->hDayShortNames)
      {
      tbGlobalFree(lpBook->hDayShortNames);
      lpBook->hDayShortNames = 0;
      }

   if (*lpszShortDays)
      {
      lpBook->hDayShortNames = tbGlobalAlloc(GHND, (_tcslen(lpszShortDays) + 1) * sizeof(TCHAR));
      lpszTemp = (LPTSTR)tbGlobalLock(lpBook->hDayShortNames);
      _tcscpy(lpszTemp, lpszShortDays);
      tbGlobalUnlock(lpBook->hDayShortNames);  
      }
   }

// Set long day names
if (lpszLongDays)
   {
   if (lpBook->hDayLongNames)
      {
      tbGlobalFree(lpBook->hDayLongNames);
      lpBook->hDayLongNames = 0;
      }

   if (*lpszLongDays)
      {
      lpBook->hDayLongNames = tbGlobalAlloc(GHND, (_tcslen(lpszLongDays) + 1) * sizeof(TCHAR));
      lpszTemp = (LPTSTR)tbGlobalLock(lpBook->hDayLongNames);
      _tcscpy(lpszTemp, lpszLongDays);
      tbGlobalUnlock(lpBook->hDayLongNames);  
      }
   }

// Set short month names
if (lpszShortMonths)
   {
   if (lpBook->hMonthShortNames)
      {
      tbGlobalFree(lpBook->hMonthShortNames);
      lpBook->hMonthShortNames = 0;
      }

   if (*lpszShortMonths)
      {
      lpBook->hMonthShortNames = tbGlobalAlloc(GHND, (_tcslen(lpszShortMonths) + 1) * sizeof(TCHAR));
      lpszTemp = (LPTSTR)tbGlobalLock(lpBook->hMonthShortNames);
      _tcscpy(lpszTemp, lpszShortMonths);
      tbGlobalUnlock(lpBook->hMonthShortNames);  
      }
   }

// Set long month names
if (lpszLongMonths)
   {
   if (lpBook->hMonthLongNames)
      {
      tbGlobalFree(lpBook->hMonthLongNames);
      lpBook->hMonthLongNames = 0;
      }

   if (*lpszLongMonths)
      {
      lpBook->hMonthLongNames = tbGlobalAlloc(GHND, (_tcslen(lpszLongMonths) + 1) * sizeof(TCHAR));
      lpszTemp = (LPTSTR)tbGlobalLock(lpBook->hMonthLongNames);
      _tcscpy(lpszTemp, lpszLongMonths);
      tbGlobalUnlock(lpBook->hMonthLongNames);  
      }
   }

if (lpszOkText)
   {
   if (lpBook->hOkText)
      {
      tbGlobalFree(lpBook->hOkText);
      lpBook->hOkText = 0;
      }

   if (*lpszOkText)
      {
      lpBook->hOkText = tbGlobalAlloc(GHND, (_tcslen(lpszOkText) + 1) * sizeof(TCHAR));
      lpszTemp = (LPTSTR)tbGlobalLock(lpBook->hOkText);
      _tcscpy(lpszTemp, lpszOkText);
      tbGlobalUnlock(lpBook->hOkText);  
      }
   }

if (lpszCancelText)
   {
   if (lpBook->hCancelText)
      {
      tbGlobalFree(lpBook->hCancelText);
      lpBook->hCancelText = 0;
      }

   if (*lpszCancelText)
      {
      lpBook->hCancelText = tbGlobalAlloc(GHND, (_tcslen(lpszCancelText) + 1) * sizeof(TCHAR));
      lpszTemp = (LPTSTR)tbGlobalLock(lpBook->hCancelText);
      _tcscpy(lpszTemp, lpszCancelText);
      tbGlobalUnlock(lpBook->hCancelText);  
      }
   }

}

void DLLENTRY SSSetCalTextOverride(HWND hWnd, LPCTSTR lpszShortDays, LPCTSTR lpszLongDays,
                                   LPCTSTR lpszShortMonths, LPCTSTR lpszLongMonths,
                                   LPCTSTR lpszOkText, LPCTSTR lpszCancelText)
{
LPSS_BOOK lpBook = SS_BookLock(hWnd);

SS_SetCalTextOverride(lpBook, lpszShortDays, lpszLongDays,
                      lpszShortMonths, lpszLongMonths,
                      lpszOkText, lpszCancelText);
SS_BookUnlock(hWnd);
}

void DLLENTRY SSGetCalTextOverride(HWND hWnd, LPTSTR lpszShortDays, LPSHORT pnLenShortDays, 
                          LPTSTR lpszLongDays, LPSHORT pnLenLongDays, 
                          LPTSTR lpszShortMonths, LPSHORT pnLenShortMonths, 
                          LPTSTR lpszLongMonths, LPSHORT pnLenLongMonths, 
                          LPTSTR lpszOkText, LPSHORT pnLenOkText, 
                          LPTSTR lpszCancelText, LPSHORT pnLenCancelText)
{
LPSS_BOOK lpBook = SS_BookLock(hWnd);
LPTSTR    lpszTemp;

if (lpBook->hDayShortNames)
   {
   lpszTemp = (LPTSTR)tbGlobalLock(lpBook->hDayShortNames);
   if (lpszShortDays)
      _tcscpy(lpszShortDays, lpszTemp);

   if (NULL != pnLenShortDays)
      *pnLenShortDays = lstrlen(lpszTemp);

   tbGlobalUnlock(lpBook->hDayShortNames);  
   }
else
   {
   if (NULL != pnLenShortDays)
      *pnLenShortDays = 0;
   }

if (lpBook->hDayLongNames)
   {
   lpszTemp = (LPTSTR)tbGlobalLock(lpBook->hDayLongNames);
   if (lpszLongDays)
      _tcscpy(lpszLongDays, lpszTemp);

   if (NULL != pnLenLongDays)
      *pnLenLongDays = lstrlen(lpszTemp);

   tbGlobalUnlock(lpBook->hDayLongNames);  
   }
else
   {
   if (NULL != pnLenLongDays)
      *pnLenLongDays = 0;
   }


if (lpBook->hMonthShortNames)
   {
   lpszTemp = (LPTSTR)tbGlobalLock(lpBook->hMonthShortNames);
   if (lpszShortMonths)
      _tcscpy(lpszShortMonths, lpszTemp);

   if (NULL != pnLenShortMonths)
      *pnLenShortMonths = lstrlen(lpszTemp);

   tbGlobalUnlock(lpBook->hMonthShortNames);  
   }
else
   {
   if (NULL != pnLenShortMonths)
      *pnLenShortMonths = 0;
   }

if (lpBook->hMonthLongNames)
   {
   lpszTemp = (LPTSTR)tbGlobalLock(lpBook->hMonthLongNames);
   if (lpszLongMonths)
      _tcscpy(lpszLongMonths, lpszTemp);

   if (NULL != pnLenLongMonths)
      *pnLenLongMonths = lstrlen(lpszTemp);

   tbGlobalUnlock(lpBook->hMonthLongNames);  
   }
else
   {
   if (NULL != pnLenLongMonths)
      *pnLenLongMonths = 0;
   }

if (lpBook->hOkText)
   {
   lpszTemp = (LPTSTR)tbGlobalLock(lpBook->hOkText);
   if (lpszOkText)
      _tcscpy(lpszOkText, lpszTemp);

   if (NULL != pnLenOkText)
      *pnLenOkText = lstrlen(lpszTemp);

   tbGlobalUnlock(lpBook->hOkText);  
   }
else
   {
   if (NULL != pnLenOkText)
      *pnLenOkText = 0;
   }

if (lpBook->hCancelText)
   {
   lpszTemp = (LPTSTR)tbGlobalLock(lpBook->hCancelText);
   if (lpszCancelText)
      _tcscpy(lpszCancelText, lpszTemp);

   if (NULL != pnLenCancelText)
      *pnLenCancelText = lstrlen(lpszTemp);

   tbGlobalUnlock(lpBook->hCancelText);  
   }
else
   {
   if (NULL != pnLenCancelText)
      *pnLenCancelText = 0;
   }

SS_BookUnlock(hWnd);
}


#endif // SS_V35

WORD DLLENTRY SSSetClipboardOptions(HWND hWnd, WORD wOptions)
{
LPSS_BOOK lpBook = SS_BookLock(hWnd);
WORD      wOptionsOld = (WORD)lpBook->bClipboardOptions;

if (wOptions >= 0 && wOptions <= 15)
   lpBook->bClipboardOptions = (BYTE)wOptions;

SS_BookUnlock(hWnd);

return (wOptionsOld);
}


WORD DLLENTRY SSGetClipboardOptions(HWND hWnd)
{
LPSS_BOOK lpBook = SS_BookLock(hWnd);
WORD      wOptions = (WORD)lpBook->bClipboardOptions;

SS_BookUnlock(hWnd);
return (wOptions);
}

#if defined(_WIN64) || defined(_IA64)
void DLLENTRY SSSetPrintOptions(HWND hWnd, LPSS_PRINTFORMAT lpPrintFormat,
                                SS_PRINTFUNC lpfnPrintProc, LONG_PTR lAppData)
#else
void DLLENTRY SSSetPrintOptions(HWND hWnd, LPSS_PRINTFORMAT lpPrintFormat,
                                SS_PRINTFUNC lpfnPrintProc, LONG lAppData)
#endif
{
LPSPREADSHEET lpSS = SS_SheetLock(hWnd);
SS_PAGEFORMAT PageFormat;

SS_PrintFormat2PageFormat(&PageFormat, lpPrintFormat);

SS_SetPrintOptions(lpSS, &PageFormat, lpfnPrintProc, lAppData);
SS_SheetUnlock(hWnd);
}

#if defined(_WIN64) || defined(_IA64)
void DLLENTRY SSGetPrintOptionsEx(HWND hWnd, LPSS_PAGEFORMAT lpPageFormat,
                                  SS_PRINTFUNC *lpfnPrintProc, LONG_PTR *lplAppData)
#else
void DLLENTRY SSGetPrintOptionsEx(HWND hWnd, LPSS_PAGEFORMAT lpPageFormat,
                                  SS_PRINTFUNC *lpfnPrintProc, LPLONG lplAppData)
#endif
{
LPSPREADSHEET lpSS = SS_SheetLock(hWnd);

if (lpPageFormat)
	*lpPageFormat = lpSS->PrintOptions.pf;

if (lpfnPrintProc)
	*lpfnPrintProc = lpSS->PrintOptions.lpfnPrintProc;

if (lplAppData)
	*lplAppData = lpSS->PrintOptions.lAppData;

SS_SheetUnlock(hWnd);
}

#if defined(_WIN64) || defined(_IA64)
void DLLENTRY SSSetPrintOptionsEx(HWND hWnd, LPSS_PAGEFORMAT lpPageFormat,
                                  SS_PRINTFUNC lpfnPrintProc, LONG_PTR lAppData)
#else
void DLLENTRY SSSetPrintOptionsEx(HWND hWnd, LPSS_PAGEFORMAT lpPageFormat,
                                  SS_PRINTFUNC lpfnPrintProc, LONG lAppData)
#endif
{
LPSPREADSHEET lpSS = SS_SheetLock(hWnd);
SS_SetPrintOptions(lpSS, lpPageFormat, lpfnPrintProc, lAppData);
SS_SheetUnlock(hWnd);
}

#if defined(_WIN64) || defined(_IA64)
void SS_SetPrintOptions(LPSPREADSHEET lpSS, LPSS_PAGEFORMAT lpPageFormat,
                        SS_PRINTFUNC lpfnPrintProc, LONG_PTR lAppData)
#else
void SS_SetPrintOptions(LPSPREADSHEET lpSS, LPSS_PAGEFORMAT lpPageFormat,
                        SS_PRINTFUNC lpfnPrintProc, LONG lAppData)
#endif
{
if (lpPageFormat)
   _fmemcpy(&lpSS->PrintOptions.pf, lpPageFormat, sizeof(SS_PAGEFORMAT));
else
   {
	if (lpSS->PrintOptions.pf.hHeader)
		GlobalFree(lpSS->PrintOptions.pf.hHeader);

	if (lpSS->PrintOptions.pf.hFooter)
		GlobalFree(lpSS->PrintOptions.pf.hFooter);

   _fmemset(&lpSS->PrintOptions.pf, '\0', sizeof(SS_PAGEFORMAT));
   lpSS->PrintOptions.pf.fDrawBorder = TRUE;
   lpSS->PrintOptions.pf.fDrawShadows = TRUE;
   lpSS->PrintOptions.pf.fDrawColors = FALSE;
   lpSS->PrintOptions.pf.fShowGrid = TRUE;
   lpSS->PrintOptions.pf.fShowColHeaders = TRUE;
   lpSS->PrintOptions.pf.fShowRowHeaders = TRUE;
   lpSS->PrintOptions.pf.fUseDataMax = TRUE;
   lpSS->PrintOptions.pf.dPrintType = 0;
   lpSS->PrintOptions.pf.x1CellRange = 0;
   lpSS->PrintOptions.pf.y1CellRange = 0;
   lpSS->PrintOptions.pf.x2CellRange = 0;
   lpSS->PrintOptions.pf.y2CellRange = 0;
   lpSS->PrintOptions.pf.nPageStart = 1;
   lpSS->PrintOptions.pf.nPageEnd = 1;
   lpSS->PrintOptions.pf.fMarginLeft = 0;
   lpSS->PrintOptions.pf.fMarginTop = 0;
   lpSS->PrintOptions.pf.fMarginRight = 0;
   lpSS->PrintOptions.pf.fMarginBottom = 0;
   lpSS->PrintOptions.pf.hDCPrinter = 0;
   lpSS->PrintOptions.pf.wOrientation = 0;
   lpSS->PrintOptions.pf.fSmartPrint = FALSE;
	lpSS->PrintOptions.pf.lFirstPageNumber = 1;
#ifdef SS_V70
   lpSS->PrintOptions.pf.ZoomFactor = 1.0;
   lpSS->PrintOptions.pf.wScalingMethod = SS_PRINTSCALINGMETHOD_NONE;
   lpSS->PrintOptions.pf.nBestFitPagesWide = 1;
   lpSS->PrintOptions.pf.nBestFitPagesTall = 1;
   lpSS->PrintOptions.pf.fCenterOnPageH = FALSE;
   lpSS->PrintOptions.pf.fCenterOnPageV = FALSE;
#endif // SS_V70
   }

lpSS->PrintOptions.lpfnPrintProc = lpfnPrintProc;
lpSS->PrintOptions.lAppData = lAppData;
}


void SS_PageFormat2PrintFormat(LPSS_PAGEFORMAT lpPageFormat, LPSS_PRINTFORMAT lpPrintFormat)
{
_fmemset(lpPrintFormat, '\0', sizeof(SS_PRINTFORMAT));

lpPrintFormat->fDrawBorder = lpPageFormat->fDrawBorder;
lpPrintFormat->fDrawShadows = lpPageFormat->fDrawShadows;
lpPrintFormat->fDrawColors = lpPageFormat->fDrawColors;
lpPrintFormat->fShowGrid = lpPageFormat->fShowGrid;
lpPrintFormat->fShowColHeaders = lpPageFormat->fShowColHeaders;
lpPrintFormat->fShowRowHeaders = lpPageFormat->fShowRowHeaders;
lpPrintFormat->fUseDataMax = lpPageFormat->fUseDataMax;
lpPrintFormat->dPrintType = lpPageFormat->dPrintType;
lpPrintFormat->x1CellRange = lpPageFormat->x1CellRange;
lpPrintFormat->y1CellRange = lpPageFormat->y1CellRange;
lpPrintFormat->x2CellRange = lpPageFormat->x2CellRange;
lpPrintFormat->y2CellRange = lpPageFormat->y2CellRange;
lpPrintFormat->nPageStart = lpPageFormat->nPageStart;
lpPrintFormat->nPageEnd = lpPageFormat->nPageEnd;
lpPrintFormat->fMarginLeft = lpPageFormat->fMarginLeft;
lpPrintFormat->fMarginTop = lpPageFormat->fMarginTop;
lpPrintFormat->fMarginRight = lpPageFormat->fMarginRight;
lpPrintFormat->fMarginBottom = lpPageFormat->fMarginBottom;
lpPrintFormat->hDCPrinter = lpPageFormat->hDCPrinter;
lpPrintFormat->wOrientation = lpPageFormat->wOrientation;
lpPrintFormat->fSmartPrint = lpPageFormat->fSmartPrint;
#ifdef SS_V70
lpPrintFormat->wScalingMethod = lpPageFormat->wScalingMethod;
lpPrintFormat->ZoomFactor = lpPageFormat->ZoomFactor;
lpPrintFormat->nBestFitPagesWide = lpPageFormat->nBestFitPagesWide;
lpPrintFormat->nBestFitPagesTall = lpPageFormat->nBestFitPagesTall;
lpPrintFormat->fCenterOnPageH = lpPageFormat->fCenterOnPageH;
lpPrintFormat->fCenterOnPageV = lpPageFormat->fCenterOnPageV;
#endif // SS_V70
}


void SS_PrintFormat2PageFormat(LPSS_PAGEFORMAT lpPageFormat, LPSS_PRINTFORMAT lpPrintFormat)
{
_fmemset(lpPageFormat, '\0', sizeof(SS_PAGEFORMAT));

lpPageFormat->fDrawBorder = lpPrintFormat->fDrawBorder;
lpPageFormat->fDrawShadows = lpPrintFormat->fDrawShadows;
lpPageFormat->fDrawColors = lpPrintFormat->fDrawColors;
lpPageFormat->fShowGrid = lpPrintFormat->fShowGrid;
lpPageFormat->fShowColHeaders = lpPrintFormat->fShowColHeaders;
lpPageFormat->fShowRowHeaders = lpPrintFormat->fShowRowHeaders;
lpPageFormat->fUseDataMax = lpPrintFormat->fUseDataMax;
lpPageFormat->dPrintType = lpPrintFormat->dPrintType;
lpPageFormat->x1CellRange = lpPrintFormat->x1CellRange;
lpPageFormat->y1CellRange = lpPrintFormat->y1CellRange;
lpPageFormat->x2CellRange = lpPrintFormat->x2CellRange;
lpPageFormat->y2CellRange = lpPrintFormat->y2CellRange;
lpPageFormat->nPageStart = lpPrintFormat->nPageStart;
lpPageFormat->nPageEnd = lpPrintFormat->nPageEnd;
lpPageFormat->fMarginLeft = lpPrintFormat->fMarginLeft;
lpPageFormat->fMarginTop = lpPrintFormat->fMarginTop;
lpPageFormat->fMarginRight = lpPrintFormat->fMarginRight;
lpPageFormat->fMarginBottom = lpPrintFormat->fMarginBottom;
lpPageFormat->hDCPrinter = lpPrintFormat->hDCPrinter;
lpPageFormat->wOrientation = lpPrintFormat->wOrientation;
lpPageFormat->fSmartPrint = lpPrintFormat->fSmartPrint;
// RFW - 1/31/04 - 15641
lpPageFormat->lFirstPageNumber = 1;

#ifdef SS_V70
lpPageFormat->wScalingMethod = lpPrintFormat->wScalingMethod;
lpPageFormat->ZoomFactor = lpPrintFormat->ZoomFactor;
lpPageFormat->nBestFitPagesWide = lpPrintFormat->nBestFitPagesWide;
lpPageFormat->nBestFitPagesTall = lpPrintFormat->nBestFitPagesTall;
lpPageFormat->fCenterOnPageH = lpPrintFormat->fCenterOnPageH;
lpPageFormat->fCenterOnPageV = lpPrintFormat->fCenterOnPageV;
#endif // SS_V70
}


#ifdef SS_DLL

#if defined(_WIN64) || defined(_IA64)
BOOL DLLENTRY SSGetPrintOptions(HWND hWnd, LPSS_PRINTFORMAT lpPrintFormat,
                                SS_PRINTFUNC FAR *lpfnPrintProc,
                                LONG_PTR *lplAppData)
#else
BOOL DLLENTRY SSGetPrintOptions(HWND hWnd, LPSS_PRINTFORMAT lpPrintFormat,
                                SS_PRINTFUNC FAR *lpfnPrintProc,
                                LPLONG lplAppData)
#endif
{
LPSPREADSHEET lpSS = SS_SheetLock(hWnd);
BOOL          fRet = TRUE;

if (lpPrintFormat)
	SS_PageFormat2PrintFormat(&lpSS->PrintOptions.pf, lpPrintFormat);
if (lpfnPrintProc)
   *lpfnPrintProc = lpSS->PrintOptions.lpfnPrintProc;
if (lplAppData)
   *lplAppData = lpSS->PrintOptions.lAppData;

SS_SheetUnlock(hWnd);
return (fRet);
}


long DLLENTRY SSGetNextPageBreakCol(HWND hWnd, long lPrevCol)
{
LPSPREADSHEET    lpSS = SS_SheetLock(hWnd);
SS_PRINTPAGECALC PrintPageCalc;
long             lRet;
SS_PAGEFORMAT    PageFormat;

PageFormat = lpSS->PrintOptions.pf;
PageFormat.dPrintType |= SS_PRINTTYPE_CELLRANGE;
PageFormat.x1CellRange = lPrevCol;
PageFormat.x2CellRange = -1;
PageFormat.y1CellRange = -1;
PageFormat.y2CellRange = -1;

PrintPageCalc.lPageCnt = -1;
SS_Print(lpSS, NULL, &PageFormat, lpSS->PrintOptions.lpfnPrintProc,
         lpSS->PrintOptions.lAppData, 0, SS_PRINT_ACTION_NEXTCOL, 0, NULL,
         &PrintPageCalc, NULL, NULL, NULL, NULL);
lRet = PrintPageCalc.lPageCnt;

SS_SheetUnlock(hWnd);
return (lRet);
}


long DLLENTRY SSGetNextPageBreakRow(HWND hWnd, long lPrevRow)
{
LPSPREADSHEET    lpSS = SS_SheetLock(hWnd);
SS_PRINTPAGECALC PrintPageCalc;
long             lRet;
SS_PAGEFORMAT    PageFormat;

PageFormat = lpSS->PrintOptions.pf;
PageFormat.dPrintType |= SS_PRINTTYPE_CELLRANGE;
PageFormat.y1CellRange = lPrevRow;
PageFormat.y2CellRange = -1;
PageFormat.x1CellRange = -1;
PageFormat.x2CellRange = -1;

PrintPageCalc.lPageCnt = -1;
SS_Print(lpSS, NULL, &PageFormat, lpSS->PrintOptions.lpfnPrintProc,
         lpSS->PrintOptions.lAppData, 0, SS_PRINT_ACTION_NEXTROW, 0, NULL,
         &PrintPageCalc, NULL, NULL, NULL, NULL);
lRet = PrintPageCalc.lPageCnt;

SS_SheetUnlock(hWnd);
return (lRet);
}


long DLLENTRY SSGetPrintPageCount(HWND hWnd)
{
LPSPREADSHEET lpSS = SS_SheetLock(hWnd);
PRVW_INFO     PrvwInfo;
long          lRet = 0;

SS_PrintPreviewGetInfo(lpSS, &PrvwInfo);
lRet = PrvwInfo.lPageEnd;

SS_SheetUnlock(hWnd);
return (lRet);
}

#endif // SS_DLL


void DLLENTRY SSGetSelColor(HWND hWnd, LPCOLORREF lpclrBack, LPCOLORREF lpclrFore)
{
LPSS_BOOK lpBook;

lpBook = SS_BookLock(hWnd);
SS_GetSelColor(lpBook, lpclrBack, lpclrFore);
SS_BookUnlock(hWnd);
}


void SS_GetSelColor(LPSS_BOOK lpBook, LPCOLORREF lpclrBack, LPCOLORREF lpclrFore)
{
SS_COLORTBLITEM ColorTblItem;

if (lpclrBack)
   {
   SS_GetColorItem(&ColorTblItem, lpBook->SelColor.BackgroundId);
   *lpclrBack = ColorTblItem.Color;
   }

if (lpclrFore)
   {
   SS_GetColorItem(&ColorTblItem, lpBook->SelColor.ForegroundId);
   *lpclrFore = ColorTblItem.Color;
   }
}


void DLLENTRY SSSetSelColor(HWND hWnd, COLORREF clrBack, COLORREF clrFore)
{
LPSS_BOOK lpBook;

lpBook = SS_BookLock(hWnd);
SS_SetSelColor(lpBook, clrBack, clrFore);
SS_BookUnlock(hWnd);
}


void SS_SetSelColor(LPSS_BOOK lpBook, COLORREF clrBack, COLORREF clrFore)
{
lpBook->SelColor.BackgroundId = SS_AddColor(clrBack);
lpBook->SelColor.ForegroundId = SS_AddColor(clrFore);

SS_InvalidateRect(lpBook, NULL, TRUE);
}


#ifdef SS_DLL

BOOL DLLENTRY SSSetArray(HWND hWnd, LPVOID lpArray, SS_COORD ColLeft,
                         SS_COORD RowTop, SS_COORD ArrayColCnt,
                         SS_COORD ArrayRowCnt, WORD wDataType)
{
LPSPREADSHEET lpSS;
BOOL          fRet;

lpSS = SS_SheetLock(hWnd);
fRet = SS_SetArray(lpSS, lpArray, ColLeft, RowTop, ArrayColCnt, ArrayRowCnt,
                   wDataType);
SS_SheetUnlock(hWnd);
return (fRet);
}


BOOL SS_SetArray(LPSPREADSHEET lpSS, LPVOID lpArray, SS_COORD ColLeft,
                 SS_COORD RowTop, SS_COORD ArrayColCnt,
                 SS_COORD ArrayRowCnt, WORD wDataType)
{
HPSTR    hpszArray = (HPSTR)lpArray;
SS_COORD Col;
SS_COORD Row;
long     lSize;
long     lValue;
double   dfValue;
#if defined(_WIN64) || defined(_IA64)
LONG_PTR lpValue;
#endif
BOOL     fRet = TRUE;

switch (wDataType)
   {
   case SS_ARRAYTYPE_INT:
      lSize = sizeof(int);
      for (Row = 0; Row < ArrayRowCnt; Row++)
         for (Col = 0; Col < ArrayColCnt; Col++)
            {
            lValue = (long)*(LPINT)(hpszArray + (((Row * ArrayColCnt) + Col) *
                                                 lSize));
            SS_SetDataRange(lpSS, ColLeft + Col, RowTop + Row, ColLeft + Col,
                            RowTop + Row, (LPTSTR)&lValue,
                            SS_VALUE_INT, FALSE, TRUE);
            }

      break;

   case SS_ARRAYTYPE_SHORT:
      lSize = sizeof(short);
      for (Row = 0; Row < ArrayRowCnt; Row++)
         for (Col = 0; Col < ArrayColCnt; Col++)
            {
            lValue = (long)*(LPSHORT)(hpszArray + (((Row * ArrayColCnt) + Col) *
                                                   lSize));
            SS_SetDataRange(lpSS, ColLeft + Col, RowTop + Row, ColLeft + Col,
                            RowTop + Row, (LPTSTR)&lValue,
                            SS_VALUE_INT, FALSE, TRUE);
            }

      break;

   case SS_ARRAYTYPE_LONG:
      lSize = sizeof(long);
      for (Row = 0; Row < ArrayRowCnt; Row++)
         for (Col = 0; Col < ArrayColCnt; Col++)
            {
            lValue = (long)*(LPLONG)(hpszArray + (((Row * ArrayColCnt) + Col) *
                                                  lSize));
            SS_SetDataRange(lpSS, ColLeft + Col, RowTop + Row, ColLeft + Col,
                            RowTop + Row, (LPTSTR)&lValue,
                            SS_VALUE_INT, FALSE, TRUE);
            }

      break;

   case SS_ARRAYTYPE_FLOAT:
      lSize = sizeof(float);
      for (Row = 0; Row < ArrayRowCnt; Row++)
         for (Col = 0; Col < ArrayColCnt; Col++)
            {
            dfValue = (double)*(LPFLOAT)(hpszArray + (((Row * ArrayColCnt) + Col) *
                                                      lSize));
            SS_SetDataRange(lpSS, ColLeft + Col, RowTop + Row, ColLeft + Col,
                            RowTop + Row, (LPTSTR)&dfValue,
                            SS_VALUE_FLOAT, FALSE, TRUE);
            }

      break;

   case SS_ARRAYTYPE_DOUBLE:
      lSize = sizeof(double);
      for (Row = 0; Row < ArrayRowCnt; Row++)
         for (Col = 0; Col < ArrayColCnt; Col++)
            {
            dfValue = *(LPDOUBLE)(hpszArray + (((Row * ArrayColCnt) + Col) *
                                               lSize));
            SS_SetDataRange(lpSS, ColLeft + Col, RowTop + Row, ColLeft + Col,
                            RowTop + Row, (LPTSTR)&dfValue,
                            SS_VALUE_FLOAT, FALSE, TRUE);
            }

      break;

#if defined(_WIN64) || defined(_IA64)
   case SS_ARRAYTYPE_LONG_PTR:
	    lSize = sizeof(LONG_PTR);
      for (Row = 0; Row < ArrayRowCnt; Row++)
         for (Col = 0; Col < ArrayColCnt; Col++)
            {
            lpValue = *(PLONG_PTR)(hpszArray + (((Row * ArrayColCnt) + Col) *
                                               lSize));
            SS_SetDataRange(lpSS, ColLeft + Col, RowTop + Row, ColLeft + Col,
                            RowTop + Row, (LPTSTR)&lpValue,
                            SS_VALUE_LONG_PTR, FALSE, TRUE);
            }
#endif
/*
   case SS_ARRAYTYPE_TEXT:
   case SS_ARRAYTYPE_VALUE:
      {
      BOOL fValue = (wDataType == SS_ARRAYTYPE_VALUE);
      lSize = sizeof(LPSTR);
      for (Row = 0; Row < ArrayRowCnt; Row++)
         for (Col = 0; Col < ArrayColCnt; Col++)
            {
            LPSTR lpszValue = *(LPSTR FAR *)(hpszArray + (((Row * ArrayColCnt) + Col) *
                                             lSize));
            SS_SetDataRange(lpSS, ColLeft + Col, RowTop + Row, ColLeft + Col,
                            RowTop + Row, lpszValue, fValue, FALSE, TRUE);
            }
      }
      break;
*/
   }

return (fRet);
}


BOOL DLLENTRY SSGetArray(HWND hWnd, LPVOID lpArray, SS_COORD ColLeft,
                         SS_COORD RowTop, SS_COORD ArrayColCnt,
                         SS_COORD ArrayRowCnt, WORD wDataType)
{
LPSPREADSHEET lpSS;
BOOL          fRet;

lpSS = SS_SheetLock(hWnd);
fRet = SS_GetArray(lpSS, lpArray, ColLeft, RowTop, ArrayColCnt, ArrayRowCnt,
                   wDataType);
SS_SheetUnlock(hWnd);
return (fRet);
}


BOOL SS_GetArray(LPSPREADSHEET lpSS, LPVOID lpArray, SS_COORD ColLeft,
                 SS_COORD RowTop, SS_COORD ArrayColCnt,
                 SS_COORD ArrayRowCnt, WORD wDataType)
{
HPSTR    hpszArray = (HPSTR)lpArray;
SS_COORD Col;
SS_COORD Row;
long     lSize;
long     lValue;
double   dfValue;
BOOL     fRet = TRUE;

switch (wDataType)
   {
   case SS_ARRAYTYPE_INT:
      lSize = sizeof(int);
      for (Row = 0; Row < ArrayRowCnt; Row++)
         for (Col = 0; Col < ArrayColCnt; Col++)
            {
            if (!SS_GetInteger(lpSS, ColLeft + Col, RowTop + Row, &lValue))
               lValue = 0;

            *(LPINT)(hpszArray + (((Row * ArrayColCnt) + Col) *
                                  lSize)) = (int)lValue;
            }

      break;

   case SS_ARRAYTYPE_SHORT:
      lSize = sizeof(short);
      for (Row = 0; Row < ArrayRowCnt; Row++)
         for (Col = 0; Col < ArrayColCnt; Col++)
            {
            if (!SS_GetInteger(lpSS, ColLeft + Col, RowTop + Row, &lValue))
               lValue = 0;

            *(LPSHORT)(hpszArray + (((Row * ArrayColCnt) + Col) *
                                    lSize)) = (short)lValue;
            }

      break;

   case SS_ARRAYTYPE_LONG:
      lSize = sizeof(long);
      for (Row = 0; Row < ArrayRowCnt; Row++)
         for (Col = 0; Col < ArrayColCnt; Col++)
            {
            if (!SS_GetInteger(lpSS, ColLeft + Col, RowTop + Row, &lValue))
               lValue = 0;

            *(LPLONG)(hpszArray + (((Row * ArrayColCnt) + Col) *
                                   lSize)) = lValue;
            }

      break;

   case SS_ARRAYTYPE_FLOAT:
      lSize = sizeof(float);
      for (Row = 0; Row < ArrayRowCnt; Row++)
         for (Col = 0; Col < ArrayColCnt; Col++)
            {
            if (!SS_GetFloat(lpSS, ColLeft + Col, RowTop + Row, &dfValue))
               dfValue = 0.0;

            *(LPFLOAT)(hpszArray + (((Row * ArrayColCnt) + Col) *
                                    lSize)) = (float)dfValue;
            }

      break;

   case SS_ARRAYTYPE_DOUBLE:
      lSize = sizeof(double);
      for (Row = 0; Row < ArrayRowCnt; Row++)
         for (Col = 0; Col < ArrayColCnt; Col++)
            {
            if (!SS_GetFloat(lpSS, ColLeft + Col, RowTop + Row, &dfValue))
               dfValue = 0.0;

            *(LPDOUBLE)(hpszArray + (((Row * ArrayColCnt) + Col) *
                                     lSize)) = dfValue;
            }

      break;

/*
   case SS_ARRAYTYPE_TEXT:
   case SS_ARRAYTYPE_VALUE:
      fValue = (wDataType == SS_ARRAYTYPE_VALUE);
      lSize = sizeof(LPSTR);

      for (Row = 0; Row < ArrayRowCnt; Row++)
         for (Col = 0; Col < ArrayColCnt; Col++)
            {
            if (!SS_GetData(lpSS, Col, Row, &lValue))
               lValue = 0;

            *(LPLONG)(hpszArray + (((Row * ArrayColCnt) + Col) *
                                   lSize)) = lValue;
            }

      if (fValue)
         hData = SS_GetValue(lpSS, x, y);
      else
         hData = SS_GetData(lpSS, &CellType, x, y, FALSE);

      for (Row = 0; Row < ArrayRowCnt; Row++)
         for (Col = 0; Col < ArrayColCnt; Col++)
            {
            lpszValue = *(LPSTR FAR *)(hpszArray + (((Row * ArrayColCnt) + Col) *
                                                    lSize));
            SS_SetDataRange(lpSS, ColLeft + Col, RowTop + Row, ColLeft + Col,
                            RowTop + Row, lpszValue, fValue, FALSE, TRUE);
            }

      break;
*/
   }

return (fRet);
}

#endif // SS_DLL


short DLLENTRY SSGetTwoDigitYearMax(HWND hWnd)
{
LPSS_BOOK lpBook;
short     nTwoDigitYearMax;

lpBook = SS_BookLock(hWnd);
nTwoDigitYearMax = SS_GetTwoDigitYearMax(lpBook);
SS_BookUnlock(hWnd);

return (nTwoDigitYearMax);
}


short SS_GetTwoDigitYearMax(LPSS_BOOK lpBook)
{
return (lpBook->nTwoDigitYearMax);
}


BOOL DLLENTRY SSSetTwoDigitYearMax(HWND hWnd, short nTwoDigitYearMax)
{
LPSS_BOOK lpBook;
BOOL      fRet;

lpBook = SS_BookLock(hWnd);
fRet = SS_SetTwoDigitYearMax(lpBook, nTwoDigitYearMax);
SS_BookUnlock(hWnd);

return (fRet);
}


BOOL SS_SetTwoDigitYearMax(LPSS_BOOK lpBook, short nTwoDigitYearMax)
{
BOOL fRet = FALSE;

if (nTwoDigitYearMax >= 1100 && nTwoDigitYearMax <= 9999)
   {
   lpBook->nTwoDigitYearMax = nTwoDigitYearMax;
   fRet = TRUE;
   }

return (fRet);
}

#endif // SS_V30

#ifdef SS_V35
void SSx_GetCalText(HWND hWnd, LPCALTEXT lpCalText)
{
LPCTSTR lpPtr;
LPCTSTR lpPtrOld;
short   dCnt;

LPSS_BOOK lpBook = SS_BookLock(hWnd);

if (!lpBook)
	return;

// Get short day names
if (lpBook->hDayShortNames)
   {
   LPTSTR lpszShortDays = (LPTSTR)tbGlobalLock(lpBook->hDayShortNames);

   _fmemset(lpCalText->DayShortNames, '\0', sizeof(lpCalText->DayShortNames));

   for (dCnt = 0, lpPtr = lpszShortDays, lpPtrOld = lpPtr; dCnt < 7 &&
        (lpPtr = _ftcschr(lpPtr, '\t')); dCnt++, lpPtr++, lpPtrOld = lpPtr)
      {
      _fmemcpy(lpCalText->DayShortNames[dCnt], lpPtrOld,
               min((short)(lpPtr - lpPtrOld), DATENAMESZ) * sizeof(TCHAR));
      lpCalText->DayShortNames[dCnt][min((short)(lpPtr - lpPtrOld), DATENAMESZ)] = '\0';
      }

   if (lpPtrOld && dCnt < 7)
      {
      _fmemcpy(lpCalText->DayShortNames[dCnt], lpPtrOld,
               min(lstrlen(lpPtrOld), DATENAMESZ) * sizeof(TCHAR));
      lpCalText->DayShortNames[dCnt][min(lstrlen(lpPtrOld), DATENAMESZ)] = '\0';
      }

   tbGlobalUnlock(lpBook->hDayShortNames);
   }

// Get long day names
if (lpBook->hDayLongNames)
   {
   LPTSTR lpszLongDays = (LPTSTR)tbGlobalLock(lpBook->hDayLongNames);

   _fmemset(lpCalText->DayLongNames, '\0', sizeof(lpCalText->DayLongNames));

   for (dCnt = 0, lpPtr = lpszLongDays, lpPtrOld = lpPtr; dCnt < 7 &&
        (lpPtr = _ftcschr(lpPtr, '\t')); dCnt++, lpPtr++, lpPtrOld = lpPtr)
      {
      _fmemcpy(lpCalText->DayLongNames[dCnt], lpPtrOld,
               min((short)(lpPtr - lpPtrOld), DATENAMESZ) * sizeof(TCHAR));
      lpCalText->DayLongNames[dCnt][min((short)(lpPtr - lpPtrOld), DATENAMESZ)] = '\0';
      }

   if (lpPtrOld && dCnt < 7)
      {
      _fmemcpy(lpCalText->DayLongNames[dCnt], lpPtrOld,
               min(lstrlen(lpPtrOld), DATENAMESZ) * sizeof(TCHAR));
      lpCalText->DayLongNames[dCnt][min(lstrlen(lpPtrOld), DATENAMESZ)] = '\0';
      }
   tbGlobalUnlock(lpBook->hDayLongNames);
   }

if (lpBook->hMonthShortNames)
   {
   LPTSTR lpszShortMonths = (LPTSTR)tbGlobalLock(lpBook->hMonthShortNames);

   _fmemset(lpCalText->MonthShortNames, '\0', sizeof(lpCalText->MonthShortNames));

   for (dCnt = 0, lpPtr = lpszShortMonths, lpPtrOld = lpPtr; dCnt < 12 &&
        (lpPtr = _ftcschr(lpPtr, '\t')); dCnt++, lpPtr++, lpPtrOld = lpPtr)
      {                                        
      _fmemcpy(lpCalText->MonthShortNames[dCnt], lpPtrOld,
               min((short)(lpPtr - lpPtrOld), DATENAMESZ) * sizeof(TCHAR));
      lpCalText->MonthShortNames[dCnt][min((short)(lpPtr - lpPtrOld), DATENAMESZ)] = '\0';
      }

   if (lpPtrOld && dCnt < 12)
      {
      _fmemcpy(lpCalText->MonthShortNames[dCnt], lpPtrOld,
               min(lstrlen(lpPtrOld), DATENAMESZ) * sizeof(TCHAR));
      lpCalText->MonthShortNames[dCnt][min(lstrlen(lpPtrOld), DATENAMESZ)] = '\0';
      }
   tbGlobalUnlock(lpBook->hMonthShortNames);
   }

if (lpBook->hMonthLongNames)
   {
   LPTSTR lpszLongMonths = (LPTSTR)tbGlobalLock(lpBook->hMonthLongNames);

   _fmemset(lpCalText->MonthLongNames, '\0', sizeof(lpCalText->MonthLongNames));

   for (dCnt = 0, lpPtr = lpszLongMonths, lpPtrOld = lpPtr; dCnt < 12 &&
        (lpPtr = _ftcschr(lpPtr, '\t')); dCnt++, lpPtr++, lpPtrOld = lpPtr)
      {
      _fmemcpy(lpCalText->MonthLongNames[dCnt], lpPtrOld,
               min((short)(lpPtr - lpPtrOld), 14) * sizeof(TCHAR));
      lpCalText->MonthLongNames[dCnt][min((short)(lpPtr - lpPtrOld), 14)] = '\0';
      }

   if (lpPtrOld && dCnt < 12)
      {
      _fmemcpy(lpCalText->MonthLongNames[dCnt], lpPtrOld,
               min(lstrlen(lpPtrOld), 14) * sizeof(TCHAR));
      lpCalText->MonthLongNames[dCnt][min(lstrlen(lpPtrOld), 14)] = '\0';
      }
   tbGlobalUnlock(lpBook->hMonthLongNames);
   }

if (lpBook->hOkText)
   {
   LPTSTR lpszOkText = (LPTSTR)tbGlobalLock(lpBook->hOkText);
   lstrcpyn(lpCalText->OkText, lpszOkText, STRING_SIZE(lpCalText->OkText));
   tbGlobalUnlock(lpBook->hOkText);
   }

if (lpBook->hCancelText)
   {
   LPTSTR lpszCancelText = (LPTSTR)tbGlobalLock(lpBook->hCancelText);
   lstrcpyn(lpCalText->CancelText, lpszCancelText, STRING_SIZE(lpCalText->CancelText));
   tbGlobalUnlock(lpBook->hCancelText);
   }

SS_BookUnlock(hWnd);
}
#endif // SS_V35

#ifdef SS_V40


void DLLENTRY SSSetRowHeaderCols(HWND hWnd, SS_COORD lRowHeaderCols)
{
LPSPREADSHEET lpSS;

lpSS = SS_SheetLock(hWnd);
SS_SetRowHeaderCols(lpSS, lRowHeaderCols);
SS_SheetUnlock(hWnd);
}


void SS_SetRowHeaderCols(LPSPREADSHEET lpSS, SS_COORD lRowHeaderCols)
{
if (lRowHeaderCols >= 1 && lRowHeaderCols <= 255)
	{
	SSx_SetRowHeaderCols(lpSS, lRowHeaderCols);
	SS_AutoSize(lpSS->lpBook, FALSE);
	}
}


SS_COORD DLLENTRY SSGetRowHeaderCols(HWND hWnd)
{
LPSPREADSHEET lpSS;
SS_COORD lRet;

lpSS = SS_SheetLock(hWnd);
lRet = SS_GetRowHeaderCols(lpSS);
SS_SheetUnlock(hWnd);
return (lRet);
}


SS_COORD SS_GetRowHeaderCols(LPSPREADSHEET lpSS)
{
return (lpSS->Col.HeaderCnt);
}


void DLLENTRY SSSetRowHeadersAutoTextIndex(HWND hWnd, SS_COORD lRowHeadersAutoTextIndex)
{
LPSPREADSHEET lpSS;

lpSS = SS_SheetLock(hWnd);
SS_SetRowHeadersAutoTextIndex(lpSS, lRowHeadersAutoTextIndex);
SS_SheetUnlock(hWnd);
}


void SS_SetRowHeadersAutoTextIndex(LPSPREADSHEET lpSS, SS_COORD lRowHeadersAutoTextIndex)
{
if (lRowHeadersAutoTextIndex >= -1 && lRowHeadersAutoTextIndex < lpSS->Col.HeaderCnt)
	{
	lpSS->Col.lHeaderDisplayIndex = lRowHeadersAutoTextIndex;
	SS_InvalidateColRange(lpSS, 0, lpSS->Col.HeaderCnt - 1);
	}
}


SS_COORD DLLENTRY SSGetRowHeadersAutoTextIndex(HWND hWnd)
{
LPSPREADSHEET lpSS;
SS_COORD lRet;

lpSS = SS_SheetLock(hWnd);
lRet = SS_GetRowHeadersAutoTextIndex(lpSS);
SS_SheetUnlock(hWnd);
return (lRet);
}


SS_COORD SS_GetRowHeadersAutoTextIndex(LPSPREADSHEET lpSS)
{
return (lpSS->Col.lHeaderDisplayIndex);
}


void DLLENTRY SSSetColHeaderRows(HWND hWnd, SS_COORD lColHeaderRows)
{
LPSPREADSHEET lpSS;

lpSS = SS_SheetLock(hWnd);
SS_SetColHeaderRows(lpSS, lColHeaderRows);
SS_SheetUnlock(hWnd);
}


void SS_SetColHeaderRows(LPSPREADSHEET lpSS, SS_COORD lColHeaderRows)
{
if (lColHeaderRows >= 1 && lColHeaderRows <= 255)
	{
	SSx_SetColHeaderRows(lpSS, lColHeaderRows);
	SS_AutoSize(lpSS->lpBook, FALSE);
	}
}


SS_COORD DLLENTRY SSGetColHeaderRows(HWND hWnd)
{
LPSPREADSHEET lpSS;
SS_COORD lRet;

lpSS = SS_SheetLock(hWnd);
lRet = SS_GetColHeaderRows(lpSS);
SS_SheetUnlock(hWnd);
return (lRet);
}


SS_COORD SS_GetColHeaderRows(LPSPREADSHEET lpSS)
{
return (lpSS->Row.HeaderCnt);
}


void DLLENTRY SSSetColHeadersAutoTextIndex(HWND hWnd, SS_COORD lColHeadersAutoTextIndex)
{
LPSPREADSHEET lpSS;

lpSS = SS_SheetLock(hWnd);
SS_SetColHeadersAutoTextIndex(lpSS, lColHeadersAutoTextIndex);
SS_SheetUnlock(hWnd);
}


void SS_SetColHeadersAutoTextIndex(LPSPREADSHEET lpSS, SS_COORD lColHeadersAutoTextIndex)
{
if (lColHeadersAutoTextIndex >= -1 && lColHeadersAutoTextIndex < lpSS->Row.HeaderCnt)
	{
	lpSS->Row.lHeaderDisplayIndex = lColHeadersAutoTextIndex;
	SS_InvalidateRowRange(lpSS, 0, lpSS->Row.HeaderCnt - 1);
	}
}


SS_COORD DLLENTRY SSGetColHeadersAutoTextIndex(HWND hWnd)
{
LPSPREADSHEET lpSS;
SS_COORD lRet;

lpSS = SS_SheetLock(hWnd);
lRet = SS_GetColHeadersAutoTextIndex(lpSS);
SS_SheetUnlock(hWnd);
return (lRet);
}


SS_COORD SS_GetColHeadersAutoTextIndex(LPSPREADSHEET lpSS)
{
return (lpSS->Row.lHeaderDisplayIndex);
}


void DLLENTRY SSSetColHeadersUserSortIndex(HWND hWnd, SS_COORD lColHeadersUserSortIndex)
{
LPSPREADSHEET lpSS;

lpSS = SS_SheetLock(hWnd);
SS_SetColHeadersUserSortIndex(lpSS, lColHeadersUserSortIndex);
SS_SheetUnlock(hWnd);
}


void SS_SetColHeadersUserSortIndex(LPSPREADSHEET lpSS, SS_COORD lColHeadersUserSortIndex)
{
if (lColHeadersUserSortIndex >= -1 && lColHeadersUserSortIndex < lpSS->Row.HeaderCnt)
	{
	lpSS->lColHeadersUserSortIndex = lColHeadersUserSortIndex;
	SS_InvalidateRowRange(lpSS, 0, lpSS->Row.HeaderCnt - 1);
	}
}


SS_COORD DLLENTRY SSGetColHeadersUserSortIndex(HWND hWnd)
{
LPSPREADSHEET lpSS;
SS_COORD lRet;

lpSS = SS_SheetLock(hWnd);
lRet = SS_GetColHeadersUserSortIndex(lpSS);
SS_SheetUnlock(hWnd);
return (lRet);
}


SS_COORD SS_GetColHeadersUserSortIndex(LPSPREADSHEET lpSS)
{
return (lpSS->lColHeadersUserSortIndex);
}


short DLLENTRY SSGetCellTag(HWND hWnd, SS_COORD Col, SS_COORD Row, LPTSTR CellTag)
{
LPSPREADSHEET lpSS;
short         nRet;

lpSS = SS_SheetLock(hWnd);
SS_AdjustCellCoords(lpSS, &Col, &Row);
nRet = SS_GetCellTag(lpSS, Col, Row, CellTag);
SS_SheetUnlock(hWnd);

return (nRet);
}


short SS_GetCellTag(LPSPREADSHEET lpSS, SS_COORD Col, SS_COORD Row, LPTSTR CellTag)
{
TBGLOBALHANDLE hCellTag = 0;
BOOL           nRet = 0;

if (Row != SS_ALLROWS && Col != SS_ALLCOLS)
	{
   LPSS_CELL lpCell = SS_LockCellItem(lpSS, Col, Row);
	if (lpCell)
		hCellTag = lpCell->hCellTag;
   SS_UnlockCellItem(lpSS, Col, Row);
	}

if (Row != SS_ALLROWS && !hCellTag)
	{
   LPSS_ROW lpRow = SS_LockRowItem(lpSS, Row);
	if (lpRow)
		hCellTag = lpRow->hCellTag;
   SS_UnlockRowItem(lpSS, Row);
	}

if (Col != SS_ALLCOLS && !hCellTag)
	{
   LPSS_COL lpCol = SS_LockColItem(lpSS, Col);
	if (lpCol)
		hCellTag = lpCol->hCellTag;
   SS_UnlockColItem(lpSS, Col);
	}

if (!hCellTag)
	hCellTag = lpSS->hCellTag;

if (hCellTag)
	{
	LPTSTR lpszCellTag = (LPTSTR)tbGlobalLock(hCellTag);
	nRet = lstrlen(lpszCellTag);
	if (nRet && CellTag)
		_tcscpy(CellTag, lpszCellTag);     

	tbGlobalUnlock(hCellTag);  
	}

return nRet;
}


BOOL DLLENTRY SSSetCellTag(HWND hWnd, SS_COORD Col, SS_COORD Row, LPCTSTR CellTag)
{
LPSPREADSHEET lpSS;
BOOL          bRet;

lpSS = SS_SheetLock(hWnd);
SS_AdjustCellCoords(lpSS, &Col, &Row);
bRet = SS_SetCellTagRange(lpSS, Col, Row, Col, Row, CellTag);
SS_SheetUnlock(hWnd);

return (bRet);
}


BOOL SS_SetCellTagRange(LPSPREADSHEET lpSS, SS_COORD Col, SS_COORD Row, SS_COORD Col2, SS_COORD Row2, LPCTSTR lpszCellTag)
{
TBGLOBALHANDLE hCellTag = 0;
LPTSTR         lpsz;
SS_COORD       x, y;
BOOL           fRet = FALSE;

SS_LimitRangeCoords(lpSS, &Col, &Row, &Col2, &Row2);

for (y = Row; y <= Row2; y++)
   for (x = Col; x <= Col2; x++)
      {
		if (lpszCellTag && *lpszCellTag)
			{
			hCellTag = tbGlobalAlloc(GHND, (lstrlen(lpszCellTag)+1)*sizeof(TCHAR));
			if (hCellTag)
				{
				lpsz = (LPTSTR)tbGlobalLock(hCellTag);
				_tcscpy(lpsz, lpszCellTag);
				tbGlobalUnlock(hCellTag);
				}
			}

		if (y != SS_ALLROWS && x != SS_ALLCOLS)
			{
			LPSS_CELL lpCell = SS_AllocLockCell(lpSS, NULL, x, y);
			if (lpCell)
				{
				if (lpCell->hCellTag)
					tbGlobalFree(lpCell->hCellTag);
				lpCell->hCellTag = hCellTag;
				fRet = TRUE;
				}
			SS_UnlockCellItem(lpSS, x, y);
			}
		else if (y != SS_ALLROWS && x == SS_ALLCOLS)
			{
			LPSS_ROW lpRow = SS_AllocLockRow(lpSS, y);
			if (lpRow)
				{
				if (lpRow->hCellTag)
					tbGlobalFree(lpRow->hCellTag);
				lpRow->hCellTag = hCellTag;
				fRet = TRUE;
				}
			SS_UnlockRowItem(lpSS, y);
			}
		else if (x != SS_ALLROWS && y == SS_ALLCOLS)
			{
			LPSS_COL lpCol = SS_AllocLockCol(lpSS, x);
			if (lpCol)
				{
				if (lpCol->hCellTag)
					tbGlobalFree(lpCol->hCellTag);
				lpCol->hCellTag = hCellTag;
				fRet = TRUE;
				}
			SS_UnlockColItem(lpSS, x);
			}
		else
			{
			if (lpSS->hCellTag)
				tbGlobalFree(lpSS->hCellTag);
			lpSS->hCellTag = hCellTag;
			fRet = TRUE;
			}

		if (!fRet && hCellTag)
			tbGlobalFree(hCellTag);
		}

return (fRet);
}

#endif // SS_V40


BOOL DLLENTRY SSSetColBoundInfo(HWND hWnd, SS_COORD Col, LPCTSTR lpFieldName, BYTE bDataFillEvent)
{
LPSPREADSHEET lpSS;
BOOL          bRet;

lpSS = SS_SheetLock(hWnd);
SS_AdjustCellCoords(lpSS, &Col, NULL);
bRet = SS_SetColFieldName(lpSS, Col, lpFieldName);
SS_SetColDataFillEvent(lpSS, Col, bDataFillEvent);
SS_SheetUnlock(hWnd);

return (bRet);
}


long DLLENTRY SSGetColBoundInfo(HWND hWnd, SS_COORD Col, LPTSTR lpFieldName, LPBYTE lpbDataFillEvent)
{
LPSPREADSHEET  lpSS;
TBGLOBALHANDLE hFieldName;
long           lRet = 0;

lpSS = SS_SheetLock(hWnd);
SS_AdjustCellCoords(lpSS, &Col, NULL);
hFieldName = SS_GetColFieldName(lpSS, Col);

if (hFieldName)
	{
	LPTSTR lpszFieldName = (LPTSTR)tbGlobalLock(hFieldName);

	if (lpFieldName)
//		lstrcpy(lpFieldName, lpszFieldName);
		_tcscpy(lpFieldName, lpszFieldName);

	lRet = lstrlen(lpszFieldName);

	tbGlobalUnlock(hFieldName);
	}

if (lpbDataFillEvent)
	*lpbDataFillEvent = SS_GetColDataFillEvent(lpSS, Col);

SS_SheetUnlock(hWnd);

return (lRet);
}


void DLLENTRY SSClearColBoundInfo(HWND hWnd)
{
LPSPREADSHEET lpSS = SS_SheetLock(hWnd);
SS_COORD      Col;
SS_COORD      ColCnt = SS_GetColCnt(lpSS);

for (Col = 0; Col < ColCnt; Col++)
	{
	SS_SetColFieldName(lpSS, Col, NULL);
	SS_SetColDataFillEvent(lpSS, Col, 0);
	}

SS_SheetUnlock(hWnd);
}

#ifdef SS_V70

void DLLENTRY SSGetScrollBarSize(HWND hWnd, LPSHORT lpdVScrollWidth, LPSHORT lpdHScrollHeight)
{
LPSS_BOOK lpBook = SS_BookLock(hWnd);
SS_GetScrollBarSize(lpBook, lpdVScrollWidth, lpdHScrollHeight);
SS_BookUnlock(hWnd);
}


void SS_GetScrollBarSize(LPSS_BOOK lpBook, LPSHORT lpdVScrollWidth, LPSHORT lpdHScrollHeight)
{
if (lpdVScrollWidth)
	*lpdVScrollWidth = lpBook->dVScrollWidthProp;

if (lpdHScrollHeight)
	*lpdHScrollHeight = lpBook->dHScrollHeightProp;
}


void DLLENTRY SSSetScrollBarSize(HWND hWnd, short dVScrollWidth, short dHScrollHeight)
{
LPSS_BOOK lpBook = SS_BookLock(hWnd);
SS_SetScrollBarSize(lpBook, dVScrollWidth, dHScrollHeight);
SS_BookUnlock(hWnd);
}


void SS_SetScrollBarSize(LPSS_BOOK lpBook, short dVScrollWidth, short dHScrollHeight)
{
if (dVScrollWidth > 0 || dVScrollWidth == -1)
	{
	lpBook->dVScrollWidthProp = dVScrollWidth;

	if (dVScrollWidth > 0)
		lpBook->dVScrollWidth = dVScrollWidth;
	else if (dVScrollWidth == -1)
		lpBook->dVScrollWidth = GetSystemMetrics(SM_CXVSCROLL);
	}

if (dHScrollHeight > 0 || dHScrollHeight == -1)
	{
	SS_InvalidateTabStrip(lpBook);
	lpBook->dHScrollHeightProp = dHScrollHeight;

	if (dHScrollHeight > 0)
		lpBook->dHScrollHeight = dHScrollHeight;
	else if (dHScrollHeight == -1)
		lpBook->dHScrollHeight = GetSystemMetrics(SM_CYHSCROLL);

	SS_EmptyTabStripRect(lpBook);
	SS_InvalidateTabStrip(lpBook);
	}

SS_InvalidateRect(lpBook, NULL, FALSE);
SS_AutoSize(lpBook, FALSE);
}


WORD DLLENTRY SSGetActiveCellHighlightStyle(HWND hWnd)
{
LPSPREADSHEET lpSS = SS_SheetLock(hWnd);
WORD          wRet;

if (lpSS)
	wRet = SS_GetActiveCellHighlightStyle(lpSS);

SS_SheetUnlock(hWnd);
return (wRet);
}


WORD SS_GetActiveCellHighlightStyle(LPSPREADSHEET lpSS)
{
return (lpSS->bActiveCellHighlightStyle);
}


void DLLENTRY SSSetActiveCellHighlightStyle(HWND hWnd, WORD wActiveCellHighlightStyle)
{
LPSPREADSHEET lpSS = SS_SheetLock(hWnd);
SS_SetActiveCellHighlightStyle(lpSS, wActiveCellHighlightStyle);
SS_SheetUnlock(hWnd);
}


void SS_SetActiveCellHighlightStyle(LPSPREADSHEET lpSS, WORD wActiveCellHighlightStyle)
{
if (wActiveCellHighlightStyle >= 0 && wActiveCellHighlightStyle <= 1)
	{
	SS_HighlightCell(lpSS, FALSE);
	lpSS->bActiveCellHighlightStyle = (BYTE)wActiveCellHighlightStyle;
	SS_HighlightCell(lpSS, TRUE);
	}
}


SS_COORD DLLENTRY SSColLetterToNumber(LPCTSTR lpszText)
{
return (SS_ColLetterToNumber(lpszText));
}


void DLLENTRY SSGetScrollBarColor(HWND hWnd, LPCOLORREF lpVScrollBarColor, LPCOLORREF lpHScrollBarColor)
{
LPSS_BOOK lpBook = SS_BookLock(hWnd);
SS_GetScrollBarColor(lpBook, lpVScrollBarColor, lpHScrollBarColor);
SS_BookUnlock(hWnd);
}


void SS_GetScrollBarColor(LPSS_BOOK lpBook, LPCOLORREF lpVScrollBarColor, LPCOLORREF lpHScrollBarColor)
{
SS_COLORTBLITEM ColorTblItem;

if (lpVScrollBarColor)
   {
   SS_GetColorItem(&ColorTblItem, lpBook->VScrollBarColorId);
   *lpVScrollBarColor = ColorTblItem.Color;
   }

if (lpHScrollBarColor)
   {
   SS_GetColorItem(&ColorTblItem, lpBook->HScrollBarColorId);
   *lpHScrollBarColor = ColorTblItem.Color;
   }
}


void DLLENTRY SSSetScrollBarColor(HWND hWnd, COLORREF VScrollBarColor, COLORREF HScrollBarColor)
{
LPSS_BOOK lpBook = SS_BookLock(hWnd);
SS_SetScrollBarColor(lpBook, VScrollBarColor, HScrollBarColor);
SS_BookUnlock(hWnd);
}


void SS_SetScrollBarColor(LPSS_BOOK lpBook, COLORREF VScrollBarColor, COLORREF HScrollBarColor)
{
lpBook->VScrollBarColorId = SS_AddColor(VScrollBarColor);
lpBook->HScrollBarColorId  = SS_AddColor(HScrollBarColor);

InvalidateRect(lpBook->hWndVScroll, NULL, TRUE);
InvalidateRect(lpBook->hWndHScroll, NULL, TRUE);
}


short DLLENTRY SSGetSheet(HWND hWnd)
{
LPSS_BOOK lpBook = SS_BookLock(hWnd);
short     nSheet;

nSheet = SS_GetSheet(lpBook);
SS_BookUnlock(hWnd);
return (nSheet);
}


short SS_GetSheet(LPSS_BOOK lpBook)
{
return (lpBook->nSheet == -1 ? -1 : lpBook->nSheet + 1);
}


void DLLENTRY SSSetSheet(HWND hWnd, short nSheet)
{
LPSS_BOOK lpBook = SS_BookLock(hWnd);
SS_SetSheet(lpBook, nSheet);
SS_BookUnlock(hWnd);
}


void SS_SetSheet(LPSS_BOOK lpBook, short nSheet)
{
if (nSheet > 0 && nSheet <= lpBook->nSheetCnt)
	lpBook->nSheet = nSheet == -1 ? -1 : nSheet - 1;
}


short DLLENTRY SSGetSheetSendingMsg(HWND hWnd)
{
LPSS_BOOK lpBook = SS_BookLock(hWnd);
short     nSheet;

nSheet = SS_GetSheetSendingMsg(lpBook);
SS_BookUnlock(hWnd);
return (nSheet);
}


short SS_GetSheetSendingMsg(LPSS_BOOK lpBook)
{
return (lpBook->nSheetSendingMsg == -1 ? -1 : lpBook->nSheetSendingMsg + 1);
}


BOOL DLLENTRY SSInsertSheets(HWND hWnd, short nSheetIndex, short nSheetCnt)
{
LPSS_BOOK lpBook = SS_BookLock(hWnd);
BOOL      fRet = SS_InsertSheets(lpBook, (short)(nSheetIndex == -1 ? -1 : nSheetIndex - 1), nSheetCnt);

SS_BookUnlock(hWnd);

return (fRet);
}


BOOL DLLENTRY SSDeleteSheets(HWND hWnd, short nSheetIndex, short nSheetCnt)
{
LPSS_BOOK lpBook = SS_BookLock(hWnd);
BOOL      fRet = SS_DeleteSheets(lpBook, (short)(nSheetIndex == -1 ? -1 : nSheetIndex - 1), nSheetCnt);

if (lpBook->nSheetCnt == 0)
	SS_SetSheetCount(lpBook, 1);

SS_BookUnlock(hWnd);

return (fRet);
}


BOOL DLLENTRY SSMoveSheets(HWND hWnd, short nSheetSrc, short nSheetCnt, short nSheetDest)
{
LPSS_BOOK lpBook = SS_BookLock(hWnd);
BOOL      fRet = SS_MoveSheets(lpBook, (short)(nSheetSrc == -1 ? -1 : nSheetSrc - 1), nSheetCnt, (short)(nSheetDest == -1 ? -1 : nSheetDest - 1));

SS_BookUnlock(hWnd);

return (fRet);
}

HANDLE DLLENTRY SSLoadPicture(LPCTSTR lpszFileName, short nPictType)
{
  HANDLE hRet = NULL;
  HINSTANCE hInstDll;
  FPLOADIMAGEPROC pFPLoadImage;
  char path[MAX_PATH+1] = {0};
#if _UNICODE
  WideCharToMultiByte(CP_ACP, 0, lpszFileName, lstrlen(lpszFileName), (char*)path, MAX_PATH, NULL, NULL);
#else
  strncpy(path, lpszFileName, __min(strlen(lpszFileName), MAX_PATH));
#endif
#ifdef _DEBUG
  hInstDll = LoadLibrary(_T("fpimaged.dll"));
#else
  hInstDll = LoadLibrary(_T("fpimage.dll"));
#endif
  pFPLoadImage = hInstDll ? (FPLOADIMAGEPROC)GetProcAddress(hInstDll, "FPLoadImage") : NULL;
  if( !pFPLoadImage )
  {
//    MessageBox(NULL, DLL_NOT_FOUND, _T("FarPoint Spread"), MB_OK);
    return NULL;
  }
  hRet = pFPLoadImage(path, nPictType);
  FreeLibrary(hInstDll);
  return hRet;
}

HANDLE DLLENTRY SSLoadPictureBuffer(HGLOBAL hGlobal, long lSize, short nPictType)
{
  HANDLE hRet;
#ifdef _DEBUG
  HINSTANCE hInstDll = LoadLibrary(_T("fpimaged.dll"));
#else
  HINSTANCE hInstDll = LoadLibrary(_T("fpimage.dll"));
#endif
  FPLOADIMAGEBUFFERPROC pFPLoadImageBuffer = hInstDll ? (FPLOADIMAGEBUFFERPROC)GetProcAddress(hInstDll, "FPLoadImageBuffer") : NULL;
  if( !pFPLoadImageBuffer )
  {
//    MessageBox(NULL, DLL_NOT_FOUND, _T("FarPoint Spread"), MB_OK);
    return NULL;
  }
  hRet = pFPLoadImageBuffer(hGlobal, lSize, nPictType);
  FreeLibrary(hInstDll);
  return hRet;
}

HANDLE DLLENTRY SSLoadResPicture(HINSTANCE hInstance, LPCTSTR lpszResName, LPCTSTR lpszResType, short nPictType)
{
  HANDLE hRet;
  HINSTANCE hInstDll;
  FPLOADRESIMAGEPROC pFPLoadResImage;
  char name[MAX_PATH+1] = {0};
  char type[MAX_PATH+1] = {0};
#if _UNICODE
  if( lpszResName && HIWORD(lpszResName) && *lpszResName )
    WideCharToMultiByte(CP_ACP, 0, lpszResName, lstrlen(lpszResName), (char*)name, MAX_PATH, NULL, NULL);
  if( lpszResType && HIWORD(lpszResType) && *lpszResType )
    WideCharToMultiByte(CP_ACP, 0, lpszResType, lstrlen(lpszResType), (char*)type, MAX_PATH, NULL, NULL);
#else
  if( lpszResName && HIWORD(lpszResName) && *lpszResName )
    strncpy(name, lpszResName, __min(strlen(lpszResName), MAX_PATH));
  if( lpszResType && HIWORD(lpszResType) && *lpszResType )
    strncpy(type, lpszResType, __min(strlen(lpszResType), MAX_PATH));
#endif
#ifdef _DEBUG
  hInstDll = LoadLibrary(_T("fpimaged.dll"));
#else
  hInstDll = LoadLibrary(_T("fpimage.dll"));
#endif
  pFPLoadResImage = hInstDll ? (FPLOADRESIMAGEPROC)GetProcAddress(hInstDll, "FPLoadResImage") : NULL;
  if( !pFPLoadResImage )
  {
//    MessageBox(NULL, DLL_NOT_FOUND, _T("FarPoint Spread"), MB_OK);
    return NULL;
  }
  hRet = (HANDLE)pFPLoadResImage(hInstance, !HIWORD(lpszResName) ? (LPSTR)lpszResName : name, !HIWORD(lpszResType) ? (LPSTR)lpszResType : type, nPictType);
  FreeLibrary(hInstDll);
  return hRet;
}

BOOL DLLENTRY SSSavePicture(HANDLE hPict, HPALETTE hPal, LPCTSTR lpszFileName, short nSaveType)
{
  BOOL bRet;
  HINSTANCE hInstDll;
  FPSAVEIMAGEPROC pFPSaveImage;
  char path[MAX_PATH+1] = {0};
#if _UNICODE
  WideCharToMultiByte(CP_ACP, 0, lpszFileName, lstrlen(lpszFileName), (char*)path, MAX_PATH, NULL, NULL);
#else
  strncpy(path, lpszFileName, __min(strlen(lpszFileName), MAX_PATH));
#endif
#ifdef _DEBUG
  hInstDll = LoadLibrary(_T("fpimaged.dll"));
#else
  hInstDll = LoadLibrary(_T("fpimage.dll"));
#endif
  pFPSaveImage = hInstDll ? (FPSAVEIMAGEPROC)GetProcAddress(hInstDll, "FPSaveImage") : NULL;
  if( !pFPSaveImage )
  {
//    MessageBox(NULL, DLL_NOT_FOUND, _T("FarPoint Spread"), MB_OK);
    return FALSE;
  }
  bRet = pFPSaveImage(hPict, hPal, path, nSaveType);
  FreeLibrary(hInstDll);
  return bRet;
}

BOOL DLLENTRY SSSavePictureBuffer(HANDLE hPict, HPALETTE hPal, short nSaveType, HGLOBAL *phGlobal, long *plSize)
{
  BOOL bRet;
#ifdef _DEBUG
  HINSTANCE hInstDll = LoadLibrary(_T("fpimaged.dll"));
#else
  HINSTANCE hInstDll = LoadLibrary(_T("fpimage.dll"));
#endif
  FPSAVEIMAGEBUFFERPROC pFPSaveImageBuffer = hInstDll ? (FPSAVEIMAGEBUFFERPROC)GetProcAddress(hInstDll, "FPSaveImageBuffer") : NULL;
  if( !pFPSaveImageBuffer )
  {
//    MessageBox(NULL, DLL_NOT_FOUND, _T("FarPoint Spread"), MB_OK);
    return FALSE;
  }
  bRet = pFPSaveImageBuffer(hPict, hPal, nSaveType, phGlobal, plSize);
  FreeLibrary(hInstDll);
  return bRet;
}


#endif // SS_V70


SS_COORD SS_GetNextNonEmptyRow(LPSPREADSHEET lpSS, SS_COORD Row)
{
   SS_COORD Start = Row + 1;
   SS_COORD End = SS_GetRowCnt(lpSS);
   SS_COORD NextRow;
   LPSS_ROW lpRow = NULL;

   for( NextRow = Start; NextRow < End && lpRow == SS_LockRowItem(lpSS, NextRow) && lpRow == NULL; NextRow++ );
   if( lpRow )
      SS_UnlockRowItem(lpSS, NextRow);
   return NextRow < End ? NextRow : -1;
}

SS_COORD SS_GetNextNonEmptyColumnInRow(LPSPREADSHEET lpSS, SS_COORD Row, SS_COORD Col)
{
   SS_COORD  Start = Col + 1;
   SS_COORD  End = SS_GetColCnt(lpSS);
   SS_COORD  NextCol = -1;
   LPSS_ROW  lpRow;
   LPSS_COL  lpCol = NULL;
   LPSS_CELL lpCell = NULL;

   if( Row == SS_ALLROWS )
   {
      for( NextCol = Start; NextCol < End && lpCol == SS_LockColItem(lpSS, NextCol) && lpCol == NULL; NextCol++ );
      if( lpCol )
         SS_UnlockColItem(lpSS, NextCol);
   }
   else if( lpRow = SS_LockRowItem(lpSS, Row) )
   {
      for( NextCol = Start; NextCol < End && lpCell == SSx_LockCellItem(lpSS, lpRow, NextCol, Row) && lpCell == NULL; NextCol++ );
      if( lpCell )
         SSx_UnlockCellItem(lpSS, lpRow, NextCol, Row);
   }
   return NextCol < End ? NextCol : -1;
}

SS_COORD DLLENTRY SSGetNextNonEmptyRow(HWND hWnd, SS_COORD Row)
{
   SS_COORD NextRow;
   LPSPREADSHEET lpSS = SS_SheetLock(hWnd);

   SS_AdjustCellCoords(lpSS, NULL, &Row);
   NextRow = SS_GetNextNonEmptyRow(lpSS, Row);
   SS_AdjustCellCoordsOut(lpSS, NULL, &NextRow);
   SS_SheetUnlock(hWnd);
   return NextRow;
}

SS_COORD DLLENTRY SSGetNextNonEmptyColumnInRow(HWND hWnd, SS_COORD Row, SS_COORD Col)
{
   SS_COORD NextCol;
   LPSPREADSHEET lpSS = SS_SheetLock(hWnd);

   SS_AdjustCellCoords(lpSS, &Col, &Row);
   NextCol = SS_GetNextNonEmptyColumnInRow(lpSS, Row, Col);
   SS_AdjustCellCoordsOut(lpSS, &NextCol, NULL);
   SS_SheetUnlock(hWnd);
   return NextCol;
}


void SSx_ClearCheckBoxPictures(LPSPREADSHEET lpSS, LPSS_COL lpCol, LPSS_ROW lpRow,
                               LPSS_CELL lpCell, SS_COORD Col, SS_COORD Row)
{
SS_CELLTYPE CellType;

if (SSx_RetrieveCellType(lpSS, &CellType, lpCol, lpRow, lpCell, Col, Row))
	{
	if (SS_TYPE_CHECKBOX == CellType.Type)
		{
		CellType.Spec.CheckBox.hPictUpName = (TBGLOBALHANDLE)0;
		CellType.Spec.CheckBox.bPictUpType = 0;
		CellType.Spec.CheckBox.hPictDownName = (TBGLOBALHANDLE)0;
		CellType.Spec.CheckBox.bPictDownType = 0;
		CellType.Spec.CheckBox.hPictFocusUpName = (TBGLOBALHANDLE)0;
		CellType.Spec.CheckBox.bPictFocusUpType = 0;
		CellType.Spec.CheckBox.hPictFocusDownName = (TBGLOBALHANDLE)0;
		CellType.Spec.CheckBox.bPictFocusDownType = 0;
		CellType.Spec.CheckBox.hPictGrayName = (TBGLOBALHANDLE)0;
		CellType.Spec.CheckBox.bPictGrayType = 0;
		CellType.Spec.CheckBox.hPictFocusGrayName = (TBGLOBALHANDLE)0;
		CellType.Spec.CheckBox.bPictFocusGrayType = 0;

#if SS_OCX
		CellType.Spec.CheckBox.lpPictUp = 0;
		CellType.Spec.CheckBox.lpPictDown = 0;
		CellType.Spec.CheckBox.lpPictFocusUp = 0;
		CellType.Spec.CheckBox.lpPictFocusDown = 0;
		CellType.Spec.CheckBox.lpPictGray = 0;
		CellType.Spec.CheckBox.lpPictFocusGray = 0;
#endif // SS_OCX

		SSx_SetTypeCheckBoxDefaults(lpSS->lpBook, &CellType);

		if (CellType.Spec.CheckBox.hText)
			{
			HGLOBAL hTextNew;
			LPTSTR  lpszText = (LPTSTR)GlobalLock(CellType.Spec.CheckBox.hText);

			hTextNew = SS_CreateTextHandle(lpszText, 0);
			GlobalUnlock(CellType.Spec.CheckBox.hText);
			CellType.Spec.CheckBox.hText = hTextNew;
			}

		SS_SetCellTypeRange(lpSS, Col, Row, Col, Row, &CellType);
		}
	}
}


void SS_ClearCheckBoxPictures(LPSS_BOOK lpBook)
{
LPSPREADSHEET lpSSTemp;
LPSS_ROW      lpRow;
LPSS_CELL     lpCell;
SS_COORD      i = -1;
SS_COORD      j = -1;
short         nSheet;
BOOL          RedrawOld;

RedrawOld = lpBook->Redraw;
lpBook->Redraw = FALSE;

for (nSheet = 0; nSheet < lpBook->nSheetCnt; nSheet++)
	{
	lpSSTemp = SS_BookLockSheetIndex(lpBook, nSheet);

	//Clear each column in sheet
	j = SS_GetNextNonEmptyColumnInRow(lpSSTemp, SS_ALLROWS, j);

	for( ; j != -1; j = SS_GetNextNonEmptyColumnInRow(lpSSTemp, SS_ALLROWS, j) )
		SSx_ClearCheckBoxPictures(lpSSTemp, NULL, NULL, NULL, j, SS_ALLROWS);

	j = -1;

	// Clear each row
	SSx_ClearCheckBoxPictures(lpSSTemp, NULL, NULL, NULL, -1, -1);

	// Clear each cell
   i = SS_GetNextNonEmptyRow(lpSSTemp, -1);
   for( ; i != -1; i = SS_GetNextNonEmptyRow(lpSSTemp, i) )
		{
		if (lpRow = SS_LockRowItem(lpSSTemp, i))
			{
			SSx_ClearCheckBoxPictures(lpSSTemp, NULL, lpRow, NULL, -1, i);

			j = SS_GetNextNonEmptyColumnInRow(lpSSTemp, i, j);

			for( ; j != -1; j = SS_GetNextNonEmptyColumnInRow(lpSSTemp, i, j) )
				if (lpCell = SS_LockCellItem(lpSSTemp, j, i))
					{
					SSx_ClearCheckBoxPictures(lpSSTemp, NULL, lpRow, lpCell, j, i);
					SS_UnlockCellItem(lpSSTemp, j, i);
					}

			j = -1;
		   SS_UnlockRowItem(lpSSTemp, i);
			}
		}
	}

SS_BookSetRedraw(lpBook, RedrawOld);
}


void DLLENTRY SSClearCheckBoxPictures(HWND hWnd)
{
LPSS_BOOK lpBook = SS_BookLock(hWnd);
SS_ClearCheckBoxPictures(lpBook);
SS_BookUnlock(hWnd);
}
