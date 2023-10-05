#pragma once


#include "DlgProgressBar.h"
#include "SSheet.h"

class CDlgProgBarExportXls : public CDlgProgressBar
{
	DECLARE_DYNAMIC( CDlgProgBarExportXls )

public:
	CDlgProgBarExportXls( CWnd* pParent = NULL, CString Title = _T("") );
	~CDlgProgBarExportXls();

	void SetParamForExport( CSSheet* pSheetHM, CTable* pTab, int iTabOffset );
	bool CanExport( void ) { return m_fCanExport; }
	static UINT ThreadInitSheetModules( LPVOID pParam );

protected:
	DECLARE_MESSAGE_MAP()
	virtual BOOL OnInitDialog();
	afx_msg void OnClose();
	// In case user cancel by pressing ESC key.
	virtual void OnCancel();

private:
	bool _StopThread( void );
	// Initialize a sheetHMCalc with all modules and sub modules listed recursively in the tab.
	bool _InitSheetModules( CSSheet* pSheetHM, CTable* pTab );

protected:
	bool		m_fCanExport;
	CEvent*		m_pclStopEvent;
	CEvent*		m_pclSuspendEvent;
	CEvent*		m_pclResumeEvent;
	CEvent*		m_pclThreadStopped;
	CEvent*		m_pclThreadSuspended;
	CWinThread* m_pThread;
	CSSheet*	m_pSheetHM;
	CTable*		m_pTab;
	int			m_iMaxTabSheet;
	int			m_iTabOffset;
	bool		m_fCancel;
};
