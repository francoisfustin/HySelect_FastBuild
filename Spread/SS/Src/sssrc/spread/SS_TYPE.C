/*********************************************************
* SS_TYPE.C            
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
//    RWP01 10.30.96  <SCS4002> Problem adding new string to combobox
//              list.  Was a result of calling 
//              GetTypeComboBoxCount().
//              
//---------*---------*---------*---------*---------*---------*---------*-----

#include <windows.h>
#include <windowsx.h>
#include <tchar.h>
#include <stdio.h>
#include <stddef.h>
#include <stdlib.h>
#include <string.h>
#include <ctype.h>
#include <process.h>
#include <math.h>
#include <limits.h>
#include <locale.h>
#include "spread.h"
#include "ss_alloc.h"
#include "ss_cell.h"
#include "ss_col.h"
#include "ss_data.h"
#include "ss_emode.h"
#include "ss_main.h"
#include "ss_row.h"
#include "ss_type.h"
#include "ss_user.h"
#include "ss_calc.h"
#include "..\classes\checkbox.h"
#include "..\edit\editpic\editpic.h"
#include "..\edit\editdate\editdate.h"
// fix for bug #8154 -scl
#ifdef SS_V30
#include "fplist.h"
#endif

#ifdef SS_V80
#include "ss_draw.h"
extern WINENTRY SS_CustomProc(HWND hWnd, UINT Msg, WPARAM wParam, LPARAM lParam); // in ss_edit.c
#endif

#ifdef SS_USEAWARE
#include "utools.h"
#endif

#define DOUBLE_INFINITY_VALUE       ((ULONGLONG)0x7ff0000000000000)

#ifdef SPREAD_JPN
// 96' 6/24 Added by BOC Gao. 
// - For load resource before Date window is created.
// - So that if user only set CellType , Format and 
// - Value (Text) property can work correctly. 

#include "..\..\vbx\stringrc.h"

TCHAR   szYearGlobal[4];
TCHAR   szMonthGlobal[4];
TCHAR   szDayGlobal[4];
extern  TCHAR NengoName[12][8];

TCHAR   szHourGlobal[3];
TCHAR   szMinuteGlobal[3];
TCHAR   szSecondGlobal[3];
TCHAR   szAmGlobal[5];
TCHAR   szPmGlobal[5];
// ----------------------------------------------<<
#endif //SPREAD_JPN

extern HANDLE hDynamicInst;

static void           SS_TimeGetDefault(LPTIMEFORMAT lpTimeFormat);
static LPSS_CELLTYPE  SSx_SetCellTypeStruct(LPSPREADSHEET lpSS,
                                       LPSS_CELLTYPE CellType,
                                       LPSS_COL lpCol, LPSS_ROW lpRow,
                                       LPSS_CELL Cell, SS_COORD CellCol,
                                       SS_COORD CellRow);
static TBGLOBALHANDLE SS_tbCreateTextHandle(LPCTSTR lpszText, short Len);
#if SS_V80
static BOOL           SS_TypeControlAlloc(LPTBGLOBALHANDLE lphControls,
                                          LPSHORT lpControlsAllocCnt,
                                          LPSHORT lpControlsCnt, HWND hWndCtrl,
                                          short Type, short CtrlID,
                                          long lStyle, CT_HANDLE hCT, BOOL fRenderer, long ldisp, BOOL fDestroy);
#else
static BOOL           SS_TypeControlAlloc(LPTBGLOBALHANDLE lphControls,
                                          LPSHORT lpControlsAllocCnt,
                                          LPSHORT lpControlsCnt, HWND hWndCtrl,
                                          short Type, short CtrlID,
                                          long lStyle);
#endif
static BOOL           SS_FormatDataPic(LPSS_CELLTYPE lpCellType,
                                       LPCTSTR lpszText, LPTSTR lpszTextFmt,
                                       BOOL fValue);
static BOOL           SS_FormatDataFloat(LPSPREADSHEET lpSS,
                                         LPSS_CELLTYPE lpCellType, LPCTSTR lpszText,
                                         LPTSTR lpszTextFmt, LPDOUBLE lpdfValue,
                                         BOOL fValue, BOOL fCheckRange);
static BOOL           SS_FormatDataCheckBox(LPSS_CELLTYPE lpCellType, LPCTSTR lpszText,
                                            LPTSTR lpszTextFmt);
static BOOL           SS_FormatDataButton(LPSS_CELLTYPE lpCellType, LPCTSTR lpszText,
                                          LPTSTR lpszTextFmt);
static BOOL           SS_FloatStringStrip(LPSPREADSHEET lpSS,
                                          LPSS_CELLTYPE lpCellType, LPTSTR lpszText);
#ifdef SS_V40
static void SS_NumFormat(LPSPREADSHEET lpSS, LPSS_CELLTYPE lpCellType, double dfVal, LPTSTR lpszText, BOOL fValue);
static BOOL SS_FormatDataCurrency(LPSPREADSHEET lpSS, LPSS_CELLTYPE lpCellType, LPCTSTR lpszText,
                           LPTSTR lpszTextFmt, LPDOUBLE lpdfValue, BOOL fValue, BOOL fCheckRange);
static BOOL SS_FormatDataNumber(LPSPREADSHEET lpSS, LPSS_CELLTYPE lpCellType, LPCTSTR lpszText,
                           LPTSTR lpszTextFmt, LPDOUBLE lpdfValue, BOOL fValue, BOOL fCheckRange);
static BOOL SS_FormatDataPercent(LPSPREADSHEET lpSS, LPSS_CELLTYPE lpCellType, LPCTSTR lpszText,
                          LPTSTR lpszTextFmt, LPDOUBLE lpdfValue, BOOL fValue, BOOL fCheckRange);
#endif // SS_V40
#ifdef SS_V70
static BOOL SS_FormatDataScientific(LPSPREADSHEET lpSS, LPSS_CELLTYPE lpCellType, LPCTSTR lpszText,
                          LPTSTR lpszTextFmt, LPDOUBLE lpdfValue, BOOL fValue, BOOL fCheckRange);
#endif SS_V70
static BOOL           SS_FormatDataInteger(LPSS_CELLTYPE lpCellType,
                                           LPCTSTR lpszText, LPTSTR lpszTextFmt,
                                           LPLONG lplValue, BOOL fValue, BOOL fCheckRange);
static TBGLOBALHANDLE SS_FormatDataEdit(LPSS_BOOK lpBook, LPSS_CELLTYPE lpCellType,
                                        LPCTSTR lpszText, LPTSTR lpszTextFmt,
                                        BOOL fValue, BOOL fCheckLen);
static BOOL           SS_UnFormatDataPic(LPSS_CELLTYPE lpCellType,
                                         LPTSTR lpszText, LPTSTR lpszTextValue);
static BOOL           SS_UnFormatDataDate(LPSPREADSHEET lpSS, LPSS_CELLTYPE lpCellType,
                                          LPTSTR lpszText,
                                          LPTSTR lpszTextValue);
static BOOL           SS_UnFormatDataTime(LPSS_CELLTYPE lpCellType,
                                          LPTSTR lpszText,
                                          LPTSTR lpszTextValue);
static short          SS_ComboBoxInsert(LPSS_CELLTYPE lpCellType,
                                        LPTSTR lpText, short dItemNum);
static short          SS_ComboBoxDelete(LPSS_CELLTYPE lpCellType,
                                        short dItemNum);

static int            SS_DateIsLeapYear(int Year);
static int            SS_DateDaysPerMonth(int Month, int Year);
static BOOL           SS_DateIsValid(int Month, int Day, int Year);
static long           SS_TranslateEditStyle(long lStyle);

// fix for bug #8154 -scl
#ifdef SS_V30
extern TBGLOBALHANDLE SSCB_GetTextFromListEx(HWND hWnd, long dSel);
#endif

LPSS_CELLTYPE DLLENTRY SSSetTypeEdit(HWND hWnd, LPSS_CELLTYPE CellType,
                                     long Style, int Len, short ChrSet,
                                     short ChrCase)
{
return SS_SetTypeEdit(CellType, Style, Len, ChrSet, ChrCase);
}


LPSS_CELLTYPE SS_SetTypeEdit(LPSS_CELLTYPE CellType,
                             long Style, int Len, short ChrSet,
                             short ChrCase)
{
_fmemset(CellType, '\0', sizeof(SS_CELLTYPE));

if (Style & ES_MULTILINE)
   Style |= ES_AUTOVSCROLL;

CellType->Type              = SS_TYPE_EDIT;
CellType->Style             = SS_TranslateEditStyle(Style);
CellType->Spec.Edit.Len     = Len;
CellType->Spec.Edit.ChrSet  = ChrSet;
CellType->Spec.Edit.ChrCase = ChrCase;

return (CellType);
}


#ifdef SS_GP

LPSS_CELLTYPE DLLENTRY SSSetTypeEditExpanding(hWnd, CellType, Style, Len,
                                              ChrSet, ChrCase, dfWidth,
                                              nRowCnt)

HWND          hWnd;
LPSS_CELLTYPE CellType;
long          Style;
short         Len;
short         ChrSet;
short         ChrCase;
double        dfWidth;
short         nRowCnt;
{
_fmemset(CellType, '\0', sizeof(SS_CELLTYPE));

CellType->Type              = SS_TYPE_EDIT;
CellType->Style             = Style;
CellType->Spec.Edit.Len     = Len;
CellType->Spec.Edit.ChrSet  = ChrSet;
CellType->Spec.Edit.ChrCase = ChrCase;
CellType->Spec.Edit.dfWidth = dfWidth;
CellType->Spec.Edit.nRowCnt = nRowCnt;

if (dfWidth)
   CellType->Style          |= ES_MULTILINE;

return (CellType);
}

#endif


LPSS_CELLTYPE DLLENTRY SSSetTypePic(HWND hWnd, LPSS_CELLTYPE CellType,
                                    long Style, LPCTSTR Mask)
{
LPSPREADSHEET lpSS;
LPSS_CELLTYPE Ret;

lpSS = SS_SheetLock(hWnd);
Ret = SS_SetTypePic(lpSS, CellType, Style, Mask);
SS_SheetUnlock(hWnd);
return Ret;
}


LPSS_CELLTYPE SS_SetTypePic(LPSPREADSHEET lpSS, LPSS_CELLTYPE CellType,
                            long Style, LPCTSTR Mask)
{
_fmemset(CellType, '\0', sizeof(SS_CELLTYPE));

if (!Mask)
   return (NULL);

CellType->Type  = SS_TYPE_PIC;
CellType->Style = SS_TranslateEditStyle(Style);

if (!(CellType->Spec.Pic.hMask = SS_tbCreateTextHandle(Mask, 0)))
   return (NULL);

return (CellType);
}


GLOBALHANDLE SS_CreateTextHandle(lpszText, Len)

LPCTSTR      lpszText;
short        Len;
{
GLOBALHANDLE hText;
LPTSTR       lpszTextTemp;

if (Len <= 0)
   Len = lstrlen(lpszText) + 1;

if (!(hText = GlobalAlloc(GMEM_MOVEABLE | GMEM_ZEROINIT, Len * sizeof(TCHAR))))
   return (0);

lpszTextTemp = (LPTSTR)GlobalLock(hText);
lstrcpy(lpszTextTemp, lpszText);
GlobalUnlock(hText);

return (hText);
}


static TBGLOBALHANDLE SS_tbCreateTextHandle(lpszText, Len)

LPCTSTR        lpszText;
short          Len;
{
TBGLOBALHANDLE hText;
LPTSTR         lpszTextTemp;

if (!lpszText)
   return (0);

if (Len == 0)
   Len = lstrlen(lpszText) + 1;

if (!(hText = tbGlobalAlloc(GMEM_MOVEABLE | GMEM_ZEROINIT, Len * sizeof(TCHAR))))
   return (0);

lpszTextTemp = (LPTSTR)tbGlobalLock(hText);
lstrcpy(lpszTextTemp, lpszText);
tbGlobalUnlock(hText);

return (hText);
}


#ifndef SS_NOCT_COMBO

LPSS_CELLTYPE DLLENTRY SSSetTypeComboBox(HWND hWnd, LPSS_CELLTYPE CellType,
                                         long Style, LPCTSTR lpItems)
{
LPSPREADSHEET lpSS;
LPSS_CELLTYPE Ret;

lpSS = SS_SheetLock(hWnd);
Ret = SS_SetTypeComboBox(lpSS, CellType, Style, lpItems, -1, -1, 0, 0, SS_COMBOBOX_AUTOSEARCH_SINGLECHAR);
SS_SheetUnlock(hWnd);
return Ret;
}

#ifdef SS_V40
LPSS_CELLTYPE DLLENTRY SSSetTypeComboBoxEx(HWND hWnd, LPSS_CELLTYPE CellType,
                                           long Style, LPCTSTR lpItems,
                                           short dMaxRows, short dMaxEditLen,
                                           short dComboWidth, HWND hWndDropDown, BYTE bAutoSearch)
{
LPSPREADSHEET lpSS;
LPSS_CELLTYPE Ret;

lpSS = SS_SheetLock(hWnd);
Ret = SS_SetTypeComboBox(lpSS, CellType, Style, lpItems, dMaxRows, dMaxEditLen,
                         dComboWidth, hWndDropDown, bAutoSearch);
SS_SheetUnlock(hWnd);
return Ret;
}

#else

LPSS_CELLTYPE DLLENTRY SSSetTypeComboBoxEx(HWND hWnd, LPSS_CELLTYPE CellType,
                                           long Style, LPCTSTR lpItems,
                                           short dMaxRows, short dMaxEditLen,
                                           short dComboWidth, HWND hWndDropDown)
{
LPSPREADSHEET lpSS;
LPSS_CELLTYPE Ret;

lpSS = SS_SheetLock(hWnd);
Ret = SS_SetTypeComboBox(lpSS, CellType, Style, lpItems, dMaxRows, dMaxEditLen,
                         dComboWidth, hWndDropDown, SS_COMBOBOX_AUTOSEARCH_SINGLECHAR);
SS_SheetUnlock(hWnd);
return Ret;
}
#endif // SS_V40


LPSS_CELLTYPE SS_SetTypeComboBox(LPSPREADSHEET lpSS, LPSS_CELLTYPE CellType,
                                 long Style, LPCTSTR lpItems, short dMaxRows,
                                 short dMaxEditLen, short dComboWidth,
                                 HWND hWndDropDown, BYTE bAutoSearch)
{
TBGLOBALHANDLE hItems = 0;
LPTSTR         lpItemsTemp;
LPTSTR         Ptr;
short          dItemCnt;

_fmemset(CellType, '\0', sizeof(SS_CELLTYPE));

CellType->Type  = SS_TYPE_COMBOBOX;
CellType->Style = Style;

if (lpItems)
   {
   if (!(hItems = tbGlobalAlloc(GMEM_MOVEABLE | GMEM_ZEROINIT,
                                (lstrlen(lpItems) + 1) * sizeof(TCHAR))))
      return (0);

   lpItemsTemp = (LPTSTR)tbGlobalLock(hItems);
   lstrcpy(lpItemsTemp, lpItems);

   dItemCnt = 1;

   Ptr = lpItemsTemp;
   while (Ptr = _ftcschr(Ptr, '\t'))
      {
      *Ptr = '\0';
      Ptr++;

      if (*Ptr)
         dItemCnt++;
      }

   CellType->Spec.ComboBox.dItemCnt = dItemCnt;

   tbGlobalUnlock(hItems);
   }

CellType->Spec.ComboBox.hItems = hItems;
CellType->Spec.ComboBox.dMaxRows = min(SHRT_MAX, dMaxRows);
CellType->Spec.ComboBox.dMaxEditLen = dMaxEditLen;
CellType->Spec.ComboBox.dComboWidth = dComboWidth;
CellType->Spec.ComboBox.hWndDropDown = hWndDropDown;
#ifdef SS_V40
CellType->Spec.ComboBox.AutoSearch = bAutoSearch;
#endif // SS_V40

return (CellType);
}

#endif


LPSS_CELLTYPE DLLENTRY SSSetTypeStaticText(HWND hWnd, LPSS_CELLTYPE CellType,
                                           long TextStyle)
{
LPSPREADSHEET lpSS;
LPSS_CELLTYPE Ret;

lpSS = SS_SheetLock(hWnd);
Ret = SS_SetTypeStaticText(lpSS, CellType, TextStyle);
SS_SheetUnlock(hWnd);
return Ret;
}


LPSS_CELLTYPE SS_SetTypeStaticText(LPSPREADSHEET lpSS, LPSS_CELLTYPE CellType,
                                   long TextStyle)
{
_fmemset(CellType, '\0', sizeof(SS_CELLTYPE));

CellType->Type  = SS_TYPE_STATICTEXT;
CellType->Style = TextStyle;

#ifdef SS_DEFPREFIX
CellType->Style |= SS_TEXT_PREFIX;
#endif

return (CellType);
}


#ifndef SS_NOCT_OWNERDRAW

LPSS_CELLTYPE DLLENTRY SSSetTypeOwnerDraw(HWND hWnd, LPSS_CELLTYPE CellType,
                                          long lStyle)
{
LPSPREADSHEET lpSS;
LPSS_CELLTYPE Ret;

lpSS = SS_SheetLock(hWnd);
Ret = SS_SetTypeOwnerDraw(lpSS, CellType, lStyle);
SS_SheetUnlock(hWnd);
return Ret;
}


LPSS_CELLTYPE SS_SetTypeOwnerDraw(LPSPREADSHEET lpSS, LPSS_CELLTYPE CellType,
                                  long lStyle)
{
_fmemset(CellType, '\0', sizeof(SS_CELLTYPE));

CellType->Type  = SS_TYPE_OWNERDRAW;
CellType->Style = lStyle;

return (CellType);
}

#endif


#ifndef SS_NOCT_BUTTON

LPSS_CELLTYPE DLLENTRY SSSetTypeButton(HWND hWnd, LPSS_CELLTYPE CellType,
                                       long  Style, LPCTSTR Text,
                                       LPCTSTR Picture, short nPictureType,
                                       LPCTSTR PictureDown, short nPictureDownType,
                                       short ButtonType, short nShadowSize,
                                       LPSUPERBTNCOLOR lpColor)
{
LPSPREADSHEET lpSS;
LPSS_CELLTYPE Ret;

lpSS = SS_SheetLock(hWnd);
Ret = SS_SetTypeButton(lpSS, CellType, Style, Text, Picture, nPictureType,
                       PictureDown, nPictureDownType, ButtonType, nShadowSize,
                       lpColor);
SS_SheetUnlock(hWnd);
return Ret;
}


LPSS_CELLTYPE SS_SetTypeButton(LPSPREADSHEET lpSS, LPSS_CELLTYPE CellType,
                               long  Style, LPCTSTR Text,
                               LPCTSTR Picture, short nPictureType,
                               LPCTSTR PictureDown, short nPictureDownType,
                               short ButtonType, short nShadowSize,
                               LPSUPERBTNCOLOR lpColor)
{
short           dRet = TRUE;

_fmemset(CellType, '\0', sizeof(SS_CELLTYPE));

CellType->Type  = SS_TYPE_BUTTON;
CellType->Style = Style | SBS_NOPOINTERFOCUS;

CellType->Spec.Button.nPictureType = nPictureType;
CellType->Spec.Button.nPictureDownType = nPictureDownType;

CellType->Spec.Button.ButtonType = (BYTE)ButtonType;
CellType->Spec.Button.ShadowSize = (BYTE)nShadowSize;

if (lpColor)
   {
   _fmemcpy(&CellType->Spec.Button.Color, lpColor, sizeof(SUPERBTNCOLOR));

   if (SS_ISDEFCOLOR(CellType->Spec.Button.Color.Color))
      CellType->Spec.Button.Color.Color = SS_EncodeSysColor(COLOR_BTNFACE);

   if (SS_ISDEFCOLOR(CellType->Spec.Button.Color.ColorBorder))
      CellType->Spec.Button.Color.ColorBorder = SS_EncodeSysColor(COLOR_BTNTEXT);

   if (SS_ISDEFCOLOR(CellType->Spec.Button.Color.ColorShadow))
      CellType->Spec.Button.Color.ColorShadow = SS_EncodeSysColor(COLOR_BTNSHADOW);

   if (SS_ISDEFCOLOR(CellType->Spec.Button.Color.ColorHighlight))
      CellType->Spec.Button.Color.ColorHighlight = SS_EncodeSysColor(COLOR_BTNHIGHLIGHT);

   if (SS_ISDEFCOLOR(CellType->Spec.Button.Color.ColorText))
      CellType->Spec.Button.Color.ColorText = SS_EncodeSysColor(COLOR_BTNTEXT);
   }

if (Text)
   if (!(CellType->Spec.Button.hText = SS_CreateTextHandle(Text, 0)))
      return (NULL);

if (Picture)
   if (nPictureType & SUPERBTN_PICT_HANDLE)
      CellType->Spec.Button.hPictName = *(HANDLE FAR *)Picture;
   else
      if (!(CellType->Spec.Button.hPictName = SS_CreateTextHandle(Picture, 0)))
         {
         if (CellType->Spec.Button.hText)
            {
            GlobalFree(CellType->Spec.Button.hText);
            CellType->Spec.Button.hText = 0;
            }

         return (NULL);
         }

if (PictureDown)
   if (nPictureDownType & SUPERBTN_PICT_HANDLE)
      CellType->Spec.Button.hPictDownName = *(HANDLE FAR *)PictureDown;
   else
      if (!(CellType->Spec.Button.hPictDownName =
            SS_CreateTextHandle(PictureDown, 0)))
         {
         if (!(nPictureType & SUPERBTN_PICT_HANDLE) &&
             CellType->Spec.Button.hPictName)
            {
            GlobalFree(CellType->Spec.Button.hPictName);
            CellType->Spec.Button.hPictName = 0;
            }

         if (CellType->Spec.Button.hText)
            {
            GlobalFree(CellType->Spec.Button.hText);
            CellType->Spec.Button.hText = 0;
            }

         return (NULL);
         }

/*
lpSS = SS_Lock(hWnd);

if (!lpColor && CellType->ControlID)
   SendMessage(SS_GetControlhWnd(lpSS, CellType->ControlID), SBM_GETCOLOR,
               0, (long)(LPVOID)&CellType->Spec.Button.Color);

SS_Unlock(hWnd);
*/

if (!lpColor)
   {
   CellType->Spec.Button.Color.Color          = RGBCOLOR_DEFAULT;
   CellType->Spec.Button.Color.ColorBorder    = RGBCOLOR_DEFAULT;
   CellType->Spec.Button.Color.ColorShadow    = RGBCOLOR_DEFAULT;
#if (WINVER >= 0x030a)
   CellType->Spec.Button.Color.ColorHighlight = RGBCOLOR_DEFAULT;
#else
   CellType->Spec.Button.Color.ColorHighlight = RGBCOLOR_WHITE;
#endif
   CellType->Spec.Button.Color.ColorText      = RGBCOLOR_DEFAULT;
   }

return (CellType);
}


BOOL DLLENTRY SSSetButtonText(HWND hWnd, SS_COORD Col, SS_COORD Row,
                              LPCTSTR lpText)
{
LPSPREADSHEET lpSS;
SS_CELLTYPE   CellType;
LPTSTR        lpPict;
GLOBALHANDLE  hPict;
BOOL          fRet = FALSE;

lpSS = SS_SheetLock(hWnd);
SS_AdjustCellCoords(lpSS, &Col, &Row);
if (SS_GetCellType(lpSS, Col, Row, &CellType))
   {
   if (CellType.Type == SS_TYPE_BUTTON)
      {
      if (CellType.Spec.Button.hPictName &&
          !(CellType.Spec.Button.nPictureType & SUPERBTN_PICT_HANDLE))
         {
         lpPict = (LPTSTR)GlobalLock(CellType.Spec.Button.hPictName);
         hPict = SS_CreateTextHandle(lpPict, 0);
         GlobalUnlock(CellType.Spec.Button.hPictName);
         CellType.Spec.Button.hPictName = hPict;
         }

      if (CellType.Spec.Button.hPictDownName &&
          !(CellType.Spec.Button.nPictureDownType & SUPERBTN_PICT_HANDLE))
         {
         lpPict = (LPTSTR)GlobalLock(CellType.Spec.Button.hPictDownName);
         hPict = SS_CreateTextHandle(lpPict, 0);
         GlobalUnlock(CellType.Spec.Button.hPictDownName);
         CellType.Spec.Button.hPictDownName = hPict;
         }

      CellType.Spec.Button.hText = 0;

      if (lpText && lstrlen(lpText))
         CellType.Spec.Button.hText = SS_CreateTextHandle(lpText, 0);

      fRet = SS_SetCellTypeRange(lpSS, Col, Row, Col, Row, &CellType);
      }
   }

SS_SheetUnlock(hWnd);

return (fRet);
}


int DLLENTRY SSGetButtonText(HWND hWnd, SS_COORD Col, SS_COORD Row,
                             LPTSTR lpText)
{
SS_CELLTYPE CellType;
LPTSTR      lpTextTemp;
int         iRet = 0;

if (lpText)
   *lpText = '\0';

if (SSGetCellType(hWnd, Col, Row, &CellType))
   {
   if (CellType.Type == SS_TYPE_BUTTON)
      {
      if (CellType.Spec.Button.hText)
         {
         lpTextTemp = (LPTSTR)GlobalLock(CellType.Spec.Button.hText);
         iRet = lstrlen(lpTextTemp);
         if (lpText)
            lstrcpy(lpText, lpTextTemp);
         GlobalUnlock(CellType.Spec.Button.hText);
         }
      }
   }

return (iRet);
}

#endif


#ifndef SS_NOCT_CHECK

LPSS_CELLTYPE DLLENTRY SSSetTypeCheckBox(HWND hWnd, LPSS_CELLTYPE lpCellType,
                                         long lStyle, LPCTSTR lpText,
                                         LPCTSTR lpPictUp,
                                         WORD wPictUpType,
                                         LPCTSTR lpPictDown,
                                         WORD wPictDownType,
                                         LPCTSTR lpPictFocusUp,
                                         WORD wPictFocusUpType,
                                         LPCTSTR lpPictFocusDown,
                                         WORD wPictFocusDownType,
                                         LPCTSTR lpPictGray,
                                         WORD wPictGrayType,
                                         LPCTSTR lpPictFocusGray,
                                         WORD wPictFocusGrayType)
{
LPSPREADSHEET lpSS;
LPSS_CELLTYPE Ret;

lpSS = SS_SheetLock(hWnd);
Ret = SS_SetTypeCheckBox(lpSS, lpCellType, lStyle, lpText,
                         lpPictUp, wPictUpType,
                         lpPictDown, wPictDownType,
                         lpPictFocusUp, wPictFocusUpType,
                         lpPictFocusDown, wPictFocusDownType,
                         lpPictGray, wPictGrayType,
                         lpPictFocusGray, wPictFocusGrayType);
SS_SheetUnlock(hWnd);
return Ret;
}


void SSx_SetTypeCheckBoxDefaults(LPSS_BOOK lpBook, LPSS_CELLTYPE lpCellType)
{
CHECKBOXPICTS Picts;

if (!lpBook->fCheckBoxDefBitmapsLoaded)
   {
   lpBook->fCheckBoxDefBitmapsLoaded = TRUE;
   CheckBoxLoadDefBitmaps(hDynamicInst);
   }

tbCheckBoxGetPict(hDynamicInst, &Picts);

lpCellType->Spec.CheckBox.hPictUpName = (TBGLOBALHANDLE)Picts.PictUp.hPict;
lpCellType->Spec.CheckBox.bPictUpType |= BT_HANDLE | Picts.PictUp.PictType;
lpCellType->Spec.CheckBox.hPictDownName = (TBGLOBALHANDLE)Picts.PictDown.hPict;
lpCellType->Spec.CheckBox.bPictDownType |= BT_HANDLE | Picts.PictDown.PictType;
lpCellType->Spec.CheckBox.hPictFocusUpName = (TBGLOBALHANDLE)Picts.PictFocusUp.hPict;
lpCellType->Spec.CheckBox.bPictFocusUpType |= BT_HANDLE | Picts.PictFocusUp.PictType;
lpCellType->Spec.CheckBox.hPictFocusDownName = (TBGLOBALHANDLE)Picts.PictFocusDown.hPict;
lpCellType->Spec.CheckBox.bPictFocusDownType |= BT_HANDLE | Picts.PictFocusDown.PictType;
lpCellType->Spec.CheckBox.hPictGrayName = (TBGLOBALHANDLE)Picts.PictGray.hPict;
lpCellType->Spec.CheckBox.bPictGrayType |= BT_HANDLE | Picts.PictGray.PictType;
lpCellType->Spec.CheckBox.hPictFocusGrayName = (TBGLOBALHANDLE)Picts.PictFocusGray.hPict;
lpCellType->Spec.CheckBox.bPictFocusGrayType |= BT_HANDLE | Picts.PictFocusGray.PictType;
}


