//--------------------------------------------------------------------
//
//  File: cal_expr.c
//
//  Description: Expression evaluation routines
//

#include <ctype.h>
#include <errno.h>
#include <string.h>
#include <stdio.h>
#include <stdlib.h>
#include <locale.h>

#include "calc.h"
#include "cal_cell.h"
#include "cal_col.h"
#include "cal_cust.h"
#include "cal_dde.h"
#include "cal_err.h"
#include "cal_expr.h"
#include "cal_func.h"
#include "cal_mem.h"
#include "cal_name.h"
#include "cal_oper.h"
#include "cal_row.h"
#include "cal_tok.h"

void CalcStringReplaceCh(LPTSTR szStr, TCHAR cNew, TCHAR cOld);
double CalcStringToDouble(LPCTSTR lpText);
TCHAR  CalcGetDefDecimalChar(void);

//--------------------------------------------------------------------
//
//  The VALUESTACK_ALLOCINC constant determines the rate at which
//  a CALC_VALUESTACK grows.
//

#define VALUESTACK_ALLOCINC 32

//--------------------------------------------------------------------
//
//  The CALC_VALUESTACK structure represents a dynamically sizable
//  stack of CALC_VALUE items.
//

typedef struct tagCALC_VALUESTACK
{
  CALC_HANDLE hElem;
  LPCALC_VALUE lpElem;
  int nElemCnt;
  int nAllocCnt;
} CALC_VALUESTACK, FAR* LPCALC_VALUESTACK;

//--------------------------------------------------------------------
//
//  The ValStackInit() function initializes the stack.
//

BOOL ValStackInit(LPCALC_VALUESTACK lpStack)
{
  lpStack->hElem = 0;
  lpStack->lpElem = 0;
  lpStack->nElemCnt = 0;
  lpStack->nAllocCnt = 0;
  return TRUE;
}

//--------------------------------------------------------------------
//
//  The ValStackRemove() function removes the given number of items
//  from the top of the stack.
//

BOOL ValStackRemove(LPCALC_VALUESTACK lpStack, int nArgs)
{
  for( ; lpStack->nElemCnt > 0 && nArgs > 0; lpStack->nElemCnt--, nArgs-- )
    ValFree(lpStack->lpElem + lpStack->nElemCnt - 1);
  return nArgs <= 0;
}

//--------------------------------------------------------------------
//
//  The ValStackFree() function destroys the stack.  All memory
//  associated with the stack is freed.
//

BOOL ValStackFree(LPCALC_VALUESTACK lpStack)
{
  ValStackRemove(lpStack, lpStack->nElemCnt);
  if( lpStack->lpElem )
    CalcMemUnlock(lpStack->hElem );
  if( lpStack->hElem )
    CalcMemFree(lpStack->hElem);
  lpStack->hElem = 0;
  lpStack->lpElem = 0;
  lpStack->nElemCnt = 0;
  lpStack->nAllocCnt = 0;
  return TRUE;
}

//--------------------------------------------------------------------
//
//  The ValStackClear() function removes all items from the stack.
//

BOOL ValStackClear(LPCALC_VALUESTACK lpStack)
{
  ValStackRemove(lpStack, lpStack->nElemCnt);
  return TRUE;
}

//--------------------------------------------------------------------
//
//  The ValStackPush() function pushes an item onto the top of the
//  stack.
//

BOOL ValStackPush(LPCALC_VALUESTACK lpStack, LPCALC_VALUE lpItem)
{
  BOOL bRet = FALSE;

  if( lpStack->nElemCnt >= lpStack->nAllocCnt )
  {
    lpStack->nAllocCnt += VALUESTACK_ALLOCINC;
    if( lpStack->lpElem )
      CalcMemUnlock(lpStack->hElem);
    if( lpStack->hElem )
      lpStack->hElem = CalcMemReAlloc(lpStack->hElem, lpStack->nAllocCnt*sizeof(CALC_VALUE));
    else
      lpStack->hElem = CalcMemAlloc(lpStack->nAllocCnt*sizeof(CALC_VALUE));
    if( !lpStack->hElem || !(lpStack->lpElem = CalcMemLock(lpStack->hElem)) )
    {
      lpStack->hElem = 0;
      lpStack->lpElem = 0;
      lpStack->nElemCnt = 0;
      lpStack->nAllocCnt = 0;
    }
  }
  if( lpStack->nElemCnt < lpStack->nAllocCnt )
  {
    lpStack->lpElem[lpStack->nElemCnt] = *lpItem;
    lpStack->nElemCnt++;
    bRet = TRUE;
  }
  return bRet;
}

//--------------------------------------------------------------------
//
//  The ValStackPop() function pops an item from the top of the stack.
//

BOOL ValStackPop(LPCALC_VALUESTACK lpStack, LPCALC_VALUE lpItem)
{
  BOOL bRet = FALSE;

  if( lpStack->nElemCnt > 0 )
  {
    lpStack->nElemCnt--;
    *lpItem = lpStack->lpElem[lpStack->nElemCnt];
    bRet = TRUE;
  }
  return bRet;
}

//--------------------------------------------------------------------
//
//  The most significate bit is used to indicate a relative address.
//  Thus, the next most significate bit indicates if the value is
//  positive or negative.
//

#define CALC_RELBIT 0x80000000L
#define CALC_NEGBIT 0x40000000L

//--------------------------------------------------------------------
//
//  The CoordIsRel() function determines if the given coordinate is
//  relative.
//

BOOL CoordIsRel(long lCoord)
{
  return (lCoord & CALC_RELBIT) != 0;
}

//--------------------------------------------------------------------
//
//  The CoordIsAbs() function determines if the given coordinate is
//  absolute.
//

BOOL CoordIsAbs(long lCoord)
{
  return (lCoord & CALC_RELBIT) == 0;
}

//--------------------------------------------------------------------
//
//  The CoordPlus() function adds an offset to a coordinate.  The
//  coordinate can be relative or absolute.
//

long CoordPlus(long lCoord, long lOffset)
{
  if( lCoord & CALC_RELBIT )
  {
    if( !(lCoord & CALC_NEGBIT) )
      lCoord &= ~CALC_RELBIT;
    lCoord += lOffset;
    lCoord |= CALC_RELBIT;
  }
  else
    lCoord += lOffset;
  return lCoord;
}

//--------------------------------------------------------------------
//
//  The CoordMakeAbs() function converts a relative coordinate to
//  an absolute coordinate.  Absolute coordinates are left untouched.
//

long CoordMakeAbs(long lCoord, long lBase)
{
  if( lCoord & CALC_RELBIT )
  {
    if( !(lCoord & CALC_NEGBIT) )
      lCoord &= ~CALC_RELBIT;
    lCoord += lBase;
  }
  return lCoord;
}

//--------------------------------------------------------------------
//
//  The CoordMakeRel() function converts an absolute coordinate to
//  a relative coordinate.  The relative offset is computed from
//  the base coordinate.
//

long CoordMakeRel(long lCoord, long lBase)
{
  return (lCoord - lBase) | CALC_RELBIT;
}

//--------------------------------------------------------------------
//
//  The CoordMax() function returns the maxium of two coordinates
//  when both coordinates are absolute or both coordinates are
//  relative.  The function is undefined when a absolute coordinate
//  and a relative coordinate are mixed.
//

long CoordMax(long lCoord1, long lCoord2)
{
  long lRet;

  if( CoordIsRel(lCoord1) || CoordIsRel(lCoord2) )
  {
    lCoord1 = CoordMakeAbs(lCoord1, 0);
    lCoord2 = CoordMakeAbs(lCoord2, 0);
    lRet = CoordMakeRel(max(lCoord1, lCoord2), 0);
  }
  else
    lRet = max(lCoord1, lCoord2);
  return lRet;
}

//--------------------------------------------------------------------
//
//  The CoordMin() function returns the minium of two coordinates
//  when both coordinates are absolute or both coordinates are
//  relative.  The function is undefined when a absolute coordinate
//  and a relative coordinate are mixed.
//

long CoordMin(long lCoord1, long lCoord2)
{
  long lRet;

  if( CoordIsRel(lCoord1) || CoordIsRel(lCoord2) )
  {
    lCoord1 = CoordMakeAbs(lCoord1, 0);
    lCoord2 = CoordMakeAbs(lCoord2, 0);
    lRet = CoordMakeRel(min(lCoord1, lCoord2), 0);
  }
  else
    lRet = min(lCoord1, lCoord2);
  return lRet;
}

//--------------------------------------------------------------------
//
//  The AlphaToCoord() function converts the text representation
//  of a column letter (A, B, C, etc) into a long data type.
//

static LPTSTR AlphaToCoord(LPTSTR lpszText, LPLONG lplCoord)
{
  BOOL bInRange = TRUE;
  long lCoord = 0;
  for( ; bInRange && _istalpha((_TUCHAR)*lpszText); lpszText++)
  {
    bInRange &= lCoord <= CALC_MAX_COORD / 26;
    lCoord = 26 * lCoord + _totupper((_TUCHAR)*lpszText) - 'A' + 1;
    bInRange &= lCoord <= CALC_MAX_COORD;
  }
  if( lplCoord )
    *lplCoord = bInRange ? lCoord : 0;
  return lpszText;
}

//--------------------------------------------------------------------
//
//  The DigitToCoord() function converts the text representation
//  of a column/row number into a long data type.
//

static LPTSTR DigitToCoord(LPTSTR lpszText, LPLONG lplCoord)
{
  BOOL bInRange = TRUE;
  long lCoord = 0;
  for( ; bInRange && _istdigit((_TUCHAR)*lpszText); lpszText++ )
  {
    bInRange &= lCoord <= CALC_MAX_COORD / 10;
    lCoord = 10 * lCoord + *lpszText - '0';
    bInRange &= lCoord <= CALC_MAX_COORD;
  }
  if( lplCoord )
    *lplCoord = bInRange ? lCoord : 0;
  return lpszText;
}

//--------------------------------------------------------------------
//
//  The APoundToCell() function converts A# formated text into a
//  relative cell reference.
//

static BOOL APoundToCell(LPTSTR lpszText, LPLONG lplCol, LPLONG lplRow)
{
  long lCol;
  long lRow;
  BOOL bRet = TRUE;
  
  lCol = 0;
  lRow = 0;
  if( *lpszText == '#' )
  {
    lCol = CoordMakeRel(lCol, lCol);
    lpszText++;
  }
  else
  {
    lpszText = AlphaToCoord(lpszText, &lCol);
    bRet &= lCol > 0;
  }
  if( *lpszText == '#' )
  {
    lRow = CoordMakeRel(lRow, lRow);
    lpszText++;
  }
  else
  {
    lpszText = DigitToCoord(lpszText, &lRow);
    bRet &= lRow > 0;
  }
  *lplCol = lCol;
  *lplRow = lRow;
  return bRet && 0 == *lpszText;
}

//--------------------------------------------------------------------
//
//  The A1ToCell() function converts A1 formated text into a relative
//  cell reference.
//

static BOOL A1ToCell(long lBaseCol, long lBaseRow, LPTSTR lpszText,
                     LPLONG lplCol, LPLONG lplRow)
{
  long lCol;
  long lRow;
  BOOL bAbsCol; // is absolute column reference
  BOOL bAbsRow; // is absolute row reference
  BOOL bRet;
  
  lCol = 0;
  lRow = 0;
  if( bAbsCol = *lpszText == '$' )
    lpszText++;
  lpszText = AlphaToCoord(lpszText, &lCol);
  if( bAbsRow = *lpszText == '$' )
    lpszText++;
  lpszText = DigitToCoord(lpszText, &lRow);
  if( bRet = lCol > 0 && lRow > 0 && 0 == *lpszText )
  {
    if( !bAbsCol )
    {
      lCol = CoordMakeRel(lCol, lBaseCol);
    }
    if( !bAbsRow )
   {
      lRow = CoordMakeRel(lRow, lBaseRow);
    }
  }
  *lplCol = lCol;
  *lplRow = lRow;
  return bRet;
}

//--------------------------------------------------------------------
//
//  The R1C1ToCell() function converts R1C1 formated text into a
//  relative cell reference.
//

static BOOL R1C1ToCell(LPTSTR lpszText, LPLONG lplCol, LPLONG lplRow)
{
  long lCol;
  long lRow;
  BOOL bNegative;
  BOOL bRelative;
  BOOL bRet = TRUE;
  
  lCol = 0;
  lRow = 0;
  if( bRet &= 'r' == *lpszText || 'R' == *lpszText )
    *lpszText++;
  if( bRelative = '[' == *lpszText )
  {
    lpszText++;
    if( bNegative = '-' == *lpszText )
      lpszText++;
    lpszText = DigitToCoord(lpszText, &lRow);
    if( bRet &= ']' == *lpszText )
      *lpszText++;
    if( bNegative )
      lRow = - lRow;
  }
  else
  {
    lpszText = DigitToCoord(lpszText, &lRow);
    bRelative = lRow == 0;
  }
  if( bRelative )
    lRow = CoordMakeRel(lRow, 0);
  if( bRet &= 'c' == *lpszText || 'C' == *lpszText )
    *lpszText++;
  if( bRelative = '[' == *lpszText )
  {
    lpszText++;
    if( bNegative = '-' == *lpszText )
      lpszText++;
    lpszText = DigitToCoord(lpszText, &lCol);
    if( bRet &= ']' == *lpszText )
      *lpszText++;
    if( bNegative )
      lCol = - lCol;
  }
  else
  {
    lpszText = DigitToCoord(lpszText, &lCol);
    bRelative = lCol == 0;
  }
  if( bRelative )
    lCol = CoordMakeRel(lCol, 0);
  *lplCol = lCol;
  *lplRow = lRow;
  return bRet && 0 == *lpszText;
}

//--------------------------------------------------------------------

static int CollapseSingleQuote(LPTSTR lpszDest, int nDestLen, LPTSTR lpszSrc, int nSrcLen)
{
  int i;
  int j;
  for (i = 0, j = 0; i < nSrcLen && j < nDestLen; i++)
  {
    lpszDest[j++] = lpszSrc[i];
    if (lpszSrc[i] == '\'')
      i++;
  }
  return j;
}

//--------------------------------------------------------------------

static BOOL TextToCell(LPCALC_BOOK lpBook, long lBaseCol, long lBaseRow,
                       short nRefStyle, LPTSTR lpszText,
                       LPCALC_SHEET FAR* lplpSheet, LPLONG lplCol, LPLONG lplRow)
{
  LPTSTR lpszSheet = NULL;
  int nSheetLen = 0;
  BOOL bRet = TRUE;

  if ('\'' == *lpszText)
  {
    lpszText++;
    lpszSheet = lpszText;
    while ((*lpszText && *lpszText != '\'') || (*lpszText == '\'' && *(lpszText+1) == '\''))
    {
      if (*lpszText == '\'')
        lpszText++;
      lpszText++;
    }
    nSheetLen = (int)(lpszText - lpszSheet);
    if (bRet &= '\'' == *lpszText)
      lpszText++;
    if (bRet &= '!' == *lpszText)
      lpszText++;
    bRet &= nSheetLen > 0;
  }
  else if (_tcschr(lpszText, '!') != NULL)
  {
    lpszSheet = lpszText;
    while (*lpszText && *lpszText != '!')
      lpszText++;
    nSheetLen = (int)(lpszText - lpszSheet);
    if (bRet &= '!' == *lpszText)
      lpszText++;
    bRet &= nSheetLen > 0;
  }
  if (lpszSheet != NULL && nSheetLen > 0)
  {
    TCHAR szCopy[CALC_MAX_SHEET_LEN];
    int nCopyLen;
    nCopyLen = CollapseSingleQuote(szCopy, sizeof(szCopy)/sizeof(TCHAR), lpszSheet, nSheetLen);
    *lplpSheet = lpBook->lpfnGetSheetFromName(lpBook->hBook, szCopy, nCopyLen);
    bRet &= *lplpSheet != NULL;
  }
  else
    *lplpSheet = NULL;
  if (CALC_REFSTYLE_R1C1 == nRefStyle)
    bRet &= R1C1ToCell(lpszText, lplCol, lplRow);
  else if (CALC_REFSTYLE_A1 == nRefStyle)
    bRet &= A1ToCell(lBaseCol, lBaseRow, lpszText, lplCol, lplRow);
  else
    bRet &= APoundToCell(lpszText, lplCol, lplRow);
  return bRet;
}

//--------------------------------------------------------------------
//
//  The CellToAPound() function converts a relative cell reference
//  to its A# formated text represention.
//
//  Some column and row references entered in A1 or R1C1 notation
//  can not be represented in A# notation.  These cases will
//  be displayed as "?" in A# notation.
//

static int CellToAPound(long lCol, long lRow,
                        LPTSTR lpszText, int nLen)
{
  TCHAR szTemp[CALC_MAX_COORD_LEN];
  BOOL bRelCol = CoordIsRel(lCol);
  BOOL bRelRow = CoordIsRel(lRow);
  int iReturn = 0;
  int i = 0;
  
  if( bRelRow )
  {
    if( CoordMakeAbs(lRow,0) == 0 )
      szTemp[i++] = '#';
    else
      szTemp[i++] = '?';
  }
  else
  {
    if( lRow > 0 )
    {
      for( i = 0; lRow > 0; lRow /= 10 )
        szTemp[i++] = '0' + (TCHAR)(lRow % 10);
    }
    else
      szTemp[i++] = '?';
  }
  if( bRelCol )
  {
    if( CoordMakeAbs(lCol,0) == 0 )
      szTemp[i++] = '#';
    else
      szTemp[i++] = '?';
  }
  else
  {
    if( lCol > 0 )
    {
      for( ; lCol > 0; lCol = (lCol - 1) / 26 )
        szTemp[i++] = 'A' + (TCHAR)((lCol - 1) % 26);
    }
    else
      szTemp[i++] = '?';
  }
  if( i < nLen )
  {
    iReturn = i;
    for( ; i > 0; lpszText++ )
      *lpszText = szTemp[--i];
    *lpszText = '\0';
  }
  return iReturn;
}

//--------------------------------------------------------------------
//
//  The CellToA1() function converts a relative cell reference to its
//  A1 formated text represention.
//

static int CellToA1(long lCol, long lRow,
                    long lBaseCol, long lBaseRow,
                    LPTSTR lpszText, int nLen)
{
  TCHAR szTemp[CALC_MAX_COORD_LEN];
  BOOL bRelCol = CoordIsRel(lCol);
  BOOL bRelRow = CoordIsRel(lRow);
  int iReturn = 0;
  int i = 0;
  
  if( bRelCol )
    lCol = CoordMakeAbs(lCol, lBaseCol);
  if( bRelRow )
    lRow = CoordMakeAbs(lRow, lBaseRow);
  if( lRow > 0 )
  {
    for( i = 0; lRow > 0; lRow /= 10 )
      szTemp[i++] = '0' + (TCHAR)(lRow % 10);
  }
  else
    szTemp[i++] = '?';
  if( !bRelRow )
    szTemp[i++] = '$';
  if( lCol > 0 )
  {
    for( ; lCol > 0; lCol = (lCol - 1) / 26 )
      szTemp[i++] = 'A' + (TCHAR)((lCol - 1) % 26);
  }
  else
    szTemp[i++] = '?';
  if( !bRelCol )
    szTemp[i++] = '$';
  if( i < nLen )
  {
    iReturn = i;
    for( ; i > 0; lpszText++ )
      *lpszText = szTemp[--i];
    *lpszText = '\0';
  }
  return iReturn;
}

//--------------------------------------------------------------------
//
//  The CellToR1C1() function converts a relative cell reference to its
//  R1C1 formated text representation.
//

static int CellToR1C1(long lCol, long lRow,
                      LPTSTR lpszText, int nLen)
{
  BOOL bRelCol = CoordIsRel(lCol);
  BOOL bRelRow = CoordIsRel(lRow);
  BOOL bNonZeroCol;
  BOOL bNonZeroRow;
  BOOL bNegCol;
  BOOL bNegRow;
  TCHAR szTemp[CALC_MAX_COORD_LEN];
  int iReturn = 0;
  int i = 0;

  if( bRelCol )
    lCol = CoordMakeAbs(lCol, 0);
  if( bRelRow )
    lRow = CoordMakeAbs(lRow, 0);
  if( bNegCol = lCol < 0 )
    lCol = -lCol;
  if( bNegRow = lRow < 0 )
    lRow = -lRow;
  bNonZeroCol = lCol != 0;
  bNonZeroRow = lRow != 0;
  if( bRelCol && bNonZeroCol )
    szTemp[i++] = ']';
  for( ; lCol > 0; lCol /= 10 )
    szTemp[i++] = '0' + (TCHAR)(lCol % 10);
  if( bNegCol )
    szTemp[i++] = '-';
  if( bRelCol && bNonZeroCol )
    szTemp[i++] = '[';
  szTemp[i++] = 'C';
  if( bRelRow && bNonZeroRow )
    szTemp[i++] = ']';
  for( ; lRow > 0; lRow /= 10 )
    szTemp[i++] = '0' + (TCHAR)(lRow % 10);
  if( bNegRow )
    szTemp[i++] = '-';
  if( bRelRow && bNonZeroRow )
    szTemp[i++] = '[';
  szTemp[i++] = 'R';
  if( i < nLen )
  {
    iReturn = i;
    for( ; i > 0; lpszText++ )
      *lpszText = szTemp[--i];
    *lpszText = '\0';
  }
  return iReturn;
}

//--------------------------------------------------------------------
//
//  The RangeToAPound() function converts a relative range reference
//  to its A# formated text represention.
//

static int RangeToAPound(long lCol1, long lRow1,
                         long lCol2, long lRow2,
                         LPTSTR lpszText, int nLen)
{
  int iReturn;
  
  iReturn = CellToAPound(lCol1, lRow1, lpszText, nLen);
  if( iReturn < nLen )
    lpszText[iReturn++] = ':';
  iReturn += CellToAPound(lCol2, lRow2, lpszText+iReturn, nLen-iReturn);
  if( iReturn < nLen )
    lpszText[iReturn] = '\0';
  else
    iReturn = 0;
  return iReturn;
}

//--------------------------------------------------------------------
//
//  The RangeToA1() function converts a relative range reference to
//  its A1 formated text represention.
//

//static int RangeToA1(long lCol1, long lRow1,
//                     long lCol2, long lRow2,
//                     long lBaseCol, long lBaseRow,
//                     LPTSTR lpszText, int nLen)
//{
//  int iReturn;
//  
//  iReturn = CellToA1(lCol1, lRow1, lBaseCol, lBaseRow, lpszText, nLen);
//  if( iReturn < nLen )
//    lpszText[iReturn++] = ':';
//  iReturn += CellToA1(lCol2, lRow2, lBaseCol, lBaseRow,
//                      lpszText+iReturn, nLen-iReturn);
//  if( iReturn < nLen )
//    lpszText[iReturn] = '\0';
//  else
//    iReturn = 0;
//  return iReturn;
//}

//--------------------------------------------------------------------
//
//  The RangeToR1C1() function converts a relative range reference to
//  its R1C1 formated text representation.
//

//static int RangeToR1C1(long lCol1, long lRow1,
//                       long lCol2, long lRow2,
//                       LPTSTR lpszText, int nLen)
//{
//  int iReturn;
//  
//  iReturn = CellToR1C1(lCol1, lRow1, lpszText, nLen);
//  if( iReturn < nLen )
//    lpszText[iReturn++] = ':';
//  iReturn += CellToR1C1(lCol2, lRow2, lpszText+iReturn, nLen-iReturn);
//  if( iReturn < nLen )
//    lpszText[iReturn] = '\0';
//  else
//    iReturn = 0;
//  return iReturn;
//}

//--------------------------------------------------------------------
//
//  The CopyTokenText() function allocates a copy of the token's text
//  and returns a handle to the copied text.
//

static CALC_HANDLE CopyTokenText(LPCALC_TOKEN lpToken)
{
  CALC_HANDLE hCopy;
  LPTSTR lpszCopy;
  
  hCopy = CalcMemAlloc((lpToken->nLen + 1) * sizeof(TCHAR));
  if( hCopy && (lpszCopy = (LPTSTR)CalcMemLock(hCopy)) )
  {
    lstrcpyn(lpszCopy, lpToken->lpszText, lpToken->nLen + 1);
    lpszCopy[lpToken->nLen] = '\0';
    CalcMemUnlock(hCopy);
  }
  return hCopy;
}

//--------------------------------------------------------------------
//
//  The CopyHandleText() function allocates a copy of the handle's
//  text and returns a handle to the copied text.
//

static CALC_HANDLE CopyHandleText(CALC_HANDLE hText)
{
  CALC_HANDLE hCopy;
  LPTSTR lpszCopy;
  LPTSTR lpszText;
  
  if( hText && (lpszText = (LPTSTR)CalcMemLock(hText)) )
  {
    hCopy = CalcMemAlloc((lstrlen(lpszText)+1) * sizeof(TCHAR));
    if( hCopy && (lpszCopy = (LPTSTR)CalcMemLock(hCopy)) )
    {
      lstrcpy(lpszCopy, lpszText);
      CalcMemUnlock(hCopy);
    }
    CalcMemUnlock(hText);
  }
  return hCopy;
}

//--------------------------------------------------------------------
//
//  The StrCopyToken() function copies the token's text into lpszText.
//  The function makes sure that lpszText ends with a '/0', unlike
//  _fstrncpy() function.
//

static int StrCopyToken(LPCALC_TOKEN lpToken,
                        LPTSTR lpszText, int nTextLen)
{
  int nLen = min(lpToken->nLen, nTextLen - 1);
  
  lstrcpyn(lpszText, lpToken->lpszText, nLen + 1);
  lpszText[nLen] = '\0';
  return nLen;
}

//--------------------------------------------------------------------
//
//  The TokenToExpr() function converts a single token to an
//  expression component.  If successful, the function returns TRUE.
//  Otherwise, the function returns FALSE.
//

static BOOL TokenToExpr(LPCALC_BOOK lpBook, long lCol, long lRow,
                        LPCALC_TOKEN lpToken, LPCALC_EXPRINFO lpExpr)
{
  BOOL bError = FALSE;
  CALC_HANDLE hName;
  CALC_HANDLE hFunc;
  int nId;
  LPCALC_SHEET lpCellSheet;
  long lCellCol;
  long lCellRow;
  static TCHAR szText[CALC_MAX_NAME_LEN];  // could not be on stack in Win16 medium memory model
  static TCHAR* lpszEnd;

  switch( lpToken->nType )
  {
    case CALC_TOKEN_OPERATOR:
      lpExpr->nType = CALC_EXPR_OPERATOR;
      lpExpr->u.Oper.nId = lpToken->u.Oper.nId;
      break;
    case CALC_TOKEN_SEPARATOR:
      lpExpr->nType = CALC_EXPR_SEPARATOR;
      lpExpr->u.Sep.nId = lpToken->u.Sep.nId;
      break;
    case CALC_TOKEN_LONG:
      errno = 0;
      StrCopyToken(lpToken, szText, sizeof(szText)/sizeof(TCHAR));
      lpExpr->nType = CALC_EXPR_LONG;
      lpExpr->u.Long.lVal = _tcstol(szText, &lpszEnd, 10);
      if( ERANGE == errno )
      {
        errno = 0;
        lpExpr->nType = CALC_EXPR_DOUBLE;
        // RFW - 2/4/02 - 9423     lpExpr->u.Double.dfVal = _tcstod(szText, &lpszEnd);
        lpExpr->u.Double.dfVal = CalcStringToDouble(szText);
      }
      if( errno )
      {
        errno = 0;
        bError = TRUE;
      }
      break;
    case CALC_TOKEN_DOUBLE:
      errno = 0;
      StrCopyToken(lpToken, szText, sizeof(szText)/sizeof(TCHAR));
      lpExpr->nType = CALC_EXPR_DOUBLE;
      // RFW - 2/4/02 - 9423     lpExpr->u.Double.dfVal = _tcstod(szText, &lpszEnd);
      lpExpr->u.Double.dfVal = CalcStringToDouble(szText);
      if( errno )
      {
        errno = 0;
        bError = TRUE;
      }
      break;
    case CALC_TOKEN_NAME:
      StrCopyToken(lpToken, szText, sizeof(szText)/sizeof(TCHAR));
      if( hName = NameLookup(&lpBook->Names, szText) )
      {
        lpExpr->nType = CALC_EXPR_NAME;
        lpExpr->u.CustName.hName = NameCreateRef(hName);
      }
      else if( TextToCell(lpBook, lCol, lRow, lpBook->nRefStyle, szText, &lpCellSheet, &lCellCol, &lCellRow) )
      {
        if (lpCellSheet != NULL)
        {
          lpExpr->nType = CALC_EXPR_EXTERNALCELL;
          lpExpr->u.ExternalCell.lpSheet = lpCellSheet;
          lpExpr->u.ExternalCell.lCol = lCellCol;
          lpExpr->u.ExternalCell.lRow = lCellRow;
        }
        else
        {
          lpExpr->nType = CALC_EXPR_CELL;
          lpExpr->u.Cell.lCol = lCellCol;
          lpExpr->u.Cell.lRow = lCellRow;
        }
      }
      else
        bError = TRUE;
      break;
    case CALC_TOKEN_STRING:
      lpExpr->nType = CALC_EXPR_STRING;
      lpExpr->u.String.hText = CopyTokenText(lpToken);
      break;
    case CALC_TOKEN_FUNCTION:
      StrCopyToken(lpToken, szText, sizeof(szText)/sizeof(TCHAR));
      if( hFunc = CustFuncLookup(&lpBook->CustFuncs, szText) )
      {
        if( CustFuncGetMinArgs(hFunc) <= lpToken->u.Func.nArgs
            && lpToken->u.Func.nArgs <= CustFuncGetMaxArgs(hFunc) )
        {
          lpExpr->nType = CALC_EXPR_CUSTFUNC;
          lpExpr->u.CustFunc.hFunc = CustFuncCreateRef(hFunc);
          lpExpr->u.CustFunc.nArgs = lpToken->u.Func.nArgs;
        }
        else
          bError = TRUE;
      }
      else if( nId = FuncLookup(szText) )
      {
        if( FuncGetMinArgs(nId) <= lpToken->u.Func.nArgs
            && lpToken->u.Func.nArgs <= FuncGetMaxArgs(nId) )
        {
          lpExpr->nType = CALC_EXPR_FUNCTION;
          lpExpr->u.Func.nId = nId;
          lpExpr->u.Func.nArgs = lpToken->u.Func.nArgs;
        }
        else
          bError = TRUE;
      }
      else
        bError = TRUE;
      break;
    case CALC_TOKEN_ERROR:
      if (nId = ErrLookup(lpToken->lpszText, lpToken->nLen))
      {
        lpExpr->nType = CALC_EXPR_ERROR;
        lpExpr->u.Error.nId = nId;
      }
      else
        bError = TRUE;
      break;
  }
  if( bError )
    lpExpr->nType = CALC_EXPR_NULL;
  return !bError;
}

