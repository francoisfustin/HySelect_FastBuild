#pragma once


#include "FormViewEx.h"
#include "ProductSelectionParameters.h"

class CRViewWizardSelBase : public CFormViewEx
{
public:
	enum
	{
		IDD = IDV_RVIEW
	};

	// This allows this class to communicate with 'CDlgWizardSelPM'.
	class INotificationHandler
	{
	public:
		enum
		{
			NH_First,
			NH_OnButtonBackClicked = NH_First,
			NH_OnButtonNextClicked = ( NH_OnButtonBackClicked << 1 ),
			NH_ActivateStepWithCurrentRightView = ( NH_OnButtonNextClicked << 1 ),
			NH_OnSwitchNormalAdvancedMode = ( NH_ActivateStepWithCurrentRightView << 1 ),
			NH_OnInputChange = ( NH_OnSwitchNormalAdvancedMode << 1 ),
			NH_IsEditingMode = ( NH_OnInputChange << 1 ),
			NH_VerifyFluidCharacteristic = ( NH_IsEditingMode << 1 ),
			NH_GetDlgWizePMRightViewInputID = ( NH_VerifyFluidCharacteristic << 1 ),
			NH_Last = NH_VerifyFluidCharacteristic, 
			NH_All = ( NH_OnButtonBackClicked | NH_OnButtonNextClicked | NH_ActivateStepWithCurrentRightView | NH_OnSwitchNormalAdvancedMode | NH_OnInputChange 
					| NH_IsEditingMode | NH_VerifyFluidCharacteristic | NH_GetDlgWizePMRightViewInputID )
		};
		INotificationHandler() {}
		virtual ~INotificationHandler() {}
		virtual void OnRViewWizSelPMButtonBackClicked() = 0;
		virtual void OnRViewWizSelPMButtonNextClicked() = 0;
		virtual void OnRViewWizSelPMActivateStepWithCurrentRightView( int iCurrentRightView ) = 0;
		virtual void OnRViewWizSelPMSwitchNormalAdvancedMode( int iWhichInputDialog ) = 0;
		virtual void OnRViewWizSelPMInputChange( bool bErrorStatus ) = 0;
		virtual bool OnRViewWizSelPMIsEditingMode() = 0;
		virtual void OnRViewWizSelPMVerifyFluidCharacteristics( bool &bEnable, CString &strMsg ) = 0;
		virtual int OnRViewWizSelPMGetDlgWizePMRightViewInputID( int iDlgWizePMRightViewInputID ) = 0;
	};

	CRViewWizardSelBase( CMainFrame::RightViewList eRightViewWizardID );
	virtual ~CRViewWizardSelBase() {}

	// To verify if there is some selection displayed.
	virtual bool IsEmpty( void ) { return true; }

	// Allow to all 'DlgWizPMRightViewInputxxx' class to prevent when user has changed an input value.
	virtual void OnInputChange( int iErrorCode );

	// Allow to all 'DlgWizPMRightViewInputxxx' class to get the current status of the edition mode.
	bool IsEditingMode( void );

	void RegisterNotificationHandler( INotificationHandler *pclHandler );
	void UnregisterNotificationHandler( INotificationHandler *pclHandler );

	BOOL Create( LPCTSTR lpszClassName, LPCTSTR lpszWindowName, DWORD dwStyle, const RECT& rect, CWnd* pParentWnd, UINT nID, CCreateContext* pContext = NULL );

	CMainFrame::RightViewList GetRViewID( void ) { return m_eRViewID; }

	// Called by 'CDlgWizardSelBase' when the right view gains the focus.
	// Param 'fFirst' is 'true' if we must set the focus on the first available group. Otherwise set the focus on the last group.
	// Remark: 'W' (for wrapper) is just to be sure to no interfere with the 'SetFocus' in the base class.
	void SetFocusW( bool fFirst );

	// Called by any of the 'DlgWizPMRightViewInputxxx' classes to check if fluid characteristic is good or not.
	void VerifyFluidCharacteristic( bool &bEnable, CString &strMsg );

// Protected members.
protected:
	DECLARE_MESSAGE_MAP()

	virtual afx_msg LRESULT OnSwitchNormalAdvancedMode( WPARAM wParam, LPARAM lParam );
	virtual afx_msg LRESULT OnButtonBackNextClicked( WPARAM wParam, LPARAM lParam );

	// Overrides 'CFormViewEx' virtual methods.
	virtual void OnInitialUpdate( );

// Protected variables.
protected:
	INotificationHandler *m_pNotificationHandlers;

// Private variables.
private:
	CWizardSelectionParameters *m_pclWizSelParams;
	CMainFrame::RightViewList m_eRViewID;
};
