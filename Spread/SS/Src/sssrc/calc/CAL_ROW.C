//--------------------------------------------------------------------
//
//  File: cal_row.c
//
//  Description: Row formulas and data dependencies.
//

#include "calc.h"
#include "cal_cell.h"
#include "cal_expr.h"
#include "cal_mem.h"
#include "cal_row.h"

//--------------------------------------------------------------------
//
//  The RowLookup() function searches the vector for the given
//  row.  If found, a handle to the row is returned.  Otherwise,
//  NULL is returned.

CALC_HANDLE RowLookup(LPCALC_SHEET lpSheet, long lRow)
{
  return VectGet(&lpSheet->Rows, lRow);
}

//--------------------------------------------------------------------
//
//  The RowLookupAlloc() function searches the vector for the given
//  row.  If found, a handle to the row is returned.  If not found,
//  the function creates a new row in the vector and returns a
//  handle to the new row.  If an error occurs during creation of
//  the new row, then the function returns NULL.

CALC_HANDLE RowLookupAlloc(LPCALC_SHEET lpSheet, long lRow)
{
  CALC_HANDLE hRow;
  
  if( !(hRow = VectGet(&lpSheet->Rows, lRow)) )
  {
    if( 0 <= lRow && lRow <= CALC_MAX_COORD )
    {
      hRow = RowCreate(lpSheet, lRow);
      VectSet(&lpSheet->Rows, lRow, hRow);
    }
  }
  return hRow;
}

//--------------------------------------------------------------------
//
//
//  The RowSetExpr() function assigns an expression to the given
//  row.  The function returns the previously assigned expression.
//

CALC_HANDLE RowSetExpr(LPCALC_SHEET lpSheet, long lRow, CALC_HANDLE hExpr)
{
  LPCALC_ROW lpRow;
  CALC_HANDLE hRow;
  CALC_HANDLE hReturn = 0;
  
  if( hExpr )
    hRow = RowLookupAlloc(lpSheet, lRow);
  else
    hRow = RowLookup(lpSheet, lRow);
  if( hRow && (lpRow = Row_MemLock(hRow)) )
  {
    hReturn = lpRow->hExpr;
    ExprSetLinkToRow(lpRow->hExpr, hRow, TRUE, FALSE);
    lpRow->hExpr = hExpr;
    ExprSetLinkToRow(lpRow->hExpr, hRow, TRUE, TRUE);
    Row_MemUnlock(hRow);
  }
  return hReturn;
}

//--------------------------------------------------------------------
//
//  The RowAddLinks() function creates data links to the given row.
//  Links from named expressions to the row are untouched.
//

BOOL RowSetLinksFromExpr(LPMATRIX lpCellTable, LPVECTOR lpRowTable, long lRow,
                         BOOL bLink)
{
  CALC_HANDLE hRow;
  LPCALC_ROW lpRow;
  BOOL bReturn = FALSE;
  
  hRow = VectGet(lpRowTable, lRow);
  if( hRow && (lpRow = Row_MemLock(hRow)) )
  {
    ExprSetLinkToRow(lpRow->hExpr, hRow, FALSE, bLink);
    bReturn = TRUE;
    Row_MemUnlock(hRow);
  }
  return bReturn;
}

//--------------------------------------------------------------------
//
//  The RowGetExpr() function retrieves the expression assigned to
//  the given row.
//

CALC_HANDLE RowGetExpr(LPCALC_SHEET lpSheet, long lRow)
{
  LPCALC_ROW lpRow;
  CALC_HANDLE hRow;
  CALC_HANDLE hReturn = 0;

  if( (hRow = VectGet(&lpSheet->Rows, lRow))
      && (lpRow = Row_MemLock(hRow)) )
  {
    hReturn = lpRow->hExpr;
    Row_MemUnlock(hRow);
  }
  return hReturn;
}

//--------------------------------------------------------------------
//
//  The RowCreate() function creates and initializes a row structure.
//  The function returns a handle to the newly created row structure.
//

