#include "stdafx.h"
#include "SerialCom.h"
#include "Global.h"

CCom::CCom()
{
	m_pParent = NULL;
	m_StrError = _T("");
	m_dwComErr = 0;
	m_hCom = (HANDLE)0;
	m_iPortNumber = 0;
	m_uiInputBufferSize = 0;
	m_uiOutputBufferSize = 0;
	ZeroMemory( &m_dcb, sizeof( m_dcb ) );
	m_dwEventMask = 0;
	ZeroMemory( &m_ComTimeouts, sizeof( m_ComTimeouts ) );
	ZeroMemory( &m_Ov, sizeof( m_Ov ) );
	m_uiCount = 0;
	m_lBaudRate = 0;
	m_cParity = 0;
	m_iWordLength = 0;
	m_iStopBits = 0;
	m_pThread = NULL;
	m_hCloseCom = (HANDLE)0;
	ZeroMemory( &m_hArrayEvent, sizeof( m_hArrayEvent ) );
	m_bThreadExist = false;
}
//-----------------------------------------
CCom::~CCom()
{
    // Destruct    
    PortClose( );
}
//-----------------------------------------
DWORD CCom::GetError()
{
    LPVOID lpMsgBuf;
    DWORD dw;
    
    FormatMessage(
        FORMAT_MESSAGE_ALLOCATE_BUFFER |
        FORMAT_MESSAGE_FROM_SYSTEM |
        FORMAT_MESSAGE_IGNORE_INSERTS,
        NULL,
        (dw=GetLastError()),
        0, // Default language
        (LPTSTR) &lpMsgBuf,
        0,
        NULL
        );   
    m_StrError=CString((LPTSTR)lpMsgBuf);   
    // Free the buffer.
    LocalFree( lpMsgBuf );
    return dw;
}
//-----------------------------------------
bool CCom::PortOpen(int portnumber,long baudrate,char parity,int wordlength,int stopbits)
{
    if(m_hCom != INVALID_HANDLE_VALUE)
	{
		return false;
	}
	//
    m_uiInputBufferSize=1050;
    m_uiOutputBufferSize=1050;
    
    memset(&m_Ov,0,sizeof(m_Ov));
    
	// Don't use CString class, this is not supported under Win98
	TCHAR sz[20];
    _stprintf_s(sz,SIZEOFINTCHAR(sz),_T("\\\\.\\COM%d"), portnumber); 
	m_hCom = CreateFile( sz,
        GENERIC_READ | GENERIC_WRITE,
        0,
        NULL,
        OPEN_EXISTING,
        FILE_FLAG_OVERLAPPED,
        NULL );
    if ( m_hCom == INVALID_HANDLE_VALUE )
    {
        DWORD dwError = GetError();
		//CString TxtFile=CString(_T("Com.txt"));
		//CFileTxt f;
		//f.Open(TxtFile,CFile::modeCreate|CFile::modeWrite|CFile::modeNoTruncate);
		//f.SeekToEnd();
		//CString err; err.Format(_T(" %d: "), dwError);
		//CString str = CString(_T("Invalid handle ")) + sz  + err +GetStringError()+ CString(_T("\n"));
		//f.WriteTxtLine(str);
		//f.Close();
        return false;
    }   
    m_iPortNumber = portnumber;
    SetupComm( m_hCom, m_uiInputBufferSize, m_uiOutputBufferSize );
    ::GetCommMask( m_hCom, &m_dwEventMask );
    ::SetCommMask( m_hCom, 0 );
    
    m_dcb.fBinary = 1;
    m_dcb.fParity = 0;
    m_dcb.fNull = 0;
    m_dcb.XonChar = XON;
    m_dcb.XoffChar = XOFF;
    m_dcb.XonLim = (WORD)( ( m_uiInputBufferSize) / 4 );
    m_dcb.XoffLim = (WORD)( ( m_uiOutputBufferSize ) / 4 );
    m_dcb.EofChar = 0;
    m_dcb.EvtChar = 0;
    m_dcb.fOutxDsrFlow = 0;
    m_dcb.fOutxCtsFlow = 0;
    m_dcb.fDtrControl = DTR_CONTROL_ENABLE;
    m_dcb.fRtsControl = RTS_CONTROL_ENABLE;
    
    GetCommState( m_hCom, &m_dcb );
        
    GetCommTimeouts( m_hCom, &m_ComTimeouts );
        
    m_ComTimeouts.ReadIntervalTimeout = 1000;
    m_ComTimeouts.ReadTotalTimeoutMultiplier = 1000;
    m_ComTimeouts.ReadTotalTimeoutConstant = 1000;
    m_ComTimeouts.WriteTotalTimeoutMultiplier = 1000;
    m_ComTimeouts.WriteTotalTimeoutConstant = 1000;
    
    SetCommTimeouts( m_hCom, &m_ComTimeouts );
  
	if (PortSet(baudrate,parity,wordlength,stopbits))
		return true;
	// PortSetting fails, close PortCom and return false
	PortClose();
	return false;
}
//-----------------------------------------
bool CCom::SetTimeouts(DWORD dwRxTimeout /*=5000*/,DWORD dwTxTimeout /*=5000*/)
{
	// 
    if(m_hCom == INVALID_HANDLE_VALUE)
    {
        TRACE0("CCom::SetTimeouts': NULL handle !");      
        return 0;
    } 
    COMMTIMEOUTS  commTimeOuts ;
    commTimeOuts.ReadIntervalTimeout = dwRxTimeout;
    commTimeOuts.ReadTotalTimeoutMultiplier = 1;
    commTimeOuts.ReadTotalTimeoutConstant = dwRxTimeout;
    commTimeOuts.WriteTotalTimeoutMultiplier = 1;
    commTimeOuts.WriteTotalTimeoutConstant = dwTxTimeout;
    
    SetCommTimeouts(m_hCom, &commTimeOuts ) ;
    return true;
}
//-----------------------------------------
bool CCom::PortSet(long baudrate,char parity,int wordlength,int stopbits )
{   
    int result;
    
    if(m_hCom==INVALID_HANDLE_VALUE) return false;
    
    m_lBaudRate=baudrate;
    m_cParity=parity;
    m_iWordLength=wordlength;
    m_iStopBits=stopbits;
    
    //char *szBaud = new char[50];
    //sprintf(szBaud, "baud=%d parity=%c data=%d stop=%d", baudrate,parity,wordlength,stopbits);
	if (result=GetCommState(m_hCom,&m_dcb))
    {
        m_dcb.fRtsControl = RTS_CONTROL_ENABLE;
		m_dcb.fDtrControl = DTR_CONTROL_ENABLE;
        m_dcb.BaudRate = m_lBaudRate;
		m_dcb.Parity = m_cParity;
		m_dcb.ByteSize=m_iWordLength;
		m_dcb.StopBits=m_iStopBits;
		/*if (BuildCommDCB(szBaud, &m_dcb))*/ 
		result=SetCommState(m_hCom, &m_dcb);   
    }
    if ( result < 0 )  GetError();
    //delete szBaud;	
    PurgeCom();
    return( (result >0) );
}
//-----------------------------------------
bool CCom::PortClose()
{
    //
    if(m_hCom==INVALID_HANDLE_VALUE) return false;
    
    if(m_pThread) // fermeture thread
    {
        do
        {
            SetEvent(m_hCloseCom);
        }
        while(m_bThreadExist);
        CloseHandle(m_hCloseCom);
    }
    if(m_Ov.hEvent) CloseHandle(m_Ov.hEvent);
    m_Ov.hEvent=NULL;
    m_hCloseCom=NULL;
    m_pThread=NULL;
    
    EscapeCommFunction( m_hCom, CLRDTR );
    EscapeCommFunction( m_hCom, CLRRTS );
    int status = CancelIo (m_hCom);
    status= CloseHandle(m_hCom);   
    m_hCom=INVALID_HANDLE_VALUE;
	m_iPortNumber = -1;
    if ( status ) return true;
    GetError();
    return false;
}
//-----------------------------------------
bool CCom::WriteChar(const char onechar)
{
	char buf[5];
	buf[0] = onechar;
	return WriteBuffer(buf,1);
}
//-----------------------------------------
bool CCom::WriteBuffer(const char *buffer,unsigned int ucount /*=0*/)
{
	if (m_hCom==INVALID_HANDLE_VALUE) return false;
	
	int result;
    DWORD comerr;
    COMSTAT comstat;
    unsigned int amounttowrite;
    DWORD amountwritten;
    
    if(!ucount) ucount=(UINT)strlen(buffer);
    
    ClearCommError( m_hCom, &comerr, &comstat );
    m_dwComErr |= comerr;
    amounttowrite = m_uiOutputBufferSize - comstat.cbOutQue;
    if ( ucount < amounttowrite ) amounttowrite = ucount;
    
    result = WriteFile( m_hCom,
        buffer,
        (int) amounttowrite,
        &amountwritten,
        &m_Ov );
    
    m_uiCount = amountwritten;
    if ( result == 0 )
    {
        if ( ( comerr = GetLastError() ) != ERROR_IO_PENDING )
        {
            ClearCommError( m_hCom, &comerr, &comstat );
            m_dwComErr|= comerr;
        }
        else m_uiCount = amounttowrite;
    }
    if ( m_uiCount < ucount ) return(0);
    return( 1 ); // succes
}
//-----------------------------------------
int CCom::ReadBuffer(char *buffer,unsigned int ucount)
{
	if (m_hCom==INVALID_HANDLE_VALUE) return -1;
	int result;
    COMSTAT comstat;
    DWORD comerr;
    DWORD countread;
    DWORD counttoread;
    
    ClearCommError( m_hCom, &comerr, &comstat );
    if ( comerr > 0 ) m_dwComErr |= comerr;
    
    if ( comstat.cbInQue > 0 )
    {
        if ( comstat.cbInQue < ucount ) counttoread = comstat.cbInQue;
        else                        counttoread = ucount;
        result = ReadFile( m_hCom,buffer,(int) counttoread,&countread,&m_Ov );
        m_uiCount = countread;
        if ( result == 0 )
        {
            if ( ( comerr = GetLastError() ) != ERROR_IO_PENDING )
            {
                ClearCommError( m_hCom, &comerr, &comstat );
                m_dwComErr |= comerr;
            }
        }
        if ( m_uiCount < ucount )
        {
            if ( GetOverlappedResult( m_hCom, &m_Ov, &countread, TRUE ) )
            {
                m_uiCount = countread;
                return ( 1 );
            }
            return ( - 1);
        }
        return( 1 );
    }
    else
    {
        m_uiCount = 0;
        return( -1 );
    }
    return -1;
}
//-----------------------------------------
bool CCom::ReadChar(char &rchar )
{    
	return((ReadBuffer(&rchar,1)==1));
}
//-----------------------------------------
long CCom::SizeUsedInRXBuf()
{
	if (m_hCom==INVALID_HANDLE_VALUE) return -1;
	COMSTAT comstat;
    DWORD comerr;
    
    ClearCommError( m_hCom, &comerr, &comstat );
    m_dwComErr |= comerr;
    return comstat.cbInQue;
}
long CCom::SizeUsedInTXBuf()
{
	if (m_hCom==INVALID_HANDLE_VALUE) return -1;
	COMSTAT comstat;
    DWORD comerr;
    ClearCommError( m_hCom, &comerr, &comstat );
    m_dwComErr |= comerr;
    return comstat.cbOutQue;
}

