#pragma once



#define RVIEWWIZARDSELPM_MINWIDTH						900
#define RVIEWWIZARDSELPM_MINHEIGHT						400
#define RVIEWWIZARDSELPM_TEXTPICTCONTAINER_WIDTH		350

class CDlgWizardPM_RightViewInput_Base;
class CDlgWizardPM_RightViewPictureContainer;
class CDlgWizardPM_RightViewTextContainer;
class CRViewWizardSelPM : public CRViewWizardSelBase
{
public:
	class CTextIDValue
	{
	public:
		CTextIDValue() { m_iIDS = -1; }
		CTextIDValue( int iIDS ) { m_iIDS = iIDS; }
		CTextIDValue( int iIDS, int iPhysicalUnit, double dValue ) { m_iIDS = iIDS; m_vecValues.push_back( std::pair<int, double>( iPhysicalUnit, dValue ) ); }
		CTextIDValue( int iIDS, int iPhysicalUnit1, double dValue1, int iPhysicalUnit2, double dValue2 ) 
				{ m_iIDS = iIDS; m_vecValues.push_back( std::pair<int, double>( iPhysicalUnit1, dValue1 ) ); m_vecValues.push_back( std::pair<int, double>( iPhysicalUnit2, dValue2 ) ); }
		void Copy( CTextIDValue clTextIDValue ) { m_iIDS = clTextIDValue.m_iIDS; m_vecValues = clTextIDValue.m_vecValues; }
		int m_iIDS;
		std::vector<std::pair<int, double>> m_vecValues;
	};

	CRViewWizardSelPM();
	virtual ~CRViewWizardSelPM();

	// Allow the 'DlgWizardSelPM' to change the background color when user changes the visual style.
	void SetApplicationBackground( COLORREF cBackColor );

	////////////////////////////////////////////////////////////////////////////////////////////////////////////////
	// Overrides 'CRViewWizardSelBase' public virtual methods.
	// To verify if there is some selection displayed.
	virtual bool IsEmpty( void );

	// Allow to all the 'DlgWizPMRightViewInputxxx' classes to prevent when user has changed an input value.
	// At its turn the 'CRViewWizardSelBase' base class will warn the 'CDlgWizardSelPM' class thanks to the 
	// call to the 'OnInputChange' notification handler (see 'CRViewWizardSelBase.h' for definitions).
	// Param: 'iErrorMaskNormal' will allow us to clear/set corresponding error bits for the normal view.
	// Param: 'iErrorMaskAdvanced' will allow us to clear/set corresponding error bits for the advanced view.
	virtual void OnInputChange( int iErrorCode, int iErrorMaskNormal, int iErrorMaskAdvanced = 0 );
	// End of overriding 'CRViewWizardSelBase' public virtual methods.
	////////////////////////////////////////////////////////////////////////////////////////////////////////////////

	void SetWizarSelectionParameters( CWizardSelPMParams *pclWizardSelParams ) { m_pclWizardSelParams = pclWizardSelParams; }

	// Force the current right view input dialog (if exist) to save its edit fields in the 'CPMInputUser' class.
	void FillPMInputUser( void );

	// Called when for example the 'CDlgWizardSelPM' class receives the 'OnWaterChange' message and needs to apply
	// an update of some values in the current 'CDlgWizardPM_RightViewInput_XXX' class. It's the case when for example user 
	// changes the fluid temperature in the ribbon bar.
	// Because changes are directly applied in in the 'CPMInputUser' variable of the 'CDlgWizardSelPM' class and the
	// 'CRViewWizardSelPM' class has a pointer on this variable, there is no need to pass the 'CPMInputUser' as argument.
	void ApplyPMInputUserUpdated( bool bWaterCharUpdated = false, bool bShowErrorMsg = true );
	
	BOOL Create( LPCTSTR lpszClassName, LPCTSTR lpszWindowName, DWORD dwStyle, const RECT &rect, CWnd *pParentWnd, UINT nID, 
			CCreateContext *pContext = NULL );

	void DetachCurrentInputDialog( void );
	void AttachNewInputDialog( CWizardSelPMParams *pclWizardSelParams, int iRightViewInputdialog, bool bBackEnable, bool bNextEnable );
	void UpdatePicture( int iRightViewInputdialog );
	void UpdateText( int iRightViewInputdialog );

	// Allow 'CDlgWizardSelPM' to know if there is error in a specific right view input dialog.
	// The dialog can be one other that the current one that is displayed.
	// If user make a change for example in the fluid temperature in the ribbon, 'CDlgWizardSelPM' has to check if it is needed to mark
	// in red the 'Temperature' static text even if the user is not currently on this step.
	bool IsAtLeastOneError( int iRightViewInputID );

// Protected members.
protected:
	DECLARE_MESSAGE_MAP()

	virtual afx_msg void OnSize( UINT nType, int cx, int cy );
	virtual afx_msg LRESULT OnButtonBackNextClicked( WPARAM wParam, LPARAM lParam );
	virtual afx_msg BOOL OnMouseWheel( UINT nFlags, short zDelta, CPoint pt );

	// Overrides 'CFormView' virtual methods.
	virtual void OnInitialUpdate( );
	virtual void OnActivateView( BOOL bActivate, CView *pActivateView, CView *pDeactiveView );

// Private methods.
private:
	UINT _CreateRightViewInputDialog( int iRightViewInputdialog, CDlgWizardPM_RightViewInput_Base **ppclDlgWizPM_RightViewInputBase );

// Private variables.
private:
	CWizardSelPMParams *m_pclWizardSelParams;
	CDlgWizardPM_RightViewInput_Base *m_pclDlgWizPM_RightViewInputBase;
	CDlgWizardPM_RightViewPictureContainer *m_pclDlgWizPM_RVPictureContainer;
	CDlgWizardPM_RightViewTextContainer *m_pclDlgWizPM_RVTextContainer;

	typedef enum _NormalAdvMode
	{
		NAM_NoAdvancedMode,						// No advanced mode exist.
		NAM_AdvancedMode_Normal,				// Advanced mode exist: it is the normal mode.
		NAM_AdvancedMode_Advanced				// Advanced mode exist: it is the advanced mode.
	}NormalAdvMode;

	typedef struct _ErrorStatusAndLink
	{
		int m_iErrorCode;					// Set if there is error in this input value dialog.
		NormalAdvMode m_eMode;				// Set if this input value dialog is the normal or advanced mode.
		int m_iNormalID;					// For 'NAM_AdvancedMode_Advanced' allow to retrieve the normal mode.
		std::vector<int> m_vecAdvIDList;	// List of all advanced input dialog link to the normal mode.
	}ErrorStatusAndLink;
	map<int, ErrorStatusAndLink> m_mapInputValueDlgLinks;

	CArray<CArray<CTextIDValue>> m_arTextIDSList;
	CArray<int> m_arImageIDBList;
};

extern CRViewWizardSelPM *pRViewWizardSelPM;
