#pragma once


#include "DlgCtrlPropPage.h"
#include "ExtNumEdit.h"
#include "afxwin.h"


class CDlgTPGeneralSmartValves : public CDlgCtrlPropPage
{
	DECLARE_DYNAMIC( CDlgTPGeneralSmartValves )

public:
	enum { IDD = IDD_TABDIALOGTECHSMARTVALVES };

	CDlgTPGeneralSmartValves( CWnd *pParent = NULL );
	
	// Overriding 'CDlgCtrlPropPage' methods.
	virtual void Init( bool bResetToDefault = false );
	virtual void Save( CString strSectionName );
	virtual bool VerifyInfo();

// Protected methods.
protected:
	virtual void DoDataExchange( CDataExchange *pDX );
	virtual BOOL OnInitDialog();

// Protected variables.
protected:
	CTADatastruct *m_pTADS;
	
	CExtNumEdit	m_EditDpMaxForBestSmartValveSuggestion;
	double m_dDpMaxForBestSmartValveSuggestion;
};
