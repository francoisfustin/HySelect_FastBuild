#pragma once


#include <map>
#include <vector>
#include "MessageManager.h"

class CSSheet;
class CSheetDescription;
class CViewDescription;

///////////////////////////////////////////////////////////////////////////////////////////////////////////
// If you want to use special class description, you must create inherited class from this one with your 
// specific features.
class CCellDescription
{
protected:
	CCellDescription( int iCellType, CPoint ptCellPosition, CSheetDescription* pclSheetDescription ) 
		{ m_iCellType = iCellType; m_ptCellPosition = ptCellPosition; m_pclSheetDescription = pclSheetDescription; }
public:
	virtual ~CCellDescription() {}

public:
	// Getter.
	int GetCellType( void ) { return m_iCellType; }
	CPoint GetCellPosition( void ) { return m_ptCellPosition; }
	CSSheet *GetpSSheet( void );

protected:
	int m_iCellType;
	CPoint m_ptCellPosition;
	CSheetDescription* m_pclSheetDescription;
};

class CResizingColumnInfo
{
public:
	class INotificationHandler
	{
	public:
		virtual void ResizingColumnInfoNH_OnSSheetColumnWidthChanged( UINT uiSheetDescriptionID, long lColumnID ) = 0;
	};

	CResizingColumnInfo();
	~CResizingColumnInfo();

	// Allow to reset all data concerned by resizing.
	void Reset( void );

	// Register an handler to manage notification initiated by 'CDlgOutputSpreadContainer'.
	void RegisterNotificationHandler( INotificationHandler *pclHandler );
	
	// Unregister an handler.
	void UnregisterNotificationHandler( INotificationHandler *pclHandler );

	void SetSheetDescription( CSheetDescription *pclSheetDescription );

	// Allow to add a range of column that can be resized.
	// Param: 'lFromColumn' and 'lToColumn' define the range of columns that use can resize.
	// Param: 'lCursorFromRom' and 'lCursorToRow' define the range of rows where mouse pointer will transform to double vertical line.
	// Remark: if just only one column, use same value for both variables 'lFromColumn' and 'lToColumn'.
	bool AddRangeColumn( long lFromColumn, long lToColumn, long lCursorFromRow, long lCursorToRow, bool fUpdateRowRange = true );

	// Allow to remove all columns previously inserted before with 'AddRangeColumn'.
	bool RemoveAllColumn( void );

	// Param: 'lDblClkFromRow' and 'lDblClkToRow' define the range of row to check to automatically resize when user double clicks on the title column.
	void ActivateFeature( bool fActivate, bool fExcludeDblClick = false, long lDblClkFromRow = 0, long lDblClkToRow = 0, bool fAutoColSeparator = true );

	// Allow to know if resizing feature is activated or not.
	bool IsActivated( void ) { return m_fActivated; }

	// Force a rescan of all defined column to adapt map with reality (example: needed when user double click on title group 
	// to force reset column width at their default values).
	void Rescan( void ) { if( true == m_fActivated ) _RecomputeMap(); }

	// Allow to set a range in pixel for detection of a column separator.
	void SetRangeForDetection( short nPixel ) { m_nDetectionRange = nPixel; }

	void OnMouseEvent( int iMouseEventType, long lColumn, long lRow, CPoint ptMouse = CPoint( 0, 0 ) );

	// Private methods.
private:
	// Allow to detect if mouse pointer pass over a separator.
	bool _IsPointerOverSeparator( CPoint ptPointer, long lXOffset );

	void _StartMoving( long lPosX, long lXOffset );
	void _StopMoving( void );

	// Allow to manually initialize some internal variables when a resizing begins.
	void _StartResizing( void );

	// Allow to manually stop current resizing.
	void _StopResizing( void );

	// Allow to manually cancel any current resizing.
	void _CancelResizing( void );

	void _ClearMap( void );
	void _RecomputeMap( void );
	void _DrawLine( void );
	void _RestoreLine( void );

	void _DumpMap( CString str );

