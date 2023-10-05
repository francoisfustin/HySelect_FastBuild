#pragma once


#include "ExtComboBox.h"
#include "XGroupBox.h"
#include "afxwin.h"

class CDlgIndSelCv : public CDlgIndSelCtrlBase
{
public:
	enum{ IDD = IDD_DLGINDSELCV };

	// Remark: These values correspond to the radios order in the dialog. Make sure that the tab order starts well first on the 'Flow'
	//         and goes after on the 'Power' radio. Otherwise the values below are no more valid.
	enum RadioKvsOrDp
	{
		Radio_Kvs	= 0,
		Radio_Dp = 1
	};

	CDlgIndSelCv( CWnd *pParent = NULL );
	virtual ~CDlgIndSelCv() {}
	
	////////////////////////////////////////////////////////////////////////////////////////////////////////////////
	// Overrides 'CDlgSelectionBase' public virtual methods.
	virtual void SaveSelectionParameters();

	// Called by 'CDlgLeftTabSelManager' when user change application type.
	virtual void ChangeApplicationType( ProjectType eProductSelectionApplicationType );

	virtual CString GetDlgCaption(){ return TASApp.LoadLocalizedString(IDS_LEFTTABTITLE_CV); }
	// End of overriding 'CDlgSelectionBase' public virtual methods.
	////////////////////////////////////////////////////////////////////////////////////////////////////////////////

protected:

	void SetCheckStatus( int iStatus ) { m_CheckStatus = iStatus; }

	virtual void DoDataExchange( CDataExchange *pDX );
	virtual BOOL OnInitDialog();

	DECLARE_MESSAGE_MAP()
	afx_msg void OnCbnSelChange2w3w();
	afx_msg void OnCbnSelChangeType();
	afx_msg void OnCbnSelChangeCtrlType();
	afx_msg void OnCbnSelChangeFamily();
	afx_msg void OnCbnSelChangeMaterial();
	afx_msg void OnCbnSelChangeConnect();
	afx_msg void OnCbnSelChangeVersion();
	afx_msg void OnCbnSelChangePN();
	afx_msg void OnCbnSelChangeKvs();
	afx_msg void OnCbnEditChangeKvs();
	afx_msg void OnBnClickedCheckGroupKvsOrDp();
	afx_msg void OnBnClickedRadioKvsOrDp();
	afx_msg void OnCbnSelChangePowerSupply();
	afx_msg void OnCbnSelChangeInputSignal();
	afx_msg void OnCbnSelChangeDRPFct();
	afx_msg void OnBnClickedCheckboxSet();
	afx_msg void OnBnClickedCheckFailSafe();
	afx_msg LRESULT OnModifySelectedCV( WPARAM wParam, LPARAM lParam );

	////////////////////////////////////////////////////////////////////////////////////////////////////////////////
	// Overrides 'CDlgIndSelBase' protected afx_msg methods.
	virtual afx_msg void OnBnClickedSuggest();
	virtual afx_msg void OnBnClickedSelect();
	virtual afx_msg void OnEnChangeDp();
	virtual afx_msg void OnEnKillFocusFlow();
	// End of overriding 'CDlgIndSelBase' protected afx_msg methods.
	////////////////////////////////////////////////////////////////////////////////////////////////////////////////

	////////////////////////////////////////////////////////////////////////////////////////////////////////////////
	// Overrides 'CDlgSelectionBase' protected afx_msg methods.
	virtual afx_msg LRESULT OnNewDocument( WPARAM wParam = 0, LPARAM lParam = 0 );
	virtual afx_msg LRESULT OnPipeChange( WPARAM wParam = 0, LPARAM lParam = 0 );
	virtual afx_msg LRESULT OnTechParChange( WPARAM wParam = 0, LPARAM lParam = 0 );
	virtual afx_msg LRESULT OnUnitChange( WPARAM wParam = 0, LPARAM lParam = 0 );
	virtual afx_msg LRESULT OnWaterChange( WPARAM wParam = 0, LPARAM lParam = 0 );
	// End of overriding 'CDlgSelectionBase' protected afx_msg methods.
	////////////////////////////////////////////////////////////////////////////////////////////////////////////////

