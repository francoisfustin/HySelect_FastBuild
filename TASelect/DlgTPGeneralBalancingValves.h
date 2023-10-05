#pragma once


#include "DlgCtrlPropPage.h"

class CDlgTPGeneralBalancingValves : public CDlgCtrlPropPage
{
	DECLARE_DYNAMIC( CDlgTPGeneralBalancingValves )

public:
	enum { IDD = IDD_TABDIALOGTECHVALVES };
	
	CDlgTPGeneralBalancingValves( CWnd *pParent = NULL );

	// Overriding 'CDlgCtrlPropPage' methods.
	virtual void Init( bool besetToDefault = false );
	virtual void Save( CString strSectionName );
	virtual bool VerifyInfo();

// Protected methods.
protected:
	DECLARE_MESSAGE_MAP()
	virtual void DoDataExchange( CDataExchange *pDX );
	virtual BOOL OnInitDialog();
	afx_msg void OnEnKillFocusValveMinDp();
	afx_msg void OnEnKillFocusValveMaxDp();
	afx_msg void OnEnSetFocusValveMinDp();
	afx_msg void OnEnSetFocusValveMaxDp();

// Protected variables.
protected:
	CTADatastruct *m_pTADS;

	CExtNumEdit m_EditValveMinDp;
	CExtNumEdit m_EditValveMaxDp;

	double m_dValveMinDp;
	double m_dValveMaxDp;
};
