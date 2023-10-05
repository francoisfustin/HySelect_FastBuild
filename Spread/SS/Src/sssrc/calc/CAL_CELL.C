//--------------------------------------------------------------------
//
//  File: cal_cell.c
//
//  Description: Cell formulas and data dependencies.
//

#include "calc.h"
#include "cal_cell.h"
#include "cal_col.h"
#include "cal_row.h"
#include "cal_expr.h"
#include "cal_mem.h"

//--------------------------------------------------------------------
//
//  The CellLookup() function searches the matrix for the given
//  cell.  If found, a handle to the cell is returned.  Otherwise,
//  NULL is returned.
//

CALC_HANDLE CellLookup(LPCALC_SHEET lpSheet, long lCol, long lRow)
{
  return MatGet(&lpSheet->Cells, lCol, lRow);
}

//--------------------------------------------------------------------
//
//  The CellLookupAlloc() function searches the matrix for the given
//  cell.  If found, a handle to the cell is returned.  If not found,
//  the function creates a new cell in the matrix and return a handle
//  of the new cell.  If an error occurs during creation of the new
//  cell, then the function returns NULL.
//

CALC_HANDLE CellLookupAlloc(LPCALC_SHEET lpSheet, long lCol, long lRow)
{
  CALC_HANDLE hCell;
  
  if( !(hCell = MatGet(&lpSheet->Cells, lCol, lRow)) )
  {
    if( 0 <= lCol && lCol <= CALC_MAX_COORD && 0 <= lRow && lRow <= CALC_MAX_COORD )
    {
      hCell = CellCreate(lpSheet, lCol, lRow);
      MatSet(&lpSheet->Cells, lCol, lRow, hCell);
    }
  }
  return hCell;
}

//--------------------------------------------------------------------
//
//  The CellSetExpr() function assigns an expression to the given
//  cell.  The function returns the previously assign expression.
//

CALC_HANDLE CellSetExpr(LPCALC_SHEET lpSheet, long lCol, long lRow,
                        CALC_HANDLE hExpr)
{
  LPCALC_CELL lpCell;
  CALC_HANDLE hCell;
  CALC_HANDLE hReturn = 0;
  
  if( hExpr )
    hCell = CellLookupAlloc(lpSheet, lCol, lRow);
  else
    hCell = CellLookup(lpSheet, lCol, lRow);
  if( hCell && (lpCell = Cell_MemLock(hCell)) )
  {
    hReturn = lpCell->hExpr;
    ExprSetLinkToCell(lpCell->hExpr, hCell, TRUE, FALSE);
    lpCell->hExpr = hExpr;
    ExprSetLinkToCell(lpCell->hExpr, hCell, TRUE, TRUE);
    Cell_MemUnlock(hCell);
  }
  return hReturn;
}

//--------------------------------------------------------------------
//
//  The CellSetLinksFromExpr() function adds data links to the given cell.
//  Links from named expressions to the cell are untouched.
//

BOOL CellSetLinksFromExpr(LPMATRIX lpTable, long lCol, long lRow, BOOL bLink)
{
  CALC_HANDLE hCell;
  LPCALC_CELL lpCell;
  BOOL bReturn = FALSE;
  
  hCell = MatGet(lpTable, lCol, lRow);
  if( hCell && (lpCell = Cell_MemLock(hCell)) )
  {
    ExprSetLinkToCell(lpCell->hExpr, hCell, FALSE, bLink);
    bReturn = TRUE;
    Cell_MemUnlock(hCell);
  }
  return bReturn;
}

//--------------------------------------------------------------------
//
//  The CellGetExpr() function retrieves the expression assigned to
//  the given cell.
//

CALC_HANDLE CellGetExpr(LPCALC_SHEET lpSheet, long lCol, long lRow)
{
  LPCALC_CELL lpCell;
  CALC_HANDLE hCell;
  CALC_HANDLE hReturn = 0;

  hCell = CellLookup(lpSheet, lCol, lRow);
  if( hCell && (lpCell = Cell_MemLock(hCell)) )
  {
    hReturn = lpCell->hExpr;
    Cell_MemUnlock(hCell);
  }
  return hReturn;
}

//--------------------------------------------------------------------
//
//  The CellCreate() creates and initializes a cell structure.
//

