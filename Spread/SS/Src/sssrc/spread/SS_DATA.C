/*********************************************************
* SS_DATA.C
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
#include <tchar.h>
#include <stdio.h>
#include <stddef.h>
#include <stdlib.h>
#include <string.h>
#include <process.h>
#include <math.h>
#include "fptools.h"
#include "spread.h"
#include "ss_alloc.h"
#include "ss_bool.h"
#include "ss_calc.h"
#include "ss_cell.h"
#include "ss_col.h"
#include "ss_data.h"
#include "ss_draw.h"
#include "ss_emode.h"
#include "ss_main.h"
#include "ss_row.h"
#include "ss_scrol.h"
#include "ss_type.h"
#include "ss_user.h"

#ifdef SS_USEAWARE
#include "utools.h"
#endif

BOOL DLLENTRY           SSx_SetData(LPSPREADSHEET  lpSS,
                                    SS_COORD x, SS_COORD y, LPCTSTR lpData,
                                    BOOL fValue, LPBOOL lpfSetVScrollBar,
                                    BOOL fIgnoreLockedCells, BOOL fCheckLen);


BOOL DLLENTRY SSClearData(HWND hWnd, SS_COORD Col, SS_COORD Row)
{
LPSPREADSHEET lpSS;
BOOL bRet;

lpSS = SS_SheetLock(hWnd);
SS_AdjustCellCoords(lpSS, &Col, &Row);
bRet = SS_ClearDataRange(lpSS, Col, Row, Col, Row, FALSE, FALSE);
SS_SheetUnlock(hWnd);
return bRet;
}


BOOL DLLENTRY SSClearDataRange(HWND hWnd, SS_COORD Col, SS_COORD Row,
                               SS_COORD Col2, SS_COORD Row2)
{
LPSPREADSHEET lpSS;
BOOL bRet;

lpSS = SS_SheetLock(hWnd);
SS_AdjustCellRangeCoords(lpSS, &Col, &Row, &Col2, &Row2);
bRet = SS_ClearDataRange(lpSS, Col, Row, Col2, Row2, FALSE, FALSE);
SS_SheetUnlock(hWnd);
return bRet;
}


BOOL SS_ClearDataRange(LPSPREADSHEET lpSS,
                       SS_COORD Col, SS_COORD Row,
                       SS_COORD Col2, SS_COORD Row2,
                       BOOL fIgnoreLockedCells,
                       BOOL fSendDataChange)
{
LPSS_ROW      lpRow;
LPSS_CELL     lpCell;
SS_CELLTYPE   CellType;
SS_COORD      ColAtOld;
SS_COORD      ColLeftOld;
SS_COORD      RowAtOld;
SS_COORD      RowTopOld;
#ifdef SS_OLDCALC
SS_COORD      ColAt;
SS_COORD      RowAt;
#endif
SS_COORD      i;
SS_COORD      j;
BOOL          fEditMode = FALSE;
BOOL          Ret = FALSE;
SS_COORD      lColOut;
SS_COORD      lRowOut;

if (SS_ALLCOLS == Col || SS_ALLCOLS == Col2)
   Col = Col2 = SS_ALLCOLS;
if (SS_ALLROWS == Row || SS_ALLROWS == Row2)
   Row = Row2 = SS_ALLROWS;
// was not freeing default data when -1, -1 passed -scl
if( SS_ALLCOLS == Col && SS_ALLROWS == Row )
{
  SSx_FreeData(&lpSS->DefaultData);
  SS_InvalidateScrollArea(lpSS, SS_SCROLLAREA_ALL);
}

ColAtOld = lpSS->Col.CurAt;
ColLeftOld = lpSS->Col.UL;
RowAtOld = lpSS->Row.CurAt;
RowTopOld = lpSS->Row.UL;

if (Col == -1)
   Col = lpSS->Col.HeaderCnt;

if (Col == -1 || Col2 == -1)
   Col2 = lpSS->Col.DataCnt - 1;

if (Row == -1)
   Row = lpSS->Row.HeaderCnt;

if (lpSS->fVirtualMode)
   Row = max(Row, lpSS->Virtual.VTop);

if (Row == -1 || Row2 == -1)
   Row2 = lpSS->Row.DataCnt - 1;

if (lpSS->wOpMode == SS_OPMODE_ROWMODE)
   {
   SS_CellEditModeOff(lpSS, 0);
   SS_HighlightCell(lpSS, FALSE);
   }

else if (lpSS->lpBook->EditModeOn &&
         lpSS->Col.CurAt >= Col && lpSS->Col.CurAt <= Col2 &&
         lpSS->Row.CurAt >= Row && lpSS->Row.CurAt <= Row2)
   fEditMode = TRUE;

for (i = Row; i <= Row2; i++)
   {
   if (lpRow = SS_LockRowItem(lpSS, i))
      {
      for (j = Col; j <= Col2; j++)
         {
         if (lpCell = SSx_LockCellItem(lpSS, lpRow, j, i))
            {
            SS_RetrieveCellType(lpSS, &CellType, lpCell, j, i);
            if (!fIgnoreLockedCells ||
                !(SS_GetLock(lpSS, j, i, TRUE) ||
                  CellType.Type == SS_TYPE_STATICTEXT))
               {
               if (lpCell->Data.bDataType)
                  {
                  SSx_FreeData(&lpCell->Data);
						if (lpSS->lpBook->fAllowCellOverflow)
							{
	                  SS_ResetCellOverflow(lpSS, j, i);
							SS_OverflowAdjustNeighbor(lpSS, j, i);
							}

                  if (fSendDataChange && lpSS->lpBook->hWnd)
                     SS_SendMsgCoords(lpSS, SSM_DATACHANGE,
                                      GetDlgCtrlID(lpSS->lpBook->hWnd), j, i);
                  
                  #ifdef SS_OLDCALC
                  if (lpSS->lpBook->CalcAuto)
                     {
                     ColAt = lpSS->Col.CurAt;
                     RowAt = lpSS->Row.CurAt;
                     lpSS->Col.CurAt = j;
                     lpSS->Row.CurAt = i;
                     SS_CalcDependencies(lpSS, j, i);
                     lpSS->Col.CurAt = ColAt;
                     lpSS->Row.CurAt = RowAt;
                     }
                  #elif !defined(SS_NOCALC)
						lColOut = j;
						lRowOut = i;
						SS_AdjustCellCoordsOut(lpSS, &lColOut, &lRowOut);
                  CalcMarkDependForEval(&lpSS->CalcInfo, lColOut, lRowOut);
                  #endif
                  }
               }

            SSx_UnlockCellItem(lpSS, lpRow, j, i);
            }
         }

      SS_UnlockRowItem(lpSS, i);
      }
   }

#if !defined(SS_NOCALC) && !defined(SS_OLDCALC)
if (lpSS->lpBook->CalcAuto)
  CalcEvalNeededCells(&lpSS->lpBook->CalcInfo);
#endif

/*
if (SS_SetDataRange(lpSS, Col, Row, Col2, Row2, NULL, FALSE,
                    fIgnoreLockedCells, TRUE, TRUE))
*/
   {
//   if ((Row == SS_ALLROWS || Row >= lpSS->Row.HeaderCnt) &&
//       (Row2 == SS_ALLROWS || Row2 <= lpSS->Row.DataCnt - 1) &&
//       Col2 >= lpSS->Col.DataCnt - 1)
//      SS_SetDataColCnt(lpSS, max(lpSS->Col.HeaderCnt, Col));

   if ((Col == SS_ALLCOLS || Col >= lpSS->Col.HeaderCnt) &&
       (Col2 == SS_ALLCOLS || Col2 <= lpSS->Col.DataCnt - 1) &&
       Row2 >= lpSS->Row.DataCnt - 1)
      {
//      SS_SetDataRowCnt(lpSS, max(lpSS->Row.HeaderCnt, Row));

      if (lpSS->wOpMode == SS_OPMODE_ROWMODE &&
          lpSS->Row.CurAt > lpSS->Row.DataCnt)
         lpSS->Row.CurAt = lpSS->Row.DataCnt;
      }

   SS_AdjustDataColCnt(lpSS, Col, Col2);
   SS_AdjustDataRowCnt(lpSS, Row, Row2);

   Ret = TRUE;
   }

SS_InvalidateRange(lpSS, Col, Row, Col2, Row2);

if (lpSS->wOpMode == SS_OPMODE_ROWMODE)
   {
   if (lpSS->fRowModeEditing)
      {
      lpSS->fRowModeEditing = FALSE;
#ifndef SS_NOBUTTONDRAWMODE
      SS_InvalidateButtons(lpSS, ColAtOld, ColLeftOld,
                           RowAtOld, RowTopOld);
#endif
      }

   SS_HighlightCell(lpSS, TRUE);
   }

else if (fEditMode)
   SS_SetEditModeOnData(lpSS);

return (Ret);
}


GLOBALHANDLE DLLENTRY SSClipOut(hWnd, Col, Row, Col2, Row2)

HWND           hWnd;
SS_COORD       Col;
SS_COORD       Row;
SS_COORD       Col2;
SS_COORD       Row2;
{
LPSPREADSHEET lpSS;
GLOBALHANDLE hRet;

lpSS = SS_SheetLock(hWnd);
SS_AdjustCellCoords(lpSS, &Col, &Row);
SS_AdjustCellCoords(lpSS, &Col2, &Row2);
hRet = SS_ClipOut(lpSS, Col, Row, Col2, Row2, FALSE, FALSE);
SS_SheetUnlock(hWnd);
return hRet;
}


GLOBALHANDLE DLLENTRY SSClipValueOut(hWnd, Col, Row, Col2, Row2)

HWND           hWnd;
SS_COORD       Col;
SS_COORD       Row;
SS_COORD       Col2;
SS_COORD       Row2;
{
LPSPREADSHEET lpSS;
GLOBALHANDLE hRet;

lpSS = SS_SheetLock(hWnd);
SS_AdjustCellCoords(lpSS, &Col, &Row);
SS_AdjustCellCoords(lpSS, &Col2, &Row2);
hRet = SS_ClipOut(lpSS, Col, Row, Col2, Row2, TRUE, FALSE);
SS_SheetUnlock(hWnd);
return hRet;
}
//>>99918148 Leon 20101104
GLOBALHANDLE SS_DoubleQuotes_Encapsulation(LPTSTR lpData)
{
	GLOBALHANDLE htemp;
	LPTSTR lpsz, lpsz2, lpsz3;
	int rIndex,rLen, rSize;	
	rSize = _tcslen(lpData) + 3;
	lpsz2 = lpData;						
	while(lpsz2)
	{			
		lpsz2 = _tcschr(lpsz2, '"');
		if(lpsz2)
		{
			rSize++;
			lpsz2 = lpsz2+1;		
		}
		else
			break;
	}
	htemp = GlobalAlloc(GMEM_MOVEABLE, rSize * sizeof(TCHAR));					
	lpsz = (LPTSTR)GlobalLock(htemp);					
	if( lpsz )
	{
		_tcscpy(lpsz, _T("\""));
		lpsz2 = lpData;						
		while(lpsz2!=NULL)
		{	
			lpsz3 = _tcschr(lpsz2, '"');
			if(lpsz3)
			{	
				rIndex = (int)(lpsz3 - lpsz2);							
				rLen = _tcslen(lpsz);
				if(rIndex!=0)
					_tcsncpy(lpsz + rLen, lpsz2, rIndex);
				_tcscpy(lpsz + rLen + rIndex, _T("\"\""));				
				lpsz2 = _tcschr(lpsz2, '"');
				lpsz2 = lpsz2+1;
			}
			else
			{							
				_tcscpy(lpsz+_tcslen(lpsz),lpsz2);							
				break;
			}						
		}											
		_tcscpy(lpsz + _tcslen(lpsz), _T("\""));
	}	
	tbGlobalUnlock(htemp);	
	return htemp;
}

