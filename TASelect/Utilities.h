//////////////////////////////////////////////////////////////////////////
//  Utilities.h : header file
//	Version: 1.2							Date: 14/04/08
//	CRankEx added
//////////////////////////////////////////////////////////////////////////

#pragma once


#include <map>
#include <cctype>    // pour tolower et toupper
#include "Global.h"
#include "WinSysUtil.h"

/////////////////////////////////////////////////////////////////////////////
// numeric utilities functions
/////////////////////////////////////////////////////////////////////////////
// Is Not a Number
template<typename T>bool IsNaN( T num )
{
	return num != num;
}

/////////////////////////////////////////////////////////////////////////////
// std::string utilities functions
/////////////////////////////////////////////////////////////////////////////
// TrimRight & TrimLeft
template<typename T>void StrTrimLeft( T &str )
{
	size_t found = str.find_first_not_of( ' ' );

	if( found && found != T::npos )
	{
		str.erase( 0, found );
	}
}

template<typename T>void StrTrimRight( T &str )
{
	size_t found = str.find_last_not_of( ' ' );

	if( ( found + 1 ) < str.size() && found != T::npos )
	{
		str.erase( found + 1 );
	}
}

// Remove all TCHAR c from a string.
template<typename T>int RemoveTChar( T *pStr, TCHAR c )
{
	int iCount = 0;
	size_t found;
	found = pStr->find_first_of( c );

	while( found != T::npos )
	{
		pStr->erase( pStr->begin() + found, pStr->begin() + found + 1 );
		found = pStr->find_first_of( c, found );
	}

	return iCount;
}

// Cpp type to string conversion
template<typename T>_string to_string( const T &Value )
{
	// Use an output stream.
#ifdef UNICODE
	std::wostringstream oss;
#else
	std::ostringstream oss;
#endif

	oss << Value;
	// return string from stream.
	return oss.str();
}

// Convert a string to a specified type
template<typename T>
bool from_string( const _string &Str, T &Value )
{
	// Build an input string with given string.
#ifdef UNICODE
	std::wistringstream iss( Str );
#else
	std::istringstream iss( Str );
#endif

	// Try to make conversion.
	iss >> Value;
	return ( true == iss.eof() && false == iss.fail() );
}

// Test string type
template<typename T>bool is_of_type( const _string &Str )
{
	// Build an input string with given string.
#ifdef UNICODE
	std::wistringstream iss( Str );
#else
	std::istringstream iss( Str );
#endif

	// Temporary object used for conversion.
	T tmp;

	// Try to make conversion and test if string is empty.
	return ( iss >> tmp ) && ( iss.eof() );
}

#ifndef TCHAR
	#ifdef UNICODE
		#define TCHAR wchar_t
	#else
		#define TCHAR char
	#endif
#endif

struct _tolower
{
	TCHAR operator()( TCHAR c ) const
	{
		return std::tolower( static_cast<TCHAR>( c ) );
	}
};

struct _toupper
{
	TCHAR operator()( TCHAR c ) const
	{
		return std::toupper( static_cast<TCHAR>( c ) );
	}
};


CString GetLocaleInfoSeparator();

/////////////////////////////////////////////////////////////////////////////
// Writes value with MaxDig significant digits and MinDec decimals.
/////////////////////////////////////////////////////////////////////////////

// !!! PAY ATTENTION. These functions use static variable to work on it an return a pointer on this static variable.
// !!! This is why you can't call these functions in the same line otherwise you will have only the last result.
// !!! Ex: str.Format( _T("%s blabla %s"), WriteDouble( dValue1, 2 ), WriteDouble( dValue2, 2) ); will not work.
// !!! Either you define before two CString variable and set them with the respective WriteDouble return value and
// !!! call: str.Format( _T("%s blabla %s"), str1, str2 );
// !!! Or you can do: str.Format( _T(%s blabla %s"), CString( WriteDouble( dValue1, 2) ), CString( WriteDouble( dValue2, 2 ) ) );

