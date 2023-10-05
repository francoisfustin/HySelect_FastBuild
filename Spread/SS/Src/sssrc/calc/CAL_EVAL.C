//--------------------------------------------------------------------
//
//  File: cal_eval.c
//
//  Description: Evaluation of cells
//

#include <math.h>

#include "calc.h"
#include "cal_cell.h"
#include "cal_col.h"
#include "cal_row.h"
#include "cal_expr.h"
#include "cal_func.h"
#include "cal_mem.h"

//--------------------------------------------------------------------
//
//  The MarkDeepDepend() function searchs for all dependents (direct
//  or indirect) of the the currently marked cells, columns, and
//  rows.  All dependents are added to the list of marked cells.
//

#if !defined(SS_V70)
static void MarkDeepDepend(LPCALC_INFO lpCalc)
{
  BITMATRIX search;
  CALC_HANDLE hCell;
  CALC_HANDLE hCol;
  CALC_HANDLE hRow;
  long lCol;
  long lRow;
  long lMaxRows;
  long lMaxCols;

  lpCalc->lpfnGetDataCnt(lpCalc->hSS, &lMaxCols, &lMaxRows);  
  search = lpCalc->CellsToEval;
  BitMatInit(&lpCalc->CellsToEval);
  while( BitVectRemoveFirst(&lpCalc->ColsToEval, &lCol) )
  {
    //???? this code bindly marks cells which have a cell or row formula ????
    for( lRow = 1; lRow <= lMaxRows; lRow++ )
      BitMatSet(&search, lCol, lRow, TRUE);
  }
  while( BitVectRemoveFirst(&lpCalc->RowsToEval, &lRow) )
  {
    //???? this code bindly marks cells which have a cell formula ????
    for( lCol = 1; lCol <= lMaxCols; lCol++ )
      BitMatSet(&search, lCol, lRow, TRUE);
  }
  while( BitMatRemoveFirst(&search, &lCol, &lRow) )
  {
    if( !BitMatSet(&lpCalc->CellsToEval, lCol, lRow, TRUE) )
    {
      if( hCell = MatGet(&lpCalc->Cells, lCol, lRow) )
        CellMarkDepend(hCell, &search, &lpCalc->ColsToEval, &lpCalc->RowsToEval);
      if( hCol = VectGet(&lpCalc->Cols, lCol) )
        ColMarkDepend(hCol, lRow, &search);
      if( hRow = VectGet(&lpCalc->Rows, lRow) )
        RowMarkDepend(hRow, lCol, &search);
      while( BitVectRemoveFirst(&lpCalc->ColsToEval, &lCol) )
      {
        for( lRow = 1; lRow <= lMaxRows; lRow++ )
          BitMatSet(&search, lCol, lRow, TRUE);
      }
      while( BitVectRemoveFirst(&lpCalc->RowsToEval, &lRow) )
      {
        for( lCol = 1; lCol <= lMaxCols; lCol++ )
          BitMatSet(&search, lCol, lRow, TRUE);
      }
    }
  }
}
#endif

//--------------------------------------------------------------------
//
//  The PrecedInList() function determines if any of the precedent
//  cells are in the list.
//

#if !defined(SS_V70)
static BOOL PrecedInList(LPMATRIX lpCellTable,
                         LPVECTOR lpColTable, LPVECTOR lpRowTable,
                         long lCol, long lRow,
                         LPBITMATRIX lpList)
{
  CALC_HANDLE hExpr = 0;
  CALC_HANDLE hCell;
  CALC_HANDLE hCol;
  CALC_HANDLE hRow;
  LPCALC_CELL lpCell;
  LPCALC_COL lpCol;
  LPCALC_ROW lpRow;
  BOOL bReturn = FALSE;
  
  if( (hCell = MatGet(lpCellTable, lCol, lRow))
      && (lpCell = (LPCALC_CELL)CalcMemLock(hCell)) )
  {
    if( hExpr = lpCell->hExpr )
      bReturn = BitMatHasInter(&lpCell->PrecedCells, lpList);
    CalcMemUnlock(hCell);
  }
  if( (!hExpr)
      && (hRow = VectGet(lpRowTable, lRow))
      && (lpRow = (LPCALC_ROW)CalcMemLock(hRow)) )
  {
    if( hExpr = lpRow->hExpr )
      bReturn = BitMatHasInter(&lpRow->AbsPrecedCells, lpList)
                || BitMatHasInterOffset(&lpRow->RelPrecedCells, lpList,
                                        lCol - REL_OFFSET, 0);
    CalcMemUnlock(hRow);
  }
  if( (!hExpr)
      && (hCol = VectGet(lpColTable, lCol))
      && (lpCol = (LPCALC_COL)CalcMemLock(hCol)) )
  {
    if( hExpr = lpCol->hExpr )
      bReturn = BitMatHasInter(&lpCol->AbsPrecedCells, lpList)
                || BitMatHasInterOffset(&lpCol->RelPrecedCells, lpList,
                                        0, lRow - REL_OFFSET);
    CalcMemUnlock(hCol);
  }
  return bReturn;
}
#endif

