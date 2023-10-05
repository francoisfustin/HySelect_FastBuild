//////////////////////////////////////////////////////////////////////////
//  Utilities.cpp : implementation file
//	Version: 1.2							Date: 14/04/08
//	CRankEx added
//////////////////////////////////////////////////////////////////////////


#include "stdafx.h"
#include <Windows.h>
#include <math.h>
#include <float.h>
#include <OleCtl.h>
#include <Ole2.h>
#include "accctrl.h"
#include "aclapi.h"

#include "global.h"
#include "DataBase.h"
#include "TASelect.h"
#include "units.h"
#include "Utilities.h"
#include "HydroMod.h"
#include "RegAccess.h"
#include "ExtComboBox.h"

LPCTSTR WriteDouble( double dValue, int iMaxDig, int iMinDec, bool bClean, int iMaxDec )
{
	static _string str;
	TCHAR locinfo[4];

	if( false == TASApp.IsUnitTest() )
	{
		
		GetLocaleInfo( LOCALE_USER_DEFAULT, LOCALE_SDECIMAL, locinfo, SIZEOFINTCHAR( locinfo ) );
	}
	else
	{
		_tcscpy( locinfo, localeconv()->_W_decimal_point );
	}

#ifdef UNICODE
	std::wostringstream oss;
#else
	std::ostringstream oss;
#endif

	if( dValue != 0.0 && ( fabs( dValue ) >= 1.0e+6 || fabs( dValue ) < 1.0e-4 ) )
	{
		oss.flags( std::ios::right | std::ios::scientific );
		oss.width( 20 );					// Total Number of char

		// HYS-1583: Since November 2020, Microsoft has updated their STL librairies and apparently has corrected
		// some of them. So, the 0 parameter for the 'precision' method for stringstream object has now a different
		// behavior. Before, set 0 forced stringstream to have 6 digits for the scientific notation.
		// Now, if we set 0, it is well 0 decimal value !! This is why we changed the calls of WriteDouble from 0
		// to 6 for the 'iMaxDigit' where it was needed.
		
		// BUT "WriteDouble" is called also by "WriteCUDouble". This last method can have the 'iMaxDig' parameter 
		// to the default -1 value. In this case, the 'iMaxDig' is filled with the data in the unit structure links
		// to the current unit. For example "WriteCUDouble( _U_PRESSURE, 25000000, true )", for 'Pascal' we have
		// "UnitDesign( _T("Pa"), 1.0, 0.0, 0 ) );" in the "Units.cpp" file. Thus 'iMaxDigit' is set to 0 (4th value).
		// Before, the value was written as "2.5e+6 Pa". Now if we let 0, the value will be written as "2e+6 Pa".
		// In this case, we will force precision to 6 when 'iMaxDigit' is 0 !!

		if( 0 == iMaxDig )
		{
			iMaxDig = 6;
		}

		oss.precision( iMaxDig );			// Number of decimals
		oss << dValue;
		str = oss.str();
		StrTrimLeft( str );
		
		// Remove leading zeros in exponent.
		int n = ( int )str.find( 'e' );
		int found = ( int )str.find_first_not_of( '0', n + 2 );

		if( found != ( int )_string::npos )
		{
			str.erase( n + 2, found - ( n + 2 ) );
		}

		// Remove trailing zeros in mantissa.
		if( true == bClean )
		{
			while( str.at( n - 1 ) == '0' )
			{
				str.erase( --n, 1 );
			}

			if( str.at( n - 1 ) == '.' )
			{
				str.erase( --n, 1 );
			}
		}
	}
	else
	{
		int n = 0;

		// HYS-1550: iMaxDig >= 0 and iMinDec >= 0. 
		// While iMaxDig - iMinDec <= dValue's integer part width => precision is iMinDec

		if( dValue != 0.0 )
		{
			// Evaluate nbr of digits for rounding
			// HYS-1550: n = the integer part of dValue.
			n = ( int )( log10( fabs( dValue ) ) + 1 );
		}

		iMaxDig = max( iMaxDig, n + iMinDec );

		iMaxDig = max( 1, iMaxDig );

		// HYS-1922
		if( iMaxDec != -1 && iMaxDig - n > iMaxDec )
		{
			// Review iMaxDigit. iMaxDec cannot be exceeded in the decimal part.
			iMaxDig = n + iMaxDec;
		}

		oss.flags( std::ios::right | std::ios::fixed );

		oss.width( iMaxDig );					// Total Number of char

		oss.precision( iMaxDig - n );			// Number of decimals

		// HYS-1550 - See details in the Jira card.
		double dT1 = dValue * pow( 10, ( iMaxDig - n ) );
		int iTemp = (int)( ( dT1 - floorf( (float)dT1 ) ) * 100.0 );

		if( iTemp >= 45 && iTemp <= 49 )
		{
			dValue = ( floorf( (float)dT1 ) + 0.5 ) / pow( 10, ( iMaxDig - n ) );
		}

		oss << dValue;

		str = oss.str();

		StrTrimLeft( str );

		// Remove trailing zeros.
		if(true == bClean )
		{
			int found = 0;

			if( str.find( '.' ) != _string::npos )
			{
				found = ( int )str.find_last_not_of( '0' );

				if( found != _string::npos )
				{
					str.erase( found + 1 );
				}
			}

			found = ( int )str.find_last_not_of( '.' );

			if( found != _string::npos )
			{
				str.erase( found + 1 );
			}
		}
	}

	int iDec = str.find_last_of( '.' );

	if( iDec != _string::npos )
	{
		str.replace( iDec, 1, 1, locinfo[0] );
	}

	return str.c_str();
}

LPCTSTR WriteCUDouble( int iPhysicalType, double dSI, bool fFormattedWithUnit, int iMaxDig, int iMinDec, bool fClean,
					   int iUnitIndex )
{
	double dCU = CDimValue::SItoCU( iPhysicalType, dSI );
	CUnitDatabase *pUnitDB = CDimValue::AccessUDB();
	UnitDesign_struct ud = pUnitDB->GetUnit( iPhysicalType, ( -1 == iUnitIndex ) ? pUnitDB->GetDefaultUnitIndex( iPhysicalType ) : iUnitIndex );
	static _string str;

	if( 0.0 == dCU )
	{
		str = WriteDouble( dCU, min( 1, iMaxDig == -1 ? GetMaxDig( ud ) : iMaxDig ), min( 1, iMinDec == -1 ? GetMinDec( ud ) : iMinDec ), fClean, GetMaxDec( ud ) );
	}
	else
	{
		str = WriteDouble( dCU, iMaxDig == -1 ? GetMaxDig( ud ) : iMaxDig, iMinDec == -1 ? GetMinDec( ud ) : iMinDec, fClean, GetMaxDec( ud ) );
	}

	if( true == fFormattedWithUnit )
	{
		str += _string( _T(" ") ) + pUnitDB->GetNameOfDefaultUnit( iPhysicalType );
	}

	return( str.c_str() );
}

