/*********************************************************
* SS_XLPARSE.C
*
* Copyright (C) 1999 - FarPoint Technologies
* All Rights Reserved.
*
* No part of this source code may be copied, modified or
* reproduced in any form without retaining the above
* copyright notice.  This source code, or source code
* derived from it, may not be redistributed without
* express written permission of FarPoint Technologies.
*
* UPDATE LOG:
* -----------
*	RAP	- 12.10.98
* RAP01 - GIC10982                                          10.27.99
*********************************************************/
#if defined(SS_V30) && defined(WIN32)

#if ((defined(SS_OCX) || defined(FP_DLL)) && defined(WIN32))
#ifndef WIN32_LEAN_AND_MEAN
#define WIN32_LEAN_AND_MEAN   //exclude conflicts with VBAPI.H in WIN32
#endif
#endif

#include <windows.h>
#include <math.h>
#include <tchar.h>
#if defined(SS_DDE)
#include <oaidl.h>
#endif
#include <stdlib.h>
#include <stdio.h>
#include <string.h>
#include <ole2.h>
#include <memory.h>
#include "..\..\..\..\fplibs\fptools\src\fpconvrt.h"
#include "spread.h"
#include "ss_alloc.h"
#include "ss_bool.h"
#include "ss_calc.h"
#include "ss_col.h"
#include "ss_data.h"
#include "ss_dde.h"
#include "ss_doc.h"
#include "ss_draw.h"
#include "ss_formu.h"
#include "ss_main.h"
#include "ss_row.h"
#include "ss_save.h"
#include "ss_save4.h"
#include "ss_type.h"
#include "ss_user.h"
#include "ss_virt.h"
#include "ss_win.h"
#include "ss_excel.h"

#if !defined(SS_NOCALC) && !defined(SS_OLDCALC)
#include "..\calc\calc.h"
#include "..\calc\cal_cell.h"
#include "..\calc\cal_col.h"
#include "..\calc\cal_cust.h"
#include "..\calc\cal_dde.h"
#include "..\calc\cal_expr.h"
#include "..\calc\cal_mem.h"
#include "..\calc\cal_name.h"
#include "..\calc\cal_oper.h"
#include "..\calc\cal_row.h"
#endif
#include "..\classes\wintools.h"
#include "..\classes\checkbox.h"

//--------------------------------------------------------------------

#define FORMULA_PARTS_MAX 80000
#define FORMULA_PART_MAX 2000

short FP_API xl_ParseRecord(LPSPREADSHEET lpSS, LPSSXL lpss, LPyyFORMULA lpfx, LPTSTR lptstrFx, LPyCELLVAL lpcell,
                            LPBYTE lpbType, LPFMT lpf, LPBYTE lpFx, LPTSTR lptstr, DWORD *pdw, LPWORD pwStrCount);



//---Formula Parsing---//
BOOL BuildInfix(LPTSTR lptstr, LPWORD lpwStrCount, TCHAR tc)
{
  BOOL Ret = FALSE;
  
  _stprintf(&(lptstr[(*lpwStrCount)*FORMULA_PART_MAX]),_T("%s%c%s"),&(lptstr[(*lpwStrCount-2)*FORMULA_PART_MAX]),tc,&(lptstr[(*lpwStrCount-1)*FORMULA_PART_MAX]));
  lstrcpy(&(lptstr[(*lpwStrCount-2)*FORMULA_PART_MAX]), &(lptstr[(*lpwStrCount)*FORMULA_PART_MAX])); 
  (*lpwStrCount)--;

  return Ret;
}

BOOL BuildInfix2(LPTSTR lptstr, LPWORD lpwStrCount, TCHAR tc, TCHAR tc2)
{
  BOOL Ret = FALSE;
  
  _stprintf(&(lptstr[(*lpwStrCount)*FORMULA_PART_MAX]),_T("%s%c%c%s"),&(lptstr[(*lpwStrCount-2)*FORMULA_PART_MAX]),tc,tc2,&(lptstr[(*lpwStrCount-1)*FORMULA_PART_MAX]));
  lstrcpy(&(lptstr[(*lpwStrCount-2)*FORMULA_PART_MAX]), &(lptstr[(*lpwStrCount)*FORMULA_PART_MAX])); 
  (*lpwStrCount)--;

  return Ret;
}

BOOL BuildInfixPopx(LPTSTR lptstr, LPWORD lpwStrCount, TCHAR tc, BYTE bArgs)
{
  BOOL Ret = FALSE;
  WORD wIndex;

  _stprintf(&(lptstr[(*lpwStrCount)*FORMULA_PART_MAX]),_T("%s%c%s"),&(lptstr[(*lpwStrCount-bArgs)*FORMULA_PART_MAX]),tc,&(lptstr[(*lpwStrCount-bArgs+1)*FORMULA_PART_MAX]));
  for (wIndex=2; wIndex<bArgs; wIndex++)
  {
    _stprintf(&(lptstr[(*lpwStrCount)*FORMULA_PART_MAX]),_T("%s%c%s"),&(lptstr[(*lpwStrCount)*FORMULA_PART_MAX]),tc,&(lptstr[(*lpwStrCount-bArgs+wIndex)*FORMULA_PART_MAX]));
  }
  _stprintf(&(lptstr[(*lpwStrCount)*FORMULA_PART_MAX]),_T("%s"),&(lptstr[(*lpwStrCount)*FORMULA_PART_MAX]));
  lstrcpy(&(lptstr[(*lpwStrCount-bArgs)*FORMULA_PART_MAX]), &(lptstr[(*lpwStrCount)*FORMULA_PART_MAX])); 
  (*lpwStrCount) -= (bArgs-1);

  return Ret;
}

BOOL BuildInfixFnx(LPTSTR lptstr, LPWORD lpwStrCount, LPTSTR lptstrOp, BYTE bArgs)
{
  BOOL Ret = FALSE;
  WORD wIndex;

  if (lptstrOp == NULL) //for ptgParen
    _stprintf(&(lptstr[(*lpwStrCount)*FORMULA_PART_MAX]),_T("(%s"), &(lptstr[(*lpwStrCount-bArgs)*FORMULA_PART_MAX]));
  else
    _stprintf(&(lptstr[(*lpwStrCount)*FORMULA_PART_MAX]),_T("%s(%s"),lptstrOp, &(lptstr[(*lpwStrCount-bArgs)*FORMULA_PART_MAX]));
  for (wIndex=bArgs-1; wIndex>0; wIndex--)
  {
    _stprintf(&(lptstr[(*lpwStrCount)*FORMULA_PART_MAX]),_T("%s,%s"),&(lptstr[(*lpwStrCount)*FORMULA_PART_MAX]),&(lptstr[(*lpwStrCount-wIndex)*FORMULA_PART_MAX]));
  }
  _stprintf(&(lptstr[(*lpwStrCount)*FORMULA_PART_MAX]),_T("%s)"),&(lptstr[(*lpwStrCount)*FORMULA_PART_MAX]));
  lstrcpy(&(lptstr[(*lpwStrCount-bArgs)*FORMULA_PART_MAX]), &(lptstr[(*lpwStrCount)*FORMULA_PART_MAX])); 
  (*lpwStrCount) -= (bArgs-1);

  return Ret;
}

#if SS_V80
BOOL BuildExternFnx(LPTSTR lptstr, LPWORD lpwStrCount, BYTE bArgs)
{
  BOOL Ret = FALSE;

//  Ret = BuildInfixFnx(&(lptstr[FORMULA_PART_MAX]), lpwStrCount, lptstr, bArgs);
  Ret = BuildInfixFnx(lptstr, lpwStrCount, lptstr, (BYTE)(bArgs-1));

 // (*lpwStrCount) -= 2;
  (*lpwStrCount)--;

  lstrcpy(&(lptstr[(*lpwStrCount-1)*FORMULA_PART_MAX]), &(lptstr[(*lpwStrCount)*FORMULA_PART_MAX])); 

  return Ret;
}
#endif


BOOL ParseIFParams(LPSPREADSHEET lpSS, LPSSXL lpss, LPyyFORMULA lpfx, LPTSTR lptstrFx, LPyCELLVAL lpcell,
                   LPBYTE lpbType, LPFMT lpf, LPBYTE lpFx, LPTSTR lptstr, DWORD *pdw, LPWORD pwStrCount)
{
  short   nArgCount = 0;
  LPATTR  lpAttr = NULL;
  LPFUNCVAR lpfvv = NULL;
  long    lOffset = 0L;
  DWORD   dw = *pdw;
  DWORD   dwAnchor = 0;
  WORD    wStrCount = *pwStrCount;
  short   nLen = 0;
  short   nErr = FALSE;
  BOOL    fNoFalse = FALSE;
  
  // The condition has already been added to the "converted buffer". Excel follows
  // the condition with a ATTR record containing the offset to the FALSE argument.
  // The TRUE argument follows the ATTR.  Following the TRUE & FALSE arguments is a
  // ATTR record with an offset to the end of the IF statement. The last record in a
  // IF statement is a FuncVarV specifying the number of parameters.
  
  // Process the TRUE argument
  lpAttr = (LPATTR)LPOFFSET(lpFx, dw);
  lOffset = lpAttr->wData-sizeof(ATTR); //the offset to the FALSE argument.
  dw += sizeof(ATTR);
  dwAnchor = dw;
  while (dw - dwAnchor < (DWORD)lOffset)
  {
    nErr = xl_ParseRecord(lpSS, lpss, lpfx, lptstrFx, lpcell, lpbType, lpf, lpFx, lptstr, &dw, &wStrCount);
    if (nErr)
      return (nErr);
  }

  // Process the FALSE argument
  lpAttr = (LPATTR)LPOFFSET(lpFx,dw);
  if (lpAttr->wData+1 >= (sizeof(ATTR)+sizeof(FUNCVAR))) // there is FALSE argument
  {  
    lOffset = lpAttr->wData+1-sizeof(ATTR)-sizeof(FUNCVAR); //the offset to the FuncVarV block.
    dw += sizeof(ATTR);
    dwAnchor = dw;
    while (dw - dwAnchor < (DWORD)lOffset)
    {
      nErr = xl_ParseRecord(lpSS, lpss, lpfx, lptstrFx, lpcell, lpbType, lpf, lpFx, lptstr, &dw, &wStrCount);
      if (nErr)
        return (nErr);
    }
  }
  else
    fNoFalse = TRUE; //

  // Build the IF record and tie things up
  dw += sizeof(ATTR);
  lpfvv = (LPFUNCVAR)LPOFFSET(lpFx,dw);

  if (lpfvv->cargs == 2 || fNoFalse)
  {
    if (lpfvv->cargs == 2)
      lpfvv->cargs++;
    _tcscpy(&(lptstr[wStrCount*FORMULA_PART_MAX]), _T("\"\""));
//    _tcscpy(&(lptstr[wStrCount*FORMULA_PART_MAX]), _T("0"));
    wStrCount++;
  }
  BuildInfixFnx(lptstr, (LPWORD)&wStrCount, _T("IF"), lpfvv->cargs);

  *pdw = dw;
  *pwStrCount = wStrCount;

  return nErr;
}


