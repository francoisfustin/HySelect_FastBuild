#pragma once


#include "afxwin.h"
#include "MyToolTipCtrl.h"
#include "DlgOutputSpreadContainer.h"


// Remark: 'BS' is for Button Special.
#define BS_PIXELS_EDGEHEADER_BUTTON		4			// Number of pixels between button and left/right edge of the header.
#define BS_PIXELS_EDGE_IMAGE			3			// Number of pixels between the left edge of the button and the image.
#define BS_PIXELS_BUTTONS				7			// Number of pixels between each button.
#define BS_PIXELS_LEFTEDGE_TEXT			4			// Number of pixels between left edge of the button and the text (always left aligned).
#define BS_PIXELS_RIGHTEDGE_TEXT		8			// Number of pixels between right edge of the button and the text.
#define BS_PIXELS_TOPBOTTOMEDGE_TEXT	2			// Number of pixels between top/bottom edge of the button and the text.

#define BS_COLOR_HIGHLIGHTED_BORDER_SOLID	RGB( 229, 195, 101 )	// Solid color when button is highlighted or pressed.
#define BS_COLOR_HIGHLIGHTED_TOP_START		RGB( 255, 252, 242 )	// Start gradient color for the top half part of the background button when it is highlighted.
#define BS_COLOR_HIGHLIGHTED_TOP_FINISH		RGB( 255, 243, 207 )	// Finish gradient color for the top half part of the background button when it is highlighted.
#define BS_COLOR_HIGHLIGHTED_BOTTOM_SOLID	RGB( 255, 236, 181 )	// Solid color for the bottom half part of the background button when it is highlighted.
#define BS_COLOR_PRESSED_SOLID				RGB( 255, 239, 187 )	// Solid color for the background button when it is in the pressed state.

// Remark: 'DO' is for Dialog Output.
#define DO_COLOR_BUTTON_SEPARATOR			RGB( 133, 145, 162 )	// Solid color for the thin separator between buttons.

class CDlgOutput : public CDialogEx, public CDlgOutputSpreadContainer::INotificationHandler
{
	DECLARE_DYNAMIC( CDlgOutput )

	class CVButton
	{
	public:
		enum Type{ Normal, Special };
		enum State{ NotExist, Unpressed, Pressed };
		CVButton( CDlgOutput *pDlgOutput ) { m_pDlgOutput = pDlgOutput; }
		virtual ~CVButton() {}
		virtual CWnd *GetpWnd() = 0;
		virtual int GetID() = 0;
		virtual State GetState() = 0;
		virtual void SetCaption( CString strCaption ) = 0;
		virtual void EnableButton( bool bEnable ) = 0;
		virtual bool IsEnable( void ) = 0;
	protected:
		CDlgOutput *m_pDlgOutput;
	};

	class CButtonNormal : public CMFCButton, public CVButton
	{
	public:
		CButtonNormal( CDlgOutput *pDlgOutput ) : CMFCButton(), CVButton( pDlgOutput ) {}
		virtual ~CButtonNormal() {}
		virtual CWnd *GetpWnd() { return this; }
		virtual int GetID() { return GetDlgCtrlID(); }
		virtual State GetState() { return State::Unpressed; }
		virtual void SetCaption( CString strCaption ) { SetWindowText( strCaption ); }
		virtual void EnableButton( bool bEnable ) { EnableWindow( true == bEnable ? TRUE : FALSE ); }
		virtual bool IsEnable( void ) { return ( WS_DISABLED == ( GetStyle() & WS_DISABLED ) ) ? false : true; }
	protected:
		DECLARE_MESSAGE_MAP()
		afx_msg void OnLButtonDown( UINT nFlags, CPoint point );
	};

