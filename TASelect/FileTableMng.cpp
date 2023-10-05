#include "stdafx.h"
#include <afxtempl.h>
#include <string>
#include <map>
#include "Global.h"
#include "Utilities.h"
#include "FileTableMng.h"

////////////////////////////////////////////////////////////////
//
//	Class for the management of a HostFileTable (table structure
//  of download files)
//
////////////////////////////////////////////////////////////////

CFileTable::CFileTable()
{
	_MakeEmpty();
}

bool CFileTable::SetActivePosition( POSITION pos )
{
	m_pos = pos;
	bool fEnd = ( NULL == m_pos ) ? true : false;
	m_ActivePos = m_pos;

	if( false == fEnd )
	{
		m_Active = m_List.GetNext( m_pos );
	}

	return fEnd;
}

void CFileTable::DeleteAllItems()
{
	m_List.RemoveAll();
}

bool CFileTable::ReadFile( TCHAR *ptcFileName )
{
	// Open the file.
	CFileStatus fs;

	if( FALSE == CFile::GetStatus( ptcFileName, fs ) )
	{
		// FileName does not exist!
		return false;
	}

	CFile f( ptcFileName, CFile::modeRead );

	// Read the content.
	CString strtmp, strTargetFile;
	int iIndex;
	SRecord rec;
	_MakeEmpty();
	bool fError = false;

	CString str;
	for( bool bContinue = _ReadTextLine( f, str ); true == bContinue; bContinue = _ReadTextLine( f, str ) )
	{
		ZeroMemory( &rec, sizeof( rec ) );
		rec.m_dReqAppVersion = VersionStringToDouble( DEFREQAPPVERSION );

		// Decompose the str into:
		// source; target; version; size; min App Version; date; requiring App Version

		// Source ------------------------------------------------
		if( true == str.IsEmpty() )
		{
			continue;
		}

		// Never change source folder; this could be stored on a linux server that is case sensitive for filename!
		iIndex = str.Find( ';' );

		if( iIndex < 1 )
		{
			continue;
		}

		strtmp = str.Left( iIndex );
		strtmp.TrimRight();
		_tcscpy_s( rec.m_tcSource, SIZEOFINTCHAR( rec.m_tcSource ), strtmp );
		str = str.Mid( iIndex + 1 );

		// Target ------------------------------------------------
		iIndex = str.Find( ';' );

		if( iIndex < 1 )
		{
			#ifdef VERBOSE
			CString msg;
			msg.Format( _T("Syntax error in %s : line %s\n"), ptcFileName, str );
			AfxMessageBox( msg, MB_ICONSTOP );
			_MakeEmpty();
			#else
			//TRACE(_T("Syntax error in %s : line %s"),FileName,str);
			#endif
			fError = true;
			break;
		}

		ASSERT( str.GetLength() < sizeof( rec.m_tcTarget ) );
		strtmp = str.Left( iIndex );
		strtmp.TrimRight();
		_tcscpy_s( rec.m_tcTarget, SIZEOFINTCHAR( rec.m_tcTarget ), strtmp );
		strTargetFile = strtmp;
		str = str.Mid( iIndex + 1 );

		// Version ------------------------------------------------
		iIndex = str.Find( ';' );

		if( iIndex < 1 )
		{
			#ifdef VERBOSE
			CString msg;
			msg.Format( _T("Syntax error in %s : line %s\n"), ptcFileName, str );
			AfxMessageBox( msg, MB_ICONSTOP );
			_MakeEmpty();
			#else
			//TRACE(_T("Syntax error in %s : line %s\n"),FileName,str);
			#endif
			fError = true;
			break;
		}

		strtmp = str.Left( iIndex );
		strtmp.TrimRight();
		rec.m_dVersion = VersionStringToDouble( strtmp ); //atof(str.Left(index));

		if( rec.m_dVersion <= 0 )
		{
			#ifdef VERBOSE
			CString msg;
			msg.Format( _T("Syntax error in %s : line %s\n"), ptcFileName, str );
			AfxMessageBox( msg, MB_ICONSTOP );
			_MakeEmpty();
			#else
			//TRACE(_T("Syntax error in %s : line %s\n"),FileName,str);
			#endif
			fError = true;
			break;
		}

		str = str.Mid( iIndex + 1 );

		// Size --------------------------------------------------
		iIndex = str.Find( ';' );

		if( iIndex < 1 )
		{
			#ifdef VERBOSE
			CString msg;
			msg.Format( _T("Syntax error in %s : line %s\n"), ptcFileName, str );
			AfxMessageBox( msg, MB_ICONSTOP );
			_MakeEmpty();
			#else
			//TRACE(_T("Syntax error in %s : line %s\n"),FileName,str);
			#endif
			fError = true;
			break;
		}

		strtmp = str.Left( iIndex );
		strtmp.TrimRight();
		rec.m_ulSize = _ttol( strtmp );

		if( rec.m_ulSize <= 0 )
		{
			#ifdef VERBOSE
			CString msg;
			msg.Format( _T("Syntax error in %s : line %s\n"), ptcFileName, str );
			AfxMessageBox( msg, MB_ICONSTOP );
			_MakeEmpty();
			#else
			//TRACE(_T("Syntax error in %s : line %s\n"),FileName,str);
			#endif
			fError = true;
			break;
		}

		str = str.Mid( iIndex + 1 );

		// minimum Application Version ---------------------------
		iIndex = str.Find( ';' );

		if( iIndex < 1 )
		{
			#ifdef VERBOSE
			CString msg;
			msg.Format( _T("Syntax error in %s : line %s\n"), ptcFileName, str );
			AfxMessageBox( msg, MB_ICONSTOP );
			_MakeEmpty();
			#else
			//TRACE(_T("Syntax error in %s : line %s\n"),FileName,str);
			#endif
			fError = true;
			break;
		}

		strtmp = str.Left( iIndex );
		strtmp.TrimRight();
		rec.m_dMinAppVersion = VersionStringToDouble( strtmp ); //atof(str.Left(index));

		if( rec.m_dMinAppVersion <= 0 )
		{
			#ifdef VERBOSE
			CString msg;
			msg.Format( _T("Syntax error in %s : line %s\n"), ptcFileName, str );
			AfxMessageBox( msg, MB_ICONSTOP );
			_MakeEmpty();
			#else
			//TRACE(_T("Syntax error in %s : line %s\n"),FileName,str);
			#endif
			fError = true;
			break;
		}

		str = str.Mid( iIndex + 1 );

		// Skip the date -----------------------------------------
		iIndex = str.Find( ';' );

		if( iIndex < 1 )
		{
			//TRACE(_T("In %s : line with %s terminates with the date, skip the date\n"),FileName,strTargetFile);
			strtmp = str;
		}
		else
		{
			strtmp = str.Left( iIndex );
		}

		strtmp.TrimRight();
		int iYear = 0;
		int iMonth = 0;
		int iDay = 0;
		int iHour = 0;
		int iMin = 0;
		int iSec = 0;
		bool bValidDate = true;

		if( strtmp.Find( '/' ) < 0 )
		{
			bValidDate = false;
		}
		else
		{
			int iLeft, iRight;
			CString substr;
			iLeft =  strtmp.Find( '/' );
			substr = strtmp.Left( iLeft );
			iDay = _ttoi( ( LPCTSTR ) substr );
			iRight = strtmp.Find( '/', iLeft + 1 );
			substr = strtmp.Mid( iLeft + 1, iRight - iLeft - 1 );
			iMonth = _ttoi( ( LPCTSTR ) substr );
			substr = strtmp.Mid( iRight + 1, 4 );
			iYear = _ttoi( ( LPCTSTR ) substr );
			int iPos = strtmp.Find( '-', iRight + 1 );

			if( iPos != -1 )	//hour present
			{
				iLeft = iPos + 1;
				iRight = strtmp.Find( ':', iLeft );
				substr = strtmp.Mid( iLeft, iRight - iLeft );
				iHour = _ttoi( ( LPCTSTR ) substr );
				iLeft = iRight + 1;
				iRight = strtmp.Find( ':', iLeft );
				substr = strtmp.Mid( iLeft, iRight - iLeft );
				iMin = _ttoi( ( LPCTSTR ) substr );
				iLeft = iRight + 1;
				substr = strtmp.Mid( iLeft, 2 );
				iSec = _ttoi( ( LPCTSTR ) substr );
			}
		}

		if( !( iYear >= 1970 && iYear <= 2038 ) )
		{
			bValidDate = false;
		}

		if( !( iMonth > 0 && iMonth <= 12 ) )
		{
			bValidDate = false;
		}

		if( !( iDay > 0 && iDay <= 31 ) )
		{
			bValidDate = false;
		}

		if( !( iHour >= 0 && iHour <= 23 ) )
		{
			bValidDate = false;
		}

		if( !( iMin >= 0 && iMin <= 59 ) )
		{
			bValidDate = false;
		}

		if( !( iSec >= 0 && iSec <= 59 ) )
		{
			bValidDate = false;
		}

		CTime tmpDate( iYear, iMonth, iDay, iHour, iMin, iSec );
		rec.m_Date = tmpDate;

		if( false == bValidDate )
		{
			#ifdef VERBOSE
			CString msg;
			msg.Format( _T("Syntax error in %s : line %s\n"), ptcFileName, str );
			AfxMessageBox( msg, MB_ICONSTOP );
			_MakeEmpty();
			#else
			//TRACE(_T("Syntax error in %s : line %s\n"),FileName,str);
			#endif
			fError = true;
			break;
		}

		if( iIndex < 1 )
		{
			m_List.AddTail( rec );
			continue;
		}

		str = str.Mid( iIndex + 1 );

		// Requiring Application Version -------------------------
		iIndex = str.Find( ';' );

		if( iIndex < 1 )
		{
			//TRACE(_T("In %s : line with %s terminates with the reqAppVersion\n"),FileName,strTargetFile);
			strtmp = str;
		}
		else
		{
			strtmp = str.Left( iIndex );
		}

		strtmp.TrimRight();

		// If the reqAppVersion is empty, the file must always be downloaded
		// and its reqAppVersion is thus set to 9.9.9999.
		if( true == strtmp.IsEmpty() )
		{
			strtmp = DEFREQAPPVERSION;
		}

		rec.m_dReqAppVersion = VersionStringToDouble( strtmp ); //atof(str.Left(index));

		if( rec.m_dReqAppVersion < 0 )
		{
			#ifdef VERBOSE
			CString msg;
			msg.Format( _T("Syntax error in %s : line %s\n"), ptcFileName, str );
			AfxMessageBox( msg, MB_ICONSTOP );
			_MakeEmpty();
			#else
			//TRACE(_T("Syntax error in %s : line %s\n"),FileName,str);
			#endif
			fError = true;
			break;
		}

		str = str.Mid( iIndex + 1 );

		iIndex = str.Find( ';' );

		if( iIndex < 1 )
		{
			//TRACE("In %s : line with %s terminates with the CRC32\n",FileName,strTargetFile);
			strtmp = str;
		}
		else
		{
			strtmp = str.Left( iIndex );
		}

		strtmp.TrimRight();
		unsigned int crc;
		swscanf_s( strtmp, L"%08X", &crc );
		rec.m_uiCRC = crc;
		m_List.AddTail( rec );
	}

	// Close the file.
	f.Close();

	return ( !fError );
}

