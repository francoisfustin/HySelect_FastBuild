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
#include "SelProdPage6WayValve.h"

CSelProdPage6WayValve::CSelProdPage6WayValve( CArticleGroupList *pclArticleGroupList ) 
	: CSelProdPageBase( CDB_PageSetup::enCheck::SIXWAYVALVE, pclArticleGroupList )
{
	m_pSelected = NULL;
}

void CSelProdPage6WayValve::Reset( void )
{
	CSelProdPageBase::Reset();
}

bool CSelProdPage6WayValve::PreInit( HMvector &vecHMList )
{
	// Something for individual or direct selection ?
	CTable *pclTable = (CTable *)( TASApp.GetpTADS()->Get( _T("6WAYCTRLVALV_TAB") ).MP );

	if( NULL == pclTable )
	{
		ASSERT( 0 );
		return false;
	}

	m_pclSelectionTable = ( _T('\0') != *pclTable->GetFirst().ID ) ? pclTable : NULL;
	bool b6WayValveExist = ( NULL != m_pclSelectionTable );

	// Something for hydronic calculation?
	bool b6WayValveHMExist = ( vecHMList.size() > 0 );

	if( false == b6WayValveExist && false == b6WayValveHMExist )
	{
		return false;
	}
	
	m_vecHMList = vecHMList;
	SortTable();
	
	return true;
}

