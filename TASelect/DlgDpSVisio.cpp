#include "stdafx.h"


#include "afxdialogex.h"
#include "SETUPAPI.H"
#include "TAScopeUtil.h"
#include "DpSVisioIncludes.h"
#include "DlgDpSVisio.h"


IMPLEMENT_DYNAMIC( CDlgDpSVisio, CDialogEx )

CDlgDpSVisio::CDlgDpSVisio( CWnd *pParent )
	: CDialogEx( CDlgDpSVisio::IDD, pParent )
{
	m_pCurrentDialog = NULL;
	m_eCurrentDlg = Dialog_None;
	m_pclDpSVisioCommunication = NULL;
	InitializeCriticalSection( &m_CriticalSection );
	m_nTimer = (UINT_PTR)0;
	m_hLeftPanelNormal = (HBITMAP)LoadImage( AfxGetInstanceHandle(), MAKEINTRESOURCE( IDB_DPSVISIOLEFTBMPSTATUS ), IMAGE_BITMAP, 185, 305, LR_DEFAULTCOLOR );
	m_hLeftPanelUpdateRunning = (HBITMAP)LoadImage( AfxGetInstanceHandle(), MAKEINTRESOURCE( IDB_DPSVISIOLEFTBMPUPGRADERUNNING ), IMAGE_BITMAP, 185, 305, LR_DEFAULTCOLOR );
	m_hLeftPanelUpdateFinishedOK = (HBITMAP)LoadImage( AfxGetInstanceHandle(), MAKEINTRESOURCE( IDB_DPSVISIOLEFTBMPUPGRADEFINNISHEDOK ), IMAGE_BITMAP, 185, 305, LR_DEFAULTCOLOR );
	m_hLeftPanelUpdateFinishedError = (HBITMAP)LoadImage( AfxGetInstanceHandle(), MAKEINTRESOURCE( IDB_DPSVISIOLEFTBMPUPGRADEFINNISHEDERROR ), IMAGE_BITMAP, 185, 305, LR_DEFAULTCOLOR );

	m_rUpgradeStatus.m_fUpgradeStatus = true;
	m_rUpgradeStatus.m_eUpgradeError = UE_NoError;
	m_rUpgradeStatus.m_dwDFUInternalErrorCode = -1;

	m_arsFirmwareData[Device_HID].m_InterfaceClassGuid = GUID_HID;
	m_arsFirmwareData[Device_HID].m_hDevNotify = NULL;
	m_arsFirmwareData[Device_HID].m_uiRevision = 0;
	m_arsFirmwareData[Device_HID].m_strRevision = _T("");
	m_arsFirmwareData[Device_HID].m_strFilename = _T("");
	m_arsFirmwareData[Device_HID].m_strError = _T("");

	m_arsFirmwareData[Device_DFUSe].m_InterfaceClassGuid = GUID_DFUSE;
	m_arsFirmwareData[Device_DFUSe].m_hDevNotify = NULL;
	m_arsFirmwareData[Device_DFUSe].m_uiRevision = 0;
	m_arsFirmwareData[Device_DFUSe].m_strRevision = _T("");
	m_arsFirmwareData[Device_DFUSe].m_strFilename = _T("");
	m_arsFirmwareData[Device_DFUSe].m_strError = _T("");

	m_arsFirmwareData[Device_NewDFUSe].m_InterfaceClassGuid = GUID_NEWDFUSE;
	m_arsFirmwareData[Device_NewDFUSe].m_hDevNotify = NULL;
	m_arsFirmwareData[Device_NewDFUSe].m_uiRevision = 0;
	m_arsFirmwareData[Device_NewDFUSe].m_strRevision = _T("");
	m_arsFirmwareData[Device_NewDFUSe].m_strFilename = _T("");
	m_arsFirmwareData[Device_NewDFUSe].m_strError = _T("");

	m_pUSBContext = NULL;

	m_rNewDFUSeVariables.dfu_root = NULL;
	m_rNewDFUSeVariables.match_path = NULL;
	m_rNewDFUSeVariables.match_vendor = -1;
	m_rNewDFUSeVariables.match_product = -1;
	m_rNewDFUSeVariables.match_vendor_dfu = -1;
	m_rNewDFUSeVariables.match_product_dfu = -1;
	m_rNewDFUSeVariables.match_config_index = -1;
	m_rNewDFUSeVariables.match_iface_index = -1;
	m_rNewDFUSeVariables.match_iface_alt_index = -1;
	m_rNewDFUSeVariables.match_devnum = -1;
	m_rNewDFUSeVariables.match_iface_alt_name = NULL;
	m_rNewDFUSeVariables.match_serial = NULL;
	m_rNewDFUSeVariables.match_serial_dfu = NULL;
}

CDlgDpSVisio::~CDlgDpSVisio()
{
	if( NULL != m_pCurrentDialog )
	{
		if( NULL != m_pCurrentDialog->GetSafeHwnd() )
		{
			m_pCurrentDialog->DestroyWindow();
		}

		delete m_pCurrentDialog;
		m_pCurrentDialog = NULL;
	}

	if( NULL != m_pclDpSVisioCommunication )
	{
		delete m_pclDpSVisioCommunication;
		m_pclDpSVisioCommunication = NULL;
	}

	if( INVALID_HANDLE_VALUE != m_arsFirmwareData[Device_HID].m_hDevNotify )
	{
		UnregisterDeviceNotification( m_arsFirmwareData[Device_HID].m_hDevNotify );
		m_arsFirmwareData[Device_HID].m_hDevNotify = INVALID_HANDLE_VALUE;
	}

	if( INVALID_HANDLE_VALUE != m_arsFirmwareData[Device_DFUSe].m_hDevNotify )
	{
		UnregisterDeviceNotification( m_arsFirmwareData[Device_DFUSe].m_hDevNotify );
		m_arsFirmwareData[Device_DFUSe].m_hDevNotify = INVALID_HANDLE_VALUE;
	}

	if( INVALID_HANDLE_VALUE != m_arsFirmwareData[Device_NewDFUSe].m_hDevNotify )
	{
		UnregisterDeviceNotification( m_arsFirmwareData[Device_NewDFUSe].m_hDevNotify );
		m_arsFirmwareData[Device_NewDFUSe].m_hDevNotify = INVALID_HANDLE_VALUE;
	}

	if( INVALID_HANDLE_VALUE != m_hLeftPanelNormal )
	{
		DeleteObject( m_hLeftPanelNormal );
		m_hLeftPanelNormal = (HBITMAP)INVALID_HANDLE_VALUE;
	}

	if( INVALID_HANDLE_VALUE != m_hLeftPanelUpdateRunning )
	{
		DeleteObject( m_hLeftPanelUpdateRunning );
		m_hLeftPanelUpdateRunning = (HBITMAP)INVALID_HANDLE_VALUE;
	}

	if( INVALID_HANDLE_VALUE != m_hLeftPanelUpdateFinishedOK )
	{
		DeleteObject( m_hLeftPanelUpdateFinishedOK );
		m_hLeftPanelUpdateFinishedOK = (HBITMAP)INVALID_HANDLE_VALUE;
	}

	if( INVALID_HANDLE_VALUE != m_hLeftPanelUpdateFinishedError )
	{
		DeleteObject( m_hLeftPanelUpdateFinishedOK );
		m_hLeftPanelUpdateFinishedOK = (HBITMAP)INVALID_HANDLE_VALUE;
	}

	DeleteCriticalSection( &m_CriticalSection );
	
	if( NULL != GetSafeHwnd() && (UINT_PTR)0 != m_nTimer )
	{
		KillTimer( m_nTimer );
	}

	BYTE *pBuffer = m_arsFirmwareData[Device_HID].m_clFirmware.Detach();

	if( NULL != pBuffer )
	{
		free( pBuffer );
		pBuffer = NULL;
	}

	pBuffer = m_arsFirmwareData[Device_DFUSe].m_clFirmware.Detach();

	if( NULL != pBuffer )
	{
		free( pBuffer );
	}

	pBuffer = m_arsFirmwareData[Device_NewDFUSe].m_clFirmware.Detach();

	if( NULL != pBuffer )
	{
		free( pBuffer );
	}

	// DFU Se new driver.
	if( NULL != m_rNewDFUSeVariables.dfu_root && NULL != m_rNewDFUSeVariables.dfu_root->dev_handle )
	{
		libusb_close( m_rNewDFUSeVariables.dfu_root->dev_handle );
		m_rNewDFUSeVariables.dfu_root->dev_handle = NULL;
	}

	if( NULL != m_rNewDFUSeVariables.dfu_root )
	{
		disconnect_devices( &m_rNewDFUSeVariables );
	}

	if( NULL != m_pUSBContext )
	{
		libusb_exit( m_pUSBContext );
		m_pUSBContext = NULL;
	}
}