//--------------------------------------------------------------------
//
//  The FindCellToEval() function searchs for a cell which meets the
//  following criteria...
//      1) needs to be evaluated
//      2) has no precedent cells which need to be evaluated
//

#if !defined(SS_V70)
static BOOL FindCellToEval(LPCALC_INFO lpCalc, LPLONG lplCol, LPLONG lplRow)
{
  BOOL bContinue;
  
  bContinue = BitMatGetFirst(&lpCalc->CellsToEval, lplCol, lplRow);
  while( bContinue )
  {
    if( !PrecedInList(&lpCalc->Cells, &lpCalc->Cols, &lpCalc->Rows,
                      *lplCol, *lplRow, &lpCalc->CellsToEval) )
      return TRUE;
    bContinue = BitMatGetNext(&lpCalc->CellsToEval, lplCol, lplRow);
  }
  return FALSE;
}
#endif

//--------------------------------------------------------------------
//
//  The CalcEvalCell() function evaluates an individual cell.
//

BOOL CALCAPI CalcEvalCell(LPCALC_SHEET lpSheet, long lCol, long lRow)
{
  CALC_VALUE result;
  CALC_HANDLE hExpr = 0;
  CALC_HANDLE hCell;
  CALC_HANDLE hCol;
  CALC_HANDLE hRow;
  LPCALC_CELL lpCell;
  LPCALC_COL lpCol;
  LPCALC_ROW lpRow;
  BOOL bReturn = FALSE;
  
  if( hCell = CellLookup(lpSheet, lCol, lRow) )
  {
    if( lpCell = (LPCALC_CELL)CalcMemLock(hCell) )
    {
      hExpr = lpCell->hExpr;
      CalcMemUnlock(hCell);
    }
  }
  if( !hExpr && lCol != 0 && (hRow = RowLookup(lpSheet, lRow)) )
  {
    if( lpRow = (LPCALC_ROW)CalcMemLock(hRow) )
    {
      hExpr = lpRow->hExpr;
      CalcMemUnlock(hRow);
    }
  }
  if( !hExpr && lRow != 0 && (hCol = ColLookup(lpSheet, lCol)) )
  {
    if( lpCol = (LPCALC_COL)CalcMemLock(hCol) )
    {
      hExpr = lpCol->hExpr;
      CalcMemUnlock(hCol);
    }
  }
  if( hExpr && ExprEval(hExpr, lpSheet, lCol, lRow, &result) )
  {
    lpSheet->lpfnSetData(lpSheet->hSS, lCol, lRow, &result);
    ValFree(&result);
    bReturn = TRUE;
  }
  return bReturn;
}

//--------------------------------------------------------------------

#if defined(SS_V70)
void Sheet_AddDirtyCellsFromRow(LPCALC_SHEET lpThis, long lRow)
{
  long lCol;
  long lColCnt;
  lpThis->lpfnGetDataCnt(lpThis->hSS, &lColCnt, NULL);
  for (lCol = 1; lCol <= lColCnt; lCol++)
  {
    CALC_HANDLE hCell = CellLookupAlloc(lpThis, lCol, lRow);
    Book_AddDirtyCell(lpThis->lpBook, hCell);
  }
}
#endif

//--------------------------------------------------------------------

#if defined(SS_V70)
void Sheet_AddDirtyCellsFromCol(LPCALC_SHEET lpThis, long lCol)
{
  long lRow;
  long lRowCnt;
  lpThis->lpfnGetDataCnt(lpThis->hSS, NULL, &lRowCnt);
  for (lRow = 1; lRow <= lRowCnt; lRow++)
  {
    CALC_HANDLE hCell = CellLookupAlloc(lpThis, lCol, lRow);
    Book_AddDirtyCell(lpThis->lpBook, hCell);
  }
}
#endif