LPSS_CELLTYPE SS_SetTypeCheckBox(LPSPREADSHEET lpSS, LPSS_CELLTYPE lpCellType,
                                 long lStyle, LPCTSTR lpText,
                                 LPCTSTR lpPictUp, WORD wPictUpType,
                                 LPCTSTR lpPictDown, WORD wPictDownType,
                                 LPCTSTR lpPictFocusUp, WORD wPictFocusUpType,
                                 LPCTSTR lpPictFocusDown,
                                 WORD wPictFocusDownType,
                                 LPCTSTR lpPictGray,
                                 WORD wPictGrayType,
                                 LPCTSTR lpPictFocusGray,
                                 WORD wPictFocusGrayType)
{
BOOL fRet = TRUE;

_fmemset(lpCellType, '\0', sizeof(SS_CELLTYPE));

lpCellType->Type  = SS_TYPE_CHECKBOX;

#ifdef SS_UTP

if ((lStyle & 0x0F) == BS_3STATE || (lStyle & 0x0F) == BS_AUTO3STATE)
   lpCellType->Style = lStyle | BS_3STATE;
else
   lpCellType->Style = lStyle | BS_CHECKBOX;

#else

if ((lStyle & 0x0F) == BS_3STATE || (lStyle & 0x0F) == BS_AUTO3STATE)
   lpCellType->Style = lStyle | BS_AUTO3STATE;
else
   lpCellType->Style = lStyle | BS_AUTOCHECKBOX;

#endif

lpCellType->Spec.CheckBox.bPictUpType = (BYTE)wPictUpType;
lpCellType->Spec.CheckBox.bPictDownType = (BYTE)wPictDownType;
lpCellType->Spec.CheckBox.bPictFocusUpType = (BYTE)wPictFocusUpType;
lpCellType->Spec.CheckBox.bPictFocusDownType = (BYTE)wPictFocusDownType;
lpCellType->Spec.CheckBox.bPictGrayType = (BYTE)wPictGrayType;
lpCellType->Spec.CheckBox.bPictFocusGrayType = (BYTE)wPictFocusGrayType;

if (lpText && *lpText)
   if (!(lpCellType->Spec.CheckBox.hText = SS_CreateTextHandle(lpText, 0)))
      return (NULL);

if (lpPictUp)
   if (wPictUpType & BT_HANDLE)
      lpCellType->Spec.CheckBox.hPictUpName = (TBGLOBALHANDLE)(*(HANDLE FAR *)lpPictUp);
   else if (!(lpCellType->Spec.CheckBox.hPictUpName =
              SS_tbCreateTextHandle(lpPictUp, 0)))
      fRet = FALSE;

if (fRet && lpPictDown)
   if (wPictDownType & BT_HANDLE)
      lpCellType->Spec.CheckBox.hPictDownName = (TBGLOBALHANDLE)(*(HANDLE FAR *)lpPictDown);
   else if (!(lpCellType->Spec.CheckBox.hPictDownName =
              SS_tbCreateTextHandle(lpPictDown, 0)))
      fRet = FALSE;

if (fRet && lpPictFocusUp)
   if (wPictFocusUpType & BT_HANDLE)
      lpCellType->Spec.CheckBox.hPictFocusUpName =
         (TBGLOBALHANDLE)(*(HANDLE FAR *)lpPictFocusUp);
   else if (!(lpCellType->Spec.CheckBox.hPictFocusUpName =
              SS_tbCreateTextHandle(lpPictFocusUp, 0)))
      fRet = FALSE;

if (fRet && lpPictFocusDown)
   if (wPictFocusDownType & BT_HANDLE)
      lpCellType->Spec.CheckBox.hPictFocusDownName =
         (TBGLOBALHANDLE)(*(HANDLE FAR *)lpPictFocusDown);
   else if (!(lpCellType->Spec.CheckBox.hPictFocusDownName =
              SS_tbCreateTextHandle(lpPictFocusDown, 0)))
      fRet = FALSE;

if (fRet && lpPictGray)
   if (wPictGrayType & BT_HANDLE)
      lpCellType->Spec.CheckBox.hPictGrayName =
         (TBGLOBALHANDLE)(*(HANDLE FAR *)lpPictGray);
   else if (!(lpCellType->Spec.CheckBox.hPictGrayName =
              SS_tbCreateTextHandle(lpPictGray, 0)))
      fRet = FALSE;

if (fRet && lpPictFocusGray)
   if (wPictFocusGrayType & BT_HANDLE)
      lpCellType->Spec.CheckBox.hPictFocusGrayName =
         (TBGLOBALHANDLE)(*(HANDLE FAR *)lpPictFocusGray);
   else if (!(lpCellType->Spec.CheckBox.hPictFocusGrayName =
              SS_tbCreateTextHandle(lpPictFocusGray, 0)))
      fRet = FALSE;

if (!fRet)
   {
   if (!(wPictUpType & BT_HANDLE) &&
        lpCellType->Spec.CheckBox.hPictUpName)
      tbGlobalFree(lpCellType->Spec.CheckBox.hPictUpName);

   if (!(wPictDownType & BT_HANDLE) &&
        lpCellType->Spec.CheckBox.hPictDownName)
      tbGlobalFree(lpCellType->Spec.CheckBox.hPictDownName);

   if (!(wPictFocusUpType & BT_HANDLE) &&
        lpCellType->Spec.CheckBox.hPictFocusUpName)
      tbGlobalFree(lpCellType->Spec.CheckBox.hPictFocusUpName);

   if (!(wPictFocusDownType & BT_HANDLE) &&
        lpCellType->Spec.CheckBox.hPictFocusDownName)
      tbGlobalFree(lpCellType->Spec.CheckBox.hPictFocusDownName);

   if (!(wPictGrayType & BT_HANDLE) &&
        lpCellType->Spec.CheckBox.hPictGrayName)
      tbGlobalFree(lpCellType->Spec.CheckBox.hPictGrayName);

   if (!(wPictFocusGrayType & BT_HANDLE) &&
        lpCellType->Spec.CheckBox.hPictFocusGrayName)
      tbGlobalFree(lpCellType->Spec.CheckBox.hPictFocusGrayName);

   return (0);
   }

if (!lpPictUp && !lpPictDown && !lpPictFocusUp && !lpPictFocusDown &&
    !lpPictGray && !lpPictFocusGray)
	SSx_SetTypeCheckBoxDefaults(lpSS->lpBook, lpCellType);

return (lpCellType);
}

#endif


#ifndef SS_NOCT_DATE

LPSS_CELLTYPE DLLENTRY SSSetTypeDate(HWND hWnd, LPSS_CELLTYPE CellType,
                                     long Style, LPDATEFORMAT lpFormat,
                                     LPTB_DATE lpMin, LPTB_DATE lpMax)
{
LPSPREADSHEET lpSS;
LPSS_CELLTYPE Ret;

lpSS = SS_SheetLock(hWnd);
Ret = SS_SetTypeDate(lpSS, CellType, Style, lpFormat, lpMin, lpMax);
SS_SheetUnlock(hWnd);
return Ret;
}


LPSS_CELLTYPE SS_SetTypeDate(LPSPREADSHEET lpSS, LPSS_CELLTYPE CellType,
                             long Style, LPDATEFORMAT lpFormat,
                             LPTB_DATE lpMin, LPTB_DATE lpMax)
{

#ifdef SPREAD_JPN
// 96' 6/24 Added by BOC Gao. copied from EditDate.C
// - For load resource before Date window is created.
// - So that if user only set CellType , Format and 
// - Value (Text) property can work correctly. 
int i;
//- Load the separator string from the resources file. 
LoadString(hDynamicInst, IDS_SEPARATOR_YEAR +LANGUAGE_BASE, szYearGlobal,  sizeof(szYearGlobal)-1);
LoadString(hDynamicInst, IDS_SEPARATOR_MONTH+LANGUAGE_BASE, szMonthGlobal, sizeof(szMonthGlobal)-1);
LoadString(hDynamicInst, IDS_SEPARATOR_DAY  +LANGUAGE_BASE, szDayGlobal,   sizeof(szDayGlobal)-1);

//- Load the Era names from the resource file.
for ( i = 0; i < 4; i++ )
{
	TCHAR    szTemp[36];

	LoadString(hDynamicInst, IDS_ERA_MEIJI + i + LANGUAGE_BASE, szTemp,  sizeof(szTemp)-1);
	//- Copy the full Era name and it starts from the 2 byte in the string
  lstrcpy((LPTSTR)NengoName[i + 8], &szTemp[1]);
	//- Copy the first part of the Era name and it's the 2 and 3 byte.
#ifdef UNICODE
  szTemp[2] = '\0';
#else
  szTemp[3] = '\0';
#endif  
  lstrcpy((LPTSTR)NengoName[i + 4], &szTemp[1]);
	//- Copy the Era name prefix in the string
	szTemp[1] = '\0';
	lstrcpy((LPTSTR)NengoName[i], (LPTSTR)szTemp);
}
//Add by BOC 99.5.25 (hyt) for also load time separator--
	 LoadString(hDynamicInst, IDS_SEPARATOR_HOUR + LANGUAGE_BASE, szHourGlobal,  STRING_SIZE(szHourGlobal));
	 LoadString(hDynamicInst, IDS_SEPARATOR_MINUTE + LANGUAGE_BASE, szMinuteGlobal, STRING_SIZE(szMinuteGlobal));
	 LoadString(hDynamicInst, IDS_SEPARATOR_SECOND + LANGUAGE_BASE, szSecondGlobal,   STRING_SIZE(szSecondGlobal));

	 LoadString(hDynamicInst, IDS_TIME_AM + LANGUAGE_BASE, szAmGlobal,   STRING_SIZE(szAmGlobal));
	 LoadString(hDynamicInst, IDS_TIME_PM + LANGUAGE_BASE, szPmGlobal,   STRING_SIZE(szPmGlobal));
//-------------------------------------------------------

#endif	//SPREAD_JPN
// --------------------------------<<

_fmemset(CellType, '\0', sizeof(SS_CELLTYPE));

CellType->Type  = SS_TYPE_DATE;
CellType->Style = SS_TranslateEditStyle(Style);
CellType->Spec.Date.SetFormat = TRUE;

if (lpFormat)
   {
   _fmemcpy(&CellType->Spec.Date.Format, lpFormat, sizeof(DATEFORMAT));

   if (CellType->Spec.Date.Format.bSpin)
      CellType->Style |= DS_SPIN;
   }
else
   {
   DateGetFormat(0, &CellType->Spec.Date.Format);

   if (Style & DS_SPIN)
      CellType->Spec.Date.Format.bSpin = TRUE;

   if ((Style & DS_DDMONYY) == DS_DDMONYY)
      CellType->Spec.Date.Format.nFormat = IDF_DDMONYY;

   if ((Style & DS_DDMMYY) == DS_DDMMYY)
      CellType->Spec.Date.Format.nFormat = IDF_DDMMYY;

   if ((Style & DS_MMDDYY) == DS_MMDDYY)
      CellType->Spec.Date.Format.nFormat = IDF_MMDDYY;

   if ((Style & DS_YYMMDD) == DS_YYMMDD)
      CellType->Spec.Date.Format.nFormat = IDF_YYMMDD;

   if (Style & DS_CENTURY)
      CellType->Spec.Date.Format.bCentury = TRUE;
   else if (!(Style & DS_DEFCENTURY))
      CellType->Spec.Date.Format.bCentury = FALSE;
   }

if (lpMin && lpMax)
   {
   CellType->Spec.Date.SetRange = TRUE;
   _fmemcpy(&CellType->Spec.Date.Min, lpMin, sizeof(TB_DATE));
   _fmemcpy(&CellType->Spec.Date.Max, lpMax, sizeof(TB_DATE));
   }

return (CellType);
}

#endif


#ifndef SS_NOCT_TIME

LPSS_CELLTYPE DLLENTRY SSSetTypeTime(HWND hWnd, LPSS_CELLTYPE CellType,
                                     long Style, LPTIMEFORMAT lpFormat,
                                     LPTIME lpMin, LPTIME lpMax)
{
LPSPREADSHEET lpSS;
LPSS_CELLTYPE Ret;

lpSS = SS_SheetLock(hWnd);
Ret = SS_SetTypeTime(lpSS, CellType, Style, lpFormat, lpMin, lpMax);
SS_SheetUnlock(hWnd);
return Ret;
}


LPSS_CELLTYPE SS_SetTypeTime(LPSPREADSHEET lpSS, LPSS_CELLTYPE CellType,
                             long Style, LPTIMEFORMAT lpFormat,
                             LPTIME lpMin, LPTIME lpMax)
{

#ifdef SPREAD_JPN
//Add by BOC 99.5.25 (hyt) for also load time separator--
// - For load resource before Time window is created.
// - So that if user only set CellType , Format and 
// - Value (Text) property can work correctly. 

	 LoadString(hDynamicInst, IDS_SEPARATOR_HOUR + LANGUAGE_BASE, szHourGlobal,  STRING_SIZE(szHourGlobal));
	 LoadString(hDynamicInst, IDS_SEPARATOR_MINUTE + LANGUAGE_BASE, szMinuteGlobal, STRING_SIZE(szMinuteGlobal));
	 LoadString(hDynamicInst, IDS_SEPARATOR_SECOND + LANGUAGE_BASE, szSecondGlobal,   STRING_SIZE(szSecondGlobal));

	 LoadString(hDynamicInst, IDS_TIME_AM + LANGUAGE_BASE, szAmGlobal,   STRING_SIZE(szAmGlobal));
	 LoadString(hDynamicInst, IDS_TIME_PM + LANGUAGE_BASE, szPmGlobal,   STRING_SIZE(szPmGlobal));
//-------------------------------------------------------
#endif	//SPREAD_JPN
// --------------------------------<<

_fmemset(CellType, '\0', sizeof(SS_CELLTYPE));

CellType->Type  = SS_TYPE_TIME;
CellType->Style = SS_TranslateEditStyle(Style);

if (lpFormat)
   {
   CellType->Spec.Time.SetFormat = TRUE;
   _fmemcpy(&CellType->Spec.Time.Format, lpFormat, sizeof(TIMEFORMAT));

   if (CellType->Spec.Time.Format.bSpin)
      CellType->Style |= TS_SPIN;
   }

else
   {
   CellType->Spec.Time.SetFormat = TRUE;
   /*
   TimeGetFormat(SS_GetControlhWnd(lpSS, CellType->ControlID),
                 &CellType->Spec.Time.Format);
   */
   SS_TimeGetDefault(&CellType->Spec.Time.Format);

   if (Style & TS_SPIN)
      CellType->Spec.Time.Format.bSpin = TRUE;

   if (Style & TS_12HOUR)
      CellType->Spec.Time.Format.b24Hour = FALSE;

   if (Style & TS_24HOUR)
      CellType->Spec.Time.Format.b24Hour = TRUE;

   if (Style & TS_SECONDS)
      CellType->Spec.Time.Format.bSeconds = TRUE;
   }

if (lpMin && lpMax)
   {
   CellType->Spec.Time.SetRange = TRUE;
   _fmemcpy(&CellType->Spec.Time.Min, lpMin, sizeof(TIME));
   _fmemcpy(&CellType->Spec.Time.Max, lpMax, sizeof(TIME));
   }

return (CellType);
}


static void SS_TimeGetDefault(LPTIMEFORMAT lpTimeFormat)
{
TCHAR szTemp[2];

lpTimeFormat->b24Hour = (BOOL)GetProfileInt(_T("intl"), _T("iTime"), 0);
lpTimeFormat->bSeconds = FALSE;

GetProfileString(_T("intl"), _T("sTime"), _T(":"), szTemp, 2);
lpTimeFormat->cSeparator = szTemp[0];
lpTimeFormat->bSpin = 0;
}

#endif


#ifndef SS_NOCT_INT

LPSS_CELLTYPE DLLENTRY SSSetTypeInteger(hWnd, CellType, lMin, lMax)

HWND          hWnd;
LPSS_CELLTYPE CellType;
long          lMin;
long          lMax;
{
return (SSSetTypeIntegerExt(hWnd, CellType, 0L, lMin, lMax, FALSE, 0L));
}


LPSS_CELLTYPE DLLENTRY SSSetTypeIntegerExt(HWND hWnd, LPSS_CELLTYPE CellType,
                                           long lStyle, long lMin, long lMax,
                                           BOOL fSpinWrap, long lSpinInc)
{
LPSPREADSHEET lpSS;
LPSS_CELLTYPE Ret;

lpSS = SS_SheetLock(hWnd);
Ret = SS_SetTypeIntegerExt(lpSS, CellType, lStyle, lMin, lMax,
                           fSpinWrap, lSpinInc);
SS_SheetUnlock(hWnd);
return Ret;
}


LPSS_CELLTYPE SS_SetTypeIntegerExt(LPSPREADSHEET lpSS, LPSS_CELLTYPE CellType,
                                   long lStyle, long lMin, long lMax,
                                   BOOL fSpinWrap, long lSpinInc)
{
_fmemset(CellType, '\0', sizeof(SS_CELLTYPE));

CellType->Type                   = SS_TYPE_INTEGER;
CellType->Style                  = SS_TranslateEditStyle(lStyle);
CellType->Spec.Integer.Min       = lMin;
CellType->Spec.Integer.Max       = lMax;
CellType->Spec.Integer.fSpinWrap = fSpinWrap;
CellType->Spec.Integer.lSpinInc  = lSpinInc;

return (CellType);
}

#endif


LPSS_CELLTYPE DLLENTRY SSSetTypeFloat(HWND hWnd, LPSS_CELLTYPE CellType,
                                      long Style, short Left, short Right,
                                      double Min, double Max)
{
return (SSSetTypeFloatExt(hWnd, CellType, Style, Left, Right, Min, Max, NULL));
}


LPSS_CELLTYPE DLLENTRY SSSetTypeFloatExt(HWND hWnd, LPSS_CELLTYPE CellType,
                                         long Style, short Left, short Right,
                                         double Min, double Max,
                                         LPFLOATFORMAT lpFormat)
{
LPSPREADSHEET lpSS;
LPSS_CELLTYPE Ret;

lpSS = SS_SheetLock(hWnd);
Ret = SS_SetTypeFloatExt(lpSS, CellType, Style, Left, Right, Min, Max, lpFormat);
SS_SheetUnlock(hWnd);
return Ret;
}


LPSS_CELLTYPE SS_SetTypeFloatExt(LPSPREADSHEET lpSS, LPSS_CELLTYPE CellType,
                                 long Style, short Left, short Right,
                                 double Min, double Max,
                                 LPFLOATFORMAT lpFormat)
{
_fmemset(CellType, '\0', sizeof(SS_CELLTYPE));

CellType->Type             = SS_TYPE_FLOAT;
CellType->Style            = SS_TranslateEditStyle(Style);
CellType->Spec.Float.Left  = Left;
CellType->Spec.Float.Right = min(14, Right);
CellType->Spec.Float.Min   = Min;
CellType->Spec.Float.Max   = Max;

if (lpFormat)
   {
   _fmemcpy(&CellType->Spec.Float.Format, lpFormat, sizeof(FLOATFORMAT));
   CellType->Spec.Float.fSetFormat = TRUE;
   }

return (CellType);
}


#ifdef SS_V40
LPSS_CELLTYPE DLLENTRY SSSetTypeCurrency(HWND hWnd, LPSS_CELLTYPE lpCellType, long Style,
                          BYTE Right, double Min, double Max, BYTE fShowSeparator)
{
return SSSetTypeCurrencyEx(hWnd, lpCellType, Style, Right, Min, Max, fShowSeparator, TRUE,
                           NULL, NULL, NULL, 0, 0, 0, FALSE, FALSE, 1.1);
}


LPSS_CELLTYPE DLLENTRY SSSetTypeCurrencyEx(HWND hWnd, LPSS_CELLTYPE lpCellType, long Style,
                          BYTE Right, double Min, double Max, BYTE fShowSeparator, BYTE fShowCurrencySymbol,
                          LPTSTR lpszDecimal, LPTSTR lpszSeparator, LPTSTR lpszCurrency,
                          BYTE bLeadingZero, BYTE bNegCurrencyStyle, BYTE bPosCurrencyStyle,
                          BYTE fSpin, BYTE fSpinWrap, double SpinInc)
{
LPSPREADSHEET lpSS;
LPSS_CELLTYPE Ret;

lpSS = SS_SheetLock(hWnd);
Ret = SS_SetTypeCurrency(lpSS, lpCellType, Style, Right, Min, Max, fShowSeparator, fShowCurrencySymbol,
                         lpszDecimal, lpszSeparator, lpszCurrency, bLeadingZero, bNegCurrencyStyle,
                         bPosCurrencyStyle, fSpin, fSpinWrap, SpinInc);
SS_SheetUnlock(hWnd);
return Ret;
}


LPSS_CELLTYPE SS_SetTypeCurrency(LPSPREADSHEET lpSS, LPSS_CELLTYPE lpCellType, long Style,
                 BYTE Right, double Min, double Max, BYTE fShowSeparator, BYTE fShowCurrencySymbol,
                 LPTSTR lpszDecimal, LPTSTR lpszSeparator, LPTSTR lpszCurrency,
                 BYTE bLeadingZero, BYTE bNegCurrencyStyle, BYTE bPosCurrencyStyle,
                 BYTE fSpin, BYTE fSpinWrap, double SpinInc)
{
_fmemset(lpCellType, '\0', sizeof(SS_CELLTYPE));

lpCellType->Type = SS_TYPE_CURRENCY;
lpCellType->Style = SS_TranslateEditStyle(Style);
lpCellType->Spec.Currency.Min = Min;
lpCellType->Spec.Currency.Max = Max;
if (lpszDecimal)
	lstrcpyn(lpCellType->Spec.Currency.szDecimal, lpszDecimal, 4);
if (lpszSeparator)
	lstrcpyn(lpCellType->Spec.Currency.szSeparator, lpszSeparator, 4);
if (lpszCurrency)
	lstrcpyn(lpCellType->Spec.Currency.szCurrency, lpszCurrency, 6);
lpCellType->Spec.Currency.bLeadingZero = bLeadingZero;
lpCellType->Spec.Currency.Right = min(14, Right);
lpCellType->Spec.Currency.bNegStyle = bNegCurrencyStyle;
lpCellType->Spec.Currency.bPosStyle = bPosCurrencyStyle;
lpCellType->Spec.Currency.fShowSeparator = fShowSeparator;
lpCellType->Spec.Currency.fShowCurrencySymbol = fShowCurrencySymbol;
lpCellType->Spec.Currency.fSpin = fSpin;
lpCellType->Spec.Currency.fSpinWrap = fSpinWrap;
lpCellType->Spec.Currency.SpinInc = SpinInc;

return (lpCellType);
}


LPSS_CELLTYPE DLLENTRY SSSetTypeNumber(HWND hWnd, LPSS_CELLTYPE lpCellType, long Style,
                          BYTE Right, double Min, double Max, BYTE fShowSeparator)
{
return SSSetTypeNumberEx(hWnd, lpCellType, Style, Right, Min, Max, fShowSeparator, NULL, NULL,
                         0, 0, FALSE, FALSE, 1.1);
}


LPSS_CELLTYPE DLLENTRY SSSetTypeNumberEx(HWND hWnd, LPSS_CELLTYPE lpCellType, long Style,
                          BYTE Right, double Min, double Max, BYTE fShowSeparator, LPTSTR lpszDecimal,
                          LPTSTR lpszSeparator, BYTE bLeadingZero, BYTE bNegStyle,
                          BYTE fSpin, BYTE fSpinWrap, double SpinInc)
{
LPSPREADSHEET lpSS;
LPSS_CELLTYPE Ret;

lpSS = SS_SheetLock(hWnd);
Ret = SS_SetTypeNumber(lpSS, lpCellType, Style, Right, Min, Max, fShowSeparator, lpszDecimal, lpszSeparator,
                       bLeadingZero, bNegStyle, fSpin, fSpinWrap, SpinInc);
SS_SheetUnlock(hWnd);
return Ret;
}


LPSS_CELLTYPE SS_SetTypeNumber(LPSPREADSHEET lpSS, LPSS_CELLTYPE lpCellType, long Style,
                 BYTE Right, double Min, double Max, BYTE fShowSeparator, LPTSTR lpszDecimal,
                 LPTSTR lpszSeparator, BYTE bLeadingZero, BYTE bNegStyle, BYTE fSpin,
                 BYTE fSpinWrap, double SpinInc)
{
_fmemset(lpCellType, '\0', sizeof(SS_CELLTYPE));

lpCellType->Type = SS_TYPE_NUMBER;
lpCellType->Style = SS_TranslateEditStyle(Style);
lpCellType->Spec.Number.Min = Min;
lpCellType->Spec.Number.Max = Max;
if (lpszDecimal)
	lstrcpyn(lpCellType->Spec.Number.szDecimal, lpszDecimal, 4);
if (lpszSeparator)
	lstrcpyn(lpCellType->Spec.Number.szSeparator, lpszSeparator, 4);
lpCellType->Spec.Number.bLeadingZero = bLeadingZero;
lpCellType->Spec.Number.Right = min(14, Right);
lpCellType->Spec.Number.bNegStyle = bNegStyle;
lpCellType->Spec.Number.fShowSeparator = fShowSeparator;
lpCellType->Spec.Number.fSpin = fSpin;
lpCellType->Spec.Number.fSpinWrap = fSpinWrap;
lpCellType->Spec.Number.SpinInc = SpinInc;

return (lpCellType);
}


LPSS_CELLTYPE DLLENTRY SSSetTypePercent(HWND hWnd, LPSS_CELLTYPE lpCellType, long Style,
                          BYTE Right, double Min, double Max)
{
return SSSetTypePercentEx(hWnd, lpCellType, Style, Right, Min, Max, NULL, 0, 0, FALSE, FALSE, 1.1);
}


LPSS_CELLTYPE DLLENTRY SSSetTypePercentEx(HWND hWnd, LPSS_CELLTYPE lpCellType, long Style,
                          BYTE Right, double Min, double Max, LPTSTR lpszDecimal, BYTE bLeadingZero,
                          BYTE bNegStyle, BYTE fSpin, BYTE fSpinWrap, double SpinInc)
{
LPSPREADSHEET lpSS;
LPSS_CELLTYPE Ret;

lpSS = SS_SheetLock(hWnd);
Ret = SS_SetTypePercent(lpSS, lpCellType, Style, Right, Min, Max, lpszDecimal, bLeadingZero, bNegStyle,
                        fSpin, fSpinWrap, SpinInc);
SS_SheetUnlock(hWnd);
return Ret;
}


LPSS_CELLTYPE SS_SetTypePercent(LPSPREADSHEET lpSS, LPSS_CELLTYPE lpCellType, long Style,
                 BYTE Right, double Min, double Max, LPTSTR lpszDecimal, BYTE bLeadingZero, BYTE bNegStyle,
                 BYTE fSpin, BYTE fSpinWrap, double SpinInc)
{
_fmemset(lpCellType, '\0', sizeof(SS_CELLTYPE));

lpCellType->Type = SS_TYPE_PERCENT;
lpCellType->Style = SS_TranslateEditStyle(Style);
lpCellType->Spec.Percent.Min = Min;
lpCellType->Spec.Percent.Max = Max;
if (lpszDecimal)
	lstrcpyn(lpCellType->Spec.Percent.szDecimal, lpszDecimal, 4);
lpCellType->Spec.Percent.Right = min(14, Right);
lpCellType->Spec.Percent.bLeadingZero = bLeadingZero;
lpCellType->Spec.Percent.bNegStyle = bNegStyle;
lpCellType->Spec.Percent.fSpin = fSpin;
lpCellType->Spec.Percent.fSpinWrap = fSpinWrap;
lpCellType->Spec.Percent.SpinInc = SpinInc;

return (lpCellType);
}
#endif // SS_V40


#ifdef SS_V70

LPSS_CELLTYPE DLLENTRY SSSetTypeScientific(HWND hWnd, LPSS_CELLTYPE lpCellType, long Style,
                          BYTE Right, double Min, double Max, LPTSTR lpszDecimal)
{
LPSPREADSHEET lpSS;
LPSS_CELLTYPE Ret;

lpSS = SS_SheetLock(hWnd);

Ret = SS_SetTypeScientific(lpSS, lpCellType, Style, Right, Min, Max, lpszDecimal);
SS_SheetUnlock(hWnd);
return Ret;
}


LPSS_CELLTYPE SS_SetTypeScientific(LPSPREADSHEET lpSS, LPSS_CELLTYPE lpCellType, long Style,
                 BYTE Right, double Min, double Max, LPTSTR lpszDecimal)
{
_fmemset(lpCellType, '\0', sizeof(SS_CELLTYPE));

lpCellType->Type = SS_TYPE_SCIENTIFIC;
lpCellType->Style = SS_TranslateEditStyle(Style);
lpCellType->Spec.Scientific.Min = Min;
lpCellType->Spec.Scientific.Max = Max;
if (lpszDecimal)
	lstrcpyn(lpCellType->Spec.Scientific.szDecimal, lpszDecimal, 4);
lpCellType->Spec.Scientific.Right = min(14, Right);

return (lpCellType);
}
#endif // SS_V70


#if 0
LPSS_CELLTYPE DLLENTRY SSSetTypeCustom(hWnd, CellType, Style, ClassName)

HWND          hWnd;
LPSS_CELLTYPE CellType;
long          Style;
LPTSTR        ClassName;
{
LPTSTR        lpClassNameTemp;

_fmemset(CellType, '\0', sizeof(SS_CELLTYPE));

CellType->Type  = SS_TYPE_CUSTOM;
CellType->Style = Style;

if (!(CellType->Spec.Custom.hClassName = tbGlobalAlloc(GMEM_MOVEABLE |
                                                      GMEM_ZEROINIT,
                                                      (lstrlen(ClassName) + 1)
                                                      * sizeof(TCHAR))))
   return (NULL);

lpClassNameTemp = (LPTSTR)tbGlobalLock(CellType->Spec.Custom.hClassName);
lstrcpy(lpClassNameTemp, ClassName);
tbGlobalUnlock(CellType->Spec.Custom.hClassName);

return (CellType);
}
#endif


LPSS_CELLTYPE DLLENTRY SSSetTypePicture(HWND hWnd, LPSS_CELLTYPE CellType,
                                        long Style, LPCTSTR PictName)
{
LPSPREADSHEET lpSS;
LPSS_CELLTYPE Ret;

lpSS = SS_SheetLock(hWnd);
Ret = SS_SetTypePicture(lpSS, CellType, Style, PictName);
SS_SheetUnlock(hWnd);
return Ret;
}


LPSS_CELLTYPE SS_SetTypePicture(LPSPREADSHEET lpSS, LPSS_CELLTYPE CellType,
                                long Style, LPCTSTR PictName)
{
_fmemset(CellType, '\0', sizeof(SS_CELLTYPE));

CellType->Type  = SS_TYPE_PICTURE;
CellType->Style = Style;

if (Style & VPS_HANDLE)
   {
   if (PictName)
      CellType->Spec.ViewPict.hPictName = (TBGLOBALHANDLE)(*(HANDLE FAR *)PictName);
   }
else if (PictName)
   {
   // BJO 23Sep96 JAP5457 - Before fix
   //CellType->Spec.ViewPict.hPictName = (TBGLOBALHANDLE)SS_LoadPicture(lpSS->lpBook->hWnd,
   //   PictName, Style, &CellType->Spec.ViewPict.hPal);
   // BJO 23Sep96 JAP5457 - Begin fix
   CellType->Spec.ViewPict.hPictName = (TBGLOBALHANDLE)SS_LoadPicture(lpSS->lpBook->hWnd,
      PictName, Style, NULL);
   // BJO 23Sep96 JAP5457 - End fix
   CellType->Spec.ViewPict.fDeleteHandle = TRUE;
   CellType->Style = Style | VPS_HANDLE;
   SS_PicRef(lpSS->lpBook, (HANDLE)CellType->Spec.ViewPict.hPictName);
   }
//   if (!(CellType->Spec.ViewPict.hPictName =
//         SS_tbCreateTextHandle(PictName, 0)))
//      return (NULL);

return (CellType);
}


