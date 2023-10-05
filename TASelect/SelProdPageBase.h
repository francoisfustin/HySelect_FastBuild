#pragma once


#include "MultiSpreadInDialog.h"
#include "SelProdArticle.h"

#define SELPRODPAGE_SELECTIONBORDERCOLOR		RGB( 68, 189, 110 )
#define SELPRODPAGE_SELECTIONBORDERSTYLE		SS_BORDERSTYLE_FINE_DASH
#define SELPRODPAGE_SELECTIONBORDERWIDTH		3

// This a class to help us to save parameters linked to a product in the 'SelProdPageXXX'.
class CProductParam
{
public:
	CProductParam();

	// 'lRowStart' and 'lRowEnd' define the range of selectable rows.
	// 'lpParam' is the pointer on the container 'CDS_SSel...' class and 'pclProduct' on the product.
	CProductParam( CSheetDescription *pclSheetDescription, long lSelectionTopRow, long lSelectionBottomRow, LPARAM lpSelectionContainer, CData *pclProduct );
	CProductParam( const CProductParam &rProductParam );
	bool operator == ( const CProductParam &rProductParam );
	void Clear( void );

	void SetSheetDescription( CSheetDescription *pclSheetDescription ) { m_pclSheetDescription = pclSheetDescription; }
	void SetSelectionContainer( LPARAM lpParam ) { m_lpSelectionContainer = lpParam; }

	// Allow to add an additional row range for the selection.
	void AddRange( long lRowStart, long lRowEnd, CData *pclProduct );
	void UpdateRange( long lRowStartToUpdate, long lRowEndToUpdate, long lRowStartUpdate, long lRowEndUpdate );
	bool isRowInRange( long lRow );

	// Allow to retrieve product at the specified row or NULL if doesn't exist.
	CData *GetProduct( long lRow );

	// Allow to set a row range for the scrolling.
	// Remark: if these variables are not defined, we take the range defined by the selection row range ('m_lSelectionTopRow' and 'm_lSelectionBottomRow').
	//         For example, if user press key UP, we select the previous product if exists and do a vertical scrolling. In this case, we try to set the
	//         top of the selection to match the top of the view. If 'm_lScrollTopRow' is defined, we take this row as reference to match the top of the view.
	//         We can thus include for example title for the scrolling (title that is not selectable).
	void SetScrollRange( long lScrollTopRow, long lScrollBottomRow ) { m_lScrollTopRow = lScrollTopRow; m_lScrollBottomRow = lScrollBottomRow; }
	void GetScrollRange( long &lScrollTopRow, long &lScrollBottomRow );

	bool GetFirstSelectioRange( long &lFirstRow, long &lLastRow );
	bool GetNextSelectionRange( long &lFirstRow, long &lLastRow );

public:
	CSheetDescription *m_pclSheetDescription;
	long m_lSelectionTopRow;											// Contains the lowest row in all the defined selection row ranges.
	long m_lSelectionBottomRow;											// Contains the higher row in all the defined selection row ranges.
	LPARAM m_lpSelectionContainer;
	typedef std::map<std::pair<long, long>, CData *> mapPairLLCData;
	mapPairLLCData m_mapSelectionRangeList;								// Contains all row ranges for the selection.
	std::vector<CProductParam> m_vecLinkedProductParam;					// Contains a list of other 'CProductParam' classes linked to the current one.
	long m_lScrollTopRow;												// Either not defined and we take the 'm_lSelectionTopRow' or we take this one.
	long m_lScrollBottomRow;											// Either not defined and we take the 'm_lSelectionBottomRow' or we take this one.
	mapPairLLCData::iterator m_mapSelectionIter;
};

class CSelProdPageBase : public CMultiSpreadInDialog
{
public:
	//////////////////////////////////////////////////////////////////////////////////////////////////////////////
	// Enum definition
	//////////////////////////////////////////////////////////////////////////////////////////////////////////////
	enum ePointerType
	{
		eptHM,
		eptCBV,
		eptCSV,
		eptCDpC,
		eptCCV,
		eptCSmartControlValve,
		eptCSmartDpC,
		eptCMeasData
	};

