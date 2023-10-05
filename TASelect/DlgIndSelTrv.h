#pragma once


#include "Global.h"
#include "afxcmn.h"
#include "ExtNumEdit.h"
#include "XGroupBox.h"
#include "ExtComboBox.h"
#include "afxwin.h"


class CRViewSSelTrv;
class CDlgSizeRad;
class CDlgIndSelTrv : public CDlgIndSelBase
{
public:
	enum { IDD = IDD_DLGINDSELTRV };
	
	enum SelectionOptimization
	{
		Selection_NoOptimization = 0,
		Selection_Optimization1
	};

	// Remark: These values correspond to the radios order in the dialog. Make sure that the tab order starts well first on the 
	//         'Thermostatic head' and goes after on the 'Electro actuator' radio. Otherwise the values below are no more valid.
	enum RadioThermoOrElectroActuator
	{
		Radio_ThermostaticHead = 0,
		Radio_ElectroActuator = 1
	};

	CDlgIndSelTrv( CWnd *pParent = NULL );
	~CDlgIndSelTrv();

	////////////////////////////////////////////////////////////////////////////////////////////////////////////////
	// Overrides 'CDlgSelectionBase' public virtual methods.
	virtual void SaveSelectionParameters();

	// Called by 'CDlgLeftTabSelManager' when user change application type.
	virtual void ChangeApplicationType( ProjectType eProductSelectionApplicationType );
	// End of overriding 'CDlgSelectionBase' public virtual methods.
	////////////////////////////////////////////////////////////////////////////////////////////////////////////////

protected:
	virtual void DoDataExchange( CDataExchange *pDX );
	virtual BOOL OnInitDialog();
	virtual void OnDisplay() { GetDlgItem( IDC_BUTTONSUGGEST )->SetFocus(); }

	DECLARE_MESSAGE_MAP()
	afx_msg void OnBnClickedCheckGroupDp();
	afx_msg void OnButtonComputeRadiatorData();
	afx_msg void OnCbnSelChangeRadiatorValveType();
	afx_msg void OnCbnSelChangeSVFamily();
	afx_msg void OnCbnSelChangeSVConnect();
	afx_msg void OnCbnSelChangeSVVersion();
	afx_msg void OnCbnSelChangeRVFamily();
	afx_msg void OnCbnSelChangeRVConnect();
	afx_msg void OnCbnSelChangeRVVersion();
	afx_msg void OnCbnSelChangeSVInsertName();
	afx_msg void OnCbnSelChangeSVInsertFamily();
	afx_msg void OnCbnSelChangeSVInsertType();
	afx_msg void OnCbnSelChangeRVInsertFamily();
	afx_msg void OnCbnSelChangeActuatorType();
	afx_msg void OnCbnSelChangeActuatorFamily();
	afx_msg void OnCbnSelChangeActuatorVersion();
	afx_msg void OnChangeEditInsertKv();
	afx_msg void OnKillFocusEditInsertKv();
	afx_msg void OnBnClickedRadioActuatorType();
	afx_msg LRESULT OnModifySelectedTrv( WPARAM wParam, LPARAM lParam );

	////////////////////////////////////////////////////////////////////////////////////////////////////////////////
	// Overrides 'CDlgIndSelBase' protected afx_msg methods.
	virtual afx_msg void OnBnClickedSuggest();
	virtual afx_msg void OnBnClickedSelect();
	virtual afx_msg void OnEditEnterChar( NMHDR *pNMHDR, LRESULT *pResult );
	virtual afx_msg void OnEnChangeFlow();
	virtual afx_msg void OnEnChangeDp();
	// HYS-1263 : Added to update application type if it is necessary
	virtual void ActivateLeftTabDialog();
	// End of overriding 'CDlgIndSelBase' protected afx_msg methods.
	////////////////////////////////////////////////////////////////////////////////////////////////////////////////

	////////////////////////////////////////////////////////////////////////////////////////////////////////////////
	// Overrides 'CDlgSelectionBase' protected afx_msg methods.
	virtual afx_msg LRESULT OnNewDocument( WPARAM wParam = 0, LPARAM lParam = 0);
	virtual afx_msg LRESULT OnTechParChange( WPARAM wParam = 0, LPARAM lParam = 0);
	virtual afx_msg LRESULT OnUnitChange( WPARAM wParam = 0, LPARAM lParam = 0);
	virtual afx_msg LRESULT OnPipeChange( WPARAM wParam = 0, LPARAM lParam = 0);
	virtual afx_msg LRESULT OnWaterChange( WPARAM wParam = 0, LPARAM lParam = 0);
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
	virtual CStatic *GetpStaticDT() const { return ( (CStatic *)GetDlgItem( IDC_STATICDT ) ); }
	virtual CStatic *GetpStaticQUnit() const { return ( (CStatic *)GetDlgItem( IDC_STATICQUNIT ) ); }
	virtual CStatic *GetpStaticPUnit() const { return ( (CStatic *)GetDlgItem( IDC_STATICPUNIT ) ); }
	virtual CStatic *GetpStaticDTUnit() const { return ( (CStatic *)GetDlgItem( IDC_STATICDTUNIT ) ); }
	virtual CButton *GetpBtnSuggest() const { return ( (CButton *)GetDlgItem( IDC_BUTTONSUGGEST ) ); }
	virtual CButton *GetpBtnCancelModif() const { return ( (CButton *)GetDlgItem( IDC_BUTTONCANCEL ) ); }
	virtual CXGroupBox *GetpGroupDp() const { return ( (CXGroupBox *)GetDlgItem( IDC_GROUPDP ) ); }
	virtual CXGroupBox *GetpGroupQ() const { return ( (CXGroupBox *)GetDlgItem( IDC_GROUPQ ) ); }
	// End of overriding 'CDlgIndSelBase' protected methods.
	////////////////////////////////////////////////////////////////////////////////////////////////////////////////

