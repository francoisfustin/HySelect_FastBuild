/*********************************************************
* SS_SRCH.C
*
* Copyright (C) 1991-2001 - FarPoint Technologies
* All Rights Reserved.
*
* No part of this source code may be copied, modified or
* reproduced in any form without retaining the above
* copyright notice.  This source code, or source code
* derived from it, may not be redistributed without
* express written permission of FarPoint Technologies.
*********************************************************/

#include <windows.h>
#include <tchar.h>
#include <stdio.h>
#include <stddef.h>
#include <stdlib.h>
#include <string.h>
#include <process.h>
#include <math.h>
#include "spread.h"
#include "ss_cell.h"
#include "ss_col.h"
#include "ss_data.h"
#include "ss_main.h"
#include "ss_row.h"

#define SS_SEARCHFLAGS_SORTED           (SS_SEARCHFLAGS_SORTEDASCENDING | SS_SEARCHFLAGS_SORTEDDESCENDING)

typedef struct tagSS_SEARCH
   {
   WORD     wSearchFlags;
   SS_COORD lCoord;
   } SS_SEARCH, FAR *LPSS_SEARCH;
#if defined(_WIN64) || defined(_IA64)
int CALLBACK SS_SearchColCompare(LPSPREADSHEET lpSS, LPCTSTR lpszSearchText, SS_COORD lRow,
                                 LONG_PTR lUserData);
int CALLBACK SS_SearchRowCompare(LPSPREADSHEET lpSS, LPCTSTR lpszSearchText, SS_COORD lCol,
                                 LONG_PTR lUserData);
int CALLBACK SS_SearchCompare(LPSPREADSHEET lpSS, LPCTSTR lpszSearchText, SS_COORD lRow, SS_COORD lCol,
                              LONG_PTR lUserData);
long SS_SearchBinary(LPSPREADSHEET lpSS, LPCTSTR lpKey, SS_COORD lStart, SS_COORD lEnd,
                     LONG_PTR lUserData,
                     int (CALLBACK *compare)(LPSPREADSHEET, LPCTSTR, SS_COORD, LONG_PTR));
long SS_SearchLinear(LPSPREADSHEET lpSS, LPCTSTR lpKey, SS_COORD lStart, SS_COORD lEnd,
                     LONG_PTR lUserData,
                     int (CALLBACK *compare)(LPSPREADSHEET, LPCTSTR, SS_COORD, LONG_PTR));
long static SS_SearchRewind(LPSPREADSHEET lpSS, LPCTSTR lpKey, SS_COORD lStart,
                            SS_COORD lEnd, LONG_PTR lUserData,
                            int (CALLBACK *compare)(LPSPREADSHEET, LPCTSTR, SS_COORD, LONG_PTR));
#else
int CALLBACK SS_SearchColCompare(LPSPREADSHEET lpSS, LPCTSTR lpszSearchText, SS_COORD lRow,
                                 long lUserData);
int CALLBACK SS_SearchRowCompare(LPSPREADSHEET lpSS, LPCTSTR lpszSearchText, SS_COORD lCol,
                                 long lUserData);
int CALLBACK SS_SearchCompare(LPSPREADSHEET lpSS, LPCTSTR lpszSearchText, SS_COORD lRow, SS_COORD lCol,
                              long lUserData);
long SS_SearchBinary(LPSPREADSHEET lpSS, LPCTSTR lpKey, SS_COORD lStart, SS_COORD lEnd,
                     long lUserData,
                     int (CALLBACK *compare)(LPSPREADSHEET, LPCTSTR, SS_COORD, long));
long SS_SearchLinear(LPSPREADSHEET lpSS, LPCTSTR lpKey, SS_COORD lStart, SS_COORD lEnd,
                     long lUserData,
                     int (CALLBACK *compare)(LPSPREADSHEET, LPCTSTR, SS_COORD, long));
long static SS_SearchRewind(LPSPREADSHEET lpSS, LPCTSTR lpKey, SS_COORD lStart,
                            SS_COORD lEnd, long lUserData,
                            int (CALLBACK *compare)(LPSPREADSHEET, LPCTSTR, SS_COORD, long));
#endif

SS_COORD DLLENTRY SSSearchCol(HWND hWnd, SS_COORD lCol, SS_COORD lRowStart, SS_COORD lRowEnd,
                              LPCTSTR lpszText, WORD wSearchFlags)
{
LPSPREADSHEET lpSS;
SS_COORD      lRet;

lpSS = SS_SheetLock(hWnd);
SS_AdjustCellRangeCoords(lpSS, &lCol, &lRowStart, NULL, &lRowEnd);
lRet = SS_SearchCol(lpSS, lCol, lRowStart, lRowEnd, lpszText, wSearchFlags);
SS_AdjustCellCoordsOut(lpSS, NULL, &lRet);
SS_SheetUnlock(hWnd);
return lRet;
}


