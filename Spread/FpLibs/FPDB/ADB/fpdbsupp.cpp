/*************************************************************************\
* FPDBSUPP.CPP - FarPoint Data Binding source file for ADO Binding.                                                            *
*                                                                         *
* Copyright (C) 1991-1998 - FarPoint Technologies, Inc.                   *
* All Rights Reserved.                                                    *
*                                                                         *
* No part of this source code may be copied, modified or reproduced       *
* in any form without retaining the above copyright notice.  This         *
* source code, or source code derived from it, may not be redistributed   *
* without express written permission of FarPoint Technologies.            *
*                                                                         *
\*************************************************************************/
#ifdef STRICT
#undef STRICT
#endif

#ifndef NO_MFC
#include <afxctl.h>         // MFC support for OLE Custom Controls
#else  //NO_MFC
#define FP_EXCLUDE_ATL
#include "stdafx.h"
//ATL #include "fpatl.h"
#endif

#include "fpdbsupp.h"

/***************************************************************************
 *
 * FUNCTION:  CfpDBColList::CfpDBColList()
 *
 * DESCRIPTION:
 *
 *   Constructor
 *
 * PARAMETERS:
 *
 * RETURNS:
 *
 **************************************************************************/
CfpDBColList::CfpDBColList(void)
{
  m_aList = NULL;
  m_cList = 0;
  m_cAllocated = 0;
  m_hDefAccessor = NULL;
  m_cAccessorCt = 0;
  m_ulFlags   = 0;
}

/***************************************************************************
 *
 * FUNCTION:  CfpDBColList::~CfpDBColList()
 *
 * DESCRIPTION:
 *
 *   Destructor
 *
 * PARAMETERS:
 *
 * RETURNS:
 *
 **************************************************************************/
CfpDBColList::~CfpDBColList(void)
{
  if (m_aList)
    free(m_aList);
}

/***************************************************************************
 *
 * FUNCTION:  CfpDBColList::Clear()
 *
 * DESCRIPTION:
 *
 * PARAMETERS:
 *
 * RETURNS:
 *
 **************************************************************************/
void CfpDBColList::Clear(void)
{
  // CANNOT Clear if accessors have not been released!!!
  _ASSERT(m_hDefAccessor == NULL);
  if (m_hDefAccessor == NULL)
  {
    SetSize(0, FPDB_TRIM_EXTRA);
    m_ulFlags   = 0;
  }
}



/***************************************************************************
 *
 * FUNCTION:  CfpDBColList::SetSize()
 *
 * DESCRIPTION:
 *
 * PARAMETERS:
 *
 * RETURNS:
 *
 **************************************************************************/
BOOL CfpDBColList::SetSize(UINT cElements, BOOL fTrimExtra /* = FPDB_DONT_TRIM*/)
{
  FP_COLUMNITEM *pTmpList;
  BOOL fAllocAttempted;

  // If new size is 0...
  if (cElements == 0)
  {
    // Free current list if we have one (only free list if specified)
    if (m_aList && fTrimExtra)
    {
      free(m_aList);
      m_aList = NULL;
      m_cAllocated = 0;
    }

    m_cList = 0;

    // Return true.
    return TRUE;  
  }

  fAllocAttempted = TRUE;   // assumed true, set to false if not.

  // New size is greater than 0, alloc/realloc new memory block (if necessary).
  if (m_aList)
  {
    // We already have a list. If not large enough or trim was specified,
    // re-allocate the block.
    if ((cElements > m_cAllocated) || 
        (fTrimExtra && (cElements != m_cAllocated)))
      pTmpList = (FP_COLUMNITEM *)realloc(m_aList, cElements * sizeof(FP_COLUMNITEM));
    else
      fAllocAttempted = FALSE;   // nothing allocated
  }
  else
  {
    pTmpList = (FP_COLUMNITEM *)malloc(cElements * sizeof(FP_COLUMNITEM));
  }

  // Check results if allocation was attempted.
  if (fAllocAttempted)
  {
    // If allocation failed, return FALSE.
    if (!pTmpList)
      return FALSE;

    // Assign new list and new allocated size.
    m_aList = pTmpList;
    m_cAllocated = cElements;
  }

  // Assign new size list (not allocated size)
  m_cList = cElements;

  // return successful
  return TRUE;
}

/***************************************************************************
 *
 * FUNCTION:  CfpDBColList::GetSize()
 *
 * DESCRIPTION:
 *
 * PARAMETERS:
 *
 * RETURNS:  Index of "new" element in list.
 *
 **************************************************************************/
inline UINT CfpDBColList::GetSize(void)
{ 
  return m_cList;
}

CfpDBColList::operator UINT() const
{
  return m_cList;
}

CfpDBColList::operator int() const
{
  return (int)m_cList;
}

FP_COLUMNITEM& CfpDBColList::operator[](int index)
{
  return m_aList[index];
}


/***************************************************************************
 *
 * FUNCTION:  CfpDBColList::Add()
 *
 * DESCRIPTION:
 *
 * PARAMETERS:
 *
 * RETURNS:  Index of "new" element in list.
 *
 **************************************************************************/
int CfpDBColList::Add(UINT colIndex, BOOL fFindIndex /* = FALSE*/ )
{
  return AddWithFlags(colIndex, 0, fFindIndex);
}

/***************************************************************************
 *
 * FUNCTION:  CfpDBColList::AddWithFlags()
 *
 * DESCRIPTION:
 *
 * PARAMETERS:
 *
 * RETURNS:  Index of "new" element in list.
 *
 **************************************************************************/
