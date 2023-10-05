#include "stdafx.h"
#include "TASelect.h"
#include "MainFrm.h"
#include "Global.h"
#include "HydroMod.h"
#include "Hydronic.h"
#include "DlgLeftTabBase.h"
#include "DlgLeftTabSelP.h"
#include "RViewDescription.h"
#include "SelProdArticle.h"
#include "SelProdPageBase.h"
#include "SelProdPageDpCBCV.h"
#include "SelProdPageSv.h"

CSelProdPageDpCBCV::CSelProdPageDpCBCV( CArticleGroupList *pclArticleGroupList )
	: CSelProdPageBase( CDB_PageSetup::enCheck::DPCBCV, pclArticleGroupList )
{
	m_pSelected = NULL;
}

void CSelProdPageDpCBCV::Reset( void )
{
	CSelProdPageBase::Reset();
}

bool CSelProdPageDpCBCV::PreInit( HMvector &vecHMList )
{
	// Something for individual or direct selection ?
	CTable *pclTable = ( CTable * )( TASApp.GetpTADS()->Get( _T("DPCBCVALVE_TAB") ).MP );

	if( NULL == pclTable )
	{
		ASSERT( 0 );
		return false;
	}

	m_pclSelectionTable = ( '\0' != *pclTable->GetFirst().ID ) ? pclTable : NULL;
	bool fDpCBCVExist = ( NULL != m_pclSelectionTable );

	// Something for hydronic calculation?
	bool fDpCBCVHMExist = ( vecHMList.size() > 0 );

	if( false == fDpCBCVExist && false == fDpCBCVHMExist )
	{
		return false;
	}

	m_vecHMList = vecHMList;
	SortTable();
	return true;
}

