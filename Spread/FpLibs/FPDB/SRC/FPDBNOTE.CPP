/*************************************************************************\
* FPDBNOTE.CPP                                                            *
*                                                                         *
* Copyright (C) 1991-1996 - FarPoint Technologies, Inc.                   *
* All Rights Reserved.                                                    *
*                                                                         *
* No part of this source code may be copied, modified or reproduced       *
* in any form without retaining the above copyright notice.  This         *
* source code, or source code derived from it, may not be redistributed   *
* without express written permission of FarPoint Technologies.            *
*                                                                         *
\*************************************************************************/
#ifndef NO_MFC
#include <afxctl.h>         // MFC support for OLE Custom Controls
#else  //NO_MFC
#include "stdafx.h"
#include "fpatl.h"
#endif

//#include "stdafx.h"
#include "vbinterf.h"

#include "fpconvrt.h"
#include "fpdbutil.h"

#include "fpdbnote.h"

#include <limits.h>

#define DB_VT_SAFEARRAY   0x2011

class CfpBookmark : public CObject
{
public:
// Constructors
  CfpBookmark()  
     { m_Bookmark = NULL; 
       m_fSetColumn = FALSE; 
     }

  CfpBookmark(LBLOCK Bookmark)  
     { 
       m_Bookmark = LBlockClone(Bookmark); 
       m_fSetColumn = FALSE;
     }

// Destructor
  ~CfpBookmark()
     { 
       if (m_Bookmark)
         LBlockFree(m_Bookmark);
     }

// comparing
  BOOL operator == (LBLOCK Bookmark)
     {
       LONG l = LBlockLen(m_Bookmark);
       if (l == LBlockLen(Bookmark))
          if (_fmemcmp( m_Bookmark, Bookmark, (size_t)l) == 0)
             return TRUE;   // if both size and byte match, return TRUE
       return FALSE;        // else return FALSE
     }

public:
  LBLOCK m_Bookmark;
  BOOL   m_fSetColumn;
};

/***************************************************************************
 *
 * FUNCTION:  CfpDBNotify::CfpDBNotify()
 *
 * DESCRIPTION:
 *
 * PARAMETERS:
 *
 * RETURNS:
 *
 **************************************************************************/
CfpDBNotify::CfpDBNotify(void)
{
  // The list is already empty, but the function will also reset the flags.
  FlushInternalFlags();
}

/***************************************************************************
 *
 * FUNCTION:  CfpDBNotify::~CfpDBNotify()
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
CfpDBNotify::~CfpDBNotify()
{
  // Remove all from list and delete objects
  FlushInternalFlags();
}

/***************************************************************************
 *
 * FUNCTION:  CfpDBNotify::FlushInternalFlags()
 *
 * DESCRIPTION:
 *
 * PARAMETERS:
 *
 * RETURNS:
 *
 **************************************************************************/
void CfpDBNotify::FlushInternalFlags(void)
{
  // Remove all from list and delete objects
  while (!m_UpdatePendingList.IsEmpty())
    delete ((CfpBookmark *)m_UpdatePendingList.RemoveTail());

  // reset flags
  m_iUpdatePendingCurrRow = 0; 
  m_fSetColumnCurrRow = FALSE; 
  m_fRowDeleted = FALSE;
  m_fMovePending = FALSE;
}

/***************************************************************************
 *
 * FUNCTION:  CfpDBNotify::SyncBefore_InternalFlags()
 *
 * DESCRIPTION:
 *
 * PARAMETERS:
 *
 * RETURNS:
 *
 **************************************************************************/
void CfpDBNotify::SyncBefore_InternalFlags(
  DWORD dwEventWhat, ULONG cReasons, DBNOTIFYREASON rgReasons[],
  BOOL FAR *processList)
{
  BOOL fCurrRow = ((dwEventWhat & DBEVENT_CURRENT_ROW_DATA_CHANGED) != 0);

  for (register int i = 0; i < (int)cReasons; i++)
  {  
    switch (rgReasons[i].dwReason)
    {
      // If beginning one of these, don't let SyncAfter send a MODIFIED msg 
      // until a Find/Move has finished.
      case DBREASON_FIND:
      case DBREASON_MOVE:  
      case DBREASON_SEEK:  
        m_fMovePending = TRUE;
        break;

      case DBREASON_EDIT:
      case DBREASON_ADDNEW:
        AddBookmark( NULL, fCurrRow);
        break;
    }
    processList[i] = TRUE;                   // let all messages thru
  }
}