CALC_HANDLE RowCreate(LPCALC_SHEET lpSheet, long lRow)
{
  LPCALC_ROW lpRow;
  CALC_HANDLE hRow;
  CALC_HANDLE hReturn = NULL;
  
  hRow = Row_MemAlloc();
  if( hRow && (lpRow = Row_MemLock(hRow)) )
  {
    lpRow->lpSheet = lpSheet;
    lpRow->lRow = lRow;
    lpRow->hExpr = NULL;
    #if defined(SS_V70)
    lpRow->hNextAdjust = NULL;
    lpRow->hPreviousAdjust = NULL;
    lpRow->hNextDirty = NULL;
    lpRow->hPreviousDirty = NULL;
    lpRow->hRowListeners = NULL;
    #else
    BitMatInit(&lpRow->AbsPrecedCells);
    BitMatInit(&lpRow->RelPrecedCells);
    BitMatInit(&lpRow->RelDependCells);
    #endif
    hReturn = hRow;
    Row_MemUnlock(hRow);
  }
  return hReturn;
}

//--------------------------------------------------------------------
//
//  The RowDestroy() function frees all memory associated with the
//  row structure.
//

BOOL RowDestroy(CALC_HANDLE hRow)
{
  LPCALC_ROW lpRow;
  BOOL bReturn = FALSE;
  
  if( hRow && (lpRow = Row_MemLock(hRow)) )
  {
    ExprDestroy(lpRow->hExpr);
    #if !defined(SS_V70)
    BitMatFree(&lpRow->AbsPrecedCells);
    BitMatFree(&lpRow->RelPrecedCells);
    BitMatFree(&lpRow->RelDependCells);
    #endif
    bReturn = TRUE;
    Row_MemUnlock(hRow);
  }
  Row_MemFree(hRow);
  return bReturn;
}

//--------------------------------------------------------------------
//
//  The RowSetLinkToRow() function adds or removes a data link
//  between the two given rows.
//

#if !defined(SS_V70)
BOOL RowSetLinkToRow(CALC_HANDLE hRow, CALC_HANDLE hDepend, long lCol,
                     BOOL bLink)
{
  LPCALC_ROW lpRow;
  LPCALC_ROW lpDepend;
  BOOL bReturn = FALSE;
  
  if( hRow && (lpRow = Row_MemLock(hRow)) )
  {
    if( hDepend && (lpDepend = Row_MemLock(hDepend)) )
    {                
      BitMatSet(&lpRow->RelDependCells, REL_OFFSET + lCol, lpDepend->lRow, bLink);
      BitMatSet(&lpDepend->RelPrecedCells, REL_OFFSET - lCol, lpRow->lRow, bLink);
      bReturn = TRUE;
      Row_MemUnlock(hDepend);
    }
    Row_MemUnlock(hRow);
  }
  return bReturn;
}
#endif

//--------------------------------------------------------------------
//
//  The RowMarkDepend() function records (in lpDepend) all cells
//  which use the given cell.  Expressions attached to rows do not
//  affect row headers (i.e. col = 0).
//

#if !defined(SS_V70)
BOOL RowMarkDepend(CALC_HANDLE hRow, long lCol, LPBITMATRIX lpDepend)
{
  LPCALC_ROW lpRow;
  BOOL bReturn = FALSE;
  
  if( hRow && (lpRow = Row_MemLock(hRow)) )
  {
    BitMatCopyOffset(&lpRow->RelDependCells, lpDepend, lCol-REL_OFFSET, 0, 1, 0);
    bReturn = TRUE;
    Row_MemUnlock(hRow);
  }
  return bReturn;
}
#endif

//--------------------------------------------------------------------

LPCALC_SHEET Row_GetSheet(CALC_HANDLE hThis)
{
  LPCALC_ROW lpThis;
  LPCALC_SHEET hValue = NULL;

  if (hThis && (lpThis = Row_MemLock(hThis)))
  {
    hValue = lpThis->lpSheet;
    Row_MemUnlock(hThis);
  }
  return hValue;
}

//--------------------------------------------------------------------

long Row_GetRow(CALC_HANDLE hThis)
{
  LPCALC_ROW lpThis;
  long lValue = 0;

  if (hThis && (lpThis = Row_MemLock(hThis)))
  {
    lValue = lpThis->lRow;
    Row_MemUnlock(hThis);
  }
  return lValue;
}

