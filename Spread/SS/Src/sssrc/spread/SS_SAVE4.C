//--------------------------------------------------------------------
//
//  File: SS_SAVE4.C
//
//  RAP01 - GIC8975-Win98 only. Running out of memory using Windows Mem Mgt.  APR.4.99
//          Changed functions to use SmartHeap for 32-bit. 
//  RAP02 - Custom Names are causing Smart Heap error on loading if all       JUN.29.99
//          custom names have been deleted. 
//  RAP03 - GIC8975 - Change Load & Save to read & write in chunks instead of SEP.27.99
//          buffering the entire SS.
#if ((defined(SS_OCX) || defined(FP_DLL)) && defined(WIN32))
#ifndef WIN32_LEAN_AND_MEAN
#define WIN32_LEAN_AND_MEAN   //exclude conflicts with VBAPI.H in WIN32
#endif
#endif

#include <windows.h>
#include <tchar.h>
#if defined(SS_DDE)
#include <oaidl.h>
#endif
#include <stdlib.h>
#if SS_V80
#include <stdio.h>
#endif
#include <string.h>
#include <memory.h>
#include "..\..\..\..\fplibs\fptools\src\fpconvrt.h"
#include "..\..\..\..\fplibs\fptools\src\texttip.h"
#include "spread.h"
#include "ss_alloc.h"
#include "ss_bool.h"
#include "ss_calc.h"
#include "ss_col.h"
#include "ss_data.h"
#include "ss_dde.h"
#include "ss_doc.h"
#include "ss_draw.h"
#include "ss_font.h"
#include "ss_formu.h"
#include "ss_main.h"
#include "ss_row.h"
#include "ss_save.h"
#include "ss_save4.h"
#include "ss_span.h"
#include "ss_type.h"
#include "ss_user.h"
#include "ss_virt.h"
#include "ss_win.h"
#include "ss_cell.h"
#include "ss_scrol.h"
#if !defined(SS_NOCALC) && !defined(SS_OLDCALC)
#include "..\calc\calc.h"
#include "..\calc\cal_cell.h"
#include "..\calc\cal_col.h"
#include "..\calc\cal_cust.h"
#include "..\calc\cal_dde.h"
#include "..\calc\cal_err.h"
#include "..\calc\cal_expr.h"
#include "..\calc\cal_func.h"
#include "..\calc\cal_mem.h"
#include "..\calc\cal_name.h"
#include "..\calc\cal_oper.h"
#include "..\calc\cal_row.h"
#endif
#include "..\classes\wintools.h"
#include "..\classes\checkbox.h"

#ifdef SS_OCX
#include "ss_spict.h"      //prototype for CreateOcxPict()
#endif

#ifndef SS_OLDSAVE
#include "..\..\vbsrc\vbspread.h"

#if (defined(SS_BOUNDCONTROL) || defined(SS_QE) || defined(SS_VB))
//DBOCX...
#ifdef SS_OCX
#include "..\..\ssocx\ssocxdb.h"
#endif //SS_OCX
//DBOCX.
#endif

#ifdef SS_V70
#include "ss_book.h"
#endif // SS_V70

#ifdef SS_V80
#include "ss_gradient.h"
#endif
//RAP01a >>
#ifdef WIN32
#define ssGlobalAlloc   tbGlobalAlloc
#define ssGlobalReAlloc tbGlobalReAlloc
#define ssGlobalLock    tbGlobalLock
#define ssGlobalUnlock  tbGlobalUnlock
#define ssGlobalFree    tbGlobalFree
#else
#define ssGlobalAlloc   GlobalAlloc
#define ssGlobalReAlloc GlobalReAlloc
#define ssGlobalLock    GlobalLock
#define ssGlobalUnlock  GlobalUnlock
#define ssGlobalFree    GlobalFree
#endif
//RAP01a <<

//--------------------------------------------------------------------

extern HANDLE hDynamicInst;

//--------------------------------------------------------------------

HBITMAP SS_Load4Bitmap(HPBYTE lpMem, long lMemLen);
BOOL SS_Save4Bitmap(HBITMAP hbmp, LPSS_BUFF lpBuff, LPLONG lplLen);

//--------------------------------------------------------------------
//
//  The SS_LOADBLOCK structure stores the range of cells being
//  loaded and any offset adjustment to be made from the orginal
//  saved coordinates.
//

typedef struct tagSS_LOADBLOCK
{ 
  SS_COORD lCol;        // Left column being loaded
  SS_COORD lRow;        // Top row being loaded
  SS_COORD lCol2;       // Right column being loaded
  SS_COORD lRow2;       // Bottom row being loaded
  SS_COORD lColOffset;  // Column offset from orginal location
  SS_COORD lRowOffset;  // Row offset from orginal location
} SS_LOADBLOCK, FAR* LPSS_LOADBLOCK;

//--------------------------------------------------------------------
//
//  The BlockTranslate() function translates from orginal saved
//  coordinates (lCol and  lRow) to the load coordinates (*lplCol
//  and *lplRow).  If the translated coordinates are within the
//  cell range being loaded, the function returns TRUE.  Otherwise,
//  the function returns FALSE.
//

static BOOL BlockTranslate(LPSS_LOADBLOCK lpLoadBlock,
                           SS_COORD lCol, SS_COORD lRow,
                           LPSS_COORD lplCol, LPSS_COORD lplRow)
{
  if (lplCol)
    *lplCol = lCol == -1 ? -1 : lCol + lpLoadBlock->lColOffset;
  if (lplRow)
    *lplRow = lRow == -1 ? -1 : lRow + lpLoadBlock->lRowOffset;

  return (!lplCol || SS_ALLCOLS == lpLoadBlock->lCol || SS_ALLCOLS == *lplCol ||
          (lpLoadBlock->lCol <= *lplCol && *lplCol <= lpLoadBlock->lCol2)) &&
         (!lplRow || SS_ALLROWS == lpLoadBlock->lRow || SS_ALLROWS == *lplRow ||
          (lpLoadBlock->lRow <= *lplRow && *lplRow <= lpLoadBlock->lRow2));
}

//--------------------------------------------------------------------

typedef struct tagSS_FONTBUFFER
{
  HGLOBAL hMem;
  LPHFONT phFont;
  long lLen;
  long lAlloc;
} SS_FONTBUFFER, FAR* LPSS_FONTBUFFER;

//--------------------------------------------------------------------
//
//  The SS_BuffInit() function initializes a buffer.
//

BOOL SS_BuffInit(LPSS_BUFF pBuff)
{
  pBuff->hMem = 0;
  pBuff->pMem = 0;
  pBuff->lLen = 0;
  pBuff->lAlloc = 0;
#if SS_V80
  pBuff->fp = NULL;
#endif
  return TRUE;
}

#if SS_V80
BOOL SS_BuffInitFP(LPSS_BUFF pBuff, FILE *fp)
{
   SS_BuffInit(pBuff);
   pBuff->fp = fp;
   return TRUE;
}
#endif
//--------------------------------------------------------------------
//
//  The SS_BuffFree() function frees all memory associated with the
//  buffer.
//

BOOL SS_BuffFree(LPSS_BUFF pBuff)
{
  if( pBuff->pMem )
    ssGlobalUnlock(pBuff->hMem);  //RAP01c
  if( pBuff->hMem )
    ssGlobalFree(pBuff->hMem);    //RAP01c
  pBuff->hMem = 0;
  pBuff->pMem = 0;
  pBuff->lLen = 0;
  pBuff->lAlloc = 0;
  return TRUE;
}

//--------------------------------------------------------------------
//
//  The SS_BuffAlloc() function ensures that the buffer has the
//  requested amount of memory allocated and locked.
//

/* RFW - 5/25/05 - Delphi slow problem
#define BUFFER_ALLOC_CNT 4096
*/
#define BUFFER_ALLOC_CNT 40960

BOOL SS_BuffAlloc(LPSS_BUFF pBuff, long lAlloc)
{
  BOOL bRet = TRUE;
  if( pBuff->lAlloc < lAlloc )
  {
    if (pBuff->lAlloc > 0)
       lAlloc = max(lAlloc, pBuff->lAlloc + BUFFER_ALLOC_CNT);

    if( pBuff->pMem )
      ssGlobalUnlock(pBuff->hMem); //RAP01c
    if( pBuff->hMem )
      pBuff->hMem = ssGlobalReAlloc(pBuff->hMem, lAlloc, GHND); //RAP01c
    else
      pBuff->hMem = ssGlobalAlloc(GHND, lAlloc); //RAP01c
    if( pBuff->hMem )
    {
      pBuff->pMem = (BYTE HUGE *)ssGlobalLock(pBuff->hMem); //RAP01c
      pBuff->lAlloc = lAlloc;
    }
    else
    {
      pBuff->pMem = 0;
      pBuff->lLen = 0;
      pBuff->lAlloc = 0;
      bRet = FALSE;
    }
  }
  return bRet;
}

//--------------------------------------------------------------------
//
//  The SS_BuffCopy() function copies lLen bytes of data into the
//  buffer.  If needed, the buffer is increased in size.
//

BOOL SS_BuffCopy(LPSS_BUFF pBuff, HPVOID pMem, long lLen)
{
  BOOL bRet = FALSE;
  if (lLen == -1)
    lLen = lstrlen(pMem) * sizeof(TCHAR);
  if( SS_BuffAlloc(pBuff, lLen) )
  {
    MemHugeCpy(pBuff->pMem, pMem, lLen);
    pBuff->lLen = lLen;
    bRet = TRUE;
  }
  return bRet;
}

//--------------------------------------------------------------------
//
//  The SS_BuffAppend() function appends lLen bytes of data to the
//  end of the current buffer.  If needed, the buffer is increased
//  in size.
//

BOOL SS_BuffAppend(LPSS_BUFF pBuff, HPVOID pMem, long lLen)
{
  BOOL bRet = FALSE;
  if (lLen == -1)
    lLen = lstrlen(pMem) * sizeof(TCHAR);
  if( SS_BuffAlloc(pBuff, pBuff->lLen + lLen) )
  {
    MemHugeCpy(pBuff->pMem + pBuff->lLen, pMem, lLen);
    pBuff->lLen += lLen;
    bRet = TRUE;
#if SS_V80
    if( pBuff->fp && pBuff->lLen > 0x7FFF )
    { // flush buffer
      _fputts((LPTSTR)pBuff->pMem, pBuff->fp);
      memset(pBuff->pMem, 0, pBuff->lLen);
      pBuff->lLen = 0;
    }
#endif
  }
  return bRet;
}

//--------------------------------------------------------------------
//
//  The SS_BuffCat() function appends a string to the
//  end of the current buffer.  If needed, the buffer is increased
//  in size.
//

BOOL SS_BuffCat(LPSS_BUFF pBuff, HPVOID pMem)
{
  BOOL bRet = FALSE;
  long lLen = lstrlen(pMem) * sizeof(TCHAR);
  if( SS_BuffAlloc(pBuff, pBuff->lLen + lLen) )
  {
    MemHugeCpy(pBuff->pMem + pBuff->lLen, pMem, lLen);
    pBuff->lLen += lLen;
    bRet = TRUE;
#if SS_V80
    if( pBuff->fp && pBuff->lLen > 0x7FFF )
    { // flush buffer
      _fputts((LPTSTR)pBuff->pMem, pBuff->fp);
      memset(pBuff->pMem, 0, pBuff->lLen);
      pBuff->lLen = 0;
    }
#endif
  }
  return bRet;
}

//--------------------------------------------------------------------
//
//  The StrHugeLen() function finds the number of characters in
//  an ANSI 8-bit character string.
//

static long StrHugeLen(const char HUGE* hpStr)
{
long lLen = 0L;
if( hpStr )
{
  while (*hpStr++)
     lLen++;
}
return lLen;
}

//--------------------------------------------------------------------
//
//  The WcsHugeLen() function finds the number of characters in
//  an Unicode 16-bit character string.
//

static long WcsHugeLen(const wchar_t HUGE* hpStr)
{
long lLen = 0L;
if( hpStr )
{
  while (*hpStr++)
     lLen++;
}
return lLen;
}

//--------------------------------------------------------------------
//
//  The SS_BuffCopyStr() function copies a null terminated character
//  array (either CHAR or WCHAR) into the buffer (as TCHAR).
//
//  Parameters:
//    pBuff - buffer to receive text string as array of TCHAR
//    pMem - source text string (either CHAR or WCHAR)
//    bUnicode - determines if pMem contains CHAR or WCHAR
//

#if defined(WIN32) && !defined(_fwcstombs)
  #define _fwcstombs wcstombs
#endif

LPTSTR SS_BuffCopyStr(LPSS_BUFF pBuff, HPVOID pMem, BOOL bUnicode)
{
  long lTextLen;

  #if defined(_UNICODE)
  
  if( bUnicode )
  {
    lTextLen = WcsHugeLen((LPWSTR)pMem) + 1;
    SS_BuffCopy(pBuff, pMem, lTextLen * sizeof(WCHAR));
  }
  else
  {
    lTextLen = StrHugeLen((LPSTR)pMem) + 1;
    if( SS_BuffAlloc(pBuff, lTextLen * sizeof(WCHAR)) )
    {
       _fmemset(pBuff->pMem, '\0', lTextLen * sizeof(WCHAR)); // RFW - 2/26/07 - 19882
// RFW - 7/7/04 - 14270
#ifdef SPREAD_JPN
       MultiByteToWideChar(CP_ACP, 0, (LPSTR)pMem, lTextLen - 1, (LPWSTR)pBuff->pMem, lTextLen * sizeof(WCHAR));
#else
       mbstowcs((LPWSTR)pBuff->pMem, (LPSTR)pMem, lTextLen);
#endif
    }
  }

  #else
  
  if( bUnicode )
  {
	  //GAB 8/20/04
	  //lTextLen = WcsHugeLen((wchar_t FAR*)pMem) + 1;
	  lTextLen = WideCharToMultiByte(CP_ACP, 0, (wchar_t FAR*)pMem, -1, NULL, 0, NULL, NULL);
	  
	  if( SS_BuffAlloc(pBuff, lTextLen) )
//      _fwcstombs((LPSTR)pBuff->pMem, (wchar_t FAR*)pMem, (size_t)lTextLen);
		  WideCharToMultiByte(CP_ACP, 0, (wchar_t FAR*)pMem, -1, (LPSTR)pBuff->pMem, lTextLen, NULL, NULL);
  }
  else
  {
    lTextLen = StrHugeLen(pMem) + 1;
    SS_BuffCopy(pBuff, pMem, lTextLen * sizeof(char));
  }

  #endif

  return (LPTSTR)pBuff->pMem;
}

//--------------------------------------------------------------------
//
//  Template for load routines.
//

/*
BOOL SS_Load4?(LPSPREADSHEET lpSS, LPSS_REC_COMMON lpCommon)
{
  LPSS_REC_? lpRec = (LPSS_REC_?)lpCommon;
  BOOL bRet = sizeof(*lpRec) == lpRec->lRecLen;
  if( bRet )
  {
  }
  return bRet;
}
*/

//--------------------------------------------------------------------
//
//  Template for save routines.
//

/*
BOOL SS_Save4?(LPSPREADSHEET lpSS, LPSS_BUFF lpBuff)
{
  SS_REC_? rec;
  BOOL bRet = TRUE;
  rec.nRecType = SS_RID_?;
  rec.lRecLen = sizeof(rec);
  bRet = SS_BuffAppend(lpBuff, &rec, sizeof(rec));
  return bRet;
}
*/

//--------------------------------------------------------------------
//
//  The SS_Load4AutoSize() function loads auto sizing information
//  into the spreadsheet.
//

BOOL SS_Load4AutoSize(LPSPREADSHEET lpSS, LPSS_REC_COMMON lpCommon)
{
  LPSS_REC_AUTOSIZE lpRec = (LPSS_REC_AUTOSIZE)lpCommon;
  BOOL bRet = sizeof(*lpRec) == lpRec->lRecLen;
  if( bRet )
  {
    lpSS->lpBook->fAutoSize = lpRec->bAutoSize;
    lpSS->Col.AutoSizeVisibleCnt = lpRec->lVisibleCols;
    lpSS->Row.AutoSizeVisibleCnt = lpRec->lVisibleRows;
  }
  return bRet;
}

//--------------------------------------------------------------------
//
//  The SS_Save4AutoSize() function saves auto sizing information
//  into the buffer.
//

BOOL SS_Save4AutoSize(LPSPREADSHEET lpSS, LPSS_BUFF lpBuff)
{
  SS_REC_AUTOSIZE rec;
  BOOL bRet = TRUE;
  if( lpSS->lpBook->fAutoSize || lpSS->Col.AutoSizeVisibleCnt
      || lpSS->Row.AutoSizeVisibleCnt )
  { 
    rec.nRecType = SS_RID_AUTOSIZE;
    rec.lRecLen = sizeof(rec);
    rec.bAutoSize = lpSS->lpBook->fAutoSize;
    rec.lVisibleCols = lpSS->Col.AutoSizeVisibleCnt;
    rec.lVisibleRows = lpSS->Row.AutoSizeVisibleCnt;
    bRet = SS_BuffAppend(lpBuff, &rec, sizeof(rec));
  }
  return bRet;
}

//--------------------------------------------------------------------
//
//  The SS_Load4BackColorStyle() function loads back color style
//  information.
//

BOOL SS_Load4BackColorStyle(LPSS_BOOK lpBook, LPSS_REC_COMMON lpCommon)
{
  LPSS_REC_BACKCOLORSTYLE lpRec = (LPSS_REC_BACKCOLORSTYLE)lpCommon;
  BOOL bRet = sizeof(*lpRec) == lpRec->lRecLen;
  if( bRet )
  {
    lpBook->bBackColorStyle = (BYTE)lpRec->nBackColorStyle;
  }
  return bRet;
}

//--------------------------------------------------------------------
//
//  The SS_Save4BackColorStyle() function saves back color style
//  information.
//

BOOL SS_Save4BackColorStyle(LPSS_BOOK lpBook, LPSS_BUFF lpBuff)
{
  SS_REC_BACKCOLORSTYLE rec;
  BOOL bRet = TRUE;
  if( TRUE )
  {
    rec.nRecType = SS_RID_BACKCOLORSTYLE;
    rec.lRecLen = sizeof(rec);
    rec.nBackColorStyle = lpBook->bBackColorStyle;
    bRet = SS_BuffAppend(lpBuff, &rec, sizeof(rec));
  }
  return bRet;
}

//--------------------------------------------------------------------
//
//  The SS_Load4Border() function loads cell border information
//  for a spreadsheet cell.
//

BOOL SS_Load4Border(LPSPREADSHEET lpSS, LPSS_REC_COMMON lpCommon,
                    LPSS_LOADBLOCK lpLoadBlock)
{
  LPSS_REC_BORDER lpRec = (LPSS_REC_BORDER)lpCommon;
  SS_COORD lCol, lRow;
  BOOL bRet = sizeof(*lpRec) == lpRec->lRecLen;
  if( bRet &&
      BlockTranslate(lpLoadBlock, lpRec->lCol, lpRec->lRow, &lCol, &lRow) )
  {
    #ifndef SS_NOBORDERS
    SS_SetBorderRange(lpSS, lCol, lRow, lCol, lRow,
                      lpRec->wBorderType, lpRec->wBorderStyle,
                      lpRec->crBorderColor);
    #endif
  }
  return bRet;
}

//--------------------------------------------------------------------
//
//  The SS_Load4BorderExt() function loads cell border information
//  for a spreadsheet cell.
//

BOOL SS_Load4BorderExt(LPSPREADSHEET lpSS, LPSS_REC_COMMON lpCommon,
                       LPSS_LOADBLOCK lpLoadBlock)
{
  LPSS_REC_BORDER_EXT lpRec = (LPSS_REC_BORDER_EXT)lpCommon;
  SS_COORD lCol, lRow;
  BOOL bRet = sizeof(*lpRec) == lpRec->lRecLen;
  if( bRet &&
      BlockTranslate(lpLoadBlock, lpRec->lCol, lpRec->lRow, &lCol, &lRow) )
  {
    #ifndef SS_NOBORDERS
    WORD  wType[4] = {SS_BORDERTYPE_LEFT, SS_BORDERTYPE_TOP, SS_BORDERTYPE_RIGHT, SS_BORDERTYPE_BOTTOM};
    short i;

    for (i = 0; i < 4; i++)
       SS_SetBorderRange(lpSS, lCol, lRow, lCol, lRow,
                         wType[i], lpRec->wBorderStyle[i],
                         lpRec->clrBorder[i]);
    #endif
  }
  return bRet;
}

//--------------------------------------------------------------------
//
//  The SS_Save4Border() function saves cell border information for
//  a spreadsheet cell.
//

BOOL SS_Save4Border(LPSPREADSHEET lpSS, SS_COORD lCol, SS_COORD lRow,
                    TBGLOBALHANDLE hBorder, LPSS_BUFF lpBuff)
{
  SS_REC_BORDER_EXT rec;
  SS_COLORTBLITEM colorItem;
  BOOL bRet = TRUE;
  if( hBorder )
  {
    LPSS_CELLBORDER lpCellBorder = (LPSS_CELLBORDER)tbGlobalLock(hBorder);
    short           i;

    rec.nRecType = SS_RID_BORDER_EXT;
    rec.lRecLen = sizeof(rec);
    rec.lCol = lCol;
    rec.lRow = lRow;

    for (i = 0; i < 4; i++)
       {
       rec.wBorderStyle[i] = lpCellBorder->Border[i].bStyle;
       SS_GetColorItem(&colorItem, lpCellBorder->Border[i].idColor);
       rec.clrBorder[i] = colorItem.Color;
       }

    bRet = SS_BuffAppend(lpBuff, &rec, sizeof(rec));

    tbGlobalUnlock(hBorder);
  }
  return bRet;
}

//--------------------------------------------------------------------
//
//  The SS_Load4Calc() function loads calculation information
//  into the spreadsheet.
//
//  Note: Auto calc option needs to be set after all data
//        is loaded.
//

BOOL SS_Load4Calc(LPSS_BOOK lpBook, LPSS_REC_COMMON lpCommon,
                  LPBOOL lpbAutoCalc)
{
  LPSS_REC_CALC lpRec = (LPSS_REC_CALC)lpCommon;
  BOOL bRet = sizeof(*lpRec) == lpRec->lRecLen;
  if( bRet )
  {
    *lpbAutoCalc = lpRec->bAutoCalc;
    lpBook->AllowUserFormulas = lpRec->bAllowUserFormulas;
    lpBook->CalcInfo.bFormulaSync = lpRec->bFormulaSync;
  }
  return bRet;
}

//--------------------------------------------------------------------
//
//  The SS_Save4Calc() function saves calculation information
//  from the spreadsheet.

BOOL SS_Save4Calc(LPSS_BOOK lpBook, LPSS_BUFF lpBuff)
{
  SS_REC_CALC rec;
  BOOL bRet = TRUE;
  if( !lpBook->CalcAuto || lpBook->AllowUserFormulas ||
      !lpBook->CalcInfo.bFormulaSync )
  {
    rec.nRecType = SS_RID_CALC;
    rec.lRecLen = sizeof(rec);
    rec.bAutoCalc = lpBook->CalcAuto;
    rec.bAllowUserFormulas = lpBook->AllowUserFormulas;
    rec.bFormulaSync = lpBook->CalcInfo.bFormulaSync;
    bRet = SS_BuffAppend(lpBuff, &rec, sizeof(rec));
  }
  return bRet;
}

//--------------------------------------------------------------------
//
//  The SS_Load4CalcOld() function loads calculation information
//  into the spreadsheet.
//
//  Note: Auto calc option needs to be set after all data
//        is loaded.
//

BOOL SS_Load4CalcOld(LPSS_BOOK lpBook, LPSS_REC_COMMON lpCommon,
                     LPBOOL lpbAutoCalc)
{
  LPSS_REC_CALC_OLD lpRec = (LPSS_REC_CALC_OLD)lpCommon;
  BOOL bRet = sizeof(*lpRec) == lpRec->lRecLen;
  if( bRet )
  {
    *lpbAutoCalc = lpRec->bAutoCalc;
    lpBook->AllowUserFormulas = lpRec->bAllowUserFormulas;
  }
  return bRet;
}

//--------------------------------------------------------------------
//
//  The SS_Save4CalcOld() function saves calculation information
//  from the spreadsheet.
/*
BOOL SS_Save4CalcOld(LPSS_BOOK lpBOOK, LPSS_BUFF lpBuff)
{
  SS_REC_CALCOLD rec;
  BOOL bRet = TRUE;
  if( !lpBook->CalcAuto || lpBook->AllowUserFormulas )
  {
    rec.nRecType = SS_RID_CALC;
    rec.lRecLen = sizeof(rec);
    rec.bAutoCalc = lpBook->CalcAuto;
    rec.bAllowUserFormulas = lpBook->AllowUserFormulas;
    bRet = SS_BuffAppend(lpBuff, &rec, sizeof(rec));
  }
  return bRet;
}
*/

//--------------------------------------------------------------------
//
//  The SS_Load4CalcDepend() function loads a SS_REC_CALCDEPEND record.
//

BOOL SS_Load4CalcDepend(LPSS_BOOK lpBook, LPSS_BUFF lpXtra, LPSS_REC_COMMON lpCommon)
{
  LPSS_REC_CALCDEPEND lpRec = (LPSS_REC_CALCDEPEND)lpCommon;
  BOOL bRet = sizeof(*lpRec) == lpRec->lRecLen;
  if( bRet )
  {
    #if defined(SS_OCX)
    bRet = SS_OcxLoad4CalcDepend(lpBook, lpRec);
    #elif defined(SS_VB)
    bRet = SS_VbxLoad4CalcDepend(lpBook, lpRec);
    #else
    bRet &= SS_BuffAppend(lpXtra, lpRec, lpRec->lRecLen);
    #endif
  }
  return bRet;
}

//--------------------------------------------------------------------

BOOL SS_Save4CalcDepend(LPSS_BOOK lpBook, LPSS_BUFF lpBuff)
{
  #if defined(SS_OCX)
  return SS_OcxSave4CalcDepend(lpBook, lpBuff);
  #elif defined(SS_VB)
  return SS_VbxSave4CalcDepend(lpBook, lpBuff);
  #else
  return TRUE;
  #endif
}

//--------------------------------------------------------------------

BOOL SS_Load4Color(LPSPREADSHEET lpSS, LPSS_REC_COMMON lpCommon,
                   LPSS_LOADBLOCK lpLoadBlock)
{
  LPSS_REC_COLOR lpRec = (LPSS_REC_COLOR)lpCommon;
  SS_COORD lCol, lRow;
  COLORREF crBackColor, crForeColor;
  BOOL bRet = sizeof(*lpRec) == lpRec->lRecLen;

  if( bRet &&
      BlockTranslate(lpLoadBlock, lpRec->lCol, lpRec->lRow, &lCol, &lRow) )
  {
    crBackColor = lpRec->crBackColor;
    crForeColor = lpRec->crForeColor;
    if( -1 == lpRec->crBackColor )
      crBackColor = RGBCOLOR_DEFAULT;
    if( -1 == lpRec->crForeColor )
      crForeColor = RGBCOLOR_DEFAULT;
    SS_SetColorRange(lpSS, lCol, lRow, lCol, lRow, crBackColor, crForeColor);
  }
  return bRet;
}

//--------------------------------------------------------------------

BOOL SS_Save4Color(LPSPREADSHEET lpSS, SS_COORD lCol, SS_COORD lRow,
                   SS_COLORID idBackColor, SS_COLORID idForeColor,
                   LPSS_BUFF lpBuff)
{
  SS_REC_COLOR rec;
  SS_COLORTBLITEM itemBackColor;
  SS_COLORTBLITEM itemForeColor;
  BOOL bRet = TRUE;
  if( idBackColor || idForeColor )
  {
    SS_GetColorItem(&itemBackColor, idBackColor);
    SS_GetColorItem(&itemForeColor, idForeColor);
    rec.nRecType = SS_RID_COLOR;
    rec.lRecLen = sizeof(rec);
    rec.lCol = lCol;
    rec.lRow = lRow;
    rec.crBackColor = idBackColor ? itemBackColor.Color : -1;
    rec.crForeColor = idForeColor ? itemForeColor.Color : -1;
    bRet = SS_BuffAppend(lpBuff, &rec, sizeof(rec));
  }
  return bRet;
}

//--------------------------------------------------------------------

BOOL SS_Load4ColPageBreak(LPSPREADSHEET lpSS, LPSS_REC_COMMON lpCommon)
{
  LPSS_REC_COLPAGEBREAK lpRec = (LPSS_REC_COLPAGEBREAK)lpCommon;
  BOOL bRet = sizeof(*lpRec) == lpRec->lRecLen;
  if( bRet )
  {
    #ifndef SS_NOPRINT
    SS_SetColPageBreak(lpSS, lpRec->lCol, lpRec->bPageBreak);
    #endif
  }
  return bRet;
}

//--------------------------------------------------------------------

BOOL SS_Save4ColPageBreak(SS_COORD lCol, BOOL bPageBreak, LPSS_BUFF lpBuff)
{
  SS_REC_COLPAGEBREAK rec;
  BOOL bRet = TRUE;
  if( bPageBreak )
  {
    rec.nRecType = SS_RID_COLPAGEBREAK;
    rec.lRecLen = sizeof(rec);
    rec.lCol = lCol;
    rec.bPageBreak = bPageBreak;
    bRet = SS_BuffAppend(lpBuff, &rec, sizeof(rec));
  }
  return bRet;
}

//--------------------------------------------------------------------

BOOL SS_Load4ColWidth(LPSPREADSHEET lpSS, LPSS_REC_COMMON lpCommon, LPSS_LOADBLOCK lpLoadBlock)
{
  LPSS_REC_COLWIDTH lpRec = (LPSS_REC_COLWIDTH)lpCommon;
  SS_COORD lCol;
  BOOL bRet = sizeof(*lpRec) == lpRec->lRecLen;
  if( bRet &&
      BlockTranslate(lpLoadBlock, lpRec->lCol, 0, &lCol, NULL) )
  {
    SS_SetColWidth(lpSS, lCol, lpRec->dfColWidth);
  }
  return bRet;
}

//--------------------------------------------------------------------

BOOL SS_Save4ColWidth(LPSPREADSHEET lpSS, SS_COORD lCol, long lColWidthX100, LPSS_BUFF lpBuff)
{
  SS_REC_COLWIDTH rec;
  long lDefColWidthX100 = lCol >= 0 && lCol < lpSS->Col.HeaderCnt ? -200 : -1;
  BOOL bRet = TRUE;
  if( lColWidthX100 != lDefColWidthX100 )
  {
    rec.nRecType = SS_RID_COLWIDTH;
    rec.lRecLen = sizeof(rec);
    rec.lCol = lCol;
    rec.dfColWidth = (double)lColWidthX100 / 100.0;
    bRet = SS_BuffAppend(lpBuff, &rec, sizeof(rec));
  }
  return bRet;
}

//--------------------------------------------------------------------

BOOL SS_Load4CursorCustom(LPSS_BOOK lpBook, LPSS_REC_COMMON lpCommon)
{
  #ifdef WIN32

  LPSS_REC_CURSORCUSTOM lpRec = (LPSS_REC_CURSORCUSTOM)lpCommon;
  ICONINFO curInfo;
  HCURSOR hcur;
  HPBYTE lpMask;
  HPBYTE lpColor;
  BOOL bRet = (long)sizeof(*lpRec) + lpRec->lMaskLen + lpRec->lColorLen
              == lpRec->lRecLen;
  if( bRet )
  {
    lpMask = (HPBYTE)lpRec + sizeof(*lpRec);
    lpColor = lpMask + lpRec->lMaskLen;
    curInfo.fIcon = FALSE;
    curInfo.xHotspot = lpRec->xHotspot;
    curInfo.yHotspot = lpRec->yHotspot;
    curInfo.hbmMask = SS_Load4Bitmap(lpMask, lpRec->lMaskLen);
    curInfo.hbmColor = SS_Load4Bitmap(lpColor, lpRec->lColorLen);
    if( hcur = CreateIconIndirect(&curInfo) )
      SS_SetCursor(lpBook, lpRec->nCursorType, hcur, TRUE);

    if (curInfo.hbmMask)
      DeleteObject(curInfo.hbmMask);
    if (curInfo.hbmColor)
      DeleteObject(curInfo.hbmColor);
  }
  return bRet;

  #else

  return TRUE;

  #endif
}

//--------------------------------------------------------------------

BOOL SS_Save4CursorCustom(WORD wType, HCURSOR hcur, LPSS_BUFF lpBuff)
{
  #ifdef WIN32

  SS_REC_CURSORCUSTOM rec;
  ICONINFO curInfo;
  long lMaskLen;
  long lColorLen;
  long lBuffOldLen = lpBuff->lLen;
  BOOL bRet = TRUE;
  if( hcur && GetIconInfo(hcur, &curInfo) )
  {
    rec.nRecType = SS_RID_CURSORCUSTOM;
    rec.lRecLen = sizeof(rec);
    rec.nCursorType = wType;
    rec.xHotspot = curInfo.xHotspot;
    rec.yHotspot = curInfo.yHotspot;
    rec.lMaskLen = 0;
    rec.lColorLen = 0;
    bRet = SS_BuffAppend(lpBuff, &rec, sizeof(rec));
    bRet &= SS_Save4Bitmap(curInfo.hbmMask, lpBuff, &lMaskLen);
    bRet &= SS_Save4Bitmap(curInfo.hbmColor, lpBuff, &lColorLen);
    if( bRet && lpBuff->pMem )
      MemHugeCpy(&rec, lpBuff->pMem + lBuffOldLen, sizeof(rec));
    rec.lRecLen = lpBuff->lLen - lBuffOldLen;
    rec.lMaskLen = lMaskLen;
    rec.lColorLen = lColorLen;
    if( bRet && lpBuff->pMem )
      MemHugeCpy(lpBuff->pMem + lBuffOldLen, &rec, sizeof(rec));
    DeleteObject(curInfo.hbmMask);
    DeleteObject(curInfo.hbmColor);
  }
  return bRet;

  #else

  return TRUE;

  #endif
}

//--------------------------------------------------------------------

BOOL SS_Load4Cursor(LPSS_BOOK lpBook, LPSS_REC_COMMON lpCommon)
{
  LPSS_REC_CURSOR lpRec = (LPSS_REC_CURSOR)lpCommon;
  BOOL bRet = sizeof(*lpRec) == lpRec->lRecLen;
  if( bRet )
  {
    SS_SetCursor(lpBook, lpRec->nCursorType, (HCURSOR)lpRec->nCursorStyle, TRUE);
  }
  return bRet;
}

//--------------------------------------------------------------------

BOOL SS_Save4Cursor(WORD wType, HCURSOR hCursor, LPSS_BUFF lpBuff)
{
  SS_REC_CURSOR rec;
  HCURSOR hCursorDef = 0;
  BOOL bRet = TRUE;
  switch( wType )
  {
    case SS_CURSORTYPE_DEFAULT: hCursorDef = SS_CURSOR_DEFAULT; break;
    case SS_CURSORTYPE_COLRESIZE: hCursorDef = SS_CURSOR_DEFCOLRESIZE; break;
    case SS_CURSORTYPE_ROWRESIZE: hCursorDef = SS_CURSOR_DEFROWRESIZE; break;
    case SS_CURSORTYPE_BUTTON: hCursorDef = SS_CURSOR_ARROW; break;
    case SS_CURSORTYPE_GRAYAREA: hCursorDef = SS_CURSOR_ARROW; break;
    case SS_CURSORTYPE_LOCKEDCELL: hCursorDef = SS_CURSOR_DEFAULT; break;
    case SS_CURSORTYPE_COLHEADER: hCursorDef = SS_CURSOR_DEFAULT; break;
    case SS_CURSORTYPE_ROWHEADER: hCursorDef = SS_CURSOR_DEFAULT; break;
    case SS_CURSORTYPE_DRAGDROPAREA: hCursorDef = SS_CURSOR_ARROW; break;
    case SS_CURSORTYPE_DRAGDROP: hCursorDef = SS_CURSOR_ARROW; break;
  }
  if( hCursorDef != hCursor )
  {
    if( SS_CURSOR_DEFAULT == hCursor ||
        SS_CURSOR_ARROW == hCursor ||
        SS_CURSOR_DEFCOLRESIZE == hCursor ||
        SS_CURSOR_DEFROWRESIZE == hCursor )
    {
      rec.nRecType = SS_RID_CURSOR;
      rec.lRecLen = sizeof(rec);
      rec.nCursorType = wType;
      rec.nCursorStyle = (short)hCursor;
      bRet = SS_BuffAppend(lpBuff, &rec, sizeof(rec));
    }
    else
    {
      bRet &= SS_Save4CursorCustom(wType, hCursor, lpBuff);
    }
  }
  return bRet;
}


#ifdef SS_V80
//--------------------------------------------------------------------
//
//  The SS_CELLIDTABLE structure is used to translate saved custom
//  function and custom name id's back into a CT_HANDLE.
//

typedef struct tagSS_CELLIDTABLE
{
  TBGLOBALHANDLE hElem;
  LPCT_HANDLE lpElem;
  short nElemCnt;
} SS_CELLIDTABLE, FAR* LPSS_CELLIDTABLE;
//--------------------------------------------------------------------
//
//  The SS_Load4CustFuncTable() function loads a table of custom
//  functions.  The custom function handles are stored in
//  lpSavedCustomFuncs array for later use by SS_Load4CustNameTable()
//  and SS_Load4Formula() functions.
//

BOOL SS_Load4CustCellTable(LPSS_BOOK lpBook, LPSS_REC_COMMON lpCommon,
                           LPSS_CELLIDTABLE lpSavedCustCells, BOOL bUnicode)
{
  LPSS_REC_CUSTCELLTABLE lpRec = (LPSS_REC_CUSTCELLTABLE)lpCommon;
  LPSS_SUBREC_CUSTCELL lpSubrec;
  CT_HANDLE hCell;
  HPBYTE lpName;
  LPTSTR lpszName;
  SS_BUFF buff;
  int i;
  BOOL bRet = (long)sizeof(*lpRec) + lpRec->lTableLen == lpRec->lRecLen;

  if( bRet )
  {
    SS_BuffInit(&buff);
    if( lpSavedCustCells->hElem = tbGlobalAlloc(GHND,
                                                lpRec->nTableCnt *
                                                sizeof(SS_CELLIDTABLE)) )
    {
      lpSavedCustCells->lpElem = (LPCT_HANDLE)tbGlobalLock(lpSavedCustCells->hElem);
      lpSavedCustCells->nElemCnt = lpRec->nTableCnt;
      lpSubrec = (LPSS_SUBREC_CUSTCELL)((HPBYTE)lpRec + sizeof(*lpRec));
      for( i = 0; i < lpRec->nTableCnt; i++ )
      {
        lpName = (HPBYTE)lpSubrec + sizeof(*lpSubrec);
        lpszName = SS_BuffCopyStr(&buff, lpName, bUnicode);
/*
        if( hCell = CustCellTypeLookup(&lpBook->CustCellTypes, lpszName) )
        {
          if( CustCellTypeGetEditable(hCell) == lpSubrec->bEditable ||
              CustCellTypeGetCanOverflow(hCell) == lpSubrec->bCanOverflow ||
              CustCellTypeGetCanBeOverflown(hCell) == lpSubrec->bCanBeOverflown ||
              CustCellTypeGetUseRendererControl(hCell) == lpSubrec->bUseRendererControl )
            hCell = CustCellTypeCreateRef(hCell);
          else
            hCell = 0;
        }
        else
*/
        if( !CustCellTypeLookup(&lpBook->CustCellTypes, lpszName) )
        {
          hCell = CustCellTypeCreate(lpszName, lpSubrec->bEditable, lpSubrec->bCanOverflow,
                                 lpSubrec->bCanBeOverflown, lpSubrec->bUseRendererControl, 
#if SS_OCX
                                 lpBook->pCTProcs);
#else
                                 NULL);
#endif
          CustCellTypeAdd(&lpBook->CustCellTypes, hCell);
        }
        lpSavedCustCells->lpElem[i] = hCell;
        lpSubrec = (LPSS_SUBREC_CUSTCELL)((HPBYTE)lpSubrec + sizeof(*lpSubrec)
                                          + lpSubrec->lNameLen);
      }
    }
    SS_BuffFree(&buff);
  }
  return bRet;
}
//--------------------------------------------------------------------
//
//  The SS_Save4CustCellTable() function saves all the custom
//  celltypes which are currently registered with the spreadsheet.
//

BOOL SS_Save4CustCellTable(LPSS_BOOK lpBook, LPSS_BUFF lpBuff)
{
  SS_REC_CUSTCELLTABLE rec;
  SS_SUBREC_CUSTCELL subrec;
  LPCT_HANDLE lpElem;
  LPSS_CUSTOMCELLTYPE lpCustCell;
  LPTSTR lpszText;
  long lStartRec = lpBuff->lLen;
  long lStartTable;
  int i;
  BOOL bRet = TRUE;

  if( lpBook->CustCellTypes.hElem &&
      (lpElem = (LPCT_HANDLE)CT_MemLock(lpBook->CustCellTypes.hElem)) )
  {
    rec.nRecType = SS_RID_CUSTCELLTABLE;
    rec.lRecLen = sizeof(rec);
    rec.nTableCnt = lpBook->CustCellTypes.nElemCnt;
    rec.lTableLen = 0;
    bRet = SS_BuffAppend(lpBuff, &rec, sizeof(rec));
    lStartTable = lpBuff->lLen;
    for( i = 0; bRet && i < lpBook->CustCellTypes.nElemCnt; i++ )
    {
      if( lpElem[i] &&
          (lpCustCell = (LPSS_CUSTOMCELLTYPE)CT_MemLock(lpElem[i])) )
      {
        if( lpCustCell->hText &&
            (lpszText = (LPTSTR)CT_MemLock(lpCustCell->hText)) )
        {
//		  subrec.lRefCnt = lpCustCell->lRefCnt;
	      subrec.bEditable = lpCustCell->bEditable;
	      subrec.bCanOverflow = lpCustCell->bCanOverflow;
	  	  subrec.bCanBeOverflown = lpCustCell->bCanBeOverflown;
        subrec.bUseRendererControl = lpCustCell->bUseRendererControl;
//		  subrec.Procs = lpCustCell->Procs;
          subrec.lNameLen = (lstrlen(lpszText) + 1) * sizeof(TCHAR);
          bRet &= SS_BuffAppend(lpBuff, &subrec, sizeof(subrec));
          bRet &= SS_BuffAppend(lpBuff, lpszText, subrec.lNameLen);
          CT_MemUnlock(lpCustCell->hText);
        }
        CT_MemUnlock(lpElem[i]);
      }
    }
    if( bRet && lpBuff->pMem )
      MemHugeCpy(&rec, lpBuff->pMem + lStartRec, sizeof(rec));
    rec.lRecLen = lpBuff->lLen - lStartRec;
    rec.lTableLen = lpBuff->lLen - lStartTable;
    if( bRet && lpBuff->pMem )
      MemHugeCpy(lpBuff->pMem + lStartRec, &rec, sizeof(rec));
    CT_MemUnlock(lpBook->CustCellTypes.hElem);
  }
  return bRet;
}
#endif
//--------------------------------------------------------------------
//
//  The SS_CALCIDTABLE structure is used to translate saved custom
//  function and custom name id's back into a CALC_HANDLE.
//

typedef struct tagSS_CALCIDTABLE
{
  #if defined(SS_NOCALC) || defined(SS_OLDCALC)
  long lUnused1; long lUnused2; short nUnused3;
  #else
  TBGLOBALHANDLE hElem;
  LPCALC_HANDLE lpElem;
  short nElemCnt;
  #endif
} SS_CALCIDTABLE, FAR* LPSS_CALCIDTABLE;

//--------------------------------------------------------------------
//
//  The SS_Load4CustFuncTable() function loads a table of custom
//  functions.  The custom function handles are stored in
//  lpSavedCustomFuncs array for later use by SS_Load4CustNameTable()
//  and SS_Load4Formula() functions.
//

BOOL SS_Load4CustFuncTable(LPSS_BOOK lpBook, LPSS_REC_COMMON lpCommon,
                           LPSS_CALCIDTABLE lpSavedCustFuncs, BOOL bUnicode)
{
  #if defined(SS_NOCALC) || defined(SS_OLDCALC)
  return TRUE;
  #else
  LPSS_REC_CUSTFUNCTABLE lpRec = (LPSS_REC_CUSTFUNCTABLE)lpCommon;
  LPSS_SUBREC_CUSTFUNC lpSubrec;
  CALC_HANDLE hFunc;
  HPBYTE lpName;
  LPTSTR lpszName;
  SS_BUFF buff;
  int i;
  BOOL bRet = (long)sizeof(*lpRec) + lpRec->lTableLen == lpRec->lRecLen;

  if( bRet )
  {
    SS_BuffInit(&buff);
    if( lpSavedCustFuncs->hElem = tbGlobalAlloc(GHND,
                                                lpRec->nTableCnt *
                                                sizeof(SS_CALCIDTABLE)) )
    {
      lpSavedCustFuncs->lpElem = (LPCALC_HANDLE)tbGlobalLock(lpSavedCustFuncs->hElem);
      lpSavedCustFuncs->nElemCnt = lpRec->nTableCnt;
      lpSubrec = (LPSS_SUBREC_CUSTFUNC)((HPBYTE)lpRec + sizeof(*lpRec));
      for( i = 0; i < lpRec->nTableCnt; i++ )
      {
        lpName = (HPBYTE)lpSubrec + sizeof(*lpSubrec);
        lpszName = SS_BuffCopyStr(&buff, lpName, bUnicode);
        if( hFunc = CustFuncLookup(&lpBook->CalcInfo.CustFuncs, lpszName) )
        {
          if( CustFuncGetMinArgs(hFunc) == lpSubrec->nMinArgs &&
              CustFuncGetMaxArgs(hFunc) == lpSubrec->nMaxArgs &&
              CustFuncGetFlags(hFunc) == lpSubrec->lFlags )
            hFunc = CustFuncCreateRef(hFunc);
          else
            hFunc = 0;
        }
        else
        {
          hFunc = CustFuncCreate(lpszName, lpSubrec->nMinArgs,
                                 lpSubrec->nMaxArgs, NULL, lpSubrec->lFlags);
          CustFuncAdd(&lpBook->CalcInfo.CustFuncs, hFunc);
        }
        lpSavedCustFuncs->lpElem[i] = hFunc;
        lpSubrec = (LPSS_SUBREC_CUSTFUNC)((HPBYTE)lpSubrec + sizeof(*lpSubrec)
                                          + lpSubrec->lNameLen);
      }
    }
    SS_BuffFree(&buff);
  }
  return bRet;
  #endif
}

//--------------------------------------------------------------------
//
//  The SS_Save4CustFuncTable() function saves all the custom
//  functions which are currently registered with the spreadsheet.
//

BOOL SS_Save4CustFuncTable(LPSS_BOOK lpBook, LPSS_BUFF lpBuff)
{
  #if defined(SS_NOCALC) || defined(SS_OLDCALC)
  return TRUE;
  #else
  SS_REC_CUSTFUNCTABLE rec;
  SS_SUBREC_CUSTFUNC subrec;
  LPCALC_HANDLE lpElem;
  LPCALC_CUSTFUNC lpCustFunc;
  LPTSTR lpszText;
  long lStartRec = lpBuff->lLen;
  long lStartTable;
  int i;
  BOOL bRet = TRUE;

  if( lpBook->CalcInfo.CustFuncs.hElem &&
      (lpElem = (LPCALC_HANDLE)CalcMemLock(lpBook->CalcInfo.CustFuncs.hElem)) )
  {
    rec.nRecType = SS_RID_CUSTFUNCTABLE;
    rec.lRecLen = sizeof(rec);
    rec.nTableCnt = lpBook->CalcInfo.CustFuncs.nElemCnt;
    rec.lTableLen = 0;
    bRet = SS_BuffAppend(lpBuff, &rec, sizeof(rec));
    lStartTable = lpBuff->lLen;
    for( i = 0; bRet && i < lpBook->CalcInfo.CustFuncs.nElemCnt; i++ )
    {
      if( lpElem[i] &&
          (lpCustFunc = (LPCALC_CUSTFUNC)CalcMemLock(lpElem[i])) )
      {
        if( lpCustFunc->hText &&
            (lpszText = (LPTSTR)CalcMemLock(lpCustFunc->hText)) )
        {
          subrec.nMinArgs = lpCustFunc->nMinArgs;
          subrec.nMaxArgs = lpCustFunc->nMaxArgs;
          subrec.lFlags = lpCustFunc->lFlags;
          subrec.lNameLen = (lstrlen(lpszText) + 1) * sizeof(TCHAR);
          bRet &= SS_BuffAppend(lpBuff, &subrec, sizeof(subrec));
          bRet &= SS_BuffAppend(lpBuff, lpszText, subrec.lNameLen);
          CalcMemUnlock(lpCustFunc->hText);
        }
        CalcMemUnlock(lpElem[i]);
      }
    }
    if( bRet && lpBuff->pMem )
      MemHugeCpy(&rec, lpBuff->pMem + lStartRec, sizeof(rec));
    rec.lRecLen = lpBuff->lLen - lStartRec;
    rec.lTableLen = lpBuff->lLen - lStartTable;
    if( bRet && lpBuff->pMem )
      MemHugeCpy(lpBuff->pMem + lStartRec, &rec, sizeof(rec));
    CalcMemUnlock(lpBook->CalcInfo.CustFuncs.hElem);
  }
  return bRet;
  #endif
}

//--------------------------------------------------------------------

BOOL SS_Load4DdeTable(LPSPREADSHEET lpSS, LPSS_REC_COMMON lpCommon,
                      LPSS_CALCIDTABLE lpSavedDde, BOOL bUnicode)
{
  #if !defined(SS_DDE)
  return TRUE;
  #else
  LPSS_REC_DDETABLE lpRec = (LPSS_REC_DDETABLE)lpCommon;
  LPSS_SUBREC_DDE lpSubRec;
  CALC_HANDLE hDde;
  HPBYTE  lpServer, lpTopic, lpItem, lpReadExpr, lpWriteExpr;
  LPTSTR lpszServer, lpszTopic, lpszItem, lpszReadExpr, lpszWriteExpr;
  SS_BUFF buffServer, buffTopic, buffItem, buffReadExpr, buffWriteExpr;
  int i;
  BOOL bRet = (long)sizeof(*lpRec) + lpRec->lTableLen == lpRec->lRecLen;

  if( bRet )
  {
    SS_BuffInit(&buffServer);
    SS_BuffInit(&buffTopic);
    SS_BuffInit(&buffItem);
    SS_BuffInit(&buffReadExpr);
    SS_BuffInit(&buffWriteExpr);
    if( lpSavedDde->hElem = tbGlobalAlloc(GHND,
                                          lpRec->nTableCnt *
                                          sizeof(SS_CALCIDTABLE)) )
    {
      lpSavedDde->lpElem = (LPCALC_HANDLE)tbGlobalLock(lpSavedDde->hElem);
      lpSavedDde->nElemCnt = lpRec->nTableCnt;
      lpSubRec = (LPSS_SUBREC_DDE)((HPBYTE)lpRec + sizeof(*lpRec));
      for( i = 0; i < lpRec->nTableCnt; i++ )
      {
        lpServer = (HPBYTE)lpSubRec + sizeof(*lpSubRec);
        lpTopic = (HPBYTE)lpServer + lpSubRec->lServerLen;
        lpItem = (HPBYTE)lpTopic + lpSubRec->lTopicLen;
        lpReadExpr = (HPBYTE)lpItem + lpSubRec->lItemLen;
        lpWriteExpr = (HPBYTE)lpReadExpr + lpSubRec->lReadExprLen;
        lpszServer = SS_BuffCopyStr(&buffServer, lpServer, bUnicode);
        lpszTopic = SS_BuffCopyStr(&buffTopic, lpTopic, bUnicode);
        lpszItem = SS_BuffCopyStr(&buffItem, lpItem, bUnicode);
        if( hDde = DdeLookup(&lpSS->CalcInfo.DdeLinks, lpszServer, lpszTopic, lpszItem) )
          hDde = DdeCreateRef(hDde);
        else if( hDde = DdeCreate(lpszServer, lpszTopic, lpszItem, lpSS->CalcInfo.lpfnDdeUpdate, lpSS->CalcInfo.hSS) )
          DdeAdd(&lpSS->CalcInfo.DdeLinks, hDde);
        DdeSetMode(hDde, lpSubRec->nMode);
        if( lpSubRec->lReadExprLen )
        {
          lpszReadExpr = SS_BuffCopyStr(&buffReadExpr, lpReadExpr, bUnicode);
          DdeSetReadExpr(hDde, lpszReadExpr);
        }
        if( lpSubRec->lWriteExprLen )
        {
          lpszWriteExpr = SS_BuffCopyStr(&buffWriteExpr, lpWriteExpr, bUnicode);
          DdeSetWriteExpr(hDde, lpszWriteExpr);
        }
        lpSavedDde->lpElem[i] = hDde;
        lpSubRec = (LPSS_SUBREC_DDE)((HPBYTE)lpSubRec + sizeof(*lpSubRec)
                                     + lpSubRec->lServerLen
                                     + lpSubRec->lTopicLen
                                     + lpSubRec->lItemLen
                                     + lpSubRec->lReadExprLen
                                     + lpSubRec->lWriteExprLen);
      }
    }
    SS_BuffFree(&buffServer);
    SS_BuffFree(&buffTopic);
    SS_BuffFree(&buffItem);
    SS_BuffFree(&buffReadExpr);
    SS_BuffFree(&buffWriteExpr);
  }
  return bRet;
  #endif
}

//--------------------------------------------------------------------
//
//  The SS_Save4DdeTable() function saves all the DDE links which
//  are currently used in the spreadsheet.
//

BOOL SS_Save4DdeTable(LPSPREADSHEET lpSS, LPSS_BUFF lpBuff)
{
  #if !defined(SS_DDE)
  return TRUE;
  #else
  SS_REC_DDETABLE rec;
  SS_SUBREC_DDE subrec;
  LPCALC_HANDLE lpElem;
  LPCALC_DDE lpDde;
  LPTSTR lpszServer;
  LPTSTR lpszTopic;
  LPTSTR lpszItem;
  LPTSTR lpszReadExpr;
  LPTSTR lpszWriteExpr;
  long lStartRec = lpBuff->lLen;
  long lStartTable;
  int i;
  BOOL bRet = TRUE;
  
  if( lpSS->CalcInfo.DdeLinks.hElem &&
      (lpElem = (LPCALC_HANDLE)CalcMemLock(lpSS->CalcInfo.DdeLinks.hElem)) )
  {
    rec.nRecType = SS_RID_DDETABLE;
    rec.lRecLen = sizeof(rec);
    rec.nTableCnt = lpSS->CalcInfo.DdeLinks.nElemCnt;
    rec.lTableLen = 0;
    bRet = SS_BuffAppend(lpBuff, &rec, sizeof(rec));
    lStartTable = lpBuff->lLen;
    for( i = 0; bRet && i < lpSS->CalcInfo.DdeLinks.nElemCnt; i++ )
    {
      if( lpElem[i] &&
          (lpDde = (LPCALC_DDE)CalcMemLock(lpElem[i])) )
      {
        if( lpDde->hServer &&
            lpDde->hTopic &&
            lpDde->hItem &&
            (lpszServer = (LPTSTR)CalcMemLock(lpDde->hServer)) &&
            (lpszTopic = (LPTSTR)CalcMemLock(lpDde->hTopic)) &&
            (lpszItem = (LPTSTR)CalcMemLock(lpDde->hItem)) )
        {
          if( lpDde->hReadExpr )
            lpszReadExpr = (LPTSTR)CalcMemLock(lpDde->hReadExpr);
          else
            lpszReadExpr = NULL;
          if( lpDde->hWriteExpr )
            lpszWriteExpr = (LPTSTR)CalcMemLock(lpDde->hWriteExpr);
          else
            lpszWriteExpr = NULL;
          subrec.lServerLen = (lstrlen(lpszServer) + 1) * sizeof(TCHAR);
          subrec.lTopicLen = (lstrlen(lpszTopic) + 1) * sizeof(TCHAR);
          subrec.lItemLen = (lstrlen(lpszItem) + 1) * sizeof(TCHAR);
          if( lpszReadExpr )
            subrec.lReadExprLen = (lstrlen(lpszReadExpr) + 1) * sizeof(TCHAR);
          else
            subrec.lReadExprLen = 0;
          if( lpszWriteExpr )
            subrec.lWriteExprLen = (lstrlen(lpszWriteExpr) + 1) * sizeof(TCHAR);
          else
            subrec.lWriteExprLen = 0;
          subrec.nMode = lpDde->nMode;
          bRet &= SS_BuffAppend(lpBuff, &subrec, sizeof(subrec));
          bRet &= SS_BuffAppend(lpBuff, lpszServer, subrec.lServerLen);
          bRet &= SS_BuffAppend(lpBuff, lpszTopic, subrec.lTopicLen);
          bRet &= SS_BuffAppend(lpBuff, lpszItem, subrec.lItemLen);
          if( lpszReadExpr )
            bRet &= SS_BuffAppend(lpBuff, lpszReadExpr, subrec.lReadExprLen);
          if( lpszWriteExpr )
            bRet &= SS_BuffAppend(lpBuff, lpszWriteExpr, subrec.lWriteExprLen);
          CalcMemUnlock(lpDde->hServer);
          CalcMemUnlock(lpDde->hTopic);
          CalcMemUnlock(lpDde->hItem);
          if( lpDde->hReadExpr )
            CalcMemUnlock(lpDde->hReadExpr);
          if( lpDde->hWriteExpr )
            CalcMemUnlock(lpDde->hWriteExpr);
        }
        CalcMemUnlock(lpElem[i]);
      }
    }
    if( bRet && lpBuff->pMem )
      MemHugeCpy(&rec, lpBuff->pMem + lStartRec, sizeof(rec));
    rec.lRecLen = lpBuff->lLen - lStartRec;
    rec.lTableLen = lpBuff->lLen - lStartTable;
    if( bRet && lpBuff->pMem )
      MemHugeCpy(lpBuff->pMem + lStartRec, &rec, sizeof(rec));
    CalcMemUnlock(lpSS->CalcInfo.DdeLinks.hElem);
  }
  return bRet;
  #endif
}

//--------------------------------------------------------------------
//
//  The CustFuncGetSaveId() function translates an internal custom
//  function handle into an external custom function id.
//

#if !defined(SS_NOCALC) && !defined(SS_OLDCALC)
static short CustFuncGetSaveId(LPCALC_BOOK lpBook, CALC_HANDLE hFunc)
{
  LPCALC_HANDLE lpElem;
  short i;
  short nId = 0;

  if( lpBook->CustFuncs.hElem &&
      (lpElem = (LPCALC_HANDLE)CalcMemLock(lpBook->CustFuncs.hElem)) )
  {
    for( i = 0; i < lpBook->CustFuncs.nElemCnt; i++ )
    {
      if( hFunc == lpElem[i] )
      {
        nId = i;
        break;
      }
    }
    CalcMemUnlock(lpBook->CustFuncs.hElem);
  }
  return nId;
}
#endif

//--------------------------------------------------------------------
//
//  The CustNameGetSaveId() function translates an internal custom
//  name handle into an external custom name id.
//

#if !defined(SS_NOCALC) && !defined(SS_OLDCALC)
static short CustNameGetSaveId(LPCALC_BOOK lpBook, CALC_HANDLE hName)
{
  LPCALC_HANDLE lpElem;
  short i;
  short nId = 0;

  if( lpBook->Names.hElem &&
      (lpElem = (LPCALC_HANDLE)CalcMemLock(lpBook->Names.hElem)) )
  {
    for( i = 0; i < lpBook->Names.nElemCnt; i++ )
    {
      if( hName == lpElem[i] )
      {
        nId = i;
        break;
      }
    }
    CalcMemUnlock(lpBook->Names.hElem);
  }
  return nId;
}
#endif

//--------------------------------------------------------------------
//
//  The DdeGetSaveId() function translates an internal DDE link
//  handle into an external DDE link id.
//

#if defined(SS_DDE)
static short DdeGetSaveId(LPCALC_INFO lpCalc, CALC_HANDLE hDde)
{
  LPCALC_HANDLE lpElem;
  short i;
  short nId = 0;

  if( lpCalc->DdeLinks.hElem &&
      (lpElem = (LPCALC_HANDLE)CalcMemLock(lpCalc->DdeLinks.hElem)) )
  {
    for( i = 0; i < lpCalc->DdeLinks.nElemCnt; i++ )
    {
      if( hDde == lpElem[i] )
      {
        nId = i;
        break;
      }
    }
    CalcMemUnlock(lpCalc->DdeLinks.hElem);
  }
  return nId;
}
#endif

//--------------------------------------------------------------------
//
//  The SS_Load4Expr() function retrieves a parsed expression for
//  the buffer.  The parsed expression is a field within another
//  record.
//

#if !defined(SS_NOCALC) && !defined(SS_OLDCALC)
static CALC_HANDLE SS_Load4Expr(LPSS_BOOK lpBook,
                                HPBYTE lpBuff, long lLen,
                                LPSS_CALCIDTABLE lpSavedCustFuncs,
                                LPSS_CALCIDTABLE lpSavedCustNames,
                                LPSS_CALCIDTABLE lpSavedDdeLinks,
                                BOOL bUnicode)
{
  LPSS_SUBREC_EXPRCOMMON lpRecCommon;
  LPSS_SUBREC_EXPRLONG lpRecLong;
  LPSS_SUBREC_EXPRDOUBLE lpRecDouble;
  LPSS_SUBREC_EXPRSTR lpRecStr;
  LPSS_SUBREC_EXPRCELL lpRecCell;
  LPSS_SUBREC_EXPRRANGE lpRecRange;
  #if defined(SS_V70)
  LPSS_SUBREC_EXPREXTERNALCELL lpRecExternalCell;
  LPSS_SUBREC_EXPREXTERNALRANGE lpRecExternalRange;
  #endif
  LPSS_SUBREC_EXPRCUSTNAME lpRecCustName;
  LPSS_SUBREC_EXPRFUNC lpRecFunc;
  LPSS_SUBREC_EXPRCUSTFUNC lpRecCustFunc;
  LPSS_SUBREC_EXPROPER lpRecOper;
  LPSS_SUBREC_EXPRSEP lpRecSep;
  LPSS_SUBREC_EXPRERROR lpRecError;
  #if defined(SS_DDE)
  LPSS_SUBREC_EXPRDDE lpRecDde;
  #endif
  LPCALC_EXPRINFO lpExpr;
  #if defined(SS_V70)
  LPCALC_SHEET lpSheet;
  #endif
  CALC_HANDLE hCustFunc;
  CALC_HANDLE hCustName;
  #if defined(SS_DDE)
  CALC_HANDLE hDde;
  #endif
  LPTSTR lpszText;
  LPTSTR lpszTemp;
  SS_BUFF buff;
  long lInc;
  short nRecCnt = 0;
  CALC_HANDLE hExpr = 0;

  SS_BuffInit(&buff);
  lpRecCommon = (LPSS_SUBREC_EXPRCOMMON)lpBuff;
  while( (HPBYTE)lpRecCommon < (HPBYTE)lpBuff + lLen )
  {
    switch( lpRecCommon->nType )
    {
      case CALC_EXPR_LONG:
        lInc = sizeof(SS_SUBREC_EXPRLONG);
        break;
      case CALC_EXPR_DOUBLE:
        lInc = sizeof(SS_SUBREC_EXPRDOUBLE);
        break;
      case CALC_EXPR_STRING:
        lpRecStr = (LPSS_SUBREC_EXPRSTR)lpRecCommon;
        lInc = sizeof(SS_SUBREC_EXPRSTR) + lpRecStr->nLen;
        break;
      case CALC_EXPR_CELL:
        lInc = sizeof(SS_SUBREC_EXPRCELL);
        break;
      case CALC_EXPR_RANGE:
        lInc = sizeof(SS_SUBREC_EXPRRANGE);
        #if !defined(SS_V70)
        nRecCnt += 2;  // range is converted to two cells and an operator
        #endif
        break;
      #if defined(SS_V70)
      case CALC_EXPR_EXTERNALCELL:
        lInc = sizeof(SS_SUBREC_EXPREXTERNALCELL);
        break;
      case CALC_EXPR_EXTERNALRANGE:
        lInc = sizeof(SS_SUBREC_EXPREXTERNALRANGE);
        break;
      #endif
      case CALC_EXPR_NAME:
        lInc = sizeof(SS_SUBREC_EXPRCUSTNAME);
        break;
      case CALC_EXPR_FUNCTION:
        lInc = sizeof(SS_SUBREC_EXPRFUNC);
        break;
      case CALC_EXPR_CUSTFUNC:
        lInc = sizeof(SS_SUBREC_EXPRCUSTFUNC);
        break;
      case CALC_EXPR_OPERATOR:
        lInc = sizeof(SS_SUBREC_EXPROPER);
        break;
      case CALC_EXPR_SEPARATOR:
        lInc = sizeof(SS_SUBREC_EXPRSEP);
        break;
      case CALC_EXPR_DDE:
        lInc = sizeof(SS_SUBREC_EXPRDDE);
        break;
      case CALC_EXPR_ERROR:
        lInc = sizeof(SS_SUBREC_EXPRERROR);
        break;
      default:
        return 0;
    }
    nRecCnt++;
    lpRecCommon = (LPSS_SUBREC_EXPRCOMMON)((HPBYTE)lpRecCommon + lInc);
  }
  if( hExpr = CalcMemAlloc((2+nRecCnt) * sizeof(CALC_EXPRINFO)) )
  {
    if( lpExpr = CalcMemLock(hExpr) )
    {
      lpExpr->nType = CALC_EXPR_BEGIN;
      lpExpr->u.Header.lRefCnt = 1;
      lpExpr++;
      lpRecCommon = (LPSS_SUBREC_EXPRCOMMON)lpBuff;
      while( (HPBYTE)lpRecCommon < (HPBYTE)lpBuff + lLen )
      {
        switch( lpRecCommon->nType )
        {
          case CALC_EXPR_LONG:
            lpRecLong = (LPSS_SUBREC_EXPRLONG)lpRecCommon;
            lpExpr->nType = CALC_EXPR_LONG;
            lpExpr->u.Long.lVal = lpRecLong->lVal;
            lInc = sizeof(SS_SUBREC_EXPRLONG);
            break;
          case CALC_EXPR_DOUBLE:
            lpRecDouble = (LPSS_SUBREC_EXPRDOUBLE)lpRecCommon;
            lpExpr->nType = CALC_EXPR_DOUBLE;
            lpExpr->u.Double.dfVal = lpRecDouble->dfVal;
            lInc = sizeof(SS_SUBREC_EXPRDOUBLE);
            break;
          case CALC_EXPR_STRING:
            lpRecStr = (LPSS_SUBREC_EXPRSTR)lpRecCommon;
            lpszText = SS_BuffCopyStr(&buff,
                                      (HPBYTE)lpRecStr + sizeof(*lpRecStr),
                                      bUnicode);
            lpExpr->nType = CALC_EXPR_STRING;
            lpExpr->u.String.hText = CalcMemAlloc((lstrlen(lpszText) + 1)
                                                  * sizeof(TCHAR));
            lpszTemp = CalcMemLock(lpExpr->u.String.hText);
            lstrcpy(lpszTemp, lpszText);
            CalcMemUnlock(lpExpr->u.String.hText);
            lInc = sizeof(SS_SUBREC_EXPRSTR) + lpRecStr->nLen;
            break;
          case CALC_EXPR_CELL:
            lpRecCell = (LPSS_SUBREC_EXPRCELL)lpRecCommon;
            lpExpr->nType = CALC_EXPR_CELL;
            lpExpr->u.Cell.lCol = lpRecCell->lCol;
            lpExpr->u.Cell.lRow = lpRecCell->lRow;
            lInc = sizeof(SS_SUBREC_EXPRCELL);
            break;
          case CALC_EXPR_RANGE:
            lpRecRange = (LPSS_SUBREC_EXPRRANGE)lpRecCommon;
            #if defined(SS_V70)
            lpExpr->nType = CALC_EXPR_RANGE;
            lpExpr->u.Range.lCol1 = lpRecRange->lCol1;
            lpExpr->u.Range.lRow1 = lpRecRange->lRow1;
            lpExpr->u.Range.lCol2 = lpRecRange->lCol2;
            lpExpr->u.Range.lRow2 = lpRecRange->lRow2;
            #else
            lpExpr->nType = CALC_EXPR_CELL;
            lpExpr->u.Cell.lCol = lpRecRange->lCol1;
            lpExpr->u.Cell.lRow = lpRecRange->lRow1;
            lpExpr++;
            lpExpr->nType = CALC_EXPR_CELL;
            lpExpr->u.Cell.lCol = lpRecRange->lCol2;
            lpExpr->u.Cell.lRow = lpRecRange->lRow2;
            lpExpr++;
            lpExpr->nType = CALC_EXPR_OPERATOR;
            lpExpr->u.Oper.nId = CALC_OPER_RANGE;
            #endif
            lInc = sizeof(SS_SUBREC_EXPRRANGE);
            break;
          #if defined(SS_V70)
          case CALC_EXPR_EXTERNALCELL:
            lpRecExternalCell = (LPSS_SUBREC_EXPREXTERNALCELL)lpRecCommon;
            lpSheet = SS_CalcGetSheetFromIndex(lpBook->CalcInfo.hBook, lpRecExternalCell->nSheet);
            if (lpSheet != NULL)
            {
              lpExpr->nType = CALC_EXPR_EXTERNALCELL;
              lpExpr->u.ExternalCell.lpSheet = lpSheet;
              lpExpr->u.ExternalCell.lCol = lpRecExternalCell->lCol;
              lpExpr->u.ExternalCell.lRow = lpRecExternalCell->lRow;
            }
            else
            {
              lpExpr->nType = CALC_EXPR_ERROR;
              lpExpr->u.Error.nId = CALC_ERROR_REF;
            }
            lInc = sizeof(SS_SUBREC_EXPREXTERNALCELL);
            break;
          case CALC_EXPR_EXTERNALRANGE:
            lpRecExternalRange = (LPSS_SUBREC_EXPREXTERNALRANGE)lpRecCommon;
            lpSheet = SS_CalcGetSheetFromIndex(lpBook->CalcInfo.hBook, lpRecExternalRange->nSheet);
            if (lpSheet != NULL)
            {
              lpExpr->nType = CALC_EXPR_EXTERNALRANGE;
              lpExpr->u.ExternalRange.lpSheet = lpSheet;
              lpExpr->u.ExternalRange.lCol1 = lpRecExternalRange->lCol1;
              lpExpr->u.ExternalRange.lRow1 = lpRecExternalRange->lRow1;
              lpExpr->u.ExternalRange.lCol2 = lpRecExternalRange->lCol2;
              lpExpr->u.ExternalRange.lRow2 = lpRecExternalRange->lRow2;
            }
            else
            {
              lpExpr->nType = CALC_EXPR_ERROR;
              lpExpr->u.Error.nId = CALC_ERROR_REF;
            }
            lInc = sizeof(SS_SUBREC_EXPREXTERNALRANGE);
            break;
          #endif
          case CALC_EXPR_NAME:
            lpRecCustName = (LPSS_SUBREC_EXPRCUSTNAME)lpRecCommon;
            if( lpSavedCustNames->lpElem )
              hCustName = lpSavedCustNames->lpElem[lpRecCustName->nId];
            else
              hCustName = 0;
            lpExpr->nType = CALC_EXPR_NAME;
            lpExpr->u.CustName.hName = NameCreateRef(hCustName);
            lInc = sizeof(SS_SUBREC_EXPRCUSTNAME);
            break;
          case CALC_EXPR_FUNCTION:
            lpRecFunc = (LPSS_SUBREC_EXPRFUNC)lpRecCommon;
            lpExpr->nType = CALC_EXPR_FUNCTION;
            lpExpr->u.Func.nId = lpRecFunc->nId;
            lpExpr->u.Func.nArgs = lpRecFunc->nArgs;
            #if defined(SS_V35)
            // fix problem created when AND and OR were modified in V35
            switch (lpExpr->u.Func.nId)
            {
              case CALC_FUNC_AND_OLD: lpExpr->u.Func.nId = CALC_FUNC_AND; break;
              case CALC_FUNC_OR_OLD: lpExpr->u.Func.nId = CALC_FUNC_OR; break;
            }
            #endif
            lInc = sizeof(SS_SUBREC_EXPRFUNC);
            break;
          case CALC_EXPR_CUSTFUNC:
            lpRecCustFunc = (LPSS_SUBREC_EXPRCUSTFUNC)lpRecCommon;
            if( lpSavedCustFuncs->lpElem )
              hCustFunc = lpSavedCustFuncs->lpElem[lpRecCustFunc->nId];
            else
              hCustFunc = 0;
            lpExpr->nType = CALC_EXPR_CUSTFUNC;
            lpExpr->u.CustFunc.hFunc = CustFuncCreateRef(hCustFunc);
            lpExpr->u.CustFunc.nArgs = lpRecCustFunc->nArgs;
            lInc = sizeof(SS_SUBREC_EXPRCUSTFUNC);
            break;
          case CALC_EXPR_OPERATOR:
            lpRecOper = (LPSS_SUBREC_EXPROPER)lpRecCommon;
            lpExpr->nType = CALC_EXPR_OPERATOR;
            lpExpr->u.Oper.nId = lpRecOper->nId;
            #if defined(SS_V70)
            if (lpExpr->nType == CALC_EXPR_OPERATOR && lpExpr->u.Oper.nId == CALC_OPER_RANGE)
            {
              if ((lpExpr-1)->nType == CALC_EXPR_CELL && (lpExpr-2)->nType == CALC_EXPR_CELL)
              {
                CALC_EXPRINFO temp;
                temp.nType = CALC_EXPR_RANGE;
                temp.u.Range.lCol1 = (lpExpr-2)->u.Cell.lCol;
                temp.u.Range.lRow1 = (lpExpr-2)->u.Cell.lRow;
                temp.u.Range.lCol2 = (lpExpr-1)->u.Cell.lCol;
                temp.u.Range.lRow2 = (lpExpr-1)->u.Cell.lRow;
                *(lpExpr-2) = temp;
                lpExpr -= 2;
              }
            }
            #endif
            lInc = sizeof(SS_SUBREC_EXPROPER);
            break;
          case CALC_EXPR_SEPARATOR:
            lpRecSep = (LPSS_SUBREC_EXPRSEP)lpRecCommon;
            lpExpr->nType = CALC_EXPR_SEPARATOR;
            lpExpr->u.Sep.nId = lpRecSep->nId;
            lInc = sizeof(SS_SUBREC_EXPRSEP);
            break;
          #if defined(SS_DDE)
          case CALC_EXPR_DDE:
            lpRecDde = (LPSS_SUBREC_EXPRDDE)lpRecCommon;
            if( lpSavedDdeLinks->lpElem )
              hDde = lpSavedDdeLinks->lpElem[lpRecDde->nId];
            else
              hDde = 0;
            lpExpr->nType = CALC_EXPR_DDE;
            lpExpr->u.Dde.hDde = DdeCreateRef(hDde);
            lpExpr->u.Dde.lItemCol = lpRecDde->lItemCol;
            lpExpr->u.Dde.lItemRow = lpRecDde->lItemRow;
            lInc = sizeof(SS_SUBREC_EXPRDDE);
            break;
          #endif
          case CALC_EXPR_ERROR:
            lpRecError = (LPSS_SUBREC_EXPRERROR)lpRecCommon;
            lpExpr->nType = CALC_EXPR_ERROR;
            lpExpr->u.Error.nId = lpRecError->nId;
            lInc = sizeof(SS_SUBREC_EXPRERROR);
            break;
          default:
            return 0;
        }
        lpExpr++;
        lpRecCommon = (LPSS_SUBREC_EXPRCOMMON)((HPBYTE)lpRecCommon + lInc);
      }
      lpExpr->nType = CALC_EXPR_END;
      CalcMemUnlock(hExpr);
    }
  }
  SS_BuffFree(&buff);
  return hExpr;
}
#endif

//--------------------------------------------------------------------
//
//  The SS_Save4Expr() functions appends a parsed expression to the
//  buffer.  The parsed expression is saved as a field within another
//  record.
//

#if !defined(SS_NOCALC) && !defined(SS_OLDCALC)
static BOOL SS_Save4Expr(LPSS_BUFF lpBuff, LPCALC_BOOK lpBook,
                         CALC_HANDLE hExpr)
{
  SS_SUBREC_EXPRLONG recLong;
  SS_SUBREC_EXPRDOUBLE recDouble;
  SS_SUBREC_EXPRSTR recString;
  SS_SUBREC_EXPRCELL recCell;
  #if defined(SS_V70)
  SS_SUBREC_EXPRRANGE recRange;
  SS_SUBREC_EXPREXTERNALCELL recExternalCell;
  SS_SUBREC_EXPREXTERNALRANGE recExternalRange;
  #endif
  SS_SUBREC_EXPRCUSTNAME recCustName;
  SS_SUBREC_EXPRFUNC recFunc;
  SS_SUBREC_EXPRCUSTFUNC recCustFunc;
  SS_SUBREC_EXPROPER recOper;
  SS_SUBREC_EXPRSEP recSep;
  #if defined(SS_DDE)
  SS_SUBREC_EXPRDDE recDde;
  #endif
  SS_SUBREC_EXPRERROR recError;
  LPCALC_EXPRINFO lpExpr;
  LPTSTR lpszText;
  BOOL bRet = TRUE;

  if( hExpr && (lpExpr = (LPCALC_EXPRINFO)CalcMemLock(hExpr)) )
  {
    for( ; lpExpr->nType != CALC_EXPR_END; lpExpr++ )
    {
      switch( lpExpr->nType )
      {
        case CALC_EXPR_LONG:
          recLong.nType = CALC_EXPR_LONG;
          recLong.lVal = lpExpr->u.Long.lVal;
          bRet &= SS_BuffAppend(lpBuff, &recLong, sizeof(recLong));
          break;
        case CALC_EXPR_DOUBLE:
          recDouble.nType = CALC_EXPR_DOUBLE;
          recDouble.dfVal = lpExpr->u.Double.dfVal;
          bRet &= SS_BuffAppend(lpBuff, &recDouble, sizeof(recDouble));
          break;
        case CALC_EXPR_STRING:
          lpszText = (LPTSTR)CalcMemLock(lpExpr->u.String.hText);
          recString.nType = CALC_EXPR_STRING;
          recString.nLen = (short)((lstrlen(lpszText) + 1) * sizeof(TCHAR));
          bRet &= SS_BuffAppend(lpBuff, &recString, sizeof(recString));
          bRet &= SS_BuffAppend(lpBuff, lpszText, recString.nLen);
          CalcMemUnlock(lpExpr->u.String.hText);
          break;
        case CALC_EXPR_CELL:
          recCell.nType = CALC_EXPR_CELL;
          recCell.lCol = lpExpr->u.Cell.lCol;
          recCell.lRow = lpExpr->u.Cell.lRow;
          bRet &= SS_BuffAppend(lpBuff, &recCell, sizeof(recCell));
          break;
        #if defined(SS_V70)
        case CALC_EXPR_RANGE:
          recRange.nType = CALC_EXPR_RANGE;
          recRange.lCol1 = lpExpr->u.Range.lCol1;
          recRange.lRow1 = lpExpr->u.Range.lRow1;
          recRange.lCol2 = lpExpr->u.Range.lCol2;
          recRange.lRow2 = lpExpr->u.Range.lRow2;
          bRet &= SS_BuffAppend(lpBuff, &recRange, sizeof(recRange));
          break;
        case CALC_EXPR_EXTERNALCELL:
          recExternalCell.nType = CALC_EXPR_EXTERNALCELL;
          recExternalCell.nSheet = SS_CalcGetIndex(lpExpr->u.ExternalCell.lpSheet->hSS);
          recExternalCell.lCol = lpExpr->u.ExternalCell.lCol;
          recExternalCell.lRow = lpExpr->u.ExternalCell.lRow;
          bRet &= SS_BuffAppend(lpBuff, &recExternalCell, sizeof(recExternalCell));
          break;
        case CALC_EXPR_EXTERNALRANGE:
          recExternalRange.nType = CALC_EXPR_EXTERNALRANGE;
          recExternalRange.nSheet = SS_CalcGetIndex(lpExpr->u.ExternalRange.lpSheet->hSS);
          recExternalRange.lCol1 = lpExpr->u.ExternalRange.lCol1;
          recExternalRange.lRow1 = lpExpr->u.ExternalRange.lRow1;
          recExternalRange.lCol2 = lpExpr->u.ExternalRange.lCol2;
          recExternalRange.lRow2 = lpExpr->u.ExternalRange.lRow2;
          bRet &= SS_BuffAppend(lpBuff, &recExternalRange, sizeof(recExternalRange));
          break;
        #endif
        case CALC_EXPR_NAME:
          recCustName.nType = CALC_EXPR_NAME;
          recCustName.nId = CustNameGetSaveId(lpBook, lpExpr->u.CustName.hName);
          bRet &= SS_BuffAppend(lpBuff, &recCustName, sizeof(recCustName));
          break;
        case CALC_EXPR_FUNCTION:
          recFunc.nType = CALC_EXPR_FUNCTION;
          recFunc.nId = lpExpr->u.Func.nId;
          recFunc.nArgs = lpExpr->u.Func.nArgs;
          bRet &= SS_BuffAppend(lpBuff, &recFunc, sizeof(recFunc));
          break;
        case CALC_EXPR_CUSTFUNC:
          recCustFunc.nType = CALC_EXPR_CUSTFUNC;
          recCustFunc.nId = CustFuncGetSaveId(lpBook, lpExpr->u.CustFunc.hFunc);
          recCustFunc.nArgs = lpExpr->u.CustFunc.nArgs;
          bRet &= SS_BuffAppend(lpBuff, &recCustFunc, sizeof(recCustFunc));
          break;
        case CALC_EXPR_OPERATOR:
          recOper.nType = CALC_EXPR_OPERATOR;
          recOper.nId = lpExpr->u.Oper.nId;
          bRet &= SS_BuffAppend(lpBuff, &recOper, sizeof(recOper));
          break;
        case CALC_EXPR_SEPARATOR:
          recSep.nType = CALC_EXPR_SEPARATOR;
          recSep.nId = lpExpr->u.Sep.nId;
          bRet &= SS_BuffAppend(lpBuff, &recSep, sizeof(recSep));
          break;
        #if defined(SS_DDE)
        case CALC_EXPR_DDE:
          recDde.nType = CALC_EXPR_DDE;
          recDde.nId = DdeGetSaveId(lpCalc, lpExpr->u.Dde.hDde);
          recDde.lItemCol = lpExpr->u.Dde.lItemCol;
          recDde.lItemRow = lpExpr->u.Dde.lItemRow;
          bRet &= SS_BuffAppend(lpBuff, &recDde, sizeof(recDde));
          break;
        #endif
        case CALC_EXPR_ERROR:
          recError.nType = CALC_EXPR_ERROR;
          recError.nId = lpExpr->u.Error.nId;
          bRet &= SS_BuffAppend(lpBuff, &recError, sizeof(recError));
          break;
      }
    }
    CalcMemUnlock(hExpr);
  }
  return bRet;
}
#endif
  
//--------------------------------------------------------------------
//
//  The SS_Load4CustNameTable() function retrieves a table of custom
//  names from the buffer.  The lpSavedCustFuncs and lpSavedCustNames
//  arrays are used to tranlate external ids into internal handles.
//

BOOL SS_Load4CustNameTable(LPSS_BOOK lpBook, LPSS_REC_COMMON lpCommon,
                           LPSS_CALCIDTABLE lpSavedCustFuncs,
                           LPSS_CALCIDTABLE lpSavedCustNames,
                           LPSS_CALCIDTABLE lpSavedDdeLinks,
                           BOOL bUnicode)
{
  #if defined(SS_NOCALC) || defined(SS_OLDCALC)
  return TRUE;
  #else
  LPSS_REC_CUSTNAMETABLE lpRec = (LPSS_REC_CUSTNAMETABLE)lpCommon;
  LPSS_SUBREC_CUSTNAME lpSubrec;
  SS_BUFF buff;
  HPBYTE lpName;
  HPBYTE lpExpr;
  LPTSTR lpszName;
  CALC_HANDLE hName;
  CALC_HANDLE hExpr;
  int i;
  BOOL bRet = (long)sizeof(*lpRec) + lpRec->lTableLen == lpRec->lRecLen;

//RAP02d  if( bRet )
  if( bRet  && lpRec->nTableCnt>0) //RAP02a
  {
    SS_BuffInit(&buff);
    if( lpSavedCustNames->hElem = tbGlobalAlloc(GHND,
                                                lpRec->nTableCnt *
                                                sizeof(CALC_HANDLE)) )
    {
      lpSavedCustNames->lpElem
        = (LPCALC_HANDLE)tbGlobalLock(lpSavedCustNames->hElem);
      lpSavedCustNames->nElemCnt = lpRec->nTableCnt;
      lpSubrec = (LPSS_SUBREC_CUSTNAME)((HPBYTE)lpRec + sizeof(*lpRec));
      for( i = 0; i < lpRec->nTableCnt; i++ )
      {
        lpName = (HPBYTE)lpSubrec + sizeof(*lpSubrec);
        lpszName = SS_BuffCopyStr(&buff, lpName, bUnicode);
        if( hName = NameLookup(&lpBook->CalcInfo.Names, lpszName) )
          hName = NameCreateRef(hName);
        else if( hName = NameCreate(&lpBook->CalcInfo, lpszName, NULL) )
          NameAdd(&lpBook->CalcInfo.Names, hName);
        lpSavedCustNames->lpElem[i] = hName;
        lpSubrec = (LPSS_SUBREC_CUSTNAME)((HPBYTE)lpSubrec +
                                          sizeof(*lpSubrec) +
                                          lpSubrec->lTextLen +
                                          lpSubrec->lExprLen);
      }
      lpSubrec = (LPSS_SUBREC_CUSTNAME)((HPBYTE)lpRec + sizeof(*lpRec));
      for( i = 0; i < lpRec->nTableCnt; i++ )
      {
        lpExpr = (HPBYTE)lpSubrec + sizeof(*lpSubrec) + lpSubrec->lTextLen;
        if( lpSubrec->lExprLen )
        {
          hExpr = SS_Load4Expr(lpBook, lpExpr, lpSubrec->lExprLen,
                               lpSavedCustFuncs, lpSavedCustNames,
                               lpSavedDdeLinks, bUnicode);
          NameSetExpr(lpSavedCustNames->lpElem[i], hExpr);
        }
        lpSubrec = (LPSS_SUBREC_CUSTNAME)((HPBYTE)lpSubrec +
                                          sizeof(*lpSubrec) +
                                          lpSubrec->lTextLen +
                                          lpSubrec->lExprLen);

      }
    }
    SS_BuffFree(&buff);
  }
  return bRet;
  #endif
}

//--------------------------------------------------------------------
//
//  The SS_Save4CustNameTable() saves all custom names which are
//  currently defined in the spreadsheet.
//

BOOL SS_Save4CustNameTable(LPSS_BOOK lpBook, BOOL bSaveExpr,
                           LPSS_BUFF lpBuff)
{
  #if defined(SS_NOCALC) || defined(SS_OLDCALC)
  return TRUE;
  #else
  SS_REC_CUSTNAMETABLE rec;
  SS_SUBREC_CUSTNAME subrec;
  LPCALC_HANDLE lpElem;
  LPCALC_NAME lpCustName;
  LPTSTR lpszText;
  long lStartRec = lpBuff->lLen;
  long lStartSubrec;
  long lStartTable;
  long lStartExpr;
  int i;
  BOOL bRet = TRUE;

  if( lpBook->CalcInfo.Names.hElem &&
      (lpElem = (LPCALC_HANDLE)CalcMemLock(lpBook->CalcInfo.Names.hElem)) )
  {
    rec.nRecType = SS_RID_CUSTNAMETABLE;
    rec.lRecLen = sizeof(rec);
    rec.nTableCnt = lpBook->CalcInfo.Names.nElemCnt;
    rec.lTableLen = 0;
    bRet = SS_BuffAppend(lpBuff, &rec, sizeof(rec));
    lStartTable = lpBuff->lLen;
    for( i = 0; bRet && i < lpBook->CalcInfo.Names.nElemCnt; i++ )
    {
      if( lpElem[i] && (lpCustName = (LPCALC_NAME)CalcMemLock(lpElem[i])) )
      {
        if( lpCustName->hText &&
            (lpszText = (LPTSTR)CalcMemLock(lpCustName->hText)) )
        {
          subrec.lTextLen = (lstrlen(lpszText) + 1) * sizeof(TCHAR);
          subrec.lExprLen = 0;
          lStartSubrec = lpBuff->lLen;
          bRet &= SS_BuffAppend(lpBuff, &subrec, sizeof(subrec));
          bRet &= SS_BuffAppend(lpBuff, lpszText, subrec.lTextLen);
          if( bSaveExpr )
          {
            lStartExpr = lpBuff->lLen;
            bRet &= SS_Save4Expr(lpBuff, &lpBook->CalcInfo, lpCustName->hExpr);
            if( bRet && lpBuff->pMem )
              MemHugeCpy(&subrec, lpBuff->pMem + lStartSubrec, sizeof(subrec));
            subrec.lExprLen = lpBuff->lLen - lStartExpr;
            if( bRet && lpBuff->pMem )
              MemHugeCpy(lpBuff->pMem + lStartSubrec, &subrec, sizeof(subrec));
          }
          CalcMemUnlock(lpCustName->hText);
        }
        CalcMemUnlock(lpElem[i]);
      }
    }
    if( bRet && lpBuff->pMem )
      MemHugeCpy(&rec, lpBuff->pMem + lStartRec, sizeof(rec));
    rec.lRecLen = lpBuff->lLen - lStartRec;
    rec.lTableLen = lpBuff->lLen - lStartTable;
    if( bRet && lpBuff->pMem )
      MemHugeCpy(lpBuff->pMem + lStartRec, &rec, sizeof(rec));
    CalcMemUnlock(lpBook->CalcInfo.Names.hElem);
  }
  return bRet;
  #endif
}

//--------------------------------------------------------------------
//
//  The SS_Load4DataAware() function loads a SS_REC_DATAAWARE record.
//

BOOL SS_Load4DataAware(LPSPREADSHEET lpSS, LPSS_BUFF lpXtra, LPSS_REC_COMMON lpCommon)
{
  LPSS_REC_DATAAWARE lpRec = (LPSS_REC_DATAAWARE)lpCommon;
  BOOL bRet = sizeof(*lpRec) == lpRec->lRecLen;
  if( bRet )
  {
#if (defined(SS_BOUNDCONTROL) || defined(SS_QE))

#ifdef SS_OCX  
    // Lock structures
    LPVBSPREAD lpSpread = 
      (LPVBSPREAD)SSOcxLockVBSpread((LONG)lpSS->lpBook->lpOleControl);
#else //not SS_OCX
    HCTL       hCtl = VBGetHwndControl(lpSS->lpBook->hWnd);
    LPVBSPREAD lpSpread = (LPVBSPREAD)VBDerefControl(hCtl);
#endif
    lpSS->DataFieldNameCnt = 0;
    lpSpread->DataOpts.fCellTypes = lpRec->bDAutoCellTypes;
    lpSpread->DataOpts.fAutoFill  = lpRec->bDAutoFill;
    lpSpread->DataOpts.fHeadings  = lpRec->bDAutoHeadings;
    lpSpread->DataOpts.fAutoSave  = lpRec->bDAutoSave;
    lpSpread->DataOpts.fSizeCols  = lpRec->bDAutoSizeCols;
    lpSpread->DataOpts.fAutoDataInform = lpRec->bDInformActiveRowChange;

#ifdef SS_OCX
   // Unlock structures
    SSOcxUnlockVBSpread((LONG)lpSS->lpBook->lpOleControl);
#endif

#else  // not bound - copy record to buffer
    bRet &= SS_BuffAppend(lpXtra, lpRec, lpRec->lRecLen);
#endif // SS_BOUNDCONTROL || SS_QE
  }
  return bRet;
}

//--------------------------------------------------------------------

#if (defined(SS_BOUNDCONTROL) || defined(SS_QE))
BOOL SS_Save4DataAware(LPSPREADSHEET lpSS, LPSS_BUFF lpBuff)
{
  BOOL bRet = TRUE;
#ifdef SS_OCX  
    // Lock structures
  LONG lObject = (LONG)lpSS->lpBook->lpOleControl;
    LPVBSPREAD lpSpread = (LPVBSPREAD)SSOcxLockVBSpread(lObject);
#else //not SS_OCX
  LONG lObject = (LONG)VBGetHwndControl(lpSS->lpBook->hWnd);
    LPVBSPREAD lpSpread = (LPVBSPREAD)VBDerefControl((HCTL)lObject);
#endif
  SS_REC_DATAAWARE rec;

  if( lpSpread->DataOpts.fCellTypes != TRUE || 
      lpSpread->DataOpts.fAutoFill != TRUE ||
      lpSpread->DataOpts.fHeadings != TRUE ||
      lpSpread->DataOpts.fAutoSave != TRUE ||
      lpSpread->DataOpts.fSizeCols != 2 ||
      lpSpread->DataOpts.fAutoDataInform != TRUE )
  {
    rec.nRecType = SS_RID_DATAAWARE;
    rec.lRecLen = sizeof(rec);
    rec.bDAutoCellTypes = lpSpread->DataOpts.fCellTypes;
    rec.bDAutoFill      = lpSpread->DataOpts.fAutoFill;
    rec.bDAutoHeadings  = lpSpread->DataOpts.fHeadings;
    rec.bDAutoSave      = lpSpread->DataOpts.fAutoSave;
    rec.bDAutoSizeCols  = lpSpread->DataOpts.fSizeCols;
    rec.bDInformActiveRowChange = lpSpread->DataOpts.fAutoDataInform;
    bRet = SS_BuffAppend(lpBuff, &rec, sizeof(rec));
  }
  
#ifdef SS_OCX
  // Unlock structures
  SSOcxUnlockVBSpread((LONG)lObject);
#endif

  return bRet;
}
#endif // SS_BOUNDCONTROL


//--------------------------------------------------------------------
//
//  The SS_Load4DataAwareCol() function loads a SS_REC_DATAAWARECOL
//  record.
//

BOOL SS_Load4DataAwareCol(LPSPREADSHEET lpSS, LPSS_BUFF lpXtra, 
                          LPSS_REC_COMMON lpCommon, BOOL bUnicode)
{
  LPSS_REC_DATAAWARECOL lpRec = (LPSS_REC_DATAAWARECOL)lpCommon;
  SS_BUFF buffDataField;
  HPBYTE lpDataField;
  LPTSTR lpszDataField;
  BOOL bRet = sizeof(*lpRec) <= lpRec->lRecLen;
  if( bRet )
  {
    SS_BuffInit(&buffDataField);
    lpDataField = (HPBYTE)lpRec + sizeof(*lpRec);
    lpszDataField = SS_BuffCopyStr(&buffDataField, lpDataField, bUnicode);

#if (defined(SS_BOUNDCONTROL) || defined(SS_V40))
  {
  LPSS_COL lpCol;

    if (lpCol = SS_AllocLockCol(lpSS, lpRec->lCol))
    {
      LONG lLen;
      lpCol->bDataFillEvent = lpRec->bDataFillEvent;

      // This is to take care of backward compatibility
      if (lpCol->bDataFillEvent && lpCol->bDataFillEvent != SS_DATAFILLEVENT_NO)
         lpCol->bDataFillEvent = SS_DATAFILLEVENT_YES;

      if (lLen = (long)_ftcslen(lpszDataField))
      {
        if (lpCol->hDBFieldName = tbGlobalAlloc(GHND, (lLen + 1) * sizeof(TCHAR)))
        {
          LPTSTR lpText = (LPTSTR)tbGlobalLock(lpCol->hDBFieldName);
          _ftcscpy(lpText, lpszDataField); 
          tbGlobalUnlock(lpCol->hDBFieldName);
          lpSS->DataFieldNameCnt++;
        }
      }
      SS_UnlockColItem(lpSS, lpRec->lCol);
    }
  }

#else  // not bound - copy record to buffer
    {
    SS_REC_DATAAWARECOL recTemp;
    
    recTemp = *lpRec;
    recTemp.lDataFieldLen = (_ftcslen(lpszDataField) + 1) * sizeof(TCHAR);
    recTemp.lRecLen = sizeof(recTemp) + recTemp.lDataFieldLen;
    bRet &= SS_BuffAppend(lpXtra, &recTemp, sizeof(recTemp));
    bRet &= SS_BuffAppend(lpXtra, lpszDataField, recTemp.lDataFieldLen);
    }
#endif // SS_BOUNDCONTROL || SS_V40

    SS_BuffFree(&buffDataField);
  }
  return bRet;
}

//--------------------------------------------------------------------

#if (defined(SS_BOUNDCONTROL) || defined(SS_V40))
BOOL SS_Save4DataAwareCol(LPSPREADSHEET lpSS, SS_COORD lCol,
                          LPSS_BUFF lpBuff)
{
  BOOL bRet = TRUE;
  SS_REC_DATAAWARECOL rec;
  LPSS_COL lpCol;

  if (lpCol = SS_LockColItem(lpSS, lCol))
  {
    LPTSTR lpszDataField = (LPTSTR)tbGlobalLock(lpCol->hDBFieldName);
    BOOL   fDataFieldLocked = TRUE;
    LONG   lDataFieldLen;

    // if no data, use null
    if (!lpszDataField)
    {
      lpszDataField = _T("");
      fDataFieldLocked = FALSE;     // not locked
    }
    
    lDataFieldLen = lstrlen(lpszDataField) + 1;
// RAP
//    if( lpCol->fDataFillEvent && lDataFieldLen > 1 )
    if( lpCol->bDataFillEvent || lDataFieldLen > 1 )
// RAP
    {  
      rec.nRecType = SS_RID_DATAAWARECOL;
      rec.lCol = lCol;
      rec.lRecLen = sizeof(rec) + lDataFieldLen * sizeof(TCHAR);
      rec.bDataFillEvent = lpCol->bDataFillEvent;
      rec.lDataFieldLen = lDataFieldLen * sizeof(TCHAR);
      bRet = SS_BuffAppend(lpBuff, &rec, sizeof(rec));
      bRet &= SS_BuffAppend(lpBuff, lpszDataField, 
                            lDataFieldLen * sizeof(TCHAR));
    }
    if (fDataFieldLocked)
      tbGlobalUnlock(lpCol->hDBFieldName);
  }

  return bRet;
}
#endif // SS_BOUNDCONTROL || SS_V40


//--------------------------------------------------------------------
//
//  The SS_Load4DataAwareQE() function loads a SS_REC_DATAAWAREQE
//  record.
//

BOOL SS_Load4DataAwareQE(LPSPREADSHEET lpSS, LPSS_BUFF lpXtra, 
                         LPSS_REC_COMMON lpCommon, BOOL bUnicode)
{
  LPSS_REC_DATAAWAREQE lpRec = (LPSS_REC_DATAAWAREQE)lpCommon;
  SS_BUFF buffDataConnect;
  SS_BUFF buffDataSelect;
  HPBYTE lpDataConnect;
  HPBYTE lpDataSelect;
  LPTSTR lpszDataConnect;
  LPTSTR lpszDataSelect;
  BOOL bRet = (long)sizeof(*lpRec) + lpRec->lDataConnectLen
              + lpRec->lDataSelectLen == lpRec->lRecLen;
  if( bRet )
  {
#if (defined(SS_QE))
    LPTSTR lpText;
    LONG   lLen;
#ifdef SS_OCX  
    // Lock structures
    LPVBSPREAD lpSpread = 
       (LPVBSPREAD)SSOcxLockVBSpread((LONG)lpSS->lpOleControl);
#else //not SS_OCX
    HCTL       hCtl = VBGetHwndControl(lpSS->lpBook->hWnd);
    LPVBSPREAD lpSpread = (LPVBSPREAD)VBDerefControl(hCtl);
#endif //SS_OCX
#endif //SS_QE

    SS_BuffInit(&buffDataConnect);
    SS_BuffInit(&buffDataSelect);
    lpDataConnect = (HPBYTE)lpRec + sizeof(*lpRec);
    lpDataSelect = lpDataConnect + lpRec->lDataConnectLen;
    lpszDataConnect = SS_BuffCopyStr(&buffDataConnect, lpDataConnect, bUnicode);
    lpszDataSelect = SS_BuffCopyStr(&buffDataSelect, lpDataSelect, bUnicode);

#if (defined(SS_QE))
    // Set DataConnect property
    if (lLen = _ftcslen(lpszDataConnect))
  {
      if (lpSpread->DBInfo.hDataConnect = tbGlobalAlloc(GHND,
                                          (lLen + 1) * sizeof(TCHAR)))
      {
        lpText = (LPTSTR)tbGlobalLock(lpSpread->DBInfo.hDataConnect);
    _ftcscpy(lpText, lpszDataConnect); 
        tbGlobalUnlock(lpSpread->DBInfo.hDataConnect);
      }
  }
    // Set DataSelect property
    if (lLen = _ftcslen(lpszDataSelect))
    {
      if (lpSpread->DBInfo.hDataSelect = tbGlobalAlloc(GHND,
            (lLen + 1) * sizeof(TCHAR)))
      {
        lpText = (LPTSTR)tbGlobalLock(lpSpread->DBInfo.hDataSelect);
    _ftcscpy(lpText, lpszDataSelect); 
        tbGlobalUnlock(lpSpread->DBInfo.hDataSelect);
      }
    }
#else  // not bound - copy record to buffer
    {
    SS_REC_DATAAWAREQE recTemp;
    
    recTemp = *lpRec;
    recTemp.lDataConnectLen = (long)((_ftcslen(lpszDataConnect) + 1) * sizeof(TCHAR));
    recTemp.lDataSelectLen = (long)((_ftcslen(lpszDataSelect) + 1) * sizeof(TCHAR));
    recTemp.lRecLen = sizeof(recTemp) + recTemp.lDataConnectLen
                      + recTemp.lDataSelectLen;
    bRet &= SS_BuffAppend(lpXtra, &recTemp, sizeof(recTemp));
    bRet &= SS_BuffAppend(lpXtra, lpszDataConnect, recTemp.lDataConnectLen);
    bRet &= SS_BuffAppend(lpXtra, lpszDataSelect, recTemp.lDataSelectLen);
    }
#endif // SS_QE
    SS_BuffFree(&buffDataConnect);
    SS_BuffFree(&buffDataSelect);

#if (defined(SS_OCX) && defined(SS_QE))  // not likely.
   // Unlock structures
   SSOcxUnlockVBSpread((LONG)lpSS->lpOleControl);
#endif
  }
  return bRet;
}


//--------------------------------------------------------------------

#if (defined(SS_QE))
BOOL SS_Save4DataAwareQE(LPSPREADSHEET lpSS, LPSS_BUFF lpBuff)
{
  BOOL bRet = TRUE;
#ifdef SS_OCX  
    // Lock structures
  LONG lObject = (LONG)lpSS->lpOleControl;
  LPVBSPREAD lpSpread = (LPVBSPREAD)SSOcxLockVBSpread(lObject);
#else //not SS_OCX
  LONG lObject = (LONG)VBGetHwndControl(lpSS->lpBook->hWnd);
  LPVBSPREAD lpSpread = (LPVBSPREAD)VBDerefControl((HCTL)lObject);
#endif
  SS_REC_DATAAWAREQE rec;

  LPTSTR lpszDataConnect = (LPTSTR)tbGlobalLock(lpSpread->DBInfo.hDataConnect);
  BOOL   fDataConnectLocked = TRUE;
  LONG   lDataConnectLen;
  LPTSTR lpszDataSelect = (LPTSTR)tbGlobalLock(lpSpread->DBInfo.hDataSelect);
  BOOL   fDataSelectLocked = TRUE;
  LONG   lDataSelectLen;
  
  // if no data, use null
  if (!lpszDataConnect)
  {
    lpszDataConnect = _T("");
    fDataConnectLocked = FALSE;     // not locked
  }
  // if no data, use null
  if (!lpszDataSelect)
  {
    lpszDataSelect = _T("");
    fDataSelectLocked = FALSE;     // not locked
  }
  lDataConnectLen = lstrlen(lpszDataConnect) + 1;
  lDataSelectLen = lstrlen(lpszDataSelect) + 1;

  rec.nRecType = SS_RID_DATAAWAREQE;
  rec.lRecLen = sizeof(rec) + 
                (lDataConnectLen + lDataSelectLen) * sizeof(TCHAR);
  rec.lDataConnectLen = lDataConnectLen;
  rec.lDataSelectLen = lDataSelectLen;
  bRet = SS_BuffAppend(lpBuff, &rec, sizeof(rec));
  bRet &= SS_BuffAppend(lpBuff, lpszDataConnect, 
                        lDataConnectLen * sizeof(TCHAR));
  bRet &= SS_BuffAppend(lpBuff, lpszDataSelect, 
                        lDataSelectLen * sizeof(TCHAR));
  if (fDataConnectLocked)
     tbGlobalUnlock(lpSpread->DBInfo.hDataConnect);
  if (fDataSelectLocked)
     tbGlobalUnlock(lpSpread->DBInfo.hDataSelect);
  
#ifdef SS_OCX
  // Unlock structures
  SSOcxUnlockVBSpread((LONG)lObject);
#endif

  return bRet;
}
#endif // SS_QE

//--------------------------------------------------------------------

BOOL SS_Load4DefFloatFormat(LPSPREADSHEET lpSS, LPSS_REC_COMMON lpCommon)
{
  LPSS_REC_DEFFLOATFORMAT lpRec = (LPSS_REC_DEFFLOATFORMAT)lpCommon;
  BOOL bRet = sizeof(*lpRec) == lpRec->lRecLen;
  if( bRet )
  {
    lpSS->lpBook->DefaultFloatFormat.cCurrencySign = (TCHAR)lpRec->nCurrChar;
    lpSS->lpBook->DefaultFloatFormat.cDecimalSign = (TCHAR)lpRec->nDecChar;
    lpSS->lpBook->DefaultFloatFormat.cSeparator = (TCHAR)lpRec->nSepChar;
  }
  return bRet;
}

//--------------------------------------------------------------------

BOOL SS_Save4DefFloatFormat(LPSPREADSHEET lpSS, LPSS_BUFF lpBuff)
{
  SS_REC_DEFFLOATFORMAT rec;
  BOOL bRet = TRUE;
  if( lpSS->lpBook->DefaultFloatFormat.cCurrencySign ||
      lpSS->lpBook->DefaultFloatFormat.cDecimalSign ||
      lpSS->lpBook->DefaultFloatFormat.cSeparator )
  {
    rec.nRecType = SS_RID_DEFFLOATFORMAT;
    rec.lRecLen = sizeof(rec);
    rec.nCurrChar = lpSS->lpBook->DefaultFloatFormat.cCurrencySign;
    rec.nDecChar = lpSS->lpBook->DefaultFloatFormat.cDecimalSign;
    rec.nSepChar = lpSS->lpBook->DefaultFloatFormat.cSeparator;
    bRet = SS_BuffAppend(lpBuff, &rec, sizeof(rec));
  }
  return bRet;
}

//--------------------------------------------------------------------
//
//  Note: The redraw option needs to be set after all the data
//        is loaded.
//

BOOL SS_Load4Display(LPSS_BOOK lpBook, LPSS_REC_COMMON lpCommon,
                    LPBOOL lpbRedraw)
{
  LPSS_REC_DISPLAY lpRec = (LPSS_REC_DISPLAY)lpCommon;
  BOOL bRet = sizeof(*lpRec) == lpRec->lRecLen;
  if( bRet )
  {
    lpBook->fAllowCellOverflow = lpRec->bAllowCellOverflow;
    lpBook->wButtonDrawMode = lpRec->wButtonDrawMode;
    *lpbRedraw = lpRec->bRedraw;
    lpBook->fRetainSelBlock = lpRec->bRetainSelBlock;
    lpBook->fNoBorder = lpRec->bNoBorder;
  }
  return bRet;
}

//--------------------------------------------------------------------

BOOL SS_Save4Display(LPSS_BOOK lpBook, LPSS_BUFF lpBuff)
{
  SS_REC_DISPLAY rec;
  BOOL bRet = TRUE;
#ifdef SS_OCX
  BOOL fRetainSelBlock = TRUE;
#else
  BOOL fRetainSelBlock = 0;
#endif // SS_OCX

// This is being removed because of the difference between the OCX and DLL in Designer.
//  if( lpSS->fAllowCellOverflow || lpSS->wButtonDrawMode ||
//      !lpSS->Redraw || lpSS->fRetainSelBlock != fRetainSelBlock || lpSS->fNoBorder )
  {
    rec.nRecType = SS_RID_DISPLAY;
    rec.lRecLen = sizeof(rec);
    rec.bAllowCellOverflow = lpBook->fAllowCellOverflow;
    rec.wButtonDrawMode = lpBook->wButtonDrawMode;
    rec.bRedraw = lpBook->Redraw;
    rec.bRetainSelBlock = lpBook->fRetainSelBlock;
    rec.bNoBorder = lpBook->fNoBorder;
    bRet = SS_BuffAppend(lpBuff, &rec, sizeof(rec));
  }
  return bRet;
}

//--------------------------------------------------------------------

BOOL SS_Load4Edit(LPSS_BOOK lpBook, LPSS_REC_COMMON lpCommon)
{
  LPSS_REC_EDIT lpRec = (LPSS_REC_EDIT)lpCommon;
  BOOL bRet = sizeof(*lpRec) == lpRec->lRecLen;
  if( bRet )
  {
    lpBook->fAllowDragDrop = lpRec->bAllowDragDrop;
    lpBook->fAllowMultipleSelBlocks = lpRec->bAllowMultiBlocks;
    lpBook->fArrowsExitEditMode = lpRec->bArrowsExitEditMode;
    lpBook->fAutoClipboard = lpRec->bAutoClipboard;
    lpBook->wEnterAction = lpRec->wEditEnterAction;
    lpBook->fEditModePermanent = lpRec->bEditModePermanent;
    lpBook->fEditModeReplace = lpRec->bEditModeReplace;
    lpBook->fMoveActiveOnFocus = lpRec->bMoveActiveOnFocus;
    lpBook->fNoBeep = lpRec->bNoBeep;
    lpBook->fProcessTab = lpRec->bProcessTab;
  }
  return bRet;
}

//--------------------------------------------------------------------

BOOL SS_Save4Edit(LPSS_BOOK lpBook, LPSS_BUFF lpBuff)
{
  SS_REC_EDIT rec;
  BOOL bRet = TRUE;
  if( lpBook->fAllowDragDrop || lpBook->fAllowMultipleSelBlocks ||
      lpBook->fArrowsExitEditMode || !lpBook->fAutoClipboard ||
      lpBook->wEnterAction || lpBook->fEditModePermanent ||
      lpBook->fEditModeReplace || !lpBook->fMoveActiveOnFocus ||
      lpBook->fNoBeep || lpBook->fProcessTab )
  {
    rec.nRecType = SS_RID_EDIT;
    rec.lRecLen = sizeof(rec);
    rec.bAllowDragDrop = lpBook->fAllowDragDrop;
    rec.bAllowMultiBlocks = lpBook->fAllowMultipleSelBlocks;
    rec.bArrowsExitEditMode = lpBook->fArrowsExitEditMode;
    rec.bAutoClipboard =  lpBook->fAutoClipboard;
    rec.wEditEnterAction = lpBook->wEnterAction;
    rec.bEditModePermanent = lpBook->fEditModePermanent;
    rec.bEditModeReplace = lpBook->fEditModeReplace;
    rec.bMoveActiveOnFocus = lpBook->fMoveActiveOnFocus;
    rec.bNoBeep = lpBook->fNoBeep;
    rec.bProcessTab = lpBook->fProcessTab;
    bRet = SS_BuffAppend(lpBuff, &rec, sizeof(rec));
  }
  return bRet;
}

//--------------------------------------------------------------------

BOOL SS_Load4Font(LPSPREADSHEET lpSS, LPSS_REC_COMMON lpCommon,
                  LPSS_FONTBUFFER lpFontBuff, LPSS_LOADBLOCK lpLoadBlock)
{
  LPSS_REC_FONT lpRec = (LPSS_REC_FONT)lpCommon;
  SS_COORD lCol, lRow;
  BOOL bRet = sizeof(*lpRec) == lpRec->lRecLen;
  if( bRet &&
      BlockTranslate(lpLoadBlock, lpRec->lCol, lpRec->lRow, &lCol, &lRow) )
  {
    if( lpFontBuff->phFont )
      SS_SetFontRange(lpSS, lCol, lRow, lCol, lRow,
                      lpFontBuff->phFont[lpRec->nFontId], FALSE);
  }
  return bRet;
}

//--------------------------------------------------------------------
//
//  In the spreadsheet, -1 represents undefined font.
//  In the spreadsheet, the font table has a base index of one.
//  In the saved buffer, the font table has a base index of zero.
//

BOOL SS_Save4Font(SS_COORD lCol, SS_COORD lRow, SS_FONTID nFontId,
                  LPSS_BUFF lpBuff)
{
  SS_REC_FONT rec;
  BOOL bRet = TRUE;
  if( -1 != nFontId )
  {
    rec.nRecType = SS_RID_FONT;
    rec.lRecLen = sizeof(rec);
    rec.lCol = lCol;
    rec.lRow = lRow;
    rec.nFontId = nFontId - 1;
    bRet = SS_BuffAppend(lpBuff, &rec, sizeof(rec));
  }
  return bRet;
}

//--------------------------------------------------------------------
//
//  The SS_Load4FontTable() function loads the list of fonts used
//  by the spreadsheet.  The font height is saved in units of
//  1 / HEIGHT_UNITS_PER_POINT of a point.
//
//  Note: dyPixelsPerInch is defined in FPTOOLS library.
//

#define POINTS_PER_INCH        72
#define HEIGHT_UNITS_PER_POINT 100
#define HEIGHT_UNITS_PER_INCH  (HEIGHT_UNITS_PER_POINT * POINTS_PER_INCH)

BOOL SS_Load4FontTable(LPSPREADSHEET lpSS, LPSS_REC_COMMON lpCommon,
                      LPSS_FONTBUFFER lpFontBuff, BOOL bUnicode)
{
  LPSS_REC_FONTTABLE lpRec = (LPSS_REC_FONTTABLE)lpCommon;
  LPSS_SUBREC_LOGFONT lpSubRec;
  LOGFONT logFont;
  HPBYTE lpFaceName;
  LPTSTR lpszFaceName;
  SS_BUFF buff;
  int i;
  BOOL bRet = sizeof(*lpRec) <= lpRec->lRecLen;
  if( bRet )
  {
    lpFontBuff->hMem = GlobalAlloc(GHND, lpRec->nTableCnt * sizeof(HFONT));
    lpFontBuff->phFont = (LPHFONT)GlobalLock(lpFontBuff->hMem);
    lpFontBuff->lLen = lpRec->nTableCnt;
    lpFontBuff->lAlloc = lpRec->nTableCnt;
    if( lpFontBuff->phFont )
    {
      SS_BuffInit(&buff);
      lpSubRec = (LPSS_SUBREC_LOGFONT)((HPBYTE)lpRec + sizeof(*lpRec));
      for( i = 0; i < lpRec->nTableCnt; i++ )
      {
        lpFaceName = (HPBYTE)lpSubRec + sizeof(*lpSubRec);
        lpszFaceName = SS_BuffCopyStr(&buff, lpFaceName, bUnicode);
        logFont.lfHeight = MulDiv((int)lpSubRec->lfHeight, dyPixelsPerInch,
                                  HEIGHT_UNITS_PER_INCH);
        logFont.lfWidth = 0;
        logFont.lfEscapement = (int)lpSubRec->lfEscapement;
        logFont.lfOrientation = (int)lpSubRec->lfOrientation;
        logFont.lfWeight = (int)lpSubRec->lfWeight;
        logFont.lfItalic = lpSubRec->lfItalic;
        logFont.lfUnderline = lpSubRec->lfUnderline;
        logFont.lfStrikeOut = lpSubRec->lfStrikeOut;
        logFont.lfCharSet = lpSubRec->lfCharSet;
        logFont.lfOutPrecision = lpSubRec->lfOutPrecision;
        logFont.lfClipPrecision = lpSubRec->lfClipPrecision;
        logFont.lfQuality = lpSubRec->lfQuality;
        logFont.lfPitchAndFamily = lpSubRec->lfPitchAndFamily;
        lstrcpy(logFont.lfFaceName, lpszFaceName);
        lpFontBuff->phFont[i] = CreateFontIndirect(&logFont);
        lpSubRec = (LPSS_SUBREC_LOGFONT)((HPBYTE)lpSubRec +
                                         sizeof(*lpSubRec) +
                                         lpSubRec->lfFaceNameLen);
      }
      SS_BuffFree(&buff);
    }
    bRet = 0 != lpFontBuff->hMem && 0 != lpFontBuff->phFont;
  }
  return bRet;  
}

//--------------------------------------------------------------------
//
//  The SS_Save4FontTable() saves the list of fonts currently being
//  used in the spreadsheet.  The font height is saved in units
//  of 1 / SS_SAVE4_FONTPTSCALE of a point.
//
//  Note: dyPixelsPerInch is defined in FPTOOLS library.
//

BOOL SS_Save4FontTable(LPSS_BUFF lpBuff)
{
  SS_REC_FONTTABLE rec;
  SS_SUBREC_LOGFONT logFont;
  LPSS_FONT lpFontTable;
  long lBuffLenOld = lpBuff->lLen;
  int i;
  BOOL bRet = TRUE;  
  if( FontTable.dTableCnt )
  {
    lpFontTable = SS_FontTableLock();
    rec.nRecType = SS_RID_FONTTABLE;
    rec.lRecLen = sizeof(rec);
    rec.nTableCnt = FontTable.dTableCnt;
    bRet = SS_BuffAppend(lpBuff, &rec, sizeof(rec));
    for( i = 0; i < rec.nTableCnt; i++ )
    {
      logFont.lfHeight = MulDiv(lpFontTable[i].LogFont.lfHeight,
                                HEIGHT_UNITS_PER_INCH, dyPixelsPerInch);
      logFont.lfEscapement = lpFontTable[i].LogFont.lfEscapement;
      logFont.lfOrientation = lpFontTable[i].LogFont.lfOrientation;
      logFont.lfWeight = lpFontTable[i].LogFont.lfWeight;
      logFont.lfItalic = lpFontTable[i].LogFont.lfItalic;
      logFont.lfUnderline = lpFontTable[i].LogFont.lfUnderline;
      logFont.lfStrikeOut = lpFontTable[i].LogFont.lfStrikeOut;
      logFont.lfCharSet = lpFontTable[i].LogFont.lfCharSet;
      logFont.lfOutPrecision = lpFontTable[i].LogFont.lfOutPrecision;
      logFont.lfClipPrecision = lpFontTable[i].LogFont.lfClipPrecision;
      logFont.lfQuality = lpFontTable[i].LogFont.lfQuality;
      logFont.lfPitchAndFamily = lpFontTable[i].LogFont.lfPitchAndFamily;
      logFont.lfFaceNameLen = (lstrlen(lpFontTable[i].LogFont.lfFaceName) + 1)
                               * sizeof(TCHAR);
      bRet &= SS_BuffAppend(lpBuff, &logFont, sizeof(logFont));
      bRet &= SS_BuffAppend(lpBuff, lpFontTable[i].LogFont.lfFaceName,
                            logFont.lfFaceNameLen);
    }
    if( bRet && lpBuff->pMem )
      MemHugeCpy(&rec, lpBuff->pMem + lBuffLenOld, sizeof(rec));
    rec.lRecLen = lpBuff->lLen - lBuffLenOld;
    if( bRet && lpBuff->pMem )
      MemHugeCpy(lpBuff->pMem + lBuffLenOld, &rec, sizeof(rec));
    SS_FontTableUnlock();
  }
  return bRet;
}

//--------------------------------------------------------------------
//
//  The SS_Load4FontTableOld() function was used during beta testing
//  to load the fonts used by the spreadsheet.  The font height is
//  saved in units of points.
//

BOOL SS_Load4FontTableOld(LPSPREADSHEET lpSS, LPSS_REC_COMMON lpCommon,
                          LPSS_FONTBUFFER lpFontBuff, BOOL bUnicode)
{
  LPSS_REC_FONTTABLEOLD lpRec = (LPSS_REC_FONTTABLEOLD)lpCommon;
  LPSS_SUBREC_LOGFONTOLD lpSubRec;
  LOGFONT logFont;
  HPBYTE lpFaceName;
  LPTSTR lpszFaceName;
  SS_BUFF buff;
  int i;
  BOOL bRet = sizeof(*lpRec) <= lpRec->lRecLen;
  if( bRet )
  {
    lpFontBuff->hMem = GlobalAlloc(GHND, lpRec->nTableCnt * sizeof(HFONT));
    lpFontBuff->phFont = (LPHFONT)GlobalLock(lpFontBuff->hMem);
    lpFontBuff->lLen = lpRec->nTableCnt;
    lpFontBuff->lAlloc = lpRec->nTableCnt;
    if( lpFontBuff->phFont )
    {
      SS_BuffInit(&buff);
      lpSubRec = (LPSS_SUBREC_LOGFONTOLD)((HPBYTE)lpRec + sizeof(*lpRec));
      for( i = 0; i < lpRec->nTableCnt; i++ )
      {
        lpFaceName = (HPBYTE)lpSubRec + sizeof(*lpSubRec);
        lpszFaceName = SS_BuffCopyStr(&buff, lpFaceName, bUnicode);
        logFont.lfHeight = INTPT_TO_PIXELS((int)lpSubRec->lfHeight);
        logFont.lfWidth = 0;
        logFont.lfEscapement = (int)lpSubRec->lfEscapement;
        logFont.lfOrientation = (int)lpSubRec->lfOrientation;
        logFont.lfWeight = (int)lpSubRec->lfWeight;
        logFont.lfItalic = lpSubRec->lfItalic;
        logFont.lfUnderline = lpSubRec->lfUnderline;
        logFont.lfStrikeOut = lpSubRec->lfStrikeOut;
        logFont.lfCharSet = lpSubRec->lfCharSet;
        logFont.lfOutPrecision = lpSubRec->lfOutPrecision;
        logFont.lfClipPrecision = lpSubRec->lfClipPrecision;
        logFont.lfQuality = lpSubRec->lfQuality;
        logFont.lfPitchAndFamily = lpSubRec->lfPitchAndFamily;
        lstrcpy(logFont.lfFaceName, lpszFaceName);
        lpFontBuff->phFont[i] = CreateFontIndirect(&logFont);
        lpSubRec = (LPSS_SUBREC_LOGFONTOLD)((HPBYTE)lpSubRec +
                                            sizeof(*lpSubRec) +
                                            lpSubRec->lfFaceNameLen);
      }
      SS_BuffFree(&buff);
    }
    bRet = 0 != lpFontBuff->hMem && 0 != lpFontBuff->phFont;
  }
  return bRet;  
}

//--------------------------------------------------------------------
//
//  The SS_Save4FontTableOld() function was used during beta testing
//  to save the list of fonts currentlly being used in spreadsheet.
//  The font height is saved in units of points.
//

/*
BOOL SS_Save4FontTableOld(LPSPREADSHEET lpSS, LPSS_BUFF lpBuff)
{
  SS_REC_FONTTABLEOLD rec;
  LPSS_REC_FONTTABLEOLD lpRec;
  SS_SUBREC_LOGFONTOLD logFont;
  LPSS_FONT lpFontTable;
  long lBuffLenOld = lpBuff->lLen;
  int i;
  BOOL bRet = TRUE;  
  if( FontTable.dTableCnt )
  {
    lpFontTable = SS_FontTableLock();
    rec.nRecType = SS_RID_FONTTABLEOLD;
    rec.lRecLen = sizeof(rec);
    rec.nTableCnt = FontTable.dTableCnt;
    bRet = SS_BuffAppend(lpBuff, &rec, sizeof(rec));
    for( i = 0; i < rec.nTableCnt; i++ )
    {
      logFont.lfHeight = (long)PIXELS_TO_INTPT(lpFontTable[i].LogFont.lfHeight);
      logFont.lfWidth = 0;
      logFont.lfEscapement = lpFontTable[i].LogFont.lfEscapement;
      logFont.lfOrientation = lpFontTable[i].LogFont.lfOrientation;
      logFont.lfWeight = lpFontTable[i].LogFont.lfWeight;
      logFont.lfItalic = lpFontTable[i].LogFont.lfItalic;
      logFont.lfUnderline = lpFontTable[i].LogFont.lfUnderline;
      logFont.lfStrikeOut = lpFontTable[i].LogFont.lfStrikeOut;
      logFont.lfCharSet = lpFontTable[i].LogFont.lfCharSet;
      logFont.lfOutPrecision = lpFontTable[i].LogFont.lfOutPrecision;
      logFont.lfClipPrecision = lpFontTable[i].LogFont.lfClipPrecision;
      logFont.lfQuality = lpFontTable[i].LogFont.lfQuality;
      logFont.lfPitchAndFamily = lpFontTable[i].LogFont.lfPitchAndFamily;
      logFont.lfFaceNameLen = (lstrlen(lpFontTable[i].LogFont.lfFaceName) + 1)
                               * sizeof(TCHAR);
      bRet &= SS_BuffAppend(lpBuff, &logFont, sizeof(logFont));
      bRet &= SS_BuffAppend(lpBuff, lpFontTable[i].LogFont.lfFaceName,
                            logFont.lfFaceNameLen);
    }
    lpRec = (LPSS_REC_FONTTABLE)(lpBuff->pMem + lBuffLenOld);
    lpRec->lRecLen = lpBuff->lLen - lBuffLenOld;
    SS_FontTableUnlock();
  }
  return bRet;
}
*/

//--------------------------------------------------------------------

BOOL SS_Load4Formula(LPSPREADSHEET lpSS, LPSS_REC_COMMON lpCommon,
                     LPSS_CALCIDTABLE lpSavedCustFuncs,
                     LPSS_CALCIDTABLE lpSavedCustNames,
                     LPSS_CALCIDTABLE lpSavedDdeLinks,
                     BOOL bUnicode, LPSS_LOADBLOCK lpLoadBlock)
{
  #if defined(SS_NOCALC) || defined(SS_OLDCALC)
  return TRUE;
  #else
  LPSS_REC_FORMULA lpRec = (LPSS_REC_FORMULA)lpCommon;
  CALC_HANDLE hExprPrev;
  CALC_HANDLE hExpr;
  HPBYTE lpExpr;
  long lExprLen = lpRec->lRecLen - sizeof(*lpRec);
  SS_COORD lCol, lRow;
  BOOL bRet = (long)sizeof(*lpRec) + lpRec->lExprLen == lpRec->lRecLen;

  if( bRet &&
      BlockTranslate(lpLoadBlock, lpRec->lCol, lpRec->lRow, &lCol, &lRow) )
  {
    SS_AdjustCellCoordsOut(lpSS, &lCol, &lRow);
    lpExpr = (HPBYTE)lpRec + sizeof(*lpRec);
    hExpr = SS_Load4Expr(lpSS->lpBook, lpExpr, lExprLen,
                         lpSavedCustFuncs, lpSavedCustNames,
                         lpSavedDdeLinks, bUnicode);
    if( lCol != CALC_ALLCOLS && lRow != CALC_ALLROWS )
    {
      hExprPrev = CellSetExpr(&lpSS->CalcInfo, lCol, lRow, hExpr);
      if (lpRec->bNeedsEval)
      CalcMarkCellForEval(&lpSS->CalcInfo, lCol, lRow);
      bRet = TRUE;
    }
    else if( lRow != CALC_ALLROWS )
    {
      hExprPrev = RowSetExpr(&lpSS->CalcInfo, lRow, hExpr);
      if (lpRec->bNeedsEval)
        CalcMarkRowForEval(&lpSS->CalcInfo, lRow);
      bRet = TRUE;
    }
    else if( lCol != CALC_ALLCOLS )
    {
      hExprPrev = ColSetExpr(&lpSS->CalcInfo, lCol, hExpr);
      if (lpRec->bNeedsEval)
        CalcMarkColForEval(&lpSS->CalcInfo, lCol);
      bRet = TRUE;
    }
    ExprDestroy(hExprPrev);
  }
  return bRet;
  #endif
}

//--------------------------------------------------------------------

#if !defined(SS_NOCALC) && !defined(SS_OLDCALC)
BOOL SS_Save4Formula(LPSPREADSHEET lpSS, SS_COORD lCol, SS_COORD lRow,
                     CALC_HANDLE hExpr, BOOL bNeedsEval, LPSS_BUFF lpBuff)
{
  SS_REC_FORMULA rec;
  long lStartRec = lpBuff->lLen;
  long lStartExpr;
  BOOL bRet = TRUE;

  if( hExpr )
  {
    rec.nRecType = SS_RID_FORMULA;
    rec.lRecLen = sizeof(rec);
    rec.lCol = lCol;
    rec.lRow = lRow;
    rec.bAlwaysCalc = FALSE;
    rec.bNeedsEval = bNeedsEval;
    bRet &= SS_BuffAppend(lpBuff, &rec, sizeof(rec));
    lStartExpr = lpBuff->lLen;
    bRet &= SS_Save4Expr(lpBuff, &lpSS->lpBook->CalcInfo, hExpr);
    if( bRet && lpBuff->pMem )
      MemHugeCpy(&rec, lpBuff->pMem + lStartRec, sizeof(rec));
    rec.lRecLen = lpBuff->lLen - lStartRec;
    rec.lExprLen = lpBuff->lLen - lStartExpr;    
    if( bRet && lpBuff->pMem )
      MemHugeCpy(lpBuff->pMem + lStartRec, &rec, sizeof(rec));
  }
  return bRet;
}
#endif

//--------------------------------------------------------------------

BOOL SS_Load4FormulaUnparsed(LPSPREADSHEET lpSS, LPSS_REC_COMMON lpCommon,
                             BOOL bUnicode, LPSS_LOADBLOCK lpLoadBlock)
{
  LPSS_REC_FORMULAUNPARSED lpRec = (LPSS_REC_FORMULAUNPARSED)lpCommon;
  SS_COORD lCol, lRow;
  BOOL bRet = sizeof(*lpRec) <= lpRec->lRecLen;
  if( bRet &&
      BlockTranslate(lpLoadBlock, lpRec->lCol, lpRec->lRow, &lCol, &lRow) )
  {
    SS_BUFF buff;
    SS_BuffInit(&buff);
    SS_BuffCopyStr(&buff, (HPBYTE)lpRec + sizeof(*lpRec), bUnicode);
    #ifndef SS_NOCALC
    SS_SetFormulaRange(lpSS, lCol, lRow, lCol, lRow,
                       (LPTSTR)buff.pMem, FALSE);
    #endif
    SS_BuffFree(&buff);
  }
  return bRet;
}

//--------------------------------------------------------------------

#ifdef SS_OLDCALC
BOOL SS_Save4FormulaUnparsed(SS_COORD lCol, SS_COORD lRow,
                             LPTSTR lpszFormula, LPSS_BUFF lpBuff)
{
  SS_REC_FORMULAUNPARSED rec;
  long lFormulaLen;
  BOOL bRet = TRUE;
  if( lpszFormula && lpszFormula[0] )
  {
    lFormulaLen = lstrlen(lpszFormula) + 1;
    rec.nRecType = SS_RID_FORMULAUNPARSED;
    rec.lRecLen = sizeof(rec) + lFormulaLen * sizeof(TCHAR);
    rec.lCol = lCol;
    rec.lRow = lRow;
    bRet = SS_BuffAppend(lpBuff, &rec, sizeof(rec));
    bRet &= SS_BuffAppend(lpBuff, lpszFormula, lFormulaLen * sizeof(TCHAR));
  }
  return bRet;
}
#endif

//--------------------------------------------------------------------

#ifdef SS_OLDCALC
BOOL SS_Save4FormulaUnparsedHandle(SS_COORD lCol, SS_COORD lRow,
                                   TBGLOBALHANDLE hCalc, LPSS_BUFF lpBuff)
{
  LPSS_CALC lpCalc;
  LPTSTR lpszFormula;
  BOOL bRet = TRUE;
  if( hCalc )
  {
    lpCalc = (LPSS_CALC)tbGlobalLock(hCalc);
    if( lpCalc->hFormula )
    {
      lpszFormula = (LPTSTR)tbGlobalLock(lpCalc->hFormula);
      bRet = SS_Save4FormulaUnparsed(lCol, lRow, lpszFormula, lpBuff);
      tbGlobalUnlock(lpCalc->hFormula);
    }
    tbGlobalUnlock(hCalc);
  }
  return bRet;
}
#endif

//--------------------------------------------------------------------

BOOL SS_Load4Frozen(LPSPREADSHEET lpSS, LPSS_REC_COMMON lpCommon)
{
  LPSS_REC_FROZEN lpRec = (LPSS_REC_FROZEN)lpCommon;
  BOOL bRet = sizeof(*lpRec) == lpRec->lRecLen;
  if( bRet )
  {
    SS_SetFreeze(lpSS, lpRec->lColsFrozen, lpRec->lRowsFrozen);
  }
  return bRet;
}

//--------------------------------------------------------------------

BOOL SS_Save4Frozen(LPSPREADSHEET lpSS, LPSS_BUFF lpBuff)
{
  SS_REC_FROZEN rec;
  BOOL bRet = TRUE;
  if( lpSS->Col.Frozen || lpSS->Row.Frozen )
  {
    rec.nRecType = SS_RID_FROZEN;
    rec.lRecLen = sizeof(rec);
    rec.lColsFrozen = lpSS->Col.Frozen;
    rec.lRowsFrozen = lpSS->Row.Frozen;
    bRet = SS_BuffAppend(lpBuff, &rec, sizeof(rec));
  }
  return bRet;
}

//--------------------------------------------------------------------

BOOL SS_Load4GrayAreaColor(LPSS_BOOK lpBook, LPSS_REC_COMMON lpCommon)
{
  LPSS_REC_GRAYAREACOLOR lpRec = (LPSS_REC_GRAYAREACOLOR)lpCommon;
  BOOL bRet = sizeof(*lpRec) == lpRec->lRecLen;
  if( bRet )
  {
    lpBook->GrayAreaBackground = lpRec->crBackColor;
    lpBook->GrayAreaForeground = lpRec->crForeColor;
  }
  return bRet;
}

//--------------------------------------------------------------------

BOOL SS_Save4GrayAreaColor(LPSS_BOOK lpBook, LPSS_BUFF lpBuff)
{
  SS_REC_GRAYAREACOLOR rec;
  BOOL bRet = TRUE;
  if( RGBCOLOR_PALEGRAY != lpBook->GrayAreaBackground ||
      0 != lpBook->GrayAreaForeground )
  {
    rec.nRecType = SS_RID_GRAYAREACOLOR;
    rec.lRecLen = sizeof(rec);
    rec.crBackColor = lpBook->GrayAreaBackground;
    rec.crForeColor = lpBook->GrayAreaForeground;
    bRet = SS_BuffAppend(lpBuff, &rec, sizeof(rec));
  }
  return bRet;
}

//--------------------------------------------------------------------

BOOL SS_Load4Grid(LPSPREADSHEET lpSS, LPSS_REC_COMMON lpCommon)
{
  LPSS_REC_GRID lpRec = (LPSS_REC_GRID)lpCommon;
  BOOL bRet = sizeof(*lpRec) == lpRec->lRecLen;
  if( bRet )
  {
    lpSS->wGridType = lpRec->wGridType;
    lpSS->GridColor = lpRec->crGridColor;
  }
  return bRet;
}

//--------------------------------------------------------------------

BOOL SS_Save4Grid(LPSPREADSHEET lpSS, LPSS_BUFF lpBuff)
{
  SS_REC_GRID rec;
  BOOL bRet = TRUE;
  if( (SS_GRID_HORIZONTAL | SS_GRID_VERTICAL | SS_GRID_SOLID) != lpSS->wGridType ||
      RGBCOLOR_PALEGRAY != lpSS->GridColor )
  {
    rec.nRecType = SS_RID_GRID;
    rec.lRecLen = sizeof(rec);
    rec.wGridType = lpSS->wGridType;
    rec.crGridColor = lpSS->GridColor;
    bRet = SS_BuffAppend(lpBuff, &rec, sizeof(rec));
  }
  return bRet;
}

//--------------------------------------------------------------------

BOOL SS_Load4Header(LPSPREADSHEET lpSS, LPSS_REC_COMMON lpCommon)
{
  LPSS_REC_HEADER lpRec = (LPSS_REC_HEADER)lpCommon;
  BOOL bRet = sizeof(*lpRec) == lpRec->lRecLen;
  if( bRet )
  {
    lpSS->ColHeaderDisplay = lpRec->wColHeaderDisplay;
    lpSS->RowHeaderDisplay = lpRec->wRowHeaderDisplay;
    lpSS->Col.NumStart = lpRec->lStartColNum;
    lpSS->Row.NumStart = lpRec->lStartRowNum;
  }
  return bRet;
}

//--------------------------------------------------------------------

BOOL SS_Save4Header(LPSPREADSHEET lpSS, LPSS_BUFF lpBuff)
{
  SS_REC_HEADER rec;
  BOOL bRet = TRUE;
  if( SS_HEADERDISPLAY_LETTERS != lpSS->ColHeaderDisplay ||
      SS_HEADERDISPLAY_NUMBERS != lpSS->RowHeaderDisplay ||
      1 != lpSS->Col.NumStart ||
      1 != lpSS->Row.NumStart )
  {
    rec.nRecType = SS_RID_HEADER;
    rec.lRecLen = sizeof(rec);
    rec.wColHeaderDisplay = lpSS->ColHeaderDisplay;
    rec.wRowHeaderDisplay = lpSS->RowHeaderDisplay;
    rec.lStartColNum = lpSS->Col.NumStart;
    rec.lStartRowNum = lpSS->Row.NumStart;
    bRet = SS_BuffAppend(lpBuff, &rec, sizeof(rec));
  }
  return bRet;
}

//--------------------------------------------------------------------

BOOL SS_Load4ItemData(LPSPREADSHEET lpSS, LPSS_REC_COMMON lpCommon)
{
  LPSS_REC_ITEMDATA lpRec = (LPSS_REC_ITEMDATA)lpCommon;
  BOOL bRet = sizeof(*lpRec) == lpRec->lRecLen;
  if( bRet )
  {
    if( SS_ALLCOLS == lpRec->lCol  && SS_ALLROWS == lpRec->lRow )
      SS_SetUserData(lpSS, lpRec->lItemData);
    #ifndef SS_NOUSERDATA
    else if( SS_ALLCOLS == lpRec->lCol )
      SS_SetRowUserData(lpSS, lpRec->lRow, lpRec->lItemData);
    else if( SS_ALLROWS == lpRec->lRow )
      SS_SetColUserData(lpSS, lpRec->lCol, lpRec->lItemData);
    #endif
  }
  return bRet;
}

//--------------------------------------------------------------------

BOOL SS_Save4ItemData(SS_COORD lCol, SS_COORD lRow, long lItemData,
                      LPSS_BUFF lpBuff)
{
  SS_REC_ITEMDATA rec;
  BOOL bRet = TRUE;
  if( lItemData )
  {
    rec.nRecType = SS_RID_ITEMDATA;
    rec.lRecLen = sizeof(rec);
    rec.lCol = lCol;
    rec.lRow = lRow;
    rec.lItemData = lItemData;
    bRet = SS_BuffAppend(lpBuff, &rec, sizeof(rec));
  }
  return bRet;
}

#if defined(_WIN64) || defined(_IA64)
BOOL SS_Load4ItemData64(LPSPREADSHEET lpSS, LPSS_REC_COMMON lpCommon)
{
  LPSS_REC_ITEMDATA64 lpRec = (LPSS_REC_ITEMDATA64)lpCommon;
  BOOL bRet = sizeof(*lpRec) == lpRec->lRecLen;
  if( bRet )
  {
    if( SS_ALLCOLS == lpRec->lCol  && SS_ALLROWS == lpRec->lRow )
      SS_SetUserData(lpSS, lpRec->lItemData);
    #ifndef SS_NOUSERDATA
    else if( SS_ALLCOLS == lpRec->lCol )
      SS_SetRowUserData(lpSS, lpRec->lRow, lpRec->lItemData);
    else if( SS_ALLROWS == lpRec->lRow )
      SS_SetColUserData(lpSS, lpRec->lCol, lpRec->lItemData);
    #endif
  }
  return bRet;
}

//--------------------------------------------------------------------

BOOL SS_Save4ItemData64(SS_COORD lCol, SS_COORD lRow, LONG_PTR lItemData,
                      LPSS_BUFF lpBuff)
{
  SS_REC_ITEMDATA64 rec;
  BOOL bRet = TRUE;
  if( lItemData )
  {
    rec.nRecType = SS_RID_ITEMDATA;
    rec.lRecLen = sizeof(rec);
    rec.lCol = lCol;
    rec.lRow = lRow;
    rec.lItemData = lItemData;
    bRet = SS_BuffAppend(lpBuff, &rec, sizeof(rec));
  }
  return bRet;
}
#endif // defined(_WIN64) || defined(_IA64)

//--------------------------------------------------------------------
//
//  The SS_Load4Iteration() function loads a SS_REC_ITERATION record.
//

BOOL SS_Load4Iteration(LPSS_BOOK lpBook, LPSS_REC_COMMON lpCommon)
{
  #if defined(SS_NOCALC) || defined(SS_OLDCALC)
  return TRUE;
  #else
  LPSS_REC_ITERATION lpRec = (LPSS_REC_ITERATION)lpCommon;
  BOOL bRet = sizeof(*lpRec) == lpRec->lRecLen;
  if( bRet )
  {
    lpBook->CalcInfo.bIteration = lpRec->bIteration;
    lpBook->CalcInfo.nMaxIterations = lpRec->nMaxIterations;
    lpBook->CalcInfo.dfMaxChange = lpRec->dfMaxChange;
  }
  return bRet;
  #endif
}

//--------------------------------------------------------------------
//
//  The SS_Save4Iteration() function saves a SS_REC_ITERATION record.
//

BOOL SS_Save4Iteration(LPSS_BOOK lpBook, LPSS_BUFF lpBuff)
{
  #if defined(SS_NOCALC) || defined(SS_OLDCALC)
  return TRUE;
  #else
  SS_REC_ITERATION rec;
  BOOL bRet = TRUE;
  if( lpBook->CalcInfo.bIteration != FALSE ||
      lpBook->CalcInfo.nMaxIterations != 1 ||
      lpBook->CalcInfo.dfMaxChange != 0.01 )
  {
    rec.nRecType = SS_RID_ITERATION;
    rec.lRecLen = sizeof(rec);
    rec.bIteration = lpBook->CalcInfo.bIteration;
    rec.nMaxIterations = lpBook->CalcInfo.nMaxIterations;
    rec.dfMaxChange = lpBook->CalcInfo.dfMaxChange;
    bRet = SS_BuffAppend(lpBuff, &rec, sizeof(rec));
  }
  return bRet;
  #endif
}

//--------------------------------------------------------------------

BOOL SS_Load4Lock(LPSPREADSHEET lpSS, LPSS_REC_COMMON lpCommon,
                  LPSS_LOADBLOCK lpLoadBlock)
{
  LPSS_REC_LOCK lpRec = (LPSS_REC_LOCK)lpCommon;
  SS_COORD lCol, lRow;
  BOOL bRet = sizeof(*lpRec) == lpRec->lRecLen;
  if( bRet &&
      BlockTranslate(lpLoadBlock, lpRec->lCol, lpRec->lRow, &lCol, &lRow) )
  {
    if( SS_LOCKED_ON == lpRec->bLock )
      SS_SetLockRange(lpSS, lCol, lRow, lCol, lRow, TRUE);
    else if( SS_LOCKED_OFF == lpRec->bLock )
      SS_SetLockRange(lpSS, lCol, lRow, lCol, lRow, FALSE);
  }
  return bRet;
}

//--------------------------------------------------------------------

BOOL SS_Save4Lock(SS_COORD lCol, SS_COORD lRow, BYTE bLock, LPSS_BUFF lpBuff)
{
  SS_REC_LOCK rec;
  BOOL bRet = TRUE;
  if( bLock )
  {
    rec.nRecType = SS_RID_LOCK;
    rec.lRecLen = sizeof(rec);
    rec.lCol = lCol;
    rec.lRow = lRow;
    rec.bLock = bLock;
    bRet = SS_BuffAppend(lpBuff, &rec, sizeof(rec));
  }
  return bRet;
}

//--------------------------------------------------------------------

BOOL SS_Load4LockColor(LPSPREADSHEET lpSS, LPSS_REC_COMMON lpCommon)
{
  LPSS_REC_LOCKCOLOR lpRec = (LPSS_REC_LOCKCOLOR)lpCommon;
  COLORREF crBackColor, crForeColor;
  BOOL bRet = sizeof(*lpRec) == lpRec->lRecLen;
  if( bRet )
  {
    crBackColor = lpRec->crBackColor;
    crForeColor = lpRec->crForeColor;
    if( (COLORREF)-1 == crBackColor )
      crBackColor = RGBCOLOR_DEFAULT;
    if( (COLORREF)-1 == crForeColor )
      crForeColor = RGBCOLOR_DEFAULT;
    SS_SetLockColor(lpSS, crBackColor, crForeColor);
  }
  return bRet;
}

//--------------------------------------------------------------------

BOOL SS_Save4LockColor(LPSPREADSHEET lpSS, LPSS_BUFF lpBuff)
{
  SS_REC_LOCKCOLOR rec;
  COLORREF crBackColor;
  COLORREF crForeColor;
  BOOL bRet = TRUE;
  SS_GetLockColor(lpSS, &crBackColor, &crForeColor);
  if( !SS_ISDEFCOLOR(crBackColor) ||
      !SS_ISDEFCOLOR(crForeColor) )
  {
    rec.nRecType = SS_RID_LOCKCOLOR;
    rec.lRecLen = sizeof(rec);
    rec.crBackColor = SS_ISDEFCOLOR(crBackColor) ? (COLORREF)-1 : crBackColor;
    rec.crForeColor = SS_ISDEFCOLOR(crForeColor) ? (COLORREF)-1 : crForeColor;
    bRet = SS_BuffAppend(lpBuff, &rec, sizeof(rec));
  }
  return bRet;
}

//--------------------------------------------------------------------

BOOL SS_Load4SelColor(LPSS_BOOK lpBook, LPSS_REC_COMMON lpCommon)
{
  LPSS_REC_SELCOLOR lpRec = (LPSS_REC_SELCOLOR)lpCommon;
  COLORREF crBackColor, crForeColor;
  BOOL bRet = sizeof(*lpRec) == lpRec->lRecLen;
  if( bRet )
  {
    crBackColor = lpRec->crBackColor;
    crForeColor = lpRec->crForeColor;
    if( (COLORREF)-1 == crBackColor )
      crBackColor = RGBCOLOR_DEFAULT;
    if( (COLORREF)-1 == crForeColor )
      crForeColor = RGBCOLOR_DEFAULT;
    SS_SetSelColor(lpBook, crBackColor, crForeColor);
  }
  return bRet;
}

//--------------------------------------------------------------------

BOOL SS_Save4SelColor(LPSS_BOOK lpBook, LPSS_BUFF lpBuff)
{
  SS_REC_SELCOLOR rec;
  COLORREF crBackColor;
  COLORREF crForeColor;
  BOOL bRet = TRUE;
  SS_GetSelColor(lpBook, &crBackColor, &crForeColor);
  if( !SS_ISDEFCOLOR(crBackColor) ||
      !SS_ISDEFCOLOR(crForeColor) )
  {
    rec.nRecType = SS_RID_SELCOLOR;
    rec.lRecLen = sizeof(rec);
    rec.crBackColor = SS_ISDEFCOLOR(crBackColor) ? (COLORREF)-1 : crBackColor;
    rec.crForeColor = SS_ISDEFCOLOR(crForeColor) ? (COLORREF)-1 : crForeColor;
    bRet = SS_BuffAppend(lpBuff, &rec, sizeof(rec));
  }
  return bRet;
}

//--------------------------------------------------------------------

BOOL SS_Load4MaxSize(LPSPREADSHEET lpSS, LPSS_REC_COMMON lpCommon)
{
  LPSS_REC_MAXSIZE lpRec = (LPSS_REC_MAXSIZE)lpCommon;
  BOOL bRet = sizeof(*lpRec) == lpRec->lRecLen;
  if( bRet )
  {
    lpSS->Col.Max = lpRec->lMaxCols; //???? will this cause any problems ????
    lpSS->Row.Max = lpRec->lMaxRows; //???? will this cause any problems ????
  }
  return bRet;
}

//--------------------------------------------------------------------

BOOL SS_Save4MaxSize(LPSPREADSHEET lpSS, LPSS_BUFF lpBuff)
{
  SS_REC_MAXSIZE rec;
  BOOL bRet = TRUE;
  if( 500 != lpSS->Col.Max || 500 != lpSS->Row.Max )
  {
    rec.nRecType = SS_RID_MAXSIZE;
    rec.lRecLen = sizeof(rec);
    rec.lMaxCols = lpSS->Col.Max;
    rec.lMaxRows = lpSS->Row.Max;
    bRet = SS_BuffAppend(lpBuff, &rec, sizeof(rec));
  }
  return bRet;
}

//--------------------------------------------------------------------

BOOL SS_Load4OpMode(LPSPREADSHEET lpSS, LPSS_REC_COMMON lpCommon)
{
  LPSS_REC_OPMODE lpRec = (LPSS_REC_OPMODE)lpCommon;
  BOOL bRet = sizeof(*lpRec) == lpRec->lRecLen;
  if( bRet )
  {
    lpSS->wOpMode = lpRec->wOpMode;
  }
  return bRet;
}

//--------------------------------------------------------------------

BOOL SS_Save4OpMode(LPSPREADSHEET lpSS, LPSS_BUFF lpBuff)
{
  SS_REC_OPMODE rec;
  BOOL bRet = TRUE;
  if( SS_OPMODE_NORMAL != lpSS->wOpMode )
  {
    rec.nRecType = SS_RID_OPMODE;
    rec.lRecLen = sizeof(rec);
    rec.wOpMode = lpSS->wOpMode;
    bRet = SS_BuffAppend(lpBuff, &rec, sizeof(rec));
  }
  return bRet;
}

//--------------------------------------------------------------------
//
//  The SS_Load4Print() function loads a SS_REC_PRINT record.
//

BOOL SS_Load4Print(LPSPREADSHEET lpSS, LPSS_REC_COMMON lpCommon, BOOL bUnicode)
{
  LPSS_REC_PRINT lpRec = (LPSS_REC_PRINT)lpCommon;
  SS_BUFF buffAbortMsg;
  SS_BUFF buffFooter;
  SS_BUFF buffHeader;
  SS_BUFF buffJobName;
  HPBYTE lpAbortMsg;
  HPBYTE lpFooter;
  HPBYTE lpHeader;
  HPBYTE lpJobName;
  LPTSTR lpszAbortMsg = NULL;
  LPTSTR lpszFooter = NULL;
  LPTSTR lpszHeader = NULL;
  LPTSTR lpszJobName = NULL;
  long lAbortMsgLen = 0;
  long lFooterLen = 0;
  long lHeaderLen = 0;
  long lJobNameLen = 0;
  BOOL bRet = (long)sizeof(*lpRec) + lpRec->lAbortMsgLen + lpRec->lFooterLen +
              lpRec->lHeaderLen + lpRec->lJobNameLen == lpRec->lRecLen;
  if( bRet )
  {
    SS_BuffInit(&buffAbortMsg);
    SS_BuffInit(&buffFooter);
    SS_BuffInit(&buffHeader);
    SS_BuffInit(&buffJobName);
    lpAbortMsg = (HPBYTE)lpRec + sizeof(*lpRec);
    lpFooter = lpAbortMsg + lpRec->lAbortMsgLen;
    lpHeader = lpFooter + lpRec->lFooterLen;
    lpJobName = lpHeader + lpRec->lHeaderLen;
    if( lpRec->lAbortMsgLen )
      lpszAbortMsg = SS_BuffCopyStr(&buffAbortMsg, lpAbortMsg, bUnicode);
    if( lpRec->lFooterLen )
      lpszFooter = SS_BuffCopyStr(&buffFooter, lpFooter, bUnicode);
    if( lpRec->lHeaderLen )
      lpszHeader = SS_BuffCopyStr(&buffHeader, lpHeader, bUnicode);
    if( lpRec->lJobNameLen )
      lpszJobName = SS_BuffCopyStr(&buffJobName, lpJobName, bUnicode);

    lpSS->PrintOptions.pf.fDrawBorder = lpRec->bDrawBorder;
    lpSS->PrintOptions.pf.fDrawColors = lpRec->bDrawColor;
    lpSS->PrintOptions.pf.fDrawShadows = lpRec->bDrawShadows;
    lpSS->PrintOptions.pf.fShowGrid = lpRec->bShowGrid;
    lpSS->PrintOptions.pf.fShowColHeaders = lpRec->bShowColHeaders;
    lpSS->PrintOptions.pf.fShowRowHeaders = lpRec->bShowRowHeaders;
    lpSS->PrintOptions.pf.fUseDataMax = lpRec->bUseDataMax;
    lpSS->PrintOptions.pf.dPrintType = lpRec->nPrintType;

    lpSS->PrintOptions.pf.nPageEnd = lpRec->nPageEnd;
    lpSS->PrintOptions.pf.nPageStart = lpRec->nPageStart;
    lpSS->PrintOptions.pf.fMarginLeft = (float)lpRec->lMarginLeft / (float)1440.0;
    lpSS->PrintOptions.pf.fMarginTop = (float)lpRec->lMarginTop / (float)1440.0;
    lpSS->PrintOptions.pf.fMarginBottom = (float)lpRec->lMarginBottom / (float)1440.0;
    lpSS->PrintOptions.pf.fMarginRight = (float)lpRec->lMarginRight / (float)1440.0;
    lpSS->PrintOptions.pf.wOrientation = lpRec->nOrientation;

    SS_SetPrintFooter(lpSS, lpszFooter);
    SS_SetPrintHeader(lpSS, lpszHeader);
    SS_SetPrintAbortMsg(lpSS->lpBook, lpszAbortMsg);
    SS_SetPrintJobName(lpSS->lpBook, lpszJobName);

    SS_BuffFree(&buffAbortMsg);
    SS_BuffFree(&buffFooter);
    SS_BuffFree(&buffHeader);
    SS_BuffFree(&buffJobName);
  }
  return bRet;
}

/*
BOOL SS_Load4Print(LPSS_BOOK lpBook, LPSS_BUFF lpXtra,
                   LPSS_REC_COMMON lpCommon, BOOL bUnicode)
{
  LPSS_REC_PRINT lpRec = (LPSS_REC_PRINT)lpCommon;
  SS_REC_PRINT recTemp;
  SS_BUFF buffTemp;
  SS_BUFF buffAbortMsg;
  SS_BUFF buffFooter;
  SS_BUFF buffHeader;
  SS_BUFF buffJobName;
  HPBYTE lpAbortMsg;
  HPBYTE lpFooter;
  HPBYTE lpHeader;
  HPBYTE lpJobName;
  LPTSTR lpszAbortMsg = NULL;
  LPTSTR lpszFooter = NULL;
  LPTSTR lpszHeader = NULL;
  LPTSTR lpszJobName = NULL;
  long lAbortMsgLen = 0;
  long lFooterLen = 0;
  long lHeaderLen = 0;
  long lJobNameLen = 0;
  BOOL bRet = (long)sizeof(*lpRec) + lpRec->lAbortMsgLen + lpRec->lFooterLen +
              lpRec->lHeaderLen + lpRec->lJobNameLen == lpRec->lRecLen;
  if( bRet )
  {
    SS_BuffInit(&buffTemp);
    SS_BuffInit(&buffAbortMsg);
    SS_BuffInit(&buffFooter);
    SS_BuffInit(&buffHeader);
    SS_BuffInit(&buffJobName);
    lpAbortMsg = (HPBYTE)lpRec + sizeof(*lpRec);
    lpFooter = lpAbortMsg + lpRec->lAbortMsgLen;
    lpHeader = lpFooter + lpRec->lFooterLen;
    lpJobName = lpHeader + lpRec->lHeaderLen;
    if( lpRec->lAbortMsgLen )
    {
      lpszAbortMsg = SS_BuffCopyStr(&buffAbortMsg, lpAbortMsg, bUnicode);
      lAbortMsgLen = (lstrlen(lpszAbortMsg) + 1) * sizeof(TCHAR);
    }
    if( lpRec->lFooterLen )
    {
      lpszFooter = SS_BuffCopyStr(&buffFooter, lpFooter, bUnicode);
      lFooterLen = (lstrlen(lpszFooter) + 1) * sizeof(TCHAR);
    }
    if( lpRec->lHeaderLen )
    {
      lpszHeader = SS_BuffCopyStr(&buffHeader, lpHeader, bUnicode);
      lHeaderLen = (lstrlen(lpszHeader) + 1) * sizeof(TCHAR);
    }
    if( lpRec->lJobNameLen )
    {
      lpszJobName = SS_BuffCopyStr(&buffJobName, lpJobName, bUnicode);
      lJobNameLen = (lstrlen(lpszJobName) + 1) * sizeof(TCHAR);
    }
    recTemp = *lpRec;
    recTemp.lRecLen = sizeof(recTemp) + lAbortMsgLen + lFooterLen +
                      lHeaderLen + lJobNameLen;
    recTemp.lAbortMsgLen = lAbortMsgLen;
    recTemp.lFooterLen = lFooterLen;
    recTemp.lHeaderLen = lHeaderLen;
    recTemp.lJobNameLen = lJobNameLen;
    bRet &= SS_BuffAppend(&buffTemp, &recTemp, sizeof(recTemp));
    if( lAbortMsgLen ) 
      bRet &= SS_BuffAppend(&buffTemp, lpszAbortMsg, lAbortMsgLen);
    if( lFooterLen )
      bRet &= SS_BuffAppend(&buffTemp, lpszFooter, lFooterLen);
    if( lHeaderLen )
      bRet &= SS_BuffAppend(&buffTemp, lpszHeader, lHeaderLen);
    if( lJobNameLen )
      bRet &= SS_BuffAppend(&buffTemp, lpszJobName, lJobNameLen);
    #if defined(SS_OCX)
    bRet &= SS_OcxLoad4Print(lpBook, (LPSS_REC_PRINT)buffTemp.pMem);
    #elif defined(SS_VB)
    bRet &= SS_VbxLoad4Print(lpBook, (LPSS_REC_PRINT)buffTemp.pMem);
    #else
    bRet &= SS_BuffAppend(lpXtra, buffTemp.pMem, buffTemp.lLen);
    #endif
    SS_BuffFree(&buffTemp);
    SS_BuffFree(&buffAbortMsg);
    SS_BuffFree(&buffFooter);
    SS_BuffFree(&buffHeader);
    SS_BuffFree(&buffJobName);
  }
  return bRet;
}
*/

//--------------------------------------------------------------------

/*
BOOL SS_Save4Print(LPSS_BOOK lpBook, LPSS_BUFF lpBuff)
{
  #if defined(SS_OCX)
  return SS_OcxSave4Print(lpBook, lpBuff);
  #elif defined(SS_VB)
  return SS_VbxSave4Print(lpBook, lpBuff);
  #else
  return TRUE;
  #endif
}
*/

#ifdef SS_V30

//--------------------------------------------------------------------
//
//  The SS_Load4Print2() function loads a SS_REC_PRINT2 record.
//

BOOL SS_Load4Print2(LPSPREADSHEET lpSS, LPSS_REC_COMMON lpCommon)
{
  LPSS_REC_PRINT2 lpRec = (LPSS_REC_PRINT2)lpCommon;
  BOOL bRet = sizeof(*lpRec) == lpRec->lRecLen;
  if( bRet )
  {
    lpSS->PrintOptions.pf.fSmartPrint = lpRec->bSmartPrint;
    lpSS->PrintOptions.pf.lFirstPageNumber = lpRec->lFirstPageNumber;

    switch (lpRec->nPageOrder)
    {
      case 1: lpSS->PrintOptions.pf.dPrintType |= SS_PRINTTYPE_DOWNTHENOVER; break;
      case 2: lpSS->PrintOptions.pf.dPrintType |= SS_PRINTTYPE_OVERTHENDOWN; break;
    }
  }
  return bRet;
}

/*
BOOL SS_Load4Print2(LPSS_BOOK lpBook, LPSS_BUFF lpXtra,
                   LPSS_REC_COMMON lpCommon, BOOL bUnicode)
{
  LPSS_REC_PRINT2 lpRec = (LPSS_REC_PRINT2)lpCommon;
  BOOL bRet = sizeof(*lpRec) == lpRec->lRecLen;
  if( bRet )
  {
    #if defined(SS_OCX)
    bRet &= SS_OcxLoad4Print2(lpBook, lpRec);
    #elif defined(SS_VB)
    bRet &= SS_VbxLoad4Print2(lpBook, lpRec);
    #endif
  }
  return bRet;
}
*/
//--------------------------------------------------------------------

/*
BOOL SS_Save4Print2(LPSS_BOOK lpBook, LPSS_BUFF lpBuff)
{
  #if defined(SS_OCX)
  return SS_OcxSave4Print2(lpBook, lpBuff);
  #elif defined(SS_VB)
  return SS_VbxSave4Print2(lpBook, lpBuff);
  #else
  return TRUE;
  #endif
}
*/
#endif // SS_V30

//--------------------------------------------------------------------
//
//  The SS_Load4PrintBook() function loads the sheet print settings.
//

BOOL SS_Load4PrintBook(LPSS_BOOK lpBook, BOOL bUnicode, LPSS_REC_COMMON lpCommon)
{
  LPSS_REC_PRINTBOOK lpRec = (LPSS_REC_PRINTBOOK)lpCommon;
  SS_BUFF buffJobName;
  SS_BUFF buffAbortMsg;
  HPBYTE lpJobName;
  HPBYTE lpAbortMsg;
  LPTSTR lpszJobName = NULL;
  LPTSTR lpszAbortMsg = NULL;
  BOOL bRet = sizeof(*lpRec) <= lpRec->lRecLen;
  if( bRet )
  {
    SS_BuffInit(&buffJobName);
    SS_BuffInit(&buffAbortMsg);
    lpJobName = (HPBYTE)lpRec + sizeof(*lpRec);
    lpAbortMsg = lpJobName + lpRec->lJobNameLen;
    if (lpRec->lJobNameLen)
      lpszJobName = SS_BuffCopyStr(&buffJobName, lpJobName, bUnicode);
    if (lpRec->lAbortMsgLen)
      lpszAbortMsg = SS_BuffCopyStr(&buffAbortMsg, lpAbortMsg, bUnicode);

    SS_SetPrintJobName(lpBook, lpszJobName);
    SS_SetPrintAbortMsg(lpBook, lpszAbortMsg);

    SS_BuffFree(&buffJobName);
    SS_BuffFree(&buffAbortMsg);
  }

  return bRet;
}

//--------------------------------------------------------------------
//
//  The SS_Save4PrintBook() function stores the sheet print settings.
//

BOOL SS_Save4PrintBook(LPSS_BOOK lpBook, LPSS_BUFF lpBuff)
{
  SS_REC_PRINTBOOK rec;
  BOOL bRet = TRUE;

  if (lpBook->hPrintAbortMsg ||
	   lpBook->hPrintJobName)
  {
    LPTSTR lpszJobName = NULL;
    LPTSTR lpszAbortMsg = NULL;

    _fmemset(&rec, '\0', sizeof(rec));
    rec.nRecType = SS_RID_PRINTBOOK;
    rec.lRecLen = sizeof(rec);

    if (lpBook->hPrintJobName)
      lpszJobName = (LPTSTR)tbGlobalLock(lpBook->hPrintJobName);

    if (lpBook->hPrintAbortMsg)
      lpszAbortMsg = (LPTSTR)tbGlobalLock(lpBook->hPrintAbortMsg);

    if (lpszJobName)
      rec.lJobNameLen = (lstrlen(lpszJobName) + 1) * sizeof(TCHAR);
    if (lpszAbortMsg)
      rec.lAbortMsgLen = (lstrlen(lpszAbortMsg) + 1) * sizeof(TCHAR);

    rec.lRecLen += rec.lJobNameLen;
    rec.lRecLen += rec.lAbortMsgLen;

    bRet = SS_BuffAppend(lpBuff, &rec, sizeof(rec));
    if (lpszJobName)
      bRet &= SS_BuffAppend(lpBuff, (LPVOID)(LPCTSTR)lpszJobName, rec.lJobNameLen);
    if (lpszAbortMsg)
      bRet &= SS_BuffAppend(lpBuff, (LPVOID)(LPCTSTR)lpszAbortMsg, rec.lAbortMsgLen);

    if (lpBook->hPrintJobName)
      tbGlobalUnlock(lpBook->hPrintJobName);

    if (lpBook->hPrintAbortMsg)
      tbGlobalUnlock(lpBook->hPrintAbortMsg);
  }

  return bRet;
}

//--------------------------------------------------------------------
//
//  The SS_Load4PrintSheet() function loads the sheet print settings.
//

BOOL SS_Load4PrintSheet(LPSPREADSHEET lpSS, BOOL bUnicode, LPSS_REC_COMMON lpCommon)
{
  LPSS_REC_PRINTSHEET lpRec = (LPSS_REC_PRINTSHEET)lpCommon;
  SS_BUFF buffFooter;
  SS_BUFF buffHeader;
  HPBYTE lpFooter;
  HPBYTE lpHeader;
  LPTSTR lpszFooter = NULL;
  LPTSTR lpszHeader = NULL;
  BOOL bRet = sizeof(*lpRec) <= lpRec->lRecLen;
  if( bRet )
  {
    SS_BuffInit(&buffFooter);
    SS_BuffInit(&buffHeader);
    lpFooter = (HPBYTE)lpRec + sizeof(*lpRec);
    lpHeader = lpFooter + lpRec->lFooterLen;
    if (lpRec->lFooterLen)
      lpszFooter = SS_BuffCopyStr(&buffFooter, lpFooter, bUnicode);
    if (lpRec->lHeaderLen)
      lpszHeader = SS_BuffCopyStr(&buffHeader, lpHeader, bUnicode);

    lpSS->PrintOptions.pf.fDrawBorder = lpRec->bDrawBorder;
    lpSS->PrintOptions.pf.fDrawColors = lpRec->bDrawColor;
    lpSS->PrintOptions.pf.fDrawShadows = lpRec->bDrawShadows;
    lpSS->PrintOptions.pf.fShowGrid = lpRec->bShowGrid;
    lpSS->PrintOptions.pf.fShowColHeaders = lpRec->bShowColHeaders;
    lpSS->PrintOptions.pf.fShowRowHeaders = lpRec->bShowRowHeaders;
    lpSS->PrintOptions.pf.fUseDataMax = lpRec->bUseDataMax;
    lpSS->PrintOptions.pf.dPrintType = lpRec->nPrintType;
    lpSS->PrintOptions.pf.nPageEnd = lpRec->nPageEnd;
    lpSS->PrintOptions.pf.nPageStart = lpRec->nPageStart;
    lpSS->PrintOptions.pf.fMarginLeft = lpRec->fMarginLeft;
    lpSS->PrintOptions.pf.fMarginTop = lpRec->fMarginTop;
    lpSS->PrintOptions.pf.fMarginBottom = lpRec->fMarginBottom;
    lpSS->PrintOptions.pf.fMarginRight = lpRec->fMarginRight;
    lpSS->PrintOptions.pf.wOrientation = lpRec->nOrientation;
    lpSS->PrintOptions.pf.fSmartPrint = lpRec->bSmartPrint;
    lpSS->PrintOptions.pf.lFirstPageNumber = lpRec->lFirstPageNumber;
    lpSS->PrintOptions.pf.x1CellRange = lpRec->x1CellRange;
    lpSS->PrintOptions.pf.y1CellRange = lpRec->y1CellRange;
    lpSS->PrintOptions.pf.x2CellRange = lpRec->x2CellRange;
    lpSS->PrintOptions.pf.y2CellRange = lpRec->y2CellRange;
#ifdef SS_V70
    lpSS->PrintOptions.pf.wScalingMethod = lpRec->wScalingMethod;
    lpSS->PrintOptions.pf.ZoomFactor = lpRec->ZoomFactor;
    lpSS->PrintOptions.pf.nBestFitPagesWide = lpRec->nBestFitPagesWide;
    lpSS->PrintOptions.pf.nBestFitPagesTall = lpRec->nBestFitPagesTall;
    lpSS->PrintOptions.pf.fCenterOnPageH = lpRec->fCenterOnPageH;
    lpSS->PrintOptions.pf.fCenterOnPageV = lpRec->fCenterOnPageV;
#endif
    SS_SetPrintFooter(lpSS, lpszFooter);
    SS_SetPrintHeader(lpSS, lpszHeader);

    SS_BuffFree(&buffFooter);
    SS_BuffFree(&buffHeader);
  }

  return TRUE; // TODO7 Remove this
  return bRet;
}

//--------------------------------------------------------------------
//
//  The SS_Save4PrintSheet() function stores the sheet print settings.
//

BOOL SS_Save4PrintSheet(LPSPREADSHEET lpSS, LPSS_BUFF lpBuff)
{
  SS_REC_PRINTSHEET rec;
  BOOL bRet = TRUE;

  if (lpSS->PrintOptions.pf.fDrawBorder != TRUE ||
      lpSS->PrintOptions.pf.fDrawShadows != TRUE ||
      lpSS->PrintOptions.pf.fDrawColors != FALSE ||
      lpSS->PrintOptions.pf.fShowGrid != TRUE ||
      lpSS->PrintOptions.pf.fShowColHeaders != TRUE ||
      lpSS->PrintOptions.pf.fShowRowHeaders != TRUE ||
      lpSS->PrintOptions.pf.fUseDataMax != TRUE ||
      lpSS->PrintOptions.pf.dPrintType != 0 ||
      lpSS->PrintOptions.pf.x1CellRange != 0 ||
      lpSS->PrintOptions.pf.y1CellRange != 0 ||
      lpSS->PrintOptions.pf.x2CellRange != 0 ||
      lpSS->PrintOptions.pf.y2CellRange != 0 ||
      lpSS->PrintOptions.pf.nPageStart != 1 ||
      lpSS->PrintOptions.pf.nPageEnd != 1 ||
      lpSS->PrintOptions.pf.fMarginLeft != 0 ||
      lpSS->PrintOptions.pf.fMarginTop != 0 ||
      lpSS->PrintOptions.pf.fMarginRight != 0 ||
      lpSS->PrintOptions.pf.fMarginBottom != 0 ||
      lpSS->PrintOptions.pf.wOrientation != 0 ||
      lpSS->PrintOptions.pf.fSmartPrint != FALSE ||
	   lpSS->PrintOptions.pf.lFirstPageNumber != 1 ||
#ifdef SS_V70
      lpSS->PrintOptions.pf.ZoomFactor != 1.0 ||
      lpSS->PrintOptions.pf.wScalingMethod != SS_PRINTSCALINGMETHOD_NONE ||
      lpSS->PrintOptions.pf.nBestFitPagesWide != 1 ||
      lpSS->PrintOptions.pf.nBestFitPagesTall != 1 ||
      lpSS->PrintOptions.pf.fCenterOnPageH != FALSE ||
      lpSS->PrintOptions.pf.fCenterOnPageV != FALSE ||
#endif // SS_V70
	   lpSS->PrintOptions.pf.hFooter ||
	   lpSS->PrintOptions.pf.hHeader)

  {
    LPTSTR lpszFooter = NULL;
    LPTSTR lpszHeader = NULL;

    _fmemset(&rec, '\0', sizeof(rec));
    rec.nRecType = SS_RID_PRINTSHEET;
    rec.lRecLen = sizeof(rec);

    if (lpSS->PrintOptions.pf.hFooter)
      lpszFooter = (LPTSTR)GlobalLock(lpSS->PrintOptions.pf.hFooter);

    if (lpSS->PrintOptions.pf.hHeader)
      lpszHeader = (LPTSTR)GlobalLock(lpSS->PrintOptions.pf.hHeader);

    rec.bDrawBorder = lpSS->PrintOptions.pf.fDrawBorder;
    rec.bDrawColor = lpSS->PrintOptions.pf.fDrawColors;
    rec.bDrawShadows = lpSS->PrintOptions.pf.fDrawShadows;
    rec.bShowGrid = lpSS->PrintOptions.pf.fShowGrid;
    rec.bShowColHeaders = lpSS->PrintOptions.pf.fShowColHeaders;
    rec.bShowRowHeaders = lpSS->PrintOptions.pf.fShowRowHeaders;
    rec.bUseDataMax = lpSS->PrintOptions.pf.fUseDataMax;
    rec.nPrintType = lpSS->PrintOptions.pf.dPrintType;

    rec.nPageEnd = lpSS->PrintOptions.pf.nPageEnd;
    rec.nPageStart = lpSS->PrintOptions.pf.nPageStart;
    rec.fMarginLeft = lpSS->PrintOptions.pf.fMarginLeft;
    rec.fMarginTop = lpSS->PrintOptions.pf.fMarginTop;
    rec.fMarginBottom = lpSS->PrintOptions.pf.fMarginBottom;
    rec.fMarginRight = lpSS->PrintOptions.pf.fMarginRight;
    rec.nOrientation = lpSS->PrintOptions.pf.wOrientation;
    rec.bSmartPrint = lpSS->PrintOptions.pf.fSmartPrint;
    rec.lFirstPageNumber = lpSS->PrintOptions.pf.lFirstPageNumber;
 
#ifdef SS_V70
    rec.ZoomFactor = lpSS->PrintOptions.pf.ZoomFactor;
    rec.wScalingMethod = lpSS->PrintOptions.pf.wScalingMethod;
    rec.nBestFitPagesWide = lpSS->PrintOptions.pf.nBestFitPagesWide;
    rec.nBestFitPagesTall = lpSS->PrintOptions.pf.nBestFitPagesTall;
    rec.fCenterOnPageH = lpSS->PrintOptions.pf.fCenterOnPageH;
    rec.fCenterOnPageV = lpSS->PrintOptions.pf.fCenterOnPageV;
#else
    rec.ZoomFactor = 1.0;
    rec.wScalingMethod = 0;
    rec.nBestFitPagesWide = 1;
    rec.nBestFitPagesTall = 1;
    rec.fCenterOnPageH = FALSE;
    rec.fCenterOnPageV = FALSE;
#endif // SS_V70

    if (lpszFooter)
      rec.lFooterLen = (lstrlen(lpszFooter) + 1) * sizeof(TCHAR);
    if (lpszHeader)
      rec.lHeaderLen = (lstrlen(lpszHeader) + 1) * sizeof(TCHAR);
    rec.lRecLen += rec.lFooterLen;
    rec.lRecLen += rec.lHeaderLen;

    bRet = SS_BuffAppend(lpBuff, &rec, sizeof(rec));
    if (lpszFooter)
      bRet &= SS_BuffAppend(lpBuff, (LPVOID)(LPCTSTR)lpszFooter, rec.lFooterLen);
    if (lpszHeader)
      bRet &= SS_BuffAppend(lpBuff, (LPVOID)(LPCTSTR)lpszHeader, rec.lHeaderLen);

    if (lpSS->PrintOptions.pf.hFooter)
      GlobalUnlock(lpSS->PrintOptions.pf.hFooter);

    if (lpSS->PrintOptions.pf.hHeader)
      GlobalUnlock(lpSS->PrintOptions.pf.hHeader);
  }

  return bRet;
}

//--------------------------------------------------------------------

BOOL SS_Load4Protect(LPSS_BOOK lpBook, LPSS_REC_COMMON lpCommon)
{
  LPSS_REC_PROTECT lpRec = (LPSS_REC_PROTECT)lpCommon;
  BOOL bRet = sizeof(*lpRec) == lpRec->lRecLen;
  if( bRet )
  {
    lpBook->DocumentProtected = lpRec->bProtect;
  }
  return bRet;
}

//--------------------------------------------------------------------

BOOL SS_Save4Protect(LPSS_BOOK lpBook, LPSS_BUFF lpBuff)
{
  SS_REC_PROTECT rec;
  BOOL bRet = TRUE;
  if( !lpBook->DocumentProtected )
  {
    rec.nRecType = SS_RID_PROTECT;
    rec.lRecLen = sizeof(rec);
    rec.bProtect = lpBook->DocumentProtected;
    bRet = SS_BuffAppend(lpBuff, &rec, sizeof(rec));
  }
  return bRet;
}

//--------------------------------------------------------------------
//
//  The SS_Load4RefStyle() function loads a SS_REC_REFSTYLE record.
//

BOOL SS_Load4RefStyle(LPSS_BOOK lpBook, LPSS_REC_COMMON lpCommon)
{
  #if defined(SS_NOCALC) || defined(SS_OLDCALC)
  return TRUE;
  #else
  LPSS_REC_REFSTYLE lpRec = (LPSS_REC_REFSTYLE)lpCommon;
  BOOL bRet = sizeof(*lpRec) == lpRec->lRecLen;
  if( bRet )
  {
    lpBook->CalcInfo.nRefStyle = lpRec->nRefStyle;
  }
  return bRet;
  #endif
}

//--------------------------------------------------------------------
//
//  The SS_Save4RefStyle() function saves a SS_REC_REFSTYLE record.
//

BOOL SS_Save4RefStyle(LPSS_BOOK lpBook, LPSS_BUFF lpBuff)
{
  #if defined(SS_NOCALC) || defined(SS_OLDCALC)
  return TRUE;
  #else
  SS_REC_REFSTYLE rec;
  BOOL bRet = TRUE;
  if( lpBook->CalcInfo.nRefStyle != SS_REFSTYLE_DEFAULT )
  {
    rec.nRecType = SS_RID_REFSTYLE;
    rec.lRecLen = sizeof(rec);
    rec.nRefStyle = lpBook->CalcInfo.nRefStyle;
    bRet = SS_BuffAppend(lpBuff, &rec, sizeof(rec));
  }
  return bRet;
  #endif
}

//--------------------------------------------------------------------

BOOL SS_Load4Restrict(LPSPREADSHEET lpSS, LPSS_REC_COMMON lpCommon)
{
  LPSS_REC_RESTRICT lpRec = (LPSS_REC_RESTRICT)lpCommon;
  BOOL bRet = sizeof(*lpRec) == lpRec->lRecLen;
  if( bRet )
  {
    lpSS->RestrictCols = lpRec->bRestrictCols;
    lpSS->RestrictRows = lpRec->bRestrictRows;
  }
  return bRet;
}

//--------------------------------------------------------------------

BOOL SS_Save4Restrict(LPSPREADSHEET lpSS, LPSS_BUFF lpBuff)
{
  SS_REC_RESTRICT rec;
  BOOL bRet = TRUE;
  if( lpSS->RestrictCols || lpSS->RestrictRows )
  {
    rec.nRecType = SS_RID_RESTRICT;
    rec.lRecLen = sizeof(rec);
    rec.bRestrictCols = (BYTE)lpSS->RestrictCols;
    rec.bRestrictRows = (BYTE)lpSS->RestrictRows;
    bRet = SS_BuffAppend(lpBuff, &rec, sizeof(rec));
  }
  return bRet;
}

//--------------------------------------------------------------------

BOOL SS_Load4RowHeight(LPSPREADSHEET lpSS, LPSS_REC_COMMON lpCommon, LPSS_LOADBLOCK lpLoadBlock)
{
  LPSS_REC_ROWHEIGHT lpRec = (LPSS_REC_ROWHEIGHT)lpCommon;
  SS_COORD lRow;
  BOOL bRet = sizeof(*lpRec) == lpRec->lRecLen;
  if( bRet &&
      BlockTranslate(lpLoadBlock, 0, lpRec->lRow, NULL, &lRow) )
  {
    SS_SetRowHeight(lpSS, lRow, lpRec->dfRowHeight);
  }
  return bRet;
}

//--------------------------------------------------------------------

BOOL SS_Save4RowHeight(SS_COORD lRow, long lRowHeightX100, LPSS_BUFF lpBuff)
{
  SS_REC_ROWHEIGHT rec;
  BOOL bRet = TRUE;
  if( -1 != lRowHeightX100 )
  {
    rec.nRecType = SS_RID_ROWHEIGHT;
    rec.lRecLen = sizeof(rec);
    rec.lRow = lRow;
    rec.dfRowHeight = (double)lRowHeightX100 / 100.0;
    bRet = SS_BuffAppend(lpBuff, &rec, sizeof(rec));
  }
  return bRet;
}

//--------------------------------------------------------------------

BOOL SS_Load4RowPageBreak(LPSPREADSHEET lpSS, LPSS_REC_COMMON lpCommon)
{
  LPSS_REC_ROWPAGEBREAK lpRec = (LPSS_REC_ROWPAGEBREAK)lpCommon;
  BOOL bRet = sizeof(*lpRec) == lpRec->lRecLen;
  if( bRet )
  {
    #ifndef SS_NOPRINT
    SS_SetRowPageBreak(lpSS, lpRec->lRow, lpRec->bPageBreak);
    #endif
  }
  return bRet;
}

//--------------------------------------------------------------------

BOOL SS_Save4RowPageBreak(SS_COORD lRow, BOOL bPageBreak, LPSS_BUFF lpBuff)
{
  SS_REC_ROWPAGEBREAK rec;
  BOOL bRet = TRUE;
  if( bPageBreak )
  {
    rec.nRecType = SS_RID_ROWPAGEBREAK;
    rec.lRecLen = sizeof(rec);
    rec.lRow = lRow;
    rec.bPageBreak = bPageBreak;
    bRet = SS_BuffAppend(lpBuff, &rec, sizeof(rec));
  }
  return bRet;
}

//--------------------------------------------------------------------

BOOL SS_Load4ScrollBar(LPSS_BOOK lpBook, LPSS_REC_COMMON lpCommon)
{
  LPSS_REC_SCROLLBAR lpRec = (LPSS_REC_SCROLLBAR)lpCommon;
  BOOL bRet = sizeof(*lpRec) == lpRec->lRecLen;
  if( bRet )
  {
    lpBook->HorzScrollBar = lpRec->bHorzScrollBar;
    lpBook->VertScrollBar = lpRec->bVertScrollBar;
    lpBook->fScrollBarExtMode = lpRec->bScrollBarExtMode;
    lpBook->fScrollBarMaxAlign = lpRec->bScrollBarMaxAlign;
    lpBook->fScrollBarShowMax = lpRec->bScrollBarShowMax;
  }
  return bRet;
}

//--------------------------------------------------------------------

BOOL SS_Save4ScrollBar(LPSS_BOOK lpBook, LPSS_BUFF lpBuff)
{
  SS_REC_SCROLLBAR rec;
  BOOL bRet = TRUE;
  if( !lpBook->HorzScrollBar || !lpBook->VertScrollBar ||
      lpBook->fScrollBarExtMode | !lpBook->fScrollBarMaxAlign ||
      !lpBook->fScrollBarShowMax )
  {
    rec.nRecType = SS_RID_SCROLLBAR;
    rec.lRecLen = sizeof(rec);
    rec.bHorzScrollBar = (BYTE)lpBook->HorzScrollBar;
    rec.bVertScrollBar = (BYTE)lpBook->VertScrollBar;
    rec.bScrollBarExtMode = (BYTE)lpBook->fScrollBarExtMode;
    rec.bScrollBarMaxAlign = (BYTE)lpBook->fScrollBarMaxAlign;
    rec.bScrollBarShowMax = (BYTE)lpBook->fScrollBarShowMax;
    bRet = SS_BuffAppend(lpBuff, &rec, sizeof(rec));
  }
  return bRet;
}

//--------------------------------------------------------------------

BOOL SS_Load4SelBlockOption(LPSS_BOOK lpBook, LPSS_REC_COMMON lpCommon)
{
  LPSS_REC_SELBLOCKOPTION lpRec = (LPSS_REC_SELBLOCKOPTION)lpCommon;
  BOOL bRet = sizeof(*lpRec) == lpRec->lRecLen;
  if( bRet )
  {
    lpBook->wSelBlockOption = lpRec->wOption;
  }
  return bRet;
}

//--------------------------------------------------------------------

BOOL SS_Save4SelBlockOption(LPSS_BOOK lpBook, LPSS_BUFF lpBuff)
{
  SS_REC_SELBLOCKOPTION rec;
  BOOL bRet = TRUE;
  if( (SS_SELBLOCK_COLS | SS_SELBLOCK_ROWS | SS_SELBLOCK_BLOCKS |
       SS_SELBLOCK_ALL) != lpBook->wSelBlockOption )
  {
    rec.nRecType = SS_RID_SELBLOCKOPTION;
    rec.lRecLen = sizeof(rec);
    rec.wOption = lpBook->wSelBlockOption;
    bRet = SS_BuffAppend(lpBuff, &rec, sizeof(rec));
  }
  return bRet;
}

//--------------------------------------------------------------------

BOOL SS_Load4ShadowColor(LPSPREADSHEET lpSS, LPSS_REC_COMMON lpCommon)
{
  LPSS_REC_SHADOWCOLOR lpRec = (LPSS_REC_SHADOWCOLOR)lpCommon;
  BOOL bRet = sizeof(*lpRec) == lpRec->lRecLen;
  if( bRet )
  {
    SS_SetShadowColor(lpSS, lpRec->crShadowColor, lpRec->crShadowText,
                      lpRec->crShadowDark, lpRec->crShadowLight);
  }
  return bRet;
}

//--------------------------------------------------------------------

BOOL SS_Save4ShadowColor(LPSPREADSHEET lpSS, LPSS_BUFF lpBuff)
{
  SS_REC_SHADOWCOLOR rec;
  COLORREF crShadowColor;
  COLORREF crShadowText;
  COLORREF crShadowDark;
  COLORREF crShadowLight;
  BOOL bRet = TRUE;
  SS_GetShadowColor(lpSS, &crShadowColor, &crShadowText,
                    &crShadowDark, &crShadowLight);
  if( COLOR_BTNFACE != crShadowColor ||
      COLOR_BTNTEXT != crShadowText ||
      COLOR_BTNSHADOW != crShadowDark ||
      COLOR_BTNHIGHLIGHT != crShadowLight )
  {
    rec.nRecType = SS_RID_SHADOWCOLOR;
    rec.lRecLen = sizeof(rec);
    rec.crShadowColor = crShadowColor;
    rec.crShadowText = crShadowText;
    rec.crShadowDark = crShadowDark;
    rec.crShadowLight = crShadowLight;
    bRet = SS_BuffAppend(lpBuff, &rec, sizeof(rec));
  }
  return bRet;
}

//--------------------------------------------------------------------

static long SS_Save4PaletteSize(int nBitCount)
{
  long lPalleteSize = 0;

  if( nBitCount <= 8 )
    lPalleteSize = (1 << nBitCount) * sizeof(RGBQUAD);
  else if( nBitCount <= 16 )
    lPalleteSize = 3 * sizeof(DWORD);
  else if( nBitCount <= 24 )
    lPalleteSize = 0;
  else if( nBitCount <= 32 )
    lPalleteSize = 3 * sizeof(DWORD);
  return lPalleteSize;
}

//--------------------------------------------------------------------
//
//  Bitmaps are stored as...
//    BITMAPINFOHEADER bmih
//    RGBQUAD rgb[n]
//    BYTE bImage[m]
//

HBITMAP SS_Load4Bitmap(HPBYTE lpMem, long lMemLen)
{
  LPBITMAPINFOHEADER lpbmih = (LPBITMAPINFOHEADER)(lpMem);
  HBITMAP hbmp = 0;
  HDC hdc;
  long lPaletteSize;

  if( lMemLen )
  {
    lPaletteSize = SS_Save4PaletteSize(lpbmih->biBitCount);
    hdc = GetDC(NULL);
    hbmp = CreateDIBitmap(hdc, lpbmih, CBM_INIT,
                          (LPBYTE)lpbmih + lpbmih->biSize + lPaletteSize,
                          (LPBITMAPINFO)lpbmih, DIB_RGB_COLORS);
    ReleaseDC(NULL, hdc);
  }
  return hbmp;
}

//--------------------------------------------------------------------

BOOL SS_Save4Bitmap(HBITMAP hbmp, LPSS_BUFF lpBuff, LPLONG lplLen)
{
  HDC hdc;
  BITMAP bm;
  LPBITMAPINFOHEADER lpbmih;
#if SS_V80
//  BITMAPV5HEADER bmpV5, *lpbmpV5;
#endif
  HGLOBAL hMem;
  int nBitCount;
  long lMemSize = 0;
  long lPaletteSize;
  BOOL bRet = TRUE;

  if( hbmp )
  {
    hdc = GetDC(NULL);
#if SS_V80
/*	if( GetObject(hbmp, sizeof(BITMAPV5HEADER), (LPVOID)&bmpV5) && bmpV5.bV5Intent)
	{
		nBitCount = bmpV5.bV5Planes * bmpV5.bV5BitCount;
		if( 16 == nBitCount || 32 == nBitCount )
		  nBitCount = 24;  // 16 & 32 not supported in 16-bit windows
		lPaletteSize = SS_Save4PaletteSize(nBitCount);
		lMemSize = sizeof(BITMAPV5HEADER) + lPaletteSize;
		hMem = GlobalAlloc(GHND, lMemSize);
		lpbmpV5 = (LPBITMAPV5HEADER)GlobalLock(hMem);
		lpbmpV5->bV5Size = sizeof(BITMAPV5HEADER);
		lpbmpV5->bV5Width = bmpV5.bV5Width;
		lpbmpV5->bV5Height = bmpV5.bV5Height;
		lpbmpV5->bV5Planes = 1;
		lpbmpV5->bV5BitCount = nBitCount;
		lpbmpV5->bV5Compression = BI_RGB;
		GetDIBits(hdc, hbmp, 0, (UINT)lpbmpV5->bV5Height, NULL,
				  (LPBITMAPINFO)lpbmpV5, DIB_RGB_COLORS);
		lMemSize += lpbmpV5->bV5SizeImage;
		GlobalUnlock(hMem);
		hMem = GlobalReAlloc(hMem, lMemSize, 0);
		lpbmpV5 = (LPBITMAPV5HEADER)GlobalLock(hMem);
		GetDIBits(hdc, hbmp, 0, (UINT)lpbmpV5->bV5Height,
				  (LPBYTE)lpbmpV5 + lpbmpV5->bV5Size + lPaletteSize,
				  (LPBITMAPINFO)lpbmpV5, DIB_RGB_COLORS);
		bRet &= SS_BuffAppend(lpBuff, lpbmpV5, lMemSize);
		GlobalUnlock(hMem);
		GlobalFree(hMem);
		ReleaseDC(NULL, hdc);
	}
	else
	{
	*/
#endif
    GetObject(hbmp, sizeof(bm), &bm);
    nBitCount = bm.bmPlanes * bm.bmBitsPixel;
    if( 16 == nBitCount || 32 == nBitCount )
      nBitCount = 24;  // 16 & 32 not supported in 16-bit windows
    lPaletteSize = SS_Save4PaletteSize(nBitCount);
    lMemSize = sizeof(*lpbmih) + lPaletteSize;
    hMem = GlobalAlloc(GHND, lMemSize);
    lpbmih = (LPBITMAPINFOHEADER)GlobalLock(hMem);
    lpbmih->biSize = sizeof(*lpbmih);
    lpbmih->biWidth = bm.bmWidth;
    lpbmih->biHeight = bm.bmHeight;
    lpbmih->biPlanes = 1;
    lpbmih->biBitCount = nBitCount;
    lpbmih->biCompression = BI_RGB;
    GetDIBits(hdc, hbmp, 0, (UINT)lpbmih->biHeight, NULL,
              (LPBITMAPINFO)lpbmih, DIB_RGB_COLORS);
    lMemSize += lpbmih->biSizeImage;
    GlobalUnlock(hMem);
    hMem = GlobalReAlloc(hMem, lMemSize, 0);
    lpbmih = (LPBITMAPINFOHEADER)GlobalLock(hMem);
    GetDIBits(hdc, hbmp, 0, (UINT)lpbmih->biHeight,
              (LPBYTE)lpbmih + lpbmih->biSize + lPaletteSize,
              (LPBITMAPINFO)lpbmih, DIB_RGB_COLORS);
    bRet &= SS_BuffAppend(lpBuff, lpbmih, lMemSize);
    GlobalUnlock(hMem);
    GlobalFree(hMem);
    ReleaseDC(NULL, hdc);
#if SS_V80
	//}
#endif
  }
  if( lplLen )
    *lplLen = lMemSize;
  return bRet;
}

//--------------------------------------------------------------------
//
//  Icons are stored as...
//    long lMaskLen = # bytes used to store mask bitmap
//    long lColorLen = # bytes used to store color bitmap
//    bmMask = mask bitmap stored via SS_Save4Bitmap()
//    bmColor = color bitmap stored via SS_Save4Bitmap()
//

HICON SS_Load4Icon(HPBYTE lpMem, long lMemLen)
{
  #ifdef WIN32

  HICON hicon = 0;
  ICONINFO iconInfo;
  LPBYTE lpMask;
  LPBYTE lpColor;
  long lMaskLen;
  long lColorLen;
  if( lMemLen )
  {
    lMaskLen = *(LPLONG)lpMem;
    lColorLen = *(LPLONG)(lpMem + sizeof(long));
    lpMask = lpMem + 2 * sizeof(long);
    lpColor = lpMask + lMaskLen;
    iconInfo.fIcon = TRUE;
    iconInfo.xHotspot = 0;  // ignored for icons
    iconInfo.yHotspot = 0;  // ignored for icons
    iconInfo.hbmMask = SS_Load4Bitmap(lpMask, lMaskLen);
    iconInfo.hbmColor = SS_Load4Bitmap(lpColor, lColorLen);
    hicon = CreateIconIndirect(&iconInfo);
    DeleteObject(iconInfo.hbmMask);
    DeleteObject(iconInfo.hbmColor);
  }
  return hicon;

  #else

  return NULL;

  #endif
}

//--------------------------------------------------------------------

BOOL SS_Save4Icon(HICON hicon, LPSS_BUFF lpBuff, LPLONG lplLen)
{
  #ifdef WIN32

  ICONINFO iconInfo;
  long lBuffLenOld = lpBuff->lLen;
  long lMemSize = 0;
  long lMaskLen = 0;
  long lColorLen = 0;
  BOOL bRet = TRUE;
  if( hicon )
  {
    GetIconInfo(hicon, &iconInfo);
    bRet &= SS_BuffAppend(lpBuff, &lMaskLen, sizeof(lMaskLen));
    bRet &= SS_BuffAppend(lpBuff, &lColorLen, sizeof(lColorLen));
    bRet &= SS_Save4Bitmap(iconInfo.hbmMask, lpBuff, &lMaskLen);
    bRet &= SS_Save4Bitmap(iconInfo.hbmColor, lpBuff, &lColorLen);
    DeleteObject(iconInfo.hbmMask);
    DeleteObject(iconInfo.hbmColor);
    if( bRet && lpBuff->pMem )
    {
      MemHugeCpy(lpBuff->pMem + lBuffLenOld, &lMaskLen, sizeof(long));
      MemHugeCpy(lpBuff->pMem + lBuffLenOld + sizeof(long), &lColorLen, sizeof(long));
    }
    lMemSize = 2 * sizeof(long) + lMaskLen + lColorLen;
  }
  if( lplLen )
    *lplLen = lMemSize;
  return bRet;

  #else
  
  HBITMAP hbmp;
  long lMemSize = 0;
  BOOL bRet = TRUE;
  if( hicon)
  {
    hbmp = tbIconToBitmap(GetDesktopWindow(), hicon, RGB(255,255,255));
    bRet &= SS_Save4Bitmap(hbmp, lpBuff, &lMemSize);
    DeleteObject(hbmp);
  }
  if( lplLen )
    *lplLen = lMemSize;
  return bRet;
  
  #endif
}

//--------------------------------------------------------------------

LPTSTR SS_Load4ButtonPict(HPBYTE lpMem, long lMemLen,
                          short nType, BOOL bUnicode,
                          LPSS_BUFF lpBuff)
{
  HANDLE hPict = 0;
  LPTSTR lpszPict = 0;
  if( lMemLen )
  {
    if( nType & SUPERBTN_PICT_HANDLE )
    {
      if( nType & SUPERBTN_PICT_BITMAP )
        hPict = SS_Load4Bitmap(lpMem, lMemLen);
      else if( nType & SUPERBTN_PICT_ICON )
        hPict = SS_Load4Icon(lpMem, lMemLen);
      SS_BuffCopy(lpBuff, &hPict, sizeof(hPict));
      lpszPict = (LPTSTR)lpBuff->pMem;
    }
    else
    {
      SS_BuffCopyStr(lpBuff, lpMem, bUnicode);
      lpszPict = (LPTSTR)lpBuff->pMem;
    }
  }
  return lpszPict;
}

//--------------------------------------------------------------------

BOOL SS_Save4ButtonPict(short nType, HGLOBAL hPict, LPSS_BUFF lpBuff, LPLONG lplLen)
{
  long lMemLen = 0;
  LPTSTR lpszText;
  BOOL bRet = TRUE;
  if( hPict )
  {
    if( nType & SUPERBTN_PICT_HANDLE )
    {
      if( nType & SUPERBTN_PICT_BITMAP )
        bRet &= SS_Save4Bitmap(hPict, lpBuff, &lMemLen);
      else if(nType & SUPERBTN_PICT_ICON )
        bRet &= SS_Save4Icon(hPict, lpBuff, &lMemLen);
    }
    else
    {
      lpszText = (LPTSTR)GlobalLock(hPict);
      lMemLen = (lstrlen(lpszText) + 1) * sizeof(TCHAR);
      bRet &= SS_BuffAppend(lpBuff, lpszText, lMemLen);
      GlobalUnlock(hPict);
    }
  }
  if( lplLen )
    *lplLen = lMemLen;
  return bRet;
}

//--------------------------------------------------------------------

static short ConvertPictType(short nType)
{
  #ifndef WIN32
  if( (nType & SUPERBTN_PICT_HANDLE) && (nType & SUPERBTN_PICT_ICON) )
  {
    nType &= ~SUPERBTN_PICT_ICON;
    nType |= SUPERBTN_PICT_BITMAP;
  }
  #endif
  return nType;
}

//--------------------------------------------------------------------
//
//  Type button is stored as...
//    SS_REC_TYPEBUTTON rec
//    TCHAR szText[n]
//    Up Picture
//    Down Picture
//
//  Note: The text and picture fields are variable length and optional.
//

BOOL SS_Load4TypeButton(LPSPREADSHEET lpSS, LPSS_REC_COMMON lpCommon,
                        BOOL bUnicode, LPSS_LOADBLOCK lpLoadBlock)
{
  LPSS_REC_TYPEBUTTON lpRec = (LPSS_REC_TYPEBUTTON)lpCommon;
  SS_CELLTYPE cellType;
  SUPERBTNCOLOR superBtnColor;
  LPTSTR lpszText = NULL;
  HPBYTE lpText;
  HPBYTE lpPict;
  HPBYTE lpPictDown;
  LPTSTR lpszPict;
  LPTSTR lpszPictDown;
  SS_BUFF buff[3]; // 0=Text, 1=Pict, 2=PictDown
  SS_COORD lCol, lRow;
  BOOL bRet = (long)sizeof(*lpRec) + lpRec->lTextLen + lpRec->lPictLen
              + lpRec->lPictDownLen == lpRec->lRecLen;
  int i;
  if( bRet &&
      BlockTranslate(lpLoadBlock, lpRec->lCol, lpRec->lRow, &lCol, &lRow) )
  {
    lpText = (HPBYTE)lpRec + sizeof(*lpRec);
    lpPict = lpText + lpRec->lTextLen;
    lpPictDown = lpPict + lpRec->lPictLen;
    superBtnColor.Color = lpRec->crFace;
    superBtnColor.ColorText = lpRec->crText;
    superBtnColor.ColorHighlight = lpRec->crHilight;
    //superBtnColor.? = lpRec->crLight;      //???? needed for Win95 ????
    superBtnColor.ColorShadow = lpRec->crShadow;
    //superBtnColor.? = lpRec->crDkShadow;   //???? needed for Win95 ????
    superBtnColor.ColorBorder = lpRec->crBorder;
    if( -1 == superBtnColor.Color )
      superBtnColor.Color = RGBCOLOR_DEFAULT;
    if( -1 == superBtnColor.ColorText )
      superBtnColor.ColorText = RGBCOLOR_DEFAULT;
    if( -1 == superBtnColor.ColorHighlight )
      superBtnColor.ColorHighlight = RGBCOLOR_DEFAULT;
    if( -1 == superBtnColor.ColorShadow )
      superBtnColor.ColorShadow = RGBCOLOR_DEFAULT;
    if( -1 == superBtnColor.ColorBorder )
      superBtnColor.ColorBorder = RGBCOLOR_DEFAULT;
    for( i = 0; i < sizeof(buff) / sizeof(buff[0]); i++ )
      SS_BuffInit(&buff[i]);
    if( lpRec->lTextLen )
      lpszText = SS_BuffCopyStr(&buff[0], lpText, bUnicode);
    lpszPict = SS_Load4ButtonPict(lpPict, lpRec->lPictLen,
                                  lpRec->nPictType, bUnicode,
                                  &buff[1]);
    lpszPictDown = SS_Load4ButtonPict(lpPictDown, lpRec->lPictDownLen,
                                      lpRec->nPictDownType, bUnicode,
                                      &buff[2]);
    SS_SetTypeButton(lpSS, &cellType, lpRec->lStyle, lpszText,
                     lpszPict, lpRec->nPictType,
                     lpszPictDown, lpRec->nPictDownType,
                     lpRec->nButtonType, lpRec->nShadowSize,
                     &superBtnColor);
    if( cellType.Spec.Button.hPictName &&
        (cellType.Spec.Button.nPictureType & SUPERBTN_PICT_HANDLE) )
    {
      #if defined(SS_VB)
      cellType.Spec.Button.hPic =
        CreateVbPict(cellType.Spec.Button.hPictName,
                     (cellType.Spec.Button.nPictureType
                      & SUPERBTN_PICT_BITMAP) == SUPERBTN_PICT_BITMAP,
                     (cellType.Spec.Button.nPictureType
                      & SUPERBTN_PICT_ICON) == SUPERBTN_PICT_ICON);
      #elif defined(SS_OCX)
      cellType.Spec.Button.lpPict =
        CreateOcxPict(cellType.Spec.Button.hPictName,
                      (cellType.Spec.Button.nPictureType
                       & SUPERBTN_PICT_BITMAP) == SUPERBTN_PICT_BITMAP,
                      (cellType.Spec.Button.nPictureType
                       & SUPERBTN_PICT_ICON) == SUPERBTN_PICT_ICON);
      #else
      SS_PicRef(lpSS->lpBook, (HANDLE)cellType.Spec.Button.hPictName);
      cellType.Spec.Button.fDelHandle = TRUE;
      #endif
    }
    if( cellType.Spec.Button.hPictDownName &&
        (cellType.Spec.Button.nPictureDownType & SUPERBTN_PICT_HANDLE) )
    {
      #if defined(SS_VB)
      cellType.Spec.Button.hPicDown =
        CreateVbPict(cellType.Spec.Button.hPictDownName,
                     (cellType.Spec.Button.nPictureDownType
                      & SUPERBTN_PICT_BITMAP) == SUPERBTN_PICT_BITMAP,
                     (cellType.Spec.Button.nPictureDownType
                      & SUPERBTN_PICT_ICON) == SUPERBTN_PICT_ICON);
      #elif defined(SS_OCX)
      cellType.Spec.Button.lpPictDown =
        CreateOcxPict(cellType.Spec.Button.hPictDownName,
                      (cellType.Spec.Button.nPictureDownType
                       & SUPERBTN_PICT_BITMAP) == SUPERBTN_PICT_BITMAP,
                      (cellType.Spec.Button.nPictureDownType
                       & SUPERBTN_PICT_ICON) == SUPERBTN_PICT_ICON);
      #else
      SS_PicRef(lpSS->lpBook, (HANDLE)cellType.Spec.Button.hPictDownName);
      cellType.Spec.Button.fDelDownHandle = TRUE;
      #endif
    }
    SS_SetCellTypeRange(lpSS, lCol, lRow, lCol, lRow, &cellType);
    for( i = 0; i < sizeof(buff) / sizeof(buff[0]); i++ )
      SS_BuffFree(&buff[i]);
  }
  return bRet;
}

//--------------------------------------------------------------------

BOOL SS_Save4TypeButton(LPSPREADSHEET lpSS, SS_COORD lCol, SS_COORD lRow,
                        LPSS_CELLTYPE lpCellType, LPSS_BUFF lpBuff)
{
  SS_REC_TYPEBUTTON rec;
  LPTSTR lpszText = NULL;
  long lBuffOldLen = lpBuff->lLen;
  long lTextLen = 0;
  long lPictLen;
  long lPictDownLen;
  BOOL bRet;
  rec.nRecType = SS_RID_TYPEBUTTON;
  rec.lRecLen = sizeof(rec);
  rec.lCol = lCol;
  rec.lRow = lRow;
  rec.lStyle = lpCellType->Style;
  rec.nPictType = lpCellType->Spec.Button.nPictureType;
  rec.nPictDownType = lpCellType->Spec.Button.nPictureDownType;
  rec.nButtonType = lpCellType->Spec.Button.ButtonType;
  rec.nShadowSize = lpCellType->Spec.Button.ShadowSize;
  rec.crFace = lpCellType->Spec.Button.Color.Color;
  rec.crText = lpCellType->Spec.Button.Color.ColorText;
  rec.crHilight = lpCellType->Spec.Button.Color.ColorHighlight;
  rec.crLight = (COLORREF)-1;
  rec.crShadow = lpCellType->Spec.Button.Color.ColorShadow;
  rec.crDkShadow = (COLORREF)-1;
  rec.crBorder = lpCellType->Spec.Button.Color.ColorBorder;
  if( SS_ISDEFCOLOR(rec.crFace) )
    rec.crFace = (COLORREF)-1;
  if( SS_ISDEFCOLOR(rec.crText) )
    rec.crText = (COLORREF)-1;
  if( SS_ISDEFCOLOR(rec.crHilight) )
    rec.crHilight = (COLORREF)-1;
  if( SS_ISDEFCOLOR(rec.crLight) )
    rec.crLight = (COLORREF)-1;
  if( SS_ISDEFCOLOR(rec.crShadow) )
    rec.crShadow = (COLORREF)-1;
  if( SS_ISDEFCOLOR(rec.crDkShadow) )
    rec.crDkShadow = (COLORREF)-1;
  if( SS_ISDEFCOLOR(rec.crBorder) )
    rec.crBorder = (COLORREF)-1;
  rec.lTextLen = 0;
  rec.lPictLen = 0;
  rec.lPictDownLen = 0;
  #ifndef WIN32
  rec.nPictType = ConvertPictType(rec.nPictType);
  rec.nPictDownType = ConvertPictType(rec.nPictDownType);
  #endif
  bRet = SS_BuffAppend(lpBuff, &rec, sizeof(rec));
  if( lpCellType->Spec.Button.hText )
  {
    lpszText = (LPTSTR)GlobalLock(lpCellType->Spec.Button.hText);
    lTextLen = (lstrlen(lpszText) + 1) * sizeof(TCHAR);
    bRet &= SS_BuffAppend(lpBuff, lpszText, lTextLen);
    GlobalUnlock(lpCellType->Spec.Button.hText);
  }
  bRet &= SS_Save4ButtonPict(lpCellType->Spec.Button.nPictureType,
                             lpCellType->Spec.Button.hPictName,
                             lpBuff, &lPictLen);
  bRet &= SS_Save4ButtonPict(lpCellType->Spec.Button.nPictureDownType,
                             lpCellType->Spec.Button.hPictDownName,
                             lpBuff, &lPictDownLen);
  if( bRet && lpBuff->pMem )
    MemHugeCpy(&rec, lpBuff->pMem + lBuffOldLen, sizeof(rec));
  rec.lRecLen = lpBuff->lLen - lBuffOldLen;
  rec.lTextLen = lTextLen;
  rec.lPictLen = lPictLen;
  rec.lPictDownLen = lPictDownLen;
  if( bRet && lpBuff->pMem )
    MemHugeCpy(lpBuff->pMem + lBuffOldLen, &rec, sizeof(rec));
  return bRet;
}

//--------------------------------------------------------------------

LPTSTR SS_Load4CheckPict(HPBYTE lpMem, long lMemLen, short nType,
                         BOOL bUnicode, LPSS_BUFF lpBuff)
{
  HANDLE hPict = 0;
  LPTSTR lpszPict = 0;
  if( lMemLen )
  {
    if( nType & SUPERBTN_PICT_HANDLE )
    {
      if( nType & SUPERBTN_PICT_BITMAP )
        hPict = SS_Load4Bitmap(lpMem, lMemLen);
      else if( nType & SUPERBTN_PICT_ICON )
        hPict = SS_Load4Icon(lpMem, lMemLen);
      SS_BuffCopy(lpBuff, &hPict, sizeof(hPict));
      lpszPict = (LPTSTR)lpBuff->pMem;
    }
    else
    {
      SS_BuffCopyStr(lpBuff, lpMem, bUnicode);
      lpszPict = (LPTSTR)lpBuff->pMem;
    }
  }
  return lpszPict;
}

//--------------------------------------------------------------------

BOOL SS_Save4CheckPict(short nType, TBGLOBALHANDLE hPict, LPSS_BUFF lpBuff, LPLONG lplLen)
{
  long lMemLen = 0;
  LPTSTR lpszText;
  BOOL bRet = TRUE;
  if( hPict )
  {
    if( nType & SUPERBTN_PICT_HANDLE )
    {
      if( nType & SUPERBTN_PICT_BITMAP )
        bRet &= SS_Save4Bitmap((HBITMAP)hPict, lpBuff, &lMemLen);
      else if(nType & SUPERBTN_PICT_ICON )
        bRet &= SS_Save4Icon((HICON)hPict, lpBuff, &lMemLen);
    }
    else
    {
      lpszText = (LPTSTR)tbGlobalLock(hPict);
      lMemLen = (lstrlen(lpszText) + 1) * sizeof(TCHAR);
      bRet &= SS_BuffAppend(lpBuff, lpszText, lMemLen);
      tbGlobalUnlock(hPict);
    }
  }
  if( lplLen )
    *lplLen = lMemLen;
  return bRet;
}

//--------------------------------------------------------------------
//
//  TypeCheck is stored as...
//    SS_REC_TYPECHECK rec
//    TCHAR szText[n]
//    Unchecked Picture
//    Checked Picture
//    Grayed Picture
//    Focus Unchecked Picture
//    Focus Checked Picture
//    Focus Grayed Picture
//    Disabled Unchecked Picture
//    Disabled Checked Picture
//    Disabled Grayed Picture
//
//  Note: The text and picture fields are all variable length
//        and optional.
//

BOOL SS_Load4TypeCheck(LPSPREADSHEET lpSS, LPSS_REC_COMMON lpCommon,
                       BOOL bUnicode, LPSS_LOADBLOCK lpLoadBlock)
{
  LPSS_REC_TYPECHECK lpRec = (LPSS_REC_TYPECHECK)lpCommon;
  SS_CELLTYPE cellType;
  LPTSTR lpszText = NULL;
  HPBYTE lpText;
  HPBYTE lpPictUp;
  HPBYTE lpPictDown;
  HPBYTE lpPictGray;
  HPBYTE lpPictFocUp;
  HPBYTE lpPictFocDown;
  HPBYTE lpPictFocGray;
  HPBYTE lpPictDisUp;
  HPBYTE lpPictDisDown;
  HPBYTE lpPictDisGray;
  LPTSTR lpszPictUp;
  LPTSTR lpszPictDown;
  LPTSTR lpszPictGray;
  LPTSTR lpszPictFocUp;
  LPTSTR lpszPictFocDown;
  LPTSTR lpszPictFocGray;
  LPTSTR lpszPictDisUp;
  LPTSTR lpszPictDisDown;
  LPTSTR lpszPictDisGray;
  SS_BUFF buff[10];  // 0=Text, 1=PictUp, 2=PictDown, 3=PictGray,
                       // 4=PictFocUp, 5=PictFocDown, 6=PictFocGray,
                       // 7=PictDisUp, 8=PictDisDown, 9=PictDisGray
  SS_COORD lCol, lRow;
  BOOL bRet = (long)sizeof(*lpRec) + lpRec->lTextLen + lpRec->lPictUpLen
              + lpRec->lPictDownLen + lpRec->lPictGrayLen
              + lpRec->lPictFocUpLen + lpRec->lPictFocDownLen
              + lpRec->lPictFocGrayLen + lpRec->lPictDisUpLen
              + lpRec->lPictDisDownLen + lpRec->lPictDisGrayLen
              == lpRec->lRecLen;
  int i;
  if( bRet &&
      BlockTranslate(lpLoadBlock, lpRec->lCol, lpRec->lRow, &lCol, &lRow) )
  {
    lpText = (HPBYTE)lpRec + sizeof(*lpRec);
    lpPictUp = lpText + lpRec->lTextLen;
    lpPictDown = lpPictUp + lpRec->lPictUpLen;
    lpPictGray = lpPictDown + lpRec->lPictDownLen;
    lpPictFocUp = lpPictGray + lpRec->lPictGrayLen;
    lpPictFocDown = lpPictFocUp + lpRec->lPictFocUpLen;
    lpPictFocGray = lpPictFocDown + lpRec->lPictFocDownLen;
    lpPictDisUp = lpPictFocGray + lpRec->lPictFocGrayLen;
    lpPictDisDown = lpPictDisUp + lpRec->lPictDisUpLen;
    lpPictDisGray = lpPictDisDown + lpRec->lPictDisDownLen;
    for( i = 0; i < sizeof(buff) / sizeof(buff[0]); i++ )
      SS_BuffInit(&buff[i]);
    if( lpRec->lTextLen )
      lpszText = SS_BuffCopyStr(&buff[0], lpText, bUnicode);
    lpszPictUp = SS_Load4CheckPict(lpPictUp, lpRec->lPictUpLen,
                                   lpRec->nPictUpType, bUnicode,
                                   &buff[1]);
    lpszPictDown = SS_Load4CheckPict(lpPictDown, lpRec->lPictDownLen,
                                     lpRec->nPictDownType, bUnicode,
                                     &buff[2]);
    lpszPictGray = SS_Load4CheckPict(lpPictGray, lpRec->lPictGrayLen,
                                     lpRec->nPictGrayType, bUnicode,
                                     &buff[3]);
    lpszPictFocUp = SS_Load4CheckPict(lpPictFocUp, lpRec->lPictFocUpLen,
                                      lpRec->nPictFocUpType, bUnicode,
                                      &buff[4]);
    lpszPictFocDown = SS_Load4CheckPict(lpPictFocDown, lpRec->lPictFocDownLen,
                                        lpRec->nPictFocDownType, bUnicode,
                                        &buff[5]);
    lpszPictFocGray = SS_Load4CheckPict(lpPictFocGray, lpRec->lPictFocGrayLen,
                                        lpRec->nPictFocGrayType, bUnicode,
                                        &buff[6]);
    lpszPictDisUp = SS_Load4CheckPict(lpPictDisUp, lpRec->lPictDisUpLen,
                                      lpRec->nPictDisUpType, bUnicode,
                                      &buff[7]);
    lpszPictDisDown = SS_Load4CheckPict(lpPictDisDown, lpRec->lPictDisDownLen,
                                        lpRec->nPictDisDownType, bUnicode,
                                        &buff[8]);
    lpszPictDisGray = SS_Load4CheckPict(lpPictDisGray, lpRec->lPictDisGrayLen,
                                        lpRec->nPictDisGrayType, bUnicode,
                                        &buff[9]);
    SS_SetTypeCheckBox(lpSS, &cellType, lpRec->lStyle, lpszText,
                       lpszPictUp, lpRec->nPictUpType,
                       lpszPictDown, lpRec->nPictDownType,
                       lpszPictFocUp, lpRec->nPictFocUpType,
                       lpszPictFocDown, lpRec->nPictFocDownType,
                       lpszPictGray, lpRec->nPictGrayType,
                       lpszPictFocGray, lpRec->nPictFocGrayType);
    
    if( lpszPictUp && cellType.Spec.CheckBox.hPictUpName &&
        (cellType.Spec.CheckBox.bPictUpType & SUPERBTN_PICT_HANDLE) )
    {
      #if defined(SS_VB)
      cellType.Spec.CheckBox.hPicUp =
        CreateVbPict((HANDLE)cellType.Spec.CheckBox.hPictUpName,
                     (cellType.Spec.CheckBox.bPictUpType
                      & BT_BITMAP) == BT_BITMAP,
                     (cellType.Spec.CheckBox.bPictUpType
                      & BT_ICON) == BT_ICON);
      #elif defined(SS_OCX)
      cellType.Spec.CheckBox.lpPictUp =
        CreateOcxPict((HANDLE)cellType.Spec.CheckBox.hPictUpName,
                      (cellType.Spec.CheckBox.bPictUpType
                       & BT_BITMAP) == BT_BITMAP,
                      (cellType.Spec.CheckBox.bPictUpType
                       & BT_ICON) == BT_ICON);
      #else
      SS_PicRef(lpSS->lpBook, (HANDLE)cellType.Spec.CheckBox.hPictUpName);
      cellType.Spec.CheckBox.fDelUpHandle = TRUE;
      #endif
    }

    if( lpszPictDown && cellType.Spec.CheckBox.hPictDownName &&
        (cellType.Spec.CheckBox.bPictDownType & SUPERBTN_PICT_HANDLE) )
    {
      #if defined(SS_VB)
      cellType.Spec.CheckBox.hPicDown =
        CreateVbPict((HANDLE)cellType.Spec.CheckBox.hPictDownName,
                     (cellType.Spec.CheckBox.bPictDownType
                      & BT_BITMAP) == BT_BITMAP,
                     (cellType.Spec.CheckBox.bPictDownType
                      & BT_ICON) == BT_ICON);
      #elif defined(SS_OCX)
      cellType.Spec.CheckBox.lpPictDown =
        CreateOcxPict((HANDLE)cellType.Spec.CheckBox.hPictDownName,
                      (cellType.Spec.CheckBox.bPictDownType
                       & BT_BITMAP) == BT_BITMAP,
                      (cellType.Spec.CheckBox.bPictDownType
                       & BT_ICON) == BT_ICON);
      #else
      SS_PicRef(lpSS->lpBook, (HANDLE)cellType.Spec.CheckBox.hPictDownName);
      cellType.Spec.CheckBox.fDelDownHandle = TRUE;
      #endif
    }

    if( lpszPictGray && cellType.Spec.CheckBox.hPictGrayName &&
        (cellType.Spec.CheckBox.bPictGrayType & SUPERBTN_PICT_HANDLE) )
    {
      #if defined(SS_VB)
      cellType.Spec.CheckBox.hPicGray =
        CreateVbPict((HANDLE)cellType.Spec.CheckBox.hPictGrayName,
                     (cellType.Spec.CheckBox.bPictGrayType
                      & BT_BITMAP) == BT_BITMAP,
                     (cellType.Spec.CheckBox.bPictGrayType
                      & BT_ICON) == BT_ICON);
      #elif defined(SS_OCX)
      cellType.Spec.CheckBox.lpPictGray =
        CreateOcxPict((HANDLE)cellType.Spec.CheckBox.hPictGrayName,
                      (cellType.Spec.CheckBox.bPictGrayType
                       & BT_BITMAP) == BT_BITMAP,
                      (cellType.Spec.CheckBox.bPictGrayType
                       & BT_ICON) == BT_ICON);
      #else
      SS_PicRef(lpSS->lpBook, (HANDLE)cellType.Spec.CheckBox.hPictGrayName);
      cellType.Spec.CheckBox.fDelGrayHandle = TRUE;
      #endif
    }

    if( lpszPictFocUp && cellType.Spec.CheckBox.hPictFocusUpName &&
        (cellType.Spec.CheckBox.bPictFocusUpType & SUPERBTN_PICT_HANDLE) )
    {
      #if defined(SS_VB)
      cellType.Spec.CheckBox.hPicFocusUp =
        CreateVbPict((HANDLE)cellType.Spec.CheckBox.hPictFocusUpName,
                     (cellType.Spec.CheckBox.bPictFocusUpType
                      & BT_BITMAP) == BT_BITMAP,
                     (cellType.Spec.CheckBox.bPictFocusUpType
                      & BT_ICON) == BT_ICON);
      #elif defined(SS_OCX)
      cellType.Spec.CheckBox.lpPictFocusUp =
        CreateOcxPict((HANDLE)cellType.Spec.CheckBox.hPictFocusUpName,
                      (cellType.Spec.CheckBox.bPictFocusUpType
                       & BT_BITMAP) == BT_BITMAP,
                      (cellType.Spec.CheckBox.bPictFocusUpType
                       & BT_ICON) == BT_ICON);
      #else
      SS_PicRef(lpSS->lpBook, (HANDLE)cellType.Spec.CheckBox.hPictFocusUpName);
      cellType.Spec.CheckBox.fDelFocusUpHandle = TRUE;
      #endif
    }

    if( lpszPictFocDown && cellType.Spec.CheckBox.hPictFocusDownName &&
        (cellType.Spec.CheckBox.bPictFocusDownType & SUPERBTN_PICT_HANDLE) )
    {
      #if defined(SS_VB)
      cellType.Spec.CheckBox.hPicFocusDown =
        CreateVbPict((HANDLE)cellType.Spec.CheckBox.hPictFocusDownName,
                     (cellType.Spec.CheckBox.bPictFocusDownType
                      & BT_BITMAP) == BT_BITMAP,
                     (cellType.Spec.CheckBox.bPictFocusDownType
                      & BT_ICON) == BT_ICON);
      #elif defined(SS_OCX)
      cellType.Spec.CheckBox.lpPictFocusDown =
        CreateOcxPict((HANDLE)cellType.Spec.CheckBox.hPictFocusDownName,
                      (cellType.Spec.CheckBox.bPictFocusDownType
                       & BT_BITMAP) == BT_BITMAP,
                      (cellType.Spec.CheckBox.bPictFocusDownType
                       & BT_ICON) == BT_ICON);
      #else
      SS_PicRef(lpSS->lpBook, (HANDLE)cellType.Spec.CheckBox.hPictFocusDownName);
      cellType.Spec.CheckBox.fDelFocusDownHandle = TRUE;
      #endif
    }

    if( lpszPictFocGray && cellType.Spec.CheckBox.hPictFocusGrayName &&
        (cellType.Spec.CheckBox.bPictFocusGrayType & SUPERBTN_PICT_HANDLE) )
    {
      #if defined(SS_VB)
      cellType.Spec.CheckBox.hPicFocusGray =
        CreateVbPict((HANDLE)cellType.Spec.CheckBox.hPictFocusGrayName,
                     (cellType.Spec.CheckBox.bPictFocusGrayType
                      & BT_BITMAP) == BT_BITMAP,
                     (cellType.Spec.CheckBox.bPictFocusGrayType
                      & BT_ICON) == BT_ICON);
      #elif defined(SS_OCX)
      cellType.Spec.CheckBox.lpPictFocusGray =
        CreateOcxPict((HANDLE)cellType.Spec.CheckBox.hPictFocusGrayName,
                      (cellType.Spec.CheckBox.bPictFocusGrayType
                       & BT_BITMAP) == BT_BITMAP,
                      (cellType.Spec.CheckBox.bPictFocusGrayType
                       & BT_ICON) == BT_ICON);
      #else
      SS_PicRef(lpSS->lpBook, (HANDLE)cellType.Spec.CheckBox.hPictFocusGrayName);
      cellType.Spec.CheckBox.fDelFocusGrayHandle = TRUE;
      #endif
    }
    SS_SetCellTypeRange(lpSS, lCol, lRow, lCol, lRow, &cellType);
    for( i = 0; i < sizeof(buff) / sizeof(buff[0]); i++ )
      SS_BuffFree(&buff[i]);
  }
  return bRet;
}

//--------------------------------------------------------------------

BOOL SS_Save4TypeCheck(LPSPREADSHEET lpSS, SS_COORD lCol, SS_COORD lRow,
                       LPSS_CELLTYPE lpCellType, LPSS_BUFF lpBuff)
{
  SS_REC_TYPECHECK rec;
  CHECKBOXPICTS defPicts;
  LPTSTR lpszText;
  long lTextLen = 0;
  long lPictUpLen = 0;
  long lPictDownLen = 0;
  long lPictGrayLen = 0;
  long lPictFocUpLen = 0;
  long lPictFocDownLen = 0;
  long lPictFocGrayLen = 0;
  long lPictDisUpLen = 0;
  long lPictDisDownLen = 0;
  long lPictDisGrayLen = 0;
  long lBuffLenOld = lpBuff->lLen;
  BOOL bRet;

  rec.nRecType = SS_RID_TYPECHECK;
  rec.lRecLen = sizeof(rec);
  rec.lCol = lCol;
  rec.lRow = lRow;
  rec.lStyle = lpCellType->Style;
  rec.nPictUpType = lpCellType->Spec.CheckBox.bPictUpType;
  rec.nPictDownType = lpCellType->Spec.CheckBox.bPictDownType;
  rec.nPictGrayType = lpCellType->Spec.CheckBox.bPictGrayType;
  rec.nPictFocUpType = lpCellType->Spec.CheckBox.bPictFocusUpType;
  rec.nPictFocDownType = lpCellType->Spec.CheckBox.bPictFocusDownType;
  rec.nPictFocGrayType = lpCellType->Spec.CheckBox.bPictFocusGrayType;
  rec.nPictDisUpType = 0;
  rec.nPictDisDownType = 0;
  rec.nPictDisGrayType = 0;
  rec.lTextLen = 0;
  rec.lPictUpLen = 0;
  rec.lPictDownLen = 0;
  rec.lPictGrayLen = 0;
  rec.lPictFocUpLen = 0;
  rec.lPictFocDownLen = 0;
  rec.lPictFocGrayLen = 0;
  rec.lPictDisUpLen = 0;
  rec.lPictDisDownLen = 0;
  rec.lPictDisGrayLen = 0;
  #ifndef WIN32
  rec.nPictUpType = ConvertPictType(rec.nPictUpType);
  rec.nPictDownType = ConvertPictType(rec.nPictDownType);
  rec.nPictGrayType = ConvertPictType(rec.nPictGrayType);
  rec.nPictFocUpType = ConvertPictType(rec.nPictFocUpType);
  rec.nPictFocDownType = ConvertPictType(rec.nPictFocDownType);
  rec.nPictFocGrayType = ConvertPictType(rec.nPictFocGrayType);
  rec.nPictDisUpType = ConvertPictType(rec.nPictDisUpType);
  rec.nPictDisDownType = ConvertPictType(rec.nPictDisDownType);
  rec.nPictDisGrayType = ConvertPictType(rec.nPictDisGrayType);
  #endif
  bRet = SS_BuffAppend(lpBuff, &rec, sizeof(rec));
  if( lpCellType->Spec.CheckBox.hText )
  {
    lpszText = (LPTSTR)GlobalLock(lpCellType->Spec.CheckBox.hText);
    lTextLen = (lstrlen(lpszText) + 1) * sizeof(TCHAR);
    bRet &= SS_BuffAppend(lpBuff, lpszText, lTextLen);
    GlobalUnlock(lpCellType->Spec.CheckBox.hText);
  }
  if( lpSS->lpBook->fCheckBoxDefBitmapsLoaded )
    tbCheckBoxGetPict(hDynamicInst, &defPicts);
  else
    _fmemset(&defPicts, 0, sizeof(defPicts));
  if( lpCellType->Spec.CheckBox.hPictUpName != (TBGLOBALHANDLE)defPicts.PictUp.hPict )
    bRet &= SS_Save4CheckPict(lpCellType->Spec.CheckBox.bPictUpType,
                              lpCellType->Spec.CheckBox.hPictUpName,
                              lpBuff, &lPictUpLen);
  if( lpCellType->Spec.CheckBox.hPictDownName != (TBGLOBALHANDLE)defPicts.PictDown.hPict )
    bRet &= SS_Save4CheckPict(lpCellType->Spec.CheckBox.bPictDownType,
                              lpCellType->Spec.CheckBox.hPictDownName,
                              lpBuff, &lPictDownLen);
  if( lpCellType->Spec.CheckBox.hPictGrayName != (TBGLOBALHANDLE)defPicts.PictGray.hPict )
    bRet &= SS_Save4CheckPict(lpCellType->Spec.CheckBox.bPictGrayType,
                              lpCellType->Spec.CheckBox.hPictGrayName,
                              lpBuff, &lPictGrayLen);
  if( lpCellType->Spec.CheckBox.hPictFocusUpName != (TBGLOBALHANDLE)defPicts.PictFocusUp.hPict )
    bRet &= SS_Save4CheckPict(lpCellType->Spec.CheckBox.bPictFocusUpType,
                              lpCellType->Spec.CheckBox.hPictFocusUpName,
                              lpBuff, &lPictFocUpLen);
  if( lpCellType->Spec.CheckBox.hPictFocusDownName != (TBGLOBALHANDLE)defPicts.PictFocusDown.hPict )
    bRet &= SS_Save4CheckPict(lpCellType->Spec.CheckBox.bPictFocusDownType,
                              lpCellType->Spec.CheckBox.hPictFocusDownName,
                              lpBuff, &lPictFocDownLen);
  if( lpCellType->Spec.CheckBox.hPictFocusGrayName != (TBGLOBALHANDLE)defPicts.PictFocusGray.hPict )
    bRet &= SS_Save4CheckPict(lpCellType->Spec.CheckBox.bPictFocusGrayType,
                              lpCellType->Spec.CheckBox.hPictFocusGrayName,
                              lpBuff, &lPictFocGrayLen);
  if( bRet && lpBuff->pMem )
     MemHugeCpy(&rec, lpBuff->pMem + lBuffLenOld, sizeof(rec));
  rec.lRecLen = lpBuff->lLen - lBuffLenOld;
  rec.lTextLen = lTextLen;
  rec.lPictUpLen = lPictUpLen;
  rec.lPictDownLen = lPictDownLen;
  rec.lPictGrayLen = lPictGrayLen;
  rec.lPictFocUpLen = lPictFocUpLen;
  rec.lPictFocDownLen = lPictFocDownLen;
  rec.lPictFocGrayLen = lPictFocGrayLen;
  rec.lPictDisUpLen = lPictDisUpLen;
  rec.lPictDisDownLen = lPictDisDownLen;
  rec.lPictDisGrayLen = lPictDisGrayLen;
  if( bRet && lpBuff->pMem )
     MemHugeCpy(lpBuff->pMem + lBuffLenOld, &rec, sizeof(rec));
  return bRet;
}

//--------------------------------------------------------------------

BOOL SS_Load4TypeCombo(LPSPREADSHEET lpSS, LPSS_REC_COMMON lpCommon,
                       BOOL bUnicode, LPSS_LOADBLOCK lpLoadBlock)
{
  LPSS_REC_TYPECOMBO lpRec = (LPSS_REC_TYPECOMBO)lpCommon;
  SS_CELLTYPE cellType;
  HPBYTE lpList = NULL;
  LPTSTR lpszList = NULL;
  SS_COORD lCol, lRow;
  BOOL bRet = sizeof(*lpRec) <= lpRec->lRecLen;
  if( bRet &&
      BlockTranslate(lpLoadBlock, lpRec->lCol, lpRec->lRow, &lCol, &lRow) )
  {
    SS_BUFF buff;
    SS_BuffInit(&buff);
    lpList = (HPBYTE)lpRec + sizeof(*lpRec);
    if( sizeof(*lpRec) < lpRec->lRecLen )
      lpszList = SS_BuffCopyStr(&buff, lpList, bUnicode);
    SS_SetTypeComboBox(lpSS, &cellType, lpRec->lStyle, lpszList, -1, -1, 0, 0, SS_COMBOBOX_AUTOSEARCH_SINGLECHAR);
    SS_SetCellTypeRange(lpSS, lCol, lRow, lCol, lRow, &cellType);
    SS_BuffFree(&buff);
  }
  return bRet;
}

#ifndef SS_V30
//--------------------------------------------------------------------

BOOL SS_Save4TypeCombo(LPSPREADSHEET lpSS, SS_COORD lCol, SS_COORD lRow,
                       LPSS_CELLTYPE lpCellType, LPSS_BUFF lpBuff)
{
  SS_REC_TYPECOMBO rec;
  LPTSTR lpszItem;
  long lItemLen;
  long lBuffLenOld = lpBuff->lLen;
  int i;
  BOOL bRet;

  rec.nRecType = SS_RID_TYPECOMBO;
  rec.lRecLen = sizeof(rec);  // is updated after combo box list is saved
  rec.lCol = lCol;
  rec.lRow = lRow;
  rec.lStyle = lpCellType->Style;
  bRet = SS_BuffAppend(lpBuff, &rec, sizeof(rec));
  if( lpCellType->Spec.ComboBox.hItems &&
      (lpszItem = (LPTSTR)tbGlobalLock(lpCellType->Spec.ComboBox.hItems)) )
  {
    for( i = 0; bRet && i < lpCellType->Spec.ComboBox.dItemCnt; i++ )
    {
      lItemLen = lstrlen(lpszItem);
      bRet &= SS_BuffAppend(lpBuff, lpszItem, lItemLen * sizeof(TCHAR));
      if( i < lpCellType->Spec.ComboBox.dItemCnt - 1 )
        bRet &= SS_BuffAppend(lpBuff, _T("\t"), sizeof(TCHAR));
      else
        bRet &= SS_BuffAppend(lpBuff, _T("\0"), sizeof(TCHAR));
      lpszItem += lItemLen + 1;
    }
    tbGlobalUnlock(lpCellType->Spec.ComboBox.hItems);
    if( bRet && lpBuff->pMem )
      MemHugeCpy(&rec, lpBuff->pMem + lBuffLenOld, sizeof(rec));
    rec.lRecLen = lpBuff->lLen - lBuffLenOld;
    if( bRet && lpBuff->pMem )
      MemHugeCpy(lpBuff->pMem + lBuffLenOld, &rec, sizeof(rec));
  }
  return bRet;
}

#else

//--------------------------------------------------------------------

BOOL SS_Load4TypeComboEx(LPSPREADSHEET lpSS, LPSS_REC_COMMON lpCommon,
                         BOOL bUnicode, LPSS_LOADBLOCK lpLoadBlock)
{
  LPSS_REC_TYPECOMBOEX lpRec = (LPSS_REC_TYPECOMBOEX)lpCommon;
  SS_CELLTYPE cellType;
  HPBYTE lpList = NULL;
  LPTSTR lpszList = NULL;
  SS_COORD lCol, lRow;
  BOOL bRet = sizeof(*lpRec) <= lpRec->lRecLen;
  if( bRet &&
      BlockTranslate(lpLoadBlock, lpRec->lCol, lpRec->lRow, &lCol, &lRow) )
  {
    SS_BUFF buff;
    SS_BuffInit(&buff);
    lpList = (HPBYTE)lpRec + sizeof(*lpRec);
    if( sizeof(*lpRec) < lpRec->lRecLen )
      lpszList = SS_BuffCopyStr(&buff, lpList, bUnicode);
    SS_SetTypeComboBox(lpSS, &cellType, lpRec->lStyle, lpszList,
                       lpRec->dMaxRows, lpRec->dMaxEditLen, lpRec->dComboWidth, 0, SS_COMBOBOX_AUTOSEARCH_SINGLECHAR);
    SS_SetCellTypeRange(lpSS, lCol, lRow, lCol, lRow, &cellType);
    SS_BuffFree(&buff);
  }
  return bRet;
}

//--------------------------------------------------------------------

BOOL SS_Save4TypeComboEx(LPSPREADSHEET lpSS, SS_COORD lCol, SS_COORD lRow,
                       LPSS_CELLTYPE lpCellType, LPSS_BUFF lpBuff)
{
  SS_REC_TYPECOMBOEX rec;
  LPTSTR lpszItem;
  long lItemLen;
  long lBuffLenOld = lpBuff->lLen;
  int i;
  BOOL bRet;

  rec.nRecType = SS_RID_TYPECOMBOEX;
  rec.lRecLen = sizeof(rec);  // is updated after combo box list is saved
  rec.lCol = lCol;
  rec.lRow = lRow;
  rec.lStyle = lpCellType->Style;
  rec.dMaxRows = lpCellType->Spec.ComboBox.dMaxRows;
  rec.dMaxEditLen = lpCellType->Spec.ComboBox.dMaxEditLen;
  rec.dComboWidth = lpCellType->Spec.ComboBox.dComboWidth;
  bRet = SS_BuffAppend(lpBuff, &rec, sizeof(rec));
  if( lpCellType->Spec.ComboBox.hItems &&
      (lpszItem = (LPTSTR)tbGlobalLock(lpCellType->Spec.ComboBox.hItems)) )
  {
    for( i = 0; bRet && i < lpCellType->Spec.ComboBox.dItemCnt; i++ )
    {
      lItemLen = lstrlen(lpszItem);
      bRet &= SS_BuffAppend(lpBuff, lpszItem, lItemLen * sizeof(TCHAR));
      if( i < lpCellType->Spec.ComboBox.dItemCnt - 1 )
        bRet &= SS_BuffAppend(lpBuff, _T("\t"), sizeof(TCHAR));
      else
        bRet &= SS_BuffAppend(lpBuff, _T("\0"), sizeof(TCHAR));
      lpszItem += lItemLen + 1;
    }
    tbGlobalUnlock(lpCellType->Spec.ComboBox.hItems);
    if( bRet && lpBuff->pMem )
      MemHugeCpy(&rec, lpBuff->pMem + lBuffLenOld, sizeof(rec));
    rec.lRecLen = lpBuff->lLen - lBuffLenOld;
    if( bRet && lpBuff->pMem )
      MemHugeCpy(lpBuff->pMem + lBuffLenOld, &rec, sizeof(rec));
  }
  return bRet;
}

#endif // !defined(SS_V30)

//--------------------------------------------------------------------

#ifdef SS_V40

BOOL SS_Load4TypeComboEx4(LPSPREADSHEET lpSS, LPSS_REC_COMMON lpCommon,
                         BOOL bUnicode, LPSS_LOADBLOCK lpLoadBlock)
{
  LPSS_REC_TYPECOMBOEX4 lpRec = (LPSS_REC_TYPECOMBOEX4)lpCommon;
  SS_CELLTYPE cellType;
  SS_COORD lCol, lRow;
  BOOL bRet = sizeof(*lpRec) <= lpRec->lRecLen;
  if( bRet &&
      BlockTranslate(lpLoadBlock, lpRec->lCol, lpRec->lRow, &lCol, &lRow) )
  {
    SS_GetCellType(lpSS, lCol, lRow, &cellType);
    cellType.Spec.ComboBox.AutoSearch = lpRec->bAutoSearch;
    SS_SetCellTypeStruct(lpSS, &cellType, NULL, lCol, lRow);
  }
  return bRet;
}

//--------------------------------------------------------------------

BOOL SS_Save4TypeComboEx4(LPSPREADSHEET lpSS, SS_COORD lCol, SS_COORD lRow,
                          LPSS_CELLTYPE lpCellType, LPSS_BUFF lpBuff)
{
  SS_REC_TYPECOMBOEX4 rec;
  BOOL bRet;

  bRet = SS_Save4TypeComboEx(lpSS, lCol, lRow, lpCellType, lpBuff);

  rec.nRecType = SS_RID_TYPECOMBOEX4;
  rec.lRecLen = sizeof(rec);  // is updated after combo box list is saved
  rec.lCol = lCol;
  rec.lRow = lRow;
  rec.bAutoSearch = lpCellType->Spec.ComboBox.AutoSearch;
  bRet = SS_BuffAppend(lpBuff, &rec, sizeof(rec));
  return bRet;
}

#endif // SS_V40

//--------------------------------------------------------------------

BOOL SS_Load4TypeDate(LPSPREADSHEET lpSS, LPSS_REC_COMMON lpCommon,
                      LPSS_LOADBLOCK lpLoadBlock)
{
  LPSS_REC_TYPEDATE lpRec = (LPSS_REC_TYPEDATE)lpCommon;
  SS_CELLTYPE cellType;
  DATEFORMAT dateFormat;
  LPDATEFORMAT lpDateFormat = NULL;
  TB_DATE dateMin;
  TB_DATE dateMax;
  LPTB_DATE lpDateMin = NULL;
  LPTB_DATE lpDateMax = NULL;
  SS_COORD lCol, lRow;
  BOOL bRet = sizeof(*lpRec) == lpRec->lRecLen;
  if( bRet &&
      BlockTranslate(lpLoadBlock, lpRec->lCol, lpRec->lRow, &lCol, &lRow) )
  {
    if( lpRec->bSetFormat )
    {
      dateFormat.nFormat = lpRec->nFormat;
      dateFormat.cSeparator = (TCHAR)lpRec->nSepChar;
      dateFormat.bCentury = lpRec->bCentury;
      dateFormat.bSpin = lpRec->bSpin;
      lpDateFormat = &dateFormat;
    }
    if( lpRec->bSetRange )
    {
      dateMin.nYear = lpRec->nMinYear;
      dateMin.nMonth = lpRec->nMinMon;
      dateMin.nDay = lpRec->nMinDay;
      dateMax.nYear = lpRec->nMaxYear;
      dateMax.nMonth = lpRec->nMaxMon;
      dateMax.nDay = lpRec->nMaxDay;
      lpDateMin = &dateMin;
      lpDateMax = &dateMax;
    }
    SS_SetTypeDate(lpSS, &cellType, lpRec->lStyle, lpDateFormat,
                   lpDateMin, lpDateMax);
    SS_SetCellTypeRange(lpSS, lCol, lRow, lCol, lRow, &cellType);
  }
  return bRet;
}

//--------------------------------------------------------------------

BOOL SS_Save4TypeDate(LPSPREADSHEET lpSS, SS_COORD lCol, SS_COORD lRow,
                      LPSS_CELLTYPE lpCellType, LPSS_BUFF lpBuff)
{
  SS_REC_TYPEDATE rec;
  BOOL bRet;
  rec.nRecType = SS_RID_TYPEDATE;
  rec.lRecLen = sizeof(rec);
  rec.lCol = lCol;
  rec.lRow = lRow;
  rec.lStyle = lpCellType->Style;
  rec.bSetFormat = lpCellType->Spec.Date.SetFormat;
  rec.bSetRange = lpCellType->Spec.Date.SetRange;
  rec.nFormat = lpCellType->Spec.Date.Format.nFormat;
  rec.nSepChar = lpCellType->Spec.Date.Format.cSeparator;
  rec.bCentury = lpCellType->Spec.Date.Format.bCentury;
  rec.bSpin = lpCellType->Spec.Date.Format.bSpin;
  rec.nMinYear = lpCellType->Spec.Date.Min.nYear;
  rec.nMinMon = lpCellType->Spec.Date.Min.nMonth;
  rec.nMinDay = lpCellType->Spec.Date.Min.nDay;
  rec.nMaxYear = lpCellType->Spec.Date.Max.nYear;
  rec.nMaxMon = lpCellType->Spec.Date.Max.nMonth;
  rec.nMaxDay = lpCellType->Spec.Date.Max.nDay;
  bRet = SS_BuffAppend(lpBuff, &rec, sizeof(rec));
  return bRet;
}

//--------------------------------------------------------------------

BOOL SS_Load4TypeEdit(LPSPREADSHEET lpSS, LPSS_REC_COMMON lpCommon,
                      LPSS_LOADBLOCK lpLoadBlock)
{
  LPSS_REC_TYPEEDIT lpRec = (LPSS_REC_TYPEEDIT)lpCommon;
  SS_CELLTYPE cellType;
  SS_COORD lCol, lRow;
  BOOL bRet = sizeof(*lpRec) == lpRec->lRecLen;
  if( bRet &&
      BlockTranslate(lpLoadBlock, lpRec->lCol, lpRec->lRow, &lCol, &lRow) )
  {
    SS_SetTypeEdit(&cellType, lpRec->lStyle, (short)lpRec->lLen,
                   lpRec->nChrSet, (TCHAR)lpRec->nChrCase);
    SS_SetCellTypeRange(lpSS, lCol, lRow, lCol, lRow, &cellType);
  }
  return bRet;
}

//--------------------------------------------------------------------

BOOL SS_Save4TypeEdit(LPSPREADSHEET lpSS, SS_COORD lCol, SS_COORD lRow,
                      LPSS_CELLTYPE lpCellType, LPSS_BUFF lpBuff)
{
  SS_REC_TYPEEDIT rec;
  BOOL bRet;
  rec.nRecType = SS_RID_TYPEEDIT;
  rec.lRecLen = sizeof(rec);
  rec.lCol = lCol;
  rec.lRow = lRow;
  rec.lStyle = lpCellType->Style;
  rec.lLen = lpCellType->Spec.Edit.Len;
  rec.nChrSet = lpCellType->Spec.Edit.ChrSet;
  rec.nChrCase = lpCellType->Spec.Edit.ChrCase;
  bRet = SS_BuffAppend(lpBuff, &rec, sizeof(rec));
  return bRet;
}

//--------------------------------------------------------------------

BOOL SS_Load4TypeFloat(LPSPREADSHEET lpSS, LPSS_REC_COMMON lpCommon,
                       LPSS_LOADBLOCK lpLoadBlock)
{
  LPSS_REC_TYPEFLOAT lpRec = (LPSS_REC_TYPEFLOAT)lpCommon;
  SS_CELLTYPE cellType;
  FLOATFORMAT floatFormat;
  LPFLOATFORMAT lpFloatFormat = NULL;
  SS_COORD lCol, lRow;
  BOOL bRet = sizeof(*lpRec) == lpRec->lRecLen;
  if( bRet &&
      BlockTranslate(lpLoadBlock, lpRec->lCol, lpRec->lRow, &lCol, &lRow) )
  {
    if( lpRec->bSetFormat )
    {
      floatFormat.cCurrencySign = (TCHAR)lpRec->nCurrChar;
      floatFormat.cDecimalSign = (TCHAR)lpRec->nDecChar;
      floatFormat.cSeparator = (TCHAR)lpRec->nSepChar;
      lpFloatFormat = &floatFormat;
    }
    SS_SetTypeFloatExt(lpSS, &cellType, lpRec->lStyle, lpRec->nLeft,
                       lpRec->nRight, lpRec->dfMin, lpRec->dfMax,
                       lpFloatFormat);
    SS_SetCellTypeRange(lpSS, lCol, lRow, lCol, lRow, &cellType);
  }
  return bRet;
}

//--------------------------------------------------------------------

BOOL SS_Save4TypeFloat(LPSPREADSHEET lpSS, SS_COORD lCol, SS_COORD lRow,
                       LPSS_CELLTYPE lpCellType, LPSS_BUFF lpBuff)
{
  SS_REC_TYPEFLOAT rec;
  BOOL bRet;
  rec.nRecType = SS_RID_TYPEFLOAT;
  rec.lRecLen = sizeof(rec);
  rec.lCol = lCol;
  rec.lRow = lRow;
  rec.lStyle = lpCellType->Style;
  rec.nLeft = lpCellType->Spec.Float.Left;
  rec.nRight = lpCellType->Spec.Float.Right;
  rec.dfMin = lpCellType->Spec.Float.Min;
  rec.dfMax = lpCellType->Spec.Float.Max;
  rec.bSetFormat = lpCellType->Spec.Float.fSetFormat;
  rec.nCurrChar = lpCellType->Spec.Float.Format.cCurrencySign;
  rec.nSepChar = lpCellType->Spec.Float.Format.cSeparator;
  rec.nDecChar = lpCellType->Spec.Float.Format.cDecimalSign;
  bRet = SS_BuffAppend(lpBuff, &rec, sizeof(rec));
  return bRet;
}

//--------------------------------------------------------------------

BOOL SS_Load4TypeInteger(LPSPREADSHEET lpSS, LPSS_REC_COMMON lpCommon,
                         LPSS_LOADBLOCK lpLoadBlock)
{
  LPSS_REC_TYPEINTEGER lpRec = (LPSS_REC_TYPEINTEGER)lpCommon;
  SS_CELLTYPE cellType;
  SS_COORD lCol, lRow;
  BOOL bRet = sizeof(*lpRec) == lpRec->lRecLen;
  if( bRet &&
      BlockTranslate(lpLoadBlock, lpRec->lCol, lpRec->lRow, &lCol, &lRow) )
  {
    #ifndef SS_NOCT_INT
    SS_SetTypeIntegerExt(lpSS, &cellType, lpRec->lStyle, lpRec->lMin,
                         lpRec->lMax, lpRec->bSpinWrap, lpRec->lSpinInc);
    SS_SetCellTypeRange(lpSS, lCol, lRow, lCol, lRow, &cellType);
    #endif
  }
  return bRet;
}

//--------------------------------------------------------------------

BOOL SS_Save4TypeInteger(LPSPREADSHEET lpSS, SS_COORD lCol, SS_COORD lRow,
                         LPSS_CELLTYPE lpCellType, LPSS_BUFF lpBuff)
{
  SS_REC_TYPEINTEGER rec;
  BOOL bRet;
  rec.nRecType = SS_RID_TYPEINTEGER;
  rec.lRecLen = sizeof(rec);
  rec.lCol = lCol;
  rec.lRow = lRow;
  rec.lStyle = lpCellType->Style;
  rec.lMin = lpCellType->Spec.Integer.Min;
  rec.lMax = lpCellType->Spec.Integer.Max;
  rec.lSpinInc = lpCellType->Spec.Integer.lSpinInc;
  rec.bSpinWrap = lpCellType->Spec.Integer.fSpinWrap;
  bRet = SS_BuffAppend(lpBuff, &rec, sizeof(rec));
  return bRet;
}

//--------------------------------------------------------------------

BOOL SS_Load4TypeOwnerDraw(LPSPREADSHEET lpSS, LPSS_REC_COMMON lpCommon,
                           LPSS_LOADBLOCK lpLoadBlock)
{
  LPSS_REC_TYPEOWNERDRAW lpRec = (LPSS_REC_TYPEOWNERDRAW)lpCommon;
  SS_CELLTYPE cellType;
  SS_COORD lCol, lRow;
  BOOL bRet = sizeof(*lpRec) == lpRec->lRecLen;
  if( bRet &&
      BlockTranslate(lpLoadBlock, lpRec->lCol, lpRec->lRow, &lCol, &lRow) )
  {
    #ifndef SS_NOCT_OWNERDRAW
    SS_SetTypeOwnerDraw(lpSS, &cellType, lpRec->lStyle);
    SS_SetCellTypeRange(lpSS, lCol, lRow, lCol, lRow, &cellType);
    #endif
  }
  return bRet;
}

//--------------------------------------------------------------------

BOOL SS_Save4TypeOwnerDraw(LPSPREADSHEET lpSS, SS_COORD lCol, SS_COORD lRow,
                           LPSS_CELLTYPE lpCellType, LPSS_BUFF lpBuff)
{
  SS_REC_TYPEOWNERDRAW rec;
  BOOL bRet;
  rec.nRecType = SS_RID_TYPEOWNERDRAW;
  rec.lRecLen = sizeof(rec);
  rec.lCol = lCol;
  rec.lRow = lRow;
  rec.lStyle = lpCellType->Style;
  bRet = SS_BuffAppend(lpBuff, &rec, sizeof(rec));
  return bRet;
}

//--------------------------------------------------------------------

BOOL SS_Load4TypePic(LPSPREADSHEET lpSS, LPSS_REC_COMMON lpCommon,
                     BOOL bUnicode, LPSS_LOADBLOCK lpLoadBlock)
{
  LPSS_REC_TYPEPIC lpRec = (LPSS_REC_TYPEPIC)lpCommon;
  SS_CELLTYPE cellType;
  HPBYTE lpMask = NULL;
  LPTSTR lpszMask = NULL;
  SS_COORD lCol, lRow;
  BOOL bRet = (long)sizeof(*lpRec) + lpRec->lMaskLen == lpRec->lRecLen;
  if( bRet &&
      BlockTranslate(lpLoadBlock, lpRec->lCol, lpRec->lRow, &lCol, &lRow) )
  {
    SS_BUFF buff;
    SS_BuffInit(&buff);
    lpMask = (BYTE HUGE*)lpRec + sizeof(*lpRec);
    if( lpRec->lMaskLen )
      lpszMask = SS_BuffCopyStr(&buff, lpMask, bUnicode);
    SS_SetTypePic(lpSS, &cellType, lpRec->lStyle, lpszMask);
    SS_SetCellTypeRange(lpSS, lCol, lRow, lCol, lRow, &cellType);
    SS_BuffFree(&buff);
  }
  return bRet;
}

//--------------------------------------------------------------------

BOOL SS_Save4TypePic(LPSPREADSHEET lpSS, SS_COORD lCol, SS_COORD lRow,
                     LPSS_CELLTYPE lpCellType, LPSS_BUFF lpBuff)
{
  SS_REC_TYPEPIC rec;
  LPTSTR lpszMask;
  BOOL bRet;
  rec.nRecType = SS_RID_TYPEPIC;
  rec.lRecLen = sizeof(rec);
  rec.lCol = lCol;
  rec.lRow = lRow;
  rec.lStyle = lpCellType->Style;
  rec.lMaskLen = 0;
  if( lpCellType->Spec.Pic.hMask )
  {
    lpszMask = (LPTSTR)tbGlobalLock(lpCellType->Spec.Pic.hMask);
    rec.lMaskLen = (lstrlen(lpszMask) + 1) * sizeof(TCHAR);
    rec.lRecLen += rec.lMaskLen;
  }
  bRet = SS_BuffAppend(lpBuff, &rec, sizeof(rec));
  if( lpCellType->Spec.Pic.hMask )
  {
    bRet &= SS_BuffAppend(lpBuff, lpszMask, rec.lMaskLen);
    tbGlobalUnlock(lpCellType->Spec.Pic.hMask);
  }
  return bRet;
}

//--------------------------------------------------------------------

BOOL SS_Load4TypePicture(LPSPREADSHEET lpSS, LPSS_REC_COMMON lpCommon,
                         BOOL bUnicode, LPSS_LOADBLOCK lpLoadBlock)
{
  LPSS_REC_TYPEPICTURE lpRec = (LPSS_REC_TYPEPICTURE)lpCommon;
  HPBYTE lpPict = (HPBYTE)lpRec + sizeof(*lpRec);
  SS_CELLTYPE cellType;
  LPTSTR lpszText = NULL;
  HANDLE hPict = 0;
  SS_COORD lCol, lRow;
  #if defined(SS_OCX) || defined(SS_VB)
  BOOL bDelete = FALSE;
  #else
  BOOL bDelete = TRUE;
  #endif
  BOOL bRet = (long)sizeof(*lpRec) + lpRec->lPictLen == lpRec->lRecLen;
  if( bRet &&
      BlockTranslate(lpLoadBlock, lpRec->lCol, lpRec->lRow, &lCol, &lRow) )
  {
    if( lpRec->lStyle & VPS_HANDLE )
    {
      if( lpRec->lPictLen )
      {
        if( lpRec->lStyle & VPS_BMP )
          hPict = SS_Load4Bitmap(lpPict, lpRec->lPictLen);
        else if( lpRec->lStyle & VPS_ICON )
          hPict = SS_Load4Icon(lpPict, lpRec->lPictLen);
      }
      SS_SetTypePictureHandle(lpSS, &cellType, lpRec->lStyle,
                              hPict, NULL, bDelete);
      #if defined(SS_VB)
      cellType.Spec.ViewPict.hPic =
        CreateVbPict((HANDLE)cellType.Spec.ViewPict.hPictName,
                     (cellType.Style & VPS_BMP) == VPS_BMP,
                     (cellType.Style & VPS_ICON) == VPS_ICON);
      #elif defined(SS_OCX)
      cellType.Spec.ViewPict.lpPict =
        CreateOcxPict((HANDLE)cellType.Spec.ViewPict.hPictName,
                      (cellType.Style & VPS_BMP) == VPS_BMP,
                      (cellType.Style & VPS_ICON) == VPS_ICON);
      #endif 
      SS_SetCellTypeRange(lpSS, lCol, lRow, lCol, lRow, &cellType);
    }
    else
    {
      SS_BUFF buff;
      SS_BuffInit(&buff);
      if( lpRec->lPictLen )
        lpszText = SS_BuffCopyStr(&buff, lpPict, bUnicode);
      SS_SetTypePicture(lpSS, &cellType, lpRec->lStyle, lpszText);
      #if defined(SS_VB)
      cellType.Spec.ViewPict.hPic =
        CreateVbPict((HANDLE)cellType.Spec.ViewPict.hPictName,
                     (cellType.Style & VPS_BMP) == VPS_BMP,
                     (cellType.Style & VPS_ICON) == VPS_ICON);
      #elif defined(SS_OCX)
      cellType.Spec.ViewPict.lpPict =
        CreateOcxPict((HANDLE)cellType.Spec.ViewPict.hPictName,
                      (cellType.Style & VPS_BMP) == VPS_BMP,
                      (cellType.Style & VPS_ICON) == VPS_ICON);
      #endif 
      SS_SetCellTypeRange(lpSS, lCol, lRow, lCol, lRow, &cellType);
      SS_BuffFree(&buff);
    }
  }
  return bRet;
}

//--------------------------------------------------------------------

BOOL SS_Save4TypePicture(LPSPREADSHEET lpSS, SS_COORD lCol, SS_COORD lRow,
                         LPSS_CELLTYPE lpCellType, LPSS_BUFF lpBuff)
{
  SS_REC_TYPEPICTURE rec;
  LPTSTR lpszText;
  long lPictLen = 0;
  long lBuffLenOld = lpBuff->lLen;
  BOOL bRet;

  rec.nRecType = SS_RID_TYPEPICTURE;
  rec.lRecLen = sizeof(rec);
  rec.lCol = lCol;
  rec.lRow = lRow;
  rec.lStyle = lpCellType->Style;
  rec.lPictLen = 0;
  #ifndef WIN32
  if( (rec.lStyle & VPS_HANDLE) && (rec.lStyle & VPS_ICON) )
  {
    rec.lStyle &= ~VPS_ICON;
    rec.lStyle |= VPS_BMP;
  }
  #endif
  bRet = SS_BuffAppend(lpBuff, &rec, sizeof(rec));
  if( lpCellType->Spec.ViewPict.hPictName )
  {
    if( lpCellType->Style & VPS_HANDLE )
    {
      if( lpCellType->Style & VPS_BMP )
        bRet &= SS_Save4Bitmap((HBITMAP)lpCellType->Spec.ViewPict.hPictName, lpBuff, &lPictLen);
      else if( lpCellType->Style & VPS_ICON )
        bRet &= SS_Save4Icon((HICON)lpCellType->Spec.ViewPict.hPictName, lpBuff, &lPictLen);
    }
    else
    {
      lpszText = (LPTSTR)tbGlobalLock(lpCellType->Spec.ViewPict.hPictName);
      lPictLen = (lstrlen(lpszText) + 1) * sizeof(TCHAR);
      bRet &= SS_BuffAppend(lpBuff, lpszText, lPictLen);
      tbGlobalUnlock(lpCellType->Spec.ViewPict.hPictName);
    }
    if( bRet && lpBuff->pMem )
      MemHugeCpy(&rec, lpBuff->pMem + lBuffLenOld, sizeof(rec));
    rec.lRecLen = lpBuff->lLen - lBuffLenOld;
    rec.lPictLen = lPictLen;
    if( bRet && lpBuff->pMem )
      MemHugeCpy(lpBuff->pMem + lBuffLenOld, &rec, sizeof(rec));
  }
  return bRet;
}

//--------------------------------------------------------------------

BOOL SS_Load4TypeStatic(LPSPREADSHEET lpSS, LPSS_REC_COMMON lpCommon,
                        LPSS_LOADBLOCK lpLoadBlock)
{
  LPSS_REC_TYPESTATIC lpRec = (LPSS_REC_TYPESTATIC)lpCommon;
  SS_CELLTYPE cellType;
  SS_COORD lCol, lRow;
  BOOL bRet = sizeof(*lpRec) == lpRec->lRecLen;
  if( bRet &&
      BlockTranslate(lpLoadBlock, lpRec->lCol, lpRec->lRow, &lCol, &lRow) )
  {
    SS_SetTypeStaticText(lpSS, &cellType, lpRec->lStyle);
    SS_SetCellTypeRange(lpSS, lCol, lRow, lCol, lRow, &cellType);
  }
  return bRet;
}

//--------------------------------------------------------------------

BOOL SS_Save4TypeStatic(LPSPREADSHEET lpSS, SS_COORD lCol, SS_COORD lRow,
                        LPSS_CELLTYPE lpCellType, LPSS_BUFF lpBuff)
{
  SS_REC_TYPESTATIC rec;
  BOOL bRet;
  rec.nRecType = SS_RID_TYPESTATIC;
  rec.lRecLen = sizeof(rec);
  rec.lCol = lCol;
  rec.lRow = lRow;
  rec.lStyle = lpCellType->Style;
  bRet = SS_BuffAppend(lpBuff, &rec, sizeof(rec));
  return bRet;
}

//--------------------------------------------------------------------

BOOL SS_Load4TypeTime(LPSPREADSHEET lpSS, LPSS_REC_COMMON lpCommon,
                      LPSS_LOADBLOCK lpLoadBlock)
{
  LPSS_REC_TYPETIME lpRec = (LPSS_REC_TYPETIME)lpCommon;
  TIMEFORMAT timeFormat;
  LPTIMEFORMAT lpTimeFormat = NULL;
  TIME timeMin;
  TIME timeMax;
  LPTIME lpTimeMin = NULL;
  LPTIME lpTimeMax = NULL;
  SS_CELLTYPE cellType;
  SS_COORD lCol, lRow;
  BOOL bRet = sizeof(*lpRec) == lpRec->lRecLen;
  if( bRet &&
      BlockTranslate(lpLoadBlock, lpRec->lCol, lpRec->lRow, &lCol, &lRow) )
  {
    if( lpRec->bSetFormat )
    {
      timeFormat.b24Hour = lpRec->b24Hour;
      timeFormat.bSeconds = lpRec->bSeconds;
      timeFormat.cSeparator = (TCHAR)lpRec->nSepChar;
      timeFormat.bSpin = lpRec->bSpin;
      lpTimeFormat = &timeFormat;
    }
    if( lpRec->bSetRange )
    {
      timeMin.nHour = lpRec->nMinHour;
      timeMin.nMinute = lpRec->nMinMin;
      timeMin.nSecond = lpRec->nMinSec;
      timeMax.nHour = lpRec->nMaxHour;
      timeMax.nMinute = lpRec->nMaxMin;
      timeMax.nSecond = lpRec->nMaxSec;
      lpTimeMin = &timeMin;
      lpTimeMax = &timeMax;
    }
    SS_SetTypeTime(lpSS, &cellType, lpRec->lStyle, lpTimeFormat,
                   lpTimeMin, lpTimeMax);
    SS_SetCellTypeRange(lpSS, lCol, lRow, lCol, lRow, &cellType);
  }
  return bRet;
}

//--------------------------------------------------------------------

BOOL SS_Save4TypeTime(LPSPREADSHEET lpSS, SS_COORD lCol, SS_COORD lRow,
                      LPSS_CELLTYPE lpCellType, LPSS_BUFF lpBuff)
{
  SS_REC_TYPETIME rec;
  BOOL bRet;

  rec.nRecType = SS_RID_TYPETIME;
  rec.lRecLen = sizeof(rec);
  rec.lCol = lCol;
  rec.lRow = lRow;
  rec.lStyle = lpCellType->Style;
  rec.bSetFormat = lpCellType->Spec.Time.SetFormat;
  rec.bSetRange = lpCellType->Spec.Time.SetRange;
  rec.b24Hour = lpCellType->Spec.Time.Format.b24Hour;
  rec.bSeconds = lpCellType->Spec.Time.Format.bSeconds;
  rec.nSepChar = lpCellType->Spec.Time.Format.cSeparator;
  rec.bSpin = lpCellType->Spec.Time.Format.bSpin;
  rec.nMinHour = lpCellType->Spec.Time.Min.nHour;
  rec.nMinMin = lpCellType->Spec.Time.Min.nMinute;
  rec.nMinSec = lpCellType->Spec.Time.Min.nSecond;
  rec.nMaxHour = lpCellType->Spec.Time.Max.nHour;
  rec.nMaxMin = lpCellType->Spec.Time.Max.nMinute;
  rec.nMaxSec = lpCellType->Spec.Time.Max.nSecond;
  bRet = SS_BuffAppend(lpBuff, &rec, sizeof(rec));
  return bRet;
}

#ifdef SS_V40
//--------------------------------------------------------------------

BOOL SS_Load4TypeCurrency(LPSPREADSHEET lpSS, LPSS_REC_COMMON lpCommon,
                          LPSS_LOADBLOCK lpLoadBlock, BOOL bUnicode)
{
  LPSS_REC_TYPECURRENCY lpRec = (LPSS_REC_TYPECURRENCY)lpCommon;
  SS_CELLTYPE cellType;
  SS_COORD lCol, lRow;
  BOOL bRet = sizeof(*lpRec) <= lpRec->lRecLen;
  if( bRet &&
      BlockTranslate(lpLoadBlock, lpRec->lCol, lpRec->lRow, &lCol, &lRow) )
  {
    HPBYTE lpNext;
    LPTSTR lpszDec = NULL;
    LPTSTR lpszSep = NULL;
    LPTSTR lpszCur = NULL;
    SS_BUFF buffDec;
    SS_BUFF buffSep;
    SS_BUFF buffCur;

    SS_BuffInit(&buffDec);
    SS_BuffInit(&buffSep);
    SS_BuffInit(&buffCur);

    lpNext = (HPBYTE)lpRec + sizeof(*lpRec);
    if (lpRec->bDecLen)
    {
      lpszDec = SS_BuffCopyStr(&buffDec, lpNext, bUnicode);
      lpNext += lpRec->bDecLen;
    }
    if (lpRec->bSepLen)
    {
      lpszSep = SS_BuffCopyStr(&buffSep, lpNext, bUnicode);
      lpNext += lpRec->bSepLen;
    }
    if (lpRec->bCurLen)
    {
      lpszCur = SS_BuffCopyStr(&buffCur, lpNext, bUnicode);
      lpNext += lpRec->bCurLen;
    }

    SS_SetTypeCurrency(lpSS, &cellType, lpRec->lStyle, lpRec->Right, lpRec->dfMin, lpRec->dfMax,
                       lpRec->fShowSeparator, lpRec->fShowCurrencySymbol, lpszDec, lpszSep,
                       lpszCur, lpRec->bLeadingZero, lpRec->bNegStyle, lpRec->bPosStyle,
                       lpRec->fSpin, lpRec->fSpinWrap, lpRec->SpinInc);
    SS_SetCellTypeRange(lpSS, lCol, lRow, lCol, lRow, &cellType);

    SS_BuffFree(&buffDec);
    SS_BuffFree(&buffSep);
    SS_BuffFree(&buffCur);

  }
  return bRet;
}

//--------------------------------------------------------------------

BOOL SS_Save4TypeCurrency(LPSPREADSHEET lpSS, SS_COORD lCol, SS_COORD lRow,
                       LPSS_CELLTYPE lpCellType, LPSS_BUFF lpBuff)
{
  SS_REC_TYPECURRENCY rec;
  int iDecLen = lstrlen(lpCellType->Spec.Currency.szDecimal);
  int iSepLen = lstrlen(lpCellType->Spec.Currency.szSeparator);
  int iCurLen = lstrlen(lpCellType->Spec.Currency.szCurrency);
  BOOL bRet;

  rec.nRecType = SS_RID_TYPECURRENCY;
  rec.lRecLen = sizeof(rec);
  rec.lCol = lCol;
  rec.lRow = lRow;
  rec.lStyle = lpCellType->Style;

  rec.dfMin = lpCellType->Spec.Currency.Min;
  rec.dfMax = lpCellType->Spec.Currency.Max;
  rec.Right = (BYTE)lpCellType->Spec.Currency.Right;
  rec.bLeadingZero = lpCellType->Spec.Currency.bLeadingZero;
  rec.bNegStyle = lpCellType->Spec.Currency.bNegStyle;
  rec.bPosStyle = lpCellType->Spec.Currency.bPosStyle;
  rec.fShowSeparator = lpCellType->Spec.Currency.fShowSeparator;
  rec.fShowCurrencySymbol = lpCellType->Spec.Currency.fShowCurrencySymbol;
  rec.fSpin = lpCellType->Spec.Currency.fSpin;
  rec.fSpinWrap = lpCellType->Spec.Currency.fSpinWrap;
  rec.SpinInc = lpCellType->Spec.Currency.SpinInc;
  if (iDecLen)
    rec.bDecLen = (UCHAR)((iDecLen + 1) * sizeof(TCHAR));
  else
    rec.bDecLen = 0;
  if (iSepLen)
    rec.bSepLen = (UCHAR)((iSepLen + 1) * sizeof(TCHAR));
  else
    rec.bSepLen = 0;
  if (iCurLen)
    rec.bCurLen = (UCHAR)((iCurLen + 1) * sizeof(TCHAR));
  else
    rec.bCurLen = 0;

  rec.lRecLen += rec.bDecLen + rec.bSepLen + rec.bCurLen;
  bRet = SS_BuffAppend(lpBuff, &rec, sizeof(rec));

  if (iDecLen)
    bRet &= SS_BuffAppend(lpBuff, lpCellType->Spec.Currency.szDecimal, rec.bDecLen);
  if (iSepLen)
    bRet &= SS_BuffAppend(lpBuff, lpCellType->Spec.Currency.szSeparator, rec.bSepLen);
  if (iCurLen)
    bRet &= SS_BuffAppend(lpBuff, lpCellType->Spec.Currency.szCurrency, rec.bCurLen);

  return bRet;
}

//--------------------------------------------------------------------

BOOL SS_Load4TypeNumber(LPSPREADSHEET lpSS, LPSS_REC_COMMON lpCommon,
                          LPSS_LOADBLOCK lpLoadBlock, BOOL bUnicode)
{
  LPSS_REC_TYPENUMBER lpRec = (LPSS_REC_TYPENUMBER)lpCommon;
  SS_CELLTYPE cellType;
  SS_COORD lCol, lRow;
  BOOL bRet = sizeof(*lpRec) <= lpRec->lRecLen;
  if( bRet &&
      BlockTranslate(lpLoadBlock, lpRec->lCol, lpRec->lRow, &lCol, &lRow) )
  {
    HPBYTE lpNext;
    LPTSTR lpszDec = NULL;
    LPTSTR lpszSep = NULL;
    SS_BUFF buffDec;
    SS_BUFF buffSep;

    SS_BuffInit(&buffDec);
    SS_BuffInit(&buffSep);

    lpNext = (HPBYTE)lpRec + sizeof(*lpRec);
    if (lpRec->bDecLen)
    {
      lpszDec = SS_BuffCopyStr(&buffDec, lpNext, bUnicode);
      lpNext += lpRec->bDecLen;
    }
    if (lpRec->bSepLen)
    {
      lpszSep = SS_BuffCopyStr(&buffSep, lpNext, bUnicode);
      lpNext += lpRec->bSepLen;
    }

    SS_SetTypeNumber(lpSS, &cellType, lpRec->lStyle, lpRec->Right, lpRec->dfMin, lpRec->dfMax,
                       lpRec->fShowSeparator, lpszDec, lpszSep, lpRec->bLeadingZero, lpRec->bNegStyle,
                       lpRec->fSpin, lpRec->fSpinWrap, lpRec->SpinInc);
    SS_SetCellTypeRange(lpSS, lCol, lRow, lCol, lRow, &cellType);

    SS_BuffFree(&buffDec);
    SS_BuffFree(&buffSep);
  }
  return bRet;
}

//--------------------------------------------------------------------

BOOL SS_Save4TypeNumber(LPSPREADSHEET lpSS, SS_COORD lCol, SS_COORD lRow,
                       LPSS_CELLTYPE lpCellType, LPSS_BUFF lpBuff)
{
  SS_REC_TYPENUMBER rec;
  int iDecLen = lstrlen(lpCellType->Spec.Number.szDecimal);
  int iSepLen = lstrlen(lpCellType->Spec.Number.szSeparator);
  BOOL bRet;

  rec.nRecType = SS_RID_TYPENUMBER;
  rec.lRecLen = sizeof(rec);
  rec.lCol = lCol;
  rec.lRow = lRow;
  rec.lStyle = lpCellType->Style;

  rec.dfMin = lpCellType->Spec.Number.Min;
  rec.dfMax = lpCellType->Spec.Number.Max;
  rec.Right = (BYTE)lpCellType->Spec.Number.Right;
  rec.bLeadingZero = lpCellType->Spec.Number.bLeadingZero;
  rec.bNegStyle = lpCellType->Spec.Number.bNegStyle;
  rec.fShowSeparator = lpCellType->Spec.Number.fShowSeparator;
  rec.fSpin = lpCellType->Spec.Number.fSpin;
  rec.fSpinWrap = lpCellType->Spec.Number.fSpinWrap;
  rec.SpinInc = lpCellType->Spec.Number.SpinInc;
  if (iDecLen)
    rec.bDecLen = (UCHAR)((iDecLen + 1) * sizeof(TCHAR));
  else
    rec.bDecLen = 0;
  if (iSepLen)
    rec.bSepLen = (UCHAR)((iSepLen + 1) * sizeof(TCHAR));
  else
    rec.bSepLen = 0;

  rec.lRecLen += rec.bDecLen + rec.bSepLen;
  bRet = SS_BuffAppend(lpBuff, &rec, sizeof(rec));

  if (iDecLen)
    bRet &= SS_BuffAppend(lpBuff, lpCellType->Spec.Number.szDecimal, rec.bDecLen);
  if (iSepLen)
    bRet &= SS_BuffAppend(lpBuff, lpCellType->Spec.Number.szSeparator, rec.bSepLen);
 
  return bRet;
}

//--------------------------------------------------------------------

BOOL SS_Load4TypePercent(LPSPREADSHEET lpSS, LPSS_REC_COMMON lpCommon,
                          LPSS_LOADBLOCK lpLoadBlock, BOOL bUnicode)
{
  LPSS_REC_TYPEPERCENT lpRec = (LPSS_REC_TYPEPERCENT)lpCommon;
  SS_CELLTYPE cellType;
  SS_COORD lCol, lRow;
  BOOL bRet = sizeof(*lpRec) <= lpRec->lRecLen;
  if( bRet &&
      BlockTranslate(lpLoadBlock, lpRec->lCol, lpRec->lRow, &lCol, &lRow) )
  {
    HPBYTE lpNext;
    LPTSTR lpszDec = NULL;
    SS_BUFF buffDec;

    SS_BuffInit(&buffDec);

    lpNext = (HPBYTE)lpRec + sizeof(*lpRec);
    if (lpRec->bDecLen)
    {
      lpszDec = SS_BuffCopyStr(&buffDec, lpNext, bUnicode);
      lpNext += lpRec->bDecLen;
    }

    SS_SetTypePercent(lpSS, &cellType, lpRec->lStyle, lpRec->Right, lpRec->dfMin, lpRec->dfMax,
                      lpszDec, lpRec->bLeadingZero, lpRec->bNegStyle, lpRec->fSpin, lpRec->fSpinWrap,
                      lpRec->SpinInc);
    SS_SetCellTypeRange(lpSS, lCol, lRow, lCol, lRow, &cellType);

    SS_BuffFree(&buffDec);
  }
  return bRet;
}

//--------------------------------------------------------------------

BOOL SS_Save4TypePercent(LPSPREADSHEET lpSS, SS_COORD lCol, SS_COORD lRow,
                       LPSS_CELLTYPE lpCellType, LPSS_BUFF lpBuff)
{
  SS_REC_TYPEPERCENT rec;
  int iDecLen = lstrlen(lpCellType->Spec.Percent.szDecimal);
  BOOL bRet;

  rec.nRecType = SS_RID_TYPEPERCENT;
  rec.lRecLen = sizeof(rec);
  rec.lCol = lCol;
  rec.lRow = lRow;
  rec.lStyle = lpCellType->Style;

  rec.dfMin = lpCellType->Spec.Percent.Min;
  rec.dfMax = lpCellType->Spec.Percent.Max;
  rec.Right = (BYTE)lpCellType->Spec.Percent.Right;
  rec.bLeadingZero = lpCellType->Spec.Percent.bLeadingZero;
  rec.bNegStyle = lpCellType->Spec.Percent.bNegStyle;
  rec.fSpin = lpCellType->Spec.Percent.fSpin;
  rec.fSpinWrap = lpCellType->Spec.Percent.fSpinWrap;
  rec.SpinInc = lpCellType->Spec.Percent.SpinInc;
  if (iDecLen)
    rec.bDecLen = (UCHAR)((iDecLen + 1) * sizeof(TCHAR));
  else
    rec.bDecLen = 0;

  rec.lRecLen += rec.bDecLen;
  bRet = SS_BuffAppend(lpBuff, &rec, sizeof(rec));
 
  if (iDecLen)
    bRet &= SS_BuffAppend(lpBuff, lpCellType->Spec.Percent.szDecimal, rec.bDecLen);
 
  return bRet;
}

#endif // SS_V40

//--------------------------------------------------------------------

#ifdef SS_V70
BOOL SS_Load4TypeScientific(LPSPREADSHEET lpSS, LPSS_REC_COMMON lpCommon,
                          LPSS_LOADBLOCK lpLoadBlock, BOOL bUnicode)
{
  LPSS_REC_TYPESCIENTIFIC lpRec = (LPSS_REC_TYPESCIENTIFIC)lpCommon;
  SS_CELLTYPE cellType;
  SS_COORD lCol, lRow;
  BOOL bRet = sizeof(*lpRec) <= lpRec->lRecLen;
  if( bRet &&
      BlockTranslate(lpLoadBlock, lpRec->lCol, lpRec->lRow, &lCol, &lRow) )
  {
    HPBYTE lpNext;
    LPTSTR lpszDec = NULL;
    SS_BUFF buffDec;

    SS_BuffInit(&buffDec);

    lpNext = (HPBYTE)lpRec + sizeof(*lpRec);
    if (lpRec->bDecLen)
    {
      lpszDec = SS_BuffCopyStr(&buffDec, lpNext, bUnicode);
      lpNext += lpRec->bDecLen;
    }

    SS_SetTypeScientific(lpSS, &cellType, lpRec->lStyle, lpRec->Right, lpRec->dfMin, lpRec->dfMax,
                         lpszDec);
    SS_SetCellTypeRange(lpSS, lCol, lRow, lCol, lRow, &cellType);

    SS_BuffFree(&buffDec);
  }
  return bRet;
}

//--------------------------------------------------------------------

BOOL SS_Save4TypeScientific(LPSPREADSHEET lpSS, SS_COORD lCol, SS_COORD lRow,
                       LPSS_CELLTYPE lpCellType, LPSS_BUFF lpBuff)
{
  SS_REC_TYPESCIENTIFIC rec;
  int iDecLen = lstrlen(lpCellType->Spec.Scientific.szDecimal);
  BOOL bRet;

  rec.nRecType = SS_RID_TYPESCIENTIFIC;
  rec.lRecLen = sizeof(rec);
  rec.lCol = lCol;
  rec.lRow = lRow;
  rec.lStyle = lpCellType->Style;

  rec.dfMin = lpCellType->Spec.Scientific.Min;
  rec.dfMax = lpCellType->Spec.Scientific.Max;
  rec.Right = (BYTE)lpCellType->Spec.Scientific.Right;
  if (iDecLen)
    rec.bDecLen = (UCHAR)((iDecLen + 1) * sizeof(TCHAR));
  else
    rec.bDecLen = 0;

  rec.lRecLen += rec.bDecLen;
  bRet = SS_BuffAppend(lpBuff, &rec, sizeof(rec));

  if (iDecLen)
    bRet &= SS_BuffAppend(lpBuff, lpCellType->Spec.Scientific.szDecimal, rec.bDecLen);
 
  return bRet;
}
#endif // SS_V70

#ifdef SS_V80

void SS_BuffCopyTypeCustomItemData(long lItemDataType, LPSS_CT_VALUE ItemData, HPBYTE lpItemData, BOOL bUnicode)
{
   HPBYTE lpItemDataValue;

   ItemData->type = lItemDataType;
   switch( lItemDataType )
   {
   case SS_CT_VALUE_TYPE_LONG:
      ItemData->u.lValue = *(long*)lpItemData;
      break;
   case SS_CT_VALUE_TYPE_DOUBLE:
      ItemData->u.dblValue = *(double*)lpItemData;
      break;
   case SS_CT_VALUE_TYPE_TSTR:
      {
       SS_BUFF buff;
       long len = *(long*)lpItemData;

       SS_BuffInit(&buff);
       lpItemDataValue = lpItemData + sizeof(long);
       if( len )
       {
         LPTSTR lpszValue = SS_BuffCopyStr(&buff, lpItemDataValue, bUnicode);
         LPTSTR lpszItemData;

         ItemData->u.hszValue = GlobalAlloc(GHND, len + sizeof(TCHAR));
         if( lpszItemData = GlobalLock(ItemData->u.hszValue) )
         {
            lstrcpy(lpszItemData, lpszValue);
            GlobalUnlock(ItemData->u.hszValue);
         }
       }
      }
      break;
   case SS_CT_VALUE_TYPE_BUFFER:
      {
         long len = *(long*)lpItemData;

         lpItemDataValue = lpItemData + sizeof(long);
         if( len )
         {
            LPVOID lpItemData;

            ItemData->u.Buffer.hBuffer = GlobalAlloc(GHND, len);
            if( lpItemData = GlobalLock(ItemData->u.Buffer.hBuffer) )
            {
               MemHugeCpy(lpItemData, lpItemDataValue, len);
               GlobalUnlock(ItemData->u.Buffer.hBuffer);
               ItemData->u.Buffer.lSize = len;
            }
         }
      }
      break;
   }
}

long SS_TypeCustomItemDataSize(LPSS_CT_VALUE ItemData)
{
  LPTSTR lpszText;
  long lTextLen;
  long lRet = 0;
  switch( ItemData->type )
  {
    case SS_CT_VALUE_TYPE_LONG:
      lRet = sizeof(long);
      break;
    case SS_CT_VALUE_TYPE_DOUBLE:
      lRet = sizeof(double);
      break;
    case SS_CT_VALUE_TYPE_TSTR:
      if( ItemData->u.hszValue )
      {
        lpszText = (LPTSTR)GlobalLock(ItemData->u.hszValue);
        lTextLen = lstrlen(lpszText) + 1;
        GlobalUnlock(ItemData->u.hszValue);
      }
      lRet = sizeof(long) + lTextLen * sizeof(TCHAR);
      break;
    case SS_CT_VALUE_TYPE_BUFFER:
      lRet = sizeof(long) + ItemData->u.Buffer.lSize;
      break;
  }
  return lRet;
}

BOOL SS_Save4TypeCustomItemData(LPSS_CT_VALUE ItemData, LPSS_BUFF lpBuff)
{
  LPTSTR lpszText;
  long lTextLen;
  BOOL bRet = TRUE;
  switch( ItemData->type )
  {
    case SS_CT_VALUE_TYPE_LONG:
      bRet = SS_BuffAppend(lpBuff, &ItemData->u.lValue, sizeof(long));
      break;
    case SS_CT_VALUE_TYPE_DOUBLE:
      bRet = SS_BuffAppend(lpBuff, &ItemData->u.dblValue, sizeof(double));
      break;
    case SS_CT_VALUE_TYPE_TSTR:
      if( ItemData->u.hszValue )
      {
        lpszText = (LPTSTR)GlobalLock(ItemData->u.hszValue);
        lTextLen = lstrlen(lpszText) + 1;
        bRet = SS_BuffAppend(lpBuff, &lTextLen, sizeof(long));
        bRet &= SS_BuffAppend(lpBuff, lpszText, lTextLen * sizeof(TCHAR));
        GlobalUnlock(ItemData->u.hszValue);
      }
      break;
    case SS_CT_VALUE_TYPE_BUFFER:
       if( ItemData->u.Buffer.hBuffer && ItemData->u.Buffer.lSize > 0 )
       {
          LPVOID lpvData = (LPVOID)GlobalLock(ItemData->u.Buffer.hBuffer);
          bRet = SS_BuffAppend(lpBuff, &ItemData->u.Buffer.lSize, sizeof(long));
          bRet &= SS_BuffAppend(lpBuff, lpvData, ItemData->u.Buffer.lSize);
          GlobalUnlock(ItemData->u.Buffer.hBuffer);
       }
       break;
  }
  return bRet;
}

BOOL SS_Load4TypeCustom(LPSPREADSHEET lpSS, LPSS_REC_COMMON lpCommon,
                     BOOL bUnicode, LPSS_LOADBLOCK lpLoadBlock)
{
  LPSS_REC_TYPECUSTOM lpRec = (LPSS_REC_TYPECUSTOM)lpCommon;
  SS_CELLTYPE cellType;
  HPBYTE lpName = NULL;
  LPTSTR lpszName = NULL;
  SS_COORD lCol, lRow;
  SS_CT_VALUE ItemData = {0};
  HPBYTE lpItemData = NULL;
  BOOL bRet = (long)sizeof(*lpRec) + lpRec->lNameLen + lpRec->lItemDataLen == lpRec->lRecLen;
  if( bRet &&
      BlockTranslate(lpLoadBlock, lpRec->lCol, lpRec->lRow, &lCol, &lRow) )
  {
    SS_BUFF buff;
    SS_BuffInit(&buff);
    lpName = (BYTE HUGE*)lpRec + sizeof(*lpRec);
    if( lpRec->lNameLen )
      lpszName = SS_BuffCopyStr(&buff, lpName, bUnicode);
    lpItemData = lpName + lpRec->lNameLen * sizeof(TCHAR);
    if( lpRec->lItemDataType )
      SS_BuffCopyTypeCustomItemData(lpRec->lItemDataType, &ItemData, lpItemData, bUnicode);
    SS_SetTypeCustom(lpSS, &cellType, lpszName, lpRec->lStyle, &ItemData);
    SS_SetCellTypeRange(lpSS, lCol, lRow, lCol, lRow, &cellType);
    SS_BuffFree(&buff);
  }
  return bRet;
}

//--------------------------------------------------------------------

BOOL SS_Save4TypeCustom(LPSPREADSHEET lpSS, SS_COORD lCol, SS_COORD lRow,
                       LPSS_CELLTYPE lpCellType, LPSS_BUFF lpBuff)
{
  SS_REC_TYPECUSTOM rec;
  LPTSTR lpszName;
  BOOL bRet;
  long ItemDataType = lpCellType->Spec.Custom.ItemData.type;
  rec.nRecType = SS_RID_TYPECUSTOM;
  rec.lCol = lCol;
  rec.lRow = lRow;
  rec.lStyle = lpCellType->Style;
  rec.lNameLen = 0;
  rec.lItemDataType = ItemDataType;
  rec.lItemDataLen = SS_TypeCustomItemDataSize(&lpCellType->Spec.Custom.ItemData);
  rec.lRecLen = sizeof(rec) + rec.lItemDataLen;
  if( lpCellType->Spec.Custom.hName )
  {
    lpszName = SS_CT_LockName(lpCellType);
    rec.lNameLen = (lstrlen(lpszName) + 1) * sizeof(TCHAR);
    rec.lRecLen += rec.lNameLen;
  }
  bRet = SS_BuffAppend(lpBuff, &rec, sizeof(rec));
  if( lpCellType->Spec.Custom.hName )
  {
    bRet &= SS_BuffAppend(lpBuff, lpszName, rec.lNameLen);
    SS_CT_UnlockName(lpCellType);
  }
  if( ItemDataType )
     bRet &= SS_Save4TypeCustomItemData(&lpCellType->Spec.Custom.ItemData, lpBuff);
  return bRet;
}

#endif

#ifdef SS_V35
//--------------------------------------------------------------------

BOOL SS_Load4CellNote(LPSPREADSHEET lpSS, LPSS_REC_COMMON lpCommon,
                      BOOL bUnicode, LPSS_LOADBLOCK lpLoadBlock)
{
  LPSS_REC_CELLNOTE lpRec = (LPSS_REC_CELLNOTE)lpCommon;
  HPBYTE lpText = NULL;
  LPTSTR lpszText = NULL;
  SS_COORD lCol, lRow;
  BOOL bRet = sizeof(*lpRec) <= lpRec->lRecLen;
  if( bRet &&
      BlockTranslate(lpLoadBlock, lpRec->lCol, lpRec->lRow, &lCol, &lRow) )
  {
    SS_BUFF buff;
    SS_BuffInit(&buff);
    lpText = (HPBYTE)lpRec + sizeof(*lpRec);
    if( sizeof(*lpRec) <= lpRec->lRecLen )
      lpszText = SS_BuffCopyStr(&buff, lpText, bUnicode);
    SS_SetCellCellNote(lpSS, lCol, lRow, lpszText);

    SS_BuffFree(&buff);
  }
  return bRet;
}

//--------------------------------------------------------------------

BOOL SS_Save4CellNote(SS_COORD lCol, SS_COORD lRow, TBGLOBALHANDLE ghNote,
                      LPSS_BUFF lpBuff)
{
  SS_REC_CELLNOTE rec;
  LPTSTR lpszText;
  long lTextLen;
  BOOL bRet = TRUE;
  if( ghNote )
      {
        lpszText = (LPTSTR)tbGlobalLock(ghNote);
        lTextLen = lstrlen(lpszText) + 1;
        rec.nRecType = SS_RID_CELLNOTE;
        rec.lRecLen = sizeof(rec) + (lTextLen * sizeof(TCHAR));
        rec.lCol = lCol;
        rec.lRow = lRow;
        bRet = SS_BuffAppend(lpBuff, &rec, sizeof(rec));
        bRet &= SS_BuffAppend(lpBuff, lpszText, lTextLen * sizeof(TCHAR));
        tbGlobalUnlock(ghNote);
      }
  return bRet;
}

//--------------------------------------------------------------------
//
//  The SS_CellNoteIndicator()
//

BOOL SS_Load4CellNoteIndicator(LPSS_BOOK lpBook, LPSS_REC_COMMON lpCommon)
{
  LPSS_REC_CELLNOTEINDICATOR lpRec = (LPSS_REC_CELLNOTEINDICATOR)lpCommon;
  BOOL bRet = sizeof(*lpRec) == lpRec->lRecLen;
  if( bRet )
  {
    lpBook->nCellNoteIndicator = lpRec->nCellNoteIndicator;
  }
  return bRet;
}

#ifdef SS_V80
//--------------------------------------------------------------------
//
//  The SS_Load4PrintDevMode()
//

BOOL SS_Load4PrintDevMode(LPSS_BOOK lpBook, LPSS_REC_COMMON lpCommon)
{
  LPSS_REC_PRINTDEVMODE lpRec = (LPSS_REC_PRINTDEVMODE)lpCommon;
  BOOL bRet = sizeof(*lpRec) == lpRec->lRecLen;
  if( bRet )
  {
	lpBook->m_PrinterDevMode = lpRec->PrintDevMode;
  }
// fix for 99914272/99916363 -scl
  else
  {
    int a32 = sizeof(SS_REC_PRINTDEVMODEA32);
    int w32 = sizeof(SS_REC_PRINTDEVMODEW32);
    int a64 = sizeof(SS_REC_PRINTDEVMODEA64);
    int w64 = sizeof(SS_REC_PRINTDEVMODEW64);
    BOOL bIsDEVMODEA32 = (sizeof(SS_REC_PRINTDEVMODEA32) == lpRec->lRecLen);
    BOOL bIsDEVMODEW32 = (sizeof(SS_REC_PRINTDEVMODEW32) == lpRec->lRecLen);
    BOOL bIsDEVMODEA64 = (sizeof(SS_REC_PRINTDEVMODEA64) == lpRec->lRecLen);
    BOOL bIsDEVMODEW64 = (sizeof(SS_REC_PRINTDEVMODEW64) == lpRec->lRecLen);
    if( bIsDEVMODEA32 )
	{ // file saved by 32-bit ANSI Spread (WINVER<5)
      SS_REC_PRINTDEVMODEA32 *pRec = (SS_REC_PRINTDEVMODEA32*)lpRec;
	  DEVMODEA32 *pDevMode = &pRec->PrintDevMode;
#if _UNICODE // convert dmDeviceName and dmFormName from BYTE[] to WCHAR[]
      MultiByteToWideChar(CP_ACP, 0, pDevMode->dmDeviceName, CCHDEVICENAME, lpBook->m_PrinterDevMode.dmDeviceName, CCHDEVICENAME);
      MultiByteToWideChar(CP_ACP, 0, pDevMode->dmFormName, CCHFORMNAME, lpBook->m_PrinterDevMode.dmFormName, CCHFORMNAME);
#else // copy strings
      memcpy(lpBook->m_PrinterDevMode.dmDeviceName, pDevMode->dmDeviceName, CCHDEVICENAME);
      memcpy(lpBook->m_PrinterDevMode.dmFormName, pDevMode->dmFormName, CCHFORMNAME);
#endif
      lpBook->m_PrinterDevMode.dmSpecVersion = pDevMode->dmSpecVersion;
      lpBook->m_PrinterDevMode.dmDriverVersion = pDevMode->dmDriverVersion;
      lpBook->m_PrinterDevMode.dmSize = sizeof(lpBook->m_PrinterDevMode);
      lpBook->m_PrinterDevMode.dmDriverExtra = pDevMode->dmDriverExtra;
      lpBook->m_PrinterDevMode.dmFields = pDevMode->dmFields;
      lpBook->m_PrinterDevMode.dmPosition = pDevMode->dmPosition; // orientation, paper size, paper length, paper width
      lpBook->m_PrinterDevMode.dmScale = pDevMode->dmScale;
      lpBook->m_PrinterDevMode.dmCopies = pDevMode->dmCopies;
      lpBook->m_PrinterDevMode.dmDefaultSource = pDevMode->dmDefaultSource;
      lpBook->m_PrinterDevMode.dmPrintQuality = pDevMode->dmPrintQuality;
      lpBook->m_PrinterDevMode.dmColor = pDevMode->dmColor;
      lpBook->m_PrinterDevMode.dmDuplex = pDevMode->dmDuplex;
      lpBook->m_PrinterDevMode.dmYResolution = pDevMode->dmYResolution;
      lpBook->m_PrinterDevMode.dmTTOption = pDevMode->dmTTOption;
      lpBook->m_PrinterDevMode.dmCollate = pDevMode->dmCollate;
      lpBook->m_PrinterDevMode.dmLogPixels = pDevMode->dmLogPixels;
      lpBook->m_PrinterDevMode.dmBitsPerPel = pDevMode->dmBitsPerPel;
      lpBook->m_PrinterDevMode.dmPelsWidth = pDevMode->dmPelsWidth;
      lpBook->m_PrinterDevMode.dmPelsHeight = pDevMode->dmPelsHeight;
      lpBook->m_PrinterDevMode.dmDisplayFlags = pDevMode->dmDisplayFlags;
      lpBook->m_PrinterDevMode.dmDisplayFrequency = pDevMode->dmDisplayFrequency;
#if(WINVER >= 0x0400)
      lpBook->m_PrinterDevMode.dmICMMethod = pDevMode->dmICMMethod;
      lpBook->m_PrinterDevMode.dmICMIntent = pDevMode->dmICMIntent;
      lpBook->m_PrinterDevMode.dmMediaType = pDevMode->dmMediaType;
      lpBook->m_PrinterDevMode.dmDitherType = pDevMode->dmDitherType;
      lpBook->m_PrinterDevMode.dmReserved1 = pDevMode->dmReserved1;
      lpBook->m_PrinterDevMode.dmReserved2 = pDevMode->dmReserved2;
//#if (WINVER >= 0x0500) || (_WIN32_WINNT >= 0x0400)
//      lpBook->m_PrinterDevMode.dmPanningWidth = pDevMode->dmPanningWidth;
//      lpBook->m_PrinterDevMode.dmPanningHeight = pDevMode->dmPanningHeight;
//#endif
#endif /* WINVER >= 0x0400 */
    }
    else if( bIsDEVMODEA64 )
    { // file saved by 64-bit ANSI Spread (WINVER>=5)
      SS_REC_PRINTDEVMODEA64 *pRec = (SS_REC_PRINTDEVMODEA64*)lpRec;
	  DEVMODEA64 *pDevMode = &pRec->PrintDevMode;
#if _UNICODE // convert dmDeviceName and dmFormName from BYTE[] to WCHAR[]
      MultiByteToWideChar(CP_ACP, 0, pDevMode->dmDeviceName, CCHDEVICENAME, lpBook->m_PrinterDevMode.dmDeviceName, CCHDEVICENAME);
      MultiByteToWideChar(CP_ACP, 0, pDevMode->dmFormName, CCHFORMNAME, lpBook->m_PrinterDevMode.dmFormName, CCHFORMNAME);
#else // copy strings
      memcpy(lpBook->m_PrinterDevMode.dmDeviceName, pDevMode->dmDeviceName, CCHDEVICENAME);
      memcpy(lpBook->m_PrinterDevMode.dmFormName, pDevMode->dmFormName, CCHFORMNAME);
#endif
      lpBook->m_PrinterDevMode.dmSpecVersion = pDevMode->dmSpecVersion;
      lpBook->m_PrinterDevMode.dmDriverVersion = pDevMode->dmDriverVersion;
      lpBook->m_PrinterDevMode.dmSize = sizeof(lpBook->m_PrinterDevMode);
      lpBook->m_PrinterDevMode.dmDriverExtra = pDevMode->dmDriverExtra;
      lpBook->m_PrinterDevMode.dmFields = pDevMode->dmFields;
      lpBook->m_PrinterDevMode.dmPosition = pDevMode->dmPosition; // orientation, paper size, paper length, paper width
      lpBook->m_PrinterDevMode.dmScale = pDevMode->dmScale;
      lpBook->m_PrinterDevMode.dmCopies = pDevMode->dmCopies;
      lpBook->m_PrinterDevMode.dmDefaultSource = pDevMode->dmDefaultSource;
      lpBook->m_PrinterDevMode.dmPrintQuality = pDevMode->dmPrintQuality;
      lpBook->m_PrinterDevMode.dmColor = pDevMode->dmColor;
      lpBook->m_PrinterDevMode.dmDuplex = pDevMode->dmDuplex;
      lpBook->m_PrinterDevMode.dmYResolution = pDevMode->dmYResolution;
      lpBook->m_PrinterDevMode.dmTTOption = pDevMode->dmTTOption;
      lpBook->m_PrinterDevMode.dmCollate = pDevMode->dmCollate;
      lpBook->m_PrinterDevMode.dmLogPixels = pDevMode->dmLogPixels;
      lpBook->m_PrinterDevMode.dmBitsPerPel = pDevMode->dmBitsPerPel;
      lpBook->m_PrinterDevMode.dmPelsWidth = pDevMode->dmPelsWidth;
      lpBook->m_PrinterDevMode.dmPelsHeight = pDevMode->dmPelsHeight;
      lpBook->m_PrinterDevMode.dmDisplayFlags = pDevMode->dmDisplayFlags;
      lpBook->m_PrinterDevMode.dmDisplayFrequency = pDevMode->dmDisplayFrequency;
#if(WINVER >= 0x0400)
      lpBook->m_PrinterDevMode.dmICMMethod = pDevMode->dmICMMethod;
      lpBook->m_PrinterDevMode.dmICMIntent = pDevMode->dmICMIntent;
      lpBook->m_PrinterDevMode.dmMediaType = pDevMode->dmMediaType;
      lpBook->m_PrinterDevMode.dmDitherType = pDevMode->dmDitherType;
      lpBook->m_PrinterDevMode.dmReserved1 = pDevMode->dmReserved1;
      lpBook->m_PrinterDevMode.dmReserved2 = pDevMode->dmReserved2;
#if (WINVER >= 0x0500) || (_WIN32_WINNT >= 0x0400)
      lpBook->m_PrinterDevMode.dmPanningWidth = pDevMode->dmPanningWidth;
      lpBook->m_PrinterDevMode.dmPanningHeight = pDevMode->dmPanningHeight;
#endif
#endif /* WINVER >= 0x0400 */
    }
    else if( bIsDEVMODEW32 )
    { // file saved by 32-bit Unicode Spread (WINVER<5)
      SS_REC_PRINTDEVMODEW32 *pRec = (SS_REC_PRINTDEVMODEW32*)lpRec;
	  DEVMODEW32 *pDevMode = &pRec->PrintDevMode;
#if _UNICODE // copy strings
      memcpy(lpBook->m_PrinterDevMode.dmDeviceName, pDevMode->dmDeviceName, sizeof(TCHAR) * CCHDEVICENAME);
      memcpy(lpBook->m_PrinterDevMode.dmFormName, pDevMode->dmFormName, sizeof(TCHAR) * CCHFORMNAME);
#else //convert dmDeviceName and dmFormName from WCHAR[] to BYTE[]
      WideCharToMultiByte(CP_ACP, 0, pDevMode->dmDeviceName, CCHDEVICENAME, lpBook->m_PrinterDevMode.dmDeviceName, CCHDEVICENAME, NULL, NULL);
      WideCharToMultiByte(CP_ACP, 0, pDevMode->dmFormName, CCHFORMNAME, lpBook->m_PrinterDevMode.dmFormName, CCHFORMNAME, NULL, NULL);
#endif
      lpBook->m_PrinterDevMode.dmSpecVersion = pDevMode->dmSpecVersion;
      lpBook->m_PrinterDevMode.dmDriverVersion = pDevMode->dmDriverVersion;
      lpBook->m_PrinterDevMode.dmSize = sizeof(lpBook->m_PrinterDevMode);
      lpBook->m_PrinterDevMode.dmDriverExtra = pDevMode->dmDriverExtra;
      lpBook->m_PrinterDevMode.dmFields = pDevMode->dmFields;
      lpBook->m_PrinterDevMode.dmPosition = pDevMode->dmPosition; // orientation, paper size, paper length, paper width
      lpBook->m_PrinterDevMode.dmScale = pDevMode->dmScale;
      lpBook->m_PrinterDevMode.dmCopies = pDevMode->dmCopies;
      lpBook->m_PrinterDevMode.dmDefaultSource = pDevMode->dmDefaultSource;
      lpBook->m_PrinterDevMode.dmPrintQuality = pDevMode->dmPrintQuality;
      lpBook->m_PrinterDevMode.dmColor = pDevMode->dmColor;
      lpBook->m_PrinterDevMode.dmDuplex = pDevMode->dmDuplex;
      lpBook->m_PrinterDevMode.dmYResolution = pDevMode->dmYResolution;
      lpBook->m_PrinterDevMode.dmTTOption = pDevMode->dmTTOption;
      lpBook->m_PrinterDevMode.dmCollate = pDevMode->dmCollate;
      lpBook->m_PrinterDevMode.dmLogPixels = pDevMode->dmLogPixels;
      lpBook->m_PrinterDevMode.dmBitsPerPel = pDevMode->dmBitsPerPel;
      lpBook->m_PrinterDevMode.dmPelsWidth = pDevMode->dmPelsWidth;
      lpBook->m_PrinterDevMode.dmPelsHeight = pDevMode->dmPelsHeight;
      lpBook->m_PrinterDevMode.dmDisplayFlags = pDevMode->dmDisplayFlags;
      lpBook->m_PrinterDevMode.dmDisplayFrequency = pDevMode->dmDisplayFrequency;
#if(WINVER >= 0x0400)
      lpBook->m_PrinterDevMode.dmICMMethod = pDevMode->dmICMMethod;
      lpBook->m_PrinterDevMode.dmICMIntent = pDevMode->dmICMIntent;
      lpBook->m_PrinterDevMode.dmMediaType = pDevMode->dmMediaType;
      lpBook->m_PrinterDevMode.dmDitherType = pDevMode->dmDitherType;
      lpBook->m_PrinterDevMode.dmReserved1 = pDevMode->dmReserved1;
      lpBook->m_PrinterDevMode.dmReserved2 = pDevMode->dmReserved2;
//#if (WINVER >= 0x0500) || (_WIN32_WINNT >= 0x0400)
//      lpBook->m_PrinterDevMode.dmPanningWidth = pDevMode->dmPanningWidth;
//      lpBook->m_PrinterDevMode.dmPanningHeight = pDevMode->dmPanningHeight;
//#endif
#endif /* WINVER >= 0x0400 */
    }
    else if( bIsDEVMODEW64 )
    { // file saved by 64-bit Unicode Spread (WINVER>=5)
      SS_REC_PRINTDEVMODEW64 *pRec = (SS_REC_PRINTDEVMODEW64*)lpRec;
	  DEVMODEW64 *pDevMode = &pRec->PrintDevMode;
#if _UNICODE // copy strings
      memcpy(lpBook->m_PrinterDevMode.dmDeviceName, pDevMode->dmDeviceName, sizeof(TCHAR) * CCHDEVICENAME);
      memcpy(lpBook->m_PrinterDevMode.dmFormName, pDevMode->dmFormName, sizeof(TCHAR) * CCHFORMNAME);
#else //convert dmDeviceName and dmFormName from WCHAR[] to BYTE[]
      WideCharToMultiByte(CP_ACP, 0, pDevMode->dmDeviceName, CCHDEVICENAME, lpBook->m_PrinterDevMode.dmDeviceName, CCHDEVICENAME, NULL, NULL);
      WideCharToMultiByte(CP_ACP, 0, pDevMode->dmFormName, CCHFORMNAME, lpBook->m_PrinterDevMode.dmFormName, CCHFORMNAME, NULL, NULL);
#endif
      lpBook->m_PrinterDevMode.dmSpecVersion = pDevMode->dmSpecVersion;
      lpBook->m_PrinterDevMode.dmDriverVersion = pDevMode->dmDriverVersion;
      lpBook->m_PrinterDevMode.dmSize = sizeof(lpBook->m_PrinterDevMode);
      lpBook->m_PrinterDevMode.dmDriverExtra = pDevMode->dmDriverExtra;
      lpBook->m_PrinterDevMode.dmFields = pDevMode->dmFields;
      lpBook->m_PrinterDevMode.dmPosition = pDevMode->dmPosition; // orientation, paper size, paper length, paper width
      lpBook->m_PrinterDevMode.dmScale = pDevMode->dmScale;
      lpBook->m_PrinterDevMode.dmCopies = pDevMode->dmCopies;
      lpBook->m_PrinterDevMode.dmDefaultSource = pDevMode->dmDefaultSource;
      lpBook->m_PrinterDevMode.dmPrintQuality = pDevMode->dmPrintQuality;
      lpBook->m_PrinterDevMode.dmColor = pDevMode->dmColor;
      lpBook->m_PrinterDevMode.dmDuplex = pDevMode->dmDuplex;
      lpBook->m_PrinterDevMode.dmYResolution = pDevMode->dmYResolution;
      lpBook->m_PrinterDevMode.dmTTOption = pDevMode->dmTTOption;
      lpBook->m_PrinterDevMode.dmCollate = pDevMode->dmCollate;
      lpBook->m_PrinterDevMode.dmLogPixels = pDevMode->dmLogPixels;
      lpBook->m_PrinterDevMode.dmBitsPerPel = pDevMode->dmBitsPerPel;
      lpBook->m_PrinterDevMode.dmPelsWidth = pDevMode->dmPelsWidth;
      lpBook->m_PrinterDevMode.dmPelsHeight = pDevMode->dmPelsHeight;
      lpBook->m_PrinterDevMode.dmDisplayFlags = pDevMode->dmDisplayFlags;
      lpBook->m_PrinterDevMode.dmDisplayFrequency = pDevMode->dmDisplayFrequency;
#if(WINVER >= 0x0400)
      lpBook->m_PrinterDevMode.dmICMMethod = pDevMode->dmICMMethod;
      lpBook->m_PrinterDevMode.dmICMIntent = pDevMode->dmICMIntent;
      lpBook->m_PrinterDevMode.dmMediaType = pDevMode->dmMediaType;
      lpBook->m_PrinterDevMode.dmDitherType = pDevMode->dmDitherType;
      lpBook->m_PrinterDevMode.dmReserved1 = pDevMode->dmReserved1;
      lpBook->m_PrinterDevMode.dmReserved2 = pDevMode->dmReserved2;
#if (WINVER >= 0x0500) || (_WIN32_WINNT >= 0x0400)
      lpBook->m_PrinterDevMode.dmPanningWidth = pDevMode->dmPanningWidth;
      lpBook->m_PrinterDevMode.dmPanningHeight = pDevMode->dmPanningHeight;
#endif
#endif /* WINVER >= 0x0400 */
    }
    bRet = (bIsDEVMODEA32 || bIsDEVMODEA64 || bIsDEVMODEW32 || bIsDEVMODEW64);
  }
  return bRet;
}
//--------------------------------------------------------------------
//
//  The SS_Save4PrintDevMode()
//

BOOL SS_Save4PrintDevMode(LPSS_BOOK lpBook, LPSS_BUFF lpBuff)
{
  SS_REC_PRINTDEVMODE rec;
  BOOL bRet = TRUE;
  if( TRUE )
  {
    rec.nRecType = SS_RID_PRINTDEVMODE;
    rec.lRecLen = sizeof(rec);
	rec.PrintDevMode = lpBook->m_PrinterDevMode;
    bRet = SS_BuffAppend(lpBuff, &rec, sizeof(rec));
  }
  return bRet;
}
//--------------------------------------------------------------------
//
//  The SS_CellNoteIndicator2()
//

BOOL SS_Load4CellNoteIndicator2(LPSS_BOOK lpBook, LPSS_REC_COMMON lpCommon)
{
  LPSS_REC_CELLNOTEINDICATOR2 lpRec = (LPSS_REC_CELLNOTEINDICATOR2)lpCommon;
  BOOL bRet = sizeof(*lpRec) == lpRec->lRecLen;
  if( bRet )
  {
	lpBook->CellNoteIndicatorColor = lpRec->clrCellNoteIndicator;
	lpBook->wCellNoteIndicatorShape = lpRec->nCellNoteIndicatorShape;
  }
  return bRet;
}
//--------------------------------------------------------------------
//
//  The SS_Save4CellNoteIndicator2()
//

BOOL SS_Save4CellNoteIndicator2(LPSS_BOOK lpBook, LPSS_BUFF lpBuff)
{
  SS_REC_CELLNOTEINDICATOR2 rec;
  BOOL bRet = TRUE;
  if( TRUE )
  {
    rec.nRecType = SS_RID_CELLNOTEINDICATOR2;
    rec.lRecLen = sizeof(rec);
	rec.clrCellNoteIndicator = lpBook->CellNoteIndicatorColor;
	rec.nCellNoteIndicatorShape = lpBook->wCellNoteIndicatorShape;
    bRet = SS_BuffAppend(lpBuff, &rec, sizeof(rec));
  }
  return bRet;
}
#endif
//--------------------------------------------------------------------
//
//  The SS_Save4CellNoteIndicator()
//

BOOL SS_Save4CellNoteIndicator(LPSS_BOOK lpBook, LPSS_BUFF lpBuff)
{
  SS_REC_CELLNOTEINDICATOR rec;
  BOOL bRet = TRUE;
  if( TRUE )
  {
    rec.nRecType = SS_RID_CELLNOTEINDICATOR;
    rec.lRecLen = sizeof(rec);
    rec.nCellNoteIndicator = lpBook->nCellNoteIndicator;
    bRet = SS_BuffAppend(lpBuff, &rec, sizeof(rec));
  }
  return bRet;
}

//--------------------------------------------------------------------
//
//  The SS_Load4ShowScrollTips()
//


BOOL SS_Load4ShowScrollTips(LPSS_BOOK lpBook, LPSS_REC_COMMON lpCommon)
{
  LPSS_REC_SHOWSCROLLTIPS lpRec = (LPSS_REC_SHOWSCROLLTIPS)lpCommon;
  BOOL bRet = sizeof(*lpRec) == lpRec->lRecLen;
  if( bRet )
  {
    SS_SetShowScrollTips(lpBook, lpRec->nShowScrollTips);
    lpBook->nShowScrollTips = lpRec->nShowScrollTips;
  }
  return bRet;
}


//--------------------------------------------------------------------
//
//  The SS_Save4ShowScrollTips()
//

BOOL SS_Save4ShowScrollTips(LPSS_BOOK lpBook, LPSS_BUFF lpBuff)
{
  SS_REC_SHOWSCROLLTIPS rec;
  BOOL bRet = TRUE;
  if( bRet )
  {
    rec.nRecType = SS_RID_SHOWSCROLLTIPS;
    rec.lRecLen = sizeof(rec);
    rec.nShowScrollTips = lpBook->nShowScrollTips;
    bRet = SS_BuffAppend(lpBuff, &rec, sizeof(rec));
  }
  return bRet;
}

//--------------------------------------------------------------------
//
//  The SS_UserColAction()
//

BOOL SS_Load4UserColAction(LPSPREADSHEET lpSS, LPSS_REC_COMMON lpCommon)
{
  LPSS_REC_USERCOLACTION lpRec = (LPSS_REC_USERCOLACTION)lpCommon;
  BOOL bRet = sizeof(*lpRec) == lpRec->lRecLen;
  if( bRet )
  {
    lpSS->nUserColAction = lpRec->nUserColAction;
  }
  return bRet;
}

//--------------------------------------------------------------------
//
//  The SS_Save4UserColAction()
//

BOOL SS_Save4UserColAction(LPSPREADSHEET lpSS, LPSS_BUFF lpBuff)
{
  SS_REC_USERCOLACTION rec;
  BOOL bRet = TRUE;
  if( TRUE )
  {
    rec.nRecType = SS_RID_USERCOLACTION;
    rec.lRecLen = sizeof(rec);
    rec.nUserColAction = lpSS->nUserColAction;
    bRet = SS_BuffAppend(lpBuff, &rec, sizeof(rec));
  }
  return bRet;
}

//--------------------------------------------------------------------
//
//  The SS_ColUserSortIndicator()
//

BOOL SS_Load4ColUserSortIndicator(LPSPREADSHEET lpSS, LPSS_REC_COMMON lpCommon)
{
  LPSS_REC_COLUSERSORTINDICATOR lpRec = (LPSS_REC_COLUSERSORTINDICATOR)lpCommon;
  BOOL bRet = sizeof(*lpRec) == lpRec->lRecLen;
  if( bRet )
  {
    SS_SetColUserSortIndicator(lpSS, lpRec->lCol, lpRec->nColUserSortIndicator);
  }
  return bRet;
}

//--------------------------------------------------------------------

BOOL SS_Save4ColUserSortIndicator(SS_COORD lCol, short nColUserSortIndicator, LPSS_BUFF lpBuff)
{
  SS_REC_COLUSERSORTINDICATOR rec;
  BOOL bRet = TRUE;
  if( nColUserSortIndicator != -1 )
  {
    rec.nRecType = SS_RID_COLUSERSORTINDICATOR;
    rec.lRecLen = sizeof(rec);
    rec.lCol = lCol;
    rec.nColUserSortIndicator = nColUserSortIndicator;
    bRet = SS_BuffAppend(lpBuff, &rec, sizeof(rec));
  }
  return bRet;
}

//--------------------------------------------------------------------
//
//  The SS_ScriptEnhanced()
//

BOOL SS_Load4ScriptEnhanced(LPSS_BOOK lpBook, LPSS_REC_COMMON lpCommon)
{
  LPSS_REC_SCRIPTENHANCED lpRec = (LPSS_REC_SCRIPTENHANCED)lpCommon;
  BOOL bRet = sizeof(*lpRec) == lpRec->lRecLen;
  if( bRet )
  {
    lpBook->bScriptEnhanced = lpRec->bScriptEnhanced;
  }
  return bRet;
}

//--------------------------------------------------------------------
//
//  The SS_Save4ScriptEnhanced()
//

BOOL SS_Save4ScriptEnhanced(LPSS_BOOK lpBook, LPSS_BUFF lpBuff)
{
  SS_REC_SCRIPTENHANCED rec;
  BOOL bRet = TRUE;
  if( TRUE )
  {
    rec.nRecType = SS_RID_SCRIPTENHANCED;
    rec.lRecLen = sizeof(rec);
    rec.bScriptEnhanced = lpBook->bScriptEnhanced;
    bRet = SS_BuffAppend(lpBuff, &rec, sizeof(rec));
  }
  return bRet;
}

//--------------------------------------------------------------------

BOOL SS_Load4CalTextOverride(LPSS_BOOK lpBook, BOOL bUnicode, LPSS_REC_COMMON lpCommon)
{
  LPSS_REC_CALTEXTOVERRIDE lpRec = (LPSS_REC_CALTEXTOVERRIDE)lpCommon;
  HPBYTE lpText = NULL;
  LPTSTR lpszShortDays = NULL;
  LPTSTR lpszLongDays = NULL;
  LPTSTR lpszShortMonths = NULL;
  LPTSTR lpszLongMonths = NULL;
  LPTSTR lpszOk = NULL;
  LPTSTR lpszCancel = NULL;
  BOOL bRet = sizeof(*lpRec) <= lpRec->lRecLen;
  if( bRet )
  {
    SS_BUFF buff1, buff2, buff3, buff4, buff5, buff6;
    SS_BuffInit(&buff1);
    SS_BuffInit(&buff2);
    SS_BuffInit(&buff3);
    SS_BuffInit(&buff4);
    SS_BuffInit(&buff5);
    SS_BuffInit(&buff6);

    lpText = (HPBYTE)lpRec + sizeof(*lpRec);
    if( sizeof(*lpRec) <= lpRec->lRecLen )
      {
      if (lpRec->nShortDays)
         lpszShortDays = SS_BuffCopyStr(&buff1, lpText, bUnicode);
      lpText += lpRec->nShortDays * sizeof(TCHAR);
      if (lpRec->nLongDays)
         lpszLongDays = SS_BuffCopyStr(&buff2, lpText, bUnicode);
      lpText += lpRec->nLongDays * sizeof(TCHAR);
      if (lpRec->nShortMonths)
         lpszShortMonths = SS_BuffCopyStr(&buff3, lpText, bUnicode);
      lpText += lpRec->nShortMonths * sizeof(TCHAR);
      if (lpRec->nLongMonths)
         lpszLongMonths = SS_BuffCopyStr(&buff4, lpText, bUnicode);
      lpText += lpRec->nLongMonths * sizeof(TCHAR);
      if (lpRec->nOkText)
         lpszOk = SS_BuffCopyStr(&buff5, lpText, bUnicode);
      lpText += lpRec->nOkText * sizeof(TCHAR);
      if (lpRec->nCancelText)
         lpszCancel = SS_BuffCopyStr(&buff6, lpText, bUnicode);
      }

    SS_SetCalTextOverride(lpBook, lpszShortDays, lpszLongDays,
                          lpszShortMonths,lpszLongMonths,
                          lpszOk,lpszCancel);

    SS_BuffFree(&buff1);
    SS_BuffFree(&buff2);
    SS_BuffFree(&buff3);
    SS_BuffFree(&buff4);
    SS_BuffFree(&buff5);
    SS_BuffFree(&buff6);
  }
  return bRet;
}

//--------------------------------------------------------------------

BOOL SS_Save4CalTextOverride(LPSS_BOOK lpBook, LPSS_BUFF lpBuff)
{
  SS_REC_CALTEXTOVERRIDE rec;
  LPTSTR lpszText1, lpszText2, lpszText3, lpszText4, lpszText5, lpszText6;
  BOOL bRet = TRUE;

  rec.nRecType = SS_RID_CALTEXTOVERRIDE;

  rec.lRecLen = sizeof(rec);

  if( lpBook->hDayShortNames )
    {
    lpszText1 = (LPTSTR)tbGlobalLock(lpBook->hDayShortNames);
    rec.nShortDays = lstrlen(lpszText1) + 1;
    }
  else
    rec.nShortDays = 0;
  
  rec.lRecLen += (rec.nShortDays * sizeof(TCHAR));

  if( lpBook->hDayLongNames )
    {
    lpszText2 = (LPTSTR)tbGlobalLock(lpBook->hDayLongNames);
    rec.nLongDays = lstrlen(lpszText2) + 1;
    }
  else
    rec.nLongDays = 0;

  rec.lRecLen += (rec.nLongDays * sizeof(TCHAR));

  if( lpBook->hMonthShortNames )
    {
    lpszText3 = (LPTSTR)tbGlobalLock(lpBook->hMonthShortNames);
    rec.nShortMonths = lstrlen(lpszText3) + 1;
    }
  else
    rec.nShortMonths = 0;

  rec.lRecLen += (rec.nShortMonths * sizeof(TCHAR));

  if( lpBook->hMonthLongNames )
    {
    lpszText4 = (LPTSTR)tbGlobalLock(lpBook->hMonthLongNames);
    rec.nLongMonths = lstrlen(lpszText4) + 1;
    }
  else
    rec.nLongMonths = 0;

  rec.lRecLen += (rec.nLongMonths * sizeof(TCHAR));

  if( lpBook->hOkText )
    {
    lpszText5 = (LPTSTR)tbGlobalLock(lpBook->hOkText);
    rec.nOkText = lstrlen(lpszText5) + 1;
    }
  else
    rec.nOkText = 0;

  rec.lRecLen += (rec.nOkText * sizeof(TCHAR));

  if( lpBook->hCancelText )
    {
    lpszText6 = (LPTSTR)tbGlobalLock(lpBook->hCancelText);
    rec.nCancelText = lstrlen(lpszText6) + 1;
    }
  else
    rec.nCancelText = 0;

  rec.lRecLen += (rec.nCancelText * sizeof(TCHAR));

  bRet = SS_BuffAppend(lpBuff, &rec, sizeof(rec));
  bRet &= SS_BuffAppend(lpBuff, lpszText1, rec.nShortDays * sizeof(TCHAR));
  bRet &= SS_BuffAppend(lpBuff, lpszText2, rec.nLongDays * sizeof(TCHAR));
  bRet &= SS_BuffAppend(lpBuff, lpszText3, rec.nShortMonths * sizeof(TCHAR));
  bRet &= SS_BuffAppend(lpBuff, lpszText4, rec.nLongMonths * sizeof(TCHAR));
  bRet &= SS_BuffAppend(lpBuff, lpszText5, rec.nOkText * sizeof(TCHAR));
  bRet &= SS_BuffAppend(lpBuff, lpszText6, rec.nCancelText * sizeof(TCHAR));

  tbGlobalUnlock(lpBook->hDayShortNames);
  tbGlobalUnlock(lpBook->hDayLongNames);
  tbGlobalUnlock(lpBook->hMonthShortNames);
  tbGlobalUnlock(lpBook->hMonthLongNames);
  tbGlobalUnlock(lpBook->hOkText);
  tbGlobalUnlock(lpBook->hCancelText);

  return bRet;
}

#endif //SS_V35

//--------------------------------------------------------------------

BOOL SS_Save4Type(LPSPREADSHEET lpSS, SS_COORD lCol, SS_COORD lRow,
                  LPSS_CELLTYPE lpCellType, LPSS_BUFF lpBuff)
{
  BOOL bRet = FALSE;
  switch( lpCellType->Type )
  {
    case SS_TYPE_BUTTON:
      bRet = SS_Save4TypeButton(lpSS, lCol, lRow, lpCellType, lpBuff);
      break;
    case SS_TYPE_CHECKBOX:
      bRet = SS_Save4TypeCheck(lpSS, lCol, lRow, lpCellType, lpBuff);
      break;
    case SS_TYPE_COMBOBOX:
#ifdef SS_V40
      bRet = SS_Save4TypeComboEx4(lpSS, lCol, lRow, lpCellType, lpBuff);
#elif defined(SS_V30)
      bRet = SS_Save4TypeComboEx(lpSS, lCol, lRow, lpCellType, lpBuff);
#else
      bRet = SS_Save4TypeCombo(lpSS, lCol, lRow, lpCellType, lpBuff);
#endif
      break;
    case SS_TYPE_DATE:
      bRet = SS_Save4TypeDate(lpSS, lCol, lRow, lpCellType, lpBuff);
      break;
    case SS_TYPE_EDIT:
      bRet = SS_Save4TypeEdit(lpSS, lCol, lRow, lpCellType, lpBuff);
      break;
    case SS_TYPE_FLOAT:
      bRet = SS_Save4TypeFloat(lpSS, lCol, lRow, lpCellType, lpBuff);
      break;
    case SS_TYPE_INTEGER:
      bRet = SS_Save4TypeInteger(lpSS, lCol, lRow, lpCellType, lpBuff);
      break;
    case SS_TYPE_PIC:
      bRet = SS_Save4TypePic(lpSS, lCol, lRow, lpCellType, lpBuff);
      break;
    case SS_TYPE_PICTURE:
      bRet = SS_Save4TypePicture(lpSS, lCol, lRow, lpCellType, lpBuff);
      break;
    case SS_TYPE_OWNERDRAW:
      bRet = SS_Save4TypeOwnerDraw(lpSS, lCol, lRow, lpCellType, lpBuff);
      break;
    case SS_TYPE_STATICTEXT:
      bRet = SS_Save4TypeStatic(lpSS, lCol, lRow, lpCellType, lpBuff);
      break;
    case SS_TYPE_TIME:
      bRet = SS_Save4TypeTime(lpSS, lCol, lRow, lpCellType, lpBuff);
      break;
#ifdef SS_V40
    case SS_TYPE_CURRENCY:
      bRet = SS_Save4TypeCurrency(lpSS, lCol, lRow, lpCellType, lpBuff);
      break;
    case SS_TYPE_NUMBER:
      bRet = SS_Save4TypeNumber(lpSS, lCol, lRow, lpCellType, lpBuff);
      break;
    case SS_TYPE_PERCENT:
      bRet = SS_Save4TypePercent(lpSS, lCol, lRow, lpCellType, lpBuff);
      break;
#endif // SS_V40
#ifdef SS_V70
	case SS_TYPE_SCIENTIFIC:
      bRet = SS_Save4TypeScientific(lpSS, lCol, lRow, lpCellType, lpBuff);
      break;
#endif // SS_V70
#if SS_V80
   case SS_TYPE_CUSTOM:
      bRet = SS_Save4TypeCustom(lpSS, lCol, lRow, lpCellType, lpBuff);
      break;
#endif
  }
  return bRet;
}

//--------------------------------------------------------------------

BOOL SS_Save4TypeHandle(LPSPREADSHEET lpSS, SS_COORD lCol, SS_COORD lRow,
                        TBGLOBALHANDLE hCellType, LPSS_BUFF lpBuff)
{
  LPSS_CELLTYPE lpCellType;
  BOOL bRet = TRUE;
  if( hCellType && (lpCellType = (LPSS_CELLTYPE)tbGlobalLock(hCellType)) )
  {
    bRet = SS_Save4Type(lpSS, lCol, lRow, lpCellType, lpBuff);
    tbGlobalUnlock(hCellType);
  }
  return bRet;
}

//--------------------------------------------------------------------

BOOL SS_Load4UnitType(LPSS_BOOK lpBook, LPSS_REC_COMMON lpCommon)
{
  LPSS_REC_UNITTYPE lpRec = (LPSS_REC_UNITTYPE)lpCommon;
  BOOL bRet = sizeof(*lpRec) == lpRec->lRecLen;
  if( bRet )
  {
    lpBook->dUnitType = lpRec->nUnitType;
  }
  return bRet;
}

//--------------------------------------------------------------------

BOOL SS_Save4UnitType(LPSS_BOOK lpBook, LPSS_BUFF lpBuff)
{
  SS_REC_UNITTYPE rec;
  BOOL bRet = TRUE;
  if( SS_UNITTYPE_DEFAULT != lpBook->dUnitType )
  {
    rec.nRecType = SS_RID_UNITTYPE;
    rec.lRecLen = sizeof(rec);
    rec.nUnitType = lpBook->dUnitType;
    bRet = SS_BuffAppend(lpBuff, &rec, sizeof(rec));
  }
  return bRet;
}

//--------------------------------------------------------------------

BOOL SS_Load4UserResize(LPSPREADSHEET lpSS, LPSS_REC_COMMON lpCommon)
{
  LPSS_REC_USERRESIZE lpRec = (LPSS_REC_USERRESIZE)lpCommon;
  BOOL bRet = sizeof(*lpRec) == lpRec->lRecLen;
  if( bRet )
  {
    lpSS->wUserResize = lpRec->wResize;
  }
  return bRet;
}

//--------------------------------------------------------------------

BOOL SS_Save4UserResize(LPSPREADSHEET lpSS, LPSS_BUFF lpBuff)
{
  SS_REC_USERRESIZE rec;
  BOOL bRet = TRUE;
  if( (SS_USERRESIZE_COL | SS_USERRESIZE_ROW) != lpSS->wUserResize )
  {
    rec.nRecType = SS_RID_USERRESIZE;
    rec.lRecLen = sizeof(rec);
    rec.wResize = lpSS->wUserResize;
    bRet = SS_BuffAppend(lpBuff, &rec, sizeof(rec));
  }
  return bRet;
}

//--------------------------------------------------------------------

BOOL SS_Load4UserResizeCol(LPSPREADSHEET lpSS, LPSS_REC_COMMON lpCommon)
{
  LPSS_REC_USERRESIZECOL lpRec = (LPSS_REC_USERRESIZECOL)lpCommon;
  BOOL bRet = sizeof(*lpRec) == lpRec->lRecLen;
  if( bRet )
  {
    SS_SetUserResizeCol(lpSS, lpRec->lCol, lpRec->nResize);
  }
  return bRet;
}

//--------------------------------------------------------------------

BOOL SS_Save4UserResizeCol(LPSPREADSHEET lpSS, SS_COORD lCol, short nResize, LPSS_BUFF lpBuff)
{
  SS_REC_USERRESIZECOL rec;
  short nDefResize = lCol < 0 || lCol >= lpSS->Col.HeaderCnt ? SS_RESIZE_DEFAULT : SS_RESIZE_OFF;
  BOOL bRet = TRUE;
  if( nResize != nDefResize )
  {
    rec.nRecType = SS_RID_USERRESIZECOL;
    rec.lRecLen = sizeof(rec);
    rec.lCol = lCol;
    rec.nResize = nResize;
    bRet = SS_BuffAppend(lpBuff, &rec, sizeof(rec));
  }
  return bRet;
}

//--------------------------------------------------------------------

BOOL SS_Load4UserResizeRow(LPSPREADSHEET lpSS, LPSS_REC_COMMON lpCommon)
{
  LPSS_REC_USERRESIZEROW lpRec = (LPSS_REC_USERRESIZEROW)lpCommon;
  BOOL bRet = sizeof(*lpRec) == lpRec->lRecLen;
  if( bRet )
  {
    SS_SetUserResizeRow(lpSS, lpRec->lRow, lpRec->nResize);
  }
  return bRet;
}

//--------------------------------------------------------------------

BOOL SS_Save4UserResizeRow(LPSPREADSHEET lpSS, SS_COORD lRow, short nResize, LPSS_BUFF lpBuff)
{
  SS_REC_USERRESIZEROW rec;
  short nDefResize = lRow < 0 || lRow >= lpSS->Row.HeaderCnt ? SS_RESIZE_DEFAULT : SS_RESIZE_OFF;
  BOOL bRet = TRUE;
  if( nResize != nDefResize )
  {
    rec.nRecType = SS_RID_USERRESIZEROW;
    rec.lRecLen = sizeof(rec);
    rec.lRow = lRow;
    rec.nResize = nResize;
    bRet = SS_BuffAppend(lpBuff, &rec, sizeof(rec));
  }
  return bRet;
}

//--------------------------------------------------------------------

BOOL SS_Load4ValDbl(LPSPREADSHEET lpSS, LPSS_REC_COMMON lpCommon,
                    LPSS_LOADBLOCK lpLoadBlock)
{
  LPSS_REC_VALDBL lpRec = (LPSS_REC_VALDBL)lpCommon;
  SS_COORD lCol, lRow;
  BOOL bRet = sizeof(*lpRec) == lpRec->lRecLen;
  if( bRet &&
      BlockTranslate(lpLoadBlock, lpRec->lCol, lpRec->lRow, &lCol, &lRow) )
  {
    SS_SetDataRange(lpSS, lCol, lRow, lCol, lRow, (LPTSTR)&lpRec->dfVal,
                    SS_VALUE_FLOAT, FALSE, FALSE);
  }
  return bRet;
}

//--------------------------------------------------------------------

BOOL SS_Load4ValLong(LPSPREADSHEET lpSS, LPSS_REC_COMMON lpCommon,
                    LPSS_LOADBLOCK lpLoadBlock)
{
  LPSS_REC_VALLONG lpRec = (LPSS_REC_VALLONG)lpCommon;
  SS_COORD lCol, lRow;
  BOOL bRet = sizeof(*lpRec) == lpRec->lRecLen;
  if( bRet &&
      BlockTranslate(lpLoadBlock, lpRec->lCol, lpRec->lRow, &lCol, &lRow) )
  {
    SS_SetDataRange(lpSS, lCol, lRow, lCol, lRow, (LPTSTR)&lpRec->lVal,
                    SS_VALUE_INT, FALSE, FALSE);
  }
  return bRet;
}

//--------------------------------------------------------------------

BOOL SS_Load4ValStr(LPSPREADSHEET lpSS, LPSS_REC_COMMON lpCommon,
                    BOOL bUnicode, LPSS_LOADBLOCK lpLoadBlock)
{
  LPSS_REC_VALSTR lpRec = (LPSS_REC_VALSTR)lpCommon;
  HPBYTE lpText = NULL;
  LPTSTR lpszText = NULL;
  SS_COORD lCol, lRow;
  BOOL bRet = sizeof(*lpRec) <= lpRec->lRecLen;
  if( bRet &&
      BlockTranslate(lpLoadBlock, lpRec->lCol, lpRec->lRow, &lCol, &lRow) )
  {
    SS_BUFF buff;
    SS_BuffInit(&buff);
    lpText = (HPBYTE)lpRec + sizeof(*lpRec);
    if( sizeof(*lpRec) <= lpRec->lRecLen )
      lpszText = SS_BuffCopyStr(&buff, lpText, bUnicode);

    SS_SetDataRange(lpSS, lCol, lRow, lCol, lRow, lpszText,
                    SS_VALUE_TEXT, FALSE, FALSE);

    SS_BuffFree(&buff);
  }
  return bRet;
}

//--------------------------------------------------------------------

#if SS_V80
BOOL SS_Load4ValBuffer(LPSPREADSHEET lpSS, LPSS_REC_COMMON lpCommon,
                    BOOL bUnicode, LPSS_LOADBLOCK lpLoadBlock)
{
  LPSS_REC_VALBUFFER lpRec = (LPSS_REC_VALBUFFER)lpCommon;
  LPVOID lpData = NULL;
  SS_COORD lCol, lRow;
  BOOL bRet = sizeof(*lpRec) <= lpRec->lRecLen;
  if( bRet &&
      BlockTranslate(lpLoadBlock, lpRec->lCol, lpRec->lRow, &lCol, &lRow) )
  {
    lpData = (HPBYTE)lpRec + sizeof(*lpRec);
    if( sizeof(*lpRec) <= lpRec->lRecLen )
       SS_SetBuffer(lpSS, lCol, lRow, lpData, lpRec->lSize);
  }
  return bRet;
}
#endif

//--------------------------------------------------------------------

BOOL SS_Save4Value(SS_COORD lCol, SS_COORD lRow, LPSS_DATA lpData,
                   LPSS_BUFF lpBuff)
{
  SS_REC_VALLONG recLong;
  SS_REC_VALDBL recDbl;
  SS_REC_VALSTR recStr;
#if SS_V80
  SS_REC_VALBUFFER recBuffer;
#endif
  LPTSTR lpszText;
  long lTextLen;
  BOOL bRet = TRUE;
  switch( lpData->bDataType )
  {
    case SS_TYPE_INTEGER:
      recLong.nRecType = SS_RID_VALLONG;
      recLong.lRecLen = sizeof(recLong);
      recLong.lCol = lCol;
      recLong.lRow = lRow;
      recLong.lVal = lpData->Data.lValue;
      bRet = SS_BuffAppend(lpBuff, &recLong, sizeof(recLong));
      break;
    case SS_TYPE_FLOAT:
      recDbl.nRecType = SS_RID_VALDBL;
      recDbl.lRecLen = sizeof(recDbl);
      recDbl.lCol = lCol;
      recDbl.lRow = lRow;
      recDbl.dfVal = lpData->Data.dfValue;
      bRet = SS_BuffAppend(lpBuff, &recDbl, sizeof(recDbl));
      break;
    case SS_TYPE_EDIT:
      if( lpData->Data.hszData )
      {
        lpszText = (LPTSTR)tbGlobalLock(lpData->Data.hszData);
        lTextLen = lstrlen(lpszText) + 1;
        recStr.nRecType = SS_RID_VALSTR;
        recStr.lRecLen = sizeof(recStr) + lTextLen * sizeof(TCHAR);
        recStr.lCol = lCol;
        recStr.lRow = lRow;
        bRet = SS_BuffAppend(lpBuff, &recStr, sizeof(recStr));
        bRet &= SS_BuffAppend(lpBuff, lpszText, lTextLen * sizeof(TCHAR));
        tbGlobalUnlock(lpData->Data.hszData);
      }
      break;
#if SS_V80
    case SS_DATATYPE_BUFFER:
       if( lpData->Data.Buffer.hBuffer )
       {
          LPVOID lpvData = (LPVOID)tbGlobalLock(lpData->Data.Buffer.hBuffer);
          recBuffer.nRecType = SS_RID_VALBUFFER;
          recBuffer.lRecLen = sizeof(recBuffer) + lpData->Data.Buffer.lSize;
          recBuffer.lCol = lCol;
          recBuffer.lRow = lRow;
          recBuffer.lSize = lpData->Data.Buffer.lSize;
          bRet = SS_BuffAppend(lpBuff, &recBuffer, sizeof(recBuffer));
          bRet &= SS_BuffAppend(lpBuff, lpvData, lpData->Data.Buffer.lSize);
          tbGlobalUnlock(lpData->Data.Buffer.hBuffer);
       }
       break;
#endif
  }
  return bRet;
}

//--------------------------------------------------------------------

BOOL SS_Load4Virtual(LPSPREADSHEET lpSS, LPSS_REC_COMMON lpCommon)
{
  LPSS_REC_VIRTUAL lpRec = (LPSS_REC_VIRTUAL)lpCommon;
  BOOL bRet = sizeof(*lpRec) == lpRec->lRecLen;
  if( bRet )
  {
    lpSS->fVirtualMode = lpRec->bVMode;
    lpSS->Virtual.lVStyle = lpRec->lVStyle;
    lpSS->Virtual.VMax = lpRec->lVMax;
    lpSS->Virtual.VTop = lpRec->lVTop;
    lpSS->Virtual.VSize = lpRec->lVSize;
    lpSS->Virtual.VOverlap = lpRec->lVOverlap;
    lpSS->Virtual.VPhysSize = lpRec->lVPhysSize;
    #ifndef SS_NOSCBAR
    SS_VScrollSetSpecial(lpSS->lpBook, lpRec->bUseVScroll, lpRec->wVScrollOptions);
    #endif
  }
  return bRet;
}

//--------------------------------------------------------------------

BOOL SS_Save4Virtual(LPSPREADSHEET lpSS, LPSS_BUFF lpBuff)
{
  SS_REC_VIRTUAL rec;
  BOOL bRet = TRUE;
  if( lpSS->fVirtualMode || lpSS->lpBook->fUseSpecialVScroll ||
       lpSS->lpBook->wSpecialVScrollOptions ||
      lpSS->Virtual.lVStyle || -1 != lpSS->Virtual.VMax ||
      lpSS->Virtual.VTop || lpSS->Virtual.VSize ||
      lpSS->Virtual.VOverlap || lpSS->Virtual.VPhysSize )
  {
    rec.nRecType = SS_RID_VIRTUAL;
    rec.lRecLen = sizeof(rec);
    rec.bVMode = lpSS->fVirtualMode;
    rec.bUseVScroll = lpSS->lpBook->fUseSpecialVScroll;
    rec.wVScrollOptions = lpSS->lpBook->wSpecialVScrollOptions;
    rec.lVStyle = lpSS->Virtual.lVStyle;
    rec.lVMax = lpSS->Virtual.VMax;
    rec.lVTop = lpSS->Virtual.VTop;
    rec.lVSize = lpSS->Virtual.VSize;
    rec.lVOverlap = lpSS->Virtual.VOverlap;
    rec.lVPhysSize = lpSS->Virtual.VPhysSize;
    bRet = SS_BuffAppend(lpBuff, &rec, sizeof(rec));
  }
  return bRet;
}

#ifdef SS_V30
//--------------------------------------------------------------------
//
//  The SS_Load4ActionKey() function loads the function keys assiged to
//  which action.
//

BOOL SS_Load4ActionKey(LPSS_BOOK lpBook, LPSS_REC_COMMON lpCommon)
{
  LPSS_REC_ACTIONKEY lpRec = (LPSS_REC_ACTIONKEY)lpCommon;
  BOOL bRet = sizeof(*lpRec) == lpRec->lRecLen;
  if( bRet )
  {
    _fmemcpy(lpBook->ActionKeys, lpRec->Keys, sizeof(DWORD) * 3);
  }
  return bRet;
}

//--------------------------------------------------------------------
//
//  The SS_Save4ActionKey() function saves the function keys assiged to
//  which action.
//

BOOL SS_Save4ActionKey(LPSS_BOOK lpBook, LPSS_BUFF lpBuff)
{
  SS_REC_ACTIONKEY rec;
  BOOL bRet = TRUE;
  if( TRUE )
  {
    rec.nRecType = SS_RID_ACTIONKEY;
    rec.lRecLen = sizeof(rec);
    _fmemcpy(rec.Keys, lpBook->ActionKeys, sizeof(DWORD) * 3);
    bRet = SS_BuffAppend(lpBuff, &rec, sizeof(rec));
  }
  return bRet;
}

//--------------------------------------------------------------------
//
//  The SS_Load4OddEvenRowColor() function loads the colors used for
//  odd and even rows.
//

BOOL SS_Load4OddEvenRowColor(LPSPREADSHEET lpSS, LPSS_REC_COMMON lpCommon)
{
  LPSS_REC_ODDEVENROWCOLOR lpRec = (LPSS_REC_ODDEVENROWCOLOR)lpCommon;
  BOOL bRet = sizeof(*lpRec) == lpRec->lRecLen;
  if( bRet )
  {
    COLORREF clrBackOdd = lpRec->clrBackOdd;
    COLORREF clrForeOdd = lpRec->clrForeOdd;
    COLORREF clrBackEven = lpRec->clrBackEven;
    COLORREF clrForeEven = lpRec->clrForeEven;
    if( (COLORREF)-1 == clrBackOdd )
      clrBackOdd = RGBCOLOR_DEFAULT;
    if( (COLORREF)-1 == clrForeOdd )
      clrForeOdd = RGBCOLOR_DEFAULT;
    if( (COLORREF)-1 == clrBackEven )
      clrBackEven = RGBCOLOR_DEFAULT;
    if( (COLORREF)-1 == clrForeEven )
      clrForeEven = RGBCOLOR_DEFAULT;

    SS_SetOddEvenRowColor(lpSS, clrBackOdd, clrForeOdd, clrBackEven, clrForeEven);
  }
  return bRet;
}

//--------------------------------------------------------------------
//
//  The SS_Saved4OddEvenRowColor() function saves the colors used for
//  odd and even rows.
//

BOOL SS_Save4OddEvenRowColor(LPSPREADSHEET lpSS, LPSS_BUFF lpBuff)
{
  SS_REC_ODDEVENROWCOLOR rec;
  BOOL bRet = TRUE;
  if( TRUE )
  {
    rec.nRecType = SS_RID_ODDEVENROWCOLOR;
    rec.lRecLen = sizeof(rec);
    SS_GetOddEvenRowColor(lpSS, &rec.clrBackOdd, &rec.clrForeOdd,
                          &rec.clrBackEven, &rec.clrForeEven);
    rec.clrBackOdd = SS_ISDEFCOLOR(rec.clrBackOdd) ? (COLORREF)-1 : rec.clrBackOdd;
    rec.clrForeOdd = SS_ISDEFCOLOR(rec.clrForeOdd) ? (COLORREF)-1 : rec.clrForeOdd;
    rec.clrBackEven = SS_ISDEFCOLOR(rec.clrBackEven) ? (COLORREF)-1 : rec.clrBackEven;
    rec.clrForeEven = SS_ISDEFCOLOR(rec.clrForeEven) ? (COLORREF)-1 : rec.clrForeEven;
    bRet = SS_BuffAppend(lpBuff, &rec, sizeof(rec));
  }
  return bRet;
}

//--------------------------------------------------------------------
//
//  The SS_Load4Appearance() function loads the colors used for
//  odd and even rows.
//

BOOL SS_Load4Appearance(LPSS_BOOK lpBook, LPSS_REC_COMMON lpCommon)
{
  LPSS_REC_APPEARANCE lpRec = (LPSS_REC_APPEARANCE)lpCommon;
  BOOL bRet = sizeof(*lpRec) == lpRec->lRecLen;
  if( bRet )
  {
    SS_SetAppearance(lpBook, (WORD)lpRec->bAppearance);
  }
  return bRet;
}

//--------------------------------------------------------------------
//
//  The SS_Save4Appearance() function saves the colors used for
//  odd and even rows.
//

BOOL SS_Save4Appearance(LPSS_BOOK lpBook, LPSS_BUFF lpBuff)
{
  SS_REC_APPEARANCE rec;
  BOOL bRet = TRUE;
  if( TRUE )
  {
    rec.nRecType = SS_RID_APPEARANCE;
    rec.lRecLen = sizeof(rec);
    rec.bAppearance = lpBook->bAppearance;
    bRet = SS_BuffAppend(lpBuff, &rec, sizeof(rec));
  }
  return bRet;
}

//--------------------------------------------------------------------
//
//  The SS_Load4TextTip() function loads the TextTip info
//

#define POINTS_PER_INCH        72
#define HEIGHT_UNITS_PER_POINT 100
#define HEIGHT_UNITS_PER_INCH  (HEIGHT_UNITS_PER_POINT * POINTS_PER_INCH)

BOOL SS_Load4TextTip(LPSS_BOOK lpBook, LPSS_REC_COMMON lpCommon, BOOL bUnicode)
{
  LPSS_REC_TEXTTIP lpRec = (LPSS_REC_TEXTTIP)lpCommon;
  LPSS_SUBREC_LOGFONT lpSubRec;
  SS_BUFF buff;
  HPBYTE lpFaceName;
  LPTSTR lpszFaceName;
  BOOL bRet = sizeof(*lpRec) <= lpRec->lRecLen;

  if( bRet )
  {
   FP_TT_INFO Info;
   WORD wItems = FP_TT_ITEM_STATUS | FP_TT_ITEM_BACKCOLOR | FP_TT_ITEM_FORECOLOR |
                 FP_TT_ITEM_DELAY | FP_TT_ITEM_FONT;

   SS_BuffInit(&buff);
   lpSubRec = (LPSS_SUBREC_LOGFONT)((HPBYTE)lpRec + sizeof(*lpRec));

   _fmemset(&Info.LogFont, '\0', sizeof(LOGFONT));
   lpFaceName = (HPBYTE)lpSubRec + sizeof(*lpSubRec);
   lpszFaceName = SS_BuffCopyStr(&buff, lpFaceName, bUnicode);
   Info.LogFont.lfHeight = MulDiv((int)lpSubRec->lfHeight, dyPixelsPerInch,
                                  HEIGHT_UNITS_PER_INCH);
   Info.LogFont.lfWidth = 0;
   Info.LogFont.lfEscapement = (int)lpSubRec->lfEscapement;
   Info.LogFont.lfOrientation = (int)lpSubRec->lfOrientation;
   Info.LogFont.lfWeight = (int)lpSubRec->lfWeight;
   Info.LogFont.lfItalic = lpSubRec->lfItalic;
   Info.LogFont.lfUnderline = lpSubRec->lfUnderline;
   Info.LogFont.lfStrikeOut = lpSubRec->lfStrikeOut;
   Info.LogFont.lfCharSet = lpSubRec->lfCharSet;
   Info.LogFont.lfOutPrecision = lpSubRec->lfOutPrecision;
   Info.LogFont.lfClipPrecision = lpSubRec->lfClipPrecision;
   Info.LogFont.lfQuality = lpSubRec->lfQuality;
   Info.LogFont.lfPitchAndFamily = lpSubRec->lfPitchAndFamily;
   lstrcpy(Info.LogFont.lfFaceName, lpszFaceName);

   SS_BuffFree(&buff);

#if defined(SS_OCX) || defined(SS_VB)
#ifdef SS_OCX
   if (SS_IsDesignTime(lpBook))
#else
   if (VBGetMode() == MODE_DESIGN)
#endif
      {
      Info.fActive = FALSE;
      wItems |= FP_TT_ITEM_ACTIVE;
      }
#endif

   Info.wStatus = (WORD)lpRec->bStatus;
   Info.lDelay = lpRec->lDelay;
   Info.clrBack = lpRec->clrBack;
   Info.clrFore = lpRec->clrFore;
/*
#ifdef SS_V35 // TT_V2 is assumed if SS_V35
   Info.fShowScrollTips = lpRec->fShowScrollTips;
#elif defined(TT_V2)
   Info.fShowScrollTips = FALSE;
#endif
*/
#ifdef TT_V2
   Info.fShowScrollTips = FALSE;
#endif

   if (lpBook->hWnd)
      fpTextTipSetInfo(lpBook->hWnd, wItems, &Info);
   else
      {
      lpBook->TT_Info = Info;
#ifdef TT_V2
      lpBook->TT_Info.fShowScrollTips = FALSE;
#endif
      lpBook->fLoadTextTip = TRUE;
      }
  }

  return bRet;
}

//--------------------------------------------------------------------
//
//  The SS_Save4TextTip() function saves the TextTip info
//

BOOL SS_Save4TextTip(LPSS_BOOK lpBook, LPSS_BUFF lpBuff)
{
  SS_REC_TEXTTIP rec;
  SS_SUBREC_LOGFONT logFont;
  long lBuffLenOld = lpBuff->lLen;
  BOOL bRet = TRUE;
  if( TRUE )
  {
    FP_TT_INFO Info;

    rec.nRecType = SS_RID_TEXTTIP;
    rec.lRecLen = sizeof(rec);

    fpTextTipGetInfo(lpBook->hWnd, &Info);

    rec.clrBack = Info.clrBack;
    rec.clrFore = Info.clrFore;
    rec.bStatus = (BYTE)Info.wStatus;
    rec.lDelay = Info.lDelay;

    bRet = SS_BuffAppend(lpBuff, &rec, sizeof(rec));

    logFont.lfHeight = MulDiv(Info.LogFont.lfHeight,
                              HEIGHT_UNITS_PER_INCH, dyPixelsPerInch);
    logFont.lfEscapement = Info.LogFont.lfEscapement;
    logFont.lfOrientation = Info.LogFont.lfOrientation;
    logFont.lfWeight = Info.LogFont.lfWeight;
    logFont.lfItalic = Info.LogFont.lfItalic;
    logFont.lfUnderline = Info.LogFont.lfUnderline;
    logFont.lfStrikeOut = Info.LogFont.lfStrikeOut;
    logFont.lfCharSet = Info.LogFont.lfCharSet;
    logFont.lfOutPrecision = Info.LogFont.lfOutPrecision;
    logFont.lfClipPrecision = Info.LogFont.lfClipPrecision;
    logFont.lfQuality = Info.LogFont.lfQuality;
    logFont.lfPitchAndFamily = Info.LogFont.lfPitchAndFamily;
    logFont.lfFaceNameLen = (lstrlen(Info.LogFont.lfFaceName) + 1)
                             * sizeof(TCHAR);
    bRet &= SS_BuffAppend(lpBuff, &logFont, sizeof(logFont));
    bRet &= SS_BuffAppend(lpBuff, Info.LogFont.lfFaceName,
                          logFont.lfFaceNameLen);

    if( bRet && lpBuff->pMem )
      MemHugeCpy(&rec, lpBuff->pMem + lBuffLenOld, sizeof(rec));
    rec.lRecLen = lpBuff->lLen - lBuffLenOld;
    if( bRet && lpBuff->pMem )
      MemHugeCpy(lpBuff->pMem + lBuffLenOld, &rec, sizeof(rec));
  }

  return bRet;
}

//--------------------------------------------------------------------
//
//  The SS_Load4ScrollBarTrack() function loads the Scroll Bar
//  tracking options.
//

BOOL SS_Load4ScrollBarTrack(LPSPREADSHEET lpSS, LPSS_REC_COMMON lpCommon)
{
  LPSS_REC_SCROLLBARTRACK lpRec = (LPSS_REC_SCROLLBARTRACK)lpCommon;
  BOOL bRet = sizeof(*lpRec) == lpRec->lRecLen;
  if( bRet )
  {
    SS_SetBool(lpSS->lpBook, lpSS, SSB_VSCROLLBARTRACK, lpRec->fVTrack);
    SS_SetBool(lpSS->lpBook, lpSS, SSB_HSCROLLBARTRACK, lpRec->fHTrack);
  }
  return bRet;
}

//--------------------------------------------------------------------
//
//  The SS_Save4ScrollBarTrack() function saves the Scroll Bar
//  tracking options.
//

BOOL SS_Save4ScrollBarTrack(LPSPREADSHEET lpSS, LPSS_BUFF lpBuff)
{
  SS_REC_SCROLLBARTRACK rec;
  BOOL bRet = TRUE;
  if( TRUE )
  {
    rec.nRecType = SS_RID_SCROLLBARTRACK;
    rec.lRecLen = sizeof(rec);
    rec.fVTrack = SS_GetBool(lpSS->lpBook, lpSS, SSB_VSCROLLBARTRACK);
    rec.fHTrack = SS_GetBool(lpSS->lpBook, lpSS, SSB_HSCROLLBARTRACK);
    bRet = SS_BuffAppend(lpBuff, &rec, sizeof(rec));
  }
  return bRet;
}

//--------------------------------------------------------------------
//
//  The SS_Load4ClipboardOptions() function loads the clipboard options.
//

BOOL SS_Load4ClipboardOptions(LPSS_BOOK lpBook, LPSS_REC_COMMON lpCommon)
{
  LPSS_REC_CLIPBOARDOPTIONS lpRec = (LPSS_REC_CLIPBOARDOPTIONS)lpCommon;
  BOOL bRet = sizeof(*lpRec) == lpRec->lRecLen;
  if( bRet )
  {
    lpBook->bClipboardOptions = lpRec->bOptions;
  }
  return bRet;
}

//--------------------------------------------------------------------
//
//  The SS_Save4ClipboardOptions() function saves the clipboard options.
//

BOOL SS_Save4ClipboardOptions(LPSS_BOOK lpBook, LPSS_BUFF lpBuff)
{
  SS_REC_CLIPBOARDOPTIONS rec;
  BOOL bRet = TRUE;
  if( TRUE )
  {
    rec.nRecType = SS_RID_CLIPBOARDOPTIONS;
    rec.lRecLen = sizeof(rec);
    rec.bOptions = lpBook->bClipboardOptions;
    bRet = SS_BuffAppend(lpBuff, &rec, sizeof(rec));
  }
  return bRet;
}

//--------------------------------------------------------------------
//
//  The SS_Load4ColHidden() function loads the Col Hidden info
//

BOOL SS_Load4ColHidden(LPSPREADSHEET lpSS, LPSS_REC_COMMON lpCommon)
{
  LPSS_REC_COLHIDDEN lpRec = (LPSS_REC_COLHIDDEN)lpCommon;
  BOOL bRet = sizeof(*lpRec) == lpRec->lRecLen;
  if( bRet )
  {
    SS_ShowCol(lpSS, lpRec->lCol, !lpRec->fHidden);
  }
  return bRet;
}

//--------------------------------------------------------------------
//
//  The SS_Save4ColHidden() function saves the Col Hidden info
//

BOOL SS_Save4ColHidden(SS_COORD lCol, BOOL fHidden, LPSS_BUFF lpBuff)
{
  SS_REC_COLHIDDEN rec;
  BOOL bRet = TRUE;
  // RFW - 12/17/03
  if( fHidden )
  {
    rec.nRecType = SS_RID_COLHIDDEN;
    rec.lRecLen = sizeof(rec);
    rec.lCol = lCol;
    rec.fHidden = (BYTE)fHidden;
    bRet = SS_BuffAppend(lpBuff, &rec, sizeof(rec));
  }
  return bRet;
}

//--------------------------------------------------------------------
//
//  The SS_Load4RowHidden() function loads the Col Hidden info
//

BOOL SS_Load4RowHidden(LPSPREADSHEET lpSS, LPSS_REC_COMMON lpCommon)
{
  LPSS_REC_ROWHIDDEN lpRec = (LPSS_REC_ROWHIDDEN)lpCommon;
  BOOL bRet = sizeof(*lpRec) == lpRec->lRecLen;
  if( bRet )
  {
    SS_ShowRow(lpSS, lpRec->lRow, !lpRec->fHidden);
  }
  return bRet;
}

//--------------------------------------------------------------------
//
//  The SS_Save4RowHidden() function saves the Col Hidden info
//

BOOL SS_Save4RowHidden(SS_COORD lRow, BOOL fHidden, LPSS_BUFF lpBuff)
{
  SS_REC_ROWHIDDEN rec;
  BOOL bRet = TRUE;
  // RFW - 12/17/03
  if( fHidden )
  {
    rec.nRecType = SS_RID_ROWHIDDEN;
    rec.lRecLen = sizeof(rec);
    rec.lRow = lRow;
    rec.fHidden = (BYTE)fHidden;
    bRet = SS_BuffAppend(lpBuff, &rec, sizeof(rec));
  }
  return bRet;
}

//--------------------------------------------------------------------
//
//  The SS_Load4TwoDigitYearMax() function loads the 2 digit year max value.
//

BOOL SS_Load4TwoDigitYearMax(LPSS_BOOK lpBook, LPSS_REC_COMMON lpCommon)
{
  LPSS_REC_TWODIGITYEARMAX lpRec = (LPSS_REC_TWODIGITYEARMAX)lpCommon;
  BOOL bRet = sizeof(*lpRec) == lpRec->lRecLen;
  if( bRet )
  {
    lpBook->nTwoDigitYearMax = lpRec->nTwoDigitYearMax;
  }
  return bRet;
}

//--------------------------------------------------------------------
//
//  The SS_Save4TwoDigitYearMax() function saves the 2 digit year max value.
//

BOOL SS_Save4TwoDigitYearMax(LPSS_BOOK lpBook, LPSS_BUFF lpBuff)
{
  SS_REC_TWODIGITYEARMAX rec;
  BOOL bRet = TRUE;
  if( TRUE )
  {
    rec.nRecType = SS_RID_TWODIGITYEARMAX;
    rec.lRecLen = sizeof(rec);
    rec.nTwoDigitYearMax = lpBook->nTwoDigitYearMax;
    bRet = SS_BuffAppend(lpBuff, &rec, sizeof(rec));
  }
  return bRet;
}

#endif SS_V30

//--------------------------------------------------------------------
//
//  The SS_Save4Xtra() function saves the lpSS->hXtra buffer.  The
//  lpSS->hXtra buffer can contain the following records...
//      SS_REC_CALCDEPEND
//      SS_REC_DATAAWARE
//      SS_REC_DATAAWARECOL
//      SS_REC_DATAAWAREQE
//      SS_REC_PRINT
//

BOOL SS_Save4Xtra(LPSS_BOOK lpBook, LPSS_BUFF lpBuff, BYTE bFileVer)
{
  LPBYTE lpXtra;
  BOOL bRet = TRUE;
  if( lpBook->hXtra && lpBook->lXtraLen && lpBook->bXtraVer == bFileVer )
  {
    lpXtra = GlobalLock(lpBook->hXtra);
    bRet = SS_BuffAppend(lpBuff, lpXtra, lpBook->lXtraLen);
    GlobalUnlock(lpBook->hXtra);
  }
  return bRet;
}

#ifdef SS_V40

//--------------------------------------------------------------------
//
//  The SS_Load4CellSpan() function loads the list of cell spans
//

BOOL SS_Load4CellSpanTable(LPSPREADSHEET lpSS, LPSS_REC_COMMON lpCommon)
{
  LPSS_REC_CELLSPANTABLE lpRec = (LPSS_REC_CELLSPANTABLE)lpCommon;
  LPSS_SUBREC_CELLSPAN lpSubRec;
  int i;
  BOOL bRet = sizeof(*lpRec) <= lpRec->lRecLen;
  if( bRet )
  {
    lpSubRec = (LPSS_SUBREC_CELLSPAN)((HPBYTE)lpRec + sizeof(*lpRec));
    for( i = 0; i < lpRec->nTableCnt; i++, lpSubRec++ )
    {
      SS_AddCellSpan(lpSS, lpSubRec->lCol, lpSubRec->lRow, lpSubRec->lNumCols, lpSubRec->lNumRows);
    }
  }
  return bRet;  
}

//--------------------------------------------------------------------
//
//  The SS_Save4CellSpanTable() saves the list of CellSpans currently being
//  used in the spreadsheet.
//

BOOL SS_Save4CellSpanTable(LPSPREADSHEET lpSS, LPSS_BUFF lpBuff)
{
  SS_REC_CELLSPANTABLE rec;
  SS_SUBREC_CELLSPAN subrec;
  TBGLOBALHANDLE hNext;
  TBGLOBALHANDLE hNextTemp;
  LPSS_CELLSPAN  lpNext;
  BOOL bRet = TRUE;

  if( lpSS->hCellSpan )
  {
    rec.nRecType = SS_RID_CELLSPANTABLE;
    rec.lRecLen = sizeof(rec);
    rec.nTableCnt = 0;

    for( hNext = lpSS->hCellSpan; hNext; )
    {
      lpNext = (LPSS_CELLSPAN)tbGlobalLock(hNext);
      hNextTemp = lpNext->hNext;
      tbGlobalUnlock(hNext);
      hNext = hNextTemp;
      rec.nTableCnt++;
    }
    rec.lRecLen += (rec.nTableCnt * sizeof(subrec));
    bRet = SS_BuffAppend(lpBuff, &rec, sizeof(rec));

    for( hNext = lpSS->hCellSpan; hNext && bRet; )
    {
      lpNext = (LPSS_CELLSPAN)tbGlobalLock(hNext);
      subrec.lCol = lpNext->lCol;
      subrec.lRow = lpNext->lRow;
      subrec.lNumCols = lpNext->lNumCols;
      subrec.lNumRows = lpNext->lNumRows;
      hNextTemp = lpNext->hNext;
      tbGlobalUnlock(hNext);
      hNext = hNextTemp;
      bRet &= SS_BuffAppend(lpBuff, &subrec, sizeof(subrec));
    }
  }
  return bRet;
}

//--------------------------------------------------------------------

BOOL SS_Load4ColMerge(LPSPREADSHEET lpSS, LPSS_REC_COMMON lpCommon)
{
  LPSS_REC_COLMERGE lpRec = (LPSS_REC_COLMERGE)lpCommon;
  BOOL bRet = sizeof(*lpRec) == lpRec->lRecLen;
  if( bRet )
  {
    SS_SetColMerge(lpSS, lpRec->lCol, lpRec->bMerge);
  }
  return bRet;
}

//--------------------------------------------------------------------

BOOL SS_Save4ColMerge(SS_COORD lCol, BYTE bColMerge, LPSS_BUFF lpBuff)
{
  SS_REC_COLMERGE rec;
  BOOL bRet = TRUE;
  if( bColMerge != 0 )
  {
    rec.nRecType = SS_RID_COLMERGE;
    rec.lRecLen = sizeof(rec);
    rec.lCol = lCol;
    rec.bMerge = bColMerge;
    bRet = SS_BuffAppend(lpBuff, &rec, sizeof(rec));
  }
  return bRet;
}

//--------------------------------------------------------------------

BOOL SS_Load4ColID(LPSPREADSHEET lpSS, LPSS_REC_COMMON lpCommon,
                     BOOL bUnicode)
{
  LPSS_REC_COLID lpRec = (LPSS_REC_COLID)lpCommon;
  HPBYTE lpText = NULL;
  LPTSTR lpszText = NULL;
  BOOL bRet = sizeof(*lpRec) <= lpRec->lRecLen;
  if( bRet )
  {
    SS_BUFF buff;
    SS_BuffInit(&buff);
    lpText = (HPBYTE)lpRec + sizeof(*lpRec);
    if( sizeof(*lpRec) <= lpRec->lRecLen )
      lpszText = SS_BuffCopyStr(&buff, lpText, bUnicode);
    SS_SetColID(lpSS, lpRec->lCol, lpszText);
    SS_BuffFree(&buff);
  }
  return bRet;
}

//--------------------------------------------------------------------

BOOL SS_Save4ColID(SS_COORD lCol, TBGLOBALHANDLE hColID, LPSS_BUFF lpBuff)
{
  SS_REC_COLID rec;
  LPTSTR lpszText;
  long lTextLen;
  BOOL bRet = TRUE;

  if( hColID )
  {
    lpszText = (LPTSTR)tbGlobalLock(hColID);
    lTextLen = lstrlen(lpszText) + 1;
    rec.nRecType = SS_RID_COLID;
    rec.lRecLen = sizeof(rec) + lTextLen * sizeof(TCHAR);
    rec.lCol = lCol;
    bRet = SS_BuffAppend(lpBuff, &rec, sizeof(rec));
    bRet &= SS_BuffAppend(lpBuff, lpszText, lTextLen * sizeof(TCHAR));
    tbGlobalUnlock(hColID);
  }
  return bRet;
}

//--------------------------------------------------------------------

BOOL SS_Load4Header4(LPSPREADSHEET lpSS, LPSS_REC_COMMON lpCommon)
{
  LPSS_REC_HEADER4 lpRec = (LPSS_REC_HEADER4)lpCommon;
  BOOL bRet = sizeof(*lpRec) == lpRec->lRecLen;
  if( bRet )
  { // 26544 -scl
    BOOL fEditModePermanent = lpSS->lpBook->fEditModePermanent;
    lpSS->lpBook->fEditModePermanent = FALSE;
    SSx_SetRowHeaderCols(lpSS, lpRec->lRowHeaderCols);
    lpSS->Col.lHeaderDisplayIndex = lpRec->lRowHeadersAutoTextIndex;
    SSx_SetColHeaderRows(lpSS, lpRec->lColHeaderRows);
    lpSS->Row.lHeaderDisplayIndex = lpRec->lColHeadersAutoTextIndex;
    lpSS->lColHeadersUserSortIndex = lpRec->lColHeadersUserSortIndex;
    lpSS->lpBook->fEditModePermanent = fEditModePermanent;
  }
  return bRet;
}

//--------------------------------------------------------------------

BOOL SS_Save4Header4(LPSPREADSHEET lpSS, LPSS_BUFF lpBuff)
{
  SS_REC_HEADER4 rec;
  BOOL bRet = TRUE;
  if( 1 != lpSS->Col.HeaderCnt ||
      -1 != lpSS->Col.lHeaderDisplayIndex ||
      1 != lpSS->Row.HeaderCnt ||
      -1 != lpSS->Row.lHeaderDisplayIndex ||
      -1 != lpSS->lColHeadersUserSortIndex )
  {
    rec.nRecType = SS_RID_HEADER4;
    rec.lRecLen = sizeof(rec);
    rec.lRowHeaderCols = lpSS->Col.HeaderCnt;
    rec.lRowHeadersAutoTextIndex = lpSS->Col.lHeaderDisplayIndex;
    rec.lColHeaderRows = lpSS->Row.HeaderCnt;
    rec.lColHeadersAutoTextIndex = lpSS->Row.lHeaderDisplayIndex;
    rec.lColHeadersUserSortIndex = lpSS->lColHeadersUserSortIndex;
    bRet = SS_BuffAppend(lpBuff, &rec, sizeof(rec));
  }
  return bRet;
}

//--------------------------------------------------------------------

BOOL SS_Load4RowMerge(LPSPREADSHEET lpSS, LPSS_REC_COMMON lpCommon)
{
  LPSS_REC_ROWMERGE lpRec = (LPSS_REC_ROWMERGE)lpCommon;
  BOOL bRet = sizeof(*lpRec) == lpRec->lRecLen;
  if( bRet )
  {
    SS_SetRowMerge(lpSS, lpRec->lRow, lpRec->bMerge);
  }
  return bRet;
}

//--------------------------------------------------------------------

BOOL SS_Save4RowMerge(SS_COORD lRow, BYTE bRowMerge, LPSS_BUFF lpBuff)
{
  SS_REC_ROWMERGE rec;
  BOOL bRet = TRUE;
  if( bRowMerge != 0 )
  {
    rec.nRecType = SS_RID_ROWMERGE;
    rec.lRecLen = sizeof(rec);
    rec.lRow = lRow;
    rec.bMerge = bRowMerge;
    bRet = SS_BuffAppend(lpBuff, &rec, sizeof(rec));
  }
  return bRet;
}

//--------------------------------------------------------------------

BOOL SS_Load4Display4(LPSS_BOOK lpBook, LPSS_REC_COMMON lpCommon)
{
  LPSS_REC_DISPLAY4 lpRec = (LPSS_REC_DISPLAY4)lpCommon;
  BOOL bRet = sizeof(*lpRec) == lpRec->lRecLen;
  if( bRet )
  {
    lpBook->fEditOverflow = lpRec->bAllowEditOverflow;
  }
  return bRet;
}

//--------------------------------------------------------------------

BOOL SS_Save4Display4(LPSS_BOOK lpBook, LPSS_BUFF lpBuff)
{
  SS_REC_DISPLAY4 rec;
  BOOL bRet = TRUE;
  if( lpBook->fEditOverflow)
  {
    rec.nRecType = SS_RID_DISPLAY4;
    rec.lRecLen = sizeof(rec);
    rec.bAllowEditOverflow = lpBook->fEditOverflow;
    bRet = SS_BuffAppend(lpBuff, &rec, sizeof(rec));
  }
  return bRet;
}
//--------------------------------------------------------------------

BOOL SS_Load4CellTag(LPSPREADSHEET lpSS, LPSS_REC_COMMON lpCommon,
                     BOOL bUnicode, LPSS_LOADBLOCK lpLoadBlock)
{
  LPSS_REC_CELLTAG lpRec = (LPSS_REC_CELLTAG)lpCommon;
  HPBYTE lpText = NULL;
  LPTSTR lpszText = NULL;
  SS_COORD lCol, lRow;
  BOOL bRet = sizeof(*lpRec) <= lpRec->lRecLen;
  if( bRet &&
      BlockTranslate(lpLoadBlock, lpRec->lCol, lpRec->lRow, &lCol, &lRow) )
  {
    SS_BUFF buff;
    SS_BuffInit(&buff);
    lpText = (HPBYTE)lpRec + sizeof(*lpRec);
    if( sizeof(*lpRec) <= lpRec->lRecLen )
      lpszText = SS_BuffCopyStr(&buff, lpText, bUnicode);
    SS_SetCellTagRange(lpSS, lCol, lRow, lCol, lRow, lpszText);

    SS_BuffFree(&buff);
  }
  return bRet;
}

//--------------------------------------------------------------------

BOOL SS_Save4CellTag(SS_COORD lCol, SS_COORD lRow, TBGLOBALHANDLE ghCellTag,
                      LPSS_BUFF lpBuff)
{
  SS_REC_CELLTAG rec;
  LPTSTR lpszText;
  long lTextLen;
  BOOL bRet = TRUE;
  if( ghCellTag )
      {
        lpszText = (LPTSTR)tbGlobalLock(ghCellTag);
        lTextLen = lstrlen(lpszText) + 1;
        rec.nRecType = SS_RID_CELLTAG;
        rec.lRecLen = sizeof(rec) + (lTextLen * sizeof(TCHAR));
        rec.lCol = lCol;
        rec.lRow = lRow;
        bRet = SS_BuffAppend(lpBuff, &rec, sizeof(rec));
        bRet &= SS_BuffAppend(lpBuff, lpszText, lTextLen * sizeof(TCHAR));
        tbGlobalUnlock(ghCellTag);
      }
  return bRet;
}

//--------------------------------------------------------------------

#endif // SS_V40

#ifdef SS_V70

//--------------------------------------------------------------------
//
//  The SS_Load4SheetIndex() function loads the current sheet index.
//

BOOL SS_Load4SheetIndex(LPSS_REC_COMMON lpCommon, short *lpnSheet)
{
  LPSS_REC_SHEETINDEX lpRec = (LPSS_REC_SHEETINDEX)lpCommon;
  BOOL bRet = sizeof(*lpRec) == lpRec->lRecLen;
  if( bRet )
    *lpnSheet = lpRec->nSheet;
  return bRet;
}

//--------------------------------------------------------------------
//
//  The SS_Save4SheetIndex() function stores the current sheet index.
//

BOOL SS_Save4SheetIndex(LPSS_BUFF lpBuff, short nSheet)
{
  SS_REC_SHEETINDEX rec;
  BOOL bRet = TRUE;
  if( TRUE )
  {
    rec.nRecType = SS_RID_SHEETINDEX;
    rec.lRecLen = sizeof(rec);
    rec.nSheet = nSheet;
    bRet = SS_BuffAppend(lpBuff, &rec, sizeof(rec));
  }
  return bRet;
}

//--------------------------------------------------------------------
//
//  The SS_Load4ActiveSheet() function loads the current sheet index.
//

/* RFW - I decided not to save the ActiveSheet value

BOOL SS_Load4ActiveSheet(LPSS_BOOK lpBook, LPSS_REC_COMMON lpCommon)
{
  LPSS_REC_ACTIVESHEET lpRec = (LPSS_REC_ACTIVESHEET)lpCommon;
  BOOL bRet = sizeof(*lpRec) == lpRec->lRecLen;
  if( bRet )
    lpBook->nActiveSheet = lpRec->nActiveSheet;
  return bRet;
}

//--------------------------------------------------------------------
//
//  The SS_Save4ActiveSheet() function stores the current sheet index.
//

BOOL SS_Save4ActiveSheet(LPSS_BOOK lpBook, LPSS_BUFF lpBuff)
{
  SS_REC_ACTIVESHEET rec;
  BOOL bRet = TRUE;
  if( TRUE )
  {
    rec.nRecType = SS_RID_ACTIVESHEET;
    rec.lRecLen = sizeof(rec);
    rec.nActiveSheet = lpBook->nActiveSheet;
    bRet = SS_BuffAppend(lpBuff, &rec, sizeof(rec));
  }
  return bRet;
}
*/

//--------------------------------------------------------------------
//
//  The SS_Load4SheetCount() function loads the sheet count.
//

BOOL SS_Load4SheetCount(LPSS_BOOK lpBook, LPSS_REC_COMMON lpCommon)
{
  LPSS_REC_SHEETCOUNT lpRec = (LPSS_REC_SHEETCOUNT)lpCommon;
  BOOL bRet = sizeof(*lpRec) == lpRec->lRecLen;
  if( bRet )
    SS_SetSheetCount(lpBook, lpRec->nSheetCount);
  return bRet;
}

//--------------------------------------------------------------------
//
//  The SS_Save4SheetCount() function stores the sheet count.
//

BOOL SS_Save4SheetCount(LPSS_BOOK lpBook, LPSS_BUFF lpBuff)
{
  SS_REC_SHEETCOUNT rec;
  BOOL bRet = TRUE;
  if( TRUE )
  {
    rec.nRecType = SS_RID_SHEETCOUNT;
    rec.lRecLen = sizeof(rec);
    rec.nSheetCount = lpBook->nSheetCnt;
    bRet = SS_BuffAppend(lpBuff, &rec, sizeof(rec));
  }
  return bRet;
}

//--------------------------------------------------------------------
//
//  The SS_Load4TabStrip() function loads the sheet count.
//

BOOL SS_Load4TabStrip(LPSS_BOOK lpBook, LPSS_REC_COMMON lpCommon, LPSS_FONTBUFFER lpFontBuff)
{
  LPSS_REC_TABSTRIP lpRec = (LPSS_REC_TABSTRIP)lpCommon;
  BOOL bRet = sizeof(*lpRec) == lpRec->lRecLen;
  if( bRet )
  {
//    lpBook->nTabStripLeftSheet = lpRec->nTabStripLeftSheet;
    lpBook->wTabStripPolicy = lpRec->wTabStripPolicy;
    lpBook->dfTabStripRatio = lpRec->dfTabStripRatio;
    if (-1 != lpRec->nFontId && lpFontBuff->phFont)
      SS_SetTabStripFont(lpBook, lpFontBuff->phFont[lpRec->nFontId]);
    SS_EmptyTabStripRect(lpBook);
  }

  return bRet;
}

#ifdef SS_V80
//--------------------------------------------------------------------
//
//  The SS_Load4AppearanceStyle() function loads the sheet count.
//

BOOL SS_Load4AppearanceStyle(LPSS_BOOK lpBook, LPSS_REC_COMMON lpCommon)
{
  LPSS_REC_APPEARANCESTYLE lpRec = (LPSS_REC_APPEARANCESTYLE)lpCommon;
  BOOL bRet = sizeof(*lpRec) == lpRec->lRecLen;
  if( bRet )
  {
    SS_SetAppearanceStyle(lpBook, lpRec->wAppearanceStyle);
  }

  return bRet;
}

//--------------------------------------------------------------------
//
//  The SS_Save4AppearanceStyle() function stores the sheet count.
//

BOOL SS_Save4AppearanceStyle(LPSS_BOOK lpBook, LPSS_BUFF lpBuff)
{
  SS_REC_APPEARANCESTYLE rec;
  BOOL bRet = TRUE;
  if( TRUE )
  {
    rec.nRecType = SS_RID_APPEARANCESTYLE;
    rec.lRecLen = sizeof(rec);
//    rec.nTabStripLeftSheet = lpBook->nTabStripLeftSheet;
    rec.wAppearanceStyle = lpBook->wAppearanceStyle;
    bRet = SS_BuffAppend(lpBuff, &rec, sizeof(rec));
  }
  return bRet;
}

//--------------------------------------------------------------------
//
//  The SS_Load4EnhancedColors() function loads the sheet colors.
//

BOOL SS_Load4EnhancedColors(LPSS_BOOK lpBook, LPSS_REC_COMMON lpCommon)
{
  LPSS_REC_ENHANCEDCOLORS lpRec = (LPSS_REC_ENHANCEDCOLORS)lpCommon;
  BOOL bRet = sizeof(*lpRec) == lpRec->lRecLen;
  if( bRet )
  {
 
  SS_SetEnhancedSheetTabColors(lpBook, lpRec->sheetTabUpperNormalStartColor, lpRec->sheetTabUpperNormalEndColor,
								    lpRec->sheetTabLowerNormalStartColor,  lpRec->sheetTabLowerNormalEndColor,  lpRec->sheetTabUpperHoverStartColor,
								    lpRec->sheetTabUpperHoverEndColor,  lpRec->sheetTabLowerHoverStartColor,  lpRec->sheetTabLowerHoverEndColor,
								    lpRec->sheetTabOuterBorderColor,  lpRec->sheetTabInnerBorderColor,  lpRec->sheetScrollUpperNormalStartColor,
								    lpRec->sheetScrollUpperNormalEndColor,  lpRec->sheetScrollLowerNormalStartColor,  lpRec->sheetScrollLowerNormalEndColor,
								    lpRec->sheetScrollUpperHoverStartColor,  lpRec->sheetScrollUpperHoverEndColor,  lpRec->sheetScrollLowerHoverStartColor,
								    lpRec->sheetScrollLowerHoverEndColor, lpRec->sheetScrollUpperPushedStartColor,  lpRec->sheetScrollUpperPushedEndColor,
								    lpRec->sheetScrollLowerPushedStartColor,  lpRec->sheetScrollLowerPushedEndColor,  lpRec->sheetScrollArrowColor,
								    lpRec->sheetGrayAreaStartColor,  lpRec->sheetGrayAreaEndColor,  lpRec->sheetSplitBoxStartColor,
								    lpRec->sheetSplitBoxEndColor,  lpRec->sheetTabForeColor,  lpRec->sheetTabActiveForeColor);

 SS_SetEnhancedScrollBarColors(lpBook, lpRec->scrollTrackColor, lpRec->scrollArrowColor, lpRec->scrollUpperNormalStartColor, lpRec->scrollUpperNormalEndColor,
										   lpRec->scrollLowerNormalStartColor, lpRec->scrollLowerNormalEndColor, lpRec->scrollUpperPushedStartColor, lpRec->scrollUpperPushedEndColor,
										   lpRec->scrollLowerPushedStartColor, lpRec->scrollLowerPushedEndColor, lpRec->scrollUpperHoverStartColor, lpRec->scrollUpperHoverEndColor,
										   lpRec->scrollLowerHoverStartColor, lpRec->scrollLowerHoverEndColor, lpRec->scrollHoverBorderColor, lpRec->scrollBorderColor);

 SS_SetEnhancedColumnHeaderColors(lpBook, lpRec->colSelectedHoverUpperColor, lpRec->colSelectedHoverLowerColor, lpRec->colSelectedUpperColor,
											  lpRec->colSelectedLowerColor, lpRec->colHoverUpperColor, lpRec->colHoverLowerColor, lpRec->colUpperColor,
											  lpRec->colLowerColor, lpRec->colHeaderSelectedBorderColor, lpRec->colHeaderBorderColor);

 SS_SetEnhancedCornerColors(lpBook, lpRec->cornerBackColor, lpRec->cornerHoverColor, lpRec->cornerTriangleColor, lpRec->cornerTriangleHoverColor, lpRec->cornerTriangleBorderColor, lpRec->cornerTriangleHoverBorderColor);

 SS_SetEnhancedRowHeaderColors(lpBook, lpRec->rowSelectedHoverUpperColor, lpRec->rowSelectedHoverLowerColor, lpRec->rowSelectedUpperColor,
											  lpRec->rowSelectedLowerColor, lpRec->rowHoverUpperColor, lpRec->rowHoverLowerColor, lpRec->rowUpperColor,
											  lpRec->rowLowerColor, lpRec->rowHeaderSelectedBorderColor, lpRec->rowHeaderBorderColor);
}

  return bRet;
}

//--------------------------------------------------------------------
//
//  The SS_Save4EnhancedColors() function stores the sheet colors.
//

BOOL SS_Save4EnhancedColors(LPSS_BOOK lpBook, LPSS_BUFF lpBuff)
{
  SS_REC_ENHANCEDCOLORS rec;
  BOOL bRet = TRUE;
  if( TRUE )
  {
    rec.nRecType = SS_RID_ENHANCEDCOLORS;
    rec.lRecLen = sizeof(rec);
	rec.scrollTrackColor = lpBook->scrollTrackColor;
	rec.scrollArrowColor = lpBook->scrollArrowColor;
	rec.scrollBorderColor = lpBook->scrollBorderColor;
	rec.scrollUpperNormalStartColor = lpBook->scrollUpperNormalStartColor ;
	rec.scrollUpperNormalEndColor = lpBook->scrollUpperNormalEndColor;
	rec.scrollLowerNormalStartColor = lpBook->scrollLowerNormalStartColor;
	rec.scrollLowerNormalEndColor = lpBook->scrollLowerNormalEndColor;
	rec.scrollUpperHoverStartColor = lpBook->scrollUpperHoverStartColor;
	rec.scrollUpperHoverEndColor = lpBook->scrollUpperHoverEndColor;
	rec.scrollLowerHoverStartColor = lpBook->scrollLowerHoverStartColor;
	rec.scrollLowerHoverEndColor = lpBook->scrollLowerHoverEndColor;
	rec.scrollUpperPushedStartColor = lpBook->scrollUpperPushedStartColor;
	rec.scrollUpperPushedEndColor = lpBook->scrollUpperPushedEndColor;
	rec.scrollLowerPushedStartColor = lpBook->scrollLowerPushedStartColor;
	rec.scrollLowerPushedEndColor = lpBook->scrollLowerPushedEndColor;
	rec.scrollHoverBorderColor = lpBook->scrollHoverBorderColor;
	rec.rowHeaderSelectedBorderColor = lpBook->rowHeaderSelectedBorderColor ;
	rec.rowHeaderBorderColor = lpBook->rowHeaderBorderColor;
	rec.rowHoverUpperColor = lpBook->rowHoverUpperColor;
	rec.rowHoverLowerColor = lpBook->rowHoverLowerColor;
	rec.rowSelectedHoverUpperColor = lpBook->rowSelectedHoverUpperColor;
	rec.rowSelectedHoverLowerColor = lpBook->rowSelectedHoverLowerColor;
	rec.rowSelectedUpperColor = lpBook->rowSelectedUpperColor;
	rec.rowSelectedLowerColor = lpBook->rowSelectedLowerColor;
	rec.rowUpperColor = lpBook->rowUpperColor;
	rec.rowLowerColor = lpBook->rowLowerColor;
	rec.cornerBackColor = lpBook->cornerBackColor ;
	rec.cornerHoverColor = lpBook->cornerHoverColor ;
	rec.cornerTriangleColor = lpBook->cornerTriangleColor;
	rec.cornerTriangleHoverColor = lpBook->cornerTriangleHoverColor;
	rec.cornerTriangleBorderColor = lpBook->cornerTriangleBorderColor;
	rec.cornerTriangleHoverBorderColor = lpBook->cornerTriangleHoverBorderColor;
	rec.colHeaderSelectedBorderColor = lpBook->colHeaderSelectedBorderColor;
	rec.colHeaderBorderColor = lpBook->colHeaderBorderColor;
	rec.colHoverUpperColor = lpBook->colHoverUpperColor;
	rec.colHoverLowerColor = lpBook->colHoverLowerColor;
	rec.colSelectedHoverUpperColor = lpBook->colSelectedHoverUpperColor;
	rec.colSelectedHoverLowerColor = lpBook->colSelectedHoverLowerColor;
	rec.colSelectedUpperColor = lpBook->colSelectedUpperColor;
	rec.colSelectedLowerColor = lpBook->colSelectedLowerColor;
	rec.colUpperColor = lpBook->colUpperColor;
	rec.colLowerColor = lpBook->colLowerColor;
	rec.sheetTabUpperNormalStartColor = lpBook->sheetTabUpperNormalStartColor;
	rec.sheetTabUpperNormalEndColor = lpBook->sheetTabUpperNormalEndColor;
	rec.sheetTabLowerNormalStartColor = lpBook->sheetTabLowerNormalStartColor;
	rec.sheetTabLowerNormalEndColor = lpBook->sheetTabLowerNormalEndColor;
	rec.sheetTabUpperHoverStartColor = lpBook->sheetTabUpperHoverStartColor;
	rec.sheetTabUpperHoverEndColor = lpBook->sheetTabUpperHoverEndColor;
	rec.sheetTabLowerHoverStartColor = lpBook->sheetTabLowerHoverStartColor;
	rec.sheetTabLowerHoverEndColor = lpBook->sheetTabLowerHoverEndColor;
	rec.sheetTabInnerBorderColor = lpBook->sheetTabInnerBorderColor;
	rec.sheetTabOuterBorderColor = lpBook->sheetTabOuterBorderColor;
	rec.sheetGrayAreaStartColor = lpBook->sheetGrayAreaStartColor;
	rec.sheetGrayAreaEndColor = lpBook->sheetGrayAreaEndColor;
	rec.sheetSplitBoxStartColor = lpBook->sheetSplitBoxStartColor;
	rec.sheetSplitBoxEndColor = lpBook->sheetSplitBoxEndColor;
	rec.sheetScrollArrowColor = lpBook->sheetScrollArrowColor ;
	rec.sheetScrollUpperNormalStartColor = lpBook->sheetScrollUpperNormalStartColor;
	rec.sheetScrollUpperNormalEndColor = lpBook->sheetScrollUpperNormalEndColor;
	rec.sheetScrollLowerNormalStartColor = lpBook->sheetScrollLowerNormalStartColor;
	rec.sheetScrollLowerNormalEndColor = lpBook->sheetScrollLowerNormalEndColor;
	rec.sheetScrollUpperHoverStartColor = lpBook->sheetScrollUpperHoverStartColor;
	rec.sheetScrollUpperHoverEndColor = lpBook->sheetScrollUpperHoverEndColor;
	rec.sheetScrollLowerHoverStartColor = lpBook->sheetScrollLowerHoverStartColor;
	rec.sheetScrollLowerHoverEndColor = lpBook->sheetScrollLowerHoverEndColor;
	rec.sheetScrollUpperPushedStartColor = lpBook->sheetScrollUpperPushedStartColor;
	rec.sheetScrollUpperPushedEndColor = lpBook->sheetScrollUpperPushedEndColor;
	rec.sheetScrollLowerPushedStartColor = lpBook->sheetScrollLowerPushedStartColor;
	rec.sheetScrollLowerPushedEndColor = lpBook->sheetScrollLowerPushedEndColor;
	rec.sheetTabForeColor = lpBook->sheetTabForeColor;
	rec.sheetTabActiveForeColor = lpBook->sheetTabActiveForeColor;

    bRet = SS_BuffAppend(lpBuff, &rec, sizeof(rec));
  }
  return bRet;
}
//
//  The SS_Load4AppearanceStyle() function loads the sheet count.
//

BOOL SS_Load4ScrollBarStyle(LPSS_BOOK lpBook, LPSS_REC_COMMON lpCommon)
{
  LPSS_REC_SCROLLBARSTYLE lpRec = (LPSS_REC_SCROLLBARSTYLE)lpCommon;
  BOOL bRet = sizeof(*lpRec) == lpRec->lRecLen;
  if( bRet )
  {
    SS_SetScrollBarStyle(lpBook, lpRec->wScrollBarStyle);
  }

  return bRet;
}

//--------------------------------------------------------------------
//
//  The SS_Save4AppearanceStyle() function stores the sheet count.
//

BOOL SS_Save4ScrollBarStyle(LPSS_BOOK lpBook, LPSS_BUFF lpBuff)
{
  SS_REC_SCROLLBARSTYLE rec;
  BOOL bRet = TRUE;
  if( TRUE )
  {
    rec.nRecType = SS_RID_SCROLLBARSTYLE;
    rec.lRecLen = sizeof(rec);
//    rec.nTabStripLeftSheet = lpBook->nTabStripLeftSheet;
    rec.wScrollBarStyle = lpBook->wScrollBarStyle;
    bRet = SS_BuffAppend(lpBuff, &rec, sizeof(rec));
  }
  return bRet;
}

//--------------------------------------------------------------------
//
//  The SS_Load4UseVisualStyles() function loads the sheet count.
//

BOOL SS_Load4UseVisualStyles(LPSS_BOOK lpBook, LPSS_REC_COMMON lpCommon)
{
  LPSS_REC_USEVISUALSTYLES lpRec = (LPSS_REC_USEVISUALSTYLES)lpCommon;
  BOOL bRet = sizeof(*lpRec) == lpRec->lRecLen;
  if( bRet )
  {
    SS_SetUseVisualStyles(lpBook, lpRec->wUseVisualStyles);
  }

  return bRet;
}

//--------------------------------------------------------------------
//
//  The SS_Save4AppearanceStyle() function stores the sheet count.
//

BOOL SS_Save4UseVisualStyles(LPSS_BOOK lpBook, LPSS_BUFF lpBuff)
{
  SS_REC_USEVISUALSTYLES rec;
  BOOL bRet = TRUE;
  if( TRUE )
  {
    rec.nRecType = SS_RID_USEVISUALSTYLES;
    rec.lRecLen = sizeof(rec);
//    rec.nTabStripLeftSheet = lpBook->nTabStripLeftSheet;
    rec.wUseVisualStyles = lpBook->wUseVisualStyles;
    bRet = SS_BuffAppend(lpBuff, &rec, sizeof(rec));
  }
  return bRet;
}


//--------------------------------------------------------------------
//
//  The SS_Load4EnhanceStaticCells() function loads the sheet count.
//

BOOL SS_Load4EnhanceStaticCells(LPSS_BOOK lpBook, LPSS_REC_COMMON lpCommon)
{
  LPSS_REC_ENHANCESTATICCELLS lpRec = (LPSS_REC_ENHANCESTATICCELLS)lpCommon;
  BOOL bRet = sizeof(*lpRec) == lpRec->lRecLen;
  if( bRet )
  {
    SS_SetEnhanceStaticCells(lpBook, lpRec->bEnhanceStaticCells);
  }

  return bRet;
}

//--------------------------------------------------------------------
//
//  The SS_Save4EnhanceStaticCells() function stores the sheet count.
//

BOOL SS_Save4EnhanceStaticCells(LPSS_BOOK lpBook, LPSS_BUFF lpBuff)
{
  SS_REC_ENHANCESTATICCELLS rec;
  BOOL bRet = TRUE;
  if( TRUE )
  {
    rec.nRecType = SS_RID_ENHANCESTATICCELLS;
    rec.lRecLen = sizeof(rec);
//    rec.nTabStripLeftSheet = lpBook->nTabStripLeftSheet;
    rec.bEnhanceStaticCells = lpBook->bEnhanceStaticCells;
    bRet = SS_BuffAppend(lpBuff, &rec, sizeof(rec));
  }
  return bRet;
}
//--------------------------------------------------------------------
//
//  The SS_Load4TabStripButtonPolicy() function loads the sheet count.
//

BOOL SS_Load4TabStripButtonPolicy(LPSS_BOOK lpBook, LPSS_REC_COMMON lpCommon)
{
  LPSS_REC_TABSTRIPBUTTONPOLICY lpRec = (LPSS_REC_TABSTRIPBUTTONPOLICY)lpCommon;
  BOOL bRet = sizeof(*lpRec) == lpRec->lRecLen;
  if( bRet )
  {
    SS_SetTabStripButtonPolicy(lpBook, lpRec->wTabStripButtonPolicy);
  }

  return bRet;
}

//--------------------------------------------------------------------
//
//  The SS_Save4TabStripButtonPolicye() function stores the sheet count.
//

BOOL SS_Save4TabStripButtonPolicy(LPSS_BOOK lpBook, LPSS_BUFF lpBuff)
{
  SS_REC_TABSTRIPBUTTONPOLICY rec;
  BOOL bRet = TRUE;
  if( TRUE )
  {
    rec.nRecType = SS_RID_TABSTRIPBUTTONPOLICY;
    rec.lRecLen = sizeof(rec);
//    rec.nTabStripLeftSheet = lpBook->nTabStripLeftSheet;
    rec.wTabStripButtonPolicy = lpBook->wTabStripButtonPolicy;
    bRet = SS_BuffAppend(lpBuff, &rec, sizeof(rec));
  }
  return bRet;
}
//--------------------------------------------------------------------
//
//  The SS_Load4TabEnhancedShape() function loads the sheet count.
//

BOOL SS_Load4TabEnhancedShape(LPSS_BOOK lpBook, LPSS_REC_COMMON lpCommon)
{
  LPSS_REC_TABENHANCEDSHAPE lpRec = (LPSS_REC_TABENHANCEDSHAPE)lpCommon;
  BOOL bRet = sizeof(*lpRec) == lpRec->lRecLen;
  if( bRet )
  {
    SS_SetTabEnhancedShape(lpBook, lpRec->wTabEnhancedShape);
  }

  return bRet;
}

//--------------------------------------------------------------------
//
//  The SS_Save4TabEnhancedShape() function stores the sheet count.
//

BOOL SS_Save4TabEnhancedShape(LPSS_BOOK lpBook, LPSS_BUFF lpBuff)
{
  SS_REC_TABENHANCEDSHAPE rec;
  BOOL bRet = TRUE;
  if( TRUE )
  {
    rec.nRecType = SS_RID_TABENHANCEDSHAPE;
    rec.lRecLen = sizeof(rec);
//    rec.nTabStripLeftSheet = lpBook->nTabStripLeftSheet;
    rec.wTabEnhancedShape = lpBook->wTabEnhancedShape;
    bRet = SS_BuffAppend(lpBuff, &rec, sizeof(rec));
  }
  return bRet;
}

//--------------------------------------------------------------------
//
//  The SS_Load4HighlightHeaders() function loads the Highlight Headers.
//

BOOL SS_Load4HighlightHeaders(LPSS_BOOK lpBook, LPSS_REC_COMMON lpCommon)
{
  LPSS_REC_HIGHLIGHTHEADERS lpRec = (LPSS_REC_HIGHLIGHTHEADERS)lpCommon;
  BOOL bRet = sizeof(*lpRec) == lpRec->lRecLen;
  if( bRet )
  {
    SS_SetHighlightHeaders(lpBook, lpRec->wHighlightHeaders);
  }

  return bRet;
}

//--------------------------------------------------------------------
//
//  The SS_Save4HighlightHeaders() function stores the Highlight Headers.
//

BOOL SS_Save4HighlightHeaders(LPSS_BOOK lpBook, LPSS_BUFF lpBuff)
{
  SS_REC_HIGHLIGHTHEADERS rec;
  BOOL bRet = TRUE;
  if( TRUE )
  {
    rec.nRecType = SS_RID_HIGHLIGHTHEADERS;
    rec.lRecLen = sizeof(rec);
    rec.wHighlightHeaders = lpBook->wHighlightHeaders;
    bRet = SS_BuffAppend(lpBuff, &rec, sizeof(rec));
  }
  return bRet;
}

//--------------------------------------------------------------------
//
//  The SS_Load4HighlightStyle() function loads the Highlight Style.
//

BOOL SS_Load4HighlightStyle(LPSS_BOOK lpBook, LPSS_REC_COMMON lpCommon)
{
  LPSS_REC_HIGHLIGHTSTYLE lpRec = (LPSS_REC_HIGHLIGHTSTYLE)lpCommon;
  BOOL bRet = sizeof(*lpRec) == lpRec->lRecLen;
  if( bRet )
  {
    SS_SetHighlightStyle(lpBook, lpRec->wHighlightStyle);
  }

  return bRet;
}

//--------------------------------------------------------------------
//
//  The SS_Save4HighlightStyle() function stores the Highlight Style.
//

BOOL SS_Save4HighlightStyle(LPSS_BOOK lpBook, LPSS_BUFF lpBuff)
{
  SS_REC_HIGHLIGHTSTYLE rec;
  BOOL bRet = TRUE;
  if( TRUE )
  {
    rec.nRecType = SS_RID_HIGHLIGHTSTYLE;
    rec.lRecLen = sizeof(rec);
    rec.wHighlightStyle = lpBook->wHighlightStyle;
    bRet = SS_BuffAppend(lpBuff, &rec, sizeof(rec));
  }
  return bRet;
}

//--------------------------------------------------------------------
//
//  The SS_Load4HighlightAlphaBlend() function loads the sheet count.
//

BOOL SS_Load4HighlightAlphaBlend(LPSS_BOOK lpBook, LPSS_REC_COMMON lpCommon)
{
  LPSS_REC_HIGHLIGHTALPHABLEND lpRec = (LPSS_REC_HIGHLIGHTALPHABLEND)lpCommon;
  BOOL bRet = sizeof(*lpRec) == lpRec->lRecLen;
  if( bRet )
  {
    SS_SetHighlightAlphaBlend(lpBook, lpRec->clrHighlightAlphaBlend, lpRec->nHighlightAlphaBlend);
  }

  return bRet;
}

//--------------------------------------------------------------------
//
//  The SS_Save4HighlightAlphaBlend() function stores the sheet count.
//

BOOL SS_Save4HighlightAlphaBlend(LPSS_BOOK lpBook, LPSS_BUFF lpBuff)
{
  SS_REC_HIGHLIGHTALPHABLEND rec;
  BOOL bRet = TRUE;
  if( TRUE )
  {
    rec.nRecType = SS_RID_HIGHLIGHTALPHABLEND;
    rec.lRecLen = sizeof(rec);
	 rec.clrHighlightAlphaBlend = lpBook->clrHighlightAlphaBlend;
    rec.nHighlightAlphaBlend = (short)lpBook->bHighlightAlphaBlend;
    bRet = SS_BuffAppend(lpBuff, &rec, sizeof(rec));
  }
  return bRet;
}

#endif // SS_V80
//--------------------------------------------------------------------
//
//  The SS_Save4TabStrip() function stores the sheet count.
//

BOOL SS_Save4TabStrip(LPSS_BOOK lpBook, LPSS_BUFF lpBuff)
{
  SS_REC_TABSTRIP rec;
  BOOL bRet = TRUE;
  if( TRUE )
  {
    rec.nRecType = SS_RID_TABSTRIP;
    rec.lRecLen = sizeof(rec);
//    rec.nTabStripLeftSheet = lpBook->nTabStripLeftSheet;
    rec.wTabStripPolicy = lpBook->wTabStripPolicy;
    rec.dfTabStripRatio = lpBook->dfTabStripRatio;
    rec.nFontId = lpBook->TabStripFontId - 1;
    bRet = SS_BuffAppend(lpBuff, &rec, sizeof(rec));
  }
  return bRet;
}
//--------------------------------------------------------------------

BOOL SS_Load4SheetName(LPSS_BOOK lpBook, LPSS_REC_COMMON lpCommon, BOOL bUnicode, short nSheet)
{
  LPSS_REC_SHEETNAME lpRec = (LPSS_REC_SHEETNAME)lpCommon;
  HPBYTE lpText = NULL;
  LPTSTR lpszText = NULL;
  BOOL bRet = sizeof(*lpRec) <= lpRec->lRecLen;
  if( bRet )
  {
    SS_BUFF buff;
    SS_BuffInit(&buff);
    lpText = (HPBYTE)lpRec + sizeof(*lpRec);
    if( sizeof(*lpRec) <= lpRec->lRecLen )
      lpszText = SS_BuffCopyStr(&buff, lpText, bUnicode);
    SS_SetSheetName(lpBook, nSheet, lpszText);

    SS_BuffFree(&buff);
  }
  return bRet;
}

//--------------------------------------------------------------------

BOOL SS_Save4SheetName(TBGLOBALHANDLE ghSheetName, LPSS_BUFF lpBuff)
{
  SS_REC_SHEETNAME rec;
  LPTSTR lpszText;
  long lTextLen;
  BOOL bRet = TRUE;
  if( ghSheetName )
      {
        lpszText = (LPTSTR)tbGlobalLock(ghSheetName);
        lTextLen = lstrlen(lpszText) + 1;
        rec.nRecType = SS_RID_SHEETNAME;
        rec.lRecLen = sizeof(rec) + (lTextLen * sizeof(TCHAR));
        bRet = SS_BuffAppend(lpBuff, &rec, sizeof(rec));
        bRet &= SS_BuffAppend(lpBuff, lpszText, lTextLen * sizeof(TCHAR));
        tbGlobalUnlock(ghSheetName);
      }
  return bRet;
}

//--------------------------------------------------------------------
//
//  The SS_Load4Scroll7() function loads the sheet count.
//

BOOL SS_Load4Scroll7(LPSS_BOOK lpBook, LPSS_REC_COMMON lpCommon)
{
  LPSS_REC_SCROLL7 lpRec = (LPSS_REC_SCROLL7)lpCommon;
  BOOL bRet = sizeof(*lpRec) == lpRec->lRecLen;
  if( bRet )
  {
    lpBook->VScrollBarColorId = SS_AddColor(lpRec->clrVScrollBar);
    lpBook->HScrollBarColorId = SS_AddColor(lpRec->clrHScrollBar);
    SS_SetScrollBarSize(lpBook, lpRec->dVScrollWidth, lpRec->dHScrollHeight);
  }

  return bRet;
}

//--------------------------------------------------------------------
//
//  The SS_Save4Scroll7() function stores the sheet count.
//

BOOL SS_Save4Scroll7(LPSS_BOOK lpBook, LPSS_BUFF lpBuff)
{
  SS_REC_SCROLL7 rec;
  BOOL bRet = TRUE;
  SS_COLORTBLITEM itemColor;

  if (lpBook->dVScrollWidthProp != -1 || lpBook->dHScrollHeightProp != -1 ||
      lpBook->VScrollBarColorId || lpBook->HScrollBarColorId)
  {
    rec.nRecType = SS_RID_SCROLL7;
    rec.lRecLen = sizeof(rec);
    rec.dVScrollWidth = lpBook->dVScrollWidthProp;
    rec.dHScrollHeight = lpBook->dHScrollHeightProp;

    SS_GetColorItem(&itemColor, lpBook->VScrollBarColorId);
    rec.clrVScrollBar = lpBook->VScrollBarColorId ? itemColor.Color : -1;
    SS_GetColorItem(&itemColor, lpBook->HScrollBarColorId);
    rec.clrHScrollBar = lpBook->HScrollBarColorId ? itemColor.Color : -1;

    bRet = SS_BuffAppend(lpBuff, &rec, sizeof(rec));
  }
  return bRet;
}

//--------------------------------------------------------------------
//
//  The SS_Load4SheetSettings() function loads the sheet count.
//

BOOL SS_Load4SheetSettings(LPSPREADSHEET lpSS, LPSS_REC_COMMON lpCommon)
{
  LPSS_REC_SHEETSETTINGS lpRec = (LPSS_REC_SHEETSETTINGS)lpCommon;
  BOOL bRet = sizeof(*lpRec) == lpRec->lRecLen;
  if( bRet )
  {
    lpSS->bActiveCellHighlightStyle = lpRec->bActiveCellHighlightStyle;
    lpSS->fAllowColMove = lpRec->fAllowColMove;
    lpSS->fAllowRowMove = lpRec->fAllowRowMove;
    lpSS->fVisible = lpRec->fSheetVisible;
  }

  return bRet;
}

//--------------------------------------------------------------------
//
//  The SS_Save4SheetSettings() function stores the sheet count.
//

BOOL SS_Save4SheetSettings(LPSPREADSHEET lpSS, LPSS_BUFF lpBuff)
{
  SS_REC_SHEETSETTINGS rec;
  BOOL bRet = TRUE;

  if (lpSS->bActiveCellHighlightStyle != SS_ACTIVECELLHIGHLIGHTSTYLE_NORMAL ||
      lpSS->fAllowColMove || lpSS->fAllowRowMove || !lpSS->fVisible)
  {
    rec.nRecType = SS_RID_SHEETSETTINGS;
    rec.lRecLen = sizeof(rec);
    rec.bActiveCellHighlightStyle = lpSS->bActiveCellHighlightStyle;
    rec.fAllowColMove = lpSS->fAllowColMove;
    rec.fAllowRowMove = lpSS->fAllowRowMove;
    rec.fSheetVisible = lpSS->fVisible;

    bRet = SS_BuffAppend(lpBuff, &rec, sizeof(rec));
  }
  return bRet;
}

#endif // SS_V70

//--------------------------------------------------------------------

BOOL SS_LoadFromBuffer4(LPSS_BOOK lpBook, HPBYTE hpBuff, long lBuffLen)
{
  HPBYTE hpBuffCur;
  HPBYTE hpBuffNext;
  LPSS_REC_COMMON lpRec;
  LPSS_REC_FILE lpFileRec;
  SS_FONTBUFFER fontBuff = {0, 0, 0, 0}; // pMem, phFont, lLen, lAlloc
  SS_BUFF xtraBuff;
  SS_BUFF recBuff;
  SS_CALCIDTABLE savedCustFuncs = {0, 0, 0}; // hElem, lpElem, nElemCnt
  SS_CALCIDTABLE savedCustNames = {0, 0, 0}; // hElem, lpElem, nElemCnt
  SS_CALCIDTABLE savedDdeLinks = {0, 0, 0}; // hElem, lpElem, nElemCnt
#ifdef SS_V80
    SS_CELLIDTABLE savedCustCells = {0, 0, 0}; // hElem, lpElem, nElemCnt
#endif
  SS_LOADBLOCK loadBlock = {SS_ALLCOLS,SS_ALLCOLS,SS_ALLCOLS,SS_ALLCOLS,0,0};
  BOOL bUnicode;
  BOOL bAll;
  BOOL bAutoCalc;
  BOOL bRedraw = TRUE;
  BOOL bRedrawIgnore; // Added for V 3.0 to ignore the redraw flag when reading in.  RFW - 11/4/98
  BOOL bRet = TRUE;
  long lRecLen;
  long lFileLen;
  long lFilePos = 0;
  int i;
  short nSheet = lpBook->nActiveSheet;
//RAP03a >>
  DWORD Bytes = 0;
  long lBuffPos = 0;
  long lBuffRecycleLen = 0;
  HPBYTE hpBuffTemp = NULL;
  GLOBALHANDLE ghBigBuffer = (GLOBALHANDLE)0;
//RAP03a <<
//  HWND hWndSave = NULL; // optimize load speed -scl

  lpFileRec = (LPSS_REC_FILE)hpBuff;
  bAll = lpFileRec->bFileType & SS_FILETYPE_ALL;
  bUnicode = lpFileRec->bFileType & SS_FILETYPE_UNICODE;
  bRet = SS_SAVEVERSION4 == lpFileRec->bFileVer;
  bRet &= (bAll | bUnicode) == lpFileRec->bFileType;

  lFileLen = lpFileRec->lFileLen;

#ifdef WIN32
  if (!lpBook->hFile)
#endif
    bRet &= lBuffLen >= lpFileRec->lFileLen;
//    bRet &= lBuffLen == lpFileRec->lFileLen;

  if( bRet )
  {
    LPSPREADSHEET lpSS;
    bRedraw = SS_BookSetRedraw(lpBook, FALSE);
    SS_BuffInit(&xtraBuff);
    if( bAll )
    {
      HWND hwndOwner = lpBook->hWndOwner;
      #if defined(SS_RSDLL)
      BOOL bDesignTime = SS_DdeGetDesignTime(lpBook);
      #endif
      if (lpBook->fUseSpecialVScroll)
        SS_VScrollSetSpecial(lpBook, FALSE, 0);
      SS_ClearAll(lpBook);
      SS_InitSpreadPreHwnd(lpBook, RGB(0,0,0), RGB(255,255,255)); // need real colors
      if( lpBook->hWnd )
      {
//         hWndSave = lpBook->hWnd;
         SS_InitSpreadWithHwnd(lpBook->hWnd, lpBook);
//         lpBook->hWnd = NULL; // optimize load speed -scl
      }
//#if SS_V80
	  lpBook->fLoading = TRUE;
//#endif
      lpBook->hWndOwner = hwndOwner;
      #if defined(SS_OCX)
      SS_OcxResetOnLoad(lpBook);
      #elif defined(SS_VB)
      SS_VbxResetOnLoad(lpSS);
      #endif
      #if defined(SS_RSDLL)
      SS_DdeSetDesignTime(lpBook, bDesignTime);
      #endif
    }
    lpBook->CalcAuto = FALSE;
    bAutoCalc = TRUE;

    lpSS = SS_BookLockSheetIndex(lpBook, nSheet);

    if( !bAll )
      SS_ClearDataRange(lpSS, SS_ALLCOLS, SS_ALLROWS, SS_ALLCOLS, SS_ALLROWS,
                       FALSE, FALSE);

    // Skip buffer type, version, and size information
    hpBuffCur = hpBuff + sizeof(SS_REC_FILE);
    lBuffPos = sizeof(SS_REC_FILE);
    lFilePos = sizeof(SS_REC_FILE);
    // Process each record in buffer
    SS_BuffInit(&recBuff);
//RAP03d    while( bRet && hpBuffCur < hpBuff + lBuffLen )
#ifdef WIN32
    while( bRet && lFilePos < lFileLen )
    {
//RAP03a >>
      if (lpBook->hFile != (HANDLE)0)
      {
        short nSizeMultiple = 1;
        lpRec = (LPSS_REC_COMMON)(hpBuff + lBuffPos);
        if (lBuffLen < lBuffPos + (long)sizeof(SS_REC_COMMON) ||
            lBuffLen < lBuffPos + lpRec->lRecLen)
        {
          lBuffRecycleLen = lBuffLen - lBuffPos;
          if (lBuffRecycleLen)
            MemHugeCpy(hpBuff, (HPBYTE)lpRec, lBuffRecycleLen);

          lpRec = (LPSS_REC_COMMON)hpBuff;
          lBuffPos = 0;

          // read the next chunk from the file
          ReadFile(lpBook->hFile, (LPVOID)&hpBuff[lBuffRecycleLen], SS_FILEREADSIZE-lBuffRecycleLen,
                   &Bytes, NULL);
          if (Bytes == -1 || Bytes == 0)
            break;

//RAP03 -- add checking for very large records
          if (lpRec->lRecLen > SS_FILEREADSIZE)
          {
            hpBuffTemp = hpBuff;
            ghBigBuffer = GlobalAlloc(GHND, lpRec->lRecLen);
            hpBuff = (HPBYTE)GlobalLock(ghBigBuffer);
            if (lBuffRecycleLen+Bytes)
              MemHugeCpy(hpBuff, hpBuffTemp, lBuffRecycleLen+Bytes);

            ReadFile(lpBook->hFile, (LPVOID)&hpBuff[lBuffRecycleLen+Bytes], lpRec->lRecLen-(lBuffRecycleLen+Bytes),
                     &Bytes, NULL);
            if (Bytes == -1 || Bytes == 0)
              break;
          }

          hpBuffCur = hpBuff;

          lBuffPos = lpRec->lRecLen;
          
          lBuffLen = Bytes + lBuffRecycleLen;
        }
        else
        {
          lBuffPos += lpRec->lRecLen;
        }
      }
      else if (hpBuffCur >= hpBuff + lBuffLen)
      {
        break;
      }
//RAP03a <<
#else
    while( bRet && hpBuffCur < hpBuff + lFileLen )
    {
#endif
      MemHugeCpy(&lRecLen, hpBuffCur + sizeof(short), sizeof(long));
      lFilePos += lRecLen;
      if (lRecLen != lBuffLen)
        hpBuffNext = hpBuffCur + lRecLen;
      else
        hpBuffNext = (HPBYTE)NULL;
      if( HIWORD(hpBuffCur) == HIWORD(hpBuffNext) )
        lpRec = (LPSS_REC_COMMON)hpBuffCur;
      else
      {
        SS_BuffCopy(&recBuff, hpBuffCur, lRecLen);
        lpRec = (LPSS_REC_COMMON)(recBuff.pMem);
      }
      switch( lpRec->nRecType )
      {
        case SS_RID_AUTOSIZE: bRet = SS_Load4AutoSize(lpSS, lpRec); break;
        case SS_RID_BACKCOLORSTYLE: bRet = SS_Load4BackColorStyle(lpBook, lpRec); break;
        case SS_RID_BORDER: bRet = SS_Load4Border(lpSS, lpRec, &loadBlock); break;
        case SS_RID_BORDER_EXT: bRet = SS_Load4BorderExt(lpSS, lpRec, &loadBlock); break;
        case SS_RID_CALC: bRet = SS_Load4Calc(lpBook, lpRec, &bAutoCalc); break;
        case SS_RID_CALCOLD: bRet = SS_Load4CalcOld(lpBook, lpRec, &bAutoCalc); break;
        case SS_RID_CALCDEPEND: bRet = SS_Load4CalcDepend(lpBook, &xtraBuff, lpRec); break;
        case SS_RID_COLOR: bRet = SS_Load4Color(lpSS, lpRec, &loadBlock); break;
        case SS_RID_COLPAGEBREAK: bRet = SS_Load4ColPageBreak(lpSS, lpRec); break;
        case SS_RID_COLWIDTH: bRet = SS_Load4ColWidth(lpSS, lpRec, &loadBlock); break;
        case SS_RID_CURSOR: bRet = SS_Load4Cursor(lpBook, lpRec); break;
        case SS_RID_CURSORCUSTOM: bRet = SS_Load4CursorCustom(lpBook, lpRec); break;
        case SS_RID_CUSTFUNCTABLE: bRet = SS_Load4CustFuncTable(lpBook, lpRec, &savedCustFuncs, bUnicode); break;
        case SS_RID_CUSTNAMETABLE: bRet = SS_Load4CustNameTable(lpBook, lpRec, &savedCustFuncs, &savedCustNames, &savedDdeLinks, bUnicode); break;
        case SS_RID_DATAAWARE: bRet = SS_Load4DataAware(lpSS, &xtraBuff, lpRec); break;
        case SS_RID_DATAAWARECOL: bRet = SS_Load4DataAwareCol(lpSS, &xtraBuff, lpRec, bUnicode); break;
        case SS_RID_DATAAWAREQE: bRet = SS_Load4DataAwareQE(lpSS, &xtraBuff, lpRec, bUnicode); break;
        case SS_RID_DDETABLE: bRet = SS_Load4DdeTable(lpSS, lpRec, &savedDdeLinks, bUnicode); break;
        case SS_RID_DEFFLOATFORMAT: bRet = SS_Load4DefFloatFormat(lpSS, lpRec); break;
        case SS_RID_DISPLAY: bRet = SS_Load4Display(lpBook, lpRec, &bRedrawIgnore); break;
        case SS_RID_EDIT: bRet = SS_Load4Edit(lpBook, lpRec); break;
        case SS_RID_FONT: bRet = SS_Load4Font(lpSS, lpRec, &fontBuff, &loadBlock); break;
        case SS_RID_FONTTABLE: bRet = SS_Load4FontTable(lpSS, lpRec, &fontBuff, bUnicode); break;
        case SS_RID_FONTTABLEOLD: bRet = SS_Load4FontTableOld(lpSS, lpRec, &fontBuff, bUnicode); break;
        case SS_RID_FORMULAUNPARSED: bRet = SS_Load4FormulaUnparsed(lpSS, lpRec, bUnicode, &loadBlock); break;
        case SS_RID_FORMULA: bRet = SS_Load4Formula(lpSS, lpRec, &savedCustFuncs, &savedCustNames, &savedDdeLinks, bUnicode, &loadBlock); break;
        case SS_RID_FROZEN: bRet = SS_Load4Frozen(lpSS, lpRec); break;
        case SS_RID_GRAYAREACOLOR: bRet = SS_Load4GrayAreaColor(lpBook, lpRec); break;
        case SS_RID_GRID: bRet = SS_Load4Grid(lpSS, lpRec); break;
        case SS_RID_HEADER: bRet = SS_Load4Header(lpSS, lpRec); break;
        case SS_RID_ITEMDATA: bRet = SS_Load4ItemData(lpSS, lpRec); break;
#if defined(_WIN64) || defined(_IA64)
        case SS_RID_ITEMDATA64: bRet = SS_Load4ItemData64(lpSS, lpRec); break;
#endif
        case SS_RID_ITERATION: bRet = SS_Load4Iteration(lpBook, lpRec); break;
        case SS_RID_LOCK: bRet = SS_Load4Lock(lpSS, lpRec, &loadBlock); break;
        case SS_RID_LOCKCOLOR: bRet = SS_Load4LockColor(lpSS, lpRec); break;
        case SS_RID_SELCOLOR: bRet = SS_Load4SelColor(lpBook, lpRec); break;
        case SS_RID_MAXSIZE: bRet = SS_Load4MaxSize(lpSS, lpRec); break;
        case SS_RID_OPMODE: bRet = SS_Load4OpMode(lpSS, lpRec); break;
#ifdef SS_V30
        case SS_RID_PRINT2: bRet = SS_Load4Print2(lpSS, lpRec); break;
#endif
        case SS_RID_PRINTBOOK: bRet = SS_Load4PrintBook(lpBook, bUnicode, lpRec); break;
        case SS_RID_PRINTSHEET: bRet = SS_Load4PrintSheet(lpSS, bUnicode, lpRec); break;
        case SS_RID_PRINT: bRet = SS_Load4Print(lpSS, lpRec, bUnicode); break;
        case SS_RID_PROTECT: bRet = SS_Load4Protect(lpBook, lpRec); break;
        case SS_RID_ROWHEIGHT: bRet = SS_Load4RowHeight(lpSS, lpRec, &loadBlock); break;
        case SS_RID_ROWPAGEBREAK: bRet = SS_Load4RowPageBreak(lpSS, lpRec); break;
        case SS_RID_REFSTYLE: bRet = SS_Load4RefStyle(lpBook, lpRec); break;
        case SS_RID_RESTRICT: bRet = SS_Load4Restrict(lpSS, lpRec); break;
        case SS_RID_SCROLLBAR: bRet = SS_Load4ScrollBar(lpBook, lpRec); break;
        case SS_RID_SELBLOCKOPTION: bRet = SS_Load4SelBlockOption(lpBook, lpRec); break;
        case SS_RID_SHADOWCOLOR: bRet = SS_Load4ShadowColor(lpSS, lpRec); break;
        case SS_RID_TYPEBUTTON: bRet = SS_Load4TypeButton(lpSS, lpRec, bUnicode, &loadBlock); break;
        case SS_RID_TYPECHECK: bRet = SS_Load4TypeCheck(lpSS, lpRec, bUnicode, &loadBlock); break;
        case SS_RID_TYPECOMBO: bRet = SS_Load4TypeCombo(lpSS, lpRec, bUnicode, &loadBlock); break;
#ifdef SS_V30
        case SS_RID_TYPECOMBOEX: bRet = SS_Load4TypeComboEx(lpSS, lpRec, bUnicode, &loadBlock); break;
#endif // SS_V30
#ifdef SS_V40
        case SS_RID_TYPECOMBOEX4: bRet = SS_Load4TypeComboEx4(lpSS, lpRec, bUnicode, &loadBlock); break;
#endif // SS_V40
        case SS_RID_TYPEDATE: bRet = SS_Load4TypeDate(lpSS, lpRec, &loadBlock); break;
        case SS_RID_TYPEEDIT: bRet = SS_Load4TypeEdit(lpSS, lpRec, &loadBlock); break;
        case SS_RID_TYPEFLOAT: bRet = SS_Load4TypeFloat(lpSS, lpRec, &loadBlock); break;
        case SS_RID_TYPEINTEGER: bRet = SS_Load4TypeInteger(lpSS, lpRec, &loadBlock); break;
        case SS_RID_TYPEOWNERDRAW: bRet = SS_Load4TypeOwnerDraw(lpSS, lpRec, &loadBlock); break;
        case SS_RID_TYPEPIC: bRet = SS_Load4TypePic(lpSS, lpRec, bUnicode, &loadBlock); break;
        case SS_RID_TYPEPICTURE: bRet = SS_Load4TypePicture(lpSS, lpRec, bUnicode, &loadBlock); break;
        case SS_RID_TYPESTATIC: bRet = SS_Load4TypeStatic(lpSS, lpRec, &loadBlock); break;
        case SS_RID_TYPETIME: bRet = SS_Load4TypeTime(lpSS, lpRec, &loadBlock); break;
        case SS_RID_UNITTYPE: bRet = SS_Load4UnitType(lpBook, lpRec); break;
        case SS_RID_USERRESIZE: bRet = SS_Load4UserResize(lpSS, lpRec); break;
        case SS_RID_USERRESIZECOL: bRet = SS_Load4UserResizeCol(lpSS, lpRec); break;
        case SS_RID_USERRESIZEROW: bRet = SS_Load4UserResizeRow(lpSS, lpRec); break;
        case SS_RID_VALLONG: bRet = SS_Load4ValLong(lpSS, lpRec, &loadBlock); break;
        case SS_RID_VALDBL: bRet = SS_Load4ValDbl(lpSS, lpRec, &loadBlock); break;
        case SS_RID_VALSTR: bRet = SS_Load4ValStr(lpSS, lpRec, bUnicode, &loadBlock); break;
        case SS_RID_VIRTUAL: bRet = SS_Load4Virtual(lpSS, lpRec); break;
#ifdef SS_V30
        case SS_RID_ACTIONKEY: bRet = SS_Load4ActionKey(lpBook, lpRec); break;
        case SS_RID_ODDEVENROWCOLOR: bRet = SS_Load4OddEvenRowColor(lpSS, lpRec); break;
        case SS_RID_APPEARANCE: bRet = SS_Load4Appearance(lpBook, lpRec); break;
        case SS_RID_TEXTTIP: SS_Load4TextTip(lpBook, lpRec, bUnicode); break;
        case SS_RID_SCROLLBARTRACK: bRet = SS_Load4ScrollBarTrack(lpSS, lpRec); break;
        case SS_RID_CLIPBOARDOPTIONS: bRet = SS_Load4ClipboardOptions(lpBook, lpRec); break;
        case SS_RID_COLHIDDEN: bRet = SS_Load4ColHidden(lpSS, lpRec); break;
        case SS_RID_ROWHIDDEN: bRet = SS_Load4RowHidden(lpSS, lpRec); break;
        case SS_RID_TWODIGITYEARMAX: bRet = SS_Load4TwoDigitYearMax(lpBook, lpRec); break;
#endif // SS_V30
#ifdef SS_V35
        case SS_RID_CELLNOTE: bRet = SS_Load4CellNote(lpSS, lpRec, bUnicode, &loadBlock); break;
        case SS_RID_CELLNOTEINDICATOR: bRet = SS_Load4CellNoteIndicator(lpBook, lpRec); break;
        case SS_RID_SHOWSCROLLTIPS: bRet = SS_Load4ShowScrollTips(lpBook, lpRec); break;
        case SS_RID_USERCOLACTION: bRet = SS_Load4UserColAction(lpSS, lpRec); break;
        case SS_RID_SCRIPTENHANCED: bRet = SS_Load4ScriptEnhanced(lpBook, lpRec); break;
        case SS_RID_COLUSERSORTINDICATOR: bRet = SS_Load4ColUserSortIndicator(lpSS, lpRec); break;
        case SS_RID_CALTEXTOVERRIDE: bRet = SS_Load4CalTextOverride(lpBook, bUnicode, lpRec); break;
#endif // SS_V35
#ifdef SS_V40
        case SS_RID_CELLSPANTABLE: bRet = SS_Load4CellSpanTable(lpSS, lpRec); break;
        case SS_RID_COLMERGE: bRet = SS_Load4ColMerge(lpSS, lpRec); break;
        case SS_RID_COLID: bRet = SS_Load4ColID(lpSS, lpRec, bUnicode); break;
        case SS_RID_HEADER4: bRet = SS_Load4Header4(lpSS, lpRec); break;
        case SS_RID_ROWMERGE: bRet = SS_Load4RowMerge(lpSS, lpRec); break;
        case SS_RID_DISPLAY4: bRet = SS_Load4Display4(lpBook, lpRec); break;
        case SS_RID_CELLTAG: bRet = SS_Load4CellTag(lpSS, lpRec, bUnicode, &loadBlock); break;
        case SS_RID_TYPECURRENCY: bRet = SS_Load4TypeCurrency(lpSS, lpRec, &loadBlock, bUnicode); break;
        case SS_RID_TYPENUMBER: bRet = SS_Load4TypeNumber(lpSS, lpRec, &loadBlock, bUnicode); break;
        case SS_RID_TYPEPERCENT: bRet = SS_Load4TypePercent(lpSS, lpRec, &loadBlock, bUnicode); break;
#endif // SS_V40

#ifdef SS_V70
        case SS_RID_SHEETCOUNT: bRet = SS_Load4SheetCount(lpBook, lpRec); break;
        //case SS_RID_ACTIVESHEET: bRet = SS_Load4ActiveSheet(lpBook, lpRec); break;
        case SS_RID_SHEETNAME: bRet = SS_Load4SheetName(lpBook, lpRec, bUnicode, nSheet); break;
        case SS_RID_TABSTRIP: bRet = SS_Load4TabStrip(lpBook, lpRec, &fontBuff); break;
        case SS_RID_TYPESCIENTIFIC: bRet = SS_Load4TypeScientific(lpSS, lpRec, &loadBlock, bUnicode); break;
        case SS_RID_SCROLL7: bRet = SS_Load4Scroll7(lpBook, lpRec); break;
        case SS_RID_SHEETSETTINGS: bRet = SS_Load4SheetSettings(lpSS, lpRec); break;
        case SS_RID_SHEETINDEX:
          SS_BookUnlockSheetIndex(lpBook, nSheet);
          bRet = SS_Load4SheetIndex(lpRec, &nSheet);
          lpSS = SS_BookLockSheetIndex(lpBook, nSheet);
          if (!lpSS)
            bRet = FALSE;
          break;
#endif // SS_V70
#ifdef SS_V80
			case SS_RID_SCROLLBARSTYLE: bRet = SS_Load4ScrollBarStyle(lpBook, lpRec); break;
			case SS_RID_ENHANCESTATICCELLS: bRet = SS_Load4EnhanceStaticCells(lpBook, lpRec); break;
			case SS_RID_USEVISUALSTYLES: bRet = SS_Load4UseVisualStyles(lpBook, lpRec); break;
			case SS_RID_APPEARANCESTYLE: bRet = SS_Load4AppearanceStyle(lpBook, lpRec); break;
			case SS_RID_TABENHANCEDSHAPE: bRet = SS_Load4TabEnhancedShape(lpBook, lpRec); break;
			case SS_RID_CELLNOTEINDICATOR2: bRet = SS_Load4CellNoteIndicator2(lpBook, lpRec);break;
			case SS_RID_PRINTDEVMODE: bRet = SS_Load4PrintDevMode(lpBook, lpRec);break;
			case SS_RID_TABSTRIPBUTTONPOLICY: bRet = SS_Load4TabStripButtonPolicy(lpBook, lpRec); break;
			case SS_RID_HIGHLIGHTHEADERS: bRet = SS_Load4HighlightHeaders(lpBook, lpRec); break;
			case SS_RID_HIGHLIGHTSTYLE: bRet = SS_Load4HighlightStyle(lpBook, lpRec); break;
			case SS_RID_HIGHLIGHTALPHABLEND: bRet = SS_Load4HighlightAlphaBlend(lpBook, lpRec); break;
			case SS_RID_CUSTCELLTABLE: bRet = SS_Load4CustCellTable(lpBook, lpRec, &savedCustCells, bUnicode); break;
			case SS_RID_ENHANCEDCOLORS : bRet = SS_Load4EnhancedColors(lpBook, lpRec); break;
         case SS_RID_TYPECUSTOM: bRet = SS_Load4TypeCustom(lpSS, lpRec, bUnicode, &loadBlock); break;
         case SS_RID_VALBUFFER: bRet = SS_Load4ValBuffer(lpSS, lpRec, bUnicode, &loadBlock); break;
#endif
        default: bRet = lpRec->lRecLen >= sizeof(SS_REC_COMMON); break;
      }
      hpBuffCur = hpBuffNext;
    
      if (ghBigBuffer)
      {
        GlobalUnlock(ghBigBuffer);
        GlobalFree(ghBigBuffer);
        ghBigBuffer = (GLOBALHANDLE)0;
        hpBuff = hpBuffTemp;
        lBuffPos = lBuffLen = 0;
      }
    }
    SS_BuffFree(&recBuff);
    if( xtraBuff.pMem )
      ssGlobalUnlock(xtraBuff.hMem); //RAP01c
//Modify by BOC 99.4.30 (hyt)-------------
//for free this generate smartheap error
//#ifndef SPREAD_JPN
    if( lpBook->hXtra )
      GlobalFree(lpBook->hXtra); //RAP01c
//#endif
//-----------------------------------------
//RAP01a >>
    lpBook->lXtraLen = xtraBuff.lLen;
    lpBook->hXtra = 0;
#ifdef WIN32
    if (lpBook->lXtraLen)
		 {
		 BYTE HUGE *hpSrc, *hpDest;
		 lpBook->hXtra = GlobalAlloc(GHND, lpBook->lXtraLen);
		 hpSrc = (BYTE HUGE *)tbGlobalLock(xtraBuff.hMem);
		 hpDest = (BYTE HUGE *)GlobalLock(lpBook->hXtra);
		 MemHugeCpy((HPVOID)hpDest, (HPCVOID)hpSrc, lpBook->lXtraLen);
		 tbGlobalUnlock(xtraBuff.hMem);
		 GlobalUnlock(lpBook->hXtra);
		 SS_BuffFree(&xtraBuff);
		 }
#else
    lpBook->hXtra = xtraBuff.hMem;
#endif
//RAP01a <<

//RAP01d    lpBook->hXtra = xtraBuff.hMem;

    lpBook->bXtraVer = lpFileRec->bFileVer;
    SS_BookSetAutoCalc(lpBook, bAutoCalc);
    // obsolete - SS_BuildDependencies(lpSS);
    lpBook->Redraw = bRedraw;
//#if SS_V80
	  lpBook->fLoading = FALSE;
	  if( lpBook->fAllowCellOverflow )
	  {
		  SS_SetBool(lpBook, NULL, SSB_ALLOWCELLOVERFLOW, FALSE);
		  SS_SetBool(lpBook, NULL, SSB_ALLOWCELLOVERFLOW, TRUE);
	  }
//#endif
    if( fontBuff.hMem )
    {
      for( i = 0; i < fontBuff.lLen; i++ )
        DeleteObject(fontBuff.phFont[i]);
      GlobalUnlock(fontBuff.hMem);
      GlobalFree(fontBuff.hMem);
    }
    if( savedCustFuncs.hElem )
    {
      for( i = 0; i < savedCustFuncs.nElemCnt; i++ )
        CustFuncDestroy(savedCustFuncs.lpElem[i]);
      tbGlobalUnlock(savedCustFuncs.hElem);
      tbGlobalFree(savedCustFuncs.hElem);
    }
    if( savedCustNames.hElem )
    {
      for( i = 0; i < savedCustNames.nElemCnt; i++ )
        NameDestroy(savedCustNames.lpElem[i]);
      tbGlobalUnlock(savedCustNames.hElem);
      tbGlobalFree(savedCustNames.hElem);
    }
    #if defined(SS_DDE)
    if( savedDdeLinks.hElem )
    {
      for( i = 0; i < savedDdeLinks.nElemCnt; i++ )
        DdeDestroy(savedDdeLinks.lpElem[i]);
      tbGlobalUnlock(savedDdeLinks.hElem);
      tbGlobalFree(savedDdeLinks.hElem);
    }
    #endif

    SS_BookUnlockSheetIndex(lpBook, nSheet);
  }
//  if( hWndSave && lpBook )
//     lpBook->hWnd = hWndSave; // optimize load speed -scl
  return bRet;
}


//RAP03a >>
BOOL SS_DumpBufferToFile(LPSS_BOOK lpBook, LPSS_BUFF lpBuff, LPLONG lpFileLen)
{
  BOOL bRet = TRUE;

#ifdef WIN32
  if (lpBuff->hMem)
  {
    HPBYTE hpBuffer = (HPBYTE)tbGlobalLock(lpBuff->hMem);
    long BytesWritten = 0;

    if (!WriteFile(lpBook->hFile, hpBuffer, lpBuff->lLen, &BytesWritten, NULL) ||
        lpBuff->lLen != BytesWritten)
    {
      bRet = FALSE;
    }

    tbGlobalUnlock(lpBuff->hMem);
    tbGlobalFree(lpBuff->hMem);

    if (lpFileLen)
      *lpFileLen += lpBuff->lLen;

    SS_BuffInit(lpBuff);
  }
#endif

  return bRet;
}


SS_DumpBufferToFileOnCondition(LPSS_BOOK lpBook, BOOL bRet, LPSS_BUFF lpBuff, LPLONG lpFileLen)
{
#ifdef WIN32
if (lpBook->hFile && bRet && lpBuff->lLen > SS_FILEREADSIZE)
  bRet = SS_DumpBufferToFile(lpBook, lpBuff, lpFileLen);
#endif

return bRet;
}
//RAP03a <<


//--------------------------------------------------------------------
//
//  The SS_SaveToBuffer4() function saves the spreadsheet to a
//  block of global memory.
//

GLOBALHANDLE SS_SaveToBuffer4(LPSS_BOOK lpBook, LPLONG lplBuffLen, BOOL bDataOnly, LPBOOL lpfRet)
{
  LPSPREADSHEET lpSS;
  SS_BUFF buff;
  SS_REC_FILE recFile;
  SS_COORD i, j;
  SS_COORD lColAdjust, lRowAdjust;
  LPSS_CELL lpCell;
  LPSS_COL lpCol;
  LPSS_ROW lpRow;
  BOOL bRet = TRUE;
  GLOBALHANDLE ghReturn = (GLOBALHANDLE)0; //RAP01a
  long lFileLen = 0; //RAP03a
  short nSheet;

  SS_BuffInit(&buff);
  // Save file information
  recFile.bFileType = bDataOnly ? SS_FILETYPE_DATA : SS_FILETYPE_ALL;
  #if defined(_UNICODE)
  recFile.bFileType |= SS_FILETYPE_UNICODE;
  #endif
  recFile.bFileVer = SS_SAVEVERSION4;
  recFile.lFileLen = 0;
  bRet &= SS_BuffAppend(&buff, &recFile, sizeof(recFile));
  bRet &= SS_DumpBufferToFileOnCondition(lpBook, bRet, &buff, &lFileLen);

  // Save spreadsheet information
  #ifdef SS_V70
  bRet &= SS_Save4SheetCount(lpBook, &buff);
  #endif
  if( !bDataOnly )
  {
    bRet &= SS_Save4FontTable(&buff);
    bRet &= SS_DumpBufferToFileOnCondition(lpBook, bRet, &buff, &lFileLen);
    bRet &= SS_Save4UnitType(lpBook, &buff);  // UnitType must be before ColWidth and RowHeight
    bRet &= SS_DumpBufferToFileOnCondition(lpBook, bRet, &buff, &lFileLen);
    bRet &= SS_Save4Calc(lpBook, &buff);
    bRet &= SS_DumpBufferToFileOnCondition(lpBook, bRet, &buff, &lFileLen);
    bRet &= SS_Save4CalcDepend(lpBook, &buff);
    bRet &= SS_DumpBufferToFileOnCondition(lpBook, bRet, &buff, &lFileLen);
    bRet &= SS_Save4Cursor(SS_CURSORTYPE_DEFAULT, lpBook->CursorDefault.hCursor, &buff);
    bRet &= SS_Save4Cursor(SS_CURSORTYPE_COLRESIZE, lpBook->CursorColResize.hCursor, &buff);
    bRet &= SS_Save4Cursor(SS_CURSORTYPE_ROWRESIZE, lpBook->CursorRowResize.hCursor, &buff);
    bRet &= SS_Save4Cursor(SS_CURSORTYPE_BUTTON, lpBook->CursorButton.hCursor, &buff);
    bRet &= SS_Save4Cursor(SS_CURSORTYPE_GRAYAREA, lpBook->CursorGrayArea.hCursor, &buff);
    bRet &= SS_Save4Cursor(SS_CURSORTYPE_LOCKEDCELL, lpBook->CursorLockedCell.hCursor, &buff);
    bRet &= SS_Save4Cursor(SS_CURSORTYPE_COLHEADER, lpBook->CursorColHeader.hCursor, &buff);
    bRet &= SS_Save4Cursor(SS_CURSORTYPE_ROWHEADER, lpBook->CursorRowHeader.hCursor, &buff);
#ifdef SS_V30
    bRet &= SS_Save4Cursor(SS_CURSORTYPE_DRAGDROPAREA, lpBook->CursorDragDropArea.hCursor, &buff);
    bRet &= SS_Save4Cursor(SS_CURSORTYPE_DRAGDROP, lpBook->CursorDragDrop.hCursor, &buff);
#endif
    bRet &= SS_DumpBufferToFileOnCondition(lpBook, bRet, &buff, &lFileLen);
    bRet &= SS_Save4CustFuncTable(lpBook, &buff);
    bRet &= SS_Save4CustNameTable(lpBook, TRUE, &buff);
#ifdef SS_V80
    bRet &= SS_Save4CustCellTable(lpBook, &buff);
#endif
    bRet &= SS_DumpBufferToFileOnCondition(lpBook, bRet, &buff, &lFileLen);
    bRet &= SS_Save4Display(lpBook, &buff);
    bRet &= SS_DumpBufferToFileOnCondition(lpBook, bRet, &buff, &lFileLen);
    bRet &= SS_Save4Edit(lpBook, &buff);
    bRet &= SS_DumpBufferToFileOnCondition(lpBook, bRet, &buff, &lFileLen);
    bRet &= SS_Save4GrayAreaColor(lpBook, &buff);
    bRet &= SS_DumpBufferToFileOnCondition(lpBook, bRet, &buff, &lFileLen);
    bRet &= SS_Save4Iteration(lpBook, &buff);
    bRet &= SS_DumpBufferToFileOnCondition(lpBook, bRet, &buff, &lFileLen);
    /*
    bRet &= SS_Save4Print(lpBook, &buff);
    bRet &= SS_DumpBufferToFileOnCondition(lpBook, bRet, &buff, &lFileLen);
    */
    bRet &= SS_Save4Protect(lpBook, &buff);
    bRet &= SS_DumpBufferToFileOnCondition(lpBook, bRet, &buff, &lFileLen);
    bRet &= SS_Save4RefStyle(lpBook, &buff);
    bRet &= SS_DumpBufferToFileOnCondition(lpBook, bRet, &buff, &lFileLen);
    bRet &= SS_Save4ScrollBar(lpBook, &buff);
    bRet &= SS_DumpBufferToFileOnCondition(lpBook, bRet, &buff, &lFileLen);
    bRet &= SS_Save4SelBlockOption(lpBook, &buff);
    bRet &= SS_DumpBufferToFileOnCondition(lpBook, bRet, &buff, &lFileLen);
    bRet &= SS_Save4PrintBook(lpBook, &buff);
    bRet &= SS_DumpBufferToFileOnCondition(lpBook, bRet, &buff, &lFileLen);
#ifdef SS_V30
    bRet &= SS_Save4ActionKey(lpBook, &buff);
    bRet &= SS_DumpBufferToFileOnCondition(lpBook, bRet, &buff, &lFileLen);
    bRet &= SS_Save4Appearance(lpBook, &buff);
    bRet &= SS_DumpBufferToFileOnCondition(lpBook, bRet, &buff, &lFileLen);
    bRet &= SS_Save4TextTip(lpBook, &buff);
    bRet &= SS_DumpBufferToFileOnCondition(lpBook, bRet, &buff, &lFileLen);
    bRet &= SS_Save4ClipboardOptions(lpBook, &buff);
    bRet &= SS_DumpBufferToFileOnCondition(lpBook, bRet, &buff, &lFileLen);
    bRet &= SS_Save4SelColor(lpBook, &buff);
    bRet &= SS_DumpBufferToFileOnCondition(lpBook, bRet, &buff, &lFileLen);
    /*
    bRet &= SS_Save4Print2(lpBook, &buff);
    bRet &= SS_DumpBufferToFileOnCondition(lpBook, bRet, &buff, &lFileLen);
    */
    bRet &= SS_Save4TwoDigitYearMax(lpBook, &buff);
    bRet &= SS_DumpBufferToFileOnCondition(lpBook, bRet, &buff, &lFileLen);
#endif SS_V30

#ifdef SS_V35
    bRet &= SS_Save4CellNoteIndicator(lpBook, &buff);
    bRet &= SS_Save4ShowScrollTips(lpBook, &buff);
    bRet &= SS_Save4ScriptEnhanced(lpBook, &buff);
    bRet &= SS_Save4CalTextOverride(lpBook, &buff);
    bRet &= SS_DumpBufferToFileOnCondition(lpBook, bRet, &buff, &lFileLen);
#endif //SS_V35
  }

#ifdef SS_V70
	//bRet &= SS_Save4ActiveSheet(lpBook, &buff);
	bRet &= SS_Save4TabStrip(lpBook, &buff);
	bRet &= SS_Save4Scroll7(lpBook, &buff);
	bRet &= SS_DumpBufferToFileOnCondition(lpBook, bRet, &buff, &lFileLen);
#endif // SS_V70
#ifdef SS_V80
	bRet &= SS_Save4AppearanceStyle(lpBook, &buff);
	bRet &= SS_Save4TabEnhancedShape(lpBook, &buff);
	bRet &= SS_Save4CellNoteIndicator2(lpBook, &buff);
	bRet &= SS_Save4PrintDevMode(lpBook, &buff);
	bRet &= SS_Save4TabStripButtonPolicy(lpBook, &buff);
	bRet &= SS_Save4HighlightHeaders(lpBook, &buff);
	bRet &= SS_Save4HighlightStyle(lpBook, &buff);
	bRet &= SS_Save4HighlightAlphaBlend(lpBook, &buff);
	bRet &= SS_Save4UseVisualStyles(lpBook, &buff);
	bRet &= SS_Save4EnhanceStaticCells(lpBook, &buff);
	bRet &= SS_Save4ScrollBarStyle(lpBook, &buff);
	bRet &= SS_Save4EnhancedColors(lpBook, &buff);
#endif
	for (nSheet = 0; nSheet < lpBook->nSheetCnt; nSheet++)
	{
		lpSS = SS_BookLockSheetIndex(lpBook, nSheet);

#ifdef SS_V70
		bRet &= SS_Save4SheetIndex(&buff, nSheet);
		bRet &= SS_Save4SheetName(lpSS->hSheetName, &buff);
      bRet &= SS_Save4SheetSettings(lpSS, &buff);
#endif // SS_V70

		bRet &= SS_Save4MaxSize(lpSS, &buff);
      bRet &= SS_DumpBufferToFileOnCondition(lpBook, bRet, &buff, &lFileLen);
      bRet &= SS_Save4PrintSheet(lpSS, &buff);
      bRet &= SS_DumpBufferToFileOnCondition(lpBook, bRet, &buff, &lFileLen);

		if( !bDataOnly )
		{
			bRet &= SS_Save4AutoSize(lpSS, &buff);
			bRet &= SS_DumpBufferToFileOnCondition(lpBook, bRet, &buff, &lFileLen);
			#ifndef SS_NOBORDERS
			bRet &= SS_Save4Border(lpSS, SS_ALLCOLS, SS_ALLROWS, lpSS->hBorder, &buff);
			bRet &= SS_DumpBufferToFileOnCondition(lpBook, bRet, &buff, &lFileLen);
			#endif
			bRet &= SS_Save4BackColorStyle(lpBook, &buff);
			bRet &= SS_DumpBufferToFileOnCondition(lpBook, bRet, &buff, &lFileLen);
			bRet &= SS_Save4Color(lpSS, SS_ALLCOLS, SS_ALLROWS,
  									    lpSS->Color.BackgroundId,
									    lpSS->Color.ForegroundId, &buff);
			bRet &= SS_DumpBufferToFileOnCondition(lpBook, bRet, &buff, &lFileLen);
			bRet &= SS_Save4ColWidth(lpSS, SS_ALLCOLS, lpSS->Col.dCellSizeX100, &buff);
			bRet &= SS_DumpBufferToFileOnCondition(lpBook, bRet, &buff, &lFileLen);
         bRet &= SS_Save4UserResize(lpSS, &buff);
         bRet &= SS_DumpBufferToFileOnCondition(lpBook, bRet, &buff, &lFileLen);
#ifdef SS_V40
			bRet &= SS_Save4Header4(lpSS, &buff);
			bRet &= SS_Save4Display4(lpBook, &buff);
			bRet &= SS_Save4CellTag(SS_ALLCOLS, SS_ALLROWS, lpSS->hCellTag, &buff);
			bRet &= SS_DumpBufferToFileOnCondition(lpBook, bRet, &buff, &lFileLen);
			bRet &= SS_Save4CellSpanTable(lpSS, &buff);
			bRet &= SS_DumpBufferToFileOnCondition(lpBook, bRet, &buff, &lFileLen);
#endif // SS_V40

			#if (defined(SS_BOUNDCONTROL) || defined(SS_QE))
			bRet &= SS_Save4DataAware(lpSS, &buff);
			bRet &= SS_DumpBufferToFileOnCondition(lpBook, bRet, &buff, &lFileLen);
			#endif
			#if (defined(SS_QE))
			bRet &= SS_Save4DataAwareQE(lpSS, &buff);
			bRet &= SS_DumpBufferToFileOnCondition(lpBook, bRet, &buff, &lFileLen);
			#endif
			bRet &= SS_Save4DefFloatFormat(lpSS, &buff);
			bRet &= SS_DumpBufferToFileOnCondition(lpBook, bRet, &buff, &lFileLen);
			bRet &= SS_Save4Font(SS_ALLCOLS, SS_ALLROWS, lpSS->DefaultFontId, &buff);
			bRet &= SS_DumpBufferToFileOnCondition(lpBook, bRet, &buff, &lFileLen);
			bRet &= SS_Save4Frozen(lpSS, &buff);
			bRet &= SS_DumpBufferToFileOnCondition(lpBook, bRet, &buff, &lFileLen);
			bRet &= SS_Save4Grid(lpSS, &buff);
			bRet &= SS_DumpBufferToFileOnCondition(lpBook, bRet, &buff, &lFileLen);
			bRet &= SS_Save4Header(lpSS, &buff);
			bRet &= SS_DumpBufferToFileOnCondition(lpBook, bRet, &buff, &lFileLen);
			bRet &= SS_Save4ItemData(SS_ALLCOLS, SS_ALLROWS, (long)lpSS->lUserData, &buff);
#if defined(_WIN64) || defined(_IA64)
			bRet &= SS_Save4ItemData64(SS_ALLCOLS, SS_ALLROWS, lpSS->lUserData, &buff);
#endif
			bRet &= SS_DumpBufferToFileOnCondition(lpBook, bRet, &buff, &lFileLen);
			bRet &= SS_Save4Lock(SS_ALLCOLS, SS_ALLROWS, (BYTE)lpSS->DocumentLocked, &buff);
			bRet &= SS_DumpBufferToFileOnCondition(lpBook, bRet, &buff, &lFileLen);
			bRet &= SS_Save4LockColor(lpSS, &buff);
			bRet &= SS_DumpBufferToFileOnCondition(lpBook, bRet, &buff, &lFileLen);
			bRet &= SS_Save4OpMode(lpSS, &buff);
			bRet &= SS_DumpBufferToFileOnCondition(lpBook, bRet, &buff, &lFileLen);
			bRet &= SS_Save4Restrict(lpSS, &buff);
			bRet &= SS_DumpBufferToFileOnCondition(lpBook, bRet, &buff, &lFileLen);
			bRet &= SS_Save4RowHeight(SS_ALLROWS, lpSS->Row.dCellSizeX100, &buff);
			bRet &= SS_DumpBufferToFileOnCondition(lpBook, bRet, &buff, &lFileLen);
			bRet &= SS_Save4ShadowColor(lpSS, &buff);
			bRet &= SS_DumpBufferToFileOnCondition(lpBook, bRet, &buff, &lFileLen);
			bRet &= SS_Save4Type(lpSS, SS_ALLCOLS, SS_ALLROWS, &lpSS->DefaultCellType, &buff);
			bRet &= SS_DumpBufferToFileOnCondition(lpBook, bRet, &buff, &lFileLen);
			bRet &= SS_Save4Virtual(lpSS, &buff);
			bRet &= SS_DumpBufferToFileOnCondition(lpBook, bRet, &buff, &lFileLen);
#ifdef SS_V30
			bRet &= SS_Save4OddEvenRowColor(lpSS, &buff);
			bRet &= SS_Save4ScrollBarTrack(lpSS, &buff);
			bRet &= SS_DumpBufferToFileOnCondition(lpBook, bRet, &buff, &lFileLen);
#endif SS_V30
#ifdef SS_V35
			bRet &= SS_Save4UserColAction(lpSS, &buff);
			bRet &= SS_Save4ColUserSortIndicator(SS_ALLCOLS, lpSS->nAllColsSortIndicator,  &buff);
			if (lpSS->hCellNote)
				bRet &= SS_Save4CellNote(SS_ALLCOLS, SS_ALLROWS, lpSS->hCellNote, &buff);
			bRet &= SS_DumpBufferToFileOnCondition(lpBook, bRet, &buff, &lFileLen);
#endif //SS_V35
		}
		else // bDataOnly
		{
#ifdef SS_V40
			bRet &= SS_Save4Header4(lpSS, &buff);
			bRet &= SS_DumpBufferToFileOnCondition(lpBook, bRet, &buff, &lFileLen);
#endif // SS_V40
		}


		bRet &= SS_Save4Value(SS_ALLCOLS, SS_ALLROWS, &lpSS->DefaultData, &buff);
		bRet &= SS_DumpBufferToFileOnCondition(lpBook, bRet, &buff, &lFileLen);
		// Save column information
		for( i = 0; i < lpSS->Col.AllocCnt; i++ )
		{
		 if( lpCol = SS_LockColItem(lpSS, i) )
		 {
			if( !bDataOnly )
			{
			  #ifndef SS_NOBORDERS
			  bRet &= SS_Save4Border(lpSS, i, SS_ALLROWS, lpCol->hBorder, &buff);
			  bRet &= SS_DumpBufferToFileOnCondition(lpBook, bRet, &buff, &lFileLen);
			  #endif
			  bRet &= SS_Save4Color(lpSS, i, SS_ALLROWS, lpCol->Color.BackgroundId,
											lpCol->Color.ForegroundId, &buff);
			  bRet &= SS_DumpBufferToFileOnCondition(lpBook, bRet, &buff, &lFileLen);
			  #if (defined(SS_BOUNDCONTROL) || defined(SS_V40))
			  bRet &= SS_Save4DataAwareCol(lpSS, i, &buff);
			  bRet &= SS_DumpBufferToFileOnCondition(lpBook, bRet, &buff, &lFileLen);
			  #endif
			  bRet &= SS_Save4Font(i, SS_ALLROWS, lpCol->FontId, &buff);
			  bRet &= SS_DumpBufferToFileOnCondition(lpBook, bRet, &buff, &lFileLen);
			  #ifdef SS_OLDCALC
			  bRet &= SS_Save4FormulaUnparsedHandle(i, SS_ALLROWS, lpCol->hCalc, &buff);
			  bRet &= SS_DumpBufferToFileOnCondition(lpBook, bRet, &buff, &lFileLen);
			  #endif
			  #ifndef SS_NOUSERDATA
			  bRet &= SS_Save4ItemData(i, SS_ALLROWS, (long)lpCol->lUserData, &buff);
#if defined(_WIN64) || defined(_IA64)
			  bRet &= SS_Save4ItemData64(i, SS_ALLROWS, lpCol->lUserData, &buff);
#endif
			  bRet &= SS_DumpBufferToFileOnCondition(lpBook, bRet, &buff, &lFileLen);
			  #endif
			  bRet &= SS_Save4Lock(i, SS_ALLROWS, lpCol->ColLocked, &buff);
			  bRet &= SS_DumpBufferToFileOnCondition(lpBook, bRet, &buff, &lFileLen);
			  bRet &= SS_Save4ColPageBreak(i, lpCol->fPageBreak, &buff);
			  bRet &= SS_DumpBufferToFileOnCondition(lpBook, bRet, &buff, &lFileLen);
		#ifdef SS_V30
			  bRet &= SS_Save4ColHidden(i, lpCol->fColHidden, &buff);
			  bRet &= SS_DumpBufferToFileOnCondition(lpBook, bRet, &buff, &lFileLen);
		#endif
			  bRet &= SS_Save4ColWidth(lpSS, i, lpCol->dColWidthX100, &buff);
			  bRet &= SS_DumpBufferToFileOnCondition(lpBook, bRet, &buff, &lFileLen);
		#ifdef SS_V35
			  bRet &= SS_Save4ColUserSortIndicator(i, lpCol->nSortIndicator,  &buff);
			  bRet &= SS_DumpBufferToFileOnCondition(lpBook, bRet, &buff, &lFileLen);
		#endif // SS_V35
			  bRet &= SS_Save4TypeHandle(lpSS, i, SS_ALLROWS, lpCol->hCellType, &buff);
			  bRet &= SS_DumpBufferToFileOnCondition(lpBook, bRet, &buff, &lFileLen);
			  bRet &= SS_Save4UserResizeCol(lpSS, i, lpCol->bUserResize, &buff);
			  bRet &= SS_DumpBufferToFileOnCondition(lpBook, bRet, &buff, &lFileLen);
   
		#ifdef SS_V35
			  if (lpCol->hCellNote)
			  {
				 bRet &= SS_Save4CellNote(i, SS_ALLROWS, lpCol->hCellNote, &buff);
				 bRet &= SS_DumpBufferToFileOnCondition(lpBook, bRet, &buff, &lFileLen);
			  }
		#endif //SS_V35
			}
			bRet &= SS_Save4Value(i, SS_ALLROWS, &lpCol->Data, &buff);
			bRet &= SS_DumpBufferToFileOnCondition(lpBook, bRet, &buff, &lFileLen);
		#ifdef SS_V40
			 bRet &= SS_Save4ColMerge(i, lpCol->bMerge,  &buff);
			 bRet &= SS_Save4ColID(i, lpCol->hColID,  &buff);
			 bRet &= SS_Save4CellTag(i, SS_ALLROWS, lpCol->hCellTag, &buff);
			 bRet &= SS_DumpBufferToFileOnCondition(lpBook, bRet, &buff, &lFileLen);
		#endif // SS_V40
			SS_UnlockColItem(lpSS, i);
		 }
		}
		// Save row information
		for( j = 0; j < lpSS->Row.AllocCnt; j++)
		{
		 if( lpRow = SS_LockRowItem(lpSS, j) )
		 {
			if( !bDataOnly )
			{
			  #ifndef SS_NOBORDERS
			  bRet &= SS_Save4Border(lpSS, SS_ALLCOLS, j, lpRow->hBorder, &buff);
			  bRet &= SS_DumpBufferToFileOnCondition(lpBook, bRet, &buff, &lFileLen);
			  #endif
			  bRet &= SS_Save4Color(lpSS, SS_ALLCOLS, j, lpRow->Color.BackgroundId,
											lpRow->Color.ForegroundId, &buff);
			  bRet &= SS_DumpBufferToFileOnCondition(lpBook, bRet, &buff, &lFileLen);
			  bRet &= SS_Save4Font(SS_ALLCOLS, j, lpRow->FontId, &buff);
			  bRet &= SS_DumpBufferToFileOnCondition(lpBook, bRet, &buff, &lFileLen);
			  #ifdef SS_OLDCALC
			  bRet &= SS_Save4FormulaUnparsedHandle(SS_ALLCOLS, j, lpRow->hCalc, &buff);
			  bRet &= SS_DumpBufferToFileOnCondition(lpBook, bRet, &buff, &lFileLen);
			  #endif
			  #ifndef SS_NOUSERDATA
			  bRet &= SS_Save4ItemData(SS_ALLCOLS, j, (long)lpRow->lUserData, &buff);
#if defined(_WIN64) || defined(_IA64)
			  bRet &= SS_Save4ItemData64(SS_ALLCOLS, j, lpRow->lUserData, &buff);
#endif
			  bRet &= SS_DumpBufferToFileOnCondition(lpBook, bRet, &buff, &lFileLen);
			  #endif
			  bRet &= SS_Save4Lock(SS_ALLCOLS, j, lpRow->RowLocked, &buff);
			  bRet &= SS_DumpBufferToFileOnCondition(lpBook, bRet, &buff, &lFileLen);
			  bRet &= SS_Save4RowPageBreak(j, lpRow->fPageBreak, &buff);
			  bRet &= SS_DumpBufferToFileOnCondition(lpBook, bRet, &buff, &lFileLen);
		#ifdef SS_V30
			  bRet &= SS_Save4RowHidden(j, lpRow->fRowHidden, &buff);
			  bRet &= SS_DumpBufferToFileOnCondition(lpBook, bRet, &buff, &lFileLen);
		#endif
			  bRet &= SS_Save4RowHeight(j, lpRow->dRowHeightX100, &buff);
			  bRet &= SS_DumpBufferToFileOnCondition(lpBook, bRet, &buff, &lFileLen);
			  bRet &= SS_Save4TypeHandle(lpSS, SS_ALLCOLS, j, lpRow->hCellType, &buff);
			  bRet &= SS_DumpBufferToFileOnCondition(lpBook, bRet, &buff, &lFileLen);
			  bRet &= SS_Save4UserResizeRow(lpSS, j, lpRow->bUserResize, &buff);
			  bRet &= SS_DumpBufferToFileOnCondition(lpBook, bRet, &buff, &lFileLen);
		#ifdef SS_V40
			  bRet &= SS_Save4RowMerge(j, lpRow->bMerge,  &buff);
			  bRet &= SS_Save4CellTag(SS_ALLCOLS, j, lpRow->hCellTag, &buff);
			  bRet &= SS_DumpBufferToFileOnCondition(lpBook, bRet, &buff, &lFileLen);
		#endif // SS_V40

		#ifdef SS_V35
			  if (lpRow->hCellNote)
			  {
				 bRet &= SS_Save4CellNote(SS_ALLCOLS, j, lpRow->hCellNote, &buff);
				 bRet &= SS_DumpBufferToFileOnCondition(lpBook, bRet, &buff, &lFileLen);
			  }
		#endif //SS_V35

			}
			bRet &= SS_Save4Value(SS_ALLCOLS, j, &lpRow->Data, &buff);
			bRet &= SS_DumpBufferToFileOnCondition(lpBook, bRet, &buff, &lFileLen);

			// Save cell information
			for( i = 0; i < lpSS->Col.AllocCnt; i++ )
			{
			  if( lpCell = SSx_LockCellItem(lpSS, lpRow, i, j) )
			  {
				 if( !bDataOnly )
				 {
					#ifndef SS_NOBORDERS
					bRet &= SS_Save4Border(lpSS, i, j, lpCell->hBorder, &buff);
					bRet &= SS_DumpBufferToFileOnCondition(lpBook, bRet, &buff, &lFileLen);
					#endif
					bRet &= SS_Save4Color(lpSS, i, j, lpCell->Color.BackgroundId,
												 lpCell->Color.ForegroundId, &buff);
					bRet &= SS_DumpBufferToFileOnCondition(lpBook, bRet, &buff, &lFileLen);
					bRet &= SS_Save4Font(i, j, lpCell->FontId, &buff);
					bRet &= SS_DumpBufferToFileOnCondition(lpBook, bRet, &buff, &lFileLen);
					#ifdef SS_OLDCALC
					bRet &= SS_Save4FormulaUnparsedHandle(i, j, lpCell->hCalc, &buff);
					bRet &= SS_DumpBufferToFileOnCondition(lpBook, bRet, &buff, &lFileLen);
					#endif
					bRet &= SS_Save4Lock(i, j, lpCell->CellLocked, &buff);
					bRet &= SS_DumpBufferToFileOnCondition(lpBook, bRet, &buff, &lFileLen);
					bRet &= SS_Save4TypeHandle(lpSS, i, j, lpCell->hCellType, &buff);
					bRet &= SS_DumpBufferToFileOnCondition(lpBook, bRet, &buff, &lFileLen);

		#ifdef SS_V35
					if (lpCell->hCellNote)
					{
					  bRet &= SS_Save4CellNote(i, j, lpCell->hCellNote, &buff);
					  bRet &= SS_DumpBufferToFileOnCondition(lpBook, bRet, &buff, &lFileLen);
					}     
		#endif //SS_V35
		#ifdef SS_V40
					bRet &= SS_Save4CellTag(i, j, lpCell->hCellTag, &buff);
					bRet &= SS_DumpBufferToFileOnCondition(lpBook, bRet, &buff, &lFileLen);
		#endif // SS_V40
				 }
				 bRet &= SS_Save4Value(i, j, &lpCell->Data, &buff);
				 bRet &= SS_DumpBufferToFileOnCondition(lpBook, bRet, &buff, &lFileLen);

				 SS_UnlockCellItem(lpSS, i, j);
			  }
			}
			SS_UnlockRowItem(lpSS, j);
		 }
		}
		#if !defined(SS_NOCALC) && !defined(SS_OLDCALC)
		if( !bDataOnly )
		{
		 CALC_HANDLE hExpr;
		 CALC_HANDLE hCell;
		 CALC_HANDLE hCol;
		 CALC_HANDLE hRow;
		 BOOL bNeedsEval;
		 bRet &= SS_Save4DdeTable(lpSS, &buff);
		 bRet &= SS_DumpBufferToFileOnCondition(lpBook, bRet, &buff, &lFileLen);
		 bRet &= SS_DumpBufferToFileOnCondition(lpBook, bRet, &buff, &lFileLen);
		 hCol = VectGetFirst(&lpSS->CalcInfo.Cols, &i);
		 while( hCol )
		 {
			if( hExpr = ColGetExpr(&lpSS->CalcInfo, i) )
			{
			  bNeedsEval = CalcIsColMarkedForEval(&lpSS->CalcInfo, i);
			  lColAdjust = i;
			  SS_AdjustCellCoords(lpSS, &lColAdjust, NULL);
			  bRet &= SS_Save4Formula(lpSS, lColAdjust, SS_ALLROWS, hExpr, bNeedsEval, &buff);
			  bRet &= SS_DumpBufferToFileOnCondition(lpBook, bRet, &buff, &lFileLen);
			}
			hCol = VectGetNext(&lpSS->CalcInfo.Cols, &i);
		 }
		 hRow = VectGetFirst(&lpSS->CalcInfo.Rows, &j);
		 while( hRow )
		 {
			if( hExpr = RowGetExpr(&lpSS->CalcInfo, j) )
			{
			  bNeedsEval = CalcIsRowMarkedForEval(&lpSS->CalcInfo, j);
			  lRowAdjust = j;
			  SS_AdjustCellCoords(lpSS, NULL, &lRowAdjust);
			  bRet &= SS_Save4Formula(lpSS, SS_ALLCOLS, lRowAdjust, hExpr, bNeedsEval, &buff);
			  bRet &= SS_DumpBufferToFileOnCondition(lpBook, bRet, &buff, &lFileLen);
			}
			hRow = VectGetNext(&lpSS->CalcInfo.Rows, &j);
		 }
		 hCell = MatGetFirst(&lpSS->CalcInfo.Cells, &i, &j);
		 while( hCell )
		 {
			if( hExpr = CellGetExpr(&lpSS->CalcInfo, i, j) )
			{
			  bNeedsEval = CalcIsCellMarkedForEval(&lpSS->CalcInfo, i, j);
			  lColAdjust = i;
			  lRowAdjust = j;
			  SS_AdjustCellCoords(lpSS, &lColAdjust, &lRowAdjust);
			  bRet &= SS_Save4Formula(lpSS, lColAdjust, lRowAdjust, hExpr, bNeedsEval, &buff);
			  bRet &= SS_DumpBufferToFileOnCondition(lpBook, bRet, &buff, &lFileLen);
			}
			hCell = MatGetNext(&lpSS->CalcInfo.Cells, &i, &j);
		 }
		}
		#endif

		SS_BookUnlockSheetIndex(lpBook, nSheet);
	} // for

  if( !bDataOnly )
  {
    bRet &= SS_Save4Xtra(lpBook, &buff, recFile.bFileVer);
    bRet &= SS_DumpBufferToFileOnCondition(lpBook, bRet, &buff, &lFileLen);
  }

#ifdef WIN32
  if (lpBook->hFile && bRet && buff.lLen)
    bRet &= SS_DumpBufferToFile(lpBook, &buff, &lFileLen);
#endif
  
//RAP03a >>
#if defined(SS_V30) && defined(WIN32)
  if (lpBook->hFile && bRet && lFileLen)
  {
    if (0xFFFFFFFF != SetFilePointer(lpBook->hFile, 0, NULL, FILE_BEGIN))    
    {
      SS_BuffInit(&buff);
      // Save file information
      recFile.lFileLen = lFileLen;
      SS_BuffAppend(&buff, &recFile, sizeof(recFile));
      SS_DumpBufferToFile(lpBook, &buff, NULL);
    }
  }
  else if( bRet && buff.pMem )
#else
//RAP03a <<
  if( bRet && buff.pMem )
#endif
  {
    ((LPSS_REC_FILE)buff.pMem)->lFileLen = buff.lLen;
    ssGlobalUnlock(buff.hMem); //RAP01c
  }
  else
    SS_BuffFree(&buff);  // this resets hMem and lLen fields to zero
  if( lplBuffLen )
    *lplBuffLen = buff.lLen;

//RAP01a >>
#ifdef WIN32
  if (!lpBook->hFile)
  {
  BYTE HUGE *hpSrc, *hpDest;
  ghReturn = GlobalAlloc(GHND, buff.lLen);
  hpSrc = (BYTE HUGE *)tbGlobalLock(buff.hMem);
  hpDest = (BYTE HUGE *)GlobalLock(ghReturn);
  MemHugeCpy((HPVOID)hpDest, (HPCVOID)hpSrc, buff.lLen);
  tbGlobalUnlock(buff.hMem);
  GlobalUnlock(ghReturn);
  SS_BuffFree(&buff);
  }

  if (lpfRet)
    *lpfRet = bRet;

  return ghReturn;
#else
  return buff.hMem;
#endif
//RAP01a <<

//RAP01d  return buff.hMem; 
}

//--------------------------------------------------------------------

BOOL SS_LoadBlockFromBuffer4(LPSPREADSHEET lpSS,
                             SS_COORD lCol, SS_COORD lRow,
                             SS_COORD lCol2, SS_COORD lRow2,
                             HPBYTE lpBuff, long lBuffLen)
{
  LPSS_REC_COMMON lpRec;
  LPSS_REC_BLOCKHEADER lpBlockRec;
  SS_FONTBUFFER fontBuff = {0, 0, 0, 0}; // pMem, phFont, lLen, lAlloc
  SS_CALCIDTABLE savedCustFuncs = {0, 0, 0}; // hElem, lpElem, nElemCnt
  SS_CALCIDTABLE savedCustNames = {0, 0, 0}; // hElem, lpElem, nElemCnt
  SS_CALCIDTABLE savedDdeLinks = {0, 0, 0}; // hElem, lpElem, nElemCnt
#ifdef SS_V80
    SS_CELLIDTABLE savedCustCells = {0, 0, 0}; // hElem, lpElem, nElemCnt
#endif
  SS_LOADBLOCK loadBlock;
  BOOL bUnicode;
  BOOL bAll;
  BOOL bRedraw;
  BOOL bRet = TRUE;
  int i;

  if( SS_ALLCOLS == lCol || SS_ALLCOLS == lCol2 )
    lCol = lCol2 = SS_ALLCOLS;
  if( SS_ALLROWS == lRow || SS_ALLROWS == lRow2 )
    lRow = lRow2 = SS_ALLROWS;
  // Process block header
  lpBlockRec = (LPSS_REC_BLOCKHEADER)lpBuff;
  bAll = lpBlockRec->bType & SS_FILETYPE_ALL;
  bUnicode = lpBlockRec->bType & SS_FILETYPE_UNICODE;
  bRet = SS_SAVEVERSION4 == lpBlockRec->bVer;
  bRet &= (bAll | bUnicode) == lpBlockRec->bType;
  bRet &= lBuffLen == lpBlockRec->lLen;
  bRet &= !((SS_ALLCOLS == lpBlockRec->lCol) ^ (SS_ALLCOLS == lCol));
  bRet &= !((SS_ALLROWS == lpBlockRec->lRow) ^ (SS_ALLROWS == lRow));
  if( bRet )
  {
    SS_COORD lColsInRange = lCol2 - lCol;
    SS_COORD lRowsInRange = lRow2 - lRow;
    SS_COORD lColsInBlock = lpBlockRec->lCol2 - lpBlockRec->lCol;
    SS_COORD lRowsInBlock = lpBlockRec->lRow2 - lpBlockRec->lRow;
    SS_COORD lClearCol2 = lCol + min(lColsInRange, lColsInBlock);
    SS_COORD lClearRow2 = lRow + min(lRowsInRange, lRowsInBlock);
    loadBlock.lColOffset = lCol - lpBlockRec->lCol;
    loadBlock.lRowOffset = lRow - lpBlockRec->lRow;
    loadBlock.lCol = lCol;
    loadBlock.lRow = lRow;
    loadBlock.lCol2 = lCol2;
    loadBlock.lRow2 = lRow2;
    bRedraw = SS_BookSetRedraw(lpSS->lpBook, FALSE);
    SS_ClearRange(lpSS, lCol, lRow, lClearCol2, lClearRow2, FALSE);
    // Skip block header
    lpRec = (LPSS_REC_COMMON)(lpBuff + sizeof(SS_REC_BLOCKHEADER));
    // Process each record in buffer
    while( bRet && (HPBYTE)lpRec < lpBuff + lBuffLen )
    {
      switch( lpRec->nRecType )
      {
        case SS_RID_BORDER: bRet = SS_Load4Border(lpSS, lpRec, &loadBlock); break;
        case SS_RID_BORDER_EXT: bRet = SS_Load4BorderExt(lpSS, lpRec, &loadBlock); break;
        case SS_RID_COLOR: bRet = SS_Load4Color(lpSS, lpRec, &loadBlock); break;
        case SS_RID_CUSTFUNCTABLE: bRet = SS_Load4CustFuncTable(lpSS->lpBook, lpRec, &savedCustFuncs, bUnicode); break;
        case SS_RID_CUSTNAMETABLE: bRet = SS_Load4CustNameTable(lpSS->lpBook, lpRec, &savedCustFuncs, &savedCustNames, &savedDdeLinks, bUnicode); break;
        case SS_RID_DDETABLE: bRet = SS_Load4DdeTable(lpSS, lpRec, &savedDdeLinks, bUnicode); break;
        case SS_RID_FONT: bRet = SS_Load4Font(lpSS, lpRec, &fontBuff, &loadBlock); break;
        case SS_RID_FONTTABLE: bRet = SS_Load4FontTable(lpSS, lpRec, &fontBuff, bUnicode); break;
        case SS_RID_FORMULA: bRet = SS_Load4Formula(lpSS, lpRec, &savedCustFuncs, &savedCustNames, &savedDdeLinks, bUnicode, &loadBlock); break;
        case SS_RID_LOCK: bRet = SS_Load4Lock(lpSS, lpRec, &loadBlock); break;
        case SS_RID_TYPEBUTTON: bRet = SS_Load4TypeButton(lpSS, lpRec, bUnicode, &loadBlock); break;
        case SS_RID_TYPECHECK: bRet = SS_Load4TypeCheck(lpSS, lpRec, bUnicode, &loadBlock); break;
        case SS_RID_TYPECOMBO: bRet = SS_Load4TypeCombo(lpSS, lpRec, bUnicode, &loadBlock); break;
        case SS_RID_TYPECOMBOEX: bRet = SS_Load4TypeComboEx(lpSS, lpRec, bUnicode, &loadBlock); break;
#ifdef SS_V40
        case SS_RID_TYPECOMBOEX4: bRet = SS_Load4TypeComboEx4(lpSS, lpRec, bUnicode, &loadBlock); break;
#endif // SS_V40
        case SS_RID_TYPEDATE: bRet = SS_Load4TypeDate(lpSS, lpRec, &loadBlock); break;
        case SS_RID_TYPEEDIT: bRet = SS_Load4TypeEdit(lpSS, lpRec, &loadBlock); break;
        case SS_RID_TYPEFLOAT: bRet = SS_Load4TypeFloat(lpSS, lpRec, &loadBlock); break;
        case SS_RID_TYPEINTEGER: bRet = SS_Load4TypeInteger(lpSS, lpRec, &loadBlock); break;
        case SS_RID_TYPEOWNERDRAW: bRet = SS_Load4TypeOwnerDraw(lpSS, lpRec, &loadBlock); break;
        case SS_RID_TYPEPIC: bRet = SS_Load4TypePic(lpSS, lpRec, bUnicode, &loadBlock); break;
        case SS_RID_TYPEPICTURE: bRet = SS_Load4TypePicture(lpSS, lpRec, bUnicode, &loadBlock); break;
        case SS_RID_TYPESTATIC: bRet = SS_Load4TypeStatic(lpSS, lpRec, &loadBlock); break;
        case SS_RID_TYPETIME: bRet = SS_Load4TypeTime(lpSS, lpRec, &loadBlock); break;
        case SS_RID_VALLONG: bRet = SS_Load4ValLong(lpSS, lpRec, &loadBlock); break;
        case SS_RID_VALDBL: bRet = SS_Load4ValDbl(lpSS, lpRec, &loadBlock); break;
        case SS_RID_VALSTR: bRet = SS_Load4ValStr(lpSS, lpRec, bUnicode, &loadBlock); break;
#ifdef SS_V35
        case SS_RID_CELLNOTE: bRet = SS_Load4CellNote(lpSS, lpRec, bUnicode, &loadBlock); break;
#endif
#ifdef SS_V40
        case SS_RID_TYPECURRENCY: bRet = SS_Load4TypeCurrency(lpSS, lpRec, &loadBlock, bUnicode); break;
        case SS_RID_TYPENUMBER: bRet = SS_Load4TypeNumber(lpSS, lpRec, &loadBlock, bUnicode); break;
        case SS_RID_TYPEPERCENT: bRet = SS_Load4TypePercent(lpSS, lpRec, &loadBlock, bUnicode); break;
        case SS_RID_HEADER4: bRet = SS_Load4Header4(lpSS, lpRec); break;
        case SS_RID_CELLTAG: bRet = SS_Load4CellTag(lpSS, lpRec, bUnicode, &loadBlock); break;
        case SS_RID_COLMERGE: bRet = SS_Load4ColMerge(lpSS, lpRec); break;
        case SS_RID_COLID: bRet = SS_Load4ColID(lpSS, lpRec, bUnicode); break;
        case SS_RID_ROWMERGE: bRet = SS_Load4RowMerge(lpSS, lpRec); break;
        case SS_RID_CELLSPANTABLE: bRet = SS_Load4CellSpanTable(lpSS, lpRec); break;
#endif // SS_V40
#ifdef SS_V70
        case SS_RID_TYPESCIENTIFIC: bRet = SS_Load4TypeScientific(lpSS, lpRec, &loadBlock, bUnicode); break;
#endif // SS_V70
#ifdef SS_V80
        case SS_RID_COLWIDTH: bRet = SS_Load4ColWidth(lpSS, lpRec, &loadBlock); break;
        case SS_RID_ROWHEIGHT: bRet = SS_Load4RowHeight(lpSS, lpRec, &loadBlock); break;
        case SS_RID_CUSTCELLTABLE: bRet = SS_Load4CustCellTable(lpSS->lpBook, lpRec, &savedCustCells, bUnicode); break;
        case SS_RID_TYPECUSTOM: bRet = SS_Load4TypeCustom(lpSS, lpRec, bUnicode, &loadBlock);
#endif // SS_V80

        default: bRet = lpRec->lRecLen >= sizeof(SS_REC_COMMON); break;
      }
      lpRec = (LPSS_REC_COMMON)((HPBYTE)lpRec + lpRec->lRecLen);
    }
    SS_BookSetRedraw(lpSS->lpBook, bRedraw);
    if( fontBuff.hMem )
    {
      for( i = 0; i < fontBuff.lLen; i++ )
        DeleteObject(fontBuff.phFont[i]);
      GlobalUnlock(fontBuff.hMem);
      GlobalFree(fontBuff.hMem);
    }
    if( savedCustFuncs.hElem )
    {
      for( i = 0; i < savedCustFuncs.nElemCnt; i++ )
        CustFuncDestroy(savedCustFuncs.lpElem[i]);
      tbGlobalUnlock(savedCustFuncs.hElem);
      tbGlobalFree(savedCustFuncs.hElem);
    }
    if( savedCustNames.hElem )
    {
      for( i = 0; i < savedCustNames.nElemCnt; i++ )
        NameDestroy(savedCustNames.lpElem[i]);
      tbGlobalUnlock(savedCustNames.hElem);
      tbGlobalFree(savedCustNames.hElem);
    }
    #if defined(SS_DDE)
    if( savedDdeLinks.hElem )
    {
      for( i = 0; i < savedDdeLinks.nElemCnt; i++ )
        DdeDestroy(savedDdeLinks.lpElem[i]);
      tbGlobalUnlock(savedDdeLinks.hElem);
      tbGlobalFree(savedDdeLinks.hElem);
    }
    #endif
  }
  return bRet;
}

//--------------------------------------------------------------------

GLOBALHANDLE SS_SaveBlockToBuffer4(LPSPREADSHEET lpSS, SS_COORD lCol, SS_COORD lRow, SS_COORD lCol2, SS_COORD lRow2,
                                   LPLONG lplBuffLen, BOOL  bDataOnly, BOOL fSaveColInfo, BOOL fSaveRowInfo)
{
  SS_BUFF buff;
  SS_REC_BLOCKHEADER recBlock;
  LPSS_CELL lpCell;
  LPSS_COL lpCol;
  LPSS_ROW lpRow;
  SS_COORD lColMin, lColMax;
  SS_COORD lRowMin, lRowMax;
  SS_COORD i, j;
  GLOBALHANDLE ghReturn = (GLOBALHANDLE)0;  //RAP01a
  BOOL bRet = TRUE;
  LPSS_BOOK lpBook = lpSS->lpBook;

  SS_BuffInit(&buff);
  if( SS_ALLCOLS == lCol || SS_ALLCOLS == lCol2 )
    lCol = lCol2 = SS_ALLCOLS;
  if( SS_ALLROWS == lRow || SS_ALLROWS == lRow2 )
    lRow = lRow2 = SS_ALLROWS;
  // Save block header information
  recBlock.bType = bDataOnly ? SS_FILETYPE_DATA : SS_FILETYPE_ALL;
  #if defined(_UNICODE)
  recBlock.bType |= SS_FILETYPE_UNICODE;
  #endif
  recBlock.bVer = SS_SAVEVERSION4;
  recBlock.lLen = 0;
  recBlock.lCol = lCol;
  recBlock.lRow = lRow;
  recBlock.lCol2 = lCol2;
  recBlock.lRow2 = lRow2;
  bRet &= SS_BuffAppend(&buff, &recBlock, sizeof(recBlock));
  //Save spreadsheet information
  if( !bDataOnly )
  {
    bRet &= SS_Save4CustFuncTable(lpBook, &buff);
    bRet &= SS_Save4CustNameTable(lpBook, FALSE, &buff);
    bRet &= SS_Save4FontTable(&buff);
#ifdef SS_V80
    bRet &= SS_Save4CustCellTable(lpBook, &buff);
#endif
  }
  if( SS_ALLCOLS == lCol && SS_ALLROWS == lRow )
  {
    if( !bDataOnly )
    {
      #ifndef SS_NOBORDERS
      bRet &= SS_Save4Border(lpSS, SS_ALLCOLS, SS_ALLROWS, lpSS->hBorder, &buff);
      #endif
      bRet &= SS_Save4Color(lpSS, SS_ALLCOLS, SS_ALLROWS, lpSS->Color.BackgroundId,
                            lpSS->Color.ForegroundId, &buff);
      bRet &= SS_Save4Font(SS_ALLCOLS, SS_ALLROWS, lpSS->DefaultFontId, &buff);
      bRet &= SS_Save4Lock(SS_ALLCOLS, SS_ALLROWS, (BYTE)lpSS->DocumentLocked, &buff);
      bRet &= SS_Save4Type(lpSS, SS_ALLCOLS, SS_ALLROWS, &lpSS->DefaultCellType, &buff);

#ifdef SS_V40
      bRet &= SS_Save4Header4(lpSS, &buff);
      bRet &= SS_Save4Display4(lpBook, &buff);
      bRet &= SS_Save4CellTag(SS_ALLCOLS, SS_ALLROWS, lpSS->hCellTag, &buff);
      bRet &= SS_Save4CellSpanTable(lpSS, &buff);
#endif // SS_V40

    }
    bRet &= SS_Save4Value(SS_ALLCOLS, SS_ALLROWS, &lpSS->DefaultData, &buff);
  }
  // Save column information
  if( SS_ALLCOLS == lCol )
  {
    lColMin = 0;
    lColMax = lpSS->Col.AllocCnt;
  }
  else
  {
    lColMin = lCol;
    lColMax = lCol2;
  }
#ifdef SS_V80
  if( SS_ALLROWS == lRow || fSaveColInfo)
#else
  if( SS_ALLROWS == lRow )
#endif
  {
    for( i = lColMin; i <= lColMax; i++ )
    {
      if( lpCol = SS_LockColItem(lpSS, i) )
      {
        if( !bDataOnly )
        {
          #ifndef SS_NOBORDERS
          bRet &= SS_Save4Border(lpSS, i, SS_ALLROWS, lpCol->hBorder, &buff);
          #endif
          bRet &= SS_Save4Color(lpSS, i, SS_ALLROWS, lpCol->Color.BackgroundId,
                                lpCol->Color.ForegroundId, &buff);
          bRet &= SS_Save4Font(i, SS_ALLROWS, lpCol->FontId, &buff);
          bRet &= SS_Save4Lock(i, SS_ALLROWS, lpCol->ColLocked, &buff);
          bRet &= SS_Save4TypeHandle(lpSS, i, SS_ALLROWS, lpCol->hCellType, &buff);
#ifdef SS_V40
          bRet &= SS_Save4ColMerge(i, lpCol->bMerge,  &buff);
          bRet &= SS_Save4ColID(i, lpCol->hColID,  &buff);
          bRet &= SS_Save4CellTag(i, SS_ALLROWS, lpCol->hCellTag, &buff);
#endif // SS_V40
#ifdef SS_V80
          bRet &= SS_Save4ColWidth(lpSS, i, lpCol->dColWidthX100, &buff);
#endif // SS_V80
        }
        bRet &= SS_Save4Value(i, SS_ALLROWS, &lpCol->Data, &buff);
        SS_UnlockColItem(lpSS, i);
      }
    }
  }
  // Save row information
  if( SS_ALLROWS == lRow )
  {
    lRowMin = 0;
    lRowMax = lpSS->Row.AllocCnt;
  }
  else
  {
    lRowMin = lRow;
    lRowMax = lRow2;
  }
  for( j = lRowMin; j <= lRowMax; j++ )
  {
    if( lpRow = SS_LockRowItem(lpSS, j) )
    {
#ifdef SS_V80
      if( SS_ALLCOLS == lCol || fSaveRowInfo)
#else
      if( SS_ALLCOLS == lCol )
#endif
      {
        if( !bDataOnly )
        {
          #ifndef SS_NOBORDERS
          bRet &= SS_Save4Border(lpSS, SS_ALLCOLS, j, lpRow->hBorder, &buff);
          #endif
          bRet &= SS_Save4Color(lpSS, SS_ALLCOLS, j, lpRow->Color.BackgroundId,
                                lpRow->Color.ForegroundId, &buff);
          bRet &= SS_Save4Font(SS_ALLCOLS, j, lpRow->FontId, &buff);
          bRet &= SS_Save4Lock(SS_ALLCOLS, j, lpRow->RowLocked, &buff);
          bRet &= SS_Save4TypeHandle(lpSS, SS_ALLCOLS, j, lpRow->hCellType, &buff);
#ifdef SS_V40
          bRet &= SS_Save4RowMerge(j, lpRow->bMerge,  &buff);
          bRet &= SS_Save4CellTag(SS_ALLCOLS, j, lpRow->hCellTag, &buff);
#endif // SS_V40
#ifdef SS_V80
			  bRet &= SS_Save4RowHeight(j, lpRow->dRowHeightX100, &buff);
#endif // SS_V80

        }
        bRet &= SS_Save4Value(SS_ALLCOLS, j, &lpRow->Data, &buff);
      }
      for( i = lColMin; i <= lColMax; i++ )
      {
        if( lpCell = SSx_LockCellItem(lpSS, lpRow, i, j) )
        {
          if( !bDataOnly )
          {
            #ifndef SS_NOBORDERS
            bRet &= SS_Save4Border(lpSS, i, j, lpCell->hBorder, &buff);
            #endif
            bRet &= SS_Save4Color(lpSS, i, j, lpCell->Color.BackgroundId,
                                  lpCell->Color.ForegroundId, &buff);
            bRet &= SS_Save4Font(i, j, lpCell->FontId, &buff);
            bRet &= SS_Save4Lock(i, j, lpCell->CellLocked, &buff);
            bRet &= SS_Save4TypeHandle(lpSS, i, j, lpCell->hCellType, &buff);
#ifdef SS_V40
            bRet &= SS_Save4CellTag(i, j, lpCell->hCellTag, &buff);
#endif // SS_V40
          }
          bRet &= SS_Save4Value(i, j, &lpCell->Data, &buff);
#ifdef SS_V35
          bRet &= SS_Save4CellNote(i, j, lpCell->hCellNote, &buff);
#endif //SS_V35
          SS_UnlockCellItem(lpSS, i, j);
        }
      }
      SS_UnlockRowItem(lpSS, j);
    }
  }
  #ifndef SS_NOCALC
  if( !bDataOnly )
  {
    SS_COORD lColAdjust, lRowAdjust;
    CALC_HANDLE hExpr;
    BOOL bNeedsEval;
    bRet &= SS_Save4DdeTable(lpSS, &buff);
    if( SS_ALLCOLS == lCol )
    {
      lColMin = min(MatMinCol(&lpSS->CalcInfo.Cells), VectMinIndex(&lpSS->CalcInfo.Cols));
      lColMax = max(MatMaxCol(&lpSS->CalcInfo.Cells), VectMaxIndex(&lpSS->CalcInfo.Cols));
    }
    else
    {
      lColMin = lCol;
      lColMax = lCol2;
      SS_AdjustCellCoordsOut(lpSS, &lColMin, NULL);
      SS_AdjustCellCoordsOut(lpSS, &lColMax, NULL);
    }
    lColMin = max(lColMin, 1);

    if( SS_ALLROWS == lRow )
    {
      lRowMin = min(MatMinRow(&lpSS->CalcInfo.Cells), VectMinIndex(&lpSS->CalcInfo.Rows));
      lRowMax = max(MatMaxRow(&lpSS->CalcInfo.Cells), VectMaxIndex(&lpSS->CalcInfo.Rows));
    }
    else
    {
      lRowMin = lRow;
      lRowMax = lRow2;
      SS_AdjustCellCoordsOut(lpSS, NULL, &lRowMin);
      SS_AdjustCellCoordsOut(lpSS, NULL, &lRowMax);
    }
    lRowMin = max(lRowMin, 1);

    if( SS_ALLROWS == lRow )
    {
      for( i = lColMin; i <= lColMax; i++ )
      {
        if( (hExpr = ColGetExpr(&lpSS->CalcInfo, i)) )
        {
          bNeedsEval = CalcIsColMarkedForEval(&lpSS->CalcInfo, i);
          lColAdjust = i;
          SS_AdjustCellCoords(lpSS, &lColAdjust, NULL);
          bRet &= SS_Save4Formula(lpSS, lColAdjust, SS_ALLROWS, hExpr, bNeedsEval, &buff);
        }
      }
    }
    if( SS_ALLCOLS == lCol )
    {
      for( j = lRowMin; j <= lRowMax; j++ )
      {
        if( (hExpr = RowGetExpr(&lpSS->CalcInfo, j)) )
        {
          bNeedsEval = CalcIsRowMarkedForEval(&lpSS->CalcInfo, j);
          lRowAdjust = j;
          SS_AdjustCellCoords(lpSS, NULL, &lRowAdjust);
          bRet &= SS_Save4Formula(lpSS, SS_ALLCOLS, lRowAdjust, hExpr, bNeedsEval, &buff);
        }
      }
    }
    for( i = lColMin; i <= lColMax; i++ )
    {
      for( j = lRowMin; j <= lRowMax; j++ )
      {
        if( (hExpr = CellGetExpr(&lpSS->CalcInfo, i, j)) )
        {
          bNeedsEval = CalcIsCellMarkedForEval(&lpSS->CalcInfo, i, j);
          lColAdjust = i;
          lRowAdjust = j;
          SS_AdjustCellCoords(lpSS, &lColAdjust, &lRowAdjust);
          bRet &= SS_Save4Formula(lpSS, lColAdjust, lRowAdjust, hExpr, bNeedsEval, &buff);
        }
      }
    }
  }
  #endif
  if( bRet && buff.pMem )
  {
    ((LPSS_REC_BLOCKHEADER)buff.pMem)->lLen = buff.lLen;
    ssGlobalUnlock(buff.hMem); //RAP01c
  }
  else
    SS_BuffFree(&buff);  // this resets hMem and lLen fields to zero
  if( lplBuffLen )
    *lplBuffLen = buff.lLen;

//RAP01a >>
#ifdef WIN32
  {
  BYTE HUGE *hpSrc, *hpDest;
  ghReturn = GlobalAlloc(GHND, buff.lLen);
  hpSrc = (BYTE HUGE *)tbGlobalLock(buff.hMem);
  hpDest = (BYTE HUGE *)GlobalLock(ghReturn);
  MemHugeCpy((HPVOID)hpDest, (HPCVOID)hpSrc, buff.lLen);
  tbGlobalUnlock(buff.hMem);
  GlobalUnlock(ghReturn);
  SS_BuffFree(&buff);
  }
  return ghReturn;
#else
  return buff.hMem;
#endif
//RAP01a

//RAP01d return buff.hMem;
}

//--------------------------------------------------------------------

BOOL DLLENTRY SSLoadBlockFromBuffer(HWND hWnd,
                                   SS_COORD lCol, SS_COORD lRow,
                                   SS_COORD lCol2, SS_COORD lRow2,
                                   HPBYTE lpBuff, long lBuffLen)
{
  LPSPREADSHEET lpSS;
  BOOL bRet;

  lpSS = SS_SheetLock(hWnd);
  SS_AdjustCellRangeCoords(lpSS, &lCol, &lRow, &lCol2, &lRow2);
  bRet = SS_LoadBlockFromBuffer4(lpSS, lCol, lRow, lCol2, lRow2,
                                 lpBuff, lBuffLen);
  SS_SheetUnlock(hWnd);
  return bRet;
}

//--------------------------------------------------------------------

GLOBALHANDLE DLLENTRY SSSaveBlockToBuffer(HWND hWnd, SS_COORD lCol, SS_COORD lRow,
                                          SS_COORD lCol2, SS_COORD lRow2,
                                          LPLONG lplBuffLen, BOOL  bDataOnly)
{
  LPSPREADSHEET lpSS;
  GLOBALHANDLE hRet;

  lpSS = SS_SheetLock(hWnd);

  SS_AdjustCellRangeCoords(lpSS, &lCol, &lRow, &lCol2, &lRow2);
  hRet = SS_SaveBlockToBuffer4(lpSS, lCol, lRow, lCol2, lRow2,
                               lplBuffLen, bDataOnly, FALSE, FALSE);
  SS_SheetUnlock(hWnd);
  return hRet;
}

//--------------------------------------------------------------------

GLOBALHANDLE DLLENTRY SSSaveBlockToBufferExt(HWND hWnd, SS_COORD lCol, SS_COORD lRow,
                                             SS_COORD lCol2, SS_COORD lRow2,
                                             LPLONG lplBuffLen, BOOL  bDataOnly, BOOL fSaveColInfo, BOOL fSaveRowInfo)
{
  LPSPREADSHEET lpSS;
  GLOBALHANDLE hRet;

  lpSS = SS_SheetLock(hWnd);

  SS_AdjustCellRangeCoords(lpSS, &lCol, &lRow, &lCol2, &lRow2);
  hRet = SS_SaveBlockToBuffer4(lpSS, lCol, lRow, lCol2, lRow2,
                               lplBuffLen, bDataOnly, fSaveColInfo, fSaveRowInfo);
  SS_SheetUnlock(hWnd);
  return hRet;
}

//--------------------------------------------------------------------

HGLOBAL DLLENTRY SSGetXtra(HWND hwndSS, LPLONG lplXtraLen, LPBYTE lpbXtraVer)
{
  LPSPREADSHEET lpSS;
  LPVOID lpXtra;
  LPVOID lpCopy;
  HGLOBAL hCopy;
  HGLOBAL hRet = NULL;
  long lRetLen = 0;
  BYTE bRetVer = 0;

  lpSS = SS_SheetLock(hwndSS);
  if( lpSS->lpBook->hXtra && lpSS->lpBook->lXtraLen && (lpXtra = GlobalLock(lpSS->lpBook->hXtra)) )
  {
    if( (hCopy = GlobalAlloc(GHND, lpSS->lpBook->lXtraLen)) && (lpCopy = GlobalLock(hCopy)) )
    {
      MemHugeCpy(lpCopy, lpXtra, lpSS->lpBook->lXtraLen);
      hRet = hCopy;
      lRetLen = lpSS->lpBook->lXtraLen;
      bRetVer = lpSS->lpBook->bXtraVer;
      GlobalUnlock(hCopy);
    }
    GlobalUnlock(lpSS->lpBook->hXtra);
  }
  SS_SheetUnlock(hwndSS);
  if( lplXtraLen )
    *lplXtraLen = lRetLen;
  if( lpbXtraVer )
    *lpbXtraVer = bRetVer;
  return hRet;
}

//--------------------------------------------------------------------

BOOL DLLENTRY SSSetXtra(HWND hwndSS, HGLOBAL hXtra, long lXtraLen, BYTE bXtraVer)
{
  LPSPREADSHEET lpSS;
  LPVOID lpXtra;
  LPVOID lpCopy;
  HGLOBAL hCopy = NULL;

  lpSS = SS_SheetLock(hwndSS);
  if( lpSS->lpBook->hXtra )
    GlobalFree(lpSS->lpBook->hXtra);
  if( hXtra && lXtraLen && (lpXtra = GlobalLock(hXtra)) )
  {
    if( (hCopy = GlobalAlloc(GHND, lXtraLen)) && (lpCopy = GlobalLock(hCopy)) )
    {
      MemHugeCpy(lpCopy, lpXtra, lXtraLen);
      GlobalUnlock(hCopy);
    }
    GlobalUnlock(hXtra);
  }
  lpSS->lpBook->hXtra = hCopy;
  lpSS->lpBook->lXtraLen = lXtraLen;
  lpSS->lpBook->bXtraVer = bXtraVer;
  SS_SheetUnlock(hwndSS);
  return TRUE;
}


#endif // SS_OLDSAVE