	//////////////////////////////////////////////////////////////////////////////////////////////////////////////
	// Typedefs definition
	//////////////////////////////////////////////////////////////////////////////////////////////////////////////
	typedef std::pair<ePointerType, void *> PairPtr;
	typedef std::vector< std::pair<ePointerType, void *> > HMvector;

	//////////////////////////////////////////////////////////////////////////////////////////////////////////////
	// Defines definition
	//////////////////////////////////////////////////////////////////////////////////////////////////////////////
	enum SelProdHeaderRow
	{
		HR_FirstEmptyLine = 1,
		HR_PageTitle,
		HR_EmptyLine,
		HR_RowHeader,
		HR_FirstRow
	};

	CSelProdPageBase( CDB_PageSetup::enCheck ePage, CArticleGroupList *pclArticleGroupList, bool fUseOnlyOneSpread = true );
	virtual ~CSelProdPageBase( void ) {}
		
	virtual void Reset( void );

	virtual bool PreInit( HMvector &vecHMList ) { return false; }

	virtual bool Init( bool fResetOrder = false, bool fPrint = false ) { return false; }

	// Allow to know if current page has something to display or not.
	virtual bool HasSomethingToDisplay( void ) { return false; }

	// Allow to know if current page has something to print or not.
	virtual bool HasSomethingToPrint( void ) { return false; }

	virtual UINT GetTabTitleID( void ) { return -1; }

	// Called by 'CSelProdDockPane' when receiving WM_SIZE window message.
	virtual void OptimizePageSize( int iWidth, int iHeight );

	CDB_PageSetup::enCheck GetPageID( void ) { return m_ePageNum; }

	// Allow to retrieve a allocated memory containing info on the current selection and scrollbars position.
	// Remark: when we call 'CSelProdDockPane::RedrawAll', we destroy all 'CSelProdPageXXX'. It is thus impossible to save these data here.
	//         !! The memory must be freed by the caller !! For this, just call 'FreeSelectedProduct'.
	LPARAM BackupSelectedProduct( void );
	bool RestoreSelectedProduct( LPARAM lpParam );
	static void FreeSelectedProduct( LPARAM lpParam );
		
	int GetSSelSelectedNumbers( void );
	bool GetFirstSSelSelected( CString &strClassName, LPARAM &lpObj );
	int GetAllSSelSelected( CString &strClassName, CArray<LPARAM> &arlpObj );
	
	// Allow to retrieve a selection under the mouse pointer.
	// Param: 'ppclSSelObject' will contain the 'CDS_SSelXXX' object.
	// Param: 'ppclProduct' will contain current product. For example a DpC selection, there is one CDS_SSelDpc but there should have
	//        one Dp controller and one regulating valve depending where the mouse pointer is.
	bool GetSSelUnderMouse( CPoint ptMousePosScreen, CData **ppclSSelObject, CData **ppclProduct );
	
	void ClickOnSSel( LPARAM lpObj );

	void ClearAllProductSelected( void );

	CTable *GetpTableLinkedToPage( void ) { return m_pclSelectionTable; }
	
	// Allow to select the first product in a group.
	// Remark: if 'pclSheetDescription' is NULL, we select the first available product in the top sheet.
	//         if 'pclSheetDescription' is not NULL, we select the first available product in the corresponding sheet.
	void SelectFirstProduct( CSheetDescription *pclSheetDescription = NULL );

	// Overrides the 'CMultiSpreadBase' virtual public method.
	virtual void SSheetOnAfterPaint( void );
		
// Protected members.
protected:
	/////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
	// Overriding CMultiSpreadBase protected virtual methods.

	// Called by 'CellClickFpspread()' when user clicks on a cell.
	// Remark: must be implemented in derived classes.
	virtual void OnCellClicked( CSheetDescription *pclSheetDescription, long lClickedCol, long lClickedRow );

	// Called by 'DblClickFpspread()' when user double clicks on a cell.
	virtual void OnCellDblClicked( CSheetDescription *pclSheetDescription, long lClickedCol, long lClickedRow );

	// Called by 'RightClickFpspread()' when user right clicks on a cell.
	virtual void OnRightClick( CSheetDescription *pclSheetDescription, long lClickedCol, long lClickedRow, long lXPos, long lYPos ) {}

