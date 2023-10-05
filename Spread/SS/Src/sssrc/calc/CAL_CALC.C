 //--------------------------------------------------------------------
//
//  File: cal_calc.c
//
//  Description: External interface functions
//

#include "calc.h"
#include "cal_name.h"
#include "cal_cell.h"
#include "cal_col.h"
#include "cal_row.h"
#include "cal_cust.h"
#include "cal_func.h"
#include "cal_dde.h"
#include "cal_expr.h"
#include "cal_mem.h"

//--------------------------------------------------------------------

static void AdjustDependOnFreeSheet(LPCALC_INFO lpCalc, LPCALC_ADJUST lpAdj);

//--------------------------------------------------------------------
//
//  The InitAdjust() function initializes the LPCALC_ADJUST structure
//  which is used by the auto expression adjustment routines.
//

static void InitAdjust(LPCALC_SHEET lpSheet,
                       long lSrcCol, long lSrcRow,
                       long lSrcCol2, long lSrcRow2,
                       long lDestCol, long lDestRow,
                       LPCALC_ADJUST lpAdj)
{
  lpAdj->lpSrcSheet = lpSheet;
  lpAdj->lSrcCol1 = lSrcCol;
  lpAdj->lSrcRow1 = lSrcRow;
  lpAdj->lSrcCol2 = lSrcCol2;
  lpAdj->lSrcRow2 = lSrcRow2;
  lpAdj->lpDestSheet = lpSheet;
  lpAdj->lDestCol1 = lDestCol;
  lpAdj->lDestRow1 = lDestRow;
  lpAdj->lDestCol2 = lDestCol + lSrcCol2 - lSrcCol;
  lpAdj->lDestRow2 = lDestRow + lSrcRow2 - lSrcRow;
  lpAdj->lOffsetCol = lDestCol - lSrcCol;
  lpAdj->lOffsetRow = lDestRow - lSrcRow;
  BitMatInit(&lpAdj->matCells);
  BitVectInit(&lpAdj->vecCols);
  BitVectInit(&lpAdj->vecRows);
}

//--------------------------------------------------------------------
//
//  The FreeAdjust() function frees the LPCALC_ADJUST structure
//  which is used by the auto expression adjustment routines.
//

static void FreeAdjust(LPCALC_ADJUST lpAdj)
{
  BitMatFree(&lpAdj->matCells);
  BitVectFree(&lpAdj->vecCols);
  BitVectFree(&lpAdj->vecRows);
}

//--------------------------------------------------------------------
//
//  The CalcInitBook function initializes the workbook information.
//

BOOL CALCAPI CalcInitBook(LPCALC_BOOK lpBook, HANDLE hBook,
                          LPCALC_GETSHEETCNT lpfnGetSheetCnt,
                          LPCALC_GETSHEETFROMINDEX lpfnGetSheetFromIndex,
                          LPCALC_GETSHEETFROMNAME lpfnGetSheetFromName,
                          LPCALC_SENDCIRCULARFORMULAMSG lpfnSendCircularFormulaMsg)
{
  lpBook->hBook = hBook;
  lpBook->lpfnGetSheetCnt = lpfnGetSheetCnt;
  lpBook->lpfnGetSheetFromIndex = lpfnGetSheetFromIndex;
  lpBook->lpfnGetSheetFromName = lpfnGetSheetFromName;
  lpBook->lpfnSendCircularFormulaMsg = lpfnSendCircularFormulaMsg;
  CustFuncInit(&lpBook->CustFuncs);
  NameInit(&lpBook->Names);
  lpBook->lColA1 = 1;
  lpBook->lRowA1 = 1;
  lpBook->nRefStyle = CALC_REFSTYLE_DEFAULT;
  lpBook->bFormulaSync = TRUE;
  lpBook->bIteration = FALSE;
  lpBook->nMaxIterations = 1;
  lpBook->dfMaxChange = 0.001;
  lpBook->hHeadAdjustCells = NULL;
  lpBook->hTailAdjustCells = NULL;
  lpBook->hHeadAdjustRows = NULL;
  lpBook->hTailAdjustRows = NULL;
  lpBook->hHeadAdjustCols = NULL;
  lpBook->hTailAdjustCols = NULL;
  lpBook->hHeadDirtyCells = NULL;
  lpBook->hTailDirtyCells = NULL;
  lpBook->hHeadDirtyRows = NULL;
  lpBook->hTailDirtyRows = NULL;
  lpBook->hHeadDirtyCols = NULL;
  lpBook->hTailDirtyCols = NULL;
  return TRUE;
}

//--------------------------------------------------------------------
//
//  The CalcResetBook function resets the workbook information.
//

BOOL CALCAPI CalcResetBook(LPCALC_BOOK lpBook)
{
  HANDLE hBook = lpBook->hBook;
  LPCALC_GETSHEETCNT lpfnGetSheetCnt = lpBook->lpfnGetSheetCnt;
  LPCALC_GETSHEETFROMINDEX lpfnGetSheetFromIndex = lpBook->lpfnGetSheetFromIndex;
  LPCALC_GETSHEETFROMNAME lpfnGetSheetFromName = lpBook->lpfnGetSheetFromName;
  LPCALC_SENDCIRCULARFORMULAMSG lpfnSendCircularFormulaMsg = lpBook->lpfnSendCircularFormulaMsg;
  CalcFreeBook(lpBook);
  CalcInitBook(lpBook, hBook, lpfnGetSheetCnt, lpfnGetSheetFromIndex, lpfnGetSheetFromName, lpfnSendCircularFormulaMsg);
  return TRUE;
}

//--------------------------------------------------------------------
//
//  The CalcFreeBook function frees the workbook information.
//

BOOL CALCAPI CalcFreeBook(LPCALC_BOOK lpBook)
{
  CustFuncFree(&lpBook->CustFuncs);
  NameFree(&lpBook->Names);
  return TRUE;
}

//--------------------------------------------------------------------
//
//  The CalcInitSheet() function initializes the worksheet information.
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
                           LPCALC_DDEUPDATEPROC lpfnDdeUpdate)
{
  lpSheet->lpBook = lpBook,
  lpSheet->hSS = hSS;
  lpSheet->lpfnGetIndex = lpfnGetIndex;
  lpSheet->lpfnGetName = lpfnGetName;
  lpSheet->lpfnGetNameLen = lpfnGetNameLen;
  lpSheet->lpfnGetData = lpfnGetData;
  lpSheet->lpfnSetData = lpfnSetData;
  lpSheet->lpfnGetDataCnt = lpfnGetDataCnt;
  lpSheet->lpfnGetMaxCols = lpfnGetMaxCols;
  lpSheet->lpfnGetMaxRows = lpfnGetMaxRows;
  lpSheet->lpfnGetCustRef = lpfnGetCustRef;
  lpSheet->lpfnSetSending = lpfnSetSending;
  lpSheet->lpfnDdeUpdate = lpfnDdeUpdate;
  #if defined(SS_DDE)
  DdeInit(&lpSheet->DdeLinks);
  #endif
  MatInit(&lpSheet->Cells);
  VectInit(&lpSheet->Cols);
  VectInit(&lpSheet->Rows);
  #if defined(SS_V70)
  lpSheet->hCellCellListeners = NULL;
  lpSheet->hCellRowListeners = NULL;
  lpSheet->hCellColListeners = NULL;
  lpSheet->hRowRowListeners = NULL;
  lpSheet->hColColListeners = NULL;
  #else
  BitMatInit(&lpSheet->CellsToEval);
  BitVectInit(&lpSheet->ColsToEval);
  BitVectInit(&lpSheet->RowsToEval);
  #endif
  return TRUE;
}

//--------------------------------------------------------------------
//
//  The CalcReset() function resets the worksheet information.
//

BOOL CALCAPI CalcResetSheet(LPCALC_SHEET lpSheet)
#if defined(SS_V70)
{
  CALC_HANDLE hCol;
  CALC_HANDLE hRow;
  CALC_HANDLE hCell;
  CALC_HANDLE hExpr;
  long lCol, lRow;

  for (hCol = VectGetFirst(&lpSheet->Cols, &lCol); hCol != NULL; hCol = VectGetNext(&lpSheet->Cols, &lCol))
  {
    hExpr = ColSetExpr(lpSheet, lCol, NULL);
    ExprDestroy(hExpr);
  }
  for (hRow = VectGetFirst(&lpSheet->Rows, &lRow); hRow != NULL; hRow = VectGetNext(&lpSheet->Rows, &lRow))
  {
    hExpr = RowSetExpr(lpSheet, lRow, NULL);
    ExprDestroy(hExpr);
  }
  for (hCell = MatGetFirst(&lpSheet->Cells, &lCol, &lRow); hCell != NULL; hCell = MatGetNext(&lpSheet->Cells, &lCol, &lRow))
  {
    hExpr = CellSetExpr(lpSheet, lCol, lRow, NULL);
    ExprDestroy(hExpr);
  }
  for (hCol = VectGetFirst(&lpSheet->Cols, &lCol); hCol != NULL; hCol = VectGetNext(&lpSheet->Cols, &lCol))
  {
    if (Book_IsAdjustCol(lpSheet->lpBook, hCol))
      Book_RemoveAdjustCol(lpSheet->lpBook, hCol);
    if (Book_IsDirtyCol(lpSheet->lpBook, hCol))
      Book_RemoveDirtyCol(lpSheet->lpBook, hCol);
  }
  for (hRow = VectGetFirst(&lpSheet->Rows, &lRow); hRow != NULL; hRow = VectGetNext(&lpSheet->Rows, &lRow))
  {
    if (Book_IsAdjustRow(lpSheet->lpBook, hRow))
      Book_RemoveAdjustRow(lpSheet->lpBook, hRow);
    if (Book_IsDirtyRow(lpSheet->lpBook, hRow))
      Book_RemoveDirtyRow(lpSheet->lpBook, hRow);
  }
  for (hCell = MatGetFirst(&lpSheet->Cells, &lCol, &lRow); hCell != NULL; hCell = MatGetNext(&lpSheet->Cells, &lCol, &lRow))
  {
    if (Book_IsAdjustCell(lpSheet->lpBook, hCell))
      Book_RemoveAdjustCell(lpSheet->lpBook, hCell);
    if (Book_IsDirtyCell(lpSheet->lpBook, hCell))
      Book_RemoveDirtyCell(lpSheet->lpBook, hCell);
  }
  for (hCol = VectGetFirst(&lpSheet->Cols, &lCol); hCol != NULL; hCol = VectGetNext(&lpSheet->Cols, &lCol))
  {
    if (Col_IsEmpty(hCol))
    {
      VectSet(&lpSheet->Cols, lCol, NULL);
      ColDestroy(hCol);
    }
  }
  for (hRow = VectGetFirst(&lpSheet->Rows, &lRow); hRow != NULL; hRow = VectGetNext(&lpSheet->Rows, &lRow))
  {
    if (Row_IsEmpty(hRow))
    {
      VectSet(&lpSheet->Rows, lRow, NULL);
      RowDestroy(hRow);
    }
  }
  for (hCell = MatGetFirst(&lpSheet->Cells, &lCol, &lRow); hCell != NULL; hCell = MatGetNext(&lpSheet->Cells, &lCol, &lRow))
  {
    if (Cell_IsEmpty(hCell))
    {
      MatSet(&lpSheet->Cells, lCol, lRow, NULL);
      CellDestroy(hCell);
    }
  }
  return TRUE;
}
#else
{
  LPCALC_BOOK lpBook = lpSheet->lpBook;
  HANDLE hSS = lpSheet->hSS;
  LPCALC_GETINDEXPROC lpfnGetIndex = lpSheet->lpfnGetIndex;
  LPCALC_GETNAMEPROC lpfnGetName = lpSheet->lpfnGetName;
  LPCALC_GETNAMELENPROC lpfnGetNameLen = lpSheet->lpfnGetNameLen;
  LPCALC_GETDATAPROC lpfnGetData = lpSheet->lpfnGetData;
  LPCALC_SETDATAPROC lpfnSetData = lpSheet->lpfnSetData;
  LPCALC_GETDATACNTPROC lpfnGetDataCnt = lpSheet->lpfnGetDataCnt;
  LPCALC_GETMAXCOLSPROC lpfnGetMaxCols = lpSheet->lpfnGetMaxCols;
  LPCALC_GETMAXROWSPROC lpfnGetMaxRows = lpSheet->lpfnGetMaxRows;
  LPCALC_GETCUSTREFPROC lpfnGetCustRef = lpSheet->lpfnGetCustRef;
  LPCALC_SETSENDINGPROC lpfnSetSending = lpSheet->lpfnSetSending;
  LPCALC_DDEUPDATEPROC lpfnDdeUpdate = lpSheet->lpfnDdeUpdate;
  CalcFreeSheet(lpSheet);
  CalcInitSheet(lpSheet, lpBook, hSS,
                lpfnGetIndex, lpfnGetName, lpfnGetNameLen,
                lpfnGetData, lpfnSetData, lpfnGetDataCnt,
                lpfnGetMaxCols, lpfnGetMaxRows,
                lpfnGetCustRef, lpfnSetSending, lpfnDdeUpdate);
  return TRUE;
}
#endif

//--------------------------------------------------------------------
//
//  The CalcFree() function frees the worksheet information.
//

BOOL CALCAPI CalcFreeSheet(LPCALC_SHEET lpSheet)
{
  CALC_HANDLE hCell;
  CALC_HANDLE hCol;
  CALC_HANDLE hRow;
  #if defined(SS_V70)
  CALC_HANDLE hExpr;
  CALC_ADJUST adj;
  #endif
  long lCol;
  long lRow;

  #if defined(SS_V70)
  // remove dependency links from other sheets to this sheet
  for (hCell = MatGetFirst(&lpSheet->Cells, &lCol, &lRow); hCell != NULL; hCell = MatGetNext(&lpSheet->Cells, &lCol, &lRow))
  {
    hExpr = CellSetExpr(lpSheet, lCol, lRow, NULL);
    ExprDestroy(hExpr);
  }
  for (hCol = VectGetFirst(&lpSheet->Cols, &lCol); hCol != NULL; hCol = VectGetNext(&lpSheet->Cols, &lCol))
  {
    hExpr = ColSetExpr(lpSheet, lCol, NULL);
    ExprDestroy(hExpr);
  }
  for (hRow = VectGetFirst(&lpSheet->Rows, &lRow); hRow != NULL; hRow = VectGetNext(&lpSheet->Rows, &lRow))
  {
    hExpr = RowSetExpr(lpSheet, lRow, NULL);
    ExprDestroy(hExpr);
  }
  // remove dependency links from this sheet to other sheets
  InitAdjust(lpSheet, CALC_ALLCOLS, CALC_ALLROWS, CALC_ALLCOLS, CALC_ALLROWS, CALC_ALLCOLS, CALC_ALLROWS, &adj);
  for (hCell = MatGetFirst(&lpSheet->Cells, &lCol, &lRow); hCell != NULL; hCell = MatGetNext(&lpSheet->Cells, &lCol, &lRow))
  {
    Cell_AddListenersToAdjust(hCell);
    Sheet_CellAddListenersToAdjust(lpSheet, lCol, lRow);
    if (Book_IsAdjustCell(lpSheet->lpBook, hCell))
      Book_RemoveAdjustCell(lpSheet->lpBook, hCell);
    if (Book_IsDirtyCell(lpSheet->lpBook, hCell))
      Book_RemoveDirtyCell(lpSheet->lpBook, hCell);
  }
  for (hCol = VectGetFirst(&lpSheet->Cols, &lCol); hCol != NULL; hCol = VectGetNext(&lpSheet->Cols, &lCol))
  {
    Col_AddListenersToAdjust(hCol);
    Sheet_ColAddListenersToAdjust(lpSheet, lCol);
    if (Book_IsAdjustCol(lpSheet->lpBook, hCol))
      Book_RemoveAdjustCol(lpSheet->lpBook, hCol);
    if (Book_IsDirtyCol(lpSheet->lpBook, hCol))
      Book_RemoveDirtyCol(lpSheet->lpBook, hCol);
  }
  for (hRow = VectGetFirst(&lpSheet->Rows, &lRow); hRow != NULL; hRow = VectGetNext(&lpSheet->Rows, &lRow))
  {
    Row_AddListenersToAdjust(hRow);
    Sheet_RowAddListenersToAdjust(lpSheet, lRow);
    if (Book_IsAdjustRow(lpSheet->lpBook, hRow))
      Book_RemoveAdjustRow(lpSheet->lpBook, hRow);
    if (Book_IsDirtyRow(lpSheet->lpBook, hRow))
      Book_RemoveDirtyRow(lpSheet->lpBook, hRow);
  }
  AdjustDependOnFreeSheet(lpSheet, &adj);
  FreeAdjust(&adj);
  #endif
  #if defined(SS_DDE)
  DdeFree(&lpSheet->DdeLinks);
  #endif
  while (hCell = MatRemoveFirst(&lpSheet->Cells, &lCol, &lRow))
    CellDestroy(hCell);
  while (hCol  = VectRemoveFirst(&lpSheet->Cols, &lCol))
    ColDestroy(hCol);
  while (hRow  = VectRemoveFirst(&lpSheet->Rows, &lRow))
    RowDestroy(hRow);
  MatFree(&lpSheet->Cells);
  VectFree(&lpSheet->Cols);
  VectFree(&lpSheet->Rows);
  #if !defined(SS_V70)
  BitMatFree(&lpSheet->CellsToEval);
  BitVectFree(&lpSheet->ColsToEval);
  BitVectFree(&lpSheet->RowsToEval);
  #endif
  return TRUE;
}

//--------------------------------------------------------------------
//
//  The CalcFuncLookup() retrieves information about a built-in
//  function.
//

BOOL CALCAPI CalcFuncLookup(LPCALC_BOOK lpBook, LPCTSTR lpszName,
                            short FAR* lpnMinArgs, short FAR* lpnMaxArgs)
{
  BOOL bReturn = FALSE;
  int idFunc;

  if( idFunc = FuncLookup(lpszName) )
  {
    if( lpnMinArgs )
      *lpnMinArgs = FuncGetMinArgs(idFunc);
    if( lpnMaxArgs )
      *lpnMaxArgs = FuncGetMaxArgs(idFunc);
    bReturn = TRUE;
  }
  return bReturn;
}

//--------------------------------------------------------------------
//
//  The CalcFuncFirst() function retrieves the first built-in
//  function.
//

int CALCAPI CalcFuncFirst(LPCALC_BOOK lpBook, LPTSTR lpszName, int nLen)
{
  int idFunc;
  int nRet = 0;

  if( idFunc = FuncFirst() )
    nRet = FuncGetText(idFunc, lpszName, nLen);
  else if( nLen > 0 )
    lpszName[0] = 0;
  return nRet;
}

//--------------------------------------------------------------------
//
//  The CalcFuncNext() function retrieves the next built-in
//  function.
//

int CALCAPI CalcFuncNext(LPCALC_BOOK lpBook, LPCTSTR lpszPrevName,
                         LPTSTR lpszName, int nLen)
{
  int idFunc;
  int nRet = 0;

  if( idFunc = FuncNext(lpszPrevName) )
    nRet = FuncGetText(idFunc, lpszName, nLen);
  else if( nLen > 0 )
    lpszName[0] = 0;
  return nRet;
}

//--------------------------------------------------------------------
//
//  The CalcCustFuncAdd() function adds a custom function to the
//  calc engine.
//
//  Note: Custom functions can not be replaced while in use.
//

