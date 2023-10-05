//////////////////////////////////////////////////////////////////////////
//  DataBase.cpp : implementation file
//////////////////////////////////////////////////////////////////////////
#include "stdafx.h"

#include <math.h>
#include <float.h>
#include <vector>

#ifndef TACBX
#include <regex>
#endif

#include <algorithm>
#include "global.h"

#ifndef TACBX
	#include "TASelect.h"
	#include "ModuleVersion.h"
#else
	#include "TAPersist.hpp"
#endif

#ifndef WIN32
	#include "crc.h"
#endif

#include "DataBase.h"
#include "DataBObj.h"
#include "DataStruct.h"
#include "XmlReader.h" 

#ifndef TACBX
	#include "Hydromod.h"
#endif

// Load DB from independent task
#ifndef WIN32
#include "smx.h"
#include <usart.h>
extern TCB_PTR InitialyserTask;
#endif

// macro to convert WIN32 double to ARM double and back
#define CONVERT_ARM_DOUBLE(z) (*((uint64_t *)&z) = ((*((uint64_t *)&z)) >> 32) | (((*((uint64_t *)&z)) & 0x00000000FFFFFFFFULL) << 32))

IMPLEMENT_CLASS_REGISTER



//////////////////////////////////////////////////////////////////////////////////////////////
//
//	General purpose functions
//	
//////////////////////////////////////////////////////////////////////////////////////////////

bool g_bSBCSFile = 0;
bool g_bWriteTACBXMode = 0;

CTADatabase *GetpTADB() 
{
#ifdef TACBX
	return &m_TADB;
#else
	return TASApp.GetpTADB();
#endif
};

#ifndef TACBX //TACBX////////////////////////////////////////////////////////
bool ReadString( INPSTREAM inpf, CString &str )
{
	TCHAR tcBuffer[1024];
	*tcBuffer = 0;
	
	if( false == ReadString( inpf, tcBuffer, sizeof( tcBuffer ) ) )
	{
		return false;
	}

	str = tcBuffer;
	return true;
}

void WriteFormatedStringW( OUTSTREAM outf, CString str, CString strTab, bool bWithQuotes )
{
	CString str1; 
	str1.Format( ( true == bWithQuotes ) ? _T("\r\n%s\"%s\"") : _T("\r\n%s%s"), strTab, str );
	outf.write( (const char*)(LPCTSTR)str1, str1.GetLength() * sizeof( TCHAR ) );
}

void WriteFormatedStringA( OUTSTREAM outf, CString str, CString strTab, bool bWithQuotes )
{
	CString str1; 
	str1.Format( ( true == bWithQuotes ) ? _T("\r\n%s\"%s\"") : _T("\r\n%s%s"), strTab, str );
	CT2CA pszTemp( str1 );
	outf.write( pszTemp, str1.GetLength() );
}

void WriteFormatedStringA2( OUTSTREAM outf, CString str, CString strTab )
{
	CString str1; 
	str1.Format( _T("%s%s"), strTab, str );
	CT2CA pszTemp( str1 );
	outf.write( pszTemp, str1.GetLength() );
}

bool InheritedID( TCHAR *ptcLine, CString &ID )
{
	if( 1 == _tcslen( ptcLine ) )	// only '{'
	{
		return false;
	}

	CString str = ptcLine;
	
	// Do not change the string to upper case because, sometimes the names have upper and lower cases
	if( str.Find( _T("Inherited") ) < 0 )
	{
		return false;
	}

	// Inherited, Get ID and fill array with strings.
	int i = str.Find( _T("(") );
	int j = str.Find( _T(")") );

	if( i <= 0 || j <= i )
	{
		ASSERTA_RETURN( false );
	}

	ID = str.Mid( i + 1, j - i - 1 );

	if( ID.GetLength() > _ID_LENGTH )
	{
		ASSERTA_RETURN( false );
	}
	
	return true;
}

// Parse a line with fields delimited by a backslash. !!! Max 25 fields !!!
// Parsing takes place for nField fields. If nField==0, the number of fields 
// is automatically detected. A vector of pointers to the beginning of 
// each field is returned.
TCHAR **ParseTextLine( TCHAR *ptcLine, int *piField, TCHAR tcSeparator )
{
	HYSELECT_TRY
	{
		if( *piField < 0 )
		{
			HYSELECT_THROW( _T("Field number (%i) can't be lower than 0."), *piField );
		}
		else if( *piField >= 25 )
		{
			HYSELECT_THROW( _T("Field number (%i) can't be higher than 24."), *piField );
		}

		static TCHAR *ptcField[25];
		TCHAR *ptcItr;

		// Detection of the number of fields.
		if( 0 == *piField )
		{
			for( ptcItr = ptcLine; _T('\0') != *ptcItr; ptcItr++ )
			{
				if( tcSeparator == *ptcItr) 
				{
					(*piField)++;
				}
			}
			
			(*piField)++;
		}

		ptcField[0] = ptcLine;
		
		// Parse the line.
		for( int i = 0; i < (*piField) - 1; i++ )
		{
			for( ptcItr = ptcField[i]; _T('\0') != *ptcItr && tcSeparator != *ptcItr; ptcItr++);
			
			if( _T('\0') == *ptcItr )
			{
				HYSELECT_THROW( _T("Field at position '%i' can't be NULL."), i );
			}

			ptcField[i+1] = ptcItr+1;
			*ptcItr = _T('\0');
		}
		
		return ptcField;
	}
	HYSELECT_CATCH( clHySelectException, _T("Error in 'ParseTextLine'.") )
}

// Convert a line composed of 0 & 1 to an integer
int InterpretBinaryLine( CString str )
{
	int j = 1;
	int iResult = 0;
	
	for( int i = str.GetLength()-1; i >= 0; i-- )
	{
		if( str.GetAt( i ) == CString( _T("1") ) )
		{
			iResult += j;
		}

		j <<= 1;
	}

	return iResult;
}

#endif //TACBX////////////////////////////////////////////////////////

/////////////////////////////////////////////////////////////////////////////
// Copy a string of characters without double spaces and with a limitation 
// on the number of characters.
/////////////////////////////////////////////////////////////////////////////
TCHAR *PurgeString( TCHAR *ptcBuffer, LPCTSTR ptcSource, int iBufferLength )
{
	ASSERT( NULL != ptcBuffer );
	ASSERT( NULL != ptcSource );
	ASSERT( iBufferLength > 0 );

	// Delete the heading spaces
	for(; _T(' ') == *ptcSource; ptcSource++ );
	
	// Copy the string
	iBufferLength--;
	int iPosBuffer;

	for( iPosBuffer = 0; ( iPosBuffer < iBufferLength ) && _T('\0') != *ptcSource; iPosBuffer++ )
	{
		if( _T(' ') == *ptcSource )
		{
			for( ; _T(' ') == *ptcSource; ptcSource++ );
			ptcBuffer[iPosBuffer] = _T(' ');
		}
		else
		{
			ptcBuffer[iPosBuffer] = *(ptcSource);
			ptcSource++;
		}
	}
	
	// Check that the last character is not a space.
	if( ( iPosBuffer > 0 ) && ( _T(' ') == ptcBuffer[iPosBuffer - 1] ) )
	{
		ptcBuffer[iPosBuffer - 1] = '\0';
	}
	else
	{
		ptcBuffer[iPosBuffer] = '\0';
	}

	return ptcBuffer;
}

void WriteString( OUTSTREAM outf, LPCTSTR ptcString )
{
	ASSERT( NULL != ptcString );

	int iLength = (int)_tcslen( ptcString );
	ASSERT( iLength >= 0 );

	outf.write( (const char *)&iLength, sizeof( iLength ) );
	outf.write( (const char *)ptcString, iLength * sizeof( TCHAR ) );
}

bool ReadString( INPSTREAM inpf, _string &str )
{
	static TCHAR tcBuffer[10000];
	*tcBuffer = 0;

	if( false == ReadString( inpf, tcBuffer, 10000 ) )
	{
		return false;
	}

	str = tcBuffer;
	return true;
}

bool ReadString( INPSTREAM inpf, TCHAR *ptcString, int iMaxLen )
{
	ASSERT( NULL != ptcString );
	
	int iLength=0;
	ptcString[iLength] = _T('\0');
	inpf.read( (char*)&iLength, sizeof( iLength ) );
	
	if( iLength < 0 || iLength > iMaxLen )
	{
		return false;
	}
	
	if( inpf.rdstate() & std::ifstream::failbit )
	{
		return false;
	}
	
	if( iLength > 0 )
	{
		if( true == g_bSBCSFile )
		{
			BYTE OneChar;

			for( int i = 0; i < iLength; i++ )
			{
				inpf.read( (char *)&OneChar, sizeof( char ) ); 
				ptcString[i] = (TCHAR)OneChar;

				if( inpf.rdstate() & std::ifstream::failbit )
				{
					return false;
				}
			}
		}
		else
		{
			inpf.read( (char *)ptcString, iLength * sizeof( TCHAR ) );

			if( inpf.rdstate() & std::ifstream::failbit )
			{
				return false;
			}
		}
	}
	
	ptcString[iLength] = '\0';
	return true;
}

bool ReadIDPtr( INPSTREAM  inpf, IDPTR *pIDPTR, CTADatabase *pTADB )
{
	*pIDPTR = _NULL_IDPTR;

	if( false == ReadString( inpf, pIDPTR->ID, _ID_LENGTH + 1 ) )
	{
		return false;
	}

	if( _T('\0') != *pIDPTR->ID )
	{
		pIDPTR->DB = pTADB;
//		We don't know order of object storage in file!
//		Association should be done by CrossReferenceID() after DB reading
// 		Extend(pIDPTR);
// 		return false;
	}
	
	return true;
}

// Read/Write double taking into account endian organization for ARM processor
double ReadDouble( INPSTREAM inpf )
{
	double d;
	inpf.read( (char*)&d, sizeof( double ) );

#if !defined( WIN32 )
	CONVERT_ARM_DOUBLE( d );
#endif	

	return d;
}

void WriteDouble( OUTSTREAM outf, double d )
{
#if !defined( WIN32 )
	CONVERT_ARM_DOUBLE( d );
#endif	
	
	WriteData<>( outf, d );
}

//////////////////////////////////////////////////////////////////////////////////////////////
//
//	IDPTR
//
//////////////////////////////////////////////////////////////////////////////////////////////

const IDPTR _NULL_IDPTR/* = {_T(""), 0, 0, 0}*/;

IDPTR::IDPTR()
{
	Clear();
}

void IDPTR::Clear()
{
	memset( ID, 0, sizeof( ID )) ;
	this->MP = NULL;
	this->PP = NULL;
	this->DB = NULL;
}

IDPTR::IDPTR( _string ID )
{
	SetID( ID.c_str() );
}

IDPTR::IDPTR( TCHAR *pID )
{
	SetID( pID );
}

void IDPTR::SetID( TCHAR *pID )
{
	if( NULL == pID )
	{
		return;
	}

	int iCount = 0;
	bool bGood = false;

	while( iCount++ <= _ID_LENGTH && false == bGood )
	{
		if( _T('\0') == *(pID + iCount) ) 
		{
			bGood = true;
		}
	}

	if( false == bGood)
	{
		ASSERT( 0 );
	}

	memset( ID, 0, sizeof( ID ) );
	this->MP = NULL;
	this->PP = NULL;
	this->DB = NULL;
	_tcscpy_s( this->ID, _ID_LENGTH + 1, pID );
}

bool IDPTR::operator!=( const IDPTR& idptr ) const
{
	if( 0 != IDcmp( ID, idptr.ID ) )
	{
		return true;
	}

	if( MP != idptr.MP )
	{
		return true;
	}

	if( DB != idptr.DB )
	{
		return true;
	}

	if( PP != idptr.PP )
	{
		return true;
	}

	return false;
}

bool IDPTR::IDMatch( _string IDstr )
{
	if( 0 == IDcmp( IDstr.c_str(), ID ) ) 
	{
		return true;
	}

	return false;
}

bool IDPTR::Read( INPSTREAM  inpf, CTADatabase *pTADB )
{
	Clear();

	if( false == ReadString( inpf, ID, _ID_LENGTH + 1 ) )
	{
		return false;
	}

	if( _T('\0') != *ID )
	{
		DB = pTADB;
//		We don't know order of object storage in file!
//		Association should be done by CrossReferenceID() after DB reading
// 		Extend(this);
// 		if ( NULL != MP )
			return true;
	}
	
	return false;
}

#ifndef TACBX
bool IDPTR::ReadTxt( INPSTREAM  inpf, unsigned short *pusLineCount, CTADatabase *pTADB )
{
	Clear();
	TCHAR *ptcLine = CData::ReadTextLine( inpf, pusLineCount );
	
	if( _tcslen( ptcLine ) > _ID_LENGTH )
	{
		ASSERTA_RETURN( false );
	}
	
	_tcsncpy_s( ID, _ID_LENGTH + 1, ptcLine, _ID_LENGTH );
	
	if( _T('\0') != *ID )
	{
		DB = pTADB;
		Extend( this );

		if( NULL != MP )
		{
			return true;
		}
	}
	
	return false;
}
#endif

bool Extend( IDPTR *pIDPtr )
{
	HYSELECT_TRY
	{
		if( NULL == pIDPtr )
		{
			HYSELECT_THROW_RETURNARG( _T("'pIDPtr' argument can't be null."), false );
		}
		else if( _T('\0') == *pIDPtr->ID )
		{
			HYSELECT_THROW_RETURNARG( _T("'ID' of the 'pIDPtr' argument is not defined."), false );
		}
		else if( NULL == pIDPtr->DB )
		{
			HYSELECT_THROW_RETURNARG( _T("'DB' pointer can't be null for the '%' object."), false, pIDPtr->ID );
		}

		if( NULL == pIDPtr->MP || NULL == pIDPtr->PP )
		{
			// Pay attention: the 'Get' method sends back a reference. Thus any modification on 'RealObjectIDPtr' is applied on 
			// this object.
			IDPTR RealObjectIDPtr = pIDPtr->DB->Get( pIDPtr->ID );

			if( _T('\0') != *RealObjectIDPtr.ID )
			{
				if( NULL == pIDPtr->PP )
				{
					// Try to find the parent table pointer with the owner list.
					if( 0 != RealObjectIDPtr.MP->IsLocked() )
					{
						CTable *pclTrueParent = RealObjectIDPtr.MP->GetTrueOwner();

						if( NULL != pclTrueParent )
						{
							pIDPtr->PP = (CTable *)pclTrueParent;
						}
						else
						{
							HYSELECT_THROW_RETURNARG( _T("Impossible to find the parent in the owner list of the '%s' object."), false, pIDPtr->ID );
						}
					}
				}
			}
			else
			{
				// When reading database, it can happen that an object on which an ID point is not yet read.
				return true;
			}

			pIDPtr->MP = RealObjectIDPtr.MP;
		}

		if(	_T('\0') == *pIDPtr->ID )
		{
			HYSELECT_THROW_RETURNARG( _T("The method did not succeed to fill the 'ID' value for this object."), false );
		}
		else if( NULL == pIDPtr->MP )
		{
			HYSELECT_THROW_RETURNARG( _T("The method did not succeed to fill the 'MP' value for this object."), false );
		}
		else if( NULL == pIDPtr->DB )
		{
			HYSELECT_THROW_RETURNARG( _T("The method did not succeed to fill the 'DB' value for this object."), false );
		}

		if( 0 != _tcscmp( pIDPtr->MP->GetIDPtr().ID, pIDPtr->ID ) )
		{
			HYSELECT_THROW_RETURNARG( _T("The '%s' ID of the object to extend is not the same of the object in memory ('%s')"), false, pIDPtr->ID, pIDPtr->MP->GetIDPtr().ID );
		}
		else if( pIDPtr->DB != pIDPtr->MP->GetDB() )
		{
			HYSELECT_THROW_RETURNARG( _T("The DB pointer (0x%08X) object to extend is not the same of the object in memory (0x%08X)"), false, pIDPtr->DB, pIDPtr->MP->GetDB() );
		}

		return true;
	}
	HYSELECT_CATCH( clHySelectException, _T("Error in the 'Extend' function.") )
}

//////////////////////////////////////////////////////////////////////////////////////////////
// CDataList 

CDataList::CDataList()
	: m_ArrayStep( 16 )
{
	m_ppDataArray = 0;
	m_usArraySize = 0;
	m_usDataAmount = 0;
	m_sCursor = 0;
}

CDataList::~CDataList()
{
	MakeEmpty();
}

const IDPTR &CDataList::GetIDPtrFromID( LPCTSTR ptcID )
{
	unsigned short usPos;

	if( NULL != ptcID && _T('\0') != *ptcID && 0 != Locate( ptcID, &usPos ) )
	{
		return m_ppDataArray[usPos]->GetIDPtr();
	}

	return _NULL_IDPTR;
}

const IDPTR &CDataList::GetFirst(unsigned short &usPos )
{
	usPos = 0;
	return ( m_usDataAmount > 0 ) ? m_ppDataArray[usPos]->GetIDPtr() : _NULL_IDPTR;
} 

const IDPTR &CDataList::GetNext( unsigned short &usPos )
{
	usPos++;
	return ( usPos < m_usDataAmount ) ? m_ppDataArray[usPos]->GetIDPtr() : _NULL_IDPTR;
}

const IDPTR &CDataList::GetFirst()
{
	return ( m_usDataAmount > 0 ) ? m_ppDataArray[m_sCursor = 0]->GetIDPtr() : _NULL_IDPTR;
} 

const IDPTR &CDataList::GetNext()
{
	return ( ++m_sCursor < m_usDataAmount) ? m_ppDataArray[m_sCursor]->GetIDPtr() : _NULL_IDPTR;
}

int CDataList::GetPos()
{
	return m_sCursor;
}

void CDataList::SetPos( int iPos )
{
	if( iPos < m_usDataAmount )
	{
		m_sCursor = iPos;
	}
}

void CDataList::SetPos( LPCTSTR ptcID )
{
	unsigned short usPos;

	if( NULL != ptcID && 0 != Locate( ptcID, &usPos ) )
	{
		SetPos( usPos );
	}
}

void CDataList::Insert( const IDPTR &IDPtr )
{
	HYSELECT_TRY
	{
		// Enlarge the array if necessary.
		if( m_usDataAmount >= m_usArraySize )
		{
			CData **ppOldArray = m_ppDataArray;

			// Allocation one element more for security, see Remove function.
			m_usArraySize += m_ArrayStep;
			m_ppDataArray = new CData*[m_usArraySize + 1];
		
			if( NULL == m_ppDataArray )
			{
				HYSELECT_THROW( _T("Internal error: 'm_ppDataArray' instantiation error.") );
			}

			for( unsigned short i = 0; i < m_usDataAmount; i++ )
			{
				m_ppDataArray[i] = ppOldArray[i];
			}

			delete [] ppOldArray;
		}
		
		// sSrt ID in alphabetical order.
		unsigned short usPos;
	
		if( 0 != Locate( IDPtr.ID, &usPos ) )
		{
			HYSELECT_THROW( _T("Internal error: No position was found to insert the object '%s' in the database."), IDPtr.ID );
		}
	
		for( unsigned short i = m_usDataAmount; i > usPos; i-- )
		{
			m_ppDataArray[i] = m_ppDataArray[i - 1];
		}
	
		m_ppDataArray[usPos] = IDPtr.MP;
		m_usDataAmount++;
	}
	HYSELECT_CATCH( clHySelectException, _T("Error in 'CDataList::Insert'.") )
}

const IDPTR &CDataList::Remove( LPCTSTR ptCID )
{
	ASSERT( NULL != ptCID && _T('\0') != *ptCID );
	
	// Reduce the required array size if possible.
	if( ( m_usArraySize - m_usDataAmount ) > m_ArrayStep )
	{
		CData **ppOldArray = m_ppDataArray;
		m_ppDataArray = new CData*[m_usArraySize -= m_ArrayStep];
		
		for( unsigned short i = 0; i < m_usDataAmount; i++ )
		{
			m_ppDataArray[i] = ppOldArray[i];
		}

		delete [] ppOldArray;
	}

	unsigned short usPos;

	if( 0 == Locate( ptCID, &usPos ) )
	{
#ifndef TACBX
		ASSERT( false );
#endif
		return _NULL_IDPTR;	// the ID does not exist
	}
	
	CData *pDataObj = m_ppDataArray[usPos];

	// Size of DataArray should be m_ArraySize+1.
	for( unsigned short i = usPos; i < m_usDataAmount; i++ )
	{
		m_ppDataArray[i] = m_ppDataArray[i + 1];
	}

	m_usDataAmount--;

	return pDataObj->GetIDPtr();
}

LPCTSTR CDataList::CreateID()
{
#ifdef TACBX
	#define FIRSTID _T("{AAAA}_CBX_ID")
#else
	#define FIRSTID _T("{AAAA}_GEN_ID")
#endif

	static TCHAR tcIDBuffer[_ID_LENGTH + 1];

	// Generate ID which does not exist yet.
	// Use letters from A to Z.
	// as there cannot be more than 65536 records in the database, use 4 letters only
	ASSERT( _tcslen( FIRSTID ) <= _ID_LENGTH );
	_tcsncpy_s( (TCHAR *)tcIDBuffer, SIZEOFINTCHAR( tcIDBuffer ), (const TCHAR *)FIRSTID, SIZEOFINTCHAR( tcIDBuffer ) - 1 );
	
	unsigned short usPos;

	// We will search in the DB as soon as an ID is not exist.
	// To do that, we start with the first "{AAAA}_GEN_ID". If exists, we search for "{AAAB}_GEN_ID", "{AAAC}_GEN_ID" ... "{AAAZ}_GEN_ID".
	// After it's "{AABA}_GEN_ID", "{AABB}_GEN_ID", "{AABC}_GEN_ID" ... "{AABZ}_GEN_ID". And so on. It's what the 'for' loop does.
	do
	{
		if( 0 == Locate( tcIDBuffer, &usPos ) )		// created non existing ID.
		{
			return tcIDBuffer;
		}

		for( usPos = 4; (++tcIDBuffer[usPos] > _T('Z') ) && 0 != usPos; tcIDBuffer[usPos--] = _T('A') );
	}
	while( usPos != 0 );

	ASSERTA_RETURN( false );
}
		
