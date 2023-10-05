#pragma once


#include <list>
#include "ExtListCtrl.h"
#include "ExtTreeCtrl.h"
#include "ExtStatic.h"
#include "ProductSelectionParameters.h"

using namespace std;

#define DLGDIRSEL_LEFTCOLUMN	0
#define DLGDIRSEL_RIGHTCOLUMN	1
#define DLGDIRSEL_AJUSTCOORDS( Left, Right)	{ Left = max( Left, Right); Right = Left; }

class CDlgDirSel : public CDialogEx
{
	DECLARE_DYNAMIC( CDlgDirSel )

	enum { IDD = IDD_DLGDIRSEL };

public:
	CDlgDirSel( CWnd *pParent = NULL );
	virtual ~CDlgDirSel();

protected:
	class CTreeItemType;
	class CTreeItemFamily;
	class CTreeItemAccessory;
	class CAccessorySelected;

	DECLARE_MESSAGE_MAP()
	virtual void DoDataExchange( CDataExchange *pDX );
	virtual BOOL OnInitDialog();

	// Called when selection changes in the left tree ('Product categories').
	afx_msg void OnTvnSelChangedTreeCategory( NMHDR *pNMHDR, LRESULT *pResult );
	afx_msg void OnTvnSelChangedTreeProducts( NMHDR *pNMHDR, LRESULT *pResult );
	afx_msg void OnNMEnterTreeCategory( NMHDR *pNMHDR, LRESULT *pResult );
	afx_msg void OnNMEnterTreeProducts( NMHDR *pNMHDR, LRESULT *pResult );
	afx_msg void OnNMClickTreeProducts( NMHDR *pNMHDR, LRESULT *pResult );
//	afx_msg void OnNMSetFocusTreeProducts( NMHDR *pNMHDR, LRESULT *pResult );
	afx_msg void OnNMKillFocusTreeProducts( NMHDR *pNMHDR, LRESULT *pResult );
	afx_msg void OnBnClickedSelect();
	afx_msg void OnBnClickedCancel();
	afx_msg void OnBnClickedDirSelExport();
	afx_msg void OnBnClickedDirSelTender();
	afx_msg void OnBnClickedDeletedValves();
	afx_msg void OnEnChangeEditsearchprod();
	afx_msg void OnCbnSelchangeComboselection();
	afx_msg void OnBnClickedButtonsearch();

	// Fill the left part of the dialog named 'Product categories'.
	// Remark: This method will first fill product categories that are in 'CATEGORY_TAB' (level 1)
	//         For each category, we know table ID where we can found table containing all types linked to the category (level 2).
	//         And finally, for each type we can fill families in regards to it's a product or an actuator (level 3).
	void FillTreeProductCategories();

	// Called by 'FillTreeProductCategories' to fill product families in level 3.
	// Example: + Regulating Valve			( level 1)
	//          | + Balancing Valve			( level 2)
	//          | | -STAD					( level 3)
	//          | | -STADA					( level 3)
	//          | | -STAD-C					( level 3)
	//          ...
	// Remark: 'TPC' if for 'Tree Product Categories' (left tree of the dialog).
	// Returns: Total count of product displayed.
	int FillTPC_ProductFamilies( CTreeItemType *pclTreeItemType, HTREEITEM hCurrentItem, bool bForSetOnly = false );

	// Called by 'FillTreeProductCategories' to fill actuator families in level 3.
	// Example: + Actuator					( level 1)
	//          | + Electric				( level 2)
	//          | | -EMO 1/EMO 3 family		( level 3)
	//          | | -TA-NV24/R25 family		( level 3)
	//          | | -MC family				( level 3)
	//          ...
	// Remark: 'TPC' if for 'Tree Product Categories' (left tree of the dialog).
	// Returns: Total count of actuator displayed.
	int FillTPC_ActuatorFamilies( CTreeItemType *pclTreeItemType, HTREEITEM hCurrentItem );

	// HYS-1940: TA-Smart Dp - 07 - Add smart DpC in the confirm selection dialog.
	// See FillTPC_ActuatorFamilies for description.
	int FillTPC_MeasToolsFamilies( CTreeItemType *pclTreeItemType, HTREEITEM hCurrentItem );

	// Called by 'OnTvnSelChangedTreeCategory' when user select an Product in 'Product Categories' tree.
	// Remark: 'TP' if for 'Tree Products' (middle tree of the dialog).
	void FillTP_Products( CTreeItemFamily *pclTreeItemFamily, bool bSetAlone = false );

