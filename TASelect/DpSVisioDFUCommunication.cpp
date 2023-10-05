#include "stdafx.h"
#include "DpSVisioIncludes.h"

CDpSVisioDFUCommunication::CDpSVisioDFUCommunication( CWnd *pParent )
	: CDpSVisioBaseCommunication( pParent, CDpSVisioBaseCommunication::CT_DFU )
{
	m_pclDpSVisioCommand = &m_clDpSVisioDFUCommand;
	m_pclDpSVisioInformation = &m_clDpSVisioDFUInformation;
	m_pclDpSVisioInformation->Reset();
	m_hDFUImage = INVALID_HANDLE_VALUE;
	m_pMapping = NULL;
	m_dwNbAlternates = 0;
	m_dwOperationCode = 0;
}

CDpSVisioDFUCommunication::~CDpSVisioDFUCommunication()
{
	Reset();
}

void CDpSVisioDFUCommunication::DpSVisioConnected( CString strDeviceName )
{
	m_clDpSVisioDFUInformation.m_bIsConnected = true;

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

void CDpSVisioDFUCommunication::DpSVisioDisconnected( CString strDeviceName )
{
	Reset();
}

int CDpSVisioDFUCommunication::SendDpSVisioCommandLP( UINT16 uiCommand, LPARAM lpParam, CListBox *pOutputList )
{
	int iReturn = -1;

	switch( uiCommand )
	{
		case DFUCMD_STARTERASE:
			iReturn = _DFUStartErase( (CString*)lpParam, pOutputList );
			break;

		case DFUCMD_CHECKERASE:
			iReturn = _DFUCheckErase( (BYTE*)lpParam, pOutputList );
			break;

		case DFUCMD_STARTUPGRADE:
			iReturn = _DFUStartUpgrade( pOutputList );
			break;

		case DFUCMD_CHECKUPGRADE:
			iReturn = _DFUCheckUpgrade( (BYTE*)lpParam, pOutputList );
			break;

		case DFUCMD_DFUDETACH:
			iReturn = _DFUDetach( pOutputList );
			break;

		default:
			iReturn = DPVSISIOCOM_ERROR_DFU_SENDCMD_BADCOMMAND;
			break;
	}

	return iReturn;
}

void CDpSVisioDFUCommunication::Reset()
{
	CDpSVisioBaseCommunication::Reset();
	
	if( INVALID_HANDLE_VALUE != m_hDFUImage )
	{
		STDFUFILES_DestroyImage( &m_hDFUImage );
		m_hDFUImage = INVALID_HANDLE_VALUE;
	}

	if( NULL != m_pMapping )
	{
		STDFUPRT_DestroyMapping( &m_pMapping );
		m_pMapping = NULL;
	}

	m_dwNbAlternates = 0;
	m_clDpSVisioDFUInformation.Reset();
}

bool CDpSVisioDFUCommunication::RetrieveDpSVisioInformation( CString strDeviceName )
{
	HANDLE hDle;
	USES_CONVERSION;

	if( STDFU_NOERROR != STDFU_Open( CT2A( (LPCTSTR)strDeviceName ), &hDle ) )
	{
		return false;
	}

	USB_DEVICE_DESCRIPTOR DeviceDescriptor;

	if( STDFU_NOERROR != STDFU_GetDeviceDescriptor( &hDle, &DeviceDescriptor ) )
	{
		STDFU_Close( &hDle );
		return false;
	}

	// Get its attributes.
	UINT Dummy1, Dummy2;
	DFU_FUNCTIONAL_DESCRIPTOR DeviceDFUDescriptor;
	memset( &DeviceDFUDescriptor, 0, sizeof( DeviceDFUDescriptor ) );

	if( STDFUPRT_NOERROR != STDFU_GetDFUDescriptor( &hDle, &Dummy1, &Dummy2, &DeviceDFUDescriptor ) )
	{
		STDFU_Close( &hDle );
		return false;
	}

	if( ( DeviceDFUDescriptor.bcdDFUVersion < 0x011A ) || ( DeviceDFUDescriptor.bcdDFUVersion >= 0x0120 ) )
	{
		// Bad DFU version.
		STDFU_Close( &hDle );
		return false;
	}
	
	// Tries to get the mapping.
	PUSB_DEVICE_DESCRIPTOR Desc = NULL;

	if( STDFUPRT_NOERROR != STDFUPRT_CreateMappingFromDevice( CT2A( (LPCTSTR)strDeviceName ), &m_pMapping, &m_dwNbAlternates ) )
	{
		// Can't read memory mapping.
		STDFU_Close( &hDle );
		return false;
	}

	STDFU_Close( &hDle );
	m_clDpSVisioDFUInformation.m_strDeviceName = strDeviceName.MakeLower();

	return true;
}

int CDpSVisioDFUCommunication::_DFUStartErase( CString *pstrDFUFileName, CListBox *pOutputList )
{
	if( NULL == pstrDFUFileName || TRUE == pstrDFUFileName->IsEmpty() )
	{
		CString str = TASApp.LoadLocalizedString( IDS_DLGDPSVISIO_DFUUPGRADERUNNING_DFUFILENAMEEMPTY );
		INSERT_STRING_IN_PLIST_AND_SCROLL( pOutputList, str );

		if( NULL == pstrDFUFileName )
		{
			return DPVSISIOCOM_ERROR_DFU_STARTERASE_DFUFILENAMEPOINTERNULL;
		}
		else
		{
			return DPVSISIOCOM_ERROR_DFU_STARTERASE_DFUFILENAMEEMPTY;
		}
	}

	CString strError;
	CString str;
	HANDLE hFile;
	BYTE NbTargets;
	USES_CONVERSION;

	DWORD dwRet = STDFUFILES_OpenExistingDFUFile( CT2A( (LPCTSTR)*pstrDFUFileName ), &hFile, NULL, NULL, NULL, &NbTargets );

	if( STDFUFILES_NOERROR != dwRet )
	{
		// "Can't open the DFU file."
		str = TASApp.LoadLocalizedString( IDS_DLGDPSVISIO_DFUUPGRADERUNNING_CANTOPENDFUFILE );
		INSERT_STRING_IN_PLIST_AND_SCROLL( pOutputList, str );

		strError.Format( _T("0x%08X"), dwRet );
		FormatString( str, IDS_DLGDPSVISIO_DFUUPGRADERUNNING_ERRORCODE, strError );
		INSERT_STRING_IN_PLIST_AND_SCROLL( pOutputList, str );

		return dwRet;
	}

	HANDLE hImage;
	dwRet = STDFUFILES_ReadImageFromDFUFile( hFile, 0, &hImage );

	if( STDFUFILES_NOERROR != dwRet )
	{
		// Can't read the DFU file.
		str = TASApp.LoadLocalizedString( IDS_DLGDPSVISIO_DFUUPGRADERUNNING_CANTREADDFU );
		INSERT_STRING_IN_PLIST_AND_SCROLL( pOutputList, str );

		strError.Format( _T("0x%08X"), dwRet );
		FormatString( str, IDS_DLGDPSVISIO_DFUUPGRADERUNNING_ERRORCODE, strError );
		INSERT_STRING_IN_PLIST_AND_SCROLL( pOutputList, str );

		return dwRet;
	}

	STDFUFILES_CloseDFUFile( hFile );

	DFUThreadContext Context;
	lstrcpyA( Context.szDevLink, CT2A( (LPCTSTR)m_pclDpSVisioInformation->m_strDeviceName ) );

	Context.DfuGUID = GUID_DFUSE;
	Context.AppGUID = GUID_HID;
	Context.Operation = OPERATION_ERASE;
	Context.bDontSendFFTransfersForUpgrade = FALSE;

	// Let's backup our data before the filtering for erase. The data will be used for the upgrade phase.
	STDFUFILES_DuplicateImage( hImage, &m_hDFUImage );
	STDFUFILES_FilterImageForOperation( m_hDFUImage, m_pMapping, OPERATION_UPGRADE, FALSE );

	STDFUFILES_FilterImageForOperation( hImage, m_pMapping, OPERATION_ERASE, FALSE );
	Context.hImage = hImage;

	dwRet = STDFUPRT_LaunchOperation( &Context, &m_dwOperationCode );

	if( STDFUFILES_NOERROR != dwRet )
	{
		// Can't start DFU upgrade.
		str = TASApp.LoadLocalizedString( IDS_DLGDPSVISIO_DFUUPGRADERUNNING_CANTSTARTDFUUPGRADE );
		INSERT_STRING_IN_PLIST_AND_SCROLL( pOutputList, str );

		strError.Format( _T("0x%08X"), dwRet );
		FormatString( str, IDS_DLGDPSVISIO_DFUUPGRADERUNNING_ERRORCODE, strError );
		INSERT_STRING_IN_PLIST_AND_SCROLL( pOutputList, str );
		return dwRet;
	}

	return DPSVISIOCOM_NOERROR;
}

int CDpSVisioDFUCommunication::_DFUCheckErase( BYTE *pPercentCompleted, CListBox *pOutputList )
{
	if( NULL == pPercentCompleted )
	{
		return DPVSISIOCOM_ERROR_DFU_CHECKERASE_PERCENTPOINTERNULL;
	}
		
	if( 0 == m_dwOperationCode )
	{
		return DPVSISIOCOM_ERROR_DFU_CHECKERASE_NOOPERATIONRUNNING;
	}

	// Get the operation status.
	DFUThreadContext Context;
	STDFUPRT_GetOperationStatus( m_dwOperationCode, &Context );

	if( STDFUPRT_NOERROR != Context.ErrorCode )
	{
		STDFUPRT_StopOperation( m_dwOperationCode, &Context );
		STDFUFILES_DestroyImage( &Context.hImage );
		m_dwOperationCode = 0;

		CString str = TASApp.LoadLocalizedString( IDS_DLGDPSVISIO_DFUUPGRADERUNNING_ERASEERROR );
		INSERT_STRING_IN_PLIST_AND_SCROLL( pOutputList, str );

		CString strError;
		strError.Format( _T("0x%08X"), Context.ErrorCode );
		FormatString( str, IDS_DLGDPSVISIO_DFUUPGRADERUNNING_ERRORCODE, strError );
		INSERT_STRING_IN_PLIST_AND_SCROLL( pOutputList, str );

		return (int)Context.ErrorCode;
	}

	*pPercentCompleted = Context.Percent;
	return DPSVISIOCOM_NOERROR;
}

int CDpSVisioDFUCommunication::_DFUStartUpgrade( CListBox *pOutputList )
{
	if( 0 == m_dwOperationCode )
	{
		return DPVSISIOCOM_ERROR_DFU_STARTUPGRADE_NOOPERATIONRUNNING;
	}

	// After the erase, relaunch the upgrade phase.
	DFUThreadContext Context;
	STDFUPRT_StopOperation( m_dwOperationCode, &Context );
	STDFUFILES_DestroyImage( &Context.hImage );

	Context.Operation = OPERATION_UPGRADE;
	Context.hImage = m_hDFUImage;
	DWORD dwRet = STDFUPRT_LaunchOperation( &Context, &m_dwOperationCode );
	
	if( dwRet != STDFUPRT_NOERROR )
	{
		// Can't upgrade the DpS-Visio.
		CString str = TASApp.LoadLocalizedString( IDS_DLGDPSVISIO_DFUUPGRADERUNNING_CANTSTARTDFUUPGRADE );
		INSERT_STRING_IN_PLIST_AND_SCROLL( pOutputList, str );

		CString strError;
		strError.Format( _T("0x%08X"), dwRet );
		FormatString( str, IDS_DLGDPSVISIO_DFUUPGRADERUNNING_ERRORCODE, strError );
		INSERT_STRING_IN_PLIST_AND_SCROLL( pOutputList, str );

		return dwRet;
	}

	return DPSVISIOCOM_NOERROR;
}

int CDpSVisioDFUCommunication::_DFUCheckUpgrade( BYTE *pPercentCompleted, CListBox *pOutputList )
{
	if( NULL == pPercentCompleted )
	{
		return DPVSISIOCOM_ERROR_DFU_CHECKUPGRADE_PERCENTPOINTERNULL;
	}
		
	if( 0 == m_dwOperationCode )
	{
		return DPVSISIOCOM_ERROR_DFU_CHECKUPGRADE_NOOPERATIONRUNNING;
	}

	// Get the operation status.
	DFUThreadContext Context;
	STDFUPRT_GetOperationStatus( m_dwOperationCode, &Context );

	if( STDFUPRT_NOERROR != Context.ErrorCode )
	{
		STDFUPRT_StopOperation( m_dwOperationCode, &Context );
		STDFUFILES_DestroyImage( &Context.hImage );
		m_dwOperationCode = 0;

		CString str = TASApp.LoadLocalizedString( IDS_DLGDPSVISIO_DFUUPGRADERUNNING_ERASEERROR );
		INSERT_STRING_IN_PLIST_AND_SCROLL( pOutputList, str );

		CString strError;
		strError.Format( _T("0x%08X"), Context.ErrorCode );
		FormatString( str, IDS_DLGDPSVISIO_DFUUPGRADERUNNING_ERRORCODE, strError );
		INSERT_STRING_IN_PLIST_AND_SCROLL( pOutputList, str );

		return (int)Context.ErrorCode;
	}

	*pPercentCompleted = Context.Percent;

	if( 100 == Context.Percent )
	{
		STDFUPRT_StopOperation( m_dwOperationCode, &Context );
		m_dwOperationCode = 0;
		m_hDFUImage = INVALID_HANDLE_VALUE;
	}

	return DPSVISIOCOM_NOERROR;
}

int CDpSVisioDFUCommunication::_DFUDetach( CListBox *pOutputList )
{
	USB_DEVICE_DESCRIPTOR *pDesc = new USB_DEVICE_DESCRIPTOR;

	if( NULL == pDesc )
	{
		return -1;
	}

	DFUThreadContext Context;
	lstrcpyA( Context.szDevLink, CT2A( (LPCTSTR)m_pclDpSVisioInformation->m_strDeviceName ) );
	
	Context.DfuGUID = GUID_DFUSE;
	Context.AppGUID = GUID_HID;
	Context.Operation = OPERATION_DETACH;
	Context.hImage = NULL;

	/*
	Context.Operation = OPERATION_RETURN;
	
	HANDLE hImage;
	STDFUFILES_CreateImageFromMapping( &hImage, m_pMapping );
	STDFUFILES_SetImageName( hImage, CT2A( (LPCTSTR)m_pMapping[0].Name ) );
	STDFUFILES_FilterImageForOperation( hImage, m_pMapping, OPERATION_RETURN, FALSE );
	Context.hImage = hImage;
	*/

	DWORD dwRet = STDFUPRT_LaunchOperation( &Context, &m_dwOperationCode );

	if( STDFUPRT_NOERROR != dwRet )
	{
		CString str = TASApp.LoadLocalizedString( IDS_DLGDPSVISIO_DFUUPGRADERUNNING_DFUCANTDETACH );
		INSERT_STRING_IN_PLIST_AND_SCROLL( pOutputList, str );

		CString strError;
		strError.Format( _T("0x%08X"), dwRet );
		FormatString( str, IDS_DLGDPSVISIO_DFUUPGRADERUNNING_ERRORCODE, strError );
		INSERT_STRING_IN_PLIST_AND_SCROLL( pOutputList, str );

		return -1;
	}

	return DPSVISIOCOM_NOERROR;
}
