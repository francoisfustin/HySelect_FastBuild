/*************************************************************************\
* FPDBEVNT.CPP                                                            *
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
#define FP_EXCLUDE_ATL
#include "stdafx.h"
//ATL #include "fpatl.h"
#endif

#include "fptools.h"
#include "fptstr.h"

#include "vbinterf.h"                                                                                                                          

#include "fpconvrt.h"

//#define DBINITCONSTANTS		         // These cause variable declarations
//#define INITGUID
//#include <initguid.h>

#include "fpdbutil.h"		  
//#include "fpdbnote.h"		  

#include "fpoledb.h"		  

#include <limits.h>

// This will get translated to VBM_DATA_INITIATE
#define DATA_UINIT_CONNECT       300
#define DATA_UPOSTMSG            301


// Used for testing purposes.  Allows me to change here instead of changing
// code in various places.
#define   FPDB_UNWANTEDPHASE  DB_S_UNWANTEDPHASE
#define   FPDB_UNWANTEDREASON DB_S_UNWANTEDREASON


/***************************************************************************
 *
 * FUNCTION:  CfpOleBinding::fpOKToDo()
 *
 * DESCRIPTION:
 *
 * PARAMETERS:
 *
 * RETURNS:
 *
 **************************************************************************/
HRESULT CfpOleBinding::fpOKToDo(DBREASON eReason)
{
#define   FPDB_OKTODO_RETCODE  S_OK

  switch (eReason)
  {
    case DBREASON_ROWSET_RELEASE:      //  1
    case DBREASON_COLUMN_SET:          //  2
    case DBREASON_COLUMN_RECALCULATED: //  3
    case DBREASON_ROW_DELETE:          //  6
    case DBREASON_ROW_INSERT:          //  8
    case DBREASON_ROW_UPDATE:          // 13
    case DBREASON_ROWSET_CHANGED:      // 14
      return FPDB_OKTODO_RETCODE;
      break;

    // These reasons are defined in OLE DB 1.5 & 2.0. My documentation
    // does not explain these (OLEDB 1.1 doc), so for now, we will
    // assert.
    case DBREASON_ROWPOSITION_CHANGED:        // 15  (ver 1.5)
    case DBREASON_ROWPOSITION_CHAPTERCHANGED: // 16  (ver 1.5)
    case DBREASON_ROWPOSITION_CLEARED:        // 17  (ver 1.5)
    case DBREASON_ROW_ASYNCHINSERT:           // 18  (ver 1.5)
      return FPDB_OKTODO_RETCODE;
      break;

    case DBREASON_ROWSET_ROWSADDED:           // 19  (ver 2.0)
    case DBREASON_ROWSET_POPULATIONCOMPLETE:  // 20  (ver 2.0)
    case DBREASON_ROWSET_POPULATIONSTOPPED:   // 21  (ver 2.0)
      _ASSERT(0);
      return FPDB_OKTODO_RETCODE;
      break;

    // DBREASON_ROWSET_FETCHPOSITIONCHANGE: //  0
    // DBREASON_ROW_ACTIVATE:               //  4
    // DBREASON_ROW_RELEASE:                //  5
    // DBREASON_ROW_FIRSTCHANGE:            //  7
    // DBREASON_ROW_RESYNCH:                //  9
    // DBREASON_ROW_UNDOCHANGE:             // 10
    // DBREASON_ROW_UNDOINSERT:             // 11
    // DBREASON_ROW_UNDODELETE:             // 12
    default:
      return FPDB_OKTODO_RETCODE; // indicate that we don't want this reason
  }
}



/***************************************************************************
 *
 * FUNCTION:  CfpOleBinding::fpAboutToDo()
 *
 * DESCRIPTION:
 *
 *   This function is called (notified) BEFORE something is about to
 *   happen with this Cursor.  At this point, we have already OK'd the
 *   action.  In reaction to this notification, we send a VBM_DATA_REQUEST
 *   to the owner (by calling its hook function).
 *
 *   Typically, in response to the VBM_DATA_REQUEST, the hook function
 *   will save the data for the current row.  
 *
 *   NOTE:  We send a VBM_DATA_REQUEST msg only when the flag indicates
 *          that data has changed.
 *
 * PARAMETERS:
 *
 * RETURNS:
 *
 **************************************************************************/