bool CSelProdPageDpCBCV::Init( bool fResetOrder, bool fPrint )
{
	bool fDpCBCVExist = ( NULL != m_pclSelectionTable ) ? true : false;
	bool fDpCBCVHMExist = ( m_vecHMList.size() > 0 ) ? true : false;

	if( false == fDpCBCVExist && false == fDpCBCVHMExist )
	{
		return false;
	}

	// Call base class method to get a new 'Sheet'.
	CSheetDescription *pclSheetDescriptionDpCBCV = CMultiSpreadBase::CreateSSheet( SD_DpCBCValve );

	if( NULL == pclSheetDescriptionDpCBCV || NULL == pclSheetDescriptionDpCBCV->GetSSheetPointer() )
	{
		return false;
	}

	CSSheet *pclSheet = pclSheetDescriptionDpCBCV->GetSSheetPointer();

	if( false == CSelProdPageBase::PrepareSheet( pclSheetDescriptionDpCBCV, ColumnDescription::Pointer, fPrint ) )
	{
		m_ViewDescription.RemoveOneSheetDescription( SD_DpCBCValve );
		return false;
	}

	// Initialize column width for header and footer columns.
	if( false == m_bForPrint )
	{
		SetColWidth( pclSheetDescriptionDpCBCV, ColumnDescription::Header, DefaultColumnWidth::DCW_Header );
		SetColWidth( pclSheetDescriptionDpCBCV, ColumnDescription::Footer, DefaultColumnWidth::DCW_Footer );
	}
	else
	{
		pclSheet->ShowCol( ColumnDescription::Header, FALSE );
		pclSheet->ShowCol( ColumnDescription::Footer, FALSE );
	}

	double dWidth = ( true == m_pTADSPageSetup->GetField( PageField_enum::epfFIRSTREF ) ) ? DefaultColumnWidth::DCW_Reference1 : 0.0;
	SetColWidth( pclSheetDescriptionDpCBCV, ColumnDescription::Reference1, dWidth );

	dWidth = ( true == m_pTADSPageSetup->GetField( PageField_enum::epfSECONDREF ) ) ? DefaultColumnWidth::DCW_Reference2 : 0.0;
	SetColWidth( pclSheetDescriptionDpCBCV, ColumnDescription::Reference2, dWidth );

	dWidth = ( true == m_pTADSPageSetup->GetField( PageField_enum::epfWATERINFO ) ) ? DefaultColumnWidth::DCW_Water : 0.0;
	SetColWidth( pclSheetDescriptionDpCBCV, ColumnDescription::Water, dWidth );

	dWidth = ( true == m_pTADSPageSetup->GetField( PageField_enum::epfDPCBCVINFOPRODUCT ) ) ? DefaultColumnWidth::DCW_Product : 0.0;
	SetColWidth( pclSheetDescriptionDpCBCV, ColumnDescription::Product, dWidth );

	dWidth = ( true == m_pTADSPageSetup->GetField( PageField_enum::epfDPCBCVTECHINFO ) ) ? DefaultColumnWidth::DCW_TechnicalInfos : 0.0;
	SetColWidth( pclSheetDescriptionDpCBCV, ColumnDescription::TechnicalInfos, dWidth );

	SetColWidth( pclSheetDescriptionDpCBCV, ColumnDescription::RadiatorInfos, 0.0 );

	dWidth = ( true == m_pTADSPageSetup->GetField( PageField_enum::epfARTICLE ) ) ? DefaultColumnWidth::DCW_ArticleNumber : 0.0;
	SetColWidth( pclSheetDescriptionDpCBCV, ColumnDescription::ArticleNumber, dWidth );

	dWidth = ( true == m_pTADSPageSetup->GetField( PageField_enum::epfPIPEINFO ) ) ? DefaultColumnWidth::DCW_Pipes : 0.0;
	SetColWidth( pclSheetDescriptionDpCBCV, ColumnDescription::Pipes, dWidth );

	dWidth = ( true == m_pTADSPageSetup->GetField( PageField_enum::epfQUANTITY ) ) ? DefaultColumnWidth::DCW_Quantity : 0.0;
	SetColWidth( pclSheetDescriptionDpCBCV, ColumnDescription::Quantity, dWidth );

	dWidth = 0.0;

	if( true == TASApp.IsPriceUsed() )
	{
		dWidth = ( true == m_pTADSPageSetup->GetField( PageField_enum::epfSALEUNITPRICE ) ) ? DefaultColumnWidth::DCW_UnitPrice : 0.0;
	}

	SetColWidth( pclSheetDescriptionDpCBCV, ColumnDescription::UnitPrice, dWidth );

	dWidth = 0.0;

	if( true == TASApp.IsPriceUsed() )
	{
		dWidth = ( true == m_pTADSPageSetup->GetField( PageField_enum::epfSALETOTALPRICE ) ) ? DefaultColumnWidth::DCW_TotalPrice : 0.0;
	}

	SetColWidth( pclSheetDescriptionDpCBCV, ColumnDescription::TotalPrice, dWidth );

	SetColWidth( pclSheetDescriptionDpCBCV, ColumnDescription::Remark,
				 ( true == m_pTADSPageSetup->GetField( PageField_enum::epfREMARK ) ) ? DefaultColumnWidth::DCW_Remark : 0 );
	SetColWidth( pclSheetDescriptionDpCBCV, ColumnDescription::Pointer, DefaultColumnWidth::DCW_Pointer );
	pclSheet->ShowCol( ColumnDescription::RadiatorInfos, FALSE );

	if( false == TASApp.IsPriceUsed() )
	{
		pclSheet->ShowCol( ColumnDescription::UnitPrice, FALSE );
		pclSheet->ShowCol( ColumnDescription::TotalPrice, FALSE );
	}

	pclSheet->ShowCol( ColumnDescription::Pointer, FALSE );
	CRect rect = pclSheet->GetSheetSizeInPixels( false );
	m_mapSheetInfos[pclSheetDescriptionDpCBCV].m_dPageWidth = rect.Width();

	// Page title.
	SetPageTitle( pclSheetDescriptionDpCBCV, IDS_SSHEETSELPROD_TITLEDPCBCV );

	// Init column header.
	_InitColHeader( pclSheetDescriptionDpCBCV );

	// HYS-1571: We can optimize the column width now.
	// See Jira card to have more details about why we do that here.
	CRect rectClient;
	GetClientRect( &rectClient );
	OptimizePageSize( rectClient.Width(), rectClient.Height() );
	
	// Current position.
	long lRow = pclSheet->GetMaxRows() + 1;
	long lSelectedRow = 0;

	if( true == fDpCBCVExist )
	{
		// Number of objects.
		int iDpCBCVCount = m_pclSelectionTable->GetItemCount( CLASS( CDS_SSelDpCBCV ) );

		if( 0 == iDpCBCVCount )
		{
			return false;
		}

		CDS_SSelDpCBCV **ppaSSelDpCBCV = new CDS_SSelDpCBCV*[iDpCBCVCount];

		if( NULL == ppaSSelDpCBCV )
		{
			return false;
		}

		// Fill 'paSSelDpCBCV' array with pointer on each 'CDS_SSelDpCBCV' object of the 'DPCBCVALVE_TAB' table.
		int iMaxIndex = 0;
		int i = 0;

		for( IDPTR IDPtr = m_pclSelectionTable->GetFirst( CLASS( CDS_SSelDpCBCV ) ); _T('\0') != *IDPtr.ID; IDPtr = m_pclSelectionTable->GetNext() )
		{
			CDS_SSelDpCBCV *pclSSelDpCBCV = dynamic_cast<CDS_SSelDpCBCV *>( IDPtr.MP );
			
			if( NULL == pclSSelDpCBCV )
			{
				continue;
			}
			
			if( NULL == pclSSelDpCBCV->GetIDPtr().MP )
			{
				continue;
			}
			
			ppaSSelDpCBCV[i] = pclSSelDpCBCV;

			if( iMaxIndex < ppaSSelDpCBCV[i]->GetpSelectedInfos()->GetRowIndex() )
			{
				iMaxIndex = ppaSSelDpCBCV[i]->GetpSelectedInfos()->GetRowIndex();
			}

			++i;
		}
		
		ASSERT( i == iDpCBCVCount );
		iDpCBCVCount = i;

		// Sort 'ppaSSelDpCBCV' array.
		// Remark: 'fResetOrder' is set to 'true' only when the sorting combos are empty (No user choice) and the user clicks on the 'Apply sorting keys'.
		if( true == fResetOrder )
		{
			// Reset row index.
			for( i = 0; i < iDpCBCVCount; i++ )
			{
				ppaSSelDpCBCV[i]->GetpSelectedInfos()->SetRowIndex( i + 1 );
			}
		}
		else if( PageField_enum::epfNONE != m_pTADSPageSetup->GetKey( 0 ) || PageField_enum::epfNONE != m_pTADSPageSetup->GetKey( 1 ) 
	 			|| PageField_enum::epfNONE != m_pTADSPageSetup->GetKey( 2 ) )
		{
			// This method will sort (or not) in regards to the three sorting keys that user has chosen.
			if( NULL != pDlgLeftTabSelP )
			{
				pDlgLeftTabSelP->SortTable( (CData**)ppaSSelDpCBCV, iDpCBCVCount - 1 );
			}

			// Reset row index.
			for( i = 0; i < iDpCBCVCount; i++ )
			{
				ppaSSelDpCBCV[i]->GetpSelectedInfos()->SetRowIndex( i + 1 );
			}
		}

		// Create a remark index.
		int iRemIndex = -1;

		if( NULL != pDlgLeftTabSelP )
		{
			iRemIndex = pDlgLeftTabSelP->SetRemarkIndex( CLASS( CDS_SSelDpCBCV ), ( CData ** )ppaSSelDpCBCV, iDpCBCVCount );
		}

		// By default, the items are deleted when read.
		CRank rkl;

		enum eDpCBCVswitch
		{
			SingleSelDpCBCV,
			DirSelDpCBCV,
			LastDpCBCVswitch
		};

		bool bAtLeastOneBlockPrinted = false;

		for( int iLoopGroup = 0; iLoopGroup < LastDpCBCVswitch; iLoopGroup++ )
		{
			int iCount = 0;

			// For each selected control valve.
			for( i = 0; i < iDpCBCVCount; i++ )
			{
				CDS_SSelDpCBCV *pSelDpCBCV = dynamic_cast<CDS_SSelDpCBCV *>( ppaSSelDpCBCV[i] );

				if( NULL == pSelDpCBCV || NULL == pSelDpCBCV->GetProductIDPtr().MP )
				{
					ASSERT( 0 );
					continue;
				}

				if( SingleSelDpCBCV == iLoopGroup )
				{
					if( true == pSelDpCBCV->IsFromDirSel() )
					{
						continue;
					}
				}
				else
				{
					if( false == pSelDpCBCV->IsFromDirSel() )
					{
						continue;
					}
				}

				// All combined Dp controller and Balancing & control products are based on 'CDB_RegulatingValve'.
				CDB_RegulatingValve *pRegV = dynamic_cast<CDB_RegulatingValve *>( pSelDpCBCV->GetProductIDPtr().MP );

				if( NULL == pRegV )
				{
					ASSERT( 0 );
					continue;
				}

				rkl.Add( _T( "" ), ppaSSelDpCBCV[i]->GetpSelectedInfos()->GetRowIndex(), ( LPARAM )ppaSSelDpCBCV[i] );
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
			SetLastRow( pclSheetDescriptionDpCBCV, lRow );

			switch( iLoopGroup )
			{
				case SingleSelDpCBCV:
					SetPageTitle( pclSheetDescriptionDpCBCV, IDS_SSHEETSELPROD_SUBTITLEDPCBCVFROMSSEL, false, lRow );
					break;

				case DirSelDpCBCV:
					SetPageTitle( pclSheetDescriptionDpCBCV, IDS_SSHEETSELPROD_SUBTITLEDPCBCVFROMDIRSEL, false, lRow );
					break;

				default:
					ASSERT( 0 );
					break;
			}

			lRow = pclSheet->GetMaxRows();
			SetLastRow( pclSheetDescriptionDpCBCV, lRow );

			CString str;
			LPARAM lparam;
			bool bFirstPass = true;

			for( BOOL bContinue = rkl.GetFirst( str, lparam ); TRUE == bContinue; bContinue = rkl.GetNext( str, lparam ) )
			{
				CData *pclData = (CData *)lparam;
				CDS_SSelDpCBCV *pclSSelDpCBCV = dynamic_cast<CDS_SSelDpCBCV *>( pclData );

				if( NULL == pclSSelDpCBCV )
				{
					return false;
				}

				if( m_pSelected == pclData )
				{
					lSelectedRow = lRow;
				}

				m_rProductParam.Clear();
				m_rProductParam.SetSheetDescription( pclSheetDescriptionDpCBCV );
				m_rProductParam.SetSelectionContainer( ( LPARAM )pclSSelDpCBCV );

				long lFirstRow = lRow + 1;

				// Only if CV is present.
				if( pclSSelDpCBCV->GetProductAs<CDB_DpCBCValve>() )
				{
					lRow++;
					lRow = _FillDpCBCValve( pclSheetDescriptionDpCBCV, lRow, ( CDS_SSelDpCBCV * )pclSSelDpCBCV );

					// Spanning must be done here because it's the only place where we know exactly number of lines to span!
					// Span reference #1 and #2.
					AddCellSpanW( pclSheetDescriptionDpCBCV, ColumnDescription::Reference1, lFirstRow, 1, pclSheet->GetMaxRows() - lFirstRow + 1 );
					AddCellSpanW( pclSheetDescriptionDpCBCV, ColumnDescription::Reference2, lFirstRow, 1, pclSheet->GetMaxRows() - lFirstRow + 1 );

					// Span water characteristic.
					AddCellSpanW( pclSheetDescriptionDpCBCV, ColumnDescription::Water, lFirstRow, 1, pclSheet->GetMaxRows() - lFirstRow + 1 );

					// Add accessories.
					if( true == pclSSelDpCBCV->IsAccessoryExist() )
					{
						// Draw dash line.
						pclSheet->SetCellBorder( ColumnDescription::Reference1, lRow, ColumnDescription::Footer - 1, lRow, true, SS_BORDERTYPE_BOTTOM, SS_BORDERSTYLE_FINE_DASH );

						lRow++;
						lRow = _FillAccessoriesDpCBCVOrActr( pclSheetDescriptionDpCBCV, lRow, pclSSelDpCBCV, true );
					}
				}

				// Fill actuator if exist.
				if( NULL != dynamic_cast<CDB_ElectroActuator *>( pclSSelDpCBCV->GetActrIDPtr().MP ) )
				{
					// Draw dash line.
					pclSheet->SetCellBorder( ColumnDescription::Reference1, lRow, ColumnDescription::Footer - 1, lRow, true, SS_BORDERTYPE_BOTTOM, SS_BORDERSTYLE_FINE_DASH );

					lRow++;
					lRow = _FillActuator( pclSheetDescriptionDpCBCV, lRow, pclSSelDpCBCV );
				}

				if( true == pclSSelDpCBCV->IsActuatorAccessoryExist() )
				{
					// Draw dash line.
					pclSheet->SetCellBorder( ColumnDescription::Reference1, lRow, ColumnDescription::Footer - 1, lRow, true, SS_BORDERTYPE_BOTTOM, SS_BORDERSTYLE_FINE_DASH );

					lRow++;
					lRow = _FillAccessoriesDpCBCVOrActr( pclSheetDescriptionDpCBCV, lRow, pclSSelDpCBCV, false );
				}

				// Draw dash line.
				pclSheet->SetCellBorder( ColumnDescription::Reference1, lRow, ColumnDescription::Footer - 1, lRow, true, SS_BORDERTYPE_BOTTOM, SS_BORDERSTYLE_FINE_DASH );

				long lRowSv = pclSheet->GetMaxRows();
				CDB_ShutoffValve *pclShutoffValve = dynamic_cast<CDB_ShutoffValve*>( pclSSelDpCBCV->GetSVIDPtr().MP );

				if( NULL != pclShutoffValve )
				{
					CSelProdPageSv clSelProdPageSv( m_pclArticleGroupList );

					// Add infos on the selected ShutoffValve.
					clSelProdPageSv.FillRowSvProduct( pclSheetDescriptionDpCBCV, lRowSv + 1, pclShutoffValve, pclSSelDpCBCV->GetpSelectedInfos()->GetQuantity() );

					if( false == pclSSelDpCBCV->IsFromDirSel() ) 
					{
						double dQ = pclSSelDpCBCV->GetQ();
						double dRho = pclSSelDpCBCV->GetpSelectedInfos()->GetpWCData()->GetDens();
						double dNu = pclSSelDpCBCV->GetpSelectedInfos()->GetpWCData()->GetKinVisc();
						
						// HYS-38: Show power dt info when their radio button is checked
						CString strPowerDt = _T( "" );
						
						if( CDS_SelProd::efdPower == pclSSelDpCBCV->GetFlowDef() )
						{
							strPowerDt = WriteCUDouble( _U_TH_POWER, pclSSelDpCBCV->GetPower(), true );
							strPowerDt += _T(" / ");
							strPowerDt += WriteCUDouble( _U_DIFFTEMP, pclSSelDpCBCV->GetDT(), true );
						}
						
						clSelProdPageSv.FillRowSvInfo( pclSheetDescriptionDpCBCV, lRowSv + 1, pclShutoffValve, dQ, 0, dRho, dNu, strPowerDt );
					}

					// Because information is saved in the 'CSelProdPageSv' class, we need to manually add it here.
					m_rProductParam.AddRange( lRowSv + 1, pclSheet->GetMaxRows(), pclShutoffValve );

					lRowSv = pclSheet->GetMaxRows();

					if( true == pclSSelDpCBCV->IsSVAccessoryExist() )
					{
						// Draw dash line.
						pclSheet->SetCellBorder( ColumnDescription::Reference1, lRowSv, ColumnDescription::Footer - 1, lRowSv, true, SS_BORDERTYPE_BOTTOM, SS_BORDERSTYLE_FINE_DASH );

						// Add Sv accessories.
						lRowSv++;
						lRowSv = CSelProdPageBase::FillAccessories( pclSheetDescriptionDpCBCV, lRowSv, pclSSelDpCBCV->GetSVAccessoryList(), pclSSelDpCBCV->GetpSelectedInfos()->GetQuantity() );
					}

					// Add shutoff valve infos and accessories in the 'Article list' page only if valve has not been selected as a package.
					// Remark: if selected as a package, Sv accessories have been added in 'AddArticleList' above with DpCBCV.
					if( false == pclSSelDpCBCV->IsSelectedAsAPackage( true ) )
					{
						clSelProdPageSv.AddArticleInGroupListFromDpCBCV( pclSSelDpCBCV );
					}
				}

				// Draw line below.
				pclSheet->SetCellBorder( ColumnDescription::Reference1, lRowSv, ColumnDescription::Footer - 1, lRowSv, true, SS_BORDERTYPE_BOTTOM, SS_BORDERSTYLE_SOLID );

				// Save object pointer.
				m_rProductParam.SetScrollRange( ( true == bFirstPass ) ? lFirstRow - 1 : lFirstRow, lRowSv );
				SaveProduct( m_rProductParam );

				// Set all group as no breakable (for print).
				// Remark: include title with the group.
				pclSheet->SetFlagRowNoBreakable( ( true == bFirstPass ) ? lFirstRow - 1 : lFirstRow, lRowSv, true );

				_AddArticleList( pclSSelDpCBCV );

				bFirstPass = false;
				lRow = lRowSv;
			}

			lRow++;
			bAtLeastOneBlockPrinted = true;
		}

		// Verify if a product is selected.
		if( 0 == lSelectedRow )
		{
			m_pSelected = NULL;
		}

		FillRemarks( pclSheetDescriptionDpCBCV, lRow );

		// We add by default a blank line.
		lRow = pclSheet->GetMaxRows() + 2;

		delete[] ppaSSelDpCBCV;
	}

	if( true == fDpCBCVHMExist )
	{
		// By default, the items are deleted when read.
		CRank rkl;
		int iCount = 0;

		HMvector vecHMList;

		for( int i = 0; i < ( int )m_vecHMList.size(); i++ )
		{
			CDS_HydroMod::CCv *pHMCv = static_cast<CDS_HydroMod::CCv *>( m_vecHMList[i].second );

			if( NULL == pHMCv || NULL == pHMCv->GetCvIDPtr().MP )
			{
				continue;
			}

			// If it's not a combined Dp controller, control and balancing valve...
			if( eb3False == pHMCv->IsDpCBCV() )
			{
				continue;
			}

			if( NULL == dynamic_cast<CDB_DpCBCValve *>( pHMCv->GetCvIDPtr().MP ) )
			{
				continue;
			}

			vecHMList.push_back( CSelProdPageBase::PairPtr( CSelProdPageBase::ePointerType::eptCCV, ( void * )pHMCv ) );
			rkl.Add( _T( "" ), i, ( LPARAM )pHMCv );
			iCount++;
		}

		if( iCount > 0 )
		{
			// Create a remark index for the current group (CV, BCV or PIBCV).
			if( NULL != pDlgLeftTabSelP )
			{
				pDlgLeftTabSelP->SetRemarkIndex( &vecHMList );
			}

			// Set the subtitle.
			SetLastRow( pclSheetDescriptionDpCBCV, lRow );
			SetPageTitle( pclSheetDescriptionDpCBCV, IDS_SSHEETSELPROD_SUBTITLEDPCBCVFROMHM, false, lRow );

			lRow++;
			SetLastRow( pclSheetDescriptionDpCBCV, lRow );

			CString str;
			LPARAM lparam;
			bool bFirstPass = true;

			for( BOOL bContinue = rkl.GetFirst( str, lparam ); TRUE == bContinue; bContinue = rkl.GetNext( str, lparam ) )
			{
				CDS_HydroMod::CCv *pHMCv = ( CDS_HydroMod::CCv * )lparam;

				m_rProductParam.Clear();
				m_rProductParam.SetSheetDescription( pclSheetDescriptionDpCBCV );
				// Intentionally set to NULL to avoid clicking, double clicking or right clicking.
				m_rProductParam.SetSelectionContainer( ( LPARAM )0 );

				long lFirstRow = ++lRow;
				lRow = _FillDpCBCValve( pclSheetDescriptionDpCBCV, lRow, pHMCv );

				// HYS-721: Merge cells
				// Spanning must be done here because it's the only place where we know exactly number of lines to span!
				// Span reference #1 and #2.
				AddCellSpanW(pclSheetDescriptionDpCBCV, ColumnDescription::Reference1, lFirstRow, 1, pclSheet->GetMaxRows() - lFirstRow + 1);
				AddCellSpanW(pclSheetDescriptionDpCBCV, ColumnDescription::Reference2, lFirstRow, 1, pclSheet->GetMaxRows() - lFirstRow + 1);
				// Span water characteristic.
				AddCellSpanW(pclSheetDescriptionDpCBCV, ColumnDescription::Water, lFirstRow, 1, pclSheet->GetMaxRows() - lFirstRow + 1);

				CDB_TAProduct *pclTAProd = dynamic_cast<CDB_TAProduct*>(pHMCv->GetCvIDPtr().MP);
				lRow = FillAndAddBuiltInHMAccessories( pclSheet, pclSheetDescriptionDpCBCV, pclTAProd, lRow );

				// Fill set accessories if exist.
				if( pHMCv->GetCVActrAccSetCount() > 0 )
				{
					// Draw dash line.
					pclSheet->SetCellBorder( ColumnDescription::Reference1, lRow, ColumnDescription::Footer - 1, lRow, true, SS_BORDERTYPE_BOTTOM, SS_BORDERSTYLE_FINE_DASH );

					lRow++;
					lRow = _FillSetAccessoriesHM( pclSheetDescriptionDpCBCV, lRow, pHMCv );
				}

				// Fill adapter if exist.
				if( NULL != dynamic_cast<CDB_Product *>( ( CData * )pHMCv->GetActrAdaptIDPtr().MP ) )
				{
					// Draw dash line.
					pclSheet->SetCellBorder( ColumnDescription::Reference1, lRow, ColumnDescription::Footer - 1, lRow, true, SS_BORDERTYPE_BOTTOM, SS_BORDERSTYLE_FINE_DASH );

					// Add adapter if we can automatically select it.
					lRow++;
					lRow = _FillAdapterFromHM( pclSheetDescriptionDpCBCV, lRow, pHMCv );
				}

				// Fill actuator if exist.
				if( NULL != dynamic_cast<CDB_ElectroActuator *>( pHMCv->GetActrIDPtr().MP ) )
				{
					// Draw dash line.
					pclSheet->SetCellBorder( ColumnDescription::Reference1, lRow, ColumnDescription::Footer - 1, lRow, true, SS_BORDERTYPE_BOTTOM, SS_BORDERSTYLE_FINE_DASH );

					lRow++;
					lRow = _FillActuatorFromHM( pclSheetDescriptionDpCBCV, lRow, pHMCv );
				}

				// INSERT HERE SHUT-OFF VALVE.
				long lRowSV = lRow;

				// Draw dash line.
				pclSheet->SetCellBorder( ColumnDescription::Reference1, lRowSV, ColumnDescription::Footer - 1, lRowSV, true, SS_BORDERTYPE_BOTTOM, SS_BORDERSTYLE_FINE_DASH );

				CDS_HydroMod *pParentHM = pHMCv->GetpParentHM();

				if( true == pParentHM->IsShutoffValveExist( true ) )
				{
					CDS_HydroMod::CShutoffValve *pHMShutoffValve = pParentHM->GetpShutoffValve( CDS_HydroMod::eShutoffValveSupply );

					if( NULL == pHMShutoffValve )
					{
						pHMShutoffValve = pParentHM->GetpShutoffValve( CDS_HydroMod::eShutoffValveReturn );
					}

					CSelProdPageSv clSelProdPageSv( m_pclArticleGroupList );
					CDB_TAProduct *pclShutoffValve = NULL;
					long lRowSVProduct = clSelProdPageSv.FillRowSv( pclSheetDescriptionDpCBCV, lRowSV + 1, pParentHM, pHMShutoffValve, &pclShutoffValve );
					// HYS-1734: Add the shut-off valve general information that are different from DpCBCV.
					long lRowGen = CSelProdPageBase::FillRowGenOthers( pclSheetDescriptionDpCBCV, lRowSV + 1, pParentHM, pHMShutoffValve );
					lRowSV = max( lRowSVProduct, lRowGen );
				}

				// Save the product.
				m_rProductParam.SetScrollRange( ( true == bFirstPass ) ? lFirstRow - 1 : lFirstRow, lRowSV );
				SaveProduct( m_rProductParam );

				// Draw line below.
				pclSheet->SetCellBorder( ColumnDescription::Reference1, lRowSV, ColumnDescription::Footer - 1, lRowSV, true, SS_BORDERTYPE_BOTTOM, SS_BORDERSTYLE_SOLID );

				// Set all group as no breakable (for print).
				// Remark: include title with the group.
				pclSheet->SetFlagRowNoBreakable( ( true == bFirstPass ) ? lFirstRow - 1 : lFirstRow, lRowSV, true );

				_AddArticleList( pHMCv );

				lRow = lRowSV;
				bFirstPass = false;
			}

			lRow++;
			FillHMRemarks( &vecHMList, pclSheetDescriptionDpCBCV, lRow );
		}
	}

	// Move sheet to correct position.
	SetSheetSize();
	Invalidate();
	UpdateWindow();

	return true;
}

bool CSelProdPageDpCBCV::HasSomethingToDisplay( void )
{
	bool fDpCBCVExist = ( NULL != m_pclSelectionTable ) ? true : false;
	bool fDpCBCVHMExist = ( m_vecHMList.size() > 0 ) ? true : false;

	if( false == fDpCBCVExist && false == fDpCBCVHMExist )
	{
		return false;
	}

	return true;
}

void CSelProdPageDpCBCV::_InitColHeader( CSheetDescription *pclSheetDescription )
{
	if( NULL == pclSheetDescription || NULL == pclSheetDescription->GetSSheetPointer() )
	{
		return;
	}

	CSSheet *pclSheet = pclSheetDescription->GetSSheetPointer();
	CString str;

	// Format Columns Header.
	pclSheet->SetTextPattern( CSSheet::_SSTextPattern::ColumnHeader );
	pclSheet->SetTextPatternProperty( CSSheet::_SSTextPatternProperty::TPP_FontBackColor, ( LPARAM )_TAH_WHITE );
	str = TASApp.LoadLocalizedString( IDS_SELPHDR_REF1 );
	pclSheet->SetStaticText( ColumnDescription::Reference1, SelProdHeaderRow::HR_RowHeader, str );
	str = TASApp.LoadLocalizedString( IDS_SELPHDR_REF2 );
	pclSheet->SetStaticText( ColumnDescription::Reference2, SelProdHeaderRow::HR_RowHeader, str );
	str = TASApp.LoadLocalizedString( IDS_SELPHDR_WATER );
	pclSheet->SetStaticText( ColumnDescription::Water, SelProdHeaderRow::HR_RowHeader, str );
	str = TASApp.LoadLocalizedString( IDS_SELPHDR_PRODUCT );
	pclSheet->SetStaticText( ColumnDescription::Product, SelProdHeaderRow::HR_RowHeader, str );
	pclSheet->SetTextPatternProperty( CSSheet::_SSTextPatternProperty::TPP_StyleWordWrap, ( LPARAM )TRUE );
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
	pclSheet->SetCellBorder( ColumnDescription::Reference1, SelProdHeaderRow::HR_RowHeader, ColumnDescription::Footer - 1, SelProdHeaderRow::HR_RowHeader, true,
							 SS_BORDERTYPE_BOTTOM | SS_BORDERTYPE_TOP );

	// Freeze row header.
	pclSheet->SetFreeze( 0, SelProdHeaderRow::HR_RowHeader );
}

long CSelProdPageDpCBCV::_FillDpCBCValve( CSheetDescription *pclSheetDescription, long lRow, CDS_HydroMod::CCv *pclHMCv )
{
	if( NULL == pclSheetDescription || NULL == pclSheetDescription->GetSSheetPointer() || NULL == pclHMCv )
	{
		return lRow;
	}

	CDB_ControlValve *pTApCV = dynamic_cast<CDB_ControlValve *>( pclHMCv->GetCvIDPtr().MP );

	if( NULL == pTApCV )
	{
		return lRow;
	}

	CSSheet *pclSheet = pclSheetDescription->GetSSheetPointer();
	CString str;
	long lMaxRow, lFirstRow;
	lMaxRow = lFirstRow = lRow;

	pclSheet->SetTextPattern( CSSheet::_SSTextPattern::UserStaticColumn );
	pclSheet->SetTextPatternProperty( CSSheet::_SSTextPatternProperty::TPP_FontBold, ( LPARAM )FALSE );

	// Remark: 'true' to exclude any connection reference in the article number.
	CString strArticleNumber = pTApCV->GetArtNum( true );
	CDB_Set *pCvActSet = NULL;

	if( true == pTApCV->IsPartOfaSet() && eBool3::eb3True == pclHMCv->GetActrSelectedAsaPackage() )
	{
		CDB_Actuator *pclActuator = dynamic_cast<CDB_Actuator *>( pclHMCv->GetActrIDPtr().MP );

		if( NULL != pclActuator )
		{
			CTableSet *pTabSet = pTApCV->GetTableSet();
			ASSERT( NULL != pTabSet );
			pCvActSet = pTabSet->FindCompatibleSet( pTApCV->GetIDPtr().ID, pclActuator->GetIDPtr().ID );
		}

		if( NULL != pCvActSet )
		{
			strArticleNumber = pCvActSet->GetReference();
		}
	}

	if( NULL != pCvActSet )
	{
		FillArtNumberColForPackage( pclSheetDescription, ColumnDescription::ArticleNumber, lRow, pCvActSet, strArticleNumber );
	}
	else
	{
		FillArtNumberCol( pclSheetDescription, ColumnDescription::ArticleNumber, lRow, pTApCV, strArticleNumber );
	}

	// HYS-1734: Add the valve to get temperature and pipe in FillRowGen.
	FillRowGen( pclSheetDescription, lRow, pclHMCv->GetpParentHM(), pclHMCv );
	FillQtyPriceCol( pclSheetDescription, lRow, pTApCV, 1 );

	// Column product.
	_FillDpCBCVTAP( pclSheetDescription, lRow, pTApCV );

	// Column infos.
	lMaxRow = max( lRow, lMaxRow );
	lRow = lFirstRow;
	AddStaticText( pclSheetDescription, ColumnDescription::TechnicalInfos, lRow, _T("-") );

	// Flow.
	if( true == m_pTADSPageSetup->GetField( epfCVTECHINFOFLOW ) )
	{
		double dQ = pclHMCv->GetQ();
		ASSERT( dQ > 0.0 );

		if( 0.0 != dQ )
		{
			str = WriteCUDouble( _U_FLOW, dQ, true );
			lRow = AddStaticText( pclSheetDescription, ColumnDescription::TechnicalInfos, lRow, str );
		}
	}

	// Dp.
	if( true == m_pTADSPageSetup->GetField( epfCVTECHINFODP ) )
	{
		double dDp = pclHMCv->GetDp();

		if( 0.0 != dDp )
		{
			if( true == pTApCV->IsaPICV() )
			{
				str = TASApp.LoadLocalizedString( IDS_DPMIN );
				str += ( CString )_T(" = ");
				str += WriteCUDouble( _U_DIFFPRESS, dDp, true );
			}
			else
			{
				str = WriteCUDouble( _U_DIFFPRESS, dDp, true );
			}

			lRow = AddStaticText( pclSheetDescription, ColumnDescription::TechnicalInfos, lRow, str );
		}
	}

	// Presetting.
	if( true == m_pTADSPageSetup->GetField( epfCVTECHINFOSETTING ) )
	{
		str = _T("-");

		// HYS-1871: Display setting for DpCBCV.
		str = pclHMCv->GetSettingStr( true );

		if( true == str.IsEmpty() )
		{
			str = L"-";
		}

		lRow = AddStaticText( pclSheetDescription, ColumnDescription::TechnicalInfos, lRow, str );
	}

	m_rProductParam.AddRange( lFirstRow, pclSheet->GetMaxRows(), pTApCV );
	return pclSheet->GetMaxRows();
}

long CSelProdPageDpCBCV::_FillDpCBCValve( CSheetDescription *pclSheetDescription, long lRow, CDS_SSelDpCBCV *pclSSelDpCBCV )
{
	if( NULL == pclSheetDescription || NULL == pclSheetDescription->GetSSheetPointer() || NULL == pclSSelDpCBCV )
	{
		return lRow;
	}

	CDB_DpCBCValve *pclDpCBCValve = pclSSelDpCBCV->GetProductAs<CDB_DpCBCValve>();

	if( NULL == pclDpCBCValve )
	{
		return lRow;
	}

	CSSheet *pclSheet = pclSheetDescription->GetSSheetPointer();
	CString str;
	long lFirstRow = lRow;

	pclSheet->SetTextPattern( CSSheet::_SSTextPattern::UserStaticColumn );
	pclSheet->SetTextPatternProperty( CSSheet::_SSTextPatternProperty::TPP_FontBold, ( LPARAM )FALSE );

	CString strArticleNumber;

	if( true == pclSSelDpCBCV->IsSelectedAsAPackage( true ) )
	{
		CDB_Set *pDpCBCVActSet = dynamic_cast<CDB_Set *>( pclSSelDpCBCV->GetCvActrSetIDPtr().MP );

		if( NULL == pDpCBCVActSet )
		{
			ASSERT( 0 );
			return lRow;
		}

		strArticleNumber = pDpCBCVActSet->GetReference();
		FillArtNumberColForPackage( pclSheetDescription, ColumnDescription::ArticleNumber, lRow, pDpCBCVActSet, strArticleNumber );
	}
	else
	{
		// Remark: 'true' to exclude any connection reference in the article number.
		strArticleNumber = pclDpCBCValve->GetArtNum( true );
		FillArtNumberCol( pclSheetDescription, ColumnDescription::ArticleNumber, lRow, pclDpCBCValve, strArticleNumber );
	}

	FillRowGen( pclSheetDescription, lRow, pclSSelDpCBCV );
	FillQtyPriceCol( pclSheetDescription, lRow, pclDpCBCValve, pclSSelDpCBCV->GetpSelectedInfos()->GetQuantity() );

	// Column TA Product.
	long lRowProduct = _FillDpCBCVTAP( pclSheetDescription, lRow, pclDpCBCValve );

	// Column Infos.
	lRow = lFirstRow;
	AddStaticText( pclSheetDescription, ColumnDescription::TechnicalInfos, lRow, _T("-") );

	CDB_ControlValve::ControlValveTable eControlValveTable = pclDpCBCValve->GetCVParentTable();

	// Flow.
	// If valve is directly selected don't fill Water, pipes.
	if( false == pclSSelDpCBCV->IsFromDirSel() )
	{
		double dQ = pclSSelDpCBCV->GetQ();
		ASSERT( dQ > 0.0 );

		if( true == m_pTADSPageSetup->GetField( epfDPCBCVTECHINFOFLOW ) && 0.0 != dQ )
		{
			str = WriteCUDouble( _U_FLOW, dQ, true );
			// HYS-38: Show power dt info when their radio button is checked
			CString strPowerDt = _T( "" );
			if( CDS_SelProd::efdPower == pclSSelDpCBCV->GetFlowDef() )
			{
				strPowerDt = WriteCUDouble( _U_TH_POWER, pclSSelDpCBCV->GetPower(), true );
				strPowerDt += _T(" / ");
				strPowerDt += WriteCUDouble( _U_DIFFTEMP, pclSSelDpCBCV->GetDT(), true );
			}
			if( strPowerDt != _T( "" ) )
			{
				CString strQ = _T("(");
				strQ += str;
				strQ += _T(")");
				lRow = AddStaticText( pclSheetDescription, ColumnDescription::TechnicalInfos, lRow, strPowerDt );
				lRow = AddStaticText( pclSheetDescription, ColumnDescription::TechnicalInfos, lRow, strQ );
			}
			else
			{
				lRow = AddStaticText( pclSheetDescription, ColumnDescription::TechnicalInfos, lRow, str );
			}
		}

		double dRho = pclSSelDpCBCV->GetpSelectedInfos()->GetpWCData()->GetDens();
		double dKinVisc = pclSSelDpCBCV->GetpSelectedInfos()->GetpWCData()->GetKinVisc();

		// Dp min.
		if( true == m_pTADSPageSetup->GetField( epfDPCBCVTECHINFODPMIN ) )
		{
			// Get pressure drop over the pressure part of the valve.
			double dDpp = pclDpCBCValve->GetDppmin( dQ, dRho );
		
			// Get pressure drop over the control part of the valve.
			double dDpc = pclDpCBCValve->GetDpc( dQ, pclSSelDpCBCV->GetDpToStalibize() );

			CString str2 = TASApp.LoadLocalizedString( IDS_DPMIN );
			str = str2 + _T(" = ") + WriteCUDouble( _U_DIFFPRESS, dDpp + dDpc, true );
			lRow = AddStaticText( pclSheetDescription, ColumnDescription::TechnicalInfos, lRow, str );
		}

		// Presetting.
		if( true == m_pTADSPageSetup->GetField( epfDPCBCVTECHINFOSETTING ) )
		{
			double dH = pclSSelDpCBCV->GetOpening();

			str = _T("-");

			if( 0.0 != dH && NULL != pclDpCBCValve->GetValveCharDataPointer() )
			{
				CDB_ValveCharacteristic *pValvChar = ( CDB_ValveCharacteristic * )pclDpCBCValve->GetValveCharDataPointer();
				str = pValvChar->GetSettingString( dH, true );
			}

			lRow = AddStaticText( pclSheetDescription, ColumnDescription::TechnicalInfos, lRow, str );
		}

		// H min.
		if( true == m_pTADSPageSetup->GetField( epfDPCBCVTECHINFOHMIN ) )
		{
			str = TASApp.LoadLocalizedString( IDS_HMIN ) + _T(" = ");
			CString str2 = _T("-");

			double dHMin = pclSSelDpCBCV->GetHMin();

			if( 0.0 != dHMin )
			{
				str2 = WriteCUDouble( _U_DIFFPRESS, dHMin, true );
			}

			str += str2;
			lRow = AddStaticText( pclSheetDescription, ColumnDescription::TechnicalInfos, lRow, str );
		}
	}

	m_rProductParam.AddRange( lFirstRow, pclSheet->GetMaxRows(), pclDpCBCValve );
	return pclSheet->GetMaxRows();
}

long CSelProdPageDpCBCV::_FillDpCBCVTAP( CSheetDescription *pclSheetDescription, long lRow, CDB_TAProduct *pTAP )
{
	if( NULL == pclSheetDescription || NULL == pclSheetDescription->GetSSheetPointer() || NULL == pTAP )
	{
		return lRow;
	}

	CString str;

	// Column 'Product'.
	AddStaticText( pclSheetDescription, ColumnDescription::Product, lRow, _T("-") );

	if( true == m_pTADSPageSetup->GetField( epfDPCBCVINFOPRODUCTNAME ) )
	{
		lRow = AddStaticText( pclSheetDescription, ColumnDescription::Product, lRow, pTAP->GetName() );
	}

	// Size.
	if( true == m_pTADSPageSetup->GetField( epfDPCBCVINFOPRODUCTSIZE ) )	
	{
		lRow = AddStaticText( pclSheetDescription, ColumnDescription::Product, lRow, pTAP->GetSize() );
	}

	// Connection.
	if( true == m_pTADSPageSetup->GetField( epfDPCBCVINFOPRODUCTCONNECTION ) )
	{
		CDB_StringID *pStrID = dynamic_cast <CDB_StringID *>( pTAP->GetConnectIDPtr().MP );

		if( NULL != pStrID )
		{
			str = pStrID->GetString();

			if( 0 == IDcmp( pTAP->GetConnTabID(), _T("COMPONENT_TAB") ) )
			{
				str += CString( _T(" -> ") ) + pTAP->GetSize();
			}

			lRow = AddStaticText( pclSheetDescription, ColumnDescription::Product, lRow, str );
		}
	}

	// Version.
	if( true == m_pTADSPageSetup->GetField( epfDPCBCVINFOPRODUCTVERSION ) )
	{
		CDB_StringID *pStrID = dynamic_cast <CDB_StringID *>( pTAP->GetVersionIDPtr().MP );

		if( NULL != pStrID )
		{
			lRow = AddStaticText( pclSheetDescription, ColumnDescription::Product, lRow, pStrID->GetString() );
		}
	}

	// PN.
	if( true == m_pTADSPageSetup->GetField( epfDPCBCVINFOPRODUCTPN ) )
	{
		lRow = FillPNTminTmax( pclSheetDescription, ColumnDescription::Product, lRow, pTAP );
	}

	return pclSheetDescription->GetSSheetPointer()->GetMaxRows();
}

long CSelProdPageDpCBCV::_FillAccessoriesDpCBCVOrActr( CSheetDescription *pclSheetDescription, long lRow, CDS_SSelDpCBCV *pclSSelDpCBCV, bool fForValve )
{
	if( NULL == pclSheetDescription || NULL == pclSheetDescription->GetSSheetPointer() || NULL == pclSSelDpCBCV )
	{
		return lRow;
	}

	CAccessoryList *pclAccessoryList = ( true == fForValve ) ? pclSSelDpCBCV->GetCvAccessoryList() : pclSSelDpCBCV->GetActuatorAccessoryList();
	int iCount = pclAccessoryList->GetCount();

	if( iCount > 0 )
	{
		CRank rkList;
		CAccessoryList::AccessoryItem rAccessoryItem = pclAccessoryList->GetFirst();

		while( rAccessoryItem.IDPtr.MP != NULL )
		{
			CDB_Product *pclAccessory = dynamic_cast<CDB_Product *>( rAccessoryItem.IDPtr.MP );
			VERIFY( pclAccessory != NULL );

			if( NULL == pclAccessory || false == pclAccessory->IsAnAccessory() )
			{
				break;
			}

			CAccessoryList::AccessoryItem *pclAccessoryItem = new CAccessoryList::AccessoryItem();

			if( NULL == pclAccessoryItem )
			{
				break;
			}

			*pclAccessoryItem = rAccessoryItem;
			rkList.AddStrSort( pclAccessory->GetName(), 0, ( LPARAM )pclAccessoryItem, false, false );
			rAccessoryItem = pclAccessoryList->GetNext();
		}

		CString str;
		LPARAM lParam;

		for( BOOL fContinue = rkList.GetFirst( str, lParam ); TRUE == fContinue; fContinue = rkList.GetNext( str, lParam ) )
		{
			CAccessoryList::AccessoryItem *pclAccessoryItem = ( CAccessoryList::AccessoryItem * )lParam;
			CDB_Product *pclAccessory = dynamic_cast<CDB_Product *>( ( CData * )( pclAccessoryItem->IDPtr.MP ) );
			bool fByPair = pclAccessoryItem->fByPair;

			if( NULL == pclAccessory || false == pclAccessory->IsAnAccessory() )
			{
				continue;
			}

			// If accessory is included into DpCBCVSet, mask his article number.
			bool fWithArticleNumber = true;

			if( true == fForValve )
			{
				// TODO
			}

			int iQuantity = pclSSelDpCBCV->GetpSelectedInfos()->GetQuantity();
			// HYS-987: DPCBCV assessories
			if( pclAccessoryItem->lEditedQty != -1 )
			{
				iQuantity = pclAccessoryItem->lEditedQty;
			}
			else if( true == fByPair )
			{
				iQuantity *= 2;
			}
			delete pclAccessoryItem;

			lRow = FillAccessory( pclSheetDescription, lRow, pclAccessory, fWithArticleNumber, iQuantity );
			lRow++;
		}
	}

	return pclSheetDescription->GetSSheetPointer()->GetMaxRows();
}

long CSelProdPageDpCBCV::_FillSetAccessoriesHM( CSheetDescription *pclSheetDescription, long lRow, CDS_HydroMod::CCv *pHMCv )
{
	if( NULL == pclSheetDescription || NULL == pclSheetDescription->GetSSheetPointer() || NULL == pHMCv || NULL == pHMCv->GetpCV()
		|| _NULL_IDPTR == pHMCv->GetActrIDPtr() || 0 == pHMCv->GetCVActrAccSetCount() )
	{
		return lRow;
	}

	CDB_ControlValve *pclControlValve = pHMCv->GetpCV();
	CTableSet *pCVActSetTab = pclControlValve->GetTableSet();
	ASSERT( NULL != pCVActSetTab );

	if( NULL == pCVActSetTab )
	{
		return lRow;
	}

	CDB_Actuator *pclActuator = ( CDB_Actuator * )( pHMCv->GetActrIDPtr().MP );
	CDB_Set *pCVActrSet = pCVActSetTab->FindCompatibleSet( pclControlValve->GetIDPtr().ID, pclActuator->GetIDPtr().ID );

	if( NULL == pCVActrSet )
	{
		return lRow;
	}

	// CV-Actuator accessories set.
	CDB_RuledTable *pclRuledTable = dynamic_cast<CDB_RuledTable *>( pCVActrSet->GetAccGroupIDPtr().MP );

	if( NULL != pclRuledTable )
	{
		// CV-Actuator accessories set.
		for( int iLoopCVActrAccSet = 0; iLoopCVActrAccSet < pHMCv->GetCVActrAccSetCount(); iLoopCVActrAccSet++ )
		{
			CDB_Product *pclAccessory = (CDB_Product * )( pHMCv->GetCVActrAccSetIDPtr( iLoopCVActrAccSet ).MP );

			if( NULL == pclAccessory || false == pclAccessory->IsAnAccessory() )
			{
				continue;
			}

			bool fByPair = pclRuledTable->IsByPair( pclAccessory->GetIDPtr().ID );
			lRow = FillAccessory( pclSheetDescription, lRow, pclAccessory, false, ( false == fByPair ) ? 1 : 2 );
			lRow++;
		}
	}

	return pclSheetDescription->GetSSheetPointer()->GetMaxRows();
}

long CSelProdPageDpCBCV::_FillActuator( CSheetDescription *pclSheetDescription, long lRow, CDS_SSelDpCBCV *pclSSelDpCBCV )
{
	if( NULL == pclSheetDescription || NULL == pclSheetDescription->GetSSheetPointer() || NULL == pclSSelDpCBCV )
	{
		return lRow;
	}

	CSSheet *pclSheet = pclSheetDescription->GetSSheetPointer();

	CDB_ElectroActuator *pclElectroActuator = dynamic_cast<CDB_ElectroActuator *>( pclSSelDpCBCV->GetActrIDPtr().MP );

	if( NULL != pclElectroActuator )
	{
		long lFirstRow = lRow;

		CString str;
		pclSheet->SetTextPattern( CSSheet::_SSTextPattern::UserStaticColumn );
		pclSheet->SetTextPatternProperty( CSSheet::_SSTextPatternProperty::TPP_FontBold, ( LPARAM )FALSE );

		// Set font color to red when accessory is not deleted or not available.
		if( true == pclElectroActuator->IsDeleted() || false == pclElectroActuator->IsAvailable() )
		{
			pclSheet->SetTextPatternProperty( CSSheet::_SSTextPatternProperty::TPP_FontForeColor, ( LPARAM )_RED );
		}

		// If selected as a DpCBCV-Actuator set don't show actuator article number.
		if( true == pclSSelDpCBCV->IsSelectedAsAPackage( true ) )
		{
			AddStaticText( pclSheetDescription, ColumnDescription::ArticleNumber, lRow, _T("-") );
		}
		else
		{
			// Check first if we have a local article number available.
			CString strLocalArticleNumber = _T( "" );

			if( true == TASApp.IsLocalArtNumberUsed() )
			{
				strLocalArticleNumber = LocArtNumTab.GetLocalArticleNumber( pclElectroActuator->GetArtNum() );
				strLocalArticleNumber.Trim();
			}

			// Fill article number.
			bool fArticleNumberShown = false;

			// If we can show article number OR we can't show but there is no local article number...
			if( false == TASApp.IsTAArtNumberHidden() || ( true == TASApp.IsTAArtNumberHidden() && true == strLocalArticleNumber.IsEmpty() ) )
			{
				CString str = pclElectroActuator->GetArtNum();

				if( false == str.IsEmpty() )
				{
					AddStaticText( pclSheetDescription, ColumnDescription::ArticleNumber, lRow, pclElectroActuator->GetArtNum() );
					fArticleNumberShown = true;
				}
			}

			// Set local article number if allowed and exist.
			if( true == TASApp.IsLocalArtNumberUsed() && false == strLocalArticleNumber.IsEmpty() )
			{
				long lNextRow = lRow + ( ( true == fArticleNumberShown ) ? 1 : 0 );
				CString strPrefixLAN = TASApp.GetLocalArtNumberName();

				if( false == strPrefixLAN.IsEmpty() )
				{
					lNextRow = AddStaticText( pclSheetDescription, ColumnDescription::ArticleNumber, lNextRow, strPrefixLAN );
				}

				AddStaticText( pclSheetDescription, ColumnDescription::ArticleNumber, lNextRow, strLocalArticleNumber );
			}
		}

		long lFlagRow = pclSheet->GetMaxRows() + 1;

		if( true == pclElectroActuator->IsDeleted() )
		{
			AddStaticText( pclSheetDescription, ColumnDescription::ArticleNumber, lFlagRow, TASApp.LoadLocalizedString( IDS_TAPDELETED ) );
		}
		else if( false == pclElectroActuator->IsAvailable() )
		{
			AddStaticText( pclSheetDescription, ColumnDescription::ArticleNumber, lFlagRow, TASApp.LoadLocalizedString( IDS_TAPNOTAVAILABLE ) );
		}

		pclSheet->SetTextPatternProperty( CSSheet::_SSTextPatternProperty::TPP_FontForeColor, ( LPARAM )_BLACK );

		// Write nothing for technical info.
		AddStaticText( pclSheetDescription, ColumnDescription::TechnicalInfos, lRow, _T("-") );

		// Write quantity and prices.
		FillQtyPriceCol( pclSheetDescription, lRow, pclElectroActuator, pclSSelDpCBCV->GetpSelectedInfos()->GetQuantity() );

		// Write the actuator name.
		AddStaticText( pclSheetDescription, ColumnDescription::Product, lRow++, pclElectroActuator->GetName() );
		pclSheet->SetTextPatternProperty( CSSheet::_SSTextPatternProperty::TPP_StyleHorizontalAlign, ( LPARAM )SSS_ALIGN_LEFT );

		CDB_DpCBCValve *pclDpCBCValve = pclSSelDpCBCV->GetProductAs<CDB_DpCBCValve>();

		if( NULL != pclDpCBCValve )
		{
			CDB_CloseOffChar *pCloseOffChar = ( CDB_CloseOffChar * )pclDpCBCValve->GetCloseOffCharIDPtr().MP;

			// Set the Close off Dp.
			if( NULL != pCloseOffChar ) 
			{
				if( CDB_CloseOffChar::CloseOffDp == pCloseOffChar->GetLimitType() )
				{
					str = TASApp.LoadLocalizedString( IDS_SSHEETSELPROD_CLOSEOFFDP );
					double dCloseOffDp = pCloseOffChar->GetCloseOffDp( pclElectroActuator->GetMaxForceTorque() );

					if( dCloseOffDp > 0.0 )
					{
						str += CString( _T(" : ") ) + WriteCUDouble( _U_DIFFPRESS, dCloseOffDp, true );
					}
				}
				else
				{
					str = TASApp.LoadLocalizedString( IDS_SSHEETSELPROD_MAXINLETPRESSURE );
					double dMaxInletPressure = pCloseOffChar->GetMaxInletPressure( pclElectroActuator->GetMaxForceTorque() );

					if( dMaxInletPressure > 0.0 )
					{
						str += CString( _T(" : ") ) + WriteCUDouble( _U_PRESSURE, dMaxInletPressure, true );
					}
				}

				AddStaticText( pclSheetDescription, ColumnDescription::Product, lRow, str );
				AddCellSpanW( pclSheetDescription, ColumnDescription::Product, lRow++, 2, 1 );
			}

			// Compute Actuating time in sec.
			str = TASApp.LoadLocalizedString( IDS_SSHEETSELPROD_ACTUATING_TIME ) + CString( _T(" : ") ) + pclElectroActuator->GetActuatingTimesStr( pclDpCBCValve->GetStroke(),
					true );
			AddStaticText( pclSheetDescription, ColumnDescription::Product, lRow, str );
			AddCellSpanW( pclSheetDescription, ColumnDescription::Product, lRow++, 2, 1 );
		}

		// Write IP.
		str = TASApp.LoadLocalizedString( IDS_SSHEETSELPROD_IPXXAUTO );
		str += CString( _T(" : ") ) + pclElectroActuator->GetIPxxAuto();

		if( false == pclElectroActuator->GetIPxxManual().IsEmpty() )
		{
			str += _T("(" ) + pclElectroActuator->GetIPxxManual() + _T( ")");
		}

		AddStaticText( pclSheetDescription, ColumnDescription::Product, lRow, str );
		AddCellSpanW( pclSheetDescription, ColumnDescription::Product, lRow++, 2, 1 );

		// Write power supply.
		str = TASApp.LoadLocalizedString( IDS_SSHEETSELPROD_POWSUPPL ) + CString( _T(" : ") ) + pclElectroActuator->GetPowerSupplyStr();
		AddStaticText( pclSheetDescription, ColumnDescription::Product, lRow, str );
		AddCellSpanW( pclSheetDescription, ColumnDescription::Product, lRow++, 2, 1 );

		// Write input signal.
		str = TASApp.LoadLocalizedString( IDS_SSHEETSELPROD_INPUT ) + CString( _T(" : ") ) + pclElectroActuator->GetInOutSignalsStr( true );
		AddStaticText( pclSheetDescription, ColumnDescription::Product, lRow, str );
		AddCellSpanW( pclSheetDescription, ColumnDescription::Product, lRow++, 2, 1 );

		// Write output signal.
		str = TASApp.LoadLocalizedString( IDS_SSHEETSELPROD_OUTPUTSIG );

		if( CString( _T( "" ) ) == pclElectroActuator->GetInOutSignalsStr( false ) )
		{
			str += CString( _T(" :    -") );
		}
		else
		{
			str += CString( _T(" : ") ) + pclElectroActuator->GetInOutSignalsStr( false );
		}

		AddStaticText( pclSheetDescription, ColumnDescription::Product, lRow, str );
		AddCellSpanW( pclSheetDescription, ColumnDescription::Product, lRow++, 2, 1 );

		// Write fail safe.
		str = TASApp.LoadLocalizedString( IDS_SSHEETSELPROD_FAILSAFE );

		if( pclElectroActuator->GetFailSafe() == CDB_ElectroActuator::FailSafeType::eFSTypeElectronic )
		{
			str += CString( _T( " : " ) ) + TASApp.LoadLocalizedString( IDS_STRELECFAILSAFE );
		}
		else if( pclElectroActuator->GetFailSafe() == CDB_ElectroActuator::FailSafeType::eFSTypeSpringReturn )
		{
			str += CString( _T( " : " ) ) + TASApp.LoadLocalizedString( IDS_STRSPRINGFAILSAFE );
		}
		else
		{
			str += CString( _T( " : " ) ) + TASApp.LoadLocalizedString( IDS_NO );
		}

		AddStaticText( pclSheetDescription, ColumnDescription::Product, lRow, str );
		AddCellSpanW( pclSheetDescription, ColumnDescription::Product, lRow++, 2, 1 );

		// Write Default return position
		str = ( (int)pclElectroActuator->GetFailSafe() > 0 ) ? TASApp.LoadLocalizedString( IDS_SSHEETSSEL_ACTRFSP ) : TASApp.LoadLocalizedString( IDS_SSHEETSSEL_ACTRDRP );

		if( CString( _T( "" ) ) == pclElectroActuator->GetDefaultReturnPosStr( pclElectroActuator->GetDefaultReturnPos() ).c_str() )
		{
			str += CString( _T(" :    -") );
		}
		else
		{
			str += CString( _T(" : ") ) + pclElectroActuator->GetDefaultReturnPosStr( pclElectroActuator->GetDefaultReturnPos() ).c_str();
		}

		AddStaticText( pclSheetDescription, ColumnDescription::Product, lRow, str );
		AddCellSpanW( pclSheetDescription, ColumnDescription::Product, lRow++, 2, 1 );

		// HYS-726: Show max temp info in result tab
		str = TASApp.LoadLocalizedString( IDS_SSHEETSSELCV_ACTRMAXTEMP );

		if( DBL_MAX == pclElectroActuator->GetTmax() )
		{
			str += CString( _T(" :    -") );
		}
		else
		{
			str += CString( _T(" : ") ) + WriteDouble( pclElectroActuator->GetTmax(), 3 );
			str += TASApp.LoadLocalizedString( IDS_UNITDEGREECELSIUS );
		}

		AddStaticText( pclSheetDescription, ColumnDescription::Product, lRow, str );
		AddCellSpanW( pclSheetDescription, ColumnDescription::Product, lRow++, 2, 1 );
		// Center the text below.
		pclSheet->SetTextPatternProperty( CSSheet::_SSTextPatternProperty::TPP_StyleHorizontalAlign, ( LPARAM )SSS_ALIGN_CENTER );

		m_rProductParam.AddRange( lFirstRow, pclSheet->GetMaxRows(), pclElectroActuator );

		lRow++;
	}

	return pclSheet->GetMaxRows();
}

long CSelProdPageDpCBCV::_FillActuatorFromHM( CSheetDescription *pclSheetDescription, long lRow, CDS_HydroMod::CCv *pclHMCv )
{
	if( NULL == pclSheetDescription || NULL == pclSheetDescription->GetSSheetPointer() || NULL == pclHMCv )
	{
		return lRow;
	}

	CSSheet *pclSheet = pclSheetDescription->GetSSheetPointer();
	CString str;
	long lFirstRow = lRow;
	bool fCvActrSet = false;

	// At now we accept only electric and thermoelectric actuator.
	CDB_ElectroActuator *pclElectroActuator = dynamic_cast<CDB_ElectroActuator *>( pclHMCv->GetActrIDPtr().MP );

	if( NULL == pclElectroActuator )
	{
		return pclSheet->GetMaxRows();
	}

	CDB_ControlValve *pclControlValve = dynamic_cast<CDB_ControlValve *>( pclHMCv->GetCvIDPtr().MP );

	if( NULL != pclElectroActuator && NULL != pclControlValve )
	{
		pclSheet->SetTextPattern( CSSheet::_SSTextPattern::UserStaticColumn );
		pclSheet->SetTextPatternProperty( CSSheet::_SSTextPatternProperty::TPP_FontBold, ( LPARAM )FALSE );

		// Set font color to red when accessory is not deleted or not available.
		if( true == pclElectroActuator->IsDeleted() || false == pclElectroActuator->IsAvailable() )
		{
			pclSheet->SetTextPatternProperty( CSSheet::_SSTextPatternProperty::TPP_FontForeColor, ( LPARAM )_RED );
		}

		// If selected as a CV-Actuator set don't show actuator article number.
		if( true == pclControlValve->IsPartOfaSet() && eBool3::eb3True == pclHMCv->GetActrSelectedAsaPackage() )
		{
			AddStaticText( pclSheetDescription, ColumnDescription::ArticleNumber, lRow, _T("-") );
		}
		else
		{
			// Check first if we have a local article number available.
			CString strLocalArticleNumber = _T( "" );

			if( true == TASApp.IsLocalArtNumberUsed() )
			{
				strLocalArticleNumber = LocArtNumTab.GetLocalArticleNumber( pclElectroActuator->GetArtNum() );
				strLocalArticleNumber.Trim();
			}

			// Fill article number.
			bool fArticleNumberShown = false;

			// If we can show article number OR we can't show but there is no local article number...
			if( false == TASApp.IsTAArtNumberHidden() || ( true == TASApp.IsTAArtNumberHidden() && true == strLocalArticleNumber.IsEmpty() ) )
			{
				CString str = pclElectroActuator->GetArtNum();

				if( false == str.IsEmpty() )
				{
					AddStaticText( pclSheetDescription, ColumnDescription::ArticleNumber, lRow, pclElectroActuator->GetArtNum() );
					fArticleNumberShown = true;
				}
			}

			// If local article number exist...
			long lNextRow = lRow + ( ( true == fArticleNumberShown ) ? 1 : 0 );

			if( true == TASApp.IsLocalArtNumberUsed() && false == strLocalArticleNumber.IsEmpty() )
			{
				CString strPrefixLAN = TASApp.GetLocalArtNumberName();

				if( false == strPrefixLAN.IsEmpty() )
				{
					lNextRow = AddStaticText( pclSheetDescription, ColumnDescription::ArticleNumber, lNextRow, strPrefixLAN );
				}

				AddStaticText( pclSheetDescription, ColumnDescription::ArticleNumber, lNextRow, strLocalArticleNumber );
			}
		}

		long lFlagRow = pclSheet->GetMaxRows() + 1;

		if( true == pclElectroActuator->IsDeleted() )
		{
			AddStaticText( pclSheetDescription, ColumnDescription::ArticleNumber, lFlagRow, TASApp.LoadLocalizedString( IDS_TAPDELETED ) );
		}
		else if( false == pclElectroActuator->IsAvailable() )
		{
			AddStaticText( pclSheetDescription, ColumnDescription::ArticleNumber, lFlagRow, TASApp.LoadLocalizedString( IDS_TAPNOTAVAILABLE ) );
		}

		// Write nothing for technical info.
		AddStaticText( pclSheetDescription, ColumnDescription::TechnicalInfos, lRow, _T("-") );

		// Write quantity and prices.
		FillQtyPriceCol( pclSheetDescription, lRow, pclElectroActuator, 1 );

		// Write the actuator name.
		lRow = AddStaticText( pclSheetDescription, ColumnDescription::Product, lRow, pclElectroActuator->GetName() );
		pclSheet->SetTextPatternProperty( CSSheet::_SSTextPatternProperty::TPP_StyleHorizontalAlign, SSS_ALIGN_LEFT );

		if( pclControlValve != NULL )
		{
			// Set the Close off Dp if exist.
			CDB_CloseOffChar *pCloseOffChar = ( CDB_CloseOffChar * )pclControlValve->GetCloseOffCharIDPtr().MP;
			
			if( NULL != pCloseOffChar )
			{
				if( CDB_CloseOffChar::CloseOffDp == pCloseOffChar->GetLimitType() )
				{
					str = TASApp.LoadLocalizedString( IDS_SSHEETSELPROD_CLOSEOFFDP );
					double dCloseOffDp = pCloseOffChar->GetCloseOffDp( pclElectroActuator->GetMaxForceTorque() );

					if( dCloseOffDp > 0.0 )
					{
						str += CString( _T(" : ") ) + WriteCUDouble( _U_DIFFPRESS, dCloseOffDp, true );
					}
				}
				else if( CDB_CloseOffChar::InletPressure == pCloseOffChar->GetLimitType() )
				{
					str = TASApp.LoadLocalizedString( IDS_SSHEETSELPROD_MAXINLETPRESSURE );
					double dMaxInletPressure = pCloseOffChar->GetMaxInletPressure( pclElectroActuator->GetMaxForceTorque() );

					if( dMaxInletPressure > 0.0 )
					{
						str += CString( _T(" : ") ) + WriteCUDouble( _U_PRESSURE, dMaxInletPressure, true );
					}
				}

				lRow = AddStaticText( pclSheetDescription, ColumnDescription::Product, lRow, str );
				AddCellSpanW( pclSheetDescription, ColumnDescription::Product, lRow - 1, 2, 1 );
			}

			// Compute Actuating time in sec.
			str = TASApp.LoadLocalizedString( IDS_SSHEETSELPROD_ACTUATING_TIME ) + CString( _T(" : ") )	+ pclElectroActuator->GetActuatingTimesStr(
					  pclControlValve->GetStroke(), true );
			lRow = AddStaticText( pclSheetDescription, ColumnDescription::Product, lRow, str );
			AddCellSpanW( pclSheetDescription, ColumnDescription::Product, lRow - 1, 2, 1 );
		}

		// Write IP.
		str = TASApp.LoadLocalizedString( IDS_SSHEETSELPROD_IPXXAUTO );
		str += CString( _T(" : ") ) + pclElectroActuator->GetIPxxAuto();

		if( false == pclElectroActuator->GetIPxxManual().IsEmpty() )
		{
			str += _T("(" ) + pclElectroActuator->GetIPxxManual() + _T( ")");
		}

		AddStaticText( pclSheetDescription, ColumnDescription::Product, lRow, str );
		AddCellSpanW( pclSheetDescription, ColumnDescription::Product, lRow++, 2, 1 );

		// Write power supply.
		str = TASApp.LoadLocalizedString( IDS_SSHEETSELPROD_POWSUPPL ) + CString( _T(" : ") ) + pclElectroActuator->GetPowerSupplyStr();
		AddStaticText( pclSheetDescription, ColumnDescription::Product, lRow, str );
		AddCellSpanW( pclSheetDescription, ColumnDescription::Product, lRow++, 2, 1 );

		// Write input signal.
		str = TASApp.LoadLocalizedString( IDS_SSHEETSELPROD_INPUT ) + CString( _T(" : ") ) + pclElectroActuator->GetInOutSignalsStr( true );
		AddStaticText( pclSheetDescription, ColumnDescription::Product, lRow, str );
		AddCellSpanW( pclSheetDescription, ColumnDescription::Product, lRow++, 2, 1 );

		// Write output signal.
		str = TASApp.LoadLocalizedString( IDS_SSHEETSELPROD_OUTPUTSIG );

		if( CString( _T( "" ) ) == pclElectroActuator->GetInOutSignalsStr( false ) )
		{
			str += CString( _T(" :    -") );
		}
		else
		{
			str += CString( _T(" : ") ) + pclElectroActuator->GetInOutSignalsStr( false );
		}

		AddStaticText( pclSheetDescription, ColumnDescription::Product, lRow, str );
		AddCellSpanW( pclSheetDescription, ColumnDescription::Product, lRow++, 2, 1 );

		// Write fail safe.
		str = TASApp.LoadLocalizedString( IDS_SSHEETSELPROD_FAILSAFE );

		if( pclElectroActuator->GetFailSafe() == CDB_ElectroActuator::FailSafeType::eFSTypeElectronic )
		{
			str += CString( _T( " : " ) ) + TASApp.LoadLocalizedString( IDS_STRELECFAILSAFE );
		}
		else if( pclElectroActuator->GetFailSafe() == CDB_ElectroActuator::FailSafeType::eFSTypeSpringReturn )
		{
			str += CString( _T( " : " ) ) + TASApp.LoadLocalizedString( IDS_STRSPRINGFAILSAFE );
		}
		else
		{
			str += CString( _T( " : " ) ) + TASApp.LoadLocalizedString( IDS_NO );
		}

		AddStaticText( pclSheetDescription, ColumnDescription::Product, lRow, str );
		AddCellSpanW( pclSheetDescription, ColumnDescription::Product, lRow++, 2, 1 );

		// Write Default return position
		str = ( (int)pclElectroActuator->GetFailSafe() > 0 ) ? TASApp.LoadLocalizedString( IDS_SSHEETSSEL_ACTRFSP ) : TASApp.LoadLocalizedString( IDS_SSHEETSSEL_ACTRDRP );
		if( CString( _T( "" ) ) == pclElectroActuator->GetDefaultReturnPosStr( pclElectroActuator->GetDefaultReturnPos() ).c_str() )
		{
			str += CString( _T(" :    -") );
		}
		else
		{
			str += CString( _T(" : ") ) + pclElectroActuator->GetDefaultReturnPosStr( pclElectroActuator->GetDefaultReturnPos() ).c_str();
		}

		AddStaticText( pclSheetDescription, ColumnDescription::Product, lRow, str );
		AddCellSpanW( pclSheetDescription, ColumnDescription::Product, lRow++, 2, 1 );

		// Center the text below.
		pclSheet->SetTextPatternProperty( CSSheet::_SSTextPatternProperty::TPP_StyleHorizontalAlign, ( LPARAM )SSS_ALIGN_CENTER );

		m_rProductParam.AddRange( lFirstRow, pclSheet->GetMaxRows(), pclElectroActuator );
	}

	return pclSheet->GetMaxRows();
}

long CSelProdPageDpCBCV::_FillAdapterFromHM( CSheetDescription *pclSheetDescription, long lRow, CDS_HydroMod::CCv *pclHMCV )
{
	if( NULL == pclSheetDescription || NULL == pclSheetDescription->GetSSheetPointer() || NULL == pclHMCV )
	{
		return lRow;
	}

	CDB_Product *pclAdapter = dynamic_cast<CDB_Product *>( ( CData * )pclHMCV->GetActrAdaptIDPtr().MP );

	if( NULL == pclAdapter || false == pclAdapter->IsAnAccessory() )
	{
		return lRow;
	}

	bool fWithArticleNumber = ( eBool3::eb3True == pclHMCV->GetActrSelectedAsaPackage() ) ? false : true;
	lRow = FillAccessory( pclSheetDescription, lRow, pclAdapter, fWithArticleNumber, 1 );

	return pclSheetDescription->GetSSheetPointer()->GetMaxRows();
}

void CSelProdPageDpCBCV::_AddArticleList( CDB_Actuator *pclActuator, CAccessoryList *pclAccessoryList, int iQuantity, bool fSelectionBySet )
{
	// At now we accept only electric and thermoelectric actuator.
	CDB_ElectroActuator *pElectroActuator = dynamic_cast<CDB_ElectroActuator *>( pclActuator );

	if( NULL == pElectroActuator )
	{
		return;
	}

	CArticleGroup *pclArticleGroup = new CArticleGroup();

	if( NULL == pclArticleGroup )
	{
		return;
	}

	CArticleItem *pclArticleItem = new CArticleItem();

	if( NULL == pclArticleItem )
	{
		delete pclArticleGroup;
		return;
	}

	// Customize the actuator information.
	pclArticleItem->SetID( pElectroActuator->GetIDPtr().ID );

	if( CDB_CloseOffChar::Linear == pElectroActuator->GetOpeningType() )
	{
		pclArticleItem->SetDescription( pElectroActuator->GetName() + CString( _T("; ") ) );
		pclArticleItem->AddDescription( TASApp.LoadLocalizedString( IDS_SSHEETSELPROD_ACTFORCE ) + CString( _T(" = ") ) );
		pclArticleItem->AddDescription( WriteCUDouble( _U_FORCE, pElectroActuator->GetMaxForceTorque(), true ) + CString( _T("; ") ) );
		pclArticleItem->AddDescription( pElectroActuator->GetPowerSupplyStr() );
	}
	else
	{
		pclArticleItem->SetDescription( pElectroActuator->GetName() );
		pclArticleItem->AddDescription( CString( _T("; ") ) );
		pclArticleItem->AddDescription( TASApp.LoadLocalizedString( IDS_SSHEETSELPROD_ACTTORQUE ) );
		pclArticleItem->AddDescription( CString( _T(" = ") ) );
		pclArticleItem->AddDescription( WriteCUDouble( _U_FORCE, pElectroActuator->GetMaxForceTorque(), true ) );
		pclArticleItem->AddDescription( CString( _T("; ") ) );
		pclArticleItem->AddDescription( TASApp.LoadLocalizedString( IDS_SSHEETSELPROD_POWSUPPL ) );
		pclArticleItem->AddDescription( CString( _T(" : ") ) );
		pclArticleItem->AddDescription( pElectroActuator->GetPowerSupplyStr() );
	}

	pclArticleItem->SetPrice( TASApp.GetpTADB()->GetPrice( pElectroActuator->GetArtNum( true ) ) );
	pclArticleItem->SetQuantity( iQuantity );
	pclArticleItem->SetIsAvailable( pElectroActuator->IsAvailable() );
	pclArticleItem->SetIsDeleted( pElectroActuator->IsDeleted() );

	CString strArticleNumber = pElectroActuator->GetArtNum();
	CString strLocArtNumber = LocArtNumTab.GetLocalArticleNumber( strArticleNumber );
	pclArticleItem->CheckThingAvailability( pElectroActuator, strArticleNumber, strLocArtNumber );

	if( true == strLocArtNumber.IsEmpty() )
	{
		strLocArtNumber = _T("-");
	}

	pclArticleItem->SetArticle( strArticleNumber );
	pclArticleItem->SetLocArtNum( strLocArtNumber );

	CArticleContainer *pclArticleContainer = pclArticleGroup->AddArticle( pclArticleItem );

	// Remark: 'AddArticle' will call new CArticleContainer( pclArticleItem ) that copies content of 'pclArticleItem' and not saves the pointer.
	delete pclArticleItem;

	AddAccessoriesInArticleContainer( pclAccessoryList, pclArticleContainer, iQuantity, fSelectionBySet );

	CSelProdPageBase::AddArticleInGroupList( pclArticleGroup );

	// Remark: pay attention that 'CPage::AddArticleInGroupList' will increment quantity if article already exist or
	//         will create a new 'CArticleGroup' and will copy to it data passed as argument. This is the reason
	//         why you need to delete 'pclArticleGroup' after the call.
	delete pclArticleGroup;
}

void CSelProdPageDpCBCV::_AddArticleList( CDB_Actuator *pclActuator, std::vector<CDS_Actuator::AccessoryItem> *pvecAccessoryList, int iQuantity,
		bool fSelectionBySet )
{
	if( NULL == pclActuator || NULL == pvecAccessoryList || 0 == iQuantity )
	{
		return;
	}

	CArticleGroup *pclArticleGroup = new CArticleGroup();

	if( NULL == pclArticleGroup )
	{
		return;
	}

	CArticleItem *pclArticleItem = new CArticleItem();

	if( NULL == pclArticleItem )
	{
		delete pclArticleGroup;
		return;
	}

	// Customize the actuator information.
	pclArticleItem->SetID( pclActuator->GetIDPtr().ID );

	CDB_ElectroActuator *pclElectroActuator = dynamic_cast<CDB_ElectroActuator *>( pclActuator );
	CDB_ThermostaticActuator *pclThermostaticActuator = dynamic_cast<CDB_ThermostaticActuator *>( pclActuator );

	if( NULL != pclElectroActuator )
	{
		if( CDB_CloseOffChar::Linear == pclElectroActuator->GetOpeningType() )
		{
			pclArticleItem->SetDescription( pclElectroActuator->GetName() + CString( _T("; ") ) );
			pclArticleItem->AddDescription( TASApp.LoadLocalizedString( IDS_SSHEETSELPROD_ACTFORCE ) + CString( _T(" = ") ) );
			pclArticleItem->AddDescription( WriteCUDouble( _U_FORCE, pclElectroActuator->GetMaxForceTorque(), true ) + CString( _T("; ") ) );
			pclArticleItem->AddDescription( pclElectroActuator->GetPowerSupplyStr() );
		}
		else
		{
			pclArticleItem->SetDescription( pclElectroActuator->GetName() );
			pclArticleItem->AddDescription( CString( _T("; ") ) );
			pclArticleItem->AddDescription( TASApp.LoadLocalizedString( IDS_SSHEETSELPROD_ACTTORQUE ) );
			pclArticleItem->AddDescription( CString( _T(" = ") ) );
			pclArticleItem->AddDescription( WriteCUDouble( _U_FORCE, pclElectroActuator->GetMaxForceTorque(), true ) );
			pclArticleItem->AddDescription( CString( _T("; ") ) );
			pclArticleItem->AddDescription( TASApp.LoadLocalizedString( IDS_SSHEETSELPROD_POWSUPPL ) );
			pclArticleItem->AddDescription( CString( _T(" : ") ) );
			pclArticleItem->AddDescription( pclElectroActuator->GetPowerSupplyStr() );
		}
	}
	else if( NULL != pclThermostaticActuator )
	{
		pclArticleItem->SetDescription( pclThermostaticActuator->GetName() + CString( _T("; ") ) );

		// Temperature setting.
		int iMinSetting = pclThermostaticActuator->GetMinSetting();
		int iMaxSetting = pclThermostaticActuator->GetMaxSetting();
		CString str = TASApp.LoadLocalizedString( IDS_SSHEETSELPROD_SETTINGRANGE );

		// HYS-951: Min setting can be 0.
		if( iMinSetting >= 0 && iMaxSetting > 0 )
		{
			str += CString( _T(" : ") ) + WriteCUDouble( _U_TEMPERATURE, iMinSetting, true );
			str += CString( _T(" - ") ) + WriteCUDouble( _U_TEMPERATURE, iMaxSetting, true );
		}
		else
		{
			str += CString( _T(" : - ") );
		}

		pclArticleItem->AddDescription( str + CString( _T("; ") ) );

		// Capillary length.
		int iCapillaryLength = pclThermostaticActuator->GetCapillaryLength();
		str = TASApp.LoadLocalizedString( IDS_SSHEETSELPROD_CAPILLARYLEN );

		if( iCapillaryLength != -1 && iCapillaryLength > 0 )
		{
			str += CString( _T(" : ") ) + WriteCUDouble( _U_LENGTH, iCapillaryLength, true );
		}
		else
		{
			str += CString( _T(" : - ") );
		}

		pclArticleItem->AddDescription( str + CString( _T("; ") ) );

		// Hysteresis.
		double dHysteresis = pclThermostaticActuator->GetHysteresis();
		str = TASApp.LoadLocalizedString( IDS_SSHEETSELPROD_HYSTERESIS );

		if( dHysteresis != -1.0 && dHysteresis > 0.0 )
		{
			str += CString( _T(" : " ) ) + WriteDouble( dHysteresis, 3, 1, 1 ) + CString( _T( " K") );
		}
		else
		{
			str += CString( _T(" : - ") );
		}

		pclArticleItem->AddDescription( str + CString( _T("; ") ) );

		// Frost protection.
		int iFrostProtection = pclThermostaticActuator->GetFrostProtection();
		str = TASApp.LoadLocalizedString( IDS_SSHEETSELPROD_FROSTPROTECTION );

		if( iFrostProtection != -1 && iFrostProtection > 0 )
		{
			str += CString( _T(" : ") ) + WriteCUDouble( _U_TEMPERATURE, iFrostProtection, true );
		}
		else
		{
			str += CString( _T(" : - ") );
		}

		pclArticleItem->AddDescription( str );
	}

	pclArticleItem->SetPrice( TASApp.GetpTADB()->GetPrice( pclActuator->GetArtNum( true ) ) );
	pclArticleItem->SetQuantity( iQuantity );
	pclArticleItem->SetIsAvailable( pclActuator->IsAvailable() );
	pclArticleItem->SetIsDeleted( pclActuator->IsDeleted() );

	CString strArticleNumber = pclActuator->GetArtNum();
	CString strLocArtNumber = LocArtNumTab.GetLocalArticleNumber( strArticleNumber );
	pclArticleItem->CheckThingAvailability( pclActuator, strArticleNumber, strLocArtNumber );

	if( true == strLocArtNumber.IsEmpty() )
	{
		strLocArtNumber = _T("-");
	}

	pclArticleItem->SetArticle( strArticleNumber );
	pclArticleItem->SetLocArtNum( strLocArtNumber );

	CArticleContainer *pclArticleContainer = pclArticleGroup->AddArticle( pclArticleItem );

	// Remark: 'AddArticle' will call new CArticleContainer( pclArticleItem ) that copies content of 'pclArticleItem' and not saves the pointer.
	delete pclArticleItem;

	AddAccessoriesInArticleContainer( pvecAccessoryList, pclArticleContainer, iQuantity, fSelectionBySet );

	CSelProdPageBase::AddArticleInGroupList( pclArticleGroup );

	// Remark: pay attention that 'CPage::AddArticleInGroupList' will increment quantity if article already exist or
	//         will create a new 'CArticleGroup' and will copy to it data passed as argument. This is the reason
	//         why you need to delete 'pclArticleGroup' after the call.
	delete pclArticleGroup;
}

void CSelProdPageDpCBCV::_AddArticleList( CDB_Actuator* pclActuator, CArray<IDPTR>* parAccessory, int iQuantity, bool fSelectionBySet )
{
	if( NULL == pclActuator || NULL == parAccessory || 0 == iQuantity )
	{
		return;
	}

	std::vector<CDS_Actuator::AccessoryItem> vecAccessoryList;

	for( int iLoop = 0; iLoop < parAccessory->GetCount(); iLoop++ )
	{
		CDS_Actuator::AccessoryItem rAccessoryItem;
		rAccessoryItem.IDPtr = parAccessory->GetAt( iLoop );
		rAccessoryItem.fByPair = false;
	}
	
	_AddArticleList( pclActuator, &vecAccessoryList, iQuantity, fSelectionBySet );
}

void CSelProdPageDpCBCV::_AddArticleList( CDS_SSelDpCBCV *pclSSelDpCBCV )
{
	if( NULL == pclSSelDpCBCV )
	{
		return;
	}

	bool fAddActuator = true;

	// Add article number for control valve.
	CDB_DpCBCValve *pclDpCBCValve = pclSSelDpCBCV->GetProductAs<CDB_DpCBCValve>();

	if( pclDpCBCValve != NULL )
	{
		CArticleGroup *pclArticleGroup = new CArticleGroup();

		if( NULL == pclArticleGroup )
		{
			return;
		}

		CArticleItem *pclArticleItem = new CArticleItem();

		if( NULL == pclArticleItem )
		{
			delete pclArticleGroup;
			return;
		}

		CString str;

		// If user has done a selection by set and if there is a DpCBCV-Actuator set...
		if( true == pclSSelDpCBCV->IsSelectedAsAPackage( true ) && NULL != dynamic_cast<CDB_Set *>( pclSSelDpCBCV->GetCvActrSetIDPtr().MP ) )
		{
			CDB_Set *pDpCBCVActSet = dynamic_cast<CDB_Set *>( pclSSelDpCBCV->GetCvActrSetIDPtr().MP );

			pclArticleItem->SetID( pDpCBCVActSet->GetIDPtr().ID );

			CString strArticleNumber = pDpCBCVActSet->GetReference();
			CString strLocArtNumber = LocArtNumTab.GetLocalArticleNumber( strArticleNumber );
			pclArticleItem->CheckThingAvailability( pDpCBCVActSet, strArticleNumber, strLocArtNumber );

			if( true == strLocArtNumber.IsEmpty() )
			{
				strLocArtNumber = _T("-");
			}

			pclArticleItem->SetArticle( strArticleNumber );
			pclArticleItem->SetLocArtNum( strLocArtNumber );
			str = pDpCBCVActSet->GetName() + CString( _T(" ") ) + TASApp.LoadLocalizedString( IDS_SSHEETSELPROD_SET );
			pclArticleItem->SetDescription( str );
			pclArticleItem->SetQuantity( pclSSelDpCBCV->GetpSelectedInfos()->GetQuantity() );

			pclArticleItem->SetIsAvailable( pDpCBCVActSet->IsAvailable() );
			pclArticleItem->SetIsDeleted( pDpCBCVActSet->IsDeleted() );

			// Add valve accessories.
			CArticleContainer *pclArticleContainer = pclArticleGroup->AddArticle( pclArticleItem );

			// Remark: 'AddArticle' will call new CArticleContainer( pclArticleItem ) that copies content of 'pclArticleItem' and not saves the pointer.
			delete pclArticleItem;

			AddAccessoriesInArticleContainer( pclSSelDpCBCV->GetCvAccessoryList(), pclArticleContainer, pclSSelDpCBCV->GetpSelectedInfos()->GetQuantity(), true );

			// Add actuator accessories except for those that are already included into the set.
			CAccessoryList clActAccessoriesToAdd;
			CDB_RuledTable *pAccTab = dynamic_cast<CDB_RuledTable *>( pDpCBCVActSet->GetAccGroupIDPtr().MP );
			CAccessoryList *pclActuatorAccessoryList = pclSSelDpCBCV->GetActuatorAccessoryList();

			CAccessoryList::AccessoryItem rAccessoryItem = pclActuatorAccessoryList->GetFirst();

			while( rAccessoryItem.IDPtr.MP != NULL )
			{
				// If accessory is not included into CvActSet, add it.
				if( NULL == pAccTab || '\0' == *pAccTab->Get( rAccessoryItem.IDPtr.ID ).ID )
				{
					clActAccessoriesToAdd.Add( CAccessoryList::AccessoryType::_AT_Accessory, &rAccessoryItem );
				}

				rAccessoryItem = pclActuatorAccessoryList->GetNext();
			}

			AddAccessoriesInArticleContainer( &clActAccessoriesToAdd, pclArticleContainer, pclSSelDpCBCV->GetpSelectedInfos()->GetQuantity(), true );

			CSelProdPageBase::AddArticleInGroupList( pclArticleGroup );

			// Remark: pay attention that 'CPage::AddArticleInGroupList' will increment quantity if article already exist or
			//         will create a new 'CArticleGroup' and will copy to it data passed as argument. This is the reason
			//         why you need to delete 'pclArticleGroup' after the call.
			delete pclArticleGroup;

			// All has been already done for actuators and its accessories.
			fAddActuator = false;
		}
		else
		{
			pclArticleItem->SetID( pclDpCBCValve->GetIDPtr().ID );
			pclArticleItem->SetArticle( pclDpCBCValve->GetBodyArtNum() );

			// Check if article is available and not deleted.
			CString strArticleNumber = pclDpCBCValve->GetBodyArtNum();
			CString strLocArtNum = LocArtNumTab.GetLocalArticleNumber( strArticleNumber );
			pclArticleItem->CheckThingAvailability( pclDpCBCValve, strArticleNumber, strLocArtNum );

			if( true == strLocArtNum.IsEmpty() )
			{
				strLocArtNum = _T("-");
			}

			pclArticleItem->SetArticle( strArticleNumber );
			pclArticleItem->SetLocArtNum( strLocArtNum );

			pclArticleItem->SetQuantity( pclSSelDpCBCV->GetpSelectedInfos()->GetQuantity() );
			pclArticleItem->SetDescription( pclDpCBCValve->GetName() + CString( _T("; ") ) );

			// Add Dpl range.
			double dRho = pclSSelDpCBCV->GetpSelectedInfos()->GetpWCData()->GetDens();
			pclArticleItem->AddDescription( pclDpCBCValve->GetFormatedDplRange( pclSSelDpCBCV->GetQ(), dRho ).c_str() + CString( _T("; ") ) );
			
			// Add version.
			pclArticleItem->AddDescription( ( ( CDB_StringID * )pclDpCBCValve->GetVersionIDPtr().MP )->GetString() );

			str = ( ( CDB_StringID * )pclDpCBCValve->GetConnectIDPtr().MP )->GetString();
			str = str.Right( str.GetLength() - str.Find( '/' ) - 1 );
			pclArticleItem->AddDescription( CString( _T("; ") ) + str );

			pclArticleItem->SetPrice( TASApp.GetpTADB()->GetPrice( pclDpCBCValve->GetArtNum( true ) ) );
			pclArticleItem->SetIsAvailable( pclDpCBCValve->IsAvailable() );
			pclArticleItem->SetIsDeleted( pclDpCBCValve->IsDeleted() );

			// Add all Article Accessories.
			CArticleContainer *pclArticleContainer = pclArticleGroup->AddArticle( pclArticleItem );

			// Remark: 'AddArticle' will call new CArticleContainer( pclArticleItem ) that copies content of 'pclArticleItem' and not saves the pointer.
			delete pclArticleItem;

			AddAccessoriesInArticleContainer( pclSSelDpCBCV->GetCvAccessoryList(), pclArticleContainer, pclSSelDpCBCV->GetpSelectedInfos()->GetQuantity(), false );

			CSelProdPageBase::AddArticleInGroupList( pclArticleGroup );

			// Remark: pay attention that 'CPage::AddArticleInGroupList' will increment quantity if article already exist or
			//         will create a new 'CArticleGroup' and will copy to it data passed as argument. This is the reason
			//         why you need to delete 'pclArticlegroup' after the call.
			delete pclArticleGroup;

			// Add connection component if needed.
			if( true == pclDpCBCValve->IsConnTabUsed() )
			{
				// Inlet.
				CSelProdPageBase::AddCompArtList( pclDpCBCValve, pclSSelDpCBCV->GetpSelectedInfos()->GetQuantity(), true );
				// Outlet.
				CSelProdPageBase::AddCompArtList( pclDpCBCValve, pclSSelDpCBCV->GetpSelectedInfos()->GetQuantity(), false );
			}
		}
	}

	if( true == fAddActuator )
	{
		CDB_Actuator *pclActuator = dynamic_cast<CDB_Actuator *>( pclSSelDpCBCV->GetActrIDPtr().MP );

		if( pclActuator != NULL )
		{
			_AddArticleList( pclActuator, pclSSelDpCBCV->GetActuatorAccessoryList(), pclSSelDpCBCV->GetpSelectedInfos()->GetQuantity(),
							 pclSSelDpCBCV->IsSelectedAsAPackage( true ) );
		}
	}
}

void CSelProdPageDpCBCV::_AddArticleList( CDS_HydroMod::CCv *pclHMCV )
{
	if( NULL == pclHMCV )
	{
		return;
	}

	CDB_DpCBCValve *pclDpCBCValve = dynamic_cast<CDB_DpCBCValve *>( pclHMCV->GetCvIDPtr().MP );

	if( NULL != pclDpCBCValve )
	{
		CArticleGroup *pclArticleGroup = new CArticleGroup();

		if( NULL == pclArticleGroup )
		{
			return;
		}

		CArticleItem *pclArticleItem = new CArticleItem();

		if( NULL == pclArticleItem )
		{
			delete pclArticleGroup;
			return;
		}

		CString str;

		// If selection has been done by a set...
		if( true == pclDpCBCValve->IsPartOfaSet() && eBool3::eb3True == pclHMCV->GetActrSelectedAsaPackage() )
		{
			CDB_Set *pCvActSet = NULL;
			CDB_Actuator *pclActuator = dynamic_cast<CDB_Actuator *>( pclHMCV->GetActrIDPtr().MP );

			if( NULL != pclActuator )
			{
				// Fill CvActSet if needed.
				CTableSet *pTabSet = pclDpCBCValve->GetTableSet();
				ASSERT( NULL != pTabSet );
				pCvActSet = pTabSet->FindCompatibleSet( pclDpCBCValve->GetIDPtr().ID, pclActuator->GetIDPtr().ID );
			}

			if( NULL == pCvActSet )
			{
				ASSERT( 0 );
				return;
			}

			pclArticleItem->SetID( pCvActSet->GetIDPtr().ID );

			CString strArticleNumber = pCvActSet->GetReference();
			CString strLocArtNumber = LocArtNumTab.GetLocalArticleNumber( strArticleNumber );
			pclArticleItem->CheckThingAvailability( pCvActSet, strArticleNumber, strLocArtNumber );

			if( true == strLocArtNumber.IsEmpty() )
			{
				strLocArtNumber = _T("-");
			}

			pclArticleItem->SetArticle( strArticleNumber );
			pclArticleItem->SetLocArtNum( strLocArtNumber );
			str = pCvActSet->GetName() + CString( _T(" ") ) + TASApp.LoadLocalizedString( IDS_SSHEETSELPROD_SET );
			pclArticleItem->SetDescription( str );
			pclArticleItem->SetQuantity( 1 );

			pclArticleItem->SetIsAvailable( pCvActSet->IsAvailable() );
			pclArticleItem->SetIsDeleted( pCvActSet->IsDeleted() );

			CArticleContainer *pclArticleContainer = pclArticleGroup->AddArticle( pclArticleItem );

			// Remark: 'AddArticle' will call new CArticleContainer( pclArticleItem ) that copies content of 'pclArticleItem' and not saves the pointer.
			delete pclArticleItem;

			// Add control valve and actuator accessories belonging to the set.
			CDB_RuledTable *pclRuledTable = dynamic_cast<CDB_RuledTable *>( pCvActSet->GetAccGroupIDPtr().MP );

			if( NULL != pclRuledTable && pclHMCV->GetCVActrAccSetCount() > 0 )
			{
				CAccessoryList clAccessoryList;

				for( int iLoop = 0; iLoop < pclHMCV->GetCVActrAccSetCount(); iLoop++ )
				{
					CDB_Product *pclAccessory = (CDB_Product * )( pclHMCV->GetCVActrAccSetIDPtr( iLoop ).MP );

					if( NULL == pclAccessory || false == pclAccessory->IsAnAccessory() )
					{
						continue;
					}

					clAccessoryList.Add( pclAccessory->GetIDPtr(), CAccessoryList::_AT_SetAccessory, pclRuledTable );
				}

				AddAccessoriesInArticleContainer( &clAccessoryList, pclArticleContainer, 1, true );
			}

			// Add valve adapters.
			if( NULL != pclHMCV->GetActrAdaptIDPtr().MP )
			{
				CAccessoryList clAccessoryList;
				clAccessoryList.Add( pclHMCV->GetActrAdaptIDPtr(), CAccessoryList::_AT_Adapter );
				AddAccessoriesInArticleContainer( &clAccessoryList, pclArticleContainer, 1, true );
			}

			CSelProdPageBase::AddArticleInGroupList( pclArticleGroup );

			// Remark: pay attention that 'CPage::AddArticleInGroupList' will increment quantity if article already exist or
			//         will create a new 'CArticleGroup' and will copy to it data passed as argument. This is the reason
			//         why you need to delete 'pclArticleGroup' after the call.
			delete pclArticleGroup;

			// All has been already done for actuator and its accessories.
		}
		else
		{
			pclArticleItem->SetID( pclDpCBCValve->GetIDPtr().ID );
			pclArticleItem->SetArticle( pclDpCBCValve->GetBodyArtNum() );

			// Check if article is available and not deleted.
			CString strArticleNumber = pclDpCBCValve->GetBodyArtNum();
			CString strLocArtNum = LocArtNumTab.GetLocalArticleNumber( strArticleNumber );
			pclArticleItem->CheckThingAvailability( pclDpCBCValve, strArticleNumber, strLocArtNum );

			if( true == strLocArtNum.IsEmpty() )
			{
				strLocArtNum = _T("-");
			}

			pclArticleItem->SetArticle( strArticleNumber );
			pclArticleItem->SetLocArtNum( strLocArtNum );

			pclArticleItem->SetQuantity( 1 );
			pclArticleItem->SetDescription( pclDpCBCValve->GetName() + CString( _T("; ") ) );

			pclArticleItem->AddDescription( ( ( CDB_StringID * )pclDpCBCValve->GetVersionIDPtr().MP )->GetString() );

			str = ( ( CDB_StringID * )pclDpCBCValve->GetConnectIDPtr().MP )->GetString();
			str = str.Right( str.GetLength() - str.Find( '/' ) - 1 );
			pclArticleItem->AddDescription( CString( _T("; ") ) + str );

			pclArticleItem->SetPrice( TASApp.GetpTADB()->GetPrice( pclDpCBCValve->GetArtNum( true ) ) );
			pclArticleItem->SetIsAvailable( pclDpCBCValve->IsAvailable() );
			pclArticleItem->SetIsDeleted( pclDpCBCValve->IsDeleted() );

			CArticleContainer *pclArticleContainer = pclArticleGroup->AddArticle( pclArticleItem );

			// Remark: 'AddArticle' will call new CArticleContainer( pclArticleItem ) that copies content of 'pclArticleItem' and not saves the pointer.
			delete pclArticleItem;

			// Add all article accessories.
			CDB_RuledTable *pclRuledTable = dynamic_cast<CDB_RuledTable *>( pclDpCBCValve->GetAccessoriesGroupIDPtr().MP );

			if( NULL != pclRuledTable && pclHMCV->GetCVAccCount() > 0 )
			{
				CAccessoryList clAccessoryList;

				for( int iLoop = 0; iLoop < pclHMCV->GetCVAccCount(); iLoop++ )
				{
					CDB_Product *pclAccessory = (CDB_Product * )( pclHMCV->GetCVAccIDPtr( iLoop ).MP );

					if( NULL == pclAccessory || pclAccessory->IsAnAccessory() )
					{
						continue;
					}

					clAccessoryList.Add( pclAccessory->GetIDPtr(), CAccessoryList::_AT_Accessory, pclRuledTable );
				}

				AddAccessoriesInArticleContainer( &clAccessoryList, pclArticleContainer, 1, false );
			}

			// Add valve adapters.
			if( NULL != pclHMCV->GetActrAdaptIDPtr().MP )
			{
				CAccessoryList clAccessoryList;
				clAccessoryList.Add( pclHMCV->GetActrAdaptIDPtr(), CAccessoryList::_AT_Adapter );
				AddAccessoriesInArticleContainer( &clAccessoryList, pclArticleContainer, 1, false );
			}

			CSelProdPageBase::AddArticleInGroupList( pclArticleGroup );

			// Remark: pay attention that 'CPage::AddArticleInGroupList' will increment quantity if article already exist or
			//         will create a new 'CArticleGroup' and will copy to it data passed as argument. This is the reason
			//         why you need to delete 'pclArticleGroup' after the call.
			delete pclArticleGroup;

			// Add connection component if needed.
			if( true == pclDpCBCValve->IsConnTabUsed() )
			{
				// Inlet
				CSelProdPageBase::AddCompArtList( pclDpCBCValve, 1, true );
				// Outlet
				CSelProdPageBase::AddCompArtList( pclDpCBCValve, 1, false );
			}

			CDB_Actuator *pclActuator = dynamic_cast<CDB_Actuator *>( pclHMCV->GetActrIDPtr().MP );

			if( pclActuator != NULL )
			{
				_AddArticleList( pclActuator, pclHMCV->GetpActrAccIDPtrArray(), 1, false );
			}
		}
	}
}
