#include "stdafx.h"
#include "afxdialogex.h"
#include "DpSVisioIncludes.h"

IMPLEMENT_DYNAMIC( CDlgDpSVisioDFUUpgradeRunning, CDialogEx )

CDlgDpSVisioDFUUpgradeRunning::CDlgDpSVisioDFUUpgradeRunning( CDlgDpSVisio *pParent )
	: CDlgDpSVisioBase( CDlgDpSVisioDFUUpgradeRunning::IDD, pParent )
{
	InitializeCriticalSection( &m_CriticalSection );
	m_eStateEngine = HID_StartDFU_SendCmd;
	m_strHIDDeviceName = _T("");
	m_pThread = NULL;
	m_pclStopThreadEvent = NULL;
	m_pclThreadStoppedEvent = NULL;
	m_pDlgDpSVisioUnplugPlug = NULL;
	m_pDlgDpSVisioDFUInstallation = NULL;
	m_vecMsgToPrintInListBox.clear();
	m_nTimer = (UINT_PTR)0;
}

CDlgDpSVisioDFUUpgradeRunning::~CDlgDpSVisioDFUUpgradeRunning()
{
	if( NULL != GetSafeHwnd() && (UINT_PTR)0 != m_nTimer )
	{
		KillTimer( m_nTimer );
	}

	if( NULL != m_pclStopThreadEvent )
	{
		delete m_pclStopThreadEvent;
		m_pclStopThreadEvent = NULL;
	}

	if( NULL != m_pclThreadStoppedEvent )
	{
		delete m_pclThreadStoppedEvent;
		m_pclThreadStoppedEvent = NULL;
	}

	if( NULL != m_pThread )
	{
		delete m_pThread;
		m_pThread = NULL;
	}

	DeleteCriticalSection( &m_CriticalSection );
}

UINT CDlgDpSVisioDFUUpgradeRunning::ThreadUpgradeRunning( LPVOID pParam )
{
	CDlgDpSVisioDFUUpgradeRunning *pclDlgDpSVisioDFUUpgradeRunning = (CDlgDpSVisioDFUUpgradeRunning *)pParam;

	if( NULL == pclDlgDpSVisioDFUUpgradeRunning )
	{
		return -1;
	}

	bool bStop = false;

	while( false == bStop )
	{
		DWORD dwReturn = WaitForSingleObject( pclDlgDpSVisioDFUUpgradeRunning->m_pclStopThreadEvent->m_hObject, DLGVISIODFUUPDATERUNNING_TIMERMSEC );

		switch( dwReturn )
		{
			// Stop.
			case WAIT_OBJECT_0:
				bStop = true;
				break;

			// Timeout.
			case WAIT_TIMEOUT:

				pclDlgDpSVisioDFUUpgradeRunning->_UpgradeProgess();
				break;
		}
	}

	pclDlgDpSVisioDFUUpgradeRunning->m_pclThreadStoppedEvent->SetEvent();
	return 0;
}

void CDlgDpSVisioDFUUpgradeRunning::SetParam( LPARAM lpParam )
{
	if( CDlgDpSVisio::UpdateParam_HID == (CDlgDpSVisio::UpdateParam)lpParam )
	{
		m_eStateEngine = HID_StartDFU_SendCmd;
	}
	else if ( CDlgDpSVisio::UpdateParam_DFU == ( CDlgDpSVisio::UpdateParam )lpParam )
	{
		m_eStateEngine = DFU_StartErase;
	} 
}

void CDlgDpSVisioDFUUpgradeRunning::Stop()
{
	_StopThreadUpgradeRunning();
}

void CDlgDpSVisioDFUUpgradeRunning::OnDpSVisioConnected( CDlgDpSVisio::WhichDevice eWhichDevice )
{
	OutputDebugString( _T("OnDpSVisioConnected called...\n") );
	CString str;
	
	EnterCriticalSection( &m_CriticalSection );

	switch( m_eStateEngine )
	{
		case HID_StartDFU_WaitDFUAttach:
			
			OutputDebugString( _T("HID_StartDFU_WaitDFUAttach state engine...\n") );
			
			if( CDlgDpSVisio::Device_DFUSe == eWhichDevice || CDlgDpSVisio::Device_NewDFUSe == eWhichDevice )
			{
				OutputDebugString( _T("The device is well in DFU mode.\n") );

				// "The DpS-Visio is in DFU mode."
				str = TASApp.LoadLocalizedString( IDS_DLGDPSVISIO_DFUUPGRADERUNNING_DFUATTACHED );
				INSERT_STRING_IN_LIST_AND_SCROLL( m_UpdateOutput, str );

				// "Start erasing the DpS-Visio memory."
				str = TASApp.LoadLocalizedString( IDS_DLGDPSVISIO_DFUUPGRADERUNNING_STARTERASING );
				INSERT_STRING_IN_LIST_AND_SCROLL( m_UpdateOutput, str );

				m_eStateEngine = DFU_StartErase;
			}
			else
			{
				OutputDebugString( _T("The device is not in DFU mode.\n") );
			}

			break;

		case DFU_StartHDI_WaitHIDAttach:

			OutputDebugString( _T("DFU_StartHDI_WaitHIDAttach state engine...\n") );

			if( CDlgDpSVisio::Device_HID == eWhichDevice )
			{
				OutputDebugString( _T("The device is well in HID mode.\n") );

				// "The DpS-Visio is in HDI mode."
				str = TASApp.LoadLocalizedString( IDS_DLGDPSVISIO_UPGRADERUNNING_HIDATTACHED );
				INSERT_STRING_IN_LIST_AND_SCROLL( m_UpdateOutput, str );

				if( NULL != m_pDlgDpSVisioUnplugPlug )
				{
					m_pDlgDpSVisioUnplugPlug->DestroyWindow();
					delete m_pDlgDpSVisioUnplugPlug;
					m_pDlgDpSVisioUnplugPlug = NULL;
				}

				m_eStateEngine = HID_VerifyDevice;
			}
			else
			{
				OutputDebugString( _T("The device is not in HID mode.\n") );
			}

			break;

		default:
			str.Format( _T("default state engine: %i\n"), (int)m_eStateEngine );
			OutputDebugString( str );
			// _StopThreadUpgradeRunning();
			break;
	}
	
	OutputDebugString( _T("OnDpSVisioConnected end.\n") );
	LeaveCriticalSection( &m_CriticalSection );
}

