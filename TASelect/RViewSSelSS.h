#pragma once


#include "SSheet.h"
#include "RViewDescription.h"
#include "MultiSpreadInView.h"
//#include "ProductSelectionParameters.h"

namespace RVSCellDescription
{
	enum
	{
		// 'CD' is for Cell Description.
		CD_Button				= 0x0000,
		CD_ButtonMask			= 0x00FF,
		CD_Product				= 0x0100,
		CD_ProductMask			= 0x0100,
		CD_Link					= 0x0200,

		// 'CDB' is for Cell Description Button.
		CDB_Box					= 0x0010,
		CDB_ShowAllPriorities	= 0x0020,
		CDB_ExpandCollapseRow	= 0x0030,
		CDB_ExpandCollapseGroup = 0x0040,
		CDB_Checkbox			= 0x0050,
		CDB_TitleGroup			= 0x0060,
		
		// 'CDBC' is for Cell Description Button Checkbox.
		CDBC_Accessories		= 0x0051
	};
};

////////////////////////////////////////////////////////////////////////////////////
// CCellDescription is defined in 'RViewDescription.h'.
class CCellDescriptionButton : public CCellDescription
{ 
protected:
	CCellDescriptionButton( int iButtonType, CPoint ptCellPosition, CSheetDescription *pclSheetDescription, bool bShowStatus, 
			int iButtonState, CSSheet::PictureCellType ePictureType );
public:
	virtual ~CCellDescriptionButton() {}
	
public:
	// Getter.
	int GetButtonState( void ) { return m_iButtonState;	}
	bool GetShowStatus( void ) { return m_bShowStatus; }
	
	// Setter.
	void SetButtonState( int iButtonState, bool fApplyChange = false );
	virtual void SetShowStatus( bool bShowStatus );
	
	// Change button state.
	virtual void ToggleButtonState( bool bApplyChange = false ) = 0;
	
	// Update drawing of button in regards to its internal state variables.
	virtual void ApplyInternalChange( void );
	
	// Delete button
	// Remark: PAY ATTENTION: this method allows to clear a button when 'm_pclSSheet' is yet valid. Don't call this
	//         method from destructor for example because it would be possible that 'm_pclSSheet' has been already released.
	virtual void DeleteButton( void );

protected:
	bool m_bShowStatus;
	int m_iButtonState;
	CSSheet::PictureCellType m_ePictureType;
	std::map< int, int > m_mapStateImageID;
};
	
class CCDButtonBox : public CCellDescriptionButton
{
public:
	enum ButtonState
	{
		BoxOpened,
		BoxClosed
	};

	CCDButtonBox( CPoint ptCellPosition, CSheetDescription *pclSheetDescription, bool bShowStatus, ButtonState eButtonState );
	virtual ~CCDButtonBox() {}
	
	// Change button status.
	virtual void ToggleButtonState( bool bApplyChange = false );
};

class CCDButtonShowAllPriorities : public CCellDescriptionButton
{
public:
	enum ButtonState
	{
		ShowPriorities,
		HidePriorities
	};

	// Param: 'ptCellPosition' set in which cell to draw button.
	// Param: 'bShowStatus' set what if the button is shown or not.
	// Param: 'eButtonState' set what is the current button state.
	// Param: 'lFromRow' and 'lToRow' set the range of rows that must be shown or hidden when toggle.
	CCDButtonShowAllPriorities( CPoint ptCellPosition, CSheetDescription *pclSheetDescription, bool bShowStatus, ButtonState eButtonState, 
			long lFromRow, long lToRow );

	virtual ~CCDButtonShowAllPriorities() {}
	
	// Getter.
	void GetRange( long &lFromRow, long &lToRow ) { lFromRow = m_lFromRow; lToRow = m_lToRow; }
	long GetRangeFrom( void ) { return m_lFromRow; }
	long GetRangeTo( void ) { return m_lToRow; }

	
	/////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
	// Overriding of CCellDescriptionButton 

	// Change button status.
	virtual void ToggleButtonState( bool fApplyChange = false );
	
	// Update drawing of button in regards to its internal state variables.
	virtual void ApplyInternalChange( void );

	// End of overriding of CCellDescriptionButton 
	/////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
	

private:
	long m_lFromRow;
	long m_lToRow;
};

class CCDButtonExpandCollapseRows : public CCellDescriptionButton
{
public:
	enum ButtonState
	{
		CollapseRow,
		ExpandRow
	};

	// Param: 'ptCellPosition' set in which cell to draw button.
	// Param: 'fShowStatus' set what if the button is shown or not.
	// Param: 'eButtonState' set what is the current button state.
	// Param: 'lFromRow' and 'lToRow' set the range of rows that must be shown or hidden when toggle.
	CCDButtonExpandCollapseRows( CPoint ptCellPosition, CSheetDescription *pclSheetDescription, bool fShowStatus, ButtonState eButtonState, long lFromRow, long lToRow );
	virtual ~CCDButtonExpandCollapseRows();

	// Getter.
	void GetRange( long &lFromRow, long &lToRow ) { lFromRow = m_lFromRow; lToRow = m_lToRow; }
	long GetRangeFrom( void ) { return m_lFromRow; }
	long GetRangeTo( void ) { return m_lToRow; }

	/////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
	// Overriding of CCellDescriptionButton 

	// Change button status.
	virtual void ToggleButtonState( bool bApplyChange = false );
	
	// Update drawing of button in regards to its internal state variables.
	virtual void ApplyInternalChange( void );
	
	// Delete button
	// Remark: PAY ATTENTION: this method allows to clear a button when 'm_pclSSheet' is yet valid. Don't call this
	//         method from destructor for example because it would be possible that 'm_pclSSheet' has been already released.
	virtual void DeleteButton( void );

