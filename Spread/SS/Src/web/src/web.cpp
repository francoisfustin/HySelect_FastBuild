/*********************************************************
* web.cpp
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

// UPDATE LOG:
// jb  02.10.99 JIS4186  Initial value of checkbox should be 0
// jb  02.11.99 GIC8469	 Don't write a logfile if not specified.
// jb  02.16.99 JIS4351  If anchor is blank, make anchor default to URL
// jb  02.16.99 JIS4352  If import .xls file with URL, Excel strips the protocol off sending 
//	 					 //www.fpoint.com.  Strip off the // and put http://
// jb  07.07.99 SCS8516  Fixed multiple things that have to do with columnspanning.
// jb  07.21.99 GIC10056 Made web safe colors work with system colors.
// jb  08.03.99 JIS4550  No more duplicate jpeg's for same bitmap's, icon's
// jb  08.03.99 JIS4621  logfilefooter() now points to the lpHtml->tcBuf instead of tcBuf.
// jb  08.06.99 KEM35    writemsg() opens the logfile for write instead of append.
// jb  08.12.99 JIS4536  Verticle/Horizontal alignment was not working properly for pictures.
// jb  08.12.99 KEM36    TypePictCenter and MainTainScale were not working properly for pictures.
// jb  08.13.99 GIC10056 The system color black was not working.
// jb  09.26.00 JIS4080  The lock back color was not working...had been commented out.
// jb  05.24.01 JIMS8969 Lock color not working in DLL and password was showing up, instead of *****
// jb  05.25.01 JIMS8984 Shadow colors not working for static data cells.
// jb  05.25.01 JIMS8988 For colspan, if next cell has a different color, it was spanning.  bgcolorthesame()
//						 stoped this.  Code in columnspanning function.
// jb  27.jul.01 Tiwanna reported bug 9175.  Lockbackcolor not being exported.


#ifdef SS_V30

// fix for bug 10020 -scl
// find and replace all occurances of "BUFSIZE" with "BUFSIZ*10"

#include <windows.h>
#include <tchar.h>
#include <stdio.h>
#include <io.h>
#include <time.h>
#include <direct.h>
#include <sys/stat.h>
#include <stdlib.h>
#include "..\..\sssrc\spread\spread.h"
#include "..\..\sssrc\spread\ss_alloc.h"
#include "..\..\sssrc\spread\ss_draw.h"
#include "..\..\sssrc\spread\ss_bool.h"
#include "..\..\sssrc\spread\ss_virt.h"
#include "..\..\sssrc\spread\ss_data.h"
#include "..\..\sssrc\spread\ss_main.h"
#include "..\..\sssrc\spread\ss_user.h"
#ifdef SS_V40
#include "..\..\sssrc\spread\ss_span.h"
#include "..\..\sssrc\spread\ss_html.h"
#endif

#ifdef SS_V70
#include "..\..\..\fplibs\CxImage\CxImage\fpimages.h"
#define IGNORE_CLASSES
#include "..\..\..\fplibs\CxImage\CxImage\ximage.h"
#pragma warning(default:4117)
#endif

#define STRINGS_RESOURCE 1
#define SS_HTML_H 1
#include "web.h"

#include "..\..\sssrc\calc\cal_cell.h"
#include "..\..\sssrc\calc\cal_expr.h"
#include "..\..\sssrc\calc\cal_func.h"

// jb - 18.sep.98
//
// Initialize our log file to write error/information messages to.  Remove the old one if
// it exists.  If one was not specified, set to null.
//
static void init_logfile (LPCTSTR lpszLogFile, LPHTML lpHtml)
{
	if (lpszLogFile)
	{	_tcscpy (lpHtml->tcLogFile, lpszLogFile);
		_tremove (lpszLogFile);
	}
	else
	{	lpHtml->tcLogFile[0] = 0;
	}

	return;
}

// jb - 25.aug.99 ---------------------------------------------------------
//
// Compliments of Robby, needed to implement virtual mode in the event a customer
// will export during virtual mode. (unlikely)
//
void virtualmode (LPSPREADSHEET lpSS, SS_COORD *lRowStart, SS_COORD *lRowEnds)
{
    BOOL fNoVirtual = TRUE;
    BOOL fContinueProcessing = TRUE;
    BOOL fAtVirtualEnd = FALSE;
	SS_COORD lRowAt = *lRowStart;

	while (fContinueProcessing)
	{	if (lpSS->Virtual.VMax != 0 && lpSS->Row.Max > 0 && !fAtVirtualEnd)
		{	fNoVirtual = FALSE;
			*lRowStart = lRowAt;

			while ((*lRowStart < lpSS->Virtual.VTop || 
					*lRowStart >= lpSS->Virtual.VTop + lpSS->Virtual.VPhysSize) && 
					!fAtVirtualEnd)
			{	lpSS->Row.UL = lRowAt;
				fContinueProcessing = TRUE;
				fAtVirtualEnd = SS_VQueryData(lpSS, lRowAt);

				if (lpSS->Virtual.VMax != -1 && 
					lpSS->Virtual.VTop + lpSS->Virtual.VPhysSize - lpSS->Row.HeaderCnt >= lpSS->Virtual.VMax)
				{	fAtVirtualEnd = TRUE;
				}
			}

			*lRowEnds += lpSS->Virtual.VSize;
		}
		else
		{	fContinueProcessing = FALSE;
		}
	}

	return;
}

// jb - 18.sep.98
//
// Virtual mode.
//
void init_virtualmode (LPSPREADSHEET lpSS, SS_COORD *lRowStart, SS_COORD *lRowEnd)
{
	if (lpSS->fVirtualMode)
	{	virtualmode (lpSS, lRowStart, lRowEnd);
	}

	return;
}

// jb - 27.jul.98 ---------------------------------------------------------
//
// We will default the table to something that looks 'standard'.
// Don't let columns wrap, NOWRAP
//
void writetabletag (LPSPREADSHEET lpSS, FILE **fp, LPHTML lpHtml)
{
	TCHAR tcBuf[100], buf[20];
	WORD wGridType;
	SS_COORD numcols;

	// We won't be writing a table tag out if we are appending.
	//
	if (lpHtml->bAppendFlag) return;

	// Do the default of spacing and padding of 0.
	//
	_tcscpy (tcBuf, _T("<TABLE CELLSPACING=0 CELLPADDING=0"));

	// If we don't want gridlines AND no border, then set border to 0.  Otherwise, we get no
	// gridlines but a border around the table since the default is BORDER=1 for HTML tables.
	// Found out that if we do want a border, for netscape, must define BORDER=1 for gridlines.
	//
	if (lpHtml->bNoBorder)
	{	_tcscat (tcBuf, _T(" BORDER=0"));
	}
	else
	{	_tcscat (tcBuf, _T(" BORDER=1"));
	}

	// RULES attribute:
	// Here, we can check for either horizontal, vertical, or no grid.
	//
	wGridType = SSGetGridType (lpSS->lpBook->hWnd);
	if (wGridType == SS_GRID_HORIZONTAL || wGridType == (SS_GRID_HORIZONTAL | SS_GRID_SOLID))
		_tcscat (tcBuf, _T(" RULES=ROWS"));

	else if (wGridType == SS_GRID_VERTICAL || wGridType == (SS_GRID_VERTICAL | SS_GRID_SOLID))
		_tcscat (tcBuf, _T(" RULES=COLS"));
	
	else if (wGridType == (SS_GRID_HORIZONTAL | SS_GRID_VERTICAL) ||
			 wGridType == (SS_GRID_HORIZONTAL | SS_GRID_VERTICAL | SS_GRID_SOLID))
		_tcscat (tcBuf, _T(" RULES=ALL"));
	else
		_tcscat (tcBuf, _T(" RULES=NONE"));

	// This tell our table how many columns we have in advance and make them the same size.  Make
	// the columns one more than we need since when we have many columns on a spreadsheet, the very
	// last column gets the width left out.  The extra column doesn't hurt anything.
	//
	// jb - 22.mar.01
	// We need the COLS= for every HTML export.  For some reason, it makes the
	// widths come out more accurate.
//	if (!lpHtml->bAllowCellOverFlow)
//	{
	_tcscat (tcBuf, _T(" COLS="));
	numcols = (lpHtml->lColEnd - lpHtml->lColStart + 1);
  if( lpHtml->fDisplayRowHeaders )
    numcols += lpSS->Col.HeaderCnt;
	_tcscat (tcBuf, (_ltot (numcols, buf, 10)));
//	}
	_tcscat (tcBuf, _T(">\n"));

	_fputts(tcBuf, *fp);
	tcBuf[0] = 0;

	return;
}

// jb - 18.sep.98
//
// Start our <table> tags.  If there is no information to put into the table, close the table tag.
//
BOOL startoftable (LPSPREADSHEET lpSS, FILE **fp, LPHTML lpHtml)
{
	BOOL bRet = FALSE;

	writetabletag (lpSS, fp, lpHtml);

	if (lpHtml->fNoTable)
	{	endtable (fp, lpHtml);
		logfilefooter (lpHtml);
	 	bRet = TRUE;
	}

	return bRet;
}

// jb - 02.nov.98 ------------------------------------------------------------------
//
// Write the information message to logfile specified from Export method.
//
void writemsg (LPHTML lpHtml)
{
	FILE *fp;

	// gic8469 - 02.11.99
	// kem35 - 06.aug.99 changed open logfile from "a" to "w"
	if (lpHtml->tcLogFile[0] != 0)
	{	fp = _tfopen (lpHtml->tcLogFile, _T("a"));
		if (!fp) return;
		_fputts(lpHtml->tcBuf, fp);
		fclose (fp);
	}

	return;
}

// jb - 22.dec.94 -------------------------------------------------------------------
//
// Generate a file name which is unique. The question marks will be replaced with valid 
// filename characters.
//
TCHAR *randomname (TCHAR *tcRndName, TCHAR *tcPath, TCHAR *tcExt) 
{
	int tyrit, val;
	TCHAR *p, tcNewName[FILENAME_MAX + _MAX_PATH];
	TCHAR x[40];
	
	_tcscpy  (x, _T("0123456789ABCDEFGHIJKLMNOPQRSTUVWXYZ"));
	_tcscpy  (tcNewName, tcPath);

	// Prime the name for random ness.
	//
	_tcscat (tcNewName, _T("SPR?????."));
	_tcscat (tcNewName, tcExt);

	// tyrit a number of times to generate a unique name.
	//
	for (tyrit = 0; tyrit < 100; ++tyrit) 
	{	_tcscpy (tcRndName, tcNewName);
		p = tcRndName;

		// Replace each question mark with a character from the list.  
		//
		while ((p = _tcschr(p, _T('?'))) != NULL)
		{	val = x[rand() % ((sizeof(x) - 1) * sizeof (TCHAR))];

			// There was an error when I converted this to a TCHAR.  Sometimes,
			// out of range errors would occur so I validate to make sure we are
			// 0 - 9 and A - Z, otherwise, try again.
			//
			if (val >= 48 && val <= 57 || val >= 65 && val <= 90)
				*p = val;
		}

		// Don't want to randomly spell a bad word....
		//
		p = _tcschr (tcRndName, '.');
		*p--;*p--;*p--;*p = '9';

		// If the file does not exist, return.
		//
		if (_taccess(tcRndName, 0))
		{	break;
		}
	}

	// If we couldn't find a unique name, return the given name.
	//
	return tcRndName;
}

/***********************************************************************
* Name:   ssm_Save4Expr - Evaluate the formula stack and build an Excel
*                         BIFF representation of the formula.
*
* Usage:  long ssm_Save4Expr(LPSPREADSHEET lpSS, SS_COORD lCol, SS_COORD lRow,
*                            LPCALC_INFO lpCalc, CALC_HANDLE hExpr,
*                            LPTBGLOBALHANDLE lpgh, LPLONG lplen)
*           lpSS - pointer to the Spread structure.
*           lCol - column
*           lRow - row
*           lpCalc - <may be needed in future>
*           hExpr - handle to the formula expression.
*           lpgh - buffer to contain formula.
*           lplen - length of the resulting buffer.
*
* Return: long - Success or Failure
***********************************************************************/
long ssm_Save4Expr(LPSPREADSHEET lpSS, SS_COORD lCol, SS_COORD lRow, LPCALC_INFO lpCalc, CALC_HANDLE hExpr, LPTBGLOBALHANDLE lpgh, LPLONG lplen)
{
	LPCALC_EXPRINFO lpExpr = NULL;
	TBGLOBALHANDLE ghURL = (TBGLOBALHANDLE)0;
	long           lURLLen = 0;
	TBGLOBALHANDLE ghAnchor = (TBGLOBALHANDLE)0;
	long           lAnchorLen = 0;
	short          nParmCount = 0;
	LPTSTR         lptstrData = NULL;
	BOOL           fErr = FALSE;
	BOOL           bRet = FALSE;
	
	if (hExpr && (lpExpr = (LPCALC_EXPRINFO)CalcMemLock(hExpr)))
	{	BOOL fErr = FALSE;
		for( ; lpExpr->nType != CALC_EXPR_END; lpExpr++ )
		{	switch( lpExpr->nType )
			{	case CALC_EXPR_CELL:
				{	if (!nParmCount) //The first parameter -- must be URL	
					{	lURLLen = SS_GetDataEx(lpSS, lpExpr->u.Cell.lCol, lpExpr->u.Cell.lRow, NULL, -1) + 1;
						ghURL = (TBGLOBALHANDLE)tbGlobalAlloc(GHND, lURLLen);
						lptstrData = (LPTSTR)tbGlobalLock(ghURL);
						SS_GetDataEx(lpSS, lpExpr->u.Cell.lCol, lpExpr->u.Cell.lRow, lptstrData, -1);
						tbGlobalUnlock(ghURL);
						nParmCount++;
					}
					else //The second parameter -- must be Anchor (optional)
					{	lAnchorLen = SSGetDataLen(lpSS->lpBook->hWnd, lpExpr->u.Cell.lCol, lpExpr->u.Cell.lRow) + 1;
						ghAnchor = (TBGLOBALHANDLE)tbGlobalAlloc(GHND, lAnchorLen);
						lptstrData = (LPTSTR)tbGlobalLock(ghAnchor);
						SS_GetDataEx(lpSS, lpExpr->u.Cell.lCol, lpExpr->u.Cell.lRow, lptstrData, -1);
						tbGlobalUnlock(ghAnchor);
						nParmCount++;
					}
		        }
		        break;

				// Note: steal the string extraction part out of Robby's code, a case expression.
				//
				case CALC_EXPR_FUNCTION:
		        {	LPTSTR lptstrURL = NULL;
					LPTSTR lptstrAnchor = NULL;
					LPTSTR lptstrFunction = NULL;

					switch(lpExpr->u.Func.nId)
					{	case CALC_FUNC_URL:
							switch (nParmCount)
							{	case 0:
									//Error
							 		fErr = TRUE;
									break;
								case 1:
								{	//One parameter -- URL
									*lpgh = tbGlobalAlloc (GHND, lURLLen+lAnchorLen+100); // Gotta start somewhere...
									lptstrFunction = (LPTSTR)tbGlobalLock(*lpgh);

									if (lURLLen)
									{	lptstrURL = (LPTSTR)tbGlobalLock(ghURL);
										lstrcpy (lptstrFunction, _T("<A HREF=\""));

										// JQB03 - 02.16.99
										// Excel sends //www.fpoint.com  I don't know why they don't perserve the protocol
										// of http:// but they don't so I will have to strip the // off, then
										// default the protocol to http://
										//
										if (!_tcsncmp(lptstrURL, _T("//"), 2))
										{	lptstrURL++;
											lptstrURL++;
											lstrcat (lptstrFunction, _T("http://"));
										}

										// If the user left off the type of transfer protocol, then we will assume
										// a default of http://
										//
										else if (!_tcsstr (lptstrURL, _T("://")))
										{	lstrcat (lptstrFunction, _T("http://"));
										}

										lstrcat (lptstrFunction, lptstrURL);
										lstrcat (lptstrFunction, _T("\">"));
										lstrcat (lptstrFunction, lptstrURL);
										lstrcat (lptstrFunction, _T("</A>"));
	
										tbGlobalUnlock(ghURL);
										tbGlobalUnlock(*lpgh);
										*lplen = lstrlen(lptstrFunction) + 1;
										*lpgh = tbGlobalReAlloc(*lpgh, *lplen, GHND); //snap the memory to the right size.
									}
									else
									{	fErr = TRUE;
										tbGlobalUnlock(*lpgh);
									}
								}
								break;
								case 2:
									//Two parameters -- URL, Anchor
									*lpgh = tbGlobalAlloc(GHND, lURLLen+lAnchorLen+100); // Gotta start somewhere...
									lptstrFunction = (LPTSTR)tbGlobalLock(*lpgh);
									if (lURLLen && lAnchorLen)
									{	lptstrURL = (LPTSTR)tbGlobalLock(ghURL);
										lptstrAnchor = (LPTSTR)tbGlobalLock(ghAnchor);
										lstrcpy (lptstrFunction, _T("<A HREF=\""));
		
										// JQB03 - 02.16.99
										// Excel sends //www.fpoint.com  I don't know why they don't perserve the protocol
										// of http:// but they don't so I will have to strip the // off, then
										// default the protocol to http://
										//
										if (!_tcsncmp(lptstrURL, _T("//"), 2))
										{	lptstrURL++;
											lptstrURL++;
											lstrcat (lptstrFunction, _T("http://"));
										}

										// If the user left off the type of transfer protocol, then we will assume
										// a default of http://
										//
										else if (!_tcsstr (lptstrURL, _T("://")))
										{	lstrcat (lptstrFunction, _T("http://"));
										}

										lstrcat (lptstrFunction, lptstrURL);
										lstrcat (lptstrFunction, _T("\">"));

										// JQB02 - 02.16.99 - This means that there is no URL.
										//
										if (lURLLen == 1)
										{	fErr = TRUE;
											tbGlobalUnlock(*lpgh);
											break;
										}

										// JQB02 - 02.16.99
										// In this case, if length is one, then there is no anchor.  Default to URL.
										//
										if (lAnchorLen == 1)
										{	lstrcat (lptstrFunction, lptstrURL);
										}
										else
										{	lstrcat (lptstrFunction, lptstrAnchor);
										}

										lstrcat (lptstrFunction, _T("</A>"));

										tbGlobalUnlock(ghURL);
										tbGlobalUnlock(ghAnchor);
										tbGlobalUnlock(*lpgh);
										*lplen = lstrlen(lptstrFunction) + 1;
										*lpgh = tbGlobalReAlloc(*lpgh, *lplen * sizeof(TCHAR), GHND);
									}
									else
									{	fErr = TRUE;
										tbGlobalUnlock(*lpgh);
									}
									break;
							} //switch
						break;
					}
				}
				break;

				case CALC_EXPR_STRING:
				{
					LPCTSTR lpctstr = (LPTSTR)CalcMemLock(lpExpr->u.String.hText);
			
					if (!nParmCount) //The first parameter -- must be URL	
					{	lURLLen = lstrlen(lpctstr) + 1;
						ghURL = (TBGLOBALHANDLE)tbGlobalAlloc(GHND, lURLLen * sizeof(TCHAR));
						lptstrData = (LPTSTR)tbGlobalLock(ghURL);
						memcpy(lptstrData, lpctstr, (lURLLen-1) * sizeof(TCHAR));
						tbGlobalUnlock(ghURL);
						nParmCount++;
					}
					else //The second parameter -- must be Anchor (optional)
					{	lAnchorLen = lstrlen(lpctstr) + 1;
						ghAnchor = (TBGLOBALHANDLE)tbGlobalAlloc(GHND, lAnchorLen * sizeof(TCHAR));
						lptstrData = (LPTSTR)tbGlobalLock(ghAnchor);
						memcpy(lptstrData, lpctstr, (lAnchorLen-1) * sizeof(TCHAR));
						tbGlobalUnlock(ghAnchor);
						nParmCount++;
					}

					CalcMemUnlock(lpExpr->u.String.hText);
				}
				break;

			} //switch

			if (fErr)
			{	if (*lpgh)
				{	tbGlobalFree(*lpgh);
					*lpgh = 0;
				}

		        CalcMemUnlock(hExpr);
		        return TRUE;
			}
		} //for
	    CalcMemUnlock(hExpr);
	} //if
    return bRet;
}