//--------------------------------------------------------------------

#if defined(SS_V70)

//--------------------------------------------------------------------

BOOL Row_IsEmpty(CALC_HANDLE hThis)
{
  LPCALC_ROW lpThis;
  BOOL bValue = TRUE;

  if (hThis && (lpThis = Row_MemLock(hThis)))
  {
    bValue = lpThis->hExpr == NULL && lpThis->hRowListeners == NULL;
    Row_MemUnlock(hThis);
  }
  return bValue;
}

//--------------------------------------------------------------------

CALC_HANDLE Row_GetPreviousAdjust(CALC_HANDLE hThis)
{
  LPCALC_ROW lpThis;
  CALC_HANDLE hValue = NULL;

  if (hThis && (lpThis = Row_MemLock(hThis)))
  {
    hValue = lpThis->hPreviousAdjust;
    Row_MemUnlock(hThis);
  }
  return hValue;
}

//--------------------------------------------------------------------

void Row_SetPreviousAdjust(CALC_HANDLE hThis, CALC_HANDLE hValue)
{
  LPCALC_ROW lpThis;

  if (hThis && (lpThis = Row_MemLock(hThis)))
  {
    lpThis->hPreviousAdjust = hValue;
    Row_MemUnlock(hThis);
  }
}

//--------------------------------------------------------------------

CALC_HANDLE Row_GetNextAdjust(CALC_HANDLE hThis)
{
  LPCALC_ROW lpThis;
  CALC_HANDLE hValue = NULL;

  if (hThis && (lpThis = Row_MemLock(hThis)))
  {
    hValue = lpThis->hNextAdjust;
    Row_MemUnlock(hThis);
  }
  return hValue;
}

//--------------------------------------------------------------------

void Row_SetNextAdjust(CALC_HANDLE hThis, CALC_HANDLE hValue)
{
  LPCALC_ROW lpThis;

  if (hThis && (lpThis = Row_MemLock(hThis)))
  {
    lpThis->hNextAdjust = hValue;
    Row_MemUnlock(hThis);
  }
}

//--------------------------------------------------------------------

CALC_HANDLE Row_GetPreviousDirty(CALC_HANDLE hThis)
{
  LPCALC_ROW lpThis;
  CALC_HANDLE hValue = NULL;

  if (hThis && (lpThis = Row_MemLock(hThis)))
  {
    hValue = lpThis->hPreviousDirty;
    Row_MemUnlock(hThis);
  }
  return hValue;
}

//--------------------------------------------------------------------

void Row_SetPreviousDirty(CALC_HANDLE hThis, CALC_HANDLE hValue)
{
  LPCALC_ROW lpThis;

  if (hThis && (lpThis = Row_MemLock(hThis)))
  {
    lpThis->hPreviousDirty = hValue;
    Row_MemUnlock(hThis);
  }
}

//--------------------------------------------------------------------

CALC_HANDLE Row_GetNextDirty(CALC_HANDLE hThis)
{
  LPCALC_ROW lpThis;
  CALC_HANDLE hValue = NULL;

  if (hThis && (lpThis = Row_MemLock(hThis)))
  {
    hValue = lpThis->hNextDirty;
    Row_MemUnlock(hThis);
  }
  return hValue;
}

//--------------------------------------------------------------------

void Row_SetNextDirty(CALC_HANDLE hThis, CALC_HANDLE hValue)
{
  LPCALC_ROW lpThis;

  if (hThis && (lpThis = Row_MemLock(hThis)))
  {
    lpThis->hNextDirty = hValue;
    Row_MemUnlock(hThis);
  }
}


//--------------------------------------------------------------------

void Row_StartListening(CALC_HANDLE hThis, BOOL bIncludeNames)
{
  LPCALC_ROW lpThis;

  if (hThis && (lpThis = Row_MemLock(hThis)))
  {
    Expr_RowStartListening(lpThis->hExpr, hThis, bIncludeNames);
    Row_MemUnlock(hThis);
  }
}

//--------------------------------------------------------------------