int CDataList::Locate( LPCTSTR ptcID, unsigned short *pusPos )
{
	ASSERT( NULL != ptcID && _T('\0') != *ptcID );
	ASSERT( NULL != pusPos );
	
	if( 0 == m_usDataAmount )
	{
		*pusPos = 0;
		return 0;
	}
	
	// Use dichotomy algorithm.
	unsigned short usMinPos = 0;
	unsigned short usMaxPos = m_usDataAmount - 1;

	while( ( usMaxPos - usMinPos ) > 1 )
	{
		unsigned short usMid = ( usMinPos + usMaxPos ) / 2;
		
		if( _tcscmp( ptcID, m_ppDataArray[usMid]->GetIDPtr().ID) < 0 )
		{
			usMaxPos = usMid;
		}
		else
		{
			usMinPos = usMid;
		}
	}
	
	// Check whether the position is Minpos or MaxPos.
	int iComp = _tcscmp( ptcID, m_ppDataArray[usMaxPos]->GetIDPtr().ID );
	
	if( iComp > 0 )
	{ 
		*pusPos = usMaxPos + 1;
		return 0;
	}
	else if( 0 == iComp )
	{ 
		*pusPos = usMaxPos;
		return 1;
	}

	iComp = _tcscmp( ptcID, m_ppDataArray[usMinPos]->GetIDPtr().ID );
	
	if( iComp > 0 )
	{ 
		*pusPos = usMaxPos;
		return 0;
	}
	else if( 0 == iComp )
	{
		*pusPos = usMinPos;
		return 1;
	}

	*pusPos = usMinPos;
	return 0;
}

//////////////////////////////////////////////////////////////////////////////////////////////
//
//	CDBFileHeader class
//	
//////////////////////////////////////////////////////////////////////////////////////////////

CDBFileHeader::CDBFileHeader()
{
	wcscpy( m_strVersion, _T("") );
	m_strUID = _T("");

#ifndef TACBX
	m_strAppName = DBFH_APPNAME;
	m_strTADBKey = _T("");
	m_strAppVersion = _T("");
	m_strDBArea = _T("");
	m_strDBVersion = _T("");
	m_strHUBSchemesDLLName = _HUB_SCHEMES_DLL;
	m_strHUBSchemesDLLVersion = _T("");
#endif
}

void CDBFileHeader::Write( OUTSTREAM outf )
{
	// Write version.
	outf.write( (const char *)m_strVersion, sizeof( m_strVersion ) );

	// Write UID if exist.
	WriteString ( outf, m_strUID.c_str() );

#ifndef TACBX //TACBX////////////////////////////////////////////////////////
	// Write a HySelect description.

	CString str = DBFH_APPNAME + TASApp.GetTADBKey() + _T("(");
	CModuleVersion ver;

	if( TRUE == ver.GetFileVersionInfo( _T("HySelect.exe") ) ) 
	{
		str += TASApp.LoadLocalizedString( IDS_ABOUTBOX_VERSION );

		m_strAppVersion.Format( _T("%d.%d.%d.%d"), HIWORD( ver.dwProductVersionMS ), LOWORD( ver.dwProductVersionMS ),
				HIWORD( ver.dwProductVersionLS), LOWORD( ver.dwProductVersionLS ) );
		
		str += m_strAppVersion;
	}

	str += _T(")" ) + CString( _T("; ") );
	
	m_strDBArea = CString( ( (CDB_MultiString *)GetpTADB()->Get( _T("TADB_VERSION") ).MP )->GetString( 1 ) );
	str += m_strDBArea + CString( _T("; ") );
	
	str += TASApp.LoadLocalizedString( IDS_TADBVERSION );

	m_strDBVersion = CString( ( (CDB_MultiString *)GetpTADB()->Get( _T("TADB_VERSION") ).MP )->GetString( 0 ) );
	str += CString( _T(": ") ) + m_strDBVersion + CString( _T("\r\n") );
	
	if( NULL != TASApp.GetHMHubSchemes() )
	{
		ver.~CModuleVersion();
		
		if( TRUE == ver.GetFileVersionInfo( _HUB_SCHEMES_DLL ) )
		{
			m_strHUBSchemesDLLName = _HUB_SCHEMES_DLL;
			m_strHUBSchemesDLLVersion.Format( _T(",%s(v%d.%d.%d)"), m_strHUBSchemesDLLName, HIWORD( ver.dwProductVersionMS ), 
					LOWORD( ver.dwProductVersionMS ), HIWORD( ver.dwProductVersionLS ) );
			
			str += m_strHUBSchemesDLLName + m_strHUBSchemesDLLVersion;
		}

	}
	
	WriteString( outf, str );

#else

	// TODO Customize with information about HW/SW/DB version.
	WriteString( outf, _T("TAScope") );

#endif

}

bool CDBFileHeader::Read( INPSTREAM inpf, short nVersion )
{
	TCHAR tcBuffer[1024];
	memset( tcBuffer, 0, sizeof( tcBuffer ) );
	
	// Read version.
	if( true == g_bSBCSFile )
	{
		inpf.read( (char*)m_strVersion, _DATABASE_VERSION_LENGTH );
	}
	else
	{
		inpf.read( (char*)m_strVersion, sizeof( m_strVersion ) );
	}

	// Read Files version
	if( nVersion > 1 )
	{
		if( nVersion >= 4 )
		{
			if( false == ReadString( inpf, tcBuffer, sizeof( tcBuffer ) ) )
			{
				return false;
			}

			m_strUID = tcBuffer;
		}
		else
		{	
			//Update UID only for Datastruct
			#ifndef TACBX
				m_strUID = (LPCTSTR)GetTimeBasedUniqID();
			#endif
		}
		
		if( false == ReadString( inpf, tcBuffer, sizeof( tcBuffer ) ) )
		{
			return false;
		}

#ifndef TACBX
		_ExtractInfo( tcBuffer );
#endif

	}
	return true;
}

#ifndef TACBX
int CDBFileHeader::GetAppVersionNbr( void )
{
	if( true == m_strAppVersion.IsEmpty() )
	{
		return 0;
	}

	int iCurPos = 0;
	int iCurNumber = 1000;
	int iAppVersion = 0;
	CString strToken = m_strAppVersion.Tokenize( _T("."), iCurPos );
	
	while( _T("") != strToken )
	{
		int iNbr = _ttoi( strToken );
		iAppVersion += ( ( iNbr & 0xFF ) * iCurNumber ); 
		strToken = m_strAppVersion.Tokenize( _T("."), iCurPos );
		iCurNumber /= 10;
	}

	return iAppVersion;
}

void CDBFileHeader::_ExtractInfo( LPTSTR str )
{
	CString strInfo = str;
	
	if( true == strInfo.IsEmpty() )
	{
		return;
	}

	// Application name.
	m_strAppName = DBFH_APPNAME;
	int iPos = strInfo.Find( m_strAppName, 0 );

	if( -1 == iPos )
	{
 		m_strAppName = DBFH_OLDAPPNAME;
 		iPos = strInfo.Find( m_strAppName, 0 );
 
 		if( -1 == iPos )
 		{
			return;
		}
	}

	// Read TADB Key.
	iPos += m_strAppName.GetLength();
	int iPos2 = strInfo.Find( _T('('), iPos );

	if( -1 == iPos2 )
	{
		return;
	}

	m_strTADBKey = strInfo.Mid( iPos, iPos2 - iPos );

	// Read the application version if exist.
	iPos += ( iPos2 - iPos );
	CString strVersionName = TASApp.LoadLocalizedString( IDS_ABOUTBOX_VERSION );
	iPos2 = strInfo.Find( strVersionName, iPos );

	if( iPos2 > -1 )
	{
		iPos = iPos2 + strVersionName.GetLength();
		iPos2 = strInfo.Find( _T(')'), iPos );

		if( -1 == iPos2 )
		{
			return;
		}

		m_strAppVersion = strInfo.Mid( iPos, iPos2 - iPos );
	}

	// Read DB Area.
	iPos2 = strInfo.Find( _T(';'), iPos );

	if( -1 == iPos2 )
	{
		return;
	}

	// Bypass '; '.
	iPos = iPos2;
	iPos += 2;

	iPos2 = strInfo.Find( _T(';'), iPos );

	if( -1 == iPos2 )
	{
		return;
	}

	m_strDBArea = strInfo.Mid( iPos, iPos2 - iPos );

	// Read DB Version.
	iPos = iPos2;
	iPos2 = strInfo.Find( _T("; "), iPos );

	if( -1 == iPos2 )
	{
		return;
	}

	// Bypass '; '.
	iPos = iPos2;
	iPos += 2;

	iPos2 = strInfo.Find( _T(':'), iPos );

	if( -1 == iPos2 )
	{
		return;
	}

	// Bypass ': '.
	iPos = iPos2;
	iPos += 2;

	bool fHubScheme;
	iPos2 = strInfo.Find( _T(','), iPos );

	if( iPos2 > -1 )
	{
		m_strDBVersion = strInfo.Mid( iPos, iPos2 - iPos );
		fHubScheme = true;
	}
	else
	{
		m_strDBVersion = strInfo.Mid( iPos );
		m_strDBVersion.TrimRight();
		fHubScheme =false;
	}

	if( false == fHubScheme )
	{
		return;
	}

	// Bypass ','.
	iPos = iPos2 + 1;
	iPos2 = strInfo.Find( _T('('), iPos );

	if( -1 == iPos2 )
	{
		return;
	}

	m_strHUBSchemesDLLName = strInfo.Mid( iPos, iPos2 - iPos);
	iPos = iPos2;
	iPos2 = strInfo.Find( _T('v'), iPos );

	if( -1 == iPos2 )
	{
		return;
	}

	// Bypass 'v'.
	iPos = iPos2 + 1;
	iPos2 = strInfo.Find( _T(')'), iPos );

	if( -1 == iPos2 )
	{
		return;
	}

	m_strHUBSchemesDLLVersion = strInfo.Mid( iPos, iPos2 - iPos );
}

#endif

//////////////////////////////////////////////////////////////////////////////////////////////
//
//	CDataBase class
//	
//////////////////////////////////////////////////////////////////////////////////////////////

CDataBase::CDataBase( ChildID eChildID )
{
	HYSELECT_TRY
	{
		m_eChildID = eChildID;
		m_bSuperUserPsw = false;
		m_bDebugPsw = false;
		m_pRootTable = 0;
		Init();
	}
	HYSELECT_CATCH( clHySelectException, _T("Error in the 'CDataBase' constructor.") )
}

CDataBase::~CDataBase()
{
	TRACE(_T("\nDESTROYING CDataBase OBJECT\n"));
	MakeEmpty();
}

// CreateObjectAndInsertInTable recursive function used with DuplicateDataBaseTO
void CDataBase::CreateObjectAndInsertInTable( CDataBase *pDBto, IDPTR idptrObj )
{
	HYSELECT_TRY
	{
		if( NULL != idptrObj.PP )
		{
			IDPTR idptrPPinDBto = pDBto->Get( (idptrObj.PP)->GetIDPtr().ID );
		
			// Is the Owner exist?
			if( NULL == idptrPPinDBto.MP )
			{
				CreateObjectAndInsertInTable( pDBto, idptrObj.PP->GetIDPtr() );
				idptrPPinDBto = pDBto->Get( (idptrObj.PP)->GetIDPtr().ID );
			}
		
			if( NULL == idptrPPinDBto.MP )
			{
#if !defined (TACBX) && defined (_DEBUG)
				__debugbreak();
#endif
			}
			else
			{
				if( NULL == pDBto->Get( idptrObj.ID ).MP )
				{
					IDPTR newIDPtr = _NULL_IDPTR;
					pDBto->CreateObject( newIDPtr, (idptrObj.MP)->GetClassName(), idptrObj.ID );
					( (CTable *)idptrPPinDBto.MP )->Insert( newIDPtr );

					(idptrObj.MP)->Copy( newIDPtr.MP );
				}
			}
		}
	}
	HYSELECT_CATCH( clHySelectException, _T("Error in 'CDataBase::CreateObjectAndInsertInTable'.") )
}

// Duplicate current database to a new one
void CDataBase::DuplicateDataBaseTO( CDataBase *pDBto )
{
	HYSELECT_TRY
	{
		pDBto->Init();
		pDBto->SetUID( GetUID() );

		IDPTR idptr = _NULL_IDPTR;

		for( idptr = m_DataSet.GetFirst(); _T('\0') != *idptr.ID; idptr = m_DataSet.GetNext() )
		{
			// Floating object are not copied.
			if( NULL != idptr.PP && NULL != idptr.MP && NULL != idptr.MP->m_pOwners )
			{
				CreateObjectAndInsertInTable( pDBto, idptr );
			}
			else
			{
#if !defined (TACBX) && defined (_DEBUG)
				//__debugbreak();
#endif
			}
		}

		for( idptr = m_DataSet.GetFirst(); _T('\0') != *idptr.ID; idptr = m_DataSet.GetNext() )
		{
			if( NULL == pDBto->Get( idptr.ID ).MP || NULL == pDBto->Get( idptr.ID ).MP->m_pOwners )
			{
#if !defined (TACBX) && defined (_DEBUG)
				__debugbreak();
#endif
			}
		}
	}
	HYSELECT_CATCH( clHySelectException, _T("Error in 'CDataBase::DuplicateDataBaseTO'.") )
}

void CDataBase::Init()
{
	HYSELECT_TRY
	{
		MakeEmpty();
	
		// Not modified when empty.
		m_IsModified = 0;
		m_bRefreshResults = true;
	
		// Unknown version identification.
		*m_tcVersion = _T('\0');
	
		// Create UID based on the RTC
#ifndef TACBX
		m_UID = (LPCTSTR)GetTimeBasedUniqID();
#else
		m_UID = GetTimeBasedUniqID();
#endif
	
		// Create the default root table. The root table lock itself
		IDPTR IDPtr;
		CreateObject( IDPtr, CLASS( CTable ), _ROOT_TABLE);

		m_pRootTable = (CTable *)( IDPtr.MP );
		m_pRootTable->SetTrueParent( true );
		m_pRootTable->Lock( m_pRootTable->GetIDPtr() );
	}
	HYSELECT_CATCH( clHySelectException, _T("Error in 'CDataBase::Init'.") )
}

void CDataBase::CreateObject( IDPTR &idptr, LPCTSTR ptcClassName, LPCTSTR ptcID )
{
	HYSELECT_TRY
	{
		idptr = _NULL_IDPTR;
		CData *pData;
		
		if( NULL == ptcClassName )
		{
			HYSELECT_THROW( _T("Internal error: 'ptcClassName' can't be NULL.") );
		}
		else if( _T('\0') == *ptcClassName )
		{
			HYSELECT_THROW( _T("Internal error: 'ptcClassName' can't be empty.") );
		}
		
		if( NULL != ptcID && _T('\0') != *ptcID )
		{
			if( _tcslen( ptcID ) > _ID_LENGTH )
			{
				HYSELECT_THROW( _T("Object ID (%s) is too long (%i > %i)."), ptcID, _tcslen( ptcID ), _ID_LENGTH );
			}
			
			if( _T('\0') == *Get( ptcID ).ID )
			{
				Modified(); 
				pData = CData::New( ptcClassName, this, ptcID );
				
				if( NULL == pData )
				{
					HYSELECT_THROW( _T("Internal error: 'pData' instantiation error .") );
				}

				idptr = pData->GetIDPtr();
				
				// 'Insert()' returns 'true' is all is OK, 'false' otherwise.
				m_DataSet.Insert( idptr );
				return;
			}
			else
			{
				TRACE1( "Attempt to create a CData derived object with already existing ID=%s\n", ptcID );
				HYSELECT_THROW( _T("Object ID '%s' is already existing in the database."), ptcID );
			}
		}
		
		Modified(); 
		
		pData = CData::New( ptcClassName, this, m_DataSet.CreateID() );

		if( NULL == pData )
		{
			HYSELECT_THROW( _T("Internal error: 'pData' instantiation error .") );
		}

		idptr = pData->GetIDPtr();
		
		// 'Insert()' returns 'true' is all is OK, 'false' otherwise.
		if( _T('{') == *idptr.ID )
		{
			TRACE1( "Create object : %s\n", idptr.ID );
		}
		
		m_DataSet.Insert( idptr );
	}
	HYSELECT_CATCH( clHySelectException, _T("Error in 'CDataBase::CreateObject'.") )
}

int CDataBase::DeleteObject( IDPTR &ExtID, bool bForce )
{
	if( _T('{') == *ExtID.ID )
	{
		TRACE1( "Delete object : %s\n", ExtID.ID );
	}

	if( false == bForce && ExtID.MP->IsLocked() )
	{
		return 0;
	}

	delete m_DataSet.Remove( ExtID.ID ).MP;
	ExtID = _NULL_IDPTR;
	Modified(); 

	return 1;
}

int CDataBase::DeleteObjectRecursive( CData *pData, bool bForce )
{
	if( NULL == pData )
	{
		return 0;
	}

	CTable *pTab = dynamic_cast<CTable *>( pData );
	
	// If it's a table delete all included objects.
	if( NULL != pTab )
	{
		for( IDPTR IDPtr = pTab->GetFirst(); NULL != IDPtr.MP; IDPtr = pTab->GetNext( IDPtr.MP ) )
		{
			DeleteObjectRecursive( IDPtr.MP, bForce );
		}
	}

	if( false == bForce && pData->IsLocked() )
	{
		return 0;
	}

	IDPTR IDPtr = pData->GetIDPtr();
	
	if( NULL != IDPtr.PP )
	{
		IDPtr.PP->Remove( IDPtr );
	}

	return DeleteObject( IDPtr, bForce );
}

void CDataBase::MakeEmpty()
{
	// Warn each 'CData' derived objects about the database destruction.
	IDPTR ExtID = _NULL_IDPTR;

	for( ExtID = m_DataSet.GetFirst(); _T('\0') != *ExtID.ID; ExtID = m_DataSet.GetNext() )
	{
		ExtID.MP->OnDatabaseDestroy();
	}

	// Unlock the root table.
	if( NULL != m_pRootTable )
	{
		IDPTR idptr = m_pRootTable->GetIDPtr();

		if( _T('\0') != *idptr.ID )
		{
			m_pRootTable->Unlock( m_pRootTable->GetIDPtr() );
		}

		m_pRootTable = NULL;
	}
	
	// Delete all 'CData' derived objects.
	for( ExtID = m_DataSet.GetFirst(); _T('\0') != *ExtID.ID; ExtID = m_DataSet.GetNext() )
	{	
		if( NULL != ExtID.MP )
		{
			delete ExtID.MP;
		}
	}
	
	// Reset the 'CDataList' object.
	m_DataSet.MakeEmpty();
	Modified(); 
	
	// Unknown version identification.
	*m_tcVersion = '\0';
}

// Scan all DataSet Item and check if the Owner_ID exist
void CDataBase::CheckCrossReferenceID()
{
	for( IDPTR ExtID = m_DataSet.GetFirst(); _T('\0') != *ExtID.ID; ExtID = m_DataSet.GetNext() )
	{
		CData::OWNERS_ID *pOID = ExtID.MP->m_pOwners;

		for( ; pOID; pOID = pOID->m_pNext )
		{
			if( _T('\0') != *m_DataSet.GetIDPtrFromID( pOID->ID ).ID )
			{
				ASSERT( 0 );
			}
		}
	}		
}

#ifndef TACBX //TACBX////////////////////////////////////////////////////////
void CDataBase::RemoveCDataObject( CArray<CData *> *pAr )
{
	// Remove objects from TADB.
	for( int i = 0; i < pAr->GetCount(); i++ )
	{
		IDPTR IDPtr = pAr->GetAt( i )->GetIDPtr();
		IDPtr.PP->Remove( IDPtr );
		VERIFY( DeleteObject( IDPtr ) );
	}

	pAr->RemoveAll();
}
#endif //TACBX////////////////////////////////////////////////////////

// This function is used to write a file with a CRC at the end 
void CDataBase::Write( char *fn )
{
#if defined( TACBX ) && !defined( WIN32 )
	// TA-Scope running on the handheld.
	CRamDisk clRamDisk( true );
	
	if( 0 != clRamDisk.OpenFile( fn, 'w' ) )
	{
		return;
	}
	
	Write( clRamDisk );
	clRamDisk.CloseFile();
#else
	// Common code to the TA-Scope running on Windows and HySelect.
	std::ofstream outf( fn, std::ofstream::out | std::ofstream::binary | std::ofstream::trunc );
	Write( outf );
	outf.close();
#endif
}

void CDataBase::WriteFileIdentificator( OUTSTREAM outf, short nVersion )
{
	// Version number.
	outf.write( (const char *)(LPCTSTR)&nVersion, sizeof( nVersion ) );

#ifndef TACBX
#pragma warning( disable : 4309 )
#endif	

	// Write identification.
	const TCHAR tcFileIdentificator[] = TAFILEKEY;
	outf.write( (const char *)tcFileIdentificator, ( (int)_tcslen( tcFileIdentificator ) + 1 ) * sizeof( TCHAR ) );

#ifndef TACBX
#pragma warning( default : 4309 )
#endif	
}

void CDataBase::WriteHeader( OUTSTREAM outf )
{
	// Add version.
	m_clDBFileHeader.SetVersion( m_tcVersion );

	// Add UID if exist.
	m_clDBFileHeader.SetUID( GetUID().c_str() );

	m_clDBFileHeader.Write( outf );
}

void CDataBase::WriteObjects( OUTSTREAM outf )
{
	// Write all the objects
	BYTE OneMore = 0x26/*'&'*/;

	for( IDPTR IDPtr = m_DataSet.GetFirst(); _T('\0') != *IDPtr.ID; IDPtr = m_DataSet.GetNext() )
	{
		if( true == g_bWriteTACBXMode )
		{
			if( 0 == IDPtr.MP->GetTACBXVer() )
			{
				continue;	// Skip object
			}

			if( false == IDPtr.MP->IsExistInTASCOPE() )
			{
				continue;
			}
		}

		outf.write( (char*)&OneMore, sizeof( BYTE ) );
		IDPtr.MP->DumpToDisk( outf );
	}

	OneMore = 0x23/*'#'*/; // end marker
	outf.write( (char*)&OneMore, sizeof( BYTE ) );
	
	// Write the root table ID.
	WriteString( outf, m_pRootTable->GetIDPtr().ID );
}

