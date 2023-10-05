// jb - 1.may.01 --------------------------------------------------------------
//
// Take the SaveToTextFile, SaveRangeToTextFile, and LoadTextFile code out of 
// Spread4 and put it in Spread3.  
//	9088 bug unable to reproduce.
//
//
#ifdef SS_V35

// fix for bug 10020 -scl
// find and replace all occurances of "BUFSIZE" with "BUFSIZ*10"

#include <windows.h>
#include <tchar.h>
#include <stdlib.h>
#include <stdio.h>
#include <stdlib.h>
#include <io.h>
#include <fcntl.h>
#include <sys/stat.h>
#include <sys/types.h>
#include "spread.h"
#include "ss_bool.h"
#include "ss_col.h"
#include "ss_data.h"
#include "ss_doc.h"
#include "ss_draw.h"
#include "ss_main.h"
#include "ss_row.h"
#include "ss_text.h"
#include "ss_user.h"
#include "ss_xml.h"

#define STRINGS_RESOURCE 1
#include "..\..\web\src\web.h"


LPBYTE SS_TextBufferAlloc(long lBufferLen, long lValueLen, LPLONG lplAllocLen,
                          LPTBGLOBALHANDLE lphBuffer, long lAllocInc)
{
LPBYTE lpBuffer = 0;

if ((lBufferLen + lValueLen) > *lplAllocLen)
   {
   *lplAllocLen += max(lValueLen, lAllocInc);

   if (!(*lphBuffer))
      {
      if (!(*lphBuffer = tbGlobalAlloc(GMEM_MOVEABLE | GMEM_ZEROINIT, *lplAllocLen)))
         {
         *lplAllocLen = 0;
         return (FALSE);
         }
      }

   else
      {
      tbGlobalUnlock(*lphBuffer);
      if (!(*lphBuffer = tbGlobalReAlloc(*lphBuffer, *lplAllocLen, GMEM_MOVEABLE | GMEM_ZEROINIT)))
         {
         *lplAllocLen = 0;
         return (FALSE);
         }
      }

   }
   lpBuffer = (LPBYTE)tbGlobalLock(*lphBuffer);
	 lpBuffer += lBufferLen;

return (lpBuffer);
}

static BOOL IsFileMarkedUTF8 (LPTSTR lpDataIn, DWORD dwSize)
{
  BOOL bRet = FALSE;
  LPBYTE lpByte = (LPBYTE)lpDataIn;

  if ( dwSize > 3 )
  {
    if ( (0xEF == lpByte[0]) && (0xBB == lpByte[1]) && (0xBF == lpByte[2]) )
    {
		bRet = TRUE;
    }
  }
  return bRet;
}

LPVOID RemoveUTF8Marker (LPTSTR lpDataIn, DWORD *dwSize)
{
  LPBYTE lpByte = (LPBYTE)lpDataIn;
  *dwSize -= 3;
  lpByte = &lpByte[3];
  lpDataIn = (LPTSTR)lpByte;

  return lpDataIn;
}

LPVOID IsFileMarkedUnicode (LPWSTR lp, DWORD *dwSize)
{
  if ( *dwSize > sizeof(WCHAR) )
  {
    if ( *lp == BYTE_ORDER_MARK )
    {
      *dwSize -= sizeof(WCHAR);
      lp++;
    }
  }
  return lp;
}

LPVOID MarkFileUnicode (LPWSTR lp, DWORD *dwSize)
{
  if ( *dwSize > sizeof(WCHAR) )
  {
    *lp = BYTE_ORDER_MARK;
    *dwSize -= sizeof(WCHAR); // size of buffer is reduced by one character
    lp++;
  }
  return lp;
}


TBGLOBALHANDLE Convert(UINT iCodePage, LPTSTR lpDataIn, DWORD dwSizeIn, BOOL fUnicode, DWORD *dwBuffSize)
{
  DWORD dwSizeOut, dwSizeOutTemp;
  LPTSTR lpDataOut;
  TBGLOBALHANDLE ghData = tbGlobalAlloc(GHND, 1 * sizeof(TCHAR));
  BOOL bUsedDef = TRUE;
  LPWSTR lpInBuff, lpOutBuff;

    dwSizeOutTemp = dwSizeIn;

    if ( fUnicode )
    {
      DWORD dwFlags = WC_COMPOSITECHECK | WC_DEFAULTCHAR;

      lpInBuff = (LPWSTR)IsFileMarkedUnicode((LPWSTR)lpDataIn, &dwSizeIn);

      // Create an output file of the maximum size needed.
      ghData = tbGlobalReAlloc(ghData, dwSizeOutTemp, GHND);
      lpDataOut = (LPTSTR)tbGlobalLock(ghData);

#ifdef SPREAD_JPN
      iCodePage = CP_ACP;
      dwFlags = 0;
#endif

       // Perform the actual conversion.
       // The first time through we're going to get the true number of bytes required
       dwSizeOut = WideCharToMultiByte(iCodePage, dwFlags,
       lpInBuff,
       dwSizeIn/sizeof(WCHAR), // number of wide characters 
       (LPSTR)lpDataOut,
        0, // Set this to zero to get the required buffer size
       "\x7f", // use DEL as default char 
       &bUsedDef); // was default char used

       //Now get the buffer
       dwSizeOutTemp = WideCharToMultiByte(iCodePage, dwFlags,
       lpInBuff,
       dwSizeIn/sizeof(WCHAR), // number of wide characters 
       (LPSTR)lpDataOut,
       dwSizeOutTemp, // number of bytes written
       "\x7f", // use DEL as default char 
       &bUsedDef); // was default char used
  }
  else 
  {
    DWORD dwFlags = MB_COMPOSITE | MB_USEGLYPHCHARS;

	//GAB 08/02/04 Added to load UTF8 text files. 
	if ( IsFileMarkedUTF8(lpDataIn, dwSizeIn) )
	{
		lpDataIn = RemoveUTF8Marker (lpDataIn, &dwSizeIn);
		iCodePage = CP_UTF8;
		dwFlags = 0;
	}

 
   // Create an output file of the maximum size needed.
    /* RFW - 8/22/05 - 16649
    dwSizeOutTemp = (dwSizeIn+1) * sizeof(WCHAR);
    */
    dwSizeOutTemp = (dwSizeIn+2) * sizeof(WCHAR); // Add 2 extra chars.  1 for the Unicode marker and 1 for the null.
    ghData = tbGlobalReAlloc(ghData, dwSizeOutTemp, GHND);
    lpDataOut = (LPTSTR)tbGlobalLock(ghData);

      // Write a Byte Order Mark.
      lpOutBuff = (LPWSTR) MarkFileUnicode((LPWSTR) lpDataOut, &dwSizeOutTemp);
      // Perform the actual conversion.

      //The first time through we're going to return the true number of bytes required

#ifdef SPREAD_JPN
      iCodePage = CP_ACP;
      dwFlags = 0;
#endif

      dwSizeOut = MultiByteToWideChar(iCodePage, dwFlags,
      (LPSTR)lpDataIn,
      dwSizeIn, // length of input in bytes
      lpOutBuff, // output buffer
      0); // Set this to zero to get the required buffer size

      dwSizeOut += 1; //adjust buffer size

      dwSizeOutTemp = MultiByteToWideChar(iCodePage, dwFlags,
      (LPSTR)lpDataIn,
      dwSizeIn, // length of input in bytes
      lpOutBuff, // output buffer
      dwSizeOutTemp/sizeof(TCHAR)); // max number of wchars to write  // RFW - 6/10/05 - 16302
  }
  tbGlobalUnlock(ghData);

  *dwBuffSize = dwSizeOut;
	return ghData;

}
///////////////////////////////////////////////////////////////////////////////
// jb - 02.nov.98 ------------------------------------------------------------------
//
// Write the information message to logfile specified from Export method.
//
void write2log_text (SS_TEXT *pText)
{
	FILE *fp;

	if (pText->pszLogFile != 0)
	{	fp = _tfopen (pText->pszLogFile, _T("a"));

		if (!fp) return;
		_fputts(pText->tcBuf, fp);
		fclose (fp);
	}

	return;
}

// jb - 02.nov.98 ---------------------------------------------------------
//
// This is not a column or row header.  It is a cell and it is either empty or
// we could not get any data from it because of the type of cell.  Write this
// information to the log file.
//
static BYTE ischeckbox (LPSPREADSHEET lpSS, SS_COORD lCol, SS_COORD lRow)
{
	SS_CELLTYPE CellType;

	SS_GetCellType (lpSS, lCol, lRow, &CellType);

	if (CellType.Type == SS_TYPE_CHECKBOX)
		return TRUE;

	return FALSE;
}

