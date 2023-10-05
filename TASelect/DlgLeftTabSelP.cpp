#include "stdafx.h"
#include <windowsx.h>
#include <windows.h>

#include "TASelect.h"
#include "TASelectDoc.h"
#include "MainFrm.h"
#include "HMInclude.h"

#include "DlgPageField.h"
#include "SelProdPageBase.h"
#include "SelProdDockPane.h"
#include "RViewSelProd.h"
#include "DlgLeftTabBase.h"
#include "DlgLeftTabSelP.h"

#include "DlgExportConfigFile.h"

#ifdef _DEBUG
#define new DEBUG_NEW
#undef THIS_FILE
static char THIS_FILE[] = __FILE__;
#endif

CDlgLeftTabSelP *pDlgLeftTabSelP = NULL;

/////////////////////////////////////////////////////////////////////////////
// CDlgLeftTabSelP dialog

CDlgLeftTabSelP::CDlgLeftTabSelP( CWnd* pParent )
	: CDlgLeftTabBase( CMyMFCTabCtrl::TabIndex::etiSelP, CDlgLeftTabSelP::IDD, pParent )
{
	m_pTADB = NULL;
	m_pTADSPageSetup = NULL;
	m_pTADS = NULL;
	m_pUSERDB = NULL;
}

CDlgLeftTabSelP::~CDlgLeftTabSelP()
{
	pDlgLeftTabSelP = NULL;
}

void CDlgLeftTabSelP::OnLTTabCtrlEnterTab( CMyMFCTabCtrl::TabIndex eTabIndex, CMyMFCTabCtrl::TabIndex ePrevTabIndex )
{
	// Base class.
	CDlgLeftTabBase::OnLTTabCtrlEnterTab( eTabIndex, ePrevTabIndex );

	if( CMyMFCTabCtrl::TabIndex::etiSelP != eTabIndex )
	{
		return;
	}

	CMainFrame *pFrame = static_cast<CMainFrame*>( AfxGetMainWnd() );
	pFrame->ActiveFormView( CMainFrame::RightViewList::eRVSelProd );

	// Refresh tab title.
	CString str = TASApp.LoadLocalizedString( IDS_LEFTTABTITLE_RESULTS );

	if( NULL != pDlgLTtabctrl )
	{
		pDlgLTtabctrl->SetWindowText( str );
	}

	// Refresh the Hub Group.
	RefreshHubGroup();

	if( NULL != pRViewSelProd )
	{
		pRViewSelProd->OnEnterTab( ( CMyMFCTabCtrl::TabIndex::etiSSel == ePrevTabIndex ) ? true : false );
	}
}

void CDlgLeftTabSelP::OnLTTabCtrlLeaveTab( CMyMFCTabCtrl::TabIndex eTabIndex )
{
	// Base class.
	CDlgLeftTabBase::OnLTTabCtrlLeaveTab( eTabIndex );

	if( CMyMFCTabCtrl::TabIndex::etiSelP != eTabIndex )
	{
		return;
	}

	if( NULL != pRViewSelProd )
	{
		pRViewSelProd->OnLeaveTab();
	}
}

void CDlgLeftTabSelP::InitializeToolsDockablePaneContextID( void )
{
	CToolsDockablePane *pclToolsDockablePane = pMainFrame->GetpToolsDockablePane();

	if( NULL == pclToolsDockablePane )
	{
		return;
	}

	// Register context with the tools dockable pane for the 'Results' tab.
	CToolsDockablePane::ContextParameters *pclContext = pclToolsDockablePane->RegisterContext();

	if( NULL == pclContext )
	{
		return;
	}

	pclContext->m_bIsPaneVisible = false;
	pclContext->m_bIsPaneAutoHide = FALSE;
	pclContext->m_bIsGripperEnabled = FALSE;
	pclContext->m_bIsPaneFullScreen = false;
	pclContext->m_bIsContextMenuEnabled = true;
	pclContext->m_arIsTabVisible[CToolsDockablePane::TabID::OutputID_HMCompilation] = false;
	pclContext->m_arIsTabVisible[CToolsDockablePane::TabID::OutputID_BatchSelection] = false;
	pclContext->m_iLinkedRightView = CMainFrame::RightViewList::eRVSelProd;

	m_iContextID = pclContext->m_iContextID;
	m_bToolsDockablePaneContextInitialized = true;
}

bool CDlgLeftTabSelP::GetToolsDockablePaneContextID( int &iContextID  )
{
	if( false == CDlgLeftTabBase::GetToolsDockablePaneContextID( iContextID ) )
	{
		return false;
	}

	iContextID = m_iContextID;
	return ( -1 != m_iContextID ) ? true : false;
}

CString CDlgLeftTabSelP::GetScheme( int iIndex, CDS_SSelDpC **ppSelDpC )
{
	CTable*	pTab = (CTable*)( m_pTADS->Get( _T("DPCONTR_TAB") ).MP );
	ASSERT( NULL != pTab );

	if( false == m_pTADSPageSetup->GetField( epfDPCTECHINFOCONNECTSCHEME ) )
	{
		return _T("");
	}

	CDS_SSelDpC *pSelDpC = NULL;

	for( IDPTR IDPtr = pTab->GetFirst( CLASS( CDS_SSelDpC ) ); '\0' != *IDPtr.ID; IDPtr = pTab->GetNext() )
	{
		pSelDpC = (CDS_SSelDpC *)( IDPtr.MP );
		
		// Verify ID of DpC.
		if( NULL == pSelDpC->GetDpCIDPtr().MP )
		{
			continue;
		}
		
		if( iIndex == pSelDpC->GetSchemeIndex() )
		{
			if( true == pSelDpC->IsFromDirSel() )
			{
				return _T("");
			}

			*ppSelDpC = pSelDpC;
			return pSelDpC->GetSchemeName();
		}
	}

	return _T("");
}

int CDlgLeftTabSelP::FindLastSameItem( CData **parclData, int iFirstIndex, int iLastIndex, PageField_enum eKey )
{
	int iIndexItem1 = iFirstIndex;
	int iIndexItem2 = iFirstIndex + 1;
	
	// If key doesn't exist so return complete block ...
	if( epfNONE == eKey )
	{
		return iLastIndex;
	}
	
	while( iIndexItem2 <= iLastIndex )
	{
		// If Item1 and Item2 are different...
		if( 0 != parclData[iIndexItem1]->CompareSelectionTo( parclData[iIndexItem2], eKey ) )
		{
			return iIndexItem1;
		}

		iIndexItem1++;
		iIndexItem2++;
	}
	
	return iIndexItem1;
}