	void _OnMouseMove( long lMouseMoveCol, long lMouseMoveRow, CPoint ptMouse );
	void _OnLButtonDown( void );
	void _OnLButtonDblClk( long lLButtonDlbClkCol, long lLButtonDlbClkRow, CPoint ptMouse );
	void _OnLButtonUp( void );

private:
	struct ColInfo
	{
		long m_lID;
		long m_lLeftCoord;
		bool m_fIsResizing;
		long m_lPrevPos;
		long m_lCurrentPos;
		long m_lScrollX;
	};
	typedef std::map<long, ColInfo *> mapLongColInfo;
	typedef mapLongColInfo::iterator mapLongColInfosIter;
	
	typedef std::vector<long> vecLong;
	typedef vecLong::iterator vecLongIter;

	std::vector<INotificationHandler *> m_vecNotificationHandlerList;
	
	CSheetDescription *m_pclSheetDescription;

	mapLongColInfo m_mapColInfos;		// Will contains all column info.
	vecLong m_vecColumnList;			// Contains all column ID submitted to 'AddRangeColumn' method.
	bool m_fAutoColSeparator;			// Set to 'true' if the class add separator between each column.
	long m_lRowTopPixel;				// Save top and bottom of row.
	long m_lRowBottomPixel;
	long m_lRowTopLogical;
	long m_lRowBottomLogical;
	long m_lDblClickFromRow;
	long m_lDblClickToRow;
	bool m_fActivated;					// Set if we have activated or not the resizing feature.
	bool m_fDlbClkActivated;
	short m_nDetectionRange;			// Set number of pixel around which we have to detect column separator.
	short m_nMovePrecision;				// Set number of pixel when we have to move vertical bar.
	short m_nMinimalWidth;				// Set minimal width for column.
	bool m_fColMaxWidthIncludeTitle;	// Set 'true' if when we double click on a separator we must find the large column in row data but also in row where are separators.
	
	bool m_fCursorDisplayed;			// Set if resizing cursor is displayed or not.
	HCURSOR m_hDefaultCursor;			// To memorize what was the cursor before change it.

	ColInfo* m_pclColFocused;			// Pointer on 'ColInfo' currently being focused.

	HDC m_hMemDC;
	HBITMAP m_hMemBitmap;
	HBITMAP m_hOldBitmap;
	long m_lSaveHeight;
	HBRUSH m_hBrush;
};

class CSheetDescription
{
public:

	//////////////////////////////////////////////////////////////////////////////////////////////////////////////
	// Enum definition
	//////////////////////////////////////////////////////////////////////////////////////////////////////////////

	// Allow to define how parameter is linked with this sheet:
	//  -> SD_ParameterMode_Mono: there is only one column that contains parameter.
	//  -> SD_ParameterMode_Multi: more than one column can contain parameter (accessories sheet for example).
	enum
	{
		SD_ParameterMode_Mono = 0,
		SD_ParameterMode_Multi
	};

	//////////////////////////////////////////////////////////////////////////////////////////////////////////////
	// Typedefs definition
	//////////////////////////////////////////////////////////////////////////////////////////////////////////////

	// Allow to save all user variables.
	// Remark: Pay attention if we set a user variable that is a pointer to structure, class or any other memory allocated,
	//         you need to free yourself. Map contains only pointer on data!
	typedef std::map< USHORT, LPARAM >				mapUShortLParam;
	typedef mapUShortLParam::iterator				mapUShortLParamIter;

	// Allow to save all parameter column position. 
	typedef std::vector< long >						vecLong;
	typedef vecLong::iterator						vecLongIter;
	typedef vecLong::reverse_iterator				vecLongReverseIter;

	// Allow to manage cell type list.
	typedef std::map<ULONG, CCellDescription*>		mapULongCellDescription;
	typedef mapULongCellDescription::iterator		mapULongCellDescriptionIter;
	typedef std::vector<CCellDescription*>			vecCellDescription;
	typedef vecCellDescription::iterator			vecCellDescriptionIter;

	//////////////////////////////////////////////////////////////////////////////////////////////////////////////
	// Methods definition
	//////////////////////////////////////////////////////////////////////////////////////////////////////////////

	CSheetDescription( UINT uiSheedDescriptionID, CSSheet *pclSSheet, CPoint ptOffset, CViewDescription* pclViewDescription );
	~CSheetDescription( );

