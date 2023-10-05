//--------------------------------------------------------------------
//
//  File: cal_row.h
//
//  Description: Row formulas and data dependencies.
//

#ifndef CAL_ROW_H
#define CAL_ROW_H

#ifdef __cplusplus
extern "C" {
#endif

//--------------------------------------------------------------------
//
//  The CALC_ROW structure represents formulas and/or data
//  dependencies on individual rows.
//

typedef struct tagCALC_ROW
{
  LPCALC_SHEET lpSheet;
  long lRow;                 // row index
  CALC_HANDLE hExpr;         // expression assigned to this row
  #if defined(SS_V70)
  CALC_HANDLE hPreviousAdjust;  // previous row that needs adjusting
  CALC_HANDLE hNextAdjust;      // next row that needs adjusting
  CALC_HANDLE hPreviousDirty;   // previous column that needs recalculating
  CALC_HANDLE hNextDirty;       // next row that needs recalculating
  CALC_HANDLE hRowListeners;    // list of dependent rows
  #else
  BITMATRIX AbsPrecedCells;  // cells (absolute location) used by this row
  BITMATRIX RelPrecedCells;  // cells (relative location) used by this row
  BITMATRIX RelDependCells;  // cells (relative location) using this row
  #endif
} CALC_ROW, FAR* LPCALC_ROW;

//--------------------------------------------------------------------

#define Row_MemAlloc() (CALC_HANDLE)CalcMemAlloc(sizeof(CALC_ROW))
#define Row_MemFree(hMem) CalcMemFree(hMem)
#define Row_MemLock(hMem) (LPCALC_ROW)CalcMemLock(hMem)
#define Row_MemUnlock(hMem) CalcMemUnlock(hMem)

//--------------------------------------------------------------------
//
//  The following function handle a table of rows.
//

CALC_HANDLE RowLookup(LPCALC_SHEET lpSheet, long lRow);
CALC_HANDLE RowLookupAlloc(LPCALC_SHEET lpSheet, long lRow);

BOOL RowSetLinksFromExpr(LPMATRIX lpCellTable, LPVECTOR lpRowTable, long lRow,
                         BOOL bLink);
CALC_HANDLE RowGetExpr(LPCALC_SHEET lpSheet, long lRow);
CALC_HANDLE RowSetExpr(LPCALC_SHEET lpSheet, long lRow, CALC_HANDLE hExpr);

//--------------------------------------------------------------------
//
//  The following functions are used to handle individual rows.
//

CALC_HANDLE RowCreate(LPCALC_SHEET lpSheet, long lRow);
BOOL RowDestroy(CALC_HANDLE hRow);
BOOL RowSetLinkToRow(CALC_HANDLE hRow, CALC_HANDLE hDepend, long lCol,
                     BOOL bLink);
BOOL RowMarkDepend(CALC_HANDLE hRow, long lCol, LPBITMATRIX lpDepend);

LPCALC_SHEET Row_GetSheet(CALC_HANDLE hThis);
long Row_GetRow(CALC_HANDLE hThis);

#if defined(SS_V70)

BOOL Row_IsEmpty(CALC_HANDLE hThis);

CALC_HANDLE Row_GetPreviousAdjust(CALC_HANDLE hThis);
void Row_SetPreviousAdjust(CALC_HANDLE hThis, CALC_HANDLE hValue);
CALC_HANDLE Row_GetNextAdjust(CALC_HANDLE hThis);
void Row_SetNextAdjust(CALC_HANDLE hThis, CALC_HANDLE hValue);
CALC_HANDLE Row_GetPreviousDirty(CALC_HANDLE hThis);
void Row_SetPreviousDirty(CALC_HANDLE hThis, CALC_HANDLE hValue);
CALC_HANDLE Row_GetNextDirty(CALC_HANDLE hThis);
void Row_SetNextDirty(CALC_HANDLE hThis, CALC_HANDLE hValue);

void Row_StartListening(CALC_HANDLE hThis, BOOL bIncludeNames);
void Row_StopListening(CALC_HANDLE hThis, BOOL bIncludeNames);

void Row_AddRowListener(CALC_HANDLE hThis, CALC_HANDLE hRow, long lColOffset);
void Row_RemoveRowListener(CALC_HANDLE hThis, CALC_HANDLE hRow, long lColOffset);

void Row_AddListenersToAdjust(CALC_HANDLE hThis);
void Row_AddListenersToDirty(CALC_HANDLE hThis, long lCol);
void Row_IncrementDelayOfListeners(CALC_HANDLE hThis, long lCol);
void Row_DecrementDelayOfListeners(CALC_HANDLE hThis, long lCol);

//--------------------------------------------------------------------

typedef struct tagCALC_ROW_ROWLISTENERLIST
{
  CALC_HANDLE hRow;
  long lColOffset;
  CALC_HANDLE hNext;
} CALC_ROW_ROWLISTENERLIST, FAR* LPCALC_ROW_ROWLISTENERLIST;

//--------------------------------------------------------------------

#define Row_RowListenerList_MemAlloc() (CALC_HANDLE)CalcMemAlloc(sizeof(CALC_ROW_ROWLISTENERLIST))
#define Row_RowListenerList_MemFree(hMem) CalcMemFree(hMem)
#define Row_RowListenerList_MemLock(hMem) (LPCALC_ROW_ROWLISTENERLIST)CalcMemLock(hMem)
#define Row_RowListenerList_MemUnlock(hMem) CalcMemUnlock(hMem)

//--------------------------------------------------------------------

CALC_HANDLE Row_RowListenerList_Create(CALC_HANDLE hRow, long lColOffset, CALC_HANDLE hNext);
void Row_RowListenerList_Destroy(CALC_HANDLE hThis);
CALC_HANDLE Row_RowListenerList_GetRow(CALC_HANDLE hThis);
long Row_RowListenerList_GetColOffset(CALC_HANDLE hThis);
CALC_HANDLE Row_RowListenerList_GetNext(CALC_HANDLE hThis);
void Row_RowListenerList_SetNext(CALC_HANDLE hThis, CALC_HANDLE hValue);

#endif  // defined(SS_V70)

//--------------------------------------------------------------------

#ifdef __cplusplus
}
#endif

#endif
