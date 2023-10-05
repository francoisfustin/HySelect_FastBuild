#include "stdafx.h"
#include "Excel_Tools.h"
#include <wchar.h>


void Excel_Tools::DeleteDirectory( const TCHAR *path, bool deleteSelf /*= true*/ )
{
	// avoid to erase to short path like "C:\"
	if( wcslen( path ) < 8  || !IsDirectory( path ) )
	{
		return;
	}

	WIN32_FIND_DATA data;
	std::wstring tmppath( path );
	tmppath += _T("\\*");
	std::wstring name;
	HANDLE hFile = FindFirstFile( tmppath.c_str(), &data );

	if( hFile == INVALID_HANDLE_VALUE )
	{
		return;
	}

	do
	{
		if( IsNotRelativePath(data) )
		{
			name = path;
			name += _T("\\");
			name += data.cFileName;

			if( data.dwFileAttributes & FILE_ATTRIBUTE_DIRECTORY ) // Directory
			{
				DeleteDirectory( name.c_str() );
			}
			else // File
			{
				DeleteFile( name.c_str() );
			}
		}
	}
	while( FindNextFile( hFile, &data ) != 0 || GetLastError() != ERROR_NO_MORE_FILES );

	if( true == deleteSelf )
	{
		RemoveDirectory( path );
	}

	FindClose( hFile );
}

BOOL Excel_Tools::IsDirectory( const TCHAR *path )
{
	return PathIsDirectory( path );
}

void Excel_Tools::CreateTmpDirectory( TCHAR *path, size_t buffSize )
{
	DWORD pathSize = GetTempPath( buffSize, path );

	if( 0 == pathSize )
	{
		path[0] = 0;
	}

	_tcscat( path, _T("HYSelect_XML") );
	CreateDirectory( path, NULL );
}

void Excel_Tools::GetFileList( const TCHAR *path, std::vector<std::wstring> &fileList )
{
	// avoid to erase to short path like "C:\"
	if( !IsDirectory( path ) )
	{
		return;
	}

	WIN32_FIND_DATA data;
	std::wstring tmppath( path );
	tmppath += _T("\\*");
	std::wstring name;
	HANDLE hFile = FindFirstFile( tmppath.c_str(), &data );

	if( hFile == INVALID_HANDLE_VALUE )
	{
		return;
	}

	do
	{
		if (IsNotRelativePath(data))
		{
			name = path;
			name += _T("\\");
			name += data.cFileName;

			if( data.dwFileAttributes & FILE_ATTRIBUTE_DIRECTORY ) // Directory
			{
				std::vector<std::wstring> newList;
				newList.clear();
				GetFileList( name.c_str(), newList );

				for( unsigned int i = 0; i < newList.size(); i++ )
				{
					TCHAR newName[MAX_PATH + 1] = { 0 };
					wcscat( newName, data.cFileName );
					wcscat( newName, _T("\\") );
					wcscat( newName, newList[i].c_str() );
					fileList.push_back( newName );
				}
			}
			else // File
			{
				fileList.insert( fileList.begin(), data.cFileName );
			}
		}
	}
	while( FindNextFile( hFile, &data ) != 0 || GetLastError() != ERROR_NO_MORE_FILES );

	FindClose( hFile );
}

std::wstring Excel_Tools::s2ws( const std::string &str )
{
	typedef std::codecvt_utf8<wchar_t> convert_typeX;
	std::wstring_convert<convert_typeX, wchar_t> converterX;
	return converterX.from_bytes( str );
}

std::string Excel_Tools::ws2s( const std::wstring &wstr )
{
	typedef std::codecvt_utf8<wchar_t> convert_typeX;
	std::wstring_convert<convert_typeX, wchar_t> converterX;
	return converterX.to_bytes( wstr );
}

void Excel_Tools::LoadFile( const std::wstring &wstr, void **buf, int *size_read )
{
	if( *buf != 0 )
	{
		free( *buf );
	}

	FILE *fp = _wfopen( wstr.c_str(), _T("rb") );

	if( NULL == fp )
	{
		return;
	}

	fseek( fp, 0, SEEK_END );
	*size_read = ftell( fp );
	fseek( fp, 0, SEEK_SET );

	*buf = ( void * )malloc( *size_read );
	fread( *buf, 1, *size_read, fp );
	fclose( fp );
}

std::string Excel_Tools::GetRef( int row, int col )
{
	std::string retVal;
	char letter[] = "ABCDEFGHIJKLMNOPQRSTUVWXYZ";

	if( col > 26 )
	{
		retVal += letter[( ( col - 1 ) / 26 ) - 1];
	}

	retVal += letter[( col - 1 ) % 26];
	char number[10] = { 0 };
	retVal += _itoa( row, number, 10 );
	return retVal;
}

int Excel_Tools::IsNotRelativePath(WIN32_FIND_DATA &data)
{
	return wcscmp(data.cFileName, _T(".")) && wcscmp(data.cFileName, _T(".."));
}

void Excel_Tools::EncodeHtml( std::string& data )
{
	std::string buffer;
	buffer.reserve( data.size() );

	for( size_t pos = 0; pos != data.size(); ++pos )
	{
		switch( data[pos] )
		{
			case '&':
				buffer.append( "&amp;" );
				break;

			case '\"':
				buffer.append( "&quot;" );
				break;
			
			case '\'':
				buffer.append( "&apos;" );
				break;
			
			case '<':
				buffer.append( "&lt;" );
				break;
			
			case '>':
				buffer.append( "&gt;" );
				break;
			
			default:
				buffer.append( &data[pos], 1 );
				break;
		}
	}

	data.swap( buffer );
}

void Excel_Tools::ClearNameForSheet( std::string &data )
{
	std::vector<int> vecAmpersandPos;
	int i = 0;

	do 
	{
		i = data.find( "&", i );

		if( i != data.npos )
		{
			i++;
			vecAmpersandPos.push_back( i );
		}
	}while( i != data.npos );

	if( vecAmpersandPos.size() > 0 )
	{
		int iShift = 0;
		for( auto &iter : vecAmpersandPos )
		{
			data = data.insert( iter + iShift, "amp;" );
			iShift += 4;
		}
	}

	for( size_t pos = 0; pos != data.size(); ++pos )
	{
		switch( data[pos] )
		{
			case '\\':
			case '/':
			case '*':
			case '[':
			case ']':
			case ':':
			case '?':
				data[pos] = ' ';
				break;

			default:
				break;
		}
	}
}