TBGLOBALHANDLE SS_DoubleQuotes_Decapsulation(TBGLOBALHANDLE hTextInput, int newsize)
{
	TBGLOBALHANDLE hText1;
	LPTSTR lpsz1;
	LPTSTR lpsz2;
	int rIndex, rlen;
	BOOL fStripQuotesContinue = FALSE;	
	BOOL rnext;
	LPTSTR lpsz = (LPTSTR)tbGlobalLock(hTextInput);
	if(newsize == 0)	
		newsize = _tcslen(lpsz);	
	hText1 = tbGlobalAlloc(GMEM_MOVEABLE,newsize*sizeof(TCHAR));	
	lpsz1 = (LPTSTR)tbGlobalLock(hText1);	
	lpsz++;
	fStripQuotesContinue = TRUE;
	while(fStripQuotesContinue)
	{
		lpsz2 = _tcschr(lpsz,'"');
		if(lpsz2)
		{
			rIndex = (int)(lpsz2 - lpsz);
			rlen = _tcslen(lpsz1);									
			if(rIndex!=0)
			{										
				_tcsncpy(lpsz1 + rlen, lpsz, rIndex);
				_tcscpy(lpsz1 + rlen + rIndex, _T(""));	
				lpsz = _tcschr(lpsz,'"');
				continue;
			}
			else
			{										
				rnext = TRUE;										
				while(rnext)
				{											
					if(lpsz && lpsz[0] == '"' && lpsz++)
					{
						if(lpsz[0] == '"')
						{
							_tcscpy(lpsz1 + _tcslen(lpsz1), _T("\""));
							lpsz++;													
						}
						else
						{
							rnext = FALSE;
							break;
						}
					}
					else
						break;											
				}
				if(rnext)
					continue;
			}
			if(lpsz)
			{
				_tcscpy(lpsz1 + _tcslen(lpsz1), lpsz);
			}
			fStripQuotesContinue = FALSE;
			break;
		}
		else
		{
			break;
		}								
	}
	tbGlobalUnlock(hText1);
	tbGlobalUnlock(hTextInput);
	tbGlobalFree(hTextInput);
	hTextInput = hText1;
	return hText1;
}
//<<99918148 Leon 20101104
GLOBALHANDLE SS_ClipOut(LPSPREADSHEET  lpSS, SS_COORD Col, SS_COORD Row,
                        SS_COORD Col2, SS_COORD Row2, BOOL fValue, BOOL fClipboard)
{
GLOBALHANDLE   hBuffer = 0;
TBGLOBALHANDLE hData;
SS_CELLTYPE    CellType;
HPBYTE         Buffer;
LPTSTR         lpData;
long           BufferLen = 0;
long           AllocLen = 0;
long           Len;
SS_COORD       x;
SS_COORD       y;
//>>99918148 Leon 20101025
GLOBALHANDLE temp;
LPTSTR				 lpsz;
//<<99918148 Leon 20101025

if (Col == -1)
   {
   Col = 0;
   Col2 = lpSS->Col.DataCnt - 1;
   }
else if (Col2 == -1)
   Col2 = lpSS->Col.DataCnt - 1;

if (Row == -1)
   {
   // BJO 25Aug97 JAP7654 - Before fix
   //Row = (lpSS->fVirtualMode ? lpSS->Virtual.VTop : 0);
   // BJO 25Aug97 JAP7654 - Begin fix
   Row = 0;
   // BJO 25Aug97 JAP7654 - End fix
   Row2 = lpSS->Row.DataCnt - 1;
   }
else if (Row2 == -1)
   Row2 = lpSS->Row.DataCnt - 1;

if (Row >= lpSS->Row.HeaderCnt)
   Col2 = min(Col2, lpSS->Col.DataCnt - 1);

if (Col >= lpSS->Col.HeaderCnt)
   Row2 = min(Row2, lpSS->Row.DataCnt - 1);

if (Col > Col2 || Row > Row2)
   return (0);

for (y = Row; y <= Row2; y++)
   for (x = Col; x <= Col2; x++)
      {
#ifdef SS_UTP
      if (lpSS->fUseScrollArrows && x == lpSS->Col.HeaderCnt +
          lpSS->Col.Frozen - 1)
         continue;
#endif

      SS_RetrieveCellType(lpSS, &CellType, NULL, x, y);

		// If clipbard copy, then ignore password cells
		if (fClipboard && SS_TYPE_EDIT == CellType.Type && (ES_PASSWORD & CellType.Style))
			;
		else
			{
			if (fValue)
				hData = SS_GetValue(lpSS, x, y);
			else
				hData = SS_GetData(lpSS, &CellType, x, y, FALSE);

			if (hData)
				{
				lpData = (LPTSTR)tbGlobalLock(hData);
				//>>99918148 Leon 20101025
				if( CellType.Type == SS_TYPE_EDIT && (CellType.Style & ES_MULTILINE) && (_tcstok(lpData, _T("\t")) || _tcstok(lpData, _T("\r"))))
				{
					temp = SS_DoubleQuotes_Encapsulation(lpData);
					lpsz = (LPTSTR)GlobalLock(temp);		
				}
				else
				{
					lpsz = NULL;
				}				
				if(!lpsz)
				{				
					Len = lstrlen(lpData);
// 26939 -scl
/*            // 25724 -scl
            if( CellType.Type == SS_TYPE_EDIT && (CellType.Style & ES_MULTILINE) && (_tcstok(lpData, _T("\t")) || _tcstok(lpData, _T("\r"))) )
            {
               LPTSTR lpszQuote;
				   Buffer = SS_HugeBufferAlloc(Buffer, &BufferLen, _T("\""),
													 sizeof(TCHAR),
													 &AllocLen, &hBuffer);
               lpszQuote = _tcstok(lpData, _T("\""));
               if( lpszQuote )
               { // double quote chars
                  do
                  {
   				      Buffer = SS_HugeBufferAlloc(Buffer, &BufferLen, lpData,
													 lpszQuote - lpData,
													 &AllocLen, &hBuffer);
				         Buffer = SS_HugeBufferAlloc(Buffer, &BufferLen, _T("\""),
													 sizeof(TCHAR),
													 &AllocLen, &hBuffer);
                  } while ( lpszQuote = _tcstok(lpszQuote + 1, _T("\"")) );
               }
               else
   				   Buffer = SS_HugeBufferAlloc(Buffer, &BufferLen, lpData,
													 Len * sizeof(TCHAR),
													 &AllocLen, &hBuffer);
				   Buffer = SS_HugeBufferAlloc(Buffer, &BufferLen, _T("\""),
													 sizeof(TCHAR),
													 &AllocLen, &hBuffer);
            }
            else
*/ // ^- not ready for prime-time -scl
				   Buffer = SS_HugeBufferAlloc(Buffer, &BufferLen, lpData,
													 Len * sizeof(TCHAR),
													 &AllocLen, &hBuffer);
				}
				else
				{
					Len = lstrlen(lpsz);
					Buffer = SS_HugeBufferAlloc(Buffer, &BufferLen, lpsz,
						Len * sizeof(TCHAR),
													 &AllocLen, &hBuffer);
					GlobalUnlock(temp);
					GlobalFree(temp);
				}
                                //<<99918148 Leon 20101025
				tbGlobalUnlock(hData);

				if (fValue)
					tbGlobalFree(hData);
				}
			}

      if (x == Col2)
         Buffer = SS_HugeBufferAlloc(Buffer, &BufferLen, _TEXT("\r\n"),
                                     2 * sizeof(TCHAR), &AllocLen, &hBuffer);
      else
         Buffer = SS_HugeBufferAlloc(Buffer, &BufferLen, _TEXT("\t"),
                                     sizeof(TCHAR), &AllocLen, &hBuffer);
      }

Buffer = SS_HugeBufferAlloc(Buffer, &BufferLen, NULL, sizeof(TCHAR),
                            &AllocLen, &hBuffer);

if (hBuffer)
   GlobalUnlock(hBuffer);
return (hBuffer);
}


BOOL DLLENTRY SSClipIn(HWND hWnd, SS_COORD Col, SS_COORD Row,
                       SS_COORD Col2, SS_COORD Row2,
                       LPCTSTR Buffer, long BufferLen)
{
LPSPREADSHEET lpSS;
BOOL bRet;

lpSS = SS_SheetLock(hWnd);
SS_AdjustCellRangeCoords(lpSS, &Col, &Row, &Col2, &Row2);
bRet = SS_ClipIn(lpSS, Col, Row, Col2, Row2, Buffer, BufferLen, FALSE,
                 FALSE, FALSE, NULL, NULL);
SS_SheetUnlock(hWnd);
return bRet;
}


BOOL DLLENTRY SSClipValueIn(HWND hWnd, SS_COORD Col, SS_COORD Row,
                            SS_COORD Col2, SS_COORD Row2,
                            LPCTSTR Buffer, long BufferLen)
{
LPSPREADSHEET lpSS;
BOOL bRet;

lpSS = SS_SheetLock(hWnd);
SS_AdjustCellRangeCoords(lpSS, &Col, &Row, &Col2, &Row2);
bRet = SS_ClipIn(lpSS, Col, Row, Col2, Row2, Buffer, BufferLen, TRUE, FALSE,
                 FALSE, NULL, NULL);
SS_SheetUnlock(hWnd);
return bRet;
}


