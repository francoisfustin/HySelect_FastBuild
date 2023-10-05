#include "stdafx.h"
#include "Tender.h"
#include "DataBase.h"
#include "TASelect.h"
#include "DataBObj.h"
#include "CsvParser.h"


bool predicate_digit( TCHAR c )
{
	return !_istdigit( c );
}

CTender::CTender()
{
	m_zipPath = NULL;
	m_iIndexNumIndex = -1;
	m_vArtNumIndex.clear();
}

CTender::~CTender()
{
	delete[] m_zipPath;
	m_zip.Close();
	m_vArtNumIndex.clear();
}

// *** PUBLIC ***

void CTender::SetZipPath( const TCHAR *zipPath )
{
	delete[] m_zipPath;
	m_zipPath = NULL;
	m_zip.Close();
	m_iIndexNumIndex = -1;
	m_vArtNumIndex.clear();

	if( NULL == zipPath )
	{
		return;
	}

	size_t pathLen = wcslen( zipPath );
	m_zipPath = new TCHAR[pathLen + 1];
	wcscpy_s( m_zipPath, pathLen + 1, zipPath );
}

int CTender::GetTenderTxt( const _string &artRef, _string &tenderTxt, bool checkIndex /*= true */ )
{
	tenderTxt.clear();

	if( false == OpenZip() )
	{
		return -1;
	}

	// use index.csv to concatenate many rtf
	if( checkIndex && -1 != m_iIndexNumIndex )
	{
		int retVal = GetTenderTxtFromIndex( artRef, tenderTxt );

		if( -1 != retVal )
		{
			return retVal;
		}
	}

	_string ref = artRef;
	// remove non numeric chars
	ref.erase( std::remove_if( ref.begin(), ref.end(), predicate_digit ), ref.end() );

	if( ref.empty() )
	{
		return -1;
	}

	for( unsigned int i = 0; i < m_vArtNumIndex.size(); ++i )
	{
		if( ref == m_vArtNumIndex[i] )
		{
			void *buffer = NULL;
			GetFileBufferFromIndex( i, &buffer );

			if( NULL == buffer )
			{
				return -1;
			}

			char *richText = ( char * )buffer;

			if( char( 0xff ) == richText[0] && char( 0xfe ) == richText[1] )
			{
				TCHAR *unicodeText = ( TCHAR * )buffer + 1;
				tenderTxt = _string( unicodeText, unicodeText + _tcslen( unicodeText ) );
			}
			else
			{
				tenderTxt = _string( richText, richText + strlen( richText ) );
			}

			tenderTxt = GetTextFromRTF( tenderTxt ); // remove rtf formatting

			// trim trailing spaces
			size_t endpos = tenderTxt.find_last_not_of( _T(" \t\r\n") );

			if( string::npos != endpos )
			{
				tenderTxt = tenderTxt.substr( 0, endpos + 1 );
			}

			free( buffer );
			return i;
		}
	}

	return -1;
}

void CTender::GetFileBufferFromIndex( unsigned int i, void **buffer )
{
	size_t uSize = m_zip[i]->m_uUncomprSize;
	m_zip.OpenFile( i );
	*buffer = malloc( uSize + sizeof( TCHAR ) );
	memset( *buffer, 0, uSize + sizeof( TCHAR ) );

	size_t uReadSize = m_zip.ReadFile( *buffer, uSize );

	if( uReadSize != uSize )
	{
		CZipException::Throw();
	}

	m_zip.CloseFile();
}

int CTender::GetTenderTxtFromIndex( const _string &artRef, _string &tenderTxt )
{
	int retVal = -1;
	tenderTxt.clear();

	_string ref = artRef;
	// remove non numeric chars
	ref.erase( std::remove_if( ref.begin(), ref.end(), predicate_digit ), ref.end() );

	if( ref.empty() )
	{
		return retVal;
	}

	if( false == OpenZip() )
	{
		return retVal;
	}

	void *buffer = NULL;
	GetFileBufferFromIndex( m_iIndexNumIndex, &buffer );

	if( NULL == buffer )
	{
		return retVal;
	}

	TCHAR *unicodeText = ( TCHAR * )buffer;

	CsvParser *csvparser = CsvParser_new_from_string( unicodeText, ";", 0 );
	CsvRow *row;
	bool articleFound = false;

	while( ( row = CsvParser_getRow( csvparser ) ) )
	{
		TCHAR **rowFields = CsvParser_getFields( row );

		_string artRefIndex = _string( rowFields[0], rowFields[0] + _tcslen( rowFields[0] ) );
		// remove non numeric chars
		artRefIndex.erase( std::remove_if( artRefIndex.begin(), artRefIndex.end(), predicate_digit ), artRefIndex.end() );

		if( ref == artRefIndex )
		{
			articleFound = true;

			for( int i = 1; i < CsvParser_getNumFields( row ); ++i )
			{
				retVal++;
				_string tenderPart;
				GetTenderTxtFromPath( _string( rowFields[i], rowFields[i] + _tcslen( rowFields[i] ) ), tenderPart );
				tenderTxt += tenderPart;
				tenderTxt += L"\r\n";
			}
		}

		CsvParser_destroy_row( row );

		if( articleFound )
		{
			break;
		}
	}

	CsvParser_destroy( csvparser );

	// trim trailing spaces
	size_t endpos = tenderTxt.find_last_not_of( _T(" \t\r\n") );

	if( string::npos != endpos )
	{
		tenderTxt = tenderTxt.substr( 0, endpos + 1 );
	}

	free( buffer );

	return retVal;
}

