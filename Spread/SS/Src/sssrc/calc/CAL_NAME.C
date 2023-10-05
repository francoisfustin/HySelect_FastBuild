//--------------------------------------------------------------------
//
//  File: cal_name.c
//
//  Description: Routines to handle named expressions
//

#include <ctype.h>
#include <string.h>
#include <locale.h>

#include "calc.h"
#include "cal_cell.h"
#include "cal_col.h"
#include "cal_expr.h"
#include "cal_mem.h"
#include "cal_name.h"
#include "cal_row.h"

//--------------------------------------------------------------------
//
//  The NameInit() function initializes a named expression table.
//

BOOL NameInit(LPCALC_NAMETABLE lpTable)
{
  lpTable->hElem = 0;
  lpTable->nElemCnt = 0;
  lpTable->nAllocCnt = 0;
  lpTable->nAllocInc = 5;
  return TRUE;
}

//--------------------------------------------------------------------
//
//  The NameFree() function frees all memory associated with the
//  table.  If bDestroy == TRUE then the individual entries are
//  also freed.
//

BOOL NameFree(LPCALC_NAMETABLE lpTable)
{
  LPCALC_HANDLE lpElem;
  int i;
  
  if( lpTable->hElem )
  {
    if( lpElem = (LPCALC_HANDLE)CalcMemLock(lpTable->hElem) )
    {
      for( i = 0; i < lpTable->nElemCnt; i++ )
      {
        NameDecTableRefCnt(lpElem[i]);
        NameDestroy(lpElem[i]);
      }
      CalcMemUnlock(lpTable->hElem);
    }
    CalcMemFree(lpTable->hElem);
  }
  lpTable->hElem = 0;
  lpTable->nElemCnt = 0;
  lpTable->nAllocCnt = 0;
  return TRUE;
}

//--------------------------------------------------------------------
//
//  The Search() function uses a binary search to find the desired
//  named expression in the array of named expression handles.  If
//  found, the function returns TRUE and *lpnPos contains the
//  position of the named expression in the array.  If not found,
//  the function returns FALSE and *lpnPos contains the position
//  to insert the name.
//
//  Note: It is assumed that the array is sorted in ascending order
//        by function name.
//

static BOOL Search(LPCTSTR lpszName, LPCALC_HANDLE lpElem, int nElemCnt,
                   int FAR* lpnPos)
{
  LPCALC_NAME lpName;
  LPTSTR lpszText;
  int nFirst = 0;
  int nLast = nElemCnt - 1;
  int nMiddle = (nFirst + nLast) / 2;
  int iCmp = 1;
  
  while( iCmp != 0 && nFirst <= nLast )
  {
    if( lpElem[nMiddle] && (lpName = Name_MemLock(lpElem[nMiddle])) )
    {
      if( lpszText = (LPTSTR)CalcMemLock(lpName->hText) )
      {
        iCmp = lstrcmpi(lpszName, lpszText);
        if( iCmp < 0 )
          nLast = nMiddle - 1;  
        else if( iCmp > 0 )
          nFirst = nMiddle + 1;
        CalcMemUnlock(lpName->hText);
      }
      else  // failed to lock expression name
        nLast = nMiddle - 1;
      Name_MemUnlock(lpElem[nMiddle]);
    }
    else  // failed to lock expression
      nLast = nMiddle - 1;
    nMiddle = (nFirst + nLast) / 2;    
  }
  *lpnPos = iCmp == 0 ? nMiddle : nFirst;
  return iCmp == 0;
}

//--------------------------------------------------------------------
//
//  The NameAdd() function adds a named expression to the table.
//  If the name already existed, then the table is unmodified.  If
//  needed, the size of the array of handles is expanded.  If
//  successful, the function returns TRUE.  Otherwise, the function
//  returns FALSE.
//

BOOL NameAdd(LPCALC_NAMETABLE lpTable, CALC_HANDLE hName)
{
  LPCALC_HANDLE lpElem;
  LPCALC_NAME lpName;
  LPTSTR lpszName;
  BOOL bReturn = FALSE;
  int nPos;
  int i;
  
  if( lpTable->nElemCnt >=  lpTable->nAllocCnt )
  {
    lpTable->nAllocCnt += lpTable->nAllocInc;
    if( lpTable->hElem )
      lpTable->hElem = CalcMemReAlloc(lpTable->hElem,
                                      lpTable->nAllocCnt*sizeof(CALC_HANDLE));
    else
      lpTable->hElem = CalcMemAlloc(lpTable->nAllocCnt*sizeof(CALC_HANDLE));
    if( !lpTable->hElem )
    {
      lpTable->nElemCnt = 0;
      lpTable->nAllocCnt = 0;
    }
  }
  if( lpTable->hElem && (lpElem = (LPCALC_HANDLE)CalcMemLock(lpTable->hElem)) )
  {
    if( hName && (lpName = Name_MemLock(hName)) )
    {
      if( lpName->hText && (lpszName = (LPTSTR)CalcMemLock(lpName->hText)) )
      {
        if( !Search(lpszName, lpElem, lpTable->nElemCnt, &nPos) )
        {
          if( hName = NameCreateRef(hName) )
          {
            NameIncTableRefCnt(hName);
            for( i = lpTable->nElemCnt; i > nPos; i-- )
              lpElem[i] = lpElem[i-1];
            lpElem[nPos] = hName;
            lpTable->nElemCnt++;
            bReturn = TRUE;
          }
        }   
        CalcMemUnlock(lpName->hText);
      }
      Name_MemUnlock(hName);
    }
    CalcMemUnlock(lpTable->hElem);
  }
  return bReturn;
}