int CfpDBColList::AddWithFlags(UINT colIndex, ULONG ulFlags, 
  BOOL fFindIndex /* = FALSE*/ )
{
  int i;

  // If "fFindIndex" was specified, then FIRST, look for the given
  // colIndex in the current list.
  if (fFindIndex)
  {
    i = FindColIndex(colIndex);
    if (i >= 0)   // if found,
      return i;   // return index.
  }

  i = (int)m_cList;    // save current size

  // At this point, the element must be added to the list. Check if
  // the allocated size is too small.
  if (m_cAllocated <= m_cList)
  {
    if (!SetSize(m_cAllocated + FPDB_DBCOL_CHUNK)) // allocate a chunk
      return -1;
  }

  // Set current size to "prev size + 1" (but don't trim the extra elements)
  if (!SetSize(i+1))
    return -1;

  // Clear element and set the colIndex and flags.
  ZeroMemory(&m_aList[i], sizeof(FP_COLUMNITEM));
  m_aList[i].colIndex = colIndex;
  if (ulFlags)                       // if not 0, assign the given flags.
  {
    m_aList[i].flags = ulFlags;

    if (ulFlags & FPDB_HASSTREAMDATA)   // if includes bit, add to class flag
      m_ulFlags |= FPDB_HASSTREAMDATA;
  }

  return i;   // return index of new element
}


/***************************************************************************
 *
 * FUNCTION:  CfpDBColList::FindColIndex()
 *
 * DESCRIPTION:
 *
 *   Determines if this column is bound to the database.
 *
 * PARAMETERS:
 *
 * RETURNS:
 *
 **************************************************************************/
int CfpDBColList::FindColIndex(UINT colIndex)
{
  // Check if column is bound.
  UINT i;
  BOOL fFound = FALSE;
  UINT ct = m_cList;

  // Loop thru list of bound columns looking for the one specified.
  for (i = 0; i < ct; i++)
    if (m_aList[i].colIndex == colIndex)
    {
      fFound = TRUE;
      break;
    }

  // If found, return index, else return -1.
  return (fFound ? (int)i : -1);
}


/***************************************************************************
 *
 * FUNCTION:  CfpDBColList::FindOrdinalId()
 *
 * DESCRIPTION:
 *
 *   Returns the "colIndex" field of the element who's ordinal id matches the
 *   one specified in the parameter.
 *
 * PARAMETERS:
 *
 * RETURNS:  colIndex if found, else -1.
 *
 **************************************************************************/
int CfpDBColList::GetColIndexForOrdinalId(ULONG ulOrdId)
{
  // Check if column is bound.
  UINT i;
  UINT ct = m_cList;
  int  iRetColIndex = -1;

  // Loop thru list of bound columns looking for the one specified.
  for (i = 0; i < ct; i++)
    if (m_aList[i].ordinalId == ulOrdId)
    {
      iRetColIndex = (int)(m_aList[i].colIndex);
      break;
    }

  // If found, return index, else return -1.
  return iRetColIndex;
}

/***************************************************************************
 *
 * FUNCTION:  CfpDBColList::SetAccessor()
 *
 * DESCRIPTION:
 *
 *   Given the index, [i], set the accesssor, index may be -1.
 *   If the index is -1, then the default accessor is set.
 *
 * PARAMETERS:
 *
 * RETURNS:
 *
 **************************************************************************/
BOOL CfpDBColList::SetAccessor(int index, HACCESSOR hAccessor)
{
  // Verify that index is valid (note: -1 is allowed, 
  // but any other negative is not).
  _ASSERT(index >= -1 && index < (int)m_cList);
  // Note: this code assumes that the hAccessor passed in is not NULL.
  _ASSERT(hAccessor != NULL);
  if (index < -1 || index >= (int)m_cList || hAccessor == NULL)
    return FALSE;

  // If -1, set default accessor.
  if (index == -1)
  {
    // Note: this code assumes that the current hAccessor has been
    // released before calling this function to set one (which must
    // be valid).
    _ASSERT(m_hDefAccessor == NULL);
    if (m_hDefAccessor == NULL)
      m_hDefAccessor = hAccessor;
  }
  else
  {
    FP_COLUMNITEM *pTmpItem = &m_aList[index]; // use tmp ptr

    // Note: this code assumes that the current hAccessor has been
    // released before calling this function to set one.
    _ASSERT(pTmpItem->hAccessor == NULL);
    if (pTmpItem->hAccessor == NULL)
    {
      pTmpItem->hAccessor = hAccessor;         // set accessor
      pTmpItem->flags |= FPDB_ACCESSORCREATED; // set flag
      ++m_cAccessorCt;  // increment num of cols with their own accessor
    }
  }  

  return TRUE;
}


/***************************************************************************
 *
 * FUNCTION:  CfpDBColList::SetAccessorForColIndex()
 *
 * DESCRIPTION:
 *
 *   Given the "colIndex", this function will find the "index" for
 *   the specified "colIndex" and will set the accessor for that index.
 *   "colIndex" may be -1.
 *   If the index is -1, then the default accessor is set.
 *
 * PARAMETERS:
 *
 * RETURNS:
 *
 **************************************************************************/
BOOL CfpDBColList::SetAccessorForColIndex(int colIndex, HACCESSOR hAccessor)
{
  int index;

  // Verify that index is valid (note: -1 is allowed, 
  // but any other negative is not).
  _ASSERT(colIndex >= -1);
  if (colIndex < -1)
    return FALSE;

  // If colIndex is -1, then just use -1, else find the specified index.
  if (colIndex == -1)
    index = -1;
  else
  {
    // Find index into "m_aList" array.
    index = FindColIndex(colIndex);
    _ASSERT(index != -1);  // colIndex was not found!
    if (index == -1)       // this case is not allowed.
      return FALSE;
  }

  return SetAccessor(hAccessor, index);
}

/***************************************************************************
 *
 * FUNCTION:  CfpDBColList::HasValidAccessor()
 *
 * DESCRIPTION:
 *
 *   Returns TRUE if the list has any non-null accessors.
 *
 * PARAMETERS:
 *
 * RETURNS: TRUE or FALSE
 *
 **************************************************************************/
BOOL CfpDBColList::HasValidAccessor(void)
{
  return (m_hDefAccessor != NULL || m_cAccessorCt > 0);
}


/***************************************************************************
 *
 * FUNCTION:  CfpDBColList::GetAccessor()
 *
 * DESCRIPTION:
 *
 *   Given the index, [i], returns the accesssor, index may be -1.
 *   If the index is -1, then the default accessor is returned.
 *
 * PARAMETERS:
 *
 * RETURNS:
 *
 **************************************************************************/
