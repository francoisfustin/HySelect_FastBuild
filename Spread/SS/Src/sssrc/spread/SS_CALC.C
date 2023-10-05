/*********************************************************
* SS_CALC.C
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
#include <math.h>
#include <errno.h>
#include <string.h>
#include "spread.h"
#include "ss_alloc.h"
#include "ss_calc.h"
#include "ss_cell.h"
#include "ss_col.h"
#include "ss_func.h"
#include "ss_main.h"
#include "ss_row.h"
#include "ss_type.h"

#ifndef SS_NOCALC

#ifdef SS_OLDCALC

/**********************
* Function Prototypes
**********************/

static LPCTSTR SSx_ParseItemCoord(LPSPREADSHEET lpSS, LPCTSTR Formula,
                                  LPSS_COORD CellRow, LPSS_COORD CellCol,
                                  TBGLOBALHANDLE ValidateCalc);
static BOOL   SSx_GetFunction(LPSPREADSHEET lpSS,
                              LPSS_CALCFUNCTION CalcFunction,
                              LPCTSTR FunctionName, short Len);
static LPCTSTR SSx_CalcGetParms(LPSPREADSHEET lpSS, short FAR *ParamCnt,
                                LPCTSTR Formula, LPSS_VALUE Results,
                                TBGLOBALHANDLE ValidateCalc);
static BOOL  SS_CalcBuildDependencies1(LPSPREADSHEET lpSS, LPSS_CALC Calc);
static BOOL  SS_CalcBuildDependencies2(LPSPREADSHEET lpSS,
                                       LPSS_CALCLIST Dependents,
                                       LPSS_CELLCOORD CellCoord);
static BOOL  SS_CalcBuildDependencies3(LPSPREADSHEET lpSS,
                                       LPSS_CALCLIST Dependents,
                                       LPSS_CELLCOORD CellCoord,
                                       LPSS_CALCLIST DependentsTemp,
                                       LPSS_CELLCOORD lpDependItems1,
                                       short DependItemsCnt1, LPBOOL lpfFirst,
                                       LPBOOL lpfFirstCell);
static BOOL  SSx_CalcDependencies(LPSPREADSHEET lpSS,
                                  LPSS_CALCLIST Dependents,
                                  LPSS_CELLCOORD lpCellCurrent, BOOL fAll,
                                  BOOL fExpand);
static void  CalcMinus(LPSS_VALUE lpResult);
static void SS_FreeValue(LPSS_VALUE lpVal);
BOOL SS_AddCustomFunction(LPSPREADSHEET lpSS, LPCTSTR lpszFuncName,
                          short nParamCnt, FARPROC lpfnFuncProc);


/***********************************************************************
* Name:        SS_Calc - Calculate the result using the formula
*
* Usage:       void SS_Calc(Formula, Result, datatbl, dataid)
*                 Formula - Formula used to calculate result
*                 Result  - Result from the calculation
*                 datatbl - Data Table
*                 dataid  - Current Data ID
*
* Description: Use the provided formula to calculate the result.
*              The following symbols are used in the formula:
*                 @ - Proceeds a function name
*                 # - Proceeds a Numeric constant
*                 % - Proceeds an item in the data table
*                 $ - Proceeds string constant
*
* Return:      Void
***********************************************************************/

void SS_Calc(lpSS, Formula, Result)

LPSPREADSHEET lpSS;
LPTSTR        Formula;
LPSS_VALUE    Result;
{
SSx_Calculate(lpSS, Formula, Result, FALSE);
}


/***********************************************************************
* Name:        SS_CalcIsValid - Test the validity of a formula
*
* Usage:       BOOL SS_CalcIsValid(Formula, deftbl)
*                 Formula - Formula used to calculate result
*                 deftbl  - Definition Table
*
* Description: Calculate the formula using a NULL data table to
*              determine the validity of the formula.
*
* Return:      TRUE  - Formula valid
*              FALSE - Formula invalid
***********************************************************************/

BOOL SS_CalcIsValid(lpSS, Formula, hCalc)

LPSPREADSHEET   lpSS;
LPCTSTR         Formula;
TBGLOBALHANDLE  hCalc;
{
SS_VALUE        Result;

SSx_Calculate(lpSS, Formula, &Result, hCalc);

if (Result.Status == SS_VALUE_STATUS_ERROR)
   return (FALSE);

SS_FreeValue(&Result);

return (TRUE);
}


/***********************************************************************
* Name:        SSx_Calculate - Perform the actual calculation
*
* Usage:       LPCTSTR SSx_Calculate(Formula, Result, datatbl, deftbl,
                                     dataid)
*                 Formula - Formula used to calculate result
*                 Result  - Result from the calculation
*                 datatbl - Data Table
*                 deftbl  - Definition Table
*                 dataid  - Current Data ID
*
* Description: If the data table is not NULL, the calculation will
*              be performed.  Otherwise, the validity of the formula
*              will be determined.
*
* Return:      Pointer to place in formula.  This value is only used
*              for recursive purposes and can be ignored upon return
*              of the function.
***********************************************************************/

LPCTSTR SSx_Calculate(lpSS, Formula, Result, ValidateCalc)

LPSPREADSHEET  lpSS;
LPCTSTR        Formula;
LPSS_VALUE     Result;
TBGLOBALHANDLE ValidateCalc;
{
LPCTSTR        FormulaTemp;
SS_VALUE       Val1;
SS_VALUE       Val2;
short          Operation = OP_NONE;
BOOL           fMinus;

Val1.Status = SS_VALUE_STATUS_CLEAR;
Val2.Status = SS_VALUE_STATUS_CLEAR;
Result->Status = SS_VALUE_STATUS_EMPTY;

if (*Formula == '=')
   Formula++;

while (Formula && *Formula != '\0' && Result->Status != SS_VALUE_STATUS_ERROR)
   {
   fMinus = FALSE;

   Formula = SSx_SkipBlanks(Formula);

   if (*Formula == '-')
      {
      fMinus = TRUE;
      Formula++;
      Formula = SSx_SkipBlanks(Formula);
      }

   switch (*Formula)
      {
      case '+':
         Operation = OP_ADDITION;
         Formula++;
         break;

      /*
      case '-':
         fMinus = TRUE;
         Operation = OP_SUBTRACTION;
         Formula++;
         break;
      */

      case '^':
         Operation = OP_POW;
         Formula++;
         break;

      case '*':
         Operation = OP_MULTIPLICATION;
         Formula++;
         break;

      case '/':
         Operation = OP_DIVISION;
         Formula++;
         break;

      case '(':
         Formula++;
         Formula = SSx_Calculate(lpSS, Formula, Result, ValidateCalc);

         if (fMinus)
            CalcMinus(Result);

         if (*(Formula - 1) != ')')
            Result->Status = SS_VALUE_STATUS_ERROR;
         else if (Val1.Status == SS_VALUE_STATUS_CLEAR)
            _fmemcpy(&Val1, Result, sizeof(SS_VALUE));
         else
            _fmemcpy(&Val2, Result, sizeof(SS_VALUE));

         break;

      case ')':
      case ',':
         Formula++;
         return (Formula);
         break;

      case SS_CALC_PRFX_STR:
         Formula++;
         Formula = SSx_ParseStr(Formula, Result);
         if (Val1.Status == SS_VALUE_STATUS_CLEAR)
            _fmemcpy(&Val1, Result, sizeof(SS_VALUE));
         else
            _fmemcpy(&Val2, Result, sizeof(SS_VALUE));
         break;

      case '&':
         Operation = OP_AND;
         Formula++;
         break;

      case '|':
         Operation = OP_OR;
         Formula++;
         break;

      case '!':
         Formula++;
         if (*Formula == '=')
            {
            Operation = OP_NOTEQUAL;
            Formula++;
            }
         else
            Result->Status = SS_VALUE_STATUS_ERROR;

         break;

      case '>':
         Operation = OP_GREATER;
         Formula++;
         if (*Formula == '=')
            {
            Operation = OP_GREATEREQUAL;
            Formula++;
            }
         break;

      case '<':
         Operation = OP_LESS;
         Formula++;
         if (*Formula == '=')
            {
            Operation = OP_LESSEQUAL;
            Formula++;
            }
         break;

      case '=':
         Operation = OP_EQUAL;
         Formula++;
         break;

      default:

         /**************************
         * Check for function call
         **************************/

         if (FormulaTemp = SSx_ParseFunction(lpSS, Formula, Result,
                                             ValidateCalc))
            {
            if (fMinus)
               CalcMinus(Result);

            Formula = FormulaTemp;
            if (Val1.Status == SS_VALUE_STATUS_CLEAR)
               _fmemcpy(&Val1, Result, sizeof(SS_VALUE));
            else
               _fmemcpy(&Val2, Result, sizeof(SS_VALUE));
            }

         /*********************
         * Check for a number
         *********************/

         else if ((FormulaTemp = SSx_ParseNumber(Formula, Result)) &&
                  Result->Status != SS_VALUE_STATUS_ERROR)
            {
            if (fMinus)
               CalcMinus(Result);

            Formula = FormulaTemp;
            if (Val1.Status == SS_VALUE_STATUS_CLEAR)
               _fmemcpy(&Val1, Result, sizeof(SS_VALUE));
            else
               _fmemcpy(&Val2, Result, sizeof(SS_VALUE));
            }

         /******************************
         * Check for a Cell coordinate
         ******************************/

         else if (FormulaTemp = SSx_ParseItem(lpSS, Formula, Result,
                                              ValidateCalc))
            {
            if (fMinus)
               CalcMinus(Result);

            Formula = FormulaTemp;
            if (Val1.Status == SS_VALUE_STATUS_CLEAR)
               _fmemcpy(&Val1, Result, sizeof(SS_VALUE));
            else
               _fmemcpy(&Val2, Result, sizeof(SS_VALUE));
            }

         /****************
         * Error occured
         ****************/

         else
            {
            Result->Status = SS_VALUE_STATUS_ERROR;
            return (Formula);
            }

         break;
      }

   if (fMinus && Operation == OP_NONE)
      Operation = OP_ADDITION;

   if ((Result->Status != SS_VALUE_STATUS_ERROR || Formula == NULL) &&
       Val1.Status != SS_VALUE_STATUS_CLEAR &&
       Val2.Status != SS_VALUE_STATUS_CLEAR && Operation != OP_NONE)
      {
      SSx_Evaluate(Operation, &Val1, &Val2, Result);
      SS_FreeValue(&Val1);
      SS_FreeValue(&Val2);
      _fmemcpy(&Val1, Result, sizeof(SS_VALUE));
      Val2.Status = SS_VALUE_STATUS_CLEAR;
      Operation = OP_NONE;
      }
   }

/*******************************
* If an error occured or the
* value is empty, clear it out
*******************************/

if (Result->Status == SS_VALUE_STATUS_ERROR ||
    Result->Status == SS_VALUE_STATUS_EMPTY)
   Result->Val.ValDouble = 0.0;

return (Formula);
}


void CalcMinus(LPSS_VALUE lpResult)
{
if (lpResult->Type == SS_VALUE_TYPE_LONG)
   lpResult->Val.ValLong *= -1;

else if (lpResult->Type == SS_VALUE_TYPE_DOUBLE)
   lpResult->Val.ValDouble *= -1;
}


/***********************************************************************
* Name:        SSx_Evaluate - Evaluate the expression
*
* Usage:       void SSx_Evaluate(Operation, Val1, Val2, Result)
*                 Operation - The operation to be performed
*                 Val1      - The first value to be used
*                 Val2      - The second value to be used
*                 Result    - The result of the operation
*
* Description: Take two values and perform the desired operation
*              using those two values and return the result.
*
* Return:      Void
***********************************************************************/

void SSx_Evaluate(Operation, Val1, Val2, Result)

short      Operation;
LPSS_VALUE Val1;
LPSS_VALUE Val2;
LPSS_VALUE Result;
{
switch (Operation)
   {
   case OP_ADDITION:
      SSx_EvalAdd(Val1, Val2, Result);
      break;

   case OP_SUBTRACTION:
      SSx_EvalSubtract(Val1, Val2, Result);
      break;

   case OP_POW:
      SSx_EvalExponential(Val1, Val2, Result);
      break;

   case OP_MULTIPLICATION:
      SSx_EvalMultiply(Val1, Val2, Result);
      break;

   case OP_DIVISION:
      SSx_EvalDivide(Val1, Val2, Result);
      break;

   case OP_AND:
      SSx_EvalAnd(Val1, Val2, Result);
      break;

   case OP_OR:
      SSx_EvalOr(Val1, Val2, Result);
      break;

   case OP_NOTEQUAL:
   case OP_GREATER:
   case OP_GREATEREQUAL:
   case OP_LESS:
   case OP_LESSEQUAL:
   case OP_EQUAL:
      SSx_EvalEqual(Operation, Val1, Val2, Result);
      break;
   }
}


/***********************************************************************
* Name:        SSx_ParseNumber - Parse a number from the formula
*
* Usage:       LPCTSTR SSx_ParseNumber(Formula, ValTemp)
*                 Formula - Pointer to current place in formula
*                 ValTemp - The resulting number to be returned
*
* Description: Parse the number and determine if it is a float
*              or an integer.
*
* Return:      The current formula position after the parsed number.
***********************************************************************/

LPCTSTR SSx_ParseNumber(Formula, ValTemp)

