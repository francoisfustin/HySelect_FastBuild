
/*********************************************************
* SS_HTML.C
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
// jb  02.11.99	GIC8469	 Don't write a logfile if not specified.
// jb  02.16.99	JIS4351  If anchor is blank, make anchor default to URL
// jb  02.16.99	JIS4352  If import .xls file with URL, Excel strips the protocol off sending 
//	 					 //www.fpoint.com.  Strip off the // and put http://
// jb  07.07.99	SCS8516  Fixed multiple things that have to do with columnspanning.
// jb  07.21.99 GIC10056 Made web safe colors work with system colors.
// jb  08.03.99 JIS4550  No more duplicate jpeg's for same bitmap's, icon's
// jb  08.03.99 JIS4621  logfilefooter() now points to the lpHtml->tcBuf instead of tcBuf.
// jb  08.06.99 KEM35    writemsg() opens the logfile for write instead of append.
// jb  08.12.99 JIS4536  Verticle/Horizontal alignment was not working properly for pict
// jb  08.12.99 KEM36    TypePictCenter and MainTainScale were not working properly for pictures.
// jb  08.13.99 GIC10056 The system color black was not working.
// jb  03.10.00          Took supporting code out, made \ss\src\web\release\web.lib so this can
//                       be used in other projects, like XML, DHTML, XHTML, etc.

#include <windows.h>
#include <tchar.h>
#include <stdio.h>
#include <stdlib.h>
#include "spread.h"
#include "ss_html.h"
#if !defined(SS_V80) && !defined(SS_V70)
#include "..\..\..\..\fplibs\fptools\jpeg\jpeglib.h"
#endif
#include "ss_save4.h"
#ifdef SS_V40
#include "ss_alloc.h"
#include "ss_bool.h"
#include "ss_cell.h"
#include "ss_data.h"
#include "ss_doc.h"
#include "ss_draw.h"
#include "ss_main.h"
#include "ss_span.h"
#include "ss_user.h"
#endif

#define STRINGS_RESOURCE 1
#include "..\..\web\src\web.h"

#ifdef SS_V70
#include "..\..\..\fplibs\CxImage\CxImage\fpimages.h"
#include "..\..\..\fplibs\CxImage\CxImage\ximage.h"

BOOL SS_ExportPictToFile(HANDLE hpic, LPTSTR szPath, LPTSTR buf);
#endif
//#pragma optimize( "g", off )

#ifdef SS_V40
// check 
BOOL IsEntireRowSpanned(LPSPREADSHEET lpSS, SS_COORD lRow, LPHTML lpHtml)
{
  SS_COORD lCol;
  SS_COORD lColStart, lColAnchor, lRowAnchor;
  BOOL bRet = TRUE;

  if( lpHtml->fDisplayRowHeaders )
    lColStart = 0;
  else
    lColStart = lpSS->Col.HeaderCnt;
  if( lpHtml->fSpreadSheet )
  { // entire spreadsheet is being exported
    // return TRUE if every cell in this row
    // is part of a span but not an anchor cell
    for( lCol = lColStart; lCol < lpSS->Col.Max + lpSS->Col.HeaderCnt; lCol++ )
      if( SS_SPAN_YES != SS_GetCellSpan(lpSS, lCol, lRow, NULL, NULL, NULL, NULL) )
      {
        bRet = FALSE;
        break;
      }
  }
  else
  { // only part of the spreadsheet is being exported
    // return TRUE if every cell in the row headers
    // is part of a span but not an anchor cell, and the
    // anchor cell is within the range being exported . . .
    for( lCol = lColStart; lCol < lpSS->Col.HeaderCnt; lCol++ )
      if( SS_SPAN_YES != SS_GetCellSpan(lpSS, lCol, lRow, NULL, &lRowAnchor, NULL, NULL) 
          && lpHtml->lRowStart <= lRowAnchor && lRowAnchor <= lpHtml->lRowEnd )
      {
        bRet = FALSE;
        break;
      }
    if( bRet )
    { // . . . and every cell in this row and in the
      // range being exported is part of a span but not an anchor cell,
      // and the anchor cell is within the range being exported . . .
      for( lCol = lpHtml->lColStart; lCol <= lpHtml->lColEnd; lCol++ )
        if( SS_SPAN_YES != SS_GetCellSpan(lpSS, lCol, lRow, &lColAnchor, &lRowAnchor, NULL, NULL) 
            && lpHtml->lColStart <= lColAnchor && lColAnchor <= lpHtml->lColEnd
            && lpHtml->lRowStart <= lRowAnchor && lRowAnchor <= lpHtml->lRowEnd )
        {
          bRet = FALSE;
          break;
        }
    }
  }
  return bRet;
}
#endif
//*****************************************************************************
// Dll interface:
//
// jb - 27.jul.98 -------------------------------------------------------------
//
// This method will export the ENTIRE spreadsheet to the specified HTML file.
// If the user forgets to set maxcols and rows, this export will get the max
// column number with data and last row with data and only export that.
//
// lpszFilename - The html filename to export to.
// bAppendFlag  - Default is FALSE.  If you want to append one html file onto
//				  the bottom of another, use TRUE.  Make sure that the spread
//				  HTML file appending has the same number of columns otherwise
//				  make sure you like the appearance this will give you.
// pszLogFile   - Writes errors and other information to the log specified.
//
// jb - 27.jul.98 ---------------------------------------------------------
//
// Export the spread sheet 'range' to a specified HTML file.
//
// jb - 13 jul 98 ---------------------------------------------------------
// jb - 22.mar.00 moved 3/4 of the code to \ss\src\web\web.cpp and made a library
//                called web.lib  Did this because in the future, with XML, XHTML
//                etc. I can use these functions.
//
// Main function for reading spread sheet information and exporting it to a HTML table.
//

BOOL SS_ExportRangeToHTMLBuffer (LPSPREADSHEET lpSS, SS_COORD lColStart, SS_COORD lRowStart, SS_COORD lColEnd, SS_COORD lRowEnd, LPTSTR szPath, BOOL bUseDataMax, BOOL bAppend, LPSS_BUFF pBuff, LPCTSTR lpszTitle, LPCTSTR lpszCssFile, LPCTSTR lpszTableClass, LPCTSTR lpszTableRowClass, LPCTSTR lpszTableDefinitionClass, LPCTSTR lpszTableHeaderClass);

BOOL SS_ExportRangeToHTML (LPSPREADSHEET lpSS, SS_COORD lColStart, SS_COORD lRowStart, SS_COORD lColEnd, SS_COORD lRowEnd, LPCTSTR lpszFileName, BOOL bAppendFlag, LPCTSTR lpszLogFile, BOOL bUseDataMax, LPCTSTR lpszTitle, LPCTSTR lpszCssFile, LPCTSTR lpszTableClass, LPCTSTR lpszTableRowClass, LPCTSTR lpszTableDefinitionClass, LPCTSTR lpszTableHeaderClass)
{
#ifdef SS_V40
// fix for bug 9784 -scl
//	FILE *fp = _tfopen(lpszFileName, _T("w"));
	FILE *fp = NULL;
  TCHAR szPath[_MAX_PATH];
  int i;

  TCHAR tDrive[_MAX_DRIVE], 
   	  tDir[_MAX_DIR], 
   	  tFileName[_MAX_FNAME], 
   	  tExt[_MAX_EXT],
   	  tPath[_MAX_PATH];

  tDrive[0] = tDir[0] = tFileName[0] = tExt[0] = 0;
  memset(tPath,0,_MAX_PATH);
  _tsplitpath (lpszFileName, tDrive, tDir, tFileName, tExt);
  lstrcpy(tPath, lpszFileName);
  if (tExt[0] == 0)
    _tcscat(tPath, _T(".htm"));

  lpszFileName = tPath;

#ifdef _UNICODE
//GAB 1/26/2004 This case needs to be revisited
// fix for 14502 -scl
  if( bAppendFlag )
  {
    if( fp = _tfopen(lpszFileName, _T("r")) )
    {
      fclose(fp);
      fp = _tfopen(lpszFileName, _T("r+b"));
      while( !feof(fp) )
		  {	
        _fgetts(szPath, _MAX_PATH, fp);
			  if( !_tcsncmp(szPath, HTMLTAG, _tcslen(HTMLTAG)) )
			  {
				  long n = (lstrlen(szPath)+1)*sizeof(TCHAR);
				  fseek(fp, -n, SEEK_CUR);
				  break;
			  }
		  }
    }
  }
  else
    fp = _tfopen(lpszFileName, _T("wb"));
//GAB 1/26/2004 Add Byte Order Mark for Unicode
  if (fp && !bAppendFlag)
    _fputtc(0xFEFF,fp);
 //   fputc(0xFF,fp);

#else
  if( bAppendFlag )
  {
    if( fp = _tfopen(lpszFileName, _T("r")) )
    {
      fclose(fp);
      fp = _tfopen(lpszFileName, _T("r+"));
      while( !feof(fp) )
		  {	
        _fgetts(szPath, _MAX_PATH, fp);
			  if( !_tcsncmp(szPath, HTMLTAG, _tcslen(HTMLTAG)) )
			  {
				  long n = (lstrlen(szPath)+1)*sizeof(TCHAR);
				  fseek(fp, -n, SEEK_CUR);
				  break;
			  }
		  }
    }
  }
  else
    fp = _tfopen(lpszFileName, _T("w"));
#endif

  if( fp )
  { // chop off filename from path
    SS_BUFF Buff;

    lstrcpy(szPath, lpszFileName);
    for( i = lstrlen(szPath); i > 0 && szPath[i-1] != '\\'; i--, szPath[i] = 0);
    if( 0 == lstrlen(szPath) )
    {
      GetCurrentDirectory(_MAX_PATH, szPath);
      if( szPath[lstrlen(szPath)-1] != (TCHAR)'\\' )
        _tcscat(szPath, _T("\\"));
    }
#if SS_V80
    SS_BuffInitFP(&Buff, fp);
#else
    SS_BuffInit(&Buff);
#endif
    SS_ExportRangeToHTMLBuffer(lpSS, lColStart, lRowStart, lColEnd, lRowEnd, szPath, bUseDataMax, bAppendFlag, &Buff, lpszTitle, lpszCssFile, lpszTableClass, lpszTableRowClass, lpszTableDefinitionClass, lpszTableHeaderClass);
    if (Buff.hMem)
    {
      _fputts((LPCTSTR)Buff.pMem, fp);
      SS_BuffFree(&Buff);
      fclose(fp);
      return TRUE;
    }
  }
  return FALSE;
#else
  FILE *fp = NULL;
// fix for bug 10020 -scl
//	TCHAR tcTags[BUFSIZ];
	TCHAR tcTags[BUFSIZ*10];
	SS_COORD lRow, lCol = 0;
	HTML html;
//#ifdef SS_V40
//  SS_COORD lColAnchor, lRowAnchor;
//  BOOL fBeginRow;
//#endif

	// Put our logfile and html files in our structure.
	//
	init_filenames (lpSS, lpszFileName, lpszLogFile, &html);

	// Normalize the column and Row coordinates.
	//
  	if (!init_colandrowcoordinates (lpSS, &lColStart, &lRowStart, &lColEnd, &lRowEnd, &html, lpszFileName))
	{	return FALSE;
	}

	// Virtual mode
	//
	init_virtualmode (lpSS, &lRowStart, &lRowEnd);

	// Get default settings and store in the html structure
	//
	html = *getdefaultattributes (lpSS, lColStart, lRowStart, lColEnd, lRowEnd, lpszFileName, bAppendFlag, lpszLogFile, &html);

	// Open the HTML file for writing.
	//
	if (!openfile (lpSS->lpBook->hWnd, &html, &fp))
	{	return FALSE;
	}

	// <TABLE> tag and check to see if we have an empty table.  If so, we just write </table>
	//
	if (startoftable (lpSS, &fp, &html))
	{	
    return TRUE;
	}

	// This will write our first row, the column headers.  If we are only concerned
	// with writing just the column header, then we return.
	//
	if (startcolumnheaders (lpSS, &lRowStart, lColStart, lColEnd, &fp, &html))
	{	
    return TRUE;
	}

  if( html.fDisplayRowHeaders )
    lColStart = 0;
 // fix for bug 9886 -scl
  else
    lColStart = 1;
	
	// Now that we have written row 0 (the column headers) we can write out the 
	// rest of the rows, (unless they are hidden)
	//
	for (lRow = lRowStart; lRow <= lRowEnd; lRow++)
	{	
//#ifdef SS_V40
    // assume that we will need to insert a row begin tag
//    fBeginRow = TRUE;
    // check for spans across this entire row; 
//    if( IsEntireRowSpanned(lpSS, lRow, &html) )
//    { // insert an empty row tag
//      _fputts (_T("\t<TR></TR>\n"), fp);
//      continue;
//    }
//#endif
		if (isrowhidden (lpSS, lCol, lRow, &html))
		{	continue;
		}

		// For each row, write out all of the columns, spanning if we have to.
		//
		for (lCol = lColStart; lCol <= lColEnd; lCol++)
		{	
			tcTags[0] = html.tcBuf[0] = 0;

			// Hiding a column is the default even if the user leaves the default of
			// fDisplayRowHeaders to TRUE, if column 0 is hidden, no row headers.
			//
			if (iscolhidden (lpSS, lCol, lRow, &html))
			{	continue;
			}

//#ifdef SS_V40
//      // check for span across this cell
//      if( SS_SPAN_YES == SS_GetCellSpan(lpSS, lCol, lRow, &lColAnchor, &lRowAnchor, NULL, NULL) 
//          && lColStart <= lColAnchor && lColAnchor <= lColEnd 
//          && lRowStart <= lRowAnchor && lRowAnchor <= lRowEnd )
//        continue;
//
//      if( fBeginRow && lCol == lpSS->Col.HeaderCnt )
//        _fputts(TAGROW, fp);
//#endif
  		// If we are doing the entire spreadsheet get the appropriate tags and write
			// the row header.
			//
			if (html.fSpreadSheet)
				if (allcolumnsandrows (lpSS, tcTags, &lCol, lRow, &fp, &html)) 
        {
//#ifdef SS_V40
//          fBeginRow = FALSE;
//#endif
          continue;
        }
			// If we are just doing the row headers, write out our column 0 and continue.
			//
			if (justrowheaders (lpSS, lCol, lRow, &fp, tcTags, &html)) continue;

      // If just a specified cell or Column, then we need to write the row header
			// if turned on.  Else, just write the data.
			//
			if (html.fSpecifiedCell || html.fSpecifiedCol)
			{	displayrowheaders (lpSS, lRow, &fp, tcTags, &html);
			}


  			// Write a specified row of data.  First we need the header cell if specified.
			//
			else if  (html.fSpecifiedRow)
			{	if (displayspecifiedrow (lpSS, lRow, lCol, lColStart, &fp, tcTags, &html)) continue;
			}

			// If we are doing a range, then we need to display the associated row header
			// for that coordinate.
			//
			else if (html.fRange)
			{	if (displayrangerowheader (lpSS, lRow, &lCol, lColStart, &fp, tcTags, &html)) continue;
			}

			html.iSpannedColumn = lCol;
			writehtmltag (lpSS, lCol, lRow, tcTags, &html, &fp);
			lCol = html.iSpannedColumn;
		}
		if (_tcsstr (tcTags, _T("<TD")) || _tcsstr (tcTags, _T("<TH")))
		{  	_fputts (_T("\t</TR>\n"), fp);
		}
	}

	// End of table.
	//
	endtable      (&fp, &html);
	logfilefooter (&html);
	maincleanup   (&html);

  return TRUE;
#endif
}

#ifdef SS_V40
// appends the closest web-safe R, G, or B color value to the buffer
void WebSafeClr(LPSS_BUFF pBuff, WORD wRGB)
{
  if( wRGB < 26 )
  	SS_BuffCat(pBuff, _T("00"));
  else if( wRGB < 77 )
  	SS_BuffCat(pBuff, _T("33"));
  else if( wRGB < 128 )
  	SS_BuffCat(pBuff, _T("66"));
  else if( wRGB < 179 )
  	SS_BuffCat(pBuff, _T("99"));
  else if( wRGB < 230 )
  	SS_BuffCat(pBuff, _T("CC"));
  else
  	SS_BuffCat(pBuff, _T("FF"));
}
// returns DT_LEFT, DT_CENTER, or DT_RIGHT
UINT GetHAlign(LPSPREADSHEET lpSS, SS_COORD lCol, SS_COORD lRow)
{
  SS_CELLTYPE ct;
  UINT nHAlign;

  SS_GetCellType(lpSS, lCol, lRow, &ct);
  switch( ct.Type )
  { // alignment style bits depend on the cell type
  case SS_TYPE_PICTURE:
  case SS_TYPE_COMBOBOX:
  case SS_TYPE_CHECKBOX:
    if( ct.Style & SSS_ALIGN_RIGHT )
      nHAlign = DT_RIGHT;
    else if( ct.Style & SSS_ALIGN_CENTER )
  	  nHAlign = DT_CENTER;
    else 
  	  nHAlign = DT_LEFT;
    break;
  case SS_TYPE_STATICTEXT:
    if( ct.Style & SS_TEXT_RIGHT )
      nHAlign = DT_RIGHT;
    else if( ct.Style & SS_TEXT_CENTER )
  	  nHAlign = DT_CENTER;
    else 
  	  nHAlign = DT_LEFT;
    break;
  default:
    if( ct.Style & ES_RIGHT )
      nHAlign = DT_RIGHT;
    else if( ct.Style & ES_CENTER )
  	  nHAlign = DT_CENTER;
    else 
  	  nHAlign = DT_LEFT;
  }
  return nHAlign;
}
// returns DT_TOP, DT_VCENTER, or DT_BOTTOM
UINT GetVAlign(LPSPREADSHEET lpSS, SS_COORD lCol, SS_COORD lRow)
{
  SS_CELLTYPE ct;
  UINT nVAlign;

  SS_GetCellType(lpSS, lCol, lRow, &ct);
  // get the cell vertical alignment
  if( SS_TYPE_STATICTEXT == ct.Type )
  {
    if( ct.Style & SS_TEXT_BOTTOM )
      nVAlign = DT_BOTTOM;
    else if( ct.Style & SS_TEXT_VCENTER )
      nVAlign = DT_VCENTER;
    else 
      nVAlign = DT_TOP;
  }
  else
  {
    if( ct.Style & SSS_ALIGN_BOTTOM )
      nVAlign = DT_BOTTOM;
    else if( ct.Style & SSS_ALIGN_VCENTER )
      nVAlign = DT_VCENTER;
    else 
      nVAlign = DT_TOP;
  }
  return nVAlign;
}
// returns width of text in cell
int GetTextWidth(LPSPREADSHEET lpSS, SS_COORD lCol, SS_COORD lRow, LPTSTR szData)
{
  int nTextWidth = 0;
  UINT fuFormat = DT_LEFT | DT_CALCRECT | DT_SINGLELINE;
  RECT rc = {0};
  HDC hdc;
  HFONT hFont;
  SS_CELLTYPE ct;

  if( szData )
  {
    hdc = GetDC(lpSS->lpBook->hWnd);
    SS_GetCellType(lpSS, lCol, lRow, &ct);
    if( SS_TYPE_STATICTEXT != ct.Type || !(ct.Style | SS_TEXT_PREFIX) )
      fuFormat |= DT_NOPREFIX;
    hFont = SS_GetFontHandle(lpSS, lCol, lRow);
    if( NULL == hFont )
      hFont = GetStockObject(SYSTEM_FONT);
    if( hFont )
      SelectObject(hdc, hFont);
    DrawText(hdc, szData, lstrlen(szData), &rc, fuFormat);
    nTextWidth = rc.right - rc.left;
    ReleaseDC(lpSS->lpBook->hWnd, hdc);
  }
  return nTextWidth;
}
// exports a range of cells to an HTML buffer
BOOL SS_ExportRangeToHTMLBuffer (LPSPREADSHEET lpSS, SS_COORD lColStart, SS_COORD lRowStart, SS_COORD lColEnd, SS_COORD lRowEnd, LPTSTR szPath, BOOL bUseDataMax, BOOL bAppend, LPSS_BUFF pBuff, LPCTSTR lpszTitle, LPCTSTR lpszCssFile, LPCTSTR lpszTableClass, LPCTSTR lpszTableRowClass, LPCTSTR lpszTableDefinitionClass, LPCTSTR lpszTableHeaderClass)
{
//  UINT uiBufSize = 0;     // size of buffer
//  const UINT uiAllocSize = 4096 * sizeof(TCHAR); // alloc 4K at a time
//  const UINT uiReAllocAt = 2048 * sizeof(TCHAR);  // realloc when < 2K bytes left
  SS_COORD lCol, lRow;    // loop variables
  long lColCnt, lRowCnt;  // table column and row counts
  SS_COORD lDataCols, lDataRows;// col/row data counts
  BOOL bColHeaders;       // column headers flag
  WORD wColHeaderDisplay; // column header labels
  SS_COORD lColHeaderIndex; // column index of row header labels
  BOOL bRowHeaders;       // row headers flag
  WORD wRowHeaderDisplay; // row header labels
  SS_COORD lRowHeaderIndex; // row index of column header labels
  BOOL bBorder;           // border flag
  WORD wGridType;         // grid type
  BOOL bOverflow;         // cell overflow flag
  COLORREF clrLockBack;   // lock back color
  COLORREF clrLockFore;   // lock fore color
  COLORREF clrShadowBack; // shadow back color
  COLORREF clrShadowFore; // shadow fore color
  TCHAR buf[_MAX_PATH] = {0};// temp buffer
  SS_CELLTYPE ct;         // celltype
  BOOL bHeader;           // header cell flag
  BOOL bHeaderLabel;      // header cell label flag
  BOOL bHeader3D;		  // header cell 3D flag
  COLORREF clrBack;       // cell back color
  COLORREF clrFore;       // cell fore color
  HFONT hFont;            // cell font handle
  LOGFONT lf;             // log font for cell
  int pts, webfontsize;   // font size in pts, web font size
  int nHeight, nWidth;    // cell height, width
  UINT nHAlign, nVAlign;  // cell halign, valign (0=left/top, 1=center, 2=right/bottom)
  WORD wSpan;             // cell span
  SS_COORD lColAnchor;    // span anchor column
  SS_COORD lRowAnchor;    // span anchor row
  SS_COORD lNumCols;      // span number of columns
  SS_COORD lNumRows;      // span number of rows
  TBGLOBALHANDLE hData = 0;   // handle to cell data
  LPTSTR szData = NULL;   // ptr to cell data
  long lDataLen;          // length of cell data
  long l;                 // loop variable
  int nTextWidth;         // text width
  int nSpanWidth;         // span width
  SS_CELLTYPE ctspan;     // temp cell type for spanning
  UINT nSpanAlign;        // alignment for spanning
  TBGLOBALHANDLE hSpanData;      // span data handle
  LPTSTR szSpanData;      // ptr to span data
  long lSpanDataLen;      // length of span data
  LPSS_COL lpCol;         // ptr to spread column
  LPSS_ROW lpRow;         // ptr to spread row
  SS_SELBLOCK merge;      // merge block
  BOOL bPicts = (szPath != NULL && lstrlen(szPath));// pictures flag
#define MAX_JPEGS 500 // max number of jpegs supported
  HANDLE hPicts[MAX_JPEGS];// picture handles saved as jpegs
  LPTSTR pszPicts[MAX_JPEGS];// jpeg names
  int nPictCnt = 0;       // count of jpegs
  int nPict;              // index of current cell pict
#ifndef SS_V70
  BITMAP2JPEG bmp2jpeg;   // struct for bmp to jpeg conversion
#endif
// fix for bug 10286 -scl
  long lHiddenCols;
  BOOL bIsURL;
  long ltemp;

  // gather some info for export
  bColHeaders = SS_GetBool(lpSS->lpBook, lpSS, SSB_SHOWCOLHEADERS);
  wColHeaderDisplay = SS_GetColHeaderDisplay(lpSS);
  bRowHeaders = SS_GetBool(lpSS->lpBook, lpSS, SSB_SHOWROWHEADERS);
  wRowHeaderDisplay = SS_GetRowHeaderDisplay(lpSS);
  bBorder = !SS_GetBool(lpSS->lpBook, lpSS, SSB_NOBORDER);
  wGridType = SS_GetGridType(lpSS);
  bOverflow = SS_GetBool(lpSS->lpBook, lpSS, SSB_ALLOWCELLOVERFLOW);
  SS_GetLockColor(lpSS, &clrLockBack, &clrLockFore);
  SS_GetShadowColor(lpSS, &clrShadowBack, &clrShadowFore, NULL, NULL);
  if( lpSS->Col.lHeaderDisplayIndex == -1 )
    lColHeaderIndex = lpSS->Col.HeaderCnt - 1;
  else
    lColHeaderIndex = lpSS->Col.lHeaderDisplayIndex;
  if( lpSS->Row.lHeaderDisplayIndex == -1 )
    lRowHeaderIndex = lpSS->Row.HeaderCnt - 1;
  else
    lRowHeaderIndex = lpSS->Row.lHeaderDisplayIndex;
  // set up starting and ending col and row
  if( -1 == lColStart && -1 == lRowStart )
  {
    lColStart = 0;
    lColEnd = lpSS->Col.Max + lpSS->Col.HeaderCnt;
    lRowStart = 0;
    lRowEnd = lpSS->Row.Max + lpSS->Row.HeaderCnt;
  }
  else if( -1 == lColStart )
  {
    lColStart = 0;
    lColEnd = lpSS->Col.Max + lpSS->Col.HeaderCnt;
    lRowStart = min(lpSS->Row.Max + lpSS->Row.HeaderCnt - 1, max(0,lRowStart));
    lRowEnd = max(lRowStart, min(lRowEnd + 1, lpSS->Row.Max + lpSS->Row.HeaderCnt));
  }
  else if( -1 == lRowStart )
  {
    lColStart = min(lpSS->Col.Max + lpSS->Col.HeaderCnt - 1, max(0,lColStart));
    lColEnd = max(lColStart, min(lColEnd + 1, lpSS->Col.Max + lpSS->Col.HeaderCnt));
    lRowStart = 0;
    lRowEnd = lpSS->Row.Max + lpSS->Row.HeaderCnt;
  }
  else
  {
    lColStart = min(lpSS->Col.Max + lpSS->Col.HeaderCnt - 1, max(0,lColStart));
    lColEnd = max(lColStart, min(lColEnd + 1, lpSS->Col.Max + lpSS->Col.HeaderCnt));
    lRowStart = min(lpSS->Row.Max + lpSS->Row.HeaderCnt - 1, max(0,lRowStart));
    lRowEnd = max(lRowStart, min(lRowEnd + 1, lpSS->Row.Max + lpSS->Row.HeaderCnt));
  }
  // get col/row data counts
  SS_GetDataCnt(lpSS, &lDataCols, &lDataRows);
  if( bOverflow )
  { // check for cells that overflow with centered alignment
    TBGLOBALHANDLE hTemp;
    for( lCol = 0; lCol < lDataCols; lCol++ )
    {
      if( SS_IsColHidden(lpSS, lCol) )
        continue;
      for( lRow = 0; lRow < lDataRows; lRow++ )
      {
        if( SS_IsRowHidden(lpSS, lRow) )
          continue;
        if( DT_CENTER == GetHAlign(lpSS, lCol, lRow) && (lDataLen = SS_GetDataEx(lpSS, lCol, lRow, NULL, 0)) )
        { // cell has centered horozontal alignment and has data
          hTemp = tbGlobalAlloc(GHND, (lDataLen + 1 ) * sizeof(TCHAR));
          if( hTemp )
          {
            szData = (LPTSTR)tbGlobalLock(hTemp);
            SS_GetDataEx(lpSS, lCol, lRow, szData, lDataLen + 1);
            SS_GetCellType(lpSS, lCol, lRow, &ct);
            // hide data if cell is type edit and password style is set
            if( SS_TYPE_EDIT == ct.Type && (ES_PASSWORD & ct.Style) )
              _tcsnset(szData, '*', lDataLen);
            nTextWidth = GetTextWidth(lpSS, lCol, lRow, szData);
            tbGlobalUnlock(hTemp);
            tbGlobalFree(hTemp);

            nSpanWidth = SS_GetColWidthInPixels(lpSS, lCol);
            if( nTextWidth >  nSpanWidth)
            { // cell is overflowing, find last column of overflow
              nTextWidth /= 2;
              nSpanWidth /= 2;
              for( l = lCol + 1; l < lColEnd && nTextWidth > nSpanWidth; l++ )
                if( !SS_IsColHidden(lpSS, l) )
                  nSpanWidth += SS_GetColWidthInPixels(lpSS, l);
              lDataCols = max(lDataCols, l + 1);
            }
          }
        }
      }
    }
  }
// fix for bug 9598 -scl
  if( bUseDataMax )
  {
    if( bPicts )
    { // check for picture cells after last col/row of data
      for( lRow = lDataRows; lRow < lpSS->Row.Max + lpSS->Row.HeaderCnt; lRow++ )
      { // start from row after last row with data
// 20548 -scl
//        for( lCol = 0; lCol < lDataCols; lCol++ )
        for( lCol = 0; lCol < lDataCols + lpSS->Col.HeaderCnt; lCol++ )
        { // search every column from 0 to lDataCols - 1 for pictures
          SS_GetCellType(lpSS, lCol, lRow, &ct);
          if( SS_TYPE_PICTURE == ct.Type && ct.Spec.ViewPict.hPictName )
            lDataRows = max(lRow + 1, lDataRows);
        }
      }
      for( lCol = lDataCols; lCol < lpSS->Col.Max + lpSS->Col.HeaderCnt; lCol++ )
      { // start from column after last column with data
        for( lRow = 0; lRow < lpSS->Row.Max + lpSS->Row.HeaderCnt; lRow++ )
        { // search every row
          SS_GetCellType(lpSS, lCol, lRow, &ct);
          if( SS_TYPE_PICTURE == ct.Type && ct.Spec.ViewPict.hPictName )
            lDataCols = max(lCol + 1, lDataCols);
        }
      }
    }
    lColEnd = min(lColEnd, lDataCols);
    lRowEnd = min(lRowEnd, lDataRows);
  }
  // calculate column and row counts for the table
  if( bColHeaders && bRowHeaders )
  {
    lColCnt = lColEnd - lColStart + lpSS->Col.HeaderCnt;
    lRowCnt = lRowEnd - lRowStart + lpSS->Row.HeaderCnt;
  }
  else if( bColHeaders )
  {
// fix for bug 9886 -scl
    if( lColStart < lpSS->Col.HeaderCnt )
      lColStart = lpSS->Col.HeaderCnt;
    lColCnt = lColEnd - lColStart;
    lRowCnt = lRowEnd - lRowStart + lpSS->Row.HeaderCnt;
  }
  else if( bRowHeaders )
  {
// fix for bug 9886 -scl
    if( lRowStart < lpSS->Row.HeaderCnt )
      lRowStart = lpSS->Row.HeaderCnt;
    lColCnt = lColEnd - lColStart + lpSS->Col.HeaderCnt;
    lRowCnt = lRowEnd - lRowStart;
  }
  else
  {
// fix for bug 9886 -scl
    if( lColStart < lpSS->Col.HeaderCnt )
      lColStart = lpSS->Col.HeaderCnt;
    if( lRowStart < lpSS->Row.HeaderCnt )
      lRowStart = lpSS->Row.HeaderCnt;
    lColCnt = lColEnd - lColStart;
    lRowCnt = lRowEnd - lRowStart;
  }
  // verify that there are columns and rows to export
  if( lColCnt == 0 || lRowCnt == 0 )
    return FALSE;
  // allocate a buffer
  if( !bAppend )
	SS_BuffCat(pBuff, _T("<HTML>\n"));

  if (lpszCssFile != NULL || lpszTitle != NULL)
  {
	SS_BuffCat(pBuff, _T("<HEAD>\n"));
	if (lpszTitle != NULL)
	{
		SS_BuffCat(pBuff, _T("<TITLE>"));
		SS_BuffCat(pBuff, (LPTSTR)lpszTitle);
		SS_BuffCat(pBuff, _T("</TITLE>\n"));
	}
	if (lpszCssFile != NULL)
	{
		SS_BuffCat(pBuff, _T("<link type=\"text/css\" rel=\"stylesheet\" href=\""));
		SS_BuffCat(pBuff, (LPTSTR)lpszCssFile);
		SS_BuffCat(pBuff, _T("\">\n"));
	}
	SS_BuffCat(pBuff, _T("</HEAD>\n"));
  }
  // start TABLE tag
  SS_BuffCat(pBuff, _T("<TABLE"));
  if (lpszTableClass != NULL)
  {
		SS_BuffCat(pBuff, _T(" class=\""));
		SS_BuffCat(pBuff, (LPTSTR)lpszTableClass);
		SS_BuffCat(pBuff, _T("\""));
  }
  
  else
  {
  SS_BuffCat(pBuff, _T(" CELLSPACING=0 CELLPADDING=0"));
  // BORDER attribute
  if( bBorder )
    SS_BuffCat(pBuff, _T(" BORDER=1"));
  else
    SS_BuffCat(pBuff, _T(" BORDER=0"));
  }
  // RULES attribute
  if( wGridType & SS_GRID_HORIZONTAL && wGridType & SS_GRID_VERTICAL )
    SS_BuffCat(pBuff, _T(" RULES=ALL"));
  else if( wGridType & SS_GRID_HORIZONTAL )
    SS_BuffCat(pBuff, _T(" RULES=ROWS"));
  else if( wGridType & SS_GRID_VERTICAL )
    SS_BuffCat(pBuff, _T(" RULES=COLS"));
  else
    SS_BuffCat(pBuff, _T(" RULES=NONE"));
  // COLS and ROWS attributes
  SS_BuffCat(pBuff, _T(" COLS="));
  SS_BuffCat(pBuff, _ltot(lColCnt, buf, 10));
  SS_BuffCat(pBuff, _T(" ROWS="));
  SS_BuffCat(pBuff, _ltot(lRowCnt, buf, 10));
  SS_BuffCat(pBuff, _T(">\n"));
  // loop through each cell
  for( lRow = (bColHeaders ? 0 : lRowStart); lRow < lRowEnd; lRow++ )
  {
    // need to skip to the starting row to export?
    if( lRow < lRowStart && lRow >= lpSS->Row.HeaderCnt )
      lRow = lRowStart;
// fix for bug 9925 -scl
// check to see if the row is hidden
    if( SS_IsRowHidden(lpSS, lRow) )
      continue;

    // TR tag (table row)
	  SS_BuffCat(pBuff, _T("\t<TR"));
	  if (lpszTableRowClass != NULL)
		{
		SS_BuffCat(pBuff, _T(" class=\""));
		SS_BuffCat(pBuff, (LPTSTR)lpszTableRowClass);
		SS_BuffCat(pBuff, _T("\""));
		}

	  SS_BuffCat(pBuff, _T(">\n"));
    for( lCol = (bRowHeaders ? 0 : lColStart); lCol < lColEnd; lCol++ )
    {
      // need to skip to the starting col to export?
      if( lCol < lColStart && lCol >= lpSS->Col.HeaderCnt )
        lCol = lColStart;
// fix for bug 9925 -scl
// check to see if the column is hidden
      if( SS_IsColHidden(lpSS, lCol) )
        continue;
      // check to see if the buffer needs to be reallocated
/*
      if( uiBufSize - (lstrlen(szBuf) * sizeof(TCHAR)) < uiReAllocAt )
      { // realloc the buffer
        HGLOBAL temp;
        tbGlobalUnlock(hRet);
        temp = tbGlobalReAlloc(hRet, uiBufSize + uiAllocSize, GHND);
        if( temp == NULL )
        {
          tbGlobalFree(hRet);
          return FALSE;
        }
        hRet = temp;
        uiBufSize += uiAllocSize;
        szBuf = (LPTSTR)tbGlobalLock(hRet);
      }
*/
	  bIsURL = FALSE;
      // get cell span
      lNumCols = lNumRows = 0;
      wSpan = SS_GetCellSpan(lpSS, lCol, lRow, &lColAnchor, &lRowAnchor, &lNumCols, &lNumRows);
      if( SS_SPAN_YES == wSpan )
      { 
// fix for RUN_SPR_001_014 -scl
        // is anchor cell in sheet corner and this cell not?
        if( lColAnchor < lpSS->Col.HeaderCnt && lRowAnchor < lpSS->Row.HeaderCnt 
          && (lCol >= lpSS->Col.HeaderCnt || lRow >= lpSS->Row.HeaderCnt) )
        {
          wSpan = SS_SPAN_NO;
          lNumCols = lNumRows = 1;
        }
        // is anchor cell in row header and this cell not?
        else if( lColAnchor < lpSS->Col.HeaderCnt && lCol >= lpSS->Col.HeaderCnt )
        {
          wSpan = SS_SPAN_NO;
          lNumCols = lNumRows = 1;
        }
        // is anchor cell in column header and this cell not?
        else if( lRowAnchor < lpSS->Row.HeaderCnt && lRow >= lpSS->Row.HeaderCnt )
        {
          wSpan = SS_SPAN_NO;
          lNumCols = lNumRows = 1;
        }
        // is the anchor cell in the range being exported?
        else if( ((bRowHeaders && lColAnchor < lpSS->Col.HeaderCnt) 
            || (lColStart <= lColAnchor && lColAnchor < lColEnd))
            && ((bColHeaders && lRowAnchor < lpSS->Row.HeaderCnt)
            || (lRowStart <= lRowAnchor && lRowAnchor < lColEnd)) )
          continue; // skip this cell
        // is this the first cell in the span in the range being exported?
        else if( lCol == max(lColStart, lColAnchor) && lRow == max(lRowStart, lRowAnchor) )
        { // adjust span for range being exported
          long cols = lNumCols;
          long rows = lNumRows;
          if( lColAnchor < lColStart )
            cols -= (lColStart - lColAnchor);
          if( lColAnchor + lNumCols > lColEnd )
            cols -= (lColEnd - lCol);
          if( lRowAnchor < lRowStart )
            rows -= (lRowStart - lRowAnchor);
          if( lRowAnchor + lNumRows > lRowEnd )
            rows -= (lRowEnd - lRow);
          lNumCols = cols;
          lNumRows = rows;
        }
        else // not first cell in range
          continue; // skip this cell;
      }