	// Called by 'OnTvnSelChangedTreeCategory' when user select an actuator in 'Product Categories' tree.
	// Remark: 'TP' if for 'Tree Products' (middle tree of the dialog).
	void FillTP_Actuators( CTreeItemFamily *pclTreeItemFamily );

	// HYS-1940: TA-Smart Dp - 07 - Add smart DpC in the confirm selection dialog.
	// Called by 'OnTvnSelChangedTreeCategory' when user select a measuring tool famuly in 'Product Categories' tree.
	// Remark: 'TP' if for 'Tree Products' (middle tree of the dialog).
	void FillTP_MeasTools( CTreeItemFamily* pclTreeItemFamily );

	// Called by 'FillTP_Products' if product (level 1) contains compatible actuators that we must show at level 2.
	// Remark: 'TP' if for 'Tree Products' (middle tree of the dialog).
	void FillTP_CompatibleActuators( CTreeItemFamily *pclTreeItemFamily );
	
	// HYS-945: Buffer vessel management
	void FillTP_CompatibleBufferVessels( CTreeItemFamily *pclTreeItemFamily );
	void FillTP_IntegratedAccessories( CTreeItemFamily *pclTreeItemFamily, HTREEITEM hItemParent );

	// Called by 'FillTP_CompatibleActuators'.
	void FillTP_CompatibleActuatorsHelper( CTreeItemFamily *pclTreeItemFamily, HTREEITEM hCurrentItem, CRank &clActuatorList, CDB_Product *pclProduct );

	// Called by 'FillTP_Products' if product (level 1) contains compatible adapters that we must show at level 2.
	// Remark: 'TP' if for 'Tree Products' (middle tree of the dialog).
	void FillTP_CompatibleAdapters( CTreeItemFamily *pclTreeItemFamily );

	// Called by 'FillTP_Products' for differential pressure controller (level 1) to show compatible measuring valve (level 2).
	// Remark: 'TP' if for 'Tree Products' (middle tree of the dialog).
	void FillTP_CompatibleMeasuringValves( CTreeItemFamily *pclTreeItemFamily );

	// Called by 'FillTP_Products' for smart differential pressure controller (level 1) to show compatible Dp sensor (level 2).
	// Remark: 'TP' if for 'Tree Products' (middle tree of the dialog).
	void FillTP_CompatibleSmartDpCSets( CTreeItemFamily *pclTreeItemFamily );

	// Called by 'FillTreeProducts' if product (level 1) contains compatible accessories that we must show at level 2.
	// Remark: 'TP' if for 'Tree Products' (middle tree of the dialog).
	// Remark: 'bIsForSet' is added because with only the two 'm_bWorkingForDpCSet' and 'm_bWorkingForControlSet' variables it is not enough.
	//         For example, if we are in a selection by set for a smart differential pressure controller, the 'm_bWorkingForDpCSet' variable will be 'true'.
	//         But when accessory for the smart differential pressure controller are not in a set. So we need to be able to make the difference.
	void FillTP_CompatibleAccessories( CTreeItemFamily *pclTreeItemFamily, HTREEITEM hMainItem = NULL, bool bIsForSet = false );

	void FillProductDetails( CDB_Product *pclProduct, CExtListCtrl *pExtListCtrl );

	// HYS-1992: Fill fictif set accessories detail.
	void FillDpSAccessoryDetails( CDB_Product* pAccessory, CExtListCtrl* pExtListCtrl, bool bHideArticleNumber, bool bByPair );
	
	// HYS-1940: TA-Smart Dp - 07 - Add smart DpC in the confirm selection dialog.
	// Add a bool param. bHideArticleNumber to specify when the accessory not attached but belong to a set.
	// In this case we don't show article number.
	void FillAccessoryDetails( CDB_Product *pAccessory, CExtListCtrl *pExtListCtrl, bool bHideArticleNumber = false, bool bByPair = false );
	void FillAccessoryDetailsHelper_GetNameDescription( CDB_Product *pAccessory, CString &strName, CString &strDescription );

	void FillAdapterDetails( CDB_Product *pclAdapter, CExtListCtrl *pExtListCtrl );
	void FillActuatorDetails( CDB_Actuator *pActuator, CExtListCtrl *pExtListCtrl );

	void FillImage( CDB_Product *pclProduct );

	void ClickTreeProducts( HTREEITEM hItem );
	void ClickProduct( HTREEITEM hItem, bool fUserClick = false );
	void ClickAccessory( HTREEITEM hItem );
	void ClickAdapter( HTREEITEM hItemSelected );
	void ClickActuator( HTREEITEM hItemSelected );
	// HYS-945: Buffer vessel management
	void ClickIntegrated( HTREEITEM hItem );