	// Initialize sheet description.
	// Param: 'lRowStart' and 'lRowEnd' define rows range (included) of sheet description.
	// Param: 'nRowsByItem' set how many rows it will be necessary for one item.
	// Param: 'nParameterMode' see above.
	// Remark: 'nRowsByItem': for example with accessories, we have two lines for each item (check box and description).
	void Init( long lRowStart, long lRowEnd, short nRowsByItem, short nParameterMode = SD_ParameterMode_Mono );

	// Allow to retrieve sheet description ID.
	// Remark: - PAY ATTENTION: there is a difference between sheet description ID and SSheet ID!! First one is set by user when creating
	//           CSheetDescription. Second one is the ID set when internally create a new instance of CSSheet class. The second one allow
	//           to identify in a window message for which CSSheet to deliver.
	UINT GetSheetDescriptionID() { return m_uiSheetDescriptionID; }

	// Allow to retrieve sheet description ID.
	// Remark: same as 'GetSheetDescriptionID'.
	UINT GetSSheetID() { return m_uiSSheetID; }

	void SetGroupSheetDescriptionID( UINT uiGroupID ) { m_uiGroupID = uiGroupID; }
	UINT GetGroupSheetDescriptionID( void ) { return m_uiGroupID; }

	void SetParentSheetDescriptionID( UINT uiParentID ) { m_uiParentID = uiParentID; }
	UINT GetParentSheetDescriptionID( void ) { return m_uiParentID; }

	CSSheet* GetSSheetPointer() { return m_pclSSheet; }
	void SetSSheetPointer( CSSheet* pclSSheet ) { m_pclSSheet = pclSSheet; }
	
	// Allow to retrieve offset of SSheet.
	CPoint GetSSheetOffset() { return m_ptOffset; }
	void SetSSheetOffset( CPoint ptOffset ) { m_ptOffset = ptOffset; }
	
	int GetParameterMode() { return m_nParameterMode; }

	// Set on which column the focus must be set (for SetActiveCell method)
	void SetActiveColumn( long lColumn );

	// Add a column description.
	// Param: 'lColumn' is the position in the sheet.
	// Param: 'dWidth' defines this column width.
	// Remark: if column already exist, just update its width.
	void AddColumn( long lColumn, double dWidth );
	
	// Same as previous one but here we set width in pixel.
	void AddColumnInPixels( long lColumn, long lWidth );

	// Allow to set an user variable (like total valves, actuators and so on).
	// Param: 'usVarID' is the unique ID for user variable.
	// Param: 'lVarValue' contains the value.
	// Remark: If you use same ID of an another variable already set, this one will be lost.
	void SetUserVariable( USHORT usVarID, LPARAM lVarValue );

	// Allow to get an user variable.
	// Param: 'usVarID' is the unique ID for user variable.
	// Param: 'lVarValue' will contain the value.
	// Return: 'true' if variable is found otherwise false.
	bool GetUserVariable( USHORT usVarID, LPARAM& lVarValue );

	//////////////////////////////////////////////////////////////////////////////////////////////////////////////
	// Methods for selection (focus selection !)
	//
	// Set from which to which column selection must be done.
	// Param: 'lFromColumn' and 'lToColumn' are the two limits INCLUDED!!!
	void SetFocusColumnRange( long lFromColumn, long lToColumn );

	// Allow to retrieve column range for selection.
	// Param: 'lFromColumn' and 'lToColumn' where to save range.
	// Return: -1 in each variable if they are not defined.
	void GetFocusColumnRange( long &lFromColumn, long &lToColumn );

	// Same as 'GetRangeSelection' but separately.
	// Return: -1 in each variable if they are not defined.
	long GetSelectionFrom( void );
	long GetSelectionTo( void );
	//
	//////////////////////////////////////////////////////////////////////////////////////////////////////////////

	// Allow to retrieve what is the first column on which we can set the focus.
	// Remark: - if we are in 'SD_ParameterMode_Mono' mode, there is only one focused column defined and the value returns
	//           is the same as 'lFromColumn' with 'GetRangeSelection' method.
	//         - if we are in 'SD_ParameterMode_Multi' mode (for example accessories), focus can be set on different columns. Thus this
	//           method will return the first one of them.
	long GetFirstFocusedColumn( void );
	