	// End of overriding of CCellDescriptionButton 
	/////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

private:
	long m_lFromRow;
	long m_lToRow;
	BOOL * m_parbState;
};

class CCDButtonExpandCollapseGroup : public CCellDescriptionButton
{
public:
	enum ButtonState
	{
		CollapseRow,
		ExpandRow
	};

	// Param: 'ptCellPosition' set in which cell to draw button.
	// Param: 'bShowStatus' set what if the button is shown or not.
	// Param: 'eButtonState' set what is the current button state.
	// Param: 'lFromRow' and 'lToRow' set the range of rows that must be shown or hidden when toggle.
	CCDButtonExpandCollapseGroup( CPoint ptCellPosition, CSheetDescription *pclSheetDescription, bool bShowStatus, ButtonState eButtonState, 
			long lFromRow, long lToRow );

	virtual ~CCDButtonExpandCollapseGroup();

	// Getter.
	void GetRange( long &lFromRow, long &lToRow ) { lFromRow = m_lFromRow; lToRow = m_lToRow; }
	long GetRangeFrom( void ) { return m_lFromRow; }
	long GetRangeTo( void ) { return m_lToRow; }

	/////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
	// Overriding of CCellDescriptionButton 

	// Change button status.
	virtual void ToggleButtonState( bool bApplyChange = false );
	
	// Update drawing of button in regards to its internal state variables.
	virtual void ApplyInternalChange( void );
	
	// Delete button
	// Remark: PAY ATTENTION: this method allows to clear a button when 'm_pclSSheet' is yet valid. Don't call this
	//         method from destructor for example because it would be possible that 'm_pclSSheet' has been already released.
	virtual void DeleteButton( void );

	// End of overriding of CCellDescriptionButton 
	/////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

private:
	long m_lFromRow;
	long m_lToRow;
	BOOL * m_parbState;
};

class CCDButtonCheckbox : public CCellDescriptionButton
{
public:
	enum ButtonState
	{
		Unchecked,
		Checked
	};

	CCDButtonCheckbox( int iCheckboxType, CPoint ptCellPosition, CSheetDescription *pclSheetDescription, bool fShowStatus, bool bChecked, 
			bool bEnabled, CString strText );

	virtual ~CCDButtonCheckbox() {}

	// Getter.
	bool GetCheckStatus( void ) { return ( ( Unchecked == m_iButtonState ) ? false : true ); }
	bool GetEnableStatus( void ) { return m_bEnabled; }
	CString GetText( void ) { return m_strText; }
	
	// Setter.
	void SetCheckStatus( bool bChecked ) { m_iButtonState = ( ( false == bChecked ) ? Unchecked : Checked ); }
	void SetEnableStatus( bool bEnabled ) { m_bEnabled = bEnabled; }
	void SetText( CString strText ) { m_strText = strText; }

	/////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
	// Overriding of CCellDescriptionButton 

	// Change button status.
	virtual void ToggleButtonState( bool bApplyChange = false );
	
	// Update drawing of button in regards to its internal state variables.
	virtual void ApplyInternalChange( void );

	// End of overriding of CCellDescriptionButton 
	/////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

private:
	bool m_bEnabled;
	CString	m_strText;
};

class CCDBCheckboxAccessory : public CCDButtonCheckbox
{
public:
	CCDBCheckboxAccessory( CPoint ptCellPosition, CSheetDescription *pclSheetDescription, bool bShowStatus, bool bChecked, bool bEnabled, 
			CString strText, CDB_Product *pAccessory, CDB_RuledTableBase *pRuledTable );
	
	virtual ~CCDBCheckboxAccessory() {}

	// Getter.
	CDB_Product *GetAccessoryPointer( void ) { return m_pAccessory; }
	LPARAM GetAccessoryLParam( void ) { return (LPARAM)m_pAccessory; }
	CDB_RuledTableBase *GetRuledTable( void ) { return m_pRuledTable; }
	CString GetToolTip( void ) { return m_strToolTip; }

	// Setter.
	void SetAccessory( CDB_Product *pAccessory ) { m_pAccessory = pAccessory; }
	void SetRuledTable( CDB_RuledTable *pRuledTable ) { m_pRuledTable = pRuledTable; }
	void SetToolTip( CString strToolTip ) { m_strToolTip = strToolTip; }

private:
	CDB_Product *m_pAccessory;
	CDB_RuledTableBase* m_pRuledTable;
	CString m_strToolTip;
};

class CCDButtonTitleGroup : public CCellDescriptionButton
{
public:
	CCDButtonTitleGroup( CPoint ptCellPosition, CSheetDescription *pclSheetDescription );
	virtual ~CCDButtonTitleGroup() {}

	// Do nothing here.
	virtual void ToggleButtonState( bool bApplyChange = false ) {}

private:
	long m_lFromColumn;
	long m_lToColumn;
};

class CCellDescriptionProduct : public CCellDescription
{
public:
	CCellDescriptionProduct(CPoint ptCellPosition, CSheetDescription *pclSheetDescription, LPARAM lProduct)
		: CCellDescription(RVSCellDescription::CD_Product, ptCellPosition, pclSheetDescription) { m_lProduct = lProduct; m_lUserParam = 0; m_lSecondUserParam = 0;}
	
	virtual ~CCellDescriptionProduct() {}
	