LPSS_CELLTYPE DLLENTRY SSSetTypePictureHandle(HWND hWnd,
                                              LPSS_CELLTYPE CellType,
                                              long Style, HANDLE hPict,
                                              HPALETTE hPal,
                                              BOOL fDeleteHandle)
{
LPSPREADSHEET lpSS;
LPSS_CELLTYPE Ret;

lpSS = SS_SheetLock(hWnd);
Ret = SS_SetTypePictureHandle(lpSS, CellType, Style, hPict, hPal,
                              fDeleteHandle);
SS_SheetUnlock(hWnd);
return Ret;
}


LPSS_CELLTYPE SS_SetTypePictureHandle(LPSPREADSHEET lpSS,
                                      LPSS_CELLTYPE CellType,
                                      long Style, HANDLE hPict,
                                      HPALETTE hPal, BOOL fDeleteHandle)
{
_fmemset(CellType, '\0', sizeof(SS_CELLTYPE));

CellType->Type  = SS_TYPE_PICTURE;
CellType->Style = Style | VPS_HANDLE;

CellType->Spec.ViewPict.hPictName = (TBGLOBALHANDLE)hPict;
CellType->Spec.ViewPict.hPal = hPal;
CellType->Spec.ViewPict.fDeleteHandle = fDeleteHandle;
if (fDeleteHandle)
   SS_PicRef(lpSS->lpBook, hPict);

return (CellType);
}


LPSS_CELLTYPE SS_RetrieveCellType(LPSPREADSHEET lpSS, LPSS_CELLTYPE CellType,
                                  LPSS_CELL Cell, SS_COORD CellCol, SS_COORD CellRow)
{
LPSS_COL lpCol;
LPSS_ROW lpRow;

lpCol = SS_LockColItem(lpSS, CellCol);
lpRow = SS_LockRowItem(lpSS, CellRow);

SSx_RetrieveCellType(lpSS, CellType, lpCol, lpRow, Cell, CellCol,
                     CellRow);

SS_UnlockColItem(lpSS, CellCol);
SS_UnlockRowItem(lpSS, CellRow);

return (CellType);
}


LPSS_CELLTYPE SSx_RetrieveCellType(LPSPREADSHEET lpSS, LPSS_CELLTYPE CellType,
                                   LPSS_COL lpCol, LPSS_ROW lpRow, LPSS_CELL Cell,
                                   SS_COORD CellCol, SS_COORD CellRow)
{
LPSS_CELLTYPE CellTypeTemp;
BOOL          fCellLock = FALSE;

if (!Cell && CellCol != SS_ALLCOLS && CellRow != SS_ALLROWS)
   {
   if (Cell = SSx_LockCellItem(lpSS, lpRow, CellCol, CellRow))
      fCellLock = TRUE;
   }

if (Cell && Cell->hCellType && CellCol != SS_ALLCOLS && CellRow != SS_ALLROWS)
   {
   CellTypeTemp = (LPSS_CELLTYPE)tbGlobalLock(Cell->hCellType);
   _fmemcpy(CellType, CellTypeTemp, sizeof(SS_CELLTYPE));
   tbGlobalUnlock(Cell->hCellType);
   }

else if (CellRow != SS_ALLROWS && lpRow && CellRow < lpSS->Row.HeaderCnt &&
         lpRow->hCellType)
   {
   CellTypeTemp = (LPSS_CELLTYPE)tbGlobalLock(lpRow->hCellType);
   _fmemcpy(CellType, CellTypeTemp, sizeof(SS_CELLTYPE));
   tbGlobalUnlock(lpRow->hCellType);
   }

else if (CellCol != SS_ALLCOLS && lpCol && lpCol->hCellType)
   {
   CellTypeTemp = (LPSS_CELLTYPE)tbGlobalLock(lpCol->hCellType);
   _fmemcpy(CellType, CellTypeTemp, sizeof(SS_CELLTYPE));
   tbGlobalUnlock(lpCol->hCellType);
   }

else if (CellRow != SS_ALLROWS && lpRow && lpRow->hCellType)
   {
   CellTypeTemp = (LPSS_CELLTYPE)tbGlobalLock(lpRow->hCellType);
   _fmemcpy(CellType, CellTypeTemp, sizeof(SS_CELLTYPE));
   tbGlobalUnlock(lpRow->hCellType);
   }

else
   _fmemcpy(CellType, &lpSS->DefaultCellType, sizeof(SS_CELLTYPE));

if (fCellLock)
   SSx_UnlockCellItem(lpSS, lpRow, CellCol, CellRow);

return (CellType);
}


LPSS_CELLTYPE SS_SetCellTypeStruct(lpSS, CellType, Cell, CellCol, CellRow)

LPSPREADSHEET lpSS;
LPSS_CELLTYPE CellType;
LPSS_CELL     Cell;
SS_COORD      CellCol;
SS_COORD      CellRow;
{
LPSS_COL      lpCol;
LPSS_ROW      lpRow;

lpCol = SS_LockColItem(lpSS, CellCol);
lpRow = SS_LockRowItem(lpSS, CellRow);

SSx_SetCellTypeStruct(lpSS, CellType, lpCol, lpRow, Cell, CellCol, CellRow);

SS_UnlockColItem(lpSS, CellCol);
SS_UnlockRowItem(lpSS, CellRow);

return (CellType);
}


LPSS_CELLTYPE SSx_SetCellTypeStruct(lpSS, CellType, lpCol, lpRow, Cell,
                               CellCol, CellRow)

LPSPREADSHEET lpSS;
LPSS_CELLTYPE CellType;
LPSS_COL      lpCol;
LPSS_ROW      lpRow;
LPSS_CELL     Cell;
SS_COORD      CellCol;
SS_COORD      CellRow;
{
LPSS_CELL     lpCellOrig = Cell;
LPSS_CELLTYPE CellTypeTemp;

if (!Cell && CellCol != SS_ALLCOLS && CellRow != SS_ALLROWS)
   Cell = SS_LockCellItem(lpSS, CellCol, CellRow);

if (Cell && Cell->hCellType && CellCol != SS_ALLCOLS && CellRow != SS_ALLROWS)
   {
   CellTypeTemp = (LPSS_CELLTYPE)tbGlobalLock(Cell->hCellType);
   _fmemcpy(CellTypeTemp, CellType, sizeof(SS_CELLTYPE));
   tbGlobalUnlock(Cell->hCellType);
   }

else if (CellRow != SS_ALLROWS && lpRow && CellRow < lpSS->Row.HeaderCnt &&
         lpRow->hCellType)
   {
   CellTypeTemp = (LPSS_CELLTYPE)tbGlobalLock(lpRow->hCellType);
   _fmemcpy(CellTypeTemp, CellType, sizeof(SS_CELLTYPE));
   tbGlobalUnlock(lpRow->hCellType);
   }

else if (CellCol != SS_ALLCOLS && lpCol && lpCol->hCellType)
   {
   CellTypeTemp = (LPSS_CELLTYPE)tbGlobalLock(lpCol->hCellType);
   _fmemcpy(CellTypeTemp, CellType, sizeof(SS_CELLTYPE));
   tbGlobalUnlock(lpCol->hCellType);
   }

else if (CellRow != SS_ALLROWS && lpRow && lpRow->hCellType)
   {
   CellTypeTemp = (LPSS_CELLTYPE)tbGlobalLock(lpRow->hCellType);
   _fmemcpy(CellTypeTemp, CellType, sizeof(SS_CELLTYPE));
   tbGlobalUnlock(lpRow->hCellType);
   }

else
   _fmemcpy(&lpSS->DefaultCellType, CellType, sizeof(SS_CELLTYPE));

if (Cell && !lpCellOrig)
   SS_UnlockCellItem(lpSS, CellCol, CellRow);

return (CellType);
}


short SS_CreateControl(lpSS, CellType, fRenderer)
LPSPREADSHEET lpSS;
LPSS_CELLTYPE CellType;
BOOL          fRenderer;
{
LPSS_CONTROL  Controls;
HWND          hWndCtrl = NULL;
HWND          hWndParent;
LPTSTR        lpText;
long          lStyle;
short         CtrlID = 0;
short         i;
BOOL          fRet;
BOOL          fCreateAlways = FALSE;
#if SS_V80
CT_HANDLE     hCT = 0;
#if defined(_WIN64) || defined(_IA64)
LONG_PTR      wndProc = 0;
#else
long          wndProc = 0;
#endif
long          ldisp = 0;
BOOL          fDestroy = FALSE;

if( CellType->Type == SS_TYPE_CUSTOM )
{
   LPTSTR lpszName = SS_CT_LockName(CellType);
   hCT = CustCellTypeLookup(&lpSS->lpBook->CustCellTypes, lpszName);
   SS_CT_UnlockName(CellType);
}
else
  fRenderer = FALSE;
#endif

if (!lpSS->lpBook->hWnd)  // handle an OCX control without a HWND 
   return 0;

if (!fCreateAlways && CellType->Type != SS_TYPE_PICTURE)
   {
   if (lpSS->lpBook->hControls)
      {
      Controls = (LPSS_CONTROL)tbGlobalLock(lpSS->lpBook->hControls);
      for (i = 0; i < lpSS->lpBook->ControlsCnt; i++)
         if (Controls[i].Type == CellType->Type)
            {
            if ((CellType->Type == SS_TYPE_EDIT ||
                 CellType->Type == SS_TYPE_PIC ||
                 CellType->Type == SS_TYPE_FLOAT ||
                 CellType->Type == SS_TYPE_INTEGER ||
                 CellType->Type == SS_TYPE_BUTTON ||
#ifdef SS_V40
                 CellType->Type == SS_TYPE_CURRENCY ||
                 CellType->Type == SS_TYPE_NUMBER ||
                 CellType->Type == SS_TYPE_PERCENT ||
#endif // SS_V40
#ifdef SS_V70
                 CellType->Type == SS_TYPE_SCIENTIFIC ||
#endif // SS_V70
#ifdef SS_V80
                 CellType->Type == SS_TYPE_CUSTOM ||
#endif
                 CellType->Type == SS_TYPE_CHECKBOX) &&
                 Controls[i].Style != CellType->Style)
               continue;
#ifdef SS_V80
            if( CellType->Type == SS_TYPE_CUSTOM )
            {
               if( (hCT != Controls[i].hCT) || (fRenderer != Controls[i].fRenderer) )
                  continue;
            }
#endif
            CtrlID = Controls[i].CtrlID;
            break;
            }

      tbGlobalUnlock(lpSS->lpBook->hControls);

      if (CtrlID)
         {
#if SS_V80
         if( fRenderer && CellType->Type == SS_TYPE_CUSTOM )
         {
            CellType->Spec.Custom.RendererControlID = CtrlID;
            SS_CT_RefRenderer(lpSS, CellType);
         }
         else
         {
            CellType->ControlID = CtrlID;
            SS_CT_RefEditor(lpSS, CellType);
         }
#else
         CellType->ControlID = CtrlID;
#endif
         return (CtrlID);
         }
      }
   }

CtrlID = SS_CONTROL_BASEID + lpSS->lpBook->ControlsCnt;

#ifndef SS_V80 // recycle control array entries in v8 -scl
if (fCreateAlways)
#endif
   {
   Controls = (LPSS_CONTROL)tbGlobalLock(lpSS->lpBook->hControls);
   for (i = 0; i < lpSS->lpBook->ControlsCnt; i++)
      if (!Controls[i].CtrlID)
         {
         CtrlID = SS_CONTROL_BASEID + i;
         break;
         }

   tbGlobalUnlock(lpSS->lpBook->hControls);
   }

//CellType->Style &= ~WS_VISIBLE;

hWndParent = lpSS->lpBook->hWnd;

switch (CellType->Type)
   {
   case SS_TYPE_EDIT:
      if ((CellType->Style & ES_AUTOHSCROLL) ||
          (CellType->Style & ES_MULTILINE))
			{
			long   lStyle = CellType->Style;

			lStyle &= ~ES_LEFTALIGN;
			lStyle &= 0xffff;

			SS_RegisterEditHScroll(hDynamicInst);
         hWndCtrl = CreateWindow(SSClassNames[dClassNameIndex].SSEditHScroll,
                                 NULL, WS_CHILD | (DWORD)(WORD)lStyle, 0, 0, 0, 0,
                                 hWndParent, (HMENU)CtrlID, hDynamicInst, NULL);
			}
      else
         hWndCtrl = CreateWindow(SSClassNames[dClassNameIndex].SSEdit, _T(""),
                                 WS_CHILD | (DWORD)(WORD)CellType->Style, 0, 0, 10, 10,
                                 hWndParent, (HMENU)CtrlID, hDynamicInst, NULL);

#ifdef SS_USEAWARE
      SendMessage(hWndCtrl, EM_SETEXTSTYLE, TRUE, ESX_AUTOADVANCE);
      SendMessage(hWndCtrl, EM_SETEXTSTYLE, TRUE, ESX_AUTOBEEP);
#endif
      break;

#ifndef SS_NOCT_PIC
   case SS_TYPE_PIC:
      lStyle = CellType->Style & ~(ES_CENTER | ES_RIGHT);
      hWndCtrl = CreateWindow(SSClassNames[dClassNameIndex].SSPic, _T(""),
                              WS_CHILD | (long)(WORD)lStyle, 0, 0, 10, 10,
                              hWndParent, (HMENU)CtrlID, hDynamicInst, NULL);

#ifdef SS_USEAWARE
      SendMessage(hWndCtrl, EM_SETEXTSTYLE, TRUE, ESX_AUTOBEEP);
#endif
      break;
#endif

#ifndef SS_NOCT_INT
   case SS_TYPE_INTEGER:
		{
		TCHAR Buffer[330];

      wsprintf(Buffer, _T("%ld"), CellType->Spec.Integer.Max);

      for (i = 0; i < lstrlen(Buffer); i++)
         Buffer[i] = '9';

      lStyle = CellType->Style & ~(ES_CENTER | ES_RIGHT);
	  // BUG INT_CEL_001_020 & SPR_CEL_004_005 (3-1)
  	  // Add WS_CLIPCHILDREN Style To Prevent Caret Drawn On Spin Button
	  // Modified By HaHa 1999.11.26
      hWndCtrl = CreateWindow(SSClassNames[dClassNameIndex].SSInteger, Buffer,
                              WS_CLIPCHILDREN | WS_CHILD | (long)(WORD)lStyle, 0, 0, 65, 10,
                              hWndParent, (HMENU)CtrlID, hDynamicInst, NULL);

      IntSetRange(hWndCtrl, CellType->Spec.Integer.Min,
                  CellType->Spec.Integer.Max);
      /*
      SendMessage(hWndCtrl, EM_SETEXTSTYLE, TRUE, ESX_NOFORMATTEDEDIT);
      SendMessage(hWndCtrl, EM_SETEXTSTYLE, TRUE, ESX_AUTOADVANCE);
      SendMessage(hWndCtrl, EM_SETEXTSTYLE, TRUE, ESX_NULLSUPPORT);
      */
		}
      break;
#endif

   case SS_TYPE_FLOAT:
		{
		TCHAR Buffer[330];

      _fmemset(Buffer, '\0', sizeof(Buffer));

      for (i = 0; i < CellType->Spec.Float.Left; i++)
         _ftcscat(Buffer, _T("9"));

      _ftcscat(Buffer, _T("."));

      for (i = 0; i < CellType->Spec.Float.Right; i++)
         _ftcscat(Buffer, _T("9"));

      lStyle = CellType->Style & ~(ES_CENTER | ES_RIGHT);
      hWndCtrl = CreateWindow(SSClassNames[dClassNameIndex].SSFloat, Buffer,
                              WS_CHILD | (long)(WORD)lStyle, 0, 0, 65, 10,
                              hWndParent, (HMENU)CtrlID, hDynamicInst, NULL);

      FloatSetRange(hWndCtrl, CellType->Spec.Float.Min,
                    CellType->Spec.Float.Max);

#ifdef SS_USEAWARE
      SendMessage(hWndCtrl, EM_SETEXTSTYLE, TRUE, ESX_NOFORMATTEDEDIT);
      SendMessage(hWndCtrl, EM_SETEXTSTYLE, TRUE, ESX_AUTOADVANCE);
      SendMessage(hWndCtrl, EM_SETEXTSTYLE, TRUE, ESX_NULLSUPPORT);
      SendMessage(hWndCtrl, EM_SETEXTSTYLE, TRUE, DS_SEPARATOR);
      SendMessage(hWndCtrl, EM_SETEXTSTYLE, TRUE, ESX_AUTOBEEP);
#endif
		}
      break;

#ifdef SS_V40
   case SS_TYPE_CURRENCY:
      lStyle = CellType->Style & ~(ES_CENTER | ES_RIGHT);
      hWndCtrl = CreateWindow(SSClassNames[dClassNameIndex].SSCurrency, NULL,
                              WS_CHILD | (long)(WORD)lStyle, 0, 0, 65, 10,
                              hWndParent, (HMENU)CtrlID, hDynamicInst, NULL);
      break;

   case SS_TYPE_NUMBER:
      lStyle = CellType->Style & ~(ES_CENTER | ES_RIGHT);
      hWndCtrl = CreateWindow(SSClassNames[dClassNameIndex].SSNumber, NULL,
                              WS_CHILD | (long)(WORD)lStyle, 0, 0, 65, 10,
                              hWndParent, (HMENU)CtrlID, hDynamicInst, NULL);
      break;

   case SS_TYPE_PERCENT:
      lStyle = CellType->Style & ~(ES_CENTER | ES_RIGHT);
      hWndCtrl = CreateWindow(SSClassNames[dClassNameIndex].SSPercent, NULL,
                              WS_CHILD | (long)(WORD)lStyle, 0, 0, 65, 10,
                              hWndParent, (HMENU)CtrlID, hDynamicInst, NULL);
      break;
#endif // SS_V40
#ifdef SS_V70
   case SS_TYPE_SCIENTIFIC:
      lStyle = CellType->Style & ~(ES_CENTER | ES_RIGHT);
		lStyle |= ES_RIGHT;
      hWndCtrl = CreateWindow(SSClassNames[dClassNameIndex].SSScientific, NULL,
                              WS_CHILD | ES_AUTOHSCROLL | (long)(WORD)lStyle, 0, 0, 65, 10,
                              hWndParent, (HMENU)CtrlID, hDynamicInst, NULL);
      break;
#endif // SS_V70

#ifndef SS_NOCT_COMBO
   case SS_TYPE_COMBOBOX:
      hWndCtrl = CreateWindow(SSClassNames[dClassNameIndex].SSComboBox,
                              NULL, WS_CHILD, 0, 0, 10, 10, hWndParent,
                              (HMENU)CtrlID, hDynamicInst, NULL);

      break;
#endif

#ifndef SS_NOCT_TIME
   case SS_TYPE_TIME:
      lStyle = CellType->Style & ~(ES_CENTER | ES_RIGHT);
	  // BUG INT_CEL_001_020 & SPR_CEL_004_005 (3-2)
  	  // Add WS_CLIPCHILDREN Style To Prevent Caret Drawn On Spin Button
	  // Modified By HaHa 1999.11.26
      hWndCtrl = CreateWindow(SSClassNames[dClassNameIndex].SSTime, _T(""),
                              WS_CLIPCHILDREN | WS_CHILD | (long)(WORD)lStyle, 0, 0, 10, 10,
                              hWndParent, (HMENU)CtrlID, hDynamicInst, NULL);

      TimeGetFormat(hWndCtrl, &lpSS->lpBook->DefaultTimeFormat);
      break;
#endif

#ifndef SS_NOCT_DATE
   case SS_TYPE_DATE:
      lStyle = CellType->Style & ~(ES_CENTER | ES_RIGHT);
	  // BUG INT_CEL_001_020 & SPR_CEL_004_005 (3-3)
  	  // Add WS_CLIPCHILDREN Style To Prevent Caret Drawn On Spin Button
	  // Modified By HaHa 1999.11.26
      hWndCtrl = CreateWindow(SSClassNames[dClassNameIndex].SSDate, _T(""),
                              WS_CLIPCHILDREN | WS_CHILD | (long)(WORD)lStyle, 0, 0, 10, 10,
                              hWndParent, (HMENU)CtrlID, hDynamicInst, NULL);

      DateGetFormat(hWndCtrl, &lpSS->lpBook->DefaultDateFormat);
#ifdef SS_USEAWARE
      SendMessage(hWndCtrl, EM_SETEXTSTYLE, TRUE, ESX_NOFORMATTEDEDIT);
      SendMessage(hWndCtrl, EM_SETEXTSTYLE, TRUE, ESX_AUTOADVANCE);
      SendMessage(hWndCtrl, EM_SETEXTSTYLE, TRUE, ESX_NULLSUPPORT);
      SendMessage(hWndCtrl, EM_SETEXTSTYLE, TRUE, ESX_AUTOBEEP);
#endif
      break;
#endif

#ifdef SS_V80
   case SS_TYPE_CUSTOM:
     if( fRenderer )
     {
        if( !SS_CT_CreateRendererControl(lpSS, CellType, CtrlID, &hWndCtrl) )
        {
          LPTSTR lpszName = SS_CT_LockName(CellType);
          hWndCtrl = CreateWindowEx(0, lpszName, _T(""),
               WS_CHILD | (long)(WORD)CellType->Style & ~WS_VISIBLE, 0, 0, 0, 0,
               hWndParent, (HMENU)CtrlID, hDynamicInst, NULL);
          fDestroy = TRUE;
          SS_CT_UnlockName(CellType);
        }
     }
     else
     {
        if( !SS_CT_CreateEditorControl(lpSS, CellType, CtrlID, &hWndCtrl) )
        {
          LPTSTR lpszName = SS_CT_LockName(CellType);
          hWndCtrl = CreateWindowEx(0, lpszName, _T(""),
               WS_CHILD | (long)(WORD)CellType->Style & ~WS_VISIBLE, 0, 0, 0, 0,
               hWndParent, (HMENU)CtrlID, hDynamicInst, NULL);
          fDestroy = TRUE;
          SS_CT_UnlockName(CellType);
        }
     }
#if SS_OCX
     if( IsWindow(hWndCtrl) )
     {
        ldisp = (long)CT_GetIDispatch(hWndCtrl);
        if( ldisp )
           RemoveProp(hWndCtrl, PDISPATCH);
     }
#endif
     break;
#endif

#if 0
   case SS_TYPE_CUSTOM:
      ClassName = (LPTSTR)tbGlobalLock(CellType->Spec.Custom.hClassName);

      hWndCtrl = CreateWindow(ClassName, "", WS_CHILD | CellType->Style,
                              0, 0, 10, 10, hWnd, (HMENU)CtrlID,
                              hDynamicInst, NULL);

      tbGlobalUnlock(CellType->Spec.Custom.hClassName);
      break;
#endif

#if 0
   case SS_TYPE_PICTURE:

      if (CellType->Style & VPS_HANDLE)
         {
         hWndCtrl = CreateWindow(SSClassNames[dClassNameIndex].TBViewPict,
                                 NULL, WS_CHILD | CellType->Style, 0, 0, 0,
                                 0, hWndParent, (HMENU)CtrlID,
                                 GetWindowInstance(lpSS->lpBook->hWnd), NULL);

         SendMessage(hWndCtrl, VPM_SETPICTURE, (WORD)CellType->Style,
                     (long)CellType->Spec.ViewPict.hPictName);

         if (CellType->Spec.ViewPict.hPal)
            SendMessage(hWndCtrl, VPM_SETPALETTE, 0,
                        (long)CellType->Spec.ViewPict.hPal);
         }
      else
         {
         PictName = (LPTSTR)tbGlobalLock(CellType->Spec.ViewPict.hPictName);

         hWndCtrl = CreateWindow(SSClassNames[dClassNameIndex].TBViewPict,
                                 PictName, WS_CHILD | CellType->Style, 0, 0,
                                 0, 0, hWndParent, (HMENU)CtrlID,
                                 GetWindowInstance(lpSS->lpBook->hWnd), NULL);

         tbGlobalUnlock(CellType->Spec.ViewPict.hPictName);
         }

      break;
#endif

#ifndef SS_NOCT_BUTTON
   case SS_TYPE_BUTTON:
      lpText = NULL;

      if (CellType->Spec.Button.hText)
         lpText = (LPTSTR)GlobalLock(CellType->Spec.Button.hText);

      hWndCtrl = CreateWindow(SSClassNames[dClassNameIndex].SSSuperBtn, lpText,
                              WS_CHILD | (long)(WORD)CellType->Style, 0, 0, 0, 0,
                              hWndParent, (HMENU)CtrlID, hDynamicInst, NULL);

      if (CellType->Spec.Button.hText)
         GlobalUnlock(CellType->Spec.Button.hText);

      break;
#endif

#ifndef SS_NOCT_CHECKBOX
   case SS_TYPE_CHECKBOX:
      lpText = NULL;

      if (CellType->Spec.CheckBox.hText)
         lpText = (LPTSTR)GlobalLock(CellType->Spec.CheckBox.hText);

      hWndCtrl = CreateWindow(SSClassNames[dClassNameIndex].SSCheckBox, lpText,
                              WS_CHILD | (long)(WORD)CellType->Style, 0, 0, 0, 0,
                              hWndParent, (HMENU)CtrlID, hDynamicInst, NULL);

      if (CellType->Spec.CheckBox.hText)
         GlobalUnlock(CellType->Spec.CheckBox.hText);

      break;
#endif

   }

if (!hWndCtrl)
   CtrlID = 0;

else
   {
#if SS_V80
   fRet = SS_TypeControlAlloc(&lpSS->lpBook->hControls,
                              &lpSS->lpBook->ControlsAllocCnt,
                              &lpSS->lpBook->ControlsCnt, hWndCtrl,
                              CellType->Type, CtrlID, CellType->Style, hCT, fRenderer, ldisp, fDestroy);
#else
   fRet = SS_TypeControlAlloc(&lpSS->lpBook->hControls,
                              &lpSS->lpBook->ControlsAllocCnt,
                              &lpSS->lpBook->ControlsCnt, hWndCtrl,
                              CellType->Type, CtrlID, CellType->Style);
#endif

   if (!fRet)
      return 0;
   }
#if SS_V80
if( fRenderer && CellType->Type == SS_TYPE_CUSTOM )
   CellType->Spec.Custom.RendererControlID = CtrlID;
else
#endif
   CellType->ControlID = CtrlID;

#if SS_V80
if( fRenderer )
   SS_CT_RefRenderer(lpSS, CellType);
else
   SS_CT_RefEditor(lpSS, CellType);
#endif
return CtrlID;
}


#if SS_V80
BOOL SS_TypeControlAlloc(LPTBGLOBALHANDLE lphControls,
                         LPSHORT lpControlsAllocCnt, LPSHORT lpControlsCnt,
                         HWND hWndCtrl, short Type, short CtrlID, long lStyle, CT_HANDLE hCT, BOOL fRenderer, long ldisp, BOOL fDestroy)
#else
BOOL SS_TypeControlAlloc(LPTBGLOBALHANDLE lphControls,
                         LPSHORT lpControlsAllocCnt, LPSHORT lpControlsCnt,
                         HWND hWndCtrl, short Type, short CtrlID, long lStyle)
#endif
{
LPSS_CONTROL lpControls;
short        dIndex;

dIndex = CtrlID - SS_CONTROL_BASEID;

if (dIndex + 1 > *lpControlsAllocCnt)
   {
   if (*lpControlsAllocCnt == 0)
      *lphControls = tbGlobalAlloc(GMEM_MOVEABLE | GMEM_ZEROINIT,
                                   (long)(sizeof(SS_CONTROL) *
                                   (*lpControlsAllocCnt + SS_TYPE_ALLOC_CNT)));
   else
      *lphControls = tbGlobalReAlloc(*lphControls, (long)(sizeof(SS_CONTROL) *
                                     (*lpControlsAllocCnt +
                                     SS_TYPE_ALLOC_CNT)),
                                     GMEM_MOVEABLE | GMEM_ZEROINIT);

   if (!(*lphControls))
      return (FALSE);

   *lpControlsAllocCnt += SS_TYPE_ALLOC_CNT;
   }

lpControls = (LPSS_CONTROL)tbGlobalLock(*lphControls);

lpControls[dIndex].hWnd = hWndCtrl;
lpControls[dIndex].Type = (BYTE)Type;
lpControls[dIndex].CtrlID = CtrlID;
lpControls[dIndex].Style = lStyle;
#if SS_V80
lpControls[dIndex].hCT = hCT;
lpControls[dIndex].fRenderer = fRenderer;
lpControls[dIndex].ldisp = ldisp;
lpControls[dIndex].fDestroy = fDestroy;
#endif

if (dIndex + 1 > *lpControlsCnt)
   *lpControlsCnt = dIndex + 1;

tbGlobalUnlock(*lphControls);
return (TRUE);
}

#if SS_V80
BOOL SS_TypeControlRemove(CT_HANDLE hCT, LPTBGLOBALHANDLE lphControls,
                         LPSHORT lpControlsAllocCnt, LPSHORT lpControlsCnt)
{
BOOL         Ret = FALSE;
LPSS_CONTROL lpControls;
short        dIndex;

lpControls = (LPSS_CONTROL)tbGlobalLock(*lphControls);
for( dIndex = 0; dIndex < *lpControlsCnt; dIndex++ )
{
   if( lpControls[dIndex].hCT == hCT )
   {
#if SS_OCX
      if( lpControls[dIndex].ldisp != 0 )
         fpDispatchRelease(lpControls[dIndex].ldisp);
#endif
      if( lpControls[dIndex].wndProc != NULL )
#if defined(_WIN64) || defined(_IA4)
         SetWindowLongPtr(lpControls[dIndex].hWnd, GWLP_WNDPROC, (LONG_PTR)lpControls[dIndex].wndProc);
#else
         SetWindowLong(lpControls[dIndex].hWnd, GWL_WNDPROC, (long)lpControls[dIndex].wndProc);
#endif
      if( lpControls[dIndex].fDestroy && IsWindow(lpControls[dIndex].hWnd) )
         DestroyWindow(lpControls[dIndex].hWnd);
      memset(&lpControls[dIndex], 0, sizeof(SS_CONTROL));
      Ret = TRUE;
   }
}
tbGlobalUnlock(*lphControls);
return Ret;
}
#endif

BOOL SS_FormatData(lpSS, lpData, Col, Row, lpCellType, lpszText, fValue, fCheckRange, fCheckLen)