/***********************************************************************
* Name:   ssm_GetFormula - Retrieve a formula from a cell, or range of
*                          cells.
*
* Usage:  BOOL ssm_GetFormula(LPSPREADSHEET lpSS, SS_COORD lCol, SS_COORD lRow,
*                             LPTBGLOBALHANDLE lpgh, LPLONG lplen)
*           lpSS - pointer to the Spread structure.
*           lCol - column
*           lRow - row
*           lpgh - buffer
*           lplen - buffer length
*
* Return: BOOL - Is there a formula here ?
***********************************************************************/
BOOL ssm_GetFormulaEx(LPSPREADSHEET lpSS, SS_COORD lCol, SS_COORD lRow, LPTBGLOBALHANDLE lpgh, LPLONG lplen)
{
	CALC_HANDLE hExpr;
	long        lColMin;
	long        lColMax;
	long        lRowMin;
	long        lRowMax;
	long        lMaxRows = lpSS->Row.Max;
	long        lMaxCols = lpSS->Col.Max;
	long        i;
	long        j;
	BOOL        fIsFormula = FALSE;

	if (SS_ALLCOLS == lCol)
	{	lColMin = min(MatMinCol(&lpSS->CalcInfo.Cells), VectMinIndex(&lpSS->CalcInfo.Cols));
		lColMax = max(MatMaxCol(&lpSS->CalcInfo.Cells), VectMaxIndex(&lpSS->CalcInfo.Cols));
	}
	else
	{	lColMin = lCol;
		lColMax = lCol;
	}

	if (SS_ALLROWS == lRow)
	{	lRowMin = min(MatMinRow(&lpSS->CalcInfo.Cells), VectMinIndex(&lpSS->CalcInfo.Rows));
		lRowMax = max(MatMaxRow(&lpSS->CalcInfo.Cells), VectMaxIndex(&lpSS->CalcInfo.Rows));
	}
	else
	{	lRowMin = lRow;
		lRowMax = lRow;
	}

	for (i = lColMin; i <= lColMax; i++)
	{	for (j = lRowMin; j <= lRowMax; j++)
		{	if (hExpr = CellGetExpr(&lpSS->CalcInfo, i, j))
			{	fIsFormula = TRUE;
				if (ssm_Save4Expr(lpSS, lCol, lRow, &lpSS->CalcInfo, hExpr, lpgh, lplen))
				{	fIsFormula = FALSE;
					break;
				}
			}
		}
		if (fIsFormula == FALSE)
		break;
	}

	return fIsFormula;
}

// jb - 06.nov.98 ---------------------------------------------------------
//
// Convert up to a 8 byte hex 'string' to a decimal value. ex. "FF" = 255 decimal
//
static LONG hex2decimalfromsz (TCHAR *hex)
{
	TCHAR tcBuf[16];
	LONG lValue;
	INT i, x;

	if (_tcslen (hex) > 8) return 0;

	// Make any hex digits over 9 upper case then reverse the string for easier
	// conversion.
	//
	_tcsncpy (tcBuf, hex, 15);
	_tcsrev (tcBuf);
	_tcsupr (tcBuf);

	lValue = 0L;
	for (i = 0; tcBuf[i] != 0; i++)
	{	x = tcBuf[i];

		// These are values 0 - 9
		//
		if (x >= 48 && x <= 57)
		{	if (i == 0)
				lValue = (x - 48);
			else
				lValue += ((LONG)pow((float)16, i) * (x - 48));
		}

		// These are values A - F
		//
		else if (x >= 65 && x <= 70)
		{	if (i == 0)
				lValue = (x - 55);
			else
				lValue += ((LONG)pow((float)16, i) * (x - 55));
		}

		// We encountered something invalid.  Not 0-9 or A-F
		//
		else
			return -1;
	}

	return lValue;
}

// jb - 07.nov.98 ---------------------------------------------------------------------
//
// Since there are only 216 web safe colors and they can only be FF, CC, 99, 66, 33, 00
// per R, G, B, we must snap to those values.  (which ever one is the closest)  We do 
// this by getting the midpoint in decimal and determine which is closest to the web 
// compatible color.
//
// Hex		FF		CC		99		66		33		00
// Dec		255		204		153		102		51		0
// MidPoint		229		178		127		76		25
//
static TCHAR *snap2websafecolor (LONG color)
{
	static TCHAR tcWebSafeColor[3];

	tcWebSafeColor[0] = 0;

	// Black and white will be very common so we will try these first and eliminate the need
	// to go through more code than we have to.
	//
	if (color <= 0)
	{	_tcscpy (tcWebSafeColor, _T("00"));
	}
	else if (color >= 255)
	{	_tcscpy (tcWebSafeColor, _T("FF"));
	}

	if (tcWebSafeColor[0] != 0)
	{	return tcWebSafeColor;
	}

	// Starting from Black, 00 we go down the list comparing the mid-point of
	// our color range.  Depending on how close we are to the next valid web color,
	// we will go the to 'closest' web compliant color.
	//
	if (color > 0 && color < 51)
	{	if (color <= 25)
			_tcscpy (tcWebSafeColor, _T("00"));
		else
			_tcscpy (tcWebSafeColor, _T("33"));
	}

	else if (color >= 51  && color < 102)
	{	if (color <= 76)
			_tcscpy (tcWebSafeColor, _T("33"));
		else
			_tcscpy (tcWebSafeColor, _T("66"));
	}

	else if (color >= 102 && color < 153)
	{	if (color <= 127)
			_tcscpy (tcWebSafeColor, _T("66"));
		else
			_tcscpy (tcWebSafeColor, _T("99"));
	}

	else if (color >= 153 && color < 204)
	{	if (color <= 178)
			_tcscpy (tcWebSafeColor, _T("99"));
		else
			_tcscpy (tcWebSafeColor, _T("CC"));
	}

	else if (color >= 204 && color < 255)
	{	if (color <= 229)
			_tcscpy (tcWebSafeColor, _T("CC"));
		else
			_tcscpy (tcWebSafeColor, _T("FF"));
	}

	return tcWebSafeColor;
}

// jb - 07.nov.98 ---------------------------------------------------------
//
// Take our RGB color passed in (each parameter is a 2 byte hex string) for
// red, green, and blue and convert them to a web safe color that our browser
// can understand.
// 
static TCHAR *makewebsafecolor (TCHAR *r, TCHAR *g, TCHAR *b, LPHTML lpHtml)
{
	LONG red, green, blue;

	*lpHtml->tcBuf = 0;

	// Convert the red, green, and blue hex values from a character to the
	// decimal representation.  ex - "FF" = 255  We have already checked for
	// a bad value so we won't check for the -1 condition here.
	// 
	red   = hex2decimalfromsz (r);
	green = hex2decimalfromsz (g);
	blue  = hex2decimalfromsz (b);
	
	// There are 216 web safe colors.  They can only be (for each red, green, blue)
	// FF, CC, 99, 66, 33, 00
	//
	_tcsncpy (lpHtml->tcBuf, snap2websafecolor (red), 2);
	_tcsncpy (&lpHtml->tcBuf[2], snap2websafecolor (green), 2);
	_tcsncpy (&lpHtml->tcBuf[4], snap2websafecolor (blue), 2);
	lpHtml->tcBuf[6] = 0;

	return lpHtml->tcBuf;
}

// jb - 13.aug.99 ---------------------------------------------------------
//
// If this is a system color 0-18, then use GetSysColor instead of SSGetColor
// to get the RGB value.
// GIC10056
// 
static void check4systemcolors (COLORREF *clrBackGround, COLORREF *clrForeGround, BOOL bBGColor)
{
	COLORREF rgbB, rgbF;

	rgbB = rgbF = 0;

	// Is this a system color ? 0x80000000
	//
	if (bBGColor)
	{	rgbB = (*clrBackGround >> 28) & 0x0000000F;
	}
	else
	{	rgbF = (*clrForeGround >> 28) & 0x0000000F;
	}

	// We are not a system color, so return.
	//
	if  ((bBGColor && rgbB != 8) || (!bBGColor && rgbF != 8)) return;

	if (bBGColor)
	{	*clrBackGround = GetSysColor ((*clrBackGround & 0x000000FF));
	}

	if (!bBGColor)
	{	*clrForeGround = GetSysColor ((*clrForeGround & 0x000000FF));
	}
	return;
}

// jb - 21.jul.99 ---------------------------------------------------------
//
// Convert the last 6 bytes of the color to a string.
//
static void rgbcolor (BOOL bBGColor, LPHTML lpHtml, COLORREF clrBackGround, COLORREF clrForeGround)
{
	if (bBGColor)
	{	
// fix for 8998 -scl
    clrBackGround &= 0xFFFFFF;
    _sntprintf (lpHtml->tcBuf, 6, _T("%06x"), clrBackGround);
	}
	else
	{	
// fix for 8998 -scl
    clrForeGround &= 0xFFFFFF;
    _sntprintf (lpHtml->tcBuf, 6, _T("%06x"), clrForeGround);
	}

	return;
}

// jb - 21.jul.99 ---------------------------------------------------------
//
// The headers are static cells so we have to use the SSGetShadowColor
//
static void shadowcolors (LPSPREADSHEET lpSS, COLORREF *clrBackGround, COLORREF *clrForeGround)
{
	COLORREF x, y;

	SSGetShadowColor (lpSS->lpBook->hWnd, clrBackGround, clrForeGround, &x, &y);
	*clrBackGround = FPCOLOR (*clrBackGround);
	*clrForeGround = FPCOLOR (*clrForeGround);
	
	return;
}

// jb - 25.oct.98 ------------------------------------------------------------------
//
// Get the type of 'type AND style' from a particular cell.
//
static BOOL gettypeof (LPSPREADSHEET lpSS, SS_COORD lCol, SS_COORD lRow, BYTE nType, LONG lStyle)
{
	BOOL bRet = FALSE;
	SS_CELLTYPE cellType;

	if (SS_RetrieveCellType (lpSS, &cellType, NULL, lCol, lRow))
	{	if (nType == cellType.Type)
		{	bRet = cellType.Style & lStyle ? TRUE : FALSE;
		}
	}

	return bRet;
}

// jb - 05.nov.98 ------------------------------------------------------------------
//
// Get the type of 'style' from a particular cell.
//
static BOOL gettypeofstyle (LPSPREADSHEET lpSS, SS_COORD lCol, SS_COORD lRow, LONG lStyle)
{
	BOOL bRet = FALSE;
	SS_CELLTYPE cellType;

	if (SS_RetrieveCellType (lpSS, &cellType, NULL, lCol, lRow))
	{	bRet = cellType.Style & lStyle ? TRUE : FALSE;
	}				  

	return bRet;
}

// jq - 12.nov.98 ------------------------------------------------------------------
//
// Get the type of 'type' from a particular cell.
//
static BOOL gettypeoftype (LPSPREADSHEET lpSS, SS_COORD lCol, SS_COORD lRow, LONG nType)
{
	BOOL bRet = FALSE;
	SS_CELLTYPE cellType;

	if (SS_RetrieveCellType (lpSS, &cellType, NULL, lCol, lRow))
	{	if (cellType.Type == nType)
		{	bRet = TRUE;
		}
	}

	return bRet;
}

// jb - 25.may.01 -------------------------------------------------------------
//
// If we have a spread cell that is static, and the shadow color has been
// set, then we don't want ALL static cells to have the color of the shadow.
// Only the ones that have the raised or lowered check box checked for that
// 3-D effect.  (in spread designer)  Or, the properties TypeTextShadow and
// TypeTextShadowIn
//
static BOOL isshadowraisedorlower (LPSPREADSHEET lpSS, SS_COORD lCol, SS_COORD lRow)
{
	SS_CELLTYPE cellType;
	BOOL bValue = FALSE;

	if (SS_RetrieveCellType (lpSS, &cellType, NULL, lCol, lRow))
	{	// See if shadow is raised
		//
		bValue = cellType.Style & SS_TEXT_SHADOW ? TRUE : FALSE;

		// See if shadow is lowered
		//
		if (!bValue)
		{	bValue = cellType.Style & SS_TEXT_SHADOWIN ? TRUE : FALSE;
		}
	}

	return bValue;
}

// jb - 23.jul.98 ---------------------------------------------------------
//
// Query the spread cell for the color
//
// Note: The COLORREF stores colors as 4 bytes 0x 00BBGGRR but in HTML the colors are 
// stored 3 bytes 0x RRGGBB (reverse and 1st byte on the colorref is ignored)
//
static TCHAR *gethtmlcolor (LPSPREADSHEET lpSS, SS_COORD lCol, SS_COORD lRow, BOOL bBGColor, LPHTML lpHtml)
{
	COLORREF clrBackGround, clrForeGround;
	TCHAR r[3], g[3], b[3];
  LPSS_ROW      lpRow = SS_LockRowItem(lpSS, lRow);
  LPSS_COL      lpCol = SS_LockColItem(lpSS, lCol);

	// HEADER cell
	// Make sure 6 char's long, if not, preceed with 0  
	// For the headers use the GetShadowColor since they are static text.
	//
	if (lpHtml->bHeader)
	{	
    if (SSxx_GetLock (lpSS, lpCol, lpRow, NULL, lCol, lRow, TRUE))
		{	if ((lpHtml->clrLockBack == SPREAD_COLOR_NONE || lpHtml->clrLockFore == SPREAD_COLOR_NONE) ||
		      (lpHtml->clrLockBack == RGBCOLOR_DEFAULT || lpHtml->clrLockFore == RGBCOLOR_DEFAULT))
			{	shadowcolors (lpSS, &clrBackGround, &clrForeGround);
			}
			else
			{	clrBackGround = lpHtml->clrLockBack;
			 	clrForeGround = lpHtml->clrLockFore;
			}
		}
		else
		{	shadowcolors (lpSS, &clrBackGround, &clrForeGround);
		}

		rgbcolor (bBGColor, lpHtml, clrBackGround, clrForeGround);
	}

	// DATA cell
	//
	else
// fix for bug 9235 -scl
//	{	if (SSGetLock (lpSS->lpBook->hWnd, lCol, lRow))
	{	if (SSGetBool(lpSS->lpBook->hWnd, SSB_PROTECT) && SSxx_GetLock (lpSS, lpCol, lpRow, NULL, lCol, lRow, TRUE))
		{	clrBackGround = lpHtml->clrLockBack;
		 	clrForeGround = lpHtml->clrLockFore;

// fix for bug 9318 -scl
//      if( SPREAD_COLOR_NONE == clrBackGround )
      if( SPREAD_COLOR_NONE == clrBackGround || RGBCOLOR_DEFAULT == clrBackGround )
        SS_GetColor(lpSS, lCol, lRow, &clrBackGround, NULL);
//      if( SPREAD_COLOR_NONE == clrForeGround )
      if( SPREAD_COLOR_NONE == clrForeGround || RGBCOLOR_DEFAULT == clrForeGround )
        SS_GetColor(lpSS, lCol, lRow, NULL, &clrForeGround);
			check4systemcolors (&clrBackGround, &clrForeGround, bBGColor);
// 27.jul.01 - Tiwanna reported 9175
//			SSGetColor (lpSS->lpBook->hWnd, lCol, lRow, &clrBackGround, &clrForeGround);
		}
		else
		{	SS_GetColor (lpSS, lCol, lRow, &clrBackGround, &clrForeGround);
			check4systemcolors (&clrBackGround, &clrForeGround, bBGColor);
		}

		// JIMS8984 - jb - 26.may.01
		// See if we have static data, then see if it is 3-D.  Only want to export the
		// shadow color if it is 3-D (raised or lowered)
		//
		if (gettypeoftype (lpSS, lCol, lRow, SS_TYPE_STATICTEXT))
		{	if (isshadowraisedorlower (lpSS, lCol, lRow))
			{	shadowcolors (lpSS, &clrBackGround, &clrForeGround);
			}
		}

		rgbcolor (bBGColor, lpHtml, clrBackGround, clrForeGround);
	}

	lpHtml->tcBuf[6] = 0;
	
	// Now, get our blue, green and red.
	//
	_tcsncpy (b, lpHtml->tcBuf, 2);
	_tcsncpy (g, &lpHtml->tcBuf[2], 2);
	_tcsncpy (r, &lpHtml->tcBuf[4], 2);
	r[2] = 0;
	g[2] = 0;
	b[2] = 0;

  SS_UnlockColItem(lpSS, lCol);
  SS_UnlockRowItem(lpSS, lRow);
	return makewebsafecolor (r, g, b, lpHtml);
}

// jb - 23.sep.98 ---------------------------------------------------------------------------
//
// Return a date/time stamp, format of dd-mm-yy hh:mm am/pm  23-Sep-98 4:54pm
//
TCHAR *getdatetimestring (TCHAR *timebuf)
{	
	time_t ltime;
	struct tm *today;

	// Sets time zone from TZ environment variable.  If TZ is not set,
	// the OS is queried to obtain the default value.
	//
	_tzset();

	// Get the number of seconds since midnight, Jan 1, 1970 then convert the
	// time value correcting for the local time zone.
	//
	time (&ltime);
	today = localtime (&ltime);

	// Get date in dd mm yy hh:mm am/pm		06 Oct 98 04:56pm
	//
	_tcsftime (timebuf, 32, _T("%d-%b-%y  %I:%M%p"), today);

	return timebuf;
}

// jb - 06.oct.98 ---------------------------------------------------------
//
// Remove file after it gets to be (size) in bytes.  Don't want to fill 
// up the hard drive..  (not used now...)
//
static void killfile (INT iSize, LPCTSTR lpszLogFile)
{	
	struct _stat buf;
	int fRet;

	if (lpszLogFile != 0)
	{	fRet = _tstat (lpszLogFile, &buf);
		if (buf.st_size > iSize)
		{	_tremove(lpszLogFile);
		}
	}

	return;
}

// jb - 1.aug.98 ---------------------------------------------------------
//
// Is the row hidden ?  
//
BOOL rowhidden (LPSPREADSHEET lpSS, SS_COORD lRow)
{
	LPSS_ROW lpRow;
	BOOL fRet = FALSE;

	if (lpRow = SS_LockRowItem (lpSS, lRow))
	{	fRet = (lpRow->fRowHidden ? TRUE : FALSE);
		SS_UnlockRowItem (lpSS, lRow);
	}
	return fRet;
}

// jb - 1.aug.98 ---------------------------------------------------------
//
// Is the column hidden ?
//
BOOL colhidden (LPSPREADSHEET lpSS, SS_COORD lCol)
{
	LPSS_COL lpCol;
	BOOL     fRet = FALSE;

	if (lpCol = SS_LockColItem (lpSS, lCol))
	{	fRet = (lpCol->fColHidden ? TRUE : FALSE);
		SS_UnlockColItem (lpSS, lCol);
	}

	return fRet;
}