BOOL SS_ClipIn(LPSPREADSHEET lpSS, SS_COORD Col, SS_COORD Row,
               SS_COORD Col2, SS_COORD Row2,
               HPCTSTR Buffer, long BufferLen,
               BOOL fValue, BOOL fValidate,  BOOL fPaste,
               LPSS_COORD lpColCnt, LPSS_COORD lpRowCnt)
{
TBGLOBALHANDLE hText = 0;
HPCTSTR        szPtr;
HPCTSTR        szDataPtr;
LPTSTR         lpszText = NULL;
long           lTextLen;
long           lTextAlloc = 0;
BOOL           RedrawOld;
SS_CELLTYPE    CellType;
SS_COORD       DataColCntOld;
SS_COORD       DataRowCntOld;
SS_COORD       ColsMax;
SS_COORD       RowsMax;
SS_COORD       ColAt;
SS_COORD       RowAt;
SS_COORD       x;
SS_COORD       y;
BOOL           fHighlightOn;
//>>99918148 Leon 20101025
BOOL           fCellStart = TRUE;
BOOL           fStripQuotes = FALSE;
BOOL           fStripQuotesContinue = FALSE;
int            newsize;
//<<99918148 Leon 20101025

if (lpRowCnt)
   *lpRowCnt = 0;

if (lpColCnt)
   *lpColCnt = 0;

if (!Buffer || !BufferLen)
   return (TRUE);

DataColCntOld = lpSS->Col.DataCnt;
DataRowCntOld = lpSS->Row.DataCnt;

ColsMax = SS_GetColCnt(lpSS);
RowsMax = SS_GetRowCnt(lpSS);

if (Col == -1 || Col2 == -1)
   Col2 = ColsMax - 1;

if (Col == -1)
   Col = 0;

if (Row == -1 || Row2 == -1)
   Row2 = RowsMax - 1;

if (Row == -1)
   Row = 0;

if (Col > Col2 || Row > Row2)
   {
   return (0);
   }

szDataPtr = Buffer;
szPtr = Buffer;
x = Col;
y = Row;

RedrawOld = lpSS->lpBook->Redraw;
lpSS->lpBook->Redraw = FALSE;

ColAt = lpSS->Col.CurAt;
RowAt = lpSS->Row.CurAt;

if (lpSS->lpBook->CalcAuto && RedrawOld)
   {
   fHighlightOn = lpSS->HighlightOn;
   SS_HighlightCell(lpSS, FALSE);
   lpSS->FreezeHighlight = TRUE;
   }

// Yagi No.6 1997/9/9 Wei Feng
#ifdef	SPREAD_JPN
//SS_RetrieveCellType(lpSS, &CellType, NULL, x, y);
//-----------Deleted by dean 1999/04/16------------
//-To fix bug 01205
//if (CellType.Type == 2 && (CellType.Style & ES_MULTILINE))
//{
//    if (!SS_ClipData(lpSS, x, y, szDataPtr, fValue, fValidate))
//       return (0);
//}
//else
//--------------------------------------------------
#endif

while ((szPtr <= Buffer + BufferLen || BufferLen == -1) && y <= Row2)
   {
     // 25724 -scl //>>99918148 Leon 20101025
	   if(fCellStart)
		 {			
			 SS_RetrieveCellType(lpSS, &CellType, NULL, x, y);
			 if( SS_TYPE_EDIT == CellType.Type && (CellType.Style & ES_MULTILINE) && *szPtr == _T('\"') )
			 {				 
				 newsize = 2;
				 fStripQuotes = TRUE;
				 fStripQuotesContinue = TRUE;
				 szPtr++;
			 }
			 fCellStart= FALSE;			 
			 continue;
		 }
		 if(fStripQuotesContinue)
		 {
			 if(*szPtr == _T('\"'))
			 {
				 szPtr++;
				 if(*szPtr == _T('\"'))				 
				 {
					 newsize++;
					 szPtr++;
					 continue;			 
				 }
				 fStripQuotesContinue = FALSE;				 
			 }
			 else
			 {
				 szPtr++;
				 continue;
			 }
		 }
		 //<<99918148 Leon 20101025
   //----------Changed by dean 1999/04/16-------------
   //-To fix bug 01205
//GAB 01/07/02 Changed for Unicode
  //   if ((*szPtr == '\t' || *szPtr == '\r' || *szPtr == '\0'))
  if (((*szPtr == _T('\t') || *szPtr == _T('\r') || *szPtr == _T('\0'))))
   //if (*szPtr == '\t' || *szPtr == '\0' 
   //	   || (*szPtr == '\r' && !(CellType.Type == 2 && (CellType.Style & ES_MULTILINE) && *(szPtr+1) == 0x0A)))
   //-------------------------------------------------
      {
#ifdef SS_UTP
      if (lpSS->fUseScrollArrows && x == lpSS->Col.HeaderCnt +
          lpSS->Col.Frozen - 1)
         x++;
#endif

      if (x <= Col2)
         {
         if (lpColCnt && x > *lpColCnt)
            *lpColCnt = x;
            
         if (lpRowCnt && y > *lpRowCnt)
            *lpRowCnt = y;

         SS_RetrieveCellType(lpSS, &CellType, NULL, x, y);
         if (!fValidate || x < lpSS->Col.HeaderCnt || y < lpSS->Row.HeaderCnt ||
             !(SS_GetLock(lpSS, x, y, TRUE) || CellType.Type == SS_TYPE_STATICTEXT))
            {
            lTextLen = (long)(szPtr - szDataPtr);
            if (lTextLen + 1 > lTextAlloc)
               {
               lTextAlloc = lTextLen + 1;
               if (hText)
                  {
                  tbGlobalUnlock(hText);
                  hText = tbGlobalReAlloc(hText,lTextAlloc*sizeof(TCHAR),GMEM_MOVEABLE);
                  }
               else
                  {
                  hText = tbGlobalAlloc(GMEM_MOVEABLE,lTextAlloc*sizeof(TCHAR));
                  }
               lpszText = (LPTSTR)tbGlobalLock(hText);
               if (!lpszText)
                  return (0);
               }
            MemHugeCpy(lpszText, szDataPtr, lTextLen * sizeof(TCHAR));
						lpszText[lTextLen] = '\0';
						//>>99918148 Leon 20101025
						if(fStripQuotes)
						{
							tbGlobalUnlock(hText);
							if(lTextLen<3 || newsize<3)
								newsize = 0;								
							else
								newsize = lTextLen - newsize;
							hText = SS_DoubleQuotes_Decapsulation(hText, newsize);
							lpszText = (LPTSTR)tbGlobalLock(hText);
						}
						fStripQuotes=FALSE;
						fCellStart=TRUE;
						//<<99918148 Leon 20101025
						if (!SS_ClipData(lpSS, x, y, lpszText, fValue, fValidate))
               {
               if (hText)
                  {
                  tbGlobalUnlock(hText);
                  tbGlobalFree(hText);
                  }
               return (0);
               }
            }
#ifdef SPREAD_JPN
				// BUG SPR_EDT_002_001 (1-1)
				// When User Copy Data To A Locked Cell,
				// An Edit Error Event Need To Be Fired
				// Following Code Will Fire The Event.
				// Modified By HaHa 1999.10.28
				if (SS_GetLock(lpSS, x, y, TRUE))
					{
					if (!SS_SendMsgCoords(lpSS, SSM_CELLLOCKED, GetDlgCtrlID(lpSS->lpBook->hWnd), x, y))
						{
						// RFW - 10/25/02
						if (fPaste)
							SS_Beep(lpSS->lpBook);
						}
					}
#endif

         }
//GAB 01/07/02 Changed for Unicode
//      if (*szPtr == '\0')
      if (*szPtr == _T('\0'))
         break;

//GAB      else if (*szPtr == '\t')
      else if (*szPtr == _T('\t'))
         x++;

//GAB      else if (*szPtr == '\r')
       else if (*szPtr == _T('\r'))
         {
         y++;
         x = Col;

//GAB         if (*(szPtr + 1) == '\n')
         if (*(szPtr + 1) == _T('\n'))
            {
            szPtr++;
            }

//GAB         if (szPtr + 1 == Buffer + BufferLen || *(szPtr + 1) == '\0')
         if (szPtr + 1 == Buffer + BufferLen || *(szPtr + 1) == _T('\0'))
            {
            szPtr++;
            }
         }

      szDataPtr = szPtr;
      szDataPtr++;
      }

   szPtr++;
   }

if (hText)
   {
   tbGlobalUnlock(hText);
   tbGlobalFree(hText);
   }

if (lpSS->lpBook->CalcAuto && RedrawOld)
   {
   lpSS->FreezeHighlight = FALSE;
   SS_HighlightCell(lpSS, fHighlightOn);
   }

SS_InvalidateCellRange(lpSS, Col, Row, Col2, Row2);

SS_BookSetRedraw(lpSS->lpBook, RedrawOld);

SS_AdjustDataColCnt(lpSS, Col, Col2);
SS_AdjustDataRowCnt(lpSS, Row, Row2);

if (lpSS->Col.DataCnt > DataColCntOld)
   SS_SetHScrollBar(lpSS);

if (lpSS->Row.DataCnt > DataRowCntOld)
   SS_SetVScrollBar(lpSS);

return (TRUE);
}


BOOL SS_ClipData(LPSPREADSHEET lpSS, SS_COORD Col, SS_COORD Row,
                 LPTSTR lpszData, BOOL fValue, BOOL fSendDataChange)
{
SS_CELLTYPE CellType;
SS_DATA     DataItem;
#ifdef SS_OLDCALC
SS_COORD    ColAt;
SS_COORD    RowAt;
#endif
BOOL        fEditModeOn = FALSE;
#ifdef SPREAD_JPN
BOOL		bRet = TRUE;
#endif

if (lpSS->lpBook->EditModeOn && lpSS->Row.CurAt == Row && lpSS->Col.CurAt == Col)
   fEditModeOn = TRUE;

_fmemset(&DataItem, '\0', sizeof(SS_DATA));
SS_RetrieveCellType(lpSS, &CellType, NULL, Col, Row);
if (!SS_FormatData(lpSS, &DataItem, Col, Row, &CellType, lpszData, fValue, TRUE, TRUE))
// Yagi No.5 Wei Feng 1997/9/9
#ifdef	SPREAD_JPN
   bRet = FALSE;
#else
   DataItem.bDataType = 0;
#endif

#ifdef SS_UTP
if (lpszData && *lpszData && !DataItem.bDataType)
   lpSS->fInvalidDataClippedIn = TRUE;
#endif

// Yagi No.5 Wei Feng 1997/9/9
#ifdef	SPREAD_JPN
if (bRet && !SSxx_SetCellDataItem(lpSS, Col, Row, &DataItem))
#else
if (!SSxx_SetCellDataItem(lpSS, Col, Row, &DataItem))
#endif
   return (FALSE);

// Yagi No.5 Wei Feng 1997/9/9
#ifdef	SPREAD_JPN
if (bRet && fSendDataChange && lpSS->lpBook->hWnd)
#else
if (fSendDataChange && lpSS->lpBook->hWnd)
#endif
   SS_SendMsgCoords(lpSS, SSM_DATACHANGE,
                    GetDlgCtrlID(lpSS->lpBook->hWnd), Col, Row);

if (fEditModeOn)
   SS_SetEditModeOnData(lpSS);
 
#ifdef SS_OLDCALC
if (lpSS->CalcTableCnt && lpSS->lpBook->CalcAuto &&
    (SS_IsCellTypeFloatNum(CellType) || CellType.Type == SS_TYPE_INTEGER))
   {
   ColAt = lpSS->Col.CurAt;
   RowAt = lpSS->Row.CurAt;
   lpSS->Col.CurAt = Col;
   lpSS->Row.CurAt = Row;
   SS_CalcDependencies(lpSS, Col, Row);
   lpSS->Col.CurAt = ColAt;
   lpSS->Row.CurAt = RowAt;
   }
#elif !defined(SS_NOCALC)
SS_AdjustCellCoordsOut(lpSS, &Col, &Row);
CalcMarkDependForEval(&lpSS->CalcInfo, Col, Row);
if( lpSS->lpBook->CalcAuto )
  CalcEvalNeededCells(&lpSS->lpBook->CalcInfo);
#endif

return (TRUE);
}


BOOL DLLENTRY SSSetData(hWnd, Col, Row, lpData)

HWND     hWnd;
SS_COORD Col;
SS_COORD Row;
LPCTSTR  lpData;
{
return (SSSetDataRange(hWnd, Col, Row, Col, Row, lpData));
}


BOOL DLLENTRY SSSetDataRange(hWnd, Col, Row, Col2, Row2, lpData)

HWND          hWnd;
SS_COORD      Col;
SS_COORD      Row;
SS_COORD      Col2;
SS_COORD      Row2;
LPCTSTR       lpData;
{
LPSPREADSHEET lpSS;
BOOL          fRet;

lpSS = SS_SheetLock(hWnd);
SS_AdjustCellRangeCoords(lpSS, &Col, &Row, &Col2, &Row2);
fRet = SS_SetDataRange(lpSS, Col, Row, Col2, Row2, lpData, FALSE, FALSE, TRUE);
SS_SheetUnlock(hWnd);
return (fRet);
}


BOOL DLLENTRY SSSetValue(hWnd, Col, Row, lpData)

HWND     hWnd;
SS_COORD Col;
SS_COORD Row;
LPCTSTR  lpData;
{
return (SSSetValueRange(hWnd, Col, Row, Col, Row, lpData));
}


BOOL DLLENTRY SSSetValueRange(hWnd, Col, Row, Col2, Row2, lpData)

HWND     hWnd;
SS_COORD Col;
SS_COORD Row;
SS_COORD Col2;
SS_COORD Row2;
LPCTSTR  lpData;
{
LPSPREADSHEET lpSS;
BOOL          fRet;

lpSS = SS_SheetLock(hWnd);
SS_AdjustCellRangeCoords(lpSS, &Col, &Row, &Col2, &Row2);
fRet = SS_SetDataRange(lpSS, Col, Row, Col2, Row2, lpData, TRUE, FALSE, TRUE);
SS_SheetUnlock(hWnd);
return (fRet);
}


BOOL SS_SetValue(LPSPREADSHEET lpSS, SS_COORD Col, SS_COORD Row, LPCTSTR lpData)
{
return SS_SetDataRange(lpSS, Col, Row, Col, Row, lpData, TRUE, FALSE, TRUE);
}


BOOL SS_SetValueRange(LPSPREADSHEET lpSS, SS_COORD Col, SS_COORD Row, SS_COORD Col2, SS_COORD Row2, LPCTSTR lpData)
{
return SS_SetDataRange(lpSS, Col, Row, Col2, Row2, lpData, TRUE, FALSE, TRUE);
}


BOOL SS_SetData(LPSPREADSHEET lpSS, SS_COORD Col, SS_COORD Row, LPCTSTR lpData)
{
return SS_SetDataRange(lpSS, Col, Row, Col, Row, lpData, FALSE, FALSE, TRUE);
}


BOOL SS_SetDataRange(lpSS, Col, Row, Col2, Row2, lpData, fValue,
                     fIgnoreLockedCells, fCheckLen)

