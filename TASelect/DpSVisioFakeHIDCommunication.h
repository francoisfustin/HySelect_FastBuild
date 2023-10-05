#pragma once


class CDpSVisioBaseInformation;
class CDpSVisioFakeHIDInformation : public CDpSVisioBaseInformation
{
public:
	virtual void Reset( void )
	{
		CDpSVisioBaseInformation::Reset();

		m_bIsConnected = false;
		m_strDeviceName = _T("Unknown");
		m_strIdentifier = _T("Unknown");
		m_strSerialNumber = _T("Unknown");
		m_strFirmwareRevision = _T("Unknown");
		m_strShortFirmwareRevision = _T( "" );
		m_dFirmwareVersion = 0.0;
	}
};

class CDpSVisioBaseCommunication;
class CDpSVisioFakeHIDCommunication : public CDpSVisioBaseCommunication
{
public:
	CDpSVisioFakeHIDCommunication( CWnd *pParent );
	virtual ~CDpSVisioFakeHIDCommunication();

	// Overrides pure public virtual methods of 'CDpSVisioBaseCommunication'.
	virtual void DpSVisioConnected( CString strDeviceName );
	virtual void DpSVisioDisconnected( CString strDeviceName );
	
	virtual int ReadDpSVisioCommand( CListBox *pOutputList = NULL );
	virtual int SendDpSVisioCommand( UINT16 uiCommand, UINT8 uiSize = 0, UINT8 *pData = NULL, CListBox *pOutputList = NULL );
	virtual int SendDpSVisioCommandLP( UINT16 uiCommand, LPARAM lpParam = (LPARAM)0, CListBox *pOutputList = NULL ) { return -1; }
	virtual int SendAndReadDpSVisioCommand( UINT16 uiCommand, UINT8 uiSize = 0, UINT8 *pData = NULL, CListBox *pOutputList = NULL );

// Protected methods.
private:
	// Overrides pure protected virtual methods of 'CDpSVisioBaseCommunication'.
	virtual void Reset();
	virtual bool RetrieveDpSVisioInformation( CString strDeviceName );

// Private variables.
private:
	CDpSVisioHIDCommand m_clDpSVisioHIDCommand;
	CDpSVisioFakeHIDInformation m_clDpSVisioFakeInformation;

	UINT8 m_TxBuffer[USBD_CUSTOMHID_OUTREPORT_SENDSIZE];

	UINT8 m_uiBytesInRxBuffer;
	UINT8 m_RxBuffer[USBD_CUSTOMHID_OUTREPORT_BUF_SIZE];
	CMemFile m_clFirmware;
};