SS_COORD DLLENTRY SSSearchRow(HWND hWnd, SS_COORD lRow, SS_COORD lColStart, SS_COORD lColEnd,
                              LPCTSTR lpszText, WORD wSearchFlags)
{
LPSPREADSHEET lpSS;
SS_COORD      lRet;

lpSS = SS_SheetLock(hWnd);
SS_AdjustCellRangeCoords(lpSS, &lColStart, &lRow, &lColEnd, NULL);
lRet = SS_SearchRow(lpSS, lRow, lColStart, lColEnd, lpszText, wSearchFlags);
SS_AdjustCellCoordsOut(lpSS, &lRet, NULL);
SS_SheetUnlock(hWnd);
return lRet;

}


SS_COORD SS_SearchCol(LPSPREADSHEET lpSS, SS_COORD lCol, SS_COORD lRowStart, SS_COORD lRowEnd,
                      LPCTSTR lpszText, WORD wSearchFlags)
{
SS_SEARCH Search;
SS_COORD  lColStart, lColEnd;
SS_COORD  lColCnt, lRowCnt;
long      lRet = -1;

if (lRowStart >= SS_GetRowCnt(lpSS))
   return (lRet);

/* RFW - 4/1/08 - 22101
if (lpszText && *lpszText)
*/
if (lpszText && *lpszText && lCol >= lpSS->Col.HeaderCnt)
	{
	lColCnt = lpSS->Col.DataCnt;
	lRowCnt = lpSS->Row.DataCnt;
	}
else
	{
	lColCnt = SS_GetColCnt(lpSS);
	lRowCnt = SS_GetRowCnt(lpSS);
	}

// The code was written so if lCol == -1, then search all cols, but it would be difficult to return
// the found column.  It would be easier for the user to loop through all columns.
if (lCol < 0 || lCol >= lColCnt)
   return (lRet);

lRowStart = max(lpSS->Row.HeaderCnt - 1, lRowStart);
lRowEnd = lRowEnd == SS_ALLROWS ? lRowCnt - 1 : min(lRowEnd, lRowCnt - 1);

Search.wSearchFlags = wSearchFlags;

if (lCol == SS_ALLCOLS)
	{
	lColStart = lpSS->Col.HeaderCnt;
	lColEnd = lColCnt - 1;
	}
else
	lColStart = lColEnd = lCol;

lRowStart++;
if (lRowStart <= lRowEnd)
   {
	for (lCol = lColStart; lCol <= lColEnd && lRet < 0; lCol++)
		{
		Search.lCoord = lCol;
		if ((wSearchFlags & SS_SEARCHFLAGS_SORTED) &&
          !(wSearchFlags & SS_SEARCHFLAGS_PARTIALMATCH))
			{
#if defined(_WIN64) || defined(_IA64)
			lRet = SS_SearchBinary(lpSS, lpszText, lRowStart, lRowEnd, (LONG_PTR)(LPVOID)&Search,
										  SS_SearchColCompare);
#else
			lRet = SS_SearchBinary(lpSS, lpszText, lRowStart, lRowEnd, (long)(LPVOID)&Search,
										  SS_SearchColCompare);
#endif
			if (lRet > lpSS->Row.HeaderCnt)
#if defined(_WIN64) || defined(_IA64)
				lRet = SS_SearchRewind(lpSS, lpszText, lRowStart, lRet - 1, (LONG_PTR)(LPVOID)&Search,
											  SS_SearchColCompare);
#else
				lRet = SS_SearchRewind(lpSS, lpszText, lRowStart, lRet - 1, (long)(LPVOID)&Search,
											  SS_SearchColCompare);
#endif
			}
		else
			{
#if defined(_WIN64) || defined(_IA64)
			lRet = SS_SearchLinear(lpSS, lpszText, lRowStart, lRowEnd,
										  (LONG_PTR)(LPVOID)&Search, SS_SearchColCompare);
#else
			lRet = SS_SearchLinear(lpSS, lpszText, lRowStart, lRowEnd,
										  (long)(LPVOID)&Search, SS_SearchColCompare);
#endif
			}
		}
   }

#if 0
if (lpSS->Virtual.fVirtualMode)
   {
   SS_VIRTUALSEARCH VirtualSearch;
   BOOL             fRet = 0;

   VirtualSearch.lpszSearchString = lpszText;
   VirtualSearch.wSearchType = (WORD)bSearchMethod;
   VirtualSearch.lRowFound = lRet;

   if (!lpSS->fVirtualSearchSent)
      {
      lpSS->fVirtualSearchSent = TRUE;
      fRet = (BOOL)SS_SendMessage(lpSS, LBM_VIRTUALSEARCH, (WPARAM)lpSS->lpBook->hWnd,
                                  (LPARAM)(LPVOID)&VirtualSearch);
      lpSS->fVirtualSearchSent = FALSE;
      }

   if (!fRet)
      lRet = -1;
   else
      {
      if (VirtualSearch.lRowFound == -1)
         lRet = lpSS->lRectDotPos;
      else
         lRet = VirtualSearch.lRowFound;
      }
   }
#endif

return (lRet);
}


