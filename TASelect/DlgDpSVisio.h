#pragma once



// Defines for HID device.
#define GUID_HID							{ 0x4d1e55b2, 0xf16f, 0x11cf, { 0x88, 0xcb, 0x00, 0x11, 0x11, 0x00, 0x00, 0x30 } }
#define HIDDEVICE_VID_VALUE     			0x1EFB
#define HIDDEVICE_PID_VALUE     			0x1590
#define HIDDEVICE_VID_STRING     			_T("1EFB")
#define HIDDEVICE_PID_STRING    			_T("1590")
#define HIDDEVICE_MANUFACTURER_STRING    	_T("IMI HYDRONIC ENGINEERING")
#define HIDDEVICE_PRODUCT_STRING    		_T("DPS-VISIO-")

// Defines for DFU Se (ST Microelectrnics extension) device.
#define GUID_DFUSE							{ 0x3fe809ab, 0xfb91, 0x4cb5, { 0xa6, 0x43, 0x69, 0x67, 0x0d, 0x52, 0x36, 0x6e } }
#define DFUDEVICE_VID_VALUE     			0x0483
#define DFUDEVICE_PID_VALUE     			0xDF11
#define DFUDEVICE_VID_STRING     			_T("0483")
#define DFUDEVICE_PID_STRING    			_T("DF11")
#define DFUDEVICE_MANUFACTURER_STRING    	_T("STMICROELECTRONICS")
#define DFUDEVICE_PRODUCT_STRING    		_T("STM32")

// HYS-1911: Defines for the new DFU Se (ST Microelectrnics extension) device.
#define GUID_NEWDFUSE						{ 0x01105872, 0xbf45, 0x43be, { 0x8b, 0x67, 0x3c, 0x0f, 0x2b, 0x8c, 0xf0, 0xd9 } }
#define DFUDEVICE_VID_VALUE     			0x0483
#define DFUDEVICE_PID_VALUE     			0xDF11
#define DFUDEVICE_VID_STRING     			_T("0483")
#define DFUDEVICE_PID_STRING    			_T("DF11")
#define DFUDEVICE_MANUFACTURER_STRING    	_T("STMICROELECTRONICS")
#define DFUDEVICE_PRODUCT_STRING    		_T("STM32")

#define DLGVISIO_HOURGLASS_TIMEPASSEDCOLOR			RGB( 255, 0, 0 )
#define DLGVISIO_HOURGLASS_TIMETOPASSCOLOR			RGB( 0, 255, 0 )

class CDlgDpSVisioBase;
class CDlgDpSVisioUpgradeFinished;
class CDlgDpSVisio : public CDialogEx
{
	DECLARE_DYNAMIC( CDlgDpSVisio )

public:
	enum
	{
		IDD = IDD_DLGDPSVISIO
	};

	enum DialogToShow
	{
		Dialog_None,
		Dialog_Status,
		Dialog_HIDUpgradeRunning,
		Dialog_DFUUpgradeRunning,
		Dialog_UpgradeFinished,
	};

	enum WhichDevice
	{
		Device_HID,
		Device_DFUSe,
		Device_NewDFUSe,
		Device_NotYetDefined,
	};

	enum UpdateParam
	{
		UpdateParam_HID,
		UpdateParam_DFU
	};

	enum UpgradeError
	{
		UE_NoError,
		UE_DFUNotDetected,
		UE_DeviceDisconnected,
		UE_DeviceChanged,
		UE_SendingFWUnexpectedEOF,
		UE_SendingFWUnexpectedError,
		UE_InternalError,
	};

	struct UpgradeStatus
	{
		bool m_fUpgradeStatus;
		UpgradeError m_eUpgradeError;
		DWORD m_dwDFUInternalErrorCode;
	};

	CDlgDpSVisio( CWnd *pParent = NULL );
	virtual ~CDlgDpSVisio();

// Public methods.
public:
	void ShowDialog( DialogToShow eDlgToShow, LPARAM lpParam = (LPARAM)0 );
	void CloseApplication() { OnOK(); }
	CDpSVisioBaseCommunication *GetDpSVisioCommunication( void ) { return m_pclDpSVisioCommunication; }

	bool IsDpSVisioInNormalModeConnected( void );
	bool IsDpSVisioInDFUModeConnected( void );

	void DisconnectHIDDevice();

	// This method doesn't work! I keep it just because it has been hard to get the code.
	// 'SetupDiCallClassInstaller' can not be called from a x32 process on a x64 machine.
	void EnableDpSVisioInHIDMode( bool bEnable );
	
	bool IsFirmwareAvailable( WhichDevice eWhichDevice ) { return ( 0 == m_arsFirmwareData[eWhichDevice].m_clFirmware.GetLength() ) ? false : true; }
	CMemFile *GetFirmwareToUpload( WhichDevice eWhichDevice ) { return &m_arsFirmwareData[eWhichDevice].m_clFirmware; }

	UINT32 GetAvailableFirmwareRevisionValue( WhichDevice eWhichDevice ) { return m_arsFirmwareData[eWhichDevice].m_uiRevision; }