short FP_API xl_ParseRecord(LPSPREADSHEET lpSS, LPSSXL lpss, LPyyFORMULA lpfx, LPTSTR lptstrFx, LPyCELLVAL lpcell,
                            LPBYTE lpbType, LPFMT lpf, LPBYTE lpFx, LPTSTR lptstr, DWORD *pdw, LPWORD pwStrCount)
{
  DWORD  dw = *pdw;
  WORD   wStrCount = *pwStrCount;
  short  nErr = FALSE;

  if (lpFx == NULL)
    return nErr;

  switch (lpFx[dw])
  {
    //Values
    case ptgInt:
      dw++;
      fpLongToString(*(LPWORD)(lpFx+dw), &(lptstr[wStrCount*FORMULA_PART_MAX]));
//      _stprintf(&(lptstr[wStrCount*FORMULA_PART_MAX]), _T("%d"), *(LPWORD)(lpFx+dw));
      wStrCount++;
      dw += sizeof(WORD);
    break;
    case ptgNum:
      dw++;
      fpDoubleToString(&(lptstr[wStrCount*FORMULA_PART_MAX]), *(double FAR *)(lpFx+dw),
                       15, (TCHAR)'.', 0, TRUE, FALSE, TRUE, FALSE, 0, NULL);
//      _stprintf(&(lptstr[wStrCount*FORMULA_PART_MAX]), _T("%f"), *(double FAR *)(lpFx+dw));
      wStrCount++;
      dw += sizeof(double);
    break;
    case ptgBool:
    {
      BYTE Bool = *(LPBYTE)LPOFFSET(lpFx, dw+1);
      dw++;
      _stprintf(&(lptstr[wStrCount*FORMULA_PART_MAX]), _T("%s"), Bool?_T("1"):_T("0"));
      wStrCount++;
      dw += sizeof(BYTE);
    }
    break;
    case ptgErr:
    {
      BYTE Err = *(LPBYTE)LPOFFSET(lpFx, dw+1);
       
      switch (Err)
      {
        case 0:
          _tcscpy(&(lptstr[wStrCount*FORMULA_PART_MAX]), _T("#NULL!"));
          wStrCount++;
          dw += sizeof(BYTE);
          break;
        case 7:
          _tcscpy(&(lptstr[wStrCount*FORMULA_PART_MAX]), _T("#DIV/0!"));
          wStrCount++;
          dw += sizeof(BYTE);
          break;
        case 15:
          _tcscpy(&(lptstr[wStrCount*FORMULA_PART_MAX]), _T("#VALUE!"));
          wStrCount++;
          dw += sizeof(BYTE);
          break;
        case 23:
          _tcscpy(&(lptstr[wStrCount*FORMULA_PART_MAX]), _T("#REF!"));
          wStrCount++;
          dw += sizeof(BYTE);
          break;
        case 29:
          _tcscpy(&(lptstr[wStrCount*FORMULA_PART_MAX]), _T("#NAME?"));
          wStrCount++;
          dw += sizeof(BYTE);
          break;
        case 36:
          _tcscpy(&(lptstr[wStrCount*FORMULA_PART_MAX]), _T("#NUM!"));
          wStrCount++;
          dw += sizeof(BYTE);
          break;
        case 42:
          _tcscpy(&(lptstr[wStrCount*FORMULA_PART_MAX]), _T("#N/A"));
          wStrCount++;
          dw += sizeof(BYTE);
          break;
      }
      dw++;
    }
    break;
    case ptgMemErr:
    case ptgMemErrV:
    case ptgMemErrA:
      _tcscpy(&(lptstr[wStrCount*FORMULA_PART_MAX]), _T("#REF!"));
      dw++;
      wStrCount++;
      dw += 6;
      break;
    case ptgAreaErr:
    case ptgAreaErrV:
    case ptgAreaErrA:
      _tcscpy(&(lptstr[wStrCount*FORMULA_PART_MAX]), _T("#REF!"));
      dw++;
      wStrCount++;
      dw += 8;
      break;
    case ptgAreaErr3d:
    case ptgAreaErr3dV:
    case ptgAreaErr3dA:
    {
//      int externSheetIndex = formulaReader.ReadInt16();
//      int r = formulaReader.ReadInt16();
//      int grbit = formulaReader.ReadInt16();
      _tcscpy(&(lptstr[wStrCount*FORMULA_PART_MAX]), _T("#REF!"));
      dw++;
      wStrCount++;
      dw += 10;
    } 
      break;
    case ptgRefErr:
    case ptgRefErrV:
    case ptgRefErrA:
      _tcscpy(&(lptstr[wStrCount*FORMULA_PART_MAX]), _T("#REF!"));
      dw++;
      wStrCount++;
      dw += 4;
      break;
    case ptgRefErr3d:
    case ptgRefErr3dV:
    {
//      int  externSheetIndex = formulaReader.ReadInt16();
//      int  r = formulaReader.ReadInt16();
//      int  grbit = formulaReader.ReadInt16();
      _tcscpy(&(lptstr[wStrCount*FORMULA_PART_MAX]), _T("#REF!"));
      dw++;
      wStrCount++;
      dw += 6;
    }            
      break;

    case ptgConcat:
    {
      BuildInfixFnx(lptstr, (LPWORD)&wStrCount, _T("CONCATENATE"), 2);
      dw++;
    }
    break;    

    //Arithmetic Operators
    case ptgAdd:
      BuildInfix(lptstr, (LPWORD)&wStrCount, (TCHAR)'+');
      dw++;
    break;
    case ptgSub:
      BuildInfix(lptstr, (LPWORD)&wStrCount, (TCHAR)'-');
      dw++;
    break;
    case ptgMul:
      BuildInfix(lptstr, (LPWORD)&wStrCount, (TCHAR)'*');
      dw++;
    break;
    case ptgPercent:
      // let the percent drop-thru to the divide processing.
      _stprintf(&(lptstr[wStrCount*FORMULA_PART_MAX]), _T("%d"), 100);
      wStrCount++;
      *lpbType = PERCENTTYPE;
    case ptgDiv:
      BuildInfix(lptstr, (LPWORD)&wStrCount, (TCHAR)'/');
      dw++;
    break;
    case ptgParen:
      BuildInfixFnx(lptstr, (LPWORD)&wStrCount, NULL, 1);
      dw++;
    break;
    case ptgPower:
      BuildInfix(lptstr, (LPWORD)&wStrCount, (TCHAR)'^');
      dw++;
    break;
    case ptgRange:
      BuildInfix(lptstr, (LPWORD)&wStrCount, (TCHAR)':');
      dw++;
    break;
    case ptgUplus:
      // Don't do anything with a unary plus.  Skip it and continue.
      dw++;
    break;
    case ptgUminus:
      BuildInfixFnx(lptstr, (LPWORD)&wStrCount, _T("NEG"), 1);
      dw++;
     break;
    case ptgUnion:
      BuildInfix(lptstr, (LPWORD)&wStrCount, (TCHAR)',');
      dw++;
    break;
    //Binary Operators  
    case ptgLT:
      BuildInfix(lptstr, (LPWORD)&wStrCount, (TCHAR)'<');
      dw++;
    break;
    case ptgLE:
      BuildInfix2(lptstr, (LPWORD)&wStrCount, (TCHAR)'<',(TCHAR)'=');
      dw++;
    break;
    case ptgEQ:
      BuildInfix(lptstr, (LPWORD)&wStrCount, (TCHAR)'=');
      dw++;
    break;
    case ptgGE:
      BuildInfix2(lptstr, (LPWORD)&wStrCount, (TCHAR)'>',(TCHAR)'=');
      dw++;
    break;
    case ptgGT:
      BuildInfix(lptstr, (LPWORD)&wStrCount, (TCHAR)'>');
      dw++;
    break;
    case ptgNE:
      BuildInfix2(lptstr, (LPWORD)&wStrCount, (TCHAR)'<',(TCHAR)'>');
      dw++;
    break;
   //Function Operators 
    case ptgFuncV:
    {
      *lpbType = FLOATTYPE;
      switch(*(LPWORD)LPOFFSET(lpFx, dw+1))
      {
#ifdef SS_V80
        case xlfBinomdist:
          BuildInfixFnx(lptstr, (LPWORD)&wStrCount, _T("BINOMDIST"), 4);
        break;
        case xlfChidist:
          BuildInfixFnx(lptstr, (LPWORD)&wStrCount, _T("CHIDIST"), 2);
        break;
        case xlfChiinv:
          BuildInfixFnx(lptstr, (LPWORD)&wStrCount, _T("CHIINV"), 2);
        break;
        case xlfConfidence:
          BuildInfixFnx(lptstr, (LPWORD)&wStrCount, _T("CONFIDENCE"), 3);
        break;
        case xlfCountif:
          BuildInfixFnx(lptstr, (LPWORD)&wStrCount, _T("COUNTIF"), 2);
        break;
        case xlfCritbinom:
          BuildInfixFnx(lptstr, (LPWORD)&wStrCount, _T("CRITBINOM"), 3);
        break;
        case xlfExpondist:
          BuildInfixFnx(lptstr, (LPWORD)&wStrCount, _T("EXPONDIST"), 3);
        break;
        case xlfFdist:
          BuildInfixFnx(lptstr, (LPWORD)&wStrCount, _T("FDIST"), 3);
        break;
        case xlfFinv:
          BuildInfixFnx(lptstr, (LPWORD)&wStrCount, _T("FINV"), 3);
        break;
        case xlfFisher:
          BuildInfixFnx(lptstr, (LPWORD)&wStrCount, _T("FISHER"), 1);
        break;
        case xlfFisherinv:
          BuildInfixFnx(lptstr, (LPWORD)&wStrCount, _T("FISHERINV"), 1);
        break;
        case xlfGammadist:
          BuildInfixFnx(lptstr, (LPWORD)&wStrCount, _T("GAMMADIST"), 4);
        break;
        case xlfGammainv:
          BuildInfixFnx(lptstr, (LPWORD)&wStrCount, _T("GAMMAINV"), 3);
        break;
        case xlfGammaln:
          BuildInfixFnx(lptstr, (LPWORD)&wStrCount, _T("GAMMALN"), 1);
        break;
        case xlfHypgeomdist:
          BuildInfixFnx(lptstr, (LPWORD)&wStrCount, _T("HYPGEOMDIST"), 4);
        break;
        case xlfIspmt:
          BuildInfixFnx(lptstr, (LPWORD)&wStrCount, _T("ISPMT"), 4);
        break;
        case xlfLoginv:
          BuildInfixFnx(lptstr, (LPWORD)&wStrCount, _T("LOGINV"), 3);
        break;
        case xlfLognormdist:
          BuildInfixFnx(lptstr, (LPWORD)&wStrCount, _T("LOGNORMDIST"), 3);
        break;
        case xlfMirr:
          BuildInfixFnx(lptstr, (LPWORD)&wStrCount, _T("MIRR"), 3);
        break;
        case xlfNegbinomdist:
          BuildInfixFnx(lptstr, (LPWORD)&wStrCount, _T("NEGBINOMDIST"), 3);
        break;
        case xlfNormdist:
          BuildInfixFnx(lptstr, (LPWORD)&wStrCount, _T("NORMDIST"), 4);
        break;
        case xlfNorminv:
          BuildInfixFnx(lptstr, (LPWORD)&wStrCount, _T("NORMINV"), 3);
        break;
        case xlfNormsdist:
          BuildInfixFnx(lptstr, (LPWORD)&wStrCount, _T("NORMSDIST"), 1);
        break;
        case xlfNormsinv:
          BuildInfixFnx(lptstr, (LPWORD)&wStrCount, _T("NORMSINV"), 1);
        break;
        case xlfPoisson:
          BuildInfixFnx(lptstr, (LPWORD)&wStrCount, _T("POISSON"), 3);
        break;
        case xlfStandardize:
          BuildInfixFnx(lptstr, (LPWORD)&wStrCount, _T("STANDARDIZE"), 3);
        break;
        case xlfTdist:
          BuildInfixFnx(lptstr, (LPWORD)&wStrCount, _T("TDIST"), 3);
        break;
        case xlfTinv:
          BuildInfixFnx(lptstr, (LPWORD)&wStrCount, _T("TINV"), 2);
        break;
        case xlfWeibull:
          BuildInfixFnx(lptstr, (LPWORD)&wStrCount, _T("WEIBULL"), 4);
        break;

#endif
        case xlfAbs:
          BuildInfixFnx(lptstr, (LPWORD)&wStrCount, _T("ABS"), 1);
        break;
        case xlfAcos:
          BuildInfixFnx(lptstr, (LPWORD)&wStrCount, _T("ACOS"), 1);
        break;
        case xlfAsin:
          BuildInfixFnx(lptstr, (LPWORD)&wStrCount, _T("ASIN"), 1);
        break;
        case xlfAtan:
          BuildInfixFnx(lptstr, (LPWORD)&wStrCount, _T("ATAN"), 1);
        break;
        case xlfAtan2:
          BuildInfixFnx(lptstr, (LPWORD)&wStrCount, _T("ATAN2"), 2);
        break;
        case xlfCeiling:
          BuildInfixFnx(lptstr, (LPWORD)&wStrCount, _T("CEILING"), 2);
        break;
        case xlfCombin:
          BuildInfixFnx(lptstr, (LPWORD)&wStrCount, _T("COMBIN"), 2);
        break;
        case xlfCos:
          BuildInfixFnx(lptstr, (LPWORD)&wStrCount, _T("COS"), 1);
        break;
        case xlfCosh:
          BuildInfixFnx(lptstr, (LPWORD)&wStrCount, _T("COSH"), 1);
        break;
        case xlfDegrees:
          BuildInfixFnx(lptstr, (LPWORD)&wStrCount, _T("DEGREES"), 1);
        break;
        case xlfEven:
          BuildInfixFnx(lptstr, (LPWORD)&wStrCount, _T("EVEN"), 1);
        break;
        case xlfExp:
          BuildInfixFnx(lptstr, (LPWORD)&wStrCount, _T("EXP"), 1);
        break;
        case xlfFact:
          BuildInfixFnx(lptstr, (LPWORD)&wStrCount, _T("FACT"), 1);
        break;
        case xlfFalse:
          lstrcpy(&(lptstr[wStrCount*FORMULA_PART_MAX]), _T("FALSE()"));
          wStrCount++;
        break;
        case xlfFloor:
          BuildInfixFnx(lptstr, (LPWORD)&wStrCount, _T("FLOOR"), 2);
        break;
        case xlfInt:
          BuildInfixFnx(lptstr, (LPWORD)&wStrCount, _T("INT"), 1);
        break;
//          case xlfLog:
//            BuildInfixFnx(lptstr, (LPWORD)&wStrCount, _T("LOG"), 2);
//          break;
#ifndef SS_V35
        case xlfIsblank:
          BuildInfixFnx(lptstr, (LPWORD)&wStrCount, _T("ISEMPTY"), 1);
        break;
#endif
        case xlfIsnontext:
          BuildInfixFnx(lptstr, (LPWORD)&wStrCount, _T("ISNONTEXT"), 1);
        break;
        case xlfIsnumber:
          BuildInfixFnx(lptstr, (LPWORD)&wStrCount, _T("ISNUMBER"), 1);
        break;
        case xlfIsref:
          BuildInfixFnx(lptstr, (LPWORD)&wStrCount, _T("ISREF"), 1);
        break;
        case xlfIstext:
          BuildInfixFnx(lptstr, (LPWORD)&wStrCount, _T("ISTEXT"), 1);
        break;
        case xlfLog10:
          BuildInfixFnx(lptstr, (LPWORD)&wStrCount, _T("LOG10"), 1);
        break;
        case xlfMod:
         BuildInfixFnx(lptstr, (LPWORD)&wStrCount, _T("MOD"), 2);
        break;
        case xlfLn:
          BuildInfixFnx(lptstr, (LPWORD)&wStrCount, _T("LN"), 1);
        break;
        case xlfNot:
          BuildInfixFnx(lptstr, (LPWORD)&wStrCount, _T("NOT"), 1);
        break;
        case xlfOdd:
          BuildInfixFnx(lptstr, (LPWORD)&wStrCount, _T("ODD"), 1);
        break;
        case xlfPermut:
          BuildInfixFnx(lptstr, (LPWORD)&wStrCount, _T("PERMUT"), 2);
        break;
        case xlfPi:
          lstrcpy(&(lptstr[wStrCount*FORMULA_PART_MAX]), _T("PI()"));
          wStrCount++;
        break;
        case xlfPower:
          BuildInfixFnx(lptstr, (LPWORD)&wStrCount, _T("POWER"), 2);
        break;
        case xlfRadians:          BuildInfixFnx(lptstr, (LPWORD)&wStrCount, _T("RADIANS"), 1);
        break;
        case xlfRand:
          lstrcpy(&(lptstr[wStrCount*FORMULA_PART_MAX]), _T("RAND()"));
          wStrCount++;
        break;
        case xlfRound:
          BuildInfixFnx(lptstr, (LPWORD)&wStrCount, _T("ROUND"), 2);
        break;
        case xlfRounddown:
          BuildInfixFnx(lptstr, (LPWORD)&wStrCount, _T("ROUNDDOWN"), 2);
        break;
        case xlfRoundup:
          BuildInfixFnx(lptstr, (LPWORD)&wStrCount, _T("ROUNDUP"), 2);
        break;
        case xlfSign:
          BuildInfixFnx(lptstr, (LPWORD)&wStrCount, _T("SIGN"), 1);
        break;
        case xlfSin:
          BuildInfixFnx(lptstr, (LPWORD)&wStrCount, _T("SIN"), 1);
        break;
        case xlfSinh:
          BuildInfixFnx(lptstr, (LPWORD)&wStrCount, _T("SINH"), 1);
        break;
        case xlfSln:
          BuildInfixFnx(lptstr, (LPWORD)&wStrCount, _T("SLN"), 3);
        break;
        case xlfSqrt:
          BuildInfixFnx(lptstr, (LPWORD)&wStrCount, _T("SQRT"), 1);
        break;
        case xlfSyd:
          BuildInfixFnx(lptstr, (LPWORD)&wStrCount, _T("SYD"), 4);
        break;
        case xlfTan:
          BuildInfixFnx(lptstr, (LPWORD)&wStrCount, _T("TAN"), 1);
        break;
        case xlfTanh:
          BuildInfixFnx(lptstr, (LPWORD)&wStrCount, _T("TANH"), 1);
        break;
        case xlfTrue:
          lstrcpy(&(lptstr[wStrCount*FORMULA_PART_MAX]), _T("TRUE()"));
          wStrCount++;
        break;
#ifdef SS_V35
        case xlfAcosh:
          BuildInfixFnx(lptstr, (LPWORD)&wStrCount, _T("ACOSH"), 1);
        break;
        case xlfAsinh:
          BuildInfixFnx(lptstr, (LPWORD)&wStrCount, _T("ASINH"), 1);
        break;
        case xlfAtanh:
          BuildInfixFnx(lptstr, (LPWORD)&wStrCount, _T("ATANH"), 1);
        break;
        case xlfChar:
          BuildInfixFnx(lptstr, (LPWORD)&wStrCount, _T("CHAR"), 1);
          *lpbType = STRINGTYPE;
        break;
        case xlfClean:
          BuildInfixFnx(lptstr, (LPWORD)&wStrCount, _T("CLEAN"), 1);
          *lpbType = STRINGTYPE;
        break;
        case xlfCode:
          BuildInfixFnx(lptstr, (LPWORD)&wStrCount, _T("CODE"), 1);
        break;
        case xlfDate:
          BuildInfixFnx(lptstr, (LPWORD)&wStrCount, _T("DATE"), 3);
        break;
        case xlfDay:
          BuildInfixFnx(lptstr, (LPWORD)&wStrCount, _T("DAY"), 1);
        break;
        case xlfExact:
          BuildInfixFnx(lptstr, (LPWORD)&wStrCount, _T("EXACT"), 2);
        break;
        case xlfFind:
          BuildInfixFnx(lptstr, (LPWORD)&wStrCount, _T("find"), 3);
        break;
        case xlfHour:
          BuildInfixFnx(lptstr, (LPWORD)&wStrCount, _T("HOUR"), 1);
        break;
        case xlfIsblank:
          BuildInfixFnx(lptstr, (LPWORD)&wStrCount, _T("ISBLANK"), 1);
        break;
        case xlfLen:
          BuildInfixFnx(lptstr, (LPWORD)&wStrCount, _T("LEN"), 1);
        break;
        case xlfLower:
          BuildInfixFnx(lptstr, (LPWORD)&wStrCount, _T("LOWER"), 1);
          *lpbType = STRINGTYPE;
        break;
        case xlfMid:
          BuildInfixFnx(lptstr, (LPWORD)&wStrCount, _T("MID"), 3);
          *lpbType = STRINGTYPE;
        break;
        case xlfMinute:
          BuildInfixFnx(lptstr, (LPWORD)&wStrCount, _T("MINUTE"), 1);
        break;
        case xlfMonth:
          BuildInfixFnx(lptstr, (LPWORD)&wStrCount, _T("MONTH"), 1);
        break;
        case xlfNow:
          lstrcpy(&(lptstr[wStrCount*FORMULA_PART_MAX]), _T("NOW()"));
          wStrCount++;
        break;
        case xlfProper:
          BuildInfixFnx(lptstr, (LPWORD)&wStrCount, _T("PROPER"), 1);
          *lpbType = STRINGTYPE;
        break;
        case xlfReplace:
          BuildInfixFnx(lptstr, (LPWORD)&wStrCount, _T("REPLACE"), 4);
          *lpbType = STRINGTYPE;
        break;
        case xlfRept:
          BuildInfixFnx(lptstr, (LPWORD)&wStrCount, _T("REPT"), 2);
          *lpbType = STRINGTYPE;
        break;
        case xlfSecond:
          BuildInfixFnx(lptstr, (LPWORD)&wStrCount, _T("SECOND"), 1);
        break;
        case xlfTime:
          BuildInfixFnx(lptstr, (LPWORD)&wStrCount, _T("TIME"), 3);
          *lpbType = TIMETYPE;
        break;
        case xlfToday:
          lstrcpy(&(lptstr[wStrCount*FORMULA_PART_MAX]), _T("TODAY()"));
          wStrCount++;
          *lpbType = DATETYPE;
        break;
        case xlfTrim:
          BuildInfixFnx(lptstr, (LPWORD)&wStrCount, _T("TRIM"), 1);
          *lpbType = STRINGTYPE;
        break;
        case xlfUpper:
          BuildInfixFnx(lptstr, (LPWORD)&wStrCount, _T("UPPER"), 1);
          *lpbType = STRINGTYPE;
        break;
        case xlfYear:
          BuildInfixFnx(lptstr, (LPWORD)&wStrCount, _T("YEAR"), 1);
        break;
#endif
        default:
          xl_LogFile(lpSS->lpBook, LOG_FUNCNOTSUPPORTED, (lpcell?lpcell->wCol+1:0), (lpcell?lpcell->wRow+1:0), LPOFFSET(lpFx, dw+1));
          nErr = TRUE;
        break;
      }
      xl_LogFile(lpSS->lpBook, LOG_FUNCFORMCELLTYPE, (lpcell?lpcell->wCol+1:0), (lpcell?lpcell->wRow+1:0), NULL);
      dw += sizeof(BYTE) + sizeof(WORD);
    }
    break;

    case ptgFuncVar:
    case ptgFuncVarV:
    {
      LPFUNCVAR lpfv = (LPFUNCVAR)LPOFFSET(lpFx, dw);

      *lpbType = FLOATTYPE;
      switch(lpfv->iftab)
      {
#if SS_V80
        case xlfAvedev:
          BuildInfixFnx(lptstr, (LPWORD)&wStrCount, _T("AVEDEV"), lpfv->cargs);
        break;
        case xlfBetadist:
          BuildInfixFnx(lptstr, (LPWORD)&wStrCount, _T("BETADIST"), lpfv->cargs);
        break;
        case xlfDays360:
          BuildInfixFnx(lptstr, (LPWORD)&wStrCount, _T("DAYS360"), lpfv->cargs);
        break;
        case xlfDevsq:
          BuildInfixFnx(lptstr, (LPWORD)&wStrCount, _T("DEVSQ"), lpfv->cargs);
        break;
        case xlfNpv:
          BuildInfixFnx(lptstr, (LPWORD)&wStrCount, _T("NPV"), lpfv->cargs);
        break;
        case xlfIpmt:
          BuildInfixFnx(lptstr, (LPWORD)&wStrCount, _T("IPMT"), lpfv->cargs);
        break;
        case xlfIrr:
          BuildInfixFnx(lptstr, (LPWORD)&wStrCount, _T("IRR"), lpfv->cargs);
        break;
        case xlfPpmt:
          BuildInfixFnx(lptstr, (LPWORD)&wStrCount, _T("PPMT"), lpfv->cargs);
        break;
        case xlfVdb:
          BuildInfixFnx(lptstr, (LPWORD)&wStrCount, _T("VDB"), lpfv->cargs);
        break;
        case xlfCount:
          BuildInfixFnx(lptstr, (LPWORD)&wStrCount, _T("COUNT"), lpfv->cargs);
        break;
        case xlfCounta:
          BuildInfixFnx(lptstr, (LPWORD)&wStrCount, _T("COUNTA"), lpfv->cargs);
        break;
        case xlfSubtotal:
          BuildInfixFnx(lptstr, (LPWORD)&wStrCount, _T("SUBTOTAL"), lpfv->cargs);
        break;
        case xlfSumproduct:
          BuildInfixFnx(lptstr, (LPWORD)&wStrCount, _T("SUMPRODUCT"), lpfv->cargs);
        break;
        case xlfSkew:
          BuildInfixFnx(lptstr, (LPWORD)&wStrCount, _T("SKEW"), lpfv->cargs);
        break;
        case xlfBetainv:
          BuildInfixFnx(lptstr, (LPWORD)&wStrCount, _T("BETAINV"), lpfv->cargs);
        break;
        case xlfSumif:
          BuildInfixFnx(lptstr, (LPWORD)&wStrCount, _T("SUMIF"), lpfv->cargs);
        break;
        case 0xff:
          //Extern or custom function
          BuildExternFnx(lptstr, (LPWORD)&wStrCount, lpfv->cargs);
        break;
#endif
        case xlfAnd:
          if (lpfv->cargs > 1)
            BuildInfixPopx(lptstr, (LPWORD)&wStrCount, (TCHAR)'&', lpfv->cargs);
        break;
        case xlfAverage:
          BuildInfixFnx(lptstr, (LPWORD)&wStrCount, _T("AVERAGE"), lpfv->cargs);
        break;
        case xlfDb:
          BuildInfixFnx(lptstr, (LPWORD)&wStrCount, _T("DB"), lpfv->cargs);
        break;
        case xlfDdb:
          BuildInfixFnx(lptstr, (LPWORD)&wStrCount, _T("DDB"), lpfv->cargs);
        break;
        case xlfFv:
          BuildInfixFnx(lptstr, (LPWORD)&wStrCount, _T("FV"), lpfv->cargs);
        break;
        case xlfLog:
          BuildInfixFnx(lptstr, (LPWORD)&wStrCount, _T("LOG"), lpfv->cargs);
        break;
        case xlfMax:
          BuildInfixFnx(lptstr, (LPWORD)&wStrCount, _T("MAX"), lpfv->cargs);
        break;
        case xlfMedian:
          BuildInfixFnx(lptstr, (LPWORD)&wStrCount, _T("MEDIAN"), lpfv->cargs);
        break;
        case xlfMin:
          BuildInfixFnx(lptstr, (LPWORD)&wStrCount, _T("MIN"), lpfv->cargs);
        break;
        case xlfMode:
          BuildInfixFnx(lptstr, (LPWORD)&wStrCount, _T("MODE"), lpfv->cargs);
        break;
        case xlfNper:
          BuildInfixFnx(lptstr, (LPWORD)&wStrCount, _T("NPER"), lpfv->cargs);
        break;
        case xlfOr:
          if (lpfv->cargs > 1)
#if SS_V80 // 24919 -scl
            BuildInfixFnx(lptstr, (LPWORD)&wStrCount, _T("OR"), lpfv->cargs);
#else
            BuildInfixPopx(lptstr, (LPWORD)&wStrCount, (TCHAR)'|', lpfv->cargs);
#endif
        break;
        case xlfProduct:
          BuildInfixFnx(lptstr, (LPWORD)&wStrCount, _T("PRODUCT"), lpfv->cargs);
        break;
        case xlfPmt:
          _stprintf(&(lptstr[wStrCount*FORMULA_PART_MAX]),_T("(-%s(%s,%s*100,%s,12))"),_T("PMT"),&(lptstr[(wStrCount-(lpfv->cargs-2))*FORMULA_PART_MAX]),
                                                               &(lptstr[(wStrCount-lpfv->cargs)*FORMULA_PART_MAX]),
                                                               &(lptstr[(wStrCount-(lpfv->cargs-1))*FORMULA_PART_MAX]));
          lstrcpy(&(lptstr[(wStrCount-lpfv->cargs)*FORMULA_PART_MAX]), &(lptstr[wStrCount*FORMULA_PART_MAX])); 
          wStrCount -= (lpfv->cargs-1);
          xl_LogFile(lpSS->lpBook, LOG_PMTFUNCTIONLOAD, (lpcell?lpcell->wCol+1:0), (lpcell?lpcell->wRow+1:0), NULL);
        break;
        case xlfPv:
          BuildInfixFnx(lptstr, (LPWORD)&wStrCount, _T("PV"), lpfv->cargs);
        break;
        case xlfRank:
          BuildInfixFnx(lptstr, (LPWORD)&wStrCount, _T("RANK"), lpfv->cargs);
        break;
        case xlfStdev:
          BuildInfixFnx(lptstr, (LPWORD)&wStrCount, _T("STDEV"), lpfv->cargs);
        break;
        case xlfStdevp:
          BuildInfixFnx(lptstr, (LPWORD)&wStrCount, _T("STDEVP"), lpfv->cargs);
        break;
        case xlfSum:
          BuildInfixFnx(lptstr, (LPWORD)&wStrCount, _T("SUM"), lpfv->cargs);
        break;
        case xlfSumsq:
          BuildInfixFnx(lptstr, (LPWORD)&wStrCount, _T("SUMSQ"), lpfv->cargs);
        break;
#ifndef SS_V35
        case xlfTrunc:
          if (lpfv->cargs == 2)
          {
            BuildInfixFnx(lptstr, (LPWORD)&wStrCount, _T("TRUNCATE"), lpfv->cargs);
          }
          else
          {
            _stprintf(&(lptstr[wStrCount*FORMULA_PART_MAX]),_T("%s(%s,0)"),_T("TRUNCATE"), &(lptstr[(wStrCount-lpfv->cargs)*FORMULA_PART_MAX]));
            lstrcpy(&(lptstr[(wStrCount-lpfv->cargs)*FORMULA_PART_MAX]), &(lptstr[wStrCount*FORMULA_PART_MAX])); 
            xl_LogFile(lpSS->lpBook, LOG_TRUNCFUNCTIONLOAD, (lpcell?lpcell->wCol+1:0), (lpcell?lpcell->wRow+1:0), NULL);
          }
        break;
#endif
        case xlfVar:
          BuildInfixFnx(lptstr, (LPWORD)&wStrCount, _T("VAR"), lpfv->cargs);
        break;
        case xlfVarp:
          BuildInfixFnx(lptstr, (LPWORD)&wStrCount, _T("VARP"), lpfv->cargs);
        break;
#ifdef SS_V35
        case xlfConcatenate:
          BuildInfixFnx(lptstr, (LPWORD)&wStrCount, _T("CONCATENATE"), lpfv->cargs);
          *lpbType = STRINGTYPE;
        break;
        case xlfFind:
          BuildInfixFnx(lptstr, (LPWORD)&wStrCount, _T("FIND"), lpfv->cargs);
        break;
        case xlfLeft:
          BuildInfixFnx(lptstr, (LPWORD)&wStrCount, _T("LEFT"), lpfv->cargs);
          *lpbType = STRINGTYPE;
        break;
        case xlfRight:
          BuildInfixFnx(lptstr, (LPWORD)&wStrCount, _T("RIGHT"), lpfv->cargs);
          *lpbType = STRINGTYPE;
        break;
/*
        case xlfSearch:
          BuildInfixFnx(lptstr, (LPWORD)&wStrCount, _T("SEARCH"), lpfv->cargs);
        break;
*/
        case xlfSubstitute:
          BuildInfixFnx(lptstr, (LPWORD)&wStrCount, _T("SUBSTITUTE"), lpfv->cargs);
          *lpbType = STRINGTYPE;
        break;
        case xlfTrunc:
          if (lpfv->cargs == 2)
          {
            BuildInfixFnx(lptstr, (LPWORD)&wStrCount, _T("TRUNC"), lpfv->cargs);
          }
          else
          {
            _stprintf(&(lptstr[wStrCount*FORMULA_PART_MAX]),_T("%s(%s,0)"),_T("TRUNC"), &(lptstr[(wStrCount-lpfv->cargs)*FORMULA_PART_MAX]));
            lstrcpy(&(lptstr[(wStrCount-lpfv->cargs)*FORMULA_PART_MAX]), &(lptstr[wStrCount*FORMULA_PART_MAX])); 
            xl_LogFile(lpSS->lpBook, LOG_TRUNCFUNCTIONLOAD, (lpcell?lpcell->wCol+1:0), (lpcell?lpcell->wRow+1:0), NULL);
          }
        break;
        case xlfWeekday:
          BuildInfixFnx(lptstr, (LPWORD)&wStrCount, _T("WEEKDAY"), lpfv->cargs);
        break;
#endif
        default:
        {
          short sFn = (short)(lpfv->iftab);
          xl_LogFile(lpSS->lpBook, LOG_FUNCNOTSUPPORTED, (lpcell?lpcell->wCol+1:0), (lpcell?lpcell->wRow+1:0), &sFn);
          nErr = TRUE;
        }
        break;
      }
      xl_LogFile(lpSS->lpBook, LOG_FUNCFORMCELLTYPE, (lpcell?lpcell->wCol+1:0), (lpcell?lpcell->wRow+1:0), NULL);
      dw += sizeof(FUNCVAR);
    } 
    break;
   //Spreadsheet Name Values
    case ptgArrayV:
    break;
    case ptgRefN:
    case ptgRefNV:
    case ptgRef:
    case ptgRefV:
    case ptgRefA:
    {
      LPCELLREF8  lpCR = (LPCELLREF8)LPOFFSET(lpFx,dw);
      SS_CELLTYPE ct;
      short       sRefStyle = SSGetRefStyle(lpSS->lpBook->hWnd);
      RWCO        RwCo;
      BYTE        bCol = 0;
      WORD        wRow = 0;

      if (!lpcell)
      {
        nErr = TRUE;
        break;
      }

      bCol = (BYTE)lpCR->col;
      wRow = lpCR->rw;
      // if this is a shared formula, the ptgRef row & column are stored
      // as offsets rather than cell references.  Therefore we need to add
      // the reference cell.
      if (lpFx[dw] == ptgRefN || lpFx[dw] == ptgRefNV)
      {
        if (lpCR->fColRel)
          bCol += lpcell->wCol;
        if (lpCR->fRwRel)
          wRow += lpcell->wRow;
      }

      SSGetCellType(lpSS->lpBook->hWnd, lpcell->wCol+1, lpcell->wRow+1, (LPSS_CELLTYPE)&ct);
      if (ct.Type == SS_TYPE_FLOAT)
        *lpbType = FLOATTYPE;
      else if (ct.Type == SS_TYPE_DATE)
        *lpbType = DATETYPE;
      else if (ct.Type == SS_TYPE_TIME)
        *lpbType = TIMETYPE;
       
      RwCo.col = bCol;
      RwCo.row = (short)wRow;
      RwCo.fColRel = lpCR->fColRel;
      RwCo.fRwRel = lpCR->fRwRel;

      xl_GetCoord(lpSS, (LPRWCO)&RwCo, sRefStyle, lpcell, (LPTSTR)&(lptstr[wStrCount*FORMULA_PART_MAX]));

      dw += sizeof(CELLREF8);
      wStrCount++;
    }
    break;
    case ptgRef3d:
    case ptgRef3dV:
    {
      LPREF3D lpRef3D = (LPREF3D)LPOFFSET(lpFx,dw);
      RWCO    RwCo;
      short   sRefStyle = SSGetRefStyle(lpSS->lpBook->hWnd);
        
      RwCo.col = (BYTE)lpRef3D->col;
      RwCo.row = lpRef3D->rw;
      RwCo.fColRel = lpRef3D->fColRel;
      RwCo.fRwRel = lpRef3D->fRwRel;
#ifdef SS_V70
      if (1 == xl_GetCoord3D(lpSS, lpss, TRUE, (LPRWCO)&RwCo, sRefStyle, lpRef3D->ixti, lpcell, (LPTSTR)&(lptstr[wStrCount*FORMULA_PART_MAX])))
        nErr = TRUE;
#endif

      dw += sizeof(REF3D);
      wStrCount++;

#ifndef SS_V70
      nErr = TRUE;
#endif
    }
    break; 
/*
    case ptgMemErr:
    {
      xl_LogFile(lpSS->lpBook, LOG_FORMERROR, (lpcell?lpcell->wCol+1:0), (lpcell?lpcell->wRow+1:0), &(lpFx[dw]));
      *lpbType = 0;
      nErr = TRUE;

      dw += 6 + sizeof(BYTE);
    }
    break;
    case ptgAreaErr:
    {
      xl_LogFile(lpSS->lpBook, LOG_FORMERROR, (lpcell?lpcell->wCol+1:0), (lpcell?lpcell->wRow+1:0), &(lpFx[dw]));
      *lpbType = 0;
      nErr = TRUE;

      dw += 8 + sizeof(BYTE);
    }
    break;
    case ptgAreaErr3d:
    {
      LPAREA3D lpArea3D = (LPAREA3D)LPOFFSET(lpFx,dw);

      xl_LogFile(lpSS->lpBook, LOG_FORMERROR, (lpcell?lpcell->wCol+1:0), (lpcell?lpcell->wRow+1:0), &(lpFx[dw]));
      *lpbType = 0;

      dw += sizeof(AREA3D);

      nErr = TRUE;
    }
    break; 
    case ptgRefErr:
    {
      xl_LogFile(lpSS->lpBook, LOG_FORMERROR, (lpcell?lpcell->wCol+1:0), (lpcell?lpcell->wRow+1:0), &(lpFx[dw]));
      *lpbType = 0;
      nErr = TRUE;

      dw += 3 + sizeof(BYTE);
    }
    break;
    case ptgRefErr3d:
    {
      LPREF3D lpRef3D = (LPREF3D)LPOFFSET(lpFx,dw);

      xl_LogFile(lpSS->lpBook, LOG_FORMERROR, (lpcell?lpcell->wCol+1:0), (lpcell?lpcell->wRow+1:0), &(lpFx[dw]));
      *lpbType = 0;
      nErr = TRUE;

      dw += sizeof(REF3D);
    }
    break; 
*/
    case ptgAttr:
    {
      LPATTR lpAttr = (LPATTR)LPOFFSET(lpFx,dw);
        
      if (lpAttr->bitFAttrIf)
      {
        //IF(a,b,c)
        nErr = ParseIFParams(lpSS, lpss, lpfx, lptstrFx, lpcell, lpbType, lpf, lpFx, lptstr, &dw, (LPWORD)&wStrCount);
        if (nErr)
          break;
//          xl_LogFile(lpSS->lpBook, LOG_IFFUNCTIONLOAD, (lpcell?lpcell->wCol+1:0), (lpcell?lpcell->wRow+1:0), NULL);
//          fErr = TRUE;
//          break;
      }
      else if (lpAttr->bitFAttrSum)
        BuildInfixFnx(lptstr, (LPWORD)&wStrCount, _T("SUM"), 1);
      else if (lpAttr->bitFAttrSpace)
      {/* skip*/}
      else if (lpAttr->bitFAttrSemi)
      {/* skip*/}
      else
        nErr = TRUE;
      xl_LogFile(lpSS->lpBook, LOG_FUNCFORMCELLTYPE, (lpcell?lpcell->wCol+1:0), (lpcell?lpcell->wRow+1:0), NULL);

      dw += sizeof(ATTR);
    }
    break; 

    case ptgAreaN:
    case ptgAreaA:
    case ptgArea:
    case ptgAreaV:
    case ptgAreaNV:
    {
      LPAREA lpArea = (LPAREA)LPOFFSET(lpFx,dw);
      int    i;
      short  sRefStyle = SSGetRefStyle(lpSS->lpBook->hWnd);
      TCHAR  acVal[2][100];
      RWCO   RwCo[2];

      RwCo[0].fColRel = lpArea->fColRelFirst;
      RwCo[1].fColRel = lpArea->fColRelLast;
      RwCo[0].fRwRel = lpArea->fRwRelFirst;
      RwCo[1].fRwRel = lpArea->fRwRelLast;

      RwCo[0].col = (BYTE)lpArea->colFirst;
      RwCo[1].col = (BYTE)lpArea->colLast;
      RwCo[0].row = lpArea->rwFirst;
      RwCo[1].row = lpArea->rwLast;

      if (lpFx[dw] == ptgAreaN || lpFx[dw] == ptgAreaNV)
      {
        if (lpcell)
        {
          RwCo[0].col += lpcell->wCol;
          RwCo[1].col += lpcell->wCol;
          RwCo[0].row += lpcell->wRow;
          RwCo[1].row += lpcell->wRow;
        }
      }

      memset(acVal, 0, 2*100);

      if (lpcell)
      {
        for (i=0; i<2; i++)
        {
          xl_GetCoord(lpSS, (LPRWCO)&(RwCo[i]), sRefStyle, lpcell, (LPTSTR)&(acVal[i]));
        }
      }
      _stprintf(&(lptstr[wStrCount*FORMULA_PART_MAX]), _T("%s:%s"), acVal[0], acVal[1]);
      dw += sizeof(AREA);
      wStrCount++;
    }
    break; 
    case ptgNameV:
    case ptgName:
    {
      LPNAMED lpNamed = (LPNAMED)LPOFFSET(lpFx,dw);
      LPxNAME lpName = (LPxNAME)tbGlobalLock(lpss->ghNames);
      LPTSTR  lptstrName = (LPTSTR)tbGlobalLock(lpName[lpNamed->ilbl-1].ghName);
        
      lstrcpy(&(lptstr[wStrCount*FORMULA_PART_MAX]), lptstrName);

      tbGlobalUnlock(lpName[lpNamed->ilbl-1].ghName);
      tbGlobalUnlock(lpss->ghNames);
      dw += sizeof(NAMED);
      wStrCount++;
//      nErr = TRUE;
    }
    break;
#ifdef SS_V80ROBBY
    case ptgNameX:
    {
      LPNAMEDX lpNamedx = (LPNAMEDX)LPOFFSET(lpFx, dw);
      LPxEXTERNNAME lpName = (LPxEXTERNNAME)tbGlobalLock(lpss->ghExternNames);
      LPTSTR lptstrName = (LPTSTR)tbGlobalLock(lpName[lpNamedx->ilbl-1].ghName);

      lstrcpy(&(lptstr[wStrCount*FORMULA_PART_MAX]), lptstrName);

      tbGlobalUnlock(lpName[lpNamedx->ilbl-1].ghName);
      tbGlobalUnlock(lpss->ghExternNames);
      dw += sizeof(NAMEDX);
      wStrCount++;
    }
      break;
#endif
    case ptgArea3d:
    {
      short    sRefStyle = SSGetRefStyle(lpSS->lpBook->hWnd);
      TCHAR    acVal[2][100];
      LPAREA3D lpArea = (LPAREA3D)LPOFFSET(lpFx,dw);
      RWCO     RwCo[2];
      int      i;

      RwCo[0].col = (BYTE)lpArea->colFirst;
      RwCo[1].col = (BYTE)lpArea->colLast;
      RwCo[0].row = lpArea->rwFirst;
      RwCo[1].row = lpArea->rwLast;
      RwCo[0].fColRel = lpArea->fColRelFirst;
      RwCo[1].fColRel = lpArea->fColRelLast;
      RwCo[0].fRwRel = lpArea->fRwRelFirst;
      RwCo[1].fRwRel = lpArea->fRwRelLast;

      memset(acVal, 0, 2*100);
 
//      if (lpcell)
      {
        for (i=0; i<2; i++)
        {
#ifdef SS_V70
          xl_GetCoord3D(lpSS, lpss, (i==0?TRUE:FALSE), (LPRWCO)&(RwCo[i]), sRefStyle, lpArea->ixti, lpcell, (LPTSTR)&(acVal[i]));
#else
          xl_GetCoord(lpSS, (LPRWCO)&(RwCo[i]), sRefStyle, lpcell, (LPTSTR)&(acVal[i]));
#endif
        }
      }
      _stprintf(&(lptstr[wStrCount*FORMULA_PART_MAX]), _T("%s:%s"), acVal[0], acVal[1]);

      dw += sizeof(AREA3D);
      wStrCount++;
    }
    break;
    case ptgExp:
    {
      LPEXP lpExp = (LPEXP)LPOFFSET(lpFx,dw);
      dw += sizeof(EXP);
        
      lptstr = NULL;
      nErr = 2;
    }
    break;

    case ptgStr:
    {
      // It does not look like ptgStr uses BIFFSTR as would be expected.
      short sLen = 0;
        
      dw++;
      sLen = (short)*(LPBYTE)LPOFFSET(lpFx, dw);
//        if (lpbfstr->fHighByte)
//          sLen = 2*lpbfstr->wCharCount;
  
      dw+=sizeof(byte);
      if (sLen)
      {
        TCHAR buffer[1000];
        BYTE  bHighByte = *(LPBYTE)LPOFFSET(lpFx, dw);
        dw+=sizeof(byte);
        memset(buffer, 0, 1000*sizeof(TCHAR));
        memcpy(buffer, (LPTSTR)LPOFFSET(lpFx, dw), min(1000, sLen*(bHighByte?2:1)));
        dw += sLen*(bHighByte?2:1);
#ifdef _UNICODE
        if (!bHighByte)
        {
          TBGLOBALHANDLE gh = tbGlobalAlloc(GHND, (sLen+1)*sizeof(TCHAR));
          LPTSTR lpsz = (LPTSTR)tbGlobalLock(gh);
          MultiByteToWideChar(CP_ACP, 0, (LPCSTR)buffer, -1, lpsz, sLen+1);
          memset(buffer, 0, 1000*sizeof(TCHAR));
          memcpy(buffer, lpsz, sLen*sizeof(TCHAR));
          tbGlobalUnlock(gh);
          tbGlobalFree(gh);
        }
#else
        if (bHighByte)
        {
          TBGLOBALHANDLE gh = tbGlobalAlloc(GHND, (sLen+1)*2);
          LPTSTR lpsz = (LPTSTR)tbGlobalLock(gh);
          sLen = WideCharToMultiByte(CP_ACP, WC_COMPOSITECHECK, (LPCWSTR)buffer, -1, lpsz,
                                     (sLen+1)*2, NULL, NULL);
          memcpy(buffer, lpsz, sLen);
          tbGlobalUnlock(gh);
          tbGlobalFree(gh);
        }
#endif
        _stprintf(&(lptstr[wStrCount*FORMULA_PART_MAX]), _T("%c%s%c"), (TCHAR)'"', buffer, (TCHAR)'"');
      }
      else
      {
        dw+=sizeof(byte); // to get past the NULL (0).
        _stprintf(&(lptstr[wStrCount*FORMULA_PART_MAX]), _T("%c%c"), (TCHAR)'"', (TCHAR)'"');
      }  
      wStrCount++;
    }
    break;

    case ptgMemFunc:
    {
      dw += sizeof(BYTE);
      dw += sizeof(WORD);
    }
    break;

    default:
      xl_LogFile(lpSS->lpBook, LOG_FORMPTGNOTSUPPORTED, (lpcell?lpcell->wCol+1:0), (lpcell?lpcell->wRow+1:0), &lpFx[dw]);
      nErr = TRUE;
    break;
  }

  *pdw = dw;
  *pwStrCount = wStrCount;

  return(nErr);
}


