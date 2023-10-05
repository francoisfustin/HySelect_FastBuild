//
// SSheet.h: interface for the CSSheet class.
//
//////////////////////////////////////////////////////////////////////

#pragma once


#if _MSC_VER > 1000
#pragma once
#endif // _MSC_VER > 1000

#include "afxctl.h"
#include <float.h>
#include <math.h>
#include <afxtempl.h>
#include <map>

#include "SpreadPreview.h"
#include "MessageManager.h"
#include "Ssdllmfc.h"
#include "Global.h"


#define MAX_SHEETS										1000
#define _SS_BLOCKSELECTIONSCROLLTIME					12

#define _FLAG_COLUMN_CANBECOLLAPSED						0x80000000
// _ALWAYSHIDDEN set flag to true, needed when a column
// must stay hidden when we use Collapse/Extend functionalities
#define _FLAG_COLUMN_ALWAYSHIDDEN						0x40000000
// Must be shown even is column is empty. Of course if column can be collapsed, this one
// will be not show even if this flag is set ! This flag is only for column in expand mode!
#define _FLAG_COLUMN_SHOWEVENEMPTY						0x20000000
// Set that a row is no breakable (for print).
#define _FLAG_ROW_NOBREAKABLE							0x10000000

////////////////////////////////////////////////////////////////////////////////
// Defines use in lParam of a column
//
#define _SET_FLAG_CANBECOLLAPSED( lUserData )			( lUserData |= _FLAG_COLUMN_CANBECOLLAPSED )
#define _SET_FLAG_ALWAYSHIDDEN( lUserData )				( lUserData |= _FLAG_COLUMN_ALWAYSHIDDEN )
#define _SET_FLAG_SHOWEVENEMPTY( lUserData )			( lUserData |= _FLAG_COLUMN_SHOWEVENEMPTY )
#define _SET_FLAG_ROWNOBREAKABLE( lUserData )			( lUserData |= _FLAG_ROW_NOBREAKABLE )
#define _RESET_FLAG_CANBECOLLAPSED( lUserData )			( lUserData &= ~( _FLAG_COLUMN_CANBECOLLAPSED ) )
#define _RESET_FLAG_ALWAYSHIDDEN( lUserData )			( lUserData &= ~( _FLAG_COLUMN_ALWAYSHIDDEN ) )
#define _RESET_FLAG_SHOWEVENEMPTY( lUserData )			( lUserData &= ~( _FLAG_COLUMN_SHOWEVENEMPTY ) )
#define _RESET_FLAG_ROWNOBREAKABLE( lUserData )			( lUserData &= ~( _FLAG_ROW_NOBREAKABLE ) )
#define _IS_FLAG_CANBECOLLAPSED( lUserData )			( ( _FLAG_COLUMN_CANBECOLLAPSED == ( lUserData & _FLAG_COLUMN_CANBECOLLAPSED ) ) ? true : false )
#define _IS_FLAG_ALWAYSHIDDEN( lUserData )				( ( _FLAG_COLUMN_ALWAYSHIDDEN == ( lUserData & _FLAG_COLUMN_ALWAYSHIDDEN ) ) ? true : false )
#define _IS_FLAG_SHOWEVENEMPTY( lUserData )				( ( _FLAG_COLUMN_SHOWEVENEMPTY == ( lUserData & _FLAG_COLUMN_SHOWEVENEMPTY ) ) ? true : false )
#define _IS_FLAG_ROWNOBREAKABLE( lUserData )				( ( _FLAG_ROW_NOBREAKABLE == ( lUserData & _FLAG_ROW_NOBREAKABLE ) ) ? true : false )

// Defines some values for Expand/Collapse column button
// Remark: 'ECCB' is for Expand/Collapse Column Button
#define __GET_ECCB_COLLAPSE_ICON						( IDI_SLARROW )
#define __GET_ECCB_EXPAND_ICON							( IDI_SRARROW )
#define __GET_BUTTON_MAINCOLLAPSE_MODE_ICON				( IDI_COLLAPSEALLCOL )
#define __GET_BUTTON_MAINEXPAND_MODE_ICON				( IDI_EXPANDALLCOL )

// Values below are to maintain what is the column range for Expand/Collapse
#define _COLSTARTCOLLAPSEMASK	0x000000FF
#define _COLENDCOLLAPSEMASK		0x0000FF00
#define _GETCOLSTARTCOLLAPSE( lUserData )				( ( lUserData ) & _COLSTARTCOLLAPSEMASK )
#define _GETCOLENDCOLLAPSE( lUserData )					( ( ( lUserData ) & _COLENDCOLLAPSEMASK ) >> 8 )
#define _SETCOLSTARTCOLLAPSE( lUserData, lColStart )	{ ( lUserData ) &= ~_COLSTARTCOLLAPSEMASK; ( lUserData ) |= ( lColStart ); }
#define _SETCOLENDCOLLAPSE( lUserData, lColEnd )		{ ( lUserData ) &= ~_COLENDCOLLAPSEMASK; ( lUserData ) |= ( ( lColEnd ) << 8 ); }

#define _ECCBSTATE_MASK			0x00010000
#define _ECCBSTATE_COLLAPSE		0
#define _ECCBSTATE_EXPAND		1
#define _GETEXPANDCOLLAPSEVALUE( lUserData )			( ( ( lUserData ) & _ECCBSTATE_MASK ) >> 16 )
#define _SETEXPANDCOLLAPSEVALUE( lUserData, lState )	{ ( lUserData ) &= ~_ECCBSTATE_MASK; ( lUserData ) |= ( ( lState ) << 16 ); }
//
////////////////////////////////////////////////////////////////////////////////

#define _FONT_

class CDlgSpreadCB;
class CSSheet : public TSpread, public CMessageManager
{
// Public class.
public:
	class INotificationHandler
	{
	public:
		virtual void SSheetOnBeforePaint( void ) = 0;
		virtual void SSheetOnAfterPaint( void ) = 0;
	};

	// Public typedef
	enum _SSBorder
	{
		all = 0x00,
		top = 0x01,
		right = 0x02,
		bottom = 0x04,
		left = 0x08
	};

	enum _SSTextPattern
	{
		TitlePage,
		SubTitle,
		TitleGroup,
		UserStaticColumn,
		TitleBox,
		Arial8,
		ArialItalic8,
		ColumnHeader,
		ColumnHeaderWordWrap
	};

	// Remark: 'TPP' is for 'Text Pattern Property'
	enum _SSTextPatternProperty
	{
		TPP_FontName = 0x00,
		TPP_FontBold,
		TPP_FontItalic,
		TPP_FontUnderline,
		TPP_FontSize,
		TPP_FontForeColor,
		TPP_FontBackColor,
		TPP_StyleHorizontalAlign,
		TPP_StyleVerticalAlign,
		TPP_StyleWordWrap,
		TPP_StyleEllipses
	};

	// Allow to set one or more property for a cell.
	// Remark: this value is encoded in a long (64 bits available).
	enum _SSCellProperty
	{
		// Function properties.
		CellMainExpandCollapseButton = 0x0001,
		CellExpandCollapseColumnButton = 0x0002,
		CellExpandCollapseRowButton = 0x0004,
		CellShowAllPrioritiesButton = 0x0008,
		CellBoxButton = 0x0010,

		// Type properties.
		CellText = 0x0020,
		CellDouble = 0x0040,