	// Allow to retrieve what is the active column (for _TSpread::SetActiveCell).
	long GetActiveColumn( void );

	// Add a number of row after the last current one.
	// Param: if 'fSelectable' is set to true, that means these columns can be selected.
	void AddRows( long lRow, bool fSelectable = false );

	// Set a range of row that can be selected.
	bool SetSelectableRangeRow( long lFirstSelectableRow, long lLastSelectableRow );

	// Allow to remove only one row from the selectable range row.
	// Returns 'false' if there is not list or 'lRow' is not in the list.
	bool RemoveSelectableRow( long lRow );

	// Check if a row in a sheet can be selected or not.
	bool IsRowSelectable( long lRow );

	// Allow to retrieve which is the first selectable row in a sheet definition.
	// Return: value of the first row or -1 if there is no first row.
	long GetFirstSelectableRow( bool fExcludeHidden = true );

	// Allow to retrieve which is the next selectable row after 'lRow'.
	// Return: value of the next row or -1 if there is no next selectable row.
	// Remarks: we have also 'lColumn' parameter because on a line with multi parameters, we must also check if 'lColumn'
	//		    is not on a cell without parameter (must be the case with accessories for example).
	long GetNextSelectableRow( long lColumn, long lRow, bool fExcludeHidden = true );

	// Allow to retrieve which is the previous selectable row after 'lRow'.
	// Return: value of the previous row or -1 if there is no previous selectable row.
	// Remarks: we have also 'lColumn' parameter because on a line with multi parameters, we must also check if 'lColumn'
	//		    is not on a cell without parameter (must be the case with accessories for example).
	long GetPrevSelectableRow( long lColumn, long lRow, bool fExcludeHidden = true );
	
	// Allow to retrieve which is the last selectable row in a sheet definition.
	// Return: value of the last row or -1 if there is no last row.
	long GetLastSelectableRow( bool fExcludeHidden = true );

	// Retrieve first/last row.
	long GetFirstRow( void );
	long GetLastRow( void );

	// Allow to retrieve first row of an item (used for example by accessories).
	// Return: -1 if 'lReferenceRow' is not correct or not found.
	long GetFirstRowOfItem( long lReferenceRow );

	// Allow to retrieve number of row used for one item (used for example by accessories).
	short GetRowsByItem( void );

	// Allow to retrieve the first column.
	// Param: if 'fExcludeHidden' is set to 'true', this method will return only columns that are not hidden.
	// Return: this method returns -1 if no result.
	long GetFirstColumn( bool fExcludeHidden = false );

	// Allow to retrieve next column.
	// Param: 'lFromColumn' is the column from which to start searching next column.
	// Param: if 'fExcludeHidden' is set to 'true', this method will return only columns that are not hidden.
	// Return: this method returns -1 if no result.
	long GetNextColumn( long lFromColumn, bool fExcludeHidden = false );

	// Allow to retrieve previous column.
	// Param: 'lFromColumn' is the column from which to start searching previous column.
	// Param: if 'fExcludeHidden' is set to 'true', this method will return only columns that are not hidden.
	// Return: this method returns -1 if no result.
	long GetPreviousColumn( long lFromColumn, bool fExcludeHidden = false );
	
	// Allow to retrieve the last column.
	// Param: if 'fExcludeHidden' is set to 'true', this method will return only columns that are not hidden.
	// Return: this method returns -1 if no result.
	long GetLastColumn( bool fExcludeHidden = false );

	// Allow to set which column contains parameter.
	// Remark: If 'm_nParameterMode' is set to 'SD_ParameterMode_Mono', you have only one parameter by row. If
	//         you call 'AddColumnParam' many times in this mode, last value always overwrites previous one.
	//         If 'm_nParameterMode' is set to 'SD_ParameterMode_Multi', you can define more than one column.
	void AddParameterColumn( long lColumn );
	
	// Allow to retrieve first column where parameter is saved.
	// Return: -1 if can't find parameter.
	long GetFirstParameterColumn( void );

