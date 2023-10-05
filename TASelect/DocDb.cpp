#include "stdafx.h"
#include "DocDb.h"
#include "CsvParser.h"
#include <algorithm>


bool predicate_articleDigit( TCHAR c )
{
	return ( !_istdigit( c ) && L'|' != c );
}

CDocDb::CDocDb()
{
	m_csvPath = NULL;
	m_csvPathEn = NULL;
}


CDocDb::~CDocDb()
{
	delete[] m_csvPath;
	delete[] m_csvPathEn;
}

void CDocDb::SetCsvPath( const TCHAR *csvPath )
{
	delete[] m_csvPath;
	m_csvPath = NULL;

	if( NULL == csvPath )
	{
		return;
	}

	size_t pathLen = wcslen( csvPath );
	m_csvPath = new TCHAR[pathLen + 1];
	wcscpy_s( m_csvPath, pathLen + 1, csvPath );
	_LoadDocDb( m_csvPath );
}

void CDocDb::SetCsvPathEn( const TCHAR *csvPath )
{
	delete[] m_csvPathEn;
	m_csvPathEn = NULL;

	if( NULL == csvPath )
	{
		return;
	}

	size_t pathLen = wcslen( csvPath );
	m_csvPathEn = new TCHAR[pathLen + 1];
	wcscpy_s( m_csvPathEn, pathLen + 1, csvPath );
	_LoadDocDb( m_csvPathEn );
}

int CDocDb::_FoundLangIndex( _string lang )
{
	int retVal = -1;

	if( m_vNode.empty() )
	{
		return retVal;
	}

	CString llang = lang.c_str();

	for( unsigned int j = 0; j < m_vNode.at( 0 ).nodeText.size(); ++j )
	{
		CString lnode = m_vNode.at(0).nodeText.at(j).c_str();

		if (0 == llang.CompareNoCase(lnode))
		{
			return j;
		}
		else if ( m_vNode.at(0).nodeText.at(j) == L"en" )
		{
			retVal = j;
		}
	}

	return retVal;
}

_string CDocDb::GetPathFromArticleNumber( _string artNum, _string lang )
{

	_string retVal;
	int langIndex = _FoundLangIndex( lang );

	artNum.erase( std::remove_if( artNum.begin(), artNum.end(), predicate_articleDigit ), artNum.end() );

	if( m_vNode.empty() || -1 == langIndex )
	{
		return retVal;
	}

	for( unsigned int i = 0; i < m_vNode.size(); ++i )
	{
		if( m_vNode.at( i ).nodeName.find( artNum ) != std::string::npos )
		{
			retVal = m_vNode.at( i ).nodeText.at( langIndex );
			break;
		}
	}

	return retVal;
}

_string CDocDb::GetPathFromHTreeItem( HTREEITEM item, _string lang )
{
	_string retVal;
	int langIndex = _FoundLangIndex( lang );

	if( m_vNode.empty() || -1 == langIndex )
	{
		return retVal;
	}

	for( unsigned int i = 0; i < m_vNode.size(); ++i )
	{
		if( m_vNode.at( i ).nodeTreeItem == item )
		{
			retVal = m_vNode.at( i ).nodeText.at( langIndex );
			break;
		}
	}

	return retVal;
}

HTREEITEM CDocDb::GetHTreeItemFromArticleNumber( _string artNum )
{

	HTREEITEM retVal = NULL;

	artNum.erase( std::remove_if( artNum.begin(), artNum.end(), predicate_articleDigit ), artNum.end() );

	if( artNum.empty() )
	{
		return retVal;
	}

	for( unsigned int i = 0; i < m_vNode.size(); ++i )
	{
		if( m_vNode.at( i ).nodeName.find( artNum ) != std::string::npos )
		{
			retVal = m_vNode.at( i ).nodeTreeItem;
			break;
		}
	}

	return retVal;
}