CALC_HANDLE CellCreate(LPCALC_SHEET lpSheet, long lCol, long lRow)
{
  LPCALC_CELL lpCell;
  CALC_HANDLE hCell;
  CALC_HANDLE hReturn = 0;
  
  hCell = Cell_MemAlloc();
  if( hCell && (lpCell = Cell_MemLock(hCell)) )
  {
    lpCell->lpSheet = lpSheet;
    lpCell->lCol = lCol;
    lpCell->lRow = lRow;
    lpCell->hExpr = 0;
    #if defined(SS_V70)
    lpCell->hPreviousAdjust = NULL;
    lpCell->hNextAdjust = NULL;
    lpCell->hPreviousDirty = NULL;
    lpCell->hNextDirty = NULL;
    lpCell->hCellListeners = NULL;
    lpCell->hRowListeners = NULL;
    lpCell->hColListeners = NULL;
    lpCell->lDelay = 0;
    #else
    BitMatInit(&lpCell->PrecedCells);
    BitMatInit(&lpCell->DependCells);
    BitVectInit(&lpCell->DependCols);
    BitVectInit(&lpCell->DependRows);
    #endif
    hReturn = hCell;
    Cell_MemUnlock(hCell);
  }
  return hReturn;
}

//--------------------------------------------------------------------
//
//  The CellDestroy() function frees all memory associated with the
//  cell structure.
//

BOOL CellDestroy(CALC_HANDLE hCell)
{
  LPCALC_CELL lpCell;
  BOOL bReturn = FALSE;
  
  if( hCell && (lpCell = Cell_MemLock(hCell)) )
  {
    ExprDestroy(lpCell->hExpr);
    #if !defined(SS_V70)
    BitMatFree(&lpCell->DependCells);
    BitMatFree(&lpCell->PrecedCells);
    BitVectFree(&lpCell->DependCols);
    BitVectFree(&lpCell->DependRows);
    #endif
    bReturn = TRUE;
    Cell_MemUnlock(hCell);
  }
  Cell_MemFree(hCell);
  return bReturn;
}

//--------------------------------------------------------------------
//
//  The CellSetLinkToCell() function creates or removes a data link
//  between the two given cells.
//

#if !defined(SS_V70)
BOOL CellSetLinkToCell(CALC_HANDLE hCell, CALC_HANDLE hDepend, BOOL bLink)
{
  LPCALC_CELL lpCell;
  LPCALC_CELL lpDepend;
  BOOL bReturn = FALSE;
  
  if( hCell && (lpCell = Cell_MemLock(hCell)) )
  {
    if( hDepend && (lpDepend = Cell_MemLock(hDepend)) )
    {
      BitMatSet(&lpCell->DependCells, lpDepend->lCol, lpDepend->lRow, bLink);
      BitMatSet(&lpDepend->PrecedCells, lpCell->lCol, lpCell->lRow, bLink);
      bReturn = TRUE;
      Cell_MemUnlock(hDepend);
    }
    Cell_MemUnlock(hCell);
  }
  return bReturn;
}
#endif

//--------------------------------------------------------------------
//
//  The CellSetLinkToCol() function creates or removes a data link
//  between the cell and the column.
//

#if !defined(SS_V70)
BOOL CellSetLinkToCol(CALC_HANDLE hCell, CALC_HANDLE hDepend, BOOL bLink)
{
  LPCALC_CELL lpCell;
  LPCALC_COL lpDepend;
  BOOL bReturn = FALSE;
  
  if( hCell && (lpCell = Cell_MemLock(hCell)) )
  {
    if( hDepend && (lpDepend = Col_MemLock(hDepend)) )
    {
      BitVectSet(&lpCell->DependCols, lpDepend->lCol, bLink);
      BitMatSet(&lpDepend->AbsPrecedCells, lpCell->lCol, lpCell->lRow, bLink);
      bReturn = TRUE;
      Col_MemUnlock(hDepend);
    }
    Cell_MemUnlock(hCell);
  }
  return bReturn;
}
#endif

//--------------------------------------------------------------------
//
//  The CellSetLinkRow() function creates or removes a data link
//  between the cell and the row.
//