int CDlgLeftTabSelP::SetRemarkIndex( LPCTSTR strClassName, CData **parclData, int iArraySize, int iFirstIndex )
{
	CDS_SelThing **pSelThing = NULL;
	CData **pData = NULL;
	
	// HYS-1741: CDS_SelPWQAccServices
	// HYS-2007: CDS_SSelDpSensor
	if(	0 == _tcscmp( strClassName, _T("CData") ) 
			|| 0 == _tcscmp( strClassName, _T("CDS_HmHub") )
			|| 0 == _tcscmp( strClassName, _T("CDS_Accessory") )
			|| 0 == _tcscmp( strClassName, _T("CDS_Actuator") )
			|| 0 == _tcscmp( strClassName, _T("CDS_SSelCtrl") )
			|| 0 == _tcscmp( strClassName, _T("CDS_SSelDpC") )
			|| 0 == _tcscmp( strClassName, _T("CDS_SSelDpReliefValve") )
			|| 0 == _tcscmp( strClassName, _T("CDS_SSelDpCBCV") )
			|| 0 == _tcscmp( strClassName, _T("CDS_SSelSv") )
			|| 0 == _tcscmp( strClassName, _T("CDS_SSelBv") )
			|| 0 == _tcscmp( strClassName, _T("CDS_SSelRadSet") )
			|| 0 == _tcscmp( strClassName, _T("CDS_SSelAirVentSeparator") )
			|| 0 == _tcscmp( strClassName, _T("CDS_SSelPMaint") )
			|| 0 == _tcscmp( strClassName, _T("CDS_SSelSafetyValve") )
			|| 0 == _tcscmp( strClassName, _T("CDS_SSel6WayValve") )
			|| 0 == _tcscmp( strClassName, _T("CDS_SSelTapWaterControl") ) 
			|| 0 == _tcscmp( strClassName, _T("CDS_SSelSmartControlValve") ) 
			|| 0 == _tcscmp( strClassName, _T("CDS_SSelSmartDpC") ) 
			|| 0 == _tcscmp( strClassName, _T("CDS_SSelFloorHeatingManifold") )
			|| 0 == _tcscmp( strClassName, _T("CDS_SSelFloorHeatingValve") )
			|| 0 == _tcscmp( strClassName, _T("CDS_SSelFloorHeatingController") )
			|| 0 == _tcscmp( strClassName, _T("CDS_SelPWQAccServices") )
			|| 0 == _tcscmp( strClassName, _T("CDS_SSelDpSensor") ) )
	{
		pData = (CData **)parclData;
	}
	else if( 0 == _tcscmp( strClassName, _T("CDS_DirectSel") ) )
	{
		pSelThing = (CDS_SelThing **)parclData;
	}
	else
	{
		ASSERTA_RETURN( -1 );
	}

	int i;
	int iIndex = ( 0 != iFirstIndex ) ? iFirstIndex : 1;
	
	// Clear all remark index.
	for( i = 0; i < iArraySize; i++ )
	{	
		if( NULL != pData )
		{
			pData[i]->GetpSelectedInfos()->SetRemarkIndex( 0 );
		}
		else
		{
			pSelThing[i]->SetRemarkIndex( 0 );
		}
	}
	
	for( i = 0; i < iArraySize; i++ )
	{
		if( NULL != pData )
		{
			// If remark index already set...
			if( pData[i]->GetpSelectedInfos()->GetRemarkIndex() )
			{
				continue;
			}
			
			// If remark is empty...
			if( true == pData[i]->GetpSelectedInfos()->GetRemark().IsEmpty() )
			{
				continue;
			}
			
			pData[i]->GetpSelectedInfos()->SetRemarkIndex( iIndex );
			
			for( int j = i + 1; j < iArraySize; j++ )
			{
				// If remarks are same...
				if( 0 == StringCompare( pData[i]->GetpSelectedInfos()->GetRemark(), pData[j]->GetpSelectedInfos()->GetRemark() ) )
				{
					pData[j]->GetpSelectedInfos()->SetRemarkIndex( iIndex );
				}
			}
		}
		else
		{
			// If remark index already set...
			if( pSelThing[i]->GetRemarkIndex() )
			{
				continue;
			}

			// If remark is empty...
			if( '\0' == *( pSelThing[i]->GetRemark() ) )
			{
				continue;
			}

			pSelThing[i]->SetRemarkIndex( iIndex );

			for( int j = i + 1; j < iArraySize; j++ )
			{
				// If remarks are same...
				if( 0 == StringCompare( pSelThing[i]->GetRemark(), pSelThing[j]->GetRemark() ) )
				{
					pSelThing[j]->SetRemarkIndex( iIndex );
				}
			}
		}

		iIndex++;
	}

	return iIndex;
}

