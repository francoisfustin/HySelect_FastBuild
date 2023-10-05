/*********************************************************
* SS_FUNC.C
*
* Copyright (C) 1991-1993 - FarPoint Technologies
* All Rights Reserved.
*
* No part of this source code may be copied, modified or
* reproduced in any form without retaining the above
* copyright notice.  This source code, or source code
* derived from it, may not be redistributed without
* express written permission of FarPoint Technologies.
*********************************************************/

#include <windows.h>
#include <tchar.h>
#include <ctype.h>
#include <stdlib.h>
#include <stdio.h>
#include <string.h>
#include <math.h>
#include <errno.h>
#include "spread.h"
#include "ss_calc.h"
#include "ss_func.h"

#if defined(SS_OLDCALC) && !defined(SS_NOCALC)

SSX_CALCFUNCTION SS_CalcFunctions[] =
   {
   _T("ADD"),      2, (SS_CALCFUNC)SS_FuncAdd,
   _T("IF"),       3, (SS_CALCFUNC)SS_FuncIf,
   _T("ISEMPTY"),  1, (SS_CALCFUNC)SS_FuncIsEmpty,
   _T("NEG"),      1, (SS_CALCFUNC)SS_FuncNeg,
   _T("ABS"),      1, (SS_CALCFUNC)SS_FuncAbs,
   _T("NOT"),      1, (SS_CALCFUNC)SS_FuncNot,
   _T("PMT"),      4, (SS_CALCFUNC)SS_FuncPmt,
   _T("ROUNDUP"),  2, (SS_CALCFUNC)SS_FuncRoundUp,
   _T("ROUND"),    2, (SS_CALCFUNC)SS_FuncRound,
   _T("TRUNCATE"), 2, (SS_CALCFUNC)SS_FuncTruncate,
   };


/***********************************************************************
* Name:        SSx_CalcFunctionsCnt - Return item cnt in function list
*
* Usage:       void SSx_CalcFunctionsCnt()
*
* Description: Return the number of items in the function list
*
* Return:      Number of items in the function list
***********************************************************************/

short SSx_CalcFunctionsCnt()
{
return (sizeof(SS_CalcFunctions) / sizeof(SSX_CALCFUNCTION));
}


/***********************************************************************
* Name:        SS_FuncAdd - Function to add to numbers
*
* Usage:       BOOL SS_FuncAdd(Function, Result, datatbl, deftbl,
*                              dataid)
*                 Function - Formula to be processed
*                 Result  - Result of processing the function
*                 datatbl - Data Table
*                 deftbl  - Definition Table
*                 dataid  - Master data ID
*
* Description: This function adds two numbers.  There must be two
*              numeric parameters.
*              ADD(Value1, Value2)
*
* Return:      NULL - Invalid function
*              Pointer to position in formula following the function
***********************************************************************/

BOOL FAR PASCAL SS_FuncAdd(Result, Values, ValueCnt)

LPSS_VALUE Result;
LPSS_VALUE Values;
short      ValueCnt;
{
SSx_EvalAdd(&Values[0], &Values[1], Result);
return (TRUE);
}


/***********************************************************************
* Name:        SS_FuncIf - Function to perform IF, THEN, ELSE
*
* Usage:       BOOL SS_FuncIf(Formula, Result, datatbl, deftbl,
*                              dataid)
*                 Formula - Formula to be processed
*                 Result  - Result of processing the function
*                 datatbl - Data Table
*                 deftbl  - Definition Table
*                 dataid  - Master data ID
*
* Description: This function takes three parameters.  The first one
*              is the conditional, next the THEN statement, and last
*              then ELSE.
*              IF(Conditional, Value1, Value2)
*
* Return:      NULL - Invalid function
*              Pointer to position in formula following the function
***********************************************************************/

BOOL FAR PASCAL SS_FuncIf(Result, Values, ValueCnt)