void CDataBase::Write( OUTSTREAM outf )
{
	_string str = _string( ( (CDB_MultiString *)GetpTADB()->Get( _T("TADB_VERSION") ).MP )->GetString( 0 ) );
	RemoveTChar( &str, _T('.') );
	
	short nVersion = _ttoi( str.c_str() );
	ASSERT( nVersion > 0 );

	WriteFileIdentificator( outf, nVersion );
	WriteHeader( outf );
	WriteObjects( outf );
}

#ifndef TACBX
int CDataBase::CheckDBFile( INPSTREAM inpf )
{
	int iReturnCode = CDataBase::Ok;
	
	// Version number.
	short nVersion;
	inpf.read( (char *)&nVersion, sizeof( nVersion ) );
	
#pragma warning( disable : 4309 )
	const TCHAR tcFileIdentificator[] = TAFILEKEY;
#pragma warning( default : 4309 )

	TCHAR tcCode[32];
	memset( tcCode, 0, sizeof( tcCode ) );
	g_bSBCSFile = false;
	inpf.read( (char *)tcCode, ( (int)_tcslen( tcFileIdentificator ) + 1 ) * sizeof( TCHAR ) );
	
	if( 0 != _tcscmp( tcCode, tcFileIdentificator ) )
	{
		const BYTE arbFileIdSBCS[] = TAFILEKEY;
		BYTE *pByte = (BYTE *)tcCode;
		unsigned int i;

		for( i = 0; i < sizeof( arbFileIdSBCS ); i++ )
		{
			if( pByte[i] != arbFileIdSBCS[i] )
			{
				break;
			}
		}

		if( i < sizeof( arbFileIdSBCS ) )
		{
			TRACE( _T("Database Binary File is not recognised !\n") );
			return CDataBase::Invalid;
		}
		else
		{
			// SBCS File open with Unicode version.
			TRACE( _T("Database Binary File is SBCS !\n") ); 
			inpf.seekg( sizeof( nVersion ) + sizeof( arbFileIdSBCS ) + 1 );
			g_bSBCSFile = true;
			iReturnCode = CDataBase::OkSbcs;
		}
	} 
	
	TCHAR tcFormatVersion[_DATABASE_VERSION_LENGTH + 1];
	memset( tcFormatVersion, 0, sizeof( tcFormatVersion ) );

	// Read format version.
	inpf.read( (char*)tcFormatVersion, sizeof( tcFormatVersion ) );
	int iFVersion  = _ttoi( tcFormatVersion );

	// If the file format version is more recent than the current Format Version the file can't be opened.
	if( iFVersion > _ttoi( CTADATASTRUCT_FORMAT_VERSION ) )
	{
		return CDataBase::TooRecent;
	}
	
	if( CDataBase::Ok == iReturnCode )
	{
		// Read end of file signature.
		CString strEndFileSignature = _T("{ROOT_TABLE}");
		TCHAR tcEndFileContent[_DATABASE_VERSION_LENGTH];
		memset( tcEndFileContent, 0, sizeof( tcEndFileContent ) );
		
		inpf.seekg( 0, inpf.end );
		long lLength = (long)inpf.tellg();
		inpf.seekg( lLength - 1 * ( strEndFileSignature.GetLength() * sizeof( TCHAR ) ) );
		inpf.read( (char *)tcEndFileContent, strEndFileSignature.GetLength() * sizeof( TCHAR ) );
		
		CString str = tcEndFileContent;

		if( 0 != strEndFileSignature.Compare( tcEndFileContent ) )
		{
			// Invalid signature.
			return CDataBase::Invalid;
		}
	}

	return iReturnCode;
}
#endif
 
#ifndef TACBX
void CDataBase::Read( TCHAR *fn )
#else
void CDataBase::Read( char *fn )
#endif
{
	HYSELECT_TRY
	{
		std::ifstream inbuf( fn, std::ifstream::in | std::ifstream::binary );

		if( std::ifstream::failbit == ( inbuf.rdstate() & std::ifstream::failbit ) )
		{
			HYSELECT_THROW( _T("Can't read access to the file '%s'."), fn );
		}

		// Get file size.
		inbuf.seekg( 0, std::ios_base::end );
		long lFileLength = (long)inbuf.tellg();
		inbuf.seekg( 0 ); 

		// Goto beginning for reading the file.
		if( lFileLength <= 0 )
		{
			inbuf.close();
			HYSELECT_THROW( _T("File '%s' is empty."), fn );
		}

#if defined( TACBX ) && !defined( WIN32 )
		// TASCOPE-76: we use now an internal buffer in the 'CRamDisk' class.
		CRamDisk clRamDisk( true );
	
		if( 0 != clRamDisk.OpenFile( fn, 'r') )
		{
			return;
		}
	
		Read( clRamDisk );
	
		if( 0 != clRamDisk.CloseFile() )
		{
			return;
		}
#else
		Read( inbuf );
		inbuf.close();
#endif

	}
	HYSELECT_CATCH( clHySelectException, _T("Error in 'CDataBase::Read'.") )
}

short CDataBase::ReadFileIdentificator( INPSTREAM inpf )
{
	// Version number.
	short nVersion = 0;
	inpf.read( reinterpret_cast<char *>(&nVersion), sizeof( nVersion ) );

	g_bSBCSFile = false;
	
	// Read identification.
#ifndef TACBX
#pragma warning( disable : 4309)
#endif

	const TCHAR tcFileIdentificator[] = TAFILEKEY;
#ifndef TACBX
#pragma warning( default : 4309)
#endif
	
	TCHAR tcCode[16];
	memset( tcCode, 0, sizeof( tcCode ) );
	int iSize = ( (int)_tcslen( tcFileIdentificator ) + 1 ) * sizeof( TCHAR );
	inpf.read( (char*)tcCode, iSize );

	if( 0 != _tcscmp( tcCode, tcFileIdentificator ) )
	{
		// HYS-1866
#ifndef TACBX		
		const BYTE arbFileIdSBCS[] = TAFILEKEY;
		BYTE* pByte = (BYTE*)tcCode;
		unsigned int i;

		for( i = 0; i < sizeof( arbFileIdSBCS ); i++ )
		{
			if( pByte[i] != arbFileIdSBCS[i] )
			{
				break;
			}
		}

		if( i < sizeof( arbFileIdSBCS ) )
		{
			TRACE( _T( "Database Binary File is not recognised !\n" ) );
			return false;
		}
		else
		{
			// SBCS File....
			TRACE( _T( "Database Binary File is SBCS !\n" ) );
			inpf.seekg( sizeof( nVersion ) + sizeof( arbFileIdSBCS ) + 1, std::ifstream::beg );
			g_bSBCSFile = true;
		}
#else
		// TACOPE76: no more use of 'SBCS' (Simply Byte Character String) file.
		TRACE( _T("Database Binary File is not recognised !\n") );
		return false;
#endif
	}

	return nVersion;
}

bool CDataBase::ReadHeader( INPSTREAM inpf, short nVersion )
{
	bool bReturn = m_clDBFileHeader.Read( inpf, nVersion );

	if( true == bReturn )
	{
		SetUID( m_clDBFileHeader.GetUID() );
	}

	return bReturn;
}

void CDataBase::ReadObjects( INPSTREAM inpf, short nVersion )
{
	HYSELECT_TRY
	{
		// Pay attention: this method is called from the 'CTADatastruct::Read', 'CTADataStructX::Read' and 'CTAMetaData::Read' methods.
		// Thus 'nVersion' can be different! This is why we have now the 'm_eChildID' variable that help us.

		// Load DB from independent task: give hand to SMX periodically.
		int iCounter = 0;
	
		// Read objects from disk.
		BYTE bOneMore = 0;
	
		for( inpf.read( (char *)&bOneMore, sizeof( BYTE ) ); 0x26/*'&'*/ == bOneMore; inpf.read( (char*)&bOneMore, sizeof( BYTE ) ) )
		{
			CData::ExtractFromDisk( inpf, this );

			iCounter++;

			if( iCounter >= 10 )
			{
#ifndef WIN32
				//bump_task(self, PRI_NOCHG);
				// feed watchdog (radio module)
				AT91C_BASE_US0->US_THR = 0x37;

				// feed watchdog (Internal AT91)
				AT91C_BASE_WDTC->WDTC_WDCR = AT91C_WDTC_WDRSTT | AT91C_WDTC_KEY;
#endif
				iCounter = 0;
			}
		}
	
		// On 2013-11-28, "PARAM_SSEL" table in "PARAM_TABLE" table becomes "PARAM_INDSEL".
		// For 'CTADatastruct' version before 4.0.1.5, we need to copy "PARAM_SSEL" into "PARAM_INDSEL".
		// Remark: need to be done before calling the 'CheckAndTransferTmpObj' method.
		if( CDataBase::TADatastruct == m_eChildID && nVersion < 4015 )
		{
			CTable *pclParamTable = dynamic_cast<CTable*>( m_DataSet.GetIDPtrFromID( _T("PARAM_TABLE") ).MP );

			if( NULL != pclParamTable )
			{
				pclParamTable->ChangeIDInTempTable( _T("PARAM_SSEL"), _T("PARAM_INDSEL") );
			}
		}
	
		// Connect the root table.
		TCHAR tcID[_ID_LENGTH + 1];
	
		if( false == ReadString( inpf, tcID, sizeof( tcID ) ) )
		{
			HYSELECT_THROW( _T("Can't read root table ID at the end of the file.") );
		}
	
		m_pRootTable = (CTable*)( m_DataSet.GetIDPtrFromID( tcID ).MP );
	
		if( NULL == m_pRootTable )
		{
			HYSELECT_THROW( _T("Root table ID (%s) is defined but not found in the database."), tcID );
		}

		// Scan all DataSet CTable Object to verify if there are no temporary CTable link.
		for( IDPTR IDPtr = m_DataSet.GetFirst(); _T('\0') != *IDPtr.ID; IDPtr = m_DataSet.GetNext() )
		{
			if( true == IDPtr.MP->IsaTable() )
			{
				( (CTable *)IDPtr.MP )->CheckAndTransferTmpObj();	
			}
		}

#ifndef TACBX
		VerifyTable();
#endif

		// Not modified when read.
		Modified( 0 );
	}
	HYSELECT_CATCH( clHySelectException, _T("Error in 'CDataBase::ReadObjects'.") )
}

void CDataBase::Read( INPSTREAM inpf )
{
	HYSELECT_TRY
	{
		short nVersion = ReadFileIdentificator( inpf );

		if( nVersion <= 0 )
		{
			HYSELECT_THROW( _T("Database version can't be below or equal to 0.") );
		}
	
		// Clear the actual database content.
		MakeEmpty();

#ifndef TACBX
		swprintf_s( m_tcVersion, _DATABASE_VERSION_LENGTH + 1, L"%d", nVersion );
#else
		_string str = int2wstr( nVersion );
		_tcsncpy_s( m_tcVersion, _DATABASE_VERSION_LENGTH + 1, str.c_str(), str.size() );
#endif
	
		ReadHeader( inpf, nVersion );
		ReadObjects( inpf, nVersion );
	}
	HYSELECT_CATCH( clHySelectException, _T("Error in 'CDataBase::Read'.") )
}

#ifndef TACBX //TACBX////////////////////////////////////////////////////////
LPTSTR CDataBase::Append( INPSTREAM inpf, unsigned short usLineStart, bool bReadingObjectFromDeletedFile )
{
	HYSELECT_TRY
	{
		// Used to add some objects at the end of the current database
		// each new CData object must be announced with the word "new:" (lower case)
		// the CData derived object header and information will follow (see CData::ExtractFromTextFile).
		// The file will finish by the instruction "ROOT_TABLE(id)" where id is the root table ID.

		unsigned short usLineCount = usLineStart;

		// Read the CData derived objects.
		int iObjectNumber = 0;
		static TCHAR *ptcLine = NULL;

		do
		{
			ptcLine = CData::ReadTextLine( inpf, &usLineCount );

			if( 0 != _tcsncmp( ptcLine, _T("new:"), 4 ) && 0 != _tcsncmp( ptcLine, _T("*new:"), 5 ) && 0 != _tcsncmp( ptcLine, _T("##"), 2 ) )
			{
				break;
			}

			// Enum definition parse and store it into m_MapOfEnum
			if( 0 == _tcsncmp( ptcLine, _T("##"), 2 ) )
			{
				CString str = ptcLine;
				str.Delete( 0, 1 );		// Remove #
				int iPos = str.Find( _T('=') );
			
				CString strName = str.Left( iPos );
				strName.TrimLeft();
				strName.TrimRight();
				strName.MakeLower();
			
				CString strval = str.Right( str.GetLength() - ( iPos + 1 ) );
				strval.TrimLeft();
				strval.TrimRight();
				int iVal = _ttoi( (LPCTSTR)strval );

				m_MapOfEnum.SetAt( strName, (void *)iVal );
			}
			else
			{
				CData::ExtractFromTextFile(inpf, this, bReadingObjectFromDeletedFile, &usLineCount, ptcLine +4, _T('*') == *ptcLine );
				iObjectNumber++;
			}

		}while( 1 );

		TRACE( _T("\nObjects : %d\n"), iObjectNumber ); 
	
		// Test file validity.
		if( 0 != _tcscmp( ptcLine, _T("ROOT_TABLE(ROOT_TABLE)") ) )
		{
			HYSELECT_THROW( _T("Each database text file must end with 'ROOT_TABLE(ROOT_TABLE)' (pos: %i)."), usLineCount );
		}

		return ptcLine;
	}
	HYSELECT_CATCH( clHySelectException, _T("Error in 'CDataBase::Append'.") )
}

void CDataBase::CrossVerifyID()
{
	HYSELECT_TRY
	{
		IDPTR IDPtr = _NULL_IDPTR;
		unsigned short usPos;

		for( IDPtr = m_DataSet.GetFirst( usPos ); NULL != IDPtr.MP; IDPtr = m_DataSet.GetNext( usPos ) )
		{
			if( true == IDPtr.MP->IsAvailable() )
			{
				IDPtr.MP->CrossVerifyID();
			}
		}
	}
	HYSELECT_CATCH( clHySelectException, _T("Error in 'CDataBase::CrossVerifyID'.") )
}

void CDataBase::ReadText( INPSTREAM inpf )
{
	try
	{
		// CDataBase text file format :
		// The first line is "DATABASE_VERSION(string)" where string is a version identificator.
		// After, it is a list of CData derived object.
		// each new CData object must be announced with the word "new:" (lower case)
		// the CData derived object header and information will follow (see CData::ExtractFromTextFile).
		// The file will finish by the instruction "ROOT_TABLE(id)" where id is the root table ID.

		// Clear the actual database content
		MakeEmpty();

		// Reset Map of Enum
		RemoveAllEnumDefinition();
		unsigned short usLineCount = 0;
	
		// Read the database version.
		TCHAR *ptcLine = CData::ReadTextLine( inpf, &usLineCount );

		TCHAR *ptcCursor = NULL;
	
		for( ptcCursor = ptcLine; _T('\0') != *ptcCursor && _T('(') != *ptcCursor; ptcCursor++ );
	
		if( _T('\0') == *ptcCursor )
		{
			HYSELECT_THROW( _T("Unexpected end of line before reading the 'DATABASE_VERSION' key (pos: %i)."), usLineCount );
		}
	
		*ptcCursor = _T('\0');

		if( 0 != _tcscmp( _T("DATABASE_VERSION"), ptcLine ) )
		{
			HYSELECT_THROW( _T("First line must contains the 'DATABASE_VERSION' key (pos: %i)."), usLineCount );
		}

		for( ptcLine = ++ptcCursor; _T('\0') != *ptcCursor && _T(')') != *ptcCursor; ptcCursor++ );

		if( _T('\0') == *ptcCursor )
		{
			HYSELECT_THROW( _T("Unexpected end of line before reading database version (pos: %i)."), usLineCount );
		}

		*ptcCursor = '\0';

		if( _tcslen( ptcLine ) > _DATABASE_VERSION_LENGTH )
		{
			HYSELECT_THROW( _T("Database version (%s) is too long (%i > %i) (pos: %i)."), ptcLine, _tcslen( ptcLine ), _DATABASE_VERSION_LENGTH, usLineCount );
		}

		_tcsncpy_s( m_tcVersion, SIZEOFINTCHAR( m_tcVersion ), ptcLine, _DATABASE_VERSION_LENGTH );
		m_tcVersion[_DATABASE_VERSION_LENGTH] = _T('\0');
	
		// Read all objects.
		ptcLine = Append( inpf, usLineCount );

		// Identify the root table.
		for( ptcCursor = ptcLine; _T('\0') != *ptcCursor && _T('(') != *ptcCursor; ptcCursor++ );
	
		if( _T('\0') == *ptcCursor )
		{
			HYSELECT_THROW( _T("Unexpected end of line before reading the 'ROOT' table (pos: %i)."), usLineCount );
		}

		*ptcCursor = _T('\0');

		if( 0 != _tcscmp( _T( "ROOT_TABLE" ), ptcLine ) )
		{
			HYSELECT_THROW( _T("Second line must contains the 'ROOT_TABLE' table (pos: %i)."), usLineCount );
		}

		for( ptcLine = ++ptcCursor; _T('\0') != *ptcCursor && _T(')') != *ptcCursor; ptcCursor++ );

		if( _T('\0') == *ptcCursor )
		{
			HYSELECT_THROW( _T("Unexpected end of line before reading the ROOT table ID (pos: %i)."), usLineCount );
		}

		*ptcCursor = _T('\0');
		IDPTR IDPtr = Get( ptcLine );
	
		if( _T('\0') == *IDPtr.ID )
		{
			HYSELECT_THROW( _T("The ROOT table ID can't be empty (pos: %i)."), usLineCount );
		}
	
		m_pRootTable = (CTable*)( IDPtr.MP );
		m_pRootTable->Lock( IDPtr );

		// Perform a check. List all object which are not locked.
#if defined( _DEBUG )
		TRACE( _T("Non locked objects list : ") );
		int iNone = 1;

		for( IDPtr = m_DataSet.GetFirst(); NULL != IDPtr.MP; IDPtr = m_DataSet.GetNext() )
		{
			if( 0 == IDPtr.MP->IsLocked() )
			{
				TRACE( _T("\n%s from class %s"), IDPtr.ID, IDPtr.MP->GetClassName() );
				iNone = 0;
			}
		}

		if( iNone != 0 )
		{
			TRACE( _T("NONE") );
		}
	
		TRACE( _T("\n") );
#endif
	
	}
	catch( CHySelectException &clHySelectException )
	{
		clHySelectException.AddMessage( _T("Error in 'CDataBase::ReadText'."), __LINE__, __FILE__ );
		throw;
	}
}

// When all object are defined enum map can be erased.
void CDataBase::RemoveAllEnumDefinition()
{
	// All object are defined, erase m_MapOfEnum.
	m_MapOfEnum.RemoveAll();
}

void CDataBase::DropData( OUTSTREAM outf )
{
	CString strTab = _T("");

	// Database info.
	CString str;
	str.Format( _T("DATABASE VERSION IDENTIFICATOR is %s\n"), m_tcVersion );
	WriteFormatedStringA2( outf, str, strTab );

	if( NULL != m_pRootTable )
	{
		str.Format( _T("ROOT TABLE ID is %s\n"), CString( m_pRootTable->GetIDPtr().ID ) );
	}
	else
	{
		str.Format( _T("There is no ROOT TABLE.%s\n"), CString( m_pRootTable->GetIDPtr().ID ) );
	}

	WriteFormatedStringA2( outf, str, strTab );

	// Write root table.
	int iLevel = 0;

	if( NULL != m_pRootTable )
	{
		m_pRootTable->DropData( outf, &iLevel );
	}
}

int CDataBase::ReadEnum( CString str )
{
	if( true == str.IsEmpty() )
	{
		ASSERTA_RETURN( INT_MAX );
	}

	if( _T('#') != str.GetAt( 0 ) )
	{
		ASSERTA_RETURN( INT_MAX );
	}

	CStringArray arStr;

	// Combined '|' enum
	if( -1 != str.Find( _T('|') ) )
	{
		CString strToken;
		int iCurPos = 0;
		strToken = str.Tokenize( _T("|"), iCurPos );

		while( strToken != _T( "" ) )
		{
			strToken.MakeLower();
			arStr.Add( strToken );
			strToken = str.Tokenize( _T("|"), iCurPos );
		}
	}
	else
	{
		str.MakeLower();
		arStr.Add( str );
	}

	void *pValue;
	int retValue = 0;

	for( int i = 0; i < arStr.GetSize(); i++ )
	{
		if( FALSE == m_MapOfEnum.Lookup( (LPCTSTR)arStr[i], pValue ) )
		{
			ASSERTA_RETURN( INT_MAX );
		}

		retValue = retValue | (int)pValue;
	}

	return retValue;
}

int CDataBase::ReadEnumLine(INPSTREAM  inpf, unsigned short *pusLineCount)
{
	try
	{
		TCHAR *ptcLine = CData::ReadTextLine( inpf, pusLineCount );
		return ReadEnum( ptcLine );
	}
	catch( CHySelectException &clHySelectException )
	{
		clHySelectException.AddMessage( _T("Error in 'CDataBase::ReadEnumLine'."), __LINE__, __FILE__ );
		throw;
	}
}

void CDataBase::ParseEnumStrLine( INPSTREAM  inpf, CString strLine, int &iKey, CString &strEnum )
{
	try
	{
		if( true == strLine.IsEmpty() )
		{
			HYSELECT_THROW( _T("Line can not be empty.") );
		}

		// Found enum key.
		int iPos = strLine.Find('"');

		if( iPos < 0 )
		{
			HYSELECT_THROW( _T("A line must contain a enum variable (#EnumName) following by a value between double quotes.") );
		}
	
		CString sKey = strLine.Left( iPos );
		sKey.TrimLeft();
		sKey.TrimRight();
		sKey.MakeLower();

		void *pValue;

		if( FALSE == m_MapOfEnum.Lookup( (LPCTSTR)sKey, pValue ) )
		{
			HYSELECT_THROW( _T("The '%s' enum value is not defined in the 'tadb.txt' file."), sKey );
		}

		iKey = (int)pValue;
		strEnum = strLine.Mid( iPos, strLine.GetLength() - iPos );
		strEnum = strEnum.Mid( 1, strEnum.GetLength() - 2 );
	}
	catch( CHySelectException &clHySelectException )
	{
		clHySelectException.AddMessage( _T("Error in 'CDataBase::ParseEnumStrLine'."), __LINE__, __FILE__ );
		throw;
	}
}