	////////////////////////////////////////////////////////////////////////////////////////////////////////////////
	// Overrides 'CDlgIndSelBase' protected pure virtual methods.
	virtual CRViewSSelSS *GetLinkedRightViewSSel( void );
	// End of overriding 'CDlgIndSelBase' protected pure virtual methods.
	////////////////////////////////////////////////////////////////////////////////////////////////////////////////

	////////////////////////////////////////////////////////////////////////////////////////////////////////////////
	// Overrides 'CDlgSelectionBase' protected methods.
	virtual void ClearAll( void );
	// End of overriding 'CDlgSelectionBase' protected methods.
	////////////////////////////////////////////////////////////////////////////////////////////////////////////////

// Private members.
private:
	// Fill thermostatic radiator valve type combo.
	void _FillComboRadiatorValveType( RadiatorValveType eRadiatorValveType = RadiatorValveType::RVT_Standard );
	
	// Fill supply and return valve ComboBox.
	void _FillComboSVFamily( CString strFamilyID = _T("") );
	void _FillComboSVConnect( CString strConnectID = _T("") );
	void _FillComboSVVersion( CString strVersionID = _T("") );
	void _FillComboRVFamily( CString strFamilyID = _T("") );
	void _FillComboRVConnect( CString strConnectID = _T("") );
	void _FillComboRVVersion( CString strVersionID = _T("") );

	// Fill supply and return valve insert ComboBox.
	void _FillComboSVInsertName( CString strInsertName = _T("") );
	void _FillComboSVInsertFamily( CString strFamilyID = _T("") );
	void _FillComboSVInsertType( CString strTypeID = _T("") );
	void _FillComboRVInsertFamily( CString strFamilyID = _T("") );

	// Fill actuator ComboBox.
	void _FillComboActuatorType( CString strTypeID = _T("") );
	void _FillComboActuatorFamily( CString strFamilyID = _T("") );
	void _FillComboActuatorVersion( CString strVersionID = _T("") );

	// Check if we can launch selection.
	// Return 'false' if it's not possible.
	bool _VerifySubmittedValues();

	// Launch a suggestion procedure.
	void _LaunchSuggestion();

	// Launch a suggestion procedure, called from the 'No device trv found' dialog box.
	void _LaunchSuggestion( CString strFamilyID, CString strConnectID, CString strVersionID );

	// Retrieve the size shift from the technical parameters.
	void _GetSelSizeShift( int &iAbove, int &iBelow );

	void _UpdateLayout( void );
	void _ShowControl( CWnd *pControl, bool bShow );
	void _UpdateKvMin();

	// Allow to do selection from 'OnBnClickedButtonsuggest' with or without optimization.
	void _DoSelection( SelectionOptimization eSelectOpt, int &iSupplyValveFound, int &iReturnValveFound, bool *pbSizeShiftProblem );

	// Called when a change is done in the status of the 'Dp' group.
	void _UpdateGroupDp();

	// Called when a change is done in the 'Work with:' radio control.
	void _UpdateRadioThermoOrElectroActuator();

// Public variables.
public:
	CComboBox m_ComboRadiatorValveType;
	CExtNumEditComboBox m_ComboSVFamily;
	CExtNumEditComboBox m_ComboSVConnect;
	CExtNumEditComboBox m_ComboSVVersion;
	CExtNumEditComboBox m_ComboRVFamily;
	CExtNumEditComboBox m_ComboRVConnect;
	CExtNumEditComboBox m_ComboRVVersion;
	CComboBox m_ComboSVInsertName;
	CExtNumEditComboBox m_ComboSVInsertFamily;
	CExtNumEditComboBox m_ComboSVInsertType;
	CExtNumEditComboBox m_ComboRVInsertFamily;
	CExtNumEdit m_EditSVInsertKv;
	CExtNumEditComboBox m_ComboActuatorType;
	CExtNumEditComboBox m_ComboActuatorFamily;
	CExtNumEditComboBox m_ComboActuatorVersion;
	CButton m_ButtonCompRad;
	CXGroupBox m_GroupValve;
	CXGroupBox m_GroupThermoHead;

// Protected variables.
protected:
	CIndSelTRVParams m_clIndSelTRVParams;
	bool m_bDiffDpSV;							// Flag: true if different Dp on supply valve (thermostatic valve) are obtained.
	bool m_bDiffDpRV;							// Flag: true if different Dp on return valve are obtained.
	double m_dMaxDpSV;							// Max obtained Dp on supply valve (thermostatic valve).
	double m_dMaxDpRV;							// Max obtained Dp on return valve.
	CString m_strKvsOrCv;						// String for 'Other Kv' or 'Other Cv' selection into the m_ComboSVInsertName.
	int m_iRadioThermoOrElectroActuator;
};
