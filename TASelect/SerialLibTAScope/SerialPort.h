#pragma once
#include "..\stdafx.h"

#define XON 17
#define XOFF 19

// messages à destination de la fenêtre de traitement.
#define WM_CCOMRCV WM_USER+100 // réceptions sur la voie série
#define WM_CCOMEVENT WM_USER+101 // evénements sur la voie série.
#define WM_CCOMERROR WM_USER+102 // erreurs sur la voie série. 

// FF: Set size of internal circular buffer
// If any change made in 'Packet.h', you MUST change these values
//	-> 1: Command
//  -> 2: Data length
//  -> 4096: corresponding to MAX_DATA_LENGTH
//  -> 1: number of padding bytes (at now it's 0 or 1 !!!)
//  -> 1: padding byte
//  -> 2: CRC
#define _SERIALCOM_FIFOBUFFER	( 1 + 2 + 4096 + 1 + 1 + 2 )

// FF: struct containing some variables for thread
typedef void (*pCallBack)(void* pParam, BYTE *pbReadBuf, UINT32 const dwBytesReceived);
typedef struct _Handle
{
	HANDLE				hCom;
	HANDLE				hThread;
	HANDLE				hStartEvent;
	HANDLE				hStopEvent;
	HANDLE				hDataReceivedEvent;
	CRITICAL_SECTION	cs;
	pCallBack			pfDataReceived;
	bool				fRunning;
	// Circular buffer
	BYTE				pbFIFO[_SERIALCOM_FIFOBUFFER];
	BYTE*				pbBufferIn;
	BYTE*				pbBufferOut;
	BYTE*				pbBufferLimit;
	UINT16				wBytesInBuffer;
}SERIALCOM_HANDLE, *PSERIALCOM_HANDLE;

class CSerialPort
{
public:
	CSerialPort();
	~CSerialPort();

	bool	PortOpen(int portnumber,long baudrate,char parity,int wordlength,int stopbits, CWnd *pParent);
	bool	PortSet(long baudrate,char parity,int wordlength,int stopbits);
	bool	PortClose();

	bool	WriteBuffer(const UINT8 *pbBuffer,unsigned int ucount = 0);
	bool	WriteChar(const UINT8 bOneChar);

	bool	ReadBuffer(UINT8 * const pbBuffer, UINT32 const dwToRead);
	bool	ReadChar(UINT8 &bChar);

	bool	UseXonXoff(bool bEnable=true);
	bool	UseRtsCts(bool bEnable=true);
	bool	UseDtrDsr(bool bEnable=true);

	long	SizeUsedInRXBuf();              
	long	SizeUsedInTXBuf();              

	bool	IsRXEmpty();
	bool	IsTXEmpty();

	bool	SetCommMask(DWORD EvtMask);
	DWORD	GetCommMask();

	CString	GetStringError(){return m_StrError;}
	int		GetCountRead(){return m_count;}
	void	SetParentNotify(CWnd *pParent){m_pParent=pParent;}

	bool	PurgeCom();
	bool	PurgeRx();
	bool	PurgeTx();

 	bool	SetTimeouts(DWORD dwRxTimeout = 5000,DWORD dwTxTimeout = 5000);
 	bool	GetTimeouts(DWORD &dwRxTimeout, DWORD &dwTxTimeout);

	HANDLE	GetComHandle(){return m_hCom;};
	int		GetPortNumber(){return m_PortNumber;};
	
	static BOOL	EnumerateComPortUsingRegistry(CStringArray& ports);
	
protected:
	DWORD  GetError();

private:
	// thread de communication.
	bool				_StartThread(CWnd *pParent);
	static DWORD WINAPI	_ThreadProc(LPVOID pParam);
	PSERIALCOM_HANDLE	_GetNewHandle();
	void				_ReleaseHandle(PSERIALCOM_HANDLE &pHandle);
	static void			_DataReceivedHandler(void* pParam, BYTE *pReadBuf, UINT32 dwBytesReceived);
	void				_OnError(DWORD dwError);

protected:
	CWnd				*m_pParent;
	CString				m_StrError;
	DWORD				m_dwComErr;
	HANDLE				m_hCom;
	int					m_PortNumber;
	UINT				m_nInputBufferSize;
	UINT				m_nOutputBufferSize;
	DCB					m_dcb;
	DWORD				m_dwEventMask;
	DWORD				m_dwRxTimeOut;
	OVERLAPPED			m_Ov;
	unsigned int		m_count;

	long				m_lBaudrate;
	char				m_cParity;
	int					m_iWordLength;
	int					m_iStopBits;
	HANDLE				m_hCloseCom;
	HANDLE				m_hArrayEvent[2];
	bool				m_bThreadExist;

	// FF: Variables for thread
	PSERIALCOM_HANDLE	m_pHandle;
};