LPCTSTR      Formula;
LPSS_VALUE   ValTemp;
{
LPCTSTR      FormulaTemp = NULL;
static TCHAR *BufferPtr;
static TCHAR Buffer[SS_CALC_FORMULA_MAXLEN + 1];

_ftcsncpy(Buffer, Formula, sizeof(Buffer)/sizeof(TCHAR));

ValTemp->Val.ValLong = _tcstol(Buffer, &BufferPtr, 10);

if (BufferPtr)
   FormulaTemp = Formula + (BufferPtr - Buffer);

if (errno == ERANGE)
   {
   ValTemp->Status = SS_VALUE_STATUS_ERROR;
   errno = 0;
   }

else if (*FormulaTemp == '.')
   {
   ValTemp->Val.ValDouble = _tcstod(Buffer, &BufferPtr);

   if (BufferPtr)
      FormulaTemp = Formula + (BufferPtr - Buffer);

   if (errno == ERANGE || FormulaTemp == NULL || FormulaTemp == Formula)
      {
      ValTemp->Status = SS_VALUE_STATUS_ERROR;
      if (errno == ERANGE)
         errno = 0;
      }

   else
      {
      ValTemp->Type = SS_VALUE_TYPE_DOUBLE;
      ValTemp->Status = SS_VALUE_STATUS_OK;
      }
   }

else if (FormulaTemp == NULL || FormulaTemp == Formula)
   ValTemp->Status = SS_VALUE_STATUS_ERROR;

else
   {
   ValTemp->Type = SS_VALUE_TYPE_LONG;
   ValTemp->Status = SS_VALUE_STATUS_OK;
   }

return (FormulaTemp);
}


/***********************************************************************
* Name:        SSx_ParseItem - Parse an item using the data table
*
* Usage:       LPCTSTR SSx_ParseItem(Formula, Value, datatbl, deftbl,
*                                   dataid_master)
*                 Formula       - The formula to be used
*                 Value         - The value returned
*                 datatbl       - Data Table
*                 deftbl        - Definition Table
*                 dataid_master - The master dataid
*
* Description: Determine the dataid of the parsed item.  Get the
*              data ptr and determine if the data is a float,
*              integer or string.
*
* Return:      The current position following the parsed item
*              NULL - An error occured
***********************************************************************/

LPCTSTR SSx_ParseItem(lpSS, Formula, Value, ValidateCalc)

LPSPREADSHEET  lpSS;
LPCTSTR        Formula;
LPSS_VALUE     Value;
TBGLOBALHANDLE ValidateCalc;
{
SS_DATA        Data;
LPSS_COL       lpCol;
LPSS_ROW       lpRow;
LPSS_CELL      lpCell;
SS_CELLTYPE    CellTypeTemp;
LPSS_CELLTYPE  CellType;
LPSS_CALC      Calc;
LPSS_CELLCOORD Ancestors;
SS_VALUE       Value1;
SS_VALUE       ValueTemp;
SS_COORD       CellRow;
SS_COORD       CellCol;
SS_COORD       CellRowStart;
SS_COORD       CellColStart;
SS_COORD       CellRowEnd;
SS_COORD       CellColEnd;
LPTSTR         lpData;
LPTSTR         pValStr;
BOOL           CellRange = FALSE;

#ifdef  BUGS
// Bug-002
static TCHAR   Buffer[330];
#else
static TCHAR   Buffer[50 + 1];
#endif

if (!(Formula = SSx_ParseItemCoord(lpSS, Formula, &CellRowStart,
                                   &CellColStart, ValidateCalc)))
   return (NULL);

if (*Formula == ':')
   {
   CellRange = TRUE;
   Formula++;

   if (!(Formula = SSx_ParseItemCoord(lpSS, Formula, &CellRowEnd,
                                      &CellColEnd, ValidateCalc)))
      return (NULL);
   }
else
   {
   CellColEnd = CellColStart;
   CellRowEnd = CellRowStart;
   }

_fmemset(Value, '\0', sizeof(SS_VALUE));
Value->Status = SS_VALUE_STATUS_EMPTY;

for (CellRow = CellRowStart; CellRow <= CellRowEnd; CellRow++)
   for (CellCol = CellColStart; CellCol <= CellColEnd; CellCol++)
      {
      if ((CellCol >= SS_GetColCnt(lpSS)) ||
          (CellRow >= SS_GetRowCnt(lpSS)))
         return (Formula);

      else if (ValidateCalc)
         {
         Calc = (LPSS_CALC)tbGlobalLock(ValidateCalc);

         if (CellCol == Calc->CellCoord.Col && CellRow == Calc->CellCoord.Row)
            {
            tbGlobalUnlock(ValidateCalc);
            return (NULL);
            }

         if (Calc->Ancestors.ItemCnt + 1 > Calc->Ancestors.ItemAllocCnt)
            {
            if (Calc->Ancestors.ItemAllocCnt == 0)
               Calc->Ancestors.hItems = tbGlobalAlloc(GMEM_MOVEABLE |
                                        GMEM_ZEROINIT,
                                        (long)(sizeof(SS_CELLCOORD) *
                                        (Calc->Ancestors.ItemAllocCnt +
                                        SS_CALC_ALLOC_CNT)));
            else
               Calc->Ancestors.hItems = tbGlobalReAlloc(Calc->Ancestors.hItems,
                                        (long)(sizeof(SS_CELLCOORD) *
                                        (Calc->Ancestors.ItemAllocCnt +
                                        SS_CALC_ALLOC_CNT)),
                                        GMEM_MOVEABLE | GMEM_ZEROINIT);

            if (!Calc->Ancestors.hItems)
               {
               tbGlobalUnlock(ValidateCalc);
               return (NULL);
               }

            Calc->Ancestors.ItemAllocCnt += SS_CALC_ALLOC_CNT;
            }

         Ancestors = (LPSS_CELLCOORD)tbGlobalLock(Calc->Ancestors.hItems);

         if (Calc->Ancestors.ItemCnt == 0)
            {
            Ancestors[Calc->Ancestors.ItemCnt].Row = -1;
            Ancestors[Calc->Ancestors.ItemCnt].Col = -1;
            Calc->Ancestors.ItemCnt++;
            }

         Ancestors[Calc->Ancestors.ItemCnt].Row = CellRow;
         Ancestors[Calc->Ancestors.ItemCnt].Col = CellCol;

         Calc->Ancestors.ItemCnt++;
         tbGlobalUnlock(Calc->Ancestors.hItems);

         tbGlobalUnlock(ValidateCalc);
         }

      else
         {
         if (lpCell = SS_LockCellItem(lpSS, CellCol, CellRow))
            {
            if (!(CellType = SS_RetrieveCellType(lpSS, &CellTypeTemp,
                                                 lpCell, CellCol, CellRow)))
               return (NULL);

            if (CellType->Type == SS_TYPE_INTEGER)
               ValueTemp.Type = SS_VALUE_TYPE_LONG;

            else if (CellType->Type == SS_TYPE_FLOAT)
               ValueTemp.Type = SS_VALUE_TYPE_DOUBLE;

            else
               ValueTemp.Type = SS_VALUE_TYPE_STR;

            if (CellRange && ValueTemp.Type == SS_VALUE_TYPE_STR)
               ;

            else
               {
               lpCol = SS_LockColItem(lpSS, CellCol);
               lpRow = SS_LockRowItem(lpSS, CellRow);

               SS_GetDataStruct(lpSS, lpCol, lpRow, lpCell, &Data, CellCol,
                                CellRow);

               if (Data.bDataType)
                  {
                  ValueTemp.Status = SS_VALUE_STATUS_OK;

                  if (CellType->Type == SS_TYPE_FLOAT)
                     {
                     if (Data.bDataType == SS_TYPE_INTEGER)
                        ValueTemp.Val.ValDouble = (double)Data.Data.lValue;

                     else if (Data.bDataType == SS_TYPE_FLOAT)
                        ValueTemp.Val.ValDouble = Data.Data.dfValue;

                     else if (Data.Data.hszData)
                        {
                        lpData = (LPTSTR)tbGlobalLock(Data.Data.hszData);
                        _ftcsncpy(Buffer, lpData,
                                  sizeof(Buffer)/sizeof(TCHAR) - 1);
                        StringToFloat(Buffer, &ValueTemp.Val.ValDouble);
                        tbGlobalUnlock(Data.Data.hszData);
                        }
                     else
                        ValueTemp.Val.ValDouble = 0.0;
                     }

                  else if (CellType->Type == SS_TYPE_INTEGER)
                     {
                     if (Data.bDataType == SS_TYPE_INTEGER)
                        ValueTemp.Val.ValLong = (long)Data.Data.lValue;

                     else if (Data.bDataType == SS_TYPE_FLOAT)
                        ValueTemp.Val.ValLong = (long)Data.Data.dfValue;

                     else if (Data.Data.hszData)
                        {
                        lpData = (LPTSTR)tbGlobalLock(Data.Data.hszData);
                        _ftcsncpy(Buffer, lpData, sizeof(Buffer)/sizeof(TCHAR) - 1);
                        ValueTemp.Val.ValLong = _ttol(Buffer);
                        tbGlobalUnlock(Data.Data.hszData);
                        }
                     else
                        ValueTemp.Val.ValLong = 0;
                     }

                  else
                     {
                     lpData = (LPTSTR)tbGlobalLock(Data.Data.hszData);
                     if (!(ValueTemp.Val.hValStr = GlobalAlloc(GMEM_MOVEABLE |
                                                   GMEM_ZEROINIT,
                                                   (lstrlen(lpData) + 1)
                                                   * sizeof(TCHAR))))
                        {
                        tbGlobalUnlock(hData);
                        return (NULL);
                        }

                     pValStr = (LPTSTR)GlobalLock(ValueTemp.Val.hValStr);
                     lstrcpy(pValStr, lpData);
                     GlobalUnlock(ValueTemp.Val.hValStr);
                     tbGlobalUnlock(Data.Data.hszData);
                     }

                  if (CellRange)
                     {
                     _fmemcpy(&Value1, Value, sizeof(SS_VALUE));
                     SSx_EvalAdd(&Value1, &ValueTemp, Value);
                     }
                  else
                     _fmemcpy(Value, &ValueTemp, sizeof(SS_VALUE));
                  }

               if (lpCol)
                  SS_UnlockColItem(lpSS, CellCol);

               if (lpRow)
                  SS_UnlockRowItem(lpSS, CellRow);
               }

            SS_UnlockCellItem(lpSS, CellCol, CellRow);
            }
         }
      }

return (Formula);
}


static LPCTSTR SSx_ParseItemCoord(lpSS, Formula, CellRow, CellCol,
                                  ValidateCalc)

LPSPREADSHEET  lpSS;
LPCTSTR         Formula;
LPSS_COORD     CellRow;
LPSS_COORD     CellCol;
TBGLOBALHANDLE ValidateCalc;
{
SS_VALUE       ValTemp;
LPCTSTR        FormulaTemp;
BOOL           WildCard = FALSE;
long           Pow;
short          Len;
short          i;

if (*Formula == SS_CALC_WILDCARD)
   {
   WildCard = TRUE;
   Formula++;

   if (ValidateCalc)
      *CellCol = -1;
   else
      *CellCol = lpSS->CurCalcCell.Col;
   }

else
   {
   if (!isalpha(*Formula))
      return (NULL);

   /*
   if (isalpha(*(Formula + 1)))
      {
      *CellCol = ((toupper(Formula[0]) - 'A' + 1) * 26) +
                 (toupper(Formula[1]) - 'A');
      Formula += 2;
      }
   else
      {
      *CellCol = toupper(Formula[0]) - 'A';
      Formula += 1;
      }
   */

   *CellCol = 0;

   for (FormulaTemp = Formula + 1, Len = 1; *FormulaTemp &&
        isalpha(*FormulaTemp); Len++)
      FormulaTemp++;

   for (i = Len - 1, Pow = 1; i >= 0; i--, Pow *= 26)
      (*CellCol) += (toupper(Formula[i]) - 'A' + 1) * Pow;

   Formula = FormulaTemp;
   SS_AdjustCellCoords(lpSS, CellCol, NULL);
   }

if (*Formula == SS_CALC_WILDCARD)
   {
   if (WildCard)
      return (NULL);

   Formula++;

   if (ValidateCalc)
      *CellRow = -1;
   else
      *CellRow = lpSS->CurCalcCell.Row;
   }

else
   {
   if (!isdigit(*Formula))
      return (NULL);

   Formula = SSx_ParseNumber(Formula, &ValTemp);

   if (ValTemp.Status != SS_VALUE_STATUS_OK ||
       ValTemp.Type != SS_VALUE_TYPE_LONG)
      return (NULL);

   *CellRow = (SS_COORD)ValTemp.Val.ValLong;
   SS_AdjustCellCoords(lpSS, NULL, CellRow);
   }

return (Formula);
}


/***********************************************************************
* Name:        SSx_ParseStr - Parse out a string from the formula
*
* Usage:       LPCTSTR SSx_ParseStr(Formula, Value)
*                 Formula - Pointer to current place in formula
*                 Value   - The resulting number to be returned
*
* Description: Parse the string and set the value type to string
*
* Return:      The current formula position after the parsed string.
***********************************************************************/

LPCTSTR SSx_ParseStr(Formula, Value)

LPCTSTR    Formula;
LPSS_VALUE Value;
{
LPTSTR     TempStr;
LPTSTR     pValStr;

if ((TempStr = _ftcschr(Formula, SS_CALC_PRFX_STR)) == NULL)
   return (NULL);

Value->Type = SS_VALUE_TYPE_STR;
Value->Val.hValStr = 0;

if (TempStr == Formula)
   Value->Status = SS_VALUE_STATUS_EMPTY;
else
   {
   Value->Status = SS_VALUE_STATUS_OK;

   if (!(Value->Val.hValStr = GlobalAlloc(GMEM_MOVEABLE | GMEM_ZEROINIT,
                                          ((long)(TempStr - Formula) + 1L)
                                          * sizeof(TCHAR))))
      return (NULL);

   pValStr = (LPTSTR)GlobalLock(Value->Val.hValStr);
   _ftcsncpy(pValStr, Formula, TempStr - Formula);
   pValStr[TempStr - Formula] = '\0';
   GlobalUnlock(Value->Val.hValStr);
   }

return (TempStr + 1);
}