		// Selection properties.
		CellCantSelect = 0x0080,					// If set, CSSheet will not paint this cell when executing 'SelectOneRow'.
		CellCantRowSelect = 0x0100,					// If set, CSSheet will not select this cell when doing a row selection.
		CellCantColSelect = 0x0200,					// If set, CSSheet will not select this cell when doing a column selection.
		CellCantBlockSelect = 0x0400,				// If set, CSSheet will not select this cell when doing a block selection.
		CellNoSelection = ( CellCantSelect | CellCantRowSelect | CellCantColSelect | CellCantBlockSelect ),

		// Mouse event properties.
		CellCantLeftClick = 0x0800,
		CellCantLeftDoubleClick = 0x1000,
		CellCantRightClick = 0x2000,
		CellNoMouseEvent = ( CellCantLeftClick | CellCantLeftDoubleClick | CellCantRightClick ),

		// Other properties.
		CellEditable = 0x4000,
	};

	enum _SSKeyBoardPressed
	{
		CtrlKey,
		NoKey,
		ShiftKey
	};

	enum TextPatternModifyKey
	{
		TPMK_Bold = 0,
		TPMK_Italic,
		TPMK_Underline,
		TPMK_Size,
		TPMK_ForeColor,
		TPMK_BackColor
	};

	enum ExpandCollapseColumnState
	{
		ECCS_Invalid = -1,
		ECCS_Expanded = 0,
		ECCS_Collapsed = 1
	};

	enum ExpandCollapseColumnAction
	{
		ECCA_Toggle = -1,
		ECCA_ToCollapse = 0,
		ECCA_ToExpand = 1
	};

	enum ExpandCollapseRowState
	{
		ECRS_Invalid = -1,
		ECRS_Expanded = 0,
		ECRS_Collapsed = 1
	};

	enum ExpandCollapseRowAction
	{
		ECRA_Toggle = -1,
		ECRA_ToCollapse = 0,
		ECRA_ToExpand = 1
	};

	enum PictureCellType
	{
		Bitmap = 0,
		Icon
	};

	enum SaveSelectionChoice
	{
		SSC_None = 0x00,
		SSC_Selection = 0x01,
		SSC_BlockSelection = 0x02,
		SSC_All = ( SSC_Selection | SSC_BlockSelection )
	};

// Protected typedef
protected:
	class CTextPatternStructure
	{
	friend class CSSheet;
	public:
		CTextPatternStructure();
		CTextPatternStructure( _SSTextPattern eTextPattern, bool fLight = false );
		CTextPatternStructure( CTextPatternStructure *pclTextPatternStructure );
		~CTextPatternStructure();

		void SetProperty( _SSTextPatternProperty eProperty, LPARAM lpValue );
		LPARAM GetProperty( _SSTextPatternProperty eProperty );

		// Setter.
		void SetFontName( CString strFontName );
		void SetFontBold( BOOL fBold );
		void SetFontItalic( BOOL fItalic );
		void SetFontUnderline( BOOL fUnderline );
		void SetFontSize( int iSize );
		void SetForeColor( COLORREF foreColor );
		void SetBackColor( COLORREF backColor );
		void SetStyleHorizontalAlign( long lHorizontalAlign );
		void SetStyleVerticalAlign( long lVerticalAlign );
		void SetStyleWordWrap( bool fSet );
		void SetStyleEllipses( bool fSet );

		// Getter.
		CString* GetFontName( void );
		BOOL GetFontBold( void );
		BOOL GetFontItalic( void );
		BOOL GetFontUnderline( void );
		int GetFontSize( void );
		COLORREF GetForeColor( void );
		COLORREF GetBackColor( void );
		long GetStyleHorizontalAlign( void );
		long GetStyleVerticalAlign( void );
		bool GetStyleWordWrap( void );
		bool GetStyleEllipses( void );

	private:
		bool _FillTextPattern( _SSTextPattern eTextPattern, bool fLight );

	public:
		BOOL m_fFontBold;
		BOOL m_fFontItalic;
		BOOL m_fFontUnderline;
		int m_iFontSize;
		COLORREF m_TextColor;
		COLORREF m_BackColor;

		_SSTextPattern m_staticTextPattern;
		CFont* m_pclFont;
		HFONT hFont;
		CString m_strFontName;
		long m_lTextStyle;
		bool m_fChanged;
	};

	struct _sCellProperties
	{
		BOOL		fFontBold;
		COLORREF	TextColor;
		COLORREF	BackColor;
		long		lColumn;
	};

public:
	CSSheet();
	virtual ~CSSheet();

	// Overrides the 'CWnd::Create' allowing to create TSpread as it must be done.
	virtual BOOL Create( DWORD dwStyle, const RECT& rect, CWnd* pParentWnd, UINT nID );
	void PostPoneDelete( void );

	// Allow to do sheet initialization and set default values.
	void Init();
	
	// Allow to reset the workbook, including all sheets in the control. This method can be overridden.
	virtual void Reset( void );

	// Must absolutely disappear as soon as possible.
	void SetREDRAW( BOOL b );

	void EnableEdition( bool fEnable ) { m_fEditionEnabled = fEnable; }
	bool IsEditionEnabled( void ) { return m_fEditionEnabled; }

	/////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
	// Printing methods.

	// Allow to print sheet only in preview mode.
	// Param: 'pDC' is the pointer on the device context in which preview must be done.
	// Param: 'rectPage' contains page size to preview in pixels.
	// Param: 'nPage' is the page number to print.
	// Param: 'fAllSheetInOnePage' is set to true when we must print all sheet in only one page.
	//        See code to other comments.
	void OwnerPrintPreviewDraw( CDC *pDC, CRect &rectPage, short nPage, bool fAllSheetInOnePage = false );

	// Allow to print sheet only in normal mode.
	// Param: 'pDC' is the pointer on the device context in which preview must be done.
	// Param: 'rectPage' contains page size to preview in pixels.
	// Param: 'nPage' is the page number to print.
	// Param: 'fAllSheetInOnePage' is set to true when we must print all sheet in only one page.
	//        See code to other comments.
	// Remark: doesn't exist in DLL version.
	void OwnerPrintDraw( CDC *pDC, CRect &rectPage, short nPage, bool fAllSheetInOnePage = false );

	// Allow to know how many page there is to print.
	// Param: 'pDC' is the pointer on the device context in which preview must be done.
	// Param: 'rectPage' contains page size to preview in pixels.
	// Param: 'lPageCount' will contain the page number.
	// Remark: doesn't exist in DLL version.
	void OwnerPrintPageCount( CDC *pDC, CRect &rectPage, long &lPageCount );

	SS_PRINTFORMAT& GetSSPrintFormat( void ) { return m_ssPrintFormat; }
	void SetSSPrintFormat( SS_PRINTFORMAT& ssPrintFormat ) { m_ssPrintFormat = ssPrintFormat; }

	// End of printing methods.
	/////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
	
	
	/////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
	// Methods relative to coordinate.
	
	// Allow to do conversion from twips to pixels.
	// Remark: Twips are screen-independent units to ensure that the proportion of screen elements are the same on all display systems.
	//         TWIPS: TWentieth of an Inch Point. There is 72 points by inch. Thus one twip is 1/1440 inch.
	void TwipsToPixels( CRect &rect );

	// Allow to do conversion from pixels to twips.
	void PixelsToTwips( CRect &rect );
	
