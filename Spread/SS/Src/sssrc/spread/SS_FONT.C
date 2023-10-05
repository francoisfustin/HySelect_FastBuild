/*********************************************************
* SS_FONT.C
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
#include "spread.h"
#include "fphdc.h"
#include "ss_alloc.h"
#include "ss_col.h"
#include "ss_draw.h"
#include "ss_font.h"
#include "ss_row.h"
#include "ss_win.h"

SS_FONTPOINTS FAR DefaultFontHeight[] =
   {
   11, 10.5,                           /* 8  Point */
   13, 12.75,                          /* 10 Point */
   16, 15.75,                          /* 12 Point */
   18, 18.75,                          /* 14 Point */
   24, 22.5,                           /* 18 Point */
   32, 28.5                            /* 24 Point */
   };

SS_FONTID SSx_CreateFont(LPSS_BOOK lpBook, LPSS_FONT lpFontTable, SS_FONTID FontId, HFONT hFont, LPLOGFONT lpLogFont, BOOL fDeleteFont,
                         BOOL fCreateFont);

LPSS_FONT SS_RetrieveFont(lpSS, Font, Cell, CellCol, CellRow)

LPSPREADSHEET lpSS;
LPSS_FONT     Font;
LPSS_CELL     Cell;
SS_COORD      CellCol;
SS_COORD      CellRow;
{
SS_FONTID     FontId;

FontId = SS_RetrieveFontId(lpSS, Cell, CellCol, CellRow);

return (SS_GetFont(lpSS->lpBook, Font, FontId));
}


SS_FONTID SS_RetrieveFontId(lpSS, Cell, CellCol, CellRow)

LPSPREADSHEET lpSS;
LPSS_CELL     Cell;
SS_COORD      CellCol;
SS_COORD      CellRow;
{
LPSS_COL      lpCol;
LPSS_ROW      lpRow;
SS_FONTID     FontId;

lpCol = SS_LockColItem(lpSS, CellCol);
lpRow = SS_LockRowItem(lpSS, CellRow);

FontId = SSx_RetrieveFontId(lpSS, lpCol, lpRow, Cell, CellCol, CellRow);

SS_UnlockColItem(lpSS, CellCol);
SS_UnlockRowItem(lpSS, CellRow);

return (FontId);
}


SS_FONTID SSx_RetrieveFontId(lpSS, lpCol, lpRow, Cell, CellCol, CellRow)

LPSPREADSHEET lpSS;
LPSS_COL      lpCol;
LPSS_ROW      lpRow;
LPSS_CELL     Cell;
SS_COORD      CellCol;
SS_COORD      CellRow;
{
SS_FONTID     FontId;

if (Cell && Cell->FontId != SS_FONT_NONE)
   FontId = Cell->FontId;

else if (lpRow && CellRow < lpSS->Row.HeaderCnt &&
         lpRow->FontId != SS_FONT_NONE)
   FontId = lpRow->FontId;

else if (lpCol && lpCol->FontId != SS_FONT_NONE)
   FontId = lpCol->FontId;

else if (lpRow && lpRow->FontId != SS_FONT_NONE)
   FontId = lpRow->FontId;

else
   FontId = lpSS->DefaultFontId;

return (FontId);
}