	LPARAM GetProduct( void ) { return m_lProduct; }
	LPARAM GetUserParam( void ) { return m_lUserParam; }
	// HYS-872
	LPARAM GetSecondUserParam(void) { return m_lSecondUserParam; }
	void SetUserParam(LPARAM lUserParam) { m_lUserParam = lUserParam; }
	// HYS-872	
	void SetSecondUserParam (LPARAM lSecondUserParam) { m_lSecondUserParam = lSecondUserParam; }

private:
	LPARAM m_lProduct;
	LPARAM m_lUserParam;
	// HYS-872: add a second user parameter to save other product (buffer vessel for HYS-872 for example)
	LPARAM m_lSecondUserParam;
};

class CCellDescriptionLink : public CCellDescription
{
public:
	CCellDescriptionLink( CPoint ptCellPosition, CSheetDescription *pclSheetDescription, LPARAM lUserParam ) 
		: CCellDescription( RVSCellDescription::CD_Link, ptCellPosition, pclSheetDescription ) { m_lUserParam = lUserParam; }
	
	virtual ~CCellDescriptionLink() {}

	LPARAM GetUserParam( void ) { return m_lUserParam; }
	void SetUserParam( LPARAM lUserParam ) { m_lUserParam = lUserParam; }

private:
	LPARAM m_lUserParam;
};

// End of variables to manage cell types for single selection
////////////////////////////////////////////////////////////////////////////////////

class CRViewSSelSS : public CMultiSpreadInView
{
protected:
	
	/////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
	// Structures definition
	/////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

	// Structure to save current focus and variables linked to it.
	// Remark: it's better to use LPARAM instead of long for parameter. Because LPARAM is casted in regards to current
	//         processor (32 or 64 bits ... should be 128 soon).
	struct CurrentFocus
	{
		CurrentFocus()
		{
			Reset();
		}
		
		void Reset()
		{
			m_pclSheetDescription = NULL;
			m_rectCurrentFocus.SetRectEmpty();
			m_lParam = (LPARAM)0;
			m_rectBorderStyleSaved.SetRectEmpty();
			m_rectBorderColorSaved.SetRectEmpty();
		}
		
		CSheetDescription *m_pclSheetDescription;			// Sheet description in which focus is.
		CRect m_rectCurrentFocus;							// Focus coordinates.
		LPARAM m_lParam;									// Parameter under focus.
		CRect m_rectBorderStyleSaved;
		CRect m_rectBorderColorSaved;
	};

	/////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
	// Typedefs definition
	/////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

	typedef std::vector<CCDBCheckboxAccessory*>		vecCDCAccessoryList;			// 'CDC' is for 'Cell Definition Checkbox'
	typedef vecCDCAccessoryList::iterator			vecCDCAccessoryListIter;

public:

	class INotificationHandler
	{
	public:
		enum
		{
			NH_First					= 0x01,
			NH_OnSelectProduct			= NH_First,							// Message sent when user choose 'Select' in the context menu.
			NH_OnSelectionChanged		= ( NH_OnSelectProduct << 1 ),		// Message sent when selection changed.
			NH_OnLostFocus				= ( NH_OnSelectionChanged << 1 ),	// Message sent when the right view lost its focus.
			NH_OnKeyboardVirtualKey		= ( NH_OnLostFocus << 1 ),			// Message sent when keyboard virtual key was used.
			NH_Last						= ( NH_OnKeyboardVirtualKey << 1 ),
			NH_All						= ( NH_OnSelectProduct | NH_OnSelectionChanged | NH_OnLostFocus | NH_OnKeyboardVirtualKey )
		};
		INotificationHandler() {}
		~INotificationHandler() {}
		virtual void OnRViewSSelSelectProduct( void ) = 0;
		virtual void OnRViewSSelSelectionChanged( bool fSelectionActive ) = 0;
		virtual void OnRViewSSelLostFocusWithTabKey( bool fShiftPressed ) = 0;
		virtual void OnRViewSSelKeyboardEvent( int iKeyboardVirtualKey ) = 0;
	};

	/////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
	// Enum definition
	/////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
	
	enum { IDD = IDV_RVIEW };

	enum SheetDescriptionUserVariables
	{
		_SDUV_TOTALPRODUCT_COUNT = 1,
		_SDUV_NOPRIORITYPRODUCT_COUNT,
		_SDUV_TOTALPRODUCTACCESSORY_COUNT,
		_SDUV_TOTALADAPTER_COUNT,
		_SDUV_TOTALACTUATOR_COUNT,
		_SDUV_TOTALACTUATORACCESSORY_COUNT,
		_SDUV_SELECTEDPRODUCT,
		_SDUV_REMARKROW,
		_SDUV_TITLEFORECOLOR,
		_SDUV_TITLEBACKCOLOR,
	};

	enum DefineNextFocusedRowReturnCode
	{
		DNFERRC_ParameterNULL = 0,
		DNFERRC_RViewLoseFocusError = 1,
		DNFERRC_RViewLoseFocusOK = 2,
		DNFERRC_FocusOnPrevButNoSelectableRow = 3,
		DNFERRC_FocusOnNextButNoSelectableRow = 4,
		DNFERRC_FocusOnPreviousGroupOK = 5,
		DNFERRC_FocusOnNextGroupOK = 6
	};

	/////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
	// Methods definition
	/////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

	CRViewSSelSS( CMainFrame::RightViewList eRightViewID, bool bUseOnlyOneSpread = true, UINT nId = IDD );
	virtual ~CRViewSSelSS() {};

	// Register an handler to manage notification initiated by 'CDlgOutputSpreadContainer'.
	void RegisterNotificationHandler( INotificationHandler *pclHandler, short nNotificationHandlerFlags );
	
	// Unregister an handler.
	void UnregisterNotificationHandler( INotificationHandler *pclHandler );