	void CleanVectorOfData( int iDataType );

	// Some accessories can be selected because there are "Implacted" with one other. In this case, when user selects this other accessory,
	// we also check the implicated accessory and we disable the checkbox. If user deselect the product, we reset the checkbox by the
	// state 'CheckboxNotSelected'. But for some circumstances, we need checkboxes that must be always disabled. For example set content
	// for a smart differential pressure controller set selection.
	// HYS-1992: If we have two groupes of accessories with different item type (radio and checkbox), and click on radio
	// only accessories with radio button are cleaned.
	void CleanVectorOfSelectedAccessories( std::vector<CAccessorySelected> *pvecAccessorySelected, bool bKeepDisabledState = false, int iButtonType = 0 );
	void CleanOneLevel( CExtTreeCtrl *pTreeCtrl, HTREEITEM hItem );
	void RemoveEmptyNode( CExtTreeCtrl *pTreeCtrl, int iToplevel = 0, HTREEITEM HItem = NULL, int iCurLevel = 0 );
	void ResetFocus();
	void GetBuiltInAccessories( HTREEITEM hItem, HTREEITEM hItemProduct = NULL );
	// HYS-1992: Get builtin accessories for fictif set.
	void GetBuiltInAccessoriesForAccessory( HTREEITEM hItem );
	void GetLinkedAccessories( HTREEITEM hItem, HTREEITEM hParentItem, bool fForProduct, CDB_RuledTable::MMapName rule );
	void RemoveLinkedAccessories( HTREEITEM hItem, bool fForProduct );
	bool ConfirmSelection( CData *pSel, CString strTabID );

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
	// This function is created to manage adapter or accessory that is included in one product accessory (Stem heater).
	// The adapter is grayed out and unchecked when the stem heater is selected.
	// This is called when an action is needed on included items to not select them two times. 
	// Included items could be under different item but has the same product parent.
	void VerifyOtherExcludedProduct( HTREEITEM hItemSelected, HTREEITEM hParentItem, std::vector<CData *> &vectOtherExcluded );

	/**
	 * This function Search the Family Item corresponding to pfam on Category Tree 
	 * @author awa
	 * @param  (I) pfam: The family to search 
	 * @param  (I) hItem: The item where the search begin on
	 * @param  (IO) pfound: The booleen that stop recursivity if the value is true
	 * @param  (I) bIsAset: is true if the search concern a Set
	 * @remarks : Created by HYS791
	 */
	void FindFamilyInCategory(CData* pfam, HTREEITEM hItem, bool* pfound, bool bIsAset);

   /**
	* This function found the Item correspondind to IdptrProductID and display it
	* @author awa
	* @param  (I) IdptrProductID: IDPTR to search
	* @param  (I) hItem: The item where the search begin on
	* @param  (IO) pfound: The booleen that stop recursivity if the value is true
	* @param  (I) bIsAset: is true if the search concern a Set
	* @remarks : Created by HYS791
	*/
	void SelectAndDisplayProduct(IDPTR IdptrProductID, HTREEITEM hItem, bool* bPfound, bool bIsAset);

	// HYS-1992: Identify fictif set for Smart DpC.
	bool IsAccessoryFictifSet( CDB_Product* pclProductParent, CDB_Product* pclFictifSet );
	
	// If we have builtin accessory under item before cleaning.
	void HasBuiltinAccessory( CDB_Product* pclProduct, CDB_Product* pclProductSub, bool &bBuiltinExists, bool &bBuiltinExistsSub );

// Private members.
private:
	bool _IsSelectable( CDB_Thing *pclThing );

	CDB_Product *_GetAdapterCommonToValveAndActuator( CDB_Product *pclProduct, CDB_Actuator *pclActuator );

	void _GetActuatorListRelatedToControlValve( CDB_ControlValve *pclControlValve, CRank *pclActuatorList, bool fWorkingForASet );
	void _GetActuatorListRelatedToShutOffValve( CDB_ShutoffValve *pclShutOffValve, CRank *pclActuatorList );
	
	// For an explanation of the 'bProductAccessoryKeepDisabledState' and 'bSubProductAccessoryKeepDisabledState' arguments
	// see the comment for the 'CleanVectorOfSelectedAccessories' method.
	void _CleanCurrentProduct( bool bAlsoChildren = true, bool bProductAccessoryKeepDisabledState = false, bool bSubProductAccessoryKeepDisabledState = false );