LPCTSTR WriteCustomUnitDouble( int iPhysicalType, double dSI, int iUnitIndex )
{
	return WriteCUDouble( iPhysicalType, dSI, false, -1, -1, true, iUnitIndex );
}

ReadDoubleReturn_enum ReadDoubleFromStr( _string str, double *pdResult )
{
	// Delete all spaces and replace ',' by '.'.
	RemoveTChar( &str, ' ' );

	// Replace , by .
	size_t found = str.find( ',' );

	while( found != _string::npos )
	{
		str.replace( found, 1, _T(".") );
		found = str.find( ',', found );
	}

	*pdResult = 0.0;

	if( true == str.empty() )
	{
		return RD_EMPTY;
	}

	if( false == from_string<double>( str, *pdResult ) )
	{
		return RD_NOT_NUMBER;
	}

	if( ( fabs( *pdResult ) <= 1.0e-100 || fabs( *pdResult ) >= 1.0e+100 ) && *pdResult != 0.0 )
	{
		return RD_NOT_NUMBER;
	}

	return RD_OK;
}

ReadDoubleReturn_enum ReadDouble( CString str, double *pdResult )
{
	return ReadDoubleFromStr( ( _string )( LPCTSTR )str, pdResult );
}

ReadDoubleReturn_enum ReadDouble( CEdit &EditBox, double *pdResult )
{
	CString str;
	EditBox.GetWindowText( str );
	return ReadDouble( str, pdResult );
}

ReadDoubleReturn_enum ReadDouble( CRichEditCtrl &RichEditBox, double *pdResult )
{
	CString str;
	RichEditBox.GetWindowText( str ) ;
	return ReadDouble( str, pdResult );
}

ReadDoubleReturn_enum ReadDouble( CComboBox &ComboBox, double *pdResult )
{
	CString str;
	ComboBox.Invalidate();
	int pos = ComboBox.GetCurSel();

	if( pos >= 0 )
	{
		TCHAR *pStr = new TCHAR[ComboBox.GetLBTextLen( pos ) + 1];
		ComboBox.GetLBText( pos, pStr );
		str = pStr;
		delete[] pStr;
	}
	else
	{
		ComboBox.GetWindowText( str );
	}

	return ReadDouble( str, pdResult );
}

ReadDoubleReturn_enum ReadCUDoubleFromStr( int iPhysicalType, _string str, double *pdResult )
{
	ReadDoubleReturn_enum ret = ReadDoubleFromStr( str, pdResult );

	if( RD_OK == ret )
	{
		*pdResult = CDimValue::CUtoSI( iPhysicalType, *pdResult );
	}

	return ret;
}

ReadDoubleReturn_enum ReadCUDouble( int iPhysicalType, CString str, double *pdResult )
{
	ReadDoubleReturn_enum ret = ReadDouble( str, pdResult );

	if( RD_OK == ret )
	{
		*pdResult = CDimValue::CUtoSI( iPhysicalType, *pdResult );
	}

	return ret;
}

ReadDoubleReturn_enum ReadCUDouble( int iPhysicalType, CEdit &EditBox, double *pdResult )
{
	ReadDoubleReturn_enum ret = ReadDouble( EditBox, pdResult );

	if( RD_OK == ret )
	{
		*pdResult = CDimValue::CUtoSI( iPhysicalType, *pdResult );
	}

	return ret;
}

ReadDoubleReturn_enum ReadCUDouble( int iPhysicalType, CRichEditCtrl &RichEditBox, double *pdResult )
{
	ReadDoubleReturn_enum ret = ReadDouble( RichEditBox, pdResult );

	if( RD_OK == ret )
	{
		*pdResult = CDimValue::CUtoSI( iPhysicalType, *pdResult );
	}

	return ret;
}

ReadDoubleReturn_enum ReadCUDouble( int iPhysicalType, CComboBox &ComboBox, double *pdResult )
{
	ReadDoubleReturn_enum ret = ReadDouble( ComboBox, pdResult );

	if( RD_OK == ret )
	{
		*pdResult = CDimValue::CUtoSI( iPhysicalType, *pdResult );
	}

	return ret;
}

ReadDoubleReturn_enum ReadLongFromStr( _string str, long *pdResult )
{
	// Delete all spaces.
	RemoveTChar( &str, ' ' );

	if( true == str.empty() )
	{
		return RD_EMPTY;
	}

	// Compute the number.
	if( false == from_string<long>( str, *pdResult ) )
	{
		return RD_NOT_NUMBER;
	}

	return RD_OK;
}

ReadDoubleReturn_enum ReadLong( CString str, long *pdResult )
{
	return ReadLongFromStr( ( _string )( LPCTSTR )str, pdResult );
}

ReadDoubleReturn_enum ReadLong( CEdit &EditBox, long *pdResult )
{
	CString str;
	EditBox.GetWindowText( str );
	return ReadLong( str, pdResult );
}

ReadDoubleReturn_enum ReadLong( CComboBox &ComboBox, long *pdResult )
{
	CString str;
	ComboBox.GetWindowText( str );
	return ReadLong( str, pdResult );
}

void FormatString( CString &rString, UINT nIDS, LPCTSTR lpsz1 )
{
	rString = TASApp.LoadLocalizedString( nIDS );
	int pos;

	while( ( pos = rString.Find( _T("%1"), 0 ) ) >= 0 )
	{
		rString.Delete( pos, 2 );
		rString.Insert( pos, lpsz1 );
	}
}

void FormatString( CString &rString, CString str, LPCTSTR lpsz1 )
{
	rString = str;
	int pos;

	while( ( pos = rString.Find( _T("%1"), 0 ) ) >= 0 )
	{
		rString.Delete( pos, 2 );
		rString.Insert( pos, lpsz1 );
	}
}