// 21.jun.01 ------------------------------------------------------------------
//
// The user has the ability to read in column or row headers.  Fix the coordinate
// system to reflect this.
//
/*
static void Header(LPSPREADSHEET lpSS, SS_TEXT *pText, PTEXT_COORD pTextCOORD, long lFlags)
{
	pText->lCol = 0;
	pText->lRow = 0;

	if (pText->bLoadText == FALSE)
	{	// Do both column and row headers.
		//
		if ((pText->lFlags & SS_EXPORTTEXT_COLHEADERS) && (pText->lFlags & SS_EXPORTTEXT_ROWHEADERS))
		{	if (pTextCOORD->bColRange)
				pText->lCol = pTextCOORD->lColStart;
		}

		// Just row headers
		//
		else if (pText->lFlags & SS_EXPORTTEXT_ROWHEADERS)
			pText->lRow = lpSS->Row.HeaderCnt;

		else if (pText->lFlags & SS_EXPORTTEXT_COLHEADERS)
			pText->lCol = lpSS->Col.HeaderCnt;

		// No headers.
		//
		else
		{
    	pText->lCol = lpSS->Col.HeaderCnt;
			pText->lRow = lpSS->Row.HeaderCnt;
		}
	}

	// Must be doing a LoadTextFile, don't have to worry about ranges.
	//
	else
	{
		if (!(pText->lFlags & SS_LOADTEXT_ROWHEADERS))
      pText->lRow = lpSS->Row.HeaderCnt;

		if (!(pText->lFlags & SS_LOADTEXT_COLHEADERS))
      pText->lCol = lpSS->Col.HeaderCnt;

	}

	return;
}
*/
// jb - 24.jan.01 -------------------------------------------------------------
//
// Populate our structure with our parameters passed in.
//
static BOOL	InitSS_Text (SS_TEXT *pText, LPCTSTR pcszFile, LPCTSTR pcszCellDelim, LPCTSTR pcszColDelim, LPCTSTR pcszRowDelim, long lFlags, LPCTSTR pcszLogFile, BOOL bLoadText)
{
	BOOL bRet = TRUE;
	TCHAR tcFormat[BUFSIZ*10];
	LPTSTR pTextName;
	int len;

	_fmemset(pText, '\0', sizeof(SS_TEXT));

	pText->bLoadText = bLoadText;
	pText->lLastCol = 0;
	pText->pcszCellDelim = pcszCellDelim;
	pText->pcszColDelim  = pcszColDelim;
	pText->pcszRowDelim  = pcszRowDelim;
	pText->lFlags = lFlags;

	// Log file
	//
	if (pcszLogFile == NULL || _tcslen (pcszLogFile) == 0)
	{	pText->pszLogFile = 0;
		pText->ghLogFile = 0;
	}
	else
//GAB 12/28/01 modified for Unicode
//  {	len = (_tcslen (pcszLogFile) + 1) * sizeof (TCHAR);
//		pText->ghLogFile = GlobalAlloc (GHND, len);
//		pText->pszLogFile = GlobalLock (pText->ghLogFile);
//		_tcsncpy (pText->pszLogFile, pcszLogFile, len-1);
//		pText->pszLogFile[len-1] = 0;

	{	len = (int)_tcslen (pcszLogFile);
		pText->ghLogFile = GlobalAlloc (GHND, (len + 1) * sizeof(TCHAR));
		pText->pszLogFile = GlobalLock (pText->ghLogFile);
		_tcsncpy (pText->pszLogFile, pcszLogFile, len);
		pText->pszLogFile[len] = 0;
	}
	pText->tcBuf[0] = 0;

	// The ones that set bRet to false, are needed for our method.
	//
	if (pcszFile == NULL || *pcszFile == '\0')
	{
       LoadString((HINSTANCE)fpInstance, IDS_LOGS_INVALIDFILE, tcFormat, BUFSIZ*10-1);
		_stprintf (pText->tcBuf, tcFormat, pcszFile);

		bRet = FALSE;
	}

	pTextName = ValidateFileName(pcszFile, NULL, _T(".txt"));
	lstrcpy(pText->szFile, pTextName);
	if (pTextName)
		free (pTextName);

	// Cell delimiter
	//
	if (pcszCellDelim == NULL || _tcslen (pText->pcszCellDelim) == 0)
	{	pText->pcszCellDelim = NULL;
	}

	// Column delimiter
	//
	if (pcszColDelim == NULL || _tcslen (pText->pcszColDelim) == 0)
	{	pText->pcszColDelim = NULL;
		LoadString((HINSTANCE)fpInstance, IDS_LOGS_TEXT_NOCOLDELIMITER, tcFormat, BUFSIZ*10-1);
		_stprintf (pText->tcBuf, tcFormat);
//		bRet = FALSE;
	}

	// Row delimiter
	//
	if (pcszRowDelim == NULL || _tcslen (pText->pcszRowDelim) == 0)
	{	pText->pcszRowDelim = NULL;
		LoadString((HINSTANCE)fpInstance, IDS_LOGS_TEXT_NOROWDELIMITER, tcFormat, BUFSIZ*10-1);
		_stprintf (pText->tcBuf, tcFormat);

		// Default row delimiter to Carriage return/line feed if not specified.
		//
		pText->pcszRowDelim = _T("\r\n");
	}

	if (pText->pszLogFile != 0)
	{	
    _tremove (pText->pszLogFile);
	}

	if (pText->tcBuf[0] != 0)
	{	write2log_text (pText);
	}

	return bRet;
}

//GAB 01/21/02 THESE FUNCTIONS ARE OBSOLETE NOW
/*
// jb - 18.oct.00 -------------------------------------------------------------
//
// The cell delimiter will be put on either side of the data. If the delimiter is 
// double quotes, then the data John will be "John"
//
static void WriteCellDelimiter (SS_TEXT *pText, FILE *f)
{
	if (pText->pcszCellDelim)
	 	_fputts (pText->pcszCellDelim, f);
	return;
}

// jb - 18.oct.00 -------------------------------------------------------------
//
// The column delimiter will be between the data.  If the cell delimiter is double
// quotes, column delimiter is a comma then "John""Beckwith" would become 
// "John", "Beckwith", etc.
//
static void WriteColDelimiter (SS_TEXT *pText, FILE *f)
{	
  if (pText->pcszColDelim)
    _fputts (pText->pcszColDelim, f);
	return;
}

// jb - 18.oct.00 -------------------------------------------------------------
//
// The row delimiter probably won't be used much.  If left null, then a carriage
// return linefeed is written.  Otherwise, at the end of each row, will be the
// user defined delimiter.
//
static void WriteRowDelimiter (SS_TEXT *pText, FILE *f)
{
	if (pText->pcszRowDelim)
    _fputts (pText->pcszRowDelim, f);
  else
    _fputts (_T("\r\n"),f);    
	return;
}
OBSOLETE FUNCTIONS */

// jb - 27.oct.98 ---------------------------------------------------------
//
// Had a problem when the password field was being exported, it was was showing 
// up in the html table!  Turn it back into '*****'
//
BOOL dontshowpassword (LPSPREADSHEET lpSS, SS_COORD lCol, SS_COORD lRow, LPTSTR lptstrBuf)
{
	SS_CELLTYPE cellType;

	if (SS_RetrieveCellType (lpSS, &cellType, NULL, lCol, lRow))
	{	if (SS_TYPE_EDIT == cellType.Type && (cellType.Style & ES_PASSWORD))
		{
      // Don't let the password get exported !  Change it to '*' when exporting.
    	int len = lstrlen(lptstrBuf);
      int i;
      for (i = 0; i < len; i++)
      {
	      lptstrBuf[i] = _T('*');
      }
      lptstrBuf[i] = 0;
    }
	}

	return TRUE;
}


static void DataOut(LPTSTR pszCellData, SS_TEXT *pText, LPTSTR pszData, long *lTotalLen, long *lAllocLen, TBGLOBALHANDLE *ghData, long lAllocInc)
{
  LPTSTR pszCellDelim = NULL;
  LPTSTR pszColDelim = NULL;
  LPTSTR pszRowDelim = NULL;
  TCHAR  szCellDelim[100];
  BOOL   fCellDelim = FALSE;
  BOOL   fIsThereACellDelim = TRUE;

  if (pText->pcszCellDelim && *pText->pcszCellDelim)
    _tcscpy(szCellDelim, pText->pcszCellDelim);
  else if (pText->pcszColDelim || pText->pcszRowDelim)
    _tcscpy(szCellDelim, _T("\""));
  else
    fIsThereACellDelim = FALSE;

  if (fIsThereACellDelim)
  {
    pszCellDelim = _tcsstr(pszCellData, szCellDelim);

    if (pszCellDelim)
      fCellDelim = TRUE;

    if (pText->pcszColDelim && lstrlen(pText->pcszColDelim))
      pszColDelim = _tcsstr(pszCellData, pText->pcszColDelim);

    if (pText->pcszRowDelim && lstrlen(pText->pcszRowDelim))
      pszRowDelim = _tcsstr(pszCellData, pText->pcszRowDelim);

    if (pszRowDelim || pszColDelim || fCellDelim)
    {
//GAB      _fputts(szCellDelim, f);
       pszData = (LPTSTR)SS_TextBufferAlloc(*lTotalLen, (long)_tcslen(szCellDelim) * sizeof(TCHAR), lAllocLen, ghData, lAllocInc);
       _tcscat(pszData, szCellDelim);
       *lTotalLen += (long)_tcslen(szCellDelim) * sizeof(TCHAR);
    }    
  }
  
  if (!pszCellDelim)
  {
    // Write out the cell data
//GAB    _fputts (pszData,  f);
    pszData = (LPTSTR)SS_TextBufferAlloc(*lTotalLen, (long)_tcslen(pszCellData) * sizeof(TCHAR), lAllocLen, ghData, lAllocInc);
    _tcscat(pszData, pszCellData);
    *lTotalLen += (long)_tcslen(pszCellData) * sizeof(TCHAR);
  }

  // If a cell delimiter exists in the cell data, we need to write out 2-cell delimiters.
  while (pszCellDelim)
  {
    // Insert a null terminator to write out the data up-to the cell delimiter.
    *pszCellDelim = (TCHAR)0;

    // Write out the cell data
//GAB    _fputts(pszData,  f);
    pszData = (LPTSTR)SS_TextBufferAlloc(*lTotalLen, (long)_tcslen(pszCellData) * sizeof(TCHAR), lAllocLen, ghData, lAllocInc);
    _tcscat(pszData, pszCellData);
    *lTotalLen += (long)_tcslen(pszCellData) * sizeof(TCHAR);

//GAB    _fputts(szCellDelim, f);
    pszData = (LPTSTR)SS_TextBufferAlloc(*lTotalLen, (long)_tcslen(szCellDelim) * sizeof(TCHAR), lAllocLen, ghData, lAllocInc);
    _tcscat(pszData, szCellDelim);
    *lTotalLen += lstrlen(szCellDelim) * sizeof(TCHAR);
//GAB    _fputts(szCellDelim, f);
    pszData = (LPTSTR)SS_TextBufferAlloc(*lTotalLen, (long)_tcslen(szCellDelim) * sizeof(TCHAR), lAllocLen, ghData, lAllocInc);
    _tcscat(pszData, szCellDelim);
    *lTotalLen += (long)_tcslen(szCellDelim) * sizeof(TCHAR);
    
    // Advance the cell data pointer past the inserted null
//GAB 01/05/02 Changed for Unicode
    //pszData = pszCellDelim + lstrlen(szCellDelim)*sizeof(TCHAR);
    pszCellData = pszCellDelim + lstrlen(szCellDelim);
    
    // Check for any additional cell delimiters
    pszCellDelim = _tcsstr(pszCellData, szCellDelim);
  }
  if (fCellDelim)
  {  
    // Add the rest of the data to the file.  
//GAB    _fputts(pszData, f);
    pszData = (LPTSTR)SS_TextBufferAlloc(*lTotalLen, (long)_tcslen(pszCellData) * sizeof(TCHAR), lAllocLen, ghData, lAllocInc);
    _tcscat(pszData, pszCellData);
    *lTotalLen += (long)_tcslen(pszCellData) * sizeof(TCHAR);
  }

  /* RFW - 6/25/03 - 12024    
  if (pszColDelim || fCellDelim)
  */
  if (pszRowDelim || pszColDelim || fCellDelim)
  {
//GAB    _fputts(szCellDelim, f);
    pszData = (LPTSTR)SS_TextBufferAlloc(*lTotalLen, (long)_tcslen(szCellDelim) * sizeof(TCHAR), lAllocLen, ghData, lAllocInc);
    _tcscat(pszData, szCellDelim);
    *lTotalLen += (long)_tcslen(szCellDelim) * sizeof(TCHAR);
  }          
}