	// Allow to retrieve selection size in pixel.
	// Return: rectangle size in pixel.
	// Remark: This method can be called only for example to know the size of a column range. To do that, 'lFromRow' and 'lToRow' can
	//         be set to 0.
	CRect GetSelectionInPixels( long lFromColumn, long lFromRow, long lToColumn, long lToRow );
	
	// Same as previous one but here allow to directly pass a rectangle as argument.
	// Return: rectangle size in pixel.
	CRect GetSelectionInPixels( CRect rectRange );

	// Allow to retrieve position in pixel of the current selection set by 'rectRange'.
	// Return: rectangle position in pixel.
	// Remark: top left cell in TSpread is at the position 0, 0. Position returned in pixel is relative to this point.
	CRect GetSelectionCoordInPixels( CRect rectRange );

	// Allow to convert cell logical coordinates in pixel.
	// Param: 'lColumn' and 'lRow' are the logical coordinates.
	// Return: the cell rectangle in pixel coordinates.
	// Remark: top left cell in TSpread is at the position 0, 0. Position returned in pixel is relative to this point.
	CRect GetCellCoordInPixel( long lColumn, long lRow );

	// Returns sheet width in logical unit.
	double GetSheetWidth( void );

	// Returns sheet height in logical unit.
	double GetSheetHeight( void );
	
	// Return sheet size in pixel of displayed cols and rows including header.
	// Param: 'fWithScrollBar' if you want include scrollbar.
	CRect GetSheetSizeInPixels( bool fWithScrollBar = false );

	// Retrieve the column size in logical unit.
	// Remark: - by default logical unit is based on system fixed font.
	//         - 'W' for Wrapper. TSpread 'GetColWidthW' and 'GetMaxTextColWidthW' returns result using variable passed as argument. 
	//           To avoid to create local variable for calling this function, you can use this wrapper that returns result in a normal way.
	DOUBLE GetColWidthW( long lColumn );

	// Retrieve the column size in pixel.
	// Remark: 'W' for Wrapper.
	long GetColWidthInPixelsW( long lColumn );

	// Retrieve the row size in pixel.
	// Remark: 'W' for Wrapper.
	long GetRowHeightInPixelsW( long lRow );

	// Returns the width of the widest text string in the specified column.
	// Remark: 'W' for Wrapper.
	DOUBLE GetMaxTextColWidthW( long lColumn );

	// End of methods relative to coordinate.
	/////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
	

	/////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
	// Methods to manage Expand/Collapse columns feature.

	// Create a column with a Expand/Collapse button.
	// Param: 'lButtonCol' and 'lButtonRow' is where the button must be drawn.
	// Param: 'lFromColumn' and 'lToColumn' is the column range to expand/collapse when clicking the button.
	void CreateExpandCollapseColumnButton( long lButtonCol, long lButtonRow, long lFromColumn, long lToColumn, long lLastRow );

	// Delete a Expand/Collapse column button.
	// Param: 'lButtonCol' and 'lButtonRow' is the button position.
	void DeleteExpandCollapseColumnButton( long lButtonCol, long lButtonRow );

	// Expand or collapse a set of columns.
	// Param: 'lButtonCol' and 'lButtonRow' is the button position.
	// Param: 'lFromRow' and 'lToRow' is the range of rows to verify is a column is empty. '-1' if we must collapse columns.
	// Param: 'eAction' can be 'ECCA_Toggle', 'ECCA_ToCollapse' or 'ECCA_ToExpand'.
	// Remark: column range managed by button is saved in the button cell user data. This is why when you to expand/collapse you need
	//         to provide the button position.
	bool ChangeExpandCollapseColumnButtonState( long lButtonCol, long lButtonRow, long lFromRow = -1, long lToRow = -1, ExpandCollapseColumnAction eAction = ECCA_Toggle, bool fRedraw = true );

	// Return state of Expand/Collapse button.
	// Param: 'lButtonCol' and 'lButtonRow' is the button position.
	// Return: 'ECCS_Expanded' or 'ECCS_Collapsed'. Can be 'ECCS_Invalid' if button doesn't exist at the position.
	int GetExpandCollapseColumnState( long lButtonCol, long lButtonRow );
		
	// Allow to check if cell contains a Expand/Collapse column button.
	bool IsExpandCollapseColumnButton( long lColumn, long lRow );

	// Allow to retrieve list of all expand/collapse button on a specific row.
	void GetListOffAllExpandCollapseButton( long lButtonRow, std::vector<long>* pvecButtonList );

	// Allow to retrieve column range manage by button.
	// Param: 'lButtonCol' and 'lButtonRow' is the button position.
	// Param: 'lFromColumn' and 'lToColumn' is where to save column range.
	// Return: 'true' if ok otherwise 'false' if button doesn't exist at the position.
	bool GetColsManagedByButtonCollapse( long lButtonCol, long lButtonRow, long &lFromColumn, long &lToColumn );

	// Allow to test if a column is managed by a button expand/collapse.
	// Param: 'lColumnToTest' is the column to test.
	// Param: 'lButtonRow' is the row where the button are set.
	bool IsColManagedByButtonCollapse( long lColumnToTest, long lButtonRow );

	// Verify in a column range if there is some Expand/Collapse buttons and if they are needed.
	// Param: 'clAreaToVerify' is the column and row range cells to check.
	// Param: 'lButtonRow' is the row where Expand/Collapse buttons are set.
	// Remark: - The main aim of this method is to avoid to have Expand/Collapse button shown but with no action
	//           available when clicking on it (no column to expand or collapse).
	//         - Typically this method must be called once all columns are defined, filled and so on to clear unneeded button.
	//         - This method will scan all columns and check if Expand/Collapse button exist on the 'iButtonRow'.
	//           For each button found, this method will retrieve corresponding column range managed by this button.
	//           Then verify for each column in this range if some of them are empty or have special flags as
	//           'can be collapsed' or 'always hidden'. If at least one column can be hidden, it means that button
	//           is needed. Otherwise, if no column in range can be collapsed or hidden, the button is no more needed
	//           and we delete it.
	void VerifyExpandCollapseColumnButtons( CRect &clAreaToVerify, long lButtonRow );

	// Same as the three previous ones but here it's the main button. It manages all 'Expand/Collapse' previously defined by calling
	// 'CreateExpandCollapseColumnButton'.
	void CreateMainExpandCollapseButton( long lButtonCol, long lButtonRow );
	void DeleteMainExpandCollapseButton( long lButtonCol, long lButtonRow );
	void ChangeMainExpandCollapseButtonState( long lButtonCol, long lButtonRow, ExpandCollapseColumnAction eAction = ECCA_Toggle );

	// Return status of a main Expand/Collapse column button state.
	// Param: 'lButtonCol' and 'lButtonRow' is the button position.
	// Return: 'ECCS_Expanded' or 'ECCS_Collapsed'. Can be 'ECCS_Invalid' if button doesn't exist at the position.
	int GetMainExpandCollapseColumnState( long lButtonCol, long lButtonRow );

	// Allow to check if cell contains a main Expand/Collapse column button.
	bool IsMainExpandCollapseButton( long lColumn, long lRow );

