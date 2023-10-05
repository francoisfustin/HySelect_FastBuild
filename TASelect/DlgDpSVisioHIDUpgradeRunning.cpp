#include "stdafx.h"
#include "afxdialogex.h"
#include "DpSVisioIncludes.h"

IMPLEMENT_DYNAMIC( CDlgDpSVisioHIDUpgradeRunning, CDialogEx )

CDlgDpSVisioHIDUpgradeRunning::CDlgDpSVisioHIDUpgradeRunning( CDlgDpSVisio *pParent )
	: CDlgDpSVisioBase( CDlgDpSVisioHIDUpgradeRunning::IDD, pParent )
{
	InitializeCriticalSection( &m_CriticalSection );
	m_eStateEngine = StartFirmwareUploadSession;
	m_strHIDDeviceName = _T("");
	m_pThread = NULL;
	m_pclStopThreadEvent = NULL;
	m_pclThreadStoppedEvent = NULL;
	m_vecMsgToPrintInListBox.clear();
	m_nTimer = (UINT_PTR)0;
	m_pclFirmwareToUpload = NULL;
}

CDlgDpSVisioHIDUpgradeRunning::~CDlgDpSVisioHIDUpgradeRunning()
{
	if( NULL != GetSafeHwnd() && (UINT_PTR)0 != m_nTimer )
	{
		KillTimer( m_nTimer );
	}

	if( NULL != m_pclStopThreadEvent )
	{
		delete m_pclStopThreadEvent;
	}

	if( NULL != m_pclThreadStoppedEvent )
	{
		delete m_pclThreadStoppedEvent;
	}

	if( NULL != m_pThread )
	{
		delete m_pThread;
	}

	DeleteCriticalSection( &m_CriticalSection );
}

UINT CDlgDpSVisioHIDUpgradeRunning::ThreadUpgradeRunning( LPVOID pParam )
{
	CDlgDpSVisioHIDUpgradeRunning *pclDlgDpSVisioHIDUpgradeRunning = (CDlgDpSVisioHIDUpgradeRunning *)pParam;

	if( NULL == pclDlgDpSVisioHIDUpgradeRunning )
	{
		return -1;
	}

	bool bStop = false;

	while( false == bStop )
	{
		DWORD dwReturn = WaitForSingleObject( pclDlgDpSVisioHIDUpgradeRunning->m_pclStopThreadEvent->m_hObject, DLGVISIOHIDUPDATERUNNING_TIMERMSEC );

		switch( dwReturn )
		{
			// Stop.
			case WAIT_OBJECT_0:
				bStop = true;
				break;

			// Timeout.
			case WAIT_TIMEOUT:

				pclDlgDpSVisioHIDUpgradeRunning->_UpgradeProgess();
				break;
		}
	}

	pclDlgDpSVisioHIDUpgradeRunning->m_pclThreadStoppedEvent->SetEvent();
	return 0;
}

void CDlgDpSVisioHIDUpgradeRunning::Stop()
{
	_StopThreadUpgradeRunning();
}

