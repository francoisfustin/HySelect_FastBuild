#include "stdafx.h"
#include "TASelect.h"
#include "MainFrm.h"
#include "Global.h"
#include "HydroMod.h"
#include "DlgLeftTabBase.h"
#include "DlgLeftTabSelP.h"
#include "RViewDescription.h"
#include "SelProdArticle.h"
#include "SelProdPageBase.h"
#include "SelProdPageSafetyValve.h"

CSelProdPageSafetyValve::CSelProdPageSafetyValve( CArticleGroupList *pclArticleGroupList ) 
	: CSelProdPageBase( CDB_PageSetup::enCheck::SAFETYVALVE, pclArticleGroupList )
{
	m_pSelected = NULL;
}

void CSelProdPageSafetyValve::Reset( void )
{
	CSelProdPageBase::Reset();
}

bool CSelProdPageSafetyValve::PreInit( HMvector& vecHMList )
{
	// Something for individual or direct selection ?
	CTable *pclTable = (CTable *)( TASApp.GetpTADS()->Get( _T("SAFETYVALVE_TAB") ).MP );

	if ( NULL == pclTable )
	{
		ASSERT( 0 );
		return false;
	}

	m_pclSelectionTable = ( '\0' != *pclTable->GetFirst().ID ) ? pclTable : NULL;
	bool bSafetyValveExist = ( NULL != m_pclSelectionTable );

	// Something for hydronic calculation?
	bool bSafetyValveHMExist = ( vecHMList.size() > 0 );

	if( false == bSafetyValveExist && false == bSafetyValveHMExist )
	{
		return false;
	}
	
	m_vecHMList = vecHMList;
	SortTable();
	return true;
}

