//--------------------------------------------------------------------
//
//  File: cal_cust.c
//
//  Description: Routines to handle custom functions in formulas
//

#include <ctype.h>
#include <string.h>

#include "calc.h"
#include "cal_cust.h"
#include "cal_expr.h"
#include "cal_mem.h"
#include "cal_name.h"

//--------------------------------------------------------------------
//
//  The CustFuncInit() function initializes a custom function table.
//

BOOL CustFuncInit(LPCALC_CUSTFUNCTABLE lpTable)
{
  lpTable->hElem = 0;
  lpTable->nElemCnt = 0;
  lpTable->nAllocCnt = 0;
  lpTable->nAllocInc = 10;
  return TRUE;
}

//--------------------------------------------------------------------
//
//  The CustFuncFree() function destroys a custom function table.
//  All memory associated with the table is freed.
//

BOOL CustFuncFree(LPCALC_CUSTFUNCTABLE lpTable)
{
  LPCALC_HANDLE lpElem;
  int i;
 
  if( lpTable->hElem ) 
  {
    if( lpElem = (LPCALC_HANDLE)CalcMemLock(lpTable->hElem) )
    {
      for( i = 0; i < lpTable->nElemCnt; i++ )
        CustFuncDestroy(lpElem[i]);
      CalcMemUnlock(lpTable->hElem);
    }
    CalcMemFree(lpTable->hElem);
  }
  lpTable->hElem = 0;
  lpTable->nElemCnt = 0;
  lpTable->nAllocCnt = 0;
  return TRUE;
}

//--------------------------------------------------------------------
//
//  The Search() function uses a binary search to find the desired
//  custom function in the array of custom function handles.  If
//  found, the function returns TRUE and *lpnPos contains the
//  position of the function in the array.  If not found, the
//  function return FALSE and *lpnPos contains the position to
//  insert the function.
//
//  Note: It is assumed that the array is sorted in ascending order
//        by function name.
//

static BOOL Search(LPCTSTR lpszName, LPCALC_HANDLE lpElem, int nElemCnt,
                   int FAR* lpnPos)
{
  LPCALC_CUSTFUNC lpFunc;
  LPTSTR lpszFunc;
  int nFirst = 0;
  int nLast = nElemCnt - 1;
  int nMiddle = (nFirst + nLast) / 2;
  int iCmp = 1;
  
  while( iCmp != 0 && nFirst <= nLast )
  {
    if( lpElem[nMiddle]
        && (lpFunc = (LPCALC_CUSTFUNC)CalcMemLock(lpElem[nMiddle])) )
    {
      if( lpszFunc = (LPTSTR)CalcMemLock(lpFunc->hText) )
      {
        iCmp = lstrcmpi(lpszName, lpszFunc);
        if( iCmp < 0 )
          nLast = nMiddle - 1;  
        else if( iCmp > 0 )
          nFirst = nMiddle + 1;
        CalcMemUnlock(lpFunc->hText);
      }
      else  // failed to lock function name
        nLast = nMiddle - 1;
      CalcMemUnlock(lpElem[nMiddle]);
    }
    else  // failed to lock custom function
      nLast = nMiddle - 1;
    nMiddle = (nFirst + nLast) / 2;    
  }
  *lpnPos = iCmp == 0 ? nMiddle : nFirst;
  return iCmp == 0;
}

//--------------------------------------------------------------------
//
//  The CustFuncAdd() function adds a custom function to the table.
//  If the name already existed, the old function is replaced with
//  the new function.  If needed, the size of the array of handles is
//  expanded.  If successful, the function returns TRUE.  Otherwise,
//  if returns FALSE.
//

