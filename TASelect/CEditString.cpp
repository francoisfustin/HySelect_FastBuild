#include "stdafx.h"
#include "TASelect.h"

#include "CEditString.h"


CNumString::CNumString()
{
	m_ePhysType = _U_NODIM;
	m_PhysTypeStr = _T( "" );
	m_bPhysTypeStr = false;
	m_bUnitsUsed = false;
	m_EditType = eDouble;
	m_EditSign = eBoth;
	m_dMinDblValue = -DBL_MAX;
	m_dMaxDblValue = DBL_MAX;
	m_dCurrentValSI = 0.0;
	m_iMinIntValue = -INT_MAX;
	m_iMaxIntValue = INT_MAX;
}

CString CNumString::FormatChar( UINT uiChar, CString strOldStr, CString strNewStr, int *piPosCursor )
{
	if( NULL == piPosCursor )
	{
		return strOldStr;
	}

	// Variables.
	CString strLastChar;
	int iPosEndNum = -1;
	double dValue = 0.0;
	bool bCancelChar = false;

	// Special treatment to accept or not the input of a "-" char.
	if( ePositive == m_EditSign && 45 == uiChar )
	{
		if( eDouble == m_EditType || eAll == m_EditType )
		{
			// Allow "-" char to be added after the "e" char
			if( ( *piPosCursor ) - 2 >= 0 )
			{
				if( strNewStr.GetAt( ( *piPosCursor ) - 2 ) != CString( _T("e") ) )
				{
					bCancelChar = true;
				}
			}
			else
			{
				bCancelChar = true;
			}
		}
		else if( eINT == m_EditType )
		{
			bCancelChar = true;
		}
	}

	if( true == bCancelChar )
	{
		// Delete the last character and decrement the cursor's position.
		--( *piPosCursor );
		return strOldStr;
	}

	// If units exist in new string remove them and define its new length.
	strNewStr.MakeLower();
	iPosEndNum = strNewStr.Find( _T(" ") );

	if( iPosEndNum != -1 )
	{
		strNewStr = strNewStr.Left( iPosEndNum );
	}

	int iLength = strNewStr.GetLength() - 1;

	switch( m_EditType )
	{
		case eAll:
			break;

		case eINT:
			// Accept only digit.
			if( uiChar < 48 || uiChar > 57 )
			{
				bCancelChar = true;
			}

			if( iLength > 0 && 0 == _ttoi( strOldStr ) )
			{
				bCancelChar = true;
			}

			break;

		case eDouble:
			if( 0 == iLength )
			{
				// Do not allow "e" or "E" to be added as first number.
				// 43 = '+'; 44 = ','; 45 = '-'; 46 = '.'.
				if( uiChar != 8 && uiChar != 43 && uiChar != 44 && uiChar != 45 && uiChar != 46 )
				{
					if( RD_NOT_NUMBER == ReadDouble( strNewStr, &dValue ) )
					{
						bCancelChar = true;
					}
				}
			}
			else
			{
				// Get the last character input and verify
				// if it is allow to be written in the edit box
				char cChar = ( char )uiChar;
				strLastChar.Format( _T("%c"), cChar );

				//If the user push one of the following button,
				// it will not be considered as a character :
				// "<--" : 8				// "."   : 46
				// "+"   : 43				// "E"   : 69
				// ","   : 44				// "e"   : 101
				// "-"   : 45
				if( uiChar != 8 && uiChar != 43 && uiChar != 44 && uiChar != 45 && uiChar != 46 && uiChar != 69 && uiChar != 101 )
				{
					if( RD_NOT_NUMBER == ReadDouble( strLastChar, &dValue ) )
					{
						bCancelChar = true;
					}
				}

				// Verify the special characters were not already implemented
				if( 8 == uiChar || 43 == uiChar || 44 == uiChar || 45 == uiChar || 46 == uiChar || 69 == uiChar || 101 == uiChar )
				{
					// Allow "-" or "+" at the beginning and just after "e"
					if( 43 == uiChar || 45 == uiChar )
					{
						// Verify if the cursor is positionned at the beginning
						// If it is the case, do not make the test, otherwize
						// we will go outside the buffer
						if( ( *piPosCursor ) - 2 >= 0 && ( *piPosCursor ) - 2 < iLength )
						{
							if( strNewStr.GetAt( ( *piPosCursor ) - 2 ) != CString( _T("e") ) )
							{
								bCancelChar = true;

							}
						}
					}
					else if( 101 == uiChar || 69 == uiChar )
					{
						// Do not allow to add "e" or "E" at the beginning
						// when other numbers are already inserted
						int iPosPoint = strOldStr.Find( _T(".") );

						if( iPosPoint != -1 )
						{
							if( iPosPoint > ( int )( ( *piPosCursor ) - 1 ) )
							{
								bCancelChar = true;
							}
						}

						if( strOldStr.Find( strLastChar ) != -1 )
						{
							bCancelChar = true;
						}

						// Do not allow to add manually a "e"
						// at the first place when other
						//numbers already exist
						if( strNewStr.GetAt( 0 ) == CString( _T("e") ) )
						{
							bCancelChar = true;
						}
					}
					else if( 44 == uiChar || 46 == uiChar )
					{
						// Do not allow to add "." or "," after "e"
						int iPosE = strOldStr.Find( _T("e") );

						if( iPosE != -1 )
						{
							if( iPosE < ( int )( ( *piPosCursor ) - 1 ) )
							{
								bCancelChar = true;
							}
						}

						// Do not allow to add a second time the "," or "." character.
						if( strOldStr.Find( _T("." ) ) != -1 || strOldStr.Find( _T( ",") ) != -1 )
						{
							bCancelChar = true;
						}
					}
				}
			}
			break;
	}

	if( eAll != m_EditType )
	{
		// Verify we do not go higher than the min or max values.
		ReadDouble( strNewStr, &dValue );
		double dValSI = 0.0;
		dValSI = CDimValue::CUtoSI( m_ePhysType, dValue );

		if( eINT == m_EditType && ( (int)dValSI < m_iMinIntValue || (int)dValSI  > m_iMaxIntValue ) )
		{
			bCancelChar = true;
		}

		if( true == bCancelChar )
		{
			// Delete the last character and decrement the cursor's position.
			--( *piPosCursor );
			return strOldStr;
		}

		// Format the string that will be displayed in the edit and restitute the cursor's position.
		// Do not use the 'WriteCUDouble' because the numerical value can be not finished yet and needs inputs.
		// Example : "2.5e" will be converted into "2.5" with 'WriteCUDouble'.
		
		if( true == m_bUnitsUsed )
		{
			CUnitDatabase *pUnitDB = CDimValue::AccessUDB();
			strNewStr += _T(" ");

			// Verify if the user use Physical type
			// or a special string like "%"
			if( false == m_bPhysTypeStr )
			{
				strNewStr += pUnitDB->GetNameOfDefaultUnit( m_ePhysType ).c_str();
			}
			else
			{
				strNewStr += m_PhysTypeStr;
			}
		}
	}

	return strNewStr;
}