	// Allow to retrieve next parameter column.
	// Param: 'lColumn' is the column from which start search.
	// Remarks: * If 'm_nParameterMode' is set to 'SD_ParameterMode_Mono', this method sends back the only column that exist (or -1 if doesn't).
	//			* If 'm_nParameterMode' is set to 'SD_ParameterMode_Multi', this method tries to find column
	//			  in regards to 'lColumn' and sends back this one if exist in the parameter list.
	//			* If 'lColumn' is set to -1, function sends back the first column with parameter.
	// Return: -1 if can't find parameter.
	long GetNextParameterColumn( long lColumn );

	// Allow to retrieve previous parameter column.
	// Param: 'lColumn' is the column from which start search.
	// Remarks: same remark as 'GetNextParameterColumn'
	// Return: -1 if can't find parameter.
	long GetPrevParameterColumn( long lColumn );

	/////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
	// Methods to manage 'm_mapCellDescriptionList'.
	//

	// Allow to add one element in the list.
	// Param: 'pclCellDescription' is the cell description newly created that we have to save.
	// Return: unique identifier that caller can save to internal use or O if any error.
	ULONG AddCellDescription( CCellDescription* pclCellDescription );

	// Allow to retrieve cell description in regards to its cell coordinate.
	// Param: 'lColumn' and 'lRow' are the cell position.
	// Return: cell description at this coordinate or NULL if no one exist.
	// Remark: search a cell description at the coordinates passed as argument. If not found and if sheet is in mono parameter 
	//         mode perhaps user has clicked on row with a product. In that case, cell description will be read at the correct 
	//         position and returned by this method.
	CCellDescription* GetCellDescription( long lColumn, long lRow );

	CCellDescription* GetCellDescription( ULONG ulCellDescriptionID );

	// Allow to retrieve a list of cell description.
	// Param: 'vecCellDescriptionList' is where to save list.
	// Param: 'iCellType' is -1 if we must save all cell descriptions otherwise we save only cell descriptions with
	//        the same cell type.
	void GetCellDescriptionList( vecCellDescription& vecCellDescriptionList, int iCellType = -1 );

	// Allow to remove one element from the list.
	bool RemoveCellDescription( CCellDescription* pclCellDescription );

	// Allow to remove cell description from the list.
	// Param: 'iCellType' must be set to -1 if we must delete all cell descriptions otherwise we delete only cell descriptions with
	//        the same cell type.
	void ClearCellDescription( int iCellType = -1 );

	// Allow to retrieve first cell description in the list.
	// Param: Set 'iCellType' to a specific type to retrieve or to -1 to retrieve cell description from any type.
	// Return: first cell description or NULL if any!
	CCellDescription* GetFirstCellDescription( int iCellType = -1 );

	// Allow to retrieve next cell description from the current one.
	// Param: 'pclCurrentCellDescription' from which cell start searching.
	// Param: Set 'iCellType' to a specific type to retrieve or to -1 to retrieve cell description from any type.
	// Return: next cell description or NULL if there is no more.
	CCellDescription* GetNextCellDescription( CCellDescription *pclCurrentCellDescription, int iCellType = -1 );

	// End of methods to manage 'm_mapCellDescriptionList'.
	/////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

	/////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
	// Methods to manage remarks.

	enum RemarkFlags
	{
		NotAvailable		= 0x01,
		Deleted				= 0x02,
		FullOpening			= 0x04,
		KTHProportional		= 0x08,
		KTH3Points			= 0x10,
		ErrorMessage		= 0x20,
		WarningMessage		= 0x40,
		InfoMessage			= 0x80,
		Last				= 0x80
	};

	// Allow to restart remark generator.
	// Remark: call this function before each filling of a SheetDescription list.
	void RestartRemarkGenerator( void );