LPSS_VALUE Result;
LPSS_VALUE Values;
short      ValueCnt;
{
LPSS_VALUE Value1;
LPSS_VALUE Value2;
LPSS_VALUE Value3;
BOOL       Conditional;

Value1 = &Values[0];
Value2 = &Values[1];
Value3 = &Values[2];

if (Value1->Status == SS_VALUE_STATUS_OK)
   {
   if (Value1->Type == SS_VALUE_TYPE_DOUBLE)
      {
      if (Value1->Val.ValDouble == 0.0)
         Conditional = FALSE;
      else
         Conditional = TRUE;
      }
   else if (Value1->Type == SS_VALUE_TYPE_LONG)
      {
      if (Value1->Val.ValLong == 0)
         Conditional = FALSE;
      else
         Conditional = TRUE;
      }
   else
      Conditional = TRUE;

   if (Conditional == TRUE)
      _fmemcpy(Result, Value2, sizeof(SS_VALUE));
   else
      _fmemcpy(Result, Value3, sizeof(SS_VALUE));
   }
else
   _fmemcpy(Result, Value1, sizeof(SS_VALUE));

if (Result->Type == SS_VALUE_TYPE_STR && Result->Val.hValStr)
   {
   GLOBALHANDLE hText = Result->Val.hValStr;
   LPTSTR       lpszText;
   LPTSTR       lpszValStr;

   lpszText = (LPTSTR)GlobalLock(hText);

   if (Result->Val.hValStr = GlobalAlloc(GMEM_MOVEABLE | GMEM_ZEROINIT,
                                         (lstrlen(lpszText) + 1) * sizeof(TCHAR)))
      {
      lpszValStr = (LPTSTR)GlobalLock(Result->Val.hValStr);
      lstrcpy(lpszValStr, lpszText);
      GlobalUnlock(Result->Val.hValStr);
      }

   GlobalUnlock(hText);
   }

return (TRUE);
}


/***********************************************************************
* Name:        SS_FuncIsEmpty - Determines if data item is empty
*
* Usage:       BOOL SS_FuncIsEmpty(Formula, Result, datatbl, deftbl,
*                                   dataid)
*                 Formula - Formula to be processed
*                 Result  - Result of processing the function
*                 datatbl - Data Table
*                 deftbl  - Definition Table
*                 dataid  - Master data ID
*
* Description: Only one parameter is allowed.  If there is anything
*              in the data item, result is set to FALSE, otherwise
*              result is TRUE.
*              ISEMPTY(Value)
*
* Return:      NULL - Invalid function
*              Pointer to position in formula following the function
***********************************************************************/

BOOL FAR PASCAL SS_FuncIsEmpty(Result, Values, ValueCnt)

LPSS_VALUE Result;
LPSS_VALUE Values;
short      ValueCnt;
{
LPSS_VALUE Value;

Value = &Values[0];

Result->Type = SS_VALUE_TYPE_LONG;
Result->Status = SS_VALUE_STATUS_OK;

if (Value->Status == SS_VALUE_STATUS_OK)
   Result->Val.ValLong = FALSE;

else if (Value->Status == SS_VALUE_STATUS_EMPTY)
   Result->Val.ValLong = TRUE;

else
   Result->Status = SS_VALUE_STATUS_ERROR;

return (TRUE);
}


/***********************************************************************
* Name:        SS_FuncNeg - Return the negation of the number
*
* Usage:       BOOL SS_FuncNeg(Formula, Result, datatbl, deftbl,
*                               dataid)
*                 Formula - Formula to be processed
*                 Result  - Result of processing the function
*                 datatbl - Data Table
*                 deftbl  - Definition Table
*                 dataid  - Master data ID
*
* Description: The one parameter may be a float of integer.  Return
*              the negation of that number.
*              NEG(Value)
*
* Return:      NULL - Invalid function
*              Pointer to position in formula following the function
***********************************************************************/

BOOL FAR PASCAL SS_FuncNeg(Result, Values, ValueCnt)

LPSS_VALUE Result;
LPSS_VALUE Values;
short      ValueCnt;
{
LPSS_VALUE Value;

Value = &Values[0];

if (Value->Status != SS_VALUE_STATUS_OK)
   _fmemcpy(Result, Value, sizeof(SS_VALUE));

else if (Value->Type == SS_VALUE_TYPE_STR)
   Result->Status = SS_VALUE_STATUS_ERROR;

else
   {
   Result->Status = SS_VALUE_STATUS_OK;
   Result->Type = Value->Type;

   if (Value->Type == SS_VALUE_TYPE_DOUBLE)
      Result->Val.ValDouble = Value->Val.ValDouble * -1.0;
   else
      Result->Val.ValLong = Value->Val.ValLong * -1;
   }

return (TRUE);
}


