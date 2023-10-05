#include "stdafx.h"
#include "..\global.h"
#include "Packet.h"
#include "SerialPort.h"


//-----------------------------------------
CSerialPort::CSerialPort()
{
	m_pParent = NULL;
	m_StrError = _T("");
	m_dwComErr = 0;
	m_hCom = INVALID_HANDLE_VALUE;
	m_PortNumber = 0;
	m_nInputBufferSize = 0;
	m_nOutputBufferSize = 0;
	ZeroMemory( &m_dcb, sizeof( m_dcb ) );
	m_dwEventMask = 0;
	m_dwRxTimeOut = 0;
	ZeroMemory( &m_Ov, sizeof( m_Ov ) );
	m_count = 0;
	m_lBaudrate = 0;
	m_cParity = 0;
	m_iWordLength = 0;
	m_iStopBits = 0;
	m_hCloseCom = (HANDLE)0;
	ZeroMemory( &m_hArrayEvent, sizeof( m_hArrayEvent ) );
	m_bThreadExist = false;

	// FF: Variables for thread.
	ZeroMemory( &m_pHandle, sizeof( m_pHandle ) );
}

//-----------------------------------------
CSerialPort::~CSerialPort()
{
	// Destruct (to be sure)
	PortClose();
}

//-----------------------------------------
DWORD CSerialPort::GetError()
{
	LPVOID lpMsgBuf;
	DWORD dw;

	FormatMessage(
		FORMAT_MESSAGE_ALLOCATE_BUFFER |
		FORMAT_MESSAGE_FROM_SYSTEM |
		FORMAT_MESSAGE_IGNORE_INSERTS,
		NULL,
		(dw = GetLastError()),
		0, // Default language
		(LPTSTR) &lpMsgBuf,
		0,
		NULL
		);   
		
	m_StrError = CString((LPTSTR)lpMsgBuf); 
	  
	// Free the buffer.
	LocalFree(lpMsgBuf);
	return dw;
}

