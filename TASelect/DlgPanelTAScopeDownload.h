#pragma once


#include "wizard.h"
#include "afxcmn.h"
#include "afxwin.h"
#include "StaticImg.h"
#include "datastruct.h"

class CDlgPanelTAScopeDownload : public CDlgWizard
{
public:
	enum { IDD = IDD_PANEL_TASCOPE_DOWNLOAD };

	CDlgPanelTAScopeDownload( CWnd* pParent = NULL );
	virtual ~CDlgPanelTAScopeDownload() {}

	// 2017-01-13: 'bDebugMode' can be true if this method is called by 'CTabDlgSpecActTesting::_InterpretCmd'.
	bool DownLoadHMX( bool bTestMode = false, CString strOfflineTSCFileName = _T("") );

protected:
	DECLARE_MESSAGE_MAP()
	virtual void DoDataExchange( CDataExchange* pDX );
	virtual BOOL OnInitDialog();

	afx_msg void OnTimer( UINT_PTR nIDEvent );
	afx_msg HBRUSH OnCtlColor( CDC* pDC, CWnd* pWnd, UINT nCtlColor );
	afx_msg void OnNMClickTree( NMHDR* pNMHDR, LRESULT* pResult );
	afx_msg void OnTvnSelChangedTreeQMLog( NMHDR* pNMHDR, LRESULT* pResult );
	afx_msg void OnTvnSelChangedTreePlant( NMHDR* pNMHDR, LRESULT* pResult );
	
	// User click on the tree.
	// Display information according to the tree state.
	afx_msg LRESULT OnCheckStateChange( WPARAM wParam, LPARAM lParam );

	/////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
	// Overrides 'CDlgWizard' protected virtual methods.
	virtual BOOL OnWizCancel();
	virtual BOOL OnWizFinish();
	virtual BOOL OnWizNext();
	virtual bool OnAfterActivate();
	virtual int OnWizButFinishTextID();
	virtual int OnWizButNextTextID();
	virtual void OnGetMinMaxInfo( MINMAXINFO* lpMMI ) { lpMMI->ptMinTrackSize.x = 400; lpMMI->ptMinTrackSize.y = 400; }
	/////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

	void FillDataFromTADSX();
	bool RefreshTreeQMLog();
	bool RefreshTreePlant();

// Protected variables.
protected:
	CDlgWizTAScope* m_pParent;
	CBrush m_brWhiteBrush;
	CEdit m_EditPlantContent;
	CEdit m_EditContent;
	CTADataStructX m_TADSX;
	CTAMetaData m_MetaData;

	unsigned int m_uiNumberOfQM;
	unsigned int m_uiNumberOfLog;
	CString m_strPlantName;
	UINT_PTR m_nTimer;

	CStatic m_StaticVersionNbr;
	CTreeCtrl m_TreeQMLog;
	CTreeCtrl m_TreePlant;
	CTreeCtrl* m_pTree;

// Private methods.
private:
	void _CheckIsAtLeastOneItemSelected( void );
	ProjectType _RetrieveProjectType( CTADataStructX *pclTADataStructX );
};