bool CSelProdPage6WayValve::Init( bool bResetOrder, bool bPrint )
{
	bool b6WayValveExist = ( NULL != m_pclSelectionTable ) ? true : false;
	bool b6WayValveHMExist = ( m_vecHMList.size() > 0 ) ? true : false;

	if( false == b6WayValveExist && false == b6WayValveHMExist )
	{
		return false;
	}

	// Call base class method to get a new 'Sheet'.
	CSheetDescription *pclSheetDescription6WayValve = CMultiSpreadBase::CreateSSheet( SD_6WayValve );

	if( NULL == pclSheetDescription6WayValve || NULL == pclSheetDescription6WayValve->GetSSheetPointer() )
	{
		return false;
	}
	
	CSSheet *pclSheet = pclSheetDescription6WayValve->GetSSheetPointer();

	if( false == CSelProdPageBase::PrepareSheet( pclSheetDescription6WayValve, ColumnDescription::Pointer, bPrint ) )
	{
		m_ViewDescription.RemoveOneSheetDescription( SD_6WayValve );
		return false;
	}

	// Initialize column width for header and footer columns.
	if( false == m_bForPrint )
	{
		SetColWidth( pclSheetDescription6WayValve, ColumnDescription::Header, DefaultColumnWidth::DCW_Header );
		SetColWidth( pclSheetDescription6WayValve, ColumnDescription::Footer, DefaultColumnWidth::DCW_Footer );
	}
	else
	{
		pclSheet->ShowCol( ColumnDescription::Header, FALSE );
		pclSheet->ShowCol( ColumnDescription::Footer, FALSE );
	}

	double dWidth = ( true == m_pTADSPageSetup->GetField( PageField_enum::epfFIRSTREF ) ) ? DefaultColumnWidth::DCW_Reference1 : 0.0;
	SetColWidth( pclSheetDescription6WayValve, ColumnDescription::Reference1, dWidth );

	dWidth = ( true == m_pTADSPageSetup->GetField( PageField_enum::epfSECONDREF ) ) ? DefaultColumnWidth::DCW_Reference2 : 0.0;
	SetColWidth( pclSheetDescription6WayValve, ColumnDescription::Reference2, dWidth );
	
	dWidth = ( true == m_pTADSPageSetup->GetField( PageField_enum::epfWATERINFO ) ) ? DefaultColumnWidth::DCW_Water : 0.0;
	SetColWidth( pclSheetDescription6WayValve, ColumnDescription::Water, dWidth );
	
	dWidth = ( true == m_pTADSPageSetup->GetField( PageField_enum::epf6WAYVALVEINFOPRODUCT ) ) ? DefaultColumnWidth::DCW_Product : 0.0;
	SetColWidth( pclSheetDescription6WayValve, ColumnDescription::Product, dWidth );
	
	dWidth = ( true == m_pTADSPageSetup->GetField( PageField_enum::epf6WAYVALVETECHINFO ) ) ? DefaultColumnWidth::DCW_TechnicalInfos : 0.0;
	SetColWidth( pclSheetDescription6WayValve, ColumnDescription::TechnicalInfos, dWidth );
	
	SetColWidth( pclSheetDescription6WayValve, ColumnDescription::RadiatorInfos, 0.0 );
	
	dWidth = ( true == m_pTADSPageSetup->GetField( PageField_enum::epfARTICLE ) ) ? DefaultColumnWidth::DCW_ArticleNumber : 0.0;
	SetColWidth( pclSheetDescription6WayValve, ColumnDescription::ArticleNumber, dWidth );
	
	dWidth = ( true == m_pTADSPageSetup->GetField( PageField_enum::epfPIPEINFO ) ) ? DefaultColumnWidth::DCW_Pipes : 0.0;
	SetColWidth( pclSheetDescription6WayValve, ColumnDescription::Pipes, dWidth );
	
	dWidth = ( true == m_pTADSPageSetup->GetField( PageField_enum::epfQUANTITY ) ) ? DefaultColumnWidth::DCW_Quantity : 0.0;
	SetColWidth( pclSheetDescription6WayValve, ColumnDescription::Quantity, dWidth );
	
	dWidth = 0.0;

	if( true == TASApp.IsPriceUsed() )
	{
		dWidth = ( true == m_pTADSPageSetup->GetField( PageField_enum::epfSALEUNITPRICE ) ) ? DefaultColumnWidth::DCW_UnitPrice : 0.0;
	}
	
	SetColWidth( pclSheetDescription6WayValve, ColumnDescription::UnitPrice, dWidth );
	
	dWidth = 0.0;

	if( true == TASApp.IsPriceUsed() )
	{
		dWidth = ( true == m_pTADSPageSetup->GetField( PageField_enum::epfSALETOTALPRICE ) ) ? DefaultColumnWidth::DCW_TotalPrice : 0.0;
	}

	SetColWidth( pclSheetDescription6WayValve, ColumnDescription::TotalPrice, dWidth );
	
	SetColWidth( pclSheetDescription6WayValve, ColumnDescription::Remark, ( true == m_pTADSPageSetup->GetField( PageField_enum::epfREMARK ) ) ? DefaultColumnWidth::DCW_Remark : 0 );
	SetColWidth( pclSheetDescription6WayValve, ColumnDescription::Pointer, DefaultColumnWidth::DCW_Pointer );
	pclSheet->ShowCol( ColumnDescription::RadiatorInfos, FALSE );
	
	if( false == TASApp.IsPriceUsed() )
	{
		pclSheet->ShowCol( ColumnDescription::UnitPrice, FALSE );
		pclSheet->ShowCol( ColumnDescription::TotalPrice, FALSE );
	}

	pclSheet->ShowCol( ColumnDescription::Pointer, FALSE );
	CRect rect = pclSheet->GetSheetSizeInPixels( false );
	m_mapSheetInfos[pclSheetDescription6WayValve].m_dPageWidth = rect.Width();
	
	// Page title.
	SetPageTitle( pclSheetDescription6WayValve, IDS_SSHEETSELPROD_TITLE6WAYVALVE );

	// Init column header.
	_InitColHeader( pclSheetDescription6WayValve );

	// HYS-1571: We can optimize the column width now.
	// See Jira card to have more details about why we do that here.
	CRect rectClient;
	GetClientRect( &rectClient );
	OptimizePageSize( rectClient.Width(), rectClient.Height() );

	// Current position.
	long lRow = pclSheet->GetMaxRows() + 1;
	long lSelectedRow = 0;

	if( true == b6WayValveExist )
	{
		int i6WayValveCount = m_pclSelectionTable->GetItemCount( CLASS( CDS_SSel6WayValve ) );

		if( 0 == i6WayValveCount )
		{
			return false;
		}

		CDS_SSel6WayValve **ppaSSel6WayValve = new CDS_SSel6WayValve * [i6WayValveCount];
		
		if( NULL == ppaSSel6WayValve )
		{
			return false;
		}
		
		// Count items. Do it locally prevent to make loops several times.
		int iMaxIndex = 0;
		int i = 0;
		
		// Fill 'ppaSSel6WayValve' array with pointer on each 'CDS_SSel6WayValve' object of the '6WAYCTRLVALV_TAB' table.
		for( IDPTR IDPtr = m_pclSelectionTable->GetFirst(); _T('\0') != *IDPtr.ID; IDPtr = m_pclSelectionTable->GetNext() )
		{
			CDS_SSel6WayValve *pclSSel6WayValve = dynamic_cast<CDS_SSel6WayValve *>( IDPtr.MP );
			
			if( NULL == pclSSel6WayValve )
			{
				continue;
			}
			
			if( NULL == pclSSel6WayValve->GetIDPtr().MP )
			{
				continue;
			}
			
			ppaSSel6WayValve[i] = pclSSel6WayValve;

			if( iMaxIndex < ppaSSel6WayValve[i]->GetpSelectedInfos()->GetRowIndex() )
			{
				iMaxIndex = ppaSSel6WayValve[i]->GetpSelectedInfos()->GetRowIndex();
			}

			++i;
		}
		
		ASSERT( i == i6WayValveCount );

		// Sort 'ppaSSel6WayValve' array.
		// Remark: 'fResetOrder' is set to 'true' only when the sorting combos are empty (No user choice) and the user clicks on the 'Apply sorting keys'.
		if( true == bResetOrder )
		{
			// Reset row index.
			for( i = 0; i < i6WayValveCount; i++ )
			{
				ppaSSel6WayValve[i]->GetpSelectedInfos()->SetRowIndex( i + 1 );
			}
		}
		else if( PageField_enum::epfNONE != m_pTADSPageSetup->GetKey( 0 ) || PageField_enum::epfNONE != m_pTADSPageSetup->GetKey( 1 ) 
	 			|| PageField_enum::epfNONE != m_pTADSPageSetup->GetKey( 2 ) )
		{
			// This method will sort (or not) in regards to the three sorting keys that user has chosen.
			if( NULL != pDlgLeftTabSelP )
			{
				pDlgLeftTabSelP->SortTable( (CData**)ppaSSel6WayValve, i6WayValveCount - 1 );
			}

			// Reset row index.
			for( i = 0; i < i6WayValveCount; i++ )
			{
				ppaSSel6WayValve[i]->GetpSelectedInfos()->SetRowIndex( i + 1 );
			}
		}

		// Create a remark index.
		int iRemIndex = -1;

		if( NULL != pDlgLeftTabSelP )
		{
			iRemIndex = pDlgLeftTabSelP->SetRemarkIndex( CLASS( CDS_SSel6WayValve ), (CData **)ppaSSel6WayValve, i6WayValveCount );
		}

		// By default, the items are deleted when read.
		CRank rkl;

		enum e6WayValveSwitch
		{
			SingleSel6WayValve,
			DirSel6WayValve,
			Last6WayValveSwitch
		};

		bool bAtLeastOneBlockPrinted = false;
		
		for( int iLoopGroup = 0; iLoopGroup < Last6WayValveSwitch; iLoopGroup++ )
		{
			int iCount = 0;

			// For each selected 6-way valve.
			for( i = 0; i < i6WayValveCount; i++ )
			{	
				CDS_SSel6WayValve *pSel6WayValve = dynamic_cast<CDS_SSel6WayValve *>( ppaSSel6WayValve[i] );

				if( NULL == pSel6WayValve || NULL == pSel6WayValve->GetProductIDPtr().MP )
				{
					ASSERT( 0 );
					continue;
				}

				if( SingleSel6WayValve == iLoopGroup )
				{
					if( true == pSel6WayValve->IsFromDirSel() )
					{
						continue;
					}
				}
				else
				{
					if( false == pSel6WayValve->IsFromDirSel() )
					{
						continue;
					}
				}

				CDB_6WayValve *pcl6WayValve = dynamic_cast<CDB_6WayValve *>( pSel6WayValve->GetProductIDPtr().MP );

				if( NULL == pcl6WayValve )
				{
					ASSERT( 0 );
					continue;
				}

				rkl.Add( _T( "" ), ppaSSel6WayValve[i]->GetpSelectedInfos()->GetRowIndex(), ( LPARAM )ppaSSel6WayValve[i] );
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
			SetLastRow( pclSheetDescription6WayValve, lRow );
			
			switch( iLoopGroup )
			{
				case SingleSel6WayValve:
					SetPageTitle( pclSheetDescription6WayValve, IDS_SSHEETSELPROD_SUBTITLE6WAYVALVEFROMSSEL, false, lRow, 0.0, _WHITE, _IMI_TITLE_GROUP1, 14 );
					break;

				case DirSel6WayValve:
					SetPageTitle( pclSheetDescription6WayValve, IDS_SSHEETSELPROD_SUBTITLE6WAYVALVEFROMDIRSEL, false, lRow, 0.0, _WHITE, _IMI_TITLE_GROUP1, 14 );
					break;

				default:
					ASSERT( 0 );
					break;
			}

			CString str;
			LPARAM lparam;
			bool bFirstPass = true;
			
			for( BOOL bContinue = rkl.GetFirst( str, lparam ); TRUE == bContinue; bContinue = rkl.GetNext( str, lparam ) )
			{
				CData *pclData = (CData *)lparam;
				CDS_SSel6WayValve *pclSSel6WayValve = dynamic_cast<CDS_SSel6WayValve *>( pclData );

				if( NULL == pclSSel6WayValve )
				{
					delete[] ppaSSel6WayValve;
					return false;
				}

				CDB_6WayValve *pcl6WayValve = dynamic_cast<CDB_6WayValve *>( pclSSel6WayValve->Get6WayValveIDPtr().MP );

				if( NULL == pcl6WayValve )
				{
					delete[] ppaSSel6WayValve;
					return false;
				}

				if( m_pSelected == pclData )
				{
					lSelectedRow = lRow;
				}

				m_rProductParam.Clear();
				m_rProductParam.SetSheetDescription( pclSheetDescription6WayValve );
				m_rProductParam.SetSelectionContainer( (LPARAM)pclSSel6WayValve );

				++lRow;
				long lFirstRow = lRow;
				lRow = _Fill6WayValve( pclSheetDescription6WayValve, lRow, pclSSel6WayValve );

				// Spanning must be done here because it's the only place where we know exactly number of lines to span!
				// Span reference #1 and #2.
				AddCellSpanW( pclSheetDescription6WayValve, ColumnDescription::Reference1, lFirstRow + 1, 1, pclSheet->GetMaxRows() - lFirstRow );
				AddCellSpanW( pclSheetDescription6WayValve, ColumnDescription::Reference2, lFirstRow + 1, 1, pclSheet->GetMaxRows() - lFirstRow );
	
				// Span water characteristic.
				AddCellSpanW( pclSheetDescription6WayValve, ColumnDescription::Water, lFirstRow + 1, 1, pclSheet->GetMaxRows() - lFirstRow );
					
				// Add accessories.
				if( true == pclSSel6WayValve->IsAccessoryExist() )
				{
					// Draw dash line.
					pclSheet->SetCellBorder( ColumnDescription::Reference1, lRow, ColumnDescription::Footer - 1, lRow, true, SS_BORDERTYPE_BOTTOM, SS_BORDERSTYLE_FINE_DASH );
					
					lRow++;
					// HYS-1877: Consider set selection when filling aaccessories.
					lRow = _FillAccessories( pclSheetDescription6WayValve, lRow, pclSSel6WayValve->GetAccessoryList(), pclSSel6WayValve->GetpSelectedInfos()->GetQuantity(),
											 pclSSel6WayValve->IsSelectedAsAPackage(true) );
				}

				// Fill actuator if exist.
				if( NULL != dynamic_cast<CDB_ElectroActuator *>( pclSSel6WayValve->Get6WayValveActuatorIDPtr().MP ) )
				{
					// Draw dash line.
					pclSheet->SetCellBorder( ColumnDescription::Reference1, lRow, ColumnDescription::Footer - 1, lRow, true, SS_BORDERTYPE_BOTTOM, SS_BORDERSTYLE_FINE_DASH );

					lRow++;
					CDB_ElectroActuator *pclActuator = dynamic_cast<CDB_ElectroActuator *>( pclSSel6WayValve->Get6WayValveActuatorIDPtr().MP );
					// HYS-1877
					lRow = _FillActuator( pclSheetDescription6WayValve, lRow, pclActuator, pclSSel6WayValve, pclSSel6WayValve->GetpSelectedInfos()->GetQuantity(), false );
				}
				
				if( true == pclSSel6WayValve->IsActuatorAccessoryExist() )
				{
					// Draw dash line.
					pclSheet->SetCellBorder( ColumnDescription::Reference1, lRow, ColumnDescription::Footer - 1, lRow, true, SS_BORDERTYPE_BOTTOM, SS_BORDERSTYLE_FINE_DASH );

					lRow++;
					// HYS-1877: Consider set selection when filling aaccessories.
					lRow = _FillAccessories( pclSheetDescription6WayValve, lRow, pclSSel6WayValve->Get6WayValveActuatorAccessoryList(), pclSSel6WayValve->GetpSelectedInfos()->GetQuantity(),
											 pclSSel6WayValve->IsSelectedAsAPackage( true ) );
				}

				// Set the first group as no breakable (for print).
				// Remark: include title with the group.
				pclSheet->SetFlagRowNoBreakable( ( true == bFirstPass ) ? lFirstRow - 1 : lFirstRow, lRow, true );
				
				// Draw dash line.
				pclSheet->SetCellBorder( ColumnDescription::Reference1, lRow, ColumnDescription::Footer - 1, lRow, true, SS_BORDERTYPE_BOTTOM, SS_BORDERSTYLE_FINE_DASH );

				if( e6Way_Alone != pclSSel6WayValve->GetSelectionMode() )
				{
					if( e6Way_EQMControl == pclSSel6WayValve->GetSelectionMode() )
					{
						lRow = _FillPIBCValve( pclSheetDescription6WayValve, lRow, pclSSel6WayValve, BothSide );
					}
					else if( e6Way_OnOffControlWithPIBCV == pclSSel6WayValve->GetSelectionMode() )
					{
						lRow = _FillPIBCValve( pclSheetDescription6WayValve, lRow, pclSSel6WayValve, CoolingSide );
						lRow = _FillPIBCValve( pclSheetDescription6WayValve, lRow, pclSSel6WayValve, HeatingSide );
					}
					else
					{
						lRow = _FillBalancingValve( pclSheetDescription6WayValve, lRow, pclSSel6WayValve, CoolingSide );
						lRow = _FillBalancingValve( pclSheetDescription6WayValve, lRow, pclSSel6WayValve, HeatingSide );
					}
				}

				// Draw line below.
				pclSheet->SetCellBorder( ColumnDescription::Reference1, lRow, ColumnDescription::Footer - 1, lRow, true, SS_BORDERTYPE_BOTTOM, SS_BORDERSTYLE_SOLID );

				// Save object pointer.
				m_rProductParam.SetScrollRange( ( true == bFirstPass ) ? lFirstRow - 1 : lFirstRow, lRow );
				SaveProduct( m_rProductParam );
				
				_AddArticleList( pclSSel6WayValve );

				bFirstPass = false;
			}

			lRow++;
			bAtLeastOneBlockPrinted = true;
		}

		// Verify if a product is selected.
		if( 0 == lSelectedRow )
		{
			m_pSelected = NULL;
		}

		FillRemarks( pclSheetDescription6WayValve, lRow );

		// We add by default a blank line.
		lRow = pclSheet->GetMaxRows() + 2;

		delete[] ppaSSel6WayValve;
	}

	if( true == b6WayValveHMExist )
	{
		// By default, the items are deleted when read.
		CRank rkl;

		HMvector vecHMList;
		int iCount = 0;

		// For each hydraulic circuit.
		int i;

		for( i = 0; i < (int)m_vecHMList.size(); i++ )
		{
			CDS_HydroMod::C6WayValve *pHM6WayValve = static_cast<CDS_HydroMod::C6WayValve *>( m_vecHMList[i].second );

			if( NULL == pHM6WayValve || NULL == dynamic_cast<CDB_6WayValve *>( pHM6WayValve->Get6WayValveIDPtr().MP ) )
			{
				continue;
			}

			CDB_6WayValve *pcl6WayValve = dynamic_cast<CDB_6WayValve *>( pHM6WayValve->Get6WayValveIDPtr().MP );
			vecHMList.push_back( CSelProdPageBase::PairPtr( CSelProdPageBase::ePointerType::eptCCV, (void *)pHM6WayValve ) );
			rkl.Add( _T(""), i, (LPARAM)pHM6WayValve );
			iCount++;
		}

		if( 0 == iCount )
		{
			return false;
		}

		// Create a remark index for the current group (CV, BCV or PIBCV).
		if( NULL != pDlgLeftTabSelP )
		{
			pDlgLeftTabSelP->SetRemarkIndex( &vecHMList );
		}

		// Set the subtitle.
		SetLastRow( pclSheetDescription6WayValve, lRow );
		SetPageTitle( pclSheetDescription6WayValve, IDS_SSHEETSELPROD_SUBTITLECVFROMHM, false, lRow );
			
		lRow++;
		SetLastRow( pclSheetDescription6WayValve, lRow );
			
		CString str;
		LPARAM lparam;
		bool bFirstPass = true;

		for( BOOL bContinue = rkl.GetFirst( str, lparam ); TRUE == bContinue; bContinue = rkl.GetNext( str, lparam ) )
		{
			CDS_HydroMod::C6WayValve *pHM6WayValve = (CDS_HydroMod::C6WayValve*)lparam;

			if( NULL == pHM6WayValve )
			{
				ASSERTA_RETURN( false );
			}

			CDB_6WayValve *pcl6WayValve = dynamic_cast<CDB_6WayValve *>( pHM6WayValve->Get6WayValveIDPtr().MP );
			if( NULL == pcl6WayValve )
			{
				ASSERTA_RETURN( false );
			}

			m_rProductParam.Clear();
			m_rProductParam.SetSheetDescription( pclSheetDescription6WayValve );
			
			// Intentionally set to NULL to avoid clicking, double clicking or right clicking.
			m_rProductParam.SetSelectionContainer( (LPARAM)0 );

			long lFirstRow = ++lRow;
			lRow = _Fill6WayValve( pclSheetDescription6WayValve, lRow, pHM6WayValve );

			// HYS-721: Merge cells
			// Spanning must be done here because it's the only place where we know exactly number of lines to span!
			// Span reference #1 and #2.
			AddCellSpanW(pclSheetDescription6WayValve, ColumnDescription::Reference1, lFirstRow, 1, pclSheet->GetMaxRows() - lFirstRow + 1);
			AddCellSpanW(pclSheetDescription6WayValve, ColumnDescription::Reference2, lFirstRow, 1, pclSheet->GetMaxRows() - lFirstRow + 1);
			
			// Span water characteristic.
			AddCellSpanW(pclSheetDescription6WayValve, ColumnDescription::Water, lFirstRow, 1, pclSheet->GetMaxRows() - lFirstRow + 1);

			lRow = FillAndAddBuiltInHMAccessories( pclSheet, pclSheetDescription6WayValve, pcl6WayValve, lRow );

			// Fill adapter if exist.
			if( NULL != dynamic_cast<CDB_Product*>( (CData*)pHM6WayValve->Get6WayValveActuatorAdapterIDPtr().MP ) && 
					NULL != dynamic_cast<CDB_Product*>( (CData*)pHM6WayValve->Get6WayValveActuatorAdapterIDPtr().MP )->IsAnAccessory() )
			{
				// Draw dash line.
				pclSheet->SetCellBorder( ColumnDescription::Reference1, lRow, ColumnDescription::Footer - 1, lRow, true, SS_BORDERTYPE_BOTTOM, SS_BORDERSTYLE_FINE_DASH );

				// Add adapter if we can automatically select it.
				lRow++;
				lRow = _FillAdapterFromHM( pclSheetDescription6WayValve, lRow, pHM6WayValve );
			}

			// Fill actuator if exist.
			if( NULL != dynamic_cast<CDB_ElectroActuator *>( pHM6WayValve->Get6WayValveActuatorIDPtr().MP ) )
			{
				// Draw dash line.
				pclSheet->SetCellBorder( ColumnDescription::Reference1, lRow, ColumnDescription::Footer - 1, lRow, true, SS_BORDERTYPE_BOTTOM, SS_BORDERSTYLE_FINE_DASH );

				lRow++;

				CDB_ElectroActuator *pclActuator = dynamic_cast<CDB_ElectroActuator *>( pHM6WayValve->Get6WayValveActuatorIDPtr().MP );
				// HYS-1877: NULL parameter because the function will be reviewed for HMCalc
				lRow = _FillActuator( pclSheetDescription6WayValve, lRow, pclActuator, NULL, 1, false );
			}

			// Draw line below.
			pclSheet->SetCellBorder( ColumnDescription::Reference1, lRow, ColumnDescription::Footer - 1, lRow, true, SS_BORDERTYPE_BOTTOM, SS_BORDERSTYLE_SOLID );

			// Save the product.
			m_rProductParam.SetScrollRange( ( true == bFirstPass ) ? lFirstRow - 1 : lFirstRow, lRow );
			SaveProduct( m_rProductParam );
					
			// Set all group as no breakable (for print).
			// Remark: include title with the group.
			pclSheet->SetFlagRowNoBreakable( ( true == bFirstPass ) ? lFirstRow - 1 : lFirstRow, lRow, true );
					
			_AddArticleList( pHM6WayValve );

			bFirstPass = false;
		}

		lRow++;
		FillHMRemarks( &vecHMList, pclSheetDescription6WayValve, lRow );
	}

	// Move sheet to correct position.
	SetSheetSize();
	Invalidate();
	UpdateWindow();

	return true;
}

bool CSelProdPage6WayValve::HasSomethingToDisplay( void )
{
	bool b6WayValveExist = ( NULL != m_pclSelectionTable ) ? true : false;
	bool b6WayValveHMExist = ( m_vecHMList.size() > 0 ) ? true : false;

	if( false == b6WayValveExist && false == b6WayValveHMExist )
	{
		return false;
	}

	return true;
}

void CSelProdPage6WayValve::OnSelectCell( CProductParam &clProductParam )
{
	if( NULL == clProductParam.m_pclSheetDescription || NULL == clProductParam.m_pclSheetDescription->GetSSheetPointer() )
	{
		ASSERT_RETURN;
	}

	CSheetDescription *pclSheetDescription = clProductParam.m_pclSheetDescription;
	CSSheet *pclSSheet = pclSheetDescription->GetSSheetPointer();

	pclSSheet->SetBackColor( 2, clProductParam.m_lSelectionTopRow, m_mapSheetInfos[pclSheetDescription].m_iColNum - 2, 
			clProductParam.m_lSelectionTopRow, SELPRODPAGE_SELECTIONBORDERCOLOR );
}

void CSelProdPage6WayValve::OnUnselectCell( CProductParam &clProductParam )
{
	if( NULL == clProductParam.m_pclSheetDescription || NULL == clProductParam.m_pclSheetDescription->GetSSheetPointer() )
	{
		ASSERT_RETURN;
	}

	COLORREF clTextColor = 0;
	COLORREF clBackgroundColor = 0;
	double dRowSize = 0.0;
	int iFontSize = 0;
	_GetGroupColorAndSizes( true, BothSide, clTextColor, clBackgroundColor, dRowSize, iFontSize );

	CSheetDescription *pclSheetDescription = clProductParam.m_pclSheetDescription;
	CSSheet *pclSSheet = pclSheetDescription->GetSSheetPointer();

	pclSSheet->SetBackColor( 2, clProductParam.m_lSelectionTopRow, m_mapSheetInfos[pclSheetDescription].m_iColNum - 2, 
			clProductParam.m_lSelectionTopRow, clBackgroundColor );
}

void CSelProdPage6WayValve::_InitColHeader( CSheetDescription *pclSheetDescription )
{
	if( NULL == pclSheetDescription || NULL == pclSheetDescription->GetSSheetPointer() )
	{
		return;
	}

	CSSheet *pclSheet = pclSheetDescription->GetSSheetPointer();
	CString str;
	
	// Format Columns Header.
	pclSheet->SetTextPattern( CSSheet::_SSTextPattern::ColumnHeader );
	pclSheet->SetTextPatternProperty( CSSheet::_SSTextPatternProperty::TPP_FontBackColor, (LPARAM)_TAH_WHITE );
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

long CSelProdPage6WayValve::_Fill6WayValve( CSheetDescription* pclSheetDescription, long lRow, CDS_HydroMod::C6WayValve* pclHM6WayValve )
{
	if( NULL == pclSheetDescription || NULL == pclSheetDescription->GetSSheetPointer() || NULL == pclHM6WayValve )
	{
		return lRow;
	}

	CDB_6WayValve *pcl6WayValve = dynamic_cast<CDB_6WayValve *>( pclHM6WayValve->Get6WayValveIDPtr().MP );
	
	if( NULL == pcl6WayValve )
	{
		return lRow;
	}
	
	CSSheet *pclSheet = pclSheetDescription->GetSSheetPointer();
	CString str;
	long lMaxRow, lFirstRow;
	lMaxRow = lFirstRow = lRow;

	pclSheet->SetTextPattern( CSSheet::_SSTextPattern::UserStaticColumn );
	pclSheet->SetTextPatternProperty( CSSheet::_SSTextPatternProperty::TPP_FontBold, (LPARAM)FALSE );

	// TODO !!!!!!!!!
	
	m_rProductParam.AddRange( lFirstRow, pclSheet->GetMaxRows(), pcl6WayValve );
	return pclSheet->GetMaxRows();
}

long CSelProdPage6WayValve::_Fill6WayValve( CSheetDescription *pclSheetDescription, long lRow, CDS_SSel6WayValve *pclSSel6WayValve )
{
	if( NULL == pclSheetDescription || NULL == pclSheetDescription->GetSSheetPointer() || NULL == pclSSel6WayValve )
	{
		return lRow;
	}

	CDB_6WayValve *pcl6WayValve = pclSSel6WayValve->GetProductAs<CDB_6WayValve>(); 

	if( NULL == pcl6WayValve )
	{
		return lRow;
	}
	
	CSSheet *pclSheet = pclSheetDescription->GetSSheetPointer();
	CString str;
	
	long lRangeFirstRow = lRow;
	SetLastRow( pclSheetDescription, lRow );

	COLORREF clTextColor = 0;
	COLORREF clBackgroundColor = 0;
	double dRowSize = 0.0;
	int iFontSize = 0;
	_GetGroupColorAndSizes( true, BothSide, clTextColor, clBackgroundColor, dRowSize, iFontSize );
	SetPageTitle( pclSheetDescription, TASApp.LoadLocalizedString( IDS_SSHEETSELPROD_SUBTITLE6WAYVALVE ), false, lRow, dRowSize, clTextColor, clBackgroundColor, iFontSize );

	lRow++;
	long lFirstRow = lRow;

	pclSheet->SetTextPattern( CSSheet::_SSTextPattern::UserStaticColumn );
	pclSheet->SetTextPatternProperty( CSSheet::_SSTextPatternProperty::TPP_FontBold, (LPARAM)FALSE );

	// Remark: 'true' to exclude any connection reference in the article number.
	CString strArticleNumber = CteEMPTY_STRING;
	// HYS-1877: Display Set article number when working on a set.
	if( true == pclSSel6WayValve->IsSelectedAsAPackage( true ) )
	{
		CDB_Set* p6WayValveSet = dynamic_cast<CDB_Set*>(pclSSel6WayValve->GetCvActrSetIDPtr().MP);
		if( NULL != p6WayValveSet )
		{
			strArticleNumber = p6WayValveSet->GetReference();
			FillArtNumberColForPackage( pclSheetDescription, ColumnDescription::ArticleNumber, lRow, p6WayValveSet, strArticleNumber );
			FillQtyPriceCol( pclSheetDescription, lRow, p6WayValveSet, pclSSel6WayValve->GetpSelectedInfos()->GetQuantity() );
		}
	}
	else
	{
		strArticleNumber = pcl6WayValve->GetArtNum( true );
		FillArtNumberCol( pclSheetDescription, ColumnDescription::ArticleNumber, lRow, pcl6WayValve, strArticleNumber );
		FillQtyPriceCol( pclSheetDescription, lRow, pcl6WayValve, pclSSel6WayValve->GetpSelectedInfos()->GetQuantity() );
	}

	_Fill6WayValveRowGen( pclSheetDescription, lRow, pclSSel6WayValve );
	
	// Column TA Product.
	long lRowProduct = _Fill6WayValveTAP( pclSheetDescription, lRow, pcl6WayValve );

	// Column 'Infos'.
	lRow = lFirstRow;
	AddStaticText( pclSheetDescription, ColumnDescription::TechnicalInfos, lRow, _T("-") );

	// Flow.
	// If 6-way valve is directly selected don't fill Water, pipes.
	// HYS-1380: Display cooling and heating flow for TA-6way-valves
	if( false == pclSSel6WayValve->IsFromDirSel() )
	{
		double dCoolingFlow = pclSSel6WayValve->GetCoolingFlow();
		ASSERT( dCoolingFlow > 0.0 );

		double dHeatingFlow = pclSSel6WayValve->GetHeatingFlow();
		ASSERT( dHeatingFlow > 0.0 );
		// First loop for cooling and second for heating
		for( int i = 0; i < 2; i++ )
		{
			double dFlow = (i == 0 ) ? dCoolingFlow : dHeatingFlow;
			CWaterChar WaterChar = (i == 0 ) ? pclSSel6WayValve->GetWC(CoolingSide) : pclSSel6WayValve->GetWC( HeatingSide );
			if( true == m_pTADSPageSetup->GetField( epf6WAYVALVETECHINFOFLOW )
				&& 0.0 < dCoolingFlow && 0.0 < dHeatingFlow )
			{
				CString strCooling = TASApp.LoadLocalizedString( IDS_SSHETSELPROD_6WAYVALVE_COOLINGINFOS ) + _T( " " );
				CString strHeating = TASApp.LoadLocalizedString( IDS_SSHETSELPROD_6WAYVALVE_HEATINGINFOS ) + _T( " " );
				str = ( i == 0 ) ? strCooling : strHeating;
				lRow = AddStaticText( pclSheetDescription, ColumnDescription::TechnicalInfos, lRow, str );

				// HYS-38: Show power dt info when their radio button is checked
				CString strPowerDt = _T( "" );

				if( CDS_SelProd::efdPower == pclSSel6WayValve->GetFlowDef() )
				{
					double dPower = ( i == 0 ) ? pclSSel6WayValve->GetCoolingPower() : pclSSel6WayValve->GetHeatingPower();
					double dDT = ( i == 0 ) ? pclSSel6WayValve->GetCoolingDT() : pclSSel6WayValve->GetHeatingDT();
					strPowerDt = WriteCUDouble( _U_TH_POWER, dPower, true );
					strPowerDt += _T( " / " );
					strPowerDt += WriteCUDouble( _U_DIFFTEMP, dDT, true );
				}

				if( strPowerDt != _T( "" ) )
				{
					CString strQ = _T( "(" );
					strQ += WriteCUDouble( _U_FLOW, dFlow, true );
					strQ += _T( ")" );
					lRow = AddStaticText( pclSheetDescription, ColumnDescription::TechnicalInfos, lRow, strPowerDt );
					lRow = AddStaticText( pclSheetDescription, ColumnDescription::TechnicalInfos, lRow, strQ );
				}
				else
				{
					lRow = AddStaticText( pclSheetDescription, ColumnDescription::TechnicalInfos, lRow, WriteCUDouble( _U_FLOW, dFlow, true ) );
				}
			}

			if( true == m_pTADSPageSetup->GetField( epf6WAYVALVETECHINFODP ) )
			{
				// Dp.
				str = _T("");
				double dDp = CalcDp( dFlow, pcl6WayValve->GetKvs(), WaterChar.GetDens() );

				if( -1.0 != dDp )
				{
					str += WriteCUDouble( _U_DIFFPRESS, dDp, true );
				}
				else
				{
					str += _T( "-" );
				}

				lRow = AddStaticText( pclSheetDescription, ColumnDescription::TechnicalInfos, lRow, str );
			}
		}
	}
	
	m_rProductParam.AddRange( lRangeFirstRow, pclSheet->GetMaxRows(), pcl6WayValve );
	return pclSheet->GetMaxRows();
}

long CSelProdPage6WayValve::_Fill6WayValveTAP( CSheetDescription *pclSheetDescription, long lRow, CDB_TAProduct *pTAP )
{
	if( NULL == pclSheetDescription || NULL == pclSheetDescription->GetSSheetPointer() || NULL == pTAP
			|| NULL == dynamic_cast<CDB_6WayValve *>( pTAP ) )
	{
		return lRow;
	}

	CSSheet *pclSSheet = pclSheetDescription->GetSSheetPointer();
	CString str;

	// Column 'Product'.
	AddStaticText( pclSheetDescription, ColumnDescription::Product, lRow, _T("-") );
	
	if( true == m_pTADSPageSetup->GetField( epf6WAYVALVEINFOPRODUCTNAME ) )
	{
		lRow = AddStaticText( pclSheetDescription, ColumnDescription::Product, lRow, pTAP->GetName() );
		pclSSheet->SetFontBold( ColumnDescription::Product, lRow - 1, TRUE );
	}

	if( true == m_pTADSPageSetup->GetField( epf6WAYVALVEINFOPRODUCTKVS ) )
	{
		CDB_6WayValve *pcl6WayValve = dynamic_cast<CDB_6WayValve *>( pTAP );

		if( 0 == m_pUnitDB->GetDefaultUnitIndex( _C_KVCVCOEFF ) )
		{
			str = TASApp.LoadLocalizedString( IDS_KVS );
		}
		else
		{
			str = TASApp.LoadLocalizedString( IDS_CV );
		}

		str += CString( _T(" = ") ) + WriteCUDouble( _C_KVCVCOEFF, pcl6WayValve->GetKvs() );
		lRow = AddStaticText( pclSheetDescription, ColumnDescription::Product, lRow, str );
	}

	// Body Material.
	if( true == m_pTADSPageSetup->GetField( epf6WAYVALVEINFOPRODUCTBDYMATERIAL ) )
	{
		CDB_StringID *pStrID = dynamic_cast <CDB_StringID *>( pTAP->GetBodyMaterialIDPtr().MP );

		if( NULL != pStrID )
		{
			lRow = AddStaticText( pclSheetDescription, ColumnDescription::Product, lRow,pStrID->GetString() );
		}
	}
	
	// Connection.
	if( true == m_pTADSPageSetup->GetField( epf6WAYVALVEINFOPRODUCTCONNECTION ) )
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
	if( true == m_pTADSPageSetup->GetField( epf6WAYVALVEINFOPRODUCTVERSION ) )
	{
		CDB_StringID *pStrID = dynamic_cast <CDB_StringID *>( pTAP->GetVersionIDPtr().MP );

		if( NULL != pStrID )
		{
			lRow = AddStaticText( pclSheetDescription, ColumnDescription::Product, lRow, pStrID->GetString() );
		}
	}

	// PN.
	if( true == m_pTADSPageSetup->GetField( epf6WAYVALVEINFOPRODUCTPN ) )
	{
		lRow = FillPNTminTmax( pclSheetDescription, ColumnDescription::Product, lRow, pTAP );
	}

	return pclSheetDescription->GetSSheetPointer()->GetMaxRows();
}

long CSelProdPage6WayValve::_Fill6WayValveRowGen( CSheetDescription *pclSheetDescription, long lRow, CDS_SSel6WayValve *pclSSel6WayValve )
{
	if( NULL == pclSheetDescription || NULL == pclSheetDescription->GetSSheetPointer() || NULL == pclSSel6WayValve )
	{
		ASSERTA_RETURN( lRow );
	}

	CString str1, str2;
	long lFirstRow = lRow;

	CSSheet *pclSheet = pclSheetDescription->GetSSheetPointer();
	pclSheet->SetTextPattern( CSSheet::_SSTextPattern::UserStaticColumn );
	pclSheet->SetTextPatternProperty( CSSheet::_SSTextPatternProperty::TPP_FontBold, (LPARAM)FALSE );

	// Column Reference 1.
	pclSheet->SetRowHeight( lRow, m_dRowHeight );
	pclSheet->SetTextPatternProperty( CSSheet::_SSTextPatternProperty::TPP_StyleWordWrap, (LPARAM)TRUE );
	pclSheet->SetTextPatternProperty( CSSheet::_SSTextPatternProperty::TPP_StyleHorizontalAlign, (LPARAM)SSS_ALIGN_LEFT );
	AddStaticText( pclSheetDescription, ColumnDescription::Reference1, lRow, pclSSel6WayValve->GetpSelectedInfos()->GetReference( CSelectedInfos::eRef1 ) );

	// Column Reference 2.
	AddStaticText( pclSheetDescription, ColumnDescription::Reference2, lRow, pclSSel6WayValve->GetpSelectedInfos()->GetReference( CSelectedInfos::eRef2 ) );
	pclSheet->SetTextPatternProperty( CSSheet::_SSTextPatternProperty::TPP_StyleWordWrap, (LPARAM)FALSE );
	pclSheet->SetTextPatternProperty( CSSheet::_SSTextPatternProperty::TPP_StyleHorizontalAlign, (LPARAM)SSS_ALIGN_CENTER );

	// If it's directly selected don't fill Water, pipes.
	if( false == pclSSel6WayValve->IsFromDirSel() 
			&& ( e6Way_Alone == pclSSel6WayValve->GetSelectionMode() || e6Way_EQMControl == pclSSel6WayValve->GetSelectionMode() ) )
	{
		// Column water char.
		long lPrev = pclSheet->GetTextPatternProperty( CSSheet::_SSTextPatternProperty::TPP_StyleWordWrap );
		pclSheet->SetTextPatternProperty( CSSheet::_SSTextPatternProperty::TPP_StyleWordWrap, (LPARAM)TRUE );

		// Fluid name.
		pclSSel6WayValve->GetCoolingWC().BuildWaterNameString( str1 );

		// Cooling temperatures.
		str2 = TASApp.LoadLocalizedString( IDS_SSHEETSELPROD_6WAYVALVECOOLINGTEMP ) + _T(": ");
		str2 += WriteCUDouble( _U_TEMPERATURE, pclSSel6WayValve->GetCoolingWC().GetTemp(), false ) + CString( _T(" / ") );
		str2 += WriteCUDouble( _U_TEMPERATURE, pclSSel6WayValve->GetCoolingWC().GetTemp() + pclSSel6WayValve->GetpSelectedInfos()->GetCoolingDT(), true );
	
		CString strFinal = str1 + _T("\n") + str2;

		// Heating temperatures.
		str2 = TASApp.LoadLocalizedString( IDS_SSHEETSELPROD_6WAYVALVEHEATINGTEMP ) + _T(": ");
		str2 += WriteCUDouble( _U_TEMPERATURE, pclSSel6WayValve->GetHeatingWC().GetTemp(), false ) + CString( _T(" / ") );
		str2 += WriteCUDouble( _U_TEMPERATURE, pclSSel6WayValve->GetHeatingWC().GetTemp() - pclSSel6WayValve->GetpSelectedInfos()->GetHeatingDT(), true );

		strFinal += _T("\n") + str2;
		AddStaticText( pclSheetDescription, ColumnDescription::Water, lRow, strFinal );
		
		lRow += 3;
		pclSheet->SetTextPatternProperty( CSSheet::_SSTextPatternProperty::TPP_StyleWordWrap, (LPARAM)lPrev );

		//Build the pipe string according to page setup 
		//Quid of LinDpflag and Uflag ??? Mark in red and italic???
		//SetTextRC(row,7,...);
		CDB_Pipe *pPipe = pclSSel6WayValve->GetpSelPipe()->GetpPipe();
		lRow = lFirstRow;

		if( NULL != pPipe )
		{
			pclSheet->SetTextPatternProperty( CSSheet::_SSTextPatternProperty::TPP_StyleWordWrap, ( LPARAM )TRUE );

			CTable *pPipeTab = dynamic_cast<CTable *> (pPipe->GetIDPtr().PP);
			ASSERT( NULL != pPipeTab );

			if( NULL != pPipeTab && true == m_pTADSPageSetup->GetField( PageField_enum::epfPIPENAME ) )
			{
				lRow = AddStaticText( pclSheetDescription, ColumnDescription::Pipes, lRow, pPipeTab->GetName(), true );
			}

			if( true == m_pTADSPageSetup->GetField( PageField_enum::epfPIPESIZE ) )
			{
				lRow = AddStaticText( pclSheetDescription, ColumnDescription::Pipes, lRow, pPipe->GetName() );
			}
			
			if( true == m_pTADSPageSetup->GetField( PageField_enum::epfPIPEWATERU ) )
			{
				if( pclSSel6WayValve->GetpSelPipe()->GetU() != 0.0 )
				{
					lRow = AddStaticText( pclSheetDescription, ColumnDescription::Pipes, lRow, WriteCUDouble( _U_VELOCITY, pclSSel6WayValve->GetpSelPipe()->GetU(), true ) );
				}
			}

			if( true == m_pTADSPageSetup->GetField( PageField_enum::epfPIPEDP ) )
			{
				if( pclSSel6WayValve->GetpSelPipe()->GetLinDp() != 0.0 )
				{
					lRow = AddStaticText( pclSheetDescription, ColumnDescription::Pipes, lRow, WriteCUDouble( _U_LINPRESSDROP, pclSSel6WayValve->GetpSelPipe()->GetLinDp(), true ) );
				}
			}
		}
		else
		{
			lRow = AddStaticText( pclSheetDescription, ColumnDescription::Pipes, lRow, _T("-") );
		}
	}

	// Column Remark.
	lRow = lFirstRow;

	if( true == m_pTADSPageSetup->GetField( PageField_enum::epfREMARK ) )	
	{
		if( 0 != pclSSel6WayValve->GetpSelectedInfos()->GetRemarkIndex() )		// remark exist
		{
			str1.Format( _T("%d"), pclSSel6WayValve->GetpSelectedInfos()->GetRemarkIndex() );
			lRow = AddStaticText( pclSheetDescription, ColumnDescription::Remark, lRow, str1 );
		}
	}

	return pclSheet->GetMaxRows();
}

long CSelProdPage6WayValve::_FillAccessories( CSheetDescription *pclSheetDescription, long lRow, CAccessoryList *pclAccessoryList, int iGlobalQuantity, bool bIsSelectedAspackage )
{
	if( NULL == pclSheetDescription || NULL == pclSheetDescription->GetSSheetPointer() || NULL == pclAccessoryList || 0 == pclAccessoryList->GetCount() )
	{
		return lRow;
	}

	int iCount = pclAccessoryList->GetCount();

	CRank rkList;
	CAccessoryList::AccessoryItem rAccessoryItem = pclAccessoryList->GetFirst();

	while( NULL != rAccessoryItem.IDPtr.MP )
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
		rkList.AddStrSort( pclAccessory->GetName(), 0, (LPARAM)pclAccessoryItem, false, false );
		rAccessoryItem = pclAccessoryList->GetNext();
	}

	CString str;
	LPARAM lParam;

	for( BOOL bContinue = rkList.GetFirst( str, lParam ); TRUE == bContinue; bContinue = rkList.GetNext( str, lParam ) )
	{
		CAccessoryList::AccessoryItem *pclAccessoryItem = (CAccessoryList::AccessoryItem *)lParam;
		CDB_Product *pclAccessory = dynamic_cast<CDB_Product *>( (CData *)( pclAccessoryItem->IDPtr.MP ) );
		bool bByPair = pclAccessoryItem->fByPair;
			
		if( NULL == pclAccessory || false == pclAccessory->IsAnAccessory() )
		{
			continue;
		}

		int iQuantity = iGlobalQuantity;

		// HYS-987: If the accessory quantity is changed take the new value.
		if( -1 != pclAccessoryItem->lEditedQty )
		{
			iQuantity = pclAccessoryItem->lEditedQty;
		}
		else if( true == bByPair )
		{
			iQuantity *= 2;
		}

		delete pclAccessoryItem;

		lRow = FillAccessory( pclSheetDescription, lRow, pclAccessory, !bIsSelectedAspackage, iQuantity );
		lRow++;
	}

	return pclSheetDescription->GetSSheetPointer()->GetMaxRows();
}

long CSelProdPage6WayValve::_FillActuator( CSheetDescription *pclSheetDescription, long lRow, CDB_Actuator *pclActuator, CDS_SSelCtrl *pclSSelControlValve, 
		int iGlobalQuantity, bool bShowFailSafeDRP )
{
	if( NULL == pclSheetDescription || NULL == pclSheetDescription->GetSSheetPointer() || NULL == pclActuator 
			|| NULL == dynamic_cast<CDB_ElectroActuator *>( pclActuator ) || NULL == pclSSelControlValve )
	{
		return lRow;
	}

	// Retrieve control valve
	CDB_ControlValve* pclControlValve = pclSSelControlValve->GetProductAs<CDB_ControlValve>();
	if( NULL == pclControlValve )
	{
		ASSERTA_RETURN( lRow );
	}

	CSSheet *pclSheet = pclSheetDescription->GetSSheetPointer();
	CDB_ElectroActuator *pclElectroActuator = dynamic_cast<CDB_ElectroActuator *>( pclActuator );

	long lFirstRow = lRow;

	CString str;
	pclSheet->SetTextPattern( CSSheet::_SSTextPattern::UserStaticColumn );
	pclSheet->SetTextPatternProperty( CSSheet::_SSTextPatternProperty::TPP_FontBold, (LPARAM)FALSE );
		
	// Set font color to red when accessory is not deleted or not available.
	if( true == pclElectroActuator->IsDeleted() || false == pclElectroActuator->IsAvailable() )
	{
		pclSheet->SetTextPatternProperty( CSSheet::_SSTextPatternProperty::TPP_FontForeColor, (LPARAM)_RED );
	}

	// Check first if we have a local article number available.
	CString strLocalArticleNumber = _T("");

	// HYS-1877: Do not display Set article number when working on a set.
	if( true == pclSSelControlValve->IsSelectedAsAPackage( true ) )
	{
		AddStaticText( pclSheetDescription, ColumnDescription::ArticleNumber, lRow, _T( "-" ) );
	}
	else
	{
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
	FillQtyPriceCol( pclSheetDescription, lRow, pclElectroActuator, iGlobalQuantity, true );
		
	// Write the actuator name.
	AddStaticText( pclSheetDescription, ColumnDescription::Product, lRow, pclElectroActuator->GetName() );
	pclSheet->SetFontBold( ColumnDescription::Product, lRow, TRUE );
	AddCellSpanW( pclSheetDescription, ColumnDescription::Product, lRow++, 2, 1 );
	pclSheet->SetTextPatternProperty( CSSheet::_SSTextPatternProperty::TPP_StyleHorizontalAlign, (LPARAM)SSS_ALIGN_LEFT );

	// Set the close off value if exist.
	CDB_CloseOffChar *pCloseOffChar = (CDB_CloseOffChar *)( pclControlValve->GetCloseOffCharIDPtr().MP );

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

	// Compute actuating time in sec.
	str = TASApp.LoadLocalizedString( IDS_SSHEETSELPROD_ACTUATING_TIME ) + CString( _T(" : ") ) + pclElectroActuator->GetActuatingTimesStr( pclControlValve->GetStroke(), true );
	AddStaticText( pclSheetDescription, ColumnDescription::Product, lRow, str );
	AddCellSpanW( pclSheetDescription, ColumnDescription::Product, lRow++, 2, 1 );

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

	if( true == bShowFailSafeDRP )
	{
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
		str = TASApp.LoadLocalizedString( IDS_SSHEETSSEL_ACTRDRP );

		if( CString( _T("") ) == pclElectroActuator->GetDefaultReturnPosStr( pclElectroActuator->GetDefaultReturnPos() ).c_str() )
		{
			str += CString( _T(" :  -") );
		}
		else
		{
			str += CString( _T(" : ") ) + pclElectroActuator->GetDefaultReturnPosStr( pclElectroActuator->GetDefaultReturnPos() ).c_str();
		}

		AddStaticText( pclSheetDescription, ColumnDescription::Product, lRow, str );
		AddCellSpanW( pclSheetDescription, ColumnDescription::Product, lRow++, 2, 1 );
	}
	
	// HYS-726: Show max temp info in result tab.
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

	return pclSheet->GetMaxRows();
}

long CSelProdPage6WayValve::_FillAdapterFromHM( CSheetDescription *pclSheetDescription, long lRow, CDS_HydroMod::C6WayValve *pclHM6WayValve )
{
	if( NULL == pclSheetDescription || NULL == pclSheetDescription->GetSSheetPointer() || NULL == pclHM6WayValve )
	{
		return lRow;
	}

	CDB_Product *pclAdapter = dynamic_cast<CDB_Product*>( (CData*)pclHM6WayValve->Get6WayValveActuatorAdapterIDPtr().MP );

	if( NULL == pclAdapter || false == pclAdapter->IsAnAccessory() )
	{
		return lRow;
	}

	lRow = FillAccessory( pclSheetDescription, lRow, pclAdapter, true, 1 );

	return pclSheetDescription->GetSSheetPointer()->GetMaxRows();
}

long CSelProdPage6WayValve::_FillPIBCValve( CSheetDescription *pclSheetDescription, long lRow, CDS_SSel6WayValve *pclSSel6WayValve, SideDefinition eSideDefinition )
{
	if( NULL == pclSheetDescription || NULL == pclSheetDescription->GetSSheetPointer() || NULL == pclSSel6WayValve
			|| NULL == pclSSel6WayValve->GetCDSSSelPICv( eSideDefinition ) )
	{
		return lRow;
	}

	CDS_SSelPICv *pclSSelPIBCValve = pclSSel6WayValve->GetCDSSSelPICv( eSideDefinition );

	if( NULL == pclSSelPIBCValve->GetProductAs<CDB_PIControlValve>() )
	{
		ASSERTA_RETURN( lRow );
	}
	
	CDB_PIControlValve *pclPIBCValve = pclSSelPIBCValve->GetProductAs<CDB_PIControlValve>();
	CSSheet *pclSheet = pclSheetDescription->GetSSheetPointer();

	// Remove previous bottom border.
	pclSheet->SetCellBorder( ColumnDescription::Reference1, lRow, ColumnDescription::Footer - 1, lRow, false, SS_BORDERTYPE_BOTTOM );

	lRow++;
	long lFirstNoBreakableRow = lRow;
	SetLastRow( pclSheetDescription, lRow );

	CString strTitle;

	if( BothSide == eSideDefinition )
	{
		strTitle = TASApp.LoadLocalizedString( IDS_SSHEETSELPROD_SUBTITLE6WAYVALVEPIBCV );
	}
	else if( CoolingSide == eSideDefinition || HeatingSide == eSideDefinition )
	{
		CString strSide = TASApp.LoadLocalizedString( ( CoolingSide == eSideDefinition ) ? IDS_SSHEETSELPROD_6WAYVALVECOOLINGSIDE : IDS_SSHEETSELPROD_6WAYVALVEHEATINGSIDE );
		FormatString( strTitle, IDS_SSHEETSELPROD_SUBTITLE6WAYVALVEPIBCVPARAM, strSide );
	}
	
	COLORREF clTextColor = 0;
	COLORREF clBackgroundColor = 0;
	double dRowSize = 0.0;
	int iFontSize = 0;
	_GetGroupColorAndSizes( false, eSideDefinition, clTextColor, clBackgroundColor, dRowSize, iFontSize );

	SetPageTitle( pclSheetDescription, strTitle, false, lRow, dRowSize, clTextColor, clBackgroundColor, iFontSize );

	// Draw border around the title.
	pclSheet->SetCellBorder( 2, lRow, m_mapSheetInfos[pclSheetDescription].m_iColNum - 2, lRow, true, SS_BORDERTYPE_OUTLINE, SS_BORDERSTYLE_SOLID, clTextColor );

	lRow++;
	long lFirstRow = lRow;

	CString str;
	pclSheet->SetTextPattern( CSSheet::_SSTextPattern::UserStaticColumn );
	pclSheet->SetTextPatternProperty( CSSheet::_SSTextPatternProperty::TPP_FontBold, (LPARAM)FALSE );

	// Remark: 'true' to exclude any connection reference in the article number.
	// HYS-1877: Do not display PIBCV article number whe it belong to a set.
	CString strArticleNumber = CteEMPTY_STRING;
	if( true == pclSSelPIBCValve->IsSelectedAsAPackage( true ) )
	{
		AddStaticText( pclSheetDescription, ColumnDescription::ArticleNumber, lRow, _T( "-" ) );
		FillQtyPriceCol( pclSheetDescription, lRow, pclPIBCValve, pclSSel6WayValve->GetpSelectedInfos()->GetQuantity() );
	}
	else
	{
		strArticleNumber = pclPIBCValve->GetArtNum( true );
		FillArtNumberCol( pclSheetDescription, ColumnDescription::ArticleNumber, lRow, pclPIBCValve, strArticleNumber );

		// Remark: it's intentional that we get quantity from 'pclSSel6WayValve'. This is in this variable that we have the global quantity.
		FillQtyPriceCol( pclSheetDescription, lRow, pclPIBCValve, pclSSel6WayValve->GetpSelectedInfos()->GetQuantity() );
	}

	// If it's directly selected don't fill water characteristic and pipe info.
	if( false == pclSSel6WayValve->IsFromDirSel() 
			&& ( e6Way_OnOffControlWithPIBCV == pclSSel6WayValve->GetSelectionMode() || e6Way_OnOffControlWithSTAD == pclSSel6WayValve->GetSelectionMode() ) )
	{
		// Column water char.
		long lPrev = pclSheet->GetTextPatternProperty( CSSheet::_SSTextPatternProperty::TPP_StyleWordWrap );
		pclSheet->SetTextPatternProperty( CSSheet::_SSTextPatternProperty::TPP_StyleWordWrap, (LPARAM)TRUE );

		CString str1, str2;

		// Fluid name.
		pclSSel6WayValve->GetWC( eSideDefinition ).BuildWaterNameString( str1 );

		// Temperatures.
		str2 = TASApp.LoadLocalizedString( ( HeatingSide == eSideDefinition ) ? IDS_SSHEETSELPROD_6WAYVALVEHEATINGTEMP : IDS_SSHEETSELPROD_6WAYVALVECOOLINGTEMP ) + _T(": ");
		str2 += WriteCUDouble( _U_TEMPERATURE, pclSSel6WayValve->GetWC( eSideDefinition ).GetTemp(), false ) + CString( _T(" / ") );

		double dReturnTemp = pclSSel6WayValve->GetWC( eSideDefinition ).GetTemp();

		if( CoolingSide == eSideDefinition )
		{
			dReturnTemp += pclSSel6WayValve->GetpSelectedInfos()->GetCoolingDT();
		}
		else
		{
			dReturnTemp -= pclSSel6WayValve->GetpSelectedInfos()->GetHeatingDT();
		}

		str2 += WriteCUDouble( _U_TEMPERATURE, dReturnTemp, true );
	
		CString strFinal = str1 + _T("\n") + str2;
		AddStaticText( pclSheetDescription, ColumnDescription::Water, lRow, strFinal );

		lRow += 3;
		pclSheet->SetTextPatternProperty( CSSheet::_SSTextPatternProperty::TPP_StyleWordWrap, (LPARAM)lPrev );

		// Build the pipe string according to page setup 
		// Quid of LinDpflag and Uflag ??? Mark in red and italic???
		// SetTextRC(row,7,...);
		CDB_Pipe *pPipe = pclSSelPIBCValve->GetpSelPipe()->GetpPipe();
		lRow = lFirstRow;

		if( NULL != pPipe )
		{
			pclSheet->SetTextPatternProperty( CSSheet::_SSTextPatternProperty::TPP_StyleWordWrap, ( LPARAM )TRUE );

			CTable *pPipeTab = dynamic_cast<CTable *> (pPipe->GetIDPtr().PP);
			ASSERT( NULL != pPipeTab );

			if( NULL != pPipeTab && true == m_pTADSPageSetup->GetField( PageField_enum::epfPIPENAME ) )
			{
				lRow = AddStaticText( pclSheetDescription, ColumnDescription::Pipes, lRow, pPipeTab->GetName(), true );
			}

			if( true == m_pTADSPageSetup->GetField( PageField_enum::epfPIPESIZE ) )
			{
				lRow = AddStaticText( pclSheetDescription, ColumnDescription::Pipes, lRow, pPipe->GetName() );
			}
			
			if( true == m_pTADSPageSetup->GetField( PageField_enum::epfPIPEWATERU ) )
			{
				if( pclSSelPIBCValve->GetpSelPipe()->GetU() != 0.0 )
				{
					lRow = AddStaticText( pclSheetDescription, ColumnDescription::Pipes, lRow, WriteCUDouble( _U_VELOCITY, pclSSelPIBCValve->GetpSelPipe()->GetU(), true ) );
				}
			}

			if( true == m_pTADSPageSetup->GetField( PageField_enum::epfPIPEDP ) )
			{
				if( pclSSelPIBCValve->GetpSelPipe()->GetLinDp() != 0.0 )
				{
					lRow = AddStaticText( pclSheetDescription, ColumnDescription::Pipes, lRow, WriteCUDouble( _U_LINPRESSDROP, pclSSelPIBCValve->GetpSelPipe()->GetLinDp(), true ) );
				}
			}
		}
		else
		{
			lRow = AddStaticText( pclSheetDescription, ColumnDescription::Pipes, lRow, _T("-") );
		}
	}
	
	// Column 'TA Product'.
	lRow = lFirstRow;
	long lRowProduct = _FillPIBCValveTAP( pclSheetDescription, lRow, pclPIBCValve );

	// Column 'Infos'.
	lRow = lFirstRow;
	AddStaticText( pclSheetDescription, ColumnDescription::TechnicalInfos, lRow, _T("-") );

	// Flow.
	// If Cv is directly selected don't fill Water, pipes.
	// HYS-1380: Display Dp min. cooling and heating
	if( false == pclSSel6WayValve->IsFromDirSel() && ( e6Way_EQMControl == pclSSel6WayValve->GetSelectionMode() ) )
	{
		double dCoolingFlow = pclSSel6WayValve->GetFlow( CoolingSide );
		double dHeatingFlow = pclSSel6WayValve->GetFlow( HeatingSide );
		double dRhoCooling = pclSSel6WayValve->GetWC( CoolingSide ).GetDens();
		double dRhoHeating = pclSSel6WayValve->GetWC( HeatingSide ).GetDens();
		double dDpMinCooling = pclPIBCValve->GetDpmin( dCoolingFlow, dRhoCooling );
		double dDpMinHeating = pclPIBCValve->GetDpmin( dHeatingFlow, dRhoHeating );
		
		if( true == m_pTADSPageSetup->GetField( epfCVTECHINFODP ) )
		{
			if( -1.0 != dDpMinCooling )
			{
				str = TASApp.LoadLocalizedString( IDS_SSHETSELPROD_6WAYVALVE_DPMINCOOLING );
				str += (CString)_T( " = " );
				str += WriteCUDouble( _U_DIFFPRESS, dDpMinCooling, true );
			}
			else
				str = GetDashDotDash();

			lRow = AddStaticText( pclSheetDescription, ColumnDescription::TechnicalInfos, lRow, str );

			if( -1.0 != dDpMinHeating )
			{
				str = TASApp.LoadLocalizedString( IDS_SSHETSELPROD_6WAYVALVE_DPMINHEATING );
				str += (CString)_T( " = " );
				str += WriteCUDouble( _U_DIFFPRESS, dDpMinHeating, true );
			}
			else
				str = GetDashDotDash();

			lRow = AddStaticText( pclSheetDescription, ColumnDescription::TechnicalInfos, lRow, str );
		}
	}
	else if( false == pclSSel6WayValve->IsFromDirSel() )
	{
		// We are not in EQM control mode
		double dFlow = pclSSel6WayValve->GetFlow( eSideDefinition );
		ASSERT( dFlow > 0.0 );

		if( true == m_pTADSPageSetup->GetField( epfCVTECHINFOFLOW ) && 0.0 != dFlow )
		{
			// HYS-38: Show power dt info when their radio button is checked
			CString strPowerDt = _T( "" );

			if( CDS_SelProd::efdPower == pclSSel6WayValve->GetFlowDef() )
			{
				strPowerDt = WriteCUDouble( _U_TH_POWER, pclSSel6WayValve->GetPower( eSideDefinition ), true );
				strPowerDt += _T(" / ");
				strPowerDt += WriteCUDouble( _U_DIFFTEMP, pclSSel6WayValve->GetDT( eSideDefinition ), true );
			}

			if( strPowerDt != _T( "" ) )
			{
				CString strQ = _T("(");
				strQ += WriteCUDouble( _U_FLOW, dFlow, true );
				strQ += _T(")");
				lRow = AddStaticText( pclSheetDescription, ColumnDescription::TechnicalInfos, lRow, strPowerDt );
				lRow = AddStaticText( pclSheetDescription, ColumnDescription::TechnicalInfos, lRow, strQ );
			}
			else
			{
				lRow = AddStaticText( pclSheetDescription, ColumnDescription::TechnicalInfos, lRow, WriteCUDouble( _U_FLOW, dFlow, true ) );
			}
		}

		// DpMin 
		double dRho = pclSSel6WayValve->GetWC( eSideDefinition ).GetDens();
		if( true == m_pTADSPageSetup->GetField( epfCVTECHINFODP ) )
		{
			double dDpMin = pclPIBCValve->GetDpmin( dFlow, dRho );
			if( -1.0 != dDpMin )
			{
				str = TASApp.LoadLocalizedString( IDS_DPMIN );
				str += (CString)_T( " = " );
				str += WriteCUDouble( _U_DIFFPRESS, dDpMin, true );
			}
			else
				str = GetDashDotDash();

			lRow = AddStaticText( pclSheetDescription, ColumnDescription::TechnicalInfos, lRow, str );
		}
		// Presetting.
		CDB_ValveCharacteristic *pValveCharacteristic = pclPIBCValve->GetValveCharacteristic();

		if( true == m_pTADSPageSetup->GetField( epfCVTECHINFOSETTING ) 
			&& NULL != pValveCharacteristic && e6Way_OnOffControlWithPIBCV == pclSSel6WayValve->GetSelectionMode() )
		{
			str = TASApp.LoadLocalizedString( IDS_SSHETSELPROD_6WAYVALVE_PIBCVSETTING );
			double dPresetting = pclPIBCValve->GetPresetting( pclSSel6WayValve->GetFlow( eSideDefinition ), pclSSel6WayValve->GetWC( eSideDefinition ).GetDens(),
					pclSSel6WayValve->GetWC( eSideDefinition ).GetKinVisc() );

			if( -1.0 != dPresetting )
			{
				str += _T(" ") + pValveCharacteristic->GetSettingString( dPresetting, true );
			}
			else
			{
				str += _T(" -");
			}

			lRow = AddStaticText( pclSheetDescription, ColumnDescription::TechnicalInfos, lRow, str );
		}
	}
	
	m_rProductParam.AddRange( lFirstRow, pclSheet->GetMaxRows(), pclPIBCValve );
	
	lRow = pclSheet->GetMaxRows();

	// Spanning must be done here because it's the only place where we know exactly number of lines to span!
	// Span reference #1 and #2.
	AddCellSpanW( pclSheetDescription, ColumnDescription::Reference1, lFirstRow, 1, pclSheet->GetMaxRows() - lFirstRow + 1 );
	AddCellSpanW( pclSheetDescription, ColumnDescription::Reference2, lFirstRow, 1, pclSheet->GetMaxRows() - lFirstRow + 1 );
	
	// Span water characteristic.
	AddCellSpanW( pclSheetDescription, ColumnDescription::Water, lFirstRow, 1, pclSheet->GetMaxRows() - lFirstRow + 1 );
					
	// Add accessories.
	if( true == pclSSelPIBCValve->IsAccessoryExist() )
	{
		// Draw dash line.
		pclSheet->SetCellBorder( ColumnDescription::Reference1, lRow, ColumnDescription::Footer - 1, lRow, true, SS_BORDERTYPE_BOTTOM, SS_BORDERSTYLE_FINE_DASH );
					
		lRow++;

		// Remark: it's intentional that we get quantity from 'pclSSel6WayValve'. This is in this variable that we have the global quantity.
		// HYS-1877
		lRow = _FillAccessories( pclSheetDescription, lRow, pclSSelPIBCValve->GetAccessoryList(), pclSSel6WayValve->GetpSelectedInfos()->GetQuantity(),
								 pclSSelPIBCValve->IsSelectedAsAPackage( true ) );
	}

	// Set the PIBCV and its accessories as no breakable (for print).
	pclSheet->SetFlagRowNoBreakable( lFirstNoBreakableRow, lRow, true );

	bool bActuatorExist = ( NULL != dynamic_cast<CDB_ElectroActuator *>( pclSSelPIBCValve->GetActrIDPtr().MP ) ) ? true : false;

	// Fill actuator if exist.
	if( true == bActuatorExist )
	{
		// Draw dash line.
		pclSheet->SetCellBorder( ColumnDescription::Reference1, lRow, ColumnDescription::Footer - 1, lRow, true, SS_BORDERTYPE_BOTTOM, SS_BORDERSTYLE_FINE_DASH );

		lRow++;
		lFirstNoBreakableRow = lRow;
		CDB_ElectroActuator *pclActuator = dynamic_cast<CDB_ElectroActuator *>( pclSSelPIBCValve->GetActrIDPtr().MP );

		// Remark: it's intentional that we get quantity from 'pclSSel6WayValve'. This is in this variable that we have the global quantity.
		// HYS-1877
		lRow = _FillActuator( pclSheetDescription, lRow, pclActuator, pclSSelPIBCValve, pclSSel6WayValve->GetpSelectedInfos()->GetQuantity() );
				
		if( true == pclSSelPIBCValve->IsActuatorAccessoryExist() )
		{
			// Draw dash line.
			pclSheet->SetCellBorder( ColumnDescription::Reference1, lRow, ColumnDescription::Footer - 1, lRow, true, SS_BORDERTYPE_BOTTOM, SS_BORDERSTYLE_FINE_DASH );

			lRow++;

			// Remark: it's intentional that we get quantity from 'pclSSel6WayValve'. This is in this variable that we have the global quantity.
			// HYS-1877
			lRow = _FillAccessories( pclSheetDescription, lRow, pclSSelPIBCValve->GetActuatorAccessoryList(), pclSSel6WayValve->GetpSelectedInfos()->GetQuantity(),
									 pclSSelPIBCValve->IsSelectedAsAPackage( true ) );
		}

		// Set the group as no breakable (for print).
		pclSheet->SetFlagRowNoBreakable( lFirstNoBreakableRow, lRow, true );
	}
				
	// Draw dash line.
	pclSheet->SetCellBorder( ColumnDescription::Reference1, lRow, ColumnDescription::Footer - 1, lRow, true, SS_BORDERTYPE_BOTTOM, SS_BORDERSTYLE_FINE_DASH );
	return pclSheet->GetMaxRows();
}

long CSelProdPage6WayValve::_FillPIBCValveTAP( CSheetDescription *pclSheetDescription, long lRow, CDB_TAProduct *pTAP )
{
	if( NULL == pclSheetDescription || NULL == pclSheetDescription->GetSSheetPointer() || NULL == pTAP
			|| NULL == dynamic_cast<CDB_PIControlValve *>( pTAP ) )
	{
		return lRow;
	}

	CSSheet *pclSSheet = pclSheetDescription->GetSSheetPointer();
	CString str;

	// Column 'Product'.
	AddStaticText( pclSheetDescription, ColumnDescription::Product, lRow, _T("-") );
	
	if( true == m_pTADSPageSetup->GetField( epf6WAYVALVEINFOPRODUCTNAME ) )
	{
		lRow = AddStaticText( pclSheetDescription, ColumnDescription::Product, lRow, pTAP->GetName() );
		pclSSheet->SetFontBold( ColumnDescription::Product, lRow - 1, TRUE );
	}

	if( true == m_pTADSPageSetup->GetField( epf6WAYVALVEINFOPRODUCTKVS ) )
	{
		CDB_PIControlValve *pclPIBCValve = dynamic_cast<CDB_PIControlValve *>( pTAP );

		if( 0 == m_pUnitDB->GetDefaultUnitIndex( _C_KVCVCOEFF ) )
		{
			str = TASApp.LoadLocalizedString( IDS_KVS );
		}
		else
		{
			str = TASApp.LoadLocalizedString( IDS_CV );
		}

		str += CString( _T(" = ") ) + WriteCUDouble( _C_KVCVCOEFF, pclPIBCValve->GetKvs() );
		lRow = AddStaticText( pclSheetDescription, ColumnDescription::Product, lRow, str );
	}

	// Body Material.
	if( true == m_pTADSPageSetup->GetField( epf6WAYVALVEINFOPRODUCTBDYMATERIAL ) )
	{
		CDB_StringID *pStrID = dynamic_cast <CDB_StringID *>( pTAP->GetBodyMaterialIDPtr().MP );

		if( NULL != pStrID )
		{
			lRow = AddStaticText( pclSheetDescription, ColumnDescription::Product, lRow,pStrID->GetString() );
		}
	}
	
	// Connection.
	if( true == m_pTADSPageSetup->GetField( epf6WAYVALVEINFOPRODUCTCONNECTION ) )
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
	if( true == m_pTADSPageSetup->GetField( epf6WAYVALVEINFOPRODUCTVERSION ) )
	{
		CDB_StringID *pStrID = dynamic_cast <CDB_StringID *>( pTAP->GetVersionIDPtr().MP );

		if( NULL != pStrID )
		{
			lRow = AddStaticText( pclSheetDescription, ColumnDescription::Product, lRow, pStrID->GetString() );
		}
	}

	// PN.
	if( true == m_pTADSPageSetup->GetField( epf6WAYVALVEINFOPRODUCTPN ) )
	{
		lRow = FillPNTminTmax( pclSheetDescription, ColumnDescription::Product, lRow, pTAP );
	}

	return pclSheetDescription->GetSSheetPointer()->GetMaxRows();
}

long CSelProdPage6WayValve::_FillBalancingValve( CSheetDescription *pclSheetDescription, long lRow, CDS_SSel6WayValve *pclSSel6WayValve, SideDefinition eSideDefinition )
{
	if( NULL == pclSheetDescription || NULL == pclSheetDescription->GetSSheetPointer() || NULL == pclSSel6WayValve
			|| NULL == pclSSel6WayValve->GetCDSSSelBv( eSideDefinition ) )
	{
		return lRow;
	}

	CDS_SSelBv *pclSSelBalancingValve = pclSSel6WayValve->GetCDSSSelBv( eSideDefinition );

	if( NULL == pclSSelBalancingValve->GetProductAs<CDB_RegulatingValve>() )
	{
		ASSERTA_RETURN( lRow );
	}
	
	CDB_RegulatingValve *pclBalancingValve = pclSSelBalancingValve->GetProductAs<CDB_RegulatingValve>();
	CSSheet *pclSheet = pclSheetDescription->GetSSheetPointer();

	// Remove previous bottom border.
	pclSheet->SetCellBorder( ColumnDescription::Reference1, lRow, ColumnDescription::Footer - 1, lRow, false, SS_BORDERTYPE_BOTTOM );

	lRow++;
	long lFirstNoBreakableRow = lRow;
	SetLastRow( pclSheetDescription, lRow );

	CString strTitle;

	if( BothSide == eSideDefinition )
	{
		strTitle = TASApp.LoadLocalizedString( IDS_SSHEETSELPROD_SUBTITLE6WAYVALVEBV );
	}
	else if( CoolingSide == eSideDefinition || HeatingSide == eSideDefinition )
	{
		CString strSide = TASApp.LoadLocalizedString( ( CoolingSide == eSideDefinition ) ? IDS_SSHEETSELPROD_6WAYVALVECOOLINGSIDE : IDS_SSHEETSELPROD_6WAYVALVEHEATINGSIDE );
		FormatString( strTitle, IDS_SSHEETSELPROD_SUBTITLE6WAYVALVEBVPARAM, strSide );
	}
	
	COLORREF clTextColor = 0;
	COLORREF clBackgroundColor = 0;
	double dRowSize = 0.0;
	int iFontSize = 0;
	_GetGroupColorAndSizes( false, eSideDefinition, clTextColor, clBackgroundColor, dRowSize, iFontSize );
	
	SetPageTitle( pclSheetDescription, strTitle, false, lRow, dRowSize, clTextColor, clBackgroundColor, iFontSize );

	// Draw border around the title.
	pclSheet->SetCellBorder( 2, lRow, m_mapSheetInfos[pclSheetDescription].m_iColNum - 2, lRow, true, SS_BORDERTYPE_OUTLINE, SS_BORDERSTYLE_SOLID, clTextColor );

	++lRow;
	long lFirstRow = lRow;
	CString str1, str2;

	pclSheet->SetTextPattern( CSSheet::_SSTextPattern::UserStaticColumn );
	pclSheet->SetTextPatternProperty( CSSheet::_SSTextPatternProperty::TPP_FontBold, (LPARAM)FALSE );
		
	// Column TA Product.
	AddStaticText( pclSheetDescription, ColumnDescription::Product, lRow, _T("-") );

	if( true == m_pTADSPageSetup->GetField( epfBVINFOPRODUCTNAME ) )	
	{
		lRow = AddStaticText( pclSheetDescription, ColumnDescription::Product, lRow, pclBalancingValve->GetName() );
		pclSheet->SetFontBold( ColumnDescription::Product, lRow - 1, TRUE );
	}

	if( true == m_pTADSPageSetup->GetField( epfBVINFOPRODUCTSIZE ) )	
	{
		lRow = AddStaticText( pclSheetDescription, ColumnDescription::Product, lRow, pclBalancingValve->GetSize() );
	}

	if( true == m_pTADSPageSetup->GetField( epfBVINFOPRODUCTCONNECTION ) )	
	{
		lRow = AddStaticText( pclSheetDescription, ColumnDescription::Product, lRow, ( (CDB_StringID *)( pclBalancingValve->GetConnectIDPtr().MP ) )->GetString() );
	}

	if( true == m_pTADSPageSetup->GetField( epfBVINFOPRODUCTVERSION ) )	
	{
		lRow = AddStaticText( pclSheetDescription, ColumnDescription::Product, lRow, ( (CDB_StringID *)( pclBalancingValve->GetVersionIDPtr().MP ) )->GetString() );
	}

	if( true == m_pTADSPageSetup->GetField( epfBVINFOPRODUCTPN ) )	
	{
		lRow = FillPNTminTmax( pclSheetDescription, ColumnDescription::Product, lRow, pclBalancingValve );
	}
	
	// Column 'Quantity' and prices.
	// Remark: it's intentional that we get quantity from 'pclSSel6WayValve'. This is in this variable that we have the global quantity.
	FillQtyPriceCol( pclSheetDescription, lFirstRow, pclBalancingValve, pclSSel6WayValve->GetpSelectedInfos()->GetQuantity() );
	
	// Column 'Article number'.
	AddStaticText( pclSheetDescription, ColumnDescription::ArticleNumber, lFirstRow, _T("-") );

	// Remark: 'true' to exclude any connection reference in the article number.
	FillArtNumberCol( pclSheetDescription, ColumnDescription::ArticleNumber, lFirstRow, pclBalancingValve, pclBalancingValve->GetArtNum( true ) );

	// Column infos.
	if( false == pclSSel6WayValve->IsFromDirSel() )
	{
		lRow = lFirstRow;
		double dQ = pclSSel6WayValve->GetFlow( eSideDefinition );
		double dRho = pclSSel6WayValve->GetWC( eSideDefinition ).GetDens();
		double dNu = pclSSel6WayValve->GetWC( eSideDefinition ).GetKinVisc();
		double dPresset = pclSSelBalancingValve->GetOpening();
		
		// HYS-38: Show power dt info when their radio button is checked
		CString strPowerDt = _T( "" );
		
		if( CDS_SelProd::efdPower == pclSSel6WayValve->GetFlowDef() )
		{
			strPowerDt = WriteCUDouble( _U_TH_POWER, pclSSel6WayValve->GetPower( eSideDefinition ), true );
			strPowerDt += _T(" / ");
			strPowerDt += WriteCUDouble( _U_DIFFTEMP, pclSSel6WayValve->GetDT( eSideDefinition ), true );
		}
		
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
			double dDp = 0.0;

			if( true == pclBalancingValve->IsKvSignalEquipped() )
			{
				CDB_FixOCharacteristic *pChar = dynamic_cast<CDB_FixOCharacteristic *>( pclBalancingValve->GetValveCharDataPointer() );

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
				if( NULL != pclBalancingValve->GetValveCharacteristic() )
				{
					if( true == pclBalancingValve->GetValveCharacteristic()->GetValveDp( dQ, &dDp, dPresset, dRho, dNu ) )
					{
						lRow = AddStaticText( pclSheetDescription, ColumnDescription::TechnicalInfos, lRow , WriteCUDouble( _U_DIFFPRESS, dDp, true ) );
					}
				}
			}
		}

		if( true == m_pTADSPageSetup->GetField( epfBVTECHINFOSETTING ) )
		{	
			// Not for a fixed orifice valve.
			if( 0 != _tcscmp( pclBalancingValve->GetTypeID(), _T("RVTYPE_FO") ) )
			{
				str1 = _T("-");

				if( dPresset > 0.0 && NULL != pclBalancingValve->GetValveCharacteristic() )
				{
					str1 = pclBalancingValve->GetValveCharacteristic()->GetSettingString( dPresset, true );
				}

				lRow = AddStaticText( pclSheetDescription, ColumnDescription::TechnicalInfos, lRow, str1 );
			}
		}
	}

	lRow = lFirstRow;
	pclSheet->SetTextPattern( CSSheet::_SSTextPattern::UserStaticColumn );
	pclSheet->SetTextPatternProperty( CSSheet::_SSTextPatternProperty::TPP_FontBold, (LPARAM)FALSE );

	// No references here, all has been written for the 6-way valve

	// If it's directly selected don't fill Water, pipes.
	if( false == pclSSel6WayValve->IsFromDirSel() )
	{
		// Column water char.
		long lPrev = pclSheet->GetTextPatternProperty( CSSheet::_SSTextPatternProperty::TPP_StyleWordWrap );
		pclSheet->SetTextPatternProperty( CSSheet::_SSTextPatternProperty::TPP_StyleWordWrap, (LPARAM)TRUE );

		// Fluid name.
		pclSSel6WayValve->GetWC( eSideDefinition ).BuildWaterNameString( str1 );
	
		// Temperatures.
		str2 = TASApp.LoadLocalizedString( ( HeatingSide == eSideDefinition ) ? IDS_SSHEETSELPROD_6WAYVALVEHEATINGTEMP : IDS_SSHEETSELPROD_6WAYVALVECOOLINGTEMP ) + _T(": ");
		str2 += WriteCUDouble( _U_TEMPERATURE, pclSSel6WayValve->GetWC( eSideDefinition ).GetTemp(), false ) + CString( _T(" / ") );

		double dReturnTemp = pclSSel6WayValve->GetWC( eSideDefinition ).GetTemp();
		
		if( CoolingSide == eSideDefinition )
		{
			dReturnTemp += pclSSel6WayValve->GetpSelectedInfos()->GetCoolingDT();
		}
		else
		{
			dReturnTemp -= pclSSel6WayValve->GetpSelectedInfos()->GetHeatingDT();
		}

		pclSSel6WayValve->GetWC( eSideDefinition ).GetTemp();
		str2 += WriteCUDouble( _U_TEMPERATURE, dReturnTemp, true );

		CString strFinal = str1 + _T("\n") + str2;
		AddStaticText( pclSheetDescription, ColumnDescription::Water, lRow, strFinal );
		
		lRow += 3;
		pclSheet->SetTextPatternProperty( CSSheet::_SSTextPatternProperty::TPP_StyleWordWrap, (LPARAM)lPrev );

		//Build the pipe string according to page setup 
		//Quid of LinDpflag and Uflag ??? Mark in red and italic???
		//SetTextRC(row,7,...);
		CDB_Pipe *pPipe = pclSSelBalancingValve->GetpSelPipe()->GetpPipe();
		lRow = lFirstRow;

		if( NULL != pPipe )
		{
			pclSheet->SetTextPatternProperty( CSSheet::_SSTextPatternProperty::TPP_StyleWordWrap, ( LPARAM )TRUE );

			CTable *pPipeTab = dynamic_cast<CTable *> (pPipe->GetIDPtr().PP);
			ASSERT( NULL != pPipeTab );

			if( NULL != pPipeTab && true == m_pTADSPageSetup->GetField( PageField_enum::epfPIPENAME ) )
			{
				lRow = AddStaticText( pclSheetDescription, ColumnDescription::Pipes, lRow, pPipeTab->GetName(), true );
			}

			if( true == m_pTADSPageSetup->GetField( PageField_enum::epfPIPESIZE ) )
			{
				lRow = AddStaticText( pclSheetDescription, ColumnDescription::Pipes, lRow, pPipe->GetName() );
			}
			
			if( true == m_pTADSPageSetup->GetField( PageField_enum::epfPIPEWATERU ) )
			{
				if( pclSSelBalancingValve->GetpSelPipe()->GetU() != 0.0 )
				{
					lRow = AddStaticText( pclSheetDescription, ColumnDescription::Pipes, lRow, WriteCUDouble( _U_VELOCITY, pclSSelBalancingValve->GetpSelPipe()->GetU(), true ) );
				}
			}

			if( true == m_pTADSPageSetup->GetField( PageField_enum::epfPIPEDP ) )
			{
				if( pclSSelBalancingValve->GetpSelPipe()->GetLinDp() != 0.0 )
				{
					lRow = AddStaticText( pclSheetDescription, ColumnDescription::Pipes, lRow, WriteCUDouble( _U_LINPRESSDROP, pclSSelBalancingValve->GetpSelPipe()->GetLinDp(), true ) );
				}
			}
		}
		else
		{
			lRow = AddStaticText( pclSheetDescription, ColumnDescription::Pipes, lRow, _T("-") );
		}
	}

	// Column Remark.
	if( true == m_pTADSPageSetup->GetField( PageField_enum::epfREMARK ) )	
	{
		if( 0 != pclSSelBalancingValve->GetpSelectedInfos()->GetRemarkIndex() )		// remark exist
		{
			str1.Format( _T("%d"), pclSSelBalancingValve->GetpSelectedInfos()->GetRemarkIndex() );
			lRow = AddStaticText( pclSheetDescription, ColumnDescription::Remark, lFirstRow, str1 );
		}
	}

	lRow = pclSheet->GetMaxRows();

	// Add the selectable row range.
	m_rProductParam.AddRange( lFirstRow, lRow, pclBalancingValve );

	// Spanning must be done here because it's the only place where we know exactly number of lines to span!
	// Span reference #1 and #2.
	AddCellSpanW( pclSheetDescription, ColumnDescription::Reference1, lFirstRow, 1, lRow - lFirstRow + 1 );
	AddCellSpanW( pclSheetDescription, ColumnDescription::Reference2, lFirstRow, 1, lRow - lFirstRow + 1 );

	// Span water characteristic.
	AddCellSpanW( pclSheetDescription, ColumnDescription::Water, lFirstRow, 1, lRow - lFirstRow + 1 );

	// Add accessories.
	if( true == pclSSelBalancingValve->IsAccessoryExist() )
	{
		// Draw dash line.
		pclSheet->SetCellBorder( ColumnDescription::Reference1, lRow, ColumnDescription::Footer - 1, lRow, true, SS_BORDERTYPE_BOTTOM, SS_BORDERSTYLE_FINE_DASH );

		lRow++;

		// Remark: it's intentional that we get quantity from 'pclSSel6WayValve'. This is in this variable that we have the global quantity.
		lRow = FillAccessories( pclSheetDescription, lRow, pclSSelBalancingValve, pclSSel6WayValve->GetpSelectedInfos()->GetQuantity() );
	}

	// Draw line below.
	pclSheet->SetCellBorder( ColumnDescription::Reference1, lRow, ColumnDescription::Footer - 1, lRow, true, SS_BORDERTYPE_BOTTOM );

	// Set the BV and its accessories as no breakable (for print).
	pclSheet->SetFlagRowNoBreakable( lFirstNoBreakableRow, lRow, true );

	return pclSheet->GetMaxRows();
}

void CSelProdPage6WayValve::_AddArticleList( CDS_SSel6WayValve *pclSSel6WayValve )
{
	if( NULL == pclSSel6WayValve )
	{
		return;
	}

	// Add article number for the 6-way valve.
	CDB_6WayValve *pcl6WayValve = pclSSel6WayValve->GetProductAs<CDB_6WayValve>();
	bool fAddActuator = true;

	if( pcl6WayValve != NULL )
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
		// HYS-1877: If user has done a selection by set and if there is a CV-Actuator set, we display only set information.
		// The contain is not detailed.
		if( true == pclSSel6WayValve->IsSelectedAsAPackage( true ) )
		{
			CDB_Set* pCvActSet = dynamic_cast<CDB_Set*>(pclSSel6WayValve->GetCvActrSetIDPtr().MP);

			pclArticleItem->SetID( pCvActSet->GetIDPtr().ID );

			CString strArticleNumber = pCvActSet->GetReference();
			CString strLocArtNumber = LocArtNumTab.GetLocalArticleNumber( strArticleNumber );
			pclArticleItem->CheckThingAvailability( pCvActSet, strArticleNumber, strLocArtNumber );

			if( true == strLocArtNumber.IsEmpty() )
			{
				strLocArtNumber = _T( "-" );
			}

			pclArticleItem->SetArticle( strArticleNumber );
			pclArticleItem->SetLocArtNum( strLocArtNumber );

			str = pCvActSet->GetName() + CString( _T( " " ) ) + TASApp.LoadLocalizedString( IDS_SSHEETSELPROD_SET );
			pclArticleItem->SetDescription( str );
			pclArticleItem->SetQuantity( pclSSel6WayValve->GetpSelectedInfos()->GetQuantity() );

			pclArticleItem->SetPrice( TASApp.GetpTADB()->GetPrice( pCvActSet->GetArtNum( true ) ) );
			pclArticleItem->SetIsAvailable( pCvActSet->IsAvailable() );
			pclArticleItem->SetIsDeleted( pCvActSet->IsDeleted() );

			// Add control valve accessories.
			CArticleContainer* pclArticleContainer = pclArticleGroup->AddArticle( pclArticleItem );

			// Remark: 'AddArticle' will call new CArticleContainer( pclArticleItem ) that copies content of 'pclArticleItem' and not saves the pointer.
			delete pclArticleItem;

			AddAccessoriesInArticleContainer( pclSSel6WayValve->GetCvAccessoryList(), pclArticleContainer, pclSSel6WayValve->GetpSelectedInfos()->GetQuantity(), true );

			// Add actuator accessories except for those that are already included into the set.
			CAccessoryList clActAccessoriesToAdd;
			CDB_RuledTable* pAccTab = dynamic_cast<CDB_RuledTable*>(pCvActSet->GetAccGroupIDPtr().MP);
			CAccessoryList* pclActuatorAccessoryList = pclSSel6WayValve->GetActuatorAccessoryList();

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

			AddAccessoriesInArticleContainer( &clActAccessoriesToAdd, pclArticleContainer, pclSSel6WayValve->GetpSelectedInfos()->GetQuantity(), true );

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
			pclArticleItem->SetID( pcl6WayValve->GetIDPtr().ID );
			pclArticleItem->SetArticle( pcl6WayValve->GetBodyArtNum() );

			// Check if article is available and not deleted.
			CString strArticleNumber = pcl6WayValve->GetBodyArtNum();
			CString strLocArtNum = LocArtNumTab.GetLocalArticleNumber( strArticleNumber );
			pclArticleItem->CheckThingAvailability( pcl6WayValve, strArticleNumber, strLocArtNum );

			if( true == strLocArtNum.IsEmpty() )
			{
				strLocArtNum = _T("-");
			}

			pclArticleItem->SetArticle( strArticleNumber );
			pclArticleItem->SetLocArtNum( strLocArtNum );

			pclArticleItem->SetQuantity( pclSSel6WayValve->GetpSelectedInfos()->GetQuantity() );
			pclArticleItem->SetDescription( pcl6WayValve->GetName() + CString( _T("; ") ) );

			// Add Version.
			pclArticleItem->AddDescription( ( (CDB_StringID *)pcl6WayValve->GetVersionIDPtr().MP )->GetString() + CString( _T("; ") ) );

			// Add connection.
			str = ( (CDB_StringID *)pcl6WayValve->GetConnectIDPtr().MP )->GetString();
			str = str.Right( str.GetLength() - str.Find('/') - 1 );
			pclArticleItem->AddDescription( str + CString( _T("; ") ) );

			// Add PN.
			pclArticleItem->AddDescription( TASApp.LoadLocalizedString( IDS_PN ) + _T(" ") + pcl6WayValve->GetPN().c_str() );

			// Add Kvs.
			if( -1.0 != pcl6WayValve->GetKvs() )
			{
				str = CString( _T("; ") ) + GetKvCVString() + CString( _T(" = ") ) + WriteCUDouble( _C_KVCVCOEFF, pcl6WayValve->GetKvs() );
				pclArticleItem->AddDescription( str );
			}

			pclArticleItem->SetPrice( TASApp.GetpTADB()->GetPrice( pcl6WayValve->GetArtNum( true ) ) );
			pclArticleItem->SetIsAvailable( pcl6WayValve->IsAvailable() );
			pclArticleItem->SetIsDeleted( pcl6WayValve->IsDeleted() );

			// Add all Article Accessories.
			CArticleContainer *pclArticleContainer = pclArticleGroup->AddArticle( pclArticleItem );

			// Remark: 'AddArticle' will call new CArticleContainer( pclArticleItem ) that copies content of 'pclArticleItem' and not saves the pointer.
			delete pclArticleItem;

			AddAccessoriesInArticleContainer( pclSSel6WayValve->GetAccessoryList(), pclArticleContainer, pclSSel6WayValve->GetpSelectedInfos()->GetQuantity(), false );

			CSelProdPageBase::AddArticleInGroupList( pclArticleGroup );

			// Remark: pay attention that 'CPage::AddArticleInGroupList' will increment quantity if article already exist or
			//         will create a new 'CArticleGroup' and will copy to it data passed as argument. This is the reason
			//         why you need to delete 'pclArticlegroup' after the call.
			delete pclArticleGroup;

			// Add connection component if needed.
			if( true == pcl6WayValve->IsConnTabUsed() )
			{
				// Inlet.
				CSelProdPageBase::AddCompArtList( pcl6WayValve, pclSSel6WayValve->GetpSelectedInfos()->GetQuantity(), true );
				// Outlet.
				CSelProdPageBase::AddCompArtList( pcl6WayValve, pclSSel6WayValve->GetpSelectedInfos()->GetQuantity(), false );
			}
		}
	}

	// HYS-1877
	if( true == fAddActuator )
	{
		CDB_Actuator *pclActuator = dynamic_cast<CDB_Actuator*>( pclSSel6WayValve->GetActrIDPtr().MP );

		if( pclActuator != NULL )
		{
			_AddArticleList( pclActuator, pclSSel6WayValve->GetActuatorAccessoryList(), pclSSel6WayValve->GetpSelectedInfos()->GetQuantity() );
		}
	}

	// HYS-1877
	if( false == pclSSel6WayValve->IsSelectedAsAPackage( true ) )
	{
		switch( pclSSel6WayValve->GetSelectionMode() )
		{
		case e6Way_Alone:
			break;

		case e6Way_EQMControl:
			_AddArticleListPIBCvHelper( pclSSel6WayValve->GetCDSSSelPICv( BothSide ), pclSSel6WayValve->GetpSelectedInfos()->GetQuantity() );
			break;

		case e6Way_OnOffControlWithPIBCV:
			_AddArticleListPIBCvHelper( pclSSel6WayValve->GetCDSSSelPICv( HeatingSide ), pclSSel6WayValve->GetpSelectedInfos()->GetQuantity() );
			_AddArticleListPIBCvHelper( pclSSel6WayValve->GetCDSSSelPICv( CoolingSide ), pclSSel6WayValve->GetpSelectedInfos()->GetQuantity() );
			break;

		case e6Way_OnOffControlWithSTAD:
			_AddArticleListBvHelper( pclSSel6WayValve->GetCDSSSelBv( HeatingSide ), pclSSel6WayValve->GetpSelectedInfos()->GetQuantity() );
			_AddArticleListBvHelper( pclSSel6WayValve->GetCDSSSelBv( CoolingSide ), pclSSel6WayValve->GetpSelectedInfos()->GetQuantity() );
			break;
		}
	}
}