HACCESSOR CfpDBColList::GetAccessor(int index)
{
  HACCESSOR hAccessor = NULL;

  // Verify that index is valid (note: -1 is allowed, 
  // but any other negative is not).
  _ASSERT(index >= -1 && index < (int)m_cList);
  if (index < -1 || index >= (int)m_cList)
    return NULL;

  // If -1, return default accessor.
  if (index == -1)
  {
    hAccessor = m_hDefAccessor;
  }
  else // not default accessor
  {
    FP_COLUMNITEM *pTmpItem = &m_aList[index]; // use tmp ptr

    // If "flags" has stream bit and hAccessor is not null, release accessor.
    if (pTmpItem->flags & FPDB_ACCESSORCREATED)
      hAccessor = pTmpItem->hAccessor;
  }

  return hAccessor;
}


/***************************************************************************
 *
 * FUNCTION:  CfpDBColList::ReleaseAccessor()
 *
 * DESCRIPTION:
 *
 *   Given the index, [i], release the accesssor, index may be -1.
 *   If the index is -1, then the default accessor is set.
 *
 * PARAMETERS:
 *
 * RETURNS:
 *
 **************************************************************************/
HRESULT CfpDBColList::ReleaseAccessor(int index, CRowset *pCRowset)
{
  HRESULT hr = S_FALSE;

  // Verify that index is valid (note: -1 is allowed, 
  // but any other negative is not).
  _ASSERT(index >= -1 && index < (int)m_cList);
  if (index < -1 || index >= (int)m_cList)
    return S_FALSE;

  // If -1, release default accessor.
  if (index == -1)
  {
    if (m_hDefAccessor)
    {
      hr = pCRowset->ReleaseAccessor(m_hDefAccessor);
      m_hDefAccessor = NULL;
    }
  }
  else // not default accessor
  {
    FP_COLUMNITEM *pTmpItem = &m_aList[index]; // use tmp ptr

    // If "flags" has stream bit and hAccessor is not null, release accessor.
    if ((pTmpItem->flags & FPDB_ACCESSORCREATED) && pTmpItem->hAccessor)
    {
      hr = pCRowset->ReleaseAccessor(pTmpItem->hAccessor);
      pTmpItem->hAccessor = NULL;

      // Reset indicators
      pTmpItem->flags &= ~FPDB_ACCESSORCREATED;
      if (m_cAccessorCt > 0)
        --m_cAccessorCt;  // decrement num of cols with their own accessor
    }
  }  

  return hr;  // return results (if ReleaseAccessor was called, else S_FALSE).
}




/***************************************************************************
 *
 * FUNCTION:  CfpDBColList::ReleaseAllAccessors()
 *
 * DESCRIPTION:
 *
 *   Releases all valid accessors.
 *
 * PARAMETERS:
 *
 * RETURNS:
 *
 **************************************************************************/
void CfpDBColList::ReleaseAllAccessors(CRowset *pCRowset)
{
  int i;

  // Release the Default accessor
  ReleaseAccessor(-1, pCRowset);

  // Release any accessor specific to the column.
  i = 0;
  while ((i < (int)m_cList) && m_cAccessorCt)
  {
    if (m_aList[i].flags & FPDB_ACCESSORCREATED)
      ReleaseAccessor(i, pCRowset); // this func will decrement "m_cAccessorCt"
    ++i;
  }

  return;
}


/***************************************************************************
 *
 * FUNCTION:  CfpDBColList::GetStmAccessorCt()
 *
 * DESCRIPTION:
 *
 *   Returns the number of cols with their own accessors (non-default accessors).
 *
 * PARAMETERS:
 *
 * RETURNS:
 *
 **************************************************************************/
UINT CfpDBColList::GetStmAccessorCt(void)
{
  return m_cAccessorCt;
}




/***************************************************************************
 *
 * FUNCTION:  CfpDBColList::SetData()
 *
 * DESCRIPTION:
 *
 *   Calls CRowset's SetData() function for all of the accessors
 *   in the list.
 *
 * PARAMETERS:
 *
 * RETURNS:
 *
 **************************************************************************/
HRESULT CfpDBColList::SetData(CRowset *pCRowset, HROW hCurrRow, 
  LPSTR lpszSetDataBuffer, BOOL fBatchUpdate)
{
  HRESULT hr = S_OK;
  BOOL    fUpdateRow = TRUE;
  DBROWSTATUS *pRowStatus = NULL;

  // First call SetData() with the Default accessor.
  if (m_hDefAccessor)
  {
    hr = pCRowset->SetData(hCurrRow, m_hDefAccessor, lpszSetDataBuffer);
    // If failed, then cancel changes.
    if (hr != S_OK)
      fUpdateRow = FALSE; 
  }

  // If successful so far, and we have more accessors, continue.
  if (fUpdateRow && m_cAccessorCt)
  {
    // Loop to find the other accessors
    int  i = 0;
    UINT cStreamCt = 0;
    while ((i < (int)m_cList) && (cStreamCt < m_cAccessorCt) && fUpdateRow)
    {
      // If this column has its own accessor, call SetData().
      if ((m_aList[i].flags & FPDB_ACCESSORCREATED) &&
           m_aList[i].hAccessor)
      {
        hr = pCRowset->SetData(hCurrRow, m_aList[i].hAccessor, 
          lpszSetDataBuffer);
        // If failed, then cancel changes.
        if (hr != S_OK)
          fUpdateRow = FALSE;
        ++cStreamCt;
      }
      ++i;
    }
  }

  // If successful and not batch update, commit changes
  if (fUpdateRow && !fBatchUpdate)
  {
    hr = pCRowset->Update(&hCurrRow, 1, NULL, NULL, &pRowStatus);
    if (hr != S_OK)
      fUpdateRow = FALSE;              // not successful
    if (pRowStatus)                // check for success
    {
      if (hr == S_OK && *pRowStatus != DBROWSTATUS_S_OK)
        fUpdateRow = FALSE;              // not successful
      CoTaskMemFree(pRowStatus);   // free allocated memory
    }
  }

  // If something failed and not batch update, "rollback" the changes
  if (!fUpdateRow && !fBatchUpdate)
  {
    hr = pCRowset->Undo(&hCurrRow, 1, NULL, NULL, &pRowStatus);
    if (pRowStatus)                // check for success
      CoTaskMemFree(pRowStatus);   // free allocated memory
  }

  return (fUpdateRow ? S_OK : E_FAIL);  // return S_OK or E_FAIL
}