	class CButtonSpecial: public CDialogEx, public CVButton
	{
	public:
		CButtonSpecial( CVButton::State eInitialState, CDlgOutput *pDlgOutput );
		virtual ~CButtonSpecial();
		void SetText( CString strText ) { m_strText = strText; }
		void SetBitmap( int iBitmapID ){ m_clBitmap.LoadBitmap( iBitmapID ); }
		void SetIcon( int iIconID );
		virtual CWnd *GetpWnd() { return this; }
		virtual int GetID() { return GetDlgCtrlID(); }
		virtual State GetState() { return m_eButtonState; }
		virtual void SetCaption( CString strCaption ) { m_strText = strCaption; }
		virtual void EnableButton( bool bEnable ) { EnableWindow( true == bEnable ? TRUE : FALSE ); }
		virtual bool IsEnable( void ) { return ( WS_DISABLED == ( GetStyle() & WS_DISABLED ) ) ? false : true; }
	protected:
		DECLARE_MESSAGE_MAP()
		afx_msg void OnPaint();
		afx_msg void OnLButtonDown( UINT nFlags, CPoint point );
		afx_msg void OnMouseMove( UINT nFlags, CPoint point );
		
	private:
		CString m_strText;
		CBitmap m_clBitmap;
		HICON m_hIcon;
		BITMAP m_rIconInfos;
		CRgn m_rgn;
		bool m_bHighlighted;
		State m_eButtonState;
	};

public:
	class IBaseNotificationHandler
	{
	public:
		enum
		{
			NH_First					= 0x0001,
			NH_OnCellClicked			= NH_First,
			NH_OnCellDblClicked			= ( NH_OnCellClicked << 1 ),
			NH_OnCellEditModeOff		= ( NH_OnCellDblClicked << 1 ),
			NH_Last						= NH_OnCellEditModeOff,
			NH_All						= ( NH_OnCellClicked | NH_OnCellDblClicked | NH_OnCellEditModeOff )
		};
		IBaseNotificationHandler() {}
		virtual ~IBaseNotificationHandler() {}
		virtual bool OnDlgOutputCellClicked( int iOutputID, int iColumnID, long lRowRelative ) = 0;
		virtual bool OnDlgOutputCellDblClicked( int iOutputID, int iColumnID, long lRowRelative ) = 0;
		virtual bool OnDlgOutputCellEditModeOff( int iOutputID, int iColumnID, long lRowRelative, bool bChangeMade ) = 0;
	};

	/*
	typedef struct _RowData
	{
		CString strData;
		LPARAM lpParam;
	}RowData;
	typedef std::map<int, RowData> mapRowData;		// Contains for each columnID the corresponding data.
	typedef mapRowData::iterator mapRowDataIter;
	*/
	
	// Structure uses only to retrieve column list available to display in the result output.
	typedef struct _ColData
	{
		long m_lColumnID;
		CString m_strName;
		bool m_bAlreadyDisplayed;
		bool m_bEnable;
		bool m_bSeparator;
		struct _ColData()
		{
			m_lColumnID = -1;
			m_strName = _T("");
			m_bAlreadyDisplayed = false;
			m_bEnable = true;
			m_bSeparator = false;
		}
	}ColData;
	typedef std::map<long, ColData> mapColData;
	typedef mapColData::iterator mapColDataIter;

	class CBaseInterface
	{
	public:
		CBaseInterface();
		virtual ~CBaseInterface() {}

		virtual bool Init( void ) = 0;
		
		// Completely clear the batch selection interface.
		virtual bool Reset( int iOutputID = -1 );

		virtual bool SetFocus( int iOutputID = -1 );

		virtual bool ClearOutput( bool bAlsoHeader = false, int iOutputID = -1 );
		virtual bool BlockOutput( int iOutputID = -1 );
		virtual bool ReleaseOutput( int iOutputID = -1 );
		virtual bool BlockRedraw( int iOutputID = -1 );
		virtual bool ReleaseRedraw( int iOutputID = -1 );
		virtual bool IsRedrawBlocked( int iOutputID = -1 );
		
		// Allow to enable or disable the edition of cells.
		virtual bool EnableEdition( bool bEnable, int iOutputID = -1 );
		virtual bool IsEditionEnabled( int iOutputID = -1 );

		// Check if a specific cell is editable.
		// Remark: don't take into account the fact that edition is enabled or not. Here, we return only if cell is editable or not.
		virtual bool IsCellEditable( int iColID, long lRowRelative, bool &bIsEditable, int iOutputID = -1 );

		// Force the current cell to enter in edit mode.
		// Remark: if global edition has been disable by calling 'EnableEdition', this method has no action.
		virtual bool EditCurrentCell( int iOutputID = -1 );