// fix for RUN_SPR_001_014 -scl
      // make sure span does not exceed bounds of headers
      if( lCol < lpSS->Col.HeaderCnt && lRow < lpSS->Row.HeaderCnt )
      {
        if( lNumRows > lpSS->Row.HeaderCnt - lRow )
          lNumRows = lpSS->Row.HeaderCnt - lRow;
        if( lNumCols > lpSS->Col.HeaderCnt - lCol )
          lNumCols = lpSS->Col.HeaderCnt - lCol;
      }
      else if( lCol < lpSS->Col.HeaderCnt && lNumCols > lpSS->Col.HeaderCnt - lCol )
        lNumCols = lpSS->Col.HeaderCnt - lCol;
      else if( lRow < lpSS->Row.HeaderCnt && lNumRows > lpSS->Row.HeaderCnt - lRow )
        lNumRows = lpSS->Row.HeaderCnt - lRow;
      if( lCol + lNumCols > lColEnd )
        lNumCols = (lColEnd - lCol);
      if( lRow + lNumRows > lRowEnd )
        lNumRows = (lRowEnd - lRow);
// fix for bug 10286 -scl
// fix for 14229 -scl
//      for( l = lCol; l < lColEnd && l < lCol + lNumCols - 1; l++ )
      ltemp = lNumCols;
      for( l = lCol; l < lColEnd && l < lCol + ltemp; l++ )
        if( SS_IsColHidden(lpSS, l) )
          lNumCols--;