// jb - 20.jul.00 -------------------------------------------------------------
//
// Write out the column headers to a text file if there is one.
//

BOOL GetDefHeaderData (LPSPREADSHEET lpSS, BOOL fCol, SS_COORD lCol, SS_COORD lRow, LPTSTR pszData)
{
  SS_COORD lVal;
	WORD wStyle;
	TCHAR tcLocalBuf[100];
	SS_COORD StartRowNum, StartColNum;

  SSGetStartingNumbers (lpSS->lpBook->hWnd, &StartColNum, &StartRowNum);
  StartColNum -= lpSS->Col.HeaderCnt - 1;
  StartRowNum -= lpSS->Row.HeaderCnt - 1;

  if (fCol)
  {
    SS_COORD lHeaderRow = lpSS->Row.lHeaderDisplayIndex >= 0 &&
                          lpSS->Row.lHeaderDisplayIndex < lpSS->Row.HeaderCnt ?
                          lpSS->Row.lHeaderDisplayIndex : lpSS->Row.HeaderCnt - 1;

    if (lRow != lHeaderRow)
      return TRUE;

    lVal = lCol + StartColNum-1;

	  wStyle = SSGetColHeaderDisplay (lpSS->lpBook->hWnd);
  }
  else // Row Header
  {
    SS_COORD lHeaderCol = lpSS->Col.lHeaderDisplayIndex >= 0 &&
                          lpSS->Col.lHeaderDisplayIndex < lpSS->Col.HeaderCnt ?
                          lpSS->Col.lHeaderDisplayIndex : lpSS->Col.HeaderCnt - 1;

    if (lCol != lHeaderCol)
      return TRUE;

    lVal = lRow + StartRowNum-1;

	  wStyle = SSGetRowHeaderDisplay (lpSS->lpBook->hWnd);
  }

  if (wStyle == SS_HEADERDISPLAY_NUMBERS)
		_tcscpy (pszData, (LPTSTR)_itot(lVal, tcLocalBuf, 10));

	else if (wStyle == SS_HEADERDISPLAY_LETTERS)
		SS_DrawFormatHeaderLetter (pszData, lVal);

	return TRUE;
}

TBGLOBALHANDLE SaveTextFile (LPSPREADSHEET lpSS, SS_TEXT *pText, PTEXT_COORD pTextCOORD, long lFlags, long *lplLength)
{
  SS_COORD lColInit, lRowInit;
  SS_COORD lCol, lRow;
  SS_COORD lColMax;
  SS_COORD lRowMax;
  long lAllocLen = 0;
  long lAllocLenCellData = 0;
  long lAllocInc = 4000;
  TBGLOBALHANDLE ghData = 0;
  TBGLOBALHANDLE ghCellData = 0;
  LPTSTR pszData = NULL;
  LPTSTR pszCellData = NULL;
  LPTSTR pszDelim = NULL;
  long lLen = 0;
  long lTotalLen = 0;
  long lTotalCellDataLen = 0;


// fix for bug 9607 -scl
  SS_GetDataCnt(lpSS, &lColMax, &lRowMax);
// fix for bug 9612 -scl

  // search for data in header cells
  {
    SS_COORD c, r;
    // first search col headers
    for( r = 0; r < lpSS->Row.HeaderCnt; r++ )
      for( c = max(lpSS->Col.HeaderCnt, lColMax); c < lpSS->Col.AllocCnt; c++ )
        if( SS_GetDataLen(lpSS, c, r) )
        {
          lColMax = max(lColMax, c + 1);
          lRowMax = max(lRowMax, r + 1);
        }
    // then search row headers
    for( c = 0; c < lpSS->Col.HeaderCnt; c++ )
      for( r = max(lpSS->Row.HeaderCnt, lRowMax); r < lpSS->Row.AllocCnt; r++ )
        if( SS_GetDataLen(lpSS, c, r) )
        {
          lColMax = max(lColMax, c + 1);
          lRowMax = max(lRowMax, r + 1);
        }
  }
  // Keep in mind that with multiple levels of headers there are 4 areas 
  // to be concerned with so far as indexing: D-data area, CH-column header area within data columns,
  // RH-row header area within data rows, XX-row&column header area within row&column header area
  //
  // ------------------------
  // |XX|XX|CH|CH|CH|CH|CH|
  // |-----------------------
  // |XX|XX|CH|CH|CH|CH|CH|
  // |-----------------------
  // |RH|RH| D| D| D| D| D|
  // |-----------------------
  // |RH|RH| D| D| D| D| D|
  // |-----------------------
  // |RH|RH| D| D| D| D| D|

  // Initially, set the beginning indices to the start of the data
  lColInit = lpSS->Col.HeaderCnt;
  lRowInit = lpSS->Row.HeaderCnt;

  // If we need to get the col headers, then change the initial row
  if (lFlags & SS_EXPORTTEXT_COLHEADERS)
    lRowInit = 0;
  // If we need to get the row headers, then change the initial col
  if (lFlags & SS_EXPORTTEXT_ROWHEADERS)
    lColInit = 0;

//GAB 01/23/02 Added to fix 9720.
  if ((-1 != pTextCOORD->lRowEnd) && (0 != pTextCOORD->lRowEnd))
    lRowMax = pTextCOORD->lRowEnd + 1;
  if ((-1 != pTextCOORD->lColEnd) && (0 != pTextCOORD->lColEnd))
    lColMax = pTextCOORD->lColEnd + 1;

#ifdef SS_V40
  if (!(lFlags & SS_EXPORTTEXT_CREATE) && (lFlags & SS_EXPORTTEXT_APPEND) &&
      pText->pcszRowDelim && *pText->pcszRowDelim)
  {
    pszDelim = (LPTSTR)pText->pcszRowDelim;
    pszData = (LPTSTR)SS_TextBufferAlloc(lTotalLen, lstrlen(pszDelim) * sizeof(TCHAR), &lAllocLen, &ghData, lAllocInc);
    _tcscat(pszData, pszDelim);
    lTotalLen += lstrlen(pszDelim) * sizeof(TCHAR);
    pszDelim = NULL;
  }
#endif //SS_V40

  for (lRow = lRowInit; lRow < lRowMax; lRow++)
  {
    // If we're done with the Col headers, jump to beginning of selected range
    if ((lRow == lpSS->Row.HeaderCnt)  && (-1 != pTextCOORD->lRowStart))
      lRow = max(lpSS->Row.HeaderCnt, pTextCOORD->lRowStart);

    for (lCol = lColInit; lCol < lColMax; lCol++)
    {
    // If we're done with the Row headers, jump to beginning of selected range
      if ((lCol == lpSS->Col.HeaderCnt) && (-1 != pTextCOORD->lColStart))
        lCol = max(lpSS->Col.HeaderCnt, pTextCOORD->lColStart);

      if (lFlags & SS_EXPORTTEXT_UNFORMATTED)
        lLen = (SS_GetValueLen(lpSS, lCol, lRow)); 
      else
        lLen = (SS_GetDataLen(lpSS, lCol, lRow));

      lTotalCellDataLen = 0;
      pszCellData = (LPTSTR)SS_TextBufferAlloc(lTotalCellDataLen, max(2, (lLen + 1)) * sizeof(TCHAR), &lAllocLenCellData, &ghCellData, lAllocInc);
      
      if (lFlags & SS_EXPORTTEXT_UNFORMATTED)
        SS_GetValueEx (lpSS, lCol, lRow, pszCellData, -1);
      else
        SS_GetDataEx (lpSS, lCol, lRow, pszCellData, -1);

      if (pszCellData && !*pszCellData && ((lCol < lpSS->Col.HeaderCnt && lRow >= lpSS->Row.HeaderCnt) ||
          (lRow < lpSS->Row.HeaderCnt && lCol >= lpSS->Col.HeaderCnt)))
      {
        pszCellData = (LPTSTR)SS_TextBufferAlloc(lTotalCellDataLen, 20 * sizeof(TCHAR), &lAllocLenCellData, &ghCellData, lAllocInc);
        GetDefHeaderData (lpSS, (lCol < lpSS->Col.HeaderCnt)?FALSE:TRUE, lCol, lRow, pszCellData);
      }

      // Don't write out checkbox value if it is 0
      if (lRow >= lpSS->Row.HeaderCnt && lCol >= lpSS->Col.HeaderCnt)
      {
        if (ischeckbox(lpSS, lCol, lRow))
        {
          // RFW - 2/21/06 - 18263
          if (lFlags & SS_EXPORTTEXT_CHECKBOXFALSE)
          {
            if (pszCellData && (*pszCellData == '\0'))
            {
              _tcscpy (pszCellData, _T("0"));
              lLen += sizeof(TCHAR);
            }
          }
          else
          {
            if (pszCellData && (*pszCellData == (TCHAR)'0'))
            {
              _tcscpy (pszCellData, _T(""));
              lLen -= sizeof(TCHAR);
            }
          }
        }
      }

    lTotalCellDataLen += lLen * sizeof(TCHAR);

    // Don't write out the password.
    dontshowpassword (lpSS, lCol, lRow, pszCellData);

    // Write cell delimiter on either side of cell.
    //
    if (pszCellData && *pszCellData)
    {
// GAB 03/27/02 - 9884 Don't write cell delimiter if this is a tab file
//      if (pText->pcszColDelim)
// GAB 05/21/02 - 10179 Learn how to program Greg!    if ((pText->pcszColDelim) && !(lFlags && SS_EXPORTTEXT_TABFILE))
      if ((pText->pcszColDelim) && !(lFlags & SS_EXPORTTEXT_TABFILE))
        DataOut(pszCellData, pText, pszData, &lTotalLen, &lAllocLen, &ghData, lAllocInc);
      else
      {
        pszData = (LPTSTR)SS_TextBufferAlloc(lTotalLen, lstrlen(pszCellData) * sizeof(TCHAR), &lAllocLen, &ghData, lAllocInc);
        _tcscat(pszData, pszCellData);
        lTotalLen += lstrlen(pszCellData) * sizeof(TCHAR);
      }
		}

      // Write column delimiter in-between cells, but not after the last
			// column.
			//
      if (lCol != lColMax-1)        //Add Col Delimiter
        pszDelim = (LPTSTR)pText->pcszColDelim; 
#ifdef SS_V40
      else if (lRow != lRowMax-1)  //GAB don't write an extra row delimiter at the end if SS_V40
#else
      else
#endif //SS_V40
        pszDelim = (LPTSTR)pText->pcszRowDelim;

      if (pszDelim)
      {
        pszData = (LPTSTR)SS_TextBufferAlloc(lTotalLen, lstrlen(pszDelim) * sizeof(TCHAR), &lAllocLen, &ghData, lAllocInc);
        _tcscat(pszData, pszDelim);
        lTotalLen += lstrlen(pszDelim) * sizeof(TCHAR);
        pszDelim = NULL;
      }

    } // end of Col
  } // end of Row

  
  if (ghData)
    tbGlobalUnlock(ghData);

  if (ghCellData)
  {
    tbGlobalUnlock(ghCellData);
    tbGlobalFree(ghCellData);
  }

  if (lplLength)
    *lplLength = lTotalLen;

  return ghData;
}