void CDlgDpSVisio::ShowDialog( DialogToShow eDlgToShow, LPARAM lpParam )
{
	if( NULL != m_pCurrentDialog )
	{
		if( NULL != m_pCurrentDialog->GetSafeHwnd() )
		{
			m_pCurrentDialog->DestroyWindow();
		}

		delete m_pCurrentDialog;
		m_pCurrentDialog = NULL;
	}

	m_eCurrentDlg = eDlgToShow;
	UINT uiDialogID;

	switch( eDlgToShow )
	{
		case Dialog_Status:
			m_pCurrentDialog = new CDlgDpSVisioStatus( this );

			if( NULL == m_pCurrentDialog )
			{
				return;
			}

			m_pCurrentDialog->SetParam( lpParam );
			uiDialogID = IDD_DLGDPSVISIO_STATUS;
			SetWindowText( TASApp.LoadLocalizedString( IDS_DLGDPSVISIOSTATUS_TITLE ) );
			m_StaticLeftBanner.SetBitmap( m_hLeftPanelNormal );
			break;

		case Dialog_HIDUpgradeRunning:
			// Verify first is device is yet connected before to go further.
			if( NULL == m_pclDpSVisioCommunication )
			{
				PostMessage( WM_USER_DPSVISIO_SHOWDIALOG, (WPARAM)CDlgDpSVisio::Dialog_Status );
				return;
			}

			m_pCurrentDialog = new CDlgDpSVisioHIDUpgradeRunning( this );

			if( NULL == m_pCurrentDialog )
			{
				return;
			}

			uiDialogID = IDD_DLGDPSVISIO_UPGRADERUNNING;
			SetWindowText( TASApp.LoadLocalizedString( IDS_DLGDPSVISIO_UPGRADERUNNING_TITLE ) );
			m_StaticLeftBanner.SetBitmap( m_hLeftPanelUpdateRunning );

			break;

		case Dialog_DFUUpgradeRunning:
			EnableUpdateButtons( CDlgDpSVisio::UBT_ForceUpdate, FALSE, TRUE );

			// Verify first is device is yet connected before to go further.
			if( NULL == m_pclDpSVisioCommunication )
			{
				PostMessage( WM_USER_DPSVISIO_SHOWDIALOG, (WPARAM)CDlgDpSVisio::Dialog_Status );
				return;
			}

			m_pCurrentDialog = new CDlgDpSVisioDFUUpgradeRunning( this );

			if( NULL == m_pCurrentDialog )
			{
				return;
			}

			m_pCurrentDialog->SetParam( lpParam );
			uiDialogID = IDD_DLGDPSVISIO_UPGRADERUNNING;
			SetWindowText( TASApp.LoadLocalizedString( IDS_DLGDPSVISIO_UPGRADERUNNING_TITLE ) );
			m_StaticLeftBanner.SetBitmap( m_hLeftPanelUpdateRunning );

			break;

		case Dialog_UpgradeFinished:

			m_pCurrentDialog = new CDlgDpSVisioUpgradeFinished( this );

			if( NULL == m_pCurrentDialog )
			{
				return;
			}

			m_pCurrentDialog->SetParam( lpParam );
			uiDialogID = IDD_DLGDPSVISIO_UPGRADEFINISHED;
			// Window text is set in the 'CDlgDpSVisioUpgradeFinished::OnInitDialog' in regards to the passed parameter.
			m_StaticLeftBanner.SetBitmap( ( true == m_rUpgradeStatus.m_fUpgradeStatus ) ? m_hLeftPanelUpdateFinishedOK : m_hLeftPanelUpdateFinishedError );

			break;
	}

	if( NULL == m_pCurrentDialog )
	{
		m_eCurrentDlg = Dialog_None;
		ASSERT( 0 );
		return;
	}

	if( 0 == m_pCurrentDialog->Create( uiDialogID, this ) )
	{
		m_eCurrentDlg = Dialog_None;
		delete m_pCurrentDialog;
		m_pCurrentDialog = NULL;
		ASSERT( 0 );
		return;
	}

	CRect rectDlgContainer;
	GetDlgItem( IDC_DLGCONTAINER )->GetWindowRect( &rectDlgContainer );
	ScreenToClient( &rectDlgContainer );
	m_pCurrentDialog->SetWindowPos( NULL, rectDlgContainer.left, rectDlgContainer.top, -1, -1, SWP_NOSIZE | SWP_NOACTIVATE | SWP_NOZORDER );

	m_pCurrentDialog->ShowWindow( TRUE );
}

bool CDlgDpSVisio::IsDpSVisioInNormalModeConnected()
{
	if( NULL == m_pclDpSVisioCommunication || CDpSVisioBaseCommunication::CT_HID != m_pclDpSVisioCommunication->GetCommunicationType() )
	{
		return false;
	}

	return true;
}

bool CDlgDpSVisio::IsDpSVisioInDFUModeConnected()
{
	if( NULL == m_pclDpSVisioCommunication || CDpSVisioBaseCommunication::CT_DFU != m_pclDpSVisioCommunication->GetCommunicationType() )
	{
		return false;
	}

	return true;
}

void CDlgDpSVisio::DisconnectHIDDevice()
{
	if( NULL == m_pclDpSVisioCommunication || CDpSVisioBaseCommunication::CT_HID != m_pclDpSVisioCommunication->GetCommunicationType()
			|| NULL == m_pclDpSVisioCommunication->GetDpSVisioInformation() )
	{
		return;
	}

	_DisconnectHIDDevice( m_pclDpSVisioCommunication->GetDpSVisioInformation()->m_strDeviceName );
}

// This method doesn't work! I keep it just because it has been hard to get the code.
// 'SetupDiCallClassInstaller' can not be called from a x32 process on a x64 machine.
void CDlgDpSVisio::EnableDpSVisioInHIDMode( bool bEnable )
{
	if( NULL == m_pclDpSVisioCommunication || CDpSVisioBaseCommunication::CT_HID != m_pclDpSVisioCommunication->GetCommunicationType()
			|| NULL == m_pclDpSVisioCommunication->GetDpSVisioInformation() )
	{
		return;
	}

	GUID InterfaceClassGuid = {0x4d1e55b2, 0xf16f, 0x11cf, {0x88, 0xcb, 0x00, 0x11, 0x11, 0x00, 0x00, 0x30} };

	SP_DEVINFO_DATA rDeviceInfoData;
	rDeviceInfoData.cbSize = sizeof( SP_DEVINFO_DATA );

	SP_DEVICE_INTERFACE_DATA rDeviceInterfaceData;
	rDeviceInterfaceData.cbSize = sizeof( SP_DEVICE_INTERFACE_DATA );

	SP_DEVICE_INTERFACE_DETAIL_DATA_A *pDeviceInterfaceDetailData = NULL;
	
	int iDeviceIndex = 0;
	HDEVINFO hDeviceInfo = SetupDiGetClassDevs( &InterfaceClassGuid, NULL, NULL, DIGCF_PRESENT | DIGCF_DEVICEINTERFACE );

	for(;;)
	{
		BOOL bReturnValue = SetupDiEnumDeviceInterfaces( hDeviceInfo, NULL, &InterfaceClassGuid, iDeviceIndex,
															&rDeviceInterfaceData );

		if( FALSE == bReturnValue )
		{
			return;
		}
		
		DWORD dwRequiredSize = 0;
		SetupDiGetDeviceInterfaceDetailA( hDeviceInfo, &rDeviceInterfaceData, NULL, 0, &dwRequiredSize, NULL );

		pDeviceInterfaceDetailData = ( SP_DEVICE_INTERFACE_DETAIL_DATA_A * ) malloc( dwRequiredSize );
		pDeviceInterfaceDetailData->cbSize = sizeof( SP_DEVICE_INTERFACE_DETAIL_DATA_A );

		bReturnValue = SetupDiGetDeviceInterfaceDetailA( hDeviceInfo, &rDeviceInterfaceData, pDeviceInterfaceDetailData, dwRequiredSize,
															NULL, &rDeviceInfoData );

		if( FALSE == bReturnValue )
		{
			free( pDeviceInterfaceDetailData );
			return;
		}

		CString strDeviceName = CString( pDeviceInterfaceDetailData->DevicePath ).MakeLower();

		if( 0 == strDeviceName.Compare( m_pclDpSVisioCommunication->GetDpSVisioInformation()->m_strDeviceName ) )
		{
			SP_CLASSINSTALL_HEADER rHeader;
            rHeader.cbSize = sizeof( SP_CLASSINSTALL_HEADER );
            rHeader.InstallFunction = DIF_PROPERTYCHANGE;

			SP_PROPCHANGE_PARAMS rPropChangeParams;
            rPropChangeParams.ClassInstallHeader = rHeader;
            rPropChangeParams.StateChange = ( true == bEnable ) ? DICS_ENABLE : DICS_DISABLE;
            rPropChangeParams.Scope = DICS_FLAG_GLOBAL;
            rPropChangeParams.HwProfile = 0;

			if( FALSE == SetupDiSetClassInstallParams( hDeviceInfo, &rDeviceInfoData, (SP_CLASSINSTALL_HEADER*)&rPropChangeParams, sizeof( rPropChangeParams ) ) )
			{
				free( pDeviceInterfaceDetailData );
				return;
			}
            
			if( FALSE == SetupDiCallClassInstaller( DIF_PROPERTYCHANGE, hDeviceInfo, &rDeviceInfoData ) )
			{
				DWORD err=GetLastError();
				free( pDeviceInterfaceDetailData );
				return;
			}

			return;
		}

		free( pDeviceInterfaceDetailData );
		iDeviceIndex++;
	}
}

void CDlgDpSVisio::EnableUpdateButtons( UpdateButtonType eWhichButton, BOOL bEnable, BOOL bShow )
{
	int nID = ( UBT_Update == eWhichButton ) ? IDUPDATE : IDFORCEUPDATE;
	int nShow = ( TRUE == bShow ) ? SW_SHOW : SW_HIDE;

	GetDlgItem( nID )->EnableWindow( bEnable );
	GetDlgItem( nID )->ShowWindow( nShow );
	GetDlgItem( nID )->Invalidate();
	GetDlgItem( nID )->RedrawWindow();
}

void CDlgDpSVisio::EnableCancelButton( BOOL bEnable )
{
	GetDlgItem( IDCANCEL )->EnableWindow( bEnable );
	GetDlgItem( IDCANCEL )->Invalidate();
	GetDlgItem( IDCANCEL )->RedrawWindow();
}