//--------------------------------------------------------------------
//
//  The OperToRange() function converts two cell references and an
//  operator into a single range reference.
//

#ifdef SS_V70
static BOOL OperToRange(LPCALC_EXPRINFO lpExpr)
{
  CALC_EXPRINFO temp;
  BOOL bReturn = FALSE;
  
  if (lpExpr->nType == CALC_EXPR_OPERATOR && lpExpr->u.Oper.nId == CALC_OPER_RANGE)
  {
    if ((lpExpr-1)->nType == CALC_EXPR_CELL && (lpExpr-2)->nType == CALC_EXPR_CELL)
    {
      temp.nType = CALC_EXPR_RANGE;
      temp.u.Range.lCol1 = (lpExpr-2)->u.Cell.lCol;
      temp.u.Range.lRow1 = (lpExpr-2)->u.Cell.lRow;
      temp.u.Range.lCol2 = (lpExpr-1)->u.Cell.lCol;
      temp.u.Range.lRow2 = (lpExpr-1)->u.Cell.lRow;
      *(lpExpr-2) = temp;
      bReturn = TRUE;
    }
    else if ((lpExpr-1)->nType == CALC_EXPR_CELL && (lpExpr-2)->nType == CALC_EXPR_EXTERNALCELL)
    {
      temp.nType = CALC_EXPR_EXTERNALRANGE;
      temp.u.ExternalRange.lpSheet = (lpExpr-2)->u.ExternalCell.lpSheet;
      temp.u.ExternalRange.lCol1 = (lpExpr-2)->u.ExternalCell.lCol;
      temp.u.ExternalRange.lRow1 = (lpExpr-2)->u.ExternalCell.lRow;
      temp.u.ExternalRange.lCol2 = (lpExpr-1)->u.Cell.lCol;
      temp.u.ExternalRange.lRow2 = (lpExpr-1)->u.Cell.lRow;
      *(lpExpr-2) = temp;
      bReturn = TRUE;
    }
  }
  return bReturn;
}
#endif

//--------------------------------------------------------------------

#if defined(SS_DDE)
static BOOL FuncToDde(LPCALC_INFO lpCalc, LPCALC_EXPRINFO lpExpr,
                      LPBOOL lpbNoError, short FAR* lpnArgs)
{
  CALC_EXPRINFO temp;
  CALC_HANDLE hDde;
  LPCALC_EXPRINFO lpServer;
  LPCALC_EXPRINFO lpTopic;
  LPCALC_EXPRINFO lpItem;
  LPCALC_EXPRINFO lpCol;
  LPCALC_EXPRINFO lpRow;
  LPTSTR lpszServer;
  LPTSTR lpszTopic;
  LPTSTR lpszItem;
  short nArgs;
  BOOL bReturn = FALSE;

  if( CALC_EXPR_FUNCTION == lpExpr->nType &&
      CALC_FUNC_DDELINK == lpExpr->u.Func.nId &&
      3 <= lpExpr->u.Func.nArgs && lpExpr->u.Func.nArgs <= 5)
  {
    nArgs = lpExpr->u.Func.nArgs;
    lpServer = lpExpr - nArgs;
    lpTopic = lpServer + 1;
    lpItem = lpTopic + 1;
    lpCol = nArgs >= 4 ? lpItem + 1 : NULL;
    lpRow = nArgs >= 5 ? lpCol + 1 : NULL;
    if( CALC_EXPR_STRING == lpServer->nType &&
        CALC_EXPR_STRING == lpTopic->nType &&
        CALC_EXPR_STRING == lpItem->nType &&
        (NULL == lpCol || CALC_EXPR_LONG == lpCol->nType) &&
        (NULL == lpRow || CALC_EXPR_LONG == lpRow->nType) &&
        (lpszServer = (LPTSTR)CalcMemLock(lpServer->u.String.hText)) &&
        (lpszTopic = (LPTSTR)CalcMemLock(lpTopic->u.String.hText)) &&
        (lpszItem = (LPTSTR)CalcMemLock(lpItem->u.String.hText)) )
    {
      if( hDde = DdeLookup(&lpCalc->DdeLinks, lpszServer, lpszTopic, lpszItem) )
        hDde = DdeCreateRef(hDde);
      else if( hDde = DdeCreate(lpszServer, lpszTopic, lpszItem,
                                lpCalc->lpfnDdeUpdate, lpCalc->hSS) )
      {
        DdeAdd(&lpCalc->DdeLinks, hDde);
        DdeSetMode(hDde, CALC_LINKMODE_HOT);
      }
      CalcMemUnlock(lpServer->u.String.hText);
      CalcMemUnlock(lpTopic->u.String.hText);
      CalcMemUnlock(lpItem->u.String.hText);
      if( hDde )
      {
        CalcMemFree(lpServer->u.String.hText);
        CalcMemFree(lpTopic->u.String.hText);
        CalcMemFree(lpItem->u.String.hText);
        temp.nType = CALC_EXPR_DDE;
        temp.u.Dde.hDde = hDde;
        temp.u.Dde.lItemCol = lpCol ? lpCol->u.Long.lVal : -1;
        temp.u.Dde.lItemRow = lpRow ? lpRow->u.Long.lVal : -1;
        *(lpExpr-nArgs) = temp;
        *lpnArgs = nArgs;
        bReturn = TRUE;
      }
      else
        *lpbNoError = FALSE;
    }
    else
      *lpbNoError = FALSE;  
  }
  return bReturn;
}
#endif

//--------------------------------------------------------------------
//
//  The ExprCreate() function creates a parsed version of the
//  expression.  The function then returns a handle to the parsed
//  version.
//

CALC_HANDLE ExprCreate(LPCALC_BOOK lpBook, long lCol, long lRow, LPCTSTR lpszText)
{
  CALC_TOKENARRAY tokenArray;
  CALC_HANDLE hExpr = 0;
  LPCALC_EXPRINFO lpExpr;
  BOOL bNoError = TRUE;
  int i;
  #if defined(SS_DDE)
  short nArgsToRemove;
  #endif

  if( TokenArrayInit(&tokenArray) )
  {
    if( lCol == CALC_ALLCOLS )
      lCol = 1;
    if( lRow == CALC_ALLROWS )
      lRow = 1;  
    if( TokenParse(lpszText, &tokenArray) && tokenArray.nSize > 0 )
    {
      if( hExpr = Expr_MemAlloc(2 + tokenArray.nSize) )
      {
        if( lpExpr = Expr_MemLock(hExpr) )
        {
          lpExpr->nType = CALC_EXPR_BEGIN;
          lpExpr->u.Header.lRefCnt = 1;
          lpExpr++;
          for( i = 0; bNoError && i < tokenArray.nSize; i++, lpExpr++ )
          {
            bNoError = TokenToExpr(lpBook, lCol, lRow, tokenArray.lpItem+i, lpExpr);
            #ifdef SS_V70
            if( OperToRange(lpExpr) )
              lpExpr -= 2;
            #endif
            //#if defined(SS_DDE)
            //else if( FuncToDde(lpSheet, lpExpr, &bNoError, &nArgsToRemove) )
            #if defined(SS_DDE)
            if( FuncToDde(lpSheet, lpExpr, &bNoError, &nArgsToRemove) )
              lpExpr -= nArgsToRemove;
            #endif
          }
          lpExpr->nType = CALC_EXPR_END;
          Expr_MemUnlock(hExpr);
          if( !bNoError )
          {
            ExprDestroy(hExpr);
            hExpr = 0;
          }
        }
        else
          bNoError = FALSE;
      }
    }
    TokenArrayFree(&tokenArray);
  }
  return hExpr;
}

//--------------------------------------------------------------------
//
//  The ExprCreateFromDde() function creates an expression which
//  represents a DDE link.
//

#if defined(SS_DDE)
CALC_HANDLE ExprCreateFromDde(CALC_HANDLE hDde, long lItemCol, long lItemRow)
{
  CALC_HANDLE hExpr;
  LPCALC_EXPRINFO lpExpr;

  if( hDde && lItemCol >= -1 && lItemRow >= -1 )
  {
    if( hExpr = Expr_MemAlloc(3) )
    {
      if( lpExpr = Expr_MemLock(hExpr) )
     {
        lpExpr->nType = CALC_EXPR_BEGIN;
        lpExpr->u.Header.lRefCnt = 1;
        lpExpr++;
        lpExpr->nType = CALC_EXPR_DDE;
        lpExpr->u.Dde.hDde = DdeCreateRef(hDde);
        lpExpr->u.Dde.lItemCol = lItemCol;
        lpExpr->u.Dde.lItemRow = lItemRow;
        lpExpr++;
        lpExpr->nType = CALC_EXPR_END;
        Expr_MemUnlock(hExpr);
      }
    }
  }
  return hExpr;
}
#endif

//--------------------------------------------------------------------
//
//  The ExprCreateRef() function increments the reference counter of
//  the given expression.  If successful, the function returns the
//  handle to the orginal expression.  Otherwise, the function returns
//  NULL.
//
//  Note: It is assumed that the first expression componment
//        has nType = CALC_EXPR_BEGIN.
//

CALC_HANDLE ExprCreateRef(CALC_HANDLE hExpr)
{
  LPCALC_EXPRINFO lpExpr;
  CALC_HANDLE hReturn = 0;
  
  if( hExpr && (lpExpr = Expr_MemLock(hExpr)) )
  {
    lpExpr->u.Header.lRefCnt++;
    hReturn = hExpr;
    Expr_MemUnlock(hExpr);
  }
  return hReturn;
}

//--------------------------------------------------------------------
//
//  The ExprCreateCopy() function creates a copy of the given
//  expression.
//

CALC_HANDLE ExprCreateCopy(CALC_HANDLE hExpr)
{
  LPCALC_EXPRINFO lpExpr;
  LPCALC_EXPRINFO lpCopy;
  CALC_HANDLE hCopy;
  CALC_HANDLE hReturn = 0;
  int iSize;
  
  if( hExpr && (lpExpr = Expr_MemLock(hExpr)) )
  {
    for( iSize = 0; lpExpr[iSize].nType != CALC_EXPR_END; iSize++ )
      ;
    iSize = iSize + 1;
    hCopy = Expr_MemAlloc(iSize);
    if( hCopy && (lpCopy = Expr_MemLock(hCopy)) )
    {
      _fmemcpy(lpCopy, lpExpr, iSize * sizeof(CALC_EXPRINFO));
      for( ; lpCopy->nType != CALC_EXPR_END; lpCopy++)
      {
        switch( lpCopy->nType )
        {
          case CALC_EXPR_BEGIN:
            lpCopy->u.Header.lRefCnt = 1;
            break;
          case CALC_EXPR_STRING:
            lpCopy->u.String.hText = CopyHandleText(lpCopy->u.String.hText);
            break;
          case CALC_EXPR_NAME:
            lpCopy->u.CustName.hName
               = NameCreateRef(lpCopy->u.CustName.hName);
            break;
          case CALC_EXPR_CUSTFUNC:
            lpCopy->u.CustFunc.hFunc
              = CustFuncCreateRef(lpCopy->u.CustFunc.hFunc);
            break;
          #if defined(SS_DDE)
          case CALC_EXPR_DDE:
            lpCopy->u.Dde.hDde = DdeCreateRef(lpCopy->u.Dde.hDde);
            break;
          #endif
        }
      }
      hReturn = hCopy;
      Expr_MemUnlock(hCopy);
    }
    Expr_MemUnlock(hExpr);
  }
  return hReturn;
}

//--------------------------------------------------------------------
//
//  The ExprDestroy() function decrements the reference count for the
//  expression.  When the reference count reaches zero, all memory
//  associated with the given expression is freed.
//
//  Note: It is assumed that the first expression componment
//        has nType = CALC_EXPR_BEGIN.
//

void SS_CalcDdeRemove(long hSS, CALC_HANDLE hDde);

BOOL ExprDestroy(CALC_HANDLE hExpr)
{
  LPCALC_EXPRINFO lpExpr;
  BOOL bNoReferences = FALSE;
  
  if( hExpr && (lpExpr = Expr_MemLock(hExpr)) )
  {
    lpExpr->u.Header.lRefCnt--;
    if( bNoReferences = lpExpr->u.Header.lRefCnt == 0 )
    {
      for( ; lpExpr->nType != CALC_EXPR_END; lpExpr++ )
      {
        switch( lpExpr->nType )
        {
          case CALC_EXPR_STRING:
            CalcMemFree(lpExpr->u.String.hText);
            break;
          case CALC_EXPR_NAME:
            NameDestroy(lpExpr->u.CustName.hName);
            break;
          case CALC_EXPR_CUSTFUNC:
            CustFuncDestroy(lpExpr->u.CustFunc.hFunc);
            break;
          #if defined(SS_DDE)
          case CALC_EXPR_DDE:
            if( 2 == DdeGetRefCnt(lpExpr->u.Dde.hDde) )
            {
              LPCALC_DDE lpDde = (LPCALC_DDE)CalcMemLock(lpExpr->u.Dde.hDde);
              long hSS = lpDde->hSS;
              CalcMemUnlock(lpExpr->u.Dde.hDde);
              SS_CalcDdeRemove(hSS, lpExpr->u.Dde.hDde);
            }
            DdeDestroy(lpExpr->u.Dde.hDde);
            break;
          #endif
        }
      }
    }
    Expr_MemUnlock(hExpr);
  }
  if( bNoReferences )
    Expr_MemFree(hExpr);
  return TRUE;
}

//--------------------------------------------------------------------
//
//  The ExprGetDde() function determines if the expression represents
//  a single DDE link.  If so, the DDE link is returned.
//

CALC_HANDLE ExprGetDde(CALC_HANDLE hExpr, LPLONG lplItemCol, LPLONG lplItemRow)
{
  LPCALC_EXPRINFO lpExpr;
  CALC_HANDLE hDde = 0;

  if( hExpr && (lpExpr = Expr_MemLock(hExpr)) )
  {
    if( CALC_EXPR_BEGIN == lpExpr->nType &&
        CALC_EXPR_DDE == (lpExpr+1)->nType &&
        CALC_EXPR_END == (lpExpr+2)->nType )
    {
      hDde = (lpExpr+1)->u.Dde.hDde;
      if( lplItemCol )
        *lplItemCol = (lpExpr+1)->u.Dde.lItemCol;
      if( lplItemRow )
        *lplItemRow = (lpExpr+1)->u.Dde.lItemRow;
    }
    Expr_MemUnlock(hExpr);
  }
  return hDde;
}

//--------------------------------------------------------------------
//
//  The RelCellToAbsCell() function converts a relative cell reference
//  to an absolute cell address.
//

static void RelCellToAbsCell(long lBaseCol, long lBaseRow,
                             long lRelCol, long lRelRow,
                             LPLONG lplAbsCol, LPLONG lplAbsRow)
{
  *lplAbsCol = CoordMakeAbs(lRelCol, lBaseCol);
  *lplAbsRow = CoordMakeAbs(lRelRow, lBaseRow);
}

//--------------------------------------------------------------------
//
//  The RelRangeToAbsRange() function contverts a relative cell range
//  reference to an absolute cell range reference.
//

static void RelRangeToAbsRange(long lBaseCol, long lBaseRow,
                               long lRelCol1, long lRelRow1,
                               long lRelCol2, long lRelRow2,
                               LPLONG lplAbsCol1, LPLONG lplAbsRow1,
                               LPLONG lplAbsCol2, LPLONG lplAbsRow2)
{
  *lplAbsCol1 = CoordMakeAbs(lRelCol1, lBaseCol);
  *lplAbsRow1 = CoordMakeAbs(lRelRow1, lBaseRow);
  *lplAbsCol2 = CoordMakeAbs(lRelCol2, lBaseCol);
  *lplAbsRow2 = CoordMakeAbs(lRelRow2, lBaseRow);
}

//--------------------------------------------------------------------
//
//  The ExprContainsName() function determines if the given name is
//  contained either directly or indirectly in the given expression.
//  This function is needed to ensure named expressions are not
//  recursive in nature.
//

BOOL ExprContainsName(CALC_HANDLE hExpr, CALC_HANDLE hName)
{
  LPCALC_EXPRINFO lpExpr;
  BOOL bReturn = FALSE;
  
  if( hExpr && (lpExpr = Expr_MemLock(hExpr)) )
  {
    for( ; !bReturn && lpExpr->nType != CALC_EXPR_END; lpExpr++ )
    {
      if( lpExpr->nType == CALC_EXPR_NAME )
      {
        if( hName == lpExpr->u.CustName.hName )
          bReturn = TRUE;
        else
          bReturn = ExprContainsName(NameGetExpr(lpExpr->u.CustName.hName),
                                     hName);
      }
    }
    Expr_MemUnlock(hExpr);
  }
  return bReturn;
}

//--------------------------------------------------------------------
//
//  The ExprContainsMixedColRange() function determines whether or
//  not the expression contains a range reference which was both
//  an absolute column and a relative column.
//

BOOL ExprContainsMixedColRange(CALC_HANDLE hExpr)
{
  LPCALC_EXPRINFO lpExpr;
  BOOL bRet = FALSE;
  
  if (hExpr && (lpExpr = Expr_MemLock(hExpr)))
  {
    for (; !bRet && lpExpr->nType != CALC_EXPR_END; lpExpr++)
    {
      if (lpExpr->nType == CALC_EXPR_RANGE)
        bRet = CoordIsAbs(lpExpr->u.Range.lCol1) != CoordIsAbs(lpExpr->u.Range.lCol2);
      else if (lpExpr->nType == CALC_EXPR_EXTERNALRANGE)
        bRet = CoordIsAbs(lpExpr->u.ExternalRange.lCol1) != CoordIsAbs(lpExpr->u.ExternalRange.lCol2);
      else if (CALC_EXPR_CELL == lpExpr->nType && CALC_EXPR_CELL == (lpExpr+1)->nType && CALC_EXPR_OPERATOR == (lpExpr+2)->nType && CALC_OPER_RANGE == (lpExpr+2)->u.Oper.nId)
        bRet = CoordIsAbs(lpExpr->u.Cell.lCol) != CoordIsAbs((lpExpr+1)->u.Cell.lCol);
    }
    Expr_MemUnlock(hExpr);
  }
  return bRet;
}

//--------------------------------------------------------------------
//
//  The ExprContainsMixedColRange() function determines whether or
//  not the expression contains a range reference which was both
//  an absolute row and a relative row.
//

BOOL ExprContainsMixedRowRange(CALC_HANDLE hExpr)
{
  LPCALC_EXPRINFO lpExpr;
  BOOL bRet = FALSE;
  
  if (hExpr && (lpExpr = Expr_MemLock(hExpr)))
  {
    for (; !bRet && lpExpr->nType != CALC_EXPR_END; lpExpr++)
    {
      if (lpExpr->nType == CALC_EXPR_RANGE)
        bRet = CoordIsAbs(lpExpr->u.Range.lRow1) != CoordIsAbs(lpExpr->u.Range.lRow2);
      else if (lpExpr->nType == CALC_EXPR_EXTERNALRANGE)
        bRet = CoordIsAbs(lpExpr->u.ExternalRange.lRow1) != CoordIsAbs(lpExpr->u.ExternalRange.lRow2);
      else if (CALC_EXPR_CELL == lpExpr->nType && CALC_EXPR_CELL == (lpExpr+1)->nType && CALC_EXPR_OPERATOR == (lpExpr+2)->nType && CALC_OPER_RANGE == (lpExpr+2)->u.Oper.nId)
        bRet = CoordIsAbs(lpExpr->u.Cell.lRow) != CoordIsAbs((lpExpr+1)->u.Cell.lRow);
    }
    Expr_MemUnlock(hExpr);
  }
  return bRet;
}

//--------------------------------------------------------------------
//
//  The Skip() function moves backwards over the given number of
//  operands in the expression.
//

static LPCALC_EXPRINFO Skip(LPCALC_EXPRINFO lpExpr, int nSkip)
{
  for( ; nSkip > 0; lpExpr-- )
  {
    switch( lpExpr->nType )
    {
      case CALC_EXPR_LONG:
      case CALC_EXPR_DOUBLE:
      case CALC_EXPR_STRING:
      case CALC_EXPR_ERROR:
      case CALC_EXPR_CELL:
      case CALC_EXPR_RANGE:
      case CALC_EXPR_EXTERNALCELL:
      case CALC_EXPR_EXTERNALRANGE:
      case CALC_EXPR_NAME:
      case CALC_EXPR_DDE:
        nSkip--;
        break;
      case CALC_EXPR_FUNCTION:
        nSkip += lpExpr->u.Func.nArgs - 1;
        break;
      case CALC_EXPR_CUSTFUNC:
        nSkip += lpExpr->u.CustFunc.nArgs - 1;
        break;
      case CALC_EXPR_OPERATOR:
        if( lpExpr->u.Oper.nId != CALC_OPER_NEG )
          nSkip++;
        break;
      case CALC_EXPR_SEPARATOR:
        if( lpExpr->u.Sep.nId == CALC_SEP_LPAR )
          nSkip--;
        else if( lpExpr->u.Sep.nId == CALC_SEP_RPAR )
          nSkip++;
        break;
    }
  }
  return lpExpr;
}

//--------------------------------------------------------------------
//
//  The PrintExpr() prototype is needed by the PrintArgList()
//  function.
//
static int PrintExpr(LPCALC_EXPRINFO lpExpr,
                     long lCol, long lRow,
                     short nRefStyle,
#if SS_V80 // 24919 -scl
                     BOOL bExcel,
#endif
                     LPTSTR lpszText, int nLen);

//--------------------------------------------------------------------
//
//  The PrintArgList() function converts an arguement list back into
//  its text representation.
//
//  Note: ?? needed to add text length checking ??
//

static int PrintArgList(LPCALC_EXPRINFO lpExpr, short nArgs,
                        long lCol, long lRow,
                        short nRefStyle,
#if SS_V80 // 24919 -scl
                        BOOL bExcel,
#endif
                        LPTSTR lpszText, int nLen)
{
  int iResult = 0;
  int i;
  
  lstrcpy(lpszText+iResult, _T("("));
  iResult++;
  for( i = nArgs - 1; i >= 0; i-- )
  {
    iResult += PrintExpr(
      Skip(lpExpr,i), lCol, lRow, nRefStyle,
#if SS_V80 // 24919 -scl
      bExcel,
#endif
      lpszText+iResult, nLen-iResult
    );
    if( i != 0 )
    {
      lstrcpy(lpszText+iResult, _T(","));
      iResult++;
    }
  }
  lstrcpy(lpszText+iResult, _T(")"));
  iResult++;
  return iResult;
}

//--------------------------------------------------------------------
//
//  The LongGetText() function retrieves the text representation
//  of a long.
//

static int LongGetText(long lNum, LPTSTR lpszText, int nLen)
{
  TCHAR szNum[CALC_MAX_NUM_LEN];
  int nNumLen;
  int nRet = 0;
  
  nNumLen = wsprintf(szNum, _T("%ld"), lNum);
  if( nNumLen < nLen)
  {
    lstrcpy(lpszText, szNum);
    nRet = nNumLen;
  }
  return nRet;
}

//--------------------------------------------------------------------
//
//  The DblGetText() function retrieves the text representation
//  of a double.
//

/* RFW - 2/4/02 - 9423
static int DblGetText(double dfNum, LPTSTR lpszText, int nLen)
{
  static TCHAR szNum[CALC_MAX_NUM_LEN];
  int nNumLen;
  int nRet = 0;

  _stprintf(szNum, _T("%.12g"), dfNum);
  if( !_ftcspbrk(szNum, _T(".eE")) )
    lstrcat(szNum, _T(".0"));
  nNumLen = lstrlen(szNum);
  if( nNumLen < nLen )
  {
    lstrcpy(lpszText, szNum);
    nRet = nNumLen;
  }
  return nRet;
}
*/

static int DblGetText(double dfNum, LPTSTR lpszText, int nLen)
{
  static TCHAR szNum[CALC_MAX_NUM_LEN];
  TCHAR cDecimal = CalcGetDefDecimalChar();
  int nNumLen;
  int nRet = 0;

  _stprintf(szNum, _T("%.12g"), dfNum);
  if (cDecimal != '.')
    CalcStringReplaceCh(szNum, '.', cDecimal);
  if( !_ftcspbrk(szNum, _T(".eE")) )
    lstrcat(szNum, _T(".0"));

  nNumLen = lstrlen(szNum);
  if( nNumLen < nLen )
  {
    lstrcpy(lpszText, szNum);
    nRet = nNumLen;
  }
  return nRet;
}

//--------------------------------------------------------------------
//
//  The CellGetText() function retrieves the text representation
//  of a cell.
//

static int CellGetText(long lCol, long lRow, long lBaseCol, long lBaseRow,
                       short nRefStyle, LPTSTR lpszText, int nLen)
{
  int nRet = 0;

  if( CALC_REFSTYLE_R1C1 == nRefStyle )
    nRet = CellToR1C1(lCol, lRow, lpszText, nLen);
  else if( CALC_REFSTYLE_A1 == nRefStyle )
    nRet = CellToA1(lCol, lRow, lBaseCol, lBaseRow, lpszText, nLen);
  else
  {
    nRet = CellToAPound(lCol, lRow, lpszText, nLen);
    #ifdef SPREAD_JPN
    // The Japan version attempts to eliminate '?' characters in the
    // output by converting non-zero offset relative coordinates to
    // absolute coordinates.  While this is technically incorrect
    // behaviour it is what they requested (refer to bug #13666).
    {
      //Modify by BOC 99.8.26 (hyt)----------------
      //for adjust to correct row or col
      //if you drag cell to other position can read Formula correct
      int i;
      BOOL bHaveInvalid=FALSE;
      for(i=0;i<nRet;i++)
        if(lpszText[i]=='?')
          bHaveInvalid=TRUE;
      if(bHaveInvalid)
      {
        if(CoordIsRel(lCol) && lCol != 0x80000000)
          lCol = CoordMakeAbs(lCol, lBaseCol);
        if(CoordIsRel(lRow) && lRow!=0x80000000)
          lRow = CoordMakeAbs(lRow, lBaseRow);
        nRet = CellToAPound(lCol, lRow, lpszText, nLen);
      }
      //---------------------------------------------
    }
    #endif
  }
  return nRet;
}

//--------------------------------------------------------------------
//
//  The RangeGetText() function retrieves the text representation
//  of a range.
//

static int RangeGetText(long lCol1, long lRow1, long lCol2, long lRow2,
                        long lBaseCol, long lBaseRow, short nRefStyle,
                        LPTSTR lpszText, int nLen)
{
  int nRet = 0;

  nRet += CellGetText(lCol1, lRow1, lBaseCol, lBaseRow, nRefStyle, lpszText+nRet, nLen-nRet);
  if (nRet < nLen)
    lpszText[nRet++] = ':';
  nRet += CellGetText(lCol2, lRow2, lBaseCol, lBaseRow, nRefStyle, lpszText+nRet, nLen-nRet);
  return nRet;
}

