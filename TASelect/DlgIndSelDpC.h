#pragma once


#include "afxcmn.h"
#include "XGroupBox.h"
#include "ExtComboBox.h"
#include "ExtNumEdit.h"
#include "afxwin.h"

class CDlgInfoSSelDpC;
class CDlgIndSelDpC : public CDlgIndSelBase
{
public:
	enum { IDD = IDD_DLGINDSELDPC };

	CDlgIndSelDpC(CWnd *pParent = NULL );
	virtual ~CDlgIndSelDpC();

	void SetDpMax( double dDpMax );

	////////////////////////////////////////////////////////////////////////////////////////////////////////////////
	// Overrides 'CDlgSelectionBase' public virtual methods.
	virtual void SaveSelectionParameters();
	virtual void LeaveLeftTabDialog();
	virtual void ActivateLeftTabDialog();

	// Called by 'CDlgLeftTabSelManager' when user change application type.
	virtual void ChangeApplicationType( ProjectType eProductSelectionApplicationType );

	virtual CString GetDlgCaption(){ return TASApp.LoadLocalizedString( IDS_LEFTTABTITLE_DPC ); }
	virtual void ClearAll( void );
	// End of overriding 'CDlgSelectionBase' public virtual methods.
	////////////////////////////////////////////////////////////////////////////////////////////////////////////////

protected:
	virtual void DoDataExchange( CDataExchange *pDX );
	virtual BOOL OnInitDialog();
	// By default, set the focus on the Suggest button.
	virtual void OnDisplay() { GetDlgItem( IDC_BUTTONSUGGEST )->SetFocus(); }

	// Generated message map functions
	DECLARE_MESSAGE_MAP()
	virtual afx_msg void OnCbnSelChangeDpStab();
	virtual afx_msg void OnCbnSelChangeDpCLocation();
	virtual afx_msg void OnCbnSelChangeMvLocation();
	virtual afx_msg void OnCbnSelChangeType();
	virtual afx_msg void OnCbnSelChangeFamily();
	virtual afx_msg void OnCbnSelChangeMaterial();
	virtual afx_msg void OnCbnSelChangeConnect();
	virtual afx_msg void OnCbnSelChangeVersion();
	virtual afx_msg void OnCbnSelChangePN();
	virtual afx_msg void OnCbnSelChangeKv();
	virtual afx_msg void OnEditChangeComboKv();
	virtual afx_msg void OnKillFocusComboKv();

	virtual afx_msg void OnBnClickedCheckGroupDpBranchOrKv();
	virtual afx_msg void OnBnClickedCheckDpMax();
	virtual afx_msg void OnBnClickedCheckShowInfoWnd();
	virtual afx_msg void OnBnClickedCheckboxSet();
	
	virtual afx_msg LRESULT OnModifySelectedDpC( WPARAM wParam = 0, LPARAM lParam = 0 );
	virtual afx_msg LRESULT OnDestroyInfoDialogDpC( WPARAM wParam = 0, LPARAM lParam = 0 );

	////////////////////////////////////////////////////////////////////////////////////////////////////////////////
	// Overrides 'CDlgIndSelBase' protected afx_msg methods.
	virtual afx_msg void OnBnClickedSuggest();
	virtual afx_msg void OnBnClickedSelect();
	virtual afx_msg void OnEnChangeFlow();
	virtual afx_msg void OnEnKillFocusFlow();
	virtual afx_msg void OnEnSetFocusFlow();
	virtual afx_msg void OnEnSetFocusPower();
	virtual afx_msg void OnEditEnterChar( NMHDR *pNMHDR, LRESULT *pResult );
	// End of overriding 'CDlgIndSelBase' protected afx_msg methods.
	////////////////////////////////////////////////////////////////////////////////////////////////////////////////

	virtual afx_msg void OnEnChangeDpBranch();
	virtual afx_msg void OnEnKillFocusDpBranch();
	virtual afx_msg void OnEnChangeDpMax();
	virtual afx_msg void OnEnKillFocusDpMax();

	////////////////////////////////////////////////////////////////////////////////////////////////////////////////
	// Overrides 'CDlgSelectionBase' protected afx_msg methods.
	virtual afx_msg void OnDestroy();
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
	virtual CButton *GetpRadioFlow() const { return ( (CButton *)GetDlgItem( IDC_RADIOFLOW ) ); }
	virtual CButton *GetpRadioPower() const { return ( (CButton *)GetDlgItem( IDC_RADIOPOWER ) ); }
	virtual CStatic *GetpStaticDT() const { return ( (CStatic *)GetDlgItem( IDC_STATICDT ) ); }
	virtual CStatic *GetpStaticQUnit() const { return ( (CStatic *)GetDlgItem( IDC_STATICQUNIT ) ); }
	virtual CStatic *GetpStaticPUnit() const { return ( (CStatic *)GetDlgItem( IDC_STATICPUNIT ) ); }
	virtual CStatic *GetpStaticDTUnit() const { return ( (CStatic *)GetDlgItem( IDC_STATICDTUNIT ) ); }
	virtual CButton *GetpBtnSuggest() const { return ( (CButton *)GetDlgItem( IDC_BUTTONSUGGEST ) ); }
	virtual CButton *GetpBtnCancelModif() const { return ( (CButton *)GetDlgItem( IDC_BUTTONCANCEL ) ); }
	// End of overriding 'CDlgIndSelBase' protected methods.
	////////////////////////////////////////////////////////////////////////////////////////////////////////////////