//--------------------------------------------------------------------
//
//  The NameRemove() function removes a named expression entry
//  from the given table.  If successful, the function returns TRUE.
//  Otherwise, the function returns FALSE.
//

BOOL NameRemove(LPCALC_NAMETABLE lpTable, CALC_HANDLE hName)
{
  LPCALC_HANDLE lpElem;
  BOOL bReturn = FALSE;
  int i;
  
  if( lpTable->hElem && (lpElem = (LPCALC_HANDLE)CalcMemLock(lpTable->hElem)) )
  {
    for( i = 0; i < lpTable->nElemCnt; i++ )
    {
      if( hName == lpElem[i] )
      {
        NameDecTableRefCnt(hName);
        NameDestroy(hName);
        lpTable->nElemCnt--;
        for( ; i < lpTable->nElemCnt; i++ )
          lpElem[i] = lpElem[i+1];
        bReturn = TRUE;
      }
    }
    CalcMemUnlock(lpTable->hElem);
  }
  return bReturn;
}

//--------------------------------------------------------------------
//
//  The NameLookup() function retrieves a named expression from
//  the given table.  If found, the function returns a handle to the
//  desired named expression.  If not found, the function returns NULL.
//

CALC_HANDLE NameLookup(LPCALC_NAMETABLE lpTable, LPCTSTR lpszName)
{
  LPCALC_HANDLE lpElem;
  CALC_HANDLE hReturn = 0;
  int nPos;
  
  if( lpTable->hElem && (lpElem = (LPCALC_HANDLE)CalcMemLock(lpTable->hElem)) )
  {
    if( Search(lpszName, lpElem, lpTable->nElemCnt, &nPos) )
      hReturn = lpElem[nPos];
    CalcMemUnlock(lpTable->hElem);
  }
  return hReturn;
}

//--------------------------------------------------------------------
//
//  The NameFirst() function retrieves the first named expression
//  in the given table.  If successful, the function returns a handle
//  to the desired custom name.  Otherwise, the function returns NULL.
//

CALC_HANDLE NameFirst(LPCALC_NAMETABLE lpTable)
{
  LPCALC_HANDLE lpElem;
  CALC_HANDLE hReturn = 0;
  
  if( lpTable->hElem
      && lpTable->nElemCnt > 0
      && (lpElem = (LPCALC_HANDLE)CalcMemLock(lpTable->hElem)) )
  {
    hReturn = lpElem[0];
    CalcMemUnlock(lpTable->hElem);
  }
  return hReturn;
}

//--------------------------------------------------------------------
//
//  The NameNext() function retrieves the next named expression
//  in the given table.  If successful, the function returns a handle
//  to the desired custom name.  Otherwise, the function returns NULL.
//

CALC_HANDLE NameNext(LPCALC_NAMETABLE lpTable, LPCTSTR lpszName)
{
  LPCALC_HANDLE lpElem;
  CALC_HANDLE hReturn = 0;
  int nPos;
  
  if( lpTable->hElem && (lpElem = (LPCALC_HANDLE)CalcMemLock(lpTable->hElem)) )
  {
    if( Search(lpszName, lpElem, lpTable->nElemCnt, &nPos) )
      nPos++;
    if( nPos < lpTable->nElemCnt )
      hReturn = lpElem[nPos];
    CalcMemUnlock(lpTable->hElem);
  }
  return hReturn;
}

//--------------------------------------------------------------------
//
//  The NameIsValidName() function determines whether or not the
//  given name is valid.  The first character must be a letter
//  or an underscore.  Remaining characters must be a letter, a
//  digit, or an underscore. 
//

BOOL NameIsValidName(LPCTSTR lpszName)
{
  BOOL bRet = FALSE;


  if( '_' == *lpszName || _istalpha((_TUCHAR)*lpszName) )
  {
    for( bRet = TRUE; bRet && *lpszName ; lpszName++)
      bRet = '_' == *lpszName || '.' == *lpszName || _istalnum((_TUCHAR)*lpszName);
  }
  return bRet;
}

//--------------------------------------------------------------------
//
//  The NameCreate() function creates a named expression.  If
//  successful, the function returns a handle to the named expresison.
//  Otherwise, the function returns NULL.
//

