#pragma once


#include "DataStruct.h"
#include "DlgLTtabctrl.h"
#include "DlgLeftTabBase.h"
#include "DlgSelectionBase.h"
#include "afxcmn.h"
#include "XGroupBox.h"
#include "ExtButton.h"
#include "ExtNumEdit.h"
#include "SelectPM.h"
#include "DlgIndSelPMPanels.h"
#include "afxwin.h"
#include "ProductSelectionParameters.h"

class CDlgIndSelPressureMaintenance : public CDlgSelectionBase
{
public:
	enum { IDD = IDD_DLGINDSELPRESSUREMAINTENANCE };
	
	typedef struct _InterfacePMPanels
	{
		CDlgIndSelPressureMaintenance *m_pclParent = NULL;
		void (CDlgSelectionBase::*m_pfnClearAll)( void ) = NULL;
		CRViewSSelSS* (CDlgIndSelPressureMaintenance::*m_pfnGetLinkedRightView)() = NULL;
	}InterfacePMPanels;

	CDlgIndSelPressureMaintenance( CWnd *pParent = NULL );
	virtual ~CDlgIndSelPressureMaintenance();

	////////////////////////////////////////////////////////////////////////////////////////////////////////////////
	// Overrides 'DialogExt' public virtual methods.
	virtual void SetApplicationBackground( COLORREF cBackColor );
	// End of overriding 'DlgSelectionBase' public virtual methods.
	////////////////////////////////////////////////////////////////////////////////////////////////////////////////

	// Allow to know if individual PM is in edition mode or not.
	// This method is called  by 'CDlgWizardSelPM'.
	bool IsEditModeRunning( void ) { return m_clIndSelPMParams.m_bEditModeRunning; }

	// Called by the 'CDlgIndSelPMPanel' class when user type the 'ENTER' key in an edition field.
	void ClickOnSuggestButton( void );

	void EnableSelectButton( bool bEnabled = true );
	bool IsButtonSelectEnabled( void );
	void VerifyModificationMode();

	// Allow to retrieve the object that is edited or _NULL_IDPTR if any.
	// This method is called  by 'CDlgWizardSelPM'.
	IDPTR GetEditedObject( void ) { return m_clIndSelPMParams.m_SelIDPtr; }

	// Allow the 'CDlgIndSelPMPanelXXX' classes to change the norm if needed (EN-12828 -> EN-12953).
	void SetNorm( CString strNormID );

	////////////////////////////////////////////////////////////////////////////////////////////////////////////////
	// Overrides 'CDlgSelectionBase' public virtual methods.
	virtual CString GetDlgCaption();
	virtual void SaveSelectionParameters();
	virtual void LeaveLeftTabDialog();
	virtual void ActivateLeftTabDialog();
	virtual void ChangeApplicationType( ProjectType eProjectType );
	virtual void OnRViewSSelSelectProduct( void );
	virtual void OnRViewSSelSelectionChanged( bool bSelectionActive );
	virtual void OnRViewSSelLostFocusWithTabKey( bool bShiftPressed );
	virtual void OnRViewSSelKeyboardEvent( int iKeyboardVirtualKey );
	
	// Remark: This method is called by the 'CDlgWaterChar' dialog to know it it can validate the fluid characteristic that
	// user has defined.
	virtual void OnVerifyFluidCharacteristics( bool &bEnable, CString &strMsg, CDlgWaterChar *pclDlgWaterChar );
	// End of overriding 'CDlgSelectionBase' public virtual methods.
	////////////////////////////////////////////////////////////////////////////////////////////////////////////////

	
	////////////////////////////////////////////////////////////////////////////////////////////////////////////////
	// Overrides 'CDlgSelectionBase' public virtual pure methods.
	virtual bool ResetRightView( void );
	virtual CMainFrame::RightViewList GetRViewID( void );
	// End of overriding 'CDlgSelectionBase' public virtual pure methods.
	////////////////////////////////////////////////////////////////////////////////////////////////////////////////


	////////////////////////////////////////////////////////////////////////////////////////////////////////////////
	// Overrides 'DlgSelectionTabHelper' public virtual methods.
	// This method is called when left tab dialog has no more the focus while user presses TAB or SHIFT+TAB.
	// Param: if 'fNext' is 'true' that means user presses 'TAB', otherwise it's 'SHIFT+TAB'.
	// Remark: for the pressure maintenance panel, it's a particular case. We have the 'CDlgIndSelPM' inherited directly from 'CDlgSelectionBase'.
	//         In this dialog, we have just a few controls common to all application types (heating, cooling and solar). Below these controls, there
	//         is one other dialog (CDlgIndSelPMPanels), one dialog for one application type. 
	//         At start, the focus is set on the control in the 'CDlgIndSelPM'. If user presses and reaches the last control, we don't want to go
	//         in the right view but first in the 'CDlgIndSelPMPanels'. When last control is reached in this dialog while pressing TAB key, then we can go
	//         to the right view.
	