	// Allow to write a text with a specific reporting index to put remark below list.
	// Param: 'strText' -> text to display.
	// Param: 'lColumn' and 'lRow' -> where to write the name.
	// Param: 'eRemarkFlag' -> flag to bet set.
	// Remark: When filling a list of valves, it can have some remarks to put after TA Product name or opening for example.
	//         Calling this method will automatically choose index number and associate it to the flag. So, just fill with
	//         this method and at the end just call 'WriteRemarks'.
	void WriteTextWithFlags( CString strText, long lColumn, long lRow, RemarkFlags eRemarkFlag, CString strMessage = _T(""), COLORREF cTextColor = _BLACK, COLORREF cBackColor = _WHITE );

	// Same as above except that here we can have more than one error messages.
	void WriteTextWithFlags( CString strText, long lColumn, long lRow, RemarkFlags eRemarkFlag, std::vector<CString> vecErrorMessages, COLORREF cTextColor = _BLACK, COLORREF cBackColor = _WHITE );

	// Allow to write all remarks once a SheetDescription list has been filled with some calling to 'WriteTextWithFlags'.
	// Param: 'lRow' -> line on which to write first remark.
	// Param: 'lMergeColFrom' and 'lMergeColTo' -> contains range of cells to merge.
	long WriteRemarks( long lRow, long lMergeColFrom, long lMergeColTo );

	// Allow to delete all lines in the sheet containing remarks.
	// Remark: - PAY ATTENTION: this method doesn't clear variables in 'm_vecRemarkList' but just deletes lines in SSheet containing remarks.
	//         - This method is used for example in 'RViewSSelTrv::_UpdateReturnValveSettingDp'. Because 'Full opening' remark can appears/disappears
	//           in regards to previous state.
	void ClearRemarks( void );

	// Allow to remove a remark.
	// Remark: - If remark doesn't exist, function will return false.
	//		   - the row on which remark is set will be deleted!
	//         - PAY ATTENTION: index will be removed from 'm_vecRemarkList' and then all references must be cleared to be coherent.
	//         - All remark index will be updated and also corresponding text in all the Sheet!
	bool RemoveOneRemark( RemarkFlags eRemarkFlag );

	// Allow to check if a remark is already defined.
	bool IsRemarkExist( RemarkFlags eRemarkFlag );

	// End of methods to manage remarks.
	/////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

	// Allow to get the class that manage resizing column feature.
	CResizingColumnInfo* GetResizingColumnInfo( void ) { return &m_clResizingColumnsInfo; }

	void DumpCellDescriptionList( void );

	// To change.
	void OnMouseEvent( int iMouseEventType, long lColumn, long lRow, CPoint ptMouse = CPoint( 0, 0 ) );

	// Do not call these methods. It is only used by the 'CMultiSpreadBase' class.
	void SetShowFlag( bool fShow ) { m_bShow = fShow; }
	bool GetShowFlag( void ) { return m_bShow; }

// Private method
private:
	void _InsertRowInSelectableList( long lRow );
	void _Clean( void );
	void _WriteOneRemark( RemarkFlags eRemarkFlag, int iIndex, long lRow, long lMergeColFrom, long lMergeColTo, CString strMessage = _T("") );

private:

	// Allow to save all user variables.
	// Remark: Pay attention if we set a user variable that is a pointer to structure, class or any other memory allocated,
	//         you need to free yourself. Map contains only pointer on data!
	mapUShortLParam m_mapUserVariable;

	// Allow to save all parameter column position. 
	vecLong m_vecParameterList;
	
	// Allow to save all selectable rows.
	vecLong m_vecSelectableRowsList;

	// Allow to manage cell type list.
	mapULongCellDescription m_mapCellDescriptionList;
	ULONG m_ulCellDescriptionID;
	
	UINT m_uiSheetDescriptionID;
	UINT m_uiSSheetID;
	CSSheet *m_pclSSheet;
	CPoint m_ptOffset;
	CPoint m_ptSize;

	CViewDescription* m_pclViewDescription;
	long m_lRowStart;
	long m_lRowEnd;
	short m_nRowsByItem;
	short m_nParameterMode;
	long m_lActiveColumn;
	long m_lSelectionFromColumn;
	long m_lSelectionToColumn;

	CResizingColumnInfo m_clResizingColumnsInfo;