	// Verify in a column range state of Expand/Collapse column buttons and set main state.
	// Param: 'lMainButtonCol' and 'lMainButtonRow' is the position of main Expand/Collapse column button.
	// Param: 'lChildButtonRow' is the row where we can find child Expand/Collapse column button.
	// Param: 'lFromCol' and 'lToCol' is the range to scan child Expand/Collapse column button.
	// Remark: - This method will scan and check state of child Expand/Collapse column buttons. If all buttons are in collapsed mode, then
	//           main button will be also set in collapsed mode. The same if expanded mode. On the other hands, if we have a mix of state,
	//           we can not decide about main state.
	//         - Typically this method must be called once user has clicked on one of the child Expand/Collapse column buttons to have main
	//           button in the correct state.
	void VerifyMainExpandCollapseButton( long lMainButtonCol, long lMainButtonRow, long lChildButtonRow, long lFromCol, long lToCol );

	// Allow to check all Expand/Collapse buttons state.
	// Param: 'lFromColumn' and 'lToColumn' is the column range to check.
	// Param: 'lButtonRow' is the row where Expand/Collapse buttons are set.
	// Param: 'pfAtLeastOneGroupCollapsed' will be set to 'true' if there is at least one column range collapsed.
	// Param: 'pfAtLeastOneGroupExpanded' will be set to 'true' if there is at least one column range expanded.
	void CheckExpandCollapseColumnButtonState( long lFromColumn, long lToColumn, long lButtonRow, bool *pfAtLeastOneGroupCollapsed, bool *pfAtLeastOneGroupExpanded );
	
	// End of methods to manage Expand/Collapse columns button feature.
	/////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
	
	
	/////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
	// Methods that manage cell flags.
	
	// Defines if a column range can be collapsed (fSet = true) or not.
	void SetFlagCanBeCollapsed( long lFromColumn, long lToColumn, bool fSet );
	
	// Defines if a column range must be hidden (fSet = true) or not.
	void SetFlagAlwaysHidden( long lFromColumn, long lToColumn, bool fSet );
	
	// Defines if a column range must shown even if empty (fSet = true) or not.
	void SetFlagShowEvenEmpty( long lFromColumn, long lToColumn, bool fSet );

	// Defines if a row range can't be breakable (fSet = true) or yes. (for print).
	void SetFlagRowNoBreakable( long lFromRow, long lToRow, bool fSet );

	// Check if a column has _FLAG_COLUMN_CANBECOLLAPSED flag.
	bool IsFlagCanBeCollapsed( long lColumn );

	// Check if a column has _FLAG_COLUMN_ALWAYSHIDDEN flag.
	bool IsFlagAlwaysHidden( long lColumn );

	// Check if a column has _FLAG_COLUMN_SHOWEVENEMPTY flag.
	bool IsFlagShowEvenEmpty( long lColumn );

	// Check if a row has _FLAG_ROW_NOBREAKABLE flag.
	bool IsFlagRowNoBreakable( long lRow );

	// Check if last column is a separator.
	// Remarks: in this case, we must set 'CellCantBePainted' column property to true.
	void CheckLastColumnFlagCantBePainted( long lRowButton );

	// Use this method to permanently hide a column.
	// Remark: Expand/Collapse column button functionality doesn't expand a such column.
	void SetColumnAlwaysHidden( long lColumn, bool fHidden ) { SetColumnAlwaysHidden( lColumn, lColumn, fHidden ); }
	void SetColumnAlwaysHidden( long lFromColumn, long lToColumn, bool fHidden );

	// End of methods that manage cell flags.
	/////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
	
	
	/////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
	// Methods that manage cell property.

	// Set cell property
	void SetCellProperty( long lColumn, long lRow, long lCellProperties, bool fSet, bool fResetBefore = false );
	void SetCellProperty( long lFromColumn, long lFromRow, long lToColumn, long lToRow, long lCellProperties, bool fSet, bool fResetBefore = false );

	// Return status of Cell flag
	bool IsCellProperty( long lColumn, long lRow, long lCellProperties );

	// End of methods that manage cell property.
	/////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////


	/////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
	// Methods that manage cell parameter.

	// Set cell parameter.
	void SetCellParam( long lColumn, long lRow, long lParam );
	void SetCellParam( long lFromColumn, long lFromRow, long lToColumn, long lToRow, long lParam );

	// Get cell parameter.
	long GetCellParam( long lColumn, long lRow );

	// End of methods that manage cell parameter.
	/////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
	
	/////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
	// Methods that manage cell style.

	// Set text pattern with a predefined pattern
	// Remark: if 'fLight' is set to 'true', we get a light color version (ideal for 'cut' mode in 'SheetHMCalc' for example).
	bool SetTextPattern( _SSTextPattern eTextPattern, bool fLight = false );

	// Allow to change property of the current text pattern
	bool SetTextPatternProperty( _SSTextPatternProperty eProperty, LPARAM lpValue );
	long GetTextPatternProperty( _SSTextPatternProperty eProperty );

	bool ApplyTextPattern( long lColumn, long lRow, bool fChangeAlsoStyle = true ) { return ApplyTextPattern( lColumn, lRow, lColumn, lRow, fChangeAlsoStyle ); }
	bool ApplyTextPattern( long lFromColumn, long lFromRow, long lToColumn, long lToRow, bool fChangeAlsoStyle = true );

	// Allow to save current text pattern at specified position
	bool SaveTextPattern( long lColumn, long lRow );

	// Allow to restore saved text pattern
	bool RestoreTextPattern( long lColumn, long lRow );

	// Set cell background color.
	// Param: 'fForceColorWhenSelection' set to true if you want the color stay even if a selection is done on it.
	void SetBackColor( long lColumn, long lRow, COLORREF color, bool fForceColorWhenSelection = false ) { SetBackColor( lColumn, lRow, lColumn, lRow, color, fForceColorWhenSelection ); }
	// Set cell background color from lFromColumn to lToColumn.
	void SetBackColor( long lFromColumn, long lFromRow, long lToColumn, long lToRow, COLORREF color, bool fForceColorWhenSelection = false );

	// Get cell background color.
	COLORREF GetBackColor( long lColumn, long lRow );
	
	// Set cell foreground color.
	// Param: 'fForceColorWhenSelection' set to true if you want the color stay even if a selection is done on it.
	void SetForeColor( long lColumn, long lRow, COLORREF color, bool fForceColorWhenSelection = false ) { SetForeColor( lColumn, lRow, lColumn, lRow, color, fForceColorWhenSelection ); }
	// Set cell foreground color from lFromColumn to lToColumn.
	void SetForeColor( long lFromColumn, long lFromRow, long lToColumn, long lToRow, COLORREF color, bool fForceColorWhenSelection = false );
	
	// Get cell foreground color.
	COLORREF GetForeColor( long lColumn, long lRow );
	
	// Set the text to bold for the cells from col1 to col2 for warning purpose
	void SetFontBold( long lColumn, long lRow, BOOL bBold );
	void SetFontBold( long lFromColumn, long lFromRow, long lToColumn, long lToRow, BOOL bBold );

	// Set the size of the text for the cells from col1 to col2.
	void SetFontSize( long lColumn, long lRow, int iSize );
	void SetFontSize( long lFromColumn, long lFromRow, long lToColumn, long lToRow, int iSize );
	
	bool GetFontBold( long lColumn, long lRow );

	double GetFontSize( long lColumn, long lRow );
	
	// Underline a range of cells
	void Underline( long lColumn, long lRow, BOOL fUnderline) { Underline( lColumn, lRow, lColumn, lRow, fUnderline ); }
	void Underline( long lFromColumn, long lFromRow, long lToColumn, long lToRow, BOOL fUnderline );