/***************************************************************************
 *
 * FUNCTION:  CfpDBColList::GetFlagsPtr()
 *
 * DESCRIPTION:
 *
 *   Returns a ptr to the list's member variable used for list flags.
 *
 * PARAMETERS:
 *
 * RETURNS: ULONG *
 *
 **************************************************************************/
PULONG CfpDBColList::GetFlagsPtr(void)
{
  return &m_ulFlags;
}

/***************************************************************************
 *
 * FUNCTION:  CfpDBColList::HasStreamData()
 *
 * DESCRIPTION:
 *
 *   Returns TRUE if any of the columns uses/has stream data.
 *
 * PARAMETERS:
 *
 * RETURNS: TRUE or FALSE
 *
 **************************************************************************/
BOOL CfpDBColList::HasStreamData(void)
{
  return (m_ulFlags & FPDB_HASSTREAMDATA);
}



/***************************************************************************
 *
 * FUNCTION:  CfpDBColList::IsEmpty()
 *
 * DESCRIPTION:
 *
 *   Returns TRUE if the list is empty.
 *
 * PARAMETERS:
 *
 * RETURNS: TRUE or FALSE
 *
 **************************************************************************/
BOOL CfpDBColList::IsEmpty(void)
{
  return (m_cList == 0);
}


/***************************************************************************
 *
 * FUNCTION:  CfpDBColList::CompareList()
 *
 * DESCRIPTION:
 *
 *   Returns TRUE if there are the same number of columns and
 *   each column matches a column in the other list.  The order of
 *   the columns does not matter.
 *
 * PARAMETERS:
 *
 * RETURNS: TRUE or FALSE
 *
 **************************************************************************/
BOOL CfpDBColList::CompareList(CfpDBColList *pOther)
{
  int i;

  // If the sizes are different, return FALSE.
  if (m_cList != pOther->m_cList)
    return FALSE;

  // The sizes are the same. If the size is zero, return TRUE.
  if (m_cList == 0)
    return TRUE;

  // Now compare the list, item by item.  If one does not match, return FALSE.
  for (i = 0; i < (int)m_cList; i++)
    if (pOther->FindColIndex(m_aList[i].colIndex) == -1) // if not found
      return FALSE;

  // At this point, the lists are the same, return TRUE.
  return TRUE;
}


/***************************************************************************
 *
 * FUNCTION:  CfpDBEventQueue::CfpDBEventQueue()
 *
 * DESCRIPTION:
 *
 *   Constructor for CfpDBEventQueue class.
 *
 * PARAMETERS:
 *
 * RETURNS: 
 *
 **************************************************************************/
CfpDBEventQueue::CfpDBEventQueue(void)
{
  m_cEventCt = 0;
  m_pEvents = NULL;
  m_pLastEvent = NULL;

  m_cPreAllocCt = 0;
  m_pPreAllocEvents = NULL;

  m_cFreeCt     = 0;
  m_pFreeEvents = NULL;

  // Call Reset() to pre-allocate some empty events.
  ResetQueue(NULL); 
}

/***************************************************************************
 *
 * FUNCTION:  CfpDBEventQueue::~CfpDBEventQueue()
 *
 * DESCRIPTION:
 *
 *   Destructor for the class.
 *
 * PARAMETERS:
 *
 * RETURNS: 
 *
 **************************************************************************/
CfpDBEventQueue::~CfpDBEventQueue(void)
{
  // Reset the list and don't pre-allocate the events.
  ResetQueue(NULL, FALSE);

  // If we have pre-allocated events, free them.
  if (m_cPreAllocCt)
    free(m_pPreAllocEvents);
}


/***************************************************************************
 *
 * FUNCTION:  CfpDBEventQueue::ResetQueue()
 *
 * DESCRIPTION:
 *
 *   Resets the list of events.
 *
 * PARAMETERS:
 *
 * RETURNS: 
 *
 **************************************************************************/
void CfpDBEventQueue::ResetQueue(CRowset *pCRowset, 
  BOOL fPreAllocate /* =TRUE */ )
{
  FP_DBEVENT *pEvent;
  ULONG i;

  // If we have events, free them
  while (pEvent = GetNextEvent())
    FreeEvent(pEvent, pCRowset);

  // Now remove the free events
  // If we have free events, get rid of them
  if (m_cFreeCt)
  {
    while (m_pFreeEvents)
    {
      // Get ptr to event and move m_pFreeEvents to next event.
      pEvent = m_pFreeEvents;
      m_pFreeEvents = pEvent->pNext;

      // If free-able, then free it.
      if (pEvent->flags & FP_DBEVENT_FREEABLE)
        free(pEvent);
    }
    // Reset count.
    m_cFreeCt = 0;
  }

  m_fMsgPosted = FALSE;

  // If the pre-allocated list is empty, allocate one.
  if (m_cPreAllocCt == 0 && fPreAllocate)
  {
    // Pre-Allocate some events
    m_pPreAllocEvents = 
      (FP_DBEVENT *)malloc(FP_DBEVENT_PREALLOC_CT * sizeof(FP_DBEVENT));
    // if successful, set the count
    if (m_pPreAllocEvents)
      m_cPreAllocCt = FP_DBEVENT_PREALLOC_CT;
  }

  // If we have pre-allocated events, then add them to the free list.
  if (m_cPreAllocCt && fPreAllocate)
  {
    m_cFreeCt = m_cPreAllocCt;
    m_pFreeEvents = m_pPreAllocEvents;

    // Initialize all events to 0.
    memset(m_pPreAllocEvents, 0, FP_DBEVENT_PREALLOC_CT * sizeof(FP_DBEVENT));

    // For each item in pre-allocated block, let it point to next event,
    // (except for the last event which stays null).
    for (i = 0; i < (FP_DBEVENT_PREALLOC_CT - 1); i++)
      m_pPreAllocEvents[i].pNext = &m_pPreAllocEvents[i+1];
  }
}


/***************************************************************************
 *
 * FUNCTION:  CfpDBEventQueue::AddEvent()
 *
 * DESCRIPTION:
 *
 *   Adds an event to the queue.
 *
 * PARAMETERS:
 *
 * RETURNS: 
 *
 **************************************************************************/