void CDlgDpSVisioDFUUpgradeRunning::OnDpSVisioDisconnected( CDlgDpSVisio::WhichDevice eWhichDevice )
{
	OutputDebugString( _T("OnDpSVisioDisconnected called...\n") );
	CString str;

	EnterCriticalSection( &m_CriticalSection );

	switch( m_eStateEngine )
	{
		case HID_StartDFU_WaitHIDDetach:
			
			OutputDebugString( _T("HID_StartDFU_WaitHIDDetach state engine...\n") );

			if( CDlgDpSVisio::Device_HID == eWhichDevice )
			{
				OutputDebugString( _T("The device is well in HID mode.\n") );

				// Remarks: In HID mode, we receive here disconnected event only for the HID device that was connected and not
				//          for other ones.

				// "... HID 'DETACH' event received."
				str = TASApp.LoadLocalizedString( IDS_DLGDPSVISIO_UPGRADERUNNING_HIDDETACHED );
				INSERT_STRING_IN_LIST_AND_SCROLL( m_UpdateOutput, str );

				// "... wait the DFU 'ATTACH' event."
				str = TASApp.LoadLocalizedString( IDS_DLGDPSVISIO_DFUUPGRADERUNNING_WAITDFUDETACH );
				INSERT_STRING_IN_LIST_AND_SCROLL( m_UpdateOutput, str );

				m_uiUSBEventCounter = 0;
				m_eStateEngine = HID_StartDFU_WaitDFUAttach;

				// Now we try to detect DFU by Windows event or by polling.
				m_pParent->StartDFUPollingDetection();
			}
			else
			{
				OutputDebugString( _T("The device is not in HID mode.\n") );
			}

			break;

		case DFU_StartHDI_WaitDFUDetach:

			OutputDebugString( _T("DFU_StartHDI_WaitDFUDetach state engine...\n") );

			if( CDlgDpSVisio::Device_DFUSe == eWhichDevice || CDlgDpSVisio::Device_NewDFUSe == eWhichDevice )
			{
				OutputDebugString( _T("The device is well in DFU mode.\n") );

				// "... DFU 'DETACH' event received."
				str = TASApp.LoadLocalizedString( IDS_DLGDPSVISIO_DFUUPGRADERUNNING_DFUDETACHED );
				INSERT_STRING_IN_LIST_AND_SCROLL( m_UpdateOutput, str );

				// "... wait the HID 'ATTACH' event."
				str = TASApp.LoadLocalizedString( IDS_DLGDPSVISIO_UPGRADERUNNING_WAITHIDATTACH );
				INSERT_STRING_IN_LIST_AND_SCROLL( m_UpdateOutput, str );

				if( NULL != m_pDlgDpSVisioUnplugPlug )
				{
					m_pDlgDpSVisioUnplugPlug->SetText( TASApp.LoadLocalizedString( IDS_DLGDPSVISIO_DFUUPGRADERUNNING_PLUGUSBCABLE ), DLGVISIODFUUPDATERUNNING_UNPLUGPLUGTEXTCOLOR, 10, true );
				}

				m_uiUSBEventCounter = 0;
				m_eStateEngine = DFU_StartHDI_WaitHIDAttach;
			}
			else
			{
				OutputDebugString( _T("The device is well in DFU mode.\n") );
			}

			break;

		case HID_StartDFU_WaitDFUAttach:

			OutputDebugString( _T("HID_StartDFU_WaitDFUAttach state engine...\n") );

			// It's possible to get a DFU attach event because now we are not waiting anymore this event. Because on some computers
			// and depending on some Windows version, when the DpS-Visio goes in DFU mode, Windows doesn't detect that device leaves the
			// HID mode.

			break;

		default:
			OutputDebugString( _T("default state engine...\n") );

			m_eStateEngine = DeviceDisconnected;
			break;
	}

	OutputDebugString( _T("OnDpSVisioDisconnected end.\n") );
	LeaveCriticalSection( &m_CriticalSection );
}

