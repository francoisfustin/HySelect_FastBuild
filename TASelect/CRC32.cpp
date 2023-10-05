#include "stdafx.h"

#include <fstream>
#include "CRC32.h"

unsigned int CCRC32::Get_CRC( CString strFileName )
{
	std::ifstream inbuf( (LPCTSTR)strFileName, std::ifstream::in | std::ifstream::binary );

	if( ( inbuf.rdstate() & std::ifstream::failbit ) )
	{
		return 0;
	}

	// Get file size.
	inbuf.seekg( 0, std::ios_base::end );
	UINT uiLength = ( UINT )inbuf.tellg();
	inbuf.seekg( 0 );

	// Goto beginning for reading the file.
	if( uiLength <= 0 )
	{
		ASSERT( 0 );
		inbuf.close();
		return false;
	}

	char *pBuf = new char[uiLength];
	memset( pBuf, 0, uiLength );

	if( NULL == pBuf )
	{
		ASSERT( 0 );
		inbuf.close();
		return false;
	}

	inbuf.read( pBuf, uiLength );
	inbuf.close();
	unsigned int CRC32 = Get_CRC( ( unsigned char * )pBuf, uiLength );
	delete [] pBuf;
	return CRC32;
}

unsigned int CCRC32::Get_CRC( unsigned char *pucData, DWORD dwSize )
{
	// Be sure to use unsigned variables, because negative values introduce high bits
	// where zero bits are required.
	ULONG  ulCRC( 0xffffffff );
	int iLength;
	unsigned char *pucBuffer;

	iLength = dwSize;
	
	// Save the text in the buffer.
	pucBuffer = (unsigned char *)( LPCTSTR )pucData;

	// Perform the algorithm on each character in the string, using the lookup table values.
	while( iLength-- )
	{
		ulCRC = ( ulCRC >> 8 ) ^ m_arulCRC32Table[( ulCRC & 0xFF ) ^ *pucBuffer++];
	}

	// Exclusive OR the result with the beginning value.
	return ulCRC ^ 0xffffffff;
}

void CCRC32::_InitCRC32Table()
{
	// This is the official polynomial used by CRC-32 in PKZip, WinZip and Ethernet.
	ULONG ulPolynomial = 0x04c11db7;

	// 256 values representing ASCII character codes.
	for( int i = 0; i <= 0xFF; i++ )
	{
		m_arulCRC32Table[i] = _Reflect( i, 8 ) << 24;

		for( int j = 0; j < 8; j++ )
		{
			m_arulCRC32Table[i] = ( m_arulCRC32Table[i] << 1 ) ^ ( m_arulCRC32Table[i] & ( 1 << 31 ) ? ulPolynomial : 0 );
		}

		m_arulCRC32Table[i] = _Reflect( m_arulCRC32Table[i], 32 );
	}
}


ULONG CCRC32::_Reflect( ULONG ulReference, char ch )
{
	// Used only by the '_Init_CRC32_Table' method.
	ULONG ulValue( 0 );

	// Swap bit 0 for bit 7
	// bit 1 for bit 6, etc.
	for( int i = 1; i < ( ch + 1 ); i++ )
	{
		if( ulReference & 1 )
		{
			ulValue |= 1 << ( ch - i );
		}

		ulReference >>= 1;
	}

	return ulValue;
}