HRESULT CfpDBEventQueue::AddEvent(DBEVENTPHASE ePhase, DBREASON eReason,
  ULONG cRows, const HROW *rghRows, const ULONG *rgCols, CRowset *pCRowset)
{
  FP_DBEVENT *pEvent;
  HROW  *phRowsTmp = NULL; // tmp ptr
  ULONG *pColsTmp  = NULL; // tmp ptr
  ULONG cCols = 0;

  // If cRows is greater than 1, allocate a buffer for it.
  if (cRows > 1) // allocate buffer.
  {
    phRowsTmp = (HROW *)malloc(cRows * sizeof(HROW));
    if (!phRowsTmp)
      return E_OUTOFMEMORY;
  }

  // If an array was specified, then allocate a buffer for it.
  if (rgCols && *rgCols)
  {
    cCols = *rgCols;
    pColsTmp = (ULONG *)malloc((cCols + 1) * sizeof(ULONG));
    if (!pColsTmp)
    {
      // Free hRow buffer if one was allocated and then return an error.
      if (phRowsTmp)
        free(phRowsTmp);
      return E_OUTOFMEMORY;
    }
  }

  // Get an empty event record (which is now part of the queue).
  pEvent = GetEmptyEvent();

  // If no event, then return an error.
  if (!pEvent)
  {
    if (phRowsTmp)            // Free allocated buffer before returning.
      free(phRowsTmp);
    if (pColsTmp)             // Free other allocated buffer.
      free(pColsTmp);
    return E_OUTOFMEMORY;     // return error.
  }

  // Copy some values to the new event.
  pEvent->ePhase  = ePhase;
  pEvent->eReason = eReason;
  pEvent->cRows   = cRows;

  // If only one row then make .rghRows point to .hRow.
  if (cRows)
  {
    if (cRows == 1)
      pEvent->rghRows = &(pEvent->hRow);
    else                      // else use allocated buffer
      pEvent->rghRows = phRowsTmp;

    // Copy row handles and add reference to them.
    memcpy(pEvent->rghRows, rghRows, cRows * sizeof(HROW));

    // If rowset ptr is not null, then add ref to rows.
    if (pCRowset)
    {
      pCRowset->AddRefRows((const HROW *)pEvent->rghRows, cRows);
      pEvent->flags |= FP_DBEVENT_RELEASEROWS;  // set flag
    }
  } // if cRows

  // If there is an array of columns, copy it to the event.
  if (cCols)
  {
    // The array has the count of elements at index 0, the count does not 
    // include itself, so the total number of ULONG's is rgCol[0] + 1.
    memcpy(pColsTmp, rgCols, ((cCols + 1) * sizeof(ULONG)));
    pEvent->rgCols = pColsTmp;
  }

  return S_OK;
}


/***************************************************************************
 *
 * FUNCTION:  CfpDBEventQueue::GetEmptyEvent()
 *
 * DESCRIPTION:
 *
 *   Gets an empty event and adds it to the end of the queue.
 *
 * PARAMETERS:
 *
 * RETURNS: 
 *
 **************************************************************************/
FP_DBEVENT* CfpDBEventQueue::GetEmptyEvent(void)
{
  FP_DBEVENT *pEvent = NULL;

  // First try to get an event from the Free list.
  if (m_cFreeCt)
  {
    // Have free events, remove one from the free list.
    pEvent = m_pFreeEvents;        // get event
    m_pFreeEvents = pEvent->pNext; // make list point next event.
    --m_cFreeCt;                   // decrement free count.
    pEvent->pNext = NULL;          // terminate event.
  }
  // Else, allocate a new event
  else
  {
    pEvent = (FP_DBEVENT *)malloc(sizeof(FP_DBEVENT));
    if (!pEvent)              // If allocate failed, return null
      return NULL;

    memset(pEvent, 0, sizeof(FP_DBEVENT)); // zero event
    pEvent->flags = FP_DBEVENT_FREEABLE;   // mark that it was allocated
  }

  // Add event to the end of the queue. If queue is empty add it to
  // the beginning and the end.
  if (m_cEventCt == 0)
  {
    m_pEvents = m_pLastEvent = pEvent; // All point to new event.
  }
  else // add to end of chain
  {
    m_pLastEvent->pNext = pEvent;  // Add event to end of chain.
    m_pLastEvent = pEvent;         // Event becomes last event in list.
  }

  ++m_cEventCt;               // Increment event count.

  return pEvent;              // Return event to caller.
}


/***************************************************************************
 *
 * FUNCTION:  CfpDBEventQueue::FreeEvent()
 *
 * DESCRIPTION:
 *
 *   Frees an event by either adding it to the free list or freeing the
 *   memory for the event.
 *
 * PARAMETERS:
 *
 * RETURNS: 
 *
 **************************************************************************/
HRESULT CfpDBEventQueue::FreeEvent(FP_DBEVENT *pEvent, CRowset *pCRowset)
{
  // Return if invalid event ptr
  _ASSERT(pEvent);
  if (!pEvent)
    return E_FAIL;

  // If it has rows and they need to be released, release them.
  if (pEvent->cRows && (pEvent->flags & FP_DBEVENT_RELEASEROWS) &&
      pCRowset)
  {
    pCRowset->ReleaseRows(pEvent->rghRows, pEvent->cRows);
    pEvent->flags &= ~FP_DBEVENT_RELEASEROWS;  // clear flag
  }

  // If it has more than 1 row, free the list
  if (pEvent->cRows > 1)
    free(pEvent->rghRows);

  pEvent->cRows = 0;          // reset row count

  // If we have an array of columns, free it.
  if (pEvent->rgCols)
  {
    free(pEvent->rgCols);
    pEvent->rgCols = NULL;
  }

  // Add event to the list of free events. It is added to the beginning
  // of the list (if becomes the first event in the free list).
  pEvent->pNext = m_pFreeEvents;  // Point to first free event
  m_pFreeEvents = pEvent;         // Event becomes the first free event.
  ++m_cFreeCt;                    // Another free event.

  // At this point, decide if we need to purge the Free list.
  if (m_cFreeCt > FP_DBEVENT_FREELIST_MAX)
    TrimFreeList();

  return S_OK;
}


