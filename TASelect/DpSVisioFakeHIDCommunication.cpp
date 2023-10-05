#include "stdafx.h"
#include "DpSVisioIncludes.h"

CDpSVisioFakeHIDCommunication::CDpSVisioFakeHIDCommunication( CWnd *pParent )
	: CDpSVisioBaseCommunication( pParent, CDpSVisioBaseCommunication::CT_HID )
{
	m_uiBytesInRxBuffer = 0;
	m_pclDpSVisioCommand = &m_clDpSVisioHIDCommand;
	m_pclDpSVisioInformation = &m_clDpSVisioFakeInformation;
	m_pclDpSVisioInformation->Reset();
}

CDpSVisioFakeHIDCommunication::~CDpSVisioFakeHIDCommunication()
{
	Reset();
}

void CDpSVisioFakeHIDCommunication::DpSVisioConnected( CString strDeviceName )
{
	m_clDpSVisioFakeInformation.m_bIsConnected = true;

	int iRetry = 0;
	bool bReturn = false;
	do 
	{
		bReturn = RetrieveDpSVisioInformation( strDeviceName );

		if( false == bReturn )
		{
			iRetry++;
			Sleep( 100 );
		}
	}while( false == bReturn && iRetry < 3 );
}

void CDpSVisioFakeHIDCommunication::DpSVisioDisconnected( CString strDeviceName )
{
	Reset();
}

int CDpSVisioFakeHIDCommunication::ReadDpSVisioCommand( CListBox *pOutputList )
{
	if( false == m_clDpSVisioFakeInformation.m_bIsConnected )
	{
		CString str = TASApp.LoadLocalizedString( IDS_DLGDPSVISIO_HIDUPGRADERUNNING_DPSVISIONOTCONNECTED );
		INSERT_STRING_IN_PLIST_AND_SCROLL( pOutputList, str );
		return DPSVISIOCOM_ERROR_HID_SENDCMD_FLAGCONNECTFALSE;
	}

	if( 0 == m_uiBytesInRxBuffer )
	{
		CString str = TASApp.LoadLocalizedString( IDS_DLGDPSVISIO_HIDUPGRADERUNNING_HIDTIMEOUT );
		INSERT_STRING_IN_PLIST_AND_SCROLL( pOutputList, str );
		return DPSVISIOCOM_ERROR_HID_READCMD_HIDTIMEOUT;
	}

	m_clDpSVisioHIDCommand.SetNewBuffer( m_RxBuffer );

	CString str;
	UINT16 uiErrorCommand;
	UINT16 uiErrorCode;

	if( true == m_clDpSVisioHIDCommand.IsError( uiErrorCommand, uiErrorCode ) )
	{
		return DPSVISIOCOM_ERROR_HID_READCMD_HIDNACKRECEIVED;
	}

	return DPSVISIOCOM_NOERROR;
}

int CDpSVisioFakeHIDCommunication::SendDpSVisioCommand( UINT16 uiCommand, UINT8 uiSize, UINT8 *pData, CListBox *pOutputList )
{
	if( false == m_clDpSVisioFakeInformation.m_bIsConnected )
	{
		CString str = TASApp.LoadLocalizedString( IDS_DLGDPSVISIO_HIDUPGRADERUNNING_DPSVISIONOTCONNECTED );
		INSERT_STRING_IN_PLIST_AND_SCROLL( pOutputList, str );
		return DPSVISIOCOM_ERROR_HID_SENDCMD_FLAGCONNECTFALSE;
	}

	memset( m_TxBuffer, 0, USBD_CUSTOMHID_OUTREPORT_SENDSIZE );
	
	m_TxBuffer[0] = 0x00;
	m_TxBuffer[1] = (UINT8)( ( uiCommand & 0xFF00 ) >> 8 );
	m_TxBuffer[2] = (UINT8)( uiCommand & 0x00FF );
	m_TxBuffer[3] = 0x00;

	if( uiSize > 0 )
	{
		if( uiSize > DPSVISIOCMD_DATAMAXSIZE )
		{
			CString str = TASApp.LoadLocalizedString( IDS_DLGDPSVISIO_HIDUPGRADERUNNING_BADPAYLOADSIZE );
			INSERT_STRING_IN_PLIST_AND_SCROLL( pOutputList, str );
			return DPSVISIOCOM_ERROR_HID_SENDCMD_BADPAYLOADSIZE;
		}
		else if( NULL == pData )
		{
			CString str = TASApp.LoadLocalizedString( IDS_DLGDPSVISIO_HIDUPGRADERUNNING_BADPAYLOADPOINTER );
			INSERT_STRING_IN_PLIST_AND_SCROLL( pOutputList, str );
			return DPSVISIOCOM_ERROR_HID_SENDCMD_BADPAYLOADPOINTER;
		}

		m_TxBuffer[3] = uiSize;
		memcpy( &m_TxBuffer[4], pData, uiSize );
	}

	ZeroMemory( m_RxBuffer, USBD_CUSTOMHID_OUTREPORT_BUF_SIZE );

	switch( uiCommand )
	{
		case T_GET_SERIAL_NBR:
			{
				UINT8 arAnswer[] = { 0x80, 0x02, 0x14, 0x30, 0x35, 0x2D, 0x31, 0x37, 0x2D, 0x30, 0x37, 0x2D, 0x31, 0x30, 0x32, 0x00 };
				m_uiBytesInRxBuffer = sizeof( arAnswer );
				memcpy( m_RxBuffer, arAnswer, m_uiBytesInRxBuffer );
			}
			break;

		case UPGFW_STARTSEND:
		case UPGFW_SENDVERSION:
		case UPGFW_SENDCRC:
		case UPGFM_PROGRAM:
		case USBCMD_JUMPTODFU:
			{
				UINT8 arAnswer[] = { 0x80, 0x01, 0x00 };
				m_uiBytesInRxBuffer = sizeof( arAnswer );
				memcpy( m_RxBuffer, arAnswer, m_uiBytesInRxBuffer );
			}
			break;

		case UPGFW_SENDONEPACKET:
			{
				m_clFirmware.Write( pData, uiSize );

				UINT8 arAnswer[] = { 0x80, 0x01, 0x00 };
				m_uiBytesInRxBuffer = sizeof( arAnswer );
				memcpy( m_RxBuffer, arAnswer, m_uiBytesInRxBuffer );
			}
			break;
	}

	return DPSVISIOCOM_NOERROR;
}