BOOL CustFuncAdd(LPCALC_CUSTFUNCTABLE lpTable, CALC_HANDLE hFunc)
{
  LPCALC_HANDLE lpElem;
  LPCALC_CUSTFUNC lpFunc;
  LPCTSTR lpszName;
  BOOL bReturn = FALSE;
  int nPos;
  int i;
  
  if( lpTable->nElemCnt >=  lpTable->nAllocCnt )
  {
    lpTable->nAllocCnt += lpTable->nAllocInc;
    if( lpTable->hElem )
      lpTable->hElem = CalcMemReAlloc(lpTable->hElem,
                                   lpTable->nAllocCnt * sizeof(CALC_HANDLE));
    else
      lpTable->hElem = CalcMemAlloc(lpTable->nAllocCnt * sizeof(CALC_HANDLE));
    if( !lpTable->hElem )
    {
      lpTable->nElemCnt = 0;
      lpTable->nAllocCnt = 0;
    }
  }
  if( lpTable->hElem && (lpElem = (LPCALC_HANDLE)CalcMemLock(lpTable->hElem)) )
  {
    if( hFunc && (lpFunc = (LPCALC_CUSTFUNC)CalcMemLock(hFunc)) )
    {
      if( lpFunc->hText && (lpszName = (LPTSTR)CalcMemLock(lpFunc->hText)) )
      {
        if( !Search(lpszName, lpElem, lpTable->nElemCnt, &nPos) )
        {
          if( hFunc = CustFuncCreateRef(hFunc) )
          {
            for( i = lpTable->nElemCnt; i > nPos; i-- )
              lpElem[i] = lpElem[i-1];
            lpTable->nElemCnt++;
            lpElem[nPos] = hFunc;
            bReturn = TRUE;
          }
        }
        CalcMemUnlock(lpFunc->hText);
      }
      CalcMemUnlock(hFunc);
    }
    CalcMemUnlock(lpTable->hElem);
  }
  return bReturn;
}

//--------------------------------------------------------------------
//
//  The CustFuncRemove() function removes a custom function entry
//  from the given table.  If successful, the function returns
//  TRUE. Otherwise, the function returns FALSE.
//

BOOL CustFuncRemove(LPCALC_CUSTFUNCTABLE lpTable, CALC_HANDLE hFunc)
{
  LPCALC_HANDLE lpElem;
  BOOL bReturn = FALSE;
  int i;
  
  if( lpTable->hElem && (lpElem = (LPCALC_HANDLE)CalcMemLock(lpTable->hElem)) )
  {
    for( i = 0; i < lpTable->nElemCnt; i++ )
    {
      if( hFunc == lpElem[i] )
      {
        CustFuncDestroy(lpElem[i]);
        lpTable->nElemCnt--;
        for( ; i < lpTable->nElemCnt; i++ )
          lpElem[i] = lpElem[i+1];
        bReturn = TRUE;
      }
    }
    CalcMemUnlock(lpTable->hElem);
  }
  return bReturn;
}

//--------------------------------------------------------------------
//
//  The CustFuncLookup() function retrieves a custom function from
//  the given table.  If found, the function returns a handle to the
//  desired custom function.  If not found, the function returns NULL.

CALC_HANDLE CustFuncLookup(LPCALC_CUSTFUNCTABLE lpTable, LPCTSTR lpszName)
{
  LPCALC_HANDLE lpElem;
  CALC_HANDLE hReturn = 0;
  int nPos;
  
  if( lpTable->hElem && (lpElem = (LPCALC_HANDLE)CalcMemLock(lpTable->hElem)) )
  {
    if( Search(lpszName, lpElem, lpTable->nElemCnt, &nPos) )
      hReturn = lpElem[nPos];
    CalcMemUnlock(lpTable->hElem);
  }
  return hReturn;
}

//--------------------------------------------------------------------
//
//  The CustFuncFirst() function retrieves the first custom function
//  in the given table.  If successful, the function returns a handle
//  to the desired function.  Otherwise, the function returns NULL.
//

CALC_HANDLE CustFuncFirst(LPCALC_CUSTFUNCTABLE lpTable)
{
  LPCALC_HANDLE lpElem;
  CALC_HANDLE hReturn = 0;
  
  if( lpTable->hElem
      && lpTable->nElemCnt > 0
      && (lpElem = (LPCALC_HANDLE)CalcMemLock(lpTable->hElem)) )
  {
    hReturn = lpElem[0];
    CalcMemUnlock(lpTable->hElem);
  }
  return hReturn;
}

//--------------------------------------------------------------------
//
//  The CustFuncNext() function retrieves the next custom function
//  in the given table.  If successful, the function returns a handle
//  to the desired function.  Otherwise, the function returns NULL.
//