void CDlgDpSVisio::ChangeCancelButtonName( CString strNewName )
{
	GetDlgItem( IDCANCEL )->SetWindowText( strNewName );
	GetDlgItem( IDCANCEL )->Invalidate();
	GetDlgItem( IDCANCEL )->RedrawWindow();
}

void CDlgDpSVisio::StartDFUPollingDetection()
{
	if( (UINT_PTR)0 != m_nTimer )
	{
		return;
	}

	m_nTimer = SetTimer( _TIMERID_DLGDPSVISIODFUPOLLINGDETECTION, 1000, NULL );
}

BOOL CDlgDpSVisio::PreTranslateMessage( MSG *pMsg )
{
	if( WM_KEYDOWN == pMsg->message )
    {
        if( VK_RETURN == pMsg->wParam || VK_ESCAPE == pMsg->wParam )
        {
            return TRUE;                // Do not process further
        }
    }

    return CWnd::PreTranslateMessage( pMsg );
}

void CDlgDpSVisio::DoDataExchange( CDataExchange* pDX )
{
	CDialogEx::DoDataExchange( pDX );
	DDX_Control( pDX, IDC_STATICLEFTBANNER, m_StaticLeftBanner );
	DDX_Control( pDX, IDC_BUTTONHIDCONNECT, m_ButtonHIDConnect );
	DDX_Control( pDX, IDC_BUTTONHIDDISCONNECT, m_ButtonHIDDisconnect );
	DDX_Control( pDX, IDC_BUTTONDFUCONNECT, m_ButtonDFUConnect );
	DDX_Control( pDX, IDC_BUTTONDFUDISCONNECT, m_ButtonDFUDisconnect );
}

BEGIN_MESSAGE_MAP( CDlgDpSVisio, CDialogEx )
	ON_MESSAGE( WM_USER_DPSVISIO_FORCEHIDDETECTION, OnForceDpSVisioDetection )
	ON_MESSAGE( WM_USER_DPSVISIO_SHOWDIALOG, OnShowDialogToChange )
	ON_MESSAGE( WM_USER_DPSVISIO_ENABLEBUTTON, OnEnableDisableButton )
	ON_MESSAGE( WM_USER_DPSVISIO_EXIT, OnExit )
	ON_WM_DEVICECHANGE()
	ON_BN_CLICKED( IDUPDATE, &OnBnClickedUpdate )
	ON_BN_CLICKED( IDFORCEUPDATE, &OnBnClickedForceUpdate )
	ON_BN_CLICKED( IDCANCEL, &OnBnClickedCancel )
	ON_WM_TIMER()

#ifdef DEBUG
	ON_BN_CLICKED( IDC_BUTTONHIDCONNECT, &OnBnClickedButtonHIDConnect )
	ON_BN_CLICKED( IDC_BUTTONHIDDISCONNECT, &OnBnClickedButtonHIDDisconnect )
	ON_BN_CLICKED( IDC_BUTTONDFUCONNECT, &OnBnClickedButtonDFUConnect )
	ON_BN_CLICKED( IDC_BUTTONDFUDISCONNECT, &OnBnClickedButtonDFUDisconnect )
#endif
END_MESSAGE_MAP()

BOOL CDlgDpSVisio::OnInitDialog()
{
	CDialogEx::OnInitDialog();

	CString str = TASApp.LoadLocalizedString( IDS_DLGDPSVISIO_UPDATE );
	GetDlgItem( IDUPDATE )->SetWindowText( str );

	str = TASApp.LoadLocalizedString( IDS_DLGDPSVISIO_FORCEUPDATE );
	GetDlgItem( IDFORCEUPDATE )->SetWindowText( str );

	str = TASApp.LoadLocalizedString( IDS_CANCEL );
	GetDlgItem( IDCANCEL )->SetWindowText( str );

	// Register notification to receive USB HID connection/disconnection event in this dialog.
	DEV_BROADCAST_DEVICEINTERFACE NotificationFilter;

	ZeroMemory( &NotificationFilter, sizeof( NotificationFilter ) );
	NotificationFilter.dbcc_size = sizeof( DEV_BROADCAST_DEVICEINTERFACE );
	NotificationFilter.dbcc_devicetype = DBT_DEVTYP_DEVICEINTERFACE;
	NotificationFilter.dbcc_classguid = m_arsFirmwareData[Device_HID].m_InterfaceClassGuid;
	m_arsFirmwareData[Device_HID].m_hDevNotify = RegisterDeviceNotification( this->GetSafeHwnd(), &NotificationFilter, DEVICE_NOTIFY_WINDOW_HANDLE );

	if( NULL == m_arsFirmwareData[Device_HID].m_hDevNotify )
	{
		CString str;
		str.Format( _T("RegisterDeviceNotification for HID device failed: %d\n"), GetLastError() );
		OutputDebugString( str );
		return FALSE;
	}

	m_arsFirmwareData[Device_HID].m_strError = _CheckIfBINFileExist();

	
	// Register notification to receive USB DFUSe connection/disconnection event in this dialog.
	NotificationFilter.dbcc_classguid = m_arsFirmwareData[Device_DFUSe].m_InterfaceClassGuid;
	m_arsFirmwareData[Device_DFUSe].m_hDevNotify = RegisterDeviceNotification( this->GetSafeHwnd(), &NotificationFilter, DEVICE_NOTIFY_WINDOW_HANDLE );

	if( NULL == m_arsFirmwareData[Device_DFUSe].m_hDevNotify )
	{
		CString str;
		str.Format( _T("RegisterDeviceNotification for DFU Se device failed: %d\n"), GetLastError() );
		OutputDebugString( str );
		UnregisterDeviceNotification( m_arsFirmwareData[Device_DFUSe].m_hDevNotify );
		return FALSE;
	}

	m_arsFirmwareData[Device_DFUSe].m_strError = _CheckIfDFUFileExist( Device_DFUSe );


	// Register notification to receive new USB DFUSe connection/disconnection event in this dialog.
	NotificationFilter.dbcc_classguid = m_arsFirmwareData[Device_NewDFUSe].m_InterfaceClassGuid;
	m_arsFirmwareData[Device_NewDFUSe].m_hDevNotify = RegisterDeviceNotification( this->GetSafeHwnd(), &NotificationFilter, DEVICE_NOTIFY_WINDOW_HANDLE );

	if( NULL == m_arsFirmwareData[Device_NewDFUSe].m_hDevNotify )
	{
		CString str;
		str.Format( _T("RegisterDeviceNotification for new DFU Se device failed: %d\n"), GetLastError() );
		OutputDebugString( str );
		UnregisterDeviceNotification( m_arsFirmwareData[Device_NewDFUSe].m_hDevNotify );
		return FALSE;
	}

	m_arsFirmwareData[Device_NewDFUSe].m_strError = _CheckIfDFUFileExist( Device_NewDFUSe );


	if( false == IsFirmwareAvailable( Device_HID ) && false == IsFirmwareAvailable( Device_DFUSe ) && false == IsFirmwareAvailable( Device_NewDFUSe ) )
	{
		// Needed files to update the DpS-Visio are not found in your computer or are corrupted.
		CString str = TASApp.LoadLocalizedString( IDS_DLGDPSVISIO_NEEDEDFILESMISSINGORCORRUPTED );
		str += _T("\r\n\r\n") + TASApp.LoadLocalizedString( IDS_DLGDPSVISIO_FIRMWAREERROR_APPLYUPDATE );
		AfxMessageBox( str, MB_OK | MB_ICONERROR, 0 );
		UnregisterDeviceNotification( m_arsFirmwareData[Device_HID].m_hDevNotify );
		UnregisterDeviceNotification( m_arsFirmwareData[Device_DFUSe].m_hDevNotify );
		UnregisterDeviceNotification( m_arsFirmwareData[Device_NewDFUSe].m_hDevNotify );
		PostMessage( WM_CLOSE );
		return TRUE;
	}

	ShowDialog( Dialog_Status );

	// State be default.
	EnableUpdateButtons( UBT_Update, FALSE, TRUE );
	EnableUpdateButtons( UBT_ForceUpdate, FALSE, FALSE );

	// USB library initialisation for the new DFU Se driver
	int iReturn = libusb_init( &m_pUSBContext );
	
	if( 0 != iReturn )
	{
		FormatString( str, IDS_DLGDPSVISIO_UNABLETOINITIALIZELIBUSB, libusb_error_name( iReturn ) );
		OutputDebugString( str );
		return FALSE;
	}

	// To postpone device detection.
	PostMessage( WM_USER_DPSVISIO_FORCEHIDDETECTION );

#ifndef DEBUG
	m_ButtonHIDConnect.EnableWindow( FALSE );
	m_ButtonHIDConnect.ShowWindow( SW_HIDE );
	m_ButtonHIDDisconnect.EnableWindow( FALSE );
	m_ButtonHIDDisconnect.ShowWindow( SW_HIDE );
	m_ButtonDFUConnect.EnableWindow( FALSE );
	m_ButtonDFUConnect.ShowWindow( SW_HIDE );
	m_ButtonDFUDisconnect.EnableWindow( FALSE );
	m_ButtonDFUDisconnect.ShowWindow( SW_HIDE );
#endif

	return TRUE;
}

void CDlgDpSVisio::OnOK()
{
	// Don't call base class! (it's to avoid to loose focus when pressing [ENTER] key).
}

void CDlgDpSVisio::OnCancel()
{
	// Don't call base class! (it's to avoid to loose focus when pressing [ESCAPE] key).
}