//--------------------------------------------------------------------

double IterationChange(LPCALC_VALUE lpOldValue, LPCALC_VALUE lpNewValue)
{
  double dfOldDouble = ValGetDouble(lpOldValue);
  double dfNewDouble = ValGetDouble(lpNewValue);
  return fabs(dfNewDouble - dfOldDouble);
}

//--------------------------------------------------------------------
//
//  The CalcEvalNeededCells() function evaluates all cells which need
//  to be recalculated.
//

BOOL CALCAPI CalcEvalNeededCells(LPCALC_BOOK lpBook)
#if defined(SS_V70)
{
  CALC_HANDLE hCell;
  BOOL bProgress = FALSE;
  
  while (lpBook->hHeadDirtyRows != NULL)
  {
    Sheet_AddDirtyCellsFromRow(Row_GetSheet(lpBook->hHeadDirtyRows), Row_GetRow(lpBook->hHeadDirtyRows));
    Book_RemoveDirtyRow(lpBook, lpBook->hHeadDirtyRows);
  }
  while (lpBook->hHeadDirtyCols != NULL)
  {
    Sheet_AddDirtyCellsFromCol(Col_GetSheet(lpBook->hHeadDirtyCols), Col_GetCol(lpBook->hHeadDirtyCols));
    Book_RemoveDirtyCol(lpBook, lpBook->hHeadDirtyCols);
  }
  for (hCell = lpBook->hHeadDirtyCells; hCell != NULL; hCell = Cell_GetNextDirty(hCell))
  {
    LPCALC_SHEET lpSheet = Cell_GetSheet(hCell);
    CALC_HANDLE hRow = RowLookup(lpSheet, Cell_GetRow(hCell));
    CALC_HANDLE hCol = ColLookup(lpSheet, Cell_GetCol(hCell));
    Cell_AddListenersToDirty(hCell);
    if (hRow != NULL)
      Row_AddListenersToDirty(hRow, Cell_GetCol(hCell));
    if (hCol != NULL)
      Col_AddListenersToDirty(hCol, Cell_GetRow(hCell));
    Sheet_AddListenersToDirty(lpSheet, Cell_GetCol(hCell), Cell_GetRow(hCell));
    while (lpBook->hHeadDirtyRows != NULL)
    {
      Sheet_AddDirtyCellsFromRow(Row_GetSheet(lpBook->hHeadDirtyRows), Row_GetRow(lpBook->hHeadDirtyRows));
      Book_RemoveDirtyRow(lpBook, lpBook->hHeadDirtyRows);
    }
    while (lpBook->hHeadDirtyCols != NULL)
    {
      Sheet_AddDirtyCellsFromCol(Col_GetSheet(lpBook->hHeadDirtyCols), Col_GetCol(lpBook->hHeadDirtyCols));
      Book_RemoveDirtyCol(lpBook, lpBook->hHeadDirtyCols);
    }
  }
  for (hCell = lpBook->hHeadDirtyCells; hCell != NULL; hCell = Cell_GetNextDirty(hCell))
  {
    Cell_SetDelay(hCell, 0);
  }
  for (hCell = lpBook->hHeadDirtyCells; hCell != NULL; hCell = Cell_GetNextDirty(hCell))
  {
    LPCALC_SHEET lpSheet = Cell_GetSheet(hCell);
    CALC_HANDLE hRow = RowLookup(lpSheet, Cell_GetRow(hCell));
    CALC_HANDLE hCol = ColLookup(lpSheet, Cell_GetCol(hCell));
    Cell_IncrementDelayOfListeners(hCell);
    if (hRow != NULL)
      Row_IncrementDelayOfListeners(hRow, Cell_GetCol(hCell));
    if (hCol != NULL)
      Col_IncrementDelayOfListeners(hCol, Cell_GetRow(hCell));
    Sheet_IncrementDelayOfListeners(lpSheet, Cell_GetCol(hCell), Cell_GetRow(hCell));
  }
  do
  {
    CALC_HANDLE hNextDirty = NULL;
    bProgress = FALSE;
    for (hCell = lpBook->hHeadDirtyCells; hCell != NULL; hCell = hNextDirty)
    {
      hNextDirty = Cell_GetNextDirty(hCell);
      if (Cell_GetDelay(hCell) == 0)
      {
        LPCALC_SHEET lpSheet = Cell_GetSheet(hCell);
        CALC_HANDLE hRow = RowLookup(lpSheet, Cell_GetRow(hCell));
        CALC_HANDLE hCol = ColLookup(lpSheet, Cell_GetCol(hCell));
        CalcEvalCell(lpSheet, Cell_GetCol(hCell), Cell_GetRow(hCell));
        Book_RemoveDirtyCell(lpBook, hCell);
        Cell_DecrementDelayOfListeners(hCell);
        if (hRow != NULL)
          Row_DecrementDelayOfListeners(hRow, Cell_GetCol(hCell));
        if (hCol != NULL)
          Col_DecrementDelayOfListeners(hCol, Cell_GetRow(hCell));
        Sheet_DecrementDelayOfListeners(lpSheet, Cell_GetCol(hCell), Cell_GetRow(hCell));
        bProgress = TRUE;
      }
    }
  } while (bProgress);
  if (lpBook->hHeadDirtyCells != NULL)
  {
    if (lpBook->bIteration)
    {
      BOOL bMoreIterations = TRUE;
      int i;
      for (i = 0; bMoreIterations && i < lpBook->nMaxIterations; i++)
      {
        bMoreIterations = FALSE;
        for (hCell = lpBook->hHeadDirtyCells; hCell != NULL; hCell = Cell_GetNextDirty(hCell))
        {
          LPCALC_SHEET lpSheet = Cell_GetSheet(hCell);
          CALC_VALUE oldValue;
          CALC_VALUE newValue;
          lpSheet->lpfnGetData(lpSheet->hSS, Cell_GetCol(hCell), Cell_GetRow(hCell), &oldValue);
          CalcEvalCell(lpSheet, Cell_GetCol(hCell), Cell_GetRow(hCell));
          lpSheet->lpfnGetData(lpSheet->hSS, Cell_GetCol(hCell), Cell_GetRow(hCell), &newValue);
          if (IterationChange(&oldValue, &newValue) >= lpBook->dfMaxChange)
            bMoreIterations = TRUE;
          ValFree(&oldValue);
          ValFree(&newValue);
        }
      }
    }
    #if defined(SS_V80)
    else
    {
      for (hCell = lpBook->hHeadDirtyCells; hCell != NULL; hCell = Cell_GetNextDirty(hCell))
      {
        LPCALC_SHEET lpSheet = Cell_GetSheet(hCell);
        lpBook->lpfnSendCircularFormulaMsg(lpBook->hBook, (short)lpSheet->lpfnGetIndex(lpSheet->hSS), Cell_GetCol(hCell), Cell_GetRow(hCell));
      }
    }
    #endif
  }
  return TRUE;
}
#else
{
  LPCALC_SHEET lpSheet = lpBook->lpfnGetSheetFromIndex(lpBook->hBook, 0);
  BOOL bMoreIterations;
  BOOL bContinue;
  long lCol;
  long lRow;
  int i;

  if (lpSheet == NULL)
    return TRUE;

  MarkDeepDepend(lpSheet);
  // update all non-recursive cells
  while( FindCellToEval(lpSheet, &lCol, &lRow) )
  {
    BitMatSet(&lpSheet->CellsToEval, lCol, lRow, FALSE);
    CalcEvalCell(lpSheet, lCol, lRow);
  }
  // update all recursive cells
  if( lpBook->bIteration )
  {
    bMoreIterations = TRUE;
    for( i = 0; bMoreIterations && i < lpBook->nMaxIterations; i++ )
    {
      bMoreIterations = FALSE;
      bContinue = BitMatGetFirst(&lpSheet->CellsToEval, &lCol, &lRow);
      while( bContinue )
      {
        CALC_VALUE oldValue;
        CALC_VALUE newValue;
        lpSheet->lpfnGetData(lpSheet->hSS, lCol, lRow, &oldValue);
        CalcEvalCell(lpSheet, lCol, lRow);
        lpSheet->lpfnGetData(lpSheet->hSS, lCol, lRow, &newValue);
        if (IterationChange(&oldValue, &newValue) >= lpBook->dfMaxChange)
          bMoreIterations = TRUE;
        ValFree(&oldValue);
        ValFree(&newValue);
        bContinue = BitMatGetNext(&lpSheet->CellsToEval, &lCol, &lRow);
      }
    }
  }
  return TRUE;
}
#endif

