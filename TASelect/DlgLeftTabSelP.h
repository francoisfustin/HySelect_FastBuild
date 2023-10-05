#pragma once


#include "afxwin.h"
#include "MyToolTipCtrl.h"
#include "XGroupBox.h"
#include "ExtComboBox.h"
#include "SelProdPageBase.h"

class CDlgLeftTabSelP : public CDlgLeftTabBase
{
public:
	
	enum { IDD = IDD_DLGLEFTTABSELP };

	CDlgLeftTabSelP( CWnd* pParent = NULL );
	virtual ~CDlgLeftTabSelP();

	// Overrides 'CDlgLeftTabBase' public virtual methods.
	virtual void OnLTTabCtrlEnterTab( CMyMFCTabCtrl::TabIndex eTabIndex, CMyMFCTabCtrl::TabIndex ePrevTabIndex );
	virtual void OnLTTabCtrlLeaveTab( CMyMFCTabCtrl::TabIndex eTabIndex );

	// Overrides 'CDlgLeftTabBase' public pure virtual methods.
	virtual void InitializeToolsDockablePaneContextID( void );
	virtual bool GetToolsDockablePaneContextID( int& iContextID );

	// Return ID of Scheme index or NULL if doesn't exist.
	CString GetScheme( int iIndex, CDS_SSelDpC **ppSelDpC );
	
	// 'FindLastSameItem' search into SelManBv/SelDpC array for the last item index 
	// where the key field is matching the first item key field.
	// Return this index.
	int FindLastSameItem( CData **parclData, int iFirstIndex, int iLastIndex, PageField_enum eKey );
	
	// Set an index for remarks/Scheme.
	// Scan all object, check remarks and put the same index for the same remark.
	// Return first available index.
	int SetRemarkIndex( LPCTSTR strClassName, CData **parclData, int iArraySize, int iFirstIndex = 0 );

	// Set an index for remarks for HM calc.
	int SetRemarkIndex( CSelProdPageBase::HMvector *pvecHMVector, int iFirstIndex = 0 );

	void SetSchemeIndex( LPCTSTR strClassName, CData **parclData, int iArraySize );
	
	// Sort paSelManBV/paSelDpC array (from FirstIndex to LastIndex) with a sorting key.
	void SortSelection( CData **parclData, int iFirstIndex, int iLastIndex, PageField_enum eKey );

	void SortTable( CData **parclData, int iLast );
	
	CDB_PageSetup* GetpPageSetup() { return m_pTADSPageSetup; }

// Implementation
protected:
	DECLARE_MESSAGE_MAP()
	virtual void DoDataExchange( CDataExchange* pDX );
	virtual BOOL OnInitDialog();
	virtual void OnOK();
	virtual BOOL OnCommand( WPARAM wParam, LPARAM lParam );
	afx_msg LRESULT OnNewDocument( WPARAM wParam, LPARAM lParam );
	afx_msg LRESULT OnParamChange( WPARAM wParam, LPARAM lParam );
	afx_msg LRESULT OnWaterChange( WPARAM wParam, LPARAM lParam );
	afx_msg void OnButtonDeleteSetup();
	afx_msg void OnButtonAddSetup();
	afx_msg void OnSelChangeComboSetup();
	afx_msg void OnShowWindow( BOOL bShow, UINT nStatus );
	afx_msg void OnSelChangeComboSortKey1();
	afx_msg void OnSelChangeComboSortKey2();
	afx_msg void OnSelChangeComboSortKey3();
	afx_msg void OnButtonSort();
	afx_msg void OnButtonFormatColumn();
	afx_msg void OnBnClickedHubExportConfigurationFile();
	afx_msg void OnBnClickedExportD81();
	afx_msg void OnBnClickedExportD82();
	
	void RefreshHubGroup();
	void InitPageSetupDD( CString str = _T("") );
	void SavePageSetup( CDB_PageSetup *pPageSetup );
	void InitPageSetup( CDB_PageSetup *pPageSetup );

// Protected variables.
protected:
	CTADatabase* m_pTADB;
	CTADatastruct* m_pTADS;
	CUserDatabase* m_pUSERDB;
	CDB_PageSetup* m_pTADSPageSetup;
	CMyToolTipCtrl m_ToolTip;			// ToolTipCtrl

	CFont m_LiteFont;
	HTREEITEM HTreeItem[CDB_PageSetup::enCheck::LASTFIELD];
	CButton m_ButtonPageField;
	CButton m_ButtonDispRef;
	CMFCButton m_ButtonSort;
	CButton m_ButtonHeader;
	CMFCButton m_ButtonDelSetup;
	CMFCButton m_ButtonAddSetup;
	CExtNumEditComboBox m_ComboSetup;
	CComboBox m_ComboKey3;
	CComboBox m_ComboKey2;
	CComboBox m_ComboKey1;
	CXGroupBox m_GroupDispInfo;
	CXGroupBox m_GroupSorting;
	CXGroupBox m_GroupTAHUB;
};

extern CDlgLeftTabSelP *pDlgLeftTabSelP;