bool CFileTable::GetFirst()
{
	m_pos = m_List.GetHeadPosition();
	return GetNext();
}

bool CFileTable::GetNext()
{
	bool end = ( m_pos == NULL ) ? true : false;
	m_ActivePos = m_pos;

	if( false == end )
	{
		m_Active = m_List.GetNext( m_pos );
	}

	return end;
}

CString CFileTable::GetDateAsString()
{
	CString str;
	str.Format( _T("%02u/%02u/%04u-%02u:%02u:%02u"), m_Active.m_Date.GetDay(), m_Active.m_Date.GetMonth(), m_Active.m_Date.GetYear(),
				m_Active.m_Date.GetHour(), m_Active.m_Date.GetMinute(), m_Active.m_Date.GetSecond() );
	return str;
}

bool CFileTable::FindSource( TCHAR *ptcTarget )
{
	POSITION pos = m_List.GetHeadPosition();

	if( !pos )
	{
		return false;
	}

	do
	{
		SRecord rec;
		ZeroMemory( &rec, sizeof( rec ) );
		rec = m_List.GetNext( pos );

		if( 0 == _tcsicmp( rec.m_tcSource, ptcTarget ) )
		{
			m_Active = rec;
			return true;
		}
	}
	while( pos != NULL );

	return false;
}

