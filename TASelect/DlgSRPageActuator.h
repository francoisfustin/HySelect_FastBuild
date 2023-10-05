#pragma once


#include "ExtComboBox.h"

class CDlgSRPageActuator : public CDlgCtrlPropPage
{
	DECLARE_DYNAMIC( CDlgSRPageActuator )

public:
	// To interact with 'CDlgSearchReplace'.
	class IDlgSRPageActuatorNotificationHandler
	{
	public:
		virtual void SRPageActuator_OnRadioRemoveClicked() = 0;
		virtual void SRPageActuator_OnRadioAddClicked() = 0;
		virtual void SRPageActuator_OnCheckRemoveForSet() = 0;
		virtual void SRPageActuator_OnReplacePopupHM( CDS_HydroMod* pHM ) = 0;
	};

	enum { IDD = IDD_TABDLGSEARCHACTUATOR };

	CDlgSRPageActuator( CWnd* pParent = NULL );
	virtual ~CDlgSRPageActuator();

	void SetNotificationHandler( IDlgSRPageActuatorNotificationHandler* pNotificationHandler );
	void SetHMCalcOrIndSel( int iRadioState ) { m_iHMCalcOrIndSel = iRadioState; }

	bool IsRadioRemoveChecked( void );
	bool IsRadioAddChecked( void );
	bool IsCheckRemoveForSet( void );
	bool IsCheckFailSafeFct( void );

	// Overrides 'CDlgCtrlPropPage'.
	virtual void Init( bool bRstToDefault = false );
	virtual bool OnBnClickedBtnStart( std::vector<LPARAM> *pvecParams );

// Protected methods.
protected:
	DECLARE_MESSAGE_MAP()
	virtual void DoDataExchange( CDataExchange* pDX );
	virtual BOOL OnInitDialog();
	afx_msg void OnRadioClickedRemoveActuator();
	afx_msg void OnRadioClickedAddActuator();
	afx_msg void OnBnClickedRemoveForSet();
	afx_msg void OnBnClickedFailSafe();
	afx_msg void OnCbnSelChangePowerSupply();
	afx_msg void OnCbnSelChangeInputSignal();
	afx_msg LRESULT OnReplacePopupHM( WPARAM wParam = 0, LPARAM lParam = 0 );

// Private methods.
private:
	void _FillComboPowerSupply( void );
	void _FillComboInputSignal( void );
	void _SetCheckFailSafe();
	void _FillComboDRPFct( void );

// Private variables.
private:
	IDlgSRPageActuatorNotificationHandler *m_pNotificationHandler;
	int m_iHMCalcOrIndSel;
	CButton m_CheckRemoveForSet;
	CExtNumEditComboBox m_ComboPowerSupply;
	CExtNumEditComboBox m_ComboInputSignal;
	CExtNumEditComboBox m_ComboDRPFct;
	CButton m_CheckDowngrade;
	CButton m_CheckForceSet;
	CButton m_CheckFailSafeFct;
	CString m_strPowerSupply;
	CString m_strInputSignal;
	bool m_bFailSafeFct;
	CDB_ControlValve::DRPFunction m_eDefaultReturnPosFct;
};