//-----------------------------------------
bool CSerialPort::PortOpen(int portnumber, long baudrate, char parity, int wordlength, int stopbits, CWnd *pParent)
{
	bool fRetValue = true;

	// If one port already opened...
	if(m_hCom != INVALID_HANDLE_VALUE)
		fRetValue = false;

	if( true == fRetValue )
	{
		m_nInputBufferSize = 1050;
		m_nOutputBufferSize = 1050;
	    
		memset(&m_Ov, 0, sizeof(m_Ov));
	    
		// Don't use CString class, this is not supported under Win98
		TCHAR sz[20];
		_stprintf_s(sz, SIZEOFINTCHAR(sz), _T("\\\\.\\COM%d"), portnumber); 
		m_hCom = CreateFile(sz,
							GENERIC_READ | GENERIC_WRITE,
							0,
							NULL,
							OPEN_EXISTING,
							FILE_FLAG_OVERLAPPED,
							NULL);
						
		if(m_hCom == INVALID_HANDLE_VALUE)
		{
#ifdef DEBUG
			DWORD ErrorCode = GetLastError();
			LPVOID lpMsgBuf;
			FormatMessage( 
				FORMAT_MESSAGE_ALLOCATE_BUFFER | 
				FORMAT_MESSAGE_FROM_SYSTEM | 
				FORMAT_MESSAGE_IGNORE_INSERTS,
				NULL,
				ErrorCode,
				MAKELANGID(LANG_NEUTRAL, SUBLANG_DEFAULT), // Default language
				(LPTSTR) &lpMsgBuf,
				0,
				NULL 
			);
			LocalFree( lpMsgBuf );
#endif
			fRetValue = false;
		}
	}

	if( true == fRetValue )
	{
		m_PortNumber = portnumber;
		SetupComm(m_hCom, m_nInputBufferSize, m_nOutputBufferSize);
		
		// Reset event
		::GetCommMask(m_hCom, &m_dwEventMask);
		::SetCommMask(m_hCom, 0);

		////////////////////////////////////////////////////////////////////////////////
		// Configure communication device 
		if(!GetCommState(m_hCom, &m_dcb))
			fRetValue = false;
	}
		
	if( true == fRetValue )
	{
		m_dcb.fBinary = 1;
		m_dcb.fParity = 0;
		m_dcb.fNull = 0;
		m_dcb.XonChar = XON;
		m_dcb.XoffChar = XOFF;
		m_dcb.XonLim = (WORD)((m_nInputBufferSize) / 4);
		m_dcb.XoffLim = (WORD)((m_nOutputBufferSize) / 4);
		m_dcb.EofChar = 0;
		m_dcb.EvtChar = 0;
		m_dcb.fOutxDsrFlow = 0;
		m_dcb.fOutxCtsFlow = 0;
		m_dcb.fDtrControl = DTR_CONTROL_ENABLE;
		m_dcb.fRtsControl = RTS_CONTROL_ENABLE;
		
		if(!SetCommState(m_hCom, &m_dcb))
			fRetValue = false;
	}

	if( true == fRetValue )
	{
		////////////////////////////////////////////////////////////////////////////////
		// Configure time-out parameters
		COMMTIMEOUTS comTimeOuts;
		GetCommTimeouts(m_hCom, &comTimeOuts);

		comTimeOuts.ReadIntervalTimeout = 3000;
		comTimeOuts.ReadTotalTimeoutMultiplier = 0;
		comTimeOuts.ReadTotalTimeoutConstant = 0;
		comTimeOuts.WriteTotalTimeoutMultiplier = 0;
		comTimeOuts.WriteTotalTimeoutConstant = 0;

		// m_dwRxTimeOut = comTimeOuts.ReadTotalTimeoutConstant;
		m_dwRxTimeOut = 5000;

		SetCommTimeouts(m_hCom, &comTimeOuts);
		////////////////////////////////////////////////////////////////////////////////

		if(!PortSet(baudrate, parity, wordlength, stopbits))
			fRetValue = false;
	}

	if( true == fRetValue )
	{
		// Begin thread
		if(!_StartThread(pParent))
		{
			PortClose();
			fRetValue = false;
		}
	}
	
	return fRetValue;
}

//-----------------------------------------
// FF: at now, we don't sent these values to serial com device for Rx.
//     The reason is because in "ReadBuffer" we are waiting at the entry 
//	   of this function until we have received character or delays is
//	   expired.
bool CSerialPort::SetTimeouts(DWORD dwRxTimeout, DWORD dwTxTimeout)
{
	if(m_hCom == INVALID_HANDLE_VALUE)
		return false;
	
	COMMTIMEOUTS commTimeOuts;
	
	// See remarks above.	
	GetCommTimeouts(m_hCom, &commTimeOuts);
	commTimeOuts.WriteTotalTimeoutConstant = dwTxTimeout;
	SetCommTimeouts(m_hCom, &commTimeOuts);
	
	m_dwRxTimeOut = dwRxTimeout;
	
	return true;
}

//-----------------------------------------
bool CSerialPort::GetTimeouts(DWORD &dwRxTimeout, DWORD &dwTxTimeout)
{
	if(m_hCom == INVALID_HANDLE_VALUE)
	{
		TRACE0("CSerialPort::SetTimeouts': NULL handle !");      
		return false;
	}
		
	// Same remarks as for 'SetTimeouts'
	COMMTIMEOUTS commTimeOuts;
	GetCommTimeouts(m_hCom, &commTimeOuts);
	dwTxTimeout = commTimeOuts.WriteTotalTimeoutConstant;
	
	dwRxTimeout = m_dwRxTimeOut;

	return true;
}