CALC_HANDLE NameCreate(LPCALC_BOOK lpBook, LPCTSTR lpszName, CALC_HANDLE hExpr)
{
  LPCALC_NAME lpName;
  CALC_HANDLE hName;
  CALC_HANDLE hReturn = 0;
  LPTSTR lpszText;

  if( lpszName && lpszName[0] && NameIsValidName(lpszName) )
  {
    hName = Name_MemAlloc();
    if( hName && (lpName = Name_MemLock(hName)) )
    {
      lpName->hText = CalcMemAlloc((lstrlen(lpszName)+1) * sizeof(TCHAR));
      if( lpName->hText && (lpszText = (LPTSTR)CalcMemLock(lpName->hText)) )
      {
        lstrcpy(lpszText, lpszName);
        lpName->lpBook = lpBook;
        lpName->hExpr = hExpr;
        lpName->lRefCnt = 1;
        lpName->lTableRefCnt = 0;
        #if defined(SS_V70)
        lpName->hCellListeners = NULL;
        lpName->hRowListeners = NULL;
        lpName->hColListeners = NULL;
        lpName->hNameListeners = NULL;
        #else
        NameInit(&lpName->DependNames);
        BitMatInit(&lpName->DependCells);
        BitVectInit(&lpName->DependCols);
        BitVectInit(&lpName->DependRows);
        #endif
        ExprSetLinkToName(hExpr, hName, TRUE);
        hReturn = hName;
        CalcMemUnlock(lpName->hText);
      }
      Name_MemUnlock(hName);
    }
  }
  return hReturn;
}

//--------------------------------------------------------------------
//
//  The NameCreateRef() function increments the reference counter
//  for the named expression.  If successful, the function returns
//  the handle to the named expression.  Otherwise, the function
//  return NULL.
//

CALC_HANDLE NameCreateRef(CALC_HANDLE hName)
{
  LPCALC_NAME lpName;
  CALC_HANDLE hReturn = 0;
 
  if( hName && (lpName = Name_MemLock(hName)) )
  {
    lpName->lRefCnt++;
    hReturn = hName;
    Name_MemUnlock(hName);
  }
  return hReturn;
}

//--------------------------------------------------------------------
//
//  The NameDestroy() function destroys a custom name .  If the
//  custom name has references, then only the reference counter
//  is decremented.  If the custom name has no refernces, then
//  all memory associated with the custom name is freed.
//

BOOL NameDestroy(CALC_HANDLE hName)
{
  LPCALC_NAME lpName;
  BOOL bReturn = FALSE;
  BOOL bNoReferences = FALSE;
  
  if( hName && (lpName = Name_MemLock(hName)) )
  {
    lpName->lRefCnt--;
    if( bNoReferences = lpName->lRefCnt == 0 )
    {
      ExprSetLinkToName(lpName->hExpr, hName, FALSE);
      CalcMemFree(lpName->hText);
      ExprDestroy(lpName->hExpr);
      #if !defined(SS_V70)
      NameFree(&lpName->DependNames);
      BitMatFree(&lpName->DependCells);
      BitVectFree(&lpName->DependCols);
      BitVectFree(&lpName->DependRows);
      #endif
      bReturn = TRUE;
    }
    Name_MemUnlock(hName);
  }
  if( bNoReferences )
    Name_MemFree(hName);
  return bReturn;
}

//--------------------------------------------------------------------

LPCALC_BOOK Name_GetBook(CALC_HANDLE hThis)
{
  LPCALC_NAME lpThis;
  LPCALC_BOOK lpValue = NULL;

  if (hThis && (lpThis = Name_MemLock(hThis)))
  {
    lpValue = lpThis->lpBook;
    Name_MemUnlock(hThis);
  }
  return lpValue;
}

//--------------------------------------------------------------------
//
//  The NameGetText() function retrieves the text representation of
//  a named expression.  If successful, the function returns the
//  length of the copied string, not including the terminating NULL
//  character.  Otherwise, the function returns zero.
//

int NameGetText(CALC_HANDLE hName, LPTSTR lpszText, int nLen)
{
  LPCALC_NAME lpName;
  LPTSTR lpszName;
  int nNameLen;
  int nResult = 0;
  
  if( hName && (lpName = Name_MemLock(hName)) )
  {
    if( lpName->hText && (lpszName = (LPTSTR)CalcMemLock(lpName->hText)) )
    {
      nNameLen = lstrlen(lpszName);
      if( nNameLen < nLen )
      {
        lstrcpy(lpszText, lpszName);
        nResult = nNameLen;
      }
      CalcMemUnlock(lpName->hText);
    }
    Name_MemUnlock(hName);
  }
  if( 0 == nResult && nLen > 0 )
    lpszText[0] = 0;
  return nResult;
}

//--------------------------------------------------------------------
//
//  The NameGetTextLen() function retrieves the length of the text
//  representation of a named expression.
//

int NameGetTextLen(CALC_HANDLE hName)
{
  LPCALC_NAME lpName;
  LPTSTR lpszName;
  int iResult = 0;
  
  if( hName && (lpName = Name_MemLock(hName)) )
  {
    if( lpName->hText && (lpszName = (LPTSTR)CalcMemLock(lpName->hText)) )
    {
      iResult = lstrlen(lpszName);
      CalcMemUnlock(lpName->hText);
    }
    Name_MemUnlock(hName);
  }
  return iResult;
}

//--------------------------------------------------------------------
//
//  The NameGetRefCnt() function retrieves the reference count for
//  the named expression.
//

long NameGetRefCnt(CALC_HANDLE hName)
{
  LPCALC_NAME lpName;
  long lReturn = 0;
  
  if( hName && (lpName = Name_MemLock(hName)) )
  {
    lReturn = lpName->lRefCnt;
    Name_MemUnlock(hName);
  }
  return lReturn;
}

//--------------------------------------------------------------------