	// Allow to retrieve the parent of an children item. This method will stop to the first parent encounter when going up
	// in the hierarchy. If we have for example a TA-Fusion-P, compatible actuator and compatible accessory on this actuator, the parent of
	// the accessory will be the actuator. Except if you put 'bRootParent' to 'true'. In this case this the TA-Fusion-P that will be return.
	HTREEITEM _GetParentProduct( HTREEITEM hChildItem, bool bRootParent = false );

	// Allow to retrieve all children of an parent item.
	void _GetChildrenProduct( HTREEITEM hParentItem, std::vector<HTREEITEM> &vecChildren );

	HTREEITEM _GetAccessoryTitle( HTREEITEM hItemProduct );
	HTREEITEM _GetAdapterTitle( HTREEITEM hItemProduct );
	HTREEITEM _GetActuatorTitle( HTREEITEM hItemProduct );
	HTREEITEM _GetMeasValveTitle( HTREEITEM hItemProduct );
	HTREEITEM _GetSmartDpCSetTitle( HTREEITEM hItemProduct );
		
	// Allow to select an adapter an do adequate operation.
	// Param: - if 'hAdapterItem' is NULL, we verify if we have a control valve and an actuator already selected. In this case, we select 
	//          adapter that matches.
	//        - if 'hAdapterItem' is not NULL we directly choose this one.
	void _SelectAdapter( HTREEITEM hAdapterItem = NULL );

	// Allow to unselect all adapter below the title.
	// Param: 'fForceDisable' - set to 'true' if we must disable adapters.
	//                        - set to 'false' if we disable only if we are working by set selection.
	void _UnselectAllAdapters( HTREEITEM hAdapterTitleItem, bool bForceDisable = false );

	void _UpdateSelectButton( void );

	// Crossing accessories are those that are common between two products. For example we have stem heater between some
	// valves and actuators. In this case, we have the stem heater accessory in the valve accessory table (Prefixed by ++)
	// and the same accessory in the actuator accessory table (Prefixed by +) -> See 'CDB_RuledTable' class in 'DataBObj.h'
	// for more details. When clicking on the actuator we need to know if we must check/uncheck the common accessory in 
	// the control valve.
	void _CheckCrossingAccessoriesCVAndActuator( HTREEITEM hControlValve, HTREEITEM hActuator, bool bCheck );

	// Allow to retrieve of all accesories linked to a product.
	void _GetAccessoryList( std::vector<CTreeItemAccessory *> &vecAccessoryList, HTREEITEM hParent );

	bool _CanDisplayType( CString strTypeID );

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
	// This function is called when we select or unselect adapter. Wr verify if the adapter is
	// not included in other selected item before enabled, disabled, checked or unchecked
	bool _IsAdapterIncludedInASelectedItem( CDB_Product * pclProductIncluded, HTREEITEM hItemActuator);

// Protected variables.
protected:
	// To say if the item is select, not select, not selectable, check, etc.
	enum
	{
		RadioNotSelected			= 1,
		RadioSelected				= 2,
		RadioNotSelectedDisable		= 3,
		RadioSelectedDisable		= 4,
		CheckboxNotSelected			= 5,
		CheckboxSelected			= 6,
		CheckboxNotSelectedDisable	= 7,
		CheckboxSelectedDisable		= 8
	};

	struct sSelectedItem
	{
		HTREEITEM m_hTreeItem;
		HTREEITEM m_hTreeItemSub;	// Use to follow measuring valve in a DpC Set
		void Clean() { m_hTreeItemSub = m_hTreeItem = NULL; }
	};
	
	class CTreeItemData
	{
	public:
		typedef enum DataType{ Category = 0x0001, SubCategory = 0x0002, Type = 0x0004, Family = 0x0008, Product = 0x0010, Adapter = 0x0020, Actuator = 0x0040, Accessory = 0x0080, Title = 0x0100, Integrated = 0x0200,
			AllMiddle = ( Product | Adapter | Actuator | Accessory | Title | Integrated), 
			All = ( Category | SubCategory | Type | Family | AllMiddle ) };
		CTreeItemData( DataType eDataType ) { m_hTreeItem = (HTREEITEM)0; m_eDataType = eDataType; }
		virtual ~CTreeItemData() {}
		virtual CDB_StringID *GetStringCategoryID( void ) { return NULL; }
		virtual CDB_MultiString *GetMultiStringSubCategory( void ) { return NULL; }
		virtual CDB_StringID *GetStringTypeID( void ) { return NULL; }
		virtual CDB_StringID *GetStringFamilyID( void ) { return NULL; }
		DataType m_eDataType;
		HTREEITEM m_hTreeItem;
	};