LRESULT CDlgDpSVisio::OnForceDpSVisioDetection( WPARAM wParam, LPARAM lParam )
{
	bool bIsConnected = _CheckIfHIDDeviceConnected();

	if( false == bIsConnected )
	{
		bIsConnected = _CheckIfDFUSeDeviceConnected();
	}
	
	if( false == bIsConnected )
	{
		bIsConnected = _CheckIfNewDFUSeDeviceConnected();
	}

	return 0;
}

LRESULT CDlgDpSVisio::OnDpSVisioUpdated( WPARAM wParam, LPARAM lParam )
{
	if( Dialog_HIDUpgradeRunning == m_eCurrentDlg || Dialog_DFUUpgradeRunning == m_eCurrentDlg )
	{
		ShowDialog( Dialog_UpgradeFinished, (LPARAM)wParam );
	}

	return 0;
}

LRESULT CDlgDpSVisio::OnShowDialogToChange( WPARAM wParam, LPARAM lParam )
{
	ShowDialog( (DialogToShow)wParam, lParam );
	return 0;
}

LRESULT CDlgDpSVisio::OnEnableDisableButton( WPARAM wParam, LPARAM lParam )
{
	if( NULL != GetDlgItem( (int)wParam ) )
	{
		GetDlgItem( (int)wParam )->EnableWindow( (BOOL)lParam );
	}

	return 0;
}

LRESULT CDlgDpSVisio::OnExit( WPARAM wParam, LPARAM lParam )
{
	OnBnClickedCancel();
	return 0;
}

BOOL CDlgDpSVisio::OnDeviceChange( UINT nEventType, DWORD_PTR dwData )
{
	OutputDebugString( _T("OnDeviceChange called...\n" ) );

	if( DBT_DEVICEARRIVAL != nEventType && DBT_DEVICEREMOVECOMPLETE != nEventType )
	{
		OutputDebugString( _T("It's not a arrival or remove complete event -> end.\n" ) );
		return 0;
	}

	OutputDebugString( _T("It's a device arrival or remove complete event.\n" ) );

	DEV_BROADCAST_HDR *pDeviceBrodcastHeader = (DEV_BROADCAST_HDR *)dwData;
	DEV_BROADCAST_DEVICEINTERFACE *pDeviceInterface = NULL;

	if( NULL == pDeviceBrodcastHeader || DBT_DEVTYP_DEVICEINTERFACE != pDeviceBrodcastHeader->dbch_devicetype )
	{
		OutputDebugString( _T("Can't access to device broadcast header structure -> end.\n" ) );
		return 0;
	}

	pDeviceInterface = (DEV_BROADCAST_DEVICEINTERFACE *)dwData;
	CString strDeviceName = CString( pDeviceInterface->dbcc_name ).MakeLower();

	if( m_arsFirmwareData[Device_HID].m_InterfaceClassGuid == pDeviceInterface->dbcc_classguid )
	{
		// HID device.
		OutputDebugString( _T("HID device.\n" ) );

		switch( nEventType )
		{
			case DBT_DEVICEARRIVAL:
				OutputDebugString( _T("HID arrival.\n" ) );

				if( true == _IsDpSVisioHID( strDeviceName ) )
				{
					OutputDebugString( _T("It's a DpS-Visio in HID mode -> Connection.\n" ) );

					_ConnectHIDDevice( strDeviceName );
				}
				else
				{
					OutputDebugString( _T("It's not DpS-Visio in HID mode.\n" ) );
				}

				break;

			case DBT_DEVICEREMOVECOMPLETE:
				OutputDebugString( _T("HID remove complete.\n" ) );

				if( NULL != m_pclDpSVisioCommunication && NULL != m_pclDpSVisioCommunication->GetDpSVisioInformation()
						&& 0 == strDeviceName.Compare( m_pclDpSVisioCommunication->GetDpSVisioInformation()->m_strDeviceName ) )
				{
					OutputDebugString( _T("It's a DpS-Visio in HID mode -> Disconnection.\n" ) );
					_DisconnectHIDDevice( strDeviceName );
				}
				else
				{
					OutputDebugString( _T("No DpS-Visio in HID connected or not the same one.\n" ) );
				}
				
				break;
		}
	}
	else if( m_arsFirmwareData[Device_DFUSe].m_InterfaceClassGuid == pDeviceInterface->dbcc_classguid )
	{
		// DFU Se device.
		OutputDebugString( _T("DFU Se device.\n" ) );

		switch( nEventType )
		{
			case DBT_DEVICEARRIVAL:
				OutputDebugString( _T("DFU Se arrival.\n" ) );

				// Because now we try to detect DFU Se device either by Windows event either by polling, we need to exclude.
				// concurrent task.
				EnterCriticalSection( &m_CriticalSection );

				if( NULL != m_pclDpSVisioCommunication && CDpSVisioBaseCommunication::CT_DFU == m_pclDpSVisioCommunication->GetCommunicationType() )
				{
					OutputDebugString( _T("DFU Se already detected.\n" ) );
					LeaveCriticalSection( &m_CriticalSection );
					break;
				}
				
				OutputDebugString( _T("Check if it's well a DpS-Visio.\n" ) );

				if( true == _IsDpSVisioDFUSe( strDeviceName ) )
				{
					OutputDebugString( _T("It's well a DpS-Visio -> connection.\n" ) );

					_ConnectDFUSeDevice( strDeviceName );
					KillTimer( m_nTimer );
					m_nTimer = (UINT_PTR)0;
				}
				else
				{
					OutputDebugString( _T("It's not a DpS-Visio.\n" ) );
				}
				
				LeaveCriticalSection( &m_CriticalSection );

				break;

			case DBT_DEVICEREMOVECOMPLETE:
				OutputDebugString( _T("DFU Se remove complete.\n" ) );

				if( NULL != m_pclDpSVisioCommunication && NULL != m_pclDpSVisioCommunication->GetDpSVisioInformation()
						&& 0 == strDeviceName.Compare( m_pclDpSVisioCommunication->GetDpSVisioInformation()->m_strDeviceName ) )
				{
					OutputDebugString( _T("It's a DpS-Visio in DFU Se mode -> Disconnection.\n" ) );
					_DisconnectDFUSeDevice( strDeviceName );
				}
				else
				{
					OutputDebugString( _T("No DpS-Visio in DFU Se connected or not the same one.\n" ) );
				}

				break;
		}
	}
	else if( m_arsFirmwareData[Device_NewDFUSe].m_InterfaceClassGuid == pDeviceInterface->dbcc_classguid )
	{
		// New DFU Se device.
		OutputDebugString( _T("New DFU Se device.\n" ) );

		switch( nEventType )
		{
		case DBT_DEVICEARRIVAL:
			OutputDebugString( _T("New DFU Se arrival.\n" ) );

			// Because now we try to detect New DFU Se device either by Windows event either by polling, we need to exclude.
			// concurrent task.
			EnterCriticalSection( &m_CriticalSection );

			if( NULL != m_pclDpSVisioCommunication && CDpSVisioBaseCommunication::CT_DFU == m_pclDpSVisioCommunication->GetCommunicationType() )
			{
				OutputDebugString( _T("New DFU Se already detected.\n" ) );
				LeaveCriticalSection( &m_CriticalSection );
				break;
			}

			OutputDebugString( _T("Check if it's well a DpS-Visio.\n" ) );

			if( true == _IsDpSVisioNewDFUSe( strDeviceName ) )
			{
				OutputDebugString( _T("It's well a DpS-Visio -> connection.\n" ) );

				_ConnectNewDFUSeDevice( strDeviceName );
				KillTimer( m_nTimer );
				m_nTimer = (UINT_PTR)0;
			}
			else
			{
				OutputDebugString( _T("It's not a DpS-Visio.\n" ) );
			}

			LeaveCriticalSection( &m_CriticalSection );

			break;

		case DBT_DEVICEREMOVECOMPLETE:
			OutputDebugString( _T("New DFU Se remove complete.\n" ) );

			if( NULL != m_pclDpSVisioCommunication && NULL != m_pclDpSVisioCommunication->GetDpSVisioInformation()
					&& 0 == strDeviceName.Compare( m_pclDpSVisioCommunication->GetDpSVisioInformation()->m_strDeviceName ) )
			{
				OutputDebugString( _T("It's a DpS-Visio in new DFU Se mode -> Disconnection.\n" ) );
				_DisconnectNewDFUSeDevice( strDeviceName );
			}
			else
			{
				OutputDebugString( _T("No DpS-Visio in new DFU Se connected or not the same one.\n" ) );
			}

			break;
		}
	}

	OutputDebugString( _T("OnDeviceChange end.\n" ) );
	return 0;
}

void CDlgDpSVisio::OnBnClickedUpdate()
{
	// Remark: "Update" button is available only when DpS-Visio is connected in the normal mode (HID).
	//         Because it's only in this mode that we can retrieve information from the DpS-Visio.

	if( Dialog_Status == m_eCurrentDlg )
	{
		if( m_pclDpSVisioCommunication->GetDpSVisioInformation()->m_dFirmwareVersion < 3.0 )
		{
			// Before DpS-Visio version 3.0 there was only the DFU mode for the update.
			ShowDialog( Dialog_DFUUpgradeRunning, (LPARAM)UpdateParam_HID );
		}
		else
		{
			// Now we can update via HID mode.
			ShowDialog( Dialog_HIDUpgradeRunning );
		}

		EnableUpdateButtons( UBT_Update, FALSE, TRUE );
	}
}