	// Show or hide Cells border
	void SetCellBorder( long lColumn, long lRow, bool fShow, WORD wBorderType = SS_BORDERTYPE_OUTLINE ){ SetCellBorder( lColumn, lRow, lColumn, lRow, fShow, wBorderType, SS_BORDERSTYLE_SOLID ); }
	void SetCellBorder( long lFromColumn, long lFromRow, long lToColumn, long lToRow, bool fShow, WORD wBorderType = SS_BORDERTYPE_OUTLINE, WORD wBorderStyle = SS_BORDERSTYLE_SOLID, COLORREF color = _BLACK );
	
	// Remarks: allow to get property of only one border (no combination!)
	bool GetCellBorder( long lColumn, long lRow, WORD wBorderType, WORD *pwBorderStyle, COLORREF *pColor );
	bool GetCellBorder( long lColumn, long lRow, WORD *pwBorderType, WORD *pwBorderStyle, COLORREF *pColor );

	COLORREF GetColHeaderBackColor( bool fEven, bool fModification );
	COLORREF GetColHeaderBackColorForHub( bool fHub, bool fEven, bool fModification );

	// End of methods that manage cell style.
	/////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
	
	
	/////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
	// Methods that manage cell formating value.
	
	// Remark about the last new parameters.
	//   - 'fKeepValue' must be set to 'true' if you want to keep the current value existing in the cell. In this case, 'strText' is not used.
	//   - 'fKeepTextPattern' must be set to 'true' if you want to keep the current text pattern existing in the cell.
	//   - 'fNoMouseEvent' must be set to 'true' if you don't want that mouse event are dispatched outside TSpread.
	bool FormatStaticText( long lFromColumn, long lFromRow, long lToColumn, long lToRow, CString strText = _T(""), long lStyle = SSS_ALIGN_CENTER | SSS_ALIGN_VCENTER | SSS_ELLIPSES, bool fKeepValue = false, bool fKeepTextPattern = false, bool fNoMouseEvent = false );
	bool FormatEditText( long lColumn, long lRow, CString strText = _T(""), long lStyle = SSS_ALIGN_CENTER | SSS_ELLIPSES | ES_MULTILINE, short nMaxEditLen = 31, bool fKeepValue = false, bool fKeepTextPattern = false, bool fNoMouseEvent = false );
	
	// Remark: special method to transform a range of row in the same column.
	//         If 'fForceRowAllSame' is set to 'false', we pass each row one by one only if 'fKeepTextPattern' is set to 'false'. 
	//         If 'fForceRowAllSame' is set to 'true' and 'fKeepTextPattern' is set to 'true', we take pattern of the first row and apply the same to all concerned rows.
	bool FormatEditTextRowRange( long lColumn, long lFromRow, long lToRow, bool fForceRowAllSame = false, CString strText = _T(""), long lStyle = SSS_ALIGN_CENTER | SSS_ELLIPSES | ES_MULTILINE, short nMaxEditLen = 31, bool fKeepValue = false, bool fKeepTextPattern = false, bool fNoMouseEvent = false );
	
	bool FormatEditDouble( long lColumn, long lRow, CString strText = _T(""), long lStyle = SSS_ALIGN_LEFT | SSS_ELLIPSES | ES_MULTILINE, bool fKeepValue = false, bool fKeepTextPattern = false, bool fNoMouseEvent = false );
	
	// Same remark as for 'FormatEditTextRowRange'.
	bool FormatEditDoubleRowRange( long lColumn, long lFromRow, long lToRow, bool fForceRowAllSame = false, CString strText = _T(""), long lStyle = SSS_ALIGN_LEFT | SSS_ELLIPSES | ES_MULTILINE, bool fKeepValue = false, bool fKeepTextPattern = false, bool fNoMouseEvent = false );
	
	bool FormatEditDouble( long lColumn, long lRow, int iPhysicalType, double dSI, long lStyle = SSS_ALIGN_LEFT | SSS_ELLIPSES | ES_MULTILINE, bool fEmptyIfNull = true, bool fKeepValue = false, bool fKeepTextPattern = false, bool fNoMouseEvent = false );
	bool FormatEditCombo( long lColumn, long lRow, CString strText = _T(""), long lStyle = SSS_ALIGN_LEFT , LPARAM lParam = 0, bool fKeepValue = false, bool fNoMouseEvent = false );
	bool FormatEditCombo( long lColumn, long lRow, int iPhysicalType, double dSI, long lStyle = SSS_ALIGN_LEFT , LPARAM lParam = 0, bool fKeepValue = false, bool fNoMouseEvent = false );
	bool FormatComboList( long lColumn, long lRow, long lStyle = SSS_ALIGN_LEFT | SSS_ALIGN_VCENTER, bool fNoMouseEvent = false );
	
	// Use write formated double in a cell
	bool FormatCUDouble( long lColumn, long lRow, int iPhysicalType, double dSI, long lStyle = SSS_ALIGN_CENTER | SSS_ALIGN_VCENTER, bool fStatic = true, bool fEmptyIfNull = true, bool fKeepValue = false, bool fKeepTextPattern = false, bool fNoMouseEvent = false );
	// Allow to check if cell is a static text type. If it's the case, 'rCellType' contains all needed info.
	bool IsCellIsStaticText( long lColumn, long lRow, SS_CELLTYPE& rCellType );

	// End of methods that manage cell formating value.
	/////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////


	/////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
	// Methods that manage cell value.

	bool SetStaticText( long lFromColumn, long lToColumn, long lRow, int iTextID );
	bool SetStaticText( long lFromColumn, long lToColumn, long lRow, CString strText = _T("") );
	bool SetStaticText( long lColumn, long lRow, int iTextID );
	bool SetStaticText( long lColumn, long lRow, CString strText = _T("") );
	bool SetStaticTextOnCol( long lColumn, long lFromRow, long lToRow, CString strText = _T("") );

	// Allow to write a text in a cell and to change dimension of it if text is too large.
	// Param: 'fChangeWidth' is 'true' if text must be written in only one line. In this case, the cell width can be changed.
	//        'fChangeWidth' is 'false' if text can be written in multiple line. In this case, the cell height can be changed.
	void SetStaticTextEx( long lColumn, long lRow, CString strText, bool fChangeWidth );

	// Allow to know if current text in a cell is currently truncated or not.
	// Remark: obviously it works only for cell that has the SSS_ELLIPSES type.
	bool IsTextTruncated( long lColumn, long lRow );

	// Write a formated (user unit) double in a cell
	bool SetCUDouble( long lColumn, long lRow, int iPhysicalType, double dSI, bool fEmptyIfNull = true );

	void SetCellText( long lColumn, long lRow, CString strText );
	
	CString GetCellText( long lColumn, long lRow );
	
	// Draw a picture in a cell.
	// Param: 'iPicID' is the ID of icon or bitmap that we can find in the TASelect resources.
	// Param: 'lColumn' and 'lRow' are the cell coordinate.
	// Param: 'ePictureType' tell what kind of picture (at now there is only PictureType::Icon and PictureType::Bitmap).
	// Remark: These two next methods are to use ONLY with ID belonging to the TASelect resources.
	bool SetPictureCellWithID( int iPicID, long lColumn, long lRow, PictureCellType ePictureType ){ return SetPictureCellWithID( iPicID, lColumn, lRow, lColumn, lRow, ePictureType ); }
	