bool CSelProdPageSafetyValve::Init( bool bResetOrder, bool fPrint )
{
	bool bSafetyValveExist = ( NULL != m_pclSelectionTable ) ? true : false;
	bool bSafetyValveHMExist = ( m_vecHMList.size() > 0 ) ? true : false;

	if( false == bSafetyValveExist && false == bSafetyValveHMExist )
	{
		return false;
	}

	// Call base class method to get a new 'Sheet'.
	CSheetDescription *pclSDSafetyValve = CMultiSpreadBase::CreateSSheet( SD_SafetyValve );

	if( NULL == pclSDSafetyValve || NULL == pclSDSafetyValve->GetSSheetPointer() )
	{
		return false;
	}
	
	CSSheet *pclSheet = pclSDSafetyValve->GetSSheetPointer();

	if( false == CSelProdPageBase::PrepareSheet( pclSDSafetyValve, ColumnDescription::Pointer, fPrint ) )
	{
		m_ViewDescription.RemoveOneSheetDescription( SD_SafetyValve );
		return false;
	}

	// Initialize column width for header and footer column.
	if( false == m_bForPrint )
	{
		SetColWidth( pclSDSafetyValve, ColumnDescription::Header, DefaultColumnWidth::DCW_Header );
		SetColWidth( pclSDSafetyValve, ColumnDescription::Footer, DefaultColumnWidth::DCW_Footer );
	}
	else
	{
		pclSheet->ShowCol( ColumnDescription::Header, FALSE );
		pclSheet->ShowCol( ColumnDescription::Footer, FALSE );
	}

	double dWidth = ( true == m_pTADSPageSetup->GetField( PageField_enum::epfFIRSTREF ) ) ? DefaultColumnWidth::DCW_Reference1 : 0.0;
	SetColWidth( pclSDSafetyValve, ColumnDescription::Reference1, dWidth );
	
	dWidth = ( true == m_pTADSPageSetup->GetField( PageField_enum::epfSECONDREF ) ) ? DefaultColumnWidth::DCW_Reference2 : 0.0;
	SetColWidth( pclSDSafetyValve, ColumnDescription::Reference2, dWidth );
	
	dWidth = ( true == m_pTADSPageSetup->GetField( PageField_enum::epfWATERINFO ) ) ? DefaultColumnWidth::DCW_Water : 0.0;
	SetColWidth( pclSDSafetyValve, ColumnDescription::Water, dWidth );
	
	dWidth = ( true == m_pTADSPageSetup->GetField( PageField_enum::epfSAFETYVALVEINFOPRODUCT ) ) ? DefaultColumnWidth::DCW_Product : 0.0;
	SetColWidth( pclSDSafetyValve, ColumnDescription::Product, dWidth );
	
	dWidth = ( true == m_pTADSPageSetup->GetField( PageField_enum::epfSAFETYVALVETECHINFO ) ) ? DefaultColumnWidth::DCW_TechnicalInfos : 0.0;
	SetColWidth( pclSDSafetyValve, ColumnDescription::TechnicalInfos, dWidth  );
	
	SetColWidth( pclSDSafetyValve, ColumnDescription::RadiatorInfos, 0.0 );
	
	dWidth = ( true == m_pTADSPageSetup->GetField( PageField_enum::epfARTICLE ) ) ? DefaultColumnWidth::DCW_ArticleNumber : 0.0;
	SetColWidth( pclSDSafetyValve, ColumnDescription::ArticleNumber, dWidth );
	
	dWidth = ( true == m_pTADSPageSetup->GetField( PageField_enum::epfPIPEINFO ) ) ? DefaultColumnWidth::DCW_Pipes : 0.0;
	SetColWidth( pclSDSafetyValve, ColumnDescription::Pipes, dWidth );
	
	dWidth = ( true == m_pTADSPageSetup->GetField( PageField_enum::epfQUANTITY ) ) ? DefaultColumnWidth::DCW_Quantity : 0.0;
	SetColWidth( pclSDSafetyValve, ColumnDescription::Quantity, dWidth );
	
	dWidth = 0.0;
	
	if( true == TASApp.IsPriceUsed() )
	{
		dWidth = ( true == m_pTADSPageSetup->GetField( PageField_enum::epfSALEUNITPRICE ) ) ? DefaultColumnWidth::DCW_UnitPrice : 0.0;
	}
	
	SetColWidth( pclSDSafetyValve, ColumnDescription::UnitPrice, dWidth );
	
	dWidth = 0.0;

	if( true == TASApp.IsPriceUsed() )
	{
		dWidth = ( true == m_pTADSPageSetup->GetField( PageField_enum::epfSALETOTALPRICE ) ) ? DefaultColumnWidth::DCW_TotalPrice : 0.0;
	}
	
	SetColWidth( pclSDSafetyValve, ColumnDescription::TotalPrice, dWidth );
	
	dWidth = ( true == m_pTADSPageSetup->GetField( PageField_enum::epfREMARK ) ) ? DefaultColumnWidth::DCW_Remark : 0.0;
	SetColWidth( pclSDSafetyValve, ColumnDescription::Remark, dWidth );
	
	SetColWidth( pclSDSafetyValve, ColumnDescription::Pointer, DefaultColumnWidth::DCW_Pointer );
	
	pclSheet->ShowCol( ColumnDescription::RadiatorInfos, FALSE );
	
	if( false == TASApp.IsPriceUsed() )
	{
		pclSheet->ShowCol( ColumnDescription::UnitPrice, FALSE );
		pclSheet->ShowCol( ColumnDescription::TotalPrice, FALSE );
	}

	pclSheet->ShowCol( ColumnDescription::Pointer, FALSE );
	CRect rect = pclSheet->GetSheetSizeInPixels( false );
	m_mapSheetInfos[pclSDSafetyValve].m_dPageWidth = rect.Width();

	// Page title.
	SetPageTitle( pclSDSafetyValve, IDS_RVIEWSELP_TITLE_SAFETYVALVE );
	
	// Init column header.
	_InitColHeader( pclSDSafetyValve );
	pclSheet->SetColumnAlwaysHidden( ColumnDescription::RadiatorInfos, true );
	
	// HYS-1571: We can optimize the column width now.
	// See Jira card to have more details about why we do that here.
	CRect rectClient;
	GetClientRect( &rectClient );
	OptimizePageSize( rectClient.Width(), rectClient.Height() );

	// Current position.
	long lRow = pclSheet->GetMaxRows() + 1;
	long lSelectedRow = 0;
	
	if( true == bSafetyValveExist )
	{
		// Number of objects.
		int iSafetyValveCount = m_pclSelectionTable->GetItemCount( CLASS( CDS_SSelSafetyValve ) );
		// HYS-1741: Pressurisation and water quality accessories and service is now in SAFETYVALVE_TAB.
		int iAccessAndServicesCount = m_pclSelectionTable->GetItemCount( CLASS( CDS_SelPWQAccServices ) );

		if( 0 == iSafetyValveCount && 0 == iAccessAndServicesCount )
		{
			return false;
		}
		
		CDS_SSelSafetyValve **paSSelSafetyValve = new CDS_SSelSafetyValve*[iSafetyValveCount];
		CDS_SelPWQAccServices **paSSelAccessAndServices = new CDS_SelPWQAccServices *[iAccessAndServicesCount];

		if( NULL == paSSelSafetyValve && NULL == paSSelAccessAndServices )
		{
			return false;
		}
		
		// Fill the 'paSSelSafetyValve' array with pointer on each 'CDS_SSelSafetyValve' object of the 'SAFETYVALVE_TAB' table.
		int iMaxIndex = 0;
		int i = 0;
		int iMaxIndexAccServ = 0;
		int j = 0;

		for( IDPTR IDPtr = m_pclSelectionTable->GetFirst(); '\0' != *IDPtr.ID; IDPtr = m_pclSelectionTable->GetNext() )
		{
			// Sanity tests.
			CDS_SSelSafetyValve* pSel = dynamic_cast<CDS_SSelSafetyValve *>( IDPtr.MP );
			CDS_SelPWQAccServices* pSelAccService = dynamic_cast<CDS_SelPWQAccServices*>( IDPtr.MP );

			if( NULL != pSel && NULL != pSel->GetIDPtr().MP )
			{
				paSSelSafetyValve[i] = (CDS_SSelSafetyValve*)(void*)IDPtr.MP;

				if( iMaxIndex < paSSelSafetyValve[i]->GetpSelectedInfos()->GetRowIndex() )
				{
					iMaxIndex = paSSelSafetyValve[i]->GetpSelectedInfos()->GetRowIndex();
				}

				++i;
			}

			if( NULL != pSelAccService && NULL != pSelAccService->GetIDPtr().MP )
			{
				paSSelAccessAndServices[j] = (CDS_SelPWQAccServices*)(void*)IDPtr.MP;

				if( iMaxIndexAccServ < paSSelAccessAndServices[j]->GetpSelectedInfos()->GetRowIndex() )
				{
					iMaxIndexAccServ = paSSelAccessAndServices[j]->GetpSelectedInfos()->GetRowIndex();
				}

				++j;
			}
		}
		
		ASSERT( i == iSafetyValveCount && j == iAccessAndServicesCount );
		iSafetyValveCount = i;
		iAccessAndServicesCount = j;
		
		// Sort 'paSSelSafetyValve' array.
		// Remark: 'fResetOrder' is set to 'true' only when the sorting combos are empty (No user choice) and the user clicks on the 'Apply sorting keys'.
		if( true == bResetOrder ) 
		{
			// Reset row index.
			for( i = 0; i < iSafetyValveCount; i++ )
			{
				if( 0 == paSSelSafetyValve[i]->GetpSelectedInfos()->GetRowIndex() )
				{
					paSSelSafetyValve[i]->GetpSelectedInfos()->SetRowIndex( ++iMaxIndex );
				}
			}

			for( j = 0; j < iAccessAndServicesCount; j++ )
			{
				if( 0 == paSSelAccessAndServices[i]->GetpSelectedInfos()->GetRowIndex() )
				{
					paSSelAccessAndServices[i]->GetpSelectedInfos()->SetRowIndex( ++iMaxIndexAccServ );
				}
			}
		}
		else if( PageField_enum::epfNONE != m_pTADSPageSetup->GetKey( 0 ) || PageField_enum::epfNONE != m_pTADSPageSetup->GetKey( 1 )
				|| PageField_enum::epfNONE != m_pTADSPageSetup->GetKey( 2 ) )
		{
			if( NULL != pDlgLeftTabSelP )
			{
				pDlgLeftTabSelP->SortTable( (CData**)paSSelSafetyValve, iSafetyValveCount - 1 );
			}

			// Reset row index.
			for( i = 0; i < iSafetyValveCount; i++ )
			{
				paSSelSafetyValve[i]->GetpSelectedInfos()->SetRowIndex( i + 1 );
			}
		}

		// Create a remark index.
		// HYS-1741: One remark index for all product in this page.
		int iRemIndex = -1;
		if( NULL != pDlgLeftTabSelP )
		{
			iRemIndex = pDlgLeftTabSelP->SetRemarkIndex( CLASS( CDS_SSelSafetyValve ), (CData**)paSSelSafetyValve, iSafetyValveCount );
		}

		// Remark: - from individual selection we can have safety valve alone or with blow tank.
		//         - from direct selection we can have either safety valve or blow tank alone.
		CRank rkl;
		enum eAirVSepSwitch
		{
			SingleSelSafetyValve,
			DirSelSafetyValve,
			LastSafetyValveSwitch
		};

		bool bAtLeastOneBlockPrinted = false;

		for( int iLoopGroup = 0; iLoopGroup < LastSafetyValveSwitch; iLoopGroup++ )
		{
			int iCount = 0;

			for( i = 0; i < iSafetyValveCount; i++ )
			{
				bool bShouldbeAdded = false;
				CDS_SSelSafetyValve *pclSelSafetyValve = dynamic_cast<CDS_SSelSafetyValve *>( paSSelSafetyValve[i] );
				
				if( NULL == pclSelSafetyValve || ( NULL == pclSelSafetyValve->GetProductIDPtr().MP && NULL == pclSelSafetyValve->GetBlowTankIDPtr().MP ) )
				{
					ASSERT( 0 );
					continue;
				}

				if( DirSelSafetyValve == iLoopGroup )
				{
					if( false == pclSelSafetyValve->IsFromDirSel() )
					{
						continue;
					}
				}
				else
				{
					if( true == pclSelSafetyValve->IsFromDirSel() )
					{
						continue;
					}
				}

				switch( iLoopGroup )
				{
					case SingleSelSafetyValve:

						if( NULL != dynamic_cast<CDB_SafetyValveBase *>( pclSelSafetyValve->GetProductIDPtr().MP ) )
						{
							bShouldbeAdded = true;
						}

						break;

					case DirSelSafetyValve:

						if( NULL != dynamic_cast<CDB_SafetyValveBase *>( pclSelSafetyValve->GetProductIDPtr().MP )
								|| NULL != dynamic_cast<CDB_BlowTank *>( pclSelSafetyValve->GetBlowTankIDPtr().MP ) )
						{
							bShouldbeAdded = true;
						}

						break;
				}

				if( false == bShouldbeAdded )
				{
					continue;
				}
				
				rkl.Add( _T(""), paSSelSafetyValve[i]->GetpSelectedInfos()->GetRowIndex(), (LPARAM)paSSelSafetyValve[i] );
				iCount++;
			}

			if( 0 == iCount )
			{
				continue;
			}

			if( i > 0 && true == bAtLeastOneBlockPrinted )
			{
				lRow++;
			}

			// Set the subtitle.
			SetLastRow( pclSDSafetyValve, lRow );

			switch( iLoopGroup )
			{
				case SingleSelSafetyValve:
					SetPageTitle( pclSDSafetyValve, IDS_SSHEETSELPROD_SUBTITLESAFETYVALVEFROMSSEL, false, lRow );
					break;

				case DirSelSafetyValve:
					SetPageTitle( pclSDSafetyValve, IDS_SSHEETSELPROD_SUBTITLESAFETYVALVEFROMDIRSEL, false, lRow );
					break;
			}

			lRow = pclSheet->GetMaxRows();
			SetLastRow( pclSDSafetyValve, lRow );
			CString str;
			LPARAM lparam;
			bool bFirstPass = true;

			for( BOOL bContinue = rkl.GetFirst( str, lparam ); TRUE == bContinue; bContinue = rkl.GetNext( str, lparam ) )
			{
				CData *pclData = (CData *)lparam;
				CDS_SSelSafetyValve *pclSSelSafetyValve = dynamic_cast<CDS_SSelSafetyValve *>( pclData );

				if( NULL == pclSSelSafetyValve )
				{
					continue;
				}

				if( m_pSelected == pclData )
				{
					lSelectedRow = lRow;
				}

				CDB_SafetyValveBase *pclSafetyValve = pclSSelSafetyValve->GetProductAs<CDB_SafetyValveBase>();
				CDB_BlowTank *pclBlowTank = dynamic_cast<CDB_BlowTank *>( pclSSelSafetyValve->GetBlowTankIDPtr().MP );

				if( NULL == pclSafetyValve && NULL == pclBlowTank )
				{
					continue;
				}

				m_rProductParam.Clear();
				m_rProductParam.SetSheetDescription( pclSDSafetyValve );
				m_rProductParam.SetSelectionContainer( (LPARAM)pclSSelSafetyValve );

				++lRow;
				long lFirstRow = lRow;

				long lRowProduct;
				CData *pclProductToSave = NULL;

				// If safety valve AND blow tank exist, we display here the safety valve, blow tank will follow below.
				// If safety valve or blow tank is alone, we display it here and nothing after.
				if( NULL != pclSafetyValve || NULL != pclBlowTank )
				{
					if( NULL != pclSafetyValve )
					{
						lRowProduct = _FillRowSafetyValve( pclSDSafetyValve, lRow, pclSSelSafetyValve, pclSafetyValve );
						pclProductToSave = pclSafetyValve;
				
						long lRowGen = CSelProdPageBase::FillRowGen( pclSDSafetyValve, lRow, pclSSelSafetyValve );
						lRow = max( lRowGen, lRowProduct );

						_FillWaterMedium( pclSDSafetyValve, lFirstRow, pclSafetyValve->GetMediumName(), pclSafetyValve->GetTmin(), pclSafetyValve->GetTmax() );
					}
					else
					{
						lRowProduct = _FillRowBlowTank( pclSDSafetyValve, lRow, pclSSelSafetyValve );
						pclProductToSave = pclBlowTank;
				
						long lRowGen = CSelProdPageBase::FillRowGen( pclSDSafetyValve, lRow, pclSSelSafetyValve );
						lRow = max( lRowGen, lRowProduct );

						_FillWaterMedium( pclSDSafetyValve, lFirstRow, pclBlowTank->GetMediumName(), pclBlowTank->GetTmin(), pclBlowTank->GetTmax() );
					}

					// Add the selectable row range.
					m_rProductParam.AddRange( lFirstRow, lRow, pclProductToSave );

					// Spanning must be done here because it's the only place where we know exactly the number of lines to span!
					// Span reference #1 and #2.
					AddCellSpanW( pclSDSafetyValve, ColumnDescription::Reference1, lFirstRow, 1, pclSheet->GetMaxRows() - lFirstRow + 1 );
					AddCellSpanW( pclSDSafetyValve, ColumnDescription::Reference2, lFirstRow, 1, pclSheet->GetMaxRows() - lFirstRow + 1 );

					// Span water characteristic.
					AddCellSpanW( pclSDSafetyValve, ColumnDescription::Water, lFirstRow, 1, pclSheet->GetMaxRows() - lFirstRow + 1 );

					// Add accessories.
					if( NULL != pclSafetyValve )
					{
						if( true == pclSSelSafetyValve->IsAccessoryExist() )
						{
							// Draw dash line.
							pclSheet->SetCellBorder( ColumnDescription::Reference1, lRow, ColumnDescription::Footer - 1, lRow, true, SS_BORDERTYPE_BOTTOM, SS_BORDERSTYLE_FINE_DASH );

							lRow++;
							lRow = FillAccessories( pclSDSafetyValve, lRow, pclSSelSafetyValve, pclSSelSafetyValve->GetpSelectedInfos()->GetQuantity() );
						}
					}
					else
					{
						if( true == pclSSelSafetyValve->IsBlowTankAccessoryExist() )
						{
							// Draw dash line.
							pclSheet->SetCellBorder( ColumnDescription::Reference1, lRow, ColumnDescription::Footer - 1, lRow, true, SS_BORDERTYPE_BOTTOM, SS_BORDERSTYLE_FINE_DASH );

							lRow++;
							lRow = FillAccessories( pclSDSafetyValve, lRow, pclSSelSafetyValve->GetBlowTankAccessoryList(), false );
						}
					}
				}

				if( NULL != pclSafetyValve && NULL != pclBlowTank )
				{
					// Draw dash line.
					pclSheet->SetCellBorder( ColumnDescription::Reference1, lRow, ColumnDescription::Footer - 1, lRow, true, SS_BORDERTYPE_BOTTOM, SS_BORDERSTYLE_FINE_DASH );

					lRow++;
					lRow = _FillRowBlowTank( pclSDSafetyValve, lRow, pclSSelSafetyValve );

					if( pclSSelSafetyValve->GetBlowTankAccessoryList()->GetCount() > 0 )
					{
						// Draw dash line.
						pclSheet->SetCellBorder( ColumnDescription::Reference1, lRow, ColumnDescription::Footer - 1, lRow, true, SS_BORDERTYPE_BOTTOM, SS_BORDERSTYLE_FINE_DASH );

						lRow++;
						lRow = FillAccessories( pclSDSafetyValve, lRow, pclSSelSafetyValve->GetBlowTankAccessoryList(), false );
					}
				}

				// Draw line below.
				pclSheet->SetCellBorder( ColumnDescription::Reference1, lRow, ColumnDescription::Footer - 1, lRow, true, SS_BORDERTYPE_BOTTOM );

				// Save the object pointer.
				m_rProductParam.SetScrollRange( ( true == bFirstPass ) ? lFirstRow - 1 : lFirstRow, lRow );
				SaveProduct( m_rProductParam );

				// Set all group as no breakable (for print).
				// Remark: include title with the group.
				pclSheet->SetFlagRowNoBreakable( ( true == bFirstPass ) ? lFirstRow - 1 : lFirstRow, lRow, true );

				bFirstPass = false;
			}

			lRow++;
			bAtLeastOneBlockPrinted = true;
		}

		// HYS-1741: Add P&WQ Accessories and services products
		if( true == InitDSAccessoriesAndService( pclSDSafetyValve, paSSelAccessAndServices, iAccessAndServicesCount, iMaxIndexAccServ, bResetOrder, iRemIndex ) )
		{
			lRow = pclSheet->GetMaxRows();
			lRow++;
		}

		FillRemarks( pclSDSafetyValve, lRow );

		// We add by default a blank line.
		lRow = pclSheet->GetMaxRows() + 2;
		
		delete[] paSSelSafetyValve;
		delete[] paSSelAccessAndServices;
	}

	if( true == bSafetyValveHMExist )
	{
		if( m_vecHMList.size() > 0 )	
		{
			SetLastRow( pclSDSafetyValve, lRow );
			SetPageTitle( pclSDSafetyValve, IDS_SSHEETSELPROD_SUBTITLESAFETYVALVEFROMHM, false, lRow );
			
			lRow++;
			SetLastRow( pclSDSafetyValve, lRow );
			
			// For each element.
			bool bFirstPass = true;
			HMvector::iterator It;

			for( It = m_vecHMList.begin(); It != m_vecHMList.end(); It++ )
			{
				CDS_HydroMod *pHM = NULL;
				CDS_HydroMod::CBase *pBase = NULL;

				if( eptHM == (*It).first )
				{
					pHM = dynamic_cast<CDS_HydroMod *>( (CData *)( (*It).second ) );
				}
				else
				{
					// Separator exists and TA product exist into the DB.
					pBase = static_cast<CDS_HydroMod::CBase *>( (*It).second );

					if( NULL != pBase )
					{
						pHM = pBase->GetpParentHM();
					}
				}

				if( NULL != pHM )
				{
					m_rProductParam.Clear();
					m_rProductParam.SetSheetDescription( pclSDSafetyValve );
					// Intentionally set to NULL to avoid clicking, double clicking or right clicking.
					m_rProductParam.SetSelectionContainer( (LPARAM)0 );

					long lRowSafetyValve = _FillRowSafetyValve( pclSDSafetyValve, lRow, pHM );
					long lRowGen = CSelProdPageBase::FillRowGen( pclSDSafetyValve, lRow, pHM, pBase );
					long lLastRow = max( lRowGen, lRowSafetyValve );

					CDB_TAProduct *pclTAProd = dynamic_cast<CDB_TAProduct *>(pHM->GetIDPtr().MP);
					lLastRow = FillAndAddBuiltInHMAccessories( pclSheet, pclSDSafetyValve, pclTAProd, lLastRow );

					// Save the product.
					m_rProductParam.SetScrollRange( ( true == bFirstPass ) ? lRow - 1 : lRow, lLastRow );
					SaveProduct( m_rProductParam );

					// Draw line below.
					pclSheet->SetCellBorder( ColumnDescription::Reference1, lLastRow, ColumnDescription::Footer - 1, lLastRow, true, SS_BORDERTYPE_BOTTOM );

					// Set all group as no breakable (for print).
					// Remark: include title with the group.
					pclSheet->SetFlagRowNoBreakable( ( true == bFirstPass ) ? lRow - 1 : lRow, lLastRow, true );

					lRow = lLastRow + 1;
					bFirstPass = false;
				}
			}

			FillHMRemarks( &m_vecHMList, pclSDSafetyValve, lRow );
		}
	}

	// Move sheet to correct position.
	SetSheetSize();
	Invalidate();
	UpdateWindow();

	// Verify if a product is selected.
	if( 0 == lSelectedRow )
	{
		m_pSelected = 0;
	}

	return true;
}