void CDlgDpSVisio::OnBnClickedForceUpdate()
{
	// Remark: Forcing update can happen when the connected DpS-Visio is in HID mode and the firmware version 
	//         available is the same as the DpS-Visio firmware version.
	//         Or when DpS-Visio is connected in DFU mode and user wants to force the update without knowing 
	//         the DpS-Visio firmware version.

	if( NULL == m_pclDpSVisioCommunication )
	{
		return;
	}
	
	if( Dialog_Status == m_eCurrentDlg )
	{
		if( CDpSVisioBaseCommunication::CT_HID == m_pclDpSVisioCommunication->GetCommunicationType() )
		{
			if( m_pclDpSVisioCommunication->GetDpSVisioInformation()->m_dFirmwareVersion < 3.0 )
			{
				// Before DpS-Visio version 3.0 there was only the DFU mode for the update.
				ShowDialog( Dialog_DFUUpgradeRunning, (LPARAM)UpdateParam_HID );
			}
			else
			{
				// Now we can update via HID mode.
				ShowDialog( Dialog_HIDUpgradeRunning );
			}
		}
		else
		{
			// Force update in DFU mode.
			ShowDialog( Dialog_DFUUpgradeRunning, (LPARAM)UpdateParam_DFU );
		}
		
		EnableUpdateButtons( UBT_ForceUpdate, FALSE, TRUE );	
	}
}

void CDlgDpSVisio::OnBnClickedCancel()
{
	if( NULL != m_pCurrentDialog )	
	{
		m_pCurrentDialog->Stop();
	}

	CDialogEx::OnCancel();
}

#ifdef DEBUG
void CDlgDpSVisio::OnBnClickedButtonHIDConnect()
{
	if( NULL != m_pclDpSVisioCommunication )
	{
		// Don't connect more than one DpS-Visio at the same time to process to an update.
		CString str = TASApp.LoadLocalizedString( IDS_DLGDPSVISIO_NOTWODEVICESATSAMETIME );
		AfxMessageBox( str, MB_OK | MB_ICONEXCLAMATION, 0 );
	}
	else
	{
		m_pclDpSVisioCommunication = new CDpSVisioFakeHIDCommunication( this );

		if( NULL != m_pclDpSVisioCommunication )
		{
			m_pclDpSVisioCommunication->DpSVisioConnected( _T("") );
		}

		if( NULL != m_pCurrentDialog )
		{
			m_pCurrentDialog->OnDpSVisioConnected( WhichDevice::Device_HID );
		}
	}
}

void CDlgDpSVisio::OnBnClickedButtonHIDDisconnect()
{
	if( NULL != m_pclDpSVisioCommunication && NULL != m_pclDpSVisioCommunication->GetDpSVisioInformation() )
	{
		if( NULL != m_pCurrentDialog )
		{
			m_pCurrentDialog->OnDpSVisioDisconnected( WhichDevice::Device_HID );
		}

		delete m_pclDpSVisioCommunication;
		m_pclDpSVisioCommunication = NULL;
	}
}

void CDlgDpSVisio::OnBnClickedButtonDFUConnect()
{
	if( NULL != m_pclDpSVisioCommunication )
	{
		// Don't connect more than one DpS-Visio at the same time to process to an update.
		CString str = TASApp.LoadLocalizedString( IDS_DLGDPSVISIO_NOTWODEVICESATSAMETIME );
		AfxMessageBox( str, MB_OK | MB_ICONEXCLAMATION, 0 );
	}
	else
	{
		m_pclDpSVisioCommunication = new CDpSVisioFakeDFUCommunication( this );

		if( NULL != m_pclDpSVisioCommunication )
		{
			( (CDpSVisioFakeDFUCommunication *)m_pclDpSVisioCommunication )->Create( _T("STATIC"), _T(""), WS_CHILD, CRect(0, 0, 0, 0), this, 1234);

			m_pclDpSVisioCommunication->DpSVisioConnected( _T("") );
		}

		if( NULL != m_pCurrentDialog )
		{
			m_pCurrentDialog->OnDpSVisioConnected( WhichDevice::Device_DFUSe );
		}
	}
}

void CDlgDpSVisio::OnBnClickedButtonDFUDisconnect()
{
	if( NULL != m_pclDpSVisioCommunication && NULL != m_pclDpSVisioCommunication->GetDpSVisioInformation() )
	{
		if( NULL != m_pCurrentDialog )
		{
			m_pCurrentDialog->OnDpSVisioDisconnected( WhichDevice::Device_DFUSe );
		}

		delete m_pclDpSVisioCommunication;
		m_pclDpSVisioCommunication = NULL;
	}
}
#endif

void CDlgDpSVisio::OnTimer( UINT_PTR nIDEvent )
{
	OutputDebugString( _T("OnTimer called...\n" ) );

	if( _TIMERID_DLGDPSVISIODFUPOLLINGDETECTION != nIDEvent )
	{
		return;
	}

	EnterCriticalSection( &m_CriticalSection );

	if( NULL != m_pclDpSVisioCommunication && CDpSVisioBaseCommunication::CT_DFU == m_pclDpSVisioCommunication->GetCommunicationType() )
	{
		OutputDebugString( _T("DFU already detected.\n" ) );
		LeaveCriticalSection( &m_CriticalSection );
		return;
	}

	OutputDebugString( _T("Check if it's well a DpS-Visio in DFU Se mode.\n" ) );

	if( true == _CheckIfDFUSeDeviceConnected() )
	{
		OutputDebugString( _T("It's well a DpS-Visio in DFU Se mode.\n" ) );
		KillTimer( m_nTimer );
		m_nTimer = (UINT_PTR)m_nTimer;
	}
	else if( true == _CheckIfNewDFUSeDeviceConnected() )
	{
		OutputDebugString( _T("It's well a DpS-Visio in new DFU Se mode.\n" ) );
		KillTimer( m_nTimer );
		m_nTimer = (UINT_PTR)m_nTimer;
	}

	LeaveCriticalSection( &m_CriticalSection );
	OutputDebugString( _T("OnTimer end.\n" ) );
}

CString CDlgDpSVisio::_CheckIfBINFileExist()
{
	CString strErrorMsg = _T("");
	CString strApplyUpdateMsg = TASApp.LoadLocalizedString( IDS_DLGDPSVISIO_FIRMWAREERROR_APPLYUPDATE );
	CString strTAScopePath = CTAScopeUtil::GetTAScopeFolder();
	CString strBINFilename = strTAScopePath + CString( _T("\\") ) + CString( _DPSVISIO_BIN_FILENAME );
	
	CFileStatus status;

	if( FALSE == CFile::GetStatus( strBINFilename, status ) )
	{
		strErrorMsg = TASApp.LoadLocalizedString( IDS_DLGDPSVISIO_FIRMWAREERROR_NOTFOUND ) + _T("\r\n\r\n") + strApplyUpdateMsg;
		return strErrorMsg;
	}

	// Try to open it.
	CFile BINFile;
	
	if( 0 == BINFile.Open( strBINFilename, CFile::modeRead | CFile::typeBinary ) )
	{
		strErrorMsg = TASApp.LoadLocalizedString( IDS_DLGDPSVISIO_FIRMWAREERROR_CANTOPEN ) + _T("\r\n\r\n") + strApplyUpdateMsg;
		return strErrorMsg;
	}

	// Check the file length.
	UINT uiFileSize = (UINT)BINFile.GetLength();

	if( 0 == uiFileSize )
	{
		strErrorMsg = TASApp.LoadLocalizedString( IDS_DLGDPSVISIO_FIRMWAREERROR_FILEEMPTY ) + _T("\r\n\r\n") + strApplyUpdateMsg;
		return strErrorMsg;
	}

	BYTE *pBuffer = new BYTE[uiFileSize];

	if( NULL == pBuffer )
	{
		strErrorMsg = TASApp.LoadLocalizedString( IDS_DLGDPSVISIO_FIRMWAREERROR_TOOBIG ) + _T("\r\n\r\n") + strApplyUpdateMsg;
		return strErrorMsg;
	}

	try
	{
		if( uiFileSize != BINFile.Read( pBuffer, uiFileSize ) )
		{
			BINFile.Close();
			delete pBuffer;

			strErrorMsg = TASApp.LoadLocalizedString( IDS_DLGDPSVISIO_BINERROR_READERROR ) + _T("\r\n\r\n") + strApplyUpdateMsg;
			return strErrorMsg;
		}
	}
	catch( CFileException *pFileException )
	{
		BINFile.Close();
		delete pBuffer;

		strErrorMsg = _GetExceptionString( pFileException->m_cause );
		pFileException->Delete();

		return strErrorMsg;
	}
	
	m_arsFirmwareData[Device_HID].m_clFirmware.Attach( pBuffer, uiFileSize );

	UINT32 uiFirmwareVersion;
	UINT8 uiFirmwareVersionSize = sizeof( uiFirmwareVersion );

	// We read DpS-Visio firmware version at the end of the file.
	m_arsFirmwareData[Device_HID].m_clFirmware.Seek( -uiFirmwareVersionSize, CFile::end );

	if( uiFirmwareVersionSize != m_arsFirmwareData[Device_HID].m_clFirmware.Read( (void *)&uiFirmwareVersion, uiFirmwareVersionSize ) )
	{
		BYTE *pBuffer = m_arsFirmwareData[Device_HID].m_clFirmware.Detach();
		free( pBuffer );

		strErrorMsg = TASApp.LoadLocalizedString( IDS_DLGDPSVISIO_BINERROR_READVERSIONERROR ) + _T("\r\n\r\n") + strApplyUpdateMsg;
		return strErrorMsg;
	}

	DpSVisioUtils_CopyMSBFirst( (UINT8*)&m_arsFirmwareData[Device_HID].m_uiRevision, (UINT8*)&uiFirmwareVersion, sizeof( UINT32 ) );
	
	// Expected value SW_VERSION_HEX		0x20
	UINT8 uiTemp = uiFirmwareVersion & 0x000000FF;
	UINT8 uiVersionHi = ( uiTemp & 0xF0 ) >> 4;
	UINT8 uiVersionLo = uiTemp & 0x0F;
	
	m_arsFirmwareData[Device_HID].m_strRevision.Format( _T("%u.%u"), uiVersionHi, uiVersionLo );
	m_arsFirmwareData[Device_HID].m_strFilename = strBINFilename;

	return _T("");
}

