//--------------------------------------------------------------------
//
//  File: cal_col.c
//
//  Description: Column formulas and data dependencies.
//

#include "calc.h"
#include "cal_cell.h"
#include "cal_col.h"
#include "cal_expr.h"
#include "cal_mem.h"

//--------------------------------------------------------------------
//
//  The ColLookup() function searchs the vector for the given
//  column.  If found, a handle to the column is returned.
//  Otherwise, NULL is returned.
//

CALC_HANDLE ColLookup(LPCALC_SHEET lpSheet, long lCol)
{
  return VectGet(&lpSheet->Cols, lCol);
}

//--------------------------------------------------------------------
//
//  The ColLookupAlloc() function searchs the vecotr for the given
//  column.  If found, a handle to the column is returned.  If not
//  found, the function creates a new column in the vector and
//  returns a handle to the new column.  If an error occurs during
//  creation of the new column, then the function returns NULL.
//

CALC_HANDLE ColLookupAlloc(LPCALC_SHEET lpSheet, long lCol)
{
  CALC_HANDLE hCol;
  
  if( !(hCol = VectGet(&lpSheet->Cols, lCol)) )
  {
    if( 0 <= lCol && lCol <= CALC_MAX_COORD )
    {
      hCol = ColCreate(lpSheet, lCol);
      VectSet(&lpSheet->Cols, lCol, hCol);
    }
  }
  return hCol;
}

//--------------------------------------------------------------------
//
//  The ColSetExpr() function assigns an expression to the given
//  column.  The function returns the previously assigned expression.
//

CALC_HANDLE ColSetExpr(LPCALC_SHEET lpSheet, long lCol, CALC_HANDLE hExpr)
{
  LPCALC_COL lpCol;
  CALC_HANDLE hCol;
  CALC_HANDLE hReturn = 0;
  
  if( hExpr )
    hCol = ColLookupAlloc(lpSheet, lCol);
  else
    hCol = ColLookup(lpSheet, lCol);
  if( hCol && (lpCol = Col_MemLock(hCol)) )
  {
    hReturn = lpCol->hExpr;
    ExprSetLinkToCol(lpCol->hExpr, hCol, TRUE, FALSE);
    lpCol->hExpr = hExpr;
    ExprSetLinkToCol(lpCol->hExpr, hCol, TRUE, TRUE);
    Col_MemUnlock(hCol);
  }
  return hReturn;
}

//--------------------------------------------------------------------
//
//  The ColAddLinks() function creates data links to the given
//  column.  Links from named expressions to the column are
//  untouched.
//

BOOL ColSetLinksFromExpr(LPMATRIX lpCellTable, LPVECTOR lpColTable, long lCol,
                         BOOL bLink)
{
  CALC_HANDLE hCol;
  LPCALC_COL lpCol;
  BOOL bReturn = FALSE;
  
  hCol = VectGet(lpColTable, lCol);
  if( hCol && (lpCol = Col_MemLock(hCol)) )
  {
    ExprSetLinkToCol(lpCol->hExpr, hCol, FALSE, bLink);
    bReturn = TRUE;
    Col_MemUnlock(hCol);
  }
  return bReturn;
}

//--------------------------------------------------------------------
//
//  The ColGetExpr() function retrieves the expression assigned to
//  the given column.
//

CALC_HANDLE ColGetExpr(LPCALC_SHEET lpSheet, long lCol)
{
  LPCALC_COL lpCol;
  CALC_HANDLE hCol;
  CALC_HANDLE hReturn = 0;

  hCol = VectGet(&lpSheet->Cols, lCol);
  if( hCol && (lpCol = Col_MemLock(hCol)) )
  {
    hReturn = lpCol->hExpr;
    Col_MemUnlock(hCol);
  }
  return hReturn;
}

//--------------------------------------------------------------------
//
//  The ColCreate() function creates and initializes a column
//  structure.  The function returns a handle to the newly created
//  column structure
//