bool CSelProdPageSafetyValve::HasSomethingToDisplay( void )
{
	bool bSafetyValveExist = ( NULL != m_pclSelectionTable ) ? true : false;
	bool bSafetyValveHMExist = ( m_vecHMList.size() > 0 ) ? true : false;

	if( false == bSafetyValveExist && false == bSafetyValveHMExist )
	{
		return false;
	}

	return true;
}

void CSelProdPageSafetyValve::_InitColHeader( CSheetDescription* pclSheetDescription )
{
	if( NULL == pclSheetDescription || NULL == pclSheetDescription->GetSSheetPointer() )
	{
		return;
	}

	CSSheet *pclSheet = pclSheetDescription->GetSSheetPointer();
	CString str;
	
	// Format columns header.
	pclSheet->SetTextPattern( CSSheet::_SSTextPattern::ColumnHeader );
	pclSheet->SetTextPatternProperty( CSSheet::_SSTextPatternProperty::TPP_FontBackColor, (LPARAM)_TAH_WHITE );
	pclSheet->SetTextPatternProperty( CSSheet::_SSTextPatternProperty::TPP_StyleWordWrap, (LPARAM)TRUE );

	str = TASApp.LoadLocalizedString( IDS_SELPHDR_REF1 );
	pclSheet->SetStaticText( ColumnDescription::Reference1, SelProdHeaderRow::HR_RowHeader, str );
	str = TASApp.LoadLocalizedString( IDS_SELPHDR_REF2 );
	pclSheet->SetStaticText( ColumnDescription::Reference2, SelProdHeaderRow::HR_RowHeader, str );
	str = TASApp.LoadLocalizedString( IDS_SELPHDR_WATER );
	pclSheet->SetStaticText( ColumnDescription::Water, SelProdHeaderRow::HR_RowHeader, str );
	str = TASApp.LoadLocalizedString( IDS_SELPHDR_PRODUCT );
	pclSheet->SetStaticText( ColumnDescription::Product, SelProdHeaderRow::HR_RowHeader, str );
	str = TASApp.LoadLocalizedString( IDS_SELPHDR_TECH );
	pclSheet->SetStaticText( ColumnDescription::TechnicalInfos, SelProdHeaderRow::HR_RowHeader, str );
	str = TASApp.LoadLocalizedString( IDS_SELPHDR_ART );
	pclSheet->SetStaticText( ColumnDescription::ArticleNumber, SelProdHeaderRow::HR_RowHeader, str );
	str = TASApp.LoadLocalizedString( IDS_SELPHDR_PIPE );
	pclSheet->SetStaticText( ColumnDescription::Pipes, SelProdHeaderRow::HR_RowHeader, str );
	str = TASApp.LoadLocalizedString( IDS_SELPHDR_QTY );
	pclSheet->SetStaticText( ColumnDescription::Quantity, SelProdHeaderRow::HR_RowHeader, str );

	if( true == TASApp.IsPriceUsed() )
	{
		str = TASApp.LoadLocalizedString( IDS_SELPHDR_PRICE ) + _T("\r\n[");
		str += CString( m_pTADS->GetpTechParams()->GetCurrentCurrencyISO().c_str() ) + _T("]");
		pclSheet->SetStaticText( ColumnDescription::UnitPrice, SelProdHeaderRow::HR_RowHeader, str );

		str = TASApp.LoadLocalizedString( IDS_SELPHDR_TOTAL ) + _T("\r\n[");
		str += CString( m_pTADS->GetpTechParams()->GetCurrentCurrencyISO().c_str() ) + _T("]");
		pclSheet->SetStaticText( ColumnDescription::TotalPrice, SelProdHeaderRow::HR_RowHeader, str );
	}

	str = TASApp.LoadLocalizedString( IDS_SELPHDR_REM );
	pclSheet->SetStaticText( ColumnDescription::Remark, SelProdHeaderRow::HR_RowHeader, str );
	
	// Draw border.
	pclSheet->SetCellBorder( ColumnDescription::Reference1, SelProdHeaderRow::HR_RowHeader, ColumnDescription::Footer - 1, SelProdHeaderRow::HR_RowHeader, true, SS_BORDERTYPE_BOTTOM | SS_BORDERTYPE_TOP );
	
	// Freeze row header.
	pclSheet->SetFreeze( 0, SelProdHeaderRow::HR_RowHeader );
}