//-----------------------------------------
bool CSerialPort::PortSet(long baudrate, char parity, int wordlength, int stopbits)
{   
	int result;

	if(m_hCom == INVALID_HANDLE_VALUE)
		return false;
    
	m_lBaudrate=baudrate;
	m_cParity=parity;
	m_iWordLength=wordlength;
	m_iStopBits=stopbits;
    
	if(result = GetCommState(m_hCom,&m_dcb))
	{
		m_dcb.fRtsControl = RTS_CONTROL_ENABLE;
		m_dcb.fDtrControl = DTR_CONTROL_ENABLE;
		m_dcb.BaudRate = m_lBaudrate;
		m_dcb.Parity = m_cParity;
		m_dcb.ByteSize=m_iWordLength;
		m_dcb.StopBits=m_iStopBits;
		
		result = SetCommState(m_hCom, &m_dcb);   
	}

	if(result < 0)
		GetError();
		
	PurgeCom();
	return (result > 0);
}

//-----------------------------------------
bool CSerialPort::PortClose()
{
	bool fRetValue = true;

	if(m_hCom == INVALID_HANDLE_VALUE)
		fRetValue = false;
    
	if( true == fRetValue )
	{
		// If thread already exist...
		if(m_pHandle)
		{
			// If thread is running...
			if(m_pHandle->fRunning)
			{
				// Signal to thread to terminate and exit
				SetEvent(m_pHandle->hStopEvent);
				// Wait that thread is well finished
				WaitForSingleObject(m_pHandle->hThread, INFINITE);
			}
			_ReleaseHandle(m_pHandle);
		}
	    
		EscapeCommFunction(m_hCom, CLRDTR);
		EscapeCommFunction(m_hCom, CLRRTS);
	
		int status = CancelIo (m_hCom);
		status = CloseHandle(m_hCom);   
		m_hCom = INVALID_HANDLE_VALUE;
		m_PortNumber = -1;
		
		if(!status)
		{
			GetError();
			fRetValue = false;
		}
	}
		
	return fRetValue;
}

//-----------------------------------------
bool CSerialPort::WriteChar(const UINT8 bOneChar)
{
	UINT8 pbBuffer[5];
	pbBuffer[0] = bOneChar;
	return WriteBuffer(pbBuffer, 1);
}

//-----------------------------------------
bool CSerialPort::WriteBuffer(const UINT8 *pbBuffer, unsigned int ucount)
{
	OVERLAPPED ov;
	UINT8 const * pbTemp;
	DWORD dwWrite;
	bool fRetValue;
	BOOL fSuccess;

	if( m_hCom == INVALID_HANDLE_VALUE )
		return false;
	
	fRetValue = true;
	pbTemp = pbBuffer;
	ZeroMemory( &ov, sizeof( ov ) );
	ov.hEvent = CreateEvent( NULL, TRUE, FALSE, NULL );

	fSuccess = WriteFile( m_hCom, pbTemp, ucount, &dwWrite, &ov );
	if( FALSE == fSuccess )
	{
		// If error another than pending...
		if( ERROR_IO_PENDING != GetLastError() )
		{
			dwWrite = 0;
			fRetValue = false;
		}

		if( true == fRetValue )
		{
			// Wait that sending has finished.
			while( !GetOverlappedResult(m_hCom, &ov, &dwWrite, TRUE) )
			{
				if( ERROR_IO_INCOMPLETE != GetLastError() )
				{
					dwWrite = 0;
					fRetValue = false;
					break;
				}
			}
		}
	}

	return fRetValue;
}

// FF: new version
//-----------------------------------------
bool CSerialPort::ReadBuffer(UINT8 * const pbBuffer, UINT32 const dwToRead)
{
	bool fRetValue = true;

	if( NULL == m_pHandle )
		fRetValue = false;
	
	if( true == fRetValue )
	{
		if( 0 == m_pHandle->wBytesInBuffer )
		{
			// Wait if data becoming available after 'm_dwRxTimeOut' time
			DWORD dwReturn = WaitForSingleObject( m_pHandle->hDataReceivedEvent, m_dwRxTimeOut );
			
			if( WAIT_TIMEOUT == dwReturn || 0 == m_pHandle->wBytesInBuffer )
				fRetValue = false;
		}
	}
	
	if( true == fRetValue )
	{
 		EnterCriticalSection( &m_pHandle->cs );
		
		UINT16 wRead = 0;
		BYTE *pbTemp = pbBuffer;
		do 
		{
			*pbTemp	= *m_pHandle->pbBufferOut;

			m_pHandle->pbBufferOut++;
			if(m_pHandle->pbBufferOut == m_pHandle->pbBufferLimit)
				m_pHandle->pbBufferOut = m_pHandle->pbFIFO;

			pbTemp++;
			m_pHandle->wBytesInBuffer--;
			wRead++;
		}while( (wRead < dwToRead ) && ( m_pHandle->wBytesInBuffer > 0 ) );

		fRetValue = ( wRead == dwToRead );
		ResetEvent( m_pHandle->hDataReceivedEvent );

 		LeaveCriticalSection( &m_pHandle->cs );
	}

	return fRetValue;
}