LPSPREADSHEET  lpSS;
SS_COORD       Col;
SS_COORD       Row;
SS_COORD       Col2;
SS_COORD       Row2;
LPCTSTR        lpData;
BOOL           fValue;
BOOL           fIgnoreLockedCells;
BOOL           fCheckLen;
{
TBGLOBALHANDLE hData = 0;
BOOL           fSetVScrollBar = FALSE;
BOOL           fEmpty = FALSE;
BOOL           Ret = TRUE;
SS_COORD       x;
SS_COORD       y;

SS_LimitRangeCoords(lpSS, &Col, &Row, &Col2, &Row2);

if (lpData && fValue != SS_VALUE_FLOAT && fValue != SS_VALUE_INT &&
#if defined(_WIN64) || defined(IA64)
    fValue != SS_VALUE_LONG_PTR &&
#endif
    *lpData == '\0')
   lpData = NULL;

if (!lpData || !*lpData)
   fEmpty = TRUE;

if (Row != Row2 || Col != Col2)
   {
   BOOL RedrawOld = lpSS->lpBook->Redraw;
   lpSS->lpBook->Redraw = FALSE;
   //lpSS->lpBook->fNoInvalidate = TRUE;

   for (y = Row; Ret && y <= Row2; y++)
      for (x = Col; Ret && x <= Col2; x++)
         if (!fIgnoreLockedCells || !SS_GetLock(lpSS, x, y, TRUE))
				// RFW - 5/10/04 - 14210
            SSx_SetData(lpSS, x, y, lpData, fValue, &fSetVScrollBar,
                        fIgnoreLockedCells, fCheckLen);
				/*
            if (!SSx_SetData(lpSS, x, y, lpData, fValue, &fSetVScrollBar,
                             fIgnoreLockedCells, fCheckLen))
               Ret = FALSE;
				*/

	SS_BookSetRedraw(lpSS->lpBook, RedrawOld);
   }

else
   if (!fIgnoreLockedCells || !SS_GetLock(lpSS, Col, Row, TRUE))
      if (!SSx_SetData(lpSS, Col, Row, lpData, fValue, &fSetVScrollBar,
                       fIgnoreLockedCells, fCheckLen))
         Ret = FALSE;

if (fEmpty)
   {
   SS_AdjustDataColCnt(lpSS, Col, Col2);
   SS_AdjustDataRowCnt(lpSS, Row, Row2);
   }

/*
if (lpSS->lpBook->fNoInvalidate)
   {
   lpSS->lpBook->fNoInvalidate = FALSE;
   SS_InvalidateRange(lpSS, Col, Row, Col2, Row2);
	SS_BookSetRedraw(lpSS->lpBook, RedrawOld);
   }
*/

return (Ret);
}


BOOL DLLENTRY SSx_SetData(lpSS, x, y, lpData, fValue, lpfSetVScrollBar,
                          fIgnoreLockedCells, fCheckLen)

LPSPREADSHEET  lpSS;
SS_COORD       x;
SS_COORD       y;
LPCTSTR        lpData;
BOOL           fValue;
LPBOOL         lpfSetVScrollBar;
BOOL           fIgnoreLockedCells;
BOOL           fCheckLen;
{
TBGLOBALHANDLE hData = 0;
#ifdef WIN32
TCHAR          szBuffer[330];
#else
static TCHAR   szBuffer[50];
#endif
SS_CELLTYPE    CellType;
LPSS_CELL      lpCell;
LPSS_ROW       lpRow;
LPSS_COL       lpCol;
SS_DATA        Data;
BOOL           EditMode = FALSE;
BOOL           Ret = TRUE;
SS_COORD       i;
SS_COORD       j;
#ifdef SS_OLDCALC
SS_COORD       ColAt;
SS_COORD       RowAt;
BOOL           fHighlightOn;
SS_COORD       x1;
SS_COORD       x2;
SS_COORD       y1;
SS_COORD       y2;
#endif
SS_COORD       lColOut;
SS_COORD       lRowOut;

if (lpSS->lpBook->EditModeOn && lpSS->Row.CurAt == y && lpSS->Col.CurAt == x &&
    !lpSS->lpBook->fLeaveEditModeOn)
   EditMode = TRUE;

lpCell = SS_LockCellItem(lpSS, x, y);

SS_RetrieveCellType(lpSS, &CellType, lpCell, x, y);

if ((fValue == SS_VALUE_FLOAT && !SS_IsCellTypeFloatNum(&CellType)) ||
    (fValue == SS_VALUE_INT && CellType.Type != SS_TYPE_INTEGER && !SS_IsCellTypeFloatNum(&CellType))
#if defined(_WIN64) || defined(IA64)
    || (fValue == SS_VALUE_LONG_PTR && CellType.Type != SS_TYPE_INTEGER && !SS_IsCellTypeFloatNum(&CellType))
#endif
    )
   {
   if (CellType.Type == SS_TYPE_EDIT ||
       CellType.Type == SS_TYPE_PIC ||
       CellType.Type == SS_TYPE_STATICTEXT ||
       CellType.Type == SS_TYPE_BUTTON ||
       CellType.Type == SS_TYPE_COMBOBOX ||
       CellType.Type == SS_TYPE_CHECKBOX ||
       CellType.Type == SS_TYPE_INTEGER ||
#ifdef SS_V40
       CellType.Type == SS_TYPE_CURRENCY ||
       CellType.Type == SS_TYPE_NUMBER ||
       CellType.Type == SS_TYPE_PERCENT ||
#endif // SS_V40
#ifdef SS_V70
       CellType.Type == SS_TYPE_SCIENTIFIC ||
#endif // SS_V70
       CellType.Type == SS_TYPE_FLOAT)
      {
      if (!lpData)
			{
         szBuffer[0] = '\0';
	      fValue = SS_VALUE_TEXT;
			}
      else if (fValue == SS_VALUE_FLOAT)
         { // RFW - 9/13/99 - GIC10547
			LPSTR  lpszDouble;
         double dfVal = *(LPDOUBLE)lpData;
			double dfInt;
         int    iSign, iPos, iMaxDigits, iLen;

			modf(dfVal, &dfInt);

         _fmemset(szBuffer, '\0', sizeof(szBuffer));

			// RFW - 7/25/04 - 14903
			if (dfInt == 0.0 && dfVal < 0.0)
				{
				LongToString(-1, szBuffer);
				szBuffer[lstrlen(szBuffer) - 1] = '0';
				}
			else
				{
				// RFW - 7/18/08 - 22489
	         lpszDouble = _ecvt(dfInt, 15, &iPos, &iSign);

				// RFW - 8/18/08 - 22819
				if (iSign)
					lstrcpy(szBuffer, _T("-"));

				if (iPos > 0)
					{
		         iLen = lstrlen(szBuffer);

#ifdef _UNICODE // if unicode, convert string to LPWSTR
                // (there's no LPWSTR version of "_fcvt()").
		         MultiByteToWideChar(CP_ACP, 0, lpszDouble, iPos,
		                             &szBuffer[iLen], sizeof(szBuffer) - iLen);
#else
					lstrcpyn(&szBuffer[iLen], lpszDouble, iPos + 1);
#endif
					}
				}

         iLen = lstrlen(szBuffer);
         // 24648 -scl
         if( iLen == 0 )
            szBuffer[iLen++] = '0';
         szBuffer[iLen++] = lpSS->lpBook->WinFloatFormat.cDecimalSign;

// fix for bug 8997 -scl
//         lpszDouble = (LPTSTR)_fcvt(dfVal, 15, &iPos, &iSign);
			// RFW - 7/28/04 - 14903
         // lpszDouble = _ecvt(dfVal, 50, &iPos, &iSign);
			dfVal -= dfInt;
			/* RFW - 1/25/06 - 18064
         lpszDouble = _ecvt(dfVal, 15, &iPos, &iSign);
			*/
			/* RFW - 9/15/06 - 19458
         lpszDouble = _ecvt(dfVal, 14, &iPos, &iSign);
			*/
			iMaxDigits = 15 - max(0, iLen - 2);
         lpszDouble = _ecvt(dfVal, iMaxDigits, &iPos, &iSign);

         // Since 0's at the beginning are dropped,
         // iPos may be < 0.  If so, then place the
         // 0's on our string.
         for (; iPos < 0; iPos++)
            szBuffer[iLen++] = '0';

			/* RFW - 7/18/08 - 22489
			if (iPos < 50)
			*/
			if (iPos < iMaxDigits)
				{
#ifdef _UNICODE // if unicode, convert string to LPWSTR
                // (there's no LPWSTR version of "_fcvt()").
	         MultiByteToWideChar(CP_ACP, 0, &lpszDouble[iPos], -1,
	                             &szBuffer[iLen], sizeof(szBuffer) - iLen);
#else
	         lstrcpy(&szBuffer[iLen], &lpszDouble[iPos]);
#endif
				}

         iLen = lstrlen(szBuffer);
         while (iLen > 0 && szBuffer[--iLen] == '0')
            szBuffer[iLen] = '\0';

         if (iLen > 0 && szBuffer[iLen] == lpSS->lpBook->WinFloatFormat.cDecimalSign)
            szBuffer[iLen] = '\0';

	      fValue = SS_VALUE_VALUE;
         }
#if defined(_WIN64) || defined(IA64)
      else if( fValue == SS_VALUE_LONG_PTR )
      {
        LONG_PTR lpVal = *(LONG_PTR*)lpData;
        LongPtrToString(lpVal, szBuffer);
	      fValue = SS_VALUE_VALUE;
      }
#endif

/*
         fpDoubleToString(szBuffer, *(LPDOUBLE)lpData, 5,
                          lpSS->WinFloatFormat.cDecimalSign,
                          lpSS->WinFloatFormat.cSeparator,
                          FALSE, FALSE, TRUE, FALSE, 0, NULL);
*/
      else
			{
         LongToString(*(LPLONG)lpData, szBuffer);
	      fValue = SS_VALUE_VALUE;
			}

      lpData = szBuffer;
      }

   else
      return (FALSE);
   }

if (lpSS->lpBook->fAllowCellOverflow)
   SS_ResetCellOverflow(lpSS, x, y);

_fmemset(&Data, '\0', sizeof(SS_DATA));

if (lpData && !SS_FormatData(lpSS, &Data, x, y, &CellType, lpData, fValue, FALSE, fCheckLen))
   return (FALSE);

// BJO 15May96 SCS2798 - Begin fix part 1 of 2
if (EditMode)
   {
   BOOL wMessageBeingSentOld = lpSS->lpBook->wMessageBeingSent;
   lpSS->lpBook->wMessageBeingSent = FALSE;
   lpSS->lpBook->fDontSendEditModeMsg = TRUE;
   SS_CellEditModeOff(lpSS, 0);
   SS_PostSetEditMode(lpSS, TRUE, 1L); // RFW - 4/5/01
   lpSS->lpBook->fDontSendEditModeMsg = FALSE;
   lpSS->lpBook->wMessageBeingSent = wMessageBeingSentOld;
   }
// BJO 15May96 SCS2798 - End fix part 1 of 2

if (y >= lpSS->Row.HeaderCnt && x != SS_ALLCOLS &&
    x >= lpSS->Col.DataCnt && Data.bDataType)
   {
   SS_SetDataColCnt(lpSS, x + 1);
   *lpfSetVScrollBar = TRUE;
   }

if (x >= lpSS->Col.HeaderCnt && y != SS_ALLROWS &&
    y >= lpSS->Row.DataCnt && Data.bDataType)
   {
   SS_SetDataRowCnt(lpSS, y + 1);
   *lpfSetVScrollBar = TRUE;
   }

/***********
* Set Cell
***********/

if (x != SS_ALLCOLS && y != SS_ALLROWS)
   {
   TBGLOBALHANDLE hCell = 0;

   if (lpData && !lpCell && !(hCell = SS_AllocCell(lpSS, x, y)))
      return (FALSE);

   if (hCell)
      lpCell = (LPSS_CELL)tbGlobalLock(hCell);

   if (lpCell || (lpCell = SS_LockCellItem(lpSS, x, y)))
      {
      SSx_FreeData(&lpCell->Data);

      if (lpSS->lpBook->fAllowCellOverflow)
         SS_ResetCellOverflow(lpSS, x, y);

      _fmemcpy(&lpCell->Data, &Data, sizeof(SS_DATA));

      if (lpSS->lpBook->fAllowCellOverflow)
         SS_CalcCellOverflow(lpSS, lpCell, &CellType, x, y);

      SS_InvalidateCell(lpSS, x, y);

      if (lpSS->lpBook->fAllowCellOverflow && !lpCell->Data.bDataType)
         SS_OverflowAdjustNeighbor(lpSS, x, y);

      SS_UnlockCellItem(lpSS, x, y);
      }
   }

/**********
* Set Col
**********/

else if (x != SS_ALLCOLS && y == SS_ALLROWS)
   {
   if (lpData && !SS_AllocCol(lpSS, x))
      return (FALSE);

   for (i = lpSS->Row.HeaderCnt; i < lpSS->Row.AllocCnt; i++)
      if (lpCell = SS_LockCellItem(lpSS, x, i))
         {
         if (!fIgnoreLockedCells || !SS_GetLock(lpSS, x, i, TRUE))
            {
            SSx_FreeData(&lpCell->Data);
            SS_ResetCellOverflow(lpSS, x, i);
            }

         SS_UnlockCellItem(lpSS, x, i);
         }

   if (lpCol = SS_LockColItem(lpSS, x))
      {
      if (!fIgnoreLockedCells || !SS_GetLock(lpSS, x, SS_ALLROWS, TRUE))
         {
         SSx_FreeData(&lpCol->Data);
         SS_ResetCellOverflow(lpSS, x, y);

         _fmemcpy(&lpCol->Data, &Data, sizeof(SS_DATA));
         }

      SS_UnlockColItem(lpSS, x);
      }

   SS_InvalidateCol(lpSS, x);
   }

/**********
* Set Row
**********/

else if (x == SS_ALLCOLS && y != SS_ALLROWS)
   {
   if (lpData && !SS_AllocRow(lpSS, y))
      return (FALSE);

   if (lpRow = SS_LockRowItem(lpSS, y))
      {
      for (i = lpSS->Col.HeaderCnt; i < lpSS->Col.AllocCnt; i++)
         if (lpCell = SS_LockCellItem(lpSS, i, y))
            {
            if (!fIgnoreLockedCells || !SS_GetLock(lpSS, i, y, TRUE))
               {
               SSx_FreeData(&lpCell->Data);
               SS_ResetCellOverflow(lpSS, i, y);
               }

            SS_LockCellItem(lpSS, i, y);
            }

      if (!fIgnoreLockedCells || !SS_GetLock(lpSS, SS_ALLCOLS, y, TRUE))
         {
         SSx_FreeData(&lpRow->Data);
         SS_ResetCellOverflow(lpSS, x, y);

         _fmemcpy(&lpRow->Data, &Data, sizeof(SS_DATA));
         }

      SS_UnlockRowItem(lpSS, y);
      }

   SS_InvalidateRow(lpSS, y);
   }

/**********
* Set All
**********/

else
   {
   /**********************************
   * Clear All Cols, Rows, and Cells
   **********************************/

   for (i = lpSS->Row.HeaderCnt; i < lpSS->Row.AllocCnt; i++)
      {
      if (lpRow = SS_LockRowItem(lpSS, i))
         {
         if (!fIgnoreLockedCells || !SS_GetLock(lpSS, SS_ALLCOLS, i, TRUE))
            {
            SSx_FreeData(&lpRow->Data);
            SS_ResetCellOverflow(lpSS, x, i);

            for (j = lpSS->Col.HeaderCnt; j < lpSS->Col.AllocCnt; j++)
               if (lpCell = SSx_LockCellItem(lpSS, lpRow, j, i))
                  {
                  if (!fIgnoreLockedCells || !SS_GetLock(lpSS, j, i, TRUE))
                     {
                     SSx_FreeData(&lpCell->Data);
                     SS_ResetCellOverflow(lpSS, j, i);
                     }

                  SSx_UnlockCellItem(lpSS, lpRow, j, i);
                  }
            }

         SS_UnlockRowItem(lpSS, i);
         }
      }

   for (i = lpSS->Col.HeaderCnt; i < lpSS->Col.AllocCnt; i++)
      if (lpCol = SS_LockColItem(lpSS, i))
         {
         if (!fIgnoreLockedCells || !SS_GetLock(lpSS, i, SS_ALLROWS, TRUE))
            {
            SSx_FreeData(&lpCol->Data);
            SS_ResetCellOverflow(lpSS, i, y);
            }

         SS_UnlockColItem(lpSS, i);
         }

   if (!fIgnoreLockedCells || !SS_GetLock(lpSS, SS_ALLCOLS, SS_ALLROWS, TRUE))
      {
      SSx_FreeData(&lpSS->DefaultData);
      _fmemcpy(&lpSS->DefaultData, &Data, sizeof(SS_DATA));
      SS_InvalidateScrollArea(lpSS, SS_SCROLLAREA_ALL);
      }
   }

if (!Ret)
   return (FALSE);

#ifdef SS_OLDCALC
if (lpSS->CalcTableCnt && lpSS->lpBook->CalcAuto &&
    (SS_IsCellTypeFloatNum(CellType.Type) || CellType.Type == SS_TYPE_INTEGER))
   {
   if (x == -1)
      {
      x1 = lpSS->Col.HeaderCnt;
      x2 = lpSS->Col.DataCnt - 1;
      }
   else
      {
      x1 = x;
      x2 = x;
      }

   if (y == -1)
      {
      y1 = lpSS->Row.HeaderCnt;
      y2 = lpSS->Row.DataCnt - 1;
      }
   else
      {
      y1 = y;
      y2 = y;
      }

   ColAt = lpSS->Col.CurAt;
   RowAt = lpSS->Row.CurAt;

   fHighlightOn = lpSS->HighlightOn;
   SS_HighlightCell(lpSS, FALSE);
   lpSS->FreezeHighlight = TRUE;

   for (i = x1; i <= x2; i++)
      for (j = y1; j <= y2; j++)
         {
         lpSS->Col.CurAt = i;
         lpSS->Row.CurAt = j;
         SS_CalcDependencies(lpSS, i, j);
         }

   lpSS->FreezeHighlight = FALSE;

   lpSS->Col.CurAt = ColAt;
   lpSS->Row.CurAt = RowAt;

   SS_HighlightCell(lpSS, fHighlightOn);
   }
#elif !defined(SS_NOCALC)
lColOut = x;
lRowOut = y;

SS_AdjustCellCoordsOut(lpSS, &lColOut, &lRowOut);

CalcMarkDependForEval(&lpSS->CalcInfo, lColOut, lRowOut);
if( lpSS->lpBook->CalcAuto )
   CalcEvalNeededCells(&lpSS->lpBook->CalcInfo);
#endif

if (EditMode)
   {
// BJO 15May96 SCS2798 - Before fix
//   SS_SetEditModeOnData(lpSS);
// BJO 15May96 SCS2798 - Begin fix part 2 of 2
// RFW - 4/5/01
//   SS_PostSetEditMode(lpSS, TRUE, 1L);
// BJO 15May96 SCS2798 - End fix part 2 of 2
   }

#ifndef SS_UTP
else if (CellType.Type == SS_TYPE_CHECKBOX ||
         (CellType.Type == SS_TYPE_BUTTON &&
          CellType.Spec.Button.ButtonType == SUPERBTN_TYPE_2STATE))
   {
   lpSS->lpBook->CurCellSendingMsg.Col = x;
   lpSS->lpBook->CurCellSendingMsg.Row = y;

   if (!lpData || !lpData[0] || (!lpData[1] && lpData[0] == '0'))
      SS_SendMsgCommand(lpSS->lpBook, lpSS, SSN_BUTTONUP, FALSE);
   else
      SS_SendMsgCommand(lpSS->lpBook, lpSS, SSN_BUTTONDOWN, FALSE);
   }
#endif

return (Ret);
}