// HYS-1922: Add iMaxDec parameter. If it isn't equal to -1 we consider this value as the max number of digit int the decimal part.
// This value should be smaller than iMaxDig.

LPCTSTR WriteDouble( double dValue, int iMaxDig, int iMinDec = 0, bool bClean = false, int iMaxDec = -1 );

// same as WriteDouble, but use MaxDig and MinDec defined into UnitDesign_struct.
LPCTSTR WriteCUDouble( int iPhysicalType, double dSI, bool fFormattedWithUnit = false, int iMaxDig = -1, int iMinDec = -1, bool fClean = true,
					   int iUnitIndex = -1 );

LPCTSTR WriteCustomUnitDouble( int iPhysicalType, double dSI, int iUnitIndex );

/////////////////////////////////////////////////////////////////////////////
// Returns the double value.
/////////////////////////////////////////////////////////////////////////////
enum ReadDoubleReturn_enum
{
	RD_EMPTY,		// When the edit box is empty.
	RD_NOT_NUMBER,	// The edit box contains no number.
	RD_OK			// return a number.
};

ReadDoubleReturn_enum ReadDoubleFromStr( _string str, double *pdResult );
ReadDoubleReturn_enum ReadDouble( CString str, double *pdResult );
ReadDoubleReturn_enum ReadDouble( CEdit &EditBox, double *pdResult );
ReadDoubleReturn_enum ReadDouble( CRichEditCtrl &RichEditBox, double *pdResult );
ReadDoubleReturn_enum ReadDouble( CComboBox &ComboBox, double *pdResult );

// Return double value into Customer unit choice.
ReadDoubleReturn_enum ReadCUDoubleFromStr( int iPhysicalType, _string str, double *pdResult );
ReadDoubleReturn_enum ReadCUDouble( int iPhysicalType, CString str, double *pdResult );
ReadDoubleReturn_enum ReadCUDouble( int iPhysicalType, CEdit &EditBox, double *pdResult );
ReadDoubleReturn_enum ReadCUDouble( int iPhysicalType, CRichEditCtrl &RichEditBox, double *pdResult );
ReadDoubleReturn_enum ReadCUDouble( int iPhysicalType, CComboBox &ComboBox, double *pdResult );


ReadDoubleReturn_enum ReadLongFromStr( _string str, long *pdResult );
ReadDoubleReturn_enum ReadLong( CString str, long *pdResult );
ReadDoubleReturn_enum ReadLong( CEdit &EditBox, long *pdResult );
ReadDoubleReturn_enum ReadLong( CComboBox &ComboBox, long *pdResult );

/////////////////////////////////////////////////////////////////////////////
// Patch FormatString/2 because it doesn't support string longer than 256 bytes
/////////////////////////////////////////////////////////////////////////////
void FormatString( CString &rString, UINT nIDS, LPCTSTR lpsz1 );
void FormatString( CString &rString, CString str, LPCTSTR lpsz1 );
void FormatString( CString &rString, UINT nIDS, LPCTSTR lpsz1, LPCTSTR lpsz2 );
void FormatString( CString &rString, CString &str, LPCTSTR lpsz1, LPCTSTR lpsz2 );
void FormatString( CString &rString, UINT nIDS, LPCTSTR lpsz1, LPCTSTR lpsz2, LPCTSTR lpsz3 );
void FormatString( CString &rString, CString &str, LPCTSTR lpsz1, LPCTSTR lpsz2, LPCTSTR lpsz3 );
void FormatString( CString &rString, UINT nIDS, LPCTSTR lpsz1, LPCTSTR lpsz2, LPCTSTR lpsz3, LPCTSTR lpsz4 );
void FormatString( CString &rString, CString &str, LPCTSTR lpsz1, LPCTSTR lpsz2, LPCTSTR lpsz3, LPCTSTR lpsz4 );

CString ConvertVecStringToString( std::vector<CString> vecStrings, CString strPrefix = _T("- "), bool bReturnLine = true );