BOOL FP_API xl_ParseFormula(LPSPREADSHEET lpSS, LPSSXL lpss, LPyyFORMULA lpfx, LPTSTR lptstrFx, LPyCELLVAL lpcell, LPBYTE lpbType, LPFMT lpf)
{
  BOOL   Ret = FALSE;
  DWORD  dw=0;
  TBGLOBALHANDLE gh = tbGlobalAlloc(GHND, FORMULA_PARTS_MAX * sizeof(TCHAR));
  LPTSTR lptstr = (LPTSTR)tbGlobalLock(gh);
  WORD   wStrCount = 0;
  LPBYTE lpFx = NULL;
  short  nErr = FALSE;

  if (lpfx != NULL)
    lpFx = (LPBYTE)tbGlobalLock(lpfx->ghFormula);

  memset(lptstrFx, 0, FORMULA_PART_MAX*sizeof(TCHAR));

  *lpbType = FLOATTYPE; 
  
  if (lpf)
  {
#ifdef SS_V40
    lpf->Right = 5;
#else
    lpf->floatfmt.nDigitsLeft = 9;
    lpf->floatfmt.nDigitsRight = 5;
#endif
  }

  xl_LogFile(lpSS->lpBook, LOG_FUNCFORMCELLTYPE, (lpcell?lpcell->wCol+1:0), (lpcell?lpcell->wRow+1:0), NULL);

  while (dw<lpfx->wFormulaLen && !nErr)
  {
    nErr = xl_ParseRecord(lpSS, lpss, lpfx, lptstrFx, lpcell, lpbType, lpf, lpFx, lptstr, &dw, &wStrCount);
    Ret = nErr;
    if (nErr == TRUE || nErr == 2)
      break;
  }

  if (!nErr && lptstr)
    lstrcpy(lptstrFx, lptstr);

  tbGlobalUnlock(gh);
  tbGlobalFree(gh);
  
  return Ret;
}
//---------------------//
#ifndef RC_INVOKED
#pragma pack(1)
#endif   // ifndef RC_INVOKED