bool CFileTable::FindTarget( TCHAR *ptcTarget )
{
	POSITION pos = m_List.GetHeadPosition();

	if( NULL == pos )
	{
		return false;
	}
	
	SRecord rec;
	
	do
	{
		rec = m_List.GetNext( pos );
		
		if( 0 == _tcsicmp( rec.m_tcTarget, ptcTarget ) )
		{
			m_Active = rec;
			return true;
		}
	}
	while( NULL != pos );

	return false;
}

bool CFileTable::FindTargetWoPath( TCHAR *ptcTarget )
{
	POSITION pos = m_List.GetHeadPosition();
	
	if( NULL == pos )
	{
		return false;
	}
	
	SRecord rec;
	CString strTarget = ptcTarget;
	strTarget.MakeLower();

	do
	{
		POSITION PositionSaved = pos;
		rec = m_List.GetNext(pos);
		CString str = rec.m_tcTarget;
		str.MakeLower();

		if( str.Find( strTarget ) >= 0 )
		{
			m_Active = rec;
			SetActivePosition( PositionSaved );
			return true;
		}
	}
	while( NULL != pos );

	return false;
}

void CFileTable::Add( TCHAR *ptcSource, TCHAR *ptcTarget, double dVersion, unsigned long ulSize, double dMinAppVersion,
					  double dReqAppVersion, CTime date, unsigned int uiCRC )
{
	ASSERT( ptcSource && *ptcSource && ptcTarget && *ptcTarget );
	SRecord rec;
	_tcscpy_s( rec.m_tcSource, SIZEOFINTCHAR( rec.m_tcSource ), ptcSource );
	_tcscpy_s( rec.m_tcTarget, SIZEOFINTCHAR( rec.m_tcTarget ), ptcTarget );
	rec.m_dVersion = dVersion;
	rec.m_ulSize = ulSize;
	rec.m_dMinAppVersion = dMinAppVersion;
	rec.m_dReqAppVersion = dReqAppVersion;
	rec.m_Date = date;
	rec.m_uiCRC = uiCRC;
	m_List.AddTail( rec );
}