LPSPREADSHEET  lpSS;
LPSS_DATA      lpData;
SS_COORD       Col;
SS_COORD       Row;
LPSS_CELLTYPE  lpCellType;
LPCTSTR        lpszText;
BOOL           fValue;
BOOL           fCheckRange;
BOOL           fCheckLen;
{
TBGLOBALHANDLE hText;
LPTSTR         lpszTextTemp;
TCHAR          szTextFmtBuffer[SS_MAXDATALEN];
LPTSTR         lpszTextFmt;
BOOL           Ret = FALSE;
double         dfValue;

lpszTextFmt = szTextFmtBuffer;
*lpszTextFmt = '\0';

switch (lpCellType->Type)
   {
   case SS_TYPE_PIC:
#ifndef SS_NOCT_PIC
      Ret = SS_FormatDataPic(lpCellType, lpszText, lpszTextFmt, fValue);

		// RFW - 8/21/07 - 20793, 20796
		if (!Ret)
			{
			lpData->bDataType = 0;
#ifndef SS_NOOVERFLOW
			lpData->bOverflow = 0;
#endif
			lpData->Data.hszData = 0;
			return (TRUE);
	      }

#endif
      break;

   case SS_TYPE_INTEGER:
#ifndef SS_NOCT_INT
      {
      long lValue;

      if (fValue == SS_VALUE_INT)
         {
         lpData->bDataType = SS_TYPE_INTEGER;
         lpData->Data.lValue = *(LPLONG)lpszText;
         }

      else if (SS_FormatDataInteger(lpCellType, lpszText, lpszTextFmt,
                                    &lValue, fValue, fCheckRange))
         {
         lpData->bDataType = SS_TYPE_INTEGER;
         lpData->Data.lValue = lValue;
         }

      else
         {
         lpData->bDataType = 0;
         lpData->Data.hszData = 0;
         }
      }
#endif

      return (TRUE);

   case SS_TYPE_FLOAT:
#ifdef SS_V40
   case SS_TYPE_CURRENCY:
   case SS_TYPE_NUMBER:
   case SS_TYPE_PERCENT:
#endif // SS_V40
#ifdef SS_V70
	case SS_TYPE_SCIENTIFIC:
#endif // SS_V70
      if (fValue == SS_VALUE_FLOAT)
         {
         lpData->bDataType = SS_TYPE_FLOAT;
         lpData->Data.dfValue = *(LPDOUBLE)lpszText;
//#ifdef SS_V40
//			if (lpCellType->Type == SS_TYPE_PERCENT)
//				lpData->Data.dfValue /= 100.0;
//#endif // SS_V40
         }

      else if (fValue == SS_VALUE_INT)
         {
         lpData->bDataType = SS_TYPE_FLOAT;
         lpData->Data.dfValue = (double)*(LPLONG)lpszText;
         }

      else if (lpCellType->Type == SS_TYPE_FLOAT && SS_FormatDataFloat(lpSS, lpCellType,
               lpszText, lpszTextFmt, &dfValue, fValue, fCheckRange))
         {
         lpData->bDataType = SS_TYPE_FLOAT;
         lpData->Data.dfValue = dfValue;
         }

#ifdef SS_V40
      else if (lpCellType->Type == SS_TYPE_CURRENCY && SS_FormatDataCurrency(lpSS, lpCellType,
               lpszText, lpszTextFmt, &dfValue, fValue, fCheckRange))
         {
         lpData->bDataType = SS_TYPE_FLOAT;
         lpData->Data.dfValue = dfValue;
         }

      else if (lpCellType->Type == SS_TYPE_NUMBER && SS_FormatDataNumber(lpSS, lpCellType,
               lpszText, lpszTextFmt, &dfValue, fValue, fCheckRange))
         {
         lpData->bDataType = SS_TYPE_FLOAT;
         lpData->Data.dfValue = dfValue;
         }

      else if (lpCellType->Type == SS_TYPE_PERCENT && SS_FormatDataPercent(lpSS, lpCellType,
               lpszText, lpszTextFmt, &dfValue, fValue, fCheckRange))
         {
         lpData->bDataType = SS_TYPE_FLOAT;
         lpData->Data.dfValue = dfValue;
         }
#endif // SS_V40

#ifdef SS_V70
      else if (lpCellType->Type == SS_TYPE_SCIENTIFIC && SS_FormatDataScientific(lpSS, lpCellType,
               lpszText, lpszTextFmt, &dfValue, fValue, fCheckRange))
         {
         lpData->bDataType = SS_TYPE_FLOAT;
         lpData->Data.dfValue = dfValue;
         }
#endif // SS_V70

      else
         {
         lpData->bDataType = 0;
         lpData->Data.hszData = 0;
         }

      return (TRUE);

   case SS_TYPE_EDIT:
//      if (lstrlen(lpszText) == 0)
      if (!lpszText || *lpszText == 0)
         return (TRUE);

      else if (hText = SS_FormatDataEdit(lpSS->lpBook, lpCellType, lpszText,
                                         lpszTextFmt, fValue, fCheckLen))
         {
         lpData->bDataType = SS_TYPE_EDIT;
         lpData->Data.hszData = hText;
         return (TRUE);
         }

      break;

   case SS_TYPE_TIME:
#ifndef SS_NOCT_TIME
      Ret = SS_FormatDataTime(lpCellType, lpszText, lpszTextFmt, fValue);
#endif
      break;

   case SS_TYPE_DATE:
#ifndef SS_NOCT_DATE
      Ret = SS_FormatDataDate(lpSS, lpCellType, lpszText, lpszTextFmt, fValue);
#endif
      break;

   case SS_TYPE_COMBOBOX:
#ifndef SS_NOCT_COMBO
      if (hText = SS_FormatComboBox(lpSS, lpCellType, lpszText, lpszTextFmt, fValue, fCheckLen))
         {
         lpData->bDataType = SS_TYPE_EDIT;
         lpData->Data.hszData = hText;
         return (TRUE);
         }
#endif

      break;

   case SS_TYPE_CHECKBOX:
      Ret = SS_FormatDataCheckBox(lpCellType, lpszText, lpszTextFmt);
      break;

   case SS_TYPE_BUTTON:
      Ret = SS_FormatDataButton(lpCellType, lpszText, lpszTextFmt);
      break;

#ifdef SS_V80
   case SS_TYPE_CUSTOM:
     if( fValue == SS_VALUE_VALUE )
     {
        SS_CT_VALUE Value;
        HGLOBAL hData = NULL;
        int len = lstrlen(lpszText);
        LPTSTR lpszValue;
        LPSS_CELLTYPE CellTypeTemp = SS_CT_LockCellType(lpSS, Col, Row);

        Value.u.hszValue = GlobalAlloc(GHND, (len + 1) * sizeof(TCHAR));
        if( lpszValue = GlobalLock(Value.u.hszValue) )
        {
           Value.type = SS_CT_VALUE_TYPE_TSTR;
           lstrcpy(lpszValue, lpszText);
           GlobalUnlock(Value.u.hszValue);
        }
        Ret = SS_CT_StringFormat(lpSS, CellTypeTemp, Col, Row, &Value, &hData);
        SS_CT_UnlockCellType(lpSS, Col, Row);
        if( Ret && hData )
        {
          SS_FreeCTValue(&Value);
          Value.type = SS_CT_VALUE_TYPE_TSTR;
          Value.u.hszValue = hData;
          SS_CTValueToData(&Value, lpData);
          SS_FreeCTValue(&Value);
          return TRUE;
        }
        break;
     }
     // else fall through to default -scl
#endif
   case SS_TYPE_STATICTEXT:
   case SS_TYPE_OWNERDRAW:
//#if SS_V80
//   case SS_TYPE_CUSTOM:
//#endif
   case SS_TYPE_PICTURE:
   default:
      lpszTextFmt = (LPTSTR)lpszText;
      Ret = TRUE;
      break;
   }

if (Ret)
   {
   if (!lpszTextFmt || *lpszTextFmt == '\0')
      {
      lpData->bDataType = 0;
#ifndef SS_NOOVERFLOW
      lpData->bOverflow = 0;
#endif
      lpData->Data.hszData = 0;
      }
   else
      {
      if (!(hText = tbGlobalAlloc(GMEM_MOVEABLE | GMEM_ZEROINIT,
                                  (lstrlen(lpszTextFmt) + 1)
                                  * sizeof(TCHAR))))
         return (0);

      lpszTextTemp = (LPTSTR)tbGlobalLock(hText);
      StrCpy(lpszTextTemp, lpszTextFmt);
      tbGlobalUnlock(hText);

      lpData->bDataType = SS_TYPE_EDIT;
#ifndef SS_NOOVERFLOW
      lpData->bOverflow = 0;
#endif
      lpData->Data.hszData = hText;
      }

   return (TRUE);
   }

return (FALSE);
}

#ifndef SS_NOCT_PIC

BOOL SS_FormatDataPic(lpCellType, lpszText, lpszTextFmt, fValue)

LPSS_CELLTYPE lpCellType;
LPCTSTR       lpszText;
LPTSTR        lpszTextFmt;
BOOL          fValue;
{
GLOBALHANDLE  hMaskTemp = 0;
GLOBALHANDLE  hDefTemp = 0;
LPTSTR        lpszMask;
LPTSTR        lpszMaskTemp;
LPTSTR        Ptr;
#if defined(_WIN64) || defined(_IA64)
LONG_PTR      Len1;
LONG_PTR      Pos2 = -1;
#else
short         Len1;
short         Pos2 = -1;
#endif
BOOL          fRet;

if (lpCellType->Spec.Pic.hMask)
   {
   lpszMask = (LPTSTR)tbGlobalLock(lpCellType->Spec.Pic.hMask);

   if ((Ptr = _ftcschr(lpszMask, '\n')) || (Ptr = _ftcschr(lpszMask, '\t')))
      {
      Len1 = Ptr - lpszMask;
      Pos2 = Len1 + 1;
      }
   else
      Len1 = lstrlen(lpszMask);

   if (!(hMaskTemp = GlobalAlloc(GMEM_MOVEABLE | GMEM_ZEROINIT,
                                 (Len1 + 1) * sizeof(TCHAR))))
      {
      tbGlobalUnlock(lpCellType->Spec.Pic.hMask);
      return (FALSE);
      }

   lpszMaskTemp = (LPTSTR)GlobalLock(hMaskTemp);
   _fmemcpy(lpszMaskTemp, lpszMask, Len1 * sizeof(TCHAR));
   GlobalUnlock(hMaskTemp);

   if (Pos2 != -1)
      {
      if (!(hDefTemp = GlobalAlloc(GMEM_MOVEABLE | GMEM_ZEROINIT,
                                   (lstrlen(&lpszMask[Pos2]) + 1)
                                   * sizeof(TCHAR))))
         {
         tbGlobalUnlock(lpCellType->Spec.Pic.hMask);
         return (FALSE);
         }

      lpszMaskTemp = (LPTSTR)GlobalLock(hDefTemp);
      lstrcpy(lpszMaskTemp, &lpszMask[Pos2]);
      GlobalUnlock(hDefTemp);
      }

   tbGlobalUnlock(lpCellType->Spec.Pic.hMask);
   }

fRet = PicFmtText(0, hDefTemp, hMaskTemp, lpszText, lpszTextFmt, fValue);

if (hMaskTemp)
   GlobalFree(hMaskTemp);

if (hDefTemp)
   GlobalFree(hDefTemp);

return (fRet);
}

#endif


#ifndef SS_NOCT_DATE

BOOL SS_FormatDataDate(LPSPREADSHEET lpSS, LPSS_CELLTYPE lpCellType,
                       LPCTSTR lpszText, LPTSTR lpszTextFmt, BOOL fValue)
{
TB_DATE Date;
BOOL    fRet = TRUE;

if (fValue)
   {
   // Modified to support JPN dates. (Masanori Iwasa)
   if (SS_DateValidate(lpszText, &Date, lpCellType->Spec.Date.Format))
      DateDMYToString(&Date, lpszTextFmt, &lpCellType->Spec.Date.Format);
   else
      fRet = FALSE;
   }

else
   {
   if (*lpszText == '\0' ||
       (fRet = DateStringIsValidEx(lpszText, &lpCellType->Spec.Date.Format,
                                   lpSS->lpBook->nTwoDigitYearMax)))
      lstrcpy(lpszTextFmt, lpszText);
   }

/* RFW - 2/2/05 - 6944
#ifdef SPREAD_JPN 
*/
// 96' 6/24 Added by BOC Gao. for check date range when setting Text property
// 97' 2/3 Cut out the year checking part that makes an error when MinValue 
// is lower than 1900.
	if ( fRet && *lpszTextFmt != '\0' )
	{
		TB_DATE			DateMax, DateMin;
		LONG			lMax, lMin, lDate;

		DateMax = lpCellType->Spec.Date.Max;
		DateMin = lpCellType->Spec.Date.Min;
		DateStringToDMYEx (lpszTextFmt, &Date, &lpCellType->Spec.Date.Format, lpSS->lpBook->nTwoDigitYearMax);

		lDate = DateDMYToJulian (&Date);
		lMax = DateDMYToJulian (&DateMax);
		lMin = DateDMYToJulian (&DateMin);
		
		//Modify by BOC 99.6.15 (hyt)-----------
		//when bind to database not setting min&max
		//should not check value
		if (lMax!=-1 || lMin!=-1)
			if (lDate < lMin || lDate > lMax)
				return ( FALSE );
		//-----------------------------------------
	}
// -----------------------------------<<
/* RFW - 2/2/05 - 6944
#endif
*/

return (fRet);
}

#endif


#ifndef SS_NOCT_COMBO

TBGLOBALHANDLE SS_FormatComboBox(LPSPREADSHEET lpSS, LPSS_CELLTYPE lpCellType, LPCTSTR lpszText,
                                 LPTSTR lpszTextFmt, BOOL fValue, BOOL fCheckLen)
{
TBGLOBALHANDLE hText = 0;
LPTSTR         lpItems;
LPTSTR         lpszTemp;
TCHAR          szIndex[16];
BOOL           fFound = FALSE;
short          dIndex;
short          Len;
short          i;

if (lpszTextFmt)
   _fmemset(lpszTextFmt, '\0', 2 * sizeof(TCHAR));

if (lpszText && *lpszText)
   {
   if (fValue)
		{
      dIndex = StringToInt(lpszText);
		lpszText = _T("");
		}
   else
      dIndex = -1;

   if (lpCellType->Spec.ComboBox.hItems)
      {
      lpItems = (LPTSTR)tbGlobalLock(lpCellType->Spec.ComboBox.hItems);

      for (i = 0; i < lpCellType->Spec.ComboBox.dItemCnt; i++)
         {
// Begin Modify - by BOC(Tang) - for bug08 - 07/05/1996 ---------------------------------->>
// I don't know why spread read empty value of string as 0 at occasion(refer to 
// FetchCurrentRow function at line 1149 of Fpoledb.cpp), I can not 
// keep on tracing into GetNextRows at line 1177, so I have to
// check it here.
//#ifdef	SPREAD_JPN
         if (lpszText && ((fValue && i == dIndex) ||
             (!fValue && _ftcscmp(lpszText, lpItems) == 0)))
//#else
//         if ((fValue && i == dIndex) || (!fValue &&
//             _ftcscmp(lpszText, lpItems) == 0))
//#endif
// End Modify
            {
            wsprintf(szIndex, _T("%d"), i);
            Len = lstrlen(lpItems) + 1 + lstrlen(szIndex) + 1;
            if (hText = tbGlobalAlloc(GMEM_MOVEABLE | GMEM_ZEROINIT,Len * sizeof(TCHAR)))
               {
               lpszTemp = (LPTSTR)tbGlobalLock(hText);
               lstrcpy(lpszTemp, lpItems);
               lstrcpy(&lpszTemp[lstrlen(lpszTemp)+1], szIndex);
               tbGlobalUnlock(hText);
               }
            fFound = TRUE;
            break;
            }

         lpItems += lstrlen(lpItems) + 1;
         }

      tbGlobalUnlock(lpCellType->Spec.ComboBox.hItems);
      }

// RFW - 9358 - Moved this block below the previous if
// fix for bug #8154 -scl
#ifdef SS_V30
   else if (lpCellType->Spec.ComboBox.hWndDropDown)
   {
     if (fValue)
        {
        SendMessage(lpCellType->Spec.ComboBox.hWndDropDown, CBM_SETPROPERTY, LBPROP_ListIndex, dIndex);
        hText = SSCB_GetTextFromListEx(lpCellType->Spec.ComboBox.hWndDropDown, dIndex);
        }
     else
        {
        // editable
        if (lpCellType->Style & SS_CB_DROPDOWN)
           {
           TCHAR   szIndex[16];
           wsprintf(szIndex, _T("%d"), dIndex);

           if (hText = tbGlobalAlloc(GMEM_MOVEABLE | GMEM_ZEROINIT, 
                                     (lstrlen(lpszText) + 1 + lstrlen(szIndex) + 1) * sizeof(TCHAR)))
              {
              LPTSTR lpText = (LPTSTR)tbGlobalLock(hText);
              lstrcpy(lpText, lpszText);
              lstrcpy(&lpText[lstrlen(lpText)+1], szIndex);
              tbGlobalUnlock(hText);
              }
           fFound = TRUE;
           }
// fix for bug #9075 -scl
        else
          {
            short nColEdit = (short)SendMessage(lpCellType->Spec.ComboBox.hWndDropDown,
                                                CBM_GETPROPERTY,
                                                CBPROP_ColumnEdit, 0);
            SendMessage(lpCellType->Spec.ComboBox.hWndDropDown, LBM_SETPROPERTY,
                        LBPROP_ColumnSearch, (LPARAM)nColEdit);

            dIndex = (short)SendMessage(lpCellType->Spec.ComboBox.hWndDropDown, CB_FINDSTRINGEXACT,
                                         (WPARAM)-1, (LPARAM)lpszText);
            if (dIndex != LB_ERR)
            {
              TCHAR   szIndex[16];
              wsprintf(szIndex, _T("%d"), dIndex);

              if (hText = tbGlobalAlloc(GMEM_MOVEABLE | GMEM_ZEROINIT, 
                                       (lstrlen(lpszText) + 1 + lstrlen(szIndex) + 1) * sizeof(TCHAR)))
                {
                LPTSTR lpText = (LPTSTR)tbGlobalLock(hText);
                lstrcpy(lpText, lpszText);
                lstrcpy(&lpText[lstrlen(lpText)+1], szIndex);
                tbGlobalUnlock(hText);
                }
              fFound = TRUE;
            }
				// RFW - 9168 - 9/6/01
				else
					{
					wsprintf(szIndex, _T("%d"), -1);
					Len = lstrlen(lpszText) + 1 + lstrlen(szIndex) + 1;
					if (hText = tbGlobalAlloc(GMEM_MOVEABLE | GMEM_ZEROINIT,Len * sizeof(TCHAR)))
						{
						lpszTemp = (LPTSTR)tbGlobalLock(hText);
						lstrcpy(lpszTemp, lpszText);
						lstrcpy(&lpszTemp[lstrlen(lpszTemp)+1], szIndex);
						tbGlobalUnlock(hText);
						}
					fFound = TRUE;
					}
				 }
        }
   }
#endif

// fix for bug #9075 -scl
//   if (!fFound && ((lpCellType->Style & SS_CB_DROPDOWN) || lpCellType->Spec.ComboBox.hWndDropDown))
/* RFW - 2/15/05 - 15396
   if ((lpCellType->Style & SS_CB_DROPDOWN) && !fFound)
*/
	// If a combo cell is using a ListPro combo, at the time an SS file is being loaded, there is no way
	// to validate the text, so allow anything to be loaded.
   if (!fFound && ((lpCellType->Style & SS_CB_DROPDOWN) || lpSS->lpBook->hFile))
      {
		// RFW - 4/6/04 - 14047
		short nTextLen = lstrlen(lpszText);

		// RFW - 2/15/05 - 15733
		if (fCheckLen && lpCellType->Spec.ComboBox.dMaxEditLen != -1)
			nTextLen = min(lpCellType->Spec.ComboBox.dMaxEditLen, nTextLen);

      wsprintf(szIndex, _T("%d"), -1);
      Len = nTextLen + 1 + lstrlen(szIndex) + 1;
      if (hText = tbGlobalAlloc(GMEM_MOVEABLE | GMEM_ZEROINIT, Len * sizeof(TCHAR)))
         {
         lpszTemp = (LPTSTR)tbGlobalLock(hText);
         lstrcpyn(lpszTemp, lpszText, nTextLen + 1);
         lstrcpy(&lpszTemp[nTextLen + 1], szIndex);
         tbGlobalUnlock(hText);
         }
      }
   }

if( !hText )
   {
   if (hText = tbGlobalAlloc(GMEM_MOVEABLE | GMEM_ZEROINIT, 2 * sizeof(TCHAR)))
      {
      lpszTemp = (LPTSTR)tbGlobalLock(hText);
      _fmemset(lpszTemp, '\0', 2 * sizeof(TCHAR));
      tbGlobalUnlock(hText);
      }
   }

return (hText);
}

#endif


#ifndef SS_NOCT_TIME

BOOL SS_FormatDataTime(lpCellType, lpszText, lpszTextFmt, fValue)

LPSS_CELLTYPE lpCellType;
LPCTSTR       lpszText;
LPTSTR        lpszTextFmt;
BOOL          fValue;
{
TIME          Time;
BOOL          fRet = TRUE;

if (fValue)
   {
   if (SS_TimeValidate(lpszText, &Time))
      TimeHMSToString(&Time, lpszTextFmt, &lpCellType->Spec.Time.Format);
   else
      fRet = FALSE;
   }

else
   {
   if (*lpszText == '\0' ||
       (fRet = TimeStringIsValid(lpszText, &lpCellType->Spec.Time.Format)))
      lstrcpy(lpszTextFmt, lpszText);
   }

// 96' 6/26 Added by BOC Gao. 
#ifdef SPREAD_JPN
// - for check Time range when setting Text or Value property
	if ( fRet && *lpszTextFmt != '\0' )
	{
		TIME			TimeMax, TimeMin;
		LONG			lMax, lMin, lTime;

		TimeMax = lpCellType->Spec.Time.Max;
		TimeMin = lpCellType->Spec.Time.Min;
		TimeStringToHMS (lpszTextFmt, &Time, &lpCellType->Spec.Time.Format);

		lTime = TimeHMSToSeconds (&Time);
		lMax = TimeHMSToSeconds (&TimeMax);
		lMin = TimeHMSToSeconds (&TimeMin);
		
		//Modify by BOC 99.6.9 (hyt)-----------
		//when bind to database not setting min&max
		//should not check value
		if(lMin != 0 || lMax!=0)
			if (lTime < lMin || lTime > lMax)
				return ( FALSE );
		//--------------------------------------
	}
#endif
// -----------------------------------<<

return (fRet);
}

#endif


BOOL SS_FormatDataFloat(lpSS, lpCellType, lpszText, lpszTextFmt,
                        lpdfValue, fValue, fCheckRange)

LPSPREADSHEET lpSS;
LPSS_CELLTYPE lpCellType;
LPCTSTR       lpszText;
LPTSTR        lpszTextFmt;
LPDOUBLE      lpdfValue;
BOOL          fValue;
BOOL          fCheckRange;
{
short         Len;
BOOL          fRet = FALSE;

if (*lpszText)
   {
   Len = min(lstrlen(lpszText), 40 - 1);
   _fmemcpy(lpszTextFmt, lpszText, Len * sizeof(TCHAR));
   lpszTextFmt[Len] = '\0';

   if (fValue)
      fRet = TRUE;
   else
      fRet = SS_FloatStringStrip(lpSS, lpCellType, lpszTextFmt);

   // RFW - 6/22/00 - GIC12166
   // SS_StringToFloat(lpSS, lpszTextFmt, lpdfValue);
   StringToFloat(lpszTextFmt, lpdfValue);

#ifndef SS_UTP
   fRet = TRUE;
#endif

   if (fCheckRange &&
       (*lpdfValue < lpCellType->Spec.Float.Min ||
        *lpdfValue > lpCellType->Spec.Float.Max))
      fRet = FALSE;
   }

return (fRet);
}


static BOOL SS_FormatDataCheckBox(LPSS_CELLTYPE lpCellType, LPCTSTR lpszText,
                                  LPTSTR lpszTextFmt)
{
BOOL fRet = FALSE;
int  iVal = 0;

if (lpszText)
	iVal = _ttoi(lpszText);

/*
if ((lpCellType->Style & 0x0F) == BS_3STATE || (lpCellType->Style & 0x0F) == BS_AUTO3STATE)
   {
   if (!lpszText || !*lpszText ||
       (!lpszText[1] && (*lpszText == '0' || *lpszText == '1' || *lpszText == '2')))
		fRet = TRUE;
   }
else
   {
   if (!lpszText || !*lpszText ||
       (!lpszText[1] && (*lpszText == '0' || *lpszText == '1')))
		fRet = TRUE;
   }
*/

if (!lpszText || !*lpszText)
	fRet = TRUE;

else if ((lpCellType->Style & 0x0F) == BS_3STATE || (lpCellType->Style & 0x0F) == BS_AUTO3STATE)
   {
   if (iVal == 0 || iVal == 1 || iVal == -1 || iVal == 2)
		fRet = TRUE;
   }
else
   {
   if (iVal == 0 || iVal == 1 || iVal == -1)
		fRet = TRUE;
   }

if (fRet && lpszText)
	lstrcpy(lpszTextFmt, lpszText);

return (fRet);
}


static BOOL SS_FormatDataButton(LPSS_CELLTYPE lpCellType, LPCTSTR lpszText,
                                LPTSTR lpszTextFmt)
{
BOOL fRet = FALSE;

if (lpCellType->Spec.Button.ButtonType == SUPERBTN_TYPE_2STATE)
   {
   if (!lpszText || !*lpszText ||
       (!lpszText[1] && (*lpszText == '0' || *lpszText == '1')))
		fRet = TRUE;
   }

if (fRet && lpszText)
	lstrcpy(lpszTextFmt, lpszText);

return (fRet);
}


BOOL SS_FloatStringStrip(LPSPREADSHEET lpSS, LPSS_CELLTYPE lpCellType,
                         LPTSTR lpszText)
{
LPTSTR lpTextTemp;
LPTSTR lpTextNew;
TCHAR  cSeparator;
TCHAR  cDecimalSign;
TCHAR  cCurrencySign;
BOOL   fRet = TRUE;
SS_FLOATFORMAT ff;

SS_GetDefFloatFormat(lpSS->lpBook, &ff);

lpTextTemp = lpszText;
lpTextNew = lpszText;

if (lpCellType && lpCellType->Spec.Float.fSetFormat &&
    lpCellType->Spec.Float.Format.cSeparator)
   cSeparator = lpCellType->Spec.Float.Format.cSeparator;
else
   cSeparator = ff.cSeparator;

if (lpCellType && lpCellType->Spec.Float.fSetFormat &&
    lpCellType->Spec.Float.Format.cDecimalSign)
   cDecimalSign = lpCellType->Spec.Float.Format.cDecimalSign;
else
   cDecimalSign = ff.cDecimalSign;

if (lpCellType && lpCellType->Spec.Float.fSetFormat &&
    lpCellType->Spec.Float.Format.cCurrencySign)
   cCurrencySign = lpCellType->Spec.Float.Format.cCurrencySign;
else
   cCurrencySign = ff.cCurrencySign;

do
   {
   if (*lpTextTemp == cDecimalSign)
      {
      *lpTextNew = '.';
      lpTextNew++;
      }

   else if (*lpTextTemp == cSeparator || *lpTextTemp == cCurrencySign ||
       *lpTextTemp == ')')
      ;


   else if (*lpTextTemp == '(')
      {
      *lpTextNew = '-';
      lpTextNew++;
      }

   else if (isdigit(*lpTextTemp) || *lpTextTemp == '.' ||
            *lpTextTemp == '\0' || *lpTextTemp == '-' || *lpTextTemp == 'd' ||
            *lpTextTemp == 'D' || *lpTextTemp == 'e' || *lpTextTemp == 'E')
      {
      *lpTextNew = *lpTextTemp;
      lpTextNew++;
      }
   else
      fRet = FALSE;

   } while (*(lpTextTemp++));

return (fRet);
}


int SS_StrReplaceCh(LPTSTR szStr, TCHAR chOld, TCHAR chNew)
{
LPTSTR szPtr = szStr;

while (*szStr)
   {
   if (*szStr == chOld)
      *szStr = chNew;

   ++szStr;
   }

return ((int)(szStr - szPtr));
}


void DLLENTRY SSFloatFormat(hWnd, lpCellType, dfVal, lpszText, fValue)

HWND          hWnd;
LPSS_CELLTYPE lpCellType;
double        dfVal;
LPTSTR        lpszText;
BOOL          fValue;
{
LPSPREADSHEET lpSS;

lpSS = SS_SheetLock(hWnd);
SS_FloatFormat(lpSS, lpCellType, dfVal, lpszText, fValue);
SS_SheetUnlock(hWnd);
}


short SS_GetScientificExp(double x)
{
short nRet = 0;

x = fabs(x);
if (x >= 10)
	{
	for (; x >= 10; nRet++)
		x /= 10.0;
	}
else if (x < 1)
	{
	for (; x < 1; nRet++)
		x *= 10.0;
	}

return (nRet);
}


#ifdef SS_V70
BOOL DLLENTRY SS_ScientificFormat(double dfVal, LPTSTR lpszText, BOOL fValue, short nDecimalPlaces, LPTSTR lpszDecimal)
{
TCHAR  szDecimal[5];
double dfValAbs = fabs(dfVal);
int    iLen;

if (fValue)
  _stprintf(lpszText, _T("%.15G"), dfVal);
else
	_stprintf(lpszText, _T("%#.*E"), nDecimalPlaces, dfVal);

// sprintf always formats with 3 characters right of the E sign.  We would
// like to only have a minimum of 2 if the 3 one is not necessary, so if
// the first char right of the E is 0, then remove it.
iLen = lstrlen(lpszText);
if (iLen > 5 && lpszText[iLen - 5] == 'E' && lpszText[iLen - 3] == '0')
	StrDeleteChar(lpszText, iLen - 3);

SSx_GetScientificDecimal(lpszDecimal, szDecimal);

if (szDecimal[0] && szDecimal[0] != '.')
	SS_StrReplaceCh(lpszText, '.', szDecimal[0]);

return (TRUE);
}
#endif // SS_V70


