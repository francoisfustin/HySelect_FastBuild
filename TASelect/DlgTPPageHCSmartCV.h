#pragma once

#include "ExtComboBox.h"
#include "DlgCtrlPropPage.h"

class CDlgTPPageHCSmartCV : public CDlgCtrlPropPage
{
	DECLARE_DYNAMIC( CDlgTPPageHCSmartCV )

public:
	enum { IDD = IDD_TABDIALOGTECHHCSMART };

	CDlgTPPageHCSmartCV( CWnd *pParent = NULL );

	// Overriding 'CDlgCtrlPropPage' methods.
	virtual void Init( bool bResetToDefault = false );
	virtual void Save( CString strSectionName );

	// Protected methods.
protected:
	DECLARE_MESSAGE_MAP()
	virtual void DoDataExchange( CDataExchange *pDX );
	virtual BOOL OnInitDialog();
	afx_msg void OnCbnSelChangeLocation();

	void FillComboBoxSMCVLocation();

	// Protected variables.
protected:
	CExtNumEditComboBox m_ComboBoxLocation;
	int m_iLocation;
};
