#pragma once


#include "DlgLeftTabBase.h"
#include "DlgWizardSelBase.h"
#include "RViewWizardSelPM.h"
#include "ExtStatic.h"
#include "ProductSelectionParameters.h"

class CDlgWizardSelPM : public CDlgWizardSelBase, public CRViewWizardSelPM::INotificationHandler
{
public:
	enum { IDD = IDD_DLGWIZARDSELPM };

	CDlgWizardSelPM( CWnd *pParent = NULL );
	virtual ~CDlgWizardSelPM();

	////////////////////////////////////////////////////////////////////////////////////////////////////////////////
	// Overrides 'DialogExt' public virtual methods.
	virtual void SetApplicationBackground( COLORREF cBackColor );
	// End of overriding 'DlgSelectionBase' public virtual methods.
	////////////////////////////////////////////////////////////////////////////////////////////////////////////////

	// Allow to retrieve the object that is edited or _NULL_IDPTR if any.
	IDPTR GetEditedObject( void ) { return m_clWizardSelPMParams.m_SelIDPtr; }

	// Allow to know if wizard PM is in edition mode or not.
	// This method is called  by 'CDlgIndSelPressureMaintenance'.
	bool IsEditModeRunning( void ) { return m_clWizardSelPMParams.m_bEditModeRunning; }

	// Allow to know if PM individual selection can reset the right view or not.
	// Remark: If user goes in the 'Results' step, it means the 'RViewSSelPM::Suggest' method is called to show suggestions.
	//         If user goes in one other step but without change and directly goes in individual mode, it means that is not
	//         needed to call again 'RViewSSelPM::Suggest'.
	bool IsRightViewResetNeeded( void );

	////////////////////////////////////////////////////////////////////////////////////////////////////////////////
	// Overrides 'CDlgSelectionBase' public virtual methods.
 	virtual void SaveSelectionParameters();
	virtual	void LeaveLeftTabDialog();
	virtual void ActivateLeftTabDialog();
	
	// Called by 'CDlgLeftTabSelManager' when user change application type.
	virtual void ChangeApplicationType( ProjectType eProjectType );
	
	virtual CMainFrame::RightViewList GetRViewID( void ) { return CMainFrame::eRVWizardSelPM; }

	// Remark: This method is called by the 'CDlgWaterChar' dialog to know it it can validate the fluid characteristic that
	// user has defined.
	virtual void OnVerifyFluidCharacteristics( bool &bEnable, CString &strMsg, CDlgWaterChar *pclDlgWaterChar );
	// End of overriding 'CDlgSelectionBase' public virtual methods.
	////////////////////////////////////////////////////////////////////////////////////////////////////////////////

	////////////////////////////////////////////////////////////////////////////////////////////////////////////////
	// Overrides 'DlgSelectionTabHelper' public virtual methods.
	//
	// This method is called when left tab dialog has no more the focus while user presses TAB or SHIFT+TAB.
	// Param: if 'fNext' is 'true' that means user presses 'TAB', otherwise it's 'SHIFT+TAB'.
	// This method is called when left tab dialog has no more the focus while user presses TAB or SHIFT+TAB.
	// Param: if 'fNext' that means user presses 'TAB', otherwise it's 'SHIFT+TAB'.
	// Return 'false' if the focus must stay on the left tab, otherwise the individual or batch selection in the right view has taken the focus.
	virtual bool LeftTabKillFocus( bool bNext );

	// End of overriding 'DlgSelectionTabHelper' public virtual methods.
	////////////////////////////////////////////////////////////////////////////////////////////////////////////////

