//--------------------------------------------------------------------
//
//  File: cal_cust.h
//
//  Description: Routines to handle custom functions in formulas
//

#ifndef CAL_CUST_H
#define CAL_CUST_H

#ifdef __cplusplus
extern "C" {
#endif

//--------------------------------------------------------------------
//
//  The following diagram shows an example of a custom function table
//  that contains "cube(x)" custom function.
//
//  +-----------+     +--------+--------+--------+---
//  | hElem     |---->| handle | handle | handle | ...
//  | nElemCnt  |     +--------+--------+--------+---
//  | nAllocCnt |       |
//  | nAllocInc |       |
//  +-----------+       |
//                      |     +--------------+     +--------+
//                      +---->| hText        |---->| "cube" |
//                            | lpfnProc     |     +--------+
//                            | nMinArgs = 1 |
//                            | nMaxArgs = 1 |
//                            | lRefCnt = 1  |
//                            +--------------+
//

//--------------------------------------------------------------------
//
//  The CALC_CUSTFUNC structure represents a custom function.
//

typedef struct tagCALC_CUSTFUNC
{
  CALC_HANDLE hText;               // handle to name of custom function
  CALC_CUSTEVALPROC lpfnProc;      // evaluation procedure
  short nMinArgs;                  // min number of arguements
  short nMaxArgs;                  // max number of arguements
  long lFlags;                     // want cell and/or range references
  long lRefCnt;                    // number of references to custom function
} CALC_CUSTFUNC, FAR* LPCALC_CUSTFUNC;

//--------------------------------------------------------------------
//
//  The following functions are used to handle a table of custom
//  functions.
//

BOOL CustFuncInit(LPCALC_CUSTFUNCTABLE lpTable);
BOOL CustFuncFree(LPCALC_CUSTFUNCTABLE lpTable);
BOOL CustFuncAdd(LPCALC_CUSTFUNCTABLE lpTable, CALC_HANDLE hFunc);
BOOL CustFuncRemove(LPCALC_CUSTFUNCTABLE lpTable, CALC_HANDLE hFunc);
CALC_HANDLE CustFuncLookup(LPCALC_CUSTFUNCTABLE lpTable, LPCTSTR lpszName);
CALC_HANDLE CustFuncFirst(LPCALC_CUSTFUNCTABLE lpTable);
CALC_HANDLE CustFuncNext(LPCALC_CUSTFUNCTABLE lpTable, LPCTSTR lpszName);

//--------------------------------------------------------------------
//
//  The following functions are used to handle an individual custom
//  function.
//

CALC_HANDLE CustFuncCreate(LPCTSTR lpszName, short nMinArgs, short nMaxArgs,
                           CALC_CUSTEVALPROC lpfnProc, long lFlags);
CALC_HANDLE CustFuncCreateRef(CALC_HANDLE hFunc);
BOOL CustFuncDestroy(CALC_HANDLE hFunc);
long CustFuncGetRefCnt(CALC_HANDLE hFunc);
int  CustFuncGetText(CALC_HANDLE hFunc, LPTSTR lpszText, int nLen);
int  CustFuncGetTextLen(CALC_HANDLE hFunc);
LPCTSTR CustFuncLockText(CALC_HANDLE hFunc);
void CustFuncUnlockText(CALC_HANDLE hFunc);
short CustFuncGetMinArgs(CALC_HANDLE hFunc);
short CustFuncGetMaxArgs(CALC_HANDLE hFunc);
long CustFuncGetFlags(CALC_HANDLE hFunc);
CALC_CUSTEVALPROC CustFuncGetProc(CALC_HANDLE hFunc);
short CustFuncSetMinArgs(CALC_HANDLE hFunc, short nMinArgs);
short CustFuncSetMaxArgs(CALC_HANDLE hFunc, short nMaxArgs);
BOOL CustFuncSetProc(CALC_HANDLE hFunc, CALC_CUSTEVALPROC lpfnProc);

//--------------------------------------------------------------------

#ifdef __cplusplus
}
#endif

#endif