/***********************************************************************
* Name:        SSx_EvalAdd - Add 2 values together
*
* Usage:       void SSx_EvalAdd(Val1, Val2, Result)
*                 Val1   - The first Value to be added
*                 Val2   - The second value to be added
*                 Result - The result of the addition
*
* Description: Add two numbers together.  If one of the two is a
*              float, the result will be a float. Otherwise the
*              result will be an integer.
*
* Return:      Void
***********************************************************************/

void SSx_EvalAdd(Val1, Val2, Result)

LPSS_VALUE Val1;
LPSS_VALUE Val2;
LPSS_VALUE Result;
{
if (Val1->Type == SS_VALUE_TYPE_STR || Val2->Type == SS_VALUE_TYPE_STR)
   Result->Status = SS_VALUE_STATUS_ERROR;

else
   {
   if (Val1->Status == SS_VALUE_STATUS_OK ||
       Val2->Status == SS_VALUE_STATUS_OK)
      Result->Status = SS_VALUE_STATUS_OK;
   else if (Val1->Status == SS_VALUE_STATUS_NONE ||
            Val2->Status == SS_VALUE_STATUS_NONE)
      Result->Status = SS_VALUE_STATUS_NONE;
   else
      Result->Status = SS_VALUE_STATUS_EMPTY;

   if (Val1->Type == SS_VALUE_TYPE_DOUBLE ||
       Val2->Type == SS_VALUE_TYPE_DOUBLE)
      {
      Result->Type = SS_VALUE_TYPE_DOUBLE;
      Result->Val.ValDouble = 0.0;

      if (Result->Status == SS_VALUE_STATUS_OK)
         {
         if (Val1->Status == SS_VALUE_STATUS_OK)
            Result->Val.ValDouble = (Val1->Type == SS_VALUE_TYPE_DOUBLE ?
               Val1->Val.ValDouble : (double)Val1->Val.ValLong);

         if (Val2->Status == SS_VALUE_STATUS_OK)
            Result->Val.ValDouble += (Val2->Type == SS_VALUE_TYPE_DOUBLE ?
               Val2->Val.ValDouble : (double)Val2->Val.ValLong);
         }
      }

   else
      {
      Result->Val.ValLong = 0;
      Result->Type = SS_VALUE_TYPE_LONG;

      if (Result->Status == SS_VALUE_STATUS_OK)
         {
         if (Val1->Status == SS_VALUE_STATUS_OK)
            Result->Val.ValLong = Val1->Val.ValLong;

         if (Val2->Status == SS_VALUE_STATUS_OK)
            Result->Val.ValLong += Val2->Val.ValLong;
         }
      }
   }
}


/***********************************************************************
* Name:        SSx_EvalDivide - Divide value 1 by value 2
*
* Usage:       void SSx_EvalDivide(Val1, Val2, Result)
*                 Val1   - Dividend (The number to be divided)
*                 Val2   - Divisor (The dividing number)
*                 Result - Result of the division
*
* Description: Divide value 1 by value 2.  If one of the two is a
*              float, the result will be a float. Otherwise the
*              result will be an integer.
*
* Return:      Void
***********************************************************************/

void SSx_EvalDivide(Val1, Val2, Result)

LPSS_VALUE Val1;
LPSS_VALUE Val2;
LPSS_VALUE Result;
{
if (Val1->Type == SS_VALUE_TYPE_STR || Val2->Type == SS_VALUE_TYPE_STR)
   Result->Status = SS_VALUE_STATUS_ERROR;

else
   {
   if (Val1->Status == SS_VALUE_STATUS_OK &&
       Val2->Status == SS_VALUE_STATUS_OK)
      Result->Status = SS_VALUE_STATUS_OK;
   else if (Val1->Status == SS_VALUE_STATUS_NONE ||
            Val2->Status == SS_VALUE_STATUS_NONE)
      Result->Status = SS_VALUE_STATUS_NONE;
   else
      Result->Status = SS_VALUE_STATUS_EMPTY;

   if (Val1->Type == SS_VALUE_TYPE_DOUBLE ||
       Val2->Type == SS_VALUE_TYPE_DOUBLE)
      {
      Result->Type = SS_VALUE_TYPE_DOUBLE;
      Result->Val.ValDouble = 0.0;

      if (Result->Status == SS_VALUE_STATUS_OK)
         {
         if (Val1->Type == SS_VALUE_TYPE_DOUBLE)
            Result->Val.ValDouble = Val1->Val.ValDouble;
         else
            Result->Val.ValDouble = (double)Val1->Val.ValLong;

         if (Val2->Type == SS_VALUE_TYPE_DOUBLE)
            {
            if (Val2->Val.ValDouble == 0.0)
               Result->Status = SS_VALUE_STATUS_EMPTY;
            else
               Result->Val.ValDouble /= Val2->Val.ValDouble;
            }
         else
            {
            if (Val2->Val.ValLong == 0)
               Result->Status = SS_VALUE_STATUS_EMPTY;
            else
               Result->Val.ValDouble /= (double)Val2->Val.ValLong;
            }
         }
      }

   else
      {
      Result->Val.ValLong = 0;
      Result->Type = SS_VALUE_TYPE_LONG;

      if (Result->Status == SS_VALUE_STATUS_OK)
         {
         if (Val2->Val.ValLong == 0)
            Result->Status = SS_VALUE_STATUS_EMPTY;
         else
            if (Val1->Val.ValLong % Val2->Val.ValLong)
               {
               Result->Type = SS_VALUE_TYPE_DOUBLE;
               Result->Val.ValDouble = (double)Val1->Val.ValLong /
                                       (double)Val2->Val.ValLong;
               }
            else
               Result->Val.ValLong = Val1->Val.ValLong / Val2->Val.ValLong;
         }
      }
   }
}


/***********************************************************************
* Name:        SSx_EvalMultiply - Multiply value 1 and value 2
*
* Usage:       void SSx_EvalMultiply(Val1, Val2, Result)
*                 Val1   - The first value to be multiplied
*                 Val2   - The second value to be multiplied
*                 Result - Result of the multiplication
*
* Description: Multiply value 1 to value 2.  If one of the two is
*              a float, the result will be a float. Otherwise the
*              result will be an integer.
*
* Return:      Void
***********************************************************************/

void SSx_EvalMultiply(Val1, Val2, Result)

LPSS_VALUE Val1;
LPSS_VALUE Val2;
LPSS_VALUE Result;
{
if (Val1->Type == SS_VALUE_TYPE_STR || Val2->Type == SS_VALUE_TYPE_STR)
   Result->Status = SS_VALUE_STATUS_ERROR;

else
   {
   if (Val1->Status == SS_VALUE_STATUS_OK &&
       Val2->Status == SS_VALUE_STATUS_OK)
      Result->Status = SS_VALUE_STATUS_OK;
   else if (Val1->Status == SS_VALUE_STATUS_NONE ||
            Val2->Status == SS_VALUE_STATUS_NONE)
      Result->Status = SS_VALUE_STATUS_NONE;
   else
      Result->Status = SS_VALUE_STATUS_EMPTY;

   if (Val1->Type == SS_VALUE_TYPE_DOUBLE ||
       Val2->Type == SS_VALUE_TYPE_DOUBLE)
      {
      Result->Type = SS_VALUE_TYPE_DOUBLE;
      Result->Val.ValDouble = 0.0;

      if (Result->Status == SS_VALUE_STATUS_OK)
         {
         if (Val1->Type == SS_VALUE_TYPE_DOUBLE)
            Result->Val.ValDouble = Val1->Val.ValDouble;
         else
            Result->Val.ValDouble = (double)Val1->Val.ValLong;

#ifdef  BUGS
// Bug-007
         {
         long double Val;   

         if (Val2->Type == SS_VALUE_TYPE_DOUBLE)
            Val = Result->Val.ValDouble * Val2->Val.ValDouble;
         else
            Val = Result->Val.ValDouble * (double)Val2->Val.ValLong;

         if (Val != 0.0 && (fabsl(Val) > 1.7E+308 || fabsl(Val) < 1.7E-308))
            Result->Status = SS_VALUE_STATUS_ERROR;
         else
            Result->Val.ValDouble = (double)Val;
         }
#else
         if (Val2->Type == SS_VALUE_TYPE_DOUBLE)
            Result->Val.ValDouble *= Val2->Val.ValDouble;
         else
            Result->Val.ValDouble *= (double)Val2->Val.ValLong;
#endif

         }
      }

   else
      {

#ifdef  BUGS
// Bug-007
      double Val = (double)Val1->Val.ValLong * (double)Val2->Val.ValLong;

      if ( Val > 2147483647 || Val < -2147483647 )  
         {
         Result->Type = SS_VALUE_TYPE_DOUBLE;             
      
         if (Result->Status == SS_VALUE_STATUS_OK)
             Result->Val.ValDouble = (double)Val1->Val.ValLong * (double)Val2->Val.ValLong;
         } 
      else 
         {
         Result->Type = SS_VALUE_TYPE_LONG;

         if (Result->Status == SS_VALUE_STATUS_OK)
            Result->Val.ValLong = Val1->Val.ValLong * Val2->Val.ValLong;
         }
#else
      Result->Type = SS_VALUE_TYPE_LONG;

      if (Result->Status == SS_VALUE_STATUS_OK)
         Result->Val.ValLong = Val1->Val.ValLong * Val2->Val.ValLong;
#endif
      }
   }
}


/***********************************************************************
* Name:        SSx_EvalSubtract - Subtract value 2 from value 1
*
* Usage:       void SSx_EvalSubtract(Val1, Val2, Result)
*                 Val1   - The first value
*                 Val2   - The value to be subracted from the first
*                 Result - The result of the subtraction
*
* Description: Subtraction value 2 from value 1.  If one of the two
*              is a float, the result will be a float. Otherwise the
*              result will be an integer.
*
* Return:      Void
***********************************************************************/

void SSx_EvalSubtract(Val1, Val2, Result)

LPSS_VALUE Val1;
LPSS_VALUE Val2;
LPSS_VALUE Result;
{
if (Val1->Type == SS_VALUE_TYPE_STR || Val2->Type == SS_VALUE_TYPE_STR)
   Result->Status = SS_VALUE_STATUS_ERROR;

else
   {
   if (Val1->Status == SS_VALUE_STATUS_OK)
      Result->Status = SS_VALUE_STATUS_OK;
   else if (Val1->Status == SS_VALUE_STATUS_NONE)
      Result->Status = SS_VALUE_STATUS_NONE;
   else
      Result->Status = SS_VALUE_STATUS_EMPTY;

   if (Val1->Type == SS_VALUE_TYPE_DOUBLE ||
       Val2->Type == SS_VALUE_TYPE_DOUBLE)
      {
      Result->Type = SS_VALUE_TYPE_DOUBLE;
      Result->Val.ValDouble = 0.0;

      if (Result->Status == SS_VALUE_STATUS_OK)
         {
         if (Val1->Type == SS_VALUE_TYPE_DOUBLE)
            Result->Val.ValDouble = Val1->Val.ValDouble;
         else
            Result->Val.ValDouble = (double)Val1->Val.ValLong;

         if (Val2->Status == SS_VALUE_STATUS_OK)
            {
            if (Val2->Type == SS_VALUE_TYPE_DOUBLE)
               Result->Val.ValDouble -= Val2->Val.ValDouble;
            else
               Result->Val.ValDouble -= (double)Val2->Val.ValLong;
            }
         }
      }

   else
      {
      Result->Val.ValLong = 0;
      Result->Type = SS_VALUE_TYPE_LONG;

      if (Result->Status == SS_VALUE_STATUS_OK)
         {
         Result->Val.ValLong = Val1->Val.ValLong;

         if (Val2->Status == SS_VALUE_STATUS_OK)
            Result->Val.ValLong -= Val2->Val.ValLong;
         }
      }
   }
}


/***********************************************************************
* Name:        SSx_EvalExponential - Raise value 1 to the power of
*                                     value 2
*
* Usage:       void SSx_EvalExponential(Val1, Val2, Result)
*                 Val1   - The number to be raised
*                 Val2   - The power of value 1
*                 Result - The result of the exponentiation
*
* Description: Raise value 1 to the power of value 2.  If one of the
*              two is a float, the result will be a float. Otherwise
*              the result will be an integer.
*
* Return:      Void
***********************************************************************/

void SSx_EvalExponential(Val1, Val2, Result)

LPSS_VALUE Val1;
LPSS_VALUE Val2;
LPSS_VALUE Result;
{
long       i;



if (Val1->Type == SS_VALUE_TYPE_STR || Val2->Type == SS_VALUE_TYPE_STR)
   Result->Status = SS_VALUE_STATUS_ERROR;

else
   {
   if (Val1->Status == SS_VALUE_STATUS_OK &&
       Val2->Status == SS_VALUE_STATUS_OK)
      Result->Status = SS_VALUE_STATUS_OK;
   else if (Val1->Status == SS_VALUE_STATUS_NONE ||
            Val2->Status == SS_VALUE_STATUS_NONE)
      Result->Status = SS_VALUE_STATUS_NONE;
   else
      Result->Status = SS_VALUE_STATUS_EMPTY;

   if (Val1->Type == SS_VALUE_TYPE_DOUBLE ||
       Val2->Type == SS_VALUE_TYPE_DOUBLE)
      {
      Result->Type = SS_VALUE_TYPE_DOUBLE;
      Result->Val.ValDouble = 0.0;

      if (Result->Status == SS_VALUE_STATUS_OK)
         {
         if (Val1->Type == SS_VALUE_TYPE_DOUBLE)
            Result->Val.ValDouble = Val1->Val.ValDouble;
         else
            Result->Val.ValDouble = (double)Val1->Val.ValLong;

#ifdef  BUGS
// Bug-008
         errno = 0;       // EZERO;
#endif

         if (Val2->Type == SS_VALUE_TYPE_DOUBLE)
            Result->Val.ValDouble = pow(Result->Val.ValDouble,
                                        Val2->Val.ValDouble);
         else
            Result->Val.ValDouble = pow(Result->Val.ValDouble,
                                       (double)Val2->Val.ValLong);
#ifdef  BUGS
// Bug-008
         {
         if (errno ==  EDOM || errno == ERANGE)
            Result->Status = SS_VALUE_STATUS_ERROR;
         }
#endif

         }
      }

   else
      {
      Result->Val.ValLong = Val1->Val.ValLong;
      Result->Type = SS_VALUE_TYPE_LONG;

      if (Result->Status == SS_VALUE_STATUS_OK)

#ifdef  BUGS
// Bug-009
         {
         BOOL bNegative = FALSE;

         if ( !Val2->Val.ValLong ) {
            if (Val1->Val.ValLong) 
              Result->Val.ValLong = 1;
            else
              Result->Status = SS_VALUE_STATUS_ERROR;
            return;
         }

         if (Val2->Val.ValLong < 0 ) 
            {
            Val2->Val.ValLong *= -1; 
            bNegative = TRUE;
            }
#endif
         for (i = 1; i < Val2->Val.ValLong; i++)
            Result->Val.ValLong *= Val1->Val.ValLong;

#ifdef  BUGS
// Bug-009
         if (bNegative)
            {
            Result->Type = SS_VALUE_TYPE_DOUBLE;
            Result->Val.ValDouble = 1.0 / (double)Result->Val.ValLong;
            }
         }
#endif
      }
   }
}