BOOL NameIncTableRefCnt(CALC_HANDLE hName)
{
  LPCALC_NAME lpName;
  BOOL bRet = FALSE;
  
  if( hName && (lpName = Name_MemLock(hName)) )
  {
    lpName->lTableRefCnt++;
    bRet = TRUE;
    Name_MemUnlock(hName);
  }
  return bRet;
}

//--------------------------------------------------------------------

BOOL NameDecTableRefCnt(CALC_HANDLE hName)
{
  LPCALC_NAME lpName;
  BOOL bRet = FALSE;
  
  if( hName && (lpName = Name_MemLock(hName)) )
  {
    lpName->lTableRefCnt--;
    bRet = TRUE;
    Name_MemUnlock(hName);
  }
  return bRet;
}

//--------------------------------------------------------------------

long NameGetTableRefCnt(CALC_HANDLE hName)
{
  LPCALC_NAME lpName;
  long lRet = 0;
  
  if( hName && (lpName = Name_MemLock(hName)) )
  {
    lRet = lpName->lTableRefCnt;
    Name_MemUnlock(hName);
  }
  return lRet;
}

//--------------------------------------------------------------------
//
//  The NameGetExpr() function retrieves the expression represented
//  by the named expression.  If successful, the function returns a
//  handle to the expression.  Otherwise, the function returns NULL.
//

CALC_HANDLE NameGetExpr(CALC_HANDLE hName)
{
  LPCALC_NAME lpName;
  CALC_HANDLE hResult = 0;
  
  if( hName && (lpName = Name_MemLock(hName)) )
  {
    hResult = lpName->hExpr;
    Name_MemUnlock(hName);
  }
  return hResult;
}

//--------------------------------------------------------------------
//
//  The NameSetExpr() function assigns the expression represented
//  by the named expression.  If successful, the function returns
//  TRUE.  Otherwise, the function returns FALSE.

BOOL NameSetExpr(CALC_HANDLE hName, CALC_HANDLE hExpr)
{
  LPCALC_NAME lpName;
  BOOL bReturn = FALSE;

  if( ExprContainsName(hExpr, hName) )
  {
    ExprDestroy(hExpr);
  }
  else if( hName && (lpName = Name_MemLock(hName)) )
  {
    ExprSetLinkToName(lpName->hExpr, hName, FALSE);
    ExprDestroy(lpName->hExpr);
    lpName->hExpr = hExpr;
    ExprSetLinkToName(lpName->hExpr, hName, TRUE);
    bReturn = TRUE;
    Name_MemUnlock(hName);
  }
  return bReturn;
}

//--------------------------------------------------------------------
//
//  The NameSetLinkToCell() function adds or removes a data link from
//  the named expression to the cell (i.e. the cell is dependent on
//  the named expression).  This data link allows updates to the name
//  to be reflected in the cell.
//

#if !defined(SS_V70)
BOOL NameSetLinkToCell(CALC_HANDLE hName, long lCol, long lRow, BOOL bLink)
{
  LPCALC_NAME lpName;
  BOOL bReturn = FALSE;
  
  if( hName && (lpName = Name_MemLock(hName)) )
  {
    BitMatSet(&lpName->DependCells, lCol, lRow, bLink);
    bReturn = TRUE;
    Name_MemUnlock(hName);
  }
  return bReturn;
}
#endif

//--------------------------------------------------------------------
//
//  The NameSetLinkToCol() function adds or removes a data link from
//  the named expression to the column (i.e. the column is dependent
//  on the named expression).  This data link allows updates to the
//  name to be reflected in the column.
//

#if !defined(SS_V70)
BOOL NameSetLinkToCol(CALC_HANDLE hName, long lCol, BOOL bLink)
{
  LPCALC_NAME lpName;
  BOOL bReturn = FALSE;
  
  if( hName && (lpName = Name_MemLock(hName)) )
  {
    BitVectSet(&lpName->DependCols, lCol, bLink);
    bReturn = TRUE;
    Name_MemUnlock(hName);
  }
  return bReturn;
}
#endif

//--------------------------------------------------------------------
//
//  The NameSetLinkToRow() function adds or removes a data link from
//  the named expression to the row (i.e. the row is dependent on the
//  named expression).  This data link allows updates to the name to
//  be reflected in the row.
//

#if !defined(SS_V70)
BOOL NameSetLinkToRow(CALC_HANDLE hName, long lRow, BOOL bLink)
{
  LPCALC_NAME lpName;
  BOOL bReturn = FALSE;
  
  if( hName && (lpName = Name_MemLock(hName)) )
  {
    BitVectSet(&lpName->DependRows, lRow, bLink);
    bReturn = TRUE;
    Name_MemUnlock(hName);
  }
  return bReturn;
}
#endif

//--------------------------------------------------------------------
//
//  The NameSetLinkToName() function adds or reomves a data link
//  between the two named expression (i.e. hDepend is dependent
//  on hName).  This data link allows updates to hName to be
//  reflected in hDepend.
//

