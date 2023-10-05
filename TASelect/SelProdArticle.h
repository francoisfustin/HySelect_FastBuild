#pragma once

// How the articles are saved?
//
// Before the pressurisation maintenance selection, we had to save only one article and 0, 1 or more accessories linked to this article.
// With pressurisation, we can have more than one article for one selection. And when we display the result, we want to group the articles for 
// the same selection to be clearer.
//
// Each component of one selection (Article and accessories) is saved in a 'CArticleItem' class. For a selection we have to save the main article 
// and the list of its accessories. This is why we have the 'CArticleContainer' class. The main article is saved in a 'CArticleItem' class and the 
// list of its accessories are saved in a vector of 'CArticleItem'.
//
// Each set of article and its accessories (the 'CArticleContainer' class) are saved in the 'CArticleGroup' class.
//
// In the 'CArticleGroup' we have a vector of 'CArticleContainer'. The first 'CArticleContainer' (index 0) is always the main one. For a pressurisation
// maintenance selection for example, the first one can be a Transfero TV 4.1, the next one the Transfero Vessel, the next one a Pleno and so on.
//
// All 'CArticleGroup' are thus saved in the 'CArticleGroupList'.
//
// Let's take some examples.
//
/////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
//
// 1 selection: Product A + Accessory A1
//
// We have only one 'CArticleContainer' in the 'CArticleGroup'. And the 'CArticleContainer' contains the article and one accessory.
//
// CArticleGroupList: m_vecArticleGroupList[0] = CArticleGroup0
//  |
//  +-> CArticleGroup0: m_vecArticleContainerList[0] = CArticleContainer0
//       |
//       +-> CArticleContainer0 : m_clArticle = CArticleItem = Product A
//       |    | 
//       +-> CArticleContainer0 : m_vecAccessoryList[0] = CArticleItem0 = Accessory A1
//
/////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
//
// 1 selection: Product A + Accessory A1 + Accessory A2
//
// We have only one 'CArticleContainer' in the 'CArticleGroup'. And this 'CArticleContainer' contains the article and two accessories.
//
// CArticleGroupList: m_vecArticleGroupList[0] = CArticleGroup0
//  |
//  +-> CArticleGroup0: m_vecArticleContainerList[0] = CArticleContainer0
//       |
//       +-> CArticleContainer0 : m_clArticle = CArticleItem = Product A
//       |
//       +-> CArticleContainer0 : m_vecArticleList[0] = CArticleItem0 = Accessory A1
//       |
//       +-> CArticleContainer0 : m_vecArticleList[1] = CArticleItem1 = Accessory A2
//
/////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
//
// 1 selection: Product A + Accessory A1 + Accessory A2 + Product B + Accessory B1 + Product C + Accessory C1 + Accessory C2
//
// We have three 'CArticleContainer' in the 'CArticleGroup' for 'Product A', 'Product B' and 'Product C'.
//
// CArticleGroupList: m_vecArticleGroupList[0] = CArticleGroup0
//  |
//  +-> CArticleGroup0: m_vecArticleContainerList[0] = CArticleContainer0
//  |    |
//  |    +-> CArticleContainer0 : m_clArticle = CArticleItem = Product A
//  |    |
//  |    +-> CArticleContainer0 : m_vecArticleList[0] = CArticleItem0 = Accessory A1
//  |    |
//  |    +-> CArticleContainer0 : m_vecArticleList[1] = CArticleItem1 = Accessory A2
//  |
//  +-> CArticleGroup0: m_vecArticleContainerList[1] = CArticleContainer1
//  |    |
//  |    +-> CArticleContainer1 : m_clArticle = CArticleItem = Product B
//  |    |
//  |    +-> CArticleContainer1 : m_vecArticleList[0] = CArticleItem0 = Accessory B1
//  |
//  +-> CArticleGroup0: m_vecArticleContainerList[2] = CArticleContainer2
//       |
//       +-> CArticleContainer2 : m_clArticle = CArticleItem = Product C
//       |
//       +-> CArticleContainer2 : m_vecArticleList[0] = CArticleItem0 = Accessory C1
//       |
//       +-> CArticleContainer2 : m_vecArticleList[1] = CArticleItem1 = Accessory C2
//
/////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
//
// 2 selections: Product A + Accessory A1
//               Product B + Accessory B1
//
// We have only one 'CArticleContainer' by product (and one 'CArticleGroup' by line).
//
// CArticleGroupList: m_vecArticleGroupList[0] = CArticleGroup0
//  |
//  +-> CArticleGroup0: m_vecArticleContainerList[0] = CArticleContainer0
//       |
//       +-> CArticleContainer0 : m_clArticle = CArticleItem = Product A
//       |
//       +-> CArticleContainer0 : m_vecArticleList[0] = CArticleItem0 = Accessory A1
//
// CArticleGroupList: m_vecArticleGroupList[1] = CArticleGroup1
//  |
//  +-> CArticleGroup1: m_vecArticleContainerList[0] = CArticleContainer0
//       |
//       +-> CArticleContainer0 : m_clArticle = CArticleItem = Product B
//       |
//       +-> CArticleContainer0 : m_vecArticleList[0] = CArticleItem0 = Accessory B1
//
/////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