		// Allow to reset horizontal and vertical scrolling to the top left part of the screen.
		virtual bool ResetScrolling( int iOutputID = -1 );

		// If 'true', columns that are set as auto resizable (see 'DlgOutputHelper::CColDef::m_fAutoResized') will be resized to occupy all the 
		// available space.
		virtual bool AutomaticColumnResize( bool bAutomaticColumnResize, int iOutputID = -1 );

		// If 'true', a blank line will be automatically added at the end.
		virtual bool AutomaticLastBlankLine( bool bEnable, int iOutputID = -1 );

		// Change the selection background color.
		virtual bool SetSelectionColor( COLORREF backgroundColor, int iOutputID = -1 );
	
		// Enable the possibility to select a range of cells.
		virtual bool EnableBlockSelection( bool bEnable, WORD wBorderStyle = SS_BORDERSTYLE_SOLID, COLORREF backgroundColor = _TAH_SELECTED_ORANGE, int iOutputID = -1 );

		// Check if a block selection is currently active.
		virtual bool IsBlockSelectionExist( int iOutputID = -1 );
		
		// Retrieve the current block selection in RELATIVE coordinates.
		virtual bool GetBlockSelection( int &iStartColID, long &lStartRow, int &iEndColID, long &lEndRow, int iOutputID = -1 );

		// Allow to manually set the block selection.
		// Returns 'false' if it's no possible with coordinates passed as arguments.
		virtual bool SetBlockSelection( int iStartColID, long lStartRow, int iEndColID, long lEndRow, int iOutputID = -1 );
		
		// If a block selection is active (with multiple rows or/and columns), we reset the block selection to the origin cell (where selection
		// has started).
		virtual bool ResetBlockSelectionToOrigin( int iOutputID = -1 );
		
		// If block selection is displayed, we erase it and reset parameters concerned by it.
		virtual bool EraseBlockSelection( int iOutputID = -1 );
		
		// Enable the possibility to select a complete row by clicking in the left part of the 'iColumnIDRef' column.
		virtual bool EnableRowSelection( bool bEnable, int iColumnIDRef, COLORREF backgroundColor = _TAH_SELECTED_ORANGE, int iOutputID = -1 );
		
		virtual bool SetColumnWidth( int iColumnID, double dWidth, int iOutputID = -1 );
		virtual bool SetColumnWidthInPixel( int iColumnID, long lWidth, int iOutputID = -1 );
		virtual bool GetColumnWidth( int iColumnID, double &dWidth, int iOutputID = -1 );
		virtual bool GetColumnWidthInPixel( int iColumnID, long &lWidth, int iOutputID = -1 );

		virtual bool SetDataRowHeight( double dRowHeight, int iOutputID = -1 );

		virtual bool GetColumnHeaderRowsAbsolute( long &lColumnAbsoluteStart, long &lColumnAbsoluteEnd, int iOutputID = -1 );

		// Allow to enable/disable a specific button in the top panel of the batch selection output.
		virtual bool EnableTopPanelButton( int iButtonID, bool bEnable, int iOutputID = -1 );
		
		// Set to 'true' to enable event 'Text tip fetch' from TSpread.
		virtual bool EnableTextTipFetch( bool bSet, int iOutputID = -1 );
		
		// Add a trigger for a text tip fetch.
		// It's the 'CDlgOutputSpreadContainer' destructor that will delete the 'pclCellBase'.
		virtual bool AddTextTipFetchTrigger( DlgOutputHelper::CCellBase *pclCellBase, long lRow, CString strText, int iOutputID = -1 );

		virtual bool RemoveAllTextTipFetchTrigger( int iOutputID = -1 );

		virtual bool AddBlankLineBeforeMainHeader( double dHeight, int iOutputID = -1 );

		// Typically, use this method if you have previously added headers in the description. This method will automatically insert a row at the first 
		// position and merge all columns belonging to the header. It's to have a group with title above the header (like in individual selection).
		// Param: 'iMainHeaderID' to identify after.
		virtual bool AddMainHeaderDef( int iMainHeaderID, int iMainHeaderSubID, DlgOutputHelper::CColDef &clColDef, CString strText, int iOutputID = -1 );

		virtual bool AddMainHeaderFontDef( int iMainHeaderID, int iMainHeaderSubID, DlgOutputHelper::CFontDef &clFontDef, int iOutputID = -1 );