// jb - 16.may.01 -------------------------------------------------------------
//
// Put information in our log file indicating what type of file we are creating.
//
static void HeaderLog_Text (SS_TEXT *pText, long lFlags)
{
	TCHAR tcFormat[BUFSIZ*10];
	TCHAR tcTimeBuf[32];

	LoadString((HINSTANCE)fpInstance, IDS_LOGS_HEAD1_TEXT, tcFormat, BUFSIZ*10-1);
	_stprintf (pText->tcBuf, tcFormat, getdatetimestring (tcTimeBuf), pText->szFile);
	write2log_text (pText);

	if ((lFlags & SS_EXPORTTEXT_UNFORMATTED) && (lFlags & SS_EXPORTTEXT_CREATE))
	{	LoadString((HINSTANCE)fpInstance, IDS_LOGS_TEXT_UNFORMATCREATE, tcFormat, BUFSIZ*10-1);
	   	_stprintf (pText->tcBuf, tcFormat, pText->szFile);
	}
	else if ((lFlags & SS_EXPORTTEXT_UNFORMATTED) && (lFlags & SS_EXPORTTEXT_APPEND))
	{	LoadString((HINSTANCE)fpInstance, IDS_LOGS_TEXT_UNFORMATAPPEND, tcFormat, BUFSIZ*10-1);
	   	_stprintf (pText->tcBuf, tcFormat, pText->szFile);
	}
	else if (lFlags & SS_EXPORTTEXT_CREATE)
	{	LoadString((HINSTANCE)fpInstance, IDS_LOGS_TEXT_CREATE, tcFormat, BUFSIZ*10-1);
	   	_stprintf (pText->tcBuf, tcFormat, pText->szFile);
	}
	else if (lFlags == SS_EXPORTTEXT_APPEND)
	{	LoadString((HINSTANCE)fpInstance, IDS_LOGS_TEXT_APPEND, tcFormat, BUFSIZ*10-1);
	   	_stprintf (pText->tcBuf, tcFormat, pText->szFile);
	}
	else
	{	LoadString((HINSTANCE)fpInstance, IDS_LOGS_TEXT_UNKNOWNFLAG, tcFormat, BUFSIZ*10-1);
	   	_stprintf (pText->tcBuf, tcFormat);
	}

	write2log_text (pText);	

	LoadString((HINSTANCE)fpInstance, IDS_LOGS_HEAD3, tcFormat, BUFSIZ*10-1);
	_stprintf (pText->tcBuf, tcFormat);
	write2log_text (pText);

	LoadString((HINSTANCE)fpInstance, IDS_LOGS_HEAD4, tcFormat, BUFSIZ*10-1);
	_stprintf (pText->tcBuf, tcFormat);
	write2log_text (pText);

	return;
}

// jb - 17.may.01 -------------------------------------------------------------
//
// Write what happened after we tried to save the spread cells to a text file.
//
static void writefooter (SS_TEXT *pText, TEXT_COORD *pWEB_COORD, BOOL bRet)
{
	TCHAR tcFormat[BUFSIZ*10];

	if (bRet)
	{	LoadString((HINSTANCE)fpInstance, IDS_LOGS_SUCCESSEXPORT_TEXT, tcFormat, BUFSIZ*10-1);
	   	_stprintf (pText->tcBuf, tcFormat, pWEB_COORD->lColStart, pWEB_COORD->lColEnd, pWEB_COORD->lRowStart, pWEB_COORD->lRowEnd, pText->szFile);
		write2log_text (pText);
	}
	else
	{	LoadString((HINSTANCE)fpInstance, IDS_LOGS_FAILEDEXPORT, tcFormat, BUFSIZ*10-1);
	   	_stprintf (pText->tcBuf, tcFormat, pText->szFile);
	}

	LoadString((HINSTANCE)fpInstance, IDS_LOGS_FOOTER_TEXT, tcFormat, BUFSIZ*10-1);
   	_stprintf (pText->tcBuf, tcFormat);
	write2log_text (pText);

	return;
}