HRESULT CfpOleBinding::fpAboutToDo( DBREASON eReason,
  ULONG cRows, const HROW *rghRows, const ULONG *rgCols)
{
  DATAACCESS da = {0};
  LRESULT lResult;
  BOOL  fUpdateAlreadyInProgress = FALSE;
  BOOL  fDataChanged = TRUE;
  SHORT sAction;

  // For DEBUG only: add to list
  _ASSERT(m_DBEventList->Add("AB", eReason,
    (m_iNotifyStatus & NOTIFY_SYNCBEFORE_IGNORED) != 0));

  // If we are no longer "under advisement", OR
  // if nothing changed, return OK
  if ((m_iNotifyStatus & NOTIFY_SYNCBEFORE_IGNORED) ||
      (!(m_ulCtlFlags & CTLFLG_DATACHANGED)))
    fDataChanged = FALSE;

  // Translate the DBReasons for "AboutToDo" into VB DATA_xxxx messages
  sAction = fpTranslateAboutToDoReason( eReason, cRows, rghRows,
    fDataChanged);

  // If sAction is -1, return unwanted, else if sAction is 0 or ignore
  // event, then return S_OK.
  if (sAction == -1)
    return FPDB_UNWANTEDPHASE;
  else if (sAction == 0 || !fDataChanged)
    return S_OK;

  // suspend advising
  m_iNotifyStatus |= NOTIFY_SYNCBEFORE_IGNORED;

  // Determine if an EDIT or ADD has already started.  If (EDIT or ADD) &&
  // SETCOLUMN 
  if ((m_iUpdateStatus & (UPDSTAT_BEGIN_EDIT | UPDSTAT_BEGIN_ADD)) &&
      (m_iUpdateStatus & UPDSTAT_SETCOLUMN))
     fUpdateAlreadyInProgress = TRUE;

  da.sAction = sAction;
  da.lpfpOleBinding = this;

  // Call the owner's function to process the VBM_DATA message
  _ASSERT(m_lpfuncDataMsg); //Callback func MUST BE SET BY OWNER!
  if (m_lpfuncDataMsg)
     lResult = m_lpfuncDataMsg( (LPVOID)m_lpOwner, 
       VBM_DATA_REQUEST, (WPARAM)0, (LPARAM)(LPVOID)&da);

  // If above function "caused" an update to start, finish the update
  // Q: Should we check that (m_ulCtlFlags & CTLFLG_DATACHANGED) == 0 ?
  if (!fUpdateAlreadyInProgress &&
      (m_iUpdateStatus & (UPDSTAT_BEGIN_EDIT | UPDSTAT_BEGIN_ADD)))
  {
    if (!fpFinishUpdatingRow())  // function resets flags
      fpInvokeError();
  }

  // Resume advising
  m_iNotifyStatus &= ~NOTIFY_SYNCBEFORE_IGNORED;

  return S_OK;
}



/***************************************************************************
 *
 * FUNCTION:  CfpOleBinding::fpFailedToDo()
 *
 * DESCRIPTION:
 *
 * PARAMETERS:
 *
 * RETURNS:
 *
 **************************************************************************/
HRESULT CfpOleBinding::fpFailedToDo(DBREASON eReason)
{
#ifdef _DEBUG
  _ASSERT(m_DBEventList->Add("F!", eReason));
#endif

  // If failed to do the DELETE, then clear any list of bookmarks
  if (eReason == DBREASON_ROW_DELETE)
    fpClearEventBookmarks(FPDB_USE_DELETE_LIST);

  return S_OK;
}

/***************************************************************************
 *
 * FUNCTION:  CfpOleBinding::fpSynchAfter()
 *
 * DESCRIPTION:
 *
 * PARAMETERS:
 *
 * RETURNS:
 *
 **************************************************************************/
HRESULT CfpOleBinding::fpSynchAfter(DBREASON eReason)
{
  return S_OK;
}


/***************************************************************************
 *
 * FUNCTION:  CfpOleBinding::fpDidEvent()
 *
 * DESCRIPTION:
 *
 * PARAMETERS:
 *
 * RETURNS:
 *
 **************************************************************************/
HRESULT CfpOleBinding::fpDidEvent(DBREASON eReason, ULONG cRows, 
  const HROW *rghRows, const ULONG *rgCols)
{
  HRESULT hr = S_OK;
  BOOL fNoCurrRow = FALSE;
  UINT uFlags;

  // If Reason is "CHAPTERCHANGED", then check if we have a different
  // chapter.  If same chapters, then just reposition the row, else 
  // continue with refreshing the data.
  if (eReason == DBREASON_ROWPOSITION_CHAPTERCHANGED)
  {
    HCHAPTER hChapter = NULL;
    HROW     hRow     = NULL;
    DBPOSITIONFLAGS dwPosFlags;

    // Get current chapter
    if (m_pRowsetPos)
    {
      hr = m_pRowsetPos->GetRowPosition(&hChapter, &hRow, &dwPosFlags);

      if (hr == S_OK) // if Get was successful, check current row
      {
        // If we have a valid hRow, then release it, else set flag for later.
        if (hRow)
          CRowset::ReleaseRows(&hRow, 1);  // release hRow
        else
          fNoCurrRow = TRUE;               // Don't have a hRow
        fpReleaseChapter(&hChapter);
      }
    }
  }

  // Call function to process Reason.
  hr = fpEachDidEvent(eReason, cRows, rghRows, rgCols);

  // if reason is CHAPTERCHAPTER changed, then call fpDidEvent again
  // with ROWPOSITION_CHANGED.
  if (hr == S_OK && eReason == DBREASON_ROWPOSITION_CHAPTERCHANGED)
  {
    // If current row is invalid, then set current row to first.
    if (fNoCurrRow)
      fpMoveCurrRow(DATA_MOVEFIRST, NULL, &uFlags);
    hr = fpEachDidEvent(DBREASON_ROWPOSITION_CHANGED, cRows, rghRows, NULL);
  }

  return hr;
}

