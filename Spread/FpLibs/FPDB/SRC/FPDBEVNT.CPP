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
#if !defined(NO_MFC)
#include <afxctl.h>         // MFC support for OLE Custom Controls
#else  //NO_MFC
#include "stdafx.h"
#include "fpatl.h"
#endif

#ifdef SS40
#include "ss40dao.h"
#endif

//#include "stdafx.h"
#include "vbinterf.h"

#include "fpconvrt.h"

//#define DBINITCONSTANTS		         // These cause variable declarations
//#define INITGUID
//#include <initguid.h>

#include "fpdbutil.h"		  
#include "fpdbnote.h"		  

#include "fpdbcol.h"
#include "fpoledb.h"		  

#include <limits.h>

#define DB_VT_SAFEARRAY   0x2011
#define BMKMAX 256

// This will get translated to VBM_DATA_INITIATE
#define DATA_UINIT_CONNECT       300

// For some reason, this isn't defined for WIN16
#ifndef WIN32
#define METHOD_PROLOGUE_EX(theClass, localClass) \
	METHOD_PROLOGUE(theClass, localClass)
#endif

// if not VC40, define ...
#ifdef SS40

#define METHOD_PROLOGUE(theClass, localClass) \
	theClass* pThis = \
		((theClass*)((BYTE*)this - offsetof(theClass, m_x##localClass))); \
	AFX_MANAGE_STATE(pThis->m_pModuleState) \
	pThis; // avoid warning from compiler 

#define METHOD_PROLOGUE_ METHOD_PROLOGUE

#define AFX_MANAGE_STATE(p)

#elif (!defined(METHOD_PROLOGUE_) && !defined(NO_MFC))
#define METHOD_PROLOGUE_ METHOD_PROLOGUE
#endif

#if defined(SS40) || !defined(NO_MFC) // MFC
STDMETHODIMP_(ULONG) CfpOleBinding::XNotifyDBEvents::AddRef()
{
//	METHOD_MANAGE_STATE(CfpOleBinding, NotifyDBEvents)
	METHOD_PROLOGUE_(CfpOleBinding, NotifyDBEvents)
#ifdef SS40
  return (ULONG)DataManager_ExternalAddRef(pThis->m_lpOwner);
#else
	return (ULONG)pThis->m_lpOwner->ExternalAddRef();
#endif
}

STDMETHODIMP_(ULONG) CfpOleBinding::XNotifyDBEvents::Release()
{
//	METHOD_MANAGE_STATE(CfpOleBinding, NotifyDBEvents)
	METHOD_PROLOGUE_(CfpOleBinding, NotifyDBEvents)
#ifdef SS40
  return (ULONG)DataManager_ExternalRelease(pThis->m_lpOwner);
#else
	return (ULONG)pThis->m_lpOwner->ExternalRelease();
#endif
}

STDMETHODIMP CfpOleBinding::XNotifyDBEvents::QueryInterface(REFIID iid, LPVOID FAR *ppvObj)
{
//	METHOD_MANAGE_STATE(CfpOleBinding, NotifyDBEvents)
	METHOD_PROLOGUE_(CfpOleBinding, NotifyDBEvents)
	if (/* IsEqualIID(iid, IID_IUnknown) || */
		IsEqualIID(iid, IID_INotifyDBEvents))
	{
		*ppvObj = this;								 
		AddRef();
		return NOERROR;
	}
	
	return ResultFromScode(E_NOINTERFACE);
}
#endif // MFC

STDMETHODIMP FPDB_IMPL_FUNC(CfpOleBinding, XNotifyDBEvents)OKToDo( 
  DWORD dwEventWhat, ULONG cReasons, DBNOTIFYREASON rgReasons[])
{
  return S_OK;
}
STDMETHODIMP FPDB_IMPL_FUNC(CfpOleBinding, XNotifyDBEvents)Cancelled( 
  DWORD dwEventWhat, ULONG cReasons, DBNOTIFYREASON rgReasons[])
{
  return S_OK;
}

/***************************************************************************
 *
 * FUNCTION:  CfpOleBinding::XNotifyDBEvents::SyncBefore()
 *
 * DESCRIPTION:
 *
 *   This function is called (notified) BEFORE something is about to
 *   happen with this Cursor.  At this point, we have already OK'd the
 *   action.  In reaction to this notification, we simply set some
 *   flags with m_NotifyPreprocessor.SyncBefore_InternalFlags().
 *
 *   The real processing is handled in the "AboutToDo" event.
 *
 *   05/23/96 NOTE: On this date, the code was changed so that the real
 *                  processing is in AboutToDo instead of SyncBefore.
 *
 * PARAMETERS:
 *
 * RETURNS:
 *
 **************************************************************************/
STDMETHODIMP FPDB_IMPL_FUNC(CfpOleBinding, XNotifyDBEvents)SyncBefore( 
  DWORD dwEventWhat, ULONG cReasons, DBNOTIFYREASON rgReasons[])
{
#define SB_MAX_REASONS 5
  BOOL  processList[SB_MAX_REASONS];
  BOOL  fIgnoreEvent;

  METHOD_PROLOGUE_(CfpOleBinding, NotifyDBEvents);

  // make sure there are not more Reason than we were coded for:
  ASSERT( cReasons <= (ULONG)SB_MAX_REASONS);
  if (cReasons > (ULONG)SB_MAX_REASONS)
    cReasons = SB_MAX_REASONS;

  fIgnoreEvent = ((pThis->m_iNotifyStatus & NOTIFY_SYNCBEFORE_IGNORED) != 0);

  // For DEBUG only: add to list
  ASSERT(pThis->m_DBEventList->Add("SB", cReasons, (LPVOID)rgReasons,
    fIgnoreEvent));

  // Set internal flags related to this message
  pThis->m_NotifyPreprocessor.SyncBefore_InternalFlags(dwEventWhat, 
    cReasons, rgReasons, processList);
  
  return S_OK;
}


/***************************************************************************
 *
 * FUNCTION:  CfpOleBinding::XNotifyDBEvents::AboutToDo()
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
STDMETHODIMP FPDB_IMPL_FUNC(CfpOleBinding, XNotifyDBEvents)AboutToDo( 
  DWORD dwEventWhat, ULONG cReasons, DBNOTIFYREASON rgReasons[])
{
#define AB_MAX_REASONS 5
  DATAACCESS da = {0};
  SHORT actionArray[AB_MAX_REASONS];
  SHORT actionCt;
  LRESULT lResult;
  BOOL  fUpdateAlreadyInProgress = FALSE;
  BOOL  processList[AB_MAX_REASONS];

  for (int i = 0; i < AB_MAX_REASONS; i++)
    processList[i] = TRUE;

  METHOD_PROLOGUE_(CfpOleBinding, NotifyDBEvents)

  // make sure there are not more Reason than we were coded for:
  ASSERT( cReasons <= (ULONG)AB_MAX_REASONS);
  if (cReasons > (ULONG)AB_MAX_REASONS)
    cReasons = AB_MAX_REASONS;

  // For DEBUG only: add to list
  ASSERT(pThis->m_DBEventList->Add("AB", cReasons, (LPVOID)rgReasons,
    (pThis->m_iNotifyStatus & NOTIFY_SYNCBEFORE_IGNORED) != 0));

  // If we are no longer "under advisement", OR
  // if nothing changed, return OK
  if ((pThis->m_iNotifyStatus & NOTIFY_SYNCBEFORE_IGNORED) ||
      (!(pThis->m_ulCtlFlags & CTLFLG_DATACHANGED)))
     return S_OK;

  // Translate the DBReasons for "AboutToDo" into VB DATA_xxxx messages
  actionCt = this->TranslateAboutToDoDBReasons( dwEventWhat, cReasons, 
               rgReasons, AB_MAX_REASONS, &actionArray[0], processList);

  if (actionCt > 0)
  {
    // suspend advising
    pThis->m_iNotifyStatus |= NOTIFY_SYNCBEFORE_IGNORED;

    // Determine if an EDIT or ADD has already started.  If (EDIT or ADD) &&
    // SETCOLUMN 
    if ((pThis->m_iUpdateStatus & (UPDSTAT_BEGIN_EDIT | UPDSTAT_BEGIN_ADD)) &&
        (pThis->m_iUpdateStatus & UPDSTAT_SETCOLUMN))
       fUpdateAlreadyInProgress = TRUE;

    da.sAction = actionArray[0];
    da.lpfpOleBinding = this;

    // *** VB3 Feature:  For INSERT ROW, VB3 does NOT send a VBM_DATA_REQUEST,
    // *** but it will send a DATA_AVAILABLE (via SyncAfter).
    if (da.sAction == DATA_ADDNEW)
       da.sAction = (SHORT)-1;
  
    // Call the owner's function to process the VBM_DATA message
    if (da.sAction != (SHORT)-1)
    {
       ASSERT(pThis->m_lpfuncDataMsg);//Callback func MUST BE SET BY OWNER!
       if (pThis->m_lpfuncDataMsg)
          lResult = pThis->m_lpfuncDataMsg( (LPVOID)pThis->m_lpOwner, 
            VBM_DATA_REQUEST, (WPARAM)0, (LPARAM)(LPVOID)&da);
    }

    // If above function "caused" an update to start, finish the update
    // Q: Should we check that (m_ulCtlFlags & CTLFLG_DATACHANGED) == 0 ?
    if (!fUpdateAlreadyInProgress &&
        (pThis->m_iUpdateStatus & (UPDSTAT_BEGIN_EDIT | UPDSTAT_BEGIN_ADD)))
    {
       if (!pThis->FinishUpdatingRow())  // function resets flags
          pThis->fpInvokeError();
    }
    // Resume advising
    pThis->m_iNotifyStatus &= ~NOTIFY_SYNCBEFORE_IGNORED;
  }

  return S_OK;
}



STDMETHODIMP FPDB_IMPL_FUNC(CfpOleBinding, XNotifyDBEvents)FailedToDo( 
  DWORD dwEventWhat, ULONG cReasons, DBNOTIFYREASON rgReasons[])
{
#ifdef _DEBUG
  METHOD_PROLOGUE_(CfpOleBinding, NotifyDBEvents)
  ASSERT(pThis->m_DBEventList->Add("F!", cReasons, (LPVOID)rgReasons));
#endif
  return S_OK;
}

/***************************************************************************
 *
 * FUNCTION:  CfpOleBinding::XNotifyDBEvents::SyncAfter()
 *
 * DESCRIPTION:
 *
 * PARAMETERS:
 *
 * RETURNS:
 *
 **************************************************************************/
STDMETHODIMP FPDB_IMPL_FUNC(CfpOleBinding, XNotifyDBEvents)SyncAfter( 
  DWORD dwEventWhat, ULONG cReasons, DBNOTIFYREASON rgReasons[])
{
#define SA_MAX_REASONS 5
  DATAACCESS da = {0};
  SHORT actionArray[SA_MAX_REASONS];
  SHORT actionCt;
  SHORT index;
  UINT  uDataMsg;
  LRESULT lResult;
  HLSTR hlstrNewRowBmark = NULL;
//  UINT  uFlags;
  DWORD dwCurrRowFlag;
  DWORD dwMask;
  BOOL  processList[SA_MAX_REASONS];
  BOOL  fIgnoreEvent;

//	METHOD_MANAGE_STATE(CfpOleBinding, NotifyDBEvents)
  METHOD_PROLOGUE_(CfpOleBinding, NotifyDBEvents)

  // make sure there are not more Reason than we were coded for:
  ASSERT( cReasons <= (ULONG)SA_MAX_REASONS);
  if (cReasons > (ULONG)SA_MAX_REASONS)
    cReasons = SA_MAX_REASONS;

  fIgnoreEvent = ((pThis->m_iNotifyStatus & NOTIFY_SYNCAFTER_IGNORED) != 0);

  // For DEBUG only: add to list
  ASSERT(pThis->m_DBEventList->Add("SA", cReasons, (LPVOID)rgReasons,
    fIgnoreEvent));

  // Set internal flags related to this message
  pThis->m_NotifyPreprocessor.SyncAfter_InternalFlags(dwEventWhat, 
    cReasons, rgReasons, processList);

  // If we are no longer "under advisement", just return
  if (fIgnoreEvent)
     return S_OK;

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
     pThis->CurrentRowChanged(dwMask, dwCurrRowFlag);

  // Translate the DBReasons for "SyncAfter" into VB DATA_xxxx messages
  actionCt = this->TranslateSyncAfterDBReasons( dwEventWhat, cReasons, 
               rgReasons, SA_MAX_REASONS, &actionArray[0], processList,
               &hlstrNewRowBmark);

  // suspend advising
  pThis->m_iNotifyStatus |= NOTIFY_SYNCAFTER_IGNORED;

  // if we have "actions" which we care about...
  index = 0;
  da.lpfpOleBinding = this;
  while (index < actionCt)
  {
    da.sAction = actionArray[index];

    if (da.sAction == DATA_DELETE)
    {
      // if the current record just got deleted...
      if (dwEventWhat & (DWORD)DBEVENT_CURRENT_ROW_DATA_CHANGED)
         pThis->CurrentRowChanged(FLAG_CURR_ROW_INVALID, FLAG_CURR_ROW_INVALID);
    }

    // Pass the new row's bookmark in da.hlstrBookMark
    if (hlstrNewRowBmark)
       da.hlstrBookMark = hlstrNewRowBmark;

    // if action is still valid, send VBM_DATA... msg
    if (da.sAction != (SHORT)-1)
    {
       // special processing of VBM_DATA_INITIATE
       if (da.sAction == DATA_UINIT_CONNECT)
          uDataMsg = VBM_DATA_INITIATE;
       else
          uDataMsg = VBM_DATA_AVAILABLE;
       // Call the owner's function to process the VBM_DATA message
       ASSERT(pThis->m_lpfuncDataMsg);//Callback func MUST BE SET BY OWNER!
       if (pThis->m_lpfuncDataMsg)
          lResult = pThis->m_lpfuncDataMsg( (LPVOID)pThis->m_lpOwner, 
            uDataMsg, (WPARAM)0, (LPARAM)(LPVOID)&da);
    }

    // If we have a bookmark and this is the last action, free the bookmark.
    if (hlstrNewRowBmark && (index+1) == actionCt)
    {
       LBlockFree((LBLOCK)hlstrNewRowBmark);
       hlstrNewRowBmark = NULL;
    }
    ++index;
  }

  // Resume advising
  pThis->m_iNotifyStatus &= ~NOTIFY_SYNCAFTER_IGNORED;

  return S_OK;
}

STDMETHODIMP FPDB_IMPL_FUNC(CfpOleBinding, XNotifyDBEvents)DidEvent( 
  DWORD dwEventWhat, ULONG cReasons, DBNOTIFYREASON rgReasons[])
{
  return S_OK;
}

/***************************************************************************
 *
 * FUNCTION:  CfpOleBinding::XNotifyDBEvents::TranslateAboutToDoDBReasons()
 *
 * DESCRIPTION:
 *
 *   Translates the DBNOTIFYREASONs to DATAACCESS.sAction (ex: DATA_REFRESH)
 *   for "SyncBefore" notification messages.  These can cause us to send
 *   a VBM_DATA_REQUEST message to the Control.
 *
 *   "sAction" codes are returned in passed array and returns the 
 *   number of codes put in rgActionArray.  NOTE:  ONLY ONE VBM_DATA_REQUEST
 *   message is sent regardless of the number of reasons.
 *
 * PARAMETERS:
 *
 * RETURNS:
 *
 *   Number of reasons translated.
 *
 **************************************************************************/
SHORT FPDB_IMPL_FUNC(CfpOleBinding, XNotifyDBEvents)TranslateAboutToDoDBReasons( 
  DWORD dwEventWhat, ULONG cReasons, DBNOTIFYREASON FAR *rgReasons, 
  USHORT uActionSize, SHORT FAR *rgActionArray, BOOL FAR *processList)
{
  SHORT sActionCt = 0;
  SHORT sAction;
  ULONG index;

  index = 0;
  while ((index < cReasons) && (sActionCt < (SHORT)uActionSize))
  {
    sAction = 0;
    // If message is "allowed" to be processed...
    if (processList[index])
    {
      switch (rgReasons[index].dwReason)
      {
        // BeginUpdate was called (w/DBROWACTION_ADD)
        case DBREASON_ADDNEW:
// VB3 does not send these
//        sAction = DATA_ADDNEW;       
          break;

        // Database is closing
        case DBREASON_CLOSE:
          sAction = DATA_CLOSE;
          break;

        // BeginUpdate was called (w/DBROWACTION_UPDATE)
        case DBREASON_EDIT:
          sAction = DATA_UPDATE;
          break;

        case DBREASON_FIND:
          sAction = DATA_FINDFIRST;
          break;

        // Current row has moved to a different row
        case DBREASON_MOVE:  
        case DBREASON_MOVEPERCENT:
        case DBREASON_MOVEDINCURSOR:  // row updated and its position moved
        case DBREASON_SEEK: 
          sAction = DATA_BOOKMARK;
          break;

        case DBREASON_FILTERCHANGED:
        case DBREASON_FINDCRITERIACHANGED:
        case DBREASON_NEWINDEX:       // index of query that created the
                                      // cursor was changed
        case DBREASON_NEWPARAMETERS:
        case DBREASON_QUERYSPECCHANGED:
        case DBREASON_RECALC:
//      case DBREASON_REFRESH:      // Ignoring DAO's message
        case DBREASON_ROLLBACK:
        case DBREASON_SETRANGECHANGED:
        case DBREASON_SORTCHANGED:
        case UDBREASON_SOURCECHANGED:
        case UDBREASON_SELF_REFRESH:
          sAction = DATA_REFRESH;
          break;

        case UDBREASON_DISCONNECT:      // DBREASON_EDIT+101
	    case DBREASON_UNLOAD:
          sAction = DATA_UNLOAD;
          break;

        // Ignore these reasons
        case DBREASON_DELETED:  
        case DBREASON_REMOVEDFROMCURSOR:  
        case DBREASON_ROWFIXUP:
        case DBREASON_INSERTED:         // Update called to complete ADDNEW
        case DBREASON_MODIFIED:         // Update called to complete UPDATE
        case DBREASON_PERCENT:
        case DBREASON_BEGINTRANSACT:
        case DBREASON_COMMIT:  
        case DBREASON_BULK_ERROR:
        case DBREASON_BULK_NOTTRANSACTABLE:
        case DBREASON_BULK_ABOUTTOEXECUTE:
        case DBREASON_CANCELUPDATE:
        case DBREASON_SETCOLUMN:
        case UDBREASON_INIT_CONNECT:    // DBREASON_EDIT+100
        case DBREASON_READDATA:
        default:  
             break;
      }
    } // if processList[i]

    // Add Action to return array
    if (sAction)
    {
      rgActionArray[sActionCt] = sAction;
      ++sActionCt;
    }
    ++index;  // next reason
  }

  return sActionCt;
}


/***************************************************************************
 *
 * FUNCTION:  CfpOleBinding::XNotifyDBEvents::TranslateSyncAfterDBReasons()
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
SHORT FPDB_IMPL_FUNC(CfpOleBinding, XNotifyDBEvents)TranslateSyncAfterDBReasons( 
  DWORD dwEventWhat, ULONG cReasons, DBNOTIFYREASON FAR *rgReasons, 
  USHORT uActionSize, SHORT FAR *rgActionArray, BOOL FAR *processList,
  LPVOID FAR *lphlstrBmark)
{
  SHORT sActionCt = 0;
  SHORT sAction;
  ULONG index;

  METHOD_PROLOGUE_(CfpOleBinding, NotifyDBEvents)
  
  *lphlstrBmark = NULL;

  index = 0;
  while ((index < cReasons) && (sActionCt < (SHORT)uActionSize))
  {
    sAction = 0;
    // If message is "allowed" to be processed...
    if (processList[index])
    {
      switch (rgReasons[index].dwReason)
      {
        // Current row deleted
        case DBREASON_DELETED:  
        case DBREASON_REMOVEDFROMCURSOR:
          if (rgReasons[index].arg1.vt == (VARTYPE)DB_VT_SAFEARRAY)
          {
             CopySafeArrayToLBlock(rgReasons[index].arg1.parray, 
               (LBLOCK FAR *)lphlstrBmark);
             sAction = DATA_DELETE;
          }
          else
          {
//           if (dwEventWhat & (DWORD)DBEVENT_CURRENT_ROW_CHANGED)
//              sAction = DATA_DELETE;
//           else
//              sAction = DATA_REFRESH;
             sAction = DATA_DELETE;
          }
          break;

        // Row inserted at current
        case DBREASON_INSERTED: 
          if (rgReasons[index].arg1.vt == (VARTYPE)DB_VT_SAFEARRAY)
             CopySafeArrayToLBlock(rgReasons[index].arg1.parray, 
               (LBLOCK FAR *)lphlstrBmark);
          // Put 2 actions in the action array (ADDNEW & UPDATE)
          rgActionArray[sActionCt] = DATA_ADDNEW;
          ++sActionCt;
          sAction = DATA_UPDATE;
          break;

        // Data in current row changed
        case DBREASON_MODIFIED:  
        case DBREASON_ROWFIXUP:
          if (rgReasons[index].arg1.vt == (VARTYPE)DB_VT_SAFEARRAY)
          {
             CopySafeArrayToLBlock(rgReasons[index].arg1.parray, 
               (LBLOCK FAR *)lphlstrBmark);
             sAction = DATA_UPDATE;
          }
          else
          {
//             if (dwEventWhat & (DWORD)DBEVENT_CURRENT_ROW_DATA_CHANGED)
//                sAction = DATA_UPDATE;
//             else
//                sAction = DATA_REFRESH;
               sAction = DATA_UPDATE;
          }
          break;

        case DBREASON_READDATA:
          sAction = DATA_UPDATE;
          break;

        // Reasons to re-query ALL records
        case DBREASON_MOVEDINCURSOR:
        case DBREASON_NEWINDEX:
//      case DBREASON_REFRESH:      // Ignoring DAO's message
        case DBREASON_SORTCHANGED:
        case DBREASON_ROLLBACK:
        case UDBREASON_SOURCECHANGED:
        case UDBREASON_SELF_REFRESH:
          sAction = DATA_REFRESH;         // add "Move"
          break;

        // Reasons to re-query ALL records
        case DBREASON_RECALC:
        case DBREASON_NEWPARAMETERS:
        case DBREASON_FILTERCHANGED:
        case DBREASON_QUERYSPECCHANGED:
        case DBREASON_FINDCRITERIACHANGED:
        case DBREASON_SETRANGECHANGED:
          if (pThis->m_fRemoteDataCtl || pThis->m_ColInfoList.IsEmpty())
          {
            if (pThis->GetMetaData(NULL, NULL))
            {
              pThis->BindDataFieldColumn(pThis->m_strBoundDataField);
              sAction = DATA_REFRESH;
            }
          }
          else  
          {
            sAction = DATA_REFRESH;
          }
          break;

        // Current row has moved to a different row
        case DBREASON_MOVE:  
        case DBREASON_FIND:
        case DBREASON_MOVEPERCENT:
        case DBREASON_SEEK: 
          sAction = DATA_BOOKMARK;
          break;

        // Database was closed
        case DBREASON_CLOSE:
          sAction = DATA_CLOSE;
          break;

        // Special User-defined messages
        case UDBREASON_INIT_CONNECT:    // DBREASON_EDIT+100
          sAction = DATA_UINIT_CONNECT;
          break;

        case UDBREASON_DISCONNECT:      // DBREASON_EDIT+101
	    case DBREASON_UNLOAD:
          sAction = DATA_UNLOAD;
          break;

        // Ignore these reasons
        case DBREASON_PERCENT:
        case DBREASON_ADDNEW:
        case DBREASON_BEGINTRANSACT:
        case DBREASON_COMMIT:  
        case DBREASON_BULK_ERROR:
        case DBREASON_BULK_NOTTRANSACTABLE:
        case DBREASON_BULK_ABOUTTOEXECUTE:
        case DBREASON_CANCELUPDATE:
        case DBREASON_EDIT:
        case DBREASON_SETCOLUMN:
        default:  
          break;
      }
    }  //if processList[i]

    // Add Action to return array
    if (sAction)
    {
      rgActionArray[sActionCt] = sAction;
      ++sActionCt;
    }
    ++index;  // next reason
  }

  return sActionCt;
}