void CDataBase::ApplyFilters()
{
	try
	{
		CArray<CData *> Ar;

		// Loop on all objects in the database.
		for( IDPTR IDPtr = m_DataSet.GetFirst(); _T('\0') != *IDPtr.ID; IDPtr = m_DataSet.GetNext() )
		{
			// Skip ROOT_TABLE.
			if( 0 != _tcscmp( IDPtr.ID, _T("ROOT_TABLE") ) )
			{
				POSITION Pos = NULL;

				// Check all filter for the current class name.
				do
				{
					int iFilter = CFilterTab::FS_NotFiltered;				// by default do nothing
					iFilter = FilterTab.IsFiltered( Pos, IDPtr.MP->GetClassName(), IDPtr.PP->GetIDPtr().ID, IDPtr.ID );

					// Pos is now on the next element.
					if( CFilterTab::FS_MustBeDeleted == iFilter )
					{
						Ar.Add( IDPtr.MP );
						Pos = NULL;
					}
					else if( CFilterTab::FS_Filtered == iFilter )
					{
						// Call object's filter function with current filter string.
						// In regards to the object, can be 'CData', 'CDB_Product', 'CDB_TAProduct', 'CDB_Actuator' or 'CDB_AlienProduct'.
						IDPtr.MP->CheckFilter( &FilterTab, FilterTab.GetFieldLineString( Pos ) );
					}

				}while( NULL != Pos );
			}
		}
		
		RemoveCDataObject( &Ar );
	}
	catch( CHySelectException &clHySelectException )
	{
		clHySelectException.AddMessage( _T("Error in 'CDataBase::ApplyFilters'."), __LINE__, __FILE__ );
		throw;
	}
}

void CDataBase::VerifyTable()
{
	try
	{
		// Scan all DataSet CTable Object to cross-verify links
		int iNumOfTables = 0;
		int iNumOfObjects = 0;
		unsigned short usPos;

		for( IDPTR IDPtr = m_DataSet.GetFirst( usPos ); _T('\0') != *IDPtr.ID; IDPtr = m_DataSet.GetNext( usPos ) )
		{
			iNumOfObjects++;

			if( true == IDPtr.MP->IsaTable() )
			{
				iNumOfTables++;
				CTable *pTable = (CTable *)( IDPtr.MP );

				for( IDPTR ChildIDPtr = pTable->GetFirst(); _T('\0') != *ChildIDPtr.ID; ChildIDPtr = pTable->GetNext() )
				{
					if( true == pTable->GetTrueParent() )
					{
						// If the current table is the true parent, 'PP' of the object pointed must be the same as this table.
						if( ChildIDPtr.MP->GetIDPtr().PP != IDPtr.MP )
						{
							HYSELECT_THROW( _T("The PP of the current object (%s) doesn't point on the current table (%s) that is however the true parent."), ChildIDPtr.ID, IDPtr.ID );
						}
					}
					else
					{
						if( ChildIDPtr.MP->GetIDPtr().PP == IDPtr.MP )
						{
							HYSELECT_THROW( _T("The PP of the current object (%s) points on the current table (%s) that is not the true parent."), ChildIDPtr.ID, IDPtr.ID );
						}
					}
				}
			}
		}
	}
	catch( CHySelectException &clHySelectException )
	{
		clHySelectException.AddMessage( _T("Error in 'CDataBase::VerifyTable'."), __LINE__, __FILE__ );
		throw;
	}
}

#ifdef DEBUG
void CDataBase::ExportDataSet( CString strFileName, bool bOnlyNoOwner )
{
	CFile cFile;

	if( 0 == cFile.Open( strFileName, CFile::modeCreate | CFile::modeWrite ) )
	{
		return;
	}

	for( IDPTR IDPtr = m_DataSet.GetFirst(); _T('\0') != *IDPtr.ID; IDPtr = m_DataSet.GetNext() )
	{
		USES_CONVERSION;

		if( true == bOnlyNoOwner && NULL != IDPtr.MP && NULL != (CData*)( IDPtr.MP )->m_pOwners )
		{
			continue;
		}

		CString str;
		str.Format( _T("%s\n"), IDPtr.ID );
		cFile.Write( T2A( str.GetBuffer() ), str.GetLength() );
	}

	cFile.Close();
}

void CDataBase::ExportAllArticleNumber( CString strFileName )
{
	CFile cFile;

	if( 0 == cFile.Open( strFileName, CFile::modeCreate | CFile::modeWrite ) )
	{
		return;
	}

	for( IDPTR IDPtr = m_DataSet.GetFirst(); _T('\0') != *IDPtr.ID; IDPtr = m_DataSet.GetNext() )
	{
		USES_CONVERSION;

		CString strFullArticleNumber = ( (CData *)( IDPtr.MP ) )->GetArtNum();
		strFullArticleNumber.Trim();

		if( true == strFullArticleNumber.IsEmpty() )
		{
			continue;
		}

		CString strArticleCleaned = _T("");
	
		for( int i = 0; i < strFullArticleNumber.GetLength(); ++i )
		{
			if( 0 != _istalnum( strFullArticleNumber.GetAt( i ) ) )
			{
				strArticleCleaned.AppendChar( strFullArticleNumber.GetAt( i ) );
			}
		}

		if( false == strArticleCleaned.IsEmpty() )
		{
			strArticleCleaned.Append( _T(" - ") );
			strArticleCleaned.Append( IDPtr.ID );
			strArticleCleaned.AppendChar( _T('\n' ) );
			cFile.Write( T2A( strArticleCleaned.GetBuffer() ), strArticleCleaned.GetLength() );
		}
	}

	cFile.Close();
}
#endif

void CDataBase::SearchByArticleNumber( CString strArticleNumber, CArray <CString> *pArrstr )
{
	int nField = 0;

	pArrstr->RemoveAll();

	// Here we browse the DataBase.
	for( IDPTR IDPtr = m_DataSet.GetFirst(); _T('\0') != *IDPtr.ID; IDPtr = m_DataSet.GetNext() )
	{
		USES_CONVERSION;
		CString strAN = _T("");
		CString strANf = _T("");
		CString strFullArticleNumber = ((CData*)(IDPtr.MP))->GetArtNum();
		CString strID = IDPtr.ID;

		if( true == strFullArticleNumber.IsEmpty() )
		{
			continue;
		}

		// if the strings match, ID is kept
		StrCleanArticleNumber( strArticleNumber, &strAN );
		StrCleanArticleNumber( strFullArticleNumber, &strANf );
		
		if( true == WildcardMatch( strANf, strAN ) )
		{
			nField = pArrstr->GetSize();
			pArrstr->SetSize( nField + 1 );
			pArrstr->SetAt( nField, strID );
		}
	}
}

void CDataBase::VerifyDataSetIntegrity()
{
	std::vector<CData *> vecOrphanList;

	// HYS-853: When performing auto-save process we have to delete orphan
	// parents before single orphan.
	std::vector<CData *> vecOrphanListNoChild;
	IDPTR IDPtrPrevious;
	
	for( IDPTR IDPtr = m_DataSet.GetFirst(); _T('\0') != *IDPtr.ID; IDPtr = m_DataSet.GetNext() )
	{
		if( NULL == IDPtr.MP->m_pOwners )
		{
			if( NULL != dynamic_cast<CDS_HydroMod *>( IDPtr.MP ) )
			{
				if( NULL != dynamic_cast<CDS_HydroMod *>( IDPtr.MP )->GetFirst().MP )
				{
					vecOrphanList.push_back( IDPtr.MP );
				}
				else
				{
					vecOrphanListNoChild.push_back( IDPtr.MP );
				}
			}
			else
			{
				ASSERT( 0 );
				vecOrphanList.push_back( IDPtr.MP );
			}
		}
	}
	
	for( int i = 0; i < (int)vecOrphanList.size(); i++ )
	{
		DeleteObjectRecursive( vecOrphanList.at( i ), true );
	}
	
	for( int i = 0; i < (int)vecOrphanListNoChild.size(); i++ )
	{
		DeleteObjectRecursive( vecOrphanListNoChild.at( i ), true );
	}
}

#endif //TACBX////////////////////////////////////////////////////////


//////////////////////////////////////////////////////////////////////////////////////////////
//
//	CData class
//	
//////////////////////////////////////////////////////////////////////////////////////////////

IMPLEMENT_DATA( CData )

CData::CData( CDataBase *pDataBase, LPCTSTR ptcID )
{
	ASSERT(pDataBase);
	m_Identificator = _NULL_IDPTR;
	_tcsncpy_s( m_Identificator.ID, SIZEOFINTCHAR( m_Identificator.ID ), ptcID, SIZEOFINTCHAR( m_Identificator.ID ) - 1 );
	m_Identificator.MP = this;
	m_Identificator.DB = pDataBase;
	m_pOwners = 0;
	m_bTable = false;
	m_bDeleted = false;
	m_bAvailable = true;		// 1 if this product is available in this (country) version
	m_bHidden = false;		// 1 if this product is Hidden
	m_usTACBXVer = 1;		// All object class are written by default
	m_bExistInTASCOPE = false;// Object itself is not written by default
}

CData::~CData()
{
	// delete the owner list
	OWNERS_ID *pOwner = m_pOwners;
	OWNERS_ID *pNextOwner = 0;

	while( NULL != pOwner )
	{
		pNextOwner = pOwner->m_pNext;
		delete pOwner;
		pOwner = pNextOwner;
	}

#if defined(_DEBUG) 
	// Normally, a CData object should not be destroyed if it is owned by others !
	if( NULL != m_pOwners )
	{
		TRACE( _T("Unlocked object is destroyed. ID=%s\n"), m_Identificator.ID );
	}
#endif

}

bool CData::IsSelectable( bool bForceForaNewPlant, bool bForHub, bool bIsForHMCalc, bool bIsforTools )
{
#ifndef TACBX //TACBX////////////////////////////////////////////////////////
	bool bIsSelectable = false;
	bool bDeleted = IsDeleted();	
	bool bCheckDisplayDeleted = false;
	if( false == bForceForaNewPlant || true == bIsforTools )
	{
		if( NULL != TASApp.GetpTADS()->GetpPersistData() && true == TASApp.GetpTADS()->GetpPersistData()->IsOldValveAreUsed() )
		{
			bDeleted = false;
		}
	}

	// HYS-1221 : Add bIsForHMCalc parameter to check if deleted products are available for HM Calc then they can be selectable
	if( true == bIsForHMCalc )
	{
		if( NULL != TASApp.GetpTADS()->GetpProjectParams() && NULL != TASApp.GetpTADS()->GetpProjectParams()->GetpHmCalcParams() 
			&& true == TASApp.GetpTADS()->GetpProjectParams()->GetpHmCalcParams()->IsOldProductsAvailableForHC() )
		{
			bCheckDisplayDeleted = true;
		}
	}

	bIsSelectable = ( true == IsAvailable( bForHub ) && false == bDeleted && ( false == IsHidden() || true == TASApp.IsHidingPswSet() ) );

	if( false == bIsSelectable && true == bCheckDisplayDeleted )
	{
		bIsSelectable = ( true == bDeleted && true == IsAvailable() && ( false == IsHidden() ) );
	}

	return bIsSelectable;
#else
	
	bool bShowDeleted = (CTAPersist::Get_ShowOldValve());
	bool bShowHidden =  (CTAPersist::Get_ShowAllValve());

	//Flag Showdeleted invalidate test on deleted (Show all deleted valve)
	//Flag ShowHidden invalidate tests on available and hidden flags 

	if( true == bShowHidden && true == bShowDeleted )
	{
		return true;
	}
	else if( true == bShowHidden )
	{
		return !IsDeleted();
	}
	else if( true == bShowDeleted )
	{
		return ( IsAvailable( bForHub ) && !IsHidden() );
	}
	else
	{
		return ( IsAvailable( bForHub ) && !IsDeleted() && !IsHidden() );
	}
#endif
}

CTable *CData::GetTrueOwner()
{
	// Check if owner list exists.
	if( NULL == m_pOwners || NULL == m_Identificator.DB )
	{
		return NULL;
	}
	
	CTable *pclTrueOwner = NULL;

	for( OWNERS_ID *pOwner = m_pOwners; NULL != pOwner && NULL == pclTrueOwner; pOwner = pOwner->m_pNext )
	{
		CTable *pclTable = dynamic_cast<CTable *>( m_Identificator.DB->Get( pOwner->ID ).MP );

		if( NULL == pclTable )
		{
			continue;
		}

		if( true == pclTable->GetTrueParent() )
		{
			pclTrueOwner = pclTable;
		}
	}

	return pclTrueOwner;
}

bool CData::FindOwner( LPCTSTR ptcID )
{
	if( NULL == this )
	{
		return false;
	}

	// Check if owner list exists.
	if( false == HasOwner() )
	{
		return false;
	}

	// Look for the input ID in the list and set return value.
	OWNERS_ID *pPrevious = 0;
	OWNERS_ID *pOwner;

	for( pOwner = m_pOwners; NULL != pOwner && 0 != _tcscmp( pOwner->ID, ptcID ); pOwner = pOwner->m_pNext )
	{
		pPrevious = pOwner;
	}

	if( NULL != pOwner )	// the ID is found
	{
		if( NULL != pPrevious )
		{
			return true;
		}
		else if( 0 == _tcscmp( m_pOwners->ID, ptcID ) )
		{
			return true;
		}
	}

	return false;
}

void CData::Lock( const IDPTR &LockerID )
{
	HYSELECT_TRY
	{
		// Check if the LockerID is from the same database
		if( LockerID.DB != m_Identificator.DB )
		{
			HYSELECT_THROW( _T("Internal error: Can't lock object '%s' created in a database different of the current one."), LockerID.ID );
		}
	
		// First look if the Locker object has not yet locked this
		// If not, create an OWNER_ID instance and add it in the list
		OWNERS_ID *pOwner;

		for( pOwner = m_pOwners; NULL != pOwner && 0 != _tcscmp( pOwner->ID, LockerID.ID ); pOwner = pOwner->m_pNext );
	
		if( NULL == pOwner )
		{	
			OWNERS_ID *pNewOwner = new OWNERS_ID;

			if( NULL == pNewOwner )
			{
				HYSELECT_THROW( _T("Internal error: 'pNewOwner' instantiation error.") );
			}

			_tcsncpy_s( pNewOwner->ID,SIZEOFINTCHAR( pNewOwner->ID ), LockerID.ID, _ID_LENGTH );
			pNewOwner->ID[_ID_LENGTH] = '\0';
			pNewOwner->m_pNext = m_pOwners;
			m_pOwners = pNewOwner;
		}
	}
	HYSELECT_CATCH( clHySelectException, _T("Error in 'CData::Lock'.") )
}

void CData::Unlock( const IDPTR &UnlockerID )
{
	if( NULL == this )
	{
		return;	// prevent from unlocking an already deleted object
	}
	
	// Check if the LockerID is from the same database.
	if( UnlockerID.DB != m_Identificator.DB )
	{
		TRACE( _T("Attempt to unlock with alien object\n") );
		return;
	}
	
	// Look for the unlocker ID in the list and delete it.
	OWNERS_ID *pPrevious = 0;
	OWNERS_ID *pOwner;
	
	for( pOwner = m_pOwners; NULL != pOwner && 0 != _tcscmp( pOwner->ID, UnlockerID.ID ); pOwner = pOwner->m_pNext )
	{
		pPrevious = pOwner;
	}
	
	if( NULL != pOwner )	// the ID is found
	{
		if( NULL != pPrevious )
		{
			pPrevious->m_pNext = pOwner->m_pNext;
		}
		else
		{
			m_pOwners = pOwner->m_pNext;
		}

		delete pOwner;
	}	
}

void CData::DumpToDisk( OUTSTREAM outf )
{
	// Version.
	short Version = max( 1, _ttoi( m_Identificator.DB->GetVersion() ) );
	outf.write( (const char *)(LPCTSTR)&Version, sizeof( Version ) );
	
	// Data object class.
	WriteString( outf, GetClassName() );
	
	// Identification.
	WriteString( outf, m_Identificator.ID );
	
	// All other object information.
	Write( outf );
}

void CData::ExtractFromDisk( INPSTREAM inpf, CDataBase *pDataBase )
{
	HYSELECT_TRY
	{
		// Version number.
		short Version = 0;
		inpf.read( (char *)&Version, sizeof( Version ) );
		
		if( 0 == Version )
		{
			HYSELECT_THROW( _T("Version number can't be 0.") );
		}
		
		// Read data object class and identification.
		static TCHAR tcClassName[_CLASS_NAME_LENGTH + 1];
		static TCHAR tcObjectID[_ID_LENGTH + 1];

		if( false == ReadString( inpf, tcClassName, sizeof( tcClassName ) ) )
		{
			HYSELECT_THROW( _T("Can't read the class name.") );
		}

		if( false == ReadString( inpf, tcObjectID, sizeof( tcObjectID ) ) )
		{
			HYSELECT_THROW( _T("Can't read the object ID.") );
		}

		TRACE2( "ClassName: %s\t ObjectID: %s\n", tcClassName, tcObjectID );

		// Into HySelect some CTable have been converted to another derived class for instance "PIPING_TAB" becomes a CTableHM.
		// Use the code below to correct old tsp files

		// 2016-10-05: Pay attention! Version written with 'CData::Write' can be either the DB version of the 'tadb.db' file or
		//             the DB version of the datastruct of a tsp file. This is why we need to check at which database belongs the
		//             object before to check the version.

#ifndef TACBX
		if( CDataBase::TADatastruct == pDataBase->GetDatabaseType() && Version <= 4007 )
		{
			// Refactoring operation
			if( 0 == _tcscmp( tcClassName, _T("CDS_SelCtrl") ) )
			{
				wcscpy_s( tcClassName, _T("CDS_SSelCtrl") );
			}
			else if( 0 == _tcscmp( tcClassName, _T("CDS_SelCv") ) )
			{
				wcscpy_s( tcClassName, _T("CDS_SSelCv") );
			}
			else if( 0 == _tcscmp( tcClassName, _T("CDS_SelPICv") ) )
			{
				wcscpy_s( tcClassName, _T("CDS_SSelPICv") );
			}
			else if( 0 == _tcscmp( tcClassName, _T("CDS_SelBCv") ) )
			{
				wcscpy_s( tcClassName, _T("CDS_SSelBCv") );
			}
		}

		if( 0 == _tcscmp( tcClassName, _T("CTable") ) )
		{
			if( 0 == _tcscmp( tcObjectID, _T("PIPING_TAB") ) )
			{
				wcscpy_s( tcClassName, _T("CTableHM") );
			}
			else if( 0 == _tcscmp( tcObjectID, _T("TMPHUB_TAB") ) )
			{
				wcscpy_s( tcClassName, _T("CTableHM") );
			}
			else if( 0 == _tcscmp( tcObjectID, _T("SELHUB_TAB") ) )
			{
				wcscpy_s( tcClassName, _T("CTableHM") );
			}
			else if( 0 == _tcscmp( tcObjectID, _T("LOGDATA_TAB") ) )
			{
				wcscpy_s( tcClassName, _T("CDS_LogTable") );
			}
			else if( 0 == _tcscmp( tcObjectID, _T("TMPLOGDATA_TAB") ) )
			{
				wcscpy_s( tcClassName, _T("CDS_LogTable") );
			}
		}

		// 2016-10-06: Pay attention: CDS_SSelParameter was changed into CDS_IndSepParameter in CTADataStruct version 4.0.1.4 and not in 4.0.1.1.
		if( CDataBase::TADatastruct == pDataBase->GetDatabaseType() && Version <= 4014 )
		{
			// Refactoring operations.
			if( 0 == _tcscmp( tcClassName, _T("CDS_SSelParameter") ) )
			{
				wcscpy_s( tcClassName, _T("CDS_IndSelParameter") );
			}
			
			if( 0 == _tcscmp( tcObjectID, _T("PARAM_SSEL") ) )
			{
				wcscpy_s( tcObjectID, _T("PARAM_INDSEL") );
			}
		}
#endif
		
		// Create the object and call virtual function Read
		IDPTR idptr;

		if( _T('\0') != *tcClassName && _T('\0') != *tcObjectID )
		{
			if( NULL == pDataBase )
			{
				HYSELECT_THROW( _T("Internal error: 'pDataBase' parameter can't br NULL.") );
			}
			
			pDataBase->CreateObject( idptr, tcClassName, tcObjectID );

			if( false == idptr.MP->Read( inpf ) )
			{
				// Returns 'true' to signal error.
				HYSELECT_THROW( _T("Can't read the object '%s'."), tcObjectID );
			}
		}
		else
		{
			if( _T('\0') == *tcClassName )
			{
				HYSELECT_THROW( _T("Class name can't be empty."), tcObjectID );
			}
			else
			{
				HYSELECT_THROW( _T("Object ID can't be empty."), tcObjectID );
			}
		}
	}
	HYSELECT_CATCH( clHySelectException, _T("Error in 'CData::ExtractFromDisk'.") )
}

