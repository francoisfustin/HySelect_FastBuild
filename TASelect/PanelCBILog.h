#pragma once

class CWizardCBI;
class CPanelCBILog : public CDlgWizard
{
public:
	enum { IDD = IDD_PANELCBILOG };
	CPanelCBILog( CWnd* pParent = NULL );
	virtual ~CPanelCBILog();

protected:
	DECLARE_MESSAGE_MAP()
	virtual void DoDataExchange( CDataExchange* pDX );
	virtual BOOL OnInitDialog();
	afx_msg HBRUSH OnCtlColor( CDC* pDC, CWnd* pWnd, UINT nCtlColor );
	afx_msg void OnEndlabeleditList( NMHDR* pNMHDR, LRESULT* pResult );
	afx_msg void OnButselect();
	afx_msg void OnButunselect();
	afx_msg void OnClickList( NMHDR* pNMHDR, LRESULT* pResult );

	/////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
	// Overrides 'CDlgWizard' protected virtual methods.
	virtual BOOL OnWizFinish();
	virtual bool OnActivate();
	virtual bool OnAfterActivate();
	virtual int OnWizButtons() { return CWizardManager::WizButFinish; }
	virtual int OnWizButFinishTextID() { return IDS_WIZBUT_FINISH; }
	virtual void OnGetMinMaxInfo( MINMAXINFO* lpMMI ) { lpMMI->ptMinTrackSize.x = 400; lpMMI->ptMinTrackSize.y = 400; }
	/////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

// Private variables.
private:
	CTADatastruct* m_pTADS;
	CWizardCBI *m_pParent;
	CBrush m_brWhiteBrush;
	CListCtrl m_List;
	CEdit m_EditText;
	CImageList m_ImageList;
};