int CDlgLeftTabSelP::SetRemarkIndex( CSelProdPageBase::HMvector *pvecHMVector, int iFirstIndex )
{
	if( NULL == pvecHMVector || 0 == (int)pvecHMVector->size() )
	{
		return -1;
	}

	int i;
	int iIndex = ( 0 != iFirstIndex ) ? iFirstIndex : 1;
	
	// Clear all remark index.
	for( i = 0; i < (int)pvecHMVector->size(); i++ )
	{
		CDS_HydroMod *pHM = NULL;

		if( CSelProdPageBase::eptHM == pvecHMVector->at( i ).first )
		{
			pHM = dynamic_cast<CDS_HydroMod *>( (CData*)( pvecHMVector->at( i ).second ) );
		}
		else
		{
			CDS_HydroMod::CBase *pBase = static_cast<CDS_HydroMod::CBase *>( pvecHMVector->at( i ).second );

			if( NULL != pBase )
			{
				pHM = pBase->GetpParentHM();
			}
		}

		if( NULL != pHM )
		{
			pHM->SetRemarkIndex( 0 );
		}
	}
	
	// Make a first loop to merge remarks and comments. It's exactly the same thing that 'CDlgConfSel' do for the individual selection.
	// Remark: We merge locally but not set the merge with 'CDS_HydroMod::SetRemL1'. Otherwise, if we edit a circuit and remark contains 
	//         all (remark by user and comment on the product), comment on the product will be added and we will get the user remark and
	//         twice the comment on the product. And so on.
	std::vector<CString> vecRemarkCommentMerged;

	for( i = 0; i < (int)pvecHMVector->size(); i++ )
	{
		CDS_HydroMod *pHM = NULL;
		CDS_HydroMod::CBase *pBase = NULL;

		if( CSelProdPageBase::eptHM == pvecHMVector->at( i ).first )
		{
			// CBI object.
			pHM = dynamic_cast<CDS_HydroMod *>( (CData*)( pvecHMVector->at( i ).second ) );
		}
		else
		{
			// CBV, CShutoffValve, CDpC, CCv or CMeasData.
			pBase = static_cast<CDS_HydroMod::CBase *>( pvecHMVector->at( i ).second );
		}

		// If remark index already set...
		if( ( NULL != pHM && 0 != pHM->GetRemarkIndex() ) ||  ( NULL != pBase && 0 != pBase->GetpParentHM()->GetRemarkIndex() ) )
		{
			vecRemarkCommentMerged.push_back( _T("") );
			continue;
		}

		CString strRemark = _T("");

		// Check if remark exist.
		if( NULL != pHM )
		{
			if( false == pHM->GetRemL1().IsEmpty() )
			{
				strRemark = pHM->GetRemL1();
			}

			// Check comment in the product in CBI.
			// Remark: Why only CBI? Because in the 'CSelProdDockPane::_ExtractHMInfos' method we insert in the 'm_arHMmap' a
			//        'CDS_HydroMod*' only in the case of the circuit contains a CBI valve. For all other cases it's either
			//        'CDS_HyDroMod::CBV*', 'CDS_HyDroMod::CCV*', 'CDS_HyDroMod::CDpC*', 'CDS_HyDroMod::CCv*' or 'CDS_HyDroMod::CMeasData*'.
			if( NULL != dynamic_cast<CDB_Product*>( pHM->GetCBIValveIDPtr().MP ) )
			{
				CDB_Product *pclProduct = dynamic_cast<CDB_Product*>( pHM->GetCBIValveIDPtr().MP );
					
				if( false == CString( pclProduct->GetComment() ).IsEmpty() )
				{
					if( true == strRemark.IsEmpty() )
					{
						strRemark = CString( pclProduct->GetComment() );
					}
					else
					{
						strRemark += _T("\r\n") + CString( pclProduct->GetComment() );
					}
				}
			}
		}
		else if( NULL != pBase )
		{
			CString strRemark1 = pBase->GetpParentHM()->GetRemL1();
			CString strComment1;

			switch( pvecHMVector->at( i ).first )
			{
				case CSelProdPageBase::eptCBV:
					{
						CDS_HydroMod::CBV *pclHMBv = dynamic_cast<CDS_HydroMod::CBV *>( pBase );
						pHM = pclHMBv->GetpParentHM();
						
						if( NULL != pclHMBv->GetpTAP() )
						{
							strComment1 = pclHMBv->GetpTAP()->GetComment();
						}
					}

					break;

				case CSelProdPageBase::eptCSV:
					{
						CDS_HydroMod::CShutoffValve *pclHMShutoffValve = dynamic_cast<CDS_HydroMod::CShutoffValve *>( pBase );
						pHM = pclHMShutoffValve->GetpParentHM();
						
						if( NULL != pclHMShutoffValve->GetpTAP() )
						{
							strComment1 = pclHMShutoffValve->GetpTAP()->GetComment();
						}
					}
					break;

				case CSelProdPageBase::eptCDpC:
					{
						CDS_HydroMod::CDpC *pclHMDpC = dynamic_cast<CDS_HydroMod::CDpC*>( pBase );
						pHM = pclHMDpC->GetpParentHM();
						
						if( NULL != pclHMDpC->GetpTAP() )
						{
							strComment1 = pclHMDpC->GetpTAP()->GetComment();
						}

						// Check also if there is a regulating valve linked with DpC.
						if( NULL != pHM->GetpBv() && NULL != pHM->GetpBv()->GetpTAP() &&
							false == CString( pHM->GetpBv()->GetpTAP()->GetComment() ).IsEmpty() )
						{
							if( true == strComment1.IsEmpty() )
							{
								strComment1 = CString( pHM->GetpBv()->GetpTAP()->GetComment() );
							}
							else
							{
								strComment1 += _T("\r\n") + CString( pHM->GetpBv()->GetpTAP()->GetComment() );
							}
						}
					}
					break;

				case CSelProdPageBase::eptCCV:
					{
						CDS_HydroMod::CCv *pclHMCv = dynamic_cast<CDS_HydroMod::CCv*>( pBase );
						pHM = pclHMCv->GetpParentHM();
						
						CDB_ControlValve *pclControlValve = dynamic_cast<CDB_ControlValve*>( pclHMCv->GetpTAP() );
						
						if( NULL != pclControlValve )
						{
							strComment1 = pclHMCv->GetpTAP()->GetComment();

							// If supply valve is a combined Dp controller, balancing and control valve, we can have a shutoff valve on the 
							// return pipe.
							if( true == pclControlValve->IsaDPCBCV() && NULL != pHM->GetpShutoffValve( CDS_HydroMod::eShutoffValveReturn ) &&
								NULL != dynamic_cast<CDB_ShutoffValve*>( pHM->GetpShutoffValve( CDS_HydroMod::eShutoffValveReturn )->GetpTAP() ) )
							{
								CDB_ShutoffValve *pclSV = dynamic_cast<CDB_ShutoffValve*>( pHM->GetpShutoffValve( CDS_HydroMod::eShutoffValveReturn )->GetpTAP() );

								if( true == strComment1.IsEmpty() )
								{
									strComment1 = CString( pclSV->GetComment() );
								}
								else
								{
									strComment1 += _T("\r\n") + CString( pclSV->GetComment() );
								}
							}
						}
					}
					break;

				case CSelProdPageBase::eptCMeasData:
					CDS_HydroMod::CMeasData *pclHMMeasData = dynamic_cast<CDS_HydroMod::CMeasData*>( pBase );
					pHM = pclHMMeasData->GetpParentHM();
					// TODO.
					break;
			}

			strRemark = strRemark1;

			if( NULL != pHM && false == strComment1.IsEmpty() )
			{
				// It is to help us to fill remark and comment in the result page.
				pHM->SetComment( strComment1 );

				if( true == strRemark.IsEmpty() )
				{
					strRemark = strComment1;
				}
				else
				{
					strRemark += _T("\r\n") + strComment1;
				}
			}
		}

		if( true == strRemark.IsEmpty() )
		{
			vecRemarkCommentMerged.push_back( _T("") );
			continue;
		}

		vecRemarkCommentMerged.push_back( strRemark );
	}

	// Do now a second loop to set the remark index.
	for( i = 0; i < (int)pvecHMVector->size(); i++ )
	{
		CDS_HydroMod *pHM = NULL;

		if( CSelProdPageBase::eptHM == pvecHMVector->at( i ).first )
		{
			// CBI object.
			pHM = dynamic_cast<CDS_HydroMod *>( (CData*)( pvecHMVector->at( i ).second ) );
		}
		else
		{
			// CBV, CShutoffValve, CDpC, CCv or CMeasData.
			CDS_HydroMod::CBase *pBase = static_cast<CDS_HydroMod::CBase *>( pvecHMVector->at( i ).second );
			pHM = pBase->GetpParentHM();
		}

		// If remark index already set...
		if( NULL != pHM && 0 != pHM->GetRemarkIndex() )
		{
			continue;
		}

		// Check if remark exist.
		if( true == vecRemarkCommentMerged[i].IsEmpty() )
		{
			continue;
		}

		pHM->SetRemarkIndex( iIndex );

		// Check if one other remark is not the same.
		for( int j = i + 1; j < (int)pvecHMVector->size(); j++ )
		{
			CDS_HydroMod *pHMj = NULL;

			if( CSelProdPageBase::eptHM == pvecHMVector->at( j ).first )
			{
				// CBI object.
				pHMj = dynamic_cast<CDS_HydroMod *>( (CData*)( pvecHMVector->at( j ).second ) );
			}
			else
			{
				// CBV, CShutoffValve, CDpC, CCv or CMeasData.
				CDS_HydroMod::CBase *pBasej = static_cast<CDS_HydroMod::CBase *>( pvecHMVector->at( j ).second );
				pHMj = pBasej->GetpParentHM();
			}

			// If remarks are same...
			if( 0 == StringCompare( vecRemarkCommentMerged[i], vecRemarkCommentMerged[j] ) )
			{
				pHMj->SetRemarkIndex( iIndex );
			}
		}

		iIndex++;
	}

	return iIndex;
}