/***********************************************************************
* Name:        SSx_EvalEqual - Perform a relational operation on
*                               2 values
*
* Usage:       void SSx_EvalEqual(Operation, Val1, Val2, Result)
*                 Operation - The operation to be performed. The
*                             following operations can be performed:
*                                OP_NOTEQUAL     - Not Equal
*                                OP_GREATER      - Greater than
*                                OP_GREATEREQUAL - Greater than or equal
*                                OP_LESS         - Less than
*                                OP_LESSEQUAL    - Less than or equal
*                                OP_EQUAL        - Equal
*                 Val1      - First value
*                 Val2      - Second value
*                 Result    - The result of the operation
*
* Description: Perform relational operation on value 1 and value 2.
*              TRUE or FALSE is stored in (Result.Val.ValLong).
*
* Return:      Void
***********************************************************************/

void SSx_EvalEqual(Operation, Val1, Val2, Result)

short      Operation;
LPSS_VALUE Val1;
LPSS_VALUE Val2;
LPSS_VALUE Result;
{
LPTSTR     pValStr1;
LPTSTR     pValStr2;
double     TempDouble1;
double     TempDouble2;

Result->Type = SS_VALUE_TYPE_LONG;

if (Val1->Status == SS_VALUE_STATUS_OK && Val2->Status == SS_VALUE_STATUS_OK)
   {
   Result->Status = SS_VALUE_STATUS_OK;
   Result->Val.ValLong = FALSE;

   if ((Val1->Type == SS_VALUE_TYPE_DOUBLE ||
        Val2->Type == SS_VALUE_TYPE_DOUBLE) &&
        Val1->Type != SS_VALUE_TYPE_STR && Val2->Type != SS_VALUE_TYPE_STR)
      {
      if (Val1->Type == SS_VALUE_TYPE_DOUBLE)
         TempDouble1 = Val1->Val.ValDouble;
      else
         TempDouble1 = (double)Val1->Val.ValLong;

      if (Val2->Type == SS_VALUE_TYPE_DOUBLE)
         TempDouble2 = Val2->Val.ValDouble;
      else
         TempDouble2 = (double)Val2->Val.ValLong;

      switch (Operation)
         {
         case OP_NOTEQUAL:
            if (TempDouble1 != TempDouble2)
               Result->Val.ValLong = TRUE;

            break;

         case OP_GREATER:
            if (TempDouble1 > TempDouble2)
               Result->Val.ValLong = TRUE;

            break;

         case OP_GREATEREQUAL:
            if (TempDouble1 >= TempDouble2)
               Result->Val.ValLong = TRUE;

            break;

         case OP_LESS:
            if (TempDouble1 < TempDouble2)
               Result->Val.ValLong = TRUE;

            break;

         case OP_LESSEQUAL:
            if (TempDouble1 <= TempDouble2)
               Result->Val.ValLong = TRUE;

            break;

         case OP_EQUAL:
            if (TempDouble1 == TempDouble2)
               Result->Val.ValLong = TRUE;

            break;

         default:
            break;
         }
      }

   else if (Val1->Type == SS_VALUE_TYPE_LONG &&
            Val2->Type == SS_VALUE_TYPE_LONG)
      {
      switch (Operation)
         {
         case OP_NOTEQUAL:
            if (Val1->Val.ValLong != Val2->Val.ValLong)
               Result->Val.ValLong = TRUE;

            break;

         case OP_GREATER:
            if (Val1->Val.ValLong > Val2->Val.ValLong)
               Result->Val.ValLong = TRUE;

            break;

         case OP_GREATEREQUAL:
            if (Val1->Val.ValLong >= Val2->Val.ValLong)
               Result->Val.ValLong = TRUE;

            break;

         case OP_LESS:
            if (Val1->Val.ValLong < Val2->Val.ValLong)
               Result->Val.ValLong = TRUE;

            break;

         case OP_LESSEQUAL:
            if (Val1->Val.ValLong <= Val2->Val.ValLong)
               Result->Val.ValLong = TRUE;

            break;

         case OP_EQUAL:
            if (Val1->Val.ValLong == Val2->Val.ValLong)
               Result->Val.ValLong = TRUE;

            break;

         default:
            break;
         }
      }

   else if (Val1->Type == SS_VALUE_TYPE_STR && Val2->Type == SS_VALUE_TYPE_STR)
      {
      pValStr1 = (LPTSTR)GlobalLock(Val1->Val.hValStr);
      pValStr2 = (LPTSTR)GlobalLock(Val2->Val.hValStr);

      switch (Operation)
         {
         case OP_NOTEQUAL:
            if (lstrcmpi(pValStr1, pValStr2) != 0)
               Result->Val.ValLong = TRUE;

            break;

         case OP_GREATER:
            if (lstrcmpi(pValStr1, pValStr2) > 0)
               Result->Val.ValLong = TRUE;

            break;

         case OP_GREATEREQUAL:
            if (lstrcmpi(pValStr1, pValStr2) >= 0)
               Result->Val.ValLong = TRUE;

            break;

         case OP_LESS:
            if (lstrcmpi(pValStr1, pValStr2) < 0)
               Result->Val.ValLong = TRUE;

            break;

         case OP_LESSEQUAL:
            if (lstrcmpi(pValStr1, pValStr2) <= 0)
               Result->Val.ValLong = TRUE;

            break;

         case OP_EQUAL:
            if (lstrcmpi(pValStr1, pValStr2) == 0)
               Result->Val.ValLong = TRUE;

            break;

         default:
            break;
         }

      GlobalUnlock(Val1->Val.hValStr);
      GlobalUnlock(Val2->Val.hValStr);
      }
   else
      Result->Status = SS_VALUE_STATUS_ERROR;
   }
else if (Val1->Status == SS_VALUE_STATUS_NONE ||
         Val2->Status == SS_VALUE_STATUS_NONE)
   Result->Status = SS_VALUE_STATUS_NONE;
else
   Result->Status = SS_VALUE_STATUS_EMPTY;
}


/***********************************************************************
* Name:        SSx_EvalAnd - Perform a logical AND on two values
*
* Usage:       void SSx_EvalAnd(Val1, Val2, Result)
*                 Val1      - First value
*                 Val2      - Second value
*                 Result    - The result of the operation
*
* Description: Perform a logical AND on value 1 and value 2.
*              TRUE or FALSE is stored in (Result.Val.ValLong).
*
* Return:      Void
***********************************************************************/

void SSx_EvalAnd(Val1, Val2, Result)

LPSS_VALUE Val1;
LPSS_VALUE Val2;
LPSS_VALUE Result;
{
Result->Type = SS_VALUE_TYPE_LONG;

if (Val1->Status == SS_VALUE_STATUS_OK && Val2->Status == SS_VALUE_STATUS_OK)
   {
   Result->Status = SS_VALUE_STATUS_OK;

   if (Val1->Type == SS_VALUE_TYPE_DOUBLE)
      Result->Val.ValLong = Val1->Val.ValDouble == 0.0 ? FALSE : TRUE;
   else if (Val1->Type == SS_VALUE_TYPE_LONG)
      Result->Val.ValLong = Val1->Val.ValLong == 0 ? FALSE : TRUE;
   else
      Result->Val.ValLong = TRUE;

   if (Result->Val.ValLong == TRUE)
      {
      if (Val2->Type == SS_VALUE_TYPE_DOUBLE)
         Result->Val.ValLong = Val2->Val.ValDouble == 0.0 ? FALSE : TRUE;
      else if (Val1->Type == SS_VALUE_TYPE_LONG)
         Result->Val.ValLong = Val2->Val.ValLong == 0 ? FALSE : TRUE;
      else
         Result->Val.ValLong = TRUE;
      }
   }
else if (Val1->Status == SS_VALUE_STATUS_NONE ||
         Val2->Status == SS_VALUE_STATUS_NONE)
   Result->Status = SS_VALUE_STATUS_NONE;
else
   Result->Status = SS_VALUE_STATUS_EMPTY;
}


/***********************************************************************
* Name:        SSx_EvalOr - Perform a logical OR on two values
*
* Usage:       void SSx_EvalOr(Val1, Val2, Result)
*                 Val1      - First value
*                 Val2      - Second value
*                 Result    - The result of the operation
*
* Description: Perform a logical OR on value 1 and value 2.
*              TRUE or FALSE is stored in (Result.Val.ValLong).
*
* Return:      Void
***********************************************************************/

void SSx_EvalOr(Val1, Val2, Result)

LPSS_VALUE Val1;
LPSS_VALUE Val2;
LPSS_VALUE Result;
{
Result->Type = SS_VALUE_TYPE_LONG;

if (Val1->Status == SS_VALUE_STATUS_OK && Val2->Status == SS_VALUE_STATUS_OK)
   {
   Result->Status = SS_VALUE_STATUS_OK;

   if (Val1->Type == SS_VALUE_TYPE_DOUBLE)
      Result->Val.ValLong = Val1->Val.ValDouble == 0.0 ? FALSE : TRUE;
   else if (Val1->Type == SS_VALUE_TYPE_LONG)
      Result->Val.ValLong = Val1->Val.ValLong == 0 ? FALSE : TRUE;
   else
      Result->Val.ValLong = TRUE;

   if (Result->Val.ValLong == FALSE)
      {
      if (Val2->Type == SS_VALUE_TYPE_DOUBLE)
         Result->Val.ValLong = Val2->Val.ValDouble == 0.0 ? FALSE : TRUE;
      else if (Val2->Type == SS_VALUE_TYPE_LONG)
         Result->Val.ValLong = Val2->Val.ValLong == 0 ? FALSE : TRUE;
      else
         Result->Val.ValLong = TRUE;
      }
   }
else if (Val1->Status == SS_VALUE_STATUS_NONE ||
         Val2->Status == SS_VALUE_STATUS_NONE)
   Result->Status = SS_VALUE_STATUS_NONE;
else
   Result->Status = SS_VALUE_STATUS_EMPTY;
}


/***********************************************************************
* Name:        SSx_ParseFunction - Parse a function name from the
*                                  formula string and execute it
*
* Usage:       LPCTSTR SSx_ParseFunction(Formula, Result, datatbl,
                                       deftbl, dataid)
*                 Formula - The formula to be used
*                 Result  - The result of the executed function
*                 datatbl - Data table
*                 deftbl  - Definition table
*                 dataid  - Master Data id
*
* Description: Parse a function name from the formula.  Check to see
*              if the function name is valid and perform that function
*
* Return:      The current formula position after the parsed function
***********************************************************************/

LPCTSTR SSx_ParseFunction(lpSS, Formula, Result, ValidateCalc)

LPSPREADSHEET   lpSS;
LPCTSTR         Formula;
LPSS_VALUE      Result;
TBGLOBALHANDLE  ValidateCalc;
{
SS_CALCFUNCTION CalcFunction;
SS_VALUE        Values[SS_CALC_MAXPARAMS];
LPCTSTR         StrAt;
LPCTSTR         lpszFunction;
short           ParamCnt;
short           dFunctionLen;
short           i;
BOOL            fRet;

StrAt =  Formula;

while (*StrAt != '\0' && *StrAt != '(')
   StrAt++;

if (*StrAt)
   {
   lpszFunction = Formula;
   dFunctionLen = (short)(StrAt - Formula);
   if (fRet = SSx_GetFunction(lpSS, &CalcFunction, lpszFunction,
                              dFunctionLen))
      {
      ParamCnt = CalcFunction.ParameterCnt;

      if (Formula = SSx_CalcGetParms(lpSS, &ParamCnt, StrAt + 1,
                                     Values, ValidateCalc))
         {
#if defined(SS_VB) || defined(SS_OCX)
         if (fRet == 2)                // Custom function
            fRet = (CalcFunction.Function)(lpSS->lpBook->hWnd, lpszFunction,
                                           dFunctionLen, Result,
                                           (LPSS_VALUE)Values, ParamCnt);
         else
            fRet = (CalcFunction.Function)(lpSS->lpBook->hWnd, Result,
                                           (LPSS_VALUE)Values, ParamCnt);
#else
         fRet = (CalcFunction.Function)(Result, (LPSS_VALUE)Values, ParamCnt);
#endif

         for (i = 0; i < ParamCnt; i++)
            SS_FreeValue(&Values[i]);

         if (!fRet)
            Formula = NULL;
         }

      if (Formula == NULL)
         Result->Status = SS_VALUE_STATUS_ERROR;

      return (Formula);
      }
   }

Result->Status = SS_VALUE_STATUS_ERROR;
return (NULL);
}


static BOOL SSx_GetFunction(lpSS, CalcFunction, FunctionName, Len)

