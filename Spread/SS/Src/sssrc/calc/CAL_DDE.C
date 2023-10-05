//--------------------------------------------------------------------
//
//  File: cal_dde.c
//
//  Description: Routines to support DDE links in formulas
//

#if defined(SS_DDE)

//--------------------------------------------------------------------

#include <string.h>

#include "calc.h"
#include "cal_dde.h"
#include "cal_func.h"
#include "cal_mem.h"
#include "cal_name.h"

#include <oleauto.h>
#include <olectl.h>

//--------------------------------------------------------------------

HRESULT DispCallMethod(LPDISPATCH lpdisp, LPOLESTR lpszMeth);
HRESULT DispGetPropVariant(LPDISPATCH lpdisp, LPOLESTR lpszProp, LPVARIANT lpvar);
HRESULT DispGetPropBool(LPDISPATCH lpdisp, LPOLESTR lpszProp, BOOL FAR* lpbVal);
HRESULT DispGetPropShort(LPDISPATCH lpdisp, LPOLESTR lpszProp, short FAR* lpnVal);
HRESULT DispSetPropVariant(LPDISPATCH lpdisp, LPOLESTR lpszProp, LPVARIANT lpVar);
HRESULT DispSetPropString(LPDISPATCH lpdisp, LPOLESTR lpszProp, LPCTSTR lpszVal);
HRESULT DispSetPropShort(LPDISPATCH lpdisp, LPOLESTR lpszProp, short nVal);
HRESULT DispSetPropDouble(LPDISPATCH lpdisp, LPOLESTR lpszProp, double dfVal);

LPDISPATCH LinkCreate(LPCTSTR lpszServer, LPCTSTR lpszTopic, LPCTSTR lpszItem, CALC_HANDLE hDde);
BOOL LinkDestroy(LPDISPATCH lpdisp, DWORD dwCookie);

//--------------------------------------------------------------------
//
//  The DdeInit() function initializes a table of DDE links.
//

BOOL DdeInit(LPCALC_DDETABLE lpTable)
{
  lpTable->hElem = 0;
  lpTable->nElemCnt = 0;
  lpTable->nAllocCnt = 0;
  lpTable->nAllocInc = 5;
  return TRUE;
}

//--------------------------------------------------------------------
//
//  The DdeFree() function frees all memory associated with a table
//  of DDE links.
//

