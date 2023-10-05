#pragma once


#define RET_OK		0
#define	RET_FAULT 	1

CString DpSVisioUtils_BufCharToCString( UINT8 *pSrc, UINT8 uiSize );
UINT8 *DpSVisioUtils_CopyMSBFirst( UINT8 *pDst, UINT8 *pSrc, UINT8 uiSize );

// Temporary CRC computing coming from the PC SliderTools program.
typedef enum
{
	Zeros,
	NonZero1 = 0xffff,
	NonZero2 = 0x1D0F
}InitialCrcValue;

void DpSVisioUtils_crc_init( InitialCrcValue initialValue );
UINT16 DpSVisioUtils_crc_ComputeCRC( UINT8 *pBuffer, UINT32 uiLength );
UINT16 DpSVisioUtils_crc_CumulativeComputeCRC( UINT16 uiPreviousCRC, UINT8 *pBuffer, UINT32 uiLength );
u_long DpSVisioUtils_crc32(char *data, int len);