	virtual bool LeftTabKillFocus( bool bNext );
	
	// End of overriding 'DlgSelectionTabHelper' public virtual methods.
	////////////////////////////////////////////////////////////////////////////////////////////////////////////////

	// Display result in normal or edition mode in regards to the current PM wizard selection mode.
	// This method is called by 'ActivateLeftTabDialog' when user intentionally goes in the individual selection mode
	// and when we close HySelect or save a file by being on the wizard page to save the wizard param.
	void UpdateWithWizardAllPrefInAllMode( void );

protected:
	virtual void DoDataExchange( CDataExchange *pDX );
	virtual BOOL OnInitDialog();
	virtual void OnOK() { /* DO NOTHING */ }

	DECLARE_MESSAGE_MAP()
	afx_msg LRESULT OnModifySelectedPMProduct( WPARAM wParam, LPARAM lParam );
	afx_msg LRESULT OnPMWQSelectionPrefsChange( WPARAM wParam, LPARAM lParam );
	afx_msg void OnBnClickedSuggest();
	afx_msg void OnBnClickedSelect();
	afx_msg void OnBnClickedReset();
	afx_msg void OnBnClickedCancel();
	afx_msg void OnCbnSelChangePMType();
	afx_msg void OnCbnSelChangeWaterMakeUpType();
	afx_msg void OnClickedCheckDegassing();
	afx_msg void OnCbnSelChangeNorm();
	
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
	// Overrides 'CDlgIndSelBase' protected pure virtual methods.
	virtual CRViewSSelSS *GetLinkedRightViewSSel( void );
	// End of overriding 'CDlgIndSelBase' protected pure virtual methods.
	////////////////////////////////////////////////////////////////////////////////////////////////////////////////

	void SetModificationMode( bool bFlag = true );
	void InitToolTips(CWnd *pWnd = NULL);

// Private methods.
private:
	void _CreateAllPanels( void );
	CDlgIndSelPMPanelBase::PanelType _GetPanelToShow( void );

	void _FillComboPressMaintType( CString strPressureTypeID = _T("") );
	void _ChangeComboPressMaintType( void );

	void _FillComboWaterMakeUpType( CString strWaterMakeUpID = _T("") );
	void _ChangeComboWaterMakeUpType( void );

	void _ChangeButtonDegassing( void );

	void _FillComboNorm( CString strNormID = _T("") );
	CString _GetNormID();

	void _VerifySuggestButton( void );

	// HYS-1427: We need to verify if use has well selected the PAZ accessory with Transfero TI when selection is done 
	// in the EN 12953 norm.
	// Returns 'false' if PAZ is needed and not selected, otherwise returns 'true'.
	bool _VerifyPAZAccessory( void );
	// HYS-1644: 
	bool _VerifyComCubeDMLConnectAccessory( void );

	// Display result in normal or edition mode in regards to the current PM wizard selection mode.
	// This method is called by 'ActivateLeftTabDialog' when user intentionally goes in the individual selection mode.
	void _UpdateWithWizardSelPMState( void );

	// Allow to update all fields, combo and so on with the current 'CPMInputUser'.
	// Param: 'bDontShowErrorMessage': This variable specify if we can show a popup window with message if there are errors.
	void _UpdateAllFields( bool bRedraw, bool bAllowShowErrorMessage );

	void _VerifyPMWQPrefs( void );

	void _VerifyFluidCharacteristics( bool bShowErrorMsg );
	void _VerifyFluidCharacteristicsHelper( bool &bEnable, CString &strMsg, CDlgWaterChar *pclDlgWaterChar = NULL );

// Private variables.
private:
	CIndSelPMParams m_clIndSelPMParams;
	InterfacePMPanels m_rInterfacePMPanels;

	std::map<int, CDlgIndSelPMPanelBase *> m_mapPanels;
	typedef std::map<int, CDlgIndSelPMPanelBase *>::iterator mapIntPanel;
	CDlgIndSelPMPanelBase::PanelType m_eCurrentPanel;
	CExtNumEditComboBox m_ComboPMType;

	CExtButton m_clButtonSuggest;
	CExtNumEditComboBox m_ComboWaterMakeUpType;
	CButton m_DegassingCheckbox;
	CExtNumEditComboBox m_ComboNorm;
};