	////////////////////////////////////////////////////////////////////////////////////////////////////////////////
	// Overrides CRViewWizardSelPM::INotificationHandler.
	virtual void OnRViewWizSelPMButtonBackClicked();
	virtual void OnRViewWizSelPMButtonNextClicked();
	virtual void OnRViewWizSelPMActivateStepWithCurrentRightView( int iCurrentRightView );
	virtual void OnRViewWizSelPMSwitchNormalAdvancedMode( int iWhichInputDialog );
	virtual void OnRViewWizSelPMInputChange( bool bErrorStatus );
	virtual bool OnRViewWizSelPMIsEditingMode() { return m_clWizardSelPMParams.m_bEditModeRunning; }
	virtual void OnRViewWizSelPMVerifyFluidCharacteristics( bool &bEnable, CString &strMsg );
	virtual int OnRViewWizSelPMGetDlgWizePMRightViewInputID( int iDlgWizePMRightViewInputID );
	// End of overriding CRViewWizardSelPM::INotificationHandler.
	////////////////////////////////////////////////////////////////////////////////////////////////////////////////

	////////////////////////////////////////////////////////////////////////////////////////////////////////////////
	// Overrides CRViewSelSS::INotificationHandler.
	virtual void OnRViewSSelSelectProduct( void );
	virtual void OnRViewSSelSelectionChanged( bool bSelectionActive );
	virtual void OnRViewSSelLostFocusWithTabKey( bool bShiftPressed );
	virtual void OnRViewSSelKeyboardEvent( int iKeyboardVirtualKey );
	// End of overriding CRViewSelSS::INotificationHandler.
	////////////////////////////////////////////////////////////////////////////////////////////////////////////////

protected:
	virtual void DoDataExchange( CDataExchange *pDX );
	virtual BOOL OnInitDialog();

	DECLARE_MESSAGE_MAP()
	virtual afx_msg void OnSize( UINT nType, int cx, int cy );
	virtual afx_msg void OnLButtonDown( UINT nFlags, CPoint point );
	virtual afx_msg LRESULT OnModifySelectedPMProduct( WPARAM wParam, LPARAM lParam );

	////////////////////////////////////////////////////////////////////////////////////////////////////////////////
	// Overrides 'CDlgSelectionBase' protected afx_msg methods.
	virtual afx_msg LRESULT OnNewDocument( WPARAM wParam = 0, LPARAM lParam = 0 );	
	virtual afx_msg LRESULT OnTechParChange( WPARAM wParam = 0, LPARAM lParam = 0 );
	virtual afx_msg LRESULT OnWaterChange( WPARAM wParam = 0, LPARAM lParam = 0 );
	// End of overriding 'CDlgSelectionBase' protected afx_msg methods.
	////////////////////////////////////////////////////////////////////////////////////////////////////////////////

// Private methods.
private:
	// Go in the good steps for editing of a selection.
	void _UpdateForEditingSelection( void );

	// Called when user clicks on one of the button on the left panel.
	void _OnBnClickedButton( int iWhichButton );

	// Automatically change the button state in regards to where the user is in the different steps.
	// Remark: This method change only variable. To apply the change, call the '_UpdateAllStatics' method.
	void _ChangeButtonStates( void );

	// In regards to the variables defining the button state, this method will redraw the left panel.
	void _UpdateAllStatics( void );

	bool _BackToProjectType( int iWhichButton );

	bool _ProjectTypeChoice_GoInHeating( int iWhichButton );
	bool _ProjectTypeChoice_GoInCooling( int iWhichButton );
	bool _ProjectTypeChoice_GoInSolar( int iWhichButton );
	bool _ProjectTypeChoice_GoInWaterMakeUpOnly( int iWhichButton );
	bool _ProjectTypeChoice_GoInDegassingOnly( int iWhichButton );

	bool _BackToProjectTypeEdition( int iWhichButton );

	bool _ProjectTypeChoice_GoInHeatingEdition( int iWhichButton );
	bool _ProjectTypeChoice_GoInCoolingEdition( int iWhichButton );
	bool _ProjectTypeChoice_GoInSolarEdition( int iWhichButton );
	bool _ProjectTypeChoice_GoInWaterMakeUpOnlyEdition( int iWhichButton );
	bool _ProjectTypeChoice_GoInDegassingOnlyEdition( int iWhichButton );

	bool _RightViewValuesInputDialogToChange( int iWhichButton );

	bool _GoToResultsStep( int iWhichButton );

	bool _Select( int iWhichButton );
	bool _ApplyEdition( int iWhichButton );