//--------------------------------------------------------------------

BOOL ContainsSpecialChar(LPTSTR lpszText, int nLen)
{
  int i;

  if (0 < nLen)
  {
    if (!_istalpha((_TUCHAR)lpszText[0]) && lpszText[0] != '_')
      return TRUE;
    for (i = 1; i < nLen; i++)
    {
      if (!_istalnum((_TUCHAR)lpszText[i]) && lpszText[i] != '_')
        return TRUE;
    }
  }
  return FALSE;
}

//--------------------------------------------------------------------

static int ExpandSingleQuote(LPTSTR lpszDest, int nDestLen, LPTSTR lpszSrc, int nSrcLen)
{
  int i;
  int j;

  for (i = 0, j = 0; i < nSrcLen && j < nDestLen; i++)
  {
    lpszDest[j++] = lpszSrc[i];
    if (lpszSrc[i] == '\'' && j < nDestLen)
      lpszDest[j++] = lpszSrc[i];
  }
  return j;
}

//--------------------------------------------------------------------
//
//  The CellGetText() function retrieves the text representation
//  of an external cell.
//

static int ExternalCellGetText(LPCALC_SHEET lpSheet, long lCol, long lRow, long lBaseCol, long lBaseRow,
                               short nRefStyle, LPTSTR lpszText, int nLen)
{
  BOOL bSpecialChar;
  TCHAR szName[CALC_MAX_SHEET_LEN];
  int nNameLen;
  int nRet = 0;

  nNameLen = lpSheet->lpfnGetName(lpSheet->hSS, szName, sizeof(szName)/sizeof(TCHAR));
  bSpecialChar = ContainsSpecialChar(szName, nNameLen);
  if (bSpecialChar && nRet < nLen)
    lpszText[nRet++] = '\'';
  nRet += ExpandSingleQuote(lpszText+nRet, nLen-nRet, szName, nNameLen);
  if (bSpecialChar && nRet < nLen)
    lpszText[nRet++] = '\'';
  if (nRet < nLen)
    lpszText[nRet++] = '!';
  nRet += CellGetText(lCol, lRow, lBaseCol, lBaseRow, nRefStyle, lpszText+nRet, nLen-nRet);
  return nRet;
}

//--------------------------------------------------------------------
//
//  The ExternalRangeGetText() function retrieves the text representation
//  of an external range.
//

static int ExternalRangeGetText(LPCALC_SHEET lpSheet, long lCol1, long lRow1, long lCol2, long lRow2,
                                long lBaseCol, long lBaseRow, short nRefStyle,
                                LPTSTR lpszText, int nLen)
{
  int nRet = 0;

  nRet += ExternalCellGetText(lpSheet, lCol1, lRow1, lBaseCol, lBaseRow, nRefStyle, lpszText+nRet, nLen-nRet);
  if (nRet < nLen)
    lpszText[nRet++] = ':';
  nRet += CellGetText(lCol2, lRow2, lBaseCol, lBaseRow, nRefStyle, lpszText+nRet, nLen-nRet);
  return nRet;
}

//--------------------------------------------------------------------
//
//  The PrintExpr() function converts a parsed expression back into
//  its text representation.
//

static int PrintExpr(LPCALC_EXPRINFO lpExpr,
                     long lCol, long lRow,
                     short nRefStyle,
#if SS_V80 // 24919 -scl
                     BOOL bExcel,
#endif

                     LPTSTR lpszText, int nLen)
{
  LPTSTR lpszStr;
  int iResult = 0;
  
  switch( lpExpr->nType )
  {
    case CALC_EXPR_LONG:
      iResult = LongGetText(lpExpr->u.Long.lVal, lpszText, nLen);
      break;    
    case CALC_EXPR_DOUBLE:
      iResult = DblGetText(lpExpr->u.Double.dfVal, lpszText, nLen);
      break;    
    case CALC_EXPR_STRING:
      if( lpszStr = (LPTSTR)CalcMemLock(lpExpr->u.String.hText) )
      {
        int nStrLen = lstrlen(lpszStr) + 2;
        if( nStrLen < nLen )
        {
          iResult = nStrLen;
          lstrcpy(lpszText, _T("\""));
          lstrcat(lpszText, lpszStr);
          lstrcat(lpszText, _T("\""));
        }
        CalcMemUnlock(lpExpr->u.String.hText);
      }
      break;    
    case CALC_EXPR_ERROR:
      iResult = ErrGetText(lpExpr->u.Error.nId, lpszText, nLen);
      break;
    case CALC_EXPR_SEPARATOR:
      if( lpExpr->u.Sep.nId == CALC_SEP_RPAR )
      {
        lstrcpy(lpszText, _T("("));
        iResult = 1;
        iResult += PrintExpr(
          lpExpr-1, lCol, lRow, nRefStyle,
#if SS_V80 // 24919 -scl
          bExcel,
#endif
          lpszText+iResult, nLen-iResult
        );
        lstrcpy(lpszText + iResult, _T(")"));
        iResult++;
      }
      break;    
    case CALC_EXPR_OPERATOR:
#if SS_V80 // 24919 -scl
       {   // replace '&' and '|' operators with AND and OR functions for Excel compatibility
          BOOL isAndOper = lpExpr->u.Oper.nId == CALC_OPER_AND;
          BOOL isOrOper = lpExpr->u.Oper.nId == CALC_OPER_OR;
          BOOL isNotOper = lpExpr->u.Oper.nId == CALC_OPER_NOT;
          if( bExcel && (isAndOper || isOrOper || isNotOper) )
          {
             if( isAndOper )
                iResult = FuncGetText(CALC_FUNC_AND, lpszText, nLen);
             else if( isOrOper )
                iResult = FuncGetText(CALC_FUNC_OR, lpszText, nLen);
             else // isNotOper
                iResult = FuncGetText(CALC_FUNC_NOT, lpszText, nLen);
             if( isAndOper || isOrOper )
               iResult += PrintArgList(lpExpr-1, 2, lCol, lRow, nRefStyle, TRUE, lpszText+iResult, nLen-iResult);
             else // isNotOper
               iResult += PrintArgList(lpExpr-1, 1, lCol, lRow, nRefStyle, TRUE, lpszText+iResult, nLen-iResult);
             break;
          }
       }
#endif
      if( OperIsBinary(lpExpr->u.Oper.nId) )
        iResult = PrintExpr(
          Skip(lpExpr-1,1), lCol, lRow, nRefStyle,
#if SS_V80 // 24919 -scl
          bExcel,
#endif
          lpszText, nLen
        );
      OperGetText(lpExpr->u.Oper.nId, lpszText+iResult, nLen-iResult);
      iResult += lstrlen(lpszText + iResult);
      iResult += PrintExpr(
        lpExpr-1, lCol, lRow, nRefStyle,
#if SS_V80 // 24919 -scl
        bExcel,
#endif
        lpszText+iResult, nLen-iResult
      );
      break;    
    case CALC_EXPR_NAME:
      iResult = NameGetText(lpExpr->u.CustName.hName, lpszText, nLen);
      if( 0 == iResult && nLen > 0 )
        iResult = lstrlen(lstrcpy(lpszText, _T("?")));  //????
      break;
    case CALC_EXPR_FUNCTION:
#if SS_V80 // 24919 -scl
       // replace PMTXL function with PMT for Excel compatibility
       if( bExcel && lpExpr->u.Func.nId == CALC_FUNC_PMTXL )
          iResult = FuncGetText(CALC_FUNC_PMT, lpszText, nLen);
//       else if ( bExcel && lpExpr->u.Func.nId == CALC_FUNC_PMT )
//       {
//
//       }
#endif
      iResult = FuncGetText(lpExpr->u.Func.nId, lpszText, nLen);
      iResult += PrintArgList(
        lpExpr-1, lpExpr->u.Func.nArgs, lCol, lRow, nRefStyle,
#if SS_V80 // 24919 -scl
        bExcel,
#endif
        lpszText+iResult, nLen-iResult
      );
      break;
    case CALC_EXPR_CUSTFUNC:
      iResult = CustFuncGetText(lpExpr->u.CustFunc.hFunc, lpszText, nLen);
      if( 0 == iResult && nLen > 0 )
        iResult = lstrlen(lstrcpy(lpszText, _T("?")));  //????
      iResult += PrintArgList(
        lpExpr-1, lpExpr->u.CustFunc.nArgs, lCol, lRow, nRefStyle,
#if SS_V80 // 24919 -scl
        bExcel,
#endif
        lpszText+iResult, nLen-iResult
      );
      break;
    case CALC_EXPR_CELL:
      iResult += CellGetText(lpExpr->u.Cell.lCol, lpExpr->u.Cell.lRow,
                             lCol, lRow, nRefStyle, lpszText, nLen);
      break;
    case CALC_EXPR_RANGE:
      iResult += RangeGetText(lpExpr->u.Range.lCol1, lpExpr->u.Range.lRow1,
                              lpExpr->u.Range.lCol2, lpExpr->u.Range.lRow2,
                              lCol, lRow, nRefStyle, lpszText, nLen);
      break;
    case CALC_EXPR_EXTERNALCELL:
      iResult += ExternalCellGetText(lpExpr->u.ExternalCell.lpSheet, lpExpr->u.ExternalCell.lCol, lpExpr->u.ExternalCell.lRow,
                                     lCol, lRow, nRefStyle, lpszText, nLen);
      break;
    case CALC_EXPR_EXTERNALRANGE:
      iResult += ExternalRangeGetText(lpExpr->u.ExternalRange.lpSheet,
                                      lpExpr->u.ExternalRange.lCol1, lpExpr->u.ExternalRange.lRow1,
                                      lpExpr->u.ExternalRange.lCol2, lpExpr->u.ExternalRange.lRow2,
                                      lCol, lRow, nRefStyle, lpszText, nLen);
      break;
    #if defined(SS_DDE)
    case CALC_EXPR_DDE:
      iResult += DdeGetText(lpExpr->u.Dde.hDde, lpExpr->u.Dde.lItemCol,
                            lpExpr->u.Dde.lItemRow, lpszText, nLen);
      break;
    #endif
  }
  return iResult;
}

//--------------------------------------------------------------------
//
//  The ExprGetText() function retrieves the text representation of
//  an expression.
//

int ExprGetText(CALC_HANDLE hExpr, long lCol, long lRow, short nRefStyle,
#if SS_V80 // 24919 -scl
                BOOL bExcel,
#endif
                LPTSTR lpszText, int nLen)
{
  LPCALC_EXPRINFO lpExpr;
  int iResult = 0;
  
  if( hExpr && (lpExpr = Expr_MemLock(hExpr)) )
  {
    if( lpExpr->nType != CALC_EXPR_END )
    {
      while( lpExpr->nType != CALC_EXPR_END )
        lpExpr++;
      iResult = PrintExpr(
        --lpExpr, lCol, lRow, nRefStyle,
#if SS_V80 // 24919 -scl
        bExcel,
#endif
        lpszText, nLen
      );
    }
    Expr_MemUnlock(hExpr);
  }
  if( iResult == 0 && nLen > 0 )
    lpszText[0] = '\0';
  return iResult;
}

//--------------------------------------------------------------------
//
//  The ExprGetTextLen() function retrieves the length of the text
//  representation of an expression.
//

int ExprGetTextLen(CALC_HANDLE hExpr, long lCol, long lRow, short nRefStyle
#if SS_V80 // 24919 -scl
                  , BOOL bExcel
#endif
                  )
{
  LPCALC_EXPRINFO lpExpr;
  LPTSTR lpszText;
  TCHAR szText[2*CALC_MAX_SHEET_LEN + 2*CALC_MAX_COORD_LEN];
  const int nTextMaxLen = 2*CALC_MAX_SHEET_LEN + 2*CALC_MAX_COORD_LEN;
  int nNameLen;
  int nCustFuncLen;
  int nCommas;
  int nRet = 0;

  if( hExpr && (lpExpr = Expr_MemLock(hExpr)) )
  {
    for( ; lpExpr->nType != CALC_EXPR_END; lpExpr++ )
    {
      switch( lpExpr->nType )
      {
        case CALC_EXPR_LONG:
          nRet += LongGetText(lpExpr->u.Long.lVal, szText, nTextMaxLen);
          break;
        case CALC_EXPR_DOUBLE:
          nRet += DblGetText(lpExpr->u.Double.dfVal, szText, nTextMaxLen);
          break;
        case CALC_EXPR_STRING:
          if( lpszText = (LPTSTR)CalcMemLock(lpExpr->u.String.hText) )
          {
            nRet += lstrlen(lpszText) + 2;
            CalcMemUnlock(lpExpr->u.String.hText);
          }
          break;
        case CALC_EXPR_ERROR:
          nRet += ErrGetTextLen(lpExpr->u.Error.nId);
          break;
        case CALC_EXPR_SEPARATOR:
          nRet += SepGetTextLen(lpExpr->u.Sep.nId);
          break;
        case CALC_EXPR_OPERATOR:
#if SS_V80 // 24919 -scl
          {   // replace '&' and '|' operators with AND and OR functions for Excel compatibility
             BOOL isAndOper = lpExpr->u.Oper.nId == CALC_OPER_AND;
             BOOL isOrOper = lpExpr->u.Oper.nId == CALC_OPER_OR;
             BOOL isNotOper = lpExpr->u.Oper.nId == CALC_OPER_NOT;
             if( bExcel && (isAndOper || isOrOper || isNotOper) )
             {
                if( isAndOper )
                   nRet += FuncGetTextLen(CALC_FUNC_AND);
                else if( isOrOper )
                   nRet += FuncGetTextLen(CALC_FUNC_OR);
                else // isNotOper
                   nRet += FuncGetTextLen(CALC_FUNC_NOT);
                if( isAndOper || isOrOper )
                  nRet += 3;
                else // isNotOper
                  nRet += 2;
                break;
             }
          }
#endif
          nRet += OperGetTextLen(lpExpr->u.Oper.nId);
          break;
        case CALC_EXPR_NAME:
          nNameLen = NameGetTextLen(lpExpr->u.CustName.hName);
          if( 0 == nNameLen )
            nNameLen = 1;
          nRet += nNameLen;
          break;
        case CALC_EXPR_FUNCTION:
          nCommas = max(0, lpExpr->u.Func.nArgs - 1);
#if SS_V80 // 24919 -scl
          // replace PMTXL function with PMT for Excel compatibility
          if( bExcel && lpExpr->u.Func.nId == CALC_FUNC_PMTXL )
            nRet += FuncGetTextLen(CALC_FUNC_PMT) + nCommas + 2;
//       else if ( bExcel && lpExpr->u.Func.nId == CALC_FUNC_PMT )
//       {
//
//       }
          else
#endif
            nRet += FuncGetTextLen(lpExpr->u.Func.nId) + nCommas + 2;
          break;
        case CALC_EXPR_CUSTFUNC:
          nCustFuncLen = CustFuncGetTextLen(lpExpr->u.CustFunc.hFunc);
          if( 0 == nCustFuncLen )
            nCustFuncLen = 1;
          nCommas = max(0, lpExpr->u.CustFunc.nArgs - 1);
          nRet += nCustFuncLen + nCommas + 2;
          break;
        #if defined(SS_DDE)
        case CALC_EXPR_DDE:
          nRet += DdeGetTextLen(lpExpr->u.Dde.hDde, lpExpr->u.Dde.lItemCol,
                                lpExpr->u.Dde.lItemRow);
          break;
        #endif
        case CALC_EXPR_CELL:
          nRet += CellGetText(lpExpr->u.Cell.lCol, lpExpr->u.Cell.lRow,
                              lCol, lRow, nRefStyle, szText, nTextMaxLen);
          break;
        case CALC_EXPR_RANGE:
          nRet += RangeGetText(lpExpr->u.Range.lCol1, lpExpr->u.Range.lRow1,
                               lpExpr->u.Range.lCol2, lpExpr->u.Range.lRow2,
                               lCol, lRow, nRefStyle, szText, nTextMaxLen);
          break;
        case CALC_EXPR_EXTERNALCELL:
          nRet += ExternalCellGetText(lpExpr->u.ExternalCell.lpSheet, lpExpr->u.ExternalCell.lCol, lpExpr->u.ExternalCell.lRow,
                                      lCol, lRow, nRefStyle, szText, nTextMaxLen);
          break;
        case CALC_EXPR_EXTERNALRANGE:
          nRet += ExternalRangeGetText(lpExpr->u.ExternalRange.lpSheet,
                                       lpExpr->u.ExternalRange.lCol1, lpExpr->u.ExternalRange.lRow1,
                                       lpExpr->u.ExternalRange.lCol2, lpExpr->u.ExternalRange.lRow2,
                                       lCol, lRow, nRefStyle, szText, nTextMaxLen);
          break;
      }
    }
    Expr_MemUnlock(hExpr);
  }
  return nRet;
}

//--------------------------------------------------------------------
//
//  The EvalCellRefs() function converts the given number of cell
//  references to the corresponding cell values.
//                                                                    

BOOL CALLBACK EvalCellRefs(LPCALC_VALUE lpArg, short nArgs)
{  
  for( ; nArgs > 0; nArgs--, lpArg++ )
  {
    if( lpArg->Status == CALC_VALUE_STATUS_OK )
    {
      if( lpArg->Type == CALC_VALUE_TYPE_CELL )
      {
        LPCALC_SHEET lpSheet = (LPCALC_SHEET)lpArg->Val.ValCell.u.lInfo;
        lpSheet->lpfnGetData(lpSheet->hSS, lpArg->Val.ValCell.Col,
                             lpArg->Val.ValCell.Row, lpArg);
      }
    }
  }
  return TRUE;
}

//--------------------------------------------------------------------
//
//  The EvalRangeRefs() function converts the given number of range
//  references to the corresponding cell values.
//                                                                    

BOOL CALLBACK EvalRangeRefs(LPCALC_VALUE lpArg, short nArgs)
{  
  for( ; nArgs > 0; nArgs--, lpArg++ )
  {
    if( lpArg->Status == CALC_VALUE_STATUS_OK )
    {
      if( lpArg->Type == CALC_VALUE_TYPE_RANGE )
      {
        LPCALC_SHEET lpSheet = (LPCALC_SHEET)lpArg->Val.ValRange.u.lInfo;
        lpSheet->lpfnGetData(lpSheet->hSS, lpArg->Val.ValRange.Col1,
                             lpArg->Val.ValRange.Row1, lpArg);
      }
    }
  }
  return TRUE;
}

//--------------------------------------------------------------------
//
//  The EvalDdeRefs() function converts the given number of dde
//  references to the corresponding values.
//                                                                    

BOOL CALLBACK EvalDdeRefs(LPCALC_VALUE lpArg, short nArgs)
{
#if defined(SS_DDE)  
  for( ; nArgs > 0; nArgs--, lpArg++ )
  {
    if( lpArg->Status == CALC_VALUE_STATUS_OK )
    {
      if( lpArg->Type == CALC_VALUE_TYPE_DDE )
        DdeGetValue(lpArg->Val.ValDde.hDde, lpArg->Val.ValDde.lCol,
                    lpArg->Val.ValDde.lRow, lpArg);
    }
  }
#endif
  return TRUE;
}

//--------------------------------------------------------------------
//
//  The ExprSemiEval() function performs a partial evaluation of the
//  given expression.  The resulting stack of values will contain all
//  cell and range refernces used by the expression during evaluation.
//
//  Note: need to check for stack overflow.
//

static BOOL SemiEvalRange(LPCALC_VALUE lpResult, LPCALC_VALUE lpArg1, LPCALC_VALUE lpArg2)
{
  LPCALC_SHEET lpSheet1, lpSheet2;
  long lCol1, lCol2, lCol3, lCol4;
  long lRow1, lRow2, lRow3, lRow4;
  
  if( CheckAllCellOrRange(lpArg1, 1) && CheckAllCellOrRange(lpArg2, 1) )
  {
    ValGetRange(lpArg1, &lpSheet1, &lCol1, &lRow1, &lCol2, &lRow2);
    ValGetRange(lpArg2, &lpSheet2, &lCol3, &lRow3, &lCol4, &lRow4);
    if( lpSheet1 == lpSheet2 &&
        CoordIsRel(lCol1) == CoordIsRel(lCol3) &&
        CoordIsRel(lRow1) == CoordIsRel(lRow3) &&
        CoordIsRel(lCol2) == CoordIsRel(lCol4) &&
        CoordIsRel(lRow2) == CoordIsRel(lRow4) )
    {
      lCol1 = CoordMin(lCol1, lCol3);
      lRow1 = CoordMin(lRow1, lRow3);
      lCol2 = CoordMax(lCol2, lCol4);
      lRow2 = CoordMax(lRow2, lRow4);
      ValSetRange(lpResult, lpSheet1, lCol1, lRow1, lCol2, lRow2);
    }
    else
      ValSetError(lpResult);
  }
  else
    ValSetError(lpResult);
  return TRUE;
}

static BOOL ExprSemiEval(LPCALC_EXPRINFO lpExpr,
                         LPCALC_SHEET lpSheet, long lCol, long lRow,
                         LPCALC_VALUESTACK lpStack)
{
  CALC_VALUE result, arg1, arg2;
  CALC_HANDLE hNameExpr;
  LPCALC_EXPRINFO lpNameExpr;
  BOOL bReturn = TRUE;
  long lCellCol, lCellRow;
  long lRangeCol1, lRangeRow1;
  long lRangeCol2, lRangeRow2;
  
  for( ; bReturn && lpExpr->nType != CALC_EXPR_END; lpExpr++ )
  {
    switch( lpExpr->nType )
    {
      case CALC_EXPR_LONG:
      case CALC_EXPR_DOUBLE:
      case CALC_EXPR_STRING:
      case CALC_EXPR_ERROR:
      case CALC_EXPR_DDE:
        ValSetEmpty(&result);
        ValStackPush(lpStack, &result);
        break;
      case CALC_EXPR_CELL:
        lCellCol = lpExpr->u.Cell.lCol;
        lCellRow = lpExpr->u.Cell.lRow;
        if( lCol != CALC_ALLCOLS )
          lCellCol = CoordMakeAbs(lCellCol, lCol);
        if( lRow != CALC_ALLROWS )
          lCellRow = CoordMakeAbs(lCellRow, lRow);
        ValSetCell(&result, lpSheet, lCellCol, lCellRow);
        ValStackPush(lpStack, &result);
        break;
      case CALC_EXPR_RANGE:
        lRangeCol1 = lpExpr->u.Range.lCol1;
        lRangeRow1 = lpExpr->u.Range.lRow1;
        lRangeCol2 = lpExpr->u.Range.lCol2;
        lRangeRow2 = lpExpr->u.Range.lRow2;
        if( lCol != CALC_ALLCOLS )
        {
          lRangeCol1 = CoordMakeAbs(lRangeCol1, lCol);
          lRangeCol2 = CoordMakeAbs(lRangeCol2, lCol);
        }
        if( lRow != CALC_ALLROWS )
        {
          lRangeRow1 = CoordMakeAbs(lRangeRow1, lRow);
          lRangeRow2 = CoordMakeAbs(lRangeRow2, lRow);
        }
        ValSetRange(&result, lpSheet, lRangeCol1, lRangeRow1, lRangeCol2, lRangeRow2);
        ValStackPush(lpStack, &result);
        break;
      case CALC_EXPR_EXTERNALCELL:
        lCellCol = lpExpr->u.ExternalCell.lCol;
        lCellRow = lpExpr->u.ExternalCell.lRow;
        if( lCol != CALC_ALLCOLS )
          lCellCol = CoordMakeAbs(lCellCol, lCol);
        if( lRow != CALC_ALLROWS )
          lCellRow = CoordMakeAbs(lCellRow, lRow);
        ValSetCell(&result, lpExpr->u.ExternalCell.lpSheet, lCellCol, lCellRow);
        ValStackPush(lpStack, &result);
        break;
      case CALC_EXPR_EXTERNALRANGE:
        lRangeCol1 = lpExpr->u.ExternalRange.lCol1;
        lRangeRow1 = lpExpr->u.ExternalRange.lRow1;
        lRangeCol2 = lpExpr->u.ExternalRange.lCol2;
        lRangeRow2 = lpExpr->u.ExternalRange.lRow2;
        if( lCol != CALC_ALLCOLS )
        {
          lRangeCol1 = CoordMakeAbs(lRangeCol1, lCol);
          lRangeCol2 = CoordMakeAbs(lRangeCol2, lCol);
        }
        if( lRow != CALC_ALLROWS )
        {
          lRangeRow1 = CoordMakeAbs(lRangeRow1, lRow);
          lRangeRow2 = CoordMakeAbs(lRangeRow2, lRow);
        }
        ValSetRange(&result, lpExpr->u.ExternalRange.lpSheet, lRangeCol1, lRangeRow1, lRangeCol2, lRangeRow2);
        ValStackPush(lpStack, &result);
        break;
      case CALC_EXPR_NAME:
        hNameExpr = NameGetExpr(lpExpr->u.CustName.hName);
        if( hNameExpr && (lpNameExpr = Expr_MemLock(hNameExpr)) )
        {
          ExprSemiEval(lpNameExpr, lpSheet, lCol, lRow, lpStack);
          Expr_MemUnlock(hNameExpr);
        }
        else
        {
          bReturn = FALSE;
          ValSetEmpty(&result);
          ValStackPush(lpStack, &result);
        }
        break;
      case CALC_EXPR_OPERATOR:
        if( lpExpr->u.Oper.nId == CALC_OPER_RANGE )
        {
          ValStackPop(lpStack, &arg2);
          ValStackPop(lpStack, &arg1);
          SemiEvalRange(&result, &arg1, &arg2);
          ValStackPush(lpStack, &result);
        }   
        break;
      case CALC_EXPR_FUNCTION:
      case CALC_EXPR_CUSTFUNC:
      case CALC_EXPR_SEPARATOR:
      case CALC_EXPR_BEGIN:
        break;
      default:
        bReturn = FALSE;
    }
  }
  return bReturn;
}

//--------------------------------------------------------------------
//
//  The ExprFullEval() function performs a full evaluation of the given
//  given expression.
//