#if !defined(SS_V70)
BOOL CellSetLinkToRow(CALC_HANDLE hCell, CALC_HANDLE hDepend, BOOL bLink)
{
  LPCALC_CELL lpCell;
  LPCALC_ROW lpDepend;
  BOOL bReturn = FALSE;
  
  if( hCell && (lpCell = Cell_MemLock(hCell)) )
  {
    if( hDepend && (lpDepend = Row_MemLock(hDepend)) )
    {
      BitVectSet(&lpCell->DependRows, lpDepend->lRow, bLink);
      BitMatSet(&lpDepend->AbsPrecedCells, lpCell->lCol, lpCell->lRow, bLink);
      bReturn = TRUE;
      Row_MemUnlock(hDepend);
    }
    Cell_MemUnlock(hCell);
  }
  return bReturn;
}
#endif

//--------------------------------------------------------------------
//
//  The CellMarkPreced() function records (in lpPrecedCells) all
//  cells which are used by the given cell.
//

#if !defined(SS_V70)
BOOL CellMarkPreced(CALC_HANDLE hCell, LPBITMATRIX lpPrecedCells)
{
  LPCALC_CELL lpCell;
  BOOL bReturn = FALSE;
  
  if( hCell && (lpCell = Cell_MemLock(hCell)) )
  {
    BitMatCopy(&lpCell->PrecedCells, lpPrecedCells);
    bReturn = TRUE;
    Cell_MemUnlock(hCell);
  }
  return bReturn;
}
#endif

//--------------------------------------------------------------------
//
//  The CellMarkDepend() function records (in lpDependCells,
//  lpDependCols, lpDependRows) all cells, columns, and rows which
//  use the given cell.
//

#if !defined(SS_V70)
BOOL CellMarkDepend(CALC_HANDLE hCell,
                    LPBITMATRIX lpDependCells,
                    LPBITVECTOR lpDependCols,
                    LPBITVECTOR lpDependRows)
{
  LPCALC_CELL lpCell;
  BOOL bReturn = FALSE;
  
  if( hCell && (lpCell = Cell_MemLock(hCell)) )
  {
    BitMatCopy(&lpCell->DependCells, lpDependCells);
    BitVectCopy(&lpCell->DependCols, lpDependCols);
    BitVectCopy(&lpCell->DependRows, lpDependRows);
    bReturn = TRUE;
    Cell_MemUnlock(hCell);
  }
  return bReturn;
}
#endif

//--------------------------------------------------------------------

LPCALC_SHEET Cell_GetSheet(CALC_HANDLE hThis)
{
  LPCALC_CELL lpThis;
  LPCALC_SHEET lpValue = NULL;

  if (hThis && (lpThis = Cell_MemLock(hThis)))
  {
    lpValue = lpThis->lpSheet;
    Cell_MemUnlock(hThis);
  }
  return lpValue;
}

//--------------------------------------------------------------------

long Cell_GetRow(CALC_HANDLE hThis)
{
  LPCALC_CELL lpThis;
  long lValue = 0;

  if (hThis && (lpThis = Cell_MemLock(hThis)))
  {
    lValue = lpThis->lRow;
    Cell_MemUnlock(hThis);
  }
  return lValue;
}

//--------------------------------------------------------------------

long Cell_GetCol(CALC_HANDLE hThis)
{
  LPCALC_CELL lpThis;
  long lValue = 0;

  if (hThis && (lpThis = Cell_MemLock(hThis)))
  {
    lValue = lpThis->lCol;
    Cell_MemUnlock(hThis);
  }
  return lValue;
}

//--------------------------------------------------------------------

#if defined(SS_V70)

//--------------------------------------------------------------------

BOOL Cell_IsEmpty(CALC_HANDLE hThis)
{
  LPCALC_CELL lpThis;
  BOOL bValue = TRUE;

  if (hThis && (lpThis = Cell_MemLock(hThis)))
  {
    bValue = lpThis->hExpr == NULL && lpThis->hCellListeners == NULL && lpThis->hRowListeners == NULL && lpThis->hColListeners == NULL;
    Cell_MemUnlock(hThis);
  }
  return bValue;
}

//--------------------------------------------------------------------

CALC_HANDLE Cell_GetPreviousAdjust(CALC_HANDLE hThis)
{
  LPCALC_CELL lpThis;
  CALC_HANDLE hValue = NULL;

  if (hThis && (lpThis = Cell_MemLock(hThis)))
  {
    hValue = lpThis->hPreviousAdjust;
    Cell_MemUnlock(hThis);
  }
  return hValue;
}