SS_COORD SS_SearchRow(LPSPREADSHEET lpSS, SS_COORD lRow, SS_COORD lColStart, SS_COORD lColEnd,
                      LPCTSTR lpszText, WORD wSearchFlags)
{
SS_SEARCH Search;
SS_COORD  lRowStart, lRowEnd;
SS_COORD  lColCnt, lRowCnt;
long      lRet = -1;

if (lColStart >= SS_GetColCnt(lpSS))
   return (lRet);

if (lpszText && *lpszText && lRow >= lpSS->Row.HeaderCnt)
	{
	lColCnt = lpSS->Col.DataCnt;
	lRowCnt = lpSS->Row.DataCnt;
	}
else
	{
	lColCnt = SS_GetColCnt(lpSS);
	lRowCnt = SS_GetRowCnt(lpSS);
	}

// The code was written so if lRow == -1, then search all rows, but it would be difficult to return
// the found row.  It would be easier for the user to loop through all rows.
if (lRow < 0 || lRow >= lRowCnt)
   return (lRet);

lColStart = max(lpSS->Col.HeaderCnt - 1, lColStart);
lColEnd = lColEnd == SS_ALLCOLS ? lColCnt - 1 : min(lColEnd, lColCnt - 1);

Search.wSearchFlags = wSearchFlags;

if (lRow == SS_ALLCOLS)
	{
	lRowStart = lpSS->Row.HeaderCnt;
	lRowEnd = lRowCnt - 1;
	}
else
	lRowStart = lRowEnd = lRow;

lColStart++;
if (lColStart <= lColEnd)
   {
	for (lRow = lRowStart; lRow <= lRowEnd && lRet < 0; lRow++)
		{
		Search.lCoord = lRow;
		if ((wSearchFlags & SS_SEARCHFLAGS_SORTED) &&
          !(wSearchFlags & SS_SEARCHFLAGS_PARTIALMATCH))
			{
#if defined(_WIN64) || defined(_IA64)
			lRet = SS_SearchBinary(lpSS, lpszText, lColStart, lColEnd, (LONG_PTR)(LPVOID)&Search,
										  SS_SearchRowCompare);
#else
			lRet = SS_SearchBinary(lpSS, lpszText, lColStart, lColEnd, (long)(LPVOID)&Search,
										  SS_SearchRowCompare);
#endif
			if (lRet > lpSS->Col.HeaderCnt)
#if defined(_WIN64) || defined(_IA64)
				lRet = SS_SearchRewind(lpSS, lpszText, lColStart, lRet - 1, (LONG_PTR)(LPVOID)&Search,
											  SS_SearchRowCompare);
#else
				lRet = SS_SearchRewind(lpSS, lpszText, lColStart, lRet - 1, (long)(LPVOID)&Search,
											  SS_SearchRowCompare);
#endif
			}
		else
			{
#if defined(_WIN64) || defined(_IA64)
			lRet = SS_SearchLinear(lpSS, lpszText, lColStart, lColEnd,
										  (LONG_PTR)(LPVOID)&Search, SS_SearchRowCompare);
#else
			lRet = SS_SearchLinear(lpSS, lpszText, lColStart, lColEnd,
										  (long)(LPVOID)&Search, SS_SearchRowCompare);
#endif
			}
		}
	}

return (lRet);
}

#if defined(_WIN64) || defined(_IA64)
int CALLBACK SS_SearchColCompare(LPSPREADSHEET lpSS, LPCTSTR lpszSearchText, SS_COORD lRow,
                                 LONG_PTR lUserData)
#else
int CALLBACK SS_SearchColCompare(LPSPREADSHEET lpSS, LPCTSTR lpszSearchText, SS_COORD lRow,
                                 long lUserData)