	// Called by 'CDlgIndSelBase' when the right view gains the focus.
	// Param 'fFirst' is 'true' if we must set the focus on the first available group. Otherwise set the focus on the last group.
	// Remark: 'W' (for wrapper) is just to be sure to no interfere with the 'SetFocus' in the base class.
	void SetFocusW( bool bFirst );

	/////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
	// Overriding CMultiSpreadInView public virtual methods.

	virtual BOOL Create( LPCTSTR lpszClassName, LPCTSTR lpszWindowName, DWORD dwStyle, const RECT &rect, CWnd *pParentWnd, UINT nID, 
			CCreateContext *pContext = NULL );

	virtual void Reset( void );

	// End of overriding CMultiSpreadInView public virtual methods.
	/////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

	// Called by 'DlgIndSelXXX' the user click 'Suggest' button.
	// Param: 'pclProductSelectionParameters' contains all needed data.
	virtual void Suggest( CProductSelelectionParameters *pclProductSelectionParameters, LPARAM lpParam = NULL ) 
			{ m_pclProdSelParams = pclProductSelectionParameters; }

	/////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
	// Public pure virtual methods to declare in inherited classes.

	// Called by 'DlgIndSelXXX' when user click 'Select' button.
	// Param: 'pSelectedProductToFill' -> class where to save current user selection.
	virtual void FillInSelected( CDS_SelProd *pSelectedProductToFill ) = 0;

	// End of public pure virtual methods to declare in inherited classes.
	/////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
	
	// Allow to initialize some internal variables.
	// Remark: called by 'CDlgIndSelXXX::OnNewDocument'.
	virtual void OnNewDocument( CDS_IndSelParameter *pSSelParameter ) {}

	// Allow to save some internal variables.
	// Remark: called by 'CDlgIndSelXXX::SaveSelectionParameters'.
	virtual void SaveSelectionParameters( CDS_IndSelParameter *pSSelParameter ) {}
	// HYS-987: Update accessory list in Edit mode
	void UpdateAccessoryList( vector< CCDBCheckboxAccessory *> vectAccChecked, CAccessoryList *pclAccListToUpdate, CAccessoryList::AccessoryType eAccType );

// Protected members.
protected:

	DECLARE_MESSAGE_MAP()
	virtual void DoDataExchange( CDataExchange *pDX );
	virtual void MoveUpDownFpspread( long lCol, long lRow ) {}
	
	// TO REMOVE
	afx_msg LRESULT OnComboDropCloseUp( WPARAM wParam, LPARAM lParam );

	/////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
	// AFX message handlers

	// Message received when user wants to see Full catalog sheet on a product.
	afx_msg void OnGetFullinfo();
	
	afx_msg void OnfltSelect();
	afx_msg void OnGetTender();
	afx_msg void OnUpdateGetTender( CCmdUI *pCmdUI );
	afx_msg void OnUpdateMenuText( CCmdUI *pCmdUI );
	
	// End of AFX message handlers
	/////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////


	/////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
	// Overriding CView protected virtual methods.
	
	virtual void OnActivateView( BOOL bActivate, CView *pActivateView, CView *pDeactiveView );

	// End of overrides CMultiSpreadInView protected virtual methods.
	/////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////


	/////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
	// Overriding CMultiSpreadInView protected virtual methods.

	// Called by 'CellClickFpspread()' when user clicks on a cell.
	// Remark: must be implemented in derived classes.
	virtual void OnCellClicked( CSheetDescription *pclSheetDescription, long lClickedCol, long lClickedRow );

	// Called by 'DblClickFpspread()' when user double clicks on a cell.
	virtual void OnCellDblClicked( CSheetDescription *pclSheetDescription, long lClickedCol, long lClickedRow );

	// Called by 'RightClickFpspread()' when user right clicks on a cell.
	virtual void OnRightClick( CSheetDescription *pclSheetDescription, long lClickedCol, long lClickedRow, long lXPos, long lYPos );

	// Called by 'PreTranslateMessage()' when user presses a key.
	// Return: 'TRUE' if message must not be dispatched.
	virtual BOOL OnKeyDown( CSheetDescription *pclSheetDescription, WPARAM wKeyPressed );
	virtual BOOL OnKeyUp( CSheetDescription *pclSheetDescription, WPARAM wKeyPressed ) { return FALSE; }

	// Called by 'TextTipFetch' when a sheet wants to show a tip.
	virtual bool OnTextTipFetch( CSheetDescription *pclSheetDescription, long lColumn, long lRow, WORD *pwMultiLine, SHORT *pnTipWidth, 
			TCHAR *pstrTipText, BOOL *pbShowTip );

	virtual bool GetCurrentFocus( CSheetDescription *&pclSheetDescription, CRect &rectFocus );

	// End of overrides CMultiSpreadInView protected virtual methods.
	/////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

	
	/////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
	// Protected pure virtual methods to declare in inherited classes.

	// This method is called when user click on a product (and not on button or accessories).
	// Return: 'true' if we need to update sheet size and scroll position.
	// Remark: this method must be defined in the inherited classes!!!
	virtual bool OnClickProduct( CSheetDescription *pclSheetDescription, CCellDescriptionProduct *pclCellDescriptionProduct, long lColumn, long lRow ) = 0;

	// This method is called when user click on a title group (to reinitialize default column width).
	// Remark: this method must be defined in the inherited classes!!!
	virtual bool ResetColumnWidth( short nSheetDescriptionID ) = 0;

	virtual bool IsSelectionReady( void ) = 0;
	
	// End of protected pure virtual methods to declare in inherited classes.
	/////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////


