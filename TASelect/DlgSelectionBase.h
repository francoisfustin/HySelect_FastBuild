#pragma once


#include "DialogExt.h"
#include "MyToolTipCtrl.h"
#include "XGroupBox.h"
#include "DlgSelectionTabHelper.h"
#include "DlgWaterChar.h"
#include "DlgCOWaterChar.h"

class CMainFrame;
class CRViewSSelSS;
class CProductSelelectionParameters;
class CDlgSelectionBase : public CDialogExt, public CDlgSelectionTabHelper, public CRViewSSelSS::INotificationHandler, public CXGroupBox::INotificationHandler,
	public CDlgWaterChar::INotificationHandler, public CDlgCOWaterChar::INotificationHandler
{
public:
	CDlgSelectionBase( CProductSelelectionParameters &clProductSelectionParam, UINT nID = 0, CWnd *pParent = NULL );
	virtual ~CDlgSelectionBase() {}

	CProductSelelectionParameters *GetProductSelectParameters() { return m_pclProdSelParam; }

	void SetToolsDockablePaneContextID( int iContextID ) { m_iToolsDockablePaneContextID = iContextID; }
	int GetToolsDockablePaneContextID( void ) { return m_iToolsDockablePaneContextID; }

	// Return name for the tab	
	virtual CString GetDlgCaption( void ) { return TASApp.LoadLocalizedString(IDS_LEFTTAB); }

	virtual	void SaveSelectionParameters( void ) {}
	virtual	void LeaveLeftTabDialog( void ) {}
	virtual void ActivateLeftTabDialog( void );

	// Called by 'CDlgLeftTabSelManager' when user change application type.
	virtual void ChangeApplicationType( ProjectType eProductSelectionApplicationType );

	// Called by 'OnModifySelectedXXXProduct' to update technical parameters with fluid and temperature data.
	virtual void OnModifySelectedProductUpdateTechData( CSelectedInfos *pclSelectedInfos );

	virtual bool IsRightViewInEdition( void ) { return false; }

	// Overrides CXGroupBox::INotificaitionHandler.
	virtual void OnGroupIconClicked( CXGroupBox *pXGroupBoxClicked );

	// Overrides 'CRViewSSelSS::ISSheetNotificationHandler'.
	virtual void OnRViewSSelSelectProduct( void ) {}
	virtual void OnRViewSSelSelectionChanged( bool fSelectionActive ) {}
	virtual void OnRViewSSelLostFocusWithTabKey( bool fShiftPressed );
	virtual void OnRViewSSelKeyboardEvent( int iKeyboardVirtualKey ) {}

	// Overrides 'CDlgWaterChar::INotificaitionHandler'.
	// Remark: This method is called by the 'CDlgWaterChar' dialog to know it it can validate the fluid characteristic that
	// user has defined.
	virtual void OnVerifyFluidCharacteristics( bool &bEnable, CString &strMsg, CDlgWaterChar *pclDlgWaterChar ) {}
	virtual void OnVerifyFluidCharacteristics( bool &bEnable, CString &strMsg, CDlgCOWaterChar *pclDlgWaterChar ) {}

	virtual void ClearAll( void );
	virtual bool ResetRightView( void ) { return true; }
	virtual CMainFrame::RightViewList GetRViewID( void ) { return CMainFrame::eUndefined; }

// Protected methods.
protected:
	virtual BOOL OnInitDialog();
	virtual BOOL OnCommand( WPARAM wParam, LPARAM lParam );
	virtual BOOL PreTranslateMessage( MSG *pMsg );

	DECLARE_MESSAGE_MAP()
	virtual afx_msg void OnDestroy() {}
	virtual afx_msg LRESULT OnNewDocument( WPARAM wParam = 0, LPARAM lParam = 0 );
	virtual afx_msg LRESULT OnPipeChange( WPARAM wParam = 0, LPARAM lParam = 0 );
	virtual afx_msg LRESULT OnTechParChange( WPARAM wParam = 0, LPARAM lParam = 0 ) { return 0; }
	virtual afx_msg LRESULT OnUnitChange( WPARAM wParam = 0, LPARAM lParam = 0 ) { return 0; }
	virtual afx_msg LRESULT OnWaterChange( WPARAM wParam = 0, LPARAM lParam = 0 );

//	virtual afx_msg void OnLButtonDown( UINT nFlags, CPoint point );

	// Allow to do some post initialization after the 'OnNewDocument' has been called.
	virtual void OnAfterNewDocument( void ) {}

	void SelectCurrentComboPipes( CString strPipeSeriesID, CString strPipeSizeID );

	void PostWMCommandToControl( CWnd *pWnd );

// Protected variables.
protected:
	CProductSelelectionParameters *m_pclProdSelParam;
	bool m_bInitialised;										// Flag that specify the TabCDialogSSel has been completely initialized
	bool m_bRViewEmpty;											// Flag = true if the RView is empty
	int m_iToolsDockablePaneContextID;

	typedef std::vector<HWND> vecpHWND;
	typedef std::map<HWND, vecpHWND > mapGroupBoxControlList;
	typedef mapGroupBoxControlList::iterator mapGroupBoxControlListIter;
	typedef std::map<HWND, HWND > mapControlGRoupBox;
	typedef mapControlGRoupBox::iterator mapControlGRoupBoxIter;
	typedef std::map<HWND, CRect > mapGroupRect;
	typedef mapGroupRect::iterator mapGroupRectIter;
	typedef std::multimap<int, HWND> mmapIntHWND;
	typedef mmapIntHWND::iterator mmapIntHWNDIter;
	typedef mmapIntHWND::reverse_iterator mmapIntHWNDRevIter;
	typedef std::map<HWND, BOOL > mapHWNDBOOL;
	typedef mapHWNDBOOL::iterator mapHWNDBOOLIter;

	mmapIntHWND m_mmapListOfControls;						// Memorize list of all controls (sorted from the lower to higher in terms of y coordinate).
	mapHWNDBOOL m_mapControlShowStatus;						// Memorize the show status for controls.
	mapHWNDBOOL m_mapControlEnableStatus;					// Memorize the enable status for controls.
	mapGroupRect m_mapControlInitialSize;					// Memorize initial client area size for all controls.
	mapGroupBoxControlList m_mapListOfControlsInGroup;		// Memorize for each group all controls in it.
	mapControlGRoupBox m_mapLinkControlAndGroup;			// Memorize link between a control and its group (for optimization).
	mapGroupRect m_mapGroupCoordinate;						// Memorize initialize client area size of all groups.
	bool m_bXGBListInitialized;
	
	CMyToolTipCtrl m_ToolTip;
	
// Private methods.
private:
	void _UpdatePipeSeriesSize( void );
	void _InitializeGroupList( void );
};
