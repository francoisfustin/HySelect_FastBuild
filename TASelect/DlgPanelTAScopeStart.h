#pragma once


#include "wizard.h"
#include "afxwin.h"
#include "afxcmn.h"
#include "StaticImg.h"

class CDlgWizTAScope;
class CDlgPanelTAScopeStart : public CDlgWizard
{
public:
	enum { IDD = IDD_PANEL_TASCOPE_START };

	CDlgPanelTAScopeStart( CWnd* pParent = NULL );
	virtual ~CDlgPanelTAScopeStart() {}

	void SetBtnInfoText( int iIDInfo, int iIDstr );
	void SetBtnInfoText( int iIDInfo, CString strText );

protected:
	DECLARE_MESSAGE_MAP()
	virtual void DoDataExchange( CDataExchange* pDX );
	virtual BOOL OnInitDialog();
	afx_msg void OnTimer( UINT_PTR nIDEvent );
	afx_msg HBRUSH OnCtlColor( CDC* pDC, CWnd* pWnd, UINT nCtlColor );
	afx_msg void OnBnClickedDownLoad();
	afx_msg void OnBnClickedUpLoad();
	afx_msg void OnBnClickedMaintenance();
	afx_msg void OnNMClickSyslink2( NMHDR* pNMHDR, LRESULT* pResult );
	afx_msg void OnClickedCheckoffline();

	/////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
	// Overrides 'CDlgWizard' protected virtual methods.
	virtual BOOL OnWizNext();
	virtual bool OnAfterActivate();
	virtual int OnWizButFinishTextID();
	virtual void OnGetMinMaxInfo( MINMAXINFO* lpMMI ) {}
	/////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

	bool UpdateVersionNumber();

// Private methods.
private:
	void _ActivateButtonsDownloadUpload( bool fEnable );

// Protected variables.
protected:
	CDlgWizTAScope *m_pParent;
	CBrush			m_brWhiteBrush;
	CBrush			m_brBtnFaceBrush;

	CMFCButton		m_ButDownLoad;
	CMFCButton		m_ButUpLoad;
	CMFCButton		m_ButMaintenance;

	CStatic			m_StaticVersionNbr;
	CStatic			m_ImgTASCOPEHeader;
	CStatic			m_LeftBanner;
	CStaticImg		m_StaticFlag;
	CLinkCtrl		m_UpdateAvailable;

	bool			m_fTADSHNEmpty;

	bool			m_fConnectionEstablished;
	CString			m_strPreviousDpsVersion;
	CString			m_strPreviousHHVersion;
	UINT_PTR		m_nTimer;
	CButton			m_CheckOffLine;
};