void FormatString( CString &rString, UINT nIDS, LPCTSTR lpsz1, LPCTSTR lpsz2 )
{
	FormatString( rString, nIDS, lpsz1 );
	int pos;

	while( ( pos = rString.Find( _T("%2"), 0 ) ) >= 0 )
	{
		rString.Delete( pos, 2 );
		rString.Insert( pos, lpsz2 );
	}
}

void FormatString( CString &rString, CString &str, LPCTSTR lpsz1, LPCTSTR lpsz2 )
{
	FormatString( rString, str, lpsz1 );
	int pos;

	while( ( pos = rString.Find( _T("%2"), 0 ) ) >= 0 )
	{
		rString.Delete( pos, 2 );
		rString.Insert( pos, lpsz2 );
	}
}

void FormatString( CString &rString, UINT nIDS, LPCTSTR lpsz1, LPCTSTR lpsz2, LPCTSTR lpsz3 )
{
	FormatString( rString, nIDS, lpsz1, lpsz2 );
	int pos;

	while( ( pos = rString.Find( _T("%3"), 0 ) ) >= 0 )
	{
		rString.Delete( pos, 2 );
		rString.Insert( pos, lpsz3 );
	};
}

void FormatString( CString &rString, CString &str, LPCTSTR lpsz1, LPCTSTR lpsz2, LPCTSTR lpsz3 )
{
	FormatString( rString, str, lpsz1, lpsz2 );
	int pos;

	while( ( pos = rString.Find( _T("%3"), 0 ) ) >= 0 )
	{
		rString.Delete( pos, 2 );
		rString.Insert( pos, lpsz3 );
	};
}

void FormatString( CString &rString, UINT nIDS, LPCTSTR lpsz1, LPCTSTR lpsz2, LPCTSTR lpsz3, LPCTSTR lpsz4 )
{
	FormatString( rString, nIDS, lpsz1, lpsz2, lpsz3 );
	int pos;

	while( ( pos = rString.Find( _T("%4"), 0 ) ) >= 0 )
	{
		rString.Delete( pos, 2 );
		rString.Insert( pos, lpsz4 );
	};
}

void FormatString( CString &rString, CString &str, LPCTSTR lpsz1, LPCTSTR lpsz2, LPCTSTR lpsz3, LPCTSTR lpsz4 )
{
	FormatString( rString, str, lpsz1, lpsz2, lpsz3 );
	int pos;

	while( ( pos = rString.Find( _T("%4"), 0 ) ) >= 0 )
	{
		rString.Delete( pos, 2 );
		rString.Insert( pos, lpsz4 );
	};
}

CString ConvertVecStringToString( std::vector<CString> vecStrings, CString strPrefix , bool bReturnLine )
{
	CString str = _T("");

	for( int i = 0; i < (int)vecStrings.size(); i++ )
	{
		if( false == strPrefix.IsEmpty() && vecStrings.size() > 1 )
		{
			str += strPrefix;
		}
		
		str += vecStrings.at( i );

		if( true == bReturnLine && i < ( (int)vecStrings.size() - 1 ) )
		{
			str += _T("\r\n");
		}
	}

	return str;
}

// 65001 is utf-8.
wchar_t *CodePageToUnicode( int codePage, const char *src )
{
	if( !src )
	{
		return 0;
	}

	int srcLen = strlen( src );

	if( !srcLen )
	{
		wchar_t *w = new wchar_t[1];
		w[0] = 0;
		return w;
	}

	int requiredSize = MultiByteToWideChar( codePage, 0, src, srcLen, 0, 0 );

	if( !requiredSize )
	{
		return 0;
	}

	wchar_t *w = new wchar_t[requiredSize + 1];
	w[requiredSize] = 0;

	int retval = MultiByteToWideChar( codePage, 0, src, srcLen, w, requiredSize );

	if( !retval )
	{
		delete[] w;
		return 0;
	}

	return w;
}

char *UnicodeToCodePage( int codePage, const wchar_t *src )
{
	if( !src )
	{
		return 0;
	}

	int srcLen = wcslen( src );

	if( !srcLen )
	{
		char *x = new char[1];
		x[0] = '\0';
		return x;
	}

	int requiredSize = WideCharToMultiByte( codePage, 0, src, srcLen, 0, 0, 0, 0 );

	if( !requiredSize )
	{
		return 0;
	}

	char *x = new char[requiredSize + 1];
	x[requiredSize] = 0;

	int retval = WideCharToMultiByte( codePage, 0, src, srcLen, x, requiredSize, 0, 0 );

	if( !retval )
	{
		delete[] x;
		return 0;
	}

	return x;
}

/////////////////////////////////////////////////////////////////////////////
// Sort CString objects in the increasing order of their sorting key.
/////////////////////////////////////////////////////////////////////////////
CRank::CRank( bool fDeleteWhenRead/*=true*/ )
{
	m_fDelete = fDeleteWhenRead;
	m_FirstItem = m_Items = 0;
}

CRank::~CRank()
{
	PurgeAll();
}
void CRank::PurgeAll()
{
	Item_struct *pItem;

	if( !m_fDelete )
	{
		m_Items = m_FirstItem;
	}

	while( m_Items )
	{
		pItem = m_Items->m_pNext;
		delete m_Items;
		m_Items = pItem;
	}

	m_FirstItem = m_Items;

}

void CRank::AddStrSort( const CString &str, double key, LPARAM itemdata, bool bAscendant, bool bAddIfExist )
{
	Item_struct **pPrecPtr = &m_Items;
	Item_struct *pCursor = m_Items;

	if( true == bAscendant )
	{
		while( NULL != pCursor && ( pCursor->m_str.Compare( str ) > 0 ) )
		{
			pPrecPtr = &pCursor->m_pNext;
			pCursor = *pPrecPtr;
		}
	}
	else
	{
		while( NULL != pCursor && ( pCursor->m_str.Compare( str ) < 0 ) )
		{
			pPrecPtr = &pCursor->m_pNext;
			pCursor = *pPrecPtr;
		}
	}

	if( false == bAddIfExist && NULL != pCursor )
	{
		// Skip it if already exist
		if( itemdata == pCursor->m_lpItemData
			&& str == pCursor->m_str
			&& key == pCursor->m_dKey )
		{
			return ;
		}
	}

	Item_struct *pNewItem = new Item_struct;
	pNewItem->m_str = str;
	pNewItem->m_dKey = key;
	pNewItem->m_lpItemData = itemdata;
	pNewItem->m_pNext = pCursor;
	*pPrecPtr = pNewItem;
	m_FirstItem = m_Items;
}