// fix for 14229 -scl
//      for( l = lRow; l < lRowEnd && l < lRow + lNumRows - 1; l++ )
      ltemp = lNumRows;
      for( l = lRow; l < lRowEnd && l < lRow + ltemp; l++ )
        if( SS_IsRowHidden(lpSS, l) )
          lNumRows--;
      // get the cell type
	    SS_GetCellType(lpSS, lCol, lRow, &ct);
      // is this a header cell?
	  bHeader = lCol < lpSS->Col.HeaderCnt || lRow < lpSS->Row.HeaderCnt;
      bHeader3D = (ct.Type == SS_TYPE_STATICTEXT && (ct.Style & SS_TEXT_SHADOW || ct.Style & SS_TEXT_SHADOWIN));
      // get cell colors
      if( !bHeader3D && SS_GetLock(lpSS, lCol, lRow, TRUE) )
      { // use lock colors, unless they are not set
        if( SPREAD_COLOR_NONE == clrLockBack || RGBCOLOR_DEFAULT == clrLockBack )
          SS_GetColor(lpSS, lCol, lRow, &clrBack, NULL);
        else
          clrBack = clrLockBack;
        if( SPREAD_COLOR_NONE == clrLockFore || RGBCOLOR_DEFAULT == clrLockFore )
          SS_GetColor(lpSS, lCol, lRow, NULL, &clrFore);
        else
          clrFore = clrLockFore;
      }
      else if( bHeader3D )
      { // use shadow colors
        clrBack = clrShadowBack;
        clrFore = clrShadowFore;
      }
      else  // use cell colors
        SS_GetColor(lpSS, lCol, lRow, &clrBack, &clrFore);
      // check for system colors
      if( clrBack & 0x80000000 )
        clrBack = GetSysColor(clrBack & 0xFF);
      if( clrFore & 0x80000000 )
        clrFore = GetSysColor(clrFore & 0xFF);
