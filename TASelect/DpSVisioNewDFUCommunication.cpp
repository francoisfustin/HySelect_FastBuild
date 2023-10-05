#include "stdafx.h"


#include <iostream>
#include <chrono>
#include "DpSVisioIncludes.h"

CDpSVisioNewDFUCommunication::CDpSVisioNewDFUCommunication( CWnd *pParent, _DFUUtilWorkVariables *prNewDFUSeVariables )
	: CDpSVisioBaseCommunication( pParent, CDpSVisioBaseCommunication::CT_DFU )
{
	m_pclDpSVisioCommand = &m_clDpSVisioDFUCommand;
	m_pclDpSVisioInformation = &m_clDpSVisioNewDFUInformation;
	m_prNewDFUSeVariables = prNewDFUSeVariables;
	m_pProcessThread = NULL;
	m_pclStopThreadEvent = NULL;
	m_pclThreadStoppedEvent = NULL;
	InitializeCriticalSection( &m_CriticalSection );

	Reset();
}

CDpSVisioNewDFUCommunication::~CDpSVisioNewDFUCommunication()
{
	Reset();

	if( NULL != m_prNewDFUSeVariables )
	{
		if( NULL != NULL != m_prNewDFUSeVariables->dfu_root )
		{
			struct dfu_if *adif = m_prNewDFUSeVariables->dfu_root;

			while( NULL != adif)
			{
				if( NULL != adif->mem_layout )
				{
					free_segment_list( adif->mem_layout );
					adif->mem_layout = NULL;
				}

				adif = adif->next;
			}
		}

		disconnect_devices( m_prNewDFUSeVariables );
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

	if( NULL != m_pProcessThread )
	{
		delete m_pProcessThread;
		m_pProcessThread = NULL;
	}

	DeleteCriticalSection( &m_CriticalSection );
}

void CDpSVisioNewDFUCommunication::DpSVisioConnected( CString strDeviceName )
{
	CString str;

	if( NULL == m_prNewDFUSeVariables )
	{
		OutputDebugString( _T("CDpSVisioNewDFUCommunication::DpSVisioConnected: 'm_prNewDFUSeVariables' can't ne NULL!\n") );
		ASSERT_RETURN;
	}

	if( NULL == m_prNewDFUSeVariables->dfu_root )
	{
		OutputDebugString( _T("CDpSVisioNewDFUCommunication::DpSVisioConnected: 'm_prNewDFUSeVariables->dfu_root' can't ne NULL!\n") );
		ASSERT_RETURN;
	}

	int iret = libusb_open( m_prNewDFUSeVariables->dfu_root->dev, &m_prNewDFUSeVariables->dfu_root->dev_handle );

	if( iret || !m_prNewDFUSeVariables->dfu_root->dev_handle )
	{
		str.Format( _T("Cannot open device: %s.\n"), libusb_error_name( iret ) );
		OutputDebugString( str );
		ASSERT_RETURN;
	}

	iret = libusb_claim_interface( m_prNewDFUSeVariables->dfu_root->dev_handle, m_prNewDFUSeVariables->dfu_root->interface );

	if( iret < 0 )
	{
		str.Format( _T("Cannot claim interface: %s.\n"), libusb_error_name( iret ) );
		OutputDebugString( str );
		ASSERT_RETURN;
	}

	if( m_prNewDFUSeVariables->dfu_root->flags & DFU_IFF_ALT)
	{
		CStringA strA;
		strA.Format( "Setting Alternate Interface #%d ...\n", m_prNewDFUSeVariables->dfu_root->altsetting );
		OutputDebugStringA( strA );
		
		iret = libusb_set_interface_alt_setting( m_prNewDFUSeVariables->dfu_root->dev_handle, m_prNewDFUSeVariables->dfu_root->interface, m_prNewDFUSeVariables->dfu_root->altsetting );
		
		if( iret < 0)
		{
			str.Format( _T("Cannot set alternate interface: %s.\n"), libusb_error_name( iret ) );
			OutputDebugString( str );
			ASSERT_RETURN;
		}
	}

	bool bStop = false;

	do
	{
		struct dfu_status status;
		iret = dfu_get_status( m_prNewDFUSeVariables->dfu_root, &status );
	
		if( iret < 0 )
		{
			CString str;
			str.Format( _T("Error get_status: %s.\n"), libusb_error_name( iret ) );
			OutputDebugString( str );
			ASSERT_RETURN;
		}
	
		str.Format( _T("DFU state(%u) = %s, status(%u) = %s\n"), status.bState, dfu_state_to_string( status.bState ), status.bStatus, dfu_status_to_string( status.bStatus ) );
		OutputDebugString( str );

		if( status.bwPollTimeout != 0 )
		{
			Sleep( status.bwPollTimeout );
		}

		switch( status.bState )
		{
			case DFU_STATE_appIDLE:
			case DFU_STATE_appDETACH:
				OutputDebugString( _T("Device still in Run-Time Mode!\n") );
				ASSERT_RETURN;
				break;
		
			case DFU_STATE_dfuERROR:
				OutputDebugString( _T("Clearing status.\n") );

				if( dfu_clear_status( m_prNewDFUSeVariables->dfu_root->dev_handle, m_prNewDFUSeVariables->dfu_root->interface) < 0 )
				{
					OutputDebugString( _T("Error clear_status.\n") );
					ASSERT_RETURN;
				}
				
				break;
		
			case DFU_STATE_dfuDNLOAD_IDLE:
			case DFU_STATE_dfuUPLOAD_IDLE:
				OutputDebugString( _T("Aborting previous incomplete transfer.\n") );

				if( dfu_abort( m_prNewDFUSeVariables->dfu_root->dev_handle, m_prNewDFUSeVariables->dfu_root->interface ) < 0 )
				{
					OutputDebugString( _T("Can't send DFU_ABORT.\n") );
					ASSERT_RETURN;
				}

				break;
		
			case DFU_STATE_dfuIDLE:
			default:
				bStop = true;
				break;
		}
	}while( false == bStop );

	m_clDpSVisioNewDFUInformation.m_bIsConnected = true;

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

void CDpSVisioNewDFUCommunication::DpSVisioDisconnected( CString strDeviceName )
{
	if( NULL == m_prNewDFUSeVariables )
	{
		OutputDebugString( _T("CDpSVisioNewDFUCommunication::DpSVisioDisconnected: 'm_prNewDFUSeVariables' can't ne NULL!\n") );
		ASSERT_RETURN;
	}

	if( NULL == m_prNewDFUSeVariables->dfu_root )
	{
		OutputDebugString( _T("CDpSVisioNewDFUCommunication::DpSVisioDisconnected: 'm_prNewDFUSeVariables->dfu_root' can't ne NULL!\n") );
		ASSERT_RETURN;
	}

	Reset();

	if( NULL != m_prNewDFUSeVariables )
	{
		// 'm_prNewDFUSeVariables->dfu_root' is free and set to NULL in this function.
		disconnect_devices( m_prNewDFUSeVariables );
	}
}

UINT CDpSVisioNewDFUCommunication::ThreadProcessRunning( LPVOID pParam )
{
	CDpSVisioNewDFUCommunication *pclDlgDpSVisioNewDFUCommunication = (CDpSVisioNewDFUCommunication *)pParam;

	if( NULL == pclDlgDpSVisioNewDFUCommunication )
	{
		return -1;
	}

	if( NULL == pclDlgDpSVisioNewDFUCommunication->m_prNewDFUSeVariables )
	{
		OutputDebugString( _T("CDpSVisioNewDFUCommunication::ThreadProcessRunning: 'm_prNewDFUSeVariables' can't be NULL!") );
		return -1;
	}

	if( NULL == pclDlgDpSVisioNewDFUCommunication->m_prNewDFUSeVariables->dfu_root )
	{
		OutputDebugString( _T("CDpSVisioNewDFUCommunication::ThreadProcessRunning: 'm_prNewDFUSeVariables->dfu_root' can't be NULL!") );
		return -1;
	}

	if( NULL == pclDlgDpSVisioNewDFUCommunication->m_prNewDFUSeVariables->dfu_root->dev_handle )
	{
		OutputDebugString( _T("CDpSVisioNewDFUCommunication::ThreadProcessRunning: 'm_prNewDFUSeVariables->dfu_root->dev_handle' can't be NULL!") );
		return -1;
	}

	// Remark: this thread will never ended by itself.
	// It must be stopped wiht the stop event.

	bool bStop = false;

	while( false == bStop )
	{
		DWORD dwReturn = WaitForSingleObject( pclDlgDpSVisioNewDFUCommunication->m_pclStopThreadEvent->m_hObject, pclDlgDpSVisioNewDFUCommunication->m_PollTime );

		switch( dwReturn )
		{
			// Stop.
			case WAIT_OBJECT_0:
				bStop = true;
				break;

			// Timeout.
			case WAIT_TIMEOUT:

				pclDlgDpSVisioNewDFUCommunication->_ProcessRunning();
				break;
		}
	}

	// Signal that the thread is stopped.
	pclDlgDpSVisioNewDFUCommunication->m_pclThreadStoppedEvent->SetEvent();

	return 0;
}

int CDpSVisioNewDFUCommunication::SendDpSVisioCommandLP( UINT16 uiCommand, LPARAM lpParam, CListBox *pOutputList )
{
	int iReturn = -1;

	switch( uiCommand )
	{
		case DFUCMD_STARTERASE:
			iReturn = _DFUStartErase( (CString *)lpParam, pOutputList );
			break;

		case DFUCMD_CHECKERASE:
			iReturn = _DFUCheckErase( (BYTE *)lpParam, pOutputList );
			break;

		case DFUCMD_STARTUPGRADE:
			iReturn = _DFUStartUpgrade( pOutputList );
			break;

		case DFUCMD_CHECKUPGRADE:
			iReturn = _DFUCheckUpgrade( (BYTE *)lpParam, pOutputList );
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

void CDpSVisioNewDFUCommunication::Reset()
{
	CDpSVisioBaseCommunication::Reset();

	m_pclDpSVisioInformation->Reset();
	m_strFileName = _T("");

	if( NULL != m_DFUFile.firmware )
	{
		free( m_DFUFile.firmware );
	}

	m_DFUFile.clear();

	m_iOperationCode = PC_NothingRunning;
	m_ui64StartTime = 0;
	m_ifunc_dfu_transfer_size = 0;
	m_iBytesRemaining = 0;
	m_iStartBytesRemaining = 0;
	m_pdata = NULL;
	m_pDataStart = NULL;
	memset( m_elementheader, 0, sizeof( m_elementheader ) );
	memset( m_targetprefix, 0, sizeof( m_targetprefix ) );
	m_dwNbElements = 0;
	m_ielement = 0;
	m_dwElementAddress = 0;
	m_dwElementSize = 0;
	m_p = 0;
	m_iProgressPercent = 0;
	m_iPreviousProgressPercent = 0;
	memset( &m_dst, 0, sizeof( m_dst ) );
	m_iTargets = 0;
	m_iLoopTarget = 0;
	m_iPageSize = 0;
	m_iChunkSize = 0;
	m_uiAddress = 0;
	m_uiEraseAddress = 0;
	m_padif = NULL;
	m_uiCurrentPage = 0;
	m_uiTotalPagesToErase = 0;
	m_uiTotalChunksToWrite = 0;
	m_PollTime = 0;

	if( NULL != m_pProcessThread )
	{
		delete m_pProcessThread;
		m_pProcessThread = NULL;
	}

	m_eProcessStateEngine = PSE_NotStarted;
	m_rProcessStatus.m_bProcessStatus = true;
	m_rProcessStatus.m_eProcessError = PE_NoError;
	m_rProcessStatus.m_dwDFUInternalErrorCode = -1;

	if( NULL != m_prNewDFUSeVariables && NULL != m_prNewDFUSeVariables->dfu_root )
	{
		struct dfu_if *adif = m_prNewDFUSeVariables->dfu_root;

		while( NULL != adif)
		{
			if( NULL != adif->mem_layout )
			{
				free_segment_list( adif->mem_layout );
				adif->mem_layout = NULL;
			}

			adif = adif->next;
		}

		if( NULL != m_prNewDFUSeVariables->dfu_root->dev_handle )
		{
			if( NULL != m_prNewDFUSeVariables->dfu_root->interface )
			{
				// Claim interface has been done in the 'DpSVisioConnected' method.
				libusb_release_interface( m_prNewDFUSeVariables->dfu_root->dev_handle, m_prNewDFUSeVariables->dfu_root->interface );
			}

			// Open USB library has been done in the 'DpSVisioConnected' method.
			libusb_close( m_prNewDFUSeVariables->dfu_root->dev_handle );
			m_prNewDFUSeVariables->dfu_root->dev_handle = NULL;
		}
	}

	m_clDpSVisioNewDFUInformation.Reset();
}

bool CDpSVisioNewDFUCommunication::RetrieveDpSVisioInformation( CString strDeviceName )
{
	m_clDpSVisioNewDFUInformation.m_strDeviceName = strDeviceName.MakeLower();
	return true;
}

int CDpSVisioNewDFUCommunication::_DFUStartErase( CString *pstrDFUFileName, CListBox *pOutputList )
{
	if( NULL == pstrDFUFileName || TRUE == pstrDFUFileName->IsEmpty() )
	{
		CString str = TASApp.LoadLocalizedString( IDS_DLGDPSVISIO_DFUUPGRADERUNNING_DFUFILENAMEEMPTY );
		INSERT_STRING_IN_PLIST_AND_SCROLL( pOutputList, str );
		m_iOperationCode = PC_NothingRunning;

		if( NULL == pstrDFUFileName )
		{
			return DPVSISIOCOM_ERROR_DFU_STARTERASE_DFUFILENAMEPOINTERNULL;
		}
		else
		{
			return DPVSISIOCOM_ERROR_DFU_STARTERASE_DFUFILENAMEEMPTY;
		}
	}

	if( NULL == m_prNewDFUSeVariables )
	{
		OutputDebugString( _T("DFUCom::_DFUStartErase: 'm_prNewDFUSeVariables' not defined!\n") );

		CString str = TASApp.LoadLocalizedString( IDS_DLGDPSVISIO_DFUUPGRADERUNNING_CANTSTARTDFUUPGRADE );
		INSERT_STRING_IN_PLIST_AND_SCROLL( pOutputList, str );

		m_iOperationCode = PC_NothingRunning;
		return DPVSISIOCOM_ERROR_DFU_STARTERASE_DFUVARIABLESEMPTY;
	}

	// Try to read the DFU file.
	m_DFUFile.name = *pstrDFUFileName;
	int ret = dfu_load_file( &m_DFUFile, MAYBE_SUFFIX, MAYBE_PREFIX, 0 );

	if( ret != 0 )
	{
		CString str;
		str.Format( _T("DFUCom::_DFUStartErase: error when loading the '%s' file.!\n"), m_DFUFile.name );
		OutputDebugString( str );

		str = TASApp.LoadLocalizedString( IDS_DLGDPSVISIO_DFUUPGRADERUNNING_CANTREADDFU );
		INSERT_STRING_IN_PLIST_AND_SCROLL( pOutputList, str );

		m_iOperationCode = PC_NothingRunning;
		return DPVSISIOCOM_ERROR_DFU_STARTERASE_CANTREADDFUFILE;
	}

	// Verify file size.
	uint8_t dfuprefix[11];
	uint8_t elementheader[8];
	m_iBytesRemaining = m_DFUFile.size.total - m_DFUFile.size.prefix - m_DFUFile.size.suffix;
	m_pdata = m_DFUFile.firmware + m_DFUFile.size.prefix;

	if( m_iBytesRemaining < (int)(sizeof(dfuprefix) + sizeof( m_targetprefix ) + sizeof(elementheader)))
	{
		OutputDebugString( _T("DFUCom::_DFUStartErase: file too small for a DfuSe file") );

		CString str = TASApp.LoadLocalizedString( IDS_DLGDPSVISIO_DFUUPGRADERUNNING_CANTREADDFU );
		INSERT_STRING_IN_PLIST_AND_SCROLL( pOutputList, str );

		if( m_DFUFile.firmware != NULL )
		{
			free( m_DFUFile.firmware );
			m_DFUFile.firmware = NULL;
		}

		m_iOperationCode = PC_NothingRunning;
		return DPVSISIOCOM_ERROR_DFU_STARTERASE_CANTREADDFUFILE;
	}

	if( dfuse_memcpy( dfuprefix, &m_pdata, &m_iBytesRemaining, sizeof( dfuprefix ) ) != 0 )
	{
		OutputDebugString( _T("DFUCom::_DFUStartErase: problem when executing 'dfuse_memcpy.'") );

		CString str = TASApp.LoadLocalizedString( IDS_DLGDPSVISIO_DFUUPGRADERUNNING_CANTREADDFU );
		INSERT_STRING_IN_PLIST_AND_SCROLL( pOutputList, str );

		if( m_DFUFile.firmware != NULL )
		{
			free( m_DFUFile.firmware );
			m_DFUFile.firmware = NULL;
		}

		m_iOperationCode = PC_NothingRunning;
		return DPVSISIOCOM_ERROR_DFU_STARTERASE_CANTREADDFUFILE;
	}

	if( strncmp( (char *)dfuprefix, "DfuSe", 5 ) )
	{
		OutputDebugString( _T("DFUCom::_DFUStartErase: no valid DfuSe signature") );

		CString str = TASApp.LoadLocalizedString( IDS_DLGDPSVISIO_DFUUPGRADERUNNING_CANTREADDFU );
		INSERT_STRING_IN_PLIST_AND_SCROLL( pOutputList, str );

		if( m_DFUFile.firmware != NULL )
		{
			free( m_DFUFile.firmware );
			m_DFUFile.firmware = NULL;
		}

		m_iOperationCode = PC_NothingRunning;
		return DPVSISIOCOM_ERROR_DFU_STARTERASE_CANTREADDFUFILE;
	}

	if( dfuprefix[5] != 0x01 )
	{
		CString str;
		str.Format( _T("DFUCom::_DFUStartErase: DFU format revision %i not supported"), dfuprefix[5] );

		OutputDebugString( str );

		str = TASApp.LoadLocalizedString( IDS_DLGDPSVISIO_DFUUPGRADERUNNING_CANTREADDFU );
		INSERT_STRING_IN_PLIST_AND_SCROLL( pOutputList, str );

		if( m_DFUFile.firmware != NULL )
		{
			free( m_DFUFile.firmware );
			m_DFUFile.firmware = NULL;
		}

		m_iOperationCode = PC_NothingRunning;
		return DPVSISIOCOM_ERROR_DFU_STARTERASE_CANTREADDFUFILE;
	}

	m_iTargets = dfuprefix[10];
	CString str;
	str.Format( _T("DFUCom::_DFUStartErase: file contains %i DFU images\n"), m_iTargets );
	OutputDebugString( str );

	// Memorize start position for data and remaining size for the step between erase and upgrade.
	m_pDataStart = m_pdata;
	m_iStartBytesRemaining = m_iBytesRemaining;

	// Verify memory layout.
	struct dfu_if *adif;

	adif = m_prNewDFUSeVariables->dfu_root;

	while( NULL != adif )
	{
		adif->mem_layout = parse_memory_layout( (char *)adif->alt_name, 0 );

		if( NULL == adif->mem_layout )
		{
			str.Format( _T("DFUCom::_DFUStartErase: failed to parse memory layout for alternate interface %i.\n"), adif->altsetting );
			OutputDebugString( str );

			str = TASApp.LoadLocalizedString( IDS_DLGDPSVISIO_DFUUPGRADERUNNING_CANTSTARTDFUUPGRADE );
			INSERT_STRING_IN_PLIST_AND_SCROLL( pOutputList, str );

			if( m_DFUFile.firmware != NULL )
			{
				free( m_DFUFile.firmware );
				m_DFUFile.firmware = NULL;
			}

			return DPVSISIOCOM_ERROR_DFU_STARTERASE_MEMORYLAYOUTERROR;
		}

		if( adif->quirks & QUIRK_DFUSE_LAYOUT )
		{
			fixup_dfuse_layout( adif, &( adif->mem_layout ) );
		}

		adif = adif->next;
	}

	// Check the transfer size.
	m_ifunc_dfu_transfer_size = libusb_le16_to_cpu( m_prNewDFUSeVariables->dfu_root->func_dfu.wTransferSize );

	if( m_ifunc_dfu_transfer_size < m_prNewDFUSeVariables->dfu_root->bMaxPacketSize0 )
	{
		m_ifunc_dfu_transfer_size = m_prNewDFUSeVariables->dfu_root->bMaxPacketSize0;
	}

	if( 0 == m_ifunc_dfu_transfer_size )
	{
		OutputDebugString( _T("DFUCom::_DFUStartErase: can't determine the transfer size!\n") );

		str = TASApp.LoadLocalizedString( IDS_DLGDPSVISIO_DFUUPGRADERUNNING_CANTSTARTDFUUPGRADE );
		INSERT_STRING_IN_PLIST_AND_SCROLL( pOutputList, str );

		if( m_DFUFile.firmware != NULL )
		{
			free( m_DFUFile.firmware );
			m_DFUFile.firmware = NULL;
		}

		m_iOperationCode = PC_NothingRunning;
		return DPVSISIOCOM_ERROR_DFU_STARTERASE_TRANSFERSIZEUNKNOWN;
	}

	if( false == _CheckNumberOfPages() )
	{
		str.Format( _T("DFUCom::_DFUStartErase: can't check number of pages (%i-%i).\n"), m_rProcessStatus.m_eProcessError, m_rProcessStatus.m_dwDFUInternalErrorCode );
		OutputDebugString( str );

		str = TASApp.LoadLocalizedString( IDS_DLGDPSVISIO_DFUUPGRADERUNNING_CANTSTARTDFUUPGRADE );
		INSERT_STRING_IN_PLIST_AND_SCROLL( pOutputList, str );

		if( m_DFUFile.firmware != NULL )
		{
			free( m_DFUFile.firmware );
			m_DFUFile.firmware = NULL;
		}

		m_iOperationCode = PC_NothingRunning;
		return DPVSISIOCOM_ERROR_DFU_STARTERASE_CANTCHECKNUMBEROFPAGES;
	}

	m_pdata = m_pDataStart;
	m_iBytesRemaining = m_iStartBytesRemaining;

	// Create an event to stop the thread when needed.
	if( NULL == m_pclStopThreadEvent )
	{
		m_pclStopThreadEvent = new CEvent( FALSE, TRUE );
	}

	if( NULL == m_pclStopThreadEvent )
	{
		if( m_DFUFile.firmware != NULL )
		{
			free( m_DFUFile.firmware );
			m_DFUFile.firmware = NULL;
		}

		OutputDebugString( _T("DFUCom::_DFUStartErase: can't create stop event!\n") );
		return DPVSISIOCOM_ERROR_DFU_STARTUPGRADE_CANTCREATESTOPEVENT;
	}

	m_pclStopThreadEvent->ResetEvent();

	// Create an event to allow the thread to set when it is well stopped.
	if( NULL == m_pclThreadStoppedEvent )
	{
		m_pclThreadStoppedEvent = new CEvent( FALSE, TRUE );
	}

	if( NULL == m_pclThreadStoppedEvent )
	{
		if( m_DFUFile.firmware != NULL )
		{
			free( m_DFUFile.firmware );
			m_DFUFile.firmware = NULL;
		}

		OutputDebugString( _T("DFUCom::_DFUStartErase: can't create stopped event!\n") );
		return DPVSISIOCOM_ERROR_DFU_STARTUPGRADE_CANTCREATESTOPPEDEVENT;
	}

	m_pclThreadStoppedEvent->ResetEvent();

	m_rProcessStatus.m_bProcessStatus = true;
	m_rProcessStatus.m_eProcessError = PE_NoError;
	m_rProcessStatus.m_dwDFUInternalErrorCode = -1;
	m_uiCurrentPage = 0;
	m_iProgressPercent = 0;
	m_iPreviousProgressPercent = 0;
	m_iLoopTarget = 1;
	m_eProcessStateEngine = PSE_RunOneTargetInFile;
	m_iOperationCode = PC_DFUErase;
	m_ui64StartTime = std::chrono::duration_cast<std::chrono::milliseconds>( std::chrono::system_clock::now().time_since_epoch() ).count();
	m_pProcessThread = AfxBeginThread( &ThreadProcessRunning, ( LPVOID )this );

	return DPSVISIOCOM_NOERROR;
}

int CDpSVisioNewDFUCommunication::_DFUCheckErase( BYTE *pPercentCompleted, CListBox *pOutputList )
{
	if( NULL == pPercentCompleted )
	{
		return DPVSISIOCOM_ERROR_DFU_CHECKERASE_PERCENTPOINTERNULL;
	}

	if( m_iOperationCode != PC_DFUErase )
	{
		OutputDebugString( _T("DpSVisioNewDFUCommunication::_DFUCheckErase: we are not in 'Erase' mode!\n") );
		m_iOperationCode = PC_NothingRunning;
		return DPVSISIOCOM_ERROR_DFU_CHECKERASE_NOOPERATIONRUNNING;
	}

	EnterCriticalSection( &m_CriticalSection );

	// Reset timeout if there is activty.
	if( m_iProgressPercent != m_iPreviousProgressPercent )
	{
		m_ui64StartTime = std::chrono::duration_cast<std::chrono::milliseconds>( std::chrono::system_clock::now().time_since_epoch() ).count();
		m_iPreviousProgressPercent = m_iProgressPercent;
	}

	// Check timeout.
	uint64_t ui64CurrentTime = std::chrono::duration_cast<std::chrono::milliseconds>( std::chrono::system_clock::now().time_since_epoch() ).count();

	if( ui64CurrentTime - m_ui64StartTime > 35000 )
	{
		OutputDebugString( _T("DpSVisioNewDFUCommunication::_DFUCheckErase: erase time out reached!\n") );
		CString str = TASApp.LoadLocalizedString( IDS_DLGDPSVISIO_DFUUPGRADERUNNING_ERASEERROR );
		INSERT_STRING_IN_PLIST_AND_SCROLL( pOutputList, str );

		_StopThreadProcessRunning();
		m_iOperationCode = PC_NothingRunning;
		LeaveCriticalSection( &m_CriticalSection );
		return DPVSISIOCOM_ERROR_DFU_CHECKERASE_TIMEOUTREACHED;
	}

	int iReturn = DPSVISIOCOM_NOERROR;

	// Retrieve % erase.
	if( PSE_WaitingToBeKilled == m_eProcessStateEngine )
	{
		bool bProcessStatus = m_rProcessStatus.m_bProcessStatus;

		// Erase process finished.
		_StopThreadProcessRunning();
		m_iOperationCode = PC_NothingRunning;

		// By setting 100, we warn the 'CDlgDpSVisioDFUUpgradeRunning' dialog that the process is finished.
		*pPercentCompleted = 100;

		if( false == bProcessStatus )
		{
			iReturn = DPVSISIOCOM_ERROR_DFU_CHECKERASE_CMDNOTCORRECTLYEXECUTED;
		}
	}
	else
	{
		*pPercentCompleted = m_iProgressPercent;
	}

	LeaveCriticalSection( &m_CriticalSection );

	return iReturn;
}

int CDpSVisioNewDFUCommunication::_DFUStartUpgrade( CListBox *pOutputList )
{
	// Create an event to stop the thread when needed.
	if( NULL == m_pclStopThreadEvent )
	{
		m_pclStopThreadEvent = new CEvent( FALSE, TRUE );
	}

	if( NULL == m_pclStopThreadEvent )
	{
		OutputDebugString( _T("DpSVisioNewDFUCommunication::_DFUStartUpgrade: can't create stop event!\n") );
		return DPVSISIOCOM_ERROR_DFU_STARTUPGRADE_CANTCREATESTOPEVENT;
	}

	m_pclStopThreadEvent->ResetEvent();

	// Create an event to allow the thread to set when it is well stopped.
	if( NULL == m_pclThreadStoppedEvent )
	{
		m_pclThreadStoppedEvent = new CEvent( FALSE, TRUE );
	}

	if( NULL == m_pclThreadStoppedEvent )
	{
		OutputDebugString( _T("DpSVisioNewDFUCommunication::_DFUStartUpgrade: can't create stopped event!\n") );
		return DPVSISIOCOM_ERROR_DFU_STARTUPGRADE_CANTCREATESTOPPEDEVENT;
	}

	m_pclThreadStoppedEvent->ResetEvent();

	// Reset the position in the DFU file.
	m_pdata = m_pDataStart;
	m_iBytesRemaining = m_iStartBytesRemaining;

	m_rProcessStatus.m_bProcessStatus = true;
	m_rProcessStatus.m_eProcessError = PE_NoError;
	m_rProcessStatus.m_dwDFUInternalErrorCode = -1;
	m_uiCurrentPage = 0;
	m_iProgressPercent = 0;
	m_iLoopTarget = 1;
	m_eProcessStateEngine = PSE_RunOneTargetInFile;
	m_iOperationCode = PC_DFUUpgrade;
	m_pProcessThread = AfxBeginThread( &ThreadProcessRunning, ( LPVOID )this );

	return DPSVISIOCOM_NOERROR;
}

int CDpSVisioNewDFUCommunication::_DFUCheckUpgrade( BYTE *pPercentCompleted, CListBox *pOutputList )
{
	if( NULL == pPercentCompleted )
	{
		return DPVSISIOCOM_ERROR_DFU_CHECKUPGRADE_PERCENTPOINTERNULL;
	}

	if( m_iOperationCode != PC_DFUUpgrade )
	{
		return DPVSISIOCOM_ERROR_DFU_CHECKUPGRADE_NOOPERATIONRUNNING;
	}

	int iReturn = DPSVISIOCOM_NOERROR;

	EnterCriticalSection( &m_CriticalSection );

	// Retrieve % upgrade.
	if( PSE_WaitingToBeKilled == m_eProcessStateEngine )
	{
		bool bProcessStatus = m_rProcessStatus.m_bProcessStatus;

		// Upgrade process finished.
		_StopThreadProcessRunning();
		m_iOperationCode = PC_NothingRunning;

		// By setting 100, we warn the 'CDlgDpSVisioDFUUpgradeRunning' dialog that the process is finished.
		*pPercentCompleted = 100;

		if( false == bProcessStatus )
		{
			iReturn = DPVSISIOCOM_ERROR_DFU_CHECKUPGRADE_CMDNOTCORRECTLYEXECUTED;
		}
	}
	else
	{
		*pPercentCompleted = m_iProgressPercent;
	}

	LeaveCriticalSection( &m_CriticalSection );

	return iReturn;
}

int CDpSVisioNewDFUCommunication::_DFUDetach( CListBox *pOutputList )
{
	USB_DEVICE_DESCRIPTOR *pDesc = new USB_DEVICE_DESCRIPTOR;

	if( NULL == pDesc )
	{
		return -1;
	}

	int iret = dfu_detach( m_prNewDFUSeVariables->dfu_root->dev_handle, m_prNewDFUSeVariables->dfu_root->interface, 1000 );

	if( iret < 0 )
	{
		CString str = TASApp.LoadLocalizedString( IDS_DLGDPSVISIO_DFUUPGRADERUNNING_DFUCANTDETACH );
		INSERT_STRING_IN_PLIST_AND_SCROLL( pOutputList, str );

		CString strError;
		strError.Format( _T("%i"), iret );
		FormatString( str, IDS_DLGDPSVISIO_DFUUPGRADERUNNING_ERRORCODE, strError );
		INSERT_STRING_IN_PLIST_AND_SCROLL( pOutputList, str );

		return -1;
	}

	return DPSVISIOCOM_NOERROR;
}

void CDpSVisioNewDFUCommunication::_ProcessRunning()
{
	EnterCriticalSection( &m_CriticalSection );

	int iret = 0;
	int iRetry = 3;
	bool bContinue = true;
	m_PollTime = 0;

	switch( m_eProcessStateEngine )
	{
		// dfuse.c: dfPSE_do_dfPSE_dnload: loop all targets in the DFU file.
		case PSE_RunOneTargetInFile:

			if( m_iLoopTarget > m_iTargets )
			{
				OUTPUT_DEBUG_STRING( _T("DFUCom::_ProcessRunning::PSE_RunOneTargetInFile: all targets in file are processed.\n") );

				m_eProcessStateEngine = PSE_RunAllTargetsInFileFinished;
				bContinue = false;
			}

			if( true == bContinue )
			{
				OUTPUT_DEBUG_STRING( _T("DFUCom::_ProcessRunning::PSE_RunOneTargetInFile: parsing DFU image %i.\n"), m_iLoopTarget );

				iret = dfuse_memcpy( m_targetprefix, &m_pdata, &m_iBytesRemaining, sizeof( m_targetprefix ) );

				if( iret != 0 )
				{
					OUTPUT_DEBUG_STRING( _T("DFUCom::_ProcessRunning::PSE_RunOneTargetInFile: error when calling the 'dfuse_memcpy' method (%i).\n"), iret );

					m_eProcessStateEngine = _SetProcessError( PE_InternalError, PIE_DFUSeMemcpyError );
					bContinue = false;
				}
			}

			if( true == bContinue )
			{
				if( strncmp( (char *)m_targetprefix, "Target", 6 ) )
				{
					CStringA strA1 = (char *)m_targetprefix;
					OUTPUT_DEBUG_STRINGA( "DFUCom::_ProcessRunning::PSE_RunOneTargetInFile: target prefix not correct (%s).\n", (LPCSTR)strA1.Left( 6 ) );

					m_eProcessStateEngine = _SetProcessError( PE_DFUFileNoValidTargetSignature, 0 );
					bContinue = false;
				}
			}

			if( true == bContinue )
			{
				if( m_targetprefix[7] )
				{
					OUTPUT_DEBUG_STRINGA( "DFUCom::_ProcessRunning::PSE_RunOneTargetInFile: target name: %s.\n", &m_targetprefix[11] );
				}
				else
				{
					OUTPUT_DEBUG_STRING( _T("DFUCom::_ProcessRunning::PSE_RunOneTargetInFile: no target name: %s.\n") );
				}

				m_dwNbElements = _quad2uint( (unsigned char *)m_targetprefix + 270 );

				OUTPUT_DEBUG_STRING( _T("DFUCom::_ProcessRunning::PSE_RunOneTargetInFile: number of elements in the current target: %i.\n"), m_dwNbElements );

				m_eProcessStateEngine = PSE_CheckAlternateSetting;
			}

			break;

		// dfuse.c: dfPSE_do_dfPSE_dnload: set alternate setting interface for the current target.
		case PSE_CheckAlternateSetting:
		{
			m_padif = m_prNewDFUSeVariables->dfu_root;
			int bAlternateSetting = m_targetprefix[6];

			OUTPUT_DEBUG_STRING( _T("DFUCom::_ProcessRunning::PSE_CheckAlternateSetting: image for alternate setting %i.\n"), bAlternateSetting );

			while( NULL != m_padif && true == bContinue )
			{
				if( bAlternateSetting == m_padif->altsetting )
				{
					OUTPUT_DEBUG_STRING( _T("DFUCom::_ProcessRunning::PSE_CheckAlternateSetting: setting Alternate Interface #%d ...\n"), m_padif->altsetting );

					m_padif->dev_handle = m_prNewDFUSeVariables->dfu_root->dev_handle;
					iret = libusb_set_interface_alt_setting( m_padif->dev_handle, m_padif->interface, m_padif->altsetting );

					if( iret < 0 )
					{
						OUTPUT_DEBUG_STRING( _T("DFUCom::_ProcessRunning::PSE_CheckAlternateSetting: cannot set alternate interface: %s.\n"), libusb_error_name( iret ) );

						m_eProcessStateEngine = _SetProcessError( PE_InternalError, PE_CantSetAlternateSetting );
						bContinue = false;
					}
					break;
				}
				m_padif = m_padif->next;
			}

			if( true == bContinue )
			{
				if( NULL == m_padif )
				{
					OUTPUT_DEBUG_STRING( _T("DFUCom::_ProcessRunning::PSE_CheckAlternateSetting: no alternate setting %d (skipping elements).\n"), bAlternateSetting );
				}

				m_dwElementAddress = 0;
				m_dwElementSize = 0;
				m_ielement = 1;

				m_eProcessStateEngine = PSE_InitOneElement;
			}
		}	
		break;

		// dfuse.c: dfPSE_do_dfPSE_dnload: run all elements in the current target.
		case PSE_InitOneElement:

			OUTPUT_DEBUG_STRING( _T("DFUCom::_ProcessRunning::PSE_InitOneElement: parsing element %i.\n"), m_ielement );

			if( m_ielement > m_dwNbElements )
			{
				// All elements in the current target are processed, we can now go to the next target.
				OUTPUT_DEBUG_STRING( _T("DFUCom::_ProcessRunning::PSE_InitOneElement: all elements in the current target are processed.\n") );
				
				m_iLoopTarget++;
				m_eProcessStateEngine = PSE_RunOneTargetInFile;
				bContinue = false;
			}

			if( true == bContinue )
			{
				iret = dfuse_memcpy( m_elementheader, &m_pdata, &m_iBytesRemaining, sizeof( m_elementheader ) );

				if( iret != 0 )
				{
					OUTPUT_DEBUG_STRING( _T("DFUCom::_ProcessRunning::PSE_InitOneElement: error when calling the 'dfuse_memcpy' method (%i).\n"), iret );

					m_eProcessStateEngine = _SetProcessError( PE_InternalError, PIE_DFUSeMemcpyError );
					bContinue = false;
				}
			}

			if( true == bContinue )
			{
				m_dwElementAddress = _quad2uint( (unsigned char *)m_elementheader );
				m_dwElementSize = _quad2uint( (unsigned char *)m_elementheader + 4 );

				OUTPUT_DEBUG_STRING( _T("DFUCom::_ProcessRunning::PSE_InitOneElement: element address = 0x%08x.\n"), m_dwElementAddress );
				OUTPUT_DEBUG_STRING( _T("DFUCom::_ProcessRunning::PSE_InitOneElement: element size = %i.\n"), m_dwElementSize );

				if( (int)m_dwElementSize > m_iBytesRemaining )
				{
					OUTPUT_DEBUG_STRING( _T("DFUCom::_ProcessRunning::PSE_InitOneElement: file too small for element size (%i < %i).\n"), m_iBytesRemaining, m_dwElementSize );

					m_eProcessStateEngine = _SetProcessError( PE_InternalError, PIE_FileTooSmallForElementSize );
					bContinue = false;
				}
			}

			if( true == bContinue )
			{
				if( NULL == m_padif )
				{
					m_ielement++;
					dfuse_memcpy( NULL, &m_pdata, &m_iBytesRemaining, m_dwElementSize );
					m_eProcessStateEngine = PSE_InitOneElement;
					bContinue = false;
				}
			}

			if( true == bContinue )
			{
				// dfuse.c: dfPSE_dnload_element.

				OUTPUT_DEBUG_STRING( _T("DFUCom::_ProcessRunning::PSE_InitOneElement: check if we can write to the last address.\n") );
				struct memsegment *segment;

				// Check at least that we can write to the last address.
				segment = find_segment( m_padif->mem_layout, m_dwElementAddress + m_dwElementSize - 1 );

				if( !segment || !( segment->memtype & DFUSE_WRITEABLE ) )
				{
					if( !segment )
					{
						OUTPUT_DEBUG_STRING( _T("DFUCom::_ProcessRunning::PSE_InitOneElement: segment to process at the last address is not found.\n") );
					}
					else
					{
						OUTPUT_DEBUG_STRING( _T("DFUCom::_ProcessRunning::PSE_InitOneElement: segment to process found but it is not writable (mem type: %i).\n"), segment->memtype );
					}

					m_eProcessStateEngine = _SetProcessError( PE_DFUDeviceLastPageNotWritable, 0 );
					bContinue = false;
				}
			}

			if( true == bContinue )
			{
				m_p = 0;

				if( PC_DFUErase == m_iOperationCode )
				{
					m_eProcessStateEngine = PSE_EraseOneElementOneChunk;
				}
				else
				{
					iRetry = 3;
					m_eProcessStateEngine = PSE_UpgradeOneElementOneChunk;
				}
			}

			break;

		case PSE_EraseOneElementOneChunk:
		{
			OUTPUT_DEBUG_STRING( _T("DFUCom::_ProcessRunning::PSE_EraseOneElementOneChunk: erase one chunk.\n") );

			if( m_p >= (int)m_dwElementSize )
			{
				OUTPUT_DEBUG_STRING( _T("DFUCom::_ProcessRunning::PSE_EraseOneElementOneChunk: all chunks of the element have been erased.\n") );
				OUTPUT_DEBUG_STRING( _T("DFUCom::_ProcessRunning::PSE_EraseOneElementOneChunk: we can now process the current element.\n") );

				m_ielement++;
				dfuse_memcpy( NULL, &m_pdata, &m_iBytesRemaining, m_dwElementSize );
				m_eProcessStateEngine = PSE_InitOneElement;
				bContinue = false;
			}

			m_iChunkSize = m_ifunc_dfu_transfer_size;
			struct memsegment *segment = NULL;

			if( true == bContinue )
			{
				// Find segment.
				m_uiAddress = m_dwElementAddress + m_p;
				
				OUTPUT_DEBUG_STRING( _T("DFUCom::_ProcessRunning::PSE_EraseOneElementOneChunk: chunk address = 0x%08x.\n"), m_uiAddress );

				segment = find_segment( m_padif->mem_layout, m_uiAddress );

				if( !segment || !( segment->memtype & DFUSE_WRITEABLE ) ) 
				{
					if( !segment )
					{
						OUTPUT_DEBUG_STRING( _T("DFUCom::_ProcessRunning::PSE_EraseOneElementOneChunk: segment not found.\n") );
					}
					else
					{
						OUTPUT_DEBUG_STRING( _T("DFUCom::_ProcessRunning::PSE_EraseOneElementOneChunk: segment found but it is not writeable (mem type: %i).\n"), segment->memtype );
					}

					m_eProcessStateEngine = _SetProcessError( PE_InternalError, PIE_DFUCantSegmentNotErasable );
					bContinue = false;
				}
			}

			if( true == bContinue )
			{
				// If the location is not in the memory map we skip erasing since we wouldn't know the correct page size for flash erase.
				if( !segment )
				{
					OUTPUT_DEBUG_STRING( _T("DFUCom::_ProcessRunning::PSE_EraseOneElementOneChunk: segment not found, continue with next chunk address.\n") );

					// Continue in the current element.
					m_p += m_ifunc_dfu_transfer_size;
					m_eProcessStateEngine = PSE_EraseOneElementOneChunk;
					bContinue = false;
				}
			}

			if( true == bContinue )
			{
				m_iPageSize = segment->pagesize;

				// Check if this is the last chunk.
				if( m_p + m_iChunkSize > (int)m_dwElementSize )
				{
					m_iChunkSize = m_dwElementSize - m_p;
				}

				OUTPUT_DEBUG_STRING( _T("DFUCom::_ProcessRunning::PSE_EraseOneElementOneChunk: page size = %i.\n"), m_iPageSize );
				OUTPUT_DEBUG_STRING( _T("DFUCom::_ProcessRunning::PSE_EraseOneElementOneChunk: chunk size = %i.\n"), m_iChunkSize );

				if( !( segment->memtype & DFUSE_ERASABLE ) )
				{
					OUTPUT_DEBUG_STRING( _T("DFUCom::_ProcessRunning::PSE_EraseOneElementOneChunk: segment not erasable, continue with next chunk address.\n") );

					// Continue in the current element.
					m_p += m_ifunc_dfu_transfer_size;
					m_eProcessStateEngine = PSE_EraseOneElementOneChunk;
					bContinue = false;
				}
			}

			if( true == bContinue )
			{
				m_uiEraseAddress = m_uiAddress;
				m_uiLastErasedPage = 1;
				m_eProcessStateEngine = PSE_EraseOneElementOneChunkOnePage;
			}
		}
		break;

		case PSE_EraseOneElementOneChunkOnePage:
		{
			OUTPUT_DEBUG_STRING( _T("DFUCom::_ProcessRunning::PSE_EraseOneElementOneChunkOnePage: go to erase the current page.\n") );

			if( m_uiEraseAddress >= m_uiAddress + m_iChunkSize )
			{
				OUTPUT_DEBUG_STRING( _T("DFUCom::_ProcessRunning::PSE_EraseOneElementOneChunkOnePage: we reach the last page.\n") );

				m_eProcessStateEngine = PSE_EraseOneElementOneChunkLastPage;
				bContinue = false;
			}

			if( true == bContinue )
			{
				if( ( m_uiEraseAddress & ~( m_iPageSize - 1 ) ) != m_uiLastErasedPage )
				{
					m_uiCurrentPage++;

					OUTPUT_DEBUG_STRING( _T("DFUCom::_ProcessRunning::PSE_EraseOneElementOneChunkOnePage: erase page at address = 0x%08x.\n"), m_uiEraseAddress );

					// When a command is sent, we need to call twice the "DFU_GETSTATUS". This is done in the 'dfuse_special_command' function.
					iret = dfuse_special_command( m_padif, m_uiEraseAddress, ERASE_PAGE, 0, &m_uiLastErasedPage );

					if( iret < 0 )
					{
						OUTPUT_DEBUG_STRING( _T("DFUCom::_ProcessRunning::PSE_EraseOneElementOneChunkOnePage: error when erasing page (%i).\n"), iret );

						m_eProcessStateEngine = _SetProcessError( PE_InternalError, PIE_DFUCantExecuteSpecialCommand );
						bContinue = false;
					}

					m_iProgressPercent = min( (int)( ( (double)m_uiCurrentPage / (double)m_uiTotalPagesToErase ) * 100.0 ), 99 );
				}
			}

			if( true == bContinue )
			{
				m_uiEraseAddress += m_iPageSize;
				m_eProcessStateEngine = PSE_EraseOneElementOneChunkOnePage;
			}
		}
		break;

		case PSE_EraseOneElementOneChunkLastPage:
		{
			OUTPUT_DEBUG_STRING( _T("DFUCom::_ProcessRunning::PSE_EraseOneElementOneChunkLastPage: last page, check if we need to erase last chunk.\n") );

			if( ( ( m_uiAddress + m_iChunkSize - 1 ) & ~( m_iPageSize - 1 ) ) != m_uiLastErasedPage )
			{
				m_uiCurrentPage++;

				OUTPUT_DEBUG_STRING( _T("DFUCom::_ProcessRunning::PSE_EraseOneElementOneChunkLastPage: erase last page at address = 0x%08x.\n"), m_uiAddress + m_iChunkSize - 1 );

				// When a command is sent, we need to call twice the "DFU_GETSTATUS". This is done in the 'dfuse_special_command' function.
				iret = dfuse_special_command( m_padif, m_uiAddress + m_iChunkSize - 1, ERASE_PAGE, 0, &m_uiLastErasedPage );

				if( iret < 0 )
				{
					OUTPUT_DEBUG_STRING( _T("DFUCom::_ProcessRunning::PSE_EraseOneElementOneChunkLastPage: error when erasing the last page (%i).\n"), iret );

					m_eProcessStateEngine = _SetProcessError( PE_InternalError, PIE_DFUCantExecuteSpecialCommand );
					bContinue = false;
				}

				m_iProgressPercent = min( (int)( ( (double)m_uiCurrentPage / (double)m_uiTotalPagesToErase ) * 100.0 ), 99 );
			}

			if( true == bContinue )
			{
				// m_iProgressPercent = m_p / m_dwElementSize;
				m_p += m_ifunc_dfu_transfer_size;
				m_eProcessStateEngine = PSE_EraseOneElementOneChunk;
			}
		}
		break;

		case PSE_UpgradeOneElementOneChunk:
		{
			OUTPUT_DEBUG_STRING( _T("DFUCom::_ProcessRunning::PSE_UpgradeOneElementOneChunk: upgrade one chunk.\n") );

			if( m_p >= (int)m_dwElementSize )
			{
				OUTPUT_DEBUG_STRING( _T("DFUCom::_ProcessRunning::PSE_UpgradeOneElementOneChunk: element upgraded, go to the next element.\n") );

				// Finished. Go the the next element.
				m_ielement++;
				dfuse_memcpy( NULL, &m_pdata, &m_iBytesRemaining, m_dwElementSize );
				m_eProcessStateEngine = PSE_InitOneElement;
				bContinue = false;
			}

			int chunk_size = m_ifunc_dfu_transfer_size;

			if( true == bContinue )
			{
				unsigned int address = m_dwElementAddress + m_p;

				// Check if this is the last chunk.
				if( m_p + chunk_size > (int)m_dwElementSize )
				{
					chunk_size = m_dwElementSize - m_p;
				}

				OUTPUT_DEBUG_STRING( _T("DFUCom::_ProcessRunning::PSE_UpgradeOneElementOneChunk: address = 0x%08x.\n"), address );
				OUTPUT_DEBUG_STRING( _T("DFUCom::_ProcessRunning::PSE_UpgradeOneElementOneChunk: chunk size = %i.\n"), chunk_size );

				// When a command is sent, we need to call twice the "DFU_GETSTATUS". This is done in the 'dfuse_special_command' function.
				iret = dfuse_special_command( m_padif, address, SET_ADDRESS, 0 );

				if( iret < 0 )
				{
					OUTPUT_DEBUG_STRING( _T("DFUCom::_ProcessRunning::PSE_UpgradeOneElementOneChunk: error when upgrading chunk (%i).\n"), iret );

					m_eProcessStateEngine = _SetProcessError( PE_InternalError, PIE_DFUCantExecuteSpecialCommand );
					bContinue = false;
				}
			}

			if( true == bContinue )
			{
				OUTPUT_DEBUG_STRING( _T("DFUCom::_ProcessRunning::PSE_UpgradeOneElementOneChunk: upgrading chunk...\n") );

				// the "dfuse_download" function simply sends the "DFU_DNLOAD" command. So, as described in the DFU standard, we need to send now
				// the first "DFU_GETSTATUS" that will allow the device to launch the write operation. We must receive the "DFU_STATE_dfuDNBUSY" status.
				// And we need to call a second time "DFU_GETSTATUS" to know the writing status.
				iret = dfuse_download( m_padif, chunk_size, chunk_size ? m_pdata + m_p : NULL, 2 );

				if( iret < 0 )
				{
					OUTPUT_DEBUG_STRING( _T("DFUCom::_ProcessRunning::PSE_UpgradeOneElementOneChunk: error when upgrading chunk (%i).\n"), iret );

					m_eProcessStateEngine = _SetProcessError( PE_InternalError, PIE_DFUCantWriteChunkElement );
					bContinue = false;
				}
			}

			if( true == bContinue )
			{
				m_eProcessStateEngine = PSE_UpgradeOneElementOneChunkFirstGetStatus;
			}
		}
		break;

		case PSE_UpgradeOneElementOneChunkFirstGetStatus:
		{
			OUTPUT_DEBUG_STRING( _T("DFUCom::_ProcessRunning::PSE_UpgradeOneElementOneChunkFirstGetStatus: first get status to allow device to launch writing.\n") );

			// We wait a little because the call to 'dfu_get_status' to fast can sometimes generate errors.
			Sleep( 5 );

			iret = dfu_get_status( m_padif, &m_dst );

			if( iret < 0 )
			{
				OUTPUT_DEBUG_STRING( _T("DFUCom::_ProcessRunning::PSE_UpgradeOneElementOneChunkFirstGetStatus: error when trying to get DFU status (%i).\n"), iret );

				// Check the DFU state.
				if( STATE_DFU_ERROR == m_dst.bState )
				{
					dfu_clear_status( m_padif->dev_handle, m_padif->interface );

					if( iret < 0 )
					{
						OUTPUT_DEBUG_STRING( _T("DFUCom::_ProcessRunning::PSE_UpgradeOneElementOneChunkFirstGetStatus: error when trying to clear DFU state (%i).\n"), iret );
						m_eProcessStateEngine = _SetProcessError( PE_InternalError, PIE_DFUErrorWhenClearingState );
						bContinue = false;
					}
				}

				if( true == bContinue )
				{
					iRetry--;

					if( iRetry <= 0 )
					{
						// Number of retry reached, abort.
						OUTPUT_DEBUG_STRING( _T("DFUCom::_ProcessRunning::PSE_UpgradeOneElementOneChunkFirstGetStatus: number of retry reached -> abort.\n") );
						m_eProcessStateEngine = _SetProcessError( PE_InternalError, PIE_DFUErrorWhenReadingState );
					}
					else
					{
						// Retry the download.
						OUTPUT_DEBUG_STRING( _T("DFUCom::_ProcessRunning::PSE_UpgradeOneElementOneChunkFirstGetStatus: retry the download operation (Retry: %i)\n"), iRetry );
						m_eProcessStateEngine = PSE_UpgradeOneElementOneChunk;
					}

					bContinue = false;
				}
			}

			if( true == bContinue )
			{
				if( STATE_DFU_ERROR == m_dst.bState )
				{
					iret = dfu_clear_status( m_padif->dev_handle, m_padif->interface );

					if( iret < 0 )
					{
						OUTPUT_DEBUG_STRING( _T("DFUCom::_ProcessRunning::PSE_UpgradeOneElementOneChunkFirstGetStatus: error when trying to clear DFU state (%i).\n"), iret );
						m_eProcessStateEngine = _SetProcessError( PE_InternalError, PIE_DFUErrorWhenClearingState );
					}
					else
					{
						iRetry--;

						if( iRetry <= 0 )
						{
							// Number of retry reached, abort.
							OUTPUT_DEBUG_STRING( _T("DFUCom::_ProcessRunning::PSE_UpgradeOneElementOneChunkFirstGetStatus: number of retry reached -> abort.\n") );
							m_eProcessStateEngine = _SetProcessError( PE_InternalError, PIE_DFUErrorToManyRetryWhenUpgrading );
						}
						else
						{
							// Retry the download.
							OUTPUT_DEBUG_STRING( _T("DFUCom::_ProcessRunning::PSE_UpgradeOneElementOneChunkFirstGetStatus: retry the download operation (Retry: %i)\n"), iRetry );
							m_eProcessStateEngine = PSE_UpgradeOneElementOneChunk;
						}
					}

					bContinue = false;
				}
				else
				{
					OUTPUT_DEBUG_STRING( _T("DFUCom::_ProcessRunning::PSE_UpgradeOneElementOneChunkFirstGetStatus: writing launched...\n") );

					if( m_dst.bStatus != STATUS_OK ||
							( m_dst.bState != STATE_DFU_DOWNLOAD_IDLE && m_dst.bState != STATE_DFU_DOWNLOAD_BUSY && m_dst.bState != STATE_DFU_MANIFEST ) )
					{
						OUTPUT_DEBUG_STRING( _T("DFUCom::_ProcessRunning::PSE_UpgradeOneElementOneChunkFirstGetStatus: error when launching the writting!\n") );

						if( m_dst.bStatus != STATUS_OK )
						{
							m_eProcessStateEngine = _SetProcessError( PE_InternalError, PIE_DFUErrorWhenWritingChunkElement );
						}
						else
						{
							m_eProcessStateEngine = _SetProcessError( PE_InternalError, PIE_DFUErrorBadFirmwareStateMachine );
						}

						bContinue = false;
					}
					else
					{
						OUTPUT_DEBUG_STRING( _T("DFUCom::_ProcessRunning::PSE_UpgradeOneElementOneChunkFirstGetStatus: no error when lauching the writing, go to wait the end.\n") );

						m_PollTime = m_dst.bwPollTimeout;
						m_eProcessStateEngine = PSE_UpgradeOneElementOneChunkWaitEndOfWritting;
					}
				}
			}
		}
		break;

		case PSE_UpgradeOneElementOneChunkWaitEndOfWritting:
		{
			OUTPUT_DEBUG_STRING( _T("DFUCom::_ProcessRunning::PSE_UpgradeOneElementOneChunkWaitEndOfWritting: get state to check if writing operation is done.\n") );

			iret = dfu_get_state( m_padif->dev_handle, m_padif->interface );

			if( iret < 0 )
			{
				OUTPUT_DEBUG_STRING( _T("DFUCom::_ProcessRunning::PSE_UpgradeOneElementOneChunkWaitEndOfWritting: error when trying to get DFU state (%i).\n"), iret );

				m_eProcessStateEngine = _SetProcessError( PE_InternalError, PIE_DFUErrorWhenReadingState );
				bContinue = false;
			}

			if( true == bContinue )
			{
				if( DFU_STATE_dfuDNBUSY == m_dst.bState )
				{
					OUTPUT_DEBUG_STRING( _T("DFUCom::_ProcessRunning::PSE_UpgradeOneElementOneChunkWaitEndOfWritting: DFU state is downloading busy.\n") );
					OUTPUT_DEBUG_STRING( _T("DFUCom::_ProcessRunning::PSE_UpgradeOneElementOneChunkWaitEndOfWritting: trying to get DFU status.\n") );

					iret = dfu_get_status( m_padif, &m_dst );

					if( iret < 0 )
					{
						OUTPUT_DEBUG_STRING( _T("DFUCom::_ProcessRunning::PSE_UpgradeOneElementOneChunkWaitEndOfWritting: error when trying to get DFU status (%i).\n"), iret );

						m_eProcessStateEngine = _SetProcessError( PE_InternalError, PIE_DFUErrorWhenReadingStatus );
						bContinue = false;
					}
					else
					{
						if( m_dst.bStatus != STATUS_OK )
						{
							OUTPUT_DEBUG_STRING( _T("DFUCom::_ProcessRunning::PSE_UpgradeOneElementOneChunkWaitEndOfWritting: DFU status is not OK (%i).\n"), iret );
							OUTPUT_DEBUG_STRING( _T("DFUCom::_ProcessRunning::PSE_UpgradeOneElementOneChunkWaitEndOfWritting: clear the DFU status.\n") );

							iret = dfu_clear_status( m_padif->dev_handle, m_padif->interface );

							if( iret < 0 )
							{
								OUTPUT_DEBUG_STRING( _T("DFUCom::_ProcessRunning::PSE_UpgradeOneElementOneChunkWaitEndOfWritting: error when trying to clear DFU state (%i).\n"), iret );
								m_eProcessStateEngine = _SetProcessError( PE_InternalError, PIE_DFUErrorWhenClearingState );
								bContinue = false;
							}
							else
							{
								iRetry--;

								if( iRetry <= 0 )
								{
									// Number of retry reached, abort.
									OUTPUT_DEBUG_STRING( _T("DFUCom::_ProcessRunning::PSE_UpgradeOneElementOneChunkWaitEndOfWritting: number of retry reached -> abort.\n") );
									m_eProcessStateEngine = _SetProcessError( PE_InternalError, PIE_DFUErrorToManyRetryWhenUpgrading );
								}
								else
								{
									// Retry the download.
									OUTPUT_DEBUG_STRING( _T("DFUCom::_ProcessRunning::PSE_UpgradeOneElementOneChunkWaitEndOfWritting: retry the download operation (Retry: %i)\n"), iRetry );
									m_eProcessStateEngine = PSE_UpgradeOneElementOneChunk;
								}
							}
							
							bContinue = false;
						}
						else
						{
							OUTPUT_DEBUG_STRING( _T("DFUCom::_ProcessRunning::PSE_UpgradeOneElementOneChunkWaitEndOfWritting: check the status of the waiting of the writing process...\n") );

							if( m_dst.bState != STATE_DFU_DOWNLOAD_IDLE && m_dst.bState != STATE_DFU_DOWNLOAD_BUSY && m_dst.bState != STATE_DFU_MANIFEST )
							{
								OUTPUT_DEBUG_STRING( _T("DFUCom::_ProcessRunning::PSE_UpgradeOneElementOneChunkWaitEndOfWritting: error while waiting the end of the writing process!\n") );

								m_eProcessStateEngine = _SetProcessError( PE_InternalError, PIE_DFUErrorWhenWritingChunkElement );
								bContinue = false;
							}
							else
							{
								OUTPUT_DEBUG_STRING( _T("DFUCom::_ProcessRunning::PSE_UpgradeOneElementOneChunkWaitEndOfWritting: no error while waiting the end of the writing process, continue to wait.\n") );

								m_PollTime = m_dst.bwPollTimeout;
								m_eProcessStateEngine = PSE_UpgradeOneElementOneChunkWaitEndOfWritting;
							}
						}
					}
				}
				else if( DFU_STATE_dfuDNLOAD_IDLE == m_dst.bState )
				{
					OUTPUT_DEBUG_STRING( _T("DFUCom::_ProcessRunning::PSE_UpgradeOneElementOneChunkWaitEndOfWritting: DFU state is downloading idle.\n") );
					OUTPUT_DEBUG_STRING( _T("DFUCom::_ProcessRunning::PSE_UpgradeOneElementOneChunkWaitEndOfWritting: go to the next chunk.\n") );
					m_eProcessStateEngine = PSE_UpgradeOneElementOneChunkFinished;
				}
			}
		}
		break;

		case PSE_UpgradeOneElementOneChunkFinished:
			
			m_uiCurrentPage++;
			m_iProgressPercent = min( (int)( ( (double)m_uiCurrentPage / (double)m_uiTotalChunksToWrite ) * 100.0 ), 99 );

			OUTPUT_DEBUG_STRING( _T("DFUCom::_ProcessRunning::PSE_UpgradeOneElementOneChunkFinished: chunk upgrade finished (status: %i).\n"), m_dst.bStatus );

			if( m_dst.bStatus != DFU_STATUS_OK )
			{
				OUTPUT_DEBUG_STRING( _T("DFUCom::_ProcessRunning::PSE_UpgradeOneElementOneChunkFinished: status is not 'DFU_STATUS_OK'.\n") );

				m_eProcessStateEngine = _SetProcessError( PE_InternalError, PIE_DFUErrorWhenWritingChunkElement );
				bContinue = false;
			}

			if( true == bContinue )
			{
				OUTPUT_DEBUG_STRING( _T("DFUCom::_ProcessRunning::PSE_UpgradeOneElementOneChunkFinished: go upgrade the next element.\n") );

				iRetry = 3;
				m_PollTime = 0;
				m_p += m_ifunc_dfu_transfer_size;
				m_eProcessStateEngine = PSE_UpgradeOneElementOneChunk;
			}

			break;

		case PSE_RunAllTargetsInFileFinished:
		{
			OUTPUT_DEBUG_STRING( _T("DFUCom::_ProcessRunning::PSE_RunAllTargetsInFileFinished: all targets has been upgraded.\n") );

			if( PC_DFUUpgrade == m_iOperationCode )
			{
				struct dfu_if *adif = m_prNewDFUSeVariables->dfu_root;

				// Free all the allocated segments.
				while( NULL != adif)
				{
					if( NULL != adif->mem_layout )
					{
						free_segment_list( adif->mem_layout );
						adif->mem_layout = NULL;
					}

					adif = adif->next;
				}
			}

			m_eProcessStateEngine = PSE_Finished;
		}

		case PSE_Finished:
			m_eProcessStateEngine = PSE_WaitingToBeKilled;
			m_pclStopThreadEvent->SetEvent();
			break;

		case PSE_UpdateError:
			m_eProcessStateEngine = PSE_WaitingToBeKilled;
			m_pclStopThreadEvent->SetEvent();
			break;

		case PSE_WaitingToBeKilled:
			break;
	}

	LeaveCriticalSection( &m_CriticalSection );
}

void CDpSVisioNewDFUCommunication::_StopThreadProcessRunning()
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
					OutputDebugString( _T("DFUCom::_StopThreadProcessRunning: timeout reached.\n") );
					ASSERT( 0 );
					bStop = true;
				}

				break;

			default:
				bStop = true;
				break;
		}
	}

	m_pProcessThread = NULL;
}

