//--------------------------------------------------------------------
//
//  File: cal_expr.h
//
//  Description: Expression evaluation routines
//

#ifndef CAL_EXPR_H
#define CAL_EXPR_H

#ifdef __cplusplus
extern "C" {
#endif

//--------------------------------------------------------------------
//
//  The following constants represent the various types of expression
//  elements.
//

#define CALC_EXPR_NULL       0
#define CALC_EXPR_LONG       1
#define CALC_EXPR_DOUBLE     2
#define CALC_EXPR_STRING     3
#define CALC_EXPR_CELL       4
#define CALC_EXPR_RANGE      5
#define CALC_EXPR_NAME       6
#define CALC_EXPR_FUNCTION   7
#define CALC_EXPR_CUSTFUNC   8
#define CALC_EXPR_OPERATOR   9
#define CALC_EXPR_SEPARATOR 10
#define CALC_EXPR_BEGIN     11
#define CALC_EXPR_END       12
#define CALC_EXPR_DDE       13
#define CALC_EXPR_ERROR     14
#define CALC_EXPR_EXTERNALCELL   15
#define CALC_EXPR_EXTERNALRANGE  16

//--------------------------------------------------------------------
//
//  The CALC_EXPRINFO structure represents an individual element in
//  an expression.  An expression is respesented as an array of
//  CALC_EXPRINFO structures.  The first element in the array contains
//  header information (including a reference count).  The last
//  element is an end of expression marker.  The remaining elements
//  are in a reverse polish notation.  The '(' and ')' separators are
//  included in the expression for unparsing purposes.
//
//  For example, the expression ...
//    (a + f(b)) ^ 2
//  would be stored as ...
//     HEADER ( a b f + ) 2 ^ END
//  The following is a more detailed diagram of the expression...
//
//     +------------------------------+
//     | nType = CALC_EXPR_BEGIN      |
//     | u.Header.lRefCnt = 1         |
//     +------------------------------+
//     | nType = CALC_EXPR_SEPARATOR  |
//     | u.Sep.nId = CALC_SEP_LPAR    |
//     +------------------------------+       +----------------------+
//     | nType = CALC_EXPR_NAME       |   /-->| named expression "a" |
//     | u.CustName.hName             |--/    +----------------------+
//     +------------------------------+
//     | nType = CALC_EXPR_NAME       |
//     | u.CustName.hName             |--\    +----------------------+
//     +------------------------------+   \-->| named expression "b" |
//     | nType = CALC_EXPR_CUSTFUNC   |       +----------------------+
//     | u.CustFunc.hFunc             |--\ 
//     | u.CustFunc.nArgs = 1         |   \ 
//     +------------------------------+    \    +---------------------+
//     | nType = CALC_EXPR_OPERATOR   |     \-->| custom function "f" |
//     | u.Oper.nId = CALC_OPER_ADD   |         +---------------------+
//     +------------------------------+
//     | nType = CALC_EXPR_SEPARATOR  |
//     | u.Sep.nId = CALC_SEP_RPAR    |
//     +------------------------------+
//     | nType = CALC_EXPR_LONG       |
//     | u.Long.lVal = 2              |
//     +------------------------------+
//     | nType = CALC_EXPR_OPERATOR   |
//     | u.Oper.nId = CALC_OPER_EXPON |
//     +------------------------------+
//     | nType = CALC_EXPR_END        |
//     +------------------------------+
//

typedef struct tagCALC_EXPRINFO
{
  int nType;
  union
  {
    struct { long lVal; } Long;
    struct { double dfVal; } Double;
    struct { CALC_HANDLE hText; } String;
    struct { long lCol, lRow; } Cell;
    struct { long lCol1, lRow1, lCol2, lRow2; } Range;
    struct { CALC_HANDLE hName; } CustName;
    struct { short nId, nArgs; } Func;
    struct { CALC_HANDLE hFunc; short nArgs; } CustFunc;
    struct { short nId; } Oper;
    struct { short nId; } Sep;
    struct { long lRefCnt; } Header;
    struct { CALC_HANDLE hDde; long lItemCol; long lItemRow; } Dde;
    struct { short nId; } Error;
    struct { LPCALC_SHEET lpSheet; long lCol, lRow; } ExternalCell;
    struct { LPCALC_SHEET lpSheet; long lCol1, lRow1, lCol2, lRow2; } ExternalRange;
  } u;
} CALC_EXPRINFO, FAR* LPCALC_EXPRINFO;

//--------------------------------------------------------------------

#define Expr_MemAlloc(nCnt) (CALC_HANDLE)CalcMemAlloc((nCnt)*sizeof(CALC_EXPRINFO))
#define Expr_MemFree(hMem) CalcMemFree(hMem)
#define Expr_MemLock(hMem) (LPCALC_EXPRINFO)CalcMemLock(hMem)
#define Expr_MemUnlock(hMem) CalcMemUnlock(hMem)

//--------------------------------------------------------------------
//
//  The following functions are used to handle an expression.
//

CALC_HANDLE ExprCreate(LPCALC_BOOK lpBook, long lCol, long lRow, LPCTSTR lpszText);
CALC_HANDLE ExprCreateFromDde(CALC_HANDLE hDde, long lItemCol, long lItemRow);
CALC_HANDLE ExprCreateRef(CALC_HANDLE hExpr);
CALC_HANDLE ExprCreateCopy(CALC_HANDLE hExpr);
BOOL ExprDestroy(CALC_HANDLE hExpr);
CALC_HANDLE ExprGetDde(CALC_HANDLE hExpr, LPLONG lplItemCol, LPLONG lplItemRow);
BOOL ExprContainsName(CALC_HANDLE hExpr, CALC_HANDLE hName);
BOOL ExprContainsAbsInRange(CALC_HANDLE hEpxr, long lBaseCol, long lBaseRow,
                            long lCol, long lRow, long lCol2, long lRow2);
BOOL ExprContainsMixedColRange(CALC_HANDLE hExpr);
BOOL ExprContainsMixedRowRange(CALC_HANDLE hExpr);
int  ExprGetText(CALC_HANDLE hExpr, long lCol, long lRow,
                 short nRefStyle, 
#if SS_V80 // 24919 -scl
                 BOOL bExcel,
#endif
                 LPTSTR lpszText, int nLen);
int  ExprGetTextLen(CALC_HANDLE hExpr, long lCol, long lRow, short nRefStyle
#if SS_V80 // 24919 -scl
                   , BOOL bExcel
#endif
                   );
BOOL ExprEval(CALC_HANDLE hExpr, LPCALC_SHEET lpSheet,
              long lCol, long lRow, LPCALC_VALUE lpResult);
BOOL ExprSetLinkToCell(CALC_HANDLE hExpr, CALC_HANDLE hDependCell, BOOL bSetNameLinks, BOOL bLink);
BOOL ExprSetLinkToCol(CALC_HANDLE hExpr, CALC_HANDLE hDependCol, BOOL bSetNameLinks, BOOL bLink);
BOOL ExprSetLinkToRow(CALC_HANDLE hExpr, CALC_HANDLE hDependRow, BOOL bSetNameLinks, BOOL bLink);
BOOL ExprSetLinkToName(CALC_HANDLE hExpr, CALC_HANDLE hName, BOOL bLink);

typedef struct tagCALC_ADJUST
{
  LPCALC_SHEET lpCurSheet;
  long lCurCol, lCurRow;
  LPCALC_SHEET lpSrcSheet;
  long lSrcCol1, lSrcRow1, lSrcCol2, lSrcRow2;
  LPCALC_SHEET lpDestSheet;
  long lDestCol1, lDestRow1, lDestCol2, lDestRow2;
  long lOffsetCol, lOffsetRow;
  BITMATRIX matCells;
  BITVECTOR vecCols;
  BITVECTOR vecRows;
} CALC_ADJUST, FAR* LPCALC_ADJUST;

CALC_HANDLE ExprIntModifyOnMove(CALC_HANDLE hExpr, LPCALC_ADJUST lpInfo);
CALC_HANDLE ExprExtModifyOnMove(CALC_HANDLE hExpr, LPCALC_ADJUST lpInfo);
CALC_HANDLE ExprIntModifyOnSwap(CALC_HANDLE hExpr, LPCALC_ADJUST lpInfo);
CALC_HANDLE ExprExtModifyOnSwap(CALC_HANDLE hExpr, LPCALC_ADJUST lpInfo);
CALC_HANDLE ExprIntModifyOnInsCols(CALC_HANDLE hExpr, LPCALC_ADJUST lpInfo);
CALC_HANDLE ExprExtModifyOnInsCols(CALC_HANDLE hExpr, LPCALC_ADJUST lpInfo);
CALC_HANDLE ExprIntModifyOnInsRows(CALC_HANDLE hExpr, LPCALC_ADJUST lpInfo);
CALC_HANDLE ExprExtModifyOnInsRows(CALC_HANDLE hExpr, LPCALC_ADJUST lpInfo);
CALC_HANDLE ExprIntModifyOnDelCols(CALC_HANDLE hExpr, LPCALC_ADJUST lpInfo);
CALC_HANDLE ExprExtModifyOnDelCols(CALC_HANDLE hExpr, LPCALC_ADJUST lpInfo);
CALC_HANDLE ExprIntModifyOnDelRows(CALC_HANDLE hExpr, LPCALC_ADJUST lpInfo);
CALC_HANDLE ExprExtModifyOnDelRows(CALC_HANDLE hExpr, LPCALC_ADJUST lpInfo);
CALC_HANDLE ExprExtModifyOnDelSheet(CALC_HANDLE hExpr, LPCALC_ADJUST lpInfo);

void Expr_CellStartListening(CALC_HANDLE hThis, CALC_HANDLE hCell, BOOL bIncludeNames);
void Expr_CellStopListening(CALC_HANDLE hThis, CALC_HANDLE hCell, BOOL bIncludeNames);
void Expr_RowStartListening(CALC_HANDLE hThis, CALC_HANDLE hRow, BOOL bIncludeNames);
void Expr_RowStopListening(CALC_HANDLE hThis, CALC_HANDLE hRow, BOOL bIncludeNames);
void Expr_ColStartListening(CALC_HANDLE hThis, CALC_HANDLE hCol, BOOL bIncludeNames);
void Expr_ColStopListening(CALC_HANDLE hThis, CALC_HANDLE hCol, BOOL bIncludeNames);
void Expr_NameStartListening(CALC_HANDLE hThis, CALC_HANDLE hName);
void Expr_NameStopListening(CALC_HANDLE hThis, CALC_HANDLE hName);

#if SS_V80 // 24477 -scl
BOOL ExprIsSubtotal(CALC_HANDLE hExpr);
#endif
//--------------------------------------------------------------------

#ifdef __cplusplus
}
#endif

#endif