// jb - 18.sep.98 ---------------------------------------------------------
//
// If the row is hidden, go to next row and write to the log file that this row
// is hidden.
//
BOOL isrowhidden (LPSPREADSHEET lpSS, SS_COORD lCol, SS_COORD lRow, LPHTML lpHtml)
{
	BOOL bRet = FALSE;
	TCHAR tcBuf[BUFSIZ*10];

	if (rowhidden (lpSS, lRow))
	{	LoadString((HINSTANCE)fpInstance, IDS_LOGS_ROWHIDDEN, tcBuf, BUFSIZ*10-1);
		_stprintf (lpHtml->tcBuf, tcBuf, lCol, lRow);
		writemsg (lpHtml);
		bRet = TRUE;
	}

	return bRet;
}

// jb - 18.sep.98 ---------------------------------------------------------
//
// If the column is hidden, go to next column and write to the log file that this
// column is hidden.
//
BOOL iscolhidden (LPSPREADSHEET lpSS, SS_COORD lCol, SS_COORD lRow, LPHTML lpHtml)
{
	BOOL bRet = FALSE;
	TCHAR tcBuf[BUFSIZ*10];

	// Only if column is greater that 0.  Other code takes care of headers.
	//
 	if (lCol != 0)
	{	if (colhidden (lpSS, lCol))
 		{	LoadString((HINSTANCE)fpInstance, IDS_LOGS_COLHIDDEN, tcBuf, BUFSIZ*10-1);
 			_stprintf (lpHtml->tcBuf, tcBuf, lCol, lRow);
 			writemsg (lpHtml);
 			bRet = TRUE;
 		}
	}

	return bRet;
}

// jb - 17.sep.98 ---------------------------------------------------------------
//
// Check to see if the coordinates passed in are greater than -1
//
static BOOL validatecoordinates (SS_COORD Col1, SS_COORD Row1, SS_COORD Col2, SS_COORD Row2, LPHTML lpHtml)
{
	TCHAR tcBuf[BUFSIZ*10], tcFormat[BUFSIZ*10];
	BOOL bRet = TRUE;

	tcBuf[0] = 0;

	// No such coordinate less than -1.
	//
	if (Col1 < -1)
	{	LoadString((HINSTANCE)fpInstance, IDS_LOGS_COL1INVALID, tcFormat, BUFSIZ*10-1);
		_stprintf (tcBuf, tcFormat, Col1, Row1);
	}

	else if (Col2 < -1)
	{	LoadString((HINSTANCE)fpInstance, IDS_LOGS_COL2INVALID, tcFormat, BUFSIZ*10-1);
		_stprintf (tcBuf, tcFormat, Col1, Row1, Col2);
	}

	else if (Row1 < -1)
	{	LoadString((HINSTANCE)fpInstance, IDS_LOGS_ROW1INVALID, tcFormat, BUFSIZ*10-1);
		_stprintf (tcBuf, tcFormat, Col1, Row1);
	}

	else if (Row2 < -1)
	{	LoadString((HINSTANCE)fpInstance, IDS_LOGS_ROW2INVALID, tcFormat, BUFSIZ*10-1);
		_stprintf (tcBuf, tcFormat, Col1, Row1, Row2);
	}

	else if (Col1 > Col2)
	{	LoadString((HINSTANCE)fpInstance, IDS_LOGS_COLRANGEERROR, tcFormat, BUFSIZ*10-1);
		_stprintf (tcBuf, tcFormat, Col1, Row1, Col2);
	}

	else if (Row1 > Row2)
	{	LoadString((HINSTANCE)fpInstance, IDS_LOGS_ROWRANGEERROR, tcFormat, BUFSIZ*10-1);
		_stprintf (tcBuf, tcFormat, Col1, Row1, Row2);
	}

	if (tcBuf[0] != 0)
	{ 	writemsg (lpHtml);
		bRet = FALSE;
	}

	return bRet;
}

// jb - 15.oct.98 ---------------------------------------------------------------
//
// See if user passed in -1 (SS_ALLCOLS) for the second column coordinate.  If
// so, then user wants to do a range of column cells.
//
static BOOL columnrange (SS_COORD lCol, SS_COORD lRow)
{
	BOOL fRet = FALSE;

	if (lCol == SS_ALLCOLS && lRow >= 0) fRet = TRUE;
	return fRet;
}

// jb - 15.oct.98 ---------------------------------------------------------------
//
// See if user passed in -1 (SS_ALLROWS) for the second row coordinate.  If so,
// then user wants to do a range of rows.
//
static BOOL rowrange (SS_COORD lCol, SS_COORD lRow)
{	
	BOOL fRet = FALSE;
	if (lCol >= 0 && lRow == SS_ALLROWS) fRet = TRUE;

	return fRet;
}

// jb - 14.oct.98 ---------------------------------------------------------------
//
// See if user passed in -1 for the first column and row coordinate.  This means we 
// are exporting the entire spreadsheet.  SS_ALLCOLS and SS_ALLROWS
//
static BOOL entirespreadsheet (SS_COORD lCol, SS_COORD lRow)
{
	BOOL fRet = FALSE;

	if (lCol == SS_ALLCOLS && lRow == SS_ALLROWS) fRet = TRUE;
	return fRet;
}

// jb - 15.oct.98 ---------------------------------------------------------------
//
// See if user passed in -1 (SS_ALLCOLS) for the first column coordinate.  If
// so, then user wants to do ALL of the column cells.
//
static BOOL allcolumns (SS_COORD lCol, SS_COORD lRow)
{
	BOOL fRet = FALSE;

	// -1, 3
	//
	if (lCol == SS_ALLCOLS && lRow >= 0) fRet = TRUE;
	return fRet;
}

// jb - 15.oct.98 ---------------------------------------------------------------
//
// See if user passed in -1 (SS_ALLROWS) for the first row coordinate.  If so,
// then user wants to do a ALL of the rows.
//
static BOOL allrows (SS_COORD lCol, SS_COORD lRow)
{	
	BOOL fRet = FALSE;
	if (lCol >= 0 && lRow == SS_ALLROWS) fRet = TRUE;

	return fRet;
}

// jb - 12.nov.98 ---------------------------------------------------------
//
// We need to make sure the next cell over has the same background color.  If not,
// then we end up spanning the previous columns color over to a different color.
// If the next cells background color is different, return FALSE.
//
static BOOL bgcolorthesame (LPSPREADSHEET lpSS, SS_COORD lCol, SS_COORD lRow)
{
	BOOL bRet = TRUE;

	COLORREF clrBGCurrent, clrFGCurrent, clrBGNext, clrFGNext;

	SS_GetColor (lpSS, lCol, lRow, &clrBGCurrent, &clrFGCurrent);
	SS_GetColor (lpSS, lCol+1, lRow, &clrBGNext, &clrFGNext);

	// If the next cell over has a different color, see if we want to span over it using
	// the previous color IF the text will overflow.
	//
	if (clrBGCurrent != clrBGNext) bRet = FALSE;

	return bRet;
}

// jb - 04.feb.99 ----------------------------------------------------------------------------
//
// We need to know if user has some pictures that are in the last cell (col or row) because
// the data count will not recognize this.
// 
static void	getothercnt (LPSPREADSHEET lpSS, SS_COORD *lColCnt, SS_COORD *lRowCnt)
{
	SS_CELLTYPE CellType;
	SS_COORD lRow, lCol;
	SS_COORD lMaxPictRow, lMaxPictCol;
	SS_COORD lMaxRows = SSGetMaxRows (lpSS->lpBook->hWnd);
	SS_COORD lMaxCols = SSGetMaxCols (lpSS->lpBook->hWnd);

	lMaxPictRow = lMaxPictCol = 0;

	// In virtual mode, rows gets maxed to 1,000,000 which makes the export
	// seem like it is hung when all it is really doing is looping 1/2 a billion times...
	//
	if (lpSS->fVirtualMode)
	{	if (lMaxRows > 500) lMaxRows = 500;
	}

	for (lRow = 1; lRow <= lMaxRows; lRow++)
	{	for (lCol = 1; lCol <= lMaxCols; lCol++)
		{	if (gettypeoftype (lpSS, lCol, lRow, SS_TYPE_PICTURE))
			{	SS_GetCellType (lpSS, lCol, lRow, &CellType);

				// Don't count an empty picture cell.
				//
				if (CellType.Spec.ViewPict.hPictName)
				{	if (lMaxPictCol < lCol)
					{	lMaxPictCol = lCol;
					}
					lMaxPictRow = lRow;
				}
			}

			// Check for 'checkboxes'
			//
			if (gettypeoftype (lpSS, lCol, lRow, SS_TYPE_CHECKBOX))
			{	SS_GetCellType (lpSS, lCol, lRow, &CellType);

				if (lMaxPictCol < lCol)
				{	lMaxPictCol = lCol;
				}
				lMaxPictRow = lRow;
			}
			
			if (!bgcolorthesame (lpSS, lCol, lRow))
			{	if (lMaxPictCol < lCol)
				{	lMaxPictCol = lCol+1;
				}
				lMaxPictRow = lRow;
			}
		}
	}

	if (*lColCnt < lMaxPictCol) *lColCnt = lMaxPictCol;
	if (*lRowCnt < lMaxPictRow) *lRowCnt = lMaxPictRow;

	return;
}

// jb - 27.jul.98 ----------------------------------------------------------------------------
//
// Need to normalize the Row column for start, stop positions with spread.  Validate
// the coordinate ranges.
//
BOOL columnrowcoordinates (LPSPREADSHEET lpSS, SS_COORD *Col1, SS_COORD *Row1, SS_COORD *Col2, SS_COORD *Row2, LPHTML lpHtml)
{
	BOOL bRet = TRUE;
	SS_COORD colcnt, rowcnt;
	SS_COORD c, r;
	TCHAR tcBuf[BUFSIZ*10-1], tcFormat[BUFSIZ*10];

	tcBuf[0] = 0;

	// Get the last maximum col/row number with data.
	//
	SS_GetDataCnt  (lpSS, &colcnt, &rowcnt);
   colcnt--;
   rowcnt--;
	getothercnt (lpSS, &colcnt, &rowcnt);

	// Are we doing the entire spreadsheet ?  This happens when ExportToHTML is called.
	// -1, -1  ---> 0, 0, maxcol, maxrow
	//
	if (entirespreadsheet(*Col1, *Row1))
	{	*Col1 = 0;
		*Row1 = 0;
		*Col2 = colcnt;
		*Row2 = rowcnt;
	}

	// We are doing ALL columns: -1, 3	---> 0, 3, maxcol, row
	//
	if (allcolumns (*Col1, *Row1))
	{	*Col1 = 0;
		*Col2 = colcnt;
	}

	// We are doing ALL rows: 3, -1	--->3, 0, col, maxrow
	//
	else if (allrows (*Col1, *Row1))
	{	*Row1 = 0;
		*Row2 = rowcnt;
	}

	// We are doing a column range, --->0, 0, maxcol, row
	//
	if (columnrange (*Col2, *Row2))
	{	*Col2 = colcnt;
	}

	// We are doing a column range,  --->0, 0, col, maxrow
	//
	if (rowrange (*Col2, *Row2))
	{	*Row2 = rowcnt;
	}

	// Make sure the coordinates make sense.  Not < -1, first coord greater than second etc.
	//
	if (!validatecoordinates (*Col1, *Row1, *Col2, *Row2, lpHtml))
		return FALSE;

	// Make sure we don't try to export over what the maximum columns/rows are.
	//
	c = lpSS->Col.Max + lpSS->Col.HeaderCnt - 1;
	r = lpSS->Row.Max + lpSS->Row.HeaderCnt - 1;
	if (lpSS->fVirtualMode)
	{	if (r > 500)
		{	r = 500;
		}
	}

	// If the second column passed in is greater than the maxcol, make it EQ to 
	// maxcol and write a message in the log file.
	//
	if (c < *Col2)
	{	*Col2 = c;
		LoadString((HINSTANCE)fpInstance, IDS_LOGS_EXCEEDEDMAXCOL, tcFormat, BUFSIZ*10-1);
		_stprintf (tcBuf, tcFormat, *Col1, *Row1, *Col2, c, *Col2, c);
	}

	// If the second row passed in is greater than the maxrow, make it EQ to 
	// maxrow and write a message in the log file.
	//
	if (r < *Row2)
	{	*Row2 = r;
		LoadString((HINSTANCE)fpInstance, IDS_LOGS_EXCEEDEDMAXROW, tcFormat, BUFSIZ*10-1);
		_stprintf (tcBuf, tcFormat, *Col1, *Row1, *Row2, r, *Row2, r);
	}

	if (tcBuf[0] != 0)
		writemsg (lpHtml);

	return bRet;
}

// jb - 18.sep.98 ---------------------------------------------------------
//
// Normalize the column and row coordinates.  Could be something like -1, -1 for all, so
// we need to get the correct starting and ending point, especially for range.
//
BOOL init_colandrowcoordinates (LPSPREADSHEET lpSS, SS_COORD *lcs, SS_COORD *lrs, SS_COORD *lce, SS_COORD *lre, LPHTML lpHtml, LPCTSTR lpszName)
{
	BOOL bRet = TRUE;
	TCHAR tcBuf[BUFSIZ*10];

	if (!columnrowcoordinates (lpSS, lcs, lrs, lce, lre, lpHtml))
	{	LoadString((HINSTANCE)fpInstance, IDS_LOGS_FAILEDEXPORT, tcBuf, BUFSIZ*10-1);
		_stprintf (lpHtml->tcBuf, tcBuf, lpszName);
		writemsg (lpHtml);
	 	logfilefooter (lpHtml);

		bRet = FALSE;
	}

	return bRet;
}

// jb - 29.oct.98 ---------------------------------------------------------
//
// Get the font size: 12pt, 14pt, etc. and normalize it to our HTML font size. 
//
static TCHAR *getfontsize (HWND hWnd, LONG lPoint, TCHAR *tcFontSize)
{
	INT iFontSize;
	float test;
	TCHAR buf[10];

// PIXELS_TO_PT(Pels) (float)((double)(Pels) * 72.0 / (double)dyPixelsPerInch)

	test = PIXELS_TO_PT(lPoint);
// fix for 8973 -scl
//	iFontSize = abs((INT)PIXELS_TO_PT(lPoint));
//	iFontSize++;
  iFontSize = MulDiv(abs(lPoint), 72, dyPixelsPerInch);

	if (iFontSize < 10)
		iFontSize = 1;
	else if (iFontSize >= 10 && iFontSize < 12)
		iFontSize = 2;
	else if (iFontSize >= 12 && iFontSize < 14)
		iFontSize = 3;
	else if (iFontSize >= 14 && iFontSize < 18)
		iFontSize = 4;
	else if (iFontSize >= 18 && iFontSize < 24)
		iFontSize = 5;
	else if (iFontSize >= 24 && iFontSize < 36)
		iFontSize = 6;
	else if (iFontSize >= 36)
		iFontSize = 7;

	_tcscpy (tcFontSize, _T("SIZE="));
	_tcsncat (tcFontSize, _itot(iFontSize, buf, 10), 3);

	return tcFontSize;
}

// jb - 23.jul.98 ---------------------------------------------------------
//
// Query the spread for the font name like Comic Sans Serrif
// attribute: <FONT FACE = "Comic Sans Serrif" ...
//
static LOGFONT *gethtmlfont (LPSPREADSHEET lpSS, SS_COORD lCol, SS_COORD lRow, LOGFONT *pLogFont)
{
	HFONT   hFont;
	HGDIOBJ sysfont;

	hFont = SS_GetFontHandle (lpSS, lCol, lRow);

	if (!hFont)
	{	sysfont = GetStockObject (SYSTEM_FONT);
		GetObject (sysfont, sizeof (LOGFONT), pLogFont);
	}
	else
	{	GetObject (hFont, sizeof (LOGFONT), pLogFont);
	}

	return pLogFont;
}

// jb - 01.jul.99 ---------------------------------------------------------
//
// Don't span if a header, the next cell has data, next to last column,
// celloverflow turned off, or a picture cell.  If we can span, then make
// sure this is a static or edit cell.
// As usual, we will try to use tcBuf, 512 bytes unless there is a huge string larger
// than 512.  This shoud reduce 
//
static BOOL nospan (LPSPREADSHEET lpSS, LPHTML lpHtml, SS_COORD lCol, SS_COORD lRow, TCHAR *tcTag)
{
	BOOL bNoSpan = FALSE;

	// If this is a row or column header or we are at the last column cell then don't span.
	//
	if (_tcsstr (tcTag, _T("<TH")) || lCol+1 > lpHtml->lColEnd)
	{	bNoSpan = TRUE;
	}

	if (SS_GetDataEx (lpSS, lCol+1, lRow, NULL, -1))
	{	bNoSpan = TRUE;
	}

	if (bNoSpan)
	{ 	return bNoSpan;
	}

	// If not celloverflow, don't span.  If we are going to span, make sure to only
	// span edit and statictext celltypes.
	//
	if (!lpHtml->bAllowCellOverFlow)
	{	bNoSpan = TRUE;
	}
	else
	{	if (!bNoSpan)
		{	if (!gettypeoftype (lpSS, lCol+1, lRow, SS_TYPE_EDIT) &&
				!gettypeoftype (lpSS, lCol+1, lRow, SS_TYPE_STATICTEXT))
		  		bNoSpan = TRUE;
		}
	}

	// Pictures will cause 'no data' to be returned from SSGetData.  Don't span over
	// a picture.
	//
	if (gettypeoftype (lpSS, lCol, lRow, SS_TYPE_PICTURE)) bNoSpan = TRUE;
	
	return bNoSpan;
}

// jb - 28.jun.99 ---------------------------------------------------------
//
// Get the width of of a string.
//
static LONG gettextwidthinpixels (LPSPREADSHEET lpSS, SS_COORD lCol, SS_COORD lRow, LPTSTR lptstrBuf, LONG *lTextWidth)
{
    RECT rect = {0, 0, 0, 0};
	HDC hdc;
  LOGFONT LogFont;
  HFONT hfont, oldfont;

	// jb - 16.aug.99
	// For windows 2000, DrawText will not fill &rect if lptstrBuf is NULL.
	//
	if (*lptstrBuf == 0)
		return 2L;

	hdc = GetDC (lpSS->lpBook->hWnd);

// fix for bug 9213 -scl
  gethtmlfont (lpSS, lCol, lRow, &LogFont);
  hfont = CreateFontIndirect(&LogFont);
  oldfont = (HFONT)SelectObject(hdc, hfont);
	DrawText (hdc, lptstrBuf, -1, &rect, DT_CALCRECT);
  SelectObject(hdc, oldfont);
  DeleteObject(hfont);
//	*lTextWidth = rect.right - rect.left + 2;
	*lTextWidth = rect.right - rect.left + 4;
	ReleaseDC (lpSS->lpBook->hWnd, hdc);

	return *lTextWidth;
}