	// Called by 'PreTranslateMessage()' when user presses a key.
	// Return: 'TRUE' if message must not be dispatched.
	virtual BOOL OnKeyDown( CSheetDescription *pclSheetDescription, WPARAM wKeyPressed );
	virtual BOOL OnKeyUp( CSheetDescription *pclSheetDescription, WPARAM wKeyPressed ) { return FALSE; }

	// Called by 'TextTipFetch' when a sheet wants to show a tip.
	virtual bool OnTextTipFetch( CSheetDescription *pclSheetDescription, long lColumn, long lRow, WORD *pwMultiLine, SHORT *pnTipWidth, TCHAR *pstrTipText, BOOL *pbShowTip );

	virtual bool GetCurrentFocus( CSheetDescription *&pclSheetDescription, CRect &rectFocus ) { return false; }

	// End of overrides CMultiSpreadBase protected virtual methods.
	/////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

	bool PrepareSheet( CSheetDescription *pclSheetDescription, int iColNum, bool fPrint = false, bool fInitRows = true );

	virtual void SortTable( void );

	void SetColWidth( CSheetDescription *pclSheetDescription, int iColumn, double dSize );

	// Extent number of rows if needed
	void SetLastRow( CSheetDescription *pclSheetDescription, long lLastRow );

	// Set a staticText in cell (col, row), increment row and verify the maximum rows.
	long AddStaticText( CSheetDescription *pclSheetDescription, long lColumn, long lRow, CString strText, bool fAdaptRowHeightIfNeeded = false );

	// Overrides 'TSpread::AddCellSpan' to be sure to have enough row before row merging.
	void AddCellSpanW( CSheetDescription *pclSheetDescription, long lColumn, long lRow, long lNumCol, long lNumRow );

	void SetPageTitle( CSheetDescription *pclSheetDescription, int iIDstr, bool bMain = true, long lRowOffset = 0, double dRowHeight = 0.0, 
			COLORREF clTextColor = -1, COLORREF clBackgroundColor = -1, int iFontSize = -1 );

	void SetPageTitle( CSheetDescription *pclSheetDescription, CString strTitle, bool bMain = true, long lRowOffset = 0, double dRowHeight = 0.0, 
			COLORREF clTextColor = -1, COLORREF clBackgroundColor = -1, int iFontSize = -1 );

	long FillRowGen( CSheetDescription *pclSheetDescription, long lRow, CDS_SelProd *pSelCtrl );
	long FillRowGen( CSheetDescription *pclSheetDescription, long lRow, CDS_HydroMod *pHM, CDS_HydroMod::CBase *pBase = NULL );
	
	// HYS-1734: Like FillRowGen function but for secondary products. For example the balancing valve selected
	// with the DpC should have its own temperature. Same case with DpCBCV and STS valve.
	long FillRowGenOthers( CSheetDescription *pclSheetDescription, long lRow, CDS_HydroMod *pHM, CDS_HydroMod::CBase *pBase = NULL );

	void FillArtNumberCol( CSheetDescription *pclSheetDescription, long lColumn, long lRow, CDB_Product *pclProduct, CString strArtNumberStr );
	
	// It's a special one for 'DpC-Mv' and 'Ctrl-Actuator' set.
	void FillArtNumberColForPackage( CSheetDescription *pclSheetDescription, long lColumn, long lRow, CDB_Set *pclSet, CString strArtNumberStr );

	void FillQtyPriceCol( CSheetDescription *pclSheetDescription, long lFirstRow, CData *pclProduct, int iQuantity, bool fShowPrice = true );

	long FillRemarks( CSheetDescription *pclSheetDescription, long lFirstRow, bool *pfRemarkExist = NULL );
	long FillHMRemarks( HMvector *pvecHMList, CSheetDescription *pclSheetDescription, long lFirstRow, bool *pfRemarkExist = NULL );

