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
#include "SelProdPageTCTapWaterControl.h"

CSelProdPageTCTapWaterControl::CSelProdPageTCTapWaterControl( CArticleGroupList *pclArticleGroupList ) 
	: CSelProdPageBase( CDB_PageSetup::enCheck::TAPWATERCONTROL, pclArticleGroupList )
{
	m_pSelected = NULL;
}

void CSelProdPageTCTapWaterControl::Reset( void )
{
	CSelProdPageBase::Reset();
}

bool CSelProdPageTCTapWaterControl::PreInit( HMvector &vecHMList )
{
	// Something for individual or direct selection ?
	CTable *pclTable = (CTable *)( TASApp.GetpTADS()->Get( _T("TAPWATERCTRL_TAB") ).MP );

	if ( NULL == pclTable )
	{
		ASSERT( 0 );
		return false;
	}

	m_pclSelectionTable = ( '\0' != *pclTable->GetFirst().ID ) ? pclTable : NULL;

	if( NULL == m_pclSelectionTable )
	{
		return false;
	}

	m_vecHMList = vecHMList;
	SortTable();
	return true;
}

bool CSelProdPageTCTapWaterControl::Init( bool bResetOrder, bool bPrint )
{
	bool bTapWaterControlExist = ( NULL != m_pclSelectionTable ) ? true : false;

	if( false == bTapWaterControlExist )
	{
		return false;
	}

	// Call base class method to get a new 'Sheet'.
	CSheetDescription *pclSDTapWaterControl = CMultiSpreadBase::CreateSSheet( SD_TapWaterControl );

	if( NULL == pclSDTapWaterControl || NULL == pclSDTapWaterControl->GetSSheetPointer() )
	{
		return false;
	}
	
	CSSheet *pclSheet = pclSDTapWaterControl->GetSSheetPointer();

	if( false == CSelProdPageBase::PrepareSheet( pclSDTapWaterControl, ColumnDescription::Pointer, bPrint ) )
	{
		m_ViewDescription.RemoveOneSheetDescription( SD_TapWaterControl );
		return false;
	}

	// Initialize column width for header and footer column.
	if( false == m_bForPrint )
	{
		SetColWidth( pclSDTapWaterControl, ColumnDescription::Header, DefaultColumnWidth::DCW_Header );
		SetColWidth( pclSDTapWaterControl, ColumnDescription::Footer, DefaultColumnWidth::DCW_Footer );
	}
	else
	{
		pclSheet->ShowCol( ColumnDescription::Header, FALSE );
		pclSheet->ShowCol( ColumnDescription::Footer, FALSE );
	}

	double dWidth = ( true == m_pTADSPageSetup->GetField( PageField_enum::epfFIRSTREF ) ) ? DefaultColumnWidth::DCW_Reference1 : 0.0;
	SetColWidth( pclSDTapWaterControl, ColumnDescription::Reference1, dWidth );
	
	dWidth = ( true == m_pTADSPageSetup->GetField( PageField_enum::epfSECONDREF ) ) ? DefaultColumnWidth::DCW_Reference2 : 0.0;
	SetColWidth( pclSDTapWaterControl, ColumnDescription::Reference2, dWidth );
	
	dWidth = ( true == m_pTADSPageSetup->GetField( PageField_enum::epfWATERINFO ) ) ? DefaultColumnWidth::DCW_Water : 0.0;
	SetColWidth( pclSDTapWaterControl, ColumnDescription::Water, dWidth );
	
	dWidth = ( true == m_pTADSPageSetup->GetField( PageField_enum::epfTAPWATERCONTROLINFOPRODUCT ) ) ? DefaultColumnWidth::DCW_Product : 0.0;
	SetColWidth( pclSDTapWaterControl, ColumnDescription::Product, dWidth );
	
	SetColWidth( pclSDTapWaterControl, ColumnDescription::TechnicalInfos, 0.0  );
	
	SetColWidth( pclSDTapWaterControl, ColumnDescription::RadiatorInfos, 0.0 );
	
	dWidth = ( true == m_pTADSPageSetup->GetField( PageField_enum::epfARTICLE ) ) ? DefaultColumnWidth::DCW_ArticleNumber : 0.0;
	SetColWidth( pclSDTapWaterControl, ColumnDescription::ArticleNumber, dWidth );
	
	dWidth = ( true == m_pTADSPageSetup->GetField( PageField_enum::epfPIPEINFO ) ) ? DefaultColumnWidth::DCW_Pipes : 0.0;
	SetColWidth( pclSDTapWaterControl, ColumnDescription::Pipes, dWidth );
	
	dWidth = ( true == m_pTADSPageSetup->GetField( PageField_enum::epfQUANTITY ) ) ? DefaultColumnWidth::DCW_Quantity : 0.0;
	SetColWidth( pclSDTapWaterControl, ColumnDescription::Quantity, dWidth );
	
	dWidth = 0.0;
	
	if( true == TASApp.IsPriceUsed() )
	{
		dWidth = ( true == m_pTADSPageSetup->GetField( PageField_enum::epfSALEUNITPRICE ) ) ? DefaultColumnWidth::DCW_UnitPrice : 0.0;
	}
	
	SetColWidth( pclSDTapWaterControl, ColumnDescription::UnitPrice, dWidth );
	
	dWidth = 0.0;

	if( true == TASApp.IsPriceUsed() )
	{
		dWidth = ( true == m_pTADSPageSetup->GetField( PageField_enum::epfSALETOTALPRICE ) ) ? DefaultColumnWidth::DCW_TotalPrice : 0.0;
	}
	
	SetColWidth( pclSDTapWaterControl, ColumnDescription::TotalPrice, dWidth );
	
	dWidth = ( true == m_pTADSPageSetup->GetField( PageField_enum::epfREMARK ) ) ? DefaultColumnWidth::DCW_Remark : 0.0;
	SetColWidth( pclSDTapWaterControl, ColumnDescription::Remark, dWidth );
	
	SetColWidth( pclSDTapWaterControl, ColumnDescription::Pointer, DefaultColumnWidth::DCW_Pointer );
	
	pclSheet->ShowCol( ColumnDescription::RadiatorInfos, FALSE );
	
	if( false == TASApp.IsPriceUsed() )
	{
		pclSheet->ShowCol( ColumnDescription::UnitPrice, FALSE );
		pclSheet->ShowCol( ColumnDescription::TotalPrice, FALSE );
	}

	pclSheet->ShowCol( ColumnDescription::Pointer, FALSE );
	CRect rect = pclSheet->GetSheetSizeInPixels( false );
	m_mapSheetInfos[pclSDTapWaterControl].m_dPageWidth = rect.Width();

	// Page title.
	SetPageTitle( pclSDTapWaterControl, IDS_RVIEWSELP_TITLE_TAPWATERCONTROL );
	
	// Init column header.
	_InitColHeader( pclSDTapWaterControl );
	pclSheet->SetColumnAlwaysHidden( ColumnDescription::RadiatorInfos, true );
	
	// HYS-1571: We can optimize the column width now.
	// See Jira card to have more details about why we do that here.
	CRect rectClient;
	GetClientRect( &rectClient );
	OptimizePageSize( rectClient.Width(), rectClient.Height() );
	
	// Current position.
	long lRow = pclSheet->GetMaxRows() + 1;
	long lSelectedRow = 0;
	
	if( true == bTapWaterControlExist )
	{
		// Number of objects.
		int iTapWaterControlCount = m_pclSelectionTable->GetItemCount( CLASS( CDS_SSelTapWaterControl ) );

		if( 0 == iTapWaterControlCount )
		{
			return false;
		}
		
		CDS_SSelTapWaterControl **paSSelTapWaterControl = new CDS_SSelTapWaterControl * [iTapWaterControlCount];

		if( NULL == paSSelTapWaterControl )
		{
			return false;
		}

		memset( paSSelTapWaterControl, NULL, sizeof(CDS_SSelTapWaterControl *) * iTapWaterControlCount );
		
		// Fill the 'paSSelTapWaterControl' array with pointer on each 'CDS_SSelTapWaterControl' object of the 'TAPWATERCTRL_TAB' table.
		int iMaxIndex = 0;
		int i = 0;

		for( IDPTR IDPtr = m_pclSelectionTable->GetFirst( CLASS( CDS_SSelTapWaterControl ) ); '\0' != *IDPtr.ID; IDPtr = m_pclSelectionTable->GetNext() )
		{
			// Sanity tests.
			CDS_SSelTapWaterControl *pSel = dynamic_cast<CDS_SSelTapWaterControl *>( IDPtr.MP );

			if( NULL == pSel )
			{
				continue;
			}
			
			if( NULL == pSel->GetIDPtr().MP )
			{
				continue;
			}
			
			paSSelTapWaterControl[i] = (CDS_SSelTapWaterControl *)( IDPtr.MP );

			if( iMaxIndex < paSSelTapWaterControl[i]->GetpSelectedInfos()->GetRowIndex() )
			{
				iMaxIndex = paSSelTapWaterControl[i]->GetpSelectedInfos()->GetRowIndex();
			}

			++i;
		}
		
		ASSERT( i == iTapWaterControlCount );
		iTapWaterControlCount = i;
		
		// Sort 'paSSelTapWaterControl' array.
		// Remark: 'bResetOrder' is set to 'true' only when the sorting combos are empty (No user choice) and the user clicks on the 'Apply sorting keys'.
		if( true == bResetOrder ) 
		{
			// Reset row index.
			for( i = 0; i < iTapWaterControlCount; i++ )
			{
				if( 0 == paSSelTapWaterControl[i]->GetpSelectedInfos()->GetRowIndex() )
				{
					paSSelTapWaterControl[i]->GetpSelectedInfos()->SetRowIndex( ++iMaxIndex );
				}
			}
		}
		else if( PageField_enum::epfNONE != m_pTADSPageSetup->GetKey( 0 ) || PageField_enum::epfNONE != m_pTADSPageSetup->GetKey( 1 )
				|| PageField_enum::epfNONE != m_pTADSPageSetup->GetKey( 2 ) )
		{
			if( NULL != pDlgLeftTabSelP )
			{
				pDlgLeftTabSelP->SortTable( (CData **)paSSelTapWaterControl, iTapWaterControlCount - 1 );
			}

			// Reset row index.
			for( i = 0; i < iTapWaterControlCount; i++ )
			{
				paSSelTapWaterControl[i]->GetpSelectedInfos()->SetRowIndex( i + 1 );
			}
		}

		// Create a remark index.
		if( NULL != pDlgLeftTabSelP )
		{
			pDlgLeftTabSelP->SetRemarkIndex( CLASS( CDS_SSelTapWaterControl ), (CData **)paSSelTapWaterControl, iTapWaterControlCount );
		}

		// Remark: For tap water control, there is only direct selection.
		CRank rkl;
		bool bAtLeastOneBlockPrinted = false;

		for( i = 0; i < iTapWaterControlCount; i++ )
		{
			bool bShouldbeAdded = false;
			CDS_SSelTapWaterControl *pclSelTapWaterControl = dynamic_cast<CDS_SSelTapWaterControl *>( paSSelTapWaterControl[i] );
				
			if( NULL == pclSelTapWaterControl || NULL == dynamic_cast<CDB_TapWaterControl *>( pclSelTapWaterControl->GetProductIDPtr().MP ) )
			{
				ASSERT_CONTINUE;
			}

			rkl.Add( _T(""), paSSelTapWaterControl[i]->GetpSelectedInfos()->GetRowIndex(), (LPARAM)paSSelTapWaterControl[i] );
		}

		// Set the subtitle.
		SetLastRow( pclSDTapWaterControl, lRow );
		SetPageTitle( pclSDTapWaterControl, IDS_SSHEETSELPROD_SUBTITLETAPWATERCONTROLFROMDIRSEL, false, lRow );

		lRow = pclSheet->GetMaxRows();
		SetLastRow( pclSDTapWaterControl, lRow );

		CString str;
		LPARAM lparam;
		bool bFirstPass = true;

		for( BOOL bContinue = rkl.GetFirst( str, lparam ); TRUE == bContinue; bContinue = rkl.GetNext( str, lparam ) )
		{
			CData *pclData = (CData *)lparam;
			CDS_SSelTapWaterControl *pclSSelTapWaterControl = dynamic_cast<CDS_SSelTapWaterControl *>( pclData );

			if( NULL == pclSSelTapWaterControl )
			{
				continue;
			}

			if( m_pSelected == pclData )
			{
				lSelectedRow = lRow;
			}

			CDB_TapWaterControl *pclTapWaterControl = pclSSelTapWaterControl->GetProductAs<CDB_TapWaterControl>();

			if( NULL == pclTapWaterControl )
			{
				continue;
			}

			m_rProductParam.Clear();
			m_rProductParam.SetSheetDescription( pclSDTapWaterControl );
			m_rProductParam.SetSelectionContainer( (LPARAM)pclSSelTapWaterControl );

			++lRow;
			long lFirstRow = lRow;

			long lRowProduct = _FillRowTapWaterControl( pclSDTapWaterControl, lRow, pclSSelTapWaterControl, pclTapWaterControl );
			long lRowGen = CSelProdPageBase::FillRowGen( pclSDTapWaterControl, lRow, pclSSelTapWaterControl );
			lRow = max( lRowGen, lRowProduct );
				
			// Add the selectable row range.
			m_rProductParam.AddRange( lFirstRow, lRow, pclTapWaterControl );

			// Spanning must be done here because it's the only place where we know exactly the number of lines to span!
			// Span reference #1 and #2.
			AddCellSpanW( pclSDTapWaterControl, ColumnDescription::Reference1, lFirstRow, 1, pclSheet->GetMaxRows() - lFirstRow + 1 );
			AddCellSpanW( pclSDTapWaterControl, ColumnDescription::Reference2, lFirstRow, 1, pclSheet->GetMaxRows() - lFirstRow + 1 );

			// Span water characteristic.
			AddCellSpanW( pclSDTapWaterControl, ColumnDescription::Water, lFirstRow, 1, pclSheet->GetMaxRows() - lFirstRow + 1 );

			// Add accessories.
			if( NULL != pclTapWaterControl )
			{
				if( true == pclSSelTapWaterControl->IsAccessoryExist() )
				{
					// Draw dash line.
					pclSheet->SetCellBorder( ColumnDescription::Reference1, lRow, ColumnDescription::Footer - 1, lRow, true, SS_BORDERTYPE_BOTTOM, SS_BORDERSTYLE_FINE_DASH );

					lRow++;
					lRow = FillAccessories( pclSDTapWaterControl, lRow, pclSSelTapWaterControl, pclSSelTapWaterControl->GetpSelectedInfos()->GetQuantity() );
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
		FillRemarks( pclSDTapWaterControl, lRow );
		
		delete[] paSSelTapWaterControl;
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

bool CSelProdPageTCTapWaterControl::HasSomethingToDisplay( void )
{
	return ( NULL != m_pclSelectionTable ) ? true : false;
}

void CSelProdPageTCTapWaterControl::_InitColHeader( CSheetDescription *pclSheetDescription )
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

long CSelProdPageTCTapWaterControl::_FillRowTapWaterControl( CSheetDescription *pclSheetDescription, long lRow, CDS_SSelTapWaterControl *pclSSelTapWaterControl, CDB_TapWaterControl *pclTapWaterControl )
{
	if( NULL == pclSheetDescription || NULL == pclSheetDescription->GetSSheetPointer() || NULL == pclSSelTapWaterControl || NULL == pclTapWaterControl )
	{
		return lRow;
	}

	CSSheet *pclSheet = pclSheetDescription->GetSSheetPointer();
	CString str1, str2;
	long lFirstRow = lRow;

	pclSheet->SetTextPattern( CSSheet::_SSTextPattern::UserStaticColumn );
	pclSheet->SetTextPatternProperty( CSSheet::_SSTextPatternProperty::TPP_FontBold, (LPARAM)FALSE );

	// Add item into the full article list.
	_AddArticleList( pclSSelTapWaterControl );
		
	// Column TA Product.
	_FillRowTapWaterControlProduct( pclSheetDescription, lRow, pclTapWaterControl, pclSSelTapWaterControl->GetpSelectedInfos()->GetQuantity() );

	return pclSheet->GetMaxRows();
}

long CSelProdPageTCTapWaterControl::_FillRowTapWaterControlProduct( CSheetDescription *pclSheetDescription, long lRow, CDB_TapWaterControl *pclTapWaterControl, int iQuantity )
{
	if( NULL == pclSheetDescription || NULL == pclTapWaterControl || NULL == TASApp.GetpTADS() || NULL == TASApp.GetpTADS()->GetpTechParams() )
	{
		return lRow;
	}

	CString str1, str2;
	long lFirstRow = lRow;
	
	// Column TA Product.
	AddStaticText( pclSheetDescription, ColumnDescription::Product, lRow, _T("-") );

	if( true == m_pTADSPageSetup->GetField( epfTAPWATERCONTROLINFOPRODUCTNAME ) )	
	{
		lRow = AddStaticText( pclSheetDescription, ColumnDescription::Product, lRow, pclTapWaterControl->GetName() );
	}
	
	if( true == m_pTADSPageSetup->GetField( epfTAPWATERCONTROLINFOPRODUCTSETTINGRANGE ) )	
	{
		lRow = AddStaticText( pclSheetDescription, ColumnDescription::Product, lRow, pclTapWaterControl->GetTempAdjustableRangeStr( true ) );
	}

	if( true == m_pTADSPageSetup->GetField( epfTAPWATERCONTROLINFOPRODUCTBDYMATERIAL ) )	
	{
		lRow = AddStaticText( pclSheetDescription, ColumnDescription::Product, lRow, pclTapWaterControl->GetBodyMaterial() );
	}
	
	if( true == m_pTADSPageSetup->GetField( epfTAPWATERCONTROLINFOPRODUCTCONNECTION ) )	
	{
		lRow = AddStaticText( pclSheetDescription, ColumnDescription::Product, lRow, pclTapWaterControl->GetConnect() );
	}

	if( true == m_pTADSPageSetup->GetField( epfTAPWATERCONTROLINFOPRODUCTVERSION ) && false == CString( pclTapWaterControl->GetVersion() ).IsEmpty() )
	{
		lRow = AddStaticText( pclSheetDescription, ColumnDescription::Product, lRow, pclTapWaterControl->GetVersion() );
	}

	// Column 'Quantity'.
	FillQtyPriceCol( pclSheetDescription, lFirstRow, pclTapWaterControl, iQuantity );
	
	// Column 'Article number'.
	AddStaticText( pclSheetDescription, ColumnDescription::ArticleNumber, lFirstRow, _T("-") );

	// Remark: 'true' to exclude any connection reference in the article number.
	FillArtNumberCol( pclSheetDescription, ColumnDescription::ArticleNumber, lFirstRow, pclTapWaterControl, pclTapWaterControl->GetArtNum( true ) );
	return lRow;
}

void CSelProdPageTCTapWaterControl::_AddArticleList( CDB_TAProduct *pclTAP, int iQuantity )
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

void CSelProdPageTCTapWaterControl::_AddArticleList( CDS_SSelTapWaterControl *pclSSelTapWaterControl )
{
	if( NULL == pclSSelTapWaterControl )
	{
		return;
	}

	CDB_TAProduct *pTAP = pclSSelTapWaterControl->GetProductAs<CDB_TAProduct>();

	if( NULL == pTAP )
	{
		return;
	}

	int iQuantity = pclSSelTapWaterControl->GetpSelectedInfos()->GetQuantity();
	CArticleGroup *pclArticleGroup = new CArticleGroup();
	ASSERT( NULL != pclArticleGroup );
	
	if( NULL == pclArticleGroup )
	{
		return;
	}

	CArticleContainer *pclArticleContainer = pclArticleGroup->AddArticle( pTAP, iQuantity );

	AddAccessoriesInArticleContainer( pclSSelTapWaterControl->GetAccessoryList(), pclArticleContainer, iQuantity, false );

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