//-----------------------------------------
bool CSerialPort::ReadChar(UINT8 &bChar)
{
	return ReadBuffer((BYTE *)&bChar, 1);
}

//-----------------------------------------
long CSerialPort::SizeUsedInRXBuf()
{
	if(m_hCom == INVALID_HANDLE_VALUE)
		return -1;
		
	COMSTAT comstat;
	DWORD comerr;

	ClearCommError(m_hCom, &comerr, &comstat);
	m_dwComErr |= comerr;
	
	return comstat.cbInQue;
}

long CSerialPort::SizeUsedInTXBuf()
{
	if(m_hCom == INVALID_HANDLE_VALUE)
		return -1;
		
	COMSTAT comstat;
	DWORD comerr;
	ClearCommError(m_hCom, &comerr, &comstat);
	m_dwComErr |= comerr;
	
	return comstat.cbOutQue;
}

//-----------------------------------------
bool CSerialPort::UseXonXoff(bool bEnable)
{
	if(m_hCom == INVALID_HANDLE_VALUE)
		return false;
		
	int result;

	m_dcb.fInX = (bEnable) ? 1 : 0;
	m_dcb.fOutX = (bEnable) ? 1 : 0;
	result = SetCommState(m_hCom, &m_dcb);
	if(result == TRUE)
		return true;
		
	GetError();
	return false;
}

//-----------------------------------------
bool CSerialPort::UseRtsCts(bool bEnable)
{
	if(m_hCom == INVALID_HANDLE_VALUE)
		return false;
		
	int result;

	m_dcb.fOutxCtsFlow = (bEnable) ? 1 : 0;
	m_dcb.fRtsControl = (bEnable) ? RTS_CONTROL_HANDSHAKE : RTS_CONTROL_DISABLE;
	result = SetCommState(m_hCom, &m_dcb);   
	if(result == TRUE)
		return true;
		
	GetError();
	return false;
}

//-----------------------------------------
bool CSerialPort::UseDtrDsr(bool bEnable)
{
	if(m_hCom == INVALID_HANDLE_VALUE)
		return false;
		
	int result;

	m_dcb.fOutxDsrFlow = (bEnable) ? 1 : 0;
	m_dcb.fDtrControl = (bEnable) ? DTR_CONTROL_HANDSHAKE : DTR_CONTROL_DISABLE;
	result = ::SetCommState(m_hCom, &m_dcb);
	if(result == TRUE)
		return true;   
		
	GetError();
	return false;
}

//-----------------------------------------
DWORD CSerialPort::GetCommMask()
{
	if(m_hCom == INVALID_HANDLE_VALUE)
	{
		TRACE0("CSerialPort::GetCommMask': INVALID handle !");      
		return 0;
	} 
	
	DWORD dwMask;
	::GetCommMask(m_hCom, &dwMask);
	return dwMask;
}

//-----------------------------------------
bool CSerialPort::SetCommMask(DWORD EvtMask)
{
	if(m_hCom == INVALID_HANDLE_VALUE)
		return false;
		
	if(!m_Ov.hEvent)
		m_Ov.hEvent = CreateEvent(	NULL,   // no security attributes
									FALSE,  // auto reset event
									FALSE,  // not signaled
									NULL    // no name
								);

	SetEvent(m_Ov.hEvent);

	return (::SetCommMask(m_hCom,EvtMask) > 0);
}