// jb - 05.nov.98 ---------------------------------------------------------
//
// ALLOWCELLOVERFLOW has to be set to true which sets NOWRAP for spanning.
//
// 1. read the next cell, if there is NO data then
// 2. add cell width to the width we have IF there is data in it.  
// 3. add one to the SPAN count.  A count greater than 1 means use SPAN=n
// 4. Set our span flag in the html structure.
// 5. make sure we don't read past our MAX col.
// 6. If it is column 1 and there is no data across, make sure the next background color is not
//    different.  Don't want to span a bgcolor of cell one all the way down.
//
static void columnspanning (LPSPREADSHEET lpSS, SS_COORD lCol, SS_COORD lRow, TCHAR *tcTag, LPHTML lpHtml)
{
	INT  iColSpan, iColWidth, iMoreWidth, i;
	LONG lTextWidth;
	BOOL bNoSpan;
	UINT uiCellDataLen;
	LPTSTR lptstrBuf = NULL;
	GLOBALHANDLE ghBuf = (GLOBALHANDLE)0;
	TCHAR tcBuf[BUFSIZ*10];

	BOOL bBiggerThanBufSize = FALSE;
	BOOL bNextCell = FALSE;
#ifdef SS_V40
  SS_COORD lNumCols;
  SS_COORD lNumRows;
  BOOL bNewSpan = FALSE;
  SS_COORD l;
#endif

	iColSpan = 1;
	iColWidth = iMoreWidth = 0;

	// We will always default to 512 for our buffer.  In the event that we need more space, 
	// we will allocate more, then go back to 512 if we can.
	//
	uiCellDataLen = (SS_GetDataEx (lpSS, lCol, lRow, NULL, -1) + 1) * sizeof (TCHAR);
	if (uiCellDataLen > BUFSIZ*10)
	{ 	ghBuf     = GlobalAlloc (GHND, uiCellDataLen);
 		lptstrBuf = (LPTSTR)GlobalLock (ghBuf);
		bBiggerThanBufSize = TRUE;
	}

	// Span only static and edit cells.  Don't span headers.
	//
#ifdef SS_V40
	// check for col span on this cell
	if( SS_SPAN_ANCHOR == SS_GetCellSpan(lpSS, lCol, lRow, NULL, NULL, &lNumCols, &lNumRows) )
	{
/*
    long c, r;
    long c1 = lCol - lpHtml->lColStart;
    long r1 = lRow - lpHtml->lRowStart;
    long c2 = c1 + lNumCols;
    long r2 = r1 + lNumRows;
    long numcols = lpHtml->lColEnd - lpHtml->lColStart + 1;

    if( lpHtml->fDisplayRowHeaders )
    {
      numcols = lpHtml->lColEnd - lpHtml->lColStart + 1 + lpSS->Col.HeaderCnt;
      if( lCol < lpSS->Col.HeaderCnt )
        c1 = lCol;
      else
        c1 = lCol - lpHtml->lColStart + lpSS->Col.HeaderCnt;
      if( lpHtml->fDisplayColHeaders )
      {
        if( lRow < lpSS->Row.HeaderCnt )
          r1 = lRow;
        else
          r1 = lRow - lpHtml->lRowStart + lpSS->Row.HeaderCnt;
      }
      else
        r1 = lRow - lpHtml->lRowStart;
    }
    else
    {
      numcols = lpHtml->lColEnd - lpHtml->lColStart + 1;
      c1 = lCol - lpHtml->lColStart;
      if( lpHtml->fDisplayColHeaders )
      {
        if( lRow < lpSS->Row.HeaderCnt )
          r1 = lRow;
        else
          r1 = lRow - lpHtml->lRowStart + lpSS->Row.HeaderCnt;
      }
      else
        r1 = lRow - lpHtml->lRowStart;
    }
    c2 = c1 + lNumCols;
    r2 = r1 + lNumRows;
    for( c = c1; c < c2; c++ )
      for( r = r1; r < r2; r++ )
        lpHtml->pbspanning[r * numcols + c] = TRUE;
*/
		lpHtml->iSpannedColumn = (int)(lCol + lNumCols - 1);
    bNewSpan = bNoSpan = TRUE;
    // check for spans that go past last row or column being exported
    if( lCol + lNumCols - 1 > lpHtml->lColEnd )
      lNumCols -= lpHtml->lColEnd - lCol + 1;
    if( lRow + lNumRows - 1 > lpHtml->lRowEnd )
      lNumRows -= lpHtml->lRowEnd - lRow + 1;
	}
  else
#endif
	bNoSpan = nospan (lpSS, lpHtml, lCol, lRow, tcTag);
	iColWidth = SS_GetColWidthInPixels (lpSS, lCol);

	if (!bNoSpan)
	{	if (bBiggerThanBufSize)
		{	SS_GetDataEx (lpSS, lCol, lRow, lptstrBuf, -1);
// fix for bug 9213 -scl
//			gettextwidthinpixels (lpSS->lpBook->hWnd, lptstrBuf, &lTextWidth);
			gettextwidthinpixels (lpSS, lCol, lRow, lptstrBuf, &lTextWidth);
		}
		else
		{	SS_GetDataEx (lpSS, lCol, lRow, tcBuf, -1);
// fix for bug 9213 -scl
//			gettextwidthinpixels (lpSS->lpBook->hWnd, tcBuf, &lTextWidth);
			gettextwidthinpixels (lpSS, lCol, lRow, tcBuf, &lTextWidth);
		}

		// We found NO data in the next column cell.  We will now start to span.
		//
		for (i = lCol; i <= lpHtml->lColEnd; i++)
		{	uiCellDataLen = (SS_GetDataEx (lpSS, i+1, lRow, NULL, -1) + 1) * sizeof (TCHAR);
			if (uiCellDataLen > BUFSIZ*10)
			{ 	GlobalUnlock (ghBuf);
				ghBuf     = GlobalReAlloc (ghBuf, uiCellDataLen, GHND);
 				lptstrBuf = (LPTSTR)GlobalLock (ghBuf);
				bBiggerThanBufSize = TRUE;
			}

			// JIMS8988 - jb - 26.may.01
			// Make sure if the next cell over has a different color, and we are spanning, then 
			// stop the span.
			//
			// If no data in next cell, and not a picture in next cell, and the color is the same,
			// then span.
			//
			if (!gettypeoftype (lpSS,   i+1, lRow, SS_TYPE_PICTURE) && uiCellDataLen == 1)
			{	bNextCell = TRUE;
			}
			else
			{	bNextCell = FALSE;
			}

			if (bNextCell)
			{	iMoreWidth = SS_GetColWidthInPixels (lpSS, i);
				
				// If we have enough width, we may want to keep spanning IF the next cell
				// over is blank and the same color as the current cell.
				//
			 	if (iColWidth > lTextWidth)
				{	if (!bgcolorthesame (lpSS, i, lRow) ||
						SS_GetDataEx (lpSS, i+1, lRow, lptstrBuf, -1))
					{	break;
					}
// fix for bug 9213 -scl
//					else
//				 	{	iColSpan++;
//						lpHtml->iSpannedColumn = i+1;
//						iColWidth += iMoreWidth;
//					}
			 	}
			 	else
			 	{ 	iColSpan++;
				  	lpHtml->iSpannedColumn = i+1;
				  	iColWidth += iMoreWidth;
					
					// We spanned on column and added the width's together.  See if this is enough.
					// otherwise, keep spanning.
					//
					if (iColWidth > lTextWidth)
					{	if (!bgcolorthesame (lpSS, i, lRow) ||
							SS_GetDataEx (lpSS, i+1, lRow, lptstrBuf, -1))
						{	break;
						}
					}
				}
			}
			else
			{	break;
			}
		}
	}

	// JIM8967 - jb - 26.may.01
	// Don't use the width when spanning columns.  We are already getting the width by using
	// empty cells that are next to the cell containing data.  Once we have enough width to
	// contain the data, then we stop spanning.  Taking the width out makes the columns more
	// in line with the width seen on the spread.
	//
	_tcscat (tcTag, _T(" WIDTH="));
#ifdef SS_V40
  if( bNewSpan )
  {
    for( l = lCol + 1; l < lCol + lNumCols; l++ )
      iColWidth += SS_GetColWidthInPixels (lpSS, l);
  }
#endif
	_tcscat (tcTag, _itot (iColWidth, lpHtml->tcBuf, 10));

#ifdef SS_V40
  if( bNewSpan )
  {
    if( lNumCols > 1 )
    {
		  _tcscat (tcTag, _T(" COLSPAN="));
		  _tcscat (tcTag, _ltot (lNumCols, lpHtml->tcBuf, 10));
    }
    if( lNumRows > 1 )
    {
		  _tcscat (tcTag, _T(" ROWSPAN="));
		  _tcscat (tcTag, _ltot (lNumRows, lpHtml->tcBuf, 10));
    }
  }
  else
#endif
	if (!bNoSpan && iColSpan > 1)
	{	_tcscat (tcTag, _T(" COLSPAN="));
		_tcscat (tcTag, _itot (iColSpan, lpHtml->tcBuf, 10));
	}
	_tcscat (tcTag, _T(">"));

	if (bBiggerThanBufSize)
	{	GlobalUnlock (ghBuf);
		GlobalFree (ghBuf);
	}

	return;
}

// jb - 20.nov.98 ---------------------------------------------------------
//
// If celloverflow is turned on, the put NOWRAP in the TABLE tag.  Otherwise, leave it
// out.  
// Note: When nowrap is present, and the width is less than the width of the information
// to put in the cell, the browser ignores NOWRAP and wraps.
//
static void getcelloverflow (TCHAR *tcBuf, LPHTML lpHtml)
{
	if (lpHtml->bAllowCellOverFlow)
	{	_tcscat (tcBuf, _T(" NOWRAP"));
	}
	return;
}

// jb - 10.aug.99 ---------------------------------------------------------
//
// The picture alignment works a little differently than the statictext alignment
// for HTML and for spread.  
//
static long getpicturestyle (LPSPREADSHEET lpSS, SS_COORD lCol, SS_COORD lRow)
{
	long lAlignment = 0;
	SS_CELLTYPE cellType;

	if (SS_RetrieveCellType (lpSS, &cellType, NULL, lCol, lRow))
	{	lAlignment = cellType.Style;
	}				  

	return lAlignment;
}

// jb - 10.aug.99 ---------------------------------------------------------
//
// Alignment for pictures.  Code taken from spread designer (ctypedlg.cpp)
// JIS4536
// KEM36
//
static void getpicturealignment (LPSPREADSHEET lpSS, SS_COORD lCol, SS_COORD lRow, TCHAR *tcAttribute)
{
	SS_CELLTYPE ct;
	WORD	wHoriz, wVert;

	SS_GetCellType (lpSS, lCol, lRow, &ct);
	wHoriz = HIWORD (ct.Style);
	wVert  = wHoriz;

    wHoriz  = ((wHoriz & 0x0070) >> 5);
    wVert   = ((wVert  & 0x0007) >> 1);

	// Do Horizontal alignment for the picture cells.
	//
	switch (wHoriz)
	{
		case 0:
			_tcscat (tcAttribute, _T(" ALIGN=LEFT"));
			break;

		case 1:
			_tcscat (tcAttribute, _T(" ALIGN=CENTER"));
			break;

		case 2:
			_tcscat (tcAttribute, _T(" ALIGN=RIGHT"));
			break;

		default:
			break;
	}

	// Do verticle alignment for the picture cells.
	//
	switch (wVert)
	{
		case 0:
			_tcscat (tcAttribute, _T(" VALIGN=TOP"));
			break;

		case 1:
			_tcscat (tcAttribute, _T(" VALIGN=CENTER"));
			break;

		case 2:
			_tcscat (tcAttribute, _T(" VALIGN=BOTTOM"));
			break;

		default:
			break;
	}
	return;
}

// jb - 20.nov.98 ---------------------------------------------------------
//
// VALIGN - gets the vertical alignment.
//
static void getvalignment (LPSPREADSHEET lpSS, SS_COORD lCol, SS_COORD lRow, TCHAR *tcAttribute)
{
	// VALIGN - vertical
	//
	if (gettypeof (lpSS, lCol, lRow, SS_TYPE_STATICTEXT, SS_TEXT_TOP))
		_tcscat (tcAttribute, _T(" VALIGN=TOP"));

	else if (gettypeof (lpSS, lCol, lRow, SS_TYPE_STATICTEXT, SS_TEXT_VCENTER))
		_tcscat (tcAttribute, _T(" VALIGN=CENTER"));

	else if (gettypeof (lpSS, lCol, lRow, SS_TYPE_STATICTEXT, SS_TEXT_BOTTOM))
		_tcscat (tcAttribute, _T(" VALIGN=BOTTOM"));

	else if (gettypeofstyle (lpSS, lCol, lRow, SSS_ALIGN_VCENTER))
		_tcscat (tcAttribute, _T(" VALIGN=CENTER"));

	else if (gettypeofstyle (lpSS, lCol, lRow, SSS_ALIGN_BOTTOM))
		_tcscat (tcAttribute, _T(" VALIGN=BOTTOM"));

	else
		_tcscat (tcAttribute, _T(" VALIGN=TOP"));
	

	return;
}

// jb - 20.nov.98 ---------------------------------------------------------
//
// Set the horizontal and vertical alignment.
// ALIGN=LEFT, CENTER, RIGHT.
// Notice the gettypeofstyle function near the end.
// This is for everything that is NOT static text, like integer, float, etc.  It uses
// some different stuff like ES_CENTER instead of SS_TEXT_CENTER.
// bool flag of 0 is left alignment, 1 is center, 2 is right.
//
static void gethalignment (LPSPREADSHEET lpSS, SS_COORD lCol, SS_COORD lRow, TCHAR *tcAttribute, LPHTML lpHtml)
{
	lpHtml->iAlignment = 0;

	// ALIGN - horizontal
	//
	if (gettypeof (lpSS, lCol, lRow, SS_TYPE_STATICTEXT, SS_TEXT_LEFT))
		_tcscat (tcAttribute, _T(" ALIGN=LEFT"));

	else if (gettypeof (lpSS, lCol, lRow, SS_TYPE_STATICTEXT, SS_TEXT_CENTER))
		_tcscat (tcAttribute, _T(" ALIGN=CENTER"));

	else if (gettypeof (lpSS, lCol, lRow, SS_TYPE_STATICTEXT, SS_TEXT_RIGHT))
		_tcscat (tcAttribute, _T(" ALIGN=RIGHT"));

	else if (gettypeofstyle (lpSS, lCol, lRow, ES_CENTER))
		_tcscat (tcAttribute, _T(" ALIGN=CENTER"));

	else if (gettypeofstyle (lpSS, lCol, lRow, ES_RIGHT))
		_tcscat (tcAttribute, _T(" ALIGN=RIGHT"));
	else _tcscat (tcAttribute, _T(" ALIGN=LEFT"));

	if (_tcsstr (tcAttribute, _T("CENTER")))
		lpHtml->iAlignment = 1;

	else if (_tcsstr (tcAttribute, _T("RIGHT")))
		lpHtml->iAlignment = 2;

	return;
}

// jb - 20.nov.98 ---------------------------------------------------------
//
// Get the height of the cell in pixels.
//
static void getheight (LPSPREADSHEET lpSS, SS_COORD lCol, SS_COORD lRow, LPTSTR lptstrAttribute, LPHTML lpHtml)
{
	TCHAR tcBuf[17];
	INT iRowHeight = SS_GetRowHeightInPixels (lpSS, lRow);
#ifdef SS_V40
  SS_COORD lNumRows;
  SS_COORD l;
#endif

	_tcscat (lptstrAttribute, (LPTSTR)_T(" HEIGHT="));
#ifdef SS_V40
  // check for row span for this cell
  if( SS_SPAN_ANCHOR == SS_GetCellSpan(lpSS, lCol, lRow, NULL, NULL, NULL, &lNumRows) 
      && lNumRows > 1 )
  {
    if( lRow + lNumRows - 1 > lpHtml->lRowEnd )
      lNumRows -= lpHtml->lRowEnd - lRow + 1;
    for( l = lRow + 1; l < lRow + lNumRows; l++ )
      iRowHeight += SS_GetRowHeightInPixels (lpSS, l);
  }
/*  
while( IsEntireRowSpanned(lpSS, ++lRow, lpHtml) )
  {
    iRowHeight += SS_GetRowHeightInPixels (lpSS, lRow);
  }
*/
#endif
	_tcscat (lptstrAttribute, (LPTSTR)_itot (iRowHeight, tcBuf, 10));

	return;
}

// jb - 20.nov.98 --------------------------------------------------------------------------------
//
// Get background color.  For static text, we get the color differently.  The defaults are
// grey headers, white background for cells with black text.  This is the default for HTML
// as well so when encountered, I don't write it.
//
static void getbgcolor (LPSPREADSHEET lpSS, SS_COORD lCol, SS_COORD lRow, TCHAR *tcAttribute, LPHTML lpHtml)
{
	// We need this flag because header's are static text, different way to
	// get colors.
	//
	if (_tcsstr (tcAttribute, _T("<TH")))
		lpHtml->bHeader = TRUE;
	else
		lpHtml->bHeader = FALSE;

	_tcscpy (lpHtml->tcBuf, gethtmlcolor (lpSS, lCol, lRow, 1, lpHtml));
	if (lpHtml->bHeader)
	{	_tcscat (tcAttribute, _T(" BGCOLOR=\"#"));
		_tcscat (tcAttribute, lpHtml->tcBuf);
		_tcscat (tcAttribute, _T("\""));
	}
	else
	{	if (_tcscmp (_T("FFFFFF"), lpHtml->tcBuf))
		{	_tcscat (tcAttribute, _T(" BGCOLOR=\"#"));
			_tcscat (tcAttribute, lpHtml->tcBuf);
			_tcscat (tcAttribute, _T("\""));
		}
	}

	return;
}

// jb - 20.nov.98 ------------------------------------------------------------------
//
// Get the font text color.  If it is 0 then don't add it since that is the default for HTML.
// Note: Have a temporary hack for the header font color.  Will be fixing this when time permits.
// If it is a header, it uses a different api and returns blue, 000099 which is incorrect.
//
static void getfontcolor (LPSPREADSHEET lpSS, SS_COORD lCol, SS_COORD lRow, TCHAR *tcAttribute, LPHTML lpHtml)
{
	_tcscpy (lpHtml->tcBuf, gethtmlcolor (lpSS, lCol, lRow, 0, lpHtml));
	if (lpHtml->bHeader)
	{	_tcscat (tcAttribute, _T(" COLOR=\"#"));
		_tcscat (tcAttribute, lpHtml->tcBuf);
		_tcscat (tcAttribute, _T("\""));
	}
	else
	{	if (_tcscmp (_T("000000"), lpHtml->tcBuf))
		{	_tcscat (tcAttribute, _T(" COLOR=\"#"));
			_tcscat (tcAttribute, lpHtml->tcBuf);
			_tcscat (tcAttribute, _T("\""));
		}
	}

	return;
}

// jb - 20.nov.98 ------------------------------------------------------------------
//
// Get the font face, (the font name) for our text.
//
static void getfontface (TCHAR *tcAttribute, TCHAR *tcFaceName)
{
	_tcscat (tcAttribute, _T(" FACE=\""));
	_tcscat (tcAttribute, tcFaceName);
	_tcscat (tcAttribute, _T("\">"));

	return;
}

