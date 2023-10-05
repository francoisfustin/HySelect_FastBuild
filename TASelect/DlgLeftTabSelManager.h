#pragma once


#include "DialogExt.h"
#include "DlgLeftTabBase.h"
#include "PMSelectionHelper.h"

class CDlgSelectionBase;
class CDlgLeftTabSelManager : public CDlgLeftTabBase
{
	DECLARE_DYNCREATE( CDlgLeftTabSelManager )

public:
	CDlgLeftTabSelManager( CWnd *pParent = NULL );
	virtual ~CDlgLeftTabSelManager();
	
	enum{ IDD = IDD_DLGLEFTTABSELMANAGER };

	// Allow to change the 'IDC_STATICGROUP' size to allow 'DlgLeftTabBase' to draw the corporate logo.
	void ChangeStaticGroupSize( CRect rectNewSize );

	// This method is called by 'CMainFrame::OnRbnPSAppplicationType' when user chooses in the combo application type in the main ribbon.
	void ChangeApplicationType( ProjectType eNewProjectType );

	CMainFrame::RightViewList UpdateLeftTabDialog( ProductSubCategory eProductSubCategory, ProductSelectionMode eProductSelectionMode );

	// Allow to retrieve the current left dialog shown.
	CDlgSelectionBase *GetCurrentLeftTabDialog();

	// Allow to retrieve the left dialog for a specific product sub category in the current selection mode.
	CDlgSelectionBase *GetLeftTabDialog( ProductSubCategory eProductSubCategory );

	// Allow to retrieve the left dialog for a specific product sub category in a specific selection mode.
	CDlgSelectionBase *GetLeftTabDialog( ProductSubCategory eProductSubCategory, ProductSelectionMode eProductSelectionMode );

	bool GetCurrentProductSubCategoryAndSelectionMode( ProductSubCategory &eProductSubCategory, ProductSelectionMode &eProductSelectionMode );

	CPMSelectionHelper *GetPMSelectionHelper( void ) { return &m_clPMSelectionHelper; }

	void WriteAllSSelParamsToRegistry();
	void SaveAllSSelParamsToTADS();

	// Overrides 'CDlgLeftTabBase' public virtual methods.
	virtual void OnLTTabCtrlEnterTab( CMyMFCTabCtrl::TabIndex eTabIndex, CMyMFCTabCtrl::TabIndex ePrevTabIndex );
	virtual void OnLTTabCtrlLeaveTab( CMyMFCTabCtrl::TabIndex eTabIndex );

	// Overrides 'CDlgLeftTabBase' public pure virtual methods.
	virtual void InitializeToolsDockablePaneContextID( void );

	// Allow to retrieve the context ID for the current product sub category and the current selection mode.
	virtual bool GetToolsDockablePaneContextID( int &iContextID );

	// Allow to retrieve the context ID for the specific product sub category.
	bool GetToolsDockablePaneContextID( ProductSubCategory eProductSubCatagory, int &iContextID );

	// Allow to retrieve the context ID for the specific product sub category and for the specific selection mode.
	bool GetToolsDockablePaneContextID( ProductSubCategory eProductSubCatagory, ProductSelectionMode eProductSelectionMode, int &iContextID );

	// Set if we must set the tool dockable pane in full mode or not.
	// If 'fApply' is set to 'true', we directly apply the change. Otherwise, we change in the context but not apply.
	void SetToolsDockablePaneFullScreenMode( bool bFullscreen, bool bApply );

	virtual void OnApplicationLook( COLORREF cBackColor );

	DECLARE_MESSAGE_MAP()
	virtual void DoDataExchange( CDataExchange *pDX );
	virtual BOOL OnInitDialog();
	virtual afx_msg void OnHScroll( UINT nSBCode, UINT nPos, CScrollBar *pScrollBar );
	virtual afx_msg void OnVScroll( UINT nSBCode, UINT nPos, CScrollBar *pScrollBar );
	virtual afx_msg void OnSize( UINT nType, int cx, int cy );

// Private methods.
private:
	bool _AddPage( UINT nDialogID, ProductSubCategory eProductSubCategory, ProductSelectionMode eProductSelectionMode );

	CDlgSelectionBase *_CreatePage( UINT nDialogID );

	CDlgSelectionBase *_SelectPage( ProductSubCategory eProductSubCategory, ProductSelectionMode eProductSelectionMode );

	void _CheckScrollBar( void );

	typedef enum FullScreenMode
	{
		FSM_Undefined,
		FSM_True,
		FSM_False
	};

	bool _UpdateBatchSelectionOutputDialog( ProductSubCategory eProductSubCategory, ProductSelectionMode eProductSelectionMode, 
				bool bEnable, FullScreenMode eFullScreenMode = FullScreenMode::FSM_Undefined );

// Private variables.
private:
	bool m_bHScrollBarVisible;
	bool m_bVScrollBarVisible;

	// Contains the pointers on the dialog corresponding to a product category (For example: BV) for the three
	// selection mode (Individual, batch and wizard).
	class CLeftTabDialogsByProductSubCategory
	{
	public:
		CLeftTabDialogsByProductSubCategory();
		virtual ~CLeftTabDialogsByProductSubCategory();
		void CleanAll( void );