BOOL CALCAPI CalcCustFuncAdd(LPCALC_BOOK lpBook, LPCTSTR lpszName,
                             short nMinArgs, short nMaxArgs,
                             CALC_CUSTEVALPROC lpfnProc, long lFlags)
{
  CALC_HANDLE hFunc;
  BOOL bReturn = FALSE;

  if( hFunc = CustFuncLookup(&lpBook->CustFuncs, lpszName) )
  {
    if( CustFuncGetMinArgs(hFunc) >= nMinArgs &&
        CustFuncGetMaxArgs(hFunc) <= nMaxArgs &&
        CustFuncGetProc(hFunc) == NULL &&
        CustFuncGetFlags(hFunc) == lFlags )
    {
      if( CustFuncGetMinArgs(hFunc) > nMinArgs )
        CustFuncSetMinArgs(hFunc, nMinArgs);
      if( CustFuncGetMaxArgs(hFunc) < nMaxArgs )
        CustFuncSetMaxArgs(hFunc, nMaxArgs);
      bReturn = CustFuncSetProc(hFunc, lpfnProc);
    }
  }
  else if( hFunc = CustFuncCreate(lpszName, nMinArgs, nMaxArgs, lpfnProc, lFlags) )
  {
    CustFuncAdd(&lpBook->CustFuncs, hFunc);  // table creates its own reference
    CustFuncDestroy(hFunc);  // destroy the local reference
    bReturn = TRUE;
  }
  return bReturn;
}

//--------------------------------------------------------------------
//
//  The CalcCustFuncRemove() function removes a custom function
//  from the calc engine.
//
//  Note: Custom functions can not be removed while in use.
//

BOOL CALCAPI CalcCustFuncRemove(LPCALC_BOOK lpBook, LPCTSTR lpszName)
{
  CALC_HANDLE hFunc;
  BOOL bReturn = FALSE;

  if( hFunc = CustFuncLookup(&lpBook->CustFuncs, lpszName) )
  {
    if( CustFuncGetRefCnt(hFunc) <= 1 )
    {
      CustFuncRemove(&lpBook->CustFuncs, hFunc);
      bReturn = TRUE;
    }
  }
  return bReturn;
}

//--------------------------------------------------------------------
//
//  The CalcCustFuncLookup() function retrieves information about
//  the custom function.
//

BOOL CALCAPI CalcCustFuncLookup(LPCALC_BOOK lpBook, LPCTSTR lpszName,
                                short FAR* lpnMinArgs, short FAR* lpnMaxArgs,
                                LPLONG lplFlags, LPLONG lplRefCnt)
{
  CALC_HANDLE hFunc;
  BOOL bReturn = FALSE;

  if( hFunc = CustFuncLookup(&lpBook->CustFuncs, lpszName) )
  {
    if( lpnMinArgs )
      *lpnMinArgs = CustFuncGetMinArgs(hFunc);
    if( lpnMaxArgs )
      *lpnMaxArgs = CustFuncGetMaxArgs(hFunc);
    if( lplFlags )
      *lplFlags = CustFuncGetFlags(hFunc);
    if( lplRefCnt )
      *lplRefCnt = CustFuncGetRefCnt(hFunc);
    bReturn = TRUE;
  }
  return bReturn;
  
}

//--------------------------------------------------------------------
//
//  The CalcCustFuncFirst() function retrieves the first custom
//  function.
//

int CALCAPI CalcCustFuncFirst(LPCALC_BOOK lpBook, LPTSTR lpszName, int nLen)
{
  CALC_HANDLE hFunc;
  int nRet = 0;

  if( hFunc = CustFuncFirst(&lpBook->CustFuncs) )
    nRet = CustFuncGetText(hFunc, lpszName, nLen);
  else if( nLen > 0 )
    lpszName[0] = 0;
  return nRet;
}

//--------------------------------------------------------------------
//
//  The CalcCustFuncFirst() function retrieves the length of the
//  first custom function.
//

int CALCAPI CalcCustFuncFirstLen(LPCALC_BOOK lpBook)
{
  CALC_HANDLE hFunc;
  int nRet = 0;

  if( hFunc = CustFuncFirst(&lpBook->CustFuncs) )
    nRet = CustFuncGetTextLen(hFunc);
  return nRet;
}

//--------------------------------------------------------------------
//
//  The CalcCustFuncNext() function retrives the next custom function.
//

int CALCAPI CalcCustFuncNext(LPCALC_BOOK lpBook, LPCTSTR lpszPrevName,
                             LPTSTR lpszName, int nLen)
{
  CALC_HANDLE hFunc;
  int nRet = 0;

  if( hFunc = CustFuncNext(&lpBook->CustFuncs, lpszPrevName) )
    nRet = CustFuncGetText(hFunc, lpszName, nLen);
  else if( nLen > 0 )
    lpszName[0] = 0;
  return nRet;
}

//--------------------------------------------------------------------
//
//  The CalcCustFuncNextLen() function retrives the length of the
//  next custom function.
//

int CALCAPI CalcCustFuncNextLen(LPCALC_BOOK lpBook, LPCTSTR lpszPrevName)
{
  CALC_HANDLE hFunc;
  int nRet = 0;

  if( hFunc = CustFuncNext(&lpBook->CustFuncs, lpszPrevName) )
    nRet = CustFuncGetTextLen(hFunc);
  return nRet;
}

//--------------------------------------------------------------------

static void SetLinksUsingName(LPCALC_INFO lpCalc, CALC_HANDLE hName,
                              BOOL bLink)
#if defined(SS_V70)
{
  if (bLink)
    Name_StartListeners(hName);
  else
    Name_StopListeners(hName);
}
#else
{
  LPCALC_NAME lpName;
  BITMATRIX dependCells;
  BITVECTOR dependCols;
  BITVECTOR dependRows;
  BOOL bContinue;
  long lCol;
  long lRow;

  if( hName && (lpName = (LPCALC_NAME)CalcMemLock(hName)) )
  {
    BitMatInit(&dependCells);
    BitVectInit(&dependCols);
    BitVectInit(&dependRows);
    NameMarkDepend(hName, &dependCells, &dependCols, &dependRows);
    bContinue = BitMatGetFirst(&dependCells, &lCol, &lRow);
    while( bContinue )
    {
      CellSetLinksFromExpr(&lpCalc->Cells, lCol, lRow, bLink);
      bContinue = BitMatGetNext(&dependCells, &lCol, &lRow);
    }
    bContinue = BitVectGetFirst(&dependCols, &lCol);
    while( bContinue )
    {
      ColSetLinksFromExpr(&lpCalc->Cells, &lpCalc->Cols, lCol, bLink);
      bContinue = BitVectGetNext(&dependCols, &lCol);
    }
    bContinue = BitVectGetFirst(&dependRows, &lRow);
    while( bContinue )
    {
      RowSetLinksFromExpr(&lpCalc->Cells, &lpCalc->Rows, lRow, bLink);
      bContinue = BitVectGetNext(&dependRows, &lRow);
    }
    BitMatFree(&dependCells);
    BitVectFree(&dependCols);
    BitVectFree(&dependRows);
    CalcMemUnlock(hName);
  }
}
#endif

//--------------------------------------------------------------------
//
//  The CalcCustNameAdd() function adds a custom named expression to
//  the calc engine.
//

BOOL CALCAPI CalcCustNameAdd(LPCALC_BOOK lpBook, LPCTSTR lpszName, LPCTSTR lpszExpr)
{
  CALC_HANDLE hName;
  CALC_HANDLE hExpr;
  BOOL bReturn = FALSE;

  if( lpszExpr && *lpszExpr && (hExpr = ExprCreate(lpBook, lpBook->lColA1, lpBook->lRowA1, lpszExpr)) )
  {
    if( hName = NameLookup(&lpBook->Names, lpszName) )
    {
      LPCALC_SHEET lpSheet = lpBook->lpfnGetSheetFromIndex(lpBook->hBook, 0);
      SetLinksUsingName(lpSheet, hName, FALSE);
      NameSetExpr(hName, hExpr);
      SetLinksUsingName(lpSheet, hName, TRUE);
      #if defined(SS_V70)
      Name_AddListenersToDirty(hName);
      #else
      NameMarkDepend(hName, &lpSheet->CellsToEval, &lpSheet->ColsToEval, &lpSheet->RowsToEval);
      #endif
      bReturn = TRUE;
    }
    else if( hName = NameCreate(lpBook, lpszName, hExpr) )
    {
      NameAdd(&lpBook->Names, hName);  // table creates its own reference
      NameDestroy(hName);  // destroy the local reference
      bReturn = TRUE;
    }
  }
/*
  else
  {
    if( hName = NameLookup(&lpBook->Names, lpszName) )
    {
      LPCALC_SHEET lpSheet = lpBook->lpfnGetSheetFromIndex(lpBook->hBook, 0);
      SetLinksUsingName(lpSheet, hName, FALSE);
      NameSetExpr(hName, NULL);
      SetLinksUsingName(lpSheet, hName, TRUE);
      #if defined(SS_V70)
      Name_AddListenersToDirty(hName);
      #else
      NameMarkDepend(hName, &lpSheet->CellsToEval, &lpSheet->ColsToEval, &lpSheet->RowsToEval);
      #endif
      bReturn = TRUE;
    }
    else if( hName = NameCreate(lpBook, lpszName, NULL) )
    {
      NameAdd(&lpBook->Names, hName);  // table creates its own reference
      NameDestroy(hName);  // destroy the local reference
      bReturn = TRUE;
    }
  }
*/
  return bReturn;
}

//--------------------------------------------------------------------
//
//  The CalcCustNameRemove() removes a custom named expression from
//  the calc engine.
//

BOOL CALCAPI CalcCustNameRemove(LPCALC_BOOK lpBook, LPCTSTR lpszName)
{
  CALC_HANDLE hName;
  BOOL bReturn = FALSE;

  if( hName = NameLookup(&lpBook->Names, lpszName) )
  {
    if( NameGetRefCnt(hName) <= NameGetTableRefCnt(hName) )
    {
      NameSetExpr(hName, NULL);
      NameRemove(&lpBook->Names, hName);
      bReturn = TRUE;
    }
  }
  return bReturn;
}

//--------------------------------------------------------------------
//
//  The CalcCustNameLookup() retrieves information about a custom
//  named expression.
//

int CALCAPI CalcCustNameLookup(LPCALC_BOOK lpBook, LPCTSTR lpszName, LPTSTR lpszExpr, int nLen, LPLONG lplRefCnt)
{
  CALC_HANDLE hName;
  CALC_HANDLE hExpr;
  int nRet = 0;

  if( hName = NameLookup(&lpBook->Names, lpszName) )
  {
    hExpr = NameGetExpr(hName);
    nRet = ExprGetText(hExpr, lpBook->lColA1, lpBook->lRowA1,
                       lpBook->nRefStyle, 
#if SS_V80 // 24919 -scl
                       FALSE,
#endif
                       lpszExpr, nLen);
    if( lplRefCnt )
      *lplRefCnt = NameGetRefCnt(hName);
  }
  else if( nLen > 0 )
    lpszExpr[0] = 0;
  return nRet;
}

//--------------------------------------------------------------------
//
//  The CalcCustNameLookupLen() retrieves the length of a custom
//  named expression.
//

int CALCAPI CalcCustNameLookupLen(LPCALC_BOOK lpBook, LPCTSTR lpszName)
{
  CALC_HANDLE hName;
  CALC_HANDLE hExpr;
  int nRet = 0;

  if( hName = NameLookup(&lpBook->Names, lpszName) )
  {
    hExpr = NameGetExpr(hName);
    nRet = ExprGetTextLen(hExpr, lpBook->lColA1, lpBook->lRowA1,
                          lpBook->nRefStyle
#if SS_V80 // 24919 -scl
                          , FALSE
#endif
                          );
  }
  return nRet;
}

//--------------------------------------------------------------------
//
//  The CalcCustNameFirst() function retrieves the first custom named
//  expression.
//

int CALCAPI CalcCustNameFirst(LPCALC_BOOK lpBook, LPTSTR lpszName, int nLen)
{
  CALC_HANDLE hName;
  int nRet = 0;

  if( hName = NameFirst(&lpBook->Names) )
    nRet = NameGetText(hName, lpszName, nLen);
  else if( nLen > 0 )
    lpszName[0] = 0;
  return nRet;
}

//--------------------------------------------------------------------
//
//  The CalcCustNameFirstLen() function retrieves the length of the
//  first custom named expression.
//

int CALCAPI CalcCustNameFirstLen(LPCALC_BOOK lpBook)
{
  CALC_HANDLE hName;
  int nRet = 0;

  if( hName = NameFirst(&lpBook->Names) )
    nRet = NameGetTextLen(hName);
  return nRet;
}

//--------------------------------------------------------------------
//
//  The CalcCustNameNext() function retrieves the next custom named
//  expression.
//

int CALCAPI CalcCustNameNext(LPCALC_BOOK lpBook, LPCTSTR lpszPrevName, LPTSTR lpszName, int nLen)
{
  CALC_HANDLE hName;
  int nRet = 0;

  if( hName = NameNext(&lpBook->Names, lpszPrevName) )
    nRet = NameGetText(hName, lpszName, nLen);
  else
    lpszName[0] = 0;
  return nRet;
}

//--------------------------------------------------------------------
//
//  The CalcCustNameNextLen() function retrieves the length of the
//  next custom named expression.
//

int CALCAPI CalcCustNameNextLen(LPCALC_BOOK lpBook, LPCTSTR lpszPrevName)
{
  CALC_HANDLE hName;
  int nRet = 0;

  if( hName = NameNext(&lpBook->Names, lpszPrevName) )
    nRet = NameGetTextLen(hName);
  return nRet;
}

//--------------------------------------------------------------------
//
//  The CalcSetExpr() function assigns an expression to a cell,
//  column, or row.  If lpszExpr is NULL or has zero length then
//  the current cell, column, or row expression is cleared.
//  Expressions attached to a column (i.e. row = -1) do not
//  affect column headers (i.e. row = 0).  Likewise, expressions
//  attached to a row (i.e. col = -1) do not affect row headers
//  (i.e. col = 0).
//

BOOL CALCAPI CalcSetExprRange(LPCALC_SHEET lpSheet, long lCol, long lRow,
                              long lCol2, long lRow2, LPCTSTR lpszExpr)
{
  CALC_HANDLE hExpr = NULL;
  CALC_HANDLE hExprPrev = NULL;
  BOOL bClear = FALSE;
  BOOL bRet = FALSE;
  long i;
  long j;

  if( CALC_ALLCOLS == lCol || CALC_ALLCOLS == lCol2 )
    lCol = lCol2 = CALC_ALLCOLS;
  if( CALC_ALLROWS == lRow || CALC_ALLROWS == lRow2 )
    lRow = lRow2 = CALC_ALLROWS;
  if( lpszExpr && lpszExpr[0] )
    hExpr = ExprCreate(lpSheet->lpBook, lCol, lRow, lpszExpr);  
  else
    bClear = TRUE;
  if( hExpr || bClear )
  {
    if( lCol != CALC_ALLCOLS && lRow != CALC_ALLROWS )
    {
      for( i = lCol; i <= lCol2; i++ )
      {
        for( j = lRow; j <= lRow2; j++ )
        {
          hExpr = ExprCreateRef(hExpr);
          hExprPrev = CellSetExpr(lpSheet, i, j, hExpr);
          if (hExpr)
            CalcMarkCellForEval(lpSheet, i, j);
          ExprDestroy(hExprPrev);
        }
      }
      bRet = TRUE;
    }
    else if( lRow != CALC_ALLROWS && !ExprContainsMixedColRange(hExpr) )
    {
      long lMinCol = MatMinCol(&lpSheet->Cells);
      long lMaxCol = MatMaxCol(&lpSheet->Cells);
      lMinCol = max(1, lMinCol);
      for( j = lRow; j <= lRow2; j++ )
      {
        for( i = lMinCol; i<= lMaxCol; i++ )
        {
          hExprPrev = CellSetExpr(lpSheet, i, j, NULL);
          ExprDestroy(hExprPrev);
        }
        hExpr = ExprCreateRef(hExpr);
        hExprPrev = RowSetExpr(lpSheet, j, hExpr);
        if (hExpr)
          CalcMarkRowForEval(lpSheet, j);
        ExprDestroy(hExprPrev);
      }
      bRet = TRUE;
    }
    else if( lCol != CALC_ALLCOLS && !ExprContainsMixedRowRange(hExpr) )
    {
      long lMinRow = MatMinRow(&lpSheet->Cells);
      long lMaxRow = MatMaxRow(&lpSheet->Cells);
      lMinRow = max(1, lMinRow);
      for( i = lCol; i <= lCol2; i++ )
      {
        for( j = lMinRow; j <= lMaxRow; j++ )
        {
          hExprPrev = CellSetExpr(lpSheet, i, j, NULL);
          ExprDestroy(hExprPrev);
        }
        hExpr = ExprCreateRef(hExpr);
        hExprPrev = ColSetExpr(lpSheet, i, hExpr);
        if (hExpr)
          CalcMarkColForEval(lpSheet, i);
        ExprDestroy(hExprPrev);
      }
      bRet = TRUE;
    }
  }
  ExprDestroy(hExpr);
  return bRet;
}

//--------------------------------------------------------------------
//
//  The CalcGetExpr() function retrieves the text representation
//  of a cell, column, or row expression.
//

int CALCAPI CalcGetExpr(LPCALC_SHEET lpSheet, long lCol, long lRow,
#if SS_V80 // 24919 -scl
                        BOOL bExcel,
#endif
                        LPTSTR lpszExpr, int nLen)
{
  CALC_HANDLE hExpr = 0;
  CALC_HANDLE hCell;
  CALC_HANDLE hCol;
  CALC_HANDLE hRow;
  LPCALC_CELL lpCell;
  LPCALC_COL lpCol;
  LPCALC_ROW lpRow;
  int nRet = 0;

  if( lCol != CALC_ALLCOLS && lRow != CALC_ALLROWS )
  {
    hCell = MatGet(&lpSheet->Cells, lCol, lRow);
    if( hCell && (lpCell = (LPCALC_CELL)CalcMemLock(hCell)) )
    {
      hExpr = lpCell->hExpr;
      CalcMemUnlock(hCell);
    }
  }
  if( !hExpr && lCol != 0 && lRow != CALC_ALLROWS )
  {
    hRow = VectGet(&lpSheet->Rows, lRow);
    if( hRow && (lpRow = (LPCALC_ROW)CalcMemLock(hRow)) )
    {
      hExpr = lpRow->hExpr;
      CalcMemUnlock(hRow);
    }
  }
  if( !hExpr && lCol != CALC_ALLCOLS && lRow != 0 )
  {
    hCol = VectGet(&lpSheet->Cols, lCol);
    if( hCol && (lpCol = (LPCALC_COL)CalcMemLock(hCol)) )
    {
      hExpr = lpCol->hExpr;
      CalcMemUnlock(hCol);
    }
  }
  nRet = ExprGetText(hExpr, lCol, lRow, lpSheet->lpBook->nRefStyle,
#if SS_V80 // 24919 -scl
                     bExcel,
#endif
                     lpszExpr, nLen);
  return nRet;
}

//--------------------------------------------------------------------
//
//  The CalcGetExprLen() function retrieves the length of the text
//  representation of a cell, column, or row expression.
//