BEGIN_MESSAGE_MAP( CDlgDpSVisioDFUUpgradeRunning, CDialogEx )
	ON_WM_TIMER()
	ON_MESSAGE( WM_USER_DPSVISIO_STOPTHREADUPGRADE, OnThreadFinished )
	ON_MESSAGE( WM_USER_DPSVISIO_SHOWDLGUNPLUGPLUG, OnShowDlgDpSVisioUnplugPlug )
	ON_MESSAGE( WM_USER_DPSVISIO_FISNISHDLGUNPLUGPLUG, OnFinishDlgDpSVisioUnplugPlug )
	ON_MESSAGE( WM_USER_DPSVISIO_STOPDLGUNPLUGPLUG, OnDestroyDlgDpSVisioUnplugPlug )
	ON_MESSAGE( WM_USER_DPSVISIO_SHOWDLGDFUINSTALLATION, OnShowDlgDFUInstallation )
	ON_MESSAGE( WM_USER_DPSVISIO_FISNISHDFUINSTALLATION, OnFinishDlgDpSVisioDFUInstallation )
END_MESSAGE_MAP()

void CDlgDpSVisioDFUUpgradeRunning::DoDataExchange( CDataExchange* pDX )
{
	CDlgDpSVisioBase::DoDataExchange( pDX );
	DDX_Control( pDX, IDC_PROGRESS, m_UpdateProgress );
	DDX_Control( pDX, IDC_UPDATEOUTPUT, m_UpdateOutput );
	DDX_Control( pDX, IDC_STATICINFO, m_StaticInfo );
}

BOOL CDlgDpSVisioDFUUpgradeRunning::OnInitDialog()
{
	CDlgDpSVisioBase::OnInitDialog();

	CDpSVisioBaseCommunication *pclDpSVisioCommunication = m_pParent->GetDpSVisioCommunication();
	CDpSVisioBaseInformation *pclDpSVisionInformation = pclDpSVisioCommunication->GetDpSVisioInformation();

	CString str = TASApp.LoadLocalizedString( IDS_DLGDPSVISIO_UPGRADERUNNING_CURRENTVERSION );
	GetDlgItem( IDC_STATIC_CURRENTVERSIONTEXT )->SetWindowText( str );

	str = pclDpSVisionInformation->m_strShortFirmwareRevision;

	if( true == str.IsEmpty() )
	{
		str = TASApp.LoadLocalizedString( IDS_DLGDPSVISIO_DFUUPGRADERUNNING_DFUMODE );
	}
	
	GetDlgItem( IDC_STATIC_CURRENTVERSIONVALUE )->SetWindowText( str );
	
	str = TASApp.LoadLocalizedString( IDS_DLGDPSVISIO_UPGRADERUNNING_AVAILABLEVERSION );
	GetDlgItem( IDC_STATIC_AVAILABLEVERSIONTEXT )->SetWindowText( str );
	GetDlgItem( IDC_STATIC_AVAILABLEVERSIONVALUE )->SetWindowText( m_pParent->GetAvailableFirmwareRevisionString( CDlgDpSVisio::Device_DFUSe ) );

	m_UpdateProgress.SetRange( 0, 99 );
	m_UpdateProgress.SetStep( 1 );
	m_UpdateProgress.ModifyStyle( 0, PBS_MARQUEE );
	m_UpdateProgress.SetMarquee( TRUE, 30 );
	m_UpdateProgress.SetShowText( false );
	m_UpdateProgress.SetTextBackgroundColor( RGB( 255, 255, 255 ) );
	m_UpdateProgress.SetTextForegroundColor( RGB( 6, 176, 37 ) );
	m_UpdateProgress.SetBackgroundColor( RGB( 230, 230, 230 ) );
	m_UpdateProgress.SetForegroundColor( RGB( 6, 176, 37 ) );

	m_StaticInfo.SetWindowText( _T("") );
	m_UpdateOutput.ModifyStyle( LBS_SORT, 0 );

	// Save HID device name to be sure it's the same when leaving DFU mode to come back in HID mode.
	m_strHIDDeviceName = pclDpSVisionInformation->m_strDeviceName;

	// Create an event to stop the thread when needed.
	m_pclStopThreadEvent = new CEvent( FALSE, TRUE );

	if( NULL == m_pclStopThreadEvent )
	{
		return FALSE;
	}

	m_pclStopThreadEvent->ResetEvent();

	// Create an event to allow the thread to set when it is well stopped.
	m_pclThreadStoppedEvent = new CEvent( FALSE, TRUE );

	if( NULL == m_pclThreadStoppedEvent )
	{
		return FALSE;
	}

	m_pclThreadStoppedEvent->ResetEvent();
	m_pThread = AfxBeginThread( &ThreadUpgradeRunning, ( LPVOID )this );

	// To print message in the listbox.
	m_nTimer = SetTimer( _TIMERID_DLGDPSVISIOUPGRADERUNNINGPUTMSG, 500, NULL );
	
	return TRUE;
}

void CDlgDpSVisioDFUUpgradeRunning::OnTimer( UINT_PTR nIDEvent )
{
	if( 0 == m_vecMsgToPrintInListBox.size() )
	{
		return;
	}

	EnterCriticalSection( &m_CriticalSection );

	for( int i = 0; i < (int)m_vecMsgToPrintInListBox.size(); i++ )
	{
		m_UpdateOutput.SetCurSel( m_UpdateOutput.InsertString( m_UpdateOutput.GetCount(), m_vecMsgToPrintInListBox[i] ) );
	}

	m_vecMsgToPrintInListBox.clear();

	LeaveCriticalSection( &m_CriticalSection );
}

