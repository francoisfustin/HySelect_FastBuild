/*************************************************************************\
* FPDBCOL.H                                                               *
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
// fpdbcol.h : Declaration of the CfpDBColumn class.
//
//             FarPoint's implementation of a class used for 
//             DB Binding in OLE controls.
//
//             This class is intented to be a private class used by
//             the class: "CfpOleBinding".
//

#ifndef FPDBCOL_H
#define FPDBCOL_H

#ifndef __OCDB_H_
#include <ocdbid.h>
#include <olebind.h>
#include <ocdb.h>
#endif


//-------------------------------------------------------------------------
// Internal Class - CfpDBColumn
//-------------------------------------------------------------------------
class CfpDBColumn : public CObject
{
// Construction
public:
	CfpDBColumn();

// Flags for CfpDBColumn.m_uFlags
#define DB_FIXEDFIELD      0x01
#define DB_VARIABLEFIELD   0x02
#define DB_AUTOINCRFIELD   0x10
#define DB_UPDATABLEFIELD  0x20
#define UDB_NULLABLE       0x80       // Added for OLE by SCP

	CfpDBColumn( LPCTSTR szName, DBCOLUMNID ColID, ULONG ulType, 
	              ULONG ulMaxLen, DWORD dwBindType, int iIndex, 
	              UINT uFlags = (UINT)0);
    DWORD GetVBDataType(void);

    ULONG GetDataValue( LPSTR lpszFetchBuff, USHORT usVBDataType,
      ULONG ulChunkOffset, ULONG ulChunkSize,
      LONG FAR *lplData, UINT FAR *lpuFlags, ICursor FAR *lpCursor);

    ULONG GetDataValueFromRDC( LPSTR lpszFetchInline, 
      LPSTR lpszFetchOutline, USHORT usVBDataType,
      LONG FAR *lplData, UINT FAR *lpuFlags, ICursor FAR *lpCursor);

    LONG SetDataValue( DBBINDPARAMS FAR *lpBindParams, 
      USHORT usVBDataType, ULONG ulChunkOffset, ULONG ulChunkSize,
      LONG lData, UINT uFlags, ICursorUpdateARow FAR *lpUpdateCursor,
      BOOL FAR *lpbMemAllocated);

    LONG SetDataValueForRDC( DBBINDPARAMS FAR *lpBindParams, 
     USHORT usVBDataType,
     LONG lData, UINT uFlags, ICursorUpdateARow FAR *lpUpdateCursor,
     BOOL FAR *lpbMemAllocated);

    IStream FAR *BlobGetInterface( ICursor FAR *lpCursor);
//  IStream FAR *BlobGetWriteInterface( ICursorUpdateARow FAR *lpUpdateCursor);
    BOOL  BlobQuerySize(ICursor FAR *lpCursor);
    ULONG GetBlobDataValue( ICursor FAR *lpCursor, 
      LPVOID hlstrRetData, ULONG ulOffset, ULONG ulSize);
    void FreeFetchData( LPSTR lpszFetchBuff);
    void PostFetchSetup( LPSTR lpszFetchBuff);
    ULONG WriteDataValue( ICursor FAR *lpCursor, LPVOID hlstrData, 
      ULONG ulOffset, ULONG ulSize);

    BOOL FillColBindRec( DBCOLUMNBINDING *pColBind, 
      ULONG *pulInOffset, ULONG *pulOutOffset, BOOL fRemoteDataCtl, BOOL fUseEntryId);

	// Similar to above, but uses DBVARIANT bindings.
    BOOL FillColBindDbVariant( DBCOLUMNBINDING *pColBind, 
      ULONG *pulInOffset, ULONG *pulOutOffset);


    BOOL FixReadValue(LONG FAR *lplDataDest, LPSTR lpszDataSrc, 
      DWORD dwDataType, DWORD dwLenRead);

    BOOL GetDBVariantData(DBVARIANT FAR *lpdbVar, 
      DWORD FAR *dwRetDataType, LPVOID FAR *lpvRetData, DWORD FAR *dwRetVarLen);

// Member variables:
public:
	LPTSTR m_strDataField;
    DBCOLUMNID m_ColID;
    ULONG m_ulType;
    ULONG m_ulMaxLen;
    DWORD m_dwBindType;
//    ULONG m_ulEntryMaxLen;
    int   m_iIndex;
    UINT  m_uFlags;

    // Updated when Columns Bindings are set / used after Fetch
    ULONG m_obInfo;
    ULONG m_obVarDataLen;
    ULONG m_obData;
    DWORD m_dwBindTypeUsed;
    DWORD m_dwDataTypeUsed;

    // Post-read members      
    UINT  m_fNewRead;        // Read is new.  NOTE: Order does matter:
#define FPDB_COL_NOFETCH      0    // No fetch yet
#define FPDB_COL_FETCHFREED   1    // Fetch was freed by FreeFetchData()
#define FPDB_COL_POSTFETCH    2    // Fetch processed by PostFetchSetup()
#define FPDB_COL_BLOB_INFO    3    // Any blob info is now available

    BLOB FAR *m_lpBlob;
    ULONG m_ulReadSize; 

    // Write to DB members
    IStream FAR *m_lpStream;
    BLOB FAR *m_lpWriteBlob;

// Implementation
public:
	virtual ~CfpDBColumn();
};


#endif  // ifndef FPDBCOL_H