LPSPREADSHEET     lpSS;
LPSS_CALCFUNCTION CalcFunction;
LPCTSTR           FunctionName;
short             Len;
{
LPSS_CALCFUNCTION CalcFunctions;
LPTSTR            Name;
short             FuncCnt;
short             i;

FuncCnt = SSx_CalcFunctionsCnt();
for (i = 0; i < FuncCnt; i++)
   if (_ftcsnicmp(SS_CalcFunctions[i].Name, FunctionName, Len) == 0 &&
       SS_CalcFunctions[i].Name[Len] == '\0')
      {
      CalcFunction->Function = SS_CalcFunctions[i].Function;
      CalcFunction->ParameterCnt = SS_CalcFunctions[i].ParameterCnt;
      return (TRUE);
      }

CalcFunctions = (LPSS_CALCFUNCTION)tbGlobalLock(lpSS->hCalcFunctions);

for (i = 0; i < lpSS->CalcFunctionsCnt; i++)
   {
   if (CalcFunctions[i].hName)
      {
      Name = (LPTSTR)tbGlobalLock(CalcFunctions[i].hName);
      if (_ftcsnicmp(Name, FunctionName, Len) == 0 && Name[Len] == '\0')
         {
         CalcFunction->Function = CalcFunctions[i].Function;
         CalcFunction->ParameterCnt = CalcFunctions[i].ParameterCnt;
         tbGlobalUnlock(CalcFunctions[i].hName);
         tbGlobalUnlock(lpSS->hCalcFunctions);
         return (2);
         }

      tbGlobalUnlock(CalcFunctions[i].hName);
      }
   }

tbGlobalUnlock(lpSS->hCalcFunctions);
return (FALSE);
}


/***********************************************************************
* Name:        SSx_CalcGetParm - Get a parameter from the formula
*                                 and calculate it
*
* Usage:       LPCTSTR SSx_CalcGetParm(Formula, Result, last_parm,
*                                      datatbl, deftbl, dataid)
*                 Formula   - Formula to be used
*                 Result    - The result of the executed function
*                 last_parm - TRUE if last parameter, FALSE otherwise
*                 datatbl   - Data table
*                 deftbl    - Definition table
*                 dataid    - Master Data id
*
* Description: Get a parameter from the formula and calculate it as
*              if it were a new formula.
*
* Return:      The current formula position after the parsed parameter
***********************************************************************/

LPCTSTR SSx_CalcGetParm(lpSS, Formula, Result, LastParm, IsLastParm,
                       ValidateCalc)

LPSPREADSHEET  lpSS;
LPCTSTR        Formula;
LPSS_VALUE     Result;
BOOL           LastParm;
BOOL FAR      *IsLastParm;
TBGLOBALHANDLE ValidateCalc;
{
Formula = SSx_Calculate(lpSS, Formula, Result, ValidateCalc);

if (Result->Status == SS_VALUE_STATUS_ERROR || !Formula)
   Formula = NULL;

else if (IsLastParm)
   {
   if (*(Formula - 1) == ')')
      *IsLastParm = TRUE;
   else
      *IsLastParm = FALSE;
   }

else if ((!LastParm && *(Formula - 1) != ',') ||
         (LastParm && *(Formula - 1) != ')'))
   Formula = NULL;

return (Formula);
}


static LPCTSTR SSx_CalcGetParms(lpSS, ParamCnt, Formula, Results,
                                ValidateCalc)

LPSPREADSHEET  lpSS;
short FAR     *ParamCnt;
LPCTSTR        Formula;
LPSS_VALUE     Results;
TBGLOBALHANDLE ValidateCalc;
{
BOOL           IsLastParam = FALSE;
short          i;

if (*ParamCnt == SS_CALC_VARPARAMS)
   {
   for (*ParamCnt = 0; *ParamCnt < SS_CALC_MAXPARAMS && !IsLastParam;
        (*ParamCnt)++)
      if ((Formula = SSx_CalcGetParm(lpSS, Formula,
                                     &Results[*ParamCnt], FALSE,
                                     &IsLastParam, ValidateCalc)) == NULL)
         return (NULL);
   }

else
   for (i = 0; i < *ParamCnt; i++)
      if ((Formula = SSx_CalcGetParm(lpSS, Formula, &Results[i],
                                     (BOOL)(i == *ParamCnt - 1 ? TRUE : FALSE),
                                     NULL, ValidateCalc)) == NULL)
         return (NULL);

return (Formula);
}


/***********************************************************************
* Name:        SS_RoundUp - Round up a value to the specified precision
*
* Usage:       double SS_RoundUp(Val, precision)
*                 Val       - Value to be rounded up
*                 Precision - < 0 - Rounded up to the left of decimal
*                             > 0 - Rounded up to the right of decimal
*
* Description: Round up a number to the specified precision.
*
* Return:      The value rounded up
***********************************************************************/

double SS_RoundUp(Val, Precision)

double        Val;
long          Precision;
{
double        Fraction;
double        Power;

static double Integer;

Power = pow(10.0, (double)Precision);

Val *= Power;

Fraction = modf(Val, &Integer);

if (Fraction > 0.0)
   Integer++;

Val = Integer / Power;

return (Val);
}


/***********************************************************************
* Name:        SS_Round - Round a value to the specified precision
*
* Usage:       double SS_Round(Val, Precision)
*                 Val       - Value to be rounded
*                 Precision - < 0 - Rounded to the left of decimal
*                             > 0 - Rounded to the right of decimal
*
* Description: Round a number to the specified precision.  If the
*              fraction >= 0.5, the value is rounded up, otherwise
*              it is rounded down.
*
* Return:      The rounded value
***********************************************************************/

#if 0
double SS_Round(double Val, long Precision)
{
double Power;

Power = pow(10.0, (double)Precision);
Val = ((double)(long)((Val + (0.5 / Power)) * Power)) / Power;

return (Val);
}
#endif


/*
double SS_Round2(Val, Precision)

double        Val;
long          Precision;
{
double        Fraction;
double        Power;

static double Integer;

Power = pow(10.0, (double)Precision);

Val *= Power;

Fraction = modf(Val, &Integer);

if (Fraction >= 0.5)
   Integer++;

Val = Integer / Power;

return (Val);
}
*/


/***********************************************************************
* Name:        SS_Truncate - Truncate a value to the specified precision
*
* Usage:       double SS_Truncate(Val, Precision)
*                 Val       - Value to be truncated
*                 Precision - < 0 - Truncated to the left of decimal
*                             > 0 - Truncated to the right of decimal
*
* Description: Truncate a number to the specified precision
*
* Return:      The truncated value
***********************************************************************/

double SS_Truncate(Val, Precision)

double        Val;
long          Precision;
{
double        Power;

static double Integer;

Power = pow(10.0, (double)Precision);

Val *= Power;

modf(Val, &Integer);

Val = Integer / Power;

return (Val);
}


/***********************************************************************
* Name:        SSx_SkipBlanks - Skip blanks within a string
*
* Usage:       LPCTSTR SSx_SkipBlanks(Str)
*                 Str - The string to be processed
*
* Description: Skip any blanks from the current string position.
*
* Return:      The place in the string following any blanks
***********************************************************************/

LPCTSTR SSx_SkipBlanks(Str)

LPCTSTR Str;
{
while (*Str == ' ')
   Str++;

return (Str);
}


BOOL SSx_CalcTableAdd(lpSS, hCalc)

LPSPREADSHEET    lpSS;
TBGLOBALHANDLE   hCalc;
{
LPTBGLOBALHANDLE CalcTable;

if (lpSS->CalcTableCnt + 1 > lpSS->CalcTableAllocCnt)
   {
   if (lpSS->CalcTableAllocCnt == 0)
      lpSS->hCalcTable = tbGlobalAlloc(GMEM_MOVEABLE | GMEM_ZEROINIT,
                                              (long)(sizeof(TBGLOBALHANDLE) *
                                              (lpSS->CalcTableAllocCnt +
                                              SS_CALC_ALLOC_CNT)));
   else
      lpSS->hCalcTable = tbGlobalReAlloc(lpSS->hCalcTable,
                                               (long)(sizeof(TBGLOBALHANDLE) *
                                               (lpSS->CalcTableAllocCnt +
                                               SS_CALC_ALLOC_CNT)),
                                               GMEM_MOVEABLE | GMEM_ZEROINIT);

   if (!lpSS->hCalcTable)
      return (FALSE);

   lpSS->CalcTableAllocCnt += SS_CALC_ALLOC_CNT;
   }

CalcTable = (LPTBGLOBALHANDLE)tbGlobalLock(lpSS->hCalcTable);

CalcTable[lpSS->CalcTableCnt] = hCalc;
lpSS->CalcTableCnt++;

tbGlobalUnlock(lpSS->hCalcTable);
return (1);
}


BOOL SSx_CalcTableDelete(lpSS, CellCoord)

LPSPREADSHEET    lpSS;
LPSS_CELLCOORD   CellCoord;
{
LPTBGLOBALHANDLE CalcTable;
LPSS_CALC        CalcTemp;
BOOL             Ret = 0;
short            i;
short            j;

if (lpSS->CalcTableCnt)
   {
   CalcTable = (LPTBGLOBALHANDLE)tbGlobalLock(lpSS->hCalcTable);

   for (i = 0; i < lpSS->CalcTableCnt; i++)
      {
      CalcTemp = (LPSS_CALC)tbGlobalLock(CalcTable[i]);
      if (_fmemcmp(&CalcTemp->CellCoord, CellCoord, sizeof(SS_CELLCOORD)) == 0)
         {
         tbGlobalUnlock(CalcTable[i]);
         for (j = i; j < lpSS->CalcTableCnt - 1; j++)
            _fmemcpy(&CalcTable[j], &CalcTable[j + 1], sizeof(TBGLOBALHANDLE));

         _fmemset(&CalcTable[j], '\0', sizeof(TBGLOBALHANDLE));
         lpSS->CalcTableCnt--;
         Ret = TRUE;
         break;
         }
      else
         tbGlobalUnlock(CalcTable[i]);
      }

   tbGlobalUnlock(lpSS->hCalcTable);
   }

return (Ret);
}


BOOL DLLENTRY SSBuildDependencies(hWnd)

HWND             hWnd;
{
LPSPREADSHEET    lpSS;
BOOL             bRet;

lpSS = SS_Lock(hWnd);
bRet = SS_BuildDependencies(lpSS);
SS_Unlock(hWnd);
return bRet;
}


BOOL SS_BuildDependencies(LPSPREADSHEET lpSS)
{
LPTBGLOBALHANDLE CalcTable;
LPSS_ROW         lpRow;
LPSS_COL         lpCol;
LPSS_CELL        lpCell;
SS_CELLCOORD     CellCoord;
LPSS_CALC        Calc;
SS_COORD         i;
SS_COORD         j;

if (!lpSS->fCalcBuildDependencies)
   return (TRUE);

if (lpSS->CalcTableCnt == 0)
   return (TRUE);

/******************************
* Delete all dependency lists
******************************/

SS_FreeCalcList(&lpSS->CalcAllDependents);

for (i = 0; i < lpSS->Row.AllocCnt; i++)
   {
   if (lpRow = SS_LockRowItem(lpSS, i))
      {
      if (lpRow->Dependents.hItems)
         SS_FreeCalcList(&lpRow->Dependents);

      for (j = 0; j < lpSS->Col.AllocCnt; j++)
         if (lpCell = SSx_LockCellItem(lpSS, lpRow, j, i))
            {
            if (lpCell->Dependents.hItems)
               SS_FreeCalcList(&lpCell->Dependents);

            SS_UnlockCellItem(lpSS, j, i);
            }

      SS_UnlockRowItem(lpSS, i);
      }
   }

for (i = 0; i < lpSS->Col.AllocCnt; i++)
   if (lpCol = SS_LockColItem(lpSS, i))
      {
      if (lpCol->Dependents.hItems)
         SS_FreeCalcList(&lpCol->Dependents);

      SS_UnlockColItem(lpSS, i);
      }

if (lpSS->hCalcTable)
   {
   CalcTable = (LPTBGLOBALHANDLE)tbGlobalLock(lpSS->hCalcTable);
   for (i = 0; i < lpSS->CalcTableCnt; i++)
      {
      Calc = (LPSS_CALC)tbGlobalLock(CalcTable[i]);

      if (!SS_CalcBuildDependencies1(lpSS, Calc))
         {
         tbGlobalUnlock(CalcTable[i]);
         tbGlobalUnlock(lpSS->hCalcTable);
         return (FALSE);
         }

      tbGlobalUnlock(CalcTable[i]);
      }

   tbGlobalUnlock(lpSS->hCalcTable);
   }

CellCoord.Row = -1;
CellCoord.Col = -1;

if (!SS_CalcBuildDependencies2(lpSS, &lpSS->CalcAllDependents, &CellCoord))
   {
   return (FALSE);
   }

for (i = 0; i < lpSS->Row.AllocCnt; i++)
   {
   if (lpRow = SS_LockRowItem(lpSS, i))
      {
      if (lpRow->Dependents.hItems)
         {
         CellCoord.Row = i;
         CellCoord.Col = -1;

         if (!SS_CalcBuildDependencies2(lpSS, &lpRow->Dependents, &CellCoord))
            {
            SS_UnlockRowItem(lpSS, i);
            return (FALSE);
            }
         }

      for (j = 0; j < lpSS->Col.AllocCnt; j++)
         if (lpCell = SSx_LockCellItem(lpSS, lpRow, j, i))
            {
            if (lpCell->Dependents.hItems)
               {
               CellCoord.Row = i;
               CellCoord.Col = j;

               if (!SS_CalcBuildDependencies2(lpSS, &lpCell->Dependents,
                                              &CellCoord))
                  {
                  SS_UnlockCellItem(lpSS, j, i);
                  SS_UnlockRowItem(lpSS, i);
                  return (FALSE);
                  }
               }

            SS_UnlockCellItem(lpSS, j, i);
            }

      SS_UnlockRowItem(lpSS, i);
      }
   }

for (i = 0; i < lpSS->Col.AllocCnt; i++)
   if (lpCol = SS_LockColItem(lpSS, i))
      {
      if (lpCol->Dependents.hItems)
         {
         CellCoord.Row = -1;
         CellCoord.Col = i;

         if (!SS_CalcBuildDependencies2(lpSS, &lpCol->Dependents, &CellCoord))
            {
            SS_UnlockColItem(lpSS, i);
            return (FALSE);
            }
         }

      SS_UnlockColItem(lpSS, i);
      }

return (TRUE);
}