LRESULT CDlgDpSVisioDFUUpgradeRunning::OnThreadFinished( WPARAM wParam, LPARAM lParam )
{
	_StopThreadUpgradeRunning();
	m_pParent->PostMessage( WM_USER_DPSVISIO_SHOWDIALOG, wParam, lParam );
	return 0;
}

LRESULT CDlgDpSVisioDFUUpgradeRunning::OnShowDlgDpSVisioUnplugPlug( WPARAM wParam , LPARAM lParam )
{
	EnterCriticalSection( &m_CriticalSection );

	if( NULL != m_pDlgDpSVisioUnplugPlug )
	{
		m_pDlgDpSVisioUnplugPlug->DestroyWindow();
		delete m_pDlgDpSVisioUnplugPlug;
		m_pDlgDpSVisioUnplugPlug = NULL;
	}

	m_pDlgDpSVisioUnplugPlug = new CDlgDpSVisioUnplugPlug( CDlgDpSVisioUnplugPlug::WM_Infinite, this );

	if( NULL == m_pDlgDpSVisioUnplugPlug )
	{
		LeaveCriticalSection( &m_CriticalSection );
		return 0;
	}

	if( FALSE == m_pDlgDpSVisioUnplugPlug->Create( IDD_DLGDPSVISIO_UNPLUGPLUG, this ) )
	{
		delete m_pDlgDpSVisioUnplugPlug;
		m_pDlgDpSVisioUnplugPlug = NULL;
		LeaveCriticalSection( &m_CriticalSection );
		return 0;
	}

	CRect rectClientUnplugPlug;
	m_pDlgDpSVisioUnplugPlug->GetClientRect( &rectClientUnplugPlug );
	
	CRect rectScreenDlgDpSVisio;
	m_pParent->GetWindowRect( &rectScreenDlgDpSVisio );

	CPoint newPosInDlgDpSVisio( ( rectScreenDlgDpSVisio.Width() - rectClientUnplugPlug.Width() ) >> 1, 
		( rectScreenDlgDpSVisio.Height() - rectClientUnplugPlug.Height() ) >> 1 );

	m_pDlgDpSVisioUnplugPlug->SetWindowPos( NULL, rectScreenDlgDpSVisio.left + newPosInDlgDpSVisio.x, rectScreenDlgDpSVisio.top + newPosInDlgDpSVisio.y,
		-1, -1, SWP_NOSIZE | SWP_NOZORDER );

	m_pDlgDpSVisioUnplugPlug->SetText( TASApp.LoadLocalizedString( IDS_DLGDPSVISIO_DFUUPGRADERUNNING_UNPLUGUSBCABLE ), DLGVISIODFUUPDATERUNNING_UNPLUGPLUGTEXTCOLOR, 10, true );

	m_pDlgDpSVisioUnplugPlug->ShowWindow( SW_SHOW );

	m_pParent->EnableCancelButton( FALSE );

	m_eStateEngine = DFU_DlgUnplugPlugLaunched;

	LeaveCriticalSection( &m_CriticalSection );
	return 0;
}

LRESULT CDlgDpSVisioDFUUpgradeRunning::OnFinishDlgDpSVisioUnplugPlug( WPARAM wParam, LPARAM lParam )
{
	EnterCriticalSection( &m_CriticalSection );

	if( NULL != m_pDlgDpSVisioUnplugPlug )
	{
		m_pDlgDpSVisioUnplugPlug->DestroyWindow();
		delete m_pDlgDpSVisioUnplugPlug;
		m_pDlgDpSVisioUnplugPlug = NULL;
	}

	m_eStateEngine = DFU_DlgUnplugPlugDestroyed;

	LeaveCriticalSection( &m_CriticalSection );
	return 0;
}

LRESULT CDlgDpSVisioDFUUpgradeRunning::OnDestroyDlgDpSVisioUnplugPlug( WPARAM wParam, LPARAM lParam )
{
	EnterCriticalSection( &m_CriticalSection );

	if( NULL != m_pDlgDpSVisioUnplugPlug )
	{
		m_pDlgDpSVisioUnplugPlug->DestroyWindow();
		delete m_pDlgDpSVisioUnplugPlug;
		m_pDlgDpSVisioUnplugPlug = NULL;
	}

	m_eStateEngine = _SetUpgradeError( CDlgDpSVisio::UE_InternalError, DPSVISIOCOM_ERROR_UNPLUPLUGINTERRUPTEDBYUSER, UpdateError );
	
	LeaveCriticalSection( &m_CriticalSection );
	return 0;
}