#ifndef TACBX //TACBX////////////////////////////////////////////////////////
void CData::GetInheritedModifiedData( INPSTREAM  inpf, unsigned short *pusLineCount, CStringArray *pStrar, int iMaxLines )
{
	try
	{
		// Read modified lines.
		TCHAR *ptcLine = ReadTextLine( inpf, pusLineCount );

		CString str = ptcLine;

		// Opening bracket.
		if( _T('{') != *ptcLine )
		{
			HYSELECT_THROW( _T("First line must be '{' (pos: %i)."), *pusLineCount );
		}

		int iCount = 0;

		ptcLine = ReadTextLine( inpf, pusLineCount );
		str = ptcLine;

		// Stop after 20 lines and assert.
		while( str.GetAt( 0 ) != _T('}') && iCount < iMaxLines )
		{
			pStrar->Add( str );
			iCount++;
			
			ptcLine = ReadTextLine( inpf, pusLineCount );
			str = ptcLine;
		}

		if( iCount >= iMaxLines )
		{
			HYSELECT_THROW( _T("Number of line (%i) in inherited object exceeds the maximum (%i) (pos: %i)."), iCount, iMaxLines, *pusLineCount );
		}
	}
	catch( CHySelectException &clHySelectException )
	{
		clHySelectException.AddMessage( _T("Error in 'CData::GetInheritedModifiedData'."), __LINE__, __FILE__ );
		throw;
	}
}

void CData::ReadDoubleFromTextLine( INPSTREAM inpf, double &dValue, unsigned short *pusLineCount )
{
	try
	{
		TCHAR *ptcLine = ReadTextLine( inpf, pusLineCount );

		TCHAR *ptcEndPtr;
		double dVal = 0;
		dVal = _tcstod( ptcLine, &ptcEndPtr );

		if( _T('\0') != *ptcEndPtr )
		{
			HYSELECT_THROW( _T("Can't convert double value (%s) in double (pos: %i)."), ptcLine, *pusLineCount );
		}
	
		if( 0 == CString( ptcLine ).CompareNoCase( _T("DBL_MIN") ) )
		{
			dValue = DBL_MIN;
		}
		else if( 0 == CString( ptcLine ).CompareNoCase( _T("DBL_MAX") ) )
		{
			dValue = DBL_MAX;
		}
		else
		{
			dValue = dVal;
		}
	}
	catch( CHySelectException &clHySelectException )
	{
		clHySelectException.AddMessage( _T("Error in 'CData::ReadDoubleFromTextLine'."), __LINE__, __FILE__ );
		throw;
	}
}

void CData::ReadIntFromTextLine( INPSTREAM  inpf, int &iValue, unsigned short *pusLineCount )
{
	try
	{
		TCHAR *ptcLine = ReadTextLine( inpf, pusLineCount );
		TCHAR *ptcEndPtr;

		if( 0 == CString( ptcLine ).CompareNoCase( _T("INT_MIN") ) )
		{
			iValue = INT_MIN;
		}
		else if( 0 == CString( ptcLine ).CompareNoCase( _T("INT_MAX") ) )
		{
			iValue = INT_MAX;
		}
		else
		{
			int iVal = 0;
			iVal = (int)_tcstoi64( ptcLine, &ptcEndPtr, 10 );

			if( _T('\0') != *ptcEndPtr )
			{
				HYSELECT_THROW( _T("Can't convert number of function value (%s) in int (pos: %i)."), ptcLine, *pusLineCount );
			}
	
			iValue = iVal;
		}
	}
	catch( CHySelectException &clHySelectException )
	{
		clHySelectException.AddMessage( _T("Error in 'CData::ReadIntFromTextLine'."), __LINE__, __FILE__ );
		throw;
	}
}

void CData::DeleteArticle()
{
	SetDeleted( true );
}

bool CData::m_bUnicodeFile = false;

LPTSTR CData::ReadTextLine( INPSTREAM inpf, unsigned short *pusLineCount, bool bExcludeTab )
{
	try
	{
		if( NULL == pusLineCount )
		{
			HYSELECT_THROW( _T("Internal error: 'pusLineCount' can't be null .") );
		}

		static TCHAR tcBuffer[1024];
		TCHAR *ptcBuffer = tcBuffer;
		int iCount = 0;
		bool bEnded = false;

		memset( tcBuffer, 0, sizeof( tcBuffer ) );

		if( _T('\0') == *pusLineCount )
		{
			// Skip two first characters in case of unicode text file normally (0xFF 0xFE).
			m_bUnicodeFile = false;
			inpf.read( (char *)ptcBuffer, sizeof( TCHAR ) );

			if( 0xFEFF == tcBuffer[0] )
			{
				m_bUnicodeFile = true;
			}
			else
			{
				// Reset file pointer to the file beginning.
				inpf.seekg( ios_base::beg );
			}
		}

		uint8_t ChrSize = sizeof( char );

		if( true == m_bUnicodeFile )
		{
			ChrSize = sizeof( TCHAR );
		}
		
		*ptcBuffer = _T('\0');
		bool bBetweenQuotes = false;

		while( inpf.read( (char *)ptcBuffer, ChrSize ) )
		{
			if( true == bEnded && _T('\n') != *ptcBuffer )
			{
				continue;
			}

			// End of line reached.
			if( _T('\n') == *ptcBuffer )
			{
				(*pusLineCount)++;
				*ptcBuffer = _T('\0');

				if( 0 != iCount && _T('\r') == *(ptcBuffer - 1) )
				{
					*(ptcBuffer - 1) = 0;
					iCount--;
				}

				if( _T('\0') != tcBuffer[0] )
				{
					break;
				}
				else					// Empty Line, next Line
				{
					bEnded = false;
					ptcBuffer = tcBuffer;
					iCount = 0;
					bBetweenQuotes = false;
					continue;
				}
			}

			// Stop string when comment begin.
			if( 0 != iCount )
			{
				if( _T('/') == *ptcBuffer && _T('/') == *(ptcBuffer - 1) )
				{
					*(ptcBuffer - 1) = _T('\0');
					bEnded = true;
					continue;
				}
			}

			// Skip tabulation.
			if ( true == bExcludeTab && '\t' == *ptcBuffer)
			{
				continue;
			}

			// Check quotes.
			if( _T('"') == *ptcBuffer )
			{
				if( false == bBetweenQuotes )
				{
					bBetweenQuotes = true;
				}
				else
				{
					bBetweenQuotes = false;
				}
			}

			// Remove space outside quotes.
			if( false == bBetweenQuotes && _T(' ') == *ptcBuffer )
			{
				continue;
			}

			ptcBuffer++;

			if( ptcBuffer > &tcBuffer[1023] )
			{
				HYSELECT_THROW( _T("The line %i is too long (%i > 1024)."), *pusLineCount, ( ptcBuffer - tcBuffer ) );
			}

			iCount++;
		}

		if( _T('\0') == tcBuffer[0] )
		{
			HYSELECT_THROW( _T("Unexpected empty line (pos: %i)."), *pusLineCount );
		}

		CString str = tcBuffer;
		str.Trim();

		if( ACTIVATE_TRACE_DB_READING )
		{
			TRACE( _T("Ln %u:%s\n"), *pusLineCount, str );
		}

#ifdef TACBX
#pragma warning( disable : 4018)
#endif
		// Check if buffer contains a reference to a string located into the StringTab
		// CString str = Buffer;
		//int iIDSPos = str.Find(IDS_Id);
		//if (iIDSPos>-1 && Buffer[0] != '#')
		//{
		//	int iPos = iIDSPos+1;
		//	while (iPos<=str.GetLength()&&str.GetAt(iPos) != '"')
		//		++iPos;
		//	CString IDSstr=str.Mid(iIDSPos,iPos-iIDSPos);
		//	_string IDS = (LPCTSTR)IDSstr;
		//	XmlStrTab.FindAndReplace(IDS);
		//	IDSstr = IDS.c_str();
		//	str.Delete(iIDSPos,iPos-iIDSPos);
		//	str.Insert(iIDSPos,IDSstr);
		//	_tcscpy_s(Buffer,SIZEOFINTCHAR(Buffer),(LPCTSTR) str);
		//}
#ifdef TACBX
#pragma warning( default : 4018)
#endif

		_tcscpy( tcBuffer, (LPCTSTR)str );
		return tcBuffer;
	}
	catch( CHySelectException &clHySelectException )
	{
		clHySelectException.AddMessage( _T("Error in 'CData::ReadTextLine'."), __LINE__, __FILE__ );
		throw;
	}
}

void CData::ParseIDList( CArray<IDPTR> *pAr, CString list )
{
	try
	{
		int nField = 0;
		TCHAR **pptcField = ParseTextLine( (LPTSTR)(LPCTSTR)list, &nField );

		pAr->RemoveAll();
		
		for( int i = 0; i < nField; i++ )
		{
			IDPTR idptr = _NULL_IDPTR;

			if( _tcslen( pptcField[i] ) > _ID_LENGTH )
			{
				HYSELECT_THROW( _T("ID (%s) at position %i is too long (%i > %i)."), pptcField[i], i, _tcslen( pptcField[i] ), _ID_LENGTH );
			}

			_tcsncpy_s( idptr.ID, _ID_LENGTH, pptcField[i], _ID_LENGTH );

			if( 0 == _tcsicmp( idptr.ID, _T("NO_ID") ) )
			{
				continue;
			}

			idptr.DB = GetDB();
			Extend( &idptr );
			pAr->Add( idptr );
		}
	}
	catch( CHySelectException &clHySelectException )
	{
		clHySelectException.AddMessage( _T("Error in 'CData::ParseIDList'."), __LINE__, __FILE__ );
		throw;
	}
}

void CData::ParseIDList( std::vector<IDPTR> &vecIDList, CString list, TCHAR tcSeparator )
{
	try
	{
		int nField = 0;
		TCHAR **pptcField = ParseTextLine( (LPTSTR)(LPCTSTR)list, &nField, tcSeparator );

		vecIDList.clear();

		for( int i = 0; i < nField; i++ )
		{
			IDPTR idptr = _NULL_IDPTR;

			if( _tcslen( pptcField[i] ) > _ID_LENGTH )
			{
				HYSELECT_THROW( _T("ID (%s) at position %i is too long (%i > %i)."), pptcField[i], i, _tcslen( pptcField[i] ), _ID_LENGTH );
			}
			
			_tcsncpy_s( idptr.ID, _ID_LENGTH, pptcField[i], _ID_LENGTH );

			if( 0 == _tcsicmp( idptr.ID, _T("NO_ID") ) )
			{
				continue;
			}

			idptr.DB = GetDB();
			Extend( &idptr );
			vecIDList.push_back( idptr );
		}
	}
	catch( CHySelectException &clHySelectException )
	{
		clHySelectException.AddMessage( _T("Error in 'CData::ParseIDList'."), __LINE__, __FILE__ );
		throw;
	}
}

void CData::ParseDoubleList( CArray <double> *pAr, CString list )
{
	try
	{
		int nField = 0;
		TCHAR *ptcEndPtr;
		TCHAR **pptcField = ParseTextLine( (LPTSTR)(LPCTSTR)list, &nField );

		pAr->RemoveAll();
		pAr->SetSize( nField );
		
		for( int i = 0; i < nField; i++ )
		{
			double d = _tcstod( pptcField[i], &ptcEndPtr );
			
			if( _T('\0') != *ptcEndPtr )
			{
				HYSELECT_THROW( _T("Can't convert field '%s' at position %i in double."), pptcField[i], i );
			}

			pAr->SetAt( i, d );
		}
	}
	catch( CHySelectException &clHySelectException )
	{
		clHySelectException.AddMessage( _T("Error in 'CData::ParseDoubleList'."), __LINE__, __FILE__ );
		throw;
	}
}

CString CData::DeleteArticleNumber( CString strArticleNumber )
{
	if( true == strArticleNumber.IsEmpty() )
	{
		return _T("");
	}

	// From: https://www.regular-expressions.info/charclass.html
	// In most regex flavors, the only special characters or metacharacters inside a character class are the closing bracket ], 
	// the backslash \, the caret ^, and the hyphen -.
	// The closing bracket ], the caret ^ and the hyphen - can be included by escaping them with a backslash, or by placing them 
	// in a position where they do not take on their special meaning.

	// Here we search all characters excluded the '.', ' ', '-', '_' and '&' ones.
	std::wregex exp( _T(R"([^. \-_&])") );

	// And we replace each found characters by '*'.
	std::wstring wstrSource = (LPCTSTR)strArticleNumber;
	CString strResult = std::regex_replace( wstrSource, exp, _T("*") ).c_str();

	return strResult;
}

void CData::ExtractFromTextFile( INPSTREAM  inpf, CDataBase *pDataBase, bool bReadingObjectFromDeletedFile, unsigned short *pusLineCount, 
		LPCTSTR pctstrFirstLine, bool bIgnore )
{
	try
	{
		static TCHAR tcClassName[_CLASS_NAME_LENGTH + 1];
		static TCHAR ObjectID[_ID_LENGTH + 3];			// +3 -> one for \0 and two for ExistInCBX flag
		static bool bExistInTACBX = false;
		LPCTSTR ptcStrLine = pctstrFirstLine;
	
		if( NULL == pctstrFirstLine )
		{
			ptcStrLine = ReadTextLine( inpf, pusLineCount );
		}

		if( true == bIgnore )	// Ignore the object. One counts the number of opening and closing curly braces.
		{
			ptcStrLine = ReadTextLine( inpf, pusLineCount );

			if( _T('{') != *ptcStrLine )
			{
				HYSELECT_THROW( _T("First line must be '{' (pos: %i)."), *pusLineCount );
			}

			int np = 1;
		
			do
			{
				ptcStrLine = ReadTextLine( inpf, pusLineCount );

				np += ( _T('{') == *ptcStrLine ) ? 1 : 0;
				np -= ( _T('}') == *ptcStrLine ) ? 1 : 0;
			}
			while( np > 0 );
		}
		else
		{
			// The line should be "ClassName(ID)->Table_ID" where class name is the class to be instanciated,
			// ID is the object ID and TableID is the table where the object should be inserted.
			// (The table should have been already declared!). If ->TableID is missing, the object is not inserted in any table
			// and the user should take care of doing it.
	
			// 1 - read the class name.
			TCHAR *ptcCopy = tcClassName;
			LPCTSTR pctstrCursor;

			for( pctstrCursor = ptcStrLine; _T('\0') != *pctstrCursor && _T('(') != *pctstrCursor; pctstrCursor++ )
			{
				*(ptcCopy++) = *pctstrCursor;
			}

			*ptcCopy = _T('\0');

			if( _T('\0') == *pctstrCursor )
			{
				HYSELECT_THROW( _T("Unexpected end of line after reading the class name (pos: %i)."), *pusLineCount );
			}
			else if( _T('\0') == *tcClassName )
			{
				HYSELECT_THROW( _T("Class name can't be empty (pos: %i)."), *pusLineCount );
			}
	
			// 2 - read the ID.
			bExistInTACBX = false;
			ptcCopy = ObjectID;

			for( pctstrCursor++; _T('\0') != *pctstrCursor && _T(')') != *pctstrCursor; pctstrCursor++ )
			{
				*(ptcCopy++) = *pctstrCursor;
			}

			*ptcCopy = _T('\0');
	
			if( _T('\\') == *( ptcCopy - 2 ) )
			{
				bExistInTACBX = ( _T('1') == *(ptcCopy - 1) ) ? true : false;
				*(ptcCopy - 2) = _T('\0');
			}

			if( _tcslen( ObjectID ) > _ID_LENGTH )
			{
				HYSELECT_THROW( _T("Object ID (%s) is too long (%i > %i) (pos: %i)."), ObjectID, _tcslen( ObjectID ), _ID_LENGTH, *pusLineCount );
			}

			if( _T('\0') == *pctstrCursor )
			{
				HYSELECT_THROW( _T("Unexpected end of line after reading the class ID (pos: %i)."), *pusLineCount );
			}
	
			// 3 - Read the table ID where to insert this CData derived object.
			CTable *pclHostTable = NULL;
			TCHAR tcTableID[_ID_LENGTH + 1];
			tcTableID[0] = 0;
	
			if( ( _tcslen( ++pctstrCursor ) > 2 ) && ( _T('-') == *(pctstrCursor++) ) && ( _T('>') == *(pctstrCursor++) ) )
			{
				// The table ID should be from the correct length.
				if( _tcslen( pctstrCursor ) > _ID_LENGTH )
				{
					HYSELECT_THROW( _T("Table ID (%s) is too long (%i > %i) (pos: %i)."), pctstrCursor, _tcslen( pctstrCursor ), _ID_LENGTH, *pusLineCount );
				}
		
				// Keep a copy of Table ID.
				_tcsncpy_s( tcTableID, SIZEOFINTCHAR( tcTableID ), pctstrCursor, SIZEOFINTCHAR( tcTableID ) - 1 );
			}

			// 4 - Create the object (floating in DB) and call virtual function 'ReadText'.
			IDPTR NewObject;
			pDataBase->CreateObject( NewObject, tcClassName, ObjectID );

			// 5 - Check if Table exists in DB (previously made in step 3).
			if( _T('\0') != *tcTableID )
			{
				IDPTR Table = pDataBase->Get( tcTableID );

				// The table has not been declared yet.
				if( _T('\0') == *Table.ID )
				{
					HYSELECT_THROW( _T("Table ID (%s) where to insert the object is defined but not found in the database (pos: %i)."), tcTableID, *pusLineCount );
				}
		
				// The ID is not referencing to a table !!
				if( NULL == dynamic_cast<CTable*>( Table.MP ) )
				{
					HYSELECT_THROW( _T("Table ID (%s) where to insert the object is defined but it's not a 'CTable' object (pos: %i)."), tcTableID, *pusLineCount );
				}

				pclHostTable = (CTable*)( Table.MP );
				NewObject.PP = pclHostTable;
			}

			// 6 - Insert object in table.
			if( NULL != pclHostTable )
			{
				pclHostTable->Insert( NewObject );
			}

			NewObject.MP->SetExistInTASCOPE( bExistInTACBX );
			NewObject.MP->ReadText( inpf, pusLineCount );
	
			// 7 - find and apply a possible "redefinition".
			POSITION Pos = NULL;

			if( false == RedefineTab.IsEmpty() )
			{
				int iRedefined = 0;		// by default do nothing

				iRedefined = RedefineTab.IsRedefined( Pos, ObjectID );

				if( 1 == iRedefined )		// Call object's redefinition function with current redefinition string
				{
					NewObject.MP->CheckRedefinition( &RedefineTab, RedefineTab.GetFieldLineString( Pos ) );
				}
			}

			// 8 - If we are reading the 'tadb_deleted.txt' file, we automatically flag the object as deleted and we clean
			// the article number.
			if( true == bReadingObjectFromDeletedFile )
			{
				NewObject.MP->DeleteArticle();
			}
		}
	}
	HYSELECT_CATCH( clHySelectException, _T("Error in 'CData::ExtractFromTextFile'.") )
}

CData::CheckFilterReturnValue CData::CheckFilter( CFilterTab *pFilter, CString strFilter )
{
	if( true == strFilter.IsEmpty() )
	{
		ASSERTA_RETURN( CData::CFRV_Error );
	}

	// Retrieve the command.
	CString str = pFilter->GetField( 3, strFilter );
	
	if( 0 != str.CompareNoCase( _T("EDIT") ) )
	{
		return CData::CFRV_FilterNotApplied;
	}
	
	// Availability.
	str = pFilter->GetNextField( strFilter );

	if( false == str.IsEmpty() )
	{
		if( _T('0') == str[0] )
		{
			SetAvailable( false );
		}

		if( _T('1') == str[0] )
		{
			SetAvailable( true );
		}

		if( _T('0') == str[2] )
		{
			SetHidden( false );
		}

		if( _T('1') == str[2] )
		{
			SetHidden( true );
		}
	}

	return CData::CFRV_FilterApplied;
}

bool CData::BreakOnID( CString strID, bool bBreakOnFailure )
{

#ifdef DEBUG
	if ( true == WildcardMatch( (LPCTSTR)GetIDPtr().ID, (LPCTSTR) strID ) )
	{
		if ( true == bBreakOnFailure )
		{
			DebugBreak();
		}

		return true;
	}
#endif
	
	return false;
}

#endif //TACBX////////////////////////////////////////////////////////

void CData::OnDatabaseDestroy()
{
#if defined(_DEBUG)
	//if(IsClass(CLASS(CData)))
	//	TRACE(_T("Non Overriden OnDatabaseDestroy() is called\n"));
#endif 
}

void CData::Write(OUTSTREAM outf)
{
	// version n°
	short Version = 4;
	outf.write((const char *)(LPCTSTR)&Version, sizeof(Version));
	// Write flags
	outf.write((char *)&m_bDeleted, sizeof(m_bDeleted));
	outf.write((char *)&m_bAvailable, sizeof(m_bAvailable));
	outf.write((char *)&m_bHidden, sizeof(m_bHidden));
	outf.write((char *)&m_bExistInTASCOPE, sizeof(m_bExistInTASCOPE));
	// m_Owner is used as a flag to tell Read function where there is at least one item in the owner list or not
	// write just -1 or 0, keep binary identical TADB.DB created with same *.txt at a different time or on different machine
	void *flag;
	(m_pOwners)?flag=(void*)-1:flag=NULL;
	outf.write((char *)&flag, sizeof(flag));
	// Owner list
	for(OWNERS_ID* pOwner = m_pOwners; pOwner; pOwner = pOwner->m_pNext)
	{
		flag = (void *)pOwner->m_pNext;
		(pOwner->m_pNext)?pOwner->m_pNext=(OWNERS_ID*)-1:pOwner->m_pNext=NULL;
		WriteString(outf,pOwner->ID);
		outf.write((char *)&pOwner->m_pNext, sizeof(pOwner->m_pNext));
		pOwner->m_pNext=(OWNERS_ID*)flag;
	}
}			