#if 0
SS_FONTID SS_InitFont(LPSS_BOOK lpBook, HFONT hFont, BOOL fDeleteFont,
                      BOOL fCreateFont, LPBOOL lpfDeleteFontObject)
{
HDC        hdc;
HFONT      hFontOld;
TEXTMETRIC fm;
LOGFONT    LogFont;
LPSS_FONT  lpFontTable;
SS_FONTID  FontId;
SS_FONTID  i;
double     FontCellHeight = 0.0;
int        Size;
short      Attr = 0;
UINT       nCh;
int FAR   *lpCharWidths;

_fmemset(&LogFont, '\0', sizeof(LOGFONT));
GetObject(hFont, sizeof(LOGFONT), &LogFont);

// BJO 30Oct96 SEL6622 - Begin fix
// In Win31 and WinNT, GetObject() zero fills unused portion of
// lfFaceName[].  In Win95, GetObject() leaves garbage in unused
// portion of lfFaceName[].
for (i = lstrlen(LogFont.lfFaceName) + 1; i < LF_FACESIZE; i++)
   LogFont.lfFaceName[i] = '\0';
// BJO 30Oct96 SEL6622 - End fix

if (lpfDeleteFontObject)
   *lpfDeleteFontObject = FALSE;

if (!SS_FontTableAlloc())
   return (-1);

lpFontTable = SS_FontTableLock();

for (i = 0; i < FontTable.dTableCnt; i++)
   if (lpFontTable[i].hFont == hFont || _fmemcmp(&LogFont,
       &lpFontTable[i].LogFont, sizeof(LOGFONT)) == 0)
      {
      if (lpFontTable[i].hFont != hFont && fDeleteFont == -1)
         if (lpfDeleteFontObject)
            *lpfDeleteFontObject = TRUE;

      SS_FontTableUnlock();
      return (i + 1);
      }

/*************************
* Add Font to font table
*************************/

FontId = FontTable.dTableCnt;

if (hFont && fCreateFont)
   hFont = (HFONT)CreateFontIndirect(&LogFont);

lpFontTable[FontId].FontId = FontId;
lpFontTable[FontId].hFont = hFont;
lpFontTable[FontId].DeleteFont = fDeleteFont;

_fmemcpy(&lpFontTable[FontId].LogFont, &LogFont, sizeof(LOGFONT));

hdc = SS_GetDC(lpBook);
hFontOld = SelectObject(hdc, hFont);
GetTextMetrics(hdc, &fm);

// Create array for Char widths.  SCP 12/31/98
nCh = max(fm.tmLastChar,255);
lpFontTable[FontId].hCharWidths = tbGlobalAlloc(GSHARE, sizeof(int)*(nCh + 1));
lpCharWidths = (int FAR *)tbGlobalLock(lpFontTable[FontId].hCharWidths);
if (lpCharWidths)
  {
  UINT j;
  GetCharWidth(hdc, 0, nCh, lpCharWidths);
  if (fm.tmOverhang)
    for (j = 0; j <= (UINT)nCh; j++)
      lpCharWidths[j] -= fm.tmOverhang;
  tbGlobalUnlock(lpFontTable[FontId].hCharWidths);
  }
// end of Char widths code

lpFontTable[FontId].dwFontLanguageInfo = GetFontLanguageInfo(hdc);

SelectObject(hdc, hFontOld);
SS_ReleaseDC(lpBook, hdc);

lpFontTable[FontId].FontHeight = (short)fm.tmHeight + 2;

if (fm.tmInternalLeading == 0 && fm.tmAscent > 0)
   Size = fm.tmAscent;
else
   Size = fm.tmHeight - fm.tmInternalLeading;

for (i = 0; i < sizeof(DefaultFontHeight) / sizeof(SS_FONTPOINTS); i++)
   if (Size <= DefaultFontHeight[i].Size)
      {
      FontCellHeight = (DefaultFontHeight[i].CellPoints * (double)Size) /
                       DefaultFontHeight[i].Size;
      break;
      }

if (FontCellHeight == 0.0 && i > 0)
   FontCellHeight = (DefaultFontHeight[i - 1].CellPoints * (double)Size) /
                    DefaultFontHeight[i - 1].Size;

lpFontTable[FontId].FontCellHeight = FontCellHeight;

FontTable.dTableCnt++;
FontId = FontTable.dTableCnt;

SS_FontTableUnlock();

return (FontId);
}
#endif

SS_FONTID SS_InitFont(LPSS_BOOK lpBook, HFONT hFont, BOOL fDeleteFont,
                      BOOL fCreateFont, LPBOOL lpfDeleteFontObject)
{
LOGFONT   LogFont;
LPSS_FONT lpFontTable;
SS_FONTID FontId;
SS_FONTID i;

_fmemset(&LogFont, '\0', sizeof(LOGFONT));
GetObject(hFont, sizeof(LOGFONT), &LogFont);

// BJO 30Oct96 SEL6622 - Begin fix
// In Win31 and WinNT, GetObject() zero fills unused portion of
// lfFaceName[].  In Win95, GetObject() leaves garbage in unused
// portion of lfFaceName[].
for (i = lstrlen(LogFont.lfFaceName) + 1; i < LF_FACESIZE; i++)
   LogFont.lfFaceName[i] = '\0';
// BJO 30Oct96 SEL6622 - End fix

if (lpfDeleteFontObject)
   *lpfDeleteFontObject = FALSE;

if (!SS_FontTableAlloc())
   return (-1);

lpFontTable = SS_FontTableLock();

for (i = 0; i < FontTable.dTableCnt; i++)
   if (lpFontTable[i].hFont == hFont || _fmemcmp(&LogFont,
       &lpFontTable[i].LogFont, sizeof(LOGFONT)) == 0)
      {
      if (lpFontTable[i].hFont != hFont && fDeleteFont == -1)
         if (lpfDeleteFontObject)
            *lpfDeleteFontObject = TRUE;

      SS_FontTableUnlock();
      return (i + 1);
      }

/*************************
* Add Font to font table
*************************/

FontId = SSx_CreateFont(lpBook, lpFontTable, FontTable.dTableCnt, hFont, &LogFont, fDeleteFont, fCreateFont);
FontTable.dTableCnt++;

SS_FontTableUnlock();

return (FontId);
}