static BOOL ExprFullEval(LPCALC_EXPRINFO lpExpr, LPCALC_SHEET lpSheet,
                         long lCol, long lRow, LPCALC_VALUE lpResult,
                         LPCALC_VALUESTACK lpStack)
{
  LPTSTR lpszText;
  CALC_VALUE result;
  CALC_VALUE val;
  CALC_HANDLE hNameExpr;
  LPCALC_EXPRINFO lpNameExpr;
  CALC_EVALPROC lpfnProc;
  CALC_CUSTEVALPROC lpfnCustProc;
  long lFlags;
  long lCellCol, lCellRow;
  long lRangeCol1, lRangeRow1;
  long lRangeCol2, lRangeRow2;
  BOOL bReturn = TRUE;
  short nArgs;
  
  for( ; bReturn && lpExpr->nType != CALC_EXPR_END; lpExpr++ )
  {
    switch( lpExpr->nType )
    {
      case CALC_EXPR_LONG:
        ValSetLong(&val, lpExpr->u.Long.lVal);
        ValStackPush(lpStack, &val);
        break;
      case CALC_EXPR_DOUBLE:
        ValSetDouble(&val, lpExpr->u.Double.dfVal);
        ValStackPush(lpStack, &val);
        break;
      case CALC_EXPR_STRING:
        lpszText = (LPTSTR)CalcMemLock(lpExpr->u.String.hText);
        ValSetString(&val, lpszText);
        CalcMemUnlock(lpExpr->u.String.hText);
        ValStackPush(lpStack, &val);
        break;
      case CALC_EXPR_ERROR:
        ValSetError(&val);
        ValStackPush(lpStack, &val);
        break;
      case CALC_EXPR_CELL:
        RelCellToAbsCell(lCol, lRow, lpExpr->u.Cell.lCol, lpExpr->u.Cell.lRow, &lCellCol, &lCellRow);
        ValSetCell(&val, lpSheet, lCellCol, lCellRow);
        ValStackPush(lpStack, &val);
        break;
      case CALC_EXPR_RANGE:
        RelRangeToAbsRange(lCol, lRow,
                           lpExpr->u.Range.lCol1, lpExpr->u.Range.lRow1,
                           lpExpr->u.Range.lCol2, lpExpr->u.Range.lRow2,
                           &lRangeCol1, &lRangeRow1,
                           &lRangeCol2, &lRangeRow2);
        ValSetRange(&val, lpSheet, lRangeCol1, lRangeRow1, lRangeCol2, lRangeRow2);
        ValStackPush(lpStack, &val);
        break;
      case CALC_EXPR_EXTERNALCELL:
        RelCellToAbsCell(lCol, lRow, lpExpr->u.ExternalCell.lCol, lpExpr->u.ExternalCell.lRow, &lCellCol, &lCellRow);
        ValSetCell(&val, lpExpr->u.ExternalCell.lpSheet, lCellCol, lCellRow);
        ValStackPush(lpStack, &val);
        break;
      case CALC_EXPR_EXTERNALRANGE:
        RelRangeToAbsRange(lCol, lRow,
                           lpExpr->u.ExternalRange.lCol1, lpExpr->u.ExternalRange.lRow1,
                           lpExpr->u.ExternalRange.lCol2, lpExpr->u.ExternalRange.lRow2,
                           &lRangeCol1, &lRangeRow1,
                           &lRangeCol2, &lRangeRow2);
        ValSetRange(&val, lpExpr->u.ExternalRange.lpSheet, lRangeCol1, lRangeRow1, lRangeCol2, lRangeRow2);
        ValStackPush(lpStack, &val);
        break;
      case CALC_EXPR_NAME:
        hNameExpr = NameGetExpr(lpExpr->u.CustName.hName);
        if( hNameExpr && (lpNameExpr = Expr_MemLock(hNameExpr)) )
        {
          ExprFullEval(lpNameExpr, lpSheet, lCol, lRow, &result, lpStack);
          ValStackPush(lpStack, &result);
          Expr_MemUnlock(hNameExpr);
        }
        else
        {
          bReturn = FALSE;
          ValSetError(&val);
          ValStackPush(lpStack, &val);
        }
        break;
      case CALC_EXPR_FUNCTION:
        nArgs = lpExpr->u.Func.nArgs;
        lFlags = FuncGetFlags(lpExpr->u.Func.nId);
        if( !(lFlags & CALC_WANTCELLREF) )
          EvalCellRefs(lpStack->lpElem + lpStack->nElemCnt - nArgs, nArgs);
        if( !(lFlags & CALC_WANTRANGEREF) )
          EvalRangeRefs(lpStack->lpElem + lpStack->nElemCnt - nArgs, nArgs);
        if( !(lFlags & CALC_WANTDDEREF) )
          EvalDdeRefs(lpStack->lpElem + lpStack->nElemCnt - nArgs, nArgs);
        if( lpfnProc = FuncGetProc(lpExpr->u.Func.nId) )
        {
          bReturn = lpfnProc(&result, lpStack->lpElem + lpStack->nElemCnt - nArgs, nArgs);
          ValStackRemove(lpStack, nArgs);
          ValStackPush(lpStack, &result);
        }
        else
        {
          bReturn = FALSE;
          ValStackRemove(lpStack, nArgs);
          ValSetError(&val);
          ValStackPush(lpStack, &val);
        }
        break;
      case CALC_EXPR_CUSTFUNC:
        nArgs = lpExpr->u.CustFunc.nArgs;
        lFlags = CustFuncGetFlags(lpExpr->u.CustFunc.hFunc);
        if( !(lFlags & CALC_WANTCELLREF) )
          EvalCellRefs(lpStack->lpElem + lpStack->nElemCnt - nArgs, nArgs);
        if( !(lFlags & CALC_WANTRANGEREF) )
          EvalRangeRefs(lpStack->lpElem + lpStack->nElemCnt - nArgs, nArgs);
        if( !(lFlags & CALC_WANTDDEREF) )
          EvalDdeRefs(lpStack->lpElem + lpStack->nElemCnt - nArgs, nArgs);
        if( lpfnCustProc = CustFuncGetProc(lpExpr->u.CustFunc.hFunc ) )
        {
          LPCALC_VALUE lpArg;
          int i;
          for( i = 1; i <= nArgs; i++ )
          {
            lpArg = lpStack->lpElem + lpStack->nElemCnt - i;
            if( CALC_VALUE_STATUS_OK == lpArg->Status )
            {
              if( CALC_VALUE_TYPE_CELL == lpArg->Type )
              {
                #ifdef SS_V70
                lpArg->Val.ValCell.nSheet++;
                #endif
                lpArg->Val.ValCell.u.lInfo = lpSheet->lpfnGetCustRef(lpSheet->hSS);
              }
              else if( CALC_VALUE_TYPE_RANGE == lpArg->Type )
              {
                #ifdef SS_V70
                lpArg->Val.ValRange.nSheet++;
                #endif
                lpArg->Val.ValRange.u.lInfo = lpSheet->lpfnGetCustRef(lpSheet->hSS);
              }
            }
          }
          #if defined(SS_OCX) || defined(SS_VB)
          {
          LPCTSTR lpszFuncName = CustFuncLockText(lpExpr->u.CustFunc.hFunc);
          lpSheet->lpfnSetSending(lpSheet->hSS, lCol, lRow);
          // RFW - 6/19/04 - 14622
          _fmemset(&result, '\0', sizeof(CALC_VALUE));
          lpfnCustProc(lpSheet->hSS, lpszFuncName, &result,
                       lpStack->lpElem + lpStack->nElemCnt - nArgs, nArgs);
          CustFuncUnlockText(lpExpr->u.CustFunc.hFunc);
          }
          #else
          lpfnCustProc(&result, lpStack->lpElem + lpStack->nElemCnt - nArgs, nArgs);
          #endif
          // Note: It was never intended for custom functions to return reference values but
          // we need this code to prevent breaking some existing applications (see bug #15342).
          if( CALC_VALUE_STATUS_OK == result.Status )
          {
            if( CALC_VALUE_TYPE_CELL == result.Type )
            {
              #if defined(_WIN64) || defined(_IA64)
              result.Val.ValCell.nSheet--;
              result.Val.ValCell.u.lInfo = (LONG_PTR)lpSheet->lpBook->lpfnGetSheetFromIndex(lpSheet->lpBook->hBook, result.Val.ValCell.nSheet);
              #elif SS_V70
              result.Val.ValCell.nSheet--;
              result.Val.ValCell.u.lInfo = (long)lpSheet->lpBook->lpfnGetSheetFromIndex(lpSheet->lpBook->hBook, result.Val.ValCell.nSheet);
              #else
              result.Val.ValCell.u.lInfo = (long)lpSheet;
              #endif
            }
            else if( CALC_VALUE_TYPE_RANGE == result.Type )
            {
			  #if defined(_WIN64) || defined(_IA64)
              result.Val.ValRange.nSheet--;
              result.Val.ValRange.u.lInfo = (LONG_PTR)lpSheet->lpBook->lpfnGetSheetFromIndex(lpSheet->lpBook->hBook, result.Val.ValRange.nSheet);
              #elif SS_V70
              result.Val.ValRange.nSheet--;
              result.Val.ValRange.u.lInfo = (long)lpSheet->lpBook->lpfnGetSheetFromIndex(lpSheet->lpBook->hBook, result.Val.ValRange.nSheet);
              #else
              result.Val.ValRange.u.lInfo = (long)lpSheet;
              #endif
            }
          }
          ValStackRemove(lpStack, nArgs);
          ValStackPush(lpStack, &result);
        }
        else
        {
          bReturn = FALSE;
          ValSetError(&val);
          ValStackRemove(lpStack, nArgs);
          ValStackPush(lpStack, &val);
        }
        break;
      case CALC_EXPR_OPERATOR:
        nArgs = OperIsBinary(lpExpr->u.Oper.nId) ? 2 : 1;
        if( lpExpr->u.Oper.nId != CALC_OPER_RANGE )
        {
          EvalCellRefs(lpStack->lpElem + lpStack->nElemCnt - nArgs, nArgs);
          EvalRangeRefs(lpStack->lpElem + lpStack->nElemCnt - nArgs, nArgs);
          EvalDdeRefs(lpStack->lpElem + lpStack->nElemCnt - nArgs, nArgs);
        }
        if( lpfnProc = OperGetProc(lpExpr->u.Oper.nId) )
        {
          bReturn = lpfnProc(&result, lpStack->lpElem + lpStack->nElemCnt - nArgs, nArgs);
          ValStackRemove(lpStack, nArgs);
          ValStackPush(lpStack, &result);
        }
        else
        {
          bReturn = FALSE;
          ValSetError(&val);
          ValStackRemove(lpStack, nArgs);
          ValStackPush(lpStack, &val);
        }
        break;
      #if defined(SS_DDE)
      case CALC_EXPR_DDE:
        ValSetDde(&val, lpExpr->u.Dde.hDde,
                  lpExpr->u.Dde.lItemCol, lpExpr->u.Dde.lItemRow);
        ValStackPush(lpStack, &val);
        break;
      #endif
      case CALC_EXPR_SEPARATOR:
      case CALC_EXPR_BEGIN:
        break;
      default:
        bReturn = FALSE;
        ValSetError(lpResult);
        break;
    }
  }
  ValStackPop(lpStack, lpResult);
  return bReturn;
}

//--------------------------------------------------------------------
//
//  The ExprEval() function evaluates the given expression.  All
//  relative addresses are evaluated relative to lCol and lRow.  If
//  successful, the function returns TRUE and *lpResult holds the
//  result of the evaluation.  Otherwise, the function returns FALSE.
//

BOOL ExprEval(CALC_HANDLE hExpr, LPCALC_SHEET lpSheet, long lCol, long lRow,
              LPCALC_VALUE lpResult)
{
  CALC_VALUESTACK stack;
  LPCALC_EXPRINFO lpExpr;
  BOOL bReturn = FALSE;

  hExpr = ExprCreateRef(hExpr);
  if( hExpr && (lpExpr = Expr_MemLock(hExpr)) )
  {
    if( ValStackInit(&stack) )
    {
      if( bReturn = ExprFullEval(lpExpr, lpSheet, lCol, lRow, lpResult, &stack) )
      {
        EvalCellRefs(lpResult, 1);
        EvalRangeRefs(lpResult, 1);
        EvalDdeRefs(lpResult, 1);
      }
      ValStackFree(&stack);
    }
    Expr_MemUnlock(hExpr);
  }
  ExprDestroy(hExpr);
  return bReturn;
}

//--------------------------------------------------------------------
//
//  The ExprSetLinkToCell() function adds or removes data links
//  between the expression and the cell.  This allows the given cell
//  to be updated any time any of the cells or names in the expression
//  are updated.
//
//  Note: need to add stack overflow protection !!!
//

BOOL ExprSetLinkToCell(CALC_HANDLE hExpr, CALC_HANDLE hDependCell, BOOL bSetNameLinks, BOOL bLink)
#if defined(SS_V70)
{
  if (bLink)
    Expr_CellStartListening(hExpr, hDependCell, bSetNameLinks);
  else
    Expr_CellStopListening(hExpr, hDependCell, bSetNameLinks);
  return TRUE;
}
#else
{
  LPCALC_EXPRINFO lpExpr;
  LPCALC_VALUE lpStack;
  CALC_VALUESTACK stack;
  CALC_HANDLE hCell;
  BOOL bEmptyRange;
  BOOL bReturn = FALSE;
  long i, j, k;
  
  if( hExpr && (lpExpr = Expr_MemLock(hExpr)) )
  {
    long lDependCol = Cell_GetCol(hDependCell);
    long lDependRow = Cell_GetRow(hDependCell);
    ValStackInit(&stack);
    ExprSemiEval(lpExpr, Cell_GetSheet(hDependCell), lDependCol, lDependRow, &stack);
    if( bSetNameLinks )
    {
      for( ; lpExpr->nType != CALC_EXPR_END; lpExpr++ )
      {
        if( lpExpr->nType == CALC_EXPR_NAME )
          NameSetLinkToCell(lpExpr->u.CustName.hName, lDependCol, lDependRow,
                            bLink);
        #if defined(SS_DDE)
        else if( lpExpr->nType == CALC_EXPR_DDE )
          DdeSetLinkToCell(lpExpr->u.Dde.hDde, lDependCol, lDependRow, bLink);
        #endif
      }
    }
    for( k = 0; k < stack.nElemCnt; k++ )
    {
      lpStack = stack.lpElem + k;
      if( CALC_VALUE_STATUS_OK == lpStack->Status)
      {
        if( CALC_VALUE_TYPE_CELL == lpStack->Type )
        {
          hCell = CellLookupAlloc((LPCALC_SHEET)lpStack->Val.ValCell.u.lInfo, lpStack->Val.ValCell.Col, lpStack->Val.ValCell.Row);
          CellSetLinkToCell(hCell, hDependCell, bLink);
        }
        else if (CALC_VALUE_TYPE_RANGE == lpStack->Type )
        {
          bEmptyRange = TRUE;
          for( i = lpStack->Val.ValRange.Col1; i <= lpStack->Val.ValRange.Col2; i++ )
            for( j = lpStack->Val.ValRange.Row1; j <= lpStack->Val.ValRange.Row2; j++ )
            {
              hCell = CellLookupAlloc((LPCALC_SHEET)lpStack->Val.ValRange.u.lInfo, i, j);
              CellSetLinkToCell(hCell, hDependCell, bLink);
              bEmptyRange = FALSE;
            }
          if( bEmptyRange )
          {
            hCell = CellLookupAlloc((LPCALC_SHEET)lpStack->Val.ValRange.u.lInfo, lpStack->Val.ValRange.Col1, lpStack->Val.ValRange.Row1);
            CellSetLinkToCell(hCell, hDependCell, bLink);
            hCell = CellLookupAlloc((LPCALC_SHEET)lpStack->Val.ValRange.u.lInfo, lpStack->Val.ValRange.Col2, lpStack->Val.ValRange.Row2);
            CellSetLinkToCell(hCell, hDependCell, bLink);
          }
        }
      }
    }
    bReturn = TRUE;
    ValStackFree(&stack);
    Expr_MemUnlock(hExpr);
  }
  return bReturn;
}
#endif

//--------------------------------------------------------------------
//
//  The ExprSetLinkToCol() function adds or removes data links
//  between the expression and the column.  This allows the given
//  column to be updated any time any of the cells or names in the
//  expression are updated.
//

BOOL ExprSetLinkToCol(CALC_HANDLE hExpr, CALC_HANDLE hDependCol, BOOL bSetNameLinks, BOOL bLink)
#if defined(SS_V70)
{
  if (bLink)
    Expr_ColStartListening(hExpr, hDependCol, bSetNameLinks);
  else
    Expr_ColStopListening(hExpr, hDependCol, bSetNameLinks);
  return TRUE;
}
#else
{
  LPCALC_EXPRINFO lpExpr;
  LPCALC_VALUE lpStack;
  CALC_VALUESTACK stack;
  CALC_HANDLE hCell;
  CALC_HANDLE hCol;
  BOOL bEmptyRange;
  BOOL bReturn = FALSE;
  long i, j, k;
  
  if( hExpr && (lpExpr = Expr_MemLock(hExpr)) )
  {
    long lDependCol = Col_GetCol(hDependCol);
    ValStackInit(&stack);
    ExprSemiEval(lpExpr, Col_GetSheet(hDependCol), lDependCol, CALC_ALLROWS, &stack);
    if( bSetNameLinks )
    {
      for( ; lpExpr->nType != CALC_EXPR_END; lpExpr++ )
      {
        if (lpExpr->nType == CALC_EXPR_NAME )
          NameSetLinkToCol(lpExpr->u.CustName.hName, lDependCol, bLink);
        #if defined(SS_DDE)
        else if( lpExpr->nType == CALC_EXPR_DDE )
          DdeSetLinkToCol(lpExpr->u.Dde.hDde, lDependCol, bLink);
        #endif
      }
    }
    for( k = 0; k < stack.nElemCnt; k++ )
    {
      lpStack = stack.lpElem + k;
      if( CALC_VALUE_STATUS_OK == lpStack->Status )
      {
        if( CALC_VALUE_TYPE_CELL == lpStack->Type )
        {
          if( CoordIsRel(lpStack->Val.ValCell.Row) )
          {
            lpStack->Val.ValCell.Row = CoordMakeAbs(lpStack->Val.ValCell.Row, 0);
            hCol = ColLookupAlloc((LPCALC_SHEET)lpStack->Val.ValCell.u.lInfo, lpStack->Val.ValCell.Col);
            ColSetLinkToCol(hCol, hDependCol, -lpStack->Val.ValCell.Row, bLink);
          }
          else
          {
            hCell = CellLookupAlloc((LPCALC_SHEET)lpStack->Val.ValCell.u.lInfo, lpStack->Val.ValCell.Col, lpStack->Val.ValCell.Row);
            CellSetLinkToCol(hCell, hDependCol, bLink);
          }
        }
        else if( CALC_VALUE_TYPE_RANGE == lpStack->Type )
        {
          if( CoordIsRel(lpStack->Val.ValRange.Row1) &&
              CoordIsRel(lpStack->Val.ValRange.Row2) )
          {
            bEmptyRange = TRUE;
            lpStack->Val.ValRange.Row1 = CoordMakeAbs(lpStack->Val.ValRange.Row1, 0);
            lpStack->Val.ValRange.Row2 = CoordMakeAbs(lpStack->Val.ValRange.Row2, 0);
            for( i = lpStack->Val.ValRange.Col1; i <= lpStack->Val.ValRange.Col2; i++ )
              for( j = lpStack->Val.ValRange.Row1; j <= lpStack->Val.ValRange.Row2; j++ )
              {
                hCol = ColLookupAlloc((LPCALC_SHEET)lpStack->Val.ValRange.u.lInfo, i);
                ColSetLinkToCol(hCol, hDependCol, -j, bLink);
                bEmptyRange = FALSE;
              }
            if( bEmptyRange )
            {
              hCol = ColLookupAlloc((LPCALC_SHEET)lpStack->Val.ValRange.u.lInfo, lpStack->Val.ValRange.Col1);
              ColSetLinkToCol(hCol, hDependCol, -lpStack->Val.ValRange.Row1, bLink);
              hCol = ColLookupAlloc((LPCALC_SHEET)lpStack->Val.ValRange.u.lInfo, lpStack->Val.ValRange.Col2);
              ColSetLinkToCol(hCol, hDependCol, -lpStack->Val.ValRange.Row2, bLink);
            }
          }
          else if( CoordIsAbs(lpStack->Val.ValRange.Row1) &&
                   CoordIsAbs(lpStack->Val.ValRange.Row2) )
          {
            bEmptyRange = TRUE;
            for( i = lpStack->Val.ValRange.Col1; i <= lpStack->Val.ValRange.Col2; i++ )
              for( j = lpStack->Val.ValRange.Row1; j <= lpStack->Val.ValRange.Row2; j++ )
              {
                hCell = CellLookupAlloc((LPCALC_SHEET)lpStack->Val.ValRange.u.lInfo, i, j);
                CellSetLinkToCol(hCell, hDependCol, bLink);
                bEmptyRange = FALSE;
              }
            if( bEmptyRange )
            {
              hCell = CellLookupAlloc((LPCALC_SHEET)lpStack->Val.ValRange.u.lInfo, lpStack->Val.ValRange.Col1, lpStack->Val.ValRange.Row1);
              CellSetLinkToCol(hCell, hDependCol, bLink);
              hCell = CellLookupAlloc((LPCALC_SHEET)lpStack->Val.ValRange.u.lInfo, lpStack->Val.ValRange.Col2, lpStack->Val.ValRange.Row2);
              CellSetLinkToCol(hCell, hDependCol, bLink);
            }
          }
        }
      }
    }
    bReturn = TRUE;
    ValStackFree(&stack);
    Expr_MemUnlock(hExpr);
  }
  return bReturn;
}
#endif

//--------------------------------------------------------------------
//
//  The ExprSetLinkToRow() function adds or removes data links
//  between the expression and the row.  This allows the given row to
//  be updated any time any of the cells or names in the expression
//  are updated.
//

BOOL ExprSetLinkToRow(CALC_HANDLE hExpr, CALC_HANDLE hDependRow, BOOL bSetNameLinks, BOOL bLink)
#if defined(SS_V70)
{
  if (bLink)
    Expr_RowStartListening(hExpr, hDependRow, bSetNameLinks);
  else
    Expr_RowStopListening(hExpr, hDependRow, bSetNameLinks);
  return TRUE;
}
#else
{
  LPCALC_EXPRINFO lpExpr;
  LPCALC_VALUE lpStack;
  CALC_VALUESTACK stack;
  CALC_HANDLE hCell;
  CALC_HANDLE hRow;
  BOOL bEmptyRange;
  BOOL bReturn = FALSE;
  long i, j, k;
  
  if( hExpr && (lpExpr = Expr_MemLock(hExpr)) )
  {
    long lDependRow = Row_GetRow(hDependRow);
    ValStackInit(&stack);
    ExprSemiEval(lpExpr, Row_GetSheet(hDependRow), CALC_ALLCOLS, lDependRow, &stack);
    if( bSetNameLinks )
    {
      for( ; lpExpr->nType != CALC_EXPR_END; lpExpr++ )
      {
        if (lpExpr->nType == CALC_EXPR_NAME )
          NameSetLinkToRow(lpExpr->u.CustName.hName, lDependRow, bLink);
        #if defined(SS_DDE)
        else if( lpExpr->nType == CALC_EXPR_DDE )
          DdeSetLinkToRow(lpExpr->u.Dde.hDde, lDependRow, bLink);
        #endif
      }
    }
    for( k = 0; k < stack.nElemCnt; k++ )
    {
      lpStack = stack.lpElem + k;
      if( CALC_VALUE_STATUS_OK == lpStack->Status )
      {
        if( CALC_VALUE_TYPE_CELL == lpStack->Type )
        {
          if( CoordIsRel(lpStack->Val.ValCell.Col) )
          {
            lpStack->Val.ValCell.Col = CoordMakeAbs(lpStack->Val.ValCell.Col, 0);
            hRow = RowLookupAlloc((LPCALC_SHEET)lpStack->Val.ValCell.u.lInfo, lpStack->Val.ValCell.Row);
            RowSetLinkToRow(hRow, hDependRow, -lpStack->Val.ValCell.Col, bLink);
          }
          else
          {
            hCell = CellLookupAlloc((LPCALC_SHEET)lpStack->Val.ValCell.u.lInfo, lpStack->Val.ValCell.Col, lpStack->Val.ValCell.Row);
            CellSetLinkToRow(hCell, hDependRow, bLink);
          }
        }
        else if (CALC_VALUE_TYPE_RANGE == lpStack->Type )
        {
          if( CoordIsRel(lpStack->Val.ValRange.Col1) &&
              CoordIsRel(lpStack->Val.ValRange.Col2) )
          {
            bEmptyRange = TRUE;
            lpStack->Val.ValRange.Col1 = CoordMakeAbs(lpStack->Val.ValRange.Col1, 0);
            lpStack->Val.ValRange.Col2 = CoordMakeAbs(lpStack->Val.ValRange.Col2, 0);
            for( i = lpStack->Val.ValRange.Col1; i <= lpStack->Val.ValRange.Col2; i++ )
              for( j = lpStack->Val.ValRange.Row1; j <= lpStack->Val.ValRange.Row2; j++ )
              {
                hRow = RowLookupAlloc((LPCALC_SHEET)lpStack->Val.ValRange.u.lInfo, j);
                RowSetLinkToRow(hRow, hDependRow, -i, bLink);
                bEmptyRange = FALSE;
              }
            if( bEmptyRange )
            {
              hRow = RowLookupAlloc((LPCALC_SHEET)lpStack->Val.ValRange.u.lInfo, lpStack->Val.ValRange.Row1);
              RowSetLinkToRow(hRow, hDependRow, -lpStack->Val.ValRange.Col1, bLink);
              hRow = RowLookupAlloc((LPCALC_SHEET)lpStack->Val.ValRange.u.lInfo, lpStack->Val.ValRange.Row2);
              RowSetLinkToRow(hRow, hDependRow, -lpStack->Val.ValRange.Col2, bLink);
            }
          }
          else if( CoordIsAbs(lpStack->Val.ValRange.Col1) &&
                   CoordIsAbs(lpStack->Val.ValRange.Col2) )
          {
            bEmptyRange = TRUE;
            for( i = lpStack->Val.ValRange.Col1; i <= lpStack->Val.ValRange.Col2; i++ )
              for( j = lpStack->Val.ValRange.Row1; j <= lpStack->Val.ValRange.Row2; j++ )
              {
                hCell = CellLookupAlloc((LPCALC_SHEET)lpStack->Val.ValRange.u.lInfo, i, j);
                CellSetLinkToRow(hCell, hDependRow, bLink);
                bEmptyRange = FALSE;
              }
            if( bEmptyRange )
            {
              hCell = CellLookupAlloc((LPCALC_SHEET)lpStack->Val.ValRange.u.lInfo, lpStack->Val.ValRange.Col1, lpStack->Val.ValRange.Row1);
              CellSetLinkToRow(hCell, hDependRow, bLink);
              hCell = CellLookupAlloc((LPCALC_SHEET)lpStack->Val.ValRange.u.lInfo, lpStack->Val.ValRange.Col2, lpStack->Val.ValRange.Row2);
              CellSetLinkToRow(hCell, hDependRow, bLink);
            }
          }
        }
      }
    }
    bReturn = TRUE;
    ValStackFree(&stack);
    Expr_MemUnlock(hExpr);
  }
  return bReturn;
}
#endif

//--------------------------------------------------------------------
//
//  The ExprSetLinkToName() function adds or removes a data link
//  between the expression and the given name.  This allows the
//  given name to be updated anytime a name in the expression
//  updated.
//

BOOL ExprSetLinkToName(CALC_HANDLE hExpr, CALC_HANDLE hName, BOOL bLink)
#if defined(SS_V70)
{
  if (bLink)
    Expr_NameStartListening(hExpr, hName);
  else
    Expr_NameStopListening(hExpr, hName);
  return TRUE;
}
#else
{
  LPCALC_EXPRINFO lpExpr;
  BOOL bReturn = FALSE;
  
  if( hExpr && (lpExpr = Expr_MemLock(hExpr)) )
  {
    for( ; lpExpr->nType != CALC_EXPR_END; lpExpr++ )
    {
      if( lpExpr->nType == CALC_EXPR_NAME )
        NameSetLinkToName(lpExpr->u.CustName.hName, hName, bLink);
      #if defined(SS_DDE)
      else if( lpExpr->nType == CALC_EXPR_DDE )
        DdeSetLinkToName(lpExpr->u.Dde.hDde, hName, bLink);
      #endif
    }
    bReturn = TRUE;
    Expr_MemUnlock(hExpr);
  }
  return bReturn;
}
#endif

//--------------------------------------------------------------------
//
//  The InSrcRange() function determines if the given cell is
//  within the source range specified in LPCALC_ADJUST structure.
//

static BOOL InSrcRange(LPCALC_SHEET lpSheet, long lCol, long lRow, LPCALC_ADJUST lpAdj)
{
  if( lpAdj->lCurCol != CALC_ALLCOLS )
    lCol = CoordMakeAbs(lCol, lpAdj->lCurCol);
  if( lpAdj->lCurRow != CALC_ALLROWS )
    lRow = CoordMakeAbs(lRow, lpAdj->lCurRow);
  return (lpSheet == lpAdj->lpSrcSheet) &&
         ((CALC_ALLCOLS == lpAdj->lSrcCol1) ||
          (CoordIsAbs(lCol) &&
           lpAdj->lSrcCol1 <= lCol && lCol <= lpAdj->lSrcCol2)) &&
         ((CALC_ALLROWS == lpAdj->lSrcRow2) ||
          (CoordIsAbs(lRow) &&
           lpAdj->lSrcRow1 <= lRow && lRow <= lpAdj->lSrcRow2));
}

//--------------------------------------------------------------------
//
//  The InDestRange() function determines if the given cell is
//  within the destination range specified in LPCALC_ADJUST
//  structure.
//

static BOOL InDestRange(LPCALC_SHEET lpSheet, long lCol, long lRow, LPCALC_ADJUST lpAdj)
{
  if( lpAdj->lCurCol != CALC_ALLCOLS )
    lCol = CoordMakeAbs(lCol, lpAdj->lCurCol);
  if( lpAdj->lCurRow != CALC_ALLROWS )
    lRow = CoordMakeAbs(lRow, lpAdj->lCurRow);
  return (lpSheet == lpAdj->lpDestSheet) &&
         ((CALC_ALLCOLS == lpAdj->lSrcCol1) ||
          (CoordIsAbs(lCol) &&
           lpAdj->lDestCol1 <= lCol && lCol <= lpAdj->lDestCol2)) &&
         ((CALC_ALLROWS == lpAdj->lSrcRow2) ||
          (CoordIsAbs(lRow) &&
           lpAdj->lDestRow1 <= lRow && lRow <= lpAdj->lDestRow2));
}

//--------------------------------------------------------------------
//
//  The ExtModifyOnMove() function modifies a cell coordinate which
//  is part of an expression which is outside of the range being
//  moved.
//