/***********************************************************************
* Name:        SS_FuncAbs - Return absolute value of parameter
*
* Usage:       BOOL SS_FuncAbs(Formula, Result, datatbl, deftbl,
*                               dataid)
*                 Formula - Formula to be processed
*                 Result  - Result of processing the function
*                 datatbl - Data Table
*                 deftbl  - Definition Table
*                 dataid  - Master data ID
*
* Description: The one parameter may be a float of integer.  Return
*              the absolute value of that number.
*              ABS(Value)
*
* Return:      NULL - Invalid function
*              Pointer to position in formula following the function
***********************************************************************/

BOOL FAR PASCAL SS_FuncAbs(Result, Values, ValueCnt)

LPSS_VALUE Result;
LPSS_VALUE Values;
short      ValueCnt;
{
LPSS_VALUE Value;

Value = &Values[0];

if (Value->Type == SS_VALUE_TYPE_STR)
   Result->Status = SS_VALUE_STATUS_ERROR;

else if (Value->Status != SS_VALUE_STATUS_OK)
   _fmemcpy(Result, Value, sizeof(SS_VALUE));

else
   {
   Result->Status = SS_VALUE_STATUS_OK;
   Result->Type = Value->Type;

   if (Value->Type == SS_VALUE_TYPE_DOUBLE)
      Result->Val.ValDouble = fabs(Value->Val.ValDouble);
   else
      Result->Val.ValLong = labs(Value->Val.ValLong);
   }

return (TRUE);
}


/***********************************************************************
* Name:        SS_FuncNot - Return result of NOT operation
*
* Usage:       BOOL SS_FuncNot(Formula, Result, datatbl, deftbl,
*                               dataid)
*                 Formula - Formula to be processed
*                 Result  - Result of processing the function
*                 datatbl - Data Table
*                 deftbl  - Definition Table
*                 dataid  - Master data ID
*
* Description: If the parameter is 0 a 1 is returned, otherwise a 0
*              is returned.
*              NOT(param)
*
* Return:      NULL - Invalid function
*              Pointer to position in formula following the function
***********************************************************************/

BOOL FAR PASCAL SS_FuncNot(Result, Values, ValueCnt)

LPSS_VALUE Result;
LPSS_VALUE Values;
short      ValueCnt;
{
LPSS_VALUE Value;

Value = &Values[0];

if (Value->Status != SS_VALUE_STATUS_OK)
   _fmemcpy(Result, Value, sizeof(SS_VALUE));
else
   {
   Result->Status = SS_VALUE_STATUS_OK;
   Result->Type = SS_VALUE_TYPE_LONG;

   if (Value->Type == SS_VALUE_TYPE_DOUBLE)
      {
      if (Value->Val.ValDouble == 0.0)
         Result->Val.ValLong = TRUE;
      else
         Result->Val.ValLong = FALSE;
      }

   else if (Value->Type == SS_VALUE_TYPE_LONG)
      {
      if (Value->Val.ValLong == 0)
         Result->Val.ValLong = TRUE;
      else
         Result->Val.ValLong = FALSE;
      }

   else
      Result->Val.ValLong = FALSE;
   }

return (TRUE);
}


/***********************************************************************
* Name:        SS_FuncPmt - Calculate the payment per term
*
* Usage:       BOOL SS_FuncPmt(Formula, Result, datatbl, deftbl,
*                               dataid)
*                 Formula - Formula to be processed
*                 Result  - Result of processing the function
*                 datatbl - Data Table
*                 deftbl  - Definition Table
*                 dataid  - Master data ID
*
* Description: Calculate the period per term using the parameters
*              PMT(Amount, Interest, Term, TermsPerYear)
*
* Return:      NULL - Invalid function
*              Pointer to position in formula following the function
***********************************************************************/

BOOL FAR PASCAL SS_FuncPmt(Result, Values, ValueCnt)