static BOOL SS_CalcBuildDependencies1(lpSS, Calc)

LPSPREADSHEET  lpSS;
LPSS_CALC      Calc;
{
LPSS_CELLCOORD Ancestors;
LPSS_CELLCOORD DependItems;
LPSS_CALCLIST  Dependents;
LPSS_ROW       lpRow;
LPSS_COL       lpCol;
LPSS_CELL      lpCell;
SS_CELLCOORD   CellCoord;
short          dAllocCnt;
short          i;

if (Calc->Ancestors.hItems)
   {
   Ancestors = (LPSS_CELLCOORD)tbGlobalLock(Calc->Ancestors.hItems);

   for (i = 0; i < Calc->Ancestors.ItemCnt; i++)
      {
      _fmemcpy(&CellCoord, &Ancestors[i], sizeof(SS_CELLCOORD));

      if (CellCoord.Col != -1 || CellCoord.Row != -1)
         {
         if (CellCoord.Col == -1 && Calc->CellCoord.Col != -1)
            CellCoord.Col = Calc->CellCoord.Col;

         if (CellCoord.Row == -1 && Calc->CellCoord.Row != -1)
            CellCoord.Row = Calc->CellCoord.Row;
         }

      /********************************************
      * Retrieve appropriate Dependents Structure
      ********************************************/

      if (CellCoord.Col == -1 && CellCoord.Row == -1)
         Dependents = &lpSS->CalcAllDependents;

      else if (CellCoord.Col == -1)
         {
         if (!(lpRow = SS_AllocLockRow(lpSS, CellCoord.Row)))
            {
            tbGlobalUnlock(Calc->Ancestors.hItems);
            return (FALSE);
            }

         Dependents = &lpRow->Dependents;
         }

      else if (CellCoord.Row == -1)
         {
         if (!(lpCol = SS_AllocLockCol(lpSS, CellCoord.Col)))
            {
            tbGlobalUnlock(Calc->Ancestors.hItems);
            return (FALSE);
            }

         Dependents = &lpCol->Dependents;
         }

      else
         {
         if (!(lpCell = SS_AllocLockCell(lpSS, NULL, CellCoord.Col,
               CellCoord.Row)))
            {
            tbGlobalUnlock(Calc->Ancestors.hItems);
            return (FALSE);
            }

         Dependents = &lpCell->Dependents;
         }

      /******************************
      * Update Dependents Structure
      ******************************/

      if (Dependents->ItemCnt + 1 > Dependents->ItemAllocCnt)
         {
         if (CellCoord.Col == -1 && CellCoord.Row == -1)
            dAllocCnt = 50;
         else
            dAllocCnt = SS_CALC_ALLOC_CNT;

         if (Dependents->ItemAllocCnt == 0)
            Dependents->hItems = tbGlobalAlloc(GMEM_MOVEABLE | GMEM_ZEROINIT,
                                               (long)(sizeof(SS_CELLCOORD) *
                                               (Dependents->ItemAllocCnt +
                                               dAllocCnt)));
         else
            Dependents->hItems = tbGlobalReAlloc(Dependents->hItems,
                                                 (long)(sizeof(SS_CELLCOORD) *
                                                 (Dependents->ItemAllocCnt +
                                                 dAllocCnt)),
                                                 GMEM_MOVEABLE | GMEM_ZEROINIT);

         if (!Dependents->hItems)
            {
            tbGlobalUnlock(Calc->Ancestors.hItems);
            return (FALSE);
            }

         Dependents->ItemAllocCnt += dAllocCnt;
         }

      DependItems = (LPSS_CELLCOORD)tbGlobalLock(Dependents->hItems);
      _fmemcpy(&DependItems[Dependents->ItemCnt], &Calc->CellCoord,
               sizeof(SS_CELLCOORD));
      tbGlobalUnlock(Dependents->hItems);
      Dependents->ItemCnt++;

      if (CellCoord.Col == -1 && CellCoord.Row == -1)
         ;

      else if (CellCoord.Col == -1)
         SS_UnlockRowItem(lpSS, CellCoord.Row);

      else if (CellCoord.Row == -1)
         SS_UnlockColItem(lpSS, CellCoord.Col);

      else
         SS_UnlockCellItem(lpSS, CellCoord.Col, CellCoord.Row);
      }

   tbGlobalUnlock(Calc->Ancestors.hItems);
   }

return (TRUE);
}


static BOOL SS_CalcBuildDependencies2(lpSS, Dependents, CellCoord)

LPSPREADSHEET  lpSS;
LPSS_CALCLIST  Dependents;
LPSS_CELLCOORD CellCoord;
{
LPSS_CALCLIST  DependentsTemp = 0;
LPSS_CELLCOORD DependItems1;
TBGLOBALHANDLE hDependItems1;
LPSS_CELL      lpCell;
LPSS_ROW       lpRow;
LPSS_COL       lpCol;
BOOL           fRet = TRUE;
BOOL           FirstAll;
BOOL           FirstCell;
short          DependItemsCnt1;
short          i;

/*************************
* Create Dependency List
*************************/

if (Dependents->ListBuilt || !Dependents->hItems)
   return (TRUE);

Dependents->ListBuilt = TRUE;

hDependItems1 = Dependents->hItems;
DependItemsCnt1 = Dependents->ItemCnt;
DependItems1 = (LPSS_CELLCOORD)tbGlobalLock(hDependItems1);

Dependents->hItems = 0;
Dependents->ItemCnt = 0;
Dependents->ItemAllocCnt = 0;

/**************************************************************
* Loop through the dependencies that were created from pass 1
**************************************************************/

FirstAll = TRUE;
FirstCell = TRUE;

for (i = 0; i < DependItemsCnt1; i++)
   {
   FirstCell = TRUE;

   /********************************************
   * Retrieve appropriate Dependents Structure
   ********************************************/

   /*
   if (CellCoord->Col != -1 && CellCoord->Row != -1)
      fRet = SS_CalcBuildDependencies3(lpSS, Dependents,
                                       CellCoord, &lpSS->CalcAllDependents,
                                       &DependItems1[i], DependItemsCnt1,
                                       &FirstAll, &FirstCell);
   */

   if (DependItems1[i].Row != -1 && (DependItems1[i].Row != CellCoord->Row ||
       DependItems1[i].Col != -1))
      {
      if (lpRow = SS_LockRowItem(lpSS, DependItems1[i].Row))
         {
         fRet = SS_CalcBuildDependencies3(lpSS, Dependents,
                                          CellCoord, &lpRow->Dependents,
                                          &DependItems1[i], DependItemsCnt1,
                                          NULL, &FirstCell);

         SS_UnlockRowItem(lpSS, DependItems1[i].Row);
         }
      }

   if (DependItems1[i].Col != -1 && (DependItems1[i].Col != CellCoord->Col ||
       DependItems1[i].Row != -1))
      {
      if (lpCol = SS_LockColItem(lpSS, DependItems1[i].Col))
         {
         fRet = SS_CalcBuildDependencies3(lpSS, Dependents,
                                          CellCoord, &lpCol->Dependents,
                                          &DependItems1[i], DependItemsCnt1,
                                          NULL, &FirstCell);

         SS_UnlockColItem(lpSS, DependItems1[i].Col);
         }
      }

   /*
   if (DependItems1[i].Col != -1 && DependItems1[i].Row != -1 &&
       (DependItems1[i].Col != CellCoord->Col ||
       DependItems1[i].Row != CellCoord->Row))
   */
   if (DependItems1[i].Col != -1 && DependItems1[i].Row != -1)
      {
      if (lpCell = SS_LockCellItem(lpSS, DependItems1[i].Col,
                                   DependItems1[i].Row))
         {
         fRet = SS_CalcBuildDependencies3(lpSS, Dependents,
                                          CellCoord, &lpCell->Dependents,
                                          &DependItems1[i], DependItemsCnt1,
                                          NULL, &FirstCell);

         SS_UnlockCellItem(lpSS, DependItems1[i].Col,
                           DependItems1[i].Row);
         }
      }

   if (!fRet)
      return (FALSE);

   tbGlobalUnlock(Dependents->hItems);
   }

/***********************************************
* Re allocate the table so no memory is wasted
***********************************************/

if (Dependents->ItemCnt)
   {
   if (!(Dependents->hItems = tbGlobalReAlloc(Dependents->hItems,
                                              (long)(sizeof(SS_CELLCOORD) *
                                              Dependents->ItemCnt),
                                              GMEM_MOVEABLE | GMEM_ZEROINIT)))
      return (FALSE);
   }

else if (Dependents->hItems)
   {
   tbGlobalFree(Dependents->hItems);
   Dependents->hItems = 0;
   }

Dependents->ItemAllocCnt = Dependents->ItemCnt;

tbGlobalUnlock(hDependItems1);
tbGlobalFree(hDependItems1);

return (TRUE);
}


static BOOL SS_CalcBuildDependencies3(lpSS, Dependents,
                                      CellCoord, DependentsTemp,
                                      lpDependItems1, DependItemsCnt1,
                                      lpfFirst, lpfFirstCell)

LPSPREADSHEET  lpSS;
LPSS_CALCLIST  Dependents;
LPSS_CELLCOORD CellCoord;
LPSS_CALCLIST  DependentsTemp;
LPSS_CELLCOORD lpDependItems1;
short          DependItemsCnt1;
LPBOOL         lpfFirst;
LPBOOL         lpfFirstCell;
{
LPSS_CELLCOORD DependItemsTemp;
LPSS_CELLCOORD DependItems2;
short          New;
short          Old;
short          Cnt = 0;

if (!DependentsTemp)
   return (FALSE);

if (!DependentsTemp->ListBuilt && DependentsTemp->hItems)
   if (!SS_CalcBuildDependencies2(lpSS, DependentsTemp,
                                  lpDependItems1))
      return (FALSE);

DependItemsTemp = (LPSS_CELLCOORD)tbGlobalLock(DependentsTemp->hItems);

if (Dependents->hItems)
   DependItems2 = (LPSS_CELLCOORD)tbGlobalLock(Dependents->hItems);

for (New = 0; New <= DependentsTemp->ItemCnt; New++)
   {
   Cnt++;

   if (Dependents->ItemCnt + New + 1 > Dependents->ItemAllocCnt)
      {
      if (Dependents->hItems)
         tbGlobalUnlock(Dependents->hItems);

      if (Dependents->ItemAllocCnt == 0)
         Dependents->hItems = tbGlobalAlloc(GMEM_MOVEABLE | GMEM_ZEROINIT,
                                            (long)(sizeof(SS_CELLCOORD) *
                                            (Dependents->ItemAllocCnt +
                                            SS_CALC_ALLOC_CNT)));
      else
         Dependents->hItems = tbGlobalReAlloc(Dependents->hItems,
                                              (long)(sizeof(SS_CELLCOORD) *
                                              (Dependents->ItemAllocCnt +
                                              SS_CALC_ALLOC_CNT)),
                                              GMEM_MOVEABLE | GMEM_ZEROINIT);

      if (!Dependents->hItems)
         return (FALSE);

      Dependents->ItemAllocCnt += SS_CALC_ALLOC_CNT;
      DependItems2 = (LPSS_CELLCOORD)tbGlobalLock(Dependents->hItems);
      }

   if (New == 0)
      {
      if (lpfFirstCell && *lpfFirstCell)
         {
         _fmemcpy(&DependItems2[Dependents->ItemCnt + New],
                  lpDependItems1, sizeof(SS_CELLCOORD));

         for (Old = 0; Old < Dependents->ItemCnt; Old++)
            if (_fmemcmp(&DependItems2[Old], lpDependItems1,
                         sizeof(SS_CELLCOORD)) == 0)
               DependItems2[Old].Row = -2;
         }
      }

   else if (lpfFirst && !(*lpfFirst))
      break;

   else
      {
      if (_fmemcmp(CellCoord, &DependItemsTemp[New - 1],
                   sizeof(SS_CELLCOORD)) == 0)
         DependItems2[Dependents->ItemCnt + New].Row = -2;

      else
         {
         _fmemcpy(&DependItems2[Dependents->ItemCnt + New],
                  &DependItemsTemp[New - 1], sizeof(SS_CELLCOORD));

         for (Old = 0; Old <= Dependents->ItemCnt; Old++)
            if (_fmemcmp(&DependItems2[Old], &DependItemsTemp[New - 1],
                sizeof(SS_CELLCOORD)) == 0)
               DependItems2[Old].Row = -2;
         }
      }
   }

tbGlobalUnlock(Dependents->hItems);
tbGlobalUnlock(DependentsTemp->hItems);

Old = 0;
New = 0;

DependItems2 = (LPSS_CELLCOORD)tbGlobalLock(Dependents->hItems);
while (New < Dependents->ItemCnt + Cnt)
   {
   if (DependItems2[New].Row != -2)
      {
      if (New != Old)
         _fmemcpy(&DependItems2[Old], &DependItems2[New],
                  sizeof(SS_CELLCOORD));

      Old++;
      }

   New++;
   }

tbGlobalUnlock(Dependents->hItems);

Dependents->ItemCnt = Old;

if (lpfFirst)
   *lpfFirst = FALSE;

if (lpfFirstCell)
   *lpfFirstCell = FALSE;

return (TRUE);
}


BOOL SS_ReCalcAll(lpSS)