//-----------------------------------------
bool CCom::UseXonXoff(bool bEnable)
{
	if (m_hCom==INVALID_HANDLE_VALUE) return false;
    int result;
    
    m_dcb.fInX = ( bEnable ) ? 1 : 0;
    m_dcb.fOutX = ( bEnable ) ? 1 : 0;
    result= SetCommState( m_hCom, &m_dcb );
    if ( result == TRUE ) return true;
    GetError();
    return false;
}
//-----------------------------------------
bool CCom::UseRtsCts(bool bEnable)
{
	if (m_hCom==INVALID_HANDLE_VALUE) return false;
    int result;
    
    m_dcb.fOutxCtsFlow = ( bEnable ) ? 1 : 0;
    m_dcb.fRtsControl = ( bEnable) ? RTS_CONTROL_HANDSHAKE : RTS_CONTROL_DISABLE;
    result= SetCommState( m_hCom, &m_dcb );   
    if ( result == TRUE ) return true;
    GetError();
    return false;
}
//-----------------------------------------
bool CCom::UseDtrDsr(bool bEnable)
{
	if (m_hCom==INVALID_HANDLE_VALUE) return false;
    int result;
    
    m_dcb.fOutxDsrFlow = ( bEnable ) ? 1 : 0;
    m_dcb.fDtrControl = ( bEnable ) ? DTR_CONTROL_HANDSHAKE : DTR_CONTROL_DISABLE;
    result= ::SetCommState( m_hCom, &m_dcb );
    if ( result == TRUE ) return true;   
    GetError();
    return false;
}
//-----------------------------------------
bool CCom::WaitCommEvent(DWORD &rEvtMask)
{
	if (m_hCom==INVALID_HANDLE_VALUE)
    {
        TRACE0("CCom::WaitCommEvent': INVALID_HANDLE_VALUE !");
        ASSERT(FALSE);
        return false;
    }   
    
    if(!::WaitCommEvent(m_hCom,&rEvtMask,&m_Ov))
    {      
        TRACE1("\nCCom::WaitCommEvent:%d",GetLastError());
        GetError();
        return false;
    }
    return true;
}
//-----------------------------------------
DWORD CCom::GetCommMask()
{
	if (m_hCom==INVALID_HANDLE_VALUE)
    {
        TRACE0("CCom::GetCommMask': INVALID handle !");      
        return 0;
    } 
    DWORD dwMask;
    ::GetCommMask( m_hCom, &dwMask);
    return dwMask;
}
//-----------------------------------------
bool CCom::SetCommMask(DWORD EvtMask)
{
	if (m_hCom==INVALID_HANDLE_VALUE) return false;
    if(!m_Ov.hEvent)
        m_Ov.hEvent=CreateEvent(NULL,   // no security attributes
                                FALSE,  // auto reset event
                                FALSE,  // not signaled
                                NULL    // no name
                                );
    
    SetEvent(m_Ov.hEvent);
    
    return (::SetCommMask(m_hCom,EvtMask)>0);

}
//-----------------------------------------
bool CCom::PurgeRx()
{
	if (m_hCom==INVALID_HANDLE_VALUE)
    {
        TRACE0("CCom::PurgeRx': NULL handle !");      
        return false;
    } 
    ::PurgeComm(m_hCom, PURGE_RXCLEAR);
    return true;
}
//-----------------------------------------
bool CCom::PurgeTx()
{
	if (m_hCom==INVALID_HANDLE_VALUE)
    {
        TRACE0("CCom::PurgeTx': NULL handle !");      
        return false;
    } 
    ::PurgeComm(m_hCom, PURGE_TXCLEAR);
    return true;
}
//-----------------------------------------
bool CCom::PurgeCom()
{
	// BE carefull clean all windows message before to call it, otherwise Blue Windows crash !
	if (m_hCom==INVALID_HANDLE_VALUE)
    {
        TRACE0("CCom::PurgeCom': NULL handle !");      
        return false;
    } 
    // flush the port
    ::PurgeComm(m_hCom, PURGE_RXCLEAR | PURGE_TXCLEAR | PURGE_RXABORT | PURGE_TXABORT);
    return true;
}
//-----------------------------------------
bool CCom::StartThread(CWnd *pParent)
{
    // 
    SetParentNotify(pParent);
    
    if(!m_pParent || !PurgeCom()) return false;
    if(m_bThreadExist) // femeture thread .
    {
        do
        {
            SetEvent(m_hCloseCom);
        }
        while (m_bThreadExist);
        CloseHandle(m_hCloseCom);
    }
    m_hCloseCom =CreateEvent(NULL, TRUE, FALSE, NULL);
    m_hArrayEvent[0]=m_hCloseCom;
    m_hArrayEvent[1]=m_Ov.hEvent;
    
    if (!(m_pThread = AfxBeginThread(Thread, this))) return false;
    
    TRACE("Le Thread Démarre\n");
    return true;
}
//-----------------------------------------
bool CCom::IsRXEmpty() 
{
    long n;
    
    n = SizeUsedInRXBuf();
    if ( n < 0 )
    {
        OnError(GetError());	
        return true;
    }
    return(( n==0));
}
//-----------------------------------------
bool CCom::IsTXEmpty() 
{
    long n;
    
    n = SizeUsedInTXBuf();
    if ( n < 0 )
    {
        OnError(GetError());	
        return true;
    }
    return(( n==0));
}

