#include "stdafx.h"


#include "TASelect.h"
#include "MainFrm.h"
#include "Global.h"
#include "HydroMod.h"
#include "TerminalUnit.h"
#include "Hydronic.h"
#include "DlgLeftTabBase.h"
#include "DlgLeftTabSelP.h"
#include "RViewDescription.h"
#include "SelProdArticle.h"
#include "SelProdPageBase.h"
#include "SelProdPageSmartControlValve.h"

CSelProdPageSmartControlValve::CSelProdPageSmartControlValve( CArticleGroupList *pclArticleGroupList ) 
	: CSelProdPageBase( CDB_PageSetup::enCheck::SMARTCONTROLVALVE, pclArticleGroupList )
{
	m_pSelected = NULL;
}

void CSelProdPageSmartControlValve::Reset( void )
{
	CSelProdPageBase::Reset();
}

bool CSelProdPageSmartControlValve::PreInit( HMvector &vecHMList )
{
	// Something for individual or direct selection ?
	CTable *pclTable = (CTable *)( TASApp.GetpTADS()->Get( _T("SMARTCONTROLVALVE_TAB") ).MP );

	if ( NULL == pclTable )
	{
		ASSERT( 0 );
		return false;
	}

	m_pclSelectionTable = ( '\0' != *pclTable->GetFirst().ID ) ? pclTable : NULL;
	bool bSmartControlValveExist = ( NULL != m_pclSelectionTable );

	// Something for hydronic calculation?
	bool bSmartControlValveHMExist = ( vecHMList.size() > 0 );

	if( false == bSmartControlValveExist && false == bSmartControlValveHMExist )
	{
		return false;
	}

	m_vecHMList = vecHMList;
	SortTable();
	return true;
}