long CSelProdPageSafetyValve::_FillRowSafetyValve( CSheetDescription *pclSheetDescription, long lRow, CDS_SSelSafetyValve *pclSSelSafetyValve, CDB_SafetyValveBase *pclSafetyValve )
{
	if( NULL == pclSheetDescription || NULL == pclSheetDescription->GetSSheetPointer() || NULL == pclSSelSafetyValve || NULL == pclSafetyValve )
	{
		return lRow;
	}

	CSSheet *pclSheet = pclSheetDescription->GetSSheetPointer();
	CString str1, str2;
	long lFirstRow = lRow;

	pclSheet->SetTextPattern( CSSheet::_SSTextPattern::UserStaticColumn );
	pclSheet->SetTextPatternProperty( CSSheet::_SSTextPatternProperty::TPP_FontBold, (LPARAM)FALSE );

	// Add item into the full article list.
	_AddSafetyValveArticleList( pclSSelSafetyValve );
		
	// Column TA Product.
	_FillRowSafetyValveProduct( pclSheetDescription, lRow, pclSafetyValve, pclSSelSafetyValve->GetpSelectedInfos()->GetQuantity() * pclSSelSafetyValve->GetQuantityNeeded() );

	// Column Infos.
	if( false == pclSSelSafetyValve->IsFromDirSel() )
	{
		_FillRowSafetyValveInfo( pclSheetDescription, lFirstRow, pclSSelSafetyValve, pclSafetyValve );
	}

	return pclSheet->GetMaxRows();
}