int CALCAPI CalcGetExprLen(LPCALC_SHEET lpSheet, long lCol, long lRow
#if SS_V80 // 24919 -scl
                          , BOOL bExcel
#endif
                          )
{
  CALC_HANDLE hExpr = 0;
  CALC_HANDLE hCell;
  CALC_HANDLE hCol;
  CALC_HANDLE hRow;
  LPCALC_CELL lpCell;
  LPCALC_COL lpCol;
  LPCALC_ROW lpRow;
  int nRet = 0;

  if( lCol != CALC_ALLCOLS && lRow != CALC_ALLROWS )
  {
    hCell = MatGet(&lpSheet->Cells, lCol, lRow);
    if( hCell && (lpCell = (LPCALC_CELL)CalcMemLock(hCell)) )
    {
      hExpr = lpCell->hExpr;
      CalcMemUnlock(hCell);
    }
  }
  if( !hExpr && lCol != 0 && lRow != CALC_ALLROWS )
  {
    hRow = VectGet(&lpSheet->Rows, lRow);
    if( hRow && (lpRow = (LPCALC_ROW)CalcMemLock(hRow)) )
    {
      hExpr = lpRow->hExpr;
      CalcMemUnlock(hRow);
    }
  }
  if( !hExpr && lCol != CALC_ALLCOLS && lRow != 0 )
  {
    hCol = VectGet(&lpSheet->Cols, lCol);
    if( hCol && (lpCol = (LPCALC_COL)CalcMemLock(hCol)) )
    {
      hExpr = lpCol->hExpr;
      CalcMemUnlock(hCol);
    }
  }
  return ExprGetTextLen(hExpr, lCol, lRow, lpSheet->lpBook->nRefStyle
#if SS_V80 // 24919 -scl
                        , bExcel
#endif
     );
}

//--------------------------------------------------------------------
//
//  The InitLoop() function initializes the limits and directions
//  needed in the col/row loops in the copy/move/swap routines.
//

static void InitLoop(long lSrcCol, long lSrcRow,
                     long lSrcCol2, long lSrcRow2,
                     long lDestCol, long lDestRow,
                     LPLONG lplColStart, LPLONG lplRowStart,
                     LPLONG lplColEnd, LPLONG lplRowEnd,
                     LPLONG lplColInc, LPLONG lplRowInc,
                     LPLONG lplColOffset, LPLONG lplRowOffset)
{
  *lplColOffset = lDestCol - lSrcCol;
  *lplRowOffset = lDestRow - lSrcRow;
  if( *lplColOffset > 0 )
  {
    *lplColStart = lSrcCol2;
    *lplColEnd = lSrcCol - 1;
    *lplColInc = -1;
  }
  else
  {
    *lplColStart = lSrcCol;
    *lplColEnd = lSrcCol2 + 1;
    *lplColInc = 1;
  }
  if( *lplRowOffset > 0 )
  {
    *lplRowStart = lSrcRow2;
    *lplRowEnd = lSrcRow - 1;
    *lplRowInc = -1;
  }
  else
  {
    *lplRowStart = lSrcRow;
    *lplRowEnd = lSrcRow2 + 1;
    *lplRowInc = 1;
  }
}

//--------------------------------------------------------------------
//
//  The CalcCopyRangeEx() function copies a range of cell, column,
//  and/or row expressions to another location.
//

static BOOL CalcCopyRangeEx(LPCALC_INFO lpCalc,
                            long lSrcCol, long lSrcRow,
                            long lSrcCol2, long lSrcRow2,
                            long lDestCol, long lDestRow)

{
  CALC_HANDLE hExpr;
  long lColStart, lRowStart;
  long lColEnd, lRowEnd;
  long lColInc, lRowInc;
  long lColOffset, lRowOffset;
  long lCol, lRow;
  long lColMin, lRowMin;
  long lColMax, lRowMax;
  
  InitLoop(lSrcCol, lSrcRow, lSrcCol2, lSrcRow2, lDestCol, lDestRow,
           &lColStart, &lRowStart, &lColEnd, &lRowEnd,
           &lColInc, &lRowInc, &lColOffset, &lRowOffset);
  if( CALC_ALLCOLS == lSrcCol )
  {
    lColMin = MatMinCol(&lpCalc->Cells);
    lColMax = MatMaxCol(&lpCalc->Cells);
  }
  if( CALC_ALLROWS == lSrcRow )
  {
    lRowMin = MatMinRow(&lpCalc->Cells);
    lRowMax = MatMaxRow(&lpCalc->Cells);
  }
  for( lRow = lRowStart; lRow != lRowEnd; lRow += lRowInc )
  {
    for( lCol = lColStart; lCol != lColEnd; lCol += lColInc )
    {
      CalcMarkDependForEval(lpCalc, lCol+lColOffset, lRow+lRowOffset);
      if( lRow != CALC_ALLROWS && lCol != CALC_ALLCOLS )
      {
        hExpr = ExprCreateRef(CellGetExpr(lpCalc, lCol, lRow));
        if( hExpr )
          CalcMarkCellForEval(lpCalc, lCol+lColOffset, lRow+lRowOffset);
        hExpr = CellSetExpr(lpCalc,lCol+lColOffset,lRow+lRowOffset,hExpr);
        ExprDestroy(hExpr);
      }
      else if( lRow != CALC_ALLROWS )
      {
        hExpr = ExprCreateRef(RowGetExpr(lpCalc, lRow));
        if( hExpr )
          CalcMarkRowForEval(lpCalc, lRow + lRowOffset);
        hExpr = RowSetExpr(lpCalc, lRow + lRowOffset , hExpr);
        ExprDestroy(hExpr);
        CalcCopyRangeEx(lpCalc,lColMin,lRow,lColMax,lRow,lColMin,lRow+lRowOffset);
      }
      else if( lCol != CALC_ALLCOLS )
      {
        hExpr = ExprCreateRef(ColGetExpr(lpCalc, lCol));
        if( hExpr )
          CalcMarkColForEval(lpCalc, lCol + lColOffset);
        hExpr = ColSetExpr(lpCalc, lCol + lColOffset, hExpr);
        ExprDestroy(hExpr);
        CalcCopyRangeEx(lpCalc,lCol,lRowMin,lCol,lRowMax,lCol+lColOffset,lRowMin);
      }
    }
  }
  return TRUE;
}

//--------------------------------------------------------------------
//
//  The CalcCopyRange() function copies a range of cell, column,
//  and/or row expressions to another location.
//

BOOL CALCAPI CalcCopyRange(LPCALC_SHEET lpSheet,
                           long lSrcCol, long lSrcRow,
                           long lSrcCol2, long lSrcRow2,
                           long lDestCol, long lDestRow)

{
  BOOL bRet;

  if( lSrcCol > lSrcCol2 || lSrcRow > lSrcRow2 )
    return FALSE;
  if( CALC_ALLCOLS==lSrcCol || CALC_ALLCOLS==lSrcCol2 || CALC_ALLCOLS==lDestCol )
    lSrcCol = lSrcCol2 = lDestCol = CALC_ALLCOLS;
  if( CALC_ALLROWS==lSrcRow || CALC_ALLROWS==lSrcRow2 || CALC_ALLROWS==lDestRow )
    lSrcRow = lSrcRow2 = lDestRow = CALC_ALLROWS;
  bRet = CalcCopyRangeEx(lpSheet, lSrcCol, lSrcRow, lSrcCol2, lSrcRow2,
                         lDestCol, lDestRow);
  return bRet;
}

//--------------------------------------------------------------------
//

static BOOL InSrcRange(LPCALC_ADJUST lpAdj, LPCALC_SHEET lpSheet, long lCol, long lRow)
{
  return (lpSheet == lpAdj->lpSrcSheet) &&
         ((CALC_ALLCOLS == lpAdj->lSrcCol1) ||
          (lpAdj->lSrcCol1 <= lCol && lCol <= lpAdj->lSrcCol2)) &&
         ((CALC_ALLROWS == lpAdj->lSrcRow2) ||
          (lpAdj->lSrcRow1 <= lRow && lRow <= lpAdj->lSrcRow2));
}

static BOOL InDestRange(LPCALC_ADJUST lpAdj, LPCALC_SHEET lpSheet, long lCol, long lRow)
{
  return (lpSheet == lpAdj->lpDestSheet) &&
         ((CALC_ALLCOLS == lpAdj->lDestCol1) ||
          (lpAdj->lDestCol1 <= lCol && lCol <= lpAdj->lDestCol2)) &&
         ((CALC_ALLROWS == lpAdj->lDestRow2) ||
          (lpAdj->lDestRow1 <= lRow && lRow <= lpAdj->lDestRow2));
}

static BOOL InSrcOrDestRange(LPCALC_ADJUST lpAdj, LPCALC_SHEET lpSheet, long lCol, long lRow)
{
  return InSrcRange(lpAdj, lpSheet, lCol, lRow) || InDestRange(lpAdj, lpSheet, lCol, lRow);
}

//--------------------------------------------------------------------
//
//  The MarkCellDepend() function marks cells, columns, and rows
//  which are dependent on the given cell.
//

static void MarkCellDepend(LPCALC_SHEET lpSheet, long lCol, long lRow, LPCALC_ADJUST lpAdj)
#if defined(SS_V70)
{
  CALC_HANDLE hCell;

  hCell = CellLookup(lpSheet, lCol, lRow);
  Cell_AddListenersToAdjust(hCell);
  Sheet_CellAddListenersToAdjust(lpSheet, lCol, lRow);
}
#else
{
  CALC_HANDLE hCell;
  LPCALC_CELL lpCell;
  BOOL bFound;
  long x, y;

  hCell = CellLookup(lpSheet, lCol, lRow);
  if( hCell && (lpCell = (LPCALC_CELL)CalcMemLock(hCell)) )
  {
    bFound = BitMatGetFirst(&lpCell->DependCells, &x, &y);
    while( bFound )
    {
      if( !InSrcOrDestRange(lpAdj, lpSheet, x, y) )
        BitMatSet(&lpAdj->matCells, x, y, TRUE);
      bFound = BitMatGetNext(&lpCell->DependCells, &x, &y);
    }
    bFound = BitVectGetFirst(&lpCell->DependCols, &x);
    while( bFound )
    {
      if( !InSrcOrDestRange(lpAdj, lpSheet, x, CALC_ALLROWS) )
        BitVectSet(&lpAdj->vecCols, x, TRUE);
      bFound = BitVectGetNext(&lpCell->DependCols, &x);
    }
    bFound = BitVectGetFirst(&lpCell->DependRows, &y);
    while( bFound )
    {
      if( !InSrcOrDestRange(lpAdj, lpSheet, CALC_ALLCOLS, y) )
        BitVectSet(&lpAdj->vecRows, y, TRUE);
      bFound = BitVectGetNext(&lpCell->DependRows, &y);
    }
    CalcMemUnlock(hCell);
  }
}
#endif

//--------------------------------------------------------------------
//
//  The MarkColDepend() function marks columns which are dependent
//  on the given column.

static void MarkColDepend(LPCALC_SHEET lpSheet, long lCol, LPCALC_ADJUST lpAdj)
#if defined(SS_V70)
{
  CALC_HANDLE hCol;

  hCol = ColLookup(lpSheet, lCol);
  Col_AddListenersToAdjust(hCol);
  Sheet_ColAddListenersToAdjust(lpSheet, lCol);
}
#else
{
  CALC_HANDLE hCol;
  LPCALC_COL lpCol;
  BOOL bFound;
  long x, y;

  hCol = ColLookup(lpSheet, lCol);
  if( hCol && (lpCol = (LPCALC_COL)CalcMemLock(hCol)) )
  {
    bFound = BitMatGetFirst(&lpCol->RelDependCells, &x, &y);
    while( bFound )
    {
      if( !InSrcOrDestRange(lpAdj, lpSheet, x, CALC_ALLROWS) )
        BitVectSet(&lpAdj->vecCols, x, TRUE);
      bFound = BitMatGetNext(&lpCol->RelDependCells, &x, &y);
    }
    CalcMemUnlock(hCol);
  }
}
#endif

//--------------------------------------------------------------------
//
//  The MarkRowDepend() function marks rows which are dependent
//  on the given row.
//

static void MarkRowDepend(LPCALC_SHEET lpSheet, long lRow, LPCALC_ADJUST lpAdj)
#if defined(SS_V70)
{
  CALC_HANDLE hRow;

  hRow = RowLookup(lpSheet, lRow);
  Row_AddListenersToAdjust(hRow);
  Sheet_RowAddListenersToAdjust(lpSheet, lRow);
}
#else
{
  CALC_HANDLE hRow;
  LPCALC_ROW lpRow;
  BOOL bFound;
  long x, y;

  hRow = RowLookup(lpSheet, lRow);
  if( hRow && (lpRow = (LPCALC_ROW)CalcMemLock(hRow)) )
  {
    bFound = BitMatGetFirst(&lpRow->RelDependCells, &x, &y);
    while( bFound )
    {
      if( !InSrcOrDestRange(lpAdj, lpSheet, CALC_ALLCOLS, y) )
        BitVectSet(&lpAdj->vecRows, y, TRUE);
      bFound = BitMatGetNext(&lpRow->RelDependCells, &x, &y);
    }
    CalcMemUnlock(hRow);
  }
}
#endif

//--------------------------------------------------------------------
//
//  The AdjustDependOnMove() function modifies all expressions
//  outside of the block being moved which reference cells in
//  inside of the block being moved.
//

static void AdjustDependOnMove(LPCALC_INFO lpCalc, LPCALC_ADJUST lpAdj, CALC_HANDLE lpfnExprExtModify(CALC_HANDLE, LPCALC_ADJUST))
#if defined(SS_V70)
{
  LPCALC_BOOK lpBook = lpCalc->lpBook;
  CALC_HANDLE hExpr;
  CALC_HANDLE hName;
  LPCALC_HANDLE lpElem;
  LPCALC_SHEET lpSheet;
  long lCol, lRow;
  int i;

  // Modify cell expressions which depend on source block
  while (lpBook->hHeadAdjustCells != NULL)
  {
    lpSheet = Cell_GetSheet(lpBook->hHeadAdjustCells);
    lCol = Cell_GetCol(lpBook->hHeadAdjustCells);
    lRow = Cell_GetRow(lpBook->hHeadAdjustCells);
    if (!InSrcOrDestRange(lpAdj, lpSheet, lCol, lRow))
    {
      lpAdj->lpCurSheet = lpSheet;
      lpAdj->lCurCol = lCol;
      lpAdj->lCurRow = lRow;
      hExpr = CellSetExpr(lpSheet, lCol, lRow, NULL);
      hExpr = lpfnExprExtModify(hExpr, lpAdj);
      hExpr = CellSetExpr(lpSheet, lCol, lRow, hExpr);
      Book_AddDirtyCell(lpBook, lpBook->hHeadAdjustCells);
    }
    Book_RemoveAdjustCell(lpBook, lpBook->hHeadAdjustCells);
  }
  // Modify column expressions which depend on source block
  while (lpBook->hHeadAdjustCols != NULL)
  {
    lpSheet = Col_GetSheet(lpBook->hHeadAdjustCols);
    lCol = Col_GetCol(lpBook->hHeadAdjustCols);
    if (!InSrcOrDestRange(lpAdj, lpSheet, lCol, CALC_ALLROWS))
    {
      lpAdj->lpCurSheet = lpSheet;
      lpAdj->lCurCol = lCol;
      lpAdj->lCurRow = CALC_ALLROWS;
      hExpr = ColSetExpr(lpSheet, lCol, NULL);
      hExpr = lpfnExprExtModify(hExpr, lpAdj);
      hExpr = ColSetExpr(lpSheet, lCol, hExpr);
      Book_AddDirtyCol(lpBook, lpBook->hHeadAdjustCols);
    }
    Book_RemoveAdjustCol(lpBook, lpBook->hHeadAdjustCols);
  }
  // Modify row expressions which depend on source block
  while (lpBook->hHeadAdjustRows != NULL)
  {
    lpSheet = Row_GetSheet(lpBook->hHeadAdjustRows);
    lRow = Row_GetRow(lpBook->hHeadAdjustRows);
    if (!InSrcOrDestRange(lpAdj, lpSheet, CALC_ALLCOLS, lRow))
    {
      lpAdj->lpCurSheet = lpSheet;
      lpAdj->lCurCol = CALC_ALLCOLS;
      lpAdj->lCurRow = lRow;
      hExpr = RowSetExpr(lpSheet, lRow, NULL);
      hExpr = lpfnExprExtModify(hExpr, lpAdj);
      hExpr = RowSetExpr(lpSheet, lRow, hExpr);
      Book_AddDirtyRow(lpBook, lpBook->hHeadAdjustRows);
    }
    Book_RemoveAdjustRow(lpBook, lpBook->hHeadAdjustRows);
  }
  // Modify custom named expressions which depend on source block
  if( lpCalc->lpBook->Names.hElem &&
      (lpElem = (LPCALC_HANDLE)CalcMemLock(lpCalc->lpBook->Names.hElem)) )
  {
    lpAdj->lpCurSheet = NULL;
    lpAdj->lCurCol = CALC_ALLCOLS;
    lpAdj->lCurRow = CALC_ALLROWS;
    for( i = 0; i < lpCalc->lpBook->Names.nElemCnt; i++ )
    {
      hName = lpElem[i];
      hExpr = NameGetExpr(hName);
      SetLinksUsingName(lpCalc, hName, FALSE);
      hExpr = lpfnExprExtModify(ExprCreateRef(hExpr), lpAdj);
      NameSetExpr(hName, hExpr);
      SetLinksUsingName(lpCalc, hName, TRUE);
      #if defined(SS_V70)
      Name_AddListenersToDirty(hName);
      #else
      NameMarkDepend(hName, &lpCalc->CellsToEval, &lpCalc->ColsToEval, &lpCalc->RowsToEval);
      #endif
    }
    CalcMemUnlock(lpCalc->lpBook->Names.hElem);
  }
}
#else
{
  CALC_HANDLE hExpr;
  CALC_HANDLE hName;
  LPCALC_HANDLE lpElem;
  long lCol, lRow;
  int i;

  // Modify cell expressions which depend on source block
  while( BitMatRemoveFirst(&lpAdj->matCells, &lCol, &lRow) )
  {
    BitMatSet(&lpCalc->CellsToEval, lCol, lRow, TRUE);
    lpAdj->lpCurSheet = lpCalc;
    lpAdj->lCurCol = lCol;
    lpAdj->lCurRow = lRow;
    hExpr = CellSetExpr(lpCalc, lCol, lRow, NULL);
    hExpr = lpfnExprExtModify(hExpr, lpAdj);
    hExpr = CellSetExpr(lpCalc, lCol, lRow, hExpr);
  }
  // Modify column expressions which depend on source block
  while( BitVectRemoveFirst(&lpAdj->vecCols, &lCol) )
  {
    BitVectSet(&lpCalc->ColsToEval, lCol, TRUE);
    lpAdj->lpCurSheet = lpCalc;
    lpAdj->lCurCol = lCol;
    lpAdj->lCurRow = CALC_ALLROWS;
    hExpr = ColSetExpr(lpCalc, lCol, NULL);
    hExpr = lpfnExprExtModify(hExpr, lpAdj);
    hExpr = ColSetExpr(lpCalc, lCol, hExpr);
  }
  // Modify row expressions which depend on source block
  while( BitVectRemoveFirst(&lpAdj->vecRows, &lRow) )
  {
    BitVectSet(&lpCalc->RowsToEval, lRow, TRUE);
    lpAdj->lpCurSheet = lpCalc;
    lpAdj->lCurCol = CALC_ALLCOLS;
    lpAdj->lCurRow = lRow;
    hExpr = RowSetExpr(lpCalc, lRow, NULL);
    hExpr = lpfnExprExtModify(hExpr, lpAdj);
    hExpr = RowSetExpr(lpCalc, lRow, hExpr);
  }
  // Modify custom named expressions which depend on source block
  if( lpCalc->lpBook->Names.hElem &&
      (lpElem = (LPCALC_HANDLE)CalcMemLock(lpCalc->lpBook->Names.hElem)) )
  {
    lpAdj->lpCurSheet = lpCalc;
    lpAdj->lCurCol = CALC_ALLCOLS;
    lpAdj->lCurRow = CALC_ALLROWS;
    for( i = 0; i < lpCalc->lpBook->Names.nElemCnt; i++ )
    {
      hName = lpElem[i];
      hExpr = NameGetExpr(hName);
      SetLinksUsingName(lpCalc, hName, FALSE);
      hExpr = lpfnExprExtModify(ExprCreateRef(hExpr), lpAdj);
      NameSetExpr(hName, hExpr);
      SetLinksUsingName(lpCalc, hName, TRUE);
      NameMarkDepend(hName, &lpCalc->CellsToEval, &lpCalc->ColsToEval,
                     &lpCalc->RowsToEval);
    }
    CalcMemUnlock(lpCalc->lpBook->Names.hElem);
  }
}
#endif