// Define one article.
class CArticleItem
{
public:
	CArticleItem() { Init(); }
	CArticleItem( CArticleItem *pclArticleItem );

	void Init();

	// 'strDefinitionAdditionalInfo' can be defined if you want to add additional info to the product description.
	// It's the case for example for Transfero TV where we add _T(" (Degassing/pump)"), the function name.
	void Init( CDB_Thing *pThing, int iQuantity, CString strDefinitionAdditionalInfo = _T("") );
	void CopyFrom( CArticleItem *pclArticleItem );
	void CheckThingAvailability( CDB_Thing *pThing, CString &strArticle, CString &strLocalArticle );

	// Setter
	void SetID( CString strID ) { m_strID = strID; }
	void SetArticle( CString strArticle ) { m_strArticle = strArticle; }
	void SetLocArtNum( CString strLocArtNum ) { m_strLocArtNum = strLocArtNum; }
	void SetDescription( CString strDescription ) { m_strDescription = strDescription; }
	void AddDescription( CString strDescription ) { m_strDescription += strDescription; }
	void SetPrice( double dPrice ) { m_dPrice = dPrice; }
	void SetQuantity( int iQuantity ) { m_iQuantity = iQuantity; }
	void SetIsAvailable( bool bIsAvailable ) { m_bIsAvailable = bIsAvailable; }
	void SetIsDeleted( bool bIsDeleted ) { m_bIsDeleted = bIsDeleted; }
	void SetIsAttached( bool bIsAttached ) { m_bIsAttached = bIsAttached; }
	void SetIsSelectedInSet( bool bIsSelectedInSet ) { m_bIsSelectedInSet = bIsSelectedInSet; }
	void AddQuantity( int iQuantity ) { m_iQuantity += iQuantity; }

	// Getter
	CString GetID( void ) { return m_strID; }
	CString GetArticle( void ) { return m_strArticle; }
	CString GetLocArtNum( void ) { return m_strLocArtNum; }
	CString GetDescription( void ) { return m_strDescription; }
	double GetPrice( void ) { return m_dPrice; }
	int GetQuantity( void ) { return m_iQuantity; }
	bool GetIsAvailable( void ) { return m_bIsAvailable; }
	bool GetIsDeleted( void ) { return m_bIsDeleted; }
	bool GetIsAttached( void ) { return m_bIsAttached; }
	bool GetIsSelectedInSet( void ) { return m_bIsSelectedInSet; }

private:
	CString m_strID;
	CString m_strArticle;
	CString m_strLocArtNum;
	CString m_strDescription;
	double m_dPrice;
	int m_iQuantity;
	bool m_bIsAvailable;
	bool m_bIsDeleted;
	bool m_bIsAttached;
	bool m_bIsSelectedInSet;
};

// Define a container for one article and some sub articles (usually accessories).
// Remark: for an article we can have some accessories linked to it (considered as article items).
//         It's the reason why we have 'm_clArticle' for the main article and 'm_clAccessoryList' that will
//         eventually contain list of these accessories.
class CArticleContainer
{
public:
	typedef std::vector<CArticleItem*> vecArticleList;
	typedef vecArticleList::iterator vecArticleListIter;