CString CDlgDpSVisio::_CheckIfDFUFileExist( int iWhichDFU )
{
	CString strErrorMsg = _T("");
	CString strApplyUpdateMsg = TASApp.LoadLocalizedString( IDS_DLGDPSVISIO_FIRMWAREERROR_APPLYUPDATE );
	CString strTAScopePath = CTAScopeUtil::GetTAScopeFolder();
	CString strDFUFilename = strTAScopePath + CString( _T("\\") ) + CString( _DPSVISIO_DFU_FILENAME );
	
	CFileStatus status;

	if( FALSE == CFile::GetStatus( strDFUFilename, status ) )
	{
		strErrorMsg = TASApp.LoadLocalizedString( IDS_DLGDPSVISIO_FIRMWAREERROR_NOTFOUND ) + _T("\r\n\r\n") + strApplyUpdateMsg;
		return strErrorMsg;
	}

	// Try open it.
	CFile DFUFile;
	
	if( 0 == DFUFile.Open( strDFUFilename, CFile::modeRead | CFile::typeBinary ) )
	{
		strErrorMsg = TASApp.LoadLocalizedString( IDS_DLGDPSVISIO_FIRMWAREERROR_CANTOPEN ) + _T("\r\n\r\n") + strApplyUpdateMsg;
		return strErrorMsg;
	}

	// Check the file length.
	UINT uiFileSize = (UINT)DFUFile.GetLength();

	if( 0 == uiFileSize )
	{
		strErrorMsg = TASApp.LoadLocalizedString( IDS_DLGDPSVISIO_FIRMWAREERROR_FILEEMPTY ) + _T("\r\n\r\n") + strApplyUpdateMsg;
		return strErrorMsg;
	}

	UINT8 uiPrefixSize = sizeof( DFUPREFIX );
	union 
	{
		DFUPREFIX rDFUPrefix;
		UINT8 uiBuffer[sizeof( DFUPREFIX )];
	}_DFUPrefix;

	if( uiPrefixSize != DFUFile.Read( _DFUPrefix.uiBuffer, uiPrefixSize ) )
	{
		DFUFile.Close();

		// Error when reading the DFU file.
		strErrorMsg = TASApp.LoadLocalizedString( IDS_DLGDPSVISIO_DFUERROR_READERROR ) + _T("\r\n\r\n") + strApplyUpdateMsg;
		return strErrorMsg;
	}

	if( 0 != strncmp( _DFUPrefix.rDFUPrefix.szSignature, "DfuSe", PREFIX_SIGNATURE_SIZE ) )
	{
		DFUFile.Close();

		// Error when reading the DFU file.
		strErrorMsg = TASApp.LoadLocalizedString( IDS_DLGDPSVISIO_DFUERROR_READERROR ) + _T("\r\n\r\n") + strApplyUpdateMsg;
		return strErrorMsg;
	}

	UINT8 uiVersionSize = sizeof( UINT32 );
	UINT8 uiVersionAndSuffixeSize = uiVersionSize + sizeof( DFUSUFFIX );
	
	union
	{
		struct
		{
			UINT32 uiVersion;
			DFUSUFFIX rDFUSuffix;
		}_VersionAndSuffix;
		UINT8 uiBuffer[sizeof( UINT32 ) + sizeof( DFUSUFFIX )];
	}_DFUSuffix;

	// We read suffix and DpS-Visio firmware version that precedes.
	DFUFile.Seek( -uiVersionAndSuffixeSize, CFile::end );

	if( uiVersionAndSuffixeSize != DFUFile.Read( _DFUSuffix.uiBuffer, uiVersionAndSuffixeSize ) )
	{
		DFUFile.Close();

		// Error when reading the DFU file.
		strErrorMsg = TASApp.LoadLocalizedString( IDS_DLGDPSVISIO_DFUERROR_READERROR ) + _T("\r\n\r\n") + strApplyUpdateMsg;
		return strErrorMsg;
	}

	if( 0 != strncmp( _DFUSuffix._VersionAndSuffix.rDFUSuffix.ucDfuSignature, "UFD", 3 ) )
	{
		DFUFile.Close();

		// Error when reading the DFU file.
		strErrorMsg = TASApp.LoadLocalizedString( IDS_DLGDPSVISIO_DFUERROR_READERROR ) + _T("\r\n\r\n") + strApplyUpdateMsg;
		return strErrorMsg;
	}

	UINT16 uiVendorID = ( _DFUSuffix._VersionAndSuffix.rDFUSuffix.idVendorHi << 8 ) ;
	uiVendorID += _DFUSuffix._VersionAndSuffix.rDFUSuffix.idVendorLo;

	if( DFUDEVICE_VID_VALUE != uiVendorID )
	{
		DFUFile.Close();

		// Error when reading the DFU file.
		strErrorMsg = TASApp.LoadLocalizedString( IDS_DLGDPSVISIO_DFUERROR_READERROR ) + _T("\r\n\r\n") + strApplyUpdateMsg;
		return strErrorMsg;
	}

	// Expected value SW_VERSION_HEX		0x20
	UINT8 uiVersion = _DFUSuffix._VersionAndSuffix.uiVersion & 0x000000FF;
	UINT8 uiVersionHi = ( uiVersion & 0xF0 ) >> 4;
	UINT8 uiVersionLo = uiVersion & 0x0F;

	m_arsFirmwareData[iWhichDFU].m_strRevision.Format( _T("%u.%u"), uiVersionHi, uiVersionLo );
	m_arsFirmwareData[iWhichDFU].m_strFilename = strDFUFilename;

	return _T("");
}

bool CDlgDpSVisio::_CheckIfHIDDeviceConnected()
{
	struct hid_device_info *pDeviceInfo;
	struct hid_device_info *pCurrentDevice;

	pDeviceInfo = hid_enumerate( HIDDEVICE_VID_VALUE, HIDDEVICE_PID_VALUE );
	pCurrentDevice = pDeviceInfo;
	CString str;
	bool bIsconnected = false;
	CString strDeviceName = _T( "" );

	while( NULL != pCurrentDevice && false == bIsconnected )
	{
		strDeviceName = CString( CA2W( pCurrentDevice->path ) );

		if( true == _IsDpSVisioHID( strDeviceName ) )
		{
			bIsconnected = true;
		}
		else
		{
			pCurrentDevice = pCurrentDevice->next;
		}
	}

	if( true == bIsconnected )
	{
		_ConnectHIDDevice( strDeviceName );
	}

	hid_free_enumeration( pDeviceInfo );
	return bIsconnected;
}

bool CDlgDpSVisio::_CheckIfDFUSeDeviceConnected()
{
	GUID Guid = GUID_DFUSE;
	HDEVINFO hDevInfo = SetupDiGetClassDevs( &Guid, NULL, NULL, DIGCF_PRESENT | DIGCF_INTERFACEDEVICE );

	if( INVALID_HANDLE_VALUE == hDevInfo )
	{
		return false;
	}

	SP_INTERFACE_DEVICE_DATA rInterfaceDeviceData;
	rInterfaceDeviceData.cbSize = sizeof( SP_INTERFACE_DEVICE_DATA );

	bool bIsConnected = false;
	CString strDeviceName = _T( "" );

	for( DWORD dwDeviceIndex = 0; TRUE == SetupDiEnumDeviceInterfaces( hDevInfo, NULL, &Guid, dwDeviceIndex, &rInterfaceDeviceData); ++dwDeviceIndex )
	{
		DWORD dwDetailsSize;
		SetupDiGetDeviceInterfaceDetail( hDevInfo, &rInterfaceDeviceData, NULL, 0, &dwDetailsSize, NULL );

		PSP_INTERFACE_DEVICE_DETAIL_DATA pDetails = (PSP_INTERFACE_DEVICE_DETAIL_DATA)new BYTE[dwDetailsSize];
		pDetails->cbSize = sizeof( SP_INTERFACE_DEVICE_DETAIL_DATA );
		SP_DEVINFO_DATA rDeviceInfoData = { sizeof( SP_DEVINFO_DATA ) };

		if( FALSE == SetupDiGetDeviceInterfaceDetail( hDevInfo, &rInterfaceDeviceData, pDetails, dwDetailsSize, NULL, &rDeviceInfoData ) )
		{
			continue;
		}

		bool bIsDpSVisioDFU = _IsDpSVisioDFUSe( pDetails->DevicePath );
		
		if( true == bIsDpSVisioDFU )
		{
			strDeviceName = pDetails->DevicePath;
			bIsConnected = true;
		}

		delete[] pDetails;

		if( true == bIsDpSVisioDFU )
		{
			break;
		}
	}

	if( true == bIsConnected )
	{
		_ConnectDFUSeDevice( strDeviceName );
	}

	return bIsConnected;
}