	long FillAccessories( CSheetDescription *pclSheetDescription, long lRow, CDS_SSel *pSSel, int iGroupQuantity, int iDistributedQty = 1, CProductParam *pclProductParam = NULL );
	// HYS-2007: Add the choice to display or not the accessory's article number.
	long FillAccessories( CSheetDescription *pclSheetDescription, long lRow, CAccessoryList *pclAccessoryList, int iGroupQuantity, int iDistributedQty = 1, CProductParam *pclProductParam = NULL, bool bWithArticleNumber = true );
	long FillAndAddBuiltInHMAccessories( CSSheet *pclSheet, CSheetDescription *pclSheetDescription, CDB_TAProduct *pTAprod, long lRow );

	virtual long FillAccessory( CSheetDescription *pclSheetDescription, long lRow, CDB_Product *pAccessory, bool fWithArticleNumber, int iQuantity, 
			CProductParam *pclProductParam = NULL, LPARAM lpParam = NULL );

	virtual void FillAccessoryHelper_GetNameDescription( CDB_Product *pclAccessory, CString &strName, CString &strDescription, LPARAM lpParam = NULL );
	virtual void FillAccessoryHelper_GetErrorMessage( CDB_Product *pclAccessory, int iCol, std::vector<CString> *pvecErrorMsgList, LPARAM lpParam = NULL );

	long FillPNTminTmax( CSheetDescription *pclSheetDescription, long lColumn, long lRow, CDB_Product *product );

	// Save product.
	void SaveProduct( CProductParam &clProductParam );

	void AddCompArtList( CDB_TAProduct *pTAP, int iQuantity, bool fInlet );
	void AddArticleInGroupList( CArticleGroup *pclArticleGroup );
		
	// Allow to add an sub article to an existent article.
	// Remark: if 'pclArticle' is NULL we add an article and not a sub article.
	void AddAccessoryInArticleContainer( CDB_Product *pclAccessory, CArticleContainer *pclArticleContainer, int iQuantity, bool bSelectionBySet );
		
	// Allow to add a list of sub articles to an existent article.
	// Param: 'bForceToInsertInArticleContainer' set to 'true' if we must insert this article in 'pclArticleContainer'.
	// Remark: if 'pclArticleContainer' is NULL we add a list of article and not a list of sub article.
	void AddAccessoriesInArticleContainer( CAccessoryList *pclAccessoryList, CArticleContainer *pclArticleContainer, int iQuantity, bool bSelectionBySet, 
			int iDistributedQty = 1, bool bForceToInsertInArticleContainer = false );
	
	// Allow to add a list of sub articles to an existent article.
	// Param: 'bForceToInsertInArticleContainer' set to 'true' if we must insert this article in 'pclArticleContainer'.
	// Remark: if 'pclArticleContainer' is NULL we add a list of article and not a list of sub article.
	void AddAccessoriesInArticleContainer( std::vector<CDS_Actuator::AccessoryItem> *pvecAccessoryList, CArticleContainer *pclArticleContainer, 
			int iQuantity, bool bSelectionBySet, int iDistributedQty = 1, bool bForceToInsertInArticleContainer = false );

	// Allow to add a list of sub articles to an existent article.
	// Param: 'bForceToInsertInArticleContainer' set to 'true' if we must insert this article in 'pclArticleContainer'.
	// Remark: if 'pclArticleContainer' is NULL we add a list of article and not a list of sub article.
	void AddAccessoriesInArticleContainer( CArray<IDPTR> *parAccessoryList, CDB_RuledTableBase *pclRuledTable, CArticleContainer *pclArticleContainer, 
			int iQuantity, bool bSelectionBySet, int iDistributedQty = 1, bool bForceToInsertInArticleContainer = false );

	// Allow the inherited class to be warned when a cell is hovered by the mouse pointer.
	// Returns 'true' if current method has displayed a tooltip.
	virtual bool OnTextTipFetch( long lColumn, long lRow, WORD *pwMultiLine, SHORT *pnTipWidth, TCHAR *pstrTipText, BOOL *pbShowTip ) { return false; }

	virtual void OnLeftColChanged( void ) {}
	virtual void OnTopRowChanged( void ) {}

	// Allow the inherited class to be warned when a cell is selected (By key, click, other).
	virtual void OnSelectCell( CProductParam &clProducParam ) {}
	virtual void OnUnselectCell( CProductParam &clProducParam ) {}