/***************************************************************************
 *
 * FUNCTION:  CfpOleBinding::SyncAfterInternalFlags()
 *
 * DESCRIPTION:
 *
 * PARAMETERS:
 *
 * RETURNS:
 *
 **************************************************************************/
void CfpDBNotify::SyncAfter_InternalFlags(
  DWORD dwEventWhat, ULONG cReasons, DBNOTIFYREASON rgReasons[],
  BOOL FAR *processList)
{
  int i;
  BOOL fValue;
  BOOL fCurrRow;
  LBLOCK Bookmark;

  fCurrRow = ((dwEventWhat & DBEVENT_CURRENT_ROW_DATA_CHANGED) != 0);

  for (i = 0; i < (int)cReasons; i++)
  {
    fValue = TRUE;
    Bookmark = NULL;
    // get parameters from reason
    switch (rgReasons[i].dwReason)
    {
      case DBREASON_EDIT:           // reset SetColumn flag
      case DBREASON_ADDNEW:         // reset SetColumn flag
      case DBREASON_SETCOLUMN:      // set SetColumn flag
      case DBREASON_MODIFIED:
      case DBREASON_INSERTED:
        if (!fCurrRow &&
            (rgReasons[i].arg1.vt == (VARTYPE)DB_VT_SAFEARRAY))
          CopySafeArrayToLBlock(rgReasons[i].arg1.parray, &Bookmark);
        break;
    }

    switch (rgReasons[i].dwReason)
    {
      case DBREASON_EDIT:           // reset SetColumn flag
      case DBREASON_ADDNEW:         // reset SetColumn flag
//##1   AddBookmark( Bookmark, fCurrRow);  //##1 moved to SyncBefore!
        fValue = FALSE;            // don't process this message 
        break;

      case DBREASON_SETCOLUMN:      // set SetColumn flag
        FlagSetColumn( Bookmark, fCurrRow);
        fValue = FALSE;             // don't process this message 
        break;

      case DBREASON_MODIFIED:
        // if SetColumn() was called for this bookmark, return TRUE
        // else return FALSE.  NOTE: RemoveBookmark() returns TRUE if
        // found in the list AND m_fSetColumn was TRUE.
        // NOTE: If "within" a Find or Move, return FALSE.
        fValue = (RemoveBookmark( Bookmark, fCurrRow) != NULL);
        if (m_fMovePending)
          fValue = FALSE;
        break;

      case DBREASON_INSERTED:
        // remove from list, but return TRUE (default return code)
        RemoveBookmark( Bookmark, fCurrRow);
        break;

      case DBREASON_DELETED:
        // Current row deleted, current row will be invalid until 
        // cursor is moved.
        if (fCurrRow)
           m_fRowDeleted = TRUE;
        break;

      case DBREASON_MOVE:
      case DBREASON_MOVEPERCENT:
//    case DBREASON_REFRESH:               // Ignoring DAO Refresh
      case UDBREASON_SOURCECHANGED:
      case UDBREASON_SELF_REFRESH:
      case DBREASON_FIND:
      case DBREASON_READDATA:
      case DBREASON_SEEK:
        m_fMovePending = FALSE;
        m_fRowDeleted = FALSE;             // RESET flag
        break;
    } // switch

    // Free memory for bookmark
    if (Bookmark)
       LBlockFree(Bookmark);

    processList[i] = fValue;
  } //for 

  return;

} // SyncAfter_InternalFlags()


/***************************************************************************
 *
 * FUNCTION:  CfpDBNotify::WasCurrentRowDeleted()
 *
 * DESCRIPTION:
 *
 * PARAMETERS:
 *
 * RETURNS:
 *
 **************************************************************************/
BOOL CfpDBNotify::WasCurrentRowDeleted()
{
  return m_fRowDeleted;
}


/***************************************************************************
 *
 * FUNCTION:  CfpDBNotify::AddBookmark()
 *
 * DESCRIPTION:
 *
 * PARAMETERS:
 *
 * RETURNS:
 *
 **************************************************************************/