//--------------------------------------------------------------------
//
//  The CalcEvalAllCells() function evaluates all cells which contain
//  an expression.
//

BOOL CALCAPI CalcEvalAllCells(LPCALC_BOOK lpBook)
#if defined(SS_V70)
{
  short nSheetCnt = lpBook->lpfnGetSheetCnt(lpBook->hBook);
  short i;

  for (i = 0; i < nSheetCnt; i++)
  {
    LPCALC_SHEET lpSheet = lpBook->lpfnGetSheetFromIndex(lpBook->hBook, i);
    CALC_HANDLE hCell;
    CALC_HANDLE hCol;
    CALC_HANDLE hRow;
    long lCol;
    long lRow;

    hCell = MatGetFirst(&lpSheet->Cells, &lCol, &lRow);
    while( hCell )
    {
      if( CellGetExpr(lpSheet, lCol, lRow) )
        CalcMarkCellForEval(lpSheet, lCol, lRow);
      hCell = MatGetNext(&lpSheet->Cells, &lCol, &lRow);
    }
    hCol = VectGetFirst(&lpSheet->Cols, &lCol);
    while( hCol )
    {
      if( ColGetExpr(lpSheet, lCol) )
        CalcMarkColForEval(lpSheet, lCol);
      hCol = VectGetNext(&lpSheet->Cols, &lCol);
    }
    hRow = VectGetFirst(&lpSheet->Rows, &lRow);
    while( hRow )
    {
      if( RowGetExpr(lpSheet, lRow) )
        CalcMarkRowForEval(lpSheet, lRow);
      hRow = VectGetNext(&lpSheet->Rows, &lRow);
    }
  }
  CalcEvalNeededCells(lpBook);
  return TRUE;
}
#else
{
  LPCALC_SHEET lpSheet = lpBook->lpfnGetSheetFromIndex(lpBook->hBook, 0);
  CALC_HANDLE hCell;
  CALC_HANDLE hCol;
  CALC_HANDLE hRow;
  long lCol;
  long lRow;

  hCell = MatGetFirst(&lpSheet->Cells, &lCol, &lRow);
  while( hCell )
  {
    if( CellGetExpr(lpSheet, lCol, lRow) )
      BitMatSet(&lpSheet->CellsToEval, lCol, lRow, TRUE);
    hCell = MatGetNext(&lpSheet->Cells, &lCol, &lRow);
  }
  hCol = VectGetFirst(&lpSheet->Cols, &lCol);
  while( hCol )
  {
    if( ColGetExpr(lpSheet, lCol) )
      BitVectSet(&lpSheet->ColsToEval, lCol, TRUE);
    hCol = VectGetNext(&lpSheet->Cols, &lCol);
  }
  hRow = VectGetFirst(&lpSheet->Rows, &lRow);
  while( hRow )
  {
    if( RowGetExpr(lpSheet, lRow) )
      BitVectSet(&lpSheet->RowsToEval, lRow, TRUE);
    hRow = VectGetNext(&lpSheet->Rows, &lRow);
  }
  CalcEvalNeededCells(lpBook);
  return TRUE;
}
#endif