bool CData::Read(INPSTREAM  inpf)
{
	// version n°
	short Version ;
	inpf.read((char *)&Version, sizeof(Version));
	if (Version<1 || Version>4)
		return false;
	// Read flags
	if (Version > 1)
	{
		inpf.read((char *)&m_bDeleted, sizeof(m_bDeleted));
		inpf.read((char *)&m_bAvailable, sizeof(m_bAvailable));
		inpf.read((char *)&m_bHidden, sizeof(m_bHidden));
	}
	if (Version > 3)
		inpf.read((char *)&m_bExistInTASCOPE, sizeof(m_bExistInTASCOPE));

	// Read Owner list
	OWNERS_ID* pItem = 0;
	inpf.read((char *)&pItem, sizeof(pItem));
	int next = pItem!=0;
	int iIDSize = _ID_LENGTH+1;
	while(next)
	{
		pItem = new OWNERS_ID;
		if (Version < 3)
		{
			iIDSize = _OLD_ID_LENGTH+1;
			if (g_bSBCSFile)
			{
				memset (pItem->ID,0,sizeof(pItem->ID));
				// size stored in file in SBCS is SIZEOFINTCHAR(pItem->ID) + number of
				// bytes needed for structure member alignement; OWNER_ID struct is stored in one blok ...
				// ULONG size = (ULONG)&pItem->pNext - (ULONG)pItem->ID;
				for (int i=0; i<iIDSize; i++)
					inpf.read((char *)&pItem->ID[i],sizeof(BYTE));
				inpf.read((char *)&pItem->m_pNext, sizeof (pItem->m_pNext));
			}
			else
			{
				memset (pItem->ID,0,sizeof(pItem->ID));
				char *pBuf = (char *)&pItem->ID[0];
				for (int i=0; i<(int)(iIDSize*sizeof(TCHAR)); i++)
				{
					inpf.read((char *)pBuf++,sizeof(char));
//					ASSERT(!(inpf.rdstate() & std::ifstream::failbit));
				}
				inpf.read((char *)&pItem->m_pNext, sizeof (pItem->m_pNext));
//				ASSERT(!(inpf.rdstate() & std::ifstream::failbit));
			}
		}
		else
		{
			memset (pItem->ID,0,sizeof(pItem->ID));
			if (!ReadString(inpf,pItem->ID, sizeof(pItem->ID)))
				return false;
			inpf.read((char *)&pItem->m_pNext, sizeof (pItem->m_pNext));
		}
		next = pItem->m_pNext!=0;
		pItem->m_pNext = m_pOwners;
		m_pOwners = pItem;
	}
	return true;
}

#ifndef TACBX //TACBX////////////////////////////////////////////////////////
void CData::WriteText(OUTSTREAM outf, CString &strTab)
{
	// Write Object Header
	CString str = _T("");

	if( true == m_bExistInTASCOPE )
	{
		str.Format( _T("\r\n\r\nnew:%s(%s\\1)->%s"), GetClassName(), GetIDPtr().ID, GetIDPtr().PP->GetIDPtr().ID );
	}
	else
	{
		str.Format( _T("\r\n\r\nnew:%s(%s)->%s"), GetClassName(), GetIDPtr().ID, GetIDPtr().PP->GetIDPtr().ID );
	}

	outf.write( (const char *)(LPCTSTR)str, str.GetLength() * sizeof( TCHAR ) );
}

void CData::DropData( OUTSTREAM outf, int* piLevel )
{
	CString strTab = _T("");

	for(int i = *piLevel; i; i-- )
	{
		strTab += _T("\t");
	}

	CString str;
	str.Format( _T("m_Identificator.ID = %s\n"), CString( m_Identificator.ID ) );
	WriteFormatedStringA2( outf, str, strTab );
	str.Format( _T("m_bTable = %s\n"), ( true == m_bTable ) ? _T("true") : _T("false") );
	WriteFormatedStringA2( outf, str, strTab );
	str.Format( _T("m_Deleted = %s\n"), ( true == m_bDeleted ) ? _T("true") : _T("false") );
	WriteFormatedStringA2( outf, str, strTab );
	str.Format( _T("m_Available = %s\n"), ( true == m_bAvailable ) ? _T("true") : _T("false") );
	WriteFormatedStringA2( outf, str, strTab );
	str.Format( _T("m_Hidden = %s\n"), ( true == m_bHidden ) ? _T("true") : _T("false") );
	WriteFormatedStringA2( outf, str, strTab );
	str.Format( _T("m_usTACBXVer = %u\n"), m_usTACBXVer );
	WriteFormatedStringA2( outf, str, strTab );
	str.Format( _T("m_bExistInTASCOPE = %s\n\n"), ( true == m_bExistInTASCOPE ) ? _T("true") : _T("false") );
	WriteFormatedStringA2( outf, str, strTab );
}
#endif //TACBX////////////////////////////////////////////////////////

void CData::Copy( CData *pclDestination )
{
	pclDestination->SetHidden( IsHidden() );
	pclDestination->SetFlagTable( IsaTable() );
	pclDestination->SetDeleted( IsDeleted() );
	pclDestination->SetAvailable( IsAvailable() );
	pclDestination->SetExistInTASCOPE( IsExistInTASCOPE() );
}

#ifndef TACBX
// PAY ATTENTION!! Must not be used without knowing what we are doing! Changing the ID of an object is
// completely forbidden. The only use of this method is for renaming old pipe ID (See Jira card HYS-1590).
void CData::ChangeID( LPTSTR lpNewID )
{
	HYSELECT_TRY
	{
		if( _tcslen( lpNewID ) > _ID_LENGTH )
		{
			HYSELECT_THROW( _T("Object ID (%s) is too long (%i > %i)."), lpNewID, _tcslen( lpNewID ), _ID_LENGTH );
		}

		_tcscpy( m_Identificator.ID, lpNewID );
	}
	HYSELECT_CATCH( clHySelectException, _T("Error in 'CData::ChangeID'.") )
}
#endif

#ifndef TACBX
int CData::CompareSelectionTo( CData* pclSelectionCompareWith, int iKey )
{
	if( NULL == pclSelectionCompareWith )
	{
		return 0;
	}

	CSelectedInfos *pclSelectedInfoToCompare = GetpSelectedInfos();
	CSelectedInfos *pclSelectedInfoCompareWith = pclSelectionCompareWith->GetpSelectedInfos();

	if( NULL == pclSelectedInfoToCompare || NULL == pclSelectedInfoCompareWith )
	{
		return 0;
	}

	int iReturn = 0;
	switch( iKey )
	{
		case PageField_enum::epfFIRSTREF:
			iReturn = StringCompare(	pclSelectedInfoToCompare->GetReference( CSelectedInfos::eReferences::eRef1 ),
										pclSelectedInfoCompareWith->GetReference( CSelectedInfos::eReferences::eRef1 ) );
			break;

		case PageField_enum::epfSECONDREF:
			iReturn = StringCompare(	pclSelectedInfoToCompare->GetReference( CSelectedInfos::eReferences::eRef2 ),
										pclSelectedInfoCompareWith->GetReference( CSelectedInfos::eReferences::eRef2 ) );
			break;

		case PageField_enum::epfQUANTITY:
			if( pclSelectedInfoToCompare->GetQuantity() < pclSelectedInfoCompareWith->GetQuantity() )
			{
				iReturn = -1;
			}
			else if( pclSelectedInfoToCompare->GetQuantity() > pclSelectedInfoCompareWith->GetQuantity() )
			{
				iReturn = 1;
			}
			else
			{
				iReturn = 0;
			}
			break;

		case PageField_enum::epfSALEUNITPRICE:
			if( pclSelectedInfoToCompare->GetPrice() < pclSelectedInfoCompareWith->GetPrice() )
			{
				iReturn = -1;
			}
			else if( pclSelectedInfoToCompare->GetPrice() > pclSelectedInfoCompareWith->GetPrice() )
			{
				iReturn = 1;
			}
			else
			{
				iReturn = 0;
			}
			break;
	}

	return iReturn;
}
#endif

//////////////////////////////////////////////////////////////////////////////////////////////
// Member class CDataClassRegistry

CData::CDataClassRegistry::CDataClassRegistry(LPCTSTR file)
{
	TRACE1("The class register is created in file %s\n", file);
	m_prClassTable = 0;
	m_usClassAmount = 0;
}	

CData::CDataClassRegistry::~CDataClassRegistry()
{
	delete [] m_prClassTable;
}

void CData::CDataClassRegistry::Add(LPCTSTR ClassName, CData* (*NewFunc)(CDataBase*, LPCTSTR))
{
	ASSERT(ClassName&&*ClassName);
	ASSERT(_tcslen(ClassName)<=_CLASS_NAME_LENGTH);
	ASSERT(NewFunc);

	// enlarge the table and insert the new class name in alphabetical order
	Assoc_struct* OldTable = m_prClassTable;
	m_prClassTable = new Assoc_struct[++m_usClassAmount];
	unsigned short i;
	for (i=0; (i<(m_usClassAmount-1))&&(_tcscmp(ClassName, OldTable[i].ClassName)>0); i++)
		m_prClassTable[i] = OldTable[i];
	_tcsncpy_s(m_prClassTable[i].ClassName,SIZEOFINTCHAR(m_prClassTable[i].ClassName), ClassName,SIZEOFINTCHAR(m_prClassTable[i].ClassName)-1);
	m_prClassTable[i].NewFunc = NewFunc;
	for(i++; i<m_usClassAmount; i++)
		m_prClassTable[i] = OldTable[i-1];
	delete [] OldTable;
	TRACE2("%u - %s class is registered.\n", m_usClassAmount, ClassName);
}
		
CData* CData::CDataClassRegistry::New(LPCTSTR ClassName, CDataBase* pDataBase, LPCTSTR ID)
{
	ASSERT(m_usClassAmount);
	ASSERT(ClassName&&*ClassName);
	ASSERT(pDataBase);
	ASSERT(ID&&*ID);
	
	// First look for the ClassName association
	// Dichotomic algorithm is used.
	unsigned short MinPos = 0;
	unsigned short MaxPos = m_usClassAmount-1;

	while((MaxPos-MinPos)>1)
	{
		unsigned short MidPos = (MaxPos + MinPos) / 2;
		if(_tcscmp(m_prClassTable[MidPos].ClassName, ClassName)<0)
			MinPos = MidPos;
		else
			MaxPos = MidPos;
	}
	
	// Create the object
	if(!_tcscmp(m_prClassTable[MinPos].ClassName, ClassName))
		return (*m_prClassTable[MinPos].NewFunc)(pDataBase, ID);
#if defined(_DEBUG)
	if(_tcscmp(m_prClassTable[MaxPos].ClassName, ClassName))
	{
		// 2016-10-20: See comments before definition of the 'IMPLEMENT_CLASS_REGISTER' in the 'DataBase.h' file.
		TRACE(_T("Try to create an object from non registered class %s\n"),ClassName);
		ASSERT(false);
	} 
#endif
	return (*m_prClassTable[MaxPos].NewFunc)(pDataBase, ID);
}

//////////////////////////////////////////////////////////////////////////////////////////////
//
//	CTable class.
//			
//////////////////////////////////////////////////////////////////////////////////////////////

IMPLEMENT_DATA( CTable )
	
CTable::CTable( CDataBase *pDataBase, LPCTSTR ID )
: CData( pDataBase, ID ),
	m_Name(ID)
{
	*m_CNAccess = _T('\0');

	// We set 'true' because when reading old version of "user.db" or "pipes.db" we have not this information
	// but we are sure that all tables are parent by default.
	m_bTrueParent = true;

	SetFlagTable( true );
	m_pStr = NULL;
	MakeEmpty();
}

CTable::~CTable()
{
	MakeEmpty();
}

void CTable::MakeEmpty()
{
	// Unlock all objects.
	for( IDPTR IDPtr = m_IDPtrList.GetFirst(); _T('\0') != *IDPtr.ID; IDPtr = m_IDPtrList.GetNext() )
	{
		IDPtr.MP->Unlock( GetIDPtr() );
	}
	
	// Delete list.
	m_IDPtrList.MakeEmpty();
	Modified();		
}

void CTable::CheckAndTransferTmpObj()
{
	if( true == m_arIDptr.empty() )
	{
		return;
	}
	
	HYSELECT_TRY
	{
		for( std::vector <IDPTR>::iterator iter = m_arIDptr.begin(); iter != m_arIDptr.end(); iter++ )
		{
			IDPTR RealObjectIDPtr = (*iter).DB->Get( (*iter).ID );
			bool bCanInsert = false;

			// Verify first local IDPTR.
			// Remark: Normally all objects in 'm_arIDptr' have MP NULL.
			if( NULL == (*iter).MP )
			{
				// If we have not yet the object pointed by this local IDPTR, and the real object exist...
				if( NULL != RealObjectIDPtr.MP )
				{
					// We can update our local IDPTR.
					(*iter).MP = RealObjectIDPtr.MP;
					bCanInsert = true;
				}
			}

			// Now we can verify if the parent of the real object is already set.
			if( true == m_bTrueParent && NULL == RealObjectIDPtr.PP )
			{
				// Remark: We do it only if this table is the true parent.
				// Remark: Keep in mind that 'RealObjectIDPtr' is the real object (The 'Get' method returns a reference).
				RealObjectIDPtr.PP = this;
			}

			if( true == bCanInsert )
			{
				Insert( *iter );
			}
		}

		m_arIDptr.clear();
	}
	HYSELECT_CATCH( clHySelectException, _T("Error in 'CTable::CheckAndTransferTmpObj'.") )
}

LPCTSTR CTable::SetName( LPCTSTR ptstrName )
{
	ASSERT( NULL != ptstrName );
	
	m_Name = ptstrName;
	Modified();

	return m_Name.c_str();
}

LPCTSTR CTable::GetName()
{
	m_pStr = GetpXmlStrTab()->GetIDSStr( m_Name );

	if( NULL != m_pStr )
	{
		return m_pStr->c_str();
	}
	else
	{
		return m_Name.c_str();
	}
}

void CTable::Insert( IDPTR &LocalIDPtr, bool bSetParentTable )
{
	HYSELECT_TRY
	{
		Extend( &LocalIDPtr );

		// If IDPtr point to a Table, check if the name does not already exist for an other child table.
		if( true == LocalIDPtr.MP->IsaTable() )
		{
			CTable *pTab = dynamic_cast<CTable *>( LocalIDPtr.MP );

			if( NULL != pTab )
			{
				// HYS-1498: when reading binary database, we can have a table with ID for which their PP pointers are not yet defined. 
				// For example 'ADAP_FUSTION_150_01' (CDB_Product) is read first, inserted in the database but not have its parent defined yet.
				// After that 'ADPGR_FUSION_150' (CDB_RuledTable) is read and insert the 'ADAP_FUSION_150_01' object for which PP pointer is always not defined.
				// Then follow the 'CVADAPTERGROUP_TAB' (CTable) that will insert the 'ADPGR_FUSION_150' table. So we arrive in this method and we want to run all
				// items in the 'ADPGR_FUSION_150'. But it will crash because 'GetFirst' and 'GetNext' method send back a extended IDPTR. But here, PP is not defined
				// and it doesn't work. So we change with 'GetFirstID' that only sends back ID without to try to extend the IDPTR.

				for( _string strID = pTab->GetFirstID(); 0 != _tcscmp( _T(""), strID.c_str() ); strID = pTab->GetNextID( strID ) )
				{
					IDPTR idptr = GetDB()->Get( strID.c_str() );

					if( NULL != idptr.MP && true == idptr.MP->IsaTable() )
					{
						if( 0 == _tcscmp( ( (CTable *)idptr.MP )->GetName(), pTab->GetName() ) )
						{
							HYSELECT_THROW( _T("Name '%s' is already existing in the database."), pTab->GetName() );
						}
					}
				}
			}
		}

		if( true == bSetParentTable )
		{
			// Here we change the parent of the local IDPTR not the real object pointer by 'MP'.
			LocalIDPtr.PP = (CTable *)( GetIDPtr().MP );
		}

		// Here we modify the parent of the real object !!
		if( true == m_bTrueParent )
		{
			LocalIDPtr.MP->SetParentTable( LocalIDPtr.PP );
		}
	
		m_IDPtrList.Insert( LocalIDPtr );
	
		// Lock the CData derived object which is inserted in the table.
		if( true == bSetParentTable )
		{
			LocalIDPtr.MP->Lock( GetIDPtr() );
		}
	
		Modified();		
	}
	HYSELECT_CATCH_ARG( clHySelectException, _T("Error in 'CTable::Insert' with the object '%s'."), LocalIDPtr.ID )
}

int CTable::Remove( IDPTR &IDPtr )
{
	HYSELECT_TRY
	{
		Extend( &IDPtr );
	
		if( _T('{') == *IDPtr.ID )
		{
			TRACE2( "Remove %s from %s\n", IDPtr.ID, this->GetIDPtr().ID );
		}
	
		// Verify the parent table.
		if( this != IDPtr.PP )
		{
			HYSELECT_THROW_RETURNARG( _T("The '%s' object can't be removed because the current table ('%s') is not its parent."), 0, IDPtr.ID, GetIDPtr().ID );
		}
	
		IDPtr.MP->Unlock( GetIDPtr() );
		m_IDPtrList.Remove( IDPtr.ID );
	
		// In case of GetFirst() GetNext() loop decrease position index.
		m_IDPtrList.SetPos( m_IDPtrList.GetPos() - 1 );

		return 0;
	}
	HYSELECT_CATCH_ARG( clHySelectException, _T("Error in 'CTable::Remove' method with the object '%s'."), IDPtr.ID )
}

const IDPTR &CTable::Get( LPCTSTR ID )
{
	IDPTR IDPtr = m_IDPtrList.GetIDPtrFromID( ID );
	
	// After a reading from the disk, the memory pointer is not initialised; Take care.
	if( _T('\0') == *IDPtr.ID )
	{
		return _NULL_IDPTR;
	}
	
	Extend( &IDPtr );
	return IDPtr.MP->GetIDPtr();
}

const IDPTR &CTable::GetFirst( LPCTSTR ptstrClassName )
{
	IDPTR idptr = m_IDPtrList.GetFirst(); 
	*m_CNAccess = _T('\0'); 

	if( _T('\0') == *idptr.ID )
	{
		return _NULL_IDPTR;
	}
	
	if( NULL != ptstrClassName )
	{
		_tcsncpy_s( m_CNAccess, SIZEOFINTCHAR( m_CNAccess ), ptstrClassName, SIZEOFINTCHAR( m_CNAccess ) - 1 );
	
		while( NULL != idptr.MP && false == idptr.MP->IsClass( ptstrClassName ) )
		{
			idptr = m_IDPtrList.GetNext();
		}
	}	
	
	if( _T('\0') == *idptr.ID )
	{
		return _NULL_IDPTR;
	}
	
	Extend( &idptr );
	return idptr.MP->GetIDPtr();
}

const IDPTR &CTable::GetNext()
{
	IDPTR idptr = m_IDPtrList.GetNext(); 

	if( _T('\0') != *m_CNAccess )
	{
		while( NULL != idptr.MP && false == idptr.MP->IsClass( m_CNAccess ) )
		{
			idptr = m_IDPtrList.GetNext();
		}
	}	
	
	if( _T('\0') == *idptr.ID )
	{
		return _NULL_IDPTR;
	}
	
	Extend( &idptr );
	return idptr.MP->GetIDPtr();
}

const IDPTR &CTable::GetNext( CData *pDataObj )
{
	if( NULL == pDataObj )
	{
		return _NULL_IDPTR;
	}
	
	m_IDPtrList.SetPos( pDataObj->GetIDPtr().ID );
	IDPTR idptr = m_IDPtrList.GetNext();
	
	if( _T('\0') != *m_CNAccess && NULL != idptr.MP )
	{
		while( false == idptr.MP->IsClass( m_CNAccess ) )
		{
			idptr = m_IDPtrList.GetNext();
		}
	}	
	
	if( _T('\0') == *idptr.ID )
	{
		return _NULL_IDPTR;
	}
	
	Extend( &idptr );
	return idptr.MP->GetIDPtr();
}

const _string CTable::GetFirstID( LPCTSTR ptstrClassName )
{
	IDPTR idptr = m_IDPtrList.GetFirst(); 
	*m_CNAccess = _T('\0'); 

	if( _T('\0') == *idptr.ID )
	{
		return idptr.ID;
	}
	
	if( NULL != ptstrClassName )
	{
		_tcsncpy_s( m_CNAccess, SIZEOFINTCHAR( m_CNAccess ), ptstrClassName, SIZEOFINTCHAR( m_CNAccess ) - 1 );
	
		while( NULL != idptr.MP && false == idptr.MP->IsClass( ptstrClassName ) )
		{
			idptr = m_IDPtrList.GetNext();
		}
	}	
	
	return idptr.ID;
}

const _string CTable::GetNextID()
{
	IDPTR idptr = m_IDPtrList.GetNext(); 

	if( _T('\0') != *m_CNAccess )
	{
		while( NULL != idptr.MP && false == idptr.MP->IsClass( m_CNAccess ) )
		{
			idptr = m_IDPtrList.GetNext();
		}
	}	
	
	return idptr.ID;
}

const _string CTable::GetNextID( _string strID )
{
	if( _T("") == strID )
	{
		return _T("");
	}
	
	m_IDPtrList.SetPos( strID.c_str() );
	IDPTR idptr = m_IDPtrList.GetNext();
	
	if( _T('\0') != *m_CNAccess && NULL != idptr.MP )
	{
		while( false == idptr.MP->IsClass( m_CNAccess ) )
		{
			idptr = m_IDPtrList.GetNext();
		}
	}	
	
	return idptr.ID;
}

int CTable::GetItemCount( LPCTSTR ptstrClassName )
{
	if( NULL == ptstrClassName )
	{
		return m_IDPtrList.GetCount();
	}
	
	int iCount = 0;
	IDPTR idptr = GetFirst( ptstrClassName );
	
	while( NULL != idptr.MP )
	{
		iCount++;
		idptr = GetNext();
	}
	
	return iCount;
}

CData *CTable::GetMP( LPCTSTR ptstrClassName, LPCTSTR ID )
{
	IDPTR IDPtr = m_IDPtrList.GetIDPtrFromID( ID );

	// After a reading from the disk, the memory pointer is not initialised; Take care.
	Extend( &IDPtr );
	return IDPtr.MP;
}

bool CTable::IsExist( LPCTSTR ptstrClassName )
{
	IDPTR idptr = GetFirst( ptstrClassName ); 
	return ( NULL != idptr.MP ) ? 1 : 0;
}

IDPTR CTable::GetIDInTempTable( LPCTSTR ID )
{
	if( true == m_arIDptr.empty() )
	{
		return _NULL_IDPTR;
	}
	
	IDPTR ReturnIDPtr = _NULL_IDPTR;
	
	for( std::vector<IDPTR>::iterator iter = m_arIDptr.begin(); iter != m_arIDptr.end() && _NULL_IDPTR == ReturnIDPtr; ++iter )
	{
		IDPTR idptr = *iter;
	
		if( 0 == _tcscmp( (*iter).ID, ID ) )
		{
			ReturnIDPtr = *iter;
		}
	}
	
	return ReturnIDPtr;
}

