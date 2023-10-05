#include "stdafx.h"
#include "TASelect.h"
#include "SelProdArticle.h"


CArticleItem::CArticleItem( CArticleItem *pclArticleItem )
{
	CopyFrom( pclArticleItem );
}

void CArticleItem::Init()
{
	m_strID.Empty();
	m_strArticle.Empty();
	m_strLocArtNum.Empty();
	m_strDescription.Empty();
	m_dPrice = 0.0;
	m_iQuantity = 0;
	m_bIsAvailable = true;
	m_bIsDeleted = false;
	m_bIsAttached = false;
	m_bIsSelectedInSet = false;
}

void CArticleItem::Init( CDB_Thing *pThing, int iQuantity, CString strDefinitionAdditionalInfo )
{
	Init();

	CDB_Product *pclProduct = dynamic_cast<CDB_Product *>( pThing );
	ASSERT( NULL != pclProduct );

	if( NULL != pclProduct )
	{
		m_strID = pclProduct->GetIDPtr().ID;
		m_strArticle = pclProduct->GetBodyArtNum();

		// Check if article is available and not deleted.
		m_strLocArtNum = LocArtNumTab.GetLocalArticleNumber( m_strArticle );
		CheckThingAvailability( pclProduct, m_strArticle, m_strLocArtNum );

		if( true == m_strLocArtNum.IsEmpty() )
		{
			m_strLocArtNum = _T("-");
		}

		m_strDescription =	pclProduct->GetName();

		if( false == strDefinitionAdditionalInfo.IsEmpty() )
		{
			m_strDescription += strDefinitionAdditionalInfo;
		}

		if( NULL != pclProduct->GetVersionIDPtr().MP )
		{
			m_strDescription += CString( _T("; ") ) + ( (CDB_StringID *)( pclProduct->GetVersionIDPtr().MP ) )->GetString();
		}

		CString str = _T("");

		if( NULL != dynamic_cast<CDB_TAProduct *>( pclProduct ) )
		{
			CDB_TAProduct *pclTAProduct = dynamic_cast<CDB_TAProduct *>( pclProduct );

			if( NULL != pclTAProduct->GetConnectIDPtr().MP )
			{
				str = ( (CDB_StringID *)( pclTAProduct->GetConnectIDPtr().MP ) )->GetString();
				str = CString( _T("; ") ) + str.Right( str.GetLength() - str.Find( '/' ) - 1 );
			}
		}

		m_strDescription += str;

		if( NULL != pclProduct->GetPNIDPtr().MP )
		{
			m_strDescription += CString( _T("; " ) ) + TASApp.LoadLocalizedString( IDS_SSHEETSELPROD_PN ) + CString( _T( " ") );
			m_strDescription += pclProduct->GetPN().c_str();
		}

		m_dPrice = TASApp.GetpTADB()->GetPrice( pclProduct->GetArtNum( true ) );
		m_iQuantity = iQuantity;
	}
}

void CArticleItem::CopyFrom( CArticleItem *pclArticleItem )
{
	if( NULL == pclArticleItem )
	{
		return;
	}

	m_strID = pclArticleItem->GetID();
	m_strArticle = pclArticleItem->GetArticle();
	m_strLocArtNum = pclArticleItem->GetLocArtNum();
	m_strDescription = pclArticleItem->GetDescription();
	m_dPrice = pclArticleItem->GetPrice();
	m_iQuantity = pclArticleItem->GetQuantity();
	m_bIsAvailable = pclArticleItem->GetIsAvailable();
	m_bIsDeleted = pclArticleItem->GetIsDeleted();
}