//GAB 01/17/02 - Made changes throughout this function for Unicode.
// The new method will save a text file with any delimiter (string) for cells, columns, rows, and sheets.
// The lFlags, can be set to SPRD_methodname_CREATE, SPRD_methodname_APPEND, or
// SPRD_methodname_UNFORMATTED.
// _CREATE - (default) creates a new file, over-writes existing one.
// _APPEND - creates a new file if doesn't exist, otherwise, appends to existing one.
// _UNFORMATTED (default is formatted) data is saved unformatted.
//
BOOL SS_ExportRangeToTextFile (LPSPREADSHEET lpSS, SS_COORD lCol, SS_COORD lRow, SS_COORD lCol2, SS_COORD lRow2, LPCTSTR pcszFile, LPCTSTR pcszCellDelim, LPCTSTR pcszColDelim, LPCTSTR pcszRowDelim, long lFlags, LPCTSTR pcszLogFile, BOOL fUnicode)
{
	BOOL bRet = FALSE;
	FILE *f;
	TCHAR mode[6];
	TEXT_COORD textcoord;
	SS_TEXT text;
	TCHAR tcFormat[BUFSIZ*10];
  TBGLOBALHANDLE ghData, ghDataOut;
  short nBOF = (short)BYTE_ORDER_MARK;
  long lInBuffLen;
  LPTSTR pszData;
  DWORD dwOutBuffLen = 0;

	// Allocate then store our parameters passed in to the method.  Put 
	// them in SS_TEXT text;
	//
	if (!InitSS_Text (&text, pcszFile, pcszCellDelim, pcszColDelim, pcszRowDelim, lFlags, pcszLogFile, 0))
    return bRet;

	HeaderLog_Text (&text, lFlags);

	// If create flag, over-write existing file or create new one.
	//
	if (lFlags & SS_EXPORTTEXT_APPEND) 
		_tcsncpy (mode, _T("ab"), 2);
	else
		_tcsncpy (mode, _T("wb"), 2);

	// If for some reason both create and append are OR'ed, then default to create.
	//
	if ( (lFlags & SS_EXPORTTEXT_CREATE) && (lFlags & SS_EXPORTTEXT_APPEND))
		_tcsncpy (mode, _T("wb"), 2);

	mode[2] = 0;

	if (!(f = _tfopen (text.szFile, mode)))
	{LoadString((HINSTANCE)fpInstance, IDS_LOGS_CANTOPENFILE, tcFormat, BUFSIZ*10-1);
		_stprintf (text.tcBuf, tcFormat, text.szFile);
		write2log_text (&text);

 		return bRet;
	}
//GAB 01/23/02 Added for fix 9720
	textcoord.lColStart	= lCol;
	textcoord.lColEnd		= lCol2;
	textcoord.lRowStart	= lRow;
	textcoord.lRowEnd		= lRow2;

  ghData = SaveTextFile(lpSS, &text, &textcoord, lFlags, &lInBuffLen);
  if ( ghData )
  {
    bRet = TRUE;
    pszData = (LPTSTR)tbGlobalLock(ghData);

//GAB 01/10/02 Save file based on fUnicode flag.
    #if defined(UNICODE) || defined(_UNICODE)
      if ( !fUnicode )  //Convert file from Unicode to ANSI
      {
//        ghDataOut = Convert( CP_ACP, pszData, lInBuffLen * sizeof(TCHAR), TRUE, &dwOutBuffLen );
        ghDataOut = Convert( CP_ACP, pszData, lInBuffLen, TRUE, &dwOutBuffLen );
        tbGlobalUnlock(ghData);
        tbGlobalFree(ghData);
        pszData = (LPTSTR)tbGlobalLock(ghDataOut);
        fwrite(pszData, fUnicode ? sizeof(WCHAR) : sizeof(char), dwOutBuffLen, f);
        tbGlobalUnlock(ghDataOut);
        tbGlobalFree(ghDataOut);
      }
      else  //Unicode - keep file Unicode
      {
        fwrite(&nBOF, sizeof(short), 1, f);
        _fputts(pszData, f);
        tbGlobalUnlock(ghData);
        tbGlobalFree(ghData);
      }
    #else  
      if ( fUnicode ) //Convert file from ANSI to Unicode
      {
        ghDataOut = Convert( CP_ACP, pszData, lInBuffLen * sizeof(TCHAR), FALSE, &dwOutBuffLen );
        tbGlobalUnlock(ghData);
        tbGlobalFree(ghData);
        pszData = (LPTSTR)tbGlobalLock(ghDataOut);
        fwrite(pszData, fUnicode ? sizeof(WCHAR) : sizeof(char), dwOutBuffLen, f);
        tbGlobalUnlock(ghDataOut);
        tbGlobalFree(ghDataOut);
      }
      else  //ANSI - keep file ANSI
      {
        fwrite(pszData, sizeof(char), lInBuffLen, f);
        tbGlobalUnlock(ghData);
        tbGlobalFree(ghData);
      }
    #endif
  }
	writefooter (&text, &textcoord, bRet);

  if (f)
	{	fclose (f);
	}

	return bRet;
}

// jb - 2.may.01 --------------------------------------------------------------
//
// Export a range of spread cells to store in a text file
//
BOOL DLLENTRY SSExportRangeToTextFile (HWND hWnd, SS_COORD lCol, SS_COORD lRow, SS_COORD lCol2, SS_COORD lRow2, LPCTSTR pcszFile, LPCTSTR pcszCellDelim, LPCTSTR pcszColDelim, LPCTSTR pcszRowDelim, long lFlags, LPCTSTR pcszLogFile)
{
	LPSPREADSHEET lpSS;
	BOOL          bRet;

	lpSS = SS_SheetLock (hWnd);
//GAB 01/10/02 Added the last parameter (fUnicode) and set it to FALSE because this is the 
//ANSI version of SSExportRangeToTextFile
	bRet = SS_ExportRangeToTextFile (lpSS, lCol, lRow, lCol2, lRow2, pcszFile, pcszCellDelim, pcszColDelim, pcszRowDelim, lFlags, pcszLogFile, FALSE);
	SS_SheetUnlock (hWnd);

	return bRet;
}

// jb - 2.may.01 --------------------------------------------------------------
//
// Export spread data to a Text file
//
BOOL DLLENTRY SSExportToTextFile (HWND hWnd, LPCTSTR pcszFile, LPCTSTR pcszCellDelim, LPCTSTR pcszColDelim, LPCTSTR pcszRowDelim, long lFlags, LPCTSTR pcszLogFile)
{	
	return SSExportRangeToTextFile (hWnd, SS_ALLCOLS, SS_ALLROWS, 0, 0, pcszFile, pcszCellDelim, pcszColDelim, pcszRowDelim, lFlags, pcszLogFile);
}

//GAB 01/09/02
// Added the following two functions SSExportToTextFileU and SSExportRangeToTextFileU for 
// Unicode compatibility.
#ifdef SS_V40
BOOL DLLENTRY SSExportRangeToTextFileU (HWND hWnd, SS_COORD lCol, SS_COORD lRow, SS_COORD lCol2, SS_COORD lRow2, LPCTSTR pcszFile, LPCTSTR pcszCellDelim, LPCTSTR pcszColDelim, LPCTSTR pcszRowDelim, long lFlags, LPCTSTR pcszLogFile)
{
	LPSPREADSHEET lpSS;
	BOOL          bRet;

	lpSS = SS_SheetLock (hWnd);
	bRet = SS_ExportRangeToTextFile (lpSS, lCol, lRow, lCol2, lRow2, pcszFile, pcszCellDelim, pcszColDelim, pcszRowDelim, lFlags, pcszLogFile, TRUE);
	SS_SheetUnlock (hWnd);

	return bRet;
}

BOOL DLLENTRY SSExportToTextFileU (HWND hWnd, LPCTSTR pcszFile, LPCTSTR pcszCellDelim, LPCTSTR pcszColDelim, LPCTSTR pcszRowDelim, long lFlags, LPCTSTR pcszLogFile)
{	
	return SSExportRangeToTextFileU (hWnd, SS_ALLCOLS, SS_ALLROWS, 0, 0, pcszFile, pcszCellDelim, pcszColDelim, pcszRowDelim, lFlags, pcszLogFile);
}
#endif //SS_V40

/////////////////////////////////////////////////////////////
/////////////////////////////////////////////////////////////
// jb - 1.nov.00 --------------------------------------------------------------
//
// Get the text file and store it into a big buffer for parsing.  Another way to do
// this is to get chunks of this, then parse.  Depending on the delimiters that could
// be present in the text file, the parsing was extremely slow with fseek backwards
// for repositioning each time if the file pointer ended up in the middle of data in
// a record, or right on a delimiter or in the middle of a sheet delimiter.  For speed
// and size, ended up going with a straight buffer, loading the entire text file.
//
BOOL TextFile2Buffer (LPCTSTR pcszFile, GLOBALHANDLE *gh, long *lBufferLen)
{
	long dBytes;
	long lFileLen;
  LPBYTE pBuffer;

  int f;

	// If can't find file, complain.
	//
  f = _topen(pcszFile, _O_RDONLY | _O_BINARY);  

	if (-1 == f)
	{	return FALSE;
	}
	
  lFileLen = _filelength(f) + sizeof(TCHAR); //+ TCHAR for the NULL terminator
	*gh = GlobalAlloc(GHND, lFileLen);
  if (!*gh) 
  {
    _close (f);
	  return FALSE;
  }

  pBuffer = (LPBYTE)GlobalLock (*gh);
  dBytes = _read (f, pBuffer, lFileLen);

  _close(f);

	GlobalUnlock(*gh);

  *lBufferLen = dBytes;

	return TRUE;


/*
	long dBytes;
	FILE *f;

	// If can't find file, complain.
	//
	f = _tfopen (pcszFile, _T("rb"));
	if (!f)
	{	return FALSE;
	}
	
	*lBufferLen = 0;
	for (;;)
	{	if (!*hBuffer)
		{	if (!(*hBuffer = GlobalAlloc ((GMEM_MOVEABLE | GMEM_ZEROINIT), SS_BUFF_ALLOCINC)))
			{	fclose (f);
			    return FALSE;
			}
		}
		else
		{	GlobalUnlock (*hBuffer);
			if (!(*hBuffer = GlobalReAlloc (*hBuffer, (*lBufferLen + SS_BUFF_ALLOCINC), (GMEM_MOVEABLE | GMEM_ZEROINIT))))
			{	fclose (f);
				return FALSE;
			}
		}

		pBuffer = (LPTSTR)GlobalLock (*hBuffer);
		dBytes = fread (&pBuffer[*lBufferLen], 1, SS_BUFF_ALLOCINC, f);
		*lBufferLen += dBytes;

		if (feof (f))
		{	break;
		}

		if (ferror (f))
		{	fclose (f);
			return FALSE;
		}

		if (dBytes < SS_BUFF_ALLOCINC)
		{ 	break;
		}
	}

	GlobalUnlock(*hBuffer);
	fclose (f);

	return pBuffer;
*/
}


