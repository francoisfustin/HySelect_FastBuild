#pragma once


#include "DlgCtrlPropPage.h"
#include "ExtNumEdit.h"

class CDlgTPGeneralFixedOrifices : public CDlgCtrlPropPage
{
	DECLARE_DYNAMIC( CDlgTPGeneralFixedOrifices )

public:
	enum { IDD = IDD_TABDIALOGTECHFIXORIFICE };

	CDlgTPGeneralFixedOrifices( CWnd* pParent = NULL );

	// Overriding 'CDlgCtrlPropPage' methods.
	virtual void Init( bool fResetToDefault = false );
	virtual void Save( CString strSectionName );
	virtual bool VerifyInfo();

// Protected methods.
protected:
	DECLARE_MESSAGE_MAP()
	virtual void DoDataExchange( CDataExchange* pDX );
	virtual BOOL OnInitDialog();
	afx_msg void OnEnKillFocusFOMinDp();
	afx_msg void OnEnKillFocusFOMaxDp();
	afx_msg void OnEnSetFocusFOMinDp();
	afx_msg void OnEnSetFocusFOMaxDp();

// Protected variables.
protected:
	CTADatastruct*	m_pTADS;

	CExtNumEdit		m_EditFoMinDp;
	CExtNumEdit		m_EditFoMaxDp;

	double			m_dFoMinDp;
	double			m_dFoMaxDp;
};
