#pragma once


class CDpSVisioDFUInformation : public CDpSVisioBaseInformation
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

class CDpSVisioDFUCommunication : public CDpSVisioBaseCommunication
{
public:
	CDpSVisioDFUCommunication( CWnd *pParent );
	virtual ~CDpSVisioDFUCommunication();

	// Overrides pure public virtual methods of 'CDpSVisioBaseCommunication'.
	virtual void DpSVisioConnected( CString strDeviceName );
	virtual void DpSVisioDisconnected( CString strDeviceName );

	virtual int ReadDpSVisioCommand( CListBox *pOutputList = NULL ) { return -1; }
	virtual int SendDpSVisioCommand( UINT16 uiCommand, UINT8 uiSize = 0, UINT8 *pData = NULL, CListBox *pOutputList = NULL ) { return -1; }
	virtual int SendDpSVisioCommandLP( UINT16 uiCommand, LPARAM lpParam = (LPARAM)0, CListBox *pOutputList = NULL );
	virtual int SendAndReadDpSVisioCommand( UINT16 uiCommand, UINT8 uiSize = 0, UINT8 *pData = NULL, CListBox *pOutputList = NULL ) { return -1; }

// Protected methods.
private:
	// Overrides pure protected virtual methods of 'CDpSVisioBaseCommunication'.
	virtual void Reset();
	virtual bool RetrieveDpSVisioInformation( CString strDeviceName );

// Private methods.
private:
	int _DFUStartErase( CString *pstrDFUFileName, CListBox *pOutputList );
	int _DFUCheckErase( BYTE *pPercentCompleted, CListBox *pOutputList );
	int _DFUStartUpgrade( CListBox *pOutputList );
	int _DFUCheckUpgrade( BYTE *pPercentCompleted, CListBox *pOutputList );
	int _DFUDetach( CListBox *pOutputList );

// Private variables.
private:
	CDpSVisioDFUCommand m_clDpSVisioDFUCommand;
	CDpSVisioDFUInformation m_clDpSVisioDFUInformation;
	HANDLE m_hDFUImage;
	PMAPPING m_pMapping;
	DWORD m_dwNbAlternates;
	DWORD m_dwOperationCode;
};