/***************************************************************************
 *
 * FUNCTION:  CfpOleBinding::fpEachDidEvent()
 *
 * DESCRIPTION:
 *
 * PARAMETERS:
 *
 * RETURNS:
 *
 **************************************************************************/
HRESULT CfpOleBinding::fpEachDidEvent(DBREASON eReason, ULONG cRows, 
  const HROW *rghRows, const ULONG *rgCols)
{
  HRESULT hr = S_OK;
  SHORT sAction;
  UINT  uDataMsg;
  LRESULT lResult;
  HLSTR hlstrNewRowBmark = NULL;
//  DWORD dwCurrRowFlag;
//  DWORD dwMask;
  BOOL  fIgnoreEvent;
  DATAACCESS da = {0};

  LPVOID tmpBmkList;
  ULONG  tmpBmkListCt;
  ULONG *tmpColIdList;
  WPARAM wParam;
  LPARAM lParam;

  // make sure there are not more Reason than we were coded for:
  fIgnoreEvent = ((m_iNotifyStatus & NOTIFY_SYNCAFTER_IGNORED) != 0);

  // For DEBUG only: add to list
  _ASSERT(m_DBEventList->Add("DE", eReason, fIgnoreEvent));

  // Set internal flags related to this message
//m_NotifyPreprocessor.SyncAfter_InternalFlags(eReason, cRows, rghRows);

/*
  // if the current row has changed, invalidate the last fetch
  dwCurrRowFlag = (DWORD)0;
  dwMask = FLAG_CURR_ROW_MOVED | FLAG_CURR_ROW_DATACHANGED | FLAG_NON_CURR_DATACHANGED;
  if (dwEventWhat & (DWORD)DBEVENT_CURRENT_ROW_CHANGED)
     dwCurrRowFlag |= FLAG_CURR_ROW_MOVED;
  if (dwEventWhat & (DWORD)DBEVENT_CURRENT_ROW_DATA_CHANGED)
     dwCurrRowFlag |= FLAG_CURR_ROW_DATACHANGED;
  if (dwEventWhat & (DWORD)DBEVENT_NONCURRENT_ROW_DATA_CHANGED)
     dwCurrRowFlag |= FLAG_NON_CURR_DATACHANGED;
  if (dwCurrRowFlag)
     CurrentRowChanged(dwMask, dwCurrRowFlag);
*/

  // Push (save) current list of bookmarks.
  fpResetEventBookmarks(&tmpBmkList, &tmpBmkListCt, &tmpColIdList);

  // Translate the DBReasons for "SyncAfter" into VB DATA_xxxx messages
  sAction = fpTranslateDidEventReason( eReason, cRows, rghRows, rgCols);

  // If sAction is -1, return unwanted, else if sAction is 0 or ignore
  // event, then return S_OK.
  hr = S_FALSE;        // If hr gets changed, then use as flag to return
  if (sAction == -1)
    hr = FPDB_UNWANTEDPHASE;
  else if (sAction == 0 || fIgnoreEvent)
    hr = S_OK;

  if (hr != S_FALSE)   // if hr changed (from S_FALSE), then return
  {
    fpRestoreEventBookmarks(tmpBmkList, tmpBmkListCt, tmpColIdList);
    return hr;
  }

  // suspend advising
  m_iNotifyStatus |= NOTIFY_SYNCAFTER_IGNORED;

  // if we have "actions" which we care about...
  da.sAction = sAction;
  da.lpfpOleBinding = this;

  // Initialize parameters.
  wParam = 0;
  lParam = (LPARAM)(LPVOID)&da;
  // special processing of VBM_DATA_INITIATE
  if (da.sAction == DATA_UINIT_CONNECT)
    uDataMsg = VBM_DATA_INITIATE;
  else
    uDataMsg = VBM_DATA_AVAILABLE;

  // Call the owner's function to process the VBM_DATA message
  _ASSERT(m_lpfuncDataMsg);//Callback func MUST BE SET BY OWNER!
  if (m_lpfuncDataMsg)
    lResult = m_lpfuncDataMsg((LPVOID)m_lpOwner, 
      uDataMsg, wParam, lParam);
  
  if (hlstrNewRowBmark)
    LBlockFree((LBLOCK)hlstrNewRowBmark);

  // Resume advising
  m_iNotifyStatus &= ~NOTIFY_SYNCAFTER_IGNORED;

  // Pop (restore) previous list of bookmarks
  fpRestoreEventBookmarks(tmpBmkList, tmpBmkListCt, tmpColIdList);

  return S_OK;
}


/***************************************************************************
 *
 * FUNCTION:  CfpOleBinding::fpTranslateAboutToDoReason()
 *
 * DESCRIPTION:
 *
 *   Translates the DBNOTIFYREASONs to DATAACCESS.sAction (ex: DATA_REFRESH)
 *   for "SyncBefore" notification messages.  These can cause us to send
 *   a VBM_DATA_REQUEST message to the Control.
 *
 * PARAMETERS:
 *
 * RETURNS:
 *
 *   sAction
 *
 **************************************************************************/