	// Same as above except that we can merge several columns and rows before to set the picture.
	// Remark: The aim is not to set the same picture in a range, but well to merge and after that to set only one picture in the merged cells!
	bool SetPictureCellWithID( int iPicID, long lFromColumn, long lFromRow, long lToColumn, long lToRow, PictureCellType ePictureType );
	
	// Allow to set picture in a cell.
	// Param: 'hImage' is the handle on icon or bitmap.
	// Param: 'lColumn' and 'lRow' are the cell coordinate.
	// Param: 'fSaveInPictureContainer' true if we must save picture in a container.
	// Param: 'hImageSelected' is the handle on icon or bitmap in the selected state or 'INVALID_HANDLE_VALUE'.
	// Remark: see code for more remarks.
	bool SetPictureCellWithHandle( HANDLE hImage, long lColumn, long lRow, bool fSaveInPictureContainer, long lStyle = VPS_BMP | VPS_CENTER | VPS_STRETCH | VPS_MAINTAINSCALE, HANDLE hImageSelected = INVALID_HANDLE_VALUE );

	// Allow to remove the picture from the cell and to transform the cell in a static text.
	bool ClearPictureCell( long lColumn, long lRow );

	// Allow to check if cell is a picture type. If it's the case, 'rCellType' contains all needed info.
	bool IsCellIsAPicture( long lColumn, long lRow, SS_CELLTYPE& rCellType );

	// Allow to change cell style to a check box.
	// Param: 'lColumn' and 'lRow' is where to set the check box.
	// Param: 'strText' is the text set to the right of the check box.
	// Param: 'fCheck' set if check box is checked or not.
	// Param: 'fEnabled' is set to 'false' if check box must be grayed and unselectable.
	bool SetCheckBox( long lColumn, long lRow, CString strText, bool fCheck, bool fEnabled );

	// Returns 'true' if there is no data in column.
	bool IsColumnEmpty( long lColumn, long lFromRow, long lToRow );

	// End of methods that manage cell value.
	/////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////


	/////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
	// Methods that manage row selection.

	// Allow to select more than one row.
	// Param: 'parlSelRows' is an array containing all rows to select.
	// Param: 'backgroundColor' defines the background color of selected rows.
	// Param: 'textColor' defines the text color of selected rows.
	// Remark: will call 'SelectOneRow' for each row.
	void SelectMutipleRows( CArray<long> *parlSelRows, COLORREF backgroundColor = _TAH_SELECTED_ORANGE, COLORREF textColor = SPREAD_COLOR_IGNORE );

	// Allow to select more than one row.
	// Param: 'lFromRow' and 'lToRow' is the range of rows to select.
	// Param: 'lFromColumn' and 'lToColumn' is the range of columns to select. If not defined, take 0 to max column.
	// Param: 'backgroundColor' defines the background color of selected rows.
	// Param: 'textColor' defines the text color of selected rows.
	// Remark: will call 'SelectOneRow' for each row.
	void SelectMutipleRows( long lFromRow, long lToRow, long lFromColumn = 0, long lToColumn = -1, COLORREF backgroundColor = _TAH_SELECTED_ORANGE, COLORREF textColor = SPREAD_COLOR_IGNORE );

	// Allow to select one line and to highlight with 'color'.
	// Param: 'lRow' is the row to select.
	// Param: 'lFromColumn' and 'lToColumn' is the range of columns to select. If not defined, take 0 to max column.
	// Param: 'backgroundColor' defines the background color of selected row.
	// Param: 'textColor' defines the text color of selected row.
	// Remark: this method add current line background color and font in m_mapSelectedRow and change background color with 'color'.
	void SelectOneRow( long lRow, long lFromColumn = 0, long lToColumn = -1, COLORREF backgroundColor = _TAH_SELECTED_ORANGE, COLORREF textColor = SPREAD_COLOR_IGNORE );

	// Unselect all rows if 'lSelectedRow' is set to -1 or only 'lSelectedRow'.
	// Param: 'fOnlyLastSelection' to 'true' if you want remove only the last selection that was be done on the row or 'false' if you
	//        want to remove all selections.
	void UnSelectMultipleRows( long lSelectedRow = -1, bool fCleanSheet = true, bool fOnlyLastSelection = true );

	// Fill an array with all selected rows and return number of selected rows.
	int	GetSelectedRows( CArray<long> *parlSelRows = NULL );

	// Retrieve coordinates of the current selection.
	void GetSelectionPos( long& lStartCol, long& lStartRow, long& lEndCol, long& lEndRow );

	COLORREF GetDefaultSelectionColor( void ) { return _TAH_SELECTED_ORANGE; }

	// End of methods that manage row selection.
	/////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////


	/////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
	// Methods that manage block selection.

	// Set to 'true' if user can select a block of cells with the mouse.
	void BS_Activate( bool fManageBlockSelection, WORD wBorderStyle = SS_BORDERSTYLE_SOLID, COLORREF backgroundColor = _TAH_SELECTED_ORANGE );
	bool BS_IsActivated( void );
	bool BS_IsSelectionExist( void );
	bool BS_GetSelection( long& lStartCol, long& lStartRow, long& lEndCol, long& lEndRow );

	// Allow to manually set the block selection on a range of cells.
	// Param: if 'fVerifyOnlyCorners' is set to 'true', we verify if all the perimeter is selectable. Otherwise, we check only the 4 corners.
	bool BS_SetSelection( long lStartCol, long lStartRow, long lEndCol, long lEndRow, bool fVerifyOnlyCorners = false );
	bool BS_IsCellInSelection( long lColumn, long lRow );
	bool BS_DeleteDataInSelection( void );
	
	// Allow to cancel any block selection operation and to reset appropriate variables.
	// Param: if 'fResetOrigin' is set to 'true' and if a column and row origins exist, the current block selection will be 
	//        reset to this cell. Otherwise, origins will be reset and it will have no more block selection.
	void BS_CancelSelection( bool fResetOrigin = true );

	// Allow to know if mouse pointer is in the block selection.
	bool BS_IsMousePointerInSelection( CPoint ptMouse );
	// End of methods that manage block selection.
	/////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
	

	/////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
	// Methods that manage row selection.

	// Set to 'true' if user can select a complete row.
	void RS_Activate( bool fManageLineSelection, COLORREF backgroundColor = _TAH_SELECTED_ORANGE );
	void RS_SetBackground( COLORREF backgroundColor );
	void RS_SetCursor( UINT uiIconID );
	void RS_SetPixelTolerance( short nPixels );
	void RS_SetColumnReference( long lColumn );
	bool RS_IsSelectionExist( void );
	bool RS_GetSelection( long& lStartRow, long& lEndRow );

	// End of methods that manage row selection.
	/////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////


	/////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
	// Methods that wraps some TSpread methods.
	//
	// Remark: 'W' for Wrapper. 
	
	// Overrides 'TSpread::AddCellSpan' method to set a 'CellNoMouseClickEvent' property in each cell.
	BOOL AddCellSpanW( long lColumn, long lRow, long lNumCol, long lNumRow );

	// Allow to remove cell span in a different way of 'TSpread::RemoveCellSpan'.
	void RemoveCellSpanW( long lFromColumn, long lFromRow, long lToColumn, long lToRow );

	// Allow to retrieve cell type.
	// Remark: 'TSpread::GetCellType' returns result using variable passed as argument. To avoid to create local
	//         variable for calling this function, you can use this wrapper that returns result in a normal way.
	BYTE GetCellTypeW( long lColumn, long lRow );