	/////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
	// Methods to manage CCellDescriptionProduct

	// Create a cell with a product.
	// Param: 'lColumn' and 'lRow' -> cell coordinates where to set button.
	// Param: 'LPARAM' -> is the product.
	// Param: 'pclSSheetDescription' is the sheet description in which button must be added.
	CCellDescriptionProduct *CreateCellDescriptionProduct( long lColumn, long lRow, LPARAM lProduct, CSheetDescription *pclSSheetDescription );

	// Allow to retrieve a LPARAM from the cell at the position defined by 'lColumn' and 'lRow'.
	// Return: 0 if cell description at position is not a product or if there is nothing.
	LPARAM GetCDProduct( long lColumn, long lRow, CSheetDescription *pclSheetDescription, CCellDescriptionProduct **ppclCDProduct = NULL );

	// Allow to find a product in a specified row and containing the same lpProduct.
	// Param: if 'lRow' is set to -1 we search on all available rows in the 'pclSheetDescription'.
	// Remark: to see example of using, see 'Suggest()' method in all inherited classes.
	CCellDescriptionProduct *FindCDProduct( long lRow, LPARAM lpProduct, CSheetDescription *pclSheetDescription );

	// Same as above but instead of doing a search on the product, we do it with the user param.
	virtual CCellDescriptionProduct *FindCDProductByUserParam( long lRow, LPARAM lpUserParam, CSheetDescription *pclSheetDescription );

	// End of methods to manage CCellDescriptionProduct
	/////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////


	/////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
	// Methods to manage box button.

	// Create button.
	// Param: 'lColumn' and 'lRow' -> cell coordinates where to set button.
	// Param: 'bEnabled' -> 'false' is this checkbox is not selectable.
	// Param: 'eButtonState' -> set in which mode the button is set.
	// Param: 'pclSSheetDescription' is the sheet description in which button must be added.
	CCDButtonBox *CreateCellDescriptionBox( long lColumn, long lRow, bool bEnabled, CCDButtonBox::ButtonState eButtonState, 
			CSheetDescription *pclSSheetDescription );

	// Allow to retrieve a cell description button box from the cell at the position defined by 'lColumn' and 'lRow'.
	// Return: NULL if nothing.
	CCDButtonBox *GetCDButtonBox( long lColumn, long lRow, CSheetDescription *pclSheetDescription );

	// End of methods to manage box button.
	/////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

	
	/////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
	// Methods to manage Show/Hide all priorities button.

	// Create button.
	// Param: 'lColumn' and 'lRow' -> cell coordinates where to set button.
	// Param: 'bEnabled' -> 'false' is this checkbox is not selectable.
	// Param: 'eButtonState' -> set in which mode the button is set.
	// Param: 'lFromRow' and 'lToRow' set which are the lines to Show or Hide when toggle button.
	// Param: 'pclSSheetDescription' is the sheet description in which button must be added.
	// Param: 'fApplyForAllSheet' -> 'true' if a modification on this button impact all displayed sheet
	CCDButtonShowAllPriorities *CreateShowAllPrioritiesButton( long lColumn, long lRow, bool bEnabled, CCDButtonShowAllPriorities::ButtonState eButtonState, 
			long lFromRow, long lToRow, CSheetDescription *pclSSheetDescription );

	// Allow to retrieve in which row is located the 'Show all priorities' button.
	// Return -1 if not found.
	long GetShowAllPrioritiesButtonRow( CSheetDescription *pclSSheetDescription, CCDButtonShowAllPriorities **pclButton );

	void CheckShowAllPrioritiesButtonState( CSheetDescription *pclSheetDescription, long lRowSelected );

	// HYS-1539 : If the Rview exists and was not activated when doing a _ShowAllPrioritiesArrowClicked,
	// the priority button is not updated. So when this Rview is activated and user move the mouse on
	// the button, we update it to have the good picture and tooltip.
	void UpdateShowAllPrioritiesButtonState( CSheetDescription *pclSheetDescription, CCDButtonShowAllPriorities* pclButton, CCellDescription* pclCellDescription );

	// End of methods to manage Show/Hide all priorities button.
	/////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////


	/////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
	// Methods to manage Expand/Collapse rows button.

	// Create button.
	// Param: 'lColumn' and 'lRow' -> cell coordinates where to set button.
	// Param: 'bEnabled' -> 'false' is this checkbox is not selectable.
	// Param: 'eButtonState' -> set in which mode the button is set.
	// Param: 'lFromRow' and 'lToRow' set which are the lines to Show or Hide when toggle button.
	// Param: 'pclSSheetDescription' is the sheet description in which button must be added.
	CCDButtonExpandCollapseRows *CreateExpandCollapseRowsButton( long lColumn, long lRow, bool bEnabled, CCDButtonExpandCollapseRows::ButtonState eButtonState, 
			long lFromRow, long lToRow, CSheetDescription *pclSSheetDescription );

	// Allow to delete the button.
	// Param: 'pCDBExpandCollapseRows' is the pointer on the button.
	// Param: 'pclSSheetDescription' is the sheet description from which button must be deleted.
	// Remark: PAY ATTENTION! 'pCDBExpandCollapseRows' will be reseted to NULL!!!
	void DeleteExpandCollapseRowsButton( CCDButtonExpandCollapseRows *&pCDBExpandCollapseRows, CSheetDescription *pclSSheetDescription );

	// End of methods to manage Expand/Collapse rows button.
	/////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////


	/////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
	// Methods to manage Expand/Collapse group button.