LPSPREADSHEET lpSS;
{
if (lpSS->CalcAllDependents.hItems)
   {
   if (!SSx_CalcDependencies(lpSS, &lpSS->CalcAllDependents,
                             NULL, TRUE, FALSE))
      return (FALSE);
   }

return (TRUE);
}


BOOL SS_CalcDependencies(lpSS, Col, Row)

LPSPREADSHEET lpSS;
SS_COORD      Col;
SS_COORD      Row;
{
SS_CELLCOORD  CellCoord;
SS_CELLCOORD  CurAtCalcSave;
LPSS_CELL     lpCell;
LPSS_ROW      lpRow;
LPSS_COL      lpCol;
BOOL          fFreezeHighlightOld;

lpCell = SS_LockCellItem(lpSS, Col, Row);
lpRow = SS_LockRowItem(lpSS, Row);
lpCol = SS_LockColItem(lpSS, Col);

CurAtCalcSave.Col = lpSS->Col.CurAtCalc;
CurAtCalcSave.Row = lpSS->Row.CurAtCalc;
lpSS->Col.CurAtCalc = Col;
lpSS->Row.CurAtCalc = Row;

fFreezeHighlightOld = lpSS->FreezeHighlight;

if ((lpCell && lpCell->Dependents.hItems) ||
    (lpRow && lpRow->Dependents.hItems) ||
    (lpCol && lpCol->Dependents.hItems))
   {
   if (lpCol && lpCol->Dependents.hItems)
      {
      SS_HighlightCell(lpSS, FALSE);
      lpSS->FreezeHighlight = TRUE;

      if (!SSx_CalcDependencies(lpSS, &lpCol->Dependents, NULL,
                                FALSE, TRUE))
         {
         SS_UnlockRowItem(lpSS, Row);
         SS_UnlockColItem(lpSS, Col);
         lpSS->FreezeHighlight = fFreezeHighlightOld;
         return (FALSE);
         }
      }

   if (lpRow && lpRow->Dependents.hItems)
      {
      SS_HighlightCell(lpSS, FALSE);
      lpSS->FreezeHighlight = TRUE;

      if (!SSx_CalcDependencies(lpSS, &lpRow->Dependents, NULL,
                                FALSE, TRUE))
         {
         SS_UnlockRowItem(lpSS, Row);
         SS_UnlockColItem(lpSS, Col);
         lpSS->FreezeHighlight = fFreezeHighlightOld;
         return (FALSE);
         }
      }

   if (lpCell && lpCell->Dependents.hItems)
      {
      SS_HighlightCell(lpSS, FALSE);
      lpSS->FreezeHighlight = TRUE;

      CellCoord.Col = Col;
      CellCoord.Row = Row;

      if (!SSx_CalcDependencies(lpSS, &lpCell->Dependents,
                                &CellCoord, FALSE, FALSE))
         {
         SS_UnlockRowItem(lpSS, Row);
         SS_UnlockColItem(lpSS, Col);
         lpSS->FreezeHighlight = fFreezeHighlightOld;
         return (FALSE);
         }
      }

   lpSS->FreezeHighlight = fFreezeHighlightOld;
   SS_HighlightCell(lpSS, TRUE);
   }

lpSS->Col.CurAtCalc = CurAtCalcSave.Col;
lpSS->Row.CurAtCalc = CurAtCalcSave.Row;

SS_UnlockRowItem(lpSS, Row);
SS_UnlockColItem(lpSS, Col);
SS_UnlockCellItem(lpSS, Col, Row);

return (TRUE);
}


static BOOL SSx_CalcDependencies(lpSS, Dependents, lpCellCoord,
                                 fAll, fExpand)

LPSPREADSHEET  lpSS;
LPSS_CALCLIST  Dependents;
LPSS_CELLCOORD lpCellCoord;
BOOL           fAll;
BOOL           fExpand;
{
LPSS_CELLCOORD DependItems;
BOOL           Ret;
BOOL           fRetValue = TRUE;
short          i;
SS_COORD       j;
SS_COORD       Col;
SS_COORD       Row;

if (Dependents->hItems)
   {
   DependItems = (LPSS_CELLCOORD)tbGlobalLock(Dependents->hItems);

   lpSS->fCalcInProgress = TRUE;
   /*
   lpSS->Col.CurAtSave = lpSS->Col.CurAt;
   lpSS->Row.CurAtSave = lpSS->Row.CurAt;
   */

   for (i = 0; i < Dependents->ItemCnt; i++)
      {
      if (lpCellCoord && lpCellCoord->Col == DependItems[i].Col &&
          lpCellCoord->Row == DependItems[i].Row)
         continue;

      if (fAll && DependItems[i].Col == -1)
         for (j = 1; j < lpSS->Col.DataCnt; j++)
            {
            lpSS->Col.CurAtCalc = j;
            Ret = SSx_CalcCell(lpSS, &DependItems[i]);
            }

      else if (fAll && DependItems[i].Row == -1)
         for (j = 1; j < lpSS->Row.DataCnt; j++)
            {
            lpSS->Row.CurAtCalc = j;
            Ret = SSx_CalcCell(lpSS, &DependItems[i]);
            }

      else
         {
         Ret = SSx_CalcCell(lpSS, &DependItems[i]);

         Col = DependItems[i].Col;
         Row = DependItems[i].Row;

         if (Col == -1)
            Col = lpSS->Col.CurAtCalc;

         if (Row == -1)
            Row = lpSS->Row.CurAtCalc;

         /* RFW 7/14/93
         if (!lpCellCoord || (lpCellCoord->Col != Col ||
             lpCellCoord->Row != Row))
            SS_CalcDependencies(lpSS, Col, Row);
         */

         if (DependItems[i].Col == -1 || DependItems[i].Row == -1)
            SS_CalcDependencies(lpSS, Col, Row);
         }

      if (Ret == FALSE)
         break;
      else if (Ret == -1)
         {
         fRetValue = FALSE;
         break;
         }
      }

   lpSS->fCalcInProgress = FALSE;
   /*
   lpSS->Col.CurAt = lpSS->Col.CurAtSave;
   lpSS->Row.CurAt = lpSS->Row.CurAtSave;
   */

   tbGlobalUnlock(Dependents->hItems);
   }

return (fRetValue);
}


BOOL SSx_CalcCell(lpSS, CellCoord)

LPSPREADSHEET  lpSS;
LPSS_CELLCOORD CellCoord;
{
TBGLOBALHANDLE hCalc = 0;
LPSS_CELL      lpCell = 0;
LPSS_CELLTYPE  CellType = 0;
SS_CELLTYPE    CellTypeTemp;
SS_CELLCOORD   CellCoordTemp;
LPSS_CALC      Calc;
LPSS_COL       lpCol;
LPSS_ROW       lpRow;
SS_VALUE       Result;
SS_DATA        Data;
LPTSTR         Formula;
LPTSTR         Ptr;

if ((CellCoord->Col != -1 && CellCoord->Col >= SS_GetColCnt(lpSS)) ||
    (CellCoord->Row != -1 && CellCoord->Row >= SS_GetRowsMax(lpSS)))
   return (TRUE);

_fmemcpy(&CellCoordTemp, CellCoord, sizeof(SS_CELLCOORD));

if (CellCoordTemp.Col == -1)
   CellCoordTemp.Col = lpSS->Col.CurAtCalc;

if (CellCoordTemp.Row == -1)
   CellCoordTemp.Row = lpSS->Row.CurAtCalc;

if (lpCell = SS_LockCellItem(lpSS, CellCoordTemp.Col, CellCoordTemp.Row))
   hCalc = lpCell->hCalc;

if (!hCalc)
   {
   if (lpRow = SS_LockRowItem(lpSS, CellCoordTemp.Row))
      {
      hCalc = lpRow->hCalc;
      SS_UnlockRowItem(lpSS, CellCoordTemp.Row);
      }

   if (!hCalc)
      {
      if (lpCol = SS_LockColItem(lpSS, CellCoordTemp.Col))
         {
         hCalc = lpCol->hCalc;
         SS_UnlockColItem(lpSS, CellCoordTemp.Col);
         }
      }

   /*
   if (CellCoordTemp.Col == -1)
      {
      CellCoordTemp.Col = lpSS->Col.CurAtCalc;
      lpRow = SS_LockRowItem(lpSS, CellCoordTemp.Row);
      hCalc = lpRow->hCalc;
      SS_UnlockRowItem(lpSS, CellCoordTemp.Row);
      }

   else if (CellCoordTemp.Row == -1)
      {
      CellCoordTemp.Row = lpSS->Row.CurAtCalc;
      lpCol = SS_LockColItem(lpSS, CellCoordTemp.Col);
      hCalc = lpCol->hCalc;
      SS_UnlockColItem(lpSS, CellCoordTemp.Col);
      }
   */
   }

CellType = SS_RetrieveCellType(lpSS, &CellTypeTemp, lpCell,
                               CellCoordTemp.Col, CellCoordTemp.Row);

if (hCalc)
   {
   Calc = (LPSS_CALC)tbGlobalLock(hCalc);

   if (Calc->hFormula)
      {
      Formula = (LPTSTR)tbGlobalLock(Calc->hFormula);

      _fmemcpy(&lpSS->CurCalcCell, &CellCoordTemp,
               sizeof(SS_CELLCOORD));

      _fmemcpy(&lpSS->CurCellSendingMsg, &CellCoordTemp,
               sizeof(SS_CELLCOORD));

      SS_Calc(lpSS, Formula, &Result);

      if (Result.Status == SS_VALUE_STATUS_ERROR)
         {
         tbGlobalUnlock(Calc->hFormula);
         tbGlobalUnlock(hCalc);
         return (FALSE);
         }

      if (Result.Status != SS_VALUE_STATUS_EMPTY)
         {
         if (CellType && CellType->Type == SS_TYPE_INTEGER)
            {
            _fmemset(&Data, 0, sizeof(SS_DATA));
            Data.bDataType = SS_TYPE_INTEGER;
            if (Result.Type == SS_VALUE_TYPE_LONG)
               {
               Data.Data.lValue = Result.Val.ValLong;
               SS_SetCellDataItem(lpSS, CellCoordTemp.Col, CellCoordTemp.Row,
                                  &Data);
               }
            else if (Result.Type == SS_VALUE_TYPE_DOUBLE)
               {
               Data.Data.lValue = (long)Result.Val.ValDouble;
               SS_SetCellDataItem(lpSS, CellCoordTemp.Col, CellCoordTemp.Row,
                                  &Data);
               }
            else
               SS_SetCellData(lpSS, CellCoordTemp.Col, CellCoordTemp.Row,
                              _T(""), 0);
            }

         else if (CellType && CellType->Type == SS_TYPE_FLOAT)
            {
            _fmemset(&Data, 0, sizeof(SS_DATA));
            Data.bDataType = SS_TYPE_FLOAT;
            if (Result.Type == SS_VALUE_TYPE_LONG)
               {
               Data.Data.dfValue = (double)Result.Val.ValLong;
               SS_SetCellDataItem(lpSS, CellCoordTemp.Col, CellCoordTemp.Row,
                                  &Data);
               }
            else if (Result.Type == SS_VALUE_TYPE_DOUBLE)
               {
               Data.Data.dfValue = Result.Val.ValDouble;
               SS_SetCellDataItem(lpSS, CellCoordTemp.Col, CellCoordTemp.Row,
                                  &Data);
               }
            else
               SS_SetCellData(lpSS, CellCoordTemp.Col, CellCoordTemp.Row,
                              _T(""), 0);
            }

         else
            {
            if (Result.Type == SS_VALUE_TYPE_STR)
               {
               Ptr = (LPTSTR)GlobalLock(Result.Val.hValStr);
               SS_SetCellData(lpSS, CellCoordTemp.Col, CellCoordTemp.Row, Ptr,
                              (short)lstrlen(Ptr));
               GlobalUnlock(Result.Val.hValStr);
               }
            }
         }

      else if (CellType && (CellType->Type == SS_TYPE_STATICTEXT ||
               CellType->Type == SS_TYPE_EDIT))
         ;

      else
         SS_SetCellData(lpSS, CellCoordTemp.Col, CellCoordTemp.Row, _T(""), 0);

      SS_InvalidateCell(lpSS, CellCoordTemp.Col, CellCoordTemp.Row);

      tbGlobalUnlock(Calc->hFormula);
      SS_FreeValue(&Result);
      }

   tbGlobalUnlock(hCalc);
   }

if (lpCell)
   SS_UnlockCellItem(lpSS, CellCoordTemp.Col, CellCoordTemp.Row);

return (TRUE);
}


BOOL DLLENTRY SSAddCustomFunction(HWND hWnd, LPCTSTR lpszFuncName,
                                  short nParamCnt, FARPROC lpfnFuncProc)
{
LPSPREADSHEET lpSS;
BOOL          bRet;

lpSS = SS_Lock(hWnd);
bRet = SS_AddCustomFunction(lpSS, lpszFuncName, nParamCnt, lpfnFuncProc);
SS_Unlock(hWnd);
return bRet;
}


