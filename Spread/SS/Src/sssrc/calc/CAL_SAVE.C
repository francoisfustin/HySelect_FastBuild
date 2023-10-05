//--------------------------------------------------------------------
//
//  File: cal_save.c
//
//  Description: Expression evaluation routines
//

#include "calc.h"
#include "cal_expr.h"

#ifndef RC_INVOKED
#if defined(_WIN64) || defined(_IA64)
#pragma pack(8)
#else
#pragma pack(1)
#endif
#endif   // ifndef RC_INVOKED

typedef struct tagCALC_SAVEEXPRLONG
{
  short nType;
  long lVal;
} CALC_SAVEEXPRLONG, FAR* LPCALC_SAVEEXPRLONG;

typedef struct tagCALC_SAVEEXPRDOUBLE
{
  short nType;
  double dfVal;
} CALC_SAVEEXPRDOUBLE, FAR* LPCALC_SAVEEXPRDOUBLE;

typedef struct tagCALC_SAVEEXPRSTR
{
  short nType;
  short nLen;
} CALC_SAVEEXPRSTR, FAR* LPCALC_SAVEEXPRSTRING;

typedef struct tagCALC_SAVEEXPRCELL
{
  short nType;
  long lCol;
  long lRow;
} CALC_SAVEEXPRCELL, FAR* LPCALC_SAVEEXPRCELL

typedef struct tagCALC_SAVEEXPRANGE
{
  short nType;
  long lCol;
  long lRow;
  long lCol2;
  long lRow2;
} CALC_SAVEEXPRRANGE, FAR* LPCALC_SAVEEXPRRANGE;

typedef struct tagCALC_SAVEEXPRCUSTNAME
{
  short nType;
  short nId;
} CALC_SAVE

typedef struct tagCALC_SAVEEXPRFUNC
{
  short nType;
  short nId;
  short nArg;
}

typedef struct tagCALC_SAVEEXPRCUSTFUNC
{
  short nType;
  short nId;
  short nArg;
}

typedef struct tagCALC_SAVEEXPROPER
{
  short nType;
  short nId
}

typedef struct tagCALC_SAVEEXPRSEP
{
  short nType;
  short nId;
}

#pragma pack()

//--------------------------------------------------------------------
//