int DLLENTRY SSGetData(HWND hWnd, SS_COORD Col, SS_COORD Row, LPTSTR lpBuffer)
{
LPSPREADSHEET lpSS;
int           nLen = 0;

if (lpBuffer)
   lpBuffer[0] = '\0';

lpSS = SS_SheetLock(hWnd);
SS_AdjustCellCoords(lpSS, &Col, &Row);

nLen = SS_GetDataEx(lpSS, Col, Row, lpBuffer, -1);
SS_SheetUnlock(hWnd);

return nLen;
}


int SS_GetDataEx(LPSPREADSHEET lpSS, SS_COORD Col, SS_COORD Row, LPTSTR lpBuffer, int iMaxLen)
{
TBGLOBALHANDLE hData;
SS_CELLTYPE    CellType;
LPTSTR         lpData;
int            nLen = 0;

if (lpBuffer)
   lpBuffer[0] = '\0';

SS_RetrieveCellType(lpSS, &CellType, NULL, Col, Row);

if (lpSS->lpBook->EditModeOn && lpSS->Col.CurAt == Col && lpSS->Row.CurAt == Row)
   hData = SS_GetEditModeOnData(lpSS->lpBook->hWnd, lpSS, FALSE);
else
   hData = SS_GetData(lpSS, &CellType, Col, Row, FALSE);

if (hData)
   {
   lpData = (LPTSTR)tbGlobalLock(hData);
   if (lpBuffer && iMaxLen)
		{
		if (iMaxLen > 0)
			lstrcpyn(lpBuffer, lpData, iMaxLen);
		else
			lstrcpy(lpBuffer, lpData);
		}
   nLen = lstrlen(lpData);
   tbGlobalUnlock(hData);

   if (lpSS->lpBook->EditModeOn && lpSS->Col.CurAt == Col &&
       lpSS->Row.CurAt == Row)
      tbGlobalFree(hData);
   }

return nLen;
}


int DLLENTRY SSGetDataLen(HWND hWnd, SS_COORD Col, SS_COORD Row)
{
LPSPREADSHEET lpSS;
int           iLen = 0;

lpSS = SS_SheetLock(hWnd);

SS_AdjustCellCoords(lpSS, &Col, &Row);
iLen = SS_GetDataLen(lpSS, Col, Row);

SS_SheetUnlock(hWnd);
return iLen;
}


int SS_GetDataLen(LPSPREADSHEET lpSS, SS_COORD Col, SS_COORD Row)
{
SS_CELLTYPE    CellType;
TBGLOBALHANDLE hData;
LPTSTR         lpData;
int            iLen = 0;

SS_RetrieveCellType(lpSS, &CellType, NULL, Col, Row);

if (lpSS->lpBook->EditModeOn && lpSS->Col.CurAt == Col && lpSS->Row.CurAt == Row)
   hData = SS_GetEditModeOnData(lpSS->lpBook->hWnd, lpSS, FALSE);
else
   hData = SS_GetData(lpSS, &CellType, Col, Row, FALSE);

if (hData)
   {
   lpData = (LPTSTR)tbGlobalLock(hData);
   iLen = lstrlen(lpData);
   tbGlobalUnlock(hData);

   if (lpSS->lpBook->EditModeOn && lpSS->Col.CurAt == Col &&
       lpSS->Row.CurAt == Row)
      tbGlobalFree(hData);
   }

return iLen;
}


int DLLENTRY SSGetValue(HWND hWnd, SS_COORD Col, SS_COORD Row, LPTSTR lpBuffer)
{
LPSPREADSHEET  lpSS;
int            nLen = 0;

lpSS = SS_SheetLock(hWnd);

SS_AdjustCellCoords(lpSS, &Col, &Row);
nLen = SS_GetValueEx(lpSS, Col, Row, lpBuffer, -1);
SS_SheetUnlock(hWnd);
return nLen;
}

int DLLENTRY SS_GetValueEx(LPSPREADSHEET lpSS, SS_COORD Col, SS_COORD Row, LPTSTR lpBuffer, int iMaxLen)
{
TBGLOBALHANDLE hData;
LPTSTR         lpData;
int            nLen = 0;

if (lpBuffer)
   lpBuffer[0] = '\0';

if (hData = SS_GetValue(lpSS, Col, Row))
{
  lpData = (LPTSTR)tbGlobalLock(hData);
  if (lpBuffer && iMaxLen)
  {
    if (iMaxLen > 0)
      lstrcpyn(lpBuffer, lpData, iMaxLen);
    else
      lstrcpy(lpBuffer, lpData);
  }
  nLen = lstrlen(lpData);
  tbGlobalUnlock(hData);
  tbGlobalFree(hData);
}

return nLen;
}