long CSelProdPageSafetyValve::_FillRowSafetyValve( CSheetDescription* pclSheetDescription, long lRow, CDS_HydroMod *pclHM )
{
	if( NULL == pclSheetDescription || NULL == pclSheetDescription->GetSSheetPointer() || NULL == pclHM ) 
	{
		return lRow;
	}

	CSSheet *pclSheet = pclSheetDescription->GetSSheetPointer();
	CString str1, str2;
	long lFirstRow = lRow;

	// TO DO !!
	double dQ = 0.0;
	CDB_SafetyValveBase* pclSafetyValve = NULL;

	if( NULL == pclSafetyValve )
	{
		return pclSheet->GetMaxRows();
	}
	
	pclSheet->SetTextPattern( CSSheet::_SSTextPattern::UserStaticColumn );
	pclSheet->SetTextPatternProperty( CSSheet::_SSTextPatternProperty::TPP_FontBold, (LPARAM)FALSE );

	// Add item into the full article list.
	_AddArticleList( pclSafetyValve, 1 );

	// Column TA Product.
	_FillRowSafetyValveProduct( pclSheetDescription, lRow, pclSafetyValve, 1 );

	// Column Infos.
	_FillRowSafetyValveInfoFromHM( pclSheetDescription, lFirstRow, pclHM, pclSafetyValve );
	
	return pclSheet->GetMaxRows();
}

long CSelProdPageSafetyValve::_FillRowSafetyValveProduct( CSheetDescription *pclSheetDescription, long lRow, CDB_SafetyValveBase *pclSafetyValve, int iQuantity )
{
	if( NULL == pclSheetDescription || NULL == pclSafetyValve || NULL == TASApp.GetpTADS() || NULL == TASApp.GetpTADS()->GetpTechParams() )
	{
		return lRow;
	}

	CString str1, str2;
	long lFirstRow = lRow;
	
	// Column TA Product.
	AddStaticText( pclSheetDescription, ColumnDescription::Product, lRow, _T("-") );

	if( true == m_pTADSPageSetup->GetField( epfSAFETYVALVEINFOPRODUCTNAME ) )	
	{
		lRow = AddStaticText( pclSheetDescription, ColumnDescription::Product, lRow, pclSafetyValve->GetName() );
	}
	
	if( true == m_pTADSPageSetup->GetField( epfSAFETYVALVEINFOPRODUCTSIZE ) )	
	{
		lRow = AddStaticText( pclSheetDescription, ColumnDescription::Product, lRow, pclSafetyValve->GetSize() );
	}

	if( true == m_pTADSPageSetup->GetField( epfSAFETYVALVEINFOPRODUCTCONNECTION ) )	
	{
		lRow = AddStaticText( pclSheetDescription, ColumnDescription::Product, lRow, pclSafetyValve->GetInOutletConnectionSizeString() );
	}
	
	// Column 'Quantity'.
	FillQtyPriceCol( pclSheetDescription, lFirstRow, pclSafetyValve, iQuantity );
	
	// Column 'Article number'.
	AddStaticText( pclSheetDescription, ColumnDescription::ArticleNumber, lFirstRow, _T("-") );

	// Remark: 'true' to exclude any connection reference in the article number.
	FillArtNumberCol( pclSheetDescription, ColumnDescription::ArticleNumber, lFirstRow, pclSafetyValve, pclSafetyValve->GetArtNum( true ) );
	return lRow;
}

long CSelProdPageSafetyValve::_FillRowSafetyValveInfo( CSheetDescription *pclSheetDescription, long lRow, CDS_SSelSafetyValve *pclSSelSafetyValve, CDB_SafetyValveBase *pclSafetyValve )
{
	if( NULL == pclSheetDescription || NULL == pclSafetyValve || NULL == TASApp.GetpTADS() || NULL == TASApp.GetpTADS()->GetpTechParams() )
	{
		return lRow;
	}

	// Column Infos.
	CString str;
	AddStaticText( pclSheetDescription, ColumnDescription::TechnicalInfos, lRow, _T("-") );			// Default string

	if( true == m_pTADSPageSetup->GetField( epfSAFETYVALVETECHINFOSETPRESSURE ) )
	{
		str = TASApp.LoadLocalizedString( IDS_SSHEETSELPROD_SAFETYVALVE_SETPRESSURE );

		// Pay attention: here it's well the set pressure chosen by the user.
		str += CString( _T(": ") ) + WriteCUDouble( _U_PRESSURE, pclSSelSafetyValve->GetSetPressure(), true );
		lRow = AddStaticText( pclSheetDescription, ColumnDescription::TechnicalInfos, lRow, str );
	}

	if( ProjectType::Heating == pclSSelSafetyValve->GetpSelectedInfos()->GetApplicationType() 
			&& true == m_pTADSPageSetup->GetField( epfSAFETYVALVETECHINFOHEATGENERATORTYPE ) )
	{
		CDS_TechnicalParameter *pTechP = TASApp.GetpTADS()->GetpTechParams();
		CString str = TASApp.LoadLocalizedString( pTechP->GetProductSelectionApplicationTypeIDS( Heating ) );
		str += _T(", ");

		CDB_StringID *pStrID = dynamic_cast<CDB_StringID *>( TASApp.GetpTADB()->Get( pclSSelSafetyValve->GetSystemHeatGeneratorTypeID() ).MP );

		if( NULL == pStrID )
		{
			return lRow;
		}

		str += pStrID->GetString();
		lRow = AddStaticText( pclSheetDescription, ColumnDescription::TechnicalInfos, lRow, str );
	}

	if( true == m_pTADSPageSetup->GetField( epfSAFETYVALVETECHINFOPOWERMAX ) )
	{
		double dPowerLimit = pclSafetyValve->GetPowerLimit( pclSSelSafetyValve->GetSystemHeatGeneratorTypeID(), pclSSelSafetyValve->GetNormID() );
		str = TASApp.LoadLocalizedString( IDS_SSHEETSELPROD_SAFETYVALVE_POWERMAX );
		str += CString( _T(": ") ) + WriteCUDouble( _U_TH_POWER, dPowerLimit, true );
		lRow = AddStaticText( pclSheetDescription, ColumnDescription::TechnicalInfos, lRow, str );
	}

	if( true == m_pTADSPageSetup->GetField( epfSAFETYVALVETECHINFOPOWERSYSTEM ) )
	{
		str = TASApp.LoadLocalizedString( IDS_SSHEETSELPROD_SAFETYVALVE_POWERSYSTEM );
		str += CString( _T(": ") ) + WriteCUDouble( _U_TH_POWER, pclSSelSafetyValve->GetInstalledPower(), true );
		lRow = AddStaticText( pclSheetDescription, ColumnDescription::TechnicalInfos, lRow, str );
	}

	if( ProjectType::Solar == pclSSelSafetyValve->GetpSelectedInfos()->GetApplicationType()
		&& true == m_pTADSPageSetup->GetField( epfSAFETYVALVETECHINFOCOLLECTORMAX ) )
	{
		CDB_SafetyValveSolar *pclSafetyValveSolar = (CDB_SafetyValveSolar *)pclSafetyValve;
		str = TASApp.LoadLocalizedString( IDS_SSHEETSELPROD_SAFETYVALVE_COLLECTORMAX );
		str += CString( _T(": ") ) + WriteCUDouble( _U_AREA, pclSafetyValveSolar->GetCollectorLimit(), true );
		lRow = AddStaticText( pclSheetDescription, ColumnDescription::TechnicalInfos, lRow, str );
	}

	if( ProjectType::Solar == pclSSelSafetyValve->GetpSelectedInfos()->GetApplicationType()
		&& true == m_pTADSPageSetup->GetField( epfSAFETYVALVETECHINFOCOLLECTOR ) )
	{
		str = TASApp.LoadLocalizedString( IDS_SSHEETSELPROD_SAFETYVALVE_COLLECTOR );
		str += CString( _T(": ") ) + WriteCUDouble( _U_AREA, pclSSelSafetyValve->GetInstalledCollector(), true );
		lRow = AddStaticText( pclSheetDescription, ColumnDescription::TechnicalInfos, lRow, str );
	}

	return lRow;
}