	class CTreeItemTitle : public CTreeItemData
	{
	public:
		typedef enum TitleType{ Undefined, CompatibleAccessories, CompatibleAdapters, CompatibleActuators, CompatibleMeasValve, CompatibleBufferVessels, CompatibleSmartDpCSets };

		CTreeItemTitle() : CTreeItemData( CTreeItemData::DataType::Title ) { m_eTitleType = TitleType::Undefined; }
		CTreeItemTitle( TitleType eTitleType ) : CTreeItemData( CTreeItemData::DataType::Title ) { m_eTitleType = eTitleType; }
		virtual ~CTreeItemTitle() {}
		TitleType m_eTitleType;
	};

	class CTreeItemCategory : public CTreeItemData
	{
	public:
		CTreeItemCategory() : CTreeItemData( CTreeItemData::DataType::Category ) 
			{ m_pclCategoryID = NULL; }
		CTreeItemCategory( CDB_StringID *pclCategoryID ) : CTreeItemData( CTreeItemData::DataType::Category ) 
			{ m_pclCategoryID = pclCategoryID; }
		virtual ~CTreeItemCategory() {}
		virtual CDB_StringID *GetStringCategoryID( void ) { return m_pclCategoryID; }
		CDB_StringID *m_pclCategoryID;
	};

	class CTreeItemSubCategory : public CTreeItemData
	{
	public:
		CTreeItemSubCategory() : CTreeItemData( CTreeItemData::DataType::SubCategory ) 
			{ m_pclSubCategory = NULL; m_pclParentCategoryID = NULL; }
		CTreeItemSubCategory( CDB_MultiString *pclSubCategory, CTreeItemCategory *pclTreeItemCategory ) : CTreeItemData( CTreeItemData::DataType::SubCategory ) 
			{ m_pclSubCategory = pclSubCategory; m_pclParentCategoryID = pclTreeItemCategory->m_pclCategoryID; }
		virtual ~CTreeItemSubCategory() {}
		virtual CDB_StringID *GetStringCategoryID( void ) { return m_pclParentCategoryID; }
		virtual CDB_MultiString *GetMultiStringSubCategory( void ) { return m_pclSubCategory; }
		CDB_StringID *m_pclParentCategoryID;
		CDB_MultiString *m_pclSubCategory;
	};

	class CTreeItemType : public CTreeItemData
	{
	public:
		CTreeItemType() : CTreeItemData( CTreeItemData::DataType::Type ) 
			{ m_pclTypeID = NULL; m_pclParentSubCategory = NULL; m_pclParentCategoryID = NULL; }
		CTreeItemType( CDB_StringID *pclTypeID, CTreeItemSubCategory *pclTreeItemSubCategory ) : CTreeItemData( CTreeItemData::DataType::Type ) 
			{ m_pclTypeID = pclTypeID; m_pclParentSubCategory = pclTreeItemSubCategory->m_pclSubCategory; m_pclParentCategoryID = pclTreeItemSubCategory->m_pclParentCategoryID; }
		virtual ~CTreeItemType() {}
		virtual CDB_StringID *GetStringCategoryID( void ) { return m_pclParentCategoryID; }
		virtual CDB_MultiString *GetMultiStringSubCategory( void ) { return m_pclParentSubCategory; }
		virtual CDB_StringID *GetStringTypeID( void ) { return m_pclTypeID; }
		CDB_StringID *m_pclTypeID;
		CDB_StringID *m_pclParentCategoryID;
		CDB_MultiString *m_pclParentSubCategory;
	};