		virtual bool SetMainHeaderRowHeight( int iMainHeaderID, double dHeight, int iOutputID = -1 );

		virtual bool SetColumnHeaderRows( long lRow, int iOutputID = -1 );

		// Set to 'true' to freeze all available header rows.
		virtual bool FreezeAllHeaderRows( bool bFreeze, int iOutputID = -1 );
		
		// Add a header description for a specific column.
		// Param: 'iColumnID' to identify after.
		// Param: 'clColDef' contains all needed data (see description in 'DlgOutputHelper.h'.
		// Remark: If there is more than one row for header, all cells in each row will have this description.
		virtual bool AddColumnHeaderDef( int iColumnID, DlgOutputHelper::CColDef &clColDef, int iOutputID = -1 );
		
		// Add a header font description. Same remark as 'AddColumnHeaderDef'.
		// Remark: If there is more than one row for header, all cells in each row will have this description.
		virtual bool AddColumnHeaderFontDef( int iColumnID, DlgOutputHelper::CFontDef &clFontDef, int iOutputID = -1 );	

		virtual bool FillColumnHeader( int iColumnID, long lRow, CString strData, double dHeight, bool bLineBelow = false, int iOutputID = -1 );

		virtual bool AddColumnDefinition( int iColumnID, DlgOutputHelper::CColDef &clColDef, int iOutputID = -1 );
		
		virtual bool RemoveColumn( int iColumnID, int iOutputID = -1 );
		
		virtual bool AddColumnFontDefiniton( int iColumnID, DlgOutputHelper::CFontDef &clFontDef, int iOutputID = -1 );
		
		// Returns number of rows used by data (not included headers).
		bool GetRows( long &lRows, int iOutputID = -1 );
		
		// Returns number of rows used by data and headers.
		bool GetTotalRows( long &lTotalRows, int iOutputID = -1 );

		// Allow to retrieve all data on a row. Data are saved in the map (Key is the column ID and value is a structure containing
		// the date in a CString format).
		// Param: 'lCurrentRow' to retrieve the current row.
		virtual bool GetFirstRow( DlgOutputHelper::mapIntCellBase *pmapCellData, long &lCurrentRowRelative, int iOutputID = -1 );
		virtual bool GetNextRow( DlgOutputHelper::mapIntCellBase *pmapCellData, long &lCurrentRowRelative, int iOutputID = -1 );
		virtual bool GetRow( DlgOutputHelper::mapIntCellBase *pmapCellData, long lRowRelative, int iOutputID = -1 );
		virtual bool IsRowValidForData( long lRowRelative, int iOutputID = -1 );
		virtual bool SelectRow( long lRowRelative, bool bAutoScroll = false, int iOutputID = -1 );
		virtual bool GetSelectedRow( long &lCurrentSelectedRowRelative, int iOutputID = -1 );

		// Allow to save all the current selections.
		virtual bool SaveSelections( CSSheet::SaveSelectionChoice eWhichSelection = CSSheet::SSC_All, int iOutputID = -1 );

		// Allow to clear all the current selections.
		virtual bool ClearCurrentSelections( CSSheet::SaveSelectionChoice eWhichSelection = CSSheet::SSC_All, int iOutputID = -1 );
	
		// Restore previously selections saved.
		virtual bool RestoreSelections( int iOutputID = -1 );

		virtual bool GetColID2Absolute( int iColumnID, long &lColAbsolute, int iOutputID = -1 );
		
		// Allow to reset the content of a specific cell if 'lToRowRelative' is set to -1 or a range of cells.
		virtual bool ResetCellContent( int iColumnID, long lFromRowRelative, long lToRowRelative = -1, int iOutputID = -1 );
		
		// Change the background color of a specific cell.
		// Param: 'fForceColorWhenSelection' set to true if this background color must be kept even if a selection occurs in this cell.
		virtual bool ChangeCellBackground( int iStartColID, int iEndColID, long lStartRowRelative, long lEndRowRelative, COLORREF crColor, 
				bool bForceColorWhenSelection = false, int iOutputID = -1 );

