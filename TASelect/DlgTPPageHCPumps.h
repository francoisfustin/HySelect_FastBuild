#pragma once


#include "DlgCtrlPropPage.h"

class CDlgTPPageHCPumps : public CDlgCtrlPropPage
{
	DECLARE_DYNAMIC( CDlgTPPageHCPumps )

public:
	enum { IDD = IDD_TABDIALOGTECHHCPUMPS };
	
	CDlgTPPageHCPumps( CWnd *pParent = NULL );

	// Overriding 'CDlgCtrlPropPage' methods.
	virtual void Init( bool besetToDefault = false );
	virtual void Save( CString strSectionName );
	virtual bool VerifyInfo();

// Protected methods.
protected:
	DECLARE_MESSAGE_MAP()
	virtual void DoDataExchange( CDataExchange *pDX );
	virtual BOOL OnInitDialog();
	afx_msg void OnEnKillFocusDefaultSecondaryPumpHMin();
	afx_msg void OnEnSetFocusDefaultSecondaryPumpHMin();

// Protected variables.
protected:
	CTADatastruct *m_pTADS;

	CExtNumEdit m_EditDefaultSecondaryPumpHMin;

	double m_dDefaultSecondaryPumpHMin;
};