SHORT CfpOleBinding::fpTranslateAboutToDoReason( DBREASON eReason,
  ULONG cRows, const HROW *rghRows, BOOL fDataChanged)
{
  SHORT sAction = 0;

  switch (eReason)
  {
    // Database is closing
    case DBREASON_ROWSET_RELEASE: // 1
      sAction = DATA_CLOSE;
      break;

    // BeginUpdate was called (w/DBROWACTION_UPDATE)
//  case DBREASON_COLUMN_SET:     //  2
//  case DBREASON_ROW_UPDATE:     // 13
//    sAction = DATA_UPDATE;
//    break;

    // Current row has moved to a different row
    case DBREASON_ROWPOSITION_CHANGED:        // 15  (ver 1.5)
    case DBREASON_ROWPOSITION_CHAPTERCHANGED: // 16  (ver 1.5)
    case DBREASON_ROWPOSITION_CLEARED:        // 17  (ver 1.5)
    case DBREASON_ROW_ASYNCHINSERT:           // 18  (ver 1.5)
      sAction = DATA_BOOKMARK;
      break;

    case UDBREASON_DISCONNECT:      // DBREASON_EDIT+101
      sAction = DATA_UNLOAD;
      break;

    case DBREASON_ROW_DELETE:          //  6
      // Allocate an array of CBookmarks and fetch the bookmarks
      // from the passed list of "to be deleted" hRows.
      if (cRows)
        fpSaveEventBookmarks(cRows, rghRows, FPDB_NOCOLIDS, FPDB_USE_DELETE_LIST);
      break;

    // Ignore these reasons
    case DBREASON_COLUMN_RECALCULATED: //  3   // what to do???
    case DBREASON_ROW_INSERT:          //  8
    case DBREASON_ROWSET_CHANGED:      // 14   // what to do???
    case UDBREASON_SOURCECHANGED:
    case UDBREASON_SELF_REFRESH:
    case UDBREASON_INIT_CONNECT:    // DBREASON_EDIT+100
      break;

    default:
      sAction = -1;                 // DB_S_UNWANTEDPHASE
      break;
  }

  return sAction;
}


/***************************************************************************
 *
 * FUNCTION:  CfpOleBinding::fpTranslateDidEventReason()
 *
 * DESCRIPTION:
 *
 *   Translates the DBNOTIFYREASONs to DATAACCESS.sAction (ex: DATA_REFRESH)
 *   for "SyncAfter" notification messages.
 *   "sAction" codes are returned in passed array.
 *   Returns number of codes put in rgActionArray.
 *
 * PARAMETERS:
 *
 *   Output
 *   ------
 *   lpHlstrBmark .... Returned bookmark of inserted row if DATA_ADDNEW
 *
 * RETURNS:
 *
 *   Number of reasons translated.
 *
 **************************************************************************/