void CDlgDpSVisioHIDUpgradeRunning::OnDpSVisioConnected( CDlgDpSVisio::WhichDevice eWhichDevice )
{
	OutputDebugString( _T("OnDpSVisioConnected called...\n") );
	CString str;

	EnterCriticalSection( &m_CriticalSection );

	switch( m_eStateEngine )
	{
		case WaitHIDAttach:

			OutputDebugString( _T("'WaitHIDAttach' state engine...\n") );

			// "The DpS-Visio is in HDI mode."
			str = TASApp.LoadLocalizedString( IDS_DLGDPSVISIO_UPGRADERUNNING_HIDATTACHED );
			INSERT_STRING_IN_LIST_AND_SCROLL( m_UpdateOutput, str );

			m_eStateEngine = VerifyDevice;
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

void CDlgDpSVisioHIDUpgradeRunning::OnDpSVisioDisconnected( CDlgDpSVisio::WhichDevice eWhichDevice )
{
	OutputDebugString( _T("OnDpSVisioDisconnected called...\n") );
	CString str;

	EnterCriticalSection( &m_CriticalSection );

	switch( m_eStateEngine )
	{
		case WaitHIDDetach:
			
			OutputDebugString( _T("'WaitHIDDetach' state engine...\n") );

			OutputDebugString( _T("The device is well in HID mode.\n") );

			// Remarks: In HID mode, we receive here disconnected event only for the HID device that was connected and not
			//          for other ones.

			// "... HID 'DETACH' event received."
			str = TASApp.LoadLocalizedString( IDS_DLGDPSVISIO_UPGRADERUNNING_HIDDETACHED );
			INSERT_STRING_IN_LIST_AND_SCROLL( m_UpdateOutput, str );

			// "... wait the HID 'ATTACH' event."
			str = TASApp.LoadLocalizedString( IDS_DLGDPSVISIO_UPGRADERUNNING_WAITHIDATTACH );
			INSERT_STRING_IN_LIST_AND_SCROLL( m_UpdateOutput, str );

			m_uiUSBEventCounter = 0;
			m_eStateEngine = WaitHIDAttach;
			break;

		default:
			OutputDebugString( _T("default state engine...\n") );

			m_eStateEngine = DeviceDisconnected;
			break;
	}

	OutputDebugString( _T("OnDpSVisioDisconnected end.\n") );
	LeaveCriticalSection( &m_CriticalSection );
}

BEGIN_MESSAGE_MAP( CDlgDpSVisioHIDUpgradeRunning, CDialogEx )
	ON_WM_TIMER()
	ON_MESSAGE( WM_USER_DPSVISIO_STOPTHREADUPGRADE, OnThreadFinished )
END_MESSAGE_MAP()

void CDlgDpSVisioHIDUpgradeRunning::DoDataExchange( CDataExchange* pDX )
{
	CDlgDpSVisioBase::DoDataExchange( pDX );
	DDX_Control( pDX, IDC_PROGRESS, m_UpdateProgress );
	DDX_Control( pDX, IDC_UPDATEOUTPUT, m_UpdateOutput );
	DDX_Control( pDX, IDC_STATICINFO, m_StaticInfo );
}

BOOL CDlgDpSVisioHIDUpgradeRunning::OnInitDialog()
{
	CDlgDpSVisioBase::OnInitDialog();

	CDpSVisioBaseCommunication *pclDpSVisioCommunication = m_pParent->GetDpSVisioCommunication();
	CDpSVisioBaseInformation *pclDpSVisionInformation = pclDpSVisioCommunication->GetDpSVisioInformation();

	CString str = TASApp.LoadLocalizedString( IDS_DLGDPSVISIO_UPGRADERUNNING_CURRENTVERSION );
	GetDlgItem( IDC_STATIC_CURRENTVERSIONTEXT )->SetWindowText( str );

	str = pclDpSVisionInformation->m_strShortFirmwareRevision;
	GetDlgItem( IDC_STATIC_CURRENTVERSIONVALUE )->SetWindowText( str );
	
	str = TASApp.LoadLocalizedString( IDS_DLGDPSVISIO_UPGRADERUNNING_AVAILABLEVERSION );
	GetDlgItem( IDC_STATIC_AVAILABLEVERSIONTEXT )->SetWindowText( str );
	GetDlgItem( IDC_STATIC_AVAILABLEVERSIONVALUE )->SetWindowText( m_pParent->GetAvailableFirmwareRevisionString( CDlgDpSVisio::Device_HID ) );

	m_UpdateProgress.SetRange( 0, 99 );
	m_UpdateProgress.SetStep( 1 );
//	m_UpdateProgress.ModifyStyle( PBS_MARQUEE, 0 );
//	m_UpdateProgress.SetMarquee( TRUE, 30 );
	m_UpdateProgress.SetShowText( false );
	m_UpdateProgress.SetTextBackgroundColor( RGB( 255, 255, 255 ) );
	m_UpdateProgress.SetTextForegroundColor( RGB( 6, 176, 37 ) );
	m_UpdateProgress.SetBackgroundColor( RGB( 230, 230, 230 ) );
	m_UpdateProgress.SetForegroundColor( RGB( 6, 176, 37 ) );

	m_StaticInfo.SetWindowText( _T("") );
	m_UpdateOutput.ModifyStyle( LBS_SORT, 0 );

	// Save HID device name to be sure it's the same when unplug/plug when finalizing the upgrade.
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
	
	m_pclFirmwareToUpload = m_pParent->GetFirmwareToUpload( CDlgDpSVisio::Device_HID );

	if( NULL == m_pclFirmwareToUpload )
	{
		return FALSE;
	}

	return TRUE;
}

void CDlgDpSVisioHIDUpgradeRunning::OnTimer( UINT_PTR nIDEvent )
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

LRESULT CDlgDpSVisioHIDUpgradeRunning::OnThreadFinished( WPARAM wParam, LPARAM lParam )
{
	_StopThreadUpgradeRunning();
	m_pParent->PostMessage( WM_USER_DPSVISIO_SHOWDIALOG, wParam, lParam );
	return 0;
}

void CDlgDpSVisioHIDUpgradeRunning::_UpgradeProgess()
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
		case StartFirmwareUploadSession:
			
			// "Start programming the DpS-Visio."
			str = TASApp.LoadLocalizedString( IDS_DLGDPSVISIO_UPGRADERUNNING_STARTPROGRAMMING );
			m_vecMsgToPrintInListBox.push_back( str );

			if( CDpSVisioBaseCommunication::CT_HID != pclDpSVisioCommunication->GetCommunicationType() )
			{
				_SetUpgradeError( CDlgDpSVisio::UE_DeviceChanged, 0 );
				bContinue = false;
			}

			if( true == bContinue )
			{
				UINT32 uiFileSize = (UINT32)m_pclFirmwareToUpload->GetLength();
				UINT32 uiFileSizeInv = 0;
				DpSVisioUtils_CopyMSBFirst( (UINT8*)&uiFileSizeInv, (UINT8*)&uiFileSize, sizeof( UINT32 ) );

				int iReturnCode = pclDpSVisioCommunication->SendAndReadDpSVisioCommand( UPGFW_STARTSEND, sizeof( UINT32 ), (UINT8*)&uiFileSizeInv, &m_UpdateOutput );

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

				m_eStateEngine = SendFirmwareVersion;
			}
			
			break;

		case SendFirmwareVersion:
			
			// Send firmware version.
			str = TASApp.LoadLocalizedString( IDS_DLGDPSVISIOHIDUPGRADERUNNING_SENDFIRMWAREVERSION );
			m_vecMsgToPrintInListBox.push_back( str );

			if( CDpSVisioBaseCommunication::CT_HID != pclDpSVisioCommunication->GetCommunicationType() )
			{
				m_eStateEngine = _SetUpgradeError( CDlgDpSVisio::UE_DeviceChanged, 0 );
				bContinue = false;
			}
			
			if( true == bContinue )
			{
				UINT32 uiFirmwareVersion = m_pParent->GetAvailableFirmwareRevisionValue( CDlgDpSVisio::Device_HID );
				int iReturnCode = pclDpSVisioCommunication->SendAndReadDpSVisioCommand( UPGFW_SENDVERSION, sizeof( UINT32 ), (UINT8*)&uiFirmwareVersion, &m_UpdateOutput );

				if( DPSVISIOCOM_NOERROR != iReturnCode )
				{
					m_eStateEngine = _SetUpgradeError( CDlgDpSVisio::UE_InternalError, iReturnCode );
					bContinue = false;
				}
			}

			if( true == bContinue )
			{
				// "Sending firmware..."
				str = TASApp.LoadLocalizedString( IDS_DLGDPSVISIOHIDUPGRADERUNNING_SENDINGFIRMWARE );
				m_vecMsgToPrintInListBox.push_back( str );

				m_pclFirmwareToUpload->Seek( 0, CFile::begin );

				m_eStateEngine = SendOnePacket;
			}

			break;

		case SendOnePacket:
			{
				UINT8 arData[CMD_DATAMAXSIZE];
				UINT32 uiBlockCount = 1;
				ULONGLONG ullFileLeft = m_pclFirmwareToUpload->GetLength();

				// Because we will send all the file in this thread, we want to allow 'OnTimer' to work.
				LeaveCriticalSection( &m_CriticalSection );

				do
				{
					if( CDpSVisioBaseCommunication::CT_HID != pclDpSVisioCommunication->GetCommunicationType() )
					{
						m_eStateEngine = _SetUpgradeError( CDlgDpSVisio::UE_DeviceChanged, 0 );
						bContinue = false;
					}
					
					UINT8 uiSizeToCopy = ( ullFileLeft >= CMD_DATAMAXSIZE ) ? CMD_DATAMAXSIZE : (UINT8)ullFileLeft;

					if( true == bContinue )
					{
						if( uiSizeToCopy != m_pclFirmwareToUpload->Read( arData, (UINT)uiSizeToCopy ) )
						{
							// "Unexpected end of file reached!!"
							str = TASApp.LoadLocalizedString( IDS_DLGDPSVISIO_HIDUPGRADERUNNING_UNEXPECTEDEOF );
							m_vecMsgToPrintInListBox.push_back( str );
					
							m_eStateEngine = _SetUpgradeError( CDlgDpSVisio::UE_SendingFWUnexpectedEOF, 0 );
							bContinue = false;
						}
					}

					if( true == bContinue )
					{
						if( -1 == pclDpSVisioCommunication->SendAndReadDpSVisioCommand( UPGFW_SENDONEPACKET, uiSizeToCopy, arData , &m_UpdateOutput ) )
						{
							// "Unexpected error when sending one block to the DpS-Visio."
							str = TASApp.LoadLocalizedString( IDS_DLGDPSVISIO_HIDUPGRADERUNNING_UNEXPECTEDERROR );
							m_vecMsgToPrintInListBox.push_back( str );

							m_eStateEngine = _SetUpgradeError( CDlgDpSVisio::UE_SendingFWUnexpectedError, 0 );
							bContinue = false;
						}
					}
			
					if( true == bContinue )
					{
						uiBlockCount++;
						UINT8 bPercentCompleted = 100 - (UINT8)( ullFileLeft * 100 / m_pclFirmwareToUpload->GetLength() );
						m_UpdateProgress.SetPos( bPercentCompleted );

						ullFileLeft -= uiSizeToCopy;

						if( 0 == ullFileLeft )
						{
							// "The firmware has well been sent to the DpS-Visio."
							str = TASApp.LoadLocalizedString( IDS_DLGDPSVISIO_HIDUPGRADERUNNING_FWSENT );
							m_vecMsgToPrintInListBox.push_back( str );

							// Reactivate the critical section.
							EnterCriticalSection( &m_CriticalSection );
							m_eStateEngine = SendCRC;
						}
					}
				}while( ullFileLeft > 0 );
			}

			break;

		case SendCRC:
			
			// "Sending CRC..."
			str = TASApp.LoadLocalizedString( IDS_DLGDPSVISIO_HIDUPGRADERUNNING_SENDCRC );
			m_vecMsgToPrintInListBox.push_back( str );

			if( CDpSVisioBaseCommunication::CT_HID != pclDpSVisioCommunication->GetCommunicationType() )
			{
				m_eStateEngine = _SetUpgradeError( CDlgDpSVisio::UE_DeviceChanged, 0 );
				bContinue = false;
			}
			
			if( true == bContinue )
			{
				UINT32 uiFileSize = (UINT32)m_pclFirmwareToUpload->GetLength();
				UINT8 *pBuffer = m_pclFirmwareToUpload->Detach();
				UINT32 uiCRC = DpSVisioUtils_crc32( (char *)pBuffer, uiFileSize );
				m_pclFirmwareToUpload->Attach( pBuffer, uiFileSize);

				UINT32 uiCRCInv = 0;
				DpSVisioUtils_CopyMSBFirst( (UINT8*)&uiCRCInv, (UINT8*)&uiCRC, sizeof(UINT32) );

				int iReturnCode = pclDpSVisioCommunication->SendAndReadDpSVisioCommand( UPGFW_SENDCRC, sizeof( uiCRCInv ), (UINT8*)&uiCRCInv, &m_UpdateOutput );

				if( DPSVISIOCOM_NOERROR != iReturnCode )
				{
					m_eStateEngine = _SetUpgradeError( CDlgDpSVisio::UE_InternalError, iReturnCode );
					bContinue = false;
				}
			}

			if( true == bContinue )
			{
				m_eStateEngine = StartUpgrade;
			}

			break;

		case StartUpgrade:
			
			// "The DpS-Visio will now restart to update with the new firmware."
			str = TASApp.LoadLocalizedString( IDS_DLGDPSVISIO_HIDUPGRADERUNNING_PROGRAM );
			m_vecMsgToPrintInListBox.push_back( str );

			if( CDpSVisioBaseCommunication::CT_HID != pclDpSVisioCommunication->GetCommunicationType() )
			{
				m_eStateEngine = _SetUpgradeError( CDlgDpSVisio::UE_DeviceChanged, 0 );
				bContinue = false;
			}
			
			if( true == bContinue )
			{
				int iReturnCode = pclDpSVisioCommunication->SendAndReadDpSVisioCommand( UPGFM_PROGRAM, 0, NULL, &m_UpdateOutput );

				if( DPSVISIOCOM_NOERROR != iReturnCode )
				{
					m_eStateEngine = _SetUpgradeError( CDlgDpSVisio::UE_InternalError, iReturnCode );
					bContinue = false;
				}
			}

			if( true == bContinue )
			{
				m_eStateEngine = WaitHIDDetach;
			}
			
			break;

		case WaitHIDDetach:
			// It's the 'OnDpSVisioDisconnected' that will change the engine state.
			break;

		case WaitHIDAttach:
			// It's the 'OnDpSVisioConnected' that will change the engine state.
			break;

		case VerifyDevice:

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

void CDlgDpSVisioHIDUpgradeRunning::_StopThreadUpgradeRunning()
{
	m_pclStopThreadEvent->SetEvent();

	bool bStop = false;
	UINT16 uiCount = 0;

	while( false == bStop )
	{
		DWORD dwReturn = WaitForSingleObject( m_pclThreadStoppedEvent->m_hObject, DLGVISIOHIDUPDATERUNNING_INTERVALTOWAITSTOP );

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

CDlgDpSVisioHIDUpgradeRunning::_StateEngine CDlgDpSVisioHIDUpgradeRunning::_SetUpgradeError( CDlgDpSVisio::UpgradeError eUpgradeError, DWORD dwInternalError, _StateEngine eStateEngine )
{
	m_pParent->GetUpgradeStatusData()->m_fUpgradeStatus = false;
	m_pParent->GetUpgradeStatusData()->m_eUpgradeError = eUpgradeError;
	m_pParent->GetUpgradeStatusData()->m_dwDFUInternalErrorCode = dwInternalError;

	return eStateEngine;
}
