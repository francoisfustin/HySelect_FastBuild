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
#include "SelProdPageBv.h"

CSelProdPageBv::CSelProdPageBv( CArticleGroupList *pclArticleGroupList ) 
	: CSelProdPageBase( CDB_PageSetup::enCheck::BV, pclArticleGroupList )
{
	m_pSelected = NULL;
}

void CSelProdPageBv::Reset( void )
{
	CSelProdPageBase::Reset();
}

bool CSelProdPageBv::PreInit( HMvector &vecHMList )
{
	// Something for individual or direct selection ?
	CTable *pclTable = (CTable*)( TASApp.GetpTADS()->Get( _T("REGVALV_TAB") ).MP );	

	if ( NULL == pclTable )
	{
		ASSERT( 0 );
		return false;
	}

	m_pclSelectionTable = ( _T('\0') != *pclTable->GetFirst().ID ) ? pclTable : NULL;
	bool fBvExist = ( NULL != m_pclSelectionTable );

	// Something for hydronic calculation?
	bool fBvHMExist = ( vecHMList.size() > 0 );

	if( false == fBvExist && false == fBvHMExist )
	{
		return false;
	}
	
	m_vecHMList = vecHMList;
	SortTable();
	return true;
}

bool CSelProdPageBv::Init( bool fResetOrder, bool fPrint )
{
	bool fBvExist = ( NULL != m_pclSelectionTable ) ? true : false;
	bool fBvHMExist = ( m_vecHMList.size() > 0 ) ? true : false;

	if( false == fBvExist && false == fBvHMExist )
	{
		return false;
	}

	// Call base class method to get a new 'Sheet'.
	CSheetDescription *pclSheetDescriptionBv = CMultiSpreadBase::CreateSSheet( SD_BalancingValve );

	if( NULL == pclSheetDescriptionBv || NULL == pclSheetDescriptionBv->GetSSheetPointer() )
	{
		return false;
	}
	
	CSSheet *pclSheet = pclSheetDescriptionBv->GetSSheetPointer();

	if( false == CSelProdPageBase::PrepareSheet( pclSheetDescriptionBv, ColumnDescription::Pointer, fPrint ) )
	{
		m_ViewDescription.RemoveOneSheetDescription( SD_BalancingValve );
		return false;
	}

	// Initialize column width for header and footer columns.
	if( false == m_bForPrint )
	{
		SetColWidth( pclSheetDescriptionBv, ColumnDescription::Header, DefaultColumnWidth::DCW_Header );
		SetColWidth( pclSheetDescriptionBv, ColumnDescription::Footer, DefaultColumnWidth::DCW_Footer );
	}
	else
	{
		pclSheet->ShowCol( ColumnDescription::Header, FALSE );
		pclSheet->ShowCol( ColumnDescription::Footer, FALSE );
	}

	double dWidth = ( true == m_pTADSPageSetup->GetField( PageField_enum::epfFIRSTREF ) ) ? DefaultColumnWidth::DCW_Reference1 : 0.0;
	SetColWidth( pclSheetDescriptionBv, ColumnDescription::Reference1, dWidth );

	dWidth = ( true == m_pTADSPageSetup->GetField( PageField_enum::epfSECONDREF ) ) ? DefaultColumnWidth::DCW_Reference2 : 0.0;
	SetColWidth( pclSheetDescriptionBv, ColumnDescription::Reference2, dWidth );
	
	dWidth = ( true == m_pTADSPageSetup->GetField( PageField_enum::epfWATERINFO ) ) ? DefaultColumnWidth::DCW_Water : 0.0;
	SetColWidth( pclSheetDescriptionBv, ColumnDescription::Water, dWidth );
	
	dWidth = ( true == m_pTADSPageSetup->GetField( PageField_enum::epfBVINFOPRODUCT ) ) ? DefaultColumnWidth::DCW_Product : 0.0;
	SetColWidth( pclSheetDescriptionBv, ColumnDescription::Product, dWidth );
	
	dWidth = ( true == m_pTADSPageSetup->GetField( PageField_enum::epfBVTECHINFO ) ) ? DefaultColumnWidth::DCW_TechnicalInfos : 0.0;
	SetColWidth( pclSheetDescriptionBv, ColumnDescription::TechnicalInfos, dWidth );
	
	SetColWidth( pclSheetDescriptionBv, ColumnDescription::RadiatorInfos, 0.0 );
	
	dWidth = ( true == m_pTADSPageSetup->GetField( PageField_enum::epfARTICLE ) ) ? DefaultColumnWidth::DCW_ArticleNumber : 0.0;
	SetColWidth( pclSheetDescriptionBv, ColumnDescription::ArticleNumber, dWidth );
	
	dWidth = ( true == m_pTADSPageSetup->GetField( PageField_enum::epfPIPEINFO ) ) ? DefaultColumnWidth::DCW_Pipes : 0.0;
	SetColWidth( pclSheetDescriptionBv, ColumnDescription::Pipes, dWidth );
	
	dWidth = ( true == m_pTADSPageSetup->GetField( PageField_enum::epfQUANTITY ) ) ? DefaultColumnWidth::DCW_Quantity : 0.0;
	SetColWidth( pclSheetDescriptionBv, ColumnDescription::Quantity, dWidth );
	
	dWidth = 0.0;
	
	if( true == TASApp.IsPriceUsed() )
	{
		dWidth = ( true == m_pTADSPageSetup->GetField( PageField_enum::epfSALEUNITPRICE ) ) ? DefaultColumnWidth::DCW_UnitPrice : 0.0;
	}

	SetColWidth( pclSheetDescriptionBv, ColumnDescription::UnitPrice, dWidth );

	dWidth = 0.0;
	
	if( true == TASApp.IsPriceUsed() )
	{
		dWidth = ( true == m_pTADSPageSetup->GetField( PageField_enum::epfSALETOTALPRICE ) ) ? DefaultColumnWidth::DCW_TotalPrice : 0.0;
	}

	SetColWidth( pclSheetDescriptionBv, ColumnDescription::TotalPrice, dWidth );

	dWidth = ( true == m_pTADSPageSetup->GetField( PageField_enum::epfREMARK ) ) ? DefaultColumnWidth::DCW_Remark : 0.0;
	SetColWidth( pclSheetDescriptionBv, ColumnDescription::Remark, dWidth );
	
	
	SetColWidth( pclSheetDescriptionBv, ColumnDescription::Pointer, DefaultColumnWidth::DCW_Pointer );
	
	pclSheet->ShowCol( ColumnDescription::RadiatorInfos, FALSE );

	if( false == TASApp.IsPriceUsed() )
	{
		pclSheet->ShowCol( ColumnDescription::UnitPrice, FALSE );
		pclSheet->ShowCol( ColumnDescription::TotalPrice, FALSE );
	}

	pclSheet->ShowCol( ColumnDescription::Pointer, FALSE );
	CRect rect = pclSheet->GetSheetSizeInPixels( false );
	m_mapSheetInfos[pclSheetDescriptionBv].m_dPageWidth = rect.Width();

	// Page title.
	SetPageTitle( pclSheetDescriptionBv, IDS_RVIEWSELP_TITLE_BV );
	
	// Init column header.
	_InitColHeader( pclSheetDescriptionBv );
	pclSheet->SetColumnAlwaysHidden( ColumnDescription::RadiatorInfos, true );

	// HYS-1571: We can optimize the column width now.
	// See Jira card to have more details about why we do that here.
	CRect rectClient;
	GetClientRect( &rectClient );
	OptimizePageSize( rectClient.Width(), rectClient.Height() );

	// Current position.
	long lRow = pclSheet->GetMaxRows() + 1;
	long lSelectedRow = 0;

	if( true == fBvExist )
	{
		// Filling Balancing valves from SSel or from Advance Hydraulic mode.
		// Create an array of SelManBv pointer.

		// Number of objects.
		int iBvCount = m_pclSelectionTable->GetItemCount( CLASS( CDS_SSelBv ) );

		if( 0 == iBvCount )
		{
			return false;
		}
		
		CDS_SSelBv **paSSelBv = new CDS_SSelBv*[iBvCount];

		if( NULL == paSSelBv )
		{
			return false;
		}
		
		// Fill 'paSSelBv' array with pointer on each 'CDS_SSelBv' object of the 'REGVALV_TAB' table.
		int iMaxIndex = 0;
		int i = 0;
		
		for( IDPTR IDPtr = m_pclSelectionTable->GetFirst( CLASS( CDS_SSelBv ) ); '\0' != *IDPtr.ID; IDPtr = m_pclSelectionTable->GetNext() )
		{
			CDS_SSelBv *pclSSelBv = dynamic_cast<CDS_SSelBv *>( IDPtr.MP );

			if( NULL == pclSSelBv )
			{
				continue;
			}
			
			if( NULL == pclSSelBv->GetIDPtr().MP )
			{
				continue;
			}
			
			paSSelBv[i] = pclSSelBv;

			if( iMaxIndex < paSSelBv[i]->GetpSelectedInfos()->GetRowIndex() )
			{
				iMaxIndex = paSSelBv[i]->GetpSelectedInfos()->GetRowIndex();
			}

			++i;
		}
		
		ASSERT( i == iBvCount );
		iBvCount = i;
		
		// Remark: 'fResetOrder' is set to 'true' only when the sorting combos are empty (No user choice) and the user clicks on the 'Apply sorting keys'.
		if( true == fResetOrder )
		{
			// Reset row index.
			for( i = 0; i < iBvCount; i++ )
			{
				paSSelBv[i]->GetpSelectedInfos()->SetRowIndex( i + 1 );
			}
		}
		else if( PageField_enum::epfNONE != m_pTADSPageSetup->GetKey( 0 ) || PageField_enum::epfNONE != m_pTADSPageSetup->GetKey( 1 ) 
	 			|| PageField_enum::epfNONE != m_pTADSPageSetup->GetKey( 2 ) )
		{
			// This method will sort (or not) in regards to the three sorting keys that user has chosen.
			if( NULL != pDlgLeftTabSelP )
			{
				pDlgLeftTabSelP->SortTable( (CData**)paSSelBv, iBvCount - 1 );
			}

			// Reset row index.
			for( i = 0; i < iBvCount; i++ )
			{
				paSSelBv[i]->GetpSelectedInfos()->SetRowIndex( i + 1 );
			}
		}

		// Create a remark index.
		if( NULL != pDlgLeftTabSelP )
		{
			pDlgLeftTabSelP->SetRemarkIndex( CLASS( CDS_SSelBv ), (CData**)paSSelBv, iBvCount );
		}

		CRank rkl;
		enum eBvSwitch
		{
			SingleSelBv,
			DirSelBv,
			LastBvSwitch
		};
		bool bAtLeastOneBlockPrinted = false;

		for( int iLoopGroup = 0; iLoopGroup < LastBvSwitch; iLoopGroup++ )
		{
			int iCount = 0;

			for( i = 0; i < iBvCount; i++ )
			{
				CDS_SSelBv *pclSelBv = dynamic_cast<CDS_SSelBv *>( paSSelBv[i] );
				
				if( NULL == pclSelBv || NULL == pclSelBv->GetProductIDPtr().MP )
				{
					ASSERT( 0 );
					continue;
				}

				if( DirSelBv == iLoopGroup )
				{
					if( false == pclSelBv->IsFromDirSel() )
					{
						continue;
					}
				}
				else
				{
					if( true == pclSelBv->IsFromDirSel() )
					{
						continue;
					}
				}

				// All Bv products are based on a 'CDB_TAProduct' object.
				CDB_TAProduct *pclTAProduct = dynamic_cast<CDB_TAProduct *>( pclSelBv->GetProductIDPtr().MP );

				if( NULL == pclTAProduct )
				{
					ASSERT( 0 );
					continue;
				}

				rkl.Add( _T(""), paSSelBv[i]->GetpSelectedInfos()->GetRowIndex(), (LPARAM)paSSelBv[i] );
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
			SetLastRow( pclSheetDescriptionBv, lRow );
			
			switch( iLoopGroup )
			{
				case DirSelBv:
					SetPageTitle( pclSheetDescriptionBv, IDS_SSHEETSELPROD_SUBTITLEBVFROMDIRSEL, false, lRow );
					break;

				case SingleSelBv:
					SetPageTitle( pclSheetDescriptionBv, IDS_SSHEETSELPROD_SUBTITLEBVFROMSSEL, false, lRow );
					break;
			}

			lRow = pclSheet->GetMaxRows();
			SetLastRow( pclSheetDescriptionBv, lRow );

			CString str;
			LPARAM lparam;
			bool bFirstPass = true;

			for( BOOL bContinue = rkl.GetFirst( str, lparam ); TRUE == bContinue; bContinue = rkl.GetNext( str, lparam ) )
			{
				CData *pclData = (CData*)lparam;
				CDS_SSelBv *pclSSelBv = dynamic_cast<CDS_SSelBv *>( pclData );

				if( NULL == pclSSelBv )
				{
					continue;
				}

				if( m_pSelected == pclData )
				{
					lSelectedRow = lRow;
				}

				CDB_TAProduct *pclBv = pclSSelBv->GetProductAs<CDB_TAProduct>();

				if( NULL == pclBv )
				{
					continue;
				}

				m_rProductParam.Clear();
				m_rProductParam.SetSheetDescription( pclSheetDescriptionBv );
				m_rProductParam.SetSelectionContainer( (LPARAM)pclSSelBv );

				++lRow;
				long lFirstRow = lRow;

				long lRowBV = _FillRowBv( pclSheetDescriptionBv, lRow, pclSSelBv, pclBv );
				long lRowGen = CSelProdPageBase::FillRowGen( pclSheetDescriptionBv, lRow, pclSSelBv );
				lRow = max( lRowGen, lRowBV );

				// Add the selectable row range.
				m_rProductParam.AddRange( lFirstRow, pclSheet->GetMaxRows(), pclBv );

				// Spanning must be done here because it's the only place where we know exactly number of lines to span!
				// Span reference #1 and #2.
				AddCellSpanW( pclSheetDescriptionBv, ColumnDescription::Reference1, lFirstRow, 1, pclSheet->GetMaxRows() - lFirstRow + 1 );
				AddCellSpanW( pclSheetDescriptionBv, ColumnDescription::Reference2, lFirstRow, 1, pclSheet->GetMaxRows() - lFirstRow + 1 );

				// Span water characteristic.
				AddCellSpanW( pclSheetDescriptionBv, ColumnDescription::Water, lFirstRow, 1, pclSheet->GetMaxRows() - lFirstRow + 1 );

				// Add accessories.
				if( true == pclSSelBv->IsAccessoryExist() )
				{
					// Draw dash line.
					pclSheet->SetCellBorder( ColumnDescription::Reference1, lRow, ColumnDescription::Footer - 1, lRow, true, SS_BORDERTYPE_BOTTOM, SS_BORDERSTYLE_FINE_DASH );

					lRow++;
					lRow = FillAccessories( pclSheetDescriptionBv, lRow, pclSSelBv, pclSSelBv->GetpSelectedInfos()->GetQuantity() );
				}

				// Save product.
				m_rProductParam.SetScrollRange( ( true == bFirstPass ) ? lFirstRow - 1 : lFirstRow, lRow );
				SaveProduct( m_rProductParam );

				// Draw line below.
				pclSheet->SetCellBorder( ColumnDescription::Reference1, lRow, ColumnDescription::Footer - 1, lRow, true, SS_BORDERTYPE_BOTTOM );

				// Set all group as no breakable (for print).
				// Remark: include title with the group.
				pclSheet->SetFlagRowNoBreakable( ( true == bFirstPass ) ? lFirstRow - 1 : lFirstRow, lRow, true );

				bFirstPass = false;
			}

			lRow++;
			bAtLeastOneBlockPrinted = true;
		}

		FillRemarks( pclSheetDescriptionBv, lRow );

		// We add by default a blank line.
		lRow = pclSheet->GetMaxRows() + 2;
		
		delete[] paSSelBv;
	}

	if( true == fBvHMExist )
	{
		if( m_vecHMList.size() > 0 )	
		{
			// Create a remark index.
			if( NULL != pDlgLeftTabSelP )
			{
				pDlgLeftTabSelP->SetRemarkIndex( &m_vecHMList );
			}

			SetLastRow( pclSheetDescriptionBv, lRow );
			SetPageTitle( pclSheetDescriptionBv, IDS_SSHEETSELPROD_SUBTITLEBVFROMHM, false, lRow );	

			lRow++;
			SetLastRow( pclSheetDescriptionBv, lRow );
			
			// For each element.
			bool bFirstPass = true;
			HMvector::iterator It;

			for( It = m_vecHMList.begin(); It != m_vecHMList.end(); It++ )
			{
				CDS_HydroMod *pHM = NULL;
				CDS_HydroMod::CBase *pBase = NULL;

				if( eptHM == (*It).first )
				{
					pHM = dynamic_cast<CDS_HydroMod *>( (CData*)( (*It).second ) );
				}
				else
				{
					// Bv exist and TA product exist into the DB.
					pBase = static_cast<CDS_HydroMod::CBase *>( (*It).second );

					if( NULL != pBase )
					{
						pHM = pBase->GetpParentHM();
					}
				}

				if( NULL != pHM )
				{
					m_rProductParam.Clear();
					m_rProductParam.SetSheetDescription( pclSheetDescriptionBv );
					// Intentionally set to NULL to avoid clicking, double clicking or right clicking.
					m_rProductParam.SetSelectionContainer( (LPARAM)0 );

					long lRowBV = FillRowBv( pclSheetDescriptionBv, lRow, pHM, pBase );
					long lRowGen = CSelProdPageBase::FillRowGen( pclSheetDescriptionBv, lRow, pHM, pBase );

					long lLastRow = max( lRowGen, lRowBV );
					
					// HYS-721: Merge cells
					long lFirstRow = lRow;
					// Spanning must be done here because it's the only place where we know exactly number of lines to span!
					// Span reference #1 and #2.
					AddCellSpanW(pclSheetDescriptionBv, ColumnDescription::Reference1, lFirstRow, 1, pclSheet->GetMaxRows() - lFirstRow + 1);
					AddCellSpanW(pclSheetDescriptionBv, ColumnDescription::Reference2, lFirstRow, 1, pclSheet->GetMaxRows() - lFirstRow + 1);
					// Span water characteristic.
					AddCellSpanW(pclSheetDescriptionBv, ColumnDescription::Water, lFirstRow, 1, pclSheet->GetMaxRows() - lFirstRow + 1);

					CDB_TAProduct *pclBv = NULL;

					// If 'pHM' exists and not 'pBV' means that valve has been added in CBI mode.
					if (NULL == pBase)
					{
						pclBv = dynamic_cast<CDB_TAProduct *>(pHM->GetCBIValveIDPtr().MP);
					}
					else
					{
						if (pHM->GetpBv() == pBase)
						{
							pclBv = dynamic_cast<CDB_TAProduct *>(pHM->GetpBv()->GetIDPtr().MP);
						}
						else if (pHM->GetpBypBv() == pBase)
						{
							pclBv = dynamic_cast<CDB_TAProduct *>(pHM->GetpBypBv()->GetIDPtr().MP);
						}
						else if (pHM->GetpSecBv() == pBase)
						{
							pclBv = dynamic_cast<CDB_TAProduct *>(pHM->GetpSecBv()->GetIDPtr().MP);
						}
						else if (pHM->GetpCV() == pBase)
						{
							pclBv = dynamic_cast<CDB_TAProduct *>(pHM->GetpCV()->GetCvIDPtr().MP);
						}
					}

					if( NULL != pclBv )
					{
						CDB_TAProduct *pclTAProd = dynamic_cast<CDB_TAProduct *>( pclBv->GetIDPtr().MP );
						lLastRow = FillAndAddBuiltInHMAccessories( pclSheet, pclSheetDescriptionBv, pclTAProd, lLastRow );
					}

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

			FillHMRemarks( &m_vecHMList, pclSheetDescriptionBv, lRow );
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

bool CSelProdPageBv::HasSomethingToDisplay( void )
{
	bool fBvExist = ( NULL != m_pclSelectionTable ) ? true : false;
	bool fBvHMExist = ( m_vecHMList.size() > 0 ) ? true : false;
	if( false == fBvExist && false == fBvHMExist )
		return false;
	return true;
}

long CSelProdPageBv::FillRowBvProduct( CSheetDescription *pclSheetDescription, long lRow, CDB_TAProduct *pclBv, int iQuantity, bool fForDpCSet )
{
	if( NULL == pclSheetDescription || NULL == pclBv )
		return lRow;

	CString str1, str2;
	long lFirstRow = lRow;
	
	// Column TA Product.
	AddStaticText( pclSheetDescription, ColumnDescription::Product, lRow, _T("-") );

	if( true == m_pTADSPageSetup->GetField( epfBVINFOPRODUCTNAME ) )	
	{
		lRow = AddStaticText( pclSheetDescription, ColumnDescription::Product, lRow, pclBv->GetName() );
	}

	if( true == m_pTADSPageSetup->GetField( epfBVINFOPRODUCTSIZE ) )	
	{
		lRow = AddStaticText( pclSheetDescription, ColumnDescription::Product, lRow, pclBv->GetSize() );
	}

	if( true == m_pTADSPageSetup->GetField( epfBVINFOPRODUCTCONNECTION ) )	
	{
		lRow = AddStaticText( pclSheetDescription, ColumnDescription::Product, lRow, ( (CDB_StringID *)( pclBv->GetConnectIDPtr().MP ) )->GetString() );
	}

	if( true == m_pTADSPageSetup->GetField( epfBVINFOPRODUCTVERSION ) )	
	{
		lRow = AddStaticText( pclSheetDescription, ColumnDescription::Product, lRow, ( (CDB_StringID *)( pclBv->GetVersionIDPtr().MP ) )->GetString() );
	}

	if( true == m_pTADSPageSetup->GetField( epfBVINFOPRODUCTPN ) )	
	{
		lRow = FillPNTminTmax( pclSheetDescription, ColumnDescription::Product, lRow, pclBv );
	}
	
	// Column 'Quantity' and prices.
	FillQtyPriceCol( pclSheetDescription, lFirstRow, pclBv, iQuantity, !fForDpCSet );
	
	// Column 'Article number'.
	AddStaticText( pclSheetDescription, ColumnDescription::ArticleNumber, lFirstRow, _T("-") );

	// Remark: 'true' to exclude any connection reference in the article number.
	if( false == fForDpCSet )
	{
		FillArtNumberCol( pclSheetDescription, ColumnDescription::ArticleNumber, lFirstRow, pclBv, pclBv->GetArtNum( true ) );
	}
	else
	{
		if( true == m_pTADSPageSetup->GetField( PageField_enum::epfARTICLE ) )
		{
			lRow = AddStaticText( pclSheetDescription, ColumnDescription::ArticleNumber, lRow, _T("-") );
		}
	}

	return lRow;
}

long CSelProdPageBv::FillRowBvInfo( CSheetDescription* pclSheetDescription, long lRow, CDB_TAProduct* pclBv, double dQ, double dDp, 
		double dPresset, double dRho, double dNu, CString strPowerDt )
{
	if( NULL == pclSheetDescription || NULL == pclBv )
	{
		return lRow;
	}

	CString str1;
	
	// Column Infos.
	AddStaticText( pclSheetDescription, ColumnDescription::TechnicalInfos, lRow, _T("-") );			// Default string
	
	if( true == m_pTADSPageSetup->GetField( epfBVTECHINFOFLOW ) )
	{
		if( dQ > 0.0 )
		{
			// HYS-38: Show power dt info when their radio button is checked
			if( strPowerDt != _T( "" ) )
			{
				CString strQ = _T("(");
				strQ += WriteCUDouble( _U_FLOW, dQ, true );
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

	if( true == m_pTADSPageSetup->GetField( epfBVTECHINFODP ) )
	{
		if( true == pclBv->IsKvSignalEquipped() )
		{
			CDB_FixOCharacteristic *pChar = dynamic_cast<CDB_FixOCharacteristic *>( pclBv->GetValveCharDataPointer() );

			if( NULL != pChar && -1.0 != pChar->GetKvSignal() )
			{
				double dDpSignal = CalcDp( dQ, pChar->GetKvSignal(), dRho );		
				CString str;
				str = TASApp.LoadLocalizedString( IDS_SSHEETSELP_DPSIGNAL );
				str += _T(" ") + CString( WriteCUDouble( _U_DIFFPRESS, dDpSignal, true ) );
				lRow = AddStaticText( pclSheetDescription, ColumnDescription::TechnicalInfos, lRow, str );
				dDp = CalcDp( dQ, pChar->GetKv(), dRho );		
			}
		}
		
		if( dDp > 0.0 )
		{
			lRow = AddStaticText( pclSheetDescription, ColumnDescription::TechnicalInfos, lRow , WriteCUDouble( _U_DIFFPRESS, dDp, true ) );
		}
		else	//dDp == 0.0
		{
			// Compute Dp based on opening and flow.
			if( NULL != pclBv->GetValveCharacteristic() )
			{
				if( true == pclBv->GetValveCharacteristic()->GetValveDp( dQ, &dDp, dPresset, dRho, dNu ) )
				{
					lRow = AddStaticText( pclSheetDescription, ColumnDescription::TechnicalInfos, lRow , WriteCUDouble( _U_DIFFPRESS, dDp, true ) );
				}
			}
		}
	}

	if( true == m_pTADSPageSetup->GetField( epfBVTECHINFOSETTING ) )
	{	
		// Not for a fixed orifice valve.
		if( 0 != _tcscmp( pclBv->GetTypeID(), _T("RVTYPE_FO") ) )
		{
			str1 = _T("-");

			if( dPresset > 0.0 && NULL != pclBv->GetValveCharacteristic() )
			{
				str1 = pclBv->GetValveCharacteristic()->GetSettingString( dPresset, true );
			}

			lRow = AddStaticText( pclSheetDescription, ColumnDescription::TechnicalInfos, lRow, str1 );
		}
	}
	
	return lRow;
}

long CSelProdPageBv::FillRowBv( CSheetDescription *pclSheetDescription, long lRow, CDS_HydroMod *pclHM, CDS_HydroMod::CBase *pclBase, CDB_TAProduct **ppMv )
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
	double dPresset = 0.0;
	CDB_TAProduct *pclBv = NULL;
	CWaterChar *pclWaterChar = NULL;
	
	// If 'pHM' exists and not 'pBV' means that valve has been added in CBI mode.
	if( NULL == pclBase ) 
	{
		pclBv = dynamic_cast<CDB_TAProduct *>( pclHM->GetCBIValveIDPtr().MP );
		dQ = pclHM->GetQDesign();
		dDp = pclHM->GetDp();
		dPresset = pclHM->GetPresetting();

		// In CBI mode we absolutely don't have details about the water characteristic in the circuit.
		// It's the general one.
		pclWaterChar = TASApp.GetpTADS()->GetpWCForProject()->GetpWCData();
	}
	else
	{
		if( pclHM->GetpBv() == pclBase )
		{
			pclBv = dynamic_cast<CDB_TAProduct *>( pclHM->GetpBv()->GetIDPtr().MP );
			dQ = pclHM->GetpBv()->GetQ();
			dDp = pclHM->GetpBv()->GetDp();
			dPresset = pclHM->GetpBv()->GetSetting();
			pclWaterChar = pclHM->GetpWaterChar( CDS_HydroMod::eHMObj::eBVprim );
		}
		else if( pclHM->GetpBypBv() == pclBase )
		{
			pclBv = dynamic_cast<CDB_TAProduct *>( pclHM->GetpBypBv()->GetIDPtr().MP );
			dQ = pclHM->GetpBypBv()->GetQ();
			dDp = pclHM->GetpBypBv()->GetDp();
			dPresset = pclHM->GetpBypBv()->GetSetting();
			pclWaterChar = pclHM->GetpWaterChar( CDS_HydroMod::eHMObj::eBVbyp );
		}
		else if( pclHM->GetpSecBv() == pclBase )
		{
			pclBv = dynamic_cast<CDB_TAProduct *>( pclHM->GetpSecBv()->GetIDPtr().MP );
			dQ = pclHM->GetpSecBv()->GetQ();
			dDp = pclHM->GetpSecBv()->GetDp();
			dPresset = pclHM->GetpSecBv()->GetSetting();
			pclWaterChar = pclHM->GetpWaterChar( CDS_HydroMod::eHMObj::eBVsec );
		}
		else if( pclHM->GetpCV() == pclBase )
		{
			pclBv = dynamic_cast<CDB_TAProduct *>( pclHM->GetpCV()->GetCvIDPtr().MP );
			dQ = pclHM->GetpCV()->GetQ();
			dDp = pclHM->GetpCV()->GetDp();
			dPresset = pclHM->GetpCV()->GetSetting();
			pclWaterChar = pclHM->GetpWaterChar( CDS_HydroMod::eHMObj::eCV );
		}
	}

	if( NULL == pclBv )
	{
		return pclSheet->GetMaxRows();
	}
	
	pclSheet->SetTextPattern( CSSheet::_SSTextPattern::UserStaticColumn );
	pclSheet->SetTextPatternProperty( CSSheet::_SSTextPatternProperty::TPP_FontBold, (LPARAM)FALSE );

	// Add item into the full article list.
	_AddArticleList( pclBv, 1 );

	// Column TA Product.
	FillRowBvProduct( pclSheetDescription, lRow, pclBv, 1 );

	// Column infos.
	ASSERT( NULL != pclWaterChar );
	FillRowBvInfo( pclSheetDescription, lFirstRow, pclBv, dQ, dDp, dPresset, pclWaterChar->GetDens(), pclWaterChar->GetKinVisc(), _T("") );
	
	if( NULL != ppMv )
	{
		*ppMv = pclBv;
	}

	return pclSheet->GetMaxRows();
}

void CSelProdPageBv::AddArticleInGroupList( CDS_SSel* pSSelBv )
{
	if( NULL == pSSelBv )
	{
		return;
	}

	CDB_TAProduct* pTAP = pSSelBv->GetProductAs<CDB_TAProduct>();

	if( NULL == pTAP )
	{
		return;
	}

	int iQuantity = pSSelBv->GetpSelectedInfos()->GetQuantity();
	CArticleGroup *pclArticleGroup = new CArticleGroup();
	ASSERT( NULL != pclArticleGroup );

	if( NULL == pclArticleGroup )
	{
		return;
	}
	
	CArticleContainer* pclArticleContainer = pclArticleGroup->AddArticle( pTAP, iQuantity );
	AddAccessoriesInArticleContainer( pSSelBv->GetAccessoryList(), pclArticleContainer, iQuantity, false );

	CSelProdPageBase::AddArticleInGroupList( pclArticleGroup );

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

void CSelProdPageBv::_InitColHeader( CSheetDescription* pclSheetDescription )
{
	if( NULL == pclSheetDescription || NULL == pclSheetDescription->GetSSheetPointer() )
		return;

	CSSheet* pclSheet = pclSheetDescription->GetSSheetPointer();
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

long CSelProdPageBv::_FillRowBv( CSheetDescription* pclSheetDescription, long lRow, CDS_SSelBv* pSSelBv, CDB_TAProduct* pclBv )
{
	if( NULL == pclSheetDescription || NULL == pclSheetDescription->GetSSheetPointer() || NULL == pSSelBv || NULL == pclBv )
		return lRow;

	CSSheet* pclSheet = pclSheetDescription->GetSSheetPointer();
	CString str1, str2;
	long lFirstRow = lRow;

	pclSheet->SetTextPattern( CSSheet::_SSTextPattern::UserStaticColumn );
	pclSheet->SetTextPatternProperty( CSSheet::_SSTextPatternProperty::TPP_FontBold, (LPARAM)FALSE );

	// Add item into the full article list.
	AddArticleInGroupList( pSSelBv );
		
	// Column TA Product.
	FillRowBvProduct( pclSheetDescription, lRow, pclBv, pSSelBv->GetpSelectedInfos()->GetQuantity() );

	// Column infos.
	if( false == pSSelBv->IsFromDirSel() )
	{
		double dQ = pSSelBv->GetQ();
		double dRho = pSSelBv->GetpSelectedInfos()->GetpWCData()->GetDens();
		double dNu = pSSelBv->GetpSelectedInfos()->GetpWCData()->GetKinVisc();
		double dPresset = pSSelBv->GetOpening();
		
		// HYS-38: Show power dt info when their radio button is checked.
		CString strPowerDt = _T( "" );
		if( CDS_SelProd::efdPower == pSSelBv->GetFlowDef() )
		{
			strPowerDt = WriteCUDouble( _U_TH_POWER, pSSelBv->GetPower(), true );
			strPowerDt += _T(" / ");
			strPowerDt += WriteCUDouble( _U_DIFFTEMP, pSSelBv->GetDT(), true );
		}
		
		FillRowBvInfo( pclSheetDescription, lFirstRow, pclBv, dQ, 0, dPresset, dRho, dNu, strPowerDt );
	}

	return pclSheet->GetMaxRows();
}

void CSelProdPageBv::_AddArticleList( CDB_TAProduct* pclTAP, int iQuantity )
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
