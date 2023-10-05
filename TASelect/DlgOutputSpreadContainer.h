#pragma once

#include "SSheet.h"
#include "RViewDescription.h"
#include "DlgOutputHelper.h"
#include "MessageManager.h"

class CDlgOutputSpreadContainer : public CDialogEx, public CMessageManager
{
	DECLARE_DYNAMIC( CDlgOutputSpreadContainer )

public:

	// Remark: these notifications are used to notify events from 'CDlgOutputSpreadContainer' to its parent 'CDlgOutput'.
	class INotificationHandler
	{
	public:
		enum
		{
			NH_First					= 0x01,
			NH_OnCellClicked			= NH_First,								// Message sent when user double click on a cell.
			NH_OnCellSelected			= ( NH_OnCellClicked << 1 ),			// Message sent when a row is selected (by click or by navigation keys).
			NH_OnCellRClicked			= ( NH_OnCellSelected << 1 ),			// Message sent when user right click on a cell.
			NH_OnCellEditModeOff		= ( NH_OnCellRClicked << 1 ),			// Message sent when user has finished to edit the content of a cell.
			NH_OnKeyboardShortcut		= ( NH_OnCellEditModeOff << 1 ),		// Message sent when keyboard shortcut was used.
			NH_OnKeyboardVirtualKey		= ( NH_OnKeyboardShortcut << 1 ),		// Message sent when keyboard virtual key was used.
			NH_Last						= NH_OnKeyboardVirtualKey,
			NH_All						= ( NH_OnCellClicked | NH_OnCellSelected | NH_OnCellRClicked | NH_OnCellEditModeOff | NH_OnKeyboardShortcut | NH_OnKeyboardVirtualKey )
		};
		INotificationHandler() {}
		~INotificationHandler() {}
		virtual void OnOutputSpreadContainerCellClicked( int iOutputID, int iColumnID, long lRow, CPoint ptMouse, DlgOutputHelper::vecCCellParam* pVecParamList ) = 0;
		virtual void OnOutputSpreadContainerCellDblClicked( int iOutputID, int iColumnID, long lRow, CPoint ptMouse, DlgOutputHelper::vecCCellParam* pVecParamList ) = 0;
		virtual void OnOutputSpreadContainerCellRClicked( int iOutputID, int iColumnID, long lRow, CPoint ptMouse, DlgOutputHelper::vecCCellParam* pVecParamList ) = 0;
		virtual void OnOutputSpreadContainerCellEditModeOff( int iOutputID, int iColumnID, long lRow, bool fChangeMade ) = 0;
		virtual void OnOutputSpreadContainerKeyboardShortcut( int iOutputID, int iKeyboardShortcut ) = 0;
		virtual void OnOutputSpreadContainerKeyboardVirtualKey( int iOutputID, int iKeyboardVirtualKey ) = 0;
	};

	enum { IDD = IDD_DLGOUTPUTSPREAD };

	CDlgOutputSpreadContainer( int iOutputID, CWnd* pParent = NULL );
	virtual ~CDlgOutputSpreadContainer();

	void Reset();

	// Register an handler to manage notification initiated by 'CDlgOutputSpreadContainer'.
	void RegisterNotificationHandler( INotificationHandler *pclHandler, short nNotificationHandlerFlags );
	
	// Unregister an handler.
	void UnregisterNotificationHandler( INotificationHandler *pclHandler );

	// Allow to block the redrawing (avoid flicking when preparing the output).
	bool BlockRedraw( void );
	bool ReleaseRedraw( bool fUpdateLayout = true );
	bool IsRedrawBlocked( void );

	// Allows to block output. It means that all message are well sent to the spread but not displayed.
	// When calling 'ReleaseOuput', these messages become all visible.
	void BlockOutput( void );
	void ReleaseOutput( void );

	// Allow to enable or disable the edition of cells.
	bool EnableEdition( bool fEnable );
	bool IsEditionEnabled( void );

	// Check if a specific cell is editable.
	// Remark: don't take into account the fact that edition is enabled or not. Here, we return only if cell is editable or not.
	bool IsCellEditable( int iColID, long lRowRelative, bool& fIsEditable );

	// Force the current cell to enter in edit mode.
	// Remark: if global edition has been disable by calling 'EnableEdition', this method has no action.
	bool EditCurrentCell( void );

