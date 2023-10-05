#pragma once


#include "MyToolTipCtrl.h"
#include "ExtNumEdit.h"
#include "DialogExt.h"
#include "DlgReplacePopup.h"

#define MINDPVALUE 1000

class CDlgCtrlPropPage : public CDialogExt
{
	DECLARE_DYNAMIC( CDlgCtrlPropPage )

public:
	enum { IDD = IDD_DLGCTRLPROPPAGE };

	CDlgCtrlPropPage( CWnd* pParent = NULL );

	virtual CString GetHeaderTitle() { return m_csHeaderTitle; }
	virtual CString GetPageName() { return m_csPageName; }
	virtual void Init( bool fResetToDefault = false ) {}
	virtual void Save( CString strSectionName ) {}
	// HYS-1221 : Created as virtual to update HM combo products preferences regarding the checkbox that
	// allows building project with deleted products
	virtual void UpdateComboWithDeleted() {}
	virtual bool VerifyInfo();
	virtual bool OnBnClickedBtnStart( std::vector<LPARAM> *pvecParams ) { return true; }

	// Help us for 'Search and replace' functionalities.
	// Remark: 'LPARAM' to avoid compilation problem (can't include 'HMTreeListCtrl.h').
	virtual void SetTree( LPARAM pHMTreeListCtrl ) {}

// Protected members.
public:
	virtual void DoDataExchange( CDataExchange* pDX );
	virtual BOOL OnInitDialog();
	virtual BOOL OnCommand( WPARAM wParam, LPARAM lParam );

	void SetCorrectDblMinMaxBackColor( CExtNumEdit* pclMin, CExtNumEdit* pclMax, bool fReset = false );
	void ModifyHeaderTitle( CString scNewHeader ) { m_csHeaderTitle = scNewHeader; }
	void SetPageNumber( int i ) { m_iPageNumber = i; }
	int GetPageNumber() { return m_iPageNumber; }

// Protected variables.
protected:
	CString m_csPageName;
	CString	m_csHeaderTitle;
	int m_iPageNumber;
	CDlgReplacePopup* m_pDlgReplacePopup;
	CMyToolTipCtrl  m_ToolTip;
};