bool CDlgDpSVisio::_CheckIfNewDFUSeDeviceConnected()
{
	GUID Guid = GUID_NEWDFUSE;
	HDEVINFO hDevInfo = SetupDiGetClassDevs( &Guid, NULL, NULL, DIGCF_PRESENT | DIGCF_INTERFACEDEVICE );

	if( INVALID_HANDLE_VALUE == hDevInfo )
	{
		return false;
	}

	SP_INTERFACE_DEVICE_DATA rInterfaceDeviceData;
	rInterfaceDeviceData.cbSize = sizeof( SP_INTERFACE_DEVICE_DATA );

	bool bIsConnected = false;
	CString strDeviceName = _T( "" );

	for( DWORD dwDeviceIndex = 0; TRUE == SetupDiEnumDeviceInterfaces( hDevInfo, NULL, &Guid, dwDeviceIndex, &rInterfaceDeviceData); ++dwDeviceIndex )
	{
		DWORD dwDetailsSize;
		SetupDiGetDeviceInterfaceDetail( hDevInfo, &rInterfaceDeviceData, NULL, 0, &dwDetailsSize, NULL );

		PSP_INTERFACE_DEVICE_DETAIL_DATA pDetails = (PSP_INTERFACE_DEVICE_DETAIL_DATA)new BYTE[dwDetailsSize];
		pDetails->cbSize = sizeof( SP_INTERFACE_DEVICE_DETAIL_DATA );
		SP_DEVINFO_DATA rDeviceInfoData = { sizeof( SP_DEVINFO_DATA ) };

		if( FALSE == SetupDiGetDeviceInterfaceDetail( hDevInfo, &rInterfaceDeviceData, pDetails, dwDetailsSize, NULL, &rDeviceInfoData ) )
		{
			continue;
		}

		bool bIsDpSVisioDFU = _IsDpSVisioNewDFUSe( pDetails->DevicePath );

		if( true == bIsDpSVisioDFU )
		{
			strDeviceName = pDetails->DevicePath;
			bIsConnected = true;
		}

		delete[] pDetails;

		if( true == bIsDpSVisioDFU )
		{
			break;
		}
	}

	if( true == bIsConnected )
	{
		_ConnectNewDFUSeDevice( strDeviceName );
	}

	return bIsConnected;
}

bool CDlgDpSVisio::_IsDpSVisioHID( CString strDeviceName )
{
	// Check VID.
	strDeviceName.MakeUpper();
	int iPos = strDeviceName.Find( _T("VID_") );

	if( -1 == iPos )
	{
		return false;
	}

	if( 0 != strDeviceName.Mid( iPos + 4, 4 ).Compare( HIDDEVICE_VID_STRING ) )
	{
		return false;
	}

	// Check PID.
	iPos = strDeviceName.Find( _T("PID_") );

	if( -1 == iPos )
	{
		return false;
	}

	if( 0 != strDeviceName.Mid( iPos + 4, 4 ).Compare( HIDDEVICE_PID_STRING ) )
	{
		return false;
	}

	// Try to open the HID device.
	hid_device *pHIDDpSVisio = hid_open( HIDDEVICE_VID_VALUE, HIDDEVICE_PID_VALUE, NULL );

	if( NULL == pHIDDpSVisio )
	{
		OutputDebugString( _T("unable to open device\n") );
		return false;
	}

	// Read the manufacturer string.
	wchar_t wstr[255];
	wstr[0] = 0x0000;
	int iResult = hid_get_manufacturer_string( pHIDDpSVisio, wstr, 255 );

	if( iResult < 0 )
	{
		OutputDebugString( _T("Unable to read manufacturer string\n") );
		hid_close( pHIDDpSVisio );
		hid_exit();
		return false;
	}

	if( 0 != CString( wstr ).MakeUpper().Compare( HIDDEVICE_MANUFACTURER_STRING ) )
	{
		OutputDebugString( _T("Not the good manufacturer\n") );
		hid_close( pHIDDpSVisio );
		hid_exit();
		return false;
	}

	// Read the product string.
	wstr[0] = 0x0000;
	iResult = hid_get_product_string( pHIDDpSVisio, wstr, 255 );

	if( iResult < 0 )
	{
		OutputDebugString( _T("Unable to read product string\n") );
		hid_close( pHIDDpSVisio );
		hid_exit();
		return false;
	}

	if( 0 != CString( wstr ).Left( CString( HIDDEVICE_PRODUCT_STRING ).GetLength() ).MakeUpper().Compare( HIDDEVICE_PRODUCT_STRING ) )
	{
		OutputDebugString( _T("Not the good product.\n") );
		hid_close( pHIDDpSVisio );
		hid_exit();
		return false;
	}

	hid_close( pHIDDpSVisio );
	hid_exit();

	return true;
}

bool CDlgDpSVisio::_IsDpSVisioDFUSe( CString strDeviceName )
{
	// Check VID.
	strDeviceName.MakeUpper();
	int iPos = strDeviceName.Find( _T("VID_") );

	if( -1 == iPos )
	{
		return false;
	}

	if( 0 != strDeviceName.Mid( iPos + 4, 4 ).Compare( DFUDEVICE_VID_STRING ) )
	{
		return false;
	}

	// Check PID.
	iPos = strDeviceName.Find( _T("PID_") );

	if( -1 == iPos )
	{
		return false;
	}

	if( 0 != strDeviceName.Mid( iPos + 4, 4 ).Compare( DFUDEVICE_PID_STRING ) )
	{
		return false;
	}

	// Try to open the DFU Se device.
	HANDLE hDle;
	USES_CONVERSION;
	
	if( STDFU_NOERROR != STDFU_Open( (CT2A)(LPCTSTR)strDeviceName, &hDle ) )
	{
		OutputDebugString( _T("unable to open device\n") );
		return false;
	}

	// Get descriptor.
	USB_DEVICE_DESCRIPTOR DeviceDesc;

	if( STDFU_NOERROR != STDFU_GetDeviceDescriptor( &hDle, &DeviceDesc ) )
	{
		OutputDebugString( _T("unable to get descriptor\n") );
		return false;
	}

	// Read the manufacturer string.
	char cStringDescriptor[255];
	cStringDescriptor[0] = 0x0000;

	if( STDFU_NOERROR != STDFU_GetStringDescriptor( &hDle, DeviceDesc.iManufacturer, cStringDescriptor, 255 ) )
	{
		OutputDebugString( _T("Unable to read manufacturer string\n") );
		STDFU_Close( &hDle );
		return false;
	}

	CString strManufacturer = A2T( CStringA( cStringDescriptor ) );
	strManufacturer.MakeUpper();

	if( 0 != strManufacturer.Compare( DFUDEVICE_MANUFACTURER_STRING ) )
	{
		OutputDebugString( _T("Not the good manufacturer\n") );
		STDFU_Close( &hDle );
		return false;
	}

	// Read the product string.
	cStringDescriptor[0] = 0x0000;

	if( STDFU_NOERROR != STDFU_GetStringDescriptor( &hDle, DeviceDesc.iProduct, cStringDescriptor, 255 ) )
	{
		OutputDebugString( _T("Unable to read product string\n") );
		STDFU_Close( &hDle );
		return false;
	}

	CString strProduct = A2T( CStringA( cStringDescriptor ) );
	strProduct.MakeUpper();

	if( 0 != strProduct.Left( CString( DFUDEVICE_PRODUCT_STRING ).GetLength() ).Compare( DFUDEVICE_PRODUCT_STRING ) )
	{
		OutputDebugString( _T("Not the good product.\n") );
		STDFU_Close( &hDle );
		return false;
	}

	STDFU_Close( &hDle );
	return true;
}

bool CDlgDpSVisio::_IsDpSVisioNewDFUSe( CString strDeviceName )
{
	if( NULL == m_pUSBContext )
	{
		OutputDebugString( _T("Internal error: USB interface not initialized for DFU Se new driver.\n") );
		return false;
	}

	// Check VID.
	strDeviceName.MakeUpper();
	int iPos = strDeviceName.Find( _T("VID_") );

	if( -1 == iPos )
	{
		return false;
	}

	if( 0 != strDeviceName.Mid( iPos + 4, 4 ).Compare( DFUDEVICE_VID_STRING ) )
	{
		return false;
	}

	// Check PID.
	iPos = strDeviceName.Find( _T("PID_") );

	if( -1 == iPos )
	{
		return false;
	}

	if( 0 != strDeviceName.Mid( iPos + 4, 4 ).Compare( DFUDEVICE_PID_STRING ) )
	{
		return false;
	}

	// Check if device is well existing.
	probe_devices( m_pUSBContext, &m_rNewDFUSeVariables );

	if( NULL == m_rNewDFUSeVariables.dfu_root )
	{
		OutputDebugString( _T("CDlgDpSVisio::_IsDpSVisioNewDFUSe: no valid DFU device found!") );
		return false;
	}

	// Try to open the new DFU Se device.
	int iReturn = libusb_open( m_rNewDFUSeVariables.dfu_root->dev, &m_rNewDFUSeVariables.dfu_root->dev_handle );
	
	if( iReturn || NULL == m_rNewDFUSeVariables.dfu_root->dev_handle )
	{
		CString str;
		str.Format( _T("Cannot open device: %s"), libusb_error_name( iReturn ) );
		OutputDebugString( str );

		disconnect_devices( &m_rNewDFUSeVariables );
		return false;
	}

	// Get descriptor.
	struct libusb_device_descriptor desc;
	iReturn = libusb_get_device_descriptor( m_rNewDFUSeVariables.dfu_root->dev, &desc );

	if( iReturn )
	{
		OutputDebugString( _T("Can't retrieve descriptor.\n") );
		_CloseNewDFUSeDeviceHelper( true );
		return false;
	}

	// Read the manufacturer string.
	TCHAR cStringDescriptor[255];
	iReturn = get_utf8_string_descriptor( m_rNewDFUSeVariables.dfu_root->dev_handle, desc.iManufacturer, (unsigned char *)cStringDescriptor, sizeof( cStringDescriptor ) );

	if( iReturn < 0 )
	{
		OutputDebugString( _T("Failed to retrieve the manufacturer string.\n") );
		_CloseNewDFUSeDeviceHelper( true );
		return false;
	}

	USES_CONVERSION;
	cStringDescriptor[iReturn >> 1] = _T('\0');
	CString strManufacturer = cStringDescriptor;
	strManufacturer.MakeUpper();

	if( 0 != strManufacturer.Compare( DFUDEVICE_MANUFACTURER_STRING ) )
	{
		OutputDebugString( _T("Not the good manufacturer\n") );
		_CloseNewDFUSeDeviceHelper( true );
		return false;
	}

	// Read the product string.
	cStringDescriptor[0] = 0x0000;

	iReturn = get_utf8_string_descriptor( m_rNewDFUSeVariables.dfu_root->dev_handle, desc.iProduct, (unsigned char *)cStringDescriptor, sizeof( cStringDescriptor ) );

	if( iReturn < 0 )
	{
		OutputDebugString( _T("Failed to retrieve the product string.\n") );
		_CloseNewDFUSeDeviceHelper( true );
		return false;
	}

	cStringDescriptor[iReturn >> 1] = _T('\0');
	CString strProduct = cStringDescriptor;
	strProduct.MakeUpper();

	if( 0 != strProduct.Left( CString( DFUDEVICE_PRODUCT_STRING ).GetLength() ).Compare( DFUDEVICE_PRODUCT_STRING ) )
	{
		OutputDebugString( _T("Not the good product.\n") );
		_CloseNewDFUSeDeviceHelper( true );
		return false;
	}

	_CloseNewDFUSeDeviceHelper( false );

	return true;
}