int CTender::GetTenderTxtFromPath( const _string &path, _string &tenderTxt )
{
	tenderTxt.clear();

	for( ZIP_INDEX_TYPE index = 0; index < m_zip.GetCount(); ++index )
	{
		CZipFileHeader *fileHeader = m_zip.GetFileInfo( index );
		CZipString fileName = fileHeader->GetFileName();
		_string zipPath = fileName;

		if( zipPath == path )
		{
			void *buffer = NULL;
			GetFileBufferFromIndex( index, &buffer );

			if( NULL == buffer )
			{
				return -1;
			}

			char *content = ( char * )buffer;

			if (char(0xff) == content[0] && char(0xfe) == content[1])
			{
				TCHAR *unicodeText = (TCHAR *)buffer + 1;
				tenderTxt = _string(unicodeText, unicodeText + _tcslen(unicodeText));
			}
			else
			{
				tenderTxt = _string(content, content + strlen(content));
			}

			tenderTxt = GetTextFromRTF( tenderTxt );

			// trim trailing spaces
			size_t endpos = tenderTxt.find_last_not_of( _T(" \t\r\n") );

			if( string::npos != endpos )
			{
				tenderTxt = tenderTxt.substr( 0, endpos + 1 );
			}

			free( buffer );
			break;
		}

	}

	return 0;
}

int CTender::GetTenderID( const _string &artRef )
{

	_string ref = artRef;
	// remove non numeric chars
	ref.erase( std::remove_if( ref.begin(), ref.end(), predicate_digit ), ref.end() );

	if( ref.empty() )
	{
		return -1;
	}

	if( m_vArtNumIndex.empty() )
	{
		OpenZip();
	}

	for( unsigned int i = 0; i < m_vArtNumIndex.size(); ++i )
	{
		if( ref == m_vArtNumIndex[i] )
		{
			return i;
		}
	}

	return -1;
}

void CTender::RemoveUnusedTender()
{
	std::vector<_string> artList = GetArtNumVector();
	std::vector<int> filesToKeep;
	CZipIndexesArray filesToRemove;
	OpenZip();

	for( unsigned int i = 0; i < artList.size(); ++i )
	{
		int index = GetTenderID( artList[i] );

		if( index >= 0 )
		{
			filesToKeep.push_back( index );
		}
	}

	// sort and remove doublon;
	std::sort( filesToKeep.begin(), filesToKeep.end() );
	auto last = std::unique( filesToKeep.begin(), filesToKeep.end() );
	// change the physical size to fit the real size
	filesToKeep.erase( last, filesToKeep.end() );

	for( unsigned int j = 0; j < m_vArtNumIndex.size(); ++j )
	{
		if( ( filesToKeep.empty() || j != filesToKeep[0] ) && false == m_vArtNumIndex[j].empty() )
		{
			filesToRemove.Add( j );
		}

		if( false == filesToKeep.empty() && j == filesToKeep[0] )
		{
			filesToKeep.erase( filesToKeep.begin() );
		}
	}

	m_zip.Close();
	m_zip.Open( m_zipPath ); // RW mode
	m_zip.RemoveFiles( filesToRemove );
	m_zip.Close();
}