BOOL CfpDBNotify::AddBookmark( LBLOCK Bookmark, BOOL fCurrRow)
{
  BOOL fReturn = TRUE;

  // if current row...
  if (fCurrRow)
  {
    // Increment "update pending" count
    ++m_iUpdatePendingCurrRow;
  }
  else
  {
    // If no bookmark...
    if (!Bookmark)
      fReturn = FALSE;
    else
    {
      POSITION pos = FindBookmark( Bookmark);
      CfpBookmark *pCB; 
      // If already in list (should not be), reset flag
      if (pos)
      {
        pCB = (CfpBookmark *)m_UpdatePendingList.GetAt(pos);
        pCB->m_fSetColumn = FALSE;
      }
      else
      {
        // add to list, if fails, return false
        if (!m_UpdatePendingList.AddTail( new CfpBookmark(Bookmark)))
          fReturn = FALSE;
      }
    }
  }
  return fReturn;
}


/***************************************************************************
 *
 * FUNCTION:  CfpDBNotify::FindBookmark()
 *
 * DESCRIPTION:
 *
 * PARAMETERS:
 *
 * RETURNS:
 *
 **************************************************************************/
POSITION CfpDBNotify::FindBookmark( LBLOCK Bookmark)
{
  CfpBookmark *pCB = NULL;
  POSITION     pos;
  BOOL         fFound;

  if (!Bookmark || m_UpdatePendingList.IsEmpty())
     return NULL;

  pos = m_UpdatePendingList.GetHeadPosition();
  if (pos)
     pCB = (CfpBookmark *)m_UpdatePendingList.GetHead();
  fFound = FALSE;
  while (pCB && !fFound)
  {
    if (*pCB == Bookmark)
      fFound = TRUE;
    else
      pCB = (CfpBookmark *)m_UpdatePendingList.GetNext(pos);
  }
  return pos;    // NULL if not found, else POSITION of CfpBookmark
}


/***************************************************************************
 *
 * FUNCTION:  CfpDBNotify::RemoveBookmark()
 *
 * DESCRIPTION:
 *
 * PARAMETERS:
 *
 * RETURNS:
 *
 **************************************************************************/
BOOL CfpDBNotify::RemoveBookmark( LBLOCK Bookmark, BOOL fCurrRow)
{
  BOOL fReturn = FALSE;

  // if current row...
  if (fCurrRow)
  {
    if (m_iUpdatePendingCurrRow == 1)   // if last one pending, 
      fReturn = m_fSetColumnCurrRow;    //  return value of setcolumn flag
    // reset flags
    if (m_iUpdatePendingCurrRow)
    {  
      --m_iUpdatePendingCurrRow;
      if (m_iUpdatePendingCurrRow == 0)
        m_fSetColumnCurrRow = FALSE;
    }
  }
  else
  {
    // find it in the list
    POSITION pos = FindBookmark( Bookmark);
    CfpBookmark *pCB;

    // if found, set return code and remove from list
    if (pos)
    {
       pCB = (CfpBookmark *)m_UpdatePendingList.GetAt(pos);
       fReturn = pCB->m_fSetColumn;
       m_UpdatePendingList.RemoveAt(pos);      // remove from list
       delete pCB;                             // destroy object
    }
  }
  // return TRUE if found and flag was set
  return fReturn;
}

/***************************************************************************
 *
 * FUNCTION:  CfpDBNotify::FlagSetColumn()
 *
 * DESCRIPTION:
 *
 * PARAMETERS:
 *
 * RETURNS:
 *
 **************************************************************************/
void CfpDBNotify::FlagSetColumn( LBLOCK Bookmark, BOOL fCurrRow)
{
  // if current row, set flag for current row
  if (fCurrRow)
  {
    if (m_iUpdatePendingCurrRow)
      m_fSetColumnCurrRow = TRUE;
  }
  else
  {
    POSITION pos;
    CfpBookmark *pCB;

    // if NULL, use last in list, else find bookmark
    if (Bookmark)
      pos = FindBookmark( Bookmark);
    else
      pos = m_UpdatePendingList.GetTailPosition();

    // if found, set m_fSetColumn flag
    if (pos)
    {
       pCB = (CfpBookmark *)m_UpdatePendingList.GetAt(pos);
       pCB->m_fSetColumn = TRUE;
    }
  }
}