// Take a double and turn it into a string
void SS_FloatFormat(LPSPREADSHEET lpSS, LPSS_CELLTYPE lpCellType, double dfVal, LPTSTR lpszText, BOOL fValue)
{
BYTE           bFractionalWidth;
TCHAR          cSeparator;
TCHAR          cDecimalSign;
TCHAR          cCurrencySign;
SS_FLOATFORMAT ff;

#ifdef SS_V40
if (SS_IsCellTypeNum(lpCellType))
	{
	SS_NumFormat(lpSS, lpCellType, dfVal, lpszText, fValue);
	return;
	}
#endif // SS_V40

#ifdef SS_V70
if (lpCellType->Type == SS_TYPE_SCIENTIFIC)
   {
	SS_ScientificFormat(dfVal, lpszText, fValue, lpCellType->Spec.Scientific.Right, lpCellType->Spec.Scientific.szDecimal);
	return;
	}
#endif // SS_V70

SS_GetDefFloatFormat(lpSS->lpBook, &ff);

if (lpCellType->Type == SS_TYPE_FLOAT)
   {
   // BJO 12Jul96 KEM3755 - Before fix
   //#ifndef SS_USEAWARE
   //if (lpCellType->Spec.Float.Right == 0)
   //   bFractionalWidth = 1;
   //else
   //#endif
   //   bFractionalWidth = (BYTE)lpCellType->Spec.Float.Right;
   // BJO 12Jul96 KEM3755 - Begin fix
      bFractionalWidth = (BYTE)lpCellType->Spec.Float.Right;
   // BJO 12Jul96 KEM3755 - End fix
   }
else if (lpCellType->Type == SS_TYPE_INTEGER)
   bFractionalWidth = 0;
else
   bFractionalWidth = 2;

if (lpCellType->Type == SS_TYPE_FLOAT && lpCellType->Spec.Float.fSetFormat &&
    lpCellType->Spec.Float.Format.cSeparator)
   cSeparator = lpCellType->Spec.Float.Format.cSeparator;
else
   cSeparator = ff.cSeparator;

if (lpCellType->Type == SS_TYPE_FLOAT && lpCellType->Spec.Float.fSetFormat &&
    lpCellType->Spec.Float.Format.cDecimalSign)
   cDecimalSign = lpCellType->Spec.Float.Format.cDecimalSign;
else
   cDecimalSign = ff.cDecimalSign;

if (lpCellType->Type == SS_TYPE_FLOAT && lpCellType->Spec.Float.fSetFormat &&
    lpCellType->Spec.Float.Format.cCurrencySign)
   cCurrencySign = lpCellType->Spec.Float.Format.cCurrencySign;
else
   cCurrencySign = ff.cCurrencySign;

#ifdef SS_USEAWARE

if (fValue)
   fpDoubleToString(lpszText, dfVal, bFractionalWidth,
                    '.', 0, FALSE, (lpCellType->Style & FS_MONEY) ? TRUE :
                    FALSE, TRUE, FALSE, 0, NULL);
else
   {
   TCHAR szCurrencySign[1 + 1];

   szCurrencySign[0] = cCurrencySign;
   szCurrencySign[1] = '\0';

   fpDoubleToString(lpszText, dfVal, bFractionalWidth, cDecimalSign,
                    cSeparator, FALSE, (lpCellType->Style & FS_MONEY) ? TRUE :
                    FALSE, TRUE, FALSE, 0,
                    (lpCellType->Style & FS_MONEY) ? szCurrencySign : NULL);
   }

#else

/* RFW - 11/4/05 - 17533
dfVal = ApproxRound(dfVal, bFractionalWidth);
*/
dfVal = ApproxRound2(dfVal, bFractionalWidth);

if (!fValue && (lpCellType->Style & FS_SEPARATOR))
   StrPrintf(lpszText, _T("%.*Sm"), bFractionalWidth,
             cSeparator, cDecimalSign, dfVal);
else
   {
   StrPrintf(lpszText, _T("%.*f"), bFractionalWidth, dfVal);

   if (!fValue)
      SS_StrReplaceCh(lpszText, '.', cDecimalSign);
   }

if (lpCellType->Type == SS_TYPE_FLOAT)
   {
   LPTSTR lpszPtr;

   if (lpszPtr = _ftcschr(lpszText, (_TINT)(fValue ? '.' : cDecimalSign)))
      {
      if (lpCellType->Spec.Float.Right == 0)
         *lpszPtr = '\0';
      else
         lpszPtr[lpCellType->Spec.Float.Right + 1] = '\0';
      }

   if (!fValue && (lpCellType->Style & FS_MONEY))
      StrInsertChar(cCurrencySign, lpszText, dfVal < 0.0 ? 1 : 0);
   }

else if (lpCellType->Type == SS_TYPE_INTEGER)
   {
   LPTSTR lpszPtr;

   if (lpszPtr = _ftcschr(lpszText, (_TINT)(fValue ? '.' : cDecimalSign)))
      *lpszPtr = '\0';
   }

#endif
}


#ifdef SS_V40
void SS_NumFormat(LPSPREADSHEET lpSS, LPSS_CELLTYPE lpCellType, double dfVal, LPTSTR lpszText, BOOL fValue)
{
*lpszText = '\0';

if (!fValue)
	{
	SSNUM_FORMAT Format;

	switch (lpCellType->Type)
		{
		case SS_TYPE_CURRENCY:
			SS_CreateCurrencyFormatStruct(lpCellType, &Format);
			break;

		case SS_TYPE_NUMBER:
			SS_CreateNumberFormatStruct(lpCellType, &Format);
			break;

		case SS_TYPE_PERCENT:
			SS_CreatePercentFormatStruct(lpCellType, &Format);
			dfVal *= 100.0;
			break;
		}

	/* RFW - 11/4/03 - 12556
	{
	double dfValLeft;

	dfVal = modf(dfVal, &dfValLeft);
	dfVal = ApproxRound(dfVal, Format.bFractionalWidth + 1); // This is to get around a floating point error.
	dfVal = ApproxRound(dfVal, Format.bFractionalWidth);
	dfVal += dfValLeft;
	}
	*/

	/* RFW - 11/14/05 - 12556
	dfVal = ApproxRound(dfVal, Format.bFractionalWidth);
	*/
	dfVal = ApproxRound2(dfVal, Format.bFractionalWidth);

	NumValueToString(&Format, dfVal, lpszText);
	}

else
	{
	LPTSTR lpszPtr;
	BYTE   bFractionalWidth;

	if (lpCellType->Type == SS_TYPE_CURRENCY)
      bFractionalWidth = (BYTE)lpCellType->Spec.Currency.Right;
	else if (lpCellType->Type == SS_TYPE_NUMBER)
      bFractionalWidth = (BYTE)lpCellType->Spec.Number.Right;
	else if (lpCellType->Type == SS_TYPE_PERCENT)
      bFractionalWidth = (BYTE)lpCellType->Spec.Percent.Right + 2;

	dfVal = ApproxRound(dfVal, bFractionalWidth);

	StrPrintf(lpszText, _T("%.*f"), bFractionalWidth, dfVal);
	
	if (lpszPtr = _ftcschr(lpszText, (_TINT)'.'))
		{
		if (bFractionalWidth == 0)
			*lpszPtr = '\0';
		else
			lpszPtr[bFractionalWidth + 1] = '\0';
		}
	}
}


BOOL SS_FormatDataCurrency(LPSPREADSHEET lpSS, LPSS_CELLTYPE lpCellType, LPCTSTR lpszText,
                           LPTSTR lpszTextFmt, LPDOUBLE lpdfValue, BOOL fValue, BOOL fCheckRange)
{
BOOL fRet = FALSE;

if (*lpszText)
   {
   short Len = min(lstrlen(lpszText), 40 - 1);
   _fmemcpy(lpszTextFmt, lpszText, Len * sizeof(TCHAR));
   lpszTextFmt[Len] = '\0';

   if (fValue)
		StringToFloat(lpszTextFmt, lpdfValue);
   else
		{
		SSNUM_FORMAT Format;
		SS_CreateCurrencyFormatStruct(lpCellType, &Format);
		NumStringToValue(&Format, lpszTextFmt, lpdfValue);
		}

   fRet = TRUE;
   if (fCheckRange &&
       (*lpdfValue < lpCellType->Spec.Currency.Min ||
        *lpdfValue > lpCellType->Spec.Currency.Max))
      fRet = FALSE;
   }

return (fRet);
}


BOOL SS_FormatDataNumber(LPSPREADSHEET lpSS, LPSS_CELLTYPE lpCellType, LPCTSTR lpszText,
                           LPTSTR lpszTextFmt, LPDOUBLE lpdfValue, BOOL fValue, BOOL fCheckRange)
{
BOOL fRet = FALSE;

if (*lpszText)
   {
   short Len = min(lstrlen(lpszText), 40 - 1);
   _fmemcpy(lpszTextFmt, lpszText, Len * sizeof(TCHAR));
   lpszTextFmt[Len] = '\0';

   if (fValue)
		StringToFloat(lpszTextFmt, lpdfValue);
   else
		{
		SSNUM_FORMAT Format;
		SS_CreateNumberFormatStruct(lpCellType, &Format);
		NumStringToValue(&Format, lpszTextFmt, lpdfValue);
		}

   fRet = TRUE;
   if (fCheckRange &&
       (*lpdfValue < lpCellType->Spec.Number.Min ||
        *lpdfValue > lpCellType->Spec.Number.Max))
      fRet = FALSE;
   }

return (fRet);
}


BOOL SS_FormatDataPercent(LPSPREADSHEET lpSS, LPSS_CELLTYPE lpCellType, LPCTSTR lpszText,
                           LPTSTR lpszTextFmt, LPDOUBLE lpdfValue, BOOL fValue, BOOL fCheckRange)
{
BOOL fRet = FALSE;

if (*lpszText)
   {
   short Len = min(lstrlen(lpszText), 40 - 1);
   _fmemcpy(lpszTextFmt, lpszText, Len * sizeof(TCHAR));
   lpszTextFmt[Len] = '\0';

   if (fValue)
		StringToFloat(lpszTextFmt, lpdfValue);
   else
		{
		SSNUM_FORMAT Format;
		SS_CreatePercentFormatStruct(lpCellType, &Format);
		NumStringToValue(&Format, lpszTextFmt, lpdfValue);
		*lpdfValue /= 100.0;
		}

   fRet = TRUE;
	/* RFW - 7/30/03 - JPN bug.
   if (fCheckRange &&
       (*lpdfValue < (lpCellType->Spec.Percent.Min / 100.0) ||
        *lpdfValue > (lpCellType->Spec.Percent.Max / 100.0)))
	*/
   if (fCheckRange)
		{
		TCHAR szBuf[100];
		double dfVal = *lpdfValue * 100.0;

		// The following code was added to take care of an Intel floating point problem.
		StrPrintf(szBuf, _T("%.*f"), lpCellType->Spec.Percent.Right, dfVal);
		StringToFloat(szBuf, &dfVal);

      if (dfVal < lpCellType->Spec.Percent.Min ||
          dfVal > lpCellType->Spec.Percent.Max)
			fRet = FALSE;
		}
   }

return (fRet);
}
#endif // SS_V40


#ifdef SS_V70

double SS_ScientificToFloat(LPTSTR lpszText, LPSS_CELLTYPE lpCellType)
{
TCHAR  szText[200];
TCHAR  szDecimal[5 + 1];
double dfValue;

SS_GetScientificDecimal(lpCellType, szDecimal);

lstrcpyn(szText, lpszText, 200);
if (szDecimal[0] != '.')
   SS_StrReplaceCh(szText, szDecimal[0], '.');

StringToFloat(szText, &dfValue);
return (dfValue);
}

BOOL SS_FormatDataScientific(LPSPREADSHEET lpSS, LPSS_CELLTYPE lpCellType, LPCTSTR lpszText,
                             LPTSTR lpszTextFmt, LPDOUBLE lpdfValue, BOOL fValue, BOOL fCheckRange)
{
BOOL fRet = FALSE;

if (*lpszText)
   {
   short Len = min(lstrlen(lpszText), 40 - 1);
   _fmemcpy(lpszTextFmt, lpszText, Len * sizeof(TCHAR));
   lpszTextFmt[Len] = '\0';

	*lpdfValue = SS_ScientificToFloat(lpszTextFmt, lpCellType);

	/* RFW - 5/28/04 - 14446
	if ((ULONGLONG)*lpdfValue >= DOUBLE_INFINITY_VALUE)
		{
		*lpdfValue = 0;
		return (FALSE);
		}
	*/

   fRet = TRUE;
   if (fCheckRange &&
       (*lpdfValue < lpCellType->Spec.Scientific.Min ||
        *lpdfValue > lpCellType->Spec.Scientific.Max))
      fRet = FALSE;
   }

return (fRet);
}
#endif // SS_V70


BOOL SS_FormatDataInteger(lpCellType, lpszText, lpszTextFmt,
                          lplValue, fValue, fCheckRange)

LPSS_CELLTYPE lpCellType;
LPCTSTR       lpszText;
LPTSTR        lpszTextFmt;
LPLONG        lplValue;
BOOL          fValue;
BOOL          fCheckRange;
{
BOOL          fRet = FALSE;

if (lpszText && *lpszText)
   {
   *lplValue = StringToLong(lpszText);

   if (!fCheckRange ||
       (*lplValue >= lpCellType->Spec.Integer.Min &&
        *lplValue <= lpCellType->Spec.Integer.Max))
      fRet = TRUE;
   }

return (fRet);
}


TBGLOBALHANDLE SS_FormatDataEdit(LPSS_BOOK lpBook, LPSS_CELLTYPE  lpCellType,
                                 LPCTSTR lpszText, LPTSTR lpszTextFmt,
                                 BOOL fValue, BOOL fCheckLen)
{
TBGLOBALHANDLE hText;
BOOL           fRet = TRUE;
short          dLen;
short          i;

if (fCheckLen)
   dLen = min(lpCellType->Spec.Edit.Len, lstrlen(lpszText));
else
   dLen = lstrlen(lpszText);

if (dLen == 0)
   return (0);

//if (dLen < 0 || dLen > 1999)
//	return (0);

if (!(hText = tbGlobalAlloc(GMEM_MOVEABLE | GMEM_ZEROINIT,
                            (dLen + 1) * sizeof(TCHAR))))
   return (0);

lpszTextFmt = (LPTSTR)tbGlobalLock(hText);

lpszTextFmt[dLen] = '\0';

if (lpCellType->Spec.Edit.ChrSet == SS_CHRSET_CHR &&
    lpCellType->Spec.Edit.ChrCase == SS_CASE_NOCASE)
   {
   _fmemcpy(lpszTextFmt, lpszText, dLen * sizeof(TCHAR));
   // Wei Feng for Incident 185
#ifndef _UNICODE
   for (i = 0; i < dLen; i++)
      {
      if (IsDBCSLeadByte( (BYTE)(*(lpszTextFmt + i)) ) )
         {
         if (i == dLen - 1)
            (BYTE)(*(lpszTextFmt + i)) = 0;
         i++;
         }
      }
#endif
   }
else
   {
   int j = 0;
   for (i = 0, j = 0; fRet && i < dLen; i++)
      {
      // Yagi No.5 Wei Feng, 1997/9/9
/*      #ifdef	SPREAD_JPN
      int nRet = SS_EditIsValidKey(lpSS->lpBook, lpCellType, lpszText[i]);
      if(nRet && nRet != 2)
      #else
      if (SS_EditIsValidKey(lpSS->lpBook, lpCellType, lpszText[i]))
      #endif*/

	  int nRet = 0;
	  if (lpCellType->Style & (ES_AUTOHSCROLL | ES_MULTILINE) &&
            (lpszText[i] == '\r' || lpszText[i] == '\n'))
           nRet = 1;
        else
           nRet = SS_EditIsValidKey(lpBook, lpCellType, lpszText[i]);

      if (nRet == 1)
         {
         if (lpCellType->Spec.Edit.ChrCase == SS_CASE_UCASE)
            #if defined(_WIN64) || defined(_IA64)
            lpszTextFmt[j/*i*/] = (TCHAR)CharUpper((LPTSTR)MAKELONG_PTR(lpszText[i],0));
            #elif defined(WIN32)
            lpszTextFmt[j/*i*/] = (TCHAR)CharUpper((LPTSTR)MAKELONG(lpszText[i],0));
            #else
            lpszTextFmt[j/*i*/] = (TCHAR)(long)AnsiUpper((LPTSTR)MAKELONG(lpszText[i],0));
            #endif

         else if (lpCellType->Spec.Edit.ChrCase == SS_CASE_LCASE)
            #if defined(_WIN64) || defined(_IA64)
            lpszTextFmt[j/*i*/] = (TCHAR)CharLower((LPTSTR)MAKELONG_PTR(lpszText[i],0));
            #elif defined(WIN32)
            lpszTextFmt[j/*i*/] = (TCHAR)CharLower((LPTSTR)MAKELONG(lpszText[i],0));
            #else
            lpszTextFmt[j/*i*/] = (TCHAR)(long)AnsiLower((LPTSTR)MAKELONG(lpszText[i],0));
            #endif

         else
            lpszTextFmt[j/*i*/] = lpszText[i];
         // 96' 6/24 Added by BOC Gao. for Japanese processing
         j++;
#ifndef _UNICODE
         if ( IsDBCSLeadByte((BYTE)lpszText[i]) ) 
            {
	         if (i == dLen - 1)
					lpszTextFmt[j - 1] = '\0';
				else
					{
					i++;
					lpszTextFmt[j] = lpszText[i];
					j++;
					}
            }
#endif
         // -------------------------<<
         }
      else
         // Yagi No. 5 Wei Feng 1997/9/9
         #if defined(SPREAD_JPN) && !defined(_UNICODE)
         {
         if ( IsDBCSLeadByte((BYTE)lpszText[i]) ) 
            i++;
         } // IsValidate
         #else
         fRet = FALSE;
         #endif
      } // for

   // Yagi No. 5 Wei Feng 1997/9/9
   #ifdef	SPREAD_JPN
   lpszTextFmt[j] = '\0';
   if (j == 0)	
      fRet = FALSE;
   #endif
   }

tbGlobalUnlock(hText);

if (!fRet)
   {
   tbGlobalFree(hText);
   hText = 0;
   }

return (hText);
}


TBGLOBALHANDLE SS_UnFormatData(lpSS, Col, Row, lpCellType, lpszText)

LPSPREADSHEET  lpSS;
SS_COORD       Col;
SS_COORD       Row;
LPSS_CELLTYPE  lpCellType;
LPTSTR         lpszText;
{
TBGLOBALHANDLE hText;
LPTSTR         lpszTextTemp;
TCHAR          szTextValueBuffer[SS_MAXDATALEN];
LPTSTR         lpszTextValue;
BOOL           Ret = FALSE;

lpszTextValue = szTextValueBuffer;
_fmemset(szTextValueBuffer, '\0', sizeof(szTextValueBuffer));

switch (lpCellType->Type)
   {
   case SS_TYPE_PIC:
#ifndef SS_NOCT_PIC
      Ret = SS_UnFormatDataPic(lpCellType, lpszText, lpszTextValue);
#endif
      break;

   case SS_TYPE_DATE:
#ifndef SS_NOCT_DATE
      Ret = SS_UnFormatDataDate(lpSS, lpCellType, lpszText, lpszTextValue);
#endif
      break;

   case SS_TYPE_TIME:
#ifndef SS_NOCT_TIME
      Ret = SS_UnFormatDataTime(lpCellType, lpszText, lpszTextValue);
#endif
      break;

   case SS_TYPE_EDIT:
   case SS_TYPE_FLOAT:
   case SS_TYPE_INTEGER:
#ifdef SS_V40
   case SS_TYPE_CURRENCY:
   case SS_TYPE_NUMBER:
   case SS_TYPE_PERCENT:
#endif // SS_V40
#ifdef SS_V70
	case SS_TYPE_SCIENTIFIC:
#endif // SS_V70
      if (!(hText = tbGlobalAlloc(GMEM_MOVEABLE | GMEM_ZEROINIT,
                                  (lstrlen(lpszText) + 1) * sizeof(TCHAR))))
         return (0);

      lpszTextTemp = (LPTSTR)tbGlobalLock(hText);
      lstrcpy(lpszTextTemp, lpszText);
      tbGlobalUnlock(hText);

      return (hText);

   case SS_TYPE_COMBOBOX:
      lpszTextValue = lpszText + lstrlen(lpszText) + 1;
      Ret = TRUE;
      break;

   case SS_TYPE_BUTTON:
   case SS_TYPE_CHECKBOX:
      lpszTextValue = lpszText;
      if (!lpszText || !*lpszText)
         lpszTextValue = _T("0");

      Ret = TRUE;
      break;

#ifdef SS_V80
   case SS_TYPE_CUSTOM:
      {
         SS_DATA DataItem = {0};
         TBGLOBALHANDLE hUnformatted = 0;
         if( SS_CT_StringUnformat(lpSS, lpCellType, Col, Row, lpszText, &DataItem) )
         {
           switch( DataItem.bDataType )
           {
           case SS_DATATYPE_EDIT:
             hUnformatted = DataItem.Data.hszData;
             break;
           case SS_DATATYPE_FLOAT:
             break;
           case SS_DATATYPE_INTEGER:
             break;
           }
           return hUnformatted;
         }
      }
      break;
#endif // SS_V80

   case SS_TYPE_STATICTEXT:
   case SS_TYPE_OWNERDRAW:
//#if SS_V80
//   case SS_TYPE_CUSTOM:
//#endif
   case SS_TYPE_PICTURE:
   default:
      lpszTextValue = lpszText;
      Ret = TRUE;
      break;
   }

if (Ret)
   {
   int nTextLen = lstrlen(lpszTextValue) + 1;
   if (!(hText = tbGlobalAlloc(GMEM_MOVEABLE | GMEM_ZEROINIT,
                               nTextLen * sizeof(TCHAR))))
      return (0);

   lpszTextTemp = (LPTSTR)tbGlobalLock(hText);
   StrnCpy(lpszTextTemp, lpszTextValue, nTextLen);
   tbGlobalUnlock(hText);

   return (hText);
   }

return (0);
}


#ifndef SS_NOCT_PIC

BOOL SS_UnFormatDataPic(lpCellType, lpszText, lpszTextValue)

LPSS_CELLTYPE lpCellType;
LPTSTR        lpszText;
LPTSTR        lpszTextValue;
{
LPTSTR        lpszPicMask;
BOOL          Ret;

if (!lpCellType->Spec.Pic.hMask)
   return (FALSE);

lpszPicMask = (LPTSTR)tbGlobalLock(lpCellType->Spec.Pic.hMask);

Ret = PicToValue(lpszText, lpszPicMask, lpszTextValue);

tbGlobalUnlock(lpCellType->Spec.Pic.hMask);
return (Ret);
}

#endif


#ifndef SS_NOCT_DATE

BOOL SS_UnFormatDataDate(LPSPREADSHEET lpSS, LPSS_CELLTYPE lpCellType,
                         LPTSTR lpszText, LPTSTR lpszTextValue)
{
TB_DATE Date;
BOOL    fRet = FALSE;

#ifndef SS_USEAWARE
if (DateStringIsValidEx(lpszText, &lpCellType->Spec.Date.Format,
                        lpSS->lpBook->nTwoDigitYearMax))
#endif
   if (DateStringToDMYEx(lpszText, &Date, &lpCellType->Spec.Date.Format,
                         lpSS->lpBook->nTwoDigitYearMax))
      // Modified to support JPN dates. (Masanori Iwasa)
      fRet = SS_DateToString(lpszTextValue, &Date, lpCellType->Spec.Date.Format);

return (fRet);
}

#endif


#ifndef SS_NOCT_TIME

BOOL SS_UnFormatDataTime(lpCellType, lpszText, lpszTextValue)

LPSS_CELLTYPE lpCellType;
LPTSTR        lpszText;
LPTSTR        lpszTextValue;
{
TIME          Time;
BOOL          fRet = FALSE;

if (TimeStringIsValid(lpszText, &lpCellType->Spec.Time.Format))
   if (TimeStringToHMS(lpszText, &Time, &lpCellType->Spec.Time.Format))
      fRet = SS_TimeToString(lpszTextValue, &Time);

return (fRet);
}

#endif


#ifndef SS_NOCT_COMBO

LRESULT DLLENTRY SSComboBoxSendMessage(hWnd, Col, Row, uMsg, wParam, lParam)

HWND          hWnd;
SS_COORD      Col;
SS_COORD      Row;
UINT          uMsg;
WPARAM        wParam;
LPARAM        lParam;
{
LPSPREADSHEET lpSS;
LRESULT       lRet = 0L;

lpSS = SS_SheetLock(hWnd);
SS_AdjustCellCoords(lpSS, &Col, &Row);

lRet = SS_ComboBoxSendMessage(lpSS, Col, Row, uMsg, wParam, lParam);

SS_SheetUnlock(hWnd);
return (lRet);
}


LRESULT SS_ComboBoxSendMessage(LPSPREADSHEET lpSS, SS_COORD Col,
                               SS_COORD Row, UINT uMsg, WPARAM wParam,
                               LPARAM lParam)
{
SS_CELLTYPE CellType;
LPSS_CELL   lpCell = NULL;
LPSS_COL    lpCol;
LPSS_ROW    lpRow;
LPTSTR      lpItems;
LRESULT     lRet = 0L;
BOOL        fSetCellType = FALSE;
BOOL        fChangeCellType = TRUE;
short       i;

// RFW - 4/4/00 - GEN_001_001
lpCol = SS_LockColItem(lpSS, Col);
lpRow = SS_LockRowItem(lpSS, Row);
if (Col != SS_ALLCOLS && Row != SS_ALLROWS)
   lpCell = SSx_LockCellItem(lpSS, lpRow, Col, Row);

if (SS_RetrieveCellType(lpSS, &CellType, lpCell, Col, Row) &&
    CellType.Type == SS_TYPE_COMBOBOX)
   {
   if (uMsg == CB_GETLBTEXT || uMsg == CB_GETLBTEXTLEN || uMsg == CB_GETCOUNT ||
       uMsg == CB_GETCURSEL || uMsg == CB_SETCURSEL)
      fChangeCellType = FALSE;

// RFW - 4/4/00 - GEN_001_001
   if (fChangeCellType &&
       ((Col != SS_ALLCOLS && Row != SS_ALLROWS && (!lpCell || !lpCell->hCellType)) ||
		  (Col != SS_ALLCOLS && Row == SS_ALLROWS && (!lpCol || !lpCol->hCellType)) ||
        (Col == SS_ALLCOLS && Row != SS_ALLROWS && (!lpRow || !lpRow->hCellType))))
      {
      SS_CELLTYPE CellTypeTemp;

      SS_CopyCellType(lpSS, &CellTypeTemp, &CellType);
      _fmemcpy(&CellType, &CellTypeTemp, sizeof(SS_CELLTYPE));
      fSetCellType = TRUE;
      }

   switch (uMsg)
      {
      case CB_ADDSTRING:
			lpSS->fComboListChanged = TRUE;
         lRet = (long)SS_ComboBoxInsert(&CellType, (LPTSTR)lParam, -1);
         break;

      case CB_DELETESTRING:
			lpSS->fComboListChanged = TRUE;
         lRet = (long)SS_ComboBoxDelete(&CellType, (short)wParam);
         break;

      case CB_GETLBTEXT:
         if (lParam && (short)wParam < CellType.Spec.ComboBox.dItemCnt &&
             CellType.Spec.ComboBox.dItemCnt && CellType.Spec.ComboBox.hItems)
            {
            lpItems = (LPTSTR)tbGlobalLock(CellType.Spec.ComboBox.hItems);
            for (i = 0; i < (short)wParam; i++)
               lpItems += lstrlen(lpItems) + 1;

            lRet = lstrlen(lpItems);
            lstrcpy((LPTSTR)lParam, lpItems);

            tbGlobalUnlock(CellType.Spec.ComboBox.hItems);
            }

         break;

      case CB_GETLBTEXTLEN:
         if ((short)wParam < CellType.Spec.ComboBox.dItemCnt &&
             CellType.Spec.ComboBox.dItemCnt && CellType.Spec.ComboBox.hItems)
            {
            lpItems = (LPTSTR)tbGlobalLock(CellType.Spec.ComboBox.hItems);
            for (i = 0; i < (short)wParam; i++)
               lpItems += lstrlen(lpItems) + 1;

            lRet = lstrlen(lpItems);

            tbGlobalUnlock(CellType.Spec.ComboBox.hItems);
            }

         break;

      case CB_INSERTSTRING:
			lpSS->fComboListChanged = TRUE;
         lRet = (long)SS_ComboBoxInsert(&CellType, (LPTSTR)lParam, (short)wParam);
         break;

      case CB_RESETCONTENT:
			lpSS->fComboListChanged = TRUE;
// Added by BOC FMH for a string remains after reset the combo cell 1996.07.10. ---------->>
#ifdef SPREAD_JPN
         SS_SetValue(lpSS, Col, Row, _T(""));
#endif
//--------------------<<
         CellType.Spec.ComboBox.dItemCnt = 0;

         if (CellType.Spec.ComboBox.hItems)
            tbGlobalFree(CellType.Spec.ComboBox.hItems);

         CellType.Spec.ComboBox.hItems = 0;

         lRet = 0;
         break;

      case SS_CBM_SETLIST:
//---------Delteted by dean 2000/02/16--------
//-To fix bug GEN_001_001, When you set Col and Row property to -1 to create
//- combobox cell, TypeComboBoxList property can not set items correctly.
         if (CellType.Spec.ComboBox.hItems)
            tbGlobalFree(CellType.Spec.ComboBox.hItems);
//---------------------------------------------

         CellType.Spec.ComboBox.dItemCnt = 0;
         CellType.Spec.ComboBox.hItems = 0;

			lpSS->fComboListChanged = TRUE;

         if (lParam)
            {
            TBGLOBALHANDLE hItems;
            LPTSTR         lpItemsTemp;
            LPTSTR         Ptr;
            short          dItemCnt;

            if (!(hItems = tbGlobalAlloc(GMEM_MOVEABLE | GMEM_ZEROINIT,
                                         (lstrlen((LPTSTR)lParam) + 1) * sizeof(TCHAR))))
               return (0);

            lpItemsTemp = (LPTSTR)tbGlobalLock(hItems);
            lstrcpy(lpItemsTemp, (LPTSTR)lParam);

            dItemCnt = 1;

            Ptr = lpItemsTemp;
            while (Ptr = _ftcschr(Ptr, '\t'))
               {
               *Ptr = '\0';
               Ptr++;

               if (*Ptr)
                  dItemCnt++;
               }

            CellType.Spec.ComboBox.dItemCnt = dItemCnt;
            CellType.Spec.ComboBox.hItems = hItems;

            tbGlobalUnlock(hItems);
            }

         break;

      case CB_GETCOUNT:
         lRet = CellType.Spec.ComboBox.dItemCnt;
         break;

      case CB_GETCURSEL:
         {
         TBGLOBALHANDLE hData;

         if (hData = SS_GetValue(lpSS, Col, Row))
            {
            LPTSTR lpData = (LPTSTR)tbGlobalLock(hData);

			if (*lpData)
			   lRet = StringToLong(lpData);
			else
			   lRet = -1;

            tbGlobalUnlock(hData);
            tbGlobalFree(hData);
            }
         else
            lRet = CB_ERR;
         }

         break;

      case CB_SETCURSEL:
         {
         TCHAR Buffer[10];

         LongToString((long)wParam, Buffer);
         if (!SS_SetDataRange(lpSS, Col, Row, Col, Row, Buffer, TRUE, FALSE, TRUE))
            lRet = -1;
         }

         break;

      }

//RWP01c
//--------*---------
// if (fSetCellType)
//    SS_SetCellTypeRange(lpSS, Col, Row, Col, Row, &CellType);
// else
//    SS_SetCellTypeStruct(lpSS, &CellType, lpCell, Col, Row);
//--------*---------
   if (fChangeCellType)
      {
      if (fSetCellType)
			{
			lpSS->lpBook->fLeaveEditModeOn = TRUE;
         SS_SetCellTypeRange(lpSS, Col, Row, Col, Row, &CellType);
			lpSS->lpBook->fLeaveEditModeOn = FALSE;
			}
      else
		//---------Added by dean 2000/02/16--------
		//-To fix bug GEN_001_001, When you set Col and Row property to -1 to create
		//- combobox cell, TypeComboBoxList property can not set items correctly.
//		if (Col == -1 || Row == -1)
//	       SS_SetCellTypeRange(lpSS, Col, Row, Col, Row, &CellType);
//		else
		//------------------------------------------
         SS_SetCellTypeStruct(lpSS, &CellType, lpCell, Col, Row);
      }
//RWP01c
   }

if (lpCell)
   SS_UnlockCellItem(lpSS, Col, Row);

return (lRet);
}