void Row_StopListening(CALC_HANDLE hThis, BOOL bIncludeNames)
{
  LPCALC_ROW lpThis;

  if (hThis && (lpThis = Row_MemLock(hThis)))
  {
    Expr_RowStopListening(lpThis->hExpr, hThis, bIncludeNames);
    Row_MemUnlock(hThis);
  }
}

//--------------------------------------------------------------------

void Row_AddRowListener(CALC_HANDLE hThis, CALC_HANDLE hRow, long lColOffset)
{
  LPCALC_ROW lpThis;
  CALC_HANDLE hList;

  if (hThis && (lpThis = Row_MemLock(hThis)))
  {
    for (hList = lpThis->hRowListeners; hList; hList = Row_RowListenerList_GetNext(hList))
    {
      if (hRow == Row_RowListenerList_GetRow(hList) && lColOffset == Row_RowListenerList_GetColOffset(hList))
      {
        break;
      }
    }
    if (!hList)
      lpThis->hRowListeners = Row_RowListenerList_Create(hRow, lColOffset, lpThis->hRowListeners);
    Row_MemUnlock(hThis);
  }
}

//--------------------------------------------------------------------

void Row_RemoveRowListener(CALC_HANDLE hThis, CALC_HANDLE hRow, long lColOffset)
{
  LPCALC_ROW lpThis;
  CALC_HANDLE hList;
  CALC_HANDLE hPrev;

  if (hThis && (lpThis = Row_MemLock(hThis)))
  {
    for (hPrev = NULL, hList = lpThis->hRowListeners; hList; hPrev = hList, hList = Row_RowListenerList_GetNext(hList))
    {
      if (hRow == Row_RowListenerList_GetRow(hList) && lColOffset == Row_RowListenerList_GetColOffset(hList))
        break;
    }
    if (hList)
    {
      if (hPrev)
        Row_RowListenerList_SetNext(hPrev, Row_RowListenerList_GetNext(hList));
      else
        lpThis->hRowListeners = Row_RowListenerList_GetNext(hList);
      Row_RowListenerList_Destroy(hList);
    }
    Row_MemUnlock(hThis);
  }
}

//--------------------------------------------------------------------

void Row_AddListenersToAdjust(CALC_HANDLE hThis)
{
  LPCALC_ROW lpThis;
  LPCALC_BOOK lpBook;
  CALC_HANDLE hList;

  if (hThis && (lpThis = Row_MemLock(hThis)))
  {
    lpBook = Row_GetSheet(hThis)->lpBook;
    for (hList = lpThis->hRowListeners; hList != NULL; hList = Row_RowListenerList_GetNext(hList))
      Book_AddAdjustRow(lpBook, Row_RowListenerList_GetRow(hList));
    Row_MemUnlock(hThis);
  }
}

//--------------------------------------------------------------------

void Row_AddListenersToDirty(CALC_HANDLE hThis, long lCol)
{
  LPCALC_ROW lpThis;
  LPCALC_BOOK lpBook;
  CALC_HANDLE hList;

  if (hThis && (lpThis = Row_MemLock(hThis)))
  {
    lpBook = Row_GetSheet(hThis)->lpBook;
    for (hList = lpThis->hRowListeners; hList != NULL; hList = Row_RowListenerList_GetNext(hList))
    {
      CALC_HANDLE hRow = Row_RowListenerList_GetRow(hList);
      if (lCol == CALC_ALLCOLS)
        Book_AddDirtyRow(lpBook, hRow);
      else
      {
        long lColOffset = Row_RowListenerList_GetColOffset(hList);
        if (lCol + lColOffset > 0)
        {
          CALC_HANDLE hCell = CellLookupAlloc(Row_GetSheet(hRow), lCol + lColOffset, Row_GetRow(hRow));
          Book_AddDirtyCell(lpBook, hCell);
        }
      }
    }
    Row_MemUnlock(hThis);
  }
}

//--------------------------------------------------------------------