	class CTreeItemFamily : public CTreeItemData
	{
	public:
		CTreeItemFamily() : CTreeItemData( CTreeItemData::DataType::Family ) 
			{	m_pclFamilyID = NULL; m_pclParentTypeID = NULL; m_pclParentSubCategory = NULL; m_pclParentCategoryID = NULL; }
		CTreeItemFamily( CDB_StringID *pclFamilyID, CTreeItemType *pclTreeItemType ) : CTreeItemData( CTreeItemData::DataType::Family ) 
			{	m_pclFamilyID = pclFamilyID; m_pclParentTypeID = pclTreeItemType->m_pclTypeID; m_pclParentSubCategory = pclTreeItemType->m_pclParentSubCategory; 
				m_pclParentCategoryID = pclTreeItemType->m_pclParentCategoryID; }
		virtual ~CTreeItemFamily() {}
		virtual CDB_StringID *GetStringCategoryID( void ) { return m_pclParentCategoryID; }
		virtual CDB_MultiString *GetMultiStringSubCategory( void ) { return m_pclParentSubCategory; }
		virtual CDB_StringID *GetStringTypeID( void ) { return m_pclParentTypeID; }
		virtual CDB_StringID *GetStringFamilyID( void ) { return m_pclFamilyID; }
		CDB_StringID *m_pclParentCategoryID;
		CDB_MultiString *m_pclParentSubCategory;
		CDB_StringID *m_pclParentTypeID;
		CDB_StringID *m_pclFamilyID;
	};

	class CTreeItemProduct : public CTreeItemData
	{
	public:
		CTreeItemProduct() : CTreeItemData( CTreeItemData::DataType::Product ) 
			{	m_pclProduct = NULL; m_pclParentFamilyID = NULL; m_pclParentTypeID = NULL; m_pclParentSubCategory = NULL; m_pclParentCategoryID = NULL; }
		CTreeItemProduct( CDB_Product *pclProduct, CTreeItemFamily *pclTreeItemFamily ) : CTreeItemData( CTreeItemData::DataType::Product ) 
			{	m_pclProduct = pclProduct; m_pclParentFamilyID = pclTreeItemFamily->m_pclFamilyID; m_pclParentTypeID = pclTreeItemFamily->m_pclParentTypeID; 
				m_pclParentSubCategory = pclTreeItemFamily->m_pclParentSubCategory; m_pclParentCategoryID = pclTreeItemFamily->m_pclParentCategoryID; }
		virtual ~CTreeItemProduct() {}
		virtual CDB_StringID *GetStringCategoryID( void ) { return m_pclParentCategoryID; }
		virtual CDB_MultiString *GetMultiStringSubCategory( void ) { return m_pclParentSubCategory; }
		virtual CDB_StringID *GetStringTypeID( void ) { return m_pclParentTypeID; }
		virtual CDB_StringID *GetStringFamilyID( void ) { return m_pclParentFamilyID; }
		CDB_Product *m_pclProduct;
		CDB_StringID *m_pclParentCategoryID;
		CDB_MultiString *m_pclParentSubCategory;
		CDB_StringID *m_pclParentTypeID;
		CDB_StringID *m_pclParentFamilyID;
	};

	class CTreeItemAdapter : public CTreeItemData
	{
	public:
		CTreeItemAdapter() : CTreeItemData( CTreeItemData::DataType::Adapter )
			{	m_pclAdapter = NULL; m_pclParentFamilyID = NULL; m_pclParentTypeID = NULL; m_pclParentSubCategory = NULL; m_pclParentCategoryID = NULL; }
		
		CTreeItemAdapter( CDB_Product *pclAdapter, CTreeItemFamily *pclTreeItemFamily ) : CTreeItemData( CTreeItemData::DataType::Adapter )
			{	m_pclAdapter = pclAdapter; m_pclParentFamilyID = pclTreeItemFamily->m_pclFamilyID; m_pclParentTypeID = pclTreeItemFamily->m_pclParentTypeID; 
				m_pclParentSubCategory = pclTreeItemFamily->m_pclParentSubCategory; m_pclParentCategoryID = pclTreeItemFamily->m_pclParentCategoryID; }
		
		virtual ~CTreeItemAdapter() {}
		virtual CDB_StringID *GetStringCategoryID( void ) { return m_pclParentCategoryID; }
		virtual CDB_MultiString *GetMultiStringSubCategory( void ) { return m_pclParentSubCategory; }
		virtual CDB_StringID *GetStringTypeID( void ) { return m_pclParentTypeID; }
		virtual CDB_StringID *GetStringFamilyID( void ) { return m_pclParentFamilyID; }
		CDB_Product *m_pclAdapter;
		CDB_StringID *m_pclParentCategoryID;
		CDB_MultiString *m_pclParentSubCategory;
		CDB_StringID *m_pclParentTypeID;
		CDB_StringID *m_pclParentFamilyID;
	};

	class CTreeItemActuator : public CTreeItemData
	{
	public:
		CTreeItemActuator() : CTreeItemData( CTreeItemData::DataType::Actuator )
			{	m_pclActuator = NULL; m_pclProduct = NULL; m_pclParentFamilyID = NULL; m_pclParentTypeID = NULL; 
				m_pclParentSubCategory = NULL; m_pclParentCategoryID = NULL; }