#if SS_V80
SS_FONTID SS_InitFontLogFont(LPSS_BOOK lpBook, LOGFONT *pLogFont)
{
LOGFONT   LogFont;
LPSS_FONT lpFontTable;
SS_FONTID FontId;
SS_FONTID i;

if( pLogFont )
   _fmemcpy(&LogFont, pLogFont, sizeof(LOGFONT));
else
   return 0;

// BJO 30Oct96 SEL6622 - Begin fix
// In Win31 and WinNT, GetObject() zero fills unused portion of
// lfFaceName[].  In Win95, GetObject() leaves garbage in unused
// portion of lfFaceName[].
for (i = lstrlen(LogFont.lfFaceName) + 1; i < LF_FACESIZE; i++)
   LogFont.lfFaceName[i] = '\0';
// BJO 30Oct96 SEL6622 - End fix

if (!SS_FontTableAlloc())
   return (-1);

lpFontTable = SS_FontTableLock();

for (i = 0; i < FontTable.dTableCnt; i++)
   if (_fmemcmp(&LogFont, &lpFontTable[i].LogFont, sizeof(LOGFONT)) == 0)
      {
      SS_FontTableUnlock();
      return (i + 1);
      }

/*************************
* Add Font to font table
*************************/

FontId = SSx_CreateFont(lpBook, lpFontTable, FontTable.dTableCnt, NULL, &LogFont, TRUE, TRUE);
FontTable.dTableCnt++;

SS_FontTableUnlock();

return (FontId);
}
#endif

SS_FONTID SSx_CreateFont(LPSS_BOOK lpBook, LPSS_FONT lpFontTable, SS_FONTID FontId, HFONT hFont, LPLOGFONT lpLogFont, BOOL fDeleteFont,
                         BOOL fCreateFont)
{
HDC        hdc;
HFONT      hFontOld;
TEXTMETRIC fm;
SS_FONTID  i;
double     FontCellHeight = 0.0;
int        Size;
short      Attr = 0;
UINT       nCh;
int FAR   *lpCharWidths;

/*************************
* Add Font to font table
*************************/

if (fCreateFont)
   hFont = (HFONT)CreateFontIndirect(lpLogFont);

lpFontTable[FontId].FontId = FontId;
lpFontTable[FontId].hFont = hFont;
lpFontTable[FontId].DeleteFont = fDeleteFont;

_fmemcpy(&lpFontTable[FontId].LogFont, lpLogFont, sizeof(LOGFONT));

hdc = SS_GetDC(lpBook);
hFontOld = SelectObject(hdc, hFont);
GetTextMetrics(hdc, &fm);

// Create array for Char widths.  SCP 12/31/98
nCh = max(fm.tmLastChar,255);

if (nCh < 1000) // RFW - 2/27/07 - 19756
	{
	lpFontTable[FontId].hCharWidths = tbGlobalAlloc(GSHARE, sizeof(int)*(nCh + 1));
	lpCharWidths = (int FAR *)tbGlobalLock(lpFontTable[FontId].hCharWidths);
	if (lpCharWidths)
	  {
	  UINT j;
	  GetCharWidth(hdc, 0, nCh, lpCharWidths);
	  if (fm.tmOverhang)
		 for (j = 0; j <= (UINT)nCh; j++)
			lpCharWidths[j] -= fm.tmOverhang;
	  tbGlobalUnlock(lpFontTable[FontId].hCharWidths);
	  }
	}
	// end of Char widths code

lpFontTable[FontId].dwFontLanguageInfo = GetFontLanguageInfo(hdc);

SelectObject(hdc, hFontOld);
SS_ReleaseDC(lpBook, hdc);

lpFontTable[FontId].FontHeight = (short)fm.tmHeight + 2;

if (fm.tmInternalLeading == 0 && fm.tmAscent > 0)
   Size = fm.tmAscent;
else
   Size = fm.tmHeight - fm.tmInternalLeading;

for (i = 0; i < sizeof(DefaultFontHeight) / sizeof(SS_FONTPOINTS); i++)
   if (Size <= DefaultFontHeight[i].Size)
      {
      FontCellHeight = (DefaultFontHeight[i].CellPoints * (double)Size) /
                       DefaultFontHeight[i].Size;
      break;
      }

if (FontCellHeight == 0.0 && i > 0)
   FontCellHeight = (DefaultFontHeight[i - 1].CellPoints * (double)Size) /
                    DefaultFontHeight[i - 1].Size;

lpFontTable[FontId].FontCellHeight = FontCellHeight;
return (FontId + 1);
}


