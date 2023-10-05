#pragma once


#include "MessageManager.h"

class CMultiSpreadBase : public CMessageManager, public CSSheet::INotificationHandler
{
public:


	// Enum defined for calling 'SetScrollPosition'.
	enum ScrollPosition
	{
		NoScroll = -1,
		ScrollToCompute = 0,
		ScrollToTop = 1,
		ScrollToBottom = 2,
		ScrollUpOnePage = 3,
		ScrollDnOnePage = 4
	};

	CMultiSpreadBase( bool nUseOnlyOneSpread = true );
	virtual ~CMultiSpreadBase();

	virtual void Reset( void );

	// Returns 'true' if the current dialog contains only one sheet (it's the case for almost all pages except SelProgPagePM in which
	// there are several sheets.
	bool UseOnlyOneSpread( void ) { return m_bUseOnlyOneSpread; }
	
	// Returns the top sheet. 
	CSSheet *GetSheetPointer( void );

	void GetScrollingPos( int &iVertPos, int &iHorzPos );

	enum ScrollType
	{
		ST_Horz,
		ST_Vert
	};
	bool IsScrollingExist( ScrollType eScrollType );

	// Allow to retrieve what is the 'CSheetDescription', column & row under the mouse pointer.
	// Param: 'point' must be in screen coordinates.
	void GetInfoUnderMouse( CPoint ptMousePosScreen, CSheetDescription *&pclSheetDescription, long &lColumn, long &lRow );

	// Allow to retrieve the total size of all sheets.
	CRect GetTotalSize( void ) { return m_rectVirtualSize; }

	// Allow to copy all available sheet in only one sheet.
	void CopyMultiSheetInSheet( CSSheet *pclSheetDst );
	
	// To verify if there is some selection displayed.
	virtual bool IsEmpty( void );

// Protected members
protected:
	void BasePreTranslateMessage( MSG *pMsg );

	void BaseOnSize( UINT nType, int cx, int cy );
	void BaseOnHScroll( CWnd *pWnd, UINT nSBCode, UINT nPos, CScrollBar *pScrollBar );
	void BaseOnVScroll( CWnd *pWnd, UINT nSBCode, UINT nPos, CScrollBar *pScrollBar );
	LRESULT BaseMM_OnMessageReceive( WPARAM wParam, LPARAM lParam );
	LRESULT BaseTextTipFetch( WPARAM wParam, LPARAM lParam );

	/**
	 * Modified by HYS-816 to manage combo on spread. When having combo as a cell type we should return
	 * false to allow action on this cell 
	 */
	LRESULT BaseLeaveCell( WPARAM wParam, LPARAM lParam );
	LRESULT BaseRowHeightChange( WPARAM wParam, LPARAM lParam );

	void BaseOnUpdateMenuText( CCmdUI *pCmdUI );
	
	bool OnLeaveCell( CSheetDescription *pclSheetDescription, long lColumn, long lRow, long lNewColumn, long lNewRow );

	// Allow to get the pointer on 'CFormView' or on a 'CDialog' in regard to what is the inherited class.
	virtual CWnd *GetpWnd( void ) = 0;

	// Called by 'OnSSheetMouseEvent()' when user clicks on a cell.
	// Remark: must be implemented in inherited classes.
	virtual void OnCellClicked( CSheetDescription *pclSheetDescription, long lClickedCol, long lClickedRow ) = 0;

	// Called by 'OnSSheetMouseEvent()' when user double clicks on a cell.
	virtual void OnCellDblClicked( CSheetDescription *pclSheetDescription, long lClickedCol, long lClickedRow ) = 0;

	// Called by 'OnSSheetMouseEvent()' when user right clicks on a cell.
	virtual void OnRightClick( CSheetDescription *pclSheetDescription, long lClickedCol, long lClickedRow, long lXPos, long lYPos ) = 0;

	// Called by 'OnSSheetKeyboardEvent()' when user presses a key.
	// Return: 'TRUE' if message must not be dispatched.
	virtual BOOL OnKeyDown( CSheetDescription *pclSheetDescription, WPARAM wKeyPressed ) = 0;
	virtual BOOL OnKeyUp( CSheetDescription *pclSheetDescription, WPARAM wKeyPressed ) = 0;

	// Called by 'TextTipFetch' when a sheet wants to show a tip.
	virtual bool OnTextTipFetch( CSheetDescription *pclSheetDescription, long lColumn, long lRow, WORD *pwMultiLine, SHORT *pnTipWidth, TCHAR *pstrTipText, BOOL *pbShowTip ) = 0;

	virtual bool GetCurrentFocus( CSheetDescription *&pclSheetDescription, CRect &rectFocus ) = 0;

	/////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
	// Methods to manage sheets.
	