	// Create button.
	// Param: 'lColumn' and 'lRow' -> cell coordinates where to set button.
	// Param: 'bEnabled' -> 'false' is this checkbox is not selectable.
	// Param: 'eButtonState' -> set in which mode the button is set.
	// Param: 'lFromRow' and 'lToRow' set which are the lines to Show or Hide when toggle button.
	// Param: 'pclSSheetDescription' is the sheet description in which button must be added.
	CCDButtonExpandCollapseGroup *CreateExpandCollapseGroupButton( long lColumn, long lRow, bool bEnabled, 
			CCDButtonExpandCollapseGroup::ButtonState eButtonState,  long lFromRow, long lToRow, CSheetDescription *pclSSheetDescription );

	// Allow to delete the button.
	// Param: 'pCDBExpandCollapseGroup' is the pointer on the button.
	// Param: 'pclSSheetDescription' is the sheet description from which button must be deleted.
	// Remark: PAY ATTENTION! 'pCDBExpandCollapseGroup' will be reseted to NULL!!!
	void DeleteExpandCollapseGroupButton( CCDButtonExpandCollapseGroup *&pCDBExpandCollapseGroup, CSheetDescription *pclSSheetDescription );

	// Allow to inherited class to be alerted when this button is pressed.
	virtual void OnExpandCollapeGroupButtonClicked( CCDButtonExpandCollapseGroup *pCDBExpandCollapseGroup, CSheetDescription *pclSSheetDescription ) {}

	// End of methods to manage Expand/Collapse group button.
	/////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////


	/////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
	// Methods to manage accessories.

	// Create checkbox accessory
	// Param: 'lColumn' and 'lRow' -> cell coordinates where to set checkbox.
	// Param: 'bChecked' -> checkbox state.
	// Param: 'bEnabled' -> 'false' is this checkbox is not selectable.
	// Param: 'strText' -> text to display.
	// Param: 'pAccessory' -> parameter to save.
	// Param= 'pRuledTabble' is the ruled table linked to this accessory.
	// Param: 'pvecAccessoryList' is the list where to save newly created checkbox accessory.
	// Param: 'pclSSheetDescription' is the sheet description in which checkbox must be added.
	CCDBCheckboxAccessory *CreateCheckboxAccessory( long lColumn, long lRow, bool bChecked, bool bEnabled, CString strText, CDB_Product *pAccessory,
			CDB_RuledTableBase *pRuledTable, vecCDCAccessoryList *pvecAccessoryList, CSheetDescription *pclSheetDescription );

	// Allow to verify all checkbox accessories state in regards to the current one selected.
	// Param: 'pSelectedAccessory' is the currently selected accessory.
	// Param: 'bChecked' set if accessory is selected or not.
	// Param: 'pvecAccessoryList' is the list of all accessories.
	// Param: 'fForceChange' set to 'true' if you want check/uncheck even if checkbox is disabled.
	void VerifyCheckboxAccessories( CDB_Product *pSelectedAccessory, bool bChecked, vecCDCAccessoryList *pvecAccessoryList, bool bForceChange = false );

	// Allow to disable some accessories that belongs to an identified group.
	// Param: 'lpTabID' is the ID of the owner group or NULL if we want to disable all accessories.
	// Param: 'pvecAccessoryList' is the list of all accessories.
	void DisableCheckboxAccessories( LPCTSTR lpTabID, vecCDCAccessoryList *pvecAccessoryList );

	// Allow to enable some accessories that belongs to an identified group.
	// Param: 'lpTabID' is the ID of the owner group or NULL if we want to enable all accessories.
	// Param: 'pvecAccessoryList' is the list of all accessories.
	void EnableCheckboxAccessories( LPCTSTR lpTabID, vecCDCAccessoryList *pvecAccessoryList );

	// Allow to remove all accessories from a list (or some of them depending of the ruled table is passed as argument).
	// Param: 'pvecAccessoryList' is the list of all accessories.
	// Param: 'pRuledTable' if not NULL we have to remove only accessories with same ruled table.
	// Param: 'pclSSheetDescription' is the sheet description from which checkbox must be removed.
	void RemoveCheckboxAccessories( vecCDCAccessoryList *pvecAccessoryList, CSheetDescription *pclSSheetDescription, CDB_RuledTableBase *pRuledTable = NULL );

	// This method is called to gray incompatible adapter (at now only for BCV and PICV).
	// When user select a control valve, it's possible that there is some TA actuators available for it. But it's also possible to put actuator from other
	// manufacturer. To do that we just need an adapter between control valve and actuator. So when user select a control valve, we show all adapters and also
	// TA actuators. If user click on an adapter, we need to check if this one is compatible with displayed TA actuators or not. If it's not the case then
	// we grayed all actuators that are not compatible. In the same way if user click on an TA actuator, we have to run list of all displayed adapters to check
	// which ones can be used with control valve. Then all incompatible adapters will be also grayed.
	// Param: 'pAccessory' -> pointer on the current accessory (or adapter) concerned by the call.
	// Param: 'bChecked' -> set if accessory is selected or not.
	virtual void GrayUncompatibleAccessoriesCheckbox( CDB_Product *pclAdapter, bool bChecked ) {}
	
	///////////////////////////////////////////////////////////////////////////////////
	// HYS-2031
	// In the database to manage excluded products we do as follow:
	//   * In Cv accessory table (CDB_RuledTable where stem heater is listed),
	//     in the accessory line, we add exlusion sign before adding excluded product.
	//     Stem heater \ !Adapter.
	//    
	//   * In actuator adapter table(CDB_RuledTable where the adapter is listed),
	//     in the adapter line, we add exclusion sign before adding excluded product.
	//     Adapter \ !Stem heater
	// 
	// We look the two tables to manage exclusion
	// /////////////////////////////////////////////////////////////////////////////////
	// This function is created to manage adapter that is included in one product accessory (Stem heater).
	// the adapter is grayed out and unchecked when the stem heater is selected.
	// This is called when an action is needed on pclAccessory that is th estem heater.
	virtual void GrayOtherExcludedOrUncompatibleProduct( CDB_Product *pclAccessory, bool bChecked ) {}

