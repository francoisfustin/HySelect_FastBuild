#include "stdafx.h"
#include "DpSVisioIncludes.h"

void CDpSVisioHIDCommand::SetNewBuffer( UINT8 *pBuffer )
{
	if( NULL == pBuffer )
	{
		return;
	}

	memcpy( m_Buffer, pBuffer, USBD_CUSTOMHID_OUTREPORT_BUF_SIZE );
}

UINT8 CDpSVisioHIDCommand::GetCategoryCommand()
{
	return m_Buffer[DPSVISIOCMD_CMDCATEGINDEX];
}

UINT8 CDpSVisioHIDCommand::GetCommand()
{
	return m_Buffer[DPSVISIOCMD_CMDINDEX];
}

UINT16 CDpSVisioHIDCommand::GetFullCommand()
{
	return ( GetCategoryCommand() << 8 ) + GetCommand();
}

UINT8 CDpSVisioHIDCommand::GetPayloadSize()
{
	return m_Buffer[DPSVISIOCMD_DATALENINDEX];
}

UINT8 *CDpSVisioHIDCommand::GetPayLoadPointer()
{
	return &m_Buffer[DPSVISIOCMD_DATAINDEX];
}

bool CDpSVisioHIDCommand::GetPayloadData( int iIndex, UINT8 *pData, UINT8 uiSize )
{
	if( ( iIndex + uiSize - 1 ) >= GetPayloadSize() || NULL == pData )
	{
		return false;
	}

	memcpy( pData, &m_Buffer[DPSVISIOCMD_DATAINDEX + iIndex], uiSize );
	return true;
}

bool CDpSVisioHIDCommand::IsError( UINT16 &uiCommand, UINT16 &uiCode )
{
	UINT16 uiFullCommand = GetFullCommand();
	bool fReturn = false;

	if( USBCMD_NACK == uiFullCommand )
	{
		memcpy( &uiCommand, &m_Buffer[DPSVISIOCMD_DATAINDEX], DPSVISIOCMD_FULLCMDSIZE );
		DpSVisioUtils_CopyMSBFirst( (UINT8*)&uiCode, &m_Buffer[DPSVISIOCMD_DATAINDEX + DPSVISIOCMD_FULLCMDSIZE], DPSVISIOERROR_SIZE );
		fReturn = true;
	}

	return fReturn;
}
