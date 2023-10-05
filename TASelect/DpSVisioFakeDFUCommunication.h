#pragma once


#define FAKEDFUTIMER				0x3000
#define FAKEDFU_TIMERSECTOERASE		2
#define FAKEDFU_TIMERERASETIC		( FAKEDFU_TIMERSECTOERASE * 10 )
#define FAKEDFU_TIMERSECTOPROGRAM	4
#define FAKEDFU_TIMERPROGRAMTIC		( FAKEDFU_TIMERSECTOPROGRAM * 10 )

class CDpSVisioFakeDFUInformation : public CDpSVisioBaseInformation
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

class CDpSVisioFakeDFUCommunication : public CWnd, public CDpSVisioBaseCommunication
{
public:
	CDpSVisioFakeDFUCommunication( CWnd *pParent );
	virtual ~CDpSVisioFakeDFUCommunication();

	// Overrides pure public virtual methods of 'CDpSVisioBaseCommunication'.
	virtual void DpSVisioConnected( CString strDeviceName );
	virtual void DpSVisioDisconnected( CString strDeviceName );

	virtual int ReadDpSVisioCommand( CListBox *pOutputList = NULL ) { return -1; }
	virtual int SendDpSVisioCommand( UINT16 uiCommand, UINT8 uiSize = 0, UINT8 *pData = NULL, CListBox *pOutputList = NULL ) { return -1; }
	virtual int SendDpSVisioCommandLP( UINT16 uiCommand, LPARAM lpParam = (LPARAM)0, CListBox *pOutputList = NULL );
	virtual int SendAndReadDpSVisioCommand( UINT16 uiCommand, UINT8 uiSize = 0, UINT8 *pData = NULL, CListBox *pOutputList = NULL ) { return -1; }

protected:
	DECLARE_MESSAGE_MAP()
	virtual afx_msg void OnTimer( UINT_PTR nIDEvent );

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

	CString _GetExceptionString( UINT uiCode );

// Private variables.
private:
	CDpSVisioDFUCommand m_clDpSVisioDFUCommand;
	CDpSVisioFakeDFUInformation m_clDpSVisioFakeDFUInformation;
	CMemFile m_FirmwareToUpload;

	UINT_PTR m_nTimer;
	UINT8 m_bPercent;
};