// Transform a 3 field version string (X.Y.Z) into a double.
// !!! The first field (X in the above line example) is not limited.
// !!! The 2,3,4 fields (Y, Z and x in the above line example)
// !!! can be made of only two digits (limited to 99).
double CFileTable::VersionStringToDouble( CString str )
{
	if( true == str.IsEmpty() )
	{
		return 0.0;
	}

	double dVal = 0.0;
	int iStrt = 0;
	double dMul = 100.0;
	CString str1;
	int iField = 0;

	while( iStrt >= 0 )
	{
		iField++;
		iStrt = str.Find( '.', iStrt );

		if( iStrt > 0 )
		{
			str1 = str.Left( iStrt );
		}
		else
		{
			str1 = str;
		}

		str = str.Right( str.GetLength() - ( iStrt + 1 ) );
		dVal += _tcstod( ( str1 ), '\0' ) * dMul ;
		dMul /= 100;
	}

	// Verify field numbers.
	if( iField < 2 || iField > 4 )
	{
		#ifdef VERBOSE
		AfxMessageBox( L"Version contains an invalid number of field, please check!", MB_ICONSTOP );
		#endif
		return 0.0;
	}

	return dVal;
}

// Transform a double into a 3 or 4 field version string (X.Y.Z.x).
// !!! The first field (X in the above line example) is not limited.
// !!! The 2,3,4 fields (Y, Z and x in the above line example)
// !!! can be made of only two digits (limited to 99).
CString CFileTable::VersionDoubleToString( double dVal, int iNbrField )
{
	if( dVal == 9999.99 || iNbrField < 1 || iNbrField > 4 )
	{
		return L"-";
	}

	CString str;
	str.Format( _T("%.4f"), dVal );

	str.Insert( str.GetLength() - 2, _T(".") );

	if( dVal > 99.9 )
	{
		int iDecimal = str.Find( _T(".") );
		str.Insert( iDecimal - 2, _T(".") );
	}
	else
	{
		str.Insert( 0, _T("0.") );
	}

	CString strResult;

	int curPos = 0;
	int iDigit = -1;
	int iFieldCount = 0;
	CString strToken = str.Tokenize( _T("."), curPos );
	
	while( _T("") != strToken )
	{
		iDigit = _ttoi( strToken );
		TCHAR tcBuffer[10];
		strResult += CString( _itow( iDigit, tcBuffer, 10 ) );
		iFieldCount++;

		strToken = _T("");

		if( iFieldCount < iNbrField )
		{
			strToken = str.Tokenize( _T("."), curPos );

			if( _T("") != strToken )
			{
				strResult += _T(".");
			}
		}
	}

	return strResult;
}