int CDpSVisioFakeHIDCommunication::SendAndReadDpSVisioCommand( UINT16 uiCommand, UINT8 uiSize, UINT8 *pData, CListBox *pOutputList )
{
	int iReturnCode = SendDpSVisioCommand( uiCommand, uiSize, pData, pOutputList );

	if( DPSVISIOCOM_NOERROR != iReturnCode )
	{
		return iReturnCode;
	}

	return ReadDpSVisioCommand( pOutputList );
}

void CDpSVisioFakeHIDCommunication::Reset()
{
	CDpSVisioBaseCommunication::Reset();
	m_clDpSVisioFakeInformation.Reset();
}

bool CDpSVisioFakeHIDCommunication::RetrieveDpSVisioInformation( CString strDeviceName )
{
	if( false == m_clDpSVisioFakeInformation.m_bIsConnected )
	{
		return false;
	}

	m_clDpSVisioFakeInformation.m_strIdentifier = _T("DPS-VISIO-1D600AF8");
	m_clDpSVisioFakeInformation.m_strFirmwareRevision = _T("DPS-VISIO SW# 3.0");
	// m_clDpSVisioFakeInformation.m_strFirmwareRevision = _T("DPS-VISIO SW# 2.0");

	int iPos = m_clDpSVisioFakeInformation.m_strFirmwareRevision.ReverseFind( _T('#') );

	if( -1 == iPos )
	{
		m_clDpSVisioFakeInformation.m_strFirmwareRevision = TASApp.LoadLocalizedString( IDS_DSPVISIOCOMMUNICATION_BADFIRMWAREVERSION );
		return false;
	}

	int iRight = m_clDpSVisioFakeInformation.m_strFirmwareRevision.GetLength() - iPos - 1;

	if( iRight < 0 )
	{
		m_clDpSVisioFakeInformation.m_strFirmwareRevision = TASApp.LoadLocalizedString( IDS_DSPVISIOCOMMUNICATION_BADFIRMWAREVERSION );
		return false;
	}

	m_clDpSVisioFakeInformation.m_strShortFirmwareRevision = m_clDpSVisioFakeInformation.m_strFirmwareRevision.Right( iRight );
	m_clDpSVisioFakeInformation.m_strShortFirmwareRevision.Trim();

	TCHAR *pStopString;
	m_clDpSVisioFakeInformation.m_dFirmwareVersion = _tcstod( (LPCTSTR)m_clDpSVisioFakeInformation.m_strShortFirmwareRevision, &pStopString );

	// Get serial number (Available from firmware version 2.1).
	m_clDpSVisioFakeInformation.m_strSerialNumber = _T("-");

	if( m_clDpSVisioFakeInformation.m_dFirmwareVersion > 2.0 )
	{
		if( DPSVISIOCOM_NOERROR != SendAndReadDpSVisioCommand( T_GET_SERIAL_NBR ) )
		{
			return false;
		}

		UINT16 uiCommand;
		UINT16 uiCode;

		if( false == m_clDpSVisioHIDCommand.IsError( uiCommand, uiCode ) )
		{
			m_clDpSVisioFakeInformation.m_strSerialNumber = DpSVisioUtils_BufCharToCString( m_clDpSVisioHIDCommand.GetPayLoadPointer(), m_clDpSVisioHIDCommand.GetPayloadSize() );
		}
		else
		{
			m_clDpSVisioFakeInformation.m_strSerialNumber = _T("Error");
		}
	}

	m_clDpSVisioFakeInformation.m_strDeviceName = strDeviceName.MakeLower();

	return true;
}