void CRank::AddTail( CString str, double key, LPARAM itemdata )
{
	m_Items = m_FirstItem;
	Item_struct **pPrecPtr = &m_Items;
	Item_struct *Cursor = m_Items;

	while( Cursor )
	{
		pPrecPtr = &Cursor->m_pNext;
		Cursor = *pPrecPtr;
	}

	Item_struct *pNewItem = new Item_struct;
	pNewItem->m_str = str;
	pNewItem->m_dKey = key;
	pNewItem->m_lpItemData = itemdata;
	pNewItem->m_pNext = Cursor;
	*pPrecPtr = pNewItem;
	m_FirstItem = m_Items;
}

void CRank::AddHead( CString str, double key, LPARAM itemdata )
{
	m_Items = m_FirstItem;
	Item_struct **pPrecPtr = &m_Items;
	Item_struct *Cursor = m_Items;
	Item_struct *pNewItem = new Item_struct;
	pNewItem->m_str = str;
	pNewItem->m_dKey = key;
	pNewItem->m_lpItemData = itemdata;
	pNewItem->m_pNext = Cursor;
	*pPrecPtr = pNewItem;
	m_FirstItem = m_Items;
}

void CRank::Add( const CString &str, double key, LPARAM itemdata/*=0*/, bool bAscendant/*=true*/, bool bAddIfExist/*=true*/ )
{
	m_Items = m_FirstItem;
	Item_struct **ppPrecPtr = &m_Items;
	Item_struct *pCursor = m_Items;

	if( true == bAscendant )
	{
		while( NULL != pCursor && ( pCursor->m_dKey < key ) )
		{
			ppPrecPtr = &pCursor->m_pNext;
			pCursor = *ppPrecPtr;
		}
	}
	else
	{
		while( NULL != pCursor && ( pCursor->m_dKey > key ) )
		{
			ppPrecPtr = &pCursor->m_pNext;
			pCursor = *ppPrecPtr;
		}
	}

	if( false == bAddIfExist && NULL != pCursor )
	{
		// Skip it if already exist
		if( itemdata == pCursor->m_lpItemData
				&& str == pCursor->m_str
				&& key == pCursor->m_dKey )
		{
			return;
		}
	}

	Item_struct *pNewItem = new Item_struct;
	pNewItem->m_str = str;
	pNewItem->m_dKey = key;
	pNewItem->m_lpItemData = itemdata;
	pNewItem->m_pNext = pCursor;
	*ppPrecPtr = pNewItem;
	m_FirstItem = m_Items;
}

BOOL CRank::Delete( CString str, LPARAM itemdata )
{
	//m_Items = m_FirstItem;
	Item_struct *pItem, *pPrv;

	for( pItem = m_FirstItem; pItem; )
	{
		if( itemdata && itemdata != pItem->m_lpItemData )
		{
			pPrv = pItem;
			pItem = pItem->m_pNext;
			continue;
		};

		if( !str.IsEmpty() && str != pItem->m_str )
		{
			pPrv = pItem;
			pItem = pItem->m_pNext;
			continue;
		};

		if( pItem == m_Items )
		{
			m_Items = pItem->m_pNext;
		}

		// delete first Item
		if( pItem == m_FirstItem )
		{
			m_FirstItem = pItem->m_pNext;
			delete pItem;
			return true;
		}
		else
		{
			pPrv->m_pNext = pItem->m_pNext;
			delete pItem;
			return true;
		}
	};

	return false;
}

BOOL CRank::GetFirst( CString &str, LPARAM &itemdata, double *pdKey /*=NULL*/ )
{
	if( !m_fDelete )
	{
		m_Items = m_FirstItem;
	}

	return GetNext( str, itemdata, pdKey );
}

BOOL CRank::GetNext( CString &str, LPARAM &itemdata, double *pdKey /*=NULL*/ )
{
	if( !m_Items )
	{
		return FALSE;
	}

	str = m_Items->m_str;
	itemdata = m_Items->m_lpItemData;

	if( pdKey )
	{
		*pdKey = m_Items->m_dKey;
	}

	Item_struct *pItem = m_Items;
	m_Items = m_Items->m_pNext;

	if( m_fDelete )
	{
		// End of list reached reset m_FirstItem
		if( !m_Items )
		{
			m_FirstItem = NULL;
		}

		delete pItem;
	}

	return TRUE;
}

int CRank::Transfer( CListBox *pList )
{
	if( pList->GetCount() )
	{
		pList->ResetContent();
	}

	CString str;
	LPARAM itemdata;

	for( BOOL cont = GetFirst( str, itemdata ); cont; cont = GetNext( str, itemdata ) )
	{
		int iItem = pList->AddString( str );
		pList->SetItemData( iItem, itemdata );
	};

	return pList->GetCount();
}

int CRank::Transfer( CComboBox *pCombo )
{
	if( pCombo->GetCount() )
	{
		pCombo->ResetContent();
	}

	CString str;
	LPARAM itemdata;

	for( BOOL cont = GetFirst( str, itemdata ); cont; cont = GetNext( str, itemdata ) )
	{
		int iItem = pCombo->AddString( str );
		pCombo->SetItemData( iItem, itemdata );
	};

	return pCombo->GetCount();
}

int CRank::Transfer( CRank *pRankList )
{
	if( pRankList->GetCount() )
	{
		pRankList->PurgeAll();
	}

	CString str;
	LPARAM itemdata;
	double Key = 0;

	for( BOOL cont = GetFirst( str, itemdata ); cont; cont = GetNext( str, itemdata ) )
	{
		pRankList->Add( str, Key, itemdata );
		Key++;
	};

	return pRankList->GetCount();
}

BOOL CRank::IfExist( CString str )
{
	Item_struct *pItem = m_FirstItem;

	while( pItem )
	{
		if( pItem->m_str == str )
		{
			return true;
		}

		pItem = pItem->m_pNext;
	};

	return false;
}

BOOL CRank::IfExist( LPARAM lparam )
{
	Item_struct *pItem = m_FirstItem;

	while( pItem )
	{
		if( pItem->m_lpItemData == lparam )
		{
			return true;
		}

		pItem = pItem->m_pNext;
	};

	return false;
}

BOOL CRank::IfExist( double Key )
{
	Item_struct *pItem = m_FirstItem;

	while( pItem )
	{
		if( pItem->m_dKey == Key )
		{
			return true;
		}

		pItem = pItem->m_pNext;
	};

	return false;
}