LPSS_VALUE Result;
LPSS_VALUE Values;
short      ValueCnt;
{
LPSS_VALUE Amt;
LPSS_VALUE Interest;
LPSS_VALUE Term;
LPSS_VALUE TermsPerYear;
double     dAmt;
double     dInterest;
double     dTerm;
double     dTermsPerYear;
double     dIr;
double     dPmt;

Amt = &Values[0];
Interest = &Values[1];
Term = &Values[2];
TermsPerYear = &Values[3];

if (Amt->Type == SS_VALUE_TYPE_STR || Interest->Type == SS_VALUE_TYPE_STR ||
    Term->Type == SS_VALUE_TYPE_STR || TermsPerYear->Type == SS_VALUE_TYPE_STR)
   Result->Status = SS_VALUE_STATUS_ERROR;

else if (Amt->Status != SS_VALUE_STATUS_OK ||
         Interest->Status != SS_VALUE_STATUS_OK ||
         Term->Status != SS_VALUE_STATUS_OK ||
         TermsPerYear->Status != SS_VALUE_STATUS_OK)
   Result->Status = SS_VALUE_STATUS_EMPTY;

else
   {
   if (Amt->Type == SS_VALUE_TYPE_DOUBLE)
      dAmt = Amt->Val.ValDouble;
   else
      dAmt = (double)Amt->Val.ValLong;

   if (Interest->Type == SS_VALUE_TYPE_DOUBLE)
      dInterest = Interest->Val.ValDouble;
   else
      dInterest = (double)Interest->Val.ValLong;

   if (Term->Type == SS_VALUE_TYPE_DOUBLE)
      dTerm = Term->Val.ValDouble;
   else
      dTerm = (double)Term->Val.ValLong;

   if (TermsPerYear->Type == SS_VALUE_TYPE_DOUBLE)
      dTermsPerYear = TermsPerYear->Val.ValDouble;
   else
      dTermsPerYear = (double)TermsPerYear->Val.ValLong;

   if (dInterest == 0 || dTerm == 0 || dTermsPerYear == 0)
      return (FALSE);

   dIr = dInterest / 100 / dTermsPerYear;
   dPmt = (dIr / (pow(1 + dIr, dTerm) - 1) + dIr) * dAmt;
   dPmt = SS_RoundUp(dPmt, 2L);

   Result->Type = SS_VALUE_TYPE_DOUBLE;
   Result->Status = SS_VALUE_STATUS_OK;
   Result->Val.ValDouble = dPmt;
   }

return (TRUE);
}


/***********************************************************************
* Name:        SS_FuncRoundUp - Round a number up to the significant
*                               decimal place
*
* Usage:       BOOL SS_FuncRoundUp(Formula, Result, datatbl, deftbl,
*                                   dataid)
*                 Formula - Formula to be processed
*                 Result  - Result of processing the function
*                 datatbl - Data Table
*                 deftbl  - Definition Table
*                 dataid  - Master data ID
*
* Description: Round a number up using the precision provided
*              ROUNDUP(Value, precision)
*
* Return:      NULL - Invalid function
*              Pointer to position in formula following the function
***********************************************************************/

BOOL FAR PASCAL SS_FuncRoundUp(Result, Values, ValueCnt)

LPSS_VALUE Result;
LPSS_VALUE Values;
short      ValueCnt;
{
LPSS_VALUE Value1;
LPSS_VALUE Value2;
double     dVal1;

Value1 = &Values[0];
Value2 = &Values[1];

if (Value1->Type == SS_VALUE_TYPE_STR || Value2->Type == SS_VALUE_TYPE_STR)
   return (FALSE);

if (Value1->Status != SS_VALUE_STATUS_OK ||
    Value2->Status != SS_VALUE_STATUS_OK)
   {
   Result->Status = SS_VALUE_STATUS_EMPTY;
   return (TRUE);
   }

if (Value1->Type == SS_VALUE_TYPE_DOUBLE)
   dVal1 = Value1->Val.ValDouble;
else
   dVal1 = (double)Value1->Val.ValLong;

if (Value2->Type != SS_VALUE_TYPE_LONG)
   return (FALSE);

Result->Val.ValDouble = SS_RoundUp(dVal1, Value2->Val.ValLong);
Result->Type = SS_VALUE_TYPE_DOUBLE;
Result->Status = SS_VALUE_STATUS_OK;

return (TRUE);
}


