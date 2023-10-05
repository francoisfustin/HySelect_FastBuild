//--------------------------------------------------------------------
//
//  File: cal_oper.c
//
//  Description: Built-in operators
//

#include <string.h>

#include "calc.h"
#include "cal_oper.h"

//--------------------------------------------------------------------
//
//  The OperTable[] array holds information about allowable operators.
//
//  Note: The order of the OperTable table must match the order of the
//        CALC_OPER_? constants which are defined in CAL_OPER.H file.
//        This allows quicker lookups.
//
//  Note: Valid operators are always represented by a nonzero id code.
//        Thus, OperTable[0] is not used.
//

const CALC_OPERATOR OperTable[] =
{
  {_T(""),   FALSE, 0, NULL},
  {_T(":"),  TRUE,  8, Calc_range},
  {_T("-"),  FALSE, 7, Calc_neg},
  {_T("^"),  TRUE,  6, Calc_expon},
  {_T("*"),  TRUE,  5, Calc_mult},
  {_T("/"),  TRUE,  5, Calc_div},
  {_T("+"),  TRUE,  4, Calc_add},
  {_T("-"),  TRUE,  4, Calc_sub},
  {_T(""),   TRUE,  3, Calc_concat},  // was _T("&") in early beta
  {_T("="),  TRUE,  2, Calc_equal},
  {_T("<>"), TRUE,  2, Calc_notequal},
  {_T("<"),  TRUE,  2, Calc_less},
  {_T(">"),  TRUE,  2, Calc_greater},
  {_T("<="), TRUE,  2, Calc_lesseq},
  {_T(">="), TRUE,  2, Calc_greatereq},
  {_T("&"),  TRUE,  1, Calc_and},
  {_T("|"),  TRUE,  1, Calc_or},
  {_T("!"),  FALSE, 7, Calc_not},
};

const int sizeOperTable = sizeof(OperTable) / sizeof(OperTable[0]);

//--------------------------------------------------------------------
//
//  The OperLookup() function determines if the given text represents
//  a valid operator of the given type (unary or binary).  If so, the
//  function returns the id code of the operator.  Otherwise, the
//  function returns zero.
//

int OperLookup(LPCTSTR lpszText, BOOL bBinary)
{
  int i;
  
  for(i = 0; i < sizeOperTable; i++)
    if( bBinary == OperTable[i].bBinary )
      if( lstrcmp(lpszText, OperTable[i].lpszText) == 0)
        return i;
  return 0;
}

//--------------------------------------------------------------------
//
//  The OperFirst() function retrieves the first operator.
//

int OperFirst(void)
{
  return 1;
}

//--------------------------------------------------------------------
//
//  The OperNext() function retrieves the next operator.
//

int OperNext(int nId)
{
  if( nId < sizeof(OperTable) - 1 )
    return nId + 1;
  else
    return 0;
}

//--------------------------------------------------------------------
//
//  The OperIsBinary() function determines if the given operator is
//  a binary operator.
//

BOOL OperIsBinary(int nId)
{
  if( 0 < nId && nId < sizeOperTable )
    return OperTable[nId].bBinary;
  else
    return FALSE;
}

//--------------------------------------------------------------------
//
//  The OperGetText() function retrieves the text representation of
//  the given operator.
//

int OperGetText(int nId, LPTSTR lpszText, int nLen)
{
  int nOperLen;
  int nResult = 0;
  
  if( 0 < nId && nId < sizeOperTable )
  {
    nOperLen = lstrlen(OperTable[nId].lpszText);
    if( nOperLen < nLen )
    {
      lstrcpy(lpszText, OperTable[nId].lpszText);
      nResult = nOperLen;
    }
  }
  return nResult;
}

//--------------------------------------------------------------------
//
//  The OperGetTextLen() function retrieves the length of the text
//  representation of the given operator.
//

int OperGetTextLen(int nId)
{
  int nRet = 0;
  
  if( 0 < nId && nId < sizeOperTable )
    nRet = lstrlen(OperTable[nId].lpszText);
  return nRet;
}

//--------------------------------------------------------------------
//
//  The OperGetPred() function retrieves the precendence of the given
//  operator.
//

int OperGetPred(int nId)
{
  if( 0 < nId && nId < sizeOperTable )
    return OperTable[nId].nPred;
  else
    return 0;
}

//--------------------------------------------------------------------
//
//  The OperGetProc() function retrieves the routine used to evaluate
//  the given operator.
//

CALC_EVALPROC OperGetProc(int nId)
{
  if( 0 < nId && nId < sizeOperTable )
    return OperTable[nId].lpfnProc;
  else
    return NULL;
}