		CTreeItemActuator( CDB_Actuator *pclActuator, CDB_Product *pclProduct, CTreeItemFamily *pclTreeItemFamily ) 
				: CTreeItemData( CTreeItemData::DataType::Actuator ) 
			{	m_pclActuator = pclActuator; m_pclProduct = pclProduct; m_pclParentFamilyID = pclTreeItemFamily->m_pclFamilyID; 
				m_pclParentTypeID = pclTreeItemFamily->m_pclParentTypeID; m_pclParentSubCategory = pclTreeItemFamily->m_pclParentSubCategory; 
				m_pclParentCategoryID = pclTreeItemFamily->m_pclParentCategoryID; }

		virtual ~CTreeItemActuator() {}
		virtual CDB_StringID *etStringCategoryID( void ) { return m_pclParentCategoryID; }
		virtual CDB_MultiString *etMultiStringSubCategory( void ) { return m_pclParentSubCategory; }
		virtual CDB_StringID *etStringTypeID( void ) { return m_pclParentTypeID; }
		virtual CDB_StringID *etStringFamilyID( void ) { return m_pclParentFamilyID; }
		CDB_Actuator *m_pclActuator;
		CDB_Product *m_pclProduct;
		CDB_StringID *m_pclParentCategoryID;
		CDB_MultiString *m_pclParentSubCategory;
		CDB_StringID *m_pclParentTypeID;
		CDB_StringID *m_pclParentFamilyID;
	};
	
	class CTreeItemAccessory : public CTreeItemData
	{
	public:
		CTreeItemAccessory() 
			: CTreeItemData( CTreeItemData::DataType::Accessory ) 
			{	m_pclAccessory = NULL; m_eAccessoryType = CAccessoryList::AccessoryType::_AT_Undefined; m_pclRuledTable = NULL; 
				m_pclParentFamilyID = NULL; m_pclParentTypeID = NULL; m_pclParentSubCategory = NULL; m_pclParentCategoryID = NULL; m_pclParent = NULL; }

		CTreeItemAccessory( CDB_Product *pclAccessory, CAccessoryList::AccessoryType eAccessoryType, CDB_RuledTable *pclRuledTable, CTreeItemFamily *pclTreeItemFamily, CDB_Product *pclParent = NULL ) 
			: CTreeItemData( CTreeItemData::DataType::Accessory )
			{	m_pclAccessory = pclAccessory; m_eAccessoryType = eAccessoryType; m_pclRuledTable = pclRuledTable; m_pclParentFamilyID = pclTreeItemFamily->m_pclFamilyID; 
				m_pclParentTypeID = pclTreeItemFamily->m_pclParentTypeID; m_pclParentSubCategory = pclTreeItemFamily->m_pclParentSubCategory; 
				m_pclParentCategoryID = pclTreeItemFamily->m_pclParentCategoryID; m_pclParent = pclParent; }
		
		virtual ~CTreeItemAccessory() {}
		virtual CDB_StringID *GetStringCategoryID( void ) { return m_pclParentCategoryID; }
		virtual CDB_MultiString *GetMultiStringSubCategory( void ) { return m_pclParentSubCategory; }
		virtual CDB_StringID *GetStringTypeID( void ) { return m_pclParentTypeID; }
		virtual CDB_StringID *GetStringFamilyID( void ) { return m_pclParentFamilyID; }
		CDB_Product *m_pclAccessory;
		CDB_RuledTable *m_pclRuledTable;
		CAccessoryList::AccessoryType m_eAccessoryType;
		CDB_StringID *m_pclParentCategoryID;
		CDB_MultiString *m_pclParentSubCategory;
		CDB_StringID *m_pclParentTypeID;
		CDB_StringID *m_pclParentFamilyID;
		CDB_Product *m_pclParent;	// HYS-1618/HYS-1619: Allow to know at which product belongs the accessory.
	};
	
	// HYS-945: Buffer vessel management
	class CTreeItemIntegrated : public CTreeItemData
	{
	public:
		CTreeItemIntegrated() : CTreeItemData( CTreeItemData::DataType::Integrated )
		{
			m_pclIntegrated = NULL; m_pclParentFamilyID = NULL; m_pclParentTypeID = NULL; m_pclParentSubCategory = NULL; m_pclParentCategoryID = NULL;
		}
		
