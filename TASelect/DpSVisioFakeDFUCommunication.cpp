#include "stdafx.h"
#include "DpSVisioIncludes.h"

CDpSVisioFakeDFUCommunication::CDpSVisioFakeDFUCommunication( CWnd *pParent )
	: CDpSVisioBaseCommunication( pParent, CDpSVisioBaseCommunication::CT_DFU )
{
	m_pclDpSVisioCommand = &m_clDpSVisioDFUCommand;
	m_pclDpSVisioInformation = &m_clDpSVisioFakeDFUInformation;
	m_pclDpSVisioInformation->Reset();
	m_bPercent = 0;
}

CDpSVisioFakeDFUCommunication::~CDpSVisioFakeDFUCommunication()
{
	Reset();
}

void CDpSVisioFakeDFUCommunication::DpSVisioConnected( CString strDeviceName )
{
	m_clDpSVisioFakeDFUInformation.m_bIsConnected = true;

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

void CDpSVisioFakeDFUCommunication::DpSVisioDisconnected( CString strDeviceName )
{
	Reset();
}

int CDpSVisioFakeDFUCommunication::SendDpSVisioCommandLP( UINT16 uiCommand, LPARAM lpParam, CListBox *pOutputList )
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

		default:
			iReturn = DPVSISIOCOM_ERROR_DFU_SENDCMD_BADCOMMAND;
			break;
	}

	return iReturn;
}

BEGIN_MESSAGE_MAP( CDpSVisioFakeDFUCommunication, CWnd )
	ON_WM_TIMER()
END_MESSAGE_MAP()

void CDpSVisioFakeDFUCommunication::OnTimer( UINT_PTR nIDEvent )
{
	m_bPercent++;

	if( 100 == m_bPercent )
	{
		KillTimer( m_nTimer );
	}
}

void CDpSVisioFakeDFUCommunication::Reset()
{
	CDpSVisioBaseCommunication::Reset();
	
	BYTE *pBuffer = m_FirmwareToUpload.Detach();

	if( NULL != pBuffer )
	{
		free( pBuffer );
		pBuffer = NULL;
	}

	m_clDpSVisioFakeDFUInformation.Reset();
}

bool CDpSVisioFakeDFUCommunication::RetrieveDpSVisioInformation( CString strDeviceName )
{
	m_clDpSVisioFakeDFUInformation.m_strDeviceName = strDeviceName.MakeLower();
	return true;
}

int CDpSVisioFakeDFUCommunication::_DFUStartErase( CString *pstrDFUFileName, CListBox *pOutputList )
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

	CString strApplyUpdateMsg = TASApp.LoadLocalizedString( IDS_DLGDPSVISIO_FIRMWAREERROR_APPLYUPDATE );
	CString strErrorMsg;
	CFileStatus status;

	if( FALSE == CFile::GetStatus( *pstrDFUFileName, status ) )
	{
		strErrorMsg = _T("The firmware for the DpS-Visio was not found.\r\n\r\n") + strApplyUpdateMsg;
		INSERT_STRING_IN_PLIST_AND_SCROLL( pOutputList, strErrorMsg );
		return -1;
	}

	// Try to open it.
	CFile DFUFile;
	
	if( 0 == DFUFile.Open( *pstrDFUFileName, CFile::modeRead | CFile::typeBinary ) )
	{
		strErrorMsg = _T("Can't open the firmware of the DpS-Visio.\r\n\r\n") + strApplyUpdateMsg;
		INSERT_STRING_IN_PLIST_AND_SCROLL( pOutputList, strErrorMsg );
		return -1;
	}

	// Check the file length.
	UINT uiFileSize = (UINT)DFUFile.GetLength();

	if( 0 == uiFileSize )
	{
		strErrorMsg = _T("Firmware is empty.\r\n\r\n") + strApplyUpdateMsg;
		INSERT_STRING_IN_PLIST_AND_SCROLL( pOutputList, strErrorMsg );
		return -1;
	}

	BYTE *pBuffer = new BYTE[uiFileSize];

	if( NULL == pBuffer )
	{
		strErrorMsg = _T("Can't create the internal buffer to read the buffer.\r\n\r\n") + strApplyUpdateMsg;
		INSERT_STRING_IN_PLIST_AND_SCROLL( pOutputList, strErrorMsg );
		return -1;
	}

	try
	{
		if( uiFileSize != DFUFile.Read( pBuffer, uiFileSize ) )
		{
			DFUFile.Close();
			delete pBuffer;

			strErrorMsg = _T("Error when reading the binary file.\r\n\r\n") + strApplyUpdateMsg;
			INSERT_STRING_IN_PLIST_AND_SCROLL( pOutputList, strErrorMsg );
			return -1;
		}
	}
	catch( CFileException *pFileException )
	{
		DFUFile.Close();
		delete pBuffer;

		strErrorMsg = _GetExceptionString( pFileException->m_cause );
		INSERT_STRING_IN_PLIST_AND_SCROLL( pOutputList, strErrorMsg );
		pFileException->Delete();

		return -1;
	}
	
	m_FirmwareToUpload.Attach( pBuffer, uiFileSize );

	// Launch timer.
	m_bPercent = 0;
	m_nTimer = SetTimer( FAKEDFUTIMER, FAKEDFU_TIMERERASETIC, NULL );

	return DPSVISIOCOM_NOERROR;
}

int CDpSVisioFakeDFUCommunication::_DFUCheckErase( BYTE *pPercentCompleted, CListBox *pOutputList )
{
	if( NULL == pPercentCompleted )
	{
		return DPVSISIOCOM_ERROR_DFU_CHECKERASE_PERCENTPOINTERNULL;
	}
		
	*pPercentCompleted = m_bPercent;
	return DPSVISIOCOM_NOERROR;
}

int CDpSVisioFakeDFUCommunication::_DFUStartUpgrade( CListBox *pOutputList )
{
	// Launch timer.
	m_bPercent = 0;
	m_nTimer = SetTimer( FAKEDFUTIMER, FAKEDFU_TIMERPROGRAMTIC, NULL );

	return DPSVISIOCOM_NOERROR;
}

int CDpSVisioFakeDFUCommunication::_DFUCheckUpgrade( BYTE *pPercentCompleted, CListBox *pOutputList )
{
	if( NULL == pPercentCompleted )
	{
		return DPVSISIOCOM_ERROR_DFU_CHECKUPGRADE_PERCENTPOINTERNULL;
	}

	*pPercentCompleted = m_bPercent;
	return DPSVISIOCOM_NOERROR;
}

CString CDpSVisioFakeDFUCommunication::_GetExceptionString( UINT uiCode )
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