	////////////////////////////////////////////////////////////////////////////////////////////////////////////////
	// Overrides 'CDlgIndSelBase' protected methods.
	virtual CExtNumEdit *GetpEditFlow() const { return ( (CExtNumEdit *)GetDlgItem( IDC_EDITFLOW ) ); }
	virtual CExtNumEdit *GetpEditPower() const { return ( (CExtNumEdit *)GetDlgItem( IDC_EDITPOWER ) ); }
	virtual CExtNumEdit *GetpEditDT() const { return ( (CExtNumEdit *)GetDlgItem( IDC_EDITDT ) ); }
	virtual CExtNumEdit *GetpEditDp() const { return ( (CExtNumEdit *)GetDlgItem( IDC_EDITDP ) ); }
	virtual CButton *GetpRadioFlow() const { return ( (CButton *)GetDlgItem( IDC_RADIOFLOW ) ); }
	virtual CButton *GetpRadioPower() const { return ( (CButton *)GetDlgItem( IDC_RADIOPOWER ) ); }
	virtual CComboBoxEx *GetpComboKvs() const { return ( (CComboBoxEx *)GetDlgItem( IDC_COMBOKVS ) ); }
	virtual CStatic *GetpStaticDT() const { return ( (CStatic *)GetDlgItem( IDC_STATICDT ) ); }
	virtual CStatic *GetpStaticQUnit() const { return ( (CStatic *)GetDlgItem( IDC_STATICQUNIT ) ); }
	virtual CStatic *GetpStaticPUnit() const { return ( (CStatic *)GetDlgItem( IDC_STATICPUNIT ) ); }
	virtual CStatic *GetpStaticDTUnit() const { return ( (CStatic *)GetDlgItem( IDC_STATICDTUNIT ) ); }
	virtual CButton *GetpBtnSuggest() const { return ( (CButton *)GetDlgItem( IDC_BUTTONSUGGEST ) ); }
	virtual CButton *GetpBtnCancelModif() const { return ( (CButton *)GetDlgItem( IDC_BUTTONCANCEL ) ); }
	virtual void FillCombo2w3w( CDB_ControlProperties::CV2W3W eCv2w3w = CDB_ControlProperties::CV2W3W::CV2W );
	virtual void FillComboCtrlType( CDB_ControlProperties::CvCtrlType eCvCtrlType = CDB_ControlProperties::eCvProportional );
	// End of overriding 'CDlgIndSelBase' protected methods.
	////////////////////////////////////////////////////////////////////////////////////////////////////////////////

	////////////////////////////////////////////////////////////////////////////////////////////////////////////////
	// Overrides 'CDlgIndSelBase' protected pure virtual methods.
	virtual CRViewSSelSS *GetLinkedRightViewSSel( void );
	// End of overriding 'CDlgIndSelBase' protected pure virtual methods.
	////////////////////////////////////////////////////////////////////////////////////////////////////////////////

// Private methods.
private:
	void _FillComboType( CString strTypeID = _T("") );
	void _FillComboFamily( CString strFamilyID = _T("") );
	void _FillComboBodyMat( CString strBodyMaterialID = _T("") );
	void _FillComboConnect( CString strConnectID = _T("") );
	void _FillComboVersion( CString strVersionID = _T("") );
	void _FillComboPN( CString strPNID = _T("") );
	int _FillComboKvs( double dKvs = 0.0 );
	void _FillComboPowerSupply( CString strPowerSupplyID = _T("") );
	void _FillComboInputSignal( CString strInputSignalID = _T("") );
	void _SetCheckFailSafeFct();
	void _FillComboDRPFct( CDB_ControlValve::DRPFunction eDRPFct = CDB_ControlValve::DRPFunction::drpfUndefined );

	// Launch a suggestion procedure, called from the 'No device found' dialog box.
	void _LaunchSuggestion( CDB_ControlProperties::CvCtrlType eCvCtrlType, CString strFamilyID, CString strMaterialID, CString strConnectID, 
			CString strVersionID, CString strPNID, bool bSelectionBySet );
	
	// If the selection made by the user doesn't work, try to make an alternative selection.
	// The function return true if the user accept to change the criterion
	bool _TryAlternativeSuggestion( bool bSizeShiftProblem );

	// Verify if we can enable/disable this button and if we can check/uncheck it.
	void _VerifyCheckPackageStatus();

	// Refresh Kvs/Cv string in function of user selection.
	void _SetKvsCvString();

	void _UpdateGroupKvsOrDp();
	void _UpdateRadioKvsOrDp();
	// HYS-1079: Verify if we can enable/disable this button and if we can check/uncheck it.
	void _VerifyCheckboxFailsafeStatus();

// Private variables.
private:
	CIndSelCVParams m_clIndSelCVParams;

	CExtNumEditComboBox m_ComboKvs;
	CExtNumEditComboBox m_Combo2w3w;
	CExtNumEditComboBox m_ComboType;
	CExtNumEditComboBox m_ComboCtrlType;
	CExtNumEditComboBox m_ComboFamily;
	CExtNumEditComboBox m_ComboMaterial;
	CExtNumEditComboBox m_ComboConnect;
	CExtNumEditComboBox m_ComboVersion;
	CExtNumEditComboBox m_ComboPN;
	CExtNumEditComboBox m_ComboPowerSupply;
	CExtNumEditComboBox m_ComboInputSignal;
	CExtNumEditComboBox m_ComboDRPFct;
	
	int m_iRadioKvsOrDp;
	CButton m_CheckGroupDpKvs;

	CButton m_CheckboxSet;
	CXGroupBox m_GroupKvDp;
	CXGroupBox m_GroupValveType;
	CXGroupBox m_GroupValve;
	CXGroupBox m_GroupActuator;
	CButton m_CheckboxFailSafe;
	int m_CheckStatus;
};