//--------------------------------------------------------------------
//
//  The AdjustDependOnSwap() function modifies all expressions
//  outside of the blocks being swapped which reference cells in
//  inside of the blocks being swapped.
//

static void AdjustDependOnSwap(LPCALC_INFO lpCalc, LPCALC_ADJUST lpAdj)
#if defined(SS_V70)
{
  LPCALC_BOOK lpBook = lpCalc->lpBook;
  CALC_HANDLE hExpr;
  CALC_HANDLE hName;
  LPCALC_HANDLE lpElem;
  LPCALC_SHEET lpSheet;
  long lCol, lRow;
  int i;

  // Modify cell expressions which depend on source block
  while (lpBook->hHeadAdjustCells != NULL)
  {
    lpSheet = Cell_GetSheet(lpBook->hHeadAdjustCells);
    lCol = Cell_GetCol(lpBook->hHeadAdjustCells);
    lRow = Cell_GetRow(lpBook->hHeadAdjustCells);
    if (!InSrcOrDestRange(lpAdj, lpSheet, lCol, lRow))
    {
      lpAdj->lpCurSheet = lpSheet;
      lpAdj->lCurCol = lCol;
      lpAdj->lCurRow = lRow;
      hExpr = CellSetExpr(lpSheet, lCol, lRow, NULL);
      hExpr = ExprExtModifyOnSwap(hExpr, lpAdj);
      hExpr = CellSetExpr(lpSheet, lCol, lRow, hExpr);
      Book_AddDirtyCell(lpBook, lpBook->hHeadAdjustCells);
    }
    Book_RemoveAdjustCell(lpBook, lpBook->hHeadAdjustCells);
  }
  // Modify column expressions which depend on source block
  while (lpBook->hHeadAdjustCols != NULL)
  {
    lpSheet = Col_GetSheet(lpBook->hHeadAdjustCols);
    lCol = Col_GetCol(lpBook->hHeadAdjustCols);
    if (!InSrcOrDestRange(lpAdj, lpSheet, lCol, CALC_ALLROWS))
    {
      lpAdj->lpCurSheet = lpSheet;
      lpAdj->lCurCol = lCol;
      lpAdj->lCurRow = CALC_ALLROWS;
      hExpr = ColSetExpr(lpSheet, lCol, NULL);
      hExpr = ExprExtModifyOnSwap(hExpr, lpAdj);
      hExpr = ColSetExpr(lpSheet, lCol, hExpr);
      Book_AddDirtyCol(lpBook, lpBook->hHeadAdjustCols);
    }
    Book_RemoveAdjustCol(lpBook, lpBook->hHeadAdjustCols);
  }
  // Modify row expressions which depend on source block
  while (lpBook->hHeadAdjustRows != NULL)
  {
    lpSheet = Row_GetSheet(lpBook->hHeadAdjustRows);
    lRow = Row_GetRow(lpBook->hHeadAdjustRows);
    if (!InSrcOrDestRange(lpAdj, lpSheet, CALC_ALLCOLS, lRow))
    {
      lpAdj->lpCurSheet = lpSheet;
      lpAdj->lCurCol = CALC_ALLCOLS;
      lpAdj->lCurRow = lRow;
      hExpr = RowSetExpr(lpSheet, lRow, NULL);
      hExpr = ExprExtModifyOnSwap(hExpr, lpAdj);
      hExpr = RowSetExpr(lpSheet, lRow, hExpr);
      Book_AddDirtyRow(lpBook, lpBook->hHeadAdjustRows);
    }
    Book_RemoveAdjustRow(lpBook, lpBook->hHeadAdjustRows);
  }
  // Modify custom named expressions which depend on source block
  if( lpCalc->lpBook->Names.hElem &&
      (lpElem = (LPCALC_HANDLE)CalcMemLock(lpCalc->lpBook->Names.hElem)) )
  {
    lpAdj->lpCurSheet = NULL;
    lpAdj->lCurCol = CALC_ALLCOLS;
    lpAdj->lCurRow = CALC_ALLROWS;
    for( i = 0; i < lpCalc->lpBook->Names.nElemCnt; i++ )
    {
      hName = lpElem[i];
      hExpr = NameGetExpr(hName);
      SetLinksUsingName(lpCalc, hName, FALSE);
      hExpr = ExprExtModifyOnSwap(ExprCreateRef(hExpr), lpAdj);
      NameSetExpr(hName, hExpr);
      SetLinksUsingName(lpCalc, hName, TRUE);
      #if defined(SS_V70)
      Name_AddListenersToDirty(hName);
      #else
      NameMarkDepend(hName, &lpCalc->CellsToEval, &lpCalc->ColsToEval, &lpCalc->RowsToEval);
      #endif
    }
    CalcMemUnlock(lpCalc->lpBook->Names.hElem);
  }
}
#else
{
  CALC_HANDLE hExpr;
  CALC_HANDLE hName;
  LPCALC_HANDLE lpElem;
  long lCol, lRow;
  int i;

  // Modify cell expressions which depend on source block
  while( BitMatRemoveFirst(&lpAdj->matCells, &lCol, &lRow) )
  {
    BitMatSet(&lpCalc->CellsToEval, lCol, lRow, TRUE);
    lpAdj->lpCurSheet = lpCalc;
    lpAdj->lCurCol = lCol;
    lpAdj->lCurRow = lRow;
    hExpr = CellSetExpr(lpCalc, lCol, lRow, NULL);
    hExpr = ExprExtModifyOnSwap(hExpr, lpAdj);
    hExpr = CellSetExpr(lpCalc, lCol, lRow, hExpr);
  }
  // Modify column expressions which depend on source block
  while( BitVectRemoveFirst(&lpAdj->vecCols, &lCol) )
  {
    BitVectSet(&lpCalc->ColsToEval, lCol, TRUE);
    lpAdj->lpCurSheet = lpCalc;
    lpAdj->lCurCol = lCol;
    lpAdj->lCurRow = CALC_ALLROWS;
    hExpr = ColSetExpr(lpCalc, lCol, NULL);
    hExpr = ExprExtModifyOnSwap(hExpr, lpAdj);
    hExpr = ColSetExpr(lpCalc, lCol, hExpr);
  }
  // Modify row expressions which depend on source block
  while( BitVectRemoveFirst(&lpAdj->vecRows, &lRow) )
  {
    BitVectSet(&lpCalc->RowsToEval, lRow, TRUE);
    lpAdj->lpCurSheet = lpCalc;
    lpAdj->lCurCol = CALC_ALLCOLS;
    lpAdj->lCurRow = lRow;
    hExpr = RowSetExpr(lpCalc, lRow, NULL);
    hExpr = ExprExtModifyOnSwap(hExpr, lpAdj);
    hExpr = RowSetExpr(lpCalc, lRow, hExpr);
  }
  // Modify custom named expressions which depend on source block
  if( lpCalc->lpBook->Names.hElem &&
      (lpElem = (LPCALC_HANDLE)CalcMemLock(lpCalc->lpBook->Names.hElem)) )
  {
    lpAdj->lpCurSheet = lpCalc;
    lpAdj->lCurCol = CALC_ALLCOLS;
    lpAdj->lCurRow = CALC_ALLROWS;
    for( i = 0; i < lpCalc->lpBook->Names.nElemCnt; i++ )
    {
      hName = lpElem[i];
      hExpr = NameGetExpr(hName);
      SetLinksUsingName(lpCalc, hName, FALSE);
      hExpr = ExprExtModifyOnSwap(ExprCreateRef(hExpr), lpAdj);
      NameSetExpr(hName, hExpr);
      SetLinksUsingName(lpCalc, hName, TRUE);
      NameMarkDepend(hName, &lpCalc->CellsToEval, &lpCalc->ColsToEval,
                     &lpCalc->RowsToEval);
    }
    CalcMemUnlock(lpCalc->lpBook->Names.hElem);
  }
}
#endif

//--------------------------------------------------------------------
//
//  The CalcMoveRangeEx() function moves a range of cell, column,
//  and/or row expressions to another location.
//

static BOOL CalcMoveRangeEx(LPCALC_INFO lpCalc,
                            long lSrcCol, long lSrcRow,
                            long lSrcCol2, long lSrcRow2,
                            long lDestCol, long lDestRow,
                            LPCALC_ADJUST lpAdj,
                            CALC_HANDLE lpfnExprIntModify(CALC_HANDLE, LPCALC_ADJUST))
{
  CALC_HANDLE hExpr;
  long lColStart, lRowStart;
  long lColEnd, lRowEnd;
  long lColInc, lRowInc;
  long lColOffset, lRowOffset;
  long lCol, lRow;
  long lColMin, lRowMin;
  long lColMax, lRowMax;

  InitLoop(lSrcCol, lSrcRow, lSrcCol2, lSrcRow2, lDestCol, lDestRow,
           &lColStart, &lRowStart, &lColEnd, &lRowEnd,
           &lColInc, &lRowInc, &lColOffset, &lRowOffset);
  if( CALC_ALLCOLS == lSrcCol )
  {
    lColMin = MatMinCol(&lpCalc->Cells);
    lColMax = MatMaxCol(&lpCalc->Cells);
  }
  if( CALC_ALLROWS == lSrcRow )
  {
    lRowMin = MatMinRow(&lpCalc->Cells);
    lRowMax = MatMaxRow(&lpCalc->Cells);
  }
  for( lRow = lRowStart; lRow != lRowEnd; lRow += lRowInc )
  {
    for( lCol = lColStart; lCol != lColEnd; lCol += lColInc )
    {
      if( !lpCalc->lpBook->bFormulaSync )
        CalcMarkDependForEval(lpCalc, lCol, lRow);
      CalcMarkDependForEval(lpCalc, lCol+lColOffset, lRow+lRowOffset);
      lpAdj->lpCurSheet = lpCalc;
      lpAdj->lCurCol = lCol;
      lpAdj->lCurRow = lRow;
      if( lCol != CALC_ALLCOLS && lRow != CALC_ALLROWS )
      {
        if( lpCalc->lpBook->bFormulaSync )
        {
          MarkCellDepend(lpCalc, lCol, lRow, lpAdj);
          MarkCellDepend(lpCalc, lCol+lColOffset, lRow+lRowOffset, lpAdj);
        }
        hExpr = CellSetExpr(lpCalc, lCol, lRow, NULL);
        if( hExpr )
        {
          if( lpCalc->lpBook->bFormulaSync )
            hExpr = lpfnExprIntModify(hExpr, lpAdj);
          CalcMarkCellForEval(lpCalc, lCol+lColOffset, lRow+lRowOffset);
        }
        hExpr = CellSetExpr(lpCalc,lCol+lColOffset,lRow+lRowOffset,hExpr);
        ExprDestroy(hExpr);
      }
      else if( lRow != CALC_ALLROWS )
      {
        if( lpCalc->lpBook->bFormulaSync )
        {
          MarkRowDepend(lpCalc, lRow, lpAdj);
          MarkRowDepend(lpCalc, lRow+lRowOffset, lpAdj);
        }
        hExpr = RowSetExpr(lpCalc, lRow, NULL);
        if( hExpr )
        {
          if( lpCalc->lpBook->bFormulaSync )
            hExpr = lpfnExprIntModify(hExpr, lpAdj);
          CalcMarkRowForEval(lpCalc, lRow+lRowOffset);
        }
        hExpr = RowSetExpr(lpCalc, lRow+lRowOffset, hExpr);
        ExprDestroy(hExpr);
        CalcMoveRangeEx(lpCalc,lColMin,lRow,lColMax,lRow,lColMin,lRow+lRowOffset,lpAdj,lpfnExprIntModify);
      }
      else if( lCol != CALC_ALLCOLS )
      {
        if( lpCalc->lpBook->bFormulaSync )
        {
          MarkColDepend(lpCalc, lCol, lpAdj);
          MarkColDepend(lpCalc, lCol+lColOffset, lpAdj);
        }
        hExpr = ColSetExpr(lpCalc, lCol, NULL);
        if( hExpr )
        {
          if( lpCalc->lpBook->bFormulaSync )
            hExpr = lpfnExprIntModify(hExpr, lpAdj);
          CalcMarkColForEval(lpCalc, lCol+lColOffset);
        }
        hExpr = ColSetExpr(lpCalc, lCol+lColOffset, hExpr);
        ExprDestroy(hExpr);
        CalcMoveRangeEx(lpCalc,lCol,lRowMin,lCol,lRowMax,lCol+lColOffset,lRowMin,lpAdj,lpfnExprIntModify);
      }
    }
  }
  return TRUE;
}

//--------------------------------------------------------------------
//
//  The CalcMoveRange() function moves a range of cell, column,
//  and/or row expressions to another location.
//

BOOL CALCAPI CalcMoveRange(LPCALC_SHEET lpSheet,
                           long lSrcCol, long lSrcRow,
                           long lSrcCol2, long lSrcRow2,
                           long lDestCol, long lDestRow)
{
  CALC_ADJUST adj;
  BOOL bRet;

  if( lSrcCol > lSrcCol2 || lSrcRow > lSrcRow2 )
    return FALSE;  
  if( CALC_ALLCOLS==lSrcCol || CALC_ALLCOLS==lSrcCol2 || CALC_ALLCOLS==lDestCol )
    lSrcCol = lSrcCol2 = lDestCol = CALC_ALLCOLS;
  if( CALC_ALLROWS==lSrcRow || CALC_ALLROWS==lSrcRow2 || CALC_ALLROWS==lDestRow )
    lSrcRow = lSrcRow2 = lDestRow = CALC_ALLROWS;
  InitAdjust(lpSheet, lSrcCol, lSrcRow, lSrcCol2, lSrcRow2, lDestCol, lDestRow, &adj);
  bRet = CalcMoveRangeEx(lpSheet, lSrcCol, lSrcRow, lSrcCol2, lSrcRow2,
                         lDestCol, lDestRow, &adj, ExprIntModifyOnMove);
  if( lpSheet->lpBook->bFormulaSync )
    AdjustDependOnMove(lpSheet, &adj, ExprExtModifyOnMove);
  FreeAdjust(&adj);
  return bRet;
}

//--------------------------------------------------------------------
//
//  The CalcSwapRangeEx() function swaps a range of cell, column,
//  and/or row expressions between two locations.
//

static BOOL CalcSwapRangeEx(LPCALC_INFO lpCalc,
                            long lSrcCol, long lSrcRow,
                            long lSrcCol2, long lSrcRow2,
                            long lDestCol, long lDestRow,
                            LPCALC_ADJUST lpAdjSrc,
                            LPCALC_ADJUST lpAdjDest)
{
  CALC_HANDLE hExpr;
  long lColStart, lRowStart;
  long lColEnd, lRowEnd;
  long lColInc, lRowInc;
  long lColOffset, lRowOffset;
  long lCol, lRow;
  long lColMin, lRowMin;
  long lColMax, lRowMax;
  
  InitLoop(lSrcCol, lSrcRow, lSrcCol2, lSrcRow2, lDestCol, lDestRow,
           &lColStart, &lRowStart, &lColEnd, &lRowEnd,
           &lColInc, &lRowInc, &lColOffset, &lRowOffset);
  if( CALC_ALLCOLS == lSrcCol )
  {
    lColMin = MatMinCol(&lpCalc->Cells);
    lColMax = MatMaxCol(&lpCalc->Cells);
  }
  if( CALC_ALLROWS == lSrcRow )
  {
    lRowMin = MatMinRow(&lpCalc->Cells);
    lRowMax = MatMaxRow(&lpCalc->Cells);
  }
  for( lRow = lRowStart; lRow != lRowEnd; lRow += lRowInc )
  {
    for( lCol = lColStart; lCol != lColEnd; lCol += lColInc )
    {
      if( !lpCalc->lpBook->bFormulaSync )
      {
        CalcMarkDependForEval(lpCalc, lCol, lRow);
        CalcMarkDependForEval(lpCalc, lCol+lColOffset, lRow+lRowOffset);
      }
      lpAdjSrc->lpCurSheet = lpCalc;
      lpAdjSrc->lCurCol = lCol;
      lpAdjSrc->lCurRow = lRow;
      lpAdjDest->lpCurSheet = lpCalc;
      lpAdjDest->lCurCol = lCol+lColOffset;
      lpAdjDest->lCurRow = lRow+lRowOffset;
      if( lCol != CALC_ALLCOLS && lRow != CALC_ALLROWS )
      {
        if( lpCalc->lpBook->bFormulaSync )
        {
          MarkCellDepend(lpCalc, lCol, lRow, lpAdjSrc);
          MarkCellDepend(lpCalc, lCol+lColOffset, lRow+lRowOffset, lpAdjSrc);
        }
        hExpr = CellSetExpr(lpCalc, lCol, lRow, NULL);
        if( hExpr )
        {
          if( lpCalc->lpBook->bFormulaSync )
            hExpr = ExprIntModifyOnSwap(hExpr, lpAdjSrc);
          CalcMarkCellForEval(lpCalc, lCol+lColOffset, lRow+lRowOffset);
        }
        hExpr = CellSetExpr(lpCalc,lCol+lColOffset,lRow+lRowOffset,hExpr);
        if( hExpr )
        {
          if( lpCalc->lpBook->bFormulaSync )
            hExpr = ExprIntModifyOnSwap(hExpr, lpAdjDest);
          CalcMarkCellForEval(lpCalc, lCol, lRow);
        }
        hExpr = CellSetExpr(lpCalc, lCol, lRow, hExpr);
        ExprDestroy(hExpr);
      }
      else if( lRow != CALC_ALLROWS )
      {
        if( lpCalc->lpBook->bFormulaSync )
        {
          MarkRowDepend(lpCalc, lRow, lpAdjSrc);
          MarkRowDepend(lpCalc, lRow+lRowOffset, lpAdjSrc);
        }
        hExpr = RowSetExpr(lpCalc, lRow, NULL);
        if( hExpr )
        {
          if( lpCalc->lpBook->bFormulaSync )
            hExpr = ExprIntModifyOnSwap(hExpr, lpAdjSrc);
          CalcMarkRowForEval(lpCalc, lRow+lRowOffset);
        }
        hExpr = RowSetExpr(lpCalc, lRow+lRowOffset, hExpr);
        if( hExpr )
        {
          if( lpCalc->lpBook->bFormulaSync )
            hExpr = ExprIntModifyOnSwap(hExpr, lpAdjDest);
          CalcMarkRowForEval(lpCalc, lRow);
        }
        hExpr = RowSetExpr(lpCalc, lRow, hExpr);
        ExprDestroy(hExpr);
        CalcSwapRangeEx(lpCalc,lColMin,lRow,lColMax,lRow,lColMin,lRow+lRowOffset,lpAdjSrc,lpAdjDest);
      }
      else if( lCol != CALC_ALLCOLS  )
      {
        if( lpCalc->lpBook->bFormulaSync )
        {
          MarkColDepend(lpCalc, lCol, lpAdjSrc);
          MarkColDepend(lpCalc, lCol+lColOffset, lpAdjSrc);
        }
        hExpr = ColSetExpr(lpCalc, lCol, NULL);
        if( hExpr )
        {
          if( lpCalc->lpBook->bFormulaSync )
            hExpr = ExprIntModifyOnSwap(hExpr, lpAdjSrc);
          CalcMarkColForEval(lpCalc, lCol+lColOffset);
        }
        hExpr = ColSetExpr(lpCalc, lCol+lColOffset, hExpr);
        if( hExpr )
        {
          if( lpCalc->lpBook->bFormulaSync )
            hExpr = ExprIntModifyOnSwap(hExpr, lpAdjDest);
          CalcMarkColForEval(lpCalc, lCol);
        }
        hExpr = ColSetExpr(lpCalc, lCol, hExpr);
        ExprDestroy(hExpr);
        CalcSwapRangeEx(lpCalc,lCol,lRowMin,lCol,lRowMax,lCol+lColOffset,lRowMin,lpAdjSrc,lpAdjDest);
      }
    }
  }
  return TRUE;
}