	// Allow to reset horizontal and vertical scrolling to the top left part of the screen.
	bool ResetScrolling( void );

	// If 'true', columns that are set as auto resizable (see 'DlgOutputHelper::CColDef::m_fAutoResized') will be resized to occupy all the 
	// available space.
	void AutomaticColumnResizing( bool fAutomaticColumnResize );
	
	// If 'true', a blank line will be automatically added at the last position.
	void AutomaticLastBlankLine( bool fAutomaticLastBlankLine );

	void EnableTextTipFetch( bool fSet ) { m_fTextTipFetchEnabled = fSet; }
	void SetSelectionColor( COLORREF backgroundColor ) { m_DefaultSelectionColor = backgroundColor; }

	void AddTextTipFetchTrigger( DlgOutputHelper::CCellBase* pclCellBase, long lRow, CString strText );
	void RemoveAllTextTipFetchTrigger( void );
	
	bool AddBlankLineBeforeMainHeader( double dHeight );

	// Typically, use this method if you have previously added headers in the description. This method will automatically insert a row at the first 
	// position and merge all columns belonging to the header. It's to have a group with title above the header (like in individual selection).
	// Remark: 'iMaingHeaderID' set to which main header row we want to add this new main header.
	//         'iSubMainHeaderID' set a unique identifier to allow to group some main header or column header following the current one.
	bool AddMainHeaderDef( int iMainHeaderID, int iMainHeaderSubID, DlgOutputHelper::CColDef& clColDef, CString strText );
	bool AddMainHeaderFontDef( int iMainHeaderID, int iMainHeaderSubID, DlgOutputHelper::CFontDef& clFontDef );
	bool SetMainHeaderRowHeight( int iMainHeaderID, double dHeight );

	void SetColumnHeaderRows( long lRow );
	bool FreezeAllHeaderRows( bool fFreeze );
	long GetHeaderRows( void ) { return m_lColumnHeaderRows; }

	bool AddColumnHeaderDef( int iColumnID, DlgOutputHelper::CColDef& clColDef );
	bool AddColumnHeaderFontDef( int iColumnID, DlgOutputHelper::CFontDef& clFontDef );
	bool FillColumnHeader( int iColumnID, long lRowRelative, CString strData, double dRowHeight, bool fLineBelow = false );

	bool AddColumnDefinition( int iColumnID, DlgOutputHelper::CColDef& clColDef );
	bool AddColumnFontDefinition( int iColumnID, DlgOutputHelper::CFontDef& clFontDef );
	void SetColumnAutoResizeMode( bool fAutoResize ) { m_fAutoResize = fAutoResize; }
	bool RemoveColumn( int iColumnID );

	// Change the column width (will obviously impact header and data).
	bool SetColumnWidth( int iColumnID, double dWidth );
	bool SetColumnWidthInPixel( int iColumnID, long lWidth );
	bool GetColumnWidth( int iColumnID, double& dWidth );
	bool GetColumnWidthInPixel( int iColumnID, long& lWidth );

	void SetDataRowHeight( double dHeight ) { m_dDataRowHeight = dHeight; }

	// Allow to retrieve the absolute position of the start and end row of the column headers.
	// Remarks: this method must be called after all initialization.
	void GetColumnHeaderRowsAbsolute( long& lColumnAbsoluteStartRow, long& lColumnAbsoluteEndRow ) { lColumnAbsoluteStartRow = m_lColumnHeaderStart; lColumnAbsoluteEndRow = m_lColumnHeaderEnd; }
	
	// If 'lRow' = -1 we add message on the last row.
	bool SendMessage( DlgOutputHelper::CMessageBase* pclMessage, DlgOutputHelper::CFilterList* pclFilterList, long lRowRelative = -1, bool fScrollOnTheLastLine = true );

	// Special sending method that directly write data without using a 'CMessageBase'.
	// Param: 'iColumnID' is the column where user as right-clicked.
	// Param: 'lStartRow' MUST BE in relative coordinate (without included headers).
	// Returns an error code (defined in DlgOutputHelper::PasteDataError).
	int PasteData( CString strData, int iColumnID, long lStartRowRelative );
	