	////////////////////////////////////////////////////////////////////////////////////////////////////////////////
	// Overrides 'CDlgIndSelBase' protected pure virtual methods.
	virtual CRViewSSelSS *GetLinkedRightViewSSel( void );
	// End of overriding 'CDlgIndSelBase' protected pure virtual methods.
	////////////////////////////////////////////////////////////////////////////////////////////////////////////////

// Private methods.
private:
	void _FillComboDpStab( eDpStab eDpStabilizationOn );
	void _FillComboDpCLoc( eDpCLoc eDpCLocation );
	void _FillComboMvLoc( eMvLoc eMvLocation );
	void _FillComboType( CString strTypeID = _T("") );
	void _FillComboFamily( CString strFamilyID = _T("") );
	void _FillComboConnect( CString strConnectID = _T("") );
	void _FillComboBodyMat( CString strMaterialID = _T("") );
	void _FillComboVersion( CString strVersionID = _T("") );
	void _FillComboPN( CString strPNID = _T("") );
	void _FillComboKv( double dSelectKv = 0.0 );

	bool _UpdateMembers();

	// Launch a suggestion procedure.
	void _LaunchSuggestion( CString strFamilyID, CString strMaterialID, CString strConnectID, CString strVersionID, CString strPNID );
	
	// Launch a suggestion procedure, specifying the new MvLoc.
	void _LaunchSuggestion( int iMvLoc );
	
	// Launch a suggestion procedure, specifying the new connection, type and PN.
	void _LaunchSuggestion( CString strTypeID, CString strConnectID, CString strPNID );

	void _TryAlternativeSuggestion( int iDevFound, bool bValidFound );
	
	// Retrieve the size shift from the technical parameters.
	void _GetSelSizeShift( int &iAbove, int &iBelow );
	
	// Find out the Kvs bounds by sweeping over acceptable Dp controllers depending on the flow and by taking the best of their lower and 
	// higher Dpl bounds.
	bool _FindKvsBounds( double dFlow, double *pdLowestKvs, double *pdHighestKvs, double *pdLowestDpl, double *pdHighestDpl );
	
	// Prepare the Kvs range string according to the currently typed flow.
	// Uses FindKvsBounds() to determine the Kvs bounds.
	CString _GetKvsBoundsString();

	// Called when user changes the Dp stabilization type (On a branch or on a control valve).
	// We need in this case to switch between Dp or Kv.
	void _UpdateDpStabilizedOn();

	// Called when user clicks on the checkbox of the 'DpBranchOrKv' group.
	void _UpdateGroupDpBranchOrKvChecked();

	// Called when user changes the DpC location.
	void _UpdateDpCLocation();

	// Called when user changes the Mv location.
	void _UpdateMvLocation();

	// Called when we need to update the value of the 'Dp branch' edit field.
	void _UpdateDpBranchFieldValue();

	// Called when we need to update the state of the 'Dp branch' edit field and unit.
	void _UpdateDpBranchFieldState();

	// Called when we need to update to Kv range near the combo.
	void _UpdateStaticKvRange();

	// Called when we need to update the state of the 'Dp max' edit field and unit.
	void _UpdateDpMaxFieldState();

// Protected variables
protected:
	CIndSelDpCParams m_clIndSelDpCParams;
	
	CExtNumEditComboBox m_ComboMvLoc;
	CExtNumEditComboBox m_ComboDpCLoc;
	CExtNumEditComboBox m_ComboDpStab;
	CExtNumEditComboBox m_ComboType;
	CExtNumEditComboBox m_ComboFamily;
	CExtNumEditComboBox m_ComboBdyMat;
	CExtNumEditComboBox m_ComboConnect;
	CExtNumEditComboBox m_ComboVersion;
	CExtNumEditComboBox m_ComboPN;
	CExtNumEditComboBox m_ComboKv;
	CXGroupBox m_GroupDpC;
	CXGroupBox m_GroupDpOrKv;
	CXGroupBox m_GroupDpMax;
	CXGroupBox m_GroupConnScheme;
	CButton m_CheckboxSet;
	CExtNumEdit m_clExtEditDpBranch;
	CStatic m_StaticKvRange;
	CStatic m_StaticDpBranchUnit;
	CButton m_CheckInfoWnd;
	CExtNumEdit m_clExtEditDpMax;

	CRect m_ComboKvRect;
	CDlgInfoSSelDpC *m_pInfoDialogDpC;
};