LRESULT CDlgDpSVisioDFUUpgradeRunning::OnShowDlgDFUInstallation( WPARAM wParam , LPARAM lParam )
{
	EnterCriticalSection( &m_CriticalSection );

	if( NULL != m_pDlgDpSVisioDFUInstallation )
	{
		m_pDlgDpSVisioDFUInstallation->DestroyWindow();
		delete m_pDlgDpSVisioDFUInstallation;
		m_pDlgDpSVisioDFUInstallation = NULL;
	}

	m_pDlgDpSVisioDFUInstallation = new CDlgDpSVisioDFUInstallation( this );

	if( NULL == m_pDlgDpSVisioDFUInstallation )
	{
		LeaveCriticalSection( &m_CriticalSection );
		return 0;
	}

	if( FALSE == m_pDlgDpSVisioDFUInstallation->Create( IDD_DLGDPSVISIO_DFUINSTALLATION, this ) )
	{
		delete m_pDlgDpSVisioDFUInstallation;
		m_pDlgDpSVisioDFUInstallation = NULL;
		LeaveCriticalSection( &m_CriticalSection );
		return 0;
	}

	CRect rectClientDFUInstallation;
	m_pDlgDpSVisioDFUInstallation->GetClientRect( &rectClientDFUInstallation );
	
	CRect rectScreenDlgDpSVisio;
	m_pParent->GetWindowRect( &rectScreenDlgDpSVisio );

	CPoint newPosInDlgDpSVisio( ( rectScreenDlgDpSVisio.Width() - rectClientDFUInstallation.Width() ) >> 1, 
		( rectScreenDlgDpSVisio.Height() - rectClientDFUInstallation.Height() ) >> 1 );

	m_pDlgDpSVisioDFUInstallation->SetWindowPos( NULL, rectScreenDlgDpSVisio.left + newPosInDlgDpSVisio.x, rectScreenDlgDpSVisio.top + newPosInDlgDpSVisio.y,
		-1, -1, SWP_NOSIZE | SWP_NOZORDER );

	m_pDlgDpSVisioDFUInstallation->SetText( TASApp.LoadLocalizedString( IDS_DLGDPSVISIODFUINSTALLATION_INFO ), _BLACK, 10, false );

	m_pDlgDpSVisioDFUInstallation->ShowWindow( SW_SHOW );

	m_pParent->EnableCancelButton( FALSE );

	m_eStateEngine = DFU_DlgDFUInstallationLaunched;

	LeaveCriticalSection( &m_CriticalSection );
	return 0;
}

LRESULT CDlgDpSVisioDFUUpgradeRunning::OnFinishDlgDpSVisioDFUInstallation( WPARAM wParam, LPARAM lParam )
{
	EnterCriticalSection( &m_CriticalSection );
	m_eStateEngine = WaitingToBeKilled;
	LeaveCriticalSection( &m_CriticalSection );

	if( NULL != m_pDlgDpSVisioDFUInstallation )
	{
		m_pDlgDpSVisioDFUInstallation->DestroyWindow();
		delete m_pDlgDpSVisioDFUInstallation;
		m_pDlgDpSVisioDFUInstallation = NULL;
	}

	Stop();

	m_pParent->PostMessage( WM_USER_DPSVISIO_EXIT, 0, 0 );

	return 0;
}