	// End of methods to manage accessories.
	/////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

	
	/////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
	// Methods to manage Title group button.

	// Create button.
	// Param: 'lColumn' and 'lRow' -> cell coordinates where to set button.
	// Param: 'pclSSheetDescription' is the sheet description in which button must be added.
	CCDButtonTitleGroup *CreateTitleGroupButtons( long lFromColumn, long lToColumn, long lRow, CSheetDescription *pclSSheetDescription );

	// Allow to delete the button.
	// Param: 'pCDBTitleGroup' is the pointer on the button.
	// Param: 'pclSSheetDescription' is the sheet description from which button must be deleted.
	// Remark: PAY ATTENTION! 'pCDBTitleGroup' will be reseted to NULL!!!
	void DeleteTitleGroupButton( CCDButtonTitleGroup *&pCDBTitleGroup, CSheetDescription *pclSSheetDescription );

	// End of methods to manage Title group button.
	/////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////


	/////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
	// Methods to manage cell description link.

	// Create link.
	// Param: 'lColumn' and 'lRow' -> cell coordinates where to set the link.
	// Param: 'pclSSheetDescription' is the sheet description in which link must be added.
	CCellDescriptionLink *CreateCellDescriptionLink( long lColumn, long lRow, CSheetDescription *pclSSheetDescription, LPARAM lpParam );

	// Allow to delete the link.
	// Param: 'pCDLink' is the pointer on the link.
	// Param: 'pclSSheetDescription' is the sheet description from which link must be deleted.
	// Remark: PAY ATTENTION! 'pCDLink' will be reseted to NULL!!!
	void DeleteCellDescriptionLink( CCellDescriptionLink *&pCDLink, CSheetDescription *pclSSheetDescription );

	// This method is called by 'OnCellClicked' if the cell description is a link.
	virtual void OnCellDescriptionLinkClicked( CSheetDescription *pclSheetDescription, long lColumn, long lRow, LPARAM lpParam ) {}

	// End of methods to manage cell description link.
	/////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////


	// Allow to determine what is the next focused row from current position and in regards to current shift key state.
	// Param: 'pclSheetDescription' set in which sheet we are.
	// Param: 'lColumn' and 'lRow' is from where to start searching.
	// Param: 'bAllowLoseFocus' allow or forbid to loose the focus of the right view.
	// Param: 'lNewRow' is where to save the new focused row.
	// Param: 'pclNewSheetDescription' is the pointer of the new sheet if new focused row is not in the current sheet.
	// Param: 'bShiftPressed' to save the SHIFT key state.
	int DefineNextFocusedRow( CSheetDescription *pclCurrentSheetDescription, long lColumn, long lRow, bool bAllowLoseFocus, long &lNewRow, 
			CSheetDescription *&pclNewSheetDescription, bool &bShiftPressed );

	// Allow to prepare new focus depending of 'lColumn' and 'lRow'.
	// Param: 'pclSheetDescription' set in which sheet we have to prepare focus.
	// Param: 'lColumn' and 'lRow' are the current position from where to set the new focus.
	// Param: If 'lParam' is set to 0, the parameter is internally retrieved.
	// Remark: In fact this method calls the three private: '_KillFocus', '_PrepareFocus' and '_SetFocus'.
	void PrepareAndSetNewFocus( CSheetDescription *pclSheetDescription, long lColumn, long lRow, long lParam = 0 );

	// Kill the focus on the previous selection
	void KillCurrentFocus( void );

	// Allow to retrieve on which column is set the focus.
	long GetCurrentFocusedColumn( void );

	// Allow to retrieve on which row is set the focus.
	long GetCurrentFocusedRow( void );

	// Allow to retrieve what is the user preference for showing or not the no priority valves.
	bool GetShowAllPrioritiesFlag( int iButtonID );

	// Function that returns correct background color for group title (accessory, adapter, actuator, and so on).
	COLORREF GetTitleBackgroundColor( CSheetDescription *pclSheetDescription );

	// Following user actions it can happens that a sheet disappears. Because we alternate color for the titles,
	// we have to update the title in regards to these new changes.
	virtual void UpdateTitleBackgroundColor( CSheetDescription *pclSheetDescription ) {}

	// This method is called by inherited classes when selection state has changed.
	// Param: 'bIsReady' set to true to notify that selection is ready to be selected, otherwise set to false.
	// Remark: it's to allow 'DlgSelectionBase' to change the status of the 'Select' button.
	void SelectionHasChanged( bool bIsReady );

	// Allow to retrieve a product in the selected list (CDB_Product -> CSelectTAP).

	template<typename T>
	T *GetSelectProduct( CDB_Product *pclProduct, CSelectedProductList *pclSelectList )
	{
		if( NULL == pclProduct || NULL == pclSelectList )
		{
			return (T*)NULL;
		}

		T *pReturnSelectedProduct = NULL;
		
		for( CSelectedBase *pclSelectedProduct = pclSelectList->GetFirst<CSelectedBase>(); NULL != pclSelectedProduct; 
				pclSelectedProduct = pclSelectList->GetNext<CSelectedBase>() )
		{
			CDB_Product *pTAP = dynamic_cast<CDB_Product*>( pclSelectedProduct->GetpData() );

			if( pTAP == pclProduct )
			{
				pReturnSelectedProduct = dynamic_cast<T*>( pclSelectedProduct );
				break;
			}
		}
		return pReturnSelectedProduct;
	}