//--------------------------------------------------------------------
//
//  The CalcIsCellMarkedForEval() function determines if the cell is
//  flaged to be evaluated during the next calculation iteration.
//

BOOL CALCAPI CalcIsCellMarkedForEval(LPCALC_SHEET lpSheet, long lCol, long lRow)
#if defined(SS_V70)
{
  CALC_HANDLE hCell;

  hCell = CellLookupAlloc(lpSheet, lCol, lRow);
  return Book_IsDirtyCell(lpSheet->lpBook, hCell);
}
#else
{
  return BitMatGet(&lpSheet->CellsToEval, lCol, lRow);
}
#endif

//--------------------------------------------------------------------
//
//  The CalcIsColMarkedForEval() function determines if the column is
//  flaged to be evaluated during the next calculation iteration.
//

BOOL CALCAPI CalcIsColMarkedForEval(LPCALC_SHEET lpSheet, long lCol)
#if defined(SS_V70)
{
  CALC_HANDLE hCol;

  hCol = ColLookupAlloc(lpSheet, lCol);
  return Book_IsDirtyCol(lpSheet->lpBook, hCol);
}
#else
{
  return BitVectGet(&lpSheet->ColsToEval, lCol);
}
#endif

//--------------------------------------------------------------------
//
//  The CalcIsRowMarkedForEval() function determines if the row is
//  flaged to be evaluated during the next calculation iteration.
//