BOOL SS_AddCustomFunction(LPSPREADSHEET lpSS, LPCTSTR lpszFuncName,
                          short nParamCnt, FARPROC lpfnFuncProc)
{
LPSS_CALCFUNCTION CalcFunctions;
LPTSTR            lpszName;

if (!lpszFuncName || lstrlen(lpszFuncName) == 0)
   return (FALSE);

if (lpSS->CalcFunctionsCnt + 1 > lpSS->CalcFunctionsAllocCnt)
   {
   if (lpSS->CalcFunctionsAllocCnt == 0)
      lpSS->hCalcFunctions = tbGlobalAlloc(GMEM_MOVEABLE | GMEM_ZEROINIT,
                                    (long)(sizeof(SS_CALCFUNCTION) *
                                    (lpSS->CalcFunctionsAllocCnt +
                                    SS_CALC_ALLOC_CNT)));
   else
      lpSS->hCalcFunctions = tbGlobalReAlloc(lpSS->hCalcFunctions,
                                    (long)(sizeof(SS_CALCFUNCTION) *
                                    (lpSS->CalcFunctionsAllocCnt +
                                    SS_CALC_ALLOC_CNT)),
                                    GMEM_MOVEABLE | GMEM_ZEROINIT);

   if (!lpSS->hCalcFunctions)
      return FALSE;

   lpSS->CalcFunctionsAllocCnt += SS_CALC_ALLOC_CNT;
   }

CalcFunctions = (LPSS_CALCFUNCTION)tbGlobalLock(lpSS->hCalcFunctions);

CalcFunctions[lpSS->CalcFunctionsCnt].ParameterCnt = nParamCnt;
CalcFunctions[lpSS->CalcFunctionsCnt].Function = (SS_CALCFUNC)lpfnFuncProc;

if (!(CalcFunctions[lpSS->CalcFunctionsCnt].hName =
      tbGlobalAlloc(GMEM_MOVEABLE | GMEM_ZEROINIT,
                    (lstrlen(lpszFuncName) + 1) * sizeof(TCHAR))))
   {
   tbGlobalUnlock(lpSS->hCalcFunctions);
   return FALSE;
   }

lpszName = (LPTSTR)tbGlobalLock(CalcFunctions[lpSS->CalcFunctionsCnt].hName);
lstrcpy(lpszName, lpszFuncName);
tbGlobalUnlock(CalcFunctions[lpSS->CalcFunctionsCnt].hName);

lpSS->CalcFunctionsCnt++;

tbGlobalUnlock(lpSS->hCalcFunctions);
return TRUE;
}


void SS_FreeCalcFunctions(lpSS)

LPSPREADSHEET     lpSS;
{
LPSS_CALCFUNCTION CalcFunctions;
short             i;

if (lpSS->hCalcFunctions)
   {
   CalcFunctions = (LPSS_CALCFUNCTION)tbGlobalLock(lpSS->hCalcFunctions);

   for (i = 0; i < lpSS->CalcFunctionsCnt; i++)
      if (CalcFunctions[i].hName)
         tbGlobalFree(CalcFunctions[i].hName);

   tbGlobalUnlock(lpSS->hCalcFunctions);
   tbGlobalFree(lpSS->hCalcFunctions);
   lpSS->hCalcFunctions = 0;
   }
}


static void SS_FreeValue(LPSS_VALUE lpVal)
{
if (lpVal->Status == SS_VALUE_STATUS_OK &&
    lpVal->Type == SS_VALUE_TYPE_STR && lpVal->Val.hValStr)
   GlobalFree(lpVal->Val.hValStr);
}

#else  // !defined(SS_OLDCALC)

//--------------------------------------------------------------------
//
//  The SSAddCustomFunction() function registers a custom function
//  with the spreadsheet for use in formulas.
//

BOOL DLLENTRY SSAddCustomFunction(HWND hWnd, LPCTSTR lpszFuncName,
                                  short nParamCnt, FARPROC lpfnFuncProc)
{
LPSS_BOOK lpBook;
BOOL bRet;

lpBook = SS_BookLock(hWnd);
bRet = SS_AddCustomFunctionExt(lpBook, lpszFuncName, nParamCnt, nParamCnt,
                               lpfnFuncProc, 0);
SS_BookUnlock(hWnd);
return bRet;
}

//--------------------------------------------------------------------
//
//  The SSAddCustomFunctionExt() function registers a custom function
//  with the spreadsheet for use in formulas.
//

BOOL DLLENTRY SSAddCustomFunctionExt(HWND hWnd, LPCTSTR lpszFuncName,
                                     short nMinParamCnt, short nMaxParamCnt,
                                     FARPROC lpfnFuncProc, long lFlags)
{
LPSS_BOOK lpBook;
BOOL bRet;

lpBook = SS_BookLock(hWnd);
bRet = SS_AddCustomFunctionExt(lpBook, lpszFuncName, nMinParamCnt, nMaxParamCnt,
                               lpfnFuncProc, lFlags);
SS_BookUnlock(hWnd);
return bRet;
}

//--------------------------------------------------------------------
//
//  The SS_AddCustomFunctionExt() function registers a custom function
//  with the spreadsheet for use in formulas.
//

BOOL SS_AddCustomFunctionExt(LPSS_BOOK lpBook, LPCTSTR lpszFuncName,
                             short nMinParamCnt, short nMaxParamCnt,
                             FARPROC lpfnFuncProc, long lFlags)
{
if( SS_CALC_VARPARAMS == nMinParamCnt )
  nMinParamCnt = 0;
if( SS_CALC_VARPARAMS == nMaxParamCnt )
  nMaxParamCnt = SS_CALC_MAXPARAMS;
return CalcCustFuncAdd(&lpBook->CalcInfo, lpszFuncName,
                       nMinParamCnt, nMaxParamCnt,
                       (CALC_CUSTEVALPROC)lpfnFuncProc, lFlags);

}

//--------------------------------------------------------------------
//
//  The SS_RemoveCustomFunction() function unregisters a custom
//  function.
//

#ifdef SS_V30
BOOL SS_RemoveCustomFunction(LPSS_BOOK lpBook, LPCTSTR lpszFuncName)
{
return CalcCustFuncRemove(&lpBook->CalcInfo, lpszFuncName);
}
#endif

//--------------------------------------------------------------------
//
//  The SSRemoveCustomFunction() function unregisters a custom
//  function.
//

#ifdef SS_V30
BOOL DLLENTRY SSRemoveCustomFunction(HWND hWnd, LPCTSTR lpszFuncName)
{
LPSS_BOOK lpBook;
BOOL bRet;

lpBook = SS_BookLock(hWnd);
bRet = SS_RemoveCustomFunction(lpBook, lpszFuncName);
SS_BookUnlock(hWnd);
return bRet;
}
#endif

//--------------------------------------------------------------------
//
//  The SS_EnumCustomFunction() function is used to scan the list of
//  custom functions.
//

#ifdef SS_V30
int SS_EnumCustomFunction(LPSS_BOOK lpBook, LPCTSTR lpszPrevFunc,
                          LPTSTR lpszFunc, int nFuncLen)
{
int nRet;

if (NULL == lpszPrevFunc || 0 == lpszPrevFunc[0])
  nRet = CalcCustFuncFirst(&lpBook->CalcInfo, lpszFunc, nFuncLen);
else
  nRet = CalcCustFuncNext(&lpBook->CalcInfo, lpszPrevFunc, lpszFunc, nFuncLen);
return nRet;
}
#endif

//--------------------------------------------------------------------
//
//  The SSEnumCustomFunction() function is used to scan the list of
//  custom functions.
//

#ifdef SS_V30
int DLLENTRY SSEnumCustomFunction(HWND hWnd, LPCTSTR lpszPrevFunc,
                                  LPTSTR lpszFunc, int nFuncLen)
{
LPSS_BOOK lpBook;
int nRet;

lpBook = SS_BookLock(hWnd);
nRet = SS_EnumCustomFunction(lpBook, lpszPrevFunc, lpszFunc, nFuncLen);
SS_BookUnlock(hWnd);
return nRet;
}
#endif

//--------------------------------------------------------------------
//
//  The SS_EnumCustomFunctionLen() function retrieves the length of
//  the text representation of the next custom function.
//

#ifdef SS_V30
int SS_EnumCustomFunctionLen(LPSS_BOOK lpBook, LPCTSTR lpszPrevFunc)
{
int nRet;

if (NULL == lpszPrevFunc || 0 == lpszPrevFunc[0])
  nRet = CalcCustFuncFirstLen(&lpBook->CalcInfo);
else
  nRet = CalcCustFuncNextLen(&lpBook->CalcInfo, lpszPrevFunc);
return nRet;
}
#endif

//--------------------------------------------------------------------
//
//  The SSEnumCustomFunctionLen() function retrieves the length of
//  the text representation of the next custom function.
//

#ifdef SS_V30
int DLLENTRY SSEnumCustomFunctionLen(HWND hWnd, LPCTSTR lpszPrevFunc)
{
LPSS_BOOK lpBook;
int nRet;

lpBook = SS_BookLock(hWnd);
nRet = SS_EnumCustomFunctionLen(lpBook, lpszPrevFunc);
SS_BookUnlock(hWnd);
return nRet;
}
#endif

//--------------------------------------------------------------------
//
//  The SS_GetCustomFunction() function retrieves information about
//  the custom function.
//

#ifdef SS_V30
BOOL SS_GetCustomFunction(LPSS_BOOK lpBook, LPCTSTR lpszFunc,
                          LPSHORT lpnMinParamCnt, LPSHORT lpnMaxParamCnt,
                          LPLONG lplFlags)
{
return CalcCustFuncLookup(&lpBook->CalcInfo, lpszFunc, lpnMinParamCnt,
                          lpnMaxParamCnt, lplFlags, NULL);
}
#endif

//--------------------------------------------------------------------
//
//  The SSGetCustomFunction() function retrieves information about
//  the custom function.
//

#ifdef SS_V30
BOOL DLLENTRY SSGetCustomFunction(HWND hWnd, LPCTSTR lpszFunc,
                                  LPSHORT lpnMinParamCnt,
                                  LPSHORT lpnMaxParamCnt,
                                  LPLONG lplFlags)
{
LPSS_BOOK lpBook;
BOOL bRet;

lpBook = SS_BookLock(hWnd);
bRet = SS_GetCustomFunction(lpBook, lpszFunc, lpnMinParamCnt, lpnMaxParamCnt, lplFlags);
SS_BookUnlock(hWnd);
return bRet;
}
#endif

//--------------------------------------------------------------------
//
//  The SS_ReCalcAll() functions recalculates all cells which
//  contain a formula.
//

BOOL SS_ReCalcAll(LPSPREADSHEET lpSS)
{
CalcEvalAllCells(&lpSS->lpBook->CalcInfo);
return (TRUE);
}

//--------------------------------------------------------------------
//
//  The SS_ReCalcCell() function recalculates an individual cell.
//

#ifdef SS_V30
BOOL SS_ReCalcCell(LPSPREADSHEET lpSS, SS_COORD lCol, SS_COORD lRow)
{
SS_AdjustCellCoordsOut(lpSS, &lCol, &lRow);
return CalcEvalCell(&lpSS->CalcInfo, lCol, lRow);
}
#endif

//--------------------------------------------------------------------
//
//  The SSReCalcCell() function recalculates an individual cell.
//

#ifdef SS_V30
BOOL DLLENTRY SSReCalcCell(HWND hWnd, SS_COORD lCol, SS_COORD lRow)
{
LPSPREADSHEET lpSS;
BOOL          bRet;

lpSS = SS_SheetLock(hWnd);
SS_AdjustCellCoords(lpSS, &lCol, &lRow);
bRet = SS_ReCalcCell(lpSS, lCol, lRow);
SS_SheetUnlock(hWnd);
return bRet;
}
#endif

//--------------------------------------------------------------------
//
//  The SSBuildDependencies() and SS_BuildDependencies() function
//  are no longer used by the calculation routines.
//
BOOL DLLENTRY SSBuildDependencies(HWND hWnd)
{
return TRUE;
}

BOOL SS_BuildDependencies(LPSPREADSHEET lpSS)
{
return TRUE;
}

#endif  // SS_OLDCALC

#else  // dfeined(SS_NOCALC)

BOOL SSx_CalcTableDelete(LPSPREADSHEET lpSS, LPSS_CELLCOORD CellCoord)
{
return (1);
}


BOOL SSx_CalcTableAdd(LPSPREADSHEET lpSS, TBGLOBALHANDLE hCalc)
{
return (1);
}


BOOL SSx_CalcCell(LPSPREADSHEET lpSS, LPSS_CELLCOORD CellCoord)
{
return (1);
}


BOOL SS_CalcIsValid(LPSPREADSHEET lpSS, LPCTSTR Formula, TBGLOBALHANDLE hCalc)
{
return (1);
}


void SS_Calc(LPSPREADSHEET lpSS, LPTSTR Formula, LPSS_VALUE Result)
{
}


BOOL DLLENTRY SSBuildDependencies(HWND hWnd)
{
return (1);
}


BOOL SS_BuildDependencies(LPSPREADSHEET lpSS)
{
return (1);
}


BOOL SS_ReCalcAll(LPSPREADSHEET lpSS)
{
return (1);
}


BOOL SS_CalcDependencies(LPSPREADSHEET lpSS, SS_COORD Col, SS_COORD Row)
{
return (1);
}


BOOL DLLENTRY SSAddCustomFunction(HWND hWnd, LPCTSTR FunctionName,
                                  short ParameterCnt,
                                  FARPROC lpfnFunctionProc)
{
return (1);
}


void SS_FreeCalcFunctions(LPSPREADSHEET lpSS)
{
}

#endif // !defined(SS_NOCALC)

//--------------------------------------------------------------------

#if 0
double SS_Round(double Val, long Precision)
{
double dfPower;
double dfDec;
double dfSign = 1.0;

dfPower = pow(10.0, (double)Precision);

if (Val < 0.0)
   dfSign = -1.0;

//dfDec = Val + (dfSign * 0.5 / dfPower);
dfDec = Val;
Val = floor(Val);
dfDec -= Val;
dfDec *= dfPower * 10.0;
dfDec += 5.0;
dfDec /= 10.0;
Val += (double)(long)dfDec / dfPower;

return (Val);
}
#endif

//--------------------------------------------------------------------

// BJO 18Jun96 KEM3709 - modified SS_Round()

double SS_Round(double Val, long Precision)
{
double dfPower;
double dfDec;
static double dfInt;

dfPower = pow(10.0, (double)Precision);
Val *= dfPower;
dfDec = modf(Val, &dfInt);
Val = dfInt;
if (dfDec >= 0.5)
   Val += 1.0;
else if (dfDec <= -0.5)
   Val -= 1.0;
Val /= dfPower;
return Val;
}