int DLLENTRY SSGetValueLen(HWND hWnd, SS_COORD Col, SS_COORD Row)
{
LPSPREADSHEET  lpSS;
int            nLen = 0;

lpSS = SS_SheetLock(hWnd);

SS_AdjustCellCoords(lpSS, &Col, &Row);
nLen = SS_GetValueLen(lpSS, Col, Row);

SS_SheetUnlock(hWnd);
return nLen;
}


int SS_GetValueLen(LPSPREADSHEET lpSS, SS_COORD Col, SS_COORD Row)
{
TBGLOBALHANDLE hData;
LPTSTR         lpData;
int            nLen = 0;

if (hData = SS_GetValue(lpSS, Col, Row))
   {
   lpData = (LPTSTR)tbGlobalLock(hData);
   nLen = lstrlen(lpData);
   tbGlobalUnlock(hData);
   tbGlobalFree(hData);
   }

return nLen;
}


TBGLOBALHANDLE SS_GetValue(lpSS, Col, Row)

LPSPREADSHEET  lpSS;
SS_COORD       Col;
SS_COORD       Row;
{
TBGLOBALHANDLE hData;
TBGLOBALHANDLE hValue = 0;
SS_CELLTYPE    CellType;
LPTSTR         lpData;
SS_COORD       ColEdit, RowEdit;
BOOL           Ret = FALSE;

SS_RetrieveCellType(lpSS, &CellType, NULL, Col, Row);

// RFW - 4/19/05 - 16086
if (lpSS->lpBook->EditModeOn)
	SS_GetCellBeingEdited(lpSS, &ColEdit, &RowEdit);

if (lpSS->lpBook->EditModeOn && ColEdit == Col && RowEdit == Row)
   hData = SS_GetEditModeOnData(lpSS->lpBook->hWnd, lpSS, TRUE);
else
   hData = SS_GetData(lpSS, &CellType, Col, Row, TRUE);

if (hData || CellType.Type == SS_TYPE_BUTTON ||
             CellType.Type == SS_TYPE_CHECKBOX)
   {
   if (!hData)
      hValue = SS_UnFormatData(lpSS, Col, Row, &CellType, _T(""));
   else
      {
      lpData = (LPTSTR)tbGlobalLock(hData);
      hValue = SS_UnFormatData(lpSS, Col, Row, &CellType, lpData);
      tbGlobalUnlock(hData);
      }

   if (lpSS->lpBook->EditModeOn && hData && lpSS->Col.CurAt == Col && lpSS->Row.CurAt == Row)
      tbGlobalFree(hData);

   Ret = TRUE;
   }

return (hValue);
}


BOOL DLLENTRY SSSetFloat(hWnd, Col, Row, dfValue)

HWND     hWnd;
SS_COORD Col;
SS_COORD Row;
double   dfValue;
{
return (SSSetFloatRange(hWnd, Col, Row, Col, Row, dfValue));
}


BOOL DLLENTRY SSSetFloatRange(hWnd, Col, Row, Col2, Row2, dfValue)

HWND          hWnd;
SS_COORD      Col;
SS_COORD      Row;
SS_COORD      Col2;
SS_COORD      Row2;
double        dfValue;
{
LPSPREADSHEET lpSS;
BOOL          fRet;

lpSS = SS_SheetLock(hWnd);
SS_AdjustCellRangeCoords(lpSS, &Col, &Row, &Col2, &Row2);
fRet = SS_SetDataRange(lpSS, Col, Row, Col2, Row2, (LPTSTR)&dfValue,
                       SS_VALUE_FLOAT, FALSE, TRUE);
SS_SheetUnlock(hWnd);
return (fRet);
}


BOOL SS_SetFloatRange(LPSPREADSHEET lpSS, SS_COORD Col, SS_COORD Row,
                      SS_COORD Col2, SS_COORD Row2, double dfValue)
{
return (SS_SetDataRange(lpSS, Col, Row, Col2, Row2, (LPTSTR)&dfValue,
                        SS_VALUE_FLOAT, FALSE, TRUE));
}


BOOL DLLENTRY SSGetFloat(HWND hWnd, SS_COORD Col, SS_COORD Row, LPDOUBLE lpdfValue)
{
LPSPREADSHEET lpSS = SS_SheetLock(hWnd);
BOOL          fRet;

SS_AdjustCellCoords(lpSS, &Col, &Row);
fRet = SS_GetFloat(lpSS, Col, Row, lpdfValue);
SS_SheetUnlock(hWnd);
return (fRet);
}


BOOL SS_GetFloat(LPSPREADSHEET lpSS, SS_COORD Col, SS_COORD Row, LPDOUBLE lpdfValue)
{
TBGLOBALHANDLE hData;
LPSS_COL       lpCol = 0;
LPSS_ROW       lpRow = 0;
SS_DATA        Data;
BOOL           fDelete = FALSE;
BOOL           fRet = FALSE;

if (lpdfValue)
   *lpdfValue = 0.0;

if (lpSS->lpBook->EditModeOn && lpSS->Col.CurAt == Col &&
    lpSS->Row.CurAt == Row)
   {
	/* RFW - 12/12/02 - 11358
   if (hData = SS_GetEditModeOnData(lpSS->lpBook->hWnd, lpSS, TRUE))
	*/
   if (hData = SS_GetEditModeOnData(lpSS->lpBook->hWnd, lpSS, FALSE))
      {
      fDelete = TRUE;
      Data.bDataType = SS_TYPE_EDIT;
      Data.Data.hszData = hData;
      }
   else
      Data.bDataType = 0;
   }

else
   {
   lpCol = SS_LockColItem(lpSS, Col);
   lpRow = SS_LockRowItem(lpSS, Row);

   SS_GetDataStruct(lpSS, lpCol, lpRow, NULL, &Data, Col, Row);
   }

fRet = TRUE;

if (!Data.bDataType)
   fRet = FALSE;

else if (!lpdfValue)
   ; // avoid accessing NULL pointer

else if (Data.bDataType == SS_TYPE_INTEGER)
   *lpdfValue = (double)Data.Data.lValue;

else if (Data.bDataType == SS_TYPE_FLOAT)
   *lpdfValue = Data.Data.dfValue;

else
   {
   TBGLOBALHANDLE hValue;
   SS_CELLTYPE    CellType;
   LPTSTR         lpData;
   LPTSTR         lpValue;

   SS_RetrieveCellType(lpSS, &CellType, NULL, Col, Row);

   lpData = (LPTSTR)tbGlobalLock(Data.Data.hszData);

   hValue = SS_UnFormatData(lpSS, Col, Row, &CellType, lpData);
// fix for bug 9332 -scl
   if( hValue )
   {
     lpValue = (LPTSTR)tbGlobalLock(hValue);
     SS_StringToNum(lpSS, &CellType, lpValue, lpdfValue);
     tbGlobalUnlock(hValue);
     tbGlobalFree(hValue);

     // RFW - 6/24/03 - 12072
#ifdef SS_V40
     if (CellType.Type == SS_TYPE_PERCENT)
       *lpdfValue /= 100;
#endif
   }
   tbGlobalUnlock(Data.Data.hszData);

   if (fDelete)
      tbGlobalFree(Data.Data.hszData);
   }

if (lpCol)
   SS_UnlockColItem(lpSS, Col);

if (lpRow)
   SS_UnlockRowItem(lpSS, Row);

return (fRet);
}


BOOL DLLENTRY SSSetInteger(hWnd, Col, Row, lValue)

HWND     hWnd;
SS_COORD Col;
SS_COORD Row;
long     lValue;
{
return (SSSetIntegerRange(hWnd, Col, Row, Col, Row, lValue));
}


BOOL DLLENTRY SSSetIntegerRange(HWND hWnd, SS_COORD Col, SS_COORD Row,
                                SS_COORD Col2, SS_COORD Row2, long lValue)
{
LPSPREADSHEET lpSS;
BOOL          fRet;

lpSS = SS_SheetLock(hWnd);
SS_AdjustCellRangeCoords(lpSS, &Col, &Row, &Col2, &Row2);
fRet = SS_SetDataRange(lpSS, Col, Row, Col2, Row2, (LPTSTR)&lValue,
                       SS_VALUE_INT, FALSE, TRUE);
SS_SheetUnlock(hWnd);
return (fRet);
}


BOOL SS_SetIntegerRange(LPSPREADSHEET lpSS, SS_COORD Col, SS_COORD Row,
                        SS_COORD Col2, SS_COORD Row2, long lValue)
{
return (SS_SetDataRange(lpSS, Col, Row, Col2, Row2, (LPTSTR)&lValue,
                        SS_VALUE_INT, FALSE, TRUE));
}


BOOL DLLENTRY SSGetInteger(HWND hWnd, SS_COORD Col, SS_COORD Row, LPLONG lplValue)
{
LPSPREADSHEET lpSS = SS_SheetLock(hWnd);
BOOL          fRet;

SS_AdjustCellCoords(lpSS, &Col, &Row);
fRet = SS_GetInteger(lpSS, Col, Row, lplValue);
SS_SheetUnlock(hWnd);
return (fRet);
}


BOOL SS_GetInteger(LPSPREADSHEET lpSS, SS_COORD Col, SS_COORD Row, LPLONG lplValue)
{
TBGLOBALHANDLE hData;
LPSS_COL       lpCol = 0;
LPSS_ROW       lpRow = 0;
SS_DATA        Data;
BOOL           fDelete = FALSE;
BOOL           fRet = FALSE;

if (lplValue)
   *lplValue = 0;

if (lpSS->lpBook->EditModeOn && lpSS->Col.CurAt == Col &&
    lpSS->Row.CurAt == Row)
   {
   if (hData = SS_GetEditModeOnData(lpSS->lpBook->hWnd, lpSS, TRUE))
      {
      fDelete = TRUE;
      Data.bDataType = SS_TYPE_EDIT;
      Data.Data.hszData = hData;
      }
   else
      Data.bDataType = 0;
   }

else
   {
   lpCol = SS_LockColItem(lpSS, Col);
   lpRow = SS_LockRowItem(lpSS, Row);

   SS_GetDataStruct(lpSS, lpCol, lpRow, NULL, &Data, Col, Row);
   }

fRet = TRUE;

if (!Data.bDataType)
   fRet = FALSE;

else if (!lplValue)
   ; // avoid accessing NULL pointer

else if (Data.bDataType == SS_TYPE_INTEGER)
   *lplValue = Data.Data.lValue;

else if (Data.bDataType == SS_TYPE_FLOAT)
   *lplValue = (long)Data.Data.dfValue;

else
   {
   TBGLOBALHANDLE hValue;
   SS_CELLTYPE    CellType;
   LPTSTR         lpData;
   LPTSTR         lpValue;

   SS_RetrieveCellType(lpSS, &CellType, NULL, Col, Row);

   lpData = (LPTSTR)tbGlobalLock(Data.Data.hszData);

   hValue = SS_UnFormatData(lpSS, Col, Row, &CellType, lpData);
// fix for bug 9332 -scl
   if( hValue )
   {
     lpValue = (LPTSTR)tbGlobalLock(hValue);
     *lplValue = StringToLong(lpValue);
     tbGlobalUnlock(hValue);
     tbGlobalFree(hValue);
   }
   tbGlobalUnlock(Data.Data.hszData);

   if (fDelete)
      tbGlobalFree(Data.Data.hszData);
   }

if (lpCol)
   SS_UnlockColItem(lpSS, Col);

if (lpRow)
   SS_UnlockRowItem(lpSS, Row);

return (fRet);
}

#if defined(_WIN64) || defined(_IA64)
LONG_PTR DLLENTRY SSSetUserData(HWND hWnd, LONG_PTR lUserData)
#else
LONG DLLENTRY SSSetUserData(HWND hWnd, LONG lUserData)
#endif
{
LPSPREADSHEET lpSS;
#if defined(_WIN64) || defined(_IA64)
LONG_PTR      lRet;
#else
LONG          lRet;
#endif

lpSS = SS_SheetLock(hWnd);
lRet = SS_SetUserData(lpSS, lUserData);
SS_SheetUnlock(hWnd);
return lRet;
}