short SS_ComboBoxInsert(lpCellType, lpText, dItemNum)

LPSS_CELLTYPE  lpCellType;
LPTSTR         lpText;
short          dItemNum;
{
TBGLOBALHANDLE hItems = 0;
LPTSTR         lpItems;
LPTSTR         lpItemsNew;
long           lLen = 0;
long           lLenOld = 0;
long           lInsertPos = -1;
short          i;

if (dItemNum == -1)
   dItemNum = lpCellType->Spec.ComboBox.dItemCnt;

if (lpText && dItemNum <= lpCellType->Spec.ComboBox.dItemCnt)
   {
   lLen = lstrlen(lpText) + 1;

   if (lpCellType->Spec.ComboBox.hItems)
      {
      lpItems = (LPTSTR)tbGlobalLock(lpCellType->Spec.ComboBox.hItems);

      for (i = 0; i < lpCellType->Spec.ComboBox.dItemCnt; i++)
         {
         if (i == dItemNum)
            lInsertPos = lLenOld;

         lLenOld += lstrlen(&lpItems[lLenOld]) + 1;
         }

      if (lInsertPos == -1)
         lInsertPos = lLenOld;

      lLen += lLenOld;
      tbGlobalUnlock(lpCellType->Spec.ComboBox.hItems);
      }

#ifndef WIN32
   if (lLen > 65000)
      return (-1);
#endif

   if (!(hItems = tbGlobalAlloc(GMEM_MOVEABLE | GMEM_ZEROINIT,
                                lLen * sizeof(TCHAR))))
      return (-1);

   lpItemsNew = (LPTSTR)tbGlobalLock(hItems);
   
   if (lpCellType->Spec.ComboBox.hItems)
      {
      lpItems = (LPTSTR)tbGlobalLock(lpCellType->Spec.ComboBox.hItems);

      if (lInsertPos)
         _fmemcpy(lpItemsNew, lpItems, (size_t)lInsertPos * sizeof(TCHAR));

      _fmemcpy(&lpItemsNew[lInsertPos], lpText,
                (lstrlen(lpText) + 1) * sizeof(TCHAR));

      if (lInsertPos < lLenOld)
         _fmemcpy(&lpItemsNew[lInsertPos + lstrlen(lpText) + 1],
                  &lpItems[lInsertPos],
                  (size_t)(lLenOld - lInsertPos) * sizeof(TCHAR));

      tbGlobalUnlock(lpCellType->Spec.ComboBox.hItems);
      tbGlobalFree(lpCellType->Spec.ComboBox.hItems);
      }

   else
      lstrcpy(lpItemsNew, lpText);

   tbGlobalUnlock(hItems);
   lpCellType->Spec.ComboBox.hItems = hItems;
   lpCellType->Spec.ComboBox.dItemCnt++;
   }

return (dItemNum);
}


short SS_ComboBoxDelete(lpCellType, dItemNum)

LPSS_CELLTYPE lpCellType;
short         dItemNum;
{
TBGLOBALHANDLE hItems = 0;
LPTSTR        lpItems;
LPTSTR        lpItemsNew;
long          lLen = 0;
long          lLenOld = 0;
long          lPos1;
long          lPos2;
short         dRet = -1;
short         i;
//Modify by BOC 99.4.30 (hyt)--------------------------
//for remove item index less 0 
 //if (dItemNum < lpCellType->Spec.ComboBox.dItemCnt &&
 //    lpCellType->Spec.ComboBox.dItemCnt && lpCellType->Spec.ComboBox.hItems)
  if (dItemNum>=0 && dItemNum < lpCellType->Spec.ComboBox.dItemCnt &&
     lpCellType->Spec.ComboBox.dItemCnt && lpCellType->Spec.ComboBox.hItems)
//-----------------------------------------------------
   {
   lpItems = (LPTSTR)tbGlobalLock(lpCellType->Spec.ComboBox.hItems);

   for (i = 0; i < lpCellType->Spec.ComboBox.dItemCnt; i++)
      {
      if (i == dItemNum)
         lPos1 = lLenOld;

      lLenOld += lstrlen(&lpItems[lLenOld]) + 1;

      if (i == dItemNum)
         lPos2 = lLenOld;
      }

   if (lpCellType->Spec.ComboBox.dItemCnt > 1)
      {
      if (!(hItems = tbGlobalAlloc(GMEM_MOVEABLE | GMEM_ZEROINIT, (lLenOld -
                                   (lPos2 - lPos1)) * sizeof(TCHAR))))
         return (-1);

      lpItemsNew = (LPTSTR)tbGlobalLock(hItems);

      if (lPos1)
         _fmemcpy(lpItemsNew, lpItems, (short)lPos1 * sizeof(TCHAR));

      if (lPos2 < lLenOld)
         _fmemcpy(&lpItemsNew[lPos1], &lpItems[lPos2],
                  (short)(lLenOld - lPos2) * sizeof(TCHAR));

      tbGlobalUnlock(hItems);
      }

   tbGlobalUnlock(lpCellType->Spec.ComboBox.hItems);
   tbGlobalFree(lpCellType->Spec.ComboBox.hItems);

   lpCellType->Spec.ComboBox.hItems = hItems;

   lpCellType->Spec.ComboBox.dItemCnt--;
   dRet = lpCellType->Spec.ComboBox.dItemCnt;
   }

return (dRet);
}

#endif


#ifndef SS_NOCT_DATE

//------------------------------------------------------
// Note that last parameter was added to support
// JPN date formats. (Masanori Iwasa)
//------------------------------------------------------
BOOL SS_DateValidate(lpszText, lpDate, df)

LPCTSTR     lpszText;
LPTB_DATE   lpDate;
DATEFORMAT  df;
{
TCHAR     szBuffer[4 + 1];

   switch(df.nFormat)
      {
      case IDF_YYMM:
         if (lstrlen(lpszText) != 6 && lstrlen(lpszText) != 8)
            return (FALSE);
#ifdef SPREAD_JPN
         //----------------------------------------------------------
         // DO NO REMOVE THIS PREPROCESSOR. This is where the value
         // is handled backwords from the US version.
         //      US_VERSION : MMYYYY
         //      JP_VERSION : YYYYMM           (Masanori Iwasa)
         //----------------------------------------------------------
         _fmemcpy(szBuffer, &lpszText[0], 4 * sizeof(TCHAR));
         szBuffer[4] = 0;
         lpDate->nYear = StringToInt(szBuffer);

         _fmemcpy(szBuffer, &lpszText[4], 2 * sizeof(TCHAR));
         szBuffer[2] = 0;
         lpDate->nMonth = StringToInt(szBuffer);
#else
         if (lstrlen(lpszText) == 6)
         {
           _fmemcpy(szBuffer, &lpszText[0], 2 * sizeof(TCHAR));
           szBuffer[2] = 0;
           lpDate->nMonth = StringToInt(szBuffer);

           lpDate->nDay = 1;

           _fmemcpy(szBuffer, &lpszText[2], 4 * sizeof(TCHAR));
           szBuffer[4] = 0;
           lpDate->nYear = StringToInt(szBuffer);
         }
         else // 8
         {
           _fmemcpy(szBuffer, &lpszText[0], 2 * sizeof(TCHAR));
           szBuffer[2] = 0;
           lpDate->nMonth = StringToInt(szBuffer);

           _fmemcpy(szBuffer, &lpszText[2], 2 * sizeof(TCHAR));
           szBuffer[2] = 0;
           lpDate->nDay = StringToInt(szBuffer);

           _fmemcpy(szBuffer, &lpszText[4], 4 * sizeof(TCHAR));
           szBuffer[4] = 0;
           lpDate->nYear = StringToInt(szBuffer);
         }

#endif
         lpDate->nDay = 1;

         break;
         
      case IDF_MMDD:
         if (lstrlen(lpszText) != 4 && lstrlen(lpszText) != 8)
            return (FALSE);
         SysGetDate (&lpDate->nDay, &lpDate->nMonth, &lpDate->nYear);

         if (lstrlen(lpszText) == 4)
         {  
           //----------------------------------------------------------
           // Both the US & JPN Version will be the same.
           //      US_VERSION : MMDD
           //      JP_VERSION : MMDD              (Masanori Iwasa)
           //----------------------------------------------------------
           _fmemcpy(szBuffer, &lpszText[0], 2 * sizeof(TCHAR));
           szBuffer[2] = 0;
           lpDate->nMonth = StringToInt(szBuffer);

           _fmemcpy(szBuffer, &lpszText[2], 2 * sizeof(TCHAR));
           szBuffer[2] = 0;
           lpDate->nDay = StringToInt(szBuffer);
          }
          else
          {
#ifdef SPREAD_JPN
         _fmemcpy(szBuffer, &lpszText[0], 4 * sizeof(TCHAR));
         szBuffer[4] = 0;
         lpDate->nYear = StringToInt(szBuffer);

         _fmemcpy(szBuffer, &lpszText[4], 2 * sizeof(TCHAR));
         szBuffer[2] = 0;
         lpDate->nMonth = StringToInt(szBuffer);

         _fmemcpy(szBuffer, &lpszText[6], 2 * sizeof(TCHAR));
         szBuffer[2] = 0;
         lpDate->nDay = StringToInt(szBuffer);
#else
         _fmemcpy(szBuffer, &lpszText[0], 2 * sizeof(TCHAR));
         szBuffer[2] = 0;
         lpDate->nMonth = StringToInt(szBuffer);

         _fmemcpy(szBuffer, &lpszText[2], 2 * sizeof(TCHAR));
         szBuffer[2] = 0;
         lpDate->nDay = StringToInt(szBuffer);

         _fmemcpy(szBuffer, &lpszText[4], 4 * sizeof(TCHAR));
         szBuffer[4] = 0;
         lpDate->nYear = StringToInt(szBuffer);
#endif
          }
         break;

      default:
         if (lstrlen(lpszText) != 8) 
            return (FALSE);

#ifdef SPREAD_JPN
         //----------------------------------------------------------
         // DO NO REMOVE THIS PREPROCESSOR. This is where the value
         // is handled backwords from the US version.
         //      US_VERSION : MMDDYYYY
         //      JP_VERSION : YYYYMMDD            (Masanori Iwasa)
         //----------------------------------------------------------
         _fmemcpy(szBuffer, &lpszText[0], 4 * sizeof(TCHAR));
         szBuffer[4] = 0;
         lpDate->nYear = StringToInt(szBuffer);

         _fmemcpy(szBuffer, &lpszText[4], 2 * sizeof(TCHAR));
         szBuffer[2] = 0;
         lpDate->nMonth = StringToInt(szBuffer);

         _fmemcpy(szBuffer, &lpszText[6], 2 * sizeof(TCHAR));
         szBuffer[2] = 0;
         lpDate->nDay = StringToInt(szBuffer);
#else
         _fmemcpy(szBuffer, &lpszText[0], 2 * sizeof(TCHAR));
         szBuffer[2] = 0;
         lpDate->nMonth = StringToInt(szBuffer);

         _fmemcpy(szBuffer, &lpszText[2], 2 * sizeof(TCHAR));
         szBuffer[2] = 0;
         lpDate->nDay = StringToInt(szBuffer);

         _fmemcpy(szBuffer, &lpszText[4], 4 * sizeof(TCHAR));
         szBuffer[4] = 0;
         lpDate->nYear = StringToInt(szBuffer);
#endif
      }
return (SS_DateIsValid(lpDate->nMonth, lpDate->nDay, lpDate->nYear));
}


//------------------------------------------------------
// Note that last parameter was added to support
// JPN date formats. (Masanori Iwasa)
//------------------------------------------------------
BOOL SS_DateToString(lpszText, lpDate, df)

LPTSTR      lpszText;
LPTB_DATE   lpDate;
DATEFORMAT  df;
{
   switch(df.nFormat)
      {
       case IDF_YYMM:
#ifdef SPREAD_JPN
         //----------------------------------------------------------
         // DO NO REMOVE THIS PREPROCESSOR. This is where the value
         // is handled backwords from the US version.
         //      US_VERSION : MMYYYY
         //      JP_VERSION : YYYYMM           (Masanori Iwasa)
         //----------------------------------------------------------
          wsprintf(lpszText, _T("%04d%02d"), lpDate->nYear, lpDate->nMonth);
#else
          wsprintf(lpszText, _T("%02d01%04d"), lpDate->nMonth, lpDate->nYear);
#endif
          break;

       case IDF_MMDD:
         //----------------------------------------------------------
         // Both the US & JPN Version will be the same.
         //      US_VERSION : MMDD
         //      JP_VERSION : MMDD              (Masanori Iwasa)
         //----------------------------------------------------------
#ifdef SPREAD_JPN
          wsprintf(lpszText, _T("%02d%02d"), lpDate->nMonth, lpDate->nDay);
#else
        {  
          int day, month, year;
          SysGetDate (&day, &month, &year);
          wsprintf(lpszText, _T("%02d%02d%04d"), lpDate->nMonth, lpDate->nDay, year);
        }
#endif
          break;

       default:
#ifdef SPREAD_JPN
         //----------------------------------------------------------
         // DO NO REMOVE THIS PREPROCESSOR. This is where the value
         // is handled backwords from the US version.
         //      US_VERSION : MMDDYYYY
         //      JP_VERSION : YYYYMMDD            (Masanori Iwasa)
         //----------------------------------------------------------
          wsprintf(lpszText, _T("%04d%02d%02d"), lpDate->nYear, lpDate->nMonth, lpDate->nDay);
#else
          wsprintf(lpszText, _T("%02d%02d%04d"), lpDate->nMonth, lpDate->nDay, lpDate->nYear);
#endif
      }
return (TRUE);
}


int SS_DateIsLeapYear(int Year)
{
if (((Year % 4) == 0 && (Year % 100) != 0) ||
    ((Year % 100) == 0 && (Year % 400) == 0))
   return (TRUE);
else
   return (FALSE);
}


int SS_DateDaysPerMonth(int Month, int Year)
{
static int DaysPerMonth[2][12] =
   {
   {31, 28, 31, 30, 31, 30, 31, 31, 30, 31, 30, 31},
   {31, 29, 31, 30, 31, 30, 31, 31, 30, 31, 30, 31}
   };

if (Month <= 0 || Month > 12)
   return (0);

if (Year < 1900)
   Year += Year >= 50 ? 1900 : 2000;

return (DaysPerMonth[SS_DateIsLeapYear(Year)][Month - 1]);
}


BOOL SS_DateIsValid(int Month, int Day, int Year)
{

if (Year <= 0 || Year > HIYEAR ||
    Month < 1 || Month > 12 ||  
    Day < 1 || Day > SS_DateDaysPerMonth(Month, Year))
   return (FALSE);

return (TRUE);
}

#endif


#ifndef SS_NOCT_TIME

BOOL SS_TimeValidate(lpszText, lpTime)

LPCTSTR lpszText;
LPTIME  lpTime;
{
TCHAR   szBuffer[2 + 1];

if (lstrlen(lpszText) != 4 && lstrlen(lpszText) != 6)
   return (FALSE);

szBuffer[2] = 0;

_fmemcpy(szBuffer, &lpszText[0], 2 * sizeof(TCHAR));
lpTime->nHour = StringToInt(szBuffer);

if (lpTime->nHour < 0 || lpTime->nHour > 23)
   return (FALSE);

_fmemcpy(szBuffer, &lpszText[2], 2 * sizeof(TCHAR));
lpTime->nMinute = StringToInt(szBuffer);

if (lpTime->nMinute < 0 || lpTime->nMinute > 59)
   return (FALSE);

if (lstrlen(lpszText) == 6)
   {
   _fmemcpy(szBuffer, &lpszText[4], 2 * sizeof(TCHAR));
   lpTime->nSecond = StringToInt(szBuffer);

   if (lpTime->nSecond < 0 || lpTime->nSecond > 59)
      return (FALSE);
   }

return (TRUE);
}


BOOL SS_TimeToString(lpszText, lpTime)

LPTSTR lpszText;
LPTIME lpTime;
{
wsprintf(lpszText, _T("%02d%02d%02d"), lpTime->nHour, lpTime->nMinute,
         lpTime->nSecond);

return (TRUE);
}

#endif


short SS_TypeGetControlType(LPSPREADSHEET lpSS, short CtrlID)
{
LPSS_CONTROL lpControls;
short        Type = -1;
short        i;

lpControls = (LPSS_CONTROL)tbGlobalLock(lpSS->lpBook->hControls);

for (i = 0; i < lpSS->lpBook->ControlsCnt; i++)
   if (CtrlID == lpControls[i].CtrlID)
      {
      Type = lpControls[i].Type;
      break;
      }

tbGlobalUnlock(lpSS->lpBook->hControls);

return (Type);
}


HWND SS_GetControlhWnd(LPSPREADSHEET lpSS, short CtrlID)
{
LPSS_CONTROL lpControls;
HWND         hWnd;

lpControls = (LPSS_CONTROL)tbGlobalLock(lpSS->lpBook->hControls);
#if defined(SS_V80) && defined(SS_OCX)
//{
//   short dIndex;
//
//   for( dIndex = 0; dIndex < lpSS->lpBook->ControlsCnt; dIndex++ )
//   {
//      if( lpControls[dIndex].CtrlID == CtrlID )
//      {
//         hWnd = lpControls[dIndex].hWnd;
//         break;
//      }
//   }
//}
//#else
if( lpControls[CtrlID - SS_CONTROL_BASEID].ldisp )
{
   FPCONTROL ctl = {0};

   ctl.lpDispatch = lpControls[CtrlID - SS_CONTROL_BASEID].ldisp;
   hWnd = fpVBGetControlHwnd(&ctl);
   if( hWnd == NULL )
   {
      fpVBRecreateControlHwnd(&ctl);
      hWnd = (HWND)fpVBGetControlHwnd(&ctl);
   }
   lpControls[CtrlID - SS_CONTROL_BASEID].hWnd = hWnd;
}
else
#endif
hWnd = lpControls[CtrlID - SS_CONTROL_BASEID].hWnd;
//#endif
tbGlobalUnlock(lpSS->lpBook->hControls);

return (hWnd);
}

#if SS_V80
FARPROC SS_GetControlWndProc(LPSPREADSHEET lpSS, HWND hWnd)
{
LPSS_CONTROL lpControls;
FARPROC wndProc = NULL;
int          n;

lpControls = (LPSS_CONTROL)tbGlobalLock(lpSS->lpBook->hControls);
for( n = 0; n < lpSS->lpBook->ControlsCnt; n++ )
   if( lpControls[n].hWnd == hWnd && lpControls[n].wndProc != 0 )
   {
      wndProc = lpControls[n].wndProc;
      break;
   }
tbGlobalUnlock(lpSS->lpBook->hControls);

return (wndProc);
}

void SS_SetControlWndProc(LPSPREADSHEET lpSS, HWND hWnd, FARPROC wndProc)
{
LPSS_CONTROL lpControls;
int          n;

lpControls = (LPSS_CONTROL)tbGlobalLock(lpSS->lpBook->hControls);
for( n = 0; n < lpSS->lpBook->ControlsCnt; n++ )
   if( lpControls[n].hWnd == hWnd )
   {
      lpControls[n].wndProc = wndProc;
      break;
   }
tbGlobalUnlock(lpSS->lpBook->hControls);
}

#if SS_OCX
long SS_GetControlDispatchFromHwnd(LPSPREADSHEET lpSS, HWND hWnd)
{
LPSS_CONTROL lpControls;
long         ldisp = 0;
int          n;

lpControls = (LPSS_CONTROL)tbGlobalLock(lpSS->lpBook->hControls);
for( n = 0; n < lpSS->lpBook->ControlsCnt; n++ )
   if( lpControls[n].hWnd == hWnd )
   {
      ldisp = lpControls[n].ldisp;
      break;
   }
tbGlobalUnlock(lpSS->lpBook->hControls);

return ldisp;
}

long SS_GetControlDispatch(LPSPREADSHEET lpSS, short CtrlID)
{
LPSS_CONTROL lpControls;
long         ldisp = 0;
//int          n;

lpControls = (LPSS_CONTROL)tbGlobalLock(lpSS->lpBook->hControls);
/*
for( n = 0; n < lpSS->lpBook->ControlsCnt; n++ )
   if( lpControls[n].CtrlID == CtrlID )
   {
      ldisp = lpControls[n].ldisp;
      break;
   }
*/
ldisp = lpControls[CtrlID - SS_CONTROL_BASEID].ldisp;
tbGlobalUnlock(lpSS->lpBook->hControls);

return ldisp;
}

void SS_UpdateControlHwnd(LPSPREADSHEET lpSS, short CtrlID, HWND hWndNew)
{
LPSS_CONTROL lpControls;
long         ldisp = 0;
//int          n;

lpControls = (LPSS_CONTROL)tbGlobalLock(lpSS->lpBook->hControls);
lpControls[CtrlID - SS_CONTROL_BASEID].hWnd = hWndNew;
tbGlobalUnlock(lpSS->lpBook->hControls);
}

void SS_CT_UpdateControlHwnd(LPSPREADSHEET lpSS, LPSS_CELLTYPE CellType)
{
   HWND hWndNew;
   long ldisp;
   FPCONTROL ctl = {0};

   if( CellType->Type == SS_TYPE_CUSTOM )
   {
      if( CellType->ControlID )
      {
         ldisp = SS_GetControlDispatch(lpSS, CellType->ControlID);
         if( ldisp )
         {
            ctl.lpDispatch = ldisp;
            hWndNew = (HWND)fpVBGetControlHwnd(&ctl);
            SS_UpdateControlHwnd(lpSS, CellType->ControlID, hWndNew);
         }
      }

      if( CellType->Spec.Custom.RendererControlID  )
      {
         ldisp = SS_GetControlDispatch(lpSS, CellType->Spec.Custom.RendererControlID);
         if( ldisp )
         {
            ctl.lpDispatch = ldisp;
            hWndNew = (HWND)fpVBGetControlHwnd(&ctl);
            SS_UpdateControlHwnd(lpSS, CellType->Spec.Custom.RendererControlID, hWndNew);
         }
      }
   }
}
#endif

HWND SS_CT_GetControlhWnd(LPSPREADSHEET lpSS, CT_HANDLE hCT, long lStyle, BOOL fRenderer)
{
LPSS_CONTROL lpControls;
HWND hWnd = NULL;
int n;

lpControls = (LPSS_CONTROL)tbGlobalLock(lpSS->lpBook->hControls);
for( n = 0; n < lpSS->lpBook->ControlsCnt; n++ )
   if( lpControls[n].hCT == hCT && lpControls[n].Style == lStyle && lpControls[n].fRenderer == fRenderer )
   {
      hWnd = lpControls[n].hWnd;
      break;
   }
tbGlobalUnlock(lpSS->lpBook->hControls);
if( hWnd == NULL )
{  // force creation
  SS_CELLTYPE ct;
  LPCTSTR lpszName = CustCellTypeLockText(hCT);
  short id;

  SS_SetTypeCustom(lpSS, &ct, lpszName, lStyle, NULL);
  id = SS_CreateControl(lpSS, &ct, fRenderer);
  if( id )
  {
    lpControls = (LPSS_CONTROL)tbGlobalLock(lpSS->lpBook->hControls);
    for( n = 0; n < lpSS->lpBook->ControlsCnt; n++ )
      if( lpControls[n].hCT == hCT && lpControls[n].Style == lStyle && lpControls[n].fRenderer == fRenderer )
      {
         hWnd = lpControls[n].hWnd;
         break;
      }
    tbGlobalUnlock(lpSS->lpBook->hControls);
  }
  CustCellTypeUnlockText(hCT);
}
return hWnd;
}

BOOL SS_CT_IsEditorControl(LPSPREADSHEET lpSS, HWND hWnd)
{
   LPSS_CONTROL lpControls = (LPSS_CONTROL)tbGlobalLock(lpSS->lpBook->hControls);
   int n;

   for( n = 0; n < lpSS->lpBook->ControlsCnt; n++ )
   {
      if( lpControls[n].hCT && lpControls[n].hWnd == hWnd && !lpControls[n].fRenderer )
         return TRUE;
   }
   return FALSE;
}
#endif

HWND SS_RetrieveControlhWnd(LPSPREADSHEET lpSS, SS_COORD Col,
                            SS_COORD Row)
{
TBGLOBALHANDLE hCellType = 0;
LPSS_CELLTYPE  lpCellType = NULL;
LPSS_CELL      lpCell;
LPSS_COL       lpCol;
LPSS_ROW       lpRow;
HWND           hWndCtrl;


lpCol = SS_LockColItem(lpSS, Col);
lpRow = SS_LockRowItem(lpSS, Row);
lpCell = SSx_LockCellItem(lpSS, lpRow, Col, Row);

if (lpCell && lpCell->hCellType && Col != SS_ALLCOLS &&
    Row != SS_ALLROWS)
   hCellType = lpCell->hCellType;

else if (Row != SS_ALLROWS && lpRow && Row < lpSS->Row.HeaderCnt &&
         lpRow->hCellType)
   hCellType = lpRow->hCellType;

else if (Col != SS_ALLCOLS && lpCol && lpCol->hCellType)
   hCellType = lpCol->hCellType;

else if (Row != SS_ALLROWS && lpRow && lpRow->hCellType)
   hCellType = lpRow->hCellType;

else
   lpCellType = &lpSS->DefaultCellType;

SS_UnlockCellItem(lpSS, Col, Row);
SS_UnlockColItem(lpSS, Col);
SS_UnlockRowItem(lpSS, Row);

if (hCellType || lpCellType)
   {
   if (hCellType)
      lpCellType = (LPSS_CELLTYPE)tbGlobalLock(hCellType);

   if (!lpCellType->ControlID)
      if (!SS_CreateControl(lpSS, lpCellType, FALSE))
         return (NULL);

   hWndCtrl = SS_GetControlhWnd(lpSS, lpCellType->ControlID);

   if (hCellType)
      tbGlobalUnlock(hCellType);
   }

return (hWndCtrl);
}