BOOL CRank::GetaCopy( CString &str, double &key, LPARAM &itemdata )
{
	Item_struct *pItem = m_FirstItem;
	bool found = false;

	// Test on string content
	if( !str.IsEmpty() )
	{
		while( pItem && !found )
		{
			if( pItem->m_str == str )
			{
				found = true;
			}
			else
			{
				pItem = pItem->m_pNext;
			}
		}
	}
	// Test on LPARAM
	else if( itemdata )
	{
		while( pItem && !found )
		{
			if( pItem->m_lpItemData == itemdata )
			{
				found = true;
			}
			else
			{
				pItem = pItem->m_pNext;
			}
		}
	}
	// Test on key
	else
	{
		while( pItem && !found )
		{
			if( pItem->m_dKey == key )
			{
				found = true;
			}
			else
			{
				pItem = pItem->m_pNext;
			}
		}
	}

	if( found )
	{
		str = pItem->m_str;
		key = pItem->m_dKey;
		itemdata = pItem->m_lpItemData;
		return true;
	}

	return false;
}


int CRank::GetCount()
{
	Item_struct *pItem = m_FirstItem;
	int i = 0;

	while( pItem )
	{
		i++;
		pItem = pItem->m_pNext;
	};

	return i;
}

int CRank::FillFromTable( CTable *pTab )
{
	if( NULL == pTab )
	{
		return 0;
	}

	int iCount = 0;

	for( IDPTR idptr = pTab->GetFirst(); NULL != idptr.MP; idptr = pTab->GetNext( idptr.MP ) )
	{
		Add( idptr.ID, 0, ( LPARAM )idptr.MP );
		iCount ++;
	}

	return ( iCount );
}

/////////////////////////////////////////////////////////////////////////////
// CRankEx
CRankEx::CRankEx()
{
	m_It = m_mmap.end();
}


void CRankEx::Add( const _string &str, double dKey, LPARAM itemdata/*=0*/, bool bAddIfExist/*=true*/ )
{
	_mmap::iterator it;

	if( !bAddIfExist )
	{
		it = m_mmap.find( dKey );

		if( it != m_mmap.end() )		// Item found
		{
			return;
		}
	};

	m_mmap.insert( MMapPair( dKey, ItemPair( str, itemdata ) ) );
}

bool CRankEx::GetFirst( _string &str, LPARAM &itemdata, double *pdKey )
{
	m_It = m_mmap.begin();
	return GetNext( str, itemdata, pdKey );
}

bool CRankEx::GetNext( _string &str, LPARAM &itemdata, double *pdKey/*=NULL*/ )
{
	if( Get( str, itemdata, pdKey ) )
	{
		++m_It;
		return true;
	}

	return false;
}

// Retrieve current element without incrementation
bool CRankEx::Get( _string &str, LPARAM &itemdata, double *pdKey/*=NULL*/ )
{
	if( m_It != m_mmap.end() )
	{
		if( pdKey )
		{
			*pdKey = m_It->first;
		}

		str = m_It->second.first;
		itemdata = m_It->second.second;
		return true;
	}

	m_It = m_mmap.end();
	return false;
}
void CRankEx::PurgeAll()
{
	m_mmap.clear();
}

bool CRankEx::IfExist( _string str, LPARAM *pitemdata )
{
	_mmap::const_iterator cIt;

	for( cIt = m_mmap.begin(); cIt != m_mmap.end(); ++cIt )
	{
		if( 0 == str.compare( cIt->second.first ) )
		{
			if( NULL != pitemdata )
			{
				*pitemdata = cIt->second.second;
			}
			
			return true;
		}
	}

	return false;
}

bool CRankEx::IfExist( double dKey )
{
	_mmap::const_iterator cIt;
	cIt = m_mmap.find( dKey );
	return ( cIt != m_mmap.end() );
}

bool CRankEx::GetaCopy( _string &str, double &dKey, LPARAM &itemdata )
{
	if( false == str.empty() )
	{
		// Test on string content.
		_mmap::const_iterator cIt;

		for( cIt = m_mmap.begin(); cIt != m_mmap.end(); ++cIt )
		{
			if( 0 == str.compare( cIt->second.first ) )
			{
				dKey = cIt->first;
				itemdata = cIt->second.second;
				return true;
			}
		}
	}
	else if( ( LPARAM )0 != itemdata )
	{
		// Test on LPARAM.
		_mmap::const_iterator cIt;

		for( cIt = m_mmap.begin(); cIt != m_mmap.end(); ++cIt )
		{
			if( cIt->second.second == itemdata )
			{
				dKey = cIt->first;
				str = cIt->second.first;
				return true;
			}
		}
	}
	else
	{
		// Test on key.
		_mmap::const_iterator cIt;
		cIt = m_mmap.find( dKey );

		if( cIt != m_mmap.end() )
		{
			dKey = cIt->first;
			str = cIt->second.first;
			itemdata = cIt->second.second;
			return true;
		}
	}

	return false;
}

bool CRankEx::Delete( double dKey )
{
	_mmap::iterator It;
	It = m_mmap.find( dKey );

	if( It != m_mmap.end() )
	{
		m_mmap.erase( It );
		return true;
	}

	return false;
}

bool CRankEx::Delete()
{
	_mmap::iterator It = m_It;

	if( m_It == m_mmap.begin() )
	{
		It = m_mmap.begin();
	}
	else
	{
		--It;
	}

	m_mmap.erase( It );
	return true;
}

CRankEx &CRankEx::operator=( CRankEx &RkList )
{
	PurgeAll();

	_string str;
	LPARAM itemdata = 0;
	double dKey = 0;

	for( bool f = RkList.GetFirst( str, itemdata, &dKey ); true == f; f = RkList.GetNext( str, itemdata, &dKey ) )
	{
		Add( str, dKey, itemdata, true );
	}

	return *this;
}

#ifndef TACBX //TACBX////////////////////////////////////////////////////////
int CRankEx::Transfer( CListBox *pList )
{
	if( NULL == pList )
	{
		ASSERTA_RETURN( -1 );
	}

	if( pList->GetCount() )
	{
		pList->ResetContent();
	}

	_mmap::const_iterator cIt;

	for( cIt = m_mmap.begin(); cIt != m_mmap.end(); ++cIt )
	{
		int iItem = pList->AddString( cIt->second.first.c_str() );
		pList->SetItemData( iItem, cIt->second.second );
	}

	return pList->GetCount();
}

