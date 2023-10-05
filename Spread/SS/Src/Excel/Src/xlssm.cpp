/*********************************************************
* XLssm.cpp
*
* Copyright (C) 1999 FarPoint Technologies
* All Rights Reserved.
*
* No part of this source code may be copied, modified or
* reproduced in any form without retaining the above
* copyright notice.  This source code, or source code
* derived from it, may not be redistributed without
* express written permission of FarPoint Technologies.
*
* UPDATE LOG:
* -----------
*	RAP	- 11.19.98
* RAP01 - Backcolor not displayed in Excel                             12.3.98
* RAP02 - LYW74 - .00 being displayed for floats instead of 0.00       2.9.99
* RAP03 - GIC9468 - GPF loading Spread-created XLS file with forumla   6.2.99
* RAP04 - GIC10087 - SST being corrupted on save.                      7.27.99  
* RAP05 - GIC10305 - Incorrectly exporting xls file. CONTINUE problems 8.13.99
* RAP06 - SCS8850 - Implement support for virtual mode                 8.24.99
* RAP07 - GIC9513 - Excel97 unable to load exported XLS file with      9.13.99
*                   many strings.
* RAP08 - GIC10561 - Cell Borders not being exported correctly         9.14.99
* RAP09 - GIC10743 - Fonts converting with 0 length...                 10.11.99
* RAP10 - GIC11343                                                     2.8.00 
* RAP11 - GIC11708                                                     5.10.00
* RAP12 - TIB6982                                                      8.11.00   
* RAP13 - SEL5033                                                      8.22.00
* RAP14 - GIC12194
* RAP15 - GIC11985                                                     10.19.00
* RAP16 - SCS10117 - implement Strings in formulas: if(A1="xyz",1,2)   11.21.00
* RAP17 - Bunka Bug - exporting a cell with CellBorderStyle set but    4.9.2001
*         no setting for CellBorderType causes Excel to not display
*         the Cell-Format dialog
* RAP18 - 8690 - RAP16 broke if(A1>1,2,3).  Fixed both issues          5.7.2001
* RAP19 - 8687 - fWrap/Multiline setting in Spread are ignored.        5.8.2001
* RAP20 - 
* RAP21 - 8760                                                         5.24.2001 
* RAP22 - Checkbox modification for exporting
* RAP23 - 9310                                                         9.18.2001
* RAP25 - 9615                                                         1.9.2002 
* RAP26 - 9768 - relative cell addressing not exported correctly       2.12.2002
* RAP27 - RUN_SPR_004_010 bug with Yen char not correctly exporting    5.15.2002
*********************************************************/

#if defined(_WIN64) || defined(_IA64)
#define _CRT_SECURE_CPP_OVERLOAD_STANDARD_NAMES 1
#define _CRT_SECURE_NO_DEPRECATE
#endif

#include <windows.h>
#include <ole2.h>
#if !defined(_WIN64) && !defined(_IA64)
#include <largeint.h>
#endif
#include <stdio.h>
//#ifdef SPREAD_JPN
#include <mbctype.h>    //for support DBCS
//#endif
//#define WINMEM
#ifdef SS_V70
  #define V7_TEST
#endif

#include "spread.h"
#include "xl.h"
#include "xlbiff.h"
#include "xlutl.h"
#include "biff.h"
#include "ss_alloc.h"
#include "ss_draw.h"
#include "ss_font.h"
#include "ss_user.h"
#include "ss_save4.h"
#include "ss_virt.h"
#include "ss_win.h"
#include "ss_row.h"
#include "ss_col.h"
#include "ss_cell.h"
#include "..\..\..\include\xlcall.h"
#ifdef SS_V40
#include "ss_type.h"
#include "ss_span.h"
#endif
#ifdef SS_V70
#include "ss_book.h"
#endif
#include "..\calc\cal_col.h"
#include "..\calc\cal_row.h"
#include "..\calc\cal_cell.h"
#include "..\calc\cal_expr.h"
#include "..\calc\cal_oper.h"
#include "..\calc\cal_func.h"
#include "..\calc\cal_tok.h"
#include "..\calc\cal_err.h"
#include "..\calc\cal_name.h"


#define CHUNKSIZE 10000

long ssm_WriteBuffer(TBGLOBALHANDLE FAR *lpgh, LPBIFF lpBiff, LPVOID lp, LPLONG lplPos, LPVOID lpExtra);
long ssm_BuildXF(SS_COORD Col, SS_COORD Row, short sXFType, LPSHORT lpsIndex, LPSHORT lpsCount,
                 TBGLOBALHANDLE FAR *lpghXF, LPBIFF lpBiff, LPVOID lp, LPLONG lplXFLen, LPVOID lpXF,
                 TBGLOBALHANDLE FAR *lpghFont, LPLONG lplFontLen);
long ssm_BuildSTYLE(LPVOID lp, TBGLOBALHANDLE FAR *lpghStyle, LPLONG lplStyleLen);
long ssm_BuildFormat(LPSHORT lpsCount, TBGLOBALHANDLE FAR *lpghFormat, LPBIFF lpBiff, LPVOID lp, LPLONG lplFormatLen, LPTSTR lptstr, BOOL fUnicode);
WORD ssm_BuildFont(LPSS_BOOK lpBook, TBGLOBALHANDLE FAR *lpghFont, LPLONG lplFontLen, SS_FONTID FontId, WORD icv);
BOOL ssm_IsRowDefault(LPSPREADSHEET lpSS, SS_COORD lRow);
BOOL ssm_IsColDefault(LPSPREADSHEET lpSS, SS_COORD lCol);
BOOL ssm_IsColDefaultDrawing(LPSPREADSHEET lpSS, SS_COORD lCol);
BOOL ssm_IsRowDefaultDrawing(LPSPREADSHEET lpSS, SS_COORD lRow);
BOOL ssm_IsCellDefault(LPSPREADSHEET lpSS, SS_COORD lCol, SS_COORD lRow);
long ssm_DumpBuffer(IStream *pstm, TBGLOBALHANDLE gh, LPLONG lplLen);
long ssm_BuildBIFFStr(LPTSTR lptstr, LPTBGLOBALHANDLE lpghBiffStr, BOOL fUnicode);
long ssm_BuildSST(HWND hWnd, SS_COORD lCol, SS_COORD lRow, LPTBGLOBALHANDLE lpghSST, LPLONG lplSSTLen, LPTBGLOBALHANDLE lpghEXTSST, LPLONG lplEXTSSTLen, BYTE bCellType, LPTSTR lptstrVal, LPTBGLOBALHANDLE lpghStrings);
long ssm_FixUpSST(TBGLOBALHANDLE FAR *lpghSST, LPLONG lplSSTLen);
long ssm_Save4Expr(LPSPREADSHEET lpSS, SS_COORD lCol, SS_COORD lRow, CALC_HANDLE hExpr, LPTBGLOBALHANDLE lpgh, LPLONG lplen, LPSHORT lpxtiCount, LPTBGLOBALHANDLE lpghXTI, LPTBGLOBALHANDLE lpghCustomNames, LPLONG lplCustomNamesLen);
#ifdef SS_V70
BOOL ssm_GetFormula(LPSPREADSHEET lpSS, SS_COORD lCol, SS_COORD lRow, LPTBGLOBALHANDLE lpgh, LPLONG lplen, LPSHORT lpxtiCount, LPTBGLOBALHANDLE lpghXTI, LPTBGLOBALHANDLE lpghCustomNames, LPLONG lplCustomNamesLen);
#else
BOOL ssm_GetFormula(LPSPREADSHEET lpSS, SS_COORD lCol, SS_COORD lRow, LPTBGLOBALHANDLE lpgh, LPLONG lplen);
#endif
long ssm_AddFormula(LPSS_BOOK lpBook, SS_COORD lCol, SS_COORD lRow, TBGLOBALHANDLE ghFormula, long lFormulaLen, short sXFIndex, double *lpdblVal, TBGLOBALHANDLE *lpghVal, LPTBGLOBALHANDLE lpgh, LPLONG lplen);
WORD ssm_FontCheck(TBGLOBALHANDLE FAR *lpghFont, LPLONG lplFontLen);
long ssm_ApplyCelltype(LPSPREADSHEET lpSS, SS_COORD lCol, SS_COORD lRow, BOOL fIsFormula, LPVOID lp, BOOL fFormat, BOOL fData, double *pdblRet, LPTBGLOBALHANDLE lpghRet, LPAPPLYCELLTYPE lpAC, LPTBGLOBALHANDLE lpghStrings);
long ssm_SaveBorder(LPSS_CELLBORDER lpCellBorder, LPxXF8 lpxf);

#ifdef SS_V70
extern "C"
BOOL SSGetSheetDisplayName(HWND hWnd, short nSheet, LPTBGLOBALHANDLE lpghBuffer);
#endif

//short CustFuncGetSaveId(LPCALC_INFO lpCalc, CALC_HANDLE hFunc);
//short CustNameGetSaveId(LPCALC_INFO lpCalc, CALC_HANDLE hName);
int  NameGetText(CALC_HANDLE hName, LPTSTR lpszText, int nLen);
int  NameGetTextLen(CALC_HANDLE hName);

void DisplayError(HRESULT hr, LPTSTR lptstr);

//extern void NumGetDisplayFormat(LPSSNUM_FORMAT lpFormatSrc, LPSSNUM_FORMAT lpFormat);

#ifdef SPREAD_JPN
#define KANJISEPARATOR		'?'
#define JSEPARATOR_YEAR		_T("\"”N\"")
#define JSEPARATOR_MONTH	_T("\"ŒŽ\"")
#define JSEPARATOR_DAY		_T("\"“ú\"")
#define JSEPARATOR_HOUR     _T("\"Žž\"")
#define JSEPARATOR_MINUTE   _T("\"•ª\"")
#define JSEPARATOR_SECOND   _T("\"•b\"")
#define JTIME_AM			_T("Œß‘O")
#define JTIME_PM			_T("ŒßŒã")
#endif
/***********************************************************************
* Name:        ssm_Initialize - Initialize the COM library.
*
* Usage:       long ssm_Initialize()
*
* Return:      long - Success or Failure
***********************************************************************/
long ssm_Initialize()
{ 
	// Initialize the Component Object Model (COM) library before you can call its functions.
  HRESULT hr = S_OK;
 	
	hr = CoInitialize(NULL);
	if (S_OK != hr && S_FALSE != hr) //S_FALSE = COM Library already initialized 
  {
	  ssm_UnInitialize();
  	DisplayError(hr, _T("CoInitialize"));
    return hr;
	}

  return hr;
}

/***********************************************************************
* Name:        ssm_UnInitialize - Uninitialize the COM library.
*
* Usage:       long ssm_UnInitialize()
*
* Return:      long - Success or Failure
***********************************************************************/
long ssm_UnInitialize()
{
  CoUninitialize();
  
  return S_OK;
}

/***********************************************************************
* Name:   ssm_CreateStorageObject - Create a Structured Storage Model(SSM) docfile,
*                                   Then create a stream within the Docfile.
*
* Usage:  long ssm_CreateStorageObject(LPCWSTR szFileName, LPCWSTR szStreamName,
*                                      IStorage **ppstg, IStream **ppstm)
*            szFileName - File name for the docfile
*            szStreamName - Name to be used for the created stream
*            ppstg - pointer to the created SSM docfile
*            ppstm - pointer to the created docfile stream
*
* Return: long - Success or Failure
***********************************************************************/
long ssm_CreateStorageObject(LPCWSTR szFileName, LPCWSTR szStreamName,
                             IStorage **ppstg, IStream **ppstm)
{
  HRESULT hr;

  hr = StgCreateDocfile(szFileName, STGM_READWRITE | STGM_CREATE | 
	                      STGM_SHARE_EXCLUSIVE | STGM_DIRECT, 0, ppstg);
	if (S_OK != hr)
  {
		DisplayError(hr,_T("StgCreateDocfile"));
	  return hr;
  }

	hr = (*ppstg)->CreateStream(szStreamName, STGM_READWRITE | STGM_CREATE |
                              STGM_SHARE_EXCLUSIVE | STGM_DIRECT, 0, 0, ppstm);
	if (S_OK != hr)
  {
		DisplayError(hr,_T("CreateStream"));
	  return hr;
  }
  
  return S_OK;
}

/***********************************************************************
* Name:   ssm_OpenStorageObject - Open an existing Structured Storage Model(SSM) docfile,
*                                 Then open a stream within the Docfile in either
*                                 read or read/write mode based on the fLoad parameter.
*
* Usage:  long ssm_OpenStorageObject(LPCWSTR szFileName, LPCWSTR szStreamName,
*                                    IStorage **ppstg, IStream **ppstm, BOOL fLoad)
*            szFileName - File name for the docfile
*            szStreamName - Name of the stream to be opened
*            ppstg - pointer to the opened SSM docfile
*            ppstm - pointer to the opened docfile stream
*
* Return: long - Success or Failure
***********************************************************************/
long ssm_OpenStorageObject(LPCWSTR szFileName, LPCWSTR szStreamName,
                           IStorage **ppstg, IStream **ppstm, BOOL fLoad)
{
  HRESULT        hr;

  // Is this a valid structured storage model file?
  hr = StgIsStorageFile(szFileName);
	if (S_OK != hr)
  {
//		DisplayError(hr,_T("StgIsStorageFile"));
    return hr;
	}
    
  // Open the file in read/write shared mode.
  hr = StgOpenStorage(szFileName, NULL, 
//RAP10d                      STGM_READWRITE | STGM_SHARE_EXCLUSIVE, 
                      (fLoad?STGM_READ:STGM_READWRITE) | STGM_SHARE_EXCLUSIVE, //RAP10a
                      NULL, 0, ppstg);
  if (S_OK != hr)
  {
		DisplayError(hr,_T("StgOpenStorage"));
    return hr;
	}

  if (fLoad)
  {
    // Open the stream in exclusive-read mode.
    hr = (*ppstg)->OpenStream(szStreamName, NULL, 
                              STGM_SHARE_EXCLUSIVE | STGM_READ,
                              0, ppstm);
  }
  else
  {
    // Open the stream in write mode.
    hr = (*ppstg)->OpenStream(szStreamName, NULL, 
                              STGM_SHARE_EXCLUSIVE | STGM_READWRITE,
                              0, ppstm);
  }
  
  if (S_OK != hr)
  {
		DisplayError(hr,_T("OpenStream"));
    return hr;
	}

  return S_OK;
}

/***********************************************************************
* Name:   ssm_CleanUp - Release the stream
*
* Usage:  long ssm_CleanUp(IStream *pstm)
*           pstm - pointer to the docfile stream to release
*
* Return: long - Success or Failure
***********************************************************************/
long ssm_CleanUp(IStream *pstm)
{
  if (pstm)
    pstm->Release();

  return S_OK;
}

/***********************************************************************
* Name:   ssm_StreamRead - Read chunks of the specified stream and pass
*                          the data to the parser. Any portion of the chunk
*                          that is not used by the parser is recycled and
*                          appended to the front of the next chunk read
*                          from the stream. The stream read is accomplished 
*                          using 2 passes. The first pass retrieves and saves 
*                          any global Excel Workbook data. The second pass
*                          retrieves the individual sheet data. Information
*                          assembled from both of the passes is used to create 
*                          the resulting Spread.
*
* Usage:  long ssm_StreamRead(IStream *pstm, LPSSXL lpss, LPSPREADSHEET lpSS, 
*                             int nSheetNum, short sPass)
*           pstm - pointer to the docfile stream to read.
*           lpss - pointer to the XL stucture to be assembled from the data
*                  in the stream.
*           lpSS - pointer to the Spread Control's structure.
*           nSheetNum - which Excel sheet to read (this info is passed along
*                       to the parser)
*           sPass - which time through the read process is this? 
*
* Return: long - Success or Failure
***********************************************************************/
long ssm_StreamRead(IStream *pstm, LPSSXL lpss, LPSS_BOOK lpBook, int nSheetNum, short sPass)
{
  unsigned long ulChunk = CHUNKSIZE;
  TBGLOBALHANDLE  ghChunk = tbGlobalAlloc(GHND, ulChunk);
  LPBYTE        lpChunk = (LPBYTE)tbGlobalLock(ghChunk);
  unsigned long ulBytesRead = 0;
  unsigned long ulBytes = 0;
  LARGE_INTEGER li = {0, 0};
  HRESULT       hr = S_OK;
  short         sCurrentSheet = 0;
  short         sSheetsLoadedCount = 0;
  BOOL          fIsValidSubstream = TRUE;
  long          lTotalLen = 0;
  short         sRet = 0;
  
  if (sPass == 0)
    bif_Init((LPSSXL)lpss);

  hr = pstm->Seek(li, STREAM_SEEK_SET, NULL);
  if (S_OK != hr)
  {
    DisplayError(hr,_T("Seek"));
    tbGlobalUnlock(ghChunk);
    tbGlobalFree(ghChunk);
    ghChunk=0;
    return hr;
  }
   
  do
  {
    hr = pstm->Read(lpChunk, ulChunk, &ulBytesRead);
    if (S_OK != hr)
    {
  		DisplayError(hr,_T("Read"));
      tbGlobalUnlock(ghChunk);
      tbGlobalFree(ghChunk);
      ghChunk=0;
      return hr;
    }

    ulBytes = ulBytesRead;
    sRet = bif_Parse(lpChunk, lpss, &ulBytes, lpBook, nSheetNum, &sCurrentSheet, &sSheetsLoadedCount, sPass, &fIsValidSubstream, lTotalLen);
    lTotalLen += (CHUNKSIZE-ulBytes);

    if (sRet == EOF)
      break;
    else if (sRet == 999) //This Excel file is password protected.
    {
      tbGlobalUnlock(ghChunk);
      tbGlobalFree(ghChunk);
      ghChunk=0;
      xl_LogFile(lpBook, LOG_PASSWORD, 0, 0, NULL);
      DisplayError(hr,_T("Password"));
      return S_FALSE;
    }

    if (ulBytes != 0)
    {
      //recycle the unParsed portion of the buffer.
#if defined(_WIN64) || defined(_IA64)
	  li.QuadPart = -(long)ulBytes;
#else
      li = ConvertLongToLargeInteger(-(long)ulBytes);
#endif
      hr = pstm->Seek(li, STREAM_SEEK_CUR, NULL);
      if (S_OK != hr)
      {
        DisplayError(hr,_T("Seek"));
        tbGlobalUnlock(ghChunk);
        tbGlobalFree(ghChunk);
        ghChunk=0;
        return hr;
      }
    }
    if (sRet == 1)
    {
      //the chunk was not big enough, increase it by the default CHUNKSIZE.
      ulChunk += CHUNKSIZE;
      ulBytesRead += CHUNKSIZE;
      tbGlobalUnlock(ghChunk);
      ghChunk = tbGlobalReAlloc(ghChunk, ulChunk, GHND);
      lpChunk = (LPBYTE)tbGlobalLock(ghChunk);
    }
    memset(lpChunk, 0, ulChunk);
  }
  while (ulBytesRead == ulChunk);
 
  tbGlobalUnlock(ghChunk);
  tbGlobalFree(ghChunk);
  ghChunk=0;

  return S_OK;
}

/***********************************************************************
* Name:   ssm_StreamWrite - Build the Excel BIFF formatted buffer and 
*                           write the it to the specified stream. To
*                           correctly resolve offsets, several buffers
*                           are built and maintained throughout the 
*                           StreamWrite process. These buffers are
*                           assembled in the correct order after resolving
*                           all the offsets.
*
* Usage:  long ssm_StreamWrite(IStream *pstm, LPVOID lp, LPTSTR szSheetName)
*                              int nSheetNum, short sPass, LPTBGLOBALHANDLE lpghBuffer
*                              LPLONG lplBufferlen, BOOL fToStream)
*         pstm - pointer to the docfile stream to write.
*         lp - pointer to the Spread structure -- cast as LPVOID to pass to
*              other ssm_ functions.                  
*         szSheetName - which Excel sheet to read.
*         lpghBuffer - return buffer.
*         fToStream - write it to the stream or return the buffer?
*
* Return: long - Success or Failure
*
* Detailed description:
* The spreadsheet is picked apart, and its pieces are used to build
* Excel BIFF records. As information is gathered BIFF records are
* created and either added to the stream, or added to buffers. If the
* particular record is the next logical record that should appear in 
* the stream, it is added. If the record is part of a collection of 
* records (FORMAT, XF, FONT, STYLE, ROW, cell records), or if the record
* contains offsets that will need to be resolved after assembling all
* the BIFF records, or if the final stream-position of the BIFF record
* is such that the record size is needed to help resolve a stream offset
* in another record the record is appended to the appropriate buffer.
* After assembling all the BIFF records, any BIFF record stream-offsets
* are resolved. Reference point variables are set at each appropriate
* location, and buffer length variables are maintained for each buffer
* for the purpose of resolving offsets. Offset resolution is accomplished
* by summing the appropriate buffer lengths to produce the byte offset
* relative to the required reference point. The buffers are then 
* concatenated together in the order in which Excel expects to receive them.
* Determining this order has been accomplished though inspection of 
* Excel-created XLS files.
*
* NOTE!!!
* Refer to the Excel Developer's Kit for explanation of the cell record 
* optimized lookup offsets that exist in the INDEX, ROW and DBCELL records.
*
*************************************************************************/
long ssm_StreamWrite(IStream *pstm, LPVOID lp, LPCTSTR szSheetName, 
                     LPTBGLOBALHANDLE lpghBuffer, LPLONG lplBufferLen, BOOL fToStream, SHORT sFlags)
{
  BIFF  biff;
  short sBOFType = 0;
  WORD  wProtect = 0;
  short sXFIndex = -1;
  short sXFCount = 0;
  short sFormatCount = -1;
  TBGLOBALHANDLE  ghBiffBuffer = (TBGLOBALHANDLE)0; // 
  long          lBiffBufferLen = 0;
  TBGLOBALHANDLE  ghWorkSheet = (TBGLOBALHANDLE)0; //
  long          lWorkSheetLen = 0;
  TBGLOBALHANDLE  ghFormat = (TBGLOBALHANDLE)0;    // FORMAT records buffer
  long          lFormatLen = 0;
  TBGLOBALHANDLE  ghFont = (TBGLOBALHANDLE)0;      // FONT records buffer
  long          lFontLen = 0;
  TBGLOBALHANDLE  ghXF = (TBGLOBALHANDLE)0;        // XF records buffer
  long          lXFLen = 0;
  TBGLOBALHANDLE  ghStyle = (TBGLOBALHANDLE)0;     // STYLE records  buffer
  long          lStyleLen = 0;
  TBGLOBALHANDLE  ghGeneral = (TBGLOBALHANDLE)0;   // Workbook "general" records  buffer
  long          lGeneralLen = 0;
  TBGLOBALHANDLE  ghEOF = (TBGLOBALHANDLE)0;       // EOF record buffer
  long          lEOFLen = 0;
  TBGLOBALHANDLE  ghSST = (TBGLOBALHANDLE)0;       // SST record buffer
  long          lSSTLen = 0;
  TBGLOBALHANDLE  ghEXTSST = (TBGLOBALHANDLE)0;    // EXTSST record buffer
  long          lEXTSSTLen = 0;
  LPSS_BOOK     lpBook = (LPSS_BOOK)lp;
  TBGLOBALHANDLE   ghStrings = (TBGLOBALHANDLE)0;  // for performing binary search on sst entries.
#ifdef SS_V70  
  TBGLOBALHANDLE ghCrossSheet = (TBGLOBALHANDLE)0;
  long          lCrossSheetLen = 0;
//  TBGLOBALHANDLE ghExternNames = (TBGLOBALHANDLE)0;
//  long          lExternNamesLen = 0;
  TBGLOBALHANDLE ghNames = (TBGLOBALHANDLE)0;
  long          lNamesLen = 0;
  TBGLOBALHANDLE  ghXTI = (TBGLOBALHANDLE)0;
  short          sxtiCount = 0;
  int            nSheetIndex = lpBook->nSheet;
#endif

#ifdef SS_V70
  short          startSheet = 0;
  short          endSheet = 0;
  short          i = 0;
  TBGLOBALHANDLE ghSheets = tbGlobalAlloc(GHND, lpBook->nSheetCnt*sizeof(TBGLOBALHANDLE));
  TBGLOBALHANDLE ghCustomNames = (TBGLOBALHANDLE)0;
  long           lCustomNamesLen = 0;
  szSheetName; // to make the compiler happy.
#else
  sFlags; // to make the compiler happy.
#endif

  if (szSheetName == NULL) // szSheetName == NULL indicates to save all sheets
    lpBook->nSheet = 0;

// General Section
  xl_LogFile(lpBook, LOG_REGISTRYPROPS, 0, 0, NULL);

  biff.recnum = xlBOF_BIFF5_7_8;
  biff.datalen = sizeof(zBOF);
  ssm_WriteBuffer((TBGLOBALHANDLE FAR *)&ghBiffBuffer, &biff, (LPVOID)&sBOFType, &lBiffBufferLen, NULL);
//  ssm_WriteBuffer((TBGLOBALHANDLE FAR *)&ghBiffBuffer, &biff, lp, &lBiffBufferLen, NULL);

#ifndef V7_TEST
  biff.recnum = xlINTERFACEHDR;
  biff.datalen = 2;
  ssm_WriteBuffer((TBGLOBALHANDLE FAR *)&ghBiffBuffer, &biff, lp, &lBiffBufferLen, NULL);

  biff.recnum = xlMMS;
  biff.datalen = 2;
  ssm_WriteBuffer((TBGLOBALHANDLE FAR *)&ghBiffBuffer, &biff, lp, &lBiffBufferLen, NULL);

  biff.recnum = xlINTERFACEEND;
  biff.datalen = 0;
  ssm_WriteBuffer((TBGLOBALHANDLE FAR *)&ghBiffBuffer, &biff, lp, &lBiffBufferLen, NULL);

  biff.recnum = xlCODEPAGE;
  biff.datalen = 2;
  ssm_WriteBuffer((TBGLOBALHANDLE FAR *)&ghBiffBuffer, &biff, lp, &lBiffBufferLen, NULL);

  biff.recnum = xlDSF;
  biff.datalen = 2;
  ssm_WriteBuffer((TBGLOBALHANDLE FAR *)&ghBiffBuffer, &biff, lp, &lBiffBufferLen, NULL);

// Spread3.0 does not know about WorkBook Protection 
  biff.recnum = xlWINDOWPROTECT;
  biff.datalen = 2;
  ssm_WriteBuffer((TBGLOBALHANDLE FAR *)&ghBiffBuffer, &biff, lp, &lBiffBufferLen, NULL);

  biff.recnum = xlPROTECT;
  biff.datalen = 2;
// hatakeyama@boc.co.jp 11/21-2000	for excel document protects.
// Sets sheet protect to "None".
	wProtect = 0;
  ssm_WriteBuffer((TBGLOBALHANDLE FAR *)&ghBiffBuffer, &biff, lp, &lBiffBufferLen, (LPVOID)&wProtect);

  biff.recnum = xlPROT4REV;
  biff.datalen = 2;
  ssm_WriteBuffer((TBGLOBALHANDLE FAR *)&ghBiffBuffer, &biff, lp, &lBiffBufferLen, NULL);
#endif //TEST
  BOOL fSaveSheet = (szSheetName==NULL?FALSE:TRUE);
  biff.recnum = xlWINDOW1;
  biff.datalen = 18;
  ssm_WriteBuffer((TBGLOBALHANDLE FAR *)&ghBiffBuffer, &biff, lp, &lBiffBufferLen, (LPVOID)&fSaveSheet);
#ifndef V7_TEST
  biff.recnum = xl1904;
  biff.datalen = 2;
  ssm_WriteBuffer((TBGLOBALHANDLE FAR *)&ghBiffBuffer, &biff, lp, &lBiffBufferLen, NULL);
#endif //TEST
  biff.recnum = xlREFRESHALL;
  biff.datalen = 2;
  ssm_WriteBuffer((TBGLOBALHANDLE FAR *)&ghBiffBuffer, &biff, lp, &lBiffBufferLen, NULL);

  biff.recnum = xlBOOKBOOL;
  biff.datalen = 2;
  ssm_WriteBuffer((TBGLOBALHANDLE FAR *)&ghBiffBuffer, &biff, lp, &lBiffBufferLen, NULL);

  //XF-0 is the XF record for the Normal style
  biff.recnum = xlXF;
  biff.datalen = 20;
  ssm_BuildXF(0, 0, 0, &sXFIndex, &sXFCount, (TBGLOBALHANDLE FAR *)&ghXF, (LPBIFF)&biff, lp, (LPLONG)&lXFLen, NULL, (TBGLOBALHANDLE FAR *)&ghFont, (LPLONG)&lFontLen);

  biff.recnum = xlXF;
  biff.datalen = 20;
  ssm_BuildXF(0, 0, 0, &sXFIndex, &sXFCount, (TBGLOBALHANDLE FAR *)&ghXF, (LPBIFF)&biff, lp, (LPLONG)&lXFLen, NULL, (TBGLOBALHANDLE FAR *)&ghFont, (LPLONG)&lFontLen);

  biff.recnum = xlXF;
  biff.datalen = 20;
  ssm_BuildXF(0, 0, 0, &sXFIndex, &sXFCount, (TBGLOBALHANDLE FAR *)&ghXF, (LPBIFF)&biff, lp, (LPLONG)&lXFLen, NULL, (TBGLOBALHANDLE FAR *)&ghFont, (LPLONG)&lFontLen);

  biff.recnum = xlXF;
  biff.datalen = 20;
  ssm_BuildXF(0, 0, 0, &sXFIndex, &sXFCount, (TBGLOBALHANDLE FAR *)&ghXF, (LPBIFF)&biff, lp, (LPLONG)&lXFLen, NULL, (TBGLOBALHANDLE FAR *)&ghFont, (LPLONG)&lFontLen);

  biff.recnum = xlXF;
  biff.datalen = 20;
  ssm_BuildXF(0, 0, 0, &sXFIndex, &sXFCount, (TBGLOBALHANDLE FAR *)&ghXF, (LPBIFF)&biff, lp, (LPLONG)&lXFLen, NULL, (TBGLOBALHANDLE FAR *)&ghFont, (LPLONG)&lFontLen);

  biff.recnum = xlXF;
  biff.datalen = 20;
  ssm_BuildXF(0, 0, 0, &sXFIndex, &sXFCount, (TBGLOBALHANDLE FAR *)&ghXF, (LPBIFF)&biff, lp, (LPLONG)&lXFLen, NULL, (TBGLOBALHANDLE FAR *)&ghFont, (LPLONG)&lFontLen);

  biff.recnum = xlXF;
  biff.datalen = 20;
  ssm_BuildXF(0, 0, 0, &sXFIndex, &sXFCount, (TBGLOBALHANDLE FAR *)&ghXF, (LPBIFF)&biff, lp, (LPLONG)&lXFLen, NULL, (TBGLOBALHANDLE FAR *)&ghFont, (LPLONG)&lFontLen);

  biff.recnum = xlXF;
  biff.datalen = 20;
  ssm_BuildXF(0, 0, 0, &sXFIndex, &sXFCount, (TBGLOBALHANDLE FAR *)&ghXF, (LPBIFF)&biff, lp, (LPLONG)&lXFLen, NULL, (TBGLOBALHANDLE FAR *)&ghFont, (LPLONG)&lFontLen);

  biff.recnum = xlXF;
  biff.datalen = 20;
  ssm_BuildXF(0, 0, 0, &sXFIndex, &sXFCount, (TBGLOBALHANDLE FAR *)&ghXF, (LPBIFF)&biff, lp, (LPLONG)&lXFLen, NULL, (TBGLOBALHANDLE FAR *)&ghFont, (LPLONG)&lFontLen);

  biff.recnum = xlXF;
  biff.datalen = 20;
  ssm_BuildXF(0, 0, 0, &sXFIndex, &sXFCount, (TBGLOBALHANDLE FAR *)&ghXF, (LPBIFF)&biff, lp, (LPLONG)&lXFLen, NULL, (TBGLOBALHANDLE FAR *)&ghFont, (LPLONG)&lFontLen);

  biff.recnum = xlXF;
  biff.datalen = 20;
  ssm_BuildXF(0, 0, 0, &sXFIndex, &sXFCount, (TBGLOBALHANDLE FAR *)&ghXF, (LPBIFF)&biff, lp, (LPLONG)&lXFLen, NULL, (TBGLOBALHANDLE FAR *)&ghFont, (LPLONG)&lFontLen);

  biff.recnum = xlXF;
  biff.datalen = 20;
  ssm_BuildXF(0, 0, 0, &sXFIndex, &sXFCount, (TBGLOBALHANDLE FAR *)&ghXF, (LPBIFF)&biff, lp, (LPLONG)&lXFLen, NULL, (TBGLOBALHANDLE FAR *)&ghFont, (LPLONG)&lFontLen);

  biff.recnum = xlXF;
  biff.datalen = 20;
  ssm_BuildXF(0, 0, 0, &sXFIndex, &sXFCount, (TBGLOBALHANDLE FAR *)&ghXF, (LPBIFF)&biff, lp, (LPLONG)&lXFLen, NULL, (TBGLOBALHANDLE FAR *)&ghFont, (LPLONG)&lFontLen);

  biff.recnum = xlXF;
  biff.datalen = 20;
  ssm_BuildXF(0, 0, 0, &sXFIndex, &sXFCount, (TBGLOBALHANDLE FAR *)&ghXF, (LPBIFF)&biff, lp, (LPLONG)&lXFLen, NULL, (TBGLOBALHANDLE FAR *)&ghFont, (LPLONG)&lFontLen);

  biff.recnum = xlXF;
  biff.datalen = 20;
  ssm_BuildXF(0, 0, 0, &sXFIndex, &sXFCount, (TBGLOBALHANDLE FAR *)&ghXF, (LPBIFF)&biff, lp, (LPLONG)&lXFLen, NULL, (TBGLOBALHANDLE FAR *)&ghFont, (LPLONG)&lFontLen);

  // index 15
// 17472 >>
//  biff.recnum = xlXF;
//  biff.datalen = 20;
//  ssm_BuildXF(0, 0, 0, &sXFIndex, &sXFCount, (TBGLOBALHANDLE FAR *)&ghXF, (LPBIFF)&biff, lp, (LPLONG)&lXFLen, NULL, (TBGLOBALHANDLE FAR *)&ghFont, (LPLONG)&lFontLen);
 
  biff.recnum = xlXF;
  biff.datalen = 20;
  ssm_BuildXF(-2, -2, 1, &sXFIndex, &sXFCount, (TBGLOBALHANDLE FAR *)&ghXF, &biff, lp, &lXFLen, NULL, (TBGLOBALHANDLE FAR *)&ghFont, (LPLONG)&lFontLen);
// << 17472
  biff.recnum = xlXF;
  biff.datalen = 20; 
  ssm_BuildXF(0, 0, 0, &sXFIndex, &sXFCount, (TBGLOBALHANDLE FAR *)&ghXF, (LPBIFF)&biff, lp, (LPLONG)&lXFLen, NULL, (TBGLOBALHANDLE FAR *)&ghFont, (LPLONG)&lFontLen);

  biff.recnum = xlXF;
  biff.datalen = 20;
  ssm_BuildXF(0, 0, 0, &sXFIndex, &sXFCount, (TBGLOBALHANDLE FAR *)&ghXF, (LPBIFF)&biff, lp, (LPLONG)&lXFLen, NULL, (TBGLOBALHANDLE FAR *)&ghFont, (LPLONG)&lFontLen);

  biff.recnum = xlXF;
  biff.datalen = 20;
  ssm_BuildXF(0, 0, 0, &sXFIndex, &sXFCount, (TBGLOBALHANDLE FAR *)&ghXF, (LPBIFF)&biff, lp, (LPLONG)&lXFLen, NULL, (TBGLOBALHANDLE FAR *)&ghFont, (LPLONG)&lFontLen);

  biff.recnum = xlXF;
  biff.datalen = 20;
  ssm_BuildXF(0, 0, 0, &sXFIndex, &sXFCount, (TBGLOBALHANDLE FAR *)&ghXF, (LPBIFF)&biff, lp, (LPLONG)&lXFLen, NULL, (TBGLOBALHANDLE FAR *)&ghFont, (LPLONG)&lFontLen);

  biff.recnum = xlXF;
  biff.datalen = 20;
  ssm_BuildXF(0, 0, 0, &sXFIndex, &sXFCount, (TBGLOBALHANDLE FAR *)&ghXF, (LPBIFF)&biff, lp, (LPLONG)&lXFLen, NULL, (TBGLOBALHANDLE FAR *)&ghFont, (LPLONG)&lFontLen);

  //SpreadSheet Default XF
  biff.recnum = xlXF;
  biff.datalen = 20;
  ssm_BuildXF(0, 0, 0, &sXFIndex, &sXFCount, (TBGLOBALHANDLE FAR *)&ghXF, (LPBIFF)&biff, lp, (LPLONG)&lXFLen, NULL, (TBGLOBALHANDLE FAR *)&ghFont, (LPLONG)&lFontLen);
//  biff.recnum = xlXF;
//  biff.datalen = 20;
//  ssm_BuildXF(SS_ALLCOLS, SS_ALLROWS, 1, &sXFIndex, &sXFCount, (TBGLOBALHANDLE FAR *)&ghXF, (LPBIFF)&biff, lp, (LPLONG)&lXFLen, NULL, (TBGLOBALHANDLE FAR *)&ghFont, (LPLONG)&lFontLen);

  //STYLE
  biff.recnum = xlSTYLE;
  biff.datalen = 4;
  ssm_BuildSTYLE(lp, (TBGLOBALHANDLE FAR *)&ghStyle, (LPLONG)&lStyleLen);

#ifndef SS_V70
  biff.recnum = xlBUNDLESHEET;
  biff.datalen = 8;
  
  {
    TBGLOBALHANDLE ghSheetName = 0;
    LPTSTR         lpszSheetName = NULL;
    int            j;

    Alloc(&ghSheetName, lstrlen(szSheetName)+sizeof(TCHAR));
    lpszSheetName = (LPTSTR)tbGlobalLock(ghSheetName);
    _tcscpy(lpszSheetName, szSheetName);
    for (j=0; j<lstrlen(lpszSheetName); j++)
    {
#ifndef _UNICODE
        BYTE* pByte = (BYTE*)(&lpszSheetName[j]);
        if (IsDBCSLeadByte(*pByte) || _ismbbkana(*pByte))
  	    {
		      j++;
          continue;
	      }
        else if(*pByte > 127)
        {
          continue;
        }
  	    else if(*pByte ++ ==0x0d && *pByte==0x0a)
	      {
          j++;
          continue;
	      }
#endif
      
      if ( lpszSheetName[j] == '?'
        || lpszSheetName[j] == '\\'
        || lpszSheetName[j] == '/'
        || lpszSheetName[j] == '*'
        || lpszSheetName[j] == '['
        || lpszSheetName[j] == ']' )
      lpszSheetName[j] = '_';
    }   
    ssm_WriteBuffer((TBGLOBALHANDLE FAR *)&ghGeneral, &biff, lp, &lGeneralLen, (LPVOID)lpszSheetName);
    tbGlobalUnlock(ghSheetName);
    tbGlobalFree(ghSheetName);
  }
#endif

#ifndef V7_TEST
  biff.recnum = xlCOUNTRY;
  biff.datalen = 4;
  ssm_WriteBuffer((TBGLOBALHANDLE FAR *)&ghGeneral, &biff, lp, &lGeneralLen, NULL);
#endif

  biff.recnum = xlEOF_BIFF;
  biff.datalen = 0;
  ssm_WriteBuffer((TBGLOBALHANDLE FAR *)&ghEOF, &biff, lp, &lEOFLen, NULL);


#ifdef SS_V70
  {
    TBGLOBALHANDLE ghPrev = 0;
    LPTSTR         lpszPrevName = NULL;
    int            len = 0;

    while ((len = SSQueryCustomNameLen(lpBook->hWnd, lpszPrevName)) > 0)
    {
      TBGLOBALHANDLE gh = 0;
      LPTSTR         lpszName = NULL;
      LPBYTE         lpb = NULL;

      Alloc(&gh, (len+1)*sizeof(TCHAR));
      lpszName = (LPTSTR)tbGlobalLock(gh);

      SSQueryCustomName(lpBook->hWnd, lpszPrevName, lpszName, (len+1)*sizeof(TCHAR));
      
      if (ghPrev)
      {
        tbGlobalFree(ghPrev);
        ghPrev = 0;
      }
      Alloc(&ghPrev, (len+1)*sizeof(TCHAR));
      lpszPrevName = (LPTSTR)tbGlobalLock(ghPrev);
      _tcscpy(lpszPrevName, lpszName);
 
      Alloc(&ghCustomNames, lCustomNamesLen+(len+1)*sizeof(TCHAR));
      lpb = (LPBYTE)tbGlobalLock(ghCustomNames);
      _tcscpy((LPTSTR)(lpb+lCustomNamesLen), lpszName);
      lCustomNamesLen += (len+1)*sizeof(TCHAR);
      tbGlobalUnlock(ghCustomNames);
      tbGlobalUnlock(gh);
      tbGlobalFree(gh);
    }
    if (ghPrev != 0)
    {
      tbGlobalUnlock(ghPrev);
      tbGlobalFree(ghPrev);
    }
  }
#endif

  
// Worksheet section
#ifdef SS_V70
  if (szSheetName == NULL)
  {
    startSheet = 0;
    endSheet = lpBook->nSheetCnt;
  }
  else
  {
    startSheet = lpBook->nSheet;
    endSheet = (short)(lpBook->nSheet + 1);
  }
    
  for (i=startSheet; i<endSheet; i++)
#endif
  {
    //iterate through the list of Sheets.

    //Buffer the Worksheet data until all the offsets are resolved.
    TBGLOBALHANDLE  ghWorkSheet2 = (TBGLOBALHANDLE)0; // INDEX record buffer
    long          lWorkSheet2Len = 0;
    TBGLOBALHANDLE  ghWorkSheet3 = (TBGLOBALHANDLE)0; // Misc sheet records buffer
    long          lWorkSheet3Len = 0;
    TBGLOBALHANDLE  ghWorkSheet4 = (TBGLOBALHANDLE)0; // DIMENSIONS record buffer
    long          lWorkSheet4Len = 0;
    TBGLOBALHANDLE  ghWorkSheet5 = (TBGLOBALHANDLE)0; // ROW records buffer
    long          lWorkSheet5Len = 0;
#ifndef V7_TEST
    TBGLOBALHANDLE  ghWorkSheet6 = (TBGLOBALHANDLE)0; // DBCELL record buffer
    long          lWorkSheet6Len = 0;
#endif
    TBGLOBALHANDLE  ghCells = (TBGLOBALHANDLE)0;      // cell records buffer
    long          lCellsLen = 0;
    TBGLOBALHANDLE  ghFormula = (TBGLOBALHANDLE)0;    // formula record buffer
    long          lFormulaLen = 0;
    TBGLOBALHANDLE  ghColInfo = (TBGLOBALHANDLE)0;
    long          lColInfoLen = 0;
    LPBYTE        lpPos = NULL;
#ifndef V7_TEST
    LPINDEX8      lpIndex = NULL;
#endif
    long          lIndexPos = 0;
    long          lDBCellCount = 0;
    long          lMaxRows;
    long          lMaxCols;
    long          lRow = 0;
    long          lCol = 0;
		//>>99918069 Leon 20100928
		long					lRowViewport = 0;
		long					lColViewport = 0;
		//<<99918069 Leon 20100928
    short         sRowCount = 0;
    long          lDefColWidthPos = 0;
    DWORD         dwFirstRow = (DWORD)-1;
    DWORD         dwLastRow = (DWORD)-1;
    WORD          wFirstCol = (WORD)-1;
    WORD          wLastCol = (WORD)-1;
    BOOL          fForceRowToBeAdded = FALSE; //Set if a cell in this row is found to be non-Default.
    BOOL          fIsFormula = FALSE;
    APPLYCELLTYPE ac;
    long          lRet = 0;
    BOOL          fNoVirtual = TRUE;
    BOOL          fContinueProcessing = TRUE;
    BOOL          fAtVirtualEnd = FALSE;
    SS_COORD      RowAt = 1;
    double        dblVal = 0;
    TBGLOBALHANDLE ghVal = (TBGLOBALHANDLE)0;
    BOOL          fColInfo = FALSE;
    LPSPREADSHEET lpSS;
    short         sSheetXFIndex = -1;
#ifdef SS_V70
    lpBook->nSheet = i;
#endif
    lpSS = SS_BookLockSheetIndex(lpBook, lpBook->nSheet);
    
    lMaxRows = SS_GetRowCnt(lpSS) - lpSS->Row.HeaderCnt;
    lMaxCols = SS_GetColCnt(lpSS) - lpSS->Col.HeaderCnt;
		
		//>>99918069 Leon 20100928
		lRowViewport = lpSS->Row.HeaderCnt - 1;
		lColViewport = lpSS->Col.HeaderCnt - 1;
		//<<99918069 Leon 20100928

		//>>99918717 Leon 20101109
		lRow = lRowViewport;
		lCol = lColViewport;
		//<<99918717 Leon 20101109

    if (lMaxCols > EXCEL_MAXCOLS)
    {
      xl_LogFile(lpBook, LOG_TOOMANYCOLS, 0, 0, &lMaxCols);
      lMaxCols = EXCEL_MAXCOLS;
    }
//    else
//      lMaxCols -= lpSS->Col.HeaderCnt;

    if (lMaxRows > EXCEL_MAXROWS)
    {
      xl_LogFile(lpBook, LOG_TOOMANYROWS, 0, 0, &lMaxRows);
      lMaxRows = EXCEL_MAXROWS;
    }
//    else
//      lMaxRows -= lpSS->Row.HeaderCnt;


    biff.recnum = xlBOF_BIFF5_7_8;
    biff.datalen = sizeof(zBOF);
    sBOFType = 1;
    ssm_WriteBuffer((TBGLOBALHANDLE FAR *)&ghWorkSheet, &biff, (LPVOID)&sBOFType, &lWorkSheetLen, NULL);
//    ssm_WriteBuffer((TBGLOBALHANDLE FAR *)&ghWorkSheet, &biff, lp, &lWorkSheetLen, NULL);

    lIndexPos = lWorkSheetLen;
    biff.recnum = xlINDEX;
    biff.datalen = 16;
    ssm_WriteBuffer((TBGLOBALHANDLE FAR *)&ghWorkSheet2, &biff, lp, &lWorkSheet2Len, NULL);

    biff.recnum = xlCALCMODE;
    biff.datalen = 2;
    ssm_WriteBuffer((TBGLOBALHANDLE FAR *)&ghWorkSheet3, &biff, lp, &lWorkSheet3Len, NULL);

    biff.recnum = xlCALCCOUNT;
    biff.datalen = 2;
    ssm_WriteBuffer((TBGLOBALHANDLE FAR *)&ghWorkSheet3, &biff, lp, &lWorkSheet3Len, NULL);

    biff.recnum = xlREFMODE;
    biff.datalen = 2;
    ssm_WriteBuffer((TBGLOBALHANDLE FAR *)&ghWorkSheet3, &biff, lp, &lWorkSheet3Len, NULL);

    biff.recnum = xlITERATION;
    biff.datalen = 2;
    ssm_WriteBuffer((TBGLOBALHANDLE FAR *)&ghWorkSheet3, &biff, lp, &lWorkSheet3Len, NULL);

    biff.recnum = xlDELTA;
    biff.datalen = 8;
    ssm_WriteBuffer((TBGLOBALHANDLE FAR *)&ghWorkSheet3, &biff, lp, &lWorkSheet3Len, NULL);

    biff.recnum = xlSAVERECALC;
    biff.datalen = 2;
    ssm_WriteBuffer((TBGLOBALHANDLE FAR *)&ghWorkSheet3, &biff, lp, &lWorkSheet3Len, NULL);

//#ifndef V7_TEST
    biff.recnum = xlPRINTHEADERS;
    biff.datalen = 2;
    ssm_WriteBuffer((TBGLOBALHANDLE FAR *)&ghWorkSheet3, &biff, lp, &lWorkSheet3Len, NULL);

    biff.recnum = xlPRINTGRIDLINES;
    biff.datalen = 2;
    ssm_WriteBuffer((TBGLOBALHANDLE FAR *)&ghWorkSheet3, &biff, lp, &lWorkSheet3Len, NULL);

    biff.recnum = xlGRIDSET;
    biff.datalen = 2;
    ssm_WriteBuffer((TBGLOBALHANDLE FAR *)&ghWorkSheet3, &biff, lp, &lWorkSheet3Len, NULL);

    biff.recnum = xlGUTS;
    biff.datalen = 8;
    ssm_WriteBuffer((TBGLOBALHANDLE FAR *)&ghWorkSheet3, &biff, lp, &lWorkSheet3Len, NULL);
//#endif
    biff.recnum = xlDEFAULTROWHEIGHT;
    biff.datalen = 4;
    ssm_WriteBuffer((TBGLOBALHANDLE FAR *)&ghWorkSheet3, &biff, lp, &lWorkSheet3Len, NULL);

    biff.recnum = xlWSBOOL;
    biff.datalen = 2;
    ssm_WriteBuffer((TBGLOBALHANDLE FAR *)&ghWorkSheet3, &biff, lp, &lWorkSheet3Len, NULL);

    biff.recnum = xlHORIZONTALPAGEBREAKS;
    biff.datalen = 2;
    ssm_WriteBuffer((TBGLOBALHANDLE FAR *)&ghWorkSheet3, &biff, lp, &lWorkSheet3Len, NULL);

    biff.recnum = xlVERTICALPAGEBREAKS;
    biff.datalen = 2;
    ssm_WriteBuffer((TBGLOBALHANDLE FAR *)&ghWorkSheet3, &biff, lp, &lWorkSheet3Len, NULL);

//#ifndef V7_TEST
    biff.recnum = xlHEADER;
    biff.datalen = 0;
    ssm_WriteBuffer((TBGLOBALHANDLE FAR *)&ghWorkSheet3, &biff, lp, &lWorkSheet3Len, NULL);

    biff.recnum = xlFOOTER;
    biff.datalen = 0;
    ssm_WriteBuffer((TBGLOBALHANDLE FAR *)&ghWorkSheet3, &biff, lp, &lWorkSheet3Len, NULL);

    biff.recnum = xlHCENTER;
    biff.datalen = 2;
    ssm_WriteBuffer((TBGLOBALHANDLE FAR *)&ghWorkSheet3, &biff, lp, &lWorkSheet3Len, NULL);

    biff.recnum = xlVCENTER;
    biff.datalen = 2;
    ssm_WriteBuffer((TBGLOBALHANDLE FAR *)&ghWorkSheet3, &biff, lp, &lWorkSheet3Len, NULL);

    biff.recnum = xlLEFTMARGIN;
    biff.datalen = 8;
    ssm_WriteBuffer((TBGLOBALHANDLE FAR *)&ghWorkSheet3, &biff, lp, &lWorkSheet3Len, NULL);

    biff.recnum = xlTOPMARGIN;
    biff.datalen = 8;
    ssm_WriteBuffer((TBGLOBALHANDLE FAR *)&ghWorkSheet3, &biff, lp, &lWorkSheet3Len, NULL);

    biff.recnum = xlRIGHTMARGIN;
    biff.datalen = 8;
    ssm_WriteBuffer((TBGLOBALHANDLE FAR *)&ghWorkSheet3, &biff, lp, &lWorkSheet3Len, NULL);

    biff.recnum = xlBOTTOMMARGIN;
    biff.datalen = 8;
    ssm_WriteBuffer((TBGLOBALHANDLE FAR *)&ghWorkSheet3, &biff, lp, &lWorkSheet3Len, NULL);

    biff.recnum = xlSETUP;
    biff.datalen = 34;
    ssm_WriteBuffer((TBGLOBALHANDLE FAR *)&ghWorkSheet3, &biff, lp, &lWorkSheet3Len, NULL);
//#endif
    biff.recnum = xlPROTECT;
    biff.datalen = 2;
// hatakeyama@boc.co.jp 11/xx-2000 for Seet Protect.
    wProtect = (WORD)SSGetBool(lpBook->hWnd, SSB_PROTECT);
    ssm_WriteBuffer((TBGLOBALHANDLE FAR *)&ghWorkSheet3, &biff, lp, &lWorkSheet3Len, (LPVOID)&wProtect);

    lDefColWidthPos = lWorkSheet3Len;
    biff.recnum = xlDEFCOLWIDTH;
    biff.datalen = 2;
    ssm_WriteBuffer((TBGLOBALHANDLE FAR *)&ghWorkSheet3, &biff, lp, &lWorkSheet3Len, NULL);
   
// hatakeyama@boc.co.jp	11/29-2000	GPF for clipbord copy in excel to excel  
// Prevents GPF when Copy and Paste are used with Excel sheets.
// Is there any reason why this has been commented out?
    //Add the XF record for the entire Spreadsheet
/*
    biff.recnum = xlXF;
    biff.datalen = 20;
    ssm_BuildXF(SS_ALLCOLS, SS_ALLROWS, 1, &sXFIndex, &sXFCount, (TBGLOBALHANDLE FAR *)&ghXF, &biff, lp, &lXFLen, NULL, (TBGLOBALHANDLE FAR *)&ghFont, (LPLONG)&lFontLen);
*/
    //Add the XF record for the entire Spreadsheet
//    biff.recnum = xlXF;
//    biff.datalen = 20;
//    ssm_BuildXF(SS_ALLCOLS, SS_ALLROWS, 1, &sXFIndex, &sXFCount, (TBGLOBALHANDLE FAR *)&ghXF, &biff, lp, &lXFLen, NULL, (TBGLOBALHANDLE FAR *)&ghFont, (LPLONG)&lFontLen);
  
// Work-in-progress: GRB7247
//   *****!!! Remove the ssm_BuildXF() above this comment !!!*****

    // Check the "Entire Spreadsheet" settings.
    // Add the XF record for the entire Spreadsheet
    ac.lpnFormatCount = &sFormatCount;
    ac.lpnXFIndex = &sXFIndex;
    ac.lpnXFCount = &sXFCount;
    ac.lpghFormat = &ghFormat;
    ac.lplFormatLen = &lFormatLen;
    ac.lpghXF = &ghXF;
    ac.lplXFLen = &lXFLen;
    ac.lpghFont = &ghFont;
    ac.lplFontLen = &lFontLen;
    ac.lpghCells = &ghCells;
    ac.lplCellsLen = &lCellsLen;
    ac.lpghSST = &ghSST;
    ac.lplSSTLen = &lSSTLen;
    ac.lpghEXTSST = &ghEXTSST;
    ac.lplEXTSSTLen = &lEXTSSTLen;
    lRet = ssm_ApplyCelltype(lpSS, SS_ALLCOLS, SS_ALLROWS, fIsFormula, lp, TRUE, FALSE, &dblVal, &ghVal, &ac, &ghStrings);
    sSheetXFIndex = sXFIndex;  
//RAP25a >>
    yCOLINFO ci;
//    memset((LPyCOLINFO)&ci, 0, sizeof(yCOLINFO));
//
//    LPBYTE lpb = (LPBYTE)tbGlobalLock(ghXF);
//    LPxXF8 lpxf = (LPxXF8)(lpb + sXFIndex*sizeof(xXF8) + (sXFIndex+1)*sizeof(BIFF));
//    if (lpxf->fiBackColor != 0x40 || lpxf->fiForeColor != 0x41 || lpxf->fAtrBdr)
//    {
//      ci.wColFirst = (WORD)0;
//      ci.wColLast = (WORD)256;
//      ci.wixfe = sXFIndex;
//      biff.recnum = xlCOLINFO;
//      biff.datalen = 12;
//      ssm_WriteBuffer((TBGLOBALHANDLE FAR *)&ghColInfo, &biff, lp, &lColInfoLen, (LPVOID)&ci);
//    }

//<< RAP25a
    TBGLOBALHANDLE ghIsColDefaultDrawing = 0;
    LPBOOL         lpIsColDefaultDrawing = NULL;
    TBGLOBALHANDLE ghIsColFormula = 0;
    LPBOOL         lpIsColFormula = NULL;

    Alloc(&ghIsColDefaultDrawing, lMaxCols*sizeof(BOOL));
    lpIsColDefaultDrawing = (LPBOOL)tbGlobalLock(ghIsColDefaultDrawing);

#ifdef SS_V70
    if ((sFlags & SS_EXCELSAVEFLAG_NOFORMULAS) != SS_EXCELSAVEFLAG_NOFORMULAS)
#endif
    {
      Alloc(&ghIsColFormula, lMaxCols*sizeof(BOOL));
      lpIsColFormula = (LPBOOL)tbGlobalLock(ghIsColFormula);
    }

#if SS_V80    
		//>>99918069 Leon 20100928
    lColViewport = SS_GetNextNonEmptyColumnInRow(lpSS, SS_ALLROWS, lColViewport);
		lCol =lColViewport==-1 ? -1 : lColViewport - lpSS->Col.HeaderCnt + 1;
		//<<99918069 Leon 20100928
    // 26510 -scl
    //while (lCol != -1)
    while (lCol != -1 && lCol <= lMaxCols)
#else
    for (lCol=1; lCol<=lMaxCols; lCol++)
#endif
    {
      // Check to see if any custom column headers exist
      LPSS_CELL lpCell = SS_LockCellItem(lpSS, lCol+lpSS->Col.HeaderCnt-1, 0);
      if (lpCell)
      {
        if (lpCell->Data.bDataType)
        {
          xl_LogFile(lpBook, LOG_CUSTHEADERNOTSUPPORTED, lCol, 0, NULL);
        }
        SS_UnlockCellItem(lpSS, lCol+lpSS->Col.HeaderCnt-1, 0);
      }
      // Go thru the Spread columns and check if each column is default. If a column is not
      // default, then write the COLINFO record for that column.
      lpIsColDefaultDrawing[lCol-1] = ssm_IsColDefaultDrawing(lpSS, lCol); 
#ifdef SS_V70
      if ((sFlags & SS_EXCELSAVEFLAG_NOFORMULAS) != SS_EXCELSAVEFLAG_NOFORMULAS)
        fIsFormula = lpIsColFormula[lCol-1] = ssm_GetFormula(lpSS, lCol, 0, &ghFormula, &lFormulaLen, &sxtiCount, &ghXTI, &ghCustomNames, &lCustomNamesLen);
      else
        fIsFormula = FALSE;

      if ((fIsFormula && (sFlags & SS_EXCELSAVEFLAG_NOFORMULAS) != SS_EXCELSAVEFLAG_NOFORMULAS) || !ssm_IsColDefault(lpSS, lCol))
#else
      fIsFormula = ssm_GetFormula(lpSS, lCol, 0, &ghFormula, &lFormulaLen);
      
      if (fIsFormula || !ssm_IsColDefault(lpSS, lCol))
#endif
      {
        LPSS_COL lpCol = NULL;

        fColInfo = TRUE;
        
        memset((LPyCOLINFO)&ci, 0, sizeof(yCOLINFO));
        
        // Write-out the column format
        ac.lpnFormatCount = &sFormatCount;
        ac.lpnXFIndex = &sXFIndex;
        ac.lpnXFCount = &sXFCount;
        ac.lpghFormat = &ghFormat;
        ac.lplFormatLen = &lFormatLen;
        ac.lpghXF = &ghXF;
        ac.lplXFLen = &lXFLen;
        ac.lpghFont = &ghFont;
        ac.lplFontLen = &lFontLen;
        ac.lpghCells = &ghCells;
        ac.lplCellsLen = &lCellsLen;
        ac.lpghSST = &ghSST;
        ac.lplSSTLen = &lSSTLen;
        ac.lpghEXTSST = &ghEXTSST;
        ac.lplEXTSSTLen = &lEXTSSTLen;
        lRet = ssm_ApplyCelltype(lpSS, lCol, SS_ALLROWS, fIsFormula, lp, TRUE, FALSE, &dblVal, &ghVal, &ac, &ghStrings);
//        lRet = ssm_ApplyCelltype(lpSS, lCol, SS_ALLROWS, fIsFormula, lp, TRUE, TRUE, &dblVal, &ghVal, &ac);

        if (lRet == 2)
        {
#if SS_V80
					//>>99918069 Leon 20100928
					lColViewport = SS_GetNextNonEmptyColumnInRow(lpSS, SS_ALLROWS, lColViewport);
					lCol =lColViewport==-1?-1: lColViewport - lpSS->Col.HeaderCnt + 1;
					//<<99918069 Leon 20100928
#endif
          continue;
        }

        if (fIsFormula)
        {
          if (lFormulaLen)
          {
            ssm_AddFormula(lpBook, lCol, SS_ALLROWS, ghFormula, lFormulaLen, sXFIndex, &dblVal, &ghVal, (TBGLOBALHANDLE FAR *)&ghCells, &lCellsLen);
            fIsFormula = FALSE; //RAP03a
          }
          else
          {
            //this is URL
          }

          if (ghFormula)
          {
            tbGlobalFree(ghFormula);
            ghFormula = (TBGLOBALHANDLE)0;
          }  
        }
        
        ci.wColFirst = (WORD)(lCol-1);
        ci.wColLast = (WORD)(lCol-1);
        lpCol = (LPSS_COL)SS_LockColItem(lpSS, lCol+lpSS->Col.HeaderCnt-1);
        if (lpCol)
        {
          ci.fHidden = lpCol->fColHidden;
          SS_UnlockColItem(lpSS, lCol+lpSS->Col.HeaderCnt-1); 
        }
        ci.wixfe = sXFIndex;
        biff.recnum = xlCOLINFO;
        biff.datalen = 12;
//RAP25d        ssm_WriteBuffer((TBGLOBALHANDLE FAR *)&ghWorkSheet3, &biff, lp, &lWorkSheet3Len, (LPVOID)&ci);
        ssm_WriteBuffer((TBGLOBALHANDLE FAR *)&ghColInfo, &biff, lp, &lColInfoLen, (LPVOID)&ci); //RAP25a
      }

#if SS_V80
			//>>99918069 Leon 20100928
      lColViewport = SS_GetNextNonEmptyColumnInRow(lpSS, SS_ALLROWS, lColViewport);
			lCol = lColViewport==-1? -1 : lColViewport + lpSS->Col.HeaderCnt + 1;
			//<<99918069 Leon 20100928
#endif
    }

//RAP25a >>
//    if (fColInfo)
    {
      // Go through the ColInfo buffer and merge/compact the colinfo records
      LPBYTE     lpb = (LPBYTE)tbGlobalLock(ghColInfo);
      LPyCOLINFO lpCIi = NULL;
      LPyCOLINFO lpCIj = NULL;
      short      nColInfoRecLen = sizeof(yCOLINFO)+sizeof(BIFF);
      short      nColInfoCount = (short)(lColInfoLen/nColInfoRecLen);
      BOOL       afColInfoDelete[300];
      short      nNextCol = 0;
      //short      xf = 0x15;
      int        k;
//      short      xf = sXFIndex;

      memset(&afColInfoDelete, 0, nColInfoCount*sizeof(BOOL));       
      biff.recnum = xlCOLINFO;
      biff.datalen = 12;

      // The first entry is for the sheet, columns 0-256.
//      if (nColInfoCount > 1)
//      {
//        lpCIi = (LPyCOLINFO)lpb;
//        lpCIiCol = (LPyCOLINFO)(lpb + nColInfoRecLen + sizeof(BIFF));  
//
//        if (lpCIiCol->wColFirst == 0)
//        {
//          yCOLINFO colInfo;
//          lpCIi->wColFirst = lpCIiCol->wColLast+1;
//          memcpy(&colInfo, lpCIiCol, sizeof(yCOLINFO));
//          memcpy(lpCIiCol, lpCIi, sizeof(yCOLINFO));
//          memcpy(lpCIi, &colInfo, sizeof(yCOLINFO));
//        }
//      }
      for (k=0; k<nColInfoCount; k++)
      {
        if (!afColInfoDelete[k])
        {
          lpCIi = (LPyCOLINFO)(lpb + (k*nColInfoRecLen) + sizeof(BIFF));
          for (int j=k+1; j<nColInfoCount; j++)
          {
            lpCIj = (LPyCOLINFO)(lpb + (j*nColInfoRecLen) + sizeof(BIFF));
            if (lpCIi->wColLast+1 == lpCIj->wColFirst && 
                !memcmp(((LPBYTE)lpCIi)+2*sizeof(WORD), ((LPBYTE)lpCIj)+2*sizeof(WORD), sizeof(yCOLINFO)-2*sizeof(WORD)))
            {
              // these 2 ColInfo records can be combined.
              lpCIi->wColLast = lpCIj->wColLast;
              afColInfoDelete[j] = TRUE;
            }
            else
              break;
          }
        }
      }
      
      for (k=0; k<nColInfoCount; k++)
      {
        if (!afColInfoDelete[k])
        {
          lpCIi = (LPyCOLINFO)(lpb + (k*nColInfoRecLen) + sizeof(BIFF));
//RAP 13816D - 3/5/2004
          if (lpCIi->wColFirst != nNextCol)
          {
            memset((LPyCOLINFO)&ci, 0, sizeof(yCOLINFO));
            ci.wixfe = sSheetXFIndex;
            ci.wColFirst = nNextCol;
            ci.wColLast = (WORD)(lpCIi->wColFirst-1);
            ssm_WriteBuffer((TBGLOBALHANDLE FAR *)&ghWorkSheet3, &biff, lp, &lWorkSheet3Len, (LPVOID)&ci);
          }
          
          nNextCol = (short)min(256, (lpCIi->wColLast+1));
//RAP 13816D
          if (lpCIi->wColLast == 255)
            lpCIi->wColLast++; // I don't know why, but this is what Excel does...
  
          ssm_WriteBuffer((TBGLOBALHANDLE FAR *)&ghWorkSheet3, &biff, lp, &lWorkSheet3Len, (LPVOID)lpCIi);
        }      
      }

///* RAP: I don't think this is needed.  Since each column is inspected for default, it should work out allright.
//17472      if (nNextCol != 256)
      if (nNextCol != 256 && (sSheetXFIndex != -1 || nColInfoCount > 0)) //17472
      {
//        LPyCOLINFO lpCI0 = (LPyCOLINFO)(lpb + sizeof(BIFF));
        memset((LPyCOLINFO)&ci, 0, sizeof(yCOLINFO));
        ci.wixfe = sSheetXFIndex;
        ci.wColFirst = nNextCol;
        ci.wColLast = 256;
        ssm_WriteBuffer((TBGLOBALHANDLE FAR *)&ghWorkSheet3, &biff, lp, &lWorkSheet3Len, (LPVOID)&ci);
      }  
//*/
    }
    
    if (ghColInfo)
    {
      tbGlobalUnlock(ghColInfo);
      tbGlobalFree(ghColInfo);
      ghColInfo = (TBGLOBALHANDLE)0;
      lColInfoLen = 0;      
    }
//<< RAP25a

    biff.recnum = xlDIMENSIONS;
    biff.datalen = 14;
    ssm_WriteBuffer((TBGLOBALHANDLE FAR *)&ghWorkSheet4, &biff, lp, &lWorkSheet4Len, NULL);

//RAP06a >>
//RAP14d    if (lpSS->fVirtualMode)
    int  nVirtualMax = 1000000;
    bool isVirtual = FALSE;
    if (lpSS->Virtual.VMax != -1)
      nVirtualMax = lpSS->Virtual.VMax;
    if (lpSS->fVirtualMode && nVirtualMax != 0) //RAP14a
    {    
      lMaxRows = 0;
      RowAt = 1;
    }
    else
      lRow = 1;

    while (fContinueProcessing && lMaxRows <= 65536)
    {
      if (lpSS->fVirtualMode && nVirtualMax != 0 && lpSS->Row.Max && !fAtVirtualEnd)
      {
        fNoVirtual = FALSE;
        lRow = RowAt;
        isVirtual = TRUE;
        while ((lRow < lpSS->Virtual.VTop ||
                lRow >= lpSS->Virtual.VTop +
                lpSS->Virtual.VPhysSize) && !fAtVirtualEnd)
        {
          lpSS->Row.UL = RowAt;
          fContinueProcessing = TRUE;
          fAtVirtualEnd = SS_VQueryData(lpSS, RowAt);
          if (lpSS->Virtual.VTop + lpSS->Virtual.VPhysSize - lpSS->Row.HeaderCnt >= nVirtualMax)
            fAtVirtualEnd = TRUE;
        }

        if (lpSS->Virtual.VSize == 0)
          lMaxRows++;
        else
          lMaxRows += lpSS->Virtual.VSize;
        lMaxRows = min(lMaxRows, 65536);
      }
      else
        fContinueProcessing = FALSE;

//<< RAP06a
//RAP06d    for (lRow = 1; lRow <= lMaxRows; lRow++)


#if SS_V80
			//>>99918069 Leon 20100928
			lRowViewport = SS_GetNextNonEmptyRow(lpSS, lRowViewport);
      lRow = lRowViewport==-1? -1 : lRowViewport - lpSS->Row.HeaderCnt + 1;
			//<<99918069 Leon 20100928
      // 26510
      //while (lRow != -1)
      while (lRow != -1 && lRow <= lMaxRows)
#else
      for (; lRow <= lMaxRows; lRow++)  //RAP06a
#endif
      {
        // Check to see if any custom row headers exist
        LPSS_CELL lpCell = SS_LockCellItem(lpSS, 0, lRow+lpSS->Row.HeaderCnt-1);
        if (lpCell)
        {
          if (lpCell->Data.bDataType)
          {
            xl_LogFile(lpBook, LOG_CUSTHEADERNOTSUPPORTED, 0, lRow, NULL);
          }
          SS_UnlockCellItem(lpSS, 0, lRow+lpSS->Row.HeaderCnt-1);
        }
        // Go thru the Spread rows and check if each row is default. If a row is not
        // default, then write the ROW record for that row.
        long    lCol;
        BOOL    fRowAdded = FALSE;
        BOOL    fCellAdded = FALSE;
 
        // If the row contains a formula, is non-default, or contains a formatted cell,
        // write the Row record.
#ifdef SS_V70
        if ((sFlags & SS_EXCELSAVEFLAG_NOFORMULAS) != SS_EXCELSAVEFLAG_NOFORMULAS)
          fIsFormula = ssm_GetFormula(lpSS, 0, lRow, &ghFormula, &lFormulaLen, &sxtiCount, &ghXTI, &ghCustomNames, &lCustomNamesLen);
        else
          fIsFormula = FALSE;
        
        if ((fIsFormula && (sFlags & SS_EXCELSAVEFLAG_NOFORMULAS) != SS_EXCELSAVEFLAG_NOFORMULAS) || !ssm_IsRowDefault(lpSS, lRow) || fForceRowToBeAdded)
#else
        fIsFormula = ssm_GetFormula(lpSS, 0, lRow, &ghFormula, &lFormulaLen);

        if (fIsFormula || !ssm_IsRowDefault(lpSS, lRow) || fForceRowToBeAdded)
#endif
        {
          yROW     row;
          LPSS_ROW lpRow = NULL;

          _fmemset((LPyROW)&row, 0, sizeof(yROW));
          fForceRowToBeAdded = FALSE;
          fRowAdded = TRUE;
        
          if (!fForceRowToBeAdded)
            row.fGhostDirty = 1; //flag the row as formatted!!!

          ac.lpnFormatCount = &sFormatCount;
          ac.lpnXFIndex = &sXFIndex;
          ac.lpnXFCount = &sXFCount;
          ac.lpghFormat = &ghFormat;
          ac.lplFormatLen = &lFormatLen;
          ac.lpghXF = &ghXF;
          ac.lplXFLen = &lXFLen;
          ac.lpghFont = &ghFont;
          ac.lplFontLen = &lFontLen;
          ac.lpghCells = &ghCells;
          ac.lplCellsLen = &lCellsLen;
          ac.lpghSST = &ghSST;
          ac.lplSSTLen = &lSSTLen;
          ac.lpghEXTSST = &ghEXTSST;
          ac.lplEXTSSTLen = &lEXTSSTLen;
          lRet = ssm_ApplyCelltype(lpSS, SS_ALLCOLS, lRow, fIsFormula, lp, TRUE, FALSE, &dblVal, &ghVal, &ac, &ghStrings);

          if (lRet == 2)
          {
#if SS_V80
						//>>99918069 Leon 20100928
            lRowViewport = SS_GetNextNonEmptyRow(lpSS, lRowViewport);
						lRow = lRowViewport==-1? -1 : lRowViewport - lpSS->Row.HeaderCnt + 1;
						//<<99918069 Leon 20100928
#endif
            continue;
          }

          if (fIsFormula)
          {
            if (lFormulaLen)
            {
              ssm_AddFormula(lpBook, SS_ALLCOLS, lRow, ghFormula, lFormulaLen, sXFIndex, &dblVal, &ghVal, (TBGLOBALHANDLE FAR *)&ghCells, &lCellsLen);
              fIsFormula = FALSE; //RAP03a
            }
            else
            {
              //this is URL
            }
            if (ghFormula)
            {
              tbGlobalFree(ghFormula);
              ghFormula = (TBGLOBALHANDLE)0;
            }  
          }
       
          biff.recnum = xlROW;
          biff.datalen = 16;
          row.wRow = (WORD)(lRow-1);
          row.wixfe = sXFIndex;

          // res3, although it is a reserved field, appears to serve as
          // a flag to tell Excel to use the border info in the XF record
          // for the row.  This same field is used to tell Excel to use the
          // backcolor in the XF record for the row -- this code is in xlbiff.cpp.
          lpRow = SS_LockRowItem(lpSS, lRow+lpSS->Row.HeaderCnt-1);
          if (lpRow)
          {
            if (lpRow->hBorder)
              row.res3 = 1; // this little undocumented treat forces the 
                          // row border specified in the XF to be used.
            SS_UnlockRowItem(lpSS, lRow+lpSS->Row.HeaderCnt-1);
          }

          // add to the row buffer
//21210 >>
          TBGLOBALHANDLE ghRow = (TBGLOBALHANDLE)0;
          long lRowLen = 0;
          ssm_WriteBuffer((TBGLOBALHANDLE FAR *)&ghRow, &biff, lp, &lRowLen, (LPVOID)&row);
          xl_CatWSToWS(&ghWorkSheet5, (LPLONG)&lWorkSheet5Len, 
                       (LPTBGLOBALHANDLE)&ghRow, (LPLONG)&lRowLen);
//<< 21210

//21210        ssm_WriteBuffer((TBGLOBALHANDLE FAR *)&ghWorkSheet5, &biff, lp, &lWorkSheet5Len, (LPVOID)&row);
          sRowCount++;
  
          if (dwFirstRow == (DWORD)-1 || lRow < (long)dwFirstRow)
            dwFirstRow = (DWORD)lRow;
          if (dwLastRow == (DWORD)-1 || lRow > (long)dwLastRow)
            dwLastRow = (DWORD)lRow;

          // If this is the first row in the "row-block", write a DBCELL record to a buffer and
          // initialize it.
#ifndef V7_TEST
          if (sRowCount == 1)
          {
            // write DBCELL record to buffer6
            biff.recnum = xlDBCELL;
            biff.datalen = 4;
            ssm_WriteBuffer((TBGLOBALHANDLE FAR *)&ghWorkSheet6, &biff, lp, &lWorkSheet6Len, NULL);
          }
#endif
        }
#if SS_V80
        //>>99918069 Leon 20100928
				lColViewport = SS_GetNextNonEmptyColumnInRow(lpSS, lRowViewport, lpSS->Col.HeaderCnt -1);
        lCol = lColViewport==-1? -1 : lColViewport - lpSS->Col.HeaderCnt + 1;
				//<<99918069 Leon 20100928
        // 26510 -scl
        //while (lCol != -1)
        while (lCol != -1 && lCol <= lMaxCols)
#else
        for (lCol=1; lCol<=lMaxCols; lCol++)
#endif
        {
          // Go thru the cells in this row to determine if there are cell records
          // to be added.
#ifndef V7_TEST
          long lSaveCellsLen = lCellsLen; // use this to set determine the DBCELL offset.
#endif
          BOOL fIsColFormula = FALSE;
          BOOL fIsRowFormula = FALSE;
#ifdef SS_V70
          if ((sFlags & SS_EXCELSAVEFLAG_NOFORMULAS) != SS_EXCELSAVEFLAG_NOFORMULAS)
          {
            fIsColFormula = lpIsColFormula[lCol-1];
            if (fIsColFormula && lFormulaLen == 0)
              fIsColFormula = FALSE;
            fIsRowFormula = ssm_GetFormula(lpSS, 0, lRow, &ghFormula, &lFormulaLen, &sxtiCount, &ghXTI, &ghCustomNames, &lCustomNamesLen);
            if (fIsRowFormula && lFormulaLen == 0)
              fIsRowFormula = FALSE;
          }
#else
          fIsColFormula = ssm_GetFormula(lpSS, lCol, 0, &ghFormula, &lFormulaLen);
          if (fIsColFormula && lFormulaLen == 0)
            fIsColFormula = FALSE;
          fIsRowFormula = ssm_GetFormula(lpSS, 0, lRow, &ghFormula, &lFormulaLen);
          if (fIsRowFormula && lFormulaLen == 0)
            fIsRowFormula = FALSE;
#endif

          BOOL fIsColDefaultDrawing = lpIsColDefaultDrawing[lCol-1]; 

#ifdef SS_V70
          if ((sFlags & SS_EXCELSAVEFLAG_NOFORMULAS) != SS_EXCELSAVEFLAG_NOFORMULAS)
          {
            fIsFormula = ssm_GetFormula(lpSS, lCol, lRow, &ghFormula, &lFormulaLen, &sxtiCount, &ghXTI, &ghCustomNames, &lCustomNamesLen);
            if (fIsFormula && lFormulaLen == 0)
              fIsFormula = FALSE;
          }
          else
            fIsFormula = FALSE;

// NOTE: This change could cause the saved Excel file to grow too large.  Keep this in mind when debugging!!!
//       This could be an area to redesign in the future...
//        if ((fIsFormula && (sFlags & SS_EXCELSAVEFLAG_NOFORMULAS) != SS_EXCELSAVEFLAG_NOFORMULAS) || fIsColFormula
//            || fIsRowFormula || !ssm_IsCellDefault(lpSS, lCol, lRow) || !fIsColDefaultDrawing)
          if ((fIsFormula && (sFlags & SS_EXCELSAVEFLAG_NOFORMULAS) != SS_EXCELSAVEFLAG_NOFORMULAS) || fIsColFormula
            || fIsRowFormula || !ssm_IsCellDefault(lpSS, lCol, lRow) || !ssm_IsRowDefaultDrawing(lpSS, lRow) || !fIsColDefaultDrawing)
#else
          fIsFormula = ssm_GetFormula(lpSS, lCol, lRow, &ghFormula, &lFormulaLen);
          if (fIsFormula && lFormulaLen == 0)
            fIsFormula = FALSE;

          if (fIsFormula || fIsColFormula || fIsRowFormula ||
              !ssm_IsCellDefault(lpSS, lCol, lRow) || !fIsColDefaultDrawing)
#endif
          {
            fCellAdded = TRUE;

            // If a formatted cell is found, but the row where it exists has not
            // been added to the row buffer, then force the row to be added.
            if (!fRowAdded)
            {  
              //break from this loop and force the row to be added first.
              fForceRowToBeAdded = TRUE;
              lRow--;
							lRowViewport--; //#99918069 Leon 20100928
              if (ghFormula)
              {
                tbGlobalFree(ghFormula);
                ghFormula = (TBGLOBALHANDLE)0;
              }  
              
              break;
            }
  
            ac.lpnFormatCount = &sFormatCount;
            ac.lpnXFIndex = &sXFIndex;
            ac.lpnXFCount = &sXFCount;
            ac.lpghFormat = &ghFormat;
            ac.lplFormatLen = &lFormatLen;
            ac.lpghXF = &ghXF;
            ac.lplXFLen = &lXFLen;
            ac.lpghFont = &ghFont;
            ac.lplFontLen = &lFontLen;
            ac.lpghCells = &ghCells;
            ac.lplCellsLen = &lCellsLen;
            ac.lpghSST = &ghSST;
            ac.lplSSTLen = &lSSTLen;
            ac.lpghEXTSST = &ghEXTSST;
            ac.lplEXTSSTLen = &lEXTSSTLen;
            lRet = ssm_ApplyCelltype(lpSS, lCol, lRow, fIsFormula, lp, TRUE, TRUE, &dblVal, &ghVal, &ac, &ghStrings);

            if (lRet == 2)
            {
#if SS_V80
							//>>99918069 Leon 20100928
              lColViewport = SS_GetNextNonEmptyColumnInRow(lpSS, lRowViewport, lColViewport);
							lCol = lColViewport==-1? -1 : lColViewport - lpSS->Col.HeaderCnt + 1;
							//<<99918069 Leon 20100928
#endif
              continue;
            }
  
            if (fIsFormula)
            {
              if (lFormulaLen)
              {
                ssm_AddFormula(lpBook, lCol, lRow, ghFormula, lFormulaLen, sXFIndex, &dblVal, &ghVal, (TBGLOBALHANDLE FAR *)&ghCells, &lCellsLen);
                fIsFormula = FALSE; //RAP03a
              }
              else
              {
                //this is URL
                // write HLINK record to HLINK buffer
/* This is well on its way to being implemented.  However, there is no
   documentation on what to send as the "Hyperlink stream."  The docs
   even say that the stream definition is undocumented.
                yHLINK hlink;

                hlink.rwFirst = lRow;
                hlink.rwLast = lRow;
                hlink.colFirst = lCol;
                hlink.colLast = lCol;
  
                if (ghFormula)
                {
                  LPTSTR lpszURL = (LPTSTR)tbGlobalLock(ghFormula);
                  hlink.ghData = 
                }
  
                biff.recnum = xlHLINK;
                biff.datalen = 8;

                ssm_WriteBuffer((TBGLOBALHANDLE FAR *)&ghHlink, &biff, lp, &lHlinkLen, &hlink);
*/
              }
              if (ghFormula)
              {
                tbGlobalFree(ghFormula);
                ghFormula = (TBGLOBALHANDLE)0;
              }  
            }

            // Set the length of the CURRENT cell record in the DBCELL cell record offset
            // array. When the last row and/or cell record is written, or the 32nd row 
            // record is written (Excel block limit) the DBCELL cell record offsets will
            // be resolved.
#ifndef V7_TEST
            lWorkSheet6Len += sizeof(WORD);
            Alloc(&ghWorkSheet6, lWorkSheet6Len);
            lpPos = (LPBYTE)tbGlobalLock(ghWorkSheet6);          
            ((LPBIFF)lpPos)->datalen += sizeof(WORD);
            lpPos = lpPos + lWorkSheet6Len - sizeof(WORD);
            *(LPSHORT)lpPos = (short)(lCellsLen-lSaveCellsLen);
            tbGlobalUnlock(ghWorkSheet6);
#endif
            biff.datalen = 0;

            if (wFirstCol == (WORD)-1 || wFirstCol > lCol)
              wFirstCol = (WORD)lCol;
            if (wLastCol == (WORD)-1 || wLastCol < lCol)
              wLastCol = (WORD)lCol;
          }
#if SS_V80
					//>>99918069 Leon 20100928
          lColViewport = SS_GetNextNonEmptyColumnInRow(lpSS, lRowViewport, lColViewport);
					lCol = lColViewport==-1? -1 : lColViewport - lpSS->Col.HeaderCnt + 1;
					//<<99918069 Leon 20100928
#endif
        }

        if (fRowAdded)
        {
          if (!fCellAdded)
          {
            // If a row does not have formatted cells, it still needs to have a cell record
            // offset added to the DBCELL record.

            // When the last row and/or cell record is written, or the 32nd row 
            // record is written (Excel block limit) the DBCELL cell record offsets will
            // be resolved.
#ifndef V7_TEST
            lWorkSheet6Len += sizeof(WORD);
            Alloc(&ghWorkSheet6, lWorkSheet6Len);
            lpPos = (LPBYTE)tbGlobalLock(ghWorkSheet6);          
            ((LPBIFF)lpPos)->datalen += sizeof(WORD);
            tbGlobalUnlock(ghWorkSheet6);
#endif
          }
          else
            fCellAdded = FALSE;
  
          if (wFirstCol != (WORD)-1)
          {
            // write the ROW info
            LPyROW lpRow;
      
            lpPos = (LPBYTE)tbGlobalLock(ghWorkSheet5);
            lpRow = (LPyROW)(lpPos + lWorkSheet5Len-sizeof(yROW));
//RAP13a >>            
            lpRow->wCol1 = (WORD)(wFirstCol-1);
            lpRow->wCol2 = (WORD)wLastCol;
//<< RAP13a            
            tbGlobalUnlock(ghWorkSheet5);
          }
  
          // If this is the last row in the "row-block" (Excel maximum of 32 rows in a row-block),
          // write the ROW offset to the DBCELL record. This is the offset to the first row in
          // the row-block. Then write the DBCELL offset to the INDEX record. This is the offset 
          // from the BOF record for the Workbook global records to the DBCELL record.
          if (sRowCount == 32)
          {
#ifndef V7_TEST
            // write the ROW offset to the DBCELL record
            lpPos = (LPBYTE)tbGlobalLock(ghWorkSheet6);
            lpPos = lpPos + sizeof(BIFF);
            *(LPLONG)lpPos = lWorkSheet5Len + lCellsLen;
            tbGlobalUnlock(ghWorkSheet6);

            // write the DBCELL offset to the INDEX record
            lWorkSheet2Len += sizeof(DWORD);
            Alloc(&ghWorkSheet2, lWorkSheet2Len);
            lpPos = (LPBYTE)tbGlobalLock(ghWorkSheet2);
            lpIndex = (LPINDEX8)(lpPos + sizeof(BIFF));
            ((LPBIFF)lpPos)->datalen += sizeof(DWORD);
            lpPos = lpPos + lWorkSheet2Len - sizeof(DWORD);
            //Write the temporary offset...
            *(LPLONG)lpPos = lWorkSheet5Len + lCellsLen;
            tbGlobalUnlock(ghWorkSheet2);
#endif
            lDBCellCount++;
          }
        }

#ifndef V7_TEST
        if (sRowCount == 32)
        {
          // Resolve the DBCELL cell record offsets.
          long   lRow2Offset = lWorkSheet5Len - sizeof(yROW) - sizeof(BIFF);
          WORD   wCellRecs = 0;
          WORD   wNextOffset = 0;
          WORD   wTemp = 0;
          LPBIFF lpBiff = NULL;
          short  i;
  
          // Determine the number of cell record offsets in the DBCELL record.
          lpPos = (LPBYTE)tbGlobalLock(ghWorkSheet6);
          wCellRecs = (WORD)((((LPBIFF)lpPos)->datalen - sizeof(DWORD))/sizeof(WORD));
  
          lpPos += sizeof(BIFF) + sizeof(DWORD);

          wNextOffset = *(LPWORD)lpPos;
          *(LPWORD)lpPos = (WORD)lRow2Offset;
          lpPos += sizeof(WORD);

          for (i=0; i<wCellRecs-1; i++)
          {  
            wTemp = *(WORD *)lpPos;
            *(LPWORD)lpPos = wNextOffset;
            wNextOffset = wTemp;
            lpPos += sizeof(WORD);
          }
          tbGlobalUnlock(ghWorkSheet6);

          //RAP05a >>
          //determine if the DBCELL record exceeds the record size maximum...
          lpBiff = (LPBIFF)tbGlobalLock(ghWorkSheet6);
          if (lpBiff->datalen > EXCEL_MAXBIFF8RECLEN)
          {
            long   ndatalen = lpBiff->datalen;
            int    nContinues = ndatalen / EXCEL_MAXBIFF8RECLEN;
            LPBYTE lpPos = NULL;
            int    nMoveSize = 0;

            tbGlobalUnlock(ghWorkSheet6);
            lWorkSheet6Len += nContinues * sizeof(BIFF);
            Alloc(&ghWorkSheet6, lWorkSheet6Len);
            lpBiff = (LPBIFF)tbGlobalLock(ghWorkSheet6);
            lpBiff->datalen = EXCEL_MAXBIFF8RECLEN;
            lpPos = (LPBYTE)lpBiff;
            nMoveSize = ndatalen-EXCEL_MAXBIFF8RECLEN;
            memmove(lpPos+sizeof(BIFF)+EXCEL_MAXBIFF8RECLEN+sizeof(BIFF), 
                    lpPos+sizeof(BIFF)+EXCEL_MAXBIFF8RECLEN, 
                    nMoveSize);
            lpBiff = (LPBIFF)(lpPos+sizeof(BIFF)+EXCEL_MAXBIFF8RECLEN);
            lpBiff->recnum = xlCONTINUE;
            lpBiff->datalen = (WORD)(ndatalen-EXCEL_MAXBIFF8RECLEN);
          }
          tbGlobalUnlock(ghWorkSheet6);
          //<< RAP05a
          //add buffer6(DBCELL) to the end of Cells(cell records)
          xl_CatWSToWS(&ghCells, (LPLONG)&lCellsLen, 
                       (LPTBGLOBALHANDLE)&ghWorkSheet6, (LPLONG)&lWorkSheet6Len);

          //add Cells(cell records & DBCELL) to the end of buffer5(ROW records)
          xl_CatWSToWS(&ghWorkSheet5, (LPLONG)&lWorkSheet5Len, 
                       (LPTBGLOBALHANDLE)&ghCells, (LPLONG)&lCellsLen);

          sRowCount = 0;
        }
#else
        //add Cells(cell records & DBCELL) to the end of buffer5(ROW records)
        xl_CatWSToWS(&ghWorkSheet5, (LPLONG)&lWorkSheet5Len, 
                     (LPTBGLOBALHANDLE)&ghCells, (LPLONG)&lCellsLen);
#endif

//RAP06a >>
        if (lpSS->fVirtualMode)
        {    
//          RowAt++;
          RowAt = lRow+1;
          SS_InvalidateRect(lpBook, NULL, TRUE);
        }
//<< RAP06a
      
#if SS_V80
				//>>99918069 Leon 20100928
        lRowViewport = SS_GetNextNonEmptyRow(lpSS, lRowViewport);
				lRow = lRowViewport==-1? -1 : lRowViewport - lpSS->Row.HeaderCnt + 1;
				//<<99918069 Leon 20100928
#endif
      } // end of ROW for loop
    } //RAP06a
    
    if (sRowCount != 0)
    {
#ifndef V7_TEST
      LPBIFF lpBiff = NULL;

      // write the ROW offset to the DBCELL record
      lpPos = (LPBYTE)tbGlobalLock(ghWorkSheet6);
      lpPos = lpPos + sizeof(BIFF);
      *(LPLONG)lpPos = lWorkSheet5Len + lCellsLen;
      tbGlobalUnlock(ghWorkSheet6);

      // write the DBCELL offset to the INDEX record
      lWorkSheet2Len += sizeof(DWORD);
      Alloc(&ghWorkSheet2, lWorkSheet2Len);
      lpPos = (LPBYTE)tbGlobalLock(ghWorkSheet2);
      lpIndex = (LPINDEX8)(lpPos + sizeof(BIFF));
      ((LPBIFF)lpPos)->datalen += sizeof(DWORD);
      lpPos = lpPos + lWorkSheet2Len - sizeof(DWORD);
      //Write the temporary offset...
      *(LPLONG)lpPos = lWorkSheet5Len + lCellsLen;
      tbGlobalUnlock(ghWorkSheet2);

      lDBCellCount++;

      // Resolve the DBCELL cell record offsets.
      {
        long  lRow2Offset = lWorkSheet5Len - sizeof(yROW) - sizeof(BIFF);
        WORD  wCellRecs = 0;
        WORD  wNextOffset = 0;
        WORD  wTemp;
        short i;

        // Determine the number of cell record offsets in the DBCELL record.
        lpPos = (LPBYTE)tbGlobalLock(ghWorkSheet6);
        wCellRecs = (WORD)((((LPBIFF)lpPos)->datalen - sizeof(DWORD))/sizeof(WORD));

        lpPos += sizeof(BIFF) + sizeof(DWORD);

        wNextOffset = *(LPWORD)lpPos;
        *(LPWORD)lpPos = (WORD)lRow2Offset; // Set the offset from the 2nd Row to the first cell.
        lpPos += sizeof(WORD);

        for (i=0; i<wCellRecs-1; i++)
        {  
          wTemp = *(WORD *)lpPos;
          *(LPWORD)lpPos = wNextOffset;
          wNextOffset = wTemp;
          lpPos += sizeof(WORD);
        }
        
        tbGlobalUnlock(ghWorkSheet6);
      }

      //RAP05a >>
      //determine if the DBCELL record exceeds the record size maximum...
      lpBiff = (LPBIFF)tbGlobalLock(ghWorkSheet6);
      if (lpBiff->datalen > EXCEL_MAXBIFF8RECLEN)
      {
        long   ndatalen = lpBiff->datalen;
        int    nContinues = ndatalen / EXCEL_MAXBIFF8RECLEN;
        LPBYTE lpPos = NULL;
        int    nMoveSize = 0;

        tbGlobalUnlock(ghWorkSheet6);
        lWorkSheet6Len += nContinues * sizeof(BIFF);
        Alloc(&ghWorkSheet6, lWorkSheet6Len);
        lpBiff = (LPBIFF)tbGlobalLock(ghWorkSheet6);
        lpBiff->datalen = EXCEL_MAXBIFF8RECLEN;
        lpPos = (LPBYTE)lpBiff;
        nMoveSize = ndatalen-EXCEL_MAXBIFF8RECLEN;
        memmove(lpPos+sizeof(BIFF)+EXCEL_MAXBIFF8RECLEN+sizeof(BIFF), 
                lpPos+sizeof(BIFF)+EXCEL_MAXBIFF8RECLEN, 
                nMoveSize);
        lpBiff = (LPBIFF)(lpPos+sizeof(BIFF)+EXCEL_MAXBIFF8RECLEN);
        lpBiff->recnum = xlCONTINUE;
        lpBiff->datalen = (WORD)(ndatalen-EXCEL_MAXBIFF8RECLEN);
      }
      tbGlobalUnlock(ghWorkSheet6);
      //<< RAP05a

      //add buffer6(DBCELL) to the end of Cells(cell records)
      xl_CatWSToWS(&ghCells, (LPLONG)&lCellsLen, 
                   (LPTBGLOBALHANDLE)&ghWorkSheet6, (LPLONG)&lWorkSheet6Len);
#endif
      //add Cells(cell records & DBCELL) to the end of buffer5(ROW records)
      xl_CatWSToWS(&ghWorkSheet5, (LPLONG)&lWorkSheet5Len, 
                   (LPTBGLOBALHANDLE)&ghCells, (LPLONG)&lCellsLen);

      sRowCount = 0;
    }
#ifndef SS_V70
    //Make sure there are enough Font structures. The minimum number of font structures in a 
    //BIFF file is 4. They must be indexed: 0,1,2,3,5.  Index 4 is skipped for Excel backward
    //compatibility.
    ssm_FontCheck((LPTBGLOBALHANDLE)&ghFont, (LPLONG)&lFontLen);

    // Inspect the SST buffer to insure that it is not too large.
    // If it is, break it into CONTINUE records.
    ssm_FixUpSST((LPTBGLOBALHANDLE)&ghSST, (LPLONG)&lSSTLen);
#endif
#ifndef V7_TEST
    // Write the DEFCOLWIDTH offset to the INDEX.  This is NOT documented, but it is 
    // the way every BIFF8 file I have looked at handles this...
    lpPos = (LPBYTE)tbGlobalLock(ghWorkSheet2);
    lpIndex = (LPINDEX8)(lpPos + sizeof(BIFF));
    lpIndex->res2 = (DWORD)lBiffBufferLen + lFontLen + lFormatLen + lXFLen + lStyleLen + lGeneralLen +
                           lSSTLen + lEXTSSTLen + lEOFLen + lWorkSheetLen + lWorkSheet2Len + 
                           lDefColWidthPos;         
#endif

    if (dwFirstRow != (DWORD)-1)
    {
      // write the DIMENSION info
      LPyDIM8 lpDim;
#ifndef V7_TEST
      short   i;
#endif
      lpPos = (LPBYTE)tbGlobalLock(ghWorkSheet4);
      lpDim = (LPyDIM8)(lpPos + sizeof(BIFF));
      lpDim->dwRow1 = dwFirstRow-1;
      lpDim->dwRow2 = dwLastRow;
      if (wFirstCol != (WORD)-1)
      {
        lpDim->wCol1 = (WORD)(wFirstCol-1);
        lpDim->wCol2 = (WORD)wLastCol;
      }
      tbGlobalUnlock(ghWorkSheet4);

#ifndef V7_TEST
      // write the INDEX info
      LPINDEX8 lpIndex;

      lpPos = (LPBYTE)tbGlobalLock(ghWorkSheet2);
      lpIndex = (LPINDEX8)(lpPos + sizeof(BIFF));
      if (lpIndex->dwFirstRow == (DWORD)-1)
      {
        lpIndex->dwFirstRow = dwFirstRow-1;
        lpIndex->dwLastRowP1 = dwLastRow;
      }
      else
      {
        if (dwFirstRow-1 < lpIndex->dwFirstRow)
          lpIndex->dwFirstRow = dwFirstRow-1;
        if (dwLastRow > lpIndex->dwLastRowP1)
          lpIndex->dwLastRowP1 = dwLastRow;
      }
      lpPos = lpPos + INDEX_SIZE8 + sizeof(LPBIFF);
      for (i=0; i<lDBCellCount; i++)
      {
        LPBYTE lp = lpPos + i*sizeof(DWORD);
        *(LPLONG)lp = *(LPLONG)lp + lBiffBufferLen + lFontLen + lFormatLen + lXFLen + lStyleLen + lGeneralLen +
                                    lSSTLen + lEXTSSTLen + lEOFLen + lWorkSheetLen + lWorkSheet2Len + 
                                    lWorkSheet3Len + lWorkSheet4Len;
      }
//      tbGlobalUnlock(ghWorkSheet2);
#endif

    }
    else
    {
      //Make sure there are enough Font structures. The minimum number of font structures in a 
      //BIFF file is 4. They must be indexed: 0,1,2,3,5.  Index 4 is skipped for Excel backward
      //compatibility.
//      ssm_FontCheck((LPTBGLOBALHANDLE)&ghFont, (LPLONG)&lFontLen);
    }
   

#ifndef SS_V70
    LPxBUNDLESHT8 lpBS = NULL;
    lpPos = (LPBYTE)tbGlobalLock(ghGeneral);
    lpBS = (LPxBUNDLESHT8)(lpPos + sizeof(BIFF));
    lpBS->dwBOFPos = lBiffBufferLen + lFontLen + lFormatLen + lXFLen + lStyleLen + lGeneralLen + lSSTLen + lEXTSSTLen + lEOFLen;
    tbGlobalUnlock(ghGeneral);

    // write the SST string stream offset to the EXTSST
    if (lEXTSSTLen)
    {
//RAP07d      LPISSTINF lpisstinf;
      LPzSST lpSST = (LPzSST)((LPBYTE)tbGlobalLock(ghSST)+sizeof(BIFF));
      LPEXTSST lpEXTSST = NULL;        
      lpPos = (LPBYTE)tbGlobalLock(ghEXTSST);
      lpEXTSST = (LPEXTSST)(lpPos + sizeof(BIFF));

      lpEXTSST->wStringsPerBucket = (WORD)(lpSST->dwStrTotal/128 + (lpSST->dwStrTotal%128?1:0)); //RAP07a
//      lpEXTSST->wStringsPerBucket = (WORD)max(lpSST->dwStrTotal/129, 8); //RAP07a
//RAP07d      lpisstinf = (LPISSTINF)(lpPos + sizeof(BIFF) + sizeof(EXTSST));
//RAP07d      lpisstinf->ib += lBiffBufferLen + lFontLen + lFormatLen + lXFLen + lStyleLen + lGeneralLen;
      tbGlobalUnlock(ghSST);
      tbGlobalUnlock(ghEXTSST);
    }
#endif    
    //add buffer5 to the end of buffer4
    xl_CatWSToWS(&ghWorkSheet4, (LPLONG)&lWorkSheet4Len, 
                  (LPTBGLOBALHANDLE)&ghWorkSheet5, (LPLONG)&lWorkSheet5Len);
    //add buffer4 to the end of buffer3
    xl_CatWSToWS(&ghWorkSheet3, (LPLONG)&lWorkSheet3Len, 
                  (LPTBGLOBALHANDLE)&ghWorkSheet4, (LPLONG)&lWorkSheet4Len);
    //add buffer3 to the end of buffer2
    xl_CatWSToWS(&ghWorkSheet2, (LPLONG)&lWorkSheet2Len, 
                  (LPTBGLOBALHANDLE)&ghWorkSheet3, (LPLONG)&lWorkSheet3Len);
    //add buffer2 to the end of buffer
    xl_CatWSToWS(&ghWorkSheet, (LPLONG)&lWorkSheetLen, 
                  (LPTBGLOBALHANDLE)&ghWorkSheet2, (LPLONG)&lWorkSheet2Len);
/*
#ifdef SS_V40
//================================
{
    #define MSOSIZE_INIT 0xc8
    #define MSOSIZE_INC  0x80 
    #define SPIDBASE 0x0400

    #define msofbtDgContainer   0xf002
    #define msofbtDg            0xf008
    #define msofbtSpgrContainer 0xf003
    #define msofbtSpContainer   0xf004
    #define msofbtSpgr          0xf009
    #define msofbtSp            0xf00a
    #define msofbtOPT           0xf00b
    #define msofbtClientAnchor  0xf010
    #define msofbtClientData    0xf011
      
    TCHAR          buffer[10] = "hoo-hoo";
    short          sNoteID = 1;
    yNOTE          note;
    BYTE           msoData[8];
    TBGLOBALHANDLE gh = 0;
    LPBYTE         lpb = NULL;
    LPBYTE         lpbBeginning = NULL;
    short          sNoteCnt = 0;
    long           lLen = 0;
    long           sVersionInstance = 0;
    short          asMSOOffset[1000];
    short          sMSOOffsetIndex = 0;

    memset(asMSOOffset, 0, 1000*sizeof(short));
#pragma pack(1)
    struct {
      ULONG ver:4;
      ULONG inst:12;
      ULONG fbt:16;
      ULONG cbLength;
    } msofbh;        
    struct {
      ULONG csp;
      ULONG spidCur; //current shape id
    } fdg;
    struct {
      ULONG spid; //shape id
      ULONG grfPersistent;
    } fsp;
    struct {
      USHORT pid:14; //prop id
      USHORT fBid:1; //value is a blip ID - only valid if fComplex is False
      USHORT fComplex:1; // complex property, value is length
      ULONG op; //value
    } fopte;
    struct {
    	USHORT nFlag;
		  USHORT nCol1;
		  USHORT nX1;
		  USHORT nRow1;
		  USHORT nY1;
		  USHORT nCol2;
  		USHORT nX2;
		  USHORT nRow2;
		  USHORT nY2;
    } clientanchor;
#pragma pack()

    // initialize MSO structures
    memset(&msofbh, 0, sizeof(msofbh));
    memset(&fdg, 0, sizeof(fdg));
    memset(&fsp, 0, sizeof(fsp));
    memset(&fopte, 0, sizeof(fopte));
    memset(&clientanchor, 0, sizeof(clientanchor));

    // Get count of Notes
    sNoteCnt = 1;

    // Write out the MSO Drawing Container
    msofbh.cbLength = lLen = MSOSIZE_INIT + (MSOSIZE_INC*(sNoteCnt-1));
    Alloc(&gh, lLen);
    lpb = lpbBeginning = (LPBYTE)tbGlobalLock(gh);
    sVersionInstance = 0x000f;
    memcpy(&msofbh, &sVersionInstance, sizeof(short));
    msofbh.fbt = msofbtDgContainer;
    memcpy(lpb, &msofbh, sizeof(msofbh));
    lpb += sizeof(msofbh);

    // Write out the MSO Drawing Record
    sVersionInstance = 0x0010;
    memcpy(&msofbh, &sVersionInstance, sizeof(short));
    msofbh.fbt = msofbtDg;
    msofbh.cbLength = 0x08;
    memcpy(lpb, &msofbh, sizeof(msofbh));
    lpb += sizeof(msofbh);
    fdg.csp = sNoteCnt+1;
    fdg.spidCur = SPIDBASE + sNoteCnt;
    memcpy(lpb, &fdg, sizeof(fdg));
    lpb += sizeof(fdg);
    
    // Write out the MSO Group Shape Container
    sVersionInstance = 0x000f;
    memcpy(&msofbh, &sVersionInstance, sizeof(short));
    msofbh.fbt = msofbtSpgrContainer;
    msofbh.cbLength = lLen - 0x18; // from inspection. this takes us to after the msofbh header.
    memcpy(lpb, &msofbh, sizeof(msofbh));
    lpb += sizeof(msofbh);
    
    // Write out the MSO Shape Container 
    sVersionInstance = 0x000f;
    memcpy(&msofbh, &sVersionInstance, sizeof(short));
    msofbh.fbt = msofbtSpContainer;
    msofbh.cbLength = 0x28;  
    memcpy(lpb, &msofbh, sizeof(msofbh));
    lpb += sizeof(msofbh);

    // Write out the MSO Group Shape Record
    sVersionInstance = 0x0001;
    memcpy(&msofbh, &sVersionInstance, sizeof(short));
    msofbh.fbt = msofbtSpgr;
    msofbh.cbLength = 0x10;  
    memcpy(lpb, &msofbh, sizeof(msofbh));
    lpb += sizeof(msofbh);
    lpb += 0x10; // not sure why there is no rect bounds specified...

    // Write out the MSO Shape Record
    sVersionInstance = 0x0002;
    memcpy(&msofbh, &sVersionInstance, sizeof(short));
    msofbh.fbt = msofbtSp;
    msofbh.cbLength = 0x08;  
    memcpy(lpb, &msofbh, sizeof(msofbh));
    lpb += sizeof(msofbh);
    fsp.spid = SPIDBASE;
    fsp.grfPersistent = 0x05; //from inspection
    lpb += sizeof(fsp);

    // Write out the MSO Shape Container 
    sVersionInstance = 0x000f;
    memcpy(&msofbh, &sVersionInstance, sizeof(short));
    msofbh.fbt = msofbtSpContainer;
    msofbh.cbLength = 0x78;  
    memcpy(lpb, &msofbh, sizeof(msofbh));
    lpb += sizeof(msofbh);

    for (i=0; i<sNoteCnt; i++)
    {
      // Write out the MSO Shape Record
      sVersionInstance = 0x0ca2;
      memcpy(&msofbh, &sVersionInstance, sizeof(short));
      msofbh.fbt = msofbtSp;
      msofbh.cbLength = 0x08;  
      memcpy(lpb, &msofbh, sizeof(msofbh));
      lpb += sizeof(msofbh);
      fsp.spid = SPIDBASE + i;
      fsp.grfPersistent = 0x0a00; //from inspection
      lpb += sizeof(fsp);

      // Write out the MSO Shape Properties
      sVersionInstance = 0x0093;
      memcpy(&msofbh, &sVersionInstance, sizeof(short));
      msofbh.fbt = msofbtOPT;
      msofbh.cbLength = 0x36;  
      memcpy(lpb, &msofbh, sizeof(msofbh));
      lpb += sizeof(msofbh);
      fopte.pid = 0x80; // text id
      fopte.op = 0x40651660; // I HOPE THIS WORKS!!!
      memcpy(lpb, &fopte, sizeof(fopte));
      lpb += sizeof(fopte);
      fopte.pid = 0xbf & 0x3FFF; // size text to fit shape size
      fopte.op = 0x00080008;
      memcpy(lpb, &fopte, sizeof(fopte));
      lpb += sizeof(fopte);
      fopte.pid = 0x158; // ???
      fopte.op = 0;
      memcpy(lpb, &fopte, sizeof(fopte));
      lpb += sizeof(fopte);
      fopte.pid = 0x181; // foreground color
      fopte.op = 0x08000050;
      memcpy(lpb, &fopte, sizeof(fopte));
      lpb += sizeof(fopte);
      fopte.pid = 0x183; // back color
      fopte.op = 0x08000050;
      memcpy(lpb, &fopte, sizeof(fopte));
      lpb += sizeof(fopte);
      fopte.pid = 0x1bf; // hittest a shape as though filled
      fopte.op = 0x00110010;
      memcpy(lpb, &fopte, sizeof(fopte));
      lpb += sizeof(fopte);
      fopte.pid = 0x201; // foreshadow color
      fopte.op = 0;
      memcpy(lpb, &fopte, sizeof(fopte));
      lpb += sizeof(fopte);
      fopte.pid = 0x23f; // Excel5-style shadow
      fopte.op = 0x00030003;
      memcpy(lpb, &fopte, sizeof(fopte));
      lpb += sizeof(fopte);
      fopte.pid = 0x3bf; // print this shape
      fopte.op = 0x000a0002;
      memcpy(lpb, &fopte, sizeof(fopte));
      lpb += sizeof(fopte);

      // Write out the MSO Client Anchor 
      sVersionInstance = 0;
      memcpy(&msofbh, &sVersionInstance, sizeof(short));
      msofbh.fbt = msofbtClientAnchor;
      msofbh.cbLength = 0x12;  
      memcpy(lpb, &msofbh, sizeof(msofbh));
      lpb += sizeof(msofbh);

      clientanchor.nFlag = 0x03;
      clientanchor.nCol1 = 1; //COL1 + 1 offset
      clientanchor.nX1 = 0x00c0; // if nCol1 != nCol2; otherwise 0;
      clientanchor.nRow1 = 0; // cellnote row-2
      clientanchor.nY1 = 0x0069;
      clientanchor.nCol2 = 2;
      clientanchor.nX2 = 0x0326; // if nCol1 != nCol2; otherwise 1023;
      clientanchor.nRow2 = 5;
      clientanchor.nY2 = 0x00c4;
// from StarOffice
//	if ( nRow1 == nRow2 )
//	{
//		nY1 = 0;
//		nY2 = 255;
//	}
//	else if ( nRow1 == 0 && nRow1 == rPos.Row() )
//	{
//		nY1 = 0x001e;
//		nY2 = 0x0078;
//  }
//	else
//	{
//		nY1 = 0x0069;
//		nY2 = 0x00c4;
//	}

      memcpy(lpb, &clientanchor, sizeof(clientanchor));
      lpb += sizeof(clientanchor);


      // Write out the MSO Client Data 
      sVersionInstance = 0;
      memcpy(&msofbh, &sVersionInstance, sizeof(short));
      msofbh.fbt = msofbtClientData;
      msofbh.cbLength = 0;  
      memcpy(lpb, &msofbh, sizeof(msofbh));
      lpb += sizeof(msofbh);
      asMSOOffset[i] = (short)(lpb - lpbBeginning);
    }  

    memset(&note, 0, sizeof(yNOTE));

    biff.recnum = xlMSODRAWING;
    biff.datalen = (short)(lpb-lpbBeginning);
    ssm_WriteBuffer((TBGLOBALHANDLE FAR *)&ghWorkSheet, &biff, lp, &lWorkSheetLen, lpbBeginning);

    tbGlobalUnlock(gh);

    biff.recnum = xlOBJ;
    biff.datalen = 0;
    ssm_WriteBuffer((TBGLOBALHANDLE FAR *)&ghWorkSheet, &biff, lp, &lWorkSheetLen, &sNoteID);

    memset(msoData, 0, 8);
    msoData[2] = 0x0d;
    msoData[3] = 0xf0;  
    biff.datalen = 8;
    biff.recnum = xlMSODRAWING;
    biff.datalen = 8;
    ssm_WriteBuffer((TBGLOBALHANDLE FAR *)&ghWorkSheet, &biff, lp, &lWorkSheetLen, msoData);

    biff.recnum = xlTXO;
    biff.datalen = 18;
    ssm_WriteBuffer((TBGLOBALHANDLE FAR *)&ghWorkSheet, &biff, lp, &lWorkSheetLen, buffer);

    biff.recnum = xlNOTE;
    biff.datalen = 0;
    note.col = 1;
    note.rw = 2;
    note.idObj = 1;
    ssm_WriteBuffer((TBGLOBALHANDLE FAR *)&ghWorkSheet, &biff, lp, &lWorkSheetLen, &note);
}
//================================
#endif
*/
    biff.recnum = xlWINDOW2;
    biff.datalen = 18;

    BOOL fActiveSheet;
#ifdef SS_V70
    fActiveSheet = FALSE;
    if (fSaveSheet)
      fActiveSheet = TRUE;
    else if (i == SS_GetActiveSheet(lpBook))
    {
      if (!SS_GetSheetVisible(lpBook, i))
      {
         
      }
      else
        fActiveSheet = TRUE;
    }

    ssm_WriteBuffer((TBGLOBALHANDLE FAR *)&ghWorkSheet, &biff, lp, &lWorkSheetLen, &fActiveSheet);

    biff.recnum = xlPANE;
    biff.datalen = 10;
    ssm_WriteBuffer((TBGLOBALHANDLE FAR *)&ghWorkSheet, &biff, lp, &lWorkSheetLen, NULL);
#else
    fActiveSheet = TRUE;
    ssm_WriteBuffer((TBGLOBALHANDLE FAR *)&ghWorkSheet, &biff, lp, &lWorkSheetLen, &fActiveSheet);
#endif
    biff.recnum = xlSELECTION;
    biff.datalen = 9;
    ssm_WriteBuffer((TBGLOBALHANDLE FAR *)&ghWorkSheet, &biff, lp, &lWorkSheetLen, NULL);
  
#ifdef SS_V40
    if (lpSS->hCellSpan)
    {
      biff.recnum = xlMERGECELLS;
      biff.datalen = 0;  
      ssm_WriteBuffer((TBGLOBALHANDLE FAR *)&ghWorkSheet, &biff, lp, &lWorkSheetLen, NULL);
    
/*
      biff.recnum = xlMERGE_DONTKNOW;
      biff.datalen = 6;
      ssm_WriteBuffer((TBGLOBALHANDLE FAR *)&ghWorkSheet, &biff, lp, &lWorkSheetLen, NULL);
*/
    }
#endif

    // write end block for row/cell info -- resolve any offsets.  
    biff.recnum = xlEOF_BIFF;
    biff.datalen = 0;
    ssm_WriteBuffer((TBGLOBALHANDLE FAR *)&ghWorkSheet, &biff, lp, &lWorkSheetLen, NULL);

#ifdef SS_V70
    {
    LPTBGLOBALHANDLE lpghSheet = NULL;
    LPBYTE           lpb = NULL;

    lpghSheet = (LPTBGLOBALHANDLE)tbGlobalLock(ghSheets);
    lpghSheet[lpBook->nSheet] = tbGlobalAlloc(GHND, sizeof(long) + sizeof(TBGLOBALHANDLE));
    lpb = (LPBYTE)tbGlobalLock(lpghSheet[lpBook->nSheet]);
    
    memcpy(lpb, &lWorkSheetLen, sizeof(long));
    memcpy(lpb + sizeof(long), &ghWorkSheet, sizeof(TBGLOBALHANDLE));

    tbGlobalUnlock(lpghSheet[lpBook->nSheet]);
    tbGlobalUnlock(ghSheets);

    ghWorkSheet = (TBGLOBALHANDLE)0;
    lWorkSheetLen = 0;
    }
#endif

    if (ghIsColDefaultDrawing)
    {
      tbGlobalFree(ghIsColDefaultDrawing);
      ghIsColDefaultDrawing = 0;
    }
    if (ghIsColFormula)
    {
      tbGlobalFree(ghIsColFormula);
      ghIsColFormula = 0;
    }
  }

#ifdef SS_V70
  //Make sure there are enough Font structures. The minimum number of font structures in a 
  //BIFF file is 4. They must be indexed: 0,1,2,3,5.  Index 4 is skipped for Excel backward
  //compatibility.
  ssm_FontCheck((LPTBGLOBALHANDLE)&ghFont, (LPLONG)&lFontLen);

  // Inspect the SST buffer to insure that it is not too large.
  // If it is, break it into CONTINUE records.
  ssm_FixUpSST((LPTBGLOBALHANDLE)&ghSST, (LPLONG)&lSSTLen);

  // write the SST string stream offset to the EXTSST
  if (lEXTSSTLen)
  {
    LPzSST   lpSST = (LPzSST)((LPBYTE)tbGlobalLock(ghSST)+sizeof(BIFF));
    LPEXTSST lpEXTSST = NULL;        
    LPBYTE   lpPos = (LPBYTE)tbGlobalLock(ghEXTSST);

    lpEXTSST = (LPEXTSST)(lpPos + sizeof(BIFF));
    lpEXTSST->wStringsPerBucket = (WORD)(lpSST->dwStrTotal/128 + (lpSST->dwStrTotal%128?1:0)); //RAP07a
    tbGlobalUnlock(ghSST);
    tbGlobalUnlock(ghEXTSST);
  }
 
  {  
    TBGLOBALHANDLE   ghBundleSheets = tbGlobalAlloc(GHND, lpBook->nSheetCnt*sizeof(TBGLOBALHANDLE));
    LPTBGLOBALHANDLE lpghBundleSheets = (LPTBGLOBALHANDLE)tbGlobalLock(ghBundleSheets);
    TBGLOBALHANDLE   ghBundleSheetsLen = tbGlobalAlloc(GHND, lpBook->nSheetCnt*sizeof(long));
    LPLONG           lplBundleSheetsLen = (LPLONG)tbGlobalLock(ghBundleSheetsLen);
    BOOL             fASheetIsVisible = FALSE;

    for (i=startSheet; i<endSheet; i++)
    {
      if (SS_GetSheetVisible(lpBook, i))
      {
        fASheetIsVisible = TRUE;
        break;
      }
    }

    for (i=startSheet; i<endSheet; i++)
    {
      TBGLOBALHANDLE   ghSheetName = SS_GetSheetName(lpBook, i);
      LPTSTR           lptstrSheetName = (LPTSTR)tbGlobalLock(ghSheetName);
      long             lBundleSheetLen = 0;
      TCHAR            replacementChar = '_';
      int              j;

      if (szSheetName != NULL)
      {
        //single sheet
        lptstrSheetName = (LPTSTR)szSheetName;
      }
      else
      {
        SSGetSheetDisplayName(lpBook->hWnd, i, &ghSheetName);
        lptstrSheetName = (LPTSTR)tbGlobalLock(ghSheetName);
      }

      lpghBundleSheets[i] = (TBGLOBALHANDLE)0;

      biff.recnum = xlBUNDLESHEET;
      biff.datalen = 8;

      TBGLOBALHANDLE gh = (TBGLOBALHANDLE)0;
      LPBYTE         lpb = NULL;
      BYTE           bVisible = (BYTE)(SS_GetSheetVisible(lpBook, i)?1:0);
      int            nLen = lstrlen(lptstrSheetName);

      if (i==startSheet && !fASheetIsVisible)
        bVisible = 1;

      Alloc(&gh, sizeof(BYTE) + (nLen+1)*sizeof(WCHAR)); // to make sure it is large enough...
      lpb = (LPBYTE)tbGlobalLock(gh);
      memcpy(lpb, &bVisible, sizeof(BYTE));
      
      for (j=0; j<nLen; j++)
      {
#ifndef _UNICODE
        BYTE* pByte = (BYTE*)(&lptstrSheetName[j]);
        if(IsDBCSLeadByte(*pByte) || _ismbbkana(*pByte))
  	    {
		      j++;
          continue;
	      }
        else if(*pByte > 127)
        {
          continue;
        }
  	    else if(*pByte ++ ==0x0d && *pByte==0x0a)
	      {
          j++;
          continue;
	      }
#endif

#if !SPREAD_JPN //19762 - only perform this character replacement in non-Japanese characterset
        if ( lptstrSheetName[j] == '?'
          || lptstrSheetName[j] == '\\'
          || lptstrSheetName[j] == '/'
          || lptstrSheetName[j] == '*'
          || lptstrSheetName[j] == '['
          || lptstrSheetName[j] == ']' )
        lptstrSheetName[j] = replacementChar;
#endif //19762
      }   
#if UNICODE      
      memcpy(lpb+sizeof(BYTE), lptstrSheetName, (lstrlen(lptstrSheetName)*sizeof(TCHAR)));
#else
      memcpy(lpb+sizeof(BYTE), lptstrSheetName, nLen);
#endif

//      ssm_WriteBuffer((TBGLOBALHANDLE FAR *)&lpghBundleSheets[i], &biff, lp, &lBundleSheetLen, (LPVOID)lptstrSheetName);
      ssm_WriteBuffer((TBGLOBALHANDLE FAR *)&lpghBundleSheets[i], &biff, lp, &lBundleSheetLen, (LPVOID)lpb);

      lplBundleSheetsLen[i] = lBundleSheetLen;

      tbGlobalUnlock(gh);
      tbGlobalFree(gh);
      
      tbGlobalUnlock(ghSheetName);
      if (szSheetName == NULL)
        tbGlobalFree(ghSheetName);
    }


    {
      // Write out the NAME records
      LPBYTE lpData = (LPBYTE)tbGlobalLock(ghCustomNames);
      long   lPos = 0;
      biff.recnum = xlNAME;
      while (lPos < lCustomNamesLen)
      {
        TBGLOBALHANDLE gh = 0;
        TBGLOBALHANDLE ghNameDef = 0;
        long           lNameDefLen = 0;
        LPBYTE         lpb = NULL;
        LPTSTR         lpszName = (LPTSTR)(lpData + lPos);
        CALC_HANDLE    hName = NameLookup(&lpBook->CalcInfo.Names, lpszName);
        LPBYTE         lpName = NULL;
        TBGLOBALHANDLE ghBiffStr = 0;
        LPBYTE         lpBiffStr = NULL;
        short          len = 0;
        short          sBiffStrLen = 0;
        long           lBufferPos = 0;

        biff.datalen = 14;

        if (hName)
        {
          CALC_HANDLE hExpr = NameGetExpr(hName);
          if (hExpr)
          {
            LPSPREADSHEET lpSS;
            lpSS = SS_BookLockSheetIndex(lpBook, lpBook->nSheet);
            long lCustomNamesLenTemp = lCustomNamesLen;
            ssm_Save4Expr(lpSS, -1, -1, hExpr, &ghNameDef, &lNameDefLen, &sxtiCount, &ghXTI, &ghCustomNames, &lCustomNamesLen);
            if (lCustomNamesLen != lCustomNamesLenTemp)
            {
              tbGlobalUnlock(ghCustomNames);
              lpData = (LPBYTE)tbGlobalLock(ghCustomNames);
            }
          }
        }

#ifdef UNICODE
        sBiffStrLen = (short)ssm_BuildBIFFStr(lpszName, &ghBiffStr, TRUE);
#else
        sBiffStrLen = (short)ssm_BuildBIFFStr(lpszName, &ghBiffStr, FALSE);
#endif
        sBiffStrLen -= sizeof(WORD);
        lpBiffStr = (LPBYTE)tbGlobalLock(ghBiffStr);
        len = (short)(sizeof(WORD) + sBiffStrLen + sizeof(WORD) + lNameDefLen);
        Alloc(&gh, len);
        lpb = (LPBYTE)tbGlobalLock(gh);
#if UNICODE
        short sLen = sBiffStrLen/2 + 1;  // the +1 is for the biffstr grbit that is added in ssm_BuildBIFFStr.
        memcpy(lpb, &sLen, sizeof(WORD));
#else
        memcpy(lpb, &sBiffStrLen, sizeof(WORD));
#endif
        lBufferPos = sizeof(WORD);
        memcpy(lpb + lBufferPos, lpBiffStr + sizeof(WORD), sBiffStrLen);
        lBufferPos += sBiffStrLen;
        lpName = (LPBYTE)tbGlobalLock(ghNameDef);
        memcpy(lpb + lBufferPos, &lNameDefLen, sizeof(WORD));
        lBufferPos += sizeof(WORD);
        memcpy(lpb + lBufferPos, lpName, lNameDefLen);
        tbGlobalUnlock(ghNameDef);

        ssm_WriteBuffer((TBGLOBALHANDLE FAR *)&ghNames, &biff, lp, &lNamesLen, lpb);
        lPos += (lstrlen((LPTSTR)(lpData+lPos))+1)*sizeof(TCHAR);
        tbGlobalFree(gh);
      }
    }

    // Write out the SUPBOOK & EXTERNSHEET records
    if (sxtiCount > 0)
    {
      LPXTI  lpxti = (LPXTI)tbGlobalLock(ghXTI);
      long   lData = 0x04010000 | lpBook->nSheetCnt;
      TBGLOBALHANDLE ghData = (TBGLOBALHANDLE)0;
      LPBYTE lpData = NULL;

      biff.recnum = xlSUPBOOK;
      biff.datalen = 4;
      ssm_WriteBuffer((TBGLOBALHANDLE FAR *)&ghCrossSheet, &biff, lp, &lCrossSheetLen, &lData);

      biff.recnum = xlEXTERNSHEET;
      biff.datalen = (USHORT)(2 + sxtiCount * sizeof(XTI));
      Alloc(&ghData, biff.datalen);
      lpData = (LPBYTE)tbGlobalLock(ghData);
      memcpy(lpData, &sxtiCount, sizeof(USHORT));
      memcpy(lpData+sizeof(USHORT), lpxti, sxtiCount * sizeof(XTI));

      ssm_WriteBuffer((TBGLOBALHANDLE FAR *)&ghCrossSheet, &biff, lp, &lCrossSheetLen, lpData);
      
      tbGlobalUnlock(ghXTI);
      tbGlobalFree(ghXTI);
      tbGlobalUnlock(ghData);
      tbGlobalFree(ghData);
      ghXTI = (TBGLOBALHANDLE)0;
    }

    //add sheets to the end of EOF
    LPTBGLOBALHANDLE lpghSheets = (LPTBGLOBALHANDLE)tbGlobalLock(ghSheets);

    for (i=startSheet; i<endSheet; i++)
    {
      LPBYTE           lpb = NULL;
      long             lSheetLen = 0;
      TBGLOBALHANDLE   ghSheet = 0;
      LPxBUNDLESHT8    lpBS = NULL;
      long             lPos = lBiffBufferLen + lFontLen + lFormatLen + lXFLen + lStyleLen + lGeneralLen + lSSTLen + lEXTSSTLen + lCrossSheetLen + lNamesLen + lEOFLen;
      int              j = 0;

      if (lpghSheets[i] != 0)
      {
        lpb = (LPBYTE)tbGlobalLock(lpghSheets[i]);
        lSheetLen = (lpb? *(LPLONG)lpb: 0);
        ghSheet = *(LPTBGLOBALHANDLE)(lpb+sizeof(long));
        lpBS = NULL;
      }

      for (j=i; j<lpBook->nSheetCnt; j++)
      {
        lPos += lplBundleSheetsLen[j];
      }

      lpb = (LPBYTE)tbGlobalLock(lpghBundleSheets[i]);
      lpBS = (LPxBUNDLESHT8)(lpb + sizeof(BIFF));
      lpBS->dwBOFPos = lPos;
      lPos += lSheetLen;
      tbGlobalUnlock(lpghBundleSheets[i]);

      xl_CatWSToWS(&ghGeneral, (LPLONG)&lGeneralLen, (LPTBGLOBALHANDLE)&(lpghBundleSheets[i]), (LPLONG)&(lplBundleSheetsLen[i]));
      xl_CatWSToWS(&ghEOF, (LPLONG)&lEOFLen, (LPTBGLOBALHANDLE)&ghSheet, (LPLONG)&lSheetLen);
      
      tbGlobalUnlock(lpghSheets[i]);
      if (lpghSheets[i] != 0)
      {
        tbGlobalFree(lpghSheets[i]);
        lpghSheets[i] = (TBGLOBALHANDLE)0;
      }
    }
    tbGlobalUnlock(ghSheets);
    tbGlobalFree(ghSheets);
    tbGlobalUnlock(ghBundleSheets);
    tbGlobalFree(ghBundleSheets);
    tbGlobalFree(ghBundleSheetsLen);

    ghSheets = (TBGLOBALHANDLE)0;
    ghBundleSheets = (TBGLOBALHANDLE)0;
  }

  lpBook->nSheet = (short)nSheetIndex;
#else
  //add WorkSheet to the end of EOF
  xl_CatWSToWS(&ghEOF, (LPLONG)&lEOFLen, 
                (LPTBGLOBALHANDLE)&ghWorkSheet, (LPLONG)&lWorkSheetLen);
#endif
  //add EOF to the end of EXTSST
  xl_CatWSToWS(&ghEXTSST, (LPLONG)&lEXTSSTLen, 
                (LPTBGLOBALHANDLE)&ghEOF, (LPLONG)&lEOFLen);
  //add EXTSST to the end of SST
  xl_CatWSToWS(&ghSST, (LPLONG)&lSSTLen, 
                (LPTBGLOBALHANDLE)&ghEXTSST, (LPLONG)&lEXTSSTLen);
  if (ghStrings != 0)
  {
    tbGlobalFree(ghStrings);
    ghStrings = (TBGLOBALHANDLE)0;
  }
#ifdef SS_V70
  //add Names to the end of CrossSheet
  xl_CatWSToWS(&ghCrossSheet, (LPLONG)&lCrossSheetLen, 
                (LPTBGLOBALHANDLE)&ghNames, (LPLONG)&lNamesLen);
  //add CrossSheet to the end of General
  xl_CatWSToWS(&ghGeneral, (LPLONG)&lGeneralLen, 
                (LPTBGLOBALHANDLE)&ghCrossSheet, (LPLONG)&lCrossSheetLen);
#endif
  //add SST to the end of General
  xl_CatWSToWS(&ghGeneral, (LPLONG)&lGeneralLen, 
                (LPTBGLOBALHANDLE)&ghSST, (LPLONG)&lSSTLen);

  xl_CatWSToWS(&ghStyle, (LPLONG)&lStyleLen, 
                (LPTBGLOBALHANDLE)&ghGeneral, (LPLONG)&lGeneralLen);

  xl_CatWSToWS(&ghXF, (LPLONG)&lXFLen, 
                (LPTBGLOBALHANDLE)&ghStyle, (LPLONG)&lStyleLen);

  //add XF to the end of Format
  xl_CatWSToWS(&ghFormat, (LPLONG)&lFormatLen, 
                (LPTBGLOBALHANDLE)&ghXF, (LPLONG)&lXFLen);
  //add Format to the end of Font
  xl_CatWSToWS(&ghFont, (LPLONG)&lFontLen,
                (LPTBGLOBALHANDLE)&ghFormat, (LPLONG)&lFormatLen);
  //add Font to the end of BiffBuffer
  xl_CatWSToWS(&ghBiffBuffer, (LPLONG)&lBiffBufferLen,
                (LPTBGLOBALHANDLE)&ghFont, (LPLONG)&lFontLen);

  if (fToStream && pstm)
    ssm_DumpBuffer(pstm, ghBiffBuffer, (LPLONG)&lBiffBufferLen);
  else
  {
    *lpghBuffer = ghBiffBuffer;
    *lplBufferLen = lBiffBufferLen;
  }
  return S_OK;
}

/***********************************************************************
* Name:   ssm_Write - Read chunks of the specified stream and pass
*                     the data to the parser. Any portion of the chunk
*                     that is not used by the parser is recycled and
*
* Usage:  long ssm_Write(IStream *pstm, LPBIFF lpBiff, LPVOID lp,
*                        LPLONG lplPos, LPVOID lpExtra)
*           pstm - pointer to the docfile stream to write.
*           lpBiff - pointer to the BIFF structure to indicate what type of
*                    data is being written. This structure is also written to
*                    the file.
*           lp - pointer to the Spread structure -- cast as LPVOID to pass to
*                bif_ functions.                  
*           lplPos - pointer to the current size of the stream.
*           lpExtra - pointer to additional information needed to write out this
*                     BIFF structure & info.
*
* Return: long - Success or Failure
***********************************************************************/
long ssm_Write(IStream *pstm, LPBIFF lpBiff, LPVOID lp, LPLONG lplPos, LPVOID lpExtra)
{
  TBGLOBALHANDLE  gh = (TBGLOBALHANDLE)0;
  long          lSize = *lplPos;
  unsigned long lWriteSize;
  LPVOID        lpData;
  HRESULT       hr;

  bif_Build(lpBiff, lp, (TBGLOBALHANDLE FAR *)&gh, &lSize, lpExtra);
  lpData = (LPVOID)tbGlobalLock(gh);
  hr = pstm->Write(lpData, lSize, &lWriteSize);
  if (S_OK != hr)
  {
		DisplayError(hr,_T("Write"));
    tbGlobalUnlock(gh);
    tbGlobalFree(gh);
    gh=0;
    return hr;
  }
  *lplPos += lWriteSize;

  tbGlobalUnlock(gh);
  tbGlobalFree(gh);
  gh=0;

  return S_OK;
}

/***********************************************************************
* Name:   ssm_BuildFont - Retrieve the font info from Spread, build the
*                         Excel BIFF FONT structure, add it to the Font
*                         buffer and return the font index. Index = 4 is
*                         not used in Excel BIFF format. Therefore, the
*                         font indices are (0,1,2,3,5,6,...)
*
* Usage:  WORD ssm_BuildFont(LPSPREADSHEET lpSS, TBGLOBALHANDLE FAR *lpghFont,
*                            LPLONG lplFontLen, SS_FONTID FontId, WORD icv)
*           lpSS - pointer to the Spread structure.
*           lpghFont - pointer to the Font buffer.
*           lplFontLen - pointer to the length of the Font buffer.          
*           FontId - Spread's Id for the font to be added.
*           icv - Excel's font color index.
*
* Return: WORD - The font index or -1 if there is an error.
***********************************************************************/
WORD ssm_BuildFont(LPSS_BOOK lpBook, TBGLOBALHANDLE FAR *lpghFont, LPLONG lplFontLen, SS_FONTID FontId, WORD icv)
{
  BIFF    biff;
  SS_FONT SSFont;
  LPBYTE  lp = NULL;
  long    lPos = 0;
  WORD    wCount = 0;
  xFONT   Font;
  LPWSTR  lpFontName;
  BOOL    fFound = FALSE;
  HDC     hDC = GetDC(lpBook->hWnd);

  memset((LPBIFF)&biff, 0, sizeof(BIFF));
  memset((LPxFONT)&Font, 0, sizeof(xFONT));

  biff.recnum = xlFONT;
  biff.datalen = 16;

  if (NULL == SS_GetFont(lpBook, (LPSS_FONT)&SSFont, FontId))
  {
    ReleaseDC(lpBook->hWnd, hDC);
    return (WORD)0;
  }

  Font.wHeight = (WORD)(MulDiv(abs(SSFont.LogFont.lfHeight), 72, GetDeviceCaps(hDC, LOGPIXELSY)));
  Font.wHeight *= 20;
  Font.fItalic = SSFont.LogFont.lfItalic;
  Font.fStrikeout = SSFont.LogFont.lfStrikeOut;
  Font.wBoldStyle = (WORD)SSFont.LogFont.lfWeight;
  Font.bUls = SSFont.LogFont.lfUnderline;
  Font.bFamily = SSFont.LogFont.lfPitchAndFamily;
  Font.bCharSet = SSFont.LogFont.lfCharSet;

// The font must be a WCHAR string!
#ifndef _UNICODE
  Font.bFontNameLen = (byte)MultiByteToWideChar(CP_ACP, MB_PRECOMPOSED, SSFont.LogFont.lfFaceName, -1,
                                                NULL, 0);
  Font.ghFontName = tbGlobalAlloc(GHND, (Font.bFontNameLen+1)*sizeof(WCHAR));
  lpFontName = (LPWSTR)tbGlobalLock(Font.ghFontName);
  Font.bFontNameLen = (BYTE)MultiByteToWideChar(CP_ACP, MB_PRECOMPOSED, SSFont.LogFont.lfFaceName, -1,
                                                lpFontName, (int)Font.bFontNameLen+1);
  Font.bFontNameLen--;
#else
  Font.bFontNameLen = (BYTE)(lstrlen(SSFont.LogFont.lfFaceName));
  Font.ghFontName = tbGlobalAlloc(GHND, (Font.bFontNameLen+1)*sizeof(WCHAR));
  lpFontName = (LPWSTR)tbGlobalLock(Font.ghFontName);
  memcpy(lpFontName, SSFont.LogFont.lfFaceName, Font.bFontNameLen*sizeof(WCHAR));  
#endif

  Font.bStrType = 1; // Byte
  if (icv == 0)
//    Font.wicv = 0x0008; //font color index
    Font.wicv = 0x7fff; //font color index
  else if (icv == 0x41)
    Font.wicv = 0x7fff; //font color index
  else
    Font.wicv = icv; //font color index

  tbGlobalUnlock(Font.ghFontName);

  ReleaseDC(lpBook->hWnd, hDC);

  if (*lpghFont)
  {
    LPBIFF  lpBiff = NULL;    
    LPxFONT lpFont = NULL;

    lp = (LPBYTE)tbGlobalLock(*lpghFont);
    while (lPos < *lplFontLen)
    {
      lpBiff = (LPBIFF)lp;

      lPos += sizeof(BIFF);
      lp = lp + sizeof(BIFF);
      lpFont = (LPxFONT)lp;
      if (!memcmp(lp, &Font, sizeof(xFONT)-sizeof(TBGLOBALHANDLE)-sizeof(BOOL)) &&
          ((LPxFONT)lp)->bFontNameLen == Font.bFontNameLen)
      {
        LPTSTR lp1 = (LPTSTR)(lp + sizeof(xFONT)-sizeof(TBGLOBALHANDLE)-sizeof(BOOL));
        LPTSTR lp2 = (LPTSTR)tbGlobalLock(Font.ghFontName);
        if (!memcmp(lp1, lp2, Font.bFontNameLen*sizeof(WCHAR))) 
        {
          fFound = TRUE;
          break;
        }
        tbGlobalUnlock(Font.ghFontName);
      }
      lPos += lpBiff->datalen;
      lp = lp + lpBiff->datalen;
      wCount++;
    }
    tbGlobalUnlock(*lpghFont);
  }
  
  if (!fFound)
  {
    ssm_WriteBuffer(lpghFont, &biff, (LPVOID)lpBook, lplFontLen, (LPVOID)&Font);
  }

  tbGlobalFree(Font.ghFontName);
  Font.ghFontName=0;

  if (wCount >= 4)
    wCount++;

  return wCount;
}

/***********************************************************************
* Name:   ssm_FontCheck - Count the number of fonts in the font buffer. 
*                         If there are not 4 fonts in the buffer (Excel
*                         BIFF's minimum allowable font count), then add
*                         duplicates of the current last font to bring the
*                         font count up to 4. Index=4 is skipped due to 
*                         Excel BIFF backward compatibility issues.
*
* Usage:  WORD ssm_FontCheck(TBGLOBALHANDLE FAR *lpghFont, LPLONG lplFontLen)
*           lpSS - pointer to the Spread structure.
*           lpghFont - pointer to the Font buffer.
*           lplFontLen - pointer to the length of the Font buffer.          
*           FontId - Spread's Id for the font to be added.
*           icv - Excel's font color index.
*
* Return: WORD - Success or Failure
***********************************************************************/
WORD ssm_FontCheck(TBGLOBALHANDLE FAR *lpghFont, LPLONG lplFontLen)
{
  if (*lpghFont)
  {
    LPBIFF lpBiff = NULL;    
    LPBYTE lpFonts = (LPBYTE)tbGlobalLock(*lpghFont);
    LPBYTE lp = lpFonts;
    LPBYTE lpPrev = lp;
    long   lPos = 0;
    long   lPrevPos = lPos;
    WORD   wCount = 0;
    TBGLOBALHANDLE ghFont = 0;

    while (lPos < *lplFontLen)
    {
      lpBiff = (LPBIFF)lp;

      lPrevPos = lPos;
      lpPrev = lp;

      lPos += sizeof(BIFF) + lpBiff->datalen;
      lp = lp + sizeof(BIFF) + lpBiff->datalen;
      wCount++;
    }
    if (wCount < 5)
    {
      short sIncreaseSize;
      short i;
      lpBiff = (LPBIFF)lpPrev;
      sIncreaseSize = (short)((5-wCount)*(sizeof(BIFF) + lpBiff->datalen));
      ghFont = tbGlobalAlloc(GHND, *lplFontLen+sIncreaseSize);      
      lp = (LPBYTE)tbGlobalLock(ghFont);
      memcpy(lp, lpFonts, *lplFontLen);
      lp = lp + *lplFontLen;
      *lplFontLen += sIncreaseSize;
      for (i=0; i<5-wCount; i++)
      {
        memcpy(lp, lpPrev, sizeof(BIFF) + lpBiff->datalen);
        lp = lp + sizeof(BIFF) + lpBiff->datalen;
      }
      tbGlobalUnlock(*lpghFont);
      tbGlobalFree(*lpghFont);
      *lpghFont = ghFont;
    }
    else
      tbGlobalUnlock(*lpghFont);
  }
  return TRUE;
}

/***********************************************************************
* Name:   ssm_WriteBuffer - Write the BIFF structure and accompanying data
*                           to the specified buffer. Allow for additional
*                           data to be added to the entry if needed.
*
* Usage:  long ssm_WriteBuffer(TBGLOBALHANDLE FAR *lpgh, LPBIFF lpBiff,
*                              LPVOID lp, LPLONG lplPos, LPVOID lpExtra)
*           lpgh - pointer to the buffer.
*           lpBiff - pointer to the BIFF structure.
*           lp - pointer to the Spread structure -- passed to bif_Build().
*           lplPos - pointer to the buffer length.
*           lpExtra - pointer to additional data needed for this BIFF structure.
*
* Return: long - Success or Failure
***********************************************************************/
long ssm_WriteBuffer(TBGLOBALHANDLE FAR *lpgh, LPBIFF lpBiff, LPVOID lp, LPLONG lplPos, LPVOID lpExtra)
{
  TBGLOBALHANDLE  gh = (TBGLOBALHANDLE)0;
  long          lSize = *lplPos;
  LPVOID        lpData1;
  LPVOID        lpData2;

  bif_Build(lpBiff, lp, (TBGLOBALHANDLE FAR *)&gh, &lSize, lpExtra);

  if (gh == (TBGLOBALHANDLE)0)
    return S_OK;

  if (!*lpgh)
  {
    *lpgh = tbGlobalAlloc(GHND, *lplPos + lSize);
    lpData2 = (LPVOID)tbGlobalLock(*lpgh);
  }
  else
  {
    *lpgh = tbGlobalReAlloc(*lpgh, *lplPos + lSize, GHND);
    lpData2 = (LPVOID)tbGlobalLock(*lpgh);
    lpData2 = (LPBYTE)lpData2 + *lplPos;
  }
  *lplPos += lSize;

  lpData1 = (LPVOID)tbGlobalLock(gh);    
  memcpy(lpData2, lpData1, lSize); 

  tbGlobalUnlock(*lpgh);  
  tbGlobalUnlock(gh);
  tbGlobalFree(gh);
  gh=0;

  return S_OK;
}

/***********************************************************************
* Name:   ssm_BuildXF - Build an Excel BIFF Extended Format(XF) structure
*                       & data. Determine the type of XF to build. If it
*                       is a "Cell XF", determine the Spread heirarchy 
*                       level (spreadsheet, column, row, cell) and retrieve
*                       the necessary information from the Spread control.
*
* Usage:  long ssm_BuildXF(SS_COORD Col, SS_COORD Row, short sXFType,
*                          LPSHORT lpsIndex, LPSHORT lpsCount,
*                          TBGLOBALHANDLE FAR *lpghXF, LPBIFF lpBiff, LPVOID lp,
*                          LPLONG lplXFLen, LPVOID lpXF,
*                          TBGLOBALHANDLE FAR *lpghFont, LPLONG lplFontLen)
*           Col - column
*           Row - row
*           sXFType - type of XF structure to build (Style vs Cell).
*           lpsIndex - index of this XF structure.
*           lpsCount - count of XF's in the buffer.
*           lpghXF - XF buffer.
*           lpBiff - pointer to the BIFF structure for the XF.
*           lp - pointer to the Spread structure.
*           lplXFLen - length of the XF buffer.
*           lpXF - pointer to an XF structure that contains XF values that
*                  were assembled earlier, and need to be used in the creation
*                  of this XF structure.
*           lpghFont - buffer for the font for the XF.
*           lplFontLen - length of the font buffer.
*
* Return: long - Success or Failure
***********************************************************************/
long ssm_BuildXF(SS_COORD Col, SS_COORD Row, short sXFType, LPSHORT lpsIndex, LPSHORT lpsCount,
                 TBGLOBALHANDLE FAR *lpghXF, LPBIFF lpBiff, LPVOID lp, LPLONG lplXFLen, LPVOID lpXF,
                 TBGLOBALHANDLE FAR *lpghFont, LPLONG lplFontLen)
{
  LPSS_BOOK     lpBook = (LPSS_BOOK)lp;
  LPSPREADSHEET lpSS = SS_BookLockSheetIndex(lpBook, lpBook->nSheet);
  xXF8          xf;
  BOOL          fFound = FALSE;
  SS_CELLTYPE   ct;
  SS_COLORTBLITEM ColorTblItem;
  COLORREF      clrFore = (COLORREF)-1;
  COLORREF      clrBack = (COLORREF)-1;
  WORD          wicv = 0;

  if (lpXF)
    memcpy(&xf, lpXF, sizeof(xXF8));
  else
    memset((LPxXF)&xf, 0, sizeof(xXF8));

//  xf.fiBackColor = 0x40 & 0x7f;
//  xf.fiForeColor = 0x41 & 0x3F80;
  xf.fiBackColor = 0x40;
  xf.fiForeColor = 0x41;

  // Style XF
  if (sXFType == 0)
  {
#ifdef SPREAD_JPN
    if (*lpsCount == 0x10)
      xf.wifmt = 0x28;
    else if (*lpsCount == 0x11)
      xf.wifmt = 0x26;
    else if (*lpsCount == 0x12)
      xf.wifmt = 0x08;
    else if (*lpsCount == 0x13)
      xf.wifmt = 0x06;
    else if (*lpsCount == 0x14)
      xf.wifmt = 0x09;
#else
    if (*lpsCount == 0x10)
      xf.wifmt = 0x2B;
    else if (*lpsCount == 0x11)
      xf.wifmt = 0x29;
    else if (*lpsCount == 0x12)
      xf.wifmt = 0x2C;
    else if (*lpsCount == 0x13)
      xf.wifmt = 0x2A;
    else if (*lpsCount == 0x14)
      xf.wifmt = 0x09;
#endif
    xf.wifnt   = 0;
    xf.fLocked = 0;
    xf.fStyle = 1;
    xf.ixfParent = 0xFFF; //always 0xFFF for Style XF
    xf.f123Pfx = 0; //always 0 for Style XF  
    xf.cIndent = 0;

    if (*lpsCount == 0)
    {
      xf.fAtrBdr = 0;
      xf.fAtrPat = 0;
      xf.fAtrProt = 0;
      xf.fAtrNum = 0;
      xf.fAtrFnt = 0;
    }
    else if (*lpsCount < 15)
    {
      xf.fAtrBdr = 1;
      xf.fAtrPat = 1;
      xf.fAtrProt = 1;
      xf.fAtrNum = 1;
      xf.fAtrFnt = 0;
    }
    else
    {
      xf.fAtrBdr = 1;
      xf.fAtrPat = 1;
      xf.fAtrProt = 1;
      xf.fAtrNum = 0;
      xf.fAtrFnt = 1;
    }

    xf.fAlign = 0; //left
    xf.fVAlign = 0; //top
    xf.fJustLast = 0;
    xf.trot = 0; //rotation
    xf.fShrinkToFit = 0;
    xf.iReadingOrder = 0;
    xf.fAtrAlc = 1;
  }
  // Cell XF
  else if (sXFType == 1)
  {
//    int      Border[4];
//    COLORREF Color[4];

    if (Col == SS_ALLCOLS && Row == SS_ALLROWS)
    {
      //Add the Default SpreadSheet - XF record
      SS_GetColorItem(&ColorTblItem, lpSS->Color.ForegroundId);
      if (ColorTblItem.Color != lpBook->clrForeDefOrig)
      {
        if (ColorTblItem.Color != -1)
          xf.fiForeColor = xl_IndexFromColor(ColorTblItem.Color);
      }

      SS_GetColorItem(&ColorTblItem, lpSS->Color.BackgroundId);

      if (ColorTblItem.Color != lpBook->clrBackDefOrig && ColorTblItem.Color != -1)
      {
        xf.fiBackColor = xl_IndexFromColor(ColorTblItem.Color);
        xf.fAtrPat = 1;
        xf.fls = 1;
      }

      xf.fLocked = (lpSS->DocumentLocked == SS_LOCKED_ON?1:0);

//      xf.dgLeft = 0; 
//      xf.dgTop = 0; 
//      xf.dgRight = 0; 
//      xf.dgBottom = 0; 
//      xf.icvLeft = 0;
//      xf.icvTop = 0;
//      xf.icvRight = 0;
//      xf.icvBottom = 0;
      if (lpSS && lpSS->hBorder)
      {
        LPSS_CELLBORDER lpCellBorder = (LPSS_CELLBORDER)tbGlobalLock(lpSS->hBorder);

        ssm_SaveBorder(lpCellBorder, &xf);
        tbGlobalUnlock(lpSS->hBorder);
      }

      if (*lpsCount != 15)
      {
//17472 -- I made an incorrect assumption here.  The parent must be a style xf, not a cell xf.
//         since the xf at index 15 is a cell xf, it was causing a problem with Excel.
//         Until this is better spec'ed out, use 0 for the parent index
//        xf.ixfParent = 15; //Workbook default   <<Sheet default for Spread30>>
        xf.ixfParent = 0; //17472
        xf.wifnt = ssm_BuildFont(lpBook, lpghFont, lplFontLen, lpSS->DefaultFontId, xf.fiForeColor);
        xf.fAtrFnt = 1;
      }
      else
      {
        xf.ixfParent = 0; // this is the workbook default.
        xf.wifnt = 0;
        xf.fAtrFnt = 0;
      }
    } 
    else if (Row == SS_ALLROWS)
    {
      //Add a COLINFO - XF record
      LPSS_COL  lpCol = SS_LockColItem(lpSS, Col+lpSS->Col.HeaderCnt-1);
      SS_FONTID FontId = -1; 

//      xf.ixfParent = 15;
      if (lpCol)
      {
        if (lpCol->FontId == -1)
          FontId = lpSS->DefaultFontId;
        else
        {
          FontId = lpCol->FontId;
          xf.fAtrFnt = 1;
        }
      }

      // Follow the heirarchy to determine the correct color settings...
      xf.fAtrPat = 0;
      xf.fls = 0;
    
      if (lpCol && lpCol->Color.ForegroundId != 0)
      {
        SS_GetColorItem(&ColorTblItem, lpCol->Color.ForegroundId);
        clrFore = ColorTblItem.Color;
      }
      else
      {
        SS_GetColorItem(&ColorTblItem, lpSS->Color.ForegroundId);
        clrFore = ColorTblItem.Color;
      }

      if (lpCol && lpCol->Color.BackgroundId != 0)
      {
        SS_GetColorItem(&ColorTblItem, lpCol->Color.BackgroundId);
        clrBack = ColorTblItem.Color;
      }
      else
      {
        SS_GetColorItem(&ColorTblItem, lpSS->Color.BackgroundId);
        clrBack = ColorTblItem.Color;
      }
      if (clrFore != lpBook->clrForeDefOrig)
      {
        if (clrFore != -1)
          wicv = (WORD)xl_IndexFromColor(clrFore);
      }

      if (clrBack != lpBook->clrBackDefOrig)
      { 
        if (clrBack != -1)
        {  
          xf.fiBackColor = xl_IndexFromColor(clrBack);
          xf.fAtrPat = 1;
          xf.fls = 1;
        }
      }

      xf.wifnt = ssm_BuildFont(lpBook, lpghFont, lplFontLen, FontId, xf.fiForeColor);

      if (lpCol)
      {
        if (lpCol->ColLocked != SS_LOCKED_DEF)
          xf.fLocked = (lpCol->ColLocked==SS_LOCKED_ON?1:0);
        else if (lpSS->DocumentLocked == SS_LOCKED_ON)
          xf.fLocked = 1;
      }

      if (lpCol && lpCol->hBorder)
      {
        LPSS_CELLBORDER lpCellBorder = (LPSS_CELLBORDER)tbGlobalLock(lpCol->hBorder);

        ssm_SaveBorder(lpCellBorder, &xf);
//        short           i;
//RAP17a >>
//        BOOL            fSaveBorder = FALSE;
        
//        // Loop through the border elements.  If none of the cell borders are set, do not
//        // save off any of the border info.
//        for (i=0; i<4; i++)
//        {
//          if (lpCellBorder->Border[i].bStyle)
//          {
//            fSaveBorder = TRUE;
//            break;
//          }
//        }
//        if (fSaveBorder)
//        {
//RAP17a <<
//          for (i=0; i<4; i++)
//          {
//            SS_GetColorItem(&ColorTblItem, lpCellBorder->Border[i].idColor);
//            Color[i] = ColorTblItem.Color;
//            switch (lpCellBorder->Border[i].bStyle)
//            {
//              case SS_BORDERSTYLE_NONE:
//                Border[i] = 0x0; 
//              break;
//              case SS_BORDERSTYLE_FINE_SOLID:
//                Border[i] = 0x1;
//              break;
//              case SS_BORDERSTYLE_SOLID:
//                Border[i] = 0x2; 
//              break;
//      			  //Modify By BOC 99.7.1 (hyt)------------------
//			        //for lost Fine Dash borderstyle
//			        case SS_BORDERSTYLE_FINE_DASH:
//                Border[i] = 0x4; 
//              break;
//      			  //---------------------------------------------
//              case SS_BORDERSTYLE_DOT:
//                Border[i] = 0x7; 
//              break;
//              case SS_BORDERSTYLE_FINE_DOT:
//                Border[i] = 0x7; 
//              break;
//              case SS_BORDERSTYLE_DASH:
//                Border[i] = 0x8; 
//              break;
//              case SS_BORDERSTYLE_FINE_DASH_DOT:
//                Border[i] = 0x9; 
//              break;
//              case SS_BORDERSTYLE_DASHDOT:
//                Border[i] = 0xA; 
//              break;
//              case SS_BORDERSTYLE_FINE_DASH_DOT_DOT:
//                Border[i] = 0xB; 
//              break;
//              case SS_BORDERSTYLE_DASHDOTDOT:
//                //Modify By BOC 99.7.1 (hyt)--------------------
//                //for incorrect value
//                //Border[i] = 0xD; 
//                Border[i] = 0xC;
//                //----------------------------------------------
//              break;
//              case SS_BORDERSTYLE_BLANK:
//                Border[i] = 0x0; 
//              break;
  //RAP08a >>
//              default:
//                Border[i] = 0x2; 
//              break;
  //RAP08a <<
//            }
//          }
//          xf.dgLeft = Border[0];
//          xf.dgTop = Border[1]; 
//          xf.dgRight = Border[2]; 
//          xf.dgBottom = Border[3]; 

//          xf.icvLeft = (WORD)xl_IndexFromColor(Color[0]);
//          xf.icvTop = (WORD)xl_IndexFromColor(Color[1]);
//          xf.icvRight = (WORD)xl_IndexFromColor(Color[2]);
//          xf.icvBottom = (WORD)xl_IndexFromColor(Color[3]);
//
//          xf.fAtrBdr = 1;
//        } // if (fSaveBorder)
//
        tbGlobalUnlock(lpCol->hBorder);
      }
      else if (lpSS && lpSS->hBorder)
      {
        LPSS_CELLBORDER lpCellBorder = (LPSS_CELLBORDER)tbGlobalLock(lpSS->hBorder);

        ssm_SaveBorder(lpCellBorder, &xf);
        tbGlobalUnlock(lpSS->hBorder);
      }

      SS_UnlockColItem(lpSS, Col+lpSS->Col.HeaderCnt-1);

      xf.ixfParent = 0; //Workbook default   <<Sheet default for Spread30>>
    } 
    else if (Col == SS_ALLCOLS)
    {
      //Add a ROW - XF record
      LPSS_ROW  lpRow = SS_LockRowItem(lpSS, Row+lpSS->Row.HeaderCnt-1);
      SS_FONTID FontId = -1; 

//      xf.ixfParent = 15;
      if (lpRow)
      {
        if (lpRow->FontId == -1)
          FontId = lpSS->DefaultFontId;
        else
        {
          FontId = lpRow->FontId;
          xf.fAtrFnt = 1;
        }
      }

      // Follow the heirarchy to determine the correct color settings...
      xf.fAtrPat = 0;
      xf.fls = 0;
    
      if (lpRow && lpRow->Color.ForegroundId != 0)
      {
        SS_GetColorItem(&ColorTblItem, lpRow->Color.ForegroundId);
        clrFore = ColorTblItem.Color;
      }
      else
      {
        SS_GetColorItem(&ColorTblItem, lpSS->Color.ForegroundId);
        clrFore = ColorTblItem.Color;
      }

      if (lpRow && lpRow->Color.BackgroundId != 0)
      {
        SS_GetColorItem(&ColorTblItem, lpRow->Color.BackgroundId);
        clrBack = ColorTblItem.Color;
      }
      else
      {
        SS_GetColorItem(&ColorTblItem, lpSS->Color.BackgroundId);
        clrBack = ColorTblItem.Color;
      }
      if (clrFore != lpBook->clrForeDefOrig)
      {
        if (clrFore != -1)
          wicv = (WORD)xl_IndexFromColor(clrFore);
      }

      if (clrBack != lpBook->clrBackDefOrig)
      { 
        if (clrBack != -1)
        {  
          xf.fiBackColor = xl_IndexFromColor(clrBack);
          xf.fAtrPat = 1;
          xf.fls = 1;
        }
      }
      
      xf.wifnt = ssm_BuildFont(lpBook, lpghFont, lplFontLen, FontId, xf.fiForeColor);

      if (lpRow)
      {
        if (lpRow->RowLocked != SS_LOCKED_DEF)
          xf.fLocked = (lpRow->RowLocked==SS_LOCKED_ON?1:0);
        else if (lpSS->DocumentLocked == SS_LOCKED_ON)
          xf.fLocked = 1;
      }

      if (lpRow && lpRow->hBorder)
      {
        LPSS_CELLBORDER lpCellBorder = (LPSS_CELLBORDER)tbGlobalLock(lpRow->hBorder);

        ssm_SaveBorder(lpCellBorder, &xf);
#if 0
        short           i;
//RAP17a >>
        BOOL            fSaveBorder = FALSE;
        
        // Loop through the border elements.  If none of the cell borders are set, do not
        // save off any of the border info.
        for (i=0; i<4; i++)
        {
          if (lpCellBorder->Border[i].bStyle)
          {
            fSaveBorder = TRUE;
            break;
          }
        }
        if (fSaveBorder)
        {
//RAP17a <<
          for (i=0; i<4; i++)
          {
            SS_GetColorItem(&ColorTblItem, lpCellBorder->Border[i].idColor);
            Color[i] = ColorTblItem.Color;
            switch (lpCellBorder->Border[i].bStyle)
            {
              case SS_BORDERSTYLE_NONE:
                Border[i] = 0x0; 
              break;
              case SS_BORDERSTYLE_FINE_SOLID:
                Border[i] = 0x1;
              break;
              case SS_BORDERSTYLE_SOLID:
                Border[i] = 0x2; 
              break;
			  //Modify By BOC 99.7.1 (hyt)------------------
			  //for lost Fine Dash borderstyle
			  case SS_BORDERSTYLE_FINE_DASH:
                Border[i] = 0x4; 
              break;
			  //---------------------------------------------
              case SS_BORDERSTYLE_DOT:
                Border[i] = 0x7; 
              break;
              case SS_BORDERSTYLE_FINE_DOT:
                Border[i] = 0x7; 
              break;
              case SS_BORDERSTYLE_DASH:
                Border[i] = 0x8; 
              break;
              case SS_BORDERSTYLE_FINE_DASH_DOT:
                Border[i] = 0x9; 
              break;
              case SS_BORDERSTYLE_DASHDOT:
                Border[i] = 0xA; 
              break;
              case SS_BORDERSTYLE_FINE_DASH_DOT_DOT:
                Border[i] = 0xB; 
              break;
              case SS_BORDERSTYLE_DASHDOTDOT:
			  //Modify By BOC 99.7.1 (hyt)--------------------
			  //for incorrect value
                //Border[i] = 0xD; 
				  Border[i] = 0xC;
			  //----------------------------------------------
              break;
              case SS_BORDERSTYLE_BLANK:
                Border[i] = 0x0; 
              break;
  //RAP08a >>
              default:
                Border[i] = 0x2; 
              break;
  //RAP08a <<
            }
          }
          xf.dgLeft = Border[0];
          xf.dgTop = Border[1]; 
          xf.dgRight = Border[2]; 
          xf.dgBottom = Border[3]; 
          xf.icvLeft = (WORD)xl_IndexFromColor(Color[0]);
          xf.icvTop = (WORD)xl_IndexFromColor(Color[1]);
          xf.icvRight = (WORD)xl_IndexFromColor(Color[2]);
          xf.icvBottom = (WORD)xl_IndexFromColor(Color[3]);

          xf.fAtrBdr = 1;

        } // if fSaveBorder
#endif //#if 0
        tbGlobalUnlock(lpRow->hBorder);
      }
      else if (lpSS && lpSS->hBorder)
      {
        LPSS_CELLBORDER lpCellBorder = (LPSS_CELLBORDER)tbGlobalLock(lpSS->hBorder);

        ssm_SaveBorder(lpCellBorder, &xf);
        tbGlobalUnlock(lpSS->hBorder);
      }

      SS_UnlockRowItem(lpSS, Row+lpSS->Row.HeaderCnt-1);
      xf.ixfParent = 0; //Workbook default   <<Sheet default for Spread30>>
    }
// 17472 >>
    else if (Row == -2 && Col == -2)
    {

    }
// << 17472
    else
    {
      //Add a cell - XF record
      LPSS_CELL lpCell = SS_LockCellItem(lpSS, Col+lpSS->Col.HeaderCnt-1, Row+lpSS->Row.HeaderCnt-1);
      TBGLOBALHANDLE ghBorder[] = {0,0,0};
      SS_FONTID FontId; 
      LPSS_COL  lpCol = NULL;
      LPSS_ROW  lpRow = NULL;

//      xf.ixfParent = 15; //Workbook default   <<Sheet default for Spread30>>

      clrBack = (COLORREF)-1;
      clrFore = (COLORREF)-1;
      if (lpCell && lpCell->FontId == -1)
  	  {
	  	  //Modify by BOC 99.7.8(hyt)----------------------
  		  //not get FontID correct at sometimes
	    	//FontId = lpSS->DefaultFontId;
    		FontId = SS_GetFontId(lpSS, Col+lpSS->Col.HeaderCnt-1, Row+lpSS->Row.HeaderCnt-1);
		    if(FontId != lpSS->DefaultFontId)
    			xf.fAtrFnt = 1;	
    		//-----------------------------------------------
  	  }
      else
      {
        if (lpCell)
          FontId = lpCell->FontId;
        else
//RAP11d          FontId = 0;
          FontId = lpSS->DefaultFontId; //RAP11a
        xf.fAtrFnt = 1;
      }

      // Follow the heirarchy to determine the correct color settings...
      xf.fAtrPat = 0;
      xf.fls = 0;
    
      lpCol = SS_LockColItem(lpSS, Col+lpSS->Col.HeaderCnt-1);
      lpRow = SS_LockRowItem(lpSS, Row+lpSS->Row.HeaderCnt-1);

//      SS_GetOddEvenRowColor(lpSS, (LPCOLORREF)&clrBackOdd, (LPCOLORREF)&clrForeOdd, (LPCOLORREF)&clrBackEven, (LPCOLORREF)&clrForeEven);
    
      if (lpCell && lpCell->Color.ForegroundId != 0)
      {
        SS_GetColorItem(&ColorTblItem, lpCell->Color.ForegroundId);
        clrFore = ColorTblItem.Color;
      }
      else if (lpCol && lpCol->Color.ForegroundId != 0)
      {
        SS_GetColorItem(&ColorTblItem, lpCol->Color.ForegroundId);
        clrFore = ColorTblItem.Color;
      }
      else if (lpRow && lpRow->Color.ForegroundId != 0)
      {
        SS_GetColorItem(&ColorTblItem, lpRow->Color.ForegroundId);
        clrFore = ColorTblItem.Color;
      }
/*
      else if (0 == Row%2 && clrForeEven != -1)
        clrFore = clrForeEven;
      else if (0 != Row%2 && clrForeOdd != -1)
        clrFore = clrForeOdd;
*/
      else
      {
        SS_GetColorItem(&ColorTblItem, lpSS->Color.ForegroundId);
        clrFore = ColorTblItem.Color;
      }

      if (lpCell && lpCell->Color.BackgroundId != 0)
      {
        SS_GetColorItem(&ColorTblItem, lpCell->Color.BackgroundId);
        clrBack = ColorTblItem.Color;
      }
      else if (lpCol && lpCol->Color.BackgroundId != 0)
      {
        SS_GetColorItem(&ColorTblItem, lpCol->Color.BackgroundId);
        clrBack = ColorTblItem.Color;
      }
      else if (lpRow && lpRow->Color.BackgroundId != 0)
      {
        SS_GetColorItem(&ColorTblItem, lpRow->Color.BackgroundId);
        clrBack = ColorTblItem.Color;
      }
//        else if (0 == Row%2 && clrBackEven != -1)
//          clrBack = clrBackEven;
//        else if (0 != Row%2 && clrBackOdd != -1)
//          clrBack = clrBackOdd;
      else
      {
        SS_GetColorItem(&ColorTblItem, lpSS->Color.BackgroundId);
        clrBack = ColorTblItem.Color;
      }
      if (clrFore != lpBook->clrForeDefOrig)
      {
        if (clrFore != -1)
          wicv = (WORD)xl_IndexFromColor(clrFore);
      }

      if (clrBack != lpBook->clrBackDefOrig)
      { 
        if (clrBack != -1)
        {  
          xf.fiBackColor = xl_IndexFromColor(clrBack) & 0x7f;
          xf.fAtrPat = 1;
          xf.fls = 1;
       }
      }

      xf.wifnt = ssm_BuildFont(lpBook, lpghFont, lplFontLen, FontId, wicv);

      if (lpCell)
      {
        if (lpCell->CellLocked != SS_LOCKED_DEF)
          xf.fLocked = (lpCell->CellLocked==SS_LOCKED_ON?1:0);
        else if (NULL != lpCol && lpCol->ColLocked != SS_LOCKED_DEF)
        {
          xf.fLocked = (lpCol->ColLocked==SS_LOCKED_ON?1:0);
          SS_UnlockColItem(lpSS, Col+lpSS->Col.HeaderCnt-1);
        }
        else if (NULL != lpRow && lpRow->RowLocked != SS_LOCKED_DEF)
        {
          xf.fLocked = (lpRow->RowLocked==SS_LOCKED_ON?1:0);
          SS_UnlockRowItem(lpSS, Row+lpSS->Row.HeaderCnt-1);
        }
        if (lpSS->DocumentLocked == SS_LOCKED_ON)
          xf.fLocked = 1;
      }

      if (lpCell && lpCell->hBorder)
        ghBorder[0] = lpCell->hBorder;
      else if (lpCol && lpCol->hBorder)
        ghBorder[1] = lpCol->hBorder;
      else if (lpRow && lpRow->hBorder)
        ghBorder[2] = lpRow->hBorder;

        
      if (ghBorder[0] || ghBorder[1] || ghBorder[2] || lpSS->hBorder)
      {
        LPSS_CELLBORDER lpCellBorder = (LPSS_CELLBORDER)(ghBorder[0]?tbGlobalLock(ghBorder[0]):NULL);
        LPSS_CELLBORDER lpColBorder = (LPSS_CELLBORDER)(ghBorder[1]?tbGlobalLock(ghBorder[1]):NULL);
        LPSS_CELLBORDER lpRowBorder = (LPSS_CELLBORDER)(ghBorder[2]?tbGlobalLock(ghBorder[2]):NULL);
        LPSS_CELLBORDER lpSheetBorder = (LPSS_CELLBORDER)(lpSS->hBorder?tbGlobalLock(lpSS->hBorder):NULL);
        SS_CELLBORDER   cellBorder;
        short           i;
//RAP17a >>
//        BOOL            fSaveBorder = FALSE;
        
        memset(&cellBorder, 0, sizeof(SS_CELLBORDER));

        // merge the border records
        if (lpCellBorder == NULL)
        {
          // if there is not specified border for the "Cell"
          memcpy(&cellBorder,(lpColBorder==NULL?lpRowBorder==NULL?lpSheetBorder:lpRowBorder:lpColBorder), sizeof(SS_CELLBORDER));
        }
        else
          memcpy(&cellBorder, lpCellBorder, sizeof(SS_CELLBORDER));

        if (lpColBorder || lpRowBorder)
        {
          for (i=0; i<4; i++)
          {
            if (!cellBorder.Border[i].bStyle)
            {
              if (lpColBorder && lpColBorder->Border[i].bStyle)
              {
                cellBorder.Border[i].bStyle = lpColBorder->Border[i].bStyle;
                cellBorder.Border[i].idColor = lpColBorder->Border[i].idColor;
              }
              else if (lpRowBorder && lpRowBorder->Border[i].bStyle)
              {
                cellBorder.Border[i].bStyle = lpRowBorder->Border[i].bStyle;
                cellBorder.Border[i].idColor = lpRowBorder->Border[i].idColor;
              }
            }
          }  
        }

        ssm_SaveBorder(&cellBorder, &xf);
        if (lpCellBorder != NULL)
          tbGlobalUnlock(ghBorder[0]);
        if (lpColBorder != NULL)
          tbGlobalUnlock(ghBorder[1]);
        if (lpRowBorder != NULL)
          tbGlobalUnlock(ghBorder[2]);
        if (lpSS->hBorder != NULL)
          tbGlobalUnlock(lpSS->hBorder);
      }
      else
        xf.fAtrBdr = 0;

      SS_UnlockCellItem(lpSS, Col+lpSS->Col.HeaderCnt-1, Row+lpSS->Row.HeaderCnt-1);
      SS_UnlockColItem(lpSS, Col+lpSS->Col.HeaderCnt-1);
      SS_UnlockRowItem(lpSS, Row+lpSS->Row.HeaderCnt-1);
    
//      xf.fAtrFnt = 1;
      xf.fWrap = 1;

    }
    xf.fHidden = 0;
    xf.fStyle = 0;
    xf.f123Pfx = 0;
    xf.fJustLast = 0;
    xf.cIndent = 0;
    xf.fShrinkToFit = 0;
    xf.iReadingOrder = 0;
    xf.fAtrNum = 0;
    xf.fAtrAlc = 0;
    xf.fAtrProt = 0;
    xf.grbitDiag = 0;
    xf.dgDiag = 0;
  }

  memset(&ct, 0, sizeof(SS_CELLTYPE));
  SSGetCellType(lpBook->hWnd, Col, Row, &ct);
  //Modify By BOC 99.8.31 (hyt) ----------------------------------
  //for TypeTextWordwrap not export correct
  //if ((ct.Type == SS_TYPE_EDIT ||
  //     ct.Type == SS_TYPE_STATICTEXT) && (ct.Style & ES_MULTILINE))
  if ((ct.Type == SS_TYPE_EDIT && (ct.Style & ES_MULTILINE))||
      (ct.Type == SS_TYPE_STATICTEXT && ct.Style & SS_TEXT_WORDWRAP))
    xf.fWrap = 1;
  else
/* RAP19d >>
  {
	if (SSGetBool(lpSS->lpBook->hWnd, SSB_ALLOWCELLOVERFLOW))
      xf.fWrap  = 0;
    else
      xf.fWrap  = 1;
  }
<< RAP19d */
//RAP19a
    xf.fWrap  = 0;

//  if (*lpsCount > 16)
  if (*lpsCount > 21)
  {
    //we are past the mandatory 15 XF records for the outline styles
    int i;
    LPxXF8 lpXF;
    LPBYTE lpb = (LPBYTE)tbGlobalLock(*lpghXF);
    short  sXFSize = sizeof(BIFF)+sizeof(xXF8);
    
    for (i=15; i<*lpsCount; i++)
    {  
      lpXF = (LPxXF8)(lpb+(i*sXFSize)+sizeof(BIFF));
      if (!memcmp(lpXF, &xf, sizeof(xXF8)))
      {
        *lpsIndex = (short)i;
        fFound = TRUE;
        break;
      }
    }
  }

  if (!fFound)
  {
    (*lpsCount)++;
    *lpsIndex = (short)(*lpsCount-1);
    ssm_WriteBuffer(lpghXF, lpBiff, lp, lplXFLen, (LPVOID)&xf);
  }

  return S_OK;
}

long ssm_SaveBorder(LPSS_CELLBORDER lpCellBorder, LPxXF8 lpxf)
{
  SS_COLORTBLITEM ColorTblItem;
  short           i;
//RAP17a >>
  BOOL            fSaveBorder = FALSE;
  int             Border[4];
  COLORREF        Color[4];

  // Loop through the border elements.  If none of the cell borders are set, do not
  // save off any of the border info.
  for (i=0; i<4; i++)
  {
    if (lpCellBorder->Border[i].bStyle)
    {
      fSaveBorder = TRUE;
      break;
    }
  }
  if (fSaveBorder)
  {
//RAP17a <<
    for (i=0; i<4; i++)
    {
      SS_GetColorItem(&ColorTblItem, lpCellBorder->Border[i].idColor);
      Color[i] = ColorTblItem.Color;
      switch (lpCellBorder->Border[i].bStyle)
      {
        case SS_BORDERSTYLE_NONE:
          Border[i] = 0x0; 
        break;
        case SS_BORDERSTYLE_FINE_SOLID:
          Border[i] = 0x1;
        break;
        case SS_BORDERSTYLE_SOLID:
          Border[i] = 0x2; 
        break;
        //Modify By BOC 99.7.1 (hyt)------------------
			  //for lost Fine Dash borderstyle
			  case SS_BORDERSTYLE_FINE_DASH:
          Border[i] = 0x4; 
        break;
        //---------------------------------------------
        case SS_BORDERSTYLE_DOT:
          Border[i] = 0x7; 
        break;
        case SS_BORDERSTYLE_FINE_DOT:
          Border[i] = 0x7; 
        break;
        case SS_BORDERSTYLE_DASH:
          Border[i] = 0x8; 
        break;
        case SS_BORDERSTYLE_FINE_DASH_DOT:
          Border[i] = 0x9; 
        break;
        case SS_BORDERSTYLE_DASHDOT:
          Border[i] = 0xA; 
        break;
        case SS_BORDERSTYLE_FINE_DASH_DOT_DOT:
          Border[i] = 0xB; 
        break;
        case SS_BORDERSTYLE_DASHDOTDOT:
          //Modify By BOC 99.7.1 (hyt)--------------------
          //for incorrect value
          //Border[i] = 0xD; 
          Border[i] = 0xC;
          //----------------------------------------------
        break;
        case SS_BORDERSTYLE_BLANK:
          Border[i] = 0x0; 
        break;
//RAP08a >>
        default:
          Border[i] = 0x2; 
        break;
//RAP08a <<
      }
    }
    lpxf->dgLeft = Border[0];
    lpxf->dgTop = Border[1]; 
    lpxf->dgRight = Border[2]; 
    lpxf->dgBottom = Border[3]; 

    lpxf->icvLeft = (WORD)xl_IndexFromColor(Color[0]);
    lpxf->icvTop = (WORD)xl_IndexFromColor(Color[1]);
    lpxf->icvRight = (WORD)xl_IndexFromColor(Color[2]);
    lpxf->icvBottom = (WORD)xl_IndexFromColor(Color[3]);

    lpxf->fAtrBdr = 1;
  }

  return 0;
}


/***********************************************************************
* Name:   ssm_BuildSTYLE
*
* Usage:  long ssm_BuildSTYLE(short nBuiltIn, TBGLOBALHANDLE FAR *lpghStyle, LPLONG lplStyleLen)
*           lpghStyle - buffer
*           lplStyleLen - length of the buffer.
*
* Return: long - Success or Failure
***********************************************************************/
long ssm_BuildSTYLE(LPVOID lp, TBGLOBALHANDLE FAR *lpghStyle, LPLONG lplStyleLen)
{
  BIFF          biff;
  BISTYLE       style;

  memset((LPBISTYLE)&style, 0, sizeof(BISTYLE));

  biff.recnum = 0x293;
  biff.datalen = 4;

  style.ixfe = 0x8010;
  style.istyBuiltIn = 3;
  style.iLevel = 0xFF;
  ssm_WriteBuffer(lpghStyle, (LPBIFF)&biff, lp, lplStyleLen, (LPVOID)&style);

  style.ixfe = 0x8011;
  style.istyBuiltIn = 6;
  style.iLevel = 0xFF;
  ssm_WriteBuffer(lpghStyle, (LPBIFF)&biff, lp, lplStyleLen, (LPVOID)&style);

  style.ixfe = 0x8012;
  style.istyBuiltIn = 4;
  style.iLevel = 0xFF;
  ssm_WriteBuffer(lpghStyle, (LPBIFF)&biff, lp, lplStyleLen, (LPVOID)&style);

  style.ixfe = 0x8013;
  style.istyBuiltIn = 7;
  style.iLevel = 0xFF;
  ssm_WriteBuffer(lpghStyle, (LPBIFF)&biff, lp, lplStyleLen, (LPVOID)&style);

  style.ixfe = 0x8000;
  style.istyBuiltIn = 0;
  style.iLevel = 0xFF;
  ssm_WriteBuffer(lpghStyle, (LPBIFF)&biff, lp, lplStyleLen, (LPVOID)&style);

  style.ixfe = 0x8014;
  style.istyBuiltIn = 5;
  style.iLevel = 0xFF;
  ssm_WriteBuffer(lpghStyle, (LPBIFF)&biff, lp, lplStyleLen, (LPVOID)&style);
 
  return S_OK;
}


/***********************************************************************
* Name:   ssm_BuildFormat - Build an Excel BIFF FORMAT structure & data
*                           and return the data.
*
* Usage:  long ssm_BuildFormat(LPSHORT lpsCount, TBGLOBALHANDLE FAR *lpghFormat,
*                              LPBIFF lpBiff, LPVOID lp, LPLONG lplFormatLen,
*                              LPSTR lpstr)
*           lpsCount - format count
*           lpghFormat - buffer
*           lpBiff - pointer to the BIFF structure for the FORMAT 
*           lp - pointer to the Spread structure.
*           lplFormatLen - buffer length
*           lpstr - format string
*
* Return: long - Success or Failure
***********************************************************************/
long ssm_BuildFormat(LPSHORT lpsCount, TBGLOBALHANDLE FAR *lpghFormat, LPBIFF lpBiff, LPVOID lp, LPLONG lplFormatLen, LPTSTR lptstr, BOOL fUnicode)
{
  FORMAT        fmt;
  TBGLOBALHANDLE  ghBiffStr = (TBGLOBALHANDLE)0;
  TBGLOBALHANDLE  ghFormat = (TBGLOBALHANDLE)0;
  LPVOID        lpFormat = NULL;
  LPBYTE        lpByte = NULL;
  LPFORMAT      lpFmt = NULL;
  LPBIFFSTR     lpbiffstr = NULL;
  LPBIFF        lpbiff;
  long          lIndex = -1;
  short         i;

  memset((LPFORMAT)&fmt, 0, sizeof(FORMAT));

  (*lpsCount)++;

  fmt.wifmt = (WORD)(0xA4 + *lpsCount); //0xA4 seems to be the base format index.

  ssm_BuildBIFFStr((LPTSTR)lptstr, (LPTBGLOBALHANDLE)&ghBiffStr, fUnicode);
  lpbiffstr = (LPBIFFSTR)tbGlobalLock(ghBiffStr);
  
  //----Add By BOC for support DBCS  99.5.5(hyt)--------
  //if content have DBCS must save as UNICODE format
  long lLen;
#ifdef _UNICODE
	lLen = lpbiffstr->wCharCount*sizeof(TCHAR);
#else
	if(lpbiffstr->fHighByte || fUnicode)
		lLen = lpbiffstr->wCharCount*sizeof(WCHAR);
	else
		lLen = lpbiffstr->wCharCount*sizeof(TCHAR);
#endif

  Alloc(&ghFormat, sizeof(FORMAT) + sizeof(BIFFSTR)-1+lLen);
  lpFormat = (LPVOID)tbGlobalLock(ghFormat);
  lpByte = (LPBYTE)lpFormat;
  memcpy(lpFormat, &fmt, sizeof(FORMAT));
  lpByte += sizeof(FORMAT);
  memcpy(lpByte, lpbiffstr, sizeof(BIFFSTR)-1+lLen);

  lpbiff = (LPBIFF)tbGlobalLock(*lpghFormat);
  for (i=0; i<*lpsCount; i++)
  {
    lpFmt = (LPFORMAT)((LPBYTE)lpbiff + sizeof(BIFF) + sizeof(WORD));
    if (!memcmp(lpFmt, (LPBYTE)lpFormat+sizeof(WORD), lpbiff->datalen-sizeof(WORD)))
    {
      //Format found!!!
      lIndex = i;
      (*lpsCount)--;
      break;
    }
    lpbiff = (LPBIFF)((LPBYTE)lpbiff + sizeof(BIFF) + lpbiff->datalen);
  }
  tbGlobalUnlock(*lpghFormat);
  
  if (lIndex == -1)
  {
    //lpBiff->datalen = sizeof(FORMAT)+sizeof(BIFFSTR)-1+lpbiffstr->wCharCount*sizeof(TCHAR);
	lpBiff->datalen = (WORD)(sizeof(FORMAT)+sizeof(BIFFSTR)-1+lLen);
    ssm_WriteBuffer(lpghFormat, lpBiff, lp, lplFormatLen, (LPVOID)lpFormat);
    lIndex = *lpsCount;
  }
  tbGlobalUnlock(ghFormat);
  tbGlobalFree(ghFormat);
  ghFormat=0;
  tbGlobalUnlock(ghBiffStr);
  tbGlobalFree(ghBiffStr);
  ghBiffStr=0;

  return lIndex;
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
#ifdef SS_V70
BOOL ssm_GetFormula(LPSPREADSHEET lpSS, SS_COORD lCol, SS_COORD lRow, LPTBGLOBALHANDLE lpgh, LPLONG lplen, LPSHORT lpxtiCount, LPTBGLOBALHANDLE lpghXTI, LPTBGLOBALHANDLE lpghCustomNames, LPLONG lplCustomNamesLen)
#else
BOOL ssm_GetFormula(LPSPREADSHEET lpSS, SS_COORD lCol, SS_COORD lRow, LPTBGLOBALHANDLE lpgh, LPLONG lplen)
#endif
{
  CALC_HANDLE hExpr;
  long        lColMin;
  long        lColMax;
  long        lRowMin;
  long        lRowMax;
  long        i;
  long        j;
  BOOL        fIsFormula = FALSE;
//  lRow -= lpSS->Row.HeaderCnt;
//  lCol -= lpSS->Col.HeaderCnt;
  
  if( SS_ALLCOLS == lCol )
  {
    lColMin = min(MatMinCol(&lpSS->CalcInfo.Cells), VectMinIndex(&lpSS->CalcInfo.Cols));
    lColMax = max(MatMaxCol(&lpSS->CalcInfo.Cells), VectMaxIndex(&lpSS->CalcInfo.Cols));
  }
  else
  {
    lColMin = lCol;
    lColMax = lColMin;
  }
  if( SS_ALLROWS == lRow )
  {
    lRowMin = min(MatMinRow(&lpSS->CalcInfo.Cells), VectMinIndex(&lpSS->CalcInfo.Rows));
    lRowMax = max(MatMaxRow(&lpSS->CalcInfo.Cells), VectMaxIndex(&lpSS->CalcInfo.Rows));
  }
  else
  {
//!!rap    lRowMin = lRow-lpSS->Row.HeaderCnt;
    lRowMin = lRow;
    lRowMax = lRowMin;
  }
  for( i = lColMin; i <= lColMax; i++ )
  {
    for( j = lRowMin; j <= lRowMax; j++ )
    {
// GAB 1/25/2004      hExpr = CellGetExpr(&lpSS->CalcInfo.Cells, i, j);
            hExpr = CellGetExpr(&lpSS->CalcInfo, i, j);
      if(hExpr)
      {
        fIsFormula = TRUE;
#ifdef SS_V70
        if (ssm_Save4Expr(lpSS, lCol, lRow, hExpr, lpgh, lplen, lpxtiCount, lpghXTI, lpghCustomNames, lplCustomNamesLen))
#else
        if (ssm_Save4Expr(lpSS, lCol, lRow, hExpr, lpgh, lplen, NULL, NULL, NULL, NULL))
#endif
        {
          fIsFormula = FALSE;
          break;
        }
      }
    }
    if (fIsFormula == FALSE)
      break;
  }

  return fIsFormula;
}

/***********************************************************************
* Name:   ssm_AddFormula - Build an Excel BIFF FORMULA structure & data and
*                          return the data.
*
* Usage:  long ssm_AddFormula(LPSPREADSHEET lpSS, SS_COORD lCol, SS_COORD lRow,
*                             TBGLOBALHANDLE ghFormula, long lFormulaLen,
*                             short sXFIndex, LPTBGLOBALHANDLE lpgh, LPLONG lplen)
*           lpSS - pointer to the Spread structure.
*           lCol - column 
*           lRow - row
*           ghFormula - formula build in ssm_Save4Expr().
*           lFormulaLen - return buffer length
*           sXFIndex - Extended Format(XF) index.
*           lpgh - return buffer
*           lplen - return buffer length
*
* Return: long - Success or Failure
***********************************************************************/
long ssm_AddFormula(LPSS_BOOK lpBook, SS_COORD lCol, SS_COORD lRow, TBGLOBALHANDLE ghFormula, long lFormulaLen, short sXFIndex, double *lpdblVal, TBGLOBALHANDLE *lpghVal, LPTBGLOBALHANDLE lpgh, LPLONG lplen)
{
  //Build the Biff...
  BIFF         biff;
  FORMULA      formula;
  TBGLOBALHANDLE gh = (TBGLOBALHANDLE)0;
  LPBYTE       lp = NULL;
  LPBYTE       lpFormula = NULL;
  byte         dbl[8] = {0,0,0,0,0,0,0xFF,0xFF};

  memset(&biff, 0, sizeof(BIFF));
  memset(&formula, 0, sizeof(FORMULA));

  biff.recnum = xlFORMULA2;
  biff.datalen = (WORD)(sizeof(FORMULA) + (short)lFormulaLen);

  formula.wRow = (WORD)(lRow-1);
  formula.wCol = (WORD)(lCol-1);
  formula.wixfe = sXFIndex;

  if (*lpghVal == (TBGLOBALHANDLE)0)
  {
    // number
    formula.dblNum = *lpdblVal;
  }
  else
  {
    // string
    memcpy(&formula.dblNum, dbl, sizeof(double));
  }
//  formula.dblNum = 0.0; 
//  formula.fAlwaysCalc = 1;
//  formula.fCalcOnLoad = 1;
  formula.fAlwaysCalc = 0;
  formula.fCalcOnLoad = 0;
  formula.fShrFmla = 0;
  formula.chn = (DWORD)0;
  formula.wFormulaLen = (WORD)lFormulaLen;

  Alloc(&gh, sizeof(FORMULA) + lFormulaLen);
  lp = (LPBYTE)tbGlobalLock(gh);
  lpFormula = (LPBYTE)tbGlobalLock(ghFormula);
  memcpy(lp, &formula, sizeof(FORMULA));
  memcpy(lp + sizeof(FORMULA), lpFormula, lFormulaLen);

  ssm_WriteBuffer((TBGLOBALHANDLE FAR *)lpgh, &biff, lpBook, lplen, lp);

  tbGlobalUnlock(ghFormula);
  tbGlobalUnlock(gh);
  tbGlobalFree(gh);
  gh=0;

  if (!memcmp(&formula.dblNum, &dbl, sizeof(double)))
  {
    LPBYTE         lpb = NULL;
    long           lLen = 0;
    TBGLOBALHANDLE gh = 0;
    LPTSTR         lptstr = (LPTSTR)tbGlobalLock(*lpghVal);

    biff.recnum = xlSTRING;
    lLen = ssm_BuildBIFFStr(lptstr, &gh, FALSE);
    lpb = (LPBYTE)tbGlobalLock(gh);
    biff.datalen = (unsigned short)lLen;
    ssm_WriteBuffer(lpgh, &biff, lpBook, lplen, lpb);
    tbGlobalUnlock(gh);
    tbGlobalFree(gh);
    gh=0;
    tbGlobalUnlock(*lpghVal);
    tbGlobalFree(*lpghVal);
    *lpghVal = 0;
  }
  return S_OK;
}

/***********************************************************************
* Name:   ssm_IsRowDefault - Determine if the row has been modified
*
* Usage:  BOOL ssm_IsRowDefault(LPSPREADSHEET lpSS, SS_COORD lRow)
*           lpSS - pointer to the Spread structure.
*           lRow - row
* Return: BOOL - Is the row default ?
***********************************************************************/
BOOL ssm_IsRowDefault(LPSPREADSHEET lpSS, SS_COORD lRow)
{
  LPSS_ROW  lpRow = SS_LockRowItem(lpSS, lRow+lpSS->Row.HeaderCnt-1);
  SS_ROW    ssRow;
  BOOL      bRet = TRUE;

  if (lpRow)
  {
    memset((LPSS_ROW)&ssRow, 0, sizeof(SS_ROW));
    ssRow.dRowHeightX100 = SS_HEIGHT_DEFAULT;
    ssRow.dRowHeightInPixels = SS_HEIGHT_DEFAULT;
    ssRow.RowMaxFontId = SS_FONT_NONE;
    ssRow.FontId = SS_FONT_NONE;
    ssRow.Cells = lpRow->Cells;

    if (memcmp(lpRow, (LPSS_ROW)&ssRow, sizeof(SS_ROW)))
      bRet = FALSE;

    SS_UnlockRowItem(lpSS, lRow+lpSS->Row.HeaderCnt-1);
  }

  return bRet;
}

/***********************************************************************
* Name:   ssm_IsRowDefaultDrawing - Determine if the row has been modified
*
* Usage:  BOOL ssm_IsRowDefault(LPSPREADSHEET lpSS, SS_COORD lRow)
*           lpSS - pointer to the Spread structure.
*           lRow - row
* Return: BOOL - Is the row default ?
***********************************************************************/
BOOL ssm_IsRowDefaultDrawing(LPSPREADSHEET lpSS, SS_COORD lRow)
{
  LPSS_ROW  lpRow = SS_LockRowItem(lpSS, lRow+lpSS->Row.HeaderCnt-1);
  //SS_ROW    ssRow;
  BOOL      bRet = TRUE;

  if (lpRow)
  {
    SS_COLORTBLITEM ColorTblItem;
    
    SS_GetColorItem(&ColorTblItem, lpRow->Color.BackgroundId);
/* 17421
    memset((LPSS_ROW)&ssRow, 0, sizeof(SS_ROW));
    ssRow.dRowHeightX100 = SS_HEIGHT_DEFAULT;
    ssRow.dRowHeightInPixels = SS_HEIGHT_DEFAULT;
    ssRow.RowMaxFontId = SS_FONT_NONE;
    ssRow.FontId = SS_FONT_NONE;
    ssRow.Cells = lpRow->Cells;
*/
    if (ColorTblItem.Color != SPREAD_COLOR_NONE && ColorTblItem.Color != -1)
      bRet = FALSE;
/* 17421   
    if (bRet)
    {
      if (memcmp(lpRow, (LPSS_ROW)&ssRow, sizeof(SS_ROW)))
         bRet = FALSE;
    }
*/
    SS_UnlockRowItem(lpSS, lRow+lpSS->Row.HeaderCnt-1);
  }

  return bRet;
}

/***********************************************************************
* Name:   ssm_IsColDefault - Determine if the column has been modified.
*
* Usage:  BOOL ssm_IsColDefault(LPSPREADSHEET lpSS, SS_COORD lCol)
*           lpSS - pointer to the Spread structure.
*           lCol - column
*
* Return: BOOL - Is the column default ?
***********************************************************************/
BOOL ssm_IsColDefault(LPSPREADSHEET lpSS, SS_COORD lCol)
{
  LPSS_COL  lpCol = SS_LockColItem(lpSS, lCol+lpSS->Col.HeaderCnt-1);
  SS_COL    ssCol;
  BOOL      bRet = TRUE;

  if (lpCol)
  {
    memset((LPSS_COL)&ssCol, 0, sizeof(SS_COL));

    ssCol.FontId = SS_FONT_NONE;
    ssCol.dColWidthX100 = SS_WIDTH_DEFAULT;
    ssCol.dColWidthInPixels = SS_WIDTH_DEFAULT;
#ifdef SS_V35
    ssCol.nSortIndicator = -1;
#endif
    if (memcmp(lpCol, (LPSS_COL)&ssCol, sizeof(SS_COL)))
      bRet = FALSE;

    SS_UnlockColItem(lpSS, lCol+lpSS->Col.HeaderCnt-1);
  }

  return bRet;
}

/***********************************************************************
* Name:   ssm_IsColDefaultDrawing - Determine if the column has been modified.
*
* Usage:  BOOL ssm_IsColDefault(LPSPREADSHEET lpSS, SS_COORD lCol)
*           lpSS - pointer to the Spread structure.
*           lCol - column
*
* Return: BOOL - Is the column default ?
***********************************************************************/
BOOL ssm_IsColDefaultDrawing(LPSPREADSHEET lpSS, SS_COORD lCol)
{
  LPSS_COL  lpCol = SS_LockColItem(lpSS, lCol+lpSS->Col.HeaderCnt-1);
  //SS_COL    ssCol;
  BOOL      bRet = TRUE;

  if (lpCol)
  {
    SS_COLORTBLITEM ColorTblItem;
    
    SS_GetColorItem(&ColorTblItem, lpCol->Color.BackgroundId);
/* 17421
    memset((LPSS_COL)&ssCol, 0, sizeof(SS_COL));
    ssCol.dColWidthInPixels = -1;
    ssCol.dColWidthX100 = -1;
    ssCol.hColID = lpCol->hColID;
    ssCol.nSortIndicator = lpCol->nSortIndicator;
    ssCol.FontId = -1;
*/
    if (ColorTblItem.Color != SPREAD_COLOR_NONE && ColorTblItem.Color != -1)
      bRet = FALSE;
/* 17421
    if (bRet)
    {
      if (memcmp(lpCol, (LPSS_COL)&ssCol, sizeof(SS_COL)))
        bRet = FALSE;
    }
*/

    SS_UnlockColItem(lpSS, lCol+lpSS->Col.HeaderCnt-1);
  }

  return bRet;
}

/***********************************************************************
* Name:   ssm_IsCellDefault - Determine if the cell has been modified.
*
* Usage:  BOOL ssm_IsCellDefault(LPSPREADSHEET lpSS, SS_COORD lCol, SS_COORD lRow)
*           lpSS - pointer to the Spread structure.
*           lCol - column
*           lRow - row
*
* Return: BOOL - Is the cell default ?
***********************************************************************/
BOOL ssm_IsCellDefault(LPSPREADSHEET lpSS, SS_COORD lCol, SS_COORD lRow)
{
  BOOL      bRet = TRUE;
  SS_CELL   ssCell;
  LPSS_CELL lpCell = SS_LockCellItem(lpSS, lCol+lpSS->Col.HeaderCnt-1, lRow+lpSS->Row.HeaderCnt-1);

  if (lpCell)
  {
    memset((LPSS_CELL)&ssCell, 0, sizeof(SS_CELL));
    ssCell.FontId = SS_FONT_NONE;
    if (memcmp(lpCell, (LPSS_CELL)&ssCell, sizeof(SS_CELL)))
    {
      ssCell.Data.bOverflow = 1;
      if (memcmp(lpCell, (LPSS_CELL)&ssCell, sizeof(SS_CELL)))
        bRet = FALSE;
      else if (memcmp(&(lpCell->Color), &(ssCell.Color), sizeof(SS_COLORITEM)))
        bRet = FALSE;
      else if (memcmp(&(lpCell->Data), &(ssCell.Data), sizeof(SS_DATA)))
        bRet = FALSE;
    }

    SS_UnlockCellItem(lpSS, lCol+lpSS->Col.HeaderCnt-1, lRow+lpSS->Row.HeaderCnt-1);
  }

  // if the cell is default, check to see if it is a span cell.
#ifdef SS_V40
  if (bRet)
  {
    if (SS_SPAN_NO != SS_GetCellSpan(lpSS, lCol+lpSS->Col.HeaderCnt-1, lRow+lpSS->Row.HeaderCnt-1, NULL, NULL, NULL, NULL))
       bRet = FALSE;
  }
#endif

  return bRet;
}

/***********************************************************************
* Name:   ssm_BuildBIFFStr - Build a Biff String from the supplied text string.
*
* Usage:  long ssm_BuildBIFFStr(LPTSTR lptstr, LPTBGLOBALHANDLE lpghBiffStr)
*           lptstr - the string to convert to a Biff String
*           lpghBiffStr - the resultant Biff String
*
* Return: short - byte length of BiffStr
***********************************************************************/
long ssm_BuildBIFFStr(LPTSTR lptstr, LPTBGLOBALHANDLE lpghBiffStr, BOOL fUnicode)
{
  LPBYTE  lp;
  LPTSTR  lpStrDest;
  long    lLen = lstrlen(lptstr);
  long    lTotalLen = 0;
  
//#ifdef SPREAD_JPN
//Add by BOC 99.4.7(hyt) ----------------------------
//For support multibye

#ifndef _UNICODE
  BOOL bDBCS = false;
  BOOL bHighByte = false;
  BYTE* pByte = (BYTE*)lptstr;
  //Modify by BOC 99.8.31 (hyt)--------------------
  //for convert vbCrLf to vbLf
  int i,nCount;
  nCount = lLen;
  for(i=0;i<nCount;i++)
  {
    if(IsDBCSLeadByte(*pByte) || _ismbbkana(*pByte))
  	{
	  	bDBCS = true;
		  pByte += 2;
  		i++;
	  }
    else if(*pByte > 127)
    {
      bHighByte = true;
      pByte++;
    }
  	else if(*pByte ++ ==0x0d && *pByte==0x0a)
	  {
  		memcpy(pByte-1,pByte,nCount-i);
	  	i++;
		  lLen--;
	  }
  }
  
  //-------------------------------------------------------
  //include DBCS char must save  as UNICODE format
  if (bDBCS || fUnicode || bHighByte)		
  {
	  TBGLOBALHANDLE gh = 0;
	  Alloc(&gh, lLen*sizeof(WCHAR));
			
    LPSTR lpstrTmp = (LPSTR)tbGlobalLock(gh);

  	long lStringLen = (long)MultiByteToWideChar(CP_ACP, MB_PRECOMPOSED, lptstr, lLen,
                                                (LPWSTR)lpstrTmp, lLen);

	  lTotalLen = sizeof(BIFFSTR)-sizeof(BYTE)+lStringLen*sizeof(WCHAR);
    Alloc(lpghBiffStr, lTotalLen);
	  
    lp = (LPBYTE)tbGlobalLock(*lpghBiffStr);

	  ((LPBIFFSTR)lp)->fHighByte = 1;
	  ((LPBIFFSTR)lp)->wCharCount = (WORD)lStringLen;
	  
	  lpStrDest = (LPTSTR)(lp + sizeof(BIFFSTR)-1);
	  memcpy(lpStrDest, lpstrTmp, lStringLen*sizeof(WCHAR)); // copy without the null-term.

	  tbGlobalUnlock(*lpghBiffStr);

	  tbGlobalUnlock(gh);
    tbGlobalFree(gh);
    gh=0;

    return lTotalLen;
  }
#else
  WCHAR* pWchar = (WCHAR*)lptstr;
  int    i;
  for (i=0; i<lLen; i++)
  {
  	if (*pWchar ++ ==0x0d && *pWchar==0x0a)
	  {
  		memcpy(pWchar-1, pWchar, (lLen-i)*sizeof(WCHAR));
		  lLen--;
      lptstr[lLen] = 0;
	  }
  }
#endif
//#endif

//19275  lTotalLen = (short)(sizeof(BIFFSTR)-sizeof(BYTE)+(lLen*sizeof(TCHAR)));
  lTotalLen = (sizeof(BIFFSTR)-sizeof(BYTE)+(lLen*sizeof(TCHAR)));//19275
  Alloc(lpghBiffStr, lTotalLen);
  lp = (LPBYTE)tbGlobalLock(*lpghBiffStr);
  ((LPBIFFSTR)lp)->wCharCount = (WORD)lLen;
#ifdef _UNICODE
  ((LPBIFFSTR)lp)->fHighByte = 1;
#else //!_UNICODE
  ((LPBIFFSTR)lp)->fHighByte = 0;
#endif  
  lpStrDest = (LPTSTR)(lp + sizeof(BIFFSTR)-sizeof(BYTE));
  memcpy(lpStrDest, lptstr, lLen*sizeof(TCHAR)); // copy without the null-term.
  tbGlobalUnlock(*lpghBiffStr);
  return lTotalLen;
}


typedef struct _stringItem
{
  TBGLOBALHANDLE ghString;
  long           index;
}STRINGITEM, FAR *LPSTRINGITEM;

long FindBiffStr(LPTSTR lpsz, LPTBGLOBALHANDLE lpghStrings, DWORD dwStringCount)
{
  LPTSTR       lpString = NULL;
  LPSTRINGITEM lpStringItem = NULL;
  int          nNewStringLen = lstrlen(lpsz);

  if (dwStringCount == 0)
  {
    LPTSTR lpStringVal = NULL;
    Alloc(lpghStrings, sizeof(STRINGITEM));
    lpStringItem = (LPSTRINGITEM)tbGlobalLock(*lpghStrings);
    Alloc(&(lpStringItem->ghString), (nNewStringLen+1)*sizeof(TCHAR));
    lpStringVal = (LPTSTR)tbGlobalLock(lpStringItem->ghString);
    _tcscpy(lpStringVal, lpsz);
    lpStringItem->index = 0;
    tbGlobalUnlock(lpStringItem->ghString);

    return -1;
  }
  
  lpStringItem = (LPSTRINGITEM)tbGlobalLock(*lpghStrings);

  int low = 0;
	int high = dwStringCount - 1;
	int middle;
  int i;
	
	while (low <= high)
	{
    int compare = 0;

    middle = (low + high) / 2;
    
    lpString = (LPTSTR)tbGlobalLock(lpStringItem[middle].ghString);
    compare = _tcscmp(lpsz, lpString);
		
    if (compare == 0) //match found
    {
      tbGlobalUnlock(lpStringItem[middle].ghString);
      tbGlobalUnlock(*lpghStrings);
      return lpStringItem[middle].index;
    }
    else if (compare < 0) // search low end of array
      high = middle - 1;
    else // if (compare > 0) // search high end of array
      low = middle + 1;

    tbGlobalUnlock(lpStringItem[middle].ghString);
	}
	
	tbGlobalUnlock(*lpghStrings);
  Alloc(lpghStrings, (dwStringCount+1)*sizeof(STRINGITEM));
  lpStringItem = (LPSTRINGITEM)tbGlobalLock(*lpghStrings);
  
  // move the string items down to provide an insertion point.
  for (i=dwStringCount; i>high+1; i--)
  {
    lpStringItem[i] = lpStringItem[i-1];
  }
  lpStringItem[high+1].index = dwStringCount;
  lpStringItem[high+1].ghString = (TBGLOBALHANDLE)0;
  Alloc(&lpStringItem[high+1].ghString, (nNewStringLen+1)*sizeof(TCHAR));
  lpString = (LPTSTR)tbGlobalLock(lpStringItem[high+1].ghString);
  _tcscpy(lpString, lpsz);
  tbGlobalUnlock(lpStringItem[high+1].ghString);
  tbGlobalUnlock(*lpghStrings);

  return -1L;		//search key not found
}

/*
long FindBiffStr(LPBIFFSTR lpbiffStr, TBGLOBALHANDLE gh)
{
  long   i;
  LPBYTE lpb = (LPBYTE)tbGlobalLock(gh);
  LPSST  lpSST = (LPSST)((LPBYTE)tbGlobalLock(gh) + sizeof(BIFF));
  long   stringCount = lpSST->dwTotal;
  
  lpb += 12;

  for (i=0; i<stringCount; i++)
  {
    LPBIFFSTR lpbfstr = (LPBIFFSTR)lpb;
    long      bfstrByteLen = sizeof(BIFFSTR)-sizeof(BYTE)+lpbfstr->wCharCount*(lpbfstr->fHighByte?2:1);
    
    if (!memcmp((LPBYTE)lpbiffStr, lpb, sizeof(BIFFSTR)))
    {
      // the biffstr header is the same, continue...
      if (!memcmp(lpbiffStr, lpb, bfstrByteLen))
      {
        // we found a match!
        tbGlobalUnlock(gh);
        return i;
      }
    }
    lpb += bfstrByteLen;
  }

  return -1;
}
*/
/***********************************************************************
* Name:   ssm_BuildSST - Add a string to the Shared String Table(SST) and
*                        add the proper reference to the Extended Shared
*                        String Table(EXTSST).
*
* Usage:  long ssm_BuildSST(HWND hWnd, SS_COORD lCol, SS_COORD lRow,
*                           LPTBGLOBALHANDLE lpghSST, LPLONG lplSSTLen,
*                           LPTBGLOBALHANDLE lpghEXTSST, LPLONG lplEXTSSTLen)
*           hWnd - window handle of the Spread control.
*           lCol - column to retrieve the cell string.
*           lRow - row to retrieve the cell string.
*           lpghSST - SST buffer.
*           lplSSTLen - length of the SST buffer.
*           lpghEXTSST - EXTSST buffer.
*           lplEXTSSTLen - length of the EXTSST buffer.
*
* Return: long - The count of strings in the SST, or -1 if there is an error.
***********************************************************************/
long ssm_BuildSST(HWND hWnd, SS_COORD lCol, SS_COORD lRow, LPTBGLOBALHANDLE lpghSST, LPLONG lplSSTLen, LPTBGLOBALHANDLE lpghEXTSST, LPLONG lplEXTSSTLen, BYTE bCellType, LPTSTR lptstrVal, LPTBGLOBALHANDLE ghStrings)
{
  LPBIFF       lpBiff;
  LPBYTE       lp;
  LPBIFFSTR    lpBiffStr;
  LPSST        lpSST;
  LPEXTSST     lpEXTSST;
  ISSTINF      isstinf;
  TBGLOBALHANDLE ghBiffStr = (TBGLOBALHANDLE)0;
  long         lLen = 0;
  TBGLOBALHANDLE ghData = (TBGLOBALHANDLE)0;
  LPTSTR       lptstr = lptstrVal;
  long         lIndex = -1;
  long         lPos = *lplSSTLen;

  if (lptstrVal !=  NULL)
    lLen = lstrlen(lptstrVal);
  else
    lLen = (SSGetDataLen(hWnd, lCol, lRow) +2) * sizeof(TCHAR); //+1 for NULL

  if (lLen)
  {
    if (lptstrVal == NULL)
    {
      ghData = tbGlobalAlloc(GHND, lLen+sizeof(TCHAR)); // len + null char
      lptstr = (LPTSTR)tbGlobalLock(ghData); 

      SSGetData(hWnd, lCol, lRow, lptstr);
    
      if ((bCellType == SS_TYPE_CHECKBOX || bCellType == SS_TYPE_BUTTON) && !_tcscmp(lptstr, _T("0")))
      {
        tbGlobalUnlock(ghData);
        tbGlobalFree(ghData);
        ghData=0;

        return -1;
      }      
    }

    if (*lplSSTLen == 0)
    {
      *lplSSTLen = sizeof(BIFF) + sizeof(SST);
      Alloc(lpghSST, *lplSSTLen);
      lPos = *lplSSTLen;

      lpBiff = (LPBIFF)tbGlobalLock(*lpghSST);
      lpBiff->recnum = xlSST;
      lpBiff->datalen = sizeof(SST);
      tbGlobalUnlock(*lpghSST);

      //Build the EXTSST record
      *lplEXTSSTLen = sizeof(BIFF) + 0x0402;//sizeof(EXTSST) + sizeof(ISSTINF);
      Alloc(lpghEXTSST, *lplEXTSSTLen);
      lp = (LPBYTE)tbGlobalLock(*lpghEXTSST);
      lpBiff = (LPBIFF)lp;
      lpBiff->recnum = xlEXTSST;
      lpBiff->datalen = 0x0402; //All the Excel files I looked at had the EXTSST len= 0x0402
      lpEXTSST = (LPEXTSST)(lp + sizeof(BIFF));
//RAP07d      lpEXTSST->wStringsPerBucket = 8;
      tbGlobalUnlock(*lpghEXTSST);
      
      //do it in 2 steps because I believe it will need to be separated later???
      memset(&isstinf, 0, sizeof(ISSTINF));
      lp = (LPBYTE)tbGlobalLock(*lpghEXTSST);
      lpBiff = (LPBIFF)lp;

      lp += sizeof(BIFF) + sizeof(EXTSST);
      //Set the temporary offset for ib.
//RAP07d      isstinf.ib = sizeof(BIFF) + sizeof(SST); 
//RAP07d      isstinf.cb = sizeof(BIFF) + sizeof(SST); 
      memcpy(lp, &isstinf, sizeof(ISSTINF));
      tbGlobalUnlock(*lpghEXTSST);
    } 
    
    if (lpghSST != NULL)
    {
      lp = (LPBYTE)tbGlobalLock(*lpghSST);
      lpBiff = (LPBIFF)lp;
      lpSST = (LPSST)(lp + sizeof(BIFF));
      lIndex = FindBiffStr(lptstr, ghStrings, lpSST->dwUnique);
      tbGlobalUnlock(*lpghSST);
    }
    
    if (lIndex == -1)
    {
      ssm_BuildBIFFStr(lptstr, &ghBiffStr, FALSE);
    	lpBiffStr = (LPBIFFSTR)tbGlobalLock(ghBiffStr);
#ifdef _UNICODE
	    lLen = lpBiffStr->wCharCount*sizeof(TCHAR);
#else
    	if(lpBiffStr->fHighByte)
  	  	lLen = lpBiffStr->wCharCount*sizeof(WCHAR);
     	else
  	  	lLen = lpBiffStr->wCharCount*sizeof(TCHAR);
#endif

      *lplSSTLen += sizeof(BIFFSTR)-1+lLen;
      Alloc(lpghSST, *lplSSTLen);

      lp = (LPBYTE)tbGlobalLock(*lpghSST);
      lpBiff = (LPBIFF)lp;
      lpSST = (LPSST)(lp + sizeof(BIFF));
      lIndex = lpSST->dwTotal;

      lpSST->dwTotal++;
      lpSST->dwUnique++;

      //lpBiff->datalen += sizeof(BIFFSTR)-1+lpBiffStr->wCharCount*sizeof(TCHAR);
    	lpBiff->datalen = (WORD)(lpBiff->datalen + sizeof(BIFFSTR)-sizeof(BYTE)+lLen);
      lp += lPos;
      //memcpy(lp, lpBiffStr, sizeof(BIFFSTR)-1+lpBiffStr->wCharCount*sizeof(TCHAR));
	    memcpy(lp, lpBiffStr, sizeof(BIFFSTR)-sizeof(BYTE)+lLen);
      //End BOC---------------------------------------------------------
    }

    if (ghBiffStr)
    {
      tbGlobalUnlock(ghBiffStr);
      tbGlobalFree(ghBiffStr);
      ghBiffStr=0;
    }

    if (ghData)
    {
      tbGlobalUnlock(ghData);
      tbGlobalFree(ghData);
      ghData=0;
    }
    tbGlobalUnlock(*lpghSST);
  }
  else
    lIndex = (long)-1;
  
  return lIndex;
}

/***********************************************************************
* Name:   ssm_FixUpSST - Check to see if the SST buffer it too large. If 
*                        it is, add CONTINUE records until the size limit
*                        is satisfied. The CONTINUE records are added
*                        inplace in the SST buffer.
*
* Usage:  long ssm_FixUpSST(LPTBGLOBALHANDLE lpghSST, LPLONG lplSSTLen)
*           lpghSST - SST buffer.
*           lplSSTLen - length of the SST buffer.
*
* Return: long - The count of strings in the SST, or -1 if there is an error.
***********************************************************************/
long ssm_FixUpSST(LPTBGLOBALHANDLE lpghSST, LPLONG lplSSTLen)
{
  if (!*lplSSTLen)
    return S_OK;

  if (*lplSSTLen > EXCEL_MAXBIFF8RECLEN)
  {
    TBGLOBALHANDLE gh = (TBGLOBALHANDLE)0;
    LPBIFF lpBiff = NULL;
    long   lAllocBufferSize = 0;
    long   lBufferSize = sizeof(BIFF);
    long   lReadSize = sizeof(BIFF);
    long   lRecordSize = 0;
    long   lStringLen = 0;
    LPBYTE lpDest = NULL;
    LPBYTE lp = (LPBYTE)tbGlobalLock(*lpghSST);
    LPBIFFSTR lpbfstr = NULL;
    int    biffHdrSize = sizeof(BIFFSTR)-sizeof(BYTE);
    long   lNumOfRecsNeeded = *lplSSTLen/EXCEL_MAXBIFF8RECLEN;
    BYTE   bHighByte = 0;
    short  ngrbitCount = 0;
    bool   fContainsBiffHeader = TRUE;
    bool   fTrue = TRUE;
    short  extraBufferSize = 500; //21540

    if (*lplSSTLen%EXCEL_MAXBIFF8RECLEN)    
      lNumOfRecsNeeded++;

    // The SST record already exists, so that makes one less recorded that
    // needs to be added.
    lNumOfRecsNeeded--;  
    lAllocBufferSize = *lplSSTLen + lNumOfRecsNeeded*sizeof(BIFF) + extraBufferSize;// add some fluff. This is adjusted after 
                                                                                    // the entire SST is processed 21540
    Alloc(&gh, lAllocBufferSize); 
    lpDest = (LPBYTE)tbGlobalLock(gh);
    lpBiff = (LPBIFF)lpDest;
    lRecordSize = sizeof(SST);
    memcpy(lpDest, lp, sizeof(BIFF) + sizeof(SST));
    lp += sizeof(BIFF) + sizeof(SST);
    lpDest += sizeof(BIFF) + sizeof(SST);
    lpbfstr = (LPBIFFSTR)lp;
    bHighByte = (BYTE)(lpbfstr->fHighByte?1:0);
    lStringLen = biffHdrSize + lpbfstr->wCharCount*(bHighByte?2:1);
    do
    {
      while (( (lReadSize+lRecordSize+lStringLen) <= *lplSSTLen+ngrbitCount) && 
             (lRecordSize+lStringLen < EXCEL_MAXBIFF8RECLEN))
      {
        memcpy(lpDest, lpbfstr, lStringLen);
        lRecordSize += lStringLen;
        if (lReadSize+lRecordSize < *lplSSTLen+ngrbitCount)
        {
          lp += lStringLen;
          lpDest += lStringLen;
          lpbfstr = (LPBIFFSTR)lp;
          bHighByte = (BYTE)(lpbfstr->fHighByte?1:0);
          lStringLen = biffHdrSize + lpbfstr->wCharCount*(bHighByte?2:1);
          fContainsBiffHeader = TRUE;
        }
      }

      if (lStringLen > EXCEL_MAXBIFF8RECLEN)
      {
        long lRemainingLen = lStringLen;
        long lBytesToWrite = 0;
        
        while (lRemainingLen > 0)
        {
          lBytesToWrite = min(lRemainingLen, EXCEL_MAXBIFF8RECLEN);    

          if (lBytesToWrite + lRecordSize > EXCEL_MAXBIFF8RECLEN)            
          {
            lBytesToWrite = EXCEL_MAXBIFF8RECLEN-lRecordSize;
            if (bHighByte)
            {
              // Make sure we do not write just one byte of a 2 byte character.
              // This causes Excel file load problems.
              if (fContainsBiffHeader)
              {
                if ((lBytesToWrite-3/*BiffHdr*/)%2)
                  lBytesToWrite--;
              }
              else
              {
                if (lBytesToWrite%2) 
                  lBytesToWrite--;  
              }        
            }
          }
          memcpy(lpDest, lp, lBytesToWrite);
          lRecordSize += lBytesToWrite;
          lp += lBytesToWrite;
          lpDest += lBytesToWrite;
          
          lpBiff->datalen = (WORD)lRecordSize;
          lBufferSize += lRecordSize;
          lReadSize += lRecordSize;
          lRecordSize = 0;

          if (lReadSize >= *lplSSTLen)
            break;

          lpBiff = (LPBIFF)lpDest;
          lpDest += sizeof(BIFF);

          lRemainingLen -= lBytesToWrite;

          if (lRemainingLen > 0)
          {
            // Add a CONTINUE Record
            lpBiff->recnum = xlCONTINUE;
            lBufferSize += sizeof(BIFF);
            fContainsBiffHeader = FALSE;

            // add the grbit.
            memcpy(lpDest, &bHighByte, sizeof(BYTE));
            lpDest += sizeof(BYTE);
            lRecordSize += sizeof(BYTE);

            ngrbitCount++;
          }
        }
        if (lReadSize >= *lplSSTLen)
          break;

        lpbfstr = (LPBIFFSTR)lp;
        lStringLen = biffHdrSize + lpbfstr->wCharCount*(lpbfstr->fHighByte?2:1);
        
        if (lReadSize < *lplSSTLen)
        {
          // Add a CONTINUE Record
          lpBiff->recnum = xlCONTINUE;
          lBufferSize += sizeof(BIFF);
          fContainsBiffHeader = FALSE; 
        }
        
        continue;
      }
      else if ((lReadSize+lRecordSize+lStringLen) == *lplSSTLen && 
               (lReadSize+lRecordSize+lStringLen)<EXCEL_MAXBIFF8RECLEN)
      {
        // Special case if the last string fits exactly.  This will only be the case with the
        // very last string.
        memcpy(lpDest, lpbfstr, lStringLen);
        lRecordSize += lStringLen;
      }

      lpBiff->datalen = (WORD)lRecordSize;
      lBufferSize += lRecordSize;
      lReadSize += lRecordSize;
      lRecordSize = 0;

      if (lReadSize >= *lplSSTLen)
        break;

      lpBiff = (LPBIFF)lpDest;
      lpDest += sizeof(BIFF);
      
      // Add a CONTINUE Record
      lpBiff->recnum = xlCONTINUE;
      lBufferSize += sizeof(BIFF);

      fContainsBiffHeader = FALSE;

      if (lBufferSize + EXCEL_MAXBIFF8RECLEN > lAllocBufferSize)
      {
        tbGlobalUnlock(gh);
        lAllocBufferSize += EXCEL_MAXBIFF8RECLEN;
        Alloc(&gh, lAllocBufferSize); 
        lpDest = (LPBYTE)tbGlobalLock(gh);
        lpDest += lBufferSize;
        lpBiff = (LPBIFF)(lpDest - sizeof(BIFF));
      }

    } while (fTrue);

    tbGlobalUnlock(*lpghSST);
    tbGlobalFree(*lpghSST);
    *lpghSST=0;

    tbGlobalUnlock(gh);
    gh = tbGlobalReAlloc(gh, lBufferSize, GHND);  //readjust buffer to correct alloc size
        
    *lpghSST = gh;
    *lplSSTLen = lBufferSize;
  }
  return S_OK;
}

/***********************************************************************
* Name:   ssm_ApplyCelltype - 
*
* Usage:  long ssm_ApplyCelltype(HWND hWnd, SS_COORD lCol, SS_COORD lRow)
*
*           hWnd - window handle of the Spread control.
*           lCol - column to retrieve the cell string.
*           lRow - row to retrieve the cell string.
*
* Return: long - 0, or -1 if there is an error.
***********************************************************************/
long ssm_ApplyCelltype(LPSPREADSHEET lpSS, SS_COORD lCol, SS_COORD lRow, BOOL fIsFormula, LPVOID lp, BOOL fFormat, BOOL fData, double *lpdblRet, LPTBGLOBALHANDLE lpghRet, LPAPPLYCELLTYPE lpAC, LPTBGLOBALHANDLE lpghStrings)
{
  BIFF         biff;
  xXF8         xf;
  SS_CELLTYPE  ssCellType;
  long         lFormatIndex = -1;

  *lpghRet = 0;
  // Retrieve the celltype info from Spread and build the appropriate 
  // FORMAT and XF records.  Then add the cell record to the cell-record
  // buffer.
  SSGetCellType(lpSS->lpBook->hWnd, lCol, lRow, &ssCellType);

  if (fFormat)
  {
    int nAlign = ssCellType.Style;

    memset(&xf, 0, sizeof(xXF8));

    switch (ssCellType.Type)
    {
      case SS_TYPE_COMBOBOX:
        if (nAlign & SSS_ALIGN_LEFT)
          xf.fAlign = 1;
        else if (nAlign & SSS_ALIGN_RIGHT)
          xf.fAlign = 3;
        else if (nAlign & SSS_ALIGN_CENTER)
          xf.fAlign = 2;
      break;
      
      case SS_TYPE_INTEGER:
      case SS_TYPE_FLOAT:
#ifdef SS_V40
      case SS_TYPE_NUMBER:
      case SS_TYPE_CURRENCY:
      case SS_TYPE_PERCENT:
#endif
#ifdef SS_V70
      case SS_TYPE_SCIENTIFIC:
#endif
        if (nAlign & ES_CENTER)
          xf.fAlign = 2;
        else if (nAlign & ES_RIGHT)
          xf.fAlign = 3;
        else //Align LEFT! 
          xf.fAlign = 1;
        break;

      default:
        if (nAlign & ES_CENTER)
          xf.fAlign = 2;
        else if (nAlign & ES_RIGHT)
          xf.fAlign = 3;
        else //if (nAlign & ES_LEFT) or default
          xf.fAlign = 1;
      break;
    
      
    }

    if (nAlign & SSS_ALIGN_VCENTER)
      xf.fVAlign = 1;
    else if (nAlign & SSS_ALIGN_BOTTOM)
      xf.fVAlign = 2;

  
#ifdef SS_V40
    if ((ssCellType.Style & SSS_TEXTORIENT_MASK) == SSS_TEXTORIENT_VERT_LTR)
      xf.trot = 255;
    else if ((ssCellType.Style & SSS_TEXTORIENT_MASK) == SSS_TEXTORIENT_DOWN)
    { 
//    WORD wVAlign = xf.fVAlign;
        xf.trot = 180;
//    if (xf.fAlign == 1) //rotate down: change left align to top align
//      xf.fVAlign = 0;
//    else if (xf.fAlign == 3) //rotate down: change right align to bottom align
//      xf.fVAlign = 2;

//    if (wVAlign == 0) //rotate down: change top align to left align
//      xf.fAlign = 1;
//    else if (wVAlign == 2)  //rotate down: change bottom align to right align
//      xf.fAlign = 2;
    }
    else if ((ssCellType.Style & SSS_TEXTORIENT_MASK) == SSS_TEXTORIENT_UP)
    {
//    WORD wVAlign = xf.fVAlign;
      xf.trot = 90;
//    if (xf.fAlign == 1) //rotate up: change left align to bottom align
//      xf.fVAlign = 2;
//    else if (xf.fAlign == 3) //rotate up: change right align to top align
//      xf.fVAlign = 0;
//    else if (xf.fAlign ==2)
//      xf.fVAlign = 1;

//    if (wVAlign == 0) //rotate up: change top align to right align
//      xf.fAlign = 3;
//    else if (wVAlign == 2)  //rotate up: change bottom align to left align
//      xf.fAlign = 1;
    }
//  else if ((ssCellType.Style & SSS_TEXTORIENT_MASK) == SSS_TEXTORIENT_INVERT)
//    xf.trot = 
    else if ((ssCellType.Style & SSS_TEXTORIENT_MASK) == SSS_TEXTORIENT_VERT_RTL)
      xf.trot = 255;
    else
      xf.trot = 0;

#endif
  
  } // fFormat?

  switch (ssCellType.Type)
  {
    case SS_TYPE_DATE:
    {
      if (fFormat)
      {
        switch(ssCellType.Spec.Date.Format.nFormat)
        {          
          case IDF_DDMONYY:
          {
            TCHAR buffer[40];
      			//Add By BOC 99.5.5 (hyt) for support DBCS
#ifdef SPREAD_JPN
			      //  the DBCS separator is being used.
			      if (ssCellType.Spec.Date.Format.cSeparator == KANJISEPARATOR)
			      {
				      if (ssCellType.Spec.Date.Format.bCentury)
					      _stprintf(buffer, _T("dd%smmm%syyyy%s"),JSEPARATOR_DAY,JSEPARATOR_MONTH,JSEPARATOR_YEAR);
				      else
					      _stprintf(buffer, _T("dd%smmm%syy%s"),JSEPARATOR_DAY,JSEPARATOR_MONTH,JSEPARATOR_YEAR);
			      }
			      else
#endif	
      			{
			      	if (ssCellType.Spec.Date.Format.bCentury)
    		  			_stprintf(buffer, _T("dd\\%cmmm\\%cyyyy"), ssCellType.Spec.Date.Format.cSeparator, ssCellType.Spec.Date.Format.cSeparator);
		    	  	else
      					_stprintf(buffer, _T("dd\\%cmmm\\%cyy"), ssCellType.Spec.Date.Format.cSeparator, ssCellType.Spec.Date.Format.cSeparator);
			      }
            biff.recnum = xlFORMAT;
            biff.datalen = 0;
            lFormatIndex = ssm_BuildFormat(lpAC->lpnFormatCount, lpAC->lpghFormat, (LPBIFF)&biff, lp, lpAC->lplFormatLen, buffer, FALSE);
            xf.wifmt = (WORD)(0xA4 + lFormatIndex);
          }
          break;
          case IDF_DDMMYY:
          {
            TCHAR buffer[40];
			//Add By BOC 99.5.5 (hyt) for support DBCS
#ifdef SPREAD_JPN
			      //  the DBCS separator is being used.
			      if (ssCellType.Spec.Date.Format.cSeparator == KANJISEPARATOR)
			      {
				      if (ssCellType.Spec.Date.Format.bCentury)
					      _stprintf(buffer, _T("dd%smm%syyyy%s"),JSEPARATOR_DAY,JSEPARATOR_MONTH,JSEPARATOR_YEAR);
				      else
					      _stprintf(buffer, _T("dd%smm%syy%s"),JSEPARATOR_DAY,JSEPARATOR_MONTH,JSEPARATOR_YEAR);
			      }
			      else
#endif	
			      {
				      if (ssCellType.Spec.Date.Format.bCentury)
					      _stprintf(buffer, _T("dd\\%cmm\\%cyyyy"), ssCellType.Spec.Date.Format.cSeparator, ssCellType.Spec.Date.Format.cSeparator);
				      else
					      _stprintf(buffer, _T("dd\\%cmm\\%cyy"), ssCellType.Spec.Date.Format.cSeparator, ssCellType.Spec.Date.Format.cSeparator);
			      }
            biff.recnum = xlFORMAT;
            biff.datalen = 0;
            lFormatIndex = ssm_BuildFormat(lpAC->lpnFormatCount, lpAC->lpghFormat, (LPBIFF)&biff, lp, lpAC->lplFormatLen, buffer, FALSE);
            xf.wifmt = (WORD)(0xA4 + lFormatIndex);
          }
          break;
          case IDF_MMDDYY:
          {
            TCHAR buffer[40];
      			//Add By BOC 99.5.5 (hyt) for support DBCS
#ifdef SPREAD_JPN
			      //  the DBCS separator is being used.
			      if (ssCellType.Spec.Date.Format.cSeparator == KANJISEPARATOR)
			      {
				      if (ssCellType.Spec.Date.Format.bCentury)
					      _stprintf(buffer, _T("mm%sdd%syyyy%s"),JSEPARATOR_MONTH,JSEPARATOR_DAY,JSEPARATOR_YEAR);
				      else
					      _stprintf(buffer, _T("mm%sdd%syy%s"),JSEPARATOR_MONTH,JSEPARATOR_DAY,JSEPARATOR_YEAR);
			      }
			      else
#endif	
			      {
				      if (ssCellType.Spec.Date.Format.bCentury)
					      _stprintf(buffer, _T("mm\\%cdd\\%cyyyy"), ssCellType.Spec.Date.Format.cSeparator, ssCellType.Spec.Date.Format.cSeparator);
				      else
					      _stprintf(buffer, _T("mm\\%cdd\\%cyy"), ssCellType.Spec.Date.Format.cSeparator, ssCellType.Spec.Date.Format.cSeparator);
			      }
            biff.recnum = xlFORMAT;
            biff.datalen = 0;
            //if (ssCellType.Spec.Date.Format.bCentury)
            //  _tcscat(buffer, _T("yy"));
            lFormatIndex = ssm_BuildFormat(lpAC->lpnFormatCount, lpAC->lpghFormat, (LPBIFF)&biff, lp, lpAC->lplFormatLen, buffer, FALSE);
            xf.wifmt = (WORD)(0xA4 + lFormatIndex);
          }
          break;
          case IDF_YYMMDD:
          {
            TCHAR buffer[40];
      			//Add By BOC 99.5.5 (hyt) for support DBCS
#ifdef SPREAD_JPN
			      //  the DBCS separator is being used.
			      if (ssCellType.Spec.Date.Format.cSeparator == KANJISEPARATOR)
				      _stprintf(buffer, _T("yy%smm%sdd%s"),JSEPARATOR_YEAR,JSEPARATOR_MONTH,JSEPARATOR_DAY);
			      else
#endif	
    				_stprintf(buffer, _T("yy\\%cmm\\%cdd"), ssCellType.Spec.Date.Format.cSeparator, ssCellType.Spec.Date.Format.cSeparator);

            biff.recnum = xlFORMAT;
            biff.datalen = 0;
            if (ssCellType.Spec.Date.Format.bCentury)
            {
              _tcscat(_tcsrev(buffer), _T("yy"));
              _tcsrev(buffer);
           }
            lFormatIndex = ssm_BuildFormat(lpAC->lpnFormatCount, lpAC->lpghFormat, (LPBIFF)&biff, lp, lpAC->lplFormatLen, buffer, FALSE);
            xf.wifmt = (WORD)(0xA4 + lFormatIndex);
          }
          break;
		      //Add by BOC 99.5.5 (hyt)-----------------------------
		      //for support japanese version date format
		      case IDF_YYMM:
		      {
            TCHAR buffer[40];

#ifdef	SPREAD_JPN
			      //  the DBCS separator is being used.
			      if (ssCellType.Spec.Date.Format.cSeparator == KANJISEPARATOR)
				      _stprintf(buffer, _T("yy%smm%s"),JSEPARATOR_YEAR,JSEPARATOR_MONTH);
			      else
#endif
    				_stprintf(buffer, _T("yy\\%cmm"), ssCellType.Spec.Date.Format.cSeparator);

            biff.recnum = xlFORMAT;
            biff.datalen = 0;
            if (ssCellType.Spec.Date.Format.bCentury)
            {
              _tcscat(_tcsrev(buffer), _T("yy"));
              _tcsrev(buffer);
            }
            lFormatIndex = ssm_BuildFormat(lpAC->lpnFormatCount, lpAC->lpghFormat, (LPBIFF)&biff, lp, lpAC->lplFormatLen, buffer, FALSE);
            xf.wifmt = (WORD)(0xA4 + lFormatIndex);
          }
		      break;
		      case IDF_MMDD:
		      {
            TCHAR buffer[40];

#ifdef	SPREAD_JPN
			      //  the DBCS separator is being used.
			      if (ssCellType.Spec.Date.Format.cSeparator == KANJISEPARATOR)
				      _stprintf(buffer, _T("mm%sdd%s"),JSEPARATOR_MONTH,JSEPARATOR_DAY);
			      else
#endif
    				_stprintf(buffer, _T("mm\\%cdd"), ssCellType.Spec.Date.Format.cSeparator);

            biff.recnum = xlFORMAT;
            biff.datalen = 0;
            lFormatIndex = ssm_BuildFormat(lpAC->lpnFormatCount, lpAC->lpghFormat, (LPBIFF)&biff, lp, lpAC->lplFormatLen, buffer, FALSE);
            xf.wifmt = (WORD)(0xA4 + lFormatIndex);
          }
    		  break;
#ifdef	SPREAD_JPN
		      case IDF_NYYMMDD:
		      case IDF_NNYYMMDD:
		      case IDF_NNNNYYMMDD:
          {
            TCHAR buffer[40];
  			    //  the DBCS separator is being used.
	  		    if (ssCellType.Spec.Date.Format.cSeparator == KANJISEPARATOR)
		  		    _stprintf(buffer, _T("gee%smm%sdd%s"),JSEPARATOR_YEAR,JSEPARATOR_MONTH,JSEPARATOR_DAY);
			      else
				      _stprintf(buffer, _T("gee\\%cmm\\%cdd"), ssCellType.Spec.Date.Format.cSeparator, ssCellType.Spec.Date.Format.cSeparator);

            biff.recnum = xlFORMAT;
            biff.datalen = 0;
  			    if (ssCellType.Spec.Date.Format.nFormat == IDF_NNYYMMDD)
            {
              _tcscat(_tcsrev(buffer), _T("g"));
              _tcsrev(buffer);
            }
	  		    else if(ssCellType.Spec.Date.Format.nFormat == IDF_NNNNYYMMDD)
		  	    {
			  	    _tcscat(_tcsrev(buffer), _T("ggg"));
				      _tcsrev(buffer);
			      }
            lFormatIndex = ssm_BuildFormat(lpAC->lpnFormatCount, lpAC->lpghFormat, (LPBIFF)&biff, lp, lpAC->lplFormatLen, buffer, FALSE);
            xf.wifmt = (WORD)(0xA4 + lFormatIndex);
          }
		      break;
#endif
    		  //----------------------------------------------------
        }
    
        //Add the XF record for this cell
        biff.recnum = xlXF;
        biff.datalen = 20;
        ssm_BuildXF(lCol, lRow, 1, lpAC->lpnXFIndex, lpAC->lpnXFCount, lpAC->lpghXF, &biff, lp, lpAC->lplXFLen, &xf, lpAC->lpghFont, lpAC->lplFontLen);
      } // fFormat?

      if (fData)
      {
        NUMBER       Number;
        long         lStartDate;
        SS_DATE      StartDate;
        long         lDataLen;
        TBGLOBALHANDLE ghData;
        LPBYTE       lpData;
        SS_DATE      Date;

        memset(&Number, 0, sizeof(NUMBER));
        lDataLen = SSGetDataLen(lpSS->lpBook->hWnd, lCol, lRow);
        ghData = tbGlobalAlloc(GHND, (lDataLen+1)*sizeof(TCHAR));
        lpData = (LPBYTE)tbGlobalLock(ghData);
        SSGetData(lpSS->lpBook->hWnd, lCol, lRow, (LPTSTR)lpData);
        if (lstrlen((LPTSTR)lpData))
        {
          DateStringToDMYEx((LPCTSTR)lpData, &Date, &ssCellType.Spec.Date.Format, lpSS->lpBook->nTwoDigitYearMax);
          if (Date.nYear < 1900)
          {
            TCHAR buffer[100];
            _stprintf(buffer, _T("day:%02d month:%02d year:%04d"), Date.nDay, Date.nMonth, Date.nYear);
            xl_LogFile(lpSS->lpBook, LOG_DATEOUTOFRANGE, lCol, lRow, buffer);
//            SSSetValue(lpSS->lpBook->hWnd, lCol, lRow, (LPCTSTR)buffer);
            tbGlobalUnlock(ghData);
            tbGlobalFree(ghData);
            ghData = 0;

            break;
          }          

          StartDate.nYear = 1900;
          StartDate.nMonth = 1;
          StartDate.nDay = 1; 
          lStartDate = DateDMYToJulian(&StartDate);
          Number.num = (double)DateDMYToJulian(&Date);
          Number.num = Number.num - (double)lStartDate;
          if ((Date.nYear != 1900) || (Date.nYear == 1900 && Date.nMonth > 2))
            Number.num += 2;
          else
            Number.num++;

          tbGlobalUnlock(ghData);
          tbGlobalFree(ghData);
          ghData=0;
        }
        if (!fIsFormula)
        {  
          if (Number.num != 0)
          {
            biff.recnum = xlNUMBER;
            biff.datalen = 14;
            Number.wRow = (WORD)(lRow-1);
            Number.wCol = (WORD)(lCol-1);
            Number.wixfe = *(lpAC->lpnXFIndex);
            //write cell records to Cells
            ssm_WriteBuffer(lpAC->lpghCells, &biff, lp, lpAC->lplCellsLen, &Number);
          }
          else
          {
            //write out a BLANK cell record
            BLANK blank;
            memset(&blank, 0, sizeof(BLANK));
            biff.recnum = xlBLANK;
            biff.datalen = 6;
            blank.wRow = (WORD)(lRow-1);
            blank.wCol = (WORD)(lCol-1);
            blank.wixfe = *(lpAC->lpnXFIndex);
            //write cell records to Cells
            ssm_WriteBuffer(lpAC->lpghCells, &biff, lp, lpAC->lplCellsLen, &blank);
          }
        }
        else
        {
          *lpdblRet = Number.num;
        }
      } //fData?
    }
    break;

    case SS_TYPE_TIME:
    {
      TCHAR        buffer[20];

      if (fFormat)
      {
		//Add By BOC 99.5.5 (hyt) for support DBCS
#ifdef SPREAD_JPN
		//  the DBCS separator is being used.
		if (ssCellType.Spec.Time.Format.cSeparator == KANJISEPARATOR)
			_stprintf(buffer, _T("hh%smm%s"),JSEPARATOR_HOUR,JSEPARATOR_MINUTE);
		else
#endif	
        _stprintf(buffer, _T("hh\\%cmm"), ssCellType.Spec.Time.Format.cSeparator, ssCellType.Spec.Time.Format.cSeparator);
        if (ssCellType.Spec.Time.Format.bSeconds)
    		{
#ifdef SPREAD_JPN
			    //  the DBCS separator is being used.
			    if (ssCellType.Spec.Time.Format.cSeparator == KANJISEPARATOR)
				    _stprintf(buffer, _T("%sss%s"), buffer,JSEPARATOR_SECOND);
			    else
#endif	
		  		_stprintf(buffer, _T("%s\\%css"), buffer, ssCellType.Spec.Time.Format.cSeparator);
    		}
		//----------------------------------------------------------------------
#ifdef SPREAD_JPN

        switch (ssCellType.Spec.Time.Format.b24Hour)
        {
          case 0:
            _tcscat(buffer, _T("\\ AM/PM"));
            break;
          case 1:
            break;
          case 2:
            _tcscat(buffer, _T("\\ AM/PM"));
            break;
          case 3:
            {
            TCHAR tempBuffer[20];
            _stprintf(tempBuffer, _T("\\ AM/PM %s"), buffer);
            _tcscpy(buffer, tempBuffer);
            }
            break;
        }
#else
        if (!ssCellType.Spec.Time.Format.b24Hour)
          _tcscat(buffer, _T("\\ AM/PM"));
#endif
        biff.recnum = xlFORMAT;
        biff.datalen = 0;
        lFormatIndex = ssm_BuildFormat(lpAC->lpnFormatCount, lpAC->lpghFormat, (LPBIFF)&biff, lp, lpAC->lplFormatLen, buffer, FALSE);
        xf.wifmt = (WORD)(0xA4 + lFormatIndex);

        //Add the XF record for this cell
        biff.recnum = xlXF;
        biff.datalen = 20;
        ssm_BuildXF(lCol, lRow, 1, lpAC->lpnXFIndex, lpAC->lpnXFCount, lpAC->lpghXF, &biff, lp, lpAC->lplXFLen, &xf, lpAC->lpghFont, lpAC->lplFontLen);
      } // fFormat?

      if (fData)
      {
        NUMBER       Number;
        long         lDataLen;
        TBGLOBALHANDLE ghData;  
        LPBYTE       lpData;
        SS_TIME      Time;

        memset(&Number, 0, sizeof(NUMBER));
        lDataLen = SSGetDataLen(lpSS->lpBook->hWnd, lCol, lRow);
        if (lDataLen)
        {
          ghData = tbGlobalAlloc(GHND, (lDataLen+1)*sizeof(TCHAR));
          lpData = (LPBYTE)tbGlobalLock(ghData);
          SSGetData(lpSS->lpBook->hWnd, lCol, lRow, (LPTSTR)lpData);
          TimeStringToHMS((LPCTSTR)lpData, &Time, (LPTIMEFORMAT)&ssCellType.Spec.Time.Format);
          Number.num = (double)Time.nHour*3600 + (double)Time.nMinute*60 + (double)Time.nSecond;
          Number.num = Number.num / ((double)3600.0 * (double)24.0);
          tbGlobalUnlock(ghData);
          tbGlobalFree(ghData);
          ghData=0;
          biff.recnum = xlNUMBER;
          biff.datalen = 14;
          Number.wRow = (WORD)(lRow-1);
          Number.wCol = (WORD)(lCol-1);
          Number.wixfe = *(lpAC->lpnXFIndex);
          //write cell records to Cells
          if (fIsFormula)
            *lpdblRet = Number.num;
          else
            ssm_WriteBuffer(lpAC->lpghCells, &biff, lp, lpAC->lplCellsLen, &Number);
        }
        else
        {
          if (fIsFormula)
            *lpdblRet = 0.0;
          else
          {
            //write out a BLANK cell record
            BLANK blank;
            memset(&blank, 0, sizeof(BLANK));
            biff.recnum = xlBLANK;
            biff.datalen = 6;
            blank.wRow = (WORD)(lRow-1);
            blank.wCol = (WORD)(lCol-1);
            blank.wixfe = *(lpAC->lpnXFIndex);
            //write cell records to Cells
            ssm_WriteBuffer(lpAC->lpghCells, &biff, lp, lpAC->lplCellsLen, &blank);
      
          }
        }
      } //fData?
    }
    break;
           
    case SS_TYPE_FLOAT:
    case SS_TYPE_INTEGER:
    {  
      TCHAR  buffer[100];
      BOOL   fGeneral = TRUE;

      SSGetFloat(lpSS->lpBook->hWnd, lCol, lRow, lpdblRet);

      if (fFormat)
      {
//Modify By BOC 99.6.11 (hyt)--------------------------------
//for if integer type if set spin will export incorrect format
    		if ((ssCellType.Type == SS_TYPE_FLOAT) && (ssCellType.Style & FS_MONEY))
        {
/*
          if (ssCellType.Spec.Float.Format.cCurrencySign)
            _stprintf(buffer, _T("%c#"), ssCellType.Spec.Float.Format.cCurrencySign); //RAP15a
          else
          {
            SS_FLOATFORMAT ff;
            memset(&ff, 0, sizeof(SS_FLOATFORMAT));
            SSGetDefFloatFormat(lpSS->lpBook->hWnd, &ff);
            _stprintf(buffer, _T("%c#"), ff.cCurrencySign); //RAP15a
          }
          fGeneral = FALSE;
*/
          if (ssCellType.Spec.Float.Format.cCurrencySign)
          {
            if (ssCellType.Spec.Float.Format.cCurrencySign == '€')
            {
              //this is a euro
              _stprintf(buffer, _T("[$%s-2]#"), ssCellType.Spec.Float.Format.cCurrencySign);
            }
            else if (ssCellType.Spec.Float.Format.cCurrencySign == '\\') 
            {
              //this is yen
			  		  _tcscpy(buffer, _T("\"\\\""));
              _tcscat(buffer, _T("#"));
            }  
            else
              _stprintf(buffer, _T("[$%c]#"), ssCellType.Spec.Float.Format.cCurrencySign);
					}
          else
          {
            SS_FLOATFORMAT ff;
            memset(&ff, 0, sizeof(SS_FLOATFORMAT));
            SSGetDefFloatFormat(lpSS->lpBook->hWnd, &ff);
//            _stprintf(buffer, _T("%c#"), ff.cCurrencySign); //RAP15a
            if (ff.cCurrencySign == '€')
            {
              //this is a euro
              _stprintf(buffer, _T("[$%s-2]#"), ff.cCurrencySign);
            }
            else if (ff.cCurrencySign == '\\') 
            {
              //this is yen
			  		  _tcscpy(buffer, _T("\"\\\""));
              _tcscat(buffer, _T("#"));
            }  
            else
              _stprintf(buffer, _T("[$%c]#"), ff.cCurrencySign);
          }
          fGeneral = FALSE;
        }
        else
          lstrcpy(buffer, _T("#"));
       	if((ssCellType.Type == SS_TYPE_FLOAT) && (ssCellType.Style & FS_SEPARATOR))
//End BOC--------------------------------------------------------------------------------------
        {
//RAP02d          _tcscat(buffer, _T(",###"));
          _tcscat(buffer, _T(",##0"));
          fGeneral = FALSE;
        } 
        else
//RAP02d          _tcscat(buffer, _T("###"));
          _tcscat(buffer, _T("##0"));
  
        if ((ssCellType.Type == SS_TYPE_FLOAT) && ssCellType.Spec.Float.Right)
        {
          int i;
          _tcscat(buffer, _T("."));
          for (i=0; i<ssCellType.Spec.Float.Right; i++)
            _tcscat(buffer, _T("0"));
        }

        if (!_tcscmp(buffer, _T("###0")))
        {
          xf.wifmt = 0x01;
        }
        else
        {
          biff.recnum = xlFORMAT;
          biff.datalen = 0;
          lFormatIndex = ssm_BuildFormat(lpAC->lpnFormatCount, lpAC->lpghFormat, (LPBIFF)&biff, lp, lpAC->lplFormatLen, buffer, FALSE);
          xf.wifmt = (WORD)(0xA4 + lFormatIndex);
        }
             
        //Add the XF record for this cell
        biff.recnum = xlXF;
        biff.datalen = 20;
        ssm_BuildXF(lCol, lRow, 1, lpAC->lpnXFIndex, lpAC->lpnXFCount, lpAC->lpghXF, &biff, lp, lpAC->lplXFLen, &xf, lpAC->lpghFont, lpAC->lplFontLen);
        if (fIsFormula)
          break;
      } // fFormat?

      if (fData)
      {  
        NUMBER Number;
        if (SSGetFloat(lpSS->lpBook->hWnd, lCol, lRow, (LPDOUBLE)&Number.num))
        {
          biff.recnum = xlNUMBER;
          biff.datalen = 14;
          Number.wRow = (WORD)(lRow-1);
          Number.wCol = (WORD)(lCol-1);
          Number.wixfe = *(lpAC->lpnXFIndex);
          //write cell records to Cells
          if (fIsFormula)
            *lpdblRet = Number.num;
          else
            ssm_WriteBuffer(lpAC->lpghCells, &biff, lp, lpAC->lplCellsLen, &Number);
        } 
        else
        {
          if (fIsFormula)
            *lpdblRet = 0.0;
          else
          {
            //write out a BLANK cell record
            BLANK blank;
            memset(&blank, 0, sizeof(BLANK));
            biff.recnum = xlBLANK;
            biff.datalen = 6;
            blank.wRow = (WORD)(lRow-1);
            blank.wCol = (WORD)(lCol-1);
            blank.wixfe = *(lpAC->lpnXFIndex);
            //write cell records to Cells
            ssm_WriteBuffer(lpAC->lpghCells, &biff, lp, lpAC->lplCellsLen, &blank);
          }
        }
      } //fData?
    }
    break;

#ifdef SS_V70
    case SS_TYPE_SCIENTIFIC:
    {
      TCHAR buffer[100];
      TCHAR customBuffer[100];

      memset(buffer, 0, sizeof(TCHAR)*100);
      memset(customBuffer, 0, sizeof(TCHAR)*100);

      if (fFormat)
      {
        buffer[0] = '0';
    		if (ssCellType.Spec.Scientific.Right)
        {
          int i;
          buffer[1] = '.';

          for (i=0; i<ssCellType.Spec.Scientific.Right; i++)
            _tcscat(buffer, _T("0"));
        } 

        xf.wifmt = (WORD)-1;
        if (!_tcscmp(buffer, _T("0.0")))
        {
          if (ssCellType.Style & SSS_NEGRED)
            _stprintf(customBuffer, _T("%sE+00;[Red]-%sE+00"), buffer, buffer);
          else
            _stprintf(customBuffer, _T("%sE+00;-%sE+00"), buffer, buffer); //16132
//16132     xf.wifmt = 0x30;
        }
        else if (!_tcscmp(buffer, _T("0.00")))
        { 
          if (ssCellType.Style & SSS_NEGRED)
           _stprintf(customBuffer, _T("%sE+00;[Red]-%sE+00"), buffer, buffer);
          else
            xf.wifmt = 0x0B;
        }
        else
        {
          if (ssCellType.Style & SSS_NEGRED)
           _stprintf(customBuffer, _T("%sE+00;[Red]-%sE+00"), buffer, buffer);
          else
           _stprintf(customBuffer, _T("%sE+00"), buffer);
        }

        if (lstrlen(customBuffer) > 0)
        {
          biff.recnum = xlFORMAT;
          biff.datalen = 0;
          lFormatIndex = ssm_BuildFormat(lpAC->lpnFormatCount, lpAC->lpghFormat, (LPBIFF)&biff, lp, lpAC->lplFormatLen, customBuffer, FALSE);
           xf.wifmt = (WORD)(0xA4 + lFormatIndex);
        }

        //Add the XF record for this cell
        biff.recnum = xlXF;
        biff.datalen = 20;
        ssm_BuildXF(lCol, lRow, 1, lpAC->lpnXFIndex, lpAC->lpnXFCount, lpAC->lpghXF, &biff, lp, lpAC->lplXFLen, &xf, lpAC->lpghFont, lpAC->lplFontLen);
        if (fIsFormula)
          break;
      } // fFormat?

      if (fData)
      {  
        NUMBER Number;

        if (SSGetFloat(lpSS->lpBook->hWnd, lCol, lRow, (LPDOUBLE)&Number.num))
        {
          biff.recnum = xlNUMBER;
          biff.datalen = 14;
          Number.wRow = (WORD)(lRow-1);
          Number.wCol = (WORD)(lCol-1);
          Number.wixfe = *(lpAC->lpnXFIndex);
          //write cell records to Cells
          if (fIsFormula)
            *lpdblRet = Number.num;
          else
            ssm_WriteBuffer(lpAC->lpghCells, &biff, lp, lpAC->lplCellsLen, &Number);
        } 
        else
        {
          if (fIsFormula)
            *lpdblRet = 0.0;
          else
          {
            //write out a BLANK cell record
            BLANK blank;
            memset(&blank, 0, sizeof(BLANK));
            biff.recnum = xlBLANK;
            biff.datalen = 6;
            blank.wRow = (WORD)(lRow-1);
            blank.wCol = (WORD)(lCol-1);
            blank.wixfe = *(lpAC->lpnXFIndex);
            //write cell records to Cells
            ssm_WriteBuffer(lpAC->lpghCells, &biff, lp, lpAC->lplCellsLen, &blank);
          }
        }
      } //fData?
    }
#endif

#ifdef SS_V40
    case SS_TYPE_NUMBER:
    case SS_TYPE_CURRENCY:
    {
      TCHAR buffer[100];
      TCHAR bufferPos[100];
      TCHAR bufferNeg[100]; 
      TCHAR bufferTemp[100];
      BOOL  fGeneral = TRUE;
      
      // International format
      SSNUM_FORMAT nfSrc, nf;
      memset(&nfSrc, 0, sizeof(SSNUM_FORMAT));
      memset(&nf, 0, sizeof(SSNUM_FORMAT));

      memset(buffer, 0, 100*sizeof(TCHAR));
      memset(bufferNeg, 0, 100*sizeof(TCHAR));

      if (fFormat)
      {
        switch (ssCellType.Type)
        {
          case SS_TYPE_NUMBER:
            SS_CreateNumberFormatStruct(&ssCellType, &nfSrc);
            NumGetDisplayFormat(&nfSrc, &nf);

            lstrcpy(buffer, _T("#"));

    		    if(ssCellType.Spec.Number.fShowSeparator)
            {
              _tcscat(buffer, _T(",##"));
              fGeneral = FALSE;
            } 
            else
              _tcscat(buffer, _T("##"));

            switch (ssCellType.Spec.Number.bLeadingZero)
            {
              case 2:
                _tcscat(buffer, _T("0"));
                break;
              case 1:
                _tcscat(buffer, _T("#"));
                break;
              case 0: // international
                if (nf.bLeadingZero)
                  _tcscat(buffer, _T("0"));
                else
                  _tcscat(buffer, _T("#"));
                break;
            }

            if (ssCellType.Spec.Number.Right)
            {
              int i;
              _tcscat(buffer, _T("."));
              for (i=0; i<ssCellType.Spec.Number.Right; i++)
                _tcscat(buffer, _T("0"));
            }
            
            // Negative format
            if (ssCellType.Spec.Number.bNegStyle == 0)
              ssCellType.Spec.Number.bNegStyle = (BYTE)(nf.bNegStyle+1);

            if (ssCellType.Spec.Number.bNegStyle == 1)
            {
              if (ssCellType.Style & SSS_NEGRED)
                _stprintf(bufferNeg, _T(";[Red](%s)"), buffer);
              else
                _stprintf(bufferNeg, _T(";(%s)"), buffer);
            }
            else if (ssCellType.Spec.Number.bNegStyle == 2)
            {
              if (ssCellType.Style & SSS_NEGRED)
                _stprintf(bufferNeg, _T(";[Red]-%s"), buffer);
              else
                _stprintf(bufferNeg, _T(";-%s"), buffer);
            }
            else if (ssCellType.Spec.Number.bNegStyle == 3)
            {
              if (ssCellType.Style & SSS_NEGRED)
                _stprintf(bufferNeg, _T(";[Red]-_*%s"), buffer);
              else
                _stprintf(bufferNeg, _T(";-_*%s"), buffer);
            }
            else if (ssCellType.Spec.Number.bNegStyle == 4)
            {
              if (ssCellType.Style & SSS_NEGRED)
                _stprintf(bufferNeg, _T(";[Red]%s-"), buffer);
              else
                _stprintf(bufferNeg, _T(";%s-"), buffer);
            }
            else if (ssCellType.Spec.Number.bNegStyle == 5)
            {
              if (ssCellType.Style & SSS_NEGRED)
                _stprintf(bufferNeg, _T(";[Red]%s_*-"), buffer);
              else
                _stprintf(bufferNeg, _T(";%s_*-"), buffer);
            }
            else if (ssCellType.Style & SSS_NEGRED)
            {
              _stprintf(bufferNeg, _T(";[Red]%s"), buffer);
            }

            if (lstrlen(bufferNeg))
              _tcscat(buffer, bufferNeg);

            if (!_tcscmp(buffer, _T("###0")))
              xf.wifmt = 0x01;
            else if (!_tcscmp(buffer, _T("###0.00;-###0.00")))
              xf.wifmt = 0x02;
            else if (!_tcscmp(buffer, _T("#,##0;(#,##0)")))
              xf.wifmt = 0x25;
            else if (!_tcscmp(buffer, _T("#,##0;[Red](#,##0)")))
              xf.wifmt = 0x26;
            else if (!_tcscmp(buffer, _T("#,##0.00;(#,##0.00)")))
              xf.wifmt = 0x27;
            else if (!_tcscmp(buffer, _T("#,##0.00;[Red](#,##0)")))
              xf.wifmt = 0x28;

            else
            {
              biff.recnum = xlFORMAT;
              biff.datalen = 0;
              lFormatIndex = ssm_BuildFormat(lpAC->lpnFormatCount, lpAC->lpghFormat, (LPBIFF)&biff, lp, lpAC->lplFormatLen, buffer, FALSE);
              xf.wifmt = (WORD)(0xA4 + lFormatIndex);
            }
          break;

          case SS_TYPE_CURRENCY:
          {
            TCHAR szCurrency[5];
            BOOL  fUnicode = FALSE;
            SSNUM_FORMAT nf;

            SS_CreateCurrencyFormatStruct(&ssCellType, &nfSrc);
            NumGetDisplayFormat(&nfSrc, &nf);
     
            _tcscpy(szCurrency, nf.szCurrency);

            fGeneral = FALSE;

            if (ssCellType.Spec.Currency.bPosStyle == 0)
            {
/*
              memset(buffer, 0, 100*sizeof(TCHAR));

              if(nf.fShowSeparator)
              {
                _tcscat(buffer, _T("#,##"));
                fGeneral = FALSE;
              } 
              else
                _tcscat(buffer, _T("###"));

              switch (ssCellType.Spec.Currency.bLeadingZero)
              {
                case 2:
                  _tcscat(buffer, _T("0"));
                  break;
                case 1:
                  _tcscat(buffer, _T("#"));
                  break;
                case 0: // international
                  if (nf.bLeadingZero)
                    _tcscat(buffer, _T("0"));
                  else
                    _tcscat(buffer, _T("#"));
                  break;
              }

              if (nf.bFractionalWidth)
              {
                int i;
                _tcscat(buffer, _T("."));
                for (i=0; i<nf.bFractionalWidth; i++)
                  _tcscat(buffer, _T("0"));
              }
              if (nf.fShowCurrencySymbol)
              {
                if (lstrlen(nf.szCurrency))
                {
                  if (!_tcscmp(_T("€"), nf.szCurrency))
                  {
                    //this is a euro
                    _stprintf(szCurrency, _T("[$%s-2]"), nf.szCurrency);
                    fUnicode = TRUE;
                  }
                  else if (!_tcscmp(nf.szCurrency, _T("\\"))) 
                  {
                    //this is yen
			  		        _stprintf(szCurrency, _T("\"\\\""));
//			  		        _stprintf(szCurrency, _T("\"¥\""));
                  }  
                  else
                    _stprintf(szCurrency, _T("[$%s]"), nf.szCurrency);
					      }
              }  
*/
              ssCellType.Spec.Currency.bPosStyle = (BYTE)(nf.bPosCurrencyStyle+1);    
            }
//    		    else
            {
              if(ssCellType.Spec.Currency.fShowSeparator)
              {
                _tcscat(buffer, _T("#,##"));
                fGeneral = FALSE;
              } 
              else
                _tcscat(buffer, _T("###"));
  
              switch (ssCellType.Spec.Currency.bLeadingZero)
              {
                case 2:
                  _tcscat(buffer, _T("0"));
                  break;
                case 1:
                  _tcscat(buffer, _T("#"));
                  break;
                case 0: // international
                  if (nf.bLeadingZero)
                    _tcscat(buffer, _T("0"));
                  else
                    _tcscat(buffer, _T("#"));
                  break;
              }

              if (ssCellType.Spec.Currency.Right)
              {
                int i;
                _tcscat(buffer, _T("."));
                for (i=0; i<ssCellType.Spec.Currency.Right; i++)
                  _tcscat(buffer, _T("0"));
              }
      
              if (ssCellType.Spec.Currency.fShowCurrencySymbol)
              {
                if (lstrlen(ssCellType.Spec.Currency.szCurrency))
                {
                  if (!_tcscmp(_T("€"), ssCellType.Spec.Currency.szCurrency))
                  {
                    //this is a euro
                    _stprintf(szCurrency, _T("[$%s-2]"), ssCellType.Spec.Currency.szCurrency);
                    fUnicode = TRUE;
                  }
//RAP27a>>
                  else if (!_tcscmp(ssCellType.Spec.Currency.szCurrency, _T("\\")) || !_tcscmp(nf.szCurrency, _T("\\"))) 
                  {
                    //this is yen
			  		        _stprintf(szCurrency, _T("\"\\\""));
                  }  
//RAP27a <<
                  else
//                    _tcscpy(szCurrency, ssCellType.Spec.Currency.szCurrency);
                    _stprintf(szCurrency, _T("[$%s]"), ssCellType.Spec.Currency.szCurrency);
                }
                else
                {
                  SS_FLOATFORMAT ff;
                  memset(&ff, 0, sizeof(SS_FLOATFORMAT));
                  SSGetDefFloatFormat(lpSS->lpBook->hWnd, &ff);
                  if ((TCHAR)ff.cCurrencySign == (TCHAR)'\\') 
                  {
                    //this is yen
			  		        _stprintf(szCurrency, _T("\"\\\""));
                  }  
                }
              }  
            }
//14278 >>
            if (szCurrency[0] == '\\')
              _tcscpy(szCurrency, _T("\\\\"));
//14278 <<
            // Positive format
            if (ssCellType.Spec.Currency.bPosStyle == 0)
            {
              ssCellType.Spec.Currency.bPosStyle = (BYTE)(nf.bPosCurrencyStyle+1);
            }
            if (ssCellType.Spec.Currency.bPosStyle == 1)
            {
              if (ssCellType.Spec.Currency.fShowCurrencySymbol)
                _stprintf(bufferPos, _T("%s%s"), szCurrency, buffer);
              else
                _stprintf(bufferPos, _T("%s"), buffer);
            }
            else if (ssCellType.Spec.Currency.bPosStyle == 2)
            {
              if (ssCellType.Spec.Currency.fShowCurrencySymbol)
                _stprintf(bufferPos, _T("%s%s"), buffer, szCurrency);
              else
                _stprintf(bufferPos, _T("%s"), buffer);
            }
            else if (ssCellType.Spec.Currency.bPosStyle == 3)
            {
              if (ssCellType.Spec.Currency.fShowCurrencySymbol)
                _stprintf(bufferPos, _T("%s_*%s"), szCurrency, buffer);
              else
                _stprintf(bufferPos, _T("_*%s"), buffer);
            }
            else if (ssCellType.Spec.Currency.bPosStyle == 4)
            {
              if (ssCellType.Spec.Currency.fShowCurrencySymbol)
                _stprintf(bufferPos, _T("%s_*%s"), buffer, szCurrency);
              else
                _stprintf(bufferPos, _T("%s_*"), buffer);
            }
            
            // Negative format
            if (ssCellType.Spec.Currency.bNegStyle == 0)
            {
/*
              SSNUM_FORMAT nfSrc, nf;
              memset(&nfSrc, 0, sizeof(SSNUM_FORMAT));
              memset(&nf, 0, sizeof(SSNUM_FORMAT));
              SS_CreateCurrencyFormatStruct(&ssCellType, &nfSrc);
              NumGetDisplayFormat(&nfSrc, &nf);

              memset(buffer, 0, 100*sizeof(TCHAR));
    		      if(nf.fShowSeparator)
              {
                _tcscat(buffer, _T("#,##0"));
                fGeneral = FALSE;
              } 
              else
                _tcscat(buffer, _T("###0"));
  
              if (nf.bFractionalWidth)
              {
                int i;
                _tcscat(buffer, _T("."));
                for (i=0; i<nf.bFractionalWidth; i++)
                  _tcscat(buffer, _T("0"));
              }
              if (nf.fShowCurrencySymbol)
              {
                if (lstrlen(nf.szCurrency))
                {
                  if (!_tcscmp(_T("€"), nf.szCurrency))
                  {
                    //this is a euro
                    _stprintf(szCurrency, _T("[$%s-2]"), nf.szCurrency);
                    fUnicode = TRUE;
                  }
                  else if (!_tcscmp(nf.szCurrency, _T("\\"))) 
                  {
                    //this is yen
			  		        _stprintf(szCurrency, _T("\"\\\""));
//			  		        _stprintf(szCurrency, _T("\"¥\""));
                  }  
                  else
                    _stprintf(szCurrency, _T("[$%s]"), nf.szCurrency);
//                    _tcscpy(szCurrency, nf.szCurrency);
                }
              }  

              ssCellType.Spec.Currency.bNegStyle++;    
*/
              ssCellType.Spec.Currency.bNegStyle = (BYTE)(nf.bNegCurrencyStyle+1);
            }

            if (ssCellType.Spec.Currency.bNegStyle == 1)
            {
              if (ssCellType.Spec.Currency.fShowCurrencySymbol)
                _stprintf(bufferTemp, _T("(%s%s)"), szCurrency, buffer);
              else
                _stprintf(bufferTemp, _T("(%s)"), buffer);
              if (ssCellType.Style & SSS_NEGRED)
                _stprintf(bufferNeg, _T(";[Red]%s"), bufferTemp);
              else
                _stprintf(bufferNeg, _T(";%s"), bufferTemp);

            }
            else if (ssCellType.Spec.Currency.bNegStyle == 2)
            {
              if (ssCellType.Spec.Currency.fShowCurrencySymbol)
                _stprintf(bufferTemp, _T("-%s%s"), szCurrency, buffer);
              else
                _stprintf(bufferTemp, _T("-%s"), buffer);
              if (ssCellType.Style & SSS_NEGRED)
                _stprintf(bufferNeg, _T(";[Red]%s"), bufferTemp);
              else
                _stprintf(bufferNeg, _T(";%s"), bufferTemp);
            }
            else if (ssCellType.Spec.Currency.bNegStyle == 3)
            {
              if (ssCellType.Spec.Currency.fShowCurrencySymbol)
                _stprintf(bufferTemp, _T("%s-%s"), szCurrency, buffer);
              else
                _stprintf(bufferTemp, _T("-%s"), bufferTemp);
              if (ssCellType.Style & SSS_NEGRED)
                _stprintf(bufferNeg, _T(";[Red]%s"), bufferTemp);
              else
                _stprintf(bufferNeg, _T(";%s"), bufferTemp);
            }
            else if (ssCellType.Spec.Currency.bNegStyle == 4)
            {
              if (ssCellType.Spec.Currency.fShowCurrencySymbol)
                _stprintf(bufferTemp, _T("%s%s-"), szCurrency, buffer);
              else
                _stprintf(bufferTemp, _T("%s-"), buffer);
              if (ssCellType.Style & SSS_NEGRED)
                _stprintf(bufferNeg, _T(";[Red]%s"), bufferTemp);
              else
                _stprintf(bufferNeg, _T(";%s"), bufferTemp);
            }
            else if (ssCellType.Spec.Currency.bNegStyle == 5)
            {
              if (ssCellType.Spec.Currency.fShowCurrencySymbol)
                _stprintf(bufferTemp, _T("(%s%s)"), buffer, szCurrency);
              else
                _stprintf(bufferTemp, _T("(%s)"), buffer);
              if (ssCellType.Style & SSS_NEGRED)
                _stprintf(bufferNeg, _T(";[Red]%s"), bufferTemp);
              else
                _stprintf(bufferNeg, _T(";%s"), bufferTemp);
            }
            else if (ssCellType.Spec.Currency.bNegStyle == 6)
            {
              if (ssCellType.Spec.Currency.fShowCurrencySymbol)
                _stprintf(bufferTemp, _T("-%s%s"), buffer, szCurrency);
              else
                _stprintf(bufferTemp, _T("-%s"), buffer);
              if (ssCellType.Style & SSS_NEGRED)
                _stprintf(bufferNeg, _T(";[Red]%s"), bufferTemp);
              else
                _stprintf(bufferNeg, _T(";%s"), bufferTemp);
            }
            else if (ssCellType.Spec.Currency.bNegStyle == 7)
            {
              if (ssCellType.Spec.Currency.fShowCurrencySymbol)
                _stprintf(bufferTemp, _T("%s-%s"), buffer, szCurrency);
              else
                _stprintf(bufferTemp, _T("%s-"), buffer);
              if (ssCellType.Style & SSS_NEGRED)
                _stprintf(bufferNeg, _T(";[Red]%s"), bufferTemp);
              else
                _stprintf(bufferNeg, _T(";%s"), bufferTemp);
            }
            else if (ssCellType.Spec.Currency.bNegStyle == 8)
            {
              if (ssCellType.Spec.Currency.fShowCurrencySymbol)
                _stprintf(bufferTemp, _T("%s%s-"), buffer, szCurrency);
              else
                _stprintf(bufferTemp, _T("%s-"), buffer);
              if (ssCellType.Style & SSS_NEGRED)
                _stprintf(bufferNeg, _T(";[Red]%s"), bufferTemp);
              else
                _stprintf(bufferNeg, _T(";%s"), bufferTemp);
            }
            else if (ssCellType.Spec.Currency.bNegStyle == 9)
            {
              if (ssCellType.Spec.Currency.fShowCurrencySymbol)
                _stprintf(bufferTemp, _T("-%s_*%s"), buffer, szCurrency);
              else
                _stprintf(bufferTemp, _T("-%s_*"), buffer);
              if (ssCellType.Style & SSS_NEGRED)
                _stprintf(bufferNeg, _T(";[Red]%s"), bufferTemp);
              else
                _stprintf(bufferNeg, _T(";%s"), bufferTemp);
            }
            else if (ssCellType.Spec.Currency.bNegStyle == 10)
            {
              if (ssCellType.Spec.Currency.fShowCurrencySymbol)
                _stprintf(bufferTemp, _T("-%s_*%s"),szCurrency, buffer);
              else
                _stprintf(bufferTemp, _T("-_*%s"), buffer);
              if (ssCellType.Style & SSS_NEGRED)
                _stprintf(bufferNeg, _T(";[Red]%s"), bufferTemp);
              else
                _stprintf(bufferNeg, _T(";%s"), bufferTemp);
            }
            else if (ssCellType.Spec.Currency.bNegStyle == 11)
            {
              if (ssCellType.Spec.Currency.fShowCurrencySymbol)
                _stprintf(bufferTemp, _T("%s_*%s-"),buffer, szCurrency);
              else
                _stprintf(bufferTemp, _T("%s_*-"), szCurrency);
              if (ssCellType.Style & SSS_NEGRED)
                _stprintf(bufferNeg, _T(";[Red]%s"), bufferTemp);
              else
                _stprintf(bufferNeg, _T(";%s"), bufferTemp);
            }
            else if (ssCellType.Spec.Currency.bNegStyle == 12)
            {
              if (ssCellType.Spec.Currency.fShowCurrencySymbol)
                _stprintf(bufferTemp, _T("%s_*%s-"), szCurrency, buffer);
              else
                _stprintf(bufferTemp, _T("_*%s-"), buffer);
              if (ssCellType.Style & SSS_NEGRED)
                _stprintf(bufferNeg, _T(";[Red]%s"), bufferTemp);
              else
                _stprintf(bufferNeg, _T(";%s"), bufferTemp);
            }
            else if (ssCellType.Spec.Currency.bNegStyle == 13)
            {
              if (ssCellType.Spec.Currency.fShowCurrencySymbol)
                _stprintf(bufferTemp, _T("%s_*-%s"), szCurrency, buffer);
              else
                _stprintf(bufferTemp, _T("_*-%s"), buffer);
              if (ssCellType.Style & SSS_NEGRED)
                _stprintf(bufferNeg, _T(";[Red]%s"), bufferTemp);
              else
                _stprintf(bufferNeg, _T(";%s"), bufferTemp);
            }
            else if (ssCellType.Spec.Currency.bNegStyle == 14)
            {
              if (ssCellType.Spec.Currency.fShowCurrencySymbol)
                _stprintf(bufferTemp, _T("%s-_*%s"), buffer, szCurrency);
              else
                _stprintf(bufferTemp, _T("%s-_*"), buffer);
              if (ssCellType.Style & SSS_NEGRED)
                _stprintf(bufferNeg, _T(";[Red]%s"), bufferTemp);
              else
                _stprintf(bufferNeg, _T(";%s"), bufferTemp);
            }
            else if (ssCellType.Spec.Currency.bNegStyle == 15)
            {
              if (ssCellType.Spec.Currency.fShowCurrencySymbol)
                _stprintf(bufferTemp, _T("(%s_*%s)"), szCurrency, buffer);
              else
                _stprintf(bufferTemp, _T("(_*%s)"), buffer);
              if (ssCellType.Style & SSS_NEGRED)
                _stprintf(bufferNeg, _T(";[Red]%s"), bufferTemp);
              else
                _stprintf(bufferNeg, _T(";%s"), bufferTemp);
            }
            else if (ssCellType.Spec.Currency.bNegStyle == 16)
            {
              if (ssCellType.Spec.Currency.fShowCurrencySymbol)
                _stprintf(bufferTemp, _T("(%s_*%s)"), buffer, szCurrency);
              else
                _stprintf(bufferTemp, _T("(%s_*)"), buffer);
              if (ssCellType.Style & SSS_NEGRED)
                _stprintf(bufferNeg, _T(";[Red]%s"), bufferTemp);
              else
                _stprintf(bufferNeg, _T(";%s"), bufferTemp);
            }
            else if (ssCellType.Style & SSS_NEGRED)
            {
              _stprintf(bufferNeg, _T(";[Red]%s"), buffer);
            }

            if (lstrlen(bufferPos))
            {
              _tcscpy(buffer, bufferPos);
              if (lstrlen(bufferNeg))
                _tcscat(buffer, bufferNeg);
            }
            else if (lstrlen(bufferNeg))
            {
              _stprintf(buffer, _T("%s"), bufferNeg);
            }

            if (!_tcscmp(buffer, _T("$#,##0;($#,##0)")))
              xf.wifmt = 0x05;
            if (!_tcscmp(buffer, _T("$#,##0;[Red]($#,##0)")))
              xf.wifmt = 0x06;
            if (!_tcscmp(buffer, _T("$#,##0.00;($#,##0.00)")))
              xf.wifmt = 0x07;
            if (!_tcscmp(buffer, _T("$#,##0.00;[Red]($#,##0)")))
              xf.wifmt = 0x08;

            biff.recnum = xlFORMAT;
            biff.datalen = 0;
            lFormatIndex = ssm_BuildFormat(lpAC->lpnFormatCount, lpAC->lpghFormat, (LPBIFF)&biff, lp, lpAC->lplFormatLen, buffer, fUnicode);
            xf.wifmt = (WORD)(0xA4 + lFormatIndex);
          }
          break;
        }

        //Add the XF record for this cell
        biff.recnum = xlXF;
        biff.datalen = 20;
        ssm_BuildXF(lCol, lRow, 1, lpAC->lpnXFIndex, lpAC->lpnXFCount, lpAC->lpghXF, &biff, lp, lpAC->lplXFLen, &xf, lpAC->lpghFont, lpAC->lplFontLen);
//        if (fIsFormula)
//          break;
      } // fFormat?

      if (fData)
      {  
        NUMBER Number;
//        TBGLOBALHANDLE ghData = SS_GetData(lpSS,  &ssCellType, lCol, lRow, FALSE);
//        if (ghData != NULL)
        {
          if (SSGetFloat(lpSS->lpBook->hWnd, lCol, lRow, (LPDOUBLE)&Number.num))
          {
            biff.recnum = xlNUMBER;
            biff.datalen = 14;
            Number.wRow = (WORD)(lRow-1);
            Number.wCol = (WORD)(lCol-1);
            Number.wixfe = *(lpAC->lpnXFIndex);
            //write cell records to Cells
            if (fIsFormula)
              *lpdblRet = Number.num;
            else
              ssm_WriteBuffer(lpAC->lpghCells, &biff, lp, lpAC->lplCellsLen, &Number);
          } 
          else
          {
            if (fIsFormula)
              *lpdblRet = 0.0;
            else
            {
              //write out a BLANK cell record
              BLANK blank;
              memset(&blank, 0, sizeof(BLANK));
              biff.recnum = xlBLANK;
              biff.datalen = 6;
              blank.wRow = (WORD)(lRow-1);
              blank.wCol = (WORD)(lCol-1);
              blank.wixfe = *(lpAC->lpnXFIndex);
              //write cell records to Cells
              ssm_WriteBuffer(lpAC->lpghCells, &biff, lp, lpAC->lplCellsLen, &blank);
            }
          }
        }
      } //fData?
    }
    break;

    case SS_TYPE_PERCENT:
    {
      TCHAR buffer[100];
      TCHAR bufferNeg[100];        

      SSNUM_FORMAT nfSrc, nf;
      memset(&nfSrc, 0, sizeof(SSNUM_FORMAT));
      memset(&nf, 0, sizeof(SSNUM_FORMAT));
      SS_CreatePercentFormatStruct(&ssCellType, &nfSrc);
      NumGetDisplayFormat(&nfSrc, &nf);

      memset(buffer, 0, sizeof(TCHAR)*100);
      memset(bufferNeg, 0, sizeof(TCHAR)*100);

      if (fFormat)
      {
        switch (ssCellType.Spec.Percent.bLeadingZero)
        {
          case 2:
            _tcscat(buffer, _T("0"));
            break;
          case 1:
            _tcscat(buffer, _T("#"));
            break;
          case 0: // international
            if (nf.bLeadingZero)
              _tcscat(buffer, _T("0"));
            else
              _tcscat(buffer, _T("#"));
            break;
        }

    		if (ssCellType.Spec.Percent.Right)
        {
          int i;
          _stprintf(buffer, _T("%s%s"), buffer, _T("."));
          for (i=0; i<ssCellType.Spec.Percent.Right; i++)
            _tcscat(buffer, _T("0"));
        } 

        // Negative format
        if (ssCellType.Spec.Percent.bNegStyle == 0)
        {
          switch (nf.bNegStyle)
          {
            case 0:
              ssCellType.Spec.Percent.bNegStyle = (BYTE)1;
              break;
            case 1:
            case 2:
              ssCellType.Spec.Percent.bNegStyle = (BYTE)2;
              break;
            case 3:
            case 4:
              ssCellType.Spec.Percent.bNegStyle = (BYTE)3;
              break;
          }
        } 
  
        if (ssCellType.Spec.Percent.bNegStyle == 1)
        {
          if (ssCellType.Style & SSS_NEGRED)
            _stprintf(bufferNeg, _T("%%;[Red](%s%%)"), buffer);
          else
            _stprintf(bufferNeg, _T("%%;(%s%%)"), buffer);
        }
        else if (ssCellType.Spec.Percent.bNegStyle == 2)
        {
          if (ssCellType.Style & SSS_NEGRED)
            _stprintf(bufferNeg, _T("%%;[Red]-%s%%"), buffer);
          else
            _stprintf(bufferNeg, _T("%%;-%s%%"), buffer);
        }
        else if (ssCellType.Spec.Percent.bNegStyle == 3)
        {
          if (ssCellType.Style & SSS_NEGRED)
            _stprintf(bufferNeg, _T("%%;[Red]%s-%%"), buffer);
          else
            _stprintf(bufferNeg, _T("%%;%s-%%"), buffer);
        }
        else if (ssCellType.Spec.Percent.bNegStyle == 4)
        {
          if (ssCellType.Style & SSS_NEGRED)
            _stprintf(bufferNeg, _T("%%;[Red]%s%%-"), buffer);
          else
            _stprintf(bufferNeg, _T("%%;%s%%-"), buffer);
        }
        else if (ssCellType.Spec.Percent.bNegStyle == 5)
        {
          if (ssCellType.Style & SSS_NEGRED)
            _stprintf(bufferNeg, _T("%%;[Red]-%s_*%%"), buffer);
          else
            _stprintf(bufferNeg, _T("%%;-%s_*%%"), buffer);
        }
        else if (ssCellType.Spec.Percent.bNegStyle == 6)
        {
          if (ssCellType.Style & SSS_NEGRED)
            _stprintf(bufferNeg, _T("%%;[Red]%s_*%%-"), buffer);
          else
            _stprintf(bufferNeg, _T("%%;%s_*%%-"), buffer);
        }
        else if (ssCellType.Spec.Percent.bNegStyle == 7)
        {
          if (ssCellType.Style & SSS_NEGRED)
            _stprintf(bufferNeg, _T("%%;[Red]%s-_*%%"), buffer);
          else
            _stprintf(bufferNeg, _T("%%;%s-_*%%"), buffer);
        }
        else if (ssCellType.Spec.Percent.bNegStyle == 8)
        {
          if (ssCellType.Style & SSS_NEGRED)
            _stprintf(bufferNeg, _T("%%;[Red](%s_*%%)"), buffer);
          else
            _stprintf(bufferNeg, _T("%%;(%s_*%%)"), buffer);
        }
        else if (ssCellType.Style & SSS_NEGRED)
        {
          _stprintf(bufferNeg, _T("%%;[Red]%s%%"), buffer);
        }
        else
        {
          bufferNeg[0] = (TCHAR)'%';
        }

        if (lstrlen(bufferNeg))
          _tcscat(buffer, bufferNeg);
        
        if (!_tcscmp(buffer, _T("0%")))
        {
          xf.wifmt = 0x09;
        }
        else if (!_tcscmp(buffer, _T("0.00%")))
        {
          xf.wifmt = 0x0a;
        }
        else
        {
          biff.recnum = xlFORMAT;
          biff.datalen = 0;
          lFormatIndex = ssm_BuildFormat(lpAC->lpnFormatCount, lpAC->lpghFormat, (LPBIFF)&biff, lp, lpAC->lplFormatLen, buffer, FALSE);
          xf.wifmt = (WORD)(0xA4 + lFormatIndex);
        }

        //Add the XF record for this cell
        biff.recnum = xlXF;
        biff.datalen = 20;
        ssm_BuildXF(lCol, lRow, 1, lpAC->lpnXFIndex, lpAC->lpnXFCount, lpAC->lpghXF, &biff, lp, lpAC->lplXFLen, &xf, lpAC->lpghFont, lpAC->lplFontLen);
        if (fIsFormula)
          break;
      } // fFormat?

      if (fData)
      {  
        NUMBER Number;

        if (SSGetFloat(lpSS->lpBook->hWnd, lCol, lRow, (LPDOUBLE)&Number.num))
        {
          biff.recnum = xlNUMBER;
          biff.datalen = 14;
          Number.wRow = (WORD)(lRow-1);
          Number.wCol = (WORD)(lCol-1);
          Number.wixfe = *(lpAC->lpnXFIndex);
          //write cell records to Cells
          if (fIsFormula)
            *lpdblRet = Number.num;
          else
            ssm_WriteBuffer(lpAC->lpghCells, &biff, lp, lpAC->lplCellsLen, &Number);
        } 
        else
        {
          if (fIsFormula)
            *lpdblRet = 0.0;
          else
          {
            //write out a BLANK cell record
            BLANK blank;
            memset(&blank, 0, sizeof(BLANK));
            biff.recnum = xlBLANK;
            biff.datalen = 6;
            blank.wRow = (WORD)(lRow-1);
            blank.wCol = (WORD)(lCol-1);
            blank.wixfe = *(lpAC->lpnXFIndex);
            //write cell records to Cells
            ssm_WriteBuffer(lpAC->lpghCells, &biff, lp, lpAC->lplCellsLen, &blank);
          }
        }
      } //fData?
    }
    break;
#endif      

    case SS_TYPE_EDIT:
    case SS_TYPE_BUTTON:
    case SS_TYPE_COMBOBOX:
    case SS_TYPE_CHECKBOX:
    case SS_TYPE_STATICTEXT:
    {  
      if (fFormat)
      {
        BOOL fHChanged = FALSE;
        BOOL fVChanged = FALSE;
        switch(ssCellType.Type)
        {
          case SS_TYPE_BUTTON:
            xl_LogFile(lpSS->lpBook, LOG_EXPORTBUTTON, lCol, lRow, NULL);
          break;
          case SS_TYPE_COMBOBOX:
            xl_LogFile(lpSS->lpBook, LOG_EXPORTCOMBOBOX, lCol, lRow, NULL);
          break;
          case SS_TYPE_CHECKBOX:
            xl_LogFile(lpSS->lpBook, LOG_EXPORTCHECKBOX, lCol, lRow, NULL);

            if ((ssCellType.Style & SSS_ALIGN_LEFT) == SSS_ALIGN_LEFT)
              xf.fAlign = 1;
            else if ((ssCellType.Style & SSS_ALIGN_CENTER) == SSS_ALIGN_CENTER)
              xf.fAlign = 2;
            else if ((ssCellType.Style & SSS_ALIGN_RIGHT) == SSS_ALIGN_RIGHT)
              xf.fAlign = 3;
            else
              xf.fAlign = 0;

            if ((ssCellType.Style & SSS_ALIGN_TOP) == SSS_ALIGN_TOP)
              xf.fVAlign = 0;
            else if ((ssCellType.Style & SSS_ALIGN_BOTTOM) == SSS_ALIGN_BOTTOM)
              xf.fVAlign = 2;
            else if ((ssCellType.Style & SSS_ALIGN_VCENTER) == SSS_ALIGN_VCENTER)
              xf.fVAlign = 1;
            else
              xf.fVAlign = 1;

          break;
          case SS_TYPE_EDIT:
            if (ssCellType.Style & ES_LEFTALIGN)
            {
              xf.fAlign = 1;
              fHChanged = TRUE;
            }
            else if (ssCellType.Style & ES_CENTER)
            {  
              xf.fAlign = 2;
              fHChanged = TRUE;
            }
            else if (ssCellType.Style & ES_RIGHT)
            {
              xf.fAlign = 3;
              fHChanged = TRUE;
            }
          break;
          case SS_TYPE_STATICTEXT:
            if (ssCellType.Style & SS_TEXT_LEFT)
            {  
              xf.fAlign = 1;
              fHChanged = TRUE;
            }
            else if (ssCellType.Style & SS_TEXT_RIGHT)
            {  
              xf.fAlign = 3;
              fHChanged = TRUE;
            }
            else if (ssCellType.Style & SS_TEXT_CENTER)
            {  
              xf.fAlign = 2;
              fHChanged = TRUE;
            }
  
            if (ssCellType.Style & SS_TEXT_VCENTER)
            {  
              xf.fVAlign = 1;
              fVChanged = TRUE;
            }
            else if (ssCellType.Style & SS_TEXT_BOTTOM)
            {  
              xf.fVAlign = 2;
              fVChanged = TRUE;
            }
            else if (ssCellType.Style & SS_TEXT_TOP)
            {  
              xf.fVAlign = 0;
              fVChanged = TRUE;
            }
            xf.wifmt = 0x31;

            xl_LogFile(lpSS->lpBook, LOG_EXPORTSTATICTEXT, lCol, lRow, NULL);
          break;
        } //switch

        //Add the XF record for this cell
        biff.recnum = xlXF;
        biff.datalen = 20;
        ssm_BuildXF(lCol, lRow, 1, lpAC->lpnXFIndex, lpAC->lpnXFCount, lpAC->lpghXF, &biff, lp, lpAC->lplXFLen, &xf, lpAC->lpghFont, lpAC->lplFontLen);
      } // fFormat?

      if (fData)
      {
        TBGLOBALHANDLE gh = (TBGLOBALHANDLE)0;
        int nLen = SSGetDataLen(lpSS->lpBook->hWnd, lCol, lRow);

        if (nLen > 0) //255) //RAP20a
        {
          Alloc(&gh, (nLen+1)*sizeof(TCHAR));
          LPTSTR lptstr = (LPTSTR)tbGlobalLock(gh);
          if (ssCellType.Type == SS_TYPE_EDIT && (ssCellType.Style & ES_PASSWORD) == ES_PASSWORD)
          {
            int i=0;
            for (i=0; i<nLen; i++)
              lptstr[i] = (TCHAR)'*';
          }
          else
            SSGetData(lpSS->lpBook->hWnd, lCol, lRow, lptstr);

          {
///* RAP07d
            if (!fIsFormula)
            {
              DWORD dwisst = ssm_BuildSST(lpSS->lpBook->hWnd, lCol, lRow, lpAC->lpghSST, lpAC->lplSSTLen, lpAC->lpghEXTSST, lpAC->lplEXTSSTLen, ssCellType.Type, lptstr, lpghStrings);
              if (dwisst != (DWORD)-1)
              {
                LABELSST LabelSST;
                biff.recnum = xlLABELSST;
                biff.datalen = 10;
                LabelSST.wRow = (WORD)(lRow-1);
                LabelSST.wCol = (WORD)(lCol-1);
                LabelSST.wixfe = *(WORD *)(lpAC->lpnXFIndex);
                LabelSST.isst = dwisst;
                //write cell records to Cells
                ssm_WriteBuffer(lpAC->lpghCells, &biff, lp, lpAC->lplCellsLen, &LabelSST);
              }
              else
              {
                //write out a BLANK cell record
                BLANK blank;
                memset(&blank, 0, sizeof(BLANK));
                biff.recnum = xlBLANK;
                biff.datalen = 6;
                blank.wRow = (WORD)(lRow-1);
                blank.wCol = (WORD)(lCol-1);
                blank.wixfe = *(lpAC->lpnXFIndex);
                //write cell records to Cells
                ssm_WriteBuffer(lpAC->lpghCells, &biff, lp, lpAC->lplCellsLen, &blank);
              }
            }
            else //FORMULA
            {
              if (lpghRet)
                *lpghRet = gh;
            }
          }
        }
        else if (!fIsFormula)
        {
          //write out a BLANK cell record
          BLANK blank;
          memset(&blank, 0, sizeof(BLANK));
          biff.recnum = xlBLANK;
          biff.datalen = 6;
          blank.wRow = (WORD)(lRow-1);
          blank.wCol = (WORD)(lCol-1);
          blank.wixfe = *(lpAC->lpnXFIndex);
          //write cell records to Cells
          ssm_WriteBuffer(lpAC->lpghCells, &biff, lp, lpAC->lplCellsLen, &blank);
        }
      } //fData?
    }
    break;

    case SS_TYPE_PIC:
      if (ssCellType.Spec.Pic.hMask)
      {
        if (fFormat)
        {
          LPTSTR lptstr = (LPTSTR)tbGlobalLock((TBGLOBALHANDLE)ssCellType.Spec.Pic.hMask);
          TCHAR  szBuffer[100];
          if (0 == xl_ParsePicFormat(lpSS, lptstr, szBuffer))
          {
            biff.recnum = xlFORMAT;
            biff.datalen = 0;
            lFormatIndex = ssm_BuildFormat(lpAC->lpnFormatCount, lpAC->lpghFormat, (LPBIFF)&biff, lp, lpAC->lplFormatLen, szBuffer, FALSE);
            xf.wifmt = (WORD)(0xA4 + lFormatIndex);
          }
        } // fFormat?

//RAP08a >>
        //Add the XF record for this cell
        biff.recnum = xlXF;
        biff.datalen = 20;
        ssm_BuildXF(lCol, lRow, 1, lpAC->lpnXFIndex, lpAC->lpnXFCount, lpAC->lpghXF, &biff, lp, lpAC->lplXFLen, &xf, lpAC->lpghFont, lpAC->lplFontLen);
//RAP08a <<
          
        if (fData)
        {
          NUMBER Number;
//          short  sLen = 0;
          TBGLOBALHANDLE ghData = 0;
          LPTSTR lpData = NULL;
          long   lDataLen;
//          int    i;
    
          memset(&Number, 0, sizeof(NUMBER));
          lDataLen = SSGetDataLen(lpSS->lpBook->hWnd, lCol, lRow);
          if (lDataLen)
          {
            ghData = tbGlobalAlloc(GHND, (lDataLen+1)*sizeof(TCHAR));
            lpData = (LPTSTR)tbGlobalLock(ghData);
            SSGetData(lpSS->lpBook->hWnd, lCol, lRow, (LPTSTR)lpData);
          }      
 
          if (lpData)
          {                
// RAP 3/10/2004 - 13861
//            sLen = (short)lstrlen(lpData);
//            for (i=0; i<sLen; i++)
//            {
//              if (lpData[i] >= (TCHAR)'0' && lpData[i] <= (TCHAR)'9')
//              {
//                Number.num *= 10;
//                Number.num += lpData[i] - (TCHAR)'0'; 
//              }
//            }
//            tbGlobalUnlock(ghData);
//            tbGlobalFree(ghData);
//            ghData=0;

// RAP 3/10/2004 - 13861 >>
            DWORD dwisst = ssm_BuildSST(lpSS->lpBook->hWnd, lCol, lRow, lpAC->lpghSST, lpAC->lplSSTLen, lpAC->lpghEXTSST, lpAC->lplEXTSSTLen, ssCellType.Type, NULL, lpghStrings);
            if (dwisst != (DWORD)-1)
            {
              LABELSST LabelSST;
              biff.recnum = xlLABELSST;
              biff.datalen = 10;
              LabelSST.wRow = (WORD)(lRow-1);
              LabelSST.wCol = (WORD)(lCol-1);
              LabelSST.wixfe = *(WORD *)(lpAC->lpnXFIndex);
              LabelSST.isst = dwisst;
              //write cell records to Cells
              ssm_WriteBuffer(lpAC->lpghCells, &biff, lp, lpAC->lplCellsLen, &LabelSST);
            }
            else
            {
              //write out a BLANK cell record
              BLANK blank;
              memset(&blank, 0, sizeof(BLANK));
              biff.recnum = xlBLANK;
              biff.datalen = 6;
              blank.wRow = (WORD)(lRow-1);
              blank.wCol = (WORD)(lCol-1);
              blank.wixfe = *(lpAC->lpnXFIndex);
              //write cell records to Cells
              ssm_WriteBuffer(lpAC->lpghCells, &biff, lp, lpAC->lplCellsLen, &blank);
            }
          }
// RAP 3/10/2004 - 13861 <<

// RAP 3/10/2004 - 13861
//          if (Number.num != 0)
//          {
//            biff.recnum = xlNUMBER;
//            biff.datalen = 14;
//            Number.wRow = (WORD)(lRow-1);
//            Number.wCol = (WORD)(lCol-1);
//            Number.wixfe = *(lpAC->lpnXFIndex);
//            //write cell records to Cells
//            ssm_WriteBuffer(lpAC->lpghCells, &biff, lp, lpAC->lplCellsLen, &Number);
//          }
          else
          {
            //write out a BLANK cell record
            BLANK blank;
            memset(&blank, 0, sizeof(BLANK));
            biff.recnum = xlBLANK;
            biff.datalen = 6;
            blank.wRow = (WORD)(lRow-1);
            blank.wCol = (WORD)(lCol-1);
            blank.wixfe = *(lpAC->lpnXFIndex);
            //write cell records to Cells
            ssm_WriteBuffer(lpAC->lpghCells, &biff, lp, lpAC->lplCellsLen, &blank);
          }
        } //fData?
      }
      else
      {
        //ignore
        tbGlobalUnlock((TBGLOBALHANDLE)ssCellType.Spec.Pic.hMask);
        return 2; // tell the caller to continue;
      }
      tbGlobalUnlock((TBGLOBALHANDLE)ssCellType.Spec.Pic.hMask);
    break;

    case SS_TYPE_PICTURE:
    case SS_TYPE_OWNERDRAW:
      // since this are object-types, they are not supported
      if (fFormat)
      {
                //Add the XF record for this cell
        biff.recnum = xlXF;
        biff.datalen = 20;
        ssm_BuildXF(lCol, lRow, 1, lpAC->lpnXFIndex, lpAC->lpnXFCount, lpAC->lpghXF, &biff, lp, lpAC->lplXFLen, &xf, lpAC->lpghFont, lpAC->lplFontLen);
      }
      //write out a BLANK cell record
      BLANK blank;
      memset(&blank, 0, sizeof(BLANK));
      biff.recnum = xlBLANK;
      biff.datalen = 6;
      blank.wRow = (WORD)(lRow-1);
      blank.wCol = (WORD)(lCol-1);
      blank.wixfe = *(lpAC->lpnXFIndex);
      //write cell records to Cells
      ssm_WriteBuffer(lpAC->lpghCells, &biff, lp, lpAC->lplCellsLen, &blank);

      xl_LogFile(lpSS->lpBook, LOG_OBJCELLTYPE, lCol, lRow, &ssCellType.Type);
      return 2; // tell the caller to continue;

#if SS_V80  
    case SS_TYPE_CUSTOM:
    {
      SS_DATA data;
      memset(&data, 0, sizeof(SS_DATA));
      SS_GetDataStruct(lpSS, NULL, NULL, NULL, &data, lCol, lRow);
      
      switch (data.bDataType)
      {
        case SS_DATATYPE_EDIT:
        {
          LPTSTR lpText = (LPTSTR)tbGlobalLock(data.Data.hszData);
          DWORD dwisst = ssm_BuildSST(lpSS->lpBook->hWnd, lCol, lRow, lpAC->lpghSST, lpAC->lplSSTLen, lpAC->lpghEXTSST, lpAC->lplEXTSSTLen, ssCellType.Type, lpText, lpghStrings);
          if (dwisst != (DWORD)-1)
          {
            LABELSST LabelSST;
            biff.recnum = xlLABELSST;
            biff.datalen = 10;
            LabelSST.wRow = (WORD)(lRow-1);
            LabelSST.wCol = (WORD)(lCol-1);
            LabelSST.wixfe = *(WORD *)(lpAC->lpnXFIndex);
            LabelSST.isst = dwisst;
            //write cell records to Cells
            ssm_WriteBuffer(lpAC->lpghCells, &biff, lp, lpAC->lplCellsLen, &LabelSST);
          }
          tbGlobalUnlock(data.Data.hszData);
        }
          break;
        case SS_DATATYPE_DATETIME:
        case SS_DATATYPE_FLOAT:
        {
          NUMBER Number;

          biff.recnum = xlNUMBER;
          biff.datalen = 14;
          Number.wRow = (WORD)(lRow-1);
          Number.wCol = (WORD)(lCol-1);
          Number.wixfe = 0;
          Number.num = data.Data.dfValue;
          //write cell records to Cells
          ssm_WriteBuffer(lpAC->lpghCells, &biff, lp, lpAC->lplCellsLen, &Number);
        }
          break;
        case SS_DATATYPE_INTEGER:
        {
          NUMBER Number;

          biff.recnum = xlNUMBER;
          biff.datalen = 14;
          Number.wRow = (WORD)(lRow-1);
          Number.wCol = (WORD)(lCol-1);
          Number.wixfe = 0;
          Number.num = (double)data.Data.lValue;
          //write cell records to Cells
          ssm_WriteBuffer(lpAC->lpghCells, &biff, lp, lpAC->lplCellsLen, &Number);
        }
          break;

      }

      break;
    }
#endif //SS_V80      
  
  }
  return 0;
}

/***********************************************************************
* Name:   ssm_GetStorageObjectList - Enumerate the list of storage object
*                                    names in the storage docfile and return
*                                    the list.
*
* Usage:  long ssm_GetStorageObjectList(IStorage *pstg,
*                                       TBGLOBALHANDLE *pghStreamList,
*                                       short *lpsCount)
*           pstg - pointer to the storage docfile
*           pghStreamList - pointer to the buffer
*           lpsCount - count of object names in the buffer.
*
* Return: long - Success or Failure
***********************************************************************/
long ssm_GetStorageObjectList(IStorage *pstg, TBGLOBALHANDLE *pghStreamList, short *lpsCount)
{
  IEnumSTATSTG *penum;
  IMalloc      *pmalloc;
  STATSTG       statstg;
  unsigned long celtFetched;
  WCHAR        *pwchar;
  long          lSize = 0;
  HRESULT       hr;
  
  *lpsCount = 0;
  
  hr = pstg->EnumElements(0, NULL, 0, &penum);
  if (S_OK != hr)
  {
		DisplayError(hr,_T("EnumElements"));
    return hr;
	}

  hr = penum->Reset();
  if (S_OK != hr)
  {
		DisplayError(hr,_T("Reset"));
    return hr;
	}

  hr = S_OK;
  while (S_FALSE != hr)
  {
    hr = penum->Next(1, &statstg, &celtFetched);
    if (S_FALSE == hr)
      break;
    else if (S_OK != hr)
    {
		  DisplayError(hr,_T("Next"));
      return hr;
	  }
   
    if (0 == statstg.pwcsName)
      continue;

    if (0 == *lpsCount)
      *pghStreamList = tbGlobalAlloc(GHND, 2 * (wcslen(statstg.pwcsName)+1)); 
    else
    {
      *pghStreamList = tbGlobalReAlloc(*pghStreamList,
                                     2 * (lSize + wcslen(statstg.pwcsName)+1),
                                     GMEM_ZEROINIT);
    }
    pwchar = (LPWSTR)tbGlobalLock(*pghStreamList);
    wcscpy((pwchar + lSize), statstg.pwcsName);
    lSize += ((long)wcslen(statstg.pwcsName) + 1); 
    tbGlobalUnlock(*pghStreamList);
     
    // get the IMalloc pointer to free the pwcsName memory...
    hr = CoGetMalloc(1, &pmalloc);
    if (S_OK != hr)
    {
  		DisplayError(hr,_T("CoGetMalloc"));
      return hr;
	  }

    pmalloc->Free(statstg.pwcsName);

    (*lpsCount)++;

//    WideCharToMultiByte(CP_ACP, WC_COMPOSITECHECK, statstg.pwcsName,
//                        -1, szElementName, 256, NULL, NULL);
  }

  penum->Release();
  return S_OK;
}


/***********************************************************************
* Name:   ssm_AddExprToFormulaBuffer
*
* Return: long - Success or Failure
***********************************************************************/
//long ssm_AddExprToFormulaBuffer(LPSPREADSHEET lpSS, SS_COORD lCol, SS_COORD lRow, LPCALC_EXPRINFO lpExpr, TBGLOBALHANDLE *lpghFormula, long *lplFormulaLen, short *lpnFormulaPos, BOOL *fParen, TBGLOBALHANDLE aStrings[100], LPSHORT lpxtiCount, LPTBGLOBALHANDLE lpghXTI, LPLONG lplFirstRefOffset, LPLONG lplNextRefOffset, LPTBGLOBALHANDLE lpghCustomNames, LPLONG lplCustomNamesLen)
long ssm_AddExprToFormulaBuffer(LPSPREADSHEET lpSS, SS_COORD lCol, SS_COORD lRow, LPCALC_EXPRINFO lpExpr, TBGLOBALHANDLE *lpghFormula, long *lplFormulaLen, short *lpnFormulaPos, BOOL *fParen, TBGLOBALHANDLE aStrings[100], LPSHORT lpxtiCount, LPTBGLOBALHANDLE lpghXTI, LPTBGLOBALHANDLE lpghRefOffset, LPLONG lplRefOffsetCount, LPTBGLOBALHANDLE lpghCustomNames, LPLONG lplCustomNamesLen)
{
  TBGLOBALHANDLE ghArgs = (TBGLOBALHANDLE)0;
  LPBYTE       lpFormula;
  BYTE         bToken = 0;
  LPTSTR       lpszText = NULL;
  AREA         Area;
  AREA3D       Area3d;
  short        i = 0;
  short        nIF = 0;
  BOOL         fArea = FALSE;
  BOOL         fAlreadyProcessed = FALSE;
  BOOL         fErr = FALSE;

  switch( lpExpr->nType )
  {
    case CALC_EXPR_CUSTFUNC:
    {
      xl_LogFile(lpSS->lpBook, LOG_CUSTFUNCNOTSUPPORTED, lCol, lRow, NULL);
      fErr = TRUE;
    }
      break;   
    case CALC_EXPR_LONG:
    {
      double dbl = (double)lpExpr->u.Long.lVal;
      bToken = ptgNum;
     *lplFormulaLen += sizeof(BYTE) + sizeof(double);
      Alloc(lpghFormula, *lplFormulaLen);
      lpFormula = (LPBYTE)tbGlobalLock(*lpghFormula);
      memcpy(lpFormula + *lpnFormulaPos, &bToken, sizeof(BYTE));
      *lpnFormulaPos += sizeof(BYTE);
      memcpy(lpFormula + *lpnFormulaPos, &dbl, sizeof(double));
      *lpnFormulaPos += sizeof(double);
      tbGlobalUnlock(*lpghFormula);
    }
      break;
    case CALC_EXPR_DOUBLE:
      bToken = ptgNum;
      *lplFormulaLen += sizeof(BYTE) + sizeof(double);
      Alloc(lpghFormula, *lplFormulaLen);
      lpFormula = (LPBYTE)tbGlobalLock(*lpghFormula);
      memcpy(lpFormula + *lpnFormulaPos, &bToken, sizeof(BYTE));
      *lpnFormulaPos += sizeof(BYTE);
      memcpy(lpFormula + *lpnFormulaPos, &lpExpr->u.Double.dfVal, sizeof(double));
      *lpnFormulaPos += sizeof(double);
      tbGlobalUnlock(*lpghFormula);
      break;
    case CALC_EXPR_CELL:
    {
      CELLREF8 cr;
      WORD     wCol = 0;

      memset(&cr, 0, sizeof(CELLREF8));
      cr.bToken = ptgRefV;

      //if the row or column is & 0x80000000 then the address is relative
      if (lpExpr->u.Cell.lRow & 0x80000000)
      {
        cr.fRwRel = TRUE;
        cr.rw = (short)(lRow-1+(lpExpr->u.Cell.lRow & 0x7FFFFFFF));
      }
      else
        cr.rw = (WORD)(lpExpr->u.Cell.lRow-1);
      if (lpExpr->u.Cell.lCol & 0x80000000)
      {
        cr.fColRel = TRUE;
        wCol = (WORD)(lCol-1+(lpExpr->u.Cell.lCol & 0x7FFFFFFF));
      }
      else
        wCol = (WORD)(lpExpr->u.Cell.lCol-1);
      cr.col = wCol;

      *lplFormulaLen += sizeof(CELLREF8);
      Alloc(lpghFormula, *lplFormulaLen);

      Alloc(lpghRefOffset, ++(*lplRefOffsetCount) * sizeof(LONG));
      LPLONG lplOffset = (LPLONG)tbGlobalLock(*lpghRefOffset);
      lplOffset += (*lplRefOffsetCount)-1;
      *lplOffset = *lpnFormulaPos;
      tbGlobalUnlock(*lpghRefOffset);

      lpFormula = (LPBYTE)tbGlobalLock(*lpghFormula);
      memcpy(lpFormula + *lpnFormulaPos, &cr, sizeof(CELLREF8));
      *lpnFormulaPos += sizeof(CELLREF8);
      tbGlobalUnlock(*lpghFormula);
    }
      break;
    case CALC_EXPR_RANGE:
    {
      _fmemset((LPAREA)&Area, 0, sizeof(AREA));
      
      Area.bToken = ptgArea;

      if (lpExpr->u.Range.lRow1 & 0x80000000)
        Area.fRwRelFirst = TRUE;
      if (lpExpr->u.Range.lRow2 & 0x80000000)
        Area.fRwRelLast = TRUE;
      if (lpExpr->u.Range.lCol1 & 0x80000000)
        Area.fColRelFirst = TRUE;
      if (lpExpr->u.Range.lCol2 & 0x80000000)
        Area.fColRelLast = TRUE;
//      Area.rwFirst = (WORD)((lpExpr->u.Range.lRow1 & 0x7FFFFFFF)-1);
//      Area.rwLast = (WORD)((lpExpr->u.Range.lRow2 & 0x7FFFFFFF)-1);
//      Area.colFirst = (lpExpr->u.Range.lCol1 & 0x7FFFFFFF)-1;
//      Area.colLast = (lpExpr->u.Range.lCol2 & 0x7FFFFFFF)-1;
      if (Area.fRwRelFirst)
        Area.rwFirst = (WORD)(lRow + (lpExpr->u.Range.lRow1 & 0x7FFFFFFF)-1);
      else
        Area.rwFirst = (WORD)(lpExpr->u.Range.lRow1-1);
      if (Area.fRwRelLast)
        Area.rwLast = (WORD)(lRow + (lpExpr->u.Range.lRow2 & 0x7FFFFFFF)-1);
      else
        Area.rwLast = (WORD)(lpExpr->u.Range.lRow2-1);
      if (Area.fColRelFirst)
        Area.colFirst = lCol + (lpExpr->u.Range.lCol1 & 0x7FFFFFFF)-1;
      else
        Area.colFirst = (WORD)lpExpr->u.Range.lCol1-1;
      if (Area.fColRelLast)
        Area.colLast = lCol + (lpExpr->u.Range.lCol2 & 0x7FFFFFFF)-1;
      else
        Area.colLast = (WORD)(lpExpr->u.Range.lCol2-1);

      *lplFormulaLen += sizeof(AREA);
      Alloc(lpghFormula, *lplFormulaLen);
      lpFormula = (LPBYTE)tbGlobalLock(*lpghFormula);
      memcpy(lpFormula + *lpnFormulaPos, &Area, sizeof(AREA));
      *lpnFormulaPos += sizeof(AREA);
    }
      break;
#ifdef SS_V70
    case CALC_EXPR_EXTERNALCELL:
      {
      CELLREF3D cr;
      WORD      wCol = 0;

      memset(&cr, 0, sizeof(CELLREF3D));
      if (lRow == -1 && lCol == -1)
        cr.bToken = ptgRef3dA;
      else
        cr.bToken = ptgRef3dV;
      //if the row or column is & 0x80000000 then the address is relative
      if (lpExpr->u.ExternalCell.lpSheet != NULL)
      {
        LPXTI  lpxti = NULL;
        USHORT firstSheet = (USHORT)(lpExpr->u.ExternalCell.lpSheet->lpfnGetIndex(lpExpr->u.ExternalCell.lpSheet->hSS));

        cr.externSheetIndex = (USHORT)-1;
        if (*lpxtiCount > 0)
        {
          USHORT i;
          lpxti = (LPXTI)tbGlobalLock(*lpghXTI);
          for (i=0; i<*lpxtiCount; i++)
          {
            if (lpxti[i].supBookIndex == 0 &&
                lpxti[i].firstSheet == firstSheet &&
                lpxti[i].lastSheet == firstSheet)
            {
              cr.externSheetIndex = i;
              break;
            }
          }
          tbGlobalUnlock(*lpghXTI);
        }
        if (cr.externSheetIndex == (USHORT)-1) // not found
        {
          Alloc(lpghXTI, (++(*lpxtiCount))*sizeof(XTI));
          lpxti = (LPXTI)tbGlobalLock(*lpghXTI);
          lpxti[*lpxtiCount-1].supBookIndex = (USHORT)0;
          lpxti[*lpxtiCount-1].firstSheet = firstSheet;
          lpxti[*lpxtiCount-1].lastSheet = firstSheet;
          cr.externSheetIndex = (USHORT)(*lpxtiCount-1);
          tbGlobalUnlock(*lpghXTI);
        }
      }
      if (lpExpr->u.ExternalCell.lRow & 0x80000000)
      {
        cr.fRwRel = TRUE;
        cr.rw = (short)(lRow-1+(lpExpr->u.ExternalCell.lRow & 0x7FFFFFFF));
      }
      else
        cr.rw = (WORD)(lpExpr->u.ExternalCell.lRow-1);
      if (lpExpr->u.ExternalCell.lCol & 0x80000000)
      {
        cr.fColRel = TRUE;
        wCol = (WORD)(lCol-1+(lpExpr->u.ExternalCell.lCol & 0x7FFFFFFF));
      }
      else
        wCol = (WORD)(lpExpr->u.ExternalCell.lCol-1);
      cr.col = wCol;

      Alloc(lpghRefOffset, ++(*lplRefOffsetCount) * sizeof(LONG));
      LPLONG lplOffset = (LPLONG)tbGlobalLock(*lpghRefOffset);
      lplOffset += (*lplRefOffsetCount)-1;
      *lplOffset = *lpnFormulaPos;
      tbGlobalUnlock(*lpghRefOffset);

      *lplFormulaLen += sizeof(CELLREF3D);
      Alloc(lpghFormula, *lplFormulaLen);
      lpFormula = (LPBYTE)tbGlobalLock(*lpghFormula);
      memcpy(lpFormula + *lpnFormulaPos, &cr, sizeof(CELLREF3D));
      *lpnFormulaPos += sizeof(CELLREF3D);
      tbGlobalUnlock(*lpghFormula);
    }
      break;

    case CALC_EXPR_EXTERNALRANGE:
    {
      AREA3D area;
      WORD   wCol;
      LPXTI  lpxti = NULL;

      _fmemset((LPAREA3D)&area, 0, sizeof(AREA3D));
      area.bToken = ptgArea3d;

      //if the row or column is & 0x80000000 then the address is relative
      if (lpExpr->u.ExternalRange.lpSheet != NULL)
      {

        USHORT firstSheet = (USHORT)(lpExpr->u.ExternalCell.lpSheet->lpfnGetIndex(lpExpr->u.ExternalCell.lpSheet->hSS));

//====
        area.ixti = (USHORT)-1;
        if (*lpxtiCount > 0)
        {
          USHORT i;
          lpxti = (LPXTI)tbGlobalLock(*lpghXTI);
          for (i=0; i<*lpxtiCount; i++)
          {
            if (lpxti[i].supBookIndex == 0 &&
                lpxti[i].firstSheet == firstSheet &&
                lpxti[i].lastSheet == firstSheet)
            {
              area.ixti = i;
              break;
            }
          }
          tbGlobalUnlock(*lpghXTI);
        }
        if (area.ixti == (USHORT)-1) // not found
        {
          Alloc(lpghXTI, (++(*lpxtiCount))*sizeof(XTI));
          lpxti = (LPXTI)tbGlobalLock(*lpghXTI);
          lpxti[*lpxtiCount-1].supBookIndex = 0;
          lpxti[*lpxtiCount-1].firstSheet = firstSheet;
          lpxti[*lpxtiCount-1].lastSheet = firstSheet;
          area.ixti = (USHORT)(*lpxtiCount-1);
          tbGlobalUnlock(*lpghXTI);
        }
//====
     
      }
      if (lpExpr->u.ExternalRange.lRow1 & 0x80000000)
      {
        area.fRwRelFirst = TRUE;
        area.rwFirst = (WORD)(lRow-1+(lpExpr->u.ExternalRange.lRow1 & 0x7FFFFFFF));
      }
      else
        area.rwFirst = (WORD)((lpExpr->u.ExternalRange.lRow1 & 0x7FFFFFFF)-1);

      if (lpExpr->u.ExternalRange.lCol1 & 0x80000000)
      {
        area.fColRelFirst = TRUE;
        wCol = (WORD)(lCol-1+(lpExpr->u.ExternalRange.lCol1 & 0x7FFFFFFF));
      }
      else
        wCol = (WORD)((lpExpr->u.ExternalRange.lCol1 & 0x7FFFFFFF)-1);
      area.colFirst = wCol;

      if (lpExpr->u.ExternalRange.lRow2 & 0x80000000)
      {
        area.fRwRelLast = TRUE;
        area.rwLast = (WORD)(lRow-1+(lpExpr->u.ExternalRange.lRow2 & 0x7FFFFFFF));
      }
      else
        area.rwLast = (WORD)((lpExpr->u.ExternalRange.lRow2 & 0x7FFFFFFF)-1);

      if (lpExpr->u.ExternalRange.lCol2 & 0x80000000)
      {
        area.fColRelLast = TRUE;
        wCol = (WORD)(lCol-1+(lpExpr->u.ExternalRange.lCol2 & 0x7FFFFFFF));
      }
      else
        wCol = (WORD)((lpExpr->u.ExternalRange.lCol2 & 0x7FFFFFFF)-1);
      area.colLast = wCol;

      *lplFormulaLen += sizeof(AREA3D);
      Alloc(lpghFormula, *lplFormulaLen);

      lpFormula = (LPBYTE)tbGlobalLock(*lpghFormula);
      memcpy(lpFormula + *lpnFormulaPos, &area, sizeof(AREA3D));
      *lpnFormulaPos += sizeof(AREA3D);
      tbGlobalUnlock(*lpghFormula);
    }
      break;
#endif
    case CALC_EXPR_FUNCTION:
    {
      FUNCVAR      fv;
      ATTR         Attr;
      TBGLOBALHANDLE ghExtra = 0;
      short        sExtraLen = 0;
      short        sFunctionLen = 0;
      WORD         wToken = 0;
      BOOL         fFuncVar = FALSE;
      BOOL         fAttr = FALSE;
      fv.fPrompt = 0;
      fv.fCE = 0;

      switch (lpExpr->u.Func.nId)
      {
#if SS_V80
        case CALC_FUNC_AVEDEV:
//        case CALC_FUNC_BINOMDIST:
//        case CALC_FUNC_CHIDIST:
//        case CALC_FUNC_CHIINV:
//        case CALC_FUNC_CONFIDENCE:
        case CALC_FUNC_DEVSQ:
//        case CALC_FUNC_NPV:
//        case CALC_FUNC_IPMT:
//        case CALC_FUNC_IRR:
//        case CALC_FUNC_PPMT:
//        case CALC_FUNC_VDB:
//        case CALC_FUNC_DAYS360:
        case CALC_FUNC_SUBTOTAL:
//        case CALC_FUNC_SUMPRODUCT:
        case CALC_FUNC_SKEW:
        case CALC_FUNC_SUMIF:
#endif
        case CALC_FUNC_AVERAGE:
        case CALC_FUNC_STDEV:
        case CALC_FUNC_STDEVP:
        case CALC_FUNC_MAX:
        case CALC_FUNC_MEDIAN:
        case CALC_FUNC_MIN:
        case CALC_FUNC_MODE:
        case CALC_FUNC_PRODUCT:
        case CALC_FUNC_RANK:
          if (*lplRefOffsetCount != 0)
          {
            LPLONG lplOffset = (LPLONG)tbGlobalLock(*lpghRefOffset);
            int i;
            lpFormula = (LPBYTE)tbGlobalLock(*lpghFormula);
            for (i=*lplRefOffsetCount-1; i>=max(0, *lplRefOffsetCount-lpExpr->u.Func.nArgs); i--)
            { 
              long lOffset = lplOffset[i];
              if (lOffset == -1)
                continue;
              LPCELLREF8 lpCellRef = (LPCELLREF8)((LPBYTE)lpFormula + lplOffset[i]);
              lpCellRef->bToken = ptgRef;
            }
            *lplRefOffsetCount = max(0, *lplRefOffsetCount-lpExpr->u.Func.nArgs);
            tbGlobalUnlock(*lpghRefOffset);
//            tbGlobalFree(*lpghRefOffset);
//            *lpghRefOffset = 0;
            tbGlobalUnlock(*lpghFormula);
          }
          break;
      }
      switch(lpExpr->u.Func.nId)
      {
#ifdef SS_V80
        //FuncVarV
        case CALC_FUNC_AVEDEV:
          fv.bToken = ptgFuncVarV;
          fv.cargs = (TCHAR)lpExpr->u.Func.nArgs;
          fv.fPrompt = 0;
          fv.iftab = xlfAvedev;
          sFunctionLen += sizeof(FUNCVAR);
          fFuncVar = TRUE;
        break;
        case CALC_FUNC_BETADIST:
          fv.bToken = ptgFuncVarV;
          fv.cargs = (TCHAR)lpExpr->u.Func.nArgs;
          fv.fPrompt = 0;
          fv.iftab = xlfBetadist;
          sFunctionLen += sizeof(FUNCVAR);
          fFuncVar = TRUE;
        break;
        case CALC_FUNC_BETAINV:
          fv.bToken = ptgFuncVarV;
          fv.cargs = (TCHAR)lpExpr->u.Func.nArgs;
          fv.fPrompt = 0;
          fv.iftab = xlfBetainv;
          sFunctionLen += sizeof(FUNCVAR);
          fFuncVar = TRUE;
         break;
        case CALC_FUNC_COUNT:
          fv.bToken = ptgFuncVarV;
          fv.cargs = (TCHAR)lpExpr->u.Func.nArgs;
          fv.fPrompt = 0;
          fv.iftab = xlfCount;
          sFunctionLen += sizeof(FUNCVAR);
          fFuncVar = TRUE;
        break;
        case CALC_FUNC_COUNTA:
          fv.bToken = ptgFuncVarV;
          fv.cargs = (TCHAR)lpExpr->u.Func.nArgs;
          fv.fPrompt = 0;
          fv.iftab = xlfCounta;
          sFunctionLen += sizeof(FUNCVAR);
          fFuncVar = TRUE;
        break;
        case CALC_FUNC_COUNTIF:
          fv.bToken = ptgFuncVarV;
          fv.cargs = (TCHAR)lpExpr->u.Func.nArgs;
          fv.fPrompt = 0;
          fv.iftab = xlfCountif;
          sFunctionLen += sizeof(FUNCVAR);
          fFuncVar = TRUE;
        break;
        case CALC_FUNC_DAYS360:
          fv.bToken = ptgFuncVarV;
          fv.cargs = (TCHAR)lpExpr->u.Func.nArgs;
          fv.fPrompt = 0;
          fv.iftab = xlfDays360;
          sFunctionLen += sizeof(FUNCVAR);
          fFuncVar = TRUE;
        break;
        case CALC_FUNC_DEVSQ:
          fv.bToken = ptgFuncVarV;
          fv.cargs = (TCHAR)lpExpr->u.Func.nArgs;
          fv.fPrompt = 0;
          fv.iftab = xlfDevsq;
          sFunctionLen += sizeof(FUNCVAR);
          fFuncVar = TRUE;
        break;
        case CALC_FUNC_IPMT:
          fv.bToken = ptgFuncVarV;
          fv.cargs = (TCHAR)lpExpr->u.Func.nArgs;
          fv.fPrompt = 0;
          fv.iftab = xlfIpmt;
          sFunctionLen += sizeof(FUNCVAR);
          fFuncVar = TRUE;
        break;
        case CALC_FUNC_IRR:
          fv.bToken = ptgFuncVarV;
          fv.cargs = (TCHAR)lpExpr->u.Func.nArgs;
          fv.fPrompt = 0;
          fv.iftab = xlfIrr;
          sFunctionLen += sizeof(FUNCVAR);
          fFuncVar = TRUE;
        break;
        case CALC_FUNC_NPV:
          fv.bToken = ptgFuncVarV;
          fv.cargs = (TCHAR)lpExpr->u.Func.nArgs;
          fv.fPrompt = 0;
          fv.iftab = xlfNpv;
          sFunctionLen += sizeof(FUNCVAR);
          fFuncVar = TRUE;
        break;
        case CALC_FUNC_PPMT:
          fv.bToken = ptgFuncVarV;
          fv.cargs = (TCHAR)lpExpr->u.Func.nArgs;
          fv.fPrompt = 0;
          fv.iftab = xlfPpmt;
          sFunctionLen += sizeof(FUNCVAR);
          fFuncVar = TRUE;
        break;
        case CALC_FUNC_SKEW:
          fv.bToken = ptgFuncVarV;
          fv.cargs = (TCHAR)lpExpr->u.Func.nArgs;
          fv.fPrompt = 0;
          fv.iftab = xlfSkew;
          sFunctionLen += sizeof(FUNCVAR);
          fFuncVar = TRUE;
        break;
        case CALC_FUNC_SUMIF:
          fv.bToken = ptgFuncVarV;
          fv.cargs = (TCHAR)lpExpr->u.Func.nArgs;
          fv.fPrompt = 0;
          fv.iftab = xlfSumif;
          sFunctionLen += sizeof(FUNCVAR);
          fFuncVar = TRUE;
        break;
        case CALC_FUNC_SUBTOTAL:
          fv.bToken = ptgFuncVarV;
          fv.cargs = (TCHAR)lpExpr->u.Func.nArgs;
          fv.fPrompt = 0;
          fv.iftab = xlfSubtotal;
          sFunctionLen += sizeof(FUNCVAR);
          fFuncVar = TRUE;
        break;
        case CALC_FUNC_SUMPRODUCT:
          fv.bToken = ptgFuncVarV;
          fv.cargs = (TCHAR)lpExpr->u.Func.nArgs;
          fv.fPrompt = 0;
          fv.iftab = xlfSumproduct;
          sFunctionLen += sizeof(FUNCVAR);
          fFuncVar = TRUE;
        break;
        case CALC_FUNC_VDB:
          fv.bToken = ptgFuncVarV;
          fv.cargs = (TCHAR)lpExpr->u.Func.nArgs;
          fv.fPrompt = 0;
          fv.iftab = xlfVdb;
          sFunctionLen += sizeof(FUNCVAR);
          fFuncVar = TRUE;
        break;

        //FuncV
        case CALC_FUNC_BINOMDIST:
          bToken = ptgFuncV;
          wToken = xlfBinomdist;
          sFunctionLen += sizeof(BYTE) + sizeof(WORD);
        break;
        case CALC_FUNC_CHIDIST:
          bToken = ptgFuncV;
          wToken = xlfChidist;
          sFunctionLen += sizeof(BYTE) + sizeof(WORD);
        break;
        case CALC_FUNC_CHIINV:
          bToken = ptgFuncV;
          wToken = xlfChiinv;
          sFunctionLen += sizeof(BYTE) + sizeof(WORD);
        break;
        case CALC_FUNC_CONFIDENCE:
          bToken = ptgFuncV;
          wToken = xlfConfidence;
          sFunctionLen += sizeof(BYTE) + sizeof(WORD);
        break;
        case CALC_FUNC_CRITBINOM:
          bToken = ptgFuncV;
          wToken = xlfCritbinom;
          sFunctionLen += sizeof(BYTE) + sizeof(WORD);
        break;
        case CALC_FUNC_EXPONDIST:
          bToken = ptgFuncV;
          wToken = xlfExpondist;
          sFunctionLen += sizeof(BYTE) + sizeof(WORD);
        break;
        case CALC_FUNC_FDIST:
          bToken = ptgFuncV;
          wToken = xlfFdist;
          sFunctionLen += sizeof(BYTE) + sizeof(WORD);
        break;
        case CALC_FUNC_FINV:
          bToken = ptgFuncV;
          wToken = xlfFinv;
          sFunctionLen += sizeof(BYTE) + sizeof(WORD);
        break;
        case CALC_FUNC_FISHER:
          bToken = ptgFuncV;
          wToken = xlfFisher;
          sFunctionLen += sizeof(BYTE) + sizeof(WORD);
        break;
        case CALC_FUNC_FISHERINV:
          bToken = ptgFuncV;
          wToken = xlfFisherinv;
          sFunctionLen += sizeof(BYTE) + sizeof(WORD);
        break;
        case CALC_FUNC_GAMMADIST:
          bToken = ptgFuncV;
          wToken = xlfGammadist;
          sFunctionLen += sizeof(BYTE) + sizeof(WORD);
        break;
        case CALC_FUNC_GAMMAINV:
          bToken = ptgFuncV;
          wToken = xlfGammainv;
          sFunctionLen += sizeof(BYTE) + sizeof(WORD);
        break;
        case CALC_FUNC_GAMMALN:
          bToken = ptgFuncV;
          wToken = xlfGammaln;
          sFunctionLen += sizeof(BYTE) + sizeof(WORD);
        break;
        case CALC_FUNC_HYPGEOMDIST:
          bToken = ptgFuncV;
          wToken = xlfHypgeomdist;
          sFunctionLen += sizeof(BYTE) + sizeof(WORD);
        break;
        case CALC_FUNC_ISPMT:
          bToken = ptgFuncV;
          wToken = xlfIspmt;
          sFunctionLen += sizeof(BYTE) + sizeof(WORD);
        break;
        case CALC_FUNC_LOGINV:
          bToken = ptgFuncV;
          wToken = xlfLoginv;
          sFunctionLen += sizeof(BYTE) + sizeof(WORD);
        break;
        case CALC_FUNC_LOGNORMDIST:
          bToken = ptgFuncV;
          wToken = xlfLognormdist;
          sFunctionLen += sizeof(BYTE) + sizeof(WORD);
        break;
        case CALC_FUNC_MIRR:
          bToken = ptgFuncV;
          wToken = xlfMirr;
          sFunctionLen += sizeof(BYTE) + sizeof(WORD);
        break;
        case CALC_FUNC_NEGBINOMDIST:
          bToken = ptgFuncV;
          wToken = xlfNegbinomdist;
          sFunctionLen += sizeof(BYTE) + sizeof(WORD);
        break;
        case CALC_FUNC_NORMDIST:
          bToken = ptgFuncV;
          wToken = xlfNormdist;
          sFunctionLen += sizeof(BYTE) + sizeof(WORD);
        break;
        case CALC_FUNC_NORMINV:
          bToken = ptgFuncV;
          wToken = xlfNorminv;
          sFunctionLen += sizeof(BYTE) + sizeof(WORD);
        break;
        case CALC_FUNC_NORMSDIST:
          bToken = ptgFuncV;
          wToken = xlfNormsdist;
          sFunctionLen += sizeof(BYTE) + sizeof(WORD);
        break;
        case CALC_FUNC_NORMSINV:
          bToken = ptgFuncV;
          wToken = xlfNormsinv;
          sFunctionLen += sizeof(BYTE) + sizeof(WORD);
        break;
        case CALC_FUNC_POISSON:
          bToken = ptgFuncV;
          wToken = xlfPoisson;
          sFunctionLen += sizeof(BYTE) + sizeof(WORD);
        break;
        case CALC_FUNC_STANDARDIZE:
          bToken = ptgFuncV;
          wToken = xlfStandardize;
          sFunctionLen += sizeof(BYTE) + sizeof(WORD);
        break;
        case CALC_FUNC_TDIST:
          bToken = ptgFuncV;
          wToken = xlfTdist;
          sFunctionLen += sizeof(BYTE) + sizeof(WORD);
        break;
        case CALC_FUNC_TINV:
          bToken = ptgFuncV;
          wToken = xlfTinv;
          sFunctionLen += sizeof(BYTE) + sizeof(WORD);
        break;
        case CALC_FUNC_WEIBULL:
          bToken = ptgFuncV;
          wToken = xlfWeibull;
          sFunctionLen += sizeof(BYTE) + sizeof(WORD);
        break;

#endif
        case CALC_FUNC_NULL:
        break;
        case CALC_FUNC_ABS:
          bToken = ptgFuncV;
          wToken = xlfAbs;
          sFunctionLen += sizeof(BYTE) + sizeof(WORD);
        break;
        case CALC_FUNC_ACOS:
          bToken = ptgFuncV;
          wToken = xlfAcos;
          sFunctionLen += sizeof(BYTE) + sizeof(WORD);
        break;
        case CALC_FUNC_ASIN:
          bToken = ptgFuncV;
          wToken = xlfAsin;
          sFunctionLen += sizeof(BYTE) + sizeof(WORD);
        break;
        case CALC_FUNC_ADD:
          bToken = ptgAdd;
          sFunctionLen += sizeof(BYTE);
        break;
        case CALC_FUNC_AND:
          fv.bToken = ptgFuncVarV;
          fv.cargs = (TCHAR)lpExpr->u.Func.nArgs;
          fv.fPrompt = 0;
          fv.iftab = xlfAnd;
          sFunctionLen = sizeof(FUNCVAR);
          fFuncVar = TRUE;
        break;
        case CALC_FUNC_ATAN:
          bToken = ptgFuncV;
          wToken = xlfAtan;
          sFunctionLen += sizeof(BYTE) + sizeof(WORD);
        break;
        case CALC_FUNC_ATAN2:
          fv.bToken = ptgFuncVarV;
          fv.cargs = (TCHAR)lpExpr->u.Func.nArgs;
          fv.fPrompt = 0;
          fv.iftab = xlfAtan2;
          sFunctionLen = sizeof(FUNCVAR);
          fFuncVar = TRUE;
        break;
        case CALC_FUNC_AVERAGE:
          fv.bToken = ptgFuncVarV;
          fv.cargs = (TCHAR)lpExpr->u.Func.nArgs;
          fv.fPrompt = 0;
          fv.iftab = xlfAverage;
          sFunctionLen += sizeof(FUNCVAR);
          fFuncVar = TRUE;
        break;
        case CALC_FUNC_CEILING:
          bToken = ptgFuncV;
          wToken = xlfCeiling;
          sFunctionLen += sizeof(BYTE) + sizeof(WORD);
        break;
        case CALC_FUNC_COMBIN:
          bToken = ptgFuncV;
          wToken = xlfCombin;
          sFunctionLen += sizeof(BYTE) + sizeof(WORD);
        break;
        case CALC_FUNC_COS:
          bToken = ptgFuncV;
          wToken = xlfCos;
          sFunctionLen += sizeof(BYTE) + sizeof(WORD);
        break;
        case CALC_FUNC_COSH:
          bToken = ptgFuncV;
          wToken = xlfCosh;
          sFunctionLen += sizeof(BYTE) + sizeof(WORD);
        break;
        case CALC_FUNC_DB:
          fv.bToken = ptgFuncVarV;
          fv.cargs = (TCHAR)lpExpr->u.Func.nArgs;
          fv.fPrompt = 0;
          fv.iftab = xlfDb;
          sFunctionLen = sizeof(FUNCVAR);
          fFuncVar = TRUE;
        break;
        case CALC_FUNC_DDB:
          fv.bToken = ptgFuncVarV;
          fv.cargs = (TCHAR)lpExpr->u.Func.nArgs;
          fv.fPrompt = 0;
          fv.iftab = xlfDdb;
          sFunctionLen = sizeof(FUNCVAR);
          fFuncVar = TRUE;
        break;
        case CALC_FUNC_DEGREES:
          bToken = ptgFuncV;
          wToken = xlfDegrees;
          sFunctionLen += sizeof(BYTE) + sizeof(WORD);
        break;
        case CALC_FUNC_EVEN:
          bToken = ptgFuncV;
          wToken = xlfEven;
          sFunctionLen = sizeof(BYTE) + sizeof(WORD);
        break;
        case CALC_FUNC_EXP:
          bToken = ptgFuncV;
          wToken = xlfExp;
          sFunctionLen += sizeof(BYTE) + sizeof(WORD);
        break;
        case CALC_FUNC_FACT:
          bToken = ptgFuncV;
          wToken = xlfFact;
          sFunctionLen += sizeof(BYTE) + sizeof(WORD);
        break;
        case CALC_FUNC_FALSE:
          bToken = ptgFuncV;
          wToken = xlfFalse;
          sFunctionLen = sizeof(BYTE) + sizeof(WORD);
        break;
        case CALC_FUNC_FLOOR:
          bToken = ptgFuncV;
          wToken = xlfFloor;
          sFunctionLen += sizeof(BYTE) + sizeof(WORD);
        break;
        case CALC_FUNC_IF:
        break;
        case CALC_FUNC_FV:
          fv.bToken = ptgFuncVarV;
          fv.cargs = (TCHAR)lpExpr->u.Func.nArgs;
          fv.fPrompt = 0;
          fv.iftab = xlfFv;
          sFunctionLen = sizeof(FUNCVAR);
          fFuncVar = TRUE;
        break;
        case CALC_FUNC_INT:
          bToken = ptgFuncV;
          wToken = xlfInt;
          sFunctionLen = sizeof(BYTE) + sizeof(WORD);
        break;
        case CALC_FUNC_INVERSE:
          bToken = ptgFuncV;
          wToken = xlfMinverse;
          sFunctionLen = sizeof(BYTE) + sizeof(WORD);
        break;
        case CALC_FUNC_ISEMPTY:
          bToken = ptgFuncV;
          wToken = xlfIsblank;
          sFunctionLen = sizeof(BYTE) + sizeof(WORD);
        break;
/*
            case CALC_FUNC_ISEVEN:
              bToken = ptgFuncV;
              wToken = xlfIseven;
              sFunctionLen = sizeof(BYTE) + sizeof(WORD);
            break;
            case CALC_FUNC_ISODD:
              bToken = ptgFuncV;
              wToken = xlfIsodd;
              sFunctionLen = sizeof(BYTE) + sizeof(WORD);
            break;
*/
        case CALC_FUNC_ISNUMBER:
          bToken = ptgFuncV;
          wToken = xlfIsnumber;
          sFunctionLen = sizeof(BYTE) + sizeof(WORD);
        break;
        case CALC_FUNC_ISTEXT:
          bToken = ptgFuncV;
          wToken = xlfIstext;
          sFunctionLen = sizeof(BYTE) + sizeof(WORD);
        break;
        case CALC_FUNC_ISNONTEXT:
          bToken = ptgFuncV;
          wToken = xlfIsnontext;
          sFunctionLen = sizeof(BYTE) + sizeof(WORD);
        break;
        case CALC_FUNC_ISREF:
          bToken = ptgFuncV;
          wToken = xlfIsref;
          sFunctionLen = sizeof(BYTE) + sizeof(WORD);
//          if (*lplFirstRefOffset != -1)
//          {
//            lpFormula = (LPBYTE)tbGlobalLock(*lpghFormula);
//            LPCELLREF8 lpCellRef = (LPCELLREF8)((LPBYTE)lpFormula + *lplFirstRefOffset);
//            lpCellRef->bToken = ptgRef;
//            *lplFirstRefOffset = -1;
//            tbGlobalUnlock(*lpghFormula);
//          }

          if (*lplRefOffsetCount > 0)
          {
            LPCELLREF8 lpCellRef = NULL;
            LPLONG lplOffset = (LPLONG)tbGlobalLock(*lpghRefOffset);

            lpFormula = (LPBYTE)tbGlobalLock(*lpghFormula);
            lpCellRef = (LPCELLREF8)((LPBYTE)lpFormula + lplOffset[0]);
            lpCellRef->bToken = ptgRef;
            *lplRefOffsetCount = 0;
            tbGlobalUnlock(*lpghRefOffset);
            tbGlobalFree(*lpghRefOffset);
            *lpghRefOffset = (TBGLOBALHANDLE)0;
            tbGlobalUnlock(*lpghFormula);
          }
          tbGlobalUnlock(*lpghRefOffset);
          
        break;
        case CALC_FUNC_LOG:
          fv.bToken = ptgFuncVarV;
          fv.cargs = (TCHAR)lpExpr->u.Func.nArgs;
          fv.fPrompt = 0;
          fv.iftab = xlfLog;
          sFunctionLen = sizeof(FUNCVAR);
          fFuncVar = TRUE;
        break;
        case CALC_FUNC_LOG10:
          bToken = ptgFuncV;
          wToken = xlfLog10;
          sFunctionLen += sizeof(BYTE) + sizeof(WORD);
        break;
        case CALC_FUNC_LN:
          bToken = ptgFuncV;
          wToken = xlfLn;
          sFunctionLen += sizeof(BYTE) + sizeof(WORD);
        break;
        case CALC_FUNC_MAX:
          fv.bToken = ptgFuncVarV;
          fv.cargs = (TCHAR)lpExpr->u.Func.nArgs;
          fv.fPrompt = 0;
          fv.iftab = xlfMax;
          sFunctionLen = sizeof(FUNCVAR);
          fFuncVar = TRUE;
        break;
        case CALC_FUNC_MEDIAN:
          fv.bToken = ptgFuncVarV;
          fv.cargs = (TCHAR)lpExpr->u.Func.nArgs;
          fv.fPrompt = 0;
          fv.iftab = xlfMedian;
          sFunctionLen = sizeof(FUNCVAR);
          fFuncVar = TRUE;
        break;
        case CALC_FUNC_MIN:
          fv.bToken = ptgFuncVarV;
          fv.cargs = (TCHAR)lpExpr->u.Func.nArgs;
          fv.fPrompt = 0;
          fv.iftab = xlfMin;
          sFunctionLen = sizeof(FUNCVAR);
          fFuncVar = TRUE;
        break;
        case CALC_FUNC_MOD:
          bToken = ptgFuncV;
          wToken = xlfMod;
          sFunctionLen += sizeof(BYTE) + sizeof(WORD);
        break;
        case CALC_FUNC_MODE:
          fv.bToken = ptgFuncVarV;
          fv.cargs = (TCHAR)lpExpr->u.Func.nArgs;
          fv.fPrompt = 0;
          fv.iftab = xlfMode;
          sFunctionLen = sizeof(FUNCVAR);
          fFuncVar = TRUE;
        break;
        case CALC_FUNC_NEG:
          bToken = ptgUminus;
          sFunctionLen += sizeof(BYTE);
        break;
        case CALC_FUNC_NOT:
          bToken = ptgFuncV;
          wToken = xlfNot;
          sFunctionLen += sizeof(BYTE) + sizeof(WORD);
        break;
        case CALC_FUNC_NPER:
          fv.bToken = ptgFuncVarV;
          fv.cargs = (TCHAR)lpExpr->u.Func.nArgs;
          fv.fPrompt = 0;
          fv.iftab = xlfNper;
          sFunctionLen = sizeof(FUNCVAR);
          fFuncVar = TRUE;
        break;
        case CALC_FUNC_ODD:
          bToken = ptgFuncV;
          wToken = xlfOdd;
          sFunctionLen = sizeof(BYTE) + sizeof(WORD);
        break;
        case CALC_FUNC_OR:
          fv.bToken = ptgFuncVarV;
          fv.cargs = (TCHAR)lpExpr->u.Func.nArgs;
          fv.fPrompt = 0;
          fv.iftab = xlfOr;
          sFunctionLen = sizeof(FUNCVAR);
          fFuncVar = TRUE;
        break;
        case CALC_FUNC_PERMUT:
          bToken = ptgFuncV;
          wToken = xlfPermut;
          sFunctionLen = sizeof(BYTE) + sizeof(WORD);
        break;
        case CALC_FUNC_PI:
          bToken = ptgFuncV;
          wToken = xlfPi;
          sFunctionLen = sizeof(BYTE) + sizeof(WORD);
        break;
        case CALC_FUNC_PMT:
        {
/*
              short        sPos = 0;
              short        sArgCnt = 0;
              short        asArgPos[4] = {0,0,0,0};
              LPBYTE       lpArgs = NULL;
              TBGLOBALHANDLE ghNewFormula = (TBGLOBALHANDLE)0;
              LPBYTE       lpNewFormula = NULL;
              double       dblVal = (double)0;
              double       dblTermsPerYear = (double)0.0;
              LPBYTE       lpExtra = NULL;

              fFuncVar = TRUE;
               
              fv.bToken = ptgFuncVarV;
              fv.cargs = (char)lpExpr->u.Func.nArgs-1;
              fv.fPrompt = 0;
              fv.iftab = xlfPmt;
              sFunctionLen = sizeof(FUNCVAR);

              while (sPos < *lpnFormulaPos)
              {
                asArgPos[sArgCnt] = sPos;
                sPos += sizeof(BYTE) + sizeof(double);
                sArgCnt++;
              } //while

              *lpnFormulaPos -= (sizeof(BYTE)+sizeof(double));
              *lplFormulaLen -= (sizeof(BYTE)+sizeof(double));
              
              //Allocate a new buffer and copy the modified formula
              Alloc(&ghNewFormula, *lpnFormulaPos);
              lpNewFormula = (LPBYTE)tbGlobalLock(ghNewFormula);
              memcpy(lpNewFormula, lpFormula, *lpnFormulaPos);

              memcpy(&dblVal, &(lpFormula[sizeof(BYTE)]), sizeof(double));
              memcpy(&dblTermsPerYear, &(lpFormula[asArgPos[3]+1]), sizeof(double));

              dblVal = dblVal/100/dblTermsPerYear; // rate/100 to get percentage
                                                   // rate/TermsPerYear to get rate/term
              xl_LogFile(lpSS->lpBook, LOG_PMTFUNCTIONSAVE, lpExpr->u.Cell.lCol-1, lpExpr->u.Cell.lRow-1, NULL);

              sPos = 0;
              lpNewFormula[sPos] = ptgNum;
              sPos += sizeof(BYTE);
              memcpy(lpNewFormula+sPos, &dblVal, sizeof(double)); //Rate
              sPos += sizeof(double);
              memcpy(lpNewFormula+sPos, &(lpFormula[asArgPos[2]]), sizeof(BYTE) + sizeof(double));//Payments
              sPos += sizeof(BYTE)+sizeof(double);
              memcpy(lpNewFormula+sPos, &(lpFormula[asArgPos[0]]), sizeof(BYTE) + sizeof(double));//Value
              sPos += sizeof(BYTE)+sizeof(double);

              sExtraLen = 2 * sizeof(BYTE);
              Alloc(&ghExtra, sExtraLen);
              lpExtra = (LPBYTE)tbGlobalLock(ghExtra);
              lpExtra[0] = ptgUminus;
              lpExtra[1] = ptgParen;
              tbGlobalUnlock(ghExtra);

              tbGlobalUnlock(ghNewFormula);
              tbGlobalUnlock(*lpghFormula);
              tbGlobalFree(*lpghFormula);
              *lpghFormula = ghNewFormula;
*/
          xl_LogFile(lpSS->lpBook, LOG_PMTFUNCTIONSAVE, lCol, lRow, NULL);
          return TRUE;
        }
        break;
        case CALC_FUNC_POWER:
          bToken = ptgFuncV;
          wToken = xlfPower;
          sFunctionLen = sizeof(BYTE) + sizeof(WORD);
        break;
        case CALC_FUNC_PRODUCT:
          fv.bToken = ptgFuncVarV;
          fv.cargs = (TCHAR)lpExpr->u.Func.nArgs;
          fv.fPrompt = 0;
          fv.iftab = xlfProduct;
          sFunctionLen = sizeof(FUNCVAR);
          fFuncVar = TRUE;
        break;
        case CALC_FUNC_PV:
          fv.bToken = ptgFuncVarV;
          fv.cargs = (TCHAR)lpExpr->u.Func.nArgs;
          fv.fPrompt = 0;
          fv.iftab = xlfPv;
          sFunctionLen = sizeof(FUNCVAR);
          fFuncVar = TRUE;
        break;
        case CALC_FUNC_RADIANS:
          bToken = ptgFuncV;
          wToken = xlfRadians;
          sFunctionLen += sizeof(BYTE) + sizeof(WORD);
        break;
        case CALC_FUNC_RAND:
          bToken = ptgFuncV;
          wToken = xlfRand;
          sFunctionLen = sizeof(BYTE) + sizeof(WORD);
        break;
        case CALC_FUNC_RANK:
          fv.bToken = ptgFuncVarV;
          fv.cargs = (TCHAR)lpExpr->u.Func.nArgs;
          fv.fPrompt = 0;
          fv.iftab = xlfRank;
          sFunctionLen = sizeof(FUNCVAR);
          fFuncVar = TRUE;
        break;
        case CALC_FUNC_ROUND:
          bToken = ptgFuncV;
          wToken = xlfRound;
          sFunctionLen += sizeof(BYTE) + sizeof(WORD);
        break;
        case CALC_FUNC_ROUNDUP:
          bToken = ptgFuncV;
          wToken = xlfRoundup;
          sFunctionLen += sizeof(BYTE) + sizeof(WORD);
        break;
        case CALC_FUNC_SIGN:
          bToken = ptgFuncV;
          wToken = xlfSign;
          sFunctionLen = sizeof(BYTE) + sizeof(WORD);
        break;
        case CALC_FUNC_SIN:
          bToken = ptgFuncV;
          wToken = xlfSin;
          sFunctionLen += sizeof(BYTE) + sizeof(WORD);
        break;
        case CALC_FUNC_SINH:
          bToken = ptgFuncV;
          wToken = xlfSinh;
          sFunctionLen += sizeof(BYTE) + sizeof(WORD);
        break;
        case CALC_FUNC_SLN:
          bToken = ptgFuncV;
          wToken = xlfSln;
          sFunctionLen += sizeof(BYTE) + sizeof(WORD);
        break;
        case CALC_FUNC_SQRT:
          bToken = ptgFuncV;
          wToken = xlfSqrt;
          sFunctionLen += sizeof(BYTE) + sizeof(WORD);
        break;
        case CALC_FUNC_SYD:
          bToken = ptgFuncV;
          wToken = xlfSyd;
          sFunctionLen += sizeof(BYTE) + sizeof(WORD);
        break;
        case CALC_FUNC_SQUARE:
        {
          double dbl = (double)2.0;
          bToken = ptgNum;
          *lplFormulaLen += sizeof(BYTE) + sizeof(double);
          Alloc(lpghFormula, *lplFormulaLen);
          lpFormula = (LPBYTE)tbGlobalLock(*lpghFormula);
          memcpy(lpFormula + *lpnFormulaPos, &bToken, sizeof(BYTE));
          *lpnFormulaPos += sizeof(BYTE);
          memcpy(lpFormula + *lpnFormulaPos, &dbl, sizeof(double));
          *lpnFormulaPos += sizeof(double);
          tbGlobalUnlock(*lpghFormula);
          //perform POWER
          bToken = ptgFuncV;
          wToken = xlfPower;
          sFunctionLen = sizeof(BYTE) + sizeof(WORD);
        }
        break;
        case CALC_FUNC_STDEV:
          fv.bToken = ptgFuncVarV;
          fv.cargs = (TCHAR)lpExpr->u.Func.nArgs;
          fv.fPrompt = 0;
          fv.iftab = xlfStdev;
          sFunctionLen = sizeof(FUNCVAR);
          fFuncVar = TRUE;
        break;
        case CALC_FUNC_STDEVP:
          fv.bToken = ptgFuncVarV;
          fv.cargs = (TCHAR)lpExpr->u.Func.nArgs;
          fv.fPrompt = 0;
          fv.iftab = xlfStdevp;
          sFunctionLen = sizeof(FUNCVAR);
          fFuncVar = TRUE;
        break;
        case CALC_FUNC_SUM:
          // If there is only 1 parameter, assume it is an AREA record,
          // use the "optimized SUM" provided through the ATTR tag/record.
          if (lpExpr->u.Func.nArgs == 1)
          {
            fAttr = TRUE;
            _fmemset((LPATTR)&Attr, 0, sizeof(ATTR));
            Attr.bToken = ptgAttr;
            Attr.bitFAttrSum = 1;
            sFunctionLen = sizeof(ATTR);
          }
          else
          {              
            fv.bToken = ptgFuncVarV;
            fv.cargs = (TCHAR)lpExpr->u.Func.nArgs;
            fv.fPrompt = 0;
            fv.iftab = xlfSum;
            sFunctionLen = sizeof(FUNCVAR);
            fFuncVar = TRUE;
          }
        break;
        case CALC_FUNC_SUMSQ:
          fv.bToken = ptgFuncVarV;
          fv.cargs = (TCHAR)lpExpr->u.Func.nArgs;
          fv.fPrompt = 0;
          fv.iftab = xlfSumsq;
          sFunctionLen = sizeof(FUNCVAR);
          fFuncVar = TRUE;
        break;
        case CALC_FUNC_TAN:
          bToken = ptgFuncV;
          wToken = xlfTan;
          sFunctionLen += sizeof(BYTE) + sizeof(WORD);
        break;
        case CALC_FUNC_TANH:
          bToken = ptgFuncV;
          wToken = xlfTanh;
          sFunctionLen += sizeof(BYTE) + sizeof(WORD);
        break;
        case CALC_FUNC_TRUE:
          bToken = ptgFuncV;
          wToken = xlfTrue;
          sFunctionLen = sizeof(BYTE) + sizeof(WORD);
        break;
        case CALC_FUNC_TRUNCATE:
          fv.bToken = ptgFuncVarV;
          fv.cargs = (TCHAR)lpExpr->u.Func.nArgs;
          fv.fPrompt = 0;
          fv.iftab = xlfTrunc;
          sFunctionLen = sizeof(FUNCVAR);
          fFuncVar = TRUE;
        break;
        case CALC_FUNC_URL:
        {
          short i = 0;

/*
              if (aStrings[0])
              {
                LPTSTR lptstr = (LPTSTR)tbGlobalLock(aStrings[0]);
                LPTSTR lptstrDest = NULL;
                Alloc(lpgh, (lstrlen(lptstr)+1)*sizeof(TCHAR));
                lptstrDest = (LPTSTR)tbGlobalLock(*lpgh);
                lstrcpy(lptstrDest, lptstr);
                tbGlobalUnlock(aStrings[0]);
                tbGlobalUnlock(lptstrDest);
              }
*/
          xl_LogFile(lpSS->lpBook, LOG_URLFUNCTIONNOTIFY, lCol, lRow, (LPVOID)aStrings[0]);
          while (aStrings[i])
          {
            tbGlobalFree(aStrings[i]);
            aStrings[i] = 0;
            i++;
          }
          *lplFormulaLen = 0;

          return TRUE;
        }
        break;
        case CALC_FUNC_VAR:
          fv.bToken = ptgFuncVarV;
          fv.cargs = (TCHAR)lpExpr->u.Func.nArgs;
          fv.fPrompt = 0;
          fv.iftab = xlfVar;
          sFunctionLen = sizeof(FUNCVAR);
          fFuncVar = TRUE;
        break;
        case CALC_FUNC_VARP:
          fv.bToken = ptgFuncVarV;
          fv.cargs = (TCHAR)lpExpr->u.Func.nArgs;
          fv.fPrompt = 0;
          fv.iftab = xlfVarp;
          sFunctionLen = sizeof(FUNCVAR);
          fFuncVar = TRUE;
        break;
        case CALC_FUNC_XROOT:
//            xl_LogFile(lpSS->lpBook, LOG_XROOTFUNCTIONSAVE, lCol, lRow, NULL);
              // XROOT(x,y) is the y-root of x, or x^1/y (where "^" represents exponent)
              // Excel does not support the XROOT() function, and at this point in the formula parsing
              // the 2 parameters (x & y) have already been added to the buffer.  Rather than back-tracking
              // through the buffer (which is necessary for the IF() function), we can use the MINVERSE() function
              // to convert the last parameter y to 1/y, then use the POWER operator.
          *lplFormulaLen += (sizeof(BYTE)+sizeof(WORD) + 2*sizeof(BYTE));
          Alloc(lpghFormula, *lplFormulaLen);
          lpFormula = (LPBYTE)tbGlobalLock(*lpghFormula);
          bToken = ptgFuncV;
          wToken = xlfMinverse;
          memcpy(lpFormula + *lpnFormulaPos, &bToken, sizeof(BYTE));
          *lpnFormulaPos += sizeof(BYTE);
          memcpy(lpFormula + *lpnFormulaPos, &wToken, sizeof(WORD));
          *lpnFormulaPos += sizeof(WORD);

          bToken = ptgPower;
          memcpy(lpFormula + *lpnFormulaPos, &bToken, sizeof(BYTE));
          *lpnFormulaPos += sizeof(BYTE);
          bToken = ptgParen;
          memcpy(lpFormula + *lpnFormulaPos, &bToken, sizeof(BYTE));
          *lpnFormulaPos += sizeof(BYTE);
              
          fAlreadyProcessed = TRUE;

/*
              if (ghFormula)
                tbGlobalFree(ghFormula);
              *lplFormulaLen = 0;
              CalcMemUnlock(hExpr);
              while (aStrings[i])
              {
                tbGlobalFree(aStrings[i]);
                aStrings[i] = 0;
                i++;
              }
*/
          break;
        break;
        case CALC_FUNC_DDELINK:
        //Not supported in Spread30...
/*
              if (ghFormula)
                tbGlobalFree(ghFormula);
              *lplFormulaLen = 0;
              CalcMemUnlock(hExpr);
              while (aStrings[i])
              {
                tbGlobalFree(aStrings[i]);
                aStrings[i] = 0;
                i++;
              }
*/
            return TRUE;
          break;

#ifdef SS_V35      
        case CALC_FUNC_ACOSH:
          bToken = ptgFuncV;
          wToken = xlfAcosh;
          sFunctionLen += sizeof(BYTE) + sizeof(WORD);
        break;
        case CALC_FUNC_ASINH:
          bToken = ptgFuncV;
          wToken = xlfAsinh;
          sFunctionLen += sizeof(BYTE) + sizeof(WORD);
        break;
        case CALC_FUNC_ATANH:
          bToken = ptgFuncV;
          wToken = xlfAtanh;
          sFunctionLen += sizeof(BYTE) + sizeof(WORD);
        break;
        case CALC_FUNC_CHAR:
          bToken = ptgFuncV;
          wToken = xlfChar;
          sFunctionLen += sizeof(BYTE) + sizeof(WORD);
        break;
        case CALC_FUNC_CLEAN:
          bToken = ptgFuncV;
          wToken = xlfClean;
          sFunctionLen += sizeof(BYTE) + sizeof(WORD);
        break;
        case CALC_FUNC_CODE:
          bToken = ptgFuncV;
          wToken = xlfCode;
          sFunctionLen += sizeof(BYTE) + sizeof(WORD);
        break;
        case CALC_FUNC_CONCATENATE:
          fv.bToken = ptgFuncVarV;
          fv.cargs = (TCHAR)lpExpr->u.Func.nArgs;
          fv.fPrompt = 0;
          fv.iftab = xlfConcatenate;
          sFunctionLen = sizeof(FUNCVAR);
          fFuncVar = TRUE;
        break;
        case CALC_FUNC_DATE:
          bToken = ptgFuncV;
          wToken = xlfDate;
          sFunctionLen += sizeof(BYTE) + sizeof(WORD);
        break;
        case CALC_FUNC_DAY:
          bToken = ptgFuncV;
          wToken = xlfDay;
          sFunctionLen += sizeof(BYTE) + sizeof(WORD);
        break;
        case CALC_FUNC_EXACT:
          bToken = ptgFuncV;
          wToken = xlfExact;
          sFunctionLen += sizeof(BYTE) + sizeof(WORD);
        break;
        case CALC_FUNC_FIND:
          fv.bToken = ptgFuncVarV;
          fv.cargs = (TCHAR)lpExpr->u.Func.nArgs;
          fv.fPrompt = 0;
          fv.iftab = xlfFind;
          sFunctionLen = sizeof(FUNCVAR);
          fFuncVar = TRUE;
        break;
        case CALC_FUNC_HOUR:
          bToken = ptgFuncV;
          wToken = xlfHour;
          sFunctionLen += sizeof(BYTE) + sizeof(WORD);
        break;
        case CALC_FUNC_ISBLANK:
          bToken = ptgFuncV;
          wToken = xlfIsblank;
          sFunctionLen += sizeof(BYTE) + sizeof(WORD);
        break;
        case CALC_FUNC_LEFT:
          fv.bToken = ptgFuncVarV;
          fv.cargs = (TCHAR)lpExpr->u.Func.nArgs;
          fv.fPrompt = 0;
          fv.iftab = xlfLeft;
          sFunctionLen = sizeof(FUNCVAR);
          fFuncVar = TRUE;
        break;
        case CALC_FUNC_LEN:
          bToken = ptgFuncV;
          wToken = xlfLen;
          sFunctionLen += sizeof(BYTE) + sizeof(WORD);
        break;
        case CALC_FUNC_LOWER:
          bToken = ptgFuncV;
          wToken = xlfLower;
          sFunctionLen += sizeof(BYTE) + sizeof(WORD);
        break;
        case CALC_FUNC_MID:
          bToken = ptgFuncV;
          wToken = xlfMid;
          sFunctionLen += sizeof(BYTE) + sizeof(WORD);
        break;
        case CALC_FUNC_MINUTE:
          bToken = ptgFuncV;
          wToken = xlfMinute;
          sFunctionLen += sizeof(BYTE) + sizeof(WORD);
        break;
        case CALC_FUNC_MONTH:
          bToken = ptgFuncV;
          wToken = xlfMonth;
          sFunctionLen += sizeof(BYTE) + sizeof(WORD);
        break;
        case CALC_FUNC_NOW:
          bToken = ptgFuncV;
          wToken = xlfNow;
          sFunctionLen += sizeof(BYTE) + sizeof(WORD);
        break;
        case CALC_FUNC_PROPER:
          bToken = ptgFuncV;
          wToken = xlfProper;
          sFunctionLen += sizeof(BYTE) + sizeof(WORD);
        break;
        case CALC_FUNC_REPLACE:
          bToken = ptgFuncV;
          wToken = xlfReplace;
          sFunctionLen += sizeof(BYTE) + sizeof(WORD);
        break;
        case CALC_FUNC_REPT:
          bToken = ptgFuncV;
          wToken = xlfRept;
          sFunctionLen += sizeof(BYTE) + sizeof(WORD);
        break;
        case CALC_FUNC_RIGHT:
          fv.bToken = ptgFuncVarV;
          fv.cargs = (TCHAR)lpExpr->u.Func.nArgs;
          fv.fPrompt = 0;
          fv.iftab = xlfRight;
          sFunctionLen = sizeof(FUNCVAR);
          fFuncVar = TRUE;
        break;
        case CALC_FUNC_ROUNDDOWN:
          bToken = ptgFuncV;
          wToken = xlfRounddown;
          sFunctionLen += sizeof(BYTE) + sizeof(WORD);
        break;
/*
        case CALC_FUNC_SEARCH:

          fv.bToken = ptgFuncVarV;
          fv.cargs = (TCHAR)lpExpr->u.Func.nArgs;
          fv.fPrompt = 0;
          fv.iftab = xlfSearch;
          sFunctionLen = sizeof(FUNCVAR);
          fFuncVar = TRUE;
        break;
*/
        case CALC_FUNC_SECOND:
          bToken = ptgFuncV;
          wToken = xlfSecond;
          sFunctionLen += sizeof(BYTE) + sizeof(WORD);
        break;
        case CALC_FUNC_SUBSTITUTE:
          fv.bToken = ptgFuncVarV;
          fv.cargs = (TCHAR)lpExpr->u.Func.nArgs;
          fv.fPrompt = 0;
          fv.iftab = xlfSubstitute;
          sFunctionLen = sizeof(FUNCVAR);
          fFuncVar = TRUE;
        break;
        case CALC_FUNC_TIME:
          bToken = ptgFuncV;
          wToken = xlfTime;
          sFunctionLen += sizeof(BYTE) + sizeof(WORD);
        break;
        case CALC_FUNC_TODAY:
          bToken = ptgFuncV;
          wToken = xlfToday;
          sFunctionLen += sizeof(BYTE) + sizeof(WORD);
        break;
        case CALC_FUNC_TRIM:
          bToken = ptgFuncV;
          wToken = xlfTrim;
          sFunctionLen += sizeof(BYTE) + sizeof(WORD);
        break;
        case CALC_FUNC_TRUNC:
          fv.bToken = ptgFuncVarV;
          fv.cargs = (TCHAR)lpExpr->u.Func.nArgs;
          fv.fPrompt = 0;
          fv.iftab = xlfTrunc;
          sFunctionLen = sizeof(FUNCVAR);
          fFuncVar = TRUE;
        break;
        case CALC_FUNC_UPPER:
          bToken = ptgFuncV;
          wToken = xlfUpper;
          sFunctionLen += sizeof(BYTE) + sizeof(WORD);
        break;
        case CALC_FUNC_WEEKDAY:
          fv.bToken = ptgFuncVarV;
          fv.cargs = (TCHAR)lpExpr->u.Func.nArgs;
          fv.fPrompt = 0;
          fv.iftab = xlfWeekday;
          sFunctionLen = sizeof(FUNCVAR);
          fFuncVar = TRUE;
        break;
        case CALC_FUNC_YEAR:
          bToken = ptgFuncV;
          wToken = xlfYear;
          sFunctionLen += sizeof(BYTE) + sizeof(WORD);
        break;
#endif
          default:            
/*
              if (ghFormula)
                tbGlobalFree(ghFormula);
              *lplFormulaLen = 0;
              CalcMemUnlock(hExpr);
              while (aStrings[i])
              {
                tbGlobalFree(aStrings[i]);
                aStrings[i] = 0;
                i++;
              }
*/
            return TRUE;
          }//switch

          if (!fAlreadyProcessed)
          {
            *lplFormulaLen += sFunctionLen + sExtraLen;
            Alloc(lpghFormula, *lplFormulaLen);
            lpFormula = (LPBYTE)tbGlobalLock(*lpghFormula);

            if (fFuncVar)
            {
              memcpy(lpFormula + *lpnFormulaPos, &fv, sizeof(FUNCVAR));
              if (sExtraLen)
              {
                LPBYTE lp = (LPBYTE)tbGlobalLock(ghExtra);
                memcpy(lpFormula+*lpnFormulaPos+sizeof(FUNCVAR), lp, sExtraLen);
                tbGlobalUnlock(ghExtra);
              }
            }
            else if (fAttr)
            {
              memcpy(lpFormula + *lpnFormulaPos, &Attr, sizeof(ATTR));
            }
            else if(lpExpr->nType == CALC_EXPR_FUNCTION && lpExpr->u.Func.nId == CALC_FUNC_IF)
              nIF--;
            else if (bToken == 0x13) //unary minus
              memcpy(lpFormula + *lpnFormulaPos, &bToken, sizeof(BYTE));
            else
            {
              memcpy(lpFormula + *lpnFormulaPos, &bToken, sizeof(BYTE));
              memcpy(lpFormula + *lpnFormulaPos + sizeof(BYTE), &wToken, sizeof(WORD));
            }

            *lpnFormulaPos = (short)*lplFormulaLen;
          }

          if (ghArgs != (TBGLOBALHANDLE)0)
          {
            tbGlobalUnlock(ghArgs);
            tbGlobalFree(ghArgs);
            ghArgs=0;
          }
          tbGlobalUnlock(*lpghFormula);
        
          i=0;
          while (aStrings[i])
          {
            tbGlobalFree(aStrings[i]);
            aStrings[i] = 0;
            i++;
          }
        }
          break;

        case CALC_EXPR_STRING:
        {
          long           lLen = 0;
          BYTE           bStringLen = 0;
          TBGLOBALHANDLE gh = NULL;
          LPBYTE         lpb = NULL;

          lpszText = (LPTSTR)CalcMemLock(lpExpr->u.String.hText);
          bToken = ptgStr;
          lLen = ssm_BuildBIFFStr(lpszText, &gh, FALSE);
          lpb = (LPBYTE)tbGlobalLock(gh);
          bStringLen = (byte)*lpb;
          *lplFormulaLen += lLen; 
          Alloc(lpghFormula, *lplFormulaLen);
          lpFormula = (LPBYTE)tbGlobalLock(*lpghFormula);
          memcpy(lpFormula + *lpnFormulaPos, &bToken, sizeof(BYTE));
          *lpnFormulaPos += sizeof(BYTE);
          memcpy(lpFormula + *lpnFormulaPos, &bStringLen, sizeof(BYTE));
          *lpnFormulaPos += sizeof(BYTE);
          memcpy(lpFormula + *lpnFormulaPos, lpb+sizeof(short), lLen-2);
          *lpnFormulaPos = (short)(*lpnFormulaPos + lLen-2);
          tbGlobalUnlock(gh);
          tbGlobalFree(gh);
          gh=0;
        }
          break;

        case CALC_EXPR_NAME:
        {
#ifdef SS_V70
          WORD           res = 0; 
          LPCALC_NAME    lpName;
          WORD           index = 0;
          
          lpName = (LPCALC_NAME)CalcMemLock(lpExpr->u.CustName.hName);
          lpszText = (LPTSTR)CalcMemLock(lpName->hText);
                    
//          int nLen = NameGetTextLen(lpExpr->u.CustName.hName);
//          int iResult = NameGetText(lpExpr->u.CustName.hName, lpszText, nLen);
//          if( 0 == iResult && nLen > 0 )
//            iResult = lstrlen(lstrcpy(lpszText, _T("?")));  //????
          
          bToken = ptgNameV;
          
          if (*lpghCustomNames != (TBGLOBALHANDLE)0)
          {
            LPBYTE lpb = (LPBYTE)tbGlobalLock(*lpghCustomNames);
            BOOL   bFound = FALSE;
            long   lPos = 0;
            while (lPos < *lplCustomNamesLen)
            {
              LPTSTR lpsz = (LPTSTR)(lpb + lPos);
              if (!_tcscmp(lpsz, lpszText))
              {
                bFound = true;
                break;
              }
              else
              {
                lPos += (lstrlen(lpsz)+1)*sizeof(TCHAR);
                index++;
              }
            }
            if (!bFound)
            {
              fErr = TRUE;
              break;
            }
          }
          else
          {
            fErr = TRUE;
            break;
          }
           
          index++;
          *lplFormulaLen += sizeof(BYTE) + 2 * sizeof(WORD);
          Alloc(lpghFormula, *lplFormulaLen);
          lpFormula = (LPBYTE)tbGlobalLock(*lpghFormula);
          memcpy(lpFormula + *lpnFormulaPos, &bToken, sizeof(BYTE));
          *lpnFormulaPos += sizeof(BYTE);
          memcpy(lpFormula + *lpnFormulaPos, &index, sizeof(WORD));
          *lpnFormulaPos += sizeof(WORD);
          memcpy(lpFormula + *lpnFormulaPos, &res, sizeof(WORD));
          *lpnFormulaPos += sizeof(WORD);
          tbGlobalUnlock(*lpghFormula);

          CalcMemUnlock(lpExpr->u.CustName.hName);
#else

          fErr = TRUE;
#endif
        }
          break;
/*
        case CALC_EXPR_CUSTFUNC:
          recCustFunc.nType = CALC_EXPR_CUSTFUNC;
          recCustFunc.nId = CustFuncGetSaveId(lpCalc,
                                              lpExpr->u.CustFunc.hFunc);
          recCustFunc.nArgs = lpExpr->u.CustFunc.nArgs;
          bRet &= SS_BuffAppend(lpBuff, &recCustFunc, sizeof(recCustFunc));
          break;
*/
        case CALC_EXPR_OPERATOR:
        {
          BOOL fProcessed = FALSE; 

          switch (lpExpr->u.Oper.nId)
          {
            case CALC_OPER_NULL:
              bToken = ptgAdd;
            break;
            case CALC_OPER_RANGE:
/*
              bToken = ptgRange;
*/
            {
              //Excel used the AREA record for ranges.
              //Combine the 2 previous Refs to create an Area record.
      
              LPCELLREF8 lpCellRef = NULL;
#ifdef SS_V70
              LPREF3D    lpRef3d = NULL;
#endif
              LPLONG     lplOffset = (LPLONG)tbGlobalLock(*lpghRefOffset);
                
          
              lpFormula = (LPBYTE)tbGlobalLock(*lpghFormula);

              //Get First row/col pair.
              lpCellRef = (LPCELLREF8)((LPBYTE)lpFormula + lplOffset[*lplRefOffsetCount-2]);
              
              if (lpCellRef->bToken == ptgRef)
              {
                _fmemset((LPAREA)&Area, 0, sizeof(AREA));
                Area.bToken = ptgArea;
                Area.rwFirst = lpCellRef->rw;
                Area.colFirst = lpCellRef->col;
                Area.fColRelFirst = lpCellRef->fColRel;
                Area.fRwRelFirst = lpCellRef->fRwRel;

                //Get Last row/col pair.
                lpCellRef = (LPCELLREF8)((LPBYTE)lpFormula + lplOffset[*lplRefOffsetCount-1]);

                Area.rwLast = lpCellRef->rw;
                Area.colLast = lpCellRef->col;
                Area.fColRelLast = lpCellRef->fColRel;
                Area.fRwRelLast = lpCellRef->fRwRel;

                tbGlobalUnlock(*lpghFormula);
                *lpnFormulaPos = (short)lplOffset[*lplRefOffsetCount-2];
                *lplFormulaLen = lplOffset[*lplRefOffsetCount-2];
                tbGlobalUnlock(*lpghRefOffset);
                *lplRefOffsetCount -= 2;
                if (*lplRefOffsetCount > 0)
                  Alloc(lpghRefOffset, *lplRefOffsetCount*sizeof(long));
                else
                {
                  tbGlobalFree(*lpghRefOffset);
                  *lpghRefOffset = (TBGLOBALHANDLE)0;
                }
                Alloc(lpghFormula, *lplFormulaLen + sizeof(AREA));
                lpFormula = (LPBYTE)tbGlobalLock(*lpghFormula);
                memcpy(lpFormula + *lpnFormulaPos, &Area, sizeof(AREA));
                *lpnFormulaPos += sizeof(AREA);
                *lplFormulaLen += sizeof(AREA);

                tbGlobalUnlock(*lpghFormula);
              }
//18876, 19593>
               else if (lpCellRef->bToken == ptgRef3d || lpCellRef->bToken == ptgRef3dV)
               {
#ifdef SS_V70
                _fmemset((LPAREA3D)&Area3d, 0, sizeof(AREA3D));

                lpRef3d = (LPREF3D)lpCellRef;
                Area3d.ixti = lpRef3d->ixti;
                Area3d.bToken = ptgArea3d;  
                Area3d.rwFirst = lpRef3d->rw;
                Area3d.colFirst = lpRef3d->col;
                Area3d.fColRelFirst = lpRef3d->fColRel;
                Area3d.fRwRelFirst = lpRef3d->fRwRel;

                //Get Last row/col pair.
                lpRef3d = (LPREF3D)((LPBYTE)lpFormula + lplOffset[*lplRefOffsetCount-1]);
                if (lpRef3d->ixti != Area3d.ixti)
                {
                  fErr = true;
                  break;
                }
//19593 >>
                if (lpRef3d->rw < Area3d.rwFirst)
                {
                  Area3d.rwLast = Area3d.rwFirst;
                  Area3d.rwFirst = lpRef3d->rw;
                }
                else
                  Area3d.rwLast = lpRef3d->rw;

                if (lpRef3d->col < Area3d.colFirst)
                {
                  Area3d.colLast = Area3d.colFirst;
                  Area3d.colFirst = lpRef3d->rw;
                }
                else
                  Area3d.colLast = lpRef3d->col;

//19693                Area3d.rwLast = lpRef3d->rw;
//                Area3d.colLast = lpRef3d->col;
//<< 19693
                Area3d.fColRelLast = lpRef3d->fColRel;
                Area3d.fRwRelLast = lpRef3d->fRwRel;

                tbGlobalUnlock(*lpghFormula);
                *lpnFormulaPos = (short)lplOffset[*lplRefOffsetCount-2];
                *lplFormulaLen = lplOffset[*lplRefOffsetCount-2];
                tbGlobalUnlock(*lpghRefOffset);
                *lplRefOffsetCount -= 2;
                if (*lplRefOffsetCount > 0)
                  Alloc(lpghRefOffset, *lplRefOffsetCount*sizeof(long));
                else
                {
                  tbGlobalFree(*lpghRefOffset);
                  *lpghRefOffset = (TBGLOBALHANDLE)0;
                }
                Alloc(lpghFormula, *lplFormulaLen + sizeof(AREA3D));
                lpFormula = (LPBYTE)tbGlobalLock(*lpghFormula);
                memcpy(lpFormula + *lpnFormulaPos, &Area3d, sizeof(AREA3D));
                *lpnFormulaPos += sizeof(AREA3D);
                *lplFormulaLen += sizeof(AREA3D);

                tbGlobalUnlock(*lpghFormula);
#endif
              }
//<<18876 - RAP
              fArea = TRUE;
            }
            break;
            case CALC_OPER_NEG:
              bToken = ptgUminus;
            break;
            case CALC_OPER_EXPON:
              bToken = ptgPower;
            break;
            case CALC_OPER_MULT:
              bToken = ptgMul;
            break;
            case CALC_OPER_DIV:
              bToken = ptgDiv;
            break;
            case CALC_OPER_ADD:
              bToken = ptgAdd;
            break;
            case CALC_OPER_SUB:
              bToken = ptgSub;
            break;
            case CALC_OPER_CONCAT:
              bToken = ptgConcat;
            break;
            case CALC_OPER_EQUAL:
              bToken = ptgEQ;
            break;
            case CALC_OPER_NOTEQUAL:
              bToken = ptgNE;
            break;
            case CALC_OPER_LESS:
              bToken = ptgLT;
            break;
            case CALC_OPER_GREATER:
              bToken = ptgGT;
            break;
            case CALC_OPER_LESSEQ:
              bToken = ptgLE;
            break;
            case CALC_OPER_GREATEREQ:
              bToken = ptgGE;
            break;
//RAP23a >>
            case CALC_OPER_AND:
            {
              CALC_EXPRINFO expr;
              memset(&expr, 0, sizeof(CALC_EXPRINFO));
/*
              fv.bToken = ptgFuncVarV;
              fv.cargs = 2;
              fv.fPrompt = 0;
              fv.iftab = xlfAnd;
              sFunctionLen = sizeof(FUNCVAR);
              fFuncVar = TRUE;
*/
              expr.nType = CALC_EXPR_FUNCTION;
              expr.u.Func.nId = CALC_FUNC_AND;
              expr.u.Func.nArgs = 2;
              ssm_AddExprToFormulaBuffer(lpSS, lCol, lRow, &expr, lpghFormula, lplFormulaLen, lpnFormulaPos, fParen, aStrings, lpxtiCount, lpghXTI, lpghRefOffset, lplRefOffsetCount, lpghCustomNames, lplCustomNamesLen);
              fProcessed = TRUE;
            }
            break;
            case CALC_OPER_OR:
            {
              CALC_EXPRINFO expr;
             
              memset(&expr, 0, sizeof(CALC_EXPRINFO));
              expr.nType = CALC_EXPR_FUNCTION;
              expr.u.Func.nId = CALC_FUNC_OR;
              expr.u.Func.nArgs = 2;
              ssm_AddExprToFormulaBuffer(lpSS, lCol, lRow, &expr, lpghFormula, lplFormulaLen, lpnFormulaPos, fParen, aStrings, lpxtiCount, lpghXTI, lpghRefOffset, lplRefOffsetCount, lpghCustomNames, lplCustomNamesLen);
              fProcessed = TRUE;
            }  
            break;
            case CALC_OPER_NOT:
            {
              CALC_EXPRINFO expr;
             
              memset(&expr, 0, sizeof(CALC_EXPRINFO));
              expr.nType = CALC_EXPR_FUNCTION;
              expr.u.Func.nId = CALC_FUNC_NOT;
              ssm_AddExprToFormulaBuffer(lpSS, lCol, lRow, &expr, lpghFormula, lplFormulaLen, lpnFormulaPos, fParen, aStrings, lpxtiCount, lpghXTI, lpghRefOffset, lplRefOffsetCount, lpghCustomNames, lplCustomNamesLen);
              fProcessed = TRUE;
            }  

            break;
//<< RAP23a
          } //switch

          if (!fArea && !fProcessed) //RAP23c
          {
            // if its NOT an AREA record, treat it like a normal Operation Token.
            *lplFormulaLen += sizeof(BYTE); // 1 byte for the operator
            Alloc(lpghFormula, *lplFormulaLen);
            lpFormula = (LPBYTE)tbGlobalLock(*lpghFormula);
            memcpy(lpFormula + *lpnFormulaPos, &bToken, sizeof(BYTE));
//<< RAP23a
            *lpnFormulaPos += sizeof(BYTE);

            tbGlobalUnlock(*lpghFormula);

            short refCount = 2;
            if (lpExpr->u.Oper.nId == CALC_OPER_NEG)
              refCount = 1;

            if (*lplRefOffsetCount != 0 && *lplRefOffsetCount >= refCount)
            {
              LPLONG lplOffset = (LPLONG)tbGlobalLock(*lpghRefOffset);
              int i;
              for (i=*lplRefOffsetCount-1; i>=*lplRefOffsetCount-refCount; i--)
                lplOffset[i] = -1;

              tbGlobalUnlock(*lpghRefOffset);
            }

          }
// RAP23a>>
          if (fProcessed)
            fProcessed = FALSE;
//<< RAP23a
        } 
          break;
        case CALC_EXPR_ERROR:
        {
          BYTE err = 0;
          bToken = ptgErr;
          *lplFormulaLen += sizeof(BYTE) + sizeof(BYTE);
          Alloc(lpghFormula, *lplFormulaLen);
          lpFormula = (LPBYTE)tbGlobalLock(*lpghFormula);
          memcpy(lpFormula + *lpnFormulaPos, &bToken, sizeof(BYTE));
          switch (lpExpr->u.Error.nId)
          {
            case CALC_ERROR_DIV0:
              err = 7;
              break;
            case CALC_ERROR_NA:
              err = 42;
              break;
            case CALC_ERROR_NAME:
              err = 29;
              break;
            case CALC_ERROR_NULL:
              err = 0;
              break;
            case CALC_ERROR_NUM:
              err = 36;
              break;
            case CALC_ERROR_REF:
              err = 23;
              break;
            case CALC_ERROR_VALUE:
              err = 15;
              break;
          }
          *lpnFormulaPos += sizeof(BYTE);
          memcpy(lpFormula + *lpnFormulaPos, &err, sizeof(BYTE));
          *lpnFormulaPos += sizeof(BYTE);
          tbGlobalUnlock(*lpghFormula);
        }
          break;
        case CALC_EXPR_SEPARATOR:
          if (lpExpr->u.Sep.nId == CALC_SEP_RPAR)
          {
            *lplFormulaLen += sizeof(BYTE);
            Alloc(lpghFormula, *lplFormulaLen);
            lpFormula = (LPBYTE)tbGlobalLock(*lpghFormula);
            bToken = ptgParen;
            memcpy(lpFormula + *lpnFormulaPos, &bToken, sizeof(BYTE));
            *lpnFormulaPos += sizeof(BYTE);
            tbGlobalUnlock(*lpghFormula);
            *fParen = FALSE;
          }
          break;
      
      }
      if (fErr)
        return TRUE;

  return FALSE;
}

/***********************************************************************
* Name:   ssm_DumpIFToBuffer
*
* Return: long - Success or Failure
***********************************************************************/
long ssm_DumpIFToBuffer(LPSPREADSHEET lpSS, SS_COORD lCol, SS_COORD lRow, short nWhichIF, 
                        LPCALC_EXPRINFO *plpExpr,
#if defined(_WIN64) || defined(_IA64)
						LONG_PTR pIFNode[3][50],
#else
						long pIFNode[3][50], 
#endif
						TBGLOBALHANDLE *lpghFormula, 
                        long *lplFormulaLen, short *lpnFormulaPos, BOOL *fParen, TBGLOBALHANDLE aStrings[100], 
                        LPSHORT lpxtiCount, LPTBGLOBALHANDLE lpghXTI, LPTBGLOBALHANDLE lpghCustomNames, LPLONG lplCustomNamesLen) 
{
  LPBYTE lpFormula = NULL;
  short  nCurrentArg = 0;
  short  nWhichArg = 0;
  short  nNestedIF = 0;
  LPCALC_EXPRINFO lpExpr = *plpExpr;
  TBGLOBALHANDLE ghRefOffset = (TBGLOBALHANDLE)0;
  long           lRefOffsetCount = 0;

  while ( lpExpr->nType != CALC_EXPR_END )
  {
    for (nWhichArg=0; nWhichArg<3; nWhichArg++)
    {
#if defined(_WIN64) || defined(_IA64)
      if ((LONG_PTR)lpExpr == pIFNode[nWhichArg][nWhichIF])
#else
      if ((long)lpExpr == pIFNode[nWhichArg][nWhichIF])
#endif
        break;
    }
    if (nWhichArg<3)
    {
      // This expression is the beginning of an IF parameter
      if (nWhichArg == 0)
      {
        // First IF param
      }
      else if (nWhichArg == 0 || nWhichArg == nCurrentArg +1)
      {
        // OK, this makes sense
        nCurrentArg++;
      }
      else
      {
        //not sure why???
      }
    }
    else if ((lpExpr->nType == CALC_EXPR_FUNCTION) && (lpExpr->u.Func.nId == CALC_FUNC_IF))
    {
      // This is the IF function expression.  We are now at the end of this IF function.  
      // Write out the necessary records and return.
      FUNCVAR fv;
      LPATTR  lpAttr;
      
      memset((LPFUNCVAR)&fv, 0, sizeof(FUNCVAR));
      if (pIFNode[2][nWhichIF] != 0) // if x, then y, else z
      {
        ATTR Attr;
        memset((LPATTR)&Attr, 0, sizeof(ATTR));
        Attr.bToken = ptgAttr;
        Attr.bitFAttrGoto = 1;
        Attr.wData = 3;
  
        *lplFormulaLen += sizeof(ATTR);
        Alloc(lpghFormula, *lplFormulaLen);
        lpFormula = (LPBYTE)tbGlobalLock(*lpghFormula);
        memcpy(lpFormula + *lpnFormulaPos, &Attr, sizeof(ATTR));
        tbGlobalUnlock(*lpghFormula);

        *lpnFormulaPos = (short)*lplFormulaLen;
      
        fv.cargs = (TCHAR)3;
      }
      else // if x, then y, (NO else z)
        fv.cargs = (TCHAR)2;
    
      fv.bToken = ptgFuncVarV;
      fv.fPrompt = 0;
      fv.iftab = 1;
      *lplFormulaLen += sizeof(FUNCVAR);
      Alloc(lpghFormula, *lplFormulaLen);
      lpFormula = (LPBYTE)tbGlobalLock(*lpghFormula);
      memcpy(lpFormula + *lpnFormulaPos, &fv, sizeof(FUNCVAR));
              
      lpAttr = (LPATTR)((LPBYTE)lpFormula + pIFNode[2][nWhichIF]);
      lpAttr->wData = (WORD)(*lplFormulaLen-pIFNode[2][nWhichIF]-sizeof(ATTR)-1); ////RAP18a - The "-1" was added to comply with Excel.
//!!!NOTE: From inspection, I have determined that Excel writes out If(a,b,c) as 3 ptgATTR's 
//         followed by a ptgFUNCVARV.  The first ATTR has the "if" flag set and is the "TRUE" ATTR.
//         Also, its offset (wData value) is set to point to the beginning of the next ATTR, 
//         the "FALSE" ATTR. This offset is determined by counting from the beginning of the
//         "TRUE" ATTR to the beginning of the "FALSE" ATTR.  The "FALSE" ATTR has the "goto" 
//         flag set, and its offset is set to the beginning of the FUNCVARV.  This offset is 
//         determined from counting form the beginning of the "FALSE" ATTR to the beginning 
//         of the FUNCVAR - 1.  The last ATTR occurs after the "FALSE" ATTR and has the "goto" 
//         flag set and its offset is computed by counting from the beginning of this ATTR to 
//         the beginning of the FUNCVAR - 1.  The "landing point" for the 2 offsets that have 
//         the -1 is a NULL byte.  It appears that Excel stores all IFs with a NULL terminator
//         that is not counted when evaluating formulas.  Go figure...

//RAP16d      lpAttr->wData = (WORD)(*lplFormulaLen-pIFNode[2][nWhichIF2]-sizeof(ATTR));
//RAP18d      lpAttr->wData = (WORD)(*lplFormulaLen-pIFNode[2][nWhichIF2]-sizeof(ATTR)-sizeof(FUNCVAR)); //RAP16a

      *lpnFormulaPos = (short)*lplFormulaLen;

      // The IF() has been processed. Return...
      break;
    }
    else
    {
      // This expression is NOT the beginning of an IF parameter, nor is it the IF function expression.
      // This means it is part of the current IF parameter and needs to be added to the formula buffer.

      // Check to see if this expression is also the first parameter of a nested IF()
      for (nNestedIF=0; nNestedIF<nWhichIF; nNestedIF++)
      {
#if defined(_WIN64) || defined(_IA64)
        if ((LONG_PTR)lpExpr == pIFNode[0][nNestedIF])
#else
        if ((long)lpExpr == pIFNode[0][nNestedIF])
#endif
          break;
      }
      if (nNestedIF < nWhichIF)
      {  
        //This is a nested IF -- call recursively to write out the nested IF to the buffer.  Then
        //return and continue processing this IF.
        if (ssm_DumpIFToBuffer(lpSS, lCol, lRow, nNestedIF, &lpExpr, pIFNode, lpghFormula, lplFormulaLen, lpnFormulaPos, fParen, aStrings, lpxtiCount, lpghXTI, lpghCustomNames, lplCustomNamesLen))
          return TRUE;
      }
      else
      {
        if (ssm_AddExprToFormulaBuffer(lpSS, lCol, lRow, lpExpr, lpghFormula, lplFormulaLen, lpnFormulaPos, fParen, aStrings, lpxtiCount, lpghXTI, &ghRefOffset, &lRefOffsetCount, lpghCustomNames, lplCustomNamesLen))
          return (TRUE);
      }

      lpExpr++;
        
      continue;
    }

    if (nCurrentArg == 0)
    {
      //found beginning of first param of IF
      //This is the condition.
    }
    else if (nCurrentArg == 1)
    { 
      //found beginning of second param of IF
      //This is the TRUE result, add an if ATTR 
      //to jump to the else location.
      ATTR Attr;
      _fmemset((LPATTR)&Attr, 0, sizeof(ATTR));
      Attr.bToken = ptgAttr;
      Attr.bitFAttrIf = 1;
      Attr.wData = 0;
      *lplFormulaLen += sizeof(ATTR);
      Alloc(lpghFormula, *lplFormulaLen);
      lpFormula = (LPBYTE)tbGlobalLock(*lpghFormula);
      memcpy(lpFormula + *lpnFormulaPos, &Attr, sizeof(ATTR));
      tbGlobalUnlock(*lpghFormula);
      pIFNode[1][nWhichIF] = *lpnFormulaPos;
  
      *lpnFormulaPos = (short)*lplFormulaLen;
    } 
    else if (nCurrentArg == 2)
    {
      //found beginning of third param of IF
      //This is the FALSE result, add a goto ATTR
      LPATTR  lpAttr;

      if (pIFNode[2][nWhichIF] != 0) // if x, then y, else z
      {
        ATTR Attr;
        memset((LPATTR)&Attr, 0, sizeof(ATTR));
        Attr.bToken = ptgAttr;
        Attr.bitFAttrGoto = 1;
        Attr.wData = 0;

        *lplFormulaLen += sizeof(ATTR);
        Alloc(lpghFormula, *lplFormulaLen);
        lpFormula = (LPBYTE)tbGlobalLock(*lpghFormula);
        memcpy(lpFormula + *lpnFormulaPos, &Attr, sizeof(ATTR));
        tbGlobalUnlock(*lpghFormula);
        pIFNode[2][nWhichIF] = *lpnFormulaPos;

        *lpnFormulaPos = (short)*lplFormulaLen;
      }
      lpFormula = (LPBYTE)tbGlobalLock(*lpghFormula);
      lpAttr = (LPATTR)((LPBYTE)lpFormula + pIFNode[1][nWhichIF]);
      lpAttr->wData = (WORD)(*lplFormulaLen-pIFNode[1][nWhichIF]-sizeof(ATTR));
      tbGlobalUnlock(*lpghFormula);
    }

    // Check to see if this expression is also the first parameter of a nested IF()
    for (nNestedIF=0; nNestedIF<nWhichIF; nNestedIF++)
    {
#if defined(_WIN64) || defined(_IA64)
      if ((LONG_PTR)lpExpr == pIFNode[0][nNestedIF])
#else
      if ((long)lpExpr == pIFNode[0][nNestedIF])
#endif
        break;
    }
    if (nNestedIF < nWhichIF)
    {  
      //This is a nested IF -- call recursively to write out the nested IF to the buffer.  Then
      //return and continue processing this IF.
      if (ssm_DumpIFToBuffer(lpSS, lCol, lRow, nNestedIF, &lpExpr, pIFNode, lpghFormula, lplFormulaLen, lpnFormulaPos, fParen, aStrings, lpxtiCount, lpghXTI, lpghCustomNames, lplCustomNamesLen))
        return TRUE;
    }
    else
    {
      // Add the expression to the formula buffer.
      if (ssm_AddExprToFormulaBuffer(lpSS, lCol, lRow, lpExpr, lpghFormula, lplFormulaLen, lpnFormulaPos, fParen, aStrings, lpxtiCount, lpghXTI, &ghRefOffset, &lRefOffsetCount, lpghCustomNames, lplCustomNamesLen))
        return (TRUE);
    }

    lpExpr++;
  }

  *plpExpr = lpExpr;

  return S_OK;
}


/***********************************************************************
* Name:   ssm_BuildFormulaBuffer
*
* Return: long - Success or Failure
***********************************************************************/
long ssm_BuildFormulaBuffer(LPSPREADSHEET lpSS, SS_COORD lCol, SS_COORD lRow, LPCALC_EXPRINFO *plpExpr, 
                            short nIF,
#if defined(_WIN64) || defined(_IA64)
							LONG_PTR pIFNode[3][50], 
#else
							long pIFNode[3][50], 
#endif
							TBGLOBALHANDLE *lpghFormula, 
                            long *lplFormulaLen, short *lpnFormulaPos, TBGLOBALHANDLE aStrings[100],
                            LPSHORT lpxtiCount, LPTBGLOBALHANDLE lpghXTI, LPTBGLOBALHANDLE lpghCustomNames, LPLONG lplCustomNamesLen)
{
  LPCALC_EXPRINFO lpExpr = *plpExpr;
  short           nWhichIF;  
  BOOL            fParen = FALSE;
  TBGLOBALHANDLE  ghRefOffset = (TBGLOBALHANDLE)0;
  long            lRefOffsetCount = 0;

  while ( lpExpr->nType != CALC_EXPR_END )
  {
    // if IF() function(s) exist, check to see if we have encountered the first param of an IF()
    if (nIF)
    {
      // Search the array of 1st params of IF()s to see if there is a match for the current expression.
      for (nWhichIF=0; nWhichIF<nIF; nWhichIF++)
      {
        if (pIFNode[0][nWhichIF] == 0)
        {
          nWhichIF = 50;  
          break;
        }
#if defined(_WIN64) || defined(_IA64)
       if ((LONG_PTR)lpExpr == pIFNode[0][nWhichIF])
#else
       if ((long)lpExpr == pIFNode[0][nWhichIF])
#endif
          break;
      }

      if (nWhichIF < nIF) //found an IF param
      {
        // If an IF() function was parsed in the previous section, now is the
        // time to dump the parsed info into the output buffer.    
  
        // This function  recursively converts IF() functions within a formula.
        if (ssm_DumpIFToBuffer(lpSS, lCol, lRow, nWhichIF, &lpExpr, pIFNode, lpghFormula, lplFormulaLen, lpnFormulaPos, &fParen, aStrings, lpxtiCount, lpghXTI, lpghCustomNames, lplCustomNamesLen))
        {
          // if an error was encountered, return (TRUE).
          if (ghRefOffset != 0)
            tbGlobalFree(ghRefOffset);
          return (TRUE);
        }
      }
    }

    if (ssm_AddExprToFormulaBuffer(lpSS, lCol, lRow, lpExpr, lpghFormula, lplFormulaLen, lpnFormulaPos, &fParen, aStrings, lpxtiCount, lpghXTI, &ghRefOffset, &lRefOffsetCount, lpghCustomNames, lplCustomNamesLen))
    {
      if (ghRefOffset != 0)
        tbGlobalFree(ghRefOffset);
      return (TRUE);
    }

    lpExpr++;
  }

  if (ghRefOffset != 0)
    tbGlobalFree(ghRefOffset);

  return FALSE;
}

/***********************************************************************
* Name:   ssm_Save4Expr - Evaluate the formula stack and build an Excel
*                         BIFF representation of the formula.
*
* Usage:  long ssm_Save4Expr(LPSPREADSHEET lpSS, SS_COORD lCol, SS_COORD lRow,
*                            CALC_HANDLE hExpr,
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
long ssm_Save4Expr(LPSPREADSHEET lpSS, SS_COORD lCol, SS_COORD lRow, CALC_HANDLE hExpr, LPTBGLOBALHANDLE lpgh, LPLONG lplen, LPSHORT lpxtiCount, LPTBGLOBALHANDLE lpghXTI, LPTBGLOBALHANDLE lpghCustomNames, LPLONG lplCustomNamesLen)
{
  TBGLOBALHANDLE ghFormula = (TBGLOBALHANDLE)0;
  long         lFormulaLen = 0;
  short        sFormulaPos = 0;
  LPCALC_EXPRINFO lpExpr = NULL;
  LPCALC_EXPRINFO lpCurrPos = NULL;
  LPCALC_EXPRINFO lpBeginning = NULL;
  TBGLOBALHANDLE aStrings[100];
#if defined(_WIN64) || defined(_IA64)
  LONG_PTR     alIFNode[3][50]; //set an artificial max of 50 IF() functions in a formula
#else
  long         alIFNode[3][50]; //set an artificial max of 50 IF() functions in a formula
#endif
  short        nIFPos = 0;
  int          nIFNode = 2;
  int          nArgCount = 1;
  short        i = 0;
  short        nIF = 0;
  BOOL         bRet = FALSE;
  
  memset(aStrings, 0, 100*sizeof(TBGLOBALHANDLE));
#if defined(_WIN64) || defined(_IA64)
  memset(alIFNode, 0, 50*3*sizeof(LONG_PTR));
#else
  memset(alIFNode, 0, 50*3*sizeof(long));
#endif

  if (hExpr)
    lpExpr = (LPCALC_EXPRINFO)CalcMemLock(hExpr);

  if (lpExpr)
  {
    lpBeginning = lpExpr;

    // Excel stores the IF() function differently than Spread.  Therefore, we need to
    // go through Spread's formula buffer looking for IF() functions. When a IF() function
    // is found, backtrack through the buffer and save pointers to the buffer position for
    // the beginning of each of the IF() function parameters. A limit of 50 IF() functions
    // per formula has been set.  
    while ( lpExpr->nType != CALC_EXPR_END )
    {
      if ((lpExpr->nType == CALC_EXPR_FUNCTION) && (lpExpr->u.Func.nId == CALC_FUNC_IF))
      {
        short nIfPopCount = 0; // number of If's popped off of the alIfNode array. (a decrement to nIf)
        lpCurrPos = lpExpr; //Save a pointer to where the lookback begins.
        nIF++; // increment the IF count

        nIFNode = lpExpr->u.Func.nArgs-1; //the IF function can have 2 or 3 params.

        lpExpr--; //back-up to the top of the parameter stack so the IF record does not get
                  //re-evaluated.
        
        // Backtrack through the parameter list determining the number of arguements for each
        // expression, and saving the buffer position when the arguement count == 0, which
        // indicates the beginning of a parameter.
        while (lpExpr > lpBeginning)
        { 
          if (lpExpr->nType == CALC_EXPR_FUNCTION)
          {
//RAP24a >>
            if (lpExpr->u.Func.nId == CALC_FUNC_IF) // time to go spelunking...
            {
              lpExpr = (LPCALC_EXPRINFO)alIFNode[0][nIF-2-nIfPopCount++]; // -2 because nIF has already been  
            }                                                             // incremented, but not added.    
            else
//<< RAP24a
              //determine the number of arguements
              nArgCount += lpExpr->u.Func.nArgs;
          }
          else if (lpExpr->nType == CALC_EXPR_OPERATOR)
          {
            //determine the number of arguements
            switch (lpExpr->u.Oper.nId)
            {
              case CALC_OPER_NULL:
//              case CALC_OPER_RANGE:
              case CALC_OPER_NEG:
              case CALC_OPER_NOT:
                nArgCount++;
                break;
              case CALC_OPER_RANGE:
              case CALC_OPER_EXPON:
              case CALC_OPER_MULT:
              case CALC_OPER_DIV:
              case CALC_OPER_ADD:
              case CALC_OPER_SUB:
              case CALC_OPER_EQUAL:
              case CALC_OPER_NOTEQUAL:
              case CALC_OPER_LESS:
              case CALC_OPER_GREATER:
              case CALC_OPER_LESSEQ:
              case CALC_OPER_GREATEREQ:
              case CALC_OPER_AND:
              case CALC_OPER_OR:
                nArgCount+=2;
                break;
              case CALC_OPER_CONCAT:
                break;
            } 
          }
//          else if (lpExpr->nType == CALC_EXPR_ERROR)
//            nArgCount++;
          else if (lpExpr->nType == CALC_EXPR_SEPARATOR) 
            nArgCount++;

          nArgCount--;
          if (nArgCount == 0) //set node pointer
          {
#if defined(_WIN64) || defined(_IA64)
            alIFNode[nIFNode][nIFPos] = (LONG_PTR)lpExpr;
#else
            alIFNode[nIFNode][nIFPos] = (long)lpExpr;
#endif
            nIFNode--;
            nArgCount = 1;
            if (nIFNode == -1)
            {
              nIFPos++;
              if (nIFPos > 49)
              {
                // crossed our limit of 50 IF()'s per formula
                if (ghFormula)
                {
                  tbGlobalFree(ghFormula);
                  ghFormula=0;
                }
                lFormulaLen = 0;
                CalcMemUnlock(hExpr);
                while (aStrings[i])
                {
                  tbGlobalFree(aStrings[i]);
                  aStrings[i] = 0;
                  i++;
                }          
                return (TRUE);
              }
              nIFNode = 2;
              lpExpr = lpCurrPos;
              break;
            }
          }
          lpExpr--;
        } 
      }
      lpExpr++;
    }  

    lpExpr = lpBeginning;  // Reset pointer to beginning of Formula buffer.

    //Step through Spread's formula buffer, checking for the beginning of IF() functions
    //and processing them. Otherwise, process the expression normally.
    if (ssm_BuildFormulaBuffer(lpSS, lCol, lRow, &lpExpr, nIF, alIFNode, &ghFormula, &lFormulaLen, 
                               &sFormulaPos, aStrings, lpxtiCount, lpghXTI, lpghCustomNames, lplCustomNamesLen))
    {
      if (ghFormula)
      {
        tbGlobalFree(ghFormula);
        ghFormula = 0;
      }
      lFormulaLen = 0;
      bRet = 1;
    }

    CalcMemUnlock(hExpr);
  
    i=0;
    while (aStrings[i])
    {
      tbGlobalFree(aStrings[i]);
      aStrings[i] = 0;
      i++;
    }
  }

  *lpgh = ghFormula;
  *lplen = lFormulaLen;

  return bRet;
}


/***********************************************************************
* Name:   ssm_DumpBuffer - Dump the buffer to the stream.
*
*
* Usage:  long ssm_DumpBuffer(IStream *pstm, TBGLOBALHANDLE gh, LPLONG lplLen)
*           pstm - pointer to the stream
*           gh - buffer
*           lplen - length of the buffer
*
* Return: long - Success or Failure
***********************************************************************/
long ssm_DumpBuffer(IStream *pstm, TBGLOBALHANDLE gh, LPLONG lplLen)
{
  LPVOID  lpData = (LPVOID)tbGlobalLock(gh);
  ULONG   ulWriteSize = 0;
  HRESULT hr = pstm->Write(lpData, *lplLen, &ulWriteSize);

  if (S_OK != hr)
  {
		DisplayError(hr,_T("Write"));
    tbGlobalUnlock(gh);
    if (gh)
    {
      tbGlobalFree(gh);
      gh=0;
    }
    return hr;
  }
  *lplLen = ulWriteSize;

  tbGlobalUnlock(gh);
  if (gh)
  {
    tbGlobalFree(gh);
    gh=0;
  }

  return S_OK;
}

/***********************************************************************
* Name:        DisplayError - Outputs error message in a Message Box
*
* Usage:       void DisplayError(HRESULT hr, LPSTR lpstr)
*                 hr - Error code
*                 lpstr - Text to accompany the error
*
* Return:      Void
***********************************************************************/
void DisplayError(HRESULT hr, LPTSTR lptstr)
{
#ifdef _DEBUG
  TCHAR lpOut[256];

	_stprintf(lpOut, _T("Error: %ld -- %s"), hr, lptstr);
	MessageBox(NULL, lpOut , NULL, MB_OK | MB_ICONEXCLAMATION | 
             MB_SYSTEMMODAL);
#endif
}