static void ExtModifyOnMove(LPCALC_EXPRINFO lpExpr, LPCALC_ADJUST lpAdj)
{
  if (CALC_EXPR_CELL == lpExpr->nType)
  {
    LPCALC_SHEET lpSheet = lpAdj->lpCurSheet;
    long lCol = lpExpr->u.Cell.lCol;
    long lRow = lpExpr->u.Cell.lRow;
    if (InSrcRange(lpSheet, lCol, lRow, lpAdj))
    {
      lCol = CoordPlus(lCol, lpAdj->lOffsetCol);
      lRow = CoordPlus(lRow, lpAdj->lOffsetRow);
      lpExpr->u.Cell.lCol = lCol;
      lpExpr->u.Cell.lRow = lRow;
    }
    else if (InDestRange(lpSheet, lCol, lRow, lpAdj))
    {
      lpExpr->nType = CALC_EXPR_ERROR;
      lpExpr->u.Error.nId = CALC_ERROR_REF;
    }
  }
  else if (CALC_EXPR_RANGE == lpExpr->nType)
  {
    LPCALC_SHEET lpSheet = lpAdj->lpCurSheet;
    long lCol1 = lpExpr->u.Range.lCol1;
    long lRow1 = lpExpr->u.Range.lRow1;
    long lCol2 = lpExpr->u.Range.lCol2;
    long lRow2 = lpExpr->u.Range.lRow2;
    if (InSrcRange(lpSheet, lCol1, lRow1, lpAdj) && InSrcRange(lpSheet, lCol2, lRow2, lpAdj))
    {
      lCol1 = CoordPlus(lCol1, lpAdj->lOffsetCol);
      lRow1 = CoordPlus(lRow1, lpAdj->lOffsetRow);
      lCol2 = CoordPlus(lCol2, lpAdj->lOffsetCol);
      lRow2 = CoordPlus(lRow2, lpAdj->lOffsetRow);
      lpExpr->u.Range.lCol1 = lCol1;
      lpExpr->u.Range.lRow1 = lRow1;
      lpExpr->u.Range.lCol2 = lCol2;
      lpExpr->u.Range.lRow2 = lRow2;
    }
    else if (InDestRange(lpSheet, lCol1, lRow1, lpAdj) && InDestRange(lpSheet, lCol2, lRow2, lpAdj))
    {
      lpExpr->nType = CALC_EXPR_ERROR;
      lpExpr->u.Error.nId = CALC_ERROR_REF;
    }
  }
  else if (CALC_EXPR_EXTERNALCELL == lpExpr->nType)
  {
    LPCALC_SHEET lpSheet = lpExpr->u.ExternalCell.lpSheet;
    long lCol = lpExpr->u.ExternalCell.lCol;
    long lRow = lpExpr->u.ExternalCell.lRow;
    if (InSrcRange(lpSheet, lCol, lRow, lpAdj))
    {
      lCol = CoordPlus(lCol, lpAdj->lOffsetCol);
      lRow = CoordPlus(lRow, lpAdj->lOffsetRow);
      lpExpr->u.ExternalCell.lCol = lCol;
      lpExpr->u.ExternalCell.lRow = lRow;
    }
    else if (InDestRange(lpSheet, lCol, lRow, lpAdj))
    {
      lpExpr->nType = CALC_EXPR_ERROR;
      lpExpr->u.Error.nId = CALC_ERROR_REF;
    }
  }
  else if (CALC_EXPR_EXTERNALRANGE == lpExpr->nType)
  {
    LPCALC_SHEET lpSheet = lpExpr->u.ExternalRange.lpSheet;
    long lCol1 = lpExpr->u.ExternalRange.lCol1;
    long lRow1 = lpExpr->u.ExternalRange.lRow1;
    long lCol2 = lpExpr->u.ExternalRange.lCol2;
    long lRow2 = lpExpr->u.ExternalRange.lRow2;
    if (InSrcRange(lpSheet, lCol1, lRow1, lpAdj) && InSrcRange(lpSheet, lCol2, lRow2, lpAdj))
    {
      lCol1 = CoordPlus(lCol1, lpAdj->lOffsetCol);
      lRow1 = CoordPlus(lRow1, lpAdj->lOffsetRow);
      lCol2 = CoordPlus(lCol2, lpAdj->lOffsetCol);
      lRow2 = CoordPlus(lRow2, lpAdj->lOffsetRow);
      lpExpr->u.ExternalRange.lCol1 = lCol1;
      lpExpr->u.ExternalRange.lRow1 = lRow1;
      lpExpr->u.ExternalRange.lCol2 = lCol2;
      lpExpr->u.ExternalRange.lRow2 = lRow2;
    }
    else if (InDestRange(lpSheet, lCol1, lRow1, lpAdj) && InDestRange(lpSheet, lCol2, lRow2, lpAdj))
    {
      lpExpr->nType = CALC_EXPR_ERROR;
      lpExpr->u.Error.nId = CALC_ERROR_REF;
    }
  }
}

//--------------------------------------------------------------------
//
//  The ExprExtModifyOnMove() function modifies an expression which
//  is outside of the range being moved.
//

CALC_HANDLE ExprExtModifyOnMove(CALC_HANDLE hExpr, LPCALC_ADJUST lpAdj)
{
  LPCALC_EXPRINFO lpExpr;
  CALC_HANDLE hRet;

  hRet = ExprCreateCopy(hExpr);
  ExprDestroy(hExpr);
  if (hRet && (lpExpr = Expr_MemLock(hRet)))
  {
    for (; lpExpr->nType != CALC_EXPR_END; lpExpr++)
    {
      if (CALC_EXPR_CELL == lpExpr->nType ||
          CALC_EXPR_RANGE == lpExpr->nType ||
          CALC_EXPR_EXTERNALCELL == lpExpr->nType ||
          CALC_EXPR_EXTERNALRANGE == lpExpr->nType)
      {
        ExtModifyOnMove(lpExpr, lpAdj);
      }
    }
    Expr_MemUnlock(hRet);
  }
  return hRet;
}

//--------------------------------------------------------------------
//
//  The IntModifyOnMove() function modifies a cell coordinate which
//  is part of an expression which is inside of the range being
//  moved.
//

static void IntModifyOnMove(LPCALC_EXPRINFO lpExpr, LPCALC_ADJUST lpAdj)
{
  if (CALC_EXPR_CELL == lpExpr->nType)
  {
    LPCALC_SHEET lpSheet = lpAdj->lpCurSheet;
    long lCol = lpExpr->u.Cell.lCol;
    long lRow = lpExpr->u.Cell.lRow;
    if (InSrcRange(lpSheet, lCol, lRow, lpAdj))
    {
      if (CoordIsAbs(lCol))
        lCol = CoordPlus(lCol, lpAdj->lOffsetCol);
      if (CoordIsAbs(lRow))
        lRow = CoordPlus(lRow, lpAdj->lOffsetRow);
      lpExpr->u.Cell.lCol = lCol;
      lpExpr->u.Cell.lRow = lRow;
    }
    else if (InDestRange(lpSheet, lCol, lRow, lpAdj))
    {
      lpExpr->nType = CALC_EXPR_ERROR;
      lpExpr->u.Error.nId = CALC_ERROR_REF;
    }
    else
    {
      if (CoordIsRel(lCol))
        lCol = CoordPlus(lCol, -lpAdj->lOffsetCol);
      if (CoordIsRel(lRow))
        lRow = CoordPlus(lRow, -lpAdj->lOffsetRow);
      lpExpr->u.Cell.lCol = lCol;
      lpExpr->u.Cell.lRow = lRow;
    }
  }
  else if (CALC_EXPR_RANGE == lpExpr->nType)
  {
    LPCALC_SHEET lpSheet = lpAdj->lpCurSheet;
    long lCol1 = lpExpr->u.Range.lCol1;
    long lRow1 = lpExpr->u.Range.lRow1;
    long lCol2 = lpExpr->u.Range.lCol2;
    long lRow2 = lpExpr->u.Range.lRow2;
    if (InSrcRange(lpSheet, lCol1, lRow1, lpAdj) && InSrcRange(lpSheet, lCol2, lRow2, lpAdj))
    {
      if (CoordIsAbs(lCol1))
        lCol1 = CoordPlus(lCol1, lpAdj->lOffsetCol);
      if (CoordIsAbs(lRow1))
        lRow1 = CoordPlus(lRow1, lpAdj->lOffsetRow);
      if (CoordIsAbs(lCol2))
        lCol2 = CoordPlus(lCol2, lpAdj->lOffsetCol);
      if (CoordIsAbs(lRow2))
        lRow2 = CoordPlus(lRow2, lpAdj->lOffsetRow);
      lpExpr->u.Range.lCol1 = lCol1;
      lpExpr->u.Range.lRow1 = lRow1;
      lpExpr->u.Range.lCol2 = lCol2;
      lpExpr->u.Range.lRow2 = lRow2;
    }
    else if (InDestRange(lpSheet, lCol1, lRow1, lpAdj) && InDestRange(lpSheet, lCol2, lRow2, lpAdj))
    {
      lpExpr->nType = CALC_EXPR_ERROR;
      lpExpr->u.Error.nId = CALC_ERROR_REF;
    }
    else
    {
      if (CoordIsRel(lCol1))
        lCol1 = CoordPlus(lCol1, -lpAdj->lOffsetCol);
      if (CoordIsRel(lRow1))
        lRow1 = CoordPlus(lRow1, -lpAdj->lOffsetRow);
      if (CoordIsRel(lCol2))
        lCol2 = CoordPlus(lCol2, -lpAdj->lOffsetCol);
      if (CoordIsRel(lRow2))
        lRow2 = CoordPlus(lRow2, -lpAdj->lOffsetRow);
      lpExpr->u.Range.lCol1 = lCol1;
      lpExpr->u.Range.lRow1 = lRow1;
      lpExpr->u.Range.lCol2 = lCol2;
      lpExpr->u.Range.lRow2 = lRow2;
    }
  }
  else if (CALC_EXPR_EXTERNALCELL == lpExpr->nType)
  {
    LPCALC_SHEET lpSheet = lpExpr->u.ExternalCell.lpSheet;
    long lCol = lpExpr->u.ExternalCell.lCol;
    long lRow = lpExpr->u.ExternalCell.lRow;
    if (InSrcRange(lpSheet, lCol, lRow, lpAdj))
    {
      if (CoordIsAbs(lCol))
        lCol = CoordPlus(lCol, lpAdj->lOffsetCol);
      if (CoordIsAbs(lRow))
        lRow = CoordPlus(lRow, lpAdj->lOffsetRow);
      lpExpr->u.ExternalCell.lCol = lCol;
      lpExpr->u.ExternalCell.lRow = lRow;
    }
    else if (InDestRange(lpSheet, lCol, lRow, lpAdj))
    {
      lpExpr->nType = CALC_EXPR_ERROR;
      lpExpr->u.Error.nId = CALC_ERROR_REF;
    }
    else
    {
      if (CoordIsRel(lCol))
        lCol = CoordPlus(lCol, -lpAdj->lOffsetCol);
      if (CoordIsRel(lRow))
        lRow = CoordPlus(lRow, -lpAdj->lOffsetRow);
      lpExpr->u.ExternalCell.lCol = lCol;
      lpExpr->u.ExternalCell.lRow = lRow;
    }
  }
  else if (CALC_EXPR_EXTERNALRANGE == lpExpr->nType)
  {
    LPCALC_SHEET lpSheet = lpExpr->u.ExternalRange.lpSheet;
    long lCol1 = lpExpr->u.ExternalRange.lCol1;
    long lRow1 = lpExpr->u.ExternalRange.lRow1;
    long lCol2 = lpExpr->u.ExternalRange.lCol2;
    long lRow2 = lpExpr->u.ExternalRange.lRow2;
    if (InSrcRange(lpSheet, lCol1, lRow1, lpAdj) && InSrcRange(lpSheet, lCol2, lRow2, lpAdj))
    {
      if (CoordIsAbs(lCol1))
        lCol1 = CoordPlus(lCol1, lpAdj->lOffsetCol);
      if (CoordIsAbs(lRow1))
        lRow1 = CoordPlus(lRow1, lpAdj->lOffsetRow);
      if (CoordIsAbs(lCol2))
        lCol2 = CoordPlus(lCol2, lpAdj->lOffsetCol);
      if (CoordIsAbs(lRow2))
        lRow2 = CoordPlus(lRow2, lpAdj->lOffsetRow);
      lpExpr->u.ExternalRange.lCol1 = lCol1;
      lpExpr->u.ExternalRange.lRow1 = lRow1;
      lpExpr->u.ExternalRange.lCol2 = lCol2;
      lpExpr->u.ExternalRange.lRow2 = lRow2;
    }
    else if (InDestRange(lpSheet, lCol1, lRow1, lpAdj) && InDestRange(lpSheet, lCol2, lRow2, lpAdj))
    {
      lpExpr->nType = CALC_EXPR_ERROR;
      lpExpr->u.Error.nId = CALC_ERROR_REF;
    }
    else
    {
      if (CoordIsRel(lCol1))
        lCol1 = CoordPlus(lCol1, -lpAdj->lOffsetCol);
      if (CoordIsRel(lRow1))
        lRow1 = CoordPlus(lRow1, -lpAdj->lOffsetRow);
      if (CoordIsRel(lCol2))
        lCol2 = CoordPlus(lCol2, -lpAdj->lOffsetCol);
      if (CoordIsRel(lRow2))
        lRow2 = CoordPlus(lRow2, -lpAdj->lOffsetRow);
      lpExpr->u.ExternalRange.lCol1 = lCol1;
      lpExpr->u.ExternalRange.lRow1 = lRow1;
      lpExpr->u.ExternalRange.lCol2 = lCol2;
      lpExpr->u.ExternalRange.lRow2 = lRow2;
    }
  }
}


//--------------------------------------------------------------------
//
//  The ExprIntModifyOnMove() function modifies an expressions which
//  is inside of the range being moved.
//

CALC_HANDLE ExprIntModifyOnMove(CALC_HANDLE hExpr, LPCALC_ADJUST lpAdj)
{
  LPCALC_EXPRINFO lpExpr;
  CALC_HANDLE hRet;

  hRet = ExprCreateCopy(hExpr);
  ExprDestroy(hExpr);
  if (hRet && (lpExpr = Expr_MemLock(hRet)))
  {
    for (; lpExpr->nType != CALC_EXPR_END; lpExpr++)
    {
      if (CALC_EXPR_CELL == lpExpr->nType ||
          CALC_EXPR_RANGE == lpExpr->nType ||
          CALC_EXPR_EXTERNALCELL == lpExpr->nType ||
          CALC_EXPR_EXTERNALRANGE == lpExpr->nType)
      {
        IntModifyOnMove(lpExpr, lpAdj);
      }
    }
    Expr_MemUnlock(hRet);
  }
  return hRet;
}

//--------------------------------------------------------------------
//
//  The ExtModifyOnSwap() function modifies a cell coordinate which
//  is part of an expression which is outside of the ranges being
//  swapped.
//

static void ExtModifyOnSwap(LPCALC_EXPRINFO lpExpr, LPCALC_ADJUST lpAdj)
{
  if (CALC_EXPR_CELL == lpExpr->nType)
  {
    LPCALC_SHEET lpSheet = lpAdj->lpCurSheet;
    long lCol = lpExpr->u.Cell.lCol;
    long lRow = lpExpr->u.Cell.lRow;
    if (InSrcRange(lpSheet, lCol, lRow, lpAdj))
    {
      lCol = CoordPlus(lCol, lpAdj->lOffsetCol);
      lRow = CoordPlus(lRow, lpAdj->lOffsetRow);
    }
    else if (InDestRange(lpSheet, lCol, lRow, lpAdj))
    {
      lCol = CoordPlus(lCol, -lpAdj->lOffsetCol);
      lRow = CoordPlus(lRow, -lpAdj->lOffsetRow);
    }
    lpExpr->u.Cell.lCol = lCol;
    lpExpr->u.Cell.lRow = lRow;
  }
  else if (CALC_EXPR_RANGE == lpExpr->nType)
  {
    LPCALC_SHEET lpSheet = lpAdj->lpCurSheet;
    long lCol1 = lpExpr->u.Range.lCol1;
    long lRow1 = lpExpr->u.Range.lRow1;
    long lCol2 = lpExpr->u.Range.lCol2;
    long lRow2 = lpExpr->u.Range.lRow2;
    if (InSrcRange(lpSheet, lCol1, lRow1, lpAdj) && InSrcRange(lpSheet, lCol2, lRow2, lpAdj))
    {
      lCol1 = CoordPlus(lCol1, lpAdj->lOffsetCol);
      lRow1 = CoordPlus(lRow1, lpAdj->lOffsetRow);
      lCol2 = CoordPlus(lCol2, lpAdj->lOffsetCol);
      lRow2 = CoordPlus(lRow2, lpAdj->lOffsetRow);
    }
    else if (InDestRange(lpSheet, lCol1, lRow1, lpAdj) && InDestRange(lpSheet, lCol2, lRow2, lpAdj))
    {
      lCol1 = CoordPlus(lCol1, -lpAdj->lOffsetCol);
      lRow1 = CoordPlus(lRow1, -lpAdj->lOffsetRow);
      lCol2 = CoordPlus(lCol2, -lpAdj->lOffsetCol);
      lRow2 = CoordPlus(lRow2, -lpAdj->lOffsetRow);
    }
    lpExpr->u.Range.lCol1 = lCol1;
    lpExpr->u.Range.lRow1 = lRow1;
    lpExpr->u.Range.lCol2 = lCol2;
    lpExpr->u.Range.lRow2 = lRow2;
  }
  else if (CALC_EXPR_EXTERNALCELL == lpExpr->nType)
  {
    LPCALC_SHEET lpSheet = lpExpr->u.ExternalCell.lpSheet;
    long lCol = lpExpr->u.ExternalCell.lCol;
    long lRow = lpExpr->u.ExternalCell.lRow;
    if (InSrcRange(lpSheet, lCol, lRow, lpAdj))
    {
      lCol = CoordPlus(lCol, lpAdj->lOffsetCol);
      lRow = CoordPlus(lRow, lpAdj->lOffsetRow);
    }
    else if (InDestRange(lpSheet, lCol, lRow, lpAdj))
    {
      lCol = CoordPlus(lCol, -lpAdj->lOffsetCol);
      lRow = CoordPlus(lRow, -lpAdj->lOffsetRow);
    }
    lpExpr->u.ExternalCell.lCol = lCol;
    lpExpr->u.ExternalCell.lRow = lRow;
  }
  else if (CALC_EXPR_EXTERNALRANGE == lpExpr->nType)
  {
    LPCALC_SHEET lpSheet = lpExpr->u.ExternalRange.lpSheet;
    long lCol1 = lpExpr->u.ExternalRange.lCol1;
    long lRow1 = lpExpr->u.ExternalRange.lRow1;
    long lCol2 = lpExpr->u.ExternalRange.lCol2;
    long lRow2 = lpExpr->u.ExternalRange.lRow2;
    if (InSrcRange(lpSheet, lCol1, lRow1, lpAdj) && InSrcRange(lpSheet, lCol2, lRow2, lpAdj))
    {
      lCol1 = CoordPlus(lCol1, lpAdj->lOffsetCol);
      lRow1 = CoordPlus(lRow1, lpAdj->lOffsetRow);
      lCol2 = CoordPlus(lCol2, lpAdj->lOffsetCol);
      lRow2 = CoordPlus(lRow2, lpAdj->lOffsetRow);
    }
    else if (InDestRange(lpSheet, lCol1, lRow1, lpAdj) && InDestRange(lpSheet, lCol2, lRow2, lpAdj))
    {
      lCol1 = CoordPlus(lCol1, -lpAdj->lOffsetCol);
      lRow1 = CoordPlus(lRow1, -lpAdj->lOffsetRow);
      lCol2 = CoordPlus(lCol2, -lpAdj->lOffsetCol);
      lRow2 = CoordPlus(lRow2, -lpAdj->lOffsetRow);
    }
    lpExpr->u.ExternalRange.lCol1 = lCol1;
    lpExpr->u.ExternalRange.lRow1 = lRow1;
    lpExpr->u.ExternalRange.lCol2 = lCol2;
    lpExpr->u.ExternalRange.lRow2 = lRow2;
  }
}

//--------------------------------------------------------------------
//
//  The ExprExtModifyOnSwap() function modifies an expression which
//  is outside of the ranges being swapped.
//

CALC_HANDLE ExprExtModifyOnSwap(CALC_HANDLE hExpr, LPCALC_ADJUST lpAdj)
{
  LPCALC_EXPRINFO lpExpr;
  CALC_HANDLE hRet;

  hRet = ExprCreateCopy(hExpr);
  ExprDestroy(hExpr);
  if (hRet && (lpExpr = Expr_MemLock(hRet)))
  {
    for (; lpExpr->nType != CALC_EXPR_END; lpExpr++)
    {
      if (CALC_EXPR_CELL == lpExpr->nType ||
          CALC_EXPR_RANGE == lpExpr->nType ||
          CALC_EXPR_EXTERNALCELL == lpExpr->nType ||
          CALC_EXPR_EXTERNALRANGE == lpExpr->nType)
      {
        ExtModifyOnSwap(lpExpr, lpAdj);
      }
    }
    Expr_MemUnlock(hRet);
  }
  return hRet;
}

//--------------------------------------------------------------------
//
//  The IntModifyOnSwap() function modifies a cell coordinate which
//  is part of an expression which is inside of the ranges being
//  swapped.
//

static void IntModifyOnSwap(LPCALC_EXPRINFO lpExpr, LPCALC_ADJUST lpAdj)
{
  if (CALC_EXPR_CELL == lpExpr->nType)
  {
    LPCALC_SHEET lpSheet = lpAdj->lpCurSheet;
    long lCol = lpExpr->u.Cell.lCol;
    long lRow = lpExpr->u.Cell.lRow;
    if (InSrcRange(lpSheet, lCol, lRow, lpAdj))
    {
      if (CoordIsAbs(lCol))
        lCol = CoordPlus(lCol, lpAdj->lOffsetCol);
      if (CoordIsAbs(lRow))
        lRow = CoordPlus(lRow, lpAdj->lOffsetRow);
    }
    else if (InDestRange(lpSheet, lCol, lRow, lpAdj))
    {
      lCol = CoordPlus(lCol, -lpAdj->lOffsetCol);
      lRow = CoordPlus(lRow, -lpAdj->lOffsetRow);
      if (CoordIsRel(lCol))
        lCol = CoordPlus(lCol, -lpAdj->lOffsetCol);
      if (CoordIsRel(lRow))
        lRow = CoordPlus(lRow, -lpAdj->lOffsetRow);
    }
    else
    {
      if (CoordIsRel(lCol))
        lCol = CoordPlus(lCol, -lpAdj->lOffsetCol);
      if (CoordIsRel(lRow))
        lRow = CoordPlus(lRow, -lpAdj->lOffsetRow);
    }
    lpExpr->u.Cell.lCol = lCol;
    lpExpr->u.Cell.lRow = lRow;
  }
  else if (CALC_EXPR_RANGE == lpExpr->nType)
  {
    LPCALC_SHEET lpSheet = lpAdj->lpCurSheet;
    long lCol1 = lpExpr->u.Range.lCol1;
    long lRow1 = lpExpr->u.Range.lRow1;
    long lCol2 = lpExpr->u.Range.lCol2;
    long lRow2 = lpExpr->u.Range.lRow2;
    if (InSrcRange(lpSheet, lCol1, lRow1, lpAdj) && InSrcRange(lpSheet, lCol2, lRow2, lpAdj))
    {
      if (CoordIsAbs(lCol1))
        lCol1 = CoordPlus(lCol1, lpAdj->lOffsetCol);
      if (CoordIsAbs(lRow1))
        lRow1 = CoordPlus(lRow1, lpAdj->lOffsetRow);
      if (CoordIsAbs(lCol2))
        lCol2 = CoordPlus(lCol2, lpAdj->lOffsetCol);
      if (CoordIsAbs(lRow2))
        lRow2 = CoordPlus(lRow2, lpAdj->lOffsetRow);
    }
    else if (InDestRange(lpSheet, lCol1, lRow1, lpAdj) && InDestRange(lpSheet, lCol2, lRow2, lpAdj))
    {
      lCol1 = CoordPlus(lCol1, -lpAdj->lOffsetCol);
      lRow1 = CoordPlus(lRow1, -lpAdj->lOffsetRow);
      lCol2 = CoordPlus(lCol2, -lpAdj->lOffsetCol);
      lRow2 = CoordPlus(lRow2, -lpAdj->lOffsetRow);
      if (CoordIsRel(lCol1))
        lCol1 = CoordPlus(lCol1, -lpAdj->lOffsetCol);
      if (CoordIsRel(lRow1))
        lRow1 = CoordPlus(lRow1, -lpAdj->lOffsetRow);
      if (CoordIsRel(lCol2))
        lCol2 = CoordPlus(lCol2, -lpAdj->lOffsetCol);
      if (CoordIsRel(lRow2))
        lRow2 = CoordPlus(lRow2, -lpAdj->lOffsetRow);
    }
    else
    {
      if (CoordIsRel(lCol1))
        lCol1 = CoordPlus(lCol1, -lpAdj->lOffsetCol);
      if (CoordIsRel(lRow1))
        lRow1 = CoordPlus(lRow1, -lpAdj->lOffsetRow);
      if (CoordIsRel(lCol2))
        lCol2 = CoordPlus(lCol2, -lpAdj->lOffsetCol);
      if (CoordIsRel(lRow2))
        lRow2 = CoordPlus(lRow2, -lpAdj->lOffsetRow);
    }
    lpExpr->u.Range.lCol1 = lCol1;
    lpExpr->u.Range.lRow1 = lRow1;
    lpExpr->u.Range.lCol2 = lCol2;
    lpExpr->u.Range.lRow2 = lRow2;
  }
  else if (CALC_EXPR_EXTERNALCELL == lpExpr->nType)
  {
    LPCALC_SHEET lpSheet = lpExpr->u.ExternalCell.lpSheet;
    long lCol = lpExpr->u.ExternalCell.lCol;
    long lRow = lpExpr->u.ExternalCell.lRow;
    if (InSrcRange(lpSheet, lCol, lRow, lpAdj))
    {
      if (CoordIsAbs(lCol))
        lCol = CoordPlus(lCol, lpAdj->lOffsetCol);
      if (CoordIsAbs(lRow))
        lRow = CoordPlus(lRow, lpAdj->lOffsetRow);
    }
    else if (InDestRange(lpSheet, lCol, lRow, lpAdj))
    {
      lCol = CoordPlus(lCol, -lpAdj->lOffsetCol);
      lRow = CoordPlus(lRow, -lpAdj->lOffsetRow);
      if (CoordIsRel(lCol))
        lCol = CoordPlus(lCol, -lpAdj->lOffsetCol);
      if (CoordIsRel(lRow))
        lRow = CoordPlus(lRow, -lpAdj->lOffsetRow);
    }
    else
    {
      if (CoordIsRel(lCol))
        lCol = CoordPlus(lCol, -lpAdj->lOffsetCol);
      if (CoordIsRel(lRow))
        lRow = CoordPlus(lRow, -lpAdj->lOffsetRow);
    }
    lpExpr->u.ExternalCell.lCol = lCol;
    lpExpr->u.ExternalCell.lRow = lRow;
  }
  else if (CALC_EXPR_EXTERNALRANGE == lpExpr->nType)
  {
    LPCALC_SHEET lpSheet = lpExpr->u.ExternalRange.lpSheet;
    long lCol1 = lpExpr->u.ExternalRange.lCol1;
    long lRow1 = lpExpr->u.ExternalRange.lRow1;
    long lCol2 = lpExpr->u.ExternalRange.lCol2;
    long lRow2 = lpExpr->u.ExternalRange.lRow2;
    if (InSrcRange(lpSheet, lCol1, lRow1, lpAdj) && InSrcRange(lpSheet, lCol2, lRow2, lpAdj))
    {
      if (CoordIsAbs(lCol1))
        lCol1 = CoordPlus(lCol1, lpAdj->lOffsetCol);
      if (CoordIsAbs(lRow1))
        lRow1 = CoordPlus(lRow1, lpAdj->lOffsetRow);
      if (CoordIsAbs(lCol2))
        lCol2 = CoordPlus(lCol2, lpAdj->lOffsetCol);
      if (CoordIsAbs(lRow2))
        lRow2 = CoordPlus(lRow2, lpAdj->lOffsetRow);
    }
    else if (InDestRange(lpSheet, lCol1, lRow1, lpAdj) && InDestRange(lpSheet, lCol2, lRow2, lpAdj))
    {
      lCol1 = CoordPlus(lCol1, -lpAdj->lOffsetCol);
      lRow1 = CoordPlus(lRow1, -lpAdj->lOffsetRow);
      lCol2 = CoordPlus(lCol2, -lpAdj->lOffsetCol);
      lRow2 = CoordPlus(lRow2, -lpAdj->lOffsetRow);
      if (CoordIsRel(lCol1))
        lCol1 = CoordPlus(lCol1, -lpAdj->lOffsetCol);
      if (CoordIsRel(lRow1))
        lRow1 = CoordPlus(lRow1, -lpAdj->lOffsetRow);
      if (CoordIsRel(lCol2))
        lCol2 = CoordPlus(lCol2, -lpAdj->lOffsetCol);
      if (CoordIsRel(lRow2))
        lRow2 = CoordPlus(lRow2, -lpAdj->lOffsetRow);
    }
    else
    {
      if (CoordIsRel(lCol1))
        lCol1 = CoordPlus(lCol1, -lpAdj->lOffsetCol);
      if (CoordIsRel(lRow1))
        lRow1 = CoordPlus(lRow1, -lpAdj->lOffsetRow);
      if (CoordIsRel(lCol2))
        lCol2 = CoordPlus(lCol2, -lpAdj->lOffsetCol);
      if (CoordIsRel(lRow2))
        lRow2 = CoordPlus(lRow2, -lpAdj->lOffsetRow);
    }
    lpExpr->u.ExternalRange.lCol1 = lCol1;
    lpExpr->u.ExternalRange.lRow1 = lRow1;
    lpExpr->u.ExternalRange.lCol2 = lCol2;
    lpExpr->u.ExternalRange.lRow2 = lRow2;
  }
}