//--------------------------------------------------------------------

void Cell_SetPreviousAdjust(CALC_HANDLE hThis, CALC_HANDLE hValue)
{
  LPCALC_CELL lpThis;

  if (hThis && (lpThis = Cell_MemLock(hThis)))
  {
    lpThis->hPreviousAdjust = hValue;
    Cell_MemUnlock(hThis);
  }
}

//--------------------------------------------------------------------

CALC_HANDLE Cell_GetNextAdjust(CALC_HANDLE hThis)
{
  LPCALC_CELL lpThis;
  CALC_HANDLE hValue = NULL;

  if (hThis && (lpThis = Cell_MemLock(hThis)))
  {
    hValue = lpThis->hNextAdjust;
    Cell_MemUnlock(hThis);
  }
  return hValue;
}

//--------------------------------------------------------------------

void Cell_SetNextAdjust(CALC_HANDLE hThis, CALC_HANDLE hValue)
{
  LPCALC_CELL lpThis;

  if (hThis && (lpThis = Cell_MemLock(hThis)))
  {
    lpThis->hNextAdjust = hValue;
    Cell_MemUnlock(hThis);
  }
}

//--------------------------------------------------------------------

CALC_HANDLE Cell_GetPreviousDirty(CALC_HANDLE hThis)
{
  LPCALC_CELL lpThis;
  CALC_HANDLE hValue = NULL;

  if (hThis && (lpThis = Cell_MemLock(hThis)))
  {
    hValue = lpThis->hPreviousDirty;
    Cell_MemUnlock(hThis);
  }
  return hValue;
}

//--------------------------------------------------------------------

void Cell_SetPreviousDirty(CALC_HANDLE hThis, CALC_HANDLE hValue)
{
  LPCALC_CELL lpThis;

  if (hThis && (lpThis = Cell_MemLock(hThis)))
  {
    lpThis->hPreviousDirty = hValue;
    Cell_MemUnlock(hThis);
  }
}

//--------------------------------------------------------------------

CALC_HANDLE Cell_GetNextDirty(CALC_HANDLE hThis)
{
  LPCALC_CELL lpThis;
  CALC_HANDLE hValue = NULL;

  if (hThis && (lpThis = Cell_MemLock(hThis)))
  {
    hValue = lpThis->hNextDirty;
    Cell_MemUnlock(hThis);
  }
  return hValue;
}

//--------------------------------------------------------------------

void Cell_SetNextDirty(CALC_HANDLE hThis, CALC_HANDLE hValue)
{
  LPCALC_CELL lpThis;

  if (hThis && (lpThis = Cell_MemLock(hThis)))
  {
    lpThis->hNextDirty = hValue;
    Cell_MemUnlock(hThis);
  }
}

//--------------------------------------------------------------------

long Cell_GetDelay(CALC_HANDLE hThis)
{
  LPCALC_CELL lpThis;
  long lValue = 0;

  if (hThis && (lpThis = Cell_MemLock(hThis)))
  {
    lValue = lpThis->lDelay;
    Cell_MemUnlock(hThis);
  }
  return lValue;
}

//--------------------------------------------------------------------

void Cell_SetDelay(CALC_HANDLE hThis, long lValue)
{
  LPCALC_CELL lpThis;

  if (hThis && (lpThis = Cell_MemLock(hThis)))
  {
    lpThis->lDelay = lValue;
    Cell_MemUnlock(hThis);
  }
}

//--------------------------------------------------------------------

void Cell_StartListening(CALC_HANDLE hThis, BOOL bIncludeNames)
{
  LPCALC_CELL lpThis;

  if (hThis && (lpThis = Cell_MemLock(hThis)))
  {
    Expr_CellStartListening(lpThis->hExpr, hThis, bIncludeNames);
    Cell_MemUnlock(hThis);
  }
}

//--------------------------------------------------------------------

void Cell_StopListening(CALC_HANDLE hThis, BOOL bIncludeNames)
{
  LPCALC_CELL lpThis;

  if (hThis && (lpThis = Cell_MemLock(hThis)))
  {
    Expr_CellStopListening(lpThis->hExpr, hThis, bIncludeNames);
    Cell_MemUnlock(hThis);
  }
}

