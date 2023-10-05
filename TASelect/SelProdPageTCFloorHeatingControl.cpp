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
#include "SelProdPageTCFloorHeatingControl.h"

CSelProdPageTCFloorHeatingControl::CSelProdPageTCFloorHeatingControl( CArticleGroupList *pclArticleGroupList ) 
	: CSelProdPageBase( CDB_PageSetup::enCheck::FLOORHEATINGCONTROL, pclArticleGroupList )
{
	m_pSelected = NULL;
}

void CSelProdPageTCFloorHeatingControl::Reset( void )
{
	CSelProdPageBase::Reset();
}

bool CSelProdPageTCFloorHeatingControl::PreInit( HMvector &vecHMList )
{
	// Something for individual or direct selection ?
	CTable *pclTable = (CTable *)( TASApp.GetpTADS()->Get( _T("FLOORHCTRL_TAB") ).MP );

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

bool CSelProdPageTCFloorHeatingControl::Init( bool bResetOrder, bool bPrint )
{
	bool bFloorHeatingControlExist = ( NULL != m_pclSelectionTable ) ? true : false;

	if( false == bFloorHeatingControlExist )
	{
		return false;
	}

	// Call base class method to get a new 'Sheet'.
	CSheetDescription *pclSDFloorHeatingControl = CMultiSpreadBase::CreateSSheet( SD_FloorHeatingControl );

	if( NULL == pclSDFloorHeatingControl || NULL == pclSDFloorHeatingControl->GetSSheetPointer() )
	{
		return false;
	}
	
	CSSheet *pclSheet = pclSDFloorHeatingControl->GetSSheetPointer();

	if( false == CSelProdPageBase::PrepareSheet( pclSDFloorHeatingControl, ColumnDescription::Pointer, bPrint ) )
	{
		m_ViewDescription.RemoveOneSheetDescription( SD_FloorHeatingControl );
		return false;
	}

	// Initialize column width for header and footer column.
	if( false == m_bForPrint )
	{
		SetColWidth( pclSDFloorHeatingControl, ColumnDescription::Header, DefaultColumnWidth::DCW_Header );
		SetColWidth( pclSDFloorHeatingControl, ColumnDescription::Footer, DefaultColumnWidth::DCW_Footer );
	}
	else
	{
		pclSheet->ShowCol( ColumnDescription::Header, FALSE );
		pclSheet->ShowCol( ColumnDescription::Footer, FALSE );
	}

	double dWidth = ( true == m_pTADSPageSetup->GetField( PageField_enum::epfFIRSTREF ) ) ? DefaultColumnWidth::DCW_Reference1 : 0.0;
	SetColWidth( pclSDFloorHeatingControl, ColumnDescription::Reference1, dWidth );
	
	dWidth = ( true == m_pTADSPageSetup->GetField( PageField_enum::epfSECONDREF ) ) ? DefaultColumnWidth::DCW_Reference2 : 0.0;
	SetColWidth( pclSDFloorHeatingControl, ColumnDescription::Reference2, dWidth );
	
	SetColWidth( pclSDFloorHeatingControl, ColumnDescription::Water, 0.0  );
	
	dWidth = ( true == m_pTADSPageSetup->GetField( PageField_enum::epfFLOORHEATINGCONTROLINFOPRODUCT ) ) ? DefaultColumnWidth::DCW_Product : 0.0;
	SetColWidth( pclSDFloorHeatingControl, ColumnDescription::Product, dWidth );
	
	SetColWidth( pclSDFloorHeatingControl, ColumnDescription::TechnicalInfos, 0.0  );
	
	SetColWidth( pclSDFloorHeatingControl, ColumnDescription::RadiatorInfos, 0.0 );
	
	dWidth = ( true == m_pTADSPageSetup->GetField( PageField_enum::epfARTICLE ) ) ? DefaultColumnWidth::DCW_ArticleNumber : 0.0;
	SetColWidth( pclSDFloorHeatingControl, ColumnDescription::ArticleNumber, dWidth );
	
	SetColWidth( pclSDFloorHeatingControl, ColumnDescription::Pipes, 0.0 );
	
	dWidth = ( true == m_pTADSPageSetup->GetField( PageField_enum::epfQUANTITY ) ) ? DefaultColumnWidth::DCW_Quantity : 0.0;
	SetColWidth( pclSDFloorHeatingControl, ColumnDescription::Quantity, dWidth );
	
	dWidth = 0.0;
	
	if( true == TASApp.IsPriceUsed() )
	{
		dWidth = ( true == m_pTADSPageSetup->GetField( PageField_enum::epfSALEUNITPRICE ) ) ? DefaultColumnWidth::DCW_UnitPrice : 0.0;
	}
	
	SetColWidth( pclSDFloorHeatingControl, ColumnDescription::UnitPrice, dWidth );
	
	dWidth = 0.0;

	if( true == TASApp.IsPriceUsed() )
	{
		dWidth = ( true == m_pTADSPageSetup->GetField( PageField_enum::epfSALETOTALPRICE ) ) ? DefaultColumnWidth::DCW_TotalPrice : 0.0;
	}
	
	SetColWidth( pclSDFloorHeatingControl, ColumnDescription::TotalPrice, dWidth );
	
	dWidth = ( true == m_pTADSPageSetup->GetField( PageField_enum::epfREMARK ) ) ? DefaultColumnWidth::DCW_Remark : 0.0;
	SetColWidth( pclSDFloorHeatingControl, ColumnDescription::Remark, dWidth );
	
	SetColWidth( pclSDFloorHeatingControl, ColumnDescription::Pointer, DefaultColumnWidth::DCW_Pointer );
	
	pclSheet->ShowCol( ColumnDescription::RadiatorInfos, FALSE );
	
	if( false == TASApp.IsPriceUsed() )
	{
		pclSheet->ShowCol( ColumnDescription::UnitPrice, FALSE );
		pclSheet->ShowCol( ColumnDescription::TotalPrice, FALSE );
	}

	pclSheet->ShowCol( ColumnDescription::Pointer, FALSE );
	CRect rect = pclSheet->GetSheetSizeInPixels( false );
	m_mapSheetInfos[pclSDFloorHeatingControl].m_dPageWidth = rect.Width();

	// Page title.
	SetPageTitle( pclSDFloorHeatingControl, IDS_RVIEWSELP_TITLE_FLOORHEATINGCONTROL );
	
	// Init column header.
	_InitColHeader( pclSDFloorHeatingControl );
	pclSheet->SetColumnAlwaysHidden( ColumnDescription::RadiatorInfos, true );

	// HYS-1571: We can optimize the column width now.
	// See Jira card to have more details about why we do that here.
	CRect rectClient;
	GetClientRect( &rectClient );
	OptimizePageSize( rectClient.Width(), rectClient.Height() );
	
	// Current position.
	long lRow = pclSheet->GetMaxRows() + 1;
	long lSelectedRow = 0;

	if( true == bFloorHeatingControlExist )
	{
		// Number of objects.
		int iFloorHeatingControlSelectedCount = m_pclSelectionTable->GetItemCount();

		if( 0 == iFloorHeatingControlSelectedCount )
		{
			return false;
		}

		CDS_SSelFloorHeatingManifold **parSelectedFloorHeatingManifold = new CDS_SSelFloorHeatingManifold * [iFloorHeatingControlSelectedCount];

		if( NULL == parSelectedFloorHeatingManifold )
		{
			return false;
		}

		memset( parSelectedFloorHeatingManifold, NULL, sizeof(CDS_SSelFloorHeatingManifold *) * iFloorHeatingControlSelectedCount );

		CDS_SSelFloorHeatingValve **parSelectedFloorHeatingValve = new CDS_SSelFloorHeatingValve * [iFloorHeatingControlSelectedCount];

		if( NULL == parSelectedFloorHeatingValve )
		{
			return false;
		}

		memset( parSelectedFloorHeatingValve, NULL, sizeof(CDS_SSelFloorHeatingValve *) * iFloorHeatingControlSelectedCount );

		CDS_SSelFloorHeatingController **parSelectedFloorHeatingController = new CDS_SSelFloorHeatingController * [iFloorHeatingControlSelectedCount];

		if( NULL == parSelectedFloorHeatingController )
		{
			return false;
		}

		memset( parSelectedFloorHeatingController, NULL, sizeof(CDS_SSelFloorHeatingController *) * iFloorHeatingControlSelectedCount );

		// We have created three different array to separate the display of each category (Manifold, valve and controller).
		// But we need to collect all the object in a same araray for the remarks (We show all remarks together after the last block).
		CDS_SSel **parSelectedProduct = new CDS_SSel * [iFloorHeatingControlSelectedCount];

		if( NULL == parSelectedProduct )
		{
			return false;
		}

		memset( parSelectedProduct, NULL, sizeof(CDS_SSel *) * iFloorHeatingControlSelectedCount );

		int iFloorHeatingManifoldCount = 0;
		int iFloorHeatingValveCount = 0;
		int iFloorHeatingControllerCount = 0;
		int iMaxFloorHeatingManifoldIndex = 0;
		int iMaxFloorHeatingValveIndex = 0;
		int iMaxFloorHeatingControllerIndex = 0;
		int iProductCount = 0;

		for( IDPTR IDPtr = m_pclSelectionTable->GetFirst(); _T('\0') != *IDPtr.ID; IDPtr = m_pclSelectionTable->GetNext() )
		{
			CSelectedInfos *pInfos = IDPtr.MP->GetpSelectedInfos();

			if( NULL == pInfos )
			{
				ASSERT_CONTINUE;
			}
			
			if( NULL != dynamic_cast<CDS_SSelFloorHeatingManifold *>( IDPtr.MP ) )
			{
				parSelectedFloorHeatingManifold[iFloorHeatingManifoldCount++] = (CDS_SSelFloorHeatingManifold *)( IDPtr.MP );
				iMaxFloorHeatingManifoldIndex = max( iMaxFloorHeatingManifoldIndex, pInfos->GetRowIndex() );
			}
			else if( NULL != dynamic_cast<CDS_SSelFloorHeatingValve *>( IDPtr.MP ) )
			{
				parSelectedFloorHeatingValve[iFloorHeatingValveCount++] = (CDS_SSelFloorHeatingValve *)( IDPtr.MP );
				iMaxFloorHeatingValveIndex = max( iMaxFloorHeatingValveIndex, pInfos->GetRowIndex() );
			}
			else
			{
				parSelectedFloorHeatingController[iFloorHeatingControllerCount++] = (CDS_SSelFloorHeatingController *)( IDPtr.MP );
				iMaxFloorHeatingControllerIndex = max( iMaxFloorHeatingControllerIndex, pInfos->GetRowIndex() );
			}

			parSelectedProduct[iProductCount++] = (CDS_SSel *)( IDPtr.MP );
		}

		if( 0 == iFloorHeatingManifoldCount && 0 == iFloorHeatingValveCount && 0 == iFloorHeatingControllerCount )
		{
			delete[] parSelectedFloorHeatingManifold;
			delete[] parSelectedFloorHeatingValve;
			delete[] parSelectedFloorHeatingController;
			return false;
		}

		// Initialize Remark in TADS.
		if( NULL != pDlgLeftTabSelP )
		{
			pDlgLeftTabSelP->SetRemarkIndex( CLASS( CData ), (CData **)parSelectedProduct, iProductCount );
		}
		
		// Sort 'parSelectedFloorHeatingManifold', 'parSelectedFloorHeatingValve' and 'parSelectedFloorHeatingController' arrays.
		// Remark: 'bResetOrder' is set to 'true' only when the sorting combos are empty (No user choice) and the user clicks on the 'Apply sorting keys'.
		if( true == bResetOrder ) 
		{
			// Reset row index.
			for( int i = 0; i < iFloorHeatingManifoldCount; i++ )
			{
				if( 0 == parSelectedFloorHeatingManifold[i]->GetpSelectedInfos()->GetRowIndex() )
				{
					parSelectedFloorHeatingManifold[i]->GetpSelectedInfos()->SetRowIndex( ++iMaxFloorHeatingManifoldIndex );
				}
			}

			for( int i = 0; i < iFloorHeatingValveCount; i++ )
			{
				if( 0 == parSelectedFloorHeatingValve[i]->GetpSelectedInfos()->GetRowIndex() )
				{
					parSelectedFloorHeatingValve[i]->GetpSelectedInfos()->SetRowIndex( ++iMaxFloorHeatingValveIndex );
				}
			}

			for( int i = 0; i < iFloorHeatingControllerCount; i++ )
			{
				if( 0 == parSelectedFloorHeatingController[i]->GetpSelectedInfos()->GetRowIndex() )
				{
					parSelectedFloorHeatingController[i]->GetpSelectedInfos()->SetRowIndex( ++iMaxFloorHeatingControllerIndex );
				}
			}
		}
		else if( PageField_enum::epfNONE != m_pTADSPageSetup->GetKey( 0 ) || PageField_enum::epfNONE != m_pTADSPageSetup->GetKey( 1 )
				|| PageField_enum::epfNONE != m_pTADSPageSetup->GetKey( 2 ) )
		{
			if( NULL != pDlgLeftTabSelP )
			{
				pDlgLeftTabSelP->SortTable( (CData **)parSelectedFloorHeatingManifold, iFloorHeatingManifoldCount - 1 );
				pDlgLeftTabSelP->SortTable( (CData **)parSelectedFloorHeatingValve, iFloorHeatingValveCount - 1 );
				pDlgLeftTabSelP->SortTable( (CData **)parSelectedFloorHeatingController, iFloorHeatingControllerCount - 1 );
			}

			// Reset row index.
			for( int i = 0; i < iFloorHeatingManifoldCount; i++ )
			{
				parSelectedFloorHeatingManifold[i]->GetpSelectedInfos()->SetRowIndex( i + 1 );
			}

			for( int i = 0; i < iFloorHeatingValveCount; i++ )
			{
				parSelectedFloorHeatingValve[i]->GetpSelectedInfos()->SetRowIndex( i + 1 );
			}

			for( int i = 0; i < iFloorHeatingControllerCount; i++ )
			{
				parSelectedFloorHeatingController[i]->GetpSelectedInfos()->SetRowIndex( i + 1 );
			}
		}

		// Remark: We can have from direct selection floor heating manifolds, control valves or controllers or all.
		CRank rkl;
		enum eFloorHeatingControlSwitch
		{
			DirSelFloorHeatingManifold,
			DirSelFloorHeatingValve,
			DirSelFloorHeatingController,
			LastFloorHeatingControlSwitch
		};

		bool bAtLeastOneBlockPrinted = false;

		for( int iLoopGroup = 0; iLoopGroup < LastFloorHeatingControlSwitch; iLoopGroup++ )
		{
			int i = 0;
			int iCount = 0;
			int iEndLoop = 0;

			if( DirSelFloorHeatingManifold == iLoopGroup )
			{
				iEndLoop = iFloorHeatingManifoldCount;
			}
			else if( DirSelFloorHeatingValve == iLoopGroup )
			{
				iEndLoop = iFloorHeatingValveCount;
			}
			else
			{
				iEndLoop = iFloorHeatingControllerCount;
			}

			for( ; i < iEndLoop; i++ )
			{
				CDS_SSel *pclFloorHeatingControl = NULL;

				if( DirSelFloorHeatingManifold == iLoopGroup )
				{
					if( false == parSelectedFloorHeatingManifold[i]->IsFromDirSel() )
					{
						ASSERT_CONTINUE;
					}

					pclFloorHeatingControl = parSelectedFloorHeatingManifold[i];
				}
				else if( DirSelFloorHeatingValve == iLoopGroup )
				{
					if( false == parSelectedFloorHeatingValve[i]->IsFromDirSel() )
					{
						ASSERT_CONTINUE;
					}

					pclFloorHeatingControl = parSelectedFloorHeatingValve[i];
				}
				else
				{
					if( false == parSelectedFloorHeatingController[i]->IsFromDirSel() )
					{
						ASSERT_CONTINUE;
					}

					pclFloorHeatingControl = parSelectedFloorHeatingController[i];
				}
				
				rkl.Add( _T(""), pclFloorHeatingControl->GetpSelectedInfos()->GetRowIndex(), (LPARAM)pclFloorHeatingControl );
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
			SetLastRow( pclSDFloorHeatingControl, lRow );

			switch( iLoopGroup )
			{
				case DirSelFloorHeatingManifold:
					SetPageTitle( pclSDFloorHeatingControl, IDS_SSHEETSELPROD_SUBTITLEFLOORHEATINGMANIFOLDSFROMSSEL, false, lRow );
					break;

				case DirSelFloorHeatingValve:
					SetPageTitle( pclSDFloorHeatingControl, IDS_SSHEETSELPROD_SUBTITLEFLOORHEATINGVALVESFROMSSEL, false, lRow );
					break;

				case DirSelFloorHeatingController:
					SetPageTitle( pclSDFloorHeatingControl, IDS_SSHEETSELPROD_SUBTITLEFLOORHEATINGCONTROLLERSFROMSSEL, false, lRow );
					break;
			}

			lRow = pclSheet->GetMaxRows();
			SetLastRow( pclSDFloorHeatingControl, lRow );

			CString str;
			LPARAM lparam;
			bool bFirstPass = true;

			for( BOOL bContinue = rkl.GetFirst( str, lparam ); TRUE == bContinue; bContinue = rkl.GetNext( str, lparam ) )
			{
				CData *pclData = (CData *)lparam;
				CDS_SSel *pclSSelFloorHeatingControl = dynamic_cast<CDS_SSel *>( pclData );

				if( NULL == pclSSelFloorHeatingControl )
				{
					ASSERT_CONTINUE;
				}

				if( m_pSelected == pclData )
				{
					lSelectedRow = lRow;
				}

				if( DirSelFloorHeatingManifold == iLoopGroup )
				{
					if( NULL == dynamic_cast<CDS_SSelFloorHeatingManifold *>( pclSSelFloorHeatingControl ) )
					{
						ASSERT_CONTINUE;
					}

					if( NULL == pclSSelFloorHeatingControl->GetProductAs<CDB_FloorHeatingManifold>() )
					{
						ASSERT_CONTINUE;
					}
				}
				else if( DirSelFloorHeatingValve == iLoopGroup )
				{
					if( NULL == dynamic_cast<CDS_SSelFloorHeatingValve *>( pclSSelFloorHeatingControl ) )
					{
						ASSERT_CONTINUE;
					}

					if( NULL == pclSSelFloorHeatingControl->GetProductAs<CDB_FloorHeatingValve>() )
					{
						ASSERT_CONTINUE;
					}
				}
				else
				{
					if( NULL == dynamic_cast<CDS_SSelFloorHeatingController *>( pclSSelFloorHeatingControl ) )
					{
						ASSERT_CONTINUE;
					}

					if( NULL == pclSSelFloorHeatingControl->GetProductAs<CDB_FloorHeatingController>() )
					{
						ASSERT_CONTINUE;
					}
				}

				m_rProductParam.Clear();
				m_rProductParam.SetSheetDescription( pclSDFloorHeatingControl );
				m_rProductParam.SetSelectionContainer( (LPARAM)pclSSelFloorHeatingControl );

				++lRow;
				long lFirstRow = lRow;

				if( DirSelFloorHeatingManifold == iLoopGroup )
				{
					CDS_SSelFloorHeatingManifold *pclSSelFloorHeatingManifold = dynamic_cast<CDS_SSelFloorHeatingManifold *>( pclSSelFloorHeatingControl );
					CDB_FloorHeatingManifold *pclFloorHeatingManifold = pclSSelFloorHeatingControl->GetProductAs<CDB_FloorHeatingManifold>();
					long lRowProduct = _FillRowFloorHeatingManifold( pclSDFloorHeatingControl, lRow, pclSSelFloorHeatingManifold, pclFloorHeatingManifold );
					long lRowGen = CSelProdPageBase::FillRowGen( pclSDFloorHeatingControl, lRow, pclSSelFloorHeatingManifold );
					lRow = max( lRowGen, lRowProduct );
				}
				else if( DirSelFloorHeatingValve == iLoopGroup )
				{
					CDS_SSelFloorHeatingValve *pclSSelFloorHeatingValve = dynamic_cast<CDS_SSelFloorHeatingValve *>( pclSSelFloorHeatingControl );
					CDB_FloorHeatingValve *pclFloorHeatingValve = pclSSelFloorHeatingControl->GetProductAs<CDB_FloorHeatingValve>();
					long lRowProduct = _FillRowFloorHeatingValve( pclSDFloorHeatingControl, lRow, pclSSelFloorHeatingValve, pclFloorHeatingValve );
					long lRowGen = CSelProdPageBase::FillRowGen( pclSDFloorHeatingControl, lRow, pclSSelFloorHeatingValve );
					lRow = max( lRowGen, lRowProduct );
				}
				else
				{
					CDS_SSelFloorHeatingController *pclSSelFloorHeatingController = dynamic_cast<CDS_SSelFloorHeatingController *>( pclSSelFloorHeatingControl );
					CDB_FloorHeatingController *pclFloorHeatingController = pclSSelFloorHeatingControl->GetProductAs<CDB_FloorHeatingController>();
					long lRowProduct = _FillRowFloorHeatingController( pclSDFloorHeatingControl, lRow, pclSSelFloorHeatingController, pclFloorHeatingController );
					long lRowGen = CSelProdPageBase::FillRowGen( pclSDFloorHeatingControl, lRow, pclSSelFloorHeatingController );
					lRow = max( lRowGen, lRowProduct );
				}

				
				// Add the selectable row range.
				m_rProductParam.AddRange( lFirstRow, lRow, pclSSelFloorHeatingControl->GetProductAs<CData>() );

				// Spanning must be done here because it's the only place where we know exactly the number of lines to span!
				// Span reference #1 and #2.
				AddCellSpanW( pclSDFloorHeatingControl, ColumnDescription::Reference1, lFirstRow, 1, pclSheet->GetMaxRows() - lFirstRow + 1 );
				AddCellSpanW( pclSDFloorHeatingControl, ColumnDescription::Reference2, lFirstRow, 1, pclSheet->GetMaxRows() - lFirstRow + 1 );

				// Span water characteristic.
				AddCellSpanW( pclSDFloorHeatingControl, ColumnDescription::Water, lFirstRow, 1, pclSheet->GetMaxRows() - lFirstRow + 1 );

				// Add accessories.
				if( true == pclSSelFloorHeatingControl->IsAccessoryExist() )
				{
					// Draw dash line.
					pclSheet->SetCellBorder( ColumnDescription::Reference1, lRow, ColumnDescription::Footer - 1, lRow, true, SS_BORDERTYPE_BOTTOM, SS_BORDERSTYLE_FINE_DASH );

					lRow++;
					lRow = FillAccessories( pclSDFloorHeatingControl, lRow, pclSSelFloorHeatingControl, pclSSelFloorHeatingControl->GetpSelectedInfos()->GetQuantity() );
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
		
		FillRemarks( pclSDFloorHeatingControl, lRow );

		delete[] parSelectedFloorHeatingManifold;
		delete[] parSelectedFloorHeatingValve;
		delete[] parSelectedFloorHeatingController;
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

bool CSelProdPageTCFloorHeatingControl::HasSomethingToDisplay( void )
{
	return ( NULL != m_pclSelectionTable ) ? true : false;
}

void CSelProdPageTCFloorHeatingControl::_InitColHeader( CSheetDescription *pclSheetDescription )
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
	str = TASApp.LoadLocalizedString( IDS_SELPHDR_PRODUCT );
	pclSheet->SetStaticText( ColumnDescription::Product, SelProdHeaderRow::HR_RowHeader, str );
	str = TASApp.LoadLocalizedString( IDS_SELPHDR_ART );
	pclSheet->SetStaticText( ColumnDescription::ArticleNumber, SelProdHeaderRow::HR_RowHeader, str );
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

long CSelProdPageTCFloorHeatingControl::_FillRowFloorHeatingManifold( CSheetDescription *pclSheetDescription, long lRow, 
		CDS_SSelFloorHeatingManifold *pclSSelFloorHeatingManifold, CDB_FloorHeatingManifold *pclFloorHeatingManifold )
{
	if( NULL == pclSheetDescription || NULL == pclSheetDescription->GetSSheetPointer() || NULL == pclSSelFloorHeatingManifold || NULL == pclFloorHeatingManifold )
	{
		return lRow;
	}

	CSSheet *pclSheet = pclSheetDescription->GetSSheetPointer();
	CString str1, str2;
	long lFirstRow = lRow;

	pclSheet->SetTextPattern( CSSheet::_SSTextPattern::UserStaticColumn );
	pclSheet->SetTextPatternProperty( CSSheet::_SSTextPatternProperty::TPP_FontBold, (LPARAM)FALSE );

	// Add item into the full article list.
	_AddArticleList( pclSSelFloorHeatingManifold );
		
	// Column TA Product.
	_FillRowFloorHeatingManifoldProduct( pclSheetDescription, lRow, pclFloorHeatingManifold, pclSSelFloorHeatingManifold->GetpSelectedInfos()->GetQuantity() );

	return pclSheet->GetMaxRows();
}

long CSelProdPageTCFloorHeatingControl::_FillRowFloorHeatingManifoldProduct( CSheetDescription *pclSheetDescription, long lRow, CDB_FloorHeatingManifold *pclFloorHeatingManifold, int iQuantity )
{
	if( NULL == pclSheetDescription || NULL == pclFloorHeatingManifold || NULL == TASApp.GetpTADS() || NULL == TASApp.GetpTADS()->GetpTechParams() )
	{
		return lRow;
	}

	CString str1, str2;
	long lFirstRow = lRow;
	
	// Column TA Product.
	AddStaticText( pclSheetDescription, ColumnDescription::Product, lRow, _T("-") );

	if( true == m_pTADSPageSetup->GetField( epfFLOORHEATINGCONTROLINFOPRODUCTNAME ) )	
	{
		lRow = AddStaticText( pclSheetDescription, ColumnDescription::Product, lRow, pclFloorHeatingManifold->GetName() );
	}
	
	if( true == m_pTADSPageSetup->GetField( epfFLOORHEATINGCONTROLINFOPRODUCTADDINFO ) )	
	{
		CString str;
		str.Format( _T("%s %i"), TASApp.LoadLocalizedString( IDS_SSHEETSELPROD_FLOORHEATINGCONTROL_HEATINGCIRCUITS ), pclFloorHeatingManifold->GetHeatingCircuits() );
		lRow = AddStaticText( pclSheetDescription, ColumnDescription::Product, lRow, str );
	}

	// Column 'Quantity'.
	FillQtyPriceCol( pclSheetDescription, lFirstRow, pclFloorHeatingManifold, iQuantity );
	
	// Column 'Article number'.
	AddStaticText( pclSheetDescription, ColumnDescription::ArticleNumber, lFirstRow, _T("-") );

	// Remark: 'true' to exclude any connection reference in the article number.
	FillArtNumberCol( pclSheetDescription, ColumnDescription::ArticleNumber, lFirstRow, pclFloorHeatingManifold, pclFloorHeatingManifold->GetArtNum( true ) );
	return lRow;
}

long CSelProdPageTCFloorHeatingControl::_FillRowFloorHeatingValve( CSheetDescription *pclSheetDescription, long lRow, 
		CDS_SSelFloorHeatingValve *pclSSelFloorHeatingValve, CDB_FloorHeatingValve *pclFloorHeatingValve )
{
	if( NULL == pclSheetDescription || NULL == pclSheetDescription->GetSSheetPointer() || NULL == pclSSelFloorHeatingValve || NULL == pclFloorHeatingValve )
	{
		return lRow;
	}

	CSSheet *pclSheet = pclSheetDescription->GetSSheetPointer();
	CString str1, str2;
	long lFirstRow = lRow;

	pclSheet->SetTextPattern( CSSheet::_SSTextPattern::UserStaticColumn );
	pclSheet->SetTextPatternProperty( CSSheet::_SSTextPatternProperty::TPP_FontBold, (LPARAM)FALSE );

	// Add item into the full article list.
	_AddArticleList( pclSSelFloorHeatingValve );
		
	// Column TA Product.
	_FillRowFloorHeatingValveProduct( pclSheetDescription, lRow, pclFloorHeatingValve, pclSSelFloorHeatingValve->GetpSelectedInfos()->GetQuantity() );

	return pclSheet->GetMaxRows();
}

long CSelProdPageTCFloorHeatingControl::_FillRowFloorHeatingValveProduct( CSheetDescription *pclSheetDescription, long lRow, 
		CDB_FloorHeatingValve *pclFloorHeatingValve, int iQuantity )
{
	if( NULL == pclSheetDescription || NULL == pclFloorHeatingValve || NULL == TASApp.GetpTADS() || NULL == TASApp.GetpTADS()->GetpTechParams() )
	{
		return lRow;
	}

	CString str1, str2;
	long lFirstRow = lRow;
	
	// Column TA Product.
	AddStaticText( pclSheetDescription, ColumnDescription::Product, lRow, _T("-") );

	if( true == m_pTADSPageSetup->GetField( epfFLOORHEATINGCONTROLINFOPRODUCTNAME ) )	
	{
		lRow = AddStaticText( pclSheetDescription, ColumnDescription::Product, lRow, pclFloorHeatingValve->GetName() );
	}

	if( true == m_pTADSPageSetup->GetField( epfFLOORHEATINGCONTROLINFOPRODUCTADDINFO ) )	
	{
		// Supply or lockshield.
		lRow = AddStaticText( pclSheetDescription, ColumnDescription::Product, lRow, pclFloorHeatingValve->GetSupplyOrLockshieldString() );

		// Connection inlet.
		CString str = TASApp.LoadLocalizedString( IDS_SSHEETSELPROD_FLOORHEATINGCONTROL_CONNECTIN );
		str += CString( _T(" : ") ) + pclFloorHeatingValve->GetConnectInlet();
		lRow = AddStaticText( pclSheetDescription, ColumnDescription::Product, lRow, str );

		// Connection outlet.
		str = TASApp.LoadLocalizedString( IDS_SSHEETSELPROD_FLOORHEATINGCONTROL_CONNECTOUT );
		str += CString( _T(" : ") ) + pclFloorHeatingValve->GetConnectOutlet();
		lRow = AddStaticText( pclSheetDescription, ColumnDescription::Product, lRow, str );

		// With or without handheld (Only for supply pipe control valve).
		if( 0 == pclFloorHeatingValve->GetSupplyOrLockshield() )
		{
			lRow = AddStaticText( pclSheetDescription, ColumnDescription::Product, lRow, pclFloorHeatingValve->GetHasHandwheelString() );
		}
	}

	// Column 'Quantity'.
	FillQtyPriceCol( pclSheetDescription, lFirstRow, pclFloorHeatingValve, iQuantity );
	
	// Column 'Article number'.
	AddStaticText( pclSheetDescription, ColumnDescription::ArticleNumber, lFirstRow, _T("-") );

	// Remark: 'true' to exclude any connection reference in the article number.
	FillArtNumberCol( pclSheetDescription, ColumnDescription::ArticleNumber, lFirstRow, pclFloorHeatingValve, pclFloorHeatingValve->GetArtNum( true ) );
	return lRow;
}

long CSelProdPageTCFloorHeatingControl::_FillRowFloorHeatingController( CSheetDescription *pclSheetDescription, long lRow, 
		CDS_SSelFloorHeatingController *pclSSelFloorHeatingController, CDB_FloorHeatingController *pclFloorHeatingController )
{
	if( NULL == pclSheetDescription || NULL == pclSheetDescription->GetSSheetPointer() || NULL == pclSSelFloorHeatingController || NULL == pclFloorHeatingController )
	{
		return lRow;
	}

	CSSheet *pclSheet = pclSheetDescription->GetSSheetPointer();
	CString str1, str2;
	long lFirstRow = lRow;

	pclSheet->SetTextPattern( CSSheet::_SSTextPattern::UserStaticColumn );
	pclSheet->SetTextPatternProperty( CSSheet::_SSTextPatternProperty::TPP_FontBold, (LPARAM)FALSE );

	// Add item into the full article list.
	_AddArticleList( pclSSelFloorHeatingController );
		
	// Column TA Product.
	_FillRowFloorHeatingControllerProduct( pclSheetDescription, lRow, pclFloorHeatingController, pclSSelFloorHeatingController->GetpSelectedInfos()->GetQuantity() );

	return pclSheet->GetMaxRows();
}

long CSelProdPageTCFloorHeatingControl::_FillRowFloorHeatingControllerProduct( CSheetDescription *pclSheetDescription, long lRow, 
		CDB_FloorHeatingController *pclFloorHeatingController, int iQuantity )
{
	if( NULL == pclSheetDescription || NULL == pclFloorHeatingController || NULL == TASApp.GetpTADS() || NULL == TASApp.GetpTADS()->GetpTechParams() )
	{
		return lRow;
	}

	CString str1, str2;
	long lFirstRow = lRow;
	
	// Column TA Product.
	AddStaticText( pclSheetDescription, ColumnDescription::Product, lRow, _T("-") );

	if( true == m_pTADSPageSetup->GetField( epfFLOORHEATINGCONTROLINFOPRODUCTNAME ) )	
	{
		lRow = AddStaticText( pclSheetDescription, ColumnDescription::Product, lRow, pclFloorHeatingController->GetName() );
	}

	// Column 'Quantity'.
	FillQtyPriceCol( pclSheetDescription, lFirstRow, pclFloorHeatingController, iQuantity );
	
	// Column 'Article number'.
	AddStaticText( pclSheetDescription, ColumnDescription::ArticleNumber, lFirstRow, _T("-") );

	// Remark: 'true' to exclude any connection reference in the article number.
	FillArtNumberCol( pclSheetDescription, ColumnDescription::ArticleNumber, lFirstRow, pclFloorHeatingController, pclFloorHeatingController->GetArtNum( true ) );
	return lRow;
}

void CSelProdPageTCFloorHeatingControl::_AddArticleList( CDB_Product *pclProduct, int iQuantity )
{
	if( NULL == pclProduct )
	{
		return;
	}

	CArticleGroup *pclArticleGroup = new CArticleGroup();
	
	if( NULL == pclArticleGroup )
	{
		return;
	}

	pclArticleGroup->AddArticle( pclProduct, iQuantity );
	CSelProdPageBase::AddArticleInGroupList( pclArticleGroup );

	// Remark: pay attention that 'CPage::AddArticleInGroupList' will increment quantity if article already exist or
	//         will create a new 'CArticleGroup' and will copy to it data passed as argument. This is the reason
	//         why you need to delete 'pclArticleGroup' after the call.
	delete pclArticleGroup;
}

void CSelProdPageTCFloorHeatingControl::_AddArticleList( CDS_SSel *pclSSelTapWaterControl )
{
	if( NULL == pclSSelTapWaterControl )
	{
		return;
	}

	CDB_Product *pclProduct = pclSSelTapWaterControl->GetProductAs<CDB_Product>();

	if( NULL == pclProduct )
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

	CArticleContainer *pclArticleContainer = pclArticleGroup->AddArticle( pclProduct, iQuantity );

	AddAccessoriesInArticleContainer( pclSSelTapWaterControl->GetAccessoryList(), pclArticleContainer, iQuantity, false );

	CSelProdPageBase::AddArticleInGroupList( pclArticleGroup );

	// Remark: pay attention that 'CPage::AddArticleList' will increment quantity if article already exist or
	//         will create a new 'CArticleContainer' and will copy to it data passed as argument. This is the reason
	//         why you need to delete 'pclArticleContainer' after the call.
	delete pclArticleGroup;
}