//Add by BOC 99.5.24 (hyt)
#ifdef SPREAD_JPN
#define TOKENS _T("0#.%,:/-+$()\\\"Ee;cdwmqyhnstAa[]_*@?g")
#else
#define TOKENS _T("0#.%,:/-+$()\\\"Ee;cdwmqyhnstAa[]_*@?")
#endif

#define AMPM1 _T("AM/PM")
#define AMPM2 _T("am/pm")
#define AMPM3 _T("A/P")
#define AMPM4 _T("a/p")
#define AMPM5 _T("AMPM")

#define F_ZERO        0
#define F_PLACEHOLDER 1
#define F_DECIMAL     2
#define F_PERCENT     3
#define F_THOUSANDS   4
#define F_TIMESEP     5
#define F_DATESEP     6
#define F_LMINUS      7
#define F_LPLUS       8
#define F_LDOLLAR     9
#define F_LOPPAREN    10
#define F_LCLPAREN    11
#define F_LITERAL     12
#define F_STRING      13
#define F_SCIENTIFIC1 14
#define F_SCIENTIFIC2 15
#define F_NEWSECTION  16
#define F_DATETIME    17
#define F_DAY         18
#define F_DAYOFWEEK   19
//#define F_MONTH       20
#define F_MONTHMINUTE 20
#define F_QUARTER     21
#define F_YEAR        22
#define F_HOUR        23
//#define F_MINUTE      24
#define F_SECOND      25
#define F_COMPTIME    26
#define F_AMPM1       27
#define F_AMPM2       28 
#define F_OPBRACKET   29
#define F_CLBRACKET   30
#define F_IGNORE1     31  //_
#define F_IGNORE2     32  //* - position the currency symbol left justified?
#define F_IGNORE3     33  //@
#define F_FRACTION    34  //?
//Add by BOC 99.5.24 (hyt)
#ifdef SPREAD_JPN
#define F_ERAYEAR     35
#endif