#if defined(_WIN64) || defined(_IA64)
LONG_PTR SS_SetUserData(LPSPREADSHEET lpSS, LONG_PTR lUserData)
#else
LONG SS_SetUserData(LPSPREADSHEET lpSS, LONG lUserData)
#endif
{
#if defined(_WIN64) || defined(_IA64)
LONG_PTR lRet;
#else
LONG lRet;
#endif

lRet = lpSS->lUserData;
lpSS->lUserData = lUserData;
return lRet;
}

#if defined(_WIN64) || defined(_IA64)
LONG_PTR DLLENTRY SSGetUserData(HWND hWnd)
#else
LONG DLLENTRY SSGetUserData(HWND hWnd)
#endif
{
LPSPREADSHEET lpSS;
#if defined(_WIN64) || defined(_IA64)
LONG_PTR      lRet;
#else
LONG          lRet;
#endif

lpSS = SS_SheetLock(hWnd);
lRet = SS_GetUserData(lpSS);
SS_SheetUnlock(hWnd);
return lRet;
}

#if defined(_WIN64) || defined(_IA64)
LONG_PTR SS_GetUserData(LPSPREADSHEET lpSS)
#else
LONG SS_GetUserData(LPSPREADSHEET lpSS)
#endif
{
return lpSS->lUserData;
}


TBGLOBALHANDLE SS_GetEditModeOnData(HWND hWnd, LPSPREADSHEET lpSS, BOOL fValue)
{
TBGLOBALHANDLE hGlobalData = 0;
HWND           hWndCtrl;
LPSS_CELL      lpCell;
LPSS_CELLTYPE  CellType;
SS_CELLTYPE    CellTypeTemp;
LPTSTR         Data;

#ifdef  BUGS
// Bug-002
TCHAR          Buffer[330];
#else
TCHAR          Buffer[40];
#endif

double         dfValue;
short          Len;

if (lpSS->lpBook->EditModeOn)
   {
	SS_COORD ColEdit, RowEdit;

	SS_GetCellBeingEdited(lpSS, &ColEdit, &RowEdit); // RFW - 4/18/05 - 16009

   lpCell = SS_LockCellItem(lpSS, ColEdit, RowEdit);
   CellType = SS_RetrieveCellType(lpSS, &CellTypeTemp, lpCell, ColEdit, RowEdit);

   if (CellType->ControlID)
      hWndCtrl = SS_GetControlhWnd(lpSS, CellType->ControlID);

   switch (CellType->Type)
      {
      case SS_TYPE_EDIT:
#if SS_V80
      case SS_TYPE_CUSTOM:
#endif
			/* RFW - 5/22/03
         if (CellType->Type == SS_TYPE_EDIT)
            Len = CellType->Spec.Edit.Len;
         else
            Len = GetWindowTextLength(hWndCtrl);
			*/
         Len = GetWindowTextLength(hWndCtrl);

         if (hGlobalData = tbGlobalAlloc(GMEM_MOVEABLE | GMEM_ZEROINIT,
                                         (Len + 1) * sizeof(TCHAR)))
            {
            Data = (LPTSTR)tbGlobalLock(hGlobalData);
            _fmemset(Data, '\0', (Len + 1) * sizeof(TCHAR));
            GetWindowText(hWndCtrl, Data, Len + 1);

            if (CellType->Spec.Edit.ChrSet == SS_CHRSET_ALPHA ||
                CellType->Spec.Edit.ChrSet == SS_CHRSET_ALPHANUM)
               while (*Data && *Data == ' ')
                  Data++;

            tbGlobalUnlock(hGlobalData);
            }

         break;

      case SS_TYPE_DATE:
      case SS_TYPE_TIME:
      case SS_TYPE_INTEGER:
      case SS_TYPE_FLOAT:
      case SS_TYPE_PIC:
#ifdef SS_V40
      case SS_TYPE_CURRENCY:
      case SS_TYPE_NUMBER:
      case SS_TYPE_PERCENT:
#endif // SS_V40
#ifdef SS_V70
      case SS_TYPE_SCIENTIFIC:
#endif // SS_V70
         if (lpSS->fFormulaMode)
            {
            CellType = &lpSS->lpBook->FormulaCellType;
            hWndCtrl = SS_GetControlhWnd(lpSS, CellType->ControlID);

            if (hGlobalData = tbGlobalAlloc(GMEM_MOVEABLE | GMEM_ZEROINIT,
                                            (CellType->Spec.Edit.Len + 1)
                                            * sizeof(TCHAR)))
               {
               Data = (LPTSTR)tbGlobalLock(hGlobalData);
               GetWindowText(hWndCtrl, Data, CellType->Spec.Edit.Len + 1);
               tbGlobalUnlock(hGlobalData);
               }
            }

         else if (CellType->Type == SS_TYPE_FLOAT)
            {
#ifdef SS_USEAWARE
            if (!FloatGetValue(hWndCtrl, &dfValue))
               *Buffer = '\0';
            else
               fpDoubleToString(Buffer, dfValue, CellType->Spec.Float.Right,
                                '.', 0, FALSE, (CellType->Style & FS_MONEY) ?
                                TRUE : FALSE, TRUE, FALSE, 0, NULL);
#else
            FloatGetValue(hWndCtrl, &dfValue);

            SS_FloatFormat(lpSS, CellType, dfValue, Buffer, fValue);
#endif

            if (hGlobalData = tbGlobalAlloc(GMEM_MOVEABLE | GMEM_ZEROINIT,
                                            (lstrlen(Buffer) + 1)
                                            * sizeof(TCHAR)))
               {
               Data = (LPTSTR)tbGlobalLock(hGlobalData);
               lstrcpy(Data, Buffer);
               tbGlobalUnlock(hGlobalData);
               }
            }

#ifdef SS_V40
         else if (SS_IsCellTypeNum(CellType))
            {
            NumGetValue(hWndCtrl, &dfValue);
				if (CellType->Type == SS_TYPE_PERCENT)
					dfValue /= 100.0;

            SS_FloatFormat(lpSS, CellType, dfValue, Buffer, fValue);

            if (hGlobalData = tbGlobalAlloc(GMEM_MOVEABLE | GMEM_ZEROINIT,
                                            (lstrlen(Buffer) + 1) * sizeof(TCHAR)))
               {
               Data = (LPTSTR)tbGlobalLock(hGlobalData);
               lstrcpy(Data, Buffer);
               tbGlobalUnlock(hGlobalData);
               }
            }
#endif // SS_V40

#ifdef SS_V70
         else if (CellType->Type == SS_TYPE_SCIENTIFIC)
				{
				Len = GetWindowTextLength(hWndCtrl);

				if (hGlobalData = tbGlobalAlloc(GMEM_MOVEABLE | GMEM_ZEROINIT,
														  (Len + 1) * sizeof(TCHAR)))
					{
					Data = (LPTSTR)tbGlobalLock(hGlobalData);
					_fmemset(Data, '\0', (Len + 1) * sizeof(TCHAR));
					GetWindowText(hWndCtrl, Data, Len + 1);
					tbGlobalUnlock(hGlobalData);
					}
				}
#endif // SS_V70

         else
            {
            if (CellType->Type == SS_TYPE_PIC)
               Len = GetWindowTextLength(hWndCtrl);
            else
               Len = 40;

            if (hGlobalData = tbGlobalAlloc(GMEM_MOVEABLE | GMEM_ZEROINIT,
                                            (Len + 1) * sizeof(TCHAR)))
               {
               Data = (LPTSTR)tbGlobalLock(hGlobalData);
               _fmemset(Data, '\0', (Len + 1) * sizeof(TCHAR));
               GetWindowText(hWndCtrl, Data, Len + 1);
               tbGlobalUnlock(hGlobalData);
               }
            }

         break;

#ifndef SS_NOCT_COMBO
      case SS_TYPE_COMBOBOX:
         Len = (short)SendMessage(hWndCtrl, WM_GETTEXTLENGTH, 0, 0L);

         if (hGlobalData = tbGlobalAlloc(GHND,
                                         (Len + 1 + 6 + 1) * sizeof(TCHAR)))
            {
            LPTSTR lpItems;
            short  i;
            short  Sel = (short)SendMessage(hWndCtrl, CB_GETCURSEL, 0, 0);

            Data = (LPTSTR)tbGlobalLock(hGlobalData);
            SendMessage(hWndCtrl, WM_GETTEXT, Len + 1, (LPARAM)Data);

            lpItems = (LPTSTR)tbGlobalLock(CellType->Spec.ComboBox.hItems);

            if (Sel == -1)
               {
               for (i = 0; i < CellType->Spec.ComboBox.dItemCnt; i++)
                  {
                  //- JPNFIX0013 - (Masanori Iwasa)
                  if (_ftcscmp(Data, lpItems) == 0)
                     {
                     Sel = i;
                     break;
                     }

                  lpItems += lstrlen(lpItems) + 1;
                  }
               }

            wsprintf(&Data[Len + 1], _T("%d"), Sel);

            tbGlobalUnlock(CellType->Spec.ComboBox.hItems);
            tbGlobalUnlock(hGlobalData);
            }

         break;
#endif

      case SS_TYPE_BUTTON:
         {
         LPTSTR lpData;
         BOOL   fButtonState;

         if (CellType &&
             CellType->Spec.Button.ButtonType == SUPERBTN_TYPE_2STATE)
            {
            fButtonState = (BOOL)SendMessage(hWndCtrl, SBM_GETBUTTONSTATE, 0,
                                             0L);

            if (hGlobalData = tbGlobalAlloc(GMEM_MOVEABLE | GMEM_ZEROINIT,
                                            (1 + 1) * sizeof(TCHAR)))
               {
               lpData = (LPTSTR)tbGlobalLock(hGlobalData);
               lpData[0] = '0' + (fButtonState ? 1 : 0);
               tbGlobalUnlock(hGlobalData);
               }
            }
         }

         break;

      case SS_TYPE_CHECKBOX:
#ifdef SS_UTP
         {
         TBGLOBALHANDLE hDataTemp;
         LPTSTR         lpszDataTemp;
         LPTSTR         lpData;
         short          nLen;

         hDataTemp = SS_GetData(lpSS, CellType, ColEdit, RowEdit, FALSE);

         if (hDataTemp)
            {
            lpszDataTemp = (LPTSTR)tbGlobalLock(hDataTemp);
            nLen = lstrlen(lpszDataTemp);
            }
         else
            nLen = 1;

         if (hGlobalData = tbGlobalAlloc(GMEM_MOVEABLE | GMEM_ZEROINIT,
                                         (nLen + 1) * sizeof(TCHAR)))
            {
            lpData = (LPTSTR)tbGlobalLock(hGlobalData);

            if (hDataTemp)
               lstrcpy(lpData, lpszDataTemp);
            else
               lpData[0] = '0';

            tbGlobalUnlock(hGlobalData);
            }

         if (hDataTemp)
            tbGlobalUnlock(hDataTemp);
         }

#else
         {
         LPTSTR lpData;
         WORD   wButtonState;

         wButtonState = (WORD)SendMessage(hWndCtrl, BM_GETCHECK, 0, 0L);

         if (hGlobalData = tbGlobalAlloc(GMEM_MOVEABLE | GMEM_ZEROINIT,
                                         (1 + 1) * sizeof(TCHAR)))
            {
            lpData = (LPTSTR)tbGlobalLock(hGlobalData);
            lpData[0] = '0' + (TCHAR)wButtonState;
            tbGlobalUnlock(hGlobalData);
            }
         }
#endif

         break;

      }

   if (lpCell)
      SS_UnlockCellItem(lpSS, ColEdit, RowEdit);
   }

return (hGlobalData);
}