	// Variables for remarks.
	class CSaveRemark
	{
	public:
		CSaveRemark( RemarkFlags eRemarkFlag ) { m_eRemarkflag = eRemarkFlag; m_strMessage = _T(""); m_lColumnFrom = -1; m_lColumnTo = -1; m_lRow = -1; }

		RemarkFlags m_eRemarkflag;				// Flag that determine type of remark to write.
		CString		m_strMessage;
		long		m_lColumnFrom;				// Column range to merge before write remark.
		long		m_lColumnTo;
		long		m_lRow;						// Row where is written the remark.
		std::vector< CPoint > m_vecTextList;	// Memorize position of all text with index.
	};
	typedef std::vector< CSaveRemark >			vecCSaveRemark;
	typedef vecCSaveRemark::reverse_iterator	vecCSaveRemarkRIter;
	vecCSaveRemark		m_vecRemarkList;
	bool m_bShow;
	UINT m_uiGroupID;
	UINT m_uiParentID;
};

class CViewDescription
{
public:

	//////////////////////////////////////////////////////////////////////////////////////////////////////////////
	// Enum definition
	//////////////////////////////////////////////////////////////////////////////////////////////////////////////

	enum ButtonType
	{
		ExpandCollapse = 0,
		ShowAllPriorities,
		Last
	};

	//////////////////////////////////////////////////////////////////////////////////////////////////////////////
	// Typedefs definition
	//////////////////////////////////////////////////////////////////////////////////////////////////////////////

	typedef std::map< int, CSheetDescription* > mapSheetDescription;
	typedef mapSheetDescription::iterator mapSheetDescriptionIter;
	typedef std::vector< CSheetDescription* > vecSheetDescription;
	typedef vecSheetDescription::iterator vecSheetDescriptionIter;


	CViewDescription() {}
	~CViewDescription();
	
	void Reset( void );

	// Allow to add a new sheet.
	// Param: 'uiSheetDescriptionID' is the unique sheet description identifier.
	// Param: 'uiAfterSheetDescriptionID' is -1 if we add the sheet description at the bottom or defined is we must insert the 
	//        new sheet just after this one.
	// Param: 'pclSheet' is the pointer on the newly sheet created.
	// Return: pointer on the new CSheetDescription created or NULL if any problem.
	CSheetDescription *AddSheetDescription( UINT uiSheetDescriptionID, UINT uiAfterSheetDescriptionID, CSSheet *pclSheet, CPoint m_ptOffset );

	// Allow to verify if a sheet exist.
	bool IsSheetDescriptionExist( UINT uiSheetDescriptionID );

	// Allow to remove only one specific sheet.
	// Param: 'uiSheetDescriptionID' is the sheet description ID to remove.
	// Return: 'false' is sheet description not found with ID, otherwise 'true'.
	bool RemoveOneSheetDescription( UINT uiSheetDescriptionID );

	// Allow to remove all sheet descriptions.
	// param: 'bAlsoFirstOne': 'true' if we must also remove the first sheet. Pay attention, the first sheet is not the first defined
	//        'm_SheetsDescriptionList' but really the first in terms of position (the sheet in top position).
	void RemoveAllSheetDescription( bool bAlsoFirstOne = false );

	// Allow to remove all sheets after a specific one.
	// Param: 'uiSheetDescriptionID' is the sheet description ID after which we can remove all other sheets.
	// Param: 'bSheetIncluded' is set to 'true' is we must always remove 'uiSheetID'.
	void RemoveAllSheetAfter( UINT uiSheetDescriptionID, bool bSheetIncluded = false );

	// Returns number of sheet defined.
	int GetSheetNumber( void );

	// Allow to retrieve what is the next selectable row after 'lRow'.
	// Param: 'pclSheetDescription' set in which sheet we are.
	// Param: 'lColumn' and 'lRow' is from where to start searching.
	// Param: 'bExcludeHidden' set to 'true' if you don't want select a hidden row.
	// Param: 'lSelectableRow' is where to save row if found or -1 if no more row is available.
	// Param: 'pclNewSheetDescription' is the pointer of the new sheet if next row has not been found in current sheet.
	// Remarks: we have also 'lColumn' parameter because on a line with multi parameter, we must also check if 'lColumn'
	//		    is not on a cell with no parameter (must be the case with accessories).
	void GetNextSelectableRow( CSheetDescription *pclSheetDescription, long lColumn, long lRow, bool bExcludeHidden, long &lSelectableRow, CSheetDescription *&pclNewSheetDescription );

