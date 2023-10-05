/*********************************************************
* SS_FORMU.C
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
#include <stdio.h>
#include <stddef.h>
#include <stdlib.h>
#include <string.h>
#include <process.h>
#include <math.h>
#include "spread.h"
#include "ss_alloc.h"
#include "ss_bool.h"
#include "ss_calc.h"
#include "ss_cell.h"
#include "ss_col.h"
#include "ss_doc.h"
#include "ss_formu.h"
#include "ss_row.h"


#ifndef SS_NOCALC

#ifndef SS_OLDCALC

//--------------------------------------------------------------------
//
//  The SS_QueryCustomName() function is used to scan the list of
//  custom names.
//

int SS_QueryCustomName(LPSS_BOOK lpBook, LPCTSTR lpszPrevName,
                       LPTSTR lpszName, int nNameLen)
{
int nRet;

if (NULL == lpszPrevName || 0 == lpszPrevName[0])
   nRet = CalcCustNameFirst(&lpBook->CalcInfo, lpszName, nNameLen);
else
   nRet = CalcCustNameNext(&lpBook->CalcInfo, lpszPrevName,
                           lpszName, nNameLen);
return nRet;
}

//--------------------------------------------------------------------
//
//  The SS_QueryCustomNameLen() function retrieves the length of the
//  text representation of the next custom name.
//

int SS_QueryCustomNameLen(LPSS_BOOK lpBook, LPCTSTR lpszPrevName)
{
int nRet;

if (NULL == lpszPrevName || 0 == lpszPrevName[0])
   nRet = CalcCustNameFirstLen(&lpBook->CalcInfo);
else
   nRet = CalcCustNameNextLen(&lpBook->CalcInfo, lpszPrevName);
return nRet;
}
           
//--------------------------------------------------------------------
//
//  The SS_GetCustomName() function retrieves the value of
//  a custom name.
//

int SS_GetCustomName(LPSS_BOOK lpBook, LPCTSTR lpszName,
                     LPTSTR lpszValue, int nValueLen)
{
LPSPREADSHEET lpSS;

lpSS = SS_BookLockActiveSheet(lpBook);
SS_GetActiveCell(lpSS, &lpBook->CalcInfo.lColA1, &lpBook->CalcInfo.lRowA1);
SS_AdjustCellCoordsOut(lpSS, &lpBook->CalcInfo.lColA1, &lpBook->CalcInfo.lRowA1);
SS_BookUnlockActiveSheet(lpBook);
return CalcCustNameLookup(&lpBook->CalcInfo, lpszName, lpszValue, nValueLen, NULL);
}
           
//--------------------------------------------------------------------
//
//  The SS_GetCustomNameLen() function retrieves the length of
//  value of a custom name.
//

int SS_GetCustomNameLen(LPSS_BOOK lpBook, LPCTSTR lpszName)
{
LPSPREADSHEET lpSS;

lpSS = SS_BookLockActiveSheet(lpBook);
SS_GetActiveCell(lpSS, &lpBook->CalcInfo.lColA1, &lpBook->CalcInfo.lRowA1);
SS_AdjustCellCoordsOut(lpSS, &lpBook->CalcInfo.lColA1, &lpBook->CalcInfo.lRowA1);
SS_BookUnlockActiveSheet(lpBook);
return CalcCustNameLookupLen(&lpBook->CalcInfo, lpszName);
}
           
//--------------------------------------------------------------------
//
//  The SS_SetCustomName() function assigns a value to the
//  custom name.  If lpszValue == NULL then the name is removed
//  from the list of defined names.
//

BOOL SS_SetCustomName(LPSS_BOOK lpBook, LPCTSTR lpszName,
                      LPCTSTR lpszValue)
{
LPSPREADSHEET lpSS;
BOOL bRet;

lpSS = SS_BookLockActiveSheet(lpBook);
SS_GetActiveCell(lpSS, &lpBook->CalcInfo.lColA1, &lpBook->CalcInfo.lRowA1);
SS_AdjustCellCoordsOut(lpSS, &lpBook->CalcInfo.lColA1, &lpBook->CalcInfo.lRowA1);

if (lpszValue && lpszValue[0])
   bRet = CalcCustNameAdd(&lpBook->CalcInfo, lpszName, lpszValue);
else
{
   bRet = CalcCustNameRemove(&lpBook->CalcInfo, lpszName);
//   bRet = CalcCustNameAdd(&lpBook->CalcInfo, lpszName, NULL);
}
if (lpBook->CalcAuto)
   CalcEvalNeededCells(&lpBook->CalcInfo);
SS_BookUnlockActiveSheet(lpBook);
return bRet;
}

//--------------------------------------------------------------------
//
//  The SSQueryCustomName() function is used to scan the list of
//  custom names.
//

int DLLENTRY SSQueryCustomName(HWND hwnd, LPCTSTR lpszPrevName,
                               LPTSTR lpszName, int nNameLen)
{
LPSS_BOOK lpBook;
int nRet;

lpBook = SS_BookLock(hwnd);
nRet = SS_QueryCustomName(lpBook, lpszPrevName, lpszName, nNameLen);
SS_BookUnlock(hwnd);
return nRet;
}

//--------------------------------------------------------------------
//
//  The SSQueryCustomNameLen() function is used retrieve the length
//  of the next custom name.
//

int DLLENTRY SSQueryCustomNameLen(HWND hwnd, LPCTSTR lpszPrevName)
{
LPSS_BOOK lpBook;
int nRet;

lpBook = SS_BookLock(hwnd);
nRet = SS_QueryCustomNameLen(lpBook, lpszPrevName);
SS_BookUnlock(hwnd);
return nRet;
}
           
//--------------------------------------------------------------------
//
//  The SSGetCustomName() function retrieves the value of
//  a custom names.
//

int DLLENTRY SSGetCustomName(HWND hwnd, LPCTSTR lpszName,
                             LPTSTR lpszValue, int nValueLen)
{
LPSS_BOOK lpBook;
int nRet;

lpBook = SS_BookLock(hwnd);
nRet = SS_GetCustomName(lpBook, lpszName, lpszValue, nValueLen);
SS_BookUnlock(hwnd);
return nRet;
}
           
//--------------------------------------------------------------------
//
//  The SSGetCustomNameLen() function retrieves the length of the
//  value of a custom names.
//

int DLLENTRY SSGetCustomNameLen(HWND hwnd, LPCTSTR lpszName)
{
LPSS_BOOK lpBook;
int nRet;

lpBook = SS_BookLock(hwnd);
nRet = SS_GetCustomNameLen(lpBook, lpszName);
SS_BookUnlock(hwnd);
return nRet;
}
           
//--------------------------------------------------------------------
//
//  The SSSetCustomName() function assigns a value to the
//  custom name.  If lpszValue == NULL then the name is removed
//  from the list of defined names.
//

BOOL DLLENTRY SSSetCustomName(HWND hwnd, LPCTSTR lpszName, LPCTSTR lpszValue)
{
LPSS_BOOK lpBook;
BOOL bRet;

lpBook = SS_BookLock(hwnd);
bRet = SS_SetCustomName(lpBook, lpszName, lpszValue);
SS_BookUnlock(hwnd);
return bRet;
}
           
//--------------------------------------------------------------------
//
//  The SS_GetIteration() function retrieves the iteration options.
//

BOOL SS_GetIteration(LPSS_BOOK lpBook, LPSHORT lpnMaxIterations,
                     LPDOUBLE lpdfMaxChange)
{
  if( lpnMaxIterations )
    *lpnMaxIterations = lpBook->CalcInfo.nMaxIterations;
  if( lpdfMaxChange )
    *lpdfMaxChange = lpBook->CalcInfo.dfMaxChange;
  return lpBook->CalcInfo.bIteration;
}
           
//--------------------------------------------------------------------
//
//  The SS_SetIteration() function asigns the iteration options.
//

BOOL SS_SetIteration(LPSS_BOOK lpBook, BOOL bIteration,
                     short nMaxIterations, double dfMaxChange)
{
BOOL bRet;

bRet = lpBook->CalcInfo.bIteration;
lpBook->CalcInfo.bIteration = bIteration;
lpBook->CalcInfo.nMaxIterations = nMaxIterations;
lpBook->CalcInfo.dfMaxChange = dfMaxChange;
return bRet;
}
           
//--------------------------------------------------------------------
//
//  The SSGetIteration() function retrieves the iteration options.
//

BOOL DLLENTRY SSGetIteration(HWND hwnd, LPSHORT lpnMaxIterations,
                             LPDOUBLE lpdfMaxChange)
{
LPSS_BOOK lpBook;
BOOL bRet;

lpBook = SS_BookLock(hwnd);
bRet = SS_GetIteration(lpBook, lpnMaxIterations, lpdfMaxChange);
SS_BookUnlock(hwnd);
return bRet;
}
           
//--------------------------------------------------------------------
//
//  The SSSetIteration() function asigns the iteration options.
//

BOOL DLLENTRY SSSetIteration(HWND hwnd, BOOL bIteration,
                             short nMaxIterations, double dfMaxChange)
{
LPSS_BOOK lpBook;
BOOL bRet;

lpBook = SS_BookLock(hwnd);
bRet = SS_SetIteration(lpBook, bIteration, nMaxIterations, dfMaxChange);
SS_BookUnlock(hwnd);
return bRet;
}

//--------------------------------------------------------------------
//
//  The SS_GetRefStyle() function retrieves the reference style used
//  to parse/unparse formulas.
//

short SS_GetRefStyle(LPSS_BOOK lpBook)
{
return lpBook->CalcInfo.nRefStyle;
}

//--------------------------------------------------------------------
//
//  The SS_SetRefStyle() function assigns the reference style used
//  to parse/unparse formulas.
//

short SS_SetRefStyle(LPSS_BOOK lpBook, short nStyle)
{
short nStyleOld;

nStyleOld = lpBook->CalcInfo.nRefStyle;
if (SS_REFSTYLE_DEFAULT == nStyle || SS_REFSTYLE_A1 == nStyle || SS_REFSTYLE_R1C1 == nStyle)
   lpBook->CalcInfo.nRefStyle = nStyle;
return nStyleOld;
}
           
//--------------------------------------------------------------------
//
//  The SSGetRefStyle() function retrieves the reference style used
//  to parse/unparse formulas.
//

short DLLENTRY SSGetRefStyle(HWND hwnd)
{
LPSS_BOOK lpBook;
short nStyle;

lpBook = SS_BookLock(hwnd);
nStyle = SS_GetRefStyle(lpBook);
SS_BookUnlock(hwnd);
return nStyle;
}

//--------------------------------------------------------------------
//
//  The SSSetRefStyle() function assigns the reference style used
//  to parse/unparse formulas.
//

short DLLENTRY SSSetRefStyle(HWND hwnd, short nStyle)
{
LPSS_BOOK lpBook;
short nStyleOld;

lpBook = SS_BookLock(hwnd);
nStyleOld = SS_SetRefStyle(lpBook, nStyle);
SS_BookUnlock(hwnd);
return nStyleOld;
}

#endif  // !defined(SS_OLDCALC)

//--------------------------------------------------------------------


BOOL  SS_SetFormula(LPSPREADSHEET lpSS, SS_COORD Col, SS_COORD Row,
                    LPCTSTR Formula, BOOL BuildDependencies);

BOOL DLLENTRY SSSetFormula(hWnd, Col, Row, Formula, BuildDependencies)

HWND          hWnd;
SS_COORD      Col;
SS_COORD      Row;
LPCTSTR       Formula;
BOOL          BuildDependencies;
{
return (SSSetFormulaRange(hWnd, Col, Row, Col, Row, Formula,
                          BuildDependencies));
}


BOOL DLLENTRY SSSetFormulaRange(hWnd, Col, Row, Col2, Row2, Formula,
                                BuildDependencies)

HWND          hWnd;
SS_COORD      Col;
SS_COORD      Row;
SS_COORD      Col2;
SS_COORD      Row2;
LPCTSTR       Formula;
BOOL          BuildDependencies;
{
LPSPREADSHEET lpSS;
BOOL          bRet;

lpSS = SS_SheetLock(hWnd);
if (Col != SS_ALLCOLS)
   SS_AdjustCellCoords(lpSS, &Col, NULL);
if (Row != SS_ALLROWS)
   SS_AdjustCellCoords(lpSS, NULL, &Row);
if (Col2 != SS_ALLCOLS)
   SS_AdjustCellCoords(lpSS, &Col2, NULL);
if (Row2 != SS_ALLROWS)
   SS_AdjustCellCoords(lpSS, NULL, &Row2);
bRet = SS_SetFormulaRange(lpSS, Col, Row, Col2, Row2, Formula,
                          BuildDependencies);
SS_SheetUnlock(hWnd);
return bRet;
}


BOOL SS_SetFormulaRange(lpSS, Col, Row, Col2, Row2, Formula,
                        BuildDependencies)

LPSPREADSHEET lpSS;
SS_COORD      Col;
SS_COORD      Row;
SS_COORD      Col2;
SS_COORD      Row2;
LPCTSTR       Formula;
BOOL          BuildDependencies;
{
BOOL          Ret = FALSE;
BOOL          RedrawOld;
#ifdef SS_OLDCALC
SS_COORD      x;
SS_COORD      y;
SS_COORD      x1;
SS_COORD      x2;
SS_COORD      y1;
SS_COORD      y2;
SS_COORD      i;
SS_COORD      j;
SS_CELLCOORD  CellCoord;
#endif

SS_LimitRangeCoords(lpSS, &Col, &Row, &Col2, &Row2);

if (Col == SS_ALLCOLS && Row == SS_ALLROWS)
   return (FALSE);

if (Row != Row2 || Col != Col2)
   {
   RedrawOld = lpSS->lpBook->Redraw;
   lpSS->lpBook->Redraw = FALSE;
   }

#ifdef SS_OLDCALC

for (y = Row; y <= Row2; y++)
   for (x = Col; x <= Col2; x++)
      if (!(Ret = SS_SetFormula(lpSS, x, y, Formula, FALSE)))
         break;

if (Ret && BuildDependencies)
   {
   SS_BuildDependencies(lpSS);

   if (lpSS->lpBook->CalcAuto)
      {
      for (y = Row; y <= Row2; y++)
         for (x = Col; x <= Col2; x++)
            {
            if (x == -1)
               {
               x1 = lpSS->Col.HeaderCnt;
               x2 = lpSS->Col.DataCnt - 1;
               }
            else
               {
               x1 = x;
               x2 = x;
               }

            if (y == -1)
               {
               y1 = lpSS->Row.HeaderCnt;
               y2 = lpSS->Row.DataCnt - 1;
               }
            else
               {
               y1 = y;
               y2 = y;
            }

            for (i = x1; i <= x2; i++)
               for (j = y1; j <= y2; j++)
                  {
                  CellCoord.Col = i;
                  CellCoord.Row = j;
                  SSx_CalcCell(lpSS, &CellCoord);
                  SS_CalcDependencies(lpSS, i, j);
                  }
            }
      }
   }

#else  // !defined(SS_OLDCALC)

SS_AdjustCellCoordsOut(lpSS, &Col, &Row);
SS_AdjustCellCoordsOut(lpSS, &Col2, &Row2);
Ret = CalcSetExprRange(&lpSS->CalcInfo, Col, Row, Col2, Row2, Formula);
if (lpSS->lpBook->CalcAuto)
   CalcEvalNeededCells(&lpSS->lpBook->CalcInfo);

#endif  // defined(SS_OLDCALC)

if (Row != Row2 || Col != Col2)
	SS_BookSetRedraw(lpSS->lpBook, RedrawOld);

return (Ret);
}


#ifdef SS_OLDCALC
BOOL SS_SetFormula(lpSS, Col, Row, Formula, BuildDependencies)

LPSPREADSHEET  lpSS;
SS_COORD       Col;
SS_COORD       Row;
LPCTSTR        Formula;
BOOL           BuildDependencies;
{
TBGLOBALHANDLE hCalc;
LPSS_CALC      Calc;
LPSS_ROW       lpRow;
LPSS_COL       lpCol;
LPSS_CELL      lpCell;
LPTSTR         FormulaTemp;
BOOL           Ret = FALSE;
SS_COORD       i;

if ((Col != SS_ALLCOLS && Col >= SS_GetColCnt(lpSS)) ||
    (Row != SS_ALLROWS && Row >= SS_GetRowCnt(lpSS)))
   return (TRUE);

lpSS->CurCellSendingMsg.Col = Col;
lpSS->CurCellSendingMsg.Row = Row;

if (!Formula || lstrlen(Formula) == 0)
   return (SS_FreeFormula(lpSS, Col, Row));

else if (!(hCalc = tbGlobalAlloc(GMEM_MOVEABLE | GMEM_ZEROINIT,
                                 sizeof(SS_CALC))))
   return (TRUE);

else if (!SS_CalcIsValid(lpSS, Formula, hCalc))
   tbGlobalFree(hCalc);

else if (!SS_Alloc(lpSS, Col, Row))
   ;

else if (!SS_FreeFormula(lpSS, Col, Row))
   ;

else
   {
   Calc = (LPSS_CALC)tbGlobalLock(hCalc);

   Calc->CellCoord.Col = Col;
   Calc->CellCoord.Row = Row;

   if (!(Calc->hFormula = tbGlobalAlloc(GMEM_MOVEABLE | GMEM_ZEROINIT,
                                        (lstrlen(Formula) + 1)
                                        * sizeof(TCHAR))))
      tbGlobalUnlock(hCalc);

   else
      {
      Ret = TRUE;

      FormulaTemp = (LPTSTR)tbGlobalLock(Calc->hFormula);
      lstrcpy(FormulaTemp, Formula);
      tbGlobalUnlock(Calc->hFormula);

      tbGlobalUnlock(hCalc);

      if (!SSx_CalcTableAdd(lpSS, hCalc))
         return (FALSE);

      if (Col != SS_ALLCOLS && Row != SS_ALLROWS)
         {
         if (lpCell = SS_LockCellItem(lpSS, Col, Row))
            {
            lpCell->hCalc = hCalc;
            SS_UnlockCellItem(lpSS, Col, Row);
            }
         else
            Ret = FALSE;
         }

      else if (Col != SS_ALLCOLS && Row == SS_ALLROWS)
         {
         for (i = 1; i < lpSS->Row.AllocCnt; i++)
            SS_FreeFormula(lpSS, Col, i);

         if (lpCol = SS_LockColItem(lpSS, Col))
            {
            lpCol->hCalc = hCalc;
            SS_UnlockColItem(lpSS, Col);
            }
         }

      else if (Col == SS_ALLCOLS && Row != SS_ALLROWS)
         {
         for (i = 1; i < lpSS->Col.AllocCnt; i++)
            SS_FreeFormula(lpSS, i, Row);

         if (lpRow = SS_LockRowItem(lpSS, Row))
            {
            lpRow->hCalc = hCalc;
            SS_UnlockRowItem(lpSS, Row);
            }
         }

      if (Ret && BuildDependencies)
         if (!SS_BuildDependencies(lpSS))
            Ret = FALSE;
      }
   }

return (Ret);
}
#endif  // defined(SS_OLDCALC)


int DLLENTRY SSGetFormulaLen(hWnd, Col, Row)

HWND          hWnd;
SS_COORD      Col;
SS_COORD      Row;
{
LPSPREADSHEET lpSS;
int           Len = 0;

lpSS = SS_SheetLock(hWnd);
if (Col != SS_ALLCOLS)
   SS_AdjustCellCoords(lpSS, &Col, NULL);
if (Row != SS_ALLROWS)
   SS_AdjustCellCoords(lpSS, NULL, &Row);
Len = SS_GetFormulaLen(lpSS, Col, Row);
SS_SheetUnlock(hWnd);

return (Len);
}


int SS_GetFormulaLen(lpSS, Col, Row)

LPSPREADSHEET  lpSS;
SS_COORD       Col;
SS_COORD       Row;
{
#ifdef SS_OLDCALC

TBGLOBALHANDLE hFormula;
LPTSTR         lpFormulaTemp;
short          Len = 0;
if (hFormula = SS_GetFormula(lpSS, Col, Row))
   {
   lpFormulaTemp = (LPTSTR)tbGlobalLock(hFormula);
   Len = lstrlen(lpFormulaTemp);
   tbGlobalUnlock(hFormula);
   }
return (Len);

#else  // !defined(SS_OLDCALC)

SS_AdjustCellCoordsOut(lpSS, &Col, &Row);
return CalcGetExprLen(&lpSS->CalcInfo, Col, Row
#if SS_V80 // 24919 -scl
                     , FALSE
#endif
                     );

#endif  // defined(SS_OLDCALC)
}


int DLLENTRY SSGetFormula(hWnd, Col, Row, lpFormula)

HWND           hWnd;
SS_COORD       Col;
SS_COORD       Row;
LPTSTR         lpFormula;
{
#ifdef SS_OLDCALC

LPSPREADSHEET  lpSS;
TBGLOBALHANDLE hFormula;
LPTSTR         lpFormulaTemp;
int            Len = 0;
lpSS = SS_Lock(hWnd);
if (hFormula = SS_GetFormula(lpSS, Col, Row))
   {
   lpFormulaTemp = (LPTSTR)tbGlobalLock(hFormula);
   if (lpFormula)
      lstrcpy(lpFormula, lpFormulaTemp);
   Len = lstrlen(lpFormulaTemp);
   tbGlobalUnlock(hFormula);
   }
SS_Unlock(hWnd);
return (Len);

#else  // !defined(SS_OLDCALC)

LPSPREADSHEET lpSS;
int         Len = 0;
lpSS = SS_SheetLock(hWnd);
Len = CalcGetExpr(&lpSS->CalcInfo, Col, Row, 
#if SS_V80 // 24919 -scl
                  FALSE,
#endif
                  lpFormula, CALC_MAX_EXPR_LEN);
SS_SheetUnlock(hWnd);
return (Len);

#endif  // defined(SS_OLDCALC)
}

#if SS_V80 // 24919 -scl
int DLLENTRY SSGetExcelFormula(hWnd, Col, Row, lpFormula)

HWND           hWnd;
SS_COORD       Col;
SS_COORD       Row;
LPTSTR         lpFormula;
{
LPSPREADSHEET lpSS;
int         Len = 0;
lpSS = SS_SheetLock(hWnd);
Len = CalcGetExpr(&lpSS->CalcInfo, Col, Row, TRUE,
                  lpFormula, CALC_MAX_EXPR_LEN);
SS_SheetUnlock(hWnd);
return (Len);
}

int DLLENTRY SSGetExcelFormulaLen(hWnd, Col, Row)

HWND          hWnd;
SS_COORD      Col;
SS_COORD      Row;
{
LPSPREADSHEET lpSS;
int           Len = 0;

lpSS = SS_SheetLock(hWnd);
if (Col != SS_ALLCOLS)
   SS_AdjustCellCoords(lpSS, &Col, NULL);
if (Row != SS_ALLROWS)
   SS_AdjustCellCoords(lpSS, NULL, &Row);
Len = CalcGetExprLen(&lpSS->CalcInfo, Col, Row, TRUE);
SS_SheetUnlock(hWnd);

return (Len);
}
#endif

#ifdef SS_OLDCALC
TBGLOBALHANDLE SS_GetFormula(lpSS, Col, Row)

LPSPREADSHEET  lpSS;
SS_COORD       Col;
SS_COORD       Row;
{
TBGLOBALHANDLE hCalc;
TBGLOBALHANDLE hFormula = 0;
LPSS_COL       lpCol;
LPSS_ROW       lpRow;
LPSS_CALC      Calc;
LPSS_CELL      lpCell;

if (Col != SS_ALLCOLS)
   {
   if (lpCol = SS_LockColItem(lpSS, Col))
      {
      if (hCalc = lpCol->hCalc)
         {
         Calc = (LPSS_CALC)tbGlobalLock(hCalc);
         hFormula = Calc->hFormula;
         tbGlobalUnlock(hCalc);
         }

      SS_UnlockColItem(lpSS, Col);
      }
   }

if (Row != SS_ALLROWS)
   {
   if (lpRow = SS_LockRowItem(lpSS, Row))
      {
      if (hCalc = lpRow->hCalc)
         {
         Calc = (LPSS_CALC)tbGlobalLock(hCalc);
         if (Calc->hFormula)
            hFormula = Calc->hFormula;

         tbGlobalUnlock(hCalc);
         }

      SS_UnlockRowItem(lpSS, Row);
      }
   }

if (Col != SS_ALLCOLS && Col != SS_ALLCOLS)
   {
   if (lpCell = SS_LockCellItem(lpSS, Col, Row))
      {
      if (lpCell->hCalc)
         {
         hCalc = lpCell->hCalc;
         Calc = (LPSS_CALC)tbGlobalLock(hCalc);
         if (Calc->hFormula)
            hFormula = Calc->hFormula;

         tbGlobalUnlock(hCalc);
         }
      }
   }

return (hFormula);
}
#endif


#ifdef SS_OLDCALC
BOOL SS_FreeFormula(lpSS, Col, Row)

LPSPREADSHEET  lpSS;
SS_COORD       Col;
SS_COORD       Row;
{
TBGLOBALHANDLE hCalc = 0;
LPSS_CALC      Calc;
LPSS_COL       lpCol;
LPSS_ROW       lpRow;
LPSS_CELL      lpCell;

if (Col != SS_ALLCOLS && Row != SS_ALLROWS)
   {
   if (!(lpCell = SS_LockCellItem(lpSS, Col, Row)))
      return (TRUE);

   hCalc = lpCell->hCalc;
   lpCell->hCalc = 0;
   SS_UnlockCellItem(lpSS, Col, Row);
   }

else if (Col != SS_ALLCOLS && Row == SS_ALLROWS)
   {
   if (Col >= SS_GetColCnt(lpSS))
      return (FALSE);

   if (lpCol = SS_LockColItem(lpSS, Col))
      {
      hCalc = lpCol->hCalc;
      lpCol->hCalc = 0;
      SS_UnlockColItem(lpSS, Col);
      }
   }

else if (Col == SS_ALLCOLS && Row != SS_ALLROWS)
   {
   if (Row >= SS_GetRowCnt(lpSS))
      return (FALSE);

   if (lpRow = SS_LockRowItem(lpSS, Row))
      {
      hCalc = lpRow->hCalc;
      lpRow->hCalc = 0;
      SS_UnlockRowItem(lpSS, Row);
      }
   }

if (hCalc)
   {
   Calc = (LPSS_CALC)tbGlobalLock(hCalc);

   SSx_CalcTableDelete(lpSS, &Calc->CellCoord);

   if (Calc->hFormula)
      tbGlobalFree(Calc->hFormula);

   if (Calc->Ancestors.hItems)
      tbGlobalFree(Calc->Ancestors.hItems);

   tbGlobalUnlock(hCalc);
   tbGlobalFree(hCalc);
   }

return (TRUE);
}
#endif


#else  // defined(SS_NOCALC)

BOOL DLLENTRY SSSetFormula(hWnd, Col, Row, Formula, BuildDependencies)

HWND          hWnd;
SS_COORD      Col;
SS_COORD      Row;
LPCTSTR       Formula;
BOOL          BuildDependencies;
{
return (TRUE);
}


BOOL DLLENTRY SSSetFormulaRange(hWnd, Col, Row, Col2, Row2, Formula,
                                  BuildDependencies)

HWND          hWnd;
SS_COORD      Col;
SS_COORD      Row;
SS_COORD      Col2;
SS_COORD      Row2;
LPCTSTR       Formula;
BOOL          BuildDependencies;
{
return (TRUE);
}


int DLLENTRY SSGetFormula(hWnd, Col, Row, lpFormula)

HWND         hWnd;
SS_COORD     Col;
SS_COORD     Row;
LPTSTR       lpFormula;
{
return (TRUE);
}


int DLLENTRY SSGetFormulaLen(hWnd, Col, Row)

HWND         hWnd;
SS_COORD     Col;
SS_COORD     Row;
{
return (TRUE);
}


BOOL SS_FreeFormula(lpSS, Col, Row)

LPSPREADSHEET lpSS;
SS_COORD      Col;
SS_COORD      Row;
{
return (TRUE);
}

#endif  // !defined(SS_NOCALC)