void CSelProdPage6WayValve::_AddArticleList( CDS_HydroMod::C6WayValve *pclHM6WayValve )
{
	if( NULL == pclHM6WayValve )
	{
		return;
	}

	CDB_6WayValve *pcl6WayValve = dynamic_cast<CDB_6WayValve *>( pclHM6WayValve->Get6WayValveIDPtr().MP ); 

	if( NULL == pcl6WayValve )
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

	CString str;
	pclArticleItem->SetID( pcl6WayValve->GetIDPtr().ID );
	pclArticleItem->SetArticle( pcl6WayValve->GetBodyArtNum() );

	// Check if article is available and not deleted.
	CString strArticleNumber = pcl6WayValve->GetBodyArtNum();
	CString strLocArtNum = LocArtNumTab.GetLocalArticleNumber( strArticleNumber );
	pclArticleItem->CheckThingAvailability( pcl6WayValve, strArticleNumber, strLocArtNum );
			
	if( true == strLocArtNum.IsEmpty() )
	{
		strLocArtNum = _T("-");
	}

	pclArticleItem->SetArticle( strArticleNumber );
	pclArticleItem->SetLocArtNum( strLocArtNum );

	pclArticleItem->SetQuantity( 1 );
	pclArticleItem->SetDescription( pcl6WayValve->GetName() + CString( _T("; ") ) );
			
	if( false == pcl6WayValve->IsaPICV() && -1.0 != pcl6WayValve->GetKvs() )
	{
		str += GetKvCVString() + CString( _T(" = ") ) + WriteCUDouble( _C_KVCVCOEFF, pcl6WayValve->GetKvs() );
		pclArticleItem->AddDescription( str + CString( _T("; ") ) );
	}
			
	pclArticleItem->AddDescription( ( (CDB_StringID *)pcl6WayValve->GetVersionIDPtr().MP )->GetString() );

	str = ( (CDB_StringID *)pcl6WayValve->GetConnectIDPtr().MP )->GetString();
	str = str.Right( str.GetLength() - str.Find('/') - 1 );
	pclArticleItem->AddDescription( CString( _T("; ") ) + str );

	pclArticleItem->SetPrice( TASApp.GetpTADB()->GetPrice( pcl6WayValve->GetArtNum( true ) ) );
	pclArticleItem->SetIsAvailable( pcl6WayValve->IsAvailable() );
	pclArticleItem->SetIsDeleted( pcl6WayValve->IsDeleted() );

	CArticleContainer *pclArticleContainer = pclArticleGroup->AddArticle( pclArticleItem );

	// Remark: 'AddArticle' will call new CArticleContainer( pclArticleItem ) that copies content of 'pclArticleItem' and not saves the pointer.
	delete pclArticleItem;

	// Add all article accessories.
	CDB_RuledTable *pclRuledTable = dynamic_cast<CDB_RuledTable *>( pcl6WayValve->GetAccessoriesGroupIDPtr().MP );

	if( NULL != pclRuledTable && pclHM6WayValve->Get6WayValveAccessoryCount() > 0 )
	{
		CAccessoryList clAccessoryList;

		for( int iLoop = 0; iLoop < pclHM6WayValve->Get6WayValveAccessoryCount(); iLoop++ )
		{
			CDB_Product *pclAccessory = (CDB_Product *)( pclHM6WayValve->Get6WayValveAccessoryIDPtr( iLoop ).MP );

			if( NULL == pclAccessory || false == pclAccessory->IsAnAccessory() )
			{
				continue;
			}

			clAccessoryList.Add( pclAccessory->GetIDPtr(), CAccessoryList::_AT_Accessory, pclRuledTable );
		}
		
		AddAccessoriesInArticleContainer( &clAccessoryList, pclArticleContainer, 1, false );
	}

	// Add 6-way valve adapters.
	if( NULL != pclHM6WayValve->Get6WayValveActuatorAdapterIDPtr().MP )
	{
		CAccessoryList clAccessoryList;
		clAccessoryList.Add( pclHM6WayValve->Get6WayValveActuatorAdapterIDPtr(), CAccessoryList::_AT_Adapter );
		AddAccessoriesInArticleContainer( &clAccessoryList, pclArticleContainer, 1, false );
	}

	CSelProdPageBase::AddArticleInGroupList( pclArticleGroup );

	// Remark: pay attention that 'CPage::AddArticleInGroupList' will increment quantity if article already exist or
	//         will create a new 'CArticleGroup' and will copy to it data passed as argument. This is the reason
	//         why you need to delete 'pclArticleGroup' after the call.
	delete pclArticleGroup;
			
	// Add connection component if needed.
	if( true == pcl6WayValve->IsConnTabUsed() )
	{
		// Inlet
		CSelProdPageBase::AddCompArtList( pcl6WayValve, 1, true );
		// Outlet
		CSelProdPageBase::AddCompArtList( pcl6WayValve, 1, false );
	}

	CDB_Actuator *pclActuator = dynamic_cast<CDB_Actuator*>( pclHM6WayValve->Get6WayValveActuatorIDPtr().MP );

	if( pclActuator != NULL )
	{
		_AddArticleList( pclActuator, pclHM6WayValve->Getp6WayValveActuatorAccessoryIDPtrArray(), 1 );
	}
}