long CSelProdPageSafetyValve::_FillRowSafetyValveInfoFromHM( CSheetDescription *pclSheetDescription, long lRow, CDS_HydroMod *pHM, CDB_SafetyValveBase* pclSafetyValve )
{
	// TODO.
	return lRow;
}

long CSelProdPageSafetyValve::_FillWaterMedium( CSheetDescription *pclSheetDescription, long lRow, CString strMediumName, double dTmin, double dTmax )
{
	if( NULL == pclSheetDescription || NULL == pclSheetDescription->GetSSheetPointer() )
	{
		return lRow;
	}

	CString strTMin = WriteCUDouble( _U_TEMPERATURE, dTmin );
	CString strTMax = WriteCUDouble( _U_TEMPERATURE, dTmax, true );
	CString strFull = strMediumName + _T("(" ) + strTMin + _T( " / " ) + strTMax + _T( ")");

	long lPrev = pclSheetDescription->GetSSheetPointer()->GetTextPatternProperty( CSSheet::_SSTextPatternProperty::TPP_StyleWordWrap );
	pclSheetDescription->GetSSheetPointer()->SetTextPatternProperty( CSSheet::_SSTextPatternProperty::TPP_StyleWordWrap, (LPARAM)TRUE );
	lRow = AddStaticText( pclSheetDescription, ColumnDescription::Water, lRow, strFull );
	pclSheetDescription->GetSSheetPointer()->SetTextPatternProperty( CSSheet::_SSTextPatternProperty::TPP_StyleWordWrap, (LPARAM)lPrev );

	return lRow;
}

long CSelProdPageSafetyValve::_FillRowBlowTank( CSheetDescription *pclSheetDescription, long lRow, CDS_SSelSafetyValve *pclSSelSafetyValve )
{
	if( NULL == pclSheetDescription || NULL == pclSheetDescription->GetSSheetPointer() || NULL == pclSSelSafetyValve 
			|| NULL == dynamic_cast<CDB_BlowTank *>( pclSSelSafetyValve->GetBlowTankIDPtr().MP ) )
	{
		return lRow;
	}

	CSSheet *pclSheet = pclSheetDescription->GetSSheetPointer();
	CString str1, str2;
	long lFirstRow = lRow;

	pclSheet->SetTextPattern( CSSheet::_SSTextPattern::UserStaticColumn );
	pclSheet->SetTextPatternProperty( CSSheet::_SSTextPatternProperty::TPP_FontBold, (LPARAM)FALSE );

	// Add item into the full article list.
	_AddBlowTankArticleList( pclSSelSafetyValve );
		
	// Column TA Product.
	CDB_BlowTank *pclBlowTank = dynamic_cast<CDB_BlowTank *>( pclSSelSafetyValve->GetBlowTankIDPtr().MP );
	lRow = _FillRowBlowTankProduct( pclSheetDescription, lRow, pclBlowTank, pclSSelSafetyValve->GetpSelectedInfos()->GetQuantity() * pclSSelSafetyValve->GetQuantityNeeded() );

	// Column Infos.
	if( false == pclSSelSafetyValve->IsFromDirSel() )
	{
		_FillRowBlowTankInfo( pclSheetDescription, lFirstRow, pclBlowTank );
	}

	m_rProductParam.AddRange( lFirstRow, pclSheet->GetMaxRows(), pclBlowTank );
	return pclSheet->GetMaxRows();
}

long CSelProdPageSafetyValve::_FillRowBlowTankProduct( CSheetDescription *pclSheetDescription, long lRow, CDB_BlowTank *pclBlowTank, int iQuantity )
{
	if( NULL == pclSheetDescription || NULL == pclBlowTank || NULL == TASApp.GetpTADS() || NULL == TASApp.GetpTADS()->GetpTechParams() )
	{
		return lRow;
	}

	CString str1, str2;
	long lFirstRow = lRow;
	
	// Column TA Product.
	AddStaticText( pclSheetDescription, ColumnDescription::Product, lRow, _T("-") );

	if( true == m_pTADSPageSetup->GetField( epfBLOWTANKINFOPRODUCTNAME ) )	
	{
		lRow = AddStaticText( pclSheetDescription, ColumnDescription::Product, lRow, pclBlowTank->GetName() );
	}
	
	if( true == m_pTADSPageSetup->GetField( epfBLOWTANKINFOPRODUCTSIZE ) )	
	{
		lRow = AddStaticText( pclSheetDescription, ColumnDescription::Product, lRow, pclBlowTank->GetAllSizesString() );
	}

	// Column 'Quantity'.
	FillQtyPriceCol( pclSheetDescription, lFirstRow, pclBlowTank, iQuantity );
	
	// Column 'Article number'.
	AddStaticText( pclSheetDescription, ColumnDescription::ArticleNumber, lFirstRow, _T("-") );

	// Remark: 'true' to exclude any connection reference in the article number.
	FillArtNumberCol( pclSheetDescription, ColumnDescription::ArticleNumber, lFirstRow, pclBlowTank, pclBlowTank->GetArtNum( true ) );
	return lRow;
}

long CSelProdPageSafetyValve::_FillRowBlowTankInfo( CSheetDescription* pclSheetDescription, long lRow, CDB_BlowTank *pclBlowTank )
{
	if( NULL == pclSheetDescription || NULL == pclBlowTank || NULL == TASApp.GetpTADS() || NULL == TASApp.GetpTADS()->GetpTechParams() )
	{
		return lRow;
	}

	// Column Infos.
	CString str;
	AddStaticText( pclSheetDescription, ColumnDescription::TechnicalInfos, lRow, _T("-") );			// Default string

	if( true == m_pTADSPageSetup->GetField( epfBLOWTANKTECHINFOPS ) )
	{
		str = TASApp.LoadLocalizedString( IDS_SSHEETSELPROD_BLOWTANK_PS );
		str += CString( _T(": ") ) + WriteCUDouble( _U_PRESSURE, pclBlowTank->GetPmaxmax(), true );
		lRow = AddStaticText( pclSheetDescription, ColumnDescription::TechnicalInfos, lRow, str );
	}

	return lRow;
}