/***********************************************************************
* Name:        SS_FuncRound - Round a number to the significant
*                             decimal place
*
* Usage:       BOOL SS_FuncRound(Formula, Result, datatbl, deftbl,
*                                 dataid)
*                 Formula - Formula to be processed
*                 Result  - Result of processing the function
*                 datatbl - Data Table
*                 deftbl  - Definition Table
*                 dataid  - Master data ID
*
* Description: Round a number using the precision provided.  >= 5.0
*              is rounded up, otherwise round down
*              ROUND(Value, precision)
*
* Return:      NULL - Invalid function
*              Pointer to position in formula following the function
***********************************************************************/

BOOL FAR PASCAL SS_FuncRound(Result, Values, ValueCnt)

LPSS_VALUE Result;
LPSS_VALUE Values;
short      ValueCnt;
{
LPSS_VALUE Value1;
LPSS_VALUE Value2;
double     dVal1;

Value1 = &Values[0];
Value2 = &Values[1];

if (Value1->Type == SS_VALUE_TYPE_STR || Value2->Type == SS_VALUE_TYPE_STR)
   return (FALSE);

if (Value1->Status != SS_VALUE_STATUS_OK ||
    Value2->Status != SS_VALUE_STATUS_OK)
   {
   Result->Status = SS_VALUE_STATUS_EMPTY;
   return (TRUE);
   }

if (Value1->Type == SS_VALUE_TYPE_DOUBLE)
   dVal1 = Value1->Val.ValDouble;
else
   dVal1 = (double)Value1->Val.ValLong;

if (Value2->Type != SS_VALUE_TYPE_LONG)
   return (FALSE);

Result->Val.ValDouble = SS_Round(dVal1, Value2->Val.ValLong);
Result->Type = SS_VALUE_TYPE_DOUBLE;
Result->Status = SS_VALUE_STATUS_OK;

return (TRUE);
}


/***********************************************************************
* Name:        SS_FuncTruncate - Truncate a value to the significant
*                                decimal place
*
* Usage:       BOOL SS_FuncTruncate(Formula, Result, datatbl, deftbl,
*                                    dataid)
*                 Formula - Formula to be processed
*                 Result  - Result of processing the function
*                 datatbl - Data Table
*                 deftbl  - Definition Table
*                 dataid  - Master data ID
*
* Description: Truncate a number using the precision provided.
*              TRUNCATE(Value, precision)
*
* Return:      NULL - Invalid function
*              Pointer to position in formula following the function
***********************************************************************/

BOOL FAR PASCAL SS_FuncTruncate(Result, Values, ValueCnt)

LPSS_VALUE Result;
LPSS_VALUE Values;
short      ValueCnt;
{
LPSS_VALUE Value1;
LPSS_VALUE Value2;
double     dVal1;

Value1 = &Values[0];
Value2 = &Values[1];

if (Value1->Type == SS_VALUE_TYPE_STR || Value2->Type == SS_VALUE_TYPE_STR)
   return (FALSE);

if (Value1->Status != SS_VALUE_STATUS_OK ||
    Value2->Status != SS_VALUE_STATUS_OK)
   {
   Result->Status = SS_VALUE_STATUS_EMPTY;
   return (TRUE);
   }

if (Value1->Type == SS_VALUE_TYPE_DOUBLE)
   dVal1 = Value1->Val.ValDouble;

else if (Value1->Type == SS_VALUE_TYPE_LONG)
   dVal1 = (double)Value1->Val.ValLong;

else
   return (FALSE);

if (Value2->Type != SS_VALUE_TYPE_LONG)
   return (FALSE);

Result->Val.ValDouble = SS_Truncate(dVal1, Value2->Val.ValLong);
Result->Type = SS_VALUE_TYPE_DOUBLE;
Result->Status = SS_VALUE_STATUS_OK;

return (TRUE);
}

#endif                // defined(SS_OLDCALC) && !defined(SS_NOCALC)
