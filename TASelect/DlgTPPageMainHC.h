#pragma once


#include "DlgCtrlPropPage.h"
#include "ExtButton.h"
#include "ExtStatic.h"

class CDlgTPPageMainHC : public CDlgCtrlPropPage
{
	DECLARE_DYNAMIC( CDlgTPPageMainHC )

public:
	enum { IDD = IDD_TABDIALOGTECHMAINHC };

	CDlgTPPageMainHC( CWnd* pParent = NULL );

	// Overriding 'CDlgCtrlPropPage' methods.
	virtual void Init( bool fResetToDefault = false );
	virtual void Save( CString strSectionName );

// Protected members.
protected:
	DECLARE_MESSAGE_MAP()
	virtual void DoDataExchange( CDataExchange* pDX );
	virtual BOOL OnInitDialog();
	afx_msg void OnBnClickedAutoCheck();
	afx_msg void OnBnCheckBuildOldProduct();

// Protected variables.
protected:
	CTADatastruct*	m_pTADS;
	CButton			m_AutoCheck;
	bool			m_fAutoCheck;
	// HYS-1221 : Add a checkbox to allow building Hm calc project with deleted products
	CExtButton			m_BnCheckBuildOld;
	bool			m_bCheckBuildOld;
	CExtStatic m_staticCheckOldProducts;
};