void CTable::ChangeIDInTempTable( LPCTSTR ptstrOldID, LPCTSTR ptstrNewID )
{
	if( true == m_arIDptr.empty() )
	{
		return;
	}
	
	bool bStop = false;
	
	for( std::vector<IDPTR>::iterator iter = m_arIDptr.begin(); iter != m_arIDptr.end() && false == bStop; ++iter )
	{
		IDPTR idptr = *iter;

		if( 0 == wcscmp( (*iter).ID, ptstrOldID ) )
		{
			// Don't call 'SetID' because this method reset 'MP', 'PP' and 'DB' members.
			wcscpy_s( (*iter).ID, SIZEOFINTCHAR((*iter).ID), ptstrNewID );
			bStop = true;
		}
	}
}

void CTable::CrossVerifyID()
{
	HYSELECT_TRY
	{
		// It's only when reading the database in text mode.
		// In binary mode it's done in the 'CDataBase::ReadObjects' method.
		CheckAndTransferTmpObj();
	}
	HYSELECT_CATCH_ARG( clHySelectException, _T("Error in 'CTable::CrossVerifyID' with the object '%s'."), GetIDPtr().ID )
}

void CTable::Copy( CData *pclDestination )
{
	CTable *pclDestinationTable = dynamic_cast <CTable *>( pclDestination );

	if( NULL == pclDestinationTable )
	{
		return;
	}

	// Base class.
	CData::Copy( pclDestinationTable );

	pclDestinationTable->SetName( m_Name.c_str() );
	pclDestinationTable->SetTrueParent( m_bTrueParent );
}

void CTable::OnDatabaseDestroy()
{
	// Unlock all objects.
	MakeEmpty();
}

#define CTABLEVERSION	3
// Version 3: HYS-1301: Add 'm_bTrueParent' (See .h to comment).
void CTable::Write( OUTSTREAM outf )
{
	// Version.
	short nVersion = CTABLEVERSION;
	outf.write( (const char *)(LPCTSTR)&nVersion, sizeof( nVersion ) );
	
	// Write base class.
	CData::Write( outf );

	// HYS-1301: write this variable before the ID because we will need it when reading binary database.
	outf.write( (char *)&m_bTrueParent, sizeof( m_bTrueParent ) );
	
	// Write the list.
	BYTE bOneMore = _DB_OBJECTSEPARATOR;
	IDPTR idptr = GetFirst();

	while( NULL != idptr.MP )
	{
		outf.write( (char *)&bOneMore, sizeof( bOneMore ) );	// tell the read function there is one more
		WriteString( outf, idptr.ID );							// Write the ID only
		idptr = GetNext();
	}

	bOneMore = _DB_ENDMARKER;
	outf.write( (char *)&bOneMore, sizeof( bOneMore ) );
	
	// Write the name.
	WriteString( outf, m_Name.c_str() );
}

bool CTable::Read( INPSTREAM  inpf )
{
	// Clear actual object.
	MakeEmpty();
	
	// Version.
	short Version;
	inpf.read( (char *)&Version, sizeof( Version ) );

	if( Version < 1 || Version > CTABLEVERSION )
	{
		return false;
	}
	
	// Read base class.
	if( false == CData::Read( inpf ) )
	{
		return false;
	}

	// HYS-1301: This variable is read before the IDs.
	if( Version >= 3 )
	{
		inpf.read( (char *)&m_bTrueParent, sizeof( m_bTrueParent ) );
	}

	// Read list.
	BYTE bOneMore = 0;

	for( inpf.read( (char *)&bOneMore, sizeof( char ) ); _DB_OBJECTSEPARATOR == bOneMore; inpf.read( (char *)&bOneMore, sizeof( char ) ) )
	{
		if( ( inpf.rdstate() & std::ifstream::failbit ) )
		{
			return false;
		}
		
		IDPTR LocalIDPtr = _NULL_IDPTR;

		// By default, we can set here that the parent of the local IDPtr is well this current table.
		// Pay attention: we are well talking here about LOCAL (!!) IDPtr in the table and not the real object pointed !!!
		LocalIDPtr.PP = this;

		if( false == ReadString( inpf, LocalIDPtr.ID, sizeof( LocalIDPtr.ID ) ) )
		{
			return false;
		}

		// If no ID defined, we continue.
		if( _T('\0') == *LocalIDPtr.ID )
		{
			continue;
		}

		// Check if object is already read.
		// Pay attention that 'Get' sends back a reference on the 'IDPtr' found. Any modification of one of this 
		// internal variable modify the true object pointed.
		IDPTR RealObjectIDPtr = GetDB()->Get( LocalIDPtr.ID );

		// If object is already in the database and the current table is the true parent...
		if( NULL != RealObjectIDPtr.MP )
		{
			// Can already copy 'MP' and 'DB'.
			LocalIDPtr.MP = RealObjectIDPtr.MP;
			LocalIDPtr.DB = RealObjectIDPtr.DB;

			// If the current table is the true parent...
			if( true == m_bTrueParent )
			{
				// Set also 'PP' of the real object to point to this current table.
				RealObjectIDPtr.PP = this;
			}
			
			// Now insert the local IDPtr in the current table.
			Insert( LocalIDPtr );
		}
		else
		{
			LocalIDPtr.MP = 0;
			LocalIDPtr.DB = GetDB();
			
			// Object not already read, store it temporarily.
			m_arIDptr.push_back( LocalIDPtr );
		}
	}

	// Read name.
	if( false == ReadString( inpf, m_Name ) )
	{
		return false;
	}

	return true;
}

#ifndef TACBX //TACBX////////////////////////////////////////////////////////

#define TABLE_NAME					_T("::Name=")
#define TABLE_TRUEPARENT			_T("::TrueParent=")
#define TABLE_IDPTR					_T("::IDPtr=")

void CTable::ReadText( INPSTREAM inpf, unsigned short *pusLineCount )
{
	try
	{
		// CTable format
		// line 0 : open curve bracket and call base class.
		// line 1 : the table name between double quotes (").
		// line 2 : true parent: 0/1 (See .h for comment).
		// line 3 and followings : optional ID of the objects in the table.
		// last line : close curve bracket.
	 
		// Skip the open curve bracket.
		TCHAR *ptcLine = ReadTextLine( inpf, pusLineCount );

		if( _T('{') != *ptcLine )
		{
			HYSELECT_THROW( _T("First line must be '{' (pos: %i)."), *pusLineCount );
		}
	
		// Inherited ?
		CString ihID;
	
		if( true == InheritedID( ptcLine, ihID ) )
		{
			IDPTR idptr = GetDB()->Get( ihID );

			if( _T('\0') == *idptr.ID )
			{
				HYSELECT_THROW( _T("ID for the base object must be defined (pos: %i)."), *pusLineCount );
			}

			( (CTable *)idptr.MP )->Copy( this );
		
			CStringArray ar;
			GetInheritedModifiedData( inpf, pusLineCount, &ar );
		
			for( int i = 0; i < ar.GetCount(); i++ )
			{
				CString str = ar.GetAt( i );

				InterpretInheritedData( &str );
			}
		}
		else
		{
			// Read base.
			CData::ReadText( inpf, pusLineCount );

			// Read name.
			ptcLine = ReadTextLine( inpf, pusLineCount );
			CString str = TABLE_NAME + CString( ptcLine );

			CTable::InterpretInheritedData( &str );

			// Read true parent.
			ptcLine = ReadTextLine( inpf, pusLineCount );
			str = TABLE_TRUEPARENT + CString( ptcLine );

			CTable::InterpretInheritedData( &str );

			// Now read ID if exist.
			do 
			{
				ptcLine = ReadTextLine( inpf, pusLineCount );

				if( _T('}') == *ptcLine )
				{
					break;
				}

				str = TABLE_IDPTR + CString( ptcLine );

				CTable::InterpretInheritedData( &str );

			}while( 1 );
		}
	}
	catch( CHySelectException &clHySelectException )
	{
		clHySelectException.AddMessage( _T("Error in 'CTable::ReadText' with the object '%s'."), __LINE__, __FILE__, GetIDPtr().ID );
		throw;
	}
}

void CTable::DropData( OUTSTREAM outf, int *piLevel )
{
	// Do not call base class.
	CString strTab = _T("");

	for( int i = *piLevel; i; i-- )
	{
		strTab += _T("\t");
	}

	// Database info.
	CString str;
	str.Format( _T("TABLE \"%s\" {ID = %s}\n"), m_Name.c_str(), CString( GetIDPtr().ID ) );
	WriteFormatedStringA2( outf, str, strTab );

	str.Format( _T("TrueParent: %s\n"), ( false == m_bTrueParent ) ? _T("false") : _T("true") );
	WriteFormatedStringA2( outf, str, strTab );

	// Write children.
	(*piLevel)++;

	for( IDPTR IDPtr = GetFirst(); _T('\0') != *IDPtr.ID; IDPtr = GetNext( IDPtr.MP ) )
	{
		IDPtr.MP->DropData( outf, piLevel );
	}

	(*piLevel)--;
	WriteFormatedStringA2( outf, _T("\n"), strTab );
}

void CTable::WriteText( OUTSTREAM outf, CString &strTab )
{
	CString str, str1;

	// Write object header.
	if( NULL != dynamic_cast<CTable *>(this) )
	{
		CData::WriteText( outf, strTab );
	}
	
	// Opening bracket.
	str.Format( _T("\r\n%s{"), strTab );
	outf.write( (const char *)(LPCTSTR)str, str.GetLength() * sizeof( TCHAR ) );
	strTab += _T("\t");
	
	// Data.
	WriteFormatedStringW( outf, m_Name.c_str(), strTab , true );

	str.Format( _T("\r\n%s%i"), strTab, ( false == m_bTrueParent ) ? 0 : 1 );
	outf.write( (char *)(LPCTSTR)str, str.GetLength() * sizeof( TCHAR ) );
	
	// Closing bracket.
	strTab.Delete( 0 ); // Delete one \t
	str.Format( _T("\r\n%s}"), strTab );
	outf.write( (const char *)(LPCTSTR)str, str.GetLength() * sizeof( TCHAR ) );
}

void CTable::InterpretInheritedData( CString *pStr )
{
	try
	{
		// Base class.
		int ipos = pStr->Find( CLASS( CTable ) );

		if( pStr->Left( 2 ) != _T("::") && ipos < 0 )
		{
			CData::InterpretInheritedData( pStr );
		}
		else
		{
			if( ipos >= 0 )
			{
				pStr->Delete( ipos, _tcsclen( CLASS( CTable ) ) );
			}

			int iEq = pStr->Find( _T("=") );

			if( iEq <= 0 )
			{
				HYSELECT_THROW( _T("Inherited variable must be preceeded by '='") );
			}

			CString strField = pStr->Mid( iEq + 1, pStr->GetLength() - iEq );

			if( pStr->Find( TABLE_NAME ) > -1 )
			{
				if( ( _T( '"' ) != strField.GetAt( 0 ) ) || ( _T( '"' ) != strField.GetAt( strField.GetLength() - 1 ) ) )
				{
					HYSELECT_THROW( _T("Argument for 'CTable::Name' must be surrounded by \".") );
				}
			
				if( strField.GetLength() > ( _TABLE_NAME_LENGTH + 2 ) )
				{
					CString str;
					str.Format( _T("Argument for 'CTable::Name' is too long (%i > %i)."), strField.GetLength(), _TABLE_NAME_LENGTH + 2 );
					throw CHySelectException( str, __LINE__, __FILE__ );
				}

				// Remove double quotes.
				strField.Delete( 0, 1 );
				strField.Delete( strField.GetLength() - 1, 1 );

				SetName( strField );
			}
			else if( pStr->Find( TABLE_TRUEPARENT ) > -1 )
			{
				m_bTrueParent = ( 0 == _ttoi( (LPCTSTR)strField ) ) ? false : true;
			}
			else if( pStr->Find( TABLE_IDPTR ) > -1 )
			{
				IDPTR LocalIDPtr = _NULL_IDPTR;

				// By default, we can set here that the parent of the local IDPtr is well this current table.
				// Pay attention: we are well talking here about LOCAL (!!) IDPtr in the table and not the real object pointed !!!
				LocalIDPtr.PP = this;
				wcscpy_s( LocalIDPtr.ID, _ID_LENGTH, strField );

				// If no ID defined, we continue.
				if( _T('\0') == *LocalIDPtr.ID )
				{
					return;
				}

				// Check if object is already read.
				// Pay attention that 'Get' sends back a reference on the 'IDPtr' found. Any modification of one of this 
				// internal variables modify the true object pointed.
				IDPTR RealObjectIDPtr = GetDB()->Get( LocalIDPtr.ID );
		
				// If object is already in the database...
				if( NULL != RealObjectIDPtr.MP )
				{
					// Can already copy 'MP' and 'DB'.
					LocalIDPtr.MP = RealObjectIDPtr.MP;
					LocalIDPtr.DB = RealObjectIDPtr.DB;

					// If the current table is the true parent...
					if( true == m_bTrueParent )
					{
						// Set also 'PP' of the real object to point to this current table.
						RealObjectIDPtr.PP = this;
					}
			
					// Now insert the local IDPtr in the current table.
					Insert( LocalIDPtr );
				}
				else
				{
					LocalIDPtr.MP = 0;
					LocalIDPtr.DB = GetDB();
			
					// Object not already read, store it temporarily.
					m_arIDptr.push_back( LocalIDPtr );
				}
			}
		}
	}
	catch( CHySelectException &clHySelectException )
	{
		clHySelectException.AddMessage( _T("Error in 'CTable::InterpretInheritedData'"), __LINE__, __FILE__ );
		throw;
	}
}

#endif //TACBX////////////////////////////////////////////////////////

//////////////////////////////////////////////////////////////////////////////////////////////
//
//	CTableOrdered class.
//			
//////////////////////////////////////////////////////////////////////////////////////////////

IMPLEMENT_DATA( CTableOrdered )
	
CTableOrdered::CTableOrdered( CDataBase *pDataBase, LPCTSTR ID )
	: CTable( pDataBase, ID )
{
	m_iKeyOrder = 0;
}

void CTableOrdered::FillMapListOrdered( std::map<int, CData *> *pMap )
{
	// Clear the map.
	pMap->clear();

	// Loop on all the table to ordered them into the Map.
	for( IDPTR IDPtr = CTable::GetFirst(); _T('\0') != *IDPtr.ID; IDPtr = CTable::GetNext() )
	{
		// 0 is reserved for object with no Key order.
		if( 0 == IDPtr.MP->GetKeyOrder() )
		{
			continue;
		}
		
		// Fill the map ordered by the Key.
		pMap->insert( std::pair<int, CData *>( IDPtr.MP->GetKeyOrder(), IDPtr.MP ) );
	}
}

bool CTableOrdered::Read( INPSTREAM inpf )
{
	// Version.
	short nVersion;
	inpf.read( (char *)&nVersion, sizeof( nVersion ) );

	if( nVersion < 1 || nVersion > 1 )
	{
		return false;
	}

	// Read base class.
	if( false == CTable::Read( inpf ) )
	{
		return false;
	}
	
	// Info.
	m_iKeyOrder = 0;
	inpf.read( (char *)&m_iKeyOrder, sizeof( m_iKeyOrder ) );
	return true;
}

void CTableOrdered::Write( OUTSTREAM outf )
{
	// Version.
	short nVersion = 1;
	outf.write( (const char *)(LPCTSTR)&nVersion, sizeof( nVersion ) );

	// Write base class.
	CTable::Write( outf );

	// Info.
	outf.write( (char *)&m_iKeyOrder, sizeof( m_iKeyOrder ) );
}

#ifndef TACBX //TACBX////////////////////////////////////////////////////////
void CTableOrdered::ReadText( INPSTREAM  inpf, unsigned short *pusLineCount )
{
	try
	{
		// CTableOrdered format
		// line 0 : open curve bracket.
		// line 1 : Call CTable base class.
		// line 2 : Key number.
		// last line : close curve bracket.
	 
		// Skip the open curve bracket.
		TCHAR *ptcLine = ReadTextLine(inpf, pusLineCount); 

		if( _T('{') != *ptcLine )
		{
			HYSELECT_THROW( _T("First line must be '{' (pos: %i)."), *pusLineCount );
		}

		// Read base.
		CTable::ReadText( inpf, pusLineCount );

		// Read the key order.
		ptcLine = ReadTextLine( inpf, pusLineCount); 

		if( 0 == _ttoi( ptcLine ) )
		{
			HYSELECT_THROW( _T("Can't read the key order (%s) (pos: %i)."), ptcLine, *pusLineCount );
		}

		SetKeyOrder( _ttoi( ptcLine ) );

		// Read the close curve bracket.
		ptcLine = ReadTextLine(inpf, pusLineCount);

		if( _T('}') != *ptcLine )
		{
			HYSELECT_THROW( _T("Last line must be '}' (pos: %i)."), *pusLineCount );
		}
	}
	catch( CHySelectException &clHySelectException )
	{
		clHySelectException.AddMessage( _T("Error in 'CTableOrdered::ReadText' with the object '%s'."), __LINE__, __FILE__, GetIDPtr().ID );
		throw;
	}
}

#endif //TACBX////////////////////////////////////////////////////////


///////////////////////////////////////////////////////////////////////////////////
// class CXmlStrTab
///////////////////////////////////////////////////////////////////////////////////
CXmlStrTab XmlStrTab;
CXmlStrTab *GetpXmlStrTab(){return &XmlStrTab;};

CXmlStrTab::CXmlStrTab()
{
	Clear();
}

CXmlStrTab::~CXmlStrTab()
{
	Clear();
}

void CXmlStrTab::Init( std::string strFileName, _string strLanguage )
{
	HYSELECT_TRY
	{
		std::wstring  wlang;
		wlang = strLanguage;

		CXmlReader XmlReader;
		Clear();
		
		int iFileSize = XmlReader.Open( strFileName );

		if( 0 == iFileSize )
		{
			HYSELECT_THROW( _T("Can't open the '%s' file."), strFileName );
		}

		m_MapBuf = new WCHAR[iFileSize];

		if( NULL == m_MapBuf )
		{
			HYSELECT_THROW( _T("Internal error: 'm_MapBuf' instantiation error (pos: %i).") );
		}

		WCHAR *ptcPos;
		WCHAR *ptcPosBuffer = m_MapBuf;
		WCHAR *ptcPosText;
		WCHAR *ptcPosTrs;
		WCHAR StringTable[] = { 's', 't', 'r', 'i', 'n', 'g', '\0' };

		// Default is english.
		WCHAR DefaultLang[] = {'e','n','\0'};

		bool bInt = ( !wcscmp( (WCHAR *)wlang.c_str(), DefaultLang ) );
		
		// Skip first element that contains available language
		std::wstring strID = XmlReader.GetNextElement();

		int iCount = 0;
		
		while (!strID.empty())
		{
			WCHAR *posIDS = ptcPosBuffer;
			
			for( ptcPos = XmlReader.GetElementContent( (WCHAR*)L"IDS"); NULL != ptcPos && _T('\0') != *ptcPos; ptcPos++ )
			{
				*ptcPosBuffer++=*ptcPos;
			}
			
			*ptcPosBuffer++ = '\0';

			if( !*posIDS )
			{
				break;
			}
			
			ptcPosText = ptcPosBuffer;

			// Load default international string, should exist
			for( ptcPos = XmlReader.GetElementContent( DefaultLang, StringTable ); NULL != ptcPos && _T('\0') != *ptcPos; ptcPos++ )
			{
				if( '&' == *ptcPos )	// Special XML detected character
				{
					*ptcPosBuffer++ = XmlReader.ProcessXMLSpecChar( &ptcPos );
				}
				else
				{
					XmlReader.ProcessNewLineChar( &ptcPos, &ptcPosBuffer );
				}
			}

			if( !ptcPos || !*ptcPosText )
			{
				*ptcPosBuffer++ = ' ';
			}

			*ptcPosBuffer++ = '\0';
				
			if( !bInt )
			{
				ptcPosTrs = ptcPosText;

				// Load translated string if exist.
				for( ptcPos = XmlReader.GetElementContent( (WCHAR *)wlang.c_str(), StringTable); NULL != ptcPos && _T('\0') != *ptcPos; ptcPos++ )
				{
					if( '&' == *ptcPos )	// Special XML detected character
					{
						*ptcPosTrs++ = XmlReader.ProcessXMLSpecChar( &ptcPos );
					}
					else
					{
						XmlReader.ProcessNewLineChar( &ptcPos, &ptcPosTrs );
					}
				}

				if( ptcPosTrs != ptcPosText )				// !pos == Not found keep the international ones
				{
					*ptcPosTrs++ = '\0';
					ptcPosBuffer = ptcPosTrs;
				}		
			}

			m_mIDS.insert( std::pair<WCHAR *, WCHAR *>( posIDS, ptcPosText ) );
			++iCount;
		};

		XmlReader.Close();
	}
	HYSELECT_CATCH( clHySelectException, _T("Error in 'CXmlStrTab::Init'.") )
}

_string *CXmlStrTab::GetIDSStr( _string IDS )
{
	std::wstring wIDS;

	if( true == m_mIDS.empty() )
	{
		return NULL;
	}

	const TCHAR IDS_Id[] = _T("IDS_");

	if( IDS.size() <= 4 )
	{
		return NULL;
	}

	for( unsigned int i = 0; i < 4; i++ )
	{
		if( IDS.at(i) != IDS_Id[i] )
		{
			return NULL;
		}
	}
	
#ifndef UNICODE
	// SBCS only on PC, Use MFC to make conversion
	CStringW wstr = IDS.c_str();
	wIDS = (LPCWSTR) wstr;
#else
	wIDS = IDS;
#endif
	
	std::map<WCHAR *, WCHAR *,mapcomp>::iterator It;
	WCHAR *pos = (WCHAR *)wIDS.c_str();
	It = m_mIDS.find( pos );

	if( It != m_mIDS.end() )
	{
#ifndef UNICODE
		CStringW str = It->second;
		CString strOut = str;
		m_retStr = (LPCTSTR) strOut;
#else
		m_retStr = It->second;
#endif
		return(&m_retStr);
	}

	return NULL;
}