//-----------------------------------------
bool CSerialPort::PurgeRx()
{
	if(m_hCom == INVALID_HANDLE_VALUE)
	{
		TRACE0("CSerialPort::PurgeRx': NULL handle !");      
		return false;
	} 
	::PurgeComm(m_hCom, PURGE_RXCLEAR);
	return true;
}

//-----------------------------------------
bool CSerialPort::PurgeTx()
{
	if(m_hCom == INVALID_HANDLE_VALUE)
	{
		TRACE0("CSerialPort::PurgeTx': NULL handle !");      
		return false;
	} 
	::PurgeComm(m_hCom, PURGE_TXCLEAR);
	return true;
}

//-----------------------------------------
bool CSerialPort::PurgeCom()
{
	// BE carefull clean all windows message before to call it, otherwise Blue Windows crash !
	if(m_hCom == INVALID_HANDLE_VALUE)
	{
		TRACE0("CSerialPort::PurgeCom': NULL handle !");      
		return false;
	} 
	
	// flush the port
	::PurgeComm(m_hCom, PURGE_RXCLEAR | PURGE_TXCLEAR | PURGE_RXABORT | PURGE_TXABORT);
	return true;
}

//-----------------------------------------
bool CSerialPort::IsRXEmpty() 
{
	long n;

	n = SizeUsedInRXBuf();
	if(n < 0)
	{
		_OnError(GetError());	
		return true;
	}
	
	return (n==0);
}

//-----------------------------------------
bool CSerialPort::IsTXEmpty() 
{
	long n;

	n = SizeUsedInTXBuf();
	if(n < 0)
	{
		_OnError(GetError());	
		return true;
	}
	
	return (n==0);
}

////////////////////////////////////////////////////////////////////////////////////////
// PRIVATE MEMBERS
////////////////////////////////////////////////////////////////////////////////////////

//-----------------------------------------
bool CSerialPort::_StartThread(CWnd *pParent)
{
	bool fRetValue = true;
	
	if(m_pParent == NULL)
		m_pParent = pParent;
	
	if(!PurgeCom())
		fRetValue = false;
		
	if( true == fRetValue )
	{
		SetParentNotify(m_pParent);
		
		// If thread already exist...
		if(m_pHandle)
		{
			// If thread is running...
			if(m_pHandle->fRunning)
			{
				TerminateThread(m_pHandle->hThread, 0);
				WaitForSingleObject(m_pHandle->hThread, INFINITE);
			}
			_ReleaseHandle(m_pHandle);
		}
	
		m_pHandle = _GetNewHandle();
		if(m_pHandle == NULL)
			fRetValue = false;
	}

	if( true == fRetValue )
	{
		m_pHandle->hCom = m_hCom;
		
		// Initialize internal circular buffer
		m_pHandle->pbBufferIn = m_pHandle->pbFIFO;
		m_pHandle->pbBufferOut = m_pHandle->pbFIFO;
		m_pHandle->pbBufferLimit = &m_pHandle->pbFIFO[_SERIALCOM_FIFOBUFFER];
		m_pHandle->wBytesInBuffer = 0;
		
		// Set callback function
		m_pHandle->pfDataReceived = &CSerialPort::_DataReceivedHandler;
		
		// Create event to permits thread to signal when it receive data.
		m_pHandle->hDataReceivedEvent = CreateEvent(NULL, TRUE, FALSE, NULL);
		if(m_pHandle->hDataReceivedEvent == INVALID_HANDLE_VALUE)
			return false;
	}

	if( true == fRetValue )
	{
		// Create event to allow to check when thread is well started
		m_pHandle->hStartEvent = CreateEvent(NULL, TRUE, FALSE, NULL);
		// Create event to allow to stop thread.
		m_pHandle->hStopEvent = CreateEvent(NULL, TRUE, FALSE, NULL);
	    
		if((m_pHandle->hThread = CreateThread(NULL, 0, &CSerialPort::_ThreadProc, m_pHandle, CREATE_SUSPENDED, NULL)) == NULL)
			fRetValue = false;
	}

	if( true == fRetValue )
	{
		ResumeThread(m_pHandle->hThread);
		WaitForSingleObject( m_pHandle->hStartEvent, INFINITE );
		TRACE("Thread start: %ul\n", m_pHandle->hThread);
	}

	return fRetValue;
}

