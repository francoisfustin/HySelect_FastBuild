/****************************************************************************
* FPATLVBX.H  -  Declaration of FarPoint's ATL Functions for 
*                converting from VBX project to OCX.
*
* Copyright (C) 1991-1998 - FarPoint Technologies
* All Rights Reserved.
*
* No part of this source code may be copied, modified or
* reproduced in any form without retaining the above
* copyright notice.  This source code, or source code
* derived from it, may not be redistributed without
* express written permission of FarPoint Technologies.
*
****************************************************************************/
#ifndef FPATLVBX_H
#define FPATLVBX_H

#include "fpdbutil.h"
#include "\fplibs\fptools\atl\stdafx.h"

/////////////////////////////////////////////////////////////////////////////
//  Property sets

typedef struct tagSECTIONHEADER
{
	DWORD       cbSection;
	DWORD       cProperties;  // Number of props.
} SECTIONHEADER, *LPSECTIONHEADER;

typedef struct tagPROPERTYIDOFFSET
{
	DWORD       propertyID;
	DWORD       dwOffset;
} PROPERTYIDOFFSET, *LPPROPERTYIDOFFSET;

typedef struct tagPROPHEADER
{
	WORD        wByteOrder;    // Always 0xFFFE
	WORD        wFormat;       // Always 0
	DWORD       dwOSVer;       // System version
	CLSID       clsID;         // Application CLSID
	DWORD       cSections;     // Number of sections (must be at least 1)
} PROPHEADER, *LPPROPHEADER;

typedef struct tagFORMATIDOFFSET
{
	GUID        formatID;
	DWORD       dwOffset;
} FORMATIDOFFSET, *LPFORMATIDOFFSET;

class CfpPropertySet;
class CfpPropertySection;

class CfpPropString : public CfpString
{
public:

	CfpPropString();
	CfpPropString(LPCTSTR lpsz);
	CfpPropString(LPCTSTR key, DWORD dwValue);

	int GetLength() const;
	operator LPCTSTR() const;           // as a C string

    void MakeLower()
    { 
      if (m_tstr)
        _tcslwr(m_tstr);
    }

public:
    DWORD m_dwValue;
};

class CfpMapStringToPtr : public CStringArray
{
public:

  // Lookup
  BOOL Lookup(LPCTSTR key, DWORD& rValue) const;
  // BOOL LookupKey(LPCTSTR key, LPCTSTR& rKey) const;

  BOOL Add(LPCTSTR key, DWORD dwValue);
  
  POSITION GetStartPosition() const;
  void GetNextAssoc(POSITION& rNextPosition, CfpPropString& rKey, DWORD& rValue) const;

};

class CfpPropObList : public CObList
{
};

class CfpProperty : public CObject
{
	friend class CfpPropertySet;
	friend class CfpPropertySection;

public:
// Construction
	CfpProperty( void );
	CfpProperty( DWORD dwID, const LPVOID pValue, DWORD dwType );

// Attributes
	BOOL    Set( DWORD dwID, const LPVOID pValue, DWORD dwType );
	BOOL    Set( const LPVOID pValue, DWORD dwType );
	BOOL    Set( const LPVOID pValue );
	LPVOID  Get( DWORD* pcb );     // Returns pointer to actual value
	LPVOID  Get( void );           // Returns pointer to actual value
    BOOL    Get(CComVariant *pvar);

	DWORD   GetType( void );       // Returns property type
	void    SetType( DWORD dwType );
	DWORD   GetID( void );
	void    SetID( DWORD dwPropID );

	LPVOID  GetRawValue( void );   // Returns pointer internal value (may
									// include size information)
// Operations
	BOOL    WriteToStream( IStream* pIStream );
	BOOL    ReadFromStream( IStream* pIStream );

private:
	DWORD       m_dwPropID;
	DWORD       m_dwType;
	LPVOID      m_pValue;

	LPVOID  AllocValue(ULONG cb);
	void    FreeValue();

public:
	~CfpProperty();
};


class CfpPropertySection : public CObject
{
	friend class CfpPropertySet;
	friend class CfpProperty;

public:
// Construction
	CfpPropertySection( void );
	CfpPropertySection( CLSID FormatID );

// Attributes
	CLSID   GetFormatID( void );
	void    SetFormatID( CLSID FormatID );