#if !defined(SS_V70)
BOOL NameSetLinkToName(CALC_HANDLE hName, CALC_HANDLE hDepend, BOOL bLink)
{
  LPCALC_NAME lpName;
  BOOL bReturn = FALSE;
  
  if( hName && (lpName = Name_MemLock(hName)) )
  {
    if( bLink )
      bReturn = NameAdd(&lpName->DependNames, hDepend);
    else
      bReturn = NameRemove(&lpName->DependNames, hDepend);
    Name_MemUnlock(hName);
  }
  return bReturn;
}
#endif

//--------------------------------------------------------------------
//
//  The NameMarkDepend() function records (in lpDependCells,
//  lpDependCols, and lpDependRows) all cells, columns, and rows
//  which depend (directly or indirectly) on the given named
//  expression for data.
//

#if !defined(SS_V70)
BOOL NameMarkDepend(CALC_HANDLE hName, LPBITMATRIX lpDependCells,
                    LPBITVECTOR lpDependCols, LPBITVECTOR lpDependRows)
{
  LPCALC_NAME lpName;
  LPCALC_HANDLE lpElem;
  BOOL bReturn = TRUE;
  int i;
  
  if( hName && (lpName = Name_MemLock(hName)) )
  {
    BitMatCopy(&lpName->DependCells, lpDependCells);
    BitVectCopy(&lpName->DependCols, lpDependCols);
    BitVectCopy(&lpName->DependRows, lpDependRows);
    if( lpName->DependNames.hElem
        && (lpElem = (LPCALC_HANDLE)CalcMemLock(lpName->DependNames.hElem)) )
    {
      for( i = 0; i < lpName->DependNames.nElemCnt; i++ )
        NameMarkDepend(lpElem[i], lpDependCells, lpDependCols, lpDependRows);
      CalcMemUnlock(lpName->DependNames.hElem);
    }
    bReturn = TRUE;
    Name_MemUnlock(hName);
  }
  return bReturn;
}
#endif

//--------------------------------------------------------------------

#if defined(SS_V70)

//--------------------------------------------------------------------

void Name_StartListeners(CALC_HANDLE hThis)
{
  LPCALC_NAME lpThis;
  CALC_HANDLE hList;

  if (hThis && (lpThis = Name_MemLock(hThis)))
  {
    for (hList = lpThis->hCellListeners; hList != NULL; hList = Name_CellListenerList_GetNext(hList))
      Cell_StartListening(Name_CellListenerList_GetCell(hList), FALSE);
    for (hList = lpThis->hRowListeners; hList != NULL; hList = Name_RowListenerList_GetNext(hList))
      Row_StartListening(Name_RowListenerList_GetRow(hList), FALSE);
    for (hList = lpThis->hColListeners; hList != NULL; hList = Name_ColListenerList_GetNext(hList))
      Col_StartListening(Name_ColListenerList_GetCol(hList), FALSE);
    for (hList = lpThis->hNameListeners; hList != NULL; hList = Name_NameListenerList_GetNext(hList))
      Name_StartListeners(Name_NameListenerList_GetName(hList));
    Name_MemUnlock(hThis);
  }
}

//--------------------------------------------------------------------

void Name_StopListeners(CALC_HANDLE hThis)
{
  LPCALC_NAME lpThis;
  CALC_HANDLE hList;

  if (hThis && (lpThis = Name_MemLock(hThis)))
  {
    for (hList = lpThis->hCellListeners; hList != NULL; hList = Name_CellListenerList_GetNext(hList))
      Cell_StopListening(Name_CellListenerList_GetCell(hList), FALSE);
    for (hList = lpThis->hRowListeners; hList != NULL; hList = Name_RowListenerList_GetNext(hList))
      Row_StopListening(Name_RowListenerList_GetRow(hList), FALSE);
    for (hList = lpThis->hColListeners; hList != NULL; hList = Name_ColListenerList_GetNext(hList))
      Col_StopListening(Name_ColListenerList_GetCol(hList), FALSE);
    for (hList = lpThis->hNameListeners; hList != NULL; hList = Name_NameListenerList_GetNext(hList))
      Name_StopListeners(Name_NameListenerList_GetName(hList));
    Name_MemUnlock(hThis);
  }
}

//--------------------------------------------------------------------

void Name_AddCellListener(CALC_HANDLE hThis, CALC_HANDLE hCell)
{
  LPCALC_NAME lpThis;
  CALC_HANDLE hList;

  if (hThis && (lpThis = Name_MemLock(hThis)))
  {
    for (hList = lpThis->hCellListeners; hList; hList = Name_CellListenerList_GetNext(hList))
    {
      if (hCell == Name_CellListenerList_GetCell(hList))
        break;
    }
    if (!hList)
      lpThis->hCellListeners = Name_CellListenerList_Create(hCell, lpThis->hCellListeners);
    Name_MemUnlock(hThis);
  }
}

//--------------------------------------------------------------------

void Name_RemoveCellListener(CALC_HANDLE hThis, CALC_HANDLE hCell)
{
  LPCALC_NAME lpThis;
  CALC_HANDLE hList;
  CALC_HANDLE hPrev;

  if (hThis && (lpThis = Name_MemLock(hThis)))
  {
    for (hPrev = NULL, hList = lpThis->hCellListeners; hList; hPrev = hList, hList = Name_CellListenerList_GetNext(hList))
    {
      if (hCell == Name_CellListenerList_GetCell(hList))
        break;
    }
    if (hList)
    {
      if (hPrev)
        Name_CellListenerList_SetNext(hPrev, Name_CellListenerList_GetNext(hList));
      else
        lpThis->hCellListeners = Name_CellListenerList_GetNext(hList);
      Name_CellListenerList_Destroy(hList);
    }
    Name_MemUnlock(hThis);
  }
}

