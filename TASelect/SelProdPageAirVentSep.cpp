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
#include "SelProdPageAirVentSep.h"

CSelProdPageAirVentSep::CSelProdPageAirVentSep( CArticleGroupList *pclArticleGroupList ) 
	: CSelProdPageBase( CDB_PageSetup::enCheck::AIRVENTSEPARATOR, pclArticleGroupList )
{
	m_pSelected = NULL;
}

void CSelProdPageAirVentSep::Reset( void )
{
	CSelProdPageBase::Reset();
}

bool CSelProdPageAirVentSep::PreInit( HMvector& vecHMList )
{
	// Something for individual or direct selection ?
	CTable* pclTable = (CTable*)( TASApp.GetpTADS()->Get( _T("AIRVENTSEP_TAB") ).MP );
	if ( NULL == pclTable )
	{
		ASSERT( 0 );
		return false;
	}
	m_pclSelectionTable = ( '\0' != *pclTable->GetFirst().ID ) ? pclTable : NULL;
	bool fAirVentSeparatorExist = ( NULL != m_pclSelectionTable );

	// Something for hydronic calculation?
	bool fAirVentSeparatorHMExist = ( vecHMList.size() > 0 );

	if( false == fAirVentSeparatorExist && false == fAirVentSeparatorHMExist )
		return false;
	
	m_vecHMList = vecHMList;
	SortTable();
	return true;
}

