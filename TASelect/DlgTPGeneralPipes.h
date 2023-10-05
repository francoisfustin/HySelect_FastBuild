#pragma once


#include "DlgCtrlPropPage.h"
#include "afxwin.h"
#include <ExtStatic.h>

class CDlgTPGeneralPipes : public CDlgCtrlPropPage
{
	DECLARE_DYNAMIC( CDlgTPGeneralPipes )

public:
	enum { IDD = IDD_TABDIALOGTECHPIPES };

	CDlgTPGeneralPipes( CWnd* pParent = NULL );

	// Overriding 'CDlgCtrlPropPage' methods.
	virtual void Init( bool fResetToDefault = false );
	virtual void Save( CString strSectionName );
	virtual bool VerifyInfo();

// Protected methods.
protected:
	DECLARE_MESSAGE_MAP()
	virtual void DoDataExchange( CDataExchange* pDX );
	virtual BOOL OnInitDialog();
	afx_msg void OnEnKillFocusTargetLinearDp();
	afx_msg void OnEnKillFocusMaxLinearDp();
	afx_msg void OnEnKillFocusTargetWaterVelocity();
	afx_msg void OnEnKillFocusMaxWaterVelocity();
	afx_msg void OnEnKillFocusMinWaterVelocity();
	afx_msg void OnEnSetFocusTargetLinearDp();
	afx_msg void OnEnSetFocusMaxLinearDp();
	afx_msg void OnEnSetFocusTargetWaterVelocity();
	afx_msg void OnEnSetFocusMaxWaterVelocity();
	afx_msg void OnEnSetFocusMinWaterVelocity();
	afx_msg void OnBnClickedTargetWaterVelocity();

// Protected variables.
protected:
	CTADatastruct*	m_pTADS;
	
	CExtNumEdit		m_EditTargetLinearDp;
	CExtNumEdit		m_EditMaxLinearDp;
	CButton			m_CheckTargetWaterVelocity;
	CExtNumEdit		m_EditTargetWaterVelocity;
	CExtNumEdit		m_EditMaxWaterVelocity;
	// HYS-1878
	CExtNumEdit		m_EditMinWaterVelocity;
	CExtStatic		m_StaticWarnMinVelValue;

	double			m_dTargetLinearDp;
	double			m_dMaxLinearDp;
	bool			m_fTargetWaterVelocity;
	double			m_dTargetWaterVelocity;
	double			m_dMaxWaterVelocity;
	// HYS-1878
	double			m_dMinWaterVelocity;
	CString         m_strWarnMinVelValue;
};