// jb - 16.may.01 --------------------------------------------------------------
//
// Write information to the log file about what we intend to do.
//
static void HeaderLog_TextLoad (SS_TEXT *pText)
{
	TCHAR tcFormat[BUFSIZ*10];
	TCHAR tcTimeBuf[32];

	LoadString((HINSTANCE)fpInstance, IDS_LOGS_HEAD1_LOAD, tcFormat, BUFSIZ*10-1);
	_stprintf (pText->tcBuf, tcFormat, getdatetimestring (tcTimeBuf));
	write2log_text (pText);

	LoadString((HINSTANCE)fpInstance, IDS_LOGS_HEAD3, tcFormat, BUFSIZ*10-1);
	_stprintf (pText->tcBuf, tcFormat);
	write2log_text (pText);
	LoadString((HINSTANCE)fpInstance, IDS_LOGS_HEAD4, tcFormat, BUFSIZ*10-1);
	_stprintf (pText->tcBuf, tcFormat);
	write2log_text (pText);

	return;
}

#if 0
// jb - 15.nov.00 -------------------------------------------------------------
//
// Put the data into the cell.
//
void ApplyData (LPSPREADSHEET lpSS, TCHAR *tcBuf, SS_TEXT *pText)
{
	int len;
	int i;

	len = _tcslen (tcBuf);

	if (len)
	{	SSSetValue (lpSS->lpBook->hWnd, pText->lCol, pText->lRow, tcBuf);
	}

	for (i = 0; i < len; i++)
	{	tcBuf[i] = 0;
	}
 
	return;
}

// jb - 7.nov.00 -------------------------------------------------------------
//
// At some point, we will come to a column delimiter (required) and we need to 
// apply the data to spread, then skip over the delimiter.
//
static BOOL IsColumnDelimiter (LPSPREADSHEET lpSS, LPTSTR *pptr, SS_TEXT *pText, long *i, TCHAR *tcBuf)
{
	BOOL bRet = FALSE;
	BOOL bFlag = TRUE;
	LPTSTR pTemp;
	LPTSTR ptr = *pptr;

	// We found a column delimiter.
	//
 	if (*ptr == *pText->pcszColDelim)
 	{	// See if our column delimiter is longer than one...
		//

		{	pTemp = (LPTSTR)pText->pcszColDelim;


      if (_tcsncmp(pText->pcszColDelim, ptr, lstrlen(pText->pcszColDelim)))
      {
        bFlag = FALSE;
      }

			// The first character was the same as our column delimiter, but
			// the rest were not.  False alarm...
			//
			if (bFlag)
			{	if (tcBuf[0] != 0)
				{	tcBuf[*i] = 0;
					ApplyData (lpSS, tcBuf, pText);
          *i = 0; 
          tcBuf[*i] = 0;
 					bRet = TRUE;
				}
			}
		}
	}

	*pptr = ptr;

	return bRet;
}

// jb - 29.jan.01 -------------------------------------------------------------
//
// If the cell delimiter is a string, we need to advance past it.
//
static BOOL	CheckCellDelimiter4String (SS_TEXT *pText, LPTSTR *pptr)
{
	BOOL bFlag = TRUE;
	BOOL bRet = TRUE;
	int len;
	int i;
	LPTSTR pTemp;
	LPTSTR ptr = *pptr;

	// We found a cell delimiter.
	//
 	if (*ptr == *pText->pcszCellDelim)
 	{	// See if our cell delimiter is longer than one...
		//
		len = _tcslen (pText->pcszCellDelim);
		if (len != 1)
		{ 	pTemp = (LPTSTR)pText->pcszCellDelim;
			// String delimiter must be greater than 1
			//
			for (i = 0; i < len; i++)
			{	if (*ptr++ == *pTemp++)
				{	continue;
				}
				else
				{	bRet = FALSE;
					break;
				}
			}
		}
		else
		{	ptr++;
		}
	}

	*pptr = ptr;

	return bRet;
}

// jb - 29.jan.01 -------------------------------------------------------------
//
// If the row delimiter is a string, we need to advance past it.
//
static BOOL CheckRowDelimiter4String (LPSPREADSHEET lpSS, LPTSTR *pptr, SS_TEXT *pText, long *plVal, TCHAR *tcBuf)
{
	BOOL bRet = FALSE;
	BOOL bFlag = TRUE;
	int len;
	int j;
	LPTSTR pTemp;
	LPTSTR ptr = *pptr;

	// We found a row delimiter.
	//
 	if (*ptr == *pText->pcszRowDelim)
 	{	// See if our row delimiter is longer than one...
		//
		len = _tcslen (pText->pcszRowDelim);
		if (len == 1)
		{	if (tcBuf[0] != 0)
			{	tcBuf[*plVal] = 0;
				ApplyData (lpSS, tcBuf, pText);
        *plVal = 0; 
        tcBuf[*plVal] = 0;				
        bRet = TRUE;
				ptr++;
			}
			else
			{	pText->lRow++;
				pText->lCol = lpSS->Col.HeaderCnt;
				ptr++;
			}
		}

		// String delimiter must be greater than 1
		//
		else
		{	pTemp = (LPTSTR)pText->pcszRowDelim;
			for (j = 0; j < len; j++)
			{	if (*ptr++ == *pTemp++)
				{	continue;
				}
				else
				{	bFlag = FALSE;
					break;
				}
			}

			// The first character was the same as our row delimiter, but
			// the rest were not.  False alarm...
			//
			if (bFlag)
			{	if (tcBuf[0] != 0)
				{	tcBuf[*plVal] = 0;
					ApplyData (lpSS, tcBuf, pText);
          *plVal = 0; 
          tcBuf[*plVal] = 0;
 					bRet = TRUE;
				}
				else
				{	pText->lRow++;
					pText->lCol = lpSS->Col.HeaderCnt;
				}
			}
		}
	}

	*pptr = ptr;

	return bRet;
}

#endif // 0

#define SS_DELIM_CELL 1
#define SS_DELIM_COL  2
#define SS_DELIM_ROW  3