CALC_HANDLE ColCreate(LPCALC_SHEET lpSheet, long lCol)
{
  LPCALC_COL lpCol;
  CALC_HANDLE hCol;
  CALC_HANDLE hReturn = NULL;
  
  hCol = Col_MemAlloc();
  if( hCol && (lpCol = Col_MemLock(hCol)) )
  {
    lpCol->lpSheet = lpSheet;
    lpCol->lCol = lCol;
    lpCol->hExpr = NULL;
    #if defined(SS_V70)
    lpCol->hPreviousAdjust = NULL;
    lpCol->hNextAdjust = NULL;
    lpCol->hPreviousDirty = NULL;
    lpCol->hNextDirty = NULL;
    lpCol->hColListeners = NULL;
    #else
    BitMatInit(&lpCol->AbsPrecedCells);
    BitMatInit(&lpCol->RelPrecedCells);
    BitMatInit(&lpCol->RelDependCells);
    #endif
    hReturn = hCol;
    Col_MemUnlock(hCol);
  }
  return hReturn;
}

//--------------------------------------------------------------------
//
//  The ColDestroy() function frees all memory associated with a
//  column structure.
//

BOOL ColDestroy(CALC_HANDLE hCol)
{
  LPCALC_COL lpCol;
  BOOL bReturn = FALSE;
  
  if( hCol && (lpCol = Col_MemLock(hCol)) )
  {
    ExprDestroy(lpCol->hExpr);
    #if !defined(SS_V70)
    BitMatFree(&lpCol->AbsPrecedCells);
    BitMatFree(&lpCol->RelPrecedCells);
    BitMatFree(&lpCol->RelDependCells);
    #endif
    bReturn = TRUE;
    Col_MemUnlock(hCol);
  }
  Col_MemFree(hCol);
  return bReturn;
}

//--------------------------------------------------------------------
//
//  The ColSetLinkToCol() function adds or removes a data link
//  between the two given columns.
//

#if !defined(SS_V70)
BOOL ColSetLinkToCol(CALC_HANDLE hCol, CALC_HANDLE hDepend, long lRow,
                     BOOL bLink)
{
  LPCALC_COL lpCol;
  LPCALC_COL lpDepend;
  BOOL bReturn = FALSE;
  
  if( hCol && (lpCol = Col_MemLock(hCol)) )
  {
    if( hDepend && (lpDepend = Col_MemLock(hDepend)) )
    {                
      BitMatSet(&lpCol->RelDependCells, lpDepend->lCol, REL_OFFSET + lRow, bLink);
      BitMatSet(&lpDepend->RelPrecedCells, lpCol->lCol, REL_OFFSET - lRow, bLink);
      bReturn = TRUE;
      Col_MemUnlock(hDepend);
    }
    Col_MemUnlock(hCol);
  }
  return bReturn;
}
#endif

//--------------------------------------------------------------------
//
//  The ColMarkDepend() function records (in lpDepend) all cells
//  which use the given column.  Expressions attached to columns
//  do not affect column headers (i.e. row = 0).
//

#if !defined(SS_V70)
BOOL ColMarkDepend(CALC_HANDLE hCol, long lRow, LPBITMATRIX lpDepend)
{
  LPCALC_COL lpCol;
  BOOL bReturn = FALSE;
  
  if( hCol && (lpCol = Col_MemLock(hCol)) )
  {
    BitMatCopyOffset(&lpCol->RelDependCells, lpDepend, 0, lRow-REL_OFFSET, 0, 1);
    bReturn = TRUE;
    Col_MemUnlock(hCol);
  }
  return bReturn;
}
#endif

//--------------------------------------------------------------------

LPCALC_SHEET Col_GetSheet(CALC_HANDLE hThis)
{
  LPCALC_COL lpThis;
  LPCALC_SHEET hValue = NULL;

  if (hThis && (lpThis = Col_MemLock(hThis)))
  {
    hValue = lpThis->lpSheet;
    Col_MemUnlock(hThis);
  }
  return hValue;
}

//--------------------------------------------------------------------

long Col_GetCol(CALC_HANDLE hThis)
{
  LPCALC_COL lpThis;
  long lValue = 0;

  if (hThis && (lpThis = Col_MemLock(hThis)))
  {
    lValue = lpThis->lCol;
    Col_MemUnlock(hThis);
  }
  return lValue;
}