wchar_t *CodePageToUnicode(int codePage, const char *src);
char *UnicodeToCodePage(int codePage, const wchar_t *src);

// strcmp with a protection on empty string...
// Null pointer or empty string are always returned like the smaller.
int StringCompare( LPCTSTR pbuf1 = NULL, LPCTSTR pbuf2 = NULL );

// Convert a Byte to a HexAscii char (2 bytes).
void ByteToHexAscii( BYTE src, BYTE *pbDst );

// Return a byte corresponding to a HexAscii char.
BYTE HexAsciiToByte( BYTE *pbSrc );

// Base 26 conversion.
// Ff len != 0 return number of char asked, otherwise return needed char (max 20).
CString Base26Convert( ULONGLONG ullVal, UCHAR ucLen = 0 );

////////////////////////////////////////////////////////////////////////////////////////////////////
/// <summary>	Gets the time based uniq identifier. </summary>
///
/// <remarks>	Based on time and date compose an DB ID 9 chars
/// 			Alen, 30/09/2010. </remarks>
///
/// <returns>	The time based uniq identifier. </returns>
////////////////////////////////////////////////////////////////////////////////////////////////////
CString GetTimeBasedUniqID();

////////////////////////////////////////////////////////////////////////////////////////////////////
/// <summary>	Gets the project directory. </summary>
///
/// <remarks>	Alen, 30/09/2010. </remarks>
///
/// <returns>	The project directory. </returns>
////////////////////////////////////////////////////////////////////////////////////////////////////
CString GetProjectDirectory();

void WaitMillisec( unsigned short us );

////////////////////////////////////////////////////////////////////////////////////////////////////
/// <summary>	Removes the file described by filename. </summary>
///
/// <remarks>	USE CARREFULLY this function doesn't take into account read only attribute
/// 			Alen, 30/09/2010. </remarks>
///
/// <param name="filename">	Filename of the file. </param>
////////////////////////////////////////////////////////////////////////////////////////////////////
void RemoveFile( CString strFileName );

////////////////////////////////////////////////////////////////////////////////////////////////////
/// <summary>	File move copy. </summary>
///
/// <remarks>	Move or copy a file from a position to another.
/// 			The copied file attribute is always writable.
/// 			Alen, 30/09/2010. </remarks>
///
/// <param name="Src">				Source full path. </param>
/// <param name="Trg">				Target full path. </param>
/// <param name="bDeleteSource">	true to delete the source.
/// 								USE CARREFULLY this function doesn't take into account read only attribute
/// 								</param>
///
/// <returns>	true if it succeeds, false if it fails. </returns>
////////////////////////////////////////////////////////////////////////////////////////////////////
bool FileMoveCopy( CString strSource, CString strTarget, bool bDeleteSource );

////////////////////////////////////////////////////////////////////////////////////////////////////
/// <summary>	Deletes the files and folder. </summary>
///
/// <remarks>	USE CARREFULLY this function works recursively
/// 			and delete all that is matching the wildcard
/// 			if you use for instance *.* all files and folder included
/// 			in the path will be destroyed.
/// 			Alen, 30/09/2010. </remarks>
///
/// <param name="Path">	Full pathname of the file. </param>
/// <param name="WildCard">	The WildCard. </param>
///
/// <returns>	number of deleted files. </returns>
////////////////////////////////////////////////////////////////////////////////////////////////////
int DeleteFilesAndFolder( CString strPath, CString strWildCard );

CString GetKvCVString();

// Remark: To have same method as defined for TAScope (CTALocalize::GetDashDotDash()).
CString GetDashDotDash( void );

//////////////////////////////////////////////////////////////////////////
//    WildcardMatch
//        pszString    - Input string to match
//        pszMatch    - Match mask that may contain wildcards like ? and *
//
//        A ? sign matches any character, except an empty string.
//        A * sign matches any string inclusive an empty string.
//        Characters are compared caseless.
bool WildcardMatch( const TCHAR *pszString, const TCHAR *pszMatch );