#endif
{
LPSS_SEARCH lpSearch = (LPSS_SEARCH)lUserData;

return (SS_SearchCompare(lpSS, lpszSearchText, lRow, lpSearch->lCoord, lUserData));
}

#if defined(_WIN64) || defined(_IA64)
int CALLBACK SS_SearchRowCompare(LPSPREADSHEET lpSS, LPCTSTR lpszSearchText, SS_COORD lCol,
                                 LONG_PTR lUserData)
#else
int CALLBACK SS_SearchRowCompare(LPSPREADSHEET lpSS, LPCTSTR lpszSearchText, SS_COORD lCol,
                                 long lUserData)
#endif
{
LPSS_SEARCH lpSearch = (LPSS_SEARCH)lUserData;

return (SS_SearchCompare(lpSS, lpszSearchText, lpSearch->lCoord, lCol, lUserData));
}

#if defined(_WIN64) || defined(_IA64)
int CALLBACK SS_SearchCompare(LPSPREADSHEET lpSS, LPCTSTR lpszSearchText, SS_COORD lRow, SS_COORD lCol,
                              LONG_PTR lUserData)
#else
int CALLBACK SS_SearchCompare(LPSPREADSHEET lpSS, LPCTSTR lpszSearchText, SS_COORD lRow, SS_COORD lCol,
                              long lUserData)
#endif
{
TBGLOBALHANDLE hString;
LPSS_SEARCH    lpSearch = (LPSS_SEARCH)lUserData;
LPCTSTR        lpszItem = NULL;
int            iCode = 0;

if (lpSearch->wSearchFlags & SS_SEARCHFLAGS_VALUE)
   hString = SS_GetValue(lpSS, lCol, lRow);
else
	{
	SS_CELLTYPE CellType;
	SS_RetrieveCellType(lpSS, &CellType, NULL, lCol, lRow);
   hString = SS_GetData(lpSS, &CellType, lCol, lRow, FALSE);
	}

if (hString)
   lpszItem = (LPCTSTR)tbGlobalLock(hString);

/*********************
* Do the comparisons
*********************/

if (lpszSearchText && *lpszSearchText && (!lpszItem || !(*lpszItem)))
	/* RFW - 8/13/04 - 15036
   iCode = -1;
	*/
   iCode = -99;
else if (lpszItem && *lpszItem && (!lpszSearchText || !(*lpszSearchText)))
   iCode = 1;
else if (!lpszSearchText || !(*lpszSearchText) && (!lpszItem || !(*lpszItem)))
	iCode = 0;
else if (lpszSearchText && *lpszSearchText && lpszItem && *lpszItem)
	{
	if (lpSearch->wSearchFlags & SS_SEARCHFLAGS_PARTIALMATCH)
		{
		/*
		int iLenSearchText = lstrlen(lpszSearchText);
		int iLenItem = lstrlen(lpszItem);

		if (!(lpSearch->wSearchFlags & SS_SEARCHFLAGS_CASESENSITIVE))
			iCode = _tcsnicmp(lpszSearchText, lpszItem, min(iLenItem, iLenSearchText));
		else
			iCode = _tcsncmp(lpszSearchText, lpszItem, min(iLenItem, iLenSearchText));
		*/
		LPTSTR lpsz;

		if (!(lpSearch->wSearchFlags & SS_SEARCHFLAGS_CASESENSITIVE))
			lpsz = StriStr(lpszItem, lpszSearchText);
		else
			lpsz = _tcsstr(lpszItem, lpszSearchText);

		iCode = lpsz ? 0 : 1;
		}
	else
		{
		if (!(lpSearch->wSearchFlags & SS_SEARCHFLAGS_CASESENSITIVE))
	  		iCode = lstrcmpi(lpszSearchText, lpszItem);					
		else
			iCode = lstrcmp(lpszSearchText, lpszItem);
		}
	}

if (lpSearch->wSearchFlags & SS_SEARCHFLAGS_SORTEDDESCENDING)
   iCode = (iCode == 1 ? -1 : (iCode == -1 ? 1 : 0));

/*
if (lpSearch->wSearchFlags & SS_SEARCHFLAGS_GREATEROREQUAL)
   if (iCode < 0)
      iCode = 0;
*/

/* RFW - 3/8/05 - 15859
if (hString)
   tbGlobalUnlock(hString);
*/
if (hString)
	{
   tbGlobalUnlock(hString);
	if (lpSearch->wSearchFlags & SS_SEARCHFLAGS_VALUE)
	   tbGlobalFree(hString);
	}

return (iCode);
}