void CDlgLeftTabSelP::SetSchemeIndex( LPCTSTR strClassName, CData **parclData, int iArraySize )
{
	// Scheme is a member variable of the 'CDS_SSelDpC'.
	CDS_SSelDpC **pSSelDpC = (CDS_SSelDpC**)parclData;
	
	// Clear all scheme index.
	int i;
	for( i = 0; i < iArraySize; i++ )
	{
		pSSelDpC[i]->SetSchemeIndex( 0 );
	}
		
	int iIndex = 1;
	i = 0;
	while( i < iArraySize )
	{
		if( 0 != pSSelDpC[i]->GetSchemeIndex() )
		{
			// Scheme index already set.
			i++;
			continue;
		}

		if( TRUE == pSSelDpC[i]->GetSchemeName().IsEmpty() || true == pSSelDpC[i]->IsFromDirSel() )
		{
			// Scheme is empty or the current 'CDS_SSelDpC' object comes from direct selection.
			i++;
			continue;
		}
		
		int k = i;
		bool ffind = false;
		CString name = pSSelDpC[i]->GetSchemeName();
		int j;

		// Sort Scheme index.
		for( j = i + 1; j < iArraySize; j++ )
		{
			// If scheme index already set...
			if( 0 != pSSelDpC[j]->GetSchemeIndex() )
			{
				continue;
			}
			
			// If scheme is empty or the current 'CDS_SSelDpC' object comes from direct selection.
			if( TRUE == pSSelDpC[j]->GetSchemeName().IsEmpty() || true == pSSelDpC[j]->IsFromDirSel() )
			{
				continue;
			}
			
			if( name.CollateNoCase( pSSelDpC[j]->GetSchemeName() ) > 0 )
			{
				name = pSSelDpC[j]->GetSchemeName();
				ffind = true;
				k = j;
			}
		}

		pSSelDpC[k]->SetSchemeIndex( iIndex );
		
		// Set same index for same name.
		for( j = k + 1; j < iArraySize; j++ )
		{
			// If SchemeID are same...
			if( 0 == name.CollateNoCase( pSSelDpC[j]->GetSchemeName() ) )
			{
				pSSelDpC[j]->SetSchemeIndex( iIndex );
			}
		}

		iIndex++;

		if( false == ffind )
		{
			i++;
		}
	}
}

void CDlgLeftTabSelP::SortSelection( CData **parclData, int iFirstIndex, int iLastIndex, PageField_enum eKey )
{
	if( iFirstIndex >= iLastIndex )
	{
		return;
	}

	if( epfNONE == eKey )
	{
		return;
	}
	
	int i = iFirstIndex;
	int j = iLastIndex;
	int iPivot;
	CData *temp;

	iPivot = ( iLastIndex + iFirstIndex ) / 2;
	do
	{
		
		while( i != iPivot && parclData[i]->CompareSelectionTo( parclData[iPivot], eKey ) < 0 && i < iLastIndex )
		{
			i++;
		}
		
		while( j != iPivot && parclData[iPivot]->CompareSelectionTo( parclData[j], eKey ) < 0 && j > iFirstIndex )
		{
			j--;
		}
		
		if( i <= j )
		{
			// Swap values.
			temp = parclData[i];
			parclData[i] = parclData[j];
			parclData[j] = temp;
			
			// pivot move ?
			if( iPivot == i )
			{
				iPivot = j;
			}
			else if( iPivot == j )
			{
				iPivot = i;
			}
			
			i++;
			j--;
		}
	}while( i <= j );
	
	if( iFirstIndex < j )
	{
		SortSelection( parclData, iFirstIndex, j, eKey );
	}
	
	if( i < iLastIndex )
	{
		SortSelection( parclData, i, iLastIndex, eKey );
	}
}

void CDlgLeftTabSelP::SortTable( CData **parclData, int iLast )
{
	// All array with first key.
	
	// Last index for Key 0.
	int iLK1 = 0;

	// First index for Key 0.
	int iFK1 = 0;
	
	// Sort with key 0.
	SortSelection( parclData, iFK1, iLast, m_pTADSPageSetup->GetKey( 0 ) );
	
	do
	{
		// Search if a block exist.
		iLK1 = FindLastSameItem( parclData, iFK1, iLast, m_pTADSPageSetup->GetKey( 0 ) );

		if( iLK1 == iFK1 )
		{
			// Only one item in this block
			iFK1++;
		}
		else
		{
			// Block exist continue sorting with Key 1.
			int iLK2, iFK2;
			
			// Initialize current block index.
			iLK2 = iFK2 = iFK1;
			SortSelection( parclData, iFK1, iLK1, m_pTADSPageSetup->GetKey( 1 ) );

			do
			{
				iLK2 = FindLastSameItem( parclData, iFK2, iLK1, m_pTADSPageSetup->GetKey( 1 ) );
				
				if( iLK2 == iFK2 )
				{
					iFK2++;
				}
				else
				{
					// Found block sort with key 2.
					SortSelection( parclData, iFK2, iLK2, m_pTADSPageSetup->GetKey( 2 ) );
				
					// Next block.
					iFK2 = iLK2 + 1;
				}
				
				if( iFK2 < iLK2 )
				{
					iFK2 = iLK2 + 1;
				}
			}
			while( iLK2 < iLK1 && iFK2 < iLK1 );
		}
		
		if( iFK1 < iLK1 )
		{
			iFK1 = iLK1 + 1;
		}
	}
	while( iLK1 < iLast && iFK1 < iLast );
}