		// Enable or not the possibility to user to resize manually the column (by clicking and dragging on the separator in the title).
		virtual bool ResizingColumnActivate( bool bActivate, int iOutputID = -1 );
		virtual bool ResizingColumnAddRange( long lColStartAbsolutePos, long lColEndAbsolutePos, long lRowStart, long lRowEnd, int iOutputID = -1 );
		virtual bool ResizingColumnRemoveAll( int iOutputID = -1 );

		bool RegisterNotificationHandler( IBaseNotificationHandler *pclHandler, int iNotificationHandlerFlags, int iOutputID = -1 );
		bool UnregisterNotificationHandler( IBaseNotificationHandler *pclHandler, int iOutputID = -1 );

	protected:
		CDlgOutput *m_pDlgOutput;
	};

	enum { IDD = IDD_DLGOUTPUT };

	CDlgOutput( bool bCreateOutput, int iOutputID = -1, bool bUseSpreadContainer = true );
	virtual ~CDlgOutput();

	bool ActivateOutput( int iOutputID, bool bShow = true );
	int GetCurrentOutput( void ) { return m_iOutputID; }
	bool IsOutputExist( int iOutputID );

	//////////////////////////////////////////////////////////////////////////////////////////////////////////////
	// Methods declared in the 'CBaseInterface' class.

	// Completely clear the interface.
	virtual bool Reset( int iOutputID = -1 );

	virtual bool SetFocus( int iOutputID = -1 );

	virtual bool ClearOutput( bool bAlsoHeader = false, int iOutputID = -1 );
	virtual bool BlockOutput( int iOutputID = -1 );
	virtual bool ReleaseOutput( int iOutputID = -1 );
	virtual bool BlockRedraw( int iOutputID = -1 );
	virtual bool ReleaseRedraw( int iOutputID = -1 );
	virtual bool IsRedrawBlocked( int iOutputID = -1 );

	// Allow to enable or disable the edition of cells.
	virtual bool EnableEdition( bool bEnable, int iOutputID = -1 );
	virtual bool IsEditionEnabled( int iOutputID = -1 );

	// Check if a specific cell is editable.
	// Remark: don't take into account the fact that edition is enabled or not. Here, we return only if cell is editable or not.
	virtual bool IsCellEditable( int iColID, long lRowRelative, bool &bIsEditable, int iOutputID = -1 );

	// Force the current cell to enter in edit mode.
	// Remark: if global edition has been disable by calling 'EnableEdition', this method has no action.
	virtual bool EditCurrentCell( int iOutputID = -1 );

	// Allow to reset horizontal and vertical scrolling to the top left part of the screen.
	virtual bool ResetScrolling( int iOutputID = -1 );
	
	// If 'true', columns that are set as auto resizable (see 'DlgOutputHelper::CColDef::m_fAutoResized') will be resized to occupy all the 
	// available space.
	virtual bool AutomaticColumnResizing( bool bAutomaticColumnResize, int iOutputID = -1 );

	// If 'true', a blank line will be automatically added at the end.
	virtual bool AutomaticLastBlankLine( bool bEnable, int iOutputID = -1 );

	// Change the selection background color.
	virtual bool SetSelectionColor( COLORREF backgroundColor, int iOutputID = -1 );
	
	// Enable the possibility to select a range of cells.
	virtual bool EnableBlockSelection( bool bEnable, WORD wBorderStyle = SS_BORDERSTYLE_SOLID, COLORREF backgroundColor = _TAH_SELECTED_ORANGE, int iOutputID = -1 );

	// Check if a block selection is currently active.
	virtual bool IsBlockSelectionExist( int iOutputID = -1 );

	// Retrieve the current block selection in RELATIVE coordinates.
	virtual bool GetBlockSelection( int &iStartColID, long &lStartRow, int &iEndColID, long &lEndRow, int iOutputID = -1 );

	// Allow to manually set the block selection.
	// Returns 'false' if it's no possible with coordinates passed as arguments.
	virtual bool SetBlockSelection( int iStartColID, long lStartRow, int iEndColID, long lEndRow, int iOutputID = -1 );
	
	// If a block selection is active (with multiple rows or/and columns), we reset the block selection to the origin cell (where selection
	// has started).
	virtual bool ResetBlockSelectionToOrigin( int iOutputID = -1 );
	
	// If block selection is displayed, we erase it and reset parameters concerned by it.
	virtual bool EraseBlockSelection( int iOutputID = -1 );
	