int CRankEx::Transfer( CComboBox *pCombo, bool bIsCustomized )
{
	if( NULL == pCombo )
	{
		ASSERTA_RETURN( -1 );
	}

	if( pCombo->GetCount() > 0 )
	{
		pCombo->ResetContent();
	}

	_mmap::const_iterator cIt;

	for( cIt = m_mmap.begin(); cIt != m_mmap.end(); ++cIt )
	{
		// HYS-1398 : We have the possibilty to customized deleted products display
		CString strToAdd = cIt->second.first.c_str();
		if( true == bIsCustomized )
		{
			CDB_TAProduct *pTA = (CDB_TAProduct*)cIt->second.second;
			if( NULL != pTA && false == pTA->IsSelectable( true ) )
			{
				strToAdd += _T( " (**") + TASApp.LoadLocalizedString(IDS_STRDELETED) + _T("**)" );
			}
		}

		int iItem = pCombo->AddString( strToAdd );
		pCombo->SetItemDataPtr( iItem, (void *)cIt->second.second );
	}

	return pCombo->GetCount();
}

#endif

int CFileTxt::WriteTxtLine( LPCTSTR pbuf, int maxsize/*=0*/, TCHAR endchar/*='\0'*/ )
{
	ASSERT( pbuf );
	TCHAR CR = '\r';
	int Count = 0;

	while( *pbuf != endchar )
	{
		Count ++;

		if( maxsize && Count >= maxsize )
		{
			break;
		}

		if( *pbuf == '\n' )
		{
			Write( &CR, sizeof( TCHAR ) );
		}

		Write( pbuf, sizeof( TCHAR ) );
		pbuf++;
	}

	// Write \r\n
	/*
		CFile::Write(_T("\r\n"),sizeof(TCHAR)*2);
		Count +=2;
	*/
	return Count;
}

int CFileTxt::ReadTxtLine(TCHAR *pBuf, int maxsize, int *pLineCount /* = NULL */)
{
	ASSERT( pBuf );

	if( !pBuf )
	{
		return -1;
	}

	TCHAR *pBuffer = pBuf;
	int count = 0;
	bool ended = false;

	if( pLineCount && *pLineCount == 0 )
	{
		TRACE( _T("\nStart reading text file : %s\n"), ( LPCTSTR )this->GetFileName() );
	}


	if( GetPosition() == 0 )
	{
		Read( pBuf, sizeof( TCHAR ) );
		pBuf = pBuffer;

		// Skip two first characters in case of unicode text file normally (0xFF 0xFE).
		if (0xFEFF == pBuf[0] )			// Unicode file
		{
			m_bUnicodeFile = true;
		}
		else
		{
			SeekToBegin();
		}
	}

	uint8_t ChrSize = sizeof(char);
	if (true == m_bUnicodeFile)
	{
		ChrSize = sizeof(TCHAR);
	}

	memset( pBuf, 0, maxsize );

	while (Read(pBuf, ChrSize) && count < maxsize)
	{
		if( ended && *pBuf != '\n' )
		{
			continue;
		}

		// end of line reached
		if( *pBuf == '\n' )
		{
			if( NULL != pLineCount )
			{
				( *pLineCount )++;
			}

			*pBuf = '\0';

			if( count && *( pBuf - 1 ) == '\r' )
			{
				*( pBuf - 1 ) = 0;
				count --;
			}

			if( *pBuffer != '\0' )	// Test if first character exist
			{
				break;
			}
			else					// Empty Line, next Line
			{
				ended = false;
				pBuf = pBuffer;
				count = 0;
				continue;
			}
		}

		// stop string when comment reached
		if( count )
			if( *pBuf == '/' && *( pBuf - 1 ) == '/' )
			{
				*( pBuf - 1 ) = '\0';
				ended = true;
				continue;
			}

		pBuf ++;
		*pBuf = 0;
		count++;
	}

	return count;
}


/////////////////////////////////////////////////////////////////////////////
// CTimeUnic derivate from CTime to overload Format function
/////////////////////////////////////////////////////////////////////////////
CString CTimeUnic::Format( UINT nFormatID, bool bGmt /*=false*/ )
{
	ASSERT( nFormatID );
	CString strID;
	strID = TASApp.LoadLocalizedString( nFormatID );
	CString subStr, FormatedStr;

	// The CTime::Format function can
	// only takes %d%m%y:%H%M%S
	// Verify that each character
	// following the % is d,m,y,H,M,S
	// Otherwize return '?'
	int pos = strID.Find( '%' );

	while( pos >= 0 )
	{
		subStr = strID.Mid( pos + 1, 1 );

		if( subStr != 'd' && subStr != 'm' && subStr != 'y' &&
			subStr != 'H' && subStr != 'M' && subStr != 'S' )
		{
			return L"?";
		}

		pos = strID.Find( '%', pos + 2 );
	}

	pos = strID.Find( '%' );

	while( pos >= 0 )
	{
		subStr = strID.Mid( pos, 2 );
		FormatedStr = bGmt ? CTime::FormatGmt( subStr ) : CTime::Format( subStr );
		strID.Replace( subStr, FormatedStr );
		pos = strID.Find( '%', pos + 2 );
	}

	return strID;
}

int StringCompare( LPCTSTR pbuf1, LPCTSTR pbuf2 )
{
	// If pbuf1 not empty and pbuf 2 empty...
	if( ( NULL != pbuf1 && NULL == pbuf2 ) || ( '\0' != *pbuf1 && '\0' == *pbuf2 ) )
	{
		return 1;
	}
	
	// If pbuf1 empty and pbuf2 not empty...
	if( ( NULL == pbuf1 && NULL != pbuf2 ) || ( '\0' == *pbuf1 && '\0' != *pbuf2 ) )
	{
		return -1;
	}
	
	// If both empty...
	if( ( NULL == pbuf1 && NULL == pbuf2 ) || ( '\0' == *pbuf1 && '\0' == *pbuf2 ) )
	{
		return 0;
	}

	return _tcscmp( pbuf1, pbuf2 );
}

void ByteToHexAscii( BYTE src, BYTE *pbDst )
{
	( ( src >> 4 ) > 9 ) ? *pbDst = ( src >> 4 ) + 0x41 - 10 : *pbDst = ( src >> 4 ) + 0x30;			// nibble high.
	pbDst++;
	( ( src & 0xF ) > 9 ) ? *pbDst = ( src & 0xF ) + 0x41 - 10 : *pbDst = ( src & 0xF ) + 0x30;			// nibble low.
}