BEGIN_MESSAGE_MAP( CDlgLeftTabSelP, CDlgLeftTabBase )
	ON_MESSAGE( WM_USER_NEWDOCUMENT, OnNewDocument )
	ON_MESSAGE( WM_USER_PIPECHANGE, OnParamChange )
	ON_MESSAGE( WM_USER_TECHPARCHANGE, OnParamChange )
	ON_MESSAGE( WM_USER_UNITCHANGE, OnParamChange )
	ON_MESSAGE( WM_USER_WATERCHANGE, OnWaterChange )
	ON_BN_CLICKED( IDC_BUTTONDELSETUP, OnButtonDeleteSetup )
	ON_BN_CLICKED( IDC_BUTTONADDSETUP, OnButtonAddSetup )
	ON_CBN_SELCHANGE( IDC_COMBOSETUP, OnSelChangeComboSetup )
	ON_CBN_SELCHANGE( IDC_COMBOKEY1, OnSelChangeComboSortKey1 )
	ON_CBN_SELCHANGE( IDC_COMBOKEY2, OnSelChangeComboSortKey2 )
	ON_CBN_SELCHANGE( IDC_COMBOKEY3, OnSelChangeComboSortKey3 )
	ON_BN_CLICKED( IDC_BUTTONSORT, OnButtonSort )
	ON_BN_CLICKED( IDC_BUTTONPFIELD, OnButtonFormatColumn )
	ON_BN_CLICKED( IDC_BUTTONHUBEXPORTCONFIGURATIONFILE, &CDlgLeftTabSelP::OnBnClickedHubExportConfigurationFile )
	ON_BN_CLICKED(IDC_BUTTOND81, &CDlgLeftTabSelP::OnBnClickedExportD81)
	ON_BN_CLICKED(IDC_BUTTOND82, &CDlgLeftTabSelP::OnBnClickedExportD82)
END_MESSAGE_MAP()

void CDlgLeftTabSelP::DoDataExchange( CDataExchange* pDX )

{
	CDlgLeftTabBase::DoDataExchange( pDX );
	DDX_Control( pDX, IDC_BUTTONPFIELD, m_ButtonPageField );
	DDX_Control( pDX, IDC_BUTTONSORT, m_ButtonSort );
	DDX_Control( pDX, IDC_BUTTONDELSETUP, m_ButtonDelSetup );
	DDX_Control( pDX, IDC_BUTTONADDSETUP, m_ButtonAddSetup );
	DDX_Control( pDX, IDC_COMBOSETUP, m_ComboSetup );
	DDX_Control( pDX, IDC_COMBOKEY3, m_ComboKey3 );
	DDX_Control( pDX, IDC_COMBOKEY2, m_ComboKey2 );
	DDX_Control( pDX, IDC_COMBOKEY1, m_ComboKey1 );
	DDX_Control( pDX, IDC_GROUPDISPINFO, m_GroupDispInfo );
	DDX_Control( pDX, IDC_GROUPSORTING, m_GroupSorting );
	DDX_Control( pDX, IDC_GROUPTAHUB, m_GroupTAHUB );
	
}

