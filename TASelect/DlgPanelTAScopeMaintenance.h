#pragma once


#include "wizard.h"

class CDlgPanelTAScopeMaintenance : public CDlgWizard
{
public:
	enum { IDD = IDD_PANEL_TASCOPE_MAINTENANCE };

	CDlgPanelTAScopeMaintenance( CWnd* pParent = NULL );
	virtual ~CDlgPanelTAScopeMaintenance() {}

protected:
	DECLARE_MESSAGE_MAP()
	virtual void DoDataExchange( CDataExchange* pDX );
	virtual BOOL OnInitDialog();
	afx_msg void OnTimer( UINT_PTR nIDEvent );
	afx_msg HBRUSH OnCtlColor( CDC* pDC, CWnd* pWnd, UINT nCtlColor );
	afx_msg void OnBnCleanLocalFiles();
	afx_msg void OnBnCleanTAScopeFiles();
	afx_msg void OnBnForceUpdate();

	/////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
	// Overrides 'CDlgWizard' protected virtual methods.
	virtual BOOL OnWizCancel();
	virtual BOOL OnWizFinish();
	virtual bool OnAfterActivate();
	virtual int OnWizButFinishTextID();
	virtual void OnGetMinMaxInfo( MINMAXINFO* lpMMI ) { lpMMI->ptMinTrackSize.x = 400; lpMMI->ptMinTrackSize.y = 400; }
	/////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

// Protected variables.
protected:
	CDlgWizTAScope *m_pParent;
	CBrush			m_brWhiteBrush;
	CStatic			m_StaticVersionNbr;
	CTAMetaData		m_MetaData;
	CMFCButton		m_ButtonDeleteLocFiles;
	CMFCButton		m_ButtonDeleteTASCOPEFiles;
	CMFCButton		m_ButtonForceUpdate;
	UINT_PTR		m_nTimer;
};