//--------------------------------------------------------------------
//
//  The CalcSwapRange() function swaps a range of cell, column,
//  and/or row expressions between two locations.
//

BOOL CALCAPI CalcSwapRange(LPCALC_SHEET lpSheet,
                           long lSrcCol, long lSrcRow,
                           long lSrcCol2, long lSrcRow2,
                           long lDestCol, long lDestRow)
{
  CALC_ADJUST adjSrc, adjDest;
  long lDestCol2, lDestRow2;
  BOOL bRet;

  if( lSrcCol > lSrcCol2 || lSrcRow > lSrcRow2 )
    return FALSE;  
  if( CALC_ALLCOLS==lSrcCol || CALC_ALLCOLS==lSrcCol2 || CALC_ALLCOLS==lDestCol )
    lSrcCol = lSrcCol2 = lDestCol = CALC_ALLCOLS;
  if( CALC_ALLROWS==lSrcRow || CALC_ALLROWS==lSrcRow2 || CALC_ALLROWS==lDestRow )
    lSrcRow = lSrcRow2 = lDestRow = CALC_ALLROWS;
  lDestCol2 = lDestCol + lSrcCol2 - lSrcCol;
  lDestRow2 = lDestRow + lSrcRow2 - lSrcRow;
  InitAdjust(lpSheet, lSrcCol, lSrcRow, lSrcCol2, lSrcRow2, lDestCol, lDestRow, &adjSrc);
  InitAdjust(lpSheet, lDestCol, lDestRow, lDestCol2, lDestRow2, lSrcCol, lSrcRow, &adjDest);
  bRet = CalcSwapRangeEx(lpSheet, lSrcCol, lSrcRow, lSrcCol2, lSrcRow2,
                         lDestCol, lDestRow, &adjSrc, &adjDest);
  if( lpSheet->lpBook->bFormulaSync )
    AdjustDependOnSwap(lpSheet, &adjSrc);
  FreeAdjust(&adjSrc);
  FreeAdjust(&adjDest);
  return bRet;
}

//--------------------------------------------------------------------
//
//  The CalcClearRange() function clears a range of cell, column,
//  and/or row expressions.
//

BOOL CALCAPI CalcClearRange(LPCALC_SHEET lpSheet,
                            long lCol1, long lRow1,
                            long lCol2, long lRow2)
{
  CALC_HANDLE hExpr;
  long lMinCol = 1;
  long lMaxCol = 0;
  long lMinRow = 1;
  long lMaxRow = 0;
  long lCol;
  long lRow;

  if( CALC_ALLCOLS == lCol1 && CALC_ALLROWS == lRow1 )
  {
    lMinCol = VectMinIndex(&lpSheet->Cols);
    lMaxCol = VectMaxIndex(&lpSheet->Cols);
    lMinRow = VectMinIndex(&lpSheet->Rows);
    lMaxRow = VectMaxIndex(&lpSheet->Rows);
    lCol1 = MatMinCol(&lpSheet->Cells);
    lCol2 = MatMaxCol(&lpSheet->Cells);
    lRow1 = MatMinRow(&lpSheet->Cells);
    lRow2 = MatMaxRow(&lpSheet->Cells);
  }
  else if( CALC_ALLCOLS == lCol1 )
  {
    lMinRow = lRow1;
    lMaxRow = lRow2;
    lCol1 = MatMinCol(&lpSheet->Cells);
    lCol2 = MatMaxCol(&lpSheet->Cells);
  }
  else if( CALC_ALLROWS == lRow1 )
  {
    lMinCol = lCol1;
    lMaxCol = lCol2;
    lRow1 = MatMinRow(&lpSheet->Cells);
    lRow2 = MatMaxRow(&lpSheet->Cells);
  }
  for( lCol = lMinCol; lCol <= lMaxCol; lCol++ )
  {
    hExpr = ColSetExpr(lpSheet, lCol, NULL);
    ExprDestroy(hExpr);
  }
  for( lRow = lMinRow; lRow <= lMaxRow; lRow++ )
  {
    hExpr = RowSetExpr(lpSheet, lRow, NULL);
    ExprDestroy(hExpr);
  }
  for( lCol = lCol1; lCol <= lCol2; lCol++ )
    for( lRow = lRow1; lRow <= lRow2; lRow ++ )
    {
      hExpr = CellSetExpr(lpSheet, lCol, lRow, NULL);
      ExprDestroy(hExpr);
    }
  return TRUE;
}

//--------------------------------------------------------------------
//
//  The CalcInsColRange() function inserts a range of blank columns.
//

BOOL CALCAPI CalcInsColRange(LPCALC_SHEET lpSheet, long lCol1, long lCol2)
{
  long lMaxColInMat = MatMaxCol(&lpSheet->Cells);
  long lMaxColInVect = VectMaxIndex(&lpSheet->Cols);
  long lMaxColInSheet = lpSheet->lpfnGetMaxCols(lpSheet->hSS);
  long lMaxCol = min(max(lMaxColInMat, lMaxColInVect), lMaxColInSheet);
  CALC_ADJUST adj;

  InitAdjust(lpSheet, lCol1, CALC_ALLROWS, CALC_MAX_COORD - (lCol2 + 1 - lCol1), CALC_ALLROWS, lCol2 + 1, CALC_ALLROWS, &adj);
  if (lCol1 <= lMaxCol)
    CalcMoveRangeEx(lpSheet, lCol1, CALC_ALLROWS, lMaxCol, CALC_ALLROWS, lCol2 + 1, CALC_ALLROWS, &adj, ExprIntModifyOnInsCols);
  if (lpSheet->lpBook->bFormulaSync)
    AdjustDependOnMove(lpSheet, &adj, ExprExtModifyOnInsCols);
  FreeAdjust(&adj);
  return TRUE;
}

//--------------------------------------------------------------------
//
//  The CalcInsRowRange() function inserts a range of blank rows.
//

BOOL CALCAPI CalcInsRowRange(LPCALC_SHEET lpSheet, long lRow1, long lRow2)
{
  long lMaxRowInMat = MatMaxRow(&lpSheet->Cells);
  long lMaxRowInVect = VectMaxIndex(&lpSheet->Rows);
  long lMaxRowInSheet = lpSheet->lpfnGetMaxRows(lpSheet->hSS);
  long lMaxRow = min(max(lMaxRowInMat, lMaxRowInVect), lMaxRowInSheet);
  CALC_ADJUST adj;

  InitAdjust(lpSheet, CALC_ALLCOLS, lRow1, CALC_ALLCOLS, CALC_MAX_COORD - (lRow2 + 1 - lRow1), CALC_ALLCOLS, lRow2 + 1, &adj);
  if (lRow1 <= lMaxRow)
    CalcMoveRangeEx(lpSheet, CALC_ALLCOLS, lRow1, CALC_ALLCOLS, lMaxRow, CALC_ALLCOLS, lRow2 + 1, &adj, ExprIntModifyOnInsRows);
  if (lpSheet->lpBook->bFormulaSync)
    AdjustDependOnMove(lpSheet, &adj, ExprExtModifyOnInsRows);
  FreeAdjust(&adj);
  return TRUE;
}

//--------------------------------------------------------------------
//
//  The CalcDelColRange() function deletes a range of cell and/or
//  column expressions.
//

BOOL CALCAPI CalcDelColRange(LPCALC_SHEET lpSheet, long lCol1, long lCol2)
{
  long lMaxColInMat = MatMaxCol(&lpSheet->Cells);
  long lMaxColInVect = VectMaxIndex(&lpSheet->Cols);
  long lMaxColInSheet = lpSheet->lpfnGetMaxCols(lpSheet->hSS);
  long lMaxCol = min(max(lMaxColInMat, lMaxColInVect), lMaxColInSheet);
  CALC_ADJUST adj;

  InitAdjust(lpSheet, lCol2 + 1, CALC_ALLROWS, CALC_MAX_COORD, CALC_ALLROWS, lCol1, CALC_ALLROWS, &adj);
  if (lCol1 <= lMaxCol)
    CalcClearRange(lpSheet, lCol1, CALC_ALLROWS, lCol2, CALC_ALLROWS);
  if (lCol2 + 1 <= lMaxCol)
    CalcMoveRangeEx(lpSheet, lCol2 + 1, CALC_ALLROWS, lMaxCol, CALC_ALLROWS, lCol1, CALC_ALLROWS, &adj, ExprIntModifyOnDelCols);
  if (lpSheet->lpBook->bFormulaSync)
    AdjustDependOnMove(lpSheet, &adj, ExprExtModifyOnDelCols);
  FreeAdjust(&adj);
  return TRUE;
}

//--------------------------------------------------------------------
//
//  The CalcDelRowRange() function deletes a range of cell and/or
//  row expressions.
//

BOOL CALCAPI CalcDelRowRange(LPCALC_SHEET lpSheet, long lRow1, long lRow2)
{
  long lMaxRowInMat = MatMaxRow(&lpSheet->Cells);
  long lMaxRowInVect = VectMaxIndex(&lpSheet->Rows);
  long lMaxRowInSheet = lpSheet->lpfnGetMaxRows(lpSheet->hSS);
  long lMaxRow = min(max(lMaxRowInMat, lMaxRowInVect), lMaxRowInSheet);
  CALC_ADJUST adj;

  InitAdjust(lpSheet, CALC_ALLCOLS, lRow2 + 1, CALC_ALLCOLS, CALC_MAX_COORD, CALC_ALLCOLS, lRow1, &adj);
  if (lRow1 <= lMaxRow)
    CalcClearRange(lpSheet, CALC_ALLCOLS, lRow1, CALC_ALLCOLS, lRow2);
  if (lRow2 + 1 <= lMaxRow)
    CalcMoveRangeEx(lpSheet, CALC_ALLCOLS, lRow2 + 1, CALC_ALLCOLS, lMaxRow, CALC_ALLCOLS, lRow1, &adj, ExprIntModifyOnDelRows);
  if (lpSheet->lpBook->bFormulaSync)
    AdjustDependOnMove(lpSheet, &adj, ExprExtModifyOnDelRows);
  FreeAdjust(&adj);
  return TRUE;
}

//--------------------------------------------------------------------

#if defined(SS_V70)
static void AdjustDependOnFreeSheet(LPCALC_INFO lpCalc, LPCALC_ADJUST lpAdj)
{
  LPCALC_BOOK lpBook = lpCalc->lpBook;
  CALC_HANDLE hExpr;
  CALC_HANDLE hName;
  LPCALC_HANDLE lpElem;
  LPCALC_SHEET lpSheet;
  long lCol, lRow;
  int i;

  // Modify cell expressions which depend on source block
  while (lpBook->hHeadAdjustCells != NULL)
  {
    lpSheet = Cell_GetSheet(lpBook->hHeadAdjustCells);
    if (lpSheet != lpAdj->lpSrcSheet)
    {
      lCol = Cell_GetCol(lpBook->hHeadAdjustCells);
      lRow = Cell_GetRow(lpBook->hHeadAdjustCells);
      lpAdj->lpCurSheet = lpSheet;
      lpAdj->lCurCol = lCol;
      lpAdj->lCurRow = lRow;
      hExpr = CellSetExpr(lpSheet, lCol, lRow, NULL);
      hExpr = ExprExtModifyOnDelSheet(hExpr, lpAdj);
      hExpr = CellSetExpr(lpSheet, lCol, lRow, hExpr);
      Book_AddDirtyCell(lpBook, lpBook->hHeadAdjustCells);
    }
    Book_RemoveAdjustCell(lpBook, lpBook->hHeadAdjustCells);
  }
  // Modify column expressions which depend on source block
  while (lpBook->hHeadAdjustCols != NULL)
  {
    lpSheet = Col_GetSheet(lpBook->hHeadAdjustCols);
    if (lpSheet != lpAdj->lpSrcSheet)
    {
      lCol = Col_GetCol(lpBook->hHeadAdjustCols);
      lpAdj->lpCurSheet = lpSheet;
      lpAdj->lCurCol = lCol;
      lpAdj->lCurRow = CALC_ALLROWS;
      hExpr = ColSetExpr(lpSheet, lCol, NULL);
      hExpr = ExprExtModifyOnDelSheet(hExpr, lpAdj);
      hExpr = ColSetExpr(lpSheet, lCol, hExpr);
      Book_AddDirtyCol(lpBook, lpBook->hHeadAdjustCols);
    }
    Book_RemoveAdjustCol(lpBook, lpBook->hHeadAdjustCols);
  }
  // Modify row expressions which depend on source block
  while (lpBook->hHeadAdjustRows != NULL)
  {
    lpSheet = Row_GetSheet(lpBook->hHeadAdjustRows);
    if (lpSheet != lpAdj->lpSrcSheet)
    {
      lRow = Row_GetRow(lpBook->hHeadAdjustRows);
      lpAdj->lpCurSheet = lpSheet;
      lpAdj->lCurCol = CALC_ALLCOLS;
      lpAdj->lCurRow = lRow;
      hExpr = RowSetExpr(lpSheet, lRow, NULL);
      hExpr = ExprExtModifyOnDelSheet(hExpr, lpAdj);
      hExpr = RowSetExpr(lpSheet, lRow, hExpr);
      Book_AddDirtyRow(lpBook, lpBook->hHeadAdjustRows);
    }
    Book_RemoveAdjustRow(lpBook, lpBook->hHeadAdjustRows);
  }
  // Modify custom named expressions which depend on source block
  if( lpCalc->lpBook->Names.hElem &&
      (lpElem = (LPCALC_HANDLE)CalcMemLock(lpCalc->lpBook->Names.hElem)) )
  {
    lpAdj->lpCurSheet = NULL;
    lpAdj->lCurCol = CALC_ALLCOLS;
    lpAdj->lCurRow = CALC_ALLROWS;
    for( i = 0; i < lpCalc->lpBook->Names.nElemCnt; i++ )
    {
      hName = lpElem[i];
      hExpr = NameGetExpr(hName);
      SetLinksUsingName(lpCalc, hName, FALSE);
      hExpr = ExprExtModifyOnDelSheet(ExprCreateRef(hExpr), lpAdj);
      NameSetExpr(hName, hExpr);
      SetLinksUsingName(lpCalc, hName, TRUE);
      #if defined(SS_V70)
      Name_AddListenersToDirty(hName);
      #else
      NameMarkDepend(hName, &lpCalc->CellsToEval, &lpCalc->ColsToEval, &lpCalc->RowsToEval);
      #endif
    }
    CalcMemUnlock(lpCalc->lpBook->Names.hElem);
  }
}
#endif

//--------------------------------------------------------------------
//

//BOOL CALCAPI CalcFillDown(LPCALC_SHEET lpSheet, long lCol1, long lRow1,
//                          long lCol2, long lRow2)
//{
//  CALC_HANDLE hExpr;
//  CALC_HANDLE hExprPrev;
//  long lCol;
//  long lRow;
//
//  for( lCol = lCol1; lCol <= lCol2; lCol++ )
//  { 
//    hExpr = CellGetExpr(&lpSheet->Cells, lCol, lRow1);
//    for( lRow = lRow1 + 1; lRow <= lRow2; lRow++ )
//    {
//      hExpr = ExprCreateRef(hExpr);
//      hExprPrev = CellSetExpr(&lpSheet->Cells, lCol, lRow, hExpr);
//      BitMatSet(&lpSheet->CellsToEval, lCol, lRow, TRUE);
//      ExprDestroy(hExprPrev);
//    }
//  }
//  return TRUE;
//}

//--------------------------------------------------------------------
//

//BOOL CALCAPI CalcFillRight(LPCALC_SHEET lpSheet, long lCol1, long lRow1,
//                           long lCol2, long lRow2)
//{
//  CALC_HANDLE hExpr;
//  CALC_HANDLE hExprPrev;
//  long lCol;
//  long lRow;
//
//  for( lRow = lRow1; lRow <= lRow2; lRow++ )
//  {
//    hExpr = CellGetExpr(&lpSheet->Cells, lCol1, lRow);
//    for( lCol = lCol1 + 1; lCol <= lCol2; lCol++ )
//    {
//      hExpr = ExprCreateRef(hExpr);
//      hExprPrev = CellSetExpr(&lpSheet->Cells, lCol, lRow, hExpr);
//      BitMatSet(&lpSheet->CellsToEval, lCol, lRow, TRUE);
//      ExprDestroy(hExprPrev);
//    }
//  }
//  return TRUE;
//}

//--------------------------------------------------------------------

#if defined(SS_V70)

//--------------------------------------------------------------------

BOOL Book_IsAdjustCell(LPCALC_BOOK lpThis, CALC_HANDLE hCell)
{
  return Cell_GetPreviousAdjust(hCell) != NULL || hCell == lpThis->hHeadAdjustCells;
}

//--------------------------------------------------------------------

void Book_AddAdjustCell(LPCALC_BOOK lpThis, CALC_HANDLE hCell)
{
  if (hCell != NULL && !Book_IsAdjustCell(lpThis, hCell))
  {
    if (lpThis->hTailAdjustCells != NULL)
      Cell_SetNextAdjust(lpThis->hTailAdjustCells, hCell);
    else
      lpThis->hHeadAdjustCells = hCell;
    Cell_SetPreviousAdjust(hCell, lpThis->hTailAdjustCells);
    Cell_SetNextAdjust(hCell, NULL);
    //????Cell_StopListening(hCell, TRUE);
    lpThis->hTailAdjustCells = hCell;
  }
}

//--------------------------------------------------------------------

void Book_RemoveAdjustCell(LPCALC_BOOK lpThis, CALC_HANDLE hCell)
{
  if (Book_IsAdjustCell(lpThis, hCell))
  {
    CALC_HANDLE hPreviousAdjust = Cell_GetPreviousAdjust(hCell);
    CALC_HANDLE hNextAdjust = Cell_GetNextAdjust(hCell);
    if (hPreviousAdjust != NULL)
      Cell_SetNextAdjust(hPreviousAdjust, hNextAdjust);
    else
      lpThis->hHeadAdjustCells = hNextAdjust;
    if (hNextAdjust != NULL)
      Cell_SetPreviousAdjust(hNextAdjust, hPreviousAdjust);
    else
      lpThis->hTailAdjustCells = hPreviousAdjust;
    Cell_SetPreviousAdjust(hCell, NULL);
    Cell_SetNextAdjust(hCell, NULL);
    //????Cell_StartListening(hCell, TRUE);
  }
}

//--------------------------------------------------------------------

BOOL Book_IsAdjustRow(LPCALC_BOOK lpThis, CALC_HANDLE hRow)
{
  return Row_GetPreviousAdjust(hRow) != NULL || hRow == lpThis->hHeadAdjustRows;
}

//--------------------------------------------------------------------