	void WriteTextWithFlags( CSheetDescription *pclSheetDescription, CString strText, long lColumn, long lRow, CSheetDescription::RemarkFlags eRemarkFlag, 
			CString strMessage = _T(""), COLORREF cTextColor = _BLACK, COLORREF cBackColor = _WHITE );

	void WriteTextWithFlags( CSheetDescription *pclSheetDescription, CString strText, long lColumn, long lRow, CSheetDescription::RemarkFlags eRemarkFlag, 
			std::vector<CString> vecErrorMessages, COLORREF cTextColor = _BLACK, COLORREF cBackColor = _WHITE );

	void AddTooltipMessage( CPoint pt, CString strTooltipMessage );
	void AddTooltipMessage( CPoint pt, std::vector<CString> vecTooltipMessages );

// Protected variables
protected:
	// Remark: header column must be set to the first position (index 1).
	//         pointer column is the last (=m_iColNum).
	//         footer column must be set just before the pointer column (m_iColNum-1).
	enum ColumnDescription
	{
		Header			= 1,
		Reference1		= 2,
		Reference2		= 3,
		Water			= 4,
		Product			= 5,
		TechnicalInfos	= 6,
		RadiatorInfos	= 7,
		ArticleNumber	= 8,
		Pipes			= 9,
		Quantity		= 10,
		UnitPrice		= 11,
		TotalPrice		= 12,
		Remark			= 13,
		Footer			= 14,
		Pointer			= 15
	};
	enum DefaultColumnWidth
	{
		DCW_Header			= 2,
		DCW_Reference1		= 10,
		DCW_Reference2		= 10,
		DCW_Water			= 10,
		DCW_Product			= 12,
		DCW_TechnicalInfos	= 12,
		DCW_RadiatorInfos	= 10,
		DCW_ArticleNumber	= 10,
		DCW_Pipes			= 15,
		DCW_Quantity		= 5,
		DCW_UnitPrice		= 7,
		DCW_TotalPrice		= 7,
		DCW_Remark			= 2,
		DCW_Footer			= 2,
		DCW_Pointer			= 0
	};

	bool m_bInitialized;
	typedef std::vector< CProductParam >::iterator vecProductRegisteredIter;
	std::vector< CProductParam > m_vecProductSelected;
	typedef std::vector< CProductParam >::iterator vecProductSelectedIter;
	int m_iProductSelectedIndex;

	typedef std::map< long, CProductParam > mapProduct;
	typedef mapProduct::iterator mapProductIter;
	typedef struct _SheetInfos
	{
		int m_iColNum;
		double m_dPageWidth;
		std::map<int, long> m_mapColWidth;			// map column ID and column width in pixels.
		mapProduct m_mapProductRegistered;
		struct _SheetInfos()
		{
			m_iColNum = 0;
			m_dPageWidth = 0.0;
			m_mapColWidth.clear();
		}
	}SheetInfos;
	typedef std::map<CSheetDescription *, SheetInfos> mapSheetInfo;
	typedef mapSheetInfo::iterator mapSheetInfoIter;
	mapSheetInfo m_mapSheetInfos;

	typedef struct _BackupSelPos
	{
		int m_iCheckMemID;
 		UINT m_uiSheetDescriptionID;
		LPARAM m_lpParam;
	}BackupPosSel;

	bool m_bForPrint;
	CDB_PageSetup::enCheck m_ePageNum;
	double m_dRowHeight;

	HMvector m_vecHMList;
	CTable *m_pclSelectionTable;
	CTADatabase *m_pTADB;
	CTADatastruct *m_pTADS;
	CDB_PageSetup *m_pTADSPageSetup;
	CUnitDatabase *m_pUnitDB;
	CDS_TechnicalParameter *m_pclTechParam;
	CArticleGroupList *m_pclArticleGroupList;
	CProductParam m_rProductParam;