	// 'CopyData' will copy all cells data in a global buffer.
	GLOBALHANDLE CopyDataInGlobalBuffer( long lStartColAbsolute, long lStartRowAbsolute, long lEndColAbsolute, long lEndRowAbsolute );
	
	// Will do the same as above but returns result in a 'CString'.
	bool CopyDataInCString( long lStartColAbsolute, long lStartRowAbsolute, long lEndColAbsolute, long lEndRowAbsolute, CString& strOutput );

	// Will do the same as above but returns result in a 'CString'.
	bool CopyDataInXMLSpreadsheet( long lStartColAbsolute, long lStartRowAbsolute, long lEndColAbsolute, long lEndRowAbsolute, CString& strOutput );

	// Retrieve data for all column positions defined in 'pmapRowData'. 
	// Param: 'lCurrentRow' will contain the current row. This row is relative to the header if exist.
	bool GetFirstRow( DlgOutputHelper::mapIntCellBase* pmapCellData, long& lCurrentRowRelative );
	bool GetNextRow( DlgOutputHelper::mapIntCellBase* pmapCellData, long& lCurrentRowRelative );
	bool GetRow( DlgOutputHelper::mapIntCellBase* pmapCellData, long lCurrentRowRelative );

	// Check if row is a row of Data type.
	bool IsRowValidForData( long lCurrentRowRelative );

	////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
	// Methods for block selection.

	bool EnableBlockSelection( bool fSet, WORD wBorderStyle = SS_BORDERSTYLE_SOLID, COLORREF backgroundColor = _TAH_SELECTED_ORANGE );

	// Check if a block selection is currently active.
	bool IsBlockSelectionExist( void );

	// Check if there is data in at least one cell included in the block selection.
	bool IsDataExistInBlockSelection( void );
	
	// Retrieve the current block selection in ABSOLUTE coordinates.
	bool GetBlockSelectionAbsolute( long& lStartColAbsolute, long& lStartRowAbsolute, long& lEndColAbsolute, long& lEndRowAbsolute );

	// Retrieve the current block selection in RELATIVE coordinates.
	bool GetBlockSelectionRelative( int& iStartColID, long& lStartRowRelative, int& iEndColID, long& lEndRowRelative );

	// Allow to manually set the block selection.
	bool SetBlockSelectionRelative( int iStartColID, long lStartRowRelative, int iEndColID, long lEndRowRelative );

	// Allow to set the block selection on all cells where it's possible.
	bool SetBlockSelectionAll( void );

	// If a block selection is active (with multiple rows or/and columns), we reset the block selection to the origin cell (where selection
	// has started).
	bool ResetBlockSelectionToOrigin( void );
		
	// If block selection is displayed, we erase it and reset parameters concerned by it.
	bool EraseBlockSelection( void );

	// If block selection is displayed, content of all cells in it will be deleted. Selection is kept.
	bool EraseDataInBlockSelection( void );

	// Allow to know if mouse pointer is in the block selection.
	bool IsMousePointerInBlockSelection( CPoint ptMouse );

	// End of methods for block selection.
	////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
	

	////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
	// Methods for row selection.
	void EnableLineSelection( bool fSet, int iColumnIDRef, COLORREF backgroundColor = _TAH_SELECTED_ORANGE );

	bool IsRowSelectionExist( void );

	bool GetRowSelection( long &lStartRowRelative, long &lEndRowRelative );

	void DeleteRowSelection( void );

	// End of methods for row selection.
	////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////


	// Allow to convert absolute position to relative.
	void GetRelativeRow( long& lRow );

	void ApplyFilter( DlgOutputHelper::CFilterList* pclFilterList );

	void Clear( bool fAlsoHeader = false );
	
	// Returns row used by data (not included headers).
	long GetRows( void );
	// Returns row used by data and headers.
	long GetTotalRows( void );

	// The positions for the next methods are all relatives.
	bool SelectHeaderRow( long lRowRelative );
	bool SelectDataRow( long lRowRelative, bool fAutoscroll = true );
	bool GetSelectedDataRow( long& lCurrentSelectedRowRelative );
	
	// Allow to save all the current selections.
	bool SaveSelections( CSSheet::SaveSelectionChoice eWhichSelection = CSSheet::SSC_All );