int CDocDb::GetHTreeItemIndexFromArticleNumber( _string artNum )
{

	int retVal = -1;

	artNum.erase( std::remove_if( artNum.begin(), artNum.end(), predicate_articleDigit ), artNum.end() );

	if( artNum.empty() )
	{
		return retVal;
	}

	for( unsigned int i = 0; i < m_vNode.size(); ++i )
	{
		if( m_vNode.at( i ).nodeName.find( artNum ) != std::string::npos )
		{
			retVal = i;
			break;
		}
	}

	return retVal;
}

size_t CDocDb::Size()
{
	return m_vNode.size();
}

_string CDocDb::GetLangDoc( _string lang )
{
	if( m_vNode.empty() )
	{
		return L"";
	}

	return m_vNode.at( 0 ).nodeText.at( _FoundLangIndex( lang ) );
}

const _string &CDocDb::GetNameAt( int pos )
{
	if( m_vNode.empty() )
	{
		return m_strEmpty;
	}

	return m_vNode.at( pos ).nodeName;
}

_string &CDocDb::GetIdAt( int pos )
{
	return m_vNode.at( pos ).nodeId;
}

const _string &CDocDb::GetTextAt( int pos, _string lang )
{
	if( m_vNode.empty() )
	{
		return m_strEmpty;
	}

	return m_vNode.at( pos ).nodeText.at( _FoundLangIndex( lang ) );
}

HTREEITEM &CDocDb::GetTreeItemAt( int pos )
{
	return m_vNode.at( pos ).nodeTreeItem;
}

void CDocDb::_LoadDocDb( const TCHAR *csvPath )
{
	m_vNode.clear();

	if( NULL == csvPath )
	{
		return;
	}

	FILE *docDB = NULL;
	errno_t err = _wfopen_s( &docDB, csvPath, L"rb" );

	if( 0 != err )
	{
		return;
	}

	// find file size
	fseek( docDB, 0, SEEK_END );
	long docDblen = ftell( docDB );
	fseek( docDB, 0, SEEK_SET );

	// copy file in memory
	char *buffer = new char[docDblen + 1];
	memset( buffer, 0, ( docDblen + 1 ) * sizeof( char ) );
	size_t read = fread_s( buffer, docDblen + 1, 1, docDblen, docDB );
	ASSERT( read == docDblen );
	wchar_t *uBuffer = CodePageToUnicode( 65001, buffer );
	delete[]buffer;
	fclose( docDB );


	CsvParser *csvparser = CsvParser_new_from_string( uBuffer, ";", 0 );
	delete[] uBuffer;
	CsvRow *row;

	// parse docdb.csv to load all information
	while( ( row = CsvParser_getRow( csvparser ) ) )
	{
		TCHAR **rowFields = CsvParser_getFields( row );

		if( _T('\0') == *(rowFields[0]) || row->numOfFields_ < 3 )
		{
			CsvParser_destroy_row(row);
			continue;
		}

		treeNode tupple;
		tupple.nodeTreeItem = NULL;
		tupple.nodeId = _string( rowFields[0], rowFields[0] + _tcslen( rowFields[0] ) );

		_string strName = _string( rowFields[1], rowFields[1] + _tcslen( rowFields[1] ) );

		if( false == strName.empty() )
		{
			// HYS-1184: Determine if row if for pdf files.
			for( int i = 2; i < row->numOfFields_; ++i )
			{
				CString str = rowFields[i];

				if( false == str.IsEmpty() && -1 != str.MakeLower().Find( _T(".pdf") ) )
				{
					strName.erase( std::remove_if( strName.begin(), strName.end(), predicate_articleDigit ), strName.end() );
					break;
				}
			}
		}

		tupple.nodeName = strName;

		for( int i = 2; i < row->numOfFields_; ++i )
		{
			tupple.nodeText.push_back( _string( rowFields[i], rowFields[i] + _tcslen( rowFields[i] ) ) );
		}

		//tupple.nodeText = _string( rowFields[currLang], rowFields[currLang] + _tcslen( rowFields[currLang] ) );

		m_vNode.push_back( tupple );

		CsvParser_destroy_row( row );
	}

	CsvParser_destroy( csvparser );
}