SHORT CfpOleBinding::fpTranslateDidEventReason( DBREASON eReason, 
  ULONG cRows, const HROW *rghRows, const ULONG *rgCols)
{
  SHORT sAction;
  BOOL  fSaveBookmarks = FALSE;
  
  sAction = 0;

  switch (eReason)
  {
    // Current row deleted
    case DBREASON_ROW_DELETE:          //  6
      // Check if "m_hCurrRow" is one of the deleted rows
      if (m_hCurrRow && cRows)
        for (ULONG i = 0; i < cRows; i++)
          if (m_hCurrRow == rghRows[i])
          {
            fpRequeryCurrHRow(FPDB_RELEASE_ONLY);
            break;  // break out of "for loop" (not case statement)
          }

      // Make the "DeleteList" the "EventList" (list of bookmarks of deleted rows)
      m_cBmkEventList = m_cBmkDeleteList;  // copy
      m_aBmkEventList = m_aBmkDeleteList;  // copy
      m_cBmkDeleteList = 0;                // clear
      m_aBmkDeleteList = NULL;             // clear

      sAction = DATA_DELETE;
      break;

    // Row inserted at current
    case DBREASON_ROW_INSERT:          //  8
      // Try to get bookmark at this row. If it fails then this is one of
      // those cases where the row does not have a bookmark assigned as of yet.
      // Add the row to the list of inserted rows.
      if (cRows)
      {
        CBookmark bmk;
        HRESULT   hr = fpGetBookmarkAtHRow(rghRows[0], &bmk);
        fSaveBookmarks = TRUE;
        // If failed to get bookmark, add this row to the list of inserted rows.
        if (hr != S_OK || (ULONG)bmk == 0)
        {
          HROW      hRowDummy = 0;
          m_InsertedRowList.AddRow(rghRows[0], FPDB_ADDREF, this);
          m_InsertedRowList.FindRow(rghRows[0], &bmk); // get fake bookmark
          // Put our "fake" bookmark in the Event bookmark list.
          fpSaveEventBookmarks(1, &hRowDummy, FPDB_NOCOLIDS);
          m_aBmkEventList[0] = bmk;
          fSaveBookmarks = FALSE;  // don't get event bookmarks
        }

        sAction = DATA_ADDNEW;
        m_uFlagsFetch &= ~FPDB_FETCH_EOF;// Fetch array is no longer at EOF
      }
      break;

    // Data in current row changed
    case DBREASON_ROW_UPDATE:          // 13
      // If this row is an update for an inserted row, then
      // process that row AND fall-thru.
      if (cRows && !m_InsertedRowList.IsEmpty())
      {
        // For each row that is in the list of inserted rows, process
        // a message which indicates a replacement for the "fake" bookmark.
        UINT i;
        for (i = 0; i < cRows; i++)
          if (m_InsertedRowList.FindRow(rghRows[i], NULL))
            fpProcessEvent(DBEVENTPHASE_DIDEVENT, UDBREASON_REPLACE_BMK,
              1, &rghRows[i], FPDB_NOCOLIDS);
      }
    case DBREASON_COLUMN_RECALCULATED: //  3  // NOT SURE about this one.
    case UDBREASON_UPDATE_ROW:
	case DBREASON_ROW_UNDOCHANGE:      // 10
      // if the hRow is valid, process UPDATE
      if (cRows)
      {
        CBookmark bmk;
        HRESULT hr = fpGetBookmarkAtHRow(rghRows[0], &bmk);

        if (hr != DB_E_DELETEDROW)
        {
          sAction = DATA_UPDATE;
          // Check if we need to invalidate the fetch buffer (not fetch array)
          if ((ULONG)m_bmkFetch && bmk == m_bmkFetch)
            fpCurrentRowChanged(FLAGC_DATACHANGED, FLAGC_DATACHANGED);

          fSaveBookmarks = TRUE;
        }
      }
      break;

    case DBREASON_ROWPOSITION_CHAPTERCHANGED: // 16  (ver 1.5)
    // If UnDo insert or delete, then just refresh all data.
    case DBREASON_ROW_UNDOINSERT:      // 11
    case DBREASON_ROW_UNDODELETE:      // 12
    // If rowset changed, refresh all data.
    case DBREASON_ROWSET_CHANGED:      // 14
      // Ensure that the control positions itself on the current row
      // after it refreshes the data 
      // NOTE: Only do if NOT CHAPTERCHANGED, there's special processing
      //       for CHAPTERCHANGED in the function, fpDidEvent().
      if (eReason != DBREASON_ROWPOSITION_CHAPTERCHANGED)
        fpQueueEvent(DBEVENTPHASE_DIDEVENT, DBREASON_ROWPOSITION_CHANGED,
          0, FPDB_NOHROWS, FPDB_NOCOLIDS);
      m_InsertedRowList.ClearList(this); // Clear list of inserted rows
      fpRequeryCurrHRow();  
      sAction = DATA_REFRESH;
      break;

    // Current row has moved to a different row
    case DBREASON_ROWPOSITION_CHANGED:        // 15  (ver 1.5)
//  case DBREASON_ROWPOSITION_CHAPTERCHANGED: // 16  (ver 1.5)
      fpRequeryCurrHRow();  
      sAction = DATA_BOOKMARK;
      fpCurrentRowChanged(FLAG_CURR_ROW_MOVED, FLAG_CURR_ROW_MOVED);
      fSaveBookmarks = TRUE;
      break;

    // Database was closed
    case DBREASON_ROWSET_RELEASE:      //  1
      sAction = DATA_CLOSE;
      break;

    // Special User-defined messages
    case UDBREASON_INIT_CONNECT:    // DBREASON_EDIT+100
      sAction = DATA_UINIT_CONNECT;
      break;

    case UDBREASON_DISCONNECT:      // DBREASON_EDIT+101
      sAction = DATA_UNLOAD;
      break;

    case DBREASON_COLUMN_SET:          //  2
// ======================================================================
//    SCP 5/18/01: Changed "if" statement to exclude "batch" flag.
//                 I don't know why it originally included the "batch" flag.
//    // if Batch processing and the hRow is valid, process COLUMN_SET
//    if (m_fBatchUpdate && cRows && rgCols && *rgCols)
// ----------------------------------------------------------------------
      // if the hRow is valid, process COLUMN_SET
      if (cRows && rgCols && *rgCols)
      {
        CBookmark bmk;
        HRESULT hr = fpGetBookmarkAtHRow(rghRows[0], &bmk);

        if (hr != DB_E_DELETEDROW)
        {
          // Verify that one of the columns being set matches one 
          // that we are bound to.
          BOOL fColMatch = FALSE;
          UINT i = 0;
          UINT uColCt = (UINT)*rgCols; // Index 0 is the count of cols.
          while (i < uColCt && !fColMatch)
          {
            // If we are bound to the col at index "i+1", set flag and end loop.
            if (m_BoundCols.GetColIndexForOrdinalId(rgCols[i+1]) != -1)
              fColMatch = TRUE;
            ++i;
          }
          // If none of the columns match the one that we are bound to, 
          // then break from case statement.
          if (!fColMatch)
            break;  // Break here and eat this message.

          // Invalidate the fetch buffer (not fetch array)
          fpCurrentRowChanged(FLAGC_DATACHANGED, FLAGC_DATACHANGED);

          sAction = DATA_UCOLUPDATE;  // 214
          fSaveBookmarks = TRUE;
        }
      }
      break;


    case UDBREASON_REPLACE_BMK:  // 126
      if (cRows && !m_InsertedRowList.IsEmpty())
      {
        // Add 2 bookmarks to "event bookmark list" and then replace
        // the first one with the "fake" bookmark.  This will give
        // the control 2 bookmarks, the 1st is the bookmark that the
        // control is currently using for that row, the other bookmark
        // is the replacement bookmark. The control should start using
        // the latter as the bookmark for that row.
        if (fpSetupReplacementBmks(rghRows[0]))
          sAction = DATA_UREPLACEBMK;  // 215
      }
      break;

    // Ignore these reasons
    case DBREASON_ROWPOSITION_CLEARED:        // 17  (ver 1.5)
    case DBREASON_ROW_ASYNCHINSERT:           // 18  (ver 1.5)
    case DBREASON_ROWSET_ROWSADDED:           // 19  (ver 2.0)
    case DBREASON_ROWSET_POPULATIONCOMPLETE:  // 20  (ver 2.0)
    case DBREASON_ROWSET_POPULATIONSTOPPED:   // 21  (ver 2.0)
      _ASSERT(0);  // Why/how did we get one of these? Add support?
      break;

    default:
      sAction = -1;  // we don't what these reasons for this phase
      break;
  }
  
  if (fSaveBookmarks)
    fpSaveEventBookmarks(cRows, rghRows, rgCols);

  return sAction;
}