	// Even if we don't use the 'iWhichButton' variable for the 'CancelEdition' method, we need to keep this signature to
	// add the pointer on this function when creating 'COneButtonDefinition' object in the constructor.
	bool _CancelEdition( int iWhichButton );

	// Called when for example user changes fluid temperature in the ribbon. We need thus to update field of the
	// current right view input dialog and check for other ones if this change can not create errors.
	void _ApplyPMInputUserChangedInRightView( bool bWaterCharUpdated = false );

	// Check data before to display results with the 'RViewSSelPM' view.
	bool _GoInResultsPreliminary( void );

	// Allow to go to a specific step.
	void _GoToStep( int iStepNameID );

	void _ChangeSelectOrApplyButtonsState( bool bEnable );

	// Allow to update the dialog height in regards to the buttons really visible. In this way, 'DlgLeftTabBase' can 
	// perfectly draw the corporate logo at the bottom of the left tab.
	void _UpdateDialogHeight( void );

	// Allow to fill 'm_clIndSelPMParams' with needed data from 'm_clWizardSelPMParams' to show results in the 'RViewSSelPM' when
	// user double clicks on a batch result.
	void _CopyWizardSelParamsToIndSelParams( void );

	bool _VerifyFluidCharacteristics( bool bShowErrorMsg );
	void _VerifyFluidCharacteristicsHelper( bool &bEnable, CString &strMsg, CDlgWaterChar *pclDlgWaterChar = NULL );

	// Display result in normal or edition mode in regards to the current PM individual selection mode.
	// This method is called by 'ActivateLeftTabDialog' when user intentionally goes in the wizard selection mode.
	void _UpdateWithIndividualSelPMState();
	void _UpdateWithIndividualAllPrefInAllMode();

// Private variables.
private:
	// To make wizard selection.
	CWizardSelPMParams m_clWizardSelPMParams;

	// To allow to call the right view in individual selection mode with the wizard parameters.
	CIndSelPMParams m_clIndSelPMParams;

	typedef enum _WizardPMEngineState
	{
		WPMES_ProjectTypeChoice,
		WPMES_ValueInputStepsHeating,
		WPMES_ValueInputStepsCooling,
		WPMES_ValueInputStepsSolar,
		WPMES_ValueInputStepsWaterMakeUp,
		WPMES_ValueInputStepsDegassing,
		WPMES_ProjectTypeChoiceEdition,
		WPMES_ValueInputStepsHeatingEdition,
		WPMES_ValueInputStepsCoolingEdition,
		WPMES_ValueInputStepsSolarEdition,
		WPMES_ValueInputStepsWaterMakeUpEdition,
		WPMES_ValueInputStepsDegassingEdition,
	}WizardPMEngineState;

	WizardPMEngineState m_eWizardPMEngineState;

	int m_iCurrentButton;

	typedef enum _ButtonImgPos
	{
		BIP_None,
		BIP_Left,
		BIP_Right
	}ButtonImgPos;

	typedef enum _ButtonStatus
	{
		BS_ToDefine,
		BS_ToDo,
		BS_Disable = BS_ToDo,
		BS_Doing,
		BS_Enable = BS_Doing,
		BS_Done,
		BS_Error
	}ButtonStatus;

	typedef bool (CDlgWizardSelPM::*pfActionToDo) ( int iWhichButton );

	class COneButtonDefinition
	{
	public:
		COneButtonDefinition( int iIDSText, ButtonImgPos eImagePosition, ButtonStatus eStatus, HICON hIcon, pfActionToDo ptrfActionToDo,
			int iDlgWizePMRightViewInputID = -1 );

		// Either, by dafault we have 'm_iDlgWizePMRightViewInputID' for all norm. But we can also a specific right view for a norm.
		// In this case we call 'AddDlgWizePMRightViewInputID' with the norm and the dialog ID.
		void AddDlgWizePMRightViewInputID( PressurisationNorm eNorm, int iDlgWizePMRightViewInputID );