BOOL DdeFree(LPCALC_DDETABLE lpTable)
{
  LPCALC_HANDLE lpElem;
  int i;
  
  if( lpTable->hElem )
  {
    if( lpElem = (LPCALC_HANDLE)CalcMemLock(lpTable->hElem) )
    {
      for( i = 0; i < lpTable->nElemCnt; i++ )
        DdeDestroy(lpElem[i]);
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
//
//

static BOOL Search(LPCTSTR lpszServer, LPCTSTR lpszTopic, LPCTSTR lpszItem,
                   LPCALC_HANDLE lpElem, int nElemCnt, int FAR* lpnPos)
{
  LPCALC_DDE lpDde;
  LPTSTR lpszDdeServer;
  LPTSTR lpszDdeTopic;
  LPTSTR lpszDdeItem;
  int nFirst = 0;
  int nLast = nElemCnt - 1;
  int nMiddle = (nFirst + nLast) / 2;
  int iCmp = 1;

  while( iCmp != 0 && nFirst <= nLast )
  {
    if( lpElem[nMiddle] &&
        (lpDde = (LPCALC_DDE)CalcMemLock(lpElem[nMiddle])) )
    {
      if( (lpszDdeServer = (LPTSTR)CalcMemLock(lpDde->hServer)) &&
          (lpszDdeTopic = (LPTSTR)CalcMemLock(lpDde->hTopic)) &&
          (lpszDdeItem = (LPTSTR)CalcMemLock(lpDde->hItem)) )
      {
        iCmp = lstrcmpi(lpszServer, lpszDdeServer);
        if( 0 == iCmp )
          iCmp = lstrcmpi(lpszTopic, lpszDdeTopic);
        if( 0 == iCmp )
          iCmp = lstrcmpi(lpszItem, lpszDdeItem);
        if( iCmp < 0 )
          nLast = nMiddle - 1;
        else if( iCmp > 0 )
          nFirst = nMiddle + 1;
        CalcMemUnlock(lpDde->hServer);
        CalcMemUnlock(lpDde->hTopic);
        CalcMemUnlock(lpDde->hItem);
      }
      else // failed to lock DDE object
        nLast = nMiddle - 1;
      CalcMemUnlock(lpElem[nMiddle]);
    }
    else
      nLast = nMiddle - 1;
    nMiddle = (nFirst + nLast) / 2;
  }
  *lpnPos = iCmp == 0 ? nMiddle : nFirst;
  return iCmp == 0;
}

//--------------------------------------------------------------------
//
//  The DdeAdd() function adds a DDE object to the table.  If the
//  DDE object already existed, then the table is unmodified.  If
//  needed, the size of the array of handles is expanded.  If
//  successful, the function returns TRUE.  Otherwise, the function
//  returns FALSE.
//

BOOL DdeAdd(LPCALC_DDETABLE lpTable, CALC_HANDLE hDde)
{
  LPCALC_HANDLE lpElem;
  LPCALC_DDE lpDde;
  LPTSTR lpszServer;
  LPTSTR lpszTopic;
  LPTSTR lpszItem;
  BOOL bReturn = FALSE;
  int nPos;
  int i;

  if( lpTable->nElemCnt >=  lpTable->nAllocCnt )
  {
    lpTable->nAllocCnt += lpTable->nAllocInc;
    if( lpTable->hElem )
      lpTable->hElem = CalcMemReAlloc(lpTable->hElem,
                                      lpTable->nAllocCnt*sizeof(CALC_HANDLE));
    else
      lpTable->hElem = CalcMemAlloc(lpTable->nAllocCnt*sizeof(CALC_HANDLE));
    if( !lpTable->hElem )
    {
      lpTable->nElemCnt = 0;
      lpTable->nAllocCnt = 0;
    }
  }
  if( lpTable->hElem && (lpElem = (LPCALC_HANDLE)CalcMemLock(lpTable->hElem)) )
  {
    if( hDde && (lpDde = (LPCALC_DDE)CalcMemLock(hDde)) )
    {
      if( lpDde->hServer && lpDde->hTopic && lpDde->hItem &&
          (lpszServer = (LPTSTR)CalcMemLock(lpDde->hServer)) &&
          (lpszTopic = (LPTSTR)CalcMemLock(lpDde->hTopic)) &&
          (lpszItem = (LPTSTR)CalcMemLock(lpDde->hItem)) )
      {
        if( !Search(lpszServer, lpszTopic, lpszItem,
                    lpElem, lpTable->nElemCnt, &nPos) )
        {
          if( hDde = DdeCreateRef(hDde) )
          {
            for( i = lpTable->nElemCnt; i > nPos; i-- )
              lpElem[i] = lpElem[i-1];
            lpElem[nPos] = hDde;
            lpTable->nElemCnt++;
            bReturn = TRUE;
          }
        }
        CalcMemUnlock(lpDde->hServer);
        CalcMemUnlock(lpDde->hTopic);
        CalcMemUnlock(lpDde->hItem);
      }
      CalcMemUnlock(hDde);
    }
    CalcMemUnlock(lpTable->hElem);
  }
  return bReturn;
}

//--------------------------------------------------------------------
//
//  The DdeRemove() function removes an entry from the table of DDE
//  links.
//

BOOL DdeRemove(LPCALC_DDETABLE lpTable, CALC_HANDLE hDde)
{
  LPCALC_HANDLE lpElem;
  BOOL bReturn = FALSE;
  int i;
  
  if( lpTable->hElem && (lpElem = (LPCALC_HANDLE)CalcMemLock(lpTable->hElem)) )
  {
    for( i = 0; i < lpTable->nElemCnt; i++ )
    {
      if( hDde == lpElem[i] )
      {
        DdeDestroy(hDde);
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
//  The DdeLookup() function retrieves a DDE link from the table.
//

CALC_HANDLE DdeLookup(LPCALC_DDETABLE lpTable, LPCTSTR lpszServer, LPCTSTR lpszTopic, LPCTSTR lpszItem)
{
  LPCALC_HANDLE lpElem;
  CALC_HANDLE hReturn = 0;
  int nPos;
  
  if( lpTable->hElem && (lpElem = (LPCALC_HANDLE)CalcMemLock(lpTable->hElem)) )
  {
    if( Search(lpszServer, lpszTopic, lpszItem, lpElem, lpTable->nElemCnt, &nPos) )
      hReturn = lpElem[nPos];
    CalcMemUnlock(lpTable->hElem);
  }
  return hReturn;
}

//--------------------------------------------------------------------
//
//  The DdeFirst() function retrieves the first DDE link in the
//  given table.  If successful, the function returns a handle to
//  the desired DDE link.  Otherwise, the function returns NULL.

CALC_HANDLE DdeFirst(LPCALC_DDETABLE lpTable)
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
//  The DdeNext() function retrieves the next DDE link in the given
//  table.  If successful, the function returns a handle to the
//  desired DDE link.  Otherwise, the function return NULL.

CALC_HANDLE DdeNext(LPCALC_DDETABLE lpTable, LPCTSTR lpszServer, LPCTSTR lpszTopic, LPCTSTR lpszItem)
{
  LPCALC_HANDLE lpElem;
  CALC_HANDLE hReturn = 0;
  int nPos;
  
  if( lpTable->hElem && (lpElem = (LPCALC_HANDLE)CalcMemLock(lpTable->hElem)) )
  {
    if( Search(lpszServer, lpszTopic, lpszItem, lpElem, lpTable->nElemCnt, &nPos) )
      nPos++;
    if( nPos < lpTable->nElemCnt )
      hReturn = lpElem[nPos];
    CalcMemUnlock(lpTable->hElem);
  }
  return hReturn;
}

//--------------------------------------------------------------------
//
//  The DdeCreate() function creates a DDE link.  If successful, the
//  function returns a handle to the DDE object.  Otherwise, the
//  function returns NULL.
//

CALC_HANDLE DdeCreate(LPCTSTR lpszServer, LPCTSTR lpszTopic, LPCTSTR lpszItem,
                      LPCALC_DDEPROC lpfnDdeUpdate, long hSS)
{
  LPCALC_DDE lpDde;
  CALC_HANDLE hDde;
  CALC_HANDLE hReturn = 0;
  LPTSTR lpszServerSaved;
  LPTSTR lpszTopicSaved;
  LPTSTR lpszItemSaved;

  if( lpszServer && lpszTopic && lpszItem &&
      lpszServer[0] && lpszTopic[0] && lpszItem[0] )
  {
    hDde = CalcMemAlloc(sizeof(CALC_DDE));
    if( hDde && (lpDde = (LPCALC_DDE)CalcMemLock(hDde)) )
    {
      lpDde->hServer = CalcMemAlloc((lstrlen(lpszServer)+1) * sizeof(TCHAR));
      lpDde->hTopic = CalcMemAlloc((lstrlen(lpszTopic)+1) * sizeof(TCHAR));
      lpDde->hItem = CalcMemAlloc((lstrlen(lpszItem)+1) * sizeof(TCHAR));
      if( lpDde->hServer && lpDde->hTopic && lpDde->hItem &&
          (lpszServerSaved = (LPTSTR)CalcMemLock(lpDde->hServer)) &&
          (lpszTopicSaved = (LPTSTR)CalcMemLock(lpDde->hTopic)) &&
          (lpszItemSaved = (LPTSTR)CalcMemLock(lpDde->hItem)) )
      {
        lstrcpy(lpszServerSaved, lpszServer);
        lstrcpy(lpszTopicSaved, lpszTopic);
        lstrcpy(lpszItemSaved, lpszItem);
        lpDde->hReadExpr = 0;
        lpDde->hWriteExpr = 0;
        lpDde->hValues = 0;
        lpDde->lValueCols = 0;
        lpDde->lValueRows = 0;
        lpDde->lRefCnt = 1;
        NameInit(&lpDde->DependNames);
        BitMatInit(&lpDde->DependCells);
        BitVectInit(&lpDde->DependCols);
        BitVectInit(&lpDde->DependRows);
        lpDde->lpfnDdeUpdate = lpfnDdeUpdate;
        lpDde->hSS = hSS;
        lpDde->nMode = CALC_LINKMODE_NONE;
        lpDde->lpdispLink = LinkCreate(lpszServer, lpszTopic, lpszItem, hDde);
        if( !lpDde->lpdispLink )
          SS_CalcLinkDispatchNotCreated(lpDde->hSS, lpszServer, lpszTopic, lpszItem);
        hReturn = hDde;
        CalcMemUnlock(lpDde->hServer);
        CalcMemUnlock(lpDde->hTopic);
        CalcMemUnlock(lpDde->hItem);
      }
      CalcMemUnlock(hDde);
    }
  }
  return hReturn;
}

//--------------------------------------------------------------------
//
//  The DdeCreateRef() function increments the reference counter
//  for the DDE object.  If successful, the function returns the
//  handle to the DDE object.  Otherwise, the function returns NULL.
//

CALC_HANDLE DdeCreateRef(CALC_HANDLE hDde)
{
  LPCALC_DDE lpDde;
  CALC_HANDLE hReturn;

  if( hDde && (lpDde = (LPCALC_DDE)CalcMemLock(hDde)) )
  {
    lpDde->lRefCnt++;
    hReturn = hDde;
    CalcMemUnlock(hDde);
  }
  return hDde;
}

//--------------------------------------------------------------------
//
//  The DdeDestroy() function destroys a DDE object.  If the DDE
//  object has references, then only the reference counter is
//  decremented.  If the DDE object has no refernces, then all
//  memory associated with the DDE object is freed.
//

BOOL DdeDestroy(CALC_HANDLE hDde)
{
  LPCALC_DDE lpDde;
  LPCALC_VALUE lpVal;
  long lNumValues;
  long i;
  long hSS = 0;
  BOOL bNoReferences = FALSE;
  BOOL bReturn = FALSE;

  if( hDde && (lpDde = (LPCALC_DDE)CalcMemLock(hDde)) )
  {
    lpDde->lRefCnt--;
    if( bNoReferences = 0 == lpDde->lRefCnt )
    {
      LinkDestroy(lpDde->lpdispLink, lpDde->dwCookie);
      CalcMemFree(lpDde->hServer);
      CalcMemFree(lpDde->hTopic);
      CalcMemFree(lpDde->hItem);
      if( lpDde->hReadExpr )
        CalcMemFree(lpDde->hReadExpr);
      if( lpDde->hWriteExpr )
        CalcMemFree(lpDde->hWriteExpr);
      if( lpDde->hValues )
      {
        if( lpVal = (LPCALC_VALUE)CalcMemLock(lpDde->hValues) )
        {
          lNumValues = lpDde->lValueCols * lpDde->lValueRows;
          for( i = 0; i < lNumValues; i++ )
            ValFree(&lpVal[i]);
          CalcMemUnlock(lpDde->hValues);
        }
        CalcMemFree(lpDde->hValues);
      }
      NameFree(&lpDde->DependNames);
      BitMatFree(&lpDde->DependCells);
      BitVectFree(&lpDde->DependCols);
      BitVectFree(&lpDde->DependRows);
      bReturn = TRUE;
    }
    CalcMemUnlock(hDde);
  }
  if( bNoReferences )
    CalcMemFree(hDde);
  return bReturn;
}

//--------------------------------------------------------------------
//
//  The DdeGetValue() function retrieves the most recently received
//  value of the DDE link.
//

BOOL DdeGetValue(CALC_HANDLE hDde, long lCol, long lRow, LPCALC_VALUE lpValue)
{
  LPCALC_DDE lpDde;
  LPCALC_VALUE lpDdeVal;
  BOOL bReturn = FALSE;

  if( hDde && (lpDde = (LPCALC_DDE)CalcMemLock(hDde)) )
  {
    if( lpDde->hValues && (lpDdeVal = (LPCALC_VALUE)CalcMemLock(lpDde->hValues)))
    {
      if( -1 == lCol )
      {
        if( lpDde->lValueCols * lpDde->lValueRows > 0 )
        {
          ValSetValue(lpValue, &lpDdeVal[0]);
          bReturn = TRUE;
        }
      }
      else if( -1 == lRow )
      {
        if( lpDde->lValueCols * lpDde->lValueRows > lCol )
        {
          ValSetValue(lpValue, &lpDdeVal[lCol]);
          bReturn = TRUE;
        }
      }
      else
      {
        if( lpDde->lValueCols > lCol && lpDde->lValueRows > lRow )
        {
          ValSetValue(lpValue, &lpDdeVal[lCol + lRow * lpDde->lValueCols]);
          bReturn = TRUE;
        }
      }
      CalcMemUnlock(lpDde->hValues);
    }
    CalcMemUnlock(hDde);
  }
  if( !bReturn )
    ValSetEmpty(lpValue);
  return bReturn;
}

//--------------------------------------------------------------------
//
//  The DdeSetValue() function saves the most recently received
//  value of the DDE link.
//

BOOL DdeSetValue(CALC_HANDLE hDde, long lCol, long lRow, LPCALC_VALUE lpValue)
{
  LPCALC_DDE lpDde;
  LPCALC_VALUE lpDdeVal;
  BOOL bRet = FALSE;

  if( hDde && (lpDde = (LPCALC_DDE)CalcMemLock(hDde)) )
  {
    if( lpDde->hValues && (lpDdeVal = (LPCALC_VALUE)CalcMemLock(lpDde->hValues)) )
    {
      if( -1 == lCol )
      {
        if( lpDde->lValueCols * lpDde->lValueRows > 0 )
        {
          ValFree(&lpDdeVal[0]);
          ValSetValue(&lpDdeVal[0], lpValue);
          bRet = TRUE;
        }
      }
      else if( -1 == lRow )
      {
        if( lpDde->lValueCols * lpDde->lValueRows > lCol )
        {
          ValFree(&lpDdeVal[lCol]);
          ValSetValue(&lpDdeVal[lCol], lpValue);
          bRet = TRUE;
        }
      }
      else
      {
        if( lpDde->lValueCols > lCol && lpDde->lValueRows > lRow )
        {
          lpDdeVal += lCol + lRow * lpDde->lValueCols;
          ValFree(lpDdeVal);
          ValSetValue(lpDdeVal, lpValue);
          bRet = TRUE;
        }
      }
      CalcMemUnlock(lpDde->hValues);
    }
    CalcMemUnlock(hDde);
  }
  return bRet;
}

//--------------------------------------------------------------------

BSTR BstrFromTstr(LPCTSTR lpszSrc)
{
BSTR bstr;
int nLen;

#if defined(_UNICODE) || defined(OLE2ANSI)
bstr = SysAllocString(lpszSrc);
#else
nLen = MultiByteToWideChar(CP_ACP, 0, lpszSrc, -1, NULL, 0);
if( bstr = SysAllocStringLen(NULL, nLen) )
   MultiByteToWideChar(CP_ACP, 0, lpszSrc, -1, bstr, nLen);
#endif
return bstr;
}

//--------------------------------------------------------------------

BOOL VariantSetVal(LPVARIANT lpVar, LPCALC_VALUE lpValue)
{
  LPTSTR lpszText;
  BOOL bRet = FALSE;

  VariantClear(lpVar);
  if( CALC_VALUE_STATUS_OK == lpValue->Status )
  {
    switch( lpValue->Type )
    {
      case CALC_VALUE_TYPE_LONG:
        V_VT(lpVar) = VT_I4;
        V_I4(lpVar) = ValGetLong(lpValue);
        bRet = TRUE;
        break;
      case CALC_VALUE_TYPE_DOUBLE:
        V_VT(lpVar) = VT_R8;
        V_R8(lpVar) = ValGetDouble(lpValue);
        bRet = TRUE;
        break;
      case CALC_VALUE_TYPE_STR:
        lpszText = GlobalLock(lpValue->Val.hValStr);
        V_VT(lpVar) = VT_BSTR;
        V_BSTR(lpVar) = BstrFromTstr(lpszText);
        GlobalUnlock(lpValue->Val.hValStr);
        break;
    }
  }
  return TRUE;  
}

//--------------------------------------------------------------------

BOOL ValSetVariant(LPCALC_VALUE lpValue, LPVARIANT lpVar)
{
  switch( lpVar->vt )
  {
    case VT_UI2:
      ValSetLong(lpValue, lpVar->bVal);
      break;
    case VT_I2:
      ValSetLong(lpValue, lpVar->iVal);
      break;
    case VT_I4:
      ValSetLong(lpValue, lpVar->lVal);
      break;
    case VT_R4:
      ValSetDouble(lpValue, lpVar->fltVal);
      break;
    case VT_R8:
      ValSetDouble(lpValue, lpVar->dblVal);
      break;
    case VT_BOOL:
      ValSetBool(lpValue, lpVar->boolVal);
      break;
    case VT_BSTR:
      ValSetStringW(lpValue, lpVar->bstrVal);
      break;
    default:
      ValSetEmpty(lpValue);
      break;
  }
  return TRUE;  
}

//--------------------------------------------------------------------

BOOL DdeGetValues(CALC_HANDLE hDde, LPVARIANT lpVar)
{
  LPCALC_DDE lpDde;
  LPCALC_VALUE lpValue;
  long lNumValues;
  long i;
  BOOL bRet = FALSE;

  VariantClear(lpVar);
  if( lpDde = (LPCALC_DDE)CalcMemLock(hDde) )
  {
    if( lpDde->hValues && (lpValue = (LPCALC_VALUE)CalcMemLock(lpDde->hValues)) )
    {
      lNumValues = lpDde->lValueCols * lpDde->lValueRows;
      if( 1 == lNumValues )
      {
        VariantSetVal(lpVar, lpValue);
      }
      else if( lNumValues > 1 )
      {
        SAFEARRAYBOUND sabound[2];
        LPSAFEARRAY lpsa;
        LPVARIANT lpvarData;
        sabound[0].lLbound = 0;
        sabound[0].cElements = lpDde->lValueCols;
        sabound[1].lLbound = 0;
        sabound[1].cElements = lpDde->lValueRows;
        lpsa = SafeArrayCreate(VT_VARIANT, 2, sabound);
        SafeArrayAccessData(lpsa, &lpvarData);
        for( i = 0; i < lNumValues; i++ )
          VariantSetVal(&lpvarData[i], &lpValue[i]);
        SafeArrayUnaccessData(lpsa);
        V_VT(lpVar) = VT_ARRAY | VT_VARIANT;
        V_ARRAY(lpVar) = lpsa;
      }
      CalcMemUnlock(lpDde->hValues);
    }
    CalcMemUnlock(hDde);
  }
  return bRet;
}

//--------------------------------------------------------------------
//
//  The DdeSetValue() function saves the most recently received
//  value of the DDE link.
//

BOOL DdeSetValues(CALC_HANDLE hDde, long lCols, long lRows, LPVARIANT lpVar)
{
  LPCALC_DDE lpDde;
  LPCALC_VALUE lpDdeVal;
  long lNumValues = lCols * lRows;
  long lNumOldValues;
  int i;
  BOOL bFreeValues = TRUE;
  BOOL bRet = FALSE;

  if( hDde && (lpDde = (LPCALC_DDE)CalcMemLock(hDde)) )
  {
    if( lpDde->lValueCols != lCols || lpDde->lValueRows != lRows )
    {
      if( lpDde->hValues && (lpDdeVal = (LPCALC_VALUE)CalcMemLock(lpDde->hValues)) )
      {
        lNumOldValues = lpDde->lValueCols * lpDde->lValueRows;
        for( i = 0; i < lNumOldValues; i++, lpDdeVal++ )
          ValFree(lpDdeVal);
        CalcMemUnlock(lpDde->hValues);
        CalcMemFree(lpDde->hValues);
      }
      if( lpDde->hValues = CalcMemAlloc(lNumValues * sizeof(CALC_VALUE)) )
      {
        lpDde->lValueCols = lCols;
        lpDde->lValueRows = lRows;
        bFreeValues = FALSE;
      }
      else
      {
        lpDde->lValueCols = 0;
        lpDde->lValueRows = 0;
      }
    }
    if( lpDde->hValues && (lpDdeVal = (LPCALC_VALUE)CalcMemLock(lpDde->hValues)) )
    {
      for( i = 0; i < lNumValues; i++, lpDdeVal++, lpVar++ )
      {
        if( bFreeValues )
          ValFree(lpDdeVal);
        ValSetVariant(lpDdeVal, lpVar);
      }
      CalcMemUnlock(lpDde->hValues);
    }
    CalcMemUnlock(hDde);
  }
  return bRet;
}

//--------------------------------------------------------------------
//
//  The DdeMaxIndex() function returns the number of data values
//  in the DDE link.
//

long DdeMaxIndex(CALC_HANDLE hDde)
{
  LPCALC_DDE lpDde;
  long lRet = 0;

  if( hDde && (lpDde = (LPCALC_DDE)CalcMemLock(hDde)) )
  {
    lRet = lpDde->lValueCols * lpDde->lValueRows;
    CalcMemUnlock(hDde);
  }
  return lRet;
}

//--------------------------------------------------------------------
//
//  The DdeMaxCol() function returns the number of columns of data
//  in the DDE link.
//

long DdeMaxCol(CALC_HANDLE hDde)
{
  LPCALC_DDE lpDde;
  long lRet = 0;

  if( hDde && (lpDde = (LPCALC_DDE)CalcMemLock(hDde)) )
  {
    lRet = lpDde->lValueCols;
    CalcMemUnlock(hDde);
  }
  return lRet;
}

//--------------------------------------------------------------------
//
//  The DdeMaxRow() function returns the number of rows of data
//  in the DDE link.
//

long DdeMaxRow(CALC_HANDLE hDde)
{
  LPCALC_DDE lpDde;
  long lRet = 0;

  if( hDde && (lpDde = (LPCALC_DDE)CalcMemLock(hDde)) )
  {
    lRet = lpDde->lValueRows;
    CalcMemUnlock(hDde);
  }
  return lRet;
}

//--------------------------------------------------------------------
//
//  The DdeGetText() function retrieves the text representation of
//  the DDE link.  If successful, the function returns the length of
//  the copied text, not including the terminating NULL character.
//  Otherwise, the function returns zero.
//

int DdeGetText(CALC_HANDLE hDde, long lCol, long lRow, LPTSTR lpszText, int nLen)
{
  LPCALC_DDE lpDde;
  LPTSTR lpszServer;
  LPTSTR lpszTopic;
  LPTSTR lpszItem;
  TCHAR szCol[16];
  TCHAR szRow[16];
  int nFuncLen;
  int nServerLen;
  int nTopicLen;
  int nItemLen;
  int nColLen;
  int nRowLen;
  int nTextLen;
  int nResult = 0;

  if( hDde && (lpDde = (LPCALC_DDE)CalcMemLock(hDde)) )
  {
    if( lpDde->hServer && (lpszServer = (LPTSTR)CalcMemLock(lpDde->hServer)) &&
        lpDde->hTopic && (lpszTopic = (LPTSTR)CalcMemLock(lpDde->hTopic)) &&
        lpDde->hItem && (lpszItem = (LPTSTR)CalcMemLock(lpDde->hItem)) )
    {
      wsprintf(szCol, -1 == lCol ? _T("") : _T(",%ld"), lCol);
      wsprintf(szRow, -1 == lRow ? _T("") : _T(",%ld"), lRow);
      nFuncLen = FuncGetTextLen(CALC_FUNC_DDELINK);
      nServerLen = lstrlen(lpszServer);
      nTopicLen = lstrlen(lpszTopic);
      nItemLen = lstrlen(lpszItem);
      nColLen = lstrlen(szCol);
      nRowLen = lstrlen(szRow);
      nTextLen = nFuncLen + nServerLen + nTopicLen + nItemLen
                 + nColLen + nRowLen + 10;
      if( nTextLen < nLen )
      {
        FuncGetText(CALC_FUNC_DDELINK, lpszText, nLen);
        lstrcat(lpszText, _T("(\""));
        lstrcat(lpszText, lpszServer);
        lstrcat(lpszText, _T("\",\""));
        lstrcat(lpszText, lpszTopic);
        lstrcat(lpszText, _T("\",\""));
        lstrcat(lpszText, lpszItem);
        lstrcat(lpszText, _T("\""));
        lstrcat(lpszText, szCol);
        lstrcat(lpszText, szRow);
        lstrcat(lpszText, _T(")"));
        nResult = nTextLen;
      }
      CalcMemUnlock(lpDde->hServer);
      CalcMemUnlock(lpDde->hTopic);
      CalcMemUnlock(lpDde->hItem);
    }
    CalcMemUnlock(hDde);
  }
  if( 0 == nResult && nLen > 0 )
    lpszText[0] = 0;
  return nResult;
}

//--------------------------------------------------------------------
//
//  The DdeGetTextLen() function retrieves the length of the text
//  representation of the DDE link.
//

int DdeGetTextLen(CALC_HANDLE hDde, long lCol, long lRow)
{
  LPCALC_DDE lpDde;
  LPTSTR lpszServer;
  LPTSTR lpszTopic;
  LPTSTR lpszItem;
  TCHAR szCol[16];
  TCHAR szRow[16];
  int nFuncLen;
  int nServerLen;
  int nTopicLen;
  int nItemLen;
  int nColLen;
  int nRowLen;
  int nResult = 0;

  if( hDde && (lpDde = (LPCALC_DDE)CalcMemLock(hDde)) )
  {
    if( lpDde->hServer && (lpszServer = (LPTSTR)CalcMemLock(lpDde->hServer)) &&
        lpDde->hTopic && (lpszTopic = (LPTSTR)CalcMemLock(lpDde->hTopic)) &&
        lpDde->hItem && (lpszItem = (LPTSTR)CalcMemLock(lpDde->hItem)) )
    {
      wsprintf(szCol, -1 == lCol ? _T("") : _T(",%ld"), lCol);
      wsprintf(szRow, -1 == lRow ? _T("") : _T(",%ld"), lRow);
      nFuncLen = FuncGetTextLen(CALC_FUNC_DDELINK);
      nServerLen = lstrlen(lpszServer);
      nTopicLen = lstrlen(lpszTopic);
      nItemLen = lstrlen(lpszItem);
      nColLen = lstrlen(szCol);
      nRowLen = lstrlen(szRow);
      nResult = nFuncLen + nServerLen + nTopicLen + nItemLen +
                nColLen + nRowLen + 10;
      CalcMemUnlock(lpDde->hServer);
      CalcMemUnlock(lpDde->hTopic);
      CalcMemUnlock(lpDde->hItem);
    }
    CalcMemUnlock(hDde);
  }
  return nResult;
}

//--------------------------------------------------------------------
//
//  The DdeGetServer() function retrieves the text representation of
//  the server.  If successful, the function returns the length of
//  the copied text, not including the terminating NULL character.
//  Otherwise, the function returns zero.
//

int DdeGetServer(CALC_HANDLE hDde, LPTSTR lpszText, int nLen)
{
  LPCALC_DDE lpDde;
  LPTSTR lpszServer;
  int nServerLen;
  int nResult = 0;

  if( hDde && (lpDde = (LPCALC_DDE)CalcMemLock(hDde)) )
  {
    if( lpDde->hServer && (lpszServer = (LPTSTR)CalcMemLock(lpDde->hServer)) )
    {
      nServerLen = lstrlen(lpszServer);
      if( nServerLen < nLen )
      {
        lstrcpy(lpszText, lpszServer);
        nResult = nServerLen;
      }
      CalcMemUnlock(lpDde->hServer);
    }
    CalcMemUnlock(hDde);
  }
  if( 0 == nResult && nLen > 0 )
    lpszText[0] = 0;
  return nResult;
}

//--------------------------------------------------------------------
//
//  The DdeGetServerLen() function retrieves the length of the text
//  representation of the server.
//

int DdeGetServerLen(CALC_HANDLE hDde)
{
  LPCALC_DDE lpDde;
  LPTSTR lpszServer;
  int nResult = 0;

  if( hDde && (lpDde = (LPCALC_DDE)CalcMemLock(hDde)) )
  {
    if( lpDde->hServer && (lpszServer = (LPTSTR)CalcMemLock(lpDde->hServer)) )
    {
      nResult = lstrlen(lpszServer);
      CalcMemUnlock(lpDde->hServer);
    }
    CalcMemUnlock(hDde);
  }
  return nResult;
}

//--------------------------------------------------------------------
//
//  The DdeGetTopic() function retrieves the text representation of
//  the topic.  If successful, the function returns the length of
//  the copied text, not including the terminating NULL character.
//  Otherwise, the function returns zero.
//

int DdeGetTopic(CALC_HANDLE hDde, LPTSTR lpszText, int nLen)
{
  LPCALC_DDE lpDde;
  LPTSTR lpszTopic;
  int nTopicLen;
  int nResult = 0;

  if( hDde && (lpDde = (LPCALC_DDE)CalcMemLock(hDde)) )
  {
    if( lpDde->hTopic && (lpszTopic = (LPTSTR)CalcMemLock(lpDde->hTopic)) )
    {
      nTopicLen = lstrlen(lpszTopic);
      if( nTopicLen < nLen )
      {
        lstrcpy(lpszText, lpszTopic);
        nResult = nTopicLen;
      }
      CalcMemUnlock(lpDde->hTopic);
    }
    CalcMemUnlock(hDde);
  }
  if( 0 == nResult && nLen > 0 )
    lpszText[0] = 0;
  return nResult;
}

//--------------------------------------------------------------------
//
//  The DdeGetTopicLen() function retrieves the length of the text
//  representation of the topic.
//

int DdeGetTopicLen(CALC_HANDLE hDde)
{
  LPCALC_DDE lpDde;
  LPTSTR lpszTopic;
  int nResult = 0;

  if( hDde && (lpDde = (LPCALC_DDE)CalcMemLock(hDde)) )
  {
    if( lpDde->hTopic && (lpszTopic = (LPTSTR)CalcMemLock(lpDde->hTopic)) )
    {
      nResult = lstrlen(lpszTopic);
      CalcMemUnlock(lpDde->hTopic);
    }
    CalcMemUnlock(hDde);
  }
  return nResult;
}

//--------------------------------------------------------------------
//
//  The DdeGetItem() function retrieves the text representation of
//  the item.  If successful, the function returns the length of
//  the copied text, not including the terminating NULL character.
//  Otherwise, the function returns zero.
//

int DdeGetItem(CALC_HANDLE hDde, LPTSTR lpszText, int nLen)
{
  LPCALC_DDE lpDde;
  LPTSTR lpszItem;
  int nItemLen;
  int nResult = 0;

  if( hDde && (lpDde = (LPCALC_DDE)CalcMemLock(hDde)) )
  {
    if( lpDde->hItem && (lpszItem = (LPTSTR)CalcMemLock(lpDde->hItem)) )
    {
      nItemLen = lstrlen(lpszItem);
      if( nItemLen < nLen )
      {
        lstrcpy(lpszText, lpszItem);
        nResult = nItemLen;
      }
      CalcMemUnlock(lpDde->hItem);
    }
    CalcMemUnlock(hDde);
  }
  if( 0 == nResult && nLen > 0 )
    lpszText[0] = 0;
  return nResult;
}

//--------------------------------------------------------------------
//
//  The DdeGetItemLen() function retrieves the length of the text
//  representation of the item.
//

int DdeGetItemLen(CALC_HANDLE hDde)
{
  LPCALC_DDE lpDde;
  LPTSTR lpszItem;
  int nResult = 0;

  if( hDde && (lpDde = (LPCALC_DDE)CalcMemLock(hDde)) )
  {
    if( lpDde->hItem && (lpszItem = (LPTSTR)CalcMemLock(lpDde->hItem)) )
    {
      nResult = lstrlen(lpszItem);
      CalcMemUnlock(lpDde->hItem);
    }
    CalcMemUnlock(hDde);
  }
  return nResult;
}

//--------------------------------------------------------------------

long DdeGetRefCnt(CALC_HANDLE hDde)
{
  LPCALC_DDE lpDde;
  long lRet = 0;

  if( hDde && (lpDde = (LPCALC_DDE)CalcMemLock(hDde)) )
  {
    lRet = lpDde->lRefCnt;
    CalcMemUnlock(hDde);
  }
  return lRet;
}

//--------------------------------------------------------------------

int DdeGetReadExpr(CALC_HANDLE hDde, LPTSTR lpszText, int nLen)
{
  LPCALC_DDE lpDde;
  LPTSTR lpszReadExpr;
  int nReadExprLen;
  int nResult = 0;

  if( hDde && (lpDde = (LPCALC_DDE)CalcMemLock(hDde)) )
  {
    if( lpDde->hReadExpr && (lpszReadExpr = (LPTSTR)CalcMemLock(lpDde->hReadExpr)) )
    {
      nReadExprLen = lstrlen(lpszReadExpr);
      if( nReadExprLen < nLen )
      {
        lstrcpy(lpszText, lpszReadExpr);
        nResult = nReadExprLen;
      }
      CalcMemUnlock(lpDde->hReadExpr);
    }
    CalcMemUnlock(hDde);
  }
  if( 0 == nResult && nLen > 0 )
    lpszText[0] = 0;
  return nResult;
}

//--------------------------------------------------------------------

int DdeGetReadExprLen(CALC_HANDLE hDde)
{
  LPCALC_DDE lpDde;
  LPTSTR lpszReadExpr;
  int nResult = 0;

  if( hDde && (lpDde = (LPCALC_DDE)CalcMemLock(hDde)) )
  {
    if( lpDde->hReadExpr && (lpszReadExpr = (LPTSTR)CalcMemLock(lpDde->hReadExpr)) )
    {
      nResult = lstrlen(lpszReadExpr);
      CalcMemUnlock(lpDde->hReadExpr);
    }
    CalcMemUnlock(hDde);
  }
  return nResult;
}

//--------------------------------------------------------------------

BOOL DdeSetReadExpr(CALC_HANDLE hDde, LPCTSTR lpszText)
{
  LPCALC_DDE lpDde;
  CALC_HANDLE hReadExpr;
  LPTSTR lpszReadExpr;
  BOOL bRet = FALSE;

  if( hDde && (lpDde = (LPCALC_DDE)CalcMemLock(hDde)) )
  {
    if( lpDde->hReadExpr )
      CalcMemFree(lpDde->hReadExpr);
    lpDde->hReadExpr = 0;
    if( lpszText && lpszText[0] )
    {
      hReadExpr = CalcMemAlloc((lstrlen(lpszText)+1) * sizeof(TCHAR));
      if( hReadExpr && (lpszReadExpr = (LPTSTR)CalcMemLock(hReadExpr)) )
      {
        lstrcpy(lpszReadExpr, lpszText);
        lpDde->hReadExpr = hReadExpr;
        DispSetPropString(lpDde->lpdispLink, OLESTR("CalcReadString"), lpszReadExpr);
        bRet = TRUE;
        CalcMemUnlock(hReadExpr);
      }
    }
    else
      bRet = TRUE;
    CalcMemUnlock(hDde);
  }
  return bRet;
}

//--------------------------------------------------------------------

int DdeGetWriteExpr(CALC_HANDLE hDde, LPTSTR lpszText, int nLen)
{
  LPCALC_DDE lpDde;
  LPTSTR lpszWriteExpr;
  int nWriteExprLen;
  int nResult = 0;

  if( hDde && (lpDde = (LPCALC_DDE)CalcMemLock(hDde)) )
  {
    if( lpDde->hWriteExpr && (lpszWriteExpr = (LPTSTR)CalcMemLock(lpDde->hWriteExpr)) )
    {
      nWriteExprLen = lstrlen(lpszWriteExpr);
      if( nWriteExprLen < nLen )
      {
        lstrcpy(lpszText, lpszWriteExpr);
        nResult = nWriteExprLen;
      }
      CalcMemUnlock(lpDde->hWriteExpr);
    }
    CalcMemUnlock(hDde);
  }
  if( 0 == nResult && nLen > 0 )
    lpszText[0] = 0;
  return nResult;
}

//--------------------------------------------------------------------

int DdeGetWriteExprLen(CALC_HANDLE hDde)
{
  LPCALC_DDE lpDde;
  LPTSTR lpszWriteExpr;
  int nResult = 0;

  if( hDde && (lpDde = (LPCALC_DDE)CalcMemLock(hDde)) )
  {
    if( lpDde->hWriteExpr && (lpszWriteExpr = (LPTSTR)CalcMemLock(lpDde->hWriteExpr)) )
    {
      nResult = lstrlen(lpszWriteExpr);
      CalcMemUnlock(lpDde->hWriteExpr);
    }
    CalcMemUnlock(hDde);
  }
  return nResult;
}

//--------------------------------------------------------------------

BOOL DdeSetWriteExpr(CALC_HANDLE hDde, LPCTSTR lpszText)
{
  LPCALC_DDE lpDde;
  CALC_HANDLE hWriteExpr;
  LPTSTR lpszWriteExpr;
  BOOL bRet = FALSE;

  if( hDde && (lpDde = (LPCALC_DDE)CalcMemLock(hDde)) )
  {
    if( lpDde->hWriteExpr )
      CalcMemFree(lpDde->hWriteExpr);
    lpDde->hWriteExpr = 0;
    if( lpszText && lpszText[0] )
    {
      hWriteExpr = CalcMemAlloc((lstrlen(lpszText)+1) * sizeof(TCHAR));
      if( hWriteExpr && (lpszWriteExpr = (LPTSTR)CalcMemLock(hWriteExpr)) )
      {
        lstrcpy(lpszWriteExpr, lpszText);
        lpDde->hWriteExpr = hWriteExpr;
        DispSetPropString(lpDde->lpdispLink, OLESTR("CalcWriteString"), lpszWriteExpr);
        bRet = TRUE;
        CalcMemUnlock(hWriteExpr);
      }
    }
    else
      bRet = TRUE;
    CalcMemUnlock(hDde);
  }
  return bRet;
}

//--------------------------------------------------------------------

short DdeGetMode(CALC_HANDLE hDde)
{
  LPCALC_DDE lpDde;
  short nRet = CALC_LINKMODE_NONE;

  if( hDde && (lpDde = (LPCALC_DDE)CalcMemLock(hDde)) )
  {
    short nTemp;
    DispGetPropShort(lpDde->lpdispLink, OLESTR("LinkMode"), &nTemp);
    nRet = lpDde->nMode;
    CalcMemUnlock(hDde);
  }
  return nRet;
}

//--------------------------------------------------------------------

short DdeSetMode(CALC_HANDLE hDde, short nMode)
{
  LPCALC_DDE lpDde;
  short nRet = CALC_LINKMODE_NONE;

  if( hDde && (lpDde = (LPCALC_DDE)CalcMemLock(hDde)) )
  {
    nRet = lpDde->nMode;
    lpDde->nMode = nMode;
    if( !SS_CalcIsDesignTime(lpDde->hSS) )
      DispSetPropShort(lpDde->lpdispLink, OLESTR("LinkMode"), nMode);
    CalcMemUnlock(hDde);
  }
  return nRet;
}

//--------------------------------------------------------------------

BOOL DdeRequest(CALC_HANDLE hDde)
{
  LPCALC_DDE lpDde;
  BOOL bRet = FALSE;

  if( hDde && (lpDde = (LPCALC_DDE)CalcMemLock(hDde)) )
  {
    DispCallMethod(lpDde->lpdispLink, OLESTR("LinkRequest"));
    CalcMemUnlock(hDde);
  }
  return bRet;
}

//--------------------------------------------------------------------

BOOL DdePoke(CALC_HANDLE hDde)
{
  LPCALC_DDE lpDde;
  VARIANT var;
  BOOL bRet = FALSE;

  if( hDde && (lpDde = (LPCALC_DDE)CalcMemLock(hDde)) )
  {
    VariantInit(&var);
    DdeGetValues(hDde, &var);
    DispSetPropVariant(lpDde->lpdispLink, OLESTR("Value"), &var);
    DispCallMethod(lpDde->lpdispLink, OLESTR("LinkPoke"));
    VariantClear(&var);
    CalcMemUnlock(hDde);
  }
  return bRet;
}

//--------------------------------------------------------------------
//
//  The DdeSetLinkToCell() function adds or removes a data link from
//  the DDE object to the cell (i.e. the cell is dependent on the
//  DDE object).  This data link allows updates to the DDE object
//  to be reflected in the cell.
//

BOOL DdeSetLinkToCell(CALC_HANDLE hDde, long lCol, long lRow, BOOL bLink)
{
  LPCALC_DDE lpDde;
  BOOL bReturn = FALSE;

  if( hDde && (lpDde = (LPCALC_DDE)CalcMemLock(hDde)) )
  {
    BitMatSet(&lpDde->DependCells, lCol, lRow, bLink);
    bReturn = TRUE;
    CalcMemUnlock(hDde);
  }
  return bReturn;
}

//--------------------------------------------------------------------
//
//  The DdeSetLinkToCol() function adds or removes a data link from
//  the DDE object to the column (i.e. the column is dependent on the
//  DDE object).  This data link allows updates to the DDE object
//  to be reflected in the column.
//

BOOL DdeSetLinkToCol(CALC_HANDLE hDde, long lCol, BOOL bLink)
{
  LPCALC_DDE lpDde;
  BOOL bReturn = FALSE;

  if( hDde && (lpDde = (LPCALC_DDE)CalcMemLock(hDde)) )
  {
    BitVectSet(&lpDde->DependCols, lCol, bLink);
    bReturn = TRUE;
    CalcMemUnlock(hDde);
  }
  return bReturn;
}

//--------------------------------------------------------------------
//
//  The DdeSetLinkToRow() function adds or removes a data link from
//  the DDE object to the row (i.e. the row is dependent on the
//  DDE object).  This data link allows updates to the DDE object
//  to be reflected in the row.
//

BOOL DdeSetLinkToRow(CALC_HANDLE hDde, long lRow, BOOL bLink)
{
  LPCALC_DDE lpDde;
  BOOL bReturn = FALSE;

  if( hDde && (lpDde = (LPCALC_DDE)CalcMemLock(hDde)) )
  {
    BitVectSet(&lpDde->DependRows, lRow, bLink);
    bReturn = TRUE;
    CalcMemUnlock(hDde);
  }
  return bReturn;
}

//--------------------------------------------------------------------
//
//  The DdeSetLinkToName() function adds or removes a data link from
//  the DDE object to the custom name (i.e. the row is dependent on
//  the DDE object).  This data link allows updates to the DDE object
//  to be reflected in the custom name.
//

BOOL DdeSetLinkToName(CALC_HANDLE hDde, CALC_HANDLE hDepend, BOOL bLink)
{
  LPCALC_DDE lpDde;
  BOOL bReturn = FALSE;

  if( hDde && (lpDde = (LPCALC_DDE)CalcMemLock(hDde)) )
  {
    if( bLink )
      bReturn = NameAdd(&lpDde->DependNames, hDepend);
    else
      bReturn = NameRemove(&lpDde->DependNames, hDepend);
    CalcMemUnlock(hDde);
  }
  return bReturn;
}

//--------------------------------------------------------------------
//
//  The DdeMarkDepend() function records (in lpDependCells,
//  lpDependCols, and lpDependRows) all cells, columns, and rows
//  which depend (directly or indirectly) on the given DDE object
//  for data.
//

BOOL DdeMarkDepend(CALC_HANDLE hDde, LPBITMATRIX lpDependCells,
                   LPBITVECTOR lpDependCols, LPBITVECTOR lpDependRows)
{
  LPCALC_DDE lpDde;
  LPCALC_HANDLE lpElem;
  BOOL bReturn = TRUE;
  int i;

  if( hDde && (lpDde = (LPCALC_DDE)CalcMemLock(hDde)) )
  {
    BitMatCopy(&lpDde->DependCells, lpDependCells);
    BitVectCopy(&lpDde->DependCols, lpDependCols);
    BitVectCopy(&lpDde->DependRows, lpDependRows);
    if( lpDde->DependNames.hElem &&
        (lpElem = (LPCALC_HANDLE)CalcMemLock(lpDde->DependNames.hElem)) )
    {
      for( i = 0; i < lpDde->DependNames.nElemCnt; i++ )
        NameMarkDepend(lpElem[i], lpDependCells, lpDependCols, lpDependRows);
      CalcMemUnlock(lpDde->DependNames.hElem);
    }
    bReturn = TRUE;
    CalcMemUnlock(hDde);
  }
  return bReturn;
}

//--------------------------------------------------------------------


BOOL DdeUpdate(CALC_HANDLE hDde)
{
  LPCALC_DDE lpDde;
  BOOL bReturn = FALSE;

  if( hDde && (lpDde = (LPCALC_DDE)CalcMemLock(hDde)) )
  {
    lpDde->lpfnDdeUpdate(lpDde->hSS, hDde);
    bReturn = TRUE;
    CalcMemUnlock(hDde);
  }
  return bReturn;
}

//--------------------------------------------------------------------

BOOL DdeFirstDependCell(CALC_HANDLE hDde, LPLONG lplCol, LPLONG lplRow)
{
  LPCALC_DDE lpDde;
  BOOL bRet = FALSE;

  if( hDde && (lpDde = (LPCALC_DDE)CalcMemLock(hDde)) )
  {
    bRet = BitMatGetFirst(&lpDde->DependCells, lplCol, lplRow);
    CalcMemUnlock(hDde);
  }
  return bRet;
}

//--------------------------------------------------------------------

BOOL DdeNextDependCell(CALC_HANDLE hDde, long lColPrev, long lRowPrev, LPLONG lplCol, LPLONG lplRow)
{
  LPCALC_DDE lpDde;
  BOOL bRet = FALSE;

  if( hDde && (lpDde = (LPCALC_DDE)CalcMemLock(hDde)) )
  {
    *lplCol = lColPrev;
    *lplRow = lRowPrev;
    bRet = BitMatGetNext(&lpDde->DependCells, lplCol, lplRow);
    CalcMemUnlock(hDde);
  }
  return bRet;
}

//--------------------------------------------------------------------

BOOL DdeFirstDependCol(CALC_HANDLE hDde, LPLONG lplCol)
{
  LPCALC_DDE lpDde;
  BOOL bRet = FALSE;

  if( hDde && (lpDde = (LPCALC_DDE)CalcMemLock(hDde)) )
  {
    bRet = BitVectGetFirst(&lpDde->DependCols, lplCol);
    CalcMemUnlock(hDde);
  }
  return bRet;
}

//--------------------------------------------------------------------

BOOL DdeNextDependCol(CALC_HANDLE hDde, long lColPrev, LPLONG lplCol)
{
  LPCALC_DDE lpDde;
  BOOL bRet = FALSE;

  if( hDde && (lpDde = (LPCALC_DDE)CalcMemLock(hDde)) )
  {
    *lplCol = lColPrev;
    bRet = BitVectGetNext(&lpDde->DependCols, lplCol);
    CalcMemUnlock(hDde);
  }
  return bRet;
}

//--------------------------------------------------------------------

BOOL DdeFirstDependRow(CALC_HANDLE hDde, LPLONG lplRow)
{
  LPCALC_DDE lpDde;
  BOOL bRet = FALSE;

  if( hDde && (lpDde = (LPCALC_DDE)CalcMemLock(hDde)) )
  {
    bRet = BitVectGetFirst(&lpDde->DependRows, lplRow);
    CalcMemUnlock(hDde);
  }
  return bRet;
}

//--------------------------------------------------------------------

BOOL DdeNextDependRow(CALC_HANDLE hDde, long lRowPrev, LPLONG lplRow)
{
  LPCALC_DDE lpDde;
  BOOL bRet = FALSE;

  if( hDde && (lpDde = (LPCALC_DDE)CalcMemLock(hDde)) )
  {
    *lplRow = lRowPrev;
    bRet = BitVectGetNext(&lpDde->DependRows, lplRow);
    CalcMemUnlock(hDde);
  }
  return bRet;
}

//--------------------------------------------------------------------

CALC_HANDLE DdeFirstDependName(CALC_HANDLE hDde)
{
  LPCALC_DDE lpDde;
  CALC_HANDLE hRet = NULL;

  if( hDde && (lpDde = (LPCALC_DDE)CalcMemLock(hDde)) )
  {
    hRet = NameFirst(&lpDde->DependNames);
    CalcMemUnlock(hDde);
  }
  return hRet;
}

//--------------------------------------------------------------------

CALC_HANDLE DdeNextDependName(CALC_HANDLE hDde, LPCTSTR lpszNamePrev)
{
  LPCALC_DDE lpDde;
  CALC_HANDLE hRet = NULL;

  if( hDde && (lpDde = (LPCALC_DDE)CalcMemLock(hDde)) )
  {
    hRet = NameNext(&lpDde->DependNames, lpszNamePrev);
    CalcMemUnlock(hDde);
  }
  return hRet;
}

//--------------------------------------------------------------------

void DdeSetDesignTime(CALC_HANDLE hDde, BOOL bDesignTime)
{
  LPCALC_DDE lpDde;
  short nMode;

  if( hDde && (lpDde = (LPCALC_DDE)CalcMemLock(hDde)) )
  {
    nMode = bDesignTime ? CALC_LINKMODE_NONE : lpDde->nMode;
    DispSetPropShort(lpDde->lpdispLink, OLESTR("LinkMode"), nMode);
  }
}

//--------------------------------------------------------------------

HRESULT DispCallMethod(LPDISPATCH lpdisp, LPOLESTR lpszMeth)
{
  HRESULT hr = DISP_E_BADCALLEE;
  REFIID riid = &IID_NULL;
  LCID lcid = LOCALE_SYSTEM_DEFAULT;
  DISPID dispid;
  DISPPARAMS dispparam;
  EXCEPINFO excepinfo;
  VARIANT varResult;

  if( lpdisp )
  {
    VariantInit(&varResult);
    dispparam.rgvarg = NULL;
    dispparam.rgdispidNamedArgs = NULL;
    dispparam.cArgs = 0;
    dispparam.cNamedArgs = 0;
    memset(&excepinfo, 0, sizeof(excepinfo));
    hr = lpdisp->lpVtbl->GetIDsOfNames(lpdisp, riid, &lpszMeth,
                                       1, lcid, &dispid);
    if( SUCCEEDED(hr) )
    {
      hr = lpdisp->lpVtbl->Invoke(lpdisp, dispid, riid, lcid,
                                  DISPATCH_METHOD, &dispparam,
                                  &varResult, &excepinfo, NULL);
    }
    VariantClear(&varResult);
  }
  return hr;
}

//--------------------------------------------------------------------

HRESULT DispGetPropVariant(LPDISPATCH lpdisp, LPOLESTR lpszProp, LPVARIANT lpvar)
{
  HRESULT hr = DISP_E_BADCALLEE;
  REFIID riid = &IID_NULL;
  LCID lcid = LOCALE_SYSTEM_DEFAULT;
  DISPID dispid;
  DISPPARAMS dispparam;
  EXCEPINFO excepinfo;

  if( lpdisp )
  {
    dispparam.rgvarg = NULL;
    dispparam.rgdispidNamedArgs = NULL;
    dispparam.cArgs = 0;
    dispparam.cNamedArgs = 0;
    memset(&excepinfo, 0, sizeof(excepinfo));
    hr = lpdisp->lpVtbl->GetIDsOfNames(lpdisp, riid, &lpszProp,
                                       1, lcid, &dispid);
    if( SUCCEEDED(hr) )
    {
      hr = lpdisp->lpVtbl->Invoke(lpdisp, dispid, riid, lcid,
                                  DISPATCH_PROPERTYGET, &dispparam,
                                  lpvar, &excepinfo, NULL);
    }
  }
  return hr;
}

//--------------------------------------------------------------------

HRESULT DispGetPropBool(LPDISPATCH lpdisp, LPOLESTR lpszProp, BOOL FAR* lpbVal)
{
  HRESULT hr;
  VARIANT var;

  VariantInit(&var);
  hr = DispGetPropVariant(lpdisp, lpszProp, &var);
  if( VT_BOOL == V_VT(&var) && V_BOOL(&var) )
    *lpbVal = TRUE;
  else
    *lpbVal = FALSE;
  VariantClear(&var);
  return hr;
}

//--------------------------------------------------------------------

HRESULT DispGetPropShort(LPDISPATCH lpdisp, LPOLESTR lpszProp, short FAR* lpnVal)
{
  HRESULT hr;
  VARIANT var;

  VariantInit(&var);
  hr = DispGetPropVariant(lpdisp, lpszProp, &var);
  if( VT_I2 == V_VT(&var) )
    *lpnVal = V_I2(&var);
  else
    *lpnVal = 0;
  VariantClear(&var);
  return hr;
}

//--------------------------------------------------------------------

HRESULT DispSetPropVariant(LPDISPATCH lpdisp, LPOLESTR lpszProp, LPVARIANT lpVar)
{
  HRESULT hr = DISP_E_PARAMNOTOPTIONAL;
  REFIID riid = &IID_NULL;
  LCID lcid = LOCALE_SYSTEM_DEFAULT;
  DISPID dispid;
  DISPPARAMS dispparam;
  EXCEPINFO excepinfo;
  UINT uArgErr;
  DISPID dispidNamedArgs[1] = {DISPID_PROPERTYPUT};

  if( lpdisp )
  {
    dispparam.rgvarg = lpVar;
    dispparam.rgdispidNamedArgs = dispidNamedArgs;
    dispparam.cArgs = 1;
    dispparam.cNamedArgs = 1;
    memset(&excepinfo, 0, sizeof(excepinfo));
    hr = lpdisp->lpVtbl->GetIDsOfNames(lpdisp, riid, &lpszProp,
                                       1, lcid, &dispid);
    if( SUCCEEDED(hr) )
    {
      hr = lpdisp->lpVtbl->Invoke(lpdisp, dispid, riid, lcid,
                                  DISPATCH_PROPERTYPUT, &dispparam,
                                  NULL, &excepinfo, &uArgErr);
    }
  }
  return hr;
}

//--------------------------------------------------------------------

HRESULT DispSetPropString(LPDISPATCH lpdisp, LPOLESTR lpszProp, LPCTSTR lpszVal)
{
  HRESULT hr;
  VARIANT var;

  VariantInit(&var);
  V_VT(&var) = VT_BSTR;
  V_BSTR(&var) = BstrFromTstr(lpszVal);
  hr = DispSetPropVariant(lpdisp, lpszProp, &var);
  VariantClear(&var);
  return hr;
}

//--------------------------------------------------------------------

HRESULT DispSetPropShort(LPDISPATCH lpdisp, LPOLESTR lpszProp, short nVal)
{
  HRESULT hr;
  VARIANT var;

  VariantInit(&var);
  V_VT(&var) = VT_I2;
  V_I2(&var) = nVal;
  hr = DispSetPropVariant(lpdisp, lpszProp, &var);
  VariantClear(&var);
  return hr;
}

//--------------------------------------------------------------------

HRESULT DispSetPropDouble(LPDISPATCH lpdisp, LPOLESTR lpszProp, double dfVal)
{
  HRESULT hr;
  VARIANT var;

  VariantInit(&var);
  V_VT(&var) = VT_R8;
  V_R8(&var) = dfVal;
  hr = DispSetPropVariant(lpdisp, lpszProp, &var);
  VariantClear(&var);
  return hr;
}

//--------------------------------------------------------------------

typedef struct tagCalc_Sink
{
  IUnknown unk;
  IDispatch disp;
  ULONG ulRefCnt;
  CALC_HANDLE hDde;
} CALC_SINK, FAR* LPCALC_SINK;

LPCALC_SINK SinkCreate(CALC_HANDLE hDde);
void SinkNotify(LPCALC_SINK lpSink, long lNotification, LPVARIANT varData);

//--------------------------------------------------------------------

LPCALC_SINK SinkFromUnknown(LPUNKNOWN lpunk)
{
  return (LPCALC_SINK)((LPBYTE)lpunk - (int)(LPBYTE)&((LPCALC_SINK)0)->unk );
}

//--------------------------------------------------------------------

LPCALC_SINK SinkFromDispatch(LPDISPATCH lpdisp)
{
  return (LPCALC_SINK)((LPBYTE)lpdisp - (int)(LPBYTE)&((LPCALC_SINK)0)->disp );
}

//--------------------------------------------------------------------

HRESULT STDMETHODCALLTYPE Sink_Unk_QueryInterface(LPUNKNOWN lpunk, REFIID iid, void** ppvObject)
{
  LPCALC_SINK lpSink = SinkFromUnknown(lpunk);
  HRESULT hr = E_NOINTERFACE;

  if( IsEqualIID(iid, &IID_IUnknown) )
  {
    lpSink->unk.lpVtbl->AddRef(&lpSink->unk);
    *ppvObject = &lpSink->unk;
    hr = S_OK;
  }
  else if( IsEqualIID(iid, &IID_IDispatch) )
  {
    lpSink->disp.lpVtbl->AddRef(&lpSink->disp);
    *ppvObject = &lpSink->disp;
    hr = S_OK;
  }
  return hr;
}

//--------------------------------------------------------------------

ULONG STDMETHODCALLTYPE Sink_Unk_AddRef(LPUNKNOWN lpunk)
{
  LPCALC_SINK lpSink = SinkFromUnknown(lpunk);
  lpSink->ulRefCnt++;
  return lpSink->ulRefCnt;  
}

//--------------------------------------------------------------------

ULONG STDMETHODCALLTYPE Sink_Unk_Release(LPUNKNOWN lpunk)
{
  LPCALC_SINK lpSink = SinkFromUnknown(lpunk);
  ULONG ulRefCnt = --lpSink->ulRefCnt;
  if( 0 == ulRefCnt )
  {
    GlobalFree(lpSink);
  }
  return ulRefCnt;  
}

//--------------------------------------------------------------------

HRESULT STDMETHODCALLTYPE Sink_Disp_QueryInterface(LPDISPATCH lpdisp, REFIID iid, void** ppvObject)
{
  LPCALC_SINK lpSink = SinkFromDispatch(lpdisp);
  return lpSink->unk.lpVtbl->QueryInterface(&lpSink->unk, iid, ppvObject);
}

//--------------------------------------------------------------------

ULONG STDMETHODCALLTYPE Sink_Disp_AddRef(LPDISPATCH lpdisp)
{
  LPCALC_SINK lpSink = SinkFromDispatch(lpdisp);
  return lpSink->unk.lpVtbl->AddRef(&lpSink->unk);
}

//--------------------------------------------------------------------

ULONG STDMETHODCALLTYPE Sink_Disp_Release(LPDISPATCH lpdisp)
{
  LPCALC_SINK lpSink = SinkFromDispatch(lpdisp);
  return lpSink->unk.lpVtbl->Release(&lpSink->unk);
}

//--------------------------------------------------------------------

HRESULT STDMETHODCALLTYPE Sink_Disp_GetTypeInfoCount(LPDISPATCH lpdisp, UINT* ctinfo)
{
  LPCALC_SINK lpSink = SinkFromDispatch(lpdisp);
  return E_NOTIMPL;
}

//--------------------------------------------------------------------

HRESULT STDMETHODCALLTYPE Sink_Disp_GetTypeInfo(LPDISPATCH lpdisp, UINT itinfo, LCID lcid, ITypeInfo** pptinfo)
{
  LPCALC_SINK lpSink = SinkFromDispatch(lpdisp);
  return DISP_E_BADINDEX;
}

//--------------------------------------------------------------------

HRESULT STDMETHODCALLTYPE Sink_Disp_GetIDsOfNames(LPDISPATCH lpdisp, REFIID riid, OLECHAR** rgszNames, UINT cNames, LCID lcid, DISPID* rgdispid)
{
  LPCALC_SINK lpSink = SinkFromDispatch(lpdisp);
  return DISP_E_UNKNOWNNAME;
}

//--------------------------------------------------------------------
//
//  Note: pdispparams.rgvarg[] contains parameters in reverse order.
//

static HRESULT CheckLongVariant(DISPPARAMS* pdispparams)
{
  HRESULT hr;

  if( 2 == pdispparams->cArgs && 0 == pdispparams->cNamedArgs)
  {
    if( VT_I4 == V_VT(&pdispparams->rgvarg[1]) )
      hr = S_OK;
    else
      hr = DISP_E_TYPEMISMATCH;
  }
  else
    hr = DISP_E_BADPARAMCOUNT;
  return hr;
}

static HRESULT CheckShortString(DISPPARAMS* pdispparams)
{
  HRESULT hr;

  if( 2 == pdispparams->cArgs && 0 == pdispparams->cNamedArgs)
  {
    if( VT_I4 == V_VT(&pdispparams->rgvarg[1]) &&
        VT_BSTR == V_VT(&pdispparams->rgvarg[0]) )
      hr = S_OK;
    else
      hr = DISP_E_TYPEMISMATCH;
  }
  else
    hr = DISP_E_BADPARAMCOUNT;
  return hr;
}

static HRESULT CheckVoid(DISPPARAMS* pdispparams)
{
  HRESULT hr;

  if( 0 == pdispparams->cArgs && 0 == pdispparams->cNamedArgs)
  {
    hr = S_OK;
  }
  else
    hr = DISP_E_BADPARAMCOUNT;
  return hr;
}

static HRESULT CheckLong(DISPPARAMS* pdispparams)
{
  HRESULT hr;

  if( 1 == pdispparams->cArgs && 0 == pdispparams->cNamedArgs)
  {
    if( VT_I4 == V_VT(&pdispparams->rgvarg[0]) )
      hr = S_OK;
    else
      hr = DISP_E_TYPEMISMATCH;
  }
  else
    hr = DISP_E_BADPARAMCOUNT;
  return hr;
}

static HRESULT CheckParams(DISPPARAMS* pdispparams, unsigned short nParams, short* lpnType)
{
  unsigned short i;
  HRESULT hr = S_OK;

  if( nParams == pdispparams->cArgs && 0 == pdispparams->cNamedArgs )
  {
    for( i = 0; i < nParams; i++ )
      if( lpnType[i] != V_VT(&pdispparams->rgvarg[nParams-i]) )
        hr = DISP_E_TYPEMISMATCH;
  }
  else
    hr = DISP_E_BADPARAMCOUNT;
  return hr;
}

HRESULT STDMETHODCALLTYPE Sink_Disp_Invoke(LPDISPATCH lpdisp, DISPID dispidMember, REFIID riid, LCID lcid, USHORT wFlags, DISPPARAMS* pdispparams, VARIANT* pvarResult, EXCEPINFO* pexcepinfo, UINT* puArgErr)
{
  LPCALC_SINK lpSink = SinkFromDispatch(lpdisp);
  LPCALC_DDE lpDde;
  VARIANT varValue;
  LPCTSTR lpszServer;
  LPCTSTR lpszTopic;
  LPCTSTR lpszItem;
  LPVARIANT lpvarElem;
  UINT nDims;
  long lCols, lRows;
  HRESULT hr = S_OK;

  if( lpSink->hDde && (lpDde = (LPCALC_DDE)CalcMemLock(lpSink->hDde)) )
  {
    lpszServer = (LPCTSTR)CalcMemLock(lpDde->hServer);
    lpszTopic = (LPCTSTR)CalcMemLock(lpDde->hTopic);
    lpszItem = (LPCTSTR)CalcMemLock(lpDde->hItem);
    switch( dispidMember )
    {
      case 1: // void Notify(long lNotification, Variant varData);
        hr = CheckLongVariant(pdispparams);
        if( S_OK == hr )
        {
          long lNotification = V_I4(&pdispparams->rgvarg[1]);
          LPVARIANT lpvarData = &pdispparams->rgvarg[0];
          SinkNotify(lpSink, lNotification, lpvarData);
        }
        break;
      case 2: // void LinkError(integer iErrorCode, string sErrorString)
        hr = CheckShortString(pdispparams);
        if( S_OK == hr )
        {
          short nErrorCode = V_I2(&pdispparams->rgvarg[1]);
          BSTR bstrErrorString = V_BSTR(&pdispparams->rgvarg[0]);
          SS_CalcLinkError(lpDde->hSS, lpszServer, lpszTopic, lpszItem, "????", nErrorCode); //????????
        }
        break;
      case 3: // void LinkItemNotSupported()
        if( S_OK == (hr = CheckVoid(pdispparams)) )
          SS_CalcLinkItemNotSupported(lpDde->hSS, lpszServer, lpszTopic, lpszItem);
        break;
      case 4: // void LinkNewData()
        hr = CheckVoid(pdispparams);
        if( S_OK == hr )
        {
          VariantInit(&varValue);
          hr = DispGetPropVariant(lpDde->lpdispLink, OLESTR("Value"), &varValue);
          if( (VT_ARRAY | VT_VARIANT) == varValue.vt )
          {
            nDims = SafeArrayGetDim(varValue.parray);
            SafeArrayAccessData(varValue.parray, &lpvarElem);
            if( 1 == nDims )
            {
              lCols = varValue.parray->rgsabound[0].cElements;
              lRows = 1;
              DdeSetValues(lpSink->hDde, lCols, lRows, lpvarElem);
            }
            else if( 2 == nDims )
            { 
              lCols = varValue.parray->rgsabound[1].cElements;
              lRows = varValue.parray->rgsabound[0].cElements;
              DdeSetValues(lpSink->hDde, lCols, lRows, lpvarElem);
            }
            SafeArrayUnaccessData(varValue.parray);
          }
          else
          {
            DdeSetValues(lpSink->hDde, 1, 1, &varValue);
          }
          SS_CalcLinkNewData(lpDde->hSS, lpszServer, lpszTopic, lpszItem);
          DdeUpdate(lpSink->hDde);
          VariantClear(&varValue);
        }
        break;
      case 5: // void LinkServerDisconnected()
        hr = CheckVoid(pdispparams);
        if( S_OK == hr )
          SS_CalcLinkServerDisconnected(lpDde->hSS, lpszServer, lpszTopic, lpszItem);
        break;
      case 6: // void LinkUnableToConnectToServer()
        hr = CheckVoid(pdispparams);
        if( S_OK == hr )
          SS_CalcLinkUnableToConnectToServer(lpDde->hSS, lpszServer, lpszTopic, lpszItem);
        break;
      case 7: // void LinkItemSupported()
        hr = CheckVoid(pdispparams);
        if( S_OK == hr )
          SS_CalcLinkItemSupported(lpDde->hSS, lpszServer, lpszTopic, lpszItem);
        break;
      case 8: // void LinkNotify()
        hr = CheckVoid(pdispparams);
        if( S_OK == hr )
          SS_CalcLinkNotify(lpDde->hSS, lpszServer, lpszTopic, lpszItem);
        break;
      case 9: // void LinkOutOfMemory()
        hr = CheckVoid(pdispparams);
        if( S_OK == hr )
          SS_CalcLinkOutOfMemory(lpDde->hSS, lpszServer, lpszTopic, lpszItem);
        break;
      case 10: // void RequestCompleted(long lCode)
        hr = CheckLong(pdispparams);
        if( S_OK == hr )
        {
          long lCode = V_I4(&pdispparams->rgvarg[0]);
          SS_CalcLinkRequestCompleted(lpDde->hSS, lpszServer, lpszTopic, lpszItem, lCode);
        }
        break;
      case 11: // void PokeCompleted(long lCode)
        hr = CheckLong(pdispparams);
        if( S_OK == hr )
        {   
          long lCode = V_I4(&pdispparams->rgvarg[0]);
          SS_CalcLinkPokeCompleted(lpDde->hSS, lpszServer, lpszTopic, lpszItem, lCode);
        }
        break;
      default:
        hr = DISP_E_MEMBERNOTFOUND;
        break;
    }
    CalcMemUnlock(lpDde->hServer);
    CalcMemUnlock(lpDde->hTopic);
    CalcMemUnlock(lpDde->hItem);
    CalcMemUnlock(lpSink->hDde);
  }
  return hr;
}

//--------------------------------------------------------------------

IUnknownVtbl g_sink_unkVtbl = {Sink_Unk_QueryInterface,
                               Sink_Unk_AddRef,
                               Sink_Unk_Release};

IDispatchVtbl g_sink_dispVtbl = {Sink_Disp_QueryInterface,
                                 Sink_Disp_AddRef,
                                 Sink_Disp_Release,
                                 Sink_Disp_GetTypeInfoCount,
                                 Sink_Disp_GetTypeInfo,
                                 Sink_Disp_GetIDsOfNames,
                                 Sink_Disp_Invoke};

//--------------------------------------------------------------------

#define WORKLIST_EVENT_ALL_PARTS_PROCESSED      0
#define WORKLIST_EVENT_PART_HAS_ERROR           1
#define WORKLIST_EVENT_PART_HAS_DATA            2
#define WORKLIST_EVENT_UNABLE_TO_TALK_TO_SERVER 3
#define WORKLIST_EVENT_SERVER_DISCONNECTED      4
#define WORKLIST_EVENT_PART_NOT_SUPPORTED       5
#define WORKLIST_EVENT_PART_SUPPORTED           6
#define WORKLIST_EVENT_PART_TEXTDATA            7
#define WORKLIST_EVENT_PART_WRITE_FAILED        8
#define WORKLIST_EVENT_PART_WRITE_COMPLETED     9
#define WORKLIST_EVENT_OUT_OF_MEMORY_ON_INIT   10

//--------------------------------------------------------------------

void SinkNotify(LPCALC_SINK lpSink, long lNotification, LPVARIANT varData)
{
  LPCALC_DDE lpDde;
  //VARIANT varValue;
  //HRESULT hr;
  LPCTSTR lpszServer;
  LPCTSTR lpszTopic;
  LPCTSTR lpszItem;
  //LPVARIANT lpvarElem;
  //long lCols, lRows;
  //BOOL bNewDataSinceLastRead;

  if( lpSink->hDde && (lpDde = (LPCALC_DDE)CalcMemLock(lpSink->hDde)) )
  {
    lpszServer = (LPCTSTR)CalcMemLock(lpDde->hServer);
    lpszTopic = (LPCTSTR)CalcMemLock(lpDde->hTopic);
    lpszItem = (LPCTSTR)CalcMemLock(lpDde->hItem);
    switch( lNotification )
    {
      /*
      case WORKLIST_EVENT_PART_HAS_DATA:
        hr = DispGetPropBool(lpDde->lpdispLink, OLESTR("NewDataSinceLastRead"), &bNewDataSinceLastRead);
        if( bNewDataSinceLastRead )
        {
          VariantInit(&varValue);
          hr = DispGetPropVariant(lpDde->lpdispLink, OLESTR("Value"), &varValue);
          if( (VT_ARRAY | VT_VARIANT) == varValue.vt &&
              2 == SafeArrayGetDim(varValue.parray) )
          {
            lCols = varValue.parray->rgsabound[1].cElements;
            lRows = varValue.parray->rgsabound[0].cElements;
            SafeArrayAccessData(varValue.parray, &lpvarElem);
            DdeSetValues(lpSink->hDde, lCols, lRows, lpvarElem);
            SafeArrayUnaccessData(varValue.parray);
          }
          else
          {
            DdeSetValues(lpSink->hDde, 1, 1, &varValue);
          }
          DdeUpdate(lpSink->hDde);
          VariantClear(&varValue);
        }
        else if( CALC_LINKMODE_WARM == DdeGetMode(lpSink->hDde) )
          SS_CalcLinkNotify(lpDde->hSS, lpszServer, lpszTopic, lpszItem);
        break;
      case WORKLIST_EVENT_PART_NOT_SUPPORTED:
        SS_CalcLinkItemNotSupported(lpDde->hSS, lpszServer, lpszTopic, lpszItem);
        break;
      case WORKLIST_EVENT_PART_HAS_ERROR:
        SS_CalcLinkError(lpDde->hSS, lpszServer, lpszTopic, lpszItem, "????", 0); //????????
        break;
      case WORKLIST_EVENT_PART_SUPPORTED:
        SS_CalcLinkItemSupported(lpDde->hSS, lpszServer, lpszTopic, lpszItem);
        break;
      case WORKLIST_EVENT_OUT_OF_MEMORY_ON_INIT:
        SS_CalcLinkOutOfMemory(lpDde->hSS, lpszServer, lpszTopic, lpszItem);
        break;
      case WORKLIST_EVENT_SERVER_DISCONNECTED:
        SS_CalcLinkServerDisconnected(lpDde->hSS, lpszServer, lpszTopic, lpszItem);
        break;
      case WORKLIST_EVENT_UNABLE_TO_TALK_TO_SERVER:
        SS_CalcLinkUnableToConnectToServer(lpDde->hSS, lpszServer, lpszTopic, lpszItem);
        break;
      */
    }
    CalcMemUnlock(lpDde->hServer);
    CalcMemUnlock(lpDde->hTopic);
    CalcMemUnlock(lpDde->hItem);
    CalcMemUnlock(lpSink->hDde);
  }
}

//--------------------------------------------------------------------

LPCALC_SINK SinkCreate(CALC_HANDLE hDde)
{
  LPCALC_SINK lpSink;

  if( lpSink = (LPCALC_SINK)GlobalAlloc(GPTR, sizeof(CALC_SINK)) )
  {
    lpSink->unk.lpVtbl = &g_sink_unkVtbl;
    lpSink->disp.lpVtbl = &g_sink_dispVtbl;
    lpSink->ulRefCnt = 1;
    lpSink->hDde = hDde;
  }
  return lpSink;
}

//--------------------------------------------------------------------

#if defined(SS_DLL)
const GUID IID_IConnectionPointContainer = {0xB196B284,0xBAB4,0x101A,{0xB6,0x9C,0x00,0xAA,0x00,0x34,0x1D,0x07}};
//const GUID IID_IEnumConnectionPoints     = {0xB196B285,0xBAB4,0x101A,{0xB6,0x9C,0x00,0xAA,0x00,0x34,0x1D,0x07}};
//const GUID IID_IConnectionPoint          = {0xB196B286,0xBAB4,0x101A,{0xB6,0x9C,0x00,0xAA,0x00,0x34,0x1D,0x07}};
//const GUID IID_IEnumConnections          = {0xB196B287,0xBAB4,0x101A,{0xB6,0x9C,0x00,0xAA,0x00,0x34,0x1D,0x07}};
#endif

const GUID IID_RSJBoxLink                = {0x363b54c3, 0x260e, 0x11d0, {0x8c, 0xcb, 0xa6, 0x43, 0x4d, 0x20, 0x20, 0x8e}};
const GUID IID_DRSJBoxEvents             = {0x363B54C2,0x260E,0x11D0,{0x8C,0xCB,0xAD,0x43,0x4D,0x20,0x20,0x8E}};

//--------------------------------------------------------------------

LPDISPATCH LinkCreate(LPCTSTR lpszServer, LPCTSTR lpszTopic,
                      LPCTSTR lpszItem, CALC_HANDLE hDde)
{
  LPCALC_DDE lpDde;
  LPDISPATCH lpdisp = NULL;
  LPCONNECTIONPOINTCONTAINER lpcpc = NULL;
  LPENUMCONNECTIONPOINTS lpecp = NULL;
  LPCONNECTIONPOINT lpcp = NULL;
  HRESULT hr;

  hr = CoCreateInstance(&IID_RSJBoxLink, NULL, CLSCTX_ALL,
                       &IID_IDispatch, (LPVOID FAR*)&lpdisp);
  if( SUCCEEDED(hr) )
  {
    DispSetPropString(lpdisp, OLESTR("LinkServer"), lpszServer);
    DispSetPropString(lpdisp, OLESTR("LinkTopic"), lpszTopic);
    DispSetPropString(lpdisp, OLESTR("LinkItem"), lpszItem);
    DispSetPropShort(lpdisp, OLESTR("LinkMode"), CALC_LINKMODE_NONE);
    hr = lpdisp->lpVtbl->QueryInterface(lpdisp, &IID_IConnectionPointContainer, &lpcpc);
    if( SUCCEEDED(hr) )
    {
      hr = lpcpc->lpVtbl->FindConnectionPoint(lpcpc, &IID_DRSJBoxEvents, &lpcp);
      if( SUCCEEDED(hr) )
      {
        if( hDde && (lpDde = (LPCALC_DDE)CalcMemLock(hDde)) )
        {
          LPCALC_SINK lpsink = SinkCreate(hDde);
          hr = lpcp->lpVtbl->Advise(lpcp, &lpsink->unk, &lpDde->dwCookie);
          lpsink->unk.lpVtbl->Release(&lpsink->unk);
          CalcMemUnlock(hDde);
        }
        lpcp->lpVtbl->Release(lpcp);
      }
      lpcpc->lpVtbl->Release(lpcpc);
    }
  }
  return lpdisp;
}

//--------------------------------------------------------------------

//#pragma message(__FILE__ ": ToDo: Investigate crash when calling LinkDestroy()")

BOOL LinkDestroy(LPDISPATCH lpdisp, DWORD dwCookie)
{
  LPCONNECTIONPOINTCONTAINER lpcpc = NULL;
  LPENUMCONNECTIONPOINTS lpecp = NULL;
  LPCONNECTIONPOINT lpcp = NULL;
  HRESULT hr;

  if( lpdisp )
  {
    if( dwCookie )
    {
      hr = lpdisp->lpVtbl->QueryInterface(lpdisp, &IID_IConnectionPointContainer, &lpcpc);
      if( SUCCEEDED(hr) )
      {
        hr = lpcpc->lpVtbl->FindConnectionPoint(lpcpc, &IID_DRSJBoxEvents, &lpcp);
        if( SUCCEEDED(hr) )
        {
          hr = lpcp->lpVtbl->Unadvise(lpcp, dwCookie);
          lpcp->lpVtbl->Release(lpcp);
        }
        lpcpc->lpVtbl->Release(lpcpc);
      }
    }
    lpdisp->lpVtbl->Release(lpdisp);
  }
  return TRUE;
}

#endif  // defined(SS_DDE)