bool CSelProdPageAirVentSep::Init( bool bResetOrder, bool bPrint )
{
	if( NULL == pDlgLeftTabSelP )
	{
		ASSERTA_RETURN( false );
	}

	bool bAirVentSeparatorExist = ( NULL != m_pclSelectionTable ) ? true : false;
	bool bAirVentSeparatorHMExist = ( m_vecHMList.size() > 0 ) ? true : false;

	if( false == bAirVentSeparatorExist && false == bAirVentSeparatorHMExist )
	{
		return false;
	}

	// Call base class method to get a new 'Sheet'.
	CSheetDescription *pclSDAirVentSeparator = CMultiSpreadBase::CreateSSheet( SD_AirVentSep );

	if( NULL == pclSDAirVentSeparator || NULL == pclSDAirVentSeparator->GetSSheetPointer() )
	{
		return false;
	}
	
	CSSheet *pclSheet = pclSDAirVentSeparator->GetSSheetPointer();

	if( false == CSelProdPageBase::PrepareSheet( pclSDAirVentSeparator, ColumnDescription::Pointer, bPrint ) )
	{
		m_ViewDescription.RemoveOneSheetDescription( SD_AirVentSep );
		return false;
	}

	// Initialize column width for header and footer column.
	if( false == m_bForPrint )
	{
		SetColWidth( pclSDAirVentSeparator, ColumnDescription::Header, DefaultColumnWidth::DCW_Header );
		SetColWidth( pclSDAirVentSeparator, ColumnDescription::Footer, DefaultColumnWidth::DCW_Footer );
	}
	else
	{
		pclSheet->ShowCol( ColumnDescription::Header, FALSE );
		pclSheet->ShowCol( ColumnDescription::Footer, FALSE );
	}

	double dWidth = ( true == m_pTADSPageSetup->GetField( PageField_enum::epfFIRSTREF ) ) ? DefaultColumnWidth::DCW_Reference1 : 0.0;
	SetColWidth( pclSDAirVentSeparator, ColumnDescription::Reference1, dWidth );
	
	dWidth = ( true == m_pTADSPageSetup->GetField( PageField_enum::epfSECONDREF ) ) ? DefaultColumnWidth::DCW_Reference2 : 0.0;
	SetColWidth( pclSDAirVentSeparator, ColumnDescription::Reference2, dWidth );
	
	dWidth = ( true == m_pTADSPageSetup->GetField( PageField_enum::epfWATERINFO ) ) ? DefaultColumnWidth::DCW_Water : 0.0;
	SetColWidth( pclSDAirVentSeparator, ColumnDescription::Water, dWidth );
	
	dWidth = ( true == m_pTADSPageSetup->GetField( PageField_enum::epfSEPARATORAIRVENTINFOPRODUCT ) ) ? DefaultColumnWidth::DCW_Product : 0.0;
	SetColWidth( pclSDAirVentSeparator, ColumnDescription::Product, dWidth );
	
	dWidth = ( true == m_pTADSPageSetup->GetField( PageField_enum::epfSEPARATORAIRVENTTECHINFO ) ) ? DefaultColumnWidth::DCW_TechnicalInfos : 0.0;
	SetColWidth( pclSDAirVentSeparator, ColumnDescription::TechnicalInfos, dWidth  );
	
	SetColWidth( pclSDAirVentSeparator, ColumnDescription::RadiatorInfos, 0.0 );
	
	dWidth = ( true == m_pTADSPageSetup->GetField( PageField_enum::epfARTICLE ) ) ? DefaultColumnWidth::DCW_ArticleNumber : 0.0;
	SetColWidth( pclSDAirVentSeparator, ColumnDescription::ArticleNumber, dWidth );
	
	dWidth = ( true == m_pTADSPageSetup->GetField( PageField_enum::epfPIPEINFO ) ) ? DefaultColumnWidth::DCW_Pipes : 0.0;
	SetColWidth( pclSDAirVentSeparator, ColumnDescription::Pipes, dWidth );
	
	dWidth = ( true == m_pTADSPageSetup->GetField( PageField_enum::epfQUANTITY ) ) ? DefaultColumnWidth::DCW_Quantity : 0.0;
	SetColWidth( pclSDAirVentSeparator, ColumnDescription::Quantity, dWidth );
	
	dWidth = 0.0;
	
	if( true == TASApp.IsPriceUsed() )
	{
		dWidth = ( true == m_pTADSPageSetup->GetField( PageField_enum::epfSALEUNITPRICE ) ) ? DefaultColumnWidth::DCW_UnitPrice : 0.0;
	}
	
	SetColWidth( pclSDAirVentSeparator, ColumnDescription::UnitPrice, dWidth );
	
	dWidth = 0.0;

	if( true == TASApp.IsPriceUsed() )
	{
		dWidth = ( true == m_pTADSPageSetup->GetField( PageField_enum::epfSALETOTALPRICE ) ) ? DefaultColumnWidth::DCW_TotalPrice : 0.0;
	}
	
	SetColWidth( pclSDAirVentSeparator, ColumnDescription::TotalPrice, dWidth );
	
	dWidth = ( true == m_pTADSPageSetup->GetField( PageField_enum::epfREMARK ) ) ? DefaultColumnWidth::DCW_Remark : 0.0;
	SetColWidth( pclSDAirVentSeparator, ColumnDescription::Remark, dWidth );
	
	SetColWidth( pclSDAirVentSeparator, ColumnDescription::Pointer, DefaultColumnWidth::DCW_Pointer );
	
	pclSheet->ShowCol( ColumnDescription::RadiatorInfos, FALSE );
	
	if( false == TASApp.IsPriceUsed() )
	{
		pclSheet->ShowCol( ColumnDescription::UnitPrice, FALSE );
		pclSheet->ShowCol( ColumnDescription::TotalPrice, FALSE );
	}

	pclSheet->ShowCol( ColumnDescription::Pointer, FALSE );
	CRect rect = pclSheet->GetSheetSizeInPixels( false );
	m_mapSheetInfos[pclSDAirVentSeparator].m_dPageWidth = rect.Width();

	// Page title.
	SetPageTitle( pclSDAirVentSeparator, IDS_RVIEWSELP_TITLE_AIRVENTSEPARATOR );
	
	// Init column header.
	_InitColHeader( pclSDAirVentSeparator );
	pclSheet->SetColumnAlwaysHidden( ColumnDescription::RadiatorInfos, true );
	
	// HYS-1571: We can optimize the column width now.
	// See Jira card to have more details about why we do that here.
	CRect rectClient;
	GetClientRect( &rectClient );
	OptimizePageSize( rectClient.Width(), rectClient.Height() );
	
	// Current position.
	long lRow = pclSheet->GetMaxRows() + 1;
	long lSelectedRow = 0;
	
	if( true == bAirVentSeparatorExist )
	{
		// Number of objects.
		int iAirVentSepCount = m_pclSelectionTable->GetItemCount( CLASS( CDS_SSelAirVentSeparator ) );

		if( 0 == iAirVentSepCount )
		{	
			return false;
		}
		
		CDS_SSelAirVentSeparator **paSSelAirVentSeparator = new CDS_SSelAirVentSeparator*[iAirVentSepCount];

		if( NULL == paSSelAirVentSeparator )
		{
			return false;
		}
		
		// Fill the 'paSSelAirVentSeparator' array with pointer on each 'CDS_SSelAirVentSeparator' object of the 'AIRVENTSEP_TAB' table.
		int iMaxIndex = 0;
		int i = 0;

		for( IDPTR IDPtr = m_pclSelectionTable->GetFirst( CLASS( CDS_SSelAirVentSeparator ) ); _T('\0') != *IDPtr.ID; IDPtr = m_pclSelectionTable->GetNext() )
		{
			// Sanity tests.
			CDS_SSelAirVentSeparator *pSel = dynamic_cast<CDS_SSelAirVentSeparator *>( IDPtr.MP );

			if( NULL == pSel )
			{
				continue;
			}
			
			if( NULL == pSel->GetIDPtr().MP )
			{
				continue;
			}
			
			paSSelAirVentSeparator[i] = (CDS_SSelAirVentSeparator *)(void *)IDPtr.MP;

			if( iMaxIndex < paSSelAirVentSeparator[i]->GetpSelectedInfos()->GetRowIndex() )
			{
				iMaxIndex = paSSelAirVentSeparator[i]->GetpSelectedInfos()->GetRowIndex();
			}

			++i;
		}
		
		ASSERT( i == iAirVentSepCount );
		iAirVentSepCount = i;
		
		// Sort 'paSSelAirVentSeparator' array.
		// Remark: 'fResetOrder' is set to 'true' only when the sorting combos are empty (No user choice) and the user clicks on the 'Apply sorting keys'.
		if( true == bResetOrder ) 
		{
			// Reset row index.
			for( i = 0; i < iAirVentSepCount; i++ )
			{
				if( 0 == paSSelAirVentSeparator[i]->GetpSelectedInfos()->GetRowIndex() )
				{
					paSSelAirVentSeparator[i]->GetpSelectedInfos()->SetRowIndex( ++iMaxIndex );
				}
			}
		}
		else if( PageField_enum::epfNONE != m_pTADSPageSetup->GetKey( 0 ) || PageField_enum::epfNONE != m_pTADSPageSetup->GetKey( 1 )
				|| PageField_enum::epfNONE != m_pTADSPageSetup->GetKey( 2 ) )
		{
			if( NULL != pDlgLeftTabSelP )
			{
				pDlgLeftTabSelP->SortTable( (CData**)paSSelAirVentSeparator, iAirVentSepCount - 1 );
			}

			// Reset row index.
			for( i = 0; i < iAirVentSepCount; i++ )
			{
				paSSelAirVentSeparator[i]->GetpSelectedInfos()->SetRowIndex( i + 1 );
			}
		}

		// Create a remark index.
		if( NULL != pDlgLeftTabSelP )
		{
			pDlgLeftTabSelP->SetRemarkIndex( CLASS( CDS_SSelAirVentSeparator ), (CData**)paSSelAirVentSeparator, iAirVentSepCount );
		}

		CRank rkl;
		enum eAirVSepSwitch
		{
			SingleSelSeparator,
			DirSelSeparator,
			DirSelAirVent,
			LastAirVSepSwitch
		};

		bool bAtLeastOneBlockPrinted = false;

		for( int iLoopGroup = 0; iLoopGroup < LastAirVSepSwitch; iLoopGroup++ )
		{
			int iCount = 0;

			for( i = 0; i < iAirVentSepCount; i++ )
			{
				bool bShouldbeAdded = false;
				CDS_SSelAirVentSeparator *pclSelAirVentSep = dynamic_cast<CDS_SSelAirVentSeparator *>( paSSelAirVentSeparator[i] );

				if( NULL == pclSelAirVentSep || NULL == pclSelAirVentSep->GetProductIDPtr().MP )
				{
					ASSERT( 0 );
					continue;
				}

				if( DirSelAirVent == iLoopGroup || DirSelSeparator == iLoopGroup )
				{
					if( false == pclSelAirVentSep->IsFromDirSel() )
					{
						continue;
					}
				}
				else
				{
					if( true == pclSelAirVentSep->IsFromDirSel() )
					{
						continue;
					}
				}

				// All air vents & separator products are based on a 'CDB_TAProduct' object.
				CDB_TAProduct* pclTAProduct = dynamic_cast<CDB_TAProduct *>( pclSelAirVentSep->GetProductIDPtr().MP );
				if( NULL == pclTAProduct )
				{
					ASSERT( 0 );
					continue;
				}

				switch( iLoopGroup )
				{
					case DirSelAirVent:

						if( NULL != dynamic_cast<CDB_AirVent*>( pclTAProduct ) )
						{
							bShouldbeAdded = true;
						}

						break;

					case SingleSelSeparator:
					case DirSelSeparator:

						if( NULL != dynamic_cast<CDB_Separator*>( pclTAProduct ) )
						{
							bShouldbeAdded = true;
						}

						break;
				}

				if( false == bShouldbeAdded )
				{
					continue;
				}
				
				rkl.Add( _T(""), paSSelAirVentSeparator[i]->GetpSelectedInfos()->GetRowIndex(), (LPARAM)paSSelAirVentSeparator[i] );
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
			SetLastRow( pclSDAirVentSeparator, lRow );

			switch( iLoopGroup )
			{
				case DirSelAirVent:
					SetPageTitle( pclSDAirVentSeparator, IDS_SSHEETSELPROD_SUBTITLEAIRVENTFROMDIRSEL, false, lRow );
					break;

				case SingleSelSeparator:
					SetPageTitle( pclSDAirVentSeparator, IDS_SSHEETSELPROD_SUBTITLESEPARATORFROMSSEL, false, lRow );
					break;

				case DirSelSeparator:
					SetPageTitle( pclSDAirVentSeparator, IDS_SSHEETSELPROD_SUBTITLESEPARATORFROMDIRSEL, false, lRow );
					break;
			}

			lRow = pclSheet->GetMaxRows();
			SetLastRow( pclSDAirVentSeparator, lRow );
			CString str;
			LPARAM lparam;
			bool bFirstPass = true;

			for( BOOL bContinue = rkl.GetFirst( str, lparam ); TRUE == bContinue; bContinue = rkl.GetNext( str, lparam ) )
			{
				CData *pclData = (CData *)lparam;
				CDS_SSelAirVentSeparator *pclSSelAirVentSep = dynamic_cast<CDS_SSelAirVentSeparator *>( pclData );

				if( NULL == pclSSelAirVentSep )
				{
					continue;
				}

				if( m_pSelected == pclData )
				{
					lSelectedRow = lRow;
				}

				CDB_AirVent *pclAirVent = pclSSelAirVentSep->GetProductAs<CDB_AirVent>();
				CDB_Separator *pclSeparator = pclSSelAirVentSep->GetProductAs<CDB_Separator>();

				if( NULL == pclAirVent && NULL == pclSeparator )
				{
					continue;
				}

				m_rProductParam.Clear();
				m_rProductParam.SetSheetDescription( pclSDAirVentSeparator );
				m_rProductParam.SetSelectionContainer( (LPARAM)pclSSelAirVentSep );

				++lRow;
				long lFirstRow = lRow;

				long lRowAirVentSep;
				CData *pclProductToSave = NULL;

				if( NULL != pclAirVent )
				{
					lRowAirVentSep = _FillRowAirVent( pclSDAirVentSeparator, lRow, pclSSelAirVentSep, pclAirVent );
					pclProductToSave = pclAirVent;
				}
				else
				{
					lRowAirVentSep = _FillRowSeparator( pclSDAirVentSeparator, lRow, pclSSelAirVentSep, pclSeparator );
					pclProductToSave = pclSeparator;
				}
				
				long lRowGen = CSelProdPageBase::FillRowGen( pclSDAirVentSeparator, lRow, pclSSelAirVentSep );
				lRow = max( lRowGen, lRowAirVentSep );

				// Add the selectable row range.
				m_rProductParam.AddRange( lFirstRow, lRow, pclProductToSave );

				// Spanning must be done here because it's the only place where we know exactly the number of lines to span!
				// Span reference #1 and #2.
				AddCellSpanW( pclSDAirVentSeparator, ColumnDescription::Reference1, lFirstRow, 1, pclSheet->GetMaxRows() - lFirstRow + 1 );
				AddCellSpanW( pclSDAirVentSeparator, ColumnDescription::Reference2, lFirstRow, 1, pclSheet->GetMaxRows() - lFirstRow + 1 );

				// Span water characteristic.
				AddCellSpanW( pclSDAirVentSeparator, ColumnDescription::Water, lFirstRow, 1, pclSheet->GetMaxRows() - lFirstRow + 1 );

				// Add accessories.
				if( true == pclSSelAirVentSep->IsAccessoryExist() )
				{
					// Draw dash line.
					pclSheet->SetCellBorder( ColumnDescription::Reference1, lRow, ColumnDescription::Footer - 1, lRow, true, SS_BORDERTYPE_BOTTOM, SS_BORDERSTYLE_FINE_DASH );

					lRow++;
					lRow = FillAccessories( pclSDAirVentSeparator, lRow, pclSSelAirVentSep, pclSSelAirVentSep->GetpSelectedInfos()->GetQuantity() );
				}

				// Save the object pointer.
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

		FillRemarks( pclSDAirVentSeparator, lRow );

		// We add by default a blank line.
		lRow = pclSheet->GetMaxRows() + 2;
		
		delete[] paSSelAirVentSeparator;
	}

	if( true == bAirVentSeparatorHMExist )
	{
		if( m_vecHMList.size() > 0 )	
		{
			SetLastRow( pclSDAirVentSeparator, lRow );
			SetPageTitle( pclSDAirVentSeparator, IDS_SSHEETSELPROD_SUBTITLESEPARATORFROMHM, false, lRow );	
			
			lRow++;
			SetLastRow( pclSDAirVentSeparator, lRow );
			
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
					m_rProductParam.SetSheetDescription( pclSDAirVentSeparator );
					// Intentionally set to NULL to avoid clicking, double clicking or right clicking.
					m_rProductParam.SetSelectionContainer( (LPARAM)0 );

					long lRowSeparator = _FillRowSeparator( pclSDAirVentSeparator, lRow, pHM );
					long lRowGen = CSelProdPageBase::FillRowGen( pclSDAirVentSeparator, lRow, pHM, pBase );
					long lLastRow = max( lRowGen, lRowSeparator );

					CDB_TAProduct *pclTAProd = dynamic_cast<CDB_TAProduct*>(pHM->GetIDPtr().MP);
					lLastRow = FillAndAddBuiltInHMAccessories( pclSheet, pclSDAirVentSeparator, pclTAProd, lLastRow );

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

bool CSelProdPageAirVentSep::HasSomethingToDisplay( void )
{
	bool fAirVentSeparatorExist = ( NULL != m_pclSelectionTable ) ? true : false;
	bool fAirVentSeparatorHMExist = ( m_vecHMList.size() > 0 ) ? true : false;
	if( false == fAirVentSeparatorExist && false == fAirVentSeparatorHMExist )
		return false;

	return true;
}

void CSelProdPageAirVentSep::_InitColHeader( CSheetDescription* pclSheetDescription )
{
	if( NULL == pclSheetDescription || NULL == pclSheetDescription->GetSSheetPointer() )
		return;

	CSSheet* pclSheet = pclSheetDescription->GetSSheetPointer();
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

long CSelProdPageAirVentSep::_FillRowAirVent( CSheetDescription* pclSheetDescription, long lRow, CDS_SSelAirVentSeparator* pclSSelAirVentSep, CDB_AirVent* pclAirVent )
{
	if( NULL == pclSheetDescription || NULL == pclSheetDescription->GetSSheetPointer() || NULL == pclSSelAirVentSep || NULL == pclAirVent )
		return lRow;

	CSSheet* pclSheet = pclSheetDescription->GetSSheetPointer();
	CString str1, str2;
	long lFirstRow = lRow;

	pclSheet->SetTextPattern( CSSheet::_SSTextPattern::UserStaticColumn );
	pclSheet->SetTextPatternProperty( CSSheet::_SSTextPatternProperty::TPP_FontBold, (LPARAM)FALSE );

	// Add item into the full article list.
	_AddArticleList( pclSSelAirVentSep );
		
	// Column TA Product.
	_FillRowAirVentProduct( pclSheetDescription, lRow, pclAirVent, pclSSelAirVentSep->GetpSelectedInfos()->GetQuantity() );

	return pclSheet->GetMaxRows();
}

long CSelProdPageAirVentSep::_FillRowSeparator( CSheetDescription* pclSheetDescription, long lRow, CDS_SSelAirVentSeparator* pclSSelAirVentSep, CDB_Separator* pclSeparator )
{
	if( NULL == pclSheetDescription || NULL == pclSheetDescription->GetSSheetPointer() || NULL == pclSSelAirVentSep || NULL == pclSeparator )
		return lRow;

	CSSheet* pclSheet = pclSheetDescription->GetSSheetPointer();
	CString str1, str2;
	long lFirstRow = lRow;

	pclSheet->SetTextPattern( CSSheet::_SSTextPattern::UserStaticColumn );
	pclSheet->SetTextPatternProperty( CSSheet::_SSTextPatternProperty::TPP_FontBold, (LPARAM)FALSE );

	// Add item into the full article list.
	_AddArticleList( pclSSelAirVentSep );
		
	// Column TA Product.
	_FillRowSeparatorProduct( pclSheetDescription, lRow, pclSeparator, pclSSelAirVentSep->GetpSelectedInfos()->GetQuantity() );

	// Column Infos.
	if( false == pclSSelAirVentSep->IsFromDirSel() )
	{
		// HYS-38: Show power dt info when their radio button is checked
		CString strPowerDt = _T( "" );
		if( CDS_SelProd::efdPower ==  pclSSelAirVentSep->GetFlowDef() )
		{
			strPowerDt = WriteCUDouble( _U_TH_POWER, pclSSelAirVentSep->GetPower(), true );
			strPowerDt += _T(" / ");
			strPowerDt += WriteCUDouble( _U_DIFFTEMP, pclSSelAirVentSep->GetDT(), true );
		}
		_FillRowSeparatorInfo( pclSheetDescription, lFirstRow, pclSSelAirVentSep->GetQ(), pclSSelAirVentSep->GetDp(), strPowerDt );
	}

	return pclSheet->GetMaxRows();
}

long CSelProdPageAirVentSep::_FillRowSeparator( CSheetDescription* pclSheetDescription, long lRow, CDS_HydroMod* pclHM )
{
	if( NULL == pclSheetDescription || NULL == pclSheetDescription->GetSSheetPointer() || NULL == pclHM ) 
		return lRow;

	CSSheet* pclSheet = pclSheetDescription->GetSSheetPointer();
	CString str1, str2;
	long lFirstRow = lRow;

	// TO DO !!
	double dQ = 0.0;
	CDB_Separator* pclSeparator = NULL;

	if( NULL == pclSeparator )
		return pclSheet->GetMaxRows();
	
	pclSheet->SetTextPattern( CSSheet::_SSTextPattern::UserStaticColumn );
	pclSheet->SetTextPatternProperty( CSSheet::_SSTextPatternProperty::TPP_FontBold, (LPARAM)FALSE );

	// Add item into the full article list.
	_AddArticleList( pclSeparator, 1 );

	// Column TA Product.
	_FillRowSeparatorProduct( pclSheetDescription, lRow, pclSeparator, 1 );

	// Column Infos.
	_FillRowSeparatorInfo( pclSheetDescription, lFirstRow, dQ, pclHM->GetDp(), _T("") );
	
	return pclSheet->GetMaxRows();
}

long CSelProdPageAirVentSep::_FillRowAirVentProduct( CSheetDescription* pclSheetDescription, long lRow, CDB_AirVent* pclAirVent, int iQuantity )
{
	if( NULL == pclSheetDescription || NULL == pclAirVent )
		return lRow;

	CString str1, str2;
	long lFirstRow = lRow;
	
	// Column TA Product.
	AddStaticText( pclSheetDescription, ColumnDescription::Product, lRow, _T("-") );
	if( true == m_pTADSPageSetup->GetField( epfSEPARATORAIRVENTINFOPRODUCTNAME ) )	
		lRow = AddStaticText( pclSheetDescription, ColumnDescription::Product, lRow, pclAirVent->GetName() );

	if( true == m_pTADSPageSetup->GetField( epfSEPARATORAIRVENTINFOPRODUCTDPP ) )
	{
		str1 = TASApp.LoadLocalizedString( IDS_SELP_DPP );
		str1 += CString( _T(" = ") ) + WriteCUDouble( _U_DIFFPRESS, pclAirVent->GetDpp(), true );
		lRow = AddStaticText( pclSheetDescription, ColumnDescription::Product, lRow, str1 );
	}

	if( true == m_pTADSPageSetup->GetField( epfSEPARATORAIRVENTINFOPRODUCTCONNECTION ) )	
		lRow = AddStaticText( pclSheetDescription, ColumnDescription::Product, lRow, ( (CDB_StringID *)( pclAirVent->GetConnectIDPtr().MP ) )->GetString() );
	if( true == m_pTADSPageSetup->GetField( epfSEPARATORAIRVENTINFOPRODUCTVERSION ) )	
		lRow = AddStaticText( pclSheetDescription, ColumnDescription::Product, lRow, ( (CDB_StringID *)( pclAirVent->GetVersionIDPtr().MP ) )->GetString() );
	if( true == m_pTADSPageSetup->GetField( epfSEPARATORAIRVENTINFOPRODUCTPN ) )	
		lRow = FillPNTminTmax( pclSheetDescription, ColumnDescription::Product, lRow, pclAirVent );
	
	// Column 'Quantity'.
	FillQtyPriceCol( pclSheetDescription, lFirstRow, pclAirVent, iQuantity );
	
	// Column 'Article number'.
	AddStaticText( pclSheetDescription, ColumnDescription::ArticleNumber, lFirstRow, _T("-") );

	// Remark: 'true' to exclude any connection reference in the article number.
	FillArtNumberCol( pclSheetDescription, ColumnDescription::ArticleNumber, lFirstRow, pclAirVent, pclAirVent->GetArtNum( true ) );

	return lRow;
}

long CSelProdPageAirVentSep::_FillRowSeparatorProduct( CSheetDescription* pclSheetDescription, long lRow, CDB_Separator* pclSeparator, int iQuantity )
{
	if( NULL == pclSheetDescription || NULL == pclSeparator )
		return lRow;

	CString str1, str2;
	long lFirstRow = lRow;
	
	// Column TA Product.
	AddStaticText( pclSheetDescription, ColumnDescription::Product, lRow, _T("-") );
	if( true == m_pTADSPageSetup->GetField( epfSEPARATORAIRVENTINFOPRODUCTNAME ) )	
		lRow = AddStaticText( pclSheetDescription, ColumnDescription::Product, lRow, pclSeparator->GetName() );
	if( true == m_pTADSPageSetup->GetField( epfSEPARATORAIRVENTINFOPRODUCTCONNECTION ) )	
		lRow = AddStaticText( pclSheetDescription, ColumnDescription::Product, lRow, ( (CDB_StringID *)( pclSeparator->GetConnectIDPtr().MP ) )->GetString() );
	if( true == m_pTADSPageSetup->GetField( epfSEPARATORAIRVENTINFOPRODUCTVERSION ) )	
		lRow = AddStaticText( pclSheetDescription, ColumnDescription::Product, lRow, ( (CDB_StringID *)( pclSeparator->GetVersionIDPtr().MP ) )->GetString() );
	if( true == m_pTADSPageSetup->GetField( epfSEPARATORAIRVENTINFOPRODUCTPN ) )	
		lRow = FillPNTminTmax( pclSheetDescription, ColumnDescription::Product, lRow, pclSeparator );
	
	// Column 'Quantity'.
	FillQtyPriceCol( pclSheetDescription, lFirstRow, pclSeparator, iQuantity );
	
	// Column 'Article number'.
	AddStaticText( pclSheetDescription, ColumnDescription::ArticleNumber, lFirstRow, _T("-") );

	// Remark: 'true' to exclude any connection reference in the article number.
	FillArtNumberCol( pclSheetDescription, ColumnDescription::ArticleNumber, lFirstRow, pclSeparator, pclSeparator->GetArtNum( true ) );
	return lRow;
}

long CSelProdPageAirVentSep::_FillRowSeparatorInfo( CSheetDescription* pclSheetDescription, long lRow, double dQ, double dDp, CString strPowerDt )
{
	if( NULL == pclSheetDescription )
	{
		return lRow;
	}

	// Column Infos.
	AddStaticText( pclSheetDescription, ColumnDescription::TechnicalInfos, lRow, _T("-") );			// Default string

	if( true == m_pTADSPageSetup->GetField( epfSEPARATORAIRVENTTECHINFOFLOW ) )
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

	if( true == m_pTADSPageSetup->GetField( epfSEPARATORAIRVENTTECHINFODP ) && -1.0 != dDp )
	{
		if( dDp > 0.0 )
		{
			lRow = AddStaticText( pclSheetDescription, ColumnDescription::TechnicalInfos, lRow, WriteCUDouble( _U_DIFFPRESS, dDp, true ) );
		}
		else
		{
			ASSERT( 0 );
		}
	}

	return lRow;
}

void CSelProdPageAirVentSep::_AddArticleList( CDB_TAProduct* pclTAP, int iQuantity )
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

void CSelProdPageAirVentSep::_AddArticleList( CDS_SSelAirVentSeparator* pclSSelSeparator )
{
	if( NULL == pclSSelSeparator )
	{
		return;
	}

	CDB_TAProduct *pTAP = pclSSelSeparator->GetProductAs<CDB_TAProduct>();

	if( NULL == pTAP )
	{
		return;
	}

	int iQuantity = pclSSelSeparator->GetpSelectedInfos()->GetQuantity();
	CArticleGroup *pclArticleGroup = new CArticleGroup();
	ASSERT( NULL != pclArticleGroup );
	
	if( NULL == pclArticleGroup )
	{
		return;
	}

	CArticleContainer* pclArticleContainer = pclArticleGroup->AddArticle( pTAP, iQuantity );

	AddAccessoriesInArticleContainer( pclSSelSeparator->GetAccessoryList(), pclArticleContainer, iQuantity, false );

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