//--------------------------------------------------------------------

void Name_AddRowListener(CALC_HANDLE hThis, CALC_HANDLE hRow)
{
  LPCALC_NAME lpThis;
  CALC_HANDLE hList;

  if (hThis && (lpThis = Name_MemLock(hThis)))
  {
    for (hList = lpThis->hRowListeners; hList; hList = Name_RowListenerList_GetNext(hList))
    {
      if (hRow == Name_RowListenerList_GetRow(hList))
        break;
    }
    if (!hList)
      lpThis->hRowListeners = Name_RowListenerList_Create(hRow, lpThis->hRowListeners);
    Name_MemUnlock(hThis);
  }
}

//--------------------------------------------------------------------

void Name_RemoveRowListener(CALC_HANDLE hThis, CALC_HANDLE hRow)
{
  LPCALC_NAME lpThis;
  CALC_HANDLE hList;
  CALC_HANDLE hPrev;

  if (hThis && (lpThis = Name_MemLock(hThis)))
  {
    for (hPrev = NULL, hList = lpThis->hRowListeners; hList; hPrev = hList, hList = Name_RowListenerList_GetNext(hList))
    {
      if (hRow == Name_RowListenerList_GetRow(hList))
        break;
    }
    if (hList)
    {
      if (hPrev)
        Name_RowListenerList_SetNext(hPrev, Name_RowListenerList_GetNext(hList));
      else
        lpThis->hCellListeners = Name_RowListenerList_GetNext(hList);
      Name_RowListenerList_Destroy(hList);
    }
    Name_MemUnlock(hThis);
  }
}

//--------------------------------------------------------------------

void Name_AddColListener(CALC_HANDLE hThis, CALC_HANDLE hCol)
{
  LPCALC_NAME lpThis;
  CALC_HANDLE hList;

  if (hThis && (lpThis = Name_MemLock(hThis)))
  {
    for (hList = lpThis->hColListeners; hList; hList = Name_ColListenerList_GetNext(hList))
    {
      if (hCol == Name_ColListenerList_GetCol(hList))
        break;
    }
    if (!hList)
      lpThis->hColListeners = Name_ColListenerList_Create(hCol, lpThis->hColListeners);
    Name_MemUnlock(hThis);
  }
}

//--------------------------------------------------------------------

void Name_RemoveColListener(CALC_HANDLE hThis, CALC_HANDLE hCol)
{
  LPCALC_NAME lpThis;
  CALC_HANDLE hList;
  CALC_HANDLE hPrev;

  if (hThis && (lpThis = Name_MemLock(hThis)))
  {
    for (hPrev = NULL, hList = lpThis->hColListeners; hList; hPrev = hList, hList = Name_ColListenerList_GetNext(hList))
    {
      if (hCol == Name_ColListenerList_GetCol(hList))
        break;
    }
    if (hList)
    {
      if (hPrev)
        Name_ColListenerList_SetNext(hPrev, Name_ColListenerList_GetNext(hList));
      else
        lpThis->hColListeners = Name_ColListenerList_GetNext(hList);
      Name_ColListenerList_Destroy(hList);
    }
    Name_MemUnlock(hThis);
  }
}

//--------------------------------------------------------------------

void Name_AddNameListener(CALC_HANDLE hThis, CALC_HANDLE hName)
{
  LPCALC_NAME lpThis;
  CALC_HANDLE hList;

  if (hThis && (lpThis = Name_MemLock(hThis)))
  {
    for (hList = lpThis->hNameListeners; hList; hList = Name_NameListenerList_GetNext(hList))
    {
      if (hName == Name_NameListenerList_GetName(hList))
        break;
    }
    if (!hList)
      lpThis->hNameListeners = Name_NameListenerList_Create(hName, lpThis->hNameListeners);
    Name_MemUnlock(hThis);
  }
}

//--------------------------------------------------------------------

void Name_RemoveNameListener(CALC_HANDLE hThis, CALC_HANDLE hName)
{
  LPCALC_NAME lpThis;
  CALC_HANDLE hList;
  CALC_HANDLE hPrev;

  if (hThis && (lpThis = Name_MemLock(hThis)))
  {
    for (hPrev = NULL, hList = lpThis->hNameListeners; hList; hPrev = hList, hList = Name_NameListenerList_GetNext(hList))
    {
      if (hName == Name_NameListenerList_GetName(hList))
        break;
    }
    if (hList)
    {
      if (hPrev)
        Name_NameListenerList_SetNext(hPrev, Name_NameListenerList_GetNext(hList));
      else
        lpThis->hNameListeners = Name_NameListenerList_GetNext(hList);
      Name_NameListenerList_Destroy(hList);
    }
    Name_MemUnlock(hThis);
  }
}

//--------------------------------------------------------------------