#if SS_V80
LPSS_CELLTYPE SS_CT_LockCellType(LPSPREADSHEET lpSS, SS_COORD Col, SS_COORD Row)
{
   LPSS_CELLTYPE  lpCellType = NULL;
   TBGLOBALHANDLE hCellType = 0;
   LPSS_CELL      lpCell;
   LPSS_COL       lpCol;
   LPSS_ROW       lpRow;


   lpCol = SS_LockColItem(lpSS, Col);
   lpRow = SS_LockRowItem(lpSS, Row);
   lpCell = SSx_LockCellItem(lpSS, lpRow, Col, Row);

   if (lpCell && lpCell->hCellType && Col != SS_ALLCOLS &&
       Row != SS_ALLROWS)
      hCellType = lpCell->hCellType;

   else if (Row != SS_ALLROWS && lpRow && Row < lpSS->Row.HeaderCnt &&
            lpRow->hCellType)
      hCellType = lpRow->hCellType;

   else if (Col != SS_ALLCOLS && lpCol && lpCol->hCellType)
      hCellType = lpCol->hCellType;

   else if (Row != SS_ALLROWS && lpRow && lpRow->hCellType)
      hCellType = lpRow->hCellType;

   else
      lpCellType = &lpSS->DefaultCellType;

   SS_UnlockCellItem(lpSS, Col, Row);
   SS_UnlockColItem(lpSS, Col);
   SS_UnlockRowItem(lpSS, Row);

   if (hCellType)
      lpCellType = (LPSS_CELLTYPE)tbGlobalLock(hCellType);

   return lpCellType;
}
void SS_CT_UnlockCellType(LPSPREADSHEET lpSS, SS_COORD Col, SS_COORD Row)
{
   TBGLOBALHANDLE hCellType = 0;
   LPSS_CELL      lpCell;
   LPSS_COL       lpCol;
   LPSS_ROW       lpRow;


   lpCol = SS_LockColItem(lpSS, Col);
   lpRow = SS_LockRowItem(lpSS, Row);
   lpCell = SSx_LockCellItem(lpSS, lpRow, Col, Row);

   if (lpCell && lpCell->hCellType && Col != SS_ALLCOLS &&
       Row != SS_ALLROWS)
      hCellType = lpCell->hCellType;

   else if (Row != SS_ALLROWS && lpRow && Row < lpSS->Row.HeaderCnt &&
            lpRow->hCellType)
      hCellType = lpRow->hCellType;

   else if (Col != SS_ALLCOLS && lpCol && lpCol->hCellType)
      hCellType = lpCol->hCellType;

   else if (Row != SS_ALLROWS && lpRow && lpRow->hCellType)
      hCellType = lpRow->hCellType;

   SS_UnlockCellItem(lpSS, Col, Row);
   SS_UnlockColItem(lpSS, Col);
   SS_UnlockRowItem(lpSS, Row);

   if (hCellType)
      tbGlobalUnlock(hCellType);
}
HWND SS_GetControlRenderer(LPSPREADSHEET lpSS, LPSS_CELLTYPE CellType)
{
   HWND hWndCtrl = NULL;
   if( CellType && CellType->Type == SS_TYPE_CUSTOM )
   {
      LPTSTR lpszName = SS_CT_LockName(CellType);
      BOOL bUsingRendererControl = FALSE;

      SS_CustCellTypeLookup(lpSS->lpBook, lpszName, NULL, NULL, NULL, &bUsingRendererControl, NULL);
      SS_CT_UnlockName(CellType);
      if( !bUsingRendererControl )
        return NULL;

      if (!CellType->Spec.Custom.RendererControlID)
         if (!SS_CreateControl(lpSS, CellType, TRUE))
            return (NULL);

      hWndCtrl = SS_GetControlhWnd(lpSS, CellType->Spec.Custom.RendererControlID);
   }
   return hWndCtrl;
}
/*
HWND SS_RetrieveControlRendererhWnd(LPSPREADSHEET lpSS, SS_COORD Col,
                            SS_COORD Row)
{
TBGLOBALHANDLE hCellType = 0;
LPSS_CELLTYPE  lpCellType = NULL;
LPSS_CELL      lpCell;
LPSS_COL       lpCol;
LPSS_ROW       lpRow;
HWND           hWndCtrl = NULL;


lpCol = SS_LockColItem(lpSS, Col);
lpRow = SS_LockRowItem(lpSS, Row);
lpCell = SSx_LockCellItem(lpSS, lpRow, Col, Row);

if (lpCell && lpCell->hCellType && Col != SS_ALLCOLS &&
    Row != SS_ALLROWS)
   hCellType = lpCell->hCellType;

else if (Row != SS_ALLROWS && lpRow && Row < lpSS->Row.HeaderCnt &&
         lpRow->hCellType)
   hCellType = lpRow->hCellType;

else if (Col != SS_ALLCOLS && lpCol && lpCol->hCellType)
   hCellType = lpCol->hCellType;

else if (Row != SS_ALLROWS && lpRow && lpRow->hCellType)
   hCellType = lpRow->hCellType;

else
   lpCellType = &lpSS->DefaultCellType;

SS_UnlockCellItem(lpSS, Col, Row);
SS_UnlockColItem(lpSS, Col);
SS_UnlockRowItem(lpSS, Row);

if (hCellType || lpCellType)
   {
      if (hCellType)
         lpCellType = (LPSS_CELLTYPE)tbGlobalLock(hCellType);

      if( lpCellType->Type == SS_TYPE_CUSTOM )
      {
         LPTSTR lpszName = SS_CT_LockName(lpCellType);
         BOOL bUsingRendererControl = FALSE;

         SS_CustCellTypeLookup(lpSS->lpBook, lpszName, NULL, NULL, NULL, &bUsingRendererControl, NULL);
         SS_CT_UnlockName(lpCellType);
         if( !bUsingRendererControl )
           return NULL;

         if (!lpCellType->Spec.Custom.RendererControlID)
            if (!SS_CreateControl(lpSS, lpCellType, TRUE))
               return (NULL);

         hWndCtrl = SS_GetControlhWnd(lpSS, lpCellType->Spec.Custom.RendererControlID);

         if (hCellType)
            tbGlobalUnlock(hCellType);
      }
   }

return (hWndCtrl);
}
*/
#endif

static long SS_TranslateEditStyle(long lStyle)
{
if (lStyle & SSS_ALIGN_LEFT)
   lStyle |= ES_LEFTALIGN;
else if (lStyle & SSS_ALIGN_RIGHT)
   lStyle |= ES_RIGHT;
else if (lStyle & SSS_ALIGN_CENTER)
   lStyle |= ES_CENTER;

return (lStyle);
}

long SS_GetCellAlign(LPSS_CELLTYPE lpCellType)
{
long lAlign = lpCellType->Style & (SSS_ALIGN_TOP | SSS_ALIGN_VCENTER |
                                   SSS_ALIGN_BOTTOM | SSS_ALIGN_LEFT |
                                   SSS_ALIGN_CENTER | SSS_ALIGN_RIGHT);

switch (lpCellType->Type)
   {
   case SS_TYPE_STATICTEXT:
      if (lpCellType->Style & SS_TEXT_RIGHT)
         lAlign |= SSS_ALIGN_RIGHT;

      else if (lpCellType->Style & SS_TEXT_CENTER)
         lAlign |= SSS_ALIGN_CENTER;

      if (lpCellType->Style & SS_TEXT_BOTTOM)
         lAlign |= SSS_ALIGN_BOTTOM;

      else if (lpCellType->Style & SS_TEXT_VCENTER)
         lAlign |= SSS_ALIGN_VCENTER;

      break;

   case SS_TYPE_PICTURE:
   case SS_TYPE_BUTTON:
   case SS_TYPE_CHECKBOX:
   case SS_TYPE_COMBOBOX:              // OK
      break;

   case SS_TYPE_EDIT:
   case SS_TYPE_PIC:
   case SS_TYPE_DATE:
   case SS_TYPE_TIME:
   case SS_TYPE_INTEGER:
   case SS_TYPE_FLOAT:
#ifdef SS_V40
   case SS_TYPE_CURRENCY:
   case SS_TYPE_NUMBER:
   case SS_TYPE_PERCENT:
#endif // SS_V40
#ifdef SS_V70
	case SS_TYPE_SCIENTIFIC:
		break;
#endif // SS_V70
      if (lpCellType->Style & ES_RIGHT)
         lAlign |= SSS_ALIGN_RIGHT;
      else if (lpCellType->Style & ES_CENTER)
         lAlign |= SSS_ALIGN_CENTER;
      else if (lpCellType->Style & ES_LEFTALIGN)
         lAlign |= SSS_ALIGN_LEFT;
      else if (lpCellType->Type == SS_TYPE_INTEGER ||
               SS_IsCellTypeFloatNum(lpCellType))
         lAlign |= SSS_ALIGN_RIGHT;

      break;
   }

return (lAlign);
}


void SS_StringToNum(LPSPREADSHEET lpSS, LPSS_CELLTYPE lpCellType, LPCTSTR lpszText, LPDOUBLE lpdfVal)
{
#ifdef SS_V40
if (SS_IsCellTypeNum(lpCellType))
	{
	SSNUM_FORMAT Format;

	switch (lpCellType->Type)
		{
		case SS_TYPE_CURRENCY:
			SS_CreateCurrencyFormatStruct(lpCellType, &Format);
			break;

		case SS_TYPE_NUMBER:
			SS_CreateNumberFormatStruct(lpCellType, &Format);
			break;

		case SS_TYPE_PERCENT:
			SS_CreatePercentFormatStruct(lpCellType, &Format);
			break;
		}

	NumStringToValue(&Format, lpszText, lpdfVal);
	}

else
#endif // SS_V40
	SS_StringToFloat(lpSS, lpszText, lpdfVal);
}


void SS_StringToFloat(LPSPREADSHEET lpSS, LPCTSTR nptr, LPDOUBLE lpdfVal)
{
SS_FLOATFORMAT ff;
struct lconv  *plconv;
TCHAR cvtb[100];

// fix for 14467 -scl
// (moved down below)
//#if defined(_UNICODE)
//wcstombs((char*)cvtb, nptr, StrLen(nptr));
//#else
lstrcpyn(cvtb, nptr, 99);
//#endif

SS_GetDefFloatFormat(lpSS->lpBook, &ff);

// RFW - 4/25/00 - SCS9645
SS_FloatStringStrip(lpSS, NULL, cvtb);

if (ff.cDecimalSign && ff.cDecimalSign != '.')
   SS_StrReplaceCh(cvtb, ff.cDecimalSign, '.');

plconv = localeconv();

if (plconv->decimal_point[0] != 0 && plconv->decimal_point[0] != '.')
   SS_StrReplaceCh(cvtb, '.', plconv->decimal_point[0]);

// fix for 14467 (cont.) -scl
#if defined(_UNICODE)
// RFW - 4/17/05 fix for 15975
//wcstombs((char*)cvtb, nptr, StrLen(nptr));
// RFW - 5/18/05 - 16256
{
TCHAR cvtb2[100];
lstrcpy(cvtb2, cvtb);
wcstombs((char*)cvtb, cvtb2, 100);
}
// fix for 15054 -scl
//((char*)cvtb)[StrLen(nptr)] = 0;
#endif

cvtb[99] = '\0';
*lpdfVal = atof((char*)cvtb);
}


#ifdef SS_V40

void SS_CreateCurrencyFormatStruct(LPSS_CELLTYPE lpCellType, LPSSNUM_FORMAT lpFormat)
{
_fmemset(lpFormat, '\0', sizeof(SSNUM_FORMAT));

lstrcpy(lpFormat->szDecimal, lpCellType->Spec.Currency.szDecimal);
lstrcpy(lpFormat->szSeparator, lpCellType->Spec.Currency.szSeparator);
lstrcpy(lpFormat->szCurrency, lpCellType->Spec.Currency.szCurrency);
lpFormat->bLeadingZero = lpCellType->Spec.Currency.bLeadingZero;
lpFormat->bFractionalWidth = lpCellType->Spec.Currency.Right;
lpFormat->bNegCurrencyStyle = lpCellType->Spec.Currency.bNegStyle;
lpFormat->bPosCurrencyStyle = lpCellType->Spec.Currency.bPosStyle;
lpFormat->fCurrency = TRUE;
lpFormat->fShowSeparator = lpCellType->Spec.Currency.fShowSeparator;
lpFormat->fShowCurrencySymbol = lpCellType->Spec.Currency.fShowCurrencySymbol;
lpFormat->fSpin = lpCellType->Spec.Currency.fSpin;
lpFormat->fSpinWrap = lpCellType->Spec.Currency.fSpinWrap;
lpFormat->dfSpinInc = lpCellType->Spec.Currency.SpinInc;
}


void SS_CreateNumberFormatStruct(LPSS_CELLTYPE lpCellType, LPSSNUM_FORMAT lpFormat)
{
_fmemset(lpFormat, '\0', sizeof(SSNUM_FORMAT));

lstrcpy(lpFormat->szDecimal, lpCellType->Spec.Number.szDecimal);
lstrcpy(lpFormat->szSeparator, lpCellType->Spec.Number.szSeparator);
lpFormat->bLeadingZero = lpCellType->Spec.Number.bLeadingZero;
lpFormat->bFractionalWidth = lpCellType->Spec.Number.Right;
lpFormat->bNegStyle = lpCellType->Spec.Number.bNegStyle;
lpFormat->fShowSeparator = lpCellType->Spec.Number.fShowSeparator;
lpFormat->fSpin = lpCellType->Spec.Number.fSpin;
lpFormat->fSpinWrap = lpCellType->Spec.Number.fSpinWrap;
lpFormat->dfSpinInc = lpCellType->Spec.Number.SpinInc;
}


void SS_CreatePercentFormatStruct(LPSS_CELLTYPE lpCellType, LPSSNUM_FORMAT lpFormat)
{
SSNUM_INTLSETTINGS IntlSettings;
BYTE bNegCurrencyStyle;
BYTE bPosCurrencyStyle = 2;

_fmemset(lpFormat, '\0', sizeof(SSNUM_FORMAT));

// Turn the Percent styles into Currency styles
switch (lpCellType->Spec.Percent.bNegStyle)
	{
	case 0: // 0 - Default
		{
		NumGetSystemIntlSettings(&IntlSettings);
		switch (IntlSettings.bNegStyle)
			{
			case 0: // (1.1)
				bNegCurrencyStyle = 5; break;
			case 1: // -1.1
				bNegCurrencyStyle = 6; break;
			case 2: // - 1.1
				bNegCurrencyStyle = 6; break;
			case 3: // 1.1-
				bNegCurrencyStyle = 7; break;
			case 4: // 1.1 -
				bNegCurrencyStyle = 7; break;
			}
		break;
		}
	case 1: // 1 - (1.1%)
		bNegCurrencyStyle = 5; break;
	case 2: // 2 - -1.1%
		bNegCurrencyStyle = 6; break;
	case 3: // 3 - 1.1-%
		bNegCurrencyStyle = 7; break;
	case 4: // 4 - 1.1%-
		bNegCurrencyStyle = 8; break;
	case 5: // 5 - -1.1 %
		bPosCurrencyStyle = 4;
		bNegCurrencyStyle = 9; break;
	case 6: // 6 - 1.1 %-
		bPosCurrencyStyle = 4;
		bNegCurrencyStyle = 11; break;
	case 7: // 7 - 1.1- %
		bPosCurrencyStyle = 4;
		bNegCurrencyStyle = 14; break;
	case 8: // 8 - (1.1 %)
		bPosCurrencyStyle = 4;
		bNegCurrencyStyle = 16; break;
	default:
		bNegCurrencyStyle = 6; break;
	}

if (*lpCellType->Spec.Percent.szDecimal)
	lstrcpy(lpFormat->szDecimal, lpCellType->Spec.Percent.szDecimal);
else
	{
	NumGetSystemIntlSettings(&IntlSettings);
	lstrcpy(lpFormat->szDecimal, IntlSettings.decimal_point);
	}

lstrcpy(lpFormat->szCurrency, _T("%"));

lpFormat->bLeadingZero = lpCellType->Spec.Percent.bLeadingZero;

lpFormat->bFractionalWidth = lpCellType->Spec.Percent.Right;
lpFormat->bNegCurrencyStyle = bNegCurrencyStyle;
lpFormat->bPosCurrencyStyle = bPosCurrencyStyle;
lpFormat->fCurrency = TRUE;
lpFormat->fShowCurrencySymbol = TRUE;
lpFormat->fSpin = lpCellType->Spec.Percent.fSpin;
lpFormat->fSpinWrap = lpCellType->Spec.Percent.fSpinWrap;
lpFormat->dfSpinInc = lpCellType->Spec.Percent.SpinInc;
}

#endif // SS_V40


void DLLENTRY SSInitSpread(HWND hWnd)
{
LPSS_BOOK lpBook = SS_BookLock(hWnd);

if (!lpBook->fCheckBoxDefBitmapsLoaded)
   {
   lpBook->fCheckBoxDefBitmapsLoaded = TRUE;
   CheckBoxLoadDefBitmaps(hDynamicInst);
   }

SS_BookUnlock(hWnd);
}


#ifdef SS_V80

//--------------------------------------------------------------------
//
//  The SSAddCustomCellType() function registers a custom cell type
//  with the spreadsheet.  If a custom celltype with the specified name
//  already exists, then it is replaced.
//

BOOL DLLENTRY SSAddCustomCellType(HWND hWnd, LPCTSTR lpszName, BOOL bEditable, BOOL bCanOverflow, BOOL bCanBeOverflown, BOOL bUseRendererControl, LPSS_CT_PROCS lpProcs)
{
LPSS_BOOK lpBook;
BOOL bRet;

lpBook = SS_BookLock(hWnd);
bRet = SS_AddCustomCellType(lpBook, lpszName, bEditable, bCanOverflow, bCanBeOverflown, bUseRendererControl, lpProcs);
SS_BookUnlock(hWnd);
return bRet;
}

//--------------------------------------------------------------------
//
//  The SS_AddCustomCellType() function registers a custom cell type
//  with the spreadsheet.
//

BOOL SS_AddCustomCellType(LPSS_BOOK lpBook, LPCTSTR lpszName, BOOL bEditable, BOOL bCanOverflow, BOOL bCanBeOverflown, BOOL bUseRendererControl, LPSS_CT_PROCS lpProcs)
{
  BOOL bReturn = FALSE;
  CT_HANDLE hCT;

  if( hCT = CustCellTypeLookup(&lpBook->CustCellTypes, lpszName) )
  {
    if( !CustCellTypeHasProcs(hCT) )
    {
	  if( CustCellTypeGetEditable(hCT) != bEditable )
		CustCellTypeSetEditable(hCT, bEditable);
	  if( CustCellTypeGetCanOverflow(hCT) != bCanOverflow )
		CustCellTypeSetCanOverflow(hCT, bCanOverflow);
	  if( CustCellTypeGetCanBeOverflown(hCT) != bCanBeOverflown )
		CustCellTypeSetCanBeOverflown(hCT, bCanBeOverflown);
     if(CustCellTypeGetUseRendererControl(hCT) != bUseRendererControl )
        CustCellTypeSetUseRendererControl(hCT, bUseRendererControl);
      bReturn = CustCellTypeSetProcs(hCT, lpProcs);
    }
  }
  else if( hCT = CustCellTypeCreate(lpszName, bEditable, bCanOverflow, bCanBeOverflown, bUseRendererControl, lpProcs) )
  {
    CustCellTypeAdd(&lpBook->CustCellTypes, hCT); 
    bReturn = TRUE;
  }
  return bReturn;
}

//--------------------------------------------------------------------
//
//  The SS_CustCellTypeLookup() function retrieves information about
//  the custom cell type.
//

BOOL SS_CustCellTypeLookup(LPSS_BOOK lpBook, LPCTSTR lpszName,
                                BOOL FAR* lpbEditable, BOOL FAR* lpbCanOverflow,
								BOOL FAR* lpbCanBeOverflown, BOOL FAR* lpbUseRendererControl, LPSS_CT_PROCS lpProcs)
{
  CT_HANDLE hCT;
  BOOL bReturn = FALSE;

  if( hCT = CustCellTypeLookup(&lpBook->CustCellTypes, lpszName) )
  {
    if( lpbEditable )
      *lpbEditable = CustCellTypeGetEditable(hCT);
    if( lpbCanOverflow )
      *lpbCanOverflow = CustCellTypeGetCanOverflow(hCT);
    if( lpbCanBeOverflown )
      *lpbCanBeOverflown = CustCellTypeGetCanBeOverflown(hCT);
    if( lpbUseRendererControl )
       *lpbUseRendererControl = CustCellTypeGetUseRendererControl(hCT);
	if( lpProcs )
	  CustCellTypeGetProcs(hCT, lpProcs);
//    if( lplRefCnt )
//      *lplRefCnt = CustCellTypeGetRefCnt(hCT);
    bReturn = TRUE;
  }
  return bReturn;
  
}

//--------------------------------------------------------------------
//
//  The SS_RemoveCustomCellType() function unregisters a custom
//  celltype.
//

BOOL SS_RemoveCustomCellType(LPSS_BOOK lpBook, LPCTSTR lpszName)
{
  CT_HANDLE hCT;
  BOOL bReturn = FALSE;

  if( hCT = CustCellTypeLookup(&lpBook->CustCellTypes, lpszName) )
  {
    if( CustCellTypeGetRefCnt(hCT) <= 1 )
    {
      SS_TypeControlRemove(hCT, &lpBook->hControls, &lpBook->ControlsAllocCnt,
                              &lpBook->ControlsCnt);
      CustCellTypeRemove(&lpBook->CustCellTypes, hCT);
      bReturn = TRUE;
    }
  }
  return bReturn;
}

//--------------------------------------------------------------------
//
//  The SSRemoveCustomCellType() function unregisters a custom
//  celltype if it has no references. (If there are references,
//  then the celltype is not removed and this function returns FALSE.)
//

BOOL DLLENTRY SSRemoveCustomCellType(HWND hWnd, LPCTSTR lpszName)
{
LPSS_BOOK lpBook;
BOOL bRet;

lpBook = SS_BookLock(hWnd);
bRet = SS_RemoveCustomCellType(lpBook, lpszName);
SS_BookUnlock(hWnd);
return bRet;
}

//--------------------------------------------------------------------
//
//  The SS_CustCellTypeFirst() function retrieves the first custom
//  celltype's name.
//

int SS_CustCellTypeFirst(LPSS_BOOK lpBook, LPTSTR lpszName, int nLen)
{
  CT_HANDLE hCT;
  int nRet = 0;

  if( hCT = CustCellTypeFirst(&lpBook->CustCellTypes) )
    nRet = CustCellTypeGetText(hCT, lpszName, nLen);
  else if( nLen > 0 )
    lpszName[0] = 0;
  return nRet;
}

//--------------------------------------------------------------------
//
//  The SS_CustCellTypeFirstLen() function retrieves the length of the
//  first custom celltype's name.
//

int SS_CustCellTypeFirstLen(LPSS_BOOK lpBook)
{
  CT_HANDLE hCT;
  int nRet = 0;

  if( hCT = CustCellTypeFirst(&lpBook->CustCellTypes) )
    nRet = CustCellTypeGetTextLen(hCT);
  return nRet;
}

//--------------------------------------------------------------------
//
//  The SS_CustCellTypeNext() function retrives the next custom celltype.
//

int SS_CustCellTypeNext(LPSS_BOOK lpBook, LPCTSTR lpszPrevName,
                             LPTSTR lpszName, int nLen)
{
  CT_HANDLE hCT;
  int nRet = 0;

  if( hCT = CustCellTypeNext(&lpBook->CustCellTypes, lpszPrevName) )
    nRet = CustCellTypeGetText(hCT, lpszName, nLen);
  else if( nLen > 0 )
    lpszName[0] = 0;
  return nRet;
}

//--------------------------------------------------------------------
//
//  The SS_CustCellTypeNextLen() function retrives the length of the
//  next custom celltype.
//

int SS_CustCellTypeNextLen(LPSS_BOOK lpBook, LPCTSTR lpszPrevName)
{
  CT_HANDLE hCT;
  int nRet = 0;

  if( hCT = CustCellTypeNext(&lpBook->CustCellTypes, lpszPrevName) )
    nRet = CustCellTypeGetTextLen(hCT);
  return nRet;
}


//--------------------------------------------------------------------
//
//  The SS_EnumCustomCellType() function is used to scan the list of
//  custom celltypes.
//

int SS_EnumCustomCellType(LPSS_BOOK lpBook, LPCTSTR lpszPrevName,
                          LPTSTR lpszName, int nLen)
{
int nRet;

if (NULL == lpszPrevName || 0 == lpszPrevName[0])
  nRet = SS_CustCellTypeFirst(lpBook, lpszName, nLen);
else
  nRet = SS_CustCellTypeNext(lpBook, lpszPrevName, lpszName, nLen);
return nRet;
}

//--------------------------------------------------------------------
//
//  The SSEnumCustomCellType() function is used to scan the list of
//  custom celltypes.
//

int DLLENTRY SSEnumCustomCellType(HWND hWnd, LPCTSTR lpszPrevName,
                                  LPTSTR lpszName, int nLen)
{
LPSS_BOOK lpBook;
int nRet;

lpBook = SS_BookLock(hWnd);
nRet = SS_EnumCustomFunction(lpBook, lpszPrevName, lpszName, nLen);
SS_BookUnlock(hWnd);
return nRet;
}

//--------------------------------------------------------------------
//
//  The SS_EnumCustomCellTypeLen() function retrieves the length of
//  the text representation of the next custom celltype.
//

int SS_EnumCustomCellTypeLen(LPSS_BOOK lpBook, LPCTSTR lpszPrevName)
{
int nRet;

if (NULL == lpszPrevName || 0 == lpszPrevName[0])
  nRet = SS_CustCellTypeFirstLen(lpBook);
else
  nRet = SS_CustCellTypeNextLen(lpBook, lpszPrevName);
return nRet;
}

//--------------------------------------------------------------------
//
//  The SSEnumCustomCellTypeLen() function retrieves the length of
//  the text representation of the next custom celltype.
//

int DLLENTRY SSEnumCustomCellTypeLen(HWND hWnd, LPCTSTR lpszPrevName)
{
LPSS_BOOK lpBook;
int nRet;

lpBook = SS_BookLock(hWnd);
nRet = SS_EnumCustomCellTypeLen(lpBook, lpszPrevName);
SS_BookUnlock(hWnd);
return nRet;
}

//--------------------------------------------------------------------
//
//  The SSGetCustomCellType() function retrieves information about
//  the custom celltype.
//

BOOL DLLENTRY SSGetCustomCellType(HWND hWnd, LPCTSTR lpszName,
                                  LPBOOL lpbEditable, 
                                  LPBOOL lpbCanOverflow,
                                  LPBOOL lpbCanBeOverflown,
                                  LPBOOL lpbUseRendererControl,
								  LPSS_CT_PROCS lpProcs)
{
LPSS_BOOK lpBook;
BOOL bRet;

lpBook = SS_BookLock(hWnd);
bRet = SS_CustCellTypeLookup(lpBook, lpszName, lpbEditable, lpbCanOverflow, lpbCanBeOverflown, lpbUseRendererControl, lpProcs);
SS_BookUnlock(hWnd);
return bRet;
}

//--------------------------------------------------------------------
//
//  The SSSetTypeCustom() function initializes a celltype structure
//  for setting a custom celltype.
//

LPSS_CELLTYPE DLLENTRY SSSetTypeCustom(HWND hWnd, LPSS_CELLTYPE CellType, LPCTSTR lpszName, long lStyle, LPSS_CT_VALUE ItemData)
{
LPSPREADSHEET lpSS;
LPSS_CELLTYPE Ret;

lpSS = SS_SheetLock(hWnd);
Ret = SS_SetTypeCustom(lpSS, CellType, lpszName, lStyle, ItemData);
SS_SheetUnlock(hWnd);
return Ret;
}

//--------------------------------------------------------------------
//
//  The SS_SetTypeCustom() function initializes a celltype structure
//  for setting a custom celltype.
//

LPSS_CELLTYPE SS_SetTypeCustom(LPSPREADSHEET lpSS, LPSS_CELLTYPE CellType, LPCTSTR lpszName, long lStyle, LPSS_CT_VALUE ItemData)
{
_fmemset(CellType, '\0', sizeof(SS_CELLTYPE));

if (!lpszName)
   return (NULL);

if (!(CellType->Spec.Custom.hName = SS_CreateTextHandle(lpszName, 0)))
   return (NULL);

CellType->Type  = SS_TYPE_CUSTOM;
CellType->Style = lStyle;
if( ItemData )
   MemHugeCpy(&CellType->Spec.Custom.ItemData, ItemData, sizeof(SS_CT_VALUE));

return (CellType);
}

HWND DLLENTRY SSGetCustomEditor(HWND hWnd, LPCTSTR lpszName, long lStyle)
{
LPSPREADSHEET lpSS;
HWND Ret;

lpSS = SS_SheetLock(hWnd);
Ret = SS_GetCustomEditor(lpSS, lpszName, lStyle);
SS_SheetUnlock(hWnd);
return Ret;
}

HWND SS_GetCustomEditor(LPSPREADSHEET lpSS, LPCTSTR lpszName, long lStyle)
{
HWND hWnd;
CT_HANDLE hCT = CustCellTypeLookup(&lpSS->lpBook->CustCellTypes, lpszName);
hWnd = SS_CT_GetControlhWnd(lpSS, hCT, lStyle, FALSE);
return hWnd;
}

HWND DLLENTRY SSGetCustomRenderer(HWND hWnd, LPCTSTR lpszName, long lStyle)
{
LPSPREADSHEET lpSS;
HWND Ret;

lpSS = SS_SheetLock(hWnd);
Ret = SS_GetCustomRenderer(lpSS, lpszName, lStyle);
SS_SheetUnlock(hWnd);
return Ret;
}

HWND SS_GetCustomRenderer(LPSPREADSHEET lpSS, LPCTSTR lpszName, long lStyle)
{
HWND hWnd;
CT_HANDLE hCT = CustCellTypeLookup(&lpSS->lpBook->CustCellTypes, lpszName);
hWnd = SS_CT_GetControlhWnd(lpSS, hCT, lStyle, TRUE);
return hWnd;
}

BOOL SS_GetCustomItemData(LPSPREADSHEET lpSS, SS_COORD CellCol, SS_COORD CellRow, LPSS_CT_VALUE ItemData)
{
SS_CELLTYPE CellType;

if( ItemData && SS_GetCellType(lpSS, CellCol, CellRow, &CellType) )
{
   if( CellType.Type == SS_TYPE_CUSTOM )
   {
      MemHugeCpy(ItemData, &CellType.Spec.Custom.ItemData, sizeof(SS_CT_VALUE));
      return TRUE;
   }
}
return FALSE;
}

BOOL DLLENTRY SSGetCustomItemData(HWND hWnd, SS_COORD CellCol, SS_COORD CellRow, LPSS_CT_VALUE ItemData)
{
LPSPREADSHEET lpSS = SS_SheetLock(hWnd);
BOOL Ret;

SS_AdjustCellCoords(lpSS, &CellCol, &CellRow);
Ret = SS_GetCustomItemData(lpSS, CellCol, CellRow, ItemData);
SS_SheetUnlock(hWnd);
return Ret;
}

BOOL SS_SetCustomItemData(LPSPREADSHEET lpSS, SS_COORD CellCol, SS_COORD CellRow, LPSS_CT_VALUE ItemData)
{
SS_CELLTYPE CellType;

if( ItemData && SS_GetCellType(lpSS, CellCol, CellRow, &CellType) )
{
   if( CellType.Type == SS_TYPE_CUSTOM )
   {
      MemHugeCpy(&CellType.Spec.Custom.ItemData, ItemData, sizeof(SS_CT_VALUE));
	  SS_SetCellTypeRange(lpSS, CellCol, CellRow, CellCol, CellRow, &CellType); // 27210 -scl
      return TRUE;
   }
}
return FALSE;
}


BOOL DLLENTRY SSSetCustomItemData(HWND hWnd, SS_COORD CellCol, SS_COORD CellRow, LPSS_CT_VALUE ItemData)
{
LPSPREADSHEET lpSS = SS_SheetLock(hWnd);
BOOL Ret;

SS_AdjustCellCoords(lpSS, &CellCol, &CellRow);
Ret = SS_SetCustomItemData(lpSS, CellCol, CellRow, ItemData);
SS_SheetUnlock(hWnd);
return Ret;
}

// this function checks for a custom celltype with an editor or renderer control id and
// references the custom celltype so that it cannot be removed.
void SS_CT_RefEditor(LPSPREADSHEET lpSS, LPSS_CELLTYPE CellType)
{
   if( CellType->Type == SS_TYPE_CUSTOM && CellType->ControlID )
   {
      LPTSTR lpszName = SS_CT_LockName(CellType);
      CT_HANDLE hCT = CustCellTypeLookup(&lpSS->lpBook->CustCellTypes, lpszName);

      if( hCT )
         CustCellTypeCreateRef(hCT);
      SS_CT_UnlockName(CellType);
   }
}

void SS_CT_RefRenderer(LPSPREADSHEET lpSS, LPSS_CELLTYPE CellType)
{
   if( CellType->Type == SS_TYPE_CUSTOM && CellType->Spec.Custom.RendererControlID )
   {
      LPTSTR lpszName = SS_CT_LockName(CellType);
      CT_HANDLE hCT = CustCellTypeLookup(&lpSS->lpBook->CustCellTypes, lpszName);

      if( hCT )
         CustCellTypeCreateRef(hCT);
      SS_CT_UnlockName(CellType);
   }
}