void Book_AddAdjustRow(LPCALC_BOOK lpThis, CALC_HANDLE hRow)
{
  if (hRow != NULL && !Book_IsAdjustRow(lpThis, hRow))
  {
    if (lpThis->hTailAdjustRows != NULL)
      Row_SetNextAdjust(lpThis->hTailAdjustRows, hRow);
    else
      lpThis->hHeadAdjustRows = hRow;
    Row_SetPreviousAdjust(hRow, lpThis->hTailAdjustRows);
    Row_SetNextAdjust(hRow, NULL);
    //????Row_StopListening(hRow, TRUE);
    lpThis->hTailAdjustRows = hRow;
  }
}

//--------------------------------------------------------------------

void Book_RemoveAdjustRow(LPCALC_BOOK lpThis, CALC_HANDLE hRow)
{
  if (Book_IsAdjustRow(lpThis, hRow))
  {
    CALC_HANDLE hPreviousAdjust = Row_GetPreviousAdjust(hRow);
    CALC_HANDLE hNextAdjust = Row_GetNextAdjust(hRow);
    if (hPreviousAdjust != NULL)
      Row_SetNextAdjust(hPreviousAdjust, hNextAdjust);
    else
      lpThis->hHeadAdjustRows = hNextAdjust;
    if (hNextAdjust != NULL)
      Row_SetPreviousAdjust(hNextAdjust, hPreviousAdjust);
    else
      lpThis->hTailAdjustRows = hPreviousAdjust;
    Row_SetPreviousAdjust(hRow, NULL);
    Row_SetNextAdjust(hRow, NULL);
    //????Row_StartListening(hRow, TRUE);
  }
}

//--------------------------------------------------------------------

BOOL Book_IsAdjustCol(LPCALC_BOOK lpThis, CALC_HANDLE hCol)
{
  return Col_GetPreviousAdjust(hCol) != NULL || hCol == lpThis->hHeadAdjustCols;
}

//--------------------------------------------------------------------

void Book_AddAdjustCol(LPCALC_BOOK lpThis, CALC_HANDLE hCol)
{
  if (hCol != NULL && !Book_IsAdjustCol(lpThis, hCol))
  {
    if (lpThis->hTailAdjustCols != NULL)
      Col_SetNextAdjust(lpThis->hTailAdjustCols, hCol);
    else
      lpThis->hHeadAdjustCols = hCol;
    Col_SetPreviousAdjust(hCol, lpThis->hTailAdjustCols);
    Col_SetNextAdjust(hCol, NULL);
    //????Col_StopListening(hCol, TRUE);
    lpThis->hTailAdjustCols = hCol;
  }
}

//--------------------------------------------------------------------

void Book_RemoveAdjustCol(LPCALC_BOOK lpThis, CALC_HANDLE hCol)
{
  if (Book_IsAdjustCol(lpThis, hCol))
  {
    CALC_HANDLE hPreviousAdjust = Col_GetPreviousAdjust(hCol);
    CALC_HANDLE hNextAdjust = Col_GetNextAdjust(hCol);
    if (hPreviousAdjust != NULL)
      Col_SetNextAdjust(hPreviousAdjust, hNextAdjust);
    else
      lpThis->hHeadAdjustCols = hNextAdjust;
    if (hNextAdjust != NULL)
      Col_SetPreviousAdjust(hNextAdjust, hPreviousAdjust);
    else
      lpThis->hTailAdjustCols = hPreviousAdjust;
    Col_SetPreviousAdjust(hCol, NULL);
    Col_SetNextAdjust(hCol, NULL);
    //????Col_StartListening(hCol, TRUE);
  }
}

//--------------------------------------------------------------------

BOOL Book_IsDirtyCell(LPCALC_BOOK lpThis, CALC_HANDLE hCell)
{
  return Cell_GetPreviousDirty(hCell) != NULL || hCell == lpThis->hHeadDirtyCells;
}

//--------------------------------------------------------------------

void Book_AddDirtyCell(LPCALC_BOOK lpThis, CALC_HANDLE hCell)
{
  if (hCell != NULL && !Book_IsDirtyCell(lpThis, hCell))
  {
    if (lpThis->hTailDirtyCells != NULL)
      Cell_SetNextDirty(lpThis->hTailDirtyCells, hCell);
    else
      lpThis->hHeadDirtyCells = hCell;
    Cell_SetPreviousDirty(hCell, lpThis->hTailDirtyCells);
    Cell_SetNextDirty(hCell, NULL);
    lpThis->hTailDirtyCells = hCell;
  }
}

//--------------------------------------------------------------------

void Book_RemoveDirtyCell(LPCALC_BOOK lpThis, CALC_HANDLE hCell)
{
  if (Book_IsDirtyCell(lpThis, hCell))
  {
    CALC_HANDLE hPreviousDirty = Cell_GetPreviousDirty(hCell);
    CALC_HANDLE hNextDirty = Cell_GetNextDirty(hCell);
    if (hPreviousDirty != NULL)
      Cell_SetNextDirty(hPreviousDirty, hNextDirty);
    else
      lpThis->hHeadDirtyCells = hNextDirty;
    if (hNextDirty != NULL)
      Cell_SetPreviousDirty(hNextDirty, hPreviousDirty);
    else
      lpThis->hTailDirtyCells = hPreviousDirty;
    Cell_SetPreviousDirty(hCell, NULL);
    Cell_SetNextDirty(hCell, NULL);
  }
}

//--------------------------------------------------------------------

BOOL Book_IsDirtyRow(LPCALC_BOOK lpThis, CALC_HANDLE hRow)
{
  return Row_GetPreviousDirty(hRow) != NULL || hRow == lpThis->hHeadDirtyRows;
}

//--------------------------------------------------------------------

void Book_AddDirtyRow(LPCALC_BOOK lpThis, CALC_HANDLE hRow)
{
  if (hRow != NULL && !Book_IsDirtyRow(lpThis, hRow))
  {
    if (lpThis->hTailDirtyRows != NULL)
      Row_SetNextDirty(lpThis->hTailDirtyRows, hRow);
    else
      lpThis->hHeadDirtyRows = hRow;
    Row_SetPreviousDirty(hRow, lpThis->hTailDirtyRows);
    Row_SetNextDirty(hRow, NULL);
    lpThis->hTailDirtyRows = hRow;
  }
}

//--------------------------------------------------------------------

void Book_RemoveDirtyRow(LPCALC_BOOK lpThis, CALC_HANDLE hRow)
{
  if (Book_IsDirtyRow(lpThis, hRow))
  {
    CALC_HANDLE hPreviousDirty = Row_GetPreviousDirty(hRow);
    CALC_HANDLE hNextDirty = Row_GetNextDirty(hRow);
    if (hPreviousDirty != NULL)
      Row_SetNextDirty(hPreviousDirty, hNextDirty);
    else
      lpThis->hHeadDirtyRows = hNextDirty;
    if (hNextDirty != NULL)
      Row_SetPreviousDirty(hNextDirty, hPreviousDirty);
    else
      lpThis->hTailDirtyRows = hPreviousDirty;
    Row_SetPreviousDirty(hRow, NULL);
    Row_SetNextDirty(hRow, NULL);
  }
}

//--------------------------------------------------------------------

BOOL Book_IsDirtyCol(LPCALC_BOOK lpThis, CALC_HANDLE hCol)
{
  return Col_GetPreviousDirty(hCol) != NULL || hCol == lpThis->hHeadDirtyCols;
}

//--------------------------------------------------------------------

void Book_AddDirtyCol(LPCALC_BOOK lpThis, CALC_HANDLE hCol)
{
  if (hCol != NULL && !Book_IsDirtyCol(lpThis, hCol))
  {
    if (lpThis->hTailDirtyCols != NULL)
      Col_SetNextDirty(lpThis->hTailDirtyCols, hCol);
    else
      lpThis->hHeadDirtyCols = hCol;
    Col_SetPreviousDirty(hCol, lpThis->hTailDirtyCols);
    Col_SetNextDirty(hCol, NULL);
    lpThis->hTailDirtyCols = hCol;
  }
}

//--------------------------------------------------------------------

void Book_RemoveDirtyCol(LPCALC_BOOK lpThis, CALC_HANDLE hCol)
{
  if (Book_IsDirtyCol(lpThis, hCol))
  {
    CALC_HANDLE hPreviousDirty = Col_GetPreviousDirty(hCol);
    CALC_HANDLE hNextDirty = Col_GetNextDirty(hCol);
    if (hPreviousDirty != NULL)
      Col_SetNextDirty(hPreviousDirty, hNextDirty);
    else
      lpThis->hHeadDirtyCols = hNextDirty;
    if (hNextDirty != NULL)
      Col_SetPreviousDirty(hNextDirty, hPreviousDirty);
    else
      lpThis->hTailDirtyCols = hPreviousDirty;
    Col_SetPreviousDirty(hCol, NULL);
    Col_SetNextDirty(hCol, NULL);
  }
}

//--------------------------------------------------------------------

void Sheet_IncrementDelay(LPCALC_SHEET lpThis, long lCol, long lRow)
{
  long lColCnt = 1;
  long lRowCnt = 1;
  long r;
  long c;

  if (lCol == CALC_ALLCOLS)
  {
    lCol = 1;
    lpThis->lpfnGetDataCnt(lpThis->hSS, &lColCnt, NULL);
  }
  if (lRow == CALC_ALLROWS)
  {
    lRow = 1;
    lpThis->lpfnGetDataCnt(lpThis->hSS, NULL, &lRowCnt);
  }
  for (r = lRow; r < lRow + lRowCnt; r++)
  {
    for (c = lCol; c < lCol + lColCnt; c++)
    {
      CALC_HANDLE hCell = CellLookup(lpThis, c, r);
      if (hCell)
        Cell_SetDelay(hCell, Cell_GetDelay(hCell) + 1);
    }
  }
}

//--------------------------------------------------------------------

void Sheet_DecrementDelay(LPCALC_SHEET lpThis, long lCol, long lRow)
{
  long lColCnt = 1;
  long lRowCnt = 1;
  long r;
  long c;

  if (lCol == CALC_ALLCOLS)
  {
    lCol = 1;
    lpThis->lpfnGetDataCnt(lpThis->hSS, &lColCnt, NULL);
  }
  if (lRow == CALC_ALLROWS)
  {
    lRow = 1;
    lpThis->lpfnGetDataCnt(lpThis->hSS, NULL, &lRowCnt);
  }
  for (r = lRow; r < lRow + lRowCnt; r++)
  {
    for (c = lCol; c < lCol + lColCnt; c++)
    {
      CALC_HANDLE hCell = CellLookup(lpThis, c, r);
      if (hCell)
        Cell_SetDelay(hCell, Cell_GetDelay(hCell) - 1);
    }
  }
}

//--------------------------------------------------------------------

typedef struct tagCALC_SHEET_CELLCELLLISTENERLIST
{
  long lCol1;
  long lRow1;
  long lCol2;
  long lRow2;
  CALC_HANDLE hCell;
  CALC_HANDLE hNext;
} CALC_SHEET_CELLCELLLISTENERLIST, FAR* LPCALC_SHEET_CELLCELLLISTENERLIST;

//--------------------------------------------------------------------

#define Sheet_CellCellListenerList_MemAlloc() (CALC_HANDLE)CalcMemAlloc(sizeof(CALC_SHEET_CELLCELLLISTENERLIST))
#define Sheet_CellCellListenerList_MemFree(hMem) CalcMemFree(hMem)
#define Sheet_CellCellListenerList_MemLock(hMem) (LPCALC_SHEET_CELLCELLLISTENERLIST)CalcMemLock(hMem)
#define Sheet_CellCellListenerList_MemUnlock(hMem) CalcMemUnlock(hMem)

//--------------------------------------------------------------------

CALC_HANDLE Sheet_CellCellListenerList_Create(long lCol1, long lRow1, long lCol2, long lRow2, CALC_HANDLE hCell, CALC_HANDLE hNext)
{
  LPCALC_SHEET_CELLCELLLISTENERLIST lpThis;
  CALC_HANDLE hThis;

  hThis = Sheet_CellCellListenerList_MemAlloc();
  if (hThis && (lpThis = Sheet_CellCellListenerList_MemLock(hThis)))
  {
    lpThis->lCol1 = lCol1;
    lpThis->lRow1 = lRow1;
    lpThis->lCol2 = lCol2;
    lpThis->lRow2 = lRow2;
    lpThis->hCell = hCell;
    lpThis->hNext = hNext;
    Sheet_CellCellListenerList_MemUnlock(hThis);
  }
  return hThis;
}

//--------------------------------------------------------------------

void Sheet_CellCellListenerList_Destroy(CALC_HANDLE hThis)
{
  LPCALC_SHEET_CELLCELLLISTENERLIST lpThis;

  if (hThis && (lpThis = Sheet_CellCellListenerList_MemLock(hThis)))
  {
    Sheet_CellCellListenerList_MemUnlock(hThis);
    Sheet_CellCellListenerList_MemFree(hThis);
  }
}

//--------------------------------------------------------------------

BOOL Sheet_CellCellListenerList_Equals(CALC_HANDLE hThis, long lCol1, long lRow1, long lCol2, long lRow2, CALC_HANDLE hCell)
{
  LPCALC_SHEET_CELLCELLLISTENERLIST lpThis;
  BOOL bRet = FALSE;

  if (hThis && (lpThis = Sheet_CellCellListenerList_MemLock(hThis)))
  {
    bRet = lCol1 == lpThis->lCol1 && lRow1 == lpThis->lRow1 &&
           lCol2 == lpThis->lCol2 && lRow2 == lpThis->lRow2 &&
           hCell == lpThis->hCell;
    Sheet_CellCellListenerList_MemUnlock(hThis);
  }
  return bRet;
}

//--------------------------------------------------------------------

BOOL Sheet_CellCellListenerList_Contains(CALC_HANDLE hThis, long lCol, long lRow)
{
  LPCALC_SHEET_CELLCELLLISTENERLIST lpThis;
  BOOL bRet = FALSE;

  if (hThis && (lpThis = Sheet_CellCellListenerList_MemLock(hThis)))
  {
    bRet = (CALC_ALLCOLS == lCol || (lpThis->lCol1 <= lCol && lCol <= lpThis->lCol2)) &&
           (CALC_ALLROWS == lRow || (lpThis->lRow1 <= lRow && lRow <= lpThis->lRow2));
    Sheet_CellCellListenerList_MemUnlock(hThis);
  }
  return bRet;
}

//--------------------------------------------------------------------

CALC_HANDLE Sheet_CellCellListenerList_GetCell(CALC_HANDLE hThis)
{
  LPCALC_SHEET_CELLCELLLISTENERLIST lpThis;
  CALC_HANDLE hValue = NULL;

  if (hThis && (lpThis = Sheet_CellCellListenerList_MemLock(hThis)))
  {
    hValue = lpThis->hCell;
    Sheet_CellCellListenerList_MemUnlock(hThis);
  }
  return hValue;
}

//--------------------------------------------------------------------

CALC_HANDLE Sheet_CellCellListenerList_GetNext(CALC_HANDLE hThis)
{
  LPCALC_SHEET_CELLCELLLISTENERLIST lpThis;
  CALC_HANDLE hValue = NULL;

  if (hThis && (lpThis = Sheet_CellCellListenerList_MemLock(hThis)))
  {
    hValue = lpThis->hNext;
    Sheet_CellCellListenerList_MemUnlock(hThis);
  }
  return hValue;
}

//--------------------------------------------------------------------

void Sheet_CellCellListenerList_SetNext(CALC_HANDLE hThis, CALC_HANDLE hValue)
{
  LPCALC_SHEET_CELLCELLLISTENERLIST lpThis;

  if (hThis && (lpThis = Sheet_CellCellListenerList_MemLock(hThis)))
  {
    lpThis->hNext = hValue;
    Sheet_CellCellListenerList_MemUnlock(hThis);
  }
}

//--------------------------------------------------------------------

typedef struct tagCALC_SHEET_CELLROWLISTENERLIST
{
  long lCol1;
  long lRow1;
  long lCol2;
  long lRow2;
  CALC_HANDLE hRow;
  CALC_HANDLE hNext;
} CALC_SHEET_CELLROWLISTENERLIST, FAR* LPCALC_SHEET_CELLROWLISTENERLIST;

//--------------------------------------------------------------------

#define Sheet_CellRowListenerList_MemAlloc() (CALC_HANDLE)CalcMemAlloc(sizeof(CALC_SHEET_CELLROWLISTENERLIST))
#define Sheet_CellRowListenerList_MemFree(hMem) CalcMemFree(hMem)
#define Sheet_CellRowListenerList_MemLock(hMem) (LPCALC_SHEET_CELLROWLISTENERLIST)CalcMemLock(hMem)
#define Sheet_CellRowListenerList_MemUnlock(hMem) CalcMemUnlock(hMem)

//--------------------------------------------------------------------

CALC_HANDLE Sheet_CellRowListenerList_Create(long lCol1, long lRow1, long lCol2, long lRow2, CALC_HANDLE hRow, CALC_HANDLE hNext)
{
  LPCALC_SHEET_CELLROWLISTENERLIST lpThis;
  CALC_HANDLE hThis;

  hThis = Sheet_CellRowListenerList_MemAlloc();
  if (hThis && (lpThis = Sheet_CellRowListenerList_MemLock(hThis)))
  {
    lpThis->lCol1 = lCol1;
    lpThis->lRow1 = lRow1;
    lpThis->lCol2 = lCol2;
    lpThis->lRow2 = lRow2;
    lpThis->hRow = hRow;
    lpThis->hNext = hNext;
    Sheet_CellRowListenerList_MemUnlock(hThis);
  }
  return hThis;
}

//--------------------------------------------------------------------

void Sheet_CellRowListenerList_Destroy(CALC_HANDLE hThis)
{
  LPCALC_SHEET_CELLROWLISTENERLIST lpThis;

  if (hThis && (lpThis = Sheet_CellRowListenerList_MemLock(hThis)))
  {
    Sheet_CellRowListenerList_MemUnlock(hThis);
    Sheet_CellRowListenerList_MemFree(hThis);
  }
}

//--------------------------------------------------------------------

BOOL Sheet_CellRowListenerList_Equals(CALC_HANDLE hThis, long lCol1, long lRow1, long lCol2, long lRow2, CALC_HANDLE hRow)
{
  LPCALC_SHEET_CELLROWLISTENERLIST lpThis;
  BOOL bRet = FALSE;

  if (hThis && (lpThis = Sheet_CellRowListenerList_MemLock(hThis)))
  {
    bRet = lCol1 == lpThis->lCol1 && lRow1 == lpThis->lRow1 &&
           lCol2 == lpThis->lCol2 && lRow2 == lpThis->lRow2 &&
           hRow == lpThis->hRow;
    Sheet_CellRowListenerList_MemUnlock(hThis);
  }
  return bRet;
}

//--------------------------------------------------------------------

BOOL Sheet_CellRowListenerList_Contains(CALC_HANDLE hThis, long lCol, long lRow)
{
  LPCALC_SHEET_CELLROWLISTENERLIST lpThis;
  BOOL bRet = FALSE;

  if (hThis && (lpThis = Sheet_CellRowListenerList_MemLock(hThis)))
  {
    bRet = (CALC_ALLCOLS == lCol || (lpThis->lCol1 <= lCol && lCol <= lpThis->lCol2)) &&
           (CALC_ALLROWS == lRow || (lpThis->lRow1 <= lRow && lRow <= lpThis->lRow2));
    Sheet_CellRowListenerList_MemUnlock(hThis);
  }
  return bRet;
}

//--------------------------------------------------------------------