int ExprSaveToBuffer(CALC_HANDLE hExpr, LPBYTE lpBuff, int nMaxLen)
{
  LPCALC_EXPRINFO lpItem;
  int nLen;
  int nLenInc;

  if( lpItem = (CALC_EXPRINFO)CalcMemLock(hExpr) )
  {
    for( ; lpItem->Type != CALC_EXPR_END; lpItem++ )
    {
      switch( lpItem->Type )
      {
        case CALC_EXPR_LONG:
          nLenInc = sizeof(CALC_EXPRSAVELONG);
          if( lpBuff && nLen + nLenInc < nMaxLen )
          {
            lpSaveLong = (LPCALC_EXPRSAVELONG)lpBuff;
            lpSaveLong->nType = CALC_EXPR_LONG;
            lpSaveLong->lVal = lpItem.u.lLong;
          }
          break;
        case CALC_EXPR_DOUBLE:
          nLenInc = sizeof(CALC_EXPRSAVEDOUBLE);
          if( lpBuff && nLen + nLenInc < nMaxLen )
          {
            lpSaveDouble = (LPCALC_EXPRSAVEDOUBLE)lpBuff;
            lpSaveDouble->nType = CALC_EXPR_DOUBLE;
            lpSaveDouble->dfVal = lpItem.u.dDouble;
          }
          break;
        case CALC_EXPR_STRING:
          lpText = CalcMemLock(lpItem.u.hText);
          nTextLen = lstrlen(lpText);
          nLenInc = sizeof(CALC_EXPRSAVESTR) + (nTextLen * sizeof(TCHAR));
          ??????;
          break;
        case CALC_EXPR_CELL:
          nLenInc = sizeof(CALC_EXPRSAVECELL);
          if( lpBuff && nLen + nLenInc < nMaxLen )
          {
            lpSaveCell = (LPCALC_EXPRSAVECELL)lpBuff;
            lpSaveCell->nType = CALC_EXPR_CELL;
            lpSaveCell->lCol = lpItem.u.Cell.Col;
            lpSaveCell->lRow = lpItem.u.Cell.Row;
          }
          break;
        case CALC_EXPR_RANGE:
          nLenInc = sizeof(CALC_EXPRSAVERANGE);
          if( lpBuff && nLen + nLenInc < nMaxLen )
          {
            lpSaveRange = (LPCALC_EXPRSAVECELL)lpBuff;
            lpSaveRange->nType = CALC_EXPR_RANGE;
            lpSaveRange->lCol = lpItem.u.Range.Col1;
            lpSaveRange->lRow = lpItem.u.Range.Row1;
            lpSaveRange->lCol2 = lpItem.u.Range.Col2;
            lpSaveRange->lRow2 = lpItem.u.Range.Col2;
          }
          break;
        case CALC_EXPR_NAME:
          nLenInc = sizeof(CALC_EXPRSAVENAME);
          if( lpBuff && nLen + nLenInc < nMaxLen )
          {
            lpSaveCustName = (LPCALC_EXPRSAVECUSTNAME)lpBuff;
            lpSaveCustName->nType = CALC_EXPR_NAME;
            lpSaveCustName->nId = ????;
          }
          break;
        case CALC_EXPR_FUNCTION:
          nLenInc = sizeof(CALC_EXPRSAVEFUNC);
          if( lpBuff && nLen + nLenInc < nMaxLen )
          {
            lpSaveFunc = (LPCALC_EXPRSAVEFUNC)lpBuff;
            lpSaveFunc->nType = CALC_EXPR_FUNCTION;
            lpSaveFunc->nId = lpItem.u.Func.nId;
            lpSaveFunc->nArgs = lpItem.u.Func.nArgs;
          }
          break;
        case CALC_EXPR_CUSTFUNC:
          nLenInc = sizeof(CALC_EXPRSAVECUSTFUNC);
          if( lpBuff && nLen + nLenInc < nMaxLen )
          {
            lpSaveCustFunc = (LPCALC_EXPRSAVECUSTFUNC)lpBuff;
            lpSaveCustFunc->nType = CALC_EXPR_FUNCTION;
            lpSaveCustFunc->nId = lpItem.u.CustFunc.nId;
            lpSaveCustFunc->nArgs = lpItem.u.CustFunc.nArgs;
          }
          break;
        case CALC_EXPR_OPERATOR:
          nLenInc = sizeof(CALC_EXPRSAVEOPER);
          if( lpBuff && nLen + nLenInc < nMaxLen )
          {
            lpSaveOper = (LPCALC_EXPRSAVEOPER)lpBuff;
            lpSaveOper->nType = CALC_EXPR_OPERATOR;
            lpSaveOper->nId = lpItem.u.Oper.nId;
          }
          break;
        case CALC_EXPR_SEPARATOR:
          nLenInc = sizeof(CALC_EXPRSAVEDOUBLE);
          if( lpBuff && nLen + nLenInc < nMaxLen )
          {
            lpSaveSep = (LPCALC_EXPRSAVESEP)lpBuff;
            lpSaveSep->nType = CALC_EXPR_SEP;
            lpSaveSep->nId = lpItem.u.Sep.nId;
          }
          break;
        default:
          nLenInc = 0;
      }       
      nLen += nLenInc;
    }
    CalcMemUnlock(hExpr);
  }
  return nLen;
}

//--------------------------------------------------------------------
//

BOOL ExprLoad(CALC_HANDLE hExpr, LPBYTE lpBuff, int nLen)
{
}
