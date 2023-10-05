#pragma once


#include "DialogExt.h"
#include "afxwin.h"
#include "MacroDefs.h"

class CTabDlgSpecActTesting2 : public CDialogExt
{
public:
	enum { IDD = IDD_TABSPECACT_TESTING2 };

	CTabDlgSpecActTesting2( CWnd* pParent = NULL );
	~CTabDlgSpecActTesting2();

	void EnableMacroRunning( bool fEnable );
	bool IsMacroRunning( void );
	void WriteMacro( MSG* pMsg );
	static UINT ThreadRunMacro( LPVOID pParam );

protected:
	DECLARE_MESSAGE_MAP()
	virtual void DoDataExchange( CDataExchange* pDX );
	virtual BOOL OnInitDialog();
	afx_msg void OnSize(UINT nType, int cx, int cy);
	afx_msg void OnBnClickedStart();
	afx_msg void OnBnClickedStop();
	afx_msg void OnBnClickedClear();
	afx_msg void OnBnClickedSave();
	afx_msg void OnBnClickedLoad();
	afx_msg void OnBnClickedRun();

// Private variables.
private:
	CListBox m_List;
	bool m_fIsRecording;
	bool m_fIsMacroLoading;

	double m_dInputMouseFactorX;
 	double m_dInputMouseFactorY;

	CWinThread* m_pThread;
	CEvent*	m_pclStopEvent;
	CEvent*	m_pclThreadStopped;
	bool m_fIsRunning;
	
	vecMacro m_vecMacroSaved;
	int m_iPrevMacroIndex;

// Private methods.
private:
	bool _FillInfo( MSG *pMsg );

	bool _FillMFCTabCtrlData( CMacroWndInfo *pclWndInfo, CWnd *pFirstWnd, CWnd *pCurrentWnd, CMacro *pclMacro );
	bool _FillTreeCtrlData( CMacroWndInfo *pclWndInfo, CWnd *pFirstWnd, CWnd *pCurrentWnd, CMacro *pclMacro );
	bool _FillMFCRibbonBarData( CMacroWndInfo *pclWndInfo, CWnd *pFirstWnd, CWnd *pCurrentWnd, CMacro *pclMacro );
	
	void _ClearAllMacro( void );
};