BYTE HexAsciiToByte( BYTE *pbSrc )
{
	BYTE out;
	out = *pbSrc;
	( out > 0x39 ) ? out = out - 0x41 + 10 : out -= 0x30;
	pbSrc++;
	out <<= 4;
	( *pbSrc > 0x39 ) ? out += *pbSrc - 0x41 + 10 : out += *pbSrc - 0x30;
	return out;
}

CString Base26Convert( ULONGLONG ullVal, UCHAR ucLen )
{
	if( ucLen > 20 )
	{
		return _T( "" );
	}

	CString FillStr;
	int i;

	for( i = 0; i < ucLen; i++ )
	{
		FillStr += _T("A");
	}

	char aExp[20];
	int pos = 0;
	memset( aExp, -1, sizeof( aExp ) );

	// Base 26 conversion 'A'=0 'Z'=25
	while( pos < sizeof( aExp ) )
	{
		aExp[pos] = ( char )( ullVal % 26 );
		ullVal /= 26;

		if( 0 == ullVal )
		{
			break;
		}

		ullVal--;
		pos++;
	}

	CString str;

	while( pos >= 0 )
	{
		CString tmp;
		tmp.Format( _T("%c"), aExp[pos] + 'A' );
		str += tmp;
		pos--;
	}

	if( ucLen > 0  && ( str.GetLength() < FillStr.GetLength() ) )
	{
		FillStr.Delete( FillStr.GetLength() - str.GetLength(), str.GetLength() );
		FillStr += str;
		str = FillStr;
	}

	return str;
}

CString GetTimeBasedUniqID()
{
	// ID based on
	union _uDate
	{
		// 42 bits 4.39e12 possibilities
		// 9 characters in base 26
		struct
		{
			unsigned  nRand	:	9;  //0..1023 (random number)
			unsigned  nSec	:	6;	//0..31 (used 0 to 59)
			unsigned  nMin	:	6;	//0..63 (used 0 to 59)
			unsigned  nHour	:	5;	//0..31 (used 0 to 23)
			unsigned  nDay	:	5;	//0..31	(used 1 to 31)
			unsigned  nMonth:	4;	//0..15	(used 1 to 12)
			unsigned  nYear	:	6;	//0..63(used 0 to 99 start at 2005)
		} bf;
		ULONGLONG ulDate;
	} uDate;

	COleDateTime dt;
	dt = COleDateTime::GetCurrentTime();
	uDate.ulDate = 0;
	uDate.bf.nDay = dt.GetDay();
	uDate.bf.nMonth = dt.GetMonth();
	uDate.bf.nYear = dt.GetYear() - 2005;
	uDate.bf.nHour = dt.GetHour();
	uDate.bf.nMin = dt.GetMinute();
	uDate.bf.nSec = dt.GetSecond();

	LARGE_INTEGER lPerformanceCount;
	QueryPerformanceCounter( &lPerformanceCount );
	uDate.bf.nRand = ( unsigned int )( lPerformanceCount.LowPart ^ lPerformanceCount.HighPart );
	CString ID = Base26Convert( uDate.ulDate, 9 );
	ASSERT( ID.GetLength() == 9 );
	return ID;
}

CString GetProjectDirectory()
{
	CString InstDir = TASApp.GetStartDir();
	CString ProjDir( AfxGetApp()->GetProfileString( _T("Files" ), _T( "Project Directory" ), _T( "") ) );

	if( ProjDir != _PROJECTS_DIRECTORY )
	{
		InstDir = ProjDir;
	}
	else
	{
		InstDir = InstDir + _PROJECTS_DIRECTORY;
	}

	CPath path( InstDir );
	InstDir = path.SplitPath( ( CPath::ePathFields )( CPath::ePathFields::epfDrive + CPath::ePathFields::epfDir ) );
	return InstDir;
}

void WaitMillisec( unsigned short us )
{
	ULONGLONG ullLoopDelay = GetTickCount64();

	while( ( GetTickCount64() - ullLoopDelay ) < us );
}

void RemoveFile( CString strFileName )
{
	CFileStatus fs;

	try
	{
		if( TRUE == CFile::GetStatus( strFileName, fs ) )
		{
			// Remove file if exists.
			fs.m_attribute &= ~0x01;
			fs.m_attribute &= ~0x20;
			fs.m_mtime = 0;
			CFile::SetStatus( strFileName, fs );
			CFile::Remove( strFileName );
		}
	}
	catch( ... )
	{
		HYSELECT_THROW( _T("Error in 'CTASelectApp::ReadTADBTxT'.") );
	}
}

bool FileMoveCopy( CString strSource, CString strTarget, bool bDeleteSource )
{
	// Delete file in target directory if exist.
	CFileStatus fs;

	try
	{
		if( TRUE == CFile::GetStatus( strTarget, fs ) )
		{
			// remove the file if exists.
			fs.m_attribute &= ~0x01;
			fs.m_attribute &= ~0x20;
			fs.m_mtime = 0;
			CFile::SetStatus( strTarget, fs );
			CFile::Remove( strTarget );
		}
	}
	catch( ... )
	{
		return false;
	}

	// Copy the file (source) with the destination name.
	try
	{
		// Copy file.
		CopyFile( strSource, strTarget, false );
		CFileStatus fs;

		if( TRUE == CFile::GetStatus( strTarget, fs ) )
		{
			// Change file status to writable.
			fs.m_attribute &= !0x01;
			CFile::SetStatus( strTarget, fs );
		}
	}
	catch( ... )
	{
		return false;
	}

	// Delete source file.
	if( true == bDeleteSource )
	{
		try
		{
			if( TRUE == CFile::GetStatus( strSource, fs ) )
			{
				// Remove the file if exists.
				fs.m_attribute &= !0x01;
				CFile::SetStatus( strSource, fs );
				CFile::Remove( strSource );
			}
		}
		catch( ... )
		{
			return false;
		}
	}

	return true;
}

int DeleteFilesAndFolder( CString strPath, CString strWildCard )
{
	CFileFind finder;
	int iCount = 0;
	BOOL bWorking = finder.FindFile( strPath + _T("\\") + strWildCard );

	while( TRUE == bWorking )
	{
		bWorking = finder.FindNextFile();
		TRACE( _T("%s\n"), ( LPCTSTR )finder.GetFilePath() );

		if( TRUE == finder.IsDots() )
		{
			continue;
		}

		if( TRUE == finder.IsDirectory() )
		{
			iCount += DeleteFilesAndFolder( finder.GetFilePath(), strWildCard );
			RemoveDirectory( finder.GetFilePath() );
		}
		else
		{
			RemoveFile( ( LPCTSTR )finder.GetFilePath() );
			iCount ++;
		}
	}

	return ( iCount );
}