// jb - 7.nov.00 -------------------------------------------------------------
//
// Load up a text file into spread.  The text file can have sheet, cell, column, and row delimiters.
// Only column and row delimiters are required.
// 
static BOOL LoadTextFile (LPSPREADSHEET lpSS, LPTSTR ptr, long lBufferLen, SS_TEXT *pText, long lFlags)
{
  long   lBufferSize = 512;
  TBGLOBALHANDLE ghData = tbGlobalAlloc(GHND, lBufferSize * sizeof(TCHAR));  // RFW
  LPTSTR pszData = (LPTSTR)tbGlobalLock(ghData);
  TBGLOBALHANDLE ghPtr = 0;
  long   lSrcPos = 0;
  long   lDestPos = 0;
  short  nCellDelim = 0;
  short  nColDelim = 0;
  short  nRowDelim = 0;
  short  nMaxDelim = 0;
  BOOL   fProcessingCellData = FALSE;
  BOOL   fInCell = FALSE;
  SS_COORD lCol = lpSS->Col.HeaderCnt;
  SS_COORD lColInit = lpSS->Col.HeaderCnt;
  SS_COORD lRow = lpSS->Row.HeaderCnt;
  SS_COORD lMaxRows = SSGetMaxRows(lpSS->lpBook->hWnd);
  SS_COORD lLastCol;
  TBGLOBALHANDLE ghCellDelim = (TBGLOBALHANDLE)0;
  LPTSTR         pszCellDelim = NULL;
  TBGLOBALHANDLE ghRowDelim = (TBGLOBALHANDLE)0;
  LPTSTR         pszRowDelim = NULL;
  BOOL   bRet = TRUE;
  DWORD dwIsUnicode;
  DWORD dwOutBuffLen = 0;
  WORD wDelim;
  short nDelimLen;
  BOOL fBOM;

//GAB - 7/7/04 Fixes 14228 and 14840
  if (0 == lBufferLen)
	 lLastCol = 0;
  else
	 lLastCol = lpSS->Col.HeaderCnt;

  //is there a cell delimiter?
  if (pText->pcszCellDelim && lstrlen(pText->pcszCellDelim))
  {
    pszCellDelim = (LPTSTR)pText->pcszCellDelim;
    nCellDelim = lstrlen(pText->pcszCellDelim);
  }
  else if (pText->pcszColDelim && lstrlen(pText->pcszColDelim) || 
           pText->pcszRowDelim && lstrlen(pText->pcszRowDelim))
  {
    ghCellDelim = tbGlobalAlloc(GHND, 2*sizeof(TCHAR));
    pszCellDelim = (LPTSTR)tbGlobalLock(ghCellDelim);
    pszCellDelim[0] = (TCHAR)'\"';
    nCellDelim = 1;
  }

  //is there a row delimiter?
  if (pText->pcszRowDelim && lstrlen(pText->pcszRowDelim))
  {  
    pszRowDelim = (LPTSTR)pText->pcszRowDelim;
    nRowDelim = lstrlen(pText->pcszRowDelim);
  }
  else
  {
    ghRowDelim = tbGlobalAlloc(GHND, 3*sizeof(TCHAR));
    pszRowDelim = (LPTSTR)tbGlobalLock(ghRowDelim);
    _tcscpy(pszRowDelim, _T("\r\n"));
    nRowDelim = 2;
  }

  //is there a col delimiter?
  if (pText->pcszColDelim && lstrlen(pText->pcszColDelim))
    nColDelim = lstrlen(pText->pcszColDelim);

  nMaxDelim = max(max(nCellDelim, nColDelim), nRowDelim);

  // If we need to get the col headers, then change the initial row
	if (lFlags & SS_LOADTEXT_COLHEADERS)
  {
    SS_SetColHeaderDisplay(lpSS, SS_HEADERDISPLAY_BLANK);
// fix for bug 9607 -scl
//    lRow = SS_HEADER;
    lRow = 0;
  }
  // If we need to get the row headers, then change the initial col
	if (lFlags & SS_LOADTEXT_ROWHEADERS)
  {  
    SS_SetRowHeaderDisplay(lpSS, SS_HEADERDISPLAY_BLANK);
// fix for bug 9607 -scl
//    lColInit = SS_HEADER;
    lColInit = 0;
  }

  lCol = lColInit;

  // 26753 -scl
  //#if defined(UNICODE) || defined(_UNICODE)
  { // RFW - 3/7/07 - 17473
  //int iFlags = IS_TEXT_UNICODE_ASCII16 | IS_TEXT_UNICODE_SIGNATURE;
  int iFlags = IS_TEXT_UNICODE_SIGNATURE;
  fBOM = IsTextUnicode(ptr, lBufferLen, &iFlags) != 0;
  iFlags = IS_TEXT_UNICODE_NOT_ASCII_MASK;
  dwIsUnicode = IsTextUnicode(ptr, lBufferLen, &iFlags);
  }
  //dwIsUnicode = IsTextUnicode(ptr, lBufferLen, NULL);
  //#else
  //{ // RFW - 10/26/05 - 17473, 17474, 17791
  //int iFlags = IS_TEXT_UNICODE_ASCII16 | IS_TEXT_UNICODE_SIGNATURE;
  //dwIsUnicode = IsTextUnicode(ptr, lBufferLen, &iFlags);
  //}
  //#endif

//  if ( !dwIsUnicode )
  {
    #if defined(UNICODE) || defined(_UNICODE)
      if ( !dwIsUnicode )  //Convert file from ANSI to Unicode
      {
        ghPtr = Convert( CP_ACP, ptr, lBufferLen, FALSE, &dwOutBuffLen );
        lBufferLen = dwOutBuffLen;
        ptr = (LPTSTR)tbGlobalLock(ghPtr);
      }
      // RFW - 1/29/05 - 14277
      else
        lBufferLen /= (long)sizeof(TCHAR);
      if( fBOM) // 26753 -scl
//GAB 7/30/04 Advance pointer past BOM here instead of below. 
	     lSrcPos += 1;
    #else
      if ( dwIsUnicode ) //Convert file from Unicode to ANSI
      {
        ghPtr = Convert( CP_ACP, ptr, lBufferLen * sizeof(TCHAR), TRUE, &dwOutBuffLen );
        lBufferLen = dwOutBuffLen;
        ptr = (LPTSTR)tbGlobalLock(ghPtr);
      }
	  else if ( IsFileMarkedUTF8(ptr, lBufferLen) )
	  {
		ptr = RemoveUTF8Marker (ptr, &lBufferLen);
	  }
    #endif
  } //end else

//GAB 01/07/02 Test buffer to see if it may be Unicode. If it is advance the pointer past
// the Byte Order Mark (BOM) to the first recognizable character.
//GAB 07/30/04 Moved the line to advance the pointer for Unicode files inside
// #if defined(UNICODE) above
//  dwIsUnicode = IsTextUnicode(ptr, lBufferLen, NULL);
//  if ( dwIsUnicode )
//    lSrcPos += 1;

  if (SS_GetRowCnt(lpSS) <= 1)
    SSSetMaxRows(lpSS->lpBook->hWnd, 1);

//GAB 01/04/02 Changed for Unicode
    //while (lSrcPos < lBufferLen)
/* RFW 6/2/04 - 14277
  while ((lSrcPos * (long)sizeof(TCHAR)) < lBufferLen)
*/
  while (lSrcPos < lBufferLen)
  {
    if (lCol-SS_HEADER == lpSS->Col.HeaderCnt)
      lCol = lpSS->Col.HeaderCnt;
    if (lRow-SS_HEADER == lpSS->Row.HeaderCnt)
      lRow = lpSS->Row.HeaderCnt;

    if ((long)((lDestPos+nMaxDelim+1)) > lBufferSize)                              // RFW
//    if ((long)((lDestPos+nMaxDelim+1)*sizeof(TCHAR)) > lBufferSize)
    {
      tbGlobalUnlock(ghData);
      lBufferSize += (nMaxDelim+1);
      ghData = tbGlobalReAlloc(ghData, lBufferSize * sizeof(TCHAR), GHND);          // RFW
//      ghData = tbGlobalReAlloc(ghData, lBufferLen, GHND);
      pszData = (LPTSTR)tbGlobalLock(ghData);
    }

		// RFW - -8/23/04 - 14902
		// This code was added to handle the case where similar characters are
		// used amongst the 3 delimeters.  ex) cell = "a", col = "ab", row = "abc".
		wDelim = 0;
		nDelimLen = 0;
      if (nCellDelim > nDelimLen && pszCellDelim &&
		    !_tcsncmp(pszCellDelim, &(ptr[lSrcPos]), nCellDelim))
			{
			wDelim = SS_DELIM_CELL;
			nDelimLen = nCellDelim;
			}

		if (nColDelim > nDelimLen && pText->pcszColDelim && 
					!_tcsncmp(&(ptr[lSrcPos]), pText->pcszColDelim, nColDelim))
			{
			wDelim = SS_DELIM_COL;
			nDelimLen = nColDelim;
			}

		if (nRowDelim > nDelimLen && pszRowDelim &&
		    !_tcsncmp(&(ptr[lSrcPos]), pszRowDelim, nRowDelim))
			{
			wDelim = SS_DELIM_ROW;
			nDelimLen = nRowDelim;
			}

		// GAB 03/28/02 - 9884 If this is a tab file, don't look for cell delimiters
      if (!fInCell && wDelim == SS_DELIM_CELL &&
          lDestPos == 0 && (!(lFlags & SS_LOADTEXT_TABFILE)))
      {
        // Entering quoted cell data -- this means that there are either embedded quote(s) or embedded col delimiter(s).
   /* RFW - 3/22/02 - 9884 - I moved the check for this up, to support older tab files that did not embed quotes.
        if (lDestPos != 0)
        {
          // if this is not the first char in the cell's data, return with error
          tbGlobalUnlock(ghData);
          tbGlobalFree(ghData);
          return FALSE;
        }
  */
        fInCell = TRUE;
        fProcessingCellData = TRUE;
        lSrcPos += nCellDelim;
      }
      else if (fInCell)
      {
        // if within the quotes
        if (!_tcsncmp(pszCellDelim, &(ptr[lSrcPos]), nCellDelim))
        {
          // if this character is a quote
          lSrcPos += nCellDelim;
          if (!_tcsncmp(pszCellDelim, &(ptr[lSrcPos]), nCellDelim))
          {
            // Embedded quote -- write one out...
            _tcsncpy(&(pszData[lDestPos]), &(ptr[lSrcPos]), nCellDelim);
            lDestPos += nCellDelim;
            lSrcPos += nCellDelim;
          }
          else
          {
            // if the char is not a quote, then we are exiting quoted cell data
            fInCell = FALSE;
            pszData[lDestPos] = 0;
            lDestPos = 0;
            fProcessingCellData = FALSE;

            // Write-out the data
  	        if (lstrlen(pszData))
            {
              if (lCol >= SS_GetColCnt(lpSS))
				    SSSetMaxCols(lpSS->lpBook->hWnd, lCol - (lpSS->Col.HeaderCnt - 1));

		        if (lFlags & SS_EXPORTTEXT_UNFORMATTED)
                SS_SetValue(lpSS, lCol, lRow, pszData);
				  else
                SS_SetData(lpSS, lCol, lRow, pszData);
            }
            memset(pszData, 0, lstrlen(pszData)*sizeof(TCHAR));

            if (!nColDelim)
            {
              lCol++;
              if (lCol > lLastCol)
                lLastCol = lCol;
            }
          }
        }
        else
        {
          pszData[lDestPos++] = ptr[lSrcPos++]; 
        }
      }
    else if (wDelim == SS_DELIM_COL)
    {
      if (!fInCell && fProcessingCellData)
      {
        pszData[lDestPos] = 0;
        lDestPos = 0;
        fProcessingCellData = FALSE;

        // Write-out the data
  	    if (lstrlen(pszData))
        {
          if (lCol >= SS_GetColCnt(lpSS))
				SSSetMaxCols(lpSS->lpBook->hWnd, lCol - (lpSS->Col.HeaderCnt - 1));

          if (lFlags & SS_EXPORTTEXT_UNFORMATTED)
            SS_SetValue(lpSS, lCol, lRow, pszData);
          else
            SS_SetData(lpSS, lCol, lRow, pszData);
        }
        memset(pszData, 0, lstrlen(pszData)*sizeof(TCHAR));
      }
      lSrcPos += nColDelim;
      lCol++;
      if (lCol > lLastCol)
        lLastCol = lCol;
      fProcessingCellData = FALSE;
    }
// GAB 03/27/02 - 9884
//    else if (nRowDelim && pszRowDelim && 
//             !_tcsncmp(&(ptr[lSrcPos]), pszRowDelim, nRowDelim))
// RFW - 11/5/03 - 12543
//    else if ((nRowDelim && pszRowDelim && !_tcsncmp(&(ptr[lSrcPos]), pszRowDelim, nRowDelim)) ||
//              ((lFlags & SS_LOADTEXT_TABFILE) && (ptr[lSrcPos] == '\r' || ptr[lSrcPos] == '\n')))
// RFW - 1/27/04 - 13628 - I put it back to the way it was
//    else if ((nRowDelim && pszRowDelim && !_tcsncmp(&(ptr[lSrcPos]), pszRowDelim, nRowDelim)) ||
//              ptr[lSrcPos] == '\r' || ptr[lSrcPos] == '\n')
    else if ((wDelim == SS_DELIM_ROW) ||
              ((lFlags & SS_LOADTEXT_TABFILE) && (ptr[lSrcPos] == '\r' || ptr[lSrcPos] == '\n')))
    {
      if (fProcessingCellData)
      {
        pszData[lDestPos+1] = 0;
        lDestPos = 0;
        fProcessingCellData = FALSE;

        // Write-out the data
  	    if (lstrlen(pszData))
        {
          if (lCol >= SS_GetColCnt(lpSS))
				SSSetMaxCols(lpSS->lpBook->hWnd, lCol - (lpSS->Col.HeaderCnt - 1));

          if (lFlags & SS_EXPORTTEXT_UNFORMATTED)
            SS_SetValue(lpSS, lCol, lRow, pszData);
          else
            SS_SetData(lpSS, lCol, lRow, pszData);
        }
        memset(pszData, 0, lstrlen(pszData)*sizeof(TCHAR));
      }
// GAB 04/03/02 - 9916 Added to accomodate old Tab delimited files where the row delimiter
// was just a carriage return
      if(!_tcsncmp(&(ptr[lSrcPos]), pszRowDelim, nRowDelim))
        lSrcPos += nRowDelim;
      else
		{
        lSrcPos++;
        // If we have a <CR> or <LF>, then see if there is a trailer.  If so, skip it.
        if (ptr[lSrcPos - 1] == '\r' && ptr[lSrcPos] == '\n')
          lSrcPos++;
		}

      lRow++;
      if (lRow > lMaxRows)
      {
        lMaxRows = lRow+10;
        SSSetMaxRows(lpSS->lpBook->hWnd, lMaxRows);
      }
      lCol = lColInit;
      fProcessingCellData = FALSE;
    }
    else if (fProcessingCellData)
    {
      pszData[lDestPos++] = ptr[lSrcPos++];
    }
    else
      fProcessingCellData = TRUE;
  }
  if (fProcessingCellData)
  {
    // RFW - 7/28/04 - 14861
    if (lCol >= SS_GetColCnt(lpSS))
      SSSetMaxCols(lpSS->lpBook->hWnd, lCol - (lpSS->Col.HeaderCnt - 1));

    if (lFlags & SS_EXPORTTEXT_UNFORMATTED)
      SS_SetValue(lpSS, lCol, lRow, pszData);
    else
      SS_SetData(lpSS, lCol, lRow, pszData);
  }

  if (fProcessingCellData || lCol > lColInit)
    lRow++;

  SSSetMaxCols(lpSS->lpBook->hWnd, lLastCol - (lpSS->Col.HeaderCnt - 1));
  SSSetMaxRows(lpSS->lpBook->hWnd, lRow-1 - (lpSS->Row.HeaderCnt - 1));

  if (ghCellDelim)
  {
    tbGlobalUnlock(ghCellDelim);
    tbGlobalFree(ghCellDelim);
  }

  if (ghPtr)
  {
    tbGlobalUnlock(ghPtr);
    tbGlobalFree(ghPtr);
  }

  tbGlobalUnlock(ghData);
  tbGlobalFree(ghData);

  return bRet;
}

