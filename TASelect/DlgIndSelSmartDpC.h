#pragma once


#include "DlgLeftTabBase.h"
#include "DlgIndSelBase.h"
#include "afxcmn.h"
#include "XGroupBox.h"
#include "ExtComboBox.h"
#include "ExtNumEdit.h"
#include "ProductSelectionParameters.h"

class CDlgIndSelSmartDpC : public CDlgIndSelBase
{
public:
	enum { IDD = IDD_DLGINDSELSMARTDPC };

	CDlgIndSelSmartDpC( CWnd *pParent = NULL );
	virtual ~CDlgIndSelSmartDpC() {}

	////////////////////////////////////////////////////////////////////////////////////////////////////////////////
	// Overrides 'CDlgSelectionBase' public virtual methods.
	virtual void SaveSelectionParameters();

	// Called by 'CDlgLeftTabSelManager' when user change application type.
	virtual void ChangeApplicationType( ProjectType eProductSelectionApplicationType );

	// End of overriding 'CDlgSelectionBase' public virtual methods.
	////////////////////////////////////////////////////////////////////////////////////////////////////////////////
	
	void SetDpMax( double dDpMax );

protected:
	virtual void DoDataExchange( CDataExchange *pDX );
	virtual BOOL OnInitDialog();

	DECLARE_MESSAGE_MAP()
	afx_msg void OnCbnSelChangeBodyMat();
	afx_msg void OnCbnSelChangeConnect();
	afx_msg void OnCbnSelChangePN();
	afx_msg void OnEnChangeDpBranch();
	afx_msg void OnEnKillFocusDpBranch();
	afx_msg void OnBnClickedCheckGroupDpBranch();
	afx_msg void OnBnClickedCheckGroupDpMax();
	afx_msg void OnEnChangeDpMax();
	afx_msg void OnEnKillFocusDpMax();
	//afx_msg void OnBnClickedOnlyForSet();

	afx_msg LRESULT OnModifySelectedSmartDpC( WPARAM wParam, LPARAM lParam );

	////////////////////////////////////////////////////////////////////////////////////////////////////////////////
	// Overrides 'CDlgIndSelBase' protected afx_msg methods.
	virtual afx_msg void OnBnClickedSuggest();
	virtual afx_msg void OnBnClickedSelect();
	virtual afx_msg void OnEditEnterChar( NMHDR *pNMHDR, LRESULT *pResult );
	virtual CString GetDlgCaption() { return TASApp.LoadLocalizedString( IDS_LEFTTABTITLE_SMARTDPC ); }
	// HYS-1263 : Added to update application type if it is necessary
	virtual void ActivateLeftTabDialog();
	// End of overriding 'CDlgIndSelBase' protected afx_msg methods.
	////////////////////////////////////////////////////////////////////////////////////////////////////////////////

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
	// Overrides 'CDlgIndSelBase' protected virtual methods.
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
	virtual CXGroupBox *GetpGroupQ() const { return ( (CXGroupBox *)GetDlgItem( IDC_GROUPQ ) ); }
	// End of overriding 'CDlgIndSelBase' protected virtual methods.
	////////////////////////////////////////////////////////////////////////////////////////////////////////////////

	////////////////////////////////////////////////////////////////////////////////////////////////////////////////
	// Overrides 'CDlgIndSelBase' protected pure virtual methods.
	virtual CRViewSSelSS *GetLinkedRightViewSSel( void );
	// End of overriding 'CDlgIndSelBase' protected pure virtual methods.
	////////////////////////////////////////////////////////////////////////////////////////////////////////////////

// Private methods.
private:
	bool _SuggestIndividualSelection();
	bool _SelectIndividualSelection();

	// Fill combos.
	void _FillComboBodyMat( CString strBodyMaterialID = _T( "" ) );
	void _FillComboConnect( CString strConnectID = _T( "" ) );
	void _FillComboPN( CString strPNID = _T( "" ) );

	// Launch a suggestion procedure, called from the 'No device found' dialog box.
	void _LaunchSuggestion( CString strMaterialID, CString strConnectID, CString strPNID );

	// Called when we need to update the value of the 'Dp branch' edit field.
	void _UpdateDpBranchFieldValue();

	// Called when we need to update the state of the 'Dp branch' edit field and unit.
	void _UpdateDpBranchFieldState();

	// Called when we need to update the state of the 'Dp max' edit field and unit.
	void _UpdateDpMaxFieldState();

	// Return 0 if one (or several) data is (are) invalid.
	bool _VerifySubmittedValues();

	// HYS-1938: TA-Smart Dp - 05 - Individual selection: right view
	// This function updates the checkbox button state according to the valves availability. 
	//void _UpdateCheckOnlyForSet();

	// Private variables.
private:
	CIndSelSmartDpCParams m_clIndSelSmartDpCParams;
	// HYS-1939: TA-Smart Dp - 06 - Individual selection: create the datastruct object.
    // Set type ID to identify TA-Smart and TA-smart-Dp
	CString m_strTypeID;

	CExtNumEditComboBox m_ComboBdyMat;
	CExtNumEditComboBox m_ComboConnect;
	CExtNumEditComboBox m_ComboPN;

	CXGroupBox m_GroupValve;
	CXGroupBox m_GroupDpBr;
	CXGroupBox m_GroupDpMax;

	//CButton m_CheckOnlyForSet;

	CExtNumEdit m_clExtEditDpBranch;
	CExtNumEdit m_clExtEditDpMax;
	CStatic m_StaticDpBranchUnit;
};