	// Allow to clear all the current selections.
	bool ClearCurrentSelections( CSSheet::SaveSelectionChoice eWhichSelection = CSSheet::SSC_All );
	
	// Restore previously selections saved.
	bool RestoreSelections( void );

	bool GetColID2Absolute( int iColumnID, long& lColAbsolute );

	bool ResetCellContent( int iColumnID, long lFromRowRelative, long lToRowRelative = -1 );

	bool IsCellCanPasteData( int iColumnID );
	bool ChangeCellBackground( int iStartColID, int iEndColID, long lStartRowRelative, long lEndRowRelative, COLORREF crColor, bool fForceColorWhenSelection = false );

	// This method allow to enable/disable the possibility to resize columns.
	bool ResizingColumnActivate( bool fActivate );
	bool ResizingColumnAddRange( long lColStartAbsolutePos, long lColEndAbsolutePos, long lRowStartAbsolute, long lRowEndAbsolute );
	bool ResizingColumnRemoveAll( void );

	bool GetCellFromPixel( CPoint point, int& iColumnID, long& lRowRelative, long& lRowAbsolute );

// Protected members.
protected:
	DECLARE_MESSAGE_MAP()
	virtual void DoDataExchange(CDataExchange* pDX);    // DDX/DDV support
	virtual BOOL OnInitDialog();
	afx_msg void OnDestroy();
	afx_msg void OnPaint();
	afx_msg void OnSize( UINT nType, int cx, int cy );
	afx_msg void OnCancel();
	afx_msg void OnOK();

	// 'MessageManager' message handler.
	afx_msg LRESULT MM_OnMessageReceive( WPARAM wParam, LPARAM lParam );

	// Spread DLL message handlers.
	afx_msg LRESULT OnTextTipFetch( WPARAM wParam, LPARAM lParam );
	afx_msg LRESULT OnEditModeOff( WPARAM wParam, LPARAM lParam );

// Private members.
private:
	enum SheetDescription
	{
		SD_Output = 1
	};

	enum BitParam
	{
		BitBlock				= 0x01,		// 1: this row is blocked and can't be displayed.
		BitPreviousBlock		= 0x02,		// 0: row before blocked was shown / 1: was hidden.
		BitMaskBlocked			= ( BitBlock | BitPreviousBlock ),

		BitBlankLineBeforeMH	= 0x04,		// 1: this row is the blank line before the main header.
		BitMainHeader			= 0x08,		// 1: this row is a main header.
		BitColumnHeader			= 0x10,		// 1: this row belongs to a column header.
		BitData					= 0x20,		// 1: this row is a data.
		BitBlankLine			= 0x40,		// 1: it's a blank line added at the end.
		BitMaskDataType			= ( BitBlankLineBeforeMH | BitMainHeader | BitColumnHeader | BitData | BitBlankLine ),
		
		BitMaskAll				= ( BitBlock | BitPreviousBlock | BitBlankLineBeforeMH | BitMainHeader | BitColumnHeader | BitData | BitBlankLine )
	};

	// Methods for CSSheet.
	void _InitializeSSheet( void );
	void _ResizeColumns( bool fScrollOnTheLastLine );

	void _ExecutePreJob( DlgOutputHelper::CMessageBase* pclMessage, long lRowAbsolute );
	
	// Method helper to write output.
	bool _SendOutputText( CString strText, long lColAbsolute, long lFromRowAbsolute, long lToRowAbsolute, DlgOutputHelper::CFontDef* pclFontDef );

	bool _SendOutputBitmap( DlgOutputHelper::CCellBitmap* pclCellBitmap, long lColAbsolute, long lRowAbsolute );
	bool _SendOutputParam( DlgOutputHelper::CCellParam* pclCellParam, long lColAbsolute, long lRowAbsolute );

	bool _ReadCell( DlgOutputHelper::CCellMaster* pclCellMaster, long lColAbsolute, long lRowAbsolute );
	void _ApplyFilterRow( long lRowAbsolute, DlgOutputHelper::CFilterList* pclFilterList );

	void _AddRows( BitParam eWhatToInsert, long lWhereToInsertAbsolute, long lNumberOfRow, double dRowHeight, bool fInsert = false );

	void _DeleteRows( long lStartRow, long EndRow );
	