//--------------------------------------------------------------------

void Cell_AddCellListener(CALC_HANDLE hThis, CALC_HANDLE hCell)
{
  LPCALC_CELL lpThis;
  CALC_HANDLE hList;

  if (hThis && (lpThis = Cell_MemLock(hThis)))
  {
    for (hList = lpThis->hCellListeners; hList; hList = Cell_CellListenerList_GetNext(hList))
    {
      if (hCell == Cell_CellListenerList_GetCell(hList))
        break;
    }
    if (!hList)
      lpThis->hCellListeners = Cell_CellListenerList_Create(hCell, lpThis->hCellListeners);
    Cell_MemUnlock(hThis);
  }
}

//--------------------------------------------------------------------

void Cell_RemoveCellListener(CALC_HANDLE hThis, CALC_HANDLE hCell)
{
  LPCALC_CELL lpThis;
  CALC_HANDLE hList;
  CALC_HANDLE hPrev;

  if (hThis && (lpThis = Cell_MemLock(hThis)))
  {
    for (hPrev = NULL, hList = lpThis->hCellListeners; hList; hPrev = hList, hList = Cell_CellListenerList_GetNext(hList))
    {
      if (hCell == Cell_CellListenerList_GetCell(hList))
        break;
    }
    if (hList)
    {
      if (hPrev)
        Cell_CellListenerList_SetNext(hPrev, Cell_CellListenerList_GetNext(hList));
      else
        lpThis->hCellListeners = Cell_CellListenerList_GetNext(hList);
      Cell_CellListenerList_Destroy(hList);
    }
    Cell_MemUnlock(hThis);
  }
}

//--------------------------------------------------------------------

void Cell_AddRowListener(CALC_HANDLE hThis, CALC_HANDLE hRow)
{
  LPCALC_CELL lpThis;
  CALC_HANDLE hList;

  if (hThis && (lpThis = Cell_MemLock(hThis)))
  {
    for (hList = lpThis->hRowListeners; hList; hList = Cell_RowListenerList_GetNext(hList))
    {
      if (hRow == Cell_RowListenerList_GetRow(hList))
        break;
    }
    if (!hList)
      lpThis->hRowListeners = Cell_RowListenerList_Create(hRow, lpThis->hRowListeners);
    Cell_MemUnlock(hThis);
  }
}

//--------------------------------------------------------------------

void Cell_RemoveRowListener(CALC_HANDLE hThis, CALC_HANDLE hRow)
{
  LPCALC_CELL lpThis;
  CALC_HANDLE hList;
  CALC_HANDLE hPrev;

  if (hThis && (lpThis = Cell_MemLock(hThis)))
  {
    for (hPrev = NULL, hList = lpThis->hRowListeners; hList; hPrev = hList, hList = Cell_RowListenerList_GetNext(hList))
    {
      if (hRow == Cell_RowListenerList_GetRow(hList))
        break;
    }
    if (hList)
    {
      if (hPrev)
        Cell_RowListenerList_SetNext(hPrev, Cell_RowListenerList_GetNext(hList));
      else
        lpThis->hRowListeners = Cell_RowListenerList_GetNext(hList);
      Cell_RowListenerList_Destroy(hList);
    }
    Cell_MemUnlock(hThis);
  }
}

//--------------------------------------------------------------------

void Cell_AddColListener(CALC_HANDLE hThis, CALC_HANDLE hCol)
{
  LPCALC_CELL lpThis;
  CALC_HANDLE hList;

  if (hThis && (lpThis = Cell_MemLock(hThis)))
  {
    for (hList = lpThis->hColListeners; hList; hList = Cell_ColListenerList_GetNext(hList))
    {
      if (hCol == Cell_ColListenerList_GetCol(hList))
        break;
    }
    if (!hList)
      lpThis->hColListeners = Cell_ColListenerList_Create(hCol, lpThis->hColListeners);
    Cell_MemUnlock(hThis);
  }
}

//--------------------------------------------------------------------