/***************************************************************************
 *
 * FUNCTION:  CfpOleBinding::fpCurrentRowChanged()
 *
 * DESCRIPTION:
 *
 *   Called by XNotifyDBEvents to notify us that the current row
 *   or its position has changed.  We then update our flags on the status
 *   of the current row. 
 *
 * PARAMETERS:
 *
 * RETURNS:
 *
 **************************************************************************/
void CfpOleBinding::fpCurrentRowChanged(DWORD dwMask, DWORD dwFlags)
{
  // reset flags related to current row
  m_dwFetchStatus &= ~dwMask;
  m_dwFetchStatus |= (dwFlags & dwMask);

  // if flags didn't include CurrRowInvalid and currently it is invalid, 
  // see if something changed to make it valid.
  if (!(dwMask & FLAG_CURR_ROW_INVALID) && 
      (m_dwFetchStatus & FLAG_CURR_ROW_INVALID))
     if (dwFlags & (FLAG_CURR_ROW_MOVED | FLAG_CURR_ROW_DATACHANGED))
        m_dwFetchStatus &= ~FLAG_CURR_ROW_INVALID; // reset bit
}

/***************************************************************************
 *
 * FUNCTION:  CfpOleBinding::fpSetupReplacementBmks()
 *
 * DESCRIPTION:
 *
 *   Add 2 bookmarks to "event bookmark list" and then replace
 *   the first one with the "fake" bookmark.  This will give
 *   the control 2 bookmarks, the 1st is the bookmark that the
 *   control is currently using for that row, the other bookmark
 *   is the replacement bookmark. The control should start using
 *   the latter as the bookmark for that row.
 *
 * PARAMETERS:
 *
 * RETURNS:
 *
 **************************************************************************/
BOOL CfpOleBinding::fpSetupReplacementBmks(HROW hRow)
{
  CBookmark bmk;
  HROW rgRows[2];

  // If not in list, return FALSE.
  if (!m_InsertedRowList.FindRow(hRow, &bmk))
    return FALSE;

  // Build array of hRows (2 elements, both of "hRow")
  rgRows[0] = hRow;
  rgRows[1] = hRow;

  // Add the bookmarks to the "event bookmark list"
  fpSaveEventBookmarks(2, rgRows, FPDB_NOCOLIDS);

  // Now change the first bookmark in the list to the "fake" bookmark.
  m_aBmkEventList[0] = bmk;

  // Remove this row from the list of inserted rows.
  m_InsertedRowList.RemoveRow(hRow, this);

  return TRUE;
}



/***************************************************************************
 *
 * FUNCTION:  CfpOleBinding::fpSaveEventBookmarks()
 *
 * DESCRIPTION:
 *
 *   This function copies the passed list of bookmarks and stores them
 *   in the member variables.
 *
 * PARAMETERS:
 *
 * RETURNS:
 *
 **************************************************************************/
void CfpOleBinding::fpSaveEventBookmarks(ULONG cRows, 
  const HROW *rghRows, const ULONG *rgColIds, 
  BOOL fUseDeleteList /* = FALSE */)
{
  CBookmark **pListPtr   = &m_aBmkEventList;
  ULONG      *pListCtPtr = &m_cBmkEventList;
  ULONG     **pColIdListPtr = &m_aColIdEventList;

  if (fUseDeleteList)
  {
    pListPtr   = &m_aBmkDeleteList;
    pListCtPtr = &m_cBmkDeleteList;
    pColIdListPtr = NULL;     // Not used for Delete list.
  }

  // Insure that previous list was flushed
  _ASSERT(*pListPtr == NULL);  // List should be null!!!

  if (cRows)
  {
    // Allocate an array of CBookmarks and fetch the bookmarks
    // from the passed list of "to be deleted" hRows.
    *pListPtr = new CBookmark[cRows];
    _ASSERT(*pListPtr);  // out-of-memory

    if (*pListPtr)    // fetch bookmarks of "to be deleted" hRows
    {
      *pListCtPtr = cRows;
      for (ULONG i = 0; i < cRows; i++)
        fpGetBookmarkAtHRow(rghRows[i], &((*pListPtr)[i]) );
    }
  }

  // If valid ptr AND an array of Column id's was specified, then
  // allocate memory and copy those id's to it.
  if (pColIdListPtr && rgColIds && *rgColIds)
  {
    ULONG ct = *rgColIds;     // First element is the count (excluding it self)
    ULONG ulByteCt = (ct + 1) * sizeof(ULONG);

    *pColIdListPtr = (ULONG *)malloc(ulByteCt);
    if (*pColIdListPtr)
      memcpy(*pColIdListPtr, rgColIds, ulByteCt);
  }
}