	// Allow to Show/Hide a column range.
	// Remark: TSpread has only 'ShowCol'.
	void ShowColRange( long lFromColumn, long lToColumn, BOOL fShow );

	// Allow to get active column.
	// Remark: - the active col is the cell that receives user input.
	//         - TSpread has only 'GetActiveCell'.
	long GetActiveCol( void );

	// Allow to get active row.
	// Remark:  - the active row is the cell that receives user input.
	//          - TSpread has only 'GetActiveCell'.
	long GetActiveRow( void );

	// Returns the column of the upper-left cell displayed in the scrollable portion of the fpSpread control.
	// Remark: TSpread has only 'GetTopLeftCell'.
	long GetTopCol( void );

	// Returns the row of the upper-left cell displayed in the scrollable portion of the fpSpread control.
	// Remark: TSpread has only 'GetTopLeftCell'.
	long GetTopRow( void );

	// Returns the number of nonscrolling, frozen columns.
	// Remark: TSpread has only 'GetFreeze'.
	long GetColsFreeze( void );

	// Returns the number of nonscrolling, frozen rows.
	// Remark: TSpread has only 'GetFreeze'.
	long GetRowsFreeze( void );

	// Converts screen coordinates to column-width coordinates. 
	double LogUnitsToColWidthW( short nUnits );

	// End of methods that wraps some TSpread methods.
	/////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////


	// Call by 'DlgSpreadDB' to notify main sheet when a combo box sheet is closed.
	virtual void CloseDialogSCB( CDialogEx *pclDlgSCB, bool fModified ) { return; }
	
	// Allow to retrieve the width of text that will be displayed in the popup.
	short GetTipTextWidth( CString str );
	
	// Allow to merge rows with same values.
	void MergeRowsWithSameValues( long lFromRow, long lToRow, std::vector<long>& vecColumnList );

	// Force tab strip to go on first position.
	void ResetTabStripPosition( void );

	// Overrides 'CMessageManager'.
	virtual afx_msg LRESULT MM_OnMessageReceive( WPARAM wParam, LPARAM lParam ) { return 0; }
	
	void EmptyMessageQueue( void );

	// Allow to save current selection of your choice.
	bool SaveSelections( SaveSelectionChoice eWhichSelection );

	// Allow to clear the current specific selections.
	bool ClearCurrentSelections( SaveSelectionChoice eWhichSelection );
	
	// Restore previously selections saved.
	bool RestoreSelections();

	void RegisterNotificationHandler( INotificationHandler *pNotificationHandler ) { m_pNotificationHandler = pNotificationHandler; }
	void UnregisterNotificationHandler() { m_pNotificationHandler = NULL; }

// Protected members.
protected:
	DECLARE_MESSAGE_MAP()
	afx_msg void OnMouseMove( UINT nFlags, CPoint point );
	afx_msg LRESULT OnMouseLeave( WPARAM wParam, LPARAM lParam );
	afx_msg void OnLButtonDown( UINT nFlags, CPoint point );
	afx_msg void OnLButtonDblClk( UINT nFlags, CPoint point );
	afx_msg void OnLButtonUp( UINT nFlags, CPoint point );
	afx_msg void OnRButtonDown( UINT nFlags, CPoint point );
	afx_msg void OnCaptureChanged( CWnd* pWnd );
	afx_msg void OnSize( UINT nType, int cx, int cy );
	afx_msg LRESULT OnSSheetColWidthChange( WPARAM wParam = 0, LPARAM lParam = 0 );
	afx_msg void OnTimer( UINT_PTR nIDEvent );
	afx_msg void OnPaint();
	virtual BOOL PreTranslateMessage( MSG* pMsg );

// Private methods.
private:
	// Remark: for each row, we can have a list (vector) of cell selections.
	// That means for example for row 14, we can have a first selection on cells from 1 to 3. After that, it's completely
	// possible to have a second selection overwritten the first one on cells from 2 to 4. This allow us to retrieve the
	// correct colors after removing all selections.
	//  14, vector[0]arCellProperties[0] -> property for cell 1, 14.
	//  14, vector[0]arCellProperties[1] -> property for cell 2, 14.
	//  14, vector[0]arCellProperties[3] -> property for cell 3, 14.
	//  14, vector[1]arCellProperties[0] -> property for cell 2, 14.
	//  14, vector[1]arCellProperties[1] -> property for cell 3, 14.
	//  14, vector[1]arCellProperties[2] -> property for cell 4, 14.
	typedef CArray<_sCellProperties,_sCellProperties&> arCellProperties;
	typedef std::vector< arCellProperties* > vecCellProperties;
	typedef vecCellProperties::iterator vecCellPropertiesIter;
	typedef std::map< long, vecCellProperties > mapLongVec;
	typedef mapLongVec::iterator mapLongVecIter;
	typedef std::map< short, mapLongVec> mapShortMapLongVec;
	typedef mapShortMapLongVec::iterator mapShortMapLongVecIter; 

	// Transform SSS_ALIGN_XXX to SS_TEXT_XXX style (because spread doesn't support SSS_ALIGN_XXX style with static text cell type).
	void _ConvertSSStoSSStyle( long &lStyle );
	// Transform SS_TEXT_XXX to SSS_ALIGN_XXX style (because spread doesn't support SS_TEXT_XXX style with edit text cell type).
	void _ConvertSStoSSSStyle( long &lStyle );
	void _ClearBitmapContainer( short nSheet = -1 );
	void _SetColumnFlag(long lFromColumn, long lToColumn, long lFlagValue, bool fSet );
	void _SetRowFlag(long lFromRow, long lToRow, long lFlagValue, bool fSet );

	// Clear variables linked to a map of selected rows.
	// If 'pmapSelectedRows' is NULL, the method clears all selected rows for all sheet.
	void _ClearAllSelectedRows( mapLongVec* pmapSelectedRows );
	
	long _GetLastColNotHidden( void );
	long _GetLastRowNotHidden( void );

	void _ManageOnLButtonDown( UINT nFlags, CPoint point );
	void _ManageOnLButtonDblClk( UINT nFlags, CPoint point );

	/////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
	// Methods that manage block selection.
	
	// Allow to make the block selection with current coordinates saved in the 'm_rBlockSelectionParams' structure.
	void _BS_DoSelection( void );
	
	// Allow to restore previously border saved.
	void _BS_UndoSelection( void );

	void _BS_CheckOnMouseMove( CPoint point );

	// Allow to check what to do after a mouse event.
	// Returns: 'true' if mouse event does a selection.
	bool _BS_CheckAfterMouseEvent( CPoint point );

	BOOL _BS_CheckKeyboardDownEvent( int iVirtualKey );
	// End of methods that manage block selection.
	/////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

	/////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
	// Methods that manage row selection.
	void _RS_CheckOnMouseMove( CPoint point );
	// End of methods that manage row selection.
	/////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

public:
	void _SaveBitmap( HDC hMemDC, HBITMAP hBitmap );		// For debug purpose.

// Private variables
private:
	mapLongVec m_mapSelectedRow;
	mapShortMapLongVec m_mapSelectedRowsBySheet;
	SS_PRINTFORMAT m_ssPrintFormat;
	bool m_bInitialized;
	bool m_fEditionEnabled;