// fix for bug 9710 -scl
      { // check for currency/number/percent cell type, TypeNegRed, and negative value
        double dfVal;
        if( SS_TYPE_CURRENCY <= ct.Type 
#ifdef SS_V70
            && ct.Type <= SS_TYPE_SCIENTIFIC 
#else // SS_V70
            && ct.Type <= SS_TYPE_PERCENT 
#endif // SS_V70
            && ct.Style & SSS_NEGRED 
            && SS_GetFloat(lpSS, lCol, lRow, &dfVal) 
            && dfVal < 0 )
            clrFore = RGBCOLOR_RED;
      }
      // get cell height and width
      nHeight = SS_GetRowHeightInPixels(lpSS, lRow);
      nWidth = SS_GetColWidthInPixels(lpSS, lCol);
      // get the cell alignments
      nHAlign = GetHAlign(lpSS, lCol, lRow);
      nVAlign = GetVAlign(lpSS, lCol, lRow);
      // get cell data
      lDataLen = 0;
      bHeaderLabel = FALSE;
      if( iswebformulacell(lpSS, lCol, lRow, buf) )
      { // URL formula (call into John's code)
        lDataLen = lstrlen(buf);
        hData = tbGlobalAlloc(GHND, (lDataLen + 1 ) * sizeof(TCHAR));
        if( hData )
        {
          szData = (LPTSTR)tbGlobalLock(hData);
          lstrcpy(szData, buf);
		  bIsURL = TRUE;
        }
      }
      else if( wSpan != SS_SPAN_YES && SS_TYPE_PICTURE != ct.Type 
               && (lDataLen = SS_GetDataEx(lpSS, lCol, lRow, NULL, 0)) )
      { // get cell data
        hData = tbGlobalAlloc(GHND, (lDataLen + 1 ) * sizeof(TCHAR));
        if( hData )
        {
          szData = (LPTSTR)tbGlobalLock(hData);
          SS_GetDataEx(lpSS, lCol, lRow, szData, lDataLen + 1);
          // hide data if cell is type edit and password style is set
          if( SS_TYPE_EDIT == ct.Type && (ES_PASSWORD & ct.Style) )
            _tcsnset(szData, '*', lDataLen);
        }
      }
      else if( bHeader && SS_TYPE_PICTURE != ct.Type && lCol == lColHeaderIndex && lRow >= lpSS->Row.HeaderCnt )
      { // get row header cell label
        long lHeader = lRow - lpSS->Row.HeaderCnt + lpSS->Row.NumStart;
        *buf = 0;
        switch( wRowHeaderDisplay )
        {
        case SS_HEADERDISPLAY_NUMBERS:
          _ltot(lHeader, buf, 10);
          break;
        case SS_HEADERDISPLAY_LETTERS:
          SS_DrawFormatHeaderLetter(buf, lHeader);
          break;
        }
        if( lDataLen = lstrlen(buf) )
        {
          hData = tbGlobalAlloc(GHND, (lDataLen + 1 ) * sizeof(TCHAR));
          if( hData )
          {
            szData = (LPTSTR)tbGlobalLock(hData);
            lstrcpy(szData, buf);
            bHeaderLabel = TRUE;
          }
        }
      }
      else if( bHeader && SS_TYPE_PICTURE != ct.Type && lRow == lRowHeaderIndex && lCol >= lpSS->Col.HeaderCnt )
      { // get column header cell label
        long lHeader = lCol - lpSS->Col.HeaderCnt + lpSS->Col.NumStart;
        *buf = 0;
        switch( wColHeaderDisplay )
        {
        case SS_HEADERDISPLAY_NUMBERS:
          _ltot(lHeader, buf, 10);
          break;
        case SS_HEADERDISPLAY_LETTERS:
          SS_DrawFormatHeaderLetter(buf, lHeader);
          break;
        }
        if( lDataLen = lstrlen(buf) )
        {
          hData = tbGlobalAlloc(GHND, (lDataLen + 1 ) * sizeof(TCHAR));
          if( hData )
          {
            szData = (LPTSTR)tbGlobalLock(hData);
            lstrcpy(szData, buf);
            bHeaderLabel = TRUE;
          }
        }
      }
      // get cell font
      hFont = SS_GetFontHandle(lpSS, lCol, lRow);
      if( NULL == hFont )
        hFont = GetStockObject(SYSTEM_FONT);
      GetObject(hFont, sizeof(LOGFONT), &lf);
      pts = MulDiv(abs(lf.lfHeight), 72, lpSS->lpBook->dyPixelsPerInch);
	    if (pts < 10)
		    webfontsize = 1;
	    else if (pts < 12)
		    webfontsize = 2;
	    else if (pts < 14)
		    webfontsize = 3;
	    else if (pts < 18)
		    webfontsize = 4;
	    else if (pts < 24)
		    webfontsize = 5;
	    else if (pts < 36)
		    webfontsize = 6;
	    else 
		    webfontsize = 7;
      if( wSpan == SS_SPAN_NO )
      { // no overflow in wordwrapped static or multiline edit cells
        // check for cell overflow, left align, non-empty cell
// fix for bug 10286 -scl
        lHiddenCols = 0;
        if( bOverflow && lDataLen && !bHeaderLabel && DT_LEFT == nHAlign 
              && !(SS_TYPE_STATICTEXT == ct.Type && ct.Style & SS_TEXT_WORDWRAP) 
              && !(SS_TYPE_EDIT == ct.Type && ct.Style & ES_MULTILINE) )
        { // check for text width > cell width, empty adjacent cells
          // note: this case handles LEFT alignment ONLY;
          // right and center alignment are handled below
          nTextWidth = GetTextWidth(lpSS, lCol, lRow, szData);
          if( nTextWidth > nWidth )
          {
            lNumRows = 1;
            lNumCols = 1;
            for( l = lCol + 1; nTextWidth > nWidth && l < lColEnd && !SS_GetDataEx(lpSS, l, lRow, NULL, 0); l++ )
            { // check for row header cell, don't go past last row header cell
              if( lCol < lpSS->Col.HeaderCnt && l >= lpSS->Col.HeaderCnt )
                break;  // just going to have to wrap in the row header
// fix for bug 10286 -scl
              if( SS_IsColHidden(lpSS, l) ) 
              { // skip hidden columns
                lHiddenCols++;
                continue;
              }
              // span cells
              wSpan = SS_SPAN_YES;
              lNumCols++;
              nTextWidth -= SS_GetColWidthInPixels(lpSS, l);
            }
          }
        }
        else if( bOverflow && (bHeaderLabel || !lDataLen) )
        { // no data in cell, check for adjacent cells overflowing from the right
          // find next nonempty cell in this row, sum col widths
          nSpanWidth = 0;
          hSpanData = 0;
          for( l = lCol + 1; l < lColEnd; l++ )
          { // only other row header cells can overflow into a row header cell
            if( lCol < lpSS->Col.HeaderCnt && l >= lpSS->Col.HeaderCnt )
              break; // stop checking at first non-header cell

            // RFW - 7/3/02 - DEN_DEN_003_015
            // When checking for overflow cells, it must stop when a span is found.
				if (SS_GetCellSpan(lpSS, l, lRow, NULL, NULL, NULL, NULL) != SS_SPAN_NO)
              break;

// fix for bug 10286 -scl
            if( SS_IsColHidden(lpSS, l) )
            {
              lHiddenCols++;
              continue;
            }
            SS_GetCellType(lpSS, l, lRow, &ctspan);
            nSpanWidth += SS_GetColWidthInPixels(lpSS, l);
            if( lSpanDataLen = SS_GetDataEx(lpSS, l, lRow, NULL, 0) )
            { // cell has data
              if( SS_TYPE_STATICTEXT == ctspan.Type && ctspan.Style & SS_TEXT_WORDWRAP )
                break; // no overflow from wrapped static cells
              if( SS_TYPE_EDIT == ct.Type && ct.Style & ES_MULTILINE )
                break; // no overflow from multiline edit cells
              // get the data
              hSpanData = tbGlobalAlloc(GHND, (lSpanDataLen + 1 ) * sizeof(TCHAR));
              if( hSpanData )
              {
                szSpanData = (LPTSTR)tbGlobalLock(hSpanData);
                SS_GetDataEx(lpSS, l, lRow, szSpanData, lSpanDataLen + 1);
                // hide data if cell is type edit and password style is set
                if( SS_TYPE_EDIT == ctspan.Type && (ES_PASSWORD & ctspan.Style) )
                  _tcsnset(szSpanData, '*', lSpanDataLen);
                // get text width
                nTextWidth = GetTextWidth(lpSS, l, lRow, szSpanData);
              }
              break;
            }
          }
          // check for span data
          if( hSpanData )
          { // check for right or center horizontal alignment
            nSpanAlign = GetHAlign(lpSS, l, lRow);
            if( DT_RIGHT == nSpanAlign && nTextWidth > nSpanWidth )
            { // text extends into this column, span at this cell
              nHAlign = DT_RIGHT;
              wSpan = SS_SPAN_YES;
// fix for bug 10268 -scl
//              lNumCols = l - lCol + 1;
              lNumCols = l - lCol + 1 - lHiddenCols;
              lNumRows = 1;
              // replace hData with hSpanData, szData with szSpanData
              if( hData )
              {
                tbGlobalUnlock(hData);
                tbGlobalFree(hData);
              }
              hData = hSpanData;
              szData = szSpanData;
              lDataLen = lSpanDataLen;
            }
            else if( DT_CENTER == nSpanAlign && nTextWidth / 2 > (nSpanWidth + SS_GetColWidthInPixels(lpSS, l)) / 2 )
            { // if text extends into this column, span at this cell
              // calculate the last column spanned into
// fix for bug 10268 -scl
//              for( ; l < lColEnd && nTextWidth > nSpanWidth; l++, nSpanWidth += SS_GetColWidthInPixels(lpSS, l) );
              nSpanWidth += SS_GetColWidthInPixels(lpSS, lCol);
              for( ; l < lColEnd && nTextWidth > nSpanWidth; l++ )
                if( !SS_IsColHidden(lpSS, l) )
                  nSpanWidth += SS_GetColWidthInPixels(lpSS, l);
                else
                  lHiddenCols++;
              // does span extend to this cell?
              if( nSpanWidth - SS_GetColWidthInPixels(lpSS, lCol) >= nTextWidth )
              {
                // note: if span extends past last column of data then
                // it will come out differently in the HTML table, since
                // it cuts off at the last column of data
                nHAlign = DT_CENTER;
                wSpan = SS_SPAN_YES;
// fix for bug 10268 -scl
//                lNumCols = l - lCol + 1;
                lNumCols = l - lCol + 1 - lHiddenCols;
                lNumRows = 1;
                // replace hData with hSpanData, szData with szSpanData
                if( hData )
                {
                  tbGlobalUnlock(hData);
                  tbGlobalFree(hData);
                }
                hData = hSpanData;
                szData = szSpanData;
                lDataLen = lSpanDataLen;
              }
            }
            else // left aligned or not spanning into this cell
            { // free data, treat this cell as empty
              tbGlobalUnlock(hSpanData);
              tbGlobalFree(hSpanData);
            }
          }
        }
        if( SS_SPAN_NO == wSpan )
        { // no overflows found, now check for merges
          // is cell already merged?
          if( SS_SpanIsCellAlreadyPainted(lpSS, lCol, lRow) )
            continue; // skip this cell
          // should this cell be merged?
          lpCol = SS_LockColItem(lpSS, lCol);
          lpRow = SS_LockRowItem(lpSS, lRow);
          if( SS_MergeCalcBlock(lpSS, lRow, lpRow, lCol, lpCol, &merge) )
          {
            wSpan = SS_SPAN_YES;
            lNumCols = merge.LR.Col - merge.UL.Col + 1;
            lNumRows = merge.LR.Row - merge.UL.Row + 1;
            SS_AddBlockToPaintSpanList(lpSS, &merge);
          }
          SS_UnlockColItem(lpSS, lCol);
          SS_UnlockRowItem(lpSS, lRow);
        }
      }
      if( SS_SPAN_NO != wSpan )
      { // add spanned column widths and row heights
        for( l = lCol + 1; l < lCol + lNumCols; l++ )
          if( !SS_IsColHidden(lpSS, l) )
            nWidth += SS_GetColWidthInPixels(lpSS, l);
        for( l = lRow + 1; l < lRow + lNumRows; l++ )
          if( !SS_IsRowHidden(lpSS, l) )
            nHeight += SS_GetRowHeightInPixels(lpSS, l);
      }
      // open TH or TD tag (table header or table data)
      if( bHeader3D )
	  {
    	  SS_BuffCat(pBuff, _T("\t\t<TH"));
	      if (lpszTableHeaderClass != NULL)
			{
			SS_BuffCat(pBuff, _T(" class=\""));
			SS_BuffCat(pBuff, (LPTSTR)lpszTableHeaderClass);
			SS_BuffCat(pBuff, _T("\""));
			}
	  }
      else
	  {
    	  SS_BuffCat(pBuff, _T("\t\t\t<TD"));
	      if (lpszTableDefinitionClass != NULL)
			{
			SS_BuffCat(pBuff, _T(" class=\""));
			SS_BuffCat(pBuff, (LPTSTR)lpszTableDefinitionClass);
			SS_BuffCat(pBuff, _T("\""));
			}
	  }
      if ((bHeader3D && lpszTableHeaderClass == NULL) || lpszTableDefinitionClass == NULL)
	  {
      if( lDataLen || (bPicts && ct.Type == SS_TYPE_PICTURE && ct.Spec.ViewPict.hPictName) )
      { // ALIGN attribute
  	    SS_BuffCat(pBuff, _T(" ALIGN="));
        switch( nHAlign )
        { 
        case DT_LEFT:
 	        SS_BuffCat(pBuff, _T("LEFT"));
          break;
        case DT_CENTER:
 	        SS_BuffCat(pBuff, _T("CENTER"));
          break;
        case DT_RIGHT:
     	    SS_BuffCat(pBuff, _T("RIGHT"));  
          break;
        }
        // VALIGN attribute
  	    SS_BuffCat(pBuff, _T(" VALIGN="));
        switch( nVAlign )
        {
        case DT_TOP:
 	        SS_BuffCat(pBuff, _T("TOP"));
          break;
        case DT_VCENTER:
 	        SS_BuffCat(pBuff, _T("CENTER"));
          break;
        case DT_BOTTOM:
     	    SS_BuffCat(pBuff, _T("BOTTOM"));  
          break;
        }
      }
      // BGCOLOR attribute
	   SS_BuffCat(pBuff, _T(" BGCOLOR=\"#"));
       WebSafeClr(pBuff, (short)(clrBack & 0xFF));  // red
       WebSafeClr(pBuff, (short)((clrBack & 0xFF00) >> 8)); // green
       WebSafeClr(pBuff, (short)((clrBack & 0xFF0000) >> 16)); // blue
  	   SS_BuffCat(pBuff, _T("\"")); // close quote
	  }
      // HEIGHT attribute
  	  SS_BuffCat(pBuff, _T(" HEIGHT="));
	    SS_BuffCat(pBuff, _itot(nHeight, buf, 10));
      // WIDTH attribute
  	  SS_BuffCat(pBuff, _T(" WIDTH="));
	    SS_BuffCat(pBuff, _itot(nWidth, buf, 10));
      // COLSPAN attribute
      if( lNumCols > 1 )
      {
  	    SS_BuffCat(pBuff, _T(" COLSPAN="));
	      SS_BuffCat(pBuff, _ltot(lNumCols, buf, 10));
      }
      // ROWSPAN attribute
      if( lNumRows > 1 )
      {
  	    SS_BuffCat(pBuff, _T(" ROWSPAN="));
	      SS_BuffCat(pBuff, _ltot(lNumRows, buf, 10));
      }
      // close TH or TD
  	  SS_BuffCat(pBuff, _T(">"));
      // note: must include font tag, even if there is no text in the cell, or
      // else the row heights do not come out correctly -scl
      // start FONT tag, SIZE attribute
  	  SS_BuffCat(pBuff, _T("<FONT SIZE="));
	    SS_BuffCat(pBuff, _itot(webfontsize, buf, 10));
      // COLOR attribute
  	  SS_BuffCat(pBuff, _T(" COLOR=\"#"));
     WebSafeClr(pBuff, (short)(clrFore & 0xFF));  // red
     WebSafeClr(pBuff, (short)((clrFore & 0xFF00) >> 8)); // green
     WebSafeClr(pBuff, (short)((clrFore & 0xFF0000) >> 16)); // blue
      // close quote, FACE attribute
  	  SS_BuffCat(pBuff, _T("\" FACE=\"")); 
      SS_BuffCat(pBuff, lf.lfFaceName);
  	  SS_BuffCat(pBuff, _T("\">")); // close quote, FONT tag
      // bold tag
      if( lDataLen > 0 )
      { // check to see if the buffer needs to be reallocated
        TCHAR *pbr;
        /*
        while( uiBufSize < (lstrlen(szBuf) + lDataLen + 1) * sizeof(TCHAR) )
        { // realloc the buffer
          HGLOBAL temp;
          tbGlobalUnlock(hRet);
          temp = tbGlobalReAlloc(hRet, uiBufSize + uiAllocSize, GHND);
          if( temp == NULL )
          {
            tbGlobalFree(hRet);
            return FALSE;
          }
          hRet = temp;
          uiBufSize += uiAllocSize;
          szBuf = (LPTSTR)tbGlobalLock(hRet);
        }
        */
        // bold, italic, strikethrough, and/or underline font?
		//SS_BuffCat(pBuff, _T("<PRE>"));
        if( lf.lfWeight > 400 )
          SS_BuffCat(pBuff, _T("<B>"));
        if( lf.lfItalic )
          SS_BuffCat(pBuff, _T("<I>"));
        if( lf.lfStrikeOut )
          SS_BuffCat(pBuff, _T("<S>"));
        if( lf.lfUnderline )
          SS_BuffCat(pBuff, _T("<U>"));
// fix for 14595 -scl
//        if( _tcschr(szData, ' ') )
// fix for 15629 -scl
//        if( !bIsURL && !(SS_TYPE_STATICTEXT == ctspan.Type && ctspan.Style & SS_TEXT_WORDWRAP) && !(SS_TYPE_EDIT == ct.Type && ct.Style & ES_MULTILINE) && _tcschr(szData, ' ') )
        if( !bIsURL && !(SS_TYPE_STATICTEXT == ct.Type && ct.Style & SS_TEXT_WORDWRAP) && !(SS_TYPE_EDIT == ct.Type && ct.Style & ES_MULTILINE) && _tcschr(szData, ' ') )
		{
		  while( pbr = _tcschr(szData, ' ') )
          {
			*pbr = 0;
			SS_BuffCat(pBuff, szData);
			SS_BuffCat(pBuff, _T("&nbsp;"));
			szData = ++pbr;
		  }
		}
        if( !bIsURL && _tcschr(szData, '\n') )
        {
		  while( pbr = _tcschr(szData, '\n') )
          {
			if( pbr > szData && *(pbr - 1) == '\r' )
			{
			  *--pbr = 0;
			  SS_BuffCat(pBuff, szData);
			  SS_BuffCat(pBuff, _T("<BR>"));
			  szData = pbr + 2;
			}
			else
			{
			  *pbr = 0;
			  SS_BuffCat(pBuff, szData);
			  SS_BuffCat(pBuff, _T("<BR>"));
			  szData = ++pbr;
			}
		  }
		  if( szData && *szData )
			SS_BuffCat(pBuff, szData);
          SS_BuffCat(pBuff, _T("<BR>"));
        }
        else
          SS_BuffCat(pBuff, szData);
        tbGlobalUnlock(hData);
        tbGlobalFree(hData);
        hData = 0;
        // close bold, italic, strikethrough, and/or underline font tags
        if( lf.lfWeight > 400 )
          SS_BuffCat(pBuff, _T("</B>"));
        if( lf.lfItalic )
          SS_BuffCat(pBuff, _T("</I>"));
        if( lf.lfStrikeOut )
          SS_BuffCat(pBuff, _T("</S>"));
        if( lf.lfUnderline )
          SS_BuffCat(pBuff, _T("</U>"));
 		//SS_BuffCat(pBuff, _T("</PRE>"));
     }
      else if( bPicts && SS_TYPE_PICTURE == ct.Type )
      { // check for jpeg already exported for this picture
        nPict = -1;
        for( l = 0; l < nPictCnt; l++ )
          if( hPicts[l] == (HANDLE)ct.Spec.ViewPict.hPictName )
          { // found picture
            nPict = l;
            break;
          }
        if( -1 == nPict && nPictCnt < MAX_JPEGS )
        { // not found, add a new jpeg
#ifndef SS_V70
          bmp2jpeg.hBmp = SS_PictGetBitmap(lpSS, lCol, lRow, NULL);
          bmp2jpeg.hPal = 0;
          if( Bitmap4JPEG(&bmp2jpeg) && SaveBitmap2JPEG(bmp2jpeg.hBmp, randomname(buf, szPath, _T("jpg"))) )
#else
		    HANDLE hpic = (HANDLE)SS_PictGetBitmap(lpSS, lCol, lRow, NULL);
          if( SS_ExportPictToFile(hpic, szPath, buf) )
#endif
          { // jpeg saved okay, store this picture handle in array for reuse
            pszPicts[nPictCnt] = GlobalAlloc(GPTR, _MAX_PATH * sizeof(TCHAR));
            if( pszPicts[nPictCnt] )
            {
              lstrcpy(pszPicts[nPictCnt], buf);
#ifdef SS_V70
// 20548 -scl			  
//              hPicts[nPictCnt] = hpic;
			  hPicts[nPictCnt] = (HANDLE)ct.Spec.ViewPict.hPictName;
#else
              hPicts[nPictCnt] = bmp2jpeg.hBmp;
#endif
              nPict = nPictCnt;
              ++nPictCnt;
            }
          }
        }
        if( nPict > -1 )
        { // IMAGE tag
	        SS_BuffCat(pBuff, _T("<IMG SRC=\"file://"));
	        SS_BuffCat(pBuff, pszPicts[nPict]);
	        SS_BuffCat(pBuff, _T("\">"));
        }
      }
      else
    	  SS_BuffCat(pBuff, _T("&nbsp;")); // close quote, FONT tag
 	    SS_BuffCat(pBuff, _T("</FONT>\t"));

      // close TH or TD tag
      if( bHeader3D )
    	  SS_BuffCat(pBuff, _T("</TH>\n"));
      else
    	  SS_BuffCat(pBuff, _T("</TD>\n"));

      if( wSpan != SS_SPAN_NO )
        lCol += lNumCols - 1;
    }
    // close the TR tag
	  SS_BuffCat(pBuff, _T("\t</TR>\n"));
  }
  // close TABLE tag
  SS_BuffCat(pBuff, _T("</TABLE>\n"));
  SS_BuffCat(pBuff, _T("</HTML>\n"));
  SS_SpanFreePaintList(lpSS);
  if( bPicts )
  { // free jpeg names
    for( l = 0; l < nPictCnt; l++ )
      GlobalFree(pszPicts[l]);
  }
  return TRUE;
}