// FF: Reception is made by thread. Data received are saved in a circular buffer. It's to avoid
//     synchronisation problem especially when we are in debug mode.
//-----------------------------------------
DWORD WINAPI CSerialPort::_ThreadProc(LPVOID pParam)
{
	PSERIALCOM_HANDLE	pHandle = (PSERIALCOM_HANDLE)pParam;
	BYTE				pbByteRead[0x200];
	DWORD				dwBytesRead;
	HANDLE				waits[2];
	OVERLAPPED			ov;
	BOOL				fSuccess;
	BOOL				fCanStop;

	SetEvent( pHandle->hStartEvent );
	pHandle->fRunning = true;
	ZeroMemory( &ov, sizeof( ov ) );
	ov.hEvent = CreateEvent( NULL, FALSE, FALSE, NULL );

	waits[0] = pHandle->hStopEvent;
	waits[1] = ov.hEvent;

	fCanStop = FALSE;
	while( FALSE == fCanStop )
	{
		// Try to read data on serial port
		fSuccess = ReadFile( pHandle->hCom , pbByteRead, 1, &dwBytesRead, &ov );

		// If success...
		// PS: 'success' do not absolutely mean that there is data in serial port buffer
		if( TRUE == fSuccess )
		{
			// If there is data in serial port buffer...
			if( dwBytesRead > 0 )
			{
				// Check if there is enough space in circular buffer
				if(pHandle->wBytesInBuffer > _SERIALCOM_FIFOBUFFER)
					DebugBreak();

				// Forward received data
				pHandle->pfDataReceived( pHandle, pbByteRead, (DWORD)dwBytesRead );
			}

			// Check if stop has not been asked
			if( WaitForSingleObject( pHandle->hStopEvent, 0 ) == WAIT_OBJECT_0 )
			{
				// Cancel IO Request
				CancelIo( pHandle->hCom );
				fCanStop = TRUE;
			}
		}
		else
		{
			// Is IO Pending ?
			if( ERROR_IO_PENDING == GetLastError() )
			{
				DWORD dwRet;
				// Wait for IO or stop event.
				do 
				{
					dwRet = WaitForMultipleObjects( 2, waits, FALSE, 10 );
					switch( dwRet )
					{
						// Stop!
						case WAIT_OBJECT_0:
							// Cancel IO Request
							CancelIo( pHandle->hCom );

							// Exit thread
							fCanStop = TRUE;
							break;

						// Io completed
						case ( WAIT_OBJECT_0 + 1 ):
							if( GetOverlappedResult( pHandle->hCom, &ov, &dwBytesRead, FALSE ) )
							{
								// If there is data in serial port buffer...
								if( dwBytesRead > 0 )
								{
									// Forward received data
									pHandle->pfDataReceived( pHandle, pbByteRead, (DWORD)dwBytesRead );
								}
							}
							break;
					}
				} while ( WAIT_TIMEOUT == dwRet );
			}
			else
			{
				// Check if stop has been asked
 				if( WaitForSingleObject( pHandle->hStopEvent, 0 ) == WAIT_OBJECT_0 )
				{
					// Cancel IO Request
					CancelIo( pHandle->hCom );
 					fCanStop = TRUE;
				}
			}
		}
	}

	CloseHandle( ov.hEvent );
	pHandle->fRunning = false;
	return 0;
}