// jb - 20.nov.98 --------------------------------------------------------------
//
// Get the style of the font like strike thru, bold, underline, italics, etc.
//
static void getfontstyle (TCHAR *tcAttribute, LOGFONT *pLogFont)
{
	// Bold
	//
	if (pLogFont->lfWeight >= 700)
	{	_tcscat (tcAttribute, _T("<B>"));
	}

	// Italic
	//
	if (pLogFont->lfItalic)
	{	_tcscat (tcAttribute, _T("<I>"));
	}

	// Strike
	//
	if (pLogFont->lfStrikeOut)
	{	_tcscat (tcAttribute, _T("<S>"));
	}

	// Underline
	//
	if (pLogFont->lfUnderline)
	{	_tcscat (tcAttribute, _T("<U>"));
	}

	return;
}

// jb - 19.aug.98 ---------------------------------------------------------
//
// Get the various attributes that can be associated with the some of the main tags.
// Items like alignment, celloverflow, height, bg/fg color, font, etc.
//
static void gethtmlattributes (LPSPREADSHEET lpSS, SS_COORD lCol, SS_COORD lRow, TCHAR *tcAttribute, LPHTML lpHtml)
{
	LOGFONT *pLogFont, LogFont;
	TCHAR tcFontSize[10];

	BOOL bFont = TRUE;

	// Here, we have a case where we only want the <font size to be written if there is a blank.
	// We have to make sure it is also text, since SSGetDataLen will return 0 if it is a
	// picture/checkbox etc.
	//
  	if (!SS_GetDataEx (lpSS, lCol, lRow, NULL, -1))
	{	if (gettypeoftype (lpSS, lCol, lRow, SS_TYPE_STATICTEXT) ||
			gettypeoftype (lpSS, lCol, lRow, SS_TYPE_EDIT))

		{	bFont = FALSE;
		}
	}

	// We want the font face to always be displayed for all headers.
	//
	if (lCol < lpSS->Col.HeaderCnt || lRow < lpSS->Row.HeaderCnt)
	{	bFont = TRUE;
	}

	// NOWRAP attribute:
	//
	getcelloverflow (tcAttribute, lpHtml);

	if (gettypeoftype (lpSS, lCol, lRow, SS_TYPE_PICTURE))
	{
		// ALIGN & VALIGN attribute, pictures and checkboxes
		//
		getpicturealignment (lpSS, lCol, lRow, tcAttribute);
	}
	else
	{	// ALIGN attribute, text: - Horizontal
		//
		gethalignment (lpSS, lCol, lRow, tcAttribute, lpHtml);

		// VALIGN attribute, text: - Vertical
		//
		getvalignment (lpSS, lCol, lRow, tcAttribute);
	}

  // HEIGHT attribute:
	//
	getheight (lpSS, lCol, lRow, tcAttribute, lpHtml);

	// BACKCOLOR attribute:
	//
	getbgcolor (lpSS, lCol, lRow, tcAttribute, lpHtml);

	// COLSPAN attribute:
	//
	columnspanning (lpSS, lCol, lRow, tcAttribute, lpHtml);

	// FONT tag:
	//
	pLogFont = gethtmlfont (lpSS, lCol, lRow, &LogFont);
	_tcscat (tcAttribute, _T("<FONT "));

	// FONT SIZE attribute:
	//
  	_tcscat (tcAttribute, getfontsize (lpSS->lpBook->hWnd, pLogFont->lfHeight, tcFontSize));

	// If blank, we don't need the rest of the font information.  We do however need
	// the size.
	//
	if (bFont)
	{
		// FONT COLOR attribute:
		//
		getfontcolor (lpSS, lCol, lRow, tcAttribute, lpHtml);

		// FONT FACE attribute:
		// 
		getfontface (tcAttribute, pLogFont->lfFaceName);
	
		// Bold, Italics, Underline, Strikethru,  If this is blank, don't want to have
		// a situation where a cell has an underline <U> but blank.  This causes a
		// _ to display in HTML
		//
		getfontstyle (tcAttribute, pLogFont);
	}

	else
	{	_tcscat (tcAttribute, _T(">"));
	}

	_tcscat (tcAttribute, _T(" \t"));

	return;
}

// jb - 05.nov.98 ---------------------------------------------------------
//
// If the user just specified a file without the path, then get the current working
// directory and put the filename on.  If they forgot the extension, use .htm
//
static BOOL validatefilename (LPHTML lpHtml)
{
	BOOL bRet = TRUE;
	TCHAR tcHtmlFile[BUFSIZ*10];
	TCHAR tcBuf[BUFSIZ*10];
	TCHAR currentdir[_MAX_PATH+1];
	TCHAR tDrive[_MAX_DRIVE], 
		  tDir[_MAX_DIR], 
		  tFileName[_MAX_FNAME], 
		  tExt[_MAX_EXT];

	_tgetcwd (currentdir, _MAX_PATH);
	
	tDrive[0] = tDir[0] = tFileName[0] = tExt[0] = tcHtmlFile[0] = 0;
	*lpHtml->tcBuf = 0;
	_tsplitpath (lpHtml->tcHtmlFile, tDrive, tDir, tFileName, tExt);

	// Do we have a drive letter ?
	//
	if (tDrive[0] != 0)
	{	_tcscpy (tcHtmlFile, tDrive);
	}

	// If we don't have a directory, complain.
	//
	if (tDir[0] == 0)
	{
#ifdef SS_V40
		// If we don't have a directory, complain.
		//
		LoadString((HINSTANCE)fpInstance, IDS_LOGS_DIRECTORYNOTFOUND, tcBuf, BUFSIZ*10-1);
		_stprintf (lpHtml->tcBuf, tcBuf, lpHtml->tcHtmlFile);
		bRet = FALSE;
#else
		// If we don't have a directory, then forget the drive.  We are going to use the
		// current working directory.
		//
		_tcscpy (tcHtmlFile, currentdir);
		_tcscat (tcHtmlFile, _T("\\"));
		LoadString((HINSTANCE)fpInstance, IDS_LOGS_DIRECTORYNOTFOUND, tcBuf, BUFSIZ*10-1);
		_stprintf (lpHtml->tcBuf, tcBuf, lpHtml->tcHtmlFile, currentdir);
#endif // SS_V40
	}

	// Add our path to the drive letter if there is a drive letter, otherwise,
	// just add the path.
	//
	else
	{	if (tcHtmlFile[0] != 0)
			_tcscat (tcHtmlFile, tDir);
		else
			_tcscpy (tcHtmlFile, tDir);
	}
		
	// Get our path so we can use it for the images we may encounter later.
	//
	_tcscpy (lpHtml->tcPath, tcHtmlFile);
	
	// Lets see if we have a file name.
	//
	if (tFileName[0] == 0)
	{	LoadString((HINSTANCE)fpInstance, IDS_LOGS_INVALIDFILE, tcBuf, BUFSIZ*10-1);
		_stprintf (lpHtml->tcBuf, tcBuf, lpHtml->tcHtmlFile);
		bRet = FALSE;
	}

	// We have a filename so put it on our path.  If there is no extension, default to .htm
	//
	else
	{	_tcscat (tcHtmlFile, tFileName);
		if (tExt[0] != 0)
			_tcscat (tcHtmlFile, tExt);
		else
			_tcscat (tcHtmlFile, _T(".htm"));
	}

	// If we had an error, write the message.
	//
	if (*lpHtml->tcBuf != 0)
		writemsg (lpHtml);

	_tcscpy (lpHtml->tcHtmlFile, tcHtmlFile);

	return bRet;
}

// jb - 27.jul.98 ---------------------------------------------------------
//
// Open the HTML file passed in for writing.
//
BOOL openfile (HWND hWnd, LPHTML lpHtml, FILE **fp)
{
	TCHAR mode[4], tcBuf[BUFSIZ*10];
	long lLen;

	// Make sure the directory specified for the method exists. 
	// We give it a local name in case the user chooses a file with a longer path.
	//
	if (!validatefilename (lpHtml))
		return FALSE;

	if (lpHtml->bAppendFlag)
		_tcscpy (mode, _T("r+"));
	else
		_tcscpy (mode, _T("w"));

	// Since we are going to open it for reading and writing, the file must exist.
	//
	if (lpHtml->bAppendFlag)
	{	if (_taccess (lpHtml->tcHtmlFile, 0))
		{	LoadString((HINSTANCE)fpInstance, IDS_LOGS_FILENOTFOUND, tcBuf, BUFSIZ*10-1);
			_stprintf (lpHtml->tcBuf, tcBuf, lpHtml->tcHtmlFile);
			writemsg (lpHtml);

			return FALSE;
		}
	}

	*fp = _tfopen (lpHtml->tcHtmlFile, mode);
	if (!*fp)
	{  	LoadString((HINSTANCE)fpInstance, IDS_LOGS_CANTOPENFILE, tcBuf, BUFSIZ*10-1);
		_stprintf (lpHtml->tcBuf, tcBuf, lpHtml->tcHtmlFile);
		writemsg (lpHtml);
		logfilefooter (lpHtml);
		return FALSE;
	}

	// If we are appending, then back over the table tag.  Cannot seek back with
	// the append flag but can search for the </TABLE> tag and seek back one.
	//
	if (!_tcscmp (mode, _T("r+")))
	{	while (!feof(*fp))
		{	_fgetts(lpHtml->tcBuf, BUFSIZ*10-1, *fp);
			if (!_tcsncmp (lpHtml->tcBuf, TABLE, _tcslen (TABLE)))
			{
				//Modify By Boc 99.4.28 (hyt)
				//should use actual read size 
				//lLen = BUFSIZ*10 * sizeof(TCHAR);
				lLen = lstrlen(lpHtml->tcBuf)*sizeof(TCHAR);
				//--------------------------------
				fseek (*fp, -lLen, SEEK_CUR);
				break;
			}
		}
	}
	
	return TRUE;
}

// jb - 27.jul.98 ---------------------------------------------------------
//
// This function will get the beginning tag and ALL attributes associated for the cell.
//
static void begintag (LPSPREADSHEET lpSS, FILE **fp, SS_COORD lCol, SS_COORD lRow, TCHAR *tszStr, LPHTML lpHtml)
{
	TCHAR tcHTMLLine[BUFSIZ*10];

	// Get our tags like <TR> or <TR> newline <TH
	//
	if (_tcslen (tszStr) > 1)
	{	_tcscpy (tcHTMLLine, tszStr);

		gethtmlattributes (lpSS, lCol, lRow, tcHTMLLine, lpHtml);
		_tcsncpy (tszStr, tcHTMLLine, BUFSIZ*10-1);
	}	

	return;
}

// jb - 27.jul.98 ---------------------------------------------------------
//
// Return a string with all appropriate ending tags.
//
static BOOL endtag (SS_COORD lCol, SS_COORD lRow, TCHAR *tszWrite, LPTSTR lptstrBuf, UINT uiCellDataLen, FILE **fp)
{
	// Make sure we have a beginning and ending font tabs..but, in
	// what order ?  For good HTML, don't want <font><u> ... </font></u>
	//
	if (_tcsstr (tszWrite, _T("<B>")))
		_tcscat (lptstrBuf, (LPTSTR)_T("</B>"));		// 5

   	if (_tcsstr (tszWrite, _T("<I>")))
		_tcscat (lptstrBuf, (LPTSTR)_T("</I>"));		// 5
		
   	if (_tcsstr (tszWrite, _T("<S>")))
		_tcscat (lptstrBuf, (LPTSTR)_T("</S>"));		// 5

   	if (_tcsstr (tszWrite, _T("<U>")))
		_tcscat (lptstrBuf, (LPTSTR)_T("</U>"));		// 5

	if (_tcsstr (tszWrite, _T("<FONT")))
		_tcscat (lptstrBuf, (LPTSTR)_T("\t</FONT>"));	// 10

	// We know these will come last.
	//
	if (_tcsstr (tszWrite, _T("<TD")))
		_tcscat (lptstrBuf, (LPTSTR)_T("\t</TD>\n"));	// 10

	if (_tcsstr (tszWrite, _T("<TH")))
		_tcscat (lptstrBuf, (LPTSTR)_T("\t\t</TH>\n"));	// 12


	_fputts (lptstrBuf, *fp);							// 52
	*lptstrBuf = 0;

	return TRUE;
}

// jb - 27.oct.98 ---------------------------------------------------------
//
// We want the data in the column header.  The user may have no text
// in the header, numbers, or letters.  Also, they may have altered the 
// StartingColumnNumber.  For a blank header, we put &nbsp; 
// 
extern "C" 
{

BOOL getheaderdata (LPSPREADSHEET lpSS, SS_COORD coord, BOOL bRowHeader, LPTSTR lptstrBuf)
{
	WORD wDisplay;
	TCHAR tcLocalBuf[16];
	SS_COORD StartNum;
  SS_COORD headercnt;
  
  if( bRowHeader )
  {
    StartNum = lpSS->Row.NumStart;
    wDisplay = lpSS->RowHeaderDisplay;
    headercnt = lpSS->Row.HeaderCnt;
  }
  else
  {
    StartNum = lpSS->Col.NumStart;
	  wDisplay = lpSS->ColHeaderDisplay;
    headercnt = lpSS->Col.HeaderCnt;
  }

	coord += (StartNum - headercnt);

	if (wDisplay == SS_HEADERDISPLAY_BLANK)
		_tcscpy (lptstrBuf, (LPTSTR)_T("&nbsp;"));

	else if (wDisplay == SS_HEADERDISPLAY_NUMBERS)
	 	_tcscpy (lptstrBuf, (LPTSTR)_itot(coord, tcLocalBuf, 10));

	else if (wDisplay == SS_HEADERDISPLAY_LETTERS)
		SS_DrawFormatHeaderLetter (lptstrBuf, coord);

	else
		*lptstrBuf = 0;

	return TRUE;

}

} // extern "C"

// jb - 27.oct.98 ---------------------------------------------------------
//
// Had a problem when the password field was being exported, it was was showing 
// up in the html table!  Turn it back into '*****'
//
static BOOL dontshowpassword (LPSPREADSHEET lpSS, SS_COORD lCol, SS_COORD lRow, LPTSTR lptstrBuf)
{
	INT len, i;

	// Don't let the password get exported !  Change it to '*' when exporting.
	//
	if (gettypeof (lpSS, lCol, lRow, SS_TYPE_EDIT, ES_PASSWORD))
	{	len = (int)_tcslen (lptstrBuf);

		for (i = 0; i < len; i++)
		{	*lptstrBuf++ = _T('*');
		}

		*lptstrBuf = 0;
	}

	return TRUE;
}

// jb - 26.jul.99 ----------------------------------------------------------------------
//
// Set the coordinates
//
static void setcoords (LPHTML lpHtml, SS_COORD *c, SS_COORD *r)
{
	if (!lpHtml->lRowStart)
	{	*r = 1;
	}
	else
	{	*r = lpHtml->lRowStart;
	}

	if (!lpHtml->lColStart)
	{	*c = 1;
	}
	else
	{	*c = lpHtml->lColStart;
	}

	return;
}

// jb - 27.jul.99 ----------------------------------------------------------------------
//
// Get the name of the jpeg we stored by using the particular column, row we
// were on.
//
static BOOL getjpegname (LPHTML lpHtml, SS_COORD lCol, SS_COORD lRow)
{
	BOOL bRet = FALSE;
	int i;

	for (i = 0; i < lpHtml->jpegindex; i++)
	{	if ((lCol == lpHtml->jpeg[i].col) && (lRow == lpHtml->jpeg[i].row))
		{	_tcscpy (lpHtml->tcJpegName, lpHtml->jpeg[i].tcJpegName);
			bRet = TRUE;
			break;
		}
	}

	return bRet;
}

// jb - 27.jul.99 ----------------------------------------------------------------------
//
// See if there is another bitmap in the spread that is the same as this one.  If so,
// return the name of the jpeg.  This makes it so we won't have duplicate jpegs for
// the same picture.  We can have up to 500 pictures.  
// NOTE: make a pointer to the jpeg structure, and take the structure out of the html
//       structure.
//
static BOOL bitmapthesame (LPSPREADSHEET lpSS, SS_COORD x, SS_COORD y, SS_COORD lCol, SS_COORD lRow, LPHTML lpHtml)
{
	BOOL bRet = TRUE;
	GLOBALHANDLE ghDib2Find,    ghDib;
	GLOBALHANDLE ghBitmap2Find, ghBitmap;
	LPTSTR       lpBitmap2Find, lpBitmap;
	HBITMAP       hBitmap2Find,  hBitmap;
	LONG          lBitmap2Find,  lBitmap, i = 0;
	BOOL bDelete, bDelete2Find;

	// Get the bitmap, starting  at the first picture in the spreadsheet.
	//
	hBitmap      = SS_PictGetBitmap (lpSS, x, y, &bDelete);
	hBitmap2Find = SS_PictGetBitmap (lpSS, lCol, lRow, &bDelete2Find);

	if (ghDib2Find = BitmapToDIB (hBitmap2Find, 0))
	{	if (ghBitmap2Find = SaveDIBToBuffer (ghDib2Find, &lBitmap2Find))
		{	lpBitmap2Find = (LPTSTR)GlobalLock (ghBitmap2Find);
		}
		else
		{	GlobalFree (ghDib2Find);
			if (bDelete && hBitmap)
				DeleteObject (hBitmap);

			if (bDelete2Find && hBitmap2Find)
				DeleteObject (hBitmap2Find);

			return FALSE;
		}
	}
	else
	{ 	if (bDelete && hBitmap)
	  		DeleteObject (hBitmap);

		if (bDelete2Find && hBitmap2Find)
			DeleteObject (hBitmap2Find);

		return FALSE;
	}

	if (ghDib = BitmapToDIB (hBitmap, 0))
	{	if (ghBitmap = SaveDIBToBuffer (ghDib, &lBitmap))
		{	lpBitmap = (LPTSTR)GlobalLock (ghBitmap);
		}
		else
		{	GlobalFree (ghDib2Find);
			GlobalUnlock (ghBitmap2Find);
			GlobalFree (ghBitmap2Find);
			if (bDelete && hBitmap)
				DeleteObject (hBitmap);

			if (bDelete2Find && hBitmap2Find)
				DeleteObject (hBitmap2Find);

			return FALSE;
		}
	}

	else
	{	GlobalFree (ghDib2Find);
		GlobalUnlock (ghBitmap2Find);
		GlobalFree (ghBitmap2Find);
		if (bDelete && hBitmap)
			DeleteObject (hBitmap);

		if (bDelete2Find && hBitmap2Find)
			DeleteObject (hBitmap2Find);

		return FALSE;
	}

	// See if we have the same bitmap.
	//
	while (i < lBitmap2Find)
	{	if (*lpBitmap2Find++ == *lpBitmap++) 
		{	i++;
			continue;
		}
		else
		{	bRet = FALSE;
			break;
		}
	}

	// If bRet never was set to true, then the bitmap was the same.  Go get the name of 
	// the jpeg.
	//
	if (bRet)
	{	bRet = getjpegname (lpHtml, x, y);
	}

	GlobalUnlock (ghBitmap2Find);
	GlobalFree   (ghBitmap2Find);

	GlobalUnlock (ghBitmap);
	GlobalFree   (ghBitmap);

	GlobalFree (ghDib);
	GlobalFree (ghDib2Find);

	if (bDelete && hBitmap)
		DeleteObject (hBitmap);

	if (bDelete2Find && hBitmap2Find)
		DeleteObject (hBitmap2Find);

	return bRet;
}