void CDlgDpSVisio::_ConnectHIDDevice( CString strDeviceName )
{
	if( NULL != m_pclDpSVisioCommunication )
	{
		// Don't connect more than one DpS-Visio at the same time to process to an update.
		CString str = TASApp.LoadLocalizedString( IDS_DLGDPSVISIO_NOTWODEVICESATSAMETIME );
		AfxMessageBox( str, MB_OK | MB_ICONEXCLAMATION, 0 );
	}
	else
	{
		m_pclDpSVisioCommunication = new CDpSVisioHIDCommunication( this );

		if( NULL != m_pclDpSVisioCommunication )
		{
			m_pclDpSVisioCommunication->DpSVisioConnected( strDeviceName );
		}

		if( NULL != m_pCurrentDialog )
		{
			m_pCurrentDialog->OnDpSVisioConnected( Device_HID );
		}
	}
}

void CDlgDpSVisio::_DisconnectHIDDevice( CString strDeviceName )
{
	if( NULL != m_pclDpSVisioCommunication && NULL != m_pclDpSVisioCommunication->GetDpSVisioInformation() &&
			0 == strDeviceName.Compare( m_pclDpSVisioCommunication->GetDpSVisioInformation()->m_strDeviceName ) )
	{
		if( NULL != m_pCurrentDialog )
		{
			m_pCurrentDialog->OnDpSVisioDisconnected( Device_HID );
		}

		delete m_pclDpSVisioCommunication;
		m_pclDpSVisioCommunication = NULL;
	}
}

void CDlgDpSVisio::_ConnectDFUSeDevice( CString strDeviceName )
{
	OutputDebugString( _T("_ConnectDFUDevice called...\n") );

	if( NULL != m_pclDpSVisioCommunication )
	{
		// Don't connect more than one DpS-Visio at the same time to process to an update.
		CString str = TASApp.LoadLocalizedString( IDS_DLGDPSVISIO_NOTWODEVICESATSAMETIME );
		AfxMessageBox( str, MB_OK | MB_ICONEXCLAMATION, 0 );
	}
	else
	{
		m_pclDpSVisioCommunication = new CDpSVisioDFUCommunication( this );

		if( NULL != m_pclDpSVisioCommunication )
		{
			OutputDebugString( _T("Call 'CDpSVisioCommunication::DpSVisioConnected'.\n") );
			m_pclDpSVisioCommunication->DpSVisioConnected( strDeviceName );
		}

		if( NULL != m_pCurrentDialog )
		{
			OutputDebugString( _T("Call 'CDlgDpSVisioBase::OnDpSVisioConnected( Device_DFU )'.\n") );
			m_pCurrentDialog->OnDpSVisioConnected( Device_DFUSe );
		}
	}
	
	OutputDebugString( _T("_ConnectDFUDevice end.\n") );
}

void CDlgDpSVisio::_DisconnectDFUSeDevice( CString strDeviceName )
{
	if( NULL != m_pclDpSVisioCommunication && NULL != m_pclDpSVisioCommunication->GetDpSVisioInformation() &&
			0 == strDeviceName.Compare( m_pclDpSVisioCommunication->GetDpSVisioInformation()->m_strDeviceName ) )
	{
		if( NULL != m_pCurrentDialog )
		{
			m_pCurrentDialog->OnDpSVisioDisconnected( Device_DFUSe );
		}

		delete m_pclDpSVisioCommunication;
		m_pclDpSVisioCommunication = NULL;
	}
}

void CDlgDpSVisio::_ConnectNewDFUSeDevice( CString strDeviceName )
{
	OutputDebugString( _T("_ConnectNewDFUSeDevice called...\n") );

	if( NULL != m_pclDpSVisioCommunication )
	{
		// Don't connect more than one DpS-Visio at the same time to process to an update.
		CString str = TASApp.LoadLocalizedString( IDS_DLGDPSVISIO_NOTWODEVICESATSAMETIME );
		AfxMessageBox( str, MB_OK | MB_ICONEXCLAMATION, 0 );
	}
	else
	{
		m_pclDpSVisioCommunication = new CDpSVisioNewDFUCommunication( this, &m_rNewDFUSeVariables );

		if( NULL != m_pclDpSVisioCommunication )
		{
			OutputDebugString( _T("Call 'CDpSVisioCommunication::DpSVisioConnected'.\n") );
			m_pclDpSVisioCommunication->DpSVisioConnected( strDeviceName );
		}

		if( NULL != m_pCurrentDialog )
		{
			OutputDebugString( _T("Call 'CDlgDpSVisioBase::OnDpSVisioConnected( Device_DFU )'.\n") );
			m_pCurrentDialog->OnDpSVisioConnected( Device_NewDFUSe );
		}
	}

	OutputDebugString( _T("_ConnectNewDFUSeDevice end.\n") );
}

void CDlgDpSVisio::_DisconnectNewDFUSeDevice( CString strDeviceName )
{
	if( NULL != m_pclDpSVisioCommunication && NULL != m_pclDpSVisioCommunication->GetDpSVisioInformation() &&
			0 == strDeviceName.Compare( m_pclDpSVisioCommunication->GetDpSVisioInformation()->m_strDeviceName ) )
	{
		if( NULL != m_pCurrentDialog )
		{
			m_pCurrentDialog->OnDpSVisioDisconnected( Device_NewDFUSe );
		}

		delete m_pclDpSVisioCommunication;
		m_pclDpSVisioCommunication = NULL;
	}
}

void CDlgDpSVisio::_CloseNewDFUSeDeviceHelper( bool bAndDisconnect )
{
	if( NULL != m_rNewDFUSeVariables.dfu_root && NULL != m_rNewDFUSeVariables.dfu_root->dev_handle )
	{
		if( NULL != m_rNewDFUSeVariables.dfu_root->interface )
		{
			libusb_release_interface( m_rNewDFUSeVariables.dfu_root->dev_handle, m_rNewDFUSeVariables.dfu_root->interface );
		}
		
		libusb_close( m_rNewDFUSeVariables.dfu_root->dev_handle );
		m_rNewDFUSeVariables.dfu_root->dev_handle = NULL;
	}

	if( true == bAndDisconnect )
	{
		disconnect_devices( &m_rNewDFUSeVariables );
	}
}

CString CDlgDpSVisio::_GetExceptionString( UINT uiCode )
{
	CString strError( _T("Unknown error.") );

	switch( uiCode )
	{
		case CFileException::none:
			strError = _T("No error occurred.");
			break;

		case CFileException::genericException:
			strError = _T("An unspecified error occurred.");
			break;

		case CFileException::fileNotFound:
			strError = _T("The file could not be located.");
			break;

		case CFileException::badPath:
			strError = _T("All or part of the path is invalid.");
			break;

		case CFileException::tooManyOpenFiles:
			strError = _T("The permitted number of open files was exceeded.");
			break;

		case CFileException::accessDenied:
			strError = _T("The file could not be accessed.");
			break;

		case CFileException::invalidFile:
			strError = _T("There was an attempt to use an invalid file handle.");
			break;

		case CFileException::removeCurrentDir:
			strError = _T("The current working directory cannot be removed.");
			break;

		case CFileException::directoryFull:
			strError = _T("There are no more directory entries.");
			break;

		case CFileException::badSeek:
			strError = _T("There was an error trying to set the file pointer.");
			break;

		case CFileException::hardIO:
			strError = _T("There was a hardware error.");
			break;

		case CFileException::sharingViolation:
			strError = _T("SHARE.EXE was not loaded, or a shared region was locked.");
			break;

		case CFileException::lockViolation:
			strError = _T("There was an attempt to lock a region that was already locked.");
			break;

		case CFileException::diskFull:
			strError = _T("The disk is full.");
			break;

		case CFileException::endOfFile:
			strError = _T("The end of file was reached.");
			break;
	}

	return strError;
}

