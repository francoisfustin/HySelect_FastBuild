#include "stdafx.h"
#include "TASelect.h"
#include "MainFrm.h"
#include "Global.h"
#include "HydroMod.h"
#include "DlgLeftTabBase.h"
#include "DlgLeftTabSelP.h"
#include "RviewSSelSS.h"
#include "DlgSelectionBase.h"
#include "DlgIndSelBase.h"
#include "DlgIndSelTrv.h"
#include "RViewDescription.h"
#include "SelProdArticle.h"
#include "SelProdPageBase.h"
#include "SelProdPageTrv.h"

CSelProdPageTrv::CSelProdPageTrv( CArticleGroupList *pclArticleGroupList ) 
	: CSelProdPageBase( CDB_PageSetup::enCheck::TRV, pclArticleGroupList )
{
	m_pSelected = NULL;
}

void CSelProdPageTrv::Reset( void )
{
	CSelProdPageBase::Reset();
}

bool CSelProdPageTrv::PreInit( HMvector& vecHMList )
{
	// Something for individual or direct selection ?
	CTable* pclTable = (CTable*)( TASApp.GetpTADS()->Get( _T("RADSET_TAB") ).MP );
	if( NULL == pclTable )
	{
		ASSERT( 0 );
		return false;
	}

	m_pclSelectionTable = ( '\0' != *pclTable->GetFirst().ID ) ? pclTable : NULL;
	bool fTrvExist = ( NULL != m_pclSelectionTable );

	// Something for hydronic calculation?
	bool fTrvHMExist = ( vecHMList.size() > 0 );

	if( false == fTrvExist && false == fTrvHMExist )
		return false;
	
	m_vecHMList = vecHMList;
	SortTable();
	return true;
}