void Cell_RemoveColListener(CALC_HANDLE hThis, CALC_HANDLE hCol)
{
  LPCALC_CELL lpThis;
  CALC_HANDLE hList;
  CALC_HANDLE hPrev;

  if (hThis && (lpThis = Cell_MemLock(hThis)))
  {
    for (hPrev = NULL, hList = lpThis->hColListeners; hList; hPrev = hList, hList = Cell_ColListenerList_GetNext(hList))
    {
      if (hCol == Cell_ColListenerList_GetCol(hList))
        break;
    }
    if (hList)
    {
      if (hPrev)
        Cell_ColListenerList_SetNext(hPrev, Cell_ColListenerList_GetNext(hList));
      else
        lpThis->hColListeners = Cell_ColListenerList_GetNext(hList);
      Cell_ColListenerList_Destroy(hList);
    }
    Cell_MemUnlock(hThis);
  }
}

//--------------------------------------------------------------------

void Cell_AddListenersToAdjust(CALC_HANDLE hThis)
{
  LPCALC_CELL lpThis;
  LPCALC_BOOK lpBook;
  CALC_HANDLE hList;

  if (hThis && (lpThis = Cell_MemLock(hThis)))
  {
    lpBook = Cell_GetSheet(hThis)->lpBook;
    for (hList = lpThis->hCellListeners; hList != NULL; hList = Cell_CellListenerList_GetNext(hList))
      Book_AddAdjustCell(lpBook, Cell_CellListenerList_GetCell(hList));
    for (hList = lpThis->hRowListeners; hList != NULL; hList = Cell_RowListenerList_GetNext(hList))
      Book_AddAdjustRow(lpBook, Cell_RowListenerList_GetRow(hList));
    for (hList = lpThis->hColListeners; hList != NULL; hList = Cell_ColListenerList_GetNext(hList))
      Book_AddAdjustCol(lpBook, Cell_ColListenerList_GetCol(hList));
    Cell_MemUnlock(hThis);
  }
}

//--------------------------------------------------------------------

void Cell_AddListenersToDirty(CALC_HANDLE hThis)
{
  LPCALC_CELL lpThis;
  LPCALC_BOOK lpBook;
  CALC_HANDLE hList;

  if (hThis && (lpThis = Cell_MemLock(hThis)))
  {
    lpBook = Cell_GetSheet(hThis)->lpBook;
    for (hList = lpThis->hCellListeners; hList != NULL; hList = Cell_CellListenerList_GetNext(hList))
      Book_AddDirtyCell(lpBook, Cell_CellListenerList_GetCell(hList));
    for (hList = lpThis->hRowListeners; hList != NULL; hList = Cell_RowListenerList_GetNext(hList))
      Book_AddDirtyRow(lpBook, Cell_RowListenerList_GetRow(hList));
    for (hList = lpThis->hColListeners; hList != NULL; hList = Cell_ColListenerList_GetNext(hList))
      Book_AddDirtyCol(lpBook, Cell_ColListenerList_GetCol(hList));
    Cell_MemUnlock(hThis);
  }
}

//--------------------------------------------------------------------

void Cell_IncrementDelayOfListeners(CALC_HANDLE hThis)
{
  LPCALC_CELL lpThis;
  CALC_HANDLE hList;

  if (hThis && (lpThis = Cell_MemLock(hThis)))
  {
    for (hList = lpThis->hCellListeners; hList != NULL; hList = Cell_CellListenerList_GetNext(hList))
    {
      CALC_HANDLE hCell = Cell_CellListenerList_GetCell(hList);
      Cell_SetDelay(hCell, Cell_GetDelay(hCell) + 1);
    }
    for (hList = lpThis->hRowListeners; hList != NULL; hList = Cell_RowListenerList_GetNext(hList))
    {
      CALC_HANDLE hRow = Cell_RowListenerList_GetRow(hList);
      Sheet_IncrementDelay(Row_GetSheet(hRow), CALC_ALLCOLS, Row_GetRow(hRow));
    }
    for (hList = lpThis->hColListeners; hList != NULL; hList = Cell_RowListenerList_GetNext(hList))
    {
      CALC_HANDLE hCol = Cell_ColListenerList_GetCol(hList);
      Sheet_IncrementDelay(Col_GetSheet(hCol), Col_GetCol(hCol), CALC_ALLROWS);
    }
    Cell_MemUnlock(hThis);
  }
}

//--------------------------------------------------------------------