//--------------------------------------------------------------------

#if defined(SS_V70)

//--------------------------------------------------------------------

BOOL Col_IsEmpty(CALC_HANDLE hThis)
{
  LPCALC_COL lpThis;
  BOOL bValue = TRUE;

  if (hThis && (lpThis = Col_MemLock(hThis)))
  {
    bValue = lpThis->hExpr == NULL && lpThis->hColListeners == NULL;
    Col_MemUnlock(hThis);
  }
  return bValue;
}

//--------------------------------------------------------------------

CALC_HANDLE Col_GetPreviousAdjust(CALC_HANDLE hThis)
{
  LPCALC_COL lpThis;
  CALC_HANDLE hValue = NULL;

  if (hThis && (lpThis = Col_MemLock(hThis)))
  {
    hValue = lpThis->hPreviousAdjust;
    Col_MemUnlock(hThis);
  }
  return hValue;
}

//--------------------------------------------------------------------

void Col_SetPreviousAdjust(CALC_HANDLE hThis, CALC_HANDLE hValue)
{
  LPCALC_COL lpThis;

  if (hThis && (lpThis = Col_MemLock(hThis)))
  {
    lpThis->hPreviousAdjust = hValue;
    Col_MemUnlock(hThis);
  }
}

//--------------------------------------------------------------------

CALC_HANDLE Col_GetNextAdjust(CALC_HANDLE hThis)
{
  LPCALC_COL lpThis;
  CALC_HANDLE hValue = NULL;

  if (hThis && (lpThis = Col_MemLock(hThis)))
  {
    hValue = lpThis->hNextAdjust;
    Col_MemUnlock(hThis);
  }
  return hValue;
}

//--------------------------------------------------------------------

void Col_SetNextAdjust(CALC_HANDLE hThis, CALC_HANDLE hValue)
{
  LPCALC_COL lpThis;

  if (hThis && (lpThis = Col_MemLock(hThis)))
  {
    lpThis->hNextAdjust = hValue;
    Col_MemUnlock(hThis);
  }
}

//--------------------------------------------------------------------

CALC_HANDLE Col_GetPreviousDirty(CALC_HANDLE hThis)
{
  LPCALC_COL lpThis;
  CALC_HANDLE hValue = NULL;

  if (hThis && (lpThis = Col_MemLock(hThis)))
  {
    hValue = lpThis->hPreviousDirty;
    Col_MemUnlock(hThis);
  }
  return hValue;
}

//--------------------------------------------------------------------

void Col_SetPreviousDirty(CALC_HANDLE hThis, CALC_HANDLE hValue)
{
  LPCALC_COL lpThis;

  if (hThis && (lpThis = Col_MemLock(hThis)))
  {
    lpThis->hPreviousDirty = hValue;
    Col_MemUnlock(hThis);
  }
}

//--------------------------------------------------------------------

CALC_HANDLE Col_GetNextDirty(CALC_HANDLE hThis)
{
  LPCALC_COL lpThis;
  CALC_HANDLE hValue = NULL;

  if (hThis && (lpThis = Col_MemLock(hThis)))
  {
    hValue = lpThis->hNextDirty;
    Col_MemUnlock(hThis);
  }
  return hValue;
}

//--------------------------------------------------------------------

void Col_SetNextDirty(CALC_HANDLE hThis, CALC_HANDLE hValue)
{
  LPCALC_COL lpThis;

  if (hThis && (lpThis = Col_MemLock(hThis)))
  {
    lpThis->hNextDirty = hValue;
    Col_MemUnlock(hThis);
  }
}


//--------------------------------------------------------------------

void Col_StartListening(CALC_HANDLE hThis, BOOL bIncludeNames)
{
  LPCALC_COL lpThis;

  if (hThis && (lpThis = Col_MemLock(hThis)))
  {
    Expr_ColStartListening(lpThis->hExpr, hThis, bIncludeNames);
    Col_MemUnlock(hThis);
  }
}

//--------------------------------------------------------------------

void Col_StopListening(CALC_HANDLE hThis, BOOL bIncludeNames)
{
  LPCALC_COL lpThis;

  if (hThis && (lpThis = Col_MemLock(hThis)))
  {
    Expr_ColStopListening(lpThis->hExpr, hThis, bIncludeNames);
    Col_MemUnlock(hThis);
  }
}