CALC_HANDLE Sheet_CellRowListenerList_GetRow(CALC_HANDLE hThis)
{
  LPCALC_SHEET_CELLROWLISTENERLIST lpThis;
  CALC_HANDLE hValue = NULL;

  if (hThis && (lpThis = Sheet_CellRowListenerList_MemLock(hThis)))
  {
    hValue = lpThis->hRow;
    Sheet_CellRowListenerList_MemUnlock(hThis);
  }
  return hValue;
}

//--------------------------------------------------------------------

CALC_HANDLE Sheet_CellRowListenerList_GetNext(CALC_HANDLE hThis)
{
  LPCALC_SHEET_CELLROWLISTENERLIST lpThis;
  CALC_HANDLE hValue = NULL;

  if (hThis && (lpThis = Sheet_CellRowListenerList_MemLock(hThis)))
  {
    hValue = lpThis->hNext;
    Sheet_CellRowListenerList_MemUnlock(hThis);
  }
  return hValue;
}

//--------------------------------------------------------------------

void Sheet_CellRowListenerList_SetNext(CALC_HANDLE hThis, CALC_HANDLE hValue)
{
  LPCALC_SHEET_CELLROWLISTENERLIST lpThis;

  if (hThis && (lpThis = Sheet_CellRowListenerList_MemLock(hThis)))
  {
    lpThis->hNext = hValue;
    Sheet_CellRowListenerList_MemUnlock(hThis);
  }
}

//--------------------------------------------------------------------

typedef struct tagCALC_SHEET_CELLCOLLISTENERLIST
{
  long lCol1;
  long lRow1;
  long lCol2;
  long lRow2;
  CALC_HANDLE hCol;
  CALC_HANDLE hNext;
} CALC_SHEET_CELLCOLLISTENERLIST, FAR* LPCALC_SHEET_CELLCOLLISTENERLIST;

//--------------------------------------------------------------------

#define Sheet_CellColListenerList_MemAlloc() (CALC_HANDLE)CalcMemAlloc(sizeof(CALC_SHEET_CELLCOLLISTENERLIST))
#define Sheet_CellColListenerList_MemFree(hMem) CalcMemFree(hMem)
#define Sheet_CellColListenerList_MemLock(hMem) (LPCALC_SHEET_CELLCOLLISTENERLIST)CalcMemLock(hMem)
#define Sheet_CellColListenerList_MemUnlock(hMem) CalcMemUnlock(hMem)

//--------------------------------------------------------------------

CALC_HANDLE Sheet_CellColListenerList_Create(long lCol1, long lRow1, long lCol2, long lRow2, CALC_HANDLE hCol, CALC_HANDLE hNext)
{
  LPCALC_SHEET_CELLCOLLISTENERLIST lpThis;
  CALC_HANDLE hThis;

  hThis = Sheet_CellColListenerList_MemAlloc();
  if (hThis && (lpThis = Sheet_CellColListenerList_MemLock(hThis)))
  {
    lpThis->lCol1 = lCol1;
    lpThis->lRow1 = lRow1;
    lpThis->lCol2 = lCol2;
    lpThis->lRow2 = lRow2;
    lpThis->hCol = hCol;
    lpThis->hNext = hNext;
    Sheet_CellColListenerList_MemUnlock(hThis);
  }
  return hThis;
}

//--------------------------------------------------------------------

void Sheet_CellColListenerList_Destroy(CALC_HANDLE hThis)
{
  LPCALC_SHEET_CELLCOLLISTENERLIST lpThis;

  if (hThis && (lpThis = Sheet_CellColListenerList_MemLock(hThis)))
  {
    Sheet_CellColListenerList_MemUnlock(hThis);
    Sheet_CellColListenerList_MemFree(hThis);
  }
}

//--------------------------------------------------------------------

BOOL Sheet_CellColListenerList_Equals(CALC_HANDLE hThis, long lCol1, long lRow1, long lCol2, long lRow2, CALC_HANDLE hCol)
{
  LPCALC_SHEET_CELLCOLLISTENERLIST lpThis;
  BOOL bRet = FALSE;

  if (hThis && (lpThis = Sheet_CellColListenerList_MemLock(hThis)))
  {
    bRet = lCol1 == lpThis->lCol1 && lRow1 == lpThis->lRow1 &&
           lCol2 == lpThis->lCol2 && lRow2 == lpThis->lRow2 &&
           hCol == lpThis->hCol;
    Sheet_CellColListenerList_MemUnlock(hThis);
  }
  return bRet;
}

//--------------------------------------------------------------------

BOOL Sheet_CellColListenerList_Contains(CALC_HANDLE hThis, long lCol, long lRow)
{
  LPCALC_SHEET_CELLCOLLISTENERLIST lpThis;
  BOOL bRet = FALSE;

  if (hThis && (lpThis = Sheet_CellColListenerList_MemLock(hThis)))
  {
    bRet = (CALC_ALLCOLS == lCol || (lpThis->lCol1 <= lCol && lCol <= lpThis->lCol2)) &&
           (CALC_ALLROWS == lRow || (lpThis->lRow1 <= lRow && lRow <= lpThis->lRow2));
    Sheet_CellColListenerList_MemUnlock(hThis);
  }
  return bRet;
}

//--------------------------------------------------------------------

CALC_HANDLE Sheet_CellColListenerList_GetCol(CALC_HANDLE hThis)
{
  LPCALC_SHEET_CELLCOLLISTENERLIST lpThis;
  CALC_HANDLE hValue = NULL;

  if (hThis && (lpThis = Sheet_CellColListenerList_MemLock(hThis)))
  {
    hValue = lpThis->hCol;
    Sheet_CellColListenerList_MemUnlock(hThis);
  }
  return hValue;
}

//--------------------------------------------------------------------

CALC_HANDLE Sheet_CellColListenerList_GetNext(CALC_HANDLE hThis)
{
  LPCALC_SHEET_CELLCOLLISTENERLIST lpThis;
  CALC_HANDLE hValue = NULL;

  if (hThis && (lpThis = Sheet_CellColListenerList_MemLock(hThis)))
  {
    hValue = lpThis->hNext;
    Sheet_CellColListenerList_MemUnlock(hThis);
  }
  return hValue;
}

//--------------------------------------------------------------------

void Sheet_CellColListenerList_SetNext(CALC_HANDLE hThis, CALC_HANDLE hValue)
{
  LPCALC_SHEET_CELLCOLLISTENERLIST lpThis;

  if (hThis && (lpThis = Sheet_CellColListenerList_MemLock(hThis)))
  {
    lpThis->hNext = hValue;
    Sheet_CellColListenerList_MemUnlock(hThis);
  }
}

//--------------------------------------------------------------------

typedef struct tagCALC_SHEET_ROWROWLISTENERLIST
{
  long lRow1;
  long lRow2;
  CALC_HANDLE hRow;
  long lColOffset1;
  long lColOffset2;
  CALC_HANDLE hNext;
} CALC_SHEET_ROWROWLISTENERLIST, FAR* LPCALC_SHEET_ROWROWLISTENERLIST;

//--------------------------------------------------------------------

#define Sheet_RowRowListenerList_MemAlloc() (CALC_HANDLE)CalcMemAlloc(sizeof(CALC_SHEET_ROWROWLISTENERLIST))
#define Sheet_RowRowListenerList_MemFree(hMem) CalcMemFree(hMem)
#define Sheet_RowRowListenerList_MemLock(hMem) (LPCALC_SHEET_ROWROWLISTENERLIST)CalcMemLock(hMem)
#define Sheet_RowRowListenerList_MemUnlock(hMem) CalcMemUnlock(hMem)

//--------------------------------------------------------------------

CALC_HANDLE Sheet_RowRowListenerList_Create(long lRow1, long lRow2, CALC_HANDLE hRow, long lColOffset1, long lColOffset2, CALC_HANDLE hNext)
{
  LPCALC_SHEET_ROWROWLISTENERLIST lpThis;
  CALC_HANDLE hThis;

  hThis = Sheet_RowRowListenerList_MemAlloc();
  if (hThis && (lpThis = Sheet_RowRowListenerList_MemLock(hThis)))
  {
    lpThis->lRow1 = lRow1;
    lpThis->lRow2 = lRow2;
    lpThis->hRow = hRow;
    lpThis->lColOffset1 = lColOffset1;
    lpThis->lColOffset2 = lColOffset2;
    lpThis->hNext = hNext;
    Sheet_RowRowListenerList_MemUnlock(hThis);
  }
  return hThis;
}

//--------------------------------------------------------------------

void Sheet_RowRowListenerList_Destroy(CALC_HANDLE hThis)
{
  LPCALC_SHEET_ROWROWLISTENERLIST lpThis;

  if (hThis && (lpThis = Sheet_RowRowListenerList_MemLock(hThis)))
  {
    Sheet_RowRowListenerList_MemUnlock(hThis);
    Sheet_RowRowListenerList_MemFree(hThis);
  }
}

//--------------------------------------------------------------------

BOOL Sheet_RowRowListenerList_Equals(CALC_HANDLE hThis, long lRow1, long lRow2, CALC_HANDLE hRow, long lColOffset1, long lColOffset2)
{
  LPCALC_SHEET_ROWROWLISTENERLIST lpThis;
  BOOL bRet = FALSE;

  if (hThis && (lpThis = Sheet_RowRowListenerList_MemLock(hThis)))
  {
    bRet = lRow1 == lpThis->lRow1 && lRow2 == lpThis->lRow2 && hRow == lpThis->hRow &&
           lColOffset1 == lpThis->lColOffset1 && lColOffset2 == lpThis->lColOffset2;
    Sheet_RowRowListenerList_MemUnlock(hThis);
  }
  return bRet;
}

//--------------------------------------------------------------------

BOOL Sheet_RowRowListenerList_Contains(CALC_HANDLE hThis, long lRow)
{
  LPCALC_SHEET_ROWROWLISTENERLIST lpThis;
  BOOL bRet = FALSE;

  if (hThis && (lpThis = Sheet_RowRowListenerList_MemLock(hThis)))
  {
    bRet = (CALC_ALLROWS == lRow || (lpThis->lRow1 <= lRow && lRow <= lpThis->lRow2));
    Sheet_RowRowListenerList_MemUnlock(hThis);
  }
  return bRet;
}

//--------------------------------------------------------------------

CALC_HANDLE Sheet_RowRowListenerList_GetRow(CALC_HANDLE hThis)
{
  LPCALC_SHEET_ROWROWLISTENERLIST lpThis;
  CALC_HANDLE hValue = NULL;

  if (hThis && (lpThis = Sheet_RowRowListenerList_MemLock(hThis)))
  {
    hValue = lpThis->hRow;
    Sheet_RowRowListenerList_MemUnlock(hThis);
  }
  return hValue;
}

//--------------------------------------------------------------------

long Sheet_RowRowListenerList_GetColOffset1(CALC_HANDLE hThis)
{
  LPCALC_SHEET_ROWROWLISTENERLIST lpThis;
  long lValue = 0;

  if (hThis && (lpThis = Sheet_RowRowListenerList_MemLock(hThis)))
  {
    lValue = lpThis->lColOffset1;
    Sheet_RowRowListenerList_MemUnlock(hThis);
  }
  return lValue;
}

//--------------------------------------------------------------------

long Sheet_RowRowListenerList_GetColOffset2(CALC_HANDLE hThis)
{
  LPCALC_SHEET_ROWROWLISTENERLIST lpThis;
  long lValue = 0;

  if (hThis && (lpThis = Sheet_RowRowListenerList_MemLock(hThis)))
  {
    lValue = lpThis->lColOffset2;
    Sheet_RowRowListenerList_MemUnlock(hThis);
  }
  return lValue;
}

//--------------------------------------------------------------------

CALC_HANDLE Sheet_RowRowListenerList_GetNext(CALC_HANDLE hThis)
{
  LPCALC_SHEET_ROWROWLISTENERLIST lpThis;
  CALC_HANDLE hValue = NULL;

  if (hThis && (lpThis = Sheet_RowRowListenerList_MemLock(hThis)))
  {
    hValue = lpThis->hNext;
    Sheet_RowRowListenerList_MemUnlock(hThis);
  }
  return hValue;
}

//--------------------------------------------------------------------

void Sheet_RowRowListenerList_SetNext(CALC_HANDLE hThis, CALC_HANDLE hValue)
{
  LPCALC_SHEET_ROWROWLISTENERLIST lpThis;

  if (hThis && (lpThis = Sheet_RowRowListenerList_MemLock(hThis)))
  {
    lpThis->hNext = hValue;
    Sheet_RowRowListenerList_MemUnlock(hThis);
  }
}

//--------------------------------------------------------------------

typedef struct tagCALC_SHEET_COLCOLLISTENERLIST
{
  long lCol1;
  long lCol2;
  CALC_HANDLE hCol;
  long lRowOffset1;
  long lRowOffset2;
  CALC_HANDLE hNext;
} CALC_SHEET_COLCOLLISTENERLIST, FAR* LPCALC_SHEET_COLCOLLISTENERLIST;

//--------------------------------------------------------------------

#define Sheet_ColColListenerList_MemAlloc() (CALC_HANDLE)CalcMemAlloc(sizeof(CALC_SHEET_COLCOLLISTENERLIST))
#define Sheet_ColColListenerList_MemFree(hMem) CalcMemFree(hMem)
#define Sheet_ColColListenerList_MemLock(hMem) (LPCALC_SHEET_COLCOLLISTENERLIST)CalcMemLock(hMem)
#define Sheet_ColColListenerList_MemUnlock(hMem) CalcMemUnlock(hMem)

//--------------------------------------------------------------------

CALC_HANDLE Sheet_ColColListenerList_Create(long lCol1, long lCol2, CALC_HANDLE hCol, long lRowOffset1, long lRowOffset2, CALC_HANDLE hNext)
{
  LPCALC_SHEET_COLCOLLISTENERLIST lpThis;
  CALC_HANDLE hThis;

  hThis = Sheet_ColColListenerList_MemAlloc();
  if (hThis && (lpThis = Sheet_ColColListenerList_MemLock(hThis)))
  {
    lpThis->lCol1 = lCol1;
    lpThis->lCol2 = lCol2;
    lpThis->hCol = hCol;
    lpThis->hNext = hNext;
    lpThis->lRowOffset1 = lRowOffset1;
    lpThis->lRowOffset2 = lRowOffset2;
    Sheet_ColColListenerList_MemUnlock(hThis);
  }
  return hThis;
}

//--------------------------------------------------------------------

void Sheet_ColColListenerList_Destroy(CALC_HANDLE hThis)
{
  LPCALC_SHEET_COLCOLLISTENERLIST lpThis;

  if (hThis && (lpThis = Sheet_ColColListenerList_MemLock(hThis)))
  {
    Sheet_ColColListenerList_MemUnlock(hThis);
    Sheet_ColColListenerList_MemFree(hThis);
  }
}

//--------------------------------------------------------------------

BOOL Sheet_ColColListenerList_Equals(CALC_HANDLE hThis, long lCol1, long lCol2, CALC_HANDLE hCol, long lRowOffset1, long lRowOffset2)
{
  LPCALC_SHEET_COLCOLLISTENERLIST lpThis;
  BOOL bRet = FALSE;

  if (hThis && (lpThis = Sheet_ColColListenerList_MemLock(hThis)))
  {
    bRet = lCol1 == lpThis->lCol1 && lCol2 == lpThis->lCol2 && hCol == lpThis->hCol &&
           lRowOffset1 == lpThis->lRowOffset1 && lRowOffset2 == lpThis->lRowOffset2;
    Sheet_ColColListenerList_MemUnlock(hThis);
  }
  return bRet;
}

//--------------------------------------------------------------------

BOOL Sheet_ColColListenerList_Contains(CALC_HANDLE hThis, long lCol)
{
  LPCALC_SHEET_COLCOLLISTENERLIST lpThis;
  BOOL bRet = FALSE;

  if (hThis && (lpThis = Sheet_ColColListenerList_MemLock(hThis)))
  {
    bRet = (CALC_ALLCOLS == lCol || (lpThis->lCol1 <= lCol && lCol <= lpThis->lCol2));
    Sheet_ColColListenerList_MemUnlock(hThis);
  }
  return bRet;
}

//--------------------------------------------------------------------

CALC_HANDLE Sheet_ColColListenerList_GetCol(CALC_HANDLE hThis)
{
  LPCALC_SHEET_COLCOLLISTENERLIST lpThis;
  CALC_HANDLE hValue = NULL;

  if (hThis && (lpThis = Sheet_ColColListenerList_MemLock(hThis)))
  {
    hValue = lpThis->hCol;
    Sheet_ColColListenerList_MemUnlock(hThis);
  }
  return hValue;
}

//--------------------------------------------------------------------

long Sheet_ColColListenerList_GetRowOffset1(CALC_HANDLE hThis)
{
  LPCALC_SHEET_COLCOLLISTENERLIST lpThis;
  long lValue = 0;

  if (hThis && (lpThis = Sheet_ColColListenerList_MemLock(hThis)))
  {
    lValue = lpThis->lRowOffset1;
    Sheet_ColColListenerList_MemUnlock(hThis);
  }
  return lValue;
}

//--------------------------------------------------------------------

long Sheet_ColColListenerList_GetRowOffset2(CALC_HANDLE hThis)
{
  LPCALC_SHEET_COLCOLLISTENERLIST lpThis;
  long lValue = 0;

  if (hThis && (lpThis = Sheet_ColColListenerList_MemLock(hThis)))
  {
    lValue = lpThis->lRowOffset2;
    Sheet_ColColListenerList_MemUnlock(hThis);
  }
  return lValue;
}

//--------------------------------------------------------------------

CALC_HANDLE Sheet_ColColListenerList_GetNext(CALC_HANDLE hThis)
{
  LPCALC_SHEET_COLCOLLISTENERLIST lpThis;
  CALC_HANDLE hValue = NULL;

  if (hThis && (lpThis = Sheet_ColColListenerList_MemLock(hThis)))
  {
    hValue = lpThis->hNext;
    Sheet_ColColListenerList_MemUnlock(hThis);
  }
  return hValue;
}

//--------------------------------------------------------------------

void Sheet_ColColListenerList_SetNext(CALC_HANDLE hThis, CALC_HANDLE hValue)
{
  LPCALC_SHEET_COLCOLLISTENERLIST lpThis;

  if (hThis && (lpThis = Sheet_ColColListenerList_MemLock(hThis)))
  {
    lpThis->hNext = hValue;
    Sheet_ColColListenerList_MemUnlock(hThis);
  }
}

//--------------------------------------------------------------------

void Sheet_CellAddCellListener(LPCALC_SHEET lpThis, long lCol1, long lRow1, long lCol2, long lRow2, CALC_HANDLE hCell)
{
  CALC_HANDLE hList;

  if (lpThis)
  {
    for (hList = lpThis->hCellCellListeners; hList; hList = Sheet_CellCellListenerList_GetNext(hList))
    {
      if (Sheet_CellCellListenerList_Equals(hList, lCol1, lRow1, lCol2, lRow2, hCell))
        break;
    }
    if (!hList)
      lpThis->hCellCellListeners = Sheet_CellCellListenerList_Create(lCol1, lRow1, lCol2, lRow2, hCell, lpThis->hCellCellListeners);
  }
}

//--------------------------------------------------------------------

void Sheet_CellRemoveCellListener(LPCALC_SHEET lpThis, long lCol1, long lRow1, long lCol2, long lRow2, CALC_HANDLE hCell)
{
  CALC_HANDLE hList;
  CALC_HANDLE hPrev;

  if (lpThis)
  {
    for (hPrev = NULL, hList = lpThis->hCellCellListeners; hList; hPrev = hList, hList = Sheet_CellCellListenerList_GetNext(hList))
    {
      if (Sheet_CellCellListenerList_Equals(hList, lCol1, lRow1, lCol2, lRow2, hCell))
        break;
    }
    if (hList)
    {
      if (hPrev)
        Sheet_CellCellListenerList_SetNext(hPrev, Sheet_CellCellListenerList_GetNext(hList));
      else
        lpThis->hCellCellListeners = Sheet_CellCellListenerList_GetNext(hList);
      Sheet_CellCellListenerList_Destroy(hList);
    }
  }
}