void SS_FontTableReCreate(LPSS_BOOK lpBook)
{
LPSS_FONT lpFontTable;
short     i;

if (FontTable.hTable && FontTable.dTableCnt)
   {
   lpFontTable = SS_FontTableLock();

   for (i = 0; i < FontTable.dTableCnt; i++)
      {
      if (lpFontTable[i].DeleteFont && lpFontTable[i].hFont)
         {
         DeleteObject(lpFontTable[i].hFont);
         lpFontTable[i].hFont = 0;
         }
      // if we have an array of char widths, then free it.
      if (lpFontTable[i].hCharWidths)
         {
         tbGlobalFree(lpFontTable[i].hCharWidths);
         lpFontTable[i].hCharWidths = 0;
         }

		SSx_CreateFont(lpBook, lpFontTable, i, 0, &lpFontTable[i].LogFont, TRUE, TRUE);
      }

   SS_FontTableUnlock();
   }
}


LPSS_FONT SS_GetFont(LPSS_BOOK lpBook, LPSS_FONT Font, SS_FONTID FontId)
{
LPSS_FONT     lpFontTable;

lpFontTable = SS_FontTableLock();

if (FontId > 0 && FontId <= FontTable.dTableCnt)
   {
   _fmemcpy(Font, &lpFontTable[FontId - 1], sizeof(SS_FONT));
   SS_FontTableUnlock();
   return (Font);
   }

SS_FontTableUnlock();
return (NULL);
}


BOOL SS_SetFont(LPSS_BOOK lpBook, LPSS_FONT lpFont, SS_FONTID FontId)
{
LPSS_FONT lpFontTable;
BOOL      fRet = FALSE;

lpFontTable = SS_FontTableLock();

if (FontId > 0 && FontId <= FontTable.dTableCnt)
   {
   _fmemcpy(&lpFontTable[FontId - 1], lpFont, sizeof(SS_FONT));
   fRet = TRUE;
   }

SS_FontTableUnlock();
return (fRet);
}


BOOL SS_FontTableAlloc(void)
{
if (FontTable.dTableCnt >= FontTable.dTableAllocCnt)
   {
#ifdef WIN32
   if (!FontTable.hTable)
      InitializeCriticalSection(&FontTable.cs);

   EnterCriticalSection(&FontTable.cs);
#endif

   FontTable.hTable = SSx_AllocList(FontTable.hTable, &FontTable.dTableAllocCnt,
                                    sizeof(SS_FONT));

   if (!FontTable.hTable)
      {
      FontTable.dTableCnt = 0;
      FontTable.dTableAllocCnt = 0;
      }

#ifdef WIN32
   LeaveCriticalSection(&FontTable.cs);

   if (!FontTable.hTable)
      DeleteCriticalSection(&FontTable.cs);
#endif

   return (FontTable.hTable != 0);
   }

return (TRUE);
}


void SS_FontTableDelete(void)
{
LPSS_FONT lpFontTable;
short     i;

if (FontTable.hTable && FontTable.dTableCnt)
   {
#ifdef WIN32
   EnterCriticalSection(&FontTable.cs);
#endif
   lpFontTable = (LPSS_FONT)tbGlobalLock(FontTable.hTable);

   for (i = 0; i < FontTable.dTableCnt; i++)
      {
      if (lpFontTable[i].DeleteFont && lpFontTable[i].hFont)
         {
         DeleteObject(lpFontTable[i].hFont);
         lpFontTable[i].hFont = 0;
         }
      // if we have an array of char widths, then free it.
      if (lpFontTable[i].hCharWidths)
         {
         tbGlobalFree(lpFontTable[i].hCharWidths);
         lpFontTable[i].hCharWidths = 0;
         }
      }
   tbGlobalUnlock(FontTable.hTable);
   tbGlobalFree(FontTable.hTable);

   FontTable.hTable = 0;
   FontTable.dTableCnt = 0;
   FontTable.dTableAllocCnt = 0;

#ifdef WIN32
   LeaveCriticalSection(&FontTable.cs);
   DeleteCriticalSection(&FontTable.cs);
#endif
   }
}