// jb - 28.jul.99 --------------------------------------------------------------------------------
//
// Need to allocate some space for our jpeg filename.
// 
static BOOL addnewjpeg (LPHTML lpHtml, SS_COORD lCol, SS_COORD lRow)
{
	BOOL bRet = TRUE;
	TCHAR tcBuf[BUFSIZ*10];

	if (lpHtml->jpegindex > BUFSIZ*10) return FALSE;

	_tcscpy (tcBuf, randomname (tcBuf, lpHtml->tcPath, _T("jpg")));

	lpHtml->jpeg[lpHtml->jpegindex].ghBuf = (GLOBALHANDLE)GlobalAlloc (GHND, _tcslen (tcBuf) + 1);
	lpHtml->jpeg[lpHtml->jpegindex].tcJpegName = (LPTSTR)GlobalLock (lpHtml->jpeg[lpHtml->jpegindex].ghBuf);

	if (lpHtml->jpeg[lpHtml->jpegindex].tcJpegName)
	{	_tcscpy (lpHtml->jpeg[lpHtml->jpegindex].tcJpegName, tcBuf);
		_tcscpy (lpHtml->tcJpegName, tcBuf);
		lpHtml->jpeg[lpHtml->jpegindex].col = lCol;
		lpHtml->jpeg[lpHtml->jpegindex].row = lRow;

		GlobalUnlock (lpHtml->jpeg[lpHtml->jpegindex].tcJpegName);
		lpHtml->jpegindex++;
	}
	else
	{	bRet = FALSE;
	}

	return bRet;
}

// jb - 04.feb.99 --------------------------------------------------------------------------------
//
// See if this picture is a duplicate.  No need to make another jpeg of the same picture.
//
static BOOL issamepicture (LPSPREADSHEET lpSS, SS_COORD lCol, SS_COORD lRow, LPSS_CELLTYPE lpCellType, LPHTML lpHtml)
{
	BOOL bRet = FALSE;
	SS_COORD	x, y, row, col;
	TCHAR 	tcBuf[BUFSIZ*10];

	// Get the beginning col and row.
	//
	setcoords (lpHtml, &col, &row);

	for (y = row; y <= lpHtml->lRowEnd; y++)
	{	for (x = col; x <= lpHtml->lColEnd; x++)
		{	if (gettypeoftype (lpSS, x, y, SS_TYPE_PICTURE))
			{	//
				// We want to check for duplicate bitmaps.  If duplicate, don't make another jpeg.
				// Use the one already made.
				//
				bRet = bitmapthesame (lpSS, x, y, lCol, lRow, lpHtml);
				if (bRet)
				{  	if (lpCellType->Style & VPS_BMP)
					{	LoadString((HINSTANCE)fpInstance, IDS_LOGS_BMP_PICTURE, tcBuf, BUFSIZ*10-1);
					}
					else
					{	LoadString((HINSTANCE)fpInstance, IDS_LOGS_ICON_PICTURE, tcBuf, BUFSIZ*10-1);
					}
					_stprintf (lpHtml->tcBuf, tcBuf, lCol, lRow, lpHtml->tcJpegName);
					break;
				}
			}
		}

		if (bRet)
		{	writemsg (lpHtml);
			break;
		}
	}

	return bRet;
}

// jb - 27.jan.99 --------------------------------------------------------------------------------
//
// If the type is a picture (bitmap or icon) we will convert the icon to a bitmap, and
// bitmap to a 24 bit bitmap (to feed the jpeg compressor) then compress the image into
// a jpeg given a random name of SPRrrrrr.jpg and put into the same directory as the
// .html file.
//
static BOOL ispicture (LPSPREADSHEET lpSS, SS_COORD lCol, SS_COORD lRow, LPHTML lpHtml)
{
	BOOL bRet = FALSE;
	TCHAR tcBuf[BUFSIZ*10];
	SS_CELLTYPE CellType;
	BOOL bDelete = FALSE;

	SS_GetCellType (lpSS, lCol, lRow, &CellType);
	lpHtml->lCol = lCol;
	lpHtml->lRow = lRow;

	if (CellType.Type != SS_TYPE_PICTURE)
	{	return bRet;
	}
	else
	{	
#ifndef SS_V70
    LPBITMAP2JPEG	lpbitmap2jpeg;
		LPGLOBALHANDLE	lpghBitmap2JPEGMem;
#endif
		HBITMAP			hBitmap2Find;

		// If neither a bitmap nor an icon, complain.
		// note:error message for this.
		//
		if (CellType.Style & VPS_BMP)
		{	lpHtml->iType = SS_BITMAP;
		}
		else if (CellType.Style & VPS_ICON)
		{	lpHtml->iType = SS_ICON;
		}
		else
		{ 	LoadString((HINSTANCE)fpInstance, IDS_LOGS_INVALIDFILETYPE, tcBuf, BUFSIZ*10-1);
		   	_stprintf (lpHtml->tcBuf, tcBuf, lCol, lRow);
			return bRet;
		}

		lpHtml->tcBuf[0] = 0;

		// A function Rick wrote that will scale the image as per cell size.
		//
		hBitmap2Find = SS_PictGetBitmap (lpSS, lCol, lRow, &bDelete);
		if (!hBitmap2Find)
		{	return bRet;
		}

		// Is this the first picture we have encountered ?
		//
		if (lpHtml->bPictureHits)
		{	if (issamepicture (lpSS, lCol, lRow, &CellType, lpHtml))
			{	if (bDelete && hBitmap2Find)
					DeleteObject (hBitmap2Find);

				return TRUE;
			}
		}
		else
		{	lpHtml->bPictureHits = TRUE;
		}
#ifndef SS_V70
		lpghBitmap2JPEGMem = (LPGLOBALHANDLE)GlobalAlloc (GHND, sizeof (BITMAP2JPEG));
		lpbitmap2jpeg = (LPBITMAP2JPEG)GlobalLock (lpghBitmap2JPEGMem);

		if (!lpbitmap2jpeg)
		{  	return FALSE;
		}
#endif
		// The random name will start with SPRrrrrr.jpg and reside in the same directory
		// as the .htm file.
		//
		addnewjpeg (lpHtml, lCol, lRow);
#ifndef SS_V70
		lpbitmap2jpeg->hBmp = hBitmap2Find;
		lpbitmap2jpeg->hPal = 0;

	 	// Turn into a 24-bit bitmap, then a jpeg.
		//
		if (!Bitmap4JPEG (lpbitmap2jpeg))
		{	LoadString((HINSTANCE)fpInstance, IDS_LOGS_BMP_FAILURE, tcBuf, BUFSIZ*10-1);
			_stprintf (lpHtml->tcBuf, tcBuf, lCol, lRow, lpHtml->tcJpegName);
			return FALSE;
		}
		else
		{	bRet = SaveBitmap2JPEG (lpbitmap2jpeg->hBmp, lpHtml->tcJpegName);
			if (bRet)
			{	if (lpHtml->iType == SS_BITMAP)
				{ 	LoadString((HINSTANCE)fpInstance, IDS_LOGS_BMP_PICTURE, tcBuf, BUFSIZ*10-1);
				}
				else
				{ 	LoadString((HINSTANCE)fpInstance, IDS_LOGS_ICON_PICTURE, tcBuf, BUFSIZ*10-1);
				}
				_stprintf (lpHtml->tcBuf, tcBuf, lpHtml->lCol, lpHtml->lRow, lpHtml->tcJpegName);
			}
			else
			{	LoadString((HINSTANCE)fpInstance, IDS_LOGS_JPEG_FAILURE, tcBuf, BUFSIZ*10-1);
				_stprintf (lpHtml->tcBuf, tcBuf, lpHtml->lCol, lpHtml->lRow);
			}
			writemsg (lpHtml);
			lpHtml->tcBuf[0] = 0;
		}
#else
    {
#ifdef _DEBUG
      HINSTANCE hInstDll = LoadLibrary(_T("fpimaged.dll"));
#else
      HINSTANCE hInstDll = LoadLibrary(_T("fpimage.dll"));
#endif
      FPSAVEIMAGEPROC pFPSaveImage = hInstDll ? (FPSAVEIMAGEPROC)GetProcAddress(hInstDll, "FPSaveImage") : NULL;
      char path[MAX_PATH] = {0};
#ifdef _UNICODE
      WideCharToMultiByte(CP_ACP, 0, lpHtml->tcJpegName, lstrlen(lpHtml->tcJpegName), path, MAX_PATH, NULL, NULL);
#else
      strncpy(path, lpHtml->tcJpegName, __min(strlen(lpHtml->tcJpegName), MAX_PATH));
#endif
      if( pFPSaveImage )
        bRet = pFPSaveImage(hBitmap2Find, 0, path, CXIMAGE_FORMAT_JPG);
      else
        bRet = FALSE;
      FreeLibrary(hInstDll);
    }
#endif

		// If we cannot make the jpeg, better release our resources, otherwise store
		// the bitmap handle  to reference later to eliminate duplicate jpegs.
		//
		if (!bRet)
		{	GlobalUnlock (lpHtml->jpeg[lpHtml->jpegindex-1].ghBuf);
			GlobalFree (lpHtml->jpeg[lpHtml->jpegindex-1].tcJpegName);
			lpHtml->jpegindex--;
			if (bDelete && hBitmap2Find)
				DeleteObject (hBitmap2Find);
		}
#ifndef SS_V70
		if (lpghBitmap2JPEGMem)
		{	GlobalUnlock (lpghBitmap2JPEGMem);
			GlobalFree   (lpghBitmap2JPEGMem);
			if (bDelete && hBitmap2Find)
				DeleteObject (hBitmap2Find);
		}
#endif
		if (lpHtml->tcBuf)
		{	writemsg (lpHtml);
		}
	}

	return bRet;
}

// jb - 02.nov.98 ---------------------------------------------------------
//
// This is not a column or row header.  It is a cell and it is either empty or
// we could not get any data from it because of the type of cell.  Write this
// information to the log file.
//
static BYTE whynodata (LPSPREADSHEET lpSS, SS_COORD lCol, SS_COORD lRow, LPHTML lpHtml)
{
	SS_CELLTYPE CellType;
	TCHAR tcBuf[BUFSIZ*10];
	BYTE bType;

	SS_GetCellType (lpSS, lCol, lRow, &CellType);

	bType = CellType.Type;

	// These are valid types and can be blank.
	//
	if (bType == SS_TYPE_DATE		|| bType == SS_TYPE_EDIT		|| 
		bType == SS_TYPE_FLOAT		|| bType == SS_TYPE_INTEGER		|| 
		bType == SS_TYPE_PIC		|| bType == SS_TYPE_STATICTEXT	||
		bType == SS_TYPE_TIME		|| bType == SS_TYPE_PICTURE		||
		bType == SS_TYPE_CHECKBOX
#ifdef SS_V40
		|| bType == SS_TYPE_NUMBER || bType == SS_TYPE_CURRENCY || bType == SS_TYPE_PERCENT
#endif // SS_V40
#ifdef SS_V70
	   || bType == SS_TYPE_SCIENTIFIC
#endif // SS_V70
      )

		return bType;

	if (bType == SS_TYPE_BUTTON)
	{	LoadString((HINSTANCE)fpInstance, IDS_LOGS_BUTTON, tcBuf, BUFSIZ*10-1);
		_stprintf (lpHtml->tcBuf, tcBuf,	lCol, lRow);
	}

	else if (bType == SS_TYPE_COMBOBOX)
	{	LoadString((HINSTANCE)fpInstance, IDS_LOGS_COMBOBOX, tcBuf, BUFSIZ*10-1);
		_stprintf (lpHtml->tcBuf, tcBuf, lCol, lRow);
	}
	
	else if (bType == SS_TYPE_OWNERDRAW)
	{	LoadString((HINSTANCE)fpInstance, IDS_LOGS_OWNERDRAWN, tcBuf, BUFSIZ*10-1);
		_stprintf (lpHtml->tcBuf, tcBuf,	lCol, lRow);
	}
	
	else
	{	LoadString((HINSTANCE)fpInstance, IDS_LOGS_UNKNOWN, tcBuf, BUFSIZ*10-1);
		_stprintf (lpHtml->tcBuf, tcBuf,	lCol, lRow);
	}

	writemsg (lpHtml);

	return bType;
}

// jb - 02.dec.98 ---------------------------------------------------------
//
// This will let the user enter a HREF URL in spread.  Spread will figure out that it needs to
// export the special tags of <A HREF="http://www.fpoint.com">your link</A>
//
BOOL fixurl (LPSPREADSHEET lpSS, SS_COORD lCol, SS_COORD lRow, TBGLOBALHANDLE *lpgh, LPLONG lplen)
{
	return	ssm_GetFormulaEx(lpSS, lCol, lRow, lpgh, lplen);
}

// jb - 02.dec.98 ---------------------------------------------------------
//
// See if this is a WEB formula/function.  If so, we need to insert tags into the data but
// our HTML conversion takes <, and > and translates them to &lt;, &gt; so we need to disable
// that feature with a bWebFormula flag.
// 
BOOL iswebformulacell (LPSPREADSHEET lpSS, SS_COORD lCol, SS_COORD lRow, LPTSTR lptstrBuf)
{
	BOOL bRet = FALSE;
	TCHAR tcFormula[500];
	TBGLOBALHANDLE gh = (TBGLOBALHANDLE)0;
	long           len = 0;

	if (CalcGetExpr(&lpSS->CalcInfo, lCol, lRow, 
#if SS_V80 // 24919 -scl
                   FALSE,
#endif
                   tcFormula, 500))
	{	_tcsupr (tcFormula);
		if (_tcsstr (tcFormula, _T("URL")))
		{ 	if (fixurl (lpSS, lCol, lRow, (TBGLOBALHANDLE *)&gh, &len))
			{
				LPTSTR lpstr = (LPTSTR)tbGlobalLock(gh);
				lstrcpy(lptstrBuf, lpstr);
				tbGlobalUnlock(gh);
				tbGlobalFree(gh);
			}
			bRet = TRUE;
		}
	}

	return bRet;
}

// jb - 16.nov.98 ---------------------------------------------------------
//
// We want to generate a message in the log file if we have exported the forumla's
// results (but not the formula)
//
static BOOL isformulacell (LPSPREADSHEET lpSS, SS_COORD lCol, SS_COORD lRow, LPHTML lpHtml)
{
	SS_CELLTYPE CellType;
	TCHAR tcBuf[BUFSIZ*10];
	BOOL bRet = FALSE;

	*lpHtml->tcBuf = 0;

	SS_GetCellType (lpSS, lCol, lRow, &CellType);

	if (CalcGetExpr (&lpSS->CalcInfo, lCol, lRow, 
#if SS_V80 // 24919 -scl
                    FALSE,
#endif
                    NULL, 0))
	{	LoadString((HINSTANCE)fpInstance, IDS_LOGS_FORMULA, tcBuf, BUFSIZ*10-1);
		_stprintf (lpHtml->tcBuf, tcBuf,	lCol, lRow);
	}

	if (*lpHtml->tcBuf != 0)
	{	writemsg (lpHtml);
		bRet = TRUE;
	}

	return bRet;
}

// jb - 14.apr.99 ---------------------------------------------------------
//
// Replace a character in a string with another string or character. 
//
static BOOL replacechwithsz (LPTSTR lptstrBuf, UINT uiCellDataLen, int ch, LPTSTR lptstrInsert)
{
	GLOBALHANDLE ghNewSize = (GLOBALHANDLE)0;
	GLOBALHANDLE ghPtr     = (GLOBALHANDLE)0;

	LPTSTR	lptstrPtr = NULL;
	LPTSTR	lptstrNew = NULL;
	UINT	uiNewSize =  (UINT)_tcslen (lptstrBuf)    * sizeof (TCHAR);
			uiNewSize += (UINT)_tcslen (lptstrInsert) * sizeof (TCHAR);

	if (uiNewSize > uiCellDataLen) return FALSE;

	lptstrPtr = _tcschr (lptstrBuf, ch);

	// Did we find the character we want ?
	//
	if  (!lptstrPtr)
	{	return FALSE;
	}

	// Make room for the new string.  Don't add one because we are replacing a character.
	//
	ghNewSize  = (GLOBALHANDLE)GlobalAlloc (GHND, uiNewSize);
	lptstrNew  = (LPTSTR)GlobalLock (ghNewSize);
	if (!lptstrNew) return FALSE;

	// Put the first part of the string in, up to the char to replace.
	//
	*lptstrPtr = 0;
	*lptstrPtr++;
	_stprintf (lptstrNew, _T("%s%s%s"), lptstrBuf, lptstrInsert, lptstrPtr);
	_tcscpy (lptstrBuf, lptstrNew);

	GlobalUnlock (ghNewSize);
	GlobalFree (ghNewSize);

	return TRUE;
}

// jb - 17.nov.98 ---------------------------------------------------------
//									60		62     38
// If someone has some data using   '<' or  '>' or '&' change this to &lt;  &gt; &amp
//
BOOL look4specialcharacters (LPTSTR lptstrBuf, UINT uiCellDataLen)
{
// fix for bug 9753 -scl
//	LPTSTR lptstrPtr = NULL;
  int i;
  int len = lstrlen(lptstrBuf);

  for( i = 0; i < len && i < (int)uiCellDataLen; i++ )
  {
    if( lptstrBuf[i] == (TCHAR)'<' )
    {
      replacechwithsz (&lptstrBuf[i], uiCellDataLen - i, LESSTHAN, _T("&lt;"));
      len += 4;
      i += 4;
    }
    if( i >= (int)uiCellDataLen )
      break;
    if( lptstrBuf[i] == (TCHAR)'&' )
    {
      replacechwithsz (&lptstrBuf[i], uiCellDataLen - i, AMPERSAND, _T("&amp;"));
      len += 5;
      i += 5;
    }
    if( i >= (int)uiCellDataLen )
      break;
    if( lptstrBuf[i] == (TCHAR)'>' )
    {
      replacechwithsz (&lptstrBuf[i], uiCellDataLen - i, GREATERTHAN, _T("&gt;"));
      len += 4;
      i += 4;
    }
  }
/*
	for (;;)
	{	lptstrPtr = _tcschr (lptstrBuf, LESSTHAN);
		if (!lptstrPtr)
		{	lptstrPtr = _tcschr (lptstrBuf, GREATERTHAN);

			if (!lptstrPtr)
      {
        lptstrPtr = _tcschr(lptstrBuf, AMPERSAND);
			  if (!lptstrPtr)
          break;
        else
        {
          replacechwithsz (lptstrBuf, uiCellDataLen, AMPERSAND, _T("&amp;"));
          lptstrBuf = lptstrPtr + 5;
        }
      }
//			{	break;
//			}
//			{	ptr = _tcschr (tcBuf, AMPERSAND);
//				if (!ptr)
//					break;
//				else
//				{	// We are looking to replace the '&' with the sequence &amp for HTML, but we
					// get into a endless loop since we are replacing with a & for a &amp;
					//
//					if (_tcsncmp (ptr, _T("&amp;"), 5) && _tcsncmp (ptr, _T("&nbsp;"), 6) &&
//						_tcsncmp (ptr, _T("&lt;"), 4)  && _tcsncmp (ptr, _T("&gt;"), 4))
//					{	replacechwithsz (tcBuf, AMPERSAND, _T("&amp;"));
//					}
//				}
//			}
			else
			{  	replacechwithsz (lptstrBuf, uiCellDataLen, GREATERTHAN, _T("&gt;"));
          lptstrBuf = lptstrPtr + 4;

			}
		}
		else
		{	replacechwithsz (lptstrBuf, uiCellDataLen, LESSTHAN, _T("&lt;"));
      lptstrBuf = lptstrPtr + 4;
		}
	}
*/
	return TRUE;
}