	CArticleContainer();
	CArticleContainer( CArticleContainer *pclArticleContainer );
	CArticleContainer( CArticleItem *pclArticleItem );

	// 'strDefinitionAdditionalInfo' can be defined if you want to add additional info to the product description.
	// It's the case for example for Transfero TV where we add _T(" (Degassing/pump)"), the function name.
	CArticleContainer( CDB_Thing *pThing, int iQuantity, CString strDefinitionAdditionalInfo = _T("") );

	virtual ~CArticleContainer();

	void CopyFrom( CArticleContainer *pclArticleContainer );

	CArticleItem *GetArticleItem( void ) { return &m_clArticle; }
	
	// For accessories.
	void AddAccessory( CArticleItem *pclArticleItem );
	int GetAccessoryCount( void ) { return (int)m_vecAccessoryList.size(); }
	CArticleItem *GetFirstAccessory( void );
	CArticleItem *GetNextAccessory( void );
	vecArticleList *GetAccessoryList( void ) { return &m_vecAccessoryList; }

// private variables.
private:
	CArticleItem m_clArticle;
	vecArticleList m_vecAccessoryList;
	vecArticleListIter m_vecAccessoryListIter;
};

// Define a single or a group of 'CArticleContainer'.
// 
// For a group of article container, we can have for example a TecBox Compresso for the main article (m_clMainArticle) 
// with its accessories (m_clMainAccessoryList). And linked to this article we can have a primary vessel, 
// an intermediate vessel, a Pleno or/and a Vento. Each of these are stocked independently in one 'CArticleContainer'.
class CArticleGroup
{
public:
	typedef std::vector<CArticleContainer*> vecArticleContainer;
	typedef vecArticleContainer::iterator vecArticleContainerIter;

	CArticleGroup();
	CArticleGroup( CArticleGroup *pclArticleGroup );
	virtual ~CArticleGroup();

	void Clean();
	void CopyFrom( CArticleGroup *pclArticleGroup );

	CArticleContainer *AddArticle( CArticleItem *pclArticleItem, bool fFirstArticle = false );
		
	// 'strDefinitionAdditionalInfo' can be defined if you want to add additional info to the product description.
	// It's the case for example for Transfero TV where we add _T(" (Degassing/pump)"), the function name.
	CArticleContainer *AddArticle( CDB_Thing *pThing, int iQuantity, bool fFirstArticle = false, CString strDefinitionAdditionalInfo = _T("") );

	void AddAccessory( CArticleContainer *pclArticleContainer, CArticleItem *pclAcessory );
	void SetMergeSameArticleFlag( bool fMerge ) { m_bMergeSameArticle = fMerge; }

	int GetArticleContainerCount( void ) { return (int)m_vecArticleContainerList.size(); }
	CArticleContainer *GetFirstArticleContainer( void );
	CArticleContainer *GetNextArticleContainer( void );
	bool GetMergeSameArticleFlag( void ) { return m_bMergeSameArticle; }

private:
	// 'true' (by default) if we can merge same 'CArticleGroup' in the 'CArticleGroupList'. If 'CArticleGroup' to insert 
	// in a 'CArticleGroupList' is already exist than we simply increment by 1.
	// Typically it's for pressure maintenance product selection where we set this variable to 'false'. It's to avoid to merge
	// 2 selections that have the main article (Tecbox or statico).
	bool m_bMergeSameArticle;

	vecArticleContainer m_vecArticleContainerList;
	vecArticleContainerIter m_vecArticleContainerListIter;
};

// Define list of all article groups.
class CArticleGroupList
{
public:
	typedef std::vector<CArticleGroup*> vecArticleGroup;
	typedef std::vector<CArticleGroup*>::iterator vecArticleGroupIter;

	CArticleGroupList();
	virtual ~CArticleGroupList();

	void Clean();

	void AddArticleGroup( CArticleGroup *pclArticleGroup );
	int GetArticleGroupCount( void ) { return (int)m_vecArticleGroupList.size(); }
	CArticleGroup *GetArticleGroupAt( long lPos );
	CArticleGroup *GetFirstArticleGroup( void );
	CArticleGroup *GetNextArticleGroup( void );

private:
	vecArticleGroup m_vecArticleGroupList;
	vecArticleGroupIter m_vecArticleGroupListIter;
};
