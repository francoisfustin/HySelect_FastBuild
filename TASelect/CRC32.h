#pragma once


class CCRC32
{
public:
	CCRC32() { _InitCRC32Table(); }
	virtual ~CCRC32() {}

	// This function uses the crc32_table lookup table to generate a CRC for csData.
	unsigned int Get_CRC( CString strFileName );

	// Creates a CRC from a string buffer.
	unsigned int Get_CRC( unsigned char *pucData, DWORD dwSize );

// Private methods.
private:
	// Call this function only once to initialize the CRC table.
	void _InitCRC32Table();

	// Reflection is a requirement for the official CRC-32 standard.
	// You can create CRCs without it, but they won't conform to the standard.
	ULONG _Reflect( ULONG ulReference, char ch );

// Protected variables.
protected:
protected:
	// Lookup table array.
	ULONG m_arulCRC32Table[256];
};