/***************************************************************************
 *
 * FUNCTION:  CfpDBEventQueue::TrimFreeList()
 *
 * DESCRIPTION:
 *
 *   This trims the list of free events to 
 *   a count of "FP_DBEVENT_FREELIST_OPTM" events.
 *
 * PARAMETERS:
 *
 * RETURNS: 
 *
 **************************************************************************/
void CfpDBEventQueue::TrimFreeList(void)
{
  FP_DBEVENT *pTmpFreeList;   // Temp (new) list of free events.
  FP_DBEVENT *pTmpLastEvent;  // Last event in "new" list of free events.
  FP_DBEVENT *pNext;
  FP_DBEVENT *pCurr;

  // If list is already the right size, just return.
  if (m_cFreeCt <= FP_DBEVENT_FREELIST_OPTM)
    return;

  pTmpLastEvent = NULL;
  pTmpFreeList = NULL;
  pNext = NULL;
  pCurr = m_pFreeEvents;
  
  // Purge from list until we have FP_DBEVENT_FREELIST_OPTM or end of list.

  while (pCurr && m_cFreeCt > FP_DBEVENT_FREELIST_OPTM)
  {
    // Save ptr to next event
    pNext = pCurr->pNext;

    // If free-able, free it, else skip it.
    // Note: If an event is not freeable, it becomes the first
    //       event in the free list.
    if (pCurr->flags & FP_DBEVENT_FREEABLE)
    {
      free(pCurr);            // Free this item.
      --m_cFreeCt;            // Decrement the count of items.
    }
    else // Not freeable, put it at the beginning of the free list.
    {
      pCurr->pNext = pTmpFreeList;  // .pNext points to next event
      pTmpFreeList = pCurr;         // event becomes first in list
      // if pTmpLastEvent is null then this is the first event being
      // added to the list, it will become the last as new ones
      // are added before it. Save a ptr to this last event.
      if (!pTmpLastEvent)
        pTmpLastEvent = pCurr;
    }

    pCurr = pNext;          // process the next event
  } // while loop

  // Make the temp free list the permanent free list.
  m_pFreeEvents = pTmpFreeList;

  // If any events are leftover (pCurr != NULL), then add them
  // to the end of the free list.
  if (pCurr)
  {
    if (pTmpLastEvent)
      pTmpLastEvent->pNext = pCurr;  // Add to end of chain.
    else // Else, pCurr IS the new list of free events!
      m_pFreeEvents = pCurr;
  }
}


/***************************************************************************
 *
 * FUNCTION:  CfpDBEventQueue::GetNextEvent()
 *
 * DESCRIPTION:
 *
 *   Gets the next event from the queue and removes it from the queue.
 *
 *   When you are finished with the event, call FreeEvent(), to ensure
 *   that it is freed properly.
 *
 * PARAMETERS:
 *
 * RETURNS: 
 *
 **************************************************************************/
FP_DBEVENT* CfpDBEventQueue::GetNextEvent(void)
{
  FP_DBEVENT *pEvent;

  // If count is zero, then return null.
  if (!m_cEventCt)
    return NULL;
  
  // Get the first event and pluck it from the queue.
  pEvent = m_pEvents;
  m_pEvents = pEvent->pNext;  // Queue now starts at the next event.

  --m_cEventCt;               // Decrement the queue count.

  pEvent->pNext = NULL;       // Clear the ptr to the next event.

  return pEvent;              // Return the event to the caller.
}

/***************************************************************************
 *
 * FUNCTION:  CfpDBEventQueue::PeekNextEvent()
 *
 * DESCRIPTION:
 *
 *   Gets the next event from the queue WITHOUT removing it.
 *
 * PARAMETERS:
 *
 * RETURNS: 
 *
 **************************************************************************/
FP_DBEVENT* CfpDBEventQueue::PeekNextEvent(void)
{
  // If count is not zero, then return a ptr to the first event in
  // the queue.
  if (m_cEventCt)
    return m_pEvents;         // Return the event to the caller.
  else
    return NULL;
}

/***************************************************************************
 *
 * FUNCTION:  CfpDBEventQueue::GetEventCount()
 *
 * DESCRIPTION:
 *
 *   Gets the number of events currently in the queue.
 *
 * PARAMETERS:
 *
 * RETURNS: 
 *
 **************************************************************************/
ULONG CfpDBEventQueue::GetEventCount(void)
{
  return m_cEventCt;
}




/***************************************************************************
 *
 * FUNCTION:  CfpDbInsRowCls::CfpDbInsRowCls()
 *
 * DESCRIPTION:
 *
 * PARAMETERS:
 *
 * RETURNS: 
 *
 **************************************************************************/
CfpDbInsRowCls::CfpDbInsRowCls(void)
{
  m_uCt = 0;
  m_FirstItem = NULL;
  m_LastItem  = NULL;
  m_uNextId = 0;
}

/***************************************************************************
 *
 * FUNCTION:  CfpDbInsRowCls::~CfpDbInsRowCls()
 *
 * DESCRIPTION:
 *
 * PARAMETERS:
 *
 * RETURNS: 
 *
 **************************************************************************/
CfpDbInsRowCls::~CfpDbInsRowCls(void)
{
  if (m_uCt)
    ClearList();
}


/***************************************************************************
 *
 * FUNCTION:  CfpDbInsRowCls::ClearList()
 *
 * DESCRIPTION:
 *
 * PARAMETERS:
 *
 * RETURNS: 
 *
 **************************************************************************/
void CfpDbInsRowCls::ClearList(CRowset *pCRowset)
{
  FP_DBINSROW *pTmp;

  // If list is already empty, just return.
  if (m_uCt == 0)
    return;

  // Loop while there are elements in the list.
  while (m_FirstItem != NULL)
  {
    pTmp = m_FirstItem;
    m_FirstItem = pTmp->pNext;  // point to next item (or null if last item)

    FreeElement(pTmp, pCRowset); // free
  }

  // Clear the other member variables
  m_uCt = 0;
  m_LastItem = NULL;
  m_uNextId = 0;
}