	class classcomp { public: bool operator() ( const CPoint &pt1, const CPoint &pt2) const { if( pt1.x == pt2.x ) { return ( pt1.y > pt2.y ); } return ( pt1.x < pt2.x ); } };
	std::map<CPoint, std::vector<CString>, classcomp> m_mapTooltipMessages;

// Private methods.
private:
	struct SelectionBorderInfo
	{
		CPoint m_ptTopLeftColRow;
		CPoint m_ptTopRightColRow;
		CPoint m_ptBottomLeftColRow;
		CPoint m_ptBottomRightColRow;
		CSheetDescription *m_pclSheetDescription;
		std::vector<SelectionBorderInfo> m_vecLinkedSelectionBorderInfo;	
	};

	void _GetFirstProduct( CProductParam &clFirstProductParam, CSheetDescription *pclSheetDescription );
	void _GetLastProduct( CProductParam &clLastProductParam, CSheetDescription *pclSheetDescription );
	void _GetNextProduct( CProductParam &clFromProductParam, CProductParam &clDstProductParam );
	void _GetPrevProduct( CProductParam &clFromProductParam, CProductParam &clDstProductParam );
	bool _GetProduct( CSheetDescription *pclSheetDescription, long lRow, CProductParam &clProductParam );
	
	// Allow to retrieve the size in pixels relative to the current window of the current selection.
	// If 'clProductSelected' has no param, we take all the current selected products.
	CRect _GetSelectionSize( CProductParam &clProductSelected );

	CRect _GetScrollingRange( CProductParam &clProductSelected );

	enum
	{
		CompareProduct_Error = -1,
		CompareProduct_FirstAboveSecond,
		CompareProduct_FirstBelowSecond,
		CompareProduct_FirstEqualSecond
	};
	int _CompareTwoProductPosition( CProductParam &clFirstProductParam, CProductParam &clSecondProductParam );
	
	// In this case we have the sheet description pointer and we want to check if of product is registered in this sheet.
	bool _IsProductRegistered( CSheetDescription *pclSheetDescription, LPARAM lpParam, CProductParam &clProductParam );

	// In this case we don't have the sheet description pointer and we must search in all sheet.
	bool _IsProductExist( LPARAM lpObj, CProductParam &clProductParam, CSheetDescription *pclSheetDescription = NULL );
	
	void _SelectCell( CProductParam &clProductToSelect, CSSheet::_SSKeyBoardPressed eKeyPressed = CSSheet::_SSKeyBoardPressed::NoKey );

	// Helpers for '_SelectCell'.
	void _SelectCHelper( CProductParam &clProductToSelect, bool bFirst = true );

	// Param: 'bForceToInsertInArticleContainer' set to 'true' if we must insert this article in 'pclArticleContainer'.
	void _VerifyAndAddAccessoriesInArticleContainer( std::vector<CAccessoryList::AccessoryItem> vecAccessoryList, CArticleContainer *pclArticleContainer, 
			int iQuantity, bool bSelectionBySet, int iDistributedQty = 1, bool bForceToInsertInArticleContainer = false );

	bool _TestForSameArticle( CArticleGroup *pclArticleGroupToTest, bool bIncrementation );

	// Just use for 'sort' routine that needs a static function.
	static bool _ComparePairs( PairPtr i, PairPtr j );

	void _InvalidateSSheetRect();

	// Allow to draw a border around the current selections.
	void _DrawSelectionBorders();

	// Allow to draw a border around one selection.
	void _DrawOneSelectionBorder( SelectionBorderInfo rSelectionBorderInfo, CDC *pDC, long lXWindowOffset, long lYWindowOffset );
	void _DrawTopBorder( SelectionBorderInfo rSelectionBorderInfo, CDC *pDC, long lXWindowOffset, long lYWindowOffset );
	void _DrawVerticalBorders( SelectionBorderInfo rSelectionBorderInfo, CDC *pDC, long lXWindowOffset, long lYWindowOffset );
	void _DrawBottomBorder( SelectionBorderInfo rSelectionBorderInfo, CDC *pDC, long lXWindowOffset, long lYWindowOffset );

// Private variables.
private:
	std::vector<SelectionBorderInfo> m_vecSelectionBorderInfo;

	enum SelectionKey
	{
		Selection_NoKey,
		Selection_Ctrl,
		Selection_Shift
	};

	SelectionKey m_eSelectionKey;
};
