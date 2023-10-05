/*************************************************************************\
* FPDBNOTE.H                                                              *
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

//
// fpdbnote.h : Declaration of the CfpDBNotify class.
//
//             FarPoint's implementation of a class used for 
//             DB Binding in OLE controls.  It keeps a history of
//             DB notification messages received for the DataSource.
//
//             This class is intented to be a private class used by
//             the class: "CfpOleBinding".
//

#ifndef FPDBNOTE_H
#define FPDBNOTE_H

#ifndef __OCDB_H_
#include <ocdbid.h>
#include <olebind.h>
#include <ocdb.h>
#endif

#include "fpdbutil.h"

// User-defined DBREASONs to trigger special VBM_DATA messages 
// not represented in the current list of DBREASONs.
#define UDBREASON_INIT_CONNECT   (DBREASON_EDIT + 100)
#define UDBREASON_DISCONNECT     (DBREASON_EDIT + 101)
#define UDBREASON_SOURCECHANGED  (DBREASON_EDIT + 102)
#define UDBREASON_SELF_REFRESH   (DBREASON_EDIT + 103)
#if VC8
#define DBREASON_READDATA 33
#endif

//-------------------------------------------------------------------------
// Internal Class - CfpDBNotify
//-------------------------------------------------------------------------
class CfpDBNotify : public CObject
{
// Construction
public:
	CfpDBNotify(void);
	virtual ~CfpDBNotify(void);

// Member functions (methods):
public:
    void FlushInternalFlags();
    void SyncBefore_InternalFlags( DWORD dwEventWhat, ULONG cReasons, 
           DBNOTIFYREASON rgReasons[], BOOL FAR *processList);
    void SyncAfter_InternalFlags( DWORD dwEventWhat, ULONG cReasons, 
           DBNOTIFYREASON rgReasons[], BOOL FAR *processList);
    BOOL WasCurrentRowDeleted();

protected:
   // Action [Add/Remove/Find] for bookmark in "UpdatePending" list
   void     FlagSetColumn( LBLOCK Bmark, BOOL fCurrRow);
   BOOL     RemoveBookmark( LBLOCK Bmark, BOOL fCurrRow);
   BOOL     AddBookmark( LBLOCK Bmark, BOOL fCurrRow);
   POSITION FindBookmark( LBLOCK Bmark);

// Member variables:
protected:
   // flag that SetColumn() was called for current row
   int      m_iUpdatePendingCurrRow; 
   BOOL     m_fSetColumnCurrRow; 

   // List of Non-current bookmarks with SetColumn() called       
   CObList  m_UpdatePendingList;

   // SET after current row is deleted, RESET after a move operation
   BOOL     m_fRowDeleted;
   // If a move is pending, don't let Modified thru until move has ended.
   BOOL     m_fMovePending;        

};

#endif  // ifndef FPDBNOTE_H