long CSelProdPageSafetyValve::_FillAccessoriesServicesAlone( CSheetDescription* pclSheetDescription, long lRow, CDS_SelPWQAccServices* pclSSelAccessAndServices )
{
	if( NULL == pclSheetDescription || NULL == pclSheetDescription->GetSSheetPointer() || NULL == pclSSelAccessAndServices 
		|| NULL == pclSSelAccessAndServices->GetProductIDPtr().MP )
	{
		return lRow;
	}

	CDB_PWQPressureReducer* pclPressureReducer = dynamic_cast<CDB_PWQPressureReducer*>((CData*)(pclSSelAccessAndServices->GetProductIDPtr().MP));

	if( NULL == pclPressureReducer )
	{
		return lRow;
	}

	// First: write the common part.
	long lFirstRow = lRow;
	CSSheet* pclSheet = pclSheetDescription->GetSSheetPointer();
	CString str;
	pclSheet->SetTextPattern( CSSheet::_SSTextPattern::UserStaticColumn );
	pclSheet->SetTextPatternProperty( CSSheet::_SSTextPatternProperty::TPP_FontBold, (LPARAM)FALSE );

	// Set font color to red when product is deleted or not available.
	if( true == pclPressureReducer->IsDeleted() || false == pclPressureReducer->IsAvailable() )
	{
		pclSheet->SetTextPatternProperty( CSSheet::_SSTextPatternProperty::TPP_FontForeColor, (LPARAM)_RED );
	}

	// Check first if we have a local article number available.
	CString strLocalArticleNumber = _T( "" );

	if( true == TASApp.IsLocalArtNumberUsed() )
	{
		strLocalArticleNumber = LocArtNumTab.GetLocalArticleNumber( pclPressureReducer->GetArtNum() );
		strLocalArticleNumber.Trim();
	}

	// Fill article number.
	bool bArticleNumberShown = false;

	// If we can show article number OR we can't show but there is no local article number...
	if( false == TASApp.IsTAArtNumberHidden() || (true == TASApp.IsTAArtNumberHidden() && true == strLocalArticleNumber.IsEmpty()) )
	{
		CString str = pclPressureReducer->GetArtNum();

		if( false == str.IsEmpty() )
		{
			AddStaticText( pclSheetDescription, ColumnDescription::ArticleNumber, lRow, pclPressureReducer->GetArtNum() );
			bArticleNumberShown = true;
		}
	}

	// Set local article number if allowed and exist.
	if( true == TASApp.IsLocalArtNumberUsed() && false == strLocalArticleNumber.IsEmpty() )
	{
		long lNextRow = lRow + ((true == bArticleNumberShown) ? 1 : 0);
		CString strPrefixLAN = TASApp.GetLocalArtNumberName();

		if( false == strPrefixLAN.IsEmpty() )
		{
			lNextRow = AddStaticText( pclSheetDescription, ColumnDescription::ArticleNumber, lNextRow, strPrefixLAN );
		}

		AddStaticText( pclSheetDescription, ColumnDescription::ArticleNumber, lNextRow, strLocalArticleNumber );
	}

	long lFlagRow = pclSheet->GetMaxRows() + 1;

	if( true == pclPressureReducer->IsDeleted() )
	{
		AddStaticText( pclSheetDescription, ColumnDescription::ArticleNumber, lFlagRow, TASApp.LoadLocalizedString( IDS_TAPDELETED ) );
	}
	else if( false == pclPressureReducer->IsAvailable() )
	{
		AddStaticText( pclSheetDescription, ColumnDescription::ArticleNumber, lFlagRow, TASApp.LoadLocalizedString( IDS_TAPNOTAVAILABLE ) );
	}

	pclSheet->SetTextPatternProperty( CSSheet::_SSTextPatternProperty::TPP_FontForeColor, (LPARAM)_BLACK );

	// Write nothing for technical info.
	AddStaticText( pclSheetDescription, ColumnDescription::TechnicalInfos, lRow, _T( "-" ) );

	// Write quantity and prices.
	FillQtyPriceCol( pclSheetDescription, lRow, pclPressureReducer, pclSSelAccessAndServices->GetpSelectedInfos()->GetQuantity() );

	// Write name.
	if( true == m_pTADSPageSetup->GetField( epfPRESSUREREDUCVALVEINFOPRODUCTNAME ) )
	{
		AddStaticText( pclSheetDescription, ColumnDescription::Product, lRow, pclPressureReducer->GetName() );
	}

	// Write references and remarks
	lRow = FillRowGen( pclSheetDescription, lRow, pclSSelAccessAndServices );

	// Write material.
	lRow++;
	if( true == m_pTADSPageSetup->GetField( epfPRESSUREREDUCVALVEINFOPRODUCTBDYMATERIAL ) )
	{
		str = pclPressureReducer->GetBodyMaterial();
		lRow = AddStaticText( pclSheetDescription, ColumnDescription::Product, lRow, str );
	}

	// Write connections.
	if( true == m_pTADSPageSetup->GetField( epfPRESSUREREDUCVALVEINFOPRODUCTCONNECTION ) )
	{
		str = pclPressureReducer->GetConnect();
		lRow = AddStaticText( pclSheetDescription, ColumnDescription::Product, lRow, str );
	}

	// Write version
	if( ( true == m_pTADSPageSetup->GetField( epfPRESSUREREDUCVALVEINFOPRODUCTVERSION ) )
		&& ( false == CString( pclPressureReducer->GetVersion() ).IsEmpty() ) )
	{
		str = pclPressureReducer->GetVersion();
		lRow = AddStaticText( pclSheetDescription, ColumnDescription::Product, lRow, str );
	}

	// Write PN and temperature range.
	if( true == m_pTADSPageSetup->GetField( epfPRESSUREREDUCVALVEINFOPRODUCTPN ) )
	{
		lRow = FillPNTminTmax( pclSheetDescription, ColumnDescription::Product, lRow, pclPressureReducer );
	}

	// Pressure.
	if( true == m_pTADSPageSetup->GetField( epfPRESSUREREDUCVALVEINFOPRODUCTINLETPRESS ) )
	{
		str = TASApp.LoadLocalizedString( IDS_PR_INLETPRESSURE );
		str += CString( _T( " " ) ) + WriteCUDouble( _U_PRESSURE, pclPressureReducer->GetMaxInletPressure(), true );

		lRow = AddStaticText( pclSheetDescription, ColumnDescription::Product, lRow, str );
	}

	if( true == m_pTADSPageSetup->GetField( epfPRESSUREREDUCVALVEINFOPRODUCTOUTLETPRESS ) )
	{
		str = TASApp.LoadLocalizedString( IDS_PR_OUTLETPRESSURE );
		str += CString( _T( " " ) ) + pclPressureReducer->GetOutletPressureRange();

		lRow = AddStaticText( pclSheetDescription, ColumnDescription::Product, lRow, str );
	}
	// Center the text below.
	pclSheet->SetTextPatternProperty( CSSheet::_SSTextPatternProperty::TPP_StyleHorizontalAlign, (LPARAM)SSS_ALIGN_CENTER );

	m_rProductParam.AddRange( lFirstRow, pclSheet->GetMaxRows(), pclPressureReducer );

	return pclSheet->GetMaxRows();
}

void CSelProdPageSafetyValve::_AddArticleList( CDB_TAProduct *pclTAP, int iQuantity )
{
	if( NULL == pclTAP )
	{
		return;
	}

	CArticleGroup *pclArticleGroup = new CArticleGroup();
	
	if( NULL == pclArticleGroup )
	{
		return;
	}

	pclArticleGroup->AddArticle( pclTAP, iQuantity );
	CSelProdPageBase::AddArticleInGroupList( pclArticleGroup );

	// Remark: pay attention that 'CPage::AddArticleInGroupList' will increment quantity if article already exist or
	//         will create a new 'CArticleGroup' and will copy to it data passed as argument. This is the reason
	//         why you need to delete 'pclArticleGroup' after the call.
	delete pclArticleGroup;

	// Add connection component if needed.
	if( true == pclTAP->IsConnTabUsed() )
	{
		// Inlet.
		CSelProdPageBase::AddCompArtList( pclTAP, iQuantity, true );

		// Outlet.
		CSelProdPageBase::AddCompArtList( pclTAP, iQuantity, false );
	}
}

