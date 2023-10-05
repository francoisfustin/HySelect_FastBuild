#include "stdafx.h"
#include "DpSVisioIncludes.h"

CDpSVisioHIDCommunication::CDpSVisioHIDCommunication( CWnd *pParent )
	: CDpSVisioBaseCommunication( pParent, CDpSVisioBaseCommunication::CT_HID )
{
 	m_pHIDDpSVisio = NULL;
 	
	m_pclDpSVisioCommand = &m_clDpSVisioHIDCommand;
	m_pclDpSVisioInformation = &m_clDpSVisioHIDInformation;
	m_pclDpSVisioInformation->Reset();
}

CDpSVisioHIDCommunication::~CDpSVisioHIDCommunication()
{
	Reset();
}

void CDpSVisioHIDCommunication::DpSVisioConnected( CString strDeviceName )
{
	m_clDpSVisioHIDInformation.m_bIsConnected = true;
	m_pHIDDpSVisio = hid_open( HIDDEVICE_VID_VALUE, HIDDEVICE_PID_VALUE, NULL );

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

void CDpSVisioHIDCommunication::DpSVisioDisconnected( CString strDeviceName )
{
	Reset();
}

int CDpSVisioHIDCommunication::ReadDpSVisioCommand( CListBox *pOutputList )
{
	if( NULL == m_pHIDDpSVisio || false == m_clDpSVisioHIDInformation.m_bIsConnected )
	{
		CString str = TASApp.LoadLocalizedString( IDS_DLGDPSVISIO_HIDUPGRADERUNNING_DPSVISIONOTCONNECTED );
		INSERT_STRING_IN_PLIST_AND_SCROLL( pOutputList, str );

		if( NULL == m_pHIDDpSVisio )
		{
			return DPSVISIOCOM_ERROR_HID_SENDCMD_HIDHANDLENULL;
		}
		else
		{
			return DPSVISIOCOM_ERROR_HID_SENDCMD_FLAGCONNECTFALSE;
		}

	}

	int iBytesRead = hid_read_timeout( m_pHIDDpSVisio, m_RxBuffer, USBD_CUSTOMHID_OUTREPORT_BUF_SIZE, 10000 );

	if( 0 == iBytesRead )
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

int CDpSVisioHIDCommunication::SendDpSVisioCommand( UINT16 uiCommand, UINT8 uiSize, UINT8 *pData, CListBox *pOutputList )
{
	if( NULL == m_pHIDDpSVisio || false == m_clDpSVisioHIDInformation.m_bIsConnected )
	{
		CString str = TASApp.LoadLocalizedString( IDS_DLGDPSVISIO_HIDUPGRADERUNNING_DPSVISIONOTCONNECTED );
		INSERT_STRING_IN_PLIST_AND_SCROLL( pOutputList, str );

		if( NULL == m_pHIDDpSVisio )
		{
			return DPSVISIOCOM_ERROR_HID_SENDCMD_HIDHANDLENULL;
		}
		else
		{
			return DPSVISIOCOM_ERROR_HID_SENDCMD_FLAGCONNECTFALSE;
		}
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

	int iResult = hid_write( m_pHIDDpSVisio, m_TxBuffer, USBD_CUSTOMHID_OUTREPORT_SENDSIZE );

	if( iResult < 0 )
	{
		CString str = TASApp.LoadLocalizedString( IDS_DLGDPSVISIO_HIDUPGRADERUNNING_SENDREPORTERROR );
		INSERT_STRING_IN_PLIST_AND_SCROLL( pOutputList, str );
		return DPSVISIOCOM_ERROR_HID_SENDCMD_HIDREPORTERROR;
	}

	return DPSVISIOCOM_NOERROR;
}

int CDpSVisioHIDCommunication::SendAndReadDpSVisioCommand( UINT16 uiCommand, UINT8 uiSize, UINT8 *pData, CListBox *pOutputList )
{
	int iReturnCode = SendDpSVisioCommand( uiCommand, uiSize, pData, pOutputList );

	if( DPSVISIOCOM_NOERROR != iReturnCode )
	{
		return iReturnCode;
	}

	return ReadDpSVisioCommand( pOutputList );
}

void CDpSVisioHIDCommunication::Reset()
{
	CDpSVisioBaseCommunication::Reset();

	if( NULL != m_pHIDDpSVisio )
	{
		hid_close( m_pHIDDpSVisio );
		hid_exit();
		m_pHIDDpSVisio = NULL;
	}

	m_clDpSVisioHIDInformation.Reset();
}

bool CDpSVisioHIDCommunication::RetrieveDpSVisioInformation( CString strDeviceName )
{
	if( NULL == m_pHIDDpSVisio || false == m_clDpSVisioHIDInformation.m_bIsConnected )
	{
		return false;
	}

	UINT16 uiCommand;
	UINT16 uiCode;

	wchar_t wstr[255];
	wstr[0] = 0x0000;
	
	if( -1 == hid_get_product_string( m_pHIDDpSVisio, wstr, 255 ) )
	{
		return false;	
	}
	
	m_clDpSVisioHIDInformation.m_strIdentifier = CString( wstr );
	
	wstr[0] = 0x0000;

	if( -1 == hid_get_serial_number_string( m_pHIDDpSVisio, wstr, 255 ) )
	{
		return false;
	}

	m_clDpSVisioHIDInformation.m_strFirmwareRevision = CString( wstr );

	int iPos = m_clDpSVisioHIDInformation.m_strFirmwareRevision.ReverseFind( _T('#') );

	if( -1 == iPos )
	{
		m_clDpSVisioHIDInformation.m_strFirmwareRevision = TASApp.LoadLocalizedString( IDS_DSPVISIOCOMMUNICATION_BADFIRMWAREVERSION );
		return false;
	}

	int iRight = m_clDpSVisioHIDInformation.m_strFirmwareRevision.GetLength() - iPos - 1;

	if( iRight < 0 )
	{
		m_clDpSVisioHIDInformation.m_strFirmwareRevision = TASApp.LoadLocalizedString( IDS_DSPVISIOCOMMUNICATION_BADFIRMWAREVERSION );
		return false;
	}

	m_clDpSVisioHIDInformation.m_strShortFirmwareRevision = m_clDpSVisioHIDInformation.m_strFirmwareRevision.Right( iRight );
	m_clDpSVisioHIDInformation.m_strShortFirmwareRevision.Trim();

	TCHAR *pStopString;
	m_clDpSVisioHIDInformation.m_dFirmwareVersion = _tcstod( (LPCTSTR)m_clDpSVisioHIDInformation.m_strShortFirmwareRevision, &pStopString );

	// Get serial number (Available from firmware version 2.1).
	m_clDpSVisioHIDInformation.m_strSerialNumber = _T("-");

	if( m_clDpSVisioHIDInformation.m_dFirmwareVersion > 2.0 )
	{
		if( DPSVISIOCOM_NOERROR != SendAndReadDpSVisioCommand( T_GET_SERIAL_NBR ) )
		{
			return false;
		}

		if( false == m_clDpSVisioHIDCommand.IsError( uiCommand, uiCode ) )
		{
			m_clDpSVisioHIDInformation.m_strSerialNumber = DpSVisioUtils_BufCharToCString( m_clDpSVisioHIDCommand.GetPayLoadPointer(), m_clDpSVisioHIDCommand.GetPayloadSize() );
		}
		else
		{
			m_clDpSVisioHIDInformation.m_strSerialNumber = _T("Error");
		}
	}

	m_clDpSVisioHIDInformation.m_strDeviceName = strDeviceName.MakeLower();

	return true;
}