#ifdef SS_V70

BOOL SS_ExportPictToFile(HANDLE hpic, LPTSTR szPath, LPTSTR buf)
{
  BOOL fOK = FALSE;

#ifdef _DEBUG
  HINSTANCE hInstDll = LoadLibrary(_T("fpimaged.dll"));
#else
  HINSTANCE hInstDll = LoadLibrary(_T("fpimage.dll"));
#endif

  if (hInstDll)
  {
    FPSAVEIMAGEPROC pFPSaveImage = (FPSAVEIMAGEPROC)GetProcAddress(hInstDll, "FPSaveImage");
    LPTSTR lpszFileName = randomname(buf, szPath, _T("jpg"));
    char path[MAX_PATH] = {0};

    _fmemset(path, 0, MAX_PATH);
#if _UNICODE
    WideCharToMultiByte(CP_ACP, 0, lpszFileName, lstrlen(lpszFileName), (char*)path, MAX_PATH, NULL, NULL);
#else
    strncpy(path, lpszFileName, __min(strlen(lpszFileName), MAX_PATH));
#endif

    fOK = pFPSaveImage(hpic, (HPALETTE)0, (LPCSTR)path, (short)CXIMAGE_FORMAT_JPG);
    FreeLibrary(hInstDll);
  }

return (fOK);
}

