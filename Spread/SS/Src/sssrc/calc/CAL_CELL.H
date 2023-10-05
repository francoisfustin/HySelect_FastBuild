//--------------------------------------------------------------------
//
//  File: calc_cell.h
//
//  Description: Cell formulas and data dependencies.
//

#ifndef CALC_CELL_H
#define CALC_CELL_H

#ifdef __cplusplus
extern "C" {
#endif

//--------------------------------------------------------------------
//
//  The CALC_CELL structure represents formulas and/or data
//  dependencies on individual cells.
//

typedef struct tagCALC_CELL
{
  LPCALC_SHEET lpSheet;   // sheet
  long lCol;              // column index
  long lRow;              // row index
  CALC_HANDLE hExpr;      // expression assigned to this cell
  #if defined(SS_V70)
  CALC_HANDLE hPreviousAdjust;  // previous cell that needs adjusting
  CALC_HANDLE hNextAdjust;      // next cell that needs adjusting
  CALC_HANDLE hPreviousDirty;   // previous cell that needs recalculating
  CALC_HANDLE hNextDirty;       // next cell that needs recalculating
  CALC_HANDLE hCellListeners;   // list of dependent cells
  CALC_HANDLE hRowListeners;    // list of dependent rows
  CALC_HANDLE hColListeners;    // list of dependnt columns
  long lDelay;
  #else
  BITMATRIX PrecedCells;  // cells used by this cell
  BITMATRIX DependCells;  // cells using this cell
  BITVECTOR DependCols;   // columns using this cell
  BITVECTOR DependRows;   // rows using this cell
  #endif
} CALC_CELL, FAR* LPCALC_CELL;

//--------------------------------------------------------------------

#define Cell_MemAlloc() (CALC_HANDLE)CalcMemAlloc(sizeof(CALC_CELL))
#define Cell_MemFree(hMem) CalcMemFree(hMem)
#define Cell_MemLock(hMem) (LPCALC_CELL)CalcMemLock(hMem)
#define Cell_MemUnlock(hMem) CalcMemUnlock(hMem)

//--------------------------------------------------------------------
//
//  The following functions are used to handle a table of cells.
//

CALC_HANDLE CellLookup(LPCALC_SHEET lpSheet, long lCol, long lRow);
CALC_HANDLE CellLookupAlloc(LPCALC_SHEET lpSheet, long lCol, long lRow);

BOOL CellSetLinksFromExpr(LPMATRIX lpTable, long lCol, long lRow, BOOL bLink);
CALC_HANDLE CellGetExpr(LPCALC_SHEET lpSheet, long lCol, long lRow);
CALC_HANDLE CellSetExpr(LPCALC_SHEET lpSheet, long lCol, long lRow,
                        CALC_HANDLE hExpr);

//--------------------------------------------------------------------
//
//  The following functions handle an individual cell.
//

CALC_HANDLE CellCreate(LPCALC_SHEET lpSheet, long lCol, long lRow);
BOOL CellDestroy(CALC_HANDLE hCell);
BOOL CellSetLinkToCell(CALC_HANDLE hCell, CALC_HANDLE hDepend, BOOL bLink);
BOOL CellSetLinkToCol(CALC_HANDLE hCell, CALC_HANDLE hDepend, BOOL bLink);
BOOL CellSetLinkToRow(CALC_HANDLE hCell, CALC_HANDLE hDepend, BOOL bLink);
BOOL CellMarkPreced(CALC_HANDLE hCell, LPBITMATRIX lpPrecedCells);
BOOL CellMarkDepend(CALC_HANDLE hCell, LPBITMATRIX lpDependCells,
                    LPBITVECTOR lpDependCols, LPBITVECTOR lpDependRows);

LPCALC_SHEET Cell_GetSheet(CALC_HANDLE hThis);
long Cell_GetRow(CALC_HANDLE hThis);
long Cell_GetCol(CALC_HANDLE hThis);

#if defined(SS_V70)

BOOL Cell_IsEmpty(CALC_HANDLE hThis);

CALC_HANDLE Cell_GetPreviousAdjust(CALC_HANDLE hThis);
void Cell_SetPreviousAdjust(CALC_HANDLE hThis, CALC_HANDLE hValue);
CALC_HANDLE Cell_GetNextAdjust(CALC_HANDLE hThis);
void Cell_SetNextAdjust(CALC_HANDLE hThis, CALC_HANDLE hValue);
CALC_HANDLE Cell_GetPreviousDirty(CALC_HANDLE hThis);
void Cell_SetPreviousDirty(CALC_HANDLE hThis, CALC_HANDLE hValue);
CALC_HANDLE Cell_GetNextDirty(CALC_HANDLE hThis);
void Cell_SetNextDirty(CALC_HANDLE hThis, CALC_HANDLE hValue);
long Cell_GetDelay(CALC_HANDLE hThis);
void Cell_SetDelay(CALC_HANDLE hThis, long lValue);

void Cell_StartListening(CALC_HANDLE hThis, BOOL bIncludeNames);
void Cell_StopListening(CALC_HANDLE hThis, BOOL bIncludeNames);

void Cell_AddCellListener(CALC_HANDLE hThis, CALC_HANDLE hCell);
void Cell_RemoveCellListener(CALC_HANDLE hThis, CALC_HANDLE hCell);
void Cell_AddRowListener(CALC_HANDLE hThis, CALC_HANDLE hRow);
void Cell_RemoveRowListener(CALC_HANDLE hThis, CALC_HANDLE hRow);
void Cell_AddColListener(CALC_HANDLE hThis, CALC_HANDLE hCol);
void Cell_RemoveColListener(CALC_HANDLE hThis, CALC_HANDLE hCol);

void Cell_AddListenersToAdjust(CALC_HANDLE hThis);
void Cell_AddListenersToDirty(CALC_HANDLE hThis);
void Cell_IncrementDelayOfListeners(CALC_HANDLE hThis);
void Cell_DecrementDelayOfListeners(CALC_HANDLE hThis);

//--------------------------------------------------------------------

typedef struct tagCALC_CELL_CELLLISTENERLIST
{
  CALC_HANDLE hCell;
  CALC_HANDLE hNext;
} CALC_CELL_CELLLISTENERLIST, FAR* LPCALC_CELL_CELLLISTENERLIST;

//--------------------------------------------------------------------

#define Cell_CellListenerList_MemAlloc() (CALC_HANDLE)CalcMemAlloc(sizeof(CALC_CELL_CELLLISTENERLIST))
#define Cell_CellListenerList_MemFree(hMem) CalcMemFree(hMem)
#define Cell_CellListenerList_MemLock(hMem) (LPCALC_CELL_CELLLISTENERLIST)CalcMemLock(hMem)
#define Cell_CellListenerList_MemUnlock(hMem) CalcMemUnlock(hMem)

//--------------------------------------------------------------------

CALC_HANDLE Cell_CellListenerList_Create(CALC_HANDLE hCell, CALC_HANDLE hNext);
void Cell_CellListenerList_Destroy(CALC_HANDLE hThis);
CALC_HANDLE Cell_CellListenerList_GetCell(CALC_HANDLE hThis);
CALC_HANDLE Cell_CellListenerList_GetNext(CALC_HANDLE hThis);
void Cell_CellListenerList_SetNext(CALC_HANDLE hThis, CALC_HANDLE hValue);

//--------------------------------------------------------------------

typedef struct tagCALC_CELL_ROWLISTENERLIST
{
  CALC_HANDLE hRow;
  CALC_HANDLE hNext;
} CALC_CELL_ROWLISTENERLIST, FAR* LPCALC_CELL_ROWLISTENERLIST;

//--------------------------------------------------------------------

#define Cell_RowListenerList_MemAlloc() (CALC_HANDLE)CalcMemAlloc(sizeof(CALC_CELL_ROWLISTENERLIST))
#define Cell_RowListenerList_MemFree(hMem) CalcMemFree(hMem)
#define Cell_RowListenerList_MemLock(hMem) (LPCALC_CELL_ROWLISTENERLIST)CalcMemLock(hMem)
#define Cell_RowListenerList_MemUnlock(hMem) CalcMemUnlock(hMem)

//--------------------------------------------------------------------

CALC_HANDLE Cell_RowListenerList_Create(CALC_HANDLE hRow, CALC_HANDLE hNext);
void Cell_RowListenerList_Destroy(CALC_HANDLE hThis);
CALC_HANDLE Cell_RowListenerList_GetRow(CALC_HANDLE hThis);
CALC_HANDLE Cell_RowListenerList_GetNext(CALC_HANDLE hThis);
void Cell_RowListenerList_SetNext(CALC_HANDLE hThis, CALC_HANDLE hValue);

//--------------------------------------------------------------------

typedef struct tagCALC_CELL_COLLISTENERLIST
{
  CALC_HANDLE hCol;
  CALC_HANDLE hNext;
} CALC_CELL_COLLISTENERLIST, FAR* LPCALC_CELL_COLLISTENERLIST;

//--------------------------------------------------------------------

#define Cell_ColListenerList_MemAlloc() (CALC_HANDLE)CalcMemAlloc(sizeof(CALC_CELL_COLLISTENERLIST))
#define Cell_ColListenerList_MemFree(hMem) CalcMemFree(hMem)
#define Cell_ColListenerList_MemLock(hMem) (LPCALC_CELL_COLLISTENERLIST)CalcMemLock(hMem)
#define Cell_ColListenerList_MemUnlock(hMem) CalcMemUnlock(hMem)

//--------------------------------------------------------------------

CALC_HANDLE Cell_ColListenerList_Create(CALC_HANDLE hCol, CALC_HANDLE hNext);
void Cell_ColListenerList_Destroy(CALC_HANDLE hThis);
CALC_HANDLE Cell_ColListenerList_GetCol(CALC_HANDLE hThis);
CALC_HANDLE Cell_ColListenerList_GetNext(CALC_HANDLE hThis);
void Cell_ColListenerList_SetNext(CALC_HANDLE hThis, CALC_HANDLE hValue);

#endif  // defined(SS_V70)

//--------------------------------------------------------------------

#ifdef __cplusplus
}
#endif

#endif
