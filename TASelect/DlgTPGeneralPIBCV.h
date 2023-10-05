#pragma once


#include "DlgCtrlPropPage.h"

class CDlgTPGeneralPIBCV : public CDlgCtrlPropPage
{
	DECLARE_DYNAMIC( CDlgTPGeneralPIBCV )
	
public:
	enum { IDD = IDD_TABDIALOGTECHPICV };
	CDlgTPGeneralPIBCV( CWnd* pParent = NULL );

	// Overriding 'CDlgCtrlPropPage' methods.
	virtual void Init( bool fResetToDefault = false );
	virtual void Save( CString strSectionName );
	virtual bool VerifyInfo();	

// Protected methods.
protected:
	DECLARE_MESSAGE_MAP()
	virtual void DoDataExchange( CDataExchange* pDX );
	virtual BOOL OnInitDialog();
	afx_msg void OnBnClickedHide();
	
// Protected variables.
protected:
	CTADatastruct*	m_pTADS;

	CExtNumEdit		m_EditMinDp;
	CButton			m_Hide;

	double			m_dPICvMinDp;
	int				m_iPICvHide;
};