	typedef std::vector< CTextPatternStructure* > vecTextPatternList;
	typedef vecTextPatternList::iterator vecTextPatternListIter;
	vecTextPatternList m_vecTextPatternList;
	CTextPatternStructure *m_pclCurrentTextPatternStructure;
	CTextPatternStructure *m_pclSavedTextPatternStructure;

	// Image container.
	// Remark: save all dynamic images in a vector for all available sheets.
	class BitmapInfo
	{
	public:
		BitmapInfo() {}
		~BitmapInfo() {}
		bool operator==( const BitmapInfo &clBitmapInfo ) const { return ( clBitmapInfo.m_lCol == m_lCol && clBitmapInfo.m_lRow == m_lRow && clBitmapInfo.m_pclBitmap == m_pclBitmap ); }

		long m_lRow;
		long m_lCol;
		CBitmap *m_pclBitmap;
	};

	typedef std::vector<BitmapInfo> vecBitmapList;
	typedef vecBitmapList::iterator vecBitmapListIter;
	typedef std::map< short, vecBitmapList > mapShortBitmapInfo;
	typedef mapShortBitmapInfo::iterator mapBitmapListIter;
	mapShortBitmapInfo m_mapBitmapList;
	typedef std::map<HANDLE, HANDLE> mapHH;
	typedef mapHH::iterator mapHHIter;
	mapHH m_mapImageToImageSelected;
	mapHH m_mapImageSelectedToImage;

	HICON m_hHandOpen;
	HICON m_hHandClosed;

	// Variables for block selection.
	typedef struct _BSBorderSaveParams
	{
		long m_lColumn;
		long m_lRow;
		WORD m_wStyleLeft;
		COLORREF m_ColorLeft;
		WORD m_wStyleTop;
		COLORREF m_ColorTop;
		WORD m_wStyleRight;
		COLORREF m_ColorRight;
		WORD m_wStyleBottom;
		COLORREF m_ColorBottom;
	}BSBorderSaveParams;

	typedef struct _BlockSelectionParams
	{
		bool m_fIsActivated;
		WORD m_wBorderStyle;
		COLORREF m_BorderColor;
		bool m_fIsRunning;
		long m_lColumnStartSelection;
		long m_lRowStartSelection;
		long m_lColumnEndSelection;
		long m_lRowEndSelection;
		long m_lOriginColumn;
		long m_lOriginRow;
		bool m_fScrollStart;
		eBool3 m_b3ScrollDown;
		eBool3 m_b3ScrollRight;
		std::vector<BSBorderSaveParams> m_vecBorderSave;
		UINT_PTR m_nTimer;
		struct _BlockSelectionParams()
		{
			Reset();
		}
		void Reset( void )
		{
			m_fIsActivated = false;
			m_wBorderStyle = SS_BORDERSTYLE_SOLID;
			m_BorderColor = _RED;
			m_vecBorderSave.clear();
			m_nTimer = (UINT_PTR)0;
			ClearSelection();
		}
		void ClearSelection( void )
		{
			m_fIsRunning = false;
			m_fScrollStart = false;
			m_b3ScrollDown = eBool3::eb3Undef;
			m_b3ScrollRight = eBool3::eb3Undef;
			m_lColumnStartSelection = -1;
			m_lRowStartSelection = -1;
			m_lColumnEndSelection = -1;
			m_lRowEndSelection = -1;
			m_lOriginColumn = -1;
			m_lOriginRow = -1;
		}
		bool IsSelectionExist( void	)
		{
			if( false == m_fIsActivated )
				return false;
			return ( -1 != m_lColumnStartSelection ) ? true : false;
		}
		bool IsSelectionExist( long lColumnStartSelection )
		{
			if( false == m_fIsActivated )
				return false;
			return ( -1 != lColumnStartSelection ) ? true : false;
		}
		bool GetSelection( long& lStartCol, long& lStartRow, long& lEndCol, long& lEndRow )
		{
			if( false == IsSelectionExist() )
				return false;
			lStartCol = m_lColumnStartSelection;
			lStartRow = m_lRowStartSelection;
			lEndCol = m_lColumnEndSelection;
			lEndRow = m_lRowEndSelection;
			if( lStartCol > lEndCol )
			{
				long lTemp = lStartCol;
				lStartCol = lEndCol;
				lEndCol = lTemp;
			}
			if( lStartRow > lEndRow )
			{
				long lTemp = lStartRow;
				lStartRow = lEndRow;
				lEndRow = lTemp;
			}
			return true;
		}
	}BlockSelectionParams;
	BlockSelectionParams m_rBlockSelectionParams;

	// Variables for line selection.
	typedef struct _RowSelectionParams
	{
		bool m_fIsActivated;
		COLORREF m_BackgroundColor;
		bool m_fMousePointerChanged;
		HCURSOR m_hDefaultCursor;
		HCURSOR m_hSelectionCursor;
		bool m_fIsRunning;
		long m_lStartRowSelection;
		long m_lEndRowSelection;
		short m_nPixelTolerance;
		long m_lColumnReference;
		struct _RowSelectionParams()
		{
			Reset();
		}
		void Reset( void )
		{
			m_fIsActivated = false;
			m_BackgroundColor = _TAH_SELECTED_ORANGE;
			m_fMousePointerChanged = false;
			m_hDefaultCursor = (HCURSOR)NULL;
			m_hSelectionCursor = AfxGetApp()->LoadIcon( IDI_ARROWYELLOWRIGHT );
			m_fIsRunning = false;
			m_lStartRowSelection = -1;
			m_lEndRowSelection = -1;
			m_nPixelTolerance = 10;
			m_lColumnReference = -1;
		}
		bool IsSelectionExist( void )
		{
			if( false == m_fIsActivated )
				return false;
			if( -1 == m_lStartRowSelection && -1 == m_lEndRowSelection )
				return false;
			return true;
		}
		bool IsSelectionExist( long lStartRowSelection, long lEndRowSelection )
		{
			if( false == m_fIsActivated )
				return false;
			if( -1 == lStartRowSelection && -1 == lEndRowSelection )
				return false;
			return true;
		}
		bool GetSelection( long& lStartRow, long& lEndRow )
		{
			if( false == IsSelectionExist() )
				return false;
			lStartRow = m_lStartRowSelection;
			lEndRow = m_lEndRowSelection;
			return true;
		}
	}RowSelectionParams;
	RowSelectionParams m_rRowSelectionParams;

	typedef struct _SaveSelectionsContext
	{
		SaveSelectionChoice m_eSelectionSaved;
		mapLongVec m_mapSelection;
		long m_lColumnStartBlockSelection;
		long m_lRowStartBlockSelection;
		long m_lColumnEndBlockSelection;
		long m_lRowEndBlockSelection;
		CSSheet* m_pclSSheet;
		void SetSSheetPointer( CSSheet* pclSSheet ) { m_pclSSheet = pclSSheet; }
		void CleanSelections( void )
		{
			m_eSelectionSaved = SaveSelectionChoice::SSC_None;
			if( NULL != m_pclSSheet )
				m_pclSSheet->_ClearAllSelectedRows( &m_mapSelection );
			m_lColumnStartBlockSelection = 0;
			m_lRowStartBlockSelection = 0;
			m_lColumnEndBlockSelection = 0;
			m_lRowEndBlockSelection = 0;
		}
	}SaveSelectionsContext;
	SaveSelectionsContext m_rSaveSelectionsContext;

	HANDLE m_hOnTimerEvent;

	INotificationHandler *m_pNotificationHandler;
};