CDpSVisioNewDFUCommunication::_ProcessStateEngine CDpSVisioNewDFUCommunication::_SetProcessError( ProcessError eProcessError, DWORD dwInternalError, _ProcessStateEngine eProcessStateEngine )
{
	m_rProcessStatus.m_bProcessStatus = false;
	m_rProcessStatus.m_eProcessError = eProcessError;
	m_rProcessStatus.m_dwDFUInternalErrorCode = dwInternalError;

	return eProcessStateEngine;
}

unsigned int CDpSVisioNewDFUCommunication::_quad2uint( unsigned char *p )
{
	return (*p + (*(p + 1) << 8) + (*(p + 2) << 16) + (*(p + 3) << 24));
}

bool CDpSVisioNewDFUCommunication::_CheckNumberOfPages()
{
	m_uiTotalPagesToErase = 0;
	m_uiTotalChunksToWrite = 0;
	CString str = _T("");

	for( int iLoopTarget = 1; iLoopTarget <= m_iTargets; iLoopTarget++ )
	{
		int iret = dfuse_memcpy( m_targetprefix, &m_pdata, &m_iBytesRemaining, sizeof( m_targetprefix ) );

		if( iret != 0 )
		{
			str.Format( _T("DFUCom::_CheckNumberOfPages: error when calling the 'dfuse_memcpy' method (%i).\n"), iret );
			OutputDebugString( str );

			_SetProcessError( PE_InternalError, PIE_DFUSeMemcpyError );
			return false;
		}

		if( strncmp( (char *)m_targetprefix, "Target", 6 ) )
		{
			CStringA strA = (char *)m_targetprefix;
			strA.Format( "DFUCom::_CheckNumberOfPages: target prefix not correct (%s).\n", (LPCSTR)strA.Left( 6 ) );
			OutputDebugStringA( strA );

			_SetProcessError( PE_DFUFileNoValidTargetSignature, 0 );
			return false;
		}

		int iNbElements = _quad2uint( (unsigned char *)m_targetprefix + 270 );

		m_padif = m_prNewDFUSeVariables->dfu_root;
		int bAlternateSetting = m_targetprefix[6];

		while( NULL != m_padif )
		{
			if( bAlternateSetting == m_padif->altsetting )
			{
				str.Format( _T("DFUCom::_CheckNumberOfPages: setting Alternate Interface #%d ...\n"), m_padif->altsetting );
				OutputDebugString( str );

				m_padif->dev_handle = m_prNewDFUSeVariables->dfu_root->dev_handle;
				iret = libusb_set_interface_alt_setting( m_padif->dev_handle, m_padif->interface, m_padif->altsetting );

				if( iret < 0 )
				{
					str.Format( _T("DFUCom::_CheckNumberOfPages: cannot set alternate interface: %s.\n"), libusb_error_name( iret ) );
					OutputDebugString( str );

					_SetProcessError( PE_InternalError, PE_CantSetAlternateSetting );
					return false;
				}
				break;
			}
			m_padif = m_padif->next;
		}

		for( int iLoopElement = 1; iLoopElement <= iNbElements; iLoopElement++ )
		{
			iret = dfuse_memcpy( m_elementheader, &m_pdata, &m_iBytesRemaining, sizeof( m_elementheader ) );

			if( iret != 0 )
			{
				str.Format( _T("DFUCom::_CheckNumberOfPages: error when calling the 'dfuse_memcpy' method (%i).\n"), iret );
				OutputDebugString( str );

				_SetProcessError( PE_InternalError, PIE_DFUSeMemcpyError );
				return false;
			}

			m_dwElementAddress = _quad2uint( (unsigned char *)m_elementheader );
			m_dwElementSize = _quad2uint( (unsigned char *)m_elementheader + 4 );

			if( (int)m_dwElementSize > m_iBytesRemaining )
			{
				str.Format( _T("DFUCom::_CheckNumberOfPages: file too small for element size (%i < %i).\n"), m_iBytesRemaining, m_dwElementSize );
				OutputDebugString( str );

				_SetProcessError( PE_InternalError, PIE_FileTooSmallForElementSize );
				return false;
			}

			if( NULL == m_padif )
			{
				dfuse_memcpy( NULL, &m_pdata, &m_iBytesRemaining, m_dwElementSize );
				continue;
			}

			// Check at least that we can write to the last address.
			struct memsegment *segment = find_segment( m_padif->mem_layout, m_dwElementAddress + m_dwElementSize - 1 );

			if( !segment || !( segment->memtype & DFUSE_WRITEABLE ) )
			{
				OutputDebugString( _T("DFUCom::_CheckNumberOfPages: segment to process at the last address is not found.\n") );

				_SetProcessError( PE_DFUDeviceLastPageNotWritable, 0 );
				return false;
			}

			m_p = 0;

			while( m_p < (int)m_dwElementSize )
			{
				m_iChunkSize = m_ifunc_dfu_transfer_size;
				m_uiAddress = m_dwElementAddress + m_p;

				segment = find_segment( m_padif->mem_layout, m_uiAddress );

				if( !segment || !( segment->memtype & DFUSE_WRITEABLE ) ) 
				{
					if( !segment )
					{
						OutputDebugString( _T("DFUCom::_CheckNumberOfPages: segment not found.\n") );
					}
					else
					{
						OutputDebugString( _T("DFUCom::_CheckNumberOfPages: segment found but it is not writeable.\n") );
					}

					_SetProcessError( PE_InternalError, PIE_DFUCantSegmentNotErasable );
					return false;
				}

				m_iPageSize = segment->pagesize;

				// Check if this is the last chunk.
				if( m_p + m_iChunkSize > (int)m_dwElementSize )
				{
					m_iChunkSize = m_dwElementSize - m_p;
				}

				if( !( segment->memtype & DFUSE_ERASABLE ) )
				{
					OutputDebugString( _T("DFUCom::_CheckNumberOfPages: segment not erasable, continue with next chunk address.\n") );

					// Continue in the current element.
					m_p += m_ifunc_dfu_transfer_size;
					continue;
				}

				m_uiEraseAddress = m_uiAddress;
				m_uiLastErasedPage = 1;

				while( m_uiEraseAddress < m_uiAddress + m_iChunkSize )
				{
					if( ( m_uiEraseAddress & ~( m_iPageSize - 1 ) ) != m_uiLastErasedPage )
					{
						m_uiTotalPagesToErase++;
					}
					
					m_uiLastErasedPage = m_uiAddress & ~( m_iPageSize - 1 );
					m_uiEraseAddress += m_iPageSize;
				}

				if( ( ( m_uiAddress + m_iChunkSize - 1 ) & ~( m_iPageSize - 1 ) ) != m_uiLastErasedPage )
				{
					m_uiTotalPagesToErase++;
				}

				m_uiTotalChunksToWrite++;
				m_p += m_ifunc_dfu_transfer_size;
			}

			dfuse_memcpy( NULL, &m_pdata, &m_iBytesRemaining, m_dwElementSize );
		}
	}

	return true;
}