		void AddLeftTabDialog( CDlgSelectionBase *pDlgSelectionBase, ProductSelectionMode eProductSelectionMode );

		// Allow to retrieve the left tab dialog for a specific product selection mode.
		CDlgSelectionBase *GetLeftTabDialog( ProductSelectionMode eProductSelectionMode );

		// Allow to retrieve what is the current left tab dialog.
		CDlgSelectionBase *GetLeftTabDialog( void );

		// Allow to retrieve what is the current product selection mode.
		ProductSelectionMode GetProductSelectionMode( void );

		// Allow to set the current product selection mode.
		void SetProductSelectionMode( ProductSelectionMode eProductSelectionMode );
		
		void SetToolsDockablePaneContextID( int iContextID );
		void SetToolsDockablePaneContextID( int iContextID, ProductSelectionMode eProductSelectionMode );
		
		int GetToolsDockablePaneContextID( void );
		int GetToolsDockablePaneContextID( ProductSelectionMode eProductSelectionMode );

	// Private variables.
	private:
		CArray<CDlgSelectionBase *, CDlgSelectionBase *> m_arpLeftTabs;
		ProductSelectionMode m_eCurrentProductSelectionMode;
		int m_iContextID;
	};
	
	// Contains one 'CLeftTabDialogsByProductSubCategory' by sub product category (Example: BV).
	class CAllLeftTabDialogs
	{
	public:
		CAllLeftTabDialogs();
		virtual ~CAllLeftTabDialogs();

		void AddLeftTabDialog( CDlgSelectionBase *pDlgSelectionBase, ProductSubCategory eProductSubCategory, ProductSelectionMode eProductSelectionMode );

		// Allow to retrieve what is the current product sub category.
		ProductSubCategory GetProductSubCategory( void );

		// Allow to retrieve the left tab dialog for a specific product sub category and specific product selection mode.
		CDlgSelectionBase *GetLeftTabDialog( ProductSubCategory eProductSubCategory, ProductSelectionMode eProductSelectionMode );

		// Allow to retrieve the left tab dialog for a specific product sub category.
		CDlgSelectionBase *GetLeftTabDialog( ProductSubCategory eProductSubCategory );

		// Allow to retrieve what is the current left tab dialog.
		CDlgSelectionBase *GetLeftTabDialog( void );

		// Is at least one product selection mode exists for a specific product sub category and for a specific selection mode.
		bool IsAtLeastOneProductSelectionModeExist( ProductSubCategory eProductSubCategory, ProductSelectionMode eProductSelectionMode );

		// Allow to retrieve the product selection mode of a specific left tab dialog.
		ProductSelectionMode GetProductSelectionMode( ProductSubCategory eProductSubCategory );
		
		// Allow to retrieve what is the current product selection mode of the current left tab dialog.
		ProductSelectionMode GetProductSelectionMode( void );

		// Allow to set what is the current product sub category.
		void SetProductSubCategory( ProductSubCategory eProductSubCategory );

		// Allow to set what is the current product selection mode in the current product sub category.
		void SetProductSelectionMode( ProductSelectionMode eProductSelectionMode );

		// Allow to set the tools dockable pane context ID for the current product sub category and the current selection mode.
		void SetToolsDockablePaneContextID( int iContextID );

		// Allow to set the tools dockable pane context ID linked to a specific product sub category and the current selection mode.
		void SetToolsDockablePaneContextID( int iContextID, ProductSubCategory eProductSubCategory );

		// Allow to set the tools dockable pane context ID linked to a specific product sub category and linked to a specific selection mode.
		void SetToolsDockablePaneContextID( int iContextID, ProductSubCategory eProductSubCategory, ProductSelectionMode eProductSelectionMode );
		
		// Allow to retrieve the context ID linked for the current product sub category and current selection mode.
		// Remark: never directly call this method. The 'GetToolsDockablePaneContextID::GetContextID' method must be called instead !!!
		int GetToolsDockablePaneContextID( void );

		// Allow to retrieve the tools dockable pane context ID linked to a specific product sub category and the current selection mode.
		// Remark: never directly call this method. The 'CDlgLeftTabSelManager::GetToolsDockablePaneContextID' method must be called instead !!!
		int GetToolsDockablePaneContextID( ProductSubCategory eProductSubCategory );

		// Allow to retrieve the tools dockable pane context ID linked to a specific product sub category and specific selection mode.
		// Remark: never directly call this method. The 'CDlgLeftTabSelManager::GetToolsDockablePaneContextID' method must be called instead !!!
		int GetToolsDockablePaneContextID( ProductSubCategory eProductSubCategory, ProductSelectionMode eProductSelectionMode );

	// Private variables.
	private:
		CArray<CLeftTabDialogsByProductSubCategory*, CLeftTabDialogsByProductSubCategory*> m_arLeftDialogList;
		ProductSubCategory m_eCurrentProductSubCategory;
	};

	CAllLeftTabDialogs m_clAllLeftDialogs;

	bool m_bBypass;
	CString m_pclMainRbnCBSelPipeSizeIDSaved;

	// Helper class to allow easily exchange the 'CSelectPMList' class between individual and wizard PM selection.
	CPMSelectionHelper m_clPMSelectionHelper;
};

extern CDlgLeftTabSelManager *pDlgLeftTabSelManager;