CALC_HANDLE CustFuncNext(LPCALC_CUSTFUNCTABLE lpTable, LPCTSTR lpszName)
{
  LPCALC_HANDLE lpElem;
  CALC_HANDLE hReturn = 0;
  int nPos;
  
  if( lpTable->hElem && (lpElem = (LPCALC_HANDLE)CalcMemLock(lpTable->hElem)) )
  {
    if( Search(lpszName, lpElem, lpTable->nElemCnt, &nPos) )
      nPos++;
    if( nPos < lpTable->nElemCnt )
      hReturn = lpElem[nPos];
    CalcMemUnlock(lpTable->hElem);
  }
  return hReturn;
}

//--------------------------------------------------------------------
//
//  The CustFuncIsValidName() function determines whether or not
//  the given name is valid.  The first character must be a letter
//  or an underscore.  Remaining characters must be a letter, a
//  digit, or an underscore. 
//

BOOL CustFuncIsValidName(LPCTSTR lpszName)
{
  BOOL bRet = FALSE;

  if( '_' == *lpszName || _istalpha((_TUCHAR)*lpszName) )
  {
    for( bRet = TRUE; bRet && *lpszName ; lpszName++)
      bRet = '_' == *lpszName || _istalnum((_TUCHAR)*lpszName);
  }
  return bRet;
}

//--------------------------------------------------------------------
//
//  The CustFuncCreate() function creates a custom function.  If
//  sucessful, the function returns the handle to the custom function.
//  Otherwise, the function returns NULL.
//

CALC_HANDLE CustFuncCreate(LPCTSTR lpszName, short nMinArgs, short nMaxArgs,
                           CALC_CUSTEVALPROC lpfnProc, long lFlags)
{
  LPCALC_CUSTFUNC lpFunc;
  CALC_HANDLE hFunc;
  CALC_HANDLE hCopy;
  CALC_HANDLE hReturn = 0;
  LPTSTR lpszCopy;

  if( lpszName && lpszName[0] && CustFuncIsValidName(lpszName) )
  {
    hFunc = CalcMemAlloc(sizeof(CALC_CUSTFUNC));
    if( hFunc && (lpFunc = (LPCALC_CUSTFUNC)CalcMemLock(hFunc)) )
    {
      hCopy = CalcMemAlloc((lstrlen(lpszName)+1) * sizeof(TCHAR));
      if( hCopy && (lpszCopy = (LPTSTR)CalcMemLock(hCopy)) )
      {
        lstrcpy(lpszCopy, lpszName);
        lpFunc->hText = hCopy;
        lpFunc->lpfnProc = lpfnProc;
        lpFunc->nMinArgs = nMinArgs;
        lpFunc->nMaxArgs = nMaxArgs;
        lpFunc->lFlags = lFlags;
        lpFunc->lRefCnt = 1;
        hReturn = hFunc;
       CalcMemUnlock(hCopy);
      }   
      CalcMemUnlock(hFunc);
    }
  }
  return hReturn;
}

//--------------------------------------------------------------------
//
//  The CustFuncCreateRef() function increments the reference counter
//  for the custom function.  If successful the function returns
//  the handle to the custom function.  Otherwise, the function
//  returns NULL.
//

CALC_HANDLE CustFuncCreateRef(CALC_HANDLE hFunc)
{
  LPCALC_CUSTFUNC lpFunc;
  CALC_HANDLE hReturn = 0;
  
  if( hFunc && (lpFunc = (LPCALC_CUSTFUNC)CalcMemLock(hFunc)) )
  {
    lpFunc->lRefCnt++;
    hReturn = hFunc;
    CalcMemUnlock(hFunc);
  }
  return hReturn;
}

//--------------------------------------------------------------------
//
//  The CustFuncDestroy() function decrements the reference counter
//  for the custom function.  If the reference reaches zero then all
//  memory associated the custom function is reclaimed.
//

BOOL CustFuncDestroy(CALC_HANDLE hFunc)
{
  LPCALC_CUSTFUNC lpFunc;
  BOOL bReturn = FALSE;
  BOOL bNoReferences = FALSE;
  
  if( hFunc && (lpFunc = (LPCALC_CUSTFUNC)CalcMemLock(hFunc)) )
  {
    lpFunc->lRefCnt--;
    if( bNoReferences = lpFunc->lRefCnt == 0 )
    {
      CalcMemFree(lpFunc->hText);
      bReturn = TRUE;
    }
    CalcMemUnlock(hFunc);
  }
  if( bNoReferences )
    CalcMemFree(hFunc);
  return bReturn;
}