	// Allow to create a new sheet.
	// Param: 'uiSheetDescriptionID' is the unique ID to identify the new sheet description created.
	// Param: If 'uiAfterSheetDescriptionID' is not defined, the sheet created will be placed at the bottom. Otherwise, it will be inserted just after
	//        the sheet corresponding to 'uiAfterSheetDescriptionID' if exist.
	// Return: pointer on the new sheet description containing all needed data or NULL if any problem!
	// Remark: - PAY ATTENTION: there is a difference between sheet description ID and SSheet ID!! First one is set by user when creating
	//           CSheetDescription. Second one is the ID set when internally create a new instance of CSSheet class. The second one allow
	//           to identify in a window message for which CSSheet to deliver.
	//         - This method creates a new 'CSSheet' class with an ID computed in regards to current sheet description numbers already
	//           existing in 'm_ViewDescription'.
	//         - Pay attention that SSheet created will be deleted by 'm_ViewDescription'.
	CSheetDescription *CreateSSheet( UINT uiSheetDescriptionID, UINT uiAfterSheetDescriptionID = 0xFFFFFFFF );

	// Allow to hide a sheet.
	bool HideSSheet( UINT uiSheetDescriptionID );
	bool ShowSSheet( UINT uiSheetDescriptionID );
	
	// Allow to Enable/Disable drawing in all sheet or in a specific one.
	// Param: 'fEnable' set to 'TRUE' to enable.
	// Param: 'pclSheet' set if you want a specific sheet.
	void EnableSSheetDrawing( BOOL bEnable, CSSheet *pclSSheet = NULL );
	//
	// End of methods to manage sheets.

	// Allow to compute and resize all sheets in right view in regards to their current size.
	void SetSheetSize( bool bRedraw = true );

	// Allow to verify scrolling in regards to current focus.
	// Param: 'eScrollPosition' set if we must to go top/bottom or if it must be computed thanks to the two next parameters.
	// Param: 'pclSheetDescription' is the current sheet description that has the focus.
	// Param: 'rectFocus' is the focus rectangle coordinates in the sheet in pixel unit. The coordinates must be relative to the client area of
	//        the sheet.
	// Param: 'fTopPreference' is used when 'eScrollPosition' is set to 'ScrollToCompute'. If set to 'true', that means we compute the scrolling to set the top of the selection to
	//        the top of the visible area. Otherwise, set to 'false' to set the bottom of the selection to the bottom of the visible area.
	// Param: 'lRowReference' is used when 'eScrollPosition' is set to 'ScrollToCompute' and the 'm_fUseOnlyOneSpread' is set to 'true'. In this case,
	//        we have only on TSpread in the dialog/view and it is the TSpread itself that manage the scrolling. We just need the row to display.
	// Remark: this method can be called with 'rectSheetFocusInPixel' coordinates relative to the current window. To do that, simply pass NULL for
	//		   'pclSheetDescription'. Typically it's for the new 'SelProdPageBase' where we can have one selection that occupy more than one sheet description
	//         For example: the 'Pressure maintenance products' page can contain calculated products. Each calculated product is displayed with several sheet
	//         descriptions (Title, Input/Output and product). When a product is selected, the selection is done in the 'Input/Output' sheet and also in the
	//         'Product' sheet. This is why in this case, it's impossible to pass only one 'pclSheetDescription' and only one 'rectSheetFocusInPixel'. 
	void SetScrollPosition( ScrollPosition eScrollPosition, CSheetDescription *pclSheetDescription = NULL, CRect rectSheetFocusInPixel = CRect( 0, 0, 0, 0 ), bool bTopPreference = true, long lRowReference = 0 );

	// TODO Not yet active!
	void SetFreezeTopHeightPixel( int iHeight ) { m_iFreezeTopHeightPixels = 0; }
	int GetFreezeTopHeightPixel( void ) { return m_iFreezeTopHeightPixels; }

	// Overrides the SSheet::INotificationHandler methods.
	virtual void SSheetOnBeforePaint( void ) {}
	virtual void SSheetOnAfterPaint( void ) {}

// Private methods
private:
	// Returns false if do nothing.
	bool _CheckScrollbars( void );
	bool _IsSheetExist( HWND hSSheet, CSheetDescription *&pclSSheetDescription );

	// Allow to update the virtual size and to fix the Y offset value of each sheet description.
	void _UpdateVirtualSizeAndYOffset( std::vector<CRect> *pvecSheetSize = NULL );

// Protected variables
protected:
	CViewDescription m_ViewDescription;
	
	// Allow to manage the saving of column width.
	mapMapShortLong m_mapSSheetColumnWidth;
	bool m_bInitialised;
	CRect m_rectVirtualSize;
	CRITICAL_SECTION m_CriticalSection;
	bool m_bScrollbarRunning;

	int m_iFreezeTopHeightPixels;

	// When we use multispread mode in on dialog/view, there is one limitation due to TSpread. In fact, client area can't be higher
	// than 32767 pixels. It can be the case for SelProdPageBv in which there is only one spread that can have a lot of selection.
	// The reason why we use client rect for TSpread so big it's because if the multispread mode we don't want that each TSpread manage
	// itself their scrollbars. It's 'CMultiSpreadBase' that manage the scrollbars. And the client area of each TSpread in the dialog/view 
	// is resized to its maximum size.
	// To avoid to bypass 32767 pixels we have two solution. First, we split one big TSpread in two as soon as its height bypassed 32767 pixels
	// or is there is only on TSpread in dialog/view, we let the TSpread manage its own scrollbar. For the second solution, we set thus the 
	// client area of TSpread at the same dimension of the dialog/view. For the moment, we take this second solution.
	bool m_bUseOnlyOneSpread;
};