	BOOL    Set( DWORD dwPropID, LPVOID pValue, DWORD dwType );
	BOOL    Set( DWORD dwPropID, LPVOID pValue );
	LPVOID  Get( DWORD dwPropID, DWORD* pcb );
	LPVOID  Get( DWORD dwPropID );
	void    Remove( DWORD dwPropID );
	void    RemoveAll();

	CfpProperty* GetProperty( DWORD dwPropID );
	void AddProperty( CfpProperty* pProp );

	DWORD   GetSize( void );
	DWORD   GetCount( void );
	CfpPropObList* GetList( void ); //WAS CPtrList*

	BOOL    GetID( LPCTSTR pszName, DWORD* pdwPropID );
    BOOL    GetID(LPCOLESTR pszName, DWORD* pdwPropID);
	BOOL    SetName( DWORD dwPropID, LPCTSTR pszName );

	BOOL    SetSectionName( LPCTSTR pszName );
	LPCTSTR GetSectionName( void );

// Operations
	BOOL    WriteToStream( IStream* pIStream );
	BOOL    ReadFromStream( IStream* pIStream, LARGE_INTEGER liPropSet );
	BOOL    WriteNameDictToStream( IStream* pIStream );
	BOOL    ReadNameDictFromStream( IStream* pIStream );

private:
// Implementation
	CLSID           m_FormatID;
	SECTIONHEADER   m_SH;
	// List of properties (CfpProperty)
	CfpPropObList         m_PropList;    //WAS CPtrList
	// Dictionary of property names
	CfpMapStringToPtr m_NameDict;    //WAS CMapStringToPtr
	CfpPropString       m_strSectionName; //WAS CString

public:
	~CfpPropertySection();
};


/////////////////////////////////////////////////////////////////////////////
// CfpPropertySet

class CfpPropertySet : public CObject
{
	friend class CfpPropertySection;
	friend class CfpProperty;

public:
// Construction
	CfpPropertySet( void );
	CfpPropertySet( CLSID clsID ) ;

// Attributes
	BOOL    Set( CLSID FormatID, DWORD dwPropID, LPVOID pValue, DWORD dwType );
	BOOL    Set( CLSID FormatID, DWORD dwPropID, LPVOID pValue );
	LPVOID  Get( CLSID FormatID, DWORD dwPropID, DWORD* pcb );
	LPVOID  Get( CLSID FormatID, DWORD dwPropID );
	void    Remove( CLSID FormatID, DWORD dwPropID );
	void    Remove( CLSID FormatID );
	void    RemoveAll( );

	CfpProperty* GetProperty( CLSID FormatID, DWORD dwPropID );
	void AddProperty( CLSID FormatID, CfpProperty* pProp );
	CfpPropertySection* GetSection( CLSID FormatID );
	CfpPropertySection* AddSection( CLSID FormatID );
	void AddSection( CfpPropertySection* psect );

	WORD    GetByteOrder( void );
	WORD    GetFormatVersion( void );
	void    SetFormatVersion( WORD wFmtVersion );
	DWORD   GetOSVersion( void );
	void    SetOSVersion( DWORD dwOSVer );
	CLSID   GetClassID( void );
	void    SetClassID( CLSID clsid );
	DWORD   GetCount( void );
	CfpPropObList* GetList( void );  //WAS CPtrList*

// Operations
	BOOL    WriteToStream( IStream* pIStream );
	BOOL    ReadFromStream( IStream* pIStream );

// Implementation
private:
	PROPHEADER      m_PH;
	CfpPropObList   m_SectionList; //WAS CPtrList

public:
	~CfpPropertySet();
};

//----------------------------------------------------------------
// Function to read from Propset (during VBX conversion) to our Stream
//----------------------------------------------------------------
#ifdef __cplusplus
extern "C" {
#endif
__declspec(dllexport) HRESULT __cdecl fpLoadPropsetTofpStream(LPSTREAM *pRetStmDest,
  LPSTREAM pStmSrc, ATL_PROPMAP_ENTRY* pMap, CfpAtlBase *pfpAtlCtl);

__declspec(dllexport) HRESULT __cdecl fpAtlVbxSaveFromfpStream(LPSTREAM *ppstmSrc,
  LPSTREAM pStmDest, BOOL fClearDirty, DWORD dwCtlVersion, 
  SIZE *psizeExtent, CfpAtlBase *pfpAtlCtl);

#ifdef __cplusplus
}
#endif    

#endif // ifndef FPATLVBX_H