void CSelProdPage6WayValve::_AddArticleList( CDB_Actuator *pclActuator, CAccessoryList *pclAccessoryList, int iQuantity )
{
	// At now we accept only electric and thermoelectric actuator.
	CDB_ElectroActuator *pclElectroActuator = dynamic_cast<CDB_ElectroActuator *>( pclActuator );

	if( NULL == pclElectroActuator )
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
	pclArticleItem->SetID( pclElectroActuator->GetIDPtr().ID );
	
	pclArticleItem->SetDescription( pclElectroActuator->GetName() + CString( _T("; ") ) );

	if( CDB_CloseOffChar::Linear == pclElectroActuator->GetOpeningType() )
	{
		pclArticleItem->AddDescription( TASApp.LoadLocalizedString( IDS_SSHEETSELPROD_ACTFORCE ) + CString( _T(" = ") ) );
	}
	else
	{
		pclArticleItem->AddDescription( TASApp.LoadLocalizedString( IDS_SSHEETSELPROD_ACTTORQUE ) + CString( _T(" = ") ) );
	}

	pclArticleItem->AddDescription( WriteCUDouble( _U_FORCE, pclElectroActuator->GetMaxForceTorque(), true ) + CString( _T("; ") ) );
	pclArticleItem->AddDescription( TASApp.LoadLocalizedString( IDS_SSHEETSELPROD_POWSUPPL ) + CString( _T(" = ") ) );
	pclArticleItem->AddDescription( pclElectroActuator->GetPowerSupplyStr() + CString( _T("; ") ) );
	pclArticleItem->AddDescription( TASApp.LoadLocalizedString( IDS_SSHEETSELPROD_RELAYTYPE ) + CString( _T(" = ") ) );
	pclArticleItem->AddDescription( pclElectroActuator->GetRelayStr() );

	pclArticleItem->SetPrice( TASApp.GetpTADB()->GetPrice( pclElectroActuator->GetArtNum( true ) ) );
	pclArticleItem->SetQuantity( iQuantity );
	pclArticleItem->SetIsAvailable( pclElectroActuator->IsAvailable() );
	pclArticleItem->SetIsDeleted( pclElectroActuator->IsDeleted() );

	CString strArticleNumber = pclElectroActuator->GetArtNum();
	CString strLocArtNumber = LocArtNumTab.GetLocalArticleNumber( strArticleNumber );
	pclArticleItem->CheckThingAvailability( pclElectroActuator, strArticleNumber, strLocArtNumber );

	if( true == strLocArtNumber.IsEmpty() )
	{
		strLocArtNumber = _T("-");
	}

	pclArticleItem->SetArticle( strArticleNumber );
	pclArticleItem->SetLocArtNum( strLocArtNumber );

	CArticleContainer *pclArticleContainer = pclArticleGroup->AddArticle( pclArticleItem );

	// Remark: 'AddArticle' will call new CArticleContainer( pclArticleItem ) that copies content of 'pclArticleItem' and not saves the pointer.
	delete pclArticleItem;

	AddAccessoriesInArticleContainer( pclAccessoryList, pclArticleContainer, iQuantity, false );

	CSelProdPageBase::AddArticleInGroupList( pclArticleGroup );

	// Remark: pay attention that 'CPage::AddArticleInGroupList' will increment quantity if article already exist or
	//         will create a new 'CArticleGroup' and will copy to it data passed as argument. This is the reason
	//         why you need to delete 'pclArticleGroup' after the call.
	delete pclArticleGroup;
}