//--------------------------------------------------------------------
//
//  The CustFuncGetRefCnt() function retrieves the reference counter
//  for the custom function.  If successful the function retruns
//  TRUE.  Otherwise, the function returns FALSE.
//

long CustFuncGetRefCnt(CALC_HANDLE hFunc)
{
  LPCALC_CUSTFUNC lpFunc;
  long lReturn = 0;
  
  if( hFunc && (lpFunc = (LPCALC_CUSTFUNC)CalcMemLock(hFunc)) )
  {
    lReturn = lpFunc->lRefCnt;
    CalcMemUnlock(hFunc);
  }
  return lReturn;
}

//--------------------------------------------------------------------
//
//  The CustFuncGetText() function retrieves the text representation
//  of the function's name.  If successful, the function returns
//  the length of lpszText.  Otherwise, the function returns zero.
//

int CustFuncGetText(CALC_HANDLE hFunc, LPTSTR lpszText, int nLen)
{
  LPCALC_CUSTFUNC lpFunc;
  LPTSTR lpszFunc;
  int nFuncLen;
  int iResult = 0;
  
  if( nLen && hFunc && (lpFunc = (LPCALC_CUSTFUNC)CalcMemLock(hFunc)) )
  {
     if( lpFunc->hText && (lpszFunc = (LPTSTR)CalcMemLock(lpFunc->hText)) )
     {
       nFuncLen = lstrlen(lpszFunc);
       if( nFuncLen < nLen )
       {
         lstrcpy(lpszText, lpszFunc);
         iResult = nFuncLen;
       }
       CalcMemUnlock(lpFunc->hText);
     }
     CalcMemUnlock(hFunc);
  }
  return iResult;
}

//--------------------------------------------------------------------
//
//  The CustFuncGetTextLen() function retrieves the length of the
//  text representation of the function's name.
//

int CustFuncGetTextLen(CALC_HANDLE hFunc)
{
  LPCALC_CUSTFUNC lpFunc;
  LPTSTR lpszFunc;
  int iResult = 0;
  
  if( hFunc && (lpFunc = (LPCALC_CUSTFUNC)CalcMemLock(hFunc)) )
  {
     if( lpFunc->hText && (lpszFunc = (LPTSTR)CalcMemLock(lpFunc->hText)) )
     {
       iResult = lstrlen(lpszFunc);
       CalcMemUnlock(lpFunc->hText);
     }
     CalcMemUnlock(hFunc);
  }
  return iResult;
}

//--------------------------------------------------------------------
//
//  The CustFuncLockText() function locks the text representation
//  of the name.
//

LPCTSTR CustFuncLockText(CALC_HANDLE hFunc)
{
  LPCALC_CUSTFUNC lpFunc;
  LPCTSTR lpszFunc = NULL;
  if( hFunc && (lpFunc = (LPCALC_CUSTFUNC)CalcMemLock(hFunc)) )
  {
    if( lpFunc->hText )
    {
      lpszFunc = (LPCTSTR)CalcMemLock(lpFunc->hText);
    }
    CalcMemUnlock(hFunc);
  }
  return lpszFunc;
}

//--------------------------------------------------------------------
//
//  The CustFuncUnlockText() function unlocks the text representation
//  of the name.
//

void CustFuncUnlockText(CALC_HANDLE hFunc)
{
  LPCALC_CUSTFUNC lpFunc;
  LPCTSTR lpszFunc = NULL;
  if( hFunc && (lpFunc = (LPCALC_CUSTFUNC)CalcMemLock(hFunc)) )
  {
    if( lpFunc->hText )
    {
      CalcMemUnlock(lpFunc->hText);
    }
    CalcMemUnlock(hFunc);
  }
}

//--------------------------------------------------------------------
//
//  The CustFuncGetProc() function retrieves the procedure used to
//  evaulate a custom function.
//

