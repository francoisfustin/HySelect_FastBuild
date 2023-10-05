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
#include "SelProdPageSv.h"

CSelProdPageSv::CSelProdPageSv( CArticleGroupList *pclArticleGroupList ) 
	: CSelProdPageBase( CDB_PageSetup::enCheck::SV, pclArticleGroupList )
{
	m_pSelected = NULL;
}

void CSelProdPageSv::Reset( void )
{
	CSelProdPageBase::Reset();
}

bool CSelProdPageSv::PreInit( HMvector& vecHMList )
{
	// Something for individual or direct selection ?
	CTable *pclTable = (CTable*)( TASApp.GetpTADS()->Get( _T("SHUTOFF_TAB") ).MP );	

	if ( NULL == pclTable )
	{
		ASSERT( 0 );
		return false;
	}

	m_pclSelectionTable = ( '\0' != *pclTable->GetFirst().ID ) ? pclTable : NULL;
	bool fSvExist = ( NULL != m_pclSelectionTable );

	// Something for hydronic calculation?
	bool fSvHMExist = ( vecHMList.size() > 0 );

	if( false == fSvExist && false == fSvHMExist )
	{
		return false;
	}
	
	m_vecHMList = vecHMList;
	SortTable();
	return true;
}

bool CSelProdPageSv::Init( bool bResetOrder, bool fPrint )
{
	bool fSvExist = ( NULL != m_pclSelectionTable ) ? true : false;
	bool fSvHMExist = ( m_vecHMList.size() > 0 ) ? true : false;

	if( false == fSvExist && false == fSvHMExist )
	{
		return false;
	}

	// Call base class method to get a new 'Sheet'.
	CSheetDescription* pclSheetDescriptionSv = CMultiSpreadBase::CreateSSheet( SD_ShutoffValve );

	if( NULL == pclSheetDescriptionSv || NULL == pclSheetDescriptionSv->GetSSheetPointer() )
	{
		return false;
	}
	
	CSSheet* pclSheet = pclSheetDescriptionSv->GetSSheetPointer();

	if( false == CSelProdPageBase::PrepareSheet( pclSheetDescriptionSv, ColumnDescription::Pointer, fPrint ) )
	{
		m_ViewDescription.RemoveOneSheetDescription( SD_ShutoffValve );
		return false;
	}

	// Initialize column width for header and footer columns.
	if( false == m_bForPrint )
	{
		SetColWidth( pclSheetDescriptionSv, ColumnDescription::Header, DefaultColumnWidth::DCW_Header );
		SetColWidth( pclSheetDescriptionSv, ColumnDescription::Footer, DefaultColumnWidth::DCW_Footer );
	}
	else
	{
		pclSheet->ShowCol( ColumnDescription::Header, FALSE );
		pclSheet->ShowCol( ColumnDescription::Footer, FALSE );
	}

	double dWidth = ( true == m_pTADSPageSetup->GetField( PageField_enum::epfFIRSTREF ) ) ? DefaultColumnWidth::DCW_Reference1 : 0.0;
	SetColWidth( pclSheetDescriptionSv, ColumnDescription::Reference1, dWidth );

	dWidth = ( true == m_pTADSPageSetup->GetField( PageField_enum::epfSECONDREF ) ) ? DefaultColumnWidth::DCW_Reference2 : 0.0;
	SetColWidth( pclSheetDescriptionSv, ColumnDescription::Reference2, dWidth );
	
	dWidth = ( true == m_pTADSPageSetup->GetField( PageField_enum::epfWATERINFO ) ) ? DefaultColumnWidth::DCW_Water : 0.0;
	SetColWidth( pclSheetDescriptionSv, ColumnDescription::Water, dWidth );
	
	dWidth = ( true == m_pTADSPageSetup->GetField( PageField_enum::epfBVINFOPRODUCT ) ) ? DefaultColumnWidth::DCW_Product : 0.0;
	SetColWidth( pclSheetDescriptionSv, ColumnDescription::Product, dWidth );
	
	dWidth = ( true == m_pTADSPageSetup->GetField( PageField_enum::epfBVTECHINFO ) ) ? DefaultColumnWidth::DCW_TechnicalInfos : 0.0;
	SetColWidth( pclSheetDescriptionSv, ColumnDescription::TechnicalInfos, dWidth );
	
	SetColWidth( pclSheetDescriptionSv, ColumnDescription::RadiatorInfos, 0.0 );
	
	dWidth = ( true == m_pTADSPageSetup->GetField( PageField_enum::epfARTICLE ) ) ? DefaultColumnWidth::DCW_ArticleNumber : 0.0;
	SetColWidth( pclSheetDescriptionSv, ColumnDescription::ArticleNumber, dWidth );
	
	dWidth = ( true == m_pTADSPageSetup->GetField( PageField_enum::epfPIPEINFO ) ) ? DefaultColumnWidth::DCW_Pipes : 0.0;
	SetColWidth( pclSheetDescriptionSv, ColumnDescription::Pipes, dWidth );
	
	dWidth = ( true == m_pTADSPageSetup->GetField( PageField_enum::epfQUANTITY ) ) ? DefaultColumnWidth::DCW_Quantity : 0.0;
	SetColWidth( pclSheetDescriptionSv, ColumnDescription::Quantity, dWidth );
	
	dWidth = 0.0;
	
	if( true == TASApp.IsPriceUsed() )
	{
		dWidth = ( true == m_pTADSPageSetup->GetField( PageField_enum::epfSALEUNITPRICE ) ) ? DefaultColumnWidth::DCW_UnitPrice : 0.0;
	}

	SetColWidth( pclSheetDescriptionSv, ColumnDescription::UnitPrice, dWidth );

	dWidth = 0.0;
	
	if( true == TASApp.IsPriceUsed() )
	{
		dWidth = ( true == m_pTADSPageSetup->GetField( PageField_enum::epfSALETOTALPRICE ) ) ? DefaultColumnWidth::DCW_TotalPrice : 0.0;
	}

	SetColWidth( pclSheetDescriptionSv, ColumnDescription::TotalPrice, dWidth );

	dWidth = ( true == m_pTADSPageSetup->GetField( PageField_enum::epfREMARK ) ) ? DefaultColumnWidth::DCW_Remark : 0.0;
	SetColWidth( pclSheetDescriptionSv, ColumnDescription::Remark, dWidth );
	
	
	SetColWidth( pclSheetDescriptionSv, ColumnDescription::Pointer, DefaultColumnWidth::DCW_Pointer );
	
	pclSheet->ShowCol( ColumnDescription::RadiatorInfos, FALSE );

	if( false == TASApp.IsPriceUsed() )
	{
		pclSheet->ShowCol( ColumnDescription::UnitPrice, FALSE );
		pclSheet->ShowCol( ColumnDescription::TotalPrice, FALSE );
	}

	pclSheet->ShowCol( ColumnDescription::Pointer, FALSE );
	CRect rect = pclSheet->GetSheetSizeInPixels( false );
	m_mapSheetInfos[pclSheetDescriptionSv].m_dPageWidth = rect.Width();

	// Page title.
	SetPageTitle( pclSheetDescriptionSv, IDS_RVIEWSELP_TITLE_SV );
	
	// Init column header.
	_InitColHeader( pclSheetDescriptionSv );
	pclSheet->SetColumnAlwaysHidden( ColumnDescription::RadiatorInfos, true );
	
	// HYS-1571: We can optimize the column width now.
	// See Jira card to have more details about why we do that here.
	CRect rectClient;
	GetClientRect( &rectClient );
	OptimizePageSize( rectClient.Width(), rectClient.Height() );

	// Current position.
	long lRow = pclSheet->GetMaxRows() + 1;
	long lSelectedRow = 0;

	if( true == fSvExist )
	{
		// Filling shut-off valves from SSel or from Advance Hydraulic mode.
		// Create an array of SelManSv pointer.

		// Number of objects.
		int iSvCount = m_pclSelectionTable->GetItemCount( CLASS( CDS_SSelSv ) );

		if( 0 == iSvCount )
		{
			return false;
		}
		
		CDS_SSelSv **paSSelSv = new CDS_SSelSv*[iSvCount];

		if( NULL == paSSelSv )
		{
			return false;
		}
		
		// Fill 'paSSelSv' array with pointer on each 'CDS_SSelSv' object of the 'SHUTOFF_TAB' table.
		int iMaxIndex = 0;
		int i = 0;

		for( IDPTR IDPtr = m_pclSelectionTable->GetFirst( CLASS( CDS_SSelSv ) ); '\0' != *IDPtr.ID; IDPtr = m_pclSelectionTable->GetNext() )
		{
			CDS_SSelSv *pclSSelSv = dynamic_cast<CDS_SSelSv *>( IDPtr.MP );

			if( NULL == pclSSelSv )
			{
				continue;
			}
			
			if( NULL == pclSSelSv->GetIDPtr().MP )
			{
				continue;
			}
			
			paSSelSv[i] = pclSSelSv;

			if( iMaxIndex < paSSelSv[i]->GetpSelectedInfos()->GetRowIndex() )
			{
				iMaxIndex = paSSelSv[i]->GetpSelectedInfos()->GetRowIndex();
			}

			++i;
		}
		
		ASSERT( i == iSvCount );
		iSvCount = i;
		
		// Remark: 'bResetOrder' is set to 'true' only when the sorting combos are empty (No user choice) and the user clicks on the 'Apply sorting keys'.
		if( true == bResetOrder )
		{
			// Reset row index.
			for( i = 0; i < iSvCount; i++ )
			{
				paSSelSv[i]->GetpSelectedInfos()->SetRowIndex( i + 1 );
			}
		}
		else if( PageField_enum::epfNONE != m_pTADSPageSetup->GetKey( 0 ) || PageField_enum::epfNONE != m_pTADSPageSetup->GetKey( 1 ) 
	 			|| PageField_enum::epfNONE != m_pTADSPageSetup->GetKey( 2 ) )
		{
			// This method will sort (or not) in regards to the three sorting keys that user has chosen.
			if( NULL != pDlgLeftTabSelP )
			{
				pDlgLeftTabSelP->SortTable( (CData**)paSSelSv, iSvCount - 1 );
			}

			// Reset row index.
			for( i = 0; i < iSvCount; i++ )
			{
				paSSelSv[i]->GetpSelectedInfos()->SetRowIndex( i + 1 );
			}
		}

		// Create a remark index.
		if( NULL != pDlgLeftTabSelP )
		{
			pDlgLeftTabSelP->SetRemarkIndex( CLASS( CDS_SSelSv ), (CData**)paSSelSv, iSvCount );
		}

		CRank rkl;
		enum eSvSwitch
		{
			SingleSelSv,
			DirSelSv,
			LastSvSwitch
		};

		bool bAtLeastOneBlockPrinted = false;

		for( int iLoopGroup = 0; iLoopGroup < LastSvSwitch; iLoopGroup++ )
		{
			int iCount = 0;

			for( i = 0; i < iSvCount; i++ )
			{
				CDS_SSelSv *pclSelSv = dynamic_cast<CDS_SSelSv *>( paSSelSv[i] );

				if( NULL == pclSelSv || NULL == pclSelSv->GetProductIDPtr().MP )
				{
					ASSERT( 0 );
					continue;
				}

				if( DirSelSv == iLoopGroup )
				{
					if( false == pclSelSv->IsFromDirSel() )
					{
						continue;
					}
				}
				else
				{
					if( true == pclSelSv->IsFromDirSel() )
					{
						continue;
					}
				}

				// All shut-off valve products are based on a 'CDB_TAProduct' object.
				CDB_TAProduct* pclTAProduct = dynamic_cast<CDB_TAProduct *>( pclSelSv->GetProductIDPtr().MP );

				if( NULL == pclTAProduct )
				{
					ASSERT( 0 );
					continue;
				}

				rkl.Add( _T(""), paSSelSv[i]->GetpSelectedInfos()->GetRowIndex(), (LPARAM)paSSelSv[i] );
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
			SetLastRow( pclSheetDescriptionSv, lRow );

			switch( iLoopGroup )
			{
				case DirSelSv:
					SetPageTitle( pclSheetDescriptionSv, IDS_SSHEETSELPROD_SUBTITLESVFROMDIRSEL, false, lRow );
					break;

				case SingleSelSv:
					SetPageTitle( pclSheetDescriptionSv, IDS_SSHEETSELPROD_SUBTITLESVFROMSSEL, false, lRow );
					break;
			}

			lRow = pclSheet->GetMaxRows();
			SetLastRow( pclSheetDescriptionSv, lRow );

			CString str;
			LPARAM lpParam;
			bool bFirstPass = true;

			for( BOOL bContinue = rkl.GetFirst( str, lpParam ); TRUE == bContinue; bContinue = rkl.GetNext( str, lpParam ) )
			{
				CData *pclData = (CData*)lpParam;
				CDS_SSelSv *pclSSelSv = dynamic_cast<CDS_SSelSv *>( pclData );

				if( NULL == pclSSelSv )
				{
					continue;
				}

				if( m_pSelected == pclData )
				{
					lSelectedRow = lRow;
				}

				CDB_TAProduct *pclSv = pclSSelSv->GetProductAs<CDB_TAProduct>();

				if( NULL == pclSv )
				{
					continue;
				}

				m_rProductParam.Clear();
				m_rProductParam.SetSheetDescription( pclSheetDescriptionSv );
				m_rProductParam.SetSelectionContainer( (LPARAM)pclSSelSv );

				++lRow;
				long lFirstRow = lRow;

				long lRowSv = _FillRowSv( pclSheetDescriptionSv, lRow, pclSSelSv, pclSv );
				long lRowGen = CSelProdPageBase::FillRowGen( pclSheetDescriptionSv, lRow, pclSSelSv );
				lRow = max( lRowGen, lRowSv );

				// Add the selectable row range.
				m_rProductParam.AddRange( lFirstRow, pclSheet->GetMaxRows(), pclSv );

				// Spanning must be done here because it's the only place where we know exactly number of lines to span!
				// Span reference #1 and #2.
				AddCellSpanW( pclSheetDescriptionSv, ColumnDescription::Reference1, lFirstRow, 1, pclSheet->GetMaxRows() - lFirstRow + 1 );
				AddCellSpanW( pclSheetDescriptionSv, ColumnDescription::Reference2, lFirstRow, 1, pclSheet->GetMaxRows() - lFirstRow + 1 );

				// Span water characteristic.
				AddCellSpanW( pclSheetDescriptionSv, ColumnDescription::Water, lFirstRow, 1, pclSheet->GetMaxRows() - lFirstRow + 1 );

				// Add accessories.
				if( true == pclSSelSv->IsAccessoryExist() )
				{
					// Draw dash line.
					pclSheet->SetCellBorder( ColumnDescription::Reference1, lRow, ColumnDescription::Footer - 1, lRow, true, SS_BORDERTYPE_BOTTOM, SS_BORDERSTYLE_FINE_DASH );

					lRow++;
					lRow = FillAccessories( pclSheetDescriptionSv, lRow, pclSSelSv, pclSSelSv->GetpSelectedInfos()->GetQuantity() );
				}

				// Fill actuator if exist.
				if( NULL != dynamic_cast<CDB_ElectroActuator *>( pclSSelSv->GetActrIDPtr().MP ) )
				{
					// Draw dash line.
					pclSheet->SetCellBorder( ColumnDescription::Reference1, lRow, ColumnDescription::Footer - 1, lRow, true, SS_BORDERTYPE_BOTTOM, SS_BORDERSTYLE_FINE_DASH );

					lRow++;
					lRow = _FillActuatorSelectedWithSv( pclSheetDescriptionSv, lRow, pclSSelSv );
				}
				
				if( true == pclSSelSv->IsActuatorAccessoryExist() )
				{
					// Draw dash line.
					pclSheet->SetCellBorder( ColumnDescription::Reference1, lRow, ColumnDescription::Footer - 1, lRow, true, SS_BORDERTYPE_BOTTOM, SS_BORDERSTYLE_FINE_DASH );

					lRow++;
					lRow = FillAccessories( pclSheetDescriptionSv, lRow, pclSSelSv->GetActuatorAccessoryList(), pclSSelSv->GetpSelectedInfos()->GetQuantity() );
				}

				// Draw line below.
				pclSheet->SetCellBorder( ColumnDescription::Reference1, lRow, ColumnDescription::Footer - 1, lRow, true, SS_BORDERTYPE_BOTTOM );

				// Save product.
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

		FillRemarks( pclSheetDescriptionSv, lRow );

		// We add by default a blank line.
		lRow = pclSheet->GetMaxRows() + 2;
		
		delete[] paSSelSv;
	}

	if( true == fSvHMExist )
	{
		if( m_vecHMList.size() > 0 )	
		{
			// Create a remark index.
			if( NULL != pDlgLeftTabSelP )
			{
				pDlgLeftTabSelP->SetRemarkIndex( &m_vecHMList );
			}

			SetLastRow( pclSheetDescriptionSv, lRow );
			SetPageTitle( pclSheetDescriptionSv, IDS_SSHEETSELPROD_SUBTITLESVFROMHM, false, lRow );	
			
			lRow++;
			SetLastRow( pclSheetDescriptionSv, lRow );
			
			// For each element.
			bool fFirstPass = true;
			HMvector::iterator It;

			for( It = m_vecHMList.begin(); It != m_vecHMList.end(); It++ )
			{
				CDS_HydroMod* pHM = NULL;
				CDS_HydroMod::CBase* pBase = NULL;

				if( eptHM == (*It).first )
				{
					pHM = dynamic_cast<CDS_HydroMod *>( (CData*)( (*It).second ) );
				}
				else
				{
					// Shut-off valve exist and TA product exist into the DB.
					pBase = static_cast<CDS_HydroMod::CBase *>( (*It).second );

					if( NULL != pBase )
					{
						pHM = pBase->GetpParentHM();
					}
				}

				if( NULL != pHM )
				{
					m_rProductParam.Clear();
					m_rProductParam.SetSheetDescription( pclSheetDescriptionSv );
					// Intentionally set to NULL to avoid clicking, double clicking or right clicking.
					m_rProductParam.SetSelectionContainer( (LPARAM)0 );

					long lRowSv = FillRowSv( pclSheetDescriptionSv, lRow, pHM, pBase );
					long lRowGen = CSelProdPageBase::FillRowGen( pclSheetDescriptionSv, lRow, pHM, pBase );
					long lLastRow = max( lRowGen, lRowSv );

					CDB_TAProduct *pclShutoffValve = NULL;

					// If 'pHM' exists and not 'pclShutoffValve' means that valve has been added in CBI mode.
					if (NULL == pBase)
					{
						pclShutoffValve = dynamic_cast<CDB_TAProduct*>(pHM->GetCBIValveIDPtr().MP);
					}
					else
					{
						CDS_HydroMod::eHMObj eShutoffValveLocated = CDS_HydroMod::eNone;

						if (true == pHM->IsShutoffValveExist(CDS_HydroMod::eShutoffValveSupply, true))
						{
							eShutoffValveLocated = CDS_HydroMod::eShutoffValveSupply;
						}
						else if (true == pHM->IsShutoffValveExist(CDS_HydroMod::eShutoffValveReturn, true))
						{
							eShutoffValveLocated = CDS_HydroMod::eShutoffValveReturn;
						}

						if (CDS_HydroMod::eNone != eShutoffValveLocated && pHM->GetpShutoffValve(eShutoffValveLocated) == pBase)
						{
							pclShutoffValve = dynamic_cast<CDB_TAProduct*>(pHM->GetpShutoffValve(eShutoffValveLocated)->GetIDPtr().MP);
						}
					}

					if (NULL != pclShutoffValve)
					{
						lLastRow = FillAndAddBuiltInHMAccessories(pclSheet, pclSheetDescriptionSv, pclShutoffValve, lLastRow);
					}

					// Draw line below.
					pclSheet->SetCellBorder( ColumnDescription::Reference1, lLastRow, ColumnDescription::Footer - 1, lLastRow, true, SS_BORDERTYPE_BOTTOM );

					// Save the product.
					m_rProductParam.SetScrollRange( ( true == fFirstPass ) ? lRow - 1 : lRow, lLastRow );
					SaveProduct( m_rProductParam );

					// Set all group as no breakable (for print).
					// Remark: include title with the group.
					pclSheet->SetFlagRowNoBreakable( ( true == fFirstPass ) ? lRow - 1 : lRow, lLastRow, true );

					lRow = lLastRow + 1;
					fFirstPass = false;
				}
			}

			FillHMRemarks( &m_vecHMList, pclSheetDescriptionSv, lRow );
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

bool CSelProdPageSv::HasSomethingToDisplay( void )
{
	bool fSvExist = ( NULL != m_pclSelectionTable ) ? true : false;
	bool fSvHMExist = ( m_vecHMList.size() > 0 ) ? true : false;

	if( false == fSvExist && false == fSvHMExist )
	{
		return false;
	}

	return true;
}

long CSelProdPageSv::FillRowSvProduct( CSheetDescription* pclSheetDescription, long lRow, CDB_TAProduct *pclShutoffValve, int iQuantity )
{
	if( NULL == pclSheetDescription || NULL == pclShutoffValve )
	{
		return lRow;
	}

	CString str1, str2;
	long lFirstRow = lRow;
	
	// Column TA Product.
	AddStaticText( pclSheetDescription, ColumnDescription::Product, lRow, _T("-") );

	if( true == m_pTADSPageSetup->GetField( epfSVINFOPRODUCTNAME ) )	
	{
		lRow = AddStaticText( pclSheetDescription, ColumnDescription::Product, lRow, pclShutoffValve->GetName() );
	}

	if( true == m_pTADSPageSetup->GetField( epfSVINFOPRODUCTSIZE ) )	
	{
		lRow = AddStaticText( pclSheetDescription, ColumnDescription::Product, lRow, pclShutoffValve->GetSize() );
	}

	if( true == m_pTADSPageSetup->GetField( epfSVINFOPRODUCTCONNECTION ) )	
	{
		lRow = AddStaticText( pclSheetDescription, ColumnDescription::Product, lRow, ( (CDB_StringID *)( pclShutoffValve->GetConnectIDPtr().MP ) )->GetString() );
	}

	if( true == m_pTADSPageSetup->GetField( epfSVINFOPRODUCTVERSION ) )	
	{
		lRow = AddStaticText( pclSheetDescription, ColumnDescription::Product, lRow, ( (CDB_StringID *)( pclShutoffValve->GetVersionIDPtr().MP ) )->GetString() );
	}

	if( true == m_pTADSPageSetup->GetField( epfSVINFOPRODUCTPN ) )	
	{
		lRow = FillPNTminTmax( pclSheetDescription, ColumnDescription::Product, lRow, pclShutoffValve );
	}
	
	// Column 'Quantity' and prices.
	FillQtyPriceCol( pclSheetDescription, lFirstRow, pclShutoffValve, iQuantity );
	
	// Column 'Article number'.
	AddStaticText( pclSheetDescription, ColumnDescription::ArticleNumber, lFirstRow, _T("-") );

	// Remark: 'true' to exclude any connection reference in the article number.
	FillArtNumberCol( pclSheetDescription, ColumnDescription::ArticleNumber, lFirstRow, pclShutoffValve, pclShutoffValve->GetArtNum( true ) );
	return lRow;
}

long CSelProdPageSv::FillRowSvInfo( CSheetDescription *pclSheetDescription, long lRow, CDB_TAProduct *pclShutoffValve, double dQ, double dDp, 
	double dRho, double dNu, CString strPowerDt )
{
	if( NULL == pclSheetDescription || NULL == pclShutoffValve )
	{
		return lRow;
	}

	CString str1;
	
	// Column Infos.
	AddStaticText( pclSheetDescription, ColumnDescription::TechnicalInfos, lRow, _T("-") );			// Default string

	if( true == m_pTADSPageSetup->GetField( epfSVTECHINFOFLOW ) )
	{
		if( dQ > 0.0 )
		{
			// HYS-38: Show power dt info when their radio button is checked
			if( strPowerDt != _T("") )
			{
				CString strQ = _T("(");
				strQ  += WriteCUDouble( _U_FLOW, dQ, true );
				strQ += _T(")");
				lRow = AddStaticText( pclSheetDescription, ColumnDescription::TechnicalInfos, lRow, strPowerDt );
				lRow = AddStaticText( pclSheetDescription, ColumnDescription::TechnicalInfos, lRow, strQ );
			}
			else
			{
				lRow = AddStaticText( pclSheetDescription, ColumnDescription::TechnicalInfos, lRow, WriteCUDouble( _U_FLOW, dQ, true ) );
			}
		}
		else
		{
			ASSERT( 0 );
		}
	}

	if( true == m_pTADSPageSetup->GetField( epfSVTECHINFODP ) )
	{
		if( dDp > 0.0 )
		{
			lRow = AddStaticText( pclSheetDescription, ColumnDescription::TechnicalInfos, lRow , WriteCUDouble( _U_DIFFPRESS, dDp, true ) );
		}
		else	//dDp == 0.0
		{
			// Compute Dp based on opening and flow.
			if( NULL != pclShutoffValve->GetValveCharacteristic() )
			{
				double dDp = pclShutoffValve->GetValveCharacteristic()->GetDpFullOpening( dQ, dRho, dNu );
				
				if( -1.0 != dDp )
				{
					lRow = AddStaticText( pclSheetDescription, ColumnDescription::TechnicalInfos, lRow , WriteCUDouble( _U_DIFFPRESS, dDp, true ) );
				}
			}
		}
	}

	return lRow;
}

long CSelProdPageSv::FillRowSv( CSheetDescription *pclSheetDescription, long lRow, CDS_HydroMod *pclHM, CDS_HydroMod::CBase *pclBase, CDB_TAProduct **ppShutoffValve )
{
	if( NULL == pclSheetDescription || NULL == pclSheetDescription->GetSSheetPointer() || NULL == pclHM || NULL == pclBase )
	{
		return lRow;
	}

	CSSheet *pclSheet = pclSheetDescription->GetSSheetPointer();
	CString str1, str2;
	long lFirstRow = lRow;

	double dQ = 0.0;
	double dDp = 0.0;
	CDB_TAProduct *pclShutoffValve = NULL;
	CWaterChar *pclWaterChar = NULL;

	// If 'pHM' exists and not 'pclShutoffValve' means that valve has been added in CBI mode.
	if( NULL == pclBase ) 
	{
		pclShutoffValve = dynamic_cast<CDB_TAProduct*>( pclHM->GetCBIValveIDPtr().MP );
		dQ = pclHM->GetQDesign();
		dDp = pclHM->GetDp();

		// In CBI mode we absolutely don't have details about the water characteristic in the circuit.
		// It's the general one.
		pclWaterChar = TASApp.GetpTADS()->GetpWCForProject()->GetpWCData();
	}
	else
	{
		CDS_HydroMod::eHMObj eShutoffValveLocated = CDS_HydroMod::eNone;

		if( true == pclHM->IsShutoffValveExist( CDS_HydroMod::eShutoffValveSupply, true ) )
		{
			eShutoffValveLocated = CDS_HydroMod::eShutoffValveSupply;
		}
		else if( true == pclHM->IsShutoffValveExist( CDS_HydroMod::eShutoffValveReturn, true ) )
		{
			eShutoffValveLocated = CDS_HydroMod::eShutoffValveReturn;
		}
		else
		{
			// Error: no shut-off valve.
			return lRow;
		}

		pclWaterChar = pclHM->GetpWaterChar( eShutoffValveLocated );

		if( pclHM->GetpShutoffValve( eShutoffValveLocated ) == pclBase )
		{
			pclShutoffValve = dynamic_cast<CDB_TAProduct*>( pclHM->GetpShutoffValve( eShutoffValveLocated )->GetIDPtr().MP );
			dQ = pclHM->GetpShutoffValve( eShutoffValveLocated )->GetQ();
			dDp = pclHM->GetpShutoffValve( eShutoffValveLocated )->GetDp();
		}
	}

	if( NULL == pclShutoffValve )
	{
		return pclSheet->GetMaxRows();
	}
	
	pclSheet->SetTextPattern( CSSheet::_SSTextPattern::UserStaticColumn );
	pclSheet->SetTextPatternProperty( CSSheet::_SSTextPatternProperty::TPP_FontBold, (LPARAM)FALSE );

	// Add item into the full article list.
	_AddArticleList( pclShutoffValve, 1 );

	// Column TA Product.
	FillRowSvProduct( pclSheetDescription, lRow, pclShutoffValve, 1 );

	// Column infos.
	ASSERT( NULL != pclWaterChar );
	FillRowSvInfo( pclSheetDescription, lFirstRow, pclShutoffValve, dQ, dDp, pclWaterChar->GetDens(), pclWaterChar->GetKinVisc(),_T("") );
	
	if( NULL != ppShutoffValve )
	{
		*ppShutoffValve = pclShutoffValve;
	}

	return pclSheet->GetMaxRows();
}

void CSelProdPageSv::AddArticleInGroupListFromDpCBCV( CDS_SSelDpCBCV *pSSelDpCBCV )
{
	if( NULL == pSSelDpCBCV )
	{
		return;
	}

	CDB_ShutoffValve *pclShutoffValve = dynamic_cast<CDB_ShutoffValve*>( pSSelDpCBCV->GetSVIDPtr().MP );

	if( NULL == pclShutoffValve )
	{
		return;
	}

	int iQuantity = pSSelDpCBCV->GetpSelectedInfos()->GetQuantity();
	CArticleGroup *pclArticleGroup = new CArticleGroup();
	ASSERT( NULL != pclArticleGroup );

	if( NULL == pclArticleGroup )
	{
		return;
	}
	
	CArticleContainer *pclArticleContainer = pclArticleGroup->AddArticle( pclShutoffValve, iQuantity );
	AddAccessoriesInArticleContainer( pSSelDpCBCV->GetSVAccessoryList(), pclArticleContainer, iQuantity, false );

	CSelProdPageBase::AddArticleInGroupList( pclArticleGroup );

	// Remark: pay attention that 'CPage::AddArticleInGroupList' will increment quantity if article already exist or
	//         will create a new 'CArticleGroup' and will copy to it data passed as argument. This is the reason
	//         why you need to delete 'pclArticleGroup' after the call.
	delete pclArticleGroup;

	// Add connection component if needed.
	if( true == pclShutoffValve->IsConnTabUsed() )
	{
		// Inlet.
		CSelProdPageBase::AddCompArtList( pclShutoffValve, iQuantity, true );
		// Outlet.
		CSelProdPageBase::AddCompArtList( pclShutoffValve, iQuantity, false );
	}
}

void CSelProdPageSv::_InitColHeader( CSheetDescription* pclSheetDescription )
{
	if( NULL == pclSheetDescription || NULL == pclSheetDescription->GetSSheetPointer() )
	{
		return;
	}

	CSSheet *pclSheet = pclSheetDescription->GetSSheetPointer();
	CString str;

	// Format columns header.
	pclSheet->SetTextPattern( CSSheet::_SSTextPattern::ColumnHeader );
	pclSheet->SetTextPatternProperty( CSSheet::_SSTextPatternProperty::TPP_FontBackColor, (LPARAM)_WHITE );
	str = TASApp.LoadLocalizedString( IDS_SELPHDR_REF1 );
	pclSheet->SetStaticText( ColumnDescription::Reference1, SelProdHeaderRow::HR_RowHeader, str );
	str = TASApp.LoadLocalizedString( IDS_SELPHDR_REF2 );
	pclSheet->SetStaticText( ColumnDescription::Reference2, SelProdHeaderRow::HR_RowHeader, str );
	str = TASApp.LoadLocalizedString( IDS_SELPHDR_WATER );
	pclSheet->SetStaticText( ColumnDescription::Water, SelProdHeaderRow::HR_RowHeader, str );
	str = TASApp.LoadLocalizedString( IDS_SELPHDR_PRODUCT );
	pclSheet->SetStaticText( ColumnDescription::Product, SelProdHeaderRow::HR_RowHeader, str );
	pclSheet->SetTextPatternProperty( CSSheet::_SSTextPatternProperty::TPP_StyleWordWrap, (LPARAM)TRUE );
	str = TASApp.LoadLocalizedString( IDS_SELPHDR_TECH );
	pclSheet->SetStaticText( ColumnDescription::TechnicalInfos, SelProdHeaderRow::HR_RowHeader, str );
	str = TASApp.LoadLocalizedString( IDS_SELPHDR_RAD );
	pclSheet->SetStaticText( ColumnDescription::RadiatorInfos, SelProdHeaderRow::HR_RowHeader, str );
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

long CSelProdPageSv::_FillRowSv( CSheetDescription* pclSheetDescription, long lRow, CDS_SSelSv *pSSelSv, CDB_TAProduct *pclShutoffValve )
{
	if( NULL == pclSheetDescription || NULL == pclSheetDescription->GetSSheetPointer() || NULL == pSSelSv || NULL == pclShutoffValve )
	{
		return lRow;
	}

	CSSheet *pclSheet = pclSheetDescription->GetSSheetPointer();
	CString str1, str2;
	long lFirstRow = lRow;

	pclSheet->SetTextPattern( CSSheet::_SSTextPattern::UserStaticColumn );
	pclSheet->SetTextPatternProperty( CSSheet::_SSTextPatternProperty::TPP_FontBold, (LPARAM)FALSE );

	// Add item into the full article list.
	_AddArticleInGroupList( pSSelSv );
		
	// Column TA Product.
	FillRowSvProduct( pclSheetDescription, lRow, pclShutoffValve, pSSelSv->GetpSelectedInfos()->GetQuantity() );

	// Column infos.
	if( false == pSSelSv->IsFromDirSel() )
	{
		double dQ = pSSelSv->GetQ();
		double dRho = pSSelSv->GetpSelectedInfos()->GetpWCData()->GetDens();
		double dNu = pSSelSv->GetpSelectedInfos()->GetpWCData()->GetKinVisc();
		// HYS-38: Show power dt info when their radio button is checked
		CString strPowerDt = _T("");
		if( CDS_SelProd::efdPower == pSSelSv->GetFlowDef() )
		{
			strPowerDt = WriteCUDouble( _U_TH_POWER, pSSelSv->GetPower(), true );
			strPowerDt += _T(" / ");
			strPowerDt += WriteCUDouble( _U_DIFFTEMP, pSSelSv->GetDT(), true );
		}
		FillRowSvInfo( pclSheetDescription, lFirstRow, pclShutoffValve, dQ, 0, dRho, dNu, strPowerDt );
	}

	return pclSheet->GetMaxRows();
}

long CSelProdPageSv::_FillActuatorSelectedWithSv( CSheetDescription* pclSheetDescription, long lRow, CDS_SSelSv *pclSSelShutOff )
{
	if( NULL == pclSheetDescription || NULL == pclSheetDescription->GetSSheetPointer() || NULL == pclSSelShutOff )
	{
		return lRow;
	}

	CSSheet *pclSheet = pclSheetDescription->GetSSheetPointer();
	
	CDB_ElectroActuator *pclElectroActuator = dynamic_cast<CDB_ElectroActuator *>( pclSSelShutOff->GetActrIDPtr().MP );

	if( NULL != pclElectroActuator )
	{
		long lFirstRow = lRow;

		CString str;
		pclSheet->SetTextPattern( CSSheet::_SSTextPattern::UserStaticColumn );
		pclSheet->SetTextPatternProperty( CSSheet::_SSTextPatternProperty::TPP_FontBold, (LPARAM)FALSE );
		
		// Set font color to red when accessory is not deleted or not available.
		if( true == pclElectroActuator->IsDeleted() || false == pclElectroActuator->IsAvailable() )
		{
			pclSheet->SetTextPatternProperty( CSSheet::_SSTextPatternProperty::TPP_FontForeColor, (LPARAM)_RED );
		}

		// If selected as a CV-Actuator set don't show actuator article number.
		if( true == pclSSelShutOff->IsSelectedAsAPackage( true ) )
		{
			AddStaticText( pclSheetDescription, ColumnDescription::ArticleNumber, lRow, _T("-") );
		}
		else
		{
			// Check first if we have a local article number available.
			CString strLocalArticleNumber = _T("");

			if( true == TASApp.IsLocalArtNumberUsed() )
			{
				strLocalArticleNumber = LocArtNumTab.GetLocalArticleNumber( pclElectroActuator->GetArtNum() );
				strLocalArticleNumber.Trim();
			}

			// Fill article number.
			bool bArticleNumberShown = false;

			// If we can show article number OR we can't show but there is no local article number...
			if( false == TASApp.IsTAArtNumberHidden() || ( true == TASApp.IsTAArtNumberHidden() && true == strLocalArticleNumber.IsEmpty() )  )
			{
				CString str = pclElectroActuator->GetArtNum();

				if( false == str.IsEmpty() )
				{
					AddStaticText( pclSheetDescription, ColumnDescription::ArticleNumber, lRow, pclElectroActuator->GetArtNum() );
					bArticleNumberShown = true;
				}
			}

			// Set local article number if allowed and exist.
			if( true == TASApp.IsLocalArtNumberUsed() && false == strLocalArticleNumber.IsEmpty() )
			{
				long lNextRow = lRow + ( ( true == bArticleNumberShown ) ? 1 : 0 );
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

		pclSheet->SetTextPatternProperty( CSSheet::_SSTextPatternProperty::TPP_FontForeColor, (LPARAM)_BLACK );
				
		// Write quantity and prices.
		bool bShowPrice = !pclSSelShutOff->IsSelectedAsAPackage( true );
		FillQtyPriceCol( pclSheetDescription, lRow, pclElectroActuator, pclSSelShutOff->GetpSelectedInfos()->GetQuantity(), bShowPrice );
		
		// Write the actuator name.
		AddStaticText( pclSheetDescription, ColumnDescription::Product, lRow, pclElectroActuator->GetName() );
		AddCellSpanW( pclSheetDescription, ColumnDescription::Product, lRow++, 2, 1 );
		pclSheet->SetTextPatternProperty( CSSheet::_SSTextPatternProperty::TPP_StyleHorizontalAlign, (LPARAM)SSS_ALIGN_LEFT );

		// Write IP.
		str = TASApp.LoadLocalizedString( IDS_SSHEETSELPROD_IPXXAUTO );
		str += CString( _T(" : ") ) + pclElectroActuator->GetIPxxAuto();

		if( false == pclElectroActuator->GetIPxxManual().IsEmpty() )
		{
			str += _T("(") + pclElectroActuator->GetIPxxManual() + _T(")");
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

		if( CString( _T("") ) == pclElectroActuator->GetInOutSignalsStr( false ) )
		{
			str += CString( _T(" :    -") );
		}
		else
		{
			str += CString( _T(" : ") ) + pclElectroActuator->GetInOutSignalsStr( false );
		}

		AddStaticText( pclSheetDescription, ColumnDescription::Product, lRow, str );
		AddCellSpanW( pclSheetDescription, ColumnDescription::Product, lRow++, 2, 1 );

		// Write relay type.
		str = TASApp.LoadLocalizedString( IDS_SSHEETSELPROD_RELAYTYPE ) + CString( _T(" : ") ) + pclElectroActuator->GetRelayStr();
		AddStaticText( pclSheetDescription, ColumnDescription::Product, lRow, str );
		AddCellSpanW( pclSheetDescription, ColumnDescription::Product, lRow++, 2, 1 );

		// Write fail safe.
		str = TASApp.LoadLocalizedString( IDS_SSHEETSELPROD_FAILSAFE );

		if( pclElectroActuator->GetFailSafe() == CDB_ElectroActuator::FailSafeType::eFSTypeElectronic )
		{
			str += CString( _T( " : " ) ) + TASApp.LoadLocalizedString( IDS_STRELECFAILSAFE );
		}
		else if( pclElectroActuator->GetFailSafe() == CDB_ElectroActuator::FailSafeType::eFSTypeSpringReturn)
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
		str = TASApp.LoadLocalizedString( IDS_SSHEETSSEL_ACTRDRP );

		if( CString( _T("") ) == pclElectroActuator->GetDefaultReturnPosStr( pclElectroActuator->GetDefaultReturnPos() ).c_str() )
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
		pclSheet->SetTextPatternProperty( CSSheet::_SSTextPatternProperty::TPP_StyleHorizontalAlign, (LPARAM)SSS_ALIGN_CENTER );

		m_rProductParam.AddRange( lFirstRow, pclSheet->GetMaxRows(), pclElectroActuator );

		lRow++;
	}

	return pclSheet->GetMaxRows();
}

void CSelProdPageSv::_AddArticleList( CDB_Actuator *pclActuator, CAccessoryList *pclAccessoryList, int iQuantity, bool bSelectionBySet )
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
	
	pclArticleItem->SetDescription( pElectroActuator->GetName() + CString( _T("; ") ) );

	if( CDB_CloseOffChar::Linear == pElectroActuator->GetOpeningType() )
	{
		pclArticleItem->AddDescription( TASApp.LoadLocalizedString( IDS_SSHEETSELPROD_ACTFORCE ) + CString( _T(" = ") ) );
	}
	else
	{
		pclArticleItem->AddDescription( TASApp.LoadLocalizedString( IDS_SSHEETSELPROD_ACTTORQUE ) + CString( _T(" = ") ) );
	}

	pclArticleItem->AddDescription( WriteCUDouble( _U_FORCE, pElectroActuator->GetMaxForceTorque(), true ) + CString( _T("; ") ) );
	pclArticleItem->AddDescription( TASApp.LoadLocalizedString( IDS_SSHEETSELPROD_POWSUPPL ) + CString( _T(" = ") ) );
	pclArticleItem->AddDescription( pElectroActuator->GetPowerSupplyStr() + CString( _T("; ") ) );
	pclArticleItem->AddDescription( TASApp.LoadLocalizedString( IDS_SSHEETSELPROD_RELAYTYPE ) + CString( _T(" = ") ) );
	pclArticleItem->AddDescription( pElectroActuator->GetRelayStr() );

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

	AddAccessoriesInArticleContainer( pclAccessoryList, pclArticleContainer, iQuantity, bSelectionBySet );

	CSelProdPageBase::AddArticleInGroupList( pclArticleGroup );

	// Remark: pay attention that 'CPage::AddArticleInGroupList' will increment quantity if article already exist or
	//         will create a new 'CArticleGroup' and will copy to it data passed as argument. This is the reason
	//         why you need to delete 'pclArticleGroup' after the call.
	delete pclArticleGroup;
}

void CSelProdPageSv::_AddArticleList( CDB_TAProduct *pclTAP, int iQuantity )
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

void CSelProdPageSv::_AddArticleInGroupList( CDS_SSelSv *pSSelSv )
{
	if( NULL == pSSelSv )
	{
		return;
	}

	CDB_TAProduct *pTAP = pSSelSv->GetProductAs<CDB_TAProduct>();

	if( NULL == pTAP )
	{
		return;
	}

	int iQuantity = pSSelSv->GetpSelectedInfos()->GetQuantity();
	CArticleGroup *pclArticleGroup = new CArticleGroup();
	ASSERT( NULL != pclArticleGroup );

	if( NULL == pclArticleGroup )
	{
		return;
	}
	
	CArticleContainer *pclArticleContainer = pclArticleGroup->AddArticle( pTAP, iQuantity );
	AddAccessoriesInArticleContainer( pSSelSv->GetAccessoryList(), pclArticleContainer, iQuantity, false );

	CSelProdPageBase::AddArticleInGroupList( pclArticleGroup );

	CDB_Actuator *pclActuator = dynamic_cast<CDB_Actuator*>( pSSelSv->GetActrIDPtr().MP );

	if( NULL != pclActuator )
	{
		_AddArticleList( pclActuator, pSSelSv->GetActuatorAccessoryList(), pSSelSv->GetpSelectedInfos()->GetQuantity(), 
				pSSelSv->IsSelectedAsAPackage( true ) );
	}

	// Remark: pay attention that 'CPage::AddArticleInGroupList' will increment quantity if article already exist or
	//         will create a new 'CArticleGroup' and will copy to it data passed as argument. This is the reason
	//         why you need to delete 'pclArticleGroup' after the call.
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