LPSS_FONT SS_FontTableLock(void)
{
if (!FontTable.hTable)
   return (NULL);

#ifdef WIN32
EnterCriticalSection(&FontTable.cs);
#endif

return ((LPSS_FONT)tbGlobalLock(FontTable.hTable));
}


void SS_FontTableUnlock(void)
{
if (FontTable.hTable)
   {
   tbGlobalUnlock(FontTable.hTable);
#ifdef WIN32
   LeaveCriticalSection(&FontTable.cs);
#endif
   }
}


BOOL SS_SetRowMaxFont(LPSPREADSHEET lpSS, LPSS_ROW lpRow, SS_COORD Row, short FontIdNew)
{
LPSS_FONT lpFontTable;
LPSS_CELL lpCell;
short     dFontHeight = 0;
short     dFontHeightNew = 0;
short     FontId;
SS_COORD  i;
BOOL      fRowLocked = FALSE;	// RMA need to init to some value
BOOL      fRet = FALSE;
// 22993 -scl
//short     dRowHeightInPixelsOld;
long dRowHeightInPixelsOld;

if (!lpRow)
   {
   lpRow = SS_LockRowItem(lpSS, Row);
   fRowLocked = TRUE;
   }

lpFontTable = SS_FontTableLock();

FontId = lpRow->FontId;

if (FontId > 0)
   dFontHeight = lpFontTable[FontId - 1].FontHeight;

if (FontIdNew > 0)
   dFontHeightNew = lpFontTable[FontIdNew - 1].FontHeight;

if (dFontHeightNew >= dFontHeight)
	FontId = FontIdNew;
else
	{
	for (i = 0; i < lpSS->Col.AllocCnt; i++)
		if (lpCell = SS_LockCellItem(lpSS, i, Row))
			if (lpCell->FontId > 0)
				if (dFontHeight < lpFontTable[lpCell->FontId - 1].FontHeight)
					{
					dFontHeight = lpFontTable[lpCell->FontId - 1].FontHeight;
					FontId = lpCell->FontId;
					}
	}

SS_FontTableUnlock();

if (FontId > 0)
   lpRow->RowMaxFontId = FontId;
else
   lpRow->RowMaxFontId = SS_FONT_NONE;

dRowHeightInPixelsOld = (lpRow->dRowHeightInPixels == SS_WIDTH_DEFAULT 
#if SS_V80
                         || lpRow->dRowHeightInPixels == lpSS->Row.dCellSizeInPixels
#endif
                         ?
                         lpSS->Row.dCellSizeInPixels :
                         lpRow->dRowHeightInPixels);

if (lpRow->dRowHeightX100 != SS_WIDTH_DEFAULT || lpRow->RowMaxFontId != SS_FONT_NONE)
   lpRow->dRowHeightInPixels = SS_CalcRowHeight(lpSS, Row);
else
   lpRow->dRowHeightInPixels = SS_WIDTH_DEFAULT;

fRet = (dRowHeightInPixelsOld != lpRow->dRowHeightInPixels);

if (fRowLocked)
   SS_UnlockRowItem(lpSS, Row);

return (fRet);
}


void SS_DestroyPrinterFonts(LPSPREADSHEET lpSS)
{
LPSS_FONT lpFontTable;
SS_FONTID i;

if (lpFontTable = SS_FontTableLock())
   {
   for (i = 0; i < FontTable.dTableCnt; i++)
      {
      lpFontTable[i].nFontHeightPrinter = 0;

      if (lpFontTable[i].hFontPrinter)
         {
         DeleteObject(lpFontTable[i].hFontPrinter);
         lpFontTable[i].hFontPrinter = 0;
         }
      // if we have an array of char widths, then free it.
      if (lpFontTable[i].hCharWidthsPrinter)
         {
         tbGlobalFree(lpFontTable[i].hCharWidthsPrinter);
         lpFontTable[i].hCharWidthsPrinter = 0;
         }
      }

   SS_FontTableUnlock();
   }
}