	// Allow to retrieve the available firmware version for DpS-Visio.
	CString GetAvailableFirmwareRevisionString( WhichDevice eWhichDevice ) { return m_arsFirmwareData[eWhichDevice].m_strRevision; }

	// Allow to retrieve the file name to upload to DpS-Visio.
	CString GetAvailableFirmwareFilename( WhichDevice eWhichDevice ) { return m_arsFirmwareData[eWhichDevice].m_strFilename; }

	enum UpdateButtonType
	{
		UBT_Update,
		UBT_ForceUpdate
	};
	void EnableUpdateButtons( UpdateButtonType eWhichButton, BOOL bEnable, BOOL bShow );
	void EnableCancelButton( BOOL bEnable );
	void ChangeCancelButtonName( CString strNewName );

	// Allow to save upgrade status here to be able to pass this structure for the last 'CDlgDpSVisioUpgradeFinnished' dialog.
	UpgradeStatus *GetUpgradeStatusData( void ) { return &m_rUpgradeStatus; }

	// Because on some computer Windows doesn't detect well HID Detach and DFU Attach event, we have to periodically check
	// the DFU presence of the device.
	void StartDFUPollingDetection( void );

protected:
	virtual BOOL PreTranslateMessage( MSG *pMsg );
	virtual void DoDataExchange( CDataExchange *pDX );    // DDX/DDV support
	virtual BOOL OnInitDialog();
	virtual void OnOK();
	virtual void OnCancel();

	DECLARE_MESSAGE_MAP()
	virtual afx_msg LRESULT OnForceDpSVisioDetection( WPARAM wParam = 0, LPARAM lParam = 0 );
	virtual afx_msg LRESULT OnDpSVisioUpdated( WPARAM wParam = 0, LPARAM lParam = 0 );
	virtual afx_msg LRESULT OnShowDialogToChange( WPARAM wParam = 0, LPARAM lParam = 0 );
	virtual afx_msg LRESULT OnEnableDisableButton( WPARAM wParam = 0, LPARAM lParam = 0 );
	virtual afx_msg LRESULT OnExit( WPARAM wParam = 0, LPARAM lParam = 0 );
	virtual afx_msg BOOL OnDeviceChange( UINT nEventType, DWORD_PTR dwData );
	virtual afx_msg void OnBnClickedUpdate();
	virtual afx_msg void OnBnClickedForceUpdate();
	virtual afx_msg void OnBnClickedCancel();
	virtual afx_msg void OnTimer( UINT_PTR nIDEvent );

#ifdef DEBUG
	virtual afx_msg void OnBnClickedButtonHIDConnect();
	virtual afx_msg void OnBnClickedButtonHIDDisconnect();
	virtual afx_msg void OnBnClickedButtonDFUConnect();
	virtual afx_msg void OnBnClickedButtonDFUDisconnect();
#endif

// Private members.
private:
	CString _CheckIfBINFileExist( void );
	CString _CheckIfDFUFileExist( int iWhichDFU );
	bool _CheckIfHIDDeviceConnected( void );
	bool _CheckIfDFUSeDeviceConnected( void );
	bool _CheckIfNewDFUSeDeviceConnected( void );
	bool _IsDpSVisioHID( CString strDeviceName );
	bool _IsDpSVisioDFUSe( CString strDeviceName );
	bool _IsDpSVisioNewDFUSe( CString strDeviceName );
	void _ConnectHIDDevice( CString strDeviceName );
	void _DisconnectHIDDevice( CString strDeviceName );
	void _ConnectDFUSeDevice( CString strDeviceName );
	void _DisconnectDFUSeDevice( CString strDeviceName );
	void _ConnectNewDFUSeDevice( CString strDeviceName );
	void _DisconnectNewDFUSeDevice( CString strDeviceName );
	void _CloseNewDFUSeDeviceHelper( bool bAndDisconnect );

	CString _GetExceptionString( UINT uiCode );

// Private variables.
private:
	CDlgDpSVisioBase *m_pCurrentDialog;
	DialogToShow m_eCurrentDlg;

	CDpSVisioBaseCommunication *m_pclDpSVisioCommunication;

	CRITICAL_SECTION m_CriticalSection;
	UINT_PTR m_nTimer;

	typedef struct _FirmwareData
	{
		GUID m_InterfaceClassGuid;
		HDEVNOTIFY m_hDevNotify;
		CMemFile m_clFirmware;
		UINT32 m_uiRevision;
		CString m_strRevision;
		CString m_strFilename;
		CString m_strError;
	}FirmwareData;

	FirmwareData m_arsFirmwareData[3];

	CStatic m_StaticLeftBanner;
	HBITMAP m_hLeftPanelNormal;
	HBITMAP m_hLeftPanelUpdateRunning;
	HBITMAP m_hLeftPanelUpdateFinishedOK;
	HBITMAP m_hLeftPanelUpdateFinishedError;

	UpgradeStatus m_rUpgradeStatus;

	CButton m_ButtonHIDConnect;
	CButton m_ButtonHIDDisconnect;
	CButton m_ButtonDFUConnect;
	CButton m_ButtonDFUDisconnect;

	// For New DFU Se.
	struct libusb_context *m_pUSBContext;
	_DFUUtilWorkVariables m_rNewDFUSeVariables;
};