CString GetKvCVString()
{
	CString str;
	CUnitDatabase *pUnitDB = CDimValue::AccessUDB();

	if( NULL != pUnitDB )
	{
		if( 0 == pUnitDB->GetDefaultUnitIndex( _C_KVCVCOEFF ) )
		{
			str = TASApp.LoadLocalizedString( IDS_KVS );
		}
		else
		{
			str = TASApp.LoadLocalizedString( IDS_CV );
		}
	}

	return str;
}

CString GetDashDotDash( void )
{
	static WCHAR locinfo[2];
	GetLocaleInfo( LOCALE_USER_DEFAULT, LOCALE_SDECIMAL, locinfo, 2 * sizeof( WCHAR ) );
	CString str = _T("-" ) + CString( locinfo ) + _T( "-");
	return str;
}

bool WildcardMatch( const TCHAR *pszString, const TCHAR *pszMatch )
{
	// We have a special case where string is empty ("") and the mask is "*".
	// We need to handle this too. So we can't test on !*pszString here.
	// The loop breaks when the match string is exhausted.
	while( '\0' != *pszMatch )
	{
		// Single wildcard character.
		if( _T( '?' ) == *pszMatch )
		{
			// Matches any character except empty string.
			if( '\0' == *pszString )
			{
				return false;
			}

			// OK next.
			++pszString;
			++pszMatch;
		}
		else if( _T( '*' ) == *pszMatch )
		{
			// Need to do some tricks.

			// 1. The wildcard * is ignored.
			//    So just an empty string matches. This is done by recursion.
			//      Because we eat one character from the match string, the
			//      recursion will stop.
			if( true == WildcardMatch( pszString, pszMatch + 1 ) )
				// we have a match and the * replaces no other character
			{
				return true;
			}

			// 2. Chance we eat the next character and try it again, with a
			//    wildcard * match. This is done by recursion. Because we eat
			//      one character from the string, the recursion will stop.
			if( '\0' != *pszString && true == WildcardMatch( pszString + 1, pszMatch ) )
			{
				return true;
			}

			// Nothing worked with this wildcard.
			return false;
		}
		else
		{
			// Standard compare of 2 chars. Note that *pszSring might be 0
			// here, but then we never get a match on *pszMask that has always
			// a value while inside this loop.
			if( ::CharUpper( MAKEINTRESOURCE( MAKELONG( *pszString++, 0 ) ) )
				!=::CharUpper( MAKEINTRESOURCE( MAKELONG( *pszMatch++, 0 ) ) ) )
			{
				return false;
			}
		}
	}

	// Have a match? Only if both are at the end...
	return !*pszString && !*pszMatch;
}

void StrCleanArticleNumber(const TCHAR *tcArticleNumber, CString *pStrAN)
{
	int index = 0;
	size_t iArtNumLen = wcslen(tcArticleNumber);

	if ((0 >= iArtNumLen) || (pStrAN == NULL))
	{
		return;
	}

	for (index = 0; index < (int)iArtNumLen; index++)
	{
		if (!(tcArticleNumber[index] == _T('-')
			|| tcArticleNumber[index] == _T('_')
			|| tcArticleNumber[index] == _T(' ')
			|| tcArticleNumber[index] == _T('.')
			|| tcArticleNumber[index] == _T(';')
			|| tcArticleNumber[index] == _T(',')
			|| tcArticleNumber[index] == _T(':')
			|| tcArticleNumber[index] == _T('/')
			|| tcArticleNumber[index] == _T('&')
			|| tcArticleNumber[index] == _T('\r')
			|| tcArticleNumber[index] == _T('\n')))
		{
			pStrAN->AppendChar(tcArticleNumber[index]);
		}
	}
}

// Parse a line with fields delimited by strDelimiter.
// Parsing takes place for nField fields. An output array contains multiple string
void ParseString( CString line, CString strDelimiter, int* nField, CStringArray* p_outArray )
{
	ASSERT( nField != NULL && p_outArray != NULL && strDelimiter != CteEMPTY_STRING );
	int pos = 0;
	*nField = 0;
	CString strToken = line.Tokenize( strDelimiter, pos );
	while( pos != -1 )
	{
		strToken.TrimLeft();
		strToken.TrimRight();
		p_outArray->Add( strToken );
		*nField = *nField + 1;
		strToken = line.Tokenize( strDelimiter, pos );
	}
}

// Parse a line with fields delimited by charDelimiter.
// Parsing takes place for nField fields. An output array contains multiple string
void ParseStringByChar( CString line, TCHAR charDelimiter, int* nField, CStringArray* p_outArray )
{
	ASSERT( nField != NULL && p_outArray != NULL && charDelimiter != 0 );
	int pos = 0;
	*nField = 0;
	CString strToken = CteEMPTY_STRING;
	
	while( AfxExtractSubString( strToken, line, pos, charDelimiter ) )
	{
		pos++;
		p_outArray->Add( strToken );
		*nField = *nField + 1;
	}
}

CString CPath::SplitPath( ePathFields epf )
{
	CString str;
	TCHAR tcDrive[_MAX_DRIVE];
	TCHAR tcDirectroy[_MAX_DIR];
	TCHAR tcFileName[_MAX_FNAME];
	TCHAR tcExtension[_MAX_EXT];
	errno_t err = _tsplitpath_s( ( LPCTSTR ) m_FullPath, tcDrive, _MAX_DRIVE, tcDirectroy, _MAX_DIR, tcFileName, _MAX_FNAME, tcExtension, _MAX_EXT );

	if( 0 == err )
	{
		if( epfDrive == ( epf & epfDrive ) )
		{
			str = tcDrive;
		}

		if( epfDir == ( epf & epfDir ) )
		{
			str += tcDirectroy;
		}

		if( epfFName == ( epf & epfFName ) )
		{
			str += tcFileName;
		}

		if( epfExt == ( epf & epfExt ) )
		{
			str += tcExtension;
		}
	}

	return str;
}

CString CPath::GetPath()
{
	return SplitPath( ( ePathFields )( epfDrive | epfDir ) );
}

CString CPath::GetFilename()
{
	return SplitPath( ( ePathFields )( epfFName | epfExt ) );
}
