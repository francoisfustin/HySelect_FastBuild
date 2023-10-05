#pragma once

#include <vector>
#include <string>
#include <codecvt>

class Excel_Tools
{
public:
	// Constructors
	Excel_Tools() {}

	// Files Tools
	static void CreateTmpDirectory( TCHAR *path, size_t buffSize );
	static BOOL IsDirectory( const TCHAR *path );
	static void DeleteDirectory( const TCHAR *path, bool deleteSelf = true );
	static void GetFileList( const TCHAR *path, std::vector<std::wstring> &fileList );
	static int IsNotRelativePath(WIN32_FIND_DATA &data);
	static void LoadFile( const std::wstring &wstr, void **buf, int *size_read );

	// String Tools
	static std::wstring s2ws(const std::string &str);
	static std::string ws2s( const std::wstring &wstr );
	static std::string GetRef( int row, int col );

	static void EncodeHtml( std::string &data );

	// Not in the open xml specification but specific to Microsoft Excel.
	// Excel doesn't accept \, /, *; [, ]; : and ? characters for the sheet name.
	static void ClearNameForSheet( std::string &data );
};