#if defined(_WIN64) || defined(_IA64)
long SS_SearchBinary(LPSPREADSHEET lpSS, LPCTSTR lpKey, SS_COORD lStart, SS_COORD lEnd,
                     LONG_PTR lUserData,
                     int (CALLBACK *compare)(LPSPREADSHEET, LPCTSTR, SS_COORD, LONG_PTR))
#else
long SS_SearchBinary(LPSPREADSHEET lpSS, LPCTSTR lpKey, SS_COORD lStart, SS_COORD lEnd,
                     long lUserData,
                     int (CALLBACK *compare)(LPSPREADSHEET, LPCTSTR, SS_COORD, long))
#endif
{
long lLeft;
long lRight;
long lMiddle;
long lHalf;
long lNum = lEnd - lStart + 1;
int  iResult;

lLeft = lStart;
lRight = lEnd;

while (lLeft <= lRight)
   {
   if (lHalf = lNum / 2)
      {
      lMiddle = lLeft + ((lNum & 1) ? lHalf : (lHalf - 1));
      iResult = (*compare)(lpSS, lpKey, lMiddle, lUserData);
      if (!iResult)                     // match!
         return (lMiddle);
      else if (iResult < 0)
         {
         lRight = lMiddle - 1;
         lNum = lNum & 1 ? lHalf : lHalf - 1;
         }
      else
         {
         lLeft = lMiddle + 1;
         lNum = lHalf;
         }
      }
   else if (lNum)
      {
      iResult = (*compare)(lpSS, lpKey, lLeft, lUserData);
      if (!iResult)
         return ((long)lLeft);
      else
			{
			LPSS_SEARCH lpSearch = (LPSS_SEARCH)lUserData;

			if (lpSearch->wSearchFlags & SS_SEARCHFLAGS_GREATEROREQUAL)
				return (SS_SearchLinear(lpSS, lpKey, max(lStart, lLeft - 1),
					                     min(lLeft + 1, lEnd), lUserData, compare));
			else
				return (-1);
			}
      }
   else
      break;
   }

return (-1);
}  


#if defined(_WIN64) || defined(_IA64)
long SS_SearchLinear(LPSPREADSHEET lpSS, LPCTSTR lpKey, SS_COORD lStart, SS_COORD lEnd,
                     LONG_PTR lUserData,
                     int (CALLBACK *compare)(LPSPREADSHEET, LPCTSTR, SS_COORD, LONG_PTR))
#else
long SS_SearchLinear(LPSPREADSHEET lpSS, LPCTSTR lpKey, SS_COORD lStart, SS_COORD lEnd,
                     long lUserData,
                     int (CALLBACK *compare)(LPSPREADSHEET, LPCTSTR, SS_COORD, long))
#endif
{
LPSS_SEARCH lpSearch = (LPSS_SEARCH)lUserData;
SS_COORD    i;
int         iRet;

for (i = lStart; i <= lEnd; i++)
	{
   iRet = (*compare)(lpSS, lpKey, i, lUserData);
	/* RFW - 8/13/04 - 15036
	if (iRet == 0 ||
       ((lpSearch->wSearchFlags & SS_SEARCHFLAGS_GREATEROREQUAL) && iRet < 0))
	*/
	if (iRet == 0 ||
       ((lpSearch->wSearchFlags & SS_SEARCHFLAGS_GREATEROREQUAL) && iRet == -1))
      return (i);
	}

return (-1);
}

#if defined(_WIN64) || defined(_IA64)
long static SS_SearchRewind(LPSPREADSHEET lpSS, LPCTSTR lpKey, SS_COORD lStart,
                            SS_COORD lEnd, LONG_PTR lUserData,
                            int (CALLBACK *compare)(LPSPREADSHEET, LPCTSTR, SS_COORD, LONG_PTR))
#else
long static SS_SearchRewind(LPSPREADSHEET lpSS, LPCTSTR lpKey, SS_COORD lStart,
                            SS_COORD lEnd, long lUserData,
                            int (CALLBACK *compare)(LPSPREADSHEET, LPCTSTR, SS_COORD, long))
#endif
{
LPSS_SEARCH lpSearch = (LPSS_SEARCH)lUserData;
SS_COORD    i;
int         iRet;

for (i = lEnd; i >= lStart; i--)
	{
   iRet = (*compare)(lpSS, lpKey, i, lUserData);

	if (iRet < 0 ||
       (!(lpSearch->wSearchFlags & SS_SEARCHFLAGS_GREATEROREQUAL) && iRet > 0))
      return (i + 1);
	}

return (lStart);
}