//--------------------------------------------------------------------

void Sheet_CellAddRowListener(LPCALC_SHEET lpThis, long lCol1, long lRow1, long lCol2, long lRow2, CALC_HANDLE hRow)
{
  CALC_HANDLE hList;

  if (lpThis)
  {
    for (hList = lpThis->hCellRowListeners; hList; hList = Sheet_CellRowListenerList_GetNext(hList))
    {
      if (Sheet_CellRowListenerList_Equals(hList, lCol1, lRow1, lCol2, lRow2, hRow))
        break;
    }
    if (!hList)
      lpThis->hCellRowListeners = Sheet_CellRowListenerList_Create(lCol1, lRow1, lCol2, lRow2, hRow, lpThis->hCellRowListeners);
  }
}

//--------------------------------------------------------------------

void Sheet_CellRemoveRowListener(LPCALC_SHEET lpThis, long lCol1, long lRow1, long lCol2, long lRow2, CALC_HANDLE hRow)
{
  CALC_HANDLE hList;
  CALC_HANDLE hPrev;

  if (lpThis)
  {
    for (hPrev = NULL, hList = lpThis->hCellRowListeners; hList; hPrev = hList, hList = Sheet_CellRowListenerList_GetNext(hList))
    {
      if (Sheet_CellRowListenerList_Equals(hList, lCol1, lRow1, lCol2, lRow2, hRow))
        break;
    }
    if (hList)
    {
      if (hPrev)
        Sheet_CellRowListenerList_SetNext(hPrev, Sheet_CellRowListenerList_GetNext(hList));
      else
        lpThis->hCellRowListeners = Sheet_CellRowListenerList_GetNext(hList);
      Sheet_CellRowListenerList_Destroy(hList);
    }
  }
}

//--------------------------------------------------------------------

void Sheet_CellAddColListener(LPCALC_SHEET lpThis, long lCol1, long lRow1, long lCol2, long lRow2, CALC_HANDLE hCol)
{
  CALC_HANDLE hList;

  if (lpThis)
  {
    for (hList = lpThis->hCellColListeners; hList; hList = Sheet_CellColListenerList_GetNext(hList))
    {
      if (Sheet_CellColListenerList_Equals(hList, lCol1, lRow1, lCol2, lRow2, hCol))
        break;
    }
    if (!hList)
      lpThis->hCellColListeners = Sheet_CellColListenerList_Create(lCol1, lRow1, lCol2, lRow2, hCol, lpThis->hCellColListeners);
  }
}

//--------------------------------------------------------------------

void Sheet_CellRemoveColListener(LPCALC_SHEET lpThis, long lCol1, long lRow1, long lCol2, long lRow2, CALC_HANDLE hCol)
{
  CALC_HANDLE hList;
  CALC_HANDLE hPrev;

  if (lpThis)
  {
    for (hPrev = NULL, hList = lpThis->hCellColListeners; hList; hPrev = hList, hList = Sheet_CellColListenerList_GetNext(hList))
    {
      if (Sheet_CellColListenerList_Equals(hList, lCol1, lRow1, lCol2, lRow2, hCol))
        break;
    }
    if (hList)
    {
      if (hPrev)
        Sheet_CellColListenerList_SetNext(hPrev, Sheet_CellColListenerList_GetNext(hList));
      else
        lpThis->hCellColListeners = Sheet_CellColListenerList_GetNext(hList);
      Sheet_CellColListenerList_Destroy(hList);
    }
  }
}

//--------------------------------------------------------------------

void Sheet_RowAddRowListener(LPCALC_SHEET lpThis, long lRow1, long lRow2, CALC_HANDLE hRow, long lColOffset1, long lColOffset2)
{
  CALC_HANDLE hList;

  if (lpThis)
  {
    for (hList = lpThis->hRowRowListeners; hList; hList = Sheet_RowRowListenerList_GetNext(hList))
    {
      if (Sheet_RowRowListenerList_Equals(hList, lRow1, lRow2, hRow, lColOffset1, lColOffset2))
        break;
    }
    if (!hList)
      lpThis->hRowRowListeners = Sheet_RowRowListenerList_Create(lRow1, lRow2, hRow, lColOffset1, lColOffset2, lpThis->hRowRowListeners);
  }
}

//--------------------------------------------------------------------

void Sheet_RowRemoveRowListener(LPCALC_SHEET lpThis, long lRow1, long lRow2, CALC_HANDLE hRow, long lColOffset1, long lColOffset2)
{
  CALC_HANDLE hList;
  CALC_HANDLE hPrev;

  if (lpThis)
  {
    for (hPrev = NULL, hList = lpThis->hRowRowListeners; hList; hPrev = hList, hList = Sheet_RowRowListenerList_GetNext(hList))
    {
      if (Sheet_RowRowListenerList_Equals(hList, lRow1, lRow2, hRow, lColOffset1, lColOffset2))
        break;
    }
    if (hList)
    {
      if (hPrev)
        Sheet_RowRowListenerList_SetNext(hPrev, Sheet_RowRowListenerList_GetNext(hList));
      else
        lpThis->hRowRowListeners = Sheet_RowRowListenerList_GetNext(hList);
      Sheet_RowRowListenerList_Destroy(hList);
    }
  }
}

//--------------------------------------------------------------------

void Sheet_ColAddColListener(LPCALC_SHEET lpThis, long lCol1, long lCol2, CALC_HANDLE hCol, long lRowOffset1, long lRowOffset2)
{
  CALC_HANDLE hList;

  if (lpThis)
  {
    for (hList = lpThis->hColColListeners; hList; hList = Sheet_ColColListenerList_GetNext(hList))
    {
      if (Sheet_ColColListenerList_Equals(hList, lCol1, lCol2, hCol, lRowOffset1, lRowOffset2))
        break;
    }
    if (!hList)
      lpThis->hColColListeners = Sheet_ColColListenerList_Create(lCol1, lCol2, hCol, lRowOffset1, lRowOffset2, lpThis->hColColListeners);
  }
}

//--------------------------------------------------------------------

void Sheet_ColRemoveColListener(LPCALC_SHEET lpThis, long lCol1, long lCol2, CALC_HANDLE hCol, long lRowOffset1, long lRowOffset2)
{
  CALC_HANDLE hList;
  CALC_HANDLE hPrev;

  if (lpThis)
  {
    for (hPrev = NULL, hList = lpThis->hColColListeners; hList; hPrev = hList, hList = Sheet_ColColListenerList_GetNext(hList))
    {
      if (Sheet_ColColListenerList_Equals(hList, lCol1, lCol2, hCol, lRowOffset1, lRowOffset2))
        break;
    }
    if (hList)
    {
      if (hPrev)
        Sheet_ColColListenerList_SetNext(hPrev, Sheet_ColColListenerList_GetNext(hList));
      else
        lpThis->hColColListeners = Sheet_ColColListenerList_GetNext(hList);
      Sheet_ColColListenerList_Destroy(hList);
    }
  }
}

//--------------------------------------------------------------------

void Sheet_CellAddListenersToAdjust(LPCALC_SHEET lpThis, long lCol, long lRow)
{
  CALC_HANDLE hList;

  if (lpThis)
  {
    for (hList = lpThis->hCellCellListeners; hList != NULL; hList = Sheet_CellCellListenerList_GetNext(hList))
    {
      if (Sheet_CellCellListenerList_Contains(hList, lCol, lRow))
        Book_AddAdjustCell(lpThis->lpBook, Sheet_CellCellListenerList_GetCell(hList));
    }
    for (hList = lpThis->hCellRowListeners; hList != NULL; hList = Sheet_CellRowListenerList_GetNext(hList))
    {
      if (Sheet_CellRowListenerList_Contains(hList, lCol, lRow))
        Book_AddAdjustRow(lpThis->lpBook, Sheet_CellRowListenerList_GetRow(hList));
    }
    for (hList = lpThis->hCellColListeners; hList != NULL; hList = Sheet_CellColListenerList_GetNext(hList))
    {
      if (Sheet_CellColListenerList_Contains(hList, lCol, lRow))
        Book_AddAdjustCol(lpThis->lpBook, Sheet_CellColListenerList_GetCol(hList));
    }
  }
}

//--------------------------------------------------------------------

void Sheet_RowAddListenersToAdjust(LPCALC_SHEET lpThis, long lRow)
{
  CALC_HANDLE hList;

  if (lpThis)
  {
    for (hList = lpThis->hRowRowListeners; hList != NULL; hList = Sheet_RowRowListenerList_GetNext(hList))
    {
      if (Sheet_RowRowListenerList_Contains(hList, lRow))
        Book_AddAdjustRow(lpThis->lpBook, Sheet_RowRowListenerList_GetRow(hList));
    }
  }
}

//--------------------------------------------------------------------

void Sheet_ColAddListenersToAdjust(LPCALC_SHEET lpThis, long lCol)
{
  CALC_HANDLE hList;

  if (lpThis)
  {
    for (hList = lpThis->hColColListeners; hList != NULL; hList = Sheet_ColColListenerList_GetNext(hList))
    {
      if (Sheet_ColColListenerList_Contains(hList, lCol))
        Book_AddAdjustCol(lpThis->lpBook, Sheet_ColColListenerList_GetCol(hList));
    }
  }
}

//--------------------------------------------------------------------

void Sheet_AddListenersToDirty(LPCALC_SHEET lpThis, long lCol, long lRow)
{
  CALC_HANDLE hList;

  if (lpThis)
  {
    for (hList = lpThis->hCellCellListeners; hList != NULL; hList = Sheet_CellCellListenerList_GetNext(hList))
    {
      if (Sheet_CellCellListenerList_Contains(hList, lCol, lRow))
        Book_AddDirtyCell(lpThis->lpBook, Sheet_CellCellListenerList_GetCell(hList));
    }
    for (hList = lpThis->hCellRowListeners; hList != NULL; hList = Sheet_CellRowListenerList_GetNext(hList))
    {
      if (Sheet_CellRowListenerList_Contains(hList, lCol, lRow))
        Book_AddDirtyRow(lpThis->lpBook, Sheet_CellRowListenerList_GetRow(hList));
    }
    for (hList = lpThis->hCellColListeners; hList != NULL; hList = Sheet_CellColListenerList_GetNext(hList))
    {
      if (Sheet_CellColListenerList_Contains(hList, lCol, lRow))
        Book_AddDirtyCol(lpThis->lpBook, Sheet_CellColListenerList_GetCol(hList));
    }
    for (hList = lpThis->hRowRowListeners; hList != NULL; hList = Sheet_RowRowListenerList_GetNext(hList))
    {
      if (Sheet_RowRowListenerList_Contains(hList, lRow))
      {
        CALC_HANDLE hRow = Sheet_RowRowListenerList_GetRow(hList);
        long lColOffset1 = Sheet_RowRowListenerList_GetColOffset1(hList);
        long lColOffset2 = Sheet_RowRowListenerList_GetColOffset2(hList);
        long lColOffset;
        for (lColOffset = lColOffset1; lColOffset <= lColOffset2; lColOffset++)
        {
          if (lCol + lColOffset > 0)
          {
            CALC_HANDLE hCell = CellLookupAlloc(Row_GetSheet(hRow), lCol + lColOffset, Row_GetRow(hRow));
            Book_AddDirtyCell(lpThis->lpBook, hCell);
          }
        }
      }
    }
    for (hList = lpThis->hColColListeners; hList != NULL; hList = Sheet_ColColListenerList_GetNext(hList))
    {
      if (Sheet_ColColListenerList_Contains(hList, lCol))
      {
        CALC_HANDLE hCol = Sheet_ColColListenerList_GetCol(hList);
        long lRowOffset1 = Sheet_ColColListenerList_GetRowOffset1(hList);
        long lRowOffset2 = Sheet_ColColListenerList_GetRowOffset2(hList);
        long lRowOffset;
        for (lRowOffset = lRowOffset1; lRowOffset <= lRowOffset2; lRowOffset++)
        {
          if (lRow + lRowOffset > 0)
          {
            CALC_HANDLE hCell = CellLookupAlloc(Col_GetSheet(hCol), Col_GetCol(hCol), lRow + lRowOffset);
            Book_AddDirtyCell(lpThis->lpBook, hCell);
          }
        }
      }
    }
  }
}

//--------------------------------------------------------------------

void Sheet_IncrementDelayOfListeners(LPCALC_SHEET lpThis, long lCol, long lRow)
{
  CALC_HANDLE hList;

  if (lpThis)
  {
    for (hList = lpThis->hCellCellListeners; hList != NULL; hList = Sheet_CellCellListenerList_GetNext(hList))
    {
      if (Sheet_CellCellListenerList_Contains(hList, lCol, lRow))
      {
        CALC_HANDLE hCell = Sheet_CellCellListenerList_GetCell(hList);
        Cell_SetDelay(hCell, Cell_GetDelay(hCell) + 1);
      }
    }
    for (hList = lpThis->hCellRowListeners; hList != NULL; hList = Sheet_CellRowListenerList_GetNext(hList))
    {
      if (Sheet_CellRowListenerList_Contains(hList, lCol, lRow))
      {
        CALC_HANDLE hRow = Sheet_CellRowListenerList_GetRow(hList);
        Sheet_IncrementDelay(Row_GetSheet(hRow), CALC_ALLCOLS, Row_GetRow(hRow));
      }
    }
    for (hList = lpThis->hCellColListeners; hList != NULL; hList = Sheet_CellColListenerList_GetNext(hList))
    {
      if (Sheet_CellColListenerList_Contains(hList, lCol, lRow))
      {
        CALC_HANDLE hCol = Sheet_CellColListenerList_GetCol(hList);
        Sheet_IncrementDelay(Col_GetSheet(hCol), Col_GetCol(hCol), CALC_ALLROWS);
      }
    }
    for (hList = lpThis->hRowRowListeners; hList != NULL; hList = Sheet_RowRowListenerList_GetNext(hList))
    {
      if (Sheet_RowRowListenerList_Contains(hList, lRow))
      {
        CALC_HANDLE hRow = Sheet_RowRowListenerList_GetRow(hList);
        long lColOffset1 = Sheet_RowRowListenerList_GetColOffset1(hList);
        long lColOffset2 = Sheet_RowRowListenerList_GetColOffset2(hList);
        long lColOffset;
        for (lColOffset = lColOffset1; lColOffset <= lColOffset2; lColOffset++)
        {
          if (lCol + lColOffset > 0)
          {
            CALC_HANDLE hCell = CellLookupAlloc(Row_GetSheet(hRow), lCol + lColOffset, Row_GetRow(hRow));
            Cell_SetDelay(hCell, Cell_GetDelay(hCell) + 1);
          }
        }
      }
    }
    for (hList = lpThis->hColColListeners; hList != NULL; hList = Sheet_ColColListenerList_GetNext(hList))
    {
      if (Sheet_ColColListenerList_Contains(hList, lCol))
      {
        CALC_HANDLE hCol = Sheet_ColColListenerList_GetCol(hList);
        long lRowOffset1 = Sheet_ColColListenerList_GetRowOffset1(hList);
        long lRowOffset2 = Sheet_ColColListenerList_GetRowOffset2(hList);
        long lRowOffset;
        for (lRowOffset = lRowOffset1; lRowOffset <= lRowOffset2; lRowOffset++)
        {
          if (lRow + lRowOffset > 0)
          {
            CALC_HANDLE hCell = CellLookupAlloc(Col_GetSheet(hCol), Col_GetCol(hCol), lRow + lRowOffset);
            Cell_SetDelay(hCell, Cell_GetDelay(hCell) + 1);
          }
        }
      }
    }
  }
}

//--------------------------------------------------------------------

void Sheet_DecrementDelayOfListeners(LPCALC_SHEET lpThis, long lCol, long lRow)
{
  CALC_HANDLE hList;

  if (lpThis)
  {
    for (hList = lpThis->hCellCellListeners; hList != NULL; hList = Sheet_CellCellListenerList_GetNext(hList))
    {
      if (Sheet_CellCellListenerList_Contains(hList, lCol, lRow))
      {
        CALC_HANDLE hCell = Sheet_CellCellListenerList_GetCell(hList);
        Cell_SetDelay(hCell, Cell_GetDelay(hCell) - 1);
      }
    }
    for (hList = lpThis->hCellRowListeners; hList != NULL; hList = Sheet_CellRowListenerList_GetNext(hList))
    {
      if (Sheet_CellRowListenerList_Contains(hList, lCol, lRow))
      {
        CALC_HANDLE hRow = Sheet_CellRowListenerList_GetRow(hList);
        Sheet_DecrementDelay(Row_GetSheet(hRow), CALC_ALLCOLS, Row_GetRow(hRow));
      }
    }
    for (hList = lpThis->hCellColListeners; hList != NULL; hList = Sheet_CellColListenerList_GetNext(hList))
    {
      if (Sheet_CellColListenerList_Contains(hList, lCol, lRow))
      {
        CALC_HANDLE hCol = Sheet_CellColListenerList_GetCol(hList);
        Sheet_DecrementDelay(Col_GetSheet(hCol), Col_GetCol(hCol), CALC_ALLROWS);
      }
    }
    for (hList = lpThis->hRowRowListeners; hList != NULL; hList = Sheet_RowRowListenerList_GetNext(hList))
    {
      if (Sheet_RowRowListenerList_Contains(hList, lRow))
      {
        CALC_HANDLE hRow = Sheet_RowRowListenerList_GetRow(hList);
        long lColOffset1 = Sheet_RowRowListenerList_GetColOffset1(hList);
        long lColOffset2 = Sheet_RowRowListenerList_GetColOffset2(hList);
        long lColOffset;
        for (lColOffset = lColOffset1; lColOffset <= lColOffset2; lColOffset++)
        {
          if (lCol + lColOffset > 0)
          {
            CALC_HANDLE hCell = CellLookupAlloc(Row_GetSheet(hRow), lCol + lColOffset, Row_GetRow(hRow));
            Cell_SetDelay(hCell, Cell_GetDelay(hCell) - 1);
          }
        }
      }
    }
    for (hList = lpThis->hColColListeners; hList != NULL; hList = Sheet_ColColListenerList_GetNext(hList))
    {
      if (Sheet_ColColListenerList_Contains(hList, lCol))
      {
        CALC_HANDLE hCol = Sheet_ColColListenerList_GetCol(hList);
        long lRowOffset1 = Sheet_ColColListenerList_GetRowOffset1(hList);
        long lRowOffset2 = Sheet_ColColListenerList_GetRowOffset2(hList);
        long lRowOffset;
        for (lRowOffset = lRowOffset1; lRowOffset <= lRowOffset2; lRowOffset++)
        {
          if (lRow + lRowOffset > 0)
          {
            CALC_HANDLE hCell = CellLookupAlloc(Col_GetSheet(hCol), Col_GetCol(hCol), lRow + lRowOffset);
            Cell_SetDelay(hCell, Cell_GetDelay(hCell) - 1);
          }
        }
      }
    }
  }
}

//--------------------------------------------------------------------

#endif // defined(SS_V70)

#if SS_V80 // 24477 -scl
BOOL CALCAPI CalcIsSubtotal(LPCALC_SHEET lpSheet, long lCol, long lRow)
{
  CALC_HANDLE hExpr;

  hExpr = CellGetExpr(lpSheet, lCol, lRow);
  return ExprIsSubtotal(hExpr);
}
#endif

//--------------------------------------------------------------------
