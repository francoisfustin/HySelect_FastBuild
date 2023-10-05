#pragma once


#include "ExtComboBox.h"
#include "DlgCtrlPropPage.h"

class CDlgTPPageHCPipes : public CDlgCtrlPropPage
{
	DECLARE_DYNAMIC( CDlgTPPageHCPipes )

public:
	enum { IDD = IDD_TABDIALOGTECHHCPIPES };

	CDlgTPPageHCPipes( CWnd *pParent = NULL );

	// Overriding 'CDlgCtrlPropPage' methods.
	virtual void Init( bool bResetToDefault = false );
	virtual void Save( CString strSectionName );
	virtual bool VerifyInfo();
	
	void RestorePrjParams();

protected:
	DECLARE_MESSAGE_MAP()
	virtual void DoDataExchange( CDataExchange *pDX );
	virtual BOOL OnInitDialog();
	afx_msg void OnCbnSelChangeCircPipe();
	afx_msg void OnCbnSelChangeDistSupplyPipe();
	afx_msg void OnCbnSelChangeDistReturnPipe();
	afx_msg void OnEnKillFocusSafetyFactor();

	void FillComboBoxCircPipe();
	void FillComboBoxDistSupplyPipe();
	void FillComboBoxDistReturnPipe();
	void FillComboBox( CRank * List, CExtNumEditComboBox *pclComboBox, int iCount, CString *pCstr = NULL );

// Protected variables.
protected:
	CTADatastruct *m_pTADS;

	CExtNumEditComboBox m_ComboCircPipe;
	CExtNumEditComboBox m_ComboDistSupplyPipe;
	CExtNumEditComboBox m_ComboDistReturnPipe;
	CExtNumEdit m_EditSafetyFactor;

	CString m_strCircPipe;
	CString m_strDistSupplyPipe;
	CString m_strDistReturnPipe;
	double m_dSafetyFactor;
};
