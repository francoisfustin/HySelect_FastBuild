//--------------------------------------------------------------------
//
//  File: cal_col.h
//
//  Description: Column formulas and data dependencies.
//

#ifndef CAL_COL_H
#define CAL_COL_H

#ifdef __cplusplus
extern "C" {
#endif

//--------------------------------------------------------------------
//
//  The CALC_COL structure represents formulas and/or data
//  dependencies on individual columns.
//

typedef struct tagCALC_COL
{
  LPCALC_SHEET lpSheet;
  long lCol;                 // column index
  CALC_HANDLE hExpr;         // expression assigned to this column
  #if defined(SS_V70)
  CALC_HANDLE hPreviousAdjust;  // previous column that needs adjusting
  CALC_HANDLE hNextAdjust;      // next column that needs adjusting
  CALC_HANDLE hPreviousDirty;   // previous column that needs recalculating
  CALC_HANDLE hNextDirty;       // next column that needs recalculating
  CALC_HANDLE hColListeners;    // list of dependent columns
  #else
  BITMATRIX AbsPrecedCells;  // cells (absolute location) used by this col
  BITMATRIX RelPrecedCells;  // cells (relative location) used by this col
  BITMATRIX RelDependCells;  // cells (relative location) using this col
  #endif
} CALC_COL, FAR* LPCALC_COL;

//--------------------------------------------------------------------

#define Col_MemAlloc() (CALC_HANDLE)CalcMemAlloc(sizeof(CALC_COL))
#define Col_MemFree(hMem) CalcMemFree(hMem)
#define Col_MemLock(hMem) (LPCALC_COL)CalcMemLock(hMem)
#define Col_MemUnlock(hMem) CalcMemUnlock(hMem)

//--------------------------------------------------------------------
//
//  The following functions handle a table of columns.
//

CALC_HANDLE ColLookup(LPCALC_SHEET lpSheet, long lCol);
CALC_HANDLE ColLookupAlloc(LPCALC_SHEET lpSheet, long lCol);

BOOL ColSetLinksFromExpr(LPMATRIX lpCellTable, LPVECTOR lpColTable, long lCol,
                         BOOL bLink);
CALC_HANDLE ColGetExpr(LPCALC_SHEET lpSheet, long lCol);
CALC_HANDLE ColSetExpr(LPCALC_SHEET lpSheet, long lCol, CALC_HANDLE hExpr);


//--------------------------------------------------------------------
//
//  The following functions handle an individual column.
//

CALC_HANDLE ColCreate(LPCALC_SHEET lpSheet, long lCol);
BOOL ColDestroy(CALC_HANDLE hCol);
BOOL ColSetLinkToCol(CALC_HANDLE hCol, CALC_HANDLE hDepend, long lRow,
                     BOOL bLink);
BOOL ColMarkDepend(CALC_HANDLE hCol, long lRow, LPBITMATRIX lpDepend);

LPCALC_SHEET Col_GetSheet(CALC_HANDLE hThis);
long Col_GetCol(CALC_HANDLE hThis);

#if defined(SS_V70)

BOOL Col_IsEmpty(CALC_HANDLE hThis);

CALC_HANDLE Col_GetPreviousAdjust(CALC_HANDLE hThis);
void Col_SetPreviousAdjust(CALC_HANDLE hThis, CALC_HANDLE hValue);
CALC_HANDLE Col_GetNextAdjust(CALC_HANDLE hThis);
void Col_SetNextAdjust(CALC_HANDLE hThis, CALC_HANDLE hValue);
CALC_HANDLE Col_GetPreviousDirty(CALC_HANDLE hThis);
void Col_SetPreviousDirty(CALC_HANDLE hThis, CALC_HANDLE hValue);
CALC_HANDLE Col_GetNextDirty(CALC_HANDLE hThis);
void Col_SetNextDirty(CALC_HANDLE hThis, CALC_HANDLE hValue);

void Col_StartListening(CALC_HANDLE hThis, BOOL bIncludeNames);
void Col_StopListening(CALC_HANDLE hThis, BOOL bIncludeNames);

void Col_AddColListener(CALC_HANDLE hThis, CALC_HANDLE hCol, long lRowOffset);
void Col_RemoveColListener(CALC_HANDLE hThis, CALC_HANDLE hCol, long lRowOffset);

void Col_AddListenersToAdjust(CALC_HANDLE hThis);
void Col_AddListenersToDirty(CALC_HANDLE hThis, long lRow);
void Col_IncrementDelayOfListeners(CALC_HANDLE hThis, long lRow);
void Col_DecrementDelayOfListeners(CALC_HANDLE hThis, long lRow);

//--------------------------------------------------------------------

typedef struct tagCALC_COL_COLLISTENERLIST
{
  CALC_HANDLE hCol;
  long lRowOffset;
  CALC_HANDLE hNext;
} CALC_COL_COLLISTENERLIST, FAR* LPCALC_COL_COLLISTENERLIST;

//--------------------------------------------------------------------

#define Col_ColListenerList_MemAlloc() (CALC_HANDLE)CalcMemAlloc(sizeof(CALC_COL_COLLISTENERLIST))
#define Col_ColListenerList_MemFree(hMem) CalcMemFree(hMem)
#define Col_ColListenerList_MemLock(hMem) (LPCALC_COL_COLLISTENERLIST)CalcMemLock(hMem)
#define Col_ColListenerList_MemUnlock(hMem) CalcMemUnlock(hMem)

//--------------------------------------------------------------------

CALC_HANDLE Col_ColListenerList_Create(CALC_HANDLE hCol, long lRowOffset, CALC_HANDLE hNext);
void Col_ColListenerList_Destroy(CALC_HANDLE hThis);
CALC_HANDLE Col_ColListenerList_GetCol(CALC_HANDLE hThis);
long Col_ColListenerList_GetRowOffset(CALC_HANDLE hThis);
CALC_HANDLE Col_ColListenerList_GetNext(CALC_HANDLE hThis);
void Col_ColListenerList_SetNext(CALC_HANDLE hThis, CALC_HANDLE hValue);

#endif  // defined(SS_V70)

//--------------------------------------------------------------------

#ifdef __cplusplus
}
#endif

#endif