void CSelProdPageSafetyValve::_AddSafetyValveArticleList( CDS_SSelSafetyValve *pclSSelSafetyValve )
{
	if( NULL == pclSSelSafetyValve )
	{
		return;
	}

	CDB_TAProduct *pTAP = pclSSelSafetyValve->GetProductAs<CDB_TAProduct>();

	if( NULL == pTAP )
	{
		return;
	}

	int iQuantity = pclSSelSafetyValve->GetpSelectedInfos()->GetQuantity() * pclSSelSafetyValve->GetQuantityNeeded();
	CArticleGroup *pclArticleGroup = new CArticleGroup();
	ASSERT( NULL != pclArticleGroup );
	
	if( NULL == pclArticleGroup )
	{
		return;
	}

	CArticleContainer *pclArticleContainer = pclArticleGroup->AddArticle( pTAP, iQuantity );

	AddAccessoriesInArticleContainer( pclSSelSafetyValve->GetAccessoryList(), pclArticleContainer, iQuantity, false );

	CSelProdPageBase::AddArticleInGroupList( pclArticleGroup );

	// Remark: pay attention that 'CPage::AddArticleList' will increment quantity if article already exist or
	//         will create a new 'CArticleContainer' and will copy to it data passed as argument. This is the reason
	//         why you need to delete 'pclArticleContainer' after the call.
	delete pclArticleGroup;

	// Add connection component if needed.
	if( true == pTAP->IsConnTabUsed() )
	{
		// Inlet.
		CSelProdPageBase::AddCompArtList( pTAP, iQuantity, true );

		// Outlet.
		CSelProdPageBase::AddCompArtList( pTAP, iQuantity, false );
	}
}

void CSelProdPageSafetyValve::_AddBlowTankArticleList( CDS_SSelSafetyValve *pclSSelSafetyValve )
{
	if( NULL == pclSSelSafetyValve || NULL == dynamic_cast<CDB_BlowTank *>( pclSSelSafetyValve->GetBlowTankIDPtr().MP ) )
	{
		return;
	}

	CDB_BlowTank *pclBlowTank = dynamic_cast<CDB_BlowTank *>( pclSSelSafetyValve->GetBlowTankIDPtr().MP );

	int iQuantity = pclSSelSafetyValve->GetpSelectedInfos()->GetQuantity() * pclSSelSafetyValve->GetQuantityNeeded();
	CArticleGroup *pclArticleGroup = new CArticleGroup();
	ASSERT( NULL != pclArticleGroup );
	
	if( NULL == pclArticleGroup )
	{
		return;
	}

	CArticleContainer *pclArticleContainer = pclArticleGroup->AddArticle( pclBlowTank, iQuantity );

	AddAccessoriesInArticleContainer( pclSSelSafetyValve->GetBlowTankAccessoryList(), pclArticleContainer, iQuantity, false );

	CSelProdPageBase::AddArticleInGroupList( pclArticleGroup );

	// Remark: pay attention that 'CPage::AddArticleList' will increment quantity if article already exist or
	//         will create a new 'CArticleContainer' and will copy to it data passed as argument. This is the reason
	//         why you need to delete 'pclArticleContainer' after the call.
	delete pclArticleGroup;
}

bool CSelProdPageSafetyValve::InitDSAccessoriesAndService( CSheetDescription* pclSheetDescription, CDS_SelPWQAccServices** paSSelAccessAndServices, 
														   int iAccessAndServicesCount, int iMaxIndexAccServ, bool bResetOrder, int& iRemIndex )
{
	if( NULL == pclSheetDescription || NULL == pclSheetDescription->GetSSheetPointer() || 0 == iAccessAndServicesCount || NULL == paSSelAccessAndServices )
	{
		return false;
	}

	CSSheet* pclSheet = pclSheetDescription->GetSSheetPointer();

	//========================================
	// Sort accessories Array
	//========================================
	if( true == bResetOrder )
	{
		if( NULL != pDlgLeftTabSelP )
		{
			pDlgLeftTabSelP->SortTable( (CData**)paSSelAccessAndServices, iAccessAndServicesCount - 1 );
		}

		// Reset row index.
		for( int i = 0; i < iAccessAndServicesCount; i++ )
		{
			paSSelAccessAndServices[i]->GetpSelectedInfos()->SetRowIndex( i + 1 );
		}
	}
	else
	{
		// Reset row index.
		for( int i = 0; i < iAccessAndServicesCount; i++ )
		{
			if( 0 == paSSelAccessAndServices[i]->GetpSelectedInfos()->GetRowIndex() )
			{
				paSSelAccessAndServices[i]->GetpSelectedInfos()->SetRowIndex( ++iMaxIndexAccServ );
			}
		}
	}

	// Set Remark index.
	if( NULL != pDlgLeftTabSelP )
	{
		iRemIndex = pDlgLeftTabSelP->SetRemarkIndex( CLASS( CDS_SelPWQAccServices ), (CData**)paSSelAccessAndServices, iAccessAndServicesCount, iRemIndex );
	}

	//====================================
	// Fill and format the table
	//====================================
	CRank rkl;

	for( int i = 0; i < iAccessAndServicesCount; i++ )
	{
		rkl.Add( _T( "" ), paSSelAccessAndServices[i]->GetpSelectedInfos()->GetRowIndex(), (LPARAM)paSSelAccessAndServices[i] );
	}

	// Set the sub title.
	long lSelectedRow = 0;
	long lRow = pclSheet->GetMaxRows() + 1;

	if( 0 < m_pclSelectionTable->GetItemCount( CLASS( CDS_SSelSafetyValve ) ) )
	{
		lRow++;
	}
	SetLastRow( pclSheetDescription, lRow );
	SetPageTitle( pclSheetDescription, IDS_SELPROD_DIRSELACCESSORIESANDSERVICES, false, lRow );

	lRow++;

	CString str;
	LPARAM lParam;
	bool bFirstPass = true;

	for( BOOL bContinue = rkl.GetFirst( str, lParam ); TRUE == bContinue; bContinue = rkl.GetNext( str, lParam ) )
	{
		SetLastRow( pclSheetDescription, lRow );

		CData* pData = (CData*)lParam;
		CDS_SelPWQAccServices* pclAccessServices = dynamic_cast<CDS_SelPWQAccServices*>(pData);
		ASSERT( NULL != pclAccessServices );

		if( NULL == pclAccessServices )
		{
			return false;
		}

		if( m_pSelected == pData )
		{
			lSelectedRow = lRow;
		}

		m_rProductParam.Clear();
		m_rProductParam.SetSheetDescription( pclSheetDescription );
		m_rProductParam.SetSelectionContainer( (LPARAM)pclAccessServices );

		long lFirstRow = lRow;
		lRow = _FillAccessoriesServicesAlone( pclSheetDescription, lRow, pclAccessServices );

		// Draw dash line.
		pclSheet->SetCellBorder( ColumnDescription::Reference1, lRow, ColumnDescription::Footer - 1, lRow, true, SS_BORDERTYPE_BOTTOM, SS_BORDERSTYLE_FINE_DASH );


		// Draw line below.
		pclSheet->SetCellBorder( ColumnDescription::Reference1, lRow, ColumnDescription::Footer - 1, lRow, true, SS_BORDERTYPE_BOTTOM, SS_BORDERSTYLE_SOLID );

		// Save product.
		m_rProductParam.SetScrollRange( (true == bFirstPass) ? lFirstRow - 1 : lFirstRow, lRow );
		SaveProduct( m_rProductParam );

		// Spanning must be done here because it's the only place where we know exactly the number of lines to span!
		// Span reference #1 and #2.
		AddCellSpanW( pclSheetDescription, ColumnDescription::Reference1, lFirstRow, 1, pclSheet->GetMaxRows() - lFirstRow + 1 );
		AddCellSpanW( pclSheetDescription, ColumnDescription::Reference2, lFirstRow, 1, pclSheet->GetMaxRows() - lFirstRow + 1 );

		// Span water characteristic.
		AddCellSpanW( pclSheetDescription, ColumnDescription::Water, lFirstRow, 1, pclSheet->GetMaxRows() - lFirstRow + 1 );

		lRow++;

		_AddArticleList( (CDB_PWQPressureReducer*)pclAccessServices->GetProductIDPtr().MP, pclAccessServices->GetpSelectedInfos()->GetQuantity() );

		bFirstPass = false;
	}

	// Verify if a product is selected.
	if( 0 == lSelectedRow )
	{
		m_pSelected = 0;
	}

	return true;
}
