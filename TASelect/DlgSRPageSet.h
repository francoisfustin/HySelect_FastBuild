#pragma once


class CDlgSRPageSet : public CDlgCtrlPropPage
{
	DECLARE_DYNAMIC( CDlgSRPageSet )

public:
	// To interact with 'CDlgSearchReplace'.
	class IDlgSRPageSetNotificationHandler
	{
	public:
		virtual void SRPageSet_OnRadioSplitClicked() = 0;
		virtual void SRPageSet_OnRadioGroupClicked() = 0;
		virtual void SRPageSet_OnReplacePopupHM( CDS_HydroMod* pHM ) = 0;
	};

	enum { IDD = IDD_TABDLGSEARCHSET };
	
	CDlgSRPageSet(CWnd* pParent = NULL);
	virtual ~CDlgSRPageSet();
	
	void SetNotificationHandler( IDlgSRPageSetNotificationHandler* pNotificationHandler );
	bool IsRadioSplitChecked( void );
	bool IsRadioGroupChecked( void );

	// Overrides 'CDlgCtrlPropPage'.
	virtual void Init( bool bRstToDefault = false );
	virtual bool OnBnClickedBtnStart( std::vector<LPARAM> *pvecParams );
	
// Protected methods.
protected:
	DECLARE_MESSAGE_MAP()
	virtual void DoDataExchange( CDataExchange* pDX );
	virtual BOOL OnInitDialog();
	afx_msg void OnRadioClickedSplitCVActrSet();
	afx_msg void OnRadioClickedGroupCVActr();
	afx_msg LRESULT OnReplacePopupHM( WPARAM wParam = 0, LPARAM lParam = 0 );

// Private variables.
private:
	IDlgSRPageSetNotificationHandler* m_pNotificationHandler;
};