BOOL CDlgLeftTabSelP::OnInitDialog() 
{
	CDlgLeftTabBase::OnInitDialog();
	
	// Initialize dialog strings.
	CString str = TASApp.LoadLocalizedString( IDS_TABCDIALOGSELP_STATICSETUP );
	GetDlgItem( IDC_STATICSETUP )->SetWindowText( str );
	str = TASApp.LoadLocalizedString( IDS_TABCDIALOGSELP_GROUPDISPINFO );
	m_GroupDispInfo.SetWindowText( str );

	str = TASApp.LoadLocalizedString( IDS_TABCDIALOGSELP_BUTTONPFIELD );
	GetDlgItem( IDC_BUTTONPFIELD )->SetWindowText( str );
	str = TASApp.LoadLocalizedString( IDS_TABCDIALOGSELP_GROUPSORTING );
	m_GroupSorting.SetWindowText( str );

	str = TASApp.LoadLocalizedString( IDS_TABCDIALOGSELP_GROUPTAHUB );
	m_GroupTAHUB.SetWindowText( str );

	str = TASApp.LoadLocalizedString( IDS_TABCDIALOGSELP_BUTTONHUBEXPCONFIGFILE );
	GetDlgItem( IDC_BUTTONHUBEXPORTCONFIGURATIONFILE )->SetWindowText( str );

	str.Empty();

	pDlgLeftTabSelP	= this;
	
	// Create a ToolTipCtrl and add a tool tip for each button.
	m_ToolTip.Create( this, TTS_NOPREFIX );

	// Set Bmp and add a tool tip on Add and Delete Buttons.
	CImageList* pclImgListButton = TASApp.GetpRCImageManager()->GetImageList( CRCImageManager::ILN_Button );
	ASSERT ( NULL != pclImgListButton );

	if( NULL != pclImgListButton )
	{
		m_ButtonDelSetup.SetIcon( (HICON)pclImgListButton->ExtractIcon( CRCImageManager::ILB_TrashBin ) );
	}

	CString TTstr = TASApp.LoadLocalizedString( IDS_LVTOOLTIP_BUTDELSETUP );
	m_ToolTip.AddToolWindow( &m_ButtonDelSetup, TTstr );

	if( NULL != pclImgListButton )
	{
		m_ButtonAddSetup.SetIcon( (HICON)pclImgListButton->ExtractIcon( CRCImageManager::ILB_SavePageSetup ) );
	}

	TTstr = TASApp.LoadLocalizedString( IDS_LVTOOLTIP_BUTADDSETUP );
	m_ToolTip.AddToolWindow( &m_ButtonAddSetup, TTstr );

	if( NULL != pclImgListButton )
	{
		m_ButtonSort.SetIcon( (HICON)pclImgListButton->ExtractIcon( CRCImageManager::ILB_Apply ) );
	}

	TTstr = TASApp.LoadLocalizedString( IDS_LVTOOLTIP_BUTSORT );
	m_ToolTip.AddToolWindow( &m_ButtonSort, TTstr );

	int i;
	for( i = CDB_PageSetup::enCheck::FIRSTFIELD; i < CDB_PageSetup::enCheck::LASTFIELD; i++ )
	{
		HTreeItem[i] = 0;
	}

	// Set proper style to groups.
	CImageList* pclImgListGroupBox = TASApp.GetpRCImageManager()->GetImageList( CRCImageManager::ILN_GroupBox );
	ASSERT ( NULL != pclImgListGroupBox );

	if( NULL != pclImgListGroupBox )
	{
		m_GroupDispInfo.SetImageList( pclImgListGroupBox, CRCImageManager::ILGB_Table );
	}

	m_GroupDispInfo.SetInOffice2007Mainframe( true );
	
	if( NULL != pclImgListGroupBox )
	{
		m_GroupSorting.SetImageList( pclImgListGroupBox, CRCImageManager::ILGB_Key );
	}

	m_GroupSorting.SetInOffice2007Mainframe( true );
	
	if( NULL != pclImgListGroupBox )
	{
		m_GroupTAHUB.SetImageList( pclImgListGroupBox, CRCImageManager::ILGB_Hub );
	}

	m_GroupTAHUB.SetInOffice2007Mainframe( true );

	// Init Sorting combo box.
	i = m_ComboKey1.AddString( _T("") );
	m_ComboKey1.SetItemData( i, epfNONE );
	i = m_ComboKey2.AddString( _T("") );
	m_ComboKey2.SetItemData( i, epfNONE );
	i = m_ComboKey3.AddString( _T("") );
	m_ComboKey3.SetItemData( i, epfNONE );
	
	// First reference.
	str = TASApp.LoadLocalizedString( IDS_SELP_1STREF );
	i = m_ComboKey1.AddString( str );
	m_ComboKey1.SetItemData( i, epfFIRSTREF );
	i = m_ComboKey2.AddString( str );
	m_ComboKey2.SetItemData( i, epfFIRSTREF );
	i = m_ComboKey3.AddString( str );
	m_ComboKey3.SetItemData( i, epfFIRSTREF );
	
	// Second reference.
	str = TASApp.LoadLocalizedString( IDS_SELP_2NDREF );
	i = m_ComboKey1.AddString( str );
	m_ComboKey1.SetItemData( i, epfSECONDREF );
	i = m_ComboKey2.AddString( str );
	m_ComboKey2.SetItemData( i, epfSECONDREF );
	i = m_ComboKey3.AddString( str );
	m_ComboKey3.SetItemData( i, epfSECONDREF );
	
	// Product name.
	str = TASApp.LoadLocalizedString( IDS_SELP_NAME );
	i = m_ComboKey1.AddString( str );
	m_ComboKey1.SetItemData( i, epfBVINFOPRODUCTNAME );
	i = m_ComboKey2.AddString( str );
	m_ComboKey2.SetItemData( i, epfBVINFOPRODUCTNAME );
	i = m_ComboKey3.AddString( str );
	m_ComboKey3.SetItemData( i, epfBVINFOPRODUCTNAME );

	// Product size.
	str = TASApp.LoadLocalizedString( IDS_SELP_SIZE );
	i = m_ComboKey1.AddString( str );
	m_ComboKey1.SetItemData( i, epfBVINFOPRODUCTSIZE );
	i = m_ComboKey2.AddString( str );
	m_ComboKey2.SetItemData( i, epfBVINFOPRODUCTSIZE );
	i = m_ComboKey3.AddString( str );
	m_ComboKey3.SetItemData( i, epfBVINFOPRODUCTSIZE );

	// Unit Price.
	if( TASApp.IsPriceUsed() )
	{
		str = TASApp.LoadLocalizedString( IDS_SELP_UNITPRICE );
		i = m_ComboKey1.AddString( str );
		m_ComboKey1.SetItemData( i, epfSALEUNITPRICE );
		i = m_ComboKey2.AddString( str );
		m_ComboKey2.SetItemData( i, epfSALEUNITPRICE );
	}

	// Quantity.
	str = TASApp.LoadLocalizedString( IDS_SELP_QUANTITY );
	i = m_ComboKey1.AddString( str );
	m_ComboKey1.SetItemData( i, epfQUANTITY );
	i = m_ComboKey2.AddString( str );
	m_ComboKey2.SetItemData( i, epfQUANTITY );
	i = m_ComboKey3.AddString( str );
	m_ComboKey3.SetItemData( i, epfQUANTITY );

	// Product article number.
	str = TASApp.LoadLocalizedString( IDS_SELP_ARTICLE );
	i = m_ComboKey1.AddString( str );
	m_ComboKey1.SetItemData( i, epfARTICLE );
	i = m_ComboKey2.AddString( str );
	m_ComboKey2.SetItemData( i, epfARTICLE );
	i = m_ComboKey3.AddString( str );
	m_ComboKey3.SetItemData( i, epfARTICLE );

	m_ComboKey1.SetCurSel( 0 );
	m_ComboKey2.SetCurSel( 0 );
	m_ComboKey3.SetCurSel( 0 );

	// Init Combo Page Setup and Page Field.
	m_ComboSetup.LimitText( _STRING_LENGTH );

	if( false == TASApp.IsGAEBExport() )
	{
		GetDlgItem( IDC_GROUPGAEBEXPORT )->ShowWindow( SW_HIDE );
		GetDlgItem( IDC_BUTTOND81 )->ShowWindow( SW_HIDE );
		GetDlgItem( IDC_BUTTOND82 )->ShowWindow( SW_HIDE );
	}

	return TRUE;
}

void CDlgLeftTabSelP::OnOK() 
{
	PREVENT_ENTER_KEY

	CDlgLeftTabBase::OnOK();
}

BOOL CDlgLeftTabSelP::OnCommand( WPARAM wParam, LPARAM lParam )
{
	CWnd *pWnd = GetFocus();
	switch( wParam )
	{
		case IDOK:
			if( pWnd != GetDlgItem( IDOK ) )
			{
				return FALSE;
			}
			break;

		case IDCANCEL:
			if( pWnd != GetDlgItem( IDCANCEL ) )
			{
				return FALSE;
			}
			break;
	}

	return CDlgLeftTabBase::OnCommand( wParam, lParam );
}