//Some Common Formats
#define FMT_FRACTIONBY2 _T("#\\ ?/2")
#define FMT_FRACTIONBY4 _T("#\\ ?/4")
#define FMT_FRACTIONBY8 _T("#\\ ?/8")
#define FMT_FRACTIONBY16 _T("#\\ ?/16")
#define FMT_FRACTIONBY162 _T("#\\ ??/16")
#define FMT_FRACTIONBY10 _T("#\\ ?/10")
#define FMT_FRACTIONBY100 _T("#\\ ?/100")
#define FMT_FRACTIONBY1002 _T("#\\ ??/100")
#define FMT_FRACTION1 _T("#\\ ?/?")
#define FMT_FRACTION2 _T("#\\ ??/??")
#define FMT_FRACTION3 _T("#\\ ???/???")
#define FMT_FRACTION4 _T("#\\ ????/????")
#define FMT_FRACTION5 _T("#\\ ?????/?????")
#ifndef RC_INVOKED
#pragma pack()
#endif   // ifndef RC_INVOKED

//---Number Format Parsing---//

BOOL FP_API xl_PreParseFormat(LPSPREADSHEET lpSS, LPTSTR lptstr, LPFMT lpf, LPWORD lpwType)
{

  BOOL  Ret = FALSE;

  TCHAR buff[] ={_T("#\\ ?/2")};
  TCHAR buff2[] ={_T("#\\ ?/8")};
  TCHAR buff3[] ={_T("#\\ ?/4")};
  TCHAR buff4[] ={_T("#\\ ?/16")};
  TCHAR buff5[] ={_T("#\\ ?/10")};
  TCHAR buff6[] ={_T("#\\ ?/100")};

#ifdef SS_V40
  if (!_tcscmp(lptstr, buff))
  {
    *lpwType = NUMBERTYPE;
    lpf->Right = 1;
    xl_LogFile(lpSS->lpBook, LOG_FORMATFRACTION, 0, 0, lptstr);
    Ret = TRUE;
  }
  if (!_tcscmp(lptstr, buff2))
  {
    *lpwType = NUMBERTYPE;
    lpf->Right = 3;
    xl_LogFile(lpSS->lpBook, LOG_FORMATFRACTION, 0, 0, lptstr);
    Ret = TRUE;
  }
  if (!_tcscmp(lptstr, buff3))
  {
    *lpwType = NUMBERTYPE;
    lpf->Right = 2;
    xl_LogFile(lpSS->lpBook, LOG_FORMATFRACTION, 0, 0, lptstr);
    Ret = TRUE;
  }
  if (!_tcscmp(lptstr, buff4))
  {
    *lpwType = NUMBERTYPE;
    lpf->Right = 5;
    xl_LogFile(lpSS->lpBook, LOG_FORMATFRACTION, 0, 0, lptstr);
    Ret = TRUE;
  }
  if (!_tcscmp(lptstr, buff5))
  {
    *lpwType = NUMBERTYPE;
    lpf->Right = 2;
    xl_LogFile(lpSS->lpBook, LOG_FORMATFRACTION, 0, 0, lptstr);
    Ret = TRUE;
  }
  if (!_tcscmp(lptstr, buff6))
  {
    *lpwType = NUMBERTYPE;
    lpf->Right = 3;
    xl_LogFile(lpSS->lpBook, LOG_FORMATFRACTION, 0, 0, lptstr);
    Ret = TRUE;
  }
#else
  if (!_tcscmp(lptstr, buff))
  {
    *lpwType = FLOATTYPE;
    lpf->floatfmt.nDigitsLeft = 13;
    lpf->floatfmt.nDigitsRight = 1;
    lpf->floatfmt.fDigitLimit = TRUE;
    xl_LogFile(lpSS->lpBook, LOG_FORMATFRACTION, 0, 0, lptstr);
    Ret = TRUE;
  }
  if (!_tcscmp(lptstr, buff2))
  {
    *lpwType = FLOATTYPE;
    lpf->floatfmt.nDigitsLeft = 11;
    lpf->floatfmt.nDigitsRight = 3;
    lpf->floatfmt.fDigitLimit = TRUE;
    xl_LogFile(lpSS->lpBook, LOG_FORMATFRACTION, 0, 0, lptstr);
    Ret = TRUE;
  }
  if (!_tcscmp(lptstr, buff3))
  {
    *lpwType = FLOATTYPE;
    lpf->floatfmt.nDigitsLeft = 12;
    lpf->floatfmt.nDigitsRight = 2;
    lpf->floatfmt.fDigitLimit = TRUE;
    xl_LogFile(lpSS->lpBook, LOG_FORMATFRACTION, 0, 0, lptstr);
    Ret = TRUE;
  }
  if (!_tcscmp(lptstr, buff4))
  {
    *lpwType = FLOATTYPE;
    lpf->floatfmt.nDigitsLeft = 9;
    lpf->floatfmt.nDigitsRight = 5;
    lpf->floatfmt.fDigitLimit = TRUE;
    xl_LogFile(lpSS->lpBook, LOG_FORMATFRACTION, 0, 0, lptstr);
    Ret = TRUE;
  }
  if (!_tcscmp(lptstr, buff5))
  {
    *lpwType = FLOATTYPE;
    lpf->floatfmt.nDigitsLeft = 12;
    lpf->floatfmt.nDigitsRight = 2;
    lpf->floatfmt.fDigitLimit = TRUE;
    xl_LogFile(lpSS->lpBook, LOG_FORMATFRACTION, 0, 0, lptstr);
    Ret = TRUE;
  }
  if (!_tcscmp(lptstr, buff6))
  {
    *lpwType = FLOATTYPE;
    lpf->floatfmt.nDigitsLeft = 11;
    lpf->floatfmt.nDigitsRight = 3;
    lpf->floatfmt.fDigitLimit = TRUE;
    xl_LogFile(lpSS->lpBook, LOG_FORMATFRACTION, 0, 0, lptstr);
    Ret = TRUE;
  }
#endif
/*
  char buff[] ={"#\\ ???/???"};
  char buff2[] ={"#\\ ??/16"};
  char buff3[] ={"#\\ ?/2"};
  char buff4[] ={"#\\ ?/4"};

  if (!lstrcmp(lptstr, FMT_FRACTIONBY2) || !strcmp(lptstr, FMT_FRACTIONBY10))
  {
    *lpwType = FLOATTYPE;
    lpf->floatfmt.nDigitsLeft = 9;
    lpf->floatfmt.nDigitsRight = 1;
    xl_LogFile(lpSS->lpBook, LOG_FORMATFRACTION, 0, 0, lptstr);
  }
  else if (!lstrcmp(lptstr, FMT_FRACTIONBY4))
  {
    *lpwType = FLOATTYPE;
    lpf->floatfmt.nDigitsLeft = 9;
    lpf->floatfmt.nDigitsRight = 2;
    xl_LogFile(lpSS->lpBook, LOG_FORMATFRACTION, 0, 0, lptstr);
  }
  else if (!lstrcmp(lptstr, FMT_FRACTIONBY8) || !strcmp(lptstr, FMT_FRACTIONBY100) || !lstrcmp(lptstr, FMT_FRACTIONBY1002) ||
           !lstrcmp(lptstr, FMT_FRACTION1))
  {
    *lpwType = FLOATTYPE;
    lpf->floatfmt.nDigitsLeft = 9;
    lpf->floatfmt.nDigitsRight = 3;
    xl_LogFile(lpSS->lpBook, LOG_FORMATFRACTION, 0, 0, lptstr);
  }
  else if (!lstrcmp(lptstr, FMT_FRACTIONBY16) || !strcmp(lptstr, FMT_FRACTIONBY162) || !lstrcmp(lptstr, FMT_FRACTION2))
  {
    *lpwType = FLOATTYPE;
    lpf->floatfmt.nDigitsLeft = 9;
    lpf->floatfmt.nDigitsRight = 4;
    xl_LogFile(lpSS->lpBook, LOG_FORMATFRACTION, 0, 0, lptstr);
  }
  else if (!lstrcmp(lptstr, "#\\ ???/???"))// || !strcmp(lptstr, FMT_FRACTION4) || !strcmp(lptstr, FMT_FRACTION5))
  {
    *lpwType = FLOATTYPE;
    lpf->floatfmt.nDigitsLeft = 9;
    lpf->floatfmt.nDigitsRight = 6;
    xl_LogFile(lpSS->lpBook, LOG_FORMATFRACTION, 0, 0, lptstr);
  }
  else
  {
    *lpwType = FLOATTYPE;
    lpf->floatfmt.nDigitsLeft = 9;
    lpf->floatfmt.nDigitsRight = 9;
    xl_LogFile(lpSS->lpBook, LOG_FORMATUNKNOWN, 0, 0, lptstr);
  }
*/
  return Ret;
}