void CArticleItem::CheckThingAvailability( CDB_Thing *pThing, CString &strArticle, CString &strLocalArticle )
{
	if( NULL == pThing )
	{
		return;
	}

	m_bIsAvailable = pThing->IsAvailable();
	m_bIsDeleted = pThing->IsDeleted();

	if( false == m_bIsAvailable || true == m_bIsDeleted )
	{
		CString str;

		if( false == strArticle.IsEmpty() && strArticle.GetAt( 0 ) != _T('*') )
		{
			strArticle = _T("* ") + strArticle + _T(" *");
		}

		if( false == strLocalArticle.IsEmpty() && strLocalArticle.GetAt( 0 ) != _T('*') )
		{
			strLocalArticle = _T("* ") + strLocalArticle + _T(" *");
		}
		
		if( true == m_bIsDeleted )
		{
			str = TASApp.LoadLocalizedString( IDS_TAPDELETED );
			strArticle += _T(" ") + str;

			if( false == strLocalArticle.IsEmpty() )
			{
				strLocalArticle += _T(" ") + str;
			}
		}
		else if( false == m_bIsAvailable )
		{
			str = TASApp.LoadLocalizedString( IDS_TAPNOTAVAILABLE );
			strArticle += _T(" ") + str;

			if( false == strLocalArticle.IsEmpty() )
			{
				strLocalArticle += _T(" ") + str;
			}
		}
	}	
}

/////////////////////////////////////////////////////////////////////////////////////////////////////////////
// CArticleContainer
/////////////////////////////////////////////////////////////////////////////////////////////////////////////
CArticleContainer::CArticleContainer()
{
	m_clArticle.Init();
}

CArticleContainer::CArticleContainer( CArticleContainer *pclArticleContainer )
{
	m_clArticle.Init();

	if( NULL != pclArticleContainer )
	{
		// Copy article and sub articles if exist.
		CopyFrom( pclArticleContainer );
	}
}

CArticleContainer::CArticleContainer( CArticleItem *pclArticleItem )
{
	m_clArticle.CopyFrom( pclArticleItem );
}

CArticleContainer::CArticleContainer( CDB_Thing *pThing, int iQuantity, CString strDefinitionAdditionalInfo )
{
	m_clArticle.Init( pThing, iQuantity, strDefinitionAdditionalInfo );
}

CArticleContainer::~CArticleContainer()
{
	for( vecArticleListIter vecIter = m_vecAccessoryList.begin(); vecIter != m_vecAccessoryList.end(); ++vecIter )
	{
		if( NULL != *vecIter )
		{
			delete *vecIter;
		}
	}

	m_vecAccessoryList.clear();
}

void CArticleContainer::CopyFrom( CArticleContainer *pclArticleContainer )
{
	if( NULL == pclArticleContainer )
	{
		return;
	}

	m_clArticle.CopyFrom( pclArticleContainer->GetArticleItem() );

	vecArticleList *pvecAccessoryListToCopy = pclArticleContainer->GetAccessoryList();

	if( NULL == pvecAccessoryListToCopy || 0 == (int)pvecAccessoryListToCopy->size() )
	{
		return;
	}

	vecArticleListIter iter = pvecAccessoryListToCopy->begin();

	while( iter != pvecAccessoryListToCopy->end() )
	{
		CArticleItem* pclArticleItem = new CArticleItem();
		
		if( NULL == pclArticleItem )
		{
			continue;
		}

		pclArticleItem->CopyFrom( *iter );
		AddAccessory( pclArticleItem );
	
		iter++;
	}
}

void CArticleContainer::AddAccessory( CArticleItem *pclArticleItem )
{
	if( NULL == pclArticleItem )
	{
		return;
	}

	bool bExist = false;

	for( vecArticleListIter vecIter = m_vecAccessoryList.begin(); vecIter != m_vecAccessoryList.end() && false == bExist; ++vecIter )
	{
		CArticleItem *pclLoopSubArticle = *vecIter;

		// If sub article already exists, we just increment quantity.
		if( pclLoopSubArticle->GetID() == pclArticleItem->GetID() )
		{
			bExist = true;
			pclLoopSubArticle->SetQuantity( pclLoopSubArticle->GetQuantity() + 1 );
		}
	}

	// Add sub article if not exist in the list.
	if( false == bExist )
	{
		m_vecAccessoryList.push_back( pclArticleItem );
	}
}