//-----------------------------------------
bool CCom::ResumeThread()
{
    if(!m_pThread) return false;
    m_pThread->ResumeThread();
    return true;
}
//-----------------------------------------
bool CCom::StopThread()
{
    if(!m_pThread) return false;
    m_pThread->SuspendThread(); 
    return true;
}
//-----------------------------------------
/*virtual*/ void CCom::OnError(DWORD dwError)
{
	// 
    if(m_pParent)
        m_pParent->SendMessage(WM_CCOMERROR,(WPARAM)this,dwError);
}
//-----------------------------------------
/*static*/ UINT CCom::Thread(LPVOID pParam)
{
	// 
    CCom *pCom = (CCom*)pParam;
    int nResult;
    DWORD WaitEvent = 0;
    DWORD dwError=0;
    DWORD dwMaskEvent=0;
    
    pCom->m_bThreadExist=true; // amorce l'existance du thread
    while(1)
    {
        nResult=pCom->WaitCommEvent(dwMaskEvent);
        if (!nResult)  
        { 			
            switch(dwError=GetLastError()) 
            {             
            case 87:
            case ERROR_IO_PENDING:
                break;
                
            default:
                {
                    // erreur de communication on fait suivre ...
                    pCom->GetError();
                    pCom->OnError(dwError);					                    
                    break;
                }
            }
        }
        else
        {			
            if(pCom->IsRXEmpty()) continue;
        }
        // attente evenement pour sortie eventuelle du thread ou reception
        WaitEvent = WaitForMultipleObjects(2, pCom->m_hArrayEvent, FALSE, INFINITE);		
        
        switch (WaitEvent)
        {
        case 0: //Fermeture Thread			 								
            pCom->m_bThreadExist=false;
            AfxEndThread(1);				
            return(0);				
            
        case 1:dwMaskEvent=pCom->GetCommMask();
            if(dwMaskEvent & EV_RXCHAR) // reception sur la voie.
                pCom->m_pParent->SendMessage(WM_CCOMRCV,(WPARAM)pCom,dwMaskEvent);
            
            if ((dwMaskEvent & EV_CTS) ||  // evenements divers.
                (dwMaskEvent & EV_RXFLAG) ||
                (dwMaskEvent & EV_BREAK) ||
                (dwMaskEvent & EV_ERR)
                || (dwMaskEvent & EV_RING))
            {
                pCom->m_pParent->SendMessage(WM_CCOMEVENT,(WPARAM)pCom,dwMaskEvent);
            }
            break;				
        }
    };
    return 0;
}

//--------------------------------------------------
// Function used to enumerate all serial com ports

/*static*/BOOL CCom::EnumerateComPortUsingRegistry(CStringArray& ports)
{
  //Make sure we clear out any elements which may already be in the array(s)
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
					str.MakeLower();	
					if (str.Find(_T("device\\bt"))<0)
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
