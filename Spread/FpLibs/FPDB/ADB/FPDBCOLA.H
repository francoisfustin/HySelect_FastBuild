/*************************************************************************\
* FPDBCOLA.H                                                               *
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

//
// fpdbcola.h : Declaration of the CfpDBColumn class.
//
//             FarPoint's implementation of a class used for 
//             DB Binding in OLE controls.
//
//             This class is intented to be a private class used by
//             the class: "CfpOleBinding".
//

#ifndef FPDBCOLA_H
#define FPDBCOLA_H

#define NO_ADBCTL_PROPHELP
#ifndef VC8
#include <adbctl.h>
#endif

// Cursor's Column ID structure is now called DBID.
#define DBCOLUMNID  DBID

// Flags for CfpDBColumn.m_uFlags
#define DB_FIXEDFIELD      0x01
#define DB_VARIABLEFIELD   0x02
#define DB_AUTOINCRFIELD   0x10
#define DB_UPDATABLEFIELD  0x20
#define UDB_NULLABLE       0x80       // Added for OLE by SCP


//-------------------------------------------------------------------------
// Internal Class - CfpDBColumn
//-------------------------------------------------------------------------
class CfpDBColumn : public COLUMNINFO  // declare in "fpdbcol.h"
{
public:
  CfpDBColumn(); 
  ~CfpDBColumn(); 

  void Clear(void);

  CfpDBColumn& operator=(const CfpDBColumn& other);
  CfpDBColumn& operator=(const COLUMNINFO& other);

#define FPDB_BOOKMARK_COL  TRUE
  BOOL fpFillColBindRec(DBBINDING *pColBind, ULONG *pulInOffset, 
    LPBLOB *pBlobTmp, BOOL fBookmarkCol = FALSE);

  BOOL fpFillSetDataBinding(DBBINDING *pColBind, LPBLOB *pBlobTmp);

  HRESULT fpGetBookmark(LPSTR pszData, CBookmark *pBmk);
  DWORD   fpGetVBDataType(void);

#define FPDB_DONT_ZEROMEM  FALSE
  void    fpFreeFetchData(LPSTR pszData, BOOL fZeroMem = TRUE);
  void    fpPostFetchSetup(LPSTR pszData);

  ULONG CfpDBColumn::fpGetDataValue( LPSTR pszFetchBuff, USHORT usVBDataType,
    ULONG ulChunkOffset, ULONG ulChunkSize,
    LONG FAR *lplData, USHORT FAR *lpuFlags);

  ULONG fpGetNonStringValue( LPSTR pszGetData, USHORT usVBDataType,
    WORD wDataType, ULONG dwVarLen, LONG FAR *lplData);

  ULONG fpGetDBVariantValue( LPSTR pszGetData, USHORT usVBDataType,
      WORD wDataType, ULONG dwVarLen, LONG FAR *lplData);

  ULONG fpGetStreamValue( LPSTR pszGetData, USHORT usVBDataType,
    ULONG dwVarLen, ULONG ulChunkOffset, ULONG ulChunkSize, 
    LONG FAR *lplData, USHORT FAR *lpuFlags);

  ULONG fpGetStreamValueWithoutLen( LPSTR pszGetData, USHORT usVBDataType,
    ULONG dwVarLen, ULONG ulChunkOffset, ULONG ulChunkSize, LONG FAR *lplData, 
    USHORT FAR *lpuFlags);

  ULONG fpGetStringValue( LPSTR pszGetData, USHORT usVBDataType,
    WORD wDataType, ULONG ulChunkOffset, ULONG ulChunkSize,
    LONG FAR *lplData, DWORD dwVarLen);

  LONG fpSetDataValue( LPSTR pszSetBuffer, USHORT usVBDataType, 
    ULONG ulChunkOffset, ULONG ulChunkSize, LONG lData, UINT uFlags);

  LONG fpSetNonStringValue( LPSTR pszSetData, USHORT usVBDataType, 
    USHORT usDBtype, LONG lData);
  LONG fpSetStringValue( LPSTR pszSetBuffer, USHORT usVBDataType, 
    USHORT usDBtype, LONG lData);
  LONG fpSetStreamValue( LPSTR pszSetData, USHORT usVBDataType, 
    LONG lData, ULONG ulChunkOffset, ULONG ulChunkSize);

  void fpPrepareSetState(void);
  void fpClearSetState(LPSTR pszSetBuffer);
  BOOL fpIsStreamCol(void);       // if column uses streams
  BOOL fpHasStreamSetData(void);  // if column has data set in a stream
  BOOL fpNeedsOwnAccessor(void);

protected:
  void ClearOwnMembers(BOOL fInit = TRUE);
  void Init(void);

public:
  // Metadata column info
  LPTSTR tstrName;
  UINT   uMetaFlags;

  // Accessor used for reading/writing streams.
  HACCESSOR m_hReadAccessor;

protected:
  // Bound column info
  ULONG  m_obStatus;
  ULONG  m_obLength;
  ULONG  m_obValue;
  DBTYPE m_wTypeBind;
  DBPART m_dwPart;
  ULONG  m_cbMaxLen;

  BOOL   m_fBookmarkCol;

  ULONG  m_cbReadPos;      // Position after last read (if IUNKNOWN type)
  ULONG  m_ulStreamLen;

  // Post-read members      
  UINT  m_fNewRead;        // Read is new.  NOTE: Order does matter:
#define FPDB_COL_NOFETCH      0    // No fetch yet
#define FPDB_COL_FETCHFREED   1    // Fetch was freed by FreeFetchData()
#define FPDB_COL_POSTFETCH    2    // Fetch processed by PostFetchSetup()
#define FPDB_COL_BLOB_INFO    3    // Any blob info is now available

#define FPDB_DATA_SET         1
#define FPDB_DATA_ALLOCATED   2
#define FPDB_STREAM_CREATED   4
  UINT      m_fSet;
  VOID     *m_pSetData;
  LPSTREAM  m_pstmSet;

  DBTYPE m_wTypeBindSet;

  ISequentialStream *m_pSeqStmGet;  // SequentialStream for IUNKNOWN "Get"
};


#endif  // ifndef FPDBCOL_H