void CSelProdPage6WayValve::_AddArticleList( CDB_Actuator *pclActuator, CArray<IDPTR> *parAccessory, int iQuantity )
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
	
	_AddArticleList( pclActuator, &vecAccessoryList, iQuantity );
}

void CSelProdPage6WayValve::_AddArticleList( CDB_Actuator *pclActuator, std::vector<CDS_Actuator::AccessoryItem> *pvecAccessoryList, int iQuantity )
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
		pclArticleItem->SetDescription( pclElectroActuator->GetName() + CString( _T("; ") ) );

		if( CDB_CloseOffChar::Linear == pclElectroActuator->GetOpeningType() )
		{
			pclArticleItem->AddDescription( TASApp.LoadLocalizedString( IDS_SSHEETSELPROD_ACTFORCE ) + CString( _T(" = ") ) );
		}
		else
		{
			pclArticleItem->AddDescription( TASApp.LoadLocalizedString( IDS_SSHEETSELPROD_ACTTORQUE ) + CString( _T(" = ") ) );
		}

		pclArticleItem->AddDescription( WriteCUDouble( _U_FORCE, pclElectroActuator->GetMaxForceTorque(), true ) + CString( _T("; ") ) );
		pclArticleItem->AddDescription( TASApp.LoadLocalizedString( IDS_SSHEETSELPROD_POWSUPPL ) + CString( _T(" = ") ) );
		pclArticleItem->AddDescription( pclElectroActuator->GetPowerSupplyStr() + CString( _T("; ") ) );
		pclArticleItem->AddDescription( TASApp.LoadLocalizedString( IDS_SSHEETSELPROD_RELAYTYPE ) + CString( _T(" = ") ) );
		pclArticleItem->AddDescription( pclElectroActuator->GetRelayStr() );

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
			str += CString( _T(" : ") ) + WriteDouble( dHysteresis, 3, 1, 1 ) + CString( _T(" K") );
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

	AddAccessoriesInArticleContainer( pvecAccessoryList, pclArticleContainer, iQuantity, false );

	CSelProdPageBase::AddArticleInGroupList( pclArticleGroup );

	// Remark: pay attention that 'CPage::AddArticleInGroupList' will increment quantity if article already exist or
	//         will create a new 'CArticleGroup' and will copy to it data passed as argument. This is the reason
	//         why you need to delete 'pclArticleGroup' after the call.
	delete pclArticleGroup;
}

