//--------------------------------------------------------------------
//
//  File: calc.h
//
//  Description: FarPoint Calc Engine
//

#ifndef CALC_H
#define CALC_H

#include <windows.h>
#include <windowsx.h>
#include <tchar.h>
#include "cal_mem.h"
#include "cal_ss.h"
#include "cal_matr.h"
#include "cal_vect.h"

#ifdef __cplusplus
extern "C" {
#endif

//--------------------------------------------------------------------
//
//  The BITMATRIX structure can only handle non-negative indices.
//  Thus all relative locations are adjusted by REL_OFFSET when
//  placed into or retrieved from a BITMATRIX structure.
//

#define REL_OFFSET 1000000000L

//--------------------------------------------------------------------
//
//  The CALC_MAX_COORD constant determines the maxium coordinate
//  that the calc routines allow.
//

#define CALC_MAX_COORD 2000000000L

//--------------------------------------------------------------------
//
//  The maximum text length for any number, cell coordinate,
//  name (function, custom function, custom named expression), or
//  expression used in the calc engine.  The length includes the
//  terminating null character.
//

#define CALC_MAX_NUM_LEN     32
#define CALC_MAX_COORD_LEN   32
#define CALC_MAX_SHEET_LEN   32
#define CALC_MAX_NAME_LEN   256
#define CALC_MAX_EXPR_LEN  1024

//--------------------------------------------------------------------
//
//  The following constants are used to describe an entire column
//  or row.
//

#define CALC_ALLCOLS -1
#define CALC_ALLROWS -1

//--------------------------------------------------------------------
//
//  The CALCAPI type is used for all Calc Engine API functions.
//

#if defined(CALC_DLL)
  #if defined(WIN32) || defined(_WIN32)
    #define CALCAPI __declspec(dllexport) __stdcall
  #else
    #define CALCAPI __export FAR PASCAL
  #endif
#else
  #if defined(WIN32) || defined(_WIN32)
    #define CALCAPI __stdcall
  #else
    #define CALCAPI FAR PASCAL
  #endif
#endif

//--------------------------------------------------------------------
//
//  Following definitions provide generic character and text types
//  which can map to SBCS, DBCS, or Unicode.
//

#if !defined(WIN32) && !defined(_WIN32)
  typedef TCHAR FAR* LPTSTR;
  typedef const TCHAR FAR* LPCTSTR;
#endif

//--------------------------------------------------------------------
//
//  The CUSTFUNCTABLE structure represents a table of custom
//  functions.
//

typedef struct tagCALC_CUSTFUNCTABLE
{
  CALC_HANDLE hElem;  // handle to an array of functions handles
  int nElemCnt;       // number of functions in array
  int nAllocCnt;      // number array slots which have been allocated
  int nAllocInc;      // number of additional array slots on each ReAlloc
} CALC_CUSTFUNCTABLE, FAR* LPCALC_CUSTFUNCTABLE;

//--------------------------------------------------------------------
//
//  The CALC_NAMETABLE structure represents a table of named
//  expressions.
//

typedef struct tagCALC_NAMETABLE
{
  CALC_HANDLE hElem;  // handle to an array of functions handles
  int nElemCnt;       // number of functions in array
  int nAllocCnt;      // number array slots which have been allocated
  int nAllocInc;      // number of additional array slots on each ReAlloc
} CALC_NAMETABLE, FAR* LPCALC_NAMETABLE;

//--------------------------------------------------------------------
//
//  The CALC_DDETABLE structure represents a table of DDE links.
//

typedef struct tagCALC_DDETABLE
{
  CALC_HANDLE hElem;  // handle to an array of functions handles
  int nElemCnt;       // number of functions in array
  int nAllocCnt;      // number array slots which have been allocated
  int nAllocInc;      // number of additional array slots on each ReAlloc
} CALC_DDETABLE, FAR* LPCALC_DDETABLE;

//--------------------------------------------------------------------
//
//  The following types describe call back functions used to
//  transfer data to and from the spreadsheet.
//

struct tagCALC_BOOK;
struct tagCALC_SHEET;
typedef short (CALLBACK* LPCALC_GETSHEETCNT)(HANDLE);
typedef struct tagCALC_SHEET FAR* (CALLBACK* LPCALC_GETSHEETFROMINDEX)(HANDLE, short);
typedef struct tagCALC_SHEET FAR* (CALLBACK* LPCALC_GETSHEETFROMNAME)(HANDLE, LPTSTR, int);
typedef void (CALLBACK* LPCALC_SENDCIRCULARFORMULAMSG)(HANDLE, short, long, long);
typedef int (CALLBACK* LPCALC_GETINDEXPROC)(HANDLE);
typedef int (CALLBACK* LPCALC_GETNAMEPROC)(HANDLE, LPTSTR lpszText, int nLen);
typedef int (CALLBACK* LPCALC_GETNAMELENPROC)(HANDLE);
typedef BOOL (CALLBACK* LPCALC_GETDATAPROC)(HANDLE, long, long, LPCALC_VALUE);
typedef BOOL (CALLBACK* LPCALC_SETDATAPROC)(HANDLE, long, long, LPCALC_VALUE);
typedef BOOL (CALLBACK* LPCALC_GETDATACNTPROC)(HANDLE, LPLONG, LPLONG);
typedef long (CALLBACK* LPCALC_GETMAXCOLSPROC)(HANDLE);
typedef long (CALLBACK* LPCALC_GETMAXROWSPROC)(HANDLE);
#if _WIN64
typedef LONG_PTR (CALLBACK* LPCALC_GETCUSTREFPROC)(HANDLE);
#else
typedef long (CALLBACK* LPCALC_GETCUSTREFPROC)(HANDLE);
#endif
typedef void (CALLBACK* LPCALC_SETSENDINGPROC)(HANDLE, long, long);
typedef void (CALLBACK* LPCALC_DDEUPDATEPROC)(HANDLE, CALC_HANDLE);

//--------------------------------------------------------------------
//
//  The CALC_BOOK structure contains workbook information.
//

typedef struct tagCALC_BOOK
{
  HANDLE hBook;                    // workbook id used by the lpfnXxx functions
  LPCALC_GETSHEETCNT lpfnGetSheetCnt;  // function to get sheet count
  LPCALC_GETSHEETFROMINDEX lpfnGetSheetFromIndex; // function to get sheet from index
  LPCALC_GETSHEETFROMNAME lpfnGetSheetFromName;   // function to get sheet from name
  LPCALC_SENDCIRCULARFORMULAMSG lpfnSendCircularFormulaMsg;  // function to send SSM_CIRCULARFORMULA msg
  CALC_CUSTFUNCTABLE CustFuncs;  // user defined custom functions
  CALC_NAMETABLE Names;          // user defined named expressions
  BOOL bFormulaSync;             // determines if formulas adjusted on copy
  short nRefStyle;               // determines if A#, A1, or R1C1 style used
  long lColA1;                   // base column used in A1 style
  long lRowA1;                   // base row used in A1 style
  BOOL bIteration;               // determines if iteration used
  int nMaxIterations;            // maximum iterations of recursive formulas
  double dfMaxChange;            // maximum change between iterations
  CALC_HANDLE hHeadAdjustCells;
  CALC_HANDLE hTailAdjustCells;
  CALC_HANDLE hHeadAdjustRows;
  CALC_HANDLE hTailAdjustRows;
  CALC_HANDLE hHeadAdjustCols;
  CALC_HANDLE hTailAdjustCols;
  CALC_HANDLE hHeadDirtyCells;
  CALC_HANDLE hTailDirtyCells;
  CALC_HANDLE hHeadDirtyRows;
  CALC_HANDLE hTailDirtyRows;
  CALC_HANDLE hHeadDirtyCols;
  CALC_HANDLE hTailDirtyCols;
} CALC_BOOK, FAR* LPCALC_BOOK;

//--------------------------------------------------------------------
//
//  The CALC_SHEET structure contains worksheet information.
//

typedef struct tagCALC_SHEET
{
  LPCALC_BOOK lpBook;            // workbook information
  HANDLE hSS;                    // worksheet id used by lpfnXxx functions
  LPCALC_GETINDEXPROC lpfnGetIndex;     // function to get the sheet index
  LPCALC_GETNAMEPROC lpfnGetName;       // function to get the sheet name
  LPCALC_GETNAMELENPROC lpfnGetNameLen; // function to get the length of the sheet name
  LPCALC_GETDATAPROC lpfnGetData;       // function to get data from spreadsheet
  LPCALC_SETDATAPROC lpfnSetData;       // function to set data into spreadsheet
  LPCALC_GETDATACNTPROC lpfnGetDataCnt; // function to get data count from spreadsheet
  LPCALC_GETMAXCOLSPROC lpfnGetMaxCols;   // function to get max columns from spreadsheet
  LPCALC_GETMAXROWSPROC lpfnGetMaxRows;   // function to get max rows from spreadsheet
  LPCALC_GETCUSTREFPROC lpfnGetCustRef; // function to get custom function handle
  LPCALC_SETSENDINGPROC lpfnSetSending; // function to set cell sending coordinate
  LPCALC_DDEUPDATEPROC lpfnDdeUpdate;   // function to notify spreadsheet of DDE update
  CALC_DDETABLE DdeLinks;        // dynamic data exchange links
  MATRIX Cells;                  // cell formulas and dependencies
  VECTOR Cols;                   // column formulas and dependencies
  VECTOR Rows;                   // row formulas and dependencies
  #if defined(SS_V70)
  CALC_HANDLE hCellCellListeners;
  CALC_HANDLE hCellRowListeners;
  CALC_HANDLE hCellColListeners;
  CALC_HANDLE hRowRowListeners;
  CALC_HANDLE hColColListeners;
  #else
  BITMATRIX CellsToEval;         // cells which need recalculation
  BITVECTOR ColsToEval;          // columns which need recalculation
  BITVECTOR RowsToEval;          // rows which need recalculation
  #endif
} CALC_SHEET, FAR* LPCALC_SHEET, CALC_INFO, FAR* LPCALC_INFO;

//--------------------------------------------------------------------
//
//  The following constants describe possible values for the
//  nRefStyle field of the CALC_INFO structure.
//

#define CALC_REFSTYLE_DEFAULT 0
#define CALC_REFSTYLE_A1      1
#define CALC_REFSTYLE_R1C1    2

//--------------------------------------------------------------------
//
//  The following types describe call back functions used in the
//  evualation of operators, built-in functions, and custom functions.
//

typedef BOOL (CALLBACK* CALC_EVALPROC)(LPCALC_VALUE, LPCALC_VALUE, short);
#if defined(SS_OCX) || defined(SS_VB)
typedef BOOL (CALLBACK* CALC_CUSTEVALPROC)(HANDLE, LPCTSTR, LPCALC_VALUE,
                                           LPCALC_VALUE, short);
#else
typedef BOOL (CALLBACK* CALC_CUSTEVALPROC)(LPCALC_VALUE, LPCALC_VALUE, short);
#endif

//--------------------------------------------------------------------
//
//  The following functions are used to initialize and free a
//  CALC_BOOK structure.
//

BOOL CALCAPI CalcInitBook(LPCALC_BOOK lpBook, HANDLE hBook,
                          LPCALC_GETSHEETCNT lpfnGetSheetCnt,
                          LPCALC_GETSHEETFROMINDEX lpfnGetSheetFromIndex,
                          LPCALC_GETSHEETFROMNAME lpfnGetSheetFromName,
                          LPCALC_SENDCIRCULARFORMULAMSG lpfnSendCircularFormulaMsg);
BOOL CALCAPI CalcResetBook(LPCALC_BOOK lpBook);
BOOL CALCAPI CalcFreeBook(LPCALC_BOOK lpBook);

//--------------------------------------------------------------------
//
//  The following functions are used to initialize and free a
//  CALC_SHEET structure.
//

BOOL CALCAPI CalcInitSheet(LPCALC_SHEET lpSheet, LPCALC_BOOK lpBook, HANDLE hSS,
                           LPCALC_GETINDEXPROC lpfnGetIndex,
                           LPCALC_GETNAMEPROC lpfnGetName,
                           LPCALC_GETNAMELENPROC lpfnGetNameLen,
                           LPCALC_GETDATAPROC lpfnGetData,
                           LPCALC_SETDATAPROC lpfnSetData,
                           LPCALC_GETDATACNTPROC lpfnGetDataCnt,
                           LPCALC_GETMAXCOLSPROC lpfnGetMaxCols,
                           LPCALC_GETMAXROWSPROC lpfnGetMaxRows,
                           LPCALC_GETCUSTREFPROC lpfnGetCustRef,
                           LPCALC_SETSENDINGPROC lpfnSetSending,
                           LPCALC_DDEUPDATEPROC lpfnDdeUpdate);
BOOL CALCAPI CalcFreeSheet(LPCALC_SHEET lpSheet);
BOOL CALCAPI CalcResetSheet(LPCALC_SHEET lpSheet);

//--------------------------------------------------------------------
//
//  The following functions are used to handle built-in functions.
//

BOOL CALCAPI CalcFuncLookup(LPCALC_BOOK lpBook, LPCTSTR lpszName,
                            short FAR* lpnMinArgs, short FAR* lpnMaxArgs);
int CALCAPI CalcFuncFirst(LPCALC_BOOK lpBook, LPTSTR lpszName, int nLen);
int CALCAPI CalcFuncNext(LPCALC_BOOK lpBook, LPCTSTR lpszPrevName,
                         LPTSTR lpszName, int nLen);

//--------------------------------------------------------------------
//
//  The following functions are used to handle custom functions.
//

BOOL CALCAPI CalcCustFuncAdd(LPCALC_BOOK lpBook, LPCTSTR lpszName,
                             short nMinArgs, short nMaxArgs,
                             CALC_CUSTEVALPROC lpfnProc, long lFlags);
BOOL CALCAPI CalcCustFuncRemove(LPCALC_BOOK lpBook, LPCTSTR lpszName);
BOOL CALCAPI CalcCustFuncLookup(LPCALC_BOOK lpBook, LPCTSTR lpszName,
                                short FAR* lpnMinArgs, short FAR* lpnMaxArgs,
                                LPLONG lplFlags, LPLONG lplRefCnt);
int CALCAPI CalcCustFuncFirst(LPCALC_BOOK lpBook, LPTSTR lpszName, int nLen);
int CALCAPI CalcCustFuncFirstLen(LPCALC_BOOK lpBook);
int CALCAPI CalcCustFuncNext(LPCALC_BOOK lpBook, LPCTSTR lpszPrevName,
                             LPTSTR lpszName, int nLen);
int CALCAPI CalcCustFuncNextLen(LPCALC_BOOK lpBook, LPCTSTR lpszPrevName);

//--------------------------------------------------------------------
//
//  The following functions are used to handle custom named
//  expressions.
//

BOOL CALCAPI CalcCustNameAdd(LPCALC_BOOK lpBook, LPCTSTR lpszName, LPCTSTR lspzEpxr);
BOOL CALCAPI CalcCustNameRemove(LPCALC_BOOK lpBook, LPCTSTR lpszName);
int CALCAPI CalcCustNameLookup(LPCALC_BOOK lpBook, LPCTSTR lpszName, LPTSTR lpszEpxr, int nLen, LPLONG lplRefCnt);
int CALCAPI CalcCustNameLookupLen(LPCALC_BOOK lpBook, LPCTSTR lpszName);
int CALCAPI CalcCustNameFirst(LPCALC_BOOK lpBook, LPTSTR lpszName, int nLen);
int CALCAPI CalcCustNameFirstLen(LPCALC_BOOK lpBook);
int CALCAPI CalcCustNameNext(LPCALC_BOOK lpBook, LPCTSTR lpszPrevName, LPTSTR lpszName, int nLen);
int CALCAPI CalcCustNameNextLen(LPCALC_BOOK lpBook, LPCTSTR lpszPrevName);

//--------------------------------------------------------------------
//
//  The following functions are used to handle expressions in cells,
//  columns, and rows.
//

BOOL CALCAPI CalcSetExprRange(LPCALC_SHEET lpSheet, long lCol, long lRow,
                              long lCol2, long lRow2, LPCTSTR lpszExpr);
int  CALCAPI CalcGetExpr(LPCALC_SHEET lpSheet, long lCol, long lRow,
#if SS_V80 // 24919 -scl
                         BOOL bExcel,
#endif
                         LPTSTR lpszExpr, int nLen);
int  CALCAPI CalcGetExprLen(LPCALC_SHEET lpSheet, long lCol, long lRow
#if SS_V80 // 24919 -scl
                           , BOOL bExcel
#endif
                           );

//--------------------------------------------------------------------
//
//  The following functions are used to evaluate cells and mark
//  cells for evaluation.
//

BOOL CALCAPI CalcEvalCell(LPCALC_INFO lpCalc, long lCol, long lRow);
BOOL CALCAPI CalcEvalNeededCells(LPCALC_BOOK lpBook);
BOOL CALCAPI CalcEvalAllCells(LPCALC_BOOK lpBook);
BOOL CALCAPI CalcIsCellMarkedForEval(LPCALC_SHEET lpSheet, long lCol, long lRow);
BOOL CALCAPI CalcIsColMarkedForEval(LPCALC_SHEET lpSheet, long lCol);
BOOL CALCAPI CalcIsRowMarkedForEval(LPCALC_SHEET lpSheet, long lRow);
BOOL CALCAPI CalcMarkCellForEval(LPCALC_SHEET lpSheet, long lCol, long lRow);
BOOL CALCAPI CalcMarkColForEval(LPCALC_SHEET lpSheet, long lCol);
BOOL CALCAPI CalcMarkRowForEval(LPCALC_SHEET lpSheet, long lRow);
BOOL CALCAPI CalcMarkDependForEval(LPCALC_SHEET lpSheet, long lCol, long lRow);

//--------------------------------------------------------------------
//
//  The following functions are used to copy, move, swap, clear,
//  insert, or delete a range of cell expressions.
//

BOOL CALCAPI CalcCopyRange(LPCALC_SHEET lpSheet,
                           long lSrcCol, long lSrcRow,
                           long lSrcCol2, long lSrcRow2,
                           long lDestCol, long lDestRow);
BOOL CALCAPI CalcMoveRange(LPCALC_SHEET lpSheet,
                           long lSrcCol, long lSrcRow,
                           long lSrcCol2, long lSrcRow2,
                           long lDestCol, long lDestRow);
BOOL CALCAPI CalcSwapRange(LPCALC_SHEET lpSheet,
                           long lSrcCol, long lSrcRow,
                           long lSrcCol2, long lSrcRow2,
                           long lDestCol, long lDestRow);

BOOL CALCAPI CalcClearRange(LPCALC_SHEET lpSheet,
                            long lCol1, long lRow1,
                            long lCol2, long Row2);

BOOL CALCAPI CalcInsColRange(LPCALC_SHEET lpSheet, long lCol1, long lCol2);
BOOL CALCAPI CalcInsRowRange(LPCALC_SHEET lpSheet, long lRow1, long lRow2);
BOOL CALCAPI CalcDelColRange(LPCALC_SHEET lpSheet, long lCol1, long lCol2);
BOOL CALCAPI CalcDelRowRange(LPCALC_SHEET lpSheet, long lRow1, long lRow2);
BOOL CALCAPI CalcDelSheet(LPCALC_SHEET lpSheet);

//--------------------------------------------------------------------

BOOL ApproxEqual(double x, double y);
double ApproxFloor(double x);
double ApproxCeiling(double x);
double ApproxRound(double x, int digits);
double ApproxRound2(double x, int digits);
double ApproxRoundDown(double x, int digits);
double ApproxRoundUp(double x, int digits);

//--------------------------------------------------------------------

#if defined(SS_V70)

BOOL Book_IsAdjustCell(LPCALC_BOOK lpThis, CALC_HANDLE hCell);
void Book_AddAdjustCell(LPCALC_BOOK lpThis, CALC_HANDLE hCell);
void Book_RemoveAdjustCell(LPCALC_BOOK lpThis, CALC_HANDLE hCell);

BOOL Book_IsAdjustRow(LPCALC_BOOK lpThis, CALC_HANDLE hRow);
void Book_AddAdjustRow(LPCALC_BOOK lpThis, CALC_HANDLE hRow);
void Book_RemoveAdjustRow(LPCALC_BOOK lpThis, CALC_HANDLE hRow);

BOOL Book_IsAdjustCol(LPCALC_BOOK lpThis, CALC_HANDLE hCol);
void Book_AddAdjustCol(LPCALC_BOOK lpThis, CALC_HANDLE hCol);
void Book_RemoveAdjustCol(LPCALC_BOOK lpThis, CALC_HANDLE hCol);

BOOL Book_IsDirtyCell(LPCALC_BOOK lpThis, CALC_HANDLE hCell);
void Book_AddDirtyCell(LPCALC_BOOK lpThis, CALC_HANDLE hCell);
void Book_RemoveDirtyCell(LPCALC_BOOK lpThis, CALC_HANDLE hCell);

BOOL Book_IsDirtyRow(LPCALC_BOOK lpThis, CALC_HANDLE hRow);
void Book_AddDirtyRow(LPCALC_BOOK lpThis, CALC_HANDLE hRow);
void Book_RemoveDirtyRow(LPCALC_BOOK lpThis, CALC_HANDLE hRow);

BOOL Book_IsDirtyCol(LPCALC_BOOK lpThis, CALC_HANDLE hCol);
void Book_AddDirtyCol(LPCALC_BOOK lpThis, CALC_HANDLE hCol);
void Book_RemoveDirtyCol(LPCALC_BOOK lpThis, CALC_HANDLE hCol);

void Sheet_IncrementDelay(LPCALC_SHEET lpThis, long lCol, long lRow);
void Sheet_DecrementDelay(LPCALC_SHEET lpThis, long lCol, long lRow);

void Sheet_CellAddCellListener(LPCALC_SHEET lpThis, long lCol1, long lRow1, long lCol2, long lRow2, CALC_HANDLE hCell);
void Sheet_CellRemoveCellListener(LPCALC_SHEET lpThis, long lCol1, long lRow1, long lCol2, long lRow2, CALC_HANDLE hCell);
void Sheet_CellAddRowListener(LPCALC_SHEET lpThis, long lCol1, long lRow1, long lCol2, long lRow2, CALC_HANDLE hRow);
void Sheet_CellRemoveRowListener(LPCALC_SHEET lpThis, long lCol1, long lRow1, long lCol2, long lRow2, CALC_HANDLE hRow);
void Sheet_CellAddColListener(LPCALC_SHEET lpThis, long lCol1, long lRow1, long lCol2, long lRow2, CALC_HANDLE hCol);
void Sheet_CellRemoveColListener(LPCALC_SHEET lpThis, long lCol1, long lRow1, long lCol2, long lRow2, CALC_HANDLE hCol);
void Sheet_RowAddRowListener(LPCALC_SHEET lpThis, long lRow1, long lRow2, CALC_HANDLE hRow, long lColOffset1, long lColOffset2);
void Sheet_RowRemoveRowListener(LPCALC_SHEET lpThis, long lRow1, long lRow2, CALC_HANDLE hRow, long lColOffset1, long lColOffset2);
void Sheet_ColAddColListener(LPCALC_SHEET lpThis, long lCol1, long lCol2, CALC_HANDLE hCol, long lRowOffset1, long lRowOffset2);
void Sheet_ColRemoveColListener(LPCALC_SHEET lpThis, long lCol1, long lCol2, CALC_HANDLE hCol, long lRowOffset1, long lRowOffset2);

void Sheet_CellAddListenersToAdjust(LPCALC_SHEET lpThis, long lCol, long lRow);
void Sheet_RowAddListenersToAdjust(LPCALC_SHEET lpThis, long lRow);
void Sheet_ColAddListenersToAdjust(LPCALC_SHEET lpThis, long lCol);
void Sheet_AddListenersToDirty(LPCALC_SHEET lpThis, long lCol, long lRow);
void Sheet_IncrementDelayOfListeners(LPCALC_SHEET lpThis, long lCol, long lRow);
void Sheet_DecrementDelayOfListeners(LPCALC_SHEET lpThis, long lCol, long lRow);

#endif  // defined(SS_V70)

#if SS_V80 // 24477 -scl
BOOL CALCAPI CalcIsSubtotal(LPCALC_SHEET lpSheet, long lCol, long lRow); 
#endif
//--------------------------------------------------------------------

#ifdef __cplusplus
}
#endif

#endif