bool CFileTable::CompareVersionFileTable( CFileTable *pOldFileTable, CFileTable *pNewFileTable, double dCurrentAppVersion )
{
	// Variable.
	bool bminAppVersion = true;

	// Do a loop on the current list.
	for( bool fEndReached = GetFirst(); false == fEndReached; fEndReached = GetNext() )
	{
		if( true == pOldFileTable->FindTarget( (TCHAR*)GetTarget() ) ) 
		{
			// *** The file does exist on the old FileTable.

			// Check if minAppVersion are compatible.
			if( GetminAppVersion() > pOldFileTable->GetminAppVersion() )
			{
				bminAppVersion = false;
				break;
			}

			// Check the version number and CRC.
			if( GetVersion() > pOldFileTable->GetVersion() || GetCRC32() != pOldFileTable->GetCRC32() )
			{
				//TRACE (_T("\n File added (version does not match or CRC doesn't match): %s"),GetSource());
				pNewFileTable->Add( (TCHAR*)GetSource(), (TCHAR*)GetTarget(), GetVersion(), GetSize(), GetminAppVersion(), GetreqAppVersion(), GetDate(), GetCRC32() );
			}
		}
		else if( GetreqAppVersion() > dCurrentAppVersion )	// Check if reqAppVersion is bigger than CurrAppVersion
		{
			// *** The file does NOT exist on the old FileTable

			TRACE (_T("\n File added (file doesn't exist): %s"),GetSource());
			pNewFileTable->Add( (TCHAR*)GetSource(), (TCHAR*)GetTarget(), GetVersion(), GetSize(), GetminAppVersion(), GetreqAppVersion(), GetDate(), GetCRC32() );
		}
	}

	return bminAppVersion;
}

void CFileTable::OrderList()
{
	std::map<std::wstring, SRecord > SortedList;
	std::map<std::wstring, SRecord >::iterator It;

	// Do a loop on the current list.
	for( bool fEndReached = GetFirst(); false == fEndReached; fEndReached = GetNext() )
	{
		SortedList[( LPCTSTR )GetSource()] = m_Active;
	}

	m_List.RemoveAll();

	for( It = SortedList.begin(); It != SortedList.end(); ++It )
	{
		SRecord Rec = It->second;

		// Re-insert into the main list.
		m_List.AddTail( Rec );
	}
}

bool CFileTable::_FindTarget( TCHAR *ptcTarget )
{
	POSITION pos = m_List.GetHeadPosition();

	if( !pos )
	{
		return false;
	}

	SRecord rec;

	do
	{
		rec = m_List.GetNext( pos );

		if( 0 == _tcsicmp( rec.m_tcTarget, ptcTarget ) )
		{
			m_Active = rec;
			return true;
		}
	}
	while( pos != NULL );

	return false;
}

bool CFileTable::_ReadTextLine( CFile &f, CString &str )
{
	TCHAR tcBuffer[1024];
	TCHAR tcSrcBuf[1024];
	TCHAR *ptcBuffer = tcBuffer;
	TCHAR *ptcSrcBuf = tcSrcBuf;
	int iCount = 0;
	int iLength = 0;
	ZeroMemory( tcBuffer, sizeof( tcBuffer ) );
	ZeroMemory( tcSrcBuf, sizeof( tcSrcBuf ) );
	bool fCmd = false;

	// Read line up to '\r\n'.
	while( iLength = f.Read( ptcSrcBuf, sizeof( TCHAR ) ) )
	{
		// Skip unicode file marker.
		if( 0xfeff == tcSrcBuf[0] )
		{
			continue;
		}

		if( '\0' == *ptcSrcBuf )
		{
			break;
		}

		if( '$' == *ptcSrcBuf )
		{
			fCmd = true;
		}

		if( '\n' == *ptcSrcBuf && '\r' == *( ptcSrcBuf - 1 ) )
		{
			*( ptcSrcBuf - 1 ) = '\0';
			break;
		}

		++ptcSrcBuf;
	}

	// End of file reached.
	if( 0 == iLength )
	{
		return false;
	}

	*ptcSrcBuf = '\0';
	ptcSrcBuf = tcSrcBuf;

	// Remove space and '\t', stop when beginning of comment reached.
	for( ; '\0' != *ptcSrcBuf; ++ptcSrcBuf )
	{
		*ptcBuffer = *ptcSrcBuf;

		// skip tabulation
		if( '\t' == *ptcBuffer )
		{
			continue;
		}

		// stop string when comment begin
		if( 0 != iCount )
		{
			if( '/' == *ptcBuffer && '/' == *( ptcBuffer - 1 ) )
			{

				*ptcBuffer = '\0';
				*( ptcBuffer - 1 ) = '\0';
				break;
			}
		}

		ptcBuffer++;
		iCount++;
	}

	str = tcBuffer;
	return true;
}
