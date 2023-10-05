#pragma once


#include <map>
#include <vector>
#include "Utilities.h"
#include "ExtComboBox.h"

#define RESET_ONECOMBOCONTENT( Array, SizeRange, WhichCombo ) { Array[SizeRange][WhichCombo].clear(); }
#define NEW_COMBO_ITEMDATA( Array, SizeRange, WhichCombo, Index, Type ) { ItemData rItemData; rItemData.m_iItemType = Type; Array[SizeRange][WhichCombo][Index] = rItemData; }
#define ADD_COMBO_ITEMDATA_VECSTRID( Array, SizeRange, WhichCombo, Index, Vector ) { Array[SizeRange][WhichCombo][Index].m_vecCStrID.push_back( Vector ); }

class CDlgSelComboHelperBase
{
public:
	typedef struct _CStringID
	{
		CString m_str;
		IDPTR m_IDPtr;
		int m_int;
		short m_nID;
	}CStringID;
	typedef std::vector<CStringID> vecCStrID;
	typedef vecCStrID::iterator vecCStrIDIter;

	typedef std::map<short, vecCStrID> mapShortvecCStrID;
	typedef mapShortvecCStrID::iterator mapShortvecCStrIDIter;
	typedef std::pair<short, vecCStrID> pairShortvecCStrID;

	typedef std::vector<int> vecInt;
	typedef vecInt::iterator vecIntIter;

	typedef enum
	{
		CS_First,
		CS_All = CS_First,
		CS_Below65,
		CS_Above50,
		CS_Last
	}ComboSize;

	typedef enum
	{
		WC_First,
		WC_Type = WC_First,
		WC_CtrlType,
		WC_Family,
		WC_BodyMaterial,
		WC_Connect,
		WC_Version,
		WC_PN,
		WC_Last
	}WhichCombo;

	typedef enum
	{
		SP_PICV,
		SP_DPCBCV
	}SelectionProduct;

	typedef enum
	{
		SelType_Individual,
		SelType_Batch,
		SelType_Last
	}SelectionType;

	// 'IDT' is for 'Item Data Type'.
	enum
	{
		IDT_All,
		IDT_Specific
	};

	typedef struct _ItemData
	{
		int m_iItemType;
		vecCStrID m_vecCStrID;
	}ItemData;

	typedef std::map<short, ItemData> mapShortItemData;
	typedef mapShortItemData::iterator mapShortItemDataIter;

	CDlgSelComboHelperBase( CProductSelelectionParameters *pclProductSelParams, SelectionProduct eSelectionProduct, SelectionType eSelectionType );
	virtual ~CDlgSelComboHelperBase() {}

	void PrepareComboContents( void );

	void FillComboType( CExtNumEditComboBox *pCombo, ComboSize eComboSize, CString strTypeID = _T("") );
	void FillComboCtrlType( CExtNumEditComboBox *pCombo, ComboSize eComboSize, CDB_ControlProperties::CvCtrlType eCvCtrlType = CDB_ControlProperties::CvCtrlType::eLastCvCtrlType );
	void FillComboFamily( CExtNumEditComboBox *pCombo, ComboSize eComboSize, CString strFamilyID = _T("") );
	void FillComboBodyMaterial( CExtNumEditComboBox *pCombo, ComboSize eComboSize, CString strBodyMatID = _T("") );
	void FillComboConnect( CExtNumEditComboBox *pCombo, ComboSize eComboSize, CString strConnectID = _T("") );
	void FillComboVersion( CExtNumEditComboBox *pCombo, ComboSize eComboSize, CString strVersionID = _T("") );
	void FillComboPN( CExtNumEditComboBox *pCombo, ComboSize eComboSize, CString strPNID = _T("") );

	CString GetIDFromCombo( CExtNumEditComboBox *pclCombo, int iSelection = -1 );
	IDPTR GetIDPtrFromCombo( CExtNumEditComboBox *pclCombo, int iSelection = -1 );
	int GetIntFromCombo( CExtNumEditComboBox *pclCombo, int iSelection = -1 );

	int FindItemDataID( CExtNumEditComboBox *pclCombo, CString strValueID );
	int FindItemDataInt( CExtNumEditComboBox *pclCombo, int iValue );

	void OnCbnSelChange( CExtNumEditComboBox *pclCombo );

	CStringID *IfStrExist( vecCStrID *pvecIDPtrID, CString str );

// Protected methods.
protected:
	void PrepareVecList( CExtNumEditComboBox *pCurrentCombo, vecInt *pvecToFill );
	void FillCombo( CExtNumEditComboBox *pCurrentCombo, CRankEx *pList, short nToSelect, int iAllIds = 0 );

// Protected variables.
protected:
	CProductSelelectionParameters *m_pclProductSelParams;
	SelectionProduct m_eSelectionProduct;
	SelectionType m_eSelectionType;
	int m_iDNAbove50;
	int m_iDNBelow65;

	std::vector<int> m_vecComboOrder;
	mapShortItemData m_armapCurrentStrID[2][WC_Last];
	short m_arnCurrentChoice[2][WC_Last];
	std::map<CExtNumEditComboBox*, WhichCombo> m_mapComboPtrToEComboType;
	std::map<CExtNumEditComboBox*, ComboSize> m_mapComboPtrToEComboSize;

	typedef struct
	{
		mapShortvecCStrID m_mapPN;
		mapShortvecCStrID m_mapVersion;
		mapShortvecCStrID m_mapConnect;
		mapShortvecCStrID m_mapBodyMat;
		mapShortvecCStrID m_mapFamily;
		mapShortvecCStrID m_mapCtrlType;
		mapShortvecCStrID m_mapTypes;
	}mapContainer;

	mapContainer m_mapAllCombos[ CS_Last ];

// private methods.
private:
	void _PrepareComboContentsForControl( void );
	void _InsertContent( CDB_ControlProperties::CvCtrlType CvCtrlType, CDB_RegulatingValve *pclValve, int iInsertIn );

	int _GetPrevComboType( WhichCombo eWhichCombo );
	int _GetNextComboType( WhichCombo eWhichCombo );
	ItemData *_GetItemData( CExtNumEditComboBox *pclCombo, int iSelection = -1 );
};

class CDlgSelComboHelperPIBCV : public CDlgSelComboHelperBase
{
public:
	CDlgSelComboHelperPIBCV( CProductSelelectionParameters *pclProductSelParams, SelectionType eSelectionType );
	virtual ~CDlgSelComboHelperPIBCV() {}
};

class CDlgSelComboHelperDpCBCV : public CDlgSelComboHelperBase
{
public:
	CDlgSelComboHelperDpCBCV( CProductSelelectionParameters *pclProductSelParams, SelectionType eSelectionType );
	virtual ~CDlgSelComboHelperDpCBCV() {}
};