/***************************************************************************
 *
 * FUNCTION:  CfpDbInsRowCls::IsEmpty()
 *
 * DESCRIPTION:
 *
 * PARAMETERS:
 *
 * RETURNS: 
 *
 **************************************************************************/
BOOL CfpDbInsRowCls::IsEmpty(void)
{
  return (m_uCt == 0);
}

/***************************************************************************
 *
 * FUNCTION:  CfpDbInsRowCls::()
 *
 * DESCRIPTION:
 *
 * PARAMETERS:
 *
 * RETURNS: 
 *
 **************************************************************************/
BOOL CfpDbInsRowCls::AddRow(HROW hRow, BOOL fAddRef, CRowset *pCRowset)
{
  FP_DBINSROW *pElem = (FP_DBINSROW *)malloc(sizeof(FP_DBINSROW));

  // If allocation failed, return FALSE.
  if (!pElem)
    return FALSE;

  memset(pElem, 0, sizeof(FP_DBINSROW));

  pElem->hRow = hRow;
  
  // Add reference if instructed to.
  if (fAddRef && hRow)
  {
    _ASSERT(pCRowset);
    if (pCRowset)
    {
      HRESULT hr = pCRowset->AddRefRows(&hRow, 1);
      _ASSERT(hr == S_OK);
      if (hr == S_OK)
        pElem->fRelRef = TRUE;
    }
  }

  // Create unique bookmark for this row.
  if (hRow)
  {
    ULONG ulTmp[2];           // Temp buffer for "fake" bookmark

    ulTmp[0] = (ULONG)-1;     // 8 bytes, with 1st 4 bytes FF
    ulTmp[1] = m_uNextId;     // 2nd 4 bytes as this counter.
    ++m_uNextId;              // increment counter.

    pElem->bmkTmp = new CBookmark(sizeof(ulTmp), (BYTE *)&ulTmp[0]);
  }

  // Add element to end of list.
  if (m_LastItem == NULL)
  {
    m_FirstItem = m_LastItem = pElem; // only item in list
  }
  else
  {
    m_LastItem->pNext = pElem; // Add to end of list.
    m_LastItem = pElem;        // Point to "new" last item in list.
  }

  ++m_uCt;

  return TRUE;
}

/***************************************************************************
 *
 * FUNCTION:  CfpDbInsRowCls::FindRow()
 *
 * DESCRIPTION:
 *
 * PARAMETERS:
 *
 * RETURNS: 
 *
 **************************************************************************/
BOOL CfpDbInsRowCls::FindRow(HROW hRow, CBookmark *pRetBmk)
{
  FP_DBINSROW *ptr = m_FirstItem;
  
  // Loop while not null and not the row we seek
  while (ptr && ptr->hRow != hRow)
    ptr = ptr->pNext;

  // If not found, return FALSE.
  if (ptr == NULL)
    return FALSE;

  // If a ptr for the bookmark was passed, return the bookmark.
  if (pRetBmk)
  {
    if (ptr->bmkTmp)          // if there is a bookmark, copy it
      *pRetBmk = *(ptr->bmkTmp); 
    else                      // else, clear the bookmark passed
      pRetBmk->Clear();
  }

  return TRUE;
}

/***************************************************************************
 *
 * FUNCTION:  CfpDbInsRowCls::GetLastBookmark()
 *
 * DESCRIPTION:
 *
 * PARAMETERS:
 *
 * RETURNS: 
 *
 **************************************************************************/
BOOL CfpDbInsRowCls::GetLastBookmark(CBookmark *pRetBmk)
{
  // If no last item (list is empty) return FALSE.
  if (!m_LastItem)
    return FALSE;

  // If given a bookmark ptr, return the bookmark in it.
  if (pRetBmk)
  {
    if (m_LastItem->bmkTmp)   // if there is a bookmark, copy it
      *pRetBmk = *(m_LastItem->bmkTmp); 
    else                      // else, clear the bookmark passed
      pRetBmk->Clear();
  }

  return TRUE;    
}

/***************************************************************************
 *
 * FUNCTION:  CfpDbInsRowCls::RemoveRow()
 *
 * DESCRIPTION:
 *
 * PARAMETERS:
 *
 * RETURNS: 
 *
 **************************************************************************/
BOOL CfpDbInsRowCls::RemoveRow(HROW hRow, CRowset *pCRowset /* = NULL */)
{
  FP_DBINSROW *pPrev = NULL;
  FP_DBINSROW *pCurr = m_FirstItem;
  
  // Loop while not null and not the row we seek
  while (pCurr && pCurr->hRow != hRow)
  {
    pPrev = pCurr;
    pCurr = pCurr->pNext;
  }

  // At this point, if pCurr is NULL, then row was not found.
  if (!pCurr)
    return FALSE;

  // If this is the first item, adjust the first item ptr
  if (pCurr == m_FirstItem)
    m_FirstItem = pCurr->pNext;

  // If middle of chain, remove from chain.
  if (pPrev)
    pPrev->pNext = pCurr->pNext; // pCurr is now removed from the chain.

  // If this is the last item, adjust the last item ptr
  if (pCurr == m_LastItem)
    m_LastItem = pPrev;       // Point to previous item.

  --m_uCt;                    // Adjust item count
  
  // Free the item
  FreeElement(pCurr, pCRowset);
  
  return TRUE;
}

/***************************************************************************
 *
 * FUNCTION:  CfpDbInsRowCls::FreeElement()
 *
 * DESCRIPTION:
 *
 * PARAMETERS:
 *
 * RETURNS: 
 *
 **************************************************************************/
void CfpDbInsRowCls::FreeElement(FP_DBINSROW *pElem, CRowset *pCRowset)
{
  // If hRow has a reference, then release the reference.
  if (pElem->fRelRef && pElem->hRow)
  {
    _ASSERT(pCRowset);        // ensure that we can release the reference
    if (pCRowset)
      pCRowset->ReleaseRows(&pElem->hRow, 1);
  }

  // If we have a ptr to a bookmark object, free it
  if (pElem->bmkTmp)
    delete pElem->bmkTmp;

  // Free the element itself.
  free(pElem);
}



