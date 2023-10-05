#pragma once


#include "ExtComboBox.h"
#include "DlgCtrlPropPage.h"

class CDlgTPPageHCActuators : public CDlgCtrlPropPage
{
	DECLARE_DYNAMIC( CDlgTPPageHCActuators )

public:
	enum { IDD = IDD_TABDIALOGTECHHCACTUATORS };

	CDlgTPPageHCActuators( CWnd* pParent = NULL );
	
	// Overriding 'CDlgCtrlPropPage' methods.
	virtual void Init( bool fResetToDefault = false );
	virtual void Save( CString strSectionName );

protected:
	DECLARE_MESSAGE_MAP()
	virtual void DoDataExchange( CDataExchange* pDX );
	virtual BOOL OnInitDialog();
	afx_msg void OnBnClickedAutomaticSelect();
	afx_msg void OnBnClickedSelectByPackage();
	afx_msg void OnCbnSelChangePowerSupply();
	afx_msg void OnCbnSelChangeInputSafe();
	afx_msg void OnCbnSelChangeDRP();
	afx_msg void OnBnClickedFailSafe();

	void FillComboBoxPowerSupply();
	void FillComboBoxInputSignal();
	void FillComboBoxDRP();
	void SetCheckFailSafe( bool bKeepValueIfMatched = false );
	void FillComboBox( CRank* pList, CComboBox* pCCombo, int iCount, CString* pCstr );

// Protected variables.
protected:
	CTADatastruct*	m_pTADS;

	CButton			m_CheckAutomaticSelect;
	CButton			m_CheckSelectByPackage;
	CButton			m_CheckFailSafeFct;
	CExtNumEditComboBox	m_ComboPowerSupply;
	CExtNumEditComboBox	m_ComboInputSignal;
	CExtNumEditComboBox	m_ComboDRP;

	bool			m_fAutomaticSelect;
	bool			m_fSelectByPackage;
	CString			m_strPowerSupply;
	CString			m_strInputSignal;
	int m_iFailSafeFct;
	CDB_ControlValve::DRPFunction m_eDefaultReturnPosFct;
};