	// Helper to show tooltip on a cell when the text contained in it is truncated (when the cell has the 'Ellipses' property).
	void TextTipFetchEllipsesHelper( long lColumn, long lRow, CSheetDescription *pclSheetDescription, SHORT *pnTipWidth, CString *pStrText );

	// When right clicking on a cell and it's a 'CCellDescriptionProduct' product, if 'GetProduct' returns NULL, we can ask inherited class to interpret
	// user param if exist and it the class can do it.
	virtual CDB_Product *RetrieveProductFromUserParam( LPARAM lpParam ) { return NULL; }

// Private methods
private:
	// Prepare a new CRect focus in regards to current position.
	// Param: 'pclSheetDescription' set in which sheet we have to prepare focus.
	// Param: 'lColumn' and 'lRow' are the current position from where to set the new focus.
	// Param: If 'lParam' is set to 0, the parameter is internally retrieved.
	// Remark: preparation of focus is directly set in 'm_stCurrentFocus'!
	void _PrepareFocus( CSheetDescription *pclSheetDescription, long lColumn, long lRow, long lParam = 0 );

	// Save a new focus in the class member variables and set it
	void _SetFocus( void );

	// Function that will save the borders for the current focus 'm_stCurrentFocus' position.
	void _SaveFocusedCellBorders();
	
	// Function that restore the borders previously saved for current focus 'm_stCurrentFocus' position.
	void _RestoreFocusedCellBorders();

	enum ReturnRefreshType
	{
		RRT_NoRefreshNeeded	= 0,
		RRT_RefreshSheets	= 0x01,
		RRT_RefreshPosition = 0x02
	};

	// Called when user clicked on a box button.
	// Param: 'pclCDButton' is the pointer on all the infos linked to the button clicked.
	// Param: 'pclSheetDescription' pointer on the sheet description concerned by the click.
	// Return: 'ReturnRefreshType' is we need to update sheet size and scroll position.
	int _BoxButtonClicked( CCellDescriptionButton *pclCDButton, CSheetDescription *pclSheetDescription );

	// Called by 'OnCellClicked' to proceed when user has clicked on a Show/Hide all priorities button.
	// Return: 'ReturnRefreshType' is we need to update sheet size and scroll position.
	int _ShowAllPrioritiesArrowClicked( CCellDescription *pclCellDescription, CSheetDescription *pclSheetDescription );

	// Called by 'OnCellClicked' to proceed when user has clicked on a Expand/Collapse rows button.
	// Param: 'pclCellDescription' is the pointer on all the infos linked to the button clicked.
	// Param: 'pclSheetDescription' pointer on the sheet description concerned by the click.
	// Return: 'ReturnRefreshType' is we need to update sheet size and scroll position.
	int _ExpandCollapseRowsClicked( CCellDescription *pclCellDescription, CSheetDescription *pclSheetDescription );

	// Called by 'OnCellClicked' to proceed when user has clicked on a Expand/Collapse group button.
	// Param: 'pclCellDescription' is the pointer on all the infos linked to the button clicked.
	// Param: 'pclSheetDescription' pointer on the sheet description concerned by the click.
	// Return: 'ReturnRefreshType' is we need to update sheet size and scroll position.
	int _ExpandCollapseGroupClicked( CCellDescription *pclCellDescription, CSheetDescription *pclSheetDescription );

	// Called by 'OnCellClicked' to proceed when user has clicked on an accessory checkbox.
	// Param: 'pclCellDescription' is the pointer on all the infos linked to the button clicked.
	// Param: 'pclSheetDescription' pointer on the sheet description concerned by the click.
	// Return: 'ReturnRefreshType' is we need to update sheet size and scroll position.
	int _AccessoryCheckboxClicked( CCellDescription *pclCellDescription, CSheetDescription *pclSheetDescription );

	// Called by 'OnCellClicked' to proceed when user has clicked on an title group.
	// Param: 'pclCellDescription' is the pointer on all the infos linked to the button clicked.
	// Param: 'pclSheetDescription' pointer on the sheet description concerned by the click.
	// Return: 'ReturnRefreshType' is we need to update sheet size and scroll position.
	int _TitleGroupClicked( CCellDescription *pclCellDescription, CSheetDescription *pclSheetDescription );

	// Check if an accessory can be found in a list. Typically called by 'VerifyAccessoriesCheckbox'.
	// Param: 'pAccessory' is the accessory to search.
	// Param: 'pvecAccessoryList' is the list.
	// Return: NULL if not found, otherwise returns a 'CCTBCheckboxAccessory' that contains all data relative to this accessory.
	CCDBCheckboxAccessory *_IsAccessoryExistInList( CDB_Product *pAccessory, vecCDCAccessoryList *pvecAccessoryList );

	// Called by 'OnKeyDown' when user press a navigation key.
	// Param: 'pclSheetDescription' pointer on the sheet description concerned by the focus.
	// Param: 'wKeyPressed' is the key just pressed.
	void _CellFocused( CSheetDescription *pclSheetDescription, WPARAM wKeyPressed );

// Private variables
private:
	CProductSelelectionParameters *m_pclProdSelParams;
	CurrentFocus m_stCurrentFocus;
	bool m_bSetAccessoriesCheckboxRunning;
	CString m_strProductRightClicked;		// Only for ON_COMMAND (OnGetInfo and OnGetFullinfo).
	std::map<short, std::vector<INotificationHandler*>> m_mapNotificationHandlerList;
};