bool CSelProdPageTrv::Init( bool fResetOrder, bool fPrint )
{
	bool fTrvExist = ( NULL != m_pclSelectionTable ) ? true : false;
	bool fTrvHMExist = ( m_vecHMList.size() > 0 ) ? true : false;

	if( false == fTrvExist && false == fTrvHMExist )
	{
		return false;
	}

	// Call base class method to get a new 'Sheet'.
	CSheetDescription *pclSheetDescriptionTrv = CMultiSpreadBase::CreateSSheet( SD_Trv );

	if( NULL == pclSheetDescriptionTrv || NULL == pclSheetDescriptionTrv->GetSSheetPointer() )
	{
		return false;
	}
	
	CSSheet *pclSheet = pclSheetDescriptionTrv->GetSSheetPointer();

	if( false == CSelProdPageBase::PrepareSheet( pclSheetDescriptionTrv, ColumnDescription::Pointer, fPrint ) )
	{
		m_ViewDescription.RemoveOneSheetDescription( SD_Trv );
		return false;
	}

	// Initialize column width for header and footer column.
	if( false == m_bForPrint )
	{
		SetColWidth( pclSheetDescriptionTrv, ColumnDescription::Header, DefaultColumnWidth::DCW_Header );
		SetColWidth( pclSheetDescriptionTrv, ColumnDescription::Footer, DefaultColumnWidth::DCW_Footer );
	}
	else
	{
		pclSheet->ShowCol( ColumnDescription::Header, FALSE );
		pclSheet->ShowCol( ColumnDescription::Footer, FALSE );
	}	
	
	double dWidth = ( true == m_pTADSPageSetup->GetField( PageField_enum::epfFIRSTREF ) ) ? DefaultColumnWidth::DCW_Reference1 : 0.0;
	SetColWidth( pclSheetDescriptionTrv, ColumnDescription::Reference1, dWidth );

	dWidth = ( true == m_pTADSPageSetup->GetField( PageField_enum::epfSECONDREF ) ) ? DefaultColumnWidth::DCW_Reference2 : 0.0;
	SetColWidth( pclSheetDescriptionTrv, ColumnDescription::Reference2, dWidth );
	
	dWidth = ( true == m_pTADSPageSetup->GetField( PageField_enum::epfWATERINFO ) ) ? DefaultColumnWidth::DCW_Water : 0.0;
	SetColWidth( pclSheetDescriptionTrv, ColumnDescription::Water, dWidth );
	
	dWidth = ( true == m_pTADSPageSetup->GetField( PageField_enum::epfRD7INFOPRODUCT ) ) ? DefaultColumnWidth::DCW_Product : 0.0;
	SetColWidth( pclSheetDescriptionTrv, ColumnDescription::Product, dWidth );
	
	dWidth = ( true == m_pTADSPageSetup->GetField( PageField_enum::epfRD7TECHINFO ) ) ? DefaultColumnWidth::DCW_TechnicalInfos : 0.0;
	SetColWidth( pclSheetDescriptionTrv, ColumnDescription::TechnicalInfos, dWidth );
	
	dWidth = ( true == m_pTADSPageSetup->GetField( PageField_enum::epfRD7RADINFO ) ) ? DefaultColumnWidth::DCW_RadiatorInfos : 0.0;
	SetColWidth( pclSheetDescriptionTrv, ColumnDescription::RadiatorInfos, dWidth );

	dWidth = ( true == m_pTADSPageSetup->GetField( PageField_enum::epfARTICLE ) ) ? DefaultColumnWidth::DCW_ArticleNumber : 0.0;
	SetColWidth( pclSheetDescriptionTrv, ColumnDescription::ArticleNumber, dWidth );
	
	dWidth = ( true == m_pTADSPageSetup->GetField( PageField_enum::epfPIPEINFO ) ) ? DefaultColumnWidth::DCW_Pipes : 0.0;
	SetColWidth( pclSheetDescriptionTrv, ColumnDescription::Pipes, dWidth );
	
	dWidth = ( true == m_pTADSPageSetup->GetField( PageField_enum::epfQUANTITY ) ) ? DefaultColumnWidth::DCW_Quantity : 0.0;
	SetColWidth( pclSheetDescriptionTrv, ColumnDescription::Quantity, dWidth );
	
	dWidth = 0.0;
	
	if( true == TASApp.IsPriceUsed() )
	{
		dWidth = ( true == m_pTADSPageSetup->GetField( PageField_enum::epfSALEUNITPRICE ) ) ? DefaultColumnWidth::DCW_UnitPrice : 0.0;
	}

	SetColWidth( pclSheetDescriptionTrv, ColumnDescription::UnitPrice, dWidth );
	
	dWidth = 0.0;
	
	if( true == TASApp.IsPriceUsed() )
	{
		dWidth = ( true == m_pTADSPageSetup->GetField( PageField_enum::epfSALETOTALPRICE ) ) ? DefaultColumnWidth::DCW_TotalPrice : 0.0;
	}

	SetColWidth( pclSheetDescriptionTrv, ColumnDescription::TotalPrice, dWidth );
	
	dWidth = ( true == m_pTADSPageSetup->GetField( PageField_enum::epfREMARK ) ) ? DefaultColumnWidth::DCW_Remark : 0.0;
	SetColWidth( pclSheetDescriptionTrv, ColumnDescription::Remark, dWidth );

	SetColWidth( pclSheetDescriptionTrv, ColumnDescription::Pointer, DefaultColumnWidth::DCW_Pointer );

	if( false == TASApp.IsPriceUsed() )
	{
		pclSheet->ShowCol( ColumnDescription::UnitPrice, FALSE );
		pclSheet->ShowCol( ColumnDescription::TotalPrice, FALSE );
	}

	pclSheet->ShowCol( ColumnDescription::Pointer, FALSE );
	CRect rect = pclSheet->GetSheetSizeInPixels( false );
	m_mapSheetInfos[pclSheetDescriptionTrv].m_dPageWidth = rect.Width();

	// Page title.
	SetPageTitle( pclSheetDescriptionTrv, IDS_RVIEWSELP_TITLE_RADSET );

	// Init column header.
	_InitColHeader( pclSheetDescriptionTrv );

	// HYS-1571: We can optimize the column width now.
	// See Jira card to have more details about why we do that here.
	CRect rectClient;
	GetClientRect( &rectClient );
	OptimizePageSize( rectClient.Width(), rectClient.Height() );
	
	// Current position.
	long lRow = pclSheet->GetMaxRows() + 1;                                                                                   
	long lSelectedRow = 0;                                                                                                

	if( true == fTrvExist )
	{
		// Create an array of 'CDS_SSelRadSet' pointer.
		int iSSelTrvCount = m_pclSelectionTable->GetItemCount( CLASS( CDS_SSelRadSet ) );

		if( 0 == iSSelTrvCount )
		{
			return false;
		}
		
		CDS_SSelRadSet **parSelRadSet = new CDS_SSelRadSet*[iSSelTrvCount];

		if( NULL == parSelRadSet )
		{
			return false;
		}
		
		// Fill 'parSelRadSet' array with pointer on 'CDS_SSelRadSet' found in the 'RADSET_TAB' table.
		int iMaxIndex = 0;
		int i = 0;
		
		for( IDPTR IDPtr = m_pclSelectionTable->GetFirst( CLASS( CDS_SSelRadSet ) ); _T('\0') != *IDPtr.ID; IDPtr = m_pclSelectionTable->GetNext() )
		{
			CDS_SSelRadSet *pSelRadSet = dynamic_cast<CDS_SSelRadSet *>( IDPtr.MP );

			if( NULL == pSelRadSet )
			{
				continue;
			}
			
			// If there is no supply valve AND we are not in 'Other insert' mode...
			if( NULL == pSelRadSet->GetSupplyValveIDPtr().MP && false == pSelRadSet->IsInsertInKv() )
			{
				continue;
			}
			
			// If this valve belongs to a hydronic network...
			if( NULL != *pSelRadSet->GetHMID() )
			{
				ASSERT( 0 );
				continue;
			}
			
			// Add selection in array.
			parSelRadSet[i] = pSelRadSet;

			if( iMaxIndex < parSelRadSet[i]->GetpSelectedInfos()->GetRowIndex() )
			{
				iMaxIndex = parSelRadSet[i]->GetpSelectedInfos()->GetRowIndex();
			}

			++i;
		}

		// Verify if the number of items inserted in the array is the same as the initial number of 'CDS_SSelRadSet'.
		ASSERT( i == iSSelTrvCount );
		iSSelTrvCount = i;

		// If we must sort the 'parSelRadSet' array...
		// Remark: 'fResetOrder' is set to 'true' only when the sorting combos are empty (No user choice) and the user clicks on the 'Apply sorting keys'.
		if( true == fResetOrder )
		{	
			// Reset row index.
			for( i = 0; i < iSSelTrvCount; i++ )
			{
				if( 0 == parSelRadSet[i]->GetpSelectedInfos()->GetRowIndex() )
				{
					parSelRadSet[i]->GetpSelectedInfos()->SetRowIndex( ++iMaxIndex );
				}
			}
		}
		else if( PageField_enum::epfNONE != m_pTADSPageSetup->GetKey( 0 ) || PageField_enum::epfNONE != m_pTADSPageSetup->GetKey( 1 )
				|| PageField_enum::epfNONE != m_pTADSPageSetup->GetKey( 2 ) )
		{
			if( NULL != pDlgLeftTabSelP )
			{
				pDlgLeftTabSelP->SortTable( (CData**)parSelRadSet, iSSelTrvCount - 1 );
			}

			// Reset row index.
			for( i = 0; i < iSSelTrvCount; i++ )
			{
				parSelRadSet[i]->GetpSelectedInfos()->SetRowIndex( i + 1 );
			}
		}

		// Initialize remark and scheme index in TADS.
		if( NULL != pDlgLeftTabSelP )
		{
			pDlgLeftTabSelP->SetRemarkIndex( CLASS( CDS_SSelRadSet ), (CData**)parSelRadSet, iSSelTrvCount );		 
		}

		CRank rkl;
		enum eTrvSwitch
		{
			SingleSelTrv,
			DirSelTrv,
			LastTrvSwitch
		};
		bool bAtLeastOneBlockPrinted = false;

		for( int iLoopGroup = 0; iLoopGroup < LastTrvSwitch; iLoopGroup++ )
		{
			int iCount = 0;

			for( i = 0; i < iSSelTrvCount; i++ )
			{
				CDS_SSelRadSet *pclSSelTrv = dynamic_cast<CDS_SSelRadSet *>( parSelRadSet[i] );

				if( NULL == pclSSelTrv )
				{
					ASSERT( 0 );
					continue;
				}

				if( DirSelTrv == iLoopGroup )
				{
					if( false == pclSSelTrv->IsFromDirSel() )
					{
						continue;
					}
				}
				else
				{
					if( true == pclSSelTrv->IsFromDirSel() )
					{
						continue;
					}
				}

				rkl.Add( _T(""), parSelRadSet[i]->GetpSelectedInfos()->GetRowIndex(), (LPARAM)parSelRadSet[i] );
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
			SetLastRow( pclSheetDescriptionTrv, lRow );

			switch( iLoopGroup )
			{
				case DirSelTrv:
					SetPageTitle( pclSheetDescriptionTrv, IDS_SSHEETSELPROD_SUBTITLETRVFROMDIRSEL, false, lRow );
					break;

				case SingleSelTrv:
					SetPageTitle( pclSheetDescriptionTrv, IDS_SSHEETSELPROD_SUBTITLETRVFROMSSEL, false, lRow );
					break;
			}

			lRow = pclSheet->GetMaxRows();
			SetLastRow( pclSheetDescriptionTrv, lRow );

			CString str;
			LPARAM lparam;
			bool bFirstPass = true;

			for( BOOL bContinue = rkl.GetFirst( str, lparam ); TRUE == bContinue; bContinue = rkl.GetNext( str, lparam ) )
			{
				CData *pclData = (CData*)lparam;
				CDS_SSelRadSet *pclSSelRadSet = dynamic_cast<CDS_SSelRadSet *>( pclData );

				// We don't show the product if it doesn't exist in the DB AND it is not a 'Other insert' supply valve.
				if( NULL == pclSSelRadSet && false == pclSSelRadSet->IsInsertInKv() )
				{
					continue;
				}

				if( m_pSelected == pclData )
				{
					lSelectedRow = lRow;
				}

				m_rProductParam.Clear();
				m_rProductParam.SetSheetDescription( pclSheetDescriptionTrv );
				m_rProductParam.SetSelectionContainer( (LPARAM)pclSSelRadSet );

				++lRow;
				long lFirstRow = lRow;

				long lRowTrv;
				CData *pclProductToSave = NULL;
				
				if( false == pclSSelRadSet->IsInsertInKv() )
				{
					lRowTrv = _FillRowSupplyValve( pclSheetDescriptionTrv, lRow, pclSSelRadSet );
					pclProductToSave = dynamic_cast<CDB_TAProduct*>( pclSSelRadSet->GetSupplyValveIDPtr().MP );
				}
				else
				{
					lRowTrv = _FillRowSVInsert( pclSheetDescriptionTrv, lRow, pclSSelRadSet );
				}
					
				// Fill references 1 & 2, media characteristic and pipe information.
				long lRowGen = CSelProdPageBase::FillRowGen( pclSheetDescriptionTrv, lRow, (CDS_SelProd *)pclSSelRadSet );
				lRow = max( lRowGen, lRowTrv );

				// Add the selectable row range.
				m_rProductParam.AddRange( lFirstRow, lRow, pclProductToSave );

				// Spanning must be done here because it's the only place where we know exactly number of lines to span!
				// Span reference #1 and #2.
				AddCellSpanW( pclSheetDescriptionTrv, ColumnDescription::Reference1, lFirstRow, 1, pclSheet->GetMaxRows() - lFirstRow + 1 );
				AddCellSpanW( pclSheetDescriptionTrv, ColumnDescription::Reference2, lFirstRow, 1, pclSheet->GetMaxRows() - lFirstRow + 1 );

				// Span water characteristic.
				AddCellSpanW( pclSheetDescriptionTrv, ColumnDescription::Water, lFirstRow, 1, pclSheet->GetMaxRows() - lFirstRow + 1 );

				// Add accessories.
				if( true == pclSSelRadSet->IsSupplyValveAccessoryExist() )
				{
					// Draw dash line.
					pclSheet->SetCellBorder( ColumnDescription::Reference1, lRow, ColumnDescription::Footer - 1, lRow, true, SS_BORDERTYPE_BOTTOM, SS_BORDERSTYLE_FINE_DASH );
				
					lRow++;
					lRow = CSelProdPageBase::FillAccessories( pclSheetDescriptionTrv, lRow, pclSSelRadSet->GetSupplyValveAccessoryList(), pclSSelRadSet->GetpSelectedInfos()->GetQuantity() );
				}

				if( true == pclSSelRadSet->IsSupplyValveActuatorExist() )
				{
					// Draw dash line.
					pclSheet->SetCellBorder( ColumnDescription::Reference1, lRow, ColumnDescription::Footer - 1, lRow, true, SS_BORDERTYPE_BOTTOM, SS_BORDERSTYLE_FINE_DASH );

					lRow++;
					lRow = _FillSVActuatorFromSSel( pclSheetDescriptionTrv, lRow, pclSSelRadSet );
				}

				if( true == pclSSelRadSet->IsSVActuatorAccessoryExist() )
				{
					// Draw dash line.
					pclSheet->SetCellBorder( ColumnDescription::Reference1, lRow, ColumnDescription::Footer - 1, lRow, true, SS_BORDERTYPE_BOTTOM, SS_BORDERSTYLE_FINE_DASH );

					lRow++;
					lRow = CSelProdPageBase::FillAccessories( pclSheetDescriptionTrv, lRow, pclSSelRadSet->GetSVActuatorAccessoryList(), pclSSelRadSet->GetpSelectedInfos()->GetQuantity() );
				}

				if( true == pclSSelRadSet->IsReturnValveExist() )
				{
					// Draw dash line.
					pclSheet->SetCellBorder( ColumnDescription::Reference1, lRow, ColumnDescription::Footer - 1, lRow, true, SS_BORDERTYPE_BOTTOM, SS_BORDERSTYLE_FINE_DASH );
					
					lRow++;
					lRow = _FillRowReturnValve( pclSheetDescriptionTrv, lRow, pclSSelRadSet );
				}

				if( true == pclSSelRadSet->IsReturnValveAccessoryExist() )
				{
					// Draw dash line.
					pclSheet->SetCellBorder( ColumnDescription::Reference1, lRow, ColumnDescription::Footer - 1, lRow, true, SS_BORDERTYPE_BOTTOM, SS_BORDERSTYLE_FINE_DASH );

					lRow++;
					lRow = CSelProdPageBase::FillAccessories( pclSheetDescriptionTrv, lRow, pclSSelRadSet->GetReturnValveAccessoryList(), pclSSelRadSet->GetpSelectedInfos()->GetQuantity() );
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

		FillRemarks( pclSheetDescriptionTrv, lRow );

		// We add by default a blank line.
		lRow = pclSheet->GetMaxRows() + 2;

		delete[] parSelRadSet;
	}

	if( true == fTrvHMExist )
	{
		// Something inside ?
		if( m_vecHMList.size() > 0 )
		{
			// Create a remark index.
			if( NULL != pDlgLeftTabSelP )
			{
				pDlgLeftTabSelP->SetRemarkIndex( &m_vecHMList );
			}

			// Set the sub title.
			SetLastRow( pclSheetDescriptionTrv, lRow );                                                                                             
			SetPageTitle( pclSheetDescriptionTrv, IDS_SSHEETSELPROD_SUBTITLETRVFROMHM, false, lRow );
			
			lRow++;
			SetLastRow( pclSheetDescriptionTrv, lRow );                                                                                             
			
			// For each element.
			bool fFirstPass = true;
			HMvector::iterator It;
			
			for( It = m_vecHMList.begin(); It != m_vecHMList.end(); ++It )
			{
				CDS_HydroMod::CCv *pHMCV = static_cast<CDS_HydroMod::CCv *>( (*It).second );
				
				// CV exist and TA product exist into the DB.
				if( NULL != pHMCV && NULL != pHMCV->GetCvIDPtr().MP )
				{
					m_rProductParam.Clear();
					m_rProductParam.SetSheetDescription( pclSheetDescriptionTrv );
					// Intentionally set to NULL to avoid clicking, double clicking or right clicking.
					m_rProductParam.SetSelectionContainer( (LPARAM)0 );

					long lFirstRow = lRow;
					long lRowTrv = _FillRowSupplyValve( pclSheetDescriptionTrv, lFirstRow, pHMCV );
					long lRowGen = CSelProdPageBase::FillRowGen( pclSheetDescriptionTrv, lFirstRow, pHMCV->GetpParentHM() );
					lRow = max( lRowGen, lRowTrv );

					CDB_TAProduct *pclTAProd = dynamic_cast<CDB_TAProduct*>(pHMCV->GetCvIDPtr().MP);
					lRow = FillAndAddBuiltInHMAccessories( pclSheet, pclSheetDescriptionTrv, pclTAProd, lRow );

					// Spanning must be done here because it's the only place where we know exactly number of lines to span!
					// Span reference #1 and #2.
					AddCellSpanW( pclSheetDescriptionTrv, ColumnDescription::Reference1, lFirstRow, 1, pclSheet->GetMaxRows() - lFirstRow + 1 );
					AddCellSpanW( pclSheetDescriptionTrv, ColumnDescription::Reference2, lFirstRow, 1, pclSheet->GetMaxRows() - lFirstRow + 1 );
	
					// Span water characteristic.
					AddCellSpanW( pclSheetDescriptionTrv, ColumnDescription::Water, lFirstRow, 1, pclSheet->GetMaxRows() - lFirstRow + 1 );

					// Fill actuator if exist.
					if( NULL != dynamic_cast<CDB_Actuator *>( pHMCV->GetActrIDPtr().MP ) )
					{
						// Draw dash line.
						pclSheet->SetCellBorder( ColumnDescription::Reference1, lRow, ColumnDescription::Footer - 1, lRow, true, SS_BORDERTYPE_BOTTOM, SS_BORDERSTYLE_FINE_DASH );

						lRow++;
						lRow = _FillSVActuatorFromHM( pclSheetDescriptionTrv, lRow, pHMCV );
					}
					
					// Draw line below.
					pclSheet->SetCellBorder( ColumnDescription::Reference1, lRow, ColumnDescription::Footer - 1, lRow, true, SS_BORDERTYPE_BOTTOM );

					m_rProductParam.SetScrollRange( ( true == fFirstPass ) ? lFirstRow - 1 : lFirstRow, lRow );
					SaveProduct( m_rProductParam );

					// Set all group as no breakable (for print).
					// Remark: include title with the group.
					pclSheet->SetFlagRowNoBreakable( ( true == fFirstPass ) ? lFirstRow - 1 : lFirstRow, lRow, true );
					fFirstPass = false;
				}

				lRow++;
			}

			FillHMRemarks( &m_vecHMList, pclSheetDescriptionTrv, lRow );
		}
	}

	// Move sheet to correct position.
	SetSheetSize();
	Invalidate();
	UpdateWindow();

	// Verify if a product is selected.
	if( 0 == lSelectedRow )
		m_pSelected = 0;

	return true;
}

bool CSelProdPageTrv::HasSomethingToDisplay( void )
{
	bool fTrvExist = ( NULL != m_pclSelectionTable ) ? true : false;
	bool fTrvHMExist = ( m_vecHMList.size() > 0 ) ? true : false;
	if( false == fTrvExist && false == fTrvHMExist )
		return false;
	return true;
}

void CSelProdPageTrv::_InitColHeader( CSheetDescription* pclSheetDescription )
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

	// Freeze Row Header.
	pclSheet->SetFreeze( 0, SelProdHeaderRow::HR_RowHeader );
}

void CSelProdPageTrv::_FillRowTrvTAP( CSheetDescription* pclSheetDescription, long lRow, CDB_TAProduct* pTAP, int iQuantity )
{
	if( NULL == pclSheetDescription || NULL == pclSheetDescription->GetSSheetPointer() || NULL == pTAP )
	{
		return;
	}

	CString str1, str2;
	long lFirstRow = lRow;
	
	// Column TA Product.
	AddStaticText( pclSheetDescription, ColumnDescription::Product, lRow, _T("-") );			// Default string
	
	if( true == m_pTADSPageSetup->GetField( epfRD7INFOPRODUCTNAME ) )	
	{
		lRow = AddStaticText( pclSheetDescription, ColumnDescription::Product, lRow, pTAP->GetName() );
	}

	if( true == m_pTADSPageSetup->GetField( epfRD7INFOPRODUCTSIZE ) )	
	{
		lRow = AddStaticText( pclSheetDescription, ColumnDescription::Product, lRow, pTAP->GetSize() );
	}
	
	if( true == m_pTADSPageSetup->GetField( epfRD7INFOPRODUCTCONNECTION ) )	
	{
		lRow = AddStaticText( pclSheetDescription, ColumnDescription::Product, lRow, ( (CDB_StringID *)pTAP->GetConnectIDPtr().MP )->GetString() );
	}
	
	if( true == m_pTADSPageSetup->GetField( epfRD7INFOPRODUCTVERSION ) )	
	{
		lRow = AddStaticText( pclSheetDescription, ColumnDescription::Product, lRow, ( (CDB_StringID *)pTAP->GetVersionIDPtr().MP )->GetString() );
	}
	
	if( true == m_pTADSPageSetup->GetField( epfRD7INFOPRODUCTPN ) )	
	{
		lRow = FillPNTminTmax( pclSheetDescription, ColumnDescription::Product, lRow, pTAP );
	}
	
	// Column Quantity, Price.
	FillQtyPriceCol( pclSheetDescription, lFirstRow, pTAP, iQuantity );
	
	// Column Article number.
	lRow = lFirstRow;
	AddStaticText( pclSheetDescription, ColumnDescription::ArticleNumber, lRow, _T("-") );			// Default string
	
	// Remark: 'true' to exclude any connection reference in the article number.
	FillArtNumberCol( pclSheetDescription, ColumnDescription::ArticleNumber, lRow, pTAP, pTAP->GetArtNum( true ) );
}

long CSelProdPageTrv::_FillRowSupplyValve( CSheetDescription* pclSheetDescription, long lRow, CDS_HydroMod::CCv* pHMCv )
{
	if( NULL == pclSheetDescription || NULL == pclSheetDescription->GetSSheetPointer() || NULL == pHMCv )
		return lRow;

	CDB_TAProduct* pTAP = dynamic_cast<CDB_TAProduct*>( pHMCv->GetCvIDPtr().MP );
	if( NULL == pTAP )
		return lRow;

	CSSheet* pclSheet = pclSheetDescription->GetSSheetPointer();
	CString str1, str2;
	long lFirstRow = lRow;

	pclSheet->SetTextPattern( CSSheet::_SSTextPattern::UserStaticColumn );
	pclSheet->SetTextPatternProperty( CSSheet::_SSTextPatternProperty::TPP_FontBold, (LPARAM)FALSE );

	// Add item into the full Article List.
	_AddArticleList( pTAP, 1 );

	// Column TA Product.
	_FillRowTrvTAP( pclSheetDescription, lRow, pTAP, 1 );

	// Column Infos.
	lRow = lFirstRow;
	AddStaticText( pclSheetDescription, ColumnDescription::TechnicalInfos, lRow, _T("-") );			// Default string
	
	if( true == m_pTADSPageSetup->GetField( epfRD7TECHINFOFLOW ) )
		lRow = AddStaticText( pclSheetDescription, ColumnDescription::TechnicalInfos, lRow, WriteCUDouble( _U_FLOW, pHMCv->GetQ(), true ) );
	
	if( true == m_pTADSPageSetup->GetField( epfRD7TECHINFODP ) )
	{
		double dDp = pHMCv->GetDp();
		str2 = TASApp.LoadLocalizedString( IDS_DP );
		if( dDp <= 0 )
		{
			CString str3;
			str3 = TASApp.LoadLocalizedString( IDS_UNKNOWN );
			str3.MakeLower();
			str1 = str2 + (CString)_T(" ") + str3;
		}
		else
		{
			str1 = str2 + (CString)_T(" = ");
			str1 += WriteCUDouble( _U_DIFFPRESS, dDp, true );
		}
		lRow = AddStaticText( pclSheetDescription, ColumnDescription::TechnicalInfos, lRow, str1 );
	}

	if( true == m_pTADSPageSetup->GetField( epfRD7TECHINFOSETTING ) )
	{	
		double dSet = pHMCv->GetSetting();

		if( dSet != 0.0 )
		{
			CDB_ValveCharacteristic* pChar = ( (CDB_ThermostaticValve *)pTAP )->GetThermoCharacteristic();
			str2 = TASApp.LoadLocalizedString( IDS_SELP_TRVSETTING );
			str1 = str2 + (CString) _T(" = ");
			
			if( ( (CString)pTAP->GetFamilyID() ) == _T("FAM_TRV2") && dSet < 1.0 )
			{
				str1 += _T("<1**");
			}

			else if( NULL != pChar ) 
			{
				str1 += pChar->GetSettingString( dSet );
			}
			else
			{
				str1 += _T("-");
			}

			lRow = AddStaticText( pclSheetDescription, ColumnDescription::TechnicalInfos, lRow, str1 );
		}
	}

	return pclSheet->GetMaxRows();
}

long CSelProdPageTrv::_FillRowSupplyValve( CSheetDescription* pclSheetDescription, long lRow, CDS_SSelRadSet* pSelRadSet )
{
	if( NULL == pclSheetDescription || NULL == pclSheetDescription->GetSSheetPointer() || NULL == pSelRadSet )
		return lRow;

	CDB_TAProduct* pTAP = dynamic_cast<CDB_TAProduct*>( pSelRadSet->GetSupplyValveIDPtr().MP );
	if( NULL == pTAP )
		return lRow;

	CDB_ThermostaticValve *pThermostaticValve = dynamic_cast<CDB_ThermostaticValve*>( pTAP );
	CDB_FlowLimitedControlValve *pFlowLimitedControlValve = dynamic_cast<CDB_FlowLimitedControlValve*>( pTAP );
	if( NULL == pThermostaticValve && NULL == pFlowLimitedControlValve )
		return lRow;

	CSSheet* pclSheet = pclSheetDescription->GetSSheetPointer();
	CString str1, str2;
	long lFirstRow = lRow;

	pclSheet->SetTextPattern( CSSheet::_SSTextPattern::UserStaticColumn );
	pclSheet->SetTextPatternProperty( CSSheet::_SSTextPatternProperty::TPP_FontBold, (LPARAM)FALSE );

	// Add item into the full article list.
	// Remark: don't add if it's an insert in Kv mode.
	if( RVT_Inserts != (RadiatorValveType)pSelRadSet->GetRadiatorValveType() || 
			false == pSelRadSet->IsInsertInKv() )
	{
		_AddArticleList( pTAP, pSelRadSet->GetpSelectedInfos()->GetQuantity(), pSelRadSet->GetSupplyValveAccessoryList(), pSelRadSet->IsSelectedAsAPackage() );
	}

	// Column TA Product.
	_FillRowTrvTAP( pclSheetDescription, lRow, pTAP, pSelRadSet->GetpSelectedInfos()->GetQuantity() );

	if( true == pSelRadSet->IsFromDirSel() )
	{
		m_rProductParam.AddRange( lFirstRow, pclSheet->GetMaxRows(), pTAP );
		return lRow;
	}

	// Column infos.
	lRow = lFirstRow;
	AddStaticText( pclSheetDescription, ColumnDescription::TechnicalInfos, lRow, _T("-") );			// Default string
	
	// HYS-1305 : Add Power / DT field for trv selection
	double dQ = pSelRadSet->GetQ();
	// HYS-38: Show power dt info when their radio button is checked
	CString strPowerDt = _T( "" );
	if( CDS_SelProd::efdPower == pSelRadSet->GetFlowDef() )
	{
		strPowerDt = WriteCUDouble( _U_TH_POWER, pSelRadSet->GetPower(), true );
		strPowerDt += _T( " / " );
		strPowerDt += WriteCUDouble( _U_DIFFTEMP, pSelRadSet->GetDT(), true );
	}
	if( true == m_pTADSPageSetup->GetField( epfRD7TECHINFOFLOW ) )
	{
		if( dQ > 0.0 )
		{
			// HYS-38: Show power dt info when their radio button is checked
			if( strPowerDt != _T( "" ) )
			{
				CString strQ = _T( "(" );
				strQ += WriteCUDouble( _U_FLOW, dQ, true );
				strQ += _T( ")" );
				lRow = AddStaticText( pclSheetDescription, ColumnDescription::TechnicalInfos, lRow, strPowerDt );
				lRow = AddStaticText( pclSheetDescription, ColumnDescription::TechnicalInfos, lRow, strQ );
			}
			else
			{
				lRow = AddStaticText( pclSheetDescription, ColumnDescription::TechnicalInfos, lRow, WriteCUDouble( _U_FLOW, dQ, true ) );
			}
		}
		else
			ASSERT( 0 );
	}
	
	if( NULL != pThermostaticValve )
	{
		if( true == m_pTADSPageSetup->GetField( epfRD7TECHINFODP ) )
		{
			// Show Dp only if different from -1.
			if( pSelRadSet->GetSupplyValveDp() != -1 )
			{
				str2 = TASApp.LoadLocalizedString( IDS_DP );
				if( pSelRadSet->GetSupplyValveDp() <= 0 )
				{
					CString str3;
					str3 = TASApp.LoadLocalizedString( IDS_UNKNOWN );
					str3.MakeLower();
					str1 = str2 + (CString)_T(" ") + str3;
				}
				else
				{
					str1 = str2 + (CString)_T(" = ");
					str1 += WriteCUDouble( _U_DIFFPRESS, pSelRadSet->GetSupplyValveDp(), true );
				}
				lRow = AddStaticText( pclSheetDescription, ColumnDescription::TechnicalInfos, lRow, str1 );
			}
		}

		if( true == m_pTADSPageSetup->GetField( epfRD7TECHINFOSETTING ) )
		{	
			// HYS-1305 : Display setting for presettable insert
			if( ( 0 == _tcscmp( pThermostaticValve->GetTypeID(), _T("TRVTYPE_PRESET") ) )
				|| ( 0 == _tcscmp( pThermostaticValve->GetTypeID(), _T( "TRVTYPE_INSERT_PRESET" ) ) ) )
			{
				CDB_ValveCharacteristic* pChar = pThermostaticValve->GetThermoCharacteristic();
				str2 = TASApp.LoadLocalizedString( IDS_SELP_TRVSETTING );
				str1 = str2 + (CString)_T(" = ");
				
				if( NULL != pChar )
				{
					str1 += pChar->GetSettingString( pSelRadSet->GetSupplyValveOpening() );
				}
				else
				{
					str1 += _T("-");
				}

				lRow = AddStaticText( pclSheetDescription, ColumnDescription::TechnicalInfos, lRow, str1 );
			}
		}
	
		if( true == m_pTADSPageSetup->GetField( epfRD7TECHINFODPTOT ) )
		{
			// Show Dp only if different from -1.
			if( pSelRadSet->GetDp() != -1 )
			{
				str2 = TASApp.LoadLocalizedString( IDS_DPTOT );
				if( 0 == pSelRadSet->GetDp() )
				{
					CString str3;
					str3 = TASApp.LoadLocalizedString( IDS_UNKNOWN );
					str3.MakeLower();
					str1 = str2 + (CString)_T(" ") + str3;
				}
				else
				{
					str1 = str2 + (CString)_T(" = ");
					str1 += WriteCUDouble( _U_DIFFPRESS, pSelRadSet->GetDp(), true );
				}
				lRow = AddStaticText( pclSheetDescription, ColumnDescription::TechnicalInfos, lRow, str1 );
			}
		}
	}
	else if ( NULL != pFlowLimitedControlValve )
	{
		// We work with a 'CDB_FlowLimitedControlValve'.

		if( true == m_pTADSPageSetup->GetField( epfRD7TECHINFOSETTING ) )
		{	
			if( ( 0 == _tcscmp( pFlowLimitedControlValve->GetTypeID(), _T("TRVTYPE_FLOWLIMITED") ) )
				|| ( 0 == _tcscmp( pFlowLimitedControlValve->GetTypeID(), _T("TRVTYPE_INSERT_FL") ) ) )
			{
				CDB_FLCVCharacteristic* pChar = pFlowLimitedControlValve->GetFLCVCharacteristic();
				str2 = TASApp.LoadLocalizedString( IDS_SELP_TRVSETTING );
				str1 = str2 + (CString)_T(" = ");
			
				if( NULL != pChar )
				{
					str1 += pChar->GetSettingString( pSelRadSet->GetSupplyValveOpening() );
				}
				else
				{
					str1 += _T("-");
				}

				lRow = AddStaticText( pclSheetDescription, ColumnDescription::TechnicalInfos, lRow, str1 );
			}
		}
	
		if( true == m_pTADSPageSetup->GetField( epfRD7TECHINFODPMIN ) )
		{
			str1 = TASApp.LoadLocalizedString( IDS_SSHEETSELP_DPMIN );
			double dDpmin = -1.0;
			CDB_FLCVCharacteristic* pclFLCVCharacteristic = pFlowLimitedControlValve->GetFLCVCharacteristic();
			if( NULL != pclFLCVCharacteristic )
				dDpmin = pclFLCVCharacteristic->GetDpmin( pSelRadSet->GetSupplyValveOpening() );
			str1 += ( dDpmin != -1.0 ) ? WriteCUDouble( _U_DIFFPRESS, dDpmin, true ) : GetDashDotDash();
			lRow = AddStaticText( pclSheetDescription, ColumnDescription::TechnicalInfos, lRow, str1 );
		}

		if( true == m_pTADSPageSetup->GetField( epfRD7TECHINFOFLOWRANGE ) )
		{
			str1 = TASApp.LoadLocalizedString( IDS_SSHEETSELP_FLOWRANGE );
			double dQmin = -1.0;
			double dQmax = -1.0;
			CDB_FLCVCharacteristic* pclFLCVCharacteristic = pFlowLimitedControlValve->GetFLCVCharacteristic();
			if( NULL != pclFLCVCharacteristic )
			{
				dQmin = pclFLCVCharacteristic->GetQLFmin();
				dQmax = pclFLCVCharacteristic->GetQNFmax();
			}
			CString strQmin = ( dQmin != -1.0 ) ? WriteCUDouble( _U_FLOW, dQmin ) : GetDashDotDash();
			CString strQmax = ( dQmax != -1.0 ) ? WriteCUDouble( _U_FLOW, dQmax ) : GetDashDotDash();
			str1 += _T("[") + strQmin + _T(";") + strQmax + _T("]");
			
			TCHAR unitname[_MAXCHARS];
			CUnitDatabase *pUnitDB = CDimValue::AccessUDB();								ASSERT( NULL != pUnitDB );
			GetNameOf( pUnitDB->GetUnit( _U_FLOW, pUnitDB->GetDefaultUnitIndex( _U_FLOW ) ), unitname );
			str1 += _T(" ") + CString( unitname );
			lRow = AddStaticText( pclSheetDescription, ColumnDescription::TechnicalInfos, lRow, str1 );
		}
	}
	
	// Retrieve infos concerning radiator infos.
	RadInfo_struct rRadInfos;
	pSelRadSet->GetRadInfos( &rRadInfos );

	// Column radiator info.
	lRow = lFirstRow;
	AddStaticText( pclSheetDescription, ColumnDescription::RadiatorInfos, lRow, _T("-") );			// Default string
	if( true == m_pTADSPageSetup->GetField( epfRD7RADINFOREQPOW ) )
	{
		if( rRadInfos.dRequiredHeatOutput > 0 )
		{
			str2 = TASApp.LoadLocalizedString( IDS_SELP_PREQ );
			str2 += _T(" ");
			str1 += str2 + WriteCUDouble( _U_TH_POWER, rRadInfos.dRequiredHeatOutput, true );
			lRow = AddStaticText( pclSheetDescription, ColumnDescription::RadiatorInfos, lRow, str1 );
		}
	}

	str1 = _T("");
	if( true == m_pTADSPageSetup->GetField( epfRD7RADINFOTROOM ) )
	{
		if( rRadInfos.dRoomTemperature > 0 )
		{
			str2 = TASApp.LoadLocalizedString( IDS_SELP_TI );
			str2 += _T(" ");
			str1 = str2 + WriteCUDouble( _U_TEMPERATURE, rRadInfos.dRoomTemperature, true );
			if( true == m_pTADSPageSetup->GetField( epfRD7RADINFOTSUPPLY ) )
				str1 +=_T("; ");
		}
	}

	if( true == m_pTADSPageSetup->GetField( epfRD7RADINFOTSUPPLY ) )
	{
		if( rRadInfos.dSupplyTemperature > 0 )
		{
			str2 = TASApp.LoadLocalizedString( IDS_SELP_TS );
			str2 += _T(" ");
			str2 += WriteCUDouble( _U_TEMPERATURE, rRadInfos.dSupplyTemperature, true );
			str1 += str2;
		}
	}

	if( true == m_pTADSPageSetup->GetField( epfRD7RADINFOTROOM ) || true == m_pTADSPageSetup->GetField( epfRD7RADINFOTSUPPLY ) )
		lRow = AddStaticText( pclSheetDescription, ColumnDescription::RadiatorInfos, lRow, str1 );

	if( true == m_pTADSPageSetup->GetField( epfRD7RADINFODELTAT ) )
	{
		if( ( rRadInfos.dSupplyTemperature - rRadInfos.dReturnTemperature ) > 0 )
		{
			str2 = TASApp.LoadLocalizedString( IDS_SELP_DT );
			str2 += _T(" ");
			str1 = str2 + WriteCUDouble( _U_TEMPERATURE, rRadInfos.dSupplyTemperature - rRadInfos.dReturnTemperature, true );
			lRow = AddStaticText( pclSheetDescription, ColumnDescription::RadiatorInfos, lRow, str1 );
		}
	}

	if( true == m_pTADSPageSetup->GetField( epfRD7RADINFOINSTPOW ) )
	{
		if( rRadInfos.dInstalledPower > 0 )
		{
			FormatString( str1, IDS_SELP_PINST,
				_T(" (") + CString( WriteCUDouble( _U_TEMPERATURE, rRadInfos.dNCSupplyT, true ) ) +
				_T("/") + CString( WriteCUDouble( _U_TEMPERATURE, rRadInfos.dNCReturnT, true ) ) +
				_T("/") + CString( WriteCUDouble( _U_TEMPERATURE, rRadInfos.dNCRoomT, true ) ) + _T(")") );

			lRow = AddStaticText( pclSheetDescription, ColumnDescription::RadiatorInfos, lRow, str1 );

			str1 = WriteCUDouble( _U_TH_POWER, rRadInfos.dInstalledPower, true );
			lRow = AddStaticText( pclSheetDescription, ColumnDescription::RadiatorInfos, lRow, str1 );
		}
	}

	return pclSheet->GetMaxRows();
}

long CSelProdPageTrv::_FillRowSVInsert( CSheetDescription* pclSheetDescription, long lRow, CDS_SSelRadSet* pSelRadSet )
{
	if( NULL == pclSheetDescription || NULL == pclSheetDescription->GetSSheetPointer() || NULL == pSelRadSet )
		return lRow;

	CSSheet* pclSheet = pclSheetDescription->GetSSheetPointer();
	CString str1, str2;
	long lFirstRow = lRow;

	pclSheet->SetTextPattern( CSSheet::_SSTextPattern::UserStaticColumn );
	pclSheet->SetTextPatternProperty( CSSheet::_SSTextPatternProperty::TPP_FontBold, (LPARAM)FALSE );

	// Column 'Product'.
	AddStaticText( pclSheetDescription, ColumnDescription::Product, lRow, TASApp.LoadLocalizedString( IDS_SSHEETSELPROD_OTHERINSERT ) );
	
	// Column 'Article N°'.
	lRow = lFirstRow;
	AddStaticText( pclSheetDescription, ColumnDescription::ArticleNumber, lRow, _T("-") );

	// Column 'Qty'.
	lRow = lFirstRow;
	if( true == m_pTADSPageSetup->GetField( epfQUANTITY ) )	
	{
		str1.Format( _T("%d"), pSelRadSet->GetpSelectedInfos()->GetQuantity() );
		AddStaticText( pclSheetDescription, ColumnDescription::Quantity, lRow++, str1 );
	}

	if( true == pSelRadSet->IsFromDirSel() )
		return lRow;

	// Column 'Technical Info'.
	lRow = lFirstRow;
	AddStaticText( pclSheetDescription, ColumnDescription::TechnicalInfos, lRow, _T("-") );
	
	// HYS-1305 : Add Power / DT field for trv selection
	double dQ = pSelRadSet->GetQ();
	// HYS-38: Show power dt info when their radio button is checked
	CString strPowerDt = _T( "" );
	if( CDS_SelProd::efdPower == pSelRadSet->GetFlowDef() )
	{
		strPowerDt = WriteCUDouble( _U_TH_POWER, pSelRadSet->GetPower(), true );
		strPowerDt += _T( " / " );
		strPowerDt += WriteCUDouble( _U_DIFFTEMP, pSelRadSet->GetDT(), true );
	}
	if( true == m_pTADSPageSetup->GetField( epfRD7TECHINFOFLOW ) )
	{
		if( dQ > 0.0 )
		{
			// HYS-38: Show power dt info when their radio button is checked
			if( strPowerDt != _T( "" ) )
			{
				CString strQ = _T( "(" );
				strQ += WriteCUDouble( _U_FLOW, dQ, true );
				strQ += _T( ")" );
				lRow = AddStaticText( pclSheetDescription, ColumnDescription::TechnicalInfos, lRow, strPowerDt );
				lRow = AddStaticText( pclSheetDescription, ColumnDescription::TechnicalInfos, lRow, strQ );
			}
			else
			{
				lRow = AddStaticText( pclSheetDescription, ColumnDescription::TechnicalInfos, lRow, WriteCUDouble( _U_FLOW, dQ, true ) );
			}
		}
		else
			ASSERT( 0 );
	}

	// Add Kvs (or CV depending of current unit).
	if( !m_pUnitDB->GetDefaultUnitIndex( _C_KVCVCOEFF ) )
		str1 = TASApp.LoadLocalizedString( IDS_KVS );
	else
		str1 = TASApp.LoadLocalizedString( IDS_CV );
	str1 += CString( _T(" = ") ) + WriteCUDouble( _C_KVCVCOEFF, pSelRadSet->GetInsertKvValue(), false, 3, 2 );
	lRow = AddStaticText( pclSheetDescription, ColumnDescription::TechnicalInfos, lRow, str1 );
	
	if( true == m_pTADSPageSetup->GetField( epfRD7TECHINFODP ) )
	{
		str2 = TASApp.LoadLocalizedString( IDS_DP );
		if( pSelRadSet->GetSupplyValveDp() <= 0 )
		{
			CString str3;
			str3 = TASApp.LoadLocalizedString( IDS_UNKNOWN );
			str3.MakeLower();
			str1 = str2 + (CString)_T(" ") + str3;
		}
		else
		{
			str1 = str2 + (CString)_T(" = ");
			str1 += WriteCUDouble( _U_DIFFPRESS, pSelRadSet->GetSupplyValveDp(), true );
		}
		lRow = AddStaticText( pclSheetDescription, ColumnDescription::TechnicalInfos, lRow, str1 );
	}

	if( true == m_pTADSPageSetup->GetField( epfRD7TECHINFODPTOT ) )
	{
		// Show Dp only if different from -1.
		if( pSelRadSet->GetDp() != -1.0 )
		{
			str2 = TASApp.LoadLocalizedString( IDS_DPTOT );
			if( 0 == pSelRadSet->GetDp() )
			{
				CString str3;
				str3 = TASApp.LoadLocalizedString( IDS_UNKNOWN );
				str3.MakeLower();
				str1 = str2 + (CString)_T(" ") + str3;
			}
			else
			{
				str1 = str2 + (CString)_T(" = ");
				str1 += WriteCUDouble( _U_DIFFPRESS, pSelRadSet->GetDp(), true );
			}
			lRow = AddStaticText( pclSheetDescription, ColumnDescription::TechnicalInfos, lRow, str1 );
		}
	}
	
	// Retrieve infos concerning radiator infos.
	RadInfo_struct rRadInfos;
	pSelRadSet->GetRadInfos( &rRadInfos );

	// Column radiator info.
	lRow = lFirstRow;
	AddStaticText( pclSheetDescription, ColumnDescription::RadiatorInfos, lRow, _T("-") );
	if( true == m_pTADSPageSetup->GetField( epfRD7RADINFOREQPOW ) )
	{
		if( rRadInfos.dRequiredHeatOutput > 0 )
		{
			str2 = TASApp.LoadLocalizedString( IDS_SELP_PREQ );
			str2 += _T(" ");
			str1 += str2 + WriteCUDouble( _U_TH_POWER, rRadInfos.dRequiredHeatOutput, true );
			lRow = AddStaticText( pclSheetDescription, ColumnDescription::RadiatorInfos, lRow, str1 );
		}
	}

	str1 = _T("");
	if( true == m_pTADSPageSetup->GetField( epfRD7RADINFOTROOM ) )
	{
		if( rRadInfos.dRoomTemperature > 0 )
		{
			str2 = TASApp.LoadLocalizedString( IDS_SELP_TI );
			str2 += _T(" ");
			str1 = str2 + WriteCUDouble( _U_TEMPERATURE, rRadInfos.dRoomTemperature, true );
			if( true == m_pTADSPageSetup->GetField( epfRD7RADINFOTSUPPLY ) )
				str1 +=_T("; ");
		}
	}

	if( true == m_pTADSPageSetup->GetField( epfRD7RADINFOTSUPPLY ) )
	{
		if( rRadInfos.dSupplyTemperature > 0 )
		{
			str2 = TASApp.LoadLocalizedString( IDS_SELP_TS );
			str2 += _T(" ");
			str2 += WriteCUDouble( _U_TEMPERATURE, rRadInfos.dSupplyTemperature, true );
			str1 += str2;
		}
	}

	if( true == m_pTADSPageSetup->GetField( epfRD7RADINFOTROOM ) || true == m_pTADSPageSetup->GetField( epfRD7RADINFOTSUPPLY ) )
		lRow = AddStaticText( pclSheetDescription, ColumnDescription::RadiatorInfos, lRow, str1 );

	if( true == m_pTADSPageSetup->GetField( epfRD7RADINFODELTAT ) )
	{
		if( ( rRadInfos.dSupplyTemperature - rRadInfos.dReturnTemperature ) > 0 )
		{
			str2 = TASApp.LoadLocalizedString( IDS_SELP_DT );
			str2 += _T(" ");
			str1 = str2 + WriteCUDouble( _U_TEMPERATURE, rRadInfos.dSupplyTemperature - rRadInfos.dReturnTemperature, true );
			lRow = AddStaticText( pclSheetDescription, ColumnDescription::RadiatorInfos, lRow, str1 );
		}
	}

	if( true == m_pTADSPageSetup->GetField( epfRD7RADINFOINSTPOW ) )
	{
		if( rRadInfos.dInstalledPower > 0 )
		{
			FormatString( str1, IDS_SELP_PINST,
				_T(" (") + CString( WriteCUDouble( _U_TEMPERATURE, rRadInfos.dNCSupplyT, true ) ) +
				_T("/") + CString( WriteCUDouble( _U_TEMPERATURE, rRadInfos.dNCReturnT, true ) ) +
				_T("/") + CString( WriteCUDouble( _U_TEMPERATURE, rRadInfos.dNCRoomT, true ) ) + _T(")") );

			lRow = AddStaticText( pclSheetDescription, ColumnDescription::RadiatorInfos, lRow, str1 );

			str1 = WriteCUDouble( _U_TH_POWER, rRadInfos.dInstalledPower, true );
			lRow = AddStaticText( pclSheetDescription, ColumnDescription::RadiatorInfos, lRow, str1 );
		}
	}
	return pclSheet->GetMaxRows();
}

long CSelProdPageTrv::_FillSVActuatorFromSSel( CSheetDescription* pclSheetDescription, long lRow, CDS_SSelRadSet* pSelRadSet )
{
	if( NULL == pclSheetDescription || NULL == pclSheetDescription->GetSSheetPointer() || NULL == pSelRadSet )
	{
		return lRow;
	}

	CDB_Actuator* pclActuator = dynamic_cast<CDB_Actuator *>( pSelRadSet->GetSupplyValveActuatorIDPtr().MP );

	if( NULL == pclActuator )
	{
		return lRow;
	}

	_FillSVActuatorHelper( pclSheetDescription, lRow, pclActuator, pSelRadSet->GetpSelectedInfos()->GetQuantity(), 
		pSelRadSet->GetSVActuatorAccessoryList(), pSelRadSet->IsSelectedAsAPackage() );

	return pclSheetDescription->GetSSheetPointer()->GetMaxRows();
}

long CSelProdPageTrv::_FillSVActuatorFromHM( CSheetDescription *pclSheetDescription, long lRow, CDS_HydroMod::CCv *pclHMCv )
{
	if( NULL == pclSheetDescription || NULL == pclSheetDescription->GetSSheetPointer() || NULL == pclHMCv )
	{
		return lRow;
	}

	CDB_Actuator* pclActuator = dynamic_cast<CDB_Actuator *>( pclHMCv->GetActrIDPtr().MP );
	
	if( NULL == pclActuator )
	{
		return lRow;
	}

	bool fIsSet = false;

	CDB_ControlValve* pclControlValve = dynamic_cast<CDB_ControlValve *>( pclHMCv->GetCvIDPtr().MP );

	if( NULL != pclControlValve && true == pclControlValve->IsPartOfaSet() && eBool3::eb3True == pclHMCv->GetActrSelectedAsaPackage() )
	{
		fIsSet = true;
	}

	_FillSVActuatorHelper( pclSheetDescription, lRow, pclActuator, 1, NULL, fIsSet );

	return pclSheetDescription->GetSSheetPointer()->GetMaxRows();
}

long CSelProdPageTrv::_FillSVActuatorHelper( CSheetDescription *pclSheetDescription, long lRow, CDB_Actuator *pclActuator, int iQuantity, 
		CAccessoryList *pclAccessoryList, bool fIsSet )
{
	if( NULL == pclSheetDescription || NULL == pclSheetDescription->GetSSheetPointer() || NULL == pclActuator || 0 == iQuantity )
	{
		return lRow;
	}

	long lFirstRow = lRow;
	CSSheet *pclSheet = pclSheetDescription->GetSSheetPointer();
	CString str;
	pclSheet->SetTextPattern( CSSheet::_SSTextPattern::UserStaticColumn );
	pclSheet->SetTextPatternProperty( CSSheet::_SSTextPatternProperty::TPP_FontBold, (LPARAM)FALSE );

	// Add item into the full Article List.
	_AddArticleList( pclActuator, iQuantity, pclAccessoryList, fIsSet );
	
	// Set font color to red when accessory is not deleted or not available.
	if( true == pclActuator->IsDeleted() || false == pclActuator->IsAvailable() )
		pclSheet->SetTextPatternProperty( CSSheet::_SSTextPatternProperty::TPP_FontForeColor, (LPARAM)_RED );
	
	// Check first if we have a local article number available.
	CString strLocalArticleNumber = _T("");
	if( true == TASApp.IsLocalArtNumberUsed() )
	{
		strLocalArticleNumber = LocArtNumTab.GetLocalArticleNumber( pclActuator->GetArtNum() );
		strLocalArticleNumber.Trim();
	}

	// Fill article number.
	bool bArticleNumberShown = false;

	// If we can show article number OR we can't show but there is no local article number...
	if( false == TASApp.IsTAArtNumberHidden() || ( true == TASApp.IsTAArtNumberHidden() && true == strLocalArticleNumber.IsEmpty() )  )
	{
		CString str = pclActuator->GetArtNum();
		
		if( false == str.IsEmpty() )
		{
			AddStaticText( pclSheetDescription, ColumnDescription::ArticleNumber, lRow, pclActuator->GetArtNum() );
			bArticleNumberShown = true;
		}
	}
	
	// If local article number exist...
	if( true == TASApp.IsLocalArtNumberUsed() )
	{
		long lNextRow = lRow + ( ( true == bArticleNumberShown ) ? 1 : 0 );
		CString strPrefixLAN = TASApp.GetLocalArtNumberName();
		
		if( false == strPrefixLAN.IsEmpty() )
		{
			lNextRow = AddStaticText( pclSheetDescription, ColumnDescription::ArticleNumber, lNextRow, strPrefixLAN );
		}

		AddStaticText( pclSheetDescription, ColumnDescription::ArticleNumber, lNextRow, strLocalArticleNumber );
	}

	long lFlagRow = pclSheet->GetMaxRows() + 1;

	if( true == pclActuator->IsDeleted() )
	{
		AddStaticText( pclSheetDescription, ColumnDescription::ArticleNumber, lFlagRow, TASApp.LoadLocalizedString( IDS_TAPDELETED ) );
	}
	else if( false == pclActuator->IsAvailable() )
	{
		AddStaticText( pclSheetDescription, ColumnDescription::ArticleNumber, lFlagRow, TASApp.LoadLocalizedString( IDS_TAPNOTAVAILABLE ) );
	}

	pclSheet->SetTextPatternProperty( CSSheet::_SSTextPatternProperty::TPP_FontForeColor, (LPARAM)_BLACK );
		
	// Write nothing for technical info.
	AddStaticText( pclSheetDescription, ColumnDescription::TechnicalInfos, lRow, _T("-") );
		
	// Write quantity and prices.
	FillQtyPriceCol( pclSheetDescription, lRow, pclActuator, iQuantity );
		
	// Write the actuator name.
	AddStaticText( pclSheetDescription, ColumnDescription::Product, lRow++, pclActuator->GetName() );
	pclSheet->SetTextPatternProperty( CSSheet::_SSTextPatternProperty::TPP_StyleHorizontalAlign, (LPARAM)SSS_ALIGN_LEFT );

	// Write the actuator definition.
	CString strDescription = pclActuator->GetComment();

	if( false == strDescription.IsEmpty() )
	{
		AddStaticText( pclSheetDescription, ColumnDescription::Product, lRow, strDescription );
		AddCellSpanW( pclSheetDescription, ColumnDescription::Product, lRow++, 2, 1 );
	}

	if( NULL != dynamic_cast<CDB_ThermostaticActuator*>( pclActuator) )
	{
		CDB_ThermostaticActuator* pclThermostaticActuator = dynamic_cast<CDB_ThermostaticActuator*>( pclActuator );

		// Write setting range.
		int iMinSetting = pclThermostaticActuator->GetMinSetting();
		int iMaxSetting = pclThermostaticActuator->GetMaxSetting();
		str = TASApp.LoadLocalizedString( IDS_SSHEETSELPROD_SETTINGRANGE );

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

		AddStaticText( pclSheetDescription, ColumnDescription::Product, lRow, str );
		AddCellSpanW( pclSheetDescription, ColumnDescription::Product, lRow++, 2, 1 );

		// Write capillary length.
		int iCapillaryLength = pclThermostaticActuator->GetCapillaryLength();
		str = TASApp.LoadLocalizedString( IDS_SSHEETSELPROD_CAPILLARYLEN );
		
		if( iCapillaryLength > 0 )
		{
			str += CString( _T(" : ") ) + WriteCUDouble( _U_LENGTH, iCapillaryLength, true );
		}
		else
		{
			str += CString( _T(" : - ") );
		}

		AddStaticText( pclSheetDescription, ColumnDescription::Product, lRow, str );
		AddCellSpanW( pclSheetDescription, ColumnDescription::Product, lRow++, 2, 1 );

		// Write hysteresis.
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

		AddStaticText( pclSheetDescription, ColumnDescription::Product, lRow, str );
		AddCellSpanW( pclSheetDescription, ColumnDescription::Product, lRow++, 2, 1 );

		// Write frost protection.
		int iFrostProtection = pclThermostaticActuator->GetFrostProtection();
		str = TASApp.LoadLocalizedString( IDS_SSHEETSELPROD_FROSTPROTECTION );
		
		if( iFrostProtection > 0 )
		{
			str += CString( _T(" : ") ) + WriteCUDouble( _U_TEMPERATURE, iFrostProtection, true );
		}
		else
		{
			str += CString( _T(" : - ") );
		}

		AddStaticText( pclSheetDescription, ColumnDescription::Product, lRow, str );
		AddCellSpanW( pclSheetDescription, ColumnDescription::Product, lRow, 2, 1 );
		// HYS-726: Show max temp info in result tab
		str = TASApp.LoadLocalizedString( IDS_SSHEETSSELCV_ACTRMAXTEMP );

		if( DBL_MAX == pclThermostaticActuator->GetTmax() )
		{
			str += CString( _T(" :    -") );
		}
		else
		{
			str += CString( _T(" : ") ) + WriteDouble( pclThermostaticActuator->GetTmax(), 3 );
			str += TASApp.LoadLocalizedString( IDS_UNITDEGREECELSIUS );
		}

		AddStaticText( pclSheetDescription, ColumnDescription::Product, lRow, str );
		AddCellSpanW( pclSheetDescription, ColumnDescription::Product, lRow++, 2, 1 );
	}
	else if( NULL != dynamic_cast<CDB_ElectroActuator *>( pclActuator ) )
	{
		CDB_ElectroActuator *pclElectroActuator = dynamic_cast<CDB_ElectroActuator *>( pclActuator );

		// Write max force (or torque).
		if( CDB_CloseOffChar::Linear == pclElectroActuator->GetOpeningType() )
		{
			str = TASApp.LoadLocalizedString( IDS_SSHEETSELPROD_MAXFORCE );
		}
		else
		{
			str = TASApp.LoadLocalizedString( IDS_SSHEETSELPROD_MAXTORQUE );
		}

		str += CString( _T(" : ") ) + pclElectroActuator->GetMaxForceTorqueStr( true );
		AddStaticText( pclSheetDescription, ColumnDescription::Product, lRow, str );
		AddCellSpanW( pclSheetDescription, ColumnDescription::Product, lRow++, 2, 1 );
			
		// Compute Actuating time in sec/mm or sec/deg.
		str = TASApp.LoadLocalizedString( IDS_SSHEETSELPROD_ACTUATING_TIME ) + CString( _T(" : ") ) + pclElectroActuator->GetActuatingTimesStr( 0, true );
		AddStaticText( pclSheetDescription, ColumnDescription::Product, lRow, str );
		AddCellSpanW( pclSheetDescription, ColumnDescription::Product, lRow++, 2, 1 );

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
	}
	
	// Center the text below.
	pclSheet->SetTextPatternProperty( CSSheet::_SSTextPatternProperty::TPP_StyleHorizontalAlign, (LPARAM)SSS_ALIGN_CENTER );

	m_rProductParam.AddRange( lFirstRow, pclSheet->GetMaxRows(), pclActuator );
	return pclSheet->GetMaxRows();
}

long CSelProdPageTrv::_FillRowReturnValve( CSheetDescription* pclSheetDescription, long lRow, CDS_SSelRadSet* pSelRadSet )
{
	if( NULL == pclSheetDescription || NULL == pclSheetDescription->GetSSheetPointer() || NULL == pSelRadSet )
		return lRow;

	CDB_TAProduct* pTAP = dynamic_cast<CDB_TAProduct*>( pSelRadSet->GetReturnValveIDPtr().MP );
	if( NULL == pTAP )
		return lRow;

	CDB_RegulatingValve* pReturnValve = ( CDB_RegulatingValve *)pTAP;
	if( NULL == pReturnValve )
		return lRow;

	CSSheet* pclSheet = pclSheetDescription->GetSSheetPointer();
	CString str1, str2;
	long lFirstRow = lRow;

	pclSheet->SetTextPattern( CSSheet::_SSTextPattern::UserStaticColumn );
	pclSheet->SetTextPatternProperty( CSSheet::_SSTextPatternProperty::TPP_FontBold, (LPARAM)FALSE );

	// Add item into the full Article List.
	_AddArticleList( pTAP, pSelRadSet->GetpSelectedInfos()->GetQuantity(), pSelRadSet->GetReturnValveAccessoryList(), pSelRadSet->IsSelectedAsAPackage() );

	// TA product.
	_FillRowTrvTAP( pclSheetDescription, lRow, pReturnValve, pSelRadSet->GetpSelectedInfos()->GetQuantity() );

	// Column infos.
	lRow = lFirstRow;
	AddStaticText( pclSheetDescription, ColumnDescription::TechnicalInfos, lRow, _T("-") );
	
	// HYS-1305 : Add Power / DT field for trv selection
	double dQ = pSelRadSet->GetQ();
	// HYS-38: Show power dt info when their radio button is checked
	CString strPowerDt = _T( "" );
	if( CDS_SelProd::efdPower == pSelRadSet->GetFlowDef() )
	{
		strPowerDt = WriteCUDouble( _U_TH_POWER, pSelRadSet->GetPower(), true );
		strPowerDt += _T( " / " );
		strPowerDt += WriteCUDouble( _U_DIFFTEMP, pSelRadSet->GetDT(), true );
	}
	if( true == m_pTADSPageSetup->GetField( epfRD7TECHINFOFLOW ) )
	{
		if( dQ > 0.0 )
		{
			// HYS-38: Show power dt info when their radio button is checked
			if( strPowerDt != _T( "" ) )
			{
				CString strQ = _T( "(" );
				strQ += WriteCUDouble( _U_FLOW, dQ, true );
				strQ += _T( ")" );
				lRow = AddStaticText( pclSheetDescription, ColumnDescription::TechnicalInfos, lRow, strPowerDt );
				lRow = AddStaticText( pclSheetDescription, ColumnDescription::TechnicalInfos, lRow, strQ );
			}
			else
			{
				lRow = AddStaticText( pclSheetDescription, ColumnDescription::TechnicalInfos, lRow, WriteCUDouble( _U_FLOW, dQ, true ) );
			}
		}
		else
			ASSERT( 0 );
	}
	
	if( true == m_pTADSPageSetup->GetField( epfRD7TECHINFODP ) )
	{
		str2 = TASApp.LoadLocalizedString( IDS_DP );
		str1 = str2 + (CString)_T(" = ");
		str1 += WriteCUDouble( _U_DIFFPRESS, pSelRadSet->GetReturnValveDp(), true );
		lRow = AddStaticText( pclSheetDescription, ColumnDescription::TechnicalInfos, lRow, str1 );
	}
	
	if( true == m_pTADSPageSetup->GetField( epfRD7TECHINFOSETTING ) && RadiatorReturnValveMode::RRVM_IMI == pSelRadSet->GetReturnValveMode() )
	{	
		str2 = TASApp.LoadLocalizedString( IDS_SELP_TRVSETTING );						
		str1 = str2 + (CString)_T(" = ");


		CDB_ValveCharacteristic *pclValveCharacteristic = pReturnValve->GetValveCharacteristic();

		if( NULL != pclValveCharacteristic )
		{
			str1 += pclValveCharacteristic->GetSettingString( pSelRadSet->GetReturnValveOpening() );
		}
		else
		{
			str1 += _T("-");
		}

		lRow = AddStaticText( pclSheetDescription, ColumnDescription::TechnicalInfos, lRow, str1 );
	}

	m_rProductParam.AddRange( lFirstRow, pclSheet->GetMaxRows(), pReturnValve );
	return pclSheet->GetMaxRows();
}

void CSelProdPageTrv::_AddArticleList( CDB_TAProduct* pTAP, int iQuantity )
{
	if( NULL == pTAP )
	{
		return;
	}
	
	CArticleGroup *pclArticleGroup = new CArticleGroup();

	if( NULL == pclArticleGroup )
	{
		return;
	}

	CArticleContainer *pclArticleContainer = pclArticleGroup->AddArticle( pTAP, iQuantity );

	if( NULL == pclArticleContainer || NULL == pclArticleContainer->GetArticleItem() )
	{
		delete pclArticleContainer;
		return;
	}

	CArticleItem *pclArticleItem = pclArticleContainer->GetArticleItem();

	// Remark: 'm_strID', 'm_strArticle', 'm_strLocArtNum', 'm_dPrice', 'm_iQuantity', 'm_fIsAvailable' and 'm_fIsDeleted'
	//         variables are already set when creating 'CArticleContainer'.

	pclArticleItem->SetDescription( pTAP->GetName() + CString( _T("; ") ) + ( (CDB_StringID *)pTAP->GetVersionIDPtr().MP )->GetString() );

	CString str = ( (CDB_StringID *)pTAP->GetConnectIDPtr().MP )->GetString();
	str = str.Right( str.GetLength() - str.Find('/') - 1 );
	pclArticleItem->AddDescription( CString( _T("; ") ) + str );

	CSelProdPageBase::AddArticleInGroupList( pclArticleGroup );

	// Remark: pay attention that 'CPage::AddArticleInGroupList' will increment quantity if article already exist or
	//         will create a new 'CArticleGroup' and will copy to it data passed as argument. This is the reason
	//         why you need to delete 'pclArticleGroup' after the call.
	delete pclArticleGroup;

	if( true == pTAP->IsConnTabUsed() )
	{
		// Inlet
		CSelProdPageBase::AddCompArtList( pTAP, iQuantity, true );
		// Outlet
		CSelProdPageBase::AddCompArtList( pTAP, iQuantity, false );
	}
}

void CSelProdPageTrv::_AddArticleList( CDB_TAProduct* pTAP, int iQuantity, CAccessoryList* pclAccessoryList, bool fSelectionBySet )
{
	if( NULL == pTAP || 0 == iQuantity || NULL == pclAccessoryList )
	{
		return;
	}

	CArticleGroup *pclArticleGroup = new CArticleGroup();

	if( NULL == pclArticleGroup )
	{
		return;
	}

	CArticleContainer *pclArticleContainer = pclArticleGroup->AddArticle( pTAP, iQuantity );
	AddAccessoriesInArticleContainer( pclAccessoryList, pclArticleContainer, iQuantity, fSelectionBySet );

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

void CSelProdPageTrv::_AddArticleList( CDB_Actuator* pclActuator, int iQuantity, CAccessoryList* pclAccessoryList, bool fSelectionBySet )
{
	if( NULL == pclActuator || 0 == iQuantity )
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
	
	CDB_ThermostaticActuator *pclThermostaticActuator = dynamic_cast<CDB_ThermostaticActuator *>( pclActuator );
	CDB_ElectroActuator *pclElectroActuator = dynamic_cast<CDB_ElectroActuator *>( pclActuator );

	if( NULL != pclThermostaticActuator )
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

		if( iCapillaryLength > 0 )
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

		if( dHysteresis > 0.0 )
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

		if( iFrostProtection > 0 )
		{
			str += CString( _T(" : ") ) + WriteCUDouble( _U_TEMPERATURE, iFrostProtection, true );
		}
		else
		{
			str += CString( _T(" : - ") );
		}

		pclArticleItem->AddDescription( str );
	}
	else if( NULL != pclElectroActuator )
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
	
	pclArticleItem->SetPrice( TASApp.GetpTADB()->GetPrice( pclActuator->GetArtNum( true ) ) );
	pclArticleItem->SetQuantity( iQuantity );
	pclArticleItem->SetIsAvailable( pclActuator->IsAvailable() );
	pclArticleItem->SetIsDeleted( pclActuator->IsDeleted() );

	CString strArticleNumber = 	pclActuator->GetArtNum();
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
	
	// If actuator has accessories, we add them.
	if( NULL != pclAccessoryList )
	{
		AddAccessoriesInArticleContainer( pclAccessoryList, pclArticleContainer, iQuantity, fSelectionBySet );
	}

	CSelProdPageBase::AddArticleInGroupList( pclArticleGroup );

	// Remark: pay attention that 'CPage::AddArticleInGroupList' will increment quantity if article already exist or
	//         will create a new 'CArticleGroup' and will copy to it data passed as argument. This is the reason
	//         why you need to delete 'pclArticleGroup' after the call.
	delete pclArticleGroup;
}
