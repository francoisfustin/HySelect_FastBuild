#pragma once


#include "DialogExt.h"
#include "ExtNumEdit.h"
#include "ExtStatic.h"
#include "MyToolTipCtrl.h"


class CDlgSolarContentMultiplierFactor : public CDialogExt
{
public:
	enum { IDD = IDD_DLGSOLARCONTENTMULTIPLIERFACTOR };
	CDlgSolarContentMultiplierFactor( CPMInputUser *pclPMInputUser, CWnd *pParent = NULL );
	virtual ~CDlgSolarContentMultiplierFactor() {}

protected:
	DECLARE_MESSAGE_MAP()
	virtual void DoDataExchange( CDataExchange *pDX );
	virtual BOOL OnInitDialog();
	virtual void OnOK();
	virtual void OnCancel();

	virtual afx_msg void OnBtnMultiplierFactorClicked();
	virtual afx_msg void OnEnSetFocusMultiplierFactor();
	virtual afx_msg void OnKillFocusMultiplierFactor();
	virtual afx_msg void OnEditEnterMultiplierFactor( NMHDR *pNMHDR, LRESULT *pResult );

// Protected variables.
protected:
	int m_iCheckMultiplierFactorSaved;
	double m_dEditMultiplierFactorSaved;

	CPMInputUser *m_pclPMInputUser;
	CExtStatic m_StaticMultiplierFactor;
	CButton m_BtnCheckMultiplierFactor;
	CExtNumEdit m_EditMultiplierFactor;
	double m_dMultiplierFactorSaved;

	CMyToolTipCtrl m_ToolTip;
};