BOOL CALCAPI CalcIsRowMarkedForEval(LPCALC_SHEET lpSheet, long lRow)
#if defined(SS_V70)
{
  CALC_HANDLE hRow;

  hRow = RowLookupAlloc(lpSheet, lRow);
  return Book_IsDirtyRow(lpSheet->lpBook, hRow);
}
#else
{
  return BitVectGet(&lpSheet->RowsToEval, lRow);
}
#endif

//--------------------------------------------------------------------
//
//  The CalcMarkCellForEval() function flags the cell to be evaluated
//  during the next calculation iteration.
//

BOOL CALCAPI CalcMarkCellForEval(LPCALC_SHEET lpSheet, long lCol, long lRow)
#if defined(SS_V70)
{
  CALC_HANDLE hCell;

  hCell = CellLookupAlloc(lpSheet, lCol, lRow);
  Book_AddDirtyCell(lpSheet->lpBook, hCell);
  return TRUE;
}
#else
{
  BitMatSet(&lpSheet->CellsToEval, lCol, lRow, TRUE);
  return TRUE;
}
#endif

//--------------------------------------------------------------------
//
//  The CalcMarkColForEval() function flags the column to be evaluated
//  during the next calculation iteration.
//

BOOL CALCAPI CalcMarkColForEval(LPCALC_SHEET lpSheet, long lCol)
#if defined(SS_V70)
{
  CALC_HANDLE hCol;

  hCol = ColLookupAlloc(lpSheet, lCol);
  Book_AddDirtyCol(lpSheet->lpBook, hCol);
  return TRUE;
}
#else
{
  BitVectSet(&lpSheet->ColsToEval, lCol, TRUE);
  return TRUE;
}
#endif

//--------------------------------------------------------------------
//
//  The CalcMarkRowForEval() function flags the row to be evaluated
//  during the next calculation iteration.
//

BOOL CALCAPI CalcMarkRowForEval(LPCALC_SHEET lpSheet, long lRow)
#if defined(SS_V70)
{
  CALC_HANDLE hRow;

  hRow = RowLookupAlloc(lpSheet, lRow);
  Book_AddDirtyRow(lpSheet->lpBook, hRow);
  return TRUE;
}
#else
{
  BitVectSet(&lpSheet->RowsToEval, lRow, TRUE);
  return TRUE;
}
#endif

//--------------------------------------------------------------------
//
//  The CalcMarkDependForEval() function flags the cell's dependent
//  cells to be evaluated during the next calculation iteration.
//

BOOL CALCAPI CalcMarkDependForEval(LPCALC_SHEET lpSheet, long lCol, long lRow)
#if defined(SS_V70)
{
  CALC_HANDLE hCell;
  CALC_HANDLE hRow;
  CALC_HANDLE hCol;

  hCell = CellLookup(lpSheet, lCol, lRow);
  hRow = RowLookup(lpSheet, lRow);
  hCol = ColLookup(lpSheet, lCol);
  Cell_AddListenersToDirty(hCell);
  Row_AddListenersToDirty(hRow, lCol);
  Col_AddListenersToDirty(hCol, lRow);
  Sheet_AddListenersToDirty(lpSheet, lCol, lRow);
  return TRUE;
}
#else
{
  CALC_HANDLE hCell;
  CALC_HANDLE hCol;
  CALC_HANDLE hRow;
  BOOL bReturn = FALSE;
  
  if( lCol != CALC_ALLCOLS && lRow != CALC_ALLROWS )
  {
    if( hCell = MatGet(&lpSheet->Cells, lCol, lRow) )
      CellMarkDepend(hCell, &lpSheet->CellsToEval,
                     &lpSheet->ColsToEval, &lpSheet->RowsToEval
                     );
    if( hCol = VectGet(&lpSheet->Cols, lCol) )
      ColMarkDepend(hCol, lRow, &lpSheet->CellsToEval);
    if( hRow = VectGet(&lpSheet->Rows, lRow) )
      RowMarkDepend(hRow, lCol, &lpSheet->CellsToEval);
    bReturn = TRUE;
  }
  return bReturn;
}
#endif