BOOL xl_ParseContextFormat(LPTSTR lptstr, int iLen, LPLONG lplVal, LPBYTE lpbAction, LPINT lpiLen)
{
  int  i;
  
  *lpbAction = 0;

  if (!_tcsncmp(&lptstr[1], _T("Red"), lstrlen(_T("Red"))))
  {
    *lpbAction = 8;
    *lpiLen += lstrlen(_T("Red")) + 2; // +2 for both brackets.

    return 0;
  }

  for (i=1; i<iLen; i++)
  {
    while (isdigit(lptstr[i]))
    {
      *lplVal = (*lplVal*10) + lptstr[i]-(TCHAR)'0';
      i++;
    }

    switch (lptstr[i])
    {
      case (TCHAR)'=':
        *lpbAction |= 1;
      break;  
      case (TCHAR)'<':
        *lpbAction |= 2;
      break;  
      case (TCHAR)'>':
        *lpbAction |= 4;
      break;  
      case (TCHAR)'h':
        *lpbAction = 16;
      break;
//      default:
//        *lpbAction = 0;
    }

    if (lptstr[i] == (TCHAR)']')
    {
      i++;
      break;
    }
  }
  *lpiLen += i;
  return 0;
}

BOOL FP_API xl_ParseFormat(LPSPREADSHEET lpSS, LPTSTR lptstr, LPFMT lpf, LPWORD lpwType, LPVOID lpData)
{
  int   i;
  int   j;
  int   iLen;
  LPTSTR lpTokens = TOKENS;
  BOOL  Ret = FALSE;

  BOOL  fDecimal   = FALSE;
  BOOL  fSeparator = FALSE;
  BOOL  fScale     = FALSE;
  BOOL  fTimeSep   = FALSE;
  BOOL  fDateSep   = FALSE;
  BOOL  fCurrency  = FALSE;
  BOOL  fCurrencySymbolFirst = TRUE;
  BOOL  f12Hour    = FALSE;
  BOOL  fFraction  = FALSE;
  BOOL  fPercent   = FALSE;
  BOOL  fScientific = FALSE;
  BOOL  fPositive  = FALSE;
  BOOL  fOne       = FALSE;
  BOOL  fNotSupported = FALSE;
  int   iSciDecimals = 0;
  int   iDigitsLeftForce = 0;
  int   iDigitsRightForce = 0;
  int   iDigitsLeft = 0;
  int   iDigitsRight = 0;
  int   iDay = 0;
  int   iMonth = 0;
  int   iYear = 0;
  int   iOrder = 0;
  int   iHour = 0;
  int   iMinute = 0;
  int   iSecond = 0;
  int   iT = 0;
  int   iMiscLen = 0;
  TCHAR acLiteral[10][100];
  int   iCurrencyPos = 0;
  BOOL  fMultSections = FALSE;
#ifdef SS_V40
  BOOL  fNegParen = FALSE;
  BYTE  abNumberFormat[4] = {0,0,0,0};
  BYTE  abNegNumberFormat[4] = {0,0,0,0};
  int   iNumberFormatPos = 0;
  int   iNegNumberFormatPos = 0;
  #define number   1
  #define currency 2
  #define negative 3
  #define space    4
  #define percent  5
  #define SetNumberFormatElement(x) ((abNumberFormat[0]!=x && abNumberFormat[1]!=x && abNumberFormat[2]!=x && abNumberFormat[3]!=x)? abNumberFormat[iNumberFormatPos++]=x : 0)
  #define SetNegNumberFormatElement(x) ((abNegNumberFormat[0]!=x && abNegNumberFormat[1]!=x && abNegNumberFormat[2]!=x && abNegNumberFormat[3]!=x)? abNegNumberFormat[iNegNumberFormatPos++]=x : 0)
#else
  #define SetNumberFormatElement(x) 0
  #define SetNegNumberFormatElement(x) 0
#endif

#ifdef SPREAD_JPN
  BOOL  fDBCSSeparate = FALSE;
  int   iEraYear = 0 ;
  BOOL  fDBCSSign = FALSE;
#endif
  LPTSTR lptstrTemp = NULL;
  
  memset(acLiteral, 0, 10*100*sizeof(TCHAR));

  if (lptstr)
    iLen = lstrlen(lptstr);
  else
    return TRUE;
  
  memset(lpf, 0, sizeof(FMT));
#ifdef SS_V40
  lpf->bLeadingZero = 1;
#endif // SS_V40

#ifdef SPREAD_JPN
  if(_tcsstr(lptstr,_T("Å¢")) || _tcsstr(lptstr,_T("Å£")))
	  fDBCSSign = TRUE;
  else if(!_tcscmp(lptstr,_T("0;[Red]0")))
	  fDBCSSign = TRUE;
  else if(_tcsstr(lptstr,_T("\"\\\""))==lptstr)
	  fCurrency = TRUE;
  else if ((memchr(lptstr, 0xA5, lstrlen(lptstr))) != NULL)
  {
    _tcscpy(acLiteral[0] ,_T("\\"));
    fCurrency = TRUE;
    for (i=0; i<lstrlen(lptstr); i++)
    {
      if (lptstr[i] == (TCHAR)0xA5)
        lptstr[i] = '\\';
    }
  }
  else if(_tcsstr(lptstr,_T("$"))==lptstr || _tcsstr(lptstr, _T("\\$"))==lptstr)
  {
	  fCurrency = TRUE;
	  acLiteral[0][0] = '$';
  }
  else if(_tcsstr(lptstr,_T("\"US$\""))==lptstr)
  {
	  fCurrency = TRUE;
	  acLiteral[0][0] = '$';
  }
  else 
  {
    LPVOID lp = NULL;
#ifndef SPREAD_JPN
    if ((lp = memchr(lptstr, 0x3F, lstrlen(lptstr))) != NULL)
    {
      // euro
      fCurrency = TRUE;
      acLiteral[0][0] = 0x3F;
    }
    else if ((lp = memchr(lptstr, 0xA5, lstrlen(lptstr))) != NULL)
    {
      _tcscpy(acLiteral[0] ,_T("\\"));
	    fCurrency = TRUE;
      for (i=0; i<lstrlen(lptstr; i++)
      {
        if (lptstr[i] == (TCHAR)0xA5)
          lptstr[i] = '\\';
      }
    }
#endif
  }
#endif

  for (i=0; i<iLen; i++)
  {
    int    iToken;
  
    LPTSTR lpsz = _tcschr(lpTokens, lptstr[i]);
    if (lpsz)
      iToken = (int)((LPARAM)lpsz - (LPARAM)lpTokens)/sizeof(TCHAR);
    else
//RAP01a >>
    {
//      i++;
      continue;
    }
// RAP01a <<
//RAP01d      break;

    switch(iToken)
    {
      case (TCHAR)'2':
        if (fFraction && fDateSep)
        {
          // ?/2 format
          fDateSep = FALSE;
#ifdef SS_V40
          *lpwType = NUMBERTYPE;
          lpf->Right = 1;
#else
          *lpwType = FLOATTYPE;
          lpf->floatfmt.nDigitsLeft = 13;
          lpf->floatfmt.nDigitsRight = 1;
          lpf->floatfmt.fDigitLimit = TRUE;
#endif  
          xl_LogFile(lpSS->lpBook, LOG_FORMATFRACTION, 0, 0, lptstr);
        }
      break;
      case (TCHAR)'4':
        if (fFraction && fDateSep)
        {
          // ?/4 format
          fDateSep = FALSE;
#ifdef SS_V40
          *lpwType = NUMBERTYPE;
          lpf->Right = 2;
#else
          *lpwType = FLOATTYPE;
          lpf->floatfmt.nDigitsLeft = 12;
          lpf->floatfmt.nDigitsRight = 2;
          lpf->floatfmt.fDigitLimit = TRUE;
#endif          
          xl_LogFile(lpSS->lpBook, LOG_FORMATFRACTION, 0, 0, lptstr);
        }
      break;
      case (TCHAR)'8':
        if (fFraction && fDateSep)
        {
          // ?/8 format
          fDateSep = FALSE;
#ifdef SS_V40
          *lpwType = NUMBERTYPE;
          lpf->Right = 3;
#else
          *lpwType = FLOATTYPE;
          lpf->floatfmt.nDigitsLeft = 11;
          lpf->floatfmt.nDigitsRight = 3;
          lpf->floatfmt.fDigitLimit = TRUE;
#endif
          xl_LogFile(lpSS->lpBook, LOG_FORMATFRACTION, 0, 0, lptstr);
        }
      break;
      case (TCHAR)'1':
      {
        if (fFraction && fDateSep)
        {
          fDateSep = FALSE;
          if (lptstr[i+1] == (TCHAR)'0')
          {
            // ?/10 format
            i++;
#ifdef SS_V40
            *lpwType = NUMBERTYPE;
            if (lptstr[i+1] == (TCHAR)'0') 
            {
              i++;
              lpf->Right = 3;
            }
            else
              lpf->Right = 2;
#else
            *lpwType = FLOATTYPE;
            if (lptstr[i+1] == (TCHAR)'0') 
            {
              i++;
              lpf->floatfmt.nDigitsLeft = 11;
              lpf->floatfmt.nDigitsRight = 3;
            }
            else
            {
              lpf->floatfmt.nDigitsLeft = 12;
              lpf->floatfmt.nDigitsRight = 2;
            }
#endif
          }
          else if (lptstr[i+1] == (TCHAR)'6') 
          {
            // ?/16 format
            i++;
#ifdef SS_V40
            *lpwType = FLOATTYPE;
            lpf->Right = 5;
#else
            *lpwType = FLOATTYPE;
            lpf->floatfmt.nDigitsLeft = 9;
            lpf->floatfmt.nDigitsRight = 5;
#endif
          }
#ifndef SS_V40
          lpf->floatfmt.fDigitLimit = TRUE;
#endif
          xl_LogFile(lpSS->lpBook, LOG_FORMATFRACTION, 0, 0, lptstr);
        }
      }
      break;
      case (TCHAR)'0':
      break;

      case F_ZERO:
        //Display leading and/or trailing zeros
        if (fMultSections) //negative style format
        {
          SetNegNumberFormatElement(number);
          break;
        }

        if (!fDecimal)
          iDigitsLeftForce++;
        else
          iDigitsRightForce++;

        SetNumberFormatElement(number);
      break;
      case F_PLACEHOLDER:
        //DO NOT display leading and/or trailing zeros
        if (fMultSections) //negative style format
        {
          SetNegNumberFormatElement(number);
          break;
        }

        if (!fDecimal)
          iDigitsLeft++;
        else
          iDigitsRight++;

        SetNumberFormatElement(number);
      break;
      case F_DECIMAL:
        if (fMultSections) //negative style format
        {
          SetNegNumberFormatElement(number);
          break;
        }
  
        fDecimal = TRUE;

        SetNumberFormatElement(number);
      break;
      case F_PERCENT:
#ifdef SS_V40
        if (fMultSections) //negative style format
        {
          SetNegNumberFormatElement(percent);
          break;
        }
        SetNumberFormatElement(percent);
#else
        iDigitsRight += 2;
        xl_LogFile(lpSS->lpBook, LOG_FORMATPERCENT, 0, 0, lptstr);
#endif
        fPercent = TRUE;

      break;
      case F_THOUSANDS:
        fSeparator = TRUE;
        if (lptstr[i+1] == (TCHAR)',')
        {
          if (i == iLen)
          {
            i++;
            fScale = TRUE;
          }
          else if(lptstr[i+2] == (TCHAR)'.')
          {
            i += 2;
            fScale = TRUE;
          }
        }
      break;
      case F_TIMESEP:
        fTimeSep = TRUE;
      break;
      case F_DATESEP:
        fDateSep = TRUE;
      break;
      case F_LMINUS:
        if (iDay>0 || iMonth>0 || iYear>0)
        {
          fDateSep = TRUE;
          for (j=0; j<10; j++)
          {
            if (acLiteral[j][0] == 0)
              break;
          }
          acLiteral[j][0] = '-';
        }
#ifdef SS_V40
        else if (fMultSections) //negative style format
        {
          SetNegNumberFormatElement(negative);
        }
#else
        fNotSupported = TRUE;
        xl_LogFile(lpSS->lpBook, LOG_FORMATNOTSUPPORTED, 0, 0, lptstr);
#endif
      break;
      case F_LPLUS:
        fNotSupported = TRUE;
        xl_LogFile(lpSS->lpBook, LOG_FORMATNOTSUPPORTED, 0, 0, lptstr);
      break;
      case F_LDOLLAR:
        if (fMultSections) //negative style format
        {
          SetNegNumberFormatElement(currency);
          break;
        }

        fCurrency = TRUE;
        SetNumberFormatElement(currency);
      break;
      case F_LOPPAREN:
#ifdef SS_V40
        if (fMultSections) // this is the negative format
          fNegParen = TRUE;
#else
        fNotSupported = TRUE;
        xl_LogFile(lpSS->lpBook, LOG_FORMATNOTSUPPORTED, 0, 0, lptstr);
#endif
      break;
      case F_LCLPAREN:
#ifdef SS_V40
        // ignore
#else
        fNotSupported = TRUE;
        xl_LogFile(lpSS->lpBook, LOG_FORMATNOTSUPPORTED, 0, 0, lptstr);
#endif
      break;
      case F_OPBRACKET:
      {
        long   lVal = 0;
        BYTE   bAction = 0;
        double dbl = 0.0;
        BOOL   fOK = FALSE;
        int    nLiteralInitialSize = 0;

        if (lpData)
          dbl = *(double *)lpData;

        for (nLiteralInitialSize=0; nLiteralInitialSize<10; nLiteralInitialSize++)
        {
          if (acLiteral[nLiteralInitialSize][0] == 0)
            break;
        }

        if (lptstr[i+1] == (TCHAR)'$')
        {
          int k = 0;

          if (fMultSections) //negative style format
          {
            SetNegNumberFormatElement(currency);
            while (lptstr[i] != (TCHAR)'-' && lptstr[i] != (TCHAR)']')
              i++;
            //Go to the close bracket and skip all the unused stuff.
            while (lptstr[i] != (TCHAR)']')
              i++;
            i++;
            break;
          }

          SetNumberFormatElement(currency);

          fCurrency = TRUE;
          if (i != 0)
            fCurrencySymbolFirst = FALSE;

          iCurrencyPos = nLiteralInitialSize;
          i+=2;
          while (lptstr[i] != (TCHAR)'-' && lptstr[i] != (TCHAR)']')
          {
            acLiteral[nLiteralInitialSize][k++] = lptstr[i++];
          } 
          //Go to the close bracket and skip all the unused stuff.
          while (lptstr[i] != (TCHAR)']')
            i++;
          break;
        }

        xl_ParseContextFormat(&(lptstr[i]), iLen-i, &lVal, &bAction, &iMiscLen);
        
#ifndef SS_V40
        i += iMiscLen;
#endif
        switch (bAction)
        {
          case 1:
            //equal
            if ((long)dbl == lVal)
              fOK = TRUE;
          break;
          case 2:
            //less than
            if ((long)dbl < lVal)
              fOK = TRUE;
          break;
          case 3:
            //less than or equal
            if ((long)dbl <= lVal)
              fOK = TRUE;
          break;
          case 4:
            //greater than
            if ((long)dbl > lVal)
              fOK = TRUE;
          break;
          case 5:
            //greater than or equal
            if ((long)dbl >= lVal)
              fOK = TRUE;
          break;
#ifdef SS_V40
          case 8:
            // negative values are red
            lpf->fNegRed = TRUE;
            fOK = TRUE;
          break;
#endif
          case 16:
            fOK = TRUE;
          break;
        }
        if (!fOK)
        {
          //skip to the next format.
          while (lptstr[i] != (TCHAR)';' && i<iLen)
            i++;
          if (i == iLen)
          {
            fNotSupported = TRUE;
            break;
          }  
//          i += (iMiscLen-1);
//          continue;
        }

#ifdef SS_V40
        else
        {
          lstrcpy(&(lptstr[i]), &(lptstr[i+iMiscLen])); 
          iLen -= iMiscLen;
          i--;
        }
#else
        lstrcpy(lptstr, &(lptstr[i]));
        iLen = lstrlen(lptstr);        
        i=-1;
#endif          

#ifndef SS_V40
        fScale     = FALSE;
        fTimeSep   = FALSE;
        fDateSep   = FALSE;
        f12Hour    = FALSE;
        fFraction  = FALSE;
        fOne       = FALSE;
        fNotSupported = FALSE;
        iDigitsLeftForce = 0;
        iDigitsRightForce = 0;
        iDigitsLeft = 0;
        iDigitsRight = 0;
        iDay = 0;
        iMonth = 0;
        iYear = 0;
        iOrder = 0;
        iHour = 0;
        iMinute = 0;
        iSecond = 0;
        iT = 0;
#endif
#ifdef SPREAD_JPN
		iEraYear = 0;
		fDBCSSeparate = FALSE;
		fDBCSSign = FALSE;
#endif
    
        memset(&(acLiteral[nLiteralInitialSize][0]), 0, (10-nLiteralInitialSize)*100);
      }
      break;
      case F_CLBRACKET:
      break;
      case F_LITERAL:
        for (j=0; j<10; j++)
        {
          if (acLiteral[j][0] == 0)
            break;
        }
        i++;
#ifdef SS_V40
        if (fMultSections && lptstr[i] == (TCHAR)'(')
          fNegParen = TRUE;
        else
#endif
        {
          if (lptstr[i] == '-')
          {
            if (fMultSections) //negative style format
              SetNegNumberFormatElement(negative);
            else
              acLiteral[j][0] = lptstr[i];
          }
          else
          {
            if (lptstr[i] == ' ')
              continue;
            else
              acLiteral[j][0] = lptstr[i];
          }
#ifdef UNICODE
          if (lptstr[i] == (TCHAR)'$' || lptstr[i] == (TCHAR)'\\' || lptstr[i] == (TCHAR)0xFF65)
#else
          if (lptstr[i] == (TCHAR)'$' || lptstr[i] == (TCHAR)'\\' || lptstr[i] == (TCHAR)0xA5)
#endif
          {
            if (fMultSections) //negative style format
              SetNegNumberFormatElement(currency);
            else
              SetNumberFormatElement(currency);

            iCurrencyPos = j;            
            fCurrency = TRUE;
          }

        }
      break;
      case F_STRING:
      { 
        short iStart = i;

//        for (j=0; j<10; j++)
//        {
//          if (acLiteral[j][0] == 0)
//            break;
//        }
        int nLiteralInitialSize = 0;
        int nLiteralIndex;

        for (nLiteralInitialSize=0; nLiteralInitialSize<10; nLiteralInitialSize++)
        {
          if (acLiteral[nLiteralInitialSize][0] == 0)
            break;
        }

    		i++; //should skip first char

        for (nLiteralIndex = nLiteralInitialSize; nLiteralIndex<10; nLiteralIndex++)
        {
          if (acLiteral[nLiteralIndex][0] == 0)
            break;
        }

        for (;i<iLen; i++) 
        { 
          if (lptstr[i] == (TCHAR)'\"')
            break;
#ifdef UNICODE
          else if (lptstr[i] == (TCHAR)'$' || lptstr[i] == (TCHAR)'\\' || lptstr[i] == (TCHAR)0xFF65)
#else
          else if (lptstr[i] == (TCHAR)'$' || lptstr[i] == (TCHAR)'\\' || lptstr[i] == (TCHAR)0xA5)
#endif
          {  
            if (fMultSections) //negative style format
              SetNegNumberFormatElement(currency);
            else
              SetNumberFormatElement(currency);

            iCurrencyPos = nLiteralInitialSize;            
            fCurrency = TRUE;
          }
#ifdef SPREAD_JPN
		      if(IsDBCSLeadByte((BYTE)lptstr[i]) || lptstr[i] > 127)
		      {
#ifndef UNICODE			      
            i++;
#endif
            fDBCSSeparate = TRUE;
		      }
    		  else
#endif
            acLiteral[nLiteralIndex][i-1-iStart] = lptstr[i];
        }
      }
      break;
      case F_FRACTION:
        fFraction = TRUE;
        fDateSep = FALSE;
#ifdef SS_V40
        *lpwType = NUMBERTYPE;
        lpf->Right = 9;

#else
        *lpwType = FLOATTYPE;
        lpf->floatfmt.nDigitsLeft = 5;
        lpf->floatfmt.nDigitsRight = 9;
        lpf->floatfmt.fDigitLimit = TRUE;
#endif
      break;
      case F_SCIENTIFIC1:
      case F_SCIENTIFIC2:
        if (lptstr[i+1] == (TCHAR)'+' || lptstr[i+1] == (TCHAR)'-')
        {
          //this is scientific notation
          if (lptstr[i+1] == (TCHAR)'+')
            fPositive = TRUE;
          i++;
          
          if (fMultSections)
            break;

          *lpwType = FLOATTYPE;
          iSciDecimals = min(iDigitsRightForce, 13);
          iDigitsLeftForce = iDigitsRightForce = 0;
          fScientific = TRUE;
        }
        else
        {
          //this is not scientific notation.
#ifdef SPREAD_JPN
		if (!iYear)
          iOrder = iOrder * 10 + 3;
        iYear++;
#endif
        }
      break;
      case F_NEWSECTION:
#ifdef SS_V40
        if (!fMultSections)
        {
          lptstr[i] = 0;
          fMultSections = TRUE;
        }
        else
        {
          i = iLen;
          break;
        }
#else
        //NOTE:this will need to change for Spread40!!!
        iLen = i;
        xl_LogFile(lpSS->lpBook, LOG_MULTIPLEFORMATS, 0, 0, lptstr);
        lptstr[i] = 0;
#endif
      break;
      case F_DATETIME:
        fNotSupported = TRUE;
        xl_LogFile(lpSS->lpBook, LOG_FORMATNOTSUPPORTED, 0, 0, lptstr);
      break;
      case F_DAY:
        if (!iDay)
          iOrder = iOrder * 10 + 1;
        iDay++;
      break;
      case F_DAYOFWEEK:
        fNotSupported = TRUE;
        xl_LogFile(lpSS->lpBook, LOG_FORMATNOTSUPPORTED, 0, 0, lptstr);
      break;
//      case F_MONTH:
//        if (!iMonth)
//          iOrder = iOrder * 10 + 2;
//        iMonth++;
//      break;
      case F_MONTHMINUTE:
        if (iHour || fTimeSep)        
        {
          iMinute++;
        }
        else
        {
          if (!iMonth)
            iOrder = iOrder * 10 + 2;
          iMonth++;
        }
      break;
      case F_QUARTER:
        fNotSupported = TRUE;
        xl_LogFile(lpSS->lpBook, LOG_FORMATNOTSUPPORTED, 0, 0, lptstr);
      break;
      case F_YEAR:
        if (!iYear)
          iOrder = iOrder * 10 + 3;
        iYear++;
      break;
#ifdef SPREAD_JPN
	    case F_ERAYEAR:
		    iEraYear++;
	    break;
#endif
      case F_HOUR:
        iHour++;
      break;
//      case F_MINUTE:
//        iMinute++;
//      break;
      case F_SECOND:
        iSecond++;
      break;
      case F_COMPTIME:
        iT++;
      break;
      case F_AMPM1:
        f12Hour = TRUE;
        xl_LogFile(lpSS->lpBook, LOG_FORMATNOTSUPPORTED, 0, 0, lptstr);
      break;
      case F_AMPM2:
        f12Hour = TRUE;
        xl_LogFile(lpSS->lpBook, LOG_FORMATNOTSUPPORTED, 0, 0, lptstr);
      break;
      case F_IGNORE1:
	    	if(_tcsstr(&lptstr[i],_T("_-\\$* "))==&lptstr[i])
    		{
		      fCurrency = TRUE;
    		  acLiteral[0][0] = '$';
		      i+=6;
    		}
		    else if(_tcsstr(&lptstr[i],_T("_-\"US$\"* "))==&lptstr[i])
    		{
  	  		fCurrency = TRUE;
		      acLiteral[0][0] = '$';
    			i+=9;
		    }
        else
        {
          if (fMultSections) //negative style format
          {
            SetNegNumberFormatElement(space);
            break;
          }

          SetNumberFormatElement(space);
	        
          i++;
        }
      case F_IGNORE3:
        xl_LogFile(lpSS->lpBook, LOG_IGNOREDTOKEN, 0, 0, (LPVOID)&(lptstr[i]));
      break;
      
      default:
        xl_LogFile(lpSS->lpBook, LOG_INVALIDTOKEN, 0, 0, (LPVOID)&(lptstr[i]));
      break;
    }//switch
  }//for

 
  for (i=0; i<1; i++)
  {
    if (fNotSupported == FALSE)
    {
      int iLiterals = 0;
      int i;
    
      for (i=0; i<10; i++)
      {
        if (acLiteral[i][0] == 0)
          break;
        iLiterals++;
      }

  #ifdef SPREAD_JPN
    if (iYear || iMonth || iDay || iEraYear)
  #else
	  if (iYear || iMonth || iDay)
  #endif
      {
        //Date  
        *lpwType = DATETYPE;
        if (iMonth == 1 && iDay == 1 && iYear > 1)
        {
          switch (iOrder)
          {
            case 123: //dmy
              lpf->datefmt.nFormat = IDF_DDMMYY;
            break;
            case 213: //mdy
              lpf->datefmt.nFormat = IDF_MMDDYY;
            break;
            case 321: //ymd
              lpf->datefmt.nFormat = IDF_YYMMDD;
            break;
            default:
              xl_LogFile(lpSS->lpBook, LOG_DATEFORMATNOTSUPPORTED, 0, 0, lptstr);
              fNotSupported = TRUE;
              lpf->datefmt.nFormat = IDF_MMDDYY;
            break;
          }
        }
        else if ((iMonth == 1 || iMonth == 2) &&
                 (iDay == 1 || iDay == 2) && iYear > 1)
        {
          switch (iOrder)
          {
            case 123: //dmy
              lpf->datefmt.nFormat = IDF_DDMMYY;
            break;
            case 213: //mdy
              lpf->datefmt.nFormat = IDF_MMDDYY;
            break;
            case 321: //ymd
              lpf->datefmt.nFormat = IDF_YYMMDD;
            break;
            default:
              xl_LogFile(lpSS->lpBook, LOG_DATEFORMATNOTSUPPORTED, 0, 0, lptstr);
              fNotSupported = TRUE;
              lpf->datefmt.nFormat = IDF_MMDDYY;
            break;
          }
          if (fNotSupported)
            xl_LogFile(lpSS->lpBook, LOG_DATEFORMATNOTSUPPORTED, 0, 0, lptstr);
        }
        else if (iMonth == 3 && iDay == 1 && iYear > 1 && iOrder == 123)
        {
          //MON format
          lpf->datefmt.nFormat = IDF_DDMONYY;
        }
        else if (iMonth == 3 && (iDay == 1 || iDay == 2) && iYear > 1 && iOrder == 123)
        {
          //MON format
          lpf->datefmt.nFormat = IDF_DDMONYY;
          xl_LogFile(lpSS->lpBook, LOG_FORMAT0IGNORED, 0, 0, lptstr);
        }
  //#ifdef SPREAD_JPN
	      else if((iMonth && iDay) && iYear ==0)
	      {
  		    lpf->datefmt.nFormat = IDF_MMDD;
          xl_LogFile(lpSS->lpBook, LOG_FORMAT0IGNORED, 0, 0, lptstr);
	      }
	      else if(iYear && iMonth && iDay ==0)
	      {
	  	    lpf->datefmt.nFormat = IDF_YYMM;
          xl_LogFile(lpSS->lpBook, LOG_FORMAT0IGNORED, 0, 0, lptstr);
	      }
  //#endif
        else
        {
          xl_LogFile(lpSS->lpBook, LOG_DATEFORMATNOTSUPPORTED, 0, 0, lptstr);
          fNotSupported = TRUE;
          lpf->datefmt.nFormat = IDF_MMDDYY;
        }

  #ifdef SPREAD_JPN
	    switch(iEraYear)
	    {
		  case 1:
			  lpf->datefmt.nFormat = IDF_NYYMMDD;
		  break;
		  case 2:
			  lpf->datefmt.nFormat = IDF_NNYYMMDD;
		  break;
		  default:
			  if(iEraYear)
				  lpf->datefmt.nFormat = IDF_NNNNYYMMDD;
		  break;
	    }
  #endif

        if (iYear == 2)
          lpf->datefmt.bCentury = FALSE;
        else if (iYear == 4)
          lpf->datefmt.bCentury = TRUE;
  //Modify by BOC 99.5.24 (hyt)----------------------
  //for support japanese separator
  #ifdef SPREAD_JPN
	    if(fDBCSSeparate)
		  lpf->datefmt.cSeparator = (TCHAR)'?';
	    else
  #endif
  //-------------------------------------------------
        if (acLiteral[0][0])
          lpf->datefmt.cSeparator = acLiteral[0][0];
        else if (fDateSep)
          lpf->datefmt.cSeparator = (TCHAR)'/';
      } //Date
      else if (iHour || iMinute || iSecond || iT)
      {
        //Time
        *lpwType = TIMETYPE;
        if ((iHour == 1 || iHour == 2) && 
            (iMinute == 1 || iMinute == 2))
        {
          if (iHour == 1 || iMinute == 1 || iSecond == 1)
            xl_LogFile(lpSS->lpBook, LOG_FORMAT0IGNORED, 0, 0, lptstr);
          if (iSecond)
            lpf->timefmt.bSeconds = TRUE;

		  //Modify by BOC 99.5.24 (hyt)----------------------
		  //for support japanese separator
  #ifdef SPREAD_JPN
		  if(fDBCSSeparate)
			  lpf->timefmt.cSeparator = (TCHAR)'?';
		  else
  #endif
		  //-------------------------------------------------
          if (acLiteral[0][0])
            lpf->timefmt.cSeparator = acLiteral[0][0];
          else if (fTimeSep && lpf->timefmt.cSeparator == 0)
            lpf->timefmt.cSeparator = (TCHAR)':';

          if (!f12Hour)
            lpf->timefmt.b24Hour = TRUE;
        }
        else if (iT ==5)
        {
        }
        else
        {
          xl_LogFile(lpSS->lpBook, LOG_DATEFORMATNOTSUPPORTED, 0, 0, lptstr);
          fNotSupported = TRUE;
          *lpwType = NUMBERTYPE;
        }
      }//Time

#ifdef SS_V40
      else if (fCurrency)
      {
        // currency celltype
        *lpwType = CURRENCYTYPE;

        if (iDigitsRightForce && !iDigitsRight)
          lpf->Right = iDigitsRightForce;
        else if (iDigitsRight)
          lpf->Right = iDigitsRightForce + iDigitsRight;
        if (iDigitsLeftForce)
          lpf->bLeadingZero = 2;

        lpf->Max = 999999999999.99;
        lpf->Min = -999999999999.99;

        lpf->fShowSeparator = fSeparator;

        if (acLiteral[iCurrencyPos][0] != 0)
        {
//15572 >>
#ifdef UNICODE
          if (acLiteral[iCurrencyPos][0] == 0xFF65)
            _tcsncpy(lpf->currencyfmt.szCurrency, _T("\\"), 5);
          else
#else
          if (acLiteral[iCurrencyPos][0] == 0xA5)
            _tcsncpy(lpf->currencyfmt.szCurrency, _T("\\"), 5);
          else
#endif //<<15572
            _tcsncpy(lpf->currencyfmt.szCurrency, (LPTSTR)acLiteral[iCurrencyPos], 5);
        }

        // Positive format style
        if (abNumberFormat[0] == space)
        {
          abNumberFormat[0] = abNumberFormat[1];
          abNumberFormat[1] = abNumberFormat[2];
          abNumberFormat[2] = abNumberFormat[3];
        }
        if (abNumberFormat[0] == currency)
        {
          if (abNumberFormat[1] == number) lpf->currencyfmt.bPosStyle = 1;
          else if (abNumberFormat[1] == space && abNumberFormat[2] == number) lpf->currencyfmt.bPosStyle = 3;
        }
        else if (abNumberFormat[0] == number)
        {
          if (abNumberFormat[1] == currency) lpf->currencyfmt.bPosStyle = 2;
          else if (abNumberFormat[1] == space && abNumberFormat[2] == currency) lpf->currencyfmt.bPosStyle = 4;
        }
        
        // Negative format style
        if (abNegNumberFormat[0] == space)
        {
          abNegNumberFormat[0] = abNegNumberFormat[1];
          abNegNumberFormat[1] = abNegNumberFormat[2];
          abNegNumberFormat[2] = abNegNumberFormat[3];
        }
        switch (abNegNumberFormat[0])
        {
          case number:
            switch (abNegNumberFormat[1])
            {
              case currency:
                switch (abNegNumberFormat[2])
                {
                  case negative: lpf->bNegStyle = 8; break;
                  default: lpf->bNegStyle = (fNegParen? 5 : 6); break;
                }
              break;
              case negative:
                switch (abNegNumberFormat[2])
                {
                  case currency: lpf->bNegStyle = 7; break;
                  case space: if (abNegNumberFormat[3] == currency) lpf->bNegStyle = 14; break;
                }
              break;
              case space:
                switch (abNegNumberFormat[2])
                {
                  case currency:
                    switch (abNegNumberFormat[3])
                    {
                      case negative: lpf->bNegStyle = 11; break;
                      default: lpf->bNegStyle = (fNegParen? 16 : 9); break;
                    }
                  break;
                }       
              break;
            }
          break;
          case currency:
            switch (abNegNumberFormat[1])
            {
              case number:
                switch (abNegNumberFormat[2])
                {
                  case negative: lpf->bNegStyle = 4; break;
                  default: lpf->bNegStyle = (fNegParen? 1 : 2); break;
                }
              break;
              case negative: if (abNegNumberFormat[2] == number) lpf->bNegStyle = 3; break;
              case space:
                switch (abNegNumberFormat[2])
                {
                  case number: 
                    switch (abNegNumberFormat[3])
                    { 
                      case negative: lpf->bNegStyle = 12; break;
                      default: lpf->bNegStyle = (fNegParen? 15 : 10); break;
                    }
                  break;    
                  case negative: if (abNegNumberFormat[3] == number) lpf->bNegStyle = 13; break;
                }
              break;
            }
          break;
          case negative:
            switch (abNegNumberFormat[1])
            {
              case number: 
                switch (abNegNumberFormat[2])
                {
                  case currency: lpf->bNegStyle = 6; break;
                  case space: if (abNegNumberFormat[3] == currency) lpf->bNegStyle = 9; break;
                }
              break;
              case currency:
                switch (abNegNumberFormat[2])
                {
                  case number: lpf->bNegStyle = 2; break;
                  case space: if (abNegNumberFormat[3] == number) lpf->bNegStyle = 10; break;
                }
                break;
            }
          break;
          default:
            switch (lpf->currencyfmt.bPosStyle)
            {
              case 1:
                lpf->bNegStyle = 2;
              break;
              case 2:
                lpf->bNegStyle = 6;
              break;
              case 3:
                lpf->bNegStyle = 10;
              break;
              case 4:
                lpf->bNegStyle = 9;
              break;
            }
          break;
        }
      }
      else if (fPercent)
      {
        // percent celltype
        *lpwType = PERCENTTYPE;

        if (iDigitsRightForce && !iDigitsRight)
        {
          lpf->Right = iDigitsRightForce;
        }
        else if (iDigitsRight)
        {
          xl_LogFile(lpSS->lpBook, LOG_FORMAT0FORCED, 0, 0, lptstr);
          lpf->Right = iDigitsRightForce + iDigitsRight;
        }
        if (iDigitsLeftForce)
          lpf->bLeadingZero = 2;

        lpf->Max = 999999999999.99;
        lpf->Min = -999999999999.99;

#ifdef SS_V70
        // Negative format style
        if (abNegNumberFormat[0] == space)
        {
          abNegNumberFormat[0] = abNegNumberFormat[1];
          abNegNumberFormat[1] = abNegNumberFormat[2];
          abNegNumberFormat[2] = abNegNumberFormat[3];
        }
        switch (abNegNumberFormat[0])
        {
          case number:
            switch (abNegNumberFormat[1])
            {
              case percent:
                switch (abNegNumberFormat[2])
                {
                  case negative: lpf->bNegStyle = 4; break;
                  default: lpf->bNegStyle = (fNegParen? 1 : 4); break;
                }
              break;
              case negative:
                switch (abNegNumberFormat[2])
                {
                  case percent: lpf->bNegStyle = 3; break;
                  case space: if (abNegNumberFormat[3] == percent) lpf->bNegStyle = 7; break;
                }
              break;
              case space:
                switch (abNegNumberFormat[2])
                {
                  case percent:
                    switch (abNegNumberFormat[3])
                    {
                      case negative: lpf->bNegStyle = 6; break;
                      default: lpf->bNegStyle = (fNegParen? 8 : 6); break;
                    }
                  break;
                }       
              break;
            }
          break;
          case negative:
            switch (abNegNumberFormat[1])
            {
              case number: 
                switch (abNegNumberFormat[2])
                {
                  case percent: lpf->bNegStyle = 2; break;
                  case space: if (abNegNumberFormat[3] == percent) lpf->bNegStyle = 5; break;
                }
              break;
            }
          break;
          default:
            switch (lpf->currencyfmt.bPosStyle)
            {
              case 1:
                lpf->bNegStyle = 2;
              break;
              case 2:
                lpf->bNegStyle = 6;
              break;
              case 3:
                lpf->bNegStyle = 10;
              break;
              case 4:
                lpf->bNegStyle = 9;
              break;
            }
          break;
        }
#else
        if (fNegParen)
          lpf->bNegStyle = 1;
        else
          lpf->bNegStyle = 2;

#endif
      }
      else if (iDigitsRight || iDigitsRightForce || iDigitsLeft || iDigitsLeftForce || fDecimal || fSeparator)
      {
#ifdef SS_V70
        if (fScientific)
        {
          *lpwType = SCIENTIFICTYPE;
          lpf->Min = -1.7E308;
          lpf->Max = 1.7E308;
        }
        else
#endif
        {
          // number celltype
          *lpwType = NUMBERTYPE;
          lpf->Max = 999999999999.99;
          lpf->Min = -999999999999.99;
        }

        if (fScientific)
          lpf->Right = iSciDecimals;
        else
        {
          if (iDigitsRightForce && !iDigitsRight)
            lpf->Right = iDigitsRightForce;
          else if (iDigitsRight)
          {
            xl_LogFile(lpSS->lpBook, LOG_FORMAT0FORCED, 0, 0, lptstr);
            lpf->Right = iDigitsRightForce + iDigitsRight;
          }
          if (iDigitsLeftForce)
            lpf->bLeadingZero = 2;
        } // not Scientific


        lpf->fShowSeparator = fSeparator;

        if (fNegParen)
          lpf->bNegStyle = 1;
        else
        {
          if (abNegNumberFormat[0] == space)
          {
            abNegNumberFormat[0] = abNegNumberFormat[1];
            abNegNumberFormat[1] = abNegNumberFormat[2];
            abNegNumberFormat[2] = abNegNumberFormat[3];
          }

          if (abNegNumberFormat[0] == negative)
          {
            if (abNegNumberFormat[1] == number) lpf->bNegStyle = 2;
            else if (abNegNumberFormat[1] == space && abNegNumberFormat[2] == number) lpf->bNegStyle = 3;
          }
          else if (abNegNumberFormat[0] == number)
          {
            if (abNegNumberFormat[1] == negative) lpf->bNegStyle = 4;
            else if (abNegNumberFormat[1] == space && abNegNumberFormat[2] == negative) lpf->bNegStyle = 5;
          }
        }
      }
#else
      else if (iDigitsRight || iDigitsRightForce || fDecimal || fCurrency || fPercent || fSeparator)
      {
         //Float
        *lpwType = FLOATTYPE;

        if (fScientific)
        {
          lpf->floatfmt.nDigitsRight = iSciDecimals;
          lpf->floatfmt.nDigitsLeft = 14-iSciDecimals;
        }
        else
        {
          if (iDigitsRightForce && !iDigitsRight)
          {
            lpf->floatfmt.nDigitsRight = iDigitsRightForce;
          }
          else if (iDigitsRight)
          {
            xl_LogFile(lpSS->lpBook, LOG_FORMAT0FORCED, 0, 0, lptstr);
            lpf->floatfmt.nDigitsRight = iDigitsRightForce + iDigitsRight;
          }
          if (iDigitsLeftForce)
          {
            xl_LogFile(lpSS->lpBook, LOG_FORMAT0IGNORED, 0, 0, lptstr);
            lpf->floatfmt.nDigitsLeft = 14 - lpf->floatfmt.nDigitsRight;
          }
          else
            lpf->floatfmt.nDigitsLeft = 14 - lpf->floatfmt.nDigitsRight;
        } // not Scientific

        lpf->floatfmt.fDigitLimit = TRUE;

        lpf->floatfmt.dblMax = pow(10, lpf->floatfmt.nDigitsLeft) - (lpf->floatfmt.nDigitsRight?pow(10, (-lpf->floatfmt.nDigitsRight)):1);
        lpf->floatfmt.dblMin = -lpf->floatfmt.dblMax;

        lpf->floatfmt.fSeparator = fSeparator;
        lpf->floatfmt.fCurrency = fCurrency;
        if (fCurrency && acLiteral[iCurrencyPos][0] != 0)
        {
          lstrcpy(lpf->floatfmt.szCurrency, (LPTSTR)acLiteral[iCurrencyPos]);
          if (lstrlen(acLiteral[iCurrencyPos]) > 1)
          {
            xl_LogFile(lpSS->lpBook, LOG_CURRENCYMULTIPLECHARS, 0, 0, &(acLiteral[iCurrencyPos]));
          }
          if (!fCurrencySymbolFirst)
          {
            xl_LogFile(lpSS->lpBook, LOG_CURRENCYNOTFIRST, 0, 0, &(acLiteral[iCurrencyPos]));
          }
        }
      }//Float
      else if (fFraction)
        *lpwType = FLOATTYPE;
      else if (iDigitsLeft + iDigitsLeftForce + (2*iLiterals) == iLen)
      {
  #ifdef SPREAD_JPN
	    if(fDBCSSign)
		    *lpwType = INTEGERTYPE;
	    else
  #endif
        *lpwType = MASKTYPE;
      }
      else if (iDigitsLeft || iDigitsLeftForce)
      {
        //Integer
        *lpwType = INTEGERTYPE;
      }//Integer
#endif
#ifdef SS_V40
      else
      {
        //Edit
        lpf->Max = 999999999999.99; //19564 - just in case it evaluates to a number, make sure the min & max values are correct.
        lpf->Min = -999999999999.99;
      }//Edit
#endif
    } 
    else  //Not Supported
    { 
      *lpwType = 0;
     break;
    }
  } 

  return Ret;
}

#endif //SS_V30