PSERIALCOM_HANDLE CSerialPort::_GetNewHandle()
{
    PSERIALCOM_HANDLE pHandle = NULL;

    pHandle = (PSERIALCOM_HANDLE)malloc(sizeof(SERIALCOM_HANDLE));
    if(pHandle)
    {
        ZeroMemory(pHandle, sizeof(SERIALCOM_HANDLE));
		// TODO test code Alen _GetNewHandle() InitializeCriticalSection() rounded with try/catch
        //Windows Server 2003 and Windows XP/2000:  In low memory situations, InitializeCriticalSection
		//can raise a STATUS_NO_MEMORY exception. This exception was eliminated starting with Windows Vista
		try
		{
			InitializeCriticalSection(&pHandle->cs);
		}
		catch(...)
		{
#ifdef DEBUG
			DWORD ErrorCode = GetLastError();
			LPVOID lpMsgBuf;
			FormatMessage( 
				FORMAT_MESSAGE_ALLOCATE_BUFFER | 
				FORMAT_MESSAGE_FROM_SYSTEM | 
				FORMAT_MESSAGE_IGNORE_INSERTS,
				NULL,
				ErrorCode,
				MAKELANGID(LANG_NEUTRAL, SUBLANG_DEFAULT), // Default language
				(LPTSTR) &lpMsgBuf,
				0,
				NULL 
			);
			LocalFree( lpMsgBuf );
#endif
			return NULL;
		}
	}
    return pHandle;
}

void CSerialPort::_ReleaseHandle(PSERIALCOM_HANDLE &pHandle)
{
	if(pHandle)
	{
		TRACE(_T("Release handle: %ul\n"),pHandle->hThread);
		if(m_pHandle->hThread != INVALID_HANDLE_VALUE)
		{
			CloseHandle(m_pHandle->hThread);
			m_pHandle->hThread = INVALID_HANDLE_VALUE;
		}
		if(m_pHandle->hStopEvent != INVALID_HANDLE_VALUE)
		{
			CloseHandle(m_pHandle->hStopEvent);
			m_pHandle->hStopEvent = INVALID_HANDLE_VALUE;
		}
		if(m_pHandle->hDataReceivedEvent != INVALID_HANDLE_VALUE)
		{
			CloseHandle(m_pHandle->hDataReceivedEvent);
			m_pHandle->hDataReceivedEvent = INVALID_HANDLE_VALUE;
		}
		DeleteCriticalSection( &pHandle->cs );
		free(pHandle);
		pHandle = NULL;
	}
}

void CSerialPort::_DataReceivedHandler(void* pParam, BYTE *pReadBuf, UINT32 dwBytesReceived)
{	 
	PSERIALCOM_HANDLE pHandle = (PSERIALCOM_HANDLE)pParam;
	if( dwBytesReceived )
	{
 		EnterCriticalSection( &pHandle->cs );

		BYTE *pbSrc = pReadBuf;
		PSERIALCOM_HANDLE hdlsave = pHandle;
		UINT32 dwMem = dwBytesReceived;
		while( dwBytesReceived-- )
		{
			if( pHandle->wBytesInBuffer < _SERIALCOM_FIFOBUFFER )
			{
				*pHandle->pbBufferIn = *pbSrc;

				pHandle->pbBufferIn++;
				if(pHandle->pbBufferIn == pHandle->pbBufferLimit)
					pHandle->pbBufferIn = pHandle->pbFIFO;

				pHandle->wBytesInBuffer++;
				pbSrc++;
			}
			else
			{
				DebugBreak();
				pHandle->pbBufferIn = pHandle->pbFIFO;
				pHandle->pbBufferOut = pHandle->pbFIFO;
				pHandle->pbBufferLimit = &pHandle->pbFIFO[_SERIALCOM_FIFOBUFFER];
				pHandle->wBytesInBuffer = 0;
			}
		}

		// Signal that datas are received and available
		SetEvent( pHandle->hDataReceivedEvent );
 		LeaveCriticalSection( &pHandle->cs );
	}
}

// Function called in case of error detected in reception or transmission
// Send a message to parent window.
//-----------------------------------------
void CSerialPort::_OnError(DWORD dwError)
{
	if(m_pParent)
		m_pParent->SendMessage(WM_CCOMERROR, (WPARAM)this, dwError);
}

//--------------------------------------------------
// Function used to enumerate all serial com ports

/*static*/BOOL CSerialPort::EnumerateComPortUsingRegistry(CStringArray& ports)
{
  // Make sure we clear out any elements which may already be in the array(s)
  ports.RemoveAll();

  //What will be the return value
  BOOL bSuccess = FALSE;

  HKEY hSERIALCOMM;
  if (RegOpenKeyEx(HKEY_LOCAL_MACHINE, _T("HARDWARE\\DEVICEMAP\\SERIALCOMM"), 0, KEY_QUERY_VALUE, &hSERIALCOMM) == ERROR_SUCCESS)
  {
		//Get the max value name and max value lengths
		DWORD dwMaxValueNameLen;
		DWORD dwMaxValueLen;
		DWORD dwQueryInfo = RegQueryInfoKey(hSERIALCOMM, NULL, NULL, NULL, NULL, NULL, NULL, NULL, &dwMaxValueNameLen, &dwMaxValueLen, NULL, NULL);
		if (dwQueryInfo == ERROR_SUCCESS)
		{
			DWORD dwMaxValueNameSizeInChars = dwMaxValueNameLen + 1; //Include space for the NULL terminator
			DWORD dwMaxValueNameSizeInBytes = dwMaxValueNameSizeInChars * sizeof(TCHAR);
			DWORD dwMaxValueDataSizeInChars = dwMaxValueLen/sizeof(TCHAR) + 1; //Include space for the NULL terminator
			DWORD dwMaxValueDataSizeInBytes = dwMaxValueDataSizeInChars * sizeof(TCHAR);
		
			//Allocate some space for the value name and value data			
      ATL::CHeapPtr<TCHAR> szValueName;
      ATL::CHeapPtr<BYTE> byValue;
      if (szValueName.Allocate(dwMaxValueNameSizeInChars) && byValue.Allocate(dwMaxValueDataSizeInBytes))
      {
				bSuccess = TRUE;

				//Enumerate all the values underneath HKEY_LOCAL_MACHINE\HARDWARE\DEVICEMAP\SERIALCOMM
				DWORD dwIndex = 0;
				DWORD dwType;
				DWORD dwValueNameSize = dwMaxValueNameSizeInChars;
				DWORD dwDataSize = dwMaxValueDataSizeInBytes;
				memset(szValueName.m_pData, 0, dwMaxValueNameSizeInBytes);
				memset(byValue.m_pData, 0, dwMaxValueDataSizeInBytes);
				LONG nEnum = RegEnumValue(hSERIALCOMM, dwIndex, szValueName, &dwValueNameSize, NULL, &dwType, byValue, &dwDataSize);
				while (nEnum == ERROR_SUCCESS)
				{
					// Keep only USBSER devices limited to TA-SCOPE device
					CString str = szValueName;
					if (str.Find(_T("USBSER"))>=0)
					{
						//If the value is of the correct type, then add it to the array
						if (dwType == REG_SZ)
						{
							TCHAR* szPort = reinterpret_cast<TCHAR*>(byValue.m_pData);
							ports.Add(szPort);	
						}
					}
					//Prepare for the next time around
					dwValueNameSize = dwMaxValueNameSizeInChars;
					dwDataSize = dwMaxValueDataSizeInBytes;
					memset(szValueName.m_pData, 0, dwMaxValueNameSizeInBytes);
					memset(byValue.m_pData, 0, dwMaxValueDataSizeInBytes);
					++dwIndex;
					nEnum = RegEnumValue(hSERIALCOMM, dwIndex, szValueName, &dwValueNameSize, NULL, &dwType, byValue, &dwDataSize);
				}
      }
      else
		    SetLastError(ERROR_OUTOFMEMORY);
		}
		
		//Close the registry key now that we are finished with it    
    RegCloseKey(hSERIALCOMM);
    
    if (dwQueryInfo != ERROR_SUCCESS)
			SetLastError(dwQueryInfo);
  }
	
	return bSuccess;
}