std::vector<_string> CTender::GetArtNumTenderVector()
{
	std::vector<_string> retVal;

	if( NULL == m_zipPath )
	{
		return retVal;
	}

	try
	{
		m_zip.Open( m_zipPath, CZipArchive::zipOpenReadOnly );

		for( ZIP_INDEX_TYPE index = 0; index < m_zip.GetCount(); ++index )
		{
			CZipFileHeader *fileHeader = m_zip.GetFileInfo( index );
			CZipString fileName = fileHeader->GetFileName();
			retVal.push_back( fileName );

			wchar_t drive[MAX_PATH];
			wchar_t dir[MAX_PATH];
			wchar_t fname[MAX_PATH];
			wchar_t ext[MAX_PATH];

			_wsplitpath_s( retVal[index].c_str(), drive, dir, fname, ext );

			if( 0 == _wcsicmp( L"index", fname ) && 0 == _wcsicmp( L".csv", ext ) )
			{
				m_iIndexNumIndex = index;
			}

			retVal[index] = fname;
			// remove non numeric chars
			retVal[index].erase( std::remove_if( retVal[index].begin(), retVal[index].end(), predicate_digit ), retVal[index].end() );
		}
	}
	catch( ... )
	{
		delete[] m_zipPath;
		m_zipPath = NULL;
	}

	return retVal;
}

std::vector<_string> CTender::GetArtNumVector()
{
	std::vector<_string> retVal;
	CDataList *pDL = TASApp.GetpTADB()->GetpDataList();

	for( IDPTR IDPtr = pDL->GetFirst(); IDPtr.MP != NULL; IDPtr = pDL->GetNext() )
	{
		CDB_Product *product = dynamic_cast<CDB_Product *>( IDPtr.MP );

		if( product )
		{
			const TCHAR *artNum = product->GetBodyArtNum();
			size_t artNumLen = wcslen( artNum );
			retVal.push_back( _string( artNum ) );
		}
	}

	// sort and remove doublon;
	std::sort( retVal.begin(), retVal.end() );
	auto last = std::unique( retVal.begin(), retVal.end() );
	// change the physical size to fit the real size
	retVal.erase( last, retVal.end() );

	return retVal;
}

_string CTender::GetTextFromRTF( const _string &rtf ) const
{
	_string strCopy;
	TCHAR ch = 0;
	BOOL bBrace = FALSE;
	BOOL bSlash = FALSE;
	BOOL bAnsi = FALSE;
	int nLength = rtf.length();

	if( nLength < 1 )
	{
		return strCopy;
	}

	BOOL bFirstLetter = FALSE;

	for( int i = 0; i < nLength; i++ )
	{
		ch = rtf[i];

		if( ch == _T( '\\' ) )
		{
			bSlash = TRUE;

			if( rtf[i + 1] == _T( 'a' ) &&
				rtf[i + 2] == _T( 'n' ) &&
				rtf[i + 3] == _T( 's' ) &&
				rtf[i + 4] == _T( 'i' ) )
			{
				bAnsi = true;
			}

			if( bAnsi && rtf[i + 1] == 0x27 /* = apostrophe */ )
			{
				TCHAR strToConvert[3];
				memset( strToConvert, 0, 3 * sizeof( TCHAR ) );
				memcpy( strToConvert, &rtf[i + 2], 2 * sizeof( TCHAR ) );
				long charId = wcstol( strToConvert, NULL, 16 );
				strCopy += static_cast<wchar_t>( charId );
				i += 3; // jump after special character (+4 with increment in loop)
				bSlash = FALSE; // end of tag
			}

			continue;
		}
		else if( ch == _T( ' ' ) || ch == _T( '\r' ) || ch == _T( '\n' ) )
		{
			bSlash = FALSE;

			//Let it fall through so the space is added
			//if we have found first letter
			if( !bFirstLetter )
			{
				continue;
			}
		}
		else if( ch == _T( '{' ) )
		{
			bBrace = TRUE;
			bSlash = FALSE;
			continue;
		}
		else if( ch == _T( '}' ) )
		{
			bSlash = FALSE;
			bBrace = FALSE;
			continue;
		}

		if( !bSlash && !bBrace )
		{
			if( !bFirstLetter )
			{
				bFirstLetter = TRUE;
			}

			strCopy += ch;
			continue;
		}

	}

	return strCopy;
}

// *** PRIVATE ***

bool CTender::OpenZip()
{
	if( false == m_zip.IsClosed() )
	{
		return true;
	}

	if( NULL == m_zipPath )
	{
		return false;
	}

	try
	{
		m_iIndexNumIndex = -1;
		m_vArtNumIndex.clear();
		m_vArtNumIndex = GetArtNumTenderVector();
		return true;
	}
	catch( ... )
	{
		delete[] m_zipPath;
		m_zipPath = NULL;
		return false;
	}
}

bool CTender::IsTenderNeeded()
{
	return TASApp.IsTenderText();
}