// Find IDS and replace by his translation
bool CXmlStrTab::FindAndReplace( _string &IDS )
{
	_string *pIDS = GetIDSStr( IDS );
	
	if( NULL != pIDS )
	{
		IDS = *pIDS;
		return true;
	}

	return false;
}

void CXmlStrTab::Clear()
{
	// Clear content.
	m_mIDS.clear();

	if( NULL != m_MapBuf )
	{
		delete[]m_MapBuf;
	}

	m_MapBuf = NULL;
}

#ifndef TACBX
///////////////////////////////////////////////////////////////////////////////////
// class CXmlRcTab
///////////////////////////////////////////////////////////////////////////////////
CXmlRcTab XmlRcTab;
CXmlRcTab *GetpXmlRcTab() { return &XmlRcTab; }

CXmlRcTab XmlTipTab;
CXmlRcTab *GetpXmlTipTab() { return &XmlTipTab; }

CXmlRcTab::CXmlRcTab()
	: m_MapBuf( 0 )
{
	m_It = m_mIDS.begin();
	Clear();
}

CXmlRcTab::~CXmlRcTab()
{
	Clear();
}

void CXmlRcTab::GetLangList( std::string strFileName, std::vector<std::wstring> &vLangList )
{
	CXmlReader XmlReader;

	if( 0 == XmlReader.Open( strFileName ) )
	{
		return;
	}

	WCHAR *pos = XmlReader.GetElementContent( _T("code") );

	while( _T('\0') != *pos )
	{
		vLangList.push_back( pos );
		pos = XmlReader.GetElementContent( _T("code") );
	}

	XmlReader.Close();

}

void CXmlRcTab::Init( std::string strFileName, _string strLanguage )
{
	try
	{
		std::wstring wlang;
		wlang = strLanguage;

		CXmlReader XmlReader;
		Clear();
		
		int iFileSize = XmlReader.Open( strFileName );

		if( 0 == iFileSize )
		{
			HYSELECT_THROW( _T("Can't open the '%s' file."), strFileName );
		}
		
		m_MapBuf = new WCHAR[iFileSize];				//Allocate file size

		if( NULL == m_MapBuf ) 
		{
			HYSELECT_THROW( _T("Internal error: 'm_MapBuf' instantiation error (pos: %i).") );
		}

		WCHAR *ptcPos = NULL;
		WCHAR *ptcPosBuffer = m_MapBuf;
		WCHAR *ptcPosText = NULL;
		WCHAR *ptcPosTrs = NULL;
		WCHAR StringTable[] = {'s','t','r','i','n','g','\0'};

		// Default is eng.
		WCHAR DefaultLang[] = {'e','n','\0'};

		bool bInt = ( 0 == wcscmp( (WCHAR *)wlang.c_str(), DefaultLang ) );
		
		// Skip first element that contains available language.
		std::wstring strID = XmlReader.GetNextElement();

		int iCount = 0;

		while( false == strID.empty() )
		{
			WCHAR *posIDS = ptcPosBuffer;

			for( ptcPos = XmlReader.GetElementContent(L"IDnr"); NULL != ptcPos && _T('\0') != *ptcPos; ptcPos++ )
			{
				*ptcPosBuffer++ = *ptcPos;
			}

			*ptcPosBuffer++ = '\0';
			
			if( _T('\0') == *posIDS )
			{
				break;
			}
			
			ptcPosText = ptcPosBuffer;
			
			// Load default international string, should exist.
			for( ptcPos = XmlReader.GetElementContent( DefaultLang, StringTable ); NULL != ptcPos && _T('\0') != *ptcPos; ptcPos++ )
			{
				if( _T('&') == *ptcPos )	// Special XML detected character
				{
					*ptcPosBuffer++ = XmlReader.ProcessXMLSpecChar( &ptcPos );
				}
				else
				{
					XmlReader.ProcessNewLineChar( &ptcPos, &ptcPosBuffer );
				}
			}
		
			if( NULL == ptcPos || _T('\0') == *ptcPosText )
			{
				*ptcPosBuffer++ = ' ';
			}

			*ptcPosBuffer++ = '\0';
				
			if( false == bInt )
			{
				ptcPosTrs = ptcPosText;
				
				// Load translated string if exist.
				for( ptcPos = XmlReader.GetElementContent( (WCHAR *)wlang.c_str(), StringTable ); NULL != ptcPos && _T('\0') != *ptcPos; ptcPos++ )
				{
					if( _T('&') == *ptcPos )	// Special XML detected character
					{
						*ptcPosTrs++ = XmlReader.ProcessXMLSpecChar( &ptcPos );
					}
					else 
					{
						XmlReader.ProcessNewLineChar( &ptcPos, &ptcPosTrs );
					}
				}
				
				if( ptcPosTrs != ptcPosText )				// !pos == Not found keep the international ones
				{
					*ptcPosTrs++ = _T('\0');
					ptcPosBuffer = ptcPosTrs;
				}		
			}
			
			m_mIDS.insert( std::pair<unsigned int, WCHAR *>( _ttoi( posIDS ), ptcPosText ) );
			++iCount;
		}

		XmlReader.Close();
	}
	catch( CHySelectException &clHySelectException )
	{
		clHySelectException.AddMessage( _T("Error in 'CXmlRcTab::Init'."), __LINE__, __FILE__ );
		throw;
	}
}

WCHAR *CXmlRcTab::GetIDSStr( unsigned int uiIDS )
{
	if( 0 == m_mIDS.size() )
	{
		return NULL;
	}
	
	std::map<unsigned int, WCHAR *>::iterator It;
	It = m_mIDS.find( uiIDS );
	
	if( It != m_mIDS.end() )
	{
		return It->second;
	}

	return NULL;
}

WCHAR *CXmlRcTab::GetIDSStrByPos( unsigned int uiPos )
{
	if( 0 == m_mIDS.size() || uiPos >= m_mIDS.size() )
	{
		return NULL;
	}

	std::map<unsigned int, WCHAR *>::iterator It;
	unsigned int ui;

	for( It = m_mIDS.begin(), ui = 0; It != m_mIDS.end() && ui < uiPos; ++It, ++ui )
	{}

	return ( It != m_mIDS.end() ) ? It->second : NULL;
}

int CXmlRcTab::GetIDSByPos( unsigned int uiPos )
{
	if( 0 == m_mIDS.size() || uiPos >= m_mIDS.size() )
	{
		return NULL;
	}

	std::map<unsigned int, WCHAR *>::iterator It;
	unsigned int ui;

	for (It = m_mIDS.begin(), ui = 0; It != m_mIDS.end() && ui < uiPos; ++It, ++ui)
	{
	}

	return (It != m_mIDS.end()) ? It->first : -1;
}

WCHAR *CXmlRcTab::GetFirstIDSStr()
{
	m_It = m_mIDS.begin();
	return (m_It != m_mIDS.end()) ? m_It->second : NULL;
}

WCHAR *CXmlRcTab::GetNextIDSStr()
{
	if( m_It != m_mIDS.end() )
	{
		++m_It;
	}

	return ( m_It != m_mIDS.end() ) ? m_It->second : NULL;
}

void CXmlRcTab::Clear()
{
	// Clear content.
	m_mIDS.clear();
	
	if( NULL != m_MapBuf )
	{
		delete[]m_MapBuf;
	}
	
	m_MapBuf = NULL;
}

///////////////////////////////////////////////////////////////////////////////////
//
//								CFieldTab
//
//
///////////////////////////////////////////////////////////////////////////////////
CFieldTab::CFieldTab()
{
	m_iCurrentField = -1;
	m_iCurrentPos = -1;
	memset( &m_tcBuffer, 0, 256 * sizeof( TCHAR ) );
}

CFieldTab::~CFieldTab()
{
	m_List.RemoveAll();
}

#define FIELDSSEPARATOR	';'

void CFieldTab::ReadFieldTab( CFileTxt &inpf )
{
	TCHAR tcBuffer[1024];
	int iLine = 0;
	
	while( inpf.ReadTxtLine( tcBuffer,sizeof( tcBuffer ), &iLine ) > 0 )
	{
		CString str;					
		str = tcBuffer;
		str.TrimLeft();
		str.TrimRight();

		if( true == str.IsEmpty() )
		{
			continue;
		}

		m_List.AddTail( str );

		if( ACTIVATE_TRACE_DB_READING )
		{
			TRACE( _T("Ln %u : %s \n" ), iLine, (LPCTSTR)str );
		}
	}
}

LPCTSTR CFieldTab::GetFirstField( LPCTSTR pstr )
{
	m_iCurrentField = 0;
	m_iCurrentPos = 0;
	return GetField( 0, pstr );
}

LPCTSTR CFieldTab::GetNextField( LPCTSTR pstr )
{
	if( m_iCurrentPos > 0 )
	{
		m_iCurrentPos++;
	}

	if( m_iCurrentField < 0 || m_iCurrentPos < 0 )
	{
		return 0;
	}

	if( _tcslen( pstr ) < (unsigned)m_iCurrentPos || _T('\0') == *( pstr + m_iCurrentPos ) )
	{
		m_iCurrentPos = -1;
		m_iCurrentField = -1;
		return 0;
	}

	m_iCurrentField++;

	TCHAR *pa = m_tcBuffer;
	pstr += m_iCurrentPos;
	bool bString = false;

	while( _T('\0') != *pstr && *pstr != FIELDSSEPARATOR )
	{
		if( false == bString && ( _T(' ') == *pstr || _T('\t') == *pstr ) )	// Remove all space characters if !bString
		{
			pstr++;
		}
/*		else if( _T('\"') == *pstr )							// Toggle bString and skip when meeting a "
		{
			bString = !bString;
			pstr++;
		}
*/		else
		{
			if( _T('\"') == *pstr )
			{
				bString = true;
			}

			*pa++=*pstr++;
		}

		m_iCurrentPos++;
	}

	*pa = 0;

	// Remove first and last '"'.
	CString str = m_tcBuffer;
	str.TrimRight();

	if( _T('\"') == *m_tcBuffer )
	{
		str.Delete( 0 );
		int i = str.GetLength() - 1;

		if( i > 0 )
		{
			if( _T('\"') == str[i] )
			{
				str.Delete( i );
			}
		}
	}

	_tcsncpy_s( m_tcBuffer, SIZEOFINTCHAR( m_tcBuffer ), (LPCTSTR)str, SIZEOFINTCHAR( m_tcBuffer ) - 1 );

	return m_tcBuffer;
}

LPCTSTR CFieldTab::GetField( int index, LPCTSTR pstr, int pos )
{
	TCHAR *pa = m_tcBuffer;
	ASSERT( _tcslen( pstr ) >= (unsigned)pos );

	pstr += pos;
	int i;

	for( i = 0;  i < index && _T('\0') != *pstr; )
	{
		pos++;
		pstr++;

		if( FIELDSSEPARATOR == *pstr )
		{
			i++;
		}
	}

	if( _T('\0') == *pstr && i != index )
	{
		m_iCurrentField = -1;
		m_iCurrentPos = -1;
		return NULL;
	}

	if( i != 0 )
	{
		pstr++;
		pos++;
	}

	bool bString = false;

	while( _T('\0') != *pstr && *pstr != FIELDSSEPARATOR )
	{
		if( false == bString && ( _T(' ') == *pstr || _T('\t' )== *pstr ) )	// Remove all space characters if !bString
		{
			pstr++;
		}
		else
		{
			if( _T('\"') == *pstr )
			{
				bString = true;
			}

			*pa++=*pstr++;
		}

		pos++;
	}

	m_iCurrentPos = pos;
	*pa = 0;
	m_iCurrentField = index;

	// Trim right and remove first and last '"'.
	size_t lenArray = _tcslen( m_tcBuffer );
	
	while( 0 != iswspace( m_tcBuffer[lenArray - 1] ) )
	{
		m_tcBuffer[lenArray - 1] = 0;
		lenArray--;
	}

	if( _T('\"') == *m_tcBuffer )
	{
		memmove_s( m_tcBuffer, SIZEOFINTCHAR( m_tcBuffer ), &m_tcBuffer[1], SIZEOFINTCHAR( m_tcBuffer ) - 1 );
		lenArray--;
	}

	if( _T('\"') == m_tcBuffer[lenArray - 1] )
	{
		m_tcBuffer[lenArray - 1] = 0;
		lenArray--;
	}

	return m_tcBuffer;
}

CString CFieldTab::GetFieldLineString( POSITION Pos )
{
	CString str;
	
	if( NULL == Pos )
	{
		str = m_List.GetTail();
	}
	else
	{									// Pos indicates the next filter in the list
		m_List.GetPrev( Pos );			// Get the correct filter position  
		str = m_List.GetNext( Pos );
	}

	return str;
}

int CFieldTab::GetNumberOfFields( LPCTSTR pstr )
{
	CString str( pstr );

	if( true == str.IsEmpty() )
	{
		return 0;
	}

	int iCount = 1;
	int iStart = 0;

	while( -1 != ( iStart = str.Find( FIELDSSEPARATOR, iStart ) ) )
	{
		iCount++;
		iStart++;
	}

	return iCount;
}

///////////////////////////////////////////////////////////////////////////////////
//
//								CFilterTab
//
//
///////////////////////////////////////////////////////////////////////////////////
CFilterTab FilterTab;

//
// Read a TXT file which contains filtering instructions for a specific classname
//
// CLASSNAME;TABLE_ID;ID;EDIT;...
//	or
// CLASSNAME;TABLE_ID;ID;DELETE;...
//						
//	CLASSNAME is the access key to the filter specification; it must exist
//	TABLE_ID is the Table identificator; can be empty
//	ID is the object identificator; can be empty
//	EDIT is used when some object properties must be modified
//  DELETE is used when the object must be deleted	
//
//		EDIT in case of object derived from CDB_TAProduct, we have:
//	CLASSNAME;TABLE_ID;ID;EDIT;FAMILY;CONNECTION;VERSION;d,a,h,f (availability fields CDB_Thing);mask1,int1;Priority
//		CDB_RegulatingValve
//		CDB_FixedOrifice
//		CDB_VenturiValve
//		CDB_CommissioningSet
//		CDB_DpController
//		CDB_Component
//		CDB_ControlValve
//		CDB_ThermostaticValve
//		CDB_ShutoffValve
//
//		EDIT in case of any other object derived from CData, we have:
//	CLASSNAME;TABLE_ID;ID; EDIT ; d,a,h (availability fields CDB_Thing)  
//

int CFilterTab::IsFiltered( POSITION &Pos, LPCTSTR pClassName, LPCTSTR pTabId, LPCTSTR pID )
{
	CString str;

	if( NULL == Pos )
	{
		Pos = m_List.GetHeadPosition();
	}
	
	bool fClassNameFound = false;
	CString strKey;
	
	while( NULL != Pos )
	{
		str = m_List.GetNext( Pos );
		strKey = GetFirstField( (LPCTSTR)str );
		ASSERT( false == strKey.IsEmpty() );

		if( 0 == _tcscmp( pClassName, (LPCTSTR)strKey ) )
		{
			fClassNameFound = true;
			break;
		}
	}

	if( false == fClassNameFound )
	{
		return FS_NotFiltered;
	}

	if( true == str.IsEmpty() )
	{
		return FS_NotFiltered;
	}

	// Check 'TableID'.
	CString strTab = GetNextField( (LPCTSTR)str );
	
	if( false == strTab.IsEmpty() )
	{
		if( 0 != _tcscmp( pTabId, (LPCTSTR)strTab) )
		{
			return FS_NotFiltered;
		}
	}

	// Check ID.
	CString strID = GetNextField( (LPCTSTR)str );
	
	if( false == strID.IsEmpty() )
	{
		if( 0 != _tcscmp( pID, (LPCTSTR)strID) )
		{
			return FS_NotFiltered;
		}
	}

	// Check command.
	CString strCmd = GetNextField( (LPCTSTR)str );

	if( 0 != _tcscmp( _T("DELETE"), (LPCTSTR)strCmd ) ) 
	{
		return FS_Filtered;
	}

	return FS_MustBeDeleted;
}

///////////////////////////////////////////////////////////////////////////////////
//
//								CRedefineTab
//
//
///////////////////////////////////////////////////////////////////////////////////
CRedefineTab RedefineTab;

//
// Read a TXT file which contains redefinition instructions for a specific CDB_TAProduct
//
// ID ; Name ; Family_ID ; Qrs_ID ; CBI Type
//						
//	ID is the object identificator; it must exist
//	Name is the redefined product name; can be empty
//	Family_ID is the redefined family identificator; can be empty
//	Qrs_ID is the redefined QRS identificator; can be empty
//	CBI Type is the redefined product type in CBI; can be empty
//


// Return 0 if the object is not redefined
// Return 1 if yes
// Pos is updated with the position of the next filter string; or null at the end of the list 
int CRedefineTab::IsRedefined(POSITION &Pos,LPCTSTR pID)
{
	CString str;
	if (!Pos)
		Pos = m_List.GetHeadPosition();
	
	CString strKey;
	while (Pos)
	{
		str = m_List.GetNext(Pos);
		strKey = GetFirstField((LPCTSTR)str);
		ASSERT(!strKey.IsEmpty());

		if (!_tcscmp(pID,(LPCTSTR)strKey)) return 1;
	};

	return 0;
}

///////////////////////////////////////////////////////////////////////////////////
//
//								CLocArtNumTab
//
//
///////////////////////////////////////////////////////////////////////////////////
CLocArtNumTab LocArtNumTab;

//
// Read a TXT file which contains Local Article number for a CDB_TAProduct
//
// TA_AN; LAN
//						
//	TA_AN (TA Article Number) is the object identificator; it must exist
//	LAN is the Local Article Number; can be empty
//

void CLocArtNumTab::ReadFile( CFileTxt &inpf )
{
	HYSELECT_TRY
	{
		TCHAR tcBuffer[1024];
		int iLine = 0;

		CTable *pRootTable = (CTable *)( TASApp.GetpTADB()->Get( _T("ROOT_TABLE" ) ).MP );

		if( NULL == pRootTable )
		{
			HYSELECT_THROW( _T("Can't retrieve the 'ROOT_TABLE' from the database.") );
		}

		IDPTR NewObject;
		TASApp.GetpTADB()->CreateObject( NewObject, CLASS( CDB_MapStrStr ), _T("LOCALARTNUM") );
		pRootTable->Insert( NewObject );

		m_pMapLocalArticleNumber = dynamic_cast<CDB_MapStrStr *>( NewObject.MP );
		
		if( NULL == m_pMapLocalArticleNumber )
		{
			HYSELECT_THROW( _T("The object 'LOCALARTNUM' is created but it's not a 'CDB_MapStrStr' object.") );
		}

		CString strSeparator( _T(FIELDSSEPARATOR) );
		TCHAR tczSeparator[1024];
		_tcscpy_s( tczSeparator, strSeparator );

		while( inpf.ReadTxtLine( tcBuffer, sizeof( tcBuffer ), &iLine ) > 0 )
		{
			CString strFirst, strSecond;

			// Get first token.
			TCHAR *ptcContext = NULL;
			TCHAR *ptcToken = _tcstok_s( tcBuffer, tczSeparator, &ptcContext );

			if( NULL == ptcToken )
			{
				continue;
			}

			strFirst = ptcToken;
			_CleanArticleNumber( strFirst );

			if( true == strFirst.IsEmpty() )
			{
				continue;
			}

			// Get second token.
			// Remark: do not call '_CleanArticleNumber' for the local article number. We want to keep the format.
			ptcToken = _tcstok_s( NULL, tczSeparator, &ptcContext ); 

			if( NULL == ptcToken )
			{
				continue;
			}

			strSecond = ptcToken;
			strSecond.Trim();

			if( true == strSecond.IsEmpty() )
			{
				continue;
			}

			int iComment = strSecond.Find( _T("//") );

			if( iComment != -1 )
			{
				strSecond.Delete( iComment, strSecond.GetLength() - iComment );
			}

			strSecond.Trim();

			if( true == strSecond.IsEmpty() )
			{
				continue;
			}

			m_pMapLocalArticleNumber->Add( strFirst, strSecond );
		}
	}
	HYSELECT_CATCH( clHySelectException, _T("Error in 'CLocArtNumTab::ReadFile'.") )
}

CString CLocArtNumTab::GetLocalArticleNumber( CString strFirst )
{
	if( true == strFirst.IsEmpty() )
	{
		return _T("");
	}

	// Remark: all articles in LANTab.txt are set in format "*****.***" without any connection references.
	//         Thus when calling this method, we must exclude connection references if they are exist.
	if( strFirst.Find( _T("&") ) != -1 )
	{
		CString strTemp = strFirst;
		int iPos = strTemp.Find( _T("&") );
		strFirst = strTemp.Left( iPos );
	}

	_CleanArticleNumber( strFirst );
	CString strReturn = _T("");

	if( NULL != m_pMapLocalArticleNumber )
	{
		strReturn = m_pMapLocalArticleNumber->Get( strFirst );
	}
	else
	{
		CTable *pRootTable = (CTable *)( TASApp.GetpTADB()->Get( _T("ROOT_TABLE") ).MP );
		ASSERT( NULL != pRootTable );

		if( NULL != pRootTable )
		{
			m_pMapLocalArticleNumber = dynamic_cast<CDB_MapStrStr *>( pRootTable->Get( _T("LOCALARTNUM") ).MP );

			if( NULL != m_pMapLocalArticleNumber )
			{
				strReturn = m_pMapLocalArticleNumber->Get( strFirst );
			}
		}	
	}

	return strReturn;
}

void CLocArtNumTab::_CleanArticleNumber( CString &strArticleNumber )
{
	strArticleNumber.Trim();
	CString strArticleCleaned = _T("");
	
	for( int i = 0; i < strArticleNumber.GetLength(); ++i )
	{
		if( 0 != _istalnum( strArticleNumber.GetAt( i ) ) )
		{
			strArticleCleaned.AppendChar( strArticleNumber.GetAt( i ) );
		}
	}

	strArticleNumber = strArticleCleaned;
}


#endif //TACBX////////////////////////////////////////////////////////