CArticleItem *CArticleContainer::GetFirstAccessory()
{
	CArticleItem *pclSubArticleItem = NULL;
	
	if( m_vecAccessoryList.size() > 0 )
	{
		m_vecAccessoryListIter = m_vecAccessoryList.begin();
		pclSubArticleItem = *m_vecAccessoryListIter;
		++m_vecAccessoryListIter;
	}

	return pclSubArticleItem;
}

CArticleItem *CArticleContainer::GetNextAccessory()
{
	CArticleItem *pclSubArticleItem = NULL;
	
	if( m_vecAccessoryListIter != m_vecAccessoryList.end() )
	{
		pclSubArticleItem = *m_vecAccessoryListIter;
		++m_vecAccessoryListIter;
	}

	return pclSubArticleItem;
}

/////////////////////////////////////////////////////////////////////////////////////////////////////////////
// CArticleGroup
/////////////////////////////////////////////////////////////////////////////////////////////////////////////
CArticleGroup::CArticleGroup()
{
	m_bMergeSameArticle = true;
	m_vecArticleContainerListIter = m_vecArticleContainerList.begin();
}

CArticleGroup::CArticleGroup( CArticleGroup *pclArticleGroup )
{
	if( NULL != pclArticleGroup )
	{
		// Copy article and sub articles if exist.
		CopyFrom( pclArticleGroup );
	}
}

CArticleGroup::~CArticleGroup()
{
	Clean();
}

void CArticleGroup::Clean()
{
	if( m_vecArticleContainerList.size() > 0 )
	{
		for( m_vecArticleContainerListIter = m_vecArticleContainerList.begin(); m_vecArticleContainerListIter != m_vecArticleContainerList.end(); ++m_vecArticleContainerListIter )
		{
			if( NULL != *m_vecArticleContainerListIter )
			{
				delete *m_vecArticleContainerListIter;
			}
		}
		m_vecArticleContainerList.clear();
	}
}

void CArticleGroup::CopyFrom( CArticleGroup *pclArticleGroup )
{
	if( NULL == pclArticleGroup )
	{
		return;
	}

	m_bMergeSameArticle = pclArticleGroup->GetMergeSameArticleFlag();

	if( 0 != pclArticleGroup->GetArticleContainerCount() ) 
	{
		CArticleContainer *pclArticleContainer = pclArticleGroup->GetFirstArticleContainer();
		
		while( NULL != pclArticleContainer )
		{
			CArticleContainer *pclArticleContainerCopy = new CArticleContainer( pclArticleContainer );

			if( NULL != pclArticleContainerCopy )
			{
				m_vecArticleContainerList.push_back( pclArticleContainerCopy );
			}

			pclArticleContainer = pclArticleGroup->GetNextArticleContainer();
		}
	}
}

CArticleContainer *CArticleGroup::AddArticle( CArticleItem *pclArticleItem, bool fFirstArticle )
{
	if( NULL == pclArticleItem )
	{
		return NULL;
	}

	CArticleContainer *pclArticleContainer = new CArticleContainer( pclArticleItem );

	if( NULL == pclArticleContainer )
	{
		return NULL;
	}

	if( false == fFirstArticle || 0 == (int)m_vecArticleContainerList.size() )
	{
		m_vecArticleContainerList.push_back( pclArticleContainer );
	}
	else
	{
		m_vecArticleContainerList.insert( m_vecArticleContainerList.begin(), pclArticleContainer );
	}

	return pclArticleContainer;
}