void SS_CT_Ref(LPSPREADSHEET lpSS, LPSS_CELLTYPE CellType)
{
   SS_CT_RefEditor(lpSS, CellType);
   SS_CT_RefRenderer(lpSS, CellType);
}

void SS_CT_UnrefEditor(LPSPREADSHEET lpSS, LPSS_CELLTYPE CellType)
{
   if( CellType->Type == SS_TYPE_CUSTOM && CellType->ControlID )
   {
      LPTSTR lpszName = SS_CT_LockName(CellType);
      CT_HANDLE hCT = CustCellTypeLookup(&lpSS->lpBook->CustCellTypes, lpszName);

      if( hCT )
         CustCellTypeDestroy(hCT);
      SS_CT_UnlockName(CellType);
   }
}

void SS_CT_UnrefRenderer(LPSPREADSHEET lpSS, LPSS_CELLTYPE CellType)
{
   if( CellType->Type == SS_TYPE_CUSTOM && CellType->Spec.Custom.RendererControlID )
   {
      LPTSTR lpszName = SS_CT_LockName(CellType);
      CT_HANDLE hCT = CustCellTypeLookup(&lpSS->lpBook->CustCellTypes, lpszName);

      if( hCT )
         CustCellTypeDestroy(hCT);
      SS_CT_UnlockName(CellType);
   }
}

void SS_CT_Unref(LPSPREADSHEET lpSS, LPSS_CELLTYPE CellType)
{
   SS_CT_UnrefEditor(lpSS, CellType);
   SS_CT_UnrefRenderer(lpSS, CellType);
}

//HGLOBAL DLLENTRY SSTstrToHGlobal(LPTSTR lpszValue)
//{
//  HGLOBAL hRet = NULL;
//  LPTSTR lpsz;
//
//  if( lpszValue )
//  {
//    hRet = GlobalAlloc(GHND, (lstrlen(lpszValue)+1) * sizeof(TCHAR));
//    if( lpsz = GlobalLock(hRet) )
//    {
//      lstrcpy(lpsz, lpszValue);
//      GlobalUnlock(hRet);
//    }
//  }
//  return hRet;
//}

BOOL SS_CT_CancelEditing(LPSPREADSHEET lpSS, LPSS_CELLTYPE CellType, HWND hWndCtrl, SS_COORD CellCol, SS_COORD CellRow)
{
  BOOL Ret = FALSE;
  LPTSTR lpszName;
  SS_CT_PROCS Procs = {0};

  if( lpszName = SS_CT_LockName(CellType) )
  {
    if( SS_CustCellTypeLookup(lpSS->lpBook, lpszName, NULL, NULL, NULL, NULL, &Procs) 
      && Procs.pfnCancelEditing )
    {
      SS_CT_CANCELEDITING_ARGS args = {0};

      args.lpszName = lpszName;
      args.hWndSpread = lpSS->lpBook->hWnd;
      args.hWndEditor = hWndCtrl;
      args.Sheet = lpSS->nSheetIndex + 1;
      args.Cell.Col = CellCol;
      args.Cell.Row = CellRow;
      SS_AdjustCellCoordsOut(lpSS, &args.Cell.Col, &args.Cell.Row);
      Ret = Procs.pfnCancelEditing(&args);
#if SS_OCX
      SS_CT_UpdateControlHwnd(lpSS, CellType);
#endif
    }
    SS_CT_UnlockName(CellType);
  }
  return Ret;
}

BOOL SS_CT_CreateEditorControl(LPSPREADSHEET lpSS, LPSS_CELLTYPE CellType, int CtrlId, HWND *phWndCtrl)
{
  BOOL Ret = FALSE;
  BOOL bEditable = FALSE;
  LPTSTR lpszName;
  SS_CT_PROCS Procs = {0};

  if( lpszName = SS_CT_LockName(CellType) )
  {
    if( SS_CustCellTypeLookup(lpSS->lpBook, lpszName, &bEditable, NULL, NULL, NULL, &Procs) 
      && bEditable && Procs.pfnCreateEditorControl )
    {
      SS_CT_CREATEEDITORCONTROL_ARGS args = {0};

      args.lpszName = lpszName;
      args.hWndSpread = lpSS->lpBook->hWnd;
      args.lStyle = CellType->Style;
      args.CtrlId = CtrlId;
      Ret = Procs.pfnCreateEditorControl(&args);
      *phWndCtrl = args.hWndEditor;
    }
    SS_CT_UnlockName(CellType);
  }
  return Ret;
}

BOOL SS_CT_CreateRendererControl(LPSPREADSHEET lpSS, LPSS_CELLTYPE CellType, int CtrlId, HWND *phWndCtrl)
{
  BOOL Ret = FALSE;
  BOOL bUseRendererControl = FALSE;
  LPTSTR lpszName;
  SS_CT_PROCS Procs = {0};

  if( lpszName = SS_CT_LockName(CellType) )
  {
    if( SS_CustCellTypeLookup(lpSS->lpBook, lpszName, NULL, NULL, NULL, &bUseRendererControl, &Procs) 
      && bUseRendererControl && Procs.pfnCreateRendererControl )
    {
      SS_CT_CREATERENDERERCONTROL_ARGS args = {0};

      args.lpszName = lpszName;
      args.hWndSpread = lpSS->lpBook->hWnd;
      args.lStyle = CellType->Style;
      args.CtrlId = CtrlId;
      Ret = Procs.pfnCreateRendererControl(&args);
      *phWndCtrl = args.hWndRenderer;
    }
    SS_CT_UnlockName(CellType);
  }
  return Ret;
}

BOOL SS_CT_GetEditorValue(LPSPREADSHEET lpSS, LPSS_CELLTYPE CellType, HWND hWndCtrl, SS_COORD CellCol, SS_COORD CellRow, LPSS_DATA DataItem)
{
  BOOL Ret = FALSE;
  BOOL bEditable = FALSE;
  LPTSTR lpszName;
  SS_CT_PROCS Procs = {0};

  if( lpszName = SS_CT_LockName(CellType) )
  {
    if( SS_CustCellTypeLookup(lpSS->lpBook, lpszName, &bEditable, NULL, NULL, NULL, &Procs) 
      && bEditable && Procs.pfnGetEditorValue )
    {
      SS_CT_GETEDITORVALUE_ARGS args = {0};

      args.lpszName = lpszName;
      args.hWndSpread = lpSS->lpBook->hWnd;
      args.hWndEditor = hWndCtrl;
      args.Sheet = lpSS->nSheetIndex + 1;
      args.Cell.Col = CellCol;
      args.Cell.Row = CellRow;
      SS_AdjustCellCoordsOut(lpSS, &args.Cell.Col, &args.Cell.Row);
      if( Ret = Procs.pfnGetEditorValue(&args) )
        SS_CTValueToData(&args.Value, DataItem);
      SS_FreeCTValue(&args.Value);
#if SS_OCX
      SS_CT_UpdateControlHwnd(lpSS, CellType);
#endif
    }
    SS_CT_UnlockName(CellType);
  }
  return Ret;
}

BOOL SS_CT_InitializeEditorControl(LPSPREADSHEET lpSS, LPSS_CELLTYPE CellType, SS_COORD CellCol, SS_COORD CellRow)
{
  BOOL Ret = FALSE;
  BOOL bEditable = FALSE;
  LPTSTR lpszName;
  SS_CT_PROCS Procs;

  if( lpszName = SS_CT_LockName(CellType) )
  {
    if( SS_CustCellTypeLookup(lpSS->lpBook, lpszName, &bEditable, NULL, NULL, NULL, &Procs) 
      && bEditable && Procs.pfnInitializeControl )
    {
      SS_CT_INITIALIZECONTROL_ARGS args = {0};

      args.lpszName = lpszName;
      args.hWndSpread = lpSS->lpBook->hWnd;
      args.hWndCtrl = SS_RetrieveControlhWnd(lpSS, CellCol, CellRow);
      args.Sheet = lpSS->nSheetIndex + 1;
      args.Cell.Col = CellCol;
      args.Cell.Row = CellRow;
      SS_CustCellTypeGetAppearance(lpSS, CellType, CellCol, CellRow, NULL, NULL, &args.Appearance);
      SS_AdjustCellCoordsOut(lpSS, &args.Cell.Col, &args.Cell.Row);
      Ret = Procs.pfnInitializeControl(&args);
#if SS_OCX
      SS_CT_UpdateControlHwnd(lpSS, CellType);
#endif
    }
    SS_CT_UnlockName(CellType);
  }
  return Ret;
}

BOOL SS_CT_InitializeRendererControl(LPSPREADSHEET lpSS, LPSS_CELLTYPE CellType, SS_COORD CellCol, SS_COORD CellRow)
{
  BOOL Ret = FALSE;
  BOOL bUseRendererControl = FALSE;
  LPTSTR lpszName;
  SS_CT_PROCS Procs;

  if( lpszName = SS_CT_LockName(CellType) )
  {
    if( SS_CustCellTypeLookup(lpSS->lpBook, lpszName, NULL, NULL, NULL, &bUseRendererControl, &Procs) 
      && bUseRendererControl && Procs.pfnInitializeControl )
    {
      SS_CT_INITIALIZECONTROL_ARGS args = {0};

      args.lpszName = lpszName;
      args.hWndSpread = lpSS->lpBook->hWnd;
      args.hWndCtrl = SS_GetControlRenderer(lpSS, CellType);
      args.Sheet = lpSS->nSheetIndex + 1;
      args.Cell.Col = CellCol;
      args.Cell.Row = CellRow;
      SS_CustCellTypeGetAppearance(lpSS, CellType, CellCol, CellRow, NULL, NULL, &args.Appearance);
      SS_AdjustCellCoordsOut(lpSS, &args.Cell.Col, &args.Cell.Row);
      Ret = Procs.pfnInitializeControl(&args);
#if SS_OCX
      SS_CT_UpdateControlHwnd(lpSS, CellType);
#endif
    }
    SS_CT_UnlockName(CellType);
  }
  return Ret;
}

BOOL SS_CT_IsReservedKey(LPSPREADSHEET lpSS, SS_COORD CellCol, SS_COORD CellRow, WPARAM wParam)
{
  BOOL Ret = TRUE;
  BOOL fEditable;
  BOOL bUseRendererControl = FALSE;
  LPSS_CELLTYPE CellType;
  LPTSTR lpszName;
  SS_CT_PROCS Procs;

  CellType = SS_CT_LockCellType(lpSS, CellCol, CellRow);
  if( lpszName = SS_CT_LockName(CellType) )
  {
    if( SS_CustCellTypeLookup(lpSS->lpBook, lpszName, &fEditable, NULL, NULL, &bUseRendererControl, &Procs) 
      && fEditable && Procs.pfnIsReservedKey )
    {
      SS_CT_ISRESERVEDKEY_ARGS args = {0};

      args.lpszName = lpszName;
      args.hWndSpread = lpSS->lpBook->hWnd;
      args.hWndRenderer = bUseRendererControl ? SS_GetControlRenderer(lpSS, CellType) : NULL;
      args.Sheet = lpSS->nSheetIndex + 1;
      args.Cell.Col = CellCol;
      args.Cell.Row = CellRow;
      args.nKeyCode = (UINT)wParam;
      args.nShift = (HIBYTE(GetKeyState(VK_SHIFT)) ? 1 : 0) + (HIBYTE(GetKeyState(VK_CONTROL)) ? 2 : 0);
      SS_AdjustCellCoordsOut(lpSS, &args.Cell.Col, &args.Cell.Row);
      Ret = Procs.pfnIsReservedKey(&args);
#if SS_OCX
      SS_CT_UpdateControlHwnd(lpSS, CellType);
#endif
    }
    SS_CT_UnlockName(CellType);
  }
  SS_CT_UnlockCellType(lpSS, CellCol, CellRow);
  return Ret;
}

BOOL SS_CT_IsReservedLocation(LPSPREADSHEET lpSS, LPSS_CELLTYPE CellType, LPRECT lpRectCell, SS_COORD CellCol, SS_COORD CellRow, int MouseX, int MouseY, HCURSOR *lpCursor)
{
  BOOL Ret = FALSE;
  BOOL bUseRendererControl = FALSE;
  LPTSTR lpszName = NULL;
  SS_CT_PROCS Procs = {0};
  SS_CT_ISRESERVEDLOCATION_ARGS args = {0};
  HDC hDC;
  SS_DATA DataItem;

  if( lpszName = SS_CT_LockName(CellType) )
  {
    if( SS_CustCellTypeLookup(lpSS->lpBook, lpszName, NULL, NULL, NULL, &bUseRendererControl, &Procs)
      && Procs.pfnIsReservedLocation )
    {
      if( bUseRendererControl )
      {
         SS_CT_InitializeRendererControl(lpSS, CellType, CellCol, CellRow);
         SS_CT_SetRendererValue(lpSS, CellType, CellCol, CellRow);
      }
      args.lpszName = lpszName;
      args.hWndSpread = lpSS->lpBook->hWnd;
      args.hWndRenderer = bUseRendererControl ? SS_GetControlRenderer(lpSS, CellType) : NULL;
      args.Sheet = lpSS->nSheetIndex + 1;
      args.Cell.Col = CellCol;
      args.Cell.Row = CellRow;
      CopyRect(&args.rect, lpRectCell);
      hDC = GetDC(lpSS->lpBook->hWnd);
      SS_CustCellTypeGetAppearance(lpSS, CellType, CellCol, CellRow, hDC, NULL, &args.Appearance);
      ReleaseDC(lpSS->lpBook->hWnd, hDC);
      SS_GetDataStruct(lpSS, NULL, NULL, NULL, &DataItem, CellCol, CellRow);
      SS_DataToCTValue(&args.Value, &DataItem);
      args.p.x = MouseX;
      args.p.y = MouseY;
      SS_AdjustCellCoordsOut(lpSS, &args.Cell.Col, &args.Cell.Row);
      Ret = Procs.pfnIsReservedLocation(&args);
      if( Ret && lpCursor && args.lReservedCursorId )
      {
#if SS_OCX
         SS_CT_UpdateControlHwnd(lpSS, CellType);
#endif
         SS_CT_GetReservedCursor(lpSS, CellType, CellCol, CellRow, args.lReservedCursorId, lpCursor);
      }
      SS_FreeCTValue(&args.Value);
    }
    SS_CT_UnlockName(CellType);
  }
  return Ret;
}

BOOL SS_CT_IsValid(LPSPREADSHEET lpSS, LPSS_CELLTYPE CellType, HWND hWndCtrl, SS_COORD CellCol, SS_COORD CellRow, LPSS_DATA DataItem)
{
  BOOL Ret = TRUE;
  LPTSTR lpszName;
  SS_CT_PROCS Procs = {0};

  if( lpszName = SS_CT_LockName(CellType) )
  {
    if( SS_CustCellTypeLookup(lpSS->lpBook, lpszName, NULL, NULL, NULL, NULL, &Procs) 
      && Procs.pfnIsValid )
    {
      SS_CT_ISVALID_ARGS args = {0};

      args.lpszName = lpszName;
      args.hWndSpread = lpSS->lpBook->hWnd;
      args.hWndEditor = hWndCtrl;
      SS_DataToCTValue(&args.Value, DataItem);
      Ret = Procs.pfnIsValid(&args);
#if SS_OCX
      SS_CT_UpdateControlHwnd(lpSS, CellType);
#endif
      SS_FreeCTValue(&args.Value);
    }
    SS_CT_UnlockName(CellType);
  }
  return Ret;
}

BOOL SS_CT_GetPreferredSize(LPSPREADSHEET lpSS, LPSS_CELLTYPE CellType, HDC hDC, LPRECT lpRect, SS_COORD CellCol, SS_COORD CellRow, LPSIZE lpSize)
{
  BOOL Ret = FALSE;
  BOOL bUseRendererControl = FALSE;
  LPTSTR lpszName;
  SS_CT_PROCS Procs = {0};
  SS_DATA DataItem;

  if( lpszName = SS_CT_LockName(CellType) )
  {
    if( SS_CustCellTypeLookup(lpSS->lpBook, lpszName, NULL, NULL, NULL, &bUseRendererControl, &Procs) 
       && Procs.pfnGetPreferredSize )
    {
      SS_CT_GETPREFERREDSIZE_ARGS args = {0};

      if( bUseRendererControl )
      {
         SS_CT_InitializeRendererControl(lpSS, CellType, CellCol, CellRow);
         SS_CT_SetRendererValue(lpSS, CellType, CellCol, CellRow);
      }
      SS_GetDataStruct(lpSS, NULL, NULL, NULL, &DataItem, CellCol, CellRow);
      args.lpszName = lpszName;
      args.hDC = hDC;
      args.hWndRenderer = bUseRendererControl ? SS_GetControlRenderer(lpSS, CellType) : NULL;
      args.hWndSpread = lpSS->lpBook->hWnd;
      args.Sheet = lpSS->nSheetIndex + 1;
      args.Cell.Col = CellCol;
      args.Cell.Row = CellRow;
      SS_CustCellTypeGetAppearance(lpSS, CellType, CellCol, CellRow, hDC, NULL, &args.Appearance);
      SS_DataToCTValue(&args.Value, &DataItem);
      CopyRect(&args.rect, lpRect);
      args.sizePreferred.cx = args.rect.right - args.rect.left;
      args.sizePreferred.cy = args.rect.bottom - args.rect.top;
      SS_AdjustCellCoordsOut(lpSS, &args.Cell.Col, &args.Cell.Row);
      Ret = Procs.pfnGetPreferredSize(&args);
#if SS_OCX
      SS_CT_UpdateControlHwnd(lpSS, CellType);
#endif
      if( Ret && lpSize )
      {
         lpSize->cx = args.sizePreferred.cx;
         lpSize->cy = args.sizePreferred.cy;
      }
      SS_FreeCTValue(&args.Value);
    }
    SS_CT_UnlockName(CellType);
  }
  return Ret;
}

BOOL SS_CT_GetReservedCursor(LPSPREADSHEET lpSS, LPSS_CELLTYPE CellType, SS_COORD CellCol, SS_COORD CellRow, long lReservedCursorId, HCURSOR *phCursor)
{
  BOOL Ret = FALSE;
  BOOL bUseRendererControl = FALSE;
  LPTSTR lpszName = NULL;
  SS_CT_PROCS Procs = {0};
  SS_CT_GETRESERVEDCURSOR_ARGS args = {0};

  if( lpszName = SS_CT_LockName(CellType) )
  {
    if( SS_CustCellTypeLookup(lpSS->lpBook, lpszName, NULL, NULL, NULL, &bUseRendererControl, &Procs)
      && Procs.pfnGetReservedCursor )
    {
       args.lpszName = lpszName;
       args.hWndSpread = lpSS->lpBook->hWnd;
       args.hWndRenderer = bUseRendererControl ? SS_GetControlRenderer(lpSS, CellType) : NULL;
       args.lReservedCursorId = lReservedCursorId;
       Ret = Procs.pfnGetReservedCursor(&args);
#if SS_OCX
      SS_CT_UpdateControlHwnd(lpSS, CellType);
#endif
       if( Ret && phCursor && args.hCursor )
          *phCursor = args.hCursor;
    }
    SS_CT_UnlockName(CellType);
  }
  return Ret;
}

BOOL SS_CT_PaintCell(LPSPREADSHEET lpSS, LPSS_CELLTYPE CellType, HDC hDC, LPSSX_PRINTFORMAT lpPFormat, LPRECT Rect, SS_COORD CellCol, SS_COORD CellRow, BOOL fSelected)
{
  BOOL Ret = FALSE;
  BOOL bUseRendererControl = FALSE;
  LPTSTR lpszName;
  SS_CT_PROCS Procs = {0};
  SS_DATA DataItem;

  if( lpszName = SS_CT_LockName(CellType) )
  {
    if( SS_CustCellTypeLookup(lpSS->lpBook, lpszName, NULL, NULL, NULL, &bUseRendererControl, &Procs) )
    {
       HWND hWndCT = bUseRendererControl ? SS_GetControlRenderer(lpSS, CellType) : NULL;
       int  iSaveDC = 0;

       SS_GetDataStruct(lpSS, NULL, NULL, NULL, &DataItem, CellCol, CellRow);
       if( Procs.pfnPaintCell )
       {
          SS_CT_PAINTCELL_ARGS args = {0};

          args.lpszName = lpszName;
          args.hWndSpread = lpSS->lpBook->hWnd;
          args.hWndRenderer = hWndCT;
          args.Sheet = lpSS->nSheetIndex + 1;
          args.Cell.Col = CellCol;
          args.Cell.Row = CellRow;
          args.fIsLocked = SS_GetLock(lpSS, CellCol, CellRow, TRUE);
          args.fIsSelected = fSelected;
          args.hDC = hDC;
          CopyRect(&args.rect, Rect);
          SS_DataToCTValue(&args.Value, &DataItem);
          SS_CustCellTypeGetAppearance(lpSS, CellType, CellCol, CellRow, hDC, lpPFormat, &args.Appearance);
	       SS_AdjustCellCoordsOut(lpSS, &args.Cell.Col, &args.Cell.Row);
          Ret = Procs.pfnPaintCell(&args);
          SS_FreeCTValue(&args.Value);
#if SS_OCX
          SS_CT_UpdateControlHwnd(lpSS, CellType);
#endif
       }
   }
   SS_CT_UnlockName(CellType);
  }
  return Ret;
}

BOOL SS_CT_SetEditorValue(LPSPREADSHEET lpSS, LPSS_CELLTYPE CellType, SS_COORD CellCol, SS_COORD CellRow)
{
  BOOL Ret = FALSE;
  BOOL bEditable = FALSE;
  LPTSTR lpszName;
  SS_CT_PROCS Procs;
  SS_DATA DataItem;

  if( lpszName = SS_CT_LockName(CellType) )
  {
    if( SS_CustCellTypeLookup(lpSS->lpBook, lpszName, &bEditable, NULL, NULL, NULL, &Procs) 
      && bEditable && Procs.pfnSetValue )
    {
      SS_CT_SETVALUE_ARGS args = {0};

      args.lpszName = lpszName;
      args.hWndSpread = lpSS->lpBook->hWnd;
      args.hWndCtrl = SS_RetrieveControlhWnd(lpSS, CellCol, CellRow);
      args.Sheet = lpSS->nSheetIndex + 1;
      args.Cell.Col = CellCol;
      args.Cell.Row = CellRow;
      SS_GetDataStruct(lpSS, NULL, NULL, NULL, &DataItem, CellCol, CellRow);
      SS_DataToCTValue(&args.Value, &DataItem);
      SS_AdjustCellCoordsOut(lpSS, &args.Cell.Col, &args.Cell.Row);
      Ret = Procs.pfnSetValue(&args);
      SS_FreeCTValue(&args.Value);
#if SS_OCX
      SS_CT_UpdateControlHwnd(lpSS, CellType);
#endif
    }
    SS_CT_UnlockName(CellType);
  }
  return Ret;
}

BOOL SS_CT_SetRendererValue(LPSPREADSHEET lpSS, LPSS_CELLTYPE CellType, SS_COORD CellCol, SS_COORD CellRow)
{
  BOOL Ret = FALSE;
  BOOL bUseRendererControl = FALSE;
  LPTSTR lpszName;
  SS_CT_PROCS Procs;
  SS_DATA DataItem;

  if( lpszName = SS_CT_LockName(CellType) )
  {
    if( SS_CustCellTypeLookup(lpSS->lpBook, lpszName, NULL, NULL, NULL, &bUseRendererControl, &Procs) 
      && bUseRendererControl && Procs.pfnSetValue )
    {
      SS_CT_SETVALUE_ARGS args = {0};

      args.lpszName = lpszName;
      args.hWndSpread = lpSS->lpBook->hWnd;
      args.hWndCtrl = SS_GetControlRenderer(lpSS, CellType);
      args.Sheet = lpSS->nSheetIndex + 1;
      args.Cell.Col = CellCol;
      args.Cell.Row = CellRow;
      SS_GetDataStruct(lpSS, NULL, NULL, NULL, &DataItem, CellCol, CellRow);
      SS_DataToCTValue(&args.Value, &DataItem);
      SS_AdjustCellCoordsOut(lpSS, &args.Cell.Col, &args.Cell.Row);
      Ret = Procs.pfnSetValue(&args);
      SS_FreeCTValue(&args.Value);
#if SS_OCX
      SS_CT_UpdateControlHwnd(lpSS, CellType);
#endif
    }
    SS_CT_UnlockName(CellType);
  }
  return Ret;
}

BOOL SS_CT_StartEditing(LPSPREADSHEET lpSS, LPSS_CELLTYPE CellType, SS_COORD CellCol, SS_COORD CellRow, UINT Msg, WPARAM wParam, LPARAM lParam)
{
  BOOL Ret = FALSE;
  BOOL bEditable = FALSE;
  LPTSTR lpszName;
  SS_CT_PROCS Procs;

  if( lpszName = SS_CT_LockName(CellType) )
  {
    if( SS_CustCellTypeLookup(lpSS->lpBook, lpszName, &bEditable, NULL, NULL, NULL, &Procs) 
      && bEditable && Procs.pfnStartEditing )
    {
      SS_CT_STARTEDITING_ARGS args = {0};

      args.lpszName = lpszName;
      args.hWndSpread = lpSS->lpBook->hWnd;
      args.hWndEditor = SS_RetrieveControlhWnd(lpSS, CellCol, CellRow);
      args.Sheet = lpSS->nSheetIndex + 1;
      args.Cell.Col = CellCol;
      args.Cell.Row = CellRow;
      args.fAutoClipboard = (BOOL)lpSS->lpBook->fAutoClipboard;
      args.fSelectAll = (BOOL)lpSS->lpBook->fEditModeReplace;
      args.msg.hwnd = args.hWndEditor;
      args.msg.message = Msg;
      args.msg.wParam = wParam;
      args.msg.lParam = lParam;
      SS_AdjustCellCoordsOut(lpSS, &args.Cell.Col, &args.Cell.Row);
      Ret = Procs.pfnStartEditing(&args);
#if SS_OCX
      SS_CT_UpdateControlHwnd(lpSS, CellType);
#endif
    }
    SS_CT_UnlockName(CellType);
  }
  return Ret;
}

BOOL SS_CT_StopEditing(LPSPREADSHEET lpSS, LPSS_CELLTYPE CellType, HWND hWndCtrl, SS_COORD CellCol, SS_COORD CellRow)
{
  BOOL Ret = TRUE;
  BOOL bEditable = FALSE;
  LPTSTR lpszName;
  SS_CT_PROCS Procs = {0};

  if( lpszName = SS_CT_LockName(CellType) )
  {
    if( SS_CustCellTypeLookup(lpSS->lpBook, lpszName, &bEditable, NULL, NULL, NULL, &Procs) 
      && bEditable && Procs.pfnStopEditing )
    {
      SS_CT_STOPEDITING_ARGS args = {0};

      args.lpszName = lpszName;
      args.hWndSpread = lpSS->lpBook->hWnd;
      args.hWndEditor = hWndCtrl;
      args.Sheet = lpSS->nSheetIndex + 1;
      args.Cell.Col = CellCol;
      args.Cell.Row = CellRow;
      args.Cancel = FALSE;
      SS_AdjustCellCoordsOut(lpSS, &args.Cell.Col, &args.Cell.Row);
      Procs.pfnStopEditing(&args);
      Ret = !args.Cancel;
#if SS_OCX
      SS_CT_UpdateControlHwnd(lpSS, CellType);
#endif
    }
    SS_CT_UnlockName(CellType);
  }
  return Ret;
}

BOOL SS_CT_StringFormat(LPSPREADSHEET lpSS, LPSS_CELLTYPE CellType, SS_COORD CellCol, SS_COORD CellRow, LPSS_CT_VALUE lpValue, HGLOBAL *phszFormatted)
{
  BOOL bRet = FALSE;
  BOOL bUseRendererControl = FALSE;
  LPTSTR lpszName;
  SS_CT_PROCS Procs = {0};

  if( phszFormatted && (lpszName = SS_CT_LockName(CellType)) )
  {
    if( SS_CustCellTypeLookup(lpSS->lpBook, lpszName, NULL, NULL, NULL, &bUseRendererControl, &Procs) 
      && Procs.pfnStringFormat )
    {
      SS_CT_STRINGFORMAT_ARGS args = {0};

      args.lpszName = lpszName;
      args.hWndSpread = lpSS->lpBook->hWnd;
      args.hWndRenderer = bUseRendererControl ? SS_GetControlRenderer(lpSS, CellType) : NULL;
      MemHugeCpy(&args.Value, lpValue, sizeof(SS_CT_VALUE));
      args.lStyle = CellType->Style;
      args.hszFormatted = NULL;
      if( Procs.pfnStringFormat(&args) && args.hszFormatted )
      {
        *phszFormatted = args.hszFormatted;
        bRet = TRUE;
      }
#if SS_OCX
      SS_CT_UpdateControlHwnd(lpSS, CellType);
#endif
    }
    SS_CT_UnlockName(CellType);
  }
  return bRet;
}
BOOL SS_CT_StringUnformat(LPSPREADSHEET lpSS, LPSS_CELLTYPE CellType, SS_COORD CellCol, SS_COORD CellRow, LPTSTR lpszFormatted, LPSS_DATA DataItem)
{
  BOOL bRet = FALSE;
  BOOL bUseRendererControl = FALSE;
  LPTSTR lpszName;
  SS_CT_PROCS Procs = {0};

  if( lpszName = SS_CT_LockName(CellType) )
  {
    if( SS_CustCellTypeLookup(lpSS->lpBook, lpszName, NULL, NULL, NULL, &bUseRendererControl, &Procs) 
      && Procs.pfnStringUnformat )
    {
      SS_CT_STRINGUNFORMAT_ARGS args = {0};

      args.lpszName = lpszName;
      args.hWndSpread = lpSS->lpBook->hWnd;
      args.hWndRenderer = bUseRendererControl ? SS_GetControlRenderer(lpSS, CellType) : NULL;
      args.lStyle = CellType->Style;
      args.lpszFormatted = lpszFormatted;
      if( Procs.pfnStringUnformat(&args) )
      {
        SS_CTValueToData(&args.Value, DataItem);
        bRet = TRUE;
      }
      SS_FreeCTValue(&args.Value);
#if SS_OCX
      SS_CT_UpdateControlHwnd(lpSS, CellType);
#endif
    }
    SS_CT_UnlockName(CellType);
  }
  return bRet;
}

//--------------------------------------------------------------------
#endif