CALC_CUSTEVALPROC CustFuncGetProc(CALC_HANDLE hFunc)
{
  LPCALC_CUSTFUNC lpFunc;
  CALC_CUSTEVALPROC lpfnResult = NULL;
  
  if( hFunc && (lpFunc = (LPCALC_CUSTFUNC)CalcMemLock(hFunc)) )
  {
    lpfnResult = lpFunc->lpfnProc;
    CalcMemUnlock(hFunc);
  }
  return lpfnResult;
}

//--------------------------------------------------------------------
//
//  The CustFuncSetProc() function assigns a callback procedure used
//  to evaluate a custom function.
//

BOOL CustFuncSetProc(CALC_HANDLE hFunc, CALC_CUSTEVALPROC lpfnProc)
{
  LPCALC_CUSTFUNC lpFunc;
  BOOL bRet = FALSE;

  if( hFunc && (lpFunc = (LPCALC_CUSTFUNC)CalcMemLock(hFunc)) )
  {
    lpFunc->lpfnProc = lpfnProc;
    bRet = TRUE;
    CalcMemUnlock(hFunc);
  }
  return bRet;
}

//--------------------------------------------------------------------
//
//  The CustFuncGetMinArgs() function retrieves the minimum number of
//  parameters which the given custom function will accept.
//

short CustFuncGetMinArgs(CALC_HANDLE hFunc)
{
  LPCALC_CUSTFUNC lpFunc;
  int nResult = 0;
  
  if( hFunc && (lpFunc = (LPCALC_CUSTFUNC)CalcMemLock(hFunc)) )
  {
    nResult = lpFunc->nMinArgs;
    CalcMemUnlock(hFunc);
  }
  return nResult;
}

//--------------------------------------------------------------------
//
//  The CustFuncSetMinArgs() function retrieves the minimum number of
//  parameters which the given custom function will accept.
//

short CustFuncSetMinArgs(CALC_HANDLE hFunc, short nMinArgs)
{
  LPCALC_CUSTFUNC lpFunc;
  int nResult = 0;
  
  if( hFunc && (lpFunc = (LPCALC_CUSTFUNC)CalcMemLock(hFunc)) )
  {
    nResult = lpFunc->nMinArgs;
    if( nMinArgs < lpFunc->nMinArgs )
      lpFunc->nMinArgs = nMinArgs;
    CalcMemUnlock(hFunc);
  }
  return nResult;
}

//--------------------------------------------------------------------
//
//  The CustFuncGetMaxArgs() function retrieves the maxium number of
//  parameters which the given custom function will accept.
//

short CustFuncGetMaxArgs(CALC_HANDLE hFunc)
{
  LPCALC_CUSTFUNC lpFunc;
  int nResult = 0;
  
  if( hFunc && (lpFunc = (LPCALC_CUSTFUNC)CalcMemLock(hFunc)) )
  {
    nResult = lpFunc->nMaxArgs;
    CalcMemUnlock(hFunc);
  }
  return nResult;
}

//--------------------------------------------------------------------
//
//  The CustFuncSetMaxArgs() function retrieves the maxium number of
//  parameters which the given custom function will accept.
//

short CustFuncSetMaxArgs(CALC_HANDLE hFunc, short nMaxArgs)
{
  LPCALC_CUSTFUNC lpFunc;
  int nResult = 0;
  
  if( hFunc && (lpFunc = (LPCALC_CUSTFUNC)CalcMemLock(hFunc)) )
  {
    nResult = lpFunc->nMaxArgs;
    if( nMaxArgs > lpFunc->nMaxArgs )
      lpFunc->nMaxArgs = nMaxArgs;
    CalcMemUnlock(hFunc);
  }
  return nResult;
}

//--------------------------------------------------------------------
//
//  The CustFuncGetFlags() function  returns flags which determine
//  if the custom function accepts cell and/or range references.
//

long CustFuncGetFlags(CALC_HANDLE hFunc)
{
  LPCALC_CUSTFUNC lpFunc;
  long lResult = 0;

  if( hFunc && (lpFunc = (LPCALC_CUSTFUNC)CalcMemLock(hFunc)) )
  {
    lResult = lpFunc->lFlags;
    CalcMemUnlock(hFunc);
  }
  return lResult;
}