//--------------------------------------------------------------------
//
//  The ExprIntModifyOnSwap() function modifies an expression which
//  is outside of the ranges being swapped.
//

CALC_HANDLE ExprIntModifyOnSwap(CALC_HANDLE hExpr, LPCALC_ADJUST lpAdj)
{
  LPCALC_EXPRINFO lpExpr;
  CALC_HANDLE hRet;

  hRet = ExprCreateCopy(hExpr);
  ExprDestroy(hExpr);
  if (hRet && (lpExpr = Expr_MemLock(hRet)))
  {
    for (; lpExpr->nType != CALC_EXPR_END; lpExpr++)
    {
      if (CALC_EXPR_CELL == lpExpr->nType ||
          CALC_EXPR_RANGE == lpExpr->nType ||
          CALC_EXPR_EXTERNALCELL == lpExpr->nType ||
          CALC_EXPR_EXTERNALRANGE == lpExpr->nType)
      {
        IntModifyOnSwap(lpExpr, lpAdj);
      }
    }
    Expr_MemUnlock(hRet);
  }
  return hRet;
}

//--------------------------------------------------------------------
//
//  The ExtModifyOnInsCols() function modifies a cell coordinate which
//  is part of an expression which is outside of the range being
//  moved.
//

static void ExtModifyOnInsCols(LPCALC_EXPRINFO lpExpr, LPCALC_ADJUST lpAdj)
{
  if (CALC_EXPR_CELL == lpExpr->nType)
  {
    LPCALC_SHEET lpSheet = lpAdj->lpCurSheet;
    long lCol = lpExpr->u.Cell.lCol;
    if (InSrcRange(lpSheet, lCol, CALC_ALLROWS, lpAdj))
    {
      lCol = CoordPlus(lCol, lpAdj->lOffsetCol);
      lpExpr->u.Cell.lCol = lCol;
    }
  }
  else if (CALC_EXPR_RANGE == lpExpr->nType)
  {
    LPCALC_SHEET lpSheet = lpAdj->lpCurSheet;
    long lCol1 = lpExpr->u.Range.lCol1;
    long lCol2 = lpExpr->u.Range.lCol2;
    if (InSrcRange(lpSheet, lCol1, CALC_ALLROWS, lpAdj))
    {
      lCol1 = CoordPlus(lCol1, lpAdj->lOffsetCol);
      lpExpr->u.Range.lCol1 = lCol1;
    }
    if (InSrcRange(lpSheet, lCol2, CALC_ALLROWS, lpAdj))
    {
      lCol2 = CoordPlus(lCol2, lpAdj->lOffsetCol);
      lpExpr->u.Range.lCol2 = lCol2;
    }
  }
  else if (CALC_EXPR_EXTERNALCELL == lpExpr->nType)
  {
    LPCALC_SHEET lpSheet = lpExpr->u.ExternalCell.lpSheet;
    long lCol = lpExpr->u.ExternalCell.lCol;
    if (InSrcRange(lpSheet, lCol, CALC_ALLROWS, lpAdj))
    {
      lCol = CoordPlus(lCol, lpAdj->lOffsetCol);
      lpExpr->u.ExternalCell.lCol = lCol;
    }
  }
  else if (CALC_EXPR_EXTERNALRANGE == lpExpr->nType)
  {
    LPCALC_SHEET lpSheet = lpExpr->u.ExternalRange.lpSheet;
    long lCol1 = lpExpr->u.ExternalRange.lCol1;
    long lCol2 = lpExpr->u.ExternalRange.lCol2;
    if (InSrcRange(lpSheet, lCol1, CALC_ALLROWS, lpAdj))
    {
      lCol1 = CoordPlus(lCol1, lpAdj->lOffsetCol);
      lpExpr->u.ExternalRange.lCol1 = lCol1;
    }
    if (InSrcRange(lpSheet, lCol2, CALC_ALLROWS, lpAdj))
    {
      lCol2 = CoordPlus(lCol2, lpAdj->lOffsetCol);
      lpExpr->u.ExternalRange.lCol2 = lCol2;
    }
  }
}

//--------------------------------------------------------------------
//
//  The ExprExtModifyOnInsCols() function modifies an expression which
//  is outside of the range being moved.
//

CALC_HANDLE ExprExtModifyOnInsCols(CALC_HANDLE hExpr, LPCALC_ADJUST lpAdj)
{
  LPCALC_EXPRINFO lpExpr;
  CALC_HANDLE hRet;

  hRet = ExprCreateCopy(hExpr);
  ExprDestroy(hExpr);
  if (hRet && (lpExpr = Expr_MemLock(hRet)))
  {
    for (; lpExpr->nType != CALC_EXPR_END; lpExpr++)
    {
      if (CALC_EXPR_CELL == lpExpr->nType ||
          CALC_EXPR_RANGE == lpExpr->nType ||
          CALC_EXPR_EXTERNALCELL == lpExpr->nType ||
          CALC_EXPR_EXTERNALRANGE == lpExpr->nType)
      {
        ExtModifyOnInsCols(lpExpr, lpAdj);
      }
    }
    Expr_MemUnlock(hRet);
  }
  return hRet;
}

//--------------------------------------------------------------------
//
//  The IntModifyOnInsCols() function modifies a cell coordinate which
//  is part of an expression which is inside of the range being
//  moved.
//

static void IntModifyOnInsCols(LPCALC_EXPRINFO lpExpr, LPCALC_ADJUST lpAdj)
{
  if (CALC_EXPR_CELL == lpExpr->nType)
  {
    LPCALC_SHEET lpSheet = lpAdj->lpCurSheet;
    long lCol = lpExpr->u.Cell.lCol;
    if (InSrcRange(lpSheet, lCol, CALC_ALLROWS, lpAdj))
    {
      if (CoordIsAbs(lCol))
        lCol = CoordPlus(lCol, lpAdj->lOffsetCol);
      lpExpr->u.Cell.lCol = lCol;
    }
    else
    {
      if (CoordIsRel(lCol))
        lCol = CoordPlus(lCol, -lpAdj->lOffsetCol);
      lpExpr->u.Cell.lCol = lCol;
    }
  }
  else if (CALC_EXPR_RANGE == lpExpr->nType)
  {
    LPCALC_SHEET lpSheet = lpAdj->lpCurSheet;
    long lCol1 = lpExpr->u.Range.lCol1;
    long lCol2 = lpExpr->u.Range.lCol2;
    if (InSrcRange(lpSheet, lCol1, CALC_ALLROWS, lpAdj))
    {
      if (CoordIsAbs(lCol1))
        lCol1 = CoordPlus(lCol1, lpAdj->lOffsetCol);
      lpExpr->u.Range.lCol1 = lCol1;
    }
    else
    {
      if (CoordIsRel(lCol1))
        lCol1 = CoordPlus(lCol1, -lpAdj->lOffsetCol);
      lpExpr->u.Range.lCol1 = lCol1;
    }
    if (InSrcRange(lpSheet, lCol2, CALC_ALLROWS, lpAdj))
    {
      if (CoordIsAbs(lCol2))
        lCol2 = CoordPlus(lCol2, lpAdj->lOffsetCol);
      lpExpr->u.Range.lCol2 = lCol2;
    }
    else
    {
      if (CoordIsRel(lCol2))
        lCol2 = CoordPlus(lCol2, -lpAdj->lOffsetCol);
      lpExpr->u.Range.lCol2 = lCol2;
    }
  }
  else if (CALC_EXPR_EXTERNALCELL == lpExpr->nType)
  {
    LPCALC_SHEET lpSheet = lpExpr->u.ExternalCell.lpSheet;
    long lCol = lpExpr->u.ExternalCell.lCol;
    if (InSrcRange(lpSheet, lCol, CALC_ALLROWS, lpAdj))
    {
      if (CoordIsAbs(lCol))
        lCol = CoordPlus(lCol, lpAdj->lOffsetCol);
      lpExpr->u.ExternalCell.lCol = lCol;
    }
    else
    {
      if (CoordIsRel(lCol))
        lCol = CoordPlus(lCol, -lpAdj->lOffsetCol);
      lpExpr->u.ExternalCell.lCol = lCol;
    }
  }
  else if (CALC_EXPR_EXTERNALRANGE == lpExpr->nType)
  {
    LPCALC_SHEET lpSheet = lpExpr->u.ExternalRange.lpSheet;
    long lCol1 = lpExpr->u.ExternalRange.lCol1;
    long lCol2 = lpExpr->u.ExternalRange.lCol2;
    if (InSrcRange(lpSheet, lCol1, CALC_ALLROWS, lpAdj))
    {
      if (CoordIsAbs(lCol1))
        lCol1 = CoordPlus(lCol1, lpAdj->lOffsetCol);
      lpExpr->u.ExternalRange.lCol1 = lCol1;
    }
    else
    {
      if (CoordIsRel(lCol1))
        lCol1 = CoordPlus(lCol1, -lpAdj->lOffsetCol);
      lpExpr->u.ExternalRange.lCol1 = lCol1;
    }
    if (InSrcRange(lpSheet, lCol2, CALC_ALLROWS, lpAdj))
    {
      if (CoordIsAbs(lCol2))
        lCol2 = CoordPlus(lCol2, lpAdj->lOffsetCol);
      lpExpr->u.ExternalRange.lCol2 = lCol2;
    }
    else
    {
      if (CoordIsRel(lCol2))
        lCol2 = CoordPlus(lCol2, -lpAdj->lOffsetCol);
      lpExpr->u.ExternalRange.lCol2 = lCol2;
    }
  }
}


//--------------------------------------------------------------------
//
//  The ExprIntModifyOnInsCols() function modifies an expressions which
//  is inside of the range being moved.
//

CALC_HANDLE ExprIntModifyOnInsCols(CALC_HANDLE hExpr, LPCALC_ADJUST lpAdj)
{
  LPCALC_EXPRINFO lpExpr;
  CALC_HANDLE hRet;

  hRet = ExprCreateCopy(hExpr);
  ExprDestroy(hExpr);
  if (hRet && (lpExpr = Expr_MemLock(hRet)))
  {
    for (; lpExpr->nType != CALC_EXPR_END; lpExpr++)
    {
      if (CALC_EXPR_CELL == lpExpr->nType ||
          CALC_EXPR_RANGE == lpExpr->nType ||
          CALC_EXPR_EXTERNALCELL == lpExpr->nType ||
          CALC_EXPR_EXTERNALRANGE == lpExpr->nType)
      {
        IntModifyOnInsCols(lpExpr, lpAdj);
      }
    }
    Expr_MemUnlock(hRet);
  }
  return hRet;
}

//--------------------------------------------------------------------
//
//  The ExtModifyOnInsRows() function modifies a cell coordinate which
//  is part of an expression which is outside of the range being
//  moved.
//

static void ExtModifyOnInsRows(LPCALC_EXPRINFO lpExpr, LPCALC_ADJUST lpAdj)
{
  if (CALC_EXPR_CELL == lpExpr->nType)
  {
    LPCALC_SHEET lpSheet = lpAdj->lpCurSheet;
    long lRow = lpExpr->u.Cell.lRow;
    if (InSrcRange(lpSheet, CALC_ALLCOLS, lRow, lpAdj))
    {
      lRow = CoordPlus(lRow, lpAdj->lOffsetRow);
      lpExpr->u.Cell.lRow = lRow;
    }
  }
  else if (CALC_EXPR_RANGE == lpExpr->nType)
  {
    LPCALC_SHEET lpSheet = lpAdj->lpCurSheet;
    long lRow1 = lpExpr->u.Range.lRow1;
    long lRow2 = lpExpr->u.Range.lRow2;
    if (InSrcRange(lpSheet, CALC_ALLCOLS, lRow1, lpAdj))
    {
      lRow1 = CoordPlus(lRow1, lpAdj->lOffsetRow);
      lpExpr->u.Range.lRow1 = lRow1;
    }
    if (InSrcRange(lpSheet, CALC_ALLCOLS, lRow2, lpAdj))
    {
      lRow2 = CoordPlus(lRow2, lpAdj->lOffsetRow);
      lpExpr->u.Range.lRow2 = lRow2;
    }
  }
  else if (CALC_EXPR_EXTERNALCELL == lpExpr->nType)
  {
    LPCALC_SHEET lpSheet = lpExpr->u.ExternalCell.lpSheet;
    long lRow = lpExpr->u.ExternalCell.lRow;
    if (InSrcRange(lpSheet, CALC_ALLCOLS, lRow, lpAdj))
    {
      lRow = CoordPlus(lRow, lpAdj->lOffsetRow);
      lpExpr->u.ExternalCell.lRow = lRow;
    }
  }
  else if (CALC_EXPR_EXTERNALRANGE == lpExpr->nType)
  {
    LPCALC_SHEET lpSheet = lpExpr->u.ExternalRange.lpSheet;
    long lRow1 = lpExpr->u.ExternalRange.lRow1;
    long lRow2 = lpExpr->u.ExternalRange.lRow2;
    if (InSrcRange(lpSheet, CALC_ALLCOLS, lRow1, lpAdj))
    {
      lRow1 = CoordPlus(lRow1, lpAdj->lOffsetRow);
      lpExpr->u.ExternalRange.lRow1 = lRow1;
    }
    if (InSrcRange(lpSheet, CALC_ALLCOLS, lRow2, lpAdj))
    {
      lRow2 = CoordPlus(lRow2, lpAdj->lOffsetRow);
      lpExpr->u.ExternalRange.lRow2 = lRow2;
    }
  }
}

//--------------------------------------------------------------------
//
//  The ExprExtModifyOnInsRows() function modifies an expression which
//  is outside of the range being moved.
//

CALC_HANDLE ExprExtModifyOnInsRows(CALC_HANDLE hExpr, LPCALC_ADJUST lpAdj)
{
  LPCALC_EXPRINFO lpExpr;
  CALC_HANDLE hRet;

  hRet = ExprCreateCopy(hExpr);
  ExprDestroy(hExpr);
  if (hRet && (lpExpr = Expr_MemLock(hRet)))
  {
    for (; lpExpr->nType != CALC_EXPR_END; lpExpr++)
    {
      if (CALC_EXPR_CELL == lpExpr->nType ||
          CALC_EXPR_RANGE == lpExpr->nType ||
          CALC_EXPR_EXTERNALCELL == lpExpr->nType ||
          CALC_EXPR_EXTERNALRANGE == lpExpr->nType)
      {
        ExtModifyOnInsRows(lpExpr, lpAdj);
      }
    }
    Expr_MemUnlock(hRet);
  }
  return hRet;
}

//--------------------------------------------------------------------
//
//  The IntModifyOnInsRows() function modifies a cell coordinate which
//  is part of an expression which is inside of the range being
//  moved.
//

static void IntModifyOnInsRows(LPCALC_EXPRINFO lpExpr, LPCALC_ADJUST lpAdj)
{
  if (CALC_EXPR_CELL == lpExpr->nType)
  {
    LPCALC_SHEET lpSheet = lpAdj->lpCurSheet;
    long lRow = lpExpr->u.Cell.lRow;
    if (InSrcRange(lpSheet, CALC_ALLCOLS, lRow, lpAdj))
    {
      if (CoordIsAbs(lRow))
        lRow = CoordPlus(lRow, lpAdj->lOffsetRow);
      lpExpr->u.Cell.lRow = lRow;
    }
    else
    {
      if (CoordIsRel(lRow))
        lRow = CoordPlus(lRow, -lpAdj->lOffsetRow);
      lpExpr->u.Cell.lRow = lRow;
    }
  }
  else if (CALC_EXPR_RANGE == lpExpr->nType)
  {
    LPCALC_SHEET lpSheet = lpAdj->lpCurSheet;
    long lRow1 = lpExpr->u.Range.lRow1;
    long lRow2 = lpExpr->u.Range.lRow2;
    if (InSrcRange(lpSheet, CALC_ALLCOLS, lRow1, lpAdj))
    {
      if (CoordIsAbs(lRow1))
        lRow1 = CoordPlus(lRow1, lpAdj->lOffsetRow);
      lpExpr->u.Range.lRow1 = lRow1;
    }
    else
    {
      if (CoordIsRel(lRow1))
        lRow1 = CoordPlus(lRow1, -lpAdj->lOffsetRow);
      lpExpr->u.Range.lRow1 = lRow1;
    }
    if (InSrcRange(lpSheet, CALC_ALLCOLS, lRow2, lpAdj))
    {
      if (CoordIsAbs(lRow2))
        lRow2 = CoordPlus(lRow2, lpAdj->lOffsetRow);
      lpExpr->u.Range.lRow2 = lRow2;
    }
    else
    {
      if (CoordIsRel(lRow2))
        lRow2 = CoordPlus(lRow2, -lpAdj->lOffsetRow);
      lpExpr->u.Range.lRow2 = lRow2;
    }
  }
  else if (CALC_EXPR_EXTERNALCELL == lpExpr->nType)
  {
    LPCALC_SHEET lpSheet = lpExpr->u.ExternalCell.lpSheet;
    long lRow = lpExpr->u.ExternalCell.lRow;
    if (InSrcRange(lpSheet, CALC_ALLCOLS, lRow, lpAdj))
    {
      if (CoordIsAbs(lRow))
        lRow = CoordPlus(lRow, lpAdj->lOffsetRow);
      lpExpr->u.ExternalCell.lRow = lRow;
    }
    else
    {
      if (CoordIsRel(lRow))
        lRow = CoordPlus(lRow, -lpAdj->lOffsetRow);
      lpExpr->u.ExternalCell.lRow = lRow;
    }
  }
  else if (CALC_EXPR_EXTERNALRANGE == lpExpr->nType)
  {
    LPCALC_SHEET lpSheet = lpExpr->u.ExternalRange.lpSheet;
    long lRow1 = lpExpr->u.ExternalRange.lRow1;
    long lRow2 = lpExpr->u.ExternalRange.lRow2;
    if (InSrcRange(lpSheet, CALC_ALLCOLS, lRow1, lpAdj))
    {
      if (CoordIsAbs(lRow1))
        lRow1 = CoordPlus(lRow1, lpAdj->lOffsetRow);
      lpExpr->u.ExternalRange.lRow1 = lRow1;
    }
    else
    {
      if (CoordIsRel(lRow1))
        lRow1 = CoordPlus(lRow1, -lpAdj->lOffsetRow);
      lpExpr->u.ExternalRange.lRow1 = lRow1;
    }
    if (InSrcRange(lpSheet, CALC_ALLCOLS, lRow2, lpAdj))
    {
      if (CoordIsAbs(lRow2))
        lRow2 = CoordPlus(lRow2, lpAdj->lOffsetRow);
      lpExpr->u.ExternalRange.lRow2 = lRow2;
    }
    else
    {
      if (CoordIsRel(lRow2))
        lRow2 = CoordPlus(lRow2, -lpAdj->lOffsetRow);
      lpExpr->u.ExternalRange.lRow2 = lRow2;
    }
  }
}


//--------------------------------------------------------------------
//
//  The ExprIntModifyOnInsRows() function modifies an expressions which
//  is inside of the range being moved.
//

CALC_HANDLE ExprIntModifyOnInsRows(CALC_HANDLE hExpr, LPCALC_ADJUST lpAdj)
{
  LPCALC_EXPRINFO lpExpr;
  CALC_HANDLE hRet;

  hRet = ExprCreateCopy(hExpr);
  ExprDestroy(hExpr);
  if (hRet && (lpExpr = Expr_MemLock(hRet)))
  {
    for (; lpExpr->nType != CALC_EXPR_END; lpExpr++)
    {
      if (CALC_EXPR_CELL == lpExpr->nType ||
          CALC_EXPR_RANGE == lpExpr->nType ||
          CALC_EXPR_EXTERNALCELL == lpExpr->nType ||
          CALC_EXPR_EXTERNALRANGE == lpExpr->nType)
      {
        IntModifyOnInsRows(lpExpr, lpAdj);
      }
    }
    Expr_MemUnlock(hRet);
  }
  return hRet;
}

//--------------------------------------------------------------------
//
//  The ExtModifyOnDelCols() function modifies a cell coordinate which
//  is part of an expression which is outside of the range being
//  moved.
//

static void ExtModifyOnDelCols(LPCALC_EXPRINFO lpExpr, LPCALC_ADJUST lpAdj)
{
  if (CALC_EXPR_CELL == lpExpr->nType)
  {
    LPCALC_SHEET lpSheet = lpAdj->lpCurSheet;
    long lCol = lpExpr->u.Cell.lCol;
    if (InSrcRange(lpSheet, lCol, CALC_ALLROWS, lpAdj))
    {
      lCol = CoordPlus(lCol, lpAdj->lOffsetCol);
      lpExpr->u.Cell.lCol = lCol;
    }
    else if (InDestRange(lpSheet, lCol, CALC_ALLROWS, lpAdj))
    {
      lpExpr->nType = CALC_EXPR_ERROR;
      lpExpr->u.Error.nId = CALC_ERROR_REF;
    }
  }
  else if (CALC_EXPR_RANGE == lpExpr->nType)
  {
    LPCALC_SHEET lpSheet = lpAdj->lpCurSheet;
    long lCol1 = lpExpr->u.Range.lCol1;
    long lCol2 = lpExpr->u.Range.lCol2;
    if (InSrcRange(lpSheet, lCol1, CALC_ALLROWS, lpAdj))
    {
      if (InSrcRange(lpSheet, lCol2, CALC_ALLROWS, lpAdj))
      {
        lCol1 = CoordPlus(lCol1, lpAdj->lOffsetCol);
        lCol2 = CoordPlus(lCol2, lpAdj->lOffsetCol);
        lpExpr->u.Range.lCol1 = lCol1;
        lpExpr->u.Range.lCol2 = lCol2;
      }
      else if (InDestRange(lpSheet, lCol2, CALC_ALLROWS, lpAdj))
      {
        lpExpr->nType = CALC_EXPR_ERROR;
        lpExpr->u.Error.nId = CALC_ERROR_REF;
      }
      else
      {
        lCol1 = CoordPlus(lCol1, lpAdj->lOffsetCol);
        lpExpr->u.Range.lCol1 = lCol1;
      }
    }
    else if (InDestRange(lpSheet, lCol1, CALC_ALLROWS, lpAdj))
    {
      if (InSrcRange(lpSheet, lCol2, CALC_ALLROWS, lpAdj))
      {
        lpExpr->nType = CALC_EXPR_ERROR;
        lpExpr->u.Error.nId = CALC_ERROR_REF;
      }
      else if (InDestRange(lpSheet, lCol2, CALC_ALLROWS, lpAdj))
      {
        lpExpr->nType = CALC_EXPR_ERROR;
        lpExpr->u.Error.nId = CALC_ERROR_REF;
      }
      else
      {
        lpExpr->nType = CALC_EXPR_ERROR;
        lpExpr->u.Error.nId = CALC_ERROR_REF;
      }
    }
    else
    {
      if (InSrcRange(lpSheet, lCol2, CALC_ALLROWS, lpAdj))
      {
        lCol2 = CoordPlus(lCol2, lpAdj->lOffsetCol);
        lpExpr->u.Range.lCol2 = lCol2;
      }
      else if (InDestRange(lpSheet, lCol2, CALC_ALLROWS, lpAdj))
      {
        lpExpr->nType = CALC_EXPR_ERROR;
        lpExpr->u.Error.nId = CALC_ERROR_REF;
      }
    }
  }
  else if (CALC_EXPR_EXTERNALCELL == lpExpr->nType)
  {
    LPCALC_SHEET lpSheet = lpExpr->u.ExternalCell.lpSheet;
    long lCol = lpExpr->u.ExternalCell.lCol;
    if (InSrcRange(lpSheet, lCol, CALC_ALLROWS, lpAdj))
    {
      lCol = CoordPlus(lCol, lpAdj->lOffsetCol);
      lpExpr->u.ExternalCell.lCol = lCol;
    }
    else if (InDestRange(lpSheet, lCol, CALC_ALLROWS, lpAdj))
    {
      lpExpr->nType = CALC_EXPR_ERROR;
      lpExpr->u.Error.nId = CALC_ERROR_REF;
    }
  }
  else if (CALC_EXPR_EXTERNALRANGE == lpExpr->nType)
  {
    LPCALC_SHEET lpSheet = lpExpr->u.ExternalRange.lpSheet;
    long lCol1 = lpExpr->u.ExternalRange.lCol1;
    long lCol2 = lpExpr->u.ExternalRange.lCol2;
    if (InSrcRange(lpSheet, lCol1, CALC_ALLROWS, lpAdj))
    {
      if (InSrcRange(lpSheet, lCol2, CALC_ALLROWS, lpAdj))
      {
        lCol1 = CoordPlus(lCol1, lpAdj->lOffsetCol);
        lCol2 = CoordPlus(lCol2, lpAdj->lOffsetCol);
        lpExpr->u.ExternalRange.lCol1 = lCol1;
        lpExpr->u.ExternalRange.lCol2 = lCol2;
      }
      else if(InDestRange(lpSheet, lCol2, CALC_ALLROWS, lpAdj))
      {
        lpExpr->nType = CALC_EXPR_ERROR;
        lpExpr->u.Error.nId = CALC_ERROR_REF;
      }
      else
      {
        lCol1 = CoordPlus(lCol1, lpAdj->lOffsetCol);
        lpExpr->u.ExternalRange.lCol1 = lCol1;
      }
    }
    else if (InDestRange(lpSheet, lCol1, CALC_ALLROWS, lpAdj))
    {
      if (InSrcRange(lpSheet, lCol2, CALC_ALLROWS, lpAdj))
      {
        lpExpr->nType = CALC_EXPR_ERROR;
        lpExpr->u.Error.nId = CALC_ERROR_REF;
      }
      else if (InDestRange(lpSheet, lCol2, CALC_ALLROWS, lpAdj))
      {
        lpExpr->nType = CALC_EXPR_ERROR;
        lpExpr->u.Error.nId = CALC_ERROR_REF;
      }
      else
      {
        lpExpr->nType = CALC_EXPR_ERROR;
        lpExpr->u.Error.nId = CALC_ERROR_REF;
      }
    }
    else
    {
      if (InSrcRange(lpSheet, lCol2, CALC_ALLROWS, lpAdj))
      {
        lCol2 = CoordPlus(lCol2, lpAdj->lOffsetCol);
        lpExpr->u.ExternalRange.lCol2 = lCol2;
      }
      else if (InDestRange(lpSheet, lCol2, CALC_ALLROWS, lpAdj))
      {
        lpExpr->nType = CALC_EXPR_ERROR;
        lpExpr->u.Error.nId = CALC_ERROR_REF;
      }
    }
  }
}

//--------------------------------------------------------------------
//
//  The ExprExtModifyOnDelCols() function modifies an expression which
//  is outside of the range being moved.
//

CALC_HANDLE ExprExtModifyOnDelCols(CALC_HANDLE hExpr, LPCALC_ADJUST lpAdj)
{
  LPCALC_EXPRINFO lpExpr;
  CALC_HANDLE hRet;

  hRet = ExprCreateCopy(hExpr);
  ExprDestroy(hExpr);
  if (hRet && (lpExpr = Expr_MemLock(hRet)))
  {
    for (; lpExpr->nType != CALC_EXPR_END; lpExpr++)
    {
      if (CALC_EXPR_CELL == lpExpr->nType ||
          CALC_EXPR_RANGE == lpExpr->nType ||
          CALC_EXPR_EXTERNALCELL == lpExpr->nType ||
          CALC_EXPR_EXTERNALRANGE == lpExpr->nType)
      {
        ExtModifyOnDelCols(lpExpr, lpAdj);
      }
    }
    Expr_MemUnlock(hRet);
  }
  return hRet;
}

