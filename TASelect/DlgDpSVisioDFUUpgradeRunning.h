#pragma once


#include "ExtProgressCtrl.h"
#include "ExtStatic.h"

#define DLGVISIODFUUPDATERUNNING_TIMERMSEC						300
#define DLGVISIODFUUPDATERUNNING_INTERVALTOWAITSTOP				10
#define DLGVISIODFUUPDATERUNNING_WAITUSBEVENTTIMEOUT			70		// 70 * 300 ms = 21sec
#define DLGVISIODFUUPDATERUNNING_UNPLUGPLUGTEXTCOLOR			_TAH_ORANGE

class CDlgDpSVisioDFUUpgradeRunning : public CDlgDpSVisioBase
{
	DECLARE_DYNAMIC( CDlgDpSVisioDFUUpgradeRunning )

public:
	enum { IDD = IDD_DLGDPSVISIO_UPGRADERUNNING };

	CDlgDpSVisioDFUUpgradeRunning( CDlgDpSVisio *pParent );
	virtual ~CDlgDpSVisioDFUUpgradeRunning();

// Public methods.
public:
	static UINT ThreadUpgradeRunning( LPVOID pParam );

	// Overrides 'CDlgDpSVisioBase' public methods.
	virtual void SetParam( LPARAM lpParam );
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
	virtual afx_msg LRESULT OnShowDlgDpSVisioUnplugPlug( WPARAM wParam = 0, LPARAM lParam = 0 );
	virtual afx_msg LRESULT OnFinishDlgDpSVisioUnplugPlug( WPARAM wParam = 0, LPARAM lParam = 0 );
	virtual afx_msg LRESULT OnDestroyDlgDpSVisioUnplugPlug( WPARAM wParam = 0, LPARAM lParam = 0 );

	virtual afx_msg LRESULT OnShowDlgDFUInstallation( WPARAM wParam = 0, LPARAM lParam = 0 );
	virtual afx_msg LRESULT OnFinishDlgDpSVisioDFUInstallation( WPARAM wParam = 0, LPARAM lParam = 0 );

// Private methods.
private:
	enum _StateEngine
	{
		HID_StartDFU_SendCmd,
		HID_StartDFU_WaitAnswer,
		HID_StartDFU_WaitHIDDetach,
		HID_StartDFU_WaitDFUAttach,
		DFU_Wait_DlgDFUInstallationLaunch,
		DFU_DlgDFUInstallationLaunched,
		DFU_Wait_DlgDFUInstallationDestroy,
		DFU_StartErase,
		DFU_CheckErase,
		DFU_StartUpgrade,
		DFU_CheckUpgrade,
		DFU_Wait_DlgUnplugPlugLaunch,
		DFU_DlgUnplugPlugLaunched,
		DFU_Wait_DlgUnplugPlugDestroy,
		DFU_DlgUnplugPlugDestroyed,
		DFU_StartHDI_WaitDFUDetach,
		DFU_StartHDI_WaitHIDAttach,
		HID_VerifyDevice,
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

	CDlgDpSVisioUnplugPlug *m_pDlgDpSVisioUnplugPlug;
	CDlgDpSVisioDFUInstallation *m_pDlgDpSVisioDFUInstallation;
};
