#pragma once


#include "XGroupBox.h"
#include "ExtComboBox.h"
#include "afxwin.h"

class CDlgIndSelPICv : public CDlgIndSelCtrlBase
{
public:
	enum { IDD = IDD_DLGINDSELPICV };

	CDlgIndSelPICv( CWnd *pParent = NULL );
	virtual ~CDlgIndSelPICv();

	void SetDpMax( double dDpMax );

	////////////////////////////////////////////////////////////////////////////////////////////////////////////////
	// Overrides 'CDlgSelectionBase' public virtual methods.
	virtual void SaveSelectionParameters();
	
	// Called by 'CDlgLeftTabSelManager' when user change application type.
	virtual void ChangeApplicationType( ProjectType eProductSelectionApplicationType );

	virtual CString GetDlgCaption(){ return TASApp.LoadLocalizedString( IDS_LEFTTABTITLE_PIBCV ); }
	// End of overriding 'CDlgSelectionBase' public virtual methods.
	////////////////////////////////////////////////////////////////////////////////////////////////////////////////

protected:
	void SetCheckStatus( int iStatus ) { m_CheckStatus = iStatus; }

	virtual void DoDataExchange( CDataExchange* pDX );
	virtual BOOL OnInitDialog();

	DECLARE_MESSAGE_MAP()
	afx_msg void OnCbnSelChangeType();
	afx_msg void OnCbnSelChangeCtrlType();
	afx_msg void OnCbnSelChangeFamily();
	afx_msg void OnCbnSelChangeBodyMaterial();
	afx_msg void OnCbnSelChangeConnect();
	afx_msg void OnCbnSelChangeVersion();
	afx_msg void OnCbnSelChangePN();
	afx_msg void OnCbnSelChangePowerSupply();
	afx_msg void OnCbnSelChangeInputSignal();
	afx_msg void OnCbnSelChangeDRPFct();
	afx_msg void OnBnClickedCheckboxSet();
	afx_msg LRESULT OnModifySelectedPICV( WPARAM wParam, LPARAM lParam );

	afx_msg void OnBnClickedCheckGroupDpMax();
	afx_msg void OnEnChangeDpMax();
	afx_msg void OnEnKillFocusDpMax();

	afx_msg void OnBnClickedCheckFailSafe();

	////////////////////////////////////////////////////////////////////////////////////////////////////////////////
	// Overrides 'CDlgIndSelBaseCtrl' protected afx_msg methods.
	virtual afx_msg void OnBnClickedSuggest();
	virtual afx_msg void OnBnClickedSelect();
	virtual afx_msg void OnEditEnterChar( NMHDR* pNMHDR, LRESULT* pResult );
	// End of overriding 'CDlgIndSelBaseCtrl' protected afx_msg methods.
	////////////////////////////////////////////////////////////////////////////////////////////////////////////////

	////////////////////////////////////////////////////////////////////////////////////////////////////////////////
	// Overrides 'CDlgSelectionBase' protected afx_msg methods.
	virtual afx_msg LRESULT OnNewDocument( WPARAM wParam = 0, LPARAM lParam = 0 );
	virtual afx_msg LRESULT OnTechParChange( WPARAM wParam = 0, LPARAM lParam = 0 );
	virtual afx_msg LRESULT OnUnitChange( WPARAM wParam = 0, LPARAM lParam = 0 );
	virtual afx_msg LRESULT OnPipeChange( WPARAM wParam = 0, LPARAM lParam = 0 );
	virtual afx_msg LRESULT OnWaterChange( WPARAM wParam = 0, LPARAM lParam = 0 );
	// End of overriding 'CDlgSelectionBase' protected afx_msg methods.
	////////////////////////////////////////////////////////////////////////////////////////////////////////////////