//--------------------------------------------------------------------

void Col_AddColListener(CALC_HANDLE hThis, CALC_HANDLE hCol, long lRowOffset)
{
  LPCALC_COL lpThis;
  CALC_HANDLE hList;

  if (hThis && (lpThis = Col_MemLock(hThis)))
  {
    for (hList = lpThis->hColListeners; hList; hList = Col_ColListenerList_GetNext(hList))
    {
      if (hCol == Col_ColListenerList_GetCol(hList) && lRowOffset == Col_ColListenerList_GetRowOffset(hList))
        break;
    }
    if (!hList)
      lpThis->hColListeners = Col_ColListenerList_Create(hCol, lRowOffset, lpThis->hColListeners);
    Col_MemUnlock(hThis);
  }
}

//--------------------------------------------------------------------

void Col_RemoveColListener(CALC_HANDLE hThis, CALC_HANDLE hCol, long lRowOffset)
{
  LPCALC_COL lpThis;
  CALC_HANDLE hList;
  CALC_HANDLE hPrev;

  if (hThis && (lpThis = Col_MemLock(hThis)))
  {
    for (hPrev = NULL, hList = lpThis->hColListeners; hList; hPrev = hList, hList = Col_ColListenerList_GetNext(hList))
    {
      if (hCol == Col_ColListenerList_GetCol(hList) && lRowOffset == Col_ColListenerList_GetRowOffset(hList))
        break;
    }
    if (hList)
    {
      if (hPrev)
        Col_ColListenerList_SetNext(hPrev, Col_ColListenerList_GetNext(hList));
      else
        lpThis->hColListeners = Col_ColListenerList_GetNext(hList);
      Col_ColListenerList_Destroy(hList);
    }
    Col_MemUnlock(hThis);
  }
}

//--------------------------------------------------------------------

void Col_AddListenersToAdjust(CALC_HANDLE hThis)
{
  LPCALC_COL lpThis;
  LPCALC_BOOK lpBook;
  CALC_HANDLE hList;

  if (hThis && (lpThis = Col_MemLock(hThis)))
  {
    lpBook = Col_GetSheet(hThis)->lpBook;
    for (hList = lpThis->hColListeners; hList != NULL; hList = Col_ColListenerList_GetNext(hList))
      Book_AddAdjustCol(lpBook, Col_ColListenerList_GetCol(hList));
    Col_MemUnlock(hThis);
  }
}

//--------------------------------------------------------------------

void Col_AddListenersToDirty(CALC_HANDLE hThis, long lRow)
{
  LPCALC_COL lpThis;
  LPCALC_BOOK lpBook;
  CALC_HANDLE hList;

  if (hThis && (lpThis = Col_MemLock(hThis)))
  {
    lpBook = Col_GetSheet(hThis)->lpBook;
    for (hList = lpThis->hColListeners; hList != NULL; hList = Col_ColListenerList_GetNext(hList))
    {
      CALC_HANDLE hCol = Col_ColListenerList_GetCol(hList);
      if (lRow == CALC_ALLROWS)
        Book_AddDirtyCol(lpBook, hCol);
      else
      {
        long lRowOffset = Col_ColListenerList_GetRowOffset(hList);
        if (lRow + lRowOffset > 0)
        {
          CALC_HANDLE hCell = CellLookupAlloc(Col_GetSheet(hCol), Col_GetCol(hCol), lRow + lRowOffset);
          Book_AddDirtyCell(lpBook, hCell);
        }
      }
    }
    Col_MemUnlock(hThis);
  }
}

//--------------------------------------------------------------------

void Col_IncrementDelayOfListeners(CALC_HANDLE hThis, long lRow)
{
  LPCALC_COL lpThis;
  CALC_HANDLE hList;

  if (hThis && (lpThis = Col_MemLock(hThis)))
  {
    for (hList = lpThis->hColListeners; hList != NULL; hList = Col_ColListenerList_GetNext(hList))
    {
      long lRowOffset = Col_ColListenerList_GetRowOffset(hList);
      if (lRow + lRowOffset > 0)
      {
        CALC_HANDLE hCol = Col_ColListenerList_GetCol(hList);
        Sheet_IncrementDelay(Col_GetSheet(hCol), Col_GetCol(hCol), lRow + lRowOffset);
      }
    }
    Col_MemUnlock(hThis);
  }
}