void Cell_DecrementDelayOfListeners(CALC_HANDLE hThis)
{
  LPCALC_CELL lpThis;
  CALC_HANDLE hList;

  if (hThis && (lpThis = Cell_MemLock(hThis)))
  {
    for (hList = lpThis->hCellListeners; hList != NULL; hList = Cell_CellListenerList_GetNext(hList))
    {
      CALC_HANDLE hCell = Cell_CellListenerList_GetCell(hList);
      Cell_SetDelay(hCell, Cell_GetDelay(hCell) - 1);
    }
    for (hList = lpThis->hRowListeners; hList != NULL; hList = Cell_RowListenerList_GetNext(hList))
    {
      CALC_HANDLE hRow = Cell_RowListenerList_GetRow(hList);
      Sheet_DecrementDelay(Row_GetSheet(hRow), CALC_ALLCOLS, Row_GetRow(hRow));
    }
    for (hList = lpThis->hColListeners; hList != NULL; hList = Cell_RowListenerList_GetNext(hList))
    {
      CALC_HANDLE hCol = Cell_ColListenerList_GetCol(hList);
      Sheet_DecrementDelay(Col_GetSheet(hCol), Col_GetCol(hCol), CALC_ALLROWS);
    }
    Cell_MemUnlock(hThis);
  }
}

//--------------------------------------------------------------------

CALC_HANDLE Cell_CellListenerList_Create(CALC_HANDLE hCell, CALC_HANDLE hNext)
{
  LPCALC_CELL_CELLLISTENERLIST lpThis;
  CALC_HANDLE hThis;

  hThis = Cell_CellListenerList_MemAlloc();
  if (hThis && (lpThis = Cell_CellListenerList_MemLock(hThis)))
  {
    lpThis->hCell = hCell;
    lpThis->hNext = hNext;
    Cell_CellListenerList_MemUnlock(hThis);
  }
  return hThis;
}

//--------------------------------------------------------------------

void Cell_CellListenerList_Destroy(CALC_HANDLE hThis)
{
  LPCALC_CELL_CELLLISTENERLIST lpThis;

  if (hThis && (lpThis = Cell_CellListenerList_MemLock(hThis)))
  {
    Cell_CellListenerList_MemUnlock(hThis);
    Cell_CellListenerList_MemFree(hThis);
  }
}

//--------------------------------------------------------------------

CALC_HANDLE Cell_CellListenerList_GetCell(CALC_HANDLE hThis)
{
  LPCALC_CELL_CELLLISTENERLIST lpThis;
  CALC_HANDLE hValue = NULL;

  if (hThis && (lpThis = Cell_CellListenerList_MemLock(hThis)))
  {
    hValue = lpThis->hCell;
    Cell_CellListenerList_MemUnlock(hThis);
  }
  return hValue;
}

//--------------------------------------------------------------------

CALC_HANDLE Cell_CellListenerList_GetNext(CALC_HANDLE hThis)
{
  LPCALC_CELL_CELLLISTENERLIST lpThis;
  CALC_HANDLE hValue = NULL;

  if (hThis && (lpThis = Cell_CellListenerList_MemLock(hThis)))
  {
    hValue = lpThis->hNext;
    Cell_CellListenerList_MemUnlock(hThis);
  }
  return hValue;
}

//--------------------------------------------------------------------

void Cell_CellListenerList_SetNext(CALC_HANDLE hThis, CALC_HANDLE hValue)
{
  LPCALC_CELL_CELLLISTENERLIST lpThis;

  if (hThis && (lpThis = Cell_CellListenerList_MemLock(hThis)))
  {
    lpThis->hNext = hValue;
    Cell_CellListenerList_MemUnlock(hThis);
  }
}

//--------------------------------------------------------------------

CALC_HANDLE Cell_RowListenerList_Create(CALC_HANDLE hRow, CALC_HANDLE hNext)
{
  LPCALC_CELL_ROWLISTENERLIST lpThis;
  CALC_HANDLE hThis;

  hThis = Cell_RowListenerList_MemAlloc();
  if (hThis && (lpThis = Cell_RowListenerList_MemLock(hThis)))
  {
    lpThis->hRow = hRow;
    lpThis->hNext = hNext;
    Cell_RowListenerList_MemUnlock(hThis);
  }
  return hThis;
}

//--------------------------------------------------------------------