void Name_AddListenersToDirty(CALC_HANDLE hThis)
{
  LPCALC_NAME lpThis;
  LPCALC_BOOK lpBook;
  CALC_HANDLE hList;

  if (hThis && (lpThis = Name_MemLock(hThis)))
  {
    lpBook = lpThis->lpBook;
    for (hList = lpThis->hCellListeners; hList != NULL; hList = Name_CellListenerList_GetNext(hList))
      Book_AddDirtyCell(lpBook, Name_CellListenerList_GetCell(hList));
    for (hList = lpThis->hRowListeners; hList != NULL; hList = Name_RowListenerList_GetNext(hList))
      Book_AddDirtyRow(lpBook, Name_RowListenerList_GetRow(hList));
    for (hList = lpThis->hColListeners; hList != NULL; hList = Name_ColListenerList_GetNext(hList))
      Book_AddDirtyCol(lpBook, Name_ColListenerList_GetCol(hList));
    for (hList = lpThis->hNameListeners; hList != NULL; hList = Name_NameListenerList_GetNext(hList))
      Name_AddListenersToDirty(Name_NameListenerList_GetName(hList));
    Name_MemUnlock(hThis);
  }
}

//--------------------------------------------------------------------

CALC_HANDLE Name_CellListenerList_Create(CALC_HANDLE hCell, CALC_HANDLE hNext)
{
  LPCALC_NAME_CELLLISTENERLIST lpThis;
  CALC_HANDLE hThis;

  hThis = Name_CellListenerList_MemAlloc();
  if (hThis && (lpThis = Name_CellListenerList_MemLock(hThis)))
  {
    lpThis->hCell = hCell;
    lpThis->hNext = hNext;
    Name_CellListenerList_MemUnlock(hThis);
  }
  return hThis;
}

//--------------------------------------------------------------------

void Name_CellListenerList_Destroy(CALC_HANDLE hThis)
{
  LPCALC_NAME_CELLLISTENERLIST lpThis;

  if (hThis && (lpThis = Name_CellListenerList_MemLock(hThis)))
  {
    Name_CellListenerList_MemUnlock(hThis);
    Name_CellListenerList_MemFree(hThis);
  }
}

//--------------------------------------------------------------------

CALC_HANDLE Name_CellListenerList_GetCell(CALC_HANDLE hThis)
{
  LPCALC_NAME_CELLLISTENERLIST lpThis;
  CALC_HANDLE hValue = NULL;

  if (hThis && (lpThis = Name_CellListenerList_MemLock(hThis)))
  {
    hValue = lpThis->hCell;
    Name_CellListenerList_MemUnlock(hThis);
  }
  return hValue;
}

//--------------------------------------------------------------------

CALC_HANDLE Name_CellListenerList_GetNext(CALC_HANDLE hThis)
{
  LPCALC_NAME_CELLLISTENERLIST lpThis;
  CALC_HANDLE hValue = NULL;

  if (hThis && (lpThis = Name_CellListenerList_MemLock(hThis)))
  {
    hValue = lpThis->hNext;
    Name_CellListenerList_MemUnlock(hThis);
  }
  return hValue;
}

//--------------------------------------------------------------------

void Name_CellListenerList_SetNext(CALC_HANDLE hThis, CALC_HANDLE hValue)
{
  LPCALC_NAME_CELLLISTENERLIST lpThis;

  if (hThis && (lpThis = Name_CellListenerList_MemLock(hThis)))
  {
    lpThis->hNext = hValue;
    Name_CellListenerList_MemUnlock(hThis);
  }
}

//--------------------------------------------------------------------

CALC_HANDLE Name_RowListenerList_Create(CALC_HANDLE hRow, CALC_HANDLE hNext)
{
  LPCALC_NAME_ROWLISTENERLIST lpThis;
  CALC_HANDLE hThis;

  hThis = Name_RowListenerList_MemAlloc();
  if (hThis && (lpThis = Name_RowListenerList_MemLock(hThis)))
  {
    lpThis->hRow = hRow;
    lpThis->hNext = hNext;
    Name_RowListenerList_MemUnlock(hThis);
  }
  return hThis;
}

//--------------------------------------------------------------------

void Name_RowListenerList_Destroy(CALC_HANDLE hThis)
{
  LPCALC_NAME_ROWLISTENERLIST lpThis;

  if (hThis && (lpThis = Name_RowListenerList_MemLock(hThis)))
  {
    Name_RowListenerList_MemUnlock(hThis);
    Name_RowListenerList_MemFree(hThis);
  }
}

//--------------------------------------------------------------------

CALC_HANDLE Name_RowListenerList_GetRow(CALC_HANDLE hThis)
{
  LPCALC_NAME_ROWLISTENERLIST lpThis;
  CALC_HANDLE hValue = NULL;

  if (hThis && (lpThis = Name_RowListenerList_MemLock(hThis)))
  {
    hValue = lpThis->hRow;
    Name_RowListenerList_MemUnlock(hThis);
  }
  return hValue;
}

//--------------------------------------------------------------------

CALC_HANDLE Name_RowListenerList_GetNext(CALC_HANDLE hThis)
{
  LPCALC_NAME_ROWLISTENERLIST lpThis;
  CALC_HANDLE hValue = NULL;

  if (hThis && (lpThis = Name_RowListenerList_MemLock(hThis)))
  {
    hValue = lpThis->hNext;
    Name_RowListenerList_MemUnlock(hThis);
  }
  return hValue;
}

