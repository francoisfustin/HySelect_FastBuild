#pragma once


#include "ExtProgressCtrl.h"
#include "ExtStatic.h"

#define DLGVISIOHIDUPDATERUNNING_TIMERMSEC				300
#define DLGVISIOHIDUPDATERUNNING_INTERVALTOWAITSTOP		10
#define DLGVISIOHIDUPDATERUNNING_WAITUSBEVENTTIMEOUT	70		// 70 * 300 ms = 21sec
#define DLGVISIOHIDUPDATERUNNING_UNPLUGPLUGTEXTCOLOR	_TAH_ORANGE

class CDlgDpSVisioHIDUpgradeRunning : public CDlgDpSVisioBase
{
	DECLARE_DYNAMIC( CDlgDpSVisioHIDUpgradeRunning )

public:
	enum { IDD = IDD_DLGDPSVISIO_UPGRADERUNNING };

	CDlgDpSVisioHIDUpgradeRunning( CDlgDpSVisio *pParent );
	virtual ~CDlgDpSVisioHIDUpgradeRunning();

// Public methods.
public:
	static UINT ThreadUpgradeRunning( LPVOID pParam );

	// Overrides 'CDlgDpSVisioBase' public methods.
	virtual void Stop( void );
	virtual void OnDpSVisioConnected( CDlgDpSVisio::WhichDevice eWhichDevice );
	virtual void OnDpSVisioDisconnected( CDlgDpSVisio::WhichDevice eWhichDevice );

// Protected methods.
protected:
	virtual void DoDataExchange( CDataExchange *pDX );
	virtual BOOL OnInitDialog();

	DECLARE_MESSAGE_MAP()
	virtual afx_msg void OnTimer( UINT_PTR nIDEvent );
	virtual afx_msg LRESULT OnThreadFinished( WPARAM wParam = 0, LPARAM lParam = 0 );

// Private methods.
private:
	enum _StateEngine
	{
		StartFirmwareUploadSession,
		SendFirmwareVersion,
		SendOnePacket,
		SendCRC,
		StartUpgrade,
		WaitHIDDetach,
		WaitHIDAttach,
		VerifyDevice,
		Finished,
		UpdateError,
		WaitingToBeKilled,
		NothingToDo,
		DeviceDisconnected
	};

	void _UpgradeProgess( void );
	void _StopThreadUpgradeRunning( void );
	_StateEngine _SetUpgradeError( CDlgDpSVisio::UpgradeError eUpgradeError, DWORD dwInternalError, _StateEngine eStateEngine = UpdateError );

// Private variables.
private:
	CWinThread *m_pThread;
	CEvent *m_pclStopThreadEvent;
	CEvent *m_pclThreadStoppedEvent;

	CExtProgressCtrl m_UpdateProgress;
	CExtStatic m_StaticInfo;
	CListBox m_UpdateOutput;
	CRITICAL_SECTION m_CriticalSection;
	_StateEngine m_eStateEngine;
	CString m_strHIDDeviceName;
	UINT m_uiUSBEventCounter;

	std::vector<CString> m_vecMsgToPrintInListBox;
	UINT_PTR m_nTimer;

	CMemFile *m_pclFirmwareToUpload;
};