		CTreeItemIntegrated( CDB_Vessel *pclIntegrated, CTreeItemFamily *pclTreeItemFamily ) : CTreeItemData( CTreeItemData::DataType::Integrated )
		{
			m_pclIntegrated = pclIntegrated; m_pclParentFamilyID = pclTreeItemFamily->m_pclFamilyID; m_pclParentTypeID = pclTreeItemFamily->m_pclParentTypeID;
			m_pclParentSubCategory = pclTreeItemFamily->m_pclParentSubCategory; m_pclParentCategoryID = pclTreeItemFamily->m_pclParentCategoryID;
		}
		
		virtual ~CTreeItemIntegrated() {}
		virtual CDB_StringID *GetStringCategoryID( void ) { return m_pclParentCategoryID; }
		virtual CDB_MultiString *GetMultiStringSubCategory( void ) { return m_pclParentSubCategory; }
		virtual CDB_StringID *GetStringTypeID( void ) { return m_pclParentTypeID; }
		virtual CDB_StringID *GetStringFamilyID( void ) { return m_pclParentFamilyID; }
		CDB_Vessel *m_pclIntegrated;
		CDB_StringID *m_pclParentCategoryID;
		CDB_MultiString *m_pclParentSubCategory;
		CDB_StringID *m_pclParentTypeID;
		CDB_StringID *m_pclParentFamilyID;
	};

	// Allow to keep list of all accessories selected.
	class CAccessorySelected
	{
	public:
		CAccessorySelected() { Clear(); }
		void Clear() { m_pclAccessory = NULL; m_pclLinkToAccessory = NULL; m_eAccessoryType = CAccessoryList::AccessoryType::_AT_Undefined; m_pclRuledTable = NULL; m_hTreeItem = NULL; m_bEnable = true; m_bCheck = false; }
		
		CDB_Product *m_pclAccessory;
		CDB_Product * m_pclLinkToAccessory;		// Keep in memory on which this accessory is linked (in case of implicated/excluded relation).
		CAccessoryList::AccessoryType m_eAccessoryType;
		CDB_RuledTable *m_pclRuledTable;
		HTREEITEM m_hTreeItem;
		bool m_bEnable;
		bool m_bCheck;
	};

	CDirectSelectionParameters m_clDirectSelectionParams;
	CImageList m_TreeProdImgList;
	CExtTreeCtrl m_TreeProdCateg;
	CExtTreeCtrl m_TreeProducts;
	CExtStatic m_staticImg;
	CEnBitmap m_Bmp;
	bool m_bWorkingForDpCSet;
	bool m_bWorkingForControlSet;
	bool m_bSkipTvnSelChangedProduct;
	CExtListCtrl m_List_ArticleDetails;
	CExtListCtrl m_ListBoxFocus;
	HTREEITEM m_hParentItem;
	CEdit m_EditSearchProduct;
	CComboBox m_ComboResults;
	CMyToolTipCtrl m_ToolTip;
	CButton	m_CBDeletedValves;
	CButton m_btnSearch;
	CButton m_btnOK;
	
	// Notify the selected object.
	sSelectedItem m_ItemProductSelected;
	sSelectedItem m_ItemAdapterSelected;
	sSelectedItem m_ItemActuatorSelected;
	sSelectedItem m_ItemIntegratedSelected;

	std::vector<CTreeItemData *> m_vecItemDataInTreeList;

	// Will contain all accessories that are selected by the user AND!! those that are included, excluded or built-in.
	// WHY EXCLUDED? Because, when user clicks on an other accessory, we eventually disable check box for other accessories that are
	// excluded. If user clicks on other accessory, we must be able to re enable previous checkbox that has been disabled. This is the
	// reason why we keep also these accessories.
	std::vector<CAccessorySelected> m_vecProductAccessorySelected;
	std::vector<CAccessorySelected> m_vecSubProductAccessorySelected;
	std::vector<CAccessorySelected> m_vecActuatorAccessorySelected;
	std::vector<CAccessorySelected> m_vecIntegratedAccessorySelected;
};


class CDlgDirSelExport : public CDialogEx
{
	DECLARE_DYNAMIC( CDlgDirSelExport )

	enum { IDD = IDD_DLGDIRSELEXPORT };

public:
	CDlgDirSelExport( CWnd* pParent = NULL );
	virtual ~CDlgDirSelExport() {}
	bool IsExportAll( void ) { return m_fExportAll;  }

protected:
	DECLARE_MESSAGE_MAP()
	afx_msg void OnBnClickedAll();
	afx_msg void OnBnClickedAvailable();

private:
	bool m_fExportAll;
};