//--------------------------------------------------------------------

void Name_RowListenerList_SetNext(CALC_HANDLE hThis, CALC_HANDLE hValue)
{
  LPCALC_NAME_ROWLISTENERLIST lpThis;

  if (hThis && (lpThis = Name_RowListenerList_MemLock(hThis)))
  {
    lpThis->hNext = hValue;
    Name_RowListenerList_MemUnlock(hThis);
  }
}

//--------------------------------------------------------------------

CALC_HANDLE Name_ColListenerList_Create(CALC_HANDLE hCol, CALC_HANDLE hNext)
{
  LPCALC_NAME_COLLISTENERLIST lpThis;
  CALC_HANDLE hThis;

  hThis = Name_ColListenerList_MemAlloc();
  if (hThis && (lpThis = Name_ColListenerList_MemLock(hThis)))
  {
    lpThis->hCol = hCol;
    lpThis->hNext = hNext;
    Name_ColListenerList_MemUnlock(hThis);
  }
  return hThis;
}

//--------------------------------------------------------------------

void Name_ColListenerList_Destroy(CALC_HANDLE hThis)
{
  LPCALC_NAME_COLLISTENERLIST lpThis;

  if (hThis && (lpThis = Name_ColListenerList_MemLock(hThis)))
  {
    Name_ColListenerList_MemUnlock(hThis);
    Name_ColListenerList_MemFree(hThis);
  }
}

//--------------------------------------------------------------------

CALC_HANDLE Name_ColListenerList_GetCol(CALC_HANDLE hThis)
{
  LPCALC_NAME_COLLISTENERLIST lpThis;
  CALC_HANDLE hValue = NULL;

  if (hThis && (lpThis = Name_ColListenerList_MemLock(hThis)))
  {
    hValue = lpThis->hCol;
    Name_ColListenerList_MemUnlock(hThis);
  }
  return hValue;
}

//--------------------------------------------------------------------

CALC_HANDLE Name_ColListenerList_GetNext(CALC_HANDLE hThis)
{
  LPCALC_NAME_COLLISTENERLIST lpThis;
  CALC_HANDLE hValue = NULL;

  if (hThis && (lpThis = Name_ColListenerList_MemLock(hThis)))
  {
    hValue = lpThis->hNext;
    Name_ColListenerList_MemUnlock(hThis);
  }
  return hValue;
}

//--------------------------------------------------------------------

void Name_ColListenerList_SetNext(CALC_HANDLE hThis, CALC_HANDLE hValue)
{
  LPCALC_NAME_COLLISTENERLIST lpThis;

  if (hThis && (lpThis = Name_ColListenerList_MemLock(hThis)))
  {
    lpThis->hNext = hValue;
    Name_ColListenerList_MemUnlock(hThis);
  }
}

//--------------------------------------------------------------------

CALC_HANDLE Name_NameListenerList_Create(CALC_HANDLE hName, CALC_HANDLE hNext)
{
  LPCALC_NAME_NAMELISTENERLIST lpThis;
  CALC_HANDLE hThis;

  hThis = Name_NameListenerList_MemAlloc();
  if (hThis && (lpThis = Name_NameListenerList_MemLock(hThis)))
  {
    lpThis->hName = hName;
    lpThis->hNext = hNext;
    Name_NameListenerList_MemUnlock(hThis);
  }
  return hThis;
}

//--------------------------------------------------------------------

void Name_NameListenerList_Destroy(CALC_HANDLE hThis)
{
  LPCALC_NAME_NAMELISTENERLIST lpThis;

  if (hThis && (lpThis = Name_NameListenerList_MemLock(hThis)))
  {
    Name_NameListenerList_MemUnlock(hThis);
    Name_NameListenerList_MemFree(hThis);
  }
}

//--------------------------------------------------------------------

CALC_HANDLE Name_NameListenerList_GetName(CALC_HANDLE hThis)
{
  LPCALC_NAME_NAMELISTENERLIST lpThis;
  CALC_HANDLE hValue = NULL;

  if (hThis && (lpThis = Name_NameListenerList_MemLock(hThis)))
  {
    hValue = lpThis->hName;
    Name_NameListenerList_MemUnlock(hThis);
  }
  return hValue;
}

//--------------------------------------------------------------------

CALC_HANDLE Name_NameListenerList_GetNext(CALC_HANDLE hThis)
{
  LPCALC_NAME_NAMELISTENERLIST lpThis;
  CALC_HANDLE hValue = NULL;

  if (hThis && (lpThis = Name_NameListenerList_MemLock(hThis)))
  {
    hValue = lpThis->hNext;
    Name_NameListenerList_MemUnlock(hThis);
  }
  return hValue;
}

//--------------------------------------------------------------------

void Name_NameListenerList_SetNext(CALC_HANDLE hThis, CALC_HANDLE hValue)
{
  LPCALC_NAME_NAMELISTENERLIST lpThis;

  if (hThis && (lpThis = Name_NameListenerList_MemLock(hThis)))
  {
    lpThis->hNext = hValue;
    Name_NameListenerList_MemUnlock(hThis);
  }
}

//--------------------------------------------------------------------

#endif  // defined(SS_V70)

//--------------------------------------------------------------------