#endif // SS_V70

#endif


#ifdef SS_V80
BOOL DLLENTRY SSExportRangeToHTMLEx (HWND hWnd, SS_COORD lColStart, SS_COORD lRowStart, SS_COORD lColEnd, SS_COORD lRowEnd, LPCTSTR lpszFileName, BOOL bAppendFlag, LPCTSTR lpszLogFile, LPCTSTR lpszTitle, LPCTSTR lpszCssFile, LPCTSTR lpszTableClass, LPCTSTR lpszTableRowClass, LPCTSTR lpszTableDefinitionClass, LPCTSTR lpszTableHeaderClass)
{
	LPSPREADSHEET lpSS;
	BOOL          fRet;

	lpSS = SS_SheetLock(hWnd);
	fRet = SS_ExportRangeToHTML (lpSS, lColStart, lRowStart, lColEnd, lRowEnd, lpszFileName, bAppendFlag, lpszLogFile, FALSE, lpszTitle, lpszCssFile, lpszTableClass, lpszTableRowClass, lpszTableDefinitionClass, lpszTableHeaderClass);
	SS_SheetUnlock(hWnd);

	return fRet;
}
BOOL DLLENTRY SSExportToHTMLEx (HWND hWnd, LPCTSTR lpszFileName, BOOL bAppendFlag, LPCTSTR lpszLogFile, LPCTSTR lpszTitle, LPCTSTR lpszCssFile, LPCTSTR lpszTableClass, LPCTSTR lpszTableRowClass, LPCTSTR lpszTableDefinitionClass, LPCTSTR lpszTableHeaderClass)
{	
	LPSPREADSHEET lpSS;
	BOOL          fRet;

	lpSS = SS_SheetLock (hWnd);
	fRet = SS_ExportRangeToHTML (lpSS, SS_ALLCOLS, SS_ALLROWS, 0, 0, lpszFileName, bAppendFlag, lpszLogFile, TRUE, lpszTitle, lpszCssFile, lpszTableClass, lpszTableRowClass, lpszTableDefinitionClass, lpszTableHeaderClass);
	SS_SheetUnlock (hWnd);

	return fRet;
}
#endif
BOOL DLLENTRY SSExportRangeToHTML (HWND hWnd, SS_COORD lColStart, SS_COORD lRowStart, SS_COORD lColEnd, SS_COORD lRowEnd, LPCTSTR lpszFileName, BOOL bAppendFlag, LPCTSTR lpszLogFile)
{
	LPSPREADSHEET lpSS;
	BOOL          fRet;

	lpSS = SS_SheetLock(hWnd);
	fRet = SS_ExportRangeToHTML (lpSS, lColStart, lRowStart, lColEnd, lRowEnd, lpszFileName, bAppendFlag, lpszLogFile, FALSE, NULL, NULL, NULL, NULL, NULL, NULL);
	SS_SheetUnlock(hWnd);

	return fRet;
}
BOOL DLLENTRY SSExportToHTML (HWND hWnd, LPCTSTR lpszFileName, BOOL bAppendFlag, LPCTSTR lpszLogFile)
{	
	LPSPREADSHEET lpSS;
	BOOL          fRet;

	lpSS = SS_SheetLock (hWnd);
	fRet = SS_ExportRangeToHTML (lpSS, SS_ALLCOLS, SS_ALLROWS, 0, 0, lpszFileName, bAppendFlag, lpszLogFile, TRUE, NULL, NULL, NULL, NULL, NULL, NULL);
	SS_SheetUnlock (hWnd);

	return fRet;
}

//#pragma optimize( "", on )