		// Allow to retrieve the dialog ID to show. If only 'm_iDlgWizePMRightViewInputID' is defined, we return this ID.
		// If 'm_veciDlgWizePMRightViewInputID' contains data, we check first if norm exist in it. If it's the case, we return the 
		// corresponding dialog ID. If nothind found, we return 'm_iDlgWizePMRightViewInputID'.
		int GetDlgWizePMRightViewInputID( PressurisationNorm eNorm );

		int m_iIDSText;
		ButtonImgPos m_eImagePosition;
		ButtonStatus m_eStatus;
		HICON m_hIcon;
		pfActionToDo m_ptrfActionToDo;
		int m_iDlgWizePMRightViewInputID;
		int m_iButtonNumber;
		std::map<PressurisationNorm, int> m_mapiDlgWizePMRightViewInputID;
	};

	class COneGroupDefinition
	{
	public:
		COneGroupDefinition() { m_iVisibleButtonNumber = 0; }
		virtual ~COneGroupDefinition();

		COneButtonDefinition *AddOneButtonDefinition( COneButtonDefinition *pclOneButtonDefinition );
		void ChangeButtonStatus( int iIDSText, ButtonStatus eStatus );

		// Allow to retrieve a button definition with the right view dialog ID.
		COneButtonDefinition *GetButtonDefinitionWithDlgID( int iDlgWizePMRightViewInputID );

		// Allow to retrieve a button definition with the text on it.
		COneButtonDefinition *GetButtonDefinitionWithTextID( int iIDSText );

		COneButtonDefinition *operator[] ( int iIndex );

		CArray<COneButtonDefinition*> m_arOneGroupDefinition;
		int m_iVisibleButtonNumber;
	};

	class CAllGroupDefinitions
	{
	public:
		CAllGroupDefinitions() {}
		virtual ~CAllGroupDefinitions();

		void AddOneGroupDefinition( COneGroupDefinition *pclOneGroupDefinition );
		COneGroupDefinition *operator[] ( int iIndex );

		CArray<COneGroupDefinition*> m_arAllGroupDefinitions;
	};

	CAllGroupDefinitions m_clAllGroupDefinitions;
	COneGroupDefinition *m_pclCurrentGroupDefinition;

	CArray<CExtStatic*, CExtStatic*> m_arpButtonText;
	CArray<CStatic*, CStatic*> m_arpButtonImg;
	CArray<CStatic*, CStatic*> m_arpSeparator;
	CArray<CRect, CRect> m_arButtonTextWidthTop;
	CArray<CRect, CRect> m_arButtonImgWidthTop;
	CArray<CRect, CRect> m_arSeparatorWidthTop;

	CExtStatic m_clButtonText1;
	CExtStatic m_clButtonText2;
	CExtStatic m_clButtonText3;
	CExtStatic m_clButtonText4;
	CExtStatic m_clButtonText5;
	CExtStatic m_clButtonText6;
	CExtStatic m_clButtonText7;
	CExtStatic m_clButtonText8;
	CExtStatic m_clButtonText9;
	CExtStatic m_clButtonText10;

	CStatic m_clButtonImg1;
	CStatic m_clButtonImg2;
	CStatic m_clButtonImg3;
	CStatic m_clButtonImg4;
	CStatic m_clButtonImg5;
	CStatic m_clButtonImg6;
	CStatic m_clButtonImg7;
	CStatic m_clButtonImg8;
	CStatic m_clButtonImg9;
	CStatic m_clButtonImg10;

	CStatic m_clSeparator1;
	CStatic m_clSeparator2;
	CStatic m_clSeparator3;
	CStatic m_clSeparator4;
	CStatic m_clSeparator5;
	CStatic m_clSeparator6;
	CStatic m_clSeparator7;
	CStatic m_clSeparator8;
	CStatic m_clSeparator9;

	HICON m_hIconButtonToDo;
	HICON m_hIconButtonDoing;
	HICON m_hIconButtonDone;
	HICON m_hIconButtonLeft;
	HICON m_hIconButtonError;

	CPMInputUser m_clPMInputUserSaved;

	// Set if user has already gone at least one time at the 'Results' step.
	// Remark: it help us to know when going in individual mode if we can display
	//         results in 'RViewSSelPM' or not.
	bool m_bResultsAlreadyDisplayed;
};
