/*********************************************************
* SS_MAIN.C                     
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


//---------*---------*---------*---------*---------*---------*---------*-----
// 
// UPDATE LOG:
//
//    RWP01 <not done>  <SEL5612> Extended multiselect mode doesn't
//              support keyboard shortcuts
//  
//    RWP02 10.16.96  <JAP5266> Scroll bar area does not repaint
//              properly if ScrollBarExtMode = True
//
//    RWP03 10.18.96  <JIS3305> Can't turn off beep sound
//
//    RWP04 10.22.96  <SCS3711> Deleting a row from a bound and
//              autosized spread causes the spread to toggle
//              between showing the visible row count and one
//              less than the visible row count
//    RAP01 07.16.01  9106 - if rows>32K, scrolltips do not work
//---------*---------*---------*---------*---------*---------*---------*-----


#define OEMRESOURCE

#include <windows.h>
#include <windowsx.h>
#if !defined(WIN32) && defined(SS_OCX)
#include <compobj.h>
#include <dispatch.h>
#define __tagVARIANT
#endif
#include <tchar.h>
#include <stdio.h>
#include <stddef.h>
#include <stdlib.h>
#include <string.h>
#include <process.h>
#include <math.h>
#include <limits.h>
#include "..\..\..\..\fplibs\fptools\src\fptools.h"
#include "..\..\..\..\fplibs\fptools\src\fpncpnt.h"
#include "spread.h"
#include "fphdc.h"
#include "ss_alloc.h"
#include "ss_bool.h"
#include "ss_calc.h"
#include "ss_cell.h"
#include "ss_col.h"
#include "ss_curs.h"
#include "ss_dde.h"
#include "ss_doc.h"
#include "ss_drag.h"
#include "ss_draw.h"
#include "ss_emode.h"
#include "ss_font.h"
#include "ss_formu.h"
#include "ss_heap.h"
#include "ss_main.h"
#include "ss_multi.h"
#include "ss_ole.h"
#include "ss_row.h"
#include "ss_scbar.h"
#include "ss_scrol.h"
#include "ss_span.h"
#include "ss_type.h"
#include "ss_user.h"
#include "ss_win.h"
#ifdef SS_V70
#include "ss_book.h"
#include "ss_tab.h"
#ifdef SS_OCX
#include "ssocxdb.h"
#endif
#endif // SS_V70
#ifdef SS_V80
#include "uxtheme.h"
#include "ss_theme.h"
#include "cust_celltype.h"
#include "ss_gradient.h"
#endif
#if defined(SS_V30) && defined(WIN32)
#include "ss_excel.h"
#endif // defined(SS_V30) && defined(WIN32)

#include "..\classes\checkbox.h"
#include "..\edit\editfld.h"
#include "..\..\..\..\fplibs\fptools\src\texttip.h"

#ifndef SS_OLDCALC
#include "..\calc\cal_mem.h"
#endif

#ifdef SS_DDE
#include "..\calc\cal_dde.h"
#endif

#ifdef SS_VB
//#include <vbapi.h>
#include "..\..\vbsrc\vbspread.h"
#define VBAPI_H 1
#endif

#ifdef SS_USEAWARE
#include "utools.h"
#endif

#ifdef SS_V35
#include "ftdattim.h"
#endif

#define DEFAULT_CELLWIDTH         8
#define RESIZE_OFFSET             4
#define SS_BLOCKMODE_INITIALDELAY 60

extern HANDLE    hDynamicInst;
#ifndef SS_NO_USE_SH
extern OMEM_POOL tbOmemPoolCol;
extern OMEM_POOL tbOmemPoolRow;
extern OMEM_POOL tbOmemPoolCell;
#endif
extern HBRUSH    hBrushFocusRect;

extern SS_PAINTBITMAP PaintBitmap;

/**********************
* Function prototypes
**********************/

SS_FONTID        SS_InitDefaultFont(LPSS_BOOK lpBook);
LRESULT          SS_Proc(LPSS_BOOK lpBook, LPSPREADSHEET lpSS, HWND hWnd, UINT Msg, WPARAM wParam, LPARAM lParam);
static int       SS_IsMouseInHorizResize(LPSPREADSHEET lpSS, LPRECT Rect,
                                         LPRECT RectClient, int MouseX,
                                         int MouseY, LPSS_COORD Row);
static int       SS_IsMouseInVertResize(LPSPREADSHEET lpSS, LPRECT Rect,
                                        LPRECT RectClient, int MouseX,
                                        int MouseY, LPSS_COORD Col);
static BOOL      SS_IsMouseInButton(LPSPREADSHEET lpSS, LPRECT RectClient,
                                    int MouseX, int MouseY);
#if SS_V80
static BOOL      SS_IsMouseInReservedLocation(LPSPREADSHEET lpSS, LPRECT RectClient, 
                                            int MouseX, int MouseY, HCURSOR *lpCursor);
#endif
static BOOL      SS_IsMouseInGrayArea(LPSPREADSHEET lpSS,
                                      LPRECT RectClient, int MouseX,
                                      int MouseY);
static void      SS_Resize(HWND hWnd, LPSPREADSHEET lpSS, short Mode,
                           LPARAM Pos);
static BOOL      SS_ScrollWithMouse(HWND hWnd, LPSPREADSHEET lpSS,
                                    LPSS_CELLCOORD lpBlockCellLR,
                                    LPSS_CELLCOORD lpCurrentPos,
                                    LPSS_CELLCOORD lpNewPos, int MouseX,
                                    int MouseY);
static void      SS_SelectBlock(HWND hWnd, LPSPREADSHEET lpSS,
                                int MouseX, int MouseY);
void             SS_SelectRow(HWND hWnd, LPSPREADSHEET lpSS);
void             SSx_SelectRow(HWND hWnd, LPSPREADSHEET lpSS,
                               SS_COORD Row);
void             SS_ExtSelBegin(HWND hWnd, LPSPREADSHEET lpSS);
BOOL DLLENTRY    SmartHeapInit(void);
#ifdef SS_UTP
BOOL             SS_IsMouseInScrollArrow(HWND hWnd, LPSPREADSHEET lpSS,
                                         LPRECT lpRectClient, int MouseX,
                                         int MouseY,
                                         LPBYTE lpbModeScrollArrow,
                                         BYTE bArrowToCheck);
#endif
static long SS_ProcessLButton(LPSPREADSHEET lpSS, HWND hWnd, UINT Msg, WPARAM wParam,
                              LPARAM lParam);

//RWP04a
short       SS_GetAutoSizeVisRowsHeight(LPSPREADSHEET lpSS, SS_COORD RowCnt,
                                        LPRECT lpRect);
short   SS_GetAutoSizeVisColsWidth(LPSPREADSHEET lpSS, SS_COORD ColCnt, 
                     LPRECT lpRect);
//RWP04a

// CTF 
#ifdef SS_OCX
BOOL FP_API SSOCXSetControlSize(LPFPCONTROL lpObject, long cx, long cy);
#endif
// CTF
#ifdef SS_V35
static BOOL SS_IsMouseInCellNote(LPSPREADSHEET lpSS, SS_COORD Col, SS_COORD Row, int MouseX, int MouseY);
BOOL SS_TTScrollTip(LPSPREADSHEET lpSS, BOOL fHScroll, SS_COORD lNew);
#endif

#ifdef SS_V70
void SS_ColMoving(LPSPREADSHEET lpSS, int xMouse, int yMouse);
void SS_RowMoving(LPSPREADSHEET lpSS, int xMouse, int yMouse);
BOOL SS_IsMouseInColMove(LPSPREADSHEET lpSS, int MouseX, int MouseY);
BOOL SS_IsColMove(LPSPREADSHEET lpSS, SS_COORD Col, SS_COORD Row);
BOOL SS_IsRowMove(LPSPREADSHEET lpSS, SS_COORD Col, SS_COORD Row);
BOOL SS_IsMouseInRowMove(LPSPREADSHEET lpSS, int MouseX, int MouseY);
void SS_HoverBegin(LPSPREADSHEET lpSS, SS_COORD Col, SS_COORD Row);
void SS_HoverMove(LPSPREADSHEET lpSS);
void SS_HoverEnd(LPSPREADSHEET lpSS);
void SS_HoverArrowColSetPos(LPSPREADSHEET lpSS);
void SS_HoverArrowRowSetPos(LPSPREADSHEET lpSS);
#endif // SS_V70

//--------------------------------------------------------------------
//
//  The SS_InitSpreadPreHwnd() function handles any spreadsheet
//  initializaion which can be done before a HWND exists.
//

BOOL SS_InitSpreadPreHwnd(LPSS_BOOK lpBook, COLORREF crForegrnd,
                          COLORREF crBackgrnd)
{
lpBook->nSheet = 0;
#ifdef SS_V70
SS_SetActiveSheetIndex(lpBook, 0);
#endif
lpBook->DocumentProtected = 0;
lpBook->AllowUserFormulas = 0;
lpBook->wSelBlockOption = SS_SELBLOCK_COLS | SS_SELBLOCK_ROWS |
                        SS_SELBLOCK_BLOCKS | SS_SELBLOCK_ALL;
lpBook->fScrollBarExtMode = 0;
lpBook->EditModeOn = 0;

lpBook->fEditModePermanent = 0;
lpBook->fAutoSize = 0;
lpBook->fProcessTab = 0;
lpBook->wMessageBeingSent = 0;
lpBook->fKillFocusReceived = TRUE;
lpBook->fWindowPainted = 0;
lpBook->fNoProcessButtonDown = 0;
lpBook->fArrowsExitEditMode = 0;
lpBook->fMoveActiveOnFocus = 0;
lpBook->fFocusOn = 0;
#ifdef SS_OCX
lpBook->fRetainSelBlock = TRUE;
#else
lpBook->fRetainSelBlock = 0;
#endif // SS_OCX
lpBook->fEditModeReplace = 0;
lpBook->fNoBorder = 0;
lpBook->fNoBeep = 0;
lpBook->fInvalidated = 0;
lpBook->fSetFocusWhileMsgBeingSent = 0;
lpBook->fProcessingKillFocus = 0;
lpBook->fProcessingLButtonDown = 0;
lpBook->fAutoClipboard = 0;
lpBook->fProcessSetFocus = 0;
lpBook->fNoSetScrollBars = 0;
lpBook->fAllowMultipleSelBlocks = 0;
lpBook->fSetFocusRecieved = 0;
lpBook->fNoSetCapture = 0;
lpBook->fAllowCellOverflow = 0;
lpBook->fNoInvalidate = 0;
lpBook->fSelBlockInvisible = 0;
lpBook->fVScrollVisible = 0;
lpBook->fHScrollVisible = 0;
lpBook->fAllowDragDrop = 0;
lpBook->fDragDropBoxOn = 0;
lpBook->fUseSpecialVScroll = 0;
lpBook->fFocusHighlightOn = 0;
lpBook->fRButtonDown = 0;
lpBook->wButtonDrawMode = 0;
lpBook->wEnterAction = 0;
lpBook->wSpecialVScrollOptions = 0;
lpBook->wScrollDirection = 0;
lpBook->EditModeTurningOn = 0;
lpBook->EditModeTurningOff = 0;

lpBook->wMode = SS_MODE_NONE;

if (!(lpBook->hBuffer = tbGlobalAlloc(GMEM_MOVEABLE | GMEM_ZEROINIT, 330 * sizeof(TCHAR))))
   ;

#ifdef SS_V35
lpBook->fThumb = FALSE;
lpBook->nShowScrollTips = SS_SHOWSCROLLTIPS_OFF;
lpBook->nCellNoteIndicator = SS_CELLNOTEINDICATOR_SHOWANDFIREEVENT;
lpBook->fFetchCellNote = FALSE;
lpBook->bScriptEnhanced = FALSE;
#endif

lpBook->DocumentProtected = TRUE;
lpBook->dBorderSize = 1;

lpBook->CalcAuto = TRUE;
lpBook->VertScrollBar = TRUE;
lpBook->HorzScrollBar = TRUE;

lpBook->fScrollBarShowMax = TRUE;
lpBook->fScrollBarMaxAlign = TRUE;

lpBook->fMoveActiveOnFocus = TRUE;
lpBook->fAutoClipboard = TRUE;

lpBook->wEnterAction = SS_ENTERACTION_NONE;
lpBook->dUnitType = SS_UNITTYPE_DEFAULT;

lpBook->dXMargin = XMARGIN;
lpBook->dYMargin = YMARGIN;

FloatGetFormat(0, &lpBook->WinFloatFormat);

lpBook->GrayAreaBackground = RGBCOLOR_PALEGRAY;

// Default & header cell types ????

lpBook->CursorDefault.hCursor = SS_DEFCURSOR_DEFAULT;
lpBook->CursorColResize.hCursor = SS_DEFCURSOR_COLRESIZE;
lpBook->CursorRowResize.hCursor = SS_DEFCURSOR_ROWRESIZE;
lpBook->CursorButton.hCursor = SS_DEFCURSOR_BUTTON;
lpBook->CursorGrayArea.hCursor = SS_DEFCURSOR_GRAYAREA;
lpBook->CursorLockedCell.hCursor = SS_DEFCURSOR_LOCKEDCELL;
lpBook->CursorColHeader.hCursor = SS_DEFCURSOR_COLHEADER;
lpBook->CursorRowHeader.hCursor = SS_DEFCURSOR_ROWHEADER;
lpBook->CursorDragDropArea.hCursor = SS_DEFCURSOR_DRAGDROPAREA;
lpBook->CursorDragDrop.hCursor = SS_DEFCURSOR_DRAGDROP;

#ifndef SS_V21
lpBook->fIs95 = SS_IsWindowsV95();
#endif

#ifdef SS_V40
lpBook->fEditOverflow = FALSE;
#endif // SS_V40

#ifndef SS_NOCALC
SS_SetTypeEdit(&lpBook->FormulaCellType, ES_LEFT | ES_AUTOHSCROLL,
               SS_CALC_FORMULA_MAXLEN, SS_CHRSET_CHR, SS_CASE_NOCASE);
#endif

lpBook->bDesignTime = FALSE;

lpBook->ActionKeys[SS_KBA_CURRENT] = VK_F3;
lpBook->ActionKeys[SS_KBA_POPUP] = VK_F4;
lpBook->ActionKeys[SS_KBA_CLEAR] = VK_F2;

#ifdef SS_V30
lpBook->SelColor.BackgroundId = 0;
lpBook->SelColor.ForegroundId = 0;
#endif

lpBook->clrBackDefOrig = crBackgrnd;
lpBook->clrForeDefOrig = crForegrnd;

lpBook->nTwoDigitYearMax = 2059;
#if SS_V30
lpBook->bClipboardOptions = 15;    // copy/paste all headers
#endif
lpBook->fHScrollBarTrack = 0;
lpBook->fVScrollBarTrack = 0;

lpBook->DefaultFontId = SS_InitDefaultFont(lpBook); // TODO - This needs to be freed

#ifdef SS_V70
lpBook->nTabStripLeftSheet = 0;
lpBook->wTabStripPolicy = SS_TABSTRIPPOLICY_ASNEEDED;
lpBook->dfTabStripRatio = 0.6;
lpBook->TabStripFontId = 0;
lpBook->dVScrollWidthProp = -1;
lpBook->dHScrollHeightProp = -1;
lpBook->VScrollBarColorId = 0;
lpBook->HScrollBarColorId = 0;
#endif // SS_V70
  _fmemset(lpBook->m_szPrinterDevice, '\0', sizeof(lpBook->m_szPrinterDevice));
  _fmemset(lpBook->m_szPrinterDriver, '\0', sizeof(lpBook->m_szPrinterDriver));
  _fmemset(lpBook->m_szPrinterOutput, '\0', sizeof(lpBook->m_szPrinterOutput));
  _fmemset(&(lpBook->m_PrinterDevMode), '\0', sizeof(lpBook->m_PrinterDevMode));

#ifdef SS_V80
  lpBook->fThemesActive = SS_IsThemeActive();
lpBook->wCellNoteIndicatorShape = SS_CELLNOTEINDICATORSHAPE_SQUARE;
lpBook->wTabStripButtonPolicy = SS_TABSTRIPBUTTONPOLICY_ALWAYS;
lpBook->mouseCol = -1;
lpBook->mouseRow = -1;
lpBook->mouseTabBtn = -1;
lpBook->mouseTabSheet = -1;
lpBook->MouseOver = FALSE;
lpBook->CellNoteIndicatorColor = RGBCOLOR_RED;
lpBook->wAppearanceStyle = SS_APPEARANCESTYLE_ENHANCED;
lpBook->wUseVisualStyles = SS_USEVISUALSTYLES_INHERIT;
lpBook->wScrollBarStyle = SS_SCROLLBARSTYLE_INHERIT;
lpBook->bEnhanceStaticCells = FALSE;
lpBook->wTabEnhancedShape = SS_TABENHANCEDSHAPE_TRAPEZOID;
lpBook->sheetTabUpperNormalStartColor = SS_SHEETTAB_UPPER_STARTCOLOR;
lpBook->sheetTabUpperNormalEndColor = SS_SHEETTAB_UPPER_ENDCOLOR;
lpBook->sheetTabLowerNormalStartColor = SS_SHEETTAB_LOWER_STARTCOLOR;
lpBook->sheetTabLowerNormalEndColor = SS_SHEETTAB_LOWER_ENDCOLOR;
lpBook->sheetTabForeColor = SS_SHEETTAB_FORECOLOR;
lpBook->sheetTabActiveForeColor = SS_SHEETTAB_ACTIVEFORECOLOR;
lpBook->sheetTabUpperHoverStartColor = SS_SHEETTAB_UPPER_HOVERSTARTCOLOR;
lpBook->sheetTabUpperHoverEndColor =  SS_SHEETTAB_UPPER_HOVERENDCOLOR;
lpBook->sheetTabLowerHoverStartColor = SS_SHEETTAB_LOWER_HOVERSTARTCOLOR;
lpBook->sheetTabLowerHoverEndColor = SS_SHEETTAB_LOWER_HOVERENDCOLOR;
lpBook->sheetTabOuterBorderColor = SS_SHEETTAB_OUTERBORDERCOLOR;
lpBook->sheetTabInnerBorderColor = SS_SHEETTAB_INNERBORDERCOLOR;
lpBook->sheetScrollUpperNormalStartColor = SS_SHEETTAB_SCROLLBTN_UPPER_NORMALSTARTCOLOR;
lpBook->sheetScrollUpperNormalEndColor = SS_SHEETTAB_SCROLLBTN_UPPER_NORMALENDCOLOR;
lpBook->sheetScrollLowerNormalStartColor = SS_SHEETTAB_SCROLLBTN_LOWER_NORMALSTARTCOLOR;
lpBook->sheetScrollLowerNormalEndColor = SS_SHEETTAB_SCROLLBTN_LOWER_NORMALENDCOLOR;
lpBook->sheetScrollUpperHoverStartColor = SS_SHEETTAB_SCROLLBTN_UPPER_HOVERSTARTCOLOR;
lpBook->sheetScrollUpperHoverEndColor = SS_SHEETTAB_SCROLLBTN_UPPER_HOVERENDCOLOR;
lpBook->sheetScrollLowerHoverStartColor = SS_SHEETTAB_SCROLLBTN_LOWER_HOVERSTARTCOLOR;
lpBook->sheetScrollLowerHoverEndColor = SS_SHEETTAB_SCROLLBTN_LOWER_HOVERENDCOLOR;
lpBook->sheetScrollUpperPushedStartColor = SS_SHEETTAB_SCROLLBTN_UPPER_PUSHEDSTARTCOLOR;
lpBook->sheetScrollUpperPushedEndColor = SS_SHEETTAB_SCROLLBTN_UPPER_PUSHEDENDCOLOR;
lpBook->sheetScrollLowerPushedStartColor = SS_SHEETTAB_SCROLLBTN_LOWER_PUSHEDSTARTCOLOR;
lpBook->sheetScrollLowerPushedEndColor = SS_SHEETTAB_SCROLLBTN_LOWER_PUSHEDENDCOLOR;
lpBook->sheetScrollArrowColor = SS_SHEETTAB_SCROLLBTN_ARROWCOLOR;
lpBook->sheetGrayAreaStartColor = SS_SHEETTAB_GRAYAREA_STARTCOLOR;
lpBook->sheetGrayAreaEndColor = SS_SHEETTAB_GRAYAREA_ENDCOLOR;
lpBook->sheetSplitBoxStartColor = SS_SPLITBOX_STARTCOLOR;
lpBook->sheetSplitBoxEndColor = SS_SPLITBOX_ENDCOLOR;

lpBook->scrollTrackColor = SS_SCROLLBAR_TRACKCOLOR;
lpBook->scrollArrowColor = SS_SCROLLBAR_ARROWCOLOR;
lpBook->scrollUpperNormalStartColor = SS_SCROLLBAR_NORMAL_UPPER_STARTCOLOR;
lpBook->scrollUpperNormalEndColor = SS_SCROLLBAR_NORMAL_UPPER_ENDCOLOR;
lpBook->scrollLowerNormalStartColor = SS_SCROLLBAR_NORMAL_LOWER_STARTCOLOR;
lpBook->scrollLowerNormalEndColor = SS_SCROLLBAR_NORMAL_LOWER_ENDCOLOR;
lpBook->scrollUpperPushedStartColor = SS_SCROLLBAR_PUSHED_UPPER_STARTCOLOR;
lpBook->scrollUpperPushedEndColor = SS_SCROLLBAR_PUSHED_UPPER_ENDCOLOR;
lpBook->scrollLowerPushedStartColor = SS_SCROLLBAR_PUSHED_LOWER_STARTCOLOR;
lpBook->scrollLowerPushedEndColor = SS_SCROLLBAR_PUSHED_LOWER_ENDCOLOR;
lpBook->scrollUpperHoverStartColor = SS_SCROLLBAR_HOVER_UPPER_STARTCOLOR;
lpBook->scrollUpperHoverEndColor = SS_SCROLLBAR_HOVER_UPPER_ENDCOLOR;
lpBook->scrollLowerHoverStartColor = SS_SCROLLBAR_HOVER_LOWER_STARTCOLOR;
lpBook->scrollLowerHoverEndColor = SS_SCROLLBAR_HOVER_LOWER_ENDCOLOR;
lpBook->scrollHoverBorderColor = SS_SCROLLBAR_HOVER_BUTTON_BORDERCOLOR;
lpBook->scrollBorderColor = SS_SCROLLBAR_BUTTON_BORDERCOLOR;
lpBook->colSelectedHoverUpperColor = SS_COLHEADER_SELECTEDHOVER_UPPERCOLOR;
lpBook->colSelectedHoverLowerColor = SS_COLHEADER_SELECTEDHOVER_LOWERCOLOR;
lpBook->colSelectedUpperColor = SS_COLHEADER_SELECTED_UPPERCOLOR;
lpBook->colSelectedLowerColor = SS_COLHEADER_SELECTED_LOWERCOLOR;
lpBook->colHoverUpperColor = SS_COLHEADER_HOVER_UPPERCOLOR;
lpBook->colHoverLowerColor = SS_COLHEADER_HOVER_LOWERCOLOR;
lpBook->colUpperColor = SS_COLHEADER_UPPERCOLOR;
lpBook->colLowerColor = SS_COLHEADER_LOWERCOLOR;
lpBook->rowSelectedHoverUpperColor = SS_ROWHEADER_SELECTEDHOVER_UPPERCOLOR;
lpBook->rowSelectedHoverLowerColor = SS_ROWHEADER_SELECTEDHOVER_LOWERCOLOR;
lpBook->rowSelectedUpperColor = SS_ROWHEADER_SELECTED_UPPERCOLOR;
lpBook->rowSelectedLowerColor = SS_ROWHEADER_SELECTED_LOWERCOLOR;
lpBook->rowHoverUpperColor = SS_ROWHEADER_HOVER_UPPERCOLOR;
lpBook->rowHoverLowerColor = SS_ROWHEADER_HOVER_LOWERCOLOR;
lpBook->rowUpperColor = SS_ROWHEADER_UPPERCOLOR;
lpBook->rowLowerColor = SS_ROWHEADER_LOWERCOLOR;
lpBook->colHeaderSelectedBorderColor = SS_COLHEADER_SELECTEDBORDERCOLOR;
lpBook->colHeaderBorderColor = SS_COLHEADER_BORDERCOLOR;
lpBook->rowHeaderSelectedBorderColor = SS_ROWHEADER_SELECTEDBORDERCOLOR;
lpBook->rowHeaderBorderColor = SS_ROWHEADER_BORDERCOLOR;
lpBook->cornerBackColor = SS_CORNER_BACKCOLOR;
lpBook->cornerHoverColor = SS_CORNER_HOVER_BACKCOLOR;
lpBook->cornerTriangleColor = SS_CORNER_TRIANGLE_COLOR;
lpBook->cornerTriangleHoverColor = SS_CORNER_TRIANGLE_HOVER_COLOR;
lpBook->cornerTriangleBorderColor = SS_CORNER_TRIANGLE_BORDERCOLOR;
lpBook->cornerTriangleHoverBorderColor = SS_CORNER_TRIANGLE_HOVER_BORDERCOLOR;
lpBook->clrHighlightAlphaBlend = SS_EncodeSysColor(COLOR_HIGHLIGHT);
lpBook->bHighlightAlphaBlend = SS_HIGHLIGHT_ALPHABLEND;
lpBook->wHighlightHeaders = SS_HIGHLIGHTHEADERS_DEFAULT;
lpBook->wHighlightStyle = SS_HIGHLIGHTSTYLE_DEFAULT;
#endif
#ifndef SS_OLDCALC
CalcInitBook(&lpBook->CalcInfo, (HANDLE)lpBook, SS_CalcGetSheetCnt, SS_CalcGetSheetFromIndex, SS_CalcGetSheetFromName, SS_CalcSendCircularFormulaMsg);
#endif
#if SS_V80
CustCellTypeInit(&lpBook->CustCellTypes);
lpBook->fWmPrintSkipScrollBars = FALSE;
lpBook->osver.dwOSVersionInfoSize = sizeof(OSVERSIONINFO);
GetVersionEx(&lpBook->osver);
if( lpBook->osver.dwMajorVersion < 5 || (lpBook->osver.dwMajorVersion == 5 && lpBook->osver.dwMinorVersion == 0)) // win net 4.0 or windows 2000
   lpBook->wAppearanceStyle = SS_APPEARANCESTYLE_CLASSIC;
if( lpBook->osver.dwMajorVersion >= 5 )
   SS_LoadMsImg();
lpBook->fLoading = FALSE;
#endif
SS_SetSheetCount(lpBook, 1);

#ifdef SS_DLL
SS_SetAppearance(lpBook, SS_APPEARANCE_DEFAULT);
#else
SS_SetAppearance(lpBook, 0);
#endif

// removed this change 10/06/2009 -scl
// it causes redraw to get turned on when loading a file
// 22389 -scl
//lpBook->Redraw = TRUE;
return (0);
}


BOOL SS_SetSheetCount(LPSS_BOOK lpBook, short nSheetCnt)
{
BOOL fRet = TRUE;

if (nSheetCnt < 0 || nSheetCnt > SS_SHEET_MAX || nSheetCnt == lpBook->nSheetCnt)
	return (FALSE);

if (nSheetCnt > lpBook->nSheetCnt)
	fRet = SS_InsertSheets(lpBook, -1, (short)(nSheetCnt - lpBook->nSheetCnt));
else
	fRet = SS_DeleteSheets(lpBook, nSheetCnt, (short)(lpBook->nSheetCnt - nSheetCnt));

return (fRet);
}


BOOL SS_InsertSheets(LPSS_BOOK lpBook, short nSheetIndex, short nSheetCnt)
{
LPTBGLOBALHANDLE lpSheets;
short            nSheet;
short            nSheetCntNew;
BOOL             fRet = TRUE;

if (nSheetCnt <= 0 || lpBook->nSheetCnt + nSheetCnt > SS_SHEET_MAX)
	return (FALSE);

if (nSheetIndex == -1)
	nSheetIndex = lpBook->nSheetCnt;

if (nSheetIndex < 0 || nSheetIndex > lpBook->nSheetCnt)
	return (FALSE);

nSheetCntNew = lpBook->nSheetCnt + nSheetCnt;

if (!lpBook->hSheets)
	lpBook->hSheets = (TBGLOBALHANDLE)tbGlobalAlloc(GHND, sizeof(TBGLOBALHANDLE) * nSheetCntNew);
else
	lpBook->hSheets = (TBGLOBALHANDLE)tbGlobalReAlloc(lpBook->hSheets, sizeof(TBGLOBALHANDLE) * nSheetCntNew, GHND);

if (!lpBook->hSheets)
	return (FALSE);

lpSheets = (LPTBGLOBALHANDLE)tbGlobalLock(lpBook->hSheets);

// If necessary, move the sheets to the right to make room
if (nSheetIndex < lpBook->nSheetCnt)
	{
	LPSPREADSHEET lpSS;

	for (nSheet = nSheetCntNew - 1; nSheet >= nSheetIndex + nSheetCnt; nSheet--)
		{
		lpSheets[nSheet] = lpSheets[nSheet - nSheetCnt];
		lpSS = (LPSPREADSHEET)tbGlobalLock(lpSheets[nSheet]);
		lpSS->nSheetIndex = nSheet;
		tbGlobalUnlock(lpSheets[nSheet]);
		}
	}

for (nSheet = nSheetIndex; nSheet < nSheetIndex + nSheetCnt; nSheet++)
	{
	lpBook->nSheetCnt++; // RFW - 6/28/04 - 14694
	lpSheets[nSheet] = (TBGLOBALHANDLE)tbGlobalAlloc(GHND, sizeof(SPREADSHEET));
	SS_InitSheetPreHwnd(lpBook, lpSheets[nSheet], nSheet, TRUE);
	}

tbGlobalUnlock(lpBook->hSheets);

#ifdef SS_V70

// RFW - 7/25/04 - 14911
if (lpBook->nTabStripLeftSheet == -1)
	lpBook->nTabStripLeftSheet = SSTab_GetFirstVisibleTab(lpBook);

if (nSheetIndex <= lpBook->nActiveSheet)
	{
	short nActiveSheet = lpBook->nActiveSheet;

	nActiveSheet += nSheetCnt;
	nActiveSheet = min(nActiveSheet, lpBook->nSheetCnt - 1);

	// RFW - 1/27/05 - 15568
	lpBook->nActiveSheet = -1;
	SS_SetActiveSheet(lpBook, nActiveSheet);
	}
else
	{
	SS_EmptyTabStripRect(lpBook);
	SS_InvalidateTabStrip(lpBook);
	}
#endif // SS_V70

return (fRet);
}


BOOL SS_DeleteSheets(LPSS_BOOK lpBook, short nSheetIndex, short nSheetCnt)
{
LPTBGLOBALHANDLE lpSheets;
short            nSheetCntNew;
short            nSheet;
BOOL             fRet = TRUE;

if (nSheetIndex == -1)
	nSheetIndex = 0;

if (nSheetCnt == -1)
	nSheetCnt = lpBook->nSheetCnt;

nSheetCnt = min(nSheetCnt, lpBook->nSheetCnt - nSheetIndex);

if (lpBook->nSheetCnt - nSheetCnt < 0 || nSheetCnt <= 0)
	return (FALSE);

if (nSheetIndex < 0 || nSheetIndex >= lpBook->nSheetCnt)
	return (FALSE);

nSheetCntNew = lpBook->nSheetCnt - nSheetCnt;

// RFW - 4/10/07 - 20174
#ifdef SS_V70
SS_InvalidateTabStrip(lpBook);
#endif // SS_V70

lpSheets = (LPTBGLOBALHANDLE)tbGlobalLock(lpBook->hSheets);

for (nSheet = 0; nSheet < nSheetCnt; nSheet++)
	{
	LPSPREADSHEET lpSS = (LPSPREADSHEET)tbGlobalLock(lpSheets[nSheetIndex + nSheet]);
	SS_ClearSheet(lpSS, TRUE);
	tbGlobalUnlock(lpSheets[nSheetIndex + nSheet]);
	tbGlobalFree(lpSheets[nSheetIndex + nSheet]);
	}

if (nSheetIndex + nSheetCnt < lpBook->nSheetCnt)
	{
	LPSPREADSHEET lpSS;

	for (nSheet = nSheetIndex; nSheet < lpBook->nSheetCnt - nSheetCnt; nSheet++)
		{
		lpSheets[nSheet] = lpSheets[nSheet + nSheetCnt];
		lpSS = (LPSPREADSHEET)tbGlobalLock(lpSheets[nSheet]);
		lpSS->nSheetIndex = nSheet;
		tbGlobalUnlock(lpSheets[nSheet]);
		}
	}

tbGlobalUnlock(lpBook->hSheets);

if (nSheetCntNew == 0)
	{
	tbGlobalFree(lpBook->hSheets);
	lpBook->hSheets = 0;
	}
else
	lpBook->hSheets = (TBGLOBALHANDLE)tbGlobalReAlloc(lpBook->hSheets, sizeof(TBGLOBALHANDLE) * nSheetCntNew, GHND);

/* RFW - 4/10/07 - 20174 - Moved it up
// RFW - 1/12/05 - 15405
#ifdef SS_V70
SS_InvalidateTabStrip(lpBook);
#endif // SS_V70
*/

lpBook->nSheetCnt = nSheetCntNew;

#ifdef SS_V70
if (lpBook->nActiveSheet >= nSheetIndex)
	{
	short nActiveSheet = lpBook->nActiveSheet;

	nActiveSheet = max(nSheetIndex, nActiveSheet - nSheetCnt);
	nActiveSheet = min(nActiveSheet, max(0, lpBook->nSheetCnt - 1));
	nActiveSheet = SSTab_GetPrevVisibleTab(lpBook, (short)(nActiveSheet + 1));
	// RFW - 8/3/04 - 14948
	if (nActiveSheet == -1)
		nActiveSheet = SSTab_GetNextVisibleTab(lpBook, nActiveSheet);
	if (nActiveSheet == -1)
		nActiveSheet = 0;

	// RFW - 1/27/05 - 15568
	if (nSheetCntNew > 0)
		lpBook->nActiveSheet = -1;
	SS_SetActiveSheet(lpBook, nActiveSheet);
	}
else
	{
	SS_EmptyTabStripRect(lpBook);
	SS_InvalidateTabStrip(lpBook);
	}

lpBook->nSheet = min(lpBook->nSheet, max(0, lpBook->nSheetCnt - 1));
lpBook->nTabStripLeftSheet = min(lpBook->nTabStripLeftSheet, max(0, lpBook->nSheetCnt - 1));

if (IsRectEmpty(&lpBook->TabStripRect) &&
    lpBook->wTabStripPolicy == SS_TABSTRIPPOLICY_ASNEEDED)
	SS_AutoSize(lpBook, FALSE);
#endif // SS_V70

if (lpBook->CalcAuto)
  CalcEvalNeededCells(&lpBook->CalcInfo);

return (fRet);
}


BOOL SS_MoveSheets(LPSS_BOOK lpBook, short nSheetSrc, short nSheetCnt, short nSheetDest)
{
LPTBGLOBALHANDLE lpSheets;
TBGLOBALHANDLE   hSheetTemp;
LPSPREADSHEET    lpSS;
short            nSheet;
short            nCnt;
short            nActiveSheetOld;
BOOL             fRet = TRUE;

#define INTERSECT(a, aCnt, b, bCnt) (a < b + bCnt && a + aCnt > b)

if (nSheetDest == -1)
	nSheetDest = lpBook->nSheetCnt;

if (nSheetSrc < 0 || nSheetCnt <= 0 || nSheetDest < 0 ||
    nSheetSrc + nSheetCnt > lpBook->nSheetCnt ||
    nSheetDest > lpBook->nSheetCnt ||
    INTERSECT(nSheetSrc, nSheetCnt, nSheetDest, nSheetCnt))
	return (FALSE);

lpSheets = (LPTBGLOBALHANDLE)tbGlobalLock(lpBook->hSheets);

if (nSheetSrc < nSheetDest)
	{
	for (nCnt = 0; nCnt < nSheetCnt; nCnt++)
		{
		hSheetTemp = lpSheets[nSheetSrc];

		for (nSheet = nSheetSrc; nSheet < nSheetDest - 1; nSheet++)
			lpSheets[nSheet] = lpSheets[nSheet + 1];

		lpSheets[nSheetDest - 1] = hSheetTemp;
		}
	}
else
	{
	for (nCnt = 0; nCnt < nSheetCnt; nCnt++)
		{
		hSheetTemp = lpSheets[nSheetSrc + nSheetCnt - 1];

		for (nSheet = nSheetSrc + nSheetCnt - 1; nSheet > nSheetDest; nSheet--)
			lpSheets[nSheet] = lpSheets[nSheet - 1];

		lpSheets[nSheetDest] = hSheetTemp;
		}
	}

// Adjust the Sheet Indexes

nActiveSheetOld = lpBook->nActiveSheet;
for (nSheet = min(nSheetSrc, nSheetDest); nSheet < max(nSheetSrc + nSheetCnt, nSheetDest); nSheet++)
	{
	lpSS = (LPSPREADSHEET)tbGlobalLock(lpSheets[nSheet]);

#ifdef SS_V70
	if (lpSS->nSheetIndex == nActiveSheetOld)
		{
		SS_SetActiveSheetIndex(lpBook, nSheet);
		SS_EmptyTabStripRect(lpBook);
		SSTab_DisplayActiveTab(lpBook);
		SS_InvalidateTabStrip(lpBook);
		}
#endif // SS_V70

	lpSS->nSheetIndex = nSheet;
	tbGlobalUnlock(lpSheets[nSheet]);
	}

tbGlobalUnlock(lpBook->hSheets);

#ifdef SS_V70
SS_EmptyTabStripRect(lpBook);
SS_InvalidateTabStrip(lpBook);
#endif // SS_V70

return (fRet);
}


BOOL SS_InitSheetPreHwnd(LPSS_BOOK lpBook, TBGLOBALHANDLE hSS, short nSheetIndex, BOOL bInitCalcInfo)
{
LPSPREADSHEET lpSS;
HDC           hDC;

lpSS = (LPSPREADSHEET)tbGlobalLock(hSS);

lpSS->lpBook = lpBook;
lpSS->nSheetIndex = nSheetIndex;
lpSS->RestrictRows = 0;
lpSS->RestrictCols = 0;
lpSS->DocumentLocked = 0;
lpSS->wGridType = 0;
lpSS->HighlightOn = 0;
lpSS->FreezeHighlight = 0;
lpSS->NoEraseBkgnd = 0;
lpSS->NoPaint = 0;

lpSS->GridLines = 0;
lpSS->EscapeInProgress = 0;
lpSS->fFormulaMode = 0;
lpSS->fEditModeDataChange = 0;
lpSS->fRowModeEditing = 0;
lpSS->fRowModeChangeMade = 0;
lpSS->fSuspendInvert = 0;
lpSS->fSingleSelRowFound = 0;
lpSS->fInvertBlock = 0;
lpSS->fSetBorder = 0;
lpSS->fMultipleBlocksSelected = 0;
lpSS->fNoDrawFocusRect = 0;
lpSS->fSetActiveCellCalled = 0;
lpSS->fVirtualMode = 0;
lpSS->fChangeMade = 0;
lpSS->wOpMode = 0;
lpSS->wGridType = 0;

lpSS->Row.Frozen = 0;
lpSS->Col.Frozen = 0;
lpSS->Col.AutoSizeVisibleCnt = 0;
lpSS->Row.AutoSizeVisibleCnt = 0;

lpSS->wUserResize = SS_USERRESIZE_COL | SS_USERRESIZE_ROW;

#ifdef SS_BOUNDCONTROL
lpSS->fDirty = 0;
#endif
lpSS->DataFieldNameCnt = 0;

#ifdef SS_V35
lpSS->nUserColAction = SS_USERCOLACTION_DEFAULT;
lpSS->nAllColsSortIndicator = SS_COLUSERSORTINDICATOR_NONE;
lpSS->hCellNote = 0;
#endif

lpSS->DefaultFontId = SS_InitDefaultFont(lpBook);

//-------------------------------------------
//  The following code was pulled from SS_InitSpreadWithHwnd()

lpSS->Col.dCellSizeX100 = SS_WIDTH_DEFAULT;
lpSS->Col.dCellSizeInPixels = SS_ColWidthToPixels(lpSS, -1);
lpSS->Row.dCellSizeX100 = SS_HEIGHT_DEFAULT;
lpSS->Row.dCellSizeInPixels = SS_RowHeightToPixels(lpSS, -1, -1);

// Styles ????

lpSS->wGridType = SS_GRID_SOLID | SS_GRID_HORIZONTAL | SS_GRID_VERTICAL;
lpSS->GridColor = RGBCOLOR_PALEGRAY;
lpSS->LastCell.Coord.Row = -1;
lpSS->LastCell.Coord.Col = -1;
lpSS->Row.LastRefCoord.Coord.Row = -1;
lpSS->Col.LastRefCoord.Coord.Col = -1;

//lpSS->fIsBlockSelected = FALSE;
lpSS->BlockCellUL.Col = -1;
lpSS->BlockCellUL.Row = -1;

lpSS->Row.AllocCnt = 1;
lpSS->Col.AllocCnt = 1;

lpSS->Row.DataCnt = 1;
lpSS->Col.DataCnt = 1;

lpSS->Row.Max = 500;
lpSS->Col.Max = 500;

SSx_SetRowHeaderCols(lpSS, 1);
SSx_SetColHeaderRows(lpSS, 1);

lpSS->Col.lHeaderDisplayIndex = -1;
lpSS->Row.lHeaderDisplayIndex = -1;
#ifdef SS_V40
lpSS->lColHeadersUserSortIndex = -1;
#endif // SS_V40

lpSS->Row.UL = lpSS->Row.HeaderCnt + lpSS->Row.Frozen;
lpSS->Col.UL = lpSS->Col.HeaderCnt + lpSS->Col.Frozen;

lpSS->Row.CurAt = lpSS->Row.HeaderCnt;
lpSS->Col.CurAt = lpSS->Col.HeaderCnt;

lpSS->Row.MaxBeforeSort = -1;
lpSS->Col.MaxBeforeSort = -1;

// Col/Row CellSize ????

lpSS->Virtual.VMax = -1L;

#ifdef SS_OLDCALC
lpSS->fCalcBuildDependencies = TRUE;
lpSS->fCalcInProgress = 0;
#endif

lpSS->Color.BackgroundId  = SS_AddColor(lpBook->clrBackDefOrig);
lpSS->Color.ForegroundId  = SS_AddColor(lpBook->clrForeDefOrig);

lpSS->Color.ShadowColorId = SS_AddColor(SS_EncodeSysColor(COLOR_BTNFACE));
lpSS->Color.ShadowTextId  = SS_AddColor(SS_EncodeSysColor(COLOR_BTNTEXT));
lpSS->Color.ShadowDarkId  = SS_AddColor(SS_EncodeSysColor(COLOR_BTNSHADOW));
lpSS->Color.ShadowLightId = SS_AddColor(SS_EncodeSysColor(COLOR_BTNHIGHLIGHT));

lpSS->LockColor.BackgroundId = 0;
lpSS->LockColor.ForegroundId = 0;

lpSS->Col.NumStart = 1;
lpSS->Row.NumStart = 1;

lpSS->ColHeaderDisplay = SS_HEADERDISPLAY_LETTERS;
lpSS->RowHeaderDisplay = SS_HEADERDISPLAY_NUMBERS;

#ifdef SS_UTP
lpSS->dScrollArrowWidth = SS_SCROLLARROW_WIDTH;
lpSS->dScrollArrowHeight = SS_SCROLLARROW_HEIGHT;
#endif

SS_SetPrintOptions(lpSS, NULL, 0, 0);

/***************
* Setup colors
***************/

// OCX background and foreground colors should be passed in ?????

hDC = fpGetDC(GetDesktopWindow());
lpSS->Color.BackgroundId  = SS_AddColor(GetBkColor(hDC));
lpSS->Color.ForegroundId  = SS_AddColor(GetTextColor(hDC));
ReleaseDC(GetDesktopWindow(), hDC);

/****************************
* Set up default cell types
****************************/

#ifdef SS_V70
SS_SetTypeEdit(&lpSS->DefaultCellType, ES_LEFT |
               ES_AUTOHSCROLL, 32000, SS_CHRSET_CHR, SS_CASE_NOCASE);
#else
SS_SetTypeEdit(&lpSS->DefaultCellType, ES_LEFT |
               ES_AUTOHSCROLL, 60, SS_CHRSET_CHR, SS_CASE_NOCASE);
#endif // SS_V70

#if !defined(SS_NOCALC) && !defined(SS_OLDCALC)
CalcResetSheet(&lpSS->CalcInfo);
#endif

#if !defined(SS_NOCALC) && !defined(SS_OLDCALC)
if (bInitCalcInfo)
  CalcInitSheet(&lpSS->CalcInfo, &lpSS->lpBook->CalcInfo, (HANDLE)hSS,
                SS_CalcGetIndex, SS_CalcGetName, SS_CalcGetNameLen,
                SS_CalcGetData, SS_CalcSetData, SS_CalcGetDataCnt,
                SS_CalcGetMaxCols, SS_CalcGetMaxRows,
                SS_CalcGetCustRef, SS_CalcSetSending,
                #if defined(SS_DDE)
                SS_CalcDdeUpdate);
                #else
                NULL);
                #endif
#endif

#ifdef SS_V30
lpSS->OddRowColor.BackgroundId = 0;
lpSS->OddRowColor.ForegroundId = 0;
lpSS->EvenRowColor.BackgroundId = 0;
lpSS->EvenRowColor.ForegroundId = 0;
#endif

#ifdef SS_V70
lpSS->bActiveCellHighlightStyle = SS_ACTIVECELLHIGHLIGHTSTYLE_NORMAL;
lpSS->fAllowColMove = FALSE;
lpSS->fAllowRowMove = FALSE;
lpSS->fVisible = TRUE;
#ifdef FP_OCX
lpSS->pfpOleBinding = NULL;
lpSS->fBoundActive = FALSE;
#endif
#endif // SS_V70

tbGlobalUnlock(hSS);

return (0);
}


// Set up default font
SS_FONTID SS_InitDefaultFont(LPSS_BOOK lpBook)
{
SS_FONTID FontId;
HFONT     hFont;

#ifdef SS_DLL
{
HDC hDC;
HFONT hFontOld;

hDC = fpGetDC(lpBook->hWnd);

hFontOld = (HFONT)SelectObject(hDC, GetStockObject(SYSTEM_FIXED_FONT));

#ifdef SPREAD_JPN
{
LOGFONT LogFont;
_fmemset(&LogFont, '\0', sizeof(LogFont));
LogFont.lfHeight = -PT_TO_PIXELS(9);
LogFont.lfWeight = FW_NORMAL;
LogFont.lfCharSet = SHIFTJIS_CHARSET;
//LogFont.lfCharSet = 128;
lstrcpy(LogFont.lfFaceName, _T("ÇlÇr ÇoÉSÉVÉbÉN"));
//	lstrcpy(LogFont.lfFaceName, _T("MS PGothic"));
hFont = (HFONT)CreateFontIndirect(&LogFont);
}
#else
#ifdef SS_V70
{
LOGFONT LogFont;
_fmemset(&LogFont, '\0', sizeof(LogFont));
LogFont.lfHeight = (int)PT_TO_PIXELS(8);
LogFont.lfWeight = FW_NORMAL;
LogFont.lfCharSet = DEFAULT_CHARSET;
lstrcpy(LogFont.lfFaceName, _T("MS Sans Serif"));
hFont = CreateFontIndirect(&LogFont);
}
#else
hFont = hFontOld;
#endif // SS_V70
#endif // SPREAD_JPN

FontId = SS_InitFont(lpBook, hFont, -1, TRUE, NULL);

SelectObject(hDC, hFontOld);

if (hFont != hFontOld)
	DeleteObject(hFont);

ReleaseDC(lpBook->hWnd, hDC);
}

#else // SS_DLL
{
LOGFONT LogFont;

_fmemset(&LogFont, '\0', sizeof(LOGFONT));

#ifndef SPREAD_JPN
LogFont.lfHeight = (int)PT_TO_PIXELS(8);
LogFont.lfWeight = FW_NORMAL;
LogFont.lfCharSet = DEFAULT_CHARSET;
lstrcpy(LogFont.lfFaceName, _T("MS Sans Serif"));
#else
#ifdef WIN32
LogFont.lfHeight = (int)PT_TO_PIXELS(9);
#ifdef _UNICODE
lstrcpy(LogFont.lfFaceName, OLESTR("ÇlÇr ÇoÉSÉVÉbÉN"));
#else
lstrcpy(LogFont.lfFaceName, _T("ÇlÇr ÇoÉSÉVÉbÉN"));
#endif
#else
LogFont.lfHeight = (int)PT_TO_PIXELS(8);
lstrcpy(LogFont.lfFaceName, _T("System"));
#endif
LogFont.lfWeight = FW_NORMAL;
LogFont.lfCharSet = SHIFTJIS_CHARSET;
#endif // SPREAD_JPN

hFont = CreateFontIndirect(&LogFont);
}

FontId = SS_InitFont(lpBook, hFont, -1, TRUE, NULL);
DeleteObject(hFont);
#endif // SS_DLL

return (FontId);
}


//--------------------------------------------------------------------
//
//  The SS_InitSpreadWithHwnd() function handles any spreadsheet
//  initialization which must be done after a HWND exists.
//

BOOL SS_InitSpreadWithHwnd(HWND hWnd, LPSS_BOOK lpBook)
{
long lStyle = GetWindowLong(hWnd, GWL_STYLE);

#ifdef SS_DLL
lpBook->hWndOwner = GetParent(hWnd);
#else
lpBook->hWndOwner = 0;
#endif

if (lStyle & SSS_ALLOWUSERFORMULAS)
   lpBook->AllowUserFormulas = TRUE;

/*
if (lStyle & SSS_NORESIZE)
   lpBook->wUserResize = 0;
*/

if (lStyle & SSS_NOSELECTBLOCK)
   lpBook->wSelBlockOption = 0;

if (lStyle & SSS_ALLOWCELLOVERFLOW)
   lpBook->fAllowCellOverflow = TRUE;

if (lStyle & SSS_SCROLLBAREXTMODE)
   lpBook->fScrollBarExtMode = TRUE;

#ifdef SS_V30
{
FP_TT_INFO Info;
WORD       wItems = FP_TT_ITEM_STATUS | FP_TT_ITEM_ACTIVE;
#ifdef SS_V35
 wItems |= FP_TT_ITEM_SCROLLTIPSTATUS;
#endif

if (lpBook->fLoadTextTip)
   {
   Info = lpBook->TT_Info;
   lpBook->fLoadTextTip = FALSE;
   wItems |= FP_TT_ITEM_BACKCOLOR | FP_TT_ITEM_FORECOLOR |
             FP_TT_ITEM_DELAY | FP_TT_ITEM_FONT;
   }
else
   Info.wStatus = (WORD)0;

Info.fActive = TRUE;

#ifdef SS_V35 // this assumes TT_V2 is defined
Info.fShowScrollTips = (lpBook->nShowScrollTips?TRUE:FALSE);
#elif defined(TT_V2)
Info.fShowScrollTips = FALSE;
#endif

#if defined(SS_OCX) || defined(SS_VB)
#ifdef SS_OCX
if (SS_IsDesignTime(lpBook))
#else
if (VBGetMode() == MODE_DESIGN)
#endif
   Info.fActive = FALSE;
#endif

fpTextTipTerminate(hWnd);
fpTextTipSetInfo(hWnd, wItems, &Info);
}
#endif

return (0);
}

//--------------------------------------------------------------------
//
//  The SS_OnCtlColor() function handles WM_CTLCOLOR messages.
//

HBRUSH SS_OnCtlColor(HWND hWnd, HDC hDC, HWND hWndChild, int type)
{
LPSPREADSHEET   lpSS;
SS_COLORTBLITEM BackColorTblItem;
SS_COLORTBLITEM ForeColorTblItem;
LPSS_CELLTYPE   lpCellType;
SS_CELLTYPE     CellTypeTemp;
   
lpSS = SS_SheetLockActive(hWnd);

if (!lpSS)
	return (NULL);

if (type == CTLCOLOR_SCROLLBAR)
	{
	BOOL fDone = FALSE;

   // BJO 05Feb97 GRB5148 - Before fix
   //SS_GetColorItem(&BackColorTblItem,
   //                lpSS->ScrollBarColorId);
   // BJO 05Feb97 GRB5148 - Begin fix

#ifdef SS_V70
	if ((GetWindowLong(hWndChild, GWL_ID) == SBS_VERT) &&
	    lpSS->lpBook->VScrollBarColorId)
		{
		SS_GetColorItem(&BackColorTblItem, lpSS->lpBook->VScrollBarColorId);
		fDone = TRUE;
		}
	else if ((GetWindowLong(hWndChild, GWL_ID) == SBS_HORZ) &&
	    lpSS->lpBook->HScrollBarColorId)
		{
		SS_GetColorItem(&BackColorTblItem, lpSS->lpBook->HScrollBarColorId);
		fDone = TRUE;
		}
#endif // S_V70

	if (!fDone)
		{
		SS_SheetUnlockActive(hWnd);
		#if defined(WIN32)
		return FORWARD_WM_CTLCOLORSCROLLBAR(hWnd, hDC, hWndChild, DefWindowProc);

		#else
		return FORWARD_WM_CTLCOLOR(hWnd, hDC, hWndChild, type, DefWindowProc);
		#endif
		// BJO 05Feb97 GRB5148 - End fix
		}
	}
else
   {
   lpCellType = SS_RetrieveCellType(lpSS, &CellTypeTemp, NULL,
                                    lpSS->lpBook->CurVisCell.Col,
                                    lpSS->lpBook->CurVisCell.Row);

   if (lpCellType && lpCellType->Type == SS_TYPE_STATICTEXT &&
       (lpCellType->Style & SS_TEXT_SHADOW))
      SS_GetColorItem(&BackColorTblItem,
                      lpSS->Color.ShadowColorId);

   else
      {
      SS_GetColorTblItem(lpSS, lpSS->lpBook->CurVisCell.Col, lpSS->lpBook->CurVisCell.Row,
                         &BackColorTblItem, &ForeColorTblItem);

      if ((lpSS->LockColor.BackgroundId ||
           lpSS->LockColor.BackgroundId) &&
         SS_GetLock(lpSS, lpSS->lpBook->CurVisCell.Col, lpSS->lpBook->CurVisCell.Row, TRUE))
         {
         if (lpSS->LockColor.BackgroundId)
            SS_GetColorItem(&BackColorTblItem,
                            lpSS->LockColor.BackgroundId);
         if (lpSS->LockColor.ForegroundId)
            SS_GetColorItem(&ForeColorTblItem,
                            lpSS->LockColor.ForegroundId);
         }

      SetTextColor(hDC, SS_TranslateColor(ForeColorTblItem.Color));
      }
   }

SS_SheetUnlockActive(hWnd);

SetBkColor(hDC, SS_TranslateColor(BackColorTblItem.Color));
return (BackColorTblItem.hBrush);
}

//--------------------------------------------------------------------
//
//  The SS_OnHScroll() function handles WM_HSCROLL messages.
//

void SS_OnHScroll(HWND hWnd, HWND hWndScrollBar, UINT nCode, int nPos)
{
LPSS_BOOK       lpBook;
LPSPREADSHEET   lpSS;
SS_COORD        Col;
SS_COORD        xPos;
BOOL            fDoScroll = TRUE;

lpSS = SS_SheetLockActive(hWnd);
lpBook = lpSS->lpBook;
lpSS->fEditModeDataChange = FALSE;
lpSS->Col.ULPrev = lpSS->Col.UL;
lpSS->Row.ULPrev = lpSS->Row.UL;
SS_UpdateWindow(lpSS->lpBook);

// BJO 04Apr97 SEL23 - Begin remove
//if (SS_USESINGLESELBAR(lpSS))
//   if (GetFocus() != hWnd)
//      SetFocus(hWnd);
// BJO 04Apr97 SEL23 - End remove


switch (nCode)
   {
   /******************************
   * Scroll one line to the left
   ******************************/

   case SB_LINEUP:
      SS_ScrollLineUL(lpSS, SS_F_SCROLL_LEFT);
      break;

   /*******************************
   * Scroll one line to the right
   *******************************/

   case SB_LINEDOWN:
      SS_ScrollLineLR(lpSS, SS_F_SCROLL_RIGHT);
      break;

   /************************
   * Scroll one page right
   ************************/

   case SB_PAGEDOWN:
      SS_ScrollPageLR(lpSS, SS_F_SCROLL_RIGHT);
      break;

   /***********************
   * Scroll one page left
   ***********************/

   case SB_PAGEUP:
      SS_ScrollPageUL(lpSS, SS_F_SCROLL_LEFT);
      break;

   /******
   * Top
   ******/

   case SB_TOP:
      SS_ScrollHome(lpSS, SS_F_SCROLL_ROW);
      break;

   /*********
   * Bottom
   *********/

   case SB_BOTTOM:
      SS_ScrollEnd(lpSS, SS_F_SCROLL_ROW | SS_F_SCROLLMAX);
      break;

   /*************
   * End Scroll
   *************/

	case SB_ENDSCROLL:
		// RFW - 6/13/04 - 14432
		fpTextTipHideScrollTip(lpSS->lpBook->hWnd);
		break;

   /**********************************
   * The slider position has changed
   **********************************/

   case SB_THUMBTRACK:
#ifdef SS_V35
      lpBook->fThumb = TRUE;
#endif
   case SB_THUMBPOSITION:
      if ((nCode == SB_THUMBTRACK && !lpSS->lpBook->fHScrollBarTrack))// ||
          //(nCode == SB_THUMBPOSITION && lpSS->lpBook->fHScrollBarTrack))
#ifdef SS_V35
        fDoScroll = FALSE;
#else
        return;
#endif

#ifdef SS_V35
      if (nCode == SB_THUMBPOSITION)
      {
        lpBook->fThumb = FALSE;
        if (lpBook->fHScrollBarTrack)
          return;
      }
#endif

#ifdef SS_V35
      if (lpBook->fScrollBarMaxAlign && (lpBook->fThumb || fDoScroll)) //RAP01a
#else
      if (lpBook->fScrollBarMaxAlign)
#endif
      {  
#ifdef  BUGS
// Bug-014
      UINT     nWidthCols = 0;
      UINT     nWidthClientArea;
      RECT     Rect;
      SS_COORD nMaxCols;
      SS_COORD i;

      // Get the width of spread client area in pixels.      
      SS_GetClientRect(lpBook, &Rect);
      nWidthClientArea = Rect.right - Rect.left;
      
      // Get the whole spread width in pixels.
      nMaxCols = SS_GetColCnt(lpSS);
      for (i = 0; nWidthCols <= nWidthClientArea && i < nMaxCols; i++)
          nWidthCols += SS_GetColWidthInPixels(lpSS, i);
      
      // Do not scroll.
      if (nWidthCols <= nWidthClientArea)
      {
#ifdef SS_V35
        lpBook->fThumb = FALSE;
#endif
        break;
      }
      else
#endif
         Col = SS_GetLastPageLeftCol(lpSS);
      }
#ifdef SS_V35
      else if (lpBook->fThumb || fDoScroll) //RAP01a
#else
      else
#endif
         Col = SS_GetColCnt(lpSS) - 1;

      // BJO 10Sep97 JIS3690 - Before fix
      //nPos = min((int)Col, nPos);
      //nPos = max((int)(lpSS->Col.HeaderCnt + lpSS->Col.Frozen), nPos);
      //if (nPos <= Col && nPos != lpSS->Col.UL)
      //   SS_HPos(lpSS, nPos);
      // BJO 10Sep97 JIS3690 - Begin fix
#ifdef SS_V35
      if (Col <= SS_SBMAX && (lpBook->fThumb || fDoScroll))
#else
      if (Col <= SS_SBMAX)
#endif
         xPos = (SS_COORD)nPos;
#ifdef SS_V35
      else if (lpBook->fThumb | fDoScroll) //RAP01a
#else
      else
#endif
         {
         // Use the relationship (n-a)/(b-a) = (m-c)/(d-c)
         // to convert n in range a..b to m in range c..d
         double a = lpSS->Col.HeaderCnt + lpSS->Col.Frozen;
         double b = SS_SBMAX;
         double c = lpSS->Col.HeaderCnt + lpSS->Col.Frozen;
         #ifdef WIN32
         double d = SS_AdjustThumbSize(lpSS) ? SS_GetColCnt(lpSS) - 1 : Col;  // BJO 09Oct98 GRB6082
         #else
         double d = Col;
         #endif
         double n = nPos;
         double m = (n - a) * (d - c) / (b - a) + c;
         xPos = (SS_COORD)(m + 0.5);
         }

      xPos = min(Col, xPos);
      xPos = max(lpSS->Col.HeaderCnt + lpSS->Col.Frozen, xPos);

      // The following code prevents the scroll bar
      // from stopping on a hidden row.
#ifdef SS_V35
      if (lpSS->lpBook->fThumb || fDoScroll)
#endif
      {
      SS_COORD xPosTemp = xPos;
      SS_COORD MaxCnt = SS_GetColCnt(lpSS);

      while (xPosTemp < MaxCnt - 1 && SS_GetColWidthInPixels(lpSS, xPosTemp) == 0)
         xPosTemp++;

      if (xPosTemp >= MaxCnt - 1)
         {
		 //Modify by BOC 99.6.18 (hyt)--------------------
		 //for more decrease 1 if reach right
         //xPosTemp = xPos - 1;
		  xPosTemp = xPos;
		 //-----------------------------------------------
         while (xPosTemp >= lpSS->Col.HeaderCnt + lpSS->Col.Frozen &&
                SS_GetColWidthInPixels(lpSS, xPosTemp) == 0)
            xPosTemp--;

         if (xPosTemp >= lpSS->Col.HeaderCnt + lpSS->Col.Frozen)
            xPos = xPosTemp;
         }
      else
         xPos = xPosTemp;
      }

#ifdef SS_V35
//RAP01a >>
      if (lpSS->lpBook->fThumb)
         SS_TTScrollTip(lpSS, TRUE, xPos?xPos:1);
//<< RAP01a
#endif

#ifdef SS_V35
      if (xPos <= Col && xPos != lpSS->Col.UL && fDoScroll)
#else
      if (xPos <= Col && xPos != lpSS->Col.UL)
#endif
         SS_HPos(lpSS, xPos);
      // BJO 10Sep97 JIS3690 - End fix

#ifdef SS_V35
      if (!fDoScroll)
        return;
#endif
      break;

   }
SS_SheetUnlockActive(hWnd);
}

//--------------------------------------------------------------------
//
//  The SS_OnVScroll() function handles WM_VSCROLL messages.
//

void SS_OnVScroll(HWND hWnd, HWND hWndScrollBar, UINT nCode, int nPos)
{
LPSPREADSHEET   lpSS;
SS_COORD        Row;
SS_COORD        yPos;
SS_COORD        ScrollLast;
SS_COORD        nMax;
BOOL            fDoScroll = TRUE;

lpSS = SS_SheetLockActive(hWnd);
lpSS->fEditModeDataChange = FALSE;
lpSS->lpBook->wScrollDirection = 0;
lpSS->Col.ULPrev = lpSS->Col.UL;
lpSS->Row.ULPrev = lpSS->Row.UL;
SS_UpdateWindow(lpSS->lpBook);

// BJO 04Apr97 SEL23 - Begin remove
//if (SS_USESINGLESELBAR(lpSS))
//   if (GetFocus() != hWnd)
//      SetFocus(hWnd);
// BJO 04Apr97 SEL23 - End remove

switch (nCode)
   {
   /**********
   * Line up
   **********/

   case SB_LINEUP:
      SS_ScrollLineUL(lpSS, SS_F_SCROLL_UP);
      break;

   /************
   * Line down
   ************/

   case SB_LINEDOWN:
      SS_ScrollLineLR(lpSS, SS_F_SCROLL_DOWN);
      break;

   /************
   * Page down
   ************/

   case SB_PAGEDOWN:
      SS_ScrollPageLR(lpSS, SS_F_SCROLL_DOWN);
      break;

   /**********
   * Page up
   **********/

   case SB_PAGEUP:
      SS_ScrollPageUL(lpSS, SS_F_SCROLL_UP);
      break;

   /******
   * Top
   ******/

   case SB_TOP:
      SS_ScrollHome(lpSS, SS_F_SCROLL_COL);
      break;

   /*********
   * Bottom
   *********/

   case SB_BOTTOM:
      SS_ScrollEnd(lpSS, SS_F_SCROLL_COL | SS_F_SCROLLMAX);
      break;

   /*************
   * End Scroll
   *************/

	case SB_ENDSCROLL:
		// RFW - 6/13/04 - 14432
		fpTextTipHideScrollTip(lpSS->lpBook->hWnd);
		break;

   /**********************************
   * The slider position has changed
   **********************************/

   case SB_THUMBTRACK:
#ifdef SS_V35
      lpSS->lpBook->fThumb = TRUE;
#endif
   case SB_THUMBPOSITION:
      if ((nCode == SB_THUMBTRACK && !lpSS->lpBook->fVScrollBarTrack))// ||
          //(nCode == SB_THUMBPOSITION && lpSS->lpBook->fVScrollBarTrack))
         {
#ifdef SS_V35
         fDoScroll = FALSE;
#else
         return;
#endif
         }
#ifdef SS_V35
      else if (nCode == SB_THUMBPOSITION)
      {
        lpSS->lpBook->fThumb = FALSE;
        if (lpSS->lpBook->fVScrollBarTrack)
          return;
      }
#endif

	// RFW - 2/18/05 - 15763
	if (lpSS->fVirtualMode && (lpSS->Virtual.lVStyle & SSV_SCROLLBARSHOWBUFFER))
      ScrollLast = lpSS->Virtual.VPhysSize;

   else if (!lpSS->lpBook->fScrollBarShowMax)
      {
      ScrollLast = max(lpSS->Row.DataCnt - 1,
                       SS_DEFAULT_VSCROLL + lpSS->Row.HeaderCnt - 1);
      ScrollLast = min(SS_GetRowCnt(lpSS) - 1, ScrollLast);
      }

   else
      ScrollLast = SS_GetRowCnt(lpSS) - 1;

   nMax = ScrollLast;

#ifdef SS_V35
      if (lpSS->lpBook->fScrollBarMaxAlign && (lpSS->lpBook->fThumb || fDoScroll)) //RAP01a
#else
      if (lpSS->lpBook->fScrollBarMaxAlign)
#endif
         {
#ifdef  BUGS
// Bug-014
         UINT     nHeightRows = 0;
         UINT     nHeightClientArea;
         RECT     Rect;
         SS_COORD nMaxRows;
         SS_COORD i;

         // Get the height of spread client area in pixels.
         SS_GetClientRect(lpSS->lpBook, &Rect);
         nHeightClientArea = Rect.bottom - Rect.top;

         // Get the whole spread height in pixels.
         nMaxRows = SS_GetRowCnt(lpSS);
         for (i = 0; nHeightRows <= nHeightClientArea && i < nMaxRows; i++)
            nHeightRows += SS_GetRowHeightInPixels(lpSS, i);

         // Do not scroll.
         if (nHeightRows <= nHeightClientArea)
            {
#ifdef SS_V35
            lpSS->lpBook->fThumb = FALSE;
#endif
            break;
            }
         else
#endif
            Row = SSx_GetLastPageTopRow(lpSS, nMax + 1);
         }
#ifdef SS_V35
      else if (lpSS->lpBook->fThumb || fDoScroll) //RAP01a
#else
      else
#endif
         Row = nMax;

#ifdef SS_V35
      if ((lpSS->lpBook->fThumb || fDoScroll) && lpSS->fVirtualMode && //RAP01a
          (lpSS->Virtual.lVStyle & SSV_SCROLLBARSHOWBUFFER))
#else
      if (lpSS->fVirtualMode &&
          (lpSS->Virtual.lVStyle & SSV_SCROLLBARSHOWBUFFER))
#endif
         yPos = nPos + lpSS->Virtual.VTop - 1;

#ifdef SS_V35
      else if ((lpSS->lpBook->fThumb || fDoScroll) && Row <= SS_SBMAX) //RAP01a
#else
      else if (Row <= SS_SBMAX)
#endif
         yPos = (SS_COORD)nPos;

#ifdef SS_V35
      else if (lpSS->lpBook->fThumb || fDoScroll) //RAP01a
#else
      else
#endif
         {
         //yPos = (SS_COORD)((double)Row * ((double)nPos /
         //                     (double)SS_SBMAX));
         
         // BJO - 29Feb96 - KEM3178
         // Use the relationship (n-a)/(b-a) = (m-c)/(d-c)
         // to convert n in range a..b to m in range c..d
         double a = lpSS->Row.HeaderCnt + lpSS->Row.Frozen;
         double b = SS_SBMAX;
         double c = lpSS->Row.HeaderCnt + lpSS->Row.Frozen;
         #ifdef WIN32
         double d = SS_AdjustThumbSize(lpSS) ? SS_GetRowCnt(lpSS) - 1 : Row;
         #else
         double d = Row;
         #endif
         double n = nPos;
         double m = (n - a) * (d - c) / (b - a) + c;
         yPos = (SS_COORD)(m + 0.5);
         }

      yPos = min(Row, yPos);
      yPos = max(lpSS->Row.HeaderCnt + lpSS->Row.Frozen, yPos);

      // The following code prevents the scroll bar
      // from stopping on a hidden row.
#ifdef SS_V35
      if (lpSS->lpBook->fThumb || fDoScroll)
#endif
      {
      SS_COORD yPosTemp = yPos;
      SS_COORD MaxCnt = SS_GetRowCnt(lpSS);

      while (yPosTemp < MaxCnt - 1 &&
             SS_GetRowHeightInPixels(lpSS, yPosTemp) == 0)
         yPosTemp++;

      if (yPosTemp >= MaxCnt - 1)
         {
		 //Modify by BOC 99.6.18 (hyt)--------------------
		 //for more decrease 1 if reach bottom
         //yPosTemp = yPos - 1;
		  yPosTemp = yPos;
		 //-----------------------------------------------
         while (yPosTemp >= lpSS->Row.HeaderCnt + lpSS->Row.Frozen && SS_GetRowHeightInPixels(lpSS, yPosTemp) == 0)
            yPosTemp--;

         if (yPosTemp >= lpSS->Row.HeaderCnt + lpSS->Row.Frozen)
            yPos = yPosTemp;
         }
      else
         yPos = yPosTemp;
      }

#ifdef SS_V35
//RAP01a >>
      if (lpSS->lpBook->fThumb)
         SS_TTScrollTip(lpSS, FALSE, yPos?yPos:1);
//<< RAP01a
#endif

      /*******************************************************
      * If in virtual mode and the ScrollBarShowBuffer style
      * is not being used, then do not do anything
      *******************************************************/
#ifdef SS_V35
      if ((lpSS->lpBook->fThumb || fDoScroll) && (!lpSS->fVirtualMode || 
          (lpSS->Virtual.lVStyle & SSV_SCROLLBARSHOWBUFFER) ||
          (lpSS->Virtual.VMax > 0 && lpSS->Row.Max > 0)) && fDoScroll)
#else
      if (!lpSS->fVirtualMode ||
          (lpSS->Virtual.lVStyle & SSV_SCROLLBARSHOWBUFFER) ||
          (lpSS->Virtual.VMax > 0 && lpSS->Row.Max > 0))
#endif
        if (yPos <= Row && yPos != lpSS->Row.UL)
          SS_VPos(lpSS, yPos);
      
#ifdef SS_V35
      if (!fDoScroll)
        return;
#endif
      break;

   }

SS_SheetUnlockActive(hWnd);
}

//--------------------------------------------------------------------
//
//  The SS_OnCommand() function handles WM_COMMAND messages.
//

void SS_OnCommand(HWND hWnd, int id, HWND hWndCtl, UINT nCode)
{
LPSPREADSHEET   lpSS;
SS_CELLTYPE     CellTypeTemp;
LPSS_CELLTYPE   lpCellType;
short           dType;

lpSS = SS_SheetLockActive(hWnd);

lpCellType = SS_RetrieveCellType(lpSS, &CellTypeTemp, NULL,
                                 lpSS->Col.EditAt, lpSS->Row.EditAt);

switch (lpCellType->Type)
   {
   case SS_TYPE_DATE:
   case SS_TYPE_EDIT:
   case SS_TYPE_PIC:
   case SS_TYPE_FLOAT:
   case SS_TYPE_INTEGER:
   case SS_TYPE_TIME:
#ifdef SS_V40
   case SS_TYPE_CURRENCY:
   case SS_TYPE_NUMBER:
   case SS_TYPE_PERCENT:
#endif // SS_V40
#ifdef SS_V70
	case SS_TYPE_SCIENTIFIC:
#endif // SS_V70
      switch (nCode)
         {
			// RFW - 5/24/04 - 14465
			case EN_MAXTEXT:
            if (lpSS->lpBook->EditModeOn && lpCellType->Type == SS_TYPE_EDIT)
					{
					SS_Beep(lpSS->lpBook);
					return;
					}
				break;

         case EN_INVALIDDATA:
            SS_SendMsgCommand(lpSS->lpBook, lpSS, SSN_INVALIDDATA, FALSE);
            break;

      #ifdef SS_UTP
         case EN_AUTOADVANCEPREV:
         case EN_AUTOADVANCENEXT:
         case EN_AUTOADVANCEUP:
         case EN_AUTOADVANCEDOWN:
            lpSS->wLastAdvanceMsg = nCode;
            switch (nCode)
               {
               case EN_AUTOADVANCEPREV:
                  SS_ScrollPrevCell(lpSS);
                  break;

               case EN_AUTOADVANCENEXT:
                  SS_ScrollNextCell(lpSS);
                  break;

               case EN_AUTOADVANCEUP:
                  lpSS->dLastAdvancePos = (short)SendMessage(
                                              hWndCtl, EM_GETPIXELPOS,
                                              PIXEL_CURRENT, 0L);
                  SS_ScrollLineUL(lpSS, SS_F_SCROLL_UP | SS_F_MOVEACTIVECELL);
                  break;

               case EN_AUTOADVANCEDOWN:
                  lpSS->dLastAdvancePos = (short)SendMessage(
                                              hWndCtl, EM_GETPIXELPOS,
                                              PIXEL_CURRENT, 0L);
                  SS_ScrollLineLR(lpSS, SS_F_SCROLL_DOWN | SS_F_MOVEACTIVECELL);
                  break;
               }

            break;
      #endif

#ifdef SS_V30
         case EN_CHANGE:
            if (lpSS->lpBook->EditModeOn && !lpSS->fIgnoreEditChange)
               SS_SendMsgCoords(lpSS, SSM_EDITCHANGE, GetDlgCtrlID(hWnd),
                                lpSS->Col.EditAt, lpSS->Row.EditAt);

            break;
#endif // SS_V30

#ifdef SS_V40
         case EN_UPDATE:
#ifdef SS_V70
            if (lpSS->lpBook->EditModeOn)
					{
					SS_CELLTYPE CellType;

	            SS_RetrieveCellType(lpSS, &CellType, NULL, lpSS->Col.EditAt, lpSS->Row.EditAt);

					if (CellType.Type == SS_TYPE_SCIENTIFIC)
						{
						long lLen = GetWindowTextLength(hWndCtl);
						TBGLOBALHANDLE hText = tbGlobalAlloc(GHND, (lLen + 2) * sizeof(TCHAR));
						LPTSTR lpszText = (LPTSTR)tbGlobalLock(hText);
						BOOL fErr = FALSE;

						GetWindowText(hWndCtl, lpszText, lLen + 1);
						if (!SS_ScientificCheckRange(&CellType, lpszText, TRUE))
							{
							BOOL fChangeMade;
							SS_Beep(lpSS->lpBook);
							// RFW - 6/17/04 - 14570
							lpSS->fIgnoreEditChange = TRUE;
							fChangeMade = (BOOL)SendMessage(hWndCtl, EM_GETMODIFY, 0, 0L);
							SetWindowText(hWndCtl, lpSS->lpBook->szScientificBuffer);
							SendMessage(hWndCtl, EM_SETMODIFY, fChangeMade, 0L);
							SendMessage(hWndCtl, EM_SETSEL, (WPARAM)lpSS->lpBook->dwScientificSel1, (WPARAM)lpSS->lpBook->dwScientificSel2);
							fErr = TRUE;
							}

						if (!fErr)
							lstrcpyn(lpSS->lpBook->szScientificBuffer, lpszText, SCIENTIFIC_BUFFER_SIZE);
						tbGlobalUnlock(hText);
						tbGlobalFree(hText);
						}
					}
#endif // SS_V70

            if (lpSS->lpBook->EditModeOn && lpSS->lpBook->fEditOverflow)
					{
					long lLen = (long)SendMessage(hWndCtl, WM_GETTEXTLENGTH, 0, 0);

					if (lLen > 0)
						{
						TBGLOBALHANDLE hText;
						LPTSTR lpszText;
						RECT Rect, RectExpanded;

						hText = tbGlobalAlloc(GHND, (lLen + 1) * sizeof(TCHAR));
						lpszText = (LPTSTR)tbGlobalLock(hText);
						SendMessage(hWndCtl, WM_GETTEXT, lLen + 1, (LPARAM)lpszText);

						SS_GetLargestExpandingEditCell(lpSS, lpCellType, hWndCtl, lpSS->Col.EditAt,
                                                 lpSS->Row.EditAt, lpszText, lLen, FALSE,
                                                 &RectExpanded);
						tbGlobalUnlock(hText);
						tbGlobalFree(hText);

						GetWindowRect(hWndCtl, &Rect);
						if ((Rect.right - Rect.left != RectExpanded.right - RectExpanded.left) ||
						    (Rect.bottom - Rect.top != RectExpanded.bottom - RectExpanded.top))
				         MoveWindow(hWndCtl, RectExpanded.left, RectExpanded.top,
                                RectExpanded.right - RectExpanded.left,
                                RectExpanded.bottom - RectExpanded.top, TRUE);
						}
					}

            break;
#endif // SS_V40
         }

      break;

   case SS_TYPE_BUTTON:
   case SS_TYPE_CHECKBOX:
      switch (nCode)
         {
         case BN_CLICKED:
      //     if (!lpSS->lpBook->EditModeTurningOn &&
      //          (dType = SS_TypeGetControlType(lpSS, (short)id)) != -1)
            if (!lpSS->lpBook->fIgnoreBNClicked &&
                (dType = SS_TypeGetControlType(lpSS, (short)id)) != -1)
               {
               lpSS->lpBook->CurCellSendingMsg.Col = lpSS->Col.EditAt;
               lpSS->lpBook->CurCellSendingMsg.Row = lpSS->Row.EditAt;

               if (dType == SS_TYPE_BUTTON)
                  {
                  if (SendMessage(hWndCtl, SBM_GETBUTTONSTATE, 0, 0L))
                     SS_SendMsgCommand(lpSS->lpBook, lpSS, SSN_BUTTONDOWN, TRUE);
                  else
                     SS_SendMsgCommand(lpSS->lpBook, lpSS, SSN_BUTTONUP, TRUE);

                  if (!IsWindow(hWnd))
                     return;
                  }

               else if (dType == SS_TYPE_CHECKBOX)
                  {
      #ifdef SS_UTP
                  if (SendMessage(hWndCtl, BM_GETSTATE, 0, 0L))
                     SS_SendMsgCommand(lpSS->lpBook, lpSS, SSN_BUTTONUP, TRUE);
                  else
                     SS_SendMsgCommand(lpSS->lpBook, lpSS, SSN_BUTTONDOWN, TRUE);
      #else
                  if (SendMessage(hWndCtl, BM_GETSTATE, 0, 0L))
                     SS_SendMsgCommand(lpSS->lpBook, lpSS, SSN_BUTTONDOWN, TRUE);
                  else
                     SS_SendMsgCommand(lpSS->lpBook, lpSS, SSN_BUTTONUP, TRUE);
      #endif
                  }
               }
            SS_SheetUnlockActive(hWnd);
            return;

         case BN_CHECKBOXTYPING:
            SS_SendMsgCommandActiveSheet(hWnd, SSN_CHECKBOXTYPING, FALSE);
            break;
         }

      break;

#ifdef SS_V30
   case SS_TYPE_COMBOBOX:
      switch (nCode)
         {
         case CBN_EDITCHANGE:
            if (lpSS->lpBook->EditModeOn)
               SS_SendMsgCoords(lpSS, SSM_EDITCHANGE, GetDlgCtrlID(hWnd),
                                lpSS->Col.EditAt, lpSS->Row.EditAt);
         }

      break;
#endif
   }

SS_SheetUnlockActive(hWnd);

#ifdef SS_NOSUPERCLASS
#ifdef WIN32
VBDefControlProc(VBGetHwndControl(hWnd), hWnd, WM_COMMAND,
                 MAKEWPARAM(id, nCode), (LPARAM)hWndCtl);
#else
VBDefControlProc(VBGetHwndControl(hWnd), hWnd, WM_COMMAND,
                 (WPARAM)id, MAKELPARAM(hWndCtl, nCode));
#endif
#else
FORWARD_WM_COMMAND(hWnd, id, hWndCtl, nCode, DefWindowProc);
#endif
}


#ifdef SS_V40
void SS_GetLargestExpandingEditCell(LPSPREADSHEET lpSS, LPSS_CELLTYPE lpCellType, HWND hWndCtl,
                                    SS_COORD Col, SS_COORD Row, LPTSTR lpszText, long lLen,
                                    BOOL fBegin, LPRECT lpRectOut)
{
HFONT hFont = (HFONT)SendMessage(hWndCtl, WM_GETFONT, 0, 0);
HFONT hFontOld;
SIZE Size;
SS_COORD i;
SS_COORD Col2;
RECT RectSSClient;
RECT RectEditClient, RectEdit;
RECT Rect;
HDC hDC = GetDC(hWndCtl);
int x, y, cx, cy, iWidth;
int xXtra, yXtra, xXtraNew, yXtraNew;

GetClientRect(hWndCtl, &RectEditClient);
SendMessage(hWndCtl, EM_GETRECT, 0, (LPARAM)&RectEdit);

xXtra = RectEditClient.right - (RectEdit.right - RectEdit.left);
yXtra = 0;

xXtraNew = 1;
yXtraNew = 1;
if (lpCellType->Type == SS_TYPE_EDIT && lpCellType->Style & (ES_AUTOHSCROLL | ES_MULTILINE))
	yXtraNew = 3;

xXtra += xXtraNew;
yXtra += yXtraNew;

Col2 = Col;
if (SS_GetActualCell(lpSS, Col, Row, &Col, &Row, &Rect))
	{
	// it is a spanned cell.
	SS_SELBLOCK Block;

	if (SS_SpanCalcBlock(lpSS, Col, Row, &Block))
		Col2 = Block.LR.Col;
	}

x = Rect.left;
y = Rect.top;
cx = Rect.right - Rect.left;
cy = Rect.bottom - Rect.top;
// SS_GetCellCoord(lpSS, lpSS->Col.UL, lpSS->Row.UL, Col, Row, &x, &y, &cx, &cy);

hFontOld = SelectObject(hDC, hFont);
GetTextExtentPoint32(hDC, lpszText, lLen, &Size);

if (Size.cx > cx - xXtra)
	{
	if (lpCellType->Style & ES_RIGHT)
		{
		for (i = Col - 1; cx - xXtra < Size.cx && i >= lpSS->Col.UL; i--)
			{
			iWidth = SS_GetColWidthInPixels(lpSS, i);
			cx += iWidth;
			x -= iWidth;
			}
		}

	else if (lpCellType->Style & ES_CENTER)
		{
		SS_COORD RightVisCell = SS_GetRightVisCell(lpSS, lpSS->Col.UL);
		int iTextHalfWidthLeft = ((Size.cx - cx + xXtra) / 2);
		int iTextHalfWidthRight = ((Size.cx - cx + xXtra + 1) / 2);
		int iWidthRight = 0, iWidthLeft = 0;

		for (i = Col2 + 1; iWidthRight < iTextHalfWidthRight && i <= RightVisCell; i++)
			iWidthRight += SS_GetColWidthInPixels(lpSS, i);

		for (i = Col - 1; iWidthLeft < iTextHalfWidthLeft && i >= lpSS->Col.UL; i--)
			iWidthLeft += SS_GetColWidthInPixels(lpSS, i);

		if (!(lpCellType->Style & ES_MULTILINE) ||
          (iTextHalfWidthLeft <= iWidthLeft && iTextHalfWidthRight <= iWidthRight))
			{
			x -= iWidthLeft;
			cx += iWidthLeft + iWidthRight;
			}
		}

	else // ES_LEFT
		{
		SS_COORD RightVisCell = SS_GetRightVisCell(lpSS, lpSS->Col.UL);
		for (i = Col2 + 1; cx - xXtra < Size.cx && i <= RightVisCell; i++)
			cx += SS_GetColWidthInPixels(lpSS, i);
		}

	if ((lpCellType->Style & ES_MULTILINE) && Size.cx + xXtra > cx)
		{
		SS_COORD BottomVisCell = SS_GetBottomVisCell(lpSS, lpSS->Row.UL);

		if (fBegin)
			{
			RECT RectTemp;

			SetRect(&RectTemp, 0, 0, cx - xXtra, Size.cy);

         Size.cy = DrawText(hDC, lpszText, -1, &RectTemp, DT_WORDBREAK | DT_CALCRECT | DT_EDITCONTROL);
			}
		else
			Size.cy *= (long)SendMessage(hWndCtl, EM_GETLINECOUNT, 0, 0);

		for (i = Row + 1; cy - yXtra < Size.cy && i <= BottomVisCell; i++)
			cy += SS_GetRowHeightInPixels(lpSS, i);
		}
	}

SelectObject(hDC, hFontOld);
ReleaseDC(hWndCtl, hDC);

SS_GetClientRect(lpSS->lpBook, &RectSSClient);
x += 1 + (xXtraNew / 2);
y += 1 + (yXtraNew / 2);

SetRect(lpRectOut, x, y, min(RectSSClient.right, x + cx - xXtraNew),
        min(RectSSClient.bottom, y + cy - yXtraNew));
}
#endif // SS_V40

//--------------------------------------------------------------------

#ifndef SS_OLDCALC
short CALLBACK SS_CalcGetSheetCnt(HANDLE hBook)
{
  LPSS_BOOK lpBook = (LPSS_BOOK)hBook;

  return lpBook->nSheetCnt;
}
#endif

//--------------------------------------------------------------------

#ifndef SS_OLDCALC
LPCALC_SHEET CALLBACK SS_CalcGetSheetFromIndex(HANDLE hBook, short nIndex)
{
  LPSS_BOOK lpBook = (LPSS_BOOK)hBook;
  LPSPREADSHEET lpSS;

  lpSS = SS_BookLockSheetIndex(lpBook, nIndex);
  SS_BookUnlockSheetIndex(lpBook, nIndex);

  if (lpSS != NULL)
    return &lpSS->CalcInfo;
  else
    return NULL;
}
#endif

//--------------------------------------------------------------------

#ifndef SS_OLDCALC
LPCALC_SHEET CALLBACK SS_CalcGetSheetFromName(HANDLE hBook, LPTSTR lpszText, int nLen)
{
  LPSS_BOOK lpBook = (LPSS_BOOK)hBook;
  LPSPREADSHEET lpSS;
  BOOL bFound = FALSE;
  short i;

  for (i = 0; !bFound && i < lpBook->nSheetCnt; i++)
  {
    if (lpSS = SS_BookLockSheetIndex(lpBook, i))
    {
      #if defined(SS_V70)
      if (lpSS->hSheetName)
      {
        LPTSTR lpszSheetName = (LPTSTR)tbGlobalLock(lpSS->hSheetName);
        bFound = (_tcslen(lpszSheetName) == (size_t)nLen) && (_tcsnicmp(lpszSheetName, lpszText, nLen) == 0);
        tbGlobalUnlock(lpSS->hSheetName);
      }
      else
      {
        TCHAR szName[SS_MAXSHEETNAME + 1];
        SSTab_GetSheetDisplayName(lpBook, i, szName, SS_MAXSHEETNAME + 1);
        bFound = (_tcslen(szName) == (size_t)nLen) && (_tcsnicmp(szName, lpszText, nLen) == 0);
      }
      #endif
      SS_BookUnlockSheetIndex(lpBook, i);
    }
  }
  return bFound ? &lpSS->CalcInfo : NULL;
}
#endif

//--------------------------------------------------------------------

#ifndef SS_OLDCALC
void CALLBACK SS_CalcSendCircularFormulaMsg(HANDLE hBook, short nSheet, SS_COORD lCol, SS_COORD lRow)
{
  #if defined(SS_V80)
  LPSS_BOOK lpBook = (LPSS_BOOK)hBook;
  SS_CIRCULARFORMULA msg;

  msg.Sheet = nSheet + 1;
  msg.Col = lCol;
  msg.Row = lRow;
  SS_SendMsg(lpBook, NULL, SSM_CIRCULARFORMULA, GetDlgCtrlID(lpBook->hWnd), (LPARAM)(LPVOID)&msg);
  #endif
}
#endif

//--------------------------------------------------------------------

#ifndef SS_OLDCALC
int CALLBACK SS_CalcGetIndex(HANDLE hSS)
{
  LPSPREADSHEET lpSS;
  int nRet = 0;
  if (hSS && (lpSS = (LPSPREADSHEET)tbGlobalLock(hSS)))
  {
    nRet = lpSS->nSheetIndex;
    tbGlobalUnlock(hSS);
  }
  return nRet;
}
#endif

//--------------------------------------------------------------------

#ifndef SS_OLDCALC
int CALLBACK SS_CalcGetName(HANDLE hSS, LPTSTR lpszText, int nTextLen)
{
  LPSPREADSHEET lpSS;
  int nRet = 0;
  if (hSS && (lpSS = (LPSPREADSHEET)tbGlobalLock(hSS)))
  {
    #if defined(SS_V70)
    int nNameLen;
    if (lpSS->hSheetName)
    {
      LPTSTR lpszName = (LPTSTR)tbGlobalLock(lpSS->hSheetName);
      nNameLen = (int)_tcslen(lpszName);
      nRet = max(0, min(nNameLen, nTextLen - 1));
      _tcsncpy(lpszText, lpszName, nRet);
      tbGlobalUnlock(lpSS->hSheetName);
    }
    else
    {
      TCHAR szName[SS_MAXSHEETNAME + 1];
      SSTab_GetSheetDisplayName(lpSS->lpBook, lpSS->nSheetIndex, szName, SS_MAXSHEETNAME + 1);
      nNameLen = (int)_tcslen(szName);
      nRet = max(0, min(nNameLen, nTextLen - 1));
      _tcsncpy(lpszText, szName, nRet);
    }
    #endif
    tbGlobalUnlock(hSS);
  }
  if (nRet < nTextLen)
    lpszText[nRet] = '\0';
  return nRet;
}
#endif

//--------------------------------------------------------------------

#ifndef SS_OLDCALC
int CALLBACK SS_CalcGetNameLen(HANDLE hSS)
{
  LPSPREADSHEET lpSS;
  int nRet = 0;
  if (hSS && (lpSS = (LPSPREADSHEET)tbGlobalLock(hSS)))
  {
    #if defined(SS_V70)
    if (lpSS->hSheetName)
    {
      LPTSTR lpszName = (LPTSTR)tbGlobalLock(lpSS->hSheetName);
      nRet = (int)_tcslen(lpszName);
      tbGlobalUnlock(lpSS->hSheetName);
    }
    else
    {
      TCHAR szName[SS_MAXSHEETNAME + 1];
      SSTab_GetSheetDisplayName(lpSS->lpBook, lpSS->nSheetIndex, szName, SS_MAXSHEETNAME + 1);
      nRet = (int)_tcslen(szName);
    }
    #endif
    tbGlobalUnlock(hSS);
  }
  return nRet;
}
#endif

//--------------------------------------------------------------------
//
//  The SS_CalcGetData() function is a call back function used by
//  the calc engine to retrieve data from a cell.
//

#ifndef SS_OLDCALC
BOOL CALLBACK SS_CalcGetData(HANDLE hSS, SS_COORD lCol, SS_COORD lRow,
                             LPCALC_VALUE lpValue)
{
  LPSPREADSHEET lpSS;
  LPSS_COL lpCol;
  LPSS_ROW lpRow;
  SS_CELLTYPE cellType;
  SS_DATA data;
  LPTSTR lpszData;
  LPTSTR lpszValue;
  TCHAR szBuff[32];

  lpSS = (LPSPREADSHEET)tbGlobalLock(hSS);

  SS_AdjustCellCoords(lpSS, &lCol, &lRow);

  lpCol = SS_LockColItem(lpSS, lCol);
  lpRow = SS_LockRowItem(lpSS, lRow);
  SS_RetrieveCellType(lpSS, &cellType, NULL, lCol, lRow);
  SS_GetDataStruct(lpSS, lpCol, lpRow, NULL, &data, lCol, lRow);
  _fmemset(lpValue, 0, sizeof(SS_VALUE));
  if( !data.bDataType )
    lpValue->Status = SS_VALUE_STATUS_EMPTY;
  else if( SS_TYPE_INTEGER == cellType.Type )
  {
    lpValue->Status = SS_VALUE_STATUS_OK;
    lpValue->Type = SS_VALUE_TYPE_LONG;
    if( SS_TYPE_INTEGER == data.bDataType )
      lpValue->Val.ValLong = data.Data.lValue;
    else if( SS_TYPE_FLOAT == data.bDataType )
      lpValue->Val.ValLong = (long)data.Data.dfValue;
    else if( data.Data.hszData )
    {
      lpszData = (LPTSTR)tbGlobalLock(data.Data.hszData);
      _ftcsncpy(szBuff, lpszData, sizeof(szBuff)/sizeof(TCHAR));
      lpValue->Val.ValLong = StringToLong(szBuff);
      tbGlobalUnlock(data.Data.hszData);
    }
    else
      lpValue->Val.ValLong = 0;
  }
  else if( SS_TYPE_FLOAT == cellType.Type
#ifdef SS_V40
           || SS_TYPE_CURRENCY == cellType.Type
           || SS_TYPE_NUMBER == cellType.Type
           || SS_TYPE_PERCENT == cellType.Type
#endif // SS_V40
#ifdef SS_V70
           || SS_TYPE_SCIENTIFIC == cellType.Type
#endif // SS_V70
         )
  {
    lpValue->Status = SS_VALUE_STATUS_OK;
    lpValue->Type = SS_VALUE_TYPE_DOUBLE;
    if( SS_TYPE_INTEGER == data.bDataType )
      lpValue->Val.ValDouble = (double)data.Data.lValue;
    else if( SS_TYPE_FLOAT == data.bDataType )
      lpValue->Val.ValDouble = data.Data.dfValue;
    else if( data.Data.hszData )
    {
      lpszData = (LPTSTR)tbGlobalLock(data.Data.hszData);
      _ftcsncpy(szBuff, lpszData, sizeof(szBuff)/sizeof(TCHAR));
      SS_StringToNum(lpSS, &cellType, szBuff, &lpValue->Val.ValDouble);
      tbGlobalUnlock(data.Data.hszData);
    }
    else
      lpValue->Val.ValDouble = 0.0;
  }
#ifdef SS_V35
  else if( SS_TYPE_DATE == cellType.Type )
  {
    TB_DATE Date;

    if( lpszData = (LPTSTR)tbGlobalLock(data.Data.hszData) )
    {
      lpValue->Status = SS_VALUE_STATUS_OK;
      lpValue->Type = SS_VALUE_TYPE_DOUBLE;
      DateStringToDMYEx(lpszData, &Date, &cellType.Spec.Date.Format, lpSS->lpBook->nTwoDigitYearMax);
      ftDateToDouble(Date.nYear, Date.nMonth, Date.nDay, &lpValue->Val.ValDouble);
    }
    else
      lpValue->Status = SS_VALUE_STATUS_EMPTY;
  }
  else if( SS_TYPE_TIME == cellType.Type )
  {
    TIME Time;

    if( lpszData = (LPTSTR)tbGlobalLock(data.Data.hszData) )
    {
      lpValue->Status = SS_VALUE_STATUS_OK;
      lpValue->Type = SS_VALUE_TYPE_DOUBLE;
      TimeStringToHMS(lpszData, &Time, &cellType.Spec.Time.Format);
      ftTimeToDouble(Time.nHour, Time.nMinute, Time.nSecond, &lpValue->Val.ValDouble);
    }
    else
      lpValue->Status = SS_VALUE_STATUS_EMPTY;
  }
#endif
  else
  {
    lpValue->Status = SS_VALUE_STATUS_OK;
    lpValue->Type = SS_VALUE_TYPE_STR;
    lpszData = (LPTSTR)tbGlobalLock(data.Data.hszData);
    if( lpValue->Val.hValStr = GlobalAlloc(GHND, (lstrlen(lpszData) + 1)
                                                 * sizeof(TCHAR)) )
    {
      lpszValue = (LPTSTR)GlobalLock(lpValue->Val.hValStr);
      lstrcpy(lpszValue, lpszData);
      GlobalUnlock(lpValue->Val.hValStr); 
    }
    tbGlobalUnlock(data.Data.hszData);
  }
  SS_UnlockColItem(lpSS, lCol);
  SS_UnlockRowItem(lpSS, lRow);
  tbGlobalUnlock(hSS);
  return TRUE;
}
#endif

//--------------------------------------------------------------------
//
//  The SS_CalcSetData() function is a call back function used by
//  the calc engine to assign data to a cell.
//

#ifndef SS_OLDCALC
BOOL CALLBACK SS_CalcSetData(HANDLE hSS, SS_COORD lCol, SS_COORD lRow,
                             LPCALC_VALUE lpValue)
{
  LPSPREADSHEET lpSS;
  SS_CELLTYPE cellType;
  SS_DATA data;
  LPTSTR lpszValue;

  lpSS = (LPSPREADSHEET)tbGlobalLock(hSS);
  SS_AdjustCellCoords(lpSS, &lCol, &lRow);
  if( lpValue->Status == SS_VALUE_STATUS_OK )
  {
    SS_RetrieveCellType(lpSS, &cellType, NULL, lCol, lRow);
    switch( cellType.Type )
    {
      case SS_TYPE_INTEGER:
        _fmemset(&data, 0, sizeof(SS_DATA));
        data.bDataType = SS_TYPE_INTEGER;
        if( SS_VALUE_TYPE_LONG == lpValue->Type )
        {
          data.Data.lValue = lpValue->Val.ValLong;
          SS_SetCellDataItem(lpSS, lCol, lRow, &data);
        }
        else if( SS_VALUE_TYPE_DOUBLE == lpValue->Type )
        {
          if( LONG_MIN <= lpValue->Val.ValDouble &&
              lpValue->Val.ValDouble <= LONG_MAX )
          {
            data.Data.lValue = (long)lpValue->Val.ValDouble;
            SS_SetCellDataItem(lpSS, lCol, lRow, &data);
          }
          else
            SS_SetCellData(lpSS, lCol, lRow, _T(""), 0);
        }
        else
        {
          SS_SetCellData(lpSS, lCol, lRow, _T(""), 0);
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
        _fmemset(&data, 0, sizeof(SS_DATA));
        data.bDataType = SS_TYPE_FLOAT;
        if( SS_VALUE_TYPE_LONG == lpValue->Type )
        {
          data.Data.dfValue = (double)lpValue->Val.ValLong;
          SS_SetCellDataItem(lpSS, lCol, lRow, &data);
        }
        else if( SS_VALUE_TYPE_DOUBLE == lpValue->Type )
        {
          data.Data.dfValue = lpValue->Val.ValDouble;
          SS_SetCellDataItem(lpSS, lCol, lRow, &data);
        }
        else
        {
          SS_SetCellData(lpSS, lCol, lRow, _T(""), 0);
        }
        break;
#ifdef SS_V35
// support for serial date and time values
      case SS_TYPE_DATE:
      case SS_TYPE_TIME:
        if( SS_VALUE_TYPE_STR != lpValue->Type )
        {
          short nYear, nMonth, nDay, nHour = 0, nMinute = 0, nSecond = 0;
          TCHAR buf[9] = {0};

          if( SS_VALUE_TYPE_LONG == lpValue->Type )
            ftDoubleToDate((double)lpValue->Val.ValLong, &nYear, &nMonth, &nDay);
          else if( SS_VALUE_TYPE_DOUBLE == lpValue->Type )
          {
            ftDoubleToDate(lpValue->Val.ValDouble, &nYear, &nMonth, &nDay);
            ftDoubleToTime(lpValue->Val.ValDouble, &nHour, &nMinute, &nSecond);
          }
          if( SS_TYPE_DATE == cellType.Type )
          {
#ifdef SPREAD_JPN
              _stprintf(buf, _T("%04d%02d%02d"), nYear, nMonth, nDay);
#else
              _stprintf(buf, _T("%02d%02d%04d"), nMonth, nDay, nYear);
#endif
              // RFW - 2/3/04 - 13598
              //SS_SetDataRange(lpSS, lCol, lRow, lCol, lRow, buf, TRUE, TRUE, FALSE);
              if (SS_FormatDataDate(lpSS, &cellType, buf, buf, TRUE))
                SS_SetCellData(lpSS, lCol, lRow, buf, (short)lstrlen(buf));
          }
          else
          {
              _stprintf(buf, _T("%02d%02d%02d"), nHour, nMinute, nSecond);
              // RFW - 2/3/04 - 13598
              //SS_SetDataRange(lpSS, lCol, lRow, lCol, lRow, buf, TRUE, TRUE, FALSE);
              if (SS_FormatDataTime(&cellType, buf, buf, TRUE))
                SS_SetCellData(lpSS, lCol, lRow, buf, (short)lstrlen(buf));
          }
          break;
        }
// fix for bug 9166 -scl
        break;
#endif
      default:
        if( SS_VALUE_TYPE_STR == lpValue->Type )
        {
          lpszValue = (LPTSTR)GlobalLock(lpValue->Val.hValStr);
          SS_SetCellData(lpSS, lCol, lRow, lpszValue, (short)lstrlen(lpszValue));
          GlobalUnlock(lpValue->Val.hValStr);
        }
        else if( SS_VALUE_TYPE_LONG == lpValue->Type )
        {
          TCHAR szBuffer[330];
          LongToString (lpValue->Val.ValLong, szBuffer);
          SS_SetCellData(lpSS, lCol, lRow, szBuffer, (short)lstrlen(szBuffer));
        }
        else if( SS_VALUE_TYPE_DOUBLE == lpValue->Type )
        {
          SS_SetCellData(lpSS, lCol, lRow, _T(""), 0);
        }
        break;
    }
  }
  else
    SS_SetCellData(lpSS, lCol, lRow, _T(""), 0);
  SS_InvalidateCell(lpSS, lCol, lRow);
  tbGlobalUnlock(hSS);
  return TRUE;
}
#endif

//--------------------------------------------------------------------
//
//  The SS_CalcGetDataCnt() function is a call back function used by
//  the calc engine to determine the last column and row containing
//  data.
//

#ifndef SS_OLDCALC
BOOL CALLBACK SS_CalcGetDataCnt(HANDLE hSS, LPLONG lplColCnt, LPLONG lplRowCnt)
{
  LPSPREADSHEET lpSS;
  lpSS = (LPSPREADSHEET)tbGlobalLock(hSS);
  if (lplColCnt)
    *lplColCnt = lpSS->Col.DataCnt;
  if (lplRowCnt)
    *lplRowCnt = lpSS->Row.DataCnt;
  SS_AdjustCellCoordsOut(lpSS, lplColCnt, lplRowCnt);
  if (lplColCnt)
    *lplColCnt = max(0, *lplColCnt - 1);
  if (lplRowCnt)
    *lplRowCnt = max(0, *lplRowCnt - 1);
  tbGlobalUnlock(hSS);
  return TRUE;
}
#endif

//--------------------------------------------------------------------
//
//  The SS_CalcGetMaxCols() function is a call back function used by
//  the calc engine to determine the max number of columns.
//

#ifndef SS_OLDCALC
long CALLBACK SS_CalcGetMaxCols(HANDLE hSS)
{
  LPSPREADSHEET lpSS;
  long lVal;
  lpSS = (LPSPREADSHEET)tbGlobalLock(hSS);
  lVal = lpSS->Col.Max;
  tbGlobalUnlock(hSS);
  return lVal;
}
#endif

//--------------------------------------------------------------------
//
//  The SS_CalcGetMaxRows() function is a call back function used by
//  the calc engine to determine the max number of rows.
//

#ifndef SS_OLDCALC
long CALLBACK SS_CalcGetMaxRows(HANDLE hSS)
{
  LPSPREADSHEET lpSS;
  long lVal;
  lpSS = (LPSPREADSHEET)tbGlobalLock(hSS);
  lVal = lpSS->Row.Max;
  tbGlobalUnlock(hSS);
  return lVal;
}
#endif

//--------------------------------------------------------------------
//
//  The SS_CalcGetRefHandle() function is a call back function used
//  by the calc engine to determine a handle to pass to a custom
//  function call back routine.
//

#ifndef SS_OLDCALC
LRESULT CALLBACK SS_CalcGetCustRef(HANDLE hSS)
{
  #ifdef SS_DLL
  LPSPREADSHEET lpSS;
  LRESULT lResult;
  lpSS = (LPSPREADSHEET)tbGlobalLock(hSS);
  lResult = (LRESULT)lpSS->lpBook->hWnd;
  tbGlobalUnlock(hSS);
  return lResult;
  #else
  return (LRESULT)hSS;
  #endif
}
#endif

void CALLBACK SS_CalcSetSending(HANDLE hSS, long lCol, long lRow)
{
LPSPREADSHEET lpSS = (LPSPREADSHEET)tbGlobalLock(hSS);
SS_AdjustCellCoords(lpSS, &lCol, &lRow);
lpSS->lpBook->CurCellSendingMsg.Col = lCol;
lpSS->lpBook->CurCellSendingMsg.Row = lRow;
tbGlobalUnlock(hSS);
}

//--------------------------------------------------------------------

#if defined(SS_DDE)
void CALLBACK SS_CalcDdeUpdate(long hSS, CALC_HANDLE hDde)
{
LPSPREADSHEET lpSS = (LPSPREADSHEET)tbGlobalLock(hSS);
DdeMarkDepend(hDde, &lpSS->CalcInfo.CellsToEval, &lpSS->CalcInfo.ColsToEval,
              &lpSS->CalcInfo.RowsToEval);
if (lpSS->lpBook->CalcAuto)
   CalcEvalNeededCells(&lpSS->lpBook->CalcInfo);
tbGlobalUnlock(hSS);
}
#endif

//--------------------------------------------------------------------

#if defined(SS_DDE)
void SS_CalcDdeRemove(long hSS, CALC_HANDLE hDde)
{
LPSPREADSHEET lpSS = (LPSPREADSHEET)tbGlobalLock(hSS);
DdeRemove(&lpSS->CalcInfo.DdeLinks, hDde);
tbGlobalUnlock(hSS);
}
#endif

//--------------------------------------------------------------------

#if defined(SS_DDE)
void SS_CalcLinkDispatchNotCreated(long hSS, LPCTSTR lpszServer, LPCTSTR lpszTopic, LPCTSTR lpszItem)
{
LPSPREADSHEET lpSS = (LPSPREADSHEET)tbGlobalLock(hSS);
SS_LINKMSG linkMsg;
linkMsg.lpszServer = lpszServer;
linkMsg.lpszTopic = lpszTopic;
linkMsg.lpszItem = lpszItem;
SS_SendMsg(lpSS->lpBook->hWnd, SSM_LINKDISPATCHNOTCREATED,
             GetDlgCtrlID(lpSS->lpBook->hWnd), (LPARAM)&linkMsg);
tbGlobalUnlock(hSS);
}
#endif

//--------------------------------------------------------------------

#if defined(SS_DDE)
void SS_CalcLinkError(long hSS, LPCTSTR lpszServer, LPCTSTR lpszTopic, LPCTSTR lpszItem, LPCTSTR lpszError, short nError)
{
LPSPREADSHEET lpSS = (LPSPREADSHEET)tbGlobalLock(hSS);
SS_LINKERROR linkError;
linkError.lpszServer = lpszServer;
linkError.lpszTopic = lpszTopic;
linkError.lpszItem = lpszItem;
linkError.lpszError = lpszError;
linkError.nError = nError;
SS_SendMsg(lpSS->lpBook->hWnd, SSM_LINKITEMNOTSUPPORTED,
           GetDlgCtrlID(lpSS->lpBook->hWnd), (LPARAM)&linkError);
tbGlobalUnlock(hSS);
}
#endif

//--------------------------------------------------------------------

#if defined(SS_DDE)
void SS_CalcLinkItemNotSupported(long hSS, LPCTSTR lpszServer, LPCTSTR lpszTopic, LPCTSTR lpszItem)
{
LPSPREADSHEET lpSS = (LPSPREADSHEET)tbGlobalLock(hSS);
SS_LINKMSG linkMsg;
linkMsg.lpszServer = lpszServer;
linkMsg.lpszTopic = lpszTopic;
linkMsg.lpszItem = lpszItem;
SS_SendMsg(lpSS->lpBook->hWnd, SSM_LINKITEMNOTSUPPORTED,
             GetDlgCtrlID(lpSS->lpBook->hWnd), (LPARAM)&linkMsg);
tbGlobalUnlock(hSS);
}
#endif

//--------------------------------------------------------------------

#if defined(SS_DDE)
void SS_CalcLinkItemSupported(long hSS, LPCTSTR lpszServer, LPCTSTR lpszTopic, LPCTSTR lpszItem)
{
LPSPREADSHEET lpSS = (LPSPREADSHEET)tbGlobalLock(hSS);
SS_LINKMSG linkMsg;
linkMsg.lpszServer = lpszServer;
linkMsg.lpszTopic = lpszTopic;
linkMsg.lpszItem = lpszItem;
SS_SendMsg(lpSS->lpBook->hWnd, SSM_LINKITEMSUPPORTED,
             GetDlgCtrlID(lpSS->lpBook->hWnd), (LPARAM)&linkMsg);
tbGlobalUnlock(hSS);
}
#endif

//--------------------------------------------------------------------

#if defined(SS_DDE)
void SS_CalcLinkNewData(long hSS, LPCTSTR lpszServer, LPCTSTR lpszTopic, LPCTSTR lpszItem)
{
LPSPREADSHEET lpSS = (LPSPREADSHEET)tbGlobalLock(hSS);
SS_LINKMSG linkMsg;
linkMsg.lpszServer = lpszServer;
linkMsg.lpszTopic = lpszTopic;
linkMsg.lpszItem = lpszItem;
SS_SendMsg(lpSS->lpBook->hWnd, SSM_LINKNEWDATA,
             GetDlgCtrlID(lpSS->lpBook->hWnd), (LPARAM)&linkMsg);
tbGlobalUnlock(hSS);
}
#endif

//--------------------------------------------------------------------

#if defined(SS_DDE)
void SS_CalcLinkNotify(long hSS, LPCTSTR lpszServer, LPCTSTR lpszTopic, LPCTSTR lpszItem)
{
LPSPREADSHEET lpSS = (LPSPREADSHEET)tbGlobalLock(hSS);
SS_LINKMSG linkMsg;
linkMsg.lpszServer = lpszServer;
linkMsg.lpszTopic = lpszTopic;
linkMsg.lpszItem = lpszItem;
SS_SendMsg(lpSS->lpBook->hWnd, SSM_LINKNOTIFY,
             GetDlgCtrlID(lpSS->lpBook->hWnd), (LPARAM)&linkMsg);
tbGlobalUnlock(hSS);
}
#endif

//--------------------------------------------------------------------

#if defined(SS_DDE)
void SS_CalcLinkOutOfMemory(long hSS, LPCTSTR lpszServer, LPCTSTR lpszTopic, LPCTSTR lpszItem)
{
LPSPREADSHEET lpSS = (LPSPREADSHEET)tbGlobalLock(hSS);
SS_LINKMSG linkMsg;
linkMsg.lpszServer = lpszServer;
linkMsg.lpszTopic = lpszTopic;
linkMsg.lpszItem = lpszItem;
SS_SendMsg(lpSS->lpBook->hWnd, SSM_LINKOUTOFMEMORY,
             GetDlgCtrlID(lpSS->lpBook->hWnd), (LPARAM)&linkMsg);
tbGlobalUnlock(hSS);
}
#endif

//--------------------------------------------------------------------

#if defined(SS_DDE)
void SS_CalcLinkPokeCompleted(long hSS, LPCTSTR lpszServer, LPCTSTR lpszTopic, LPCTSTR lpszItem, long lCode)
{
LPSPREADSHEET lpSS = (LPSPREADSHEET)tbGlobalLock(hSS);
SS_LINKPOKECOMPLETED linkMsg;
linkMsg.lpszServer = lpszServer;
linkMsg.lpszTopic = lpszTopic;
linkMsg.lpszItem = lpszItem;
linkMsg.lCode = lCode;
SS_SendMsg(lpSS->lpBook->hWnd, SSM_LINKPOKECOMPLETED,
             GetDlgCtrlID(lpSS->lpBook->hWnd), (LPARAM)&linkMsg);
tbGlobalUnlock(hSS);
}
#endif

//--------------------------------------------------------------------

#if defined(SS_DDE)
void SS_CalcLinkRequestCompleted(long hSS, LPCTSTR lpszServer, LPCTSTR lpszTopic, LPCTSTR lpszItem, long lCode)
{
LPSPREADSHEET lpSS = (LPSPREADSHEET)tbGlobalLock(hSS);
SS_LINKREQUESTCOMPLETED linkMsg;
linkMsg.lpszServer = lpszServer;
linkMsg.lpszTopic = lpszTopic;
linkMsg.lpszItem = lpszItem;
linkMsg.lCode = lCode;
SS_SendMsg(lpSS->lpBook->hWnd, SSM_LINKREQUESTCOMPLETED,
             GetDlgCtrlID(lpSS->lpBook->hWnd), (LPARAM)&linkMsg);
tbGlobalUnlock(hSS);
}
#endif

//--------------------------------------------------------------------

#if defined(SS_DDE)
void SS_CalcLinkServerDisconnected(long hSS, LPCTSTR lpszServer, LPCTSTR lpszTopic, LPCTSTR lpszItem)
{
LPSPREADSHEET lpSS = (LPSPREADSHEET)tbGlobalLock(hSS);
SS_LINKMSG linkMsg;
linkMsg.lpszServer = lpszServer;
linkMsg.lpszTopic = lpszTopic;
linkMsg.lpszItem = lpszItem;
SS_SendMsg(lpSS->lpBook->hWnd, SSM_LINKSERVERDISCONNECTED,
             GetDlgCtrlID(lpSS->lpBook->hWnd), (LPARAM)&linkMsg);
tbGlobalUnlock(hSS);
}
#endif

//--------------------------------------------------------------------

#if defined(SS_DDE)
void SS_CalcLinkUnableToConnectToServer(long hSS, LPCTSTR lpszServer, LPCTSTR lpszTopic, LPCTSTR lpszItem)
{
LPSPREADSHEET lpSS = (LPSPREADSHEET)tbGlobalLock(hSS);
SS_LINKMSG linkMsg;
linkMsg.lpszServer = lpszServer;
linkMsg.lpszTopic = lpszTopic;
linkMsg.lpszItem = lpszItem;
SS_SendMsg(lpSS->lpBook->hWnd, SSM_LINKUNABLETOCONNECTTOSERVER,
             GetDlgCtrlID(lpSS->lpBook->hWnd), (LPARAM)&linkMsg);
tbGlobalUnlock(hSS);
}
#endif

//--------------------------------------------------------------------
//
//  The SS_CreatePreHwnd() function handles any of the spreadsheet
//  creation process that can be done before a HWND is created.
//

TBGLOBALHANDLE SS_CreatePreHwnd(COLORREF crForegrnd, COLORREF crBackgrnd,
                                HFONT hFont)
{
TBGLOBALHANDLE hBook;
LPSS_BOOK lpBook;
HDC hDC;
SIZE Size;

static WORD FAR Bits[]  = {0xAA, 0x55, 0xAA, 0x55, 0xAA, 0x55, 0xAA, 0x55};
static WORD FAR Bits2[] = {0x55, 0xAA, 0x55, 0xAA, 0x55, 0xAA, 0x55, 0xAA};

if (dSpreadInstCnt == 0)
   {
	// RFW - 11/30/07 - 21744
#ifndef SS_NO_USE_SH
   MemRegisterTask();
#endif 
#ifndef SS_NO_USE_SH
   tbStringPool = MemPoolInit(MEM_POOL_DEFAULT | MEM_POOL_SERIALIZE);
   tbOmemPoolCol = MemPoolInitFS(sizeof(SS_COL), 10, MEM_POOL_DEFAULT | MEM_POOL_SERIALIZE);
   tbOmemPoolRow = MemPoolInitFS(sizeof(SS_ROW), 20, MEM_POOL_DEFAULT | MEM_POOL_SERIALIZE);
   tbOmemPoolCell = MemPoolInitFS(sizeof(SS_CELL), 100, MEM_POOL_DEFAULT | MEM_POOL_SERIALIZE);
#endif

   #if !defined(SS_NOCALC) && !defined(SS_OLDCALC)
   CalcMemPoolInit();
   #endif
#ifdef SS_V80
   CT_MemPoolInit();
#endif
	/*
	MemPoolSetPageSize(tbStringPool, 32768);
	MemPoolSetPageSize(tbOmemPoolRow, 32768);
	MemPoolSetPageSize(tbOmemPoolCell, 32768);
	*/
   }

dSpreadInstCnt++;

if (!(hBook = tbGlobalAlloc(GMEM_MOVEABLE | GMEM_ZEROINIT,
                            sizeof(SS_BOOK))))
   ;

lpBook = (LPSS_BOOK)tbGlobalLock(hBook);

#ifndef SS_NO_USE_SH
lpBook->OmemPoolCol = tbOmemPoolCol;
lpBook->OmemPoolRow = tbOmemPoolRow;
lpBook->OmemPoolCell = tbOmemPoolCell;
#endif

lpBook->dVScrollWidth = GetSystemMetrics(SM_CXVSCROLL);
lpBook->dHScrollHeight = GetSystemMetrics(SM_CYHSCROLL);
lpBook->fBorderVisible = 1;

/***********************************************
* Load the button to be used for the combo box
***********************************************/

lpBook->hBitmapComboBtn = LoadBitmap(NULL, (LPTSTR)OBM_COMBO);
//GetObject(lpBook->hBitmapComboBtn, sizeof(BITMAP), (LPVOID)&Bitmap);
lpBook->dComboButtonBitmapWidth = GetSystemMetrics(SM_CXVSCROLL);

lpBook->hCursorPointer = SS_CreateCursorPointer(hDynamicInst);
lpBook->hCursorResizeH = SS_CreateCursorResizeh(hDynamicInst);
lpBook->hCursorResizeV = SS_CreateCursorResizev(hDynamicInst);
lpBook->hBitmapLines = CreateBitmap(8, 8, 1, 1, (LPVOID)Bits);
lpBook->hBitmapLines2 = CreateBitmap(8, 8, 1, 1, (LPVOID)Bits2);
lpBook->hBrushLines = CreatePatternBrush(lpBook->hBitmapLines);
lpBook->hBrushLines2 = CreatePatternBrush(lpBook->hBitmapLines2);

hDC = fpGetDC(GetDesktopWindow());
lpBook->dxPixelsPerInch = GetDeviceCaps(hDC, LOGPIXELSX);
lpBook->dyPixelsPerInch = GetDeviceCaps(hDC, LOGPIXELSY);
GetTextExtentPoint(hDC, _T("1"), 1, &Size);
lpBook->StandardFontWidth = (short)Size.cx;
ReleaseDC(GetDesktopWindow(), hDC);

SS_InitSpreadPreHwnd(lpBook, crForegrnd, crBackgrnd);

tbGlobalUnlock(hBook);

return hBook;
}


//--------------------------------------------------------------------
//
//  The SS_CreateWithHwnd() function handles any of the spreadsheet
//  creatation process than must be done after a HWND is created.
//

void SS_CreateWithHwnd(HWND hWnd, TBGLOBALHANDLE hBook)
{
LPSS_BOOK lpBook;
long      lStyle;

BOOL      fIsHorizontal = FALSE;

fpInitTextOut(hWnd);

lpBook = (LPSS_BOOK)tbGlobalLock(hBook);

#ifdef SS_NOSUPERCLASS
{
SetProp(hWnd, "Xtra1", LOWORD((long)lpSS));
SetProp(hWnd, "Xtra2", HIWORD((long)lpSS));
}
#else
#if defined(_WIN64) || defined(_IA64)
SetWindowLongPtr(hWnd, 0, (LONG_PTR)hBook);
#else
SetWindowLong(hWnd, 0, (long)hBook);
#endif
#endif

lpBook->hWnd = hWnd;
lStyle = GetWindowLong(hWnd, GWL_STYLE);

#ifndef SS_NOCHILDSCROLL
#ifdef SS_V80
if (lpBook->wAppearanceStyle && lpBook->wScrollBarStyle != 1)
{
   long lHStyle = SBRS_UPDN |SBRS_THUMB | SBS_HORZ;
   fIsHorizontal = TRUE;
   lpBook->hWndHScroll = CreateWindow(SSClassNames[dClassNameIndex].TBScrlBar,
                                    NULL, WS_CHILD | WS_VISIBLE | 
                                    lHStyle, 0, 0, 0, 0, hWnd, (HMENU)SBS_HORZ,
                                    hDynamicInst, (LPVOID)(LPBOOL)&fIsHorizontal);
   SendMessage(lpBook->hWndHScroll, SBR_SETSTYLE, (WPARAM)lpBook->wScrollBarStyle == 0 ? lpBook->wAppearanceStyle : lpBook->wScrollBarStyle-1, 0L);
   SendMessage(lpBook->hWndHScroll, SBM_SETRANGE, 1, SSGetMaxCols(hWnd));
   SendMessage(lpBook->hWndHScroll, SBR_TRACKMOUSE, lpBook->fHScrollBarTrack, 0);
   if (lpBook->hWndHScroll)
		ScrlBarEnhancedScrollBarColors(lpBook->hWndHScroll, lpBook->scrollTrackColor, lpBook->scrollArrowColor, lpBook->scrollUpperNormalStartColor, lpBook->scrollUpperNormalEndColor,
										   lpBook->scrollLowerNormalStartColor, lpBook->scrollLowerNormalEndColor, lpBook->scrollUpperPushedStartColor, lpBook->scrollUpperPushedEndColor,
										   lpBook->scrollLowerPushedStartColor, lpBook->scrollLowerPushedEndColor, lpBook->scrollUpperHoverStartColor, lpBook->scrollUpperHoverEndColor,
										   lpBook->scrollLowerHoverStartColor, lpBook->scrollLowerHoverEndColor, lpBook->scrollHoverBorderColor, lpBook->scrollBorderColor);
}
else
#endif
lpBook->hWndHScroll = CreateWindow(_T("ScrollBar"), NULL, WS_CHILD |
                                 /*
                                 WS_CLIPSIBLINGS | WS_VISIBLE |
                                 */
                                 WS_VISIBLE |
                                 SBS_HORZ, 0, 0, 0, 0,
                                 hWnd, (HMENU)SBS_HORZ, hDynamicInst, NULL);

#ifdef SS_V80
if( lpBook->fUseSpecialVScroll || (lpBook->wAppearanceStyle && lpBook->wScrollBarStyle != 1))
#else
if( lpBook->fUseSpecialVScroll)
#endif
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
#ifdef SS_V80
   else
	lStyle = SBRS_UPDN | SBRS_THUMB | SBS_VERT;
#endif
   fIsHorizontal = FALSE;
   lpBook->hWndVScroll = CreateWindow(SSClassNames[dClassNameIndex].TBScrlBar,
                                    NULL, WS_CHILD | WS_VISIBLE |
                                    lStyle, 0, 0, 0, 0, hWnd, (HMENU)SBS_VERT,
                                    hDynamicInst, (LPVOID)(LPBOOL)&fIsHorizontal);
#ifdef SS_V80
   SendMessage(lpBook->hWndVScroll, SBR_SETSTYLE, (WPARAM)lpBook->wScrollBarStyle == 0 ? lpBook->wAppearanceStyle : lpBook->wScrollBarStyle-1, 0L);
   SendMessage(lpBook->hWndVScroll, SBM_SETRANGE, 1, SSGetMaxRows(hWnd));
   SendMessage(lpBook->hWndVScroll, SBR_TRACKMOUSE, lpBook->fVScrollBarTrack, 0);
   if (lpBook->hWndVScroll)
		ScrlBarEnhancedScrollBarColors(lpBook->hWndVScroll, lpBook->scrollTrackColor, lpBook->scrollArrowColor, lpBook->scrollUpperNormalStartColor, lpBook->scrollUpperNormalEndColor,
										   lpBook->scrollLowerNormalStartColor, lpBook->scrollLowerNormalEndColor, lpBook->scrollUpperPushedStartColor, lpBook->scrollUpperPushedEndColor,
										   lpBook->scrollLowerPushedStartColor, lpBook->scrollLowerPushedEndColor, lpBook->scrollUpperHoverStartColor, lpBook->scrollUpperHoverEndColor,
										   lpBook->scrollLowerHoverStartColor, lpBook->scrollLowerHoverEndColor, lpBook->scrollHoverBorderColor, lpBook->scrollBorderColor);
#endif
   }
else
   {
   lpBook->hWndVScroll = CreateWindow(_T("ScrollBar"), NULL, WS_CHILD |
                                    /*
                                    WS_CLIPSIBLINGS | WS_VISIBLE |
                                    */
                                    WS_VISIBLE |
                                    SBS_VERT, 0, 0, 0, 0,
                                    hWnd, (HMENU)SBS_VERT, hDynamicInst, NULL);
                                    
   }

#ifdef WIN32
{
SCROLLINFO si;
WORD       wLoWordVersion = LOWORD(GetVersion());

if (HIBYTE(wLoWordVersion) == 95 ||
    (LOBYTE(wLoWordVersion) == 4 && HIBYTE(wLoWordVersion) == 0))
   {
   si.cbSize = sizeof(SCROLLINFO);
   si.fMask = SIF_PAGE;
   si.nPage = 1;
   
   SendMessage(lpBook->hWndHScroll, SBM_SETSCROLLINFO, 0, (LPARAM)(LPVOID)&si);

   if(!lpBook->fUseSpecialVScroll )
      SendMessage(lpBook->hWndVScroll, SBM_SETSCROLLINFO, 0, (LPARAM)(LPVOID)&si);
   }
}
#endif

#endif

lpBook->Redraw = FALSE;
SS_InitSpreadWithHwnd(hWnd, lpBook);

{
LPSPREADSHEET lpSS;

lpSS = SS_BookLockSheetIndex(lpBook, 0);

if (lStyle & SSS_RESTRICTROWS)
   lpSS->RestrictRows = TRUE;

if (lStyle & SSS_RESTRICTCOLS)
   lpSS->RestrictCols = TRUE;

if (lStyle & SSS_LOCKALL)
   lpSS->DocumentLocked = TRUE;

if (lStyle & SSS_NOGRIDLINES)
   lpSS->wGridType &= ~(SS_GRID_HORIZONTAL | SS_GRID_VERTICAL);

if (lStyle & SSS_HIDECOLHEADERS)
   SS_SetBool(lpBook, lpSS, SSB_SHOWCOLHEADERS, FALSE);

if (lStyle & SSS_HIDEROWHEADERS)
   SS_SetBool(lpBook, lpSS, SSB_SHOWROWHEADERS, FALSE);

SS_BookUnlockSheetIndex(lpBook, 0);
}

#ifdef SS_V70
lpBook->hTabStrip = SSTab_Create();
SSTab_Attach(lpBook->hTabStrip, lpBook->hWnd);
#endif // SS_V70

SS_BookSetRedraw(lpBook, TRUE);

tbGlobalUnlock(hBook);
}

//--------------------------------------------------------------------
//
//  The SS_OnCreate() function handles WM_CREATE messages.
//

BOOL SS_OnCreate(HWND hWnd, LPCREATESTRUCT lpCreateStruct)
{
TBGLOBALHANDLE hBook;

#ifdef SS_OCX
hBook = (TBGLOBALHANDLE)lpCreateStruct->lpCreateParams;
#else
{
HDC hDC;
HFONT hFont, hFontOld;

hDC = fpGetDC(hWnd);

hFontOld = (HFONT)SelectObject(hDC, GetStockObject(SYSTEM_FIXED_FONT));

#ifdef SPREAD_JPN
{
LOGFONT LogFont;
_fmemset(&LogFont, '\0', sizeof(LogFont));
LogFont.lfHeight = -PT_TO_PIXELS(9);
LogFont.lfWeight = FW_NORMAL;
LogFont.lfCharSet = SHIFTJIS_CHARSET;
//LogFont.lfCharSet = 128;
lstrcpy(LogFont.lfFaceName, _T("ÇlÇr ÇoÉSÉVÉbÉN"));
//	lstrcpy(LogFont.lfFaceName, _T("MS PGothic"));
hFont = (HFONT)CreateFontIndirect(&LogFont);
}
#else
#ifdef SS_V70
{
LOGFONT LogFont;
_fmemset(&LogFont, '\0', sizeof(LogFont));
LogFont.lfHeight = (int)PT_TO_PIXELS(8);
LogFont.lfWeight = FW_NORMAL;
LogFont.lfCharSet = DEFAULT_CHARSET;
lstrcpy(LogFont.lfFaceName, _T("MS Sans Serif"));
hFont = CreateFontIndirect(&LogFont);
}
#else
hFont = hFontOld;
#endif // SS_V70
#endif 

hBook = SS_CreatePreHwnd(GetTextColor(hDC), GetBkColor(hDC), hFont);
SelectObject(hDC, hFontOld);
if (hFont != hFontOld)
	DeleteObject(hFont);
ReleaseDC(hWnd, hDC);
}
#endif
SS_CreateWithHwnd(hWnd, hBook);

#if SS_V80
{
   LPSS_BOOK lpBook = SS_BookLock(hWnd);
   if( lpCreateStruct->style & SSS_APPEARANCESTYLE_CLASSIC )
      SS_SetAppearanceStyle(lpBook, SS_APPEARANCESTYLE_CLASSIC);
   else if( lpCreateStruct->style & SSS_APPEARANCESTYLE_VISUALSTYLES )
      SS_SetAppearanceStyle(lpBook, SS_APPEARANCESTYLE_VISUALSTYLES);
   SS_BookUnlock(hWnd);
}
#endif
return TRUE;
}

//--------------------------------------------------------------------
//
//  The SS_DestroyWithHwnd() function handles any of the spreadsheet
//  destruction that must be done before the HWND is destroyed.
//

void SS_DestroyWithHwnd(HWND hWnd)
{
LPSPREADSHEET lpSS;

if (lpSS = SS_SheetLockActive(hWnd))
   {
   SS_CellEditModeOff(lpSS, (HWND)-1);
   if (GetFocus() && GetParent(GetFocus()) == hWnd)
      SetFocus(0);

   SS_SheetUnlockActive(hWnd);
   }

#ifdef SS_V70
{
LPSS_BOOK lpBook = SS_BookLock(hWnd);
SSTab_Detach(lpBook->hTabStrip);
SSTab_Release(lpBook->hTabStrip);
lpBook->hTabStrip = 0;
SS_BookUnlock(hWnd);

}
#endif // SS_V70

#ifdef SS_V30
fpTextTipTerminate(hWnd);
#endif
SS_FreeWithHwnd(hWnd);
fpKillTextOut(hWnd);

if (hBrushFocusRect)
   {
   DeleteObject(hBrushFocusRect);
   hBrushFocusRect = 0;
   }
}

//--------------------------------------------------------------------
//
//  The SS_DestroyPostHwnd() function handles any of the spreadsheet
//  destruction that can be done after the HWND is destroyed.
//

void SS_DestroyPostHwnd(TBGLOBALHANDLE hBook)
{
LPSS_BOOK lpBook;

if (lpBook = (LPSS_BOOK)tbGlobalLock(hBook))
   {
#ifndef SS_NOCT_CHECK
   if (lpBook->fCheckBoxDefBitmapsLoaded)
      CheckBoxUnloadDefBitmaps();
#endif
   tbGlobalUnlock(hBook);
   }

SS_FreePostHwnd(hBook);

dSpreadInstCnt--;

// Sp25_007 [1-1]
// It Delete Font Table Only When Last Instance Destory
// If User Uses Lots Of Fonts
// The GDI Will Not Recover Until User Close All Instance Of Spread :(
// But I Can't Change It Because The Program Is Designed Like This :(
// Some GDI Problems Maybe Caused By This Reason
// Look At Above Note And Don't Waste More Time On It :)
// -- HaHa 1998.4.24

if (dSpreadInstCnt == 0)
   {
   SS_ColorTableDelete();
   SS_FontTableDelete();
#if defined(SS_V30) && defined(WIN32)
   SS_ExcelTableDelete();
#endif

#ifdef SS_V40
   if (PaintBitmap.hPaintBitmap)
		{
		DeleteObject(PaintBitmap.hPaintBitmap);
		PaintBitmap.hPaintBitmap = 0;
		}

	if (PaintBitmap.csInit)
		{
	   DeleteCriticalSection(&PaintBitmap.cs);
		PaintBitmap.csInit = FALSE;
		}

#endif // SS_V40

   SS_SortDestroy(); // RFW - 4/8/08 - 21963

#ifndef SS_NO_USE_SH
   MemPoolFree(tbStringPool);
   MemPoolFree(tbOmemPoolCol);
   MemPoolFree(tbOmemPoolRow);
   MemPoolFree(tbOmemPoolCell);
#endif
   #if !defined(SS_NOCALC) && !defined(SS_OLDCALC)
   CalcMemPoolFree();
   #endif
#if SS_V80
   CT_MemPoolFree();
   SS_UnloadUxTheme();
   SS_UnloadMsImg();
#endif

	// RFW - 11/30/07 - 21744
#ifndef SS_NO_USE_SH
   MemUnregisterTask();
#endif 
   }
}

//--------------------------------------------------------------------
//
//  The SS_OnDestroy() function handles WM_DESTROY messages.
//

void SS_OnDestroy(HWND hWnd)
{
TBGLOBALHANDLE hBook;

#if SS_NOSUPERCLASS
hBook = (TBGLOBALHANDLE)MAKELONG(GetProp(hWnd, "Xtra1"),
                                   GetProp(hWnd, "Xtra2"));
#else
#if defined(_WIN64) || defined(_IA64)
hBook = (TBGLOBALHANDLE)GetWindowLongPtr(hWnd, 0);
#else
hBook = (TBGLOBALHANDLE)GetWindowLong(hWnd, 0);
#endif
#endif

if (hBook)
	{
	SS_DestroyWithHwnd(hWnd);
#ifndef SS_OCX
	SS_DestroyPostHwnd(hBook);
#endif

	//#ifdef SPREAD_JPN
	// Sp25_010 [1-1]
	// If User Close A Program Contain Spread
	// While It Is The Last Process And IME Window Is Open 
	// Computer Will Die For Unknown Reason
	// I Think It Is The Problem Of IME Of Windows 95 :(
	// To Solve The Problem Close The IME Window When Destroy Spread Window :)
	// -- HaHa 1998.4.28
	#ifdef WIN32
		SendMessage(hWnd,WM_IME_NOTIFY,IMN_CLOSESTATUSWINDOW,0);
	#endif
	//#endif
	}
}


// RFW - 4/13/09 - 24730
void SS_SetCursorCurrent(LPSS_BOOK lpBook, HCURSOR hCursor, HCURSOR hCursorDef)
{
lpBook->hCursorCurrent = hCursor;
lpBook->fCursorCurrentDef = (hCursor == hCursorDef);
}


//--------------------------------------------------------------------

LRESULT CALLBACK SpreadSheetProc(HWND hWnd, UINT Msg, WPARAM wParam, LPARAM lParam)
{
LPSS_BOOK     lpBook;
LPSPREADSHEET lpSS;
LRESULT       lRet = 0;

lpBook = SS_BookLock(hWnd);
lpSS = SS_BookLockActiveSheet(lpBook);

lRet = SS_Proc(lpBook, lpSS, hWnd, Msg, wParam, lParam);

#if defined(_WIN64) || defined(_IA64)
if (IsWindow(hWnd)&& GetWindowLongPtr(hWnd,0) && lpBook)	// make sure lpBOok not deallocted in SS_Proc
#else
if (IsWindow(hWnd)&& GetWindowLong(hWnd,0) && lpBook)	// make sure lpBOok not deallocted in SS_Proc
#endif
{
    SS_BookUnlockActiveSheet(lpBook);
    SS_BookUnlock(hWnd);
}
return (lRet);
}


LRESULT SS_Proc(LPSS_BOOK lpBook, LPSPREADSHEET lpSS, HWND hWnd, UINT Msg, WPARAM wParam, LPARAM lParam)
{
// fix for #8963 -scl
//if (fpMouseWheel(hWnd, Msg, wParam, lParam, (HWND)(lpSS ? lpBook->hWndVScroll : 0), NULL))
#define WM_MOUSEWHEEL                   0x020A // copied from WINUSER.H
if (lpSS && lpBook->EditModeOn && WM_MOUSEWHEEL == Msg)
   {
   POINT pt;
   HWND hChild;
   TCHAR szClass[20] = {0};

   GetCursorPos(&pt);
   hChild = WindowFromPoint(pt);
   if (hChild != hWnd)
      {
      GetClassName(hChild, szClass, 20);
      if (!_tcscmp(szClass, SSClassNames[dClassNameIndex].SSListBox))
         // scroll the listbox
         fpMouseWheel(hChild, Msg, wParam, lParam, GetWindow(hChild, GW_CHILD), NULL);
      else if (!_tcscmp(szClass, SSClassNames[dClassNameIndex].SSComboBox))
         {  // change the selection in the combobox
            LRESULT lCurSel = SS_ComboBoxSendMessage(lpSS, lpSS->Col.CurAt, lpSS->Row.CurAt, CB_GETCURSEL, 0, 0);
            LRESULT lCount = SS_ComboBoxSendMessage(lpSS, lpSS->Col.CurAt, lpSS->Row.CurAt, CB_GETCOUNT, 0, 0);
            short zDelta = (short)HIWORD(wParam);
            if (zDelta < 0 && lCurSel < lCount - 1)
            {
               SendMessage(hChild, CB_SETCURSEL, ++lCurSel, 0);
               SendMessage(hChild, WM_COMMAND, MAKELONG(0, LBN_SELCHANGE), 0);
            }
            else if (zDelta > 0 && lCurSel > 0)
            {
               SendMessage(hChild, CB_SETCURSEL, --lCurSel, 0);
               SendMessage(hChild, WM_COMMAND, MAKELONG(0, LBN_SELCHANGE), 0);
            }
         }
      }

	// RFW - 8/16/04 - I changed it so it now works anytime edit mode is on.
	// RFW - 8/3/04 - Mouse wheel with EditModePermanent
	//else if (lpBook->fEditModePermanent && fpMouseWheel(hWnd, Msg, wParam, lParam, lpBook->hWndVScroll, NULL))
	else if (fpMouseWheel(hWnd, Msg, wParam, lParam, lpBook->hWndVScroll, NULL))
    // 99915276 -scl
    //return (TRUE);
		return (FALSE);

   return 0;
   }
else if (lpBook && !lpBook->EditModeOn && fpMouseWheel(hWnd, Msg, wParam, lParam, lpBook->hWndVScroll, NULL))
   // 99915276 -scl
   //return (TRUE);
   return (FALSE);

switch (Msg)
   {
   case WM_NCCREATE:
      {
      LPCREATESTRUCT lpcs = (LPCREATESTRUCT)lParam;
      lpcs->style &= ~(WS_VSCROLL | WS_HSCROLL);
      SetWindowLong(hWnd, GWL_STYLE, lpcs->style);
#ifdef SS_OCX
      HANDLE_WM_NCCREATE(hWnd, wParam, lParam, SS_OnCreate);
#endif // SS_OCX
      }
      break;

#ifndef SS_OCX
	// RFW - 10/31/02 - 11007
   case WM_CREATE:
      return HANDLE_WM_CREATE(hWnd, wParam, lParam, SS_OnCreate); 
#endif // SS_OCX

   case WM_GETDLGCODE:
		if (lpBook)
			{
			long lRet = DLGC_WANTARROWS | DLGC_WANTCHARS;

			if (lpBook->fProcessTab)
				lRet |= DLGC_WANTTAB;

			if (lpBook->wEnterAction != SS_ENTERACTION_NONE)
				lRet |= DLGC_WANTALLKEYS;

			return (lRet);
			}

		break;

#ifdef WIN32
   case WM_CTLCOLOREDIT:
      return HANDLE_WM_CTLCOLOREDIT(hWnd, wParam, lParam, SS_OnCtlColor);      
   case WM_CTLCOLORLISTBOX:
      return HANDLE_WM_CTLCOLORLISTBOX(hWnd, wParam, lParam, SS_OnCtlColor);      
   case WM_CTLCOLORBTN:
      return HANDLE_WM_CTLCOLORBTN(hWnd, wParam, lParam, SS_OnCtlColor);      
   case WM_CTLCOLORSCROLLBAR:
      return HANDLE_WM_CTLCOLORSCROLLBAR(hWnd, wParam, lParam, SS_OnCtlColor);      
   case WM_CTLCOLORSTATIC:
      return HANDLE_WM_CTLCOLORSTATIC(hWnd, wParam, lParam, SS_OnCtlColor);      
#else
   case WM_CTLCOLOR:
      return HANDLE_WM_CTLCOLOR(hWnd, wParam, lParam, SS_OnCtlColor);
#endif

	case WM_DISPLAYCHANGE:
	case WM_SYSCOLORCHANGE:
		SS_ColorTableDeleteBrushes();
		break;

   case WM_SIZE:
		if (lpSS)
			{
			UINT fBorderVis = ((GetWindowLong(hWnd, GWL_STYLE) & WS_BORDER) ? 1 : 0);
			if (fBorderVis != lpBook->fBorderVisible)
				{
				lpBook->fBorderVisible = fBorderVis;
				if (lpBook->fBorderVisible)
					{
					lpBook->dVScrollWidth--;
					lpBook->dHScrollHeight--;
					}
				else
					{
					lpBook->dVScrollWidth++;
					lpBook->dHScrollHeight++;
					}
				}


#ifdef SS_V70
			SS_EmptyTabStripRect(lpBook);
			SS_InvalidateTabStrip(lpBook);
#endif // SS_V70

			// RFW - 10/3/08 - 22891
			lpSS->Row.LRAllVis = -1;
			lpSS->Col.LRAllVis = -1;

			SS_GetClientRect(lpBook, &lpBook->ClientRect);
#if SS_V80
         lpBook->fWmSizeMoveScrollBars = TRUE;
#endif
			SS_AutoSize(lpBook, TRUE);
#if SS_V80
         lpBook->fWmSizeMoveScrollBars = FALSE;
#endif
			if (!lpBook->fInAutoSize)
				{
				RECT Rect;

				 if ((short)LOWORD(lParam) > lpBook->dClientWidth)
					 {
					 SetRect(&Rect, lpBook->dClientWidth, 0, LOWORD(lParam),
								HIWORD(lParam));
					 SS_InvalidateRect(lpBook, &Rect, TRUE);
					 }

				 if ((short)HIWORD(lParam) > lpBook->dClientHeight)
					 {
					 SetRect(&Rect, 0, lpBook->dClientHeight, LOWORD(lParam),
								HIWORD(lParam));
					 SS_InvalidateRect(lpBook, &Rect, TRUE);
					 }

	#ifndef SS_NOCHILDSCROLL
			/*
			if (lpBook->fVScrollVisible)
				{
				SetRect(&Rect, LOWORD(lParam) - lpBook->dVScrollWidth, 0,
						  LOWORD(lParam), HIWORD(lParam));
				SS_InvalidateRect(lpBook, &Rect, TRUE);
				}

			if (lpBook->fHScrollVisible)
				{
				SetRect(&Rect, 0, HIWORD(lParam) - lpBook->dHScrollHeight,
						  LOWORD(lParam), HIWORD(lParam));
				SS_InvalidateRect(lpBook, &Rect, TRUE);
				}
			*/
				 if (lpBook->fVScrollVisible && lpBook->fHScrollVisible)
					 {
					 SetRect(&Rect, LOWORD(lParam) - lpBook->dVScrollWidth,
								HIWORD(lParam) - lpBook->dHScrollHeight,
								LOWORD(lParam), HIWORD(lParam));
					 SS_InvalidateRect(lpBook, &Rect, TRUE);
					 SetRect(&Rect, lpBook->dClientWidth - lpBook->dVScrollWidth,
								lpBook->dClientHeight - lpBook->dHScrollHeight,
								lpBook->dClientWidth, lpBook->dClientHeight);
					 SS_InvalidateRect(lpBook, &Rect, TRUE);
					 }
	#endif

				 lpBook->dClientWidth = LOWORD(lParam);
				 lpBook->dClientHeight = HIWORD(lParam);

	#ifdef SS_UTP
				 SS_CalcHScrollArrowArea(hWnd, lpSS);
				 SS_CalcVScrollArrowArea(hWnd, lpSS);
	#endif
		 }
	//Added by FMH 1996.07.05. ------------------------>>
	//      for edit window may be drawn on the scroll bar if adjusting
	//      the size of spread while EditMode is ON.
	//#ifdef SPREAD_JPN
			if (SS_GetEditMode(lpBook) && (lpSS->Row.CurAt > SS_GetBottomVisCell(lpSS, lpSS->Row.UL) ||
												  lpSS->Col.CurAt > SS_GetRightVisCell(lpSS, lpSS->Col.UL)))
				 SS_SetEditMode(lpBook, FALSE);
	//#endif
	//------------------------<<
			}

      break;

#if defined(SS_V30)
   case WM_NCCALCSIZE:
   case WM_NCPAINT:
		if (lpSS)
         {
         FPNCINFO fpNCInfo;

         if (lpBook->bAppearance == SS_APPEARANCE_DEFAULT)
            {
            if (Msg == WM_NCPAINT)
               if (!IsWindowVisible(hWnd))
                  return (0);

            break;
            }

         _fmemset(&fpNCInfo, '\0', sizeof(fpNCInfo));

#ifndef SS_DLL
         if (GetWindowLong(hWnd, GWL_STYLE) & WS_BORDER)
#endif
            {
            fpNCInfo.colorBk = RGBCOLOR_BLACK;

            if (lpBook->bAppearance == SS_APPEARANCE_FLAT ||
                lpBook->bAppearance == SS_APPEARANCE_3DWITHBORDER)
               {
               fpNCInfo.lStyle |= FP_NCBORDER;
               fpNCInfo.bPenStyleOnFocus = PS_SOLID;
               fpNCInfo.bPenStyleNoFocus = PS_SOLID;
               fpNCInfo.colorBorder = GetSysColor(COLOR_WINDOWFRAME);
               fpNCInfo.bBorderSize = 1;
               }

            if (lpBook->bAppearance == SS_APPEARANCE_3D ||
                lpBook->bAppearance == SS_APPEARANCE_3DWITHBORDER)
               {
					/*
               fpNCInfo.lStyle |= (FP_NC3DINRAISE | FP_NC3DOUTRAISE);
               fpNCInfo.bOutShadowWidth = 1;
               fpNCInfo.color3DOutShadow = GetSysColor(COLOR_BTNSHADOW);
               fpNCInfo.color3DOutHiLite = RGBCOLOR_WHITE;
               fpNCInfo.bInShadowWidth = 1;
               fpNCInfo.color3DInShadow = GetSysColor(COLOR_WINDOWFRAME);
               fpNCInfo.color3DInHiLite = GetSysColor(COLOR_BTNFACE);
					*/
               fpNCInfo.lStyle |= (FP_NC3DINLOWER | FP_NC3DOUTLOWER);
               fpNCInfo.bOutShadowWidth = 1;
               fpNCInfo.color3DOutShadow = GetSysColor(COLOR_3DSHADOW);
               fpNCInfo.color3DOutHiLite = GetSysColor(COLOR_3DHILIGHT);
               fpNCInfo.bInShadowWidth = 1;
               fpNCInfo.color3DInShadow = GetSysColor(COLOR_3DDKSHADOW);
               fpNCInfo.color3DInHiLite = GetSysColor(COLOR_3DLIGHT);
               }
            }

         switch (Msg)
            {
            case WM_NCCALCSIZE:
               fpWMNCCalcSize(hWnd, &fpNCInfo, (BOOL)wParam, (LPNCCALCSIZE_PARAMS)lParam);
               return (0);

            case WM_NCPAINT:
               if (IsWindowVisible(hWnd))
                  fpWMNCPaint(hWnd, &fpNCInfo, (short)0xFFFF, 0);

               return (0);
            }
         }

      break;
#endif

#ifdef SS_V40
   case WM_CAPTURECHANGED:
		if (lpSS)
			lpBook->dwNextScrollTickCount = 0L;
		break;
#endif // SS_V40

   case WM_NCHITTEST:
		if (lpSS)
			{
			RECT Rect;
			RECT RectClient;
			int  MouseX;
			int  MouseY;
			POINT Point = {0, 0};

			SS_SetCursorCurrent(lpBook, SS_CURSOR_DEFAULT, SS_CURSOR_DEFAULT);

			GetClientRect(hWnd, &Rect);
			SS_GetClientRect(lpBook, &RectClient);
			ClientToScreen(hWnd, &Point);
			OffsetRect(&Rect, Point.x, Point.y);

			/* RFW - 5/31/07 - 20536
			MouseX = LOWORD(lParam);
			MouseY = HIWORD(lParam);
			*/
			MouseX = (short)LOWORD(lParam);
			MouseY = (short)HIWORD(lParam);

			if (MouseY >= Rect.top && MouseY < Rect.bottom &&
				 MouseX >= Rect.left && MouseX < Rect.left + RectClient.right)
				{
				POINT Point;

				Point.x = MouseX;
				Point.y = MouseY;

				ScreenToClient(hWnd, &Point);

				MouseX = Point.x;
				MouseY = Point.y;

				if (SS_IsMouseInHorizResize(lpSS, &Rect, &RectClient,
													 MouseX, MouseY, NULL) != -1)
					SS_SetCursorCurrent(lpBook, lpBook->CursorRowResize.hCursor, SS_DEFCURSOR_ROWRESIZE);

				else if (SS_IsMouseInVertResize(lpSS, &Rect, &RectClient,
														  MouseX, MouseY, NULL) != -1)
					SS_SetCursorCurrent(lpBook, lpBook->CursorColResize.hCursor, SS_DEFCURSOR_COLRESIZE);
#ifdef SS_V70
				else if (SS_IsPointInTabStripRect(lpBook, MouseX, MouseY))
					SS_SetCursorCurrent(lpBook, SS_CURSOR_ARROW, SS_CURSOR_ARROW);

				else if (SS_IsPointInTabSplitBoxRect(lpBook, MouseX, MouseY))
					SS_SetCursorCurrent(lpBook, lpBook->CursorColResize.hCursor, SS_DEFCURSOR_COLRESIZE);

				else if (SS_IsMouseInColMove(lpSS, MouseX, MouseY) ||
				         SS_IsMouseInRowMove(lpSS, MouseX, MouseY))
					SS_SetCursorCurrent(lpBook, SS_CURSOR_ARROW, SS_CURSOR_ARROW);
#endif // SS_V70

#ifndef SS_NODRAGDROP
				else if (SS_IsMouseInDragDrop(hWnd, lpSS, &RectClient, MouseX,
														MouseY, NULL, NULL))
					SS_SetCursorCurrent(lpBook, lpBook->CursorDragDropArea.hCursor, SS_DEFCURSOR_DRAGDROPAREA);
#endif

				else if (SS_IsMouseInGrayArea(lpSS, &RectClient, MouseX, MouseY))
					{
					SS_SetCursorCurrent(lpBook, lpBook->CursorGrayArea.hCursor, SS_DEFCURSOR_GRAYAREA);
#ifdef SS_MAKETRANSPARENT
					return (HTTRANSPARENT);
#endif
					}
#ifndef SS_V80 // removed to make custom cursors work in v8 -scl
				else if (lpSS->wOpMode == SS_OPMODE_READONLY ||
							lpSS->wOpMode == SS_OPMODE_SINGLESEL ||
							lpSS->wOpMode == SS_OPMODE_MULTISEL ||
							lpSS->wOpMode == SS_OPMODE_EXTSEL)
					SS_SetCursorCurrent(lpBook, SS_CURSOR_DEFAULT, SS_CURSOR_DEFAULT);
#endif
				else
					{
					SS_COORD Row;
					SS_COORD Col;
					BOOL     fMakeTransparent;

					SS_GetCellFromPixel(lpSS, &Col, &Row, NULL, NULL, MouseX, MouseY);
					SS_GetActualCell(lpSS, Col, Row, &Col, &Row, NULL);

					fMakeTransparent = TRUE;

					if (Col < lpSS->Col.HeaderCnt)
						SS_SetCursorCurrent(lpBook, lpBook->CursorRowHeader.hCursor, SS_DEFCURSOR_ROWHEADER);

					else if (Row < lpSS->Row.HeaderCnt)
						SS_SetCursorCurrent(lpBook, lpBook->CursorColHeader.hCursor, SS_DEFCURSOR_COLHEADER);

					else if (SS_GetLock(lpSS, Col, Row, TRUE))
						SS_SetCursorCurrent(lpBook, lpBook->CursorLockedCell.hCursor, SS_DEFCURSOR_LOCKEDCELL);

					else if ((lpSS->RestrictCols && Col > lpSS->Col.DataCnt) ||
								(lpSS->RestrictRows && Row > lpSS->Row.DataCnt))
						SS_SetCursorCurrent(lpBook, lpBook->CursorLockedCell.hCursor, SS_DEFCURSOR_LOCKEDCELL);

#if SS_V80
					// moved down from above to allow custom cursors for header and locked cells
					else if ( lpSS->wOpMode == SS_OPMODE_READONLY ||
								  lpSS->wOpMode == SS_OPMODE_SINGLESEL ||
								  lpSS->wOpMode == SS_OPMODE_MULTISEL ||
								  lpSS->wOpMode == SS_OPMODE_EXTSEL )
						SS_SetCursorCurrent(lpBook, SS_CURSOR_DEFAULT, SS_CURSOR_DEFAULT);
					else if( SS_IsMouseInReservedLocation(lpSS, &RectClient, MouseX, MouseY, &lpBook->hCursorCurrent))
						{
						SS_SetCursorCurrent(lpBook, lpBook->hCursorCurrent, SS_CURSOR_DEFAULT);
						fMakeTransparent = FALSE;
						}
#endif
					else if (SS_IsMouseInButton(lpSS, &RectClient, MouseX, MouseY))
						{
						fMakeTransparent = FALSE;
						SS_SetCursorCurrent(lpBook, lpBook->CursorButton.hCursor, SS_DEFCURSOR_BUTTON);
						}
					else
						{
#ifdef SS_V35
						if (SS_IsMouseInCellNote(lpSS, Col, Row, MouseX, MouseY) &&
							 (SS_CELLNOTEINDICATOR_SHOWANDFIREEVENT == lpBook->nCellNoteIndicator ||
							  SS_CELLNOTEINDICATOR_SHOWANDDONOTFIREEVENT == lpBook->nCellNoteIndicator) )
							{       
							SS_SetCursorCurrent(lpBook, SS_CURSOR_ARROW, SS_CURSOR_ARROW);
							}
						else
							{
							SS_SetCursorCurrent(lpBook, SS_CURSOR_DEFAULT, SS_CURSOR_DEFAULT);
							fMakeTransparent = FALSE;
							}
#else             
						SS_SetCursorCurrent(lpBook, SS_CURSOR_DEFAULT, SS_CURSOR_DEFAULT);
						fMakeTransparent = FALSE;
#endif
						}   

#ifdef SS_MAKETRANSPARENT
					if (fMakeTransparent)
						return (HTTRANSPARENT);
#endif
					}
				}

			else
				SS_SetCursorCurrent(lpBook, SS_CURSOR_ARROW, SS_CURSOR_ARROW);
			}

      break;

	case WM_INPUTLANGCHANGE:
		SS_FontTableReCreate(lpBook);
		break;

   case WM_SYSKEYDOWN:
      if (lpBook->fMsgSentToVB)
// RFW 10/19/97         return (0);
         break;

      if (wParam == VK_BACK)
         {
         if (lpBook->fAllowUndo)
            SS_Undo(hWnd, lpSS);

         lpBook->wMsgLast = 0;
         return (0);
         }

		{
		LPSS_CELLTYPE lpCellType;
		SS_CELLTYPE   CellTypeTemp;

      lpCellType = SS_RetrieveCellType(lpSS, &CellTypeTemp, NULL,
                                       lpSS->Col.CurAt, lpSS->Row.CurAt);

      if (lpCellType->Type == SS_TYPE_COMBOBOX &&
          ((lParam >> 29) & 0x01) && wParam == VK_DOWN)
         {
         SS_ResetBlock(lpSS);
         lpBook->wMode = SS_MODE_NONE;
         SS_CellEditModeOn(lpSS, Msg, wParam, lParam);
         }
		}

		if (wParam == VK_NEXT || wParam == VK_PRIOR)
			{
			lpSS->fKeyDownBeingPerformed = TRUE;
			SS_KeyDown(lpSS, wParam, lParam, FALSE);
			lpSS->fKeyDownBeingPerformed = FALSE;
			}

      break;

   case WM_KEYDOWN:
      // BJO 04Mar97 SCS4604 - Begin fix
      if (GetCapture() == hWnd && wParam != VK_ESCAPE)
         return 0;
      // BJO 04Mar97 SCS4604 - End fix

      if (lpBook->fMsgSentToVB)
// RFW 10/19/97         return (0);
         break;

// BJO 29Apr96 TEL1905 - Before fix
/*
      lRet = SS_KeyDown(lpSS, wParam, lParam, TRUE);

      if (lRet != -1)
         {
#ifdef SS_NOSUPERCLASS

         if (SS_IsDestroyed(hWnd))
            return (FALSE);

         VBDefControlProc(VBGetHwndControl(hWnd), hWnd, Msg, wParam, lParam);
#endif
         return (lRet);
         }
*/

// BJO 29Apr96 TEL1905 - Start of fix

#ifdef SS_NOSUPERCLASS
      {
      HCTL hCtl = VBGetHwndControl(hWnd);
      LPVBSPREAD lpSpread = (LPVBSPREAD)VBDerefControl(hCtl);
      SPREADPARAMS2 Params2;
      short nShift = 0;

      if (GetKeyState(VK_SHIFT) & 0x8000)
         wShift |= 0x01;
      if (GetKeyState(VK_CONTROL) & 0x8000)
         wShift |= 0x02;
      Params2.lpParam1 = &wParam;
      Params2.lpParam2 = &wShift;
      VBFireEvent(VBGetHwndControl(hWnd), SS_EVENT_KeyDown, &Params2);
      // BJO 28Oct96 SEL5718 - Begin fix
      if (SS_IsDestroyed(hWnd))
         return 0;
      // BJO 28Oct96 SEL5718 - End fix
      if (wParam != 0)
      {
#endif

		{
      long lRet;
		
		lpSS->fKeyDownBeingPerformed = TRUE;
      lRet = SS_KeyDown(lpSS, wParam, lParam, TRUE);
		lpSS->fKeyDownBeingPerformed = FALSE;

      if (lRet != -1)
         {
         if (SS_IsDestroyed(hWnd))
            return (FALSE);
         return (lRet);
         }
		}

#ifdef SS_NOSUPERCLASS 
      if (VK_F1 == wParam)
         VBDefControlProc(VBGetHwndControl(hWnd), hWnd, Msg, wParam, lParam);
      }
      return 0;
      }
#endif

// BJO 29Apr96 TEL1905 - End of fix
        
      break;

   case WM_KEYUP:
      if (lpBook->fMsgSentToVB)
         break;

//#ifdef SS_V40
		else if (lpSS->wOpMode == SS_OPMODE_EXTSEL && HIBYTE(GetKeyState(VK_SHIFT)) && VK_F8 == wParam)
			{
			if (!lpSS->nExtSelF8Mode)
				{
				SetTimer(hWnd, SS_F8EXTSEL_TIMER_ID, SS_F8EXTSEL_TIMER_DELAY, NULL);
				lpSS->nExtSelF8Mode = -1;
				SS_HighlightCell(lpSS, 1 == lpSS->nExtSelF8Mode);
				}
			else
				SS_KillExtSelF8Mode(lpSS);
			}
//#endif // SS_V40

      else if (wParam == VK_SHIFT && !HIBYTE(GetKeyState(VK_SHIFT)) &&
               !(HIBYTE(GetKeyState(VK_LBUTTON))))
         {
         lpBook->wMode = SS_MODE_NONE;

         if (lpSS->fExtSelHighlight &&
             (lpSS->BlockCellUL.Row != lpSS->BlockCellULPrev.Row ||
              lpSS->BlockCellUL.Col != lpSS->BlockCellULPrev.Col ||
              lpSS->BlockCellLR.Row != lpSS->BlockCellLRPrev.Row ||
              lpSS->BlockCellLR.Col != lpSS->BlockCellLRPrev.Col))
            {
            if (lpSS->fMultipleBlocksSelected)
               {
               lpSS->MultiSelBlock.dItemCnt--;
               SS_MultiSelBlockAddItem(lpSS, &lpSS->BlockCellUL,
                                       &lpSS->BlockCellLR);
               }

				/* RFW - 12/10/04 - This fix broke code
				if (lpSS->wOpMode != SS_OPMODE_EXTSEL) // RFW - 1/22/04 - 13500
				*/
	            SS_SendMsgCommand(lpBook, lpSS, SSN_BLOCKSELECTED, FALSE);
            }
         }

      break;

//#ifdef SS_V40
   case WM_TIMER:
		if (wParam == SS_F8EXTSEL_TIMER_ID)
			{
			lpSS->nExtSelF8Mode = -lpSS->nExtSelF8Mode;
			SS_HighlightCell(lpSS, 1 == lpSS->nExtSelF8Mode);
			}
	  break;
//#endif // SS_V40

   // [VS] 2007/05/05 - Fix out-of-order IME-composition bug on WinXP. - 19759
#if defined(_UNICODE) && defined(SPREAD_JPN) // RFW - 12/6/07 - 21628
   case WM_IME_STARTCOMPOSITION:
      if(SS_GetEditMode(lpSS->lpBook) == FALSE)
         SS_SetEditMode(lpSS->lpBook, TRUE);
      break;
#endif

#ifdef _UNICODE
	case WM_IME_CHAR: // RFW - 2/27/04 - 13757
//		Msg = WM_CHAR;
#endif
   case WM_CHAR:
      // BJO 04Mar97 SCS4604 - Begin fix
      if (GetCapture() == hWnd)
         return 0;
      // BJO 04Mar97 SCS4604 - End fix

      if (lpBook->fMsgSentToVB)
// RFW 10/19/97         return (0);
         break;

#if defined(SS_NOSUPERCLASS) || defined(SS_DLL)
      if (SS_SendMsg(lpBook, lpSS, SSM_KEYPRESS, 0, (LPARAM)(LPVOID)&wParam) ||
          wParam == 0)
         return (0);
#endif

#ifdef SS_UTP
      if (lpSS->fProhibitTypingWhenSel && SS_IsBlockSelected(lpSS) &&
          !(wParam == 3 || wParam == 22 || wParam == 24 || wParam == 26))
         SS_SendMsgCommand(hWnd, NULL, SSN_TYPINGPROHIBITED, FALSE);
#endif // SS_UTP
      else if (lpSS->wOpMode == SS_OPMODE_MULTISEL)
         {
#ifndef SS_NOLISTBOX
         if (wParam == 32)             // Space bar
            SS_SelectRow(hWnd, lpSS);
#endif
         }

      else if (wParam == 32 && (HIBYTE(GetKeyState(VK_CONTROL)) ||
               HIBYTE(GetKeyState(VK_SHIFT))))
         ;

      else if (lpSS->wOpMode == SS_OPMODE_SINGLESEL ||
               lpSS->wOpMode == SS_OPMODE_MULTISEL ||
               lpSS->wOpMode == SS_OPMODE_EXTSEL)
         ;

      /*********************************************************
      * Check to see if Ctrl-c, Ctrl-v or Ctrl-x were pressed.
      * If so, eat the key.
      *********************************************************/

      else if (HIBYTE(GetKeyState(VK_CONTROL)) && (wParam == 'c' - 'a' + 1 ||
               wParam == 'v' - 'a' + 1 || wParam == 'x' - 'a' + 1))
         ;

// Added by BOC FMH 1996.07.11. --------------------->>
//   for KANJI input with EditModePermanent = TRUE and EditMode = OFF
#ifdef SPREAD_JPN
      else if (wParam != '\t' && wParam != '\r' && IsWindowEnabled(hWnd) && IsWindowVisible(hWnd)
              && ((!lpBook->fEditModePermanent || (lpSS->wOpMode == SS_OPMODE_ROWMODE && !lpSS->fRowModeEditing))
                  || (lpBook->fEditModePermanent && lpSS->wOpMode == SS_OPMODE_ROWMODE && lpSS->fRowModeEditing)))
#else
//--------------------------<<
      else if (wParam != '\t' && wParam != '\r' && IsWindowEnabled(hWnd) &&
               IsWindowVisible(hWnd) && (!lpBook->fEditModePermanent ||
               (lpSS->wOpMode == SS_OPMODE_ROWMODE &&
               !lpSS->fRowModeEditing)))
#endif  // BOC FMH
         {
         SS_ResetBlock(lpSS);
         lpBook->wMode = SS_MODE_NONE;


// RFW - 8/23/01 #ifdef SPREAD_JPN
         //------------------------------------------------------------
         // This preprocessor should not be removed. It solves
         // problems for DBCS input at the cell selection level.
         // This section needs to be checked for other DBCS platforms.
         // JPNFIX0004 - (Masanori Iwasa)
         //------------------------------------------------------------
         if (lpBook->EditModeOn || lpBook->EditModeTurningOn) {
            HWND   hWndCtrl;
            
            hWndCtrl = SS_RetrieveControlhWnd(lpSS, lpSS->Col.CurAt,
                                              lpSS->Row.CurAt);

            if (hWndCtrl)
					{
               PostMessage(hWndCtrl, Msg, wParam, lParam);
// 96' 5/17 BOC Gao. for a bug when input Kanji "‚Äö¬Å‚Äö‚Äö‚Äö∆í‚Äö‚Äû" while EditMode is off.
#ifdef SS_VB
               if(IsDBCSLeadByte((BYTE)wParam))
						{
                  MSG    nMsg;

                  PeekMessage(&nMsg, hWnd, WM_CHAR, WM_CHAR, PM_REMOVE);
                  PostMessage(hWndCtrl, Msg, nMsg.wParam, nMsg.lParam);
                  }
#endif
// ----------------------------------------------------------------------------<<
               } 
            }
         else
// RFW - 8/23/01 #endif

//#ifdef SPREAD_JPN
           //- JPNFIX0005 - (Masanori Iwasa)
           if(wParam != VK_ESCAPE
#if SS_V80
             && SS_CT_IsReservedKey(lpSS, lpSS->Col.CurAt, lpSS->Row.CurAt, wParam)
#endif
             )
//#endif
//
// Modified by BOC FMH 1996.07.08. ------------------------------>>
/*  for Japanese DBCS char procession. With RowMode, input to the spread
 *  with a KANJI string while EditMode is OFF, the inputted string is
 *  showwing wrong at this case.
 */

// RFW - 8/23/01 #ifndef SPREAD_JPN
//              SS_CellEditModeOn(lpSS, Msg, wParam, lParam);
// RFW - 8/23/01 #else
              {
              BOOL fRowEditOff = (lpSS->wOpMode == SS_OPMODE_ROWMODE)
                                 && (!lpSS->fRowModeEditing);

              SS_CellEditModeOn(lpSS, Msg, wParam, lParam);

              if(fRowEditOff && IsDBCSLeadByte((BYTE)wParam))
                  {
                  HWND   hWndCtrl;
                  hWndCtrl = SS_RetrieveControlhWnd(lpSS, lpSS->Col.CurAt,
                                              lpSS->Row.CurAt);

                  if (hWndCtrl)
                     {
                     MSG nMsg;
                     PeekMessage(&nMsg, hWnd, WM_CHAR, WM_CHAR, PM_REMOVE);
                     PostMessage(hWndCtrl, WM_CHAR, nMsg.wParam, nMsg.lParam);
/* RFW - 5/18/04 - 14260
                     PeekMessage(&nMsg, hWnd, WM_CHAR, WM_CHAR, PM_REMOVE);
                     if (IsDBCSLeadByte((BYTE)wParam))
                        {
                        PostMessage(hWndCtrl, WM_CHAR, nMsg.wParam, nMsg.lParam);
                        PeekMessage(&nMsg, hWnd, WM_CHAR, WM_CHAR, PM_NOREMOVE);
                        PostMessage(hWndCtrl, WM_CHAR, nMsg.wParam, nMsg.lParam);
                        }
*/
                     }
                  }
              }
// RFW - 8/23/01 #endif
//------------------------------<<
         }

// 97' 2/3 Added by BOC Gao. for BUG000634
// When using IME's single char select, it doesn't work.
// for any other cases than above, goes here
// if there is a Japanese Character, we should process it.
// RFW - 8/23/01 #ifdef SPREAD_JPN
		else if(IsDBCSLeadByte((BYTE)wParam))
		{
			HWND   hWndCtrl;
			if (!lpBook->EditModeOn && !lpBook->EditModeTurningOn)
				SS_CellEditModeOn(lpSS, Msg, wParam, lParam);
			hWndCtrl = SS_RetrieveControlhWnd(lpSS, lpSS->Col.CurAt,
                                              lpSS->Row.CurAt);
			if (hWndCtrl) 
			{
				MSG    nMsg;
				PostMessage(hWndCtrl, Msg, wParam, lParam);
				PeekMessage(&nMsg, hWnd, WM_CHAR, WM_CHAR, PM_REMOVE);
				PostMessage(hWndCtrl, Msg, nMsg.wParam, nMsg.lParam);
			} 
		}
// RFW - 8/23/01 #endif
// ---------------------------------<<

      return (0);

   /*********************************************************
   * Any horizontal scroll (with mouse) causes this message
   *********************************************************/

   case WM_HSCROLL:
      HANDLE_WM_HSCROLL(hWnd, wParam, lParam, SS_OnHScroll);
      return (0);

   /***********************************
   * A vertical scroll action occured
   ***********************************/

   case WM_VSCROLL:
      HANDLE_WM_VSCROLL(hWnd, wParam, lParam, SS_OnVScroll);
      return (0);

   case WM_SETFOCUS:
#ifdef SS_NOPROCESSFOCUS
      if (!lpBook->EditModeTurningOn &&
          !lpBook->EditModeTurningOff &&
          !lpBook->wMessageBeingSent &&
          !lpBook->fProcessingLButtonDown &&
          !lpBook->fRetainSelBlock)
         {
         /*
         if (!GetUpdateRect(hWnd, &Rect, FALSE))
         */
            SS_DrawSelBlock(0, lpSS);

         lpSS->fSelBlockInvisible = FALSE;
         }
#ifdef SS_UTP
      if (!lpBook->EditModeTurningOn && !lpBook->EditModeTurningOff &&
          !lpBook->fProcessingLButtonDown && lpBook->EditModeOn)
         SetFocus(lpBook->hWndFocusPrev);
#endif

      break;
#endif
   case SSM_SETFOCUSINTERNAL:
		if (!lpSS)
			return (0);

      // BJO 03Jul96 JAP4544 - Begin fix
      if (lpBook->fIgnoreKillFocus)
         return 0;
      // BJO 03Jul96 JAP4544 - End fix

      lpBook->fSetFocusRecieved = TRUE;
      lpBook->fProcessSetFocus = FALSE;

      if (!lpBook->EditModeTurningOn &&
          !lpBook->EditModeTurningOff &&
          !lpBook->wMessageBeingSent &&
          !lpBook->fProcessingLButtonDown)
         {
         lpBook->fProcessSetFocus = TRUE;

#ifndef SS_NOPROCESSFOCUS
         if (!lpBook->fRetainSelBlock)
            {
            /*
            SS_DrawSelBlock(hWnd, 0, lpSS);
            */
// RFW-2/17/96               lpSS->fSelBlockInvisible = FALSE;
            /*
            if (!lpBook->fEditModePermanent)
               SS_HighlightCell(lpSS, TRUE);
            */
            }

         if (!lpBook->EditModeTurningOn &&
             !lpBook->EditModeTurningOff &&
             !lpBook->wMessageBeingSent &&
             !lpBook->fProcessingLButtonDown)
            if (GetFocus() == hWnd)
               if (lpBook->fEditModePermanent && lpBook->hWndFocusPrev)
                  {
                  lpBook->wMessageBeingSent++; //???? WM_SETFOCUS
                  SetFocus(lpBook->hWndFocusPrev);
                  lpBook->wMessageBeingSent--; //???? WM_SETFOCUS
                  lpBook->hWndFocusPrev = 0;
                  }
#endif

#ifndef SS_NOPROCESSFOCUS
         SendMessage(hWnd, SSM_PROCESSSETFOCUS, GetWindowID(hWnd), MAKELONG(hWnd, 0));
/* RFW - 12/31/01 - 3727
         PostMessage(hWnd, SSM_PROCESSSETFOCUS, GetWindowID(hWnd), MAKELONG(hWnd, 0));
*/
#else
         SendMessage(hWnd, SSM_PROCESSSETFOCUS, GetWindowID(hWnd), MAKELONG(hWnd, 0));
#endif
         }

      else if (lpBook->fProcessingLButtonDown)
         {
         lpBook->fProcessSetFocus = TRUE;
         SendMessage(hWnd, SSM_PROCESSSETFOCUS, GetWindowID(hWnd),
                     MAKELONG(hWnd, 0));
         }

      else if (lpBook->wMessageBeingSent)
         {
#ifndef SS_NOPROCESSFOCUS
// 96' 6/26 Removed by BOC Gao. for a UAE 
// When EditModePermanent = TRUE and user pushes the ALT key an UAE occurs 
// when the following code is executed.

// RFW - 3/9/04 - 13808
//#ifndef SPREAD_JPN
         // BJO 13May96 GRB1617 - Begin fix
         if (!lpBook->EditModeTurningOn && !lpBook->EditModeTurningOff &&
             lpBook->hWndFocusPrev)
            {
            lpBook->fIgnoreKillFocus = TRUE;
            SetFocus(lpBook->hWndFocusPrev);
            lpBook->fIgnoreKillFocus = FALSE;
            }
         else
//#endif
// --------------------------------------<<
         // BJO 13May96 GRB1617 - End fix
           PostMessage(hWnd, SSM_PROCESSSETFOCUS, GetWindowID(hWnd),
                       MAKELONG(hWnd, 0));
#else
         lpSS->fSetFocusWhileMsgBeingSent = TRUE;
#endif
         }

#if 0
      if (lpBook->fProcessingLButtonDown &&
          SS_IsBlockSelected(lpSS) &&
          !lpBook->fRetainSelBlock)
         /*
         SS_InvalidateRect(lpBook, NULL, TRUE);
         */
         SS_InvertBlock(0, lpSS);
#endif

      break;

#ifdef SS_NOPROCESSFOCUS
   case SSM_SETFOCUSPOSTPROCESS:
      if (!lpBook->EditModeTurningOn &&
          !lpBook->EditModeTurningOff &&
          !lpBook->wMessageBeingSent &&
          !lpBook->fProcessingLButtonDown)
         {
         HWND hWndFocus = GetFocus();

         if (!lpSS->fSetFocusRecieved &&
             (hWndFocus == hWnd || (hWndFocus && GetParent(hWndFocus) == hWnd)))
            if (lpBook->hWndFocusPrev)
               {
#ifdef SS_UTP
               if (!lpSS->fAllowEditModePermSel ||
                   !SS_IsBlockSelected(lpSS))
#endif
                  SetFocus(lpBook->hWndFocusPrev);

               lpBook->hWndFocusPrev = 0;
               }

            else if (!lpBook->fEditModePermanent)
               {
               if (SS_USESINGLESELBAR(lpSS))
                  {
                  if (lpBook->fKillFocusReceived)
                     {
                     fHighlightOnOld = lpSS->HighlightOn;

                     if (lpSS->HighlightOn)
                        {
                        lpSS->HighlightOn = FALSE;
                        lpBook->fProcessingKillFocus = TRUE;
                        }

                     SS_HighlightCell(lpSS, TRUE);

                     if (fHighlightOnOld)
                        {
                        lpSS->HighlightOn = TRUE;
                        lpBook->fProcessingKillFocus = FALSE;
                        }

                     lpBook->fKillFocusReceived = FALSE;
                     }
                  }
               else if (!lpBook->EditModeOn)
                  SS_HighlightCell(lpSS, TRUE);
               }
         }

      break;
#endif

	case WM_CANCELMODE:
		if (GetCapture() == lpBook->hWndHScroll)
			SendMessage(lpBook->hWndHScroll, Msg, wParam, lParam);
		else if (GetCapture() == lpBook->hWndVScroll)
			SendMessage(lpBook->hWndVScroll, Msg, wParam, lParam);
		break;

   case WM_KILLFOCUS:
#ifdef SS_NOPROCESSFOCUS
      //- JPNFIX0006 - (Masanori Iwasa)
      if (GetCapture() == hWnd)
         ReleaseCapture();

#ifndef SS_NODRAGDROP
      if (lpBook->wMode == SS_MODE_DRAGDROP)
			SS_DragDropDrawBox(hWnd, lpSS, FALSE);
#endif

      lpBook->wMode = SS_MODE_NONE;

//#ifdef SS_V40
		SS_KillExtSelF8Mode(lpSS);
//#endif // SS_V40

      if (lpBook->EditModeTurningOff)
         lpSS->NoEraseBkgnd = FALSE;
      if (!lpBook->EditModeOn)
         lpBook->hWndFocusPrev = 0;
      if (!lpBook->EditModeTurningOn &&
          !lpBook->EditModeTurningOff &&
          //!lpBook->wMessageBeingSent && // BJO 21Feb96 GRB5555 - Removed
          !lpBook->fRetainSelBlock &&
          !SS_USESELBAR(lpSS) &&
          !lpSS->fSelBlockInvisible)
         {
         SS_DrawSelBlock(0, lpSS);
#ifdef SS_V80
		SS_InvalidateActiveHeaders(lpSS);
#endif

         lpSS->fSelBlockInvisible = TRUE;
         if (!lpBook->fEditModePermanent)
            SS_HighlightCell(lpSS, FALSE);
         }

      break;
#endif
   case SSM_KILLFOCUSINTERNAL:
      // BJO 03Jul96 JAP4544 - Begin fix
      if (!lpSS || lpBook->fIgnoreKillFocus)
         return 0;
      // BJO 03Jul96 JAP4544 - End fix

//#ifdef SS_V40
		SS_KillExtSelF8Mode(lpSS);
//#endif // SS_V40

		if (!wParam || GetParent((HWND)wParam) != hWnd)
			{
			HWND hWndCapture = GetCapture();
			if (hWndCapture && (hWndCapture == hWnd || GetParent(hWndCapture) == hWnd))
				ReleaseCapture();

			// RFW - 7/7/04 - 14833
	      lpBook->wMode = SS_MODE_NONE;
			}

#ifndef SS_NODRAGDROP
      if (lpBook->wMode == SS_MODE_DRAGDROP)
			SS_DragDropDrawBox(hWnd, lpSS, FALSE);
#endif

#ifdef SS_V70
      if (lpBook->wMode == SS_MODE_COLMOVE || lpBook->wMode == SS_MODE_ROWMOVE)
			SS_HoverEnd(lpSS);
#endif // SS_V70

		// RFW - 7/7/04 - 14833
      // lpBook->wMode = SS_MODE_NONE;

      if (lpBook->EditModeTurningOff)
         lpSS->NoEraseBkgnd = FALSE;

      #ifndef SS_NOPROCESSFOCUS
      if (!lpBook->EditModeTurningOn &&
          !lpBook->EditModeTurningOff &&
          //!lpBook->wMessageBeingSent &&  // BJO 21Feb97 GRB5555 - Removed
          !lpBook->fRetainSelBlock &&
          !SS_USESELBAR(lpSS) &&
          !lpBook->fSelBlockInvisible)  // BJO 21Feb97 GRB5555 - Added
         {
         SS_DrawSelBlock(0, lpSS);
         lpBook->fSelBlockInvisible = TRUE;
         if (!lpBook->fEditModePermanent)
            SS_HighlightCell(lpSS, FALSE);
#ifdef SS_V80		
		 SS_InvalidateActiveHeaders(lpSS);
#endif
         }
      #endif

      if (!lpBook->EditModeTurningOn &&
          !lpBook->EditModeTurningOff &&
          !lpBook->wMessageBeingSent)
         {
#ifndef SS_NOPROCESSFOCUS
			if (wParam)
				PostMessage(hWnd, SSM_PROCESSKILLFOCUS, wParam,
						      MAKELONG(hWnd, 0));
			else
				SendMessage(hWnd, SSM_PROCESSKILLFOCUS, wParam,
		                  MAKELONG(hWnd, 0));
#else
         SendMessage(hWnd, SSM_PROCESSKILLFOCUS, wParam,
                     MAKELONG(hWnd, 0));
#endif
#ifdef SS_V80		
		 SS_InvalidateActiveHeaders(lpSS);
#endif
         }

#ifndef SS_NOPROCESSFOCUS
      else if(lpBook->wMessageBeingSent)
         SendMessage(hWnd, SSM_PROCESSKILLFOCUS, wParam, MAKELONG(hWnd, 0));
#endif
      if (SS_IsDestroyed(hWnd))
         return 0;

#ifdef SS_NOPROCESSFOCUS
      lpBook->fKillFocusReceived = TRUE;
#endif

      break;

#ifdef SS_V30
   case FPM_TT_FETCH:
		{
      LPFP_TT_FETCH lpFetch = (LPFP_TT_FETCH)lParam;
		SS_COORD      Row;
		SS_COORD      Col;
		HWND          hWndFocus = GetFocus();
/*
		if (!hWndFocus || (hWndFocus != hWnd && GetParent(hWndFocus) != hWnd))
			{
         lpFetch->fShow = FALSE;
			return (0);
			}
*/
      SS_GetCellFromPixel(lpSS, &Col, &Row, NULL, NULL,
                          lpFetch->Scan.nMouseX, lpFetch->Scan.nMouseY);

      if (Col == -2 || Row == -2)
         lpFetch->fShow = FALSE;

      else
         {
         SS_TEXTTIPFETCH SSFetch;
         TBGLOBALHANDLE  hData;
         RECT            RectOrig;
         int             nWidth;
         BOOL            fCellNote = FALSE;
#ifdef SS_V35
         TBGLOBALHANDLE  hCellNote = 0;
#endif

			SS_GetActualCell(lpSS, Col, Row, &Col, &Row, &lpFetch->Rect);

         _fmemset(&SSFetch, '\0', sizeof(SS_TEXTTIPFETCH));
         SSFetch.wMultiLine = lpFetch->wMultiLine;
         SSFetch.wAlignX = lpFetch->wAlignX;
         SSFetch.wAlignY = lpFetch->wAlignY;
         SSFetch.xOffset = lpFetch->xOffset;
         SSFetch.yOffset = lpFetch->yOffset;
         SSFetch.nWidth = lpFetch->nWidth;
         SSFetch.Col = Col;
         SSFetch.Row = Row;
         SSFetch.fShow = FALSE;

//         SS_GetCellRect(lpSS, Col, Row, &lpFetch->Rect);

  			//lpFetch->Rect.left++;
  			lpFetch->Rect.left += 2;
			lpFetch->Rect.top += 1;
         CopyRect(&RectOrig, &lpFetch->Rect);

#ifdef SS_V35
         if ((SS_CELLNOTEINDICATOR_SHOWANDFIREEVENT == lpBook->nCellNoteIndicator ||
              SS_CELLNOTEINDICATOR_DONOTSHOWANDFIREEVENT == lpBook->nCellNoteIndicator) )
           fCellNote = SS_IsMouseInCellNote(lpSS, Col, Row, lpFetch->Scan.nMouseX, lpFetch->Scan.nMouseY);

         if (fCellNote || (hData = SS_GetData(lpSS, NULL, Col, Row, FALSE)))
#else
         if (hData = SS_GetData(lpSS, NULL, Col, Row, FALSE))
#endif
            {
            LPTSTR      lpszData;
            LPSS_CELL   lpCell = SS_LockCellItem(lpSS, Col, Row);
            SS_CELLTYPE CellType;
            SS_FONT     Font;
            HFONT       hFontOld;
            HDC         hDC = GetDC(hWnd);

#ifdef SS_V35
            if (fCellNote)
            {
               LPSS_CELL lpCell = SS_LockCellItem(lpSS, Col, Row);
               LPSS_COL  lpCol = SS_LockColItem(lpSS, Col);
               LPSS_ROW  lpRow = SS_LockRowItem(lpSS, Row);
               BOOL      fDoCheck = FALSE;

               if (lpCell && lpCell->hCellNote)
                 hCellNote = lpCell->hCellNote;
               else if (lpCol && lpCol->hCellNote)
                 hCellNote = lpCol->hCellNote;
               else if (lpRow && lpRow->hCellNote)
                 hCellNote = lpRow->hCellNote;
               else if (lpSS->hCellNote)
                 hCellNote = lpSS->hCellNote;

               if (hCellNote)
                 lpszData = (LPTSTR)tbGlobalLock(hCellNote);

               SSFetch.fShow = TRUE;
            }
            else    
#endif
               lpszData = (LPTSTR)tbGlobalLock(hData); // make this call if SS_V35 or not.


            SS_RetrieveCellType(lpSS, &CellType, NULL, Col, Row);

				if (CellType.Type == SS_TYPE_EDIT && (CellType.Style & ES_PASSWORD))
					lpszData = _T("");

#ifdef SS_V70
				{
				LPTSTR lpszTemp;
 				SSFetch.hText = GlobalAlloc(GHND, (lstrlen(lpszData) + 1)*sizeof(TCHAR));
				lpszTemp = (LPTSTR)GlobalLock(SSFetch.hText);
            lstrcpy(lpszTemp, lpszData);
				GlobalUnlock(SSFetch.hText);
				}
#else
            lstrcpyn(SSFetch.szText, lpszData, SS_TT_TEXTMAX);
#endif // SS_V70

            SS_RetrieveFont(lpSS, &Font, lpCell, Col, Row);
            SS_UnlockCellItem(lpSS, Col, Row);

            if (Font.hFont)
               hFontOld = SelectObject(hDC, Font.hFont);

				{
            SIZE        Size;
            GetTextExtentPoint32(hDC, lpszData, lstrlen(lpszData), &Size);
            nWidth = (short)Size.cx;
				}

            if (nWidth)
               nWidth += lpBook->dXMargin;

            if (Font.hFont)
               SelectObject(hDC, hFontOld);

            if (CellType.Type == SS_TYPE_COMBOBOX &&
                SS_ShouldButtonBeDrawn(lpSS, Col, Row, CellType.Type))
               lpFetch->Rect.right -= lpBook->dComboButtonBitmapWidth;

            if (lpBook->fAllowCellOverflow && !fCellNote)
               {
               LPSS_CELL lpCellTemp;
               SS_COORD  TempColAt;
               long      lAlign = SS_GetCellAlign(&CellType);

               if ((lAlign & SSS_ALIGN_CENTER) ||
                   (lAlign & SSS_ALIGN_RIGHT))
                  {
                  for (TempColAt = Col - 1; TempColAt >=
                       lpSS->Col.HeaderCnt + lpSS->Col.Frozen; TempColAt--)
                     {
                     if ((lpCellTemp = SS_LockCellItem(lpSS, TempColAt, Row)) &&
                         (lpCellTemp->Data.bOverflow & SS_OVERFLOW_RIGHT))
                        lpFetch->Rect.left -= SS_GetColWidthInPixels(lpSS, TempColAt);
                     else
                        break;
                     }
                  }

               if ((lAlign & SSS_ALIGN_CENTER) ||
                   !(lAlign & SSS_ALIGN_RIGHT))
                  {
                  for (TempColAt = Col + 1; TempColAt < SS_GetColCnt(lpSS);
                       TempColAt++)
                     {
                     if ((lpCellTemp = SS_LockCellItem(lpSS, TempColAt, Row)) &&
                         (lpCellTemp->Data.bOverflow & SS_OVERFLOW_LEFT))
                        lpFetch->Rect.right += SS_GetColWidthInPixels(lpSS, TempColAt);
                     else
                        break;
                     }
                  }
               }

            if (nWidth > (short)(lpFetch->Rect.right - lpFetch->Rect.left - 1))
               {
               if ((CellType.Type == SS_TYPE_STATICTEXT &&
                    (CellType.Style & SS_TEXT_WORDWRAP)) ||
                   (CellType.Type == SS_TYPE_EDIT &&
                    (CellType.Style & ES_MULTILINE)))
                  {
                  RECT RectTemp;
                  int  iHeight;

                  SetRect(&RectTemp, lpFetch->Rect.left, lpFetch->Rect.top,
                          lpFetch->Rect.right, lpFetch->Rect.bottom);
                  SS_CalcCellMetrics(hDC, lpSS, NULL, NULL, NULL, &CellType,
                                     &RectTemp, Col, Row, FALSE, TRUE, NULL, NULL, NULL, &iHeight);
// RFW - 7/15/99 - KEM9                     if (nHeight > (short)(lpFetch->Rect.bottom - lpFetch->Rect.top - 1))
                  if (iHeight > (short)(lpFetch->Rect.bottom - lpFetch->Rect.top))
                     SSFetch.fShow = TRUE;
                  }
               else
                  SSFetch.fShow = TRUE;
               }

            ReleaseDC(hWnd, hDC);

            if (!SSFetch.fShow)
               {
               SS_CELLTYPE CellType;

               if (SS_RetrieveCellType(lpSS, &CellType, NULL, Col, Row))
                  {
                  long     lAlign = SS_GetCellAlign(&CellType);
                  int      iClientLeft = 0;
                  SS_COORD i;

                  for (i = 0; i < lpSS->Col.HeaderCnt + lpSS->Col.Frozen; i++)
                     iClientLeft += SS_GetColWidthInPixels(lpSS, i);

                  if (lAlign & SSS_ALIGN_CENTER)
                     {
/* RFW - 7/15/99 - KEM9
                     if (nWidth / 2 > (min(lpSS->ClientRect.right,
                                           lpFetch->Rect.right) -
                                       (RectOrig.left + 1 +
                                        ((RectOrig.right -
                                          RectOrig.left) / 2))))
*/
                     if (nWidth / 2 > (lpBook->ClientRect.right -
                                       (RectOrig.left + 1 +
                                        ((RectOrig.right -
                                          RectOrig.left) / 2))))
                        SSFetch.fShow = TRUE;

/* RFW - 7/15/99 - KEM9
                     else if (nWidth / 2 > ((RectOrig.left + 1 +
                                             ((RectOrig.right -
                                               RectOrig.left) / 2)) -
                                               max(iClientLeft,
                                                   lpFetch->Rect.left)))
*/
                     else if (nWidth / 2 > ((RectOrig.left + 1 +
                                             ((RectOrig.right -
                                               RectOrig.left) / 2)) -
                                               iClientLeft))
                        SSFetch.fShow = TRUE;
                     }
                  else if (lAlign & SSS_ALIGN_RIGHT)
                     {
                     if (nWidth > lpFetch->Rect.right - max(iClientLeft,
                         lpFetch->Rect.left) - 1)
                        SSFetch.fShow = TRUE;
                     }
                  else              // Left Aligned
                     {
                     if (nWidth > lpBook->ClientRect.right - lpFetch->Rect.left - 1)
                        SSFetch.fShow = TRUE;
                     }
                  }
               }

#ifdef SS_V35
            if (fCellNote)
            {
               if (hCellNote)
                 tbGlobalUnlock(hCellNote);
               SS_UnlockCellItem(lpSS, Col, Row);
               SS_UnlockColItem(lpSS, Col);
               SS_UnlockRowItem(lpSS, Row);
            } 
            else
#endif
               tbGlobalUnlock(hData);
            }

#ifdef SS_V35              
         if (fCellNote)
            {
            SSFetch.wMultiLine = FP_TT_MULTILINE_MULTI;
            }
#endif
         CopyRect(&lpFetch->Rect, &RectOrig);
#ifdef SS_V35
         lpBook->fFetchCellNote = fCellNote;
#endif
			SS_AdjustCellCoordsOut(lpSS, &SSFetch.Col, &SSFetch.Row);
         SS_SendMsg(lpBook, lpSS, SSM_TEXTTIPFETCH, GetDlgCtrlID(lpBook->hWnd),
                    (LPARAM)(LPVOID)&SSFetch);
#ifdef SS_V35
         lpBook->fFetchCellNote = !fCellNote;
#endif
         lpFetch->fShow = SSFetch.fShow;
         lpFetch->wMultiLine = SSFetch.wMultiLine;
         lpFetch->wAlignX = SSFetch.wAlignX;
         lpFetch->wAlignY = SSFetch.wAlignY;
         lpFetch->xOffset = SSFetch.xOffset;
         lpFetch->yOffset = SSFetch.yOffset;
         lpFetch->nWidth = SSFetch.nWidth;
#ifdef SS_V70
			if (SSFetch.hText)
				lpFetch->hText = SSFetch.hText;
			else
#endif // SS_V70
         if (lstrlen(SSFetch.szText) > SS_TT_TEXTMAX)
         {
           lstrcpyn(lpFetch->szText, SSFetch.szText, SS_TT_TEXTMAX);
           lpFetch->szText[SS_TT_TEXTMAX] = 0;
         }
         else if (SSFetch.szText[0])
           lstrcpy(lpFetch->szText, SSFetch.szText);

         lpFetch->Scan.ID.lID1 = Col;
         lpFetch->Scan.ID.lID2 = Row;
#ifdef SS_V35
         lpFetch->Scan.ID.lID3 = fCellNote;
#endif
         }
		}
      return (0);

   case FPM_TT_SCAN:
		{
      LPFP_TT_SCAN lpScan = (LPFP_TT_SCAN)lParam;

      SS_GetCellFromPixel(lpSS, &lpScan->ID.lID1, &lpScan->ID.lID2, NULL, NULL,
                          lpScan->nMouseX, lpScan->nMouseY);

#ifdef SS_V35          
      if (lpBook->fThumb)
         lpScan->ID.lID3 = 2;
      else if ( SS_IsMouseInCellNote(lpSS, lpScan->ID.lID1, lpScan->ID.lID2,
                                     lpScan->nMouseX, lpScan->nMouseY) &&
                (SS_CELLNOTEINDICATOR_SHOWANDFIREEVENT == lpBook->nCellNoteIndicator ||
                 SS_CELLNOTEINDICATOR_DONOTSHOWANDFIREEVENT == lpBook->nCellNoteIndicator) )
         lpScan->ID.lID3 = 1;
      else 
         lpScan->ID.lID3 = 0;
#endif

		SS_AdjustCellCoordsOut(lpSS, &lpScan->ID.lID1, &lpScan->ID.lID2);
		}
      return (0);
#endif // SS_V30

   case WM_MOUSEACTIVATE:
      if (HIWORD(lParam) == WM_LBUTTONDOWN)
         {
         HWND hWndFocus = GetFocus();

         if (!hWndFocus || (hWndFocus != hWnd && GetParent(hWndFocus) != hWnd &&
             (!GetParent(hWndFocus) ||
             GetParent(GetParent(hWndFocus)) != hWnd)))
            {
#ifdef SPREAD_JPN
            //- JPNFIX0007 - (Masanori Iwasa)
            if (lpBook->fRetainSelBlock)
#endif
                lpBook->fProcessingLButtonDown = TRUE;
            }

#ifdef SS_VB
         if (!hWndFocus || (hWndFocus != hWnd &&
             (!GetParent(hWndFocus) || (GetParent(hWndFocus) != hWnd &&
             (!GetParent(GetParent(hWndFocus)) ||
             GetParent(GetParent(hWndFocus)) != hWnd)))))
            {
            if (!lpBook->fMoveActiveOnFocus)
               lpBook->fNoProcessButtonDown = TRUE;
            }
#endif

         // BJO SEL6570 15Nov96 - Begin fix
         #ifdef SS_OCX
         if (hWndFocus && (hWndFocus == hWnd || IsChild(hWnd, hWndFocus)))
            return MA_ACTIVATE;
         #endif
         // BJO SEL6570 15Nov96 - End fix
         }

      break;

#ifdef SS_V80
	  //WM_THEMECHANGED
   case 0x031A:
       lpBook->fThemesActive = SS_IsThemeActive();
       SS_InvalidateRect(lpBook, NULL, TRUE);
	   break;

#endif
   case WM_LBUTTONDOWN:
   case WM_LBUTTONDBLCLK:
		{
		HWND hWndFocus;
#ifdef SS_V80
	  if (!HIBYTE(GetKeyState(VK_CONTROL)))
	      lpSS->sortKeyCnt = 0;
#endif
      lpBook->fProcessingLButtonDown = FALSE;
      if (lpBook->fNoProcessButtonDown)
         {
         lpBook->fNoProcessButtonDown = FALSE;

         hWndFocus = GetFocus();
         if (!hWndFocus || (hWndFocus != hWnd && GetParent(hWndFocus) != hWnd))
            SetFocus(hWnd);

         break;
         }

//#ifdef SS_V40
		SS_KillExtSelF8Mode(lpSS);
//#endif // SS_V40

      hWndFocus = GetFocus();
      if (!hWndFocus || (hWndFocus != hWnd && GetParent(hWndFocus) != hWnd))
         {
         lpBook->fProcessingLButtonDown = TRUE;
         SetFocus(hWnd);

         if (lpSS && SS_USESELBAR(lpSS))
            {
            if (lpSS->HighlightOn)
               {
               lpSS->HighlightOn = FALSE;
               lpBook->fProcessingKillFocus = TRUE;
               SS_HighlightCell(lpSS, TRUE);
               lpSS->HighlightOn = TRUE;
               lpBook->fProcessingKillFocus = FALSE;
               }
            }

         if (!lpBook->fMoveActiveOnFocus && lpBook->fNoProcessButtonDown)
            {
            lpBook->fProcessingLButtonDown = FALSE;
            lpBook->fNoProcessButtonDown = FALSE;
            break;
            }
         }

      PostMessage(hWnd, Msg == WM_LBUTTONDOWN ? SSM_PROCESSLBUTTONDOWN :
                  SSM_PROCESSLBUTTONDBLCLK, LOWORD(lParam),
                  MAKELONG(HIWORD(lParam), 0));
		}
      break;

   case WM_LBUTTONUP:
		{
      lpBook->wMsgLast = 0;

		if (lpBook->wMode == SS_MODE_IDLE) // RFW - -8/3/04 - 14853
			lpBook->wMode = SS_MODE_NONE;

      else if (lpBook->wMode == SS_MODE_RESIZE_HORIZ || lpBook->wMode == SS_MODE_RESIZE_VERT)
         {
			SS_COORD Row;
			SS_COORD Row2;
			SS_COORD Col;
			SS_COORD Col2;
			SS_COORD i;
			double   dfVal;
			WORD     wMode = lpBook->wMode;
			BOOL     fRedraw;
			int      Height;
			int      Width;
			int      WidthTemp;
			int      x;
			int      y;

         ReleaseCapture();
         SS_Resize(hWnd, lpSS, SS_RESIZEMODE_END, lParam);
         lpBook->wMode = SS_MODE_NONE;

         if (wMode == SS_MODE_RESIZE_HORIZ && lpBook->fHasMouseMoved)  // Row
            {
				BOOL fFireRowHeightChange = FALSE;

            y = lpBook->ResizeCurrentPos;

            Height = SS_GetRowHeightInPixels(lpSS, lpBook->ResizeCoord);
				Height += y - lpBook->ResizeStartPos;

				if (SS_IsAllSelected(lpSS))
					{
               SS_CalcPixelsToRowHeight(lpSS, -1, Height, &dfVal);
					SSx_SetRowHeight(lpSS, -1, dfVal, TRUE);
					SS_SendMsgColCoordRange(lpSS, SSM_ROWHEIGHTCHANGE, GetWindowID(hWnd),
						                     -1, -1);
					break;
					}

            else if (lpSS->BlockCellUL.Row != -1 &&
                     lpSS->BlockCellUL.Col == 0 &&
                     lpSS->BlockCellLR.Col == -1)
               {
               Row = lpSS->BlockCellUL.Row;
               Row2 = lpSS->BlockCellLR.Row;
               }
            else
               {
               Row = lpBook->ResizeCoord;
               Row2 = lpBook->ResizeCoord;
               }

            fRedraw = lpBook->Redraw;
				SS_BookSetRedraw(lpBook, FALSE);
            for (i = Row; i <= Row2; i++)
               {
               if (SS_GetMergedUserResizeRow(lpSS, i) != SS_RESIZE_OFF)
                  {
                  SS_CalcPixelsToRowHeight(lpSS, i, Height, &dfVal);
                  fFireRowHeightChange |= SS_SetRowHeight(lpSS, i, dfVal);
                  }
               }

				SS_BookSetRedraw(lpBook, fRedraw);

            if (lpSS->Row.CurAt >= Row && lpSS->Row.CurAt <= Row2 &&
                Height + (y - lpBook->ResizeStartPos) == 0)
               {
               SS_ScrollLineLR(lpSS, SS_F_SCROLL_DOWN | SS_F_MOVEACTIVECELL);
               if (lpSS->Row.CurAt >= Row && lpSS->Row.CurAt <= Row2)
                  SS_ScrollLineUL(lpSS, SS_F_SCROLL_UP | SS_F_MOVEACTIVECELL);
               }

				// RFW - 1/24/05 - 15498
				// If resizing the top most row that is hidden, show it.
            else if (Row == Row2 && Row < lpSS->Row.UL && Height > 0)
               {
					SS_BookSetRedraw(lpBook, fRedraw);
               SS_ScrollLineUL(lpSS, SS_F_SCROLL_UP);
               }

				if (fFireRowHeightChange)
					SS_SendMsgRowCoordRange(lpSS, SSM_ROWHEIGHTCHANGE, GetWindowID(hWnd),
						                     Row, Row2);
            SS_HighlightCell(lpSS, TRUE);
            }

         else if (wMode == SS_MODE_RESIZE_VERT && lpBook->fHasMouseMoved)  // Column
            {
				BOOL fFireColWidthChange = FALSE;

            x = lpBook->ResizeCurrentPos;

            Width = SS_GetColWidthInPixels(lpSS, lpBook->ResizeCoord);
            Width += x - lpBook->ResizeStartPos;

				if (SS_IsAllSelected(lpSS))
					{
               SS_CalcPixelsToColWidth(lpSS, -1, Width, &dfVal);
					SSx_SetColWidth(lpSS, -1, dfVal, TRUE);
					SS_SendMsgColCoordRange(lpSS, SSM_COLWIDTHCHANGE, GetWindowID(hWnd),
						                     -1, -1);
					break;
					}

            else if (lpSS->BlockCellUL.Col != -1 &&
                     lpSS->BlockCellUL.Row == 0 &&
                     lpSS->BlockCellLR.Row == -1)
               {
               Col = lpSS->BlockCellUL.Col;
               Col2 = lpSS->BlockCellLR.Col;
               }
            else
               {
               Col = lpBook->ResizeCoord;
               Col2 = lpBook->ResizeCoord;
               }

            fRedraw = lpBook->Redraw;
				SS_BookSetRedraw(lpBook, FALSE);

#ifdef SS_UTP
            GetClientRect(hWnd, &RectClient);

            Width = min(Width, RectClient.right - SS_GetColWidthInPixels(lpSS, 0) -
                        SS_SCROLLARROW_WIDTH);
#endif

            for (i = Col; i <= Col2; i++)
               {
               if (SS_GetMergedUserResizeCol(lpSS, i) != SS_RESIZE_OFF)
                  {
#ifdef SS_UTP
                  if (i > lpSS->Col.HeaderCnt + lpSS->Col.Frozen)
                     WidthTemp = min(Width, RectClient.right -
                                 SS_GetColWidthInPixels(lpSS, 0) -
                                 (SS_SCROLLARROW_WIDTH +
                                 SS_SCROLLARROW_WIDTH));
                  else
#endif
                     WidthTemp = Width;

                  SS_CalcPixelsToColWidth(lpSS, i, WidthTemp, &dfVal);
						fFireColWidthChange |= SS_SetColWidth(lpSS, i, dfVal);
                  }
               }

            if (lpSS->Col.CurAt >= Col && lpSS->Col.CurAt <= Col2 &&
                Width == 0)
               {
					SS_BookSetRedraw(lpBook, fRedraw);
               SS_ScrollLineLR(lpSS, SS_F_SCROLL_RIGHT | SS_F_MOVEACTIVECELL);
               if (lpSS->Col.CurAt >= Col && lpSS->Col.CurAt <= Col2)
                  SS_ScrollLineUL(lpSS, SS_F_SCROLL_LEFT | SS_F_MOVEACTIVECELL);
               }

				// RFW - 1/24/05 - 15498
				// If resizing the left most column that is hidden, show it.
				/* RFW - 10/26/05 - 17476
            else if (Col == Col2 && Col < lpSS->Col.UL && Width > 0)
				*/
            else if (Col == Col2 && Col < lpSS->Col.UL && Col >= lpSS->Col.HeaderCnt + lpSS->Col.Frozen && Width > 0)
               {
					SS_BookSetRedraw(lpBook, fRedraw);
               SS_ScrollLineUL(lpSS, SS_F_SCROLL_LEFT);
               }

				if (fFireColWidthChange)
					SS_SendMsgColCoordRange(lpSS, SSM_COLWIDTHCHANGE, GetWindowID(hWnd),
						                     Col, Col2);
				SS_BookSetRedraw(lpBook, fRedraw);
            SS_HighlightCell(lpSS, TRUE);
            }
         }

#ifdef SS_V70
      else if (lpBook->wMode == SS_MODE_TAB)
         {
			RECT Rect;

         ReleaseCapture();
         lpBook->wMode = SS_MODE_NONE;

			SS_GetTabStripRect(lpBook, &Rect);
			SSTab_OnLButtonUp(lpBook->hTabStrip, wParam, lParam, &Rect);
         }

      else if (lpBook->wMode == SS_MODE_TAB_RESIZE)
         {
         ReleaseCapture();
         lpBook->wMode = SS_MODE_NONE;
         }

      else if (lpBook->wMode == SS_MODE_COLMOVE)
         {
         ReleaseCapture();
			SS_HoverEnd(lpSS);

			if (lpSS->ColMoveCurrentCol != lpSS->MoveStartRange.UL.Col &&
             !SS_SendMsgBeforeColMove(lpSS, lpSS->MoveStartRange.UL.Col,
                                      lpSS->MoveStartRange.LR.Col,
                                      lpSS->ColMoveCurrentCol))
				{
				SS_COORD lCols = lpSS->MoveStartRange.LR.Col - lpSS->MoveStartRange.UL.Col + 1;

				lpBook->Redraw = FALSE;
				lpSS->Col.Max += lCols;
				SS_InsColRange(lpSS, lpSS->ColMoveCurrentCol, lpSS->ColMoveCurrentCol + lCols - 1);
				if (lpSS->ColMoveCurrentCol < lpSS->MoveStartRange.UL.Col)
					{
					lpSS->MoveStartRange.UL.Col += lCols;
					lpSS->MoveStartRange.LR.Col += lCols;
					}

				SS_MoveRange(lpSS, lpSS->MoveStartRange.UL.Col, -1, lpSS->MoveStartRange.LR.Col,
                         -1, lpSS->ColMoveCurrentCol, -1);
				SS_DelColRange(lpSS, lpSS->MoveStartRange.UL.Col, lpSS->MoveStartRange.LR.Col);
				lpSS->Col.Max -= lCols;
				lpBook->Redraw = TRUE;
				}

         lpBook->wMode = SS_MODE_NONE;
         SS_HighlightCell(lpSS, TRUE);
         }

      else if (lpBook->wMode == SS_MODE_BEGINROWMOVE)
         {
         ReleaseCapture();
         lpBook->wMode = SS_MODE_NONE;
         SS_HighlightCell(lpSS, TRUE);
			}

      else if (lpBook->wMode == SS_MODE_ROWMOVE)
         {
         ReleaseCapture();
			SS_HoverEnd(lpSS);

			if (lpSS->RowMoveCurrentRow != lpSS->MoveStartRange.UL.Row &&
             !SS_SendMsgBeforeRowMove(lpSS, lpSS->MoveStartRange.UL.Row,
                                      lpSS->MoveStartRange.LR.Row,
                                      lpSS->RowMoveCurrentRow))
				{
				SS_COORD lRows = lpSS->MoveStartRange.LR.Row - lpSS->MoveStartRange.UL.Row + 1;

				lpBook->Redraw = FALSE;
				lpSS->Row.Max += lRows;
				SS_InsRowRange(lpSS, lpSS->RowMoveCurrentRow, lpSS->RowMoveCurrentRow + lRows - 1);
				if (lpSS->RowMoveCurrentRow < lpSS->MoveStartRange.UL.Row)
					{
					lpSS->MoveStartRange.UL.Row += lRows;
					lpSS->MoveStartRange.LR.Row += lRows;
					}

				SS_MoveRange(lpSS, -1, lpSS->MoveStartRange.UL.Row, -1, lpSS->MoveStartRange.LR.Row,
                         -1, lpSS->RowMoveCurrentRow);
				SS_DelRowRange(lpSS, lpSS->MoveStartRange.UL.Row, lpSS->MoveStartRange.LR.Row);
				lpSS->Row.Max -= lRows;
				lpBook->Redraw = TRUE;
				}

         lpBook->wMode = SS_MODE_NONE;
         SS_HighlightCell(lpSS, TRUE);
         }
#endif // SS_V70

#ifndef SS_NODRAGDROP
      else if (lpBook->wMode == SS_MODE_DRAGDROP)
         {
         ReleaseCapture();
         lpBook->wMode = SS_MODE_NONE;

         SS_DragDropFinish(hWnd, lpSS);
         }
#endif

//#ifdef SPREAD_JPN
      //- JPNFIX0008 - (Masanori Iwasa)
      else if (lpSS && (lpBook->wMode == SS_MODE_BLOCK || (lpSS->wOpMode == SS_OPMODE_EXTSEL &&
			      lpBook->wMode != SS_MODE_USERSORT && lpBook->wMode != SS_MODE_BEGINCOLMOVE &&
               max(0, lpSS->BlockCellUL.Row) != lpSS->BlockCellLR.Row)))
		/* RFW - 10/31/05 - 16994
      else if (lpSS && (lpBook->wMode == SS_MODE_BLOCK || (lpSS->wOpMode == SS_OPMODE_EXTSEL &&
			      max(0, lpSS->BlockCellUL.Row) != lpSS->BlockCellLR.Row)))
		*/
/* RFW - 7/29/05 - 16433
      else if (lpSS && (lpBook->wMode == SS_MODE_BLOCK || (lpSS->wOpMode == SS_OPMODE_EXTSEL &&
			      lpSS->BlockCellUL.Row != lpSS->BlockCellLR.Row &&
			      SS_GetCellFromPixel(lpSS, &Col, &Row, NULL, NULL, x, y) &&
               !(Col >= lpSS->Col.HeaderCnt && Row >= 0 && Row < lpSS->Row.HeaderCnt))))
*/
/* RFW - 3/18/04 - 13903
      else if (lpSS && (lpBook->wMode == SS_MODE_BLOCK || lpSS->wOpMode == SS_OPMODE_EXTSEL))
*/
//		  RFW - 12/5/03 - 13092
//      else if (lpSS && (lpBook->wMode == SS_MODE_BLOCK || lpSS->wOpMode == SS_OPMODE_EXTSEL) && 
//               lpSS->BlockCellUL.Row != lpSS->BlockCellLR.Row)
//#else
//      else if (lpBook->wMode == SS_MODE_BLOCK)
//#endif
         {
         ReleaseCapture();
         lpBook->wMode = SS_MODE_NONE;
			/* RFW - 12/5/03 - 13092
         if (lpSS->fMultipleBlocksSelected ||
             lpSS->BlockCellUL.Col != lpSS->BlockCellLR.Col ||
             lpSS->BlockCellUL.Row != lpSS->BlockCellLR.Row)
			*/
         if (lpSS->fMultipleBlocksSelected ||
             lpSS->BlockCellUL.Col != lpSS->BlockCellLR.Col ||
             lpSS->BlockCellUL.Row != lpSS->BlockCellLR.Row ||
             lpSS->BlockCellUL.Col == -1 || lpSS->BlockCellUL.Row == -1)
            {
            if (lpSS->fMultipleBlocksSelected)
               SS_MultiSelBlockAddItem(lpSS, &lpSS->BlockCellUL,
                                       &lpSS->BlockCellLR);

				/* RFW - 12/10/04 - This fix broke code
				if (lpSS->wOpMode != SS_OPMODE_EXTSEL) // RFW - 1/22/04 - 13500
				*/
					SS_SendMsgCommand(lpBook, lpSS, SSN_BLOCKSELECTED, FALSE);
            }
         }

      else if (lpBook->wMode == SS_MODE_SELECT)
         {
         ReleaseCapture();
         lpBook->wMode = SS_MODE_NONE;

         if (lpSS->Row.CurAt != lpSS->Row.PrevAt)
            {
            BOOL fCancel;
            BOOL fFreezeOld = lpSS->FreezeHighlight;
            SS_HighlightCell(lpSS, FALSE);
            lpSS->FreezeHighlight = TRUE;
            fCancel = SS_LeaveCell(lpSS, lpSS->Col.PrevAt, lpSS->Col.PrevUL,
                                   lpSS->Row.PrevAt, lpSS->Row.PrevUL, 0);
            lpSS->FreezeHighlight = fFreezeOld;
            SS_HighlightCell(lpSS, TRUE);
            if( !fCancel )
               {
               SS_ShowActiveCell(lpSS, SS_SHOW_NEAREST);
               SS_InvalidateRect(lpBook, NULL, TRUE);
               }
            }
         }

#ifdef SS_UTP
      else if (lpSS->bModeScrollArrow)
         {
         ReleaseCapture();

         SS_DrawArrow(hWnd, 0, lpSS, lpSS->bModeScrollArrow, FALSE);

         SS_GetClientRect(lpBook, &RectClient);
         if (SS_IsMouseInScrollArrow(hWnd, lpSS, &RectClient,
                                     LOWORD(lParam), HIWORD(lParam),
                                     NULL, lpSS->bModeScrollArrow))
            {
            if (lpSS->bModeScrollArrow == SS_MODE_ARROW_RIGHT)
               SS_ScrollPageLR(hWnd, lpSS, SS_F_SCROLL_RIGHT |
                               SS_F_MOVEACTIVECELL);
            else if (lpSS->bModeScrollArrow == SS_MODE_ARROW_DOWN)
               SS_ScrollPageLR(hWnd, lpSS, SS_F_SCROLL_DOWN |
                               SS_F_MOVEACTIVECELL);
            else if (lpSS->bModeScrollArrow == SS_MODE_ARROW_LEFT)
               SS_ScrollPageUL(hWnd, lpSS, SS_F_SCROLL_LEFT |
                               SS_F_MOVEACTIVECELL);
            else if (lpSS->bModeScrollArrow == SS_MODE_ARROW_UP)
               SS_ScrollPageUL(hWnd, lpSS, SS_F_SCROLL_UP |
                               SS_F_MOVEACTIVECELL);

            SS_SendMsg(hWnd, SSM_ARROWPRESSED, lpSS->bModeScrollArrow, 0L);
            }

         lpSS->bModeScrollArrow = 0;
         }
#endif

      else
         {
         if (GetCapture() == hWnd)
            ReleaseCapture();

#ifdef SS_V70
			/* RFW - 7/29/04 - 14972
         if (lpSS->lpBook->wMode == SS_MODE_USERSORT)
			*/
				{
				short nUserColAction;
				int nIndicator;
				SS_COORD Col, Row;
				int MouseX = LOWORD(lParam);
				int MouseY = HIWORD(lParam);
				int x;
				int y;

				// RFW - 5/13/03 - 11935
				// RFW - 9/23/04 - 15215
				lpBook->wMode = SS_MODE_NONE;

				SS_GetCellFromPixel(lpSS, &Col, &Row, &x, &y, MouseX, MouseY);

				nUserColAction = SS_GetUserColAction(lpSS);
				nIndicator = SS_GetMergedColUserSortIndicator(lpSS, Col);

				if (Col >= lpSS->Col.HeaderCnt && Row >= 0 && 
					 Row < lpSS->Row.HeaderCnt &&
					 (SS_USERCOLACTION_SORT == nUserColAction ||
					  SS_USERCOLACTION_SORTNOINDICATOR == nUserColAction) && 
					 SS_COLUSERSORTINDICATOR_DISABLED != nIndicator)
					{
					SS_BEFOREUSERSORT beforeSort;
					SS_COORD ActiveColOld = lpSS->Col.CurAt;

					beforeSort.lCol = Col;
					beforeSort.lState = nIndicator;
					beforeSort.lDefaultAction = BEFOREUSERSORT_DEFAULTACTION_AUTOSORT;
    
					// RFW - 11/14/05 - In response to a customer complaint that the change event fired after BeforeUserSort
					SS_CellEditModeOff(lpSS, 0);
					SS_AdjustCellCoordsOut(lpSS, &beforeSort.lCol, NULL);
					SS_SendMsg(lpBook, lpSS, SSM_BEFOREUSERSORT, GetWindowID(lpBook->hWnd), 
							 (LPARAM)(LPSS_BEFOREUSERSORT)&beforeSort);

					/* RFW - 2/8/06 - 12436
					SS_SetActiveCell(lpSS, Col, lpSS->Row.HeaderCnt);
					*/
					// RFW - 5/26/09 - 25499
			      SS_ResetBlock(lpSS);

					if (BEFOREUSERSORT_DEFAULTACTION_AUTOSORT == beforeSort.lDefaultAction)
						{
						BOOL fRet;
#ifdef SS_V80
						if (lpSS->sortKeyCnt < SS_SORTKEY_MAX)
						{
						SS_SORTKEY SortKey;
						int keyIndex = -1;
						SortKey.Reference = Col;
						SortKey.wOrder  = nIndicator == SS_COLUSERSORTINDICATOR_ASCENDING ? SS_SORT_DESCENDING : SS_SORT_ASCENDING;
						keyIndex = SS_FindSortKey(lpSS->sortKeys, lpSS->sortKeyCnt, Col);
						if (keyIndex == -1)
							lpSS->sortKeys[lpSS->sortKeyCnt++] = SortKey;
						else
							lpSS->sortKeys[keyIndex] = SortKey;
						}
#else
						SS_SORTKEY SortKeys[1] = {Col, nIndicator == SS_COLUSERSORTINDICATOR_ASCENDING ? SS_SORT_DESCENDING : SS_SORT_ASCENDING};
#endif

						if (lpSS->lpBook->EditModeOn)
							SS_CellEditModeOff(lpSS, 0);
#ifdef SS_V80
						fRet = SS_SortEx(lpSS, SS_ALLCOLS, SS_ALLROWS, SS_ALLCOLS, SS_ALLROWS, SS_SORT_ROW, lpSS->sortKeys, lpSS->sortKeyCnt);
#else
						fRet = SS_SortEx(lpSS, SS_ALLCOLS, SS_ALLROWS, SS_ALLCOLS, SS_ALLROWS, SS_SORT_ROW, SortKeys, 1);
#endif

						/*
						if (lpSS->lpBook->fEditModePermanent)
							SS_CellEditModeOn(lpSS, 0, 0, 0);
						*/

						if (fRet)
							{
							SS_SetColUserSortIndicator(lpSS, -1, SS_COLUSERSORTINDICATOR_NONE);
							SS_SetColUserSortIndicator(lpSS, Col, (short)(nIndicator == SS_COLUSERSORTINDICATOR_ASCENDING ? SS_COLUSERSORTINDICATOR_DESCENDING : SS_COLUSERSORTINDICATOR_ASCENDING));
							}

						// RFW - 06-25-04 - 14515
						SS_SendMsg(lpBook, lpSS, SSM_AFTERUSERSORT, GetWindowID(lpBook->hWnd), (LPARAM)beforeSort.lCol);
						}

					// RFW - 2/20/06 - 18212
					SS_GetFirstValidRow(lpSS, Col, &Row);
					if (SS_IsCellActivatable(lpSS, Col, Row))
						{
						SS_COORD RowTopOld = lpSS->Row.UL;
						SS_COORD ColLeftOld = lpSS->Col.UL;
						SS_COORD RowAtOld = lpSS->Row.CurAt;
						SS_COORD ColAtOld = lpSS->Col.CurAt;

						SS_HighlightCell(lpSS, FALSE);
						lpSS->Col.CurAt = Col;
						lpSS->Row.CurAt = Row;

						if (SS_LeaveCell(lpSS, ColAtOld, ColLeftOld, RowAtOld, RowTopOld, 0))
							{
							SS_HighlightCell(lpSS, TRUE);
							SS_ShowActiveCell(lpSS, SS_SHOW_NEAREST);
							SS_InvalidateRect(lpBook, NULL, TRUE);
							}
						}
					else
						SS_HighlightCell(lpSS, TRUE);
					}
				}

			if (lpBook->wMode == SS_MODE_BEGINCOLMOVE)
				{
				lpBook->wMode = SS_MODE_NONE;
				SS_HighlightCell(lpSS, TRUE);
				}
#endif // SS_V70

         if (lpBook->wMode == SS_MODE_BLOCK)
            lpBook->wMode = SS_MODE_NONE;
         }

      if (lpSS && lpSS->fInvertBlock)
         {
         lpSS->fSuspendInvert = FALSE;
         SS_HighlightCell(lpSS, TRUE);
         SS_InvertBlock(0, lpSS);
         lpSS->fInvertBlock = FALSE;
         }
		}
      break;

   case WM_RBUTTONDOWN:
   case WM_RBUTTONUP:
   case WM_RBUTTONDBLCLK:
		if (!(wParam & MK_LBUTTON)) // RFW - 8/10/05 - 16571
			PostMessage(hWnd, SSM_PROCESSRBUTTON, Msg, lParam);
      break;

#ifdef SS_V80
   case WM_MOUSELEAVE:
	   {
	
	   SS_COORD col,row;
	   short nBtn;
	   int nSheet;
	   BOOL bInvalidateCell = FALSE;

	   if (!lpSS)
		   return (0);

	   col = lpBook->mouseCol;
	   row = lpBook->mouseRow;
	   nBtn = lpBook->mouseTabBtn;
	   nSheet = lpBook->mouseTabSheet;

	   lpBook->mouseCol = -1;
	   lpBook->mouseRow = -1;
	   lpBook->mouseTabBtn = 0;
	   lpBook->mouseTabSheet = -1;
	   lpBook->MouseOver = FALSE;
	   if (nBtn || nSheet != -1)
		SS_InvalidateTabStrip(lpBook);
	   bInvalidateCell = row < lpSS->Row.HeaderCnt && row >= 0 && col >= 0;
	   bInvalidateCell |= col < lpSS->Col.HeaderCnt && col >= 0 && row >= 0;
	   if (bInvalidateCell)
			{
			// RFW - 4/3/09 - 24942
			if (lpBook->wAppearanceStyle == 1)
				lpBook->Redraw = FALSE;

			SS_InvalidateCell(lpSS, col, row);

			// RFW - 4/3/09 - 24942
			if (lpBook->wAppearanceStyle == 1)
				SS_BookSetRedraw(lpBook, TRUE);
			}

	   }
	   break;
#endif

   case WM_MOUSEMOVE:
	   {

#ifdef SS_V80
		long lSpanCols,lSpanRows;
		TRACKMOUSEEVENT tme;
		RECT Rect, RectHit;
		SS_COORD Col,Row, OldCol, OldRow;
		BOOL bSame, isColHeader, isRowHeader, isOldRowHeader, isOldColHeader, bInvalidateCurCell, bInvalidateOldCell;
		short nSheet = -1;
		int nBtn = -1;
		WORD MouseX = LOWORD(lParam);
	    WORD MouseY = HIWORD(lParam);  
#endif
		if (!lpSS)
			return (0);

        if (lpBook->fMsgSentToVB)
// RFW 10/19/97         return (0);
         break;
#ifdef SS_V80
	   if (lpBook->MouseOver == FALSE)
	   {
		 lpBook->MouseOver = TRUE;
		 tme.cbSize = sizeof(TRACKMOUSEEVENT);
		 tme.dwFlags = TME_LEAVE;
		 tme.hwndTrack = hWnd;
		 tme.dwHoverTime = HOVER_DEFAULT;
		 _TrackMouseEvent(&tme);

	   }

		if (lpBook->wAppearanceStyle != 0)
		{
		bInvalidateOldCell = FALSE;
		bInvalidateCurCell = FALSE;

		SS_GetTabStripRect(lpBook, &Rect);
		SSTab_HitTest(lpBook->hTabStrip, MouseX, MouseY, &Rect, &nSheet, &nBtn, &RectHit);
        SS_GetCellFromPixel(lpSS, &Col, &Row, NULL, NULL, MouseX, MouseY);
		if (SS_GetCellSpan(lpSS, Col, Row, NULL, NULL, &lSpanCols, &lSpanRows) == SS_SPAN_YES)
			SS_GetActualCell(lpSS, Col, Row, &Col, &Row, NULL);
			
		bSame = Col == lpBook->mouseCol && Row == lpBook->mouseRow;
		isColHeader = Row < lpSS->Row.HeaderCnt && Row >= 0;
		isRowHeader = Col < lpSS->Col.HeaderCnt && Col >= 0;
		isOldColHeader = lpBook->mouseRow < lpSS->Row.HeaderCnt && lpBook->mouseRow >= 0 && lpBook->mouseCol >= 0;
		isOldRowHeader = lpBook->mouseCol < lpSS->Col.HeaderCnt && lpBook->mouseCol >= 0 && lpBook->mouseRow >= 0;
		OldCol = lpBook->mouseCol;
		OldRow = lpBook->mouseRow;
		bInvalidateOldCell = (isOldColHeader || isOldRowHeader) && !bSame;

		if (Row != lpBook->mouseRow && isRowHeader && Col >= 0 && Row >= 0)
			bInvalidateCurCell = TRUE;
		if (Col != lpBook->mouseCol && isRowHeader && Col >= 0 && Row >= 0)
			bInvalidateCurCell = TRUE;
		if (Col != lpBook->mouseCol && isColHeader && Col >= 0 && Row >= 0)
			bInvalidateCurCell = TRUE;
		if (Row != lpBook->mouseRow && isColHeader && Col >= 0 && Row >= 0)
			bInvalidateCurCell = TRUE;
		if ((isColHeader && isRowHeader) && !(isOldColHeader && isOldRowHeader) && Col >= 0 && Row >= 0)
			bInvalidateCurCell = TRUE;

		if ((nBtn || nSheet != -1) && ((lpBook->mouseCol < 0 || lpBook->mouseRow < 0) || !(isOldRowHeader || isOldColHeader)))
			bInvalidateCurCell = FALSE;
		else if ((Col < 0 || Row < 0) && (lpBook->mouseCol < 0 || lpBook->mouseRow < 0))
			bInvalidateCurCell = FALSE;

		lpBook->mouseRow = Row;
		lpBook->mouseCol = Col;

		if (bInvalidateCurCell)
			{
			// RFW - 4/3/09 - 24942
			if (lpBook->wAppearanceStyle == 1)
				lpBook->Redraw = FALSE;

			SS_InvalidateCell(lpSS, Col, Row);

			// RFW - 4/3/09 - 24942
			if (lpBook->wAppearanceStyle == 1)
				SS_BookSetRedraw(lpBook, TRUE);
			}

		if (bInvalidateOldCell)
			{
			// RFW - 4/3/09 - 24942
			if (lpBook->wAppearanceStyle == 1)
				lpBook->Redraw = FALSE;

			SS_InvalidateCell(lpSS, OldCol, OldRow);

			// RFW - 4/3/09 - 24942
			if (lpBook->wAppearanceStyle == 1)
				SS_BookSetRedraw(lpBook, TRUE);
			}
		
		if (nBtn != lpBook->mouseTabBtn)
		{
		    lpBook->mouseTabBtn = nBtn;
			SS_InvalidateTabStrip(lpBook);
		}
		if (nSheet != lpBook->mouseTabSheet)
		{
		    lpBook->mouseTabSheet = nSheet;
			SS_InvalidateTabStrip(lpBook);
		}
		

		}

#endif
#if 0
      SS_GetClientRect(lpBook, &RectClient);

      MouseX = LOWORD(lParam);
      MouseY = HIWORD(lParam);

         lpSS->hCursorCurrent = SS_CURSOR_ARROW;

         if (SS_IsMouseInHorizResize(lpSS, &Rect, &RectClient,
                                     MouseX, MouseY, NULL) != -1)
            lpSS->hCursorCurrent = SS_CURSOR_DEFROWRESIZE;

         else if (SS_IsMouseInVertResize(lpSS, &Rect, &RectClient,
                                         MouseX, MouseY, NULL) != -1)
            lpSS->hCursorCurrent = SS_CURSOR_DEFCOLRESIZE;

#ifndef SS_NODRAGDROP
         else if (SS_IsMouseInDragDrop(hWnd, lpSS, &RectClient, MouseX,
                                       MouseY, NULL, NULL))
            lpSS->hCursorCurrent = SS_CURSOR_ARROW;
#endif

         else if (SS_IsMouseInGrayArea(lpSS, &RectClient, MouseX, MouseY))
            lpSS->hCursorCurrent = lpSS->hCursorGrayArea;

         else if (lpSS->wOpMode == SS_OPMODE_READONLY ||
                  lpSS->wOpMode == SS_OPMODE_SINGLESEL ||
                  lpSS->wOpMode == SS_OPMODE_MULTISEL ||
                  lpSS->wOpMode == SS_OPMODE_EXTSEL)
            lpSS->hCursorCurrent = SS_CURSOR_ARROW;

         else
            {
            SS_GetCellFromPixel(lpSS, &Col, &Row, NULL, NULL, MouseX, MouseY);

            fMakeTransparent = TRUE;
            if (Col < lpSS->Col.HeaderCnt)
               lpSS->hCursorCurrent = lpSS->hCursorRowHeader;

            else if (Row < lpSS->Row.HeaderCnt)
               lpSS->hCursorCurrent = lpSS->hCursorColHeader;

            else if (SS_GetLock(lpSS, Col, Row, TRUE))
               lpSS->hCursorCurrent = lpSS->hCursorLockedCell;

            else if ((lpSS->RestrictCols && Col > lpSS->Col.DataCnt) ||
                     (lpSS->RestrictRows && Row > lpSS->Row.DataCnt))
               lpSS->hCursorCurrent = lpSS->hCursorLockedCell;

            else if (SS_IsMouseInButton(lpSS, &RectClient, MouseX, MouseY))
               {
               fMakeTransparent = FALSE;
               lpSS->hCursorCurrent = lpSS->hCursorButton;
               }

            else
               {
               lpSS->hCursorCurrent = SS_CURSOR_DEFAULT;
               fMakeTransparent = FALSE;
               }
            }
#endif
	   
#if 0
      hCursor = lpSS->hCursorCurrent;

      switch (hCursor)
         {
         case SS_CURSOR_DEFAULT:
            hCursor = lpSS->hCursorDefault;
            break;

         case SS_CURSOR_ARROW:
            hCursor = LoadCursor(NULL, IDC_ARROW);
            break;

         case SS_CURSOR_DEFCOLRESIZE:
            hCursor = lpSS->hCursorResizeV;
            break;

         case SS_CURSOR_DEFROWRESIZE:
            hCursor = lpSS->hCursorResizeH;
            break;
         }

      switch (hCursor)
         {
         case SS_CURSOR_DEFAULT:
            hCursor = lpSS->hCursorPointer;
            break;

         case SS_CURSOR_ARROW:
            hCursor = LoadCursor(NULL, IDC_ARROW);
            break;

         case SS_CURSOR_DEFCOLRESIZE:
            hCursor = lpSS->hCursorResizeV;
            break;

         case SS_CURSOR_DEFROWRESIZE:
            hCursor = lpSS->hCursorResizeH;
            break;
         }
#ifdef WIN32
      SetClassLong(hWnd, GCL_HCURSOR, (LONG)hCursor);
#else
      SetClassWord(hWnd, GCW_HCURSOR, (WORD)hCursor);
#endif
      SetCursor(hCursor);
#endif

		lpSS->lpBook->fHasMouseMoved = TRUE;
      PostMessage(hWnd, SSM_PROCESSMOUSEMOVE, wParam, lParam);
	  }
      break;

   case WM_SETCURSOR:
		{
		HCURSOR hCursor;

      if ((HWND)wParam != hWnd)
         break;

		if (!lpSS)
			return (0);

      hCursor = lpBook->hCursorCurrent;

	  if (lpBook->bDesignTime)
         hCursor = SS_CURSOR_DEFAULT;

      switch ((LPARAM)hCursor)
         {
         case SS_CURSOR_DEFAULT:
				// This was added to solve a cursor flickering
				// problem with Internet Explorer.
				if (!lpBook->fDontSendParentWMSetCursor && lpBook->fCursorCurrentDef)
					if (SendMessage(GetParent(hWnd), Msg, wParam, lParam))
						return (1);

            hCursor = lpBook->CursorDefault.hCursor;
            break;

         case (LPARAM)SS_CURSOR_ARROW:
				if (!lpBook->fDontSendParentWMSetCursor && lpBook->fCursorCurrentDef)
					if (SendMessage(GetParent(hWnd), Msg, wParam, lParam))
						return (1);
            hCursor = LoadCursor(NULL, IDC_ARROW);
            break;

         case (LPARAM)SS_CURSOR_DEFCOLRESIZE:
				if (!lpBook->fDontSendParentWMSetCursor && lpBook->fCursorCurrentDef)
					if (SendMessage(GetParent(hWnd), Msg, wParam, lParam))
						return (1);
            hCursor = lpBook->hCursorResizeV;
            break;

         case (LPARAM)SS_CURSOR_DEFROWRESIZE:
				if (!lpBook->fDontSendParentWMSetCursor && lpBook->fCursorCurrentDef)
					if (SendMessage(GetParent(hWnd), Msg, wParam, lParam))
						return (1);
            hCursor = lpBook->hCursorResizeH;
            break;
         }

      switch ((LPARAM)hCursor)
         {
         case SS_CURSOR_DEFAULT:
            hCursor = lpBook->hCursorPointer;
            break;

         case (LPARAM)SS_CURSOR_ARROW:
            hCursor = LoadCursor(NULL, IDC_ARROW);
            break;

         case (LPARAM)SS_CURSOR_DEFCOLRESIZE:
            hCursor = lpBook->hCursorResizeV;
            break;

         case (LPARAM)SS_CURSOR_DEFROWRESIZE:
            hCursor = lpBook->hCursorResizeH;
            break;
         }

#if defined(_WIN64) || defined(_IA64)
      SetClassLongPtr(hWnd, GCLP_HCURSOR, (LONG_PTR)hCursor);
#elif WIN32
      SetClassLong(hWnd, GCL_HCURSOR, (LONG)hCursor);
#else
      SetClassWord(hWnd, GCW_HCURSOR, (WORD)hCursor);
#endif

		SetCursor(hCursor);
		}
      return (1);

   case SSM_SETEDITMODE:
		lpBook->fSetEditModeSent = FALSE;
		lpSS = SS_BookLockActiveSheet(lpBook);

		// RFW - 6/1/09 - 25860
		if (lpSS->wOpMode != SS_OPMODE_READONLY &&
			 lpSS->wOpMode != SS_OPMODE_SINGLESEL &&
			 lpSS->wOpMode != SS_OPMODE_MULTISEL &&
			 lpSS->wOpMode != SS_OPMODE_EXTSEL)
			SS_ResetBlock(lpSS);

      if (lParam)
         lpBook->fDontSendEditModeMsg = TRUE;

      if (wParam)
         {
         HWND hWndFocus = GetFocus();

         /* RFW - 4/5/01
         if (!hWndFocus || (hWndFocus != hWnd && GetParent(hWndFocus) != hWnd))
            SetFocus(hWnd);
         */

         if (hWndFocus && (hWndFocus == hWnd || GetParent(hWndFocus) == hWnd
#if SS_V80
            || SS_CT_IsEditorControl(lpSS, hWndFocus)
#endif
            ))
           SS_CellEditModeOn(lpSS, 0, 0, 0L);
         }
      else
         {
         if (SS_CellEditModeOff(lpSS, 0))
            SS_HighlightCell(lpSS, TRUE);
         }

      if (lParam)
         lpBook->fDontSendEditModeMsg = FALSE;

      return (0);

   case SSM_NEXTCELL:
      SS_ResetBlock(lpSS);
      SS_ScrollNextCell(lpSS);
      return (0);

   case SSM_NEXTROW:
      SS_ResetBlock(lpSS);
      SSx_ScrollNextCell(lpSS, FALSE, TRUE);
      return (0);

   case SSM_PREVCELL:
      SS_ResetBlock(lpSS);
      SS_ScrollPrevCell(lpSS);
      return (0);

   case SSM_PROCESSENTERKEY:
      SS_ProcessEnterKey(lpSS);
      return (0);

   case SSM_PROCESSTAB:
		{
		BOOL fMoveActiveCell;
		BOOL fSingleSel;

      if (SS_USESELBAR(lpSS))
         {
         fMoveActiveCell = FALSE;
         fSingleSel = TRUE;

         lpSS->Col.CurAt = lpSS->Col.UL;
         }
      else if (lpSS->wOpMode == SS_OPMODE_READONLY)
         fMoveActiveCell = FALSE;
      else
         fMoveActiveCell = TRUE;

      if (lpBook->fProcessTab && fMoveActiveCell)
         {
         SS_ResetBlock(lpSS);

         if (HIBYTE(GetKeyState(VK_SHIFT)))
            SS_ScrollPrevCell(lpSS);
         else
            SS_ScrollNextCell(lpSS);
         }

      if (SS_IsDestroyed(hWnd))
         return (FALSE);

      SS_UpdateWindow(lpBook);
		}
      return (0);

   case SSM_PROCESSKILLFOCUS:
		{
      HWND hWndFocus = GetFocus();

      if (hWndFocus && (hWndFocus == hWnd || GetParent(hWndFocus) == hWnd))
         return (0);

      lpBook->hWndFocusPrev = 0;

#ifndef SS_NOPROCESSFOCUS
      if (!lpBook->EditModeTurningOn &&
          !lpBook->EditModeTurningOff)
#else
      if (!lpBook->EditModeTurningOn &&
          !lpBook->EditModeTurningOff &&
          !lpBook->wMessageBeingSent)
#endif
         {
         SS_UpdateWindow(lpBook);

         lpBook->fProcessingKillFocus = TRUE;

         // BJO 5Aug96 SEL5713 - Before fix
         //if (wParam == (WPARAM)GetParent(hWnd) && (WPARAM)GetFocus() != wParam)
         // BJO 5Aug96 SEL5713 - Begin fix
         if (wParam == 0)
            wParam = (WPARAM)-1;
         else if (IsChild((HWND)wParam,hWnd) && GetFocus() != (HWND)wParam)
         // BJO 5Aug96 SEL5713 - End fix
            wParam = (WPARAM)GetFocus();

         // BJO 10Jul97 GIL742 - Before fix
         //SS_CellEditModeOff(lpSS, (HWND)wParam);
         // BJO 10Jul97 GIL742 - Begin fix
         SS_CellEditModeOff(lpSS, (HWND)-1);
         // BJO 10Jul97 GIL742 - End fix

         if (SS_IsDestroyed(hWnd))
            return (FALSE);

         lpBook->fProcessingKillFocus = FALSE;

#if 0
         if (!lpBook->fRetainSelBlock && !SS_USESELBAR(lpSS))
            {
            SS_DrawSelBlock(hWnd, 0, lpSS);
            lpSS->fSelBlockInvisible = TRUE;
            }
#endif
         //if ((HWND)wParam != hWnd)
         if (wParam == -1 || ((HWND)wParam != hWnd && GetParent((HWND)wParam) != hWnd)) // BJO 26Mar96 TEL1844
            SS_LeaveCell(lpSS, -1, -1, -1, -1, TRUE);

         if (SS_IsDestroyed(hWnd))
            return (FALSE);

         /*
         if (!lpBook->fRetainSelBlock ||
             !SS_IsBlockSelected(lpSS))
            {
            if (SS_USESELBAR(lpSS))
               {
               lpBook->fProcessingKillFocus = TRUE;
               SS_HighlightCell(lpSS, FALSE);
               lpBook->fProcessingKillFocus = FALSE;
               }
            else
               SS_HighlightCell(lpSS, FALSE);
            }
         */

         if (SS_USESELBAR(lpSS))
            {
            lpBook->fProcessingKillFocus = TRUE;
            SS_HighlightCell(lpSS, FALSE);
            lpBook->fProcessingKillFocus = FALSE;
            }

         else if (!lpBook->fRetainSelBlock)
		 {
            SS_HighlightCell(lpSS, FALSE);
#ifdef SS_V80
			SS_InvalidateActiveHeaders(lpSS);
#endif
		 }

			// RFW - 7/1/04 - 14576
			else
            SS_HighlightCell(lpSS, TRUE);

         SS_SendMsgCommand(lpBook, lpSS, SSN_KILLFOCUS, FALSE);

#ifndef SS_NOPROCESSFOCUS
         lpBook->fKillFocusReceived = TRUE;
#endif

         lpBook->fFocusOn = FALSE;
         }
		}
      return (0);

   case SSM_PROCESSSETFOCUS:
      if (!lpBook->EditModeTurningOn &&
          !lpBook->EditModeTurningOff &&
          !lpBook->wMessageBeingSent)
         {
			BOOL fNoHighlight;
         BOOL fButtonDown = FALSE;

         if (lpBook->fProcessingLButtonDown)
            fButtonDown = TRUE;

         if (fButtonDown)
            {
				RECT  Rect;
				POINT Point;

            GetCursorPos(&Point);
            GetWindowRect(hWnd, &Rect);

            if (!PtInRect(&Rect, Point))
               fButtonDown = FALSE;
            }

         fNoHighlight = FALSE;

         if (!lpBook->fFocusOn &&
             !lpBook->fMoveActiveOnFocus && fButtonDown)
            lpBook->fNoProcessButtonDown = TRUE;

         else if (!lpBook->fFocusOn &&
                  lpBook->fMoveActiveOnFocus && fButtonDown)
            fNoHighlight = TRUE;

         if (SS_IsBlockSelected(lpSS))
            {
            if (!lpBook->fRetainSelBlock)
               {
               /*
               SS_DrawSelBlock(hWnd, 0, lpSS);
               lpSS->fSelBlockInvisible = FALSE;
               if (lpBook->fEditModePermanent)
                  SS_HighlightCell(lpSS, TRUE);
               */
               if (lpBook->fSelBlockInvisible)
               {
                  SS_DrawSelBlock(0, lpSS);
                  lpBook->fSelBlockInvisible = FALSE;  // BJO 21Feb97 GRB5555 - Added
               }

               SS_HighlightCell(lpSS, TRUE);
               }
            }
         else if (!fNoHighlight)
            {
            if (SS_USESELBAR(lpSS))
               {
					// RFW - 6/3/04 - 14418
					if (lpBook->fInvalidated)
						SS_UpdateWindow(lpBook);

               if (lpBook->fKillFocusReceived)
                  {
                  BOOL fHighlightOnOld = lpSS->HighlightOn;

                  if (lpSS->HighlightOn)
                     {
                     lpSS->HighlightOn = FALSE;
                     lpBook->fProcessingKillFocus = TRUE;
                     }

                  lpBook->fKillFocusReceived = FALSE;
	 					// RFW - 12/10/03
						if (!GetUpdateRect(hWnd, NULL, FALSE))
							SS_HighlightCell(lpSS, TRUE);

                  if (fHighlightOnOld)
                     {
                     lpSS->HighlightOn = TRUE;
                     lpBook->fProcessingKillFocus = FALSE;
                     }
                  }
               }
#if (!defined(SS_VB))
            else if (lpBook->EditModeOn && !lpBook->fEditModePermanent)
               ;
#endif
            else
					{
	            if (lpBook->fInvalidated)
	               SS_UpdateWindow(lpBook);

               SS_HighlightCell(lpSS, TRUE);
					}
            }

         if (!IsWindow(hWnd))
            return (0);

         SS_SendMsgCommand(lpBook, lpSS, SSN_SETFOCUS, FALSE);

         lpBook->fFocusOn = TRUE;
         lpBook->fSelBlockInvisible = FALSE;  // RFW - 2/4/04 - 15658
         }

      else if (lpBook->wMessageBeingSent)
         lpBook->fSetFocusWhileMsgBeingSent = TRUE;

      return (0);

   case SSM_PROCESSLBUTTONDOWN:
   case SSM_PROCESSLBUTTONDBLCLK:
      return (SS_ProcessLButton(lpSS, hWnd, Msg, wParam, lParam));

   case SSM_PROCESSMOUSEMOVE:
      if (lpBook->wMode == SS_MODE_RESIZE_HORIZ || lpBook->wMode == SS_MODE_RESIZE_VERT)
         SS_Resize(hWnd, lpSS, SS_RESIZEMODE_UPDATE, lParam);

#ifdef SS_V70
      else if (lpBook->wMode == SS_MODE_TAB)
         {
			RECT Rect;

			SS_GetTabStripRect(lpBook, &Rect);
			SSTab_OnMouseMove(lpBook->hTabStrip, wParam, lParam, &Rect);
         }

      else if (lpBook->wMode == SS_MODE_TAB_RESIZE)
			SS_TabResize(lpSS, (short)LOWORD(lParam));

      else if (lpBook->wMode == SS_MODE_BEGINCOLMOVE ||
               lpBook->wMode == SS_MODE_COLMOVE)
			SS_ColMoving(lpSS, (int)LOWORD(lParam), (int)HIWORD(lParam));

      else if (lpBook->wMode == SS_MODE_BEGINROWMOVE ||
               lpBook->wMode == SS_MODE_ROWMOVE)
			SS_RowMoving(lpSS, (int)LOWORD(lParam), (int)HIWORD(lParam));
#endif // SS_V70

      else if (wParam & MK_LBUTTON && ((lpBook->wMode == SS_MODE_BLOCK) ||
               lpBook->wMode == SS_MODE_SELECT || lpBook->wMode == SS_MODE_DRAGDROP
#ifdef SS_UTP
               || lpSS->bModeScrollArrow
#endif
               ))
         {
			RECT  RectClient;
			POINT PointClient;
			POINT Point;
         int   iSwapButton;

#ifdef SS_UTP
         BOOL fScrollArrowDown = TRUE;
         BOOL fScrollArrowDownTemp;
#endif

         SS_GetClientRect(lpBook, &RectClient);

         PointClient.x = 0;
         PointClient.y = 0;

         ClientToScreen(hWnd, &PointClient);

/*
         dwTime = GetTickCount();
         while (dwTime + SS_BLOCKMODE_INITIALDELAY > GetTickCount())
            ;
*/
         iSwapButton = GetSystemMetrics(SM_SWAPBUTTON);

			while (lpBook->wMode != SS_MODE_NONE)
            {
            if (iSwapButton)
               {
               if ((GetAsyncKeyState(VK_RBUTTON) & 0x8000) == 0)
                  break;
               }
            else
               if ((GetAsyncKeyState(VK_LBUTTON) & 0x8000) == 0)
                  break;

            GetCursorPos(&Point);

            Point.x -= PointClient.x;
            Point.y -= PointClient.y;

            if (lpBook->wMode == SS_MODE_DRAGDROP)
               {
#ifndef SS_NODRAGDROP
               SS_CELLCOORD BlockCellLR;
               SS_CELLCOORD NewPos;

               BlockCellLR.Col = lpBook->DragDropCurrentCol;
               BlockCellLR.Row = lpBook->DragDropCurrentRow;
               if (SS_ScrollWithMouse(hWnd, lpSS, &BlockCellLR,
                                      &BlockCellLR, &NewPos, Point.x,
                                      Point.y))
                  {
                  if (lpSS->DragDropBlockUL.Col +
                      (NewPos.Col - lpBook->DragDropCurrentCol) >=
                      lpSS->Col.HeaderCnt &&
                      lpSS->DragDropBlockUL.Col +
                      (SS_IsBlockSelected(lpSS) ?
                       lpSS->BlockCellLR.Col -
                       lpSS->BlockCellUL.Col : 0) +
                      (NewPos.Col - lpBook->DragDropCurrentCol) < SS_GetColCnt(lpSS))
                     lpSS->DragDropBlockUL.Col +=
                        NewPos.Col - lpBook->DragDropCurrentCol;

                  if (lpSS->DragDropBlockUL.Row +
                      (NewPos.Row - lpBook->DragDropCurrentRow) >=
                      lpSS->Row.HeaderCnt &&
                      lpSS->DragDropBlockUL.Row +
                      (SS_IsBlockSelected(lpSS) ?
                       lpSS->BlockCellLR.Row -
                       lpSS->BlockCellUL.Row : 0) +
                      (NewPos.Row - lpBook->DragDropCurrentRow) < SS_GetRowCnt(lpSS))
                     lpSS->DragDropBlockUL.Row +=
                        NewPos.Row - lpBook->DragDropCurrentRow;

                  lpBook->DragDropCurrentCol = NewPos.Col;
                  lpBook->DragDropCurrentRow = NewPos.Row;

						UpdateWindow(lpSS->lpBook->hWnd); // RFW - 12/30/05 - 17902
                  SS_DragDropDrawBox(hWnd, lpSS, TRUE);
                  }
#endif
               }

#ifdef SS_UTP
            else if (lpSS->bModeScrollArrow)
               {
               fScrollArrowDownTemp = SS_IsMouseInScrollArrow(hWnd,
                                      lpSS, &RectClient, Point.x,
                                      Point.y, NULL,
                                      lpSS->bModeScrollArrow);

               if (fScrollArrowDown != fScrollArrowDownTemp)
                  {
                  fScrollArrowDown = fScrollArrowDownTemp;
                  SS_DrawArrow(hWnd, 0, lpSS,
                               lpSS->bModeScrollArrow,
                               fScrollArrowDown);
                  }
               }
#endif

            else if (SS_USESELBAR(lpSS))
               {
#ifndef SS_NOLISTBOX
               if (lpSS->fSingleSelRowFound)
                  {
                  SS_CELLCOORD BlockCellLR;
                  SS_CELLCOORD NewPos;

                  BlockCellLR.Col = lpSS->Col.CurAt;
                  BlockCellLR.Row = lpSS->Row.CurAt;
                  if (SS_ScrollWithMouse(hWnd, lpSS, &BlockCellLR,
                                         &BlockCellLR, &NewPos,
                                         Point.x, Point.y))
                     if (NewPos.Row != lpSS->Row.CurAt)
                        {
                        if (lpSS->wOpMode == SS_OPMODE_EXTSEL)
                           SSx_SelectRow(hWnd, lpSS, NewPos.Row);

                        SS_HighlightCell(lpSS, FALSE);
                        lpSS->Row.CurAt = NewPos.Row;
                        SS_HighlightCell(lpSS, TRUE);
                        UpdateWindow(hWnd);
                        }
                  }
#endif
               }

            else
					{
					if (!lpBook->EditModeOn) // RFW - 10/15/02 - 10999
						SS_SelectBlock(hWnd, lpSS, Point.x, Point.y);
					}

            {
            MSG Msg;

            if (PeekMessage(&Msg, NULL, 0, 0, PM_NOREMOVE))
               if (Msg.message == WM_MOUSEMOVE)
                  break;
               else if (PeekMessage(&Msg, NULL, 0, 0, PM_REMOVE))
                  {
                  TranslateMessage(&Msg);
                  DispatchMessage(&Msg);
                  }
            }
            }

         if (lpSS->wOpMode == SS_OPMODE_EXTSEL)
            {
            LPSS_ROW lpRowTemp;
            SS_COORD i;

            for (i = lpSS->BlockCellUL.Row;
                 i <= lpSS->BlockCellLR.Row; i++)
               if (lpRowTemp = SS_LockRowItem(lpSS, i))
                  {
                  if (lpRowTemp->fRowSelected == 3)
                     lpRowTemp->fRowSelected = TRUE;
                  else if (lpRowTemp->fRowSelected == 2)
                     lpRowTemp->fRowSelected = FALSE;

                  SS_UnlockRowItem(lpSS, i);
                  }
            }
         }

      return (0);

   case SSM_PROCESSRBUTTON:
		{
		SS_RBUTTON RButton;
		int        x;
		int        y;

      Msg = (UINT)wParam;

      if (Msg == WM_RBUTTONUP)
         {
         if (!lpBook->fRButtonDown)
            return (0);

         lpBook->fRButtonDown = FALSE;
         }
      else
         lpBook->fRButtonDown = TRUE;

      if (Msg == WM_RBUTTONUP)
         RButton.RButtonType = RBUTTONTYPE_UP;
      else if (Msg == WM_RBUTTONDOWN)
         RButton.RButtonType = RBUTTONTYPE_DOWN;
      else if (Msg == WM_RBUTTONDBLCLK)
         RButton.RButtonType = RBUTTONTYPE_DBLCLK;

      RButton.xPos = LOWORD(lParam);
      RButton.yPos = HIWORD(lParam);

      SS_GetCellFromPixel(lpSS, &RButton.Col, &RButton.Row, &x, &y,
                          RButton.xPos, RButton.yPos);

      if (RButton.Col != -2 && RButton.Row != -2)
         {
			SS_AdjustCellCoordsOut(lpSS, &RButton.Col, &RButton.Row);
         SS_SendMsg(lpBook, lpSS, SSM_RBUTTON, GetWindowID(hWnd),
                    (LPARAM)(LPSS_RBUTTON)&RButton);
         }
		}
      return (0);

#ifdef SS_OCX
   case SSM_SETCONTROLSIZE:
      {
      FPCONTROL fpControl = {0, 0, 0L, 0L};
      LPDISPATCH lpdispExtendedCtrl;
      lpdispExtendedCtrl = SS_GetExtendedControl(lpBook);
      fpControl.lObject = (long)lpBook->lpOleControl;
      fpControl.lpDispatch = (long)lpdispExtendedCtrl;
      //SendMessage(lpBook->hWnd, WM_SETREDRAW, FALSE, 0L);
      // this function sets the control size 
      SSOCXSetControlSize((LPFPCONTROL)&fpControl, wParam, lParam);    
      //SendMessage(lpBook->hWnd, WM_SETREDRAW, TRUE, 0L);
      if (lpdispExtendedCtrl)
         lpdispExtendedCtrl->lpVtbl->Release(lpdispExtendedCtrl);
      }
      return (0);
#endif

   case WM_COMMAND:
      HANDLE_WM_COMMAND(hWnd, wParam, lParam, SS_OnCommand);
      break;

   case WM_ERASEBKGND:
#if 0
      if (lpSS->NoEraseBkgnd)
         {
         lpSS->NoEraseBkgnd = FALSE;
         return (TRUE);
         }

      SS_GetClientRect(lpBook, &Rect);

      if (lpSS->Col.LRAllVis == SS_GetColCnt(lpSS) - 1)
         Rect.right = min(Rect.right, Rect.left + SS_GetCellPosX(lpSS,
                      lpSS->Col.UL, SS_GetColCnt(lpSS) - 1) +
                      SS_GetColWidthInPixels(lpSS,
                      SS_GetColCnt(lpSS) - 1) + 1);

      if (lpSS->Row.LRAllVis == SS_GetRowCnt(lpSS) - 1)
         Rect.bottom = min(Rect.bottom, Rect.top + SS_GetCellPosY(lpSS,
                       lpSS->Row.UL, SS_GetRowCnt(lpSS) - 1) +
                       SS_GetRowHeightInPixels(lpSS,
                       SS_GetRowCnt(lpSS) - 1) + 1);

      SS_GetColorItem(&BackColorTblItem,
                      lpSS->Color.BackgroundId);


      if (SS_USESINGLESELBAR(lpSS) && GetFocus() != hWnd)
         SSx_HighlightCell(lpSS, (HDC)wParam, FALSE);
      else
         SS_HighlightCell(lpSS, FALSE);

      FillRect((HDC)wParam, &Rect, BackColorTblItem.hBrush);
#endif

      return (TRUE);

   /********************
   * Paint Spreadsheet
   ********************/

   case WM_PAINT:
		if (!SS_Paint(lpSS, (HDC)wParam))
			return (FALSE);

      break;

#ifdef FP_ADB
// fix for #8720 -scl
   case SSM_SETACTIVECELLINTERNAL:
	   // rfw - 7/18/01 - #9124
       if ((SS_COORD)wParam != lpSS->Col.CurAt || (SS_COORD)lParam != lpSS->Row.CurAt)
	      SS_SetActiveCell(lpSS, wParam, lParam);
     break;
#endif

#if (WINVER >= 0x0400)
   // New to Windows 95
   case WM_PRINT:
		{
         HDC hdc = (HDC)wParam; 
         ULONG uFlags = (ULONG)lParam; 
         FPNCINFO fpNCInfo;
		 int nBorderSize = 0;
		 POINT ptOrgOld;
		 SIZE extOrg;
		 BOOL bNC = (uFlags & (ULONG)PRF_NONCLIENT && (!(uFlags & (ULONG)PRF_CHECKVISIBLE) || IsWindowVisible(hWnd)));

#ifdef SS_V80

         _fmemset(&fpNCInfo, '\0', sizeof(fpNCInfo));
         if (bNC)
		 {
			 NCCALCSIZE_PARAMS csparams = {0};
			 POINT ptOrg;
			 SIZE ext;
#ifndef SS_DLL
         if (GetWindowLong(hWnd, GWL_STYLE) & WS_BORDER)
#endif
            {
            fpNCInfo.colorBk = RGBCOLOR_BLACK;

			switch( lpBook->bAppearance )
			{
			case SS_APPEARANCE_FLAT:
				nBorderSize = 1;
				break;
			case SS_APPEARANCE_3D:
				nBorderSize = 2;
				break;
			case SS_APPEARANCE_3DWITHBORDER:
				nBorderSize = 3;
			}
            if (lpBook->bAppearance == SS_APPEARANCE_FLAT ||
                lpBook->bAppearance == SS_APPEARANCE_3DWITHBORDER)
               {
               fpNCInfo.lStyle |= FP_NCBORDER;
               fpNCInfo.bPenStyleOnFocus = PS_SOLID;
               fpNCInfo.bPenStyleNoFocus = PS_SOLID;
               fpNCInfo.colorBorder = GetSysColor(COLOR_WINDOWFRAME);
               fpNCInfo.bBorderSize = 1;
               }

            if (lpBook->bAppearance == SS_APPEARANCE_3D ||
                lpBook->bAppearance == SS_APPEARANCE_3DWITHBORDER)
               {
               fpNCInfo.lStyle |= (FP_NC3DINLOWER | FP_NC3DOUTLOWER);
               fpNCInfo.bOutShadowWidth = 1;
               fpNCInfo.color3DOutShadow = GetSysColor(COLOR_3DSHADOW);
               fpNCInfo.color3DOutHiLite = GetSysColor(COLOR_3DHILIGHT);
               fpNCInfo.bInShadowWidth = 1;
               fpNCInfo.color3DInShadow = GetSysColor(COLOR_3DDKSHADOW);
               fpNCInfo.color3DInHiLite = GetSysColor(COLOR_3DLIGHT);
               }
            }
			fpWMNCCalcSize(hWnd, &fpNCInfo, FALSE, (LPNCCALCSIZE_PARAMS)&csparams);
			//fpWMNCPaint(hWnd, &fpNCInfo, (short)0xFFFF, hdc);
			GetViewportOrgEx(hdc, &ptOrg);
			ptOrg.x += nBorderSize;
			ptOrg.y += nBorderSize;
			SetViewportOrgEx(hdc, ptOrg.x, ptOrg.y, &ptOrgOld);
			GetViewportExtEx(hdc, &ext);
			ext.cx -= nBorderSize*2;
			ext.cy -= nBorderSize*2;
			SetViewportExtEx(hdc, ext.cx, ext.cy, &extOrg);
		 }

         if (uFlags & (ULONG)PRF_CLIENT && (!(uFlags & (ULONG)PRF_CHECKVISIBLE) || IsWindowVisible(hWnd)))
         {
            HDC hdctemp;
            HBITMAP hbmp, hbmpold;
            RECT rc;
            int i;
            HWND sbs[2];

            
            SS_Paint(lpSS, hdc);
            if( !lpSS->lpBook->fWmPrintSkipScrollBars )
            {  // paint scrollbars
               HDC hdcSpread = GetDC(lpSS->lpBook->hWnd);
               
               sbs[0] = lpSS->lpBook->hWndHScroll;
               sbs[1] = lpSS->lpBook->hWndVScroll;
               for( i = 0; i < 2; i++ )
               {
				  // fix for 99914165 - don't draw if not visible -scl
				  if( (i == 0 && !SS_GetBool(lpSS->lpBook, lpSS, SSB_HORZSCROLLBAR)) ||
					  (i == 1 && !SS_GetBool(lpSS->lpBook, lpSS, SSB_VERTSCROLLBAR)) )
					  continue;
                  GetWindowRect(sbs[i], &rc);
                  hdctemp = CreateCompatibleDC(hdcSpread);
                  hbmp = CreateCompatibleBitmap(hdcSpread, rc.right - rc.left, rc.bottom - rc.top);
                  hbmpold = SelectObject(hdctemp, hbmp);
                  SendMessage(sbs[i], WM_PRINT, (WPARAM)hdctemp, lParam);
                  MapWindowPoints(NULL, hWnd, (LPPOINT)&rc, 2);
                  BitBlt(hdc, rc.left, rc.top, rc.right - rc.left, rc.bottom - rc.top, hdctemp, 0, 0, SRCCOPY);
                  SelectObject(hdctemp, hbmpold);
                  DeleteObject(hbmp);
                  DeleteDC(hdctemp);
               }
               ReleaseDC(lpSS->lpBook->hWnd, hdcSpread);
            }
         }
         if (bNC)
		 {
			SetViewportOrgEx(hdc, ptOrgOld.x, ptOrgOld.y, NULL);
			SetViewportExtEx(hdc, extOrg.cx, extOrg.cy, NULL);
			fpWMNCPaint(hWnd, &fpNCInfo, (short)0xFFFF, hdc);
		 }

#else
	      if (uFlags & (ULONG)PRF_CLIENT)
				if (!SS_Paint(lpSS, hdc))
	            return (FALSE);

			break;
#endif // SS_V80

      }
      return 0;
#endif

   case WM_DESTROY:
      HANDLE_WM_DESTROY(hWnd, wParam, lParam, SS_OnDestroy);
      break;

   case WM_NCDESTROY:
      HANDLE_WM_NCDESTROY(hWnd, wParam, lParam, SS_OnDestroy);
      break;

#ifdef SS_V30
   case PRVWM_OPEN:
      {
      LPPRVW_OPEN lpPrvwOpen = (LPPRVW_OPEN)lParam;

      lpPrvwOpen->fAllowMarginResize = FALSE;
      }
      return (TRUE);

   case PRVWM_CLOSE:
      return (TRUE);

#ifdef SS_DLL
   case PRVWM_GETINFO:
      SS_PrintPreviewGetInfo(lpSS, (LPPRVW_INFO)lParam);
      return (TRUE);

   case PRVWM_PRINTPAGE:
      SS_PrintPreviewPrintPage(lpSS, (LPPRVW_PAGE)lParam);
      return (TRUE);

   case PRVWM_SETMARGINS:
      return (TRUE);
#endif

	case SSM_UNHIDEACTIVECOL:
		{
		LPSPREADSHEET lpSSTemp = SS_BookLockSheetIndex(lpBook, (short)wParam);
		SS_COORD Col = (SS_COORD)lParam;

		SS_UnhideActiveCol(lpSSTemp, Col);
		SS_BookUnlockSheetIndex(lpBook, (short)wParam);
		}
		return (0);

	case SSM_UNHIDEACTIVEROW:
		{
		LPSPREADSHEET lpSSTemp = SS_BookLockSheetIndex(lpBook, (short)wParam);
		SS_COORD Row = (SS_COORD)lParam;

		SS_UnhideActiveRow(lpSSTemp, Row);
		SS_BookUnlockSheetIndex(lpBook, (short)wParam);
		}
		return (0);

#if SS_V80
   case SSM_RESUBCLASSEDITOR:
      if( lpSS->lpBook->EditModeOn )
      {
         LPSS_CELLTYPE CellType = SS_CT_LockCellType(lpSS, lpSS->Col.EditAt, lpSS->Row.EditAt);
         if( CellType && CellType->Type == SS_TYPE_CUSTOM )
            SS_CT_SubclassEditor(lpSS, CellType);
      }

#endif
   }
#endif

#ifdef SS_NOSUPERCLASS
return (VBDefControlProc(VBGetHwndControl(hWnd), hWnd, Msg, wParam, lParam));
#else
return (DefWindowProc(hWnd, Msg, wParam, lParam));
#endif
}


static int SS_IsMouseInHorizResize(lpSS, Rect, RectClient, MouseX,
                                   MouseY, Row)

LPSPREADSHEET lpSS;
LPRECT        Rect;
LPRECT        RectClient;
int           MouseX;
int           MouseY;
LPSS_COORD    Row;
{
LPSS_ROW      lpRow;
int           dResize;
int           x;
int           y;
SS_COORD      RowAt;
int           Height;
int           PosY = -1;

x = SS_GetCellPosX(lpSS, 0, lpSS->Col.HeaderCnt);

if (MouseX < RectClient->left + x)
   {
   y = -1;
   RowAt = 0;

   for (; RowAt < SS_GetRowCnt(lpSS) &&
        y < RectClient->bottom + RESIZE_OFFSET; )
      {
      Height = SS_GetRowHeightInPixels(lpSS, RowAt);

      if (y + Height >= RectClient->bottom + RESIZE_OFFSET)
         break;

      if (MouseY >= RectClient->top + y + Height - RESIZE_OFFSET &&
          MouseY <= RectClient->top + y + Height + RESIZE_OFFSET)
         {
         dResize = SS_RESIZE_DEFAULT;

         if (lpRow = SS_LockRowItem(lpSS, RowAt))
            {
            if (lpRow->fRowHidden)
               dResize = SS_RESIZE_OFF;
            else
               dResize = lpRow->bUserResize;

            SS_UnlockRowItem(lpSS, RowAt);
            }

         if ((!(lpSS->wUserResize & SS_USERRESIZE_ROW) &&
             dResize == SS_RESIZE_DEFAULT) || dResize == SS_RESIZE_OFF)
            ;
         else
            {
            if (Row)
               *Row = RowAt;

            PosY = max(0, y + Height);
            break;
            }
         }

      y += Height;

      if (RowAt == lpSS->Row.HeaderCnt + lpSS->Row.Frozen - 1)
			// RFW - 7/23/03
			// This was added to allow resizing of 0 width columns.
         //RowAt = max(lpSS->Row.UL, lpSS->Row.HeaderCnt + lpSS->Row.Frozen);

			if (lpSS->Row.UL > lpSS->Row.HeaderCnt + lpSS->Row.Frozen)
				{
				RowAt = lpSS->Row.UL;
			   if (SS_GetRowHeightInPixels(lpSS, RowAt - 1) == 0)
					RowAt--;
				}
			else
				RowAt = lpSS->Row.HeaderCnt + lpSS->Row.Frozen;
      else
         RowAt++;
      }
   }

return (PosY);
}


static int SS_IsMouseInVertResize(lpSS, Rect, RectClient, MouseX,
                                  MouseY, Col)

LPSPREADSHEET lpSS;
LPRECT        Rect;
LPRECT        RectClient;
int           MouseX;
int           MouseY;
LPSS_COORD    Col;
{
LPSS_COL      lpCol;
int           dResize;
int           x;
int           y;
SS_COORD      ColAt;
int           Width;
int           PosX = -1;

y = SS_GetCellPosY(lpSS, 0, lpSS->Row.HeaderCnt);

if (MouseY < RectClient->top + y)
   {
   x = -1;
   ColAt = 0;

   for (; ColAt < SS_GetColCnt(lpSS) &&
        x < RectClient->right + RESIZE_OFFSET; )
      {
      Width = SS_GetColWidthInPixels(lpSS, ColAt);

      if (x + Width >= RectClient->right + RESIZE_OFFSET)
         break;

      if (MouseX >= RectClient->left + x + Width - RESIZE_OFFSET &&
          MouseX <= RectClient->left + x + Width + RESIZE_OFFSET)
         {
         dResize = SS_RESIZE_DEFAULT;

         if (lpCol = SS_LockColItem(lpSS, ColAt))
            {
            if (lpCol->fColHidden)
               dResize = SS_RESIZE_OFF;
            else
               dResize = lpCol->bUserResize;

            SS_UnlockColItem(lpSS, ColAt);
            }

         if ((!(lpSS->wUserResize & SS_USERRESIZE_COL) &&
             dResize == SS_RESIZE_DEFAULT) || dResize == SS_RESIZE_OFF)
            ;
         else
            {
            if (Col)
               *Col = ColAt;

            PosX = max(0, x + Width);
            break;
            }
         }

      x += Width;

      if (ColAt == lpSS->Col.HeaderCnt + lpSS->Col.Frozen - 1)
			// RFW - 7/23/03
			// This was added to allow resizing of 0 width columns.
         //ColAt = max(lpSS->Col.UL, lpSS->Col.HeaderCnt + lpSS->Col.Frozen);

			if (lpSS->Col.UL > lpSS->Col.HeaderCnt + lpSS->Col.Frozen)
				{
				ColAt = lpSS->Col.UL;
			   if (SS_GetColWidthInPixels(lpSS, ColAt - 1) == 0)
					ColAt--;
				}
			else
				ColAt = lpSS->Col.HeaderCnt + lpSS->Col.Frozen;
      else
         ColAt++;
      }
   }

return (PosX);
}


static BOOL SS_IsMouseInButton(lpSS, RectClient, MouseX, MouseY)

LPSPREADSHEET lpSS;
LPRECT        RectClient;
int           MouseX;
int           MouseY;
{
SS_CELLTYPE   CellTypeTemp;
SS_COORD      Col;
SS_COORD      Row;
BOOL          Ret = FALSE;
int           x;
int           y;
SS_COORD      ColAt;
SS_COORD      RowAt;
int           Width;
int           Height;
RECT          RectCell;

Col = -1;
Row = -1;

x = SS_GetCellPosX(lpSS, 0, lpSS->Col.HeaderCnt);

if (lpSS->Col.Frozen)
   ColAt = lpSS->Col.HeaderCnt;
else
   ColAt = lpSS->Col.UL;

for (; ColAt < SS_GetColCnt(lpSS) && x < RectClient->right; )
   {
   Width = SS_GetColWidthInPixels(lpSS, ColAt);

   if (MouseX >= RectClient->left + x + 1)
      {
      Col = -2;
      if (MouseX < RectClient->left + x + Width - 1)
         {
         Col = ColAt;
         break;
         }
      }

   x += Width;

   if (lpSS->Col.Frozen && ColAt == lpSS->Col.HeaderCnt + lpSS->Col.Frozen - 1)
      ColAt = max(lpSS->Col.UL, lpSS->Col.HeaderCnt + lpSS->Col.Frozen);
   else
      ColAt++;
   }

y = SS_GetCellPosY(lpSS, 0, lpSS->Row.HeaderCnt);

if (lpSS->Row.Frozen)
   RowAt = lpSS->Row.HeaderCnt;
else
   RowAt = lpSS->Row.UL;

for (; RowAt < SS_GetRowCnt(lpSS) && y < RectClient->bottom; )
   {
   Height = SS_GetRowHeightInPixels(lpSS, RowAt);

   if (MouseY >= RectClient->top + y)
      {
      Row = -2;
      if (MouseY < RectClient->top + y + Height - 1)
         {
         Row = RowAt;
         break;
         }
      }

   y += Height;

   if (lpSS->Row.Frozen && RowAt == lpSS->Row.HeaderCnt + lpSS->Row.Frozen - 1)
      RowAt = max(lpSS->Row.UL, lpSS->Row.HeaderCnt + lpSS->Row.Frozen);
   else
      RowAt++;
   }

if (Col >= 0 && Row >= 0)
	{
	SS_GetActualCell(lpSS, Col, Row, &Col, &Row, &RectCell);

	if (Col >= lpSS->Col.HeaderCnt && Row >= lpSS->Row.HeaderCnt)
		if (SS_RetrieveCellType(lpSS, &CellTypeTemp, NULL, Col, Row))
			if (CellTypeTemp.Type == SS_TYPE_BUTTON ||
				 CellTypeTemp.Type == SS_TYPE_CHECKBOX ||
				 (CellTypeTemp.Type == SS_TYPE_COMBOBOX &&
				 MouseX >= RectCell.right - lpSS->lpBook->dComboButtonBitmapWidth))
				Ret = TRUE;
	}

return (Ret);
}

#if SS_V80
static BOOL SS_IsMouseInReservedLocation(lpSS, RectClient, MouseX, MouseY, lpCursor)

LPSPREADSHEET lpSS;
LPRECT        RectClient;
int           MouseX;
int           MouseY;
HCURSOR       *lpCursor;
{
SS_CELLTYPE   CellTypeTemp;
SS_COORD      Col;
SS_COORD      Row;
BOOL          Ret = FALSE;
int           x;
int           y;
SS_COORD      ColAt;
SS_COORD      RowAt;
int           Width;
int           Height;
RECT          RectCell;

Col = -1;
Row = -1;

x = SS_GetCellPosX(lpSS, 0, lpSS->Col.HeaderCnt);

if (lpSS->Col.Frozen)
   ColAt = lpSS->Col.HeaderCnt;
else
   ColAt = lpSS->Col.UL;

for (; ColAt < SS_GetColCnt(lpSS) && x < RectClient->right; )
   {
   Width = SS_GetColWidthInPixels(lpSS, ColAt);

   if (MouseX >= RectClient->left + x + 1)
      {
      Col = -2;
      if (MouseX < RectClient->left + x + Width - 1)
         {
         Col = ColAt;
         break;
         }
      }

   x += Width;

   if (lpSS->Col.Frozen && ColAt == lpSS->Col.HeaderCnt + lpSS->Col.Frozen - 1)
      ColAt = max(lpSS->Col.UL, lpSS->Col.HeaderCnt + lpSS->Col.Frozen);
   else
      ColAt++;
   }

y = SS_GetCellPosY(lpSS, 0, lpSS->Row.HeaderCnt);

if (lpSS->Row.Frozen)
   RowAt = lpSS->Row.HeaderCnt;
else
   RowAt = lpSS->Row.UL;

for (; RowAt < SS_GetRowCnt(lpSS) && y < RectClient->bottom; )
   {
   Height = SS_GetRowHeightInPixels(lpSS, RowAt);

   if (MouseY >= RectClient->top + y)
      {
      Row = -2;
      if (MouseY < RectClient->top + y + Height - 1)
         {
         Row = RowAt;
         break;
         }
      }

   y += Height;

   if (lpSS->Row.Frozen && RowAt == lpSS->Row.HeaderCnt + lpSS->Row.Frozen - 1)
      RowAt = max(lpSS->Row.UL, lpSS->Row.HeaderCnt + lpSS->Row.Frozen);
   else
      RowAt++;
   }

if (Col >= 0 && Row >= 0)
	{
	SS_GetActualCell(lpSS, Col, Row, &Col, &Row, &RectCell);

	if (Col >= lpSS->Col.HeaderCnt && Row >= lpSS->Row.HeaderCnt)
		if (SS_RetrieveCellType(lpSS, &CellTypeTemp, NULL, Col, Row))
			if (CellTypeTemp.Type == SS_TYPE_CUSTOM &&
         SS_CT_IsReservedLocation(lpSS, &CellTypeTemp, &RectCell, Col, Row, MouseX, MouseY, lpCursor))
				Ret = TRUE;
	}

return (Ret);
}
#endif

static BOOL SS_IsMouseInGrayArea(lpSS, RectClient, MouseX, MouseY)

LPSPREADSHEET lpSS;
LPRECT        RectClient;
int           MouseX;
int           MouseY;
{
int           x;
int           y;

if (lpSS->Col.LR == SS_GetColCnt(lpSS) - 1)
   {
   x = SS_GetCellPosX(lpSS, lpSS->Col.UL, lpSS->Col.LR + 1);

   if (x < RectClient->right && MouseX >= x)
      return (TRUE);
   }

if (lpSS->Row.LR == SS_GetRowCnt(lpSS) - 1)
   {
   y = SS_GetCellPosY(lpSS, lpSS->Row.UL, lpSS->Row.LR + 1);

   if (y < RectClient->bottom && MouseY >= y)
      return (TRUE);
   }

return (FALSE);
}

#ifdef SS_V35
static BOOL SS_IsMouseInCellNote(LPSPREADSHEET lpSS, SS_COORD Col, SS_COORD Row, int MouseX, int MouseY)
{
#ifdef SS_V40
SS_SELBLOCK Block;
#endif // SS_V40
RECT        rcCellNote;

if (Col < lpSS->Col.HeaderCnt || Row < lpSS->Row.HeaderCnt)
   return FALSE;

// RFW - 5/25/04 - 14468
#ifdef SS_V40
if (SS_MergeCalcBlock(lpSS, Row, NULL, Col, NULL, &Block))
	{
	if (Col != Block.LR.Col || Row != Block.UL.Row)
		return (FALSE);
	}
#endif // SS_V40

// RFW - 7/21/03 - 12271
SS_GetActualCell(lpSS, Col, Row, &Col, &Row, &rcCellNote);

if (SS_GetCellNote(lpSS, Col, Row, NULL))
   {
   int xSize = MulDiv(45, dxPixelsPerInch, 1440);
   int ySize = MulDiv(45, dyPixelsPerInch, 1440);
   int nHitTestMargin = 4;

//   SS_GetCellRect(lpSS, Col, Row, &rcCellNote);
   rcCellNote.top += 1;
   rcCellNote.bottom = rcCellNote.top + ySize + nHitTestMargin;
   rcCellNote.right -= 1;
   rcCellNote.left = rcCellNote.right - xSize - nHitTestMargin;

   if ( MouseX >= rcCellNote.left && MouseX <= rcCellNote.right &&
        MouseY >= rcCellNote.top && MouseY <= rcCellNote.bottom )
   return(TRUE);
   }

return (FALSE);
}
#endif

static void SS_Resize(hWnd, lpSS, Mode, Pos)

HWND          hWnd;
LPSPREADSHEET lpSS;
short         Mode;
LPARAM        Pos;
{
HDC           hDC;
RECT          RectClient;
int           MaxX;
int           MaxY;
int           NewPos;
int           CurPos;

#ifdef SS_UTP
GetClientRect(hWnd, &RectClient);
#else
SS_GetClientRect(lpSS->lpBook, &RectClient);
#endif

hDC = SS_GetDC(lpSS->lpBook);

if (lpSS->lpBook->wMode == SS_MODE_RESIZE_HORIZ)
   {
   NewPos = (short)HIWORD(Pos) - lpSS->lpBook->ResizeOffset;
//   y = SS_GetCellPosY(lpSS, 0, lpSS->Row.HeaderCnt);
//   NewPos = max(NewPos, y);
   NewPos = min(NewPos, RectClient.bottom - 1);
   }
else
   {
   NewPos = (short)LOWORD(Pos) - lpSS->lpBook->ResizeOffset;
//   x = SS_GetCellPosX(lpSS, 0, lpSS->Col.HeaderCnt);
//   NewPos = max(NewPos, x);
   NewPos = min(NewPos, RectClient.right - 1);
   }

NewPos = max(NewPos, lpSS->lpBook->ResizeMinPos);

if (lpSS->lpBook->wMode == SS_MODE_RESIZE_HORIZ)
   MaxX = min(SS_GetCellPosX(lpSS, lpSS->Col.UL,
              lpSS->Col.LR) + SS_GetColWidthInPixels(lpSS,
              lpSS->Col.LR), RectClient.right);
else
   MaxY = min(SS_GetCellPosY(lpSS, lpSS->Row.UL,
              lpSS->Row.LR) + SS_GetRowHeightInPixels(lpSS,
              lpSS->Row.LR), RectClient.bottom);

if (Mode != SS_RESIZEMODE_START)
   {
   CurPos = lpSS->lpBook->ResizeCurrentPos;

   if (lpSS->lpBook->wMode == SS_MODE_RESIZE_HORIZ)
      PatBlt(hDC, 0, CurPos, MaxX, 1, DSTINVERT);
   else
      PatBlt(hDC, CurPos, 0, 1, MaxY, DSTINVERT);
   }

if (Mode != SS_RESIZEMODE_END)
   {
   if (lpSS->lpBook->wMode == SS_MODE_RESIZE_HORIZ)
      PatBlt(hDC, 0, NewPos, MaxX, 1, DSTINVERT);
   else
      PatBlt(hDC, NewPos, 0, 1, MaxY, DSTINVERT);
   }

SS_ReleaseDC(lpSS->lpBook, hDC);

lpSS->lpBook->ResizeCurrentPos = NewPos;
}

#define SS_CALCNEXTSCROLLTICKCOUNT(dwTickCount, iDistance) max((dwTickCount), (dwTickCount) + 300 - 25 * ((iDistance) / 2));

static BOOL SS_ScrollWithMouse(hWnd, lpSS, lpBlockCellLR, lpCurrentPos,
                               lpNewPos, MouseX, MouseY)

HWND           hWnd;
LPSPREADSHEET  lpSS;
LPSS_CELLCOORD lpBlockCellLR;
LPSS_CELLCOORD lpCurrentPos;
LPSS_CELLCOORD lpNewPos;
int            MouseX;
int            MouseY;
{
RECT           RectClient;
SS_COORD       RowAt;
SS_COORD       ColAt;
SS_COORD       Row = -1;
SS_COORD       Col = -1;
short          Width;
short          Height;
short          Left;
short          Top;
short          x;
short          y;
#ifdef SS_V40
DWORD          dwTickCount;
#endif // SS_V40

#ifdef SS_UTP
{
short dScrollArrowWidthOld = lpSS->dScrollArrowWidth;
short dScrollArrowHeightOld = lpSS->dScrollArrowHeight;

lpSS->dScrollArrowWidth = SS_SCROLLARROW_WIDTH;
lpSS->dScrollArrowHeight = SS_SCROLLARROW_HEIGHT;

SS_GetClientRect(lpSS->lpBook, &RectClient);

lpSS->dScrollArrowWidth = dScrollArrowWidthOld;
lpSS->dScrollArrowHeight = dScrollArrowHeightOld;
}
#else
SS_GetClientRect(lpSS->lpBook, &RectClient);
#endif

x = SS_GetCellPosX(lpSS, 0, lpSS->Col.HeaderCnt);
Left = x;

if (lpBlockCellLR->Col != -1 && MouseX >= RectClient.left &&
    MouseX < RectClient.right)
   {
   if (lpSS->Col.Frozen)
      ColAt = lpSS->Col.HeaderCnt;
   else
      ColAt = lpSS->Col.UL;

   for (; ColAt < SS_GetColCnt(lpSS) && x < RectClient.right; )
      {
      Width = SS_GetColWidthInPixels(lpSS, ColAt);

      if (MouseX >= RectClient.left + x &&
          MouseX < RectClient.left + x + Width)
         {
         Col = ColAt;
         break;
         }

      x += Width;

      if (lpSS->Col.Frozen && ColAt == lpSS->Col.HeaderCnt +
          lpSS->Col.Frozen - 1)
         ColAt = max(lpSS->Col.UL, lpSS->Col.HeaderCnt + lpSS->Col.Frozen);
      else
         ColAt++;
      }
   }

y = SS_GetCellPosY(lpSS, 0, lpSS->Row.HeaderCnt);
Top = y;

if (lpBlockCellLR->Row != -1 && MouseY >= RectClient.top &&
    MouseY < RectClient.bottom)
   {
   if (lpSS->Row.Frozen)
      RowAt = lpSS->Row.HeaderCnt;
   else
      RowAt = lpSS->Row.UL;

   for (; RowAt < SS_GetRowCnt(lpSS) && y < RectClient.bottom; )
      {
      Height = SS_GetRowHeightInPixels(lpSS, RowAt);

      if (MouseY >= RectClient.top + y && MouseY < RectClient.top + y + Height)
         {
         Row = RowAt;
         break;
         }

      y += Height;

      if (lpSS->Row.Frozen && RowAt == lpSS->Row.HeaderCnt +
          lpSS->Row.Frozen - 1)
         RowAt = max(lpSS->Row.UL, lpSS->Row.HeaderCnt + lpSS->Row.Frozen);
      else
         RowAt++;
      }
   }

#ifdef SS_V40
// scroll the spread with speed proportional to the distance from the border
dwTickCount = GetTickCount();
if (lpSS->lpBook->dwNextScrollTickCount > dwTickCount)
	return (FALSE);
#endif // SS_V40

if (lpSS->lpBook->wMode == SS_MODE_DRAGDROP)
   {
#ifndef SS_NODRAGDROP
   if (Col == lpSS->lpBook->DragDropCurrentCol &&
       Row == lpSS->lpBook->DragDropCurrentRow)
      return (FALSE);
   else
      SS_DragDropDrawBox(hWnd, lpSS, FALSE);
#endif
   }

#ifdef SS_V70
else if (lpSS->lpBook->wMode == SS_MODE_COLMOVE ||
         lpSS->lpBook->wMode == SS_MODE_ROWMOVE)
	;
#endif // SS_V70

else if (Col == lpCurrentPos->Col && Row == lpCurrentPos->Row)
   return (FALSE);

lpSS->Col.ULPrev = lpSS->Col.UL;
lpSS->Row.ULPrev = lpSS->Row.UL;

if (Col == -1 && lpBlockCellLR->Col != -1)
   {
   if (MouseX <= Left)
      {
		SS_COORD ColUL = lpSS->Col.UL;

      SS_ScrollLineUL(lpSS, SS_F_SCROLL_LEFT);

      if (lpSS->Col.Frozen)
         Col = lpSS->Col.HeaderCnt;
      else
         Col = lpSS->Col.UL;

#ifdef SS_V40
		if (ColUL != lpSS->Col.UL)
	      lpSS->lpBook->dwNextScrollTickCount = SS_CALCNEXTSCROLLTICKCOUNT(dwTickCount, Left - MouseX);
#endif // SS_V40
      }
   else
      {
		SS_COORD ColUL = lpSS->Col.UL;

      SS_ScrollLineLR(lpSS, SS_F_SCROLL_RIGHT);
      Col = lpSS->Col.LR;

		// Do not include empty columns
      for (; Col > lpSS->Col.HeaderCnt && SS_GetColWidthInPixels(lpSS, Col) == 0; Col--)
			;
#ifdef SS_V40
		if (ColUL != lpSS->Col.UL)
	      lpSS->lpBook->dwNextScrollTickCount = SS_CALCNEXTSCROLLTICKCOUNT(dwTickCount, MouseX - RectClient.right);
#endif // SS_V40
      }
   }

else if (Col == -1 && lpBlockCellLR->Col == -1)
   Col = 0;

if (Row == -1 && lpBlockCellLR->Row != -1)
   {
   if (MouseY <= Top)
      {
		SS_COORD RowUL = lpSS->Row.UL;

      SS_ScrollLineUL(lpSS, SS_F_SCROLL_UP);

      if (lpSS->Row.Frozen)
         Row = lpSS->Row.HeaderCnt;
      else
         Row = lpSS->Row.UL;

#ifdef SS_V40
		if (RowUL != lpSS->Row.UL)
	      lpSS->lpBook->dwNextScrollTickCount = SS_CALCNEXTSCROLLTICKCOUNT(dwTickCount, Top - MouseY);
#endif // SS_V40
      }
   else
      {
		SS_COORD RowUL = lpSS->Row.UL;

      SS_ScrollLineLR(lpSS, SS_F_SCROLL_DOWN);
      Row = lpSS->Row.LR;

		// Do not include empty rows
      for (; Row > lpSS->Row.HeaderCnt && SS_GetRowHeightInPixels(lpSS, Row) == 0; Row--)
			;

#ifdef SS_V40
		if (RowUL != lpSS->Row.UL)
			lpSS->lpBook->dwNextScrollTickCount = SS_CALCNEXTSCROLLTICKCOUNT(dwTickCount, MouseY - RectClient.bottom);
#endif // SS_V40
      }
   }

else if (Row == -1 && lpBlockCellLR->Row == -1)
   Row = 0;

lpNewPos->Col = Col;
lpNewPos->Row = Row;

return (TRUE);
}


static void SS_SelectBlock(hWnd, lpSS, MouseX, MouseY)

HWND          hWnd;
LPSPREADSHEET lpSS;
int           MouseX;
int           MouseY;
{
SS_CELLCOORD  NewPos;
SS_CELLCOORD  CurrentPos;

CurrentPos.Col = lpSS->Col.BlockCellCurrentPos;
CurrentPos.Row = lpSS->Row.BlockCellCurrentPos;

if (SS_ScrollWithMouse(hWnd, lpSS, &lpSS->BlockCellLR, &CurrentPos,
                       &NewPos, MouseX, MouseY))
   SSx_SelectBlock(lpSS, NewPos.Col, NewPos.Row);
}


void SSx_SelectBlock(lpSS, Col, Row)

LPSPREADSHEET lpSS;
SS_COORD      Col;
SS_COORD      Row;
{
HDC           hDC;
SS_SELBLOCK   BlockRectNew;
SS_SELBLOCK   BlockRectOld;
SS_SELBLOCK   BlockRectUnion;
SS_CELLCOORD  BlockCellUL;
SS_CELLCOORD  BlockCellLR;
RECT          RectOld;
RECT          Rect;
HRGN          hRgn;
HRGN          hRgn2;

if ((Col == lpSS->Col.BlockCellCurrentPos &&
    Row == lpSS->Row.BlockCellCurrentPos) || lpSS->wOpMode == SS_OPMODE_EXTSEL)
   return;

#ifdef SS_V80
	SS_InvalidateActiveHeaders(lpSS);
#endif // SS_V80

BlockCellUL.Col = min(Col, lpSS->Col.CurAt);
BlockCellUL.Row = min(Row, lpSS->Row.CurAt);

if (Col != -1 && Col >= lpSS->Col.HeaderCnt)
   BlockCellLR.Col = max(Col, lpSS->Col.CurAt);
else
   BlockCellLR.Col = -1;

if (Row != -1 && Row >= lpSS->Row.HeaderCnt)
   BlockCellLR.Row = max(Row, lpSS->Row.CurAt);
else
   BlockCellLR.Row = -1;

// RFW - 5/14/03 - 11927
if ((!(SS_SELBLOCKOPTION(lpSS) & SS_SELBLOCK_BLOCKS) &&
       (BlockCellUL.Col >= lpSS->Col.HeaderCnt && BlockCellUL.Row >= lpSS->Row.HeaderCnt)) ||
    (!(SS_SELBLOCKOPTION(lpSS) & SS_SELBLOCK_COLS) &&
       (BlockCellUL.Col >= lpSS->Col.HeaderCnt && BlockCellUL.Row < lpSS->Row.HeaderCnt)) ||
    (!(SS_SELBLOCKOPTION(lpSS) & SS_SELBLOCK_ROWS) &&
       (BlockCellUL.Row >= lpSS->Row.HeaderCnt && BlockCellUL.Col < lpSS->Col.HeaderCnt)) ||
    (!(SS_SELBLOCKOPTION(lpSS) & SS_SELBLOCK_ALL) &&
       (BlockCellUL.Row < lpSS->Row.HeaderCnt && BlockCellUL.Col < lpSS->Col.HeaderCnt)))
	return;

SS_SetBlock(&BlockRectOld, lpSS->BlockCellUL.Col,
            lpSS->BlockCellUL.Row, lpSS->BlockCellLR.Col,
            lpSS->BlockCellLR.Row);

if (SS_IsActiveSheet(lpSS))
	{
	if (!SS_CreateBlockRect(lpSS, &RectOld,
									lpSS->BlockCellUL.Col,
									lpSS->BlockCellUL.Row,
									lpSS->BlockCellLR.Col,
									lpSS->BlockCellLR.Row, FALSE))
		SetRectEmpty(&RectOld);

	hRgn = CreateRectRgnIndirect(&RectOld);

	#ifndef SS_NODRAGDROP
	if (lpSS->BlockCellUL.Col != lpSS->Col.CurAt ||
		 lpSS->BlockCellUL.Row != lpSS->Row.CurAt ||
		 lpSS->BlockCellLR.Col != lpSS->Col.CurAt ||
		 lpSS->BlockCellLR.Row != lpSS->Row.CurAt)
		if (lpSS->lpBook->fAllowDragDrop && !lpSS->fMultipleBlocksSelected)
			SS_AddDragDropRectToBlock(lpSS, hRgn);
	#endif
	}

lpSS->BlockCellUL = BlockCellUL;
lpSS->BlockCellLR = BlockCellLR;

if (SS_IsActiveSheet(lpSS))
	{
	hDC = SS_GetDC(lpSS->lpBook);

	if (SS_CreateBlockRect(lpSS, &Rect, lpSS->BlockCellUL.Col,
								  lpSS->BlockCellUL.Row, lpSS->BlockCellLR.Col,
								  lpSS->BlockCellLR.Row, FALSE))
		{
		hRgn2 = CreateRectRgnIndirect(&Rect);

	#ifndef SS_NODRAGDROP
		if (lpSS->lpBook->fAllowDragDrop && !lpSS->fMultipleBlocksSelected)
			SS_AddDragDropRectToBlock(lpSS, hRgn2);
	#endif

		CombineRgn(hRgn, hRgn, hRgn2, RGN_XOR);
		DeleteObject(hRgn2);
		}

	#ifdef SS_UTP
		if (lpSS->fUseScrollArrows)
			{
			if (SS_CreateBlockRect(lpSS, &Rect, lpSS->Col.HeaderCnt +
										 lpSS->Col.Frozen - 1, 0, lpSS->Col.HeaderCnt +
										 lpSS->Col.Frozen - 1, -1, FALSE))
				{
				hRgn2 = CreateRectRgnIndirect(&Rect);
				CombineRgn(hRgn, hRgn, hRgn2, RGN_DIFF);
				DeleteObject(hRgn2);
				}
			}

	#endif

	if (lpSS->fMultipleBlocksSelected)
		{
		SS_SetBlock(&BlockRectNew, lpSS->BlockCellUL.Col,
						lpSS->BlockCellUL.Row, lpSS->BlockCellLR.Col,
						lpSS->BlockCellLR.Row);

		/*
		SS_UnionBlock(&BlockRectUnion, &BlockRectNew, &BlockRectOld);
		*/

		if (BlockRectNew.UL.Col <= BlockRectOld.LR.Col &&
			 BlockRectNew.LR.Col >= BlockRectOld.UL.Col &&
			 BlockRectNew.UL.Row <= BlockRectOld.LR.Row &&
			 BlockRectNew.LR.Row >= BlockRectOld.UL.Row)
			{
			BlockRectUnion.UL.Col = min(BlockRectNew.UL.Col, BlockRectOld.UL.Col);
			BlockRectUnion.UL.Row = min(BlockRectNew.UL.Row, BlockRectOld.UL.Row);
			BlockRectUnion.LR.Col = max(BlockRectNew.LR.Col, BlockRectOld.LR.Col);
			BlockRectUnion.LR.Row = max(BlockRectNew.LR.Row, BlockRectOld.LR.Row);
			}
		else
			_fmemcpy(&BlockRectUnion, &BlockRectNew, sizeof(SS_SELBLOCK));

		if (SS_CreateBlockRect(lpSS, &Rect, BlockRectUnion.UL.Col,
									  BlockRectUnion.UL.Row, BlockRectUnion.LR.Col,
									  BlockRectUnion.LR.Row, FALSE))
			// RFW - 5/12/03 - 11941
			// If the keyboard is used to extend the last selection then we need
			// to subtract one from the cnt.
			hRgn = SSxx_SelectBlock(lpSS, hRgn, &Rect, (short)(lpSS->fKeyDownBeingPerformed ?
						lpSS->MultiSelBlock.dItemCnt - 1 : lpSS->MultiSelBlock.dItemCnt));
		}

	/*
	#ifdef SS_UTP
	{
	RECT Rect;
	HRGN hRgnClip;

	GetClientRect(hWnd, &Rect);
	hRgnClip = CreateRectRgnIndirect(&Rect);

	ClipWndsRgn(hWnd, hRgnClip, TRUE);
	CombineRgn(hRgn, hRgn, hRgnClip, RGN_AND);

	DeleteObject(hRgnClip);
	}
	#endif
	*/

	#ifndef SS_UTP
	SelectClipRgn(hDC, hRgn);
	#endif

	if ((lpSS->lpBook->hWnd && GetFocus() == lpSS->lpBook->hWnd) || lpSS->lpBook->fRetainSelBlock)
		{
		if (SS_INVERTSEL(lpSS))
			{
	#ifndef SS_UTP
			SelectClipRgn(hDC, hRgn);
	#endif
         InvertRgn(hDC, hRgn);
			}
		else
			{
			RECT RectTemp;
			BOOL fFreezeHighlight = lpSS->FreezeHighlight;

			SS_GetCellRect(lpSS, lpSS->Col.CurAt, lpSS->Row.CurAt, &RectTemp);
			InflateRect(&RectTemp, 2, 2);
			if (!RectInRegion(hRgn, &RectTemp))
				lpSS->FreezeHighlight = TRUE;
			InvalidateRgn(lpSS->lpBook->hWnd, hRgn, FALSE);
			UpdateWindow(lpSS->lpBook->hWnd);
			lpSS->FreezeHighlight = fFreezeHighlight;
			}
		}

	DeleteObject(hRgn);
	SelectClipRgn(hDC, 0);

	SS_ReleaseDC(lpSS->lpBook, hDC);
	}

if (lpSS->Row.BlockCellCurrentPos != Row ||
    lpSS->Col.BlockCellCurrentPos != Col)
   {
   lpSS->Row.BlockCellCurrentPos = Row;
   lpSS->Col.BlockCellCurrentPos = Col;

   SS_SendMsgCommand(lpSS->lpBook, lpSS, SSN_BLOCKCHANGED, FALSE);

#ifdef SS_V80
	SS_InvalidateActiveHeaders(lpSS);
#endif // SS_V80

   }
}

#ifdef SS_V80

int  SS_FindSortKey(SS_SORTKEY keys[] , int keyCnt, SS_COORD ref)
{
	int i;
	for ( i=0; i < keyCnt; i++)
		if (ref == keys[i].Reference)
			return i;
	return -1;
}
void SS_InvalidateColHeaders(LPSPREADSHEET lpSS, SS_COORD ColLeft, SS_COORD ColRight)
{
if (lpSS->wOpMode == 2 || ColLeft == -1)
	ColLeft = 0;

if (lpSS->wOpMode == 2 || ColRight == -1)
	ColRight = lpSS->Col.Max;

SS_InvalidateCellRange(lpSS, ColLeft, 0, ColRight, lpSS->Row.HeaderCnt-1);	
}


void SS_InvalidateRowHeaders(LPSPREADSHEET lpSS, SS_COORD RowTop, SS_COORD RowBottom)
{
if (RowTop == -1)
	RowTop = 0;

if (RowBottom == -1)
	RowBottom = lpSS->Row.Max;

SS_InvalidateCellRange(lpSS, 0, RowTop, lpSS->Col.HeaderCnt-1, RowBottom);
}


void SS_InvalidateActiveColHeaders(LPSPREADSHEET lpSS)
{

if (lpSS->MultiSelBlock.dItemCnt > 0)
   {
	LPSS_SELBLOCK lpItemList;
	SS_COORD      i;

   lpItemList = (LPSS_SELBLOCK)GlobalLock(lpSS->MultiSelBlock.hItemList);

   for (i = 0; i < lpSS->MultiSelBlock.dItemCnt; i++)
		SS_InvalidateColHeaders(lpSS, lpItemList[i].UL.Col, lpItemList[i].LR.Col);

   GlobalUnlock(lpSS->MultiSelBlock.hItemList);
	SS_InvalidateColHeaders(lpSS, lpSS->BlockCellUL.Col, lpSS->BlockCellLR.Col);
   }

else if (SS_IsBlockSelected(lpSS))
	SS_InvalidateColHeaders(lpSS, lpSS->BlockCellUL.Col, lpSS->BlockCellLR.Col);

else
	SS_InvalidateColHeaders(lpSS, lpSS->Col.CurAt, lpSS->Col.CurAt);
}


void SS_InvalidateActiveRowHeaders(LPSPREADSHEET lpSS)
{

if (lpSS->MultiSelBlock.dItemCnt > 0)
   {
	LPSS_SELBLOCK lpItemList;
	SS_COORD      i;

   lpItemList = (LPSS_SELBLOCK)GlobalLock(lpSS->MultiSelBlock.hItemList);

   for (i = 0; i < lpSS->MultiSelBlock.dItemCnt; i++)
		SS_InvalidateRowHeaders(lpSS, lpItemList[i].UL.Row, lpItemList[i].LR.Row);

   GlobalUnlock(lpSS->MultiSelBlock.hItemList);
	SS_InvalidateRowHeaders(lpSS, lpSS->BlockCellUL.Row, lpSS->BlockCellLR.Row);
   }

else if (SS_IsBlockSelected(lpSS))
	SS_InvalidateRowHeaders(lpSS, lpSS->BlockCellUL.Row, lpSS->BlockCellLR.Row);

else
	SS_InvalidateRowHeaders(lpSS, lpSS->Row.CurAt, lpSS->Row.CurAt);
}


void SS_InvalidateActiveHeaders(LPSPREADSHEET lpSS)
{
if (SS_SEL_HIGHLIGHTHEADERS(lpSS))
	{
	BOOL fRedraw = lpSS->lpBook->Redraw;
	lpSS->lpBook->Redraw = FALSE;

	SS_InvalidateActiveColHeaders(lpSS);
	SS_InvalidateActiveRowHeaders(lpSS);

	lpSS->lpBook->Redraw = fRedraw;
	}
}
#endif // SS_V80


HRGN SSxx_SelectBlock(lpSS, hRgnMain, lpRectMain, dItemCnt)

LPSPREADSHEET lpSS;
HRGN          hRgnMain;
LPRECT        lpRectMain;
short         dItemCnt;
{
LPSS_SELBLOCK lpItemList;
RECT          Rect;
HRGN          hRgn2;
HRGN          hRgnPrev = 0;
short         i;

if (lpSS->fMultipleBlocksSelected)
   {
   lpItemList = (LPSS_SELBLOCK)GlobalLock(lpSS->MultiSelBlock.hItemList);

   for (i = 0; i < dItemCnt; i++)
      {
      if (SS_CreateBlockRect(lpSS, &Rect, lpItemList[i].UL.Col,
                             lpItemList[i].UL.Row, lpItemList[i].LR.Col,
                             lpItemList[i].LR.Row, FALSE))
         {
/*
         if (IntersectRect(&Rect, lpRectMain, &Rect))
*/
            {
            if (!hRgnPrev)
               hRgnPrev = CreateRectRgnIndirect(&Rect);

            else
               {
               hRgn2 = CreateRectRgnIndirect(&Rect);
               CombineRgn(hRgnPrev, hRgnPrev, hRgn2, RGN_OR);
               DeleteObject(hRgn2);
               }
            }
         }
      }

   GlobalUnlock(lpSS->MultiSelBlock.hItemList);

   if (hRgnPrev)
      {
      HRGN hRgnDest;

      hRgnDest = CreateRectRgn(0, 0, 0, 0);
      CombineRgn(hRgnDest, hRgnMain, hRgnPrev, RGN_DIFF);
      DeleteObject(hRgnPrev);
		DeleteObject(hRgnMain);
		hRgnMain = hRgnDest;
      }
   }

return (hRgnMain);
}


BOOL SS_IsBlockSelected(lpSS)

LPSPREADSHEET lpSS;
{
/*
if (!lpSS->fMultipleBlocksSelected &&
    ((lpSS->BlockCellUL.Col == -1 &&
      lpSS->BlockCellUL.Row == -1) ||
     (lpSS->BlockCellUL.Col == lpSS->BlockCellLR.Col &&
      lpSS->BlockCellUL.Row == lpSS->BlockCellLR.Row)))
   return (FALSE);

if (!lpSS->fMultipleBlocksSelected &&
    ((lpSS->BlockCellUL.Col == -1 &&
      lpSS->BlockCellUL.Row == -1)))
   return (FALSE);
*/

if (SS_USESELBAR(lpSS))
   return (FALSE);

if (!lpSS->fMultipleBlocksSelected &&
    (
    (lpSS->BlockCellUL.Col == -1 &&
     lpSS->BlockCellUL.Row == -1) ||
     (lpSS->BlockCellUL.Col == lpSS->BlockCellLR.Col &&
      lpSS->BlockCellUL.Row == lpSS->BlockCellLR.Row &&
      lpSS->BlockCellUL.Row != -1 && lpSS->BlockCellUL.Col != -1)))
   return (FALSE);

return (TRUE);
}


BOOL SS_IsAllSelected(LPSPREADSHEET lpSS)
{
if (SS_IsBlockSelected(lpSS) &&
    lpSS->BlockCellUL.Col == 0 && lpSS->BlockCellLR.Col == -1 &&
    lpSS->BlockCellUL.Row == 0 && lpSS->BlockCellLR.Row == -1)
   return (TRUE);

return (FALSE);
}


BOOL SS_IsRowsSelected(LPSPREADSHEET lpSS)
{
if (SS_IsBlockSelected(lpSS) &&
    lpSS->BlockCellUL.Col == 0 && lpSS->BlockCellLR.Col == -1 &&
    lpSS->BlockCellUL.Row > 0)
   return (TRUE);

return (FALSE);
}


BOOL SS_IsColsSelected(LPSPREADSHEET lpSS)
{
if (SS_IsBlockSelected(lpSS) &&
    lpSS->BlockCellUL.Row == 0 && lpSS->BlockCellLR.Row == -1 &&
    lpSS->BlockCellUL.Col > 0)
   return (TRUE);

return (FALSE);
}


BOOL SS_ResetBlock(LPSPREADSHEET lpSS)
{
return (SSx_ResetBlock(lpSS, TRUE));
}


BOOL SSx_ResetBlock(LPSPREADSHEET lpSS, BOOL fDraw)
{
BOOL fBlockSelected = FALSE;
BOOL fRet = FALSE;

#ifdef SS_V80
if (lpSS->fMultipleBlocksSelected || SS_IsBlockSelected(lpSS))
	SS_InvalidateActiveHeaders(lpSS);
#endif // SS_V80

#ifndef SS_NOLISTBOX

if (lpSS->wOpMode == SS_OPMODE_EXTSEL)
   {
	if (SS_IsF8ExtSelMode(lpSS))
		return (TRUE);

   SS_SelModeSendMessage(lpSS, SS_SELMODE_CLEARSEL, 0L, 0L, 0L);

   lpSS->BlockCellUL.Row = -1;
   lpSS->BlockCellLR.Row = -1;
	// RFW - 6/28/04 - 14737
   lpSS->BlockCellUL.Col = -1;
   lpSS->BlockCellLR.Col = -1;

   lpSS->Row.BlockCellCurrentPos = lpSS->Row.CurAt;
   lpSS->ExtSelAnchorRow = lpSS->Row.CurAt;

   return (TRUE);
   }

#endif

if (SS_IsBlockSelected(lpSS))
   fBlockSelected = TRUE;

if (fDraw)
   {
   if ((lpSS->lpBook->hWnd && GetFocus() == lpSS->lpBook->hWnd) || lpSS->lpBook->fRetainSelBlock)
      fRet = SS_DrawSelBlock(0, lpSS);
   else
      fRet = !lpSS->fSuspendInvert;
   }
else
   fRet = TRUE;

lpSS->lpBook->fSelBlockInvisible = FALSE;

if (lpSS->fMultipleBlocksSelected)
   {
   if (lpSS->MultiSelBlock.hItemList)
      GlobalFree(lpSS->MultiSelBlock.hItemList);

   lpSS->MultiSelBlock.hItemList = 0;
   lpSS->MultiSelBlock.dItemCnt = 0;
   lpSS->MultiSelBlock.dItemAllocCnt = 0;
   lpSS->fMultipleBlocksSelected = FALSE;
   }

if (fRet)
   {
   lpSS->BlockCellUL.Row = -1;
   lpSS->BlockCellUL.Col = -1;

   lpSS->BlockCellLR.Row = -1;
   lpSS->BlockCellLR.Col = -1;
   }

if (fBlockSelected)
   SS_SendMsgCommand(lpSS->lpBook, lpSS, SSN_BLOCKDESELECTED, FALSE);

return (fRet);
}


void SS_GetClientRect(LPSS_BOOK lpBook, LPRECT lpRect)
{
if (lpBook->hWnd)
   GetClientRect(lpBook->hWnd, lpRect);
else
   {
   lpRect->left = 0;
   lpRect->top = 0;
   lpRect->right = lpBook->xExtent;
   lpRect->bottom = lpBook->yExtent;
   }

#ifndef SS_NOCHILDSCROLL
if (lpBook->fVScrollVisible)
   lpRect->right -= lpBook->dVScrollWidth - 1;

#ifdef SS_V70
if (lpBook->fHScrollVisible || SS_IsTabStripVisible(lpBook))
#else
if (lpBook->fHScrollVisible)
#endif // SS_V70
lpRect->bottom -= lpBook->dHScrollHeight - 1;
#ifdef kmtest
if (TRUE && SS_IsTabStripVisible(lpBook))
	 lpRect->top += lpBook->dHScrollHeight - 1;
#endif

#ifdef SS_UTP
if (lpBook->fUseScrollArrows)
   {
   if (lpBook->fRightScrollArrowVis)
      lpRect->right -= lpSS->dScrollArrowWidth;

   if (lpBook->fBottomScrollArrowVis)
      lpRect->bottom -= lpBook->dScrollArrowHeight;
   }
#endif // SS_UTP
#endif
}


BOOL SS_AutoSize(LPSS_BOOK lpBook, BOOL fWM_Size)
{
LPSPREADSHEET lpSS;
RECT          Rect;
RECT          ClientRect;
#ifndef SS_NOAUTOSIZE
RECT          ClientRectParent;
POINT         Point;
SS_COORD      Rows;
SS_COORD      Cols;
int           Width;
int           Height;
int           x;
int           y;
#endif
BOOL          fVScrollVisibleOld;
BOOL          fHScrollVisibleOld;
BOOL          fRet = FALSE;
short         nHScrollLeftOffset = 0;

if (!lpBook->hWnd)  // handle an OCX control without a HWND
  return fRet;

lpSS = SS_BookLockActiveSheet(lpBook);

if (!lpSS)
	return (fRet);

// added this fix for !SPREAD_JPN -scl
// 97' 2/3 Modified by BOC Gao. for BUG000734
// When set Redraw to False, ScrollBars display wrong
// Here just allow it recalculating.
//#ifdef SPREAD_JPN
#if SS_V80
if ((lpBook->Redraw || lpBook->fWmSizeMoveScrollBars) && !(GetWindowLong(GetParent(lpBook->hWnd), GWL_STYLE) & WS_MINIMIZE))
#else
if (lpBook->Redraw && !(GetWindowLong(GetParent(lpBook->hWnd), GWL_STYLE) & WS_MINIMIZE))
#endif
//#else
//if (lpBook->Redraw &&
//    !(GetWindowLong(GetParent(lpBook->hWnd), GWL_STYLE) & WS_MINIMIZE))
//#endif
// --------------------------------------<<
   {
   lpBook->fUpdateAutoSize = FALSE;

   fHScrollVisibleOld = lpBook->fHScrollVisible;
   fVScrollVisibleOld = lpBook->fVScrollVisible;

   SS_SetHScrollBar(lpSS);
   SS_SetVScrollBar(lpSS);

	if (lpBook->fScrollBarExtMode && lpBook->fHScrollVisible && lpBook->fVScrollVisible && !lpBook->fAutoSize)
		{
		RECT Rect;
		short x, y;

		GetClientRect(lpBook->hWnd, &Rect);
		x = SS_GetCellPosXExt(lpSS, lpSS->Col.HeaderCnt, SS_GetColCnt(lpSS), &Rect);
		y = SS_GetCellPosYExt(lpSS, lpSS->Row.HeaderCnt, SS_GetRowCnt(lpSS), &Rect);

      if (x <= Rect.right && y <= Rect.bottom)
			{
			SS_GetClientRect(lpSS->lpBook, &Rect); // RFW - 5/2/06 - 18767
			SSx_SetVScrollBar(lpSS, &Rect);
			SSx_SetHScrollBar(lpSS, &Rect);
			}
		}

	// RFW - 5/27/04 - 14491
	else if ((BOOL)lpSS->lpBook->fVScrollVisible != fVScrollVisibleOld)
	   SS_SetHScrollBar(lpSS);

#ifndef SS_NOCHILDSCROLL

/*
   if (fWM_Size || fVScrollVisibleOld != lpBook->fVScrollVisible ||
       fHScrollVisibleOld != lpBook->fHScrollVisible)
*/
      {
      short dExtra;

      GetClientRect(lpBook->hWnd, &ClientRect);

#ifdef SS_V70
      if (lpBook->fHScrollVisible || SS_IsTabStripVisible(lpBook))
#else
      if (lpBook->fHScrollVisible)
#endif // SS_V70
         dExtra = lpBook->dHScrollHeight - 1;
      else
         dExtra = 0;

#ifdef kmtest
	  dExtra += lpBook->dHScrollHeight + 1;
      MoveWindow(lpBook->hWndVScroll, ClientRect.right -
                 lpBook->dVScrollWidth + 1, lpBook->dHScrollHeight, lpBook->dVScrollWidth - 1 +
                 lpBook->fBorderVisible,
                 ClientRect.bottom - dExtra + 2, TRUE);
#else
      MoveWindow(lpBook->hWndVScroll, ClientRect.right -
                 lpBook->dVScrollWidth + 1, -1, lpBook->dVScrollWidth - 1 +
                 lpBook->fBorderVisible,
                 ClientRect.bottom - dExtra + 2, TRUE);
#endif
      if (lpBook->fVScrollVisible)
         dExtra = lpBook->dVScrollWidth - 1;
      else
         dExtra = 0;

#ifdef SS_V70
		if (SS_IsTabStripVisible(lpBook))
			{
			RECT RectSplitBox;

			SS_GetTabSplitBoxRect(lpBook, &RectSplitBox);
			nHScrollLeftOffset = (short)RectSplitBox.right + 1;
			}
#endif // SS_V70
#ifdef kmtest
		nHScrollLeftOffset = 0;
#endif

      MoveWindow(lpBook->hWndHScroll, -1 + nHScrollLeftOffset, ClientRect.bottom -
                 lpBook->dHScrollHeight + 1, ClientRect.right - dExtra + 2 - nHScrollLeftOffset,
                 lpBook->dHScrollHeight - 1 + lpBook->fBorderVisible, TRUE);

		// Invalidate the gray area in the lower left corner.
      if (!lpBook->fVScrollVisible && !lpBook->fHScrollVisible &&
          fVScrollVisibleOld && fHScrollVisibleOld)
         {
         SetRect(&Rect, ClientRect.right - lpBook->dVScrollWidth,
                 ClientRect.bottom - lpBook->dHScrollHeight,
                 ClientRect.right, ClientRect.bottom);
         SS_InvalidateRect(lpBook, &Rect, TRUE);
         }
      }

#endif

#ifdef SS_UTP
   SS_CalcHScrollArrowArea(lpBook->hWnd, lpSS);
   SS_CalcVScrollArrowArea(lpBook->hWnd, lpSS);
#endif

//RWP02c
//---------*---------
// if (lpBook->fInAutoSize)  // stop recursion if already in AutoSize
//    return fRet;
//---------*---------
   // Stop recursion if already in AutoSize AND there is no change in the 
   // visibility of either scroll bar
   if (lpBook->fInAutoSize && (BOOL)lpBook->fHScrollVisible == fHScrollVisibleOld && (BOOL)lpBook->fVScrollVisible == fVScrollVisibleOld)  
		{
		SS_BookUnlockActiveSheet(lpBook);
      return fRet;
		}
//RWP02c

   SS_GetClientRect(lpBook, &ClientRect);
   _fmemcpy(&lpBook->ClientRect, &ClientRect, sizeof(RECT));

#ifndef SS_NOAUTOSIZE
   if (lpBook->fAutoSize)
      {      

#ifdef SS_OCX
      LPDISPATCH lpdispExtendedCtrl = SS_GetExtendedControl(lpBook);
      FPCONTROL fpControl = {lpBook->hWnd, 0, (long)lpBook->lpOleControl, 
                             (long)lpdispExtendedCtrl};
      FPCONTROL fpResizingParent = {0, 0, 0, 0};  // "True" parent

      if (lpdispExtendedCtrl)
         {
         // "Frame" window for Subclassed controls
         //    fpParent.hWnd = fpGetParent(&fpControl);
         // "True parent" for Subclassed controls
         fpResizingParent.lpDispatch = fpGetControlDispatch(&fpControl, 
                                                            GC_CONTAINER);                         
         fpResizingParent.hWnd = fpVBGetControlHwnd(&fpResizingParent);
			/* RFW - 15121
         // Force following function to use the dispatch
         fpControl.hWnd = NULL;
         fpControl.lObject = (LONG)0;
			*/
         fpVBGetRectInContainer(&fpControl, &Rect);
         lpdispExtendedCtrl->lpVtbl->Release(lpdispExtendedCtrl);

			// RFW - 7/28/06 - 19223
			if (fpResizingParent.lpDispatch == 0)
	         fpResizingParent.hWnd = GetParent(fpGetParent(&fpControl));
         }
      else
         {
         fpResizingParent.hWnd = fpGetParent(&fpControl);
         fpVBGetRectInContainer(&fpControl, &Rect);
         }

      // BJO 18Feb97 SEL8129 - Begin fix
      // In rectangles returned by GetClientRect(), if width or
      // height is zero then both are zero.  We need to modify
      // rectangle returned from fpVBGetRectInContainer() to
      // match this convention.
      if (Rect.top >= Rect.bottom || Rect.left >= Rect.right)
      {
        Rect.bottom = Rect.top;
        Rect.right = Rect.left;
      }
      // BJO 18Feb97 SEL8129 - End fix

      /************************************
      * Compute the maximum size of the
      * control as the size of the parent
      ************************************/
     
      fpGetClientRect(&fpResizingParent, &ClientRectParent);
      // release dispatch now that we have the Parent's client rect
      if (fpResizingParent.lpDispatch)
         fpDispatchRelease((LONG)fpResizingParent.lpDispatch);

      Point.x = Rect.left;
      Point.y = Rect.top;
      /* RFW - 4/7/08 - 22331 - I moved this down to fpTools
      // 20261  -scl
      ScreenToClient(fpResizingParent.hWnd, &Point);
      */

		// RFW - 4/25/05 - 16133
		if (ClientRectParent.right - ClientRectParent.left > 1 &&
		    ClientRectParent.bottom - ClientRectParent.top > 1)
			{
			ClientRect.right = ClientRectParent.right - Point.x -
									 (Rect.right - Rect.left - ClientRect.right);
			ClientRect.bottom = ClientRectParent.bottom - Point.y -
									  (Rect.bottom - Rect.top - ClientRect.bottom);
			}

#else //not SS_OCX
      FPCONTROL fpParent = {0, 0, 0, 0};
      FPCONTROL fpControl = {lpBook->hWnd, 0, 0, 0};
      fpParent.hWnd = GetParent(lpBook->hWnd);
      GetWindowRect(fpControl.hWnd, &Rect);
      GetClientRect(fpParent.hWnd, &ClientRectParent);
      Point.x = Rect.left;
      Point.y = Rect.top;
      ScreenToClient(fpParent.hWnd, &Point);

      ClientRect.right = ClientRectParent.right - Point.x -
                         (Rect.right - Rect.left - ClientRect.right);
      ClientRect.bottom = ClientRectParent.bottom - Point.y -
                          (Rect.bottom - Rect.top - ClientRect.bottom);
#endif

//Modify By BOC 99.7.6 (hyt)------------------------------------
//the older bugs seem fixed by farpoint and this fixed have many 
//questions so close it

// Added by BOC FMH 1996.07.01. --------------------->>
/* BUG1. When some rows or cols are hidded, the VisibleRows or
 *       VisibleCols can be seen (while AutoSize = True) is not correct.
 * BUG2. And, the Height or Width should calculate from the current
 *       top row or col.
 */
/*#ifdef SPREAD_JPN
    {
        SS_COORD RowColMax, i;
        int      dt;

        RowColMax = SS_GetColCnt(lpSS);
        // Get the end row number
        if (lpSS->Col.AutoSizeVisibleCnt)
// 96' 12/16 Modified by BOC Gao.
//            Cols = min(lpSS->Col.AutoSizeVisibleCnt + lpSS->Col.UL, RowColMax);
		      Cols = min(lpSS->Col.AutoSizeVisibleCnt, RowColMax - 1);
// ----------------------------<<
        else
// 96' 12/16 Modified by BOC Gao.
//            Cols = RowColMax;
            Cols = RowColMax - 1;
// ----------------------------<<
        
        // Get the width of new size.
       for (i = 0, x = -1; i < lpSS->Col.HeaderCnt; i++)
           x += SS_GetColWidthInPixels(lpSS, i);

// 96' 12/16 Added by BOC Gao.
       for (i = 1; i <= min(lpSS->Col.Frozen, Cols); i++)
           x += SS_GetColWidthInPixels(lpSS, i);
// ----------------------------<<

// 96' 12/16 Modified by BOC Gao.
//			for (i = lpSS->Col.UL; i < Cols; i++)
		for (i = lpSS->Col.UL; i < Cols + lpSS->Col.UL - lpSS->Col.Frozen; i++)
// ----------------------------<<
		{
			dt = SS_GetColWidthInPixels(lpSS, i);
			if (dt == 0)
//Modify By BOC 99.6.30 (hyt)----------------------------------------
//for not calc size correct while AutoSize = True and some col hidden
				//Cols = min(Cols + 1, RowColMax);
				Cols = min(Cols + 1, RowColMax-1);
//--------------------------------------------------------------------
			else
				x += dt;

			if (x > ClientRect.right - ClientRect.left)
				break;
		}
        Width = min(x, ClientRect.right);

        RowColMax = SS_GetRowCnt(lpSS);
        // Get the end row number
        if (lpSS->Row.AutoSizeVisibleCnt)
// 96' 12/16 Modified by BOC Gao.
//            Rows = min(lpSS->Row.AutoSizeVisibleCnt + lpSS->Row.UL, RowColMax);
            Rows = min(lpSS->Row.AutoSizeVisibleCnt, RowColMax - 1);
// ----------------------------<<
        else
// 96' 12/16 Modified by BOC Gao.
//            Rows = RowColMax;
            Rows = RowColMax - 1;
// ----------------------------<<

        // Get the height of new size
        for (i = 0, y = -1; i < lpSS->Row.HeaderCnt; i++)
            y += SS_GetRowHeightInPixels(lpSS, i);

// 96' 12/16 Added by BOC Gao.
        for (i = 1; i <= min(lpSS->Row.Frozen, Rows); i++)
            y += SS_GetRowHeightInPixels(lpSS, i);
// ----------------------------<<


// 96' 12/16 Modified by BOC Gao.
//		for (i = lpSS->Row.UL; i < Rows; i++)
		for (i = lpSS->Row.UL; i < Rows + lpSS->Row.UL - lpSS->Row.Frozen; i++)
// ----------------------------<<
		{
			dt = SS_GetRowHeightInPixels(lpSS, i);
			if (dt == 0)
//Modify By BOC 99.6.30 (hyt)----------------------------------------
//for not calc size correct while AutoSize = True and some row hidden
				//Rows = min(Rows + 1, RowColMax);
				Rows = min(Rows + 1, RowColMax-1);
//--------------------------------------------------------------------
			else
				y += dt;

			if (y > ClientRect.bottom - ClientRect.top)
				break;
		}
        Height = min(y, ClientRect.bottom);
    }

#else*/
// FarPoint original codes
//--------------------------------<<
      if (lpSS->Col.AutoSizeVisibleCnt)
         Cols = min(lpSS->Col.AutoSizeVisibleCnt, SS_GetColCnt(lpSS) - lpSS->Col.HeaderCnt);
      else
         Cols = SS_GetColCnt(lpSS) - lpSS->Col.HeaderCnt;

//RWP04c
//---------*---------
//    x = SS_GetCellPosXExt(lpSS, lpSS->Col.HeaderCnt, Cols + 1, &ClientRect);
//---------*---------
      x = SS_GetAutoSizeVisColsWidth(lpSS, Cols, &ClientRect);
//RWP04c
      Width = min(x, ClientRect.right);

      if (lpSS->Row.AutoSizeVisibleCnt)
         Rows = min(lpSS->Row.AutoSizeVisibleCnt, SS_GetRowCnt(lpSS) - lpSS->Row.HeaderCnt);
      else
         Rows = SS_GetRowCnt(lpSS) - lpSS->Row.HeaderCnt;

//RWP04c
//---------*---------
//    y = SS_GetCellPosYExt(lpSS, lpSS->Row.HeaderCnt, Rows + 1, &ClientRect);
//---------*---------
      y = SS_GetAutoSizeVisRowsHeight(lpSS, Rows, &ClientRect);
//RWP04c

      Height = min(y, ClientRect.bottom);
// Added by BOC FMH 1996.07.01. -------------------->>
//#endif
//---------------------------------<<

      if (Width != lpBook->ClientRect.right || Height != lpBook->ClientRect.bottom)
         {
         Point.x = Rect.left;
         Point.y = Rect.top;

#ifdef SS_OCX
         //if (!lpBook->fInAutoSize)
         if (lpBook->hWnd)
            {
            RECT RectClient;
            RECT RectNonClient;
            int  xTra;

            GetWindowRect(lpBook->hWnd, &RectNonClient);
            CopyRect(&RectClient, &RectNonClient);
            SendMessage(lpBook->hWnd, WM_NCCALCSIZE, (WPARAM)0,
                        (LPARAM)(LPNCCALCSIZE_PARAMS)&RectClient);
            xTra = (RectNonClient.right - RectNonClient.left) -
                    (RectClient.right - RectClient.left);
//            xTra = 2;
            //lpBook->fInAutoSize = TRUE;
            // BJO 02Apr98 JOK4981 - Before fix
            //PostMessage(lpBook->hWnd, SSM_SETCONTROLSIZE, 
            //   Width +  (lpBook->fVScrollVisible ? lpBook->dVScrollWidth + 1 : 2),
            //   Height + (lpBook->fHScrollVisible ? lpBook->dHScrollHeight + 1 : 2));
            // BJO 02Apr98 JOK4981 - Begin fix
            SendMessage(lpBook->hWnd, SSM_SETCONTROLSIZE, 
               Width +  (lpBook->fVScrollVisible ? lpBook->dVScrollWidth + xTra - 1 : xTra),
#ifdef SS_V70
               Height + (lpBook->fHScrollVisible || SS_IsTabStripVisible(lpBook) ? lpBook->dHScrollHeight + xTra - 1 : xTra));
#else
               Height + (lpBook->fHScrollVisible ? lpBook->dHScrollHeight + xTra - 1 : xTra));
#endif // SS_V70
            // BJO 02Apr98 JOK4981 - End fix
            //lpBook->fInAutoSize = FALSE;
            }
#else
         ScreenToClient(GetParent(lpBook->hWnd), &Point);
         MoveWindow(lpBook->hWnd, Point.x, Point.y, Width + (Rect.right - Rect.left -
                    lpBook->ClientRect.right), Height + (Rect.bottom -
                    Rect.top - lpBook->ClientRect.bottom), TRUE);
#endif


         fRet = TRUE;
         }
      }
#endif

   }
else
   lpBook->fUpdateAutoSize = TRUE;

SS_BookUnlockActiveSheet(lpBook);

return (fRet);
}


BOOL SS_GetCellFromPixel(lpSS, lpCol, lpRow, lpX, lpY, MouseX, MouseY)

LPSPREADSHEET lpSS;
LPSS_COORD    lpCol;
LPSS_COORD    lpRow;
LPINT         lpX;
LPINT         lpY;
int           MouseX;
int           MouseY;
{
RECT          RectClient;
BOOL          fRet = TRUE;

SS_GetClientRect(lpSS->lpBook, &RectClient);

if (lpCol)
   {
   SS_COORD Col = -2;
	SS_COORD ColCnt = SS_GetColCnt(lpSS);
   // short    x = 0; RFW - 8/1/00 - SCS9882
	short    x = -1;

	// RFW - 6/23/99 - SCS8218
   if (MouseX >= RectClient.left && MouseX < RectClient.right)
      {
      SS_COORD ColAt = 0;
      short    Width;

      for (; ColAt < ColCnt && x < RectClient.right; )
         {
         Width = SS_GetColWidthInPixels(lpSS, ColAt);

         if (MouseX < RectClient.left + x + Width)
            {
            Col = ColAt;
            break;
            }

         x += Width;

         if (ColAt == lpSS->Col.HeaderCnt + lpSS->Col.Frozen - 1)
            ColAt = max(lpSS->Col.UL, lpSS->Col.HeaderCnt + lpSS->Col.Frozen);
         else
            ColAt++;
         }
      }

   *lpCol = Col;

   if (lpX)
      *lpX = x;

   if (Col == -2)
      fRet = FALSE;
   }

if (lpRow)
   {
   SS_COORD Row = -2;
	SS_COORD RowCnt = SS_GetRowCnt(lpSS);
   // short    y = 0; RFW - 8/1/00 - SCS9882
	short    y = -1;

	// RFW - 6/23/99 - SCS8218
   if (MouseY >= RectClient.top && MouseY < RectClient.bottom)
      {
      SS_COORD RowAt = 0;
      short    Height;

      for (; RowAt < RowCnt && y < RectClient.bottom; )
         {
         Height = SS_GetRowHeightInPixels(lpSS, RowAt);

         if (MouseY < RectClient.top + y + Height)
            {
            Row = RowAt;
            break;
            }

         y += Height;

         if (RowAt == lpSS->Row.HeaderCnt + lpSS->Row.Frozen - 1)
            RowAt = max(lpSS->Row.UL, lpSS->Row.HeaderCnt + lpSS->Row.Frozen);
         else
            RowAt++;
         }
      }

   *lpRow = Row;

   if (lpY)
      *lpY = y;

   if (Row == -2)
      fRet = FALSE;
   }

return (fRet);
}


void SS_Beep(LPSS_BOOK lpBook)
{
if (!lpBook->fNoBeep)
   MessageBeep(0);
}


//RWP03a
void SS_Beep2(HWND hWnd)
{
LPSS_BOOK lpBook;

if ( lpBook = SS_BookLock(GetParent(hWnd)))
	if (!lpBook->fNoBeep)
		MessageBeep(0);
}
//RWP03a


void SS_BeepLowlevel(HWND hwnd)
{
LPSS_BOOK lpBook;

if (lpBook = SS_BookLock(GetParent(hwnd)))
	SS_Beep(lpBook);

SS_BookUnlock(hwnd);
}


#ifndef SS_NOLISTBOX

void SS_SelectRow(HWND hWnd, LPSPREADSHEET lpSS)
{
SSx_SelectRow(hWnd, lpSS, lpSS->Row.CurAt);
}


void SSx_SelectRow(HWND hWnd, LPSPREADSHEET lpSS, SS_COORD Row)
{
LPSS_ROW lpRow;
LPSS_ROW lpRowTemp;
SS_COORD NewUL;
SS_COORD NewLR;
SS_COORD Row1 = 0;
SS_COORD Row2 = 0;
SS_COORD i;
HDC      hDC;
BOOL     fInvert = FALSE;

if (lpRow = SS_AllocLockRow(lpSS, Row))
   {
   /*********************
   * Extended Selection
   *********************/

   if (lpSS->wOpMode == SS_OPMODE_EXTSEL && (!SS_IsF8ExtSelMode(lpSS) || HIBYTE(GetKeyState(VK_SHIFT))))
      {
		int iLoop;

      hDC = SS_GetDC(lpSS->lpBook);

      if (lpSS->BlockCellUL.Row == -1 ||
          Row < lpSS->BlockCellUL.Row ||
          Row > lpSS->BlockCellLR.Row)
         {
         if (lpSS->BlockCellUL.Row == -1)
            {
            Row1 = Row;
            Row2 = Row;
            lpSS->BlockCellUL.Row = Row;
            lpSS->BlockCellLR.Row = Row;
            }
         else if (Row < lpSS->BlockCellUL.Row)
            {
            Row1 = Row;
            Row2 = lpSS->BlockCellUL.Row - 1;
            lpSS->BlockCellUL.Row = Row;
            }
         else
            {
            Row1 = lpSS->BlockCellLR.Row + 1;
            Row2 = Row;
            lpSS->BlockCellLR.Row = Row;
            }

         for (i = Row1; i <= Row2; i++)
            if (lpRowTemp = SS_AllocLockRow(lpSS, i))
               {
               if (lpSS->fExtSelHighlight)
                  {
                  if (!lpRowTemp->fRowSelected)
                      {
                      lpSS->MultiSelCnt++;

                      lpRowTemp->fRowSelected = TRUE;
                      SS_InvertRowBlock(hWnd, hDC, lpSS, i, i);
                      }
                  else
                      lpRowTemp->fRowSelected = 3;
                  }
               else
                  {
                  if (lpRowTemp->fRowSelected)
                      {
                      lpSS->MultiSelCnt--;
                      lpRowTemp->fRowSelected = FALSE;
                      SS_InvertRowBlock(hWnd, hDC, lpSS, i, i);
                      }
                  else
                      lpRowTemp->fRowSelected = 2;
                   }

               SS_UnlockRowItem(lpSS, i);
               }

         }

      NewUL = min(Row, lpSS->ExtSelAnchorRow);
      NewLR = max(Row, lpSS->ExtSelAnchorRow);

		// RFW - 9/8/03 - 12398
		for (iLoop = 0; iLoop < 2; iLoop++)
			{
			if (lpSS->BlockCellUL.Row < NewUL || NewLR < lpSS->BlockCellLR.Row)
				{
				if (lpSS->BlockCellUL.Row < NewUL)
					{
					Row1 = lpSS->BlockCellUL.Row;
					Row2 = NewUL - 1;
					lpSS->BlockCellUL.Row = NewUL;
					}
				else
					{
					Row1 = NewLR + 1;
					Row2 = lpSS->BlockCellLR.Row;
					lpSS->BlockCellLR.Row = NewLR;
					}

				for (i = Row1; i <= Row2; i++)
					if (lpRowTemp = SS_LockRowItem(lpSS, i))
						{
						if (lpSS->fExtSelHighlight)
							{
							if (lpRowTemp->fRowSelected == 1)
								{
								lpSS->MultiSelCnt--;
								lpRowTemp->fRowSelected = FALSE;
								SS_InvertRowBlock(hWnd, hDC, lpSS, i, i);
								}
							else if (lpRowTemp->fRowSelected == 3)
								lpRowTemp->fRowSelected = TRUE;
							}
						else
							{
							if (lpRowTemp->fRowSelected == 0)
								{
								lpSS->MultiSelCnt++;
								lpRowTemp->fRowSelected = TRUE;
								SS_InvertRowBlock(hWnd, hDC, lpSS, i, i);
								}
							else if (lpRowTemp->fRowSelected == 2)
								lpRowTemp->fRowSelected = FALSE;
							}

						SS_UnlockRowItem(lpSS, i);
						}
				}
         }

      SS_ReleaseDC(lpSS->lpBook, hDC);
      fInvert = FALSE;

      lpSS->Row.BlockCellCurrentPos = Row;
      }

   /*********************
   * Multiple Selection
   *********************/

   else
      {
      if (lpRow->fRowSelected)
         {
         lpSS->MultiSelCnt--;
         lpRow->fRowSelected = FALSE;
         }
      else
         {
         lpSS->MultiSelCnt++;
         lpRow->fRowSelected = TRUE;
         }

      hDC = SS_GetDC(lpSS->lpBook);
      SS_InvertRowBlock(hWnd, hDC, lpSS, Row, Row);
      SS_ReleaseDC(lpSS->lpBook, hDC);
      }

   SS_UnlockRowItem(lpSS, Row);
   }
}


void SS_ExtSelBegin(HWND hWnd, LPSPREADSHEET lpSS)
{
LPSS_ROW lpRow;
SS_COORD Row;
SS_COORD Row1;
SS_COORD Row2;
SS_COORD i;
BOOL     RedrawOld;

lpSS->fExtSelHighlight = TRUE;

if (HIBYTE(GetKeyState(VK_SHIFT)))
   {
   if (lpSS->ExtSelAnchorRow >= 1)
      Row = lpSS->ExtSelAnchorRow;
   else
      Row = 1;

   if (Row < lpSS->Row.CurAt)
      {
      Row1 = Row;
      Row2 = lpSS->Row.CurAt;
      lpSS->BlockCellUL.Row = Row;

      if (lpSS->BlockCellLR.Row != -1 && lpSS->BlockCellLR.Row > Row)
         lpSS->BlockCellLR.Row = min(lpSS->BlockCellLR.Row, Row2);
      else
         lpSS->BlockCellLR.Row = Row;

      lpSS->Row.BlockCellCurrentPos = lpSS->BlockCellLR.Row;
      }
   else
      {
      Row1 = lpSS->Row.CurAt;
      Row2 = Row;
      lpSS->BlockCellLR.Row = Row;

      if (lpSS->BlockCellUL.Row != -1 && lpSS->BlockCellUL.Row < Row)
         lpSS->BlockCellUL.Row = max(lpSS->BlockCellUL.Row, Row1);
      else
         lpSS->BlockCellUL.Row = Row;

      lpSS->Row.BlockCellCurrentPos = lpSS->BlockCellUL.Row;
      }

   if (!HIBYTE(GetKeyState(VK_CONTROL)))
		{
		RedrawOld = lpSS->lpBook->Redraw;
		lpSS->lpBook->Redraw = FALSE;

		for (i = lpSS->Row.HeaderCnt; i < lpSS->Row.AllocCnt; i++)
			if (i < Row1 || i > Row2)
				if (lpRow = SS_LockRowItem(lpSS, i))
					{
					if (lpRow->fRowSelected)
						{
						lpRow->fRowSelected = FALSE;
						SS_InvalidateRow(lpSS, i);
						lpSS->MultiSelCnt--;
						}

					SS_UnlockRowItem(lpSS, i);
					}

		SS_BookSetRedraw(lpSS->lpBook, RedrawOld);
		}
   }

else
   {
   if (!HIBYTE(GetKeyState(VK_CONTROL)))
      SS_SelModeSendMessage(lpSS, SS_SELMODE_CLEARSEL, 0L, 0L, 0L);
   else
      {
      if (lpRow = SS_LockRowItem(lpSS, lpSS->Row.CurAt))
         {
         if (lpRow->fRowSelected)
            lpSS->fExtSelHighlight = !lpRow->fRowSelected;

         SS_UnlockRowItem(lpSS, lpSS->Row.CurAt);
         }
      }

   lpSS->BlockCellUL.Row = -1;
   lpSS->BlockCellLR.Row = -1;
   lpSS->Row.BlockCellCurrentPos = lpSS->Row.CurAt;
   lpSS->ExtSelAnchorRow = lpSS->Row.CurAt;
   }

SS_SelectRow(hWnd, lpSS);
}

#endif


void SS_SetBlock(LPSS_SELBLOCK lpBlock, SS_COORD Col1, SS_COORD Row1,
                 SS_COORD Col2, SS_COORD Row2)
{
lpBlock->UL.Col = Col1;
lpBlock->UL.Row = Row1;
lpBlock->LR.Col = Col2;
lpBlock->LR.Row = Row2;
}


BOOL SS_IntersectBlock(LPSS_SELBLOCK lpBlock, LPSS_SELBLOCK lpSrc1,
                       LPSS_SELBLOCK lpSrc2)
{
BOOL fRet = FALSE;

if (lpSrc1->LR.Row >= lpSrc2->UL.Row && lpSrc1->UL.Row <= lpSrc2->LR.Row &&
    lpSrc1->LR.Col >= lpSrc2->UL.Col && lpSrc1->UL.Col <= lpSrc2->LR.Col)
   {
   lpBlock->UL.Col = max(lpSrc1->UL.Col, lpSrc2->UL.Col);
   lpBlock->UL.Row = max(lpSrc1->UL.Row, lpSrc2->UL.Row);
   lpBlock->LR.Col = min(lpSrc1->LR.Col, lpSrc2->LR.Col);
   lpBlock->LR.Row = min(lpSrc1->LR.Row, lpSrc2->LR.Row);
   fRet = TRUE;
   }

else
   SS_SetBlock(lpBlock, 0, 0, 0, 0);

return (fRet);
}
 

#ifdef SS_UTP
BOOL SS_IsMouseInScrollArrow(HWND hWnd, LPSPREADSHEET lpSS,
                             LPRECT lpRectClient, int MouseX, int MouseY,
                             LPBYTE lpbModeScrollArrow, BYTE bArrowToCheck)
{
RECT  RectRight;
RECT  RectDown;
RECT  RectLeft;
RECT  RectUp;
POINT Point;
BOOL  fRet = FALSE;

if (lpbModeScrollArrow)
   *lpbModeScrollArrow = 0;

SS_GetScrollArrowRects(hWnd, lpSS, &RectRight, &RectDown, NULL, NULL);

Point.x = MouseX;
Point.y = MouseY;

if ((bArrowToCheck == 0 || bArrowToCheck == SS_MODE_ARROW_RIGHT) &&
     PtInRect(&RectRight, Point))
   {
   if (lpbModeScrollArrow)
      *lpbModeScrollArrow = SS_MODE_ARROW_RIGHT;
   fRet = TRUE;
   }

else if ((bArrowToCheck == 0 || bArrowToCheck == SS_MODE_ARROW_DOWN) &&
         PtInRect(&RectDown, Point))
   {
   if (lpbModeScrollArrow)
      *lpbModeScrollArrow = SS_MODE_ARROW_DOWN;
   fRet = TRUE;
   }

else
   {
   SS_GetScrollArrowRects(hWnd, lpSS, NULL, NULL, &RectLeft, NULL);

   if ((bArrowToCheck == 0 || bArrowToCheck == SS_MODE_ARROW_LEFT) &&
        PtInRect(&RectLeft, Point))
      {
      if (lpbModeScrollArrow)
         *lpbModeScrollArrow = SS_MODE_ARROW_LEFT;
      fRet = TRUE;
      }

   else
      {
      SS_GetScrollArrowRects(hWnd, lpSS, NULL, NULL, NULL, &RectUp);

      if ((bArrowToCheck == 0 || bArrowToCheck == SS_MODE_ARROW_UP) &&
           PtInRect(&RectUp, Point))
         {
         if (lpbModeScrollArrow)
            *lpbModeScrollArrow = SS_MODE_ARROW_UP;
         fRet = TRUE;
         }
      }
   }

return (fRet);
}
#endif


long SS_KeyDown(LPSPREADSHEET lpSS, WPARAM wParam, LPARAM lParam, BOOL fSendKeyDownEvent)
{
HWND         hWnd = lpSS->lpBook->hWnd;
WORD         wShift = 0;
LRESULT      lRet;
BOOL         fMoveActiveCell;
BOOL         fSingleSel;
BOOL         fControlDown = FALSE;
BOOL         fShiftDown = FALSE;
BOOL         fAltDown = FALSE;
short        dRet = -1;
BOOL         fPrevKeyState = (BOOL)((lParam >> 30) &0x01);
// RFW - 5/13/03 - 11935
if (wParam == VK_SHIFT && fPrevKeyState == 0)
   {
   if (lpSS->wOpMode == SS_OPMODE_EXTSEL)
      lpSS->lpBook->wMode = SS_MODE_BLOCK;

   else if (SS_IsBlockSelected(lpSS))
      lpSS->lpBook->wMode = SS_MODE_BLOCK;

   else if ((SS_SELBLOCKOPTION(lpSS) & SS_SELBLOCK_BLOCKS) &&
             !(lpSS->lpBook->wMode == SS_MODE_BLOCK) && lpSS->Col.Max > 0 &&
             lpSS->Row.Max > 0)
      {
      lpSS->Col.BlockCellCurrentPos = lpSS->Col.CurAt;
      lpSS->Row.BlockCellCurrentPos = lpSS->Row.CurAt;
      lpSS->BlockCellUL.Col = lpSS->Col.CurAt;
      lpSS->BlockCellUL.Row = lpSS->Row.CurAt;
      lpSS->BlockCellLR.Col = lpSS->Col.CurAt;
      lpSS->BlockCellLR.Row = lpSS->Row.CurAt;
      lpSS->lpBook->wMode = SS_MODE_BLOCK;
      }
   }

if (GetKeyState(VK_SHIFT) & 0x8000)
   {
   fShiftDown = TRUE;
   wShift |= 0x01;
   }

else if (lpSS->wOpMode == SS_OPMODE_EXTSEL &&
         (wParam == VK_UP || wParam == VK_DOWN || wParam == VK_PRIOR ||
          wParam == VK_NEXT || wParam == VK_HOME || wParam == VK_END))
   lpSS->fExtSelHighlight = TRUE;

/* RFW - 6/23/04 - 14259
// 97' 2/5 Added by BOC Gao. for BUG000732
#ifdef SPREAD_JPN
else if (!(GetKeyState(VK_CONTROL) & 0x8000))
   lpSS->lpBook->wMode = SS_MODE_NONE;
#endif
// -------------------------------<<
*/

if (GetKeyState(VK_CONTROL) & 0x8000)
   {
   fControlDown = TRUE;
   wShift |= 0x02;
   }

if (GetKeyState(VK_MENU) & 0x8000)
   fAltDown = TRUE;

lpSS->Col.ULPrev = lpSS->Col.UL;
lpSS->Row.ULPrev = lpSS->Row.UL;

lRet = 0L;

// RFW - 3/6/04 - 13829
if (fSendKeyDownEvent)
	{
#if defined(SS_OCX) || defined(SS_VB)
   if (!lpSS->lpBook->EditModeOn)
      lRet = SS_SendMsg(lpSS->lpBook, lpSS, SSM_KEYDOWN, GetWindowID(hWnd),
                        MAKELONG(wParam, wShift));
#else
   lRet = SS_SendMsg(lpSS->lpBook, lpSS, SSM_KEYDOWN, GetWindowID(hWnd),
                     MAKELONG(wParam, wShift));
#endif
	}

if (lRet)
   return (0);

fSingleSel = FALSE;

if (SS_USESELBAR(lpSS))
   {
   fMoveActiveCell = FALSE;
   fSingleSel = TRUE;

   // RFW - 8/8/01 - 9197

   if (lpSS->Col.Frozen > 0)
	   lpSS->Col.CurAt = lpSS->Col.HeaderCnt;
   else
      lpSS->Col.CurAt = lpSS->Col.UL;
   }
else if (lpSS->wOpMode == SS_OPMODE_READONLY)
   fMoveActiveCell = FALSE;
else
   fMoveActiveCell = TRUE;

lpSS->lpBook->wMsgLast = WM_KEYDOWN;

if (fControlDown && (wParam == 'x' || wParam == 'X'))
   {
   if (lpSS->lpBook->fAutoClipboard)
      SSClipboardCut(hWnd);

   lpSS->lpBook->wMsgLast = 0;
   return (0);
   }

else if (fControlDown && (wParam == 'c' || wParam == 'C'))
   {
   if (lpSS->lpBook->fAutoClipboard)
      SSClipboardCopy(hWnd);

   lpSS->lpBook->wMsgLast = 0;
   return (0);
   }

else if (fControlDown && (wParam == 'v' || wParam == 'V'))
   {
   if (lpSS->lpBook->fAutoClipboard)
      SSClipboardPaste(hWnd);

   lpSS->lpBook->wMsgLast = 0;
   return (0);
   }

/*******
* Undo
*******/

else if ((fControlDown && (wParam == 'z' || wParam == 'Z')) ||
         ((lParam & 0x20000000L) && wParam == VK_BACK))
   {
   if (lpSS->lpBook->fAllowUndo)
      SS_Undo(hWnd, lpSS);

   lpSS->lpBook->wMsgLast = 0;
   return (0);
   }

#if 0
if ((wShift & 0x03) && wParam == VK_F12)
   SmartHeapInit();              // Reset SmartHeap
#endif

#ifdef SS_GP
if (fControlDown && (wParam == 'z' || wParam == 'Z'))
   wParam = VK_ESCAPE;
#endif

lpSS->lpBook->wScrollDirection = 0;

#ifdef SS_UTP
if (lpSS->dVKeyIns && wParam == (WPARAM)lpSS->dVKeyIns &&
    !fControlDown && !fShiftDown)
   {
   if (SS_IsBlockSelected(lpSS))
      {
      SS_COORD InsCnt;
      BOOL     fRedrawOld;

      fRedrawOld = lpSS->lpBook->Redraw;
      lpSS->lpBook->Redraw = FALSE;

      if (lpSS->BlockCellUL.Col == -1 ||
          lpSS->BlockCellLR.Col == -1)
         {
         InsCnt = lpSS->BlockCellLR.Row - lpSS->BlockCellUL.Row + 1;
         if (InsCnt > 0)
            {
            if (!SS_SendMsgInsDel(hWnd, SS_INSDEL_TYPE_INS, SS_INSDEL_OP_KEY,
                                  SS_INSDEL_ROW, lpSS->BlockCellUL.Row,
                                  lpSS->BlockCellLR.Row))
               {
               SSSetMaxRows(hWnd, SS_GetRowCnt(lpSS) - 1 + InsCnt);
               SS_InsRowRange(lpSS, lpSS->BlockCellUL.Row,
                             lpSS->BlockCellLR.Row);
               }
            }
         }

      else if (lpSS->BlockCellUL.Row == -1 ||
               lpSS->BlockCellLR.Row == -1)
         {
         InsCnt = lpSS->BlockCellLR.Col - lpSS->BlockCellUL.Col + 1;
         if (InsCnt > 0)
            {
            if (!SS_SendMsgInsDel(hWnd, SS_INSDEL_TYPE_INS, SS_INSDEL_OP_KEY,
                                  SS_INSDEL_COL, lpSS->BlockCellUL.Col,
                                  lpSS->BlockCellLR.Col))
               {
               SSSetMaxCols(hWnd, SS_GetColCnt(lpSS) - 1 + InsCnt);
               SS_InsColRange(lpSS, lpSS->BlockCellUL.Col,
                              lpSS->BlockCellLR.Col);
               }
            }
         }

		SS_BookSetRedraw(lpSS->lpBook, fRedrawOld)
      }
   }

else if (lpSS->dVKeyDel && wParam == (WPARAM)lpSS->dVKeyDel &&
         !fControlDown && !fShiftDown)
   SS_VBDelBlock(hWnd, lpSS, SS_INSDEL_OP_KEY);
#endif // SS_UTP

if (lpSS->wOpMode == SS_OPMODE_EXTSEL && (lpSS->lpBook->wMode == SS_MODE_BLOCK) &&
    (wParam == VK_UP || wParam == VK_DOWN || wParam == VK_PRIOR ||
     wParam == VK_NEXT || wParam == VK_HOME || wParam == VK_END))
   SS_HighlightCell(lpSS, FALSE);

switch (wParam)
   {
   case VK_SHIFT:
      lpSS->BlockCellULPrev.Col = lpSS->BlockCellUL.Col;
      lpSS->BlockCellULPrev.Row = lpSS->BlockCellUL.Row;
      lpSS->BlockCellLRPrev.Col = lpSS->BlockCellLR.Col;
      lpSS->BlockCellLRPrev.Row = lpSS->BlockCellLR.Row;

//      if (lpSS->wOpMode == SS_OPMODE_EXTSEL)
//         SS_ExtSelBegin(hWnd, lpSS);
      lpSS->fExtSelHighlight = TRUE;
      break;
   
   case VK_SPACE:
      if (lpSS->wOpMode == SS_OPMODE_NORMAL)
         {
			SS_CELLCOORD BlockCellUL;
			SS_CELLCOORD BlockCellLR;

         if (fControlDown && fShiftDown)
            {
            if ((SS_SELBLOCKOPTION(lpSS) & SS_SELBLOCK_ALL) && lpSS->Col.Max > 0 && lpSS->Row.Max > 0)
               {
               if (SS_IsBlockSelected(lpSS) &&
                   lpSS->BlockCellUL.Col == 0 &&
                   lpSS->BlockCellUL.Row == 0 &&
                   lpSS->BlockCellLR.Col == -1 &&
                   lpSS->BlockCellLR.Row == -1)
                  ;
               else
                  {
                  BlockCellUL.Col = 0;
                  BlockCellUL.Row = 0;
                  BlockCellLR.Col = -1;
                  BlockCellLR.Row = -1;
                  SS_SetSelectBlock(lpSS, &BlockCellUL, &BlockCellLR);
                  SS_SendMsgCommand(lpSS->lpBook, lpSS, SSN_BLOCKCHANGED, FALSE);
                 }
               return (TRUE);
               }
            }

         else if (fShiftDown)
            {
            if ((SS_SELBLOCKOPTION(lpSS) & SS_SELBLOCK_ROWS) && lpSS->Row.Max > 0)
               {  
               if (SS_IsBlockSelected(lpSS) &&
                   lpSS->BlockCellUL.Col == 0 &&
                   lpSS->BlockCellUL.Row != -1 &&
                   lpSS->BlockCellLR.Col == -1 &&
                   lpSS->BlockCellLR.Row != -1)
                  ;
               else
                  {
                  BlockCellUL.Col = 0;
                  if (SS_IsBlockSelected(lpSS))
                     BlockCellUL.Row = lpSS->BlockCellUL.Row;
                  else
                     BlockCellUL.Row = lpSS->Row.CurAt;
                  BlockCellLR.Col = -1;
                  if (SS_IsBlockSelected(lpSS))
                     BlockCellLR.Row = lpSS->BlockCellLR.Row;
                  else
                     BlockCellLR.Row = lpSS->Row.CurAt;
                  SS_SetSelectBlock(lpSS, &BlockCellUL, &BlockCellLR);
                  SS_SendMsgCommand(lpSS->lpBook, lpSS, SSN_BLOCKCHANGED, FALSE);
						// RFW - 10/22/03
                  lpSS->lpBook->wMode = SS_MODE_BLOCK;
                  }

               return (TRUE);
               }
            }

         else if (fControlDown)
            {     
            if ((SS_SELBLOCKOPTION(lpSS) & SS_SELBLOCK_COLS) && lpSS->Col.Max > 0)
               {
               if (SS_IsBlockSelected(lpSS) &&
                   lpSS->BlockCellUL.Col != -1 &&
                   lpSS->BlockCellUL.Row == 0 &&
                   lpSS->BlockCellLR.Col != -1 &&
                   lpSS->BlockCellLR.Row == -1)
                  ;
               else
                  {
                  if (SS_IsBlockSelected(lpSS))
                     BlockCellUL.Col = lpSS->BlockCellUL.Col;
                  else
                     BlockCellUL.Col = lpSS->Col.CurAt;
                  BlockCellUL.Row = 0;
                  if (SS_IsBlockSelected(lpSS))
                     BlockCellLR.Col = lpSS->BlockCellLR.Col;
                  else
                     BlockCellLR.Col = lpSS->Col.CurAt;
                  BlockCellLR.Row = -1;
                  SS_SetSelectBlock(lpSS, &BlockCellUL, &BlockCellLR);
                  SS_SendMsgCommand(lpSS->lpBook, lpSS, SSN_BLOCKCHANGED, FALSE);
                 SS_SendMsgCommand(lpSS->lpBook, lpSS, SSN_BLOCKSELECTED, FALSE);
						/* RFW - 1/24/05 - 15465
						// RFW - 3/9/04 - 13843
                  lpSS->lpBook->wMode = SS_MODE_BLOCK;
						*/
						lpSS->Col.BlockCellCurrentPos = lpSS->Col.CurAt;
						lpSS->Row.BlockCellCurrentPos = 0;
                  }

               return (TRUE);
               }
            }
         }

#ifndef SS_NOLISTBOX
      else if (lpSS->wOpMode == SS_OPMODE_EXTSEL)
         {
         LPSS_ROW lpRow;

			if (SS_IsF8ExtSelMode(lpSS))
				{
				if (!(wShift & 0x01))
				  lpSS->ExtSelAnchorRow = lpSS->Row.CurAt;
				}

			if (SS_IsF8ExtSelMode(lpSS) && (wShift & 0x01))
				{
				if (lpRow = SS_AllocLockRow(lpSS, lpSS->ExtSelAnchorRow))
					{
					lpSS->fExtSelHighlight = lpRow->fRowSelected;
					SS_UnlockRowItem(lpSS, lpSS->ExtSelAnchorRow);
					}
				}
         else if (lpRow = SS_AllocLockRow(lpSS, lpSS->Row.CurAt))
            {
            lpSS->fExtSelHighlight = !lpRow->fRowSelected;
            SS_UnlockRowItem(lpSS, lpSS->Row.CurAt);
            }

         if (!SS_IsF8ExtSelMode(lpSS) && !fControlDown)
	         SS_ResetBlock(lpSS);

         SS_SelectRow(hWnd, lpSS);

			if (SS_IsF8ExtSelMode(lpSS))
				{
				if (wShift & 0x01)
					{
			      lpSS->BlockCellUL.Row = min(lpSS->Row.CurAt, lpSS->ExtSelAnchorRow);
					lpSS->BlockCellLR.Row = max(lpSS->Row.CurAt, lpSS->ExtSelAnchorRow);
					}
				else
					{
			      lpSS->BlockCellUL.Row = lpSS->Row.CurAt;
					lpSS->BlockCellLR.Row = lpSS->Row.CurAt;
					}
				}
         }
#endif

      break;

   /*****************************************
   * Enter Key - Deactivate the active cell
   *****************************************/

   case VK_RETURN:
      if (lpSS->wOpMode != SS_OPMODE_EXTSEL)
         SS_ResetBlock(lpSS);
      /*
      if (SS_CellEditModeOff(lpSS, 0))
         SS_HighlightCell(lpSS, TRUE);

      else
         {
         if (SS_IsDestroyed(hWnd))
            return (FALSE);
      */

      if (!SS_CellEditModeOff(lpSS, 0))
         {
         if (SS_IsDestroyed(hWnd))
            return (FALSE);

         SS_HighlightCell(lpSS, TRUE);
         }

      else
         {
         if (fControlDown ||
             lpSS->lpBook->wEnterAction == SS_ENTERACTION_NONE ||
             lpSS->lpBook->wEnterAction == SS_ENTERACTION_SAME)
            {
            SS_CellEditModeOn(lpSS, WM_KEYDOWN, wParam, lParam);
            }
         else
            PostMessage(hWnd, SSM_PROCESSENTERKEY, 0, 0L);
         }

      SS_UpdateWindow(lpSS->lpBook);
      dRet = TRUE;
      break;

   case VK_ESCAPE:
      if (lpSS->lpBook->wMode == SS_MODE_RESIZE_HORIZ || lpSS->lpBook->wMode == SS_MODE_RESIZE_VERT)
			{
         ReleaseCapture();
         SS_Resize(hWnd, lpSS, SS_RESIZEMODE_END, lParam);
         lpSS->lpBook->wMode = SS_MODE_NONE;
         SS_HighlightCell(lpSS, TRUE);
			}

#ifndef SS_NODRAGDROP
      else if (lpSS->lpBook->wMode == SS_MODE_DRAGDROP)
         {
         ReleaseCapture();
         lpSS->lpBook->wMode = SS_MODE_NONE;
			SS_DragDropDrawBox(hWnd, lpSS, FALSE);
         }
#endif

#ifdef SS_V70
		else if (lpSS->lpBook->wMode == SS_MODE_COLMOVE ||
		         lpSS->lpBook->wMode == SS_MODE_BEGINCOLMOVE ||
		         lpSS->lpBook->wMode == SS_MODE_ROWMOVE ||
		         lpSS->lpBook->wMode == SS_MODE_BEGINROWMOVE)
			{
         ReleaseCapture();
			SS_HoverEnd(lpSS);
         lpSS->lpBook->wMode = SS_MODE_NONE;
         SS_HighlightCell(lpSS, TRUE);
			}
#endif // SS_V70

      return (FALSE);

   case VK_TAB:
      if (lpSS->lpBook->fProcessTab)
         {
         if (fMoveActiveCell)
            {
				if (!SS_IsF8ExtSelMode(lpSS))
					SS_ResetBlock(lpSS);

            if (fShiftDown)
               SS_ScrollPrevCell(lpSS);
            else
               SS_ScrollNextCell(lpSS);

            if (SS_IsDestroyed(hWnd))
               return (FALSE);

            SS_UpdateWindow(lpSS->lpBook);
            }
         }

      else
         {
         if (fShiftDown)
            PostMessage(GetParent(hWnd), WM_NEXTDLGCTL, 1, 0L);
         else
            PostMessage(GetParent(hWnd), WM_NEXTDLGCTL, 0, 0L);
         }

      dRet = FALSE;
      break;

   /*
   case VK_F2:
      SS_ResetBlock(lpSS);
      SS_CellEditModeOn(lpSS, 0, 0, 0L);
      return (0);
   */


//RWP01a
// case VK_F8:
//    // handles shift-f8 for putting the control into 'keyboard' extended
//    // extended multi-select mode
//    if (fShiftDown && lpSS->wOpMode == SS_OPMODE_EXTSEL)
//    {
//      MessageBox(NULL, "Toggling Keyboard Extended Multi-Select Mode", "Debug Message", MB_OK);
//        lpSS->fKeyboardExtendedMultiSelect = !(lpSS->fKeyboardExtendedMultiSelect);
//    }
//    break;
//RWP01a


   /************************************
   * Left key maps to horz scroll code
   ************************************/

   case VK_LEFT:
/* RFW - 6/24/04 - 14347
#ifdef SPREAD_JPN
	  //- Wei Feng for incident 421
	  if(!fShiftDown)
      //-		

      //- JPNFIX0009 - (Masanori Iwasa)
      if(lpSS->Col.CurAt == lpSS->Col.HeaderCnt && lpSS->Col.UL > lpSS->Col.Frozen +lpSS->Col.HeaderCnt)
         fMoveActiveCell = FALSE;
#endif
*/
      SS_ScrollLineUL(lpSS, (short)(SS_F_SCROLL_LEFT |
                     (fMoveActiveCell ? SS_F_MOVEACTIVECELL : 0)));

      if (SS_IsDestroyed(hWnd))
         return (FALSE);

      SS_UpdateWindow(lpSS->lpBook);
      dRet = FALSE;
      break;

   /*************************************
   * Right key maps to horz scroll code
   *************************************/

   case VK_RIGHT:
      SS_ScrollLineLR(lpSS, (short)(SS_F_SCROLL_RIGHT |
                     (fMoveActiveCell ? SS_F_MOVEACTIVECELL : 0)));

      if (SS_IsDestroyed(hWnd))
         return (FALSE);

      SS_UpdateWindow(lpSS->lpBook);
      dRet = FALSE;
      break;

   /**************************************
   * Up key maps to vertical scroll code
   **************************************/

   case VK_UP:
/* RFW - 6/24/04 - 14347
#ifdef SPREAD_JPN
	  //- Wei Feng for incident 421
	  if(!fShiftDown)
      //-		

      //- JPNFIX0009 - (Masanori Iwasa)
      if(lpSS->Row.CurAt == 1 && lpSS->Row.UL > lpSS->Row.Frozen +lpSS->Row.HeaderCnt)
         fMoveActiveCell = FALSE;
#endif
*/
      SS_ScrollLineUL(lpSS, (short)(SS_F_SCROLL_UP |
                     (fSingleSel ? SS_F_MOVEACTIVECELL :
                      (fMoveActiveCell ? SS_F_MOVEACTIVECELL : 0))));

      if (SS_IsDestroyed(hWnd))
         return (FALSE);

#ifndef SS_NOLISTBOX
      if (lpSS->wOpMode == SS_OPMODE_EXTSEL && (!SS_IsF8ExtSelMode(lpSS) || fShiftDown))
         {
         if (!(wShift & 0x01))
           lpSS->ExtSelAnchorRow = lpSS->Row.CurAt;
         SS_SelectRow(hWnd, lpSS);
         }
#endif

      SS_UpdateWindow(lpSS->lpBook);
      dRet = FALSE;
      break;

   /****************************************
   * Down key maps to vertical scroll code
   ****************************************/

   case VK_DOWN:
      SS_ScrollLineLR(lpSS, (short)(SS_F_SCROLL_DOWN |
                      (fSingleSel ? SS_F_MOVEACTIVECELL :
                      (fMoveActiveCell ? SS_F_MOVEACTIVECELL : 0))));

      if (SS_IsDestroyed(hWnd))
         return (FALSE);

#ifndef SS_NOLISTBOX
      if (lpSS->wOpMode == SS_OPMODE_EXTSEL && (!SS_IsF8ExtSelMode(lpSS) || fShiftDown))
         {
         if (!(wShift & 0x01))
            lpSS->ExtSelAnchorRow = lpSS->Row.CurAt;
         SS_SelectRow(hWnd, lpSS);
         }
#endif

      SS_UpdateWindow(lpSS->lpBook);
      dRet = FALSE;
      break;

   /*******************************************
   * Page Up key maps to vertical scroll code
   *******************************************/

   case VK_PRIOR:
#ifdef SS_V70
      if (fControlDown && lpSS->lpBook->nSheetCnt > 1)
			SS_SetActiveSheet(lpSS->lpBook, SSTab_GetPrevVisibleTab(lpSS->lpBook, lpSS->lpBook->nActiveSheet));

		else
#endif // SS_V70
			{
			if (!(lpSS->lpBook->wMode == SS_MODE_BLOCK))
				if (!SS_IsF8ExtSelMode(lpSS))
					SS_ResetBlock(lpSS);

			if (fControlDown || fAltDown)
				SS_ScrollPageUL(lpSS, (short)(SS_F_SCROLL_LEFT | 
					 (fMoveActiveCell ? SS_F_MOVEACTIVECELL : 0)));
			else
				SS_ScrollPageUL(lpSS, (short)(SS_F_SCROLL_UP |
					 (fSingleSel ? SS_F_MOVEACTIVECELL : 
					 (fMoveActiveCell ? SS_F_MOVEACTIVECELL : 0))));
			}

      if (SS_IsDestroyed(hWnd))
         return (FALSE);

#ifndef SS_NOLISTBOX
      if (lpSS->wOpMode == SS_OPMODE_EXTSEL && !SS_IsF8ExtSelMode(lpSS))
         {
         if (!(wShift & 0x01))
            lpSS->ExtSelAnchorRow = lpSS->Row.CurAt;
         SS_SelectRow(hWnd, lpSS);
         }
#endif

      SS_UpdateWindow(lpSS->lpBook);
      dRet = FALSE;
      break;

   /*********************************************
   * Page Down key maps to vertical scroll code
   *********************************************/

   case VK_NEXT:
#ifdef SS_V70
      if (fControlDown && lpSS->lpBook->nSheetCnt > 1)
			SS_SetActiveSheet(lpSS->lpBook, SSTab_GetNextVisibleTab(lpSS->lpBook, lpSS->lpBook->nActiveSheet));

		else
#endif // SS_V70
			{
			if (!(lpSS->lpBook->wMode == SS_MODE_BLOCK))
				if (!SS_IsF8ExtSelMode(lpSS))
					SS_ResetBlock(lpSS);

			if (fControlDown || fAltDown)
				SS_ScrollPageLR(lpSS, (short)(SS_F_SCROLL_RIGHT | 
									 (fMoveActiveCell ? SS_F_MOVEACTIVECELL : 0)));
			else
				SS_ScrollPageLR(lpSS, (short)(SS_F_SCROLL_DOWN |
									 (fSingleSel ? SS_F_MOVEACTIVECELL :
									 (fMoveActiveCell ? SS_F_MOVEACTIVECELL : 0))));
			}

      if (SS_IsDestroyed(hWnd))
         return (FALSE);

#ifndef SS_NOLISTBOX
      if (lpSS->wOpMode == SS_OPMODE_EXTSEL && !SS_IsF8ExtSelMode(lpSS))
         {
         if (!(wShift & 0x01))
            lpSS->ExtSelAnchorRow = lpSS->Row.CurAt;
         SS_SelectRow(hWnd, lpSS);
         }
#endif

      SS_UpdateWindow(lpSS->lpBook);
      dRet = FALSE;
      break;

   /****************************************
   * Home - Go to beginning of spreadsheet
   ****************************************/

   case VK_HOME:
      {
#ifdef SS_UTP
      BOOL fColsSelected = FALSE;

      if (SS_IsColsSelected(lpSS))
         fColsSelected = TRUE;
#endif

      if (!(lpSS->lpBook->wMode == SS_MODE_BLOCK) && (fMoveActiveCell ||
          lpSS->wOpMode == SS_OPMODE_EXTSEL))
			if (!SS_IsF8ExtSelMode(lpSS))
	         SS_ResetBlock(lpSS);

      SS_CellEditModeOff(lpSS, 0);

      if (SS_IsDestroyed(hWnd))
         return (FALSE);

      if (fControlDown)
         {
			SS_COORD RowAtTemp;
			SS_COORD ColAtTemp;
			SS_COORD RowAtNew;
			SS_COORD ColAtNew;
         SS_COORD RowTopOld = lpSS->Row.UL;
         SS_COORD ColLeftOld = lpSS->Col.UL;
         SS_COORD RowAtOld = lpSS->Row.CurAt;
         SS_COORD ColAtOld = lpSS->Col.CurAt;

         if (lpSS->lpBook->wMode == SS_MODE_BLOCK)
            {
            ColAtTemp = lpSS->Col.BlockCellCurrentPos;
            RowAtTemp = lpSS->Row.BlockCellCurrentPos;
            }
         else
            {
            ColAtTemp = lpSS->Col.CurAt;
            RowAtTemp = lpSS->Row.CurAt;
            }

         ColAtNew = lpSS->Col.HeaderCnt;
         RowAtNew = lpSS->Row.HeaderCnt;

         while (ColAtNew < SS_GetColCnt(lpSS) - 1 &&
                SS_GetColWidthInPixels(lpSS, ColAtNew) == 0)
            ColAtNew++;

#ifdef SS_UTP
         if (ColAtNew == lpSS->Col.HeaderCnt + lpSS->Col.Frozen - 1)
            {
            ColAtNew++;
            while (ColAtNew < SS_GetColCnt(lpSS) - 1 &&
                   SS_GetColWidthInPixels(lpSS, ColAtNew) == 0)
               ColAtNew++;
            }
#endif

         while (RowAtNew < SS_GetRowCnt(lpSS) - 1 &&
                SS_GetRowHeightInPixels(lpSS, RowAtNew) == 0)
            RowAtNew++;

         if ((lpSS->Col.UL == lpSS->Col.HeaderCnt +
              lpSS->Col.Frozen && ColAtTemp <= lpSS->Col.LR) &&
             (lpSS->Row.UL == lpSS->Row.HeaderCnt +
              lpSS->Row.Frozen && RowAtTemp <= lpSS->Row.LR))
            {
            if (lpSS->lpBook->wMode == SS_MODE_BLOCK)
               {
               if (lpSS->wOpMode == SS_OPMODE_EXTSEL)
                  lpSS->Row.CurAt = RowAtNew;
               SSx_SelectBlock(lpSS, ColAtNew, RowAtNew);
               }
            else if (fMoveActiveCell || fSingleSel)
               {
               SS_HighlightCell(lpSS, FALSE);
               lpSS->Col.CurAt = ColAtNew;
               lpSS->Row.CurAt = RowAtNew;
               if (SS_LeaveCell(lpSS, ColAtOld, ColLeftOld,
                                RowAtOld, RowTopOld, 0))
                  SS_HighlightCell(lpSS, TRUE);

               if (SS_IsDestroyed(hWnd))
                  return (FALSE);
               }
            }

         else if (lpSS->Row.UL > lpSS->Row.HeaderCnt +
                  lpSS->Row.Frozen || lpSS->Col.UL >
                  lpSS->Col.HeaderCnt + lpSS->Col.Frozen)
            {
				SS_COORD BlockCellCurrentCol;
				SS_COORD BlockCellCurrentRow;

            if (lpSS->lpBook->wMode == SS_MODE_BLOCK)
               {
               BlockCellCurrentRow = RowAtNew;
               BlockCellCurrentCol = ColAtNew;
               if (lpSS->wOpMode == SS_OPMODE_EXTSEL)
                  lpSS->Row.CurAt = RowAtNew;
               }
            else if (fMoveActiveCell || fSingleSel)
               {
               SS_HighlightCell(lpSS, FALSE);

               lpSS->Row.CurAt = RowAtNew;
               lpSS->Col.CurAt = ColAtNew;
               }

            lpSS->Row.UL = lpSS->Row.HeaderCnt + lpSS->Row.Frozen;
            lpSS->Col.UL = lpSS->Col.HeaderCnt + lpSS->Col.Frozen;
            SS_ResetCellSizeList(&lpSS->Row);
            SS_ResetCellSizeList(&lpSS->Col);

            if ((lpSS->lpBook->wMode == SS_MODE_BLOCK) || SS_LeaveCell(lpSS,
                ColAtOld, ColLeftOld, RowAtOld, RowTopOld, 0))
               {
               if (RowTopOld > lpSS->Row.HeaderCnt +
                   lpSS->Row.Frozen && ColLeftOld >
                   lpSS->Col.HeaderCnt + lpSS->Col.Frozen)
                  {
                  lpSS->lpBook->wScrollDirection = SS_VSCROLL_HOME;
                  SS_InvalidateScrollArea(lpSS, SS_SCROLLAREA_ALL);
                  SS_TopLeftChange(lpSS);
                  }

               else if (RowTopOld > lpSS->Row.HeaderCnt +
                        lpSS->Row.Frozen)
                  {
                  lpSS->lpBook->wScrollDirection = SS_VSCROLL_HOME;
                  SS_InvalidateScrollArea(lpSS, SS_SCROLLAREA_ROWS);
                  SS_TopRowChange(lpSS);
                  }

               else if (ColLeftOld > lpSS->Col.HeaderCnt +
                        lpSS->Col.Frozen)
                  {
                  SS_InvalidateScrollArea(lpSS, SS_SCROLLAREA_COLS);
                  SS_LeftColChange(lpSS);
                  }

               SS_SetVScrollBar(lpSS);
               SS_SetHScrollBar(lpSS);
               }

            if (SS_IsDestroyed(hWnd))
               return (FALSE);

            if (lpSS->lpBook->wMode == SS_MODE_BLOCK)
               SSx_SelectBlock(lpSS, BlockCellCurrentCol, BlockCellCurrentRow);
            }
         }

      else
         {
#ifdef SS_UTP
         if (fColsSelected)
            SS_ScrollHome(lpSS, SS_F_SCROLL_COL |
                          (fSingleSel ? SS_F_MOVEACTIVECELL :
                          (fMoveActiveCell ? SS_F_MOVEACTIVECELL : 0)));
         else
            SS_ScrollHome(lpSS, SS_F_SCROLL_ROW |
                          (fSingleSel ? SS_F_MOVEACTIVECELL :
                          (fMoveActiveCell ? SS_F_MOVEACTIVECELL : 0)));
#else
         SS_ScrollHome(lpSS, (short)(SS_F_SCROLL_ROW |
                       (fSingleSel ? SS_F_MOVEACTIVECELL :
                       (fMoveActiveCell ? SS_F_MOVEACTIVECELL : 0))));
#endif
         }

#ifndef SS_NOLISTBOX
      if (lpSS->wOpMode == SS_OPMODE_EXTSEL && !SS_IsF8ExtSelMode(lpSS))
         {
         if (!(wShift & 0x01))
            lpSS->ExtSelAnchorRow = lpSS->Row.CurAt;
         SS_SelectRow(hWnd, lpSS);
         }
#endif

      SS_UpdateWindow(lpSS->lpBook);
      dRet = FALSE;
      }
      break;

   /*********************************
   * End - Go to end of SpreadSheet
   *********************************/

   case VK_END:
      {
#ifdef SS_UTP
      BOOL fColsSelected = FALSE;

      if (SS_IsColsSelected(lpSS))
         fColsSelected = TRUE;
#endif

      if (!(lpSS->lpBook->wMode == SS_MODE_BLOCK) && (fMoveActiveCell ||
          lpSS->wOpMode == SS_OPMODE_EXTSEL))
			if (!SS_IsF8ExtSelMode(lpSS))
	         SS_ResetBlock(lpSS);

      SS_CellEditModeOff(lpSS, 0);

      if (SS_IsDestroyed(hWnd))
         return (FALSE);

      if (fControlDown)
         {
         SS_COORD RowTopOld = lpSS->Row.UL;
         SS_COORD ColLeftOld = lpSS->Col.UL;
         SS_COORD RowAtOld = lpSS->Row.CurAt;
         SS_COORD ColAtOld = lpSS->Col.CurAt;
			SS_COORD RowCnt;
			SS_COORD ColCnt;
			SS_COORD lColLastSpan = 0;
			SS_COORD lRowLastSpan = 0;
			BOOL     fSpan = FALSE;

			// RFW - 6/10/04 - 14340
#ifdef SS_V40
			fSpan = SS_GetSpanLR(lpSS, &lColLastSpan, &lRowLastSpan);
#endif // SS_V40

         ColCnt = lpSS->Col.DataCnt;
			if (fSpan)
				ColCnt = max(ColCnt, lColLastSpan + 1);

         if (ColCnt == lpSS->Col.HeaderCnt && ColCnt < SS_GetColCnt(lpSS))
            {
            ColCnt++;

            while (ColCnt - 1 < SS_GetColCnt(lpSS) - 1 &&
                   SS_GetColWidthInPixels(lpSS, ColCnt - 1) == 0)
               ColCnt++;
            }

         while (ColCnt - 1 > lpSS->Col.HeaderCnt &&
                SS_GetColWidthInPixels(lpSS, ColCnt - 1) == 0)
            ColCnt--;

         if (lpSS->fVirtualMode)
            RowCnt = SS_GetRowCnt(lpSS);
         else
				{
            RowCnt = lpSS->Row.DataCnt;
				if (fSpan)
					RowCnt = max(RowCnt, lRowLastSpan + 1);
				}

         if (RowCnt == lpSS->Row.HeaderCnt && RowCnt < SS_GetRowCnt(lpSS))
            RowCnt++;

         while (RowCnt - 1 > lpSS->Row.HeaderCnt &&
                SS_GetRowHeightInPixels(lpSS, RowCnt - 1) == 0)
            RowCnt--;

         if (ColCnt - 1 >= lpSS->Col.UL && ColCnt - 1 <= lpSS->Col.LRAllVis &&
             RowCnt - 1 >= lpSS->Row.UL && RowCnt - 1 <= lpSS->Row.LRAllVis)
            {
            if (lpSS->lpBook->wMode == SS_MODE_BLOCK)
               {
               if (lpSS->wOpMode == SS_OPMODE_EXTSEL)
                  lpSS->Row.CurAt = RowCnt - 1;
               SSx_SelectBlock(lpSS, ColCnt - 1, RowCnt - 1);
               }
            else if (fMoveActiveCell || fSingleSel)
               {
               SS_HighlightCell(lpSS, FALSE);
               lpSS->Col.CurAt = ColCnt - 1;
               lpSS->Row.CurAt = RowCnt - 1;
               if (SS_LeaveCell(lpSS, ColAtOld, ColLeftOld,
                                RowAtOld, RowTopOld, 0))
                  SS_HighlightCell(lpSS, TRUE);

               if (SS_IsDestroyed(hWnd))
                  return (FALSE);
               }
            }

         else
            {
				SS_COORD BlockCellCurrentCol;
				SS_COORD BlockCellCurrentRow;
				SS_COORD RowAt;
				SS_COORD ColAt;
				RECT     Rect;
				int      dWidth;
				int      dHeight;
				int      x;
				int      y;

            SS_GetClientRect(lpSS->lpBook, &Rect);
            SS_HighlightCell(lpSS, FALSE);

            /******************************
            * Determine new x coordinates
            ******************************/

				/* RFW - 5/20/04 - 14263
            x = SS_GetCellPosX(lpSS, lpSS->Col.CurAt, lpSS->Col.CurAt);
				*/
            x = SS_GetCellPosX(lpSS, lpSS->Col.HeaderCnt + lpSS->Col.Frozen,
                               lpSS->Col.HeaderCnt + lpSS->Col.Frozen);

            for (ColAt = ColCnt; ColAt > lpSS->Col.HeaderCnt +
                 lpSS->Col.Frozen && x <= Rect.right - Rect.left; )
               {
               if (--ColAt < lpSS->Col.HeaderCnt + lpSS->Col.Frozen)
                  break;

               dWidth = SS_GetColWidthInPixels(lpSS, ColAt);

               if (x + dWidth > Rect.right - Rect.left)
                  {
                  ColAt++;
                  break;
                  }

               x += dWidth;
               }

            ColAt = min(ColAt, SS_GetColCnt(lpSS) - 1);

            lpSS->Col.UL = max(lpSS->Col.HeaderCnt + lpSS->Col.Frozen, ColAt);
            SS_ResetCellSizeList(&lpSS->Col);

            if (lpSS->lpBook->wMode == SS_MODE_BLOCK)
               BlockCellCurrentCol = ColCnt - 1;
            else if (fMoveActiveCell)
               lpSS->Col.CurAt = ColCnt - 1;

            /******************************
            * Determine new y coordinates
            ******************************/
				/* RFW - 5/20/04 - 14263
            y = SS_GetCellPosY(lpSS, lpSS->Row.CurAt, lpSS->Row.CurAt);
				*/
            y = SS_GetCellPosY(lpSS, lpSS->Row.HeaderCnt + lpSS->Row.Frozen,
                               lpSS->Row.HeaderCnt + lpSS->Row.Frozen);

            for (RowAt = RowCnt; RowAt > lpSS->Row.HeaderCnt +
                 lpSS->Row.Frozen && y <= Rect.bottom - Rect.top; )
               {
               if (--RowAt < lpSS->Row.HeaderCnt + lpSS->Row.Frozen)
                  break;

               dHeight = SS_GetRowHeightInPixels(lpSS, RowAt);

               if (y + dHeight > Rect.bottom - Rect.top)
                  {
                  RowAt++;
                  break;
                  }

               y += dHeight;
               }

            RowAt = min(RowAt, SS_GetRowCnt(lpSS) - 1);

            lpSS->Row.UL = max(lpSS->Row.HeaderCnt + lpSS->Row.Frozen, RowAt);
            SS_ResetCellSizeList(&lpSS->Row);

            if (lpSS->lpBook->wMode == SS_MODE_BLOCK)
               {
               BlockCellCurrentRow = RowCnt - 1;
               if (lpSS->wOpMode == SS_OPMODE_EXTSEL)
                  lpSS->Row.CurAt = RowCnt - 1;
               }
            else if (fMoveActiveCell || fSingleSel)
               lpSS->Row.CurAt = RowCnt - 1;

            /**********************
            * RePaint SpreadSheet
            **********************/

            if (SS_LeaveCell(lpSS, ColAtOld, ColLeftOld, RowAtOld,
                RowTopOld, 0) || !fMoveActiveCell || (lpSS->lpBook->wMode == SS_MODE_BLOCK))
               {
               lpSS->lpBook->wScrollDirection = SS_VSCROLL_END;
               SS_InvalidateScrollArea(lpSS, SS_SCROLLAREA_ALL);

               SS_SetHScrollBar(lpSS);
               SS_SetVScrollBar(lpSS);
               SS_TopLeftChange(lpSS);

               if (lpSS->lpBook->wMode == SS_MODE_BLOCK)
                  SSx_SelectBlock(lpSS, BlockCellCurrentCol,
                                  BlockCellCurrentRow);
               }

            if (SS_IsDestroyed(hWnd))
               return (FALSE);
            }

         if (lpSS->Row.UL != RowTopOld)
            lpSS->lpBook->wScrollDirection = SS_VSCROLL_END;
         }

      else
         {
#ifdef SS_UTP
         if (fColsSelected)
            SS_ScrollEnd(lpSS, SS_F_SCROLL_COL |
                         (fSingleSel ? SS_F_MOVEACTIVECELL :
                         (fMoveActiveCell ? SS_F_MOVEACTIVECELL : 0)));
         else
            SS_ScrollEnd(lpSS, SS_F_SCROLL_ROW |
                         (fSingleSel ? SS_F_MOVEACTIVECELL :
                         (fMoveActiveCell ? SS_F_MOVEACTIVECELL : 0)));
#else
         SS_ScrollEnd(lpSS, (short)(SS_F_SCROLL_ROW |
                      (fSingleSel ? SS_F_MOVEACTIVECELL :
                      (fMoveActiveCell ? SS_F_MOVEACTIVECELL : 0))));
#endif
         }

#ifndef SS_NOLISTBOX
      if (lpSS->wOpMode == SS_OPMODE_EXTSEL && !SS_IsF8ExtSelMode(lpSS))
         {
         if (!(wShift & 0x01))
            lpSS->ExtSelAnchorRow = lpSS->Row.CurAt;
         SS_SelectRow(hWnd, lpSS);
         }
#endif

      SS_UpdateWindow(lpSS->lpBook);
      dRet = FALSE;
      }
      break;

   case VK_DELETE:
      if (fShiftDown)
         {
         if (lpSS->lpBook->fAutoClipboard)
            SSClipboardCut(hWnd);
         }

      lpSS->lpBook->wMsgLast = 0;
      return (0);

   case VK_INSERT:
      if (fShiftDown)
         {
         if (lpSS->lpBook->fAutoClipboard)
            SSClipboardPaste(hWnd);
         }

      else if (fControlDown)
         {
         if (lpSS->lpBook->fAutoClipboard)
            SSClipboardCopy(hWnd);
         }

      lpSS->lpBook->wMsgLast = 0;
      return (0);

   default:
      break;

   }

if (lpSS->wOpMode == SS_OPMODE_EXTSEL && (lpSS->lpBook->wMode == SS_MODE_BLOCK) &&
    (wParam == VK_UP || wParam == VK_DOWN || wParam == VK_PRIOR ||
     wParam == VK_NEXT || wParam == VK_HOME || wParam == VK_END))
   SS_HighlightCell(lpSS, TRUE);

if (dRet != -1)
   {
   if (lpSS->fInvertBlock && !(lpSS->lpBook->wMode == SS_MODE_BLOCK))
      {
      lpSS->fSuspendInvert = FALSE;
      SS_HighlightCell(lpSS, TRUE);
      SS_InvertBlock(0, lpSS);
      }

   lpSS->lpBook->wMsgLast = 0;   // RFW - Added for GRB383
   return (dRet);
   }

lpSS->lpBook->wMsgLast = 0;

return (-1);
}


static long SS_ProcessLButton(LPSPREADSHEET lpSS, HWND hWnd, UINT Msg, WPARAM wParam,
                              LPARAM lParam)
{
LPSS_BOOK     lpBook = lpSS->lpBook;
LPSPREADSHEET lpSSTemp;
SS_CELLCOORD  BlockCellUL;
SS_CELLCOORD  BlockCellLR;
LPSS_COL      lpCol;
SS_COORD      Row;
SS_COORD      Col;
WORD          MsgOrig;
RECT          Rect;
RECT          RectClient;
RECT          RectWindowClient;
HWND          hWndFocus;
BOOL          fDone = FALSE;
BOOL          fNoSetCapture = FALSE;
BOOL          fComboBtn = FALSE;
BOOL          fRet;
BOOL          Invert;
BOOL          fControlDown; 
BOOL          fHighlightOn;
int           MouseX;
int           MouseY;
int           Pos;
#ifdef SS_V80
SS_CELLCOORD oldUL, oldLR;
BOOL fReservedLocation = FALSE;
#endif
if (lpSS && IsWindowEnabled(hWnd))
   {
#ifdef SS_V80
  oldUL = lpSS->BlockCellUL;
  oldLR = lpSS->BlockCellLR;
#endif
   GetWindowRect(hWnd, &Rect);

   MouseX = (int)wParam;
   MouseY = LOWORD(lParam);

   if (Msg == SSM_PROCESSLBUTTONDOWN)
      Msg = WM_LBUTTONDOWN;
   else
      Msg = WM_LBUTTONDBLCLK;

   MsgOrig = Msg;

   lpBook->fKillFocusReceived = FALSE;
   lpBook->fProcessingLButtonDown = FALSE;
   lpSS->fSingleSelRowFound = FALSE;

   SS_GetClientRect(lpBook, &RectClient);
	GetClientRect(hWnd, &RectWindowClient);

#ifndef SS_UTP
	if (lpBook->fVScrollVisible && lpBook->fHScrollVisible &&
       MouseX >= RectClient.right && MouseY >= RectClient.bottom)
      return (0);
#endif

   lpBook->wMsgLast = WM_LBUTTONDOWN;
	lpBook->fHasMouseMoved = FALSE;

   if (Msg == WM_LBUTTONDOWN &&
       (Pos = SS_IsMouseInHorizResize(lpSS, &Rect, &RectClient, MouseX, MouseY,
                                      &Row)) != -1)
      {
      SS_COORD RowAt = Row;
      SS_COORD RowAtTemp = Row;

      while ((RowAtTemp + 1 <= SS_GetRowCnt(lpSS) - 1) &&
             (SS_GetRowHeightInPixels(lpSS, RowAtTemp + 1) == 0))
         {
         RowAtTemp++;
         if (SS_GetUserResizeRow(lpSS, RowAtTemp) != SS_RESIZE_OFF)
            RowAt = RowAtTemp;
         }

      if (RowAt <= SS_GetRowCnt(lpSS) - 1)
         Row = RowAt;

      if ((Row < lpSS->BlockCellUL.Row || Row > lpSS->BlockCellLR.Row) &&
          lpSS->wOpMode != SS_OPMODE_EXTSEL &&
          !SS_IsAllSelected(lpSS))
         SS_ResetBlock(lpSS);

      SS_CellEditModeOff(lpSS, 0);
      SetCapture(hWnd);
      lpBook->wMode = SS_MODE_RESIZE_HORIZ;
      lpBook->ResizeOffset = MouseY - Pos;
      lpBook->ResizeStartPos = Pos;
      lpBook->ResizeMinPos = Pos - SS_GetRowHeightInPixels(lpSS, Row);
      lpBook->ResizeCoord = Row;
      SS_Resize(hWnd, lpSS, SS_RESIZEMODE_START,
                MAKELONG(MouseX, MouseY));
      }

   else if (Msg == WM_LBUTTONDOWN &&
            (Pos = SS_IsMouseInVertResize(lpSS, &Rect,
                                          &RectClient, MouseX, MouseY,
                                          &Col)) != -1)
      {
      SS_COORD ColAt = Col;
      SS_COORD ColAtTemp = Col;

      while ((ColAtTemp + 1 <= SS_GetColCnt(lpSS) - 1) &&
             (SS_GetColWidthInPixels(lpSS, ColAtTemp + 1) == 0))
         {
         ColAtTemp++;
         if (SS_GetMergedUserResizeCol(lpSS, ColAtTemp) != SS_RESIZE_OFF)
            ColAt = ColAtTemp;
         }

      if (ColAt <= SS_GetColCnt(lpSS) - 1)
         Col = ColAt;

      if ((Col < lpSS->BlockCellUL.Col || Col > lpSS->BlockCellLR.Col) &&
          lpSS->wOpMode != SS_OPMODE_EXTSEL &&
          !SS_IsAllSelected(lpSS))
         SS_ResetBlock(lpSS);

      SS_CellEditModeOff(lpSS, 0);
      SetCapture(hWnd);
      lpBook->wMode = SS_MODE_RESIZE_VERT;
      lpBook->ResizeOffset = MouseX - Pos;
      lpBook->ResizeStartPos = Pos;
      lpBook->ResizeMinPos = Pos - SS_GetColWidthInPixels(lpSS, Col);
      lpBook->ResizeCoord = Col;
      SS_Resize(hWnd, lpSS, SS_RESIZEMODE_START,
                MAKELONG(MouseX, MouseY));
      }

#ifdef SS_V70

	// This allows double clicking the vertical resize to auto size the column.
   else if (Msg == WM_LBUTTONDBLCLK &&
            (Pos = SS_IsMouseInVertResize(lpSS, &Rect,
                                          &RectClient, MouseX, MouseY,
                                          &Col)) != -1)
		{
	   if (Col >= lpSS->Col.HeaderCnt)
			{
			double   dfColWidth;
			SS_COORD Col2 = Col;
			SS_COORD i;

			if (SS_IsAllSelected(lpSS))
				{
				Col = lpSS->Col.HeaderCnt;
				/* RFW - 8/11/04 - 15031
				Col2 = lpSS->Col.Max;
				*/
				Col2 = SS_GetColCnt(lpSS) - 1;
				}

         else if (lpSS->BlockCellUL.Col != -1 &&
                  lpSS->BlockCellUL.Row == 0 &&
                  lpSS->BlockCellLR.Row == -1)
            {
            Col = max(lpSS->Col.HeaderCnt, lpSS->BlockCellUL.Col);
            Col2 = lpSS->BlockCellLR.Col;
            }

			for (i = Col; i <= Col2; i++)
				{
				SS_GetMaxTextColWidth(lpSS, i, &dfColWidth);
				if (dfColWidth > 0)
					{
					// RFW - 4/22/04 - 14131
					if (SS_SetColWidth(lpSS, i, dfColWidth))
						SS_SendMsgColCoordRange(lpSS, SSM_COLWIDTHCHANGE, GetWindowID(hWnd),
														i, i);
					lpBook->wMode = SS_MODE_IDLE; // RFW - -8/3/04 - 14853
					}
				}
			}
		}

   else if ((Msg == WM_LBUTTONDOWN || Msg == WM_LBUTTONDBLCLK) && SS_IsPointInTabStripRect(lpBook, MouseX, MouseY))
      {
		RECT Rect;

      SS_CellEditModeOff(lpSS, 0);
      SetCapture(hWnd);
      lpBook->wMode = SS_MODE_TAB;
		SS_GetTabStripRect(lpBook, &Rect);
		SSTab_OnLButtonDown(lpBook->hTabStrip, wParam, MAKELONG(wParam, LOWORD(lParam)), &Rect);
      }

   else if (Msg == WM_LBUTTONDOWN && SS_IsPointInTabSplitBoxRect(lpBook, MouseX, MouseY))
      {
		RECT Rect;

      SS_CellEditModeOff(lpSS, 0);
      SetCapture(hWnd);
      lpBook->wMode = SS_MODE_TAB_RESIZE;
		SS_GetTabSplitBoxRect(lpBook, &Rect);
		lpBook->iTabResizeOffset = (int)(wParam - Rect.left);
      }
#endif // SS_V70

#ifndef SS_NODRAGDROP
   else if (Msg == WM_LBUTTONDOWN && SS_IsMouseInDragDrop(hWnd,
            lpSS, &RectClient, MouseX, MouseY, &Col, &Row))
      {
      SetCapture(hWnd);
      lpBook->wMode = SS_MODE_DRAGDROP;
      lpBook->DragDropCurrentCol = Col;
      lpBook->DragDropCurrentRow = Row;

#ifdef SS_V30
      lpBook->hCursorCurrent = lpBook->CursorDragDrop.hCursor;
      SendMessage(hWnd, WM_SETCURSOR, (WPARAM)hWnd, 0);
#endif

      if (SS_USESELBAR(lpSS))
         {
         lpSS->DragDropBlockUL.Col = SS_ALLCOLS;
         lpSS->DragDropBlockUL.Row = lpSS->Row.CurAt;
         }
      else if (SS_IsBlockSelected(lpSS))
         {
         lpSS->DragDropBlockUL.Col = lpSS->BlockCellUL.Col;
         lpSS->DragDropBlockUL.Row = lpSS->BlockCellUL.Row;
         }
      else
         {
         lpSS->DragDropBlockUL.Col = lpSS->Col.CurAt;
         lpSS->DragDropBlockUL.Row = lpSS->Row.CurAt;
         }
      }
#endif

#ifdef SS_UTP
   else if (SS_IsMouseInScrollArrow(hWnd, lpSS, &RectClient, MouseX,
            MouseY, &lpSS->bModeScrollArrow, 0))
      {
      SS_DrawArrow(hWnd, 0, lpSS, lpSS->bModeScrollArrow, TRUE);
      SetCapture(hWnd);
      }
#endif

   else
      {
#ifdef SS_V35
      short nUserColAction;
      int nIndicator;
#endif
      SS_COORD ColSave = lpSS->Col.CurAt;
      SS_COORD RowSave = lpSS->Row.CurAt;
		int x;
		int y;

      SS_GetCellFromPixel(lpSS, &Col, &Row, &x, &y, MouseX, MouseY);

#ifdef SS_V35

      nUserColAction = SS_GetUserColAction(lpSS);
      nIndicator = SS_GetMergedColUserSortIndicator(lpSS, Col);

#ifdef SS_V70
      if (WM_LBUTTONDBLCLK == Msg && SS_USERCOLACTION_AUTOSIZE == nUserColAction)
			{
	      if (Col >= lpSS->Col.HeaderCnt && Row >= 0 && 
             Row < lpSS->Row.HeaderCnt)
				{
				double dfColWidth;

            SS_GetMaxTextColWidth(lpSS, Col, &dfColWidth);
				if (dfColWidth > 0)
					{
					SS_SetColWidth(lpSS, Col, dfColWidth);
					// RFW - 4/22/04 - 14131
	            SS_SendMsgColCoordRange(lpSS, SSM_COLWIDTHCHANGE, GetWindowID(hWnd),
		                                 Col, Col);
					}
				}
			}

		if (Col >= lpSS->Col.HeaderCnt && Row >= 0 && 
          Row < lpSS->Row.HeaderCnt && lpSS->fAllowColMove)
			{
			if (SS_IsColMove(lpSS, Col, Row))
				{
				SS_ResetBlock(lpSS);
				SS_CellEditModeOff(lpSS, 0);
				SS_HighlightCell(lpSS, FALSE);
				lpBook->wMode = SS_MODE_BEGINCOLMOVE;
				SS_GetActualCellEx(lpSS, Col, Row, &lpSS->MoveStartRange, NULL);
				lpSS->ColMoveCurrentCol = lpSS->MoveStartRange.UL.Col;
				SetCapture(hWnd);
				}
			return (0);
			}

		else if (Row >= lpSS->Row.HeaderCnt && Col >= 0 && 
               Col < lpSS->Col.HeaderCnt && lpSS->fAllowRowMove)
			{
			if (SS_IsRowMove(lpSS, Col, Row))
				{
				SS_ResetBlock(lpSS);
				SS_CellEditModeOff(lpSS, 0);
				SS_HighlightCell(lpSS, FALSE);
				lpBook->wMode = SS_MODE_BEGINROWMOVE;
				SS_GetActualCellEx(lpSS, Col, Row, &lpSS->MoveStartRange, NULL);
				lpSS->RowMoveCurrentRow = lpSS->MoveStartRange.UL.Row;
				SetCapture(hWnd);
				}
			return (0);
			}

		else

#endif // SS_V70

      if (Col >= lpSS->Col.HeaderCnt && Row >= 0 && 
          Row < lpSS->Row.HeaderCnt &&
          (SS_USERCOLACTION_SORT == nUserColAction ||
           SS_USERCOLACTION_SORTNOINDICATOR == nUserColAction) && 
#ifdef SS_V40
          SS_COLUSERSORTINDICATOR_DISABLED != nIndicator)
#else
          SS_COLUSERSORTINDICATOR_DISABLED != nIndicator && SS_COLUSERSORTINDICATOR_NONE != nIndicator)
#endif // SS_V40
         {
// This was moved to WM_LBUTTONUP for SS_V70
#ifndef SS_V70
         SS_BEFOREUSERSORT beforeSort;
			SS_COORD ActiveColOld = lpSS->Col.CurAt;

         beforeSort.lCol = Col;
         beforeSort.lState = nIndicator;
         beforeSort.lDefaultAction = BEFOREUSERSORT_DEFAULTACTION_AUTOSORT;

         if (SS_SendMsgCoords(lpSS, SSM_CLICK, GetDlgCtrlID(hWnd), Col, Row))
            return (FALSE);
  
			// RFW - 11/14/05 - In response to a customer complaint that the change event fired after BeforeUserSort
			SS_CellEditModeOff(lpSS, 0);
			SS_AdjustCellCoordsOut(lpSS, &beforeSort.lCol, NULL);
         SS_SendMsg(lpBook, lpSS, SSM_BEFOREUSERSORT, GetWindowID(lpBook->hWnd), 
                (LPARAM)(LPSS_BEFOREUSERSORT)&beforeSort);

         SS_SetActiveCell(lpSS, Col, lpSS->Row.HeaderCnt);

         if (BEFOREUSERSORT_DEFAULTACTION_AUTOSORT == beforeSort.lDefaultAction)
            {
            SS_SORT sort = {SS_SORT_ROW, Col, nIndicator == SS_COLUSERSORTINDICATOR_ASCENDING ? SS_SORT_DESCENDING : SS_SORT_ASCENDING, -1, SS_SORT_NONE, -1, SS_SORT_NONE };
				BOOL fRet;

				SS_AdjustCellCoordsOut(lpSS, &sort.Key1Reference, NULL);
            fRet = SSSort(lpBook->hWnd, SS_ALLCOLS, SS_ALLROWS, SS_ALLCOLS, SS_ALLROWS, &sort);
				if (fRet)
					{
#ifdef SS_V40
	            SS_SetColUserSortIndicator(lpSS, -1, SS_COLUSERSORTINDICATOR_NONE);
#endif // SS_V40
					SS_SetColUserSortIndicator(lpSS, Col, (short)(nIndicator == SS_COLUSERSORTINDICATOR_ASCENDING ? SS_COLUSERSORTINDICATOR_DESCENDING : SS_COLUSERSORTINDICATOR_ASCENDING));
					}
            }

	     SS_SendMsg(lpBook, lpSS, SSM_AFTERUSERSORT, GetWindowID(lpBook->hWnd), (LPARAM)beforeSort.lCol);
#ifndef SS_V40
			SS_InvalidateRow(lpSS, SS_GetColHeadersUserSortRow(lpSS));
#endif // SS_V40

			// RFW - 5/13/03 - 11935
         lpBook->wMode = SS_MODE_NONE;
#else // !SS_V70
         lpBook->wMode = SS_MODE_USERSORT;

			// RFW - 7/22/04 - 14910
         if (SS_SendMsgCoords(lpSS, SSM_CLICK, GetDlgCtrlID(hWnd), Col, Row))
            return (FALSE);
         if (SS_IsDestroyed(hWnd))
            return (FALSE);

			// RFW - 7/29/04 - 14905
	      if (Msg == WM_LBUTTONDBLCLK)
				{
				if (SS_SendMsgCoords(lpSS, SSM_DBLCLK, GetDlgCtrlID(hWnd), Col, Row))
					return (FALSE);
				if (SS_IsDestroyed(hWnd))
					return (FALSE);
				}

#endif // !SS_V70

			return (0);
         }
      else
         {
#endif // SS_V35

      if (Col == -2 || Row == -2)
         return (0);

		// RFW - 1/1/02 - 9388
		if (Col == lpSS->Col.CurAt && Row == lpSS->Row.CurAt && lpBook->EditModeOn)
			return (0);

/*
      if (Col < lpSS->Col.HeaderCnt)
         Col = -1;

      if (Row < lpSS->Row.HeaderCnt)
         Row = -1;
*/

      fControlDown = FALSE;
      if (HIBYTE(GetKeyState(VK_CONTROL)) &&
          lpBook->fAllowMultipleSelBlocks && !SS_USESELBAR(lpSS))
         fControlDown = TRUE;

#ifdef SS_UTP
      if (HIBYTE(GetKeyState(VK_SHIFT)) && Msg == WM_LBUTTONDOWN &&
          !SS_IsBlockSelected(lpSS))
         ;

      else
#endif

#ifdef SPREAD_JPN
      //- JPNFIX0010 - (Masanori Iwasa)
      else
      if ((HIBYTE(GetKeyState(VK_SHIFT)) || HIBYTE(GetKeyState(VK_CONTROL))) &&
             SS_SELBLOCKOPTION(lpSS) <= SS_SELBLOCK_ROWS + SS_SELBLOCK_ROWS &&
             !SS_IsBlockSelected(lpSS) && (Col  < lpSS->Col.HeaderCnt || Row < lpSS->Row.HeaderCnt ) &&
             Msg == WM_LBUTTONDOWN)

         ;
      else
#endif

      if (HIBYTE(GetKeyState(VK_SHIFT)) && Msg == WM_LBUTTONDOWN &&
          !SS_USESELBAR(lpSS) &&
          (((SS_SELBLOCKOPTION(lpSS) & SS_SELBLOCK_BLOCKS) && Col >= lpSS->Col.HeaderCnt &&
             Row >= lpSS->Row.HeaderCnt) ||
           ((SS_SELBLOCKOPTION(lpSS) & SS_SELBLOCK_COLS) && Col >= lpSS->Col.HeaderCnt &&
             Row < lpSS->Row.HeaderCnt) ||
           ((SS_SELBLOCKOPTION(lpSS) & SS_SELBLOCK_ROWS) && Col < lpSS->Col.HeaderCnt &&
             Row >= lpSS->Row.HeaderCnt)))
         {
         BOOL fContinue = FALSE;

         if (!fContinue && !SS_IsBlockSelected(lpSS))
            {
            /*
            if (!(SS_SELBLOCKOPTION(lpSS) & SS_SELBLOCK_BLOCKS) &&
                (((SS_SELBLOCKOPTION(lpSS) & SS_SELBLOCK_COLS) && Col >= lpSS->Col.HeaderCnt &&
                   Row < lpSS->Row.HeaderCnt) ||
                 ((SS_SELBLOCKOPTION(lpSS) & SS_SELBLOCK_ROWS) &&
                   Col < lpSS->Col.HeaderCnt && Row >= lpSS->Row.HeaderCnt)))
            */
            if ((((SS_SELBLOCKOPTION(lpSS) & SS_SELBLOCK_COLS) && Col >= lpSS->Col.HeaderCnt &&
                   Row < lpSS->Row.HeaderCnt) ||
                 ((SS_SELBLOCKOPTION(lpSS) & SS_SELBLOCK_ROWS) &&
                   Col < lpSS->Col.HeaderCnt && Row >= lpSS->Row.HeaderCnt)))
               fContinue = TRUE;
            else
               {
               lpSS->BlockCellUL.Row = lpSS->Row.CurAt;
               lpSS->BlockCellUL.Col = lpSS->Col.CurAt;

               lpSS->BlockCellLR.Row = lpSS->Row.CurAt;
               lpSS->BlockCellLR.Col = lpSS->Col.CurAt;

               lpSS->Row.BlockCellCurrentPos = lpSS->Row.CurAt;
               lpSS->Col.BlockCellCurrentPos = lpSS->Col.CurAt;
               }
            }
        
         if (!fContinue)
            {
            if (lpSS->fMultipleBlocksSelected)
               lpSS->MultiSelBlock.dItemCnt--;

            lpBook->wMode = SS_MODE_BLOCK;

            SetCapture(hWnd);
            SS_SelectBlock(hWnd, lpSS, MouseX, MouseY);
            SS_HighlightCell(lpSS, TRUE);

            SS_SendMsgCoords(lpSS, SSM_CLICK, GetDlgCtrlID(hWnd), Col, Row);

            return (0);
            }
         }

      /************************
      * Select Block of Cells
      ************************/

      // BJO 12Jun98 GIC5787 - Begin fix (moved to after edit mode off)
      //if (lpSS->fAllowMultipleSelBlocks && fControlDown &&
      //    Msg != WM_LBUTTONDBLCLK &&
      //    (((SS_SELBLOCKOPTION(lpSS) & SS_SELBLOCK_BLOCKS) && Col >= lpSS->Col.HeaderCnt && Row >= lpSS->Row.HeaderCnt) ||
      //     ((SS_SELBLOCKOPTION(lpSS) & SS_SELBLOCK_ROWS) && Col < lpSS->Col.HeaderCnt) ||
      //     ((SS_SELBLOCKOPTION(lpSS) & SS_SELBLOCK_COLS) && Row < lpSS->Row.HeaderCnt)))
      //   SS_BeginMultipleSelection(hWnd, lpSS);
      //else if (lpSS->wOpMode != SS_OPMODE_EXTSEL)
      //   SS_ResetBlock(lpSS);
      // BJO 12Jun98 GIC5787 - End fix

//      if (fControlDown)
//         SS_BeginMultipleSelection(hWnd, lpSS);

      if (Msg == WM_LBUTTONDBLCLK && Row == lpSS->Row.CurAt &&
          Col == lpSS->Col.CurAt)
         {
         // BJO 12Jun98 GIC5787 - Begin fix (moved from before edit mode off)
         if (lpBook->fAllowMultipleSelBlocks && fControlDown &&
             Msg != WM_LBUTTONDBLCLK &&
             (((SS_SELBLOCKOPTION(lpSS) & SS_SELBLOCK_BLOCKS) && Col >= lpSS->Col.HeaderCnt && Row >= lpSS->Row.HeaderCnt) ||
              ((SS_SELBLOCKOPTION(lpSS) & SS_SELBLOCK_ROWS) && Col < lpSS->Col.HeaderCnt) ||
              ((SS_SELBLOCKOPTION(lpSS) & SS_SELBLOCK_COLS) && Row < lpSS->Row.HeaderCnt)))
            SS_BeginMultipleSelection(hWnd, lpSS);
         else if (lpSS->wOpMode != SS_OPMODE_EXTSEL)
            SS_ResetBlock(lpSS);
         // BJO 12Jun98 GIC5787 - End fix
         }

      else
         {
         if (!SS_CellEditModeOff(lpSS, 0))
                return (0);

         // BJO 12Jun98 GIC5787 - Begin fix (moved from before edit mode off)
			/* RFW - 8/2/05 - 16445
         if (lpBook->fAllowMultipleSelBlocks && fControlDown &&
             Msg != WM_LBUTTONDBLCLK &&
             (((SS_SELBLOCKOPTION(lpSS) & SS_SELBLOCK_BLOCKS) && Col >= lpSS->Col.HeaderCnt && Row >= lpSS->Row.HeaderCnt) ||
              ((SS_SELBLOCKOPTION(lpSS) & SS_SELBLOCK_ROWS) && Col < lpSS->Col.HeaderCnt) ||
              ((SS_SELBLOCKOPTION(lpSS) & SS_SELBLOCK_COLS) && Row < lpSS->Row.HeaderCnt)))
			*/
         if (lpBook->fAllowMultipleSelBlocks && fControlDown && !SS_IsMouseInButton(lpSS, &RectClient, MouseX, MouseY) &&
             Msg != WM_LBUTTONDBLCLK &&
             (((SS_SELBLOCKOPTION(lpSS) & SS_SELBLOCK_BLOCKS) && Col >= lpSS->Col.HeaderCnt && Row >= lpSS->Row.HeaderCnt) ||
              ((SS_SELBLOCKOPTION(lpSS) & SS_SELBLOCK_ROWS) && Col < lpSS->Col.HeaderCnt) ||
              ((SS_SELBLOCKOPTION(lpSS) & SS_SELBLOCK_COLS) && Row < lpSS->Row.HeaderCnt)))

            SS_BeginMultipleSelection(hWnd, lpSS);
         else if (lpSS->wOpMode != SS_OPMODE_EXTSEL)
            SS_ResetBlock(lpSS);
         // BJO 12Jun98 GIC5787 - End fix

         if (SS_ALLOWSELBLOCK(lpSS) && Row != -2 &&
             Col != -2 && lpSS->Col.Max > 0 &&
             lpSS->Row.Max > 0)
            {
            lpBook->wMode = SS_MODE_BLOCK;

            BlockCellUL.Row = Row;
            BlockCellUL.Col = Col;

            BlockCellLR.Row = Row;
            BlockCellLR.Col = Col;
            }

         Invert = FALSE;

         if (Col == -2 || Row == -2)
            ;

         else if (Col >= lpSS->Col.HeaderCnt && Row >= lpSS->Row.HeaderCnt)
            {
				SS_COORD RowAtOld, RowCell;
				SS_COORD ColAtOld, ColCell;
				SS_CELLTYPE CellType;

            SS_HighlightCell(lpSS, FALSE);
            RowAtOld = lpSS->Row.CurAt;
            ColAtOld = lpSS->Col.CurAt;

				// RFW - 10/27/05 - 17125
				SS_GetActualCell(lpSS, Col, Row, &ColCell, &RowCell, NULL);
            if( Col != ColCell || Row != RowCell )
            {  // update x & y -scl
               RECT rcCell;
               SS_GetCellRect(lpSS, ColCell, RowCell, &rcCell);
               x = rcCell.left;
               y = rcCell.top;
            }

            if (Col != -2 && Row != -2 && Col >= lpSS->Col.HeaderCnt && Row >= lpSS->Row.HeaderCnt &&
                SS_RetrieveCellType(lpSS, &CellType, NULL, ColCell, RowCell) &&
               (CellType.Type == SS_TYPE_BUTTON || 
#if SS_V80
               CellType.Type == SS_TYPE_CUSTOM || 
#endif
               CellType.Type == SS_TYPE_CHECKBOX) &&
                (lpSS->wOpMode != SS_OPMODE_ROWMODE ||
                 lpSS->fRowModeEditing) &&
                !SS_GetLock(lpSS, ColCell, RowCell, FALSE))
					{

					Col = ColCell;
					Row = RowCell;
					}

            lpSS->Row.CurAt = Row;
            lpSS->Col.CurAt = Col;
            ColSave = lpSS->Col.CurAt;
            RowSave = lpSS->Row.CurAt;
            fRet = SS_LeaveCell(lpSS, ColAtOld, -1, RowAtOld, -1, 0);

            if (SS_IsDestroyed(hWnd))
               return (FALSE);
            
            // Fix for A-000739 (Setting ActiveCell in LeaveCell causes
            // highlight problem)
            if (fHighlightOn = lpSS->HighlightOn)
               SS_HighlightCell(lpSS, FALSE);

            lpBook->EditModeTurningOff = TRUE;
            lpSS->FreezeHighlight = TRUE;
            SS_UpdateWindow(lpBook);
            lpSS->FreezeHighlight = FALSE;
            lpBook->EditModeTurningOff = FALSE;
            
            // Fix for A-000739
            if (fHighlightOn)
               SS_HighlightCell(lpSS, TRUE);

            if (fRet)
               {
					SS_CELLTYPE   CellType;
					LPSS_CELLTYPE lpCellType;
					SS_COORD      ColCell;
					SS_COORD      RowCell;

					SS_GetActualCell(lpSS, Col, Row, &ColCell, &RowCell, NULL);

               lpCellType = SS_RetrieveCellType(lpSS, &CellType, NULL, ColCell, RowCell);

               if (SS_USESELBAR(lpSS))
                  {
                  SS_HighlightCell(lpSS, TRUE);
                  lpSS->fSingleSelRowFound = TRUE;

#ifndef SS_NOLISTBOX
                  if (lpSS->wOpMode == SS_OPMODE_MULTISEL)
                     SS_SelectRow(hWnd, lpSS);
                  else if (lpSS->wOpMode == SS_OPMODE_EXTSEL)
                     SS_ExtSelBegin(hWnd, lpSS);
#endif
                  }
#if SS_V80
               else if( lpCellType && 
                 CellType.Type == SS_TYPE_CUSTOM &&
                 (lpBook->fEditModePermanent ||
                   SS_IsMouseInReservedLocation(lpSS, &RectClient,
                                      MouseX, MouseY, NULL)))
               {
                 fReservedLocation = TRUE;
                  fNoSetCapture = TRUE;
                  //Msg = WM_LBUTTONDBLCLK; // 27026 -scl
               }
#endif
               else if (lpCellType &&
                   CellType.Type == SS_TYPE_COMBOBOX &&
                   (lpBook->fEditModePermanent ||
                   SS_IsMouseInButton(lpSS, &RectClient,
                                      MouseX, MouseY)))
                  {
                  fComboBtn = TRUE;
                  fNoSetCapture = TRUE;
                  Msg = WM_LBUTTONDBLCLK;
                  }

               else if (lpCellType &&
                        ((CellType.Type == SS_TYPE_BUTTON ||
                        CellType.Type == SS_TYPE_CHECKBOX) &&
                        (lpSS->wOpMode != SS_OPMODE_ROWMODE ||
                         lpSS->fRowModeEditing) &&
                        !SS_GetLock(lpSS, ColCell, RowCell, FALSE)))
                  ;

               /*
               else if (lpCellType && CellType.Type == SS_TYPE_DATE &&
                        lpBook->fEditModePermanent)
                  SS_HighlightDatePermanent(hWnd);
               */

               else
                  {
                  lpSS->lLButtonDownlParam = MAKELONG(MouseX - x, MouseY - y);
#if (!defined(SS_GP))
                  if (lpBook->fEditModePermanent)
                     SS_ShowActiveCell(lpSS, SS_SHOW_NEAREST);
#endif
                  SS_HighlightCell(lpSS, TRUE);
                  lpSS->lLButtonDownlParam = 0;
                  lpSS->fSingleSelRowFound = TRUE;
                  }

               if (!(SS_SELBLOCKOPTION(lpSS) & SS_SELBLOCK_BLOCKS))
                  lpBook->wMode = SS_MODE_NONE;
               }
            else
               {
               SS_SendMsgCoords(lpSS, SSM_CLICK, GetDlgCtrlID(hWnd), Col, Row);

               if (IsWindow(hWnd))
                  {
                  SS_UpdateWindow(lpBook);
                  lpBook->wMode = SS_MODE_NONE;
                  }

               return (0);
               }
            }

         else if ((lpSS->wOpMode == SS_OPMODE_SINGLESEL ||
                   lpSS->wOpMode == SS_OPMODE_ROWMODE ||
                   (lpSS->wOpMode == SS_OPMODE_ROWMODE &&
                    !lpSS->fRowModeEditing) ||
                   lpSS->wOpMode == SS_OPMODE_MULTISEL ||
                   lpSS->wOpMode == SS_OPMODE_EXTSEL) &&
                   Col < lpSS->Col.HeaderCnt && Row >= lpSS->Row.HeaderCnt)
//                      Col < lpSS->Col.HeaderCnt && Row >= lpSS->Row.HeaderCnt && lpSS->Row.CurAt != Row)
            {
				SS_COORD RowAtOld;

            SS_HighlightCell(lpSS, FALSE);
            RowAtOld = lpSS->Row.CurAt;
            lpSS->Row.CurAt = Row;

            if (!SS_LeaveCell(lpSS, -1, -1, RowAtOld, -1, 0))
               ;

            if (SS_IsDestroyed(hWnd))
               return (FALSE);

#ifndef SS_NOLISTBOX
            SS_HighlightCell(lpSS, TRUE);
            lpSS->fSingleSelRowFound = TRUE;

            if (lpSS->wOpMode == SS_OPMODE_MULTISEL)
               SS_SelectRow(hWnd, lpSS);

            else if (lpSS->wOpMode == SS_OPMODE_EXTSEL)
               SS_ExtSelBegin(hWnd, lpSS);

            else
#endif
               {
//               SS_HighlightCell(lpSS, TRUE);
//               lpSS->fSingleSelRowFound = TRUE;
               }

            if (SS_IsDestroyed(hWnd))
               return (FALSE);

            if (SS_SendMsgCoords(lpSS, SSM_CLICK, GetDlgCtrlID(hWnd), Col, Row))
               return (FALSE);

            if (SS_IsDestroyed(hWnd))
               return (FALSE);
            else
               SS_UpdateWindow(lpBook);

            fDone = TRUE;
            }

         else if (SS_ALLOWSELBLOCK(lpSS))
            {
            SS_COORD RowAtOld = lpSS->Row.CurAt;
            SS_COORD ColAtOld = lpSS->Col.CurAt;
		      SS_COORD ColAtTemp;

            if (Col < lpSS->Col.HeaderCnt && Row >= lpSS->Row.HeaderCnt)
               {
#ifdef SS_UTP
					LPSS_ROW lpRow;

               if ((SS_SELBLOCKOPTION(lpSS) & SS_SELBLOCK_ROWS) &&
                   (!(lpRow = SS_LockRowItem(lpSS, Row)) ||
                   !lpRow->fNonSelectable))
#else
               if (SS_SELBLOCKOPTION(lpSS) & SS_SELBLOCK_ROWS)
#endif // SS_UTP
                  {
                  BlockCellUL.Col = 0;
						BlockCellLR.Col = -1;

                  if (SS_GetFirstValidCol(lpSS, Row,
                      &ColAtTemp) && (lpSS->Col.CurAt != ColAtTemp ||
                      lpSS->Row.CurAt != Row))
                     {
                     SS_HighlightCell(lpSS, FALSE);
                     lpSS->Row.CurAt = Row;
                     lpSS->Col.CurAt = ColAtTemp;
                     ColSave = lpSS->Col.CurAt;
                     RowSave = lpSS->Row.CurAt;
                     if (!SS_LeaveCell(lpSS, ColAtOld, -1, RowAtOld, -1, 0))
                        {
                        lpBook->wMode = SS_MODE_NONE;
                        return (0);
                        }

#ifdef SS_UTP
                     if (!lpSS->fAllowEditModePermSel)
#endif
                        SS_HighlightCell(lpSS, TRUE);
                     }
                  }
               else
                  lpBook->wMode = SS_MODE_NONE;
               }

         else if (Col >= lpSS->Col.HeaderCnt && Row < lpSS->Row.HeaderCnt)
            {
               if ((SS_SELBLOCKOPTION(lpSS) & SS_SELBLOCK_COLS) &&
                   (!(lpCol = SS_LockColItem(lpSS, Col)) ||
                   !lpCol->fNonSelectable))
                  {
				      SS_COORD RowAtTemp;

                  BlockCellUL.Row = 0;
						BlockCellLR.Row = -1;
/*
#ifdef SS_V40
						{
						SS_SELBLOCK Block;
						if (SS_SpanCalcBlock(lpSS, Col, Row, &Block))
							{
							BlockCellUL.Col = Block.UL.Col;
							BlockCellLR.Col = Block.LR.Col;
							}
						}
#endif // SS_V40
*/
                  if (SS_GetFirstValidRow(lpSS, Col,
                      &RowAtTemp) && (lpSS->Row.CurAt != RowAtTemp ||
                      lpSS->Col.CurAt != Col))
                     {
                     SS_HighlightCell(lpSS, FALSE);
                     lpSS->Row.CurAt = RowAtTemp;
                     lpSS->Col.CurAt = Col;
                     ColSave = lpSS->Col.CurAt;
                     RowSave = lpSS->Row.CurAt;
                     if (!SS_LeaveCell(lpSS, ColAtOld, -1, RowAtOld, -1, 0))
                        {
                        lpBook->wMode = SS_MODE_NONE;
                        return (0);
                        }

#ifdef SS_UTP
                     if (!lpSS->fAllowEditModePermSel)
#endif
                        SS_HighlightCell(lpSS, TRUE);
                     }
                  }
               else
                  lpBook->wMode = SS_MODE_NONE;
               }

            else if (Col < lpSS->Col.HeaderCnt && Row < lpSS->Row.HeaderCnt)
               {
               if (SS_SELBLOCKOPTION(lpSS) & SS_SELBLOCK_ALL)
                  {
				      SS_COORD RowAtTemp;

                  BlockCellUL.Col = 0;
                  BlockCellUL.Row = 0;
						BlockCellLR.Row = -1;
						BlockCellLR.Col = -1;

                  if (lpSS->Col.Frozen)
                     ColAtTemp = lpSS->Col.HeaderCnt;
                  else
                     ColAtTemp = lpSS->Col.UL;

                  if (lpSS->Row.Frozen)
                     RowAtTemp = lpSS->Row.HeaderCnt;
                  else
                     RowAtTemp = lpSS->Row.UL;
   
                  if (lpSS->Col.CurAt != ColAtTemp ||
                      lpSS->Row.CurAt != RowAtTemp)
                     {
                     SS_HighlightCell(lpSS, FALSE);
                     lpSS->Col.CurAt = ColAtTemp;
                     lpSS->Row.CurAt = RowAtTemp;
                     ColSave = lpSS->Col.CurAt;
                     RowSave = lpSS->Row.CurAt;
                     if (!SS_LeaveCell(lpSS, ColAtOld, -1, RowAtOld, -1, 0))
                        {
                        lpBook->wMode = SS_MODE_NONE;
                        return (0);
                        }

#ifdef SS_UTP
                     if (!lpSS->fAllowEditModePermSel)
#endif
                        SS_HighlightCell(lpSS, TRUE);
                     }
                  } 
               else
                  lpBook->wMode = SS_MODE_NONE;
               }
   
            if (lpBook->wMode == SS_MODE_BLOCK)
              Invert = TRUE;
            }

// 97' 2/4 Modified by BOC Gao. for BUG 00804
// when OpMode is not 0 and 1, click the selected row head 
// will cause click event occurs two times.
#ifdef SPREAD_JPN 
         if (ColSave != lpSS->Col.CurAt || RowSave != lpSS->Row.CurAt ||
            ( Col < lpSS->Col.HeaderCnt && Row >= lpSS->Row.HeaderCnt && lpSS->wOpMode != SS_OPMODE_NORMAL &&
			    lpSS->wOpMode != SS_OPMODE_READONLY && lpSS->wOpMode == 5))
  	     {
 		     SetCapture(hWnd);
   		 }
   		 else if(ColSave != lpSS->Col.CurAt || RowSave != lpSS->Row.CurAt ||
		    ( Col < lpSS->Col.HeaderCnt && Row >= lpSS->Row.HeaderCnt && lpSS->wOpMode != SS_OPMODE_NORMAL &&
			    lpSS->wOpMode != SS_OPMODE_READONLY))
	     {
	 	      fDone = TRUE;
	     }
#else
         if (ColSave != lpSS->Col.CurAt || RowSave != lpSS->Row.CurAt)
            fDone = TRUE;
#endif

         else if (SS_ALLOWSELBLOCK(lpSS) && Col != -2 &&
                  Row != -2)
            {
            /*
            if (HIBYTE(GetKeyState(VK_SHIFT)) &&
                SS_IsBlockSelected(lpSS))
                SSx_SelectBlock(hWnd, lpSS, Col, Row);
            else
            */
               {
               if (lpBook->wMode == SS_MODE_BLOCK)
                  {
                  lpSS->BlockCellUL.Row = BlockCellUL.Row;
                  lpSS->BlockCellUL.Col = BlockCellUL.Col;

                  lpSS->BlockCellLR.Row = BlockCellLR.Row;
                  lpSS->BlockCellLR.Col = BlockCellLR.Col;
                  }

               lpSS->Row.BlockCellCurrentPos = Row;
               lpSS->Col.BlockCellCurrentPos = Col;

               if (Invert)
                  {
                  //lpSS->fIsBlockSelected = TRUE;
                  SS_InvertBlock(0, lpSS);
#ifdef SS_V80
					if (!lpSS->lpBook->fPaintingToScreen)
						{
						BOOL fRedraw = lpSS->lpBook->Redraw;
						lpSS->lpBook->Redraw = FALSE;
						SS_InvalidateActiveHeaders(lpSS);
						lpSS->lpBook->Redraw = fRedraw;
						}
#endif // SS_V80
                  }
               }
            }
#ifdef SS_V35
            }
#endif             
         }

      if (!fDone && Msg == WM_LBUTTONDBLCLK && Col >= 0 && Row >= 0)
         {
         if (lpSS->RestrictCols && Col >
             lpSS->Col.DataCnt && lpSS->RestrictRows &&
             Row > lpSS->Row.DataCnt)
            {
            if (!SS_SendMsgCoords(lpSS, SSM_COLROWRESTRICTED, GetDlgCtrlID(hWnd), Col, Row))
               SS_Beep(lpBook);
            }

         else if (lpSS->RestrictCols &&
                  Col > lpSS->Col.DataCnt)
            {
				SS_COORD ColTemp = Col;
            SS_AdjustCellCoordsOut(lpSS, &ColTemp, NULL);
            if (!SS_SendMsg(lpBook, lpSS, SSM_COLRESTRICTED, GetDlgCtrlID(hWnd), ColTemp))
               SS_Beep(lpBook);
            }

         else if (lpSS->RestrictRows &&
                  Row > lpSS->Row.DataCnt)
            {
				SS_COORD RowTemp = Row;
            SS_AdjustCellCoordsOut(lpSS, NULL, &RowTemp);
            if (!SS_SendMsg(lpBook, lpSS, SSM_ROWRESTRICTED, GetDlgCtrlID(hWnd), RowTemp))
               SS_Beep(lpBook);
            }

         else
            {
            if (!fComboBtn)
               {
               HWND hWndFocusOld = GetFocus();

               if (SS_SendMsgCoords(lpSS, SSM_DBLCLK, GetDlgCtrlID(hWnd), Col, Row))
                  return (FALSE);

               if (SS_IsDestroyed(hWnd))
                  return (FALSE);

               if (hWndFocusOld != GetFocus())
                  return (FALSE);

               if (GetSystemMetrics(SM_SWAPBUTTON))
                  {
                  if ((GetAsyncKeyState(VK_RBUTTON) & 0x8000) == 0)
                     lpBook->fNoSetCapture = TRUE;
                  }
               else
                  if ((GetAsyncKeyState(VK_LBUTTON) & 0x8000) == 0)
                     lpBook->fNoSetCapture = TRUE;
               }

            if (Row == lpSS->Row.CurAt && Col == lpSS->Col.CurAt)
#if SS_V80
               SS_CellEditModeOn(lpSS, fComboBtn || fReservedLocation ? MsgOrig : Msg,
                                 0, MAKELONG(MouseX - x, MouseY - y));
#else
               SS_CellEditModeOn(lpSS, fComboBtn ? MsgOrig : Msg,
                                 0, MAKELONG(MouseX - x, MouseY - y));
#endif
				// RFW - 5/30/03
            else if (!lpBook->EditModeOn && !lpSS->HighlightOn)
               SS_HighlightCell(lpSS, TRUE);

            }
         }

      else if (!fDone || Msg == WM_LBUTTONDBLCLK)
         {
         if (Msg == WM_LBUTTONDBLCLK)
            {
            if (SS_SendMsgCoords(lpSS, SSM_DBLCLK, GetDlgCtrlID(hWnd),
                                 Col < lpSS->Col.HeaderCnt ? 0 : Col, Row < lpSS->Row.HeaderCnt ? 0 : Row))
               return (FALSE);

            if (SS_IsDestroyed(hWnd))
               return (FALSE);
            }

         else if (Col != -2 && Row != -2)
            {
            hWndFocus = 0;

            if (lpBook->EditModeOn)
               hWndFocus = GetFocus();

            lpSS->fSetActiveCellCalled = FALSE;

            if (SS_SendMsgCoords(lpSS, SSM_CLICK, GetDlgCtrlID(hWnd), Col, Row))
               return (FALSE);

            if (SS_IsDestroyed(hWnd))
               return (FALSE);

            SS_UpdateWindow(lpBook);

				// RFW - 5/30/07 - 20341
				lpSSTemp = SS_BookLockActiveSheet(lpBook);
				SS_BookUnlockActiveSheet(lpBook);

				if (lpSSTemp != lpSS)
					return (FALSE);

            // This handles the case where the application changes
            // the active cell within the SSM_CLICK message

            if ((Col >= lpSS->Col.HeaderCnt && lpSS->Col.CurAt != Col) ||
                (Row >= lpSS->Row.HeaderCnt && lpSS->Row.CurAt != Row) ||
                lpSS->fSetActiveCellCalled)
               lpBook->wMode = SS_MODE_NONE;
 
            lpSS->fSetActiveCellCalled = FALSE;
   
            if (lpBook->EditModeOn && hWndFocus && GetFocus() == hWnd)
               SetFocus(hWndFocus);
            }

         if (fDone)
            lpBook->fNoSetCapture = TRUE;
         else
            {
				SS_CELLTYPE   CellType;
				SS_COORD	     ColCell;
				SS_COORD      RowCell;

            if (GetSystemMetrics(SM_SWAPBUTTON))
               {
               if ((GetAsyncKeyState(VK_RBUTTON) & 0x8000) == 0)
                  lpBook->fNoSetCapture = TRUE;
               }
            else
               if ((GetAsyncKeyState(VK_LBUTTON) & 0x8000) == 0)
                  lpBook->fNoSetCapture = TRUE;

				SS_GetActualCell(lpSS, Col, Row, &ColCell, &RowCell, NULL);
            if (Col != -2 && Row != -2 && Col >= lpSS->Col.HeaderCnt && Row >= lpSS->Row.HeaderCnt &&
                SS_RetrieveCellType(lpSS, &CellType, NULL, ColCell, RowCell) &&
               (CellType.Type == SS_TYPE_BUTTON 
#if SS_V80 // 27026 -scl
               || (CellType.Type == SS_TYPE_CUSTOM && fReservedLocation)
#endif

               || CellType.Type == SS_TYPE_CHECKBOX) &&
                (lpSS->wOpMode != SS_OPMODE_ROWMODE ||
                 lpSS->fRowModeEditing) &&
                !SS_GetLock(lpSS, ColCell, RowCell, FALSE))
               {
               lpSS->Col.CurAt = ColCell;
               lpSS->Row.CurAt = RowCell;

               lpBook->wMode = SS_MODE_NONE;
		         lpBook->wMsgLast = 0;  // RFW - 6/26/08 - 22575

               SS_CellEditModeOn(lpSS, WM_LBUTTONDOWN, 0,
                                 MAKELONG(MouseX - x, MouseY - y));
               }

            else if (!lpBook->EditModeOn && !lpSS->HighlightOn)
               SS_HighlightCell(lpSS, TRUE);
            }
         }

      if (!IsWindow(hWnd))
         return (0);

      // BJO 17Feb98 GIC3207 - Before fix
      //if (fDone || lpSS->fNoSetCapture)
      //   lpSS->fNoSetCapture = FALSE;
      //
      //else if (SS_USESELBAR(lpSS))
      //   {
      //   lpSS->ModeSelect = TRUE;
      //   SetCapture(hWnd);
      //   }
      // BJO 17Feb98 GIC3207 - Begin fix
      if (lpBook->fNoSetCapture)
         lpBook->fNoSetCapture = FALSE;
	  // fix for GIC7696 -scl
	  // don't capture mouse if it is already captured
      else if (GetCapture() && GetCapture() != hWnd)
      {
  	      lpBook->wMode = SS_MODE_NONE;
   	  }
	  // fix for GIC7696 -scl
      else if (SS_USESELBAR(lpSS))
         {
			lpBook->wMode = SS_MODE_SELECT;
			SetCapture(hWnd);
         }
      else if (fDone)
         lpBook->fNoSetCapture = FALSE;
      // BJO 17Feb98 GIC3207 - End fix

      else if (fNoSetCapture)
         lpBook->wMode = SS_MODE_NONE;

      else if (lpBook->wMode == SS_MODE_BLOCK && !lpBook->fKillFocusReceived)
         SetCapture(hWnd);

      lpSS->Col.PrevAt = lpSS->Col.CurAt;
      lpSS->Row.PrevAt = lpSS->Row.CurAt;
      lpSS->Col.PrevUL = lpSS->Col.UL;
      lpSS->Row.PrevUL = lpSS->Row.UL;
      }

   if (lpSS->fInvertBlock)
      {
      lpSS->fSuspendInvert = FALSE;
      SS_HighlightCell(lpSS, TRUE);
      SS_InvertBlock(0, lpSS);
      lpSS->fInvertBlock = FALSE;
      }
   }

return (0);
}


BOOL SS_IsWindowsV95(void)
{
WORD wLoWordVersion = LOWORD(GetVersion());

return (HIBYTE(wLoWordVersion) >= 95 || LOBYTE(wLoWordVersion) >= 4);
}


//RWP04a
//---------*---------*---------*---------*---------*---------*---------*-----
//
// FUNCTION:  SS_GetAutoSizeVisRowsHeight()
//
// PARAMETERS:  LPSPREADSHEET lpSS - long pointer to the spreadsheet structure
//        SS_COORD RowCnt - number of rows to display
//        LPRECT lpRect - Size of the client rect
// 
// DESCRIPTION: This function calculates the height of <RowCnt> rows (including 
//        the headers and frozen rows).  It DOES NOT count rows with zero
//        height.
//
// RETURNS:   The height of <RowCnt> rows
//
// COMMENTS:  This function replaces the SS_GetCellPosYExt() function call
//        in SS_AutoSize() [in file ss_main.c].  SS_GetCellPosYExt() 
//        does count rows with zero height.
//
//---------*---------*---------*---------*---------*---------*---------*-----
short SS_GetAutoSizeVisRowsHeight(LPSPREADSHEET lpSS, SS_COORD RowCnt, LPRECT lpRect)
{
SS_COORD i;
short    y;
short    nHeight;
SS_COORD Cnt = 0;

y = SS_GetCellPosYExt(lpSS, 0, lpSS->Row.HeaderCnt, lpRect);

for (i = lpSS->Row.HeaderCnt; i < lpSS->Row.HeaderCnt + lpSS->Row.Frozen &&
     i < SS_GetRowCnt(lpSS) && Cnt < RowCnt; i++)
   {
   nHeight = SS_GetRowHeightInPixels(lpSS, i);
   y += nHeight;

   if (nHeight > 0)
      Cnt++;
   }

for (; i < SS_GetRowCnt(lpSS) && Cnt < RowCnt; i++)
   {
   nHeight = SS_GetRowHeightInPixels(lpSS, i);
   y += nHeight;

   if (nHeight > 0)
      Cnt++;

   if (y > lpRect->bottom - lpRect->top)
      break;
   }

return (y);
}


//---------*---------*---------*---------*---------*---------*---------*-----
//
// FUNCTION:  SS_GetAutoSizeVisColsWidth()
//
// PARAMETERS:  LPSPREADSHEET lpSS - long pointer to the spreadsheet structure
//        SS_COORD ColCnt - number of columns to display
//        LPRECT lpRect - Size of the client rect
// 
// DESCRIPTION: This function calculates the width of <ColCnt> columns 
//        (including the headers and frozen columns).  It DOES NOT count
//        columns with zero width.
//
// RETURNS:   The width of <ColCnt> columns
//
// COMMENTS:  This function replaces the SS_GetCellPosXExt() function call
//        in SS_AutoSize() [in file ss_main.c].  SS_GetCellPosXExt() 
//        does count columns with zero width.
//
//---------*---------*---------*---------*---------*---------*---------*-----
short SS_GetAutoSizeVisColsWidth(LPSPREADSHEET lpSS, SS_COORD ColCnt, LPRECT lpRect)
{
SS_COORD i;
short    x;
short    nWidth;
SS_COORD Cnt = 0;

x = SS_GetCellPosXExt(lpSS, 0, lpSS->Col.HeaderCnt, lpRect);

for (i = lpSS->Col.HeaderCnt; i < lpSS->Col.HeaderCnt + lpSS->Col.Frozen && i < SS_GetColCnt(lpSS) && Cnt < ColCnt; i++)
  {
  nWidth = SS_GetColWidthInPixels(lpSS, i);
  x += nWidth;

  if (nWidth > 0)
    Cnt++;
  }

for (; i < SS_GetColCnt(lpSS) && Cnt < ColCnt; i++)
  {
  nWidth = SS_GetColWidthInPixels(lpSS, i);
  x += nWidth;

  if (nWidth > 0)
    Cnt++;

  if (x > lpRect->right - lpRect->left)
     break;
  }

return (x);
}


void SSx_SetColHeaderRows(LPSPREADSHEET lpSS, SS_COORD lHeaderCnt)
{
SS_COORD i;
SS_COORD lHeaderCntOld = lpSS->Row.HeaderCnt;

if (lHeaderCnt < 1 || lHeaderCnt > 255)
	return;

if (lHeaderCntOld < lHeaderCnt)
	{
	SS_CELLTYPE CellType;
	LPSS_ROW    lpRow;
	SS_COORD    lNewRows = lHeaderCnt - max(1, lHeaderCntOld);

	lpSS->Row.HeaderCnt = lHeaderCnt;

	if (lNewRows > 0)
		{
		SS_InsRowRange(lpSS, lHeaderCntOld, lHeaderCntOld + lNewRows - 1);
		// Adjust ActiveCell
		if (lHeaderCntOld <= lpSS->Row.CurAt)
			{
			lpSS->Row.CurAt += lNewRows;
			lpSS->Row.CurAt = min(lpSS->Row.CurAt, SS_GetRowCnt(lpSS) - 1);
			}

		if (SS_IsRowHidden(lpSS, 0))
			{
			SS_COORD Row;

			for (Row = lHeaderCntOld; Row <= lHeaderCntOld + lNewRows - 1; Row++)
				SS_ShowRow(lpSS, Row, FALSE);
			}
		}

	SS_SetTypeStaticText(lpSS, &CellType, SS_TEXT_SHADOW | SS_TEXT_CENTER |
								SS_TEXT_VCENTER | SS_TEXT_WORDWRAP);

	for (i = lHeaderCntOld; i < lHeaderCnt; i++)
		{
		if (lpRow = SS_AllocLockRow(lpSS, i))
			{
			lpRow->bUserResize = (BYTE)SS_RESIZE_OFF;
			SS_UnlockRowItem(lpSS, i);
			}

		SS_SetCellTypeRange(lpSS, SS_ALLCOLS, i, SS_ALLCOLS, i, &CellType);
		}

   // RFW - 7/22/04 - 14844
	//lpSS->Row.CurAt += lHeaderCnt - lHeaderCntOld;
	lpSS->Row.UL += lHeaderCnt - lHeaderCntOld;
	lpSS->Row.DataCnt = max(lpSS->Row.DataCnt, lpSS->Row.HeaderCnt);
	// RFW - 5/25/04 - 14492
	lpSS->Row.LR = SS_GetBottomCell(lpSS, lpSS->Row.UL);
	// RFW - 12/27/04 - 15360
	lpSS->Row.LRAllVis = SS_GetBottomVisCell(lpSS, lpSS->Row.UL);
	}

else if (lHeaderCntOld > lHeaderCnt)
	{
	SS_DelRowRange(lpSS, lHeaderCnt, lHeaderCntOld - 1);

	// Adjust ActiveCell
	if (lHeaderCnt < lpSS->Row.CurAt)
		{
		lpSS->Row.CurAt -= min(lpSS->Row.CurAt, lHeaderCntOld) - lHeaderCnt;
		lpSS->Row.CurAt = max(lpSS->Row.HeaderCnt, lpSS->Row.CurAt);
		}

	lpSS->Row.HeaderCnt = lHeaderCnt;
	}

#ifdef SS_V40
if( SS_IsBlockSelected(lpSS) )
{ // adjust block selections -scl
  if( lpSS->lpBook->fAllowMultipleSelBlocks && lpSS->MultiSelBlock.dItemCnt )
  {
    LPSS_SELBLOCK lpItemList = GlobalLock(lpSS->MultiSelBlock.hItemList);
    short         n, cnt = lpSS->MultiSelBlock.dItemCnt;

    for( n = 0; n < cnt; n++ )
    {
      lpItemList[n].UL.Row += lHeaderCnt - lHeaderCntOld;
      lpItemList[n].LR.Row += lHeaderCnt - lHeaderCntOld;
    }
    GlobalUnlock(lpSS->MultiSelBlock.hItemList);
  }
  else
  {
    lpSS->BlockCellUL.Row += lHeaderCnt - lHeaderCntOld;
    lpSS->BlockCellLR.Row += lHeaderCnt - lHeaderCntOld;
  }
}
#endif
SS_SetVScrollBar(lpSS);
SS_InvalidateRect(lpSS->lpBook, NULL, TRUE);
}


void SSx_SetRowHeaderCols(LPSPREADSHEET lpSS, SS_COORD lHeaderCnt)
{
SS_COORD i;
SS_COORD lHeaderCntOld = lpSS->Col.HeaderCnt;

if (lHeaderCnt < 1 || lHeaderCnt > 255)
	return;

if (lHeaderCntOld < lHeaderCnt)
	{
	SS_CELLTYPE CellType;
	LPSS_COL    lpCol;
	SS_COORD    lNewCols = lHeaderCnt - max(1, lHeaderCntOld);

	lpSS->Col.HeaderCnt = lHeaderCnt;

	if (lNewCols > 0)
		{
		SS_InsColRange(lpSS, lHeaderCntOld, lHeaderCntOld + lNewCols - 1);
		// Adjust ActiveCell
		if (lHeaderCntOld <= lpSS->Col.CurAt)
			{
			lpSS->Col.CurAt += lNewCols;
			lpSS->Col.CurAt = min(lpSS->Col.CurAt, SS_GetColCnt(lpSS) - 1);
			}

		if (SS_IsColHidden(lpSS, 0))
			{
			SS_COORD Col;

			for (Col = lHeaderCntOld; Col <= lHeaderCntOld + lNewCols - 1; Col++)
				SS_ShowCol(lpSS, Col, FALSE);
			}
		}

	SS_SetTypeStaticText(lpSS, &CellType, SS_TEXT_SHADOW | SS_TEXT_CENTER |
								SS_TEXT_VCENTER | SS_TEXT_WORDWRAP);
	for (i = lHeaderCntOld; i < lHeaderCnt; i++)
		{
		if (lpCol = SS_AllocLockCol(lpSS, i))
			{
			/***********************
			* Set Row Header Width
			***********************/

			lpCol->dColWidthX100 = (long)(-2 * 100.0);
			lpCol->dColWidthInPixels = SS_ColWidthToPixels(lpSS, -2);

			lpCol->bUserResize = (BYTE)SS_RESIZE_OFF;
			SS_UnlockColItem(lpSS, i);
			}

		SS_SetCellTypeRange(lpSS, i, SS_ALLROWS, i, SS_ALLROWS, &CellType);
		}

	// RFW - 7/6/04 - 14819
	lpSS->Col.UL += lHeaderCnt - lHeaderCntOld;
	lpSS->Col.DataCnt = max(lpSS->Col.DataCnt, lpSS->Col.HeaderCnt);
	// RFW - 5/25/04 - 14492
	lpSS->Col.LR = SS_GetRightCell(lpSS, lpSS->Col.UL);
	}

else if (lHeaderCntOld > lHeaderCnt)
	{
	SS_DelColRange(lpSS, lHeaderCnt, lHeaderCntOld - 1);

	// Adjust ActiveCell and UpperLeft
	if (lHeaderCnt < lpSS->Col.CurAt)
		{
		lpSS->Col.CurAt -= min(lpSS->Col.CurAt, lHeaderCntOld) - lHeaderCnt;
		lpSS->Col.CurAt = max(lpSS->Col.HeaderCnt, lpSS->Col.CurAt);
		}

	lpSS->Col.HeaderCnt = lHeaderCnt;
	}

#ifdef SS_V40
if( SS_IsBlockSelected(lpSS) )
{ // adjust block selections -scl
  if( lpSS->lpBook->fAllowMultipleSelBlocks && lpSS->MultiSelBlock.dItemCnt )
  {
    LPSS_SELBLOCK lpItemList = GlobalLock(lpSS->MultiSelBlock.hItemList);
    short         n, cnt = lpSS->MultiSelBlock.dItemCnt;

    for( n = 0; n < cnt; n++ )
    {
      lpItemList[n].UL.Col += lHeaderCnt - lHeaderCntOld;
      lpItemList[n].LR.Col += lHeaderCnt - lHeaderCntOld;
    }
    GlobalUnlock(lpSS->MultiSelBlock.hItemList);
  }
  else
  {
    lpSS->BlockCellUL.Col += lHeaderCnt - lHeaderCntOld;
    lpSS->BlockCellLR.Col += lHeaderCnt - lHeaderCntOld;
  }
}
#endif

SS_SetHScrollBar(lpSS);
SS_InvalidateRect(lpSS->lpBook, NULL, TRUE);
}


//RWP04a

#ifdef SS_V35
BOOL SS_TTScrollTip(LPSPREADSHEET lpSS, BOOL fHScroll, SS_COORD lNew)
{
SIZE       sizeRect;
HDC        hdc;
FP_TT_INFO TextTip;
POINT      ptPos;
HWND       hwndTT = fpTextTipGetHwnd(lpSS->lpBook->hWnd);
BOOL       fScrollTip;
HFONT      hFont;
HFONT      hFontOld;
TCHAR      szPrefix[20 + 1];
TCHAR      szTipText[SS_SCROLLTIP_TEXTMAX + 1];
short      nTipWidth = -1;
BOOL       fTextChanged = FALSE;

// RFW - 2/5/07 - 19784
GetCursorPos(&ptPos);

fpTextTipGetInfo(lpSS->lpBook->hWnd, &TextTip);
hdc = GetDC(hwndTT);

lNew -= fHScroll ? lpSS->Col.HeaderCnt - 1 : lpSS->Row.HeaderCnt - 1;

fScrollTip = fpTextTipGetScrollTip(lpSS->lpBook->hWnd);

if (fHScroll && (lpSS->lpBook->nShowScrollTips == SS_SHOWSCROLLTIPS_BOTH ||
                 lpSS->lpBook->nShowScrollTips == SS_SHOWSCROLLTIPS_HORZ))
	{
#ifdef SPREAD_JPN
   lstrcpy(szPrefix, _T(" óÒ: ")); 
#else
   lstrcpy(szPrefix, _T(" Column: ")); 
#endif // SPREAD_JPN
   fpTextTipSetScrollTip(lpSS->lpBook->hWnd, TRUE);
	}
else if (!fHScroll && (lpSS->lpBook->nShowScrollTips == SS_SHOWSCROLLTIPS_BOTH ||
                       lpSS->lpBook->nShowScrollTips == SS_SHOWSCROLLTIPS_VERT))
	{
#ifdef SPREAD_JPN
   lstrcpy(szPrefix, _T(" çs: "));
#else
   lstrcpy(szPrefix, _T(" Row: "));
#endif // SPREAD_JPN
   fpTextTipSetScrollTip(lpSS->lpBook->hWnd, TRUE);
	}
else
	{
   fpTextTipSetScrollTip(lpSS->lpBook->hWnd, FALSE);
   ReleaseDC(hwndTT, hdc);
   return TRUE;
	}

_stprintf(szTipText, _T("%s%ld"), szPrefix, lNew);

if (fScrollTip)
	nTipWidth = lpSS->lpBook->nScrollTipsWidth;

#ifdef SS_V70
{
TCHAR szTipTextOld[SS_SCROLLTIP_TEXTMAX + 1];
lstrcpy(szTipTextOld, szTipText);
SS_SendMsgBeforeScrollTip(lpSS, !fHScroll, lNew, szTipText, &nTipWidth);
fTextChanged = lstrcmp(szTipText, szTipTextOld) != 0;
}
#endif // SS_V70

SendMessage(hwndTT, WM_SETTEXT, 0, (LPARAM)szTipText);

hFont = CreateFontIndirect(&TextTip.LogFont);
if (hFont)
	hFontOld = SelectObject(hdc, hFont);

if (nTipWidth == -1)
	{
	if (!fScrollTip && !fTextChanged)
		{
		TCHAR szTipMax[100];
		TCHAR szMax[20];

		if (fHScroll)
			_ltot(SS_GetColCnt(lpSS) - 1, szMax, 10);
		else
			_ltot(SS_GetRowCnt(lpSS) - 1, szMax, 10);

		_stprintf(szTipMax, _T("%s%s"), szPrefix, szMax);

		GetTextExtentPoint(hdc, szTipMax, lstrlen(szTipMax), &sizeRect);
		nTipWidth = (WORD)sizeRect.cx + 1 + 2;
		}
	else if (fTextChanged)
		{
		GetTextExtentPoint(hdc, szTipText, lstrlen(szTipText), &sizeRect);
		nTipWidth = (WORD)sizeRect.cx + 1 + 2;
		}
	}

if (nTipWidth > 0)
	{
	short      nScreenWidth = GetSystemMetrics(78);
	TEXTMETRIC tm;
	short      nHeight;

	if (nScreenWidth == 0)
		nScreenWidth = GetSystemMetrics(SM_CXSCREEN);
	if (nScreenWidth == 0)
		nScreenWidth = GetDeviceCaps(hdc, HORZRES);

	nTipWidth = min(nTipWidth, nScreenWidth);

	if (fScrollTip)
		ptPos = lpSS->lpBook->ptScrollTipsMouse;
	else
		lpSS->lpBook->ptScrollTipsMouse = ptPos;

	GetTextMetrics(hdc, &tm);
	nHeight = (short)(tm.tmHeight + 2);

	if (fHScroll)
		{
		ptPos.x -= nTipWidth/2;
		ptPos.y -= 35;
		}
	else
		{
		ptPos.x -= 20 + nTipWidth;
		ptPos.y -= nHeight/2;
		}

	ptPos.x = max(5, ptPos.x); // 5 was picked because it looked good...
	ptPos.y = max(5, ptPos.y); // ditto

	nTipWidth = (short)min(nTipWidth, nScreenWidth - ptPos.x);

	SetWindowPos(hwndTT, HWND_TOPMOST, ptPos.x, ptPos.y, nTipWidth,
				    nHeight, SWP_NOACTIVATE);
	lpSS->lpBook->nScrollTipsWidth = nTipWidth;
	}

if (fScrollTip)
	{
	InvalidateRect(hwndTT, NULL, TRUE);
	UpdateWindow(hwndTT);
	}
else
	ShowWindow(hwndTT, SW_SHOWNOACTIVATE);

if (hFont)
	{
	SelectObject(hdc, hFontOld);
	DeleteObject((HGDIOBJ)hFont);
	}  

ReleaseDC(hwndTT, hdc);
return TRUE;
}

short SS_GetCellNoteIndicator(LPSS_BOOK lpBook)
{
  return lpBook->nCellNoteIndicator;
}

void SS_SetCellNoteIndicator(LPSS_BOOK lpBook, short nNewValue)
{
  if (nNewValue >=0 && nNewValue <=3)
    lpBook->nCellNoteIndicator = nNewValue;

   SS_InvalidateRect(lpBook, NULL, TRUE);
}

BOOL SS_IsFetchCellNote(LPSS_BOOK lpBook)
{
  return lpBook->fFetchCellNote;
}
#endif


BOOL SS_GetActualCell(LPSPREADSHEET lpSS, SS_COORD lCol, SS_COORD lRow, LPSS_COORD lpCol, LPSS_COORD lpRow, LPRECT lpRect)
{
SS_CELLRANGE CellRange;
BOOL fIsCellSpan;

fIsCellSpan = SS_GetActualCellEx(lpSS, lCol, lRow, &CellRange, lpRect);

if (lpCol)
	*lpCol = CellRange.UL.Col;
if (lpRow)
	*lpRow = CellRange.UL.Row;

return (fIsCellSpan);
}


BOOL SS_GetActualCellEx(LPSPREADSHEET lpSS, SS_COORD lCol, SS_COORD lRow, LPSS_CELLRANGE lpCellRange, LPRECT lpRect)
{
SS_COORD lColCnt = 1;
SS_COORD lRowCnt = 1;
BOOL fIsCellSpan = FALSE;

#ifdef SS_V40
fIsCellSpan = SS_GetCellSpan(lpSS, lCol, lRow, &lCol, &lRow, &lColCnt, &lRowCnt);
#endif // SS_V40

if (lpRect)
	{
	int cx, cy;
#ifdef SS_V40
	long lSpanNumCols, lSpanNumRows;

	if (SS_GetCellSpan(lpSS, lCol, lRow, NULL, NULL, &lSpanNumCols, &lSpanNumRows))
		SS_GetCellRangeCoord(lpSS, lpSS->Col.UL, lpSS->Row.UL, lCol, lRow,
									lCol + lSpanNumCols - 1, lRow + lSpanNumRows - 1, NULL, &lpRect->left, &lpRect->top, &cx, &cy);
else
#endif // SS_V40
		SS_GetCellCoord(lpSS, lpSS->Col.UL, lpSS->Row.UL, lCol, lRow, &lpRect->left, &lpRect->top, &cx, &cy);

	lpRect->right = lpRect->left + cx;
	lpRect->bottom = lpRect->top + cy;
	}

lpCellRange->UL.Col = lCol;
lpCellRange->UL.Row = lRow;
lpCellRange->LR.Col = lCol + lColCnt - 1;
lpCellRange->LR.Row = lRow + lRowCnt - 1;

return (fIsCellSpan);
}


#ifdef SS_V70
void SS_ColMoving(LPSPREADSHEET lpSS, int xMouse, int yMouse)
{
MSG          Msg;
SS_CELLCOORD BlockCellLR;
SS_CELLCOORD NewPos;
RECT         RectClient;
RECT         RectCell;
POINT        PointClient;
POINT        Point;
int          iSwapButton;

SS_GetClientRect(lpSS->lpBook, &RectClient);

PointClient.x = 0;
PointClient.y = 0;

ClientToScreen(lpSS->lpBook->hWnd, &PointClient);

iSwapButton = GetSystemMetrics(SM_SWAPBUTTON);

if (lpSS->lpBook->wMode == SS_MODE_BEGINCOLMOVE)
	{
	lpSS->lpBook->wMode = SS_MODE_COLMOVE;
	SS_HoverBegin(lpSS, lpSS->MoveStartRange.UL.Col, lpSS->MoveStartRange.UL.Row);
	}
else
	SS_HoverMove(lpSS);

while (lpSS->lpBook->wMode != SS_MODE_NONE)
   {
   if (iSwapButton)
      {
      if ((GetAsyncKeyState(VK_RBUTTON) & 0x8000) == 0)
         break;
      }
   else
      if ((GetAsyncKeyState(VK_LBUTTON) & 0x8000) == 0)
         break;

   GetCursorPos(&Point);

   Point.x -= PointClient.x;
   Point.y -= PointClient.y;

   BlockCellLR.Col = lpSS->ColMoveCurrentCol;
   BlockCellLR.Row = -1;
   if (SS_ScrollWithMouse(lpSS->lpBook->hWnd, lpSS, &BlockCellLR, &BlockCellLR,
                          &NewPos, Point.x, Point.y))
		{
		SS_CELLRANGE CellRange;

		SS_GetActualCellEx(lpSS, NewPos.Col, NewPos.Row, &CellRange, &RectCell);
		NewPos.Col = CellRange.UL.Col;
		NewPos.Row = CellRange.UL.Row;

		if (Point.x > RectCell.left + ((RectCell.right - RectCell.left) / 2))
			NewPos.Col = CellRange.LR.Col + 1;

// RFW-12/15/03-12991      if (NewPos.Col != lpSS->ColMoveCurrentCol)
         {
			lpSS->ColMoveCurrentCol = NewPos.Col;
			SS_HoverArrowColSetPos(lpSS);
         }
		}

   if (PeekMessage(&Msg, NULL, 0, 0, PM_NOREMOVE))
      if (Msg.message == WM_MOUSEMOVE)
         break;
      else if (PeekMessage(&Msg, NULL, 0, 0, PM_REMOVE))
         {
         TranslateMessage(&Msg);
         DispatchMessage(&Msg);
         }
   }
}


void SS_RowMoving(LPSPREADSHEET lpSS, int xMouse, int yMouse)
{
MSG          Msg;
SS_CELLCOORD BlockCellLR;
SS_CELLCOORD NewPos;
RECT         RectClient;
RECT         RectCell;
POINT        PointClient;
POINT        Point;
int          iSwapButton;

SS_GetClientRect(lpSS->lpBook, &RectClient);

PointClient.x = 0;
PointClient.y = 0;

ClientToScreen(lpSS->lpBook->hWnd, &PointClient);

iSwapButton = GetSystemMetrics(SM_SWAPBUTTON);

if (lpSS->lpBook->wMode == SS_MODE_BEGINROWMOVE)
	{
	lpSS->lpBook->wMode = SS_MODE_ROWMOVE;
	SS_HoverBegin(lpSS, lpSS->MoveStartRange.UL.Col, lpSS->MoveStartRange.UL.Row);
	}
else
	SS_HoverMove(lpSS);

while (lpSS->lpBook->wMode != SS_MODE_NONE)
   {
   if (iSwapButton)
      {
      if ((GetAsyncKeyState(VK_RBUTTON) & 0x8000) == 0)
         break;
      }
   else
      if ((GetAsyncKeyState(VK_LBUTTON) & 0x8000) == 0)
         break;

   GetCursorPos(&Point);

   Point.x -= PointClient.x;
   Point.y -= PointClient.y;

   BlockCellLR.Row = lpSS->RowMoveCurrentRow;
   BlockCellLR.Col = -1;
   if (SS_ScrollWithMouse(lpSS->lpBook->hWnd, lpSS, &BlockCellLR, &BlockCellLR,
                          &NewPos, Point.x, Point.y))
		{
		SS_CELLRANGE CellRange;

		SS_GetActualCellEx(lpSS, NewPos.Col, NewPos.Row, &CellRange, &RectCell);
		NewPos.Col = CellRange.UL.Col;
		NewPos.Row = CellRange.UL.Row;

		if (Point.y > RectCell.top + ((RectCell.bottom - RectCell.top) / 2))
			NewPos.Row = CellRange.LR.Row + 1;

// RFW-12/15/03-12991      if (NewPos.Row != lpSS->RowMoveCurrentRow)
         {
			lpSS->RowMoveCurrentRow = NewPos.Row;
			SS_HoverArrowRowSetPos(lpSS);
         }
		}

   if (PeekMessage(&Msg, NULL, 0, 0, PM_NOREMOVE))
      if (Msg.message == WM_MOUSEMOVE)
         break;
      else if (PeekMessage(&Msg, NULL, 0, 0, PM_REMOVE))
         {
         TranslateMessage(&Msg);
         DispatchMessage(&Msg);
         }
   }
}


BOOL SS_IsMouseInColMove(LPSPREADSHEET lpSS, int MouseX, int MouseY)
{
SS_COORD Col, Row;
int x, y;

SS_GetCellFromPixel(lpSS, &Col, &Row, &x, &y, MouseX, MouseY);
return SS_IsColMove(lpSS, Col, Row);
}


BOOL SS_IsColMove(LPSPREADSHEET lpSS, SS_COORD Col, SS_COORD Row)
{
if (Col >= lpSS->Col.HeaderCnt && Row >= 0 && 
    Row < lpSS->Row.HeaderCnt && lpSS->fAllowColMove)
	{
	if (SS_IsSpanOverlap(lpSS, Col, -1, 1, -1) != SS_SPANOVERLAP_YES)
		return (TRUE);
	}

return (FALSE);
}


BOOL SS_IsMouseInRowMove(LPSPREADSHEET lpSS, int MouseX, int MouseY)
{
SS_COORD Col, Row;
int x, y;

SS_GetCellFromPixel(lpSS, &Col, &Row, &x, &y, MouseX, MouseY);
return SS_IsRowMove(lpSS, Col, Row);
}


BOOL SS_IsRowMove(LPSPREADSHEET lpSS, SS_COORD Col, SS_COORD Row)
{
if (Row >= lpSS->Row.HeaderCnt && Col >= 0 && 
    Col < lpSS->Col.HeaderCnt && lpSS->fAllowRowMove)
	{
	if (SS_IsSpanOverlap(lpSS, -1, Row, -1, 1) != SS_SPANOVERLAP_YES)
		return (TRUE);
	}

return (FALSE);
}


WINENTRY SS_HoverWndFn(HWND hWnd, UINT Msg, WPARAM wParam, LPARAM lParam);
void HoverPaint(HWND hWnd, HDC hDC, LPRECT lpRectUpdate);
BOOL RegisterHoverArrow(HANDLE hInstance);
BOOL SS_UnRegisterHoverArrow(HANDLE hInstance);
WINENTRY SS_HoverArrowWndFn(HWND hWnd, UINT Msg, WPARAM wParam, LPARAM lParam);
void HoverArrowPaint(HWND hWnd, HDC hDC, LPRECT lpRectUpdate);
HRGN CreateTopArrow(LPRECT lpRectBound);
HRGN CreateBottomArrow(LPRECT lpRectBound);
HRGN CreateLeftArrow(LPRECT lpRectBound);
HRGN CreateRightArrow(LPRECT lpRectBound);

#define SS_HOVER_TB_WIDTH  9
#define SS_HOVER_TB_HEIGHT 9
#define SS_HOVER_LR_WIDTH  9
#define SS_HOVER_LR_HEIGHT 9

BOOL RegisterHover(HANDLE hInstance)
{
WNDCLASS wc;
BOOL     bRet = TRUE;

//wc.style         = CS_HREDRAW | CS_VREDRAW | CS_SAVEBITS;
wc.style         = CS_HREDRAW | CS_VREDRAW;
wc.lpfnWndProc   = SS_HoverWndFn;
wc.cbClsExtra    = 0;
#if defined(_WIN64) || defined(_IA64)
wc.cbWndExtra    = 8;
#else
wc.cbWndExtra    = 4;
#endif
wc.hInstance     = hInstance;
wc.hIcon         = NULL;
wc.hCursor       = LoadCursor(NULL, IDC_ARROW);
wc.hbrBackground = (HBRUSH)(COLOR_BTNFACE + 1);
wc.lpszMenuName  = NULL;
wc.lpszClassName = _T("SS_Hover");
if (!RegisterClass(&wc))
   bRet = FALSE;

return bRet;
}


BOOL SS_UnRegisterHover(HANDLE hInstance)
{
return (UnregisterClass(_T("SS_Hover"), hInstance));
}


typedef struct ss_hover
   {
	HWND     hWndSS;
   SS_COORD Col;
   SS_COORD Row;
	int      xMouseOffset;
	int      yMouseOffset;
   } SS_HOVER, FAR *LPSS_HOVER;


void SS_HoverBegin(LPSPREADSHEET lpSS, SS_COORD Col, SS_COORD Row)
{
SS_HOVER Hover;
POINT pt;
POINT ptMouse;
RECT Rect;
RECT RectArrow;
HRGN hRgn;

Hover.hWndSS = lpSS->lpBook->hWnd;
Hover.Col = Col;
Hover.Row = Row;

// Determine location of popup

SS_GetActualCell(lpSS, Col, Row, &Col, &Row, &Rect);

pt.x = 0;
pt.y = 0;

ClientToScreen(lpSS->lpBook->hWnd, &pt);
OffsetRect(&Rect, pt.x, pt.y);

// Determine the offset of the mouse from the UL corner of the popup
GetCursorPos(&ptMouse);
//Hover.xMouseOffset = ptMouse.x - Rect.left;
//Hover.yMouseOffset = ptMouse.y - Rect.top;
Hover.xMouseOffset = (Rect.right - Rect.left) / 2;
Hover.yMouseOffset = Rect.bottom - Rect.top - 5;

RegisterHover(hDynamicInst);
lpSS->hWndHover = CreateWindowEx(WS_EX_TOPMOST, _T("SS_Hover"), _T(""), WS_POPUP | WS_BORDER,
                                 Rect.left, Rect.top, Rect.right - Rect.left,
                                 Rect.bottom - Rect.top, lpSS->lpBook->hWnd, NULL,
                                 hDynamicInst, &Hover);

SS_HoverMove(lpSS);
if (lpSS->hWndHover)
	ShowWindow(lpSS->hWndHover, SW_SHOWNOACTIVATE);

RegisterHoverArrow(hDynamicInst);

if (lpSS->lpBook->wMode == SS_MODE_COLMOVE)
	{
	SetRect(&RectArrow, 0, 0, SS_HOVER_TB_WIDTH, SS_HOVER_TB_HEIGHT);
	lpSS->hWndHoverArrowA = CreateWindow(_T("SS_HoverArrow"), _T(""),
								   WS_POPUP, 0, 0, SS_HOVER_TB_WIDTH, SS_HOVER_TB_HEIGHT,
								   lpSS->lpBook->hWnd, NULL, hDynamicInst, (LPVOID)0);
	hRgn = CreateTopArrow(&RectArrow);
	SetWindowRgn(lpSS->hWndHoverArrowA, hRgn, FALSE);
	DeleteObject(hRgn);

	lpSS->hWndHoverArrowB = CreateWindow(_T("SS_HoverArrow"), _T(""),
								   WS_POPUP, 0, 0, SS_HOVER_TB_WIDTH, SS_HOVER_TB_HEIGHT,
								   lpSS->lpBook->hWnd, NULL, hDynamicInst, (LPVOID)1);
	hRgn = CreateBottomArrow(&RectArrow);
	SetWindowRgn(lpSS->hWndHoverArrowB, hRgn, FALSE);
	DeleteObject(hRgn);

	SS_HoverArrowColSetPos(lpSS);
	}
else
	{
	SetRect(&RectArrow, 0, 0, SS_HOVER_LR_WIDTH, SS_HOVER_LR_HEIGHT);
	lpSS->hWndHoverArrowA = CreateWindow(_T("SS_HoverArrow"), _T(""),
								   WS_POPUP, 0, 0, SS_HOVER_LR_WIDTH, SS_HOVER_LR_HEIGHT,
								   lpSS->lpBook->hWnd, NULL, hDynamicInst, (LPVOID)0);
	hRgn = CreateLeftArrow(&RectArrow);
	SetWindowRgn(lpSS->hWndHoverArrowA, hRgn, FALSE);
	DeleteObject(hRgn);

	lpSS->hWndHoverArrowB = CreateWindow(_T("SS_HoverArrow"), _T(""),
								   WS_POPUP, 0, 0, SS_HOVER_LR_WIDTH, SS_HOVER_LR_HEIGHT,
								   lpSS->lpBook->hWnd, NULL, hDynamicInst, (LPVOID)1);
	hRgn = CreateRightArrow(&RectArrow);
	SetWindowRgn(lpSS->hWndHoverArrowB, hRgn, FALSE);
	DeleteObject(hRgn);

	SS_HoverArrowRowSetPos(lpSS);
	}
}


void SS_HoverMove(LPSPREADSHEET lpSS)
{
if (lpSS->hWndHover)
	{
#if defined(_WIN64) || defined(_IA64)
  HGLOBAL hHover = (HGLOBAL)GetWindowLongPtr(lpSS->hWndHover, 0);
#else
  HGLOBAL hHover = (HGLOBAL)GetWindowLong(lpSS->hWndHover, 0);
#endif
	LPSS_HOVER lpHover;
	POINT ptMouse;

	lpHover = (LPSS_HOVER)GlobalLock(hHover);

	GetCursorPos(&ptMouse);
	SetWindowPos(lpSS->hWndHover, NULL, ptMouse.x - lpHover->xMouseOffset,
					 ptMouse.y - lpHover->yMouseOffset, 0, 0, SWP_NOACTIVATE | SWP_NOSIZE |
					 SWP_NOZORDER);

	GlobalUnlock(hHover);
	}
}


void SS_HoverEnd(LPSPREADSHEET lpSS)
{
if (lpSS->hWndHover)
	{
	ShowWindow(lpSS->hWndHover, SW_HIDE);
	DestroyWindow(lpSS->hWndHover);
	lpSS->hWndHover = NULL;
	SS_UnRegisterHover(hDynamicInst);
	}

if (lpSS->hWndHoverArrowA || lpSS->hWndHoverArrowB)
	{
	if (lpSS->hWndHoverArrowA)
		{
		ShowWindow(lpSS->hWndHoverArrowA, SW_HIDE);
		DestroyWindow(lpSS->hWndHoverArrowA);
		lpSS->hWndHoverArrowA = NULL;
		}

	if (lpSS->hWndHoverArrowB)
		{
		ShowWindow(lpSS->hWndHoverArrowB, SW_HIDE);
		DestroyWindow(lpSS->hWndHoverArrowB);
		lpSS->hWndHoverArrowB = NULL;
		}

	SS_UnRegisterHoverArrow(hDynamicInst);
	}
}


WINENTRY SS_HoverWndFn(HWND hWnd, UINT Msg, WPARAM wParam, LPARAM lParam)
{
LPCREATESTRUCT lpcs;
HGLOBAL        hHover;
LPSS_HOVER     lpHover;
PAINTSTRUCT    Paint;
HDC            hDC;
HDC            hDCMemory;
HBITMAP        hBitmapOld;
HBITMAP        hBitmapDC;
RECT           Rect;
static int     iCnt = 0;

switch (Msg)
   {
   case WM_CREATE:
		lpcs = (LPCREATESTRUCT)lParam;
		hHover = GlobalAlloc(GHND, sizeof(SS_HOVER));
		lpHover = (LPSS_HOVER)GlobalLock(hHover);
		_fmemcpy(lpHover, lpcs->lpCreateParams, sizeof(SS_HOVER));
		GlobalUnlock(hHover);
#if defined(_WIN64) || defined(_IA64)
    SetWindowLongPtr(hWnd, 0, (LONG_PTR)hHover);
#else
    SetWindowLong(hWnd, 0, (long)hHover);
#endif
      return (0);

   case WM_ERASEBKGND:
      return (0);

   case WM_PAINT:
      hDC = fpBeginPaint(hWnd, &Paint);
      GetClientRect(hWnd, &Rect);

      /*********************************
      * Create a memory device context
      *********************************/

      hBitmapDC = CreateCompatibleBitmap(hDC, Rect.right - Rect.left,
                                         Rect.bottom - Rect.top);

      hDCMemory = CreateCompatibleDC(hDC);
      hBitmapOld = SelectObject(hDCMemory, hBitmapDC);

      HoverPaint(hWnd, hDCMemory, &Paint.rcPaint);

      /*******************************************************
      * Copy the memory device context bitmap to the display
      *******************************************************/

      BitBlt(hDC, Paint.rcPaint.left, Paint.rcPaint.top,
             Paint.rcPaint.right - Paint.rcPaint.left,
             Paint.rcPaint.bottom - Paint.rcPaint.top, hDCMemory,
             Paint.rcPaint.left, Paint.rcPaint.top, SRCCOPY);

      /***********
      * Clean up
      ***********/

      SelectObject(hDCMemory, hBitmapOld);
      DeleteDC(hDCMemory);
      DeleteObject(hBitmapDC);

      EndPaint(hWnd, &Paint);
      return (0);
   }

return (DefWindowProc(hWnd, Msg, wParam, lParam));
}


void HoverPaint(HWND hWnd, HDC hDC, LPRECT lpRectUpdate)
{
LPSPREADSHEET lpSS;
#if defined(_WIN64) || defined(_IA64)
HGLOBAL hHover = (HGLOBAL)GetWindowLongPtr(hWnd, 0);
#else
HGLOBAL hHover = (HGLOBAL)GetWindowLong(hWnd, 0);
#endif
LPSS_HOVER lpHover;
LPSS_COL lpCol;
LPSS_ROW lpRow;
RECT Rect;

GetClientRect(hWnd, &Rect);

lpHover = (LPSS_HOVER)GlobalLock(hHover);
lpSS = SS_SheetLock(lpHover->hWndSS);

lpRow = SS_LockRowItem(lpSS, lpHover->Row);
lpCol = SS_LockColItem(lpSS, lpHover->Col);

SS_DrawText(hDC, lpSS->lpBook->hWnd, lpSS, lpCol, lpRow, &Rect, lpHover->Col,
            lpHover->Row, TRUE, NULL, lpRectUpdate, NULL, NULL, FALSE, FALSE,
            lpHover->Col, lpHover->Row, FALSE, FALSE);
SS_SpanFreePaintList(lpSS);

SS_UnlockRowItem(lpSS, lpHover->Row);
SS_UnlockColItem(lpSS, lpHover->Col);

SS_SheetUnlock(lpHover->hWndSS);
GlobalUnlock(hHover);
}


BOOL RegisterHoverArrow(HANDLE hInstance)
{
WNDCLASS wc;
BOOL     bRet = TRUE;

wc.style         = CS_HREDRAW | CS_VREDRAW | CS_SAVEBITS;
wc.lpfnWndProc   = SS_HoverArrowWndFn;
wc.cbClsExtra    = 0;
wc.cbWndExtra    = 2;
wc.hInstance     = hInstance;
wc.hIcon         = NULL;
wc.hCursor       = LoadCursor(NULL, IDC_ARROW);
wc.hbrBackground = (HBRUSH)(COLOR_BTNFACE + 1);
wc.lpszMenuName  = NULL;
wc.lpszClassName = _T("SS_HoverArrow");
if (!RegisterClass(&wc))
   bRet = FALSE;

return bRet;
}


BOOL SS_UnRegisterHoverArrow(HANDLE hInstance)
{
return (UnregisterClass(_T("SS_HoverArrow"), hInstance));
}


WINENTRY SS_HoverArrowWndFn(HWND hWnd, UINT Msg, WPARAM wParam, LPARAM lParam)
{
LPCREATESTRUCT lpcs;
PAINTSTRUCT    Paint;
WORD           wArrow;
HDC            hDC;
RECT           Rect;

switch (Msg)
   {
   case WM_CREATE:
		lpcs = (LPCREATESTRUCT)lParam;
		wArrow = (WORD)lpcs->lpCreateParams;
      SetWindowWord(hWnd, 0, wArrow);
      return (0);

   case WM_ERASEBKGND:
      return (0);

   case WM_PAINT:
      hDC = fpBeginPaint(hWnd, &Paint);
      GetClientRect(hWnd, &Rect);

      HoverArrowPaint(hWnd, hDC, &Paint.rcPaint);

      EndPaint(hWnd, &Paint);
      return (0);
   }

return (DefWindowProc(hWnd, Msg, wParam, lParam));
}


void HoverArrowPaint(HWND hWnd, HDC hDC, LPRECT lpRectUpdate)
{
WORD wArrow = GetWindowWord(hWnd, 0);
HBRUSH hBrush;

hBrush = CreateSolidBrush(RGB(255, 0, 0));
FillRect(hDC, lpRectUpdate, hBrush);
DeleteObject(hBrush);
}


void SS_HoverArrowColSetPos(LPSPREADSHEET lpSS)
{
BOOL fShowA = FALSE;
BOOL fShowB = FALSE;

if (lpSS->ColMoveCurrentCol < lpSS->MoveStartRange.UL.Col ||
    lpSS->ColMoveCurrentCol > lpSS->MoveStartRange.UL.Col + 1)
	{
	SS_COORD Col = lpSS->ColMoveCurrentCol;
	SS_COORD Row = lpSS->MoveStartRange.UL.Row;
	POINT pt;
	RECT Rect, RectTop, RectBottom, RectTemp;
	RECT RectClient;

	SS_GetClientRect(lpSS->lpBook, &RectClient);

	SS_GetActualCell(lpSS, Col, Row, &Col, &Row, &Rect);

	pt.x = 0;
	pt.y = 0;

	ClientToScreen(lpSS->lpBook->hWnd, &pt);
	OffsetRect(&Rect, pt.x, pt.y);

	CopyRect(&RectTop, &Rect);
	RectTop.left -= SS_HOVER_TB_WIDTH / 2;
	RectTop.top -= SS_HOVER_TB_HEIGHT;
	RectTop.right = RectTop.left + SS_HOVER_TB_WIDTH;
	RectTop.bottom = RectTop.top + SS_HOVER_TB_HEIGHT;
	CopyRect(&RectBottom, &Rect);
	RectBottom.left -= SS_HOVER_TB_WIDTH / 2;
	RectBottom.top = Rect.bottom;
	RectBottom.right = RectBottom.left + SS_HOVER_TB_WIDTH;
	RectBottom.bottom = RectBottom.top + SS_HOVER_TB_HEIGHT;
	SetWindowPos(lpSS->hWndHoverArrowA, NULL, RectTop.left, RectTop.top, 0, 0,
                SWP_NOACTIVATE | SWP_NOSIZE | SWP_NOZORDER);
	SetWindowPos(lpSS->hWndHoverArrowB, NULL, RectBottom.left, RectBottom.top, 0, 0,
                SWP_NOACTIVATE | SWP_NOSIZE | SWP_NOZORDER);

	OffsetRect(&RectClient, pt.x, pt.y);
	RectTop.bottom += 2;
	RectBottom.top -= 2;

	if (IntersectRect(&RectTemp, &RectClient, &RectTop))
		fShowA = TRUE;

	if (IntersectRect(&RectTemp, &RectClient, &RectBottom))
		fShowB = TRUE;
	}

if (IsWindowVisible(lpSS->hWndHoverArrowA) != fShowA)
	ShowWindow(lpSS->hWndHoverArrowA, fShowA ? SW_SHOWNOACTIVATE : SW_HIDE);

if (IsWindowVisible(lpSS->hWndHoverArrowB) != fShowB)
	ShowWindow(lpSS->hWndHoverArrowB, fShowB ? SW_SHOWNOACTIVATE : SW_HIDE);
}


void SS_HoverArrowRowSetPos(LPSPREADSHEET lpSS)
{
BOOL fShowA = FALSE;
BOOL fShowB = FALSE;

if (lpSS->RowMoveCurrentRow < lpSS->MoveStartRange.UL.Row ||
    lpSS->RowMoveCurrentRow > lpSS->MoveStartRange.UL.Row + 1)
	{
	SS_COORD Col = lpSS->MoveStartRange.UL.Col;
	SS_COORD Row = lpSS->RowMoveCurrentRow;
	POINT pt;
	RECT Rect, RectLeft, RectRight, RectTemp;
	RECT RectClient;

	SS_GetClientRect(lpSS->lpBook, &RectClient);

	SS_GetActualCell(lpSS, Col, Row, &Col, &Row, &Rect);

	pt.x = 0;
	pt.y = 0;

	ClientToScreen(lpSS->lpBook->hWnd, &pt);
	OffsetRect(&Rect, pt.x, pt.y);

	CopyRect(&RectLeft, &Rect);
	RectLeft.top -= SS_HOVER_TB_HEIGHT / 2;
	RectLeft.left -= SS_HOVER_TB_WIDTH;
	RectLeft.bottom = RectLeft.top + SS_HOVER_TB_HEIGHT;
	RectLeft.right = RectLeft.left + SS_HOVER_TB_WIDTH;
	CopyRect(&RectRight, &Rect);
	RectRight.top -= SS_HOVER_TB_HEIGHT / 2;
	RectRight.left = Rect.right;
	RectRight.bottom = RectRight.top + SS_HOVER_TB_HEIGHT;
	RectRight.right = RectRight.left + SS_HOVER_TB_WIDTH;
	SetWindowPos(lpSS->hWndHoverArrowA, NULL, RectLeft.left, RectLeft.top, 0, 0,
                SWP_NOACTIVATE | SWP_NOSIZE | SWP_NOZORDER);
	SetWindowPos(lpSS->hWndHoverArrowB, NULL, RectRight.left, RectRight.top, 0, 0,
                SWP_NOACTIVATE | SWP_NOSIZE | SWP_NOZORDER);

	OffsetRect(&RectClient, pt.x, pt.y);
	RectLeft.right += 2;
	RectRight.left -= 2;

	if (IntersectRect(&RectTemp, &RectClient, &RectLeft))
		fShowA = TRUE;

	if (IntersectRect(&RectTemp, &RectClient, &RectRight))
		fShowB = TRUE;
	}

if (IsWindowVisible(lpSS->hWndHoverArrowA) != fShowA)
	ShowWindow(lpSS->hWndHoverArrowA, fShowA ? SW_SHOWNOACTIVATE : SW_HIDE);

if (IsWindowVisible(lpSS->hWndHoverArrowB) != fShowB)
	ShowWindow(lpSS->hWndHoverArrowB, fShowB ? SW_SHOWNOACTIVATE : SW_HIDE);
}


HRGN CreateArrow(LPRECT lpRects, int iRectCnt, LPRECT lpRectBound)
{
LPRGNDATA lpRgnData;
HGLOBAL hRgnData;
HRGN hRgn;
int iSize;

iSize = sizeof(RGNDATA) + (iRectCnt * sizeof(RECT)) - 1;

hRgnData = GlobalAlloc(GHND, iSize);
lpRgnData = (LPRGNDATA)GlobalLock(hRgnData);

lpRgnData->rdh.dwSize = sizeof(RGNDATAHEADER);
lpRgnData->rdh.iType = RDH_RECTANGLES;
lpRgnData->rdh.nCount = iRectCnt;
lpRgnData->rdh.nRgnSize = 0; // iRectCnt * sizeof(RECT);
lpRgnData->rdh.rcBound = *lpRectBound;
_fmemcpy(lpRgnData->Buffer, lpRects, iRectCnt * sizeof(RECT));

hRgn = ExtCreateRegion(NULL, iSize, lpRgnData);

GlobalUnlock(hRgnData);
GlobalFree(hRgnData);

return (hRgn);
}


HRGN CreateTopArrow(LPRECT lpRectBound)
{
RECT Rect[] = 
	{
	{3,0,6,4},
	{0,4,9,5},
	{1,5,8,6},
	{2,6,7,7},
	{3,7,6,8},
	{4,8,5,9}
	};

return (CreateArrow(Rect, sizeof(Rect) / sizeof(RECT), lpRectBound));
}


HRGN CreateBottomArrow(LPRECT lpRectBound)
{
RECT Rect[] = 
	{
	{4,0,5,1},
	{3,1,6,2},
	{2,2,7,3},
	{1,3,8,4},
	{0,4,9,5},
	{3,5,6,9}
	};

return (CreateArrow(Rect, sizeof(Rect) / sizeof(RECT), lpRectBound));
}


HRGN CreateLeftArrow(LPRECT lpRectBound)
{
RECT Rect[] = 
	{
	{0,3,5,6},
	{4,0,5,9},
	{5,1,6,8},
	{6,2,7,7},
	{7,3,8,6},
	{8,4,9,5}
	};

return (CreateArrow(Rect, sizeof(Rect) / sizeof(RECT), lpRectBound));
}

HRGN CreateRightArrow(LPRECT lpRectBound)
{
RECT Rect[] = 
	{
	{0,4,1,5},
	{1,3,2,6},
	{2,2,3,7},
	{3,1,4,8},
	{4,0,5,9},
	{5,3,9,6}
	};

return (CreateArrow(Rect, sizeof(Rect) / sizeof(RECT), lpRectBound));
}
#endif // SS_V70