void CSelProdPage6WayValve::_AddArticleListPIBCvHelper( CDS_SSelPICv *pclSSelPIBCValve, int iGlobalQuantity )
{
	if( NULL == pclSSelPIBCValve )
	{
		return;
	}

	// Add article number for pressure independent balancing & control valve.
	CDB_PIControlValve *pclPIBCValve = pclSSelPIBCValve->GetProductAs<CDB_PIControlValve>();

	if( NULL == pclPIBCValve )
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

	CString str;
	pclArticleItem->SetID( pclPIBCValve->GetIDPtr().ID );
	pclArticleItem->SetArticle( pclPIBCValve->GetBodyArtNum() );
			
	// Check if article is available and not deleted.
	CString strArticleNumber = pclPIBCValve->GetBodyArtNum();
	CString strLocArtNum = LocArtNumTab.GetLocalArticleNumber( strArticleNumber );
	pclArticleItem->CheckThingAvailability( pclPIBCValve, strArticleNumber, strLocArtNum );

	if( true == strLocArtNum.IsEmpty() )
	{
		strLocArtNum = _T("-");
	}

	pclArticleItem->SetArticle( strArticleNumber );
	pclArticleItem->SetLocArtNum( strLocArtNum );
			
	pclArticleItem->SetQuantity( iGlobalQuantity );

	pclArticleItem->SetDescription( pclPIBCValve->GetName() + CString( _T("; ") ) );
	
	// Add version.
	pclArticleItem->AddDescription( ( (CDB_StringID *)pclPIBCValve->GetVersionIDPtr().MP )->GetString() + CString( _T("; ") ) );

	// Add connection.
	str = ( (CDB_StringID *)pclPIBCValve->GetConnectIDPtr().MP )->GetString();
	str = str.Right( str.GetLength() - str.Find('/') - 1 );
	pclArticleItem->AddDescription( str + CString( _T("; ") ) );

	// Add PN.
	pclArticleItem->AddDescription( TASApp.LoadLocalizedString( IDS_PN ) + _T(" ") + pclPIBCValve->GetPN().c_str() );
			
	pclArticleItem->SetPrice( TASApp.GetpTADB()->GetPrice( pclPIBCValve->GetArtNum( true ) ) );
	pclArticleItem->SetIsAvailable( pclPIBCValve->IsAvailable() );
	pclArticleItem->SetIsDeleted( pclPIBCValve->IsDeleted() );

	// Add all article accessories.
	CArticleContainer *pclArticleContainer = pclArticleGroup->AddArticle( pclArticleItem );

	// Remark: 'AddArticle' will call new CArticleContainer( pclArticleItem ) that copies content of 'pclArticleItem' and not saves the pointer.
	delete pclArticleItem;

	AddAccessoriesInArticleContainer( pclSSelPIBCValve->GetCvAccessoryList(), pclArticleContainer, iGlobalQuantity, false );

	CSelProdPageBase::AddArticleInGroupList( pclArticleGroup );

	// Remark: pay attention that 'CPage::AddArticleInGroupList' will increment quantity if article already exist or
	//         will create a new 'CArticleGroup' and will copy to it data passed as argument. This is the reason
	//         why you need to delete 'pclArticlegroup' after the call.
	delete pclArticleGroup;

	// Add connection component if needed.
	if( true == pclPIBCValve->IsConnTabUsed() )
	{
		// Inlet.
		CSelProdPageBase::AddCompArtList( pclPIBCValve, iGlobalQuantity, true );
		// Outlet.
		CSelProdPageBase::AddCompArtList( pclPIBCValve, iGlobalQuantity, false );
	}

	CDB_Actuator *pclActuator = dynamic_cast<CDB_Actuator*>( pclSSelPIBCValve->GetActrIDPtr().MP );

	if( NULL != pclActuator )
	{
		_AddArticleList( pclActuator, pclSSelPIBCValve->GetActuatorAccessoryList(), iGlobalQuantity );
	}
}