//--------------------------------------------------------------------
//
//  The IntModifyOnDelCols() function modifies a cell coordinate which
//  is part of an expression which is inside of the range being
//  moved.
//

static void IntModifyOnDelCols(LPCALC_EXPRINFO lpExpr, LPCALC_ADJUST lpAdj)
{
  if (CALC_EXPR_CELL == lpExpr->nType)
  {
    LPCALC_SHEET lpSheet = lpAdj->lpCurSheet;
    long lCol = lpExpr->u.Cell.lCol;
    if (InSrcRange(lpSheet, lCol, CALC_ALLROWS, lpAdj))
    {
      if (CoordIsAbs(lCol))
        lCol = CoordPlus(lCol, lpAdj->lOffsetCol);
      lpExpr->u.Cell.lCol = lCol;
    }
    else if (InDestRange(lpSheet, lCol, CALC_ALLROWS, lpAdj))
    {
      lpExpr->nType = CALC_EXPR_ERROR;
      lpExpr->u.Error.nId = CALC_ERROR_REF;
    }
    else
    {
      if (CoordIsRel(lCol))
        lCol = CoordPlus(lCol, -lpAdj->lOffsetCol);
      lpExpr->u.Cell.lCol = lCol;
    }
  }
  else if (CALC_EXPR_RANGE == lpExpr->nType)
  {
    LPCALC_SHEET lpSheet = lpAdj->lpCurSheet;
    long lCol1 = lpExpr->u.Range.lCol1;
    long lCol2 = lpExpr->u.Range.lCol2;
    if (InSrcRange(lpSheet, lCol1, CALC_ALLROWS, lpAdj))
    {
      if (InSrcRange(lpSheet, lCol2, CALC_ALLROWS, lpAdj))
      {
        if (CoordIsAbs(lCol1))
          lCol1 = CoordPlus(lCol1, lpAdj->lOffsetCol);
        if (CoordIsAbs(lCol2))
          lCol2 = CoordPlus(lCol2, lpAdj->lOffsetCol);
        lpExpr->u.Range.lCol1 = lCol1;
        lpExpr->u.Range.lCol2 = lCol2;
      }
      else if(InDestRange(lpSheet, lCol2, CALC_ALLROWS, lpAdj))
      {
        lpExpr->nType = CALC_EXPR_ERROR;
        lpExpr->u.Error.nId = CALC_ERROR_REF;
      }
      else
      {
        if (CoordIsAbs(lCol1))
          lCol1 = CoordPlus(lCol1, lpAdj->lOffsetCol);
        if (CoordIsRel(lCol2))
          lCol2 = CoordPlus(lCol2, -lpAdj->lOffsetCol);
        lpExpr->u.Range.lCol1 = lCol1;
        lpExpr->u.Range.lCol2 = lCol2;
      }
    }
    else if (InDestRange(lpSheet, lCol1, CALC_ALLROWS, lpAdj))
    {
      if (InSrcRange(lpSheet, lCol2, CALC_ALLROWS, lpAdj))
      {
        lpExpr->nType = CALC_EXPR_ERROR;
        lpExpr->u.Error.nId = CALC_ERROR_REF;
      }
      else if (InDestRange(lpSheet, lCol2, CALC_ALLROWS, lpAdj))
      {
        lpExpr->nType = CALC_EXPR_ERROR;
        lpExpr->u.Error.nId = CALC_ERROR_REF;
      }
      else
      {
        lpExpr->nType = CALC_EXPR_ERROR;
        lpExpr->u.Error.nId = CALC_ERROR_REF;
      }
    }
    else
    {
      if (InSrcRange(lpSheet, lCol2, CALC_ALLROWS, lpAdj))
      {
        if (CoordIsRel(lCol1))
          lCol1 = CoordPlus(lCol1, -lpAdj->lOffsetCol);
        if (CoordIsAbs(lCol2))
          lCol2 = CoordPlus(lCol2, lpAdj->lOffsetCol);
        lpExpr->u.Range.lCol1 = lCol1;
        lpExpr->u.Range.lCol2 = lCol2;
      }
      else if (InDestRange(lpSheet, lCol2, CALC_ALLROWS, lpAdj))
      {
        lpExpr->nType = CALC_EXPR_ERROR;
        lpExpr->u.Error.nId = CALC_ERROR_REF;
      }
      else
      {
        if (CoordIsRel(lCol1))
          lCol1 = CoordPlus(lCol1, -lpAdj->lOffsetCol);
        if (CoordIsRel(lCol2))
          lCol2 = CoordPlus(lCol2, -lpAdj->lOffsetCol);
        lpExpr->u.Range.lCol1 = lCol1;
        lpExpr->u.Range.lCol2 = lCol2;
      }
    }
  }
  else if (CALC_EXPR_EXTERNALCELL == lpExpr->nType)
  {
    LPCALC_SHEET lpSheet = lpExpr->u.ExternalCell.lpSheet;
    long lCol = lpExpr->u.ExternalCell.lCol;
    if (InSrcRange(lpSheet, lCol, CALC_ALLROWS, lpAdj))
    {
      if (CoordIsAbs(lCol))
        lCol = CoordPlus(lCol, lpAdj->lOffsetCol);
      lpExpr->u.ExternalCell.lCol = lCol;
    }
    else if (InDestRange(lpSheet, lCol, CALC_ALLROWS, lpAdj))
    {
      lpExpr->nType = CALC_EXPR_ERROR;
      lpExpr->u.Error.nId = CALC_ERROR_REF;
    }
    else
    {
      if (CoordIsRel(lCol))
        lCol = CoordPlus(lCol, -lpAdj->lOffsetCol);
      lpExpr->u.ExternalCell.lCol = lCol;
    }
  }
  else if (CALC_EXPR_EXTERNALRANGE == lpExpr->nType)
  {
    LPCALC_SHEET lpSheet = lpExpr->u.ExternalRange.lpSheet;
    long lCol1 = lpExpr->u.ExternalRange.lCol1;
    long lCol2 = lpExpr->u.ExternalRange.lCol2;
    if (InSrcRange(lpSheet, lCol1, CALC_ALLROWS, lpAdj))
    {
      if (InSrcRange(lpSheet, lCol2, CALC_ALLROWS, lpAdj))
      {
        if (CoordIsAbs(lCol1))
          lCol1 = CoordPlus(lCol1, lpAdj->lOffsetCol);
        if (CoordIsAbs(lCol2))
          lCol2 = CoordPlus(lCol2, lpAdj->lOffsetCol);
        lpExpr->u.ExternalRange.lCol1 = lCol1;
        lpExpr->u.ExternalRange.lCol2 = lCol2;
      }
      else if (InDestRange(lpSheet, lCol2, CALC_ALLROWS, lpAdj))
      {
        lpExpr->nType = CALC_EXPR_ERROR;
        lpExpr->u.Error.nId = CALC_ERROR_REF;
      }
      else
      {
        if (CoordIsAbs(lCol1))
          lCol1 = CoordPlus(lCol1, lpAdj->lOffsetCol);
        if (CoordIsRel(lCol2))
          lCol2 = CoordPlus(lCol2, -lpAdj->lOffsetCol);
        lpExpr->u.ExternalRange.lCol1 = lCol1;
        lpExpr->u.ExternalRange.lCol2 = lCol2;
      }
    }
    else if (InDestRange(lpSheet, lCol1, CALC_ALLROWS, lpAdj))
    {
      if (InSrcRange(lpSheet, lCol2, CALC_ALLROWS, lpAdj))
      {
        lpExpr->nType = CALC_EXPR_ERROR;
        lpExpr->u.Error.nId = CALC_ERROR_REF;
      }
      else if (InDestRange(lpSheet, lCol2, CALC_ALLROWS, lpAdj))
      {
        lpExpr->nType = CALC_EXPR_ERROR;
        lpExpr->u.Error.nId = CALC_ERROR_REF;
      }
      else
      {
        lpExpr->nType = CALC_EXPR_ERROR;
        lpExpr->u.Error.nId = CALC_ERROR_REF;
      }
    }
    else
    {
      if (InSrcRange(lpSheet, lCol2, CALC_ALLROWS, lpAdj))
      {
        if (CoordIsRel(lCol1))
          lCol1 = CoordPlus(lCol1, -lpAdj->lOffsetCol);
        if (CoordIsAbs(lCol2))
          lCol2 = CoordPlus(lCol2, lpAdj->lOffsetCol);
        lpExpr->u.ExternalRange.lCol1 = lCol1;
        lpExpr->u.ExternalRange.lCol2 = lCol2;
      }
      else if (InDestRange(lpSheet, lCol2, CALC_ALLROWS, lpAdj))
      {
        lpExpr->nType = CALC_EXPR_ERROR;
        lpExpr->u.Error.nId = CALC_ERROR_REF;
      }
      else
      {
        if (CoordIsRel(lCol1))
          lCol1 = CoordPlus(lCol1, -lpAdj->lOffsetCol);
        if (CoordIsRel(lCol2))
          lCol2 = CoordPlus(lCol2, -lpAdj->lOffsetCol);
        lpExpr->u.ExternalRange.lCol1 = lCol1;
        lpExpr->u.ExternalRange.lCol2 = lCol2;
      }
    }
  }
}


//--------------------------------------------------------------------
//
//  The ExprIntModifyOnDelCols() function modifies an expressions which
//  is inside of the range being moved.
//

CALC_HANDLE ExprIntModifyOnDelCols(CALC_HANDLE hExpr, LPCALC_ADJUST lpAdj)
{
  LPCALC_EXPRINFO lpExpr;
  CALC_HANDLE hRet;

  hRet = ExprCreateCopy(hExpr);
  ExprDestroy(hExpr);
  if (hRet && (lpExpr = Expr_MemLock(hRet)))
  {
    for (; lpExpr->nType != CALC_EXPR_END; lpExpr++)
    {
      if (CALC_EXPR_CELL == lpExpr->nType ||
          CALC_EXPR_RANGE == lpExpr->nType ||
          CALC_EXPR_EXTERNALCELL == lpExpr->nType ||
          CALC_EXPR_EXTERNALRANGE == lpExpr->nType)
      {
        IntModifyOnDelCols(lpExpr, lpAdj);
      }
    }
    Expr_MemUnlock(hRet);
  }
  return hRet;
}

//--------------------------------------------------------------------
//
//  The ExtModifyOnDelRows() function modifies a cell coordinate which
//  is part of an expression which is outside of the range being
//  moved.
//

static void ExtModifyOnDelRows(LPCALC_EXPRINFO lpExpr, LPCALC_ADJUST lpAdj)
{
  if (CALC_EXPR_CELL == lpExpr->nType)
  {
    LPCALC_SHEET lpSheet = lpAdj->lpCurSheet;
    long lRow = lpExpr->u.Cell.lRow;
    if (InSrcRange(lpSheet, CALC_ALLCOLS, lRow, lpAdj))
    {
      lRow = CoordPlus(lRow, lpAdj->lOffsetRow);
      lpExpr->u.Cell.lRow = lRow;
    }
    else if (InDestRange(lpSheet, CALC_ALLCOLS, lRow, lpAdj))
    {
      lpExpr->nType = CALC_EXPR_ERROR;
      lpExpr->u.Error.nId = CALC_ERROR_REF;
    }
  }
  else if (CALC_EXPR_RANGE == lpExpr->nType)
  {
    LPCALC_SHEET lpSheet = lpAdj->lpCurSheet;
    long lRow1 = lpExpr->u.Range.lRow1;
    long lRow2 = lpExpr->u.Range.lRow2;
    if (InSrcRange(lpSheet, CALC_ALLCOLS, lRow1, lpAdj))
    {
      if (InSrcRange(lpSheet, CALC_ALLCOLS, lRow2, lpAdj))
      {
        lRow1 = CoordPlus(lRow1, lpAdj->lOffsetRow);
        lRow2 = CoordPlus(lRow2, lpAdj->lOffsetRow);
        lpExpr->u.Range.lRow1 = lRow1;
        lpExpr->u.Range.lRow2 = lRow2;
      }
      else if (InDestRange(lpSheet, CALC_ALLCOLS, lRow2, lpAdj))
      {
        lpExpr->nType = CALC_EXPR_ERROR;
        lpExpr->u.Error.nId = CALC_ERROR_REF;
      }
      else
      {
        lRow1 = CoordPlus(lRow1, lpAdj->lOffsetRow);
        lpExpr->u.Range.lRow1 = lRow1;
        lpExpr->u.Range.lRow2 = lRow2;
      }
    }
    else if (InDestRange(lpSheet, CALC_ALLCOLS, lRow1, lpAdj))
    {
      if (InSrcRange(lpSheet, CALC_ALLCOLS, lRow2, lpAdj))
      {
        lpExpr->nType = CALC_EXPR_ERROR;
        lpExpr->u.Error.nId = CALC_ERROR_REF;
      }
      else if (InDestRange(lpSheet, CALC_ALLCOLS, lRow2, lpAdj))
      {
        lpExpr->nType = CALC_EXPR_ERROR;
        lpExpr->u.Error.nId = CALC_ERROR_REF;
      }
      else
      {
        lpExpr->nType = CALC_EXPR_ERROR;
        lpExpr->u.Error.nId = CALC_ERROR_REF;
      }
    }
    else
    {
      if (InSrcRange(lpSheet, CALC_ALLCOLS, lRow2, lpAdj))
      {
        lRow2 = CoordPlus(lRow2, lpAdj->lOffsetRow);
        lpExpr->u.Range.lRow2 = lRow2;
      }
      else if (InDestRange(lpSheet, CALC_ALLCOLS, lRow2, lpAdj))
      {
        lpExpr->nType = CALC_EXPR_ERROR;
        lpExpr->u.Error.nId = CALC_ERROR_REF;
      }
    }
  }
  else if (CALC_EXPR_EXTERNALCELL == lpExpr->nType)
  {
    LPCALC_SHEET lpSheet = lpExpr->u.ExternalCell.lpSheet;
    long lRow = lpExpr->u.ExternalCell.lRow;
    if (InSrcRange(lpSheet, CALC_ALLCOLS, lRow, lpAdj))
    {
      lRow = CoordPlus(lRow, lpAdj->lOffsetRow);
      lpExpr->u.ExternalCell.lRow = lRow;
    }
    else if (InDestRange(lpSheet, CALC_ALLCOLS, lRow, lpAdj))
    {
      lpExpr->nType = CALC_EXPR_ERROR;
      lpExpr->u.Error.nId = CALC_ERROR_REF;
    }
  }
  else if (CALC_EXPR_EXTERNALRANGE == lpExpr->nType)
  {
    LPCALC_SHEET lpSheet = lpExpr->u.ExternalRange.lpSheet;
    long lRow1 = lpExpr->u.ExternalRange.lRow1;
    long lRow2 = lpExpr->u.ExternalRange.lRow2;
    if (InSrcRange(lpSheet, CALC_ALLCOLS, lRow1, lpAdj))
    {
      if (InSrcRange(lpSheet, CALC_ALLCOLS, lRow2, lpAdj))
      {
        lRow1 = CoordPlus(lRow1, lpAdj->lOffsetRow);
        lRow2 = CoordPlus(lRow2, lpAdj->lOffsetRow);
        lpExpr->u.ExternalRange.lRow1 = lRow1;
        lpExpr->u.ExternalRange.lRow2 = lRow2;
      }
      else if(InDestRange(lpSheet, CALC_ALLCOLS, lRow2, lpAdj))
      {
        lpExpr->nType = CALC_EXPR_ERROR;
        lpExpr->u.Error.nId = CALC_ERROR_REF;
      }
      else
      {
        lRow1 = CoordPlus(lRow1, lpAdj->lOffsetRow);
        lpExpr->u.ExternalRange.lRow1 = lRow1;
      }
    }
    else if (InDestRange(lpSheet, CALC_ALLCOLS, lRow1, lpAdj))
    {
      if (InSrcRange(lpSheet, CALC_ALLCOLS, lRow2, lpAdj))
      {
        lpExpr->nType = CALC_EXPR_ERROR;
        lpExpr->u.Error.nId = CALC_ERROR_REF;
      }
      else if (InDestRange(lpSheet, CALC_ALLCOLS, lRow2, lpAdj))
      {
        lpExpr->nType = CALC_EXPR_ERROR;
        lpExpr->u.Error.nId = CALC_ERROR_REF;
      }
      else
      {
        lpExpr->nType = CALC_EXPR_ERROR;
        lpExpr->u.Error.nId = CALC_ERROR_REF;
      }
    }
    else
    {
      if (InSrcRange(lpSheet, CALC_ALLCOLS, lRow2, lpAdj))
      {
        lRow2 = CoordPlus(lRow2, lpAdj->lOffsetRow);
        lpExpr->u.ExternalRange.lRow2 = lRow2;
      }
      else if (InDestRange(lpSheet, CALC_ALLCOLS, lRow2, lpAdj))
      {
        lpExpr->nType = CALC_EXPR_ERROR;
        lpExpr->u.Error.nId = CALC_ERROR_REF;
      }
    }
  }
}

//--------------------------------------------------------------------
//
//  The ExprExtModifyOnDelRows() function modifies an expression which
//  is outside of the range being moved.
//

CALC_HANDLE ExprExtModifyOnDelRows(CALC_HANDLE hExpr, LPCALC_ADJUST lpAdj)
{
  LPCALC_EXPRINFO lpExpr;
  CALC_HANDLE hRet;

  hRet = ExprCreateCopy(hExpr);
  ExprDestroy(hExpr);
  if (hRet && (lpExpr = Expr_MemLock(hRet)))
  {
    for (; lpExpr->nType != CALC_EXPR_END; lpExpr++)
    {
      if (CALC_EXPR_CELL == lpExpr->nType ||
          CALC_EXPR_RANGE == lpExpr->nType ||
          CALC_EXPR_EXTERNALCELL == lpExpr->nType ||
          CALC_EXPR_EXTERNALRANGE == lpExpr->nType)
      {
        ExtModifyOnDelRows(lpExpr, lpAdj);
      }
    }
    Expr_MemUnlock(hRet);
  }
  return hRet;
}

//--------------------------------------------------------------------
//
//  The IntModifyOnDelRows() function modifies a cell coordinate which
//  is part of an expression which is inside of the range being
//  moved.
//

static void IntModifyOnDelRows(LPCALC_EXPRINFO lpExpr, LPCALC_ADJUST lpAdj)
{
  if (CALC_EXPR_CELL == lpExpr->nType)
  {
    LPCALC_SHEET lpSheet = lpAdj->lpCurSheet;
    long lRow = lpExpr->u.Cell.lRow;
    if (InSrcRange(lpSheet, CALC_ALLCOLS, lRow, lpAdj))
    {
      if (CoordIsAbs(lRow))
        lRow = CoordPlus(lRow, lpAdj->lOffsetRow);
      lpExpr->u.Cell.lRow = lRow;
    }
    else if (InDestRange(lpSheet, CALC_ALLCOLS, lRow, lpAdj))
    {
      lpExpr->nType = CALC_EXPR_ERROR;
      lpExpr->u.Error.nId = CALC_ERROR_REF;
    }
    else
    {
      if (CoordIsRel(lRow))
        lRow = CoordPlus(lRow, -lpAdj->lOffsetRow);
      lpExpr->u.Cell.lRow = lRow;
    }
  }
  else if (CALC_EXPR_RANGE == lpExpr->nType)
  {
    LPCALC_SHEET lpSheet = lpAdj->lpCurSheet;
    long lRow1 = lpExpr->u.Range.lRow1;
    long lRow2 = lpExpr->u.Range.lRow2;
    if (InSrcRange(lpSheet, CALC_ALLCOLS, lRow1, lpAdj))
    {
      if (InSrcRange(lpSheet, CALC_ALLCOLS, lRow2, lpAdj))
      {
        if (CoordIsAbs(lRow1))
          lRow1 = CoordPlus(lRow1, lpAdj->lOffsetRow);
        if (CoordIsAbs(lRow2))
          lRow2 = CoordPlus(lRow2, lpAdj->lOffsetRow);
        lpExpr->u.Range.lRow1 = lRow1;
        lpExpr->u.Range.lRow2 = lRow2;
      }
      else if(InDestRange(lpSheet, CALC_ALLCOLS, lRow2, lpAdj))
      {
        lpExpr->nType = CALC_EXPR_ERROR;
        lpExpr->u.Error.nId = CALC_ERROR_REF;
      }
      else
      {
        if (CoordIsAbs(lRow1))
          lRow1 = CoordPlus(lRow1, lpAdj->lOffsetRow);
        if (CoordIsRel(lRow2))
          lRow2 = CoordPlus(lRow2, -lpAdj->lOffsetRow);
        lpExpr->u.Range.lRow1 = lRow1;
        lpExpr->u.Range.lRow2 = lRow2;
      }
    }
    else if (InDestRange(lpSheet, CALC_ALLCOLS, lRow1, lpAdj))
    {
      if (InSrcRange(lpSheet, CALC_ALLCOLS, lRow2, lpAdj))
      {
        lpExpr->nType = CALC_EXPR_ERROR;
        lpExpr->u.Error.nId = CALC_ERROR_REF;
      }
      else if (InDestRange(lpSheet, CALC_ALLCOLS, lRow2, lpAdj))
      {
        lpExpr->nType = CALC_EXPR_ERROR;
        lpExpr->u.Error.nId = CALC_ERROR_REF;
      }
      else
      {
        lpExpr->nType = CALC_EXPR_ERROR;
        lpExpr->u.Error.nId = CALC_ERROR_REF;
      }
    }
    else
    {
      if (InSrcRange(lpSheet, CALC_ALLCOLS, lRow2, lpAdj))
      {
        if (CoordIsRel(lRow1))
          lRow1 = CoordPlus(lRow1, -lpAdj->lOffsetRow);
        if (CoordIsAbs(lRow2))
          lRow2 = CoordPlus(lRow2, lpAdj->lOffsetRow);
        lpExpr->u.Range.lRow1 = lRow1;
        lpExpr->u.Range.lRow2 = lRow2;
      }
      else if (InDestRange(lpSheet, CALC_ALLCOLS, lRow2, lpAdj))
      {
        lpExpr->nType = CALC_EXPR_ERROR;
        lpExpr->u.Error.nId = CALC_ERROR_REF;
      }
      else
      {
        if (CoordIsRel(lRow1))
          lRow1 = CoordPlus(lRow1, -lpAdj->lOffsetRow);
        if (CoordIsRel(lRow2))
          lRow2 = CoordPlus(lRow2, -lpAdj->lOffsetRow);
        lpExpr->u.Range.lRow1 = lRow1;
        lpExpr->u.Range.lRow2 = lRow2;
      }
    }
  }
  else if (CALC_EXPR_EXTERNALCELL == lpExpr->nType)
  {
    LPCALC_SHEET lpSheet = lpExpr->u.ExternalCell.lpSheet;
    long lRow = lpExpr->u.ExternalCell.lRow;
    if (InSrcRange(lpSheet, CALC_ALLCOLS, lRow, lpAdj))
    {
      if (CoordIsAbs(lRow))
        lRow = CoordPlus(lRow, lpAdj->lOffsetRow);
      lpExpr->u.ExternalCell.lRow = lRow;
    }
    else if (InDestRange(lpSheet, CALC_ALLCOLS, lRow, lpAdj))
    {
      lpExpr->nType = CALC_EXPR_ERROR;
      lpExpr->u.Error.nId = CALC_ERROR_REF;
    }
    else
    {
      if (CoordIsRel(lRow))
        lRow = CoordPlus(lRow, -lpAdj->lOffsetRow);
      lpExpr->u.ExternalCell.lRow = lRow;
    }
  }
  else if (CALC_EXPR_EXTERNALRANGE == lpExpr->nType)
  {
    LPCALC_SHEET lpSheet = lpExpr->u.ExternalRange.lpSheet;
    long lRow1 = lpExpr->u.ExternalRange.lRow1;
    long lRow2 = lpExpr->u.ExternalRange.lRow2;
    if (InSrcRange(lpSheet, CALC_ALLCOLS, lRow1, lpAdj))
    {
      if (InSrcRange(lpSheet, CALC_ALLCOLS, lRow2, lpAdj))
      {
        if (CoordIsAbs(lRow1))
          lRow1 = CoordPlus(lRow1, lpAdj->lOffsetRow);
        if (CoordIsAbs(lRow2))
          lRow2 = CoordPlus(lRow2, lpAdj->lOffsetRow);
        lpExpr->u.ExternalRange.lRow1 = lRow1;
        lpExpr->u.ExternalRange.lRow2 = lRow2;
      }
      else if (InDestRange(lpSheet, CALC_ALLCOLS, lRow2, lpAdj))
      {
        lpExpr->nType = CALC_EXPR_ERROR;
        lpExpr->u.Error.nId = CALC_ERROR_REF;
      }
      else
      {
        if (CoordIsAbs(lRow1))
          lRow1 = CoordPlus(lRow1, lpAdj->lOffsetRow);
        if (CoordIsRel(lRow2))
          lRow2 = CoordPlus(lRow2, -lpAdj->lOffsetRow);
        lpExpr->u.ExternalRange.lRow1 = lRow1;
        lpExpr->u.ExternalRange.lRow2 = lRow2;
      }
    }
    else if (InDestRange(lpSheet, CALC_ALLCOLS, lRow1, lpAdj))
    {
      if (InSrcRange(lpSheet, CALC_ALLCOLS, lRow2, lpAdj))
      {
        lpExpr->nType = CALC_EXPR_ERROR;
        lpExpr->u.Error.nId = CALC_ERROR_REF;
      }
      else if (InDestRange(lpSheet, CALC_ALLCOLS, lRow2, lpAdj))
      {
        lpExpr->nType = CALC_EXPR_ERROR;
        lpExpr->u.Error.nId = CALC_ERROR_REF;
      }
      else
      {
        lpExpr->nType = CALC_EXPR_ERROR;
        lpExpr->u.Error.nId = CALC_ERROR_REF;
      }
    }
    else
    {
      if (InSrcRange(lpSheet, CALC_ALLCOLS, lRow2, lpAdj))
      {
        if (CoordIsRel(lRow1))
          lRow1 = CoordPlus(lRow1, -lpAdj->lOffsetRow);
        if (CoordIsAbs(lRow2))
          lRow2 = CoordPlus(lRow2, lpAdj->lOffsetRow);
        lpExpr->u.ExternalRange.lRow1 = lRow1;
        lpExpr->u.ExternalRange.lRow2 = lRow2;
      }
      else if (InDestRange(lpSheet, CALC_ALLCOLS, lRow2, lpAdj))
      {
        lpExpr->nType = CALC_EXPR_ERROR;
        lpExpr->u.Error.nId = CALC_ERROR_REF;
      }
      else
      {
        if (CoordIsRel(lRow1))
          lRow1 = CoordPlus(lRow1, -lpAdj->lOffsetRow);
        if (CoordIsRel(lRow2))
          lRow2 = CoordPlus(lRow2, -lpAdj->lOffsetRow);
        lpExpr->u.ExternalRange.lRow1 = lRow1;
        lpExpr->u.ExternalRange.lRow2 = lRow2;
      }
    }
  }
}


//--------------------------------------------------------------------
//
//  The ExprIntModifyOnDelRows() function modifies an expressions which
//  is inside of the range being moved.
//

CALC_HANDLE ExprIntModifyOnDelRows(CALC_HANDLE hExpr, LPCALC_ADJUST lpAdj)
{
  LPCALC_EXPRINFO lpExpr;
  CALC_HANDLE hRet;

  hRet = ExprCreateCopy(hExpr);
  ExprDestroy(hExpr);
  if (hRet && (lpExpr = Expr_MemLock(hRet)))
  {
    for (; lpExpr->nType != CALC_EXPR_END; lpExpr++)
    {
      if (CALC_EXPR_CELL == lpExpr->nType ||
          CALC_EXPR_RANGE == lpExpr->nType ||
          CALC_EXPR_EXTERNALCELL == lpExpr->nType ||
          CALC_EXPR_EXTERNALRANGE == lpExpr->nType)
      {
        IntModifyOnDelRows(lpExpr, lpAdj);
      }
    }
    Expr_MemUnlock(hRet);
  }
  return hRet;
}

//--------------------------------------------------------------------
//
//  The ExtModifyOnDelSheet() function modifies a cell coordinate
//  which is part of an expression which is outside of the sheet
//  being deleted.
//