//--------------------------------------------------------------------

void Col_DecrementDelayOfListeners(CALC_HANDLE hThis, long lRow)
{
  LPCALC_COL lpThis;
  CALC_HANDLE hList;

  if (hThis && (lpThis = Col_MemLock(hThis)))
  {
    for (hList = lpThis->hColListeners; hList != NULL; hList = Col_ColListenerList_GetNext(hList))
    {
      long lRowOffset = Col_ColListenerList_GetRowOffset(hList);
      if (lRow + lRowOffset > 0)
      {
        CALC_HANDLE hCol = Col_ColListenerList_GetCol(hList);
        Sheet_DecrementDelay(Col_GetSheet(hCol), Col_GetCol(hCol), lRow + lRowOffset);
      }
    }
    Col_MemUnlock(hThis);
  }
}

//--------------------------------------------------------------------

CALC_HANDLE Col_ColListenerList_Create(CALC_HANDLE hCol, long lRowOffset, CALC_HANDLE hNext)
{
  LPCALC_COL_COLLISTENERLIST lpThis;
  CALC_HANDLE hThis = NULL;

  hThis = Col_ColListenerList_MemAlloc();
  if (hThis && (lpThis = Col_ColListenerList_MemLock(hThis)))
  {
    lpThis->hCol = hCol;
    lpThis->lRowOffset = lRowOffset;
    lpThis->hNext = hNext;
    Col_ColListenerList_MemUnlock(hThis);
  }
  return hThis;
}

//--------------------------------------------------------------------

void Col_ColListenerList_Destroy(CALC_HANDLE hThis)
{
  LPCALC_COL_COLLISTENERLIST lpThis;

  if (hThis && (lpThis = Col_ColListenerList_MemLock(hThis)))
  {
    Col_ColListenerList_MemUnlock(hThis);
    Col_ColListenerList_MemFree(hThis);
  }
}

//--------------------------------------------------------------------

CALC_HANDLE Col_ColListenerList_GetCol(CALC_HANDLE hThis)
{
  LPCALC_COL_COLLISTENERLIST lpThis;
  CALC_HANDLE hValue = NULL;

  if (hThis && (lpThis = Col_ColListenerList_MemLock(hThis)))
  {
    hValue = lpThis->hCol;
    Col_ColListenerList_MemUnlock(hThis);
  }
  return hValue;
}

//--------------------------------------------------------------------

long Col_ColListenerList_GetRowOffset(CALC_HANDLE hThis)
{
  LPCALC_COL_COLLISTENERLIST lpThis;
  long lValue = 0;

  if (hThis && (lpThis = Col_ColListenerList_MemLock(hThis)))
  {
    lValue = lpThis->lRowOffset;
    Col_ColListenerList_MemUnlock(hThis);
  }
  return lValue;
}

//--------------------------------------------------------------------

CALC_HANDLE Col_ColListenerList_GetNext(CALC_HANDLE hThis)
{
  LPCALC_COL_COLLISTENERLIST lpThis;
  CALC_HANDLE hValue = NULL;

  if (hThis && (lpThis = Col_ColListenerList_MemLock(hThis)))
  {
    hValue = lpThis->hNext;
    Col_ColListenerList_MemUnlock(hThis);
  }
  return hValue;
}

//--------------------------------------------------------------------

void Col_ColListenerList_SetNext(CALC_HANDLE hThis, CALC_HANDLE hValue)
{
  LPCALC_COL_COLLISTENERLIST lpThis;

  if (hThis && (lpThis = Col_ColListenerList_MemLock(hThis)))
  {
    lpThis->hNext = hValue;
    Col_ColListenerList_MemUnlock(hThis);
  }
}

//--------------------------------------------------------------------

#endif  // defined(SS_V70)

//--------------------------------------------------------------------