void CDlgDpSVisioDFUUpgradeRunning::_UpgradeProgess()
{
	EnterCriticalSection( &m_CriticalSection );

	CString str;
	CDpSVisioBaseCommunication *pclDpSVisioCommunication = m_pParent->GetDpSVisioCommunication();

	if( DeviceDisconnected == m_eStateEngine )
	{
		m_eStateEngine = _SetUpgradeError( CDlgDpSVisio::UE_DeviceDisconnected, 0 );
	}

	BYTE bPercentCompleted = 0;
	bool bContinue = true;

	switch( m_eStateEngine )
	{
		case HID_StartDFU_SendCmd:
			
			// "Set the DpS-Visio in the DFU mode."
			str = TASApp.LoadLocalizedString( IDS_DLGDPSVISIO_DFUUPGRADERUNNING_STARTDFU );
			m_vecMsgToPrintInListBox.push_back( str );

			if( CDpSVisioBaseCommunication::CT_HID != pclDpSVisioCommunication->GetCommunicationType() )
			{
				_SetUpgradeError( CDlgDpSVisio::UE_DeviceChanged, 0 );
				bContinue = false;
			}

			if( true == bContinue )
			{
				int iReturnCode = pclDpSVisioCommunication->SendDpSVisioCommand( USBCMD_JUMPTODFU, 0, NULL, &m_UpdateOutput );

				if( DPSVISIOCOM_NOERROR != iReturnCode )
				{
					m_eStateEngine = _SetUpgradeError( CDlgDpSVisio::UE_InternalError, iReturnCode );
					bContinue = false;
				}
			}

			if( true == bContinue )
			{
				// "Command to start DFU well sent."
				str = TASApp.LoadLocalizedString( IDS_DLGDPSVISIO_DFUUPGRADERUNNING_STARTDFUCMDWELLSENT );
				m_vecMsgToPrintInListBox.push_back( str );

				// "... wait ACK."
				str = TASApp.LoadLocalizedString( IDS_DLGDPSVISIO_DFUUPGRADERUNNING_WAITDFUCMDACK );
				m_vecMsgToPrintInListBox.push_back( str );

				m_eStateEngine = HID_StartDFU_WaitAnswer;
			}
			
			break;

		case HID_StartDFU_WaitAnswer:
			
			if( CDpSVisioBaseCommunication::CT_HID != pclDpSVisioCommunication->GetCommunicationType() )
			{
				m_eStateEngine = _SetUpgradeError( CDlgDpSVisio::UE_DeviceChanged, 0 );
				bContinue = false;
			}
			
			if( true == bContinue )
			{
				int iReturnCode = pclDpSVisioCommunication->ReadDpSVisioCommand( &m_UpdateOutput );

				if( DPSVISIOCOM_NOERROR != iReturnCode )
				{
					m_eStateEngine = _SetUpgradeError( CDlgDpSVisio::UE_InternalError, iReturnCode );
					bContinue = false;
				}
			}
			
			if( true == bContinue )
			{
				// "... ACK received."
				str = TASApp.LoadLocalizedString( IDS_DLGDPSVISIO_DFUUPGRADERUNNING_ACKRECEIVED );
				m_vecMsgToPrintInListBox.push_back( str );

				// "... wait the HID 'DETACH' event."
				str = TASApp.LoadLocalizedString( IDS_DLGDPSVISIO_DFUUPGRADERUNNING_WAITHIDDETACH );
				m_vecMsgToPrintInListBox.push_back( str );

				m_uiUSBEventCounter = 0;
				m_eStateEngine = HID_StartDFU_WaitHIDDetach;
			}

			break;

		case HID_StartDFU_WaitHIDDetach:
			
			// Wait 'OnDpSVisioDisconnected' event.
			m_uiUSBEventCounter++;

			if( DLGVISIODFUUPDATERUNNING_WAITUSBEVENTTIMEOUT == m_uiUSBEventCounter )
			{
				// Because there is no method to programmatically disconnect a device, on some computers/laptops depending also on which Windows
				// version there is sometimes no DBT_DEVICEREMOVECOMPLETE event sent by Windows.
				// It is thus impossible for us to wait HID detach. Thus we go directly in the DFU attach step. 
				// Anyway in the end of the firmware programming we have to physically disconnect the device. Thus Windows will reset its internal
				// device list.
				m_pParent->DisconnectHIDDevice();

				// eStateEngine = _SetUpgradeError( CDlgDpSVisio::UE_InternalError, DPSVISIOCOM_ERROR_HID_CANTDETACH );
			}

			break;

		case HID_StartDFU_WaitDFUAttach:
			
			// Wait 'OnDpSVisioConnected' event.
			m_uiUSBEventCounter++;

			if( DLGVISIODFUUPDATERUNNING_WAITUSBEVENTTIMEOUT == m_uiUSBEventCounter )
			{
				PostMessage( WM_USER_DPSVISIO_SHOWDLGDFUINSTALLATION );
				m_eStateEngine = DFU_Wait_DlgDFUInstallationLaunch;
			}

			break;

		case DFU_Wait_DlgDFUInstallationLaunch:
			// Wait that dialog has shown the 'CDglDpSVisioDFUInstallation'.
			break;

		case DFU_DlgDFUInstallationLaunched:
			// No we wait user clicks 'OK' or 'Cancel'.

			if( true == m_pDlgDpSVisioDFUInstallation->OKPressed() )
			{
				PostMessage( WM_USER_DPSVISIO_FISNISHDFUINSTALLATION );
				m_eStateEngine = DFU_Wait_DlgDFUInstallationDestroy;
			}

			break;

		case DFU_Wait_DlgDFUInstallationDestroy:
			// Wait that dialog 'CDglDpSVisioDFUInstallation' be destroyed.
			break;

		case DFU_StartErase:

			if( CDpSVisioBaseCommunication::CT_DFU != pclDpSVisioCommunication->GetCommunicationType() )
			{
				m_eStateEngine = _SetUpgradeError( CDlgDpSVisio::UE_DeviceChanged, 0 );
				bContinue = false;
			}
			
			if( true == bContinue )
			{
				str = m_pParent->GetAvailableFirmwareFilename( CDlgDpSVisio::Device_DFUSe );

				int iReturnCode = pclDpSVisioCommunication->SendDpSVisioCommandLP( DFUCMD_STARTERASE, (LPARAM)&str );
								
				if( DPSVISIOCOM_NOERROR != iReturnCode )
				{
					m_eStateEngine = _SetUpgradeError( CDlgDpSVisio::UE_InternalError, iReturnCode );
					bContinue = false;
				}
			}

			if( true == bContinue )
			{
				// m_pParent->EnableCancelButton( FALSE );
				m_pParent->PostMessage( WM_USER_DPSVISIO_ENABLEBUTTON, (WPARAM)IDCANCEL, (LPARAM)FALSE );

				// "The DpS-Visio memory erasing is started."
				str = TASApp.LoadLocalizedString( IDS_DLGDPSVISIO_DFUUPGRADERUNNING_ERASESTARTED );
				m_vecMsgToPrintInListBox.push_back( str );

				m_StaticInfo.SetTextColor( _TAH_ORANGE );
				m_StaticInfo.SetFontSize( 10 );
				m_StaticInfo.SetFontBold( true );
				m_StaticInfo.SetWindowText( TASApp.LoadLocalizedString( IDS_DLGDPSVISIO_DFUUPGRADERUNNING_DONTTOUCH ) );

				m_UpdateProgress.ModifyStyle( PBS_MARQUEE, 0 );
				m_UpdateProgress.SetPos( 0 );

				FormatString( str, IDS_DLGDPSVISIO_DFUUPGRADERUNNING_PROGRESSERASE, _T("0%") );
				m_UpdateProgress.SetWindowText( str );
				m_UpdateProgress.SetShowText( true );

				m_eStateEngine = DFU_CheckErase;
			}

			break;

		case DFU_CheckErase:

			if( CDpSVisioBaseCommunication::CT_DFU != pclDpSVisioCommunication->GetCommunicationType() )
			{
				m_eStateEngine = _SetUpgradeError( CDlgDpSVisio::UE_DeviceChanged, 0 );
				bContinue = false;
			}
			
			if( true == bContinue )
			{
				int iReturnCode = pclDpSVisioCommunication->SendDpSVisioCommandLP( DFUCMD_CHECKERASE, (LPARAM)&bPercentCompleted );
				
				if( DPSVISIOCOM_NOERROR != iReturnCode )
				{
					m_eStateEngine = _SetUpgradeError( CDlgDpSVisio::UE_InternalError, iReturnCode );
					bContinue = false;
				}
			}

			if( true == bContinue )
			{
				CString strPercent;
				strPercent.Format( _T("%u%%"), bPercentCompleted );
				FormatString( str, IDS_DLGDPSVISIO_DFUUPGRADERUNNING_PROGRESSERASE, strPercent );
				m_UpdateProgress.SetWindowText( str );
				m_UpdateProgress.SetPos( bPercentCompleted );

				if( 100 == bPercentCompleted )
				{
					// "The DpS-Visio memory is erased."
					str = TASApp.LoadLocalizedString( IDS_DLGDPSVISIO_DFUUPGRADERUNNING_ERASEFINISED );
					m_vecMsgToPrintInListBox.push_back( str );

					// "Start programming the DpS-Visio."
					str = TASApp.LoadLocalizedString( IDS_DLGDPSVISIO_UPGRADERUNNING_STARTPROGRAMMING );
					m_vecMsgToPrintInListBox.push_back( str );

					m_eStateEngine = DFU_StartUpgrade;
				}
			}

			break;

		case DFU_StartUpgrade:

			if( CDpSVisioBaseCommunication::CT_DFU != pclDpSVisioCommunication->GetCommunicationType() )
			{
				m_eStateEngine = _SetUpgradeError( CDlgDpSVisio::UE_DeviceChanged, 0 );
				bContinue = false;
			}
			
			if( true == bContinue )
			{
				int iReturnCode = pclDpSVisioCommunication->SendDpSVisioCommandLP( DFUCMD_STARTUPGRADE );

				if( DPSVISIOCOM_NOERROR != iReturnCode )
				{
					m_eStateEngine = _SetUpgradeError( CDlgDpSVisio::UE_InternalError, iReturnCode );
					bContinue = false;
				}
			}
			
			if( true == bContinue )
			{
				// "The DpS-Visio programming is started."
				str = TASApp.LoadLocalizedString( IDS_DLGDPSVISIO_UPGRADERUNNING_UPGRADESTARTED );
				m_vecMsgToPrintInListBox.push_back( str );

				m_UpdateProgress.SetPos( 0 );
				
				FormatString( str, IDS_DLGDPSVISIO_DFUUPGRADERUNNING_PROGRESSUPGRADE, _T("0%") );
				m_UpdateProgress.SetWindowText( str );

				m_eStateEngine = DFU_CheckUpgrade;
			}

			break;

		case DFU_CheckUpgrade:

			if( CDpSVisioBaseCommunication::CT_DFU != pclDpSVisioCommunication->GetCommunicationType() )
			{
				m_eStateEngine = _SetUpgradeError( CDlgDpSVisio::UE_DeviceChanged, 0 );
				bContinue = false;
			}
			
			if( true == bContinue )
			{
				int iReturnCode = pclDpSVisioCommunication->SendDpSVisioCommandLP( DFUCMD_CHECKUPGRADE, (LPARAM)&bPercentCompleted );

				if( DPSVISIOCOM_NOERROR != iReturnCode )
				{
					m_eStateEngine = _SetUpgradeError( CDlgDpSVisio::UE_InternalError, iReturnCode );
					bContinue = false;
				}
			}
			
			if( true == bContinue )
			{
				m_UpdateProgress.SetPos( bPercentCompleted );

				CString strPercent;
				strPercent.Format( _T("%u%%"), bPercentCompleted );
				FormatString( str, IDS_DLGDPSVISIO_DFUUPGRADERUNNING_PROGRESSUPGRADE, strPercent );
				m_UpdateProgress.SetWindowText( str );

				if( 100 == bPercentCompleted )
				{
					// "The DpS-Visio is programmed."
					str = TASApp.LoadLocalizedString( IDS_DLGDPSVISIO_DFUUPGRADERUNNING_UPGRADEFINISHED );
					m_vecMsgToPrintInListBox.push_back( str );

					// "Set the DpS-Visio in the HDI mode."
					str = TASApp.LoadLocalizedString( IDS_DLGDPSVISIO_DFUUPGRADERUNNING_STARTHID );
					m_vecMsgToPrintInListBox.push_back( str );

					PostMessage( WM_USER_DPSVISIO_SHOWDLGUNPLUGPLUG );
					m_eStateEngine = DFU_Wait_DlgUnplugPlugLaunch;
				}
			}
			
			break;

		case DFU_Wait_DlgUnplugPlugLaunch:
			// Wait that dialog has shown the 'CDglDpSVisioUnplugPlug'.
			break;

		case DFU_DlgUnplugPlugLaunched:
			// m_uiUSBEventCounter = 0;
			m_eStateEngine = DFU_StartHDI_WaitDFUDetach;
			break;

		case DFU_StartHDI_WaitDFUDetach:

			// 2017-04-25: It has been decided that there will be no more a timeout.
			/*
			// Wait 'OnDpSVisioDisconnected' event.
			m_uiUSBEventCounter++;

			if( DLGVISIOUPDATERUNNING_WAITUNPLUGPLUG == m_uiUSBEventCounter )
			{
				PostMessage( WM_USER_DPSVISIO_DESTROYDLGUNPLUGPLUG );
				eStateEngine = _SetUpgradeError( CDlgDpSVisio::UE_InternalError, DPSVISIOCOM_ERROR_DFU_CANTDETACH, DFU_Wait_DlgUnplugPlugDestroy );
			}
			*/

			break;

		case DFU_Wait_DlgUnplugPlugDestroy:
			// Wait that dialog has destroyed the 'CDglDpSVisioUnplugPlug'.
			break;

		case DFU_DlgUnplugPlugDestroyed:
			m_eStateEngine = UpdateError;
			break;

		case DFU_StartHDI_WaitHIDAttach:

			// 2017-04-25: It has been decided that there will be no more a timeout.
			/*
			// Wait 'OnDpSVisioConnected' event.
			m_uiUSBEventCounter++;

			if( DLGVISIOUPDATERUNNING_WAITUNPLUGPLUG == m_uiUSBEventCounter )
			{
				PostMessage( WM_USER_DPSVISIO_DESTROYDLGUNPLUGPLUG );
				eStateEngine = _SetUpgradeError( CDlgDpSVisio::UE_InternalError, DPSVISIOCOM_ERROR_HID_CANTATTACH, DFU_Wait_DlgUnplugPlugDestroy );
			}
			*/

			break;

		case HID_VerifyDevice:

			if( CDpSVisioBaseCommunication::CT_HID != pclDpSVisioCommunication->GetCommunicationType() )
			{
				m_eStateEngine = _SetUpgradeError( CDlgDpSVisio::UE_DeviceChanged, 0 );
			}
			else if( m_strHIDDeviceName != pclDpSVisioCommunication->GetDpSVisioInformation()->m_strDeviceName )
			{
				str = TASApp.LoadLocalizedString( IDS_DLGDPSVISIO_UPGRADERUNNING_BADHIDDEVICE );
				m_vecMsgToPrintInListBox.push_back( str );

				// Stay in the same state until user plug the good device.
			}
			else
			{
				// DpS-Visio connected is the programmed one.
				m_pParent->GetUpgradeStatusData()->m_fUpgradeStatus = true;
				m_pParent->GetUpgradeStatusData()->m_dwDFUInternalErrorCode = 0;
				m_eStateEngine = Finished;
			}

		case Finished:
			PostMessage( WM_USER_DPSVISIO_STOPTHREADUPGRADE, (WPARAM)CDlgDpSVisio::Dialog_UpgradeFinished, (LPARAM)m_pParent->GetUpgradeStatusData() );
			m_eStateEngine = WaitingToBeKilled;
			break;

		case UpdateError:
			PostMessage( WM_USER_DPSVISIO_STOPTHREADUPGRADE, (WPARAM)CDlgDpSVisio::Dialog_UpgradeFinished, (LPARAM)m_pParent->GetUpgradeStatusData() );
			m_eStateEngine = WaitingToBeKilled;
			break;

		case WaitingToBeKilled:
			break;
	}

	LeaveCriticalSection( &m_CriticalSection );
}