LRESULT CDlgLeftTabSelP::OnNewDocument( WPARAM wParam, LPARAM lParam )
{
	m_pTADB = TASApp.GetpTADB();
	m_pTADS = TASApp.GetpTADS();
	m_pUSERDB = TASApp.GetpUserDB();

	// Retrieve last selected Page Setup from TADS.
	m_pTADSPageSetup = m_pTADS->GetpPageSetup();
	ASSERT( NULL != m_pTADSPageSetup );

	// Initialize ComboSetup with all possibility existing in USERDB.
	InitPageSetupDD();

	// Try to find a corresponding page setup in ComboSetup.
	if( m_ComboSetup.GetCount() > 0 )
	{
		m_ComboSetup.SetCurSel( m_ComboSetup.FindStringExact( 0, m_pTADSPageSetup->GetName() ) );
	}

	// Refresh current page.
	InitPageSetup( m_pTADSPageSetup );

	// Refresh the RViewSelProd.
	OnParamChange( 0, 0 );
	
	RefreshHubGroup();

	return 0;
}

LRESULT CDlgLeftTabSelP::OnParamChange( WPARAM wParam, LPARAM lParam )
{
	if( NULL != pRViewSelProd )
	{
		TASApp.GetpTADS()->RefreshResults( true );

		if( TRUE == pRViewSelProd->IsWindowVisible() )
		{
			pRViewSelProd->RedrawRightView();
		}
	}
	return 0;
}

LRESULT CDlgLeftTabSelP::OnWaterChange( WPARAM wParam, LPARAM lParam )
{
	if( WMUserWaterCharWParam::WM_UWC_WP_ForProductSel != ( (WMUserWaterCharWParam)wParam & WM_UWC_WP_ForProductSel )
			|| WMUserWaterCharLParam::WM_UWC_LWP_NoChange == (WMUserWaterCharLParam)lParam )
	{
		return 0;
	}

	if( NULL != pRViewSelProd )
	{
		TASApp.GetpTADS()->RefreshResults( true );

		if( TRUE == pRViewSelProd->IsWindowVisible() )
		{
			pRViewSelProd->RedrawRightView();
		}
	}

	return 0;
}

void CDlgLeftTabSelP::OnButtonDeleteSetup() 
{
	CString str, str1;
	
	// Check if one Setup entry is selected.
	m_ComboSetup.GetWindowText( str );
	
	if( _T("") == str )
	{
		return;
	}
	
	// If str cannot be found in the List box of the Combo, this means that the page setup has not been saved yet.
	if( CB_ERR == m_ComboSetup.FindStringExact( 0, str ) )
	{													
		m_ComboSetup.SetWindowText( _T("") );
		return;
	}

	CDB_PageSetup *pPageSetup = (CDB_PageSetup *)m_ComboSetup.GetItemDataPtr( m_ComboSetup.GetCurSel() );

	// Warning message.
	FormatString( str1, AFXMSG_DELETEENTRY, str );

	if( IDYES != AfxMessageBox( str1, MB_YESNO | MB_ICONQUESTION, 0 ) )
	{
		return;
	}

	IDPTR IDPtrPageSetup = ( (CDB_PageSetup*)pPageSetup )->GetIDPtr();
	ASSERT( '\0' != *IDPtrPageSetup.ID );
	
	// Get owner.
	CTable* pPSTab = (CTable *)( m_pUSERDB->Get( _T("PAGE_SETUP_TAB") ).MP );
	ASSERT( NULL != pPSTab );

	pPSTab->Remove( IDPtrPageSetup );
	VERIFY( m_pUSERDB->DeleteObject( IDPtrPageSetup ) );
	InitPageSetupDD();

	// Current Setup is erased.
	m_pTADSPageSetup->SetName( _T("") );
}

void CDlgLeftTabSelP::OnButtonAddSetup() 
{
	try
	{
		CString str, str1;
		m_ComboSetup.GetWindowText( str );

		if( _T("") == str )
		{
			return;
		}

		IDPTR IDPtr;
		CDB_PageSetup *pPageSetup;
	
		// Check if type exist.
		CTable *pPSTab = (CTable *)( m_pUSERDB->Get( _T("PAGE_SETUP_TAB") ).MP );
		
		if( NULL == pPSTab )
		{
			HYSELECT_THROW( _T("Internal error: 'PAGE_SETUP_TAB' doesn't exist in the user database.") );
		}
	
		// Check if this style already exist.
		if( pPSTab->GetItemCount( CLASS( CDB_PageSetup ) ) > 0 )
		{
			for( IDPtr = pPSTab->GetFirst( CLASS( CDB_PageSetup ) ); '\0' != *IDPtr.ID; IDPtr = pPSTab->GetNext() )
			{
				pPageSetup = (CDB_PageSetup *)( IDPtr.MP );
				str1 = pPageSetup->GetName();
			
				// If style already exist, warning message...
				if( str == str1 )
				{
					FormatString( str1, AFXMSG_ENTRYEXIST, str ); 
				
					if( IDNO == AfxMessageBox( str1, MB_YESNO | MB_ICONEXCLAMATION, 0 ) )
					{	
						m_ComboSetup.SetWindowText( _T("") );
						m_ComboSetup.SetFocus();
						return;
					}
					else
					{
						// Update Current page Setup.
						SavePageSetup( pPageSetup );
						return;
					}
				}	
			}
		}
	
		// Insert new Page Setup Style input.
		m_pUSERDB->CreateObject( IDPtr, CLASS( CDB_PageSetup ) );
		pPSTab->Insert( IDPtr );
	
		pPageSetup = (CDB_PageSetup *)IDPtr.MP;
		pPageSetup->SetName( (TCHAR *)(LPCTSTR)str );
		SavePageSetup( pPageSetup );
		InitPageSetupDD( str );	
	}
	catch( CHySelectException &clHySelectException )
	{
		clHySelectException.AddMessage( _T("Error in 'CDlgLeftTabSelP::OnButtonAddSetup'."), __LINE__, __FILE__ );
		throw;
	}
}

void CDlgLeftTabSelP::OnSelChangeComboSetup() 
{
	if( 0 == m_ComboSetup.GetCount() )
	{
		return;
	}
	
	// If no selection...
	if( m_ComboSetup.GetCurSel() < 0 )
	{
		return;
	}

	// Retrieve setup pointer.
	CDB_PageSetup *pPageSetup = (CDB_PageSetup *)m_ComboSetup.GetItemDataPtr( m_ComboSetup.GetCurSel() );
	ASSERT( NULL != pPageSetup );

	if( NULL == pPageSetup )
	{
		return;
	}
	
	InitPageSetup( pPageSetup );
	SavePageSetup( m_pTADSPageSetup );	
	
	if( NULL != pRViewSelProd )
	{
		pRViewSelProd->RedrawRightView();
	}
}

void CDlgLeftTabSelP::OnSelChangeComboSortKey1() 
{
	SavePageSetup( m_pTADSPageSetup );
}

