#pragma once


#include "ExtComboBox.h"

class CMyToolTipCtrl;
class CDlgPasteHM : public CDialogEx
{
public:
	enum { IDD = IDD_DLGPASTEHM };

	CDlgPasteHM( CWnd *pParent = NULL );

	// pHM the copied Hydraulic module pointer.
	// pHMTo the destination pointer .
	int Display( CDS_HydroMod *pHM, CTable **pTabTo, int *pIndex );

// Implementation
protected:
	DECLARE_MESSAGE_MAP()
	virtual void DoDataExchange( CDataExchange *pDX );
	virtual BOOL OnInitDialog();
	virtual void OnOK();
	afx_msg void OnRadioClickedInsertInPosition();
	afx_msg void OnRadioClickedPartnerValve();
	afx_msg void OnCbnSelChangePartnerValve();

// Protected variables.
protected:
	CTADatastruct *m_pTADS;
	CDS_HydroMod *m_pHM;
	CTable **m_pTabTo;
	CMyToolTipCtrl m_ToolTip;
	int *m_pIndex;

	CSpinButtonCtrl m_Spin;
	CEdit m_EditPos;
	CExtNumEditComboBox m_Combo;
};