/***************************************************************************
 *
 * FUNCTION:  CfpOleBinding::fpClearEventBookmarks()
 *
 * DESCRIPTION:
 *
 *   This frees the current list of bookmarks and column IDs.  The
 *   lists are stored in the member variables of the CfpOleBinding class.
 *
 * PARAMETERS:
 *
 * RETURNS:
 *
 **************************************************************************/
void CfpOleBinding::fpClearEventBookmarks(BOOL fUseDeleteList /* = FALSE */ )
{
  CBookmark **pListPtr   = &m_aBmkEventList;
  ULONG      *pListCtPtr = &m_cBmkEventList;
  ULONG     **pColIdListPtr = &m_aColIdEventList;

  // Set-up the pointers to the member variables.
  if (fUseDeleteList)         // If param is true, clear these member vars.
  {
    pListPtr   = &m_aBmkDeleteList;
    pListCtPtr = &m_cBmkDeleteList;
    pColIdListPtr = NULL;     // not used for delete list
  }

  // If points to a valid list, free the list of bookmarks and reset
  // the list & count.
  if (*pListPtr)
  {
    delete [] *pListPtr;
    *pListPtr = NULL;
    *pListCtPtr = 0;
  }

  // If there is a list of column id's, free that list.
  if (pColIdListPtr && *pColIdListPtr)
  {
    free(*pColIdListPtr);
    *pColIdListPtr = NULL;
  }
}

/***************************************************************************
 *
 * FUNCTION:  CfpOleBinding::fpResetEventBookmarks()
 *
 * DESCRIPTION:
 *
 *   This function returns the settings in the member variables
 *   that hold the current list of "Event bookmarks".  Before returning,
 *   the member variables are reset to NULL in preparation for 
 *   new values which may be set by OTHER functions.
 *
 * PARAMETERS:
 *
 * RETURNS:
 *
 **************************************************************************/
void CfpOleBinding::fpResetEventBookmarks(LPVOID *pRetList, ULONG *pRetListCt,
  ULONG **pRetColIdList)
{
  // Copy current list to passed-in parameters
  *pRetList   = m_aBmkEventList;
  *pRetListCt = m_cBmkEventList;
  *pRetColIdList = m_aColIdEventList;

  // Clear current list
  m_aBmkEventList = NULL;
  m_cBmkEventList = 0;
  m_aColIdEventList = NULL;
}

/***************************************************************************
 *
 * FUNCTION:  CfpOleBinding::fpRestoreEventBookmarks()
 *
 * DESCRIPTION:
 *
 *   This function sets the member variables to the lists specified in
 *   the parameter list.  Before it sets the new values, the previous lists
 *   are purged (with fpClearEventBookmarks()).
 *
 * PARAMETERS:
 *
 * RETURNS:
 *
 **************************************************************************/
void CfpOleBinding::fpRestoreEventBookmarks(LPVOID list, ULONG listCt, 
  ULONG *aColIds)
{
  // Clear (free) current list
  fpClearEventBookmarks();

  // Restore list to the passed-in values.
  m_aBmkEventList = (CBookmark *)list;
  m_cBmkEventList = listCt;
  m_aColIdEventList = aColIds;
}


/***************************************************************************
 *
 * FUNCTION:  CfpOleBinding::fpFilter_OKToDo()
 *
 * DESCRIPTION:
 *
 * PARAMETERS:
 *
 * RETURNS:
 *
 **************************************************************************/
HRESULT CfpOleBinding::fpFilter_OKToDo(DBREASON eReason, int *pfQueueEvent)
{
  *pfQueueEvent = FPDB_EVENT_CANCEL_MSG;
  return S_OK;
}

/***************************************************************************
 *
 * FUNCTION:  CfpOleBinding::fpFilter_AboutToDo()
 *
 * DESCRIPTION:
 *
 * PARAMETERS:
 *
 * RETURNS:
 *
 **************************************************************************/
HRESULT CfpOleBinding::fpFilter_AboutToDo(DBREASON eReason, int *pfQueueEvent)
{
  // NOTE: "*pfQueueEvent" is initialized to FPDB_EVENT_QUEUE_MSG by the caller.
  HRESULT hr = S_OK;

  // If ignoring "SYNC BEFORE" events, then Cancel event and return.
  if (m_iNotifyStatus & NOTIFY_SYNCBEFORE_IGNORED)
  {
    *pfQueueEvent = FPDB_EVENT_CANCEL_MSG;
    return S_OK;
  }

  switch (eReason)
  {
    // Process these events NOW.
    case DBREASON_ROWSET_RELEASE:             // 1
    case DBREASON_ROW_DELETE:                 //  6
    case DBREASON_ROWPOSITION_CHANGED:        // 15  (ver 1.5)
    case DBREASON_ROWPOSITION_CHAPTERCHANGED: // 16  (ver 1.5)
    case DBREASON_ROWPOSITION_CLEARED:        // 17  (ver 1.5)
    case DBREASON_ROW_ASYNCHINSERT:           // 18  (ver 1.5)
      *pfQueueEvent = FPDB_EVENT_PROCESS_NOW;
      break;
  }

  return hr;
}