void CDlgLeftTabSelP::OnSelChangeComboSortKey2() 
{
	SavePageSetup( m_pTADSPageSetup );
}

void CDlgLeftTabSelP::OnSelChangeComboSortKey3() 
{
	SavePageSetup( m_pTADSPageSetup );
}

void CDlgLeftTabSelP::OnButtonSort() 
{
	// Re-Draw RightView Selp and force sorting.
	if( NULL != pRViewSelProd )
	{
		TASApp.GetpTADS()->RefreshResults( true );

		bool fResetOrder = true;
		
		if( PageField_enum::epfNONE != m_pTADSPageSetup->GetKey( 0 ) || PageField_enum::epfNONE != m_pTADSPageSetup->GetKey( 1 ) 
	 		|| PageField_enum::epfNONE != m_pTADSPageSetup->GetKey( 2 ) )
		{
			fResetOrder = false;
		}

		pRViewSelProd->RedrawRightView( fResetOrder );
	}
}

void CDlgLeftTabSelP::OnButtonFormatColumn() 
{
	CDlgPageField dlg;
	dlg.Display();

	if( NULL != pRViewSelProd )
	{
		pRViewSelProd->RedrawRightView();
	}
}

void CDlgLeftTabSelP::OnBnClickedHubExportConfigurationFile()
{
	// Select a filename and path.
	DlgExportConfigFile dlg;
	dlg.DoModal();
}

void CDlgLeftTabSelP::RefreshHubGroup()
{
	// 'Hide', 'Enable' or 'Disable' the 'Export configuration' button.
	if( false == TASApp.IsHubDisplayed() )
	{
		GetDlgItem( IDC_GROUPTAHUB )->ShowWindow( false );
		GetDlgItem( IDC_BUTTONHUBEXPORTCONFIGURATIONFILE)->ShowWindow( false );
	}
	else 
	{
		CTable *pTab = m_pTADS->GetpHUBSelectionTable();
		ASSERT( NULL != pTab );

		if( _T('\0') != *pTab->GetFirst( CLASS( CDS_HmHub ) ).ID )
		{
			GetDlgItem( IDC_BUTTONHUBEXPORTCONFIGURATIONFILE )->EnableWindow( true );
		}
		else	
		{
			GetDlgItem( IDC_BUTTONHUBEXPORTCONFIGURATIONFILE )->EnableWindow( false );
		}
	}
}

void CDlgLeftTabSelP::InitPageSetupDD( CString str )
{
	int i;
	m_ComboSetup.ResetContent();
	m_ComboSetup.LimitText( _STRING_LENGTH );

	// Initialize ComboSetup with all possibility existing in USERDB.
	CTable* pPSTab = (CTable *)( m_pUSERDB->Get( _T("PAGE_SETUP_TAB") ).MP );
	ASSERT( NULL != pPSTab );
	
	CDB_PageSetup* pPageSetup = NULL;

	// If page setup exists...
	if( pPSTab->GetItemCount( CLASS( CDB_PageSetup ) ) > 0 )
	{
		for( IDPTR IDPtr = pPSTab->GetFirst( CLASS( CDB_PageSetup ) ); '\0' != *IDPtr.ID; IDPtr = pPSTab->GetNext() )
		{
			pPageSetup = (CDB_PageSetup *)( IDPtr.MP );
			i = m_ComboSetup.AddString( pPageSetup->GetName() );
			m_ComboSetup.SetItemDataPtr( i, (void *)pPageSetup );
		}
	}

	if( _T("") != str )
	{
		i = m_ComboSetup.FindStringExact( 0, str );

		if( i < 0 )
		{
			return;
		}

		m_ComboSetup.SetCurSel( i );								// Select Item 
	}
}

void CDlgLeftTabSelP::SavePageSetup( CDB_PageSetup *pPageSetup )
{
	CString	str;

	// Copy header, footer characteristic, displayed informations, ... 
	if( pPageSetup != m_pTADSPageSetup )
	{
		m_pTADSPageSetup->Copy( pPageSetup );
		pPageSetup->SetStyle( (CDB_PageField*)m_pTADSPageSetup ); 
		pPageSetup->SetString( (TCHAR *)m_pTADSPageSetup->GetString() );
	}

	m_ComboSetup.GetWindowText( str );
	pPageSetup->SetName( (TCHAR *)(LPCTSTR)str );
	int i = m_ComboKey1.GetCurSel();
	pPageSetup->SetKey( 0, (enum PageField_enum)m_ComboKey1.GetItemData( i ) );
	i = m_ComboKey2.GetCurSel();
	pPageSetup->SetKey( 1, (enum PageField_enum)m_ComboKey2.GetItemData( i ) );
	i = m_ComboKey3.GetCurSel();
	pPageSetup->SetKey( 2, (enum PageField_enum)m_ComboKey3.GetItemData( i ) );

	// Force to refresh the results list.
	TASApp.GetpTADS()->RefreshResults( true );
}

void CDlgLeftTabSelP::InitPageSetup( CDB_PageSetup *pPageSetup )
{
	m_pTADSPageSetup->SetStyle( (CDB_PageSetup *)pPageSetup );	// Update Current PageSetup						
	m_pTADSPageSetup->SetString( (TCHAR *)pPageSetup->GetString() );		// Field Name
	
	pPageSetup->Copy( m_pTADSPageSetup );

	int i;
	
	// Find sorting key.
	for( i = 0; i < m_ComboKey1.GetCount(); i++ )
	{
		if( (enum PageField_enum)m_ComboKey1.GetItemData(  i) == pPageSetup->GetKey( 0 ) )
		{
			m_ComboKey1.SetCurSel( i );
			break;
		}
	}

	for( i = 0; i < m_ComboKey2.GetCount(); i++ )
	{
		if( (enum PageField_enum)m_ComboKey2.GetItemData( i ) == pPageSetup->GetKey( 1 ) ) 
		{	
			m_ComboKey2.SetCurSel( i );
			break;
		}
	}
	
	for( i = 0; i < m_ComboKey3.GetCount(); i++ )
	{
		if( (enum PageField_enum)m_ComboKey3.GetItemData( i ) == pPageSetup->GetKey( 2 ) )
		{	
			m_ComboKey3.SetCurSel( i );
			break;
		}
	}
}


afx_msg void CDlgLeftTabSelP::OnBnClickedExportD81()
{
	if( NULL != pRViewSelProd )
	{
		pRViewSelProd->OnFileExportSelpD82( false );
	}
}


afx_msg void CDlgLeftTabSelP::OnBnClickedExportD82()
{
	if( NULL != pRViewSelProd )
	{
		pRViewSelProd->OnFileExportSelpD82( true );
	}
}