// jb - 02.dec.98 ---------------------------------------------------------
//
// If we have spaces after the text for center and right alignment, HTML ignores it.  
// Need to replace the spaces preceeding with &nbsp; 
//
// jb - 29.Jan.99 fixed SCS7897
//	If there are ending spaces, I would replace the 'reversed string' so I could easily
//	scan from the beginning, replacing spaces with &nbsp;  Then, when returning I reversed
//	the string again.  This was incorrect because &nbsp; became ;psbn&  
//
static BOOL blanks2nbspending (LPTSTR lptstrBuf, UINT uuiCellDataLen)
{
	const	TCHAR tcNoBreakingSpaces[] = _T("&nbsp;");
	LPTSTR	lptstrPtr   = NULL;
	UINT	i, uiNumBlanks = 0;
	UINT	uiNBSPLen = (UINT)_tcslen (tcNoBreakingSpaces)    * sizeof (TCHAR);
	UINT	uiBufLen  = (UINT)_tcslen (lptstrBuf) + uiNBSPLen * sizeof (TCHAR);

	// I want to scan for blanks from the beginning, so reverse string.
	//
	_tcsrev (lptstrBuf);
	lptstrPtr = lptstrBuf;

	while (*lptstrPtr == 32 && uuiCellDataLen > uiBufLen)
	{	lptstrPtr++;
		uiBufLen += uiNBSPLen;
		uiNumBlanks++;
	}

	// No blanks to worry about so return.
	//
	if (!uiNumBlanks)
	{ 	_tcsrev (lptstrBuf);
		return FALSE;
	}

	// Remove all the spaces from the end of the string.
	//
	_tcsrev (lptstrBuf);
	lptstrPtr = _tcsrchr (lptstrBuf, 32);
	while (*lptstrPtr == 32)
	{	lptstrPtr--;
	}

	lptstrPtr++;
	*lptstrPtr = 0;

	// Now, put on the 'no break spaces' for every occurance of a space we found
	// at the end of a string.
	//
	for (i = 0; i < uiNumBlanks; i++)
	{	_tcscat (lptstrBuf, tcNoBreakingSpaces);
	}

	return TRUE;
}

// jb - 07.apr.99 -------------------------------------------------------------
//
// Made dynamic allocation
// Strip all of the blanks off beginning of string and replace with HTML blanks called
// "no breaking spaces" --> &nbsp;
//
static BOOL blanks2nbsppreceeding (LPTSTR lptstrBuf, UINT uuiCellDataLen)
{
	GLOBALHANDLE	ghNew = (GLOBALHANDLE)0;
	LPTSTR			lptstrNew = NULL;
	const TCHAR tcNoBreakingSpaces[] = _T("&nbsp;");
	LPTSTR	lptstrPtr = NULL;
	UINT	i, uiNumBlanks = 0;
	UINT	uiNBSPLen = (UINT)_tcslen (tcNoBreakingSpaces)    * sizeof (TCHAR);
	UINT	uiBufLen  = (UINT)_tcslen (lptstrBuf) + uiNBSPLen * sizeof (TCHAR);

	lptstrPtr = lptstrBuf;

	// How many blanks preceed our string ?
	//
	while (*lptstrPtr == 32 && uuiCellDataLen > uiBufLen)
	{	uiNumBlanks++;
		uiBufLen += uiNBSPLen;
		lptstrPtr++;
	}

	// No blanks to worry about so return.
	//
	if (!uiNumBlanks)
	{	return FALSE;
	}

	ghNew = GlobalAlloc (GHND, uiBufLen + 1);
	if (!ghNew) return FALSE;
	lptstrNew = (LPTSTR)GlobalLock (ghNew);

	_tcscpy (lptstrNew, tcNoBreakingSpaces);

	for (i = 1; i < uiNumBlanks; i++)
	{	_tcscat (lptstrNew, tcNoBreakingSpaces);
	}

	_tcsncat (lptstrNew, lptstrPtr, uuiCellDataLen);
	_tcsncpy (lptstrBuf, lptstrNew, uuiCellDataLen);

	GlobalUnlock (ghNew);
	GlobalFree (ghNew);

	return TRUE;
}

// jb - 27.jan.99 ---------------------------------------------------------
//
// Make a picture tag using our new jpeg.  <IMG SRC="c:\test\test.jpg"> 
//
static void makepicturetag (TCHAR *tcTags, LPHTML lpHtml)
{
	TCHAR *ptr;

	// No need for a font tag if we are doing a picture.
	//
	ptr = _tcsstr (tcTags, _T("<FONT"));
	if (ptr)
	{	*ptr = 0;
	}

// fix for bug 9240 -scl
//	_tcscat (tcTags, _T("<IMG SRC=\""));
	_tcscat (tcTags, _T("<IMG SRC=\"file://"));
	_tcscat (tcTags, lpHtml->tcJpegName);
	_tcscat (tcTags, _T("\">"));

	return;
}

// jb - 20.aug.99 ---------------------------------------------------------
//
// When a SSGetData is done on a formula, you get data.  Need to intercept this first
// and check to see if it is a web formula for special formatting.
//
static BOOL check4formula (LPSPREADSHEET lpSS, LPHTML lpHtml, SS_COORD lCol, SS_COORD lRow, UINT uiCellDataLen, LPTSTR lptstrBuf)
{
	BOOL bRet = FALSE;

	if (SS_GetDataEx (lpSS, lCol, lRow, lptstrBuf, -1))
	{	if (isformulacell (lpSS, lCol, lRow, lpHtml))
		{	// If this is a web formula/function, don't replace the special characters for HTML
			// like > and <
			//
			if (!iswebformulacell (lpSS, lCol, lRow, lptstrBuf))
			{	look4specialcharacters (lptstrBuf, uiCellDataLen);
			}
			bRet = TRUE;
		}
	}
	return bRet;
}

// jb - 20.aug.99 ---------------------------------------------------------
//
// We want to change 'preceeding' blanks to &nbsp; if alignment is left or center
// and ending blanks to &nbsp; for right centered.
//
static void nbspalignment (LPHTML lpHtml, LPTSTR lptstrBuf, UINT uiCellDataLen)
{
	if (lpHtml->iAlignment < 2)
	{	blanks2nbsppreceeding (lptstrBuf, uiCellDataLen);
		
		if (lpHtml->iAlignment == 1)
		{	blanks2nbspending (lptstrBuf, uiCellDataLen);
		}
	}
	else
	{ 	blanks2nbspending (lptstrBuf, uiCellDataLen);
	}

	return;
}

// jb - 20.aug.99 ---------------------------------------------------------
//
// If there is no data in the cells, put a &nbsp; for the blank.
// 
static void nodatacells (LPSPREADSHEET lpSS, SS_COORD lCol, SS_COORD lRow, LPHTML lpHtml, TCHAR *tcTags, LPTSTR lptstrBuf)
{
	INT	iType;

//	if (lRow > 0 && lCol > 0)
	{	iType = whynodata (lpSS, lCol, lRow, lpHtml);

		// JIS4186 - 10.feb.99
		// In this case, SSGetData from a checkbox will return nothing if focus 
		// is not applied to the check box.
		//
		if (iType == SS_TYPE_CHECKBOX)
		{	if (*lptstrBuf == 0)
			{
				_tcscpy (lptstrBuf, (LPTSTR)_T("0"));
// RFW - 8/21/01				_tcscpy (lptstrBuf, (LPTSTR)_T("&nbsp;"));
			}
		}

		else if (iType == SS_TYPE_PICTURE)
		{	if (ispicture (lpSS, lCol, lRow, lpHtml))
			{	makepicturetag (tcTags, lpHtml);
			}
			else
			{	_tcscpy (lptstrBuf, (LPTSTR)_T("&nbsp;"));
			}
		}

		// Set flag for to indicate whether we found data or not.  Don't let our buffer
		// stay NULL.
		//
		else
		{	// JIS4186
			//
			if (*lptstrBuf == 0)
			{	_tcscpy (lptstrBuf, (LPTSTR)_T("&nbsp;"));
			}
		}
	}
	return;
}

// jb - 15.oct.98 ---------------------------------------------------------
//
// Get data from each cell, preceed it with appropriate HTML tags and ending tags.
//
BOOL writehtmltag (LPSPREADSHEET lpSS, SS_COORD lCol, SS_COORD lRow, TCHAR *tcTags, LPHTML lpHtml, FILE **fp)
{
	LPTSTR			lptstrBuf	 = NULL;
	GLOBALHANDLE	ghBuf		 = (GLOBALHANDLE)0;
	UINT			uiCellDataLen = 0;
	BOOL	bRet = TRUE;
#ifdef SS_V40
  SS_COORD lRowHeaderIndex;
  SS_COORD lColHeaderIndex;
#endif

	begintag (lpSS, fp, lCol, lRow, tcTags, lpHtml);

	// The biggest variable is "how long is the data ?" Already know that there is not going
	// to be more than 512 bytes of HTML tags so we will use the data length plus the maximum
	// amount of space we will need for tags.  This will eliminate the need to keep reallocating
	// in the many functions for each tag.
	//
	uiCellDataLen = (SS_GetDataEx (lpSS, lCol, lRow, NULL, -1) + BUFSIZ*10 + 1) * sizeof (TCHAR);
	
	ghBuf	  = (GLOBALHANDLE)GlobalAlloc (GHND, uiCellDataLen);
	lptstrBuf = (LPTSTR)GlobalLock (ghBuf);

	// Note: need to make a message here for returning if we cannot get our resources.
	//
	if (!lptstrBuf) return FALSE;

	// Check to see if we have a web formula.
	//
	if (!check4formula (lpSS, lpHtml, lCol, lRow, uiCellDataLen, lptstrBuf))
	{	
#ifdef SS_V40
    if( lpSS->Row.lHeaderDisplayIndex == -1 || lpSS->Row.lHeaderDisplayIndex >= lpSS->Row.HeaderCnt)
      lRowHeaderIndex = lpSS->Row.HeaderCnt - 1;
    else
      lRowHeaderIndex = lpSS->Row.lHeaderDisplayIndex;
    if( lpSS->Col.lHeaderDisplayIndex == -1 || lpSS->Col.lHeaderDisplayIndex >= lpSS->Col.HeaderCnt)
      lColHeaderIndex = lpSS->Col.HeaderCnt - 1;
    else
      lColHeaderIndex = lpSS->Col.lHeaderDisplayIndex;
#endif

    if (SS_GetDataEx(lpSS, lCol, lRow, lptstrBuf, -1))
		{	nbspalignment (lpHtml, lptstrBuf, uiCellDataLen);

			// JIM8969
			// If we run into a password cell, replace all with '*'
			//
			dontshowpassword (lpSS, lCol, lRow, lptstrBuf);
		}
#ifdef SS_V40
		else if (lRow == lRowHeaderIndex && lCol >= lpSS->Col.HeaderCnt)
#else
    else if (lRow == 0 && lCol != 0)
#endif
		{	getheaderdata (lpSS, lCol, FALSE, lptstrBuf);
    }
#ifdef SS_V40	
		else if (lCol == lColHeaderIndex && lRow >= lpSS->Row.HeaderCnt)
#else
    else if (lCol == 0 && lRow != 0)
#endif
		{	getheaderdata (lpSS, lRow, TRUE, lptstrBuf);
		}

		else
		{	*lptstrBuf = 0;

			// No data in these cells, could be pictures, checkboxes, things we don't support
			// for the export or empty cells.
			//
			nodatacells (lpSS, lCol, lRow, lpHtml, tcTags, lptstrBuf);

			// If we run into a password cell, replace all with '*'
			//
			dontshowpassword (lpSS, lCol, lRow, lptstrBuf);
		}
	}
	_fputts (tcTags, *fp);

	endtag (lCol, lRow, tcTags, lptstrBuf, uiCellDataLen, fp);
	
	GlobalUnlock (ghBuf);
	GlobalFree (ghBuf);

	return TRUE;
}

// jb - 15.oct.98 ---------------------------------------------------------
//
// See if we need the upper left cell.
//
static void writeupperleftcell (LPSPREADSHEET lpSS, SS_COORD lColStart, SS_COORD lColEnd, FILE **fp, LPHTML lpHtml)
{
	TCHAR tcTag[BUFSIZ*10];

	_tcscpy (tcTag, TAGROWHEADER);

	writehtmltag (lpSS, 0, 0, tcTag, lpHtml, fp);

	// This means we are just writing the row headers so we can close the row tag.
	//
	if (lColStart == 0 && lColStart == lColEnd)
		_fputts (_T("\t</TR>\n"), *fp);

	return;
}

// jb - 02 Nov 98 ---------------------------------------------------------
//
// Write out the beginning of  the html log file header.
//
static void logfileheader (LPCTSTR lpszHtmlFile, LPHTML lpHtml)
{
	TCHAR tcBuf[BUFSIZ*10];
	TCHAR tcTimeBuf[32];

	// Don't let the log file get over 100K  We won't user this yet.  Maybe one day when
	// we have a property to set this on/off
	//
//	killfile (100000, lpszLogFile);

	LoadString((HINSTANCE)fpInstance, IDS_LOGS_HEAD1_HTML, tcBuf, BUFSIZ*10-1);
	_stprintf (lpHtml->tcBuf, tcBuf, getdatetimestring (tcTimeBuf));
	writemsg (lpHtml);

	LoadString((HINSTANCE)fpInstance, IDS_LOGS_HEAD3, tcBuf, BUFSIZ*10-1);
	_stprintf (lpHtml->tcBuf, tcBuf);
	writemsg (lpHtml);

	LoadString((HINSTANCE)fpInstance, IDS_LOGS_HEAD4, tcBuf, BUFSIZ*10-1);
	_stprintf (lpHtml->tcBuf, tcBuf);
	writemsg (lpHtml);

	return;
}

// jb - 18.sep.98 ---------------------------------------------------------
//
// Initialize the html structure with the html and log filename to write to.
//
void init_filenames (LPSPREADSHEET lpSS, LPCTSTR lpctFileName, LPCTSTR lpctLogFile, LPHTML lpHtml)
{

	// JQB01 - 02.11.99
	init_logfile (lpctLogFile, lpHtml);

	// Start our log file.
	//
	logfileheader (lpctFileName, lpHtml);
  // what is with this? no reason to be setting unittype! -scl
//	SSSetUnitType (lpSS->lpBook->hWnd, SS_UNITTYPE_NORMAL);


	return;
}

// jb - 28.sep.98 ---------------------------------------------------------
//
// Write the column headers (row 0) for the spreadsheet.
//
BOOL writecolumnheaders (LPSPREADSHEET lpSS, SS_COORD *lRowStart, SS_COORD lColStart, SS_COORD lColEnd, FILE **fp, LPHTML lpHtml)
{	
	BOOL fRet = TRUE;
	SS_COORD lCol;
	TCHAR tcTags[BUFSIZ*10];
	BOOL fDirty = FALSE;
#ifdef SS_V40
  TCHAR tcTag[BUFSIZ*10];
  SS_COORD lRow;
//  int numcols;
  SS_COORD lColAnchor, lNumCols;
#else // don't do this in spread 4! -scl
	if (*lRowStart == 0) *lRowStart = 1;
#endif

	// If no display of column headers, return;
	//
	if (!lpHtml->fDisplayColHeaders)
	{	return TRUE;
	}

#ifdef SS_V40
//  if( lpHtml->fDisplayRowHeaders )
//    numcols = lpHtml->lColEnd - lpHtml->lColStart + 1 + lpSS->Col.HeaderCnt;
//  else
//    numcols = lpHtml->lColEnd - lpHtml->lColStart + 1;
  // loop through each row of column headers
  for( lRow = 0; lRow <= lpHtml->lRowEnd && lRow < lpSS->Row.HeaderCnt; lRow++ )
  {
//    fDirty = FALSE;
	  if (lpHtml->fDisplayRowHeaders)
	  {	
      _tcscpy (tcTag, TAGROWHEADER);
      for( lCol = 0; lCol < lpSS->Col.HeaderCnt; lCol++ )
      {
        // check for span across this cell
//        if( lpHtml->pbspanning[lRow * numcols + lCol] )
//          continue;
        if( SS_SPAN_YES == SS_GetCellSpan(lpSS, lCol, lRow, NULL, NULL, NULL, NULL) )
          continue;
	      writehtmltag (lpSS, lCol, lRow, tcTag, lpHtml, fp);
        _tcscpy (tcTag, TAGHEADERTAB2);
      }
      // This means we are just writing the row headers so we can close the row tag.
	    //
	    if (lColStart == 0 && lColEnd == lpSS->Col.HeaderCnt - 1)
		    _fputts (_T("\t</TR>\n"), *fp);
		  if (lpHtml->fUpperLeft) return FALSE;
      if( lColStart < lpSS->Col.HeaderCnt )
        lColStart = lpSS->Col.HeaderCnt;
	  }
	  else
		  _fputts (TAGROW, *fp);

    // If just doing a single  header row cell or all row headers, no need to
	  // continue in the column header (row 0) code.
	  //
	  if (lpHtml->fRowHeaders) return TRUE;
	  if (lpHtml->fRange && lColStart == 0 && lColEnd == lpSS->Col.HeaderCnt - 1) return TRUE;

	  // The first tag.  Only display upper left cell if both column and row 
	  // headers are to display.
	  //
	  for (lCol = lColStart; lCol <= lColEnd; lCol++)
	  {	if (colhidden (lpSS, lCol)) continue;

      // check for span across this cell
/*
      if( lpHtml->fDisplayRowHeaders )
      {
        if( lpHtml->pbspanning[lRow * numcols + lCol - lpHtml->lColStart + lpSS->Col.HeaderCnt] )
        continue;
      }
      else
      {
        if( lpHtml->pbspanning[lRow * numcols + lCol - lpHtml->lColStart] )
        continue;
      }
*/
      // check for span over this cell and skip it if the anchor cell's column
      // is inside the range being exported
      if( SS_SPAN_YES == SS_GetCellSpan(lpSS, lCol, lRow, &lColAnchor, NULL, NULL, NULL) 
            && lColStart <= lColAnchor && lColAnchor <= lColEnd )
        continue;
		  fDirty = TRUE;
		  _tcscpy (tcTags, TAGHEADERTAB2);
	  
		  // We return the tcTags string because it has 'beginning' 
		  // html tags that we will use to determine the ending tags.
		  //
		  writehtmltag (lpSS, lCol, lRow, tcTags, lpHtml, fp);
	  }

	  if (fDirty)
		  _fputts (_T("\t</TR>\n"), *fp);
    // check for spans that includes entire row of column headers
    // row tag needs to be closed
    if( SS_SPAN_NO != SS_GetCellSpan(lpSS, lColStart, lRow, NULL, NULL, &lNumCols, NULL)
        && lNumCols == lColEnd - lColStart + 1
        && (!lpHtml->fDisplayRowHeaders 
        || (SS_SPAN_NO != SS_GetCellSpan(lpSS, 0, lRow, NULL, NULL, &lNumCols, NULL)
        && lNumCols == lpSS->Col.HeaderCnt)) )
      _fputts (_T("\t<TR>\n\t</TR>\n"), *fp);
  }
  if( *lRowStart < lpSS->Row.HeaderCnt )
    *lRowStart = lpSS->Row.HeaderCnt;
#else // !SS_V40
	// Get rid of row headers if column is 0, this also gets rid of the upper
	// left cell.
	//
	if (lpHtml->fDisplayRowHeaders)
	{	
    writeupperleftcell (lpSS, lColStart, lColEnd, fp, lpHtml);
		if (lpHtml->fUpperLeft) return FALSE;
	}
	else
		_fputts (TAGROW, *fp);

	// If just doing a single  header row cell or all row headers, no need to
	// continue in the column header (row 0) code.
	//
	if (lpHtml->fHeaderRowCell || lpHtml->fRowHeaders) return TRUE;
	if (lpHtml->fRange && lColStart == 0 && lColStart == lColEnd) return TRUE;

	// If you are at the upper left cell, and we are going across to do more
	// column headers, then we can set the start to 1 because we have already
	// done it.
	//
	if (lColStart == 0 && lColStart != lColEnd) lColStart = 1;

	// The first tag.  Only display upper left cell if both column and row 
	// headers are to display.
	//
	for (lCol = lColStart; lCol <= lColEnd; lCol++)
	{	if (colhidden (lpSS, lCol)) continue;
		
		fDirty = TRUE;
		_tcscpy (tcTags, TAGHEADERTAB2);
	
		// We return the tcTags string because it has 'beginning' 
		// html tags that we will use to determine the ending tags.
		//
		writehtmltag (lpSS, lCol, 0, tcTags, lpHtml, fp);
	}

	if (fDirty)
		_fputts (_T("\t</TR>\n"), *fp);
#endif

	return TRUE;
}