void CDlgDpSVisioDFUUpgradeRunning::_StopThreadUpgradeRunning()
{
	m_pclStopThreadEvent->SetEvent();

	bool bStop = false;
	UINT16 uiCount = 0;

	while( false == bStop )
	{
		DWORD dwReturn = WaitForSingleObject( m_pclThreadStoppedEvent->m_hObject, DLGVISIODFUUPDATERUNNING_INTERVALTOWAITSTOP );

		switch( dwReturn )
		{
			// Stop.
			case WAIT_OBJECT_0:
				bStop = true;
				break;

			// Timeout.
			case WAIT_TIMEOUT:

				uiCount++;

				if( 100 == uiCount )
				{
					bStop = true;
				}

				break;

			default:
				bStop = true;
				break;
		}
	}

	m_pThread = NULL;
}

CDlgDpSVisioDFUUpgradeRunning::_StateEngine CDlgDpSVisioDFUUpgradeRunning::_SetUpgradeError( CDlgDpSVisio::UpgradeError eUpgradeError, DWORD dwInternalError, _StateEngine eStateEngine )
{
	m_pParent->GetUpgradeStatusData()->m_fUpgradeStatus = false;
	m_pParent->GetUpgradeStatusData()->m_eUpgradeError = eUpgradeError;
	m_pParent->GetUpgradeStatusData()->m_dwDFUInternalErrorCode = dwInternalError;

	return eStateEngine;
}