	// Allow to retrieve what is the previous selectable row before 'lRow'.
	// Param: 'pclSheetDescription' set in which sheet we are.
	// Param: 'lColumn' and 'lRow' is from where to start searching.
	// Param: 'bExcludeHidden' set to 'true' if you don't want select a hidden row.
	// Param: 'lSelectableRow' is where to save row if found or -1 if no more row is available.
	// Param: 'pclNewSheetDescription' is the pointer of the new sheet if previous row has not been found in current sheet.
	// Remarks: we have also 'lColumn' parameter because on a line with multi parameter, we must also check if 'lColumn'
	//		    is not on a cell with no parameter (must be the case with accessories).
	void GetPrevSelectableRow( CSheetDescription *pclSheetDescription, long lColumn, long lRow, bool bExcludeHidden, long &lSelectableRow, CSheetDescription *&pclNewSheetDescription );

	// Allow to return complete list of all sheets defined.
	// Param: 'vecSheetDescriptionList' is where to save list.
	// Remark: returns list ordered by y position from top to bottom.
	void GetSheetDescriptionList( vecSheetDescription &vecSheetDescriptionList );

	// Allow to retrieve the top sheet (sheet at the top of window).
	// Return: NULL if no sheet.
	CSheetDescription *GetTopSheetDescription( void );

	// Allow to retrieve next sheet from a current one.
	// Param: 'pclCurrentSheet' from which to start searching the next one.
	// Param: 'uiParentID' if defined, the next sheet must have the same parent, otherwise return NULL.
	// Param: 'uiGroupID' if defined, the next sheet must be in the same group, otherwise return NULL.
	// Return: NULL if no sheet, no more or 'pclCurrentSheet' is invalid.
	CSheetDescription *GetNextSheetDescription( CSheetDescription *pclCurrentSheetDescription, UINT uiParentID = -1, UINT uiGroupID = -1 );

	// Allow to retrieve previous sheet from a current one.
	// Param: 'pclCurrentSheet' from which to start searching the previous one.
	// Return: NULL if no sheet, no more or 'pclCurrentSheet' is invalid.
	CSheetDescription *GetPrevSheetDescription( CSheetDescription *pclCurrentSheetDescription );

	// Allow to retrieve the bottom sheet (sheet at the bottom of window).
	// Return: NULL if there is no sheet, otherwise returns the sheet.
	CSheetDescription *GetBottomSheetDescription( void );

	// Allow to check if a sheet is above one other.
	enum
	{
		CompareSD_Error = -1,
		CompareSD_FirstAboveSecond,
		CompareSD_FirstBelowSecond,
		CompareSD_FirstEqualSecond
	};
	int CompareTwoSDPosition( CSheetDescription *pclSDToCompare, CSheetDescription *pclSDComparedTo );

	// Retrieve a sheet description with its ID.
	// Return: NULL if not found.
	// Remark: PAY ATTENTION: there is a difference between sheet description ID and SSheet ID!!
	//         see 'GetSheetDescriptionID' and 'GetSSheetID'.
	CSheetDescription *GetFromSheetDescriptionID( UINT uiSheetDescriptionID );

	// Retrieve a sheet description with SSheet ID.
	// Return: NULL if not found.
	// Remark: PAY ATTENTION: there is a difference between sheet description ID and SSheet ID!!
	//         see 'GetSheetDescriptionID' and 'GetSSheetID'.
	CSheetDescription *GetFromSSheetID( UINT uiSSheetID );

	// To change.
	void OnMouseEvent( CSSheet *pclSSheet, int iMouseEventType, long lColumn, long lRow, CPoint ptMouse = CPoint( 0, 0 ) );

private:
 	mapSheetDescription m_SheetsDescriptionList;
 	mapSheetDescriptionIter m_SheetsListIter;

	typedef std::vector<UINT>::iterator vecSheetOrderIter;
	std::vector<UINT> m_vecSheetOrder;
};