static void ExtModifyOnDelSheet(LPCALC_EXPRINFO lpExpr, LPCALC_ADJUST lpAdj)
{
  if (CALC_EXPR_EXTERNALCELL == lpExpr->nType)
  {
    LPCALC_SHEET lpSheet = lpExpr->u.ExternalCell.lpSheet;
    if (lpSheet == lpAdj->lpSrcSheet)
    {
      lpExpr->nType = CALC_EXPR_ERROR;
      lpExpr->u.Error.nId = CALC_ERROR_REF;
    }
  }
  else if (CALC_EXPR_EXTERNALRANGE == lpExpr->nType)
  {
    LPCALC_SHEET lpSheet = lpExpr->u.ExternalRange.lpSheet;
    if (lpSheet == lpAdj->lpSrcSheet)
    {
      lpExpr->nType = CALC_EXPR_ERROR;
      lpExpr->u.Error.nId = CALC_ERROR_REF;
    }
  }
}

//--------------------------------------------------------------------
//
//  The ExprExtModifyOnDelSheet() function modifies an expression
//  which is outside of the sheet being deleted.
//

CALC_HANDLE ExprExtModifyOnDelSheet(CALC_HANDLE hExpr, LPCALC_ADJUST lpAdj)
{
  LPCALC_EXPRINFO lpExpr;
  CALC_HANDLE hRet;

  hRet = ExprCreateCopy(hExpr);
  ExprDestroy(hExpr);
  if (hRet && (lpExpr = Expr_MemLock(hRet)))
  {
    for (; lpExpr->nType != CALC_EXPR_END; lpExpr++)
    {
      if (CALC_EXPR_EXTERNALCELL == lpExpr->nType  ||
          CALC_EXPR_EXTERNALRANGE == lpExpr->nType)
      {
        ExtModifyOnDelSheet(lpExpr, lpAdj);
      }
    }
    Expr_MemUnlock(hRet);
  }
  return hRet;
}

//--------------------------------------------------------------------

void CalcStringReplaceCh(LPTSTR szStr, TCHAR cNew, TCHAR cOld)
{
  while (*szStr)
  {
    if (*szStr == cOld)
      *szStr = cNew;
    ++szStr;
  }
}

//--------------------------------------------------------------------

double CalcStringToDouble(LPCTSTR lpszText)
{
  TCHAR cDecimal = CalcGetDefDecimalChar();
  TCHAR szBuff[CALC_MAX_NUM_LEN];
  int nTextLen = (int)_tcslen(lpszText);
  int nBuffLen = min(nTextLen, sizeof(szBuff) / sizeof(TCHAR) - 1);

  _tcsncpy(szBuff, lpszText, nBuffLen);
  szBuff[nBuffLen] = '\0';
  if (cDecimal != '.')
     CalcStringReplaceCh(szBuff, cDecimal, '.');
  return _tcstod(szBuff, NULL);
}

//--------------------------------------------------------------------

TCHAR CalcGetDefDecimalChar(void)
{
  static TCHAR cDecimal = 0;

  if (!cDecimal)
  {
    struct lconv *plconv;
    plconv = localeconv();

    if (plconv->decimal_point[0])
      cDecimal = plconv->decimal_point[0];
    else
      cDecimal = '.';
  }
  return cDecimal;
}

//--------------------------------------------------------------------

#if defined(SS_V70)

//--------------------------------------------------------------------

static BOOL ShouldUseRange(long lCol1, long lRow1, long lCol2, long lRow2)
{
  long lColCount = lCol2 - lCol1 + 1;
  long lRowCount = lRow2 - lRow1 + 1;
  return lColCount > 0 && lRowCount > 0 && lColCount * lRowCount > 255;
}

//--------------------------------------------------------------------

void Expr_CellStartListening(CALC_HANDLE hThis, CALC_HANDLE hCell, BOOL bIncludeNames)
{
  LPCALC_EXPRINFO lpThis;
  LPCALC_VALUE lpStack;
  CALC_VALUESTACK stack;
  long i, j, k;

  if (hThis && (lpThis = Expr_MemLock(hThis)))
  {
    ValStackInit(&stack);
    ExprSemiEval(lpThis, Cell_GetSheet(hCell), Cell_GetCol(hCell), Cell_GetRow(hCell), &stack);
    if (bIncludeNames)
    {
      for (; lpThis->nType != CALC_EXPR_END; lpThis++)
      {
        if (lpThis->nType == CALC_EXPR_NAME)
          Name_AddCellListener(lpThis->u.CustName.hName, hCell);
      }
    }
    for (k = 0; k < stack.nElemCnt; k++)
    {
      lpStack = stack.lpElem + k;
      if (CALC_VALUE_STATUS_OK == lpStack->Status)
      {
        if (CALC_VALUE_TYPE_CELL == lpStack->Type)
        {
          CALC_HANDLE hPreced = CellLookupAlloc((LPCALC_SHEET)lpStack->Val.ValCell.u.lInfo, lpStack->Val.ValCell.Col, lpStack->Val.ValCell.Row);
          Cell_AddCellListener(hPreced, hCell);
        }
        else if (CALC_VALUE_TYPE_RANGE == lpStack->Type)
        {
          if (ShouldUseRange(lpStack->Val.ValRange.Col1, lpStack->Val.ValRange.Row1, lpStack->Val.ValRange.Col2, lpStack->Val.ValRange.Row2))
          {
            Sheet_CellAddCellListener((LPCALC_SHEET)lpStack->Val.ValRange.u.lInfo, lpStack->Val.ValRange.Col1, lpStack->Val.ValRange.Row1, lpStack->Val.ValRange.Col2, lpStack->Val.ValRange.Row2, hCell);
          }
          else
          {
            for (i = lpStack->Val.ValRange.Col1; i <= lpStack->Val.ValRange.Col2; i++)
            {
              for (j = lpStack->Val.ValRange.Row1; j <= lpStack->Val.ValRange.Row2; j++)
              {
                CALC_HANDLE hPreced = CellLookupAlloc((LPCALC_SHEET)lpStack->Val.ValRange.u.lInfo, i, j);
                Cell_AddCellListener(hPreced, hCell);
              }
            }
          }
        }
      }
    }
    ValStackFree(&stack);
    Expr_MemUnlock(hThis);
  }
}

//--------------------------------------------------------------------

void Expr_CellStopListening(CALC_HANDLE hThis, CALC_HANDLE hCell, BOOL bIncludeNames)
{
  LPCALC_EXPRINFO lpThis;
  LPCALC_VALUE lpStack;
  CALC_VALUESTACK stack;
  long i, j, k;

  if (hThis && (lpThis = Expr_MemLock(hThis)))
  {
    ValStackInit(&stack);
    ExprSemiEval(lpThis, Cell_GetSheet(hCell), Cell_GetCol(hCell), Cell_GetRow(hCell), &stack);
    if (bIncludeNames)
    {
      for (; lpThis->nType != CALC_EXPR_END; lpThis++)
      {
        if (lpThis->nType == CALC_EXPR_NAME)
          Name_RemoveCellListener(lpThis->u.CustName.hName, hCell);
      }
    }
    for (k = 0; k < stack.nElemCnt; k++)
    {
      lpStack = stack.lpElem + k;
      if (CALC_VALUE_STATUS_OK == lpStack->Status)
      {
        if (CALC_VALUE_TYPE_CELL == lpStack->Type)
        {
          CALC_HANDLE hPreced = CellLookupAlloc((LPCALC_SHEET)lpStack->Val.ValCell.u.lInfo, lpStack->Val.ValCell.Col, lpStack->Val.ValCell.Row);
          Cell_RemoveCellListener(hPreced, hCell);
        }
        else if (CALC_VALUE_TYPE_RANGE == lpStack->Type)
        {
          if (ShouldUseRange(lpStack->Val.ValRange.Col1, lpStack->Val.ValRange.Row1, lpStack->Val.ValRange.Col2, lpStack->Val.ValRange.Row2))
          {
            Sheet_CellRemoveCellListener((LPCALC_SHEET)lpStack->Val.ValRange.u.lInfo, lpStack->Val.ValRange.Col1, lpStack->Val.ValRange.Row1, lpStack->Val.ValRange.Col2, lpStack->Val.ValRange.Row2, hCell);
          }
          else
          {
            for (i = lpStack->Val.ValRange.Col1; i <= lpStack->Val.ValRange.Col2; i++)
            {
              for (j = lpStack->Val.ValRange.Row1; j <= lpStack->Val.ValRange.Row2; j++)
              {
                CALC_HANDLE hPreced = CellLookupAlloc((LPCALC_SHEET)lpStack->Val.ValRange.u.lInfo, i, j);
                Cell_RemoveCellListener(hPreced, hCell);
              }
            }
          }
        }
      }
    }
    ValStackFree(&stack);
    Expr_MemUnlock(hThis);
  }
}

//--------------------------------------------------------------------

void Expr_RowStartListening(CALC_HANDLE hThis, CALC_HANDLE hRow, BOOL bIncludeNames)
{
  LPCALC_EXPRINFO lpThis;
  LPCALC_VALUE lpStack;
  CALC_VALUESTACK stack;
  long i, j, k;

  if (hThis && (lpThis = Expr_MemLock(hThis)))
  {
    ValStackInit(&stack);
    ExprSemiEval(lpThis, Row_GetSheet(hRow), CALC_ALLCOLS, Row_GetRow(hRow), &stack);
    if (bIncludeNames)
    {
      for ( ; lpThis->nType != CALC_EXPR_END; lpThis++)
      {
        if (lpThis->nType == CALC_EXPR_NAME)
          Name_AddRowListener(lpThis->u.CustName.hName, hRow);
      }
    }
    for (k = 0; k < stack.nElemCnt; k++)
    {
      lpStack = stack.lpElem + k;
      if( CALC_VALUE_STATUS_OK == lpStack->Status )
      {
        if( CALC_VALUE_TYPE_CELL == lpStack->Type )
        {
          if( CoordIsRel(lpStack->Val.ValCell.Col) )
          {
            CALC_HANDLE hPreced = RowLookupAlloc((LPCALC_SHEET)lpStack->Val.ValCell.u.lInfo, lpStack->Val.ValCell.Row);
            lpStack->Val.ValCell.Col = CoordMakeAbs(lpStack->Val.ValCell.Col, 0);
            Row_AddRowListener(hPreced, hRow, -lpStack->Val.ValCell.Col);
          }
          else
          {
            CALC_HANDLE hPreced = CellLookupAlloc((LPCALC_SHEET)lpStack->Val.ValCell.u.lInfo, lpStack->Val.ValCell.Col, lpStack->Val.ValCell.Row);
            Cell_AddRowListener(hPreced, hRow);
          }
        }
        else if (CALC_VALUE_TYPE_RANGE == lpStack->Type )
        {
          if( CoordIsRel(lpStack->Val.ValRange.Col1) &&
              CoordIsRel(lpStack->Val.ValRange.Col2) )
          {
            lpStack->Val.ValRange.Col1 = CoordMakeAbs(lpStack->Val.ValRange.Col1, 0);
            lpStack->Val.ValRange.Col2 = CoordMakeAbs(lpStack->Val.ValRange.Col2, 0);
            if (ShouldUseRange(lpStack->Val.ValRange.Col1, lpStack->Val.ValRange.Row1, lpStack->Val.ValRange.Col2, lpStack->Val.ValRange.Row2))
            {
              Sheet_RowAddRowListener((LPCALC_SHEET)lpStack->Val.ValRange.u.lInfo, lpStack->Val.ValRange.Row1, lpStack->Val.ValRange.Row2, hRow, -lpStack->Val.ValRange.Col2, -lpStack->Val.ValRange.Col1);
            }
            else
            {
              for( i = lpStack->Val.ValRange.Col1; i <= lpStack->Val.ValRange.Col2; i++ )
              {
                for( j = lpStack->Val.ValRange.Row1; j <= lpStack->Val.ValRange.Row2; j++ )
                {
                  CALC_HANDLE hPreced = RowLookupAlloc((LPCALC_SHEET)lpStack->Val.ValRange.u.lInfo, j);
                  Row_AddRowListener(hPreced, hRow, -i);
                }
              }
            }
          }
          else if( CoordIsAbs(lpStack->Val.ValRange.Col1) &&
                   CoordIsAbs(lpStack->Val.ValRange.Col2) )
          {
            if (ShouldUseRange(lpStack->Val.ValRange.Col1, lpStack->Val.ValRange.Row1, lpStack->Val.ValRange.Col2, lpStack->Val.ValRange.Row2))
            {
              Sheet_CellAddRowListener((LPCALC_SHEET)lpStack->Val.ValRange.u.lInfo, lpStack->Val.ValRange.Col1, lpStack->Val.ValRange.Row1, lpStack->Val.ValRange.Col2, lpStack->Val.ValRange.Row2, hRow);
            }
            else
            {
              for (i = lpStack->Val.ValRange.Col1; i <= lpStack->Val.ValRange.Col2; i++)
              {
                for (j = lpStack->Val.ValRange.Row1; j <= lpStack->Val.ValRange.Row2; j++)
                {
                  CALC_HANDLE hPreced = CellLookupAlloc((LPCALC_SHEET)lpStack->Val.ValRange.u.lInfo, i, j);
                  Cell_AddRowListener(hPreced, hRow);
                }
              }
            }
          }
        }
      }
    }
    ValStackFree(&stack);
    Expr_MemUnlock(hThis);
  }
}

//--------------------------------------------------------------------

void Expr_RowStopListening(CALC_HANDLE hThis, CALC_HANDLE hRow, BOOL bIncludeNames)
{
  LPCALC_EXPRINFO lpThis;
  LPCALC_VALUE lpStack;
  CALC_VALUESTACK stack;
  long i, j, k;

  if (hThis && (lpThis = Expr_MemLock(hThis)))
  {
    ValStackInit(&stack);
    ExprSemiEval(lpThis, Row_GetSheet(hRow), CALC_ALLCOLS, Row_GetRow(hRow), &stack);
    if (bIncludeNames)
    {
      for ( ; lpThis->nType != CALC_EXPR_END; lpThis++)
      {
        if (lpThis->nType == CALC_EXPR_NAME)
          Name_RemoveRowListener(lpThis->u.CustName.hName, hRow);
      }
    }
    for (k = 0; k < stack.nElemCnt; k++)
    {
      lpStack = stack.lpElem + k;
      if( CALC_VALUE_STATUS_OK == lpStack->Status )
      {
        if( CALC_VALUE_TYPE_CELL == lpStack->Type )
        {
          if( CoordIsRel(lpStack->Val.ValCell.Col) )
          {
            CALC_HANDLE hPreced = RowLookupAlloc((LPCALC_SHEET)lpStack->Val.ValCell.u.lInfo, lpStack->Val.ValCell.Row);
            lpStack->Val.ValCell.Col = CoordMakeAbs(lpStack->Val.ValCell.Col, 0);
            Row_RemoveRowListener(hPreced, hRow, -lpStack->Val.ValCell.Col);
          }
          else
          {
            CALC_HANDLE hPreced = CellLookupAlloc((LPCALC_SHEET)lpStack->Val.ValCell.u.lInfo, lpStack->Val.ValCell.Col, lpStack->Val.ValCell.Row);
            Cell_RemoveRowListener(hPreced, hRow);
          }
        }
        else if (CALC_VALUE_TYPE_RANGE == lpStack->Type )
        {
          if( CoordIsRel(lpStack->Val.ValRange.Col1) &&
              CoordIsRel(lpStack->Val.ValRange.Col2) )
          {
            lpStack->Val.ValRange.Col1 = CoordMakeAbs(lpStack->Val.ValRange.Col1, 0);
            lpStack->Val.ValRange.Col2 = CoordMakeAbs(lpStack->Val.ValRange.Col2, 0);
            if (ShouldUseRange(lpStack->Val.ValRange.Col1, lpStack->Val.ValRange.Row1, lpStack->Val.ValRange.Col2, lpStack->Val.ValRange.Row2))
            {
              Sheet_RowRemoveRowListener((LPCALC_SHEET)lpStack->Val.ValRange.u.lInfo, lpStack->Val.ValRange.Row1, lpStack->Val.ValRange.Row2, hRow, -lpStack->Val.ValRange.Col2, -lpStack->Val.ValRange.Col1);
            }
            else
            {
              for( i = lpStack->Val.ValRange.Col1; i <= lpStack->Val.ValRange.Col2; i++ )
              {
                for( j = lpStack->Val.ValRange.Row1; j <= lpStack->Val.ValRange.Row2; j++ )
                {
                  CALC_HANDLE hPreced = RowLookupAlloc((LPCALC_SHEET)lpStack->Val.ValRange.u.lInfo, j);
                  Row_RemoveRowListener(hPreced, hRow, -i);
                }
              }
            }
          }
          else if( CoordIsAbs(lpStack->Val.ValRange.Col1) &&
                   CoordIsAbs(lpStack->Val.ValRange.Col2) )
          {
            if (ShouldUseRange(lpStack->Val.ValRange.Col1, lpStack->Val.ValRange.Row1, lpStack->Val.ValRange.Col2, lpStack->Val.ValRange.Row2))
            {
              Sheet_CellRemoveRowListener((LPCALC_SHEET)lpStack->Val.ValRange.u.lInfo, lpStack->Val.ValRange.Col1, lpStack->Val.ValRange.Row1, lpStack->Val.ValRange.Col2, lpStack->Val.ValRange.Row2, hRow);
            }
            else
            {
              for (i = lpStack->Val.ValRange.Col1; i <= lpStack->Val.ValRange.Col2; i++)
              {
                for (j = lpStack->Val.ValRange.Row1; j <= lpStack->Val.ValRange.Row2; j++)
                {
                  CALC_HANDLE hPreced = CellLookupAlloc((LPCALC_SHEET)lpStack->Val.ValRange.u.lInfo, i, j);
                  Cell_RemoveRowListener(hPreced, hRow);
                }
              }
            }
          }
        }
      }
    }
    ValStackFree(&stack);
    Expr_MemUnlock(hThis);
  }
}

//--------------------------------------------------------------------

void Expr_ColStartListening(CALC_HANDLE hThis, CALC_HANDLE hCol, BOOL bIncludeNames)
{
  LPCALC_EXPRINFO lpThis;
  LPCALC_VALUE lpStack;
  CALC_VALUESTACK stack;
  long i, j, k;

  if (hThis && (lpThis = Expr_MemLock(hThis)))
  {
    ValStackInit(&stack);
    ExprSemiEval(lpThis, Col_GetSheet(hCol), Col_GetCol(hCol), CALC_ALLROWS, &stack);
    if (bIncludeNames)
    {
      for (; lpThis->nType != CALC_EXPR_END; lpThis++)
      {
        if (lpThis->nType == CALC_EXPR_NAME)
          Name_AddColListener(lpThis->u.CustName.hName, hCol);
      }
    }
    for (k = 0; k < stack.nElemCnt; k++)
    {
      lpStack = stack.lpElem + k;
      if (CALC_VALUE_STATUS_OK == lpStack->Status)
      {
        if (CALC_VALUE_TYPE_CELL == lpStack->Type)
        {
          if (CoordIsRel(lpStack->Val.ValCell.Row))
          {
            CALC_HANDLE hPreced = ColLookupAlloc((LPCALC_SHEET)lpStack->Val.ValCell.u.lInfo, lpStack->Val.ValCell.Col);
            lpStack->Val.ValCell.Row = CoordMakeAbs(lpStack->Val.ValCell.Row, 0);
            Col_AddColListener(hPreced, hCol, -lpStack->Val.ValCell.Row);
          }
          else
          {
            CALC_HANDLE hPreced = CellLookupAlloc((LPCALC_SHEET)lpStack->Val.ValCell.u.lInfo, lpStack->Val.ValCell.Col, lpStack->Val.ValCell.Row);
            Cell_AddColListener(hPreced, hCol);
          }
        }
        else if (CALC_VALUE_TYPE_RANGE == lpStack->Type)
        {
          if (CoordIsRel(lpStack->Val.ValRange.Row1) &&
              CoordIsRel(lpStack->Val.ValRange.Row2))
          {
            lpStack->Val.ValRange.Row1 = CoordMakeAbs(lpStack->Val.ValRange.Row1, 0);
            lpStack->Val.ValRange.Row2 = CoordMakeAbs(lpStack->Val.ValRange.Row2, 0);
            if (ShouldUseRange(lpStack->Val.ValRange.Col1, lpStack->Val.ValRange.Row1, lpStack->Val.ValRange.Col2, lpStack->Val.ValRange.Row2))
            {
              Sheet_ColAddColListener((LPCALC_SHEET)lpStack->Val.ValRange.u.lInfo,lpStack->Val.ValRange.Col1, lpStack->Val.ValRange.Col2, hCol, -lpStack->Val.ValRange.Row2, -lpStack->Val.ValRange.Row1);
            }
            else
            {
              for (i = lpStack->Val.ValRange.Col1; i <= lpStack->Val.ValRange.Col2; i++)
              {
                for (j = lpStack->Val.ValRange.Row1; j <= lpStack->Val.ValRange.Row2; j++)
                {
                  CALC_HANDLE hPreced = ColLookupAlloc((LPCALC_SHEET)lpStack->Val.ValRange.u.lInfo, i);
                  Col_AddColListener(hPreced, hCol, -j);
                }
              }
            }
          }
          else if (CoordIsAbs(lpStack->Val.ValRange.Row1) &&
                   CoordIsAbs(lpStack->Val.ValRange.Row2))
          {
            if (ShouldUseRange(lpStack->Val.ValRange.Col1, lpStack->Val.ValRange.Row1, lpStack->Val.ValRange.Col2, lpStack->Val.ValRange.Row2))
            {
              Sheet_CellAddColListener((LPCALC_SHEET)lpStack->Val.ValRange.u.lInfo, lpStack->Val.ValRange.Col1, lpStack->Val.ValRange.Row1, lpStack->Val.ValRange.Col2, lpStack->Val.ValRange.Row2, hCol);
            }
            else
            {
              for (i = lpStack->Val.ValRange.Col1; i <= lpStack->Val.ValRange.Col2; i++)
              {
                for (j = lpStack->Val.ValRange.Row1; j <= lpStack->Val.ValRange.Row2; j++)
                {
                  CALC_HANDLE hPreced = CellLookupAlloc((LPCALC_SHEET)lpStack->Val.ValRange.u.lInfo, i, j);
                  Cell_AddColListener(hPreced, hCol);
                }
              }
            }
          }
        }
      }
    }
    ValStackFree(&stack);
    Expr_MemUnlock(hThis);
  }
}

//--------------------------------------------------------------------

void Expr_ColStopListening(CALC_HANDLE hThis, CALC_HANDLE hCol, BOOL bIncludeNames)
{
  LPCALC_EXPRINFO lpThis;
  LPCALC_VALUE lpStack;
  CALC_VALUESTACK stack;
  long i, j, k;

  if (hThis && (lpThis = Expr_MemLock(hThis)))
  {
    ValStackInit(&stack);
    ExprSemiEval(lpThis, Col_GetSheet(hCol), Col_GetCol(hCol), CALC_ALLROWS, &stack);
    if (bIncludeNames)
    {
      for (; lpThis->nType != CALC_EXPR_END; lpThis++)
      {
        if (lpThis->nType == CALC_EXPR_NAME)
          Name_RemoveColListener(lpThis->u.CustName.hName, hCol);
      }
    }
    for (k = 0; k < stack.nElemCnt; k++)
    {
      lpStack = stack.lpElem + k;
      if (CALC_VALUE_STATUS_OK == lpStack->Status)
      {
        if (CALC_VALUE_TYPE_CELL == lpStack->Type)
        {
          if (CoordIsRel(lpStack->Val.ValCell.Row))
          {
            CALC_HANDLE hPreced = ColLookupAlloc((LPCALC_SHEET)lpStack->Val.ValCell.u.lInfo, lpStack->Val.ValCell.Col);
            lpStack->Val.ValCell.Row = CoordMakeAbs(lpStack->Val.ValCell.Row, 0);
            Col_RemoveColListener(hPreced, hCol, -lpStack->Val.ValCell.Row);
          }
          else
          {
            CALC_HANDLE hPreced = CellLookupAlloc((LPCALC_SHEET)lpStack->Val.ValCell.u.lInfo, lpStack->Val.ValCell.Col, lpStack->Val.ValCell.Row);
            Cell_RemoveColListener(hPreced, hCol);
          }
        }
        else if (CALC_VALUE_TYPE_RANGE == lpStack->Type)
        {
          if (CoordIsRel(lpStack->Val.ValRange.Row1) &&
              CoordIsRel(lpStack->Val.ValRange.Row2))
          {
            lpStack->Val.ValRange.Row1 = CoordMakeAbs(lpStack->Val.ValRange.Row1, 0);
            lpStack->Val.ValRange.Row2 = CoordMakeAbs(lpStack->Val.ValRange.Row2, 0);
            if (ShouldUseRange(lpStack->Val.ValRange.Col1, lpStack->Val.ValRange.Row1, lpStack->Val.ValRange.Col2, lpStack->Val.ValRange.Row2))
            {
              Sheet_ColRemoveColListener((LPCALC_SHEET)lpStack->Val.ValRange.u.lInfo, lpStack->Val.ValRange.Col1, lpStack->Val.ValRange.Col2, hCol, -lpStack->Val.ValRange.Row2, -lpStack->Val.ValRange.Row1);
            }
            else
            {
              for (i = lpStack->Val.ValRange.Col1; i <= lpStack->Val.ValRange.Col2; i++)
              {
                for (j = lpStack->Val.ValRange.Row1; j <= lpStack->Val.ValRange.Row2; j++)
                {
                  CALC_HANDLE hPreced = ColLookupAlloc((LPCALC_SHEET)lpStack->Val.ValRange.u.lInfo, i);
                  Col_RemoveColListener(hPreced, hCol, -j);
                }
              }
            }
          }
          else if (CoordIsAbs(lpStack->Val.ValRange.Row1) &&
                   CoordIsAbs(lpStack->Val.ValRange.Row2))
          {
            if (ShouldUseRange(lpStack->Val.ValRange.Col1, lpStack->Val.ValRange.Row1, lpStack->Val.ValRange.Col2, lpStack->Val.ValRange.Row2))
            {
              Sheet_CellRemoveColListener((LPCALC_SHEET)lpStack->Val.ValRange.u.lInfo, lpStack->Val.ValRange.Col1, lpStack->Val.ValRange.Row1, lpStack->Val.ValRange.Col2, lpStack->Val.ValRange.Row2, hCol);
            }
            else
            {
              for (i = lpStack->Val.ValRange.Col1; i <= lpStack->Val.ValRange.Col2; i++)
              {
                for (j = lpStack->Val.ValRange.Row1; j <= lpStack->Val.ValRange.Row2; j++)
                {
                  CALC_HANDLE hPreced = CellLookupAlloc((LPCALC_SHEET)lpStack->Val.ValRange.u.lInfo, i, j);
                  Cell_RemoveColListener(hPreced, hCol);
                }
              }
            }
          }
        }
      }
    }
    ValStackFree(&stack);
    Expr_MemUnlock(hThis);
  }
}

//--------------------------------------------------------------------

void Expr_NameStartListening(CALC_HANDLE hThis, CALC_HANDLE hName)
{
  LPCALC_EXPRINFO lpThis;

  if (hThis && (lpThis = Expr_MemLock(hThis)))
  {
    for (; lpThis->nType != CALC_EXPR_END; lpThis++)
    {
      if (lpThis->nType == CALC_EXPR_NAME)
        Name_AddNameListener(lpThis->u.CustName.hName, hName);
    }
    Expr_MemUnlock(hThis);
  }
}

//--------------------------------------------------------------------

void Expr_NameStopListening(CALC_HANDLE hThis, CALC_HANDLE hName)
{
  LPCALC_EXPRINFO lpThis;

  if (hThis && (lpThis = Expr_MemLock(hThis)))
  {
    for (; lpThis->nType != CALC_EXPR_END; lpThis++)
    {
      if (lpThis->nType == CALC_EXPR_NAME)
        Name_RemoveNameListener(lpThis->u.CustName.hName, hName);
    }
    Expr_MemUnlock(hThis);
  }
}

//--------------------------------------------------------------------

#endif  // defined(SS_V70)

#if SS_V80 // 24477 -scl
BOOL ExprIsSubtotal(CALC_HANDLE hExpr)
{
  LPCALC_EXPRINFO lpExpr;
  BOOL bResult = FALSE;
  if( hExpr && (lpExpr = Expr_MemLock(hExpr)) )
  {
    for( lpExpr++; !bResult && lpExpr->nType != CALC_EXPR_END; lpExpr++ )
    {
      switch (lpExpr->nType)
      {
        case CALC_EXPR_NAME:
          if( ExprIsSubtotal(NameGetExpr(lpExpr->u.CustName.hName)) )
            bResult = TRUE;
          break;
        case CALC_EXPR_FUNCTION:
          if( lpExpr->u.Func.nId == CALC_FUNC_SUBTOTAL )
            bResult = TRUE;
          break;
      }
    } 
    Expr_MemUnlock(hExpr);
  }
  return bResult;
}
#endif
//--------------------------------------------------------------------