// jb - 18.sep.98
//
// Start our <table> tags.  If there is no information to put into the table, close the table tag.
//
BOOL startcolumnheaders (LPSPREADSHEET lpSS, SS_COORD *lRowStart, SS_COORD lColStart, SS_COORD lColEnd, FILE **fp, LPHTML lpHtml)
{
	BOOL bRet = FALSE;

	if (!writecolumnheaders (lpSS, lRowStart, lColStart, lColEnd, fp, lpHtml))
	{	if (lpHtml->fColHeaders || lpHtml->fUpperLeft || lpHtml->fHeaderColCell)
		{	endtable (fp, lpHtml);
			logfilefooter (lpHtml);
			bRet = TRUE;
		}
	}

	return bRet;
}

// jb - 29.oct.98 ---------------------------------------------------------
//
// Initialize the HTML structure. 
//
static void initializehtml (LPHTML lpHtml, SS_COORD lColStart, SS_COORD lRowStart, SS_COORD lColEnd, SS_COORD lRowEnd, LPCTSTR lpszFileName, BOOL bAppendFlag)
{
	lpHtml->fSpreadSheet	= FALSE;
	lpHtml->fColHeaders 	= FALSE;
	lpHtml->fSpecifiedRow	= FALSE;
	lpHtml->fRowHeaders 	= FALSE;
	lpHtml->fUpperLeft		= FALSE;
	lpHtml->fHeaderRowCell	= FALSE;
	lpHtml->fSpecifiedCol	= FALSE;
	lpHtml->fHeaderColCell	= FALSE;
	lpHtml->fSpecifiedCell	= FALSE;
	lpHtml->fRange			= FALSE;
	lpHtml->fNoTable		= FALSE;
	lpHtml->bNoBorder		= FALSE;
	lpHtml->bAllowCellOverFlow = FALSE;
	lpHtml->lColStart = lColStart;
	lpHtml->lColEnd   = lColEnd;
	lpHtml->lRowStart = lRowStart;
	lpHtml->lRowEnd   = lRowEnd;
	lpHtml->bAppendFlag = bAppendFlag;
	_tcscpy (lpHtml->tcHtmlFile, lpszFileName);
	*lpHtml->tcBuf = 0;
	lpHtml->jpegindex = 0;
	lpHtml->bPictureHits = FALSE;
	lpHtml->iSpannedColumn = 0;

	return;
}

// jb - 28.sep.98 ---------------------------------------------------------
//
// Write row and header tag.  The header <TH> will also have attributes.
// <TR>
//		<TH attributes>data</TH>
//
void writerowheader (LPSPREADSHEET lpSS, SS_COORD lCol, SS_COORD lRow, FILE **fp, TCHAR *tcTags, LPHTML lpHtml)
{
	writehtmltag (lpSS, lCol, lRow, tcTags, lpHtml, fp);
	return;
}

// jb - 18.sep.98 ---------------------------------------------------------
//
// Make sure you have row headers if specified.  This is useful when you are going to display
// row headers and no column headers.
//
BOOL justrowheaders (LPSPREADSHEET lpSS, SS_COORD lCol, SS_COORD lRow, FILE **fp, TCHAR *tcTags, LPHTML lpHtml)
{
	BOOL bRet = FALSE;
  SS_COORD c;

	if (lpHtml->fRowHeaders || lpHtml->fHeaderRowCell)
	{	
    _tcscpy (tcTags, TAGROWHEADER);
    for( c = lpHtml->lColStart; c < lpSS->Col.HeaderCnt; c++ )
    {
		  writerowheader (lpSS, c, lRow, fp, tcTags, lpHtml);
      _tcscpy (tcTags, TAGHEADERTAB2);
    }
		bRet = TRUE;
	}

	return bRet;
}

// jb - 18.sep.98 ---------------------------------------------------------
//
// This will display all row headers (or not) and setup for <TD> table data info.
//
void displayrowheaders (LPSPREADSHEET lpSS, SS_COORD lRow, FILE **fp, TCHAR *tcTags, LPHTML lpHtml)
{
  SS_COORD lCol;

 	if (lpHtml->fDisplayRowHeaders)
 	{	
    _tcscpy (tcTags, TAGROWHEADER);
    for( lCol = lpHtml->lColStart; lCol < lpSS->Col.HeaderCnt; lCol++ )
    {
 		  writerowheader (lpSS, lCol, lRow, fp, tcTags, lpHtml);
      _tcscpy (tcTags, TAGHEADERTAB2);
    }
 		_tcscpy (tcTags, TAGDATA3TABS);
 	}
 	else
	{	_tcscpy (tcTags, TAGROWDATA);
	}

	return;
}

// jb - 18.sep.98 ---------------------------------------------------------
//
// In this case, user specified a specific row to export.
//
BOOL displayspecifiedrow (LPSPREADSHEET lpSS, SS_COORD lRow, SS_COORD lColStart, SS_COORD lCol, FILE **fp, TCHAR *tcTags, LPHTML lpHtml)
{
	BOOL bRet = FALSE;
  SS_COORD c;

	if (lpHtml->fDisplayRowHeaders)
	{
		// Modify by BOC 99.7.19 (hyt)-----------------------------
	 	// for just export one row it will not export first column
	 	if (lColStart == lCol)
		{	
      _tcscpy (tcTags, TAGROWHEADER);
      for( c = lCol; c < lpSS->Col.HeaderCnt; c++ )
      {
			  writerowheader (lpSS, c, lRow, fp, tcTags, lpHtml);
        _tcscpy (tcTags, TAGHEADERTAB2);
      }
			if (lCol == lpSS->Col.HeaderCnt - 1) bRet = TRUE;
		}

		// If we have advanced past the first column and written our header since
		// it is turned on, all we need is our data tag.
		//
		_tcscpy (tcTags, TAGDATA3TABS);
	}
	else if (lColStart == lCol)
	{	_tcscpy (tcTags, TAGROWDATA);
	}
	else
	{	_tcscpy (tcTags, TAGDATA2TABS);
	}

	return bRet;
}

// jb - 18.sep.98 ---------------------------------------------------------
//
// And here, user wants just a range of data.  Have to have headers come out right when
// doing this...(if they want headers)
//
BOOL displayrangerowheader (LPSPREADSHEET lpSS, SS_COORD lRow, SS_COORD* plCol, SS_COORD lColStart, FILE **fp, TCHAR *tcTags, LPHTML lpHtml)
{
	BOOL bRet = FALSE;
  SS_COORD c;
#ifdef SS_V40
  SS_COORD lRowAnchor;
#endif
	if (lpHtml->fDisplayRowHeaders && *plCol < lpSS->Col.HeaderCnt)
	{	
    _tcscpy (tcTags, TAGROWHEADER);
    for( c = 0; c < lpSS->Col.HeaderCnt; c++ )
    {
#ifdef SS_V40
      // check for span over this cell and skip it if the anchor cell's row
      // is inside the range being exported
      if( SS_SPAN_YES == SS_GetCellSpan(lpSS, c, lRow, NULL, &lRowAnchor, NULL, NULL) 
          && lpHtml->lRowStart <= lRowAnchor && lRowAnchor <= lpHtml->lRowEnd )
        continue;
#endif
			writerowheader (lpSS, c, lRow, fp, tcTags, lpHtml);
      _tcscpy (tcTags, TAGHEADERTAB2);
    }
//			if (lCol == lpSS->Col.HeaderCnt - 1) bRet = TRUE;
    if( lpHtml->lColStart < lpSS->Col.HeaderCnt )
      // continue from first non-header column
      *plCol = lpSS->Col.HeaderCnt - 1;
    else
      // continue from first column to be exported
      *plCol = lpHtml->lColStart - 1;
    // (minus 1 is because the loop increments it in calling function)
    bRet = TRUE;
	  _tcscpy (tcTags, TAGDATA3TABS);
	}
	else
	{	if (lColStart == *plCol)
			_tcscpy (tcTags, TAGROWDATA);
		else
			_tcscpy (tcTags, TAGDATA2TABS);
	}

	return bRet;
}

// jb - 16.sep.98 ---------------------------------------------------------
//
// Get default settings for 
//
LPHTML getdefaultattributes (LPSPREADSHEET lpSS, SS_COORD lColStart, SS_COORD lRowStart, SS_COORD lColEnd, SS_COORD lRowEnd, LPCTSTR lpszFileName, BOOL bAppendFlag, LPCTSTR lpszLogFile, LPHTML lpHtml)
{
	ULONG ulLockBackColor, ulLockForeColor;

	// See if user turned off column/row headers.
	//
	lpHtml->fDisplayColHeaders = SS_GetBool(lpSS->lpBook, lpSS, SSB_SHOWCOLHEADERS);
	lpHtml->fDisplayRowHeaders = SS_GetBool(lpSS->lpBook, lpSS, SSB_SHOWROWHEADERS);

#ifndef SS_V40
	// If the user hides column 0 (row headers) this overrides fDisplayRowHeaders.
	//
	if (colhidden (lpSS, 0))
	{	lpHtml->fDisplayRowHeaders = FALSE;
	}

	// If the user hides row 0 (column headers) this overrides fDisplayColHeaders.
	//
	if (rowhidden (lpSS, 0))
	{	lpHtml->fDisplayColHeaders = FALSE;
	}
#endif

	SS_GetLockColor (lpSS, &ulLockBackColor, &ulLockForeColor);
	lpHtml->clrLockBack = ulLockBackColor;
	lpHtml->clrLockFore = ulLockForeColor;

	initializehtml (lpHtml, lColStart, lRowStart, lColEnd, lRowEnd, lpszFileName, bAppendFlag);

	// True if user wants to export entire spreadsheet.
	// (including col & row headers, and upper left -scl)
	if (lColStart == 0 && lColEnd > 0 && lRowStart == 0 && lRowEnd > 0)
		lpHtml->fSpreadSheet = TRUE;

	// The Column Headers
	// (range of column header cells -scl)
//	else if (lColStart >= 0 && lColEnd > 0 && lRowStart == 0 && lRowEnd == 0)
	else if (lColStart >= 0 && lColEnd > 0 && lRowStart >= 0 && lRowEnd < lpSS->Row.HeaderCnt)
		lpHtml->fColHeaders = TRUE;

	// Specified Row
	// (but not a row of column headers -scl)
	else if (lColStart >= 0 && lColEnd > 0 && lRowStart > 0 && lRowStart == lRowEnd)
		lpHtml->fSpecifiedRow = TRUE;

	// Row Headers
	// (range of row header cells -scl)
//	else if (lColStart == 0 && lColStart == lColEnd && lRowStart >= 0 && lRowEnd > 0)
	else if (lColStart >= 0 && lColEnd < lpSS->Col.HeaderCnt && lRowStart >= 0 && lRowEnd > 0)
		lpHtml->fRowHeaders = TRUE;

	// Upper Left cell(s)
	// (range of upper-left cells -scl)
//	else if (lColStart == 0 && lColEnd == 0 && lRowStart == 0 && lRowEnd == 0)
	else if (lColStart >= 0 && lColEnd < lpSS->Col.HeaderCnt && lRowStart >= 0 && lRowEnd < lpSS->Row.HeaderCnt)
		lpHtml->fUpperLeft = TRUE;

// this case is covered under "range of row header cells"
	// The header cell at specified row within row headers.
	//
//	else if (lColStart == 0 && lColStart == lColEnd && lRowStart != 0 && lRowStart == lRowEnd)
//		lpHtml->fHeaderRowCell = TRUE;

	// Specific Column
	// (but not a column of row headers -scl)
	else if (lColStart != 0 && lColStart == lColEnd && lRowStart >= 0 && lRowStart != lRowEnd)
		lpHtml->fSpecifiedCol = TRUE;

// this case is covered under "range of column header cells"
	// The header cell at specified col within col headers.
	//
//	else if (lColStart != 0 && lColStart == lColEnd && lRowStart == 0 && lRowEnd == 0)
//		lpHtml->fHeaderColCell = TRUE;

	// Specified Cell
	//
	else if (lColStart != 0 && lColStart == lColEnd && lRowStart != 0 && lRowStart == lRowEnd)
		lpHtml->fSpecifiedCell = TRUE;

	// A range of cells
	//
	else
		lpHtml->fRange = TRUE;

	if (!lpHtml->fDisplayRowHeaders)
	{	if      (lpHtml->fUpperLeft)		lpHtml->fNoTable = TRUE;
//		else if (lpHtml->fHeaderRowCell)	lpHtml->fNoTable = TRUE;
		else if (lpHtml->fRowHeaders)		lpHtml->fNoTable = TRUE;
	}

	// If not displaying the col headers, reset any of the defaults back to FALSE
	// that do not make sense if col headers are turned off.
	//
	if (!lpHtml->fDisplayColHeaders)
	{	if		(lpHtml->fUpperLeft)		lpHtml->fNoTable = TRUE;
//		else if (lpHtml->fHeaderColCell)	lpHtml->fNoTable = TRUE;
		else if (lpHtml->fColHeaders)		lpHtml->fNoTable = TRUE;
	}

	// See if we are allowing cells to overflow.
	//
	if (SS_GetBool(lpSS->lpBook, lpSS, SSB_ALLOWCELLOVERFLOW))
	{	lpHtml->bAllowCellOverFlow = TRUE;
	}

	// If no border is set, there will be NO gridlines or border.
	//
	if (SS_GetBool(lpSS->lpBook, lpSS, SSB_NOBORDER))
	{	lpHtml->bNoBorder = TRUE;
	}

	return lpHtml;
}

// jb - 07.oct.98 ---------------------------------------------------------
//
// SS_ALLCOLS, SS_ALLROWS was specified therefore, we export the entire spreadsheet.
//
//	#define TABLE			_T("</TABLE>")
//	#define TAGROW			_T("\t<TR>\n")
//	#define TAGROWHEADER	_T("\t<TR>\n\t\t<TH")
//	#define TAGROWDATA		_T("\t<TR>\n\t\t<TD")
//	#define TAGDATA3TABS	_T("\t\t\t<TD")
//	#define TAGDATA2TABS	_T("\t\t<TD")

BOOL allcolumnsandrows (LPSPREADSHEET lpSS, TCHAR *tcTags, SS_COORD* plCol, SS_COORD lRow, FILE **fp, LPHTML lpHtml)
{
	BOOL fRet = FALSE;
  SS_COORD c;
#ifdef SS_V40
//  int numcols;
//  SS_COORD lColAnchor, lRowAnchor, lNumCols;
#endif

	if (lpHtml->fDisplayRowHeaders)
	{	
//    numcols = lpHtml->lColEnd - lpHtml->lColStart + 1 + lpSS->Col.HeaderCnt;
    if (*plCol < lpSS->Col.HeaderCnt)
		{	
      _tcscpy (tcTags, TAGROWHEADER);
      for( c = 0; c < lpSS->Col.HeaderCnt; c++ )
      {
#ifdef SS_V40
        // check for span across this cell
/*
        if( lpHtml->fDisplayColHeaders )
        {
          if( lpHtml->pbspanning[(lRow - lpHtml->lRowStart + lpSS->Row.HeaderCnt) * numcols + c - lpHtml->lColStart] )
            continue;
        }
        else
        {
          if( lpHtml->pbspanning[(lRow - lpHtml->lRowStart) * numcols + *lCol - lpHtml->lColStart] )
            continue;
        }
*/
        if( SS_SPAN_YES == SS_GetCellSpan(lpSS, c, lRow, NULL, NULL, NULL, NULL) )
          continue;
#endif
	 		  writerowheader (lpSS, c, lRow, fp, tcTags, lpHtml);
        _tcscpy (tcTags, TAGHEADERTAB2);
      }
      // continue from first non-header column
      *plCol = lpSS->Col.HeaderCnt - 1;
      // (minus 1 is because the loop increments it in calling function)
			fRet = TRUE;
		}
		else
			_tcscpy (tcTags, TAGDATA3TABS);
	}
	else
// fix for 9886 -scl
//	{	if (*plCol == lpSS->Col.HeaderCnt)
	{	if (lpHtml->fHeaderRowCell && *plCol == lpSS->Col.HeaderCnt)
		{	_fputts (TAGROW, *fp);
			fRet = TRUE;
		}
		else
			_tcscpy (tcTags, TAGDATA2TABS);
	}

	return fRet;
}

// jb - 15.oct.98 -------------------------------------------------------------
//
// Based on the coordinates passed in, we may abruptly end the table.  Or it might
// be time to end the table after all the columns/rows have been written.
//
void endtable (FILE **fp, LPHTML lpHtml)
{
	TCHAR tcBuf[BUFSIZ*10];

	_fputts (TABLE, *fp);
 	fclose (*fp);

	if (lpHtml->fNoTable)
	{	LoadString((HINSTANCE)fpInstance, IDS_LOGS_FAILEDEXPORT, tcBuf, BUFSIZ*10-1);
		_stprintf (lpHtml->tcBuf, tcBuf, lpHtml->tcHtmlFile);
	}
	else
	{	LoadString((HINSTANCE)fpInstance, IDS_LOGS_SUCCESSEXPORT, tcBuf, BUFSIZ*10-1);
		_stprintf (lpHtml->tcBuf, tcBuf, lpHtml->lColStart, lpHtml->lColEnd, lpHtml->lRowStart, lpHtml->lRowEnd, lpHtml->tcHtmlFile);
	}

	writemsg (lpHtml);

	return;
}

// jb - 02.nov.98 ---------------------------------------------------------
//
// This is the ending footer for the log file.
//
void logfilefooter (LPHTML lpHtml)
{
	LoadString((HINSTANCE)fpInstance, IDS_LOGS_FOOTER_HTML, lpHtml->tcBuf, BUFSIZ*10-1);
	writemsg (lpHtml);

	return;
}

// jb - 27.jul.99 ---------------------------------------------------------
//
// If we exported any pictures, we created jpegs.  Release the memory.
//
void maincleanup (LPHTML lpHtml)
{
	int i;

	for (i = 0; i < lpHtml->jpegindex; i++)
	{	GlobalUnlock (lpHtml->jpeg[i].ghBuf);
		GlobalFree (lpHtml->jpeg[i].ghBuf);
	}

	return;
}

// end of version 3.xx
#endif