/***************************************************************************
 *
 * FUNCTION:  CfpOleBinding::fpFilter_FailedToDo()
 *
 * DESCRIPTION:
 *
 * PARAMETERS:
 *
 * RETURNS:
 *
 **************************************************************************/
HRESULT CfpOleBinding::fpFilter_FailedToDo(DBREASON eReason, int *pfQueueEvent)
{
  *pfQueueEvent = FPDB_EVENT_CANCEL_MSG;
  return S_OK;
}

/***************************************************************************
 *
 * FUNCTION:  CfpOleBinding::fpFilter_SynchAfter()
 *
 * DESCRIPTION:
 *
 * PARAMETERS:
 *
 * RETURNS:
 *
 **************************************************************************/
HRESULT CfpOleBinding::fpFilter_SynchAfter(DBREASON eReason, int *pfQueueEvent)
{
  *pfQueueEvent = FPDB_EVENT_CANCEL_MSG;
  return S_OK;
}

/***************************************************************************
 *
 * FUNCTION:  CfpOleBinding::fpFilter_DidEvent()
 *
 * DESCRIPTION:
 *
 * PARAMETERS:
 *
 * RETURNS:
 *
 **************************************************************************/
HRESULT CfpOleBinding::fpFilter_DidEvent(DBREASON eReason, int *pfQueueEvent)
{
  // NOTE: "*pfQueueEvent" is initialized to FPDB_EVENT_QUEUE_MSG by the caller.
  HRESULT hr = S_OK;
  BOOL    fReleaseCurrHRow = FALSE;
  BOOL    fInvalidateFetchBuffer = FALSE;


  // Do any special processing here. Note: The default processing is
  // to add the event to the event queue.
  switch (eReason)
  {
    // Ignore these DB events.
    case DBREASON_ROW_ACTIVATE:  // 4
    case DBREASON_ROW_RELEASE:   // 5
      *pfQueueEvent = FPDB_EVENT_CANCEL_MSG;
      hr = DB_S_UNWANTEDPHASE;
      break;

    case DBREASON_ROWPOSITION_CHANGED:        // 15  (ver 1.5)
      // Release current member var for row and change status flag.
      fReleaseCurrHRow = TRUE;
      fpCurrentRowChanged(FLAG_CURR_ROW_MOVED, FLAG_CURR_ROW_MOVED); 
      break;

    case DBREASON_ROW_DELETE:          //  6
      fpFreeFetchArray();              // Fetch array is no longer valid
      // Check if "m_hCurrRow" is one of the deleted rows
      // ??? fReleaseCurrHRow = TRUE;

      // It's possible (likely) that the current row was deleted, 
      // invalidate the fetch buffer for the "current row."
      fInvalidateFetchBuffer = TRUE;
      break;

//  case DBREASON_ROWPOSITION_CHAPTERCHANGED: // 16  (ver 1.5)
    // If UnDo insert or delete, then just refresh all data.
    case DBREASON_ROW_UNDOINSERT:      // 11
    case DBREASON_ROW_UNDODELETE:      // 12
    // If rowset changed, refresh all data.
    case DBREASON_ROWSET_CHANGED:      // 14
      fpFreeFetchArray();              // Fetch array is no longer valid
      fReleaseCurrHRow = TRUE;
      fInvalidateFetchBuffer = TRUE;
      break;

    case DBREASON_ROWPOSITION_CHAPTERCHANGED: // 16  (ver 1.5)
      fpFreeFetchArray();              // Fetch array is no longer valid
      // Release the current row but keep the current chapter.
      fpRequeryCurrHRow(FPDB_RELEASE_ONLY | FPDB_PRESERVE_CHAPTER );
      fInvalidateFetchBuffer = TRUE;
      break;

  }

  // If flag was set, release the member var which is holding the
  // current row's handle (HROW).
  if (fReleaseCurrHRow)
    /* RFW - 2/22/05 - 15391
    fpRequeryCurrHRow(FPDB_RELEASE_ONLY);
    */
    fpRequeryCurrHRow(FPDB_RELEASE_ONLY | FPDB_PRESERVE_CHAPTER);

  // If flag was set, invalidate the current fetch buffer.
  if (fInvalidateFetchBuffer)
    fpCurrentRowChanged(FLAGC_DATACHANGED, FLAGC_DATACHANGED);

  // If ignoring "SYNC AFTER" events, then cancel event and return.
  if (m_iNotifyStatus & NOTIFY_SYNCAFTER_IGNORED)
    *pfQueueEvent = FPDB_EVENT_CANCEL_MSG;

  return hr;
}
