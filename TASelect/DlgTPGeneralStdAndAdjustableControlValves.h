#pragma once


#include "DlgCtrlPropPage.h"
#include "ExtNumEdit.h"
#include "afxwin.h"


class CDlgTPGeneralStdAndAdjustableControlValves : public CDlgCtrlPropPage
{
	DECLARE_DYNAMIC( CDlgTPGeneralStdAndAdjustableControlValves )

public:
	enum { IDD = IDD_TABDIALOGTECHSTDANDADJCTRLVALVE };

	CDlgTPGeneralStdAndAdjustableControlValves( CWnd *pParent = NULL );
	
	// Overriding 'CDlgCtrlPropPage' methods.
	virtual void Init( bool bResetToDefault = false );
	virtual void Save( CString strSectionName );
	virtual bool VerifyInfo();

// Protected methods.
protected:
	DECLARE_MESSAGE_MAP()
	virtual void DoDataExchange( CDataExchange *pDX );
	virtual BOOL OnInitDialog();

	afx_msg void OnEnKillFocusMinDpProportional();
	afx_msg void OnEnKillFocusMaxDpProportional();
	afx_msg void OnEnSetFocusMinDpProportional();
	afx_msg void OnEnSetFocusMaxDpProportional();
	afx_msg void OnBnClickedReynard();
	afx_msg void OnBnClickedCheckdpcpband();

// Protected variables.
protected:
	CTADatastruct *m_pTADS;
	
	CExtNumEdit m_EditMinAuthority;
	CExtNumEdit m_EditMinAuthCstFlow;
	CExtNumEdit m_EditMinDpOnOff;
	CExtNumEdit m_EditMinDpProportional;
	CExtNumEdit m_EditMaxDpProportional;
	CButton m_CheckReynard;
	CButton m_CheckDpCPBand;
	
	double m_dCvMinAuthority;
	double m_dCvMinAuthCstFlow;
	double m_dCvMinDpOnOff;
	double m_dCvMinDpProportional;
	double m_dCvMaxDpProportional;
	double m_dCvMaxDispDp;
	int m_iCvDefKvReynard;
	int m_iUseDpCPBand;
};