void Cell_RowListenerList_Destroy(CALC_HANDLE hThis)
{
  LPCALC_CELL_ROWLISTENERLIST lpThis;

  if (hThis && (lpThis = Cell_RowListenerList_MemLock(hThis)))
  {
    Cell_RowListenerList_MemUnlock(hThis);
    Cell_RowListenerList_MemFree(hThis);
  }
}

//--------------------------------------------------------------------

CALC_HANDLE Cell_RowListenerList_GetRow(CALC_HANDLE hThis)
{
  LPCALC_CELL_ROWLISTENERLIST lpThis;
  CALC_HANDLE hValue = NULL;

  if (hThis && (lpThis = Cell_RowListenerList_MemLock(hThis)))
  {
    hValue = lpThis->hRow;
    Cell_RowListenerList_MemUnlock(hThis);
  }
  return hValue;
}

//--------------------------------------------------------------------

CALC_HANDLE Cell_RowListenerList_GetNext(CALC_HANDLE hThis)
{
  LPCALC_CELL_ROWLISTENERLIST lpThis;
  CALC_HANDLE hValue = NULL;

  if (hThis && (lpThis = Cell_RowListenerList_MemLock(hThis)))
  {
    hValue = lpThis->hNext;
    Cell_RowListenerList_MemUnlock(hThis);
  }
  return hValue;
}

//--------------------------------------------------------------------

void Cell_RowListenerList_SetNext(CALC_HANDLE hThis, CALC_HANDLE hValue)
{
  LPCALC_CELL_ROWLISTENERLIST lpThis;

  if (hThis && (lpThis = Cell_RowListenerList_MemLock(hThis)))
  {
    lpThis->hNext = hValue;
    Cell_RowListenerList_MemUnlock(hThis);
  }
}

//--------------------------------------------------------------------

CALC_HANDLE Cell_ColListenerList_Create(CALC_HANDLE hCol, CALC_HANDLE hNext)
{
  LPCALC_CELL_COLLISTENERLIST lpThis;
  CALC_HANDLE hThis;

  hThis = Cell_ColListenerList_MemAlloc();
  if (hThis && (lpThis = Cell_ColListenerList_MemLock(hThis)))
  {
    lpThis->hCol = hCol;
    lpThis->hNext = hNext;
    Cell_ColListenerList_MemUnlock(hThis);
  }
  return hThis;
}

//--------------------------------------------------------------------

void Cell_ColListenerList_Destroy(CALC_HANDLE hThis)
{
  LPCALC_CELL_COLLISTENERLIST lpThis;

  if (hThis && (lpThis = Cell_ColListenerList_MemLock(hThis)))
  {
    Cell_ColListenerList_MemUnlock(hThis);
    Cell_ColListenerList_MemFree(hThis);
  }
}

//--------------------------------------------------------------------

CALC_HANDLE Cell_ColListenerList_GetCol(CALC_HANDLE hThis)
{
  LPCALC_CELL_COLLISTENERLIST lpThis;
  CALC_HANDLE hValue = NULL;

  if (hThis && (lpThis = Cell_ColListenerList_MemLock(hThis)))
  {
    hValue = lpThis->hCol;
    Cell_ColListenerList_MemUnlock(hThis);
  }
  return hValue;
}

//--------------------------------------------------------------------

CALC_HANDLE Cell_ColListenerList_GetNext(CALC_HANDLE hThis)
{
  LPCALC_CELL_COLLISTENERLIST lpThis;
  CALC_HANDLE hValue = NULL;

  if (hThis && (lpThis = Cell_ColListenerList_MemLock(hThis)))
  {
    hValue = lpThis->hNext;
    Cell_ColListenerList_MemUnlock(hThis);
  }
  return hValue;
}

//--------------------------------------------------------------------

void Cell_ColListenerList_SetNext(CALC_HANDLE hThis, CALC_HANDLE hValue)
{
  LPCALC_CELL_COLLISTENERLIST lpThis;

  if (hThis && (lpThis = Cell_ColListenerList_MemLock(hThis)))
  {
    lpThis->hNext = hValue;
    Cell_ColListenerList_MemUnlock(hThis);
  }
}

//--------------------------------------------------------------------

#endif // defined(SS_V70)

//--------------------------------------------------------------------