/***************************************************************************
 *
 * FUNCTION:  CfpDbStream::CfpDbStream()
 *
 * DESCRIPTION:
 *
 *   Constructor.  The parameter is required for the object to work.
 *
 * PARAMETERS:
 *
 * RETURNS: 
 *
 **************************************************************************/
CfpDbStream::CfpDbStream(IStream *pstm)
{
  _ASSERT(pstm);    // This parameter cannot be null!!!
  // Keep a ptr to the stream and increment the Ref count.
  m_pStm = pstm;
  if (m_pStm)
    m_pStm->AddRef();

  m_ulRefCt = 1;
}

/***************************************************************************
 *
 * FUNCTION:  CfpDbStream::~CfpDbStream()
 *
 * DESCRIPTION:
 *
 *   Destructor.
 *
 * PARAMETERS:
 *
 * RETURNS: 
 *
 **************************************************************************/
CfpDbStream::~CfpDbStream(void)
{
  // Release reference to IStream, m_pStm.
  if (m_pStm)
    m_pStm->Release();
}

/***************************************************************************
 *
 * FUNCTION:  CfpDbStream::QueryInterface()
 *
 * DESCRIPTION:
 *
 *   This wraps the QueryInterface() of the stream, m_pStm.
 *
 * PARAMETERS:
 *
 * RETURNS: 
 *
 **************************************************************************/
HRESULT CfpDbStream::QueryInterface( REFIID riid, void **ppvObject)
{
  // Compare REFID with supported interfaces, return interface if
  // supported.
  if (riid == IID_IStream)
    *ppvObject = (IStream *)this;
  else if (riid == IID_ISequentialStream)
    *ppvObject = (ISequentialStream *)this;
  else if (riid == IID_IUnknown)
    *ppvObject = (IUnknown *)this;
  else
    return E_NOINTERFACE;     // not a supported interface

  // Increment ref count and return OK.
  AddRef();
  return S_OK;
}  

/***************************************************************************
 *
 * FUNCTION:  CfpDbStream::Release()
 *
 * DESCRIPTION:
 *
 *   Decrement the reference count by 1. When the ref count goes to 0,
 *   then free this object.
 *
 * PARAMETERS:
 *
 * RETURNS: 
 *
 **************************************************************************/
ULONG CfpDbStream::Release(void)
{
  _ASSERT(m_ulRefCt);         // This should not be 0 before processing.

  --m_ulRefCt;

  // If the ref is not zero, then return the ref count.
  if (m_ulRefCt)
    return m_ulRefCt;

  // Else, the ref is now zero, free this object.
  delete this;

  return 0;                   // Return ref count of 0.
}


/***************************************************************************
 *
 * FUNCTION:  CfpDbStream::AddRef()
 *
 * DESCRIPTION:
 *
 *   Increment the reference count by 1.
 *
 * PARAMETERS:
 *
 * RETURNS: 
 *
 **************************************************************************/
ULONG CfpDbStream::AddRef(void)
{
  ++m_ulRefCt;
  return m_ulRefCt;
}


/***************************************************************************
 *
 * FUNCTION:  CfpDbStream::Clone()
 *
 * DESCRIPTION:
 *
 *   Clone this object by creating a new CfpDbStream object with
 *   a clone of the IStream.
 *
 * PARAMETERS:
 *
 * RETURNS: 
 *
 **************************************************************************/
HRESULT CfpDbStream::Clone(IStream **ppstm)
{
  IStream *pNewStm;           // new clone of m_pStm.
  HRESULT hr;

  _ASSERT(m_pStm);            // m_pStm cannot be null!

  // Create new stream by cloning current stream.
  hr = m_pStm->Clone(&pNewStm);

  // If cloning failed, return error.
  if (hr != S_OK)
    return hr;

  // Create a new CfpDbStream object with cloned Stream.
  *ppstm = (IStream *)(new CfpDbStream(pNewStm));

  // Free our reference to the new IStream (note: "*ppstm" now has a
  // reference to it).
  pNewStm->Release();

  // If we don't have a new object, then return an error, else return OK.
  if (*ppstm == NULL)
    return E_OUTOFMEMORY;

  return S_OK;
}


//**************************************************************************
// NOTE:  The rest of the CfpDbStream functions simply call 
//        default same function in "m_pStm" for processing.
//**************************************************************************

HRESULT CfpDbStream::Read(void *pv, ULONG cb, ULONG *pcbRead)
{
  return m_pStm->Read(pv, cb, pcbRead);
}
  
HRESULT CfpDbStream::Write(const void *pv, ULONG cb, ULONG *pcbWritten)
{
  return m_pStm->Write(pv, cb, pcbWritten);
}        

HRESULT CfpDbStream::Seek(LARGE_INTEGER dlibMove, DWORD dwOrigin,
  ULARGE_INTEGER *plibNewPosition)
{
  return m_pStm->Seek(dlibMove, dwOrigin, plibNewPosition);
}
  
HRESULT CfpDbStream::SetSize(ULARGE_INTEGER libNewSize)
{
  return m_pStm->SetSize(libNewSize);
}
  
HRESULT CfpDbStream::CopyTo(IStream *pstm, ULARGE_INTEGER cb,
  ULARGE_INTEGER *pcbRead, ULARGE_INTEGER *pcbWritten)
{
  return m_pStm->CopyTo(pstm, cb, pcbRead, pcbWritten);
}
  
HRESULT CfpDbStream::Commit(DWORD grfCommitFlags)
{
  return m_pStm->Commit(grfCommitFlags);
}

HRESULT CfpDbStream::Revert(void)
{
  return m_pStm->Revert();
}

HRESULT CfpDbStream::LockRegion(ULARGE_INTEGER libOffset, 
  ULARGE_INTEGER cb, DWORD dwLockType)
{
  return m_pStm->LockRegion(libOffset, cb, dwLockType);
}
  
HRESULT CfpDbStream::UnlockRegion(ULARGE_INTEGER libOffset,
  ULARGE_INTEGER cb, DWORD dwLockType)
{
  return m_pStm->UnlockRegion(libOffset, cb, dwLockType);
}
  
HRESULT CfpDbStream::Stat(STATSTG *pstatstg, DWORD grfStatFlag)
{
  return m_pStm->Stat(pstatstg, grfStatFlag);
}
  