LPCTSTR CNumString::GetCurrentString()
{
	// Verify if the user use Physical type or a special string like "%".
	if( false == m_bPhysTypeStr )
	{
		return WriteCUDouble( m_ePhysType, m_dCurrentValSI, m_bUnitsUsed );
	}

	static _string str;
	str = WriteDouble( m_dCurrentValSI, 3, 0, 1 );
	str += _T(" ") + m_PhysTypeStr;
	return str.c_str();
}

CString CNumString::Update()
{
	CString str = WriteCUDouble( m_ePhysType, m_dCurrentValSI, m_bUnitsUsed );
	return str;
}

/////////////////////////////////////////////////////////////////////////////
// CCbiRestrString


CCbiRestrString::CCbiRestrString()
{
}

bool CCbiRestrString::CheckCBICharSet( TCHAR zechar )
{
	// CBICharSet 'a'->'z' & '<'->'Z' & '*'->':'
	const CString CBICharSet = _T(" {}'&%$!§^üÜäÄöÖø\r\"");

	if( ( CBICharSet.Find( zechar ) > -1 ) ||
		( zechar >= 'a' && zechar <= 'z' ) ||
		( zechar >= '<' && zechar <= 'Z' ) ||
		( zechar >= '*' && zechar <= ':' ) )
	{
		return true;
	}

	return false;
}
bool CCbiRestrString::CheckCBICharSet( CString *pstr )
{
	// Verify content of CEdit with authorized CBI characters
	bool flag = true;

	for( int i = 0; i < pstr->GetLength(); i++ )
	{
		TCHAR zechar = ( *pstr )[i];

		if( CheckCBICharSet( ( TCHAR )( *pstr )[i] ) )
		{
			continue;
		}

		// Character non authorized
		// AfxMessage Box
		pstr->Remove( zechar );
		// Restart scanning
		i = min( 0, i - 1 );
		flag = false;
	}

	return flag;
}