CArticleContainer *CArticleGroup::AddArticle( CDB_Thing *pThing, int iQuantity, bool bFirstArticle, CString strDefinitionAdditionalInfo )
{
	if( NULL == pThing || 0 == iQuantity )
	{
		return NULL;
	}

	CArticleContainer *pclArticleContainer = new CArticleContainer( pThing, iQuantity, strDefinitionAdditionalInfo );

	if( NULL == pclArticleContainer )
	{
		return NULL;
	}

	if( false == bFirstArticle || 0 == (int)m_vecArticleContainerList.size() )
	{
		m_vecArticleContainerList.push_back( pclArticleContainer );
	}
	else
	{
		m_vecArticleContainerList.insert( m_vecArticleContainerList.begin(), pclArticleContainer );
	}

	return pclArticleContainer;
}

void CArticleGroup::AddAccessory( CArticleContainer *pclArticleContainer, CArticleItem *pclAcessory )
{
	if( NULL == pclArticleContainer || NULL == pclAcessory )
	{
		return;
	}

	pclArticleContainer->AddAccessory( pclAcessory );
}

CArticleContainer *CArticleGroup::GetFirstArticleContainer()
{
	CArticleContainer *pclArticleContainer = NULL;

	if( m_vecArticleContainerList.size() > 0 )
	{
		m_vecArticleContainerListIter = m_vecArticleContainerList.begin();
		pclArticleContainer = *m_vecArticleContainerListIter;
		++m_vecArticleContainerListIter;
	}

	return pclArticleContainer;
}

CArticleContainer *CArticleGroup::GetNextArticleContainer()
{
	CArticleContainer *pclArticleContainer = NULL;

	if( m_vecArticleContainerListIter != m_vecArticleContainerList.end() )
	{
		pclArticleContainer = *m_vecArticleContainerListIter;
		++m_vecArticleContainerListIter;
	}

	return pclArticleContainer;
}

/////////////////////////////////////////////////////////////////////////////////////////////////////////////
// CArticleList
/////////////////////////////////////////////////////////////////////////////////////////////////////////////
CArticleGroupList::CArticleGroupList()
{
	m_vecArticleGroupListIter = m_vecArticleGroupList.begin();
}

CArticleGroupList::~CArticleGroupList()
{
	Clean();
}

void CArticleGroupList::Clean()
{
	if( m_vecArticleGroupList.size() > 0 )
	{
		for( m_vecArticleGroupListIter = m_vecArticleGroupList.begin(); m_vecArticleGroupListIter != m_vecArticleGroupList.end(); ++m_vecArticleGroupListIter )
		{
			if( NULL != *m_vecArticleGroupListIter )
			{
				delete *m_vecArticleGroupListIter;
			}
		}
		m_vecArticleGroupList.clear();
	}
}

void CArticleGroupList::AddArticleGroup( CArticleGroup *pclArticleGroup )
{
	if( NULL != pclArticleGroup )
	{
		m_vecArticleGroupList.push_back( pclArticleGroup );
	}
}

CArticleGroup *CArticleGroupList::GetArticleGroupAt( long lPos )
{
	CArticleGroup *pclArticleGroup = NULL;

	if( lPos < (long)m_vecArticleGroupList.size() )
	{
		pclArticleGroup = m_vecArticleGroupList.at( lPos );
	}

	return pclArticleGroup;
}

CArticleGroup *CArticleGroupList::GetFirstArticleGroup()
{
	CArticleGroup *pclArticleGroup = NULL;

	if( m_vecArticleGroupList.size() > 0 )
	{
		m_vecArticleGroupListIter = m_vecArticleGroupList.begin();
		pclArticleGroup = *m_vecArticleGroupListIter;
		++m_vecArticleGroupListIter;
	}

	return pclArticleGroup;
}

CArticleGroup *CArticleGroupList::GetNextArticleGroup()
{
	CArticleGroup *pclArticleGroup = NULL;

	if( m_vecArticleGroupListIter != m_vecArticleGroupList.end() )
	{
		pclArticleGroup = *m_vecArticleGroupListIter;
		++m_vecArticleGroupListIter;
	}

	return pclArticleGroup;
}