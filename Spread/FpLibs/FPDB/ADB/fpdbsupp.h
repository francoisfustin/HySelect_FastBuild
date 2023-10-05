/*************************************************************************\
* FPDBSUPP.H                                                               *
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
#ifndef FPDBSUPP_H
#define FPDBSUPP_H

#ifndef VC8
#include <adbctl.h>
#endif

// Forward declarations
class CfpDBColumn;

// This define can be used in place of an index when indicating
// the default accessor on the accessor functions.
#define FPDB_DEFAULT_ACCESSOR -1 


#define FPDB_VALUESET         0x01 // Value is set (needed?)
#define FPDB_STREAMACCESSOR   0x02 // Col uses its own stream accessor
#define FPDB_HASSTREAMDATA    0x04 // Col has stream data
#define FPDB_ACCESSORCREATED  0x08 // Accessor was created for col

typedef struct _FP_COLUMNITEM
{
  UINT  colIndex;
  ULONG ordinalId;
  ULONG flags;
  HACCESSOR hAccessor;  // accessor for a specific column

} FP_COLUMNITEM, *PFP_COLUMNITEM;

#define FPDB_DBCOL_CHUNK  10     // when "Adding" elements, allocate chunks

class CfpDBColList
{
public:
  CfpDBColList(void);

  virtual ~CfpDBColList(void);

#define FPDB_TRIM_EXTRA  TRUE     // "fTrimExtra" parameter
#define FPDB_DONT_TRIM   FALSE    // "fTrimExtra" parameter
  virtual BOOL SetSize(UINT cElements, BOOL fTrimExtra = FALSE);

  virtual void Clear(void);

  UINT GetSize(void);

  operator UINT() const;
  operator int() const;

  FP_COLUMNITEM& operator[](int index);
  
#define FPDB_FIND_INDEX   TRUE     // "fFindIndex" parameter
#define FPDB_FORCE_ADD    FALSE    // "fFindIndex" parameter
  virtual int Add(UINT colIndex, BOOL fFindIndex = FALSE);
  virtual int AddWithFlags(UINT colIndex, ULONG ulFlags, BOOL fFindIndex = FALSE);
  virtual int FindColIndex(UINT colIndex);
  virtual int GetColIndexForOrdinalId(ULONG ulOrdId);

  virtual BOOL IsEmpty(void);


  // Given the index, [i], set the accesssor, index may be -1.
  virtual BOOL SetAccessor(int index, HACCESSOR hAccessor);

  // Given the "colIndex", this function will find the "index" for
  // the specified "colIndex" and will set the accessor for that index.
  // "colIndex" may be -1.
  virtual BOOL SetAccessorForColIndex(int colIndex, HACCESSOR hAccessor);

  virtual BOOL HasValidAccessor(void);

  // Given the index, [i], returns the accesssor, index may be -1.
  virtual HACCESSOR GetAccessor(int index);

  // Given the index, [i], release the accesssor, index may be -1.
  virtual HRESULT ReleaseAccessor(int index, CRowset *pCRowset);
  virtual void    ReleaseAllAccessors(CRowset *pCRowset);

  virtual UINT    GetStmAccessorCt(void);
  virtual HRESULT SetData(CRowset *pCRowset, HROW hCurrRow, LPSTR lpszSetDataBuffer,
                    BOOL fBatchUpdate);

  virtual BOOL    HasStreamData(void);          // Does it have stream data in a column
  virtual PULONG  GetFlagsPtr(void);

  virtual BOOL    CompareList(CfpDBColList *pOther);

protected:
  UINT            m_cList;    // number of elements in list
  FP_COLUMNITEM  *m_aList;    // list

  UINT            m_cAllocated; // total number of elements allocated.

  HACCESSOR       m_hDefAccessor; // default accessor  
  UINT            m_cAccessorCt;  // num of cols with their own accessor

  // Flags about the list. The class maintains the FPDB_HASSTREAMDATA flag.
  ULONG           m_ulFlags;
};

//***********************************************************************
// DB Notify Event.  This structure is used to queue notification events.
//***********************************************************************
typedef struct _FP_DBEVENT
{
  DBEVENTPHASE    ePhase;     // DB Phase (ex. OkToDo, DidEvent)
  DBREASON        eReason;    // DB Reason (ex. RowInsert, RowChange)
  ULONG           cRows;      // Num of rows in row list.

  // NOTE 1: If cRows is 1, then .rghRows points to .hRow, else it's
  //         allocated memory which must be free when finished with the event.
  // NOTE 2: If rows > 0, then the handles must be released with 
  //         CRowset::ReleaseRows().
  HROW           *rghRows;    // Points to list or HROWs.
  HROW            hRow;       // If only 1 row, then .rghRows points here.

  ULONG          *rgCols;     // Ptr to array of col id's (index 0 is the count)

  ULONG           flags;      // Flags, see below

#define FP_DBEVENT_FREEABLE    0x0001  // May be freed individually.
#define FP_DBEVENT_RELEASEROWS 0x0002  // Rows need to be released.

  struct _FP_DBEVENT *pNext;  // Next item in the list.

} FP_DBEVENT, *LPFP_DBEVENT;

//***********************************************************************
// Class for maintaining DB Notify Event queue.
//***********************************************************************
class CfpDBEventQueue
{
  // Public Member functions.
public:
  CfpDBEventQueue(void);

  virtual ~CfpDBEventQueue(void);

  ULONG GetEventCount(void);  // Number of events pending.

  // Resets the event queue.
  void ResetQueue(CRowset *pCRowset, BOOL fPreAllocate = TRUE);

  // Adds an event to the queue.
  HRESULT AddEvent(DBEVENTPHASE ePhase, DBREASON eReason, ULONG cRows, 
    const HROW *rghRows, const ULONG *rgCols, CRowset *pCRowset);

  // Gets the next event from the queue and removes it from the queue.
  FP_DBEVENT* GetNextEvent(void);

  // Gets the next event from the queue WITHOUT removing it.
  FP_DBEVENT* PeekNextEvent(void);

  // Frees the event (dependent on its flags).
  HRESULT FreeEvent(FP_DBEVENT *pEvent, CRowset *pCRowset);

  // Public Member variable
  BOOL m_fMsgPosted;          // Whether there is currently a msg posted.

  // Protected Member Functions
protected:

  // Gets an empty event record (which is now part of the queue).
  FP_DBEVENT *GetEmptyEvent(void);

  // This trims the list of free events to FP_DBEVENT_FREELIST_OPTM count.
  void TrimFreeList(void);

  // Protected Member variables.
protected:

  FP_DBEVENT *m_pEvents;        // Current queue of events.
  ULONG       m_cEventCt;       // Num of events in the queue.
  FP_DBEVENT *m_pLastEvent;     // Last event in the queue.

  FP_DBEVENT *m_pFreeEvents;    // List of unused events.
  ULONG       m_cFreeCt;        // Num of events in the free list.

  FP_DBEVENT *m_pPreAllocEvents;// Block of pre-allocated events.
  ULONG       m_cPreAllocCt;    // Num of events in the pre-allocated list.

#define FP_DBEVENT_PREALLOC_CT  20
#define FP_DBEVENT_FREELIST_MAX   (FP_DBEVENT_PREALLOC_CT + 20)
#define FP_DBEVENT_FREELIST_OPTM  FP_DBEVENT_PREALLOC_CT

};


//***********************************************************************
// This structure is used for inserted row which have not been
// "commited" (updated).
//***********************************************************************
typedef struct _FP_DBINSROW
{
  HROW       hRow;            // Row handle of new row.
  BOOL       fRelRef;         // Whether to release the reference.
  CBookmark *bmkTmp;          // Temporary bookmark (that we create).

  struct _FP_DBINSROW *pNext;

} FP_DBINSROW, *LPFP_DBINSROW;


class CfpDbInsRowCls
{
public:
  CfpDbInsRowCls(void);

  virtual ~CfpDbInsRowCls(void);

  void ClearList(CRowset *pCRowset = NULL);
  BOOL IsEmpty(void);

#define FPDB_ADDREF TRUE
  BOOL AddRow(HROW hRow, BOOL fAddRef = FALSE, CRowset *pCRowset = NULL);

  BOOL FindRow(HROW hRow, CBookmark *pRetBmk);
  BOOL GetLastBookmark(CBookmark *pRetBmk);
  BOOL RemoveRow(HROW hRow, CRowset *pCRowset = NULL);

protected:

  void FreeElement(FP_DBINSROW *pElem, CRowset *pCRowset = NULL);

  UINT         m_uCt;
  FP_DBINSROW *m_FirstItem;
  FP_DBINSROW *m_LastItem;
  ULONG        m_uNextId;
};

//***********************************************************************
// Our own implementation of IStream because OLE's IStream does not
// support the ISequentialStream interface when queried--ours will!
//***********************************************************************
class CfpDbStream : public IStream
{
public:

  // Constructor which requires a valid IStream ptr
  CfpDbStream(IStream *pstm);

  virtual ~CfpDbStream(void);  // destructor

protected:
  IStream *m_pStm;
  ULONG    m_ulRefCt;

public:
  // *****************************************************************
  // * IUnknown functions:
  // *****************************************************************
  virtual HRESULT STDMETHODCALLTYPE QueryInterface( REFIID riid,
      void **ppvObject);
  
  virtual ULONG STDMETHODCALLTYPE AddRef( void);
  virtual ULONG STDMETHODCALLTYPE Release( void);

  // *****************************************************************
  // * ISequentialStream functions:
  // *****************************************************************
  virtual HRESULT STDMETHODCALLTYPE Read( 
      void *pv,
      ULONG cb,
      ULONG *pcbRead);
  
  virtual HRESULT STDMETHODCALLTYPE Write( 
      const void *pv,
      ULONG cb,
      ULONG *pcbWritten);
        
  // *****************************************************************
  // * IStream functions:
  // *****************************************************************
  virtual  HRESULT STDMETHODCALLTYPE Seek( 
       LARGE_INTEGER dlibMove,
       DWORD dwOrigin,
       ULARGE_INTEGER *plibNewPosition);
  
  virtual HRESULT STDMETHODCALLTYPE SetSize(ULARGE_INTEGER libNewSize);
  
  virtual  HRESULT STDMETHODCALLTYPE CopyTo( 
      IStream *pstm,
       ULARGE_INTEGER cb,
       ULARGE_INTEGER *pcbRead,
       ULARGE_INTEGER *pcbWritten);
  
  virtual HRESULT STDMETHODCALLTYPE Commit(DWORD grfCommitFlags);
  virtual HRESULT STDMETHODCALLTYPE Revert( void);

  virtual HRESULT STDMETHODCALLTYPE LockRegion( 
       ULARGE_INTEGER libOffset,
       ULARGE_INTEGER cb,
       DWORD dwLockType);
  
  virtual HRESULT STDMETHODCALLTYPE UnlockRegion( 
       ULARGE_INTEGER libOffset,
       ULARGE_INTEGER cb,
       DWORD dwLockType);
  
  virtual HRESULT STDMETHODCALLTYPE Stat( 
       STATSTG *pstatstg,
       DWORD grfStatFlag);
  
  virtual HRESULT STDMETHODCALLTYPE Clone(
       IStream **ppstm);
  
};


#endif