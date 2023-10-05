//--------------------------------------------------------------------
//
//  File: cust_celltype.c
//
//  Description: Routines to handle custom cell types
//

#if SS_V80

#include <windows.h>
#include <tchar.h>
#include "toolbox.h"
#include "cust_celltype.h"

//--------------------------------------------------------------------
//
//  The CustCellTypeInit() function initializes a custom celltype table.
//

BOOL CustCellTypeInit(LPSS_CT_TABLE lpTable)
{
  lpTable->hElem = 0;
  lpTable->nElemCnt = 0;
  lpTable->nAllocCnt = 0;
  lpTable->nAllocInc = 10;
  return TRUE;
}

//--------------------------------------------------------------------
//
//  The CustCellTypeFree() function destroys a custom celltype table.
//  All memory associated with the table is freed.
//

BOOL CustCellTypeFree(LPSS_CT_TABLE lpTable)
{
  LPCT_HANDLE lpElem;
  int i;
 
  if( lpTable->hElem ) 
  {
    if( lpElem = (LPCT_HANDLE)CT_MemLock(lpTable->hElem) )
    {
      for( i = 0; i < lpTable->nElemCnt; i++ )
        CustCellTypeDestroy(lpElem[i]);
      CT_MemUnlock(lpTable->hElem);
    }
    CT_MemFree(lpTable->hElem);
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

static BOOL Search(LPCTSTR lpszName, LPCT_HANDLE lpElem, int nElemCnt,
                   int FAR* lpnPos)
{
  LPSS_CUSTOMCELLTYPE lpCT;
  LPTSTR lpszCT;
  int nFirst = 0;
  int nLast = nElemCnt - 1;
  int nMiddle = (nFirst + nLast) / 2;
  int iCmp = 1;
  
  while( iCmp != 0 && nFirst <= nLast )
  {
    if( lpElem[nMiddle]
        && (lpCT = (LPSS_CUSTOMCELLTYPE)CT_MemLock(lpElem[nMiddle])) )
    {
      if( lpszCT = (LPTSTR)CT_MemLock(lpCT->hText) )
      {
        iCmp = lstrcmpi(lpszName, lpszCT);
        if( iCmp < 0 )
          nLast = nMiddle - 1;  
        else if( iCmp > 0 )
          nFirst = nMiddle + 1;
        CT_MemUnlock(lpCT->hText);
      }
      else  // failed to lock celltype name
        nLast = nMiddle - 1;
      CT_MemUnlock(lpElem[nMiddle]);
    }
    else  // failed to lock custom celltype
      nLast = nMiddle - 1;
    nMiddle = (nFirst + nLast) / 2;    
  }
  *lpnPos = iCmp == 0 ? nMiddle : nFirst;
  return iCmp == 0;
}

//--------------------------------------------------------------------
//
//  The CustCellTypeAdd() function adds a custom celltype to the table.
//  If the name already existed, the old celltype is replaced with
//  the new celltype.  If needed, the size of the array of handles is
//  expanded.  If successful, the function returns TRUE.  Otherwise,
//  if returns FALSE.
//

BOOL CustCellTypeAdd(LPSS_CT_TABLE lpTable, CT_HANDLE hCT)
{
  LPCT_HANDLE lpElem;
  LPSS_CUSTOMCELLTYPE lpCT;
  LPCTSTR lpszName;
  BOOL bReturn = FALSE;
  int nPos;
  int i;
  
  if( lpTable->nElemCnt >=  lpTable->nAllocCnt )
  {
    lpTable->nAllocCnt += lpTable->nAllocInc;
    if( lpTable->hElem )
      lpTable->hElem = CT_MemReAlloc(lpTable->hElem,
                                   lpTable->nAllocCnt * sizeof(CT_HANDLE));
    else
      lpTable->hElem = CT_MemAlloc(lpTable->nAllocCnt * sizeof(CT_HANDLE));
    if( !lpTable->hElem )
    {
      lpTable->nElemCnt = 0;
      lpTable->nAllocCnt = 0;
    }
  }
  if( lpTable->hElem && (lpElem = (LPCT_HANDLE)CT_MemLock(lpTable->hElem)) )
  {
    if( hCT && (lpCT = (LPSS_CUSTOMCELLTYPE)CT_MemLock(hCT)) )
    {
      if( lpCT->hText && (lpszName = (LPTSTR)CT_MemLock(lpCT->hText)) )
      {
        if( !Search(lpszName, lpElem, lpTable->nElemCnt, &nPos) )
        {
          if( hCT = CustCellTypeCreateRef(hCT) )
          {
            for( i = lpTable->nElemCnt; i > nPos; i-- )
              lpElem[i] = lpElem[i-1];
            lpTable->nElemCnt++;
            lpElem[nPos] = hCT;
            bReturn = TRUE;
          }
        }
        CT_MemUnlock(lpCT->hText);
      }
      CT_MemUnlock(hCT);
    }
    CT_MemUnlock(lpTable->hElem);
  }
  return bReturn;
}

//--------------------------------------------------------------------
//
//  The CustCellTypeRemove() function removes a custom function entry
//  from the given table.  If successful, the function returns
//  TRUE. Otherwise, the function returns FALSE.
//

BOOL CustCellTypeRemove(LPSS_CT_TABLE lpTable, CT_HANDLE hCT)
{
  LPCT_HANDLE lpElem;
  BOOL bReturn = FALSE;
  int i;
  
  if( lpTable->hElem && (lpElem = (LPCT_HANDLE)CT_MemLock(lpTable->hElem)) )
  {
    for( i = 0; i < lpTable->nElemCnt; i++ )
    {
      if( hCT == lpElem[i] )
      {
        CustCellTypeDestroy(lpElem[i]);
        lpTable->nElemCnt--;
        for( ; i < lpTable->nElemCnt; i++ )
          lpElem[i] = lpElem[i+1];
        bReturn = TRUE;
      }
    }
    CT_MemUnlock(lpTable->hElem);
  }
  return bReturn;
}

//--------------------------------------------------------------------
//
//  The CustCellTypeLookup() function retrieves a custom function from
//  the given table.  If found, the function returns a handle to the
//  desired custom function.  If not found, the function returns NULL.

CT_HANDLE CustCellTypeLookup(LPSS_CT_TABLE lpTable, LPCTSTR lpszName)
{
  LPCT_HANDLE lpElem;
  CT_HANDLE hReturn = 0;
  int nPos;
  
  if( lpTable->hElem && (lpElem = (LPCT_HANDLE)CT_MemLock(lpTable->hElem)) )
  {
    if( Search(lpszName, lpElem, lpTable->nElemCnt, &nPos) )
      hReturn = lpElem[nPos];
    CT_MemUnlock(lpTable->hElem);
  }
  return hReturn;
}

//--------------------------------------------------------------------
//
//  The CustCellTypeFirst() function retrieves the first custom function
//  in the given table.  If successful, the function returns a handle
//  to the desired function.  Otherwise, the function returns NULL.
//

CT_HANDLE CustCellTypeFirst(LPSS_CT_TABLE lpTable)
{
  LPCT_HANDLE lpElem;
  CT_HANDLE hReturn = 0;
  
  if( lpTable->hElem
      && lpTable->nElemCnt > 0
      && (lpElem = (LPCT_HANDLE)CT_MemLock(lpTable->hElem)) )
  {
    hReturn = lpElem[0];
    CT_MemUnlock(lpTable->hElem);
  }
  return hReturn;
}

//--------------------------------------------------------------------
//
//  The CustCellTypeNext() function retrieves the next custom function
//  in the given table.  If successful, the function returns a handle
//  to the desired function.  Otherwise, the function returns NULL.
//

CT_HANDLE CustCellTypeNext(LPSS_CT_TABLE lpTable, LPCTSTR lpszName)
{
  LPCT_HANDLE lpElem;
  CT_HANDLE hReturn = 0;
  int nPos;
  
  if( lpTable->hElem && (lpElem = (LPCT_HANDLE)CT_MemLock(lpTable->hElem)) )
  {
    if( Search(lpszName, lpElem, lpTable->nElemCnt, &nPos) )
      nPos++;
    if( nPos < lpTable->nElemCnt )
      hReturn = lpElem[nPos];
    CT_MemUnlock(lpTable->hElem);
  }
  return hReturn;
}

//--------------------------------------------------------------------
//
//  The CustCellTypeCreate() function creates a custom celltype.  If
//  sucessful, the function returns the handle to the custom celltype.
//  Otherwise, the function returns NULL.
//

CT_HANDLE CustCellTypeCreate(LPCTSTR lpszName, BOOL bEditable, BOOL bCanOverflow, BOOL bCanBeOverflown, BOOL bUseRendererControl, LPSS_CT_PROCS lpProcs)
{
  LPSS_CUSTOMCELLTYPE lpCT;
  CT_HANDLE hCT;
  CT_HANDLE hCopy;
  CT_HANDLE hReturn = 0;
  LPTSTR lpszCopy;

  if( lpszName && lpszName[0] )
  {
    hCT = CT_MemAlloc(sizeof(SS_CUSTOMCELLTYPE));
    if( hCT && (lpCT = (LPSS_CUSTOMCELLTYPE)CT_MemLock(hCT)) )
    {
      hCopy = CT_MemAlloc((lstrlen(lpszName)+1) * sizeof(TCHAR));
      if( hCopy && (lpszCopy = (LPTSTR)CT_MemLock(hCopy)) )
      {
		lpCT->bEditable = bEditable;
		lpCT->bCanOverflow = bCanOverflow;
		lpCT->bCanBeOverflown = bCanBeOverflown;
      lpCT->bUseRendererControl = bUseRendererControl;
      lpCT->lRefCnt = 0;
      if( lpProcs )
		   MemHugeCpy(&lpCT->Procs, lpProcs, sizeof(SS_CT_PROCS));
      lstrcpy(lpszCopy, lpszName);
      lpCT->hText = hCopy;
      hReturn = hCT;
      CT_MemUnlock(hCopy);
      }   
      CT_MemUnlock(hCT);
    }
  }
  return hReturn;
}

//--------------------------------------------------------------------
//
//  The CustCellTypeCreateRef() function increments the reference counter
//  for the custom celltype.  If successful the function returns
//  the handle to the custom celltype.  Otherwise, the function
//  returns NULL.
//

CT_HANDLE CustCellTypeCreateRef(CT_HANDLE hCT)
{
  LPSS_CUSTOMCELLTYPE lpCT;
  CT_HANDLE hReturn = 0;
  
  if( hCT && (lpCT = (LPSS_CUSTOMCELLTYPE)CT_MemLock(hCT)) )
  {
    lpCT->lRefCnt++;
    hReturn = hCT;
    CT_MemUnlock(hCT);
  }
  return hReturn;
}

//--------------------------------------------------------------------
//
//  The CustCellTypeDestroy() function decrements the reference counter
//  for the custom celltype.  If the reference reaches zero then all
//  memory associated the custom celltype is reclaimed.
//

BOOL CustCellTypeDestroy(CT_HANDLE hCT)
{
  LPSS_CUSTOMCELLTYPE lpCT;
  BOOL bReturn = FALSE;
  BOOL bNoReferences = FALSE;
  
  if( hCT && (lpCT = (LPSS_CUSTOMCELLTYPE)CT_MemLock(hCT)) )
  {
    lpCT->lRefCnt--;
    if( bNoReferences = lpCT->lRefCnt == 0 )
    {
      CT_MemFree(lpCT->hText);
      bReturn = TRUE;
    }
    CT_MemUnlock(hCT);
  }
  if( bNoReferences )
    CT_MemFree(hCT);
  return bReturn;
}

//--------------------------------------------------------------------
//
//  The CustCellTypeGetRefCnt() function retrieves the reference counter
//  for the custom celltype.  If successful the function retruns
//  TRUE.  Otherwise, the function returns FALSE.
//

long CustCellTypeGetRefCnt(CT_HANDLE hCT)
{
  LPSS_CUSTOMCELLTYPE lpCT;
  long lReturn = 0;
  
  if( hCT && (lpCT = (LPSS_CUSTOMCELLTYPE)CT_MemLock(hCT)) )
  {
    lReturn = lpCT->lRefCnt;
    CT_MemUnlock(hCT);
  }
  return lReturn;
}

//--------------------------------------------------------------------
//
//  The CustCellTypeGetText() function retrieves the text representation
//  of the celltype's name.  If successful, the function returns
//  the length of lpszText.  Otherwise, the function returns zero.
//

int CustCellTypeGetText(CT_HANDLE hCT, LPTSTR lpszText, int nLen)
{
  LPSS_CUSTOMCELLTYPE lpCT;
  LPTSTR lpszCT;
  int nFuncLen;
  int iResult = 0;
  
  if( nLen && hCT && (lpCT = (LPSS_CUSTOMCELLTYPE)CT_MemLock(hCT)) )
  {
     if( lpCT->hText && (lpszCT = (LPTSTR)CT_MemLock(lpCT->hText)) )
     {
       nFuncLen = lstrlen(lpszCT);
       if( nFuncLen < nLen )
       {
         lstrcpy(lpszText, lpszCT);
         iResult = nFuncLen;
       }
       CT_MemUnlock(lpCT->hText);
     }
     CT_MemUnlock(hCT);
  }
  return iResult;
}

//--------------------------------------------------------------------
//
//  The CustCellTypeGetTextLen() function retrieves the length of the
//  text representation of the function's name.
//

int CustCellTypeGetTextLen(CT_HANDLE hCT)
{
  LPSS_CUSTOMCELLTYPE lpCT;
  LPTSTR lpszCT;
  int iResult = 0;
  
  if( hCT && (lpCT = (LPSS_CUSTOMCELLTYPE)CT_MemLock(hCT)) )
  {
     if( lpCT->hText && (lpszCT = (LPTSTR)CT_MemLock(lpCT->hText)) )
     {
       iResult = lstrlen(lpszCT);
       CT_MemUnlock(lpCT->hText);
     }
     CT_MemUnlock(hCT);
  }
  return iResult;
}

//--------------------------------------------------------------------
//
//  The CustCellTypeLockText() function locks the text representation
//  of the name.
//

LPCTSTR CustCellTypeLockText(CT_HANDLE hCT)
{
  LPSS_CUSTOMCELLTYPE lpCT;
  LPCTSTR lpszCT = NULL;
  if( hCT && (lpCT = (LPSS_CUSTOMCELLTYPE)CT_MemLock(hCT)) )
  {
    if( lpCT->hText )
    {
      lpszCT = (LPCTSTR)CT_MemLock(lpCT->hText);
    }
    CT_MemUnlock(hCT);
  }
  return lpszCT;
}

//--------------------------------------------------------------------
//
//  The CustCellTypeUnlockText() function unlocks the text representation
//  of the name.
//

void CustCellTypeUnlockText(CT_HANDLE hCT)
{
  LPSS_CUSTOMCELLTYPE lpCT;
  LPCTSTR lpszCT = NULL;
  if( hCT && (lpCT = (LPSS_CUSTOMCELLTYPE)CT_MemLock(hCT)) )
  {
    if( lpCT->hText )
    {
      CT_MemUnlock(lpCT->hText);
    }
    CT_MemUnlock(hCT);
  }
}

//--------------------------------------------------------------------
//
//  The CustCellTypeGetEditable() function gets whether the celltype
//  supports editing.
//

BOOL CustCellTypeGetEditable(CT_HANDLE hCT)
{
  LPSS_CUSTOMCELLTYPE lpCT;
  BOOL bResult = 0;
  
  if( hCT && (lpCT = (LPSS_CUSTOMCELLTYPE)CT_MemLock(hCT)) )
  {
    bResult = lpCT->bEditable;
    CT_MemUnlock(hCT);
  }
  return bResult;
}

//--------------------------------------------------------------------
//
//  The CustCellTypeSetEditable() function sets whether the celltype
//  supports editing.
//

BOOL CustCellTypeSetEditable(CT_HANDLE hCT, BOOL bEditable)
{
  LPSS_CUSTOMCELLTYPE lpCT;
  
  if( hCT && (lpCT = (LPSS_CUSTOMCELLTYPE)CT_MemLock(hCT)) )
  {
    lpCT->bEditable = bEditable;
    CT_MemUnlock(hCT);
	return TRUE;
  }
  return FALSE;
}

//--------------------------------------------------------------------
//
//  The CustCellTypeGetCanOverflow() function gets whether the celltype
//  supports overflow into adjacent cells.
//

BOOL CustCellTypeGetCanOverflow(CT_HANDLE hCT)
{
  LPSS_CUSTOMCELLTYPE lpCT;
  BOOL bResult = 0;
  
  if( hCT && (lpCT = (LPSS_CUSTOMCELLTYPE)CT_MemLock(hCT)) )
  {
    bResult = lpCT->bCanOverflow;
    CT_MemUnlock(hCT);
  }
  return bResult;
}

//--------------------------------------------------------------------
//
//  The CustCellTypeSetCanOverflow() function sets whether the celltype
//  supports overflow into adjacent cells.
//

BOOL CustCellTypeSetCanOverflow(CT_HANDLE hCT, BOOL bCanOverflow)
{
  LPSS_CUSTOMCELLTYPE lpCT;
  
  if( hCT && (lpCT = (LPSS_CUSTOMCELLTYPE)CT_MemLock(hCT)) )
  {
    lpCT->bCanOverflow = bCanOverflow;
    CT_MemUnlock(hCT);
	return TRUE;
  }
  return FALSE;
}

//--------------------------------------------------------------------
//
//  The CustCellTypeGetCanBeOverflown() function gets whether the celltype
//  permits other cells to overflow over it.
//

BOOL CustCellTypeGetCanBeOverflown(CT_HANDLE hCT)
{
  LPSS_CUSTOMCELLTYPE lpCT;
  BOOL bResult = 0;
  
  if( hCT && (lpCT = (LPSS_CUSTOMCELLTYPE)CT_MemLock(hCT)) )
  {
    bResult = lpCT->bCanBeOverflown;
    CT_MemUnlock(hCT);
  }
  return bResult;
}

//--------------------------------------------------------------------
//
//  The CustCellTypeSetCanBeOverflown() function sets whether the celltype
//  permits other cells to overflow over it.
//

BOOL CustCellTypeSetCanBeOverflown(CT_HANDLE hCT, BOOL bCanBeOverflown)
{
  LPSS_CUSTOMCELLTYPE lpCT;
  
  if( hCT && (lpCT = (LPSS_CUSTOMCELLTYPE)CT_MemLock(hCT)) )
  {
    lpCT->bCanBeOverflown = bCanBeOverflown;
    CT_MemUnlock(hCT);
	return TRUE;
  }
  return FALSE;
}

//--------------------------------------------------------------------
//
//  The CustCellTypeGetUseRendererControl() function gets whether the celltype
//  permits other cells to overflow over it.
//

BOOL CustCellTypeGetUseRendererControl(CT_HANDLE hCT)
{
  LPSS_CUSTOMCELLTYPE lpCT;
  BOOL bResult = 0;
  
  if( hCT && (lpCT = (LPSS_CUSTOMCELLTYPE)CT_MemLock(hCT)) )
  {
    bResult = lpCT->bUseRendererControl;
    CT_MemUnlock(hCT);
  }
  return bResult;
}

//--------------------------------------------------------------------
//
//  The CustCellTypeSetUseRendererControl() function sets whether the celltype
//  permits other cells to overflow over it.
//

BOOL CustCellTypeSetUseRendererControl(CT_HANDLE hCT, BOOL bUseRendererControl)
{
  LPSS_CUSTOMCELLTYPE lpCT;
  
  if( hCT && (lpCT = (LPSS_CUSTOMCELLTYPE)CT_MemLock(hCT)) )
  {
    lpCT->bUseRendererControl = bUseRendererControl;
    CT_MemUnlock(hCT);
	return TRUE;
  }
  return FALSE;
}

//--------------------------------------------------------------------
//
//  The CustCellTypeGetProcs() function gets the array of function
//  pointers to the callback functions for the celltype.
//

BOOL CustCellTypeGetProcs(CT_HANDLE hCT, LPSS_CT_PROCS lpProcs)
{
  LPSS_CUSTOMCELLTYPE lpCT;
  
  if( lpProcs && hCT && (lpCT = (LPSS_CUSTOMCELLTYPE)CT_MemLock(hCT)) )
  {
	MemHugeCpy(lpProcs, &lpCT->Procs, sizeof(SS_CT_PROCS));
    CT_MemUnlock(hCT);
	return TRUE;
  }
  return FALSE;
}

//--------------------------------------------------------------------
//
//  The CustCellTypeSetProcs() function sets the array of function
//  pointers to the callback functions for the celltype.
//

BOOL CustCellTypeSetProcs(CT_HANDLE hCT, LPSS_CT_PROCS lpProcs)
{
  LPSS_CUSTOMCELLTYPE lpCT;
  
  if( hCT && (lpCT = (LPSS_CUSTOMCELLTYPE)CT_MemLock(hCT)) )
  {
	MemHugeCpy(&lpCT->Procs, lpProcs, sizeof(SS_CT_PROCS));
    CT_MemUnlock(hCT);
	return TRUE;
  }
  return FALSE;
}

//--------------------------------------------------------------------
//
//  The CustCellTypeHasProcs() function returns whether the custom
//  celltype has an array of fuction pointers to the callback functions.
//

BOOL CustCellTypeHasProcs(CT_HANDLE hCT)
{
  LPSS_CUSTOMCELLTYPE lpCT;
  BOOL bResult = 0;
  
  if( hCT && (lpCT = (LPSS_CUSTOMCELLTYPE)CT_MemLock(hCT)) )
  {
	bResult = lpCT->Procs.pfnCreateEditorControl != NULL 
      || lpCT->Procs.pfnCreateRendererControl != NULL
		|| lpCT->Procs.pfnInitializeControl != NULL
		|| lpCT->Procs.pfnGetEditorValue != NULL
		|| lpCT->Procs.pfnSetValue != NULL
		|| lpCT->Procs.pfnStartEditing != NULL
		|| lpCT->Procs.pfnCancelEditing != NULL
		|| lpCT->Procs.pfnStopEditing != NULL
		|| lpCT->Procs.pfnIsReservedKey != NULL
		|| lpCT->Procs.pfnIsReservedLocation != NULL
		|| lpCT->Procs.pfnIsValid != NULL
		|| lpCT->Procs.pfnGetReservedCursor != NULL
		|| lpCT->Procs.pfnGetPreferredSize != NULL
		|| lpCT->Procs.pfnPaintCell != NULL
		|| lpCT->Procs.pfnStringFormat != NULL
		|| lpCT->Procs.pfnStringUnformat != NULL;
    CT_MemUnlock(hCT);
  }
  return bResult;
}

//--------------------------------------------------------------------
//
//  The CustCellTypeGetStruct() function retrieves the custom celltype
//  structure, including all function pointers.
//

BOOL CustCellTypeGetStruct(CT_HANDLE hCT, LPSS_CUSTOMCELLTYPE lpCT_target)
{
  LPSS_CUSTOMCELLTYPE lpCT;
  
  if( lpCT_target && hCT && (lpCT = (LPSS_CUSTOMCELLTYPE)CT_MemLock(hCT)) )
  {
	MemHugeCpy(lpCT_target, lpCT, sizeof(SS_CUSTOMCELLTYPE));
    CT_MemUnlock(hCT);
	return TRUE;
  }
  return FALSE;
}

#endif // SS_V80