void CSelProdPage6WayValve::_AddArticleListBvHelper( CDS_SSelBv *pclSSelBalancingValve, int iGlobalQuantity )
{
	if( NULL == pclSSelBalancingValve )
	{
		return;
	}

	CDB_RegulatingValve *pclBalancingValve = pclSSelBalancingValve->GetProductAs<CDB_RegulatingValve>();

	if( NULL == pclBalancingValve )
	{
		return;
	}

	CArticleGroup *pclArticleGroup = new CArticleGroup();
	
	if( NULL == pclArticleGroup )
	{
		return;
	}

	pclArticleGroup->AddArticle( pclBalancingValve, iGlobalQuantity );
	CSelProdPageBase::AddArticleInGroupList( pclArticleGroup );

	// Remark: pay attention that 'CPage::AddArticleInGroupList' will increment quantity if article already exist or
	//         will create a new 'CArticleGroup' and will copy to it data passed as argument. This is the reason
	//         why you need to delete 'pclArticleGroup' after the call.
	delete pclArticleGroup;

	// Add connection component if needed.
	if( true == pclBalancingValve->IsConnTabUsed() )
	{
		// Inlet.
		CSelProdPageBase::AddCompArtList( pclBalancingValve, iGlobalQuantity, true );
		// Outlet.
		CSelProdPageBase::AddCompArtList( pclBalancingValve, iGlobalQuantity, false );
	}
}

void CSelProdPage6WayValve::_GetGroupColorAndSizes( bool bIsMain, SideDefinition eSideDefinition, COLORREF &clTextColor, COLORREF &clBackgroundColor, double &dRowSize, int &iFontSize )
{
	switch( eSideDefinition )
	{
		case BothSide:
			clTextColor = ( true == bIsMain ) ? _WHITE: _IMI_TITLE_GROUP1;
			clBackgroundColor = ( true == bIsMain ) ? _IMI_TITLE_GROUP1 : _WHITE;
			dRowSize = m_dRowHeight * 1.4;
			iFontSize = 12;
			break;

		case CoolingSide:
			clTextColor = _IMI_TITLE_GROUPCOOLING;
			clBackgroundColor = _WHITE;
			dRowSize = m_dRowHeight * 1.4;
			iFontSize = 12;
			break;

		case HeatingSide:
			clTextColor = _IMI_TITLE_GROUPHEATING;
			clBackgroundColor = _WHITE;
			dRowSize = m_dRowHeight * 1.4;
			iFontSize = 12;
			break;
	}
}