	// Enable the possibility to select a complete row by clicking in the left part of the 'iColumnIDRef' column.
	virtual bool EnableRowSelection( bool bEnable, int iColumnIDRef, COLORREF backgroundColor = _TAH_SELECTED_ORANGE, int iOutputID = -1 );
	
	virtual bool SetColumnWidth( int iColumnID, double dWidth, int iOutputID = -1 );
	virtual bool SetColumnWidthInPixel( int iColumnID, long lWidth, int iOutputID = -1 );
	virtual bool GetColumnWidth( int iColumnID, double &dWidth, int iOutputID = -1 );
	virtual bool GetColumnWidthInPixel( int iColumnID, long &lWidth, int iOutputID = -1 );

	virtual bool SetDataRowHeight( double dHeight, int iOutputID = -1 );

	// Allow to retrieve the absolute position of the start and end of the column headers.
	// Remarks: this method must be called after all initialization.
	virtual bool GetColumnHeaderRowsAbsolute( long &lColumnAbsoluteStart, long &lColumnAbsoluteEnd, int iOutputID = -1 );

	// Allow to enable/disable a specific button in the top panel of the batch selection output.
	virtual bool EnableTopPanelButton( int iButtonID, bool bEnable, int iOutputID = -1 );

	// Set to 'true' to enable event 'Text tip fetch' from TSpread.
	virtual bool EnableTextTipFetch( bool bSet, int iOutputID = -1 );
	
	// Add a trigger for a text tip fetch.
	// It's the 'CDlgOutputSpreadContainer' destructor that will delete the 'pclCellBase'.
	virtual bool AddTextTipFetchTrigger( DlgOutputHelper::CCellBase *pclCellBase, long lRow, CString strText, int iOutputID = -1 );

	virtual bool RemoveAllTextTipFetchTrigger( int iOutputID = -1 );

	virtual bool AddBlankLineBeforeMainHeader( double dHeight, int iOutputID = -1 );

	// Typically, use this method if you have previously added headers in the description. This method will automatically insert a row at the first 
	// position and merge all columns belonging to the header. It's to have a group with title above the header (like in individual selection).
	// Param: 'iMainHeaderID' to identify after.
	virtual bool AddMainHeaderDef( int iMainHeaderID, int iMainHeaderSubID, DlgOutputHelper::CColDef &clColDef, CString strText, int iOutputID = -1 );

	virtual bool AddMainHeaderFontDef( int iMainHeaderID, int iMainHeaderSubID, DlgOutputHelper::CFontDef &clFontDef, int iOutputID = -1 );	

	virtual bool SetMainHeaderRowHeight( int iMainHeaderID, double dHeight, int iOutputID = -1 );

	virtual bool SetColumnHeaderRows( long lRow, int iOutputID = -1 );

	// Set to 'true' to freeze all available header rows.
	virtual bool FreezeAllHeaderRows( bool bFreeze, int iOutputID = -1 );
	
	// Add a header description for a specific column.
	// Param: 'iColumnID' to identify after.
	// Param: 'clColDef' contains all needed data (see description in 'DlgOutputHelper.h'.
	// Remark: If there is more than one row for header, all cells in each row will have this description.
	virtual bool AddColumnHeaderDef( int iColumnID, DlgOutputHelper::CColDef &clColDef, int iOutputID = -1 );
		
	// Add a header font description. Same remark as 'AddColumnHeaderDef'.
	// Remark: If there is more than one row for header, all cells in each row will have this description.
	virtual bool AddColumnHeaderFontDef( int iColumnID, DlgOutputHelper::CFontDef &clFontDef, int iOutputID = -1 );	

	virtual bool FillColumnHeader( int iColumnID, long lRow, CString strData, double dHeight, bool bLineBelow = false, int iOutputID = -1 );

	virtual bool AddColumnDefinition( int iColumnID, DlgOutputHelper::CColDef &clColDef, int iOutputID = -1 );
		
	virtual bool RemoveColumn( int iColumnID, int iOutputID = -1 );
		
	virtual bool AddColumnFontDefiniton( int iColumnID, DlgOutputHelper::CFontDef &clFontDef, int iOutputID = -1 );

