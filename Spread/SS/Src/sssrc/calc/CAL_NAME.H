//--------------------------------------------------------------------
//
//  File: cal_name.h
//
//  Description: Routines to handle named expressions
//

#ifndef CAL_NAME_H
#define CAL_NAME_H

#ifdef __cplusplus
extern "C" {
#endif

//--------------------------------------------------------------------
//
//  The following diagram shows an example of a named expression table
//  which contains a name of "alpha" with a value of "1+2".
//
//  +-----------+     +--------+--------+--------+---
//  | hElem     |---->| handle | handle | handle | ...
//  | nElemCnt  |     +--------+--------+--------+---
//  | nAllocCnt |       |
//  | nAllocInc |       |
//  +-----------+       |
//                      |     +-------------+     +---------+
//                      +---->| hText       |---->| "alpha" |
//                         +--| hExpr       |     +---------+
//                         |  | lRefCnt = 1 |
//                         |  +-------------+
//                         |
//                         |     +----------------------------+
//                         +---->| nType = CALC_EXPR_HEADER   |
//                               | u.lRefCnt = 1              |
//                               +----------------------------+
//                               | nType = CALC_EXPR_LONG     |
//                               | u.lLong = 1                |
//                               +----------------------------+
//                               | nType = CALC_EXPR_LONG     |
//                               | u.lLong = 2                |
//                               +----------------------------+
//                               | nType = CALC_EXPR_OPERATOR |
//                               | u.nId = CALC_OPER_ADD      |
//                               +----------------------------+
//                               | nType = CALC_EXPR_END      |
//                               +----------------------------+

//--------------------------------------------------------------------
//
//  The CALC_NAME structure represents a named expression.
//

typedef struct tagCALC_NAME
{
  LPCALC_BOOK lpBook;
  CALC_HANDLE    hText;        // handle to text representation of name
  CALC_HANDLE    hExpr;        // handle to expression
  long           lRefCnt;      // reference count
  long           lTableRefCnt; // reference count due to tables   
  #if defined(SS_V70)
  CALC_HANDLE hCellListeners;  // list of dependent cells
  CALC_HANDLE hRowListeners;   // list of dependent rows
  CALC_HANDLE hColListeners;   // list of dependent columns
  CALC_HANDLE hNameListeners;  // list of dependent names
  #else
  CALC_NAMETABLE DependNames;  // dependent names which use this name
  BITMATRIX      DependCells;  // dependent cells which use this name
  BITVECTOR      DependCols;   // dependent columns which use this name
  BITVECTOR      DependRows;   // dependent rows which use this name
  #endif
} CALC_NAME, FAR* LPCALC_NAME;

//--------------------------------------------------------------------

#define Name_MemAlloc() (CALC_HANDLE)CalcMemAlloc(sizeof(CALC_NAME))
#define Name_MemFree(hMem) CalcMemFree(hMem)
#define Name_MemLock(hMem) (LPCALC_NAME)CalcMemLock(hMem)
#define Name_MemUnlock(hMem) CalcMemUnlock(hMem)

//--------------------------------------------------------------------
//
//  The following functions are used to handle a table of named
//  expressions.
//

BOOL NameInit(LPCALC_NAMETABLE lpTable);
BOOL NameFree(LPCALC_NAMETABLE lpTable);
BOOL NameAdd(LPCALC_NAMETABLE lpTable, CALC_HANDLE hName);
BOOL NameRemove(LPCALC_NAMETABLE lpTable, CALC_HANDLE hName);
CALC_HANDLE NameLookup(LPCALC_NAMETABLE lpTable, LPCTSTR lpszName);
CALC_HANDLE NameFirst(LPCALC_NAMETABLE lpTable);
CALC_HANDLE NameNext(LPCALC_NAMETABLE lpTable, LPCTSTR lpszName);

//--------------------------------------------------------------------
//
//  The following functions are used to handle an individual named
//  expression.
//

CALC_HANDLE NameCreate(LPCALC_BOOK lpBook, LPCTSTR lpszName, CALC_HANDLE hExpr);
CALC_HANDLE NameCreateRef(CALC_HANDLE hExpr);
BOOL NameDestroy(CALC_HANDLE hName);
LPCALC_BOOK Name_GetBook(CALC_HANDLE hThis);
int  NameGetText(CALC_HANDLE hName, LPTSTR lpszText, int nLen);
int  NameGetTextLen(CALC_HANDLE hName);
long NameGetRefCnt(CALC_HANDLE hName);
BOOL NameIncTableRefCnt(CALC_HANDLE hName);
BOOL NameDecTableRefCnt(CALC_HANDLE hName);
long NameGetTableRefCnt(CALC_HANDLE hName);
CALC_HANDLE NameGetExpr(CALC_HANDLE hName);
BOOL NameSetExpr(CALC_HANDLE hName, CALC_HANDLE hExpr);

#if !defined(SS_V70)
BOOL NameSetLinkToCell(CALC_HANDLE hName, long lCol, long lRow, BOOL bLink);
BOOL NameSetLinkToCol(CALC_HANDLE hName, long lCol, BOOL bLink);
BOOL NameSetLinkToRow(CALC_HANDLE hName, long lRow, BOOL bLink);
BOOL NameSetLinkToName(CALC_HANDLE hName, CALC_HANDLE hDepend, BOOL bLink);
BOOL NameMarkDepend(CALC_HANDLE hName, LPBITMATRIX lpDependCells,
                    LPBITVECTOR lpDependCols, LPBITVECTOR lpDependRows);
#endif

//--------------------------------------------------------------------

#if defined(SS_V70)

void Name_StartListeners(CALC_HANDLE hThis);
void Name_StopListeners(CALC_HANDLE hThis);

void Name_AddCellListener(CALC_HANDLE hThis, CALC_HANDLE hCell);
void Name_RemoveCellListener(CALC_HANDLE hThis, CALC_HANDLE hCell);
void Name_AddRowListener(CALC_HANDLE hThis, CALC_HANDLE hRow);
void Name_RemoveRowListener(CALC_HANDLE hThis, CALC_HANDLE hRow);
void Name_AddColListener(CALC_HANDLE hThis, CALC_HANDLE hCol);
void Name_RemoveColListener(CALC_HANDLE hThis, CALC_HANDLE hCol);
void Name_AddNameListener(CALC_HANDLE hThis, CALC_HANDLE hCell);
void Name_RemoveNameListener(CALC_HANDLE hThis, CALC_HANDLE hCell);

void Name_AddListenersToDirty(CALC_HANDLE hThis);

//--------------------------------------------------------------------

typedef struct tagCALC_NAME_CELLLISTENERLIST
{
  CALC_HANDLE hCell;
  CALC_HANDLE hNext;
} CALC_NAME_CELLLISTENERLIST, FAR* LPCALC_NAME_CELLLISTENERLIST;

//--------------------------------------------------------------------

#define Name_CellListenerList_MemAlloc() (CALC_HANDLE)CalcMemAlloc(sizeof(CALC_NAME_CELLLISTENERLIST))
#define Name_CellListenerList_MemFree(hMem) CalcMemFree(hMem)
#define Name_CellListenerList_MemLock(hMem) (LPCALC_NAME_CELLLISTENERLIST)CalcMemLock(hMem)
#define Name_CellListenerList_MemUnlock(hMem) CalcMemUnlock(hMem)

//--------------------------------------------------------------------

CALC_HANDLE Name_CellListenerList_Create(CALC_HANDLE hCell, CALC_HANDLE hNext);
void Name_CellListenerList_Destroy(CALC_HANDLE hThis);
CALC_HANDLE Name_CellListenerList_GetCell(CALC_HANDLE hThis);
CALC_HANDLE Name_CellListenerList_GetNext(CALC_HANDLE hThis);
void Name_CellListenerList_SetNext(CALC_HANDLE hThis, CALC_HANDLE hValue);

//--------------------------------------------------------------------

typedef struct tagCALC_NAME_ROWLISTENERLIST
{
  CALC_HANDLE hRow;
  CALC_HANDLE hNext;
} CALC_NAME_ROWLISTENERLIST, FAR* LPCALC_NAME_ROWLISTENERLIST;

//--------------------------------------------------------------------

#define Name_RowListenerList_MemAlloc() (CALC_HANDLE)CalcMemAlloc(sizeof(CALC_NAME_ROWLISTENERLIST))
#define Name_RowListenerList_MemFree(hMem) CalcMemFree(hMem)
#define Name_RowListenerList_MemLock(hMem) (LPCALC_NAME_ROWLISTENERLIST)CalcMemLock(hMem)
#define Name_RowListenerList_MemUnlock(hMem) CalcMemUnlock(hMem)

//--------------------------------------------------------------------

CALC_HANDLE Name_RowListenerList_Create(CALC_HANDLE hRow, CALC_HANDLE hNext);
void Name_RowListenerList_Destroy(CALC_HANDLE hThis);
CALC_HANDLE Name_RowListenerList_GetRow(CALC_HANDLE hThis);
CALC_HANDLE Name_RowListenerList_GetNext(CALC_HANDLE hThis);
void Name_RowListenerList_SetNext(CALC_HANDLE hThis, CALC_HANDLE hValue);

//--------------------------------------------------------------------

typedef struct tagCALC_NAME_COLLISTENERLIST
{
  CALC_HANDLE hCol;
  CALC_HANDLE hNext;
} CALC_NAME_COLLISTENERLIST, FAR* LPCALC_NAME_COLLISTENERLIST;

//--------------------------------------------------------------------

#define Name_ColListenerList_MemAlloc() (CALC_HANDLE)CalcMemAlloc(sizeof(CALC_NAME_COLLISTENERLIST))
#define Name_ColListenerList_MemFree(hMem) CalcMemFree(hMem)
#define Name_ColListenerList_MemLock(hMem) (LPCALC_NAME_COLLISTENERLIST)CalcMemLock(hMem)
#define Name_ColListenerList_MemUnlock(hMem) CalcMemUnlock(hMem)

//--------------------------------------------------------------------

CALC_HANDLE Name_ColListenerList_Create(CALC_HANDLE hCol, CALC_HANDLE hNext);
void Name_ColListenerList_Destroy(CALC_HANDLE hThis);
CALC_HANDLE Name_ColListenerList_GetCol(CALC_HANDLE hThis);
CALC_HANDLE Name_ColListenerList_GetNext(CALC_HANDLE hThis);
void Name_ColListenerList_SetNext(CALC_HANDLE hThis, CALC_HANDLE hValue);

//--------------------------------------------------------------------

typedef struct tagCALC_NAME_NAMELISTENERLIST
{
  CALC_HANDLE hName;
  CALC_HANDLE hNext;
} CALC_NAME_NAMELISTENERLIST, FAR* LPCALC_NAME_NAMELISTENERLIST;

//--------------------------------------------------------------------

#define Name_NameListenerList_MemAlloc() (CALC_HANDLE)CalcMemAlloc(sizeof(CALC_NAME_NAMELISTENERLIST))
#define Name_NameListenerList_MemFree(hMem) CalcMemFree(hMem)
#define Name_NameListenerList_MemLock(hMem) (LPCALC_NAME_NAMELISTENERLIST)CalcMemLock(hMem)
#define Name_NameListenerList_MemUnlock(hMem) CalcMemUnlock(hMem)

//--------------------------------------------------------------------

CALC_HANDLE Name_NameListenerList_Create(CALC_HANDLE hName, CALC_HANDLE hNext);
void Name_NameListenerList_Destroy(CALC_HANDLE hThis);
CALC_HANDLE Name_NameListenerList_GetName(CALC_HANDLE hThis);
CALC_HANDLE Name_NameListenerList_GetNext(CALC_HANDLE hThis);
void Name_NameListenerList_SetNext(CALC_HANDLE hThis, CALC_HANDLE hValue);

#endif

//--------------------------------------------------------------------

#ifdef __cplusplus
}
#endif

#endif