/**
 * This function Clean the article number by deleted any of following char: 
 * - _ blanc char . ; , : / & \r \n
 * @author awa
 * @param  (I) tcArticleNumber: The article number to be clean
 * @param  (IO) pStrAN         : pStrAN is tcArticleNumber without characters bellow
 * @remarks : Created by HYS791
 */
void StrCleanArticleNumber(const TCHAR *tcArticleNumber, CString *pStrAN);

void ParseString( CString line, CString strDelimiter, int* nField, CStringArray* p_outArray );
void ParseStringByChar( CString line, TCHAR charDelimiter, int * nField, CStringArray * p_outArray );
/////////////////////////////////////////////////////////////////////////////////////
// Class used to manage file path
class CPath
{
public:
	CPath( CString strFullPath = _T( "" ) )
	{
		m_FullPath = strFullPath;
	};

	~CPath() {};

	const enum ePathFields
	{
		epfDrive = 1,
		epfDir = 2,
		epfFName = 4,
		epfExt = 8
	};

	/////////////////////////////////////////////////////////////////////////////////////
	// Extract path of a file
	//	index	return			sample
	//		1	drive			"c:"
	//		2	dir				"\sample\crt\"
	//		4	filename		"test"
	//		8	ext				".cpp"
	//	binary combination 1+2 = path = "c:\\sample\crt\"
	CString SplitPath( ePathFields epf );
	
	CString GetPath();
	
	CString GetFilename();

private:
	CString m_FullPath;
};

/////////////////////////////////////////////////////////////////////////////
// Sort CString objects in the increasing order of their sorting key.
// The list can be consulted only once (depending of fDeleteWhenRead) because it is destroyed step by
// step when sweeping through it.
/////////////////////////////////////////////////////////////////////////////
class CTable;
class CExtNumEditComboBox;
class CRank
{
public:
	CRank( bool fDeleteWhenRead = true );
	~CRank();

// Public methods.
public:
	void AddStrSort( const CString &str, double dKey, LPARAM lpItemData, bool bAscendant = true, bool bAddIfExist = true );
	void AddTail( CString str, double dKey, LPARAM lpItemData = 0 );
	void AddHead( CString str, double dKey, LPARAM lpItemData = 0 );
	void Add( const CString &str, double dKey, LPARAM lpItemData = 0, bool bAscendant = true, bool bAddIfExist = true );

	BOOL GetFirst( CString &str, LPARAM &lpItemData, double *pdKey = NULL );
	template<typename T> BOOL GetFirstT( CString &str, T &itemdata, double *pdKey = NULL )
	{
		LPARAM lparam = reinterpret_cast<LPARAM>( itemdata );
		BOOL B = GetFirst( str, lparam, pdKey );
		itemdata = reinterpret_cast<T>( lparam );
		return B;
	};

	BOOL GetNext( CString &str, LPARAM &lpItemData, double *pdKey = NULL );
	template<typename T> BOOL GetNextT( CString &str, T &itemdata, double *pdKey = NULL )
	{
		LPARAM lparam = reinterpret_cast<LPARAM>( itemdata );
		BOOL B = GetNext( str, lparam, pdKey );
		itemdata = reinterpret_cast<T>( lparam );
		return B;
	};

	void PurgeAll();
	BOOL IfExist( CString str );
	BOOL IfExist( double dKey );
	BOOL IfExist( LPARAM lparam );
	
	// Found item based on content of str,itemdata or key (first not empty).
	// return true if item found, false otherwise, fill str,key and itemdata with content of found item.
	BOOL GetaCopy( CString &str, double &dKey, LPARAM &lpItemData );

	BOOL Delete( CString str, LPARAM lpItemData );
	template<typename T>BOOL DeleteT( CString str, T itemdata )
	{
		LPARAM lparam = reinterpret_cast<LPARAM>( itemdata );
		BOOL B = Delete( str, lparam );
		return B;
	};

	int GetCount();


	// Fill a list box or a combo box with the linked list and return the length of the list.
	int Transfer( CListBox *pList );
	int Transfer( CComboBox *pCombo );
	int Transfer( CRank *pRankList );