	// Returns row used by data (not included headers).
	virtual bool GetRows( long &lRows, int iOutputID = -1 );
	// Returns row used by data and headers.
	virtual bool GetTotalRows( long &lTotalRows, int iOutputID = -1 );

	// Allow to retrieve all data on a row. Data are saved in the map (Key is the column ID and value is a structure containing
	// the date in a CString format).
	// Param: 'lCurrentRow' to retrieve the current row.
	virtual bool GetFirstRow( DlgOutputHelper::mapIntCellBase *pmapCellData, long &lCurrentRowRelative, int iOutputID = -1 );
	virtual bool GetNextRow( DlgOutputHelper::mapIntCellBase *pmapCellData, long &lCurrentRowRelative, int iOutputID = -1 );
	virtual bool GetRow( DlgOutputHelper::mapIntCellBase *pmapCellData, long lRowRelative, int iOutputID = -1 );
	virtual bool IsRowValidForData( long lRowRelative, int iOutputID = -1 );
	virtual bool SelectRow( long lRowRelative, bool bAutoScroll = false, int iOutputID = -1 );
	virtual bool GetSelectedRow( long& lCurrentSelectedRowRelative, int iOutputID = -1 );

	// Allow to save all the current selections.
	virtual bool SaveSelections( CSSheet::SaveSelectionChoice eWhichSelection = CSSheet::SSC_All, int iOutputID = -1 );

	// Allow to clear all the current selections.
	virtual bool ClearCurrentSelections( CSSheet::SaveSelectionChoice eWhichSelection = CSSheet::SSC_All, int iOutputID = -1 );
	
	// Restore previously selections saved.
	virtual bool RestoreSelections( int iOutputID = -1 );

	virtual bool GetColID2Absolute( int iColumnID, long &lColAbsolute, int iOutputID = -1 );
	
	// Allow to reset the content of a specific cell if 'lToRowRelative' is set to -1 or a range of cells.
	virtual bool ResetCellContent( int iColumnID, long lFromRowRelative, long lToRowRelative = -1, int iOutputID = -1 );

	// Change the background color of a specific cell.
	// Param: 'fForceColorWhenSelection' set to true if this background color must be kept even if a selection occurs in this cell.
	virtual bool ChangeCellBackground( int iStartColID, int iEndColID, long lStartRowRelative, long lEndRowRelative, COLORREF crColor, bool bForceColorWhenSelection = false, int iOutputID = -1 );

	// Enable or not the possibility to user to resize manually the column (by clicking and dragging on the separator in the title).
	virtual bool ResizingColumnActivate( bool bActivate, int iOutputID = -1 );
	virtual bool ResizingColumnAddRange( long lColStartAbsolutePos, long lColEndAbsolutePos, long lRowStart, long lRowEnd, int iOutputID = -1 );
	virtual bool ResizingColumnRemoveAll( int iOutputID = -1 );

	void RegisterNotificationHandler( IBaseNotificationHandler *pclHandler, int iNotificationHandlerFlags, int iOutputID = -1 );
	void UnregisterNotificationHandler( IBaseNotificationHandler *pclHandler, int iOutputID = -1 );
	
	// End of methods declared in the 'CBaseInterface' class.
	//////////////////////////////////////////////////////////////////////////////////////////////////////////////


	// Methods to personalize the header buttons.
	// Normal button.
	// Param: 'iButtonID' -> ID that is used when message is sent to notify an event on the button.
	// Param: 'iStrID' -> ID of the text to display on the button or -1 if nothing.
	// Param: 'iImageID' -> ID of the bitmap to display on the button or -1 if nothing.
	// Param: 'iImageType' -> can be BS_BITMAP or BS_ICON.
	// Param: 'bLeftAlign' -> 'true' if button must be set to the left or 'false' if must be set to the right.
	// Param: 'sizeButton' -> define the size of the button of CSize(0) to take default.
	bool AddTopPanelButton( int iButtonID, int iStrID, int iImageID, int iImageType, bool bLeftAlign, CSize sizeButton = CSize(0) );

	// The same but directly with a CString instead of an ID.
	bool AddTopPanelButton( int iButtonID, CString strText, int iImageID, int iImageType, bool bLeftAlign, CSize sizeButton = CSize(0) );
	
