#pragma once


// Positive values are DFU errors coming from STM DFU library.
#define DPSVISIOCOM_NOERROR												0

#define DPSVISIOCOM_ERROR_HID_SENDCMD_HIDHANDLENULL						0xFFFFFFFF
#define DPSVISIOCOM_ERROR_HID_SENDCMD_FLAGCONNECTFALSE					0xFFFFFFFE
#define DPSVISIOCOM_ERROR_HID_SENDCMD_BADPAYLOADSIZE					0xFFFFFFFD
#define DPSVISIOCOM_ERROR_HID_SENDCMD_BADPAYLOADPOINTER					0xFFFFFFFC
#define DPSVISIOCOM_ERROR_HID_SENDCMD_HIDREPORTERROR					0xFFFFFFFB

#define DPSVISIOCOM_ERROR_HID_READCMD_HIDTIMEOUT						0xFFFFFFFA
#define DPSVISIOCOM_ERROR_HID_READCMD_HIDNACKRECEIVED					0xFFFFFFF9
#define DPVSISIOCOM_ERROR_DFU_SENDCMD_BADCOMMAND						0xFFFFFFF8

#define DPVSISIOCOM_ERROR_DFU_STARTERASE_DFUFILENAMEPOINTERNULL			0xFFFFFFF7
#define DPVSISIOCOM_ERROR_DFU_STARTERASE_DFUFILENAMEEMPTY				0xFFFFFFF6
#define DPVSISIOCOM_ERROR_DFU_STARTERASE_DFUVARIABLESEMPTY				0xFFFFFFF5
#define DPVSISIOCOM_ERROR_DFU_STARTERASE_CANTREADDFUFILE				0xFFFFFFF4
#define DPVSISIOCOM_ERROR_DFU_STARTERASE_CANTCHECKNUMBEROFPAGES			0xFFFFFFF3
#define DPVSISIOCOM_ERROR_DFU_STARTERASE_MEMORYLAYOUTERROR				0xFFFFFFF2
#define DPVSISIOCOM_ERROR_DFU_STARTERASE_TRANSFERSIZEUNKNOWN			0xFFFFFFF1
#define DPVSISIOCOM_ERROR_DFU_STARTERASE_CANTEXECUTECOMMAND				0xFFFFFFF0

#define DPVSISIOCOM_ERROR_DFU_CHECKERASE_PERCENTPOINTERNULL				0xFFFFFFE9
#define DPVSISIOCOM_ERROR_DFU_CHECKERASE_NOOPERATIONRUNNING				0xFFFFFFE8
#define DPVSISIOCOM_ERROR_DFU_CHECKERASE_ERRORGETSTATUS					0xFFFFFFE7
#define DPVSISIOCOM_ERROR_DFU_CHECKERASE_TIMEOUTREACHED					0xFFFFFFE6
#define DPVSISIOCOM_ERROR_DFU_CHECKERASE_CMDNOTCORRECTLYEXECUTED		0xFFFFFFE5

#define DPVSISIOCOM_ERROR_DFU_STARTUPGRADE_NOOPERATIONRUNNING			0xFFFFFFE4
#define DPVSISIOCOM_ERROR_DFU_STARTUPGRADE_CANTCREATESTOPEVENT			0xFFFFFFE3
#define DPVSISIOCOM_ERROR_DFU_STARTUPGRADE_CANTCREATESTOPPEDEVENT		0xFFFFFFE2

#define DPVSISIOCOM_ERROR_DFU_CHECKUPGRADE_PERCENTPOINTERNULL			0xFFFFFFE1
#define DPVSISIOCOM_ERROR_DFU_CHECKUPGRADE_NOOPERATIONRUNNING			0xFFFFFFE0
#define DPVSISIOCOM_ERROR_DFU_CHECKUPGRADE_CMDNOTCORRECTLYEXECUTED		0xFFFFFFDF

#define DPSVISIOCOM_ERROR_HID_CANTDETACH								0xFFFFFFDE
#define DPSVISIOCOM_ERROR_DFU_CANTATTACH								0xFFFFFFDD
#define DPSVISIOCOM_ERROR_DFU_CANTDETACH								0xFFFFFFDC
#define DPSVISIOCOM_ERROR_HID_CANTATTACH								0xFFFFFFDB

#define DPSVISIOCOM_ERROR_CANTCREATEDLGUNPLUPLUG						0xFFFFFFDA
#define DPSVISIOCOM_ERROR_UNPLUPLUGINTERRUPTEDBYUSER					0xFFFFFFD9

class CDpSVisioBaseInformation
{
public:
	virtual void Reset( void )
	{
		m_bIsConnected = false;
		m_strDeviceName = _T("Unknown");
		m_strIdentifier = _T( "" );
		m_strSerialNumber = _T( "" );
		m_strFirmwareRevision = _T( "" );
		m_strShortFirmwareRevision = _T( "" );
		m_dFirmwareVersion = 0.0;
	}

	bool m_bIsConnected;
	CString m_strDeviceName;
	CString m_strIdentifier;
	CString m_strSerialNumber;
	CString m_strFirmwareRevision;
	CString m_strShortFirmwareRevision;
	double m_dFirmwareVersion;
};

class CDpSVisioBaseCommunication
{
public:
	enum _CommunicationType
	{
		CT_HID,
		CT_DFU
	};

	CDpSVisioBaseCommunication( CWnd *pParent, _CommunicationType eCommunicationType );
	virtual ~CDpSVisioBaseCommunication() {}

	_CommunicationType GetCommunicationType( void ) { return m_eCommunicationType; }

	CDpSVisioBaseInformation *GetDpSVisioInformation() { return m_pclDpSVisioInformation; }
	CDpSVisioBaseCommand *GetDpSVisioCommand() { return m_pclDpSVisioCommand; }
	
	bool IsDpSVisioConnected( void );

	virtual void DpSVisioConnected( CString strDeviceName ) = 0;
	virtual void DpSVisioDisconnected( CString strDeviceName ) = 0;

	virtual int ReadDpSVisioCommand( CListBox *pOutputList = NULL ) = 0;
	virtual int SendDpSVisioCommand( UINT16 uiCommand, UINT8 uiSize = 0, UINT8 *pData = NULL, CListBox *pOutputList = NULL ) = 0;
	virtual int SendDpSVisioCommandLP( UINT16 uiCommand, LPARAM lpParam = (LPARAM)0, CListBox *pOutputList = NULL ) = 0;
	virtual int SendAndReadDpSVisioCommand( UINT16 uiCommand, UINT8 uiSize = 0, UINT8 *pData = NULL, CListBox *pOutputList = NULL ) = 0;

// Protected methods.
protected:
	virtual void Reset() {}
	virtual bool RetrieveDpSVisioInformation( CString strDeviceName ) = 0;

// Protected variables.
protected:
	_CommunicationType m_eCommunicationType;
	CWnd *m_pParent;
	CRITICAL_SECTION m_CriticalSection;
	CDpSVisioBaseCommand *m_pclDpSVisioCommand;
	CDpSVisioBaseInformation *m_pclDpSVisioInformation;
};