bool CSelProdPageSmartControlValve::Init( bool bResetOrder, bool bPrint )
{
	bool bSmartControlValveExist = ( NULL != m_pclSelectionTable ) ? true : false;
	bool bSmartControlValveHMExist = ( m_vecHMList.size() > 0 ) ? true : false;

	if( false == bSmartControlValveExist && false == bSmartControlValveHMExist )
	{
		return false;
	}

	// Call base class method to get a new 'Sheet'.
	CSheetDescription *pclSDSmartControlValve = CMultiSpreadBase::CreateSSheet( SD_SmartControlValve );

	if( NULL == pclSDSmartControlValve || NULL == pclSDSmartControlValve->GetSSheetPointer() )
	{
		return false;
	}
	
	CSSheet *pclSheet = pclSDSmartControlValve->GetSSheetPointer();

	if( false == CSelProdPageBase::PrepareSheet( pclSDSmartControlValve, ColumnDescription::Pointer, bPrint ) )
	{
		m_ViewDescription.RemoveOneSheetDescription( SD_SmartControlValve );
		return false;
	}

	// Initialize column width for header and footer column.
	if( false == m_bForPrint )
	{
		SetColWidth( pclSDSmartControlValve, ColumnDescription::Header, DefaultColumnWidth::DCW_Header );
		SetColWidth( pclSDSmartControlValve, ColumnDescription::Footer, DefaultColumnWidth::DCW_Footer );
	}
	else
	{
		pclSheet->ShowCol( ColumnDescription::Header, FALSE );
		pclSheet->ShowCol( ColumnDescription::Footer, FALSE );
	}

	double dWidth = ( true == m_pTADSPageSetup->GetField( PageField_enum::epfFIRSTREF ) ) ? DefaultColumnWidth::DCW_Reference1 : 0.0;
	SetColWidth( pclSDSmartControlValve, ColumnDescription::Reference1, dWidth );
	
	dWidth = ( true == m_pTADSPageSetup->GetField( PageField_enum::epfSECONDREF ) ) ? DefaultColumnWidth::DCW_Reference2 : 0.0;
	SetColWidth( pclSDSmartControlValve, ColumnDescription::Reference2, dWidth );
	
	dWidth = ( true == m_pTADSPageSetup->GetField( PageField_enum::epfWATERINFO ) ) ? DefaultColumnWidth::DCW_Water : 0.0;
	SetColWidth( pclSDSmartControlValve, ColumnDescription::Water, dWidth );
	
	dWidth = ( true == m_pTADSPageSetup->GetField( PageField_enum::epfSMARTCONTROLVALVEINFOPRODUCT ) ) ? DefaultColumnWidth::DCW_Product : 0.0;
	SetColWidth( pclSDSmartControlValve, ColumnDescription::Product, dWidth );
	
	dWidth = ( true == m_pTADSPageSetup->GetField( PageField_enum::epfSMARTCONTROLVALVETECHINFO ) ) ? DefaultColumnWidth::DCW_TechnicalInfos : 0.0;
	SetColWidth( pclSDSmartControlValve, ColumnDescription::TechnicalInfos, dWidth );
	
	SetColWidth( pclSDSmartControlValve, ColumnDescription::RadiatorInfos, 0.0 );
	
	dWidth = ( true == m_pTADSPageSetup->GetField( PageField_enum::epfARTICLE ) ) ? DefaultColumnWidth::DCW_ArticleNumber : 0.0;
	SetColWidth( pclSDSmartControlValve, ColumnDescription::ArticleNumber, dWidth );
	
	dWidth = ( true == m_pTADSPageSetup->GetField( PageField_enum::epfPIPEINFO ) ) ? DefaultColumnWidth::DCW_Pipes : 0.0;
	SetColWidth( pclSDSmartControlValve, ColumnDescription::Pipes, dWidth );
	
	dWidth = ( true == m_pTADSPageSetup->GetField( PageField_enum::epfQUANTITY ) ) ? DefaultColumnWidth::DCW_Quantity : 0.0;
	SetColWidth( pclSDSmartControlValve, ColumnDescription::Quantity, dWidth );
	
	dWidth = 0.0;
	
	if( true == TASApp.IsPriceUsed() )
	{
		dWidth = ( true == m_pTADSPageSetup->GetField( PageField_enum::epfSALEUNITPRICE ) ) ? DefaultColumnWidth::DCW_UnitPrice : 0.0;
	}
	
	SetColWidth( pclSDSmartControlValve, ColumnDescription::UnitPrice, dWidth );
	
	dWidth = 0.0;

	if( true == TASApp.IsPriceUsed() )
	{
		dWidth = ( true == m_pTADSPageSetup->GetField( PageField_enum::epfSALETOTALPRICE ) ) ? DefaultColumnWidth::DCW_TotalPrice : 0.0;
	}
	
	SetColWidth( pclSDSmartControlValve, ColumnDescription::TotalPrice, dWidth );
	
	dWidth = ( true == m_pTADSPageSetup->GetField( PageField_enum::epfREMARK ) ) ? DefaultColumnWidth::DCW_Remark : 0.0;
	SetColWidth( pclSDSmartControlValve, ColumnDescription::Remark, dWidth );
	
	SetColWidth( pclSDSmartControlValve, ColumnDescription::Pointer, DefaultColumnWidth::DCW_Pointer );
	
	pclSheet->ShowCol( ColumnDescription::RadiatorInfos, FALSE );
	
	if( false == TASApp.IsPriceUsed() )
	{
		pclSheet->ShowCol( ColumnDescription::UnitPrice, FALSE );
		pclSheet->ShowCol( ColumnDescription::TotalPrice, FALSE );
	}

	pclSheet->ShowCol( ColumnDescription::Pointer, FALSE );
	CRect rect = pclSheet->GetSheetSizeInPixels( false );
	m_mapSheetInfos[pclSDSmartControlValve].m_dPageWidth = rect.Width();

	// Page title.
	SetPageTitle( pclSDSmartControlValve, IDS_RVIEWSELP_TITLE_SMARTCONTROLVALVE );
	
	// Init column header.
	_InitColHeader( pclSDSmartControlValve );
	pclSheet->SetColumnAlwaysHidden( ColumnDescription::RadiatorInfos, true );
	
	// HYS-1571: We can optimize the column width now.
	// See Jira card to have more details about why we do that here.
	CRect rectClient;
	GetClientRect( &rectClient );
	OptimizePageSize( rectClient.Width(), rectClient.Height() );

	m_mapTooltipMessages.clear();

	// Current position.
	long lRow = pclSheet->GetMaxRows() + 1;
	long lSelectedRow = 0;
	
	if( true == bSmartControlValveExist )
	{
		// Number of objects.
		int iSmartControlValveCount = m_pclSelectionTable->GetItemCount( CLASS( CDS_SSelSmartControlValve ) );

		if( 0 == iSmartControlValveCount )
		{
			return false;
		}
		
		CDS_SSelSmartControlValve **paSSelSmartControlValve = new CDS_SSelSmartControlValve*[iSmartControlValveCount];

		if( NULL == paSSelSmartControlValve )
		{
			return false;
		}
		
		// Fill the 'paSSelSmartControlValve' array with pointer on each 'CDS_SSelSmartControlValve' object of the 'SMARTCONTROLVALVE_TAB' table.
		int iMaxIndex = 0;
		int i = 0;

		for( IDPTR IDPtr = m_pclSelectionTable->GetFirst( CLASS( CDS_SSelSmartControlValve ) ); '\0' != *IDPtr.ID; IDPtr = m_pclSelectionTable->GetNext() )
		{
			// Sanity tests.
			CDS_SSelSmartControlValve *pSel = dynamic_cast<CDS_SSelSmartControlValve *>( IDPtr.MP );

			if( NULL == pSel )
			{
				continue;
			}
			
			if( NULL == pSel->GetIDPtr().MP )
			{
				continue;
			}
			
			paSSelSmartControlValve[i] = (CDS_SSelSmartControlValve *)(void *)IDPtr.MP;

			if( iMaxIndex < paSSelSmartControlValve[i]->GetpSelectedInfos()->GetRowIndex() )
			{
				iMaxIndex = paSSelSmartControlValve[i]->GetpSelectedInfos()->GetRowIndex();
			}

			++i;
		}
		
		ASSERT( i == iSmartControlValveCount );
		iSmartControlValveCount = i;
		
		// Sort 'paSSelSmartControlValve' array.
		// Remark: 'bResetOrder' is set to 'true' only when the sorting combos are empty (No user choice) and the user clicks on the 'Apply sorting keys'.
		if( true == bResetOrder ) 
		{
			// Reset row index.
			for( i = 0; i < iSmartControlValveCount; i++ )
			{
				if( 0 == paSSelSmartControlValve[i]->GetpSelectedInfos()->GetRowIndex() )
				{
					paSSelSmartControlValve[i]->GetpSelectedInfos()->SetRowIndex( ++iMaxIndex );
				}
			}
		}
		else if( PageField_enum::epfNONE != m_pTADSPageSetup->GetKey( 0 ) || PageField_enum::epfNONE != m_pTADSPageSetup->GetKey( 1 )
				|| PageField_enum::epfNONE != m_pTADSPageSetup->GetKey( 2 ) )
		{
			if( NULL != pDlgLeftTabSelP )
			{
				pDlgLeftTabSelP->SortTable( (CData **)paSSelSmartControlValve, iSmartControlValveCount - 1 );
			}

			// Reset row index.
			for( i = 0; i < iSmartControlValveCount; i++ )
			{
				paSSelSmartControlValve[i]->GetpSelectedInfos()->SetRowIndex( i + 1 );
			}
		}

		// Create a remark index.
		if( NULL != pDlgLeftTabSelP )
		{
			pDlgLeftTabSelP->SetRemarkIndex( CLASS( CDS_SSelSmartControlValve ), (CData **)paSSelSmartControlValve, iSmartControlValveCount );
		}

		CRank rkl;
		enum eSmartControlValveSwitch
		{
			SingleSelSmartControlValve,
			DirSelSmartControlValve,
			LastTapWaterControlSwitch
		};

		bool bAtLeastOneBlockPrinted = false;

		for( int iLoopGroup = 0; iLoopGroup < LastTapWaterControlSwitch; iLoopGroup++ )
		{
			int iCount = 0;

			for( i = 0; i < iSmartControlValveCount; i++ )
			{
				bool bShouldbeAdded = false;
				CDS_SSelSmartControlValve *pclSelSmartControlValve = dynamic_cast<CDS_SSelSmartControlValve *>( paSSelSmartControlValve[i] );
				
				if( NULL == pclSelSmartControlValve || NULL == pclSelSmartControlValve->GetProductIDPtr().MP )
				{
					ASSERT( 0 );
					continue;
				}

				if( DirSelSmartControlValve == iLoopGroup )
				{
					if( false == pclSelSmartControlValve->IsFromDirSel() )
					{
						continue;
					}
				}
				else
				{
					if( true == pclSelSmartControlValve->IsFromDirSel() )
					{
						continue;
					}
				}

				switch( iLoopGroup )
				{
					case SingleSelSmartControlValve:

						if( NULL != dynamic_cast<CDB_SmartControlValve *>( pclSelSmartControlValve->GetProductIDPtr().MP ) )
						{
							bShouldbeAdded = true;
						}

						break;

					case DirSelSmartControlValve:

						if( NULL != dynamic_cast<CDB_SmartControlValve *>( pclSelSmartControlValve->GetProductIDPtr().MP ) )
						{
							bShouldbeAdded = true;
						}

						break;
				}

				if( false == bShouldbeAdded )
				{
					continue;
				}
				
				rkl.Add( _T(""), paSSelSmartControlValve[i]->GetpSelectedInfos()->GetRowIndex(), (LPARAM)paSSelSmartControlValve[i] );
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
			SetLastRow( pclSDSmartControlValve, lRow );

			switch( iLoopGroup )
			{
				case SingleSelSmartControlValve:
					SetPageTitle( pclSDSmartControlValve, IDS_SSHEETSELPROD_SUBTITLESMARTCONTROLVALVEFROMSSEL, false, lRow );
					break;

				case DirSelSmartControlValve:
					SetPageTitle( pclSDSmartControlValve, IDS_SSHEETSELPROD_SUBTITLESMARTCONTROLVALVEFROMDIRSEL, false, lRow );
					break;
			}

			lRow = pclSheet->GetMaxRows();
			SetLastRow( pclSDSmartControlValve, lRow );

			CString str;
			LPARAM lparam;
			bool bFirstPass = true;

			for( BOOL bContinue = rkl.GetFirst( str, lparam ); TRUE == bContinue; bContinue = rkl.GetNext( str, lparam ) )
			{
				pclSDSmartControlValve->RestartRemarkGenerator();

				CData *pclData = (CData *)lparam;
				CDS_SSelSmartControlValve *pclSSelSmartControlValve = dynamic_cast<CDS_SSelSmartControlValve *>( pclData );

				if( NULL == pclSSelSmartControlValve )
				{
					continue;
				}

				if( m_pSelected == pclData )
				{
					lSelectedRow = lRow;
				}

				CDB_SmartControlValve *pclSmartControlValve = pclSSelSmartControlValve->GetProductAs<CDB_SmartControlValve>();

				if( NULL == pclSmartControlValve )
				{
					continue;
				}
				
				m_rProductParam.Clear();
				m_rProductParam.SetSheetDescription( pclSDSmartControlValve );
				m_rProductParam.SetSelectionContainer( (LPARAM)pclSSelSmartControlValve );

				++lRow;
				long lFirstRow = lRow;

				// Draw line above.
				if( false == bFirstPass )
				{
					pclSheet->SetCellBorder( ColumnDescription::Reference1, lRow, ColumnDescription::Footer - 1, lRow, true, SS_BORDERTYPE_TOP );
				}

				long lRowProduct = _FillRowSmartControlValve( pclSDSmartControlValve, lRow, pclSSelSmartControlValve, pclSmartControlValve );
				long lRowGen = CSelProdPageBase::FillRowGen( pclSDSmartControlValve, lRow, pclSSelSmartControlValve );
				lRow = max( lRowGen, lRowProduct );
				
				// Add the selectable row range.
				m_rProductParam.AddRange( lFirstRow, lRow, pclSmartControlValve );

				// Spanning must be done here because it's the only place where we know exactly the number of lines to span!
				// Span reference #1 and #2.
				AddCellSpanW( pclSDSmartControlValve, ColumnDescription::Reference1, lFirstRow, 1, pclSheet->GetMaxRows() - lFirstRow + 1 );
				AddCellSpanW( pclSDSmartControlValve, ColumnDescription::Reference2, lFirstRow, 1, pclSheet->GetMaxRows() - lFirstRow + 1 );

				// Span water characteristic.
				AddCellSpanW( pclSDSmartControlValve, ColumnDescription::Water, lFirstRow, 1, pclSheet->GetMaxRows() - lFirstRow + 1 );

				// Add accessories.
				if( NULL != pclSmartControlValve )
				{
					if( true == pclSSelSmartControlValve->IsAccessoryExist() )
					{
						// Draw dash line.
						pclSheet->SetCellBorder( ColumnDescription::Reference1, lRow, ColumnDescription::Footer - 1, lRow, true, SS_BORDERTYPE_BOTTOM, SS_BORDERSTYLE_FINE_DASH );

						lRow++;
						lRow = FillAccessories( pclSDSmartControlValve, lRow, pclSSelSmartControlValve, pclSSelSmartControlValve->GetpSelectedInfos()->GetQuantity() );
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

		FillRemarks( pclSDSmartControlValve, lRow );

		// We add by default a blank line.
		lRow = pclSheet->GetMaxRows() + 2;
		
		delete[] paSSelSmartControlValve;
	}

	if( true == bSmartControlValveHMExist )
	{
		if( m_vecHMList.size() > 0 )	
		{
			// Create a remark index.
			if( NULL != pDlgLeftTabSelP )
			{
				// Remark: For 'CDS_HydroMod' we don't use anymore for the smart control valve the "FillRemarks" method.
				//         Instead we use remark generator from the "CSheetDescription" class.
				pDlgLeftTabSelP->SetRemarkIndex( &m_vecHMList );
			}

			SetLastRow( pclSDSmartControlValve, lRow );
			SetPageTitle( pclSDSmartControlValve, IDS_SSHEETSELPROD_SUBTITLESMARTCONTROLVALVEFROMHMCALC, false, lRow );	
			
			lRow++;
			SetLastRow( pclSDSmartControlValve, lRow );
			
			// For each element.
			bool bFirstPass = true;
			HMvector::iterator It;

			for( It = m_vecHMList.begin(); It != m_vecHMList.end(); It++ )
			{
				pclSDSmartControlValve->RestartRemarkGenerator();

				CDS_HydroMod *pHM = NULL;
				CDS_HydroMod::CBase *pBase = NULL;

				if( eptHM == (*It).first )
				{
					pHM = dynamic_cast<CDS_HydroMod *>( (CData*)( (*It).second ) );
				}
				else
				{
					// Smart control valve exists and TA product exist into the DB.
					pBase = static_cast<CDS_HydroMod::CBase *>( (*It).second );

					if( NULL != pBase )
					{
						pHM = pBase->GetpParentHM();
					}
				}

				if( NULL != pHM )
				{
					// Draw line above.
					if( false == bFirstPass )
					{
						pclSheet->SetCellBorder( ColumnDescription::Reference1, lRow, ColumnDescription::Footer - 1, lRow, true, SS_BORDERTYPE_TOP );
					}

					m_rProductParam.Clear();
					m_rProductParam.SetSheetDescription( pclSDSmartControlValve );
					// Intentionally set to NULL to avoid clicking, double clicking or right clicking.
					m_rProductParam.SetSelectionContainer( (LPARAM)0 );

					long lLastRowSmartControlValve = _FillRowSmartControlValve( pclSDSmartControlValve, lRow, pHM, pBase );
					long lLastRowGeneral = CSelProdPageBase::FillRowGen( pclSDSmartControlValve, lRow, pHM, pBase );

					long lLastRow = max( lLastRowGeneral, lLastRowSmartControlValve );
					
					// HYS-721: Merge cells
					long lFirstRow = lRow;
					
					// Spanning must be done here because it's the only place where we know exactly number of lines to span!
					// Span reference #1 and #2.
					AddCellSpanW(pclSDSmartControlValve, ColumnDescription::Reference1, lFirstRow, 1, pclSheet->GetMaxRows() - lFirstRow + 1);
					AddCellSpanW(pclSDSmartControlValve, ColumnDescription::Reference2, lFirstRow, 1, pclSheet->GetMaxRows() - lFirstRow + 1);
					
					// Span water characteristic.
					AddCellSpanW(pclSDSmartControlValve, ColumnDescription::Water, lFirstRow, 1, pclSheet->GetMaxRows() - lFirstRow + 1);

					CDB_TAProduct *pclSmartControlValve = NULL;

					// If 'pHM' exists and not 'pclSmartControlValve' means that smart control valve has been added in CBI mode.
					if( NULL == pBase )
					{
						pclSmartControlValve = dynamic_cast<CDB_TAProduct*>(pHM->GetCBIValveIDPtr().MP);
					}
					else
					{
						pclSmartControlValve = dynamic_cast<CDB_TAProduct *>(pHM->GetpSmartControlValve()->GetIDPtr().MP);
					}

					if( NULL != pclSmartControlValve )
					{
						CDB_TAProduct *pclTAProd = dynamic_cast<CDB_TAProduct *>( pclSmartControlValve->GetIDPtr().MP );
						lLastRow = FillAndAddBuiltInHMAccessories( pclSheet, pclSDSmartControlValve, pclTAProd, lLastRow );
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

				// For error, warning and info messages we use now the remark generator from the "CSheetDescription" class.
				lRow = pclSDSmartControlValve->WriteRemarks( lRow, ColumnDescription::Reference1, ColumnDescription::Footer - 1 );
				
				// The 'WriteRemarks' method just above returns well the next position but the internal last row in TSpread is not set.
				SetLastRow( pclSDSmartControlValve, lRow );
			}
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

bool CSelProdPageSmartControlValve::HasSomethingToDisplay( void )
{
	bool bSmartControlValveExist = ( NULL != m_pclSelectionTable ) ? true : false;
	bool bSmartControlValveHMExist = ( m_vecHMList.size() > 0 ) ? true : false;

	if( false == bSmartControlValveExist && false == bSmartControlValveHMExist )
	{
		return false;
	}

	return true;
}

void CSelProdPageSmartControlValve::_InitColHeader( CSheetDescription *pclSheetDescription )
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

long CSelProdPageSmartControlValve::_FillRowSmartControlValve( CSheetDescription *pclSheetDescription, long lRow, CDS_SSelSmartControlValve *pclSSelSmartControlValve, CDB_SmartControlValve *pclSmartControlValve )
{
	if( NULL == pclSheetDescription || NULL == pclSheetDescription->GetSSheetPointer() || NULL == pclSSelSmartControlValve || NULL == pclSmartControlValve )
	{
		return lRow;
	}

	CSSheet *pclSheet = pclSheetDescription->GetSSheetPointer();
	CString str1, str2;
	long lFirstRow = lRow;

	pclSheet->SetTextPattern( CSSheet::_SSTextPattern::UserStaticColumn );
	pclSheet->SetTextPatternProperty( CSSheet::_SSTextPatternProperty::TPP_FontBold, (LPARAM)FALSE );

	// Add item into the full article list.
	_AddArticleList( pclSSelSmartControlValve );
		
	// Column TA Product.
	_FillRowSmartControlValveProduct( pclSheetDescription, lRow, pclSmartControlValve, pclSSelSmartControlValve->GetpSelectedInfos()->GetQuantity() );

	// Column infos.
	if( false == pclSSelSmartControlValve->IsFromDirSel() )
	{
		SmartControlValveParams rSmartControlValveParams;

		rSmartControlValveParams.pclSmartControlValve = pclSmartControlValve;
		rSmartControlValveParams.dFlowMax = pclSSelSmartControlValve->GetQ();
		rSmartControlValveParams.dDpMin = CalcDp( rSmartControlValveParams.dFlowMax, pclSmartControlValve->GetSmartValveCharacteristic()->GetKvs(), 
				pclSSelSmartControlValve->GetpSelectedInfos()->GetpWCData()->GetDens() );

		rSmartControlValveParams.eLocalization = SmartValveLocalization::SmartValveLocNone;
		rSmartControlValveParams.eControlMode = SmartValveControlMode::SCVCM_Flow;
		rSmartControlValveParams.dFlowMax = rSmartControlValveParams.dFlowMax;
		rSmartControlValveParams.strPowerDT = _T("");

		if( CDS_SelProd::efdPower == pclSSelSmartControlValve->GetFlowDef() )
		{
			rSmartControlValveParams.eControlMode = SmartValveControlMode::SCVCM_Power;
			
			rSmartControlValveParams.strPowerDT = TASApp.LoadLocalizedString( IDS_SSHEETSELPROD_SMARTCONTROLVAVEPOWERMAX );
			rSmartControlValveParams.strPowerDT += _T(": ") + CString( WriteCUDouble( _U_TH_POWER, pclSSelSmartControlValve->GetPower(), true ) );
			rSmartControlValveParams.strPowerDT += _T(" / ");
			rSmartControlValveParams.strPowerDT += WriteCUDouble( _U_DIFFTEMP, pclSSelSmartControlValve->GetDT(), true );
			rSmartControlValveParams.dPowerMax = pclSSelSmartControlValve->GetPower();
		}

		_FillRowSmartControlValveInfo( pclSheetDescription, lFirstRow, rSmartControlValveParams );
	}

	return pclSheet->GetMaxRows();
}

long CSelProdPageSmartControlValve::_FillRowSmartControlValveProduct( CSheetDescription *pclSheetDescription, long lRow, CDB_SmartControlValve *pclSmartControlValve, int iQuantity, CDS_HydroMod *pclHM )
{
	if( NULL == pclSheetDescription || NULL == pclSmartControlValve || NULL == TASApp.GetpTADS() || NULL == TASApp.GetpTADS()->GetpTechParams() )
	{
		return lRow;
	}

	CString str1, str2;
	long lFirstRow = lRow;
	
	// Column TA Product.
	AddStaticText( pclSheetDescription, ColumnDescription::Product, lRow, _T("-") );

	if( true == m_pTADSPageSetup->GetField( epfSMARTCONTROLVALVEINFOPRODUCTNAME ) )	
	{
		lRow = AddStaticText( pclSheetDescription, ColumnDescription::Product, lRow, pclSmartControlValve->GetName() );
	}

	if( true == m_pTADSPageSetup->GetField( epfSMARTCONTROLVALVEINFOPRODUCTBDYMATERIAL ) )	
	{
		lRow = AddStaticText( pclSheetDescription, ColumnDescription::Product, lRow, pclSmartControlValve->GetBodyMaterial() );
	}
	
	if( true == m_pTADSPageSetup->GetField( epfSMARTCONTROLVALVEINFOPRODUCTCONNECTION ) )	
	{
		lRow = AddStaticText( pclSheetDescription, ColumnDescription::Product, lRow, pclSmartControlValve->GetConnect() );
	}

	if( true == m_pTADSPageSetup->GetField( epfSMARTCONTROLVALVEINFOPRODUCTPN ) )
	{
		long lRowTemp = lRow;
		lRow = FillPNTminTmax( pclSheetDescription, ColumnDescription::Product, lRow, pclSmartControlValve );

		if( NULL != pclHM && NULL != pclHM->GetpSmartControlValve() )
		{
			int iValidity = pclHM->GetpSmartControlValve()->CheckValidity();

			if( CDS_HydroMod::eValidityFlags::evfTempTooLow == ( iValidity & CDS_HydroMod::eValidityFlags::evfTempTooLow )
					|| CDS_HydroMod::eValidityFlags::evfTempTooHigh == ( iValidity & CDS_HydroMod::eValidityFlags::evfTempTooHigh ) )
			{
				// Set the PN/temp text in red.
				pclSheetDescription->GetSSheetPointer()->SetForeColor( ColumnDescription::Product, lRowTemp, _RED );

				// Add tooltip.
				CString strErrMsg = pclHM->GetpSmartControlValve()->GetErrorMessageString( CDS_HydroMod::ErrorWarningMessageFlags::EWMF_Temp );
				AddTooltipMessage( CPoint( ColumnDescription::Product, lRowTemp ), strErrMsg );

				// Add remark to put below.
				CString strCellText = pclSheetDescription->GetSSheetPointer()->GetCellText( ColumnDescription::Product, lRowTemp );
				pclSheetDescription->WriteTextWithFlags( strCellText, ColumnDescription::Product, lRowTemp, CSheetDescription::RemarkFlags::ErrorMessage, strErrMsg, _RED );
			}
		}
	}
		
	if( true == m_pTADSPageSetup->GetField( epfSMARTCONTROLVALVEINFOPRODUCTKVS ) )	
	{
		if( NULL != pclSmartControlValve->GetSmartValveCharacteristic() && -1.0 != pclSmartControlValve->GetSmartValveCharacteristic()->GetKvs() )
		{
			if( 0 == m_pUnitDB->GetDefaultUnitIndex( _C_KVCVCOEFF ) )
			{
				str1 = TASApp.LoadLocalizedString( IDS_KVS );
			}
			else
			{
				str1 = TASApp.LoadLocalizedString( IDS_CV );
			}

			str1 += CString( _T(" = ") ) + WriteCUDouble( _C_KVCVCOEFF, pclSmartControlValve->GetSmartValveCharacteristic()->GetKvs() );
			lRow = AddStaticText( pclSheetDescription, ColumnDescription::Product, lRow, str1 );
		}
	}

	if( true == m_pTADSPageSetup->GetField( epfSMARTCONTROLVALVEINFOPRODUCTQNOM ) )	
	{
		if( NULL != pclSmartControlValve->GetSmartValveCharacteristic() && -1.0 != pclSmartControlValve->GetSmartValveCharacteristic()->GetQnom() )
		{
			str1 = CString( _T("Qnom = ") ) + WriteCUDouble( _U_FLOW, pclSmartControlValve->GetSmartValveCharacteristic()->GetQnom(), true );
			lRow = AddStaticText( pclSheetDescription, ColumnDescription::Product, lRow, str1 );
		}
	}

	// HYS-1660

	pclSheetDescription->GetSSheetPointer()->SetTextPatternProperty( CSSheet::_SSTextPatternProperty::TPP_StyleWordWrap, (LPARAM)TRUE );

	// Write power supply.
	if( true == m_pTADSPageSetup->GetField( epfSMARTCONTROLVALVEINFOPRODUCTPOWERSUPPLY ) )	
	{
		str1 = TASApp.LoadLocalizedString( IDS_SSHEETSELPROD_POWSUPPL ) + CString( _T( " : " ) ) + pclSmartControlValve->GetPowerSupplyStr();
		lRow = AddStaticText( pclSheetDescription, ColumnDescription::Product, lRow, str1 );
	}

	// Write input signal.
	if( true == m_pTADSPageSetup->GetField( epfSMARTCONTROLVALVEINFOPRODUCTINPUTSIGNAL ) )	
	{
		str1 = TASApp.LoadLocalizedString( IDS_SSHEETSELPROD_INPUTSIG ) + CString( _T( " : " ) ) + pclSmartControlValve->GetInOutSignalsStr( true );
		lRow = AddStaticText( pclSheetDescription, ColumnDescription::Product, lRow, str1, true );
	}

	// Write output signal.
	if( true == m_pTADSPageSetup->GetField( epfSMARTCONTROLVALVEINFOPRODUCTOUTPUTSIGNAL ) )	
	{
		str1 = TASApp.LoadLocalizedString( IDS_SSHEETSELPROD_OUTPUTSIG );

		if( CString( _T( "" ) ) == pclSmartControlValve->GetInOutSignalsStr( false ) )
		{
			str1 += CString( _T( " :    -" ) );
		}
		else
		{
			str1 += CString( _T( " : " ) ) + pclSmartControlValve->GetInOutSignalsStr( false );
		}
		
		lRow = AddStaticText( pclSheetDescription, ColumnDescription::Product, lRow, str1, true );
		pclSheetDescription->GetSSheetPointer()->SetTextPatternProperty( CSSheet::_SSTextPatternProperty::TPP_StyleWordWrap, (LPARAM)FALSE );
	}

	// Column 'Quantity'.
	FillQtyPriceCol( pclSheetDescription, lFirstRow, pclSmartControlValve, iQuantity );
	
	// Column 'Article number'.
	AddStaticText( pclSheetDescription, ColumnDescription::ArticleNumber, lFirstRow, _T("-") );

	// Remark: 'true' to exclude any connection reference in the article number.
	FillArtNumberCol( pclSheetDescription, ColumnDescription::ArticleNumber, lFirstRow, pclSmartControlValve, pclSmartControlValve->GetArtNum( true ) );
	return lRow;
}

long CSelProdPageSmartControlValve::_FillRowSmartControlValve( CSheetDescription *pclSheetDescription, long lRow, CDS_HydroMod *pclHM, CDS_HydroMod::CBase *pclBase, CDB_TAProduct **ppSmartControlValve )
{
	if( NULL == pclSheetDescription || NULL == pclSheetDescription->GetSSheetPointer() || NULL == pclHM || NULL == pclBase )
	{
		return lRow;
	}

	CSSheet *pclSheet = pclSheetDescription->GetSSheetPointer();
	CString str1, str2;
	long lFirstRow = lRow;

	SmartControlValveParams rSmartControlValveParams;

	CDB_SmartControlValve *pclSmartControlValve = NULL;
	
	// If 'pHM' exists and not 'pclSmartControlValve' means that valve has been added in CBI mode.
	if( NULL == pclBase ) 
	{
		pclSmartControlValve = dynamic_cast<CDB_SmartControlValve *>( pclHM->GetCBIValveIDPtr().MP );
		rSmartControlValveParams.dFlowMax = pclHM->GetQDesign();
		rSmartControlValveParams.dDp = -1.0;
		rSmartControlValveParams.dDpMin = -1.0;
		rSmartControlValveParams.eLocalization = SmartValveLocalization::SmartValveLocNone;
		rSmartControlValveParams.eControlMode = SmartValveControlMode::SCVCM_Flow;
		rSmartControlValveParams.dPowerMax = 0.0;
	}
	else
	{
		pclSmartControlValve = dynamic_cast<CDB_SmartControlValve *>( pclHM->GetpSmartControlValve()->GetIDPtr().MP );
		rSmartControlValveParams.dFlowMax = pclHM->GetpSmartControlValve()->GetQ();
		rSmartControlValveParams.dDp = pclHM->GetpSmartControlValve()->GetDp();
		rSmartControlValveParams.dDpMin = pclHM->GetpSmartControlValve()->GetDpMin();
		rSmartControlValveParams.eLocalization = pclHM->GetpSmartControlValve()->GetLocalization();
		rSmartControlValveParams.eControlMode = pclHM->GetpSmartControlValve()->GetControlMode();

		if( SmartValveControlMode::SCVCM_Power == rSmartControlValveParams.eControlMode )
		{
			rSmartControlValveParams.dPowerMax = pclHM->GetpSmartControlValve()->GetPowerMax();

			rSmartControlValveParams.strPowerDT = TASApp.LoadLocalizedString( IDS_SSHEETSELPROD_SMARTCONTROLVAVEPOWERMAX );
			rSmartControlValveParams.strPowerDT += _T(": ") + CString( WriteCUDouble( _U_TH_POWER, pclHM->GetpSmartControlValve()->GetPowerMax(), true ) );
			rSmartControlValveParams.strPowerDT += _T(" / ");
			rSmartControlValveParams.strPowerDT += WriteCUDouble( _U_DIFFTEMP, pclHM->GetpSmartControlValve()->GetDT(), true );
		}
	}

	if( NULL == pclSmartControlValve )
	{
		return pclSheet->GetMaxRows();
	}
	
	rSmartControlValveParams.pclSmartControlValve = pclSmartControlValve;
	rSmartControlValveParams.pclHM = pclHM;

	pclSheet->SetTextPattern( CSSheet::_SSTextPattern::UserStaticColumn );
	pclSheet->SetTextPatternProperty( CSSheet::_SSTextPatternProperty::TPP_FontBold, (LPARAM)FALSE );

	// Add item into the full article list.
	_AddArticleList( pclSmartControlValve, 1 );

	// Column TA Product.
	_FillRowSmartControlValveProduct( pclSheetDescription, lRow, pclSmartControlValve, 1, pclHM );

	// Column infos.
	_FillRowSmartControlValveInfo( pclSheetDescription, lFirstRow, rSmartControlValveParams );
	
	if( NULL != ppSmartControlValve )
	{
		*ppSmartControlValve = pclSmartControlValve;
	}

	return pclSheet->GetMaxRows();
}

long CSelProdPageSmartControlValve::_FillRowSmartControlValveInfo( CSheetDescription *pclSheetDescription, long lRow, SmartControlValveParams &rSmartControlValveParams )
{
	if( NULL == pclSheetDescription || NULL == pclSheetDescription->GetSSheetPointer() || NULL == rSmartControlValveParams.pclSmartControlValve )
	{
		return lRow;
	}

	CString str1;
	int iValidity = -1;
	
	// Column Infos.
	AddStaticText( pclSheetDescription, ColumnDescription::TechnicalInfos, lRow, _T("-") );			// Default string

	if( true == m_pTADSPageSetup->GetField( epfSMARTCONTROLVALVETECHINFOFLOW ) )
	{
		if( rSmartControlValveParams.dFlowMax > 0.0 )
		{
			 CString strFlowErrorMsg = _T("");
			 CString strPowerErrorMsg = _T("");

			// Check if flow or/and power error.
			if( NULL != rSmartControlValveParams.pclHM && NULL != rSmartControlValveParams.pclHM->GetpSmartControlValve() )
			{
				// Update error messages.
				iValidity = rSmartControlValveParams.pclHM->GetpSmartControlValve()->CheckValidity();

				if( CDS_HydroMod::eValidityFlags::evfFlowTooLow == ( iValidity & CDS_HydroMod::eValidityFlags::evfFlowTooLow )
						|| CDS_HydroMod::eValidityFlags::evfFlowTooHigh == ( iValidity & CDS_HydroMod::eValidityFlags::evfFlowTooHigh ) )
				{
					strFlowErrorMsg = rSmartControlValveParams.pclHM->GetpSmartControlValve()->GetErrorMessageString( CDS_HydroMod::ErrorWarningMessageFlags::EWMF_Flow );
				}
					
				if( CDS_HydroMod::eValidityFlags::evfPowerTooLow == ( iValidity & CDS_HydroMod::eValidityFlags::evfPowerTooLow )
						|| CDS_HydroMod::eValidityFlags::evfPowerTooHigh == ( iValidity & CDS_HydroMod::eValidityFlags::evfPowerTooHigh ) )
				{
					strPowerErrorMsg = rSmartControlValveParams.pclHM->GetpSmartControlValve()->GetErrorMessageString( CDS_HydroMod::ErrorWarningMessageFlags::EWMF_Power );
				}
			}

			// HYS-38: Show power dt info when their radio button is checked.
			if( false == rSmartControlValveParams.strPowerDT.IsEmpty() )
			{
				// Power.
				if( true == strPowerErrorMsg.IsEmpty() )
				{
					lRow = AddStaticText( pclSheetDescription, ColumnDescription::TechnicalInfos, lRow, rSmartControlValveParams.strPowerDT );
				}
				else
				{
					pclSheetDescription->WriteTextWithFlags( rSmartControlValveParams.strPowerDT, ColumnDescription::TechnicalInfos, lRow, CSheetDescription::RemarkFlags::ErrorMessage, strPowerErrorMsg, _RED );
					AddTooltipMessage( CPoint( ColumnDescription::TechnicalInfos, lRow ), strPowerErrorMsg );
					lRow++;
				}

				// Flow max between parenthesis.
				CString str = _T("(") + TASApp.LoadLocalizedString( IDS_SSHEETSELPROD_SMARTCONTROLVAVEQMAX );
				str += WriteCUDouble( _U_FLOW, rSmartControlValveParams.dFlowMax, true );
				str += _T(")");

				if( true == strFlowErrorMsg.IsEmpty() )
				{
					lRow = AddStaticText( pclSheetDescription, ColumnDescription::TechnicalInfos, lRow, str );
				}
				else
				{
					pclSheetDescription->WriteTextWithFlags( str, ColumnDescription::TechnicalInfos, lRow, CSheetDescription::RemarkFlags::ErrorMessage, strFlowErrorMsg, _RED );
					AddTooltipMessage( CPoint( ColumnDescription::TechnicalInfos, lRow ), strFlowErrorMsg );
					lRow++;
				}
			}
			else
			{
				CString str = TASApp.LoadLocalizedString( IDS_SSHEETSELPROD_SMARTCONTROLVAVEQMAX );
				str += _T(": ") + CString( WriteCUDouble( _U_FLOW, rSmartControlValveParams.dFlowMax, true ) );
				
				if( true == strFlowErrorMsg.IsEmpty() )
				{
					lRow = AddStaticText( pclSheetDescription, ColumnDescription::TechnicalInfos, lRow, str );
				}
				else
				{
					pclSheetDescription->WriteTextWithFlags( str, ColumnDescription::TechnicalInfos, lRow, CSheetDescription::RemarkFlags::ErrorMessage, strFlowErrorMsg, _RED );
					AddTooltipMessage( CPoint( ColumnDescription::TechnicalInfos, lRow ), strFlowErrorMsg );
					lRow++;
				}
			}
		}
		else
		{
			ASSERT( 0 );
		}
	}

	if( true == m_pTADSPageSetup->GetField( epfSMARTCONTROLVALVETECHINFODP ) )
	{
		// Show pressure drop accross the smart control valve.

		CString strDpErrorMsg = _T("");

		// Check if pressure drop is in error.
		if( NULL != rSmartControlValveParams.pclHM && NULL != rSmartControlValveParams.pclHM->GetpSmartControlValve() )
		{
			// Update error messages if needed.
			if( -1 == iValidity )
			{
				iValidity = rSmartControlValveParams.pclHM->GetpSmartControlValve()->CheckValidity();
			}

			if( CDS_HydroMod::eValidityFlags::evfDpTooLow == ( iValidity & CDS_HydroMod::eValidityFlags::evfDpTooLow )
					|| CDS_HydroMod::eValidityFlags::evfDpTooHigh == ( iValidity & CDS_HydroMod::eValidityFlags::evfDpTooHigh ) )
			{
				strDpErrorMsg = rSmartControlValveParams.pclHM->GetpSmartControlValve()->GetErrorMessageString( CDS_HydroMod::ErrorWarningMessageFlags::EWMF_Dp );
			}
		}

		CString str = TASApp.LoadLocalizedString( IDS_SSHEETSELPROD_SMARTCONTROLVALVEDP );
		str += _T(": ");
		str += ( rSmartControlValveParams.dDp > 0.0 ) ? WriteCUDouble( _U_DIFFPRESS, rSmartControlValveParams.dDp, true ) : GetDashDotDash();

		if( true == strDpErrorMsg.IsEmpty() )
		{
			lRow = AddStaticText( pclSheetDescription, ColumnDescription::TechnicalInfos, lRow, str );
		}
		else
		{
			pclSheetDescription->WriteTextWithFlags( str, ColumnDescription::TechnicalInfos, lRow, CSheetDescription::RemarkFlags::ErrorMessage, strDpErrorMsg, _RED );
			AddTooltipMessage( CPoint( ColumnDescription::TechnicalInfos, lRow ), strDpErrorMsg );
			lRow++;
		}
	}

	if( true == m_pTADSPageSetup->GetField( epfSMARTCONTROLVALVETECHINFODPMIN ) )
	{
		// Show Dp min only if different from -1.
		CString str = TASApp.LoadLocalizedString( IDS_SSHEETSELPROD_SMARTCONTROLVALVEDPMIN );
		str += _T(": ");
		str += ( rSmartControlValveParams.dDpMin != -1.0 ) ? WriteCUDouble( _U_DIFFPRESS, rSmartControlValveParams.dDpMin, true ) : GetDashDotDash();
		lRow = AddStaticText( pclSheetDescription, ColumnDescription::TechnicalInfos, lRow, str );
	}

	if( true == m_pTADSPageSetup->GetField( epfSMARTCONTROLVALVETECHINFOLOCALIZATION ) && SmartValveLocalization::SmartValveLocNone != rSmartControlValveParams.eLocalization )
	{
		CString str = TASApp.LoadLocalizedString( IDS_SSHEETSELPROD_SMARTCONTROLVALVELOCALIZATION );

		int iIDS = ( SmartValveLocalization::SmartValveLocSupply == rSmartControlValveParams.eLocalization ) ? IDS_SSHEETSELPROD_SMARTVALVELOCALIZATION_SUPPLY : 
				IDS_SSHEETSELPROD_SMARTVALVELOCALIZATION_RETURN;

		str += _T(": ") + TASApp.LoadLocalizedString( iIDS );
		lRow = AddStaticText( pclSheetDescription, ColumnDescription::TechnicalInfos, lRow, str );
	}

	if( true == m_pTADSPageSetup->GetField( epfSMARTCONTROLVALVETECHINFOCONTROLMODE ) )
	{
		CString str = TASApp.LoadLocalizedString( IDS_SSHEETSELPROD_SMARTCONTROLVALVECONTROLMODE );

		int iIDS = ( SmartValveControlMode::SCVCM_Flow == rSmartControlValveParams.eControlMode ) ? IDS_SSHEETSELPROD_SMARTCONTROLVALVECONTROLMODE_FLOW : 
				IDS_SSHEETSELPROD_SMARTCONTROLVALVECONTROLMODE_POWER;

		str += _T(": ") + TASApp.LoadLocalizedString( iIDS );
		lRow = AddStaticText( pclSheetDescription, ColumnDescription::TechnicalInfos, lRow, str );
	}

	if( true == m_pTADSPageSetup->GetField( epfSMARTCONTROLVALVETECHINFOPOWERMAX ) && SmartValveControlMode::SCVCM_Power == rSmartControlValveParams.eControlMode 
			&& true == rSmartControlValveParams.strPowerDT.IsEmpty() )
	{
		CString strPowerErrorMsg = _T("");

		if( NULL != rSmartControlValveParams.pclHM && NULL != rSmartControlValveParams.pclHM->GetpSmartControlValve() )
		{
			// Update error messages if needed.
			if( -1 == iValidity )
			{
				iValidity = rSmartControlValveParams.pclHM->GetpSmartControlValve()->CheckValidity();
			}

			if( CDS_HydroMod::eValidityFlags::evfPowerTooLow == ( iValidity & CDS_HydroMod::eValidityFlags::evfPowerTooLow )
					|| CDS_HydroMod::eValidityFlags::evfPowerTooHigh == ( iValidity & CDS_HydroMod::eValidityFlags::evfPowerTooHigh ) )
			{
				strPowerErrorMsg = rSmartControlValveParams.pclHM->GetpSmartControlValve()->GetErrorMessageString( CDS_HydroMod::ErrorWarningMessageFlags::EWMF_Power );
			}
		}

		CString str = TASApp.LoadLocalizedString( IDS_SSHEETSELPROD_SMARTCONTROLVAVEPOWERMAX );
		str += _T(": ") + CString( WriteCUDouble( _U_TH_POWER, rSmartControlValveParams.dPowerMax, true ) );

		if( true == strPowerErrorMsg.IsEmpty() )
		{
			lRow = AddStaticText( pclSheetDescription, ColumnDescription::TechnicalInfos, lRow, str );
		}
		else
		{
			pclSheetDescription->WriteTextWithFlags( str, ColumnDescription::TechnicalInfos, lRow, CSheetDescription::RemarkFlags::ErrorMessage, strPowerErrorMsg, _RED );
			AddTooltipMessage( CPoint( ColumnDescription::TechnicalInfos, lRow ), strPowerErrorMsg );
			lRow++;
		}
	}

	return lRow;
}

void CSelProdPageSmartControlValve::_AddArticleList( CDB_TAProduct *pclTAP, int iQuantity )
{
	if( NULL == pclTAP || NULL == dynamic_cast<CDB_SmartControlValve *>( pclTAP ) )
	{
		ASSERT_RETURN;
	}

	CDB_SmartControlValve *pclSmartControlValve = dynamic_cast<CDB_SmartControlValve *>( pclTAP );

	if( NULL == pclSmartControlValve->GetSmartValveCharacteristic() )
	{
		ASSERT_RETURN;
	}

	CArticleGroup *pclArticleGroup = new CArticleGroup();
	
	if( NULL == pclArticleGroup )
	{
		ASSERT_RETURN;
	}

	CArticleContainer *pclArticleContainer = pclArticleGroup->AddArticle( pclTAP, iQuantity );

	if( NULL == pclArticleContainer || NULL == pclArticleContainer->GetArticleItem() )
	{
		ASSERT_RETURN;
	}

	// Add Kvs and nominal flow.
	CString strDescription = pclArticleContainer->GetArticleItem()->GetDescription();
	
	strDescription += _T("; ") + GetKvCVString() + _T(" = ") + CString( WriteCUDouble( _C_KVCVCOEFF, pclSmartControlValve->GetSmartValveCharacteristic()->GetKvs() ) );
	strDescription += _T("; Qnom = ") + CString( WriteCUDouble( _U_FLOW, pclSmartControlValve->GetSmartValveCharacteristic()->GetQnom(), true ) );
	strDescription += _T("; ") + TASApp.LoadLocalizedString( IDS_SELP_CABLELENGTHTOREMOTETEMPSENSOR ) + _T(" = ");
	strDescription += CString( WriteCUDouble( _U_LENGTH, pclSmartControlValve->GetRemoteTempSensorCableLength(), true ) );
	pclArticleContainer->GetArticleItem()->SetDescription( strDescription );

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

void CSelProdPageSmartControlValve::_AddArticleList( CDS_SSelSmartControlValve *pclSSelSmartControlValve )
{
	if( NULL == pclSSelSmartControlValve )
	{
		return;
	}

	CDB_SmartControlValve *pclSmartControlValve = pclSSelSmartControlValve->GetProductAs<CDB_SmartControlValve>();

	if( NULL == pclSmartControlValve )
	{
		return;
	}

	int iQuantity = pclSSelSmartControlValve->GetpSelectedInfos()->GetQuantity();
	CArticleGroup *pclArticleGroup = new CArticleGroup();
	ASSERT( NULL != pclArticleGroup );
	
	if( NULL == pclArticleGroup )
	{
		return;
	}

	CArticleContainer *pclArticleContainer = pclArticleGroup->AddArticle( pclSmartControlValve, iQuantity );

	if( NULL == pclArticleContainer || NULL == pclArticleContainer->GetArticleItem() )
	{
		ASSERT_RETURN;
	}

	// Add Kvs and nominal flow.
	CString strDescription = pclArticleContainer->GetArticleItem()->GetDescription();
	
	strDescription += _T("; ") + GetKvCVString() + _T(" = ") + CString( WriteCUDouble( _C_KVCVCOEFF, pclSmartControlValve->GetSmartValveCharacteristic()->GetKvs() ) );
	strDescription += _T("; Qnom = ") + CString( WriteCUDouble( _U_FLOW, pclSmartControlValve->GetSmartValveCharacteristic()->GetQnom(), true ) );
	strDescription += _T("; ") + TASApp.LoadLocalizedString( IDS_SELP_CABLELENGTHTOREMOTETEMPSENSOR ) + _T(" = ");
	strDescription += CString( WriteCUDouble( _U_LENGTH, pclSmartControlValve->GetRemoteTempSensorCableLength(), true ) );

	pclArticleContainer->GetArticleItem()->SetDescription( strDescription );

	AddAccessoriesInArticleContainer( pclSSelSmartControlValve->GetAccessoryList(), pclArticleContainer, iQuantity, false );

	CSelProdPageBase::AddArticleInGroupList( pclArticleGroup );

	// Remark: pay attention that 'CPage::AddArticleList' will increment quantity if article already exist or
	//         will create a new 'CArticleContainer' and will copy to it data passed as argument. This is the reason
	//         why you need to delete 'pclArticleContainer' after the call.
	delete pclArticleGroup;

	// Add connection component if needed.
	if( true == pclSmartControlValve->IsConnTabUsed() )
	{
		// Inlet.
		CSelProdPageBase::AddCompArtList( pclSmartControlValve, iQuantity, true );

		// Outlet.
		CSelProdPageBase::AddCompArtList( pclSmartControlValve, iQuantity, false );
	}
}