	int FillFromTable( CTable *pTab );

	// Do not forget to purge the table after using this function.
	void SetReadDeleteFlag( bool bFlag )
	{
		m_fDelete = bFlag;
	};

// Protected variables.
protected:
	struct Item_struct
	{
		CString m_str;
		double m_dKey;
		LPARAM m_lpItemData;
		Item_struct *m_pNext;
	};
	Item_struct *m_Items, *m_FirstItem;
	BOOL m_fDelete;
};

class CExtNumEditComboBox;
class CRankEx
{
public:
	CRankEx();
	~CRankEx() {}

// Public methods.
public:
	void Add( const _string &str, double dKey, LPARAM lpItemData = 0, bool bAddIfExist = true );
	
	bool GetFirst( _string &str, LPARAM &lpItemData, double *pdKey = NULL );
	
	template<typename T> BOOL GetFirstT( _string &str, T &itemdata, double *pdKey = NULL )
	{
		LPARAM lparam = reinterpret_cast<LPARAM>( itemdata );
		BOOL B = GetFirst( str, lparam, pdKey );
		itemdata = reinterpret_cast<T>( lparam );
		return B;
	};

	bool GetNext( _string &str, LPARAM &lpItemData, double *pdKey = NULL );
	
	template<typename T> BOOL GetNextT( _string &str, T &itemdata, double *pdKey = NULL )
	{
		LPARAM lparam = reinterpret_cast<LPARAM>( itemdata );
		BOOL B = GetNext( str, lparam, pdKey );
		itemdata = reinterpret_cast<T>( lparam );
		return B;
	};

	// Retrieve current element without incrementation.
	bool Get( _string &str, LPARAM &lpItemData, double *pdKey = NULL );

	void PurgeAll();
	bool IfExist( _string str, LPARAM *pitemdata = NULL );
	bool IfExist( double dKey );

	// Found item based on content of str,itemdata or key (first not empty).
	// Return true if item found, false otherwise, fill str,key and itemdata with content of found item.
	bool GetaCopy( _string &str, double &dKey, LPARAM &lpItemData );
	int GetCount() { return ( int )m_mmap.size(); }

	bool Delete( double dKey );

	// Delete current element, index is incremented; to be used with GetFirst, GetNext, Get.
	bool Delete();

	CRankEx &operator =( CRankEx &Equation );

#ifndef TACBX //TACBX////////////////////////////////////////////////////////
	// Fill a list box or a combo box with the linked list and return the length of the list.
	int Transfer( CListBox *pList );
	int Transfer( CComboBox *pCombo, bool bIsCustomized = false );
#endif

// Protected variables.
protected:
	typedef std::pair<_string, LPARAM> ItemPair;
	typedef std::pair< double, ItemPair> MMapPair;
	typedef std::multimap< double , ItemPair> _mmap;
	_mmap::iterator m_It;
	_mmap m_mmap;
};

class CFileTxt : public CFile
{
public:
	bool m_bUnicodeFile = false;
	CFileTxt() { CFile(); };
	CFileTxt( LPCTSTR lpszFileName, UINT nOpenFlags )
	{
		Open( lpszFileName, nOpenFlags );
		m_bUnicodeFile = false;
	};

	int ReadTxtLine( TCHAR *pbuf, int iMaxSize, int *piLineCount = NULL );
	int WriteTxtLine( LPCTSTR pbuf, int iMaxSize = 0, TCHAR tcEndChar = '\0' );
};

/////////////////////////////////////////////////////////////////////////////
// CTimeUnic derivate from CTime to overload Format function
/////////////////////////////////////////////////////////////////////////////
class CTimeUnic : public CTime
{
public:
	CTimeUnic(): CTime() {}
	CTimeUnic( __time64_t time ): CTime( time ) {}
	CString Format( UINT nFormatID, bool bGmt = false );
	CString FormatGmt( UINT nFormatID )
	{
		return Format( nFormatID, true );
	}
};