void Row_IncrementDelayOfListeners(CALC_HANDLE hThis, long lCol)
{
  LPCALC_ROW lpThis;
  CALC_HANDLE hList;

  if (hThis && (lpThis = Row_MemLock(hThis)))
  {
    for (hList = lpThis->hRowListeners; hList != NULL; hList = Row_RowListenerList_GetNext(hList))
    {
      long lColOffset = Row_RowListenerList_GetColOffset(hList);
      if (lCol + lColOffset > 0)
      {
        CALC_HANDLE hRow = Row_RowListenerList_GetRow(hList);
        Sheet_IncrementDelay(Row_GetSheet(hRow), lCol + lColOffset, Row_GetRow(hRow));
      }
    }
    Row_MemUnlock(hThis);
  }
}

//--------------------------------------------------------------------

void Row_DecrementDelayOfListeners(CALC_HANDLE hThis, long lCol)
{
  LPCALC_ROW lpThis;
  CALC_HANDLE hList;

  if (hThis && (lpThis = Row_MemLock(hThis)))
  {
    for (hList = lpThis->hRowListeners; hList != NULL; hList = Row_RowListenerList_GetNext(hList))
    {
      long lColOffset = Row_RowListenerList_GetColOffset(hList);
      if (lCol + lColOffset > 0)
      {
        CALC_HANDLE hRow = Row_RowListenerList_GetRow(hList);
        Sheet_DecrementDelay(Row_GetSheet(hRow), lCol + lColOffset, Row_GetRow(hRow));
      }
    }
    Row_MemUnlock(hThis);
  }
}

//--------------------------------------------------------------------

CALC_HANDLE Row_RowListenerList_Create(CALC_HANDLE hRow, long lColOffset, CALC_HANDLE hNext)
{
  LPCALC_ROW_ROWLISTENERLIST lpThis;
  CALC_HANDLE hThis;

  hThis = Row_RowListenerList_MemAlloc();
  if (hThis && (lpThis = Row_RowListenerList_MemLock(hThis)))
  {
    lpThis->hRow = hRow;
    lpThis->lColOffset = lColOffset;
    lpThis->hNext = hNext;
    Row_RowListenerList_MemUnlock(hThis);
  }
  return hThis;
}

//--------------------------------------------------------------------

void Row_RowListenerList_Destroy(CALC_HANDLE hThis)
{
  LPCALC_ROW_ROWLISTENERLIST lpThis;

  if (hThis && (lpThis = Row_RowListenerList_MemLock(hThis)))
  {
    Row_RowListenerList_MemUnlock(hThis);
    Row_RowListenerList_MemFree(hThis);
  }
}

//--------------------------------------------------------------------

CALC_HANDLE Row_RowListenerList_GetRow(CALC_HANDLE hThis)
{
  LPCALC_ROW_ROWLISTENERLIST lpThis;
  CALC_HANDLE hValue = NULL;

  if (hThis && (lpThis = Row_RowListenerList_MemLock(hThis)))
  {
    hValue = lpThis->hRow;
    Row_RowListenerList_MemUnlock(hThis);
  }
  return hValue;
}

//--------------------------------------------------------------------

long Row_RowListenerList_GetColOffset(CALC_HANDLE hThis)
{
  LPCALC_ROW_ROWLISTENERLIST lpThis;
  long lValue = 0;

  if (hThis && (lpThis = Row_RowListenerList_MemLock(hThis)))
  {
    lValue = lpThis->lColOffset;
    Row_RowListenerList_MemUnlock(hThis);
  }
  return lValue;
}

//--------------------------------------------------------------------

CALC_HANDLE Row_RowListenerList_GetNext(CALC_HANDLE hThis)
{
  LPCALC_ROW_ROWLISTENERLIST lpThis;
  CALC_HANDLE hValue = NULL;

  if (hThis && (lpThis = Row_RowListenerList_MemLock(hThis)))
  {
    hValue = lpThis->hNext;
    Row_RowListenerList_MemUnlock(hThis);
  }
  return hValue;
}

//--------------------------------------------------------------------

void Row_RowListenerList_SetNext(CALC_HANDLE hThis, CALC_HANDLE hValue)
{
  LPCALC_ROW_ROWLISTENERLIST lpThis;

  if (hThis && (lpThis = Row_RowListenerList_MemLock(hThis)))
  {
    lpThis->hNext = hValue;
    Row_RowListenerList_MemUnlock(hThis);
  }
}

//--------------------------------------------------------------------

#endif  // defined(SS_V70)

//--------------------------------------------------------------------
