#pragma once


#include "MyToolTipCtrl.h"
#include "ExtComboBox.h"
#include "XGroupBox.h"

/////////////////////////////////////////////////////////////////////////////
// CDlgHeader dialog
class CDlgHeader : public CDialogEx
{
// Construction
public:
	CDlgHeader(CWnd* pParent = NULL);   // standard constructor
	
// Dialog Data
	enum { IDD = IDD_DLGHEADER };
	CButton m_RadioHeaderRight;
	CButton m_RadioHeaderLeft;
	CButton m_RadioHeaderCentered;
	CButton m_RadioFooterRight;
	CButton m_RadioFooterLeft;
	CButton m_RadioFooterCentered;
	CButton m_ButtonFontFooter;
	CButton m_ButtonFontHeader;
	CEdit m_EditHeader;
	CEdit m_EditFooter;
	CButton m_Radio1StPage;
	CButton m_RadioAllPages;
	CExtNumEditComboBox m_ComboUnit;
	CStatic m_StaticUnit;
	CStatic m_StaticLogo1;
	// HYS-1090: The logo
	CStatic m_StaticCustoLogo;
	CEdit m_EditTopMargin;
	CEdit m_EditBotMargin;
	CButton m_CheckLogo;
	// The checkbox to show or not the logo
	CButton m_CheckCustoLogo;
	CButton m_Check1StPage;
	CXGroupBox m_GroupHeader;
	CXGroupBox m_GroupFooter;

// Overrides
	// ClassWizard generated virtual function overrides
	protected:
	virtual void DoDataExchange(CDataExchange* pDX);    // DDX/DDV support

// Implementation
public:
		int Display(CDB_PageSetup* pPageSetup);
		void SetValues2UUnit();
		double ChangeUnit(int PhysType,int CurrentUnit, int NewUnit, double val);

protected:

	// Generated message map functions
	virtual BOOL OnInitDialog();
	afx_msg void OnRadio1stpage();
	afx_msg void OnRadioallpages();
	afx_msg void OnClose();
	afx_msg void OnCheck1stpage();
	afx_msg void OnSelchangeCombounit();
	afx_msg void OnChangeEdittopmargin();
	afx_msg void OnChangeEditbotmargin();
	afx_msg void OnKillfocusEdittopmargin();
	afx_msg void OnKillfocusEditbotmargin();
	afx_msg void OnChecklogo();
	afx_msg void OnCheckCustologo();
	virtual void OnOK();
	virtual void OnCancel();
	afx_msg void OnButtonfontfooter();
	afx_msg void OnButtonfontheader();
	afx_msg void OnChangeEditfooter();
	afx_msg void OnKillfocusEditfooter();
	afx_msg void OnChangeEditheader();
	afx_msg void OnKillfocusEditheader();
	afx_msg void OnRadiofootercentered();
	afx_msg void OnRadiofooterleft();
	afx_msg void OnRadiofooterright();
	afx_msg void OnRadioheadercentered();
	afx_msg void OnRadioheaderleft();
	afx_msg void OnRadioheaderright();
	afx_msg HBRUSH OnCtlColor(CDC* pDC, CWnd* pWnd, UINT nCtlColor);
	afx_msg void OnButtonsetuserref();
	afx_msg void OnDestroy();
	DECLARE_MESSAGE_MAP()

		
		
	CTADatabase*	m_pTADB;
	CTADatastruct*	m_pTADS;
	CUserDatabase*	m_pUSERDB;
	CUnitDatabase*	m_pUnitDB;
	CDB_PageSetup	*m_pPageSetup,*m_pPageSetupBak;
	
	CBitmap			m_BmpLogo;
	// The logo's object
	CEnBitmap			m_BmpCustoLogo;

	LOGFONT			m_lf[_PAGESETUP_ENUM];
	CFont			m_Font[_PAGESETUP_ENUM];
	CBrush			m_brWhiteBrush;
	BYTE			m_bModified;
	int				m_iUnit;
	CMyToolTipCtrl		m_ToolTip;
	// To store last saved modifications if the current is not accepted 
	CString m_EditHeaderSaved;
	CString m_EditFooterSaved;
};