// jb - 1.nov.00 --------------------------------------------------------------
//
// Load a text file into a spread.  Must have column and row delimiters.  May also
// have cell and sheet delimeters.
//
BOOL SS_LoadTextFile (HWND hWnd, LPCTSTR pcszFile, LPCTSTR pcszCellDelim, LPCTSTR pcszColDelim, LPCTSTR pcszRowDelim, long lFlags, LPCTSTR pcszLogFile)
{
	LPSPREADSHEET lpSS;
	BOOL          bRet = FALSE;
	long          lBufferLen = 0;
	LPBYTE	     ptr;
	GLOBALHANDLE  gh = 0;
	SS_TEXT       text;
	TCHAR         tcFormat[BUFSIZ*10];
   SS_COORD      lColHdrs;
   SS_COORD      lRowHdrs;
	BOOL          RedrawOld;

	// No need to proceed if filename, column, or row delimiter is invalid.
	//
	if (!pcszFile || !_tcslen (pcszFile)) //|| !pcszColDelim  || !_tcslen (pcszColDelim) || !pcszRowDelim  || !_tcslen (pcszRowDelim))
		return bRet;

	if (!InitSS_Text (&text, pcszFile, pcszCellDelim, pcszColDelim, pcszRowDelim, lFlags, pcszLogFile, 1))
		return bRet;

	HeaderLog_TextLoad (&text);

	// We read in the text file
	//
  bRet = TextFile2Buffer(pcszFile, &gh, &lBufferLen);
  if (bRet && 0 != gh)
    ptr = (LPBYTE)GlobalLock(gh);

  if (!bRet || 0 == gh || ptr == NULL)
  {
    LoadString((HINSTANCE)fpInstance, IDS_LOGS_CANTALLOCBUFFER, tcFormat, BUFSIZ*10-1);
    _stprintf (text.tcBuf, tcFormat);
    write2log_text (&text);
    return FALSE;
	}
//	ptr[lBufferLen] = 0;

	lpSS = SS_SheetLock(hWnd);
	lColHdrs = lpSS->Col.HeaderCnt;
	lRowHdrs = lpSS->Row.HeaderCnt;

	RedrawOld = lpSS->lpBook->Redraw;
	lpSS->lpBook->Redraw = FALSE;

//GAB 01/09/02 this was changed to give users the ability to either clear the data and the 
// formatting or just the data (like the Spread 3.0 LoadTabFile)  
#ifdef SS_V40
  if ((lFlags & SS_LOADTEXT_CLEARDATAONLY) || (lFlags & SS_LOADTEXT_TABFILE))
#elif SS_V35
  if (lFlags & SS_LOADTEXT_TABFILE)
#endif //SS_V40
      SS_ClearDataRange(lpSS, SS_ALLCOLS, SS_ALLROWS, SS_ALLCOLS, SS_ALLROWS, FALSE, FALSE);
  else
		{
		LPSS_BOOK lpBook = lpSS->lpBook;

		SS_SheetUnlock(hWnd);
#ifdef SS_V70
		SS_ResetSheet(lpBook, lpBook->nSheet);
#else
		SS_Reset(lpBook);
#endif
		lpSS = SS_SheetLock(hWnd);
		if (!lpSS)
			{
			SS_BookSetRedraw(lpBook, RedrawOld);
			return (FALSE);
			}
		}

#ifdef SS_V40
   SS_SetRowHeaderCols(lpSS, lColHdrs);
   SS_SetColHeaderRows(lpSS, lRowHdrs);
#endif
  // If sheet delimiters are specified, then data will begin loading starting 
	// with sheet n to number of sheet delimiters.  The sheets will be inserted
	// if sheets already exist.
	//
	bRet = LoadTextFile (lpSS, (LPTSTR)ptr, lBufferLen, &text, lFlags);
	if (bRet)
	{	LoadString((HINSTANCE)fpInstance, IDS_LOGS_SUCCESS_LOADTEXT, tcFormat, BUFSIZ*10-1);
		_stprintf (text.tcBuf, tcFormat, text.lLastCol, text.lRow, text.szFile);
	}
	else
	{	LoadString((HINSTANCE)fpInstance, IDS_LOGS_FAILEDLOAD, tcFormat, BUFSIZ*10-1);
		_stprintf (text.tcBuf, tcFormat);
	}

  GlobalUnlock(gh);
  GlobalFree(gh);

	text.lRow -= 1;
	if (text.lLastCol < lpSS->Col.HeaderCnt)
	{	text.lLastCol = lpSS->Col.HeaderCnt;
	}

	write2log_text (&text);
	LoadString((HINSTANCE)fpInstance, IDS_LOGS_FOOTER_LOAD, tcFormat, BUFSIZ*10-1);
   	_stprintf (text.tcBuf, tcFormat);
	write2log_text (&text);

	if (text.ghLogFile)
	{	GlobalFree (text.ghLogFile);
	}

	SS_BookSetRedraw(lpSS->lpBook, RedrawOld);
	SS_SheetUnlock (hWnd);
	return bRet;
}
// jb - 2.may.01 --------------------------------------------------------------
//
// Export a range of spread cells to XML, store in a file
//
BOOL DLLENTRY SSLoadTextFile (HWND hWnd, LPCTSTR pcszFile, LPCTSTR pcszCellDelim, LPCTSTR pcszColDelim, LPCTSTR pcszRowDelim, long lFlags, LPCTSTR pcszLogFile)
{
	return SS_LoadTextFile (hWnd, pcszFile, pcszCellDelim, pcszColDelim, pcszRowDelim, lFlags, pcszLogFile);
}
#endif // SS_V35