	////////////////////////////////////////////////////////////////////////////////////////////////////////////////
	// Overrides 'CDlgIndSelBaseCtrl' protected methods.
	virtual CExtNumEdit *GetpEditFlow() const { return ( (CExtNumEdit*)GetDlgItem( IDC_EDITFLOW ) ); }
	virtual CExtNumEdit *GetpEditPower() const { return ( (CExtNumEdit*)GetDlgItem( IDC_EDITPOWER ) ); }
	virtual CExtNumEdit *GetpEditDT() const { return ( (CExtNumEdit*)GetDlgItem( IDC_EDITDT ) ); }
	virtual CExtNumEdit *GetpEditDp() const { return ( (CExtNumEdit*)GetDlgItem( IDC_EDITDP ) ); }
	virtual CButton *GetpRadioFlow() const { return ( (CButton*)GetDlgItem( IDC_RADIOFLOW ) ); }
	virtual CButton *GetpRadioPower() const { return ( (CButton*)GetDlgItem( IDC_RADIOPOWER ) ); }
	virtual CStatic *GetpStaticDT() const { return ( (CStatic*)GetDlgItem( IDC_STATICDT ) ); }
	virtual CStatic *GetpStaticQUnit() const { return ( (CStatic*)GetDlgItem( IDC_STATICQUNIT ) ); }
	virtual CStatic *GetpStaticPUnit() const { return ( (CStatic*)GetDlgItem( IDC_STATICPUNIT ) ); }
	virtual CStatic *GetpStaticDTUnit() const { return ( (CStatic*)GetDlgItem( IDC_STATICDTUNIT ) ); }
	virtual CButton *GetpBtnSuggest() const { return ( (CButton*)GetDlgItem( IDC_BUTTONSUGGEST ) ); }
	virtual CButton *GetpBtnCancelModif() const { return ( (CButton*)GetDlgItem( IDC_BUTTONCANCEL ) ); }
	// End of overriding 'CDlgIndSelBaseCtrl' protected methods.
	////////////////////////////////////////////////////////////////////////////////////////////////////////////////

	////////////////////////////////////////////////////////////////////////////////////////////////////////////////
	// Overrides 'CDlgIndSelBaseCtrl' protected pure virtual methods.
	virtual CRViewSSelSS *GetLinkedRightViewSSel( void );
	// End of overriding 'CDlgIndSelBaseCtrl' protected pure virtual methods.
	////////////////////////////////////////////////////////////////////////////////////////////////////////////////

// Private methods.
private:
	void _FillComboPowerSupply( CString strPowerSupplyID = _T("") );
	void _FillComboInputSignal( CString strInputSignalID = _T("") );
	void _SetCheckFailSafeFct();
	void _FillComboDRPFct( CDB_ControlValve::DRPFunction eDRPFct = CDB_ControlValve::DRPFunction::drpfUndefined );
	
	// Launch a suggestion procedure, called from the No Device Found dialog box.
	void _LaunchSuggestion( CString strTypeID, CDB_ControlProperties::CvCtrlType eCvCtrlType, CString strFamilyID, CString strMaterialID, 
			CString strConnectID, CString strVersionID, CString strPNID, bool bSelectionBySet );
	
	// If the selection made by the user doesn't work, try to make an alternative selection.
	// The function return true if the user accept to change the criterion.
	bool _TryAlternativeSuggestion( bool bSizeShiftProblem );

	// Verify if we can enable/disable this button and if we can check/uncheck it.
	void _VerifyCheckboxSetStatus( void );

	// HYS-1079: Verify if we can enable/disable this button and if we can check/uncheck it.
	void _VerifyCheckboxFailsafeStatus();

	// Called when we need to update the state of the 'Dp max' edit field and unit.
	void _UpdateDpMaxFieldState();

	// HYS-1877: We will not consider PIBCV that belong to a 6-way valve set
	void _FilterPIBCVSetFor6WayControlValve( CRankEx *pclRankList );

// Private variables.
private:
	CIndSelPIBCVParams m_clIndSelPIBCVParams;
	CDlgSelComboHelperPIBCV *m_pclSelComboHelperPIBCV;

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
	CButton m_CheckboxSet;
	CXGroupBox m_GroupValveType;
	CXGroupBox m_GroupValve;
	CXGroupBox m_GroupActuator;
	CXGroupBox m_GroupDpMax;
	CExtNumEdit m_clExtEditDpMax;
	CButton m_CheckboxFailSafe;
	int m_CheckStatus;
};