	// Button that stays in the pressed state (as a checkbox).
	bool AddTopPanelPushButton( int iButtonID, CVButton::State eInitialState, int iStrID, int iImageID, int iImageType, bool bLeftAlign, CSize sizeButton = CSize(0) );
	bool AddTopPanelPushButton( int iButtonID, CVButton::State eInitialState, CString strCaption, int iImageID, int iImageType, bool bLeftAlign, CSize sizeButton = CSize(0) );
	
	void UpdateTopPanelButtonCaption( int iButtonID, CString strCaption );

	CVButton::State GetTopPanelButtonState( int iButtonID );

	// No 'iOutputID' because if we receive this event it's obviously the output that is activated (m_iOutputID).
	virtual void OnHeaderButtonClicked( int iButtonID, CVButton::State eState ) {}

	// Overrides 'CDlgOutputSpreadContainer::IOutputSpreadNotificationHandler'.
	virtual void OnOutputSpreadContainerCellClicked( int iOutputID, int iColumnID, long lRow, CPoint ptMouse, DlgOutputHelper::vecCCellParam *pVecParamList );
	virtual void OnOutputSpreadContainerCellDblClicked( int iOutputID, int iColumnID, long lRow, CPoint ptMouse, DlgOutputHelper::vecCCellParam *pVecParamList );
	virtual void OnOutputSpreadContainerCellRClicked( int iOutputID, int iColumnID, long lRow, CPoint ptMouse, DlgOutputHelper::vecCCellParam *pVecParamList ) {}
	virtual void OnOutputSpreadContainerCellEditModeOff( int iOutputID, int iColumnID, long lRow, bool bChangeMade );
	virtual void OnOutputSpreadContainerKeyboardShortcut( int iOutputID, int iKeyboardShortcut ) {}
	virtual void OnOutputSpreadContainerKeyboardVirtualKey( int iOutputID, int iKeyboardVirtualKey ) {}

protected:
	DECLARE_MESSAGE_MAP()
	virtual void DoDataExchange( CDataExchange *pDX );
	virtual BOOL OnInitDialog();
	afx_msg void OnSize( UINT nType, int cx, int cy );
	afx_msg void OnPaint();
	afx_msg void OnCancel();
	afx_msg void OnOK();

	bool AddOutput( int iOutputID = -1 );
	virtual void OnDlgOutputAdded( int iOutputID, CDlgOutputSpreadContainer *pclDlgOutputSpreadContainer ) {}
	virtual void OnDlgOutputActivated( int iOutputID ) { m_iOutputID = iOutputID; }

// Private methods.
private:
	bool _AddTopPanelButton( CVButton::Type eButtonType, int iButtonID, CVButton::State eInitialState, CString strCaption, int iImageID, 
			int iImageType, bool bLeftAlign, CSize sizeButton = CSize(0) );

	void _ClearAllTopPanelButtons( void );
	CVButton *_GetTopPanelButton( int iButtonID );

// Protected variables.
protected:
	typedef std::map<int, IBaseNotificationHandler *> mapIntNotif;
	typedef mapIntNotif::iterator mapIntNotifIter;
	typedef struct _OutputContextParams
	{
		CDlgOutputSpreadContainer *m_pclDlgOutputSpreadContainer;
		mapIntNotif m_mapBaseNotificationHandlerList;
		std::map<CVButton *, bool> m_mapTopPanelButtonState;
		struct _OutputContextParams()
		{
			m_pclDlgOutputSpreadContainer = NULL;
		}
	}OutputContextParams;

	// The key is the 'OutputID'.
	typedef std::map<int, OutputContextParams> mapOutputContext;
	typedef mapOutputContext::iterator iterOutputContext;
	mapOutputContext m_mapOutputContextParams;

	CDlgOutputSpreadContainer *m_pclCurrentDlgOutputSpreadContainer;
	int m_iOutputID;

// Private variables.
private:
	bool m_bCreateOutput;
	bool m_bUseSpreadContainer;
	CMyToolTipCtrl m_ToolTip;
	std::vector<CVButton *> m_vecHeaderButtonLeftList;
	std::vector<CVButton *> m_vecHeaderButtonRightList;
	CRect m_rectHeader;
	WORD m_arwTemplate[1024];
};
