#pragma once


#include "XGroupBox.h"
#include "DlgCtrlPropPage.h"

class CDlgTPGeneral : public CDlgCtrlPropPage
{
	DECLARE_DYNAMIC( CDlgTPGeneral )

public:
	enum { IDD = IDD_TABDIALOGTECHDEVSIZE };

	CDlgTPGeneral( CWnd* pParent = NULL );

	// Overriding 'CDlgCtrlPropPage' methods.
	virtual void Init( bool fResetToDefault = false );
	virtual void Save( CString strSectionName );

// Protected methods.
protected:
	virtual void DoDataExchange( CDataExchange* pDX );
	virtual BOOL OnInitDialog();

// Protected variables.
protected:
	CButton			m_RadioTAImg;
	CButton			m_RadioHVACSymb;
	CButton			m_RadioLocSymb;
	CXGroupBox		m_GroupSchemeChoice;
};