#if SS_V80
//BOOL SS_GetHandle(LPSPREADSHEET lpSS, SS_COORD Col, SS_COORD Row, LPHANDLE pValue, LPBOOL pfOwn, SS_FREEHANDLE_FUNC *ppFreeFunc)
//{
//  SS_DATA DataItem;
//
//  SS_GetDataStruct(lpSS, NULL, NULL, NULL, &DataItem, Col, Row);
//  if( DataItem.bDataType == SS_DATATYPE_HANDLE )
//  {
//    if( pValue )
//      *pValue = DataItem.Data.Handle.hValue;
//    if( pfOwn )
//      *pfOwn = DataItem.Data.Handle.fOwn;
//    if( ppFreeFunc )
//      *ppFreeFunc = DataItem.Data.Handle.pFreeFunc;
//    return TRUE;
//  }
//  return FALSE;
//}
//
//BOOL SS_SetHandle(LPSPREADSHEET lpSS, SS_COORD Col, SS_COORD Row, HANDLE hValue, BOOL fOwn, SS_FREEHANDLE_FUNC pFreeFunc)
//{
//  SS_DATA DataItem;
//
//  SS_GetDataStruct(lpSS, NULL, NULL, NULL, &DataItem, Col, Row);
//  SSx_FreeData(&DataItem);
//  DataItem.bDataType = SS_DATATYPE_HANDLE;
//  DataItem.Data.Handle.hValue = hValue;
//  DataItem.Data.Handle.fOwn = fOwn;
//  DataItem.Data.Handle.pFreeFunc = pFreeFunc;
//  SSx_SetCellDataItem(lpSS, Col, Row, &DataItem, FALSE);
//  return TRUE;
//}

BOOL SS_GetBuffer(LPSPREADSHEET lpSS, SS_COORD Col, SS_COORD Row, LPVOID lpBuffer, long *plSize)
{
  SS_DATA DataItem;
  LPVOID lpData;

  SS_GetDataStruct(lpSS, NULL, NULL, NULL, &DataItem, Col, Row);
  if( DataItem.bDataType == SS_DATATYPE_BUFFER )
  {
    if( plSize )
       *plSize = DataItem.Data.Buffer.lSize;
    if( lpBuffer && DataItem.Data.Buffer.lSize > 0 && (lpData = (LPVOID)tbGlobalLock(DataItem.Data.Buffer.hBuffer)) )
    {
       MemHugeCpy(lpBuffer, lpData, DataItem.Data.Buffer.lSize);
       tbGlobalUnlock(DataItem.Data.Buffer.hBuffer);
    }
    return TRUE;
  }
  return FALSE;
}

BOOL DLLENTRY SSGetBuffer(HWND hWnd, SS_COORD Col, SS_COORD Row, LPVOID lpBuffer, long *plSize)
{
LPSPREADSHEET lpSS;
BOOL ret = FALSE;

lpSS = SS_SheetLock(hWnd);
SS_AdjustCellCoords(lpSS, &Col, &Row);

ret = SS_GetBuffer(lpSS, Col, Row, lpBuffer, plSize);
SS_SheetUnlock(hWnd);

return ret;
}

BOOL SS_SetBuffer(LPSPREADSHEET lpSS, SS_COORD Col, SS_COORD Row, LPVOID lpBuffer, long lSize)
{
  SS_DATA DataItem;
  LPVOID lpData;

  if( lpBuffer && lSize > 0 )
  {
     SS_GetDataStruct(lpSS, NULL, NULL, NULL, &DataItem, Col, Row);
     SSx_FreeData(&DataItem);
     DataItem.bDataType = SS_DATATYPE_BUFFER;
     DataItem.Data.Buffer.hBuffer = tbGlobalAlloc(GMEM_MOVEABLE | GMEM_ZEROINIT, lSize);
     if( lpData = (LPVOID)tbGlobalLock(DataItem.Data.Buffer.hBuffer) )
     {
        MemHugeCpy(lpData, lpBuffer, lSize);
        DataItem.Data.Buffer.lSize = lSize;
        tbGlobalUnlock(DataItem.Data.Buffer.hBuffer);
     }
     SSx_SetCellDataItem(lpSS, Col, Row, &DataItem, FALSE);
     return TRUE;
  }
  return FALSE;
}

BOOL DLLENTRY SSSetBuffer(HWND hWnd, SS_COORD Col, SS_COORD Row, LPVOID lpBuffer, long lSize)
{
LPSPREADSHEET lpSS;
BOOL ret = FALSE;

lpSS = SS_SheetLock(hWnd);
SS_AdjustCellCoords(lpSS, &Col, &Row);

ret = SS_SetBuffer(lpSS, Col, Row, lpBuffer, lSize);
SS_SheetUnlock(hWnd);

return ret;
}

// copies from DataItem to Value
BOOL SS_DataToCTValue(LPSS_CT_VALUE Value, LPSS_DATA DataItem)
{
  BOOL bRet = FALSE;
  if( Value && DataItem )
  {
     SS_FreeCTValue(Value);
     if( !DataItem->bDataType )
     {
       Value->type = SS_CT_VALUE_TYPE_TSTR;
       Value->u.hszValue = 0;
       bRet = TRUE;
     }
     else if( DataItem->bDataType == SS_DATATYPE_EDIT )
     {
       LPTSTR lpszData;
       LPTSTR lpszValue;

       if( lpszData = (LPTSTR)tbGlobalLock(DataItem->Data.hszData) )
       {
         int len = lstrlen(lpszData);
         Value->u.hszValue = GlobalAlloc(GHND, (len + 1) * sizeof(TCHAR));
         if( lpszValue = GlobalLock(Value->u.hszValue) )
         {
           Value->type = SS_CT_VALUE_TYPE_TSTR;
           lstrcpy(lpszValue, lpszData);
           GlobalUnlock(Value->u.hszValue);
           bRet = TRUE;
         }
         tbGlobalUnlock(DataItem->Data.hszData);
       }
     }
     else if( DataItem->bDataType == SS_DATATYPE_BUFFER && DataItem->Data.Buffer.lSize > 0 )
     {
       LPVOID lpData;
       LPVOID lpValue;

       if( lpData = (LPVOID)tbGlobalLock(DataItem->Data.Buffer.hBuffer) )
       {
         Value->u.Buffer.hBuffer = GlobalAlloc(GHND, DataItem->Data.Buffer.lSize);
         if( lpValue = GlobalLock(Value->u.Buffer.hBuffer) )
         {
           MemHugeCpy(lpValue, lpData, DataItem->Data.Buffer.lSize);
           GlobalUnlock(Value->u.hszValue);
           Value->type = SS_CT_VALUE_TYPE_BUFFER;
           Value->u.Buffer.lSize = DataItem->Data.Buffer.lSize;
           bRet = TRUE;
         }
         tbGlobalUnlock(DataItem->Data.hszData);
       }
     }
     else if( DataItem->bDataType == SS_DATATYPE_FLOAT )
     {
       Value->type = SS_CT_VALUE_TYPE_DOUBLE;
       Value->u.dblValue = DataItem->Data.dfValue;
       bRet = TRUE;
     }
     else if( DataItem->bDataType == SS_DATATYPE_INTEGER )
     {
       Value->type = SS_CT_VALUE_TYPE_LONG;
       Value->u.lValue = DataItem->Data.lValue;
       bRet = TRUE;
     }
     //else if( DataItem->bDataType == SS_DATATYPE_HANDLE )
     //{
     //  Value->type = SS_CT_VALUE_TYPE_HANDLE;
     //  Value->u.Handle.hValue = DataItem->Data.Handle.hValue;
     //  // DataItem keeps ownership and freeing
     //  Value->u.Handle.fOwn = FALSE;
     //  Value->u.Handle.pfnFreeHandle = NULL;
     //  bRet = TRUE;
     //}
  }
  return bRet;
}
// copies from Value to DataItem
BOOL SS_CTValueToData(LPSS_CT_VALUE Value, LPSS_DATA DataItem)
{
  BOOL bRet = FALSE;
  if( Value && DataItem )
  {
    SSx_FreeData(DataItem);
    if( Value->type == SS_CT_VALUE_TYPE_TSTR )
    {
      LPTSTR lpszData;
      LPTSTR lpszValue;

      if( Value->u.hszValue == NULL )
      {
        DataItem->bDataType = 0;
        DataItem->Data.hszData = 0;
        bRet = TRUE;
      }
      else if( lpszValue = GlobalLock(Value->u.hszValue) )
      {
        int len = lstrlen(lpszValue);
        if( len > 0 )
        {
          DataItem->bDataType = SS_DATATYPE_EDIT;
          DataItem->Data.hszData = tbGlobalAlloc(GMEM_MOVEABLE | GMEM_ZEROINIT, (len + 1) * sizeof(TCHAR));
          if( lpszData = (LPTSTR)tbGlobalLock(DataItem->Data.hszData) )
          {
            DataItem->bDataType = SS_DATATYPE_EDIT;
            lstrcpy(lpszData, lpszValue);
            tbGlobalUnlock(DataItem->Data.hszData);
            bRet = TRUE;
          }
        }
        else
        {
          DataItem->bDataType = 0;
          DataItem->Data.hszData = 0;
        }
        GlobalUnlock(Value->u.hszValue);
      }
    }
    else if( Value->type == SS_CT_VALUE_TYPE_BUFFER && Value->u.Buffer.lSize > 0 )
    {
       LPVOID lpValue = GlobalLock(Value->u.Buffer.hBuffer);
       LPVOID lpData;

       DataItem->Data.Buffer.hBuffer = tbGlobalAlloc(GMEM_MOVEABLE | GMEM_ZEROINIT, Value->u.Buffer.lSize);
       if( lpData = (LPVOID)tbGlobalLock(DataItem->Data.Buffer.hBuffer) )
       {
          MemHugeCpy(lpData, lpValue, Value->u.Buffer.lSize);
          tbGlobalUnlock(DataItem->Data.Buffer.hBuffer);
          DataItem->bDataType = SS_DATATYPE_BUFFER;
          DataItem->Data.Buffer.lSize = Value->u.Buffer.lSize;
          bRet = TRUE;
       }
    }
    else if( Value->type == SS_CT_VALUE_TYPE_DOUBLE )
    {
      DataItem->bDataType = SS_DATATYPE_FLOAT;
      DataItem->Data.dfValue = Value->u.dblValue;
      bRet = TRUE;
    }
    else if( Value->type == SS_CT_VALUE_TYPE_LONG )
    {
      DataItem->bDataType = SS_DATATYPE_INTEGER;
      DataItem->Data.lValue = Value->u.lValue;
      bRet = TRUE;
    }
    //else if( Value->type == SS_CT_VALUE_TYPE_HANDLE )
    //{
    //  DataItem->bDataType = SS_DATATYPE_HANDLE;
    //  DataItem->Data.Handle.hValue = Value->u.Handle.hValue;
    //  DataItem->Data.Handle.fOwn = Value->u.Handle.fOwn;
    //  DataItem->Data.Handle.pFreeFunc = Value->u.Handle.pfnFreeHandle;
    //  // DataItem takes over ownership and freeing
    //  Value->u.Handle.pfnFreeHandle = NULL;
    //  Value->u.Handle.fOwn = FALSE;
    //}
  }
  return bRet;
}
void SS_FreeCTValue(LPSS_CT_VALUE Value)
{
  if( Value )
  {
    if( Value->type == SS_CT_VALUE_TYPE_TSTR && Value->u.hszValue )
    {
      GlobalFree(Value->u.hszValue);
      Value->u.hszValue = 0;
    }
    else if( Value->type == SS_CT_VALUE_TYPE_BUFFER && Value->u.Buffer.hBuffer )
    {
       GlobalFree(Value->u.Buffer.hBuffer);
       Value->u.Buffer.hBuffer = 0;
    }
    //else if( Value->type == SS_CT_VALUE_TYPE_HANDLE && Value->u.Handle.hValue )
    //{
    //  if( Value->u.Handle.pfnFreeHandle )
    //    Value->u.Handle.pfnFreeHandle(Value->u.Handle.hValue);
    //  else if( Value->u.Handle.fOwn && Value->u.Handle.hValue )
    //    GlobalFree(Value->u.Handle.hValue);
    //  Value->u.Handle.hValue = NULL;
    //  Value->u.Handle.fOwn = FALSE;
    //  Value->u.Handle.pfnFreeHandle = NULL;
    //}
  }
}
#endif