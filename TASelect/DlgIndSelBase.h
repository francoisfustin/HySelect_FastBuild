#pragma once


#include "ExtNumEdit.h"
#include "Select.h"
#include "HydroMod.h"
#include "XGroupBox.h"
#include "ExtComboBox.h"
#include "DlgHMCompilationOutput.h"
#include "ProductSelectionParameters.h"
#include "DlgSelectionBase.h"

class CMainFrame;
class CDlgIndSelBase : public CDlgSelectionBase
{
public:
	// Remark: These values correspond to the radios order in the dialog. Make sure that the tab order starts well first on the 'Flow'
	//         and goes after on the 'Power' radio. Otherwise the values below are no more valid.
	enum RadioFlowOrPowerDT
	{
		Radio_Flow	= 0,
		Radio_PowerDT = 1
	};

	CDlgIndSelBase( CIndividualSelectionParameters &clIndSelParam, UINT nID = 0, CWnd *pParent = NULL );

	virtual ~CDlgIndSelBase();

	void VerifyModificationMode( void );

	// Enable the Select button.
	void EnableSelectButton( bool fEnabled = true );

	void UpdateDpFieldValue( void );
	void UpdateFlowFieldValue( void );
	void UpdatePowerFieldValue( void );
	void UpdateDTFieldValue( void );

	void UpdateFlowOrPowerDTState( void );
	
	// Allow to update (enable/disable) the Dp edit field in regards to its current state.
	virtual void UpdateDpFieldState();

	// Return the list of products that will be displayed.
	virtual bool IsSelectionBySet( void ) { return false; }

	void Suggest( void ) { OnBnClickedSuggest(); }
	bool IsButtonSelectEnabled( void );
	
	void ClearCommonInputEditors( void );

	// Verify if Dp introduced by user is valid or not.
	// Return value: can be 'VDP_FlagDpError' (If error in Dp), 'VDP_FlagDpToDisable' if the Dp field must be disabled and 'VDP_FlagOK'
	// Remark: it is the caller that must disable or not the Dp field in regards to the 'm_pclIndSelParams->m_bDpEnabled' state.
	enum
	{
		VDPFlag_OK,
		VDPFlag_DpError,
		VDPFlag_DpFieldToDisable
	};
	int VerifyDpValue( void );

	////////////////////////////////////////////////////////////////////////////////////////////////////////////////
	// Overrides 'CDlgSelectionBase' public virtual methods.
	// Set the focus on the first available edit control.
	virtual	void SaveSelectionParameters( void );
	virtual void ActivateLeftTabDialog( void );

	// Called by 'CDlgLeftTabSelManager' when user change application type.
	virtual void ChangeApplicationType( ProjectType eProductSelectionApplicationType );

	virtual void OnRViewSSelSelectProduct( void );
	virtual void OnRViewSSelSelectionChanged( bool fSelectionActive );
	virtual void OnRViewSSelKeyboardEvent( int iKeyboardVirtualKey );
	virtual void ClearAll( void );
	// End of overriding 'CDlgSelectionBase' public virtual methods.
	////////////////////////////////////////////////////////////////////////////////////////////////////////////////

	////////////////////////////////////////////////////////////////////////////////////////////////////////////////
	// Overrides 'CDlgSelectionBase' public pure virtual methods.
	virtual bool ResetRightView( void );
	virtual CMainFrame::RightViewList GetRViewID( void );
	// End of overriding 'CDlgSelectionBase' public pure virtual methods.
	////////////////////////////////////////////////////////////////////////////////////////////////////////////////

	////////////////////////////////////////////////////////////////////////////////////////////////////////////////
	// Overrides 'DlgSelectionTabHelper' public virtual methods.
	//
	// This method is called when left tab dialog has no more the focus while user presses TAB or SHIFT+TAB.
	// Param: if 'fNext' is 'true' that means user presses 'TAB', otherwise it's 'SHIFT+TAB'.
	virtual bool LeftTabKillFocus( bool fNext );
	
	// End of overriding 'DlgSelectionTabHelper' public virtual methods.
	////////////////////////////////////////////////////////////////////////////////////////////////////////////////

// Protected variables.
protected:
	CIndividualSelectionParameters *m_pclIndSelParams;

	CExtNumEdit m_clExtEditFlow;
	CExtNumEdit m_clExtEditPower;
	CExtNumEdit m_clExtEditDT;
	CExtNumEdit m_clExtEditDp;
	CXGroupBox m_clGroupDp;
	CXGroupBox m_clGroupQ;
    int m_iRadioFlowPowerDT;
	bool m_fMustFocusDT;					// Use when user click on the DT part of the group Q when power radio button is disabled

	// Variables used to manage the checkbox 'Selection by set'.
	bool m_bAutoCheckboxSetManaging;
	bool m_bAtLeastOneNo;
	bool m_bAtLeastOneYes;
	bool m_bAtLeastOneYesBreakOk;
	bool m_bAtLeastOneYesBreakOnMsg;
	bool m_bFlowEditing;					// Allow to clear the right view only on the first user change in the flow field.
	bool m_bPowerDTEditing;					// Allow to clear the right view only on the first user change in the flow field.

// Protected methods.
protected:
	virtual BOOL OnInitDialog();

	DECLARE_MESSAGE_MAP()
	virtual afx_msg void OnBnClickedSuggest() {}
	virtual afx_msg void OnBnClickedSelect() {}
	virtual afx_msg void OnBnClickedCancel();

	virtual afx_msg void OnBnClickedRadioFlowOrPowerDT();

	virtual afx_msg void OnEnChangeFlow();
	virtual afx_msg void OnEnChangePower();
	virtual afx_msg void OnEnChangeDT();
	virtual afx_msg void OnEnChangeDp() {}
	virtual afx_msg	void OnEnKillFocusFlow();
	virtual afx_msg void OnEnKillFocusPower();
	virtual afx_msg void OnEnKillFocusDT();
	virtual afx_msg void OnEnKillFocusDp();
	virtual afx_msg void OnEnSetFocusFlow();
	virtual afx_msg void OnEnSetFocusPower();
	virtual afx_msg void OnEnSetFocusDT();

	virtual afx_msg void OnEditEnterChar( NMHDR *pNMHDR, LRESULT *pResult );

	////////////////////////////////////////////////////////////////////////////////////////////////////////////////
	// Overrides 'CDlgSelectionBase' protected afx_msg methods.
	virtual afx_msg void OnDestroy() {}
	virtual afx_msg LRESULT OnNewDocument( WPARAM wParam = 0, LPARAM lParam = 0 );
	virtual afx_msg LRESULT OnPipeChange( WPARAM wParam = 0, LPARAM lParam = 0 );
	virtual afx_msg LRESULT OnTechParChange( WPARAM wParam = 0, LPARAM lParam = 0 );
	virtual afx_msg LRESULT OnUnitChange( WPARAM wParam = 0, LPARAM lParam = 0 );
	virtual afx_msg LRESULT OnWaterChange( WPARAM wParam = 0, LPARAM lParam = 0 );
	// End of overriding 'CDlgSelectionBase' protected afx_msg methods.
	////////////////////////////////////////////////////////////////////////////////////////////////////////////////

	// Access to all editable fields and buttons.
	virtual CExtNumEdit *GetpEditFlow() const { return NULL; }
	virtual CExtNumEdit *GetpEditPower() const { return NULL; }
	virtual CExtNumEdit *GetpEditDT() const { return NULL; }
	virtual CExtNumEdit *GetpEditDp() const { return NULL; }
	virtual CButton *GetpRadioFlow() const { return NULL; }
	virtual CButton *GetpRadioPower() const { return NULL; }
	virtual CComboBoxEx *GetpComboKvs() const { return NULL; }
	virtual CStatic *GetpStaticDT() const { return NULL; }
	virtual CStatic *GetpStaticQUnit() const { return NULL; }
	virtual CStatic *GetpStaticPUnit() const { return NULL; }
	virtual CStatic *GetpStaticDTUnit() const { return NULL; }
	virtual CButton *GetpBtnSuggest() const { return NULL; }			// The default Suggest button
	virtual CButton *GetpBtnSelect() const { return ( (CButton *)GetDlgItem( IDC_BUTTONSELECT ) ); }		// The default Select button
	virtual CButton *GetpBtnCancelModif() const { return NULL; }		// The default Cancel button
	virtual CXGroupBox *GetpGroupQ() const { return NULL; }
	virtual CXGroupBox *GetpGroupDp() const { return NULL; }

	// Return pointer on the current Dp check button. By default 'CDlgSelectionBase' has now the 'm_GroupDp' variable member and
	// the method will return pointer on it.
	// If Dp check button is different from an inherited class (it's the case for 'CDlgSelectionBaseCv'!), we have to overwrite this
	// method to return back the correct pointer.
	virtual CButton *GetpBtnCheckDp() { return &m_clGroupDp; }
	
	void SetModificationMode( bool bFlag = true );

	// Verify if we can enable/disable and check/uncheck the checkbox 'Selection by set'.
	void VerifyCheckboxSetStatus( CRankEx *pList );

	virtual CRViewSSelSS *GetLinkedRightViewSSel( void ) { return NULL; }

// Protected variables.
protected:
	bool m_EditFieldHandlerBlocked;

// Private methods.
private:
	// Must be called after water characteristic has been changed to update flow if the input is currently in Power/DT mode.
	void _UpdateFlowEdit( void );

	bool _ReadFlow( double &dFlow );
	bool _ReadDp( double &dDp );
	bool _ReadPower( double &dPower );
	bool _ReadDT( double &dDT );
};