	// If 'lColumn' is -1 the method prepare all columns, otherwise prepare only the specified column.
	void _PrepareCellsInRows( BitParam eWhatToPrepare, long lFromRow, long lToRow, long lColumn = -1 );
	
	long _CheckWhereToInsertColumn( int iColumnID );
	void _InsertCol( int iColumnID, long lWhereToInsertAbsolute );
	void _ApplyCellFormat( long lColStart, long lRowStart, long lColEnd, long lRowEnd, DlgOutputHelper::CColDef& clColDef );
	void _ApplyCellProperties( long lColStart, long lRowStart, long lColEnd, long lRowEnd, DlgOutputHelper::CColDef& clColDef );
	void _VerifyMainHeaders( void );

	void _OnLButtonDown( long lLButtonDownCol, long lLButtonDownRow, CPoint ptMouse );
	void _OnLButtonDblClk( long lLButtonDblClkCol, long lLButtonDblClkRow, CPoint ptMouse );
	void _OnRButtonDown( long lRButtonDownCol, long lRButtonDownRow, CPoint ptMouse );

	void _VerifyLastBlankLine( void );

// Private variables.
private:
	int			m_iOutputID;
	CSSheet*	m_pclSheet;
	long		m_lSpreadMinWidth;
	bool		m_fAutoResize;
	bool		m_fEditionEnabled;
	bool		m_fVScrollPrevState;
	bool		m_fIsRedrawBlocked;
	bool		m_fIsOutputBlocked;
	bool		m_fTextTipFetchEnabled;
	bool		m_fBlankLineBeforeMainHeaderExist;
	double		m_dBlankLineBeforeMainHeaderHeight;
	long		m_lColumnHeaderRows;
	long		m_lColumnHeaderStart;
	long		m_lColumnHeaderEnd;
	bool		m_fFreezeHeader;
	bool		m_fAutomaticLastBlankLine;
	long		m_lIterRowData;
	double		m_dDataRowHeight;
	COLORREF	m_DefaultSelectionColor;
	
	typedef struct _MainHeaderParam
	{
		long m_lRowPos;
		double m_dMainHeaderRowHeight;
		// First key is for the main header sub ID.
		DlgOutputHelper::mapIntColDef m_mapMainHeaderDefList;
		std::map<int, CString> m_mapMainHeaderText;
		struct _MainHeaderParam()
		{
			m_lRowPos = -1;
			m_dMainHeaderRowHeight = -1;
			m_mapMainHeaderDefList.clear();
			m_mapMainHeaderText.clear();
		}
	}MainHeaderParam;
	// 'int' is for the main header ID.
	std::map<int, MainHeaderParam>	m_mapMainHeaders;
	typedef std::map<int, MainHeaderParam>::iterator iterMHParam;
	typedef std::map<int, MainHeaderParam>::reverse_iterator riterMHParam;

	DlgOutputHelper::mapIntColDef	m_mapHeaderDefList;
	DlgOutputHelper::mapIntCellBase	m_mapHeaderCellBaseList;
	DlgOutputHelper::mapIntColDef	m_mapColDefList;
	DlgOutputHelper::mapIntCellBase	m_mapCellBaseList;

	// For Spread text tip fetch.
	// 'CMapWordToPtr' maps 'ColumnID' with corresponding 'CCellBase*'.
	// 'mapLongCMapWordToPtr' maps 'row' with corresponding 'CMapWordToPtr'.
	typedef CMap<int, int, DlgOutputHelper::CCellBase*, DlgOutputHelper::CCellBase*> mapIntCellBase;
	typedef CMap<long, long, mapIntCellBase*, mapIntCellBase*> mapLongMapIntCellBase;
	mapLongMapIntCellBase m_mapTextTipFetchList;
	
	std::map<short, INotificationHandler*> m_mapNotificationHandlerList;
	typedef std::map<long, int> mapLongInt;
	typedef mapLongInt::iterator mapLongIntIter;
	typedef mapLongInt::reverse_iterator mapLongIntRIter;
	mapLongInt m_mapColPosToID;
	
	// Variable to resizing columns.
	CViewDescription m_ViewDescription;
	CResizingColumnInfo* m_pclResizingColumnInfos;
	bool m_fResizingColumnActive;
};
