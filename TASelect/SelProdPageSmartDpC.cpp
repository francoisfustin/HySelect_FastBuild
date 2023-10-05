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
#include "SelProdPageSmartDpC.h"

CSelProdPageSmartDpC::CSelProdPageSmartDpC( CArticleGroupList *pclArticleGroupList ) 
	: CSelProdPageBase( CDB_PageSetup::enCheck::SMARTDPC, pclArticleGroupList )
{
	m_pSelected = NULL;
}

void CSelProdPageSmartDpC::Reset( void )
{
	CSelProdPageBase::Reset();
}

bool CSelProdPageSmartDpC::PreInit( HMvector &vecHMList )
{
	// Something for individual or direct selection ?
	CTable *pclTable = (CTable *)( TASApp.GetpTADS()->Get( _T("SMARTDPC_TAB") ).MP );

	if ( NULL == pclTable )
	{
		ASSERT( 0 );
		return false;
	}

	m_pclSelectionTable = ( _T('\0') != *pclTable->GetFirst().ID ) ? pclTable : NULL;
	bool bSmartDpCExist = ( NULL != m_pclSelectionTable );

	// Something for hydronic calculation?
	bool bSmartDpCHMExist = ( vecHMList.size() > 0 );

	if( false == bSmartDpCExist && false == bSmartDpCHMExist )
	{
		return false;
	}

	m_vecHMList = vecHMList;
	SortTable();
	return true;
}

bool CSelProdPageSmartDpC::Init( bool bResetOrder, bool bPrint )
{
	bool bSmartDpCExist = ( NULL != m_pclSelectionTable ) ? true : false;
	bool bSmartDpCHMExist = ( m_vecHMList.size() > 0 ) ? true : false;

	if( false == bSmartDpCExist && false == bSmartDpCHMExist )
	{
		return false;
	}

	// Call base class method to get a new 'Sheet'.
	CSheetDescription *pclSDSmartDpC = CMultiSpreadBase::CreateSSheet( SD_SmartDpC );

	if( NULL == pclSDSmartDpC || NULL == pclSDSmartDpC->GetSSheetPointer() )
	{
		return false;
	}
	
	CSSheet *pclSheet = pclSDSmartDpC->GetSSheetPointer();

	if( false == CSelProdPageBase::PrepareSheet( pclSDSmartDpC, ColumnDescription::Pointer, bPrint ) )
	{
		m_ViewDescription.RemoveOneSheetDescription( SD_SmartDpC );
		return false;
	}

	// Initialize column width for header and footer column.
	if( false == m_bForPrint )
	{
		SetColWidth( pclSDSmartDpC, ColumnDescription::Header, DefaultColumnWidth::DCW_Header );
		SetColWidth( pclSDSmartDpC, ColumnDescription::Footer, DefaultColumnWidth::DCW_Footer );
	}
	else
	{
		pclSheet->ShowCol( ColumnDescription::Header, FALSE );
		pclSheet->ShowCol( ColumnDescription::Footer, FALSE );
	}

	double dWidth = ( true == m_pTADSPageSetup->GetField( PageField_enum::epfFIRSTREF ) ) ? DefaultColumnWidth::DCW_Reference1 : 0.0;
	SetColWidth( pclSDSmartDpC, ColumnDescription::Reference1, dWidth );
	
	dWidth = ( true == m_pTADSPageSetup->GetField( PageField_enum::epfSECONDREF ) ) ? DefaultColumnWidth::DCW_Reference2 : 0.0;
	SetColWidth( pclSDSmartDpC, ColumnDescription::Reference2, dWidth );
	
	dWidth = ( true == m_pTADSPageSetup->GetField( PageField_enum::epfWATERINFO ) ) ? DefaultColumnWidth::DCW_Water : 0.0;
	SetColWidth( pclSDSmartDpC, ColumnDescription::Water, dWidth );
	
	dWidth = ( true == m_pTADSPageSetup->GetField( PageField_enum::epfSMARTDPCINFOPRODUCT ) ) ? DefaultColumnWidth::DCW_Product : 0.0;
	SetColWidth( pclSDSmartDpC, ColumnDescription::Product, dWidth );
	
	dWidth = ( true == m_pTADSPageSetup->GetField( PageField_enum::epfSMARTDPCTECHINFO ) ) ? DefaultColumnWidth::DCW_TechnicalInfos : 0.0;
	SetColWidth( pclSDSmartDpC, ColumnDescription::TechnicalInfos, dWidth );
	
	SetColWidth( pclSDSmartDpC, ColumnDescription::RadiatorInfos, 0.0 );
	
	dWidth = ( true == m_pTADSPageSetup->GetField( PageField_enum::epfARTICLE ) ) ? DefaultColumnWidth::DCW_ArticleNumber : 0.0;
	SetColWidth( pclSDSmartDpC, ColumnDescription::ArticleNumber, dWidth );
	
	dWidth = ( true == m_pTADSPageSetup->GetField( PageField_enum::epfPIPEINFO ) ) ? DefaultColumnWidth::DCW_Pipes : 0.0;
	SetColWidth( pclSDSmartDpC, ColumnDescription::Pipes, dWidth );
	
	dWidth = ( true == m_pTADSPageSetup->GetField( PageField_enum::epfQUANTITY ) ) ? DefaultColumnWidth::DCW_Quantity : 0.0;
	SetColWidth( pclSDSmartDpC, ColumnDescription::Quantity, dWidth );
	
	dWidth = 0.0;
	
	if( true == TASApp.IsPriceUsed() )
	{
		dWidth = ( true == m_pTADSPageSetup->GetField( PageField_enum::epfSALEUNITPRICE ) ) ? DefaultColumnWidth::DCW_UnitPrice : 0.0;
	}
	
	SetColWidth( pclSDSmartDpC, ColumnDescription::UnitPrice, dWidth );
	
	dWidth = 0.0;

	if( true == TASApp.IsPriceUsed() )
	{
		dWidth = ( true == m_pTADSPageSetup->GetField( PageField_enum::epfSALETOTALPRICE ) ) ? DefaultColumnWidth::DCW_TotalPrice : 0.0;
	}
	
	SetColWidth( pclSDSmartDpC, ColumnDescription::TotalPrice, dWidth );
	
	dWidth = ( true == m_pTADSPageSetup->GetField( PageField_enum::epfREMARK ) ) ? DefaultColumnWidth::DCW_Remark : 0.0;
	SetColWidth( pclSDSmartDpC, ColumnDescription::Remark, dWidth );
	
	SetColWidth( pclSDSmartDpC, ColumnDescription::Pointer, DefaultColumnWidth::DCW_Pointer );
	
	pclSheet->ShowCol( ColumnDescription::RadiatorInfos, FALSE );
	
	if( false == TASApp.IsPriceUsed() )
	{
		pclSheet->ShowCol( ColumnDescription::UnitPrice, FALSE );
		pclSheet->ShowCol( ColumnDescription::TotalPrice, FALSE );
	}

	pclSheet->ShowCol( ColumnDescription::Pointer, FALSE );
	CRect rect = pclSheet->GetSheetSizeInPixels( false );
	m_mapSheetInfos[pclSDSmartDpC].m_dPageWidth = rect.Width();

	// Page title.
	SetPageTitle( pclSDSmartDpC, IDS_RVIEWSELP_TITLE_SMARTDPC );
	
	// Init column header.
	_InitColHeader( pclSDSmartDpC );
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
	
	if( true == bSmartDpCExist )
	{
		// Number of objects.
		int iSmartDpCCount = m_pclSelectionTable->GetItemCount( CLASS( CDS_SSelSmartDpC ) );

		if( 0 == iSmartDpCCount )
		{
			return false;
		}
		
		CDS_SSelSmartDpC **paSSelSmartDpC = new CDS_SSelSmartDpC*[iSmartDpCCount];

		if( NULL == paSSelSmartDpC )
		{
			return false;
		}
		
		// Fill the 'paSSelSmartDpC' array with pointer on each 'CDS_SSelSmartDpC' object of the 'SMARTDPC_TAB' table.
		int iMaxIndex = 0;
		int i = 0;

		for( IDPTR IDPtr = m_pclSelectionTable->GetFirst( CLASS(CDS_SSelSmartDpC) ); _T('\0') != *IDPtr.ID; IDPtr = m_pclSelectionTable->GetNext() )
		{
			// Sanity tests.
			CDS_SSelSmartDpC *pSelSmartDpC = dynamic_cast<CDS_SSelSmartDpC *>( IDPtr.MP );

			if( NULL == pSelSmartDpC )
			{
				continue;
			}
			
			if( NULL == pSelSmartDpC->GetIDPtr().MP )
			{
				continue;
			}
			
			paSSelSmartDpC[i] = (CDS_SSelSmartDpC *)(void *)IDPtr.MP;

			if( iMaxIndex < paSSelSmartDpC[i]->GetpSelectedInfos()->GetRowIndex() )
			{
				iMaxIndex = paSSelSmartDpC[i]->GetpSelectedInfos()->GetRowIndex();
			}

			++i;
		}
		
		ASSERT( i == iSmartDpCCount );
		iSmartDpCCount = i;
		
		// Sort 'paSSelSmartDpC' array.
		// Remark: 'bResetOrder' is set to 'true' only when the sorting combos are empty (No user choice) and the user clicks on the 'Apply sorting keys'.
		if( true == bResetOrder ) 
		{
			// Reset row index.
			for( i = 0; i < iSmartDpCCount; i++ )
			{
				if( 0 == paSSelSmartDpC[i]->GetpSelectedInfos()->GetRowIndex() )
				{
					paSSelSmartDpC[i]->GetpSelectedInfos()->SetRowIndex( ++iMaxIndex );
				}
			}
		}
		else if( PageField_enum::epfNONE != m_pTADSPageSetup->GetKey( 0 ) || PageField_enum::epfNONE != m_pTADSPageSetup->GetKey( 1 )
				|| PageField_enum::epfNONE != m_pTADSPageSetup->GetKey( 2 ) )
		{
			if( NULL != pDlgLeftTabSelP )
			{
				pDlgLeftTabSelP->SortTable( (CData **)paSSelSmartDpC, iSmartDpCCount - 1 );
			}

			// Reset row index.
			for( i = 0; i < iSmartDpCCount; i++ )
			{
				paSSelSmartDpC[i]->GetpSelectedInfos()->SetRowIndex( i + 1 );
			}
		}

		// Create a remark index.
		if( NULL != pDlgLeftTabSelP )
		{
			pDlgLeftTabSelP->SetRemarkIndex( CLASS( CDS_SSelSmartDpC ), (CData **)paSSelSmartDpC, iSmartDpCCount );
		}

		CRank rkl;
		enum eSmartDpCSwitch
		{
			SingleSelSmartDpC,
			DirSelSmartDpC,
			LastSmartDpCSwitch
		};

		bool bAtLeastOneBlockPrinted = false;

		for( int iLoopGroup = 0; iLoopGroup < LastSmartDpCSwitch; iLoopGroup++ )
		{
			int iCount = 0;

			for( i = 0; i < iSmartDpCCount; i++ )
			{
				bool bShouldbeAdded = false;
				CDS_SSelSmartDpC *pclSelSmartDpC = dynamic_cast<CDS_SSelSmartDpC *>( paSSelSmartDpC[i] );
				
				if( NULL == pclSelSmartDpC || NULL == pclSelSmartDpC->GetProductIDPtr().MP )
				{
					ASSERT( 0 );
					continue;
				}

				if( DirSelSmartDpC == iLoopGroup )
				{
					if( false == pclSelSmartDpC->IsFromDirSel() )
					{
						continue;
					}
				}
				else
				{
					if( true == pclSelSmartDpC->IsFromDirSel() )
					{
						continue;
					}
				}

				switch( iLoopGroup )
				{
					case SingleSelSmartDpC:

						if( NULL != dynamic_cast<CDB_SmartControlValve *>( pclSelSmartDpC->GetProductIDPtr().MP ) )
						{
							bShouldbeAdded = true;
						}

						break;

					case DirSelSmartDpC:

						if( NULL != dynamic_cast<CDB_SmartControlValve *>( pclSelSmartDpC->GetProductIDPtr().MP ) )
						{
							bShouldbeAdded = true;
						}

						break;
				}

				if( false == bShouldbeAdded )
				{
					continue;
				}
				
				rkl.Add( _T(""), paSSelSmartDpC[i]->GetpSelectedInfos()->GetRowIndex(), (LPARAM)paSSelSmartDpC[i] );
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
			SetLastRow( pclSDSmartDpC, lRow );

			switch( iLoopGroup )
			{
				case SingleSelSmartDpC:
					SetPageTitle( pclSDSmartDpC, IDS_SSHEETSELPROD_SUBTITLESMARTDPCFROMSSEL, false, lRow );
					break;

				case DirSelSmartDpC:
					SetPageTitle( pclSDSmartDpC, IDS_SSHEETSELPROD_SUBTITLESMARTDPCFROMDIRSEL, false, lRow );
					break;
			}

			lRow = pclSheet->GetMaxRows();
			SetLastRow( pclSDSmartDpC, lRow );

			CString str;
			LPARAM lparam;
			bool bFirstPass = true;

			for( BOOL bContinue = rkl.GetFirst( str, lparam ); TRUE == bContinue; bContinue = rkl.GetNext( str, lparam ) )
			{
				pclSDSmartDpC->RestartRemarkGenerator();

				CData *pclData = (CData *)lparam;
				CDS_SSelSmartDpC *pclSSelSmartDpC = dynamic_cast<CDS_SSelSmartDpC *>( pclData );

				if( NULL == pclSSelSmartDpC )
				{
					continue;
				}

				if( m_pSelected == pclData )
				{
					lSelectedRow = lRow;
				}

				CDB_SmartControlValve *pclSmartDpC = pclSSelSmartDpC->GetProductAs<CDB_SmartControlValve>();

				if( NULL == pclSmartDpC )
				{
					continue;
				}

				m_rProductParam.Clear();
				m_rProductParam.SetSheetDescription( pclSDSmartDpC );
				m_rProductParam.SetSelectionContainer( (LPARAM)pclSSelSmartDpC );

				++lRow;
				SetLastRow( pclSDSmartDpC, lRow );

				long lFirstRow = lRow;

				// Draw line above.
				if( false == bFirstPass )
				{
					pclSheet->SetCellBorder( ColumnDescription::Reference1, lRow, ColumnDescription::Footer - 1, lRow, true, SS_BORDERTYPE_TOP );
				}

				long lRowProduct = _FillRowSmartDpC( pclSDSmartDpC, lRow, pclSSelSmartDpC, pclSmartDpC );
				long lRowGen = CSelProdPageBase::FillRowGen( pclSDSmartDpC, lRow, pclSSelSmartDpC );
				lRow = max( lRowGen, lRowProduct );
				
				// Add the selectable row range.
				m_rProductParam.AddRange( lFirstRow, lRow, pclSmartDpC );

				// Spanning must be done here because it's the only place where we know exactly the number of lines to span!
				// Span reference #1 and #2.
				AddCellSpanW( pclSDSmartDpC, ColumnDescription::Reference1, lFirstRow, 1, pclSheet->GetMaxRows() - lFirstRow + 1 );
				AddCellSpanW( pclSDSmartDpC, ColumnDescription::Reference2, lFirstRow, 1, pclSheet->GetMaxRows() - lFirstRow + 1 );

				// Span water characteristic.
				AddCellSpanW( pclSDSmartDpC, ColumnDescription::Water, lFirstRow, 1, pclSheet->GetMaxRows() - lFirstRow + 1 );

				// Add accessories.
				if( true == pclSSelSmartDpC->IsAccessoryExist() )
				{
					// Draw dash line.
					pclSheet->SetCellBorder( ColumnDescription::Reference1, lRow, ColumnDescription::Footer - 1, lRow, true, SS_BORDERTYPE_BOTTOM, SS_BORDERSTYLE_FINE_DASH );

					lRow++;
					lRow = FillAccessories( pclSDSmartDpC, lRow, pclSSelSmartDpC, pclSSelSmartDpC->GetpSelectedInfos()->GetQuantity() );

					// The adding in the article container has already be done when calling the "_FillRowSmartDpC" method.
				}
				
				// Draw line below.
				pclSheet->SetCellBorder( ColumnDescription::Reference1, lRow, ColumnDescription::Footer - 1, lRow, true, SS_BORDERTYPE_BOTTOM );

				// Add Dp sensor.
				long lRowStartDpSensor = lRow;
				CDB_DpSensor *pclDpSensor = dynamic_cast<CDB_DpSensor *>( pclSSelSmartDpC->GetDpSensorIDPtr().MP );

				if( NULL != pclDpSensor )
				{
					_FillRowDpSensor( pclSDSmartDpC, lRowStartDpSensor + 1, pclSSelSmartDpC );

					// Add the selectable row range.
					m_rProductParam.AddRange( lRowStartDpSensor + 1, pclSheet->GetMaxRows(), pclDpSensor );

					lRow = pclSheet->GetMaxRows();
					
					// Add Dp sensor accessories if exist.
					if( true == pclSSelSmartDpC->IsDpSensorAccessoryExist() )
					{
						// Draw dash line.
						pclSheet->SetCellBorder( ColumnDescription::Reference1, lRow, ColumnDescription::Footer - 1, lRow, true, SS_BORDERTYPE_BOTTOM, SS_BORDERSTYLE_FINE_DASH );
												
						lRow++;
						lRow = FillAccessories( pclSDSmartDpC, lRow, pclSSelSmartDpC->GetDpSensorAccessoryList(), pclSSelSmartDpC->GetpSelectedInfos()->GetQuantity() );
					}
				}
				else if( NULL != pclSSelSmartDpC->GetSetIDPtr().MP  
						&& true == pclSSelSmartDpC->IsSetContentAccessoryExist() )
				{
					// Don't show the "Sets" element selected by user but directly the set content just after.
					// _FillRowSet( pclSDSmartDpC, lRowStartDpSensor + 1, pclSSelSmartDpC );

					// Add the selectable row range.
					// m_rProductParam.AddRange( lRowStartDpSensor + 1, pclSheet->GetMaxRows(), pclProductSet );

					lRow = pclSheet->GetMaxRows();

					// Add set connection if exist.

					// Draw dash line.
					pclSheet->SetCellBorder( ColumnDescription::Reference1, lRow, ColumnDescription::Footer - 1, lRow, true, SS_BORDERTYPE_BOTTOM );
										
					lRow++;
					lRow = _FillSetContent( pclSDSmartDpC, lRow, pclSSelSmartDpC );
				}

				// Add the product in the 'Article list'.
				_AddArticleList( pclSSelSmartDpC );

				// Save the object pointer.
				m_rProductParam.SetScrollRange( ( true == bFirstPass ) ? lFirstRow - 1 : lFirstRow, lRow );
				SaveProduct( m_rProductParam );

				// Draw line below.
				pclSheet->SetCellBorder( ColumnDescription::Reference1, lRow, ColumnDescription::Footer - 1, lRow, true, SS_BORDERTYPE_BOTTOM );

				// Set all group as no breakable (for print).
				// Remark: include title with the group.
				pclSheet->SetFlagRowNoBreakable( ( true == bFirstPass ) ? lFirstRow - 1 : lFirstRow, lRow, true );

				// For error, warning and info messages we use now the remark generator from the "CSheetDescription" class.
				lRow++;
				lRow = pclSDSmartDpC->WriteRemarks( lRow, ColumnDescription::Reference1, ColumnDescription::Footer - 1 );
				
				// The 'WriteRemarks' method just above returns well the next position but the internal last row in TSpread is not set.
				SetLastRow( pclSDSmartDpC, lRow );

				bFirstPass = false;
			}

			bAtLeastOneBlockPrinted = true;
		}

		// FillRemarks( pclSDSmartDpC, lRow );

		// We add by default a blank line.
		lRow = pclSheet->GetMaxRows() + 1;
		
		delete[] paSSelSmartDpC;
	}

	if( true == bSmartDpCHMExist )
	{
		if( m_vecHMList.size() > 0 )	
		{
			// Create a remark index.
			if( NULL != pDlgLeftTabSelP )
			{
				// Remark: For 'CDS_HydroMod' we don't use anymore for the smart differential pressure controller the "FillRemarks" method.
				//         Instead we use remark generator from the "CSheetDescription" class.
				pDlgLeftTabSelP->SetRemarkIndex( &m_vecHMList );
			}

			SetLastRow( pclSDSmartDpC, lRow );
			SetPageTitle( pclSDSmartDpC, IDS_SSHEETSELPROD_SUBTITLESMARTDPCFROMHMCALC, false, lRow );	
			
			lRow++;
			SetLastRow( pclSDSmartDpC, lRow );
			
			// For each element.
			bool bFirstPass = true;
			HMvector::iterator It;

			for( It = m_vecHMList.begin(); It != m_vecHMList.end(); It++ )
			{
				pclSDSmartDpC->RestartRemarkGenerator();

				CDS_HydroMod *pHM = NULL;
				CDS_HydroMod::CBase *pBase = NULL;

				if( eptHM == (*It).first )
				{
					pHM = dynamic_cast<CDS_HydroMod *>( (CData*)( (*It).second ) );
				}
				else
				{
					// Smart differential pressure controller exists and TA product exist into the DB.
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
					m_rProductParam.SetSheetDescription( pclSDSmartDpC );
					// Intentionally set to NULL to avoid clicking, double clicking or right clicking.
					m_rProductParam.SetSelectionContainer( (LPARAM)0 );

					long lRowSmartDpC = _FillRowSmartDpC( pclSDSmartDpC, lRow, pHM );
					long lRowGen = CSelProdPageBase::FillRowGen( pclSDSmartDpC, lRow, pHM, pBase );

					long lLastRow = max( lRowGen, lRowSmartDpC );
					
					// HYS-721: Merge cells
					long lFirstRow = lRow;
					
					// Spanning must be done here because it's the only place where we know exactly number of lines to span!
					// Span reference #1 and #2.
					AddCellSpanW(pclSDSmartDpC, ColumnDescription::Reference1, lFirstRow, 1, pclSheet->GetMaxRows() - lFirstRow + 1);
					AddCellSpanW(pclSDSmartDpC, ColumnDescription::Reference2, lFirstRow, 1, pclSheet->GetMaxRows() - lFirstRow + 1);
					
					// Span water characteristic.
					AddCellSpanW(pclSDSmartDpC, ColumnDescription::Water, lFirstRow, 1, pclSheet->GetMaxRows() - lFirstRow + 1);

					CDB_TAProduct *pclSmartDpC = NULL;

					// If 'pHM' exists and not 'pclSmartDpC' means that smart control valve has been added in CBI mode.
					if( NULL == pBase )
					{
						pclSmartDpC = dynamic_cast<CDB_TAProduct *>( pHM->GetCBIValveIDPtr().MP );
					}
					else
					{
						pclSmartDpC = dynamic_cast<CDB_TAProduct *>( pHM->GetpSmartDpC()->GetIDPtr().MP );
					}

					if( NULL != pclSmartDpC )
					{
						// Draw line below.
						pclSheet->SetCellBorder( ColumnDescription::Reference1, lLastRow, ColumnDescription::Footer - 1, lLastRow, true, SS_BORDERTYPE_BOTTOM );

						if( eBool3::eb3False == pHM->GetpSmartDpC()->IsSelectedAsaPackage() )
						{
							// Add Dp sensor.
							CDB_DpSensor *pclDpSensor = dynamic_cast<CDB_DpSensor *>( pHM->GetpSmartDpC()->GetpDpSensor() );

							_FillRowDpSensor( pclSDSmartDpC, lLastRow + 1, pHM );

							m_rProductParam.AddRange( lLastRow + 1, pclSheet->GetMaxRows(), pclDpSensor );

							lLastRow = pclSheet->GetMaxRows();
					
							if( pHM->GetpSmartDpC()->GetDpSensorAccessoryCount() > 0 )
							{
								// Draw dash line.
								pclSheet->SetCellBorder( ColumnDescription::Reference1, lLastRow, ColumnDescription::Footer - 1, lLastRow, true, SS_BORDERTYPE_BOTTOM, SS_BORDERSTYLE_FINE_DASH );

								// Add Dp sensor accessories.
								lLastRow++;
								lLastRow = _FillDpSensorAccessoriesHM( pclSDSmartDpC, lLastRow, pHM->GetpSmartDpC() );	
							}
						}
						else
						{
							// Don't show the "Sets" element selected by user but directly the set content just after.
							// _FillRowSet( pclSDSmartDpC, lLastRow + 1, pHM );

							if( pHM->GetpSmartDpC()->GetSetContentCount() > 0 )
							{
								lRow = pclSheet->GetMaxRows();

								// Draw line below.
								pclSheet->SetCellBorder( ColumnDescription::Reference1, lLastRow, ColumnDescription::Footer - 1, lLastRow, true, SS_BORDERTYPE_BOTTOM );

								// Add smart differential pressure controller and Dp sensor set accessories.
								lLastRow++;
								lLastRow = _FillSetContent( pclSDSmartDpC, lLastRow, pHM->GetpSmartDpC() );
							}
						}
					}

					// Add the product in the 'Article list'.
					_AddArticleList( pHM->GetpSmartDpC() );

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
				lRow = pclSDSmartDpC->WriteRemarks( lRow, ColumnDescription::Reference1, ColumnDescription::Footer - 1 );
				
				// The 'WriteRemarks' method just above returns well the next position but the internal last row in TSpread is not set.
				// We add a blank line.
				lRow++;
				SetLastRow( pclSDSmartDpC, lRow );
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

bool CSelProdPageSmartDpC::HasSomethingToDisplay( void )
{
	bool bSmartDpCExist = ( NULL != m_pclSelectionTable ) ? true : false;
	bool bSmartDpCHMExist = ( m_vecHMList.size() > 0 ) ? true : false;

	if( false == bSmartDpCExist && false == bSmartDpCHMExist )
	{
		return false;
	}

	return true;
}

void CSelProdPageSmartDpC::FillAccessoryHelper_GetNameDescription( CDB_Product *pclAccessory, CString &strName, CString &strDescription, LPARAM lpParam )
{
	strName = _T("");
	strDescription = _T("");

	if( NULL == pclAccessory )
	{
		return;
	}

	strName = pclAccessory->GetName();
	strDescription = pclAccessory->GetComment();

	if( NULL == dynamic_cast<CDB_DpSensor *>( pclAccessory ) )
	{
		return;
	}

	// Add name with Dpl range.
	CDB_DpSensor *pclDpSensor = (CDB_DpSensor *)( pclAccessory );
	strName = pclDpSensor->GetFullName();
	double dDpToStabilize = 0.0;

	// Add dp to stabilize if exists.
	if( NULL != dynamic_cast<CDS_SSelSmartDpC *>( (CData *)lpParam ) )
	{
		CDS_SSelSmartDpC *pclSSelSmartDpc = (CDS_SSelSmartDpC *)lpParam;
		
		if( true == pclSSelSmartDpc->IsCheckboxDpBranchChecked() )
		{
			dDpToStabilize = pclSSelSmartDpc->GetDpBranchValue();
		}
	}
	else if( NULL != dynamic_cast<CDS_HydroMod::CSmartDpC *>( (CDS_HydroMod::CBase *)lpParam ) )
	{
		CDS_HydroMod::CSmartDpC *pclHMSmartDpC = (CDS_HydroMod::CSmartDpC *)lpParam;
		dDpToStabilize = pclHMSmartDpC->GetDpToStabilize();
	}

	if( dDpToStabilize > 0.0 )
	{
		strName += _T("; ") + TASApp.LoadLocalizedString( IDS_SSHEETSELPROD_DPSENSORDPL );
		strName += _T(": ") + CString( WriteCUDouble( _U_DIFFPRESS, dDpToStabilize, true ) );
	}

	// Add eventually PN and temperature range.
	CSelProdPageBase::FillAccessoryHelper_GetNameDescription( pclAccessory, strName, strDescription );

	CString strBurstPressure = WriteCUDouble( _U_DIFFPRESS, pclDpSensor->GetBurstPressure(), true );
	FormatString( strDescription, pclDpSensor->GetComment(), strBurstPressure );
}

void CSelProdPageSmartDpC::FillAccessoryHelper_GetErrorMessage( CDB_Product *pclAccessory, int iCol, std::vector<CString> *pvecErrorMsgList, LPARAM lpParam )
{
	if( NULL == pclAccessory || NULL == dynamic_cast<CDB_DpSensor *>( pclAccessory ) || NULL == pvecErrorMsgList )
	{
		return;
	}

	pvecErrorMsgList->clear();

	// Are we comming from product selection or hydraulic calculator ?
	CDB_DpSensor *pclDpSensor = (CDB_DpSensor *)( pclAccessory );
	double dDpBranch = 0.0;
	double dDpMax = 0.0;
	double dTemp = DBL_MAX;

	if( NULL != dynamic_cast<CDS_SSelSmartDpC *>( (CData *)lpParam ) )
	{
		CDS_SSelSmartDpC *pclSSelSmartDpC = (CDS_SSelSmartDpC *)lpParam;

		if( true == pclSSelSmartDpC->IsCheckboxDpBranchChecked() )
		{
			dDpBranch = pclSSelSmartDpC->GetDpBranchValue();
		}

		if( true == pclSSelSmartDpC->IsCheckboxDpMaxChecked() )
		{
			dDpMax = pclSSelSmartDpC->GetDpMaxValue();
		}

		if( NULL != pclSSelSmartDpC->GetpSelectedInfos() && NULL != pclSSelSmartDpC->GetpSelectedInfos()->GetpWCData() )
		{
			dTemp = pclSSelSmartDpC->GetpSelectedInfos()->GetpWCData()->GetTemp();
		}
	}
	else if( NULL != dynamic_cast<CDS_HydroMod::CSmartDpC *>( (CDS_HydroMod::CBase *)lpParam ) )
	{
		CDS_HydroMod::CSmartDpC *pclHMSmartDpC = (CDS_HydroMod::CSmartDpC *)lpParam;

		if( NULL != pclHMSmartDpC )
		{
			dDpBranch = pclHMSmartDpC->GetDpToStabilize();
			dDpMax = pclHMSmartDpC->GetDp();
		}

		CWaterChar *pclWaterChar = pclHMSmartDpC->GetpParentHM()->GetpWaterChar( CDS_HydroMod::eHMObj::eSmartDpC );

		if( NULL != pclWaterChar )
		{
			dTemp = pclWaterChar->GetTemp();
		}
	}

	CString strError = _T("");

	if( NULL != pclDpSensor )
	{
		// Only for the Dp sensor name.
		if( dDpBranch > 0.0 && ColumnDescription::Reference1 == iCol )
		{
			if( dDpBranch < pclDpSensor->GetMinMeasurableDp() )
			{
				// Dp to stabilize is below the minimum measurable pressure drop of the currenct selected Dp sensor (%1 < %2).
				CString strDpl = WriteCUDouble( _U_DIFFPRESS, dDpBranch );
				CString strMinMeasurableDpl = WriteCUDouble( _U_DIFFPRESS, pclDpSensor->GetMinMeasurableDp(), true );
				FormatString( strError, IDS_HYDROMOD_SMARTDPCDPLTOOLOW, strDpl, strMinMeasurableDpl );
				pvecErrorMsgList->push_back( strError );
			}
			else if( dDpBranch > pclDpSensor->GetMaxMeasurableDp() )
			{
				// Dp to stabilize is above the maximum measurable pressure drop of the current selected Dp sensor (%1 > %2).
				CString strDpl = WriteCUDouble( _U_DIFFPRESS, dDpBranch );
				CString strMaxMeasurableDpl = WriteCUDouble( _U_DIFFPRESS, pclDpSensor->GetMaxMeasurableDp(), true );
				FormatString( strError, IDS_HYDROMOD_SMARTDPCDPLTOOHIGH, strDpl, strMaxMeasurableDpl );
				pvecErrorMsgList->push_back( strError );
			}
		}
				
		// Only for the Dp sensor description.
		if( dDpBranch > 0.0 && ColumnDescription::Product == iCol )
		{
			// HYS-2007: if burst pressure != 0.
			if( 0 != pclDpSensor->GetBurstPressure() && dDpBranch > pclDpSensor->GetBurstPressure() )
			{
				// The maximum differential pressure is above the limit of the burst pressure (%1 > %2).
				CString str2 = WriteCUDouble( _U_DIFFPRESS, dDpBranch, true, 3, 0 );
				CString str3 = WriteCUDouble( _U_DIFFPRESS, pclDpSensor->GetBurstPressure(), true, 3, 0 );
				FormatString( strError, IDS_PRODUCTSELECTION_ERROR_BURSTPRESSURE, str2, str3 );
				pvecErrorMsgList->push_back( strError );
			}
		}

		// Only for the Dp sensor name.
		if( dTemp != DBL_MAX && ColumnDescription::Reference1 == iCol )
		{
			if( dTemp < pclDpSensor->GetTmin() )
			{
				// The fluid temperature is lower that the minimum allowed for %1 (%2 < %3).
				CString strProductName = pclDpSensor->GetName();
				CString strTemp = WriteCUDouble( _U_TEMPERATURE, dTemp );
				CString strMinTemp = WriteCUDouble( _U_TEMPERATURE, pclDpSensor->GetTmin(), true );
				FormatString( strError, IDS_HYDROMOD_TEMPTOOLOWFORPRODUCT, strProductName, strTemp, strMinTemp );
				pvecErrorMsgList->push_back( strError );
			}

			if( dTemp > pclDpSensor->GetTmax() )
			{
				// The fluid temperature is higher that the maximum allowed for %1 (%2 > %3).
				CString strProductName = pclDpSensor->GetName();
				CString strTemp = WriteCUDouble( _U_TEMPERATURE, dTemp );
				CString strMaxTemp = WriteCUDouble( _U_TEMPERATURE, pclDpSensor->GetTmax(), true );
				FormatString( strError, IDS_HYDROMOD_TEMPTOOHIGHFORPRODUCT, strProductName, strTemp, strMaxTemp );
				pvecErrorMsgList->push_back( strError );
			}
		}
	}
}

void CSelProdPageSmartDpC::_InitColHeader( CSheetDescription *pclSheetDescription )
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

long CSelProdPageSmartDpC::_FillRowSmartDpC( CSheetDescription *pclSheetDescription, long lRow, 
		CDS_SSelSmartDpC *pclSSelSmartDpC, CDB_SmartControlValve *pclSmartDpC )
{
	if( NULL == pclSheetDescription || NULL == pclSheetDescription->GetSSheetPointer() || NULL == pclSSelSmartDpC || NULL == pclSmartDpC )
	{
		return lRow;
	}

	CSSheet *pclSheet = pclSheetDescription->GetSSheetPointer();
	CString str1, str2;
	long lFirstRow = lRow;

	pclSheet->SetTextPattern( CSSheet::_SSTextPattern::UserStaticColumn );
	pclSheet->SetTextPatternProperty( CSSheet::_SSTextPatternProperty::TPP_FontBold, (LPARAM)FALSE );
		
	// Column TA Product.
	CDB_Set *pSet = NULL;
	CString ArtNum( _T("") );

	if( true == pclSSelSmartDpC->IsSelectedAsAPackage( true ) )
	{
		pSet = dynamic_cast<CDB_Set *>( pclSSelSmartDpC->GetSetIDPtr().MP );

		if( NULL == pSet )
		{
			ASSERTA_RETURN( lRow );
		}

		ArtNum = pSet->GetReference();
	}

	_FillRowSmartDpCProduct( pclSheetDescription, lRow, pSet, ArtNum, pclSSelSmartDpC );

	// Column infos.
	if( false == pclSSelSmartDpC->IsFromDirSel() )
	{
		SmartDpCParams rSmartDpCParams;

		// HYS-2067
		rSmartDpCParams.dFlowMax = pclSSelSmartDpC->GetQ();
		rSmartDpCParams.strPowerDT = _T( "" );

		if( CDS_SelProd::efdPower == pclSSelSmartDpC->GetFlowDef() )
		{
			rSmartDpCParams.strPowerDT = TASApp.LoadLocalizedString( IDS_SSHEETSELPROD_SMARTCONTROLVAVEPOWERMAX );
			rSmartDpCParams.strPowerDT += _T( ": " ) + CString( WriteCUDouble( _U_TH_POWER, pclSSelSmartDpC->GetPower(), true ) );
			rSmartDpCParams.strPowerDT += _T( " / " );
			rSmartDpCParams.strPowerDT += WriteCUDouble( _U_DIFFTEMP, pclSSelSmartDpC->GetDT(), true );
			rSmartDpCParams.dPowerMax = pclSSelSmartDpC->GetPower();
		}
		rSmartDpCParams.dDpMin = CalcDp( pclSSelSmartDpC->GetQ(), pclSmartDpC->GetKvs(), 
				pclSSelSmartDpC->GetpSelectedInfos()->GetpWCData()->GetDens() );

		rSmartDpCParams.eLocalization = SmartValveLocalization::SmartValveLocNone;
		rSmartDpCParams.pclSSelSmartDpC = pclSSelSmartDpC;

		_FillRowSmartDpCInfo( pclSheetDescription, lFirstRow, rSmartDpCParams );
	}

	return pclSheet->GetMaxRows();
}

long CSelProdPageSmartDpC::_FillRowSmartDpC( CSheetDescription *pclSheetDescription, long lRow, CDS_HydroMod *pclHM )
{
	if( NULL == pclSheetDescription || NULL == pclSheetDescription->GetSSheetPointer() || NULL == pclHM || NULL == pclHM->GetpSmartDpC() )
	{
		return lRow;
	}

	CSSheet *pclSheet = pclSheetDescription->GetSSheetPointer();
	CString str1, str2;
	long lFirstRow = lRow;

	SmartDpCParams rSmartDpCParams;

	CDB_SmartControlValve *pclSmartDpC = NULL;
	
	pclSmartDpC = dynamic_cast<CDB_SmartControlValve *>( pclHM->GetpSmartDpC()->GetIDPtr().MP);
	rSmartDpCParams.dDp = pclHM->GetpSmartDpC()->GetDp();
	rSmartDpCParams.dDpMin = pclHM->GetpSmartDpC()->GetDplmin();
	rSmartDpCParams.eLocalization = pclHM->GetpSmartDpC()->GetLocalization();
	// HYS-2067
	rSmartDpCParams.dFlowMax = pclHM->GetpSmartDpC()->GetQ();

	if( pclHM->GetpTermUnit() != NULL && CTermUnit::_QType::PdT == pclHM->GetpTermUnit()->GetQType() )
	{
		rSmartDpCParams.dPowerMax = pclHM->GetpTermUnit()->GetP();

		rSmartDpCParams.strPowerDT = TASApp.LoadLocalizedString( IDS_SSHEETSELPROD_SMARTCONTROLVAVEPOWERMAX );
		rSmartDpCParams.strPowerDT += _T( ": " ) + CString( WriteCUDouble( _U_TH_POWER, pclHM->GetpTermUnit()->GetP(), true ) );
		rSmartDpCParams.strPowerDT += _T( " / " );
		rSmartDpCParams.strPowerDT += WriteCUDouble( _U_DIFFTEMP, pclHM->GetpSmartDpC()->GetDT(), true );
	}

	if( NULL == pclSmartDpC )
	{
		return pclSheet->GetMaxRows();
	}
	
	rSmartDpCParams.pclHM = pclHM;

	pclSheet->SetTextPattern( CSSheet::_SSTextPattern::UserStaticColumn );
	pclSheet->SetTextPatternProperty( CSSheet::_SSTextPatternProperty::TPP_FontBold, (LPARAM)FALSE );

	// Column TA Product.
	CString strArticleNumber = pclSmartDpC->GetArtNum( true );
	CDB_Set *pclSmartDpCDpSensorSet = NULL;

	if( true == pclSmartDpC->IsPartOfaSet() && eBool3::eb3True == pclHM->GetpSmartDpC()->IsSelectedAsaPackage() )
	{
		CDB_Product *pclProductSet = pclHM->GetpSmartDpC()->GetpProductSet();

		if( NULL != pclProductSet )
		{
			CTableSet *pTabSet = pclSmartDpC->GetTableSet();
			ASSERT( NULL != pTabSet );

			pclSmartDpCDpSensorSet = pTabSet->FindCompatibleSet( pclSmartDpC->GetIDPtr().ID, pclProductSet->GetIDPtr().ID );
		}

		if( NULL != pclSmartDpCDpSensorSet )
		{
			strArticleNumber = pclSmartDpCDpSensorSet->GetReference();	
		}
	}

	_FillRowSmartDpCProduct( pclSheetDescription, lRow, pclSmartDpCDpSensorSet, strArticleNumber, NULL, pclHM );

	// Column infos.
	_FillRowSmartDpCInfo( pclSheetDescription, lFirstRow, rSmartDpCParams );

	return pclSheet->GetMaxRows();
}

long CSelProdPageSmartDpC::_FillRowSmartDpCProduct( CSheetDescription *pclSheetDescription, long lRow, CDB_Set *pclSmartDpCDpSensorSet, 
		CString strArtNumber, CDS_SSelSmartDpC *pclSSelSmartDpC, CDS_HydroMod *pclHM )
{
	if( NULL == pclSheetDescription || NULL == TASApp.GetpTADS() || NULL == TASApp.GetpTADS()->GetpTechParams() ||
			( NULL == pclSSelSmartDpC && NULL == pclHM ) )
	{
		return lRow;
	}

	if( NULL != pclSSelSmartDpC && NULL == dynamic_cast<CDB_SmartControlValve *>( pclSSelSmartDpC->GetSmartDpCIDPtr().MP ) )
	{
		return lRow;
	}

	if( NULL != pclHM && 
			( NULL == pclHM->GetpSmartDpC() || NULL == pclHM->GetpSmartDpC()->GetpSmartDpC() ) )
	{
		return lRow;
	}

	CDB_SmartControlValve *pclSmartDpC = NULL;
	int iQuantity = 1;

	if( NULL != pclSSelSmartDpC )
	{
		pclSmartDpC = (CDB_SmartControlValve *)( pclSSelSmartDpC->GetSmartDpCIDPtr().MP );
		iQuantity = pclSSelSmartDpC->GetpSelectedInfos()->GetQuantity();
	}
	else
	{
		pclSmartDpC = pclHM->GetpSmartDpC()->GetpSmartDpC();
	}

	CString str1, str2;
	long lFirstRow = lRow;
	int iValidity = -1;
	CString strErrorMessage = _T("");
	CString str;
	
	// Column TA Product.
	AddStaticText( pclSheetDescription, ColumnDescription::Product, lRow, _T("-") );

	if( true == m_pTADSPageSetup->GetField( epfSMARTDPCINFOPRODUCTNAME ) )	
	{
		strErrorMessage = _T("");
		str = pclSmartDpC->GetName();

		// Check if there is error on the smart differential pressure controller.
		std::vector<CString> vecSmartErrorMessages;

		if( NULL != pclSSelSmartDpC )
		{
			double dFluidTemp = pclSSelSmartDpC->GetpSelectedInfos()->GetpWCData()->GetTemp();

			if( dFluidTemp < pclSmartDpC->GetTmin() )
			{
				// The fluid temperature is lower that the minimum allowed for %1 (%2 < %3).
				CString strProductName = pclSmartDpC->GetName();
				CString strTemp = WriteCUDouble( _U_TEMPERATURE, dFluidTemp );
				CString strMinTemp = WriteCUDouble( _U_TEMPERATURE, pclSmartDpC->GetTmin(), true );
				FormatString( strErrorMessage, IDS_HYDROMOD_TEMPTOOLOWFORPRODUCT, strProductName, strTemp, strMinTemp );
				vecSmartErrorMessages.push_back( strErrorMessage );
			}
			else if( dFluidTemp > pclSmartDpC->GetTmax() )
			{
				// The fluid temperature is higher that the maximum allowed for %1 (%2 > %3).
				CString strProductName = pclSmartDpC->GetName();
				CString strTemp = WriteCUDouble( _U_TEMPERATURE, dFluidTemp );
				CString strMaxTemp = WriteCUDouble( _U_TEMPERATURE, pclSmartDpC->GetTmax(), true );
				FormatString( strErrorMessage, IDS_HYDROMOD_TEMPTOOHIGHFORPRODUCT, strProductName, strTemp, strMaxTemp );
				vecSmartErrorMessages.push_back( strErrorMessage );
			}

			if( true == pclSSelSmartDpC->IsCheckboxDpMaxChecked() && pclSSelSmartDpC->GetDpMaxValue() 
					&& pclSSelSmartDpC->GetDpMaxValue() > pclSmartDpC->GetDpmax() )
			{
				// The pressure drop accross the valve is higher that the maximum allowed (%1 > %2).
				CString strDp = WriteCUDouble( _U_DIFFPRESS, pclSSelSmartDpC->GetDpMaxValue() );
				CString strMaxDp = WriteCUDouble( _U_DIFFPRESS, pclSmartDpC->GetDpmax(), true );
				FormatString( strErrorMessage, IDS_HYDROMOD_SMARTVALVEDPTOOHIGH, strDp, strMaxDp );
				vecSmartErrorMessages.push_back( strErrorMessage );
			}
		}
		else
		{
			// Update error messages.
			iValidity = pclHM->GetpSmartDpC()->CheckValidity();

			vecSmartErrorMessages = pclHM->GetpSmartDpC()->GetErrorMessageList( CDS_HydroMod::CSmartDpC::eErrorMessageObject_SmartDpC,
					CDS_HydroMod::ErrorWarningMessageFlags::EWMF_All );
		}

		if( 0 == (int)vecSmartErrorMessages.size() )
		{
			lRow = AddStaticText( pclSheetDescription, ColumnDescription::Product, lRow, str );
		}
		else
		{
			// Add tooltip.
			AddTooltipMessage( CPoint( ColumnDescription::Product, lRow ), vecSmartErrorMessages );

			// Add remark to put below.
			WriteTextWithFlags( pclSheetDescription, str, ColumnDescription::Product, lRow, CSheetDescription::RemarkFlags::ErrorMessage, vecSmartErrorMessages, _RED );
			lRow++;
		}
	}

	if( true == m_pTADSPageSetup->GetField( epfSMARTDPCINFOPRODUCTBDYMATERIAL ) )	
	{
		lRow = AddStaticText( pclSheetDescription, ColumnDescription::Product, lRow, pclSmartDpC->GetBodyMaterial() );
	}
	
	if( true == m_pTADSPageSetup->GetField( epfSMARTDPCINFOPRODUCTCONNECTION ) )	
	{
		lRow = AddStaticText( pclSheetDescription, ColumnDescription::Product, lRow, pclSmartDpC->GetConnect() );
	}

	if( true == m_pTADSPageSetup->GetField( epfSMARTDPCINFOPRODUCTPN ) )
	{
		strErrorMessage = _T("");
		long lRowTemp = lRow;
		lRow = FillPNTminTmax( pclSheetDescription, ColumnDescription::Product, lRow, pclSmartDpC );

		if( NULL != pclSSelSmartDpC )
		{
			double dFluidTemp = pclSSelSmartDpC->GetpSelectedInfos()->GetpWCData()->GetTemp();

			if( dFluidTemp < pclSmartDpC->GetTmin() )
			{
				// The fluid temperature is lower that the minimum allowed for %1 (%2 < %3).
				CString strProductName = pclSmartDpC->GetName();
				CString strTemp = WriteCUDouble( _U_TEMPERATURE, dFluidTemp );
				CString strMinTemp = WriteCUDouble( _U_TEMPERATURE, pclSmartDpC->GetTmin(), true );
				FormatString( strErrorMessage, IDS_HYDROMOD_TEMPTOOLOWFORPRODUCT, strProductName, strTemp, strMinTemp );
			}
			else if( dFluidTemp > pclSmartDpC->GetTmax() )
			{
				// The fluid temperature is higher that the maximum allowed for %1 (%2 > %3).
				CString strProductName = pclSmartDpC->GetName();
				CString strTemp = WriteCUDouble( _U_TEMPERATURE, dFluidTemp );
				CString strMaxTemp = WriteCUDouble( _U_TEMPERATURE, pclSmartDpC->GetTmax(), true );
				FormatString( strErrorMessage, IDS_HYDROMOD_TEMPTOOHIGHFORPRODUCT, strProductName, strTemp, strMaxTemp );
			}
		}
		else
		{
			// Update error message if needed.
			if( -1 == iValidity )
			{
				iValidity = pclHM->GetpSmartDpC()->CheckValidity();
			}

			// Add tooltip.
			strErrorMessage = pclHM->GetpSmartDpC()->GetErrorMessageString( CDS_HydroMod::CSmartDpC::eErrorMessageObject_SmartDpC,
					CDS_HydroMod::ErrorWarningMessageFlags::EWMF_Temp );
		}

		if( false == strErrorMessage.IsEmpty() )
		{
			// Set the PN/temp text in red.
			pclSheetDescription->GetSSheetPointer()->SetForeColor( ColumnDescription::Product, lRowTemp, _RED );

			// Add tooltip.
			AddTooltipMessage( CPoint( ColumnDescription::Product, lRowTemp ), strErrorMessage );

			// Add remark to put below.
			CString strCellText = pclSheetDescription->GetSSheetPointer()->GetCellText( ColumnDescription::Product, lRowTemp );
			WriteTextWithFlags( pclSheetDescription, strCellText, ColumnDescription::Product, lRowTemp, CSheetDescription::RemarkFlags::ErrorMessage, strErrorMessage, _RED );
		}
	}
		
	if( true == m_pTADSPageSetup->GetField( epfSMARTDPCINFOPRODUCTKVS ) )	
	{
		if( NULL != pclSmartDpC->GetSmartValveCharacteristic() && -1.0 != pclSmartDpC->GetSmartValveCharacteristic()->GetKvs() )
		{
			if( 0 == m_pUnitDB->GetDefaultUnitIndex( _C_KVCVCOEFF ) )
			{
				str1 = TASApp.LoadLocalizedString( IDS_KVS );
			}
			else
			{
				str1 = TASApp.LoadLocalizedString( IDS_CV );
			}

			str1 += CString( _T(" = ") ) + WriteCUDouble( _C_KVCVCOEFF, pclSmartDpC->GetSmartValveCharacteristic()->GetKvs() );
			lRow = AddStaticText( pclSheetDescription, ColumnDescription::Product, lRow, str1 );
		}
	}

	if( true == m_pTADSPageSetup->GetField( epfSMARTDPCINFOPRODUCTQNOM ) )	
	{
		if( NULL != pclSmartDpC->GetSmartValveCharacteristic() && -1.0 != pclSmartDpC->GetSmartValveCharacteristic()->GetQnom() )
		{
			str1 = CString( _T("Qnom = ") ) + WriteCUDouble( _U_FLOW, pclSmartDpC->GetSmartValveCharacteristic()->GetQnom(), true );
			lRow = AddStaticText( pclSheetDescription, ColumnDescription::Product, lRow, str1 );
		}
	}

	if( true == m_pTADSPageSetup->GetField( epfSMARTDPCINFOPRODUCTDPMAX ) )
	{
		strErrorMessage = _T("");
		str = TASApp.LoadLocalizedString( IDS_SSHEETSELPROD_SMARTDPCDPMAX );
		str += _T(" = ") + CString( WriteCUDouble( _U_DIFFPRESS, pclSmartDpC->GetDpmax(), true ) );

		if( NULL != pclSSelSmartDpC )
		{
			if( true == pclSSelSmartDpC->IsCheckboxDpMaxChecked() && pclSSelSmartDpC->GetDpMaxValue() > pclSmartDpC->GetDpmax() )
			{
				// The pressure drop accross the valve is higher that the maximum allowed (%1 > %2).
				CString strDp = WriteCUDouble( _U_DIFFPRESS, pclSSelSmartDpC->GetDpMaxValue() );
				CString strMaxDp = WriteCUDouble( _U_DIFFPRESS, pclSmartDpC->GetDpmax(), true );
				FormatString( strErrorMessage, IDS_HYDROMOD_SMARTVALVEDPTOOHIGH, strDp, strMaxDp );
			}
		}
		else
		{
			// Update error message if needed.
			if( -1 == iValidity )
			{
				iValidity = pclHM->GetpSmartDpC()->CheckValidity();
			}

			// Add tooltip.
			strErrorMessage = pclHM->GetpSmartDpC()->GetErrorMessageString( CDS_HydroMod::CSmartDpC::eErrorMessageObject_SmartDpC,
					CDS_HydroMod::ErrorWarningMessageFlags::EWMF_Dp );
		}

		if( true == strErrorMessage.IsEmpty() )
		{
			lRow = AddStaticText( pclSheetDescription, ColumnDescription::Product, lRow, str );
		}
		else
		{
			// Add tooltip.
			AddTooltipMessage( CPoint( ColumnDescription::Product, lRow ), strErrorMessage );

			// Add remark to put below.
			WriteTextWithFlags( pclSheetDescription, str, ColumnDescription::Product, lRow, CSheetDescription::RemarkFlags::ErrorMessage, strErrorMessage, _RED );
			lRow++;
		}
	}

	// Columns quantity and price.
	if( NULL != pclSmartDpCDpSensorSet )
	{
		FillQtyPriceCol( pclSheetDescription, lFirstRow, pclSmartDpCDpSensorSet, iQuantity );
	}
	else
	{
		FillQtyPriceCol( pclSheetDescription, lFirstRow, pclSmartDpC, iQuantity );
	}
	
	// Column 'Article number'.
	lRow = lFirstRow;
	AddStaticText( pclSheetDescription, ColumnDescription::ArticleNumber, lRow, _T("-") );

	if( true == strArtNumber.IsEmpty() )
	{
		strArtNumber = pclSmartDpC->GetBodyArtNum();
	}

	if( NULL != pclSmartDpCDpSensorSet )
	{
		FillArtNumberColForPackage( pclSheetDescription, ColumnDescription::ArticleNumber, lRow, pclSmartDpCDpSensorSet, strArtNumber );
	}
	else
	{
		FillArtNumberCol( pclSheetDescription, ColumnDescription::ArticleNumber, lFirstRow, pclSmartDpC, strArtNumber );
	}

	return lRow;
}

long CSelProdPageSmartDpC::_FillRowSmartDpCInfo( CSheetDescription *pclSheetDescription, long lRow, 
		SmartDpCParams &rSmartDpCParams )
{
	if( NULL == pclSheetDescription || ( NULL == rSmartDpCParams.pclSSelSmartDpC && NULL == rSmartDpCParams.pclHM ) )
	{
		return lRow;
	}

	if( NULL != rSmartDpCParams.pclSSelSmartDpC && NULL == dynamic_cast<CDB_SmartControlValve *>( rSmartDpCParams.pclSSelSmartDpC->GetSmartDpCIDPtr().MP ) )
	{
		return lRow;
	}

	if( NULL != rSmartDpCParams.pclHM && 
			( NULL == rSmartDpCParams.pclHM->GetpSmartDpC() || NULL == rSmartDpCParams.pclHM->GetpSmartDpC()->GetpSmartDpC() ) )
	{
		return lRow;
	}

	CDB_SmartControlValve *pclSmartDpC = NULL;

	if( NULL != rSmartDpCParams.pclSSelSmartDpC )
	{
		pclSmartDpC = (CDB_SmartControlValve *)( rSmartDpCParams.pclSSelSmartDpC->GetSmartDpCIDPtr().MP );
	}
	else
	{
		pclSmartDpC = rSmartDpCParams.pclHM->GetpSmartDpC()->GetpSmartDpC();
	}

	CString str1;
	int iValidity = -1;
	CString strErrorMessage = _T("");
	CString str;
	
	// Column Infos.
	AddStaticText( pclSheetDescription, ColumnDescription::TechnicalInfos, lRow, _T("-") );			// Default string

	// HYS-2067
	if( true == m_pTADSPageSetup->GetField( epfSMARTDPCTECHINFOFLOW ) || true == m_pTADSPageSetup->GetField( epfSMARTDPCTECHINFOPOWERMAX ) )
	{
		if( rSmartDpCParams.dFlowMax > 0.0 )
		{
			CString strFlowErrorMsg = _T( "" );
			CString strPowerErrorMsg = _T( "" );

			// Check if flow or/and power error.
			if( NULL != rSmartDpCParams.pclHM && NULL != rSmartDpCParams.pclHM->GetpSmartControlValve() )
			{
				// Update error messages.
				iValidity = rSmartDpCParams.pclHM->GetpSmartDpC()->CheckValidity();

				if( CDS_HydroMod::eValidityFlags::evfFlowTooLow == (iValidity & CDS_HydroMod::eValidityFlags::evfFlowTooLow)
					|| CDS_HydroMod::eValidityFlags::evfFlowTooHigh == (iValidity & CDS_HydroMod::eValidityFlags::evfFlowTooHigh) )
				{
					strFlowErrorMsg = rSmartDpCParams.pclHM->GetpSmartDpC()->GetErrorMessageString( CDS_HydroMod::CSmartDpC::eErrorMessageObject_SmartDpC,
						CDS_HydroMod::ErrorWarningMessageFlags::EWMF_Flow );
				}

				if( CDS_HydroMod::eValidityFlags::evfPowerTooLow == (iValidity & CDS_HydroMod::eValidityFlags::evfPowerTooLow)
					|| CDS_HydroMod::eValidityFlags::evfPowerTooHigh == (iValidity & CDS_HydroMod::eValidityFlags::evfPowerTooHigh) )
				{
					strPowerErrorMsg = rSmartDpCParams.pclHM->GetpSmartDpC()->GetErrorMessageString( CDS_HydroMod::CSmartDpC::eErrorMessageObject_SmartDpC,
						CDS_HydroMod::ErrorWarningMessageFlags::EWMF_Power );
				}
			}

			// Show power dt info when their radio button is checked.
			if( false == rSmartDpCParams.strPowerDT.IsEmpty() && true == m_pTADSPageSetup->GetField( epfSMARTDPCTECHINFOPOWERMAX ) )
			{
				// Power.
				if( true == strPowerErrorMsg.IsEmpty() )
				{
					lRow = AddStaticText( pclSheetDescription, ColumnDescription::TechnicalInfos, lRow, rSmartDpCParams.strPowerDT );
				}
				else
				{
					pclSheetDescription->WriteTextWithFlags( rSmartDpCParams.strPowerDT, ColumnDescription::TechnicalInfos, lRow, CSheetDescription::RemarkFlags::ErrorMessage, strPowerErrorMsg, _RED );
					AddTooltipMessage( CPoint( ColumnDescription::TechnicalInfos, lRow ), strPowerErrorMsg );
					lRow++;
				}

				// Flow max between parenthesis.
				CString str = _T( "(" ) + TASApp.LoadLocalizedString( IDS_SSHEETSELPROD_SMARTCONTROLVAVEQMAX );
				str += WriteCUDouble( _U_FLOW, rSmartDpCParams.dFlowMax, true );
				str += _T( ")" );

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
			else if( true == m_pTADSPageSetup->GetField( epfSMARTDPCTECHINFOFLOW ) )
			{
				CString str = TASApp.LoadLocalizedString( IDS_SSHEETSELPROD_SMARTCONTROLVAVEQMAX );
				str += _T( ": " ) + CString( WriteCUDouble( _U_FLOW, rSmartDpCParams.dFlowMax, true ) );

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

	// This value is displayed only for the hydraulic calculator and not for the product selection.
	if( true == m_pTADSPageSetup->GetField( epfSMARTDPCTECHINFODP ) && NULL != rSmartDpCParams.pclHM && rSmartDpCParams.dDp > 0.0 )
	{
		strErrorMessage = _T("");

		str = TASApp.LoadLocalizedString( IDS_SSHEETSELPROD_SMARTDPCDP );
		str += _T(": ");
		str += ( rSmartDpCParams.dDp > 0.0 ) ? WriteCUDouble( _U_DIFFPRESS, rSmartDpCParams.dDp, true ) : GetDashDotDash();

		// Check if pressure drop is in error.
		// Update error messages.
		iValidity = rSmartDpCParams.pclHM->GetpSmartDpC()->CheckValidity();

		strErrorMessage = rSmartDpCParams.pclHM->GetpSmartDpC()->GetErrorMessageString( 
					CDS_HydroMod::CSmartDpC::eErrorMessageObject_SmartDpC, CDS_HydroMod::ErrorWarningMessageFlags::EWMF_Dp );

		if( true == strErrorMessage.IsEmpty() )
		{
			lRow = AddStaticText( pclSheetDescription, ColumnDescription::TechnicalInfos, lRow, str );
		}
		else
		{
			WriteTextWithFlags( pclSheetDescription, str, ColumnDescription::TechnicalInfos, lRow, CSheetDescription::RemarkFlags::ErrorMessage, strErrorMessage, _RED );
			AddTooltipMessage( CPoint( ColumnDescription::TechnicalInfos, lRow ), strErrorMessage );
			lRow++;
		}
	}

	if( true == m_pTADSPageSetup->GetField( epfSMARTDPCTECHINFODPMIN ) )
	{
		// Show Dp min only if different from -1.
		str = TASApp.LoadLocalizedString( IDS_SSHEETSELPROD_SMARTDPCDPMIN );
		str += _T(": ");
		str += ( rSmartDpCParams.dDpMin != -1.0 ) ? WriteCUDouble( _U_DIFFPRESS, rSmartDpCParams.dDpMin, true ) : GetDashDotDash();
		lRow = AddStaticText( pclSheetDescription, ColumnDescription::TechnicalInfos, lRow, str );
	}

	// This value is displayed only for product selection and not for hydraulic calculation.
	if( true == m_pTADSPageSetup->GetField( epfSMARTDPCTECHINFODPMAX ) && NULL != rSmartDpCParams.pclSSelSmartDpC 
			&& true == rSmartDpCParams.pclSSelSmartDpC->IsCheckboxDpMaxChecked() && rSmartDpCParams.pclSSelSmartDpC->GetDpMaxValue() > 0.0 )	
	{
		strErrorMessage = _T("");

		str = TASApp.LoadLocalizedString( IDS_SSHEETSELPROD_SMARTDPCDPMAX );
		str += _T(" = ") + CString( WriteCUDouble( _U_DIFFPRESS, rSmartDpCParams.pclSSelSmartDpC->GetDpMaxValue(), true ) );

		if( rSmartDpCParams.pclSSelSmartDpC->GetDpMaxValue() > pclSmartDpC->GetDpmax() )
		{
			// The pressure drop accross the valve is higher that the maximum allowed (%1 > %2).
			CString strDp = WriteCUDouble( _U_DIFFPRESS,  rSmartDpCParams.pclSSelSmartDpC->GetDpMaxValue() );
			CString strMaxDp = WriteCUDouble( _U_DIFFPRESS, pclSmartDpC->GetDpmax(), true );
			FormatString( strErrorMessage, IDS_HYDROMOD_SMARTVALVEDPTOOHIGH, strDp, strMaxDp );
		}

		if( true == strErrorMessage.IsEmpty() )
		{
			lRow = AddStaticText( pclSheetDescription, ColumnDescription::TechnicalInfos, lRow, str );
		}
		else
		{
			// Add tooltip.
			AddTooltipMessage( CPoint( ColumnDescription::TechnicalInfos, lRow ), strErrorMessage );

			// Add remark to put below.
			WriteTextWithFlags( pclSheetDescription, str, ColumnDescription::TechnicalInfos, lRow, CSheetDescription::RemarkFlags::ErrorMessage, strErrorMessage, _RED );
			lRow++;
		}
	}

	if( true == m_pTADSPageSetup->GetField( epfSMARTDPCTECHINFOLOCALIZATION ) && SmartValveLocalization::SmartValveLocNone != rSmartDpCParams.eLocalization )
	{
		CString str = TASApp.LoadLocalizedString( IDS_SSHEETSELPROD_SMARTDPCLOCALIZATION );

		int iIDS = ( SmartValveLocalization::SmartValveLocSupply == rSmartDpCParams.eLocalization ) ? IDS_SSHEETSELPROD_SMARTVALVELOCALIZATION_SUPPLY : 
				IDS_SSHEETSELPROD_SMARTVALVELOCALIZATION_RETURN;

		str += _T(": ") + TASApp.LoadLocalizedString( iIDS );

		lRow = AddStaticText( pclSheetDescription, ColumnDescription::TechnicalInfos, lRow, str );
	}

	return lRow;
}

long CSelProdPageSmartDpC::_FillRowDpSensor( CSheetDescription *pclSheetDescription, long lRow, CDS_SSelSmartDpC *pclSSelSmartDpC )
{
	if( NULL == pclSheetDescription || NULL == pclSheetDescription->GetSSheetPointer() || NULL == pclSSelSmartDpC )
	{
		return lRow;
	}

	CDB_DpSensor *pclDpSensor = dynamic_cast<CDB_DpSensor *>( pclSSelSmartDpC->GetDpSensorIDPtr().MP );
	
	if( NULL == pclDpSensor )
	{
		return lRow;
	}

	CSSheet *pclSheet = pclSheetDescription->GetSSheetPointer();
	CString str1, str2;
	long lFirstRow = lRow;

	pclSheet->SetTextPattern( CSSheet::_SSTextPattern::UserStaticColumn );
	pclSheet->SetTextPatternProperty( CSSheet::_SSTextPatternProperty::TPP_FontBold, (LPARAM)FALSE );

	// Column TA Product.
	bool bIsSet = pclSSelSmartDpC->IsSelectedAsAPackage( true );
	_FillRowDpSensorProduct( pclSheetDescription, lRow, bIsSet, pclSSelSmartDpC );

	if( false == pclSSelSmartDpC->IsFromDirSel() )
	{
		// Column infos.
		if( false == pclSSelSmartDpC->IsFromDirSel() )
		{
			_FillRowDpSensorInfo( pclSheetDescription, lFirstRow, false, pclSSelSmartDpC );
		}
	}

	return pclSheet->GetMaxRows();
}

long CSelProdPageSmartDpC::_FillRowDpSensor( CSheetDescription *pclSheetDescription, long lRow, CDS_HydroMod *pclHM )
{
	if( NULL == pclSheetDescription || NULL == pclSheetDescription->GetSSheetPointer() || NULL == pclHM || NULL == pclHM->GetpSmartDpC()
			|| NULL == pclHM->GetpSmartDpC()->GetpDpSensor() )
	{
		return lRow;
	}

	CSSheet *pclSheet = pclSheetDescription->GetSSheetPointer();
	CString str1, str2;
	long lFirstRow = lRow;

	CDB_DpSensor *pclDpSensor = pclHM->GetpSmartDpC()->GetpDpSensor();
	bool bIsSelectedAsAPackage = ( eBool3::eb3True == pclHM->GetpSmartDpC()->IsSelectedAsaPackage() ) ? true : false;

	pclSheet->SetTextPattern( CSSheet::_SSTextPattern::UserStaticColumn );
	pclSheet->SetTextPatternProperty( CSSheet::_SSTextPatternProperty::TPP_FontBold, (LPARAM)FALSE );

	// Column TA Product.
	_FillRowDpSensorProduct( pclSheetDescription, lRow, bIsSelectedAsAPackage, NULL, pclHM );

	// Column infos.
	_FillRowDpSensorInfo( pclSheetDescription, lFirstRow, bIsSelectedAsAPackage, NULL, pclHM );

	return pclSheet->GetMaxRows();
}

long CSelProdPageSmartDpC::_FillRowDpSensorProduct( CSheetDescription *pclSheetDescription, long lRow, bool bIsSet, 
		CDS_SSelSmartDpC *pclSSelSmartDpC, CDS_HydroMod *pclHM )
{
	if( NULL == pclSheetDescription || ( NULL == pclSSelSmartDpC && NULL == pclHM ) )
	{
		return lRow;
	}

	CDB_DpSensor *pclDpSensor = NULL;

	if( NULL != pclSSelSmartDpC )
	{
		pclDpSensor = dynamic_cast<CDB_DpSensor *>( pclSSelSmartDpC->GetDpSensorIDPtr().MP );
	}

	if( NULL == pclDpSensor )
	{
		if( NULL != pclSSelSmartDpC && true == bIsSet &&  NULL != dynamic_cast<CDB_Set *>( pclSSelSmartDpC->GetSetIDPtr().MP ) )
		{
			pclDpSensor = dynamic_cast<CDB_DpSensor *>( ( (CDB_Set *)( pclSSelSmartDpC->GetSetIDPtr().MP ) )->GetSecondIDPtr().MP );
		}
	}

	if( NULL == pclDpSensor )
	{
		if( NULL != pclHM && NULL != pclHM->GetpSmartDpC() )
		{
			pclDpSensor = pclHM->GetpSmartDpC()->GetpDpSensor();
		}
	}

	if( NULL == pclDpSensor )
	{
		return lRow;
	}

	int iQuantity = 1;

	if( NULL != pclSSelSmartDpC )
	{
		pclDpSensor = dynamic_cast<CDB_DpSensor *>( pclSSelSmartDpC->GetDpSensorIDPtr().MP );

		if( NULL == pclDpSensor )
		{
			CDB_Set *pclSet = (CDB_Set *)( pclSSelSmartDpC->GetSetIDPtr().MP );
			pclDpSensor = dynamic_cast<CDB_DpSensor *>( pclSet->GetSecondIDPtr().MP );
		}

		iQuantity = pclSSelSmartDpC->GetpSelectedInfos()->GetQuantity();
	}
	else
	{
		pclDpSensor = pclHM->GetpSmartDpC()->GetpDpSensor();
	}

	long lFirstRow = lRow;
	int iValidity = -1;
	CString strErrorMessage = _T("");
	CString str;

	// Name.
	str = pclDpSensor->GetName();

	// Check if there is error on the Dp sensor.
	std::vector<CString> vecDpSensorErrorMsgs;

	if( NULL != pclSSelSmartDpC )
	{
		if( true == pclSSelSmartDpC->IsCheckboxDpBranchChecked() && pclSSelSmartDpC->GetDpBranchValue() > 0.0 )
		{
			if( pclSSelSmartDpC->GetDpBranchValue() > pclDpSensor->GetMaxMeasurableDp() )
			{
				// Dp to stabilize is above the maximum measurable pressure drop of the current selected Dp sensor (%1 > %2).
				CString strDpl = WriteCUDouble( _U_DIFFPRESS, pclSSelSmartDpC->GetDpBranchValue() );
				CString strMaxMeasurableDpl = WriteCUDouble( _U_DIFFPRESS, pclDpSensor->GetMaxMeasurableDp(), true );
				FormatString( strErrorMessage, IDS_HYDROMOD_SMARTDPCDPLTOOHIGH, strDpl, strMaxMeasurableDpl );
				vecDpSensorErrorMsgs.push_back( strErrorMessage );
			}
			else if( pclSSelSmartDpC->GetDpBranchValue() < pclDpSensor->GetMinMeasurableDp() )
			{
				// Dp to stabilize is below the minimum measurable pressure drop of the currenct selected Dp sensor (%1 < %2).
				CString strDpl = WriteCUDouble( _U_DIFFPRESS, pclSSelSmartDpC->GetDpBranchValue() );
				CString strMinMeasurableDpl = WriteCUDouble( _U_DIFFPRESS, pclDpSensor->GetMinMeasurableDp(), true );
				FormatString( strErrorMessage, IDS_HYDROMOD_SMARTDPCDPLTOOLOW, strDpl, strMinMeasurableDpl );
				vecDpSensorErrorMsgs.push_back( strErrorMessage );
			}
		}

		double dFluidTemp = pclSSelSmartDpC->GetpSelectedInfos()->GetpWCData()->GetTemp();

		if( dFluidTemp < pclDpSensor->GetTmin() )
		{
			// The fluid temperature is lower that the minimum allowed for %1 (%2 < %3).
			CString strProductName = pclDpSensor->GetName();
			CString strTemp = WriteCUDouble( _U_TEMPERATURE, dFluidTemp );
			CString strMinTemp = WriteCUDouble( _U_TEMPERATURE, pclDpSensor->GetTmin(), true );
			FormatString( strErrorMessage, IDS_HYDROMOD_TEMPTOOLOWFORPRODUCT, strProductName, strTemp, strMinTemp );
			vecDpSensorErrorMsgs.push_back( strErrorMessage );
		}
				
		if( dFluidTemp > pclDpSensor->GetTmax() )
		{
			// The fluid temperature is higher that the maximum allowed for %1 (%2 > %3).
			CString strProductName = pclDpSensor->GetName();
			CString strTemp = WriteCUDouble( _U_TEMPERATURE, dFluidTemp );
			CString strMaxTemp = WriteCUDouble( _U_TEMPERATURE, pclDpSensor->GetTmax(), true );
			FormatString( strErrorMessage, IDS_HYDROMOD_TEMPTOOHIGHFORPRODUCT, strProductName, strTemp, strMaxTemp );
			vecDpSensorErrorMsgs.push_back( strErrorMessage );
		}
	}
	else
	{
		// Update error messages.
		iValidity = pclHM->GetpSmartDpC()->CheckValidity();

		vecDpSensorErrorMsgs = pclHM->GetpSmartDpC()->GetErrorMessageList( CDS_HydroMod::CSmartDpC::eErrorMessageObject_DpSensor,
				CDS_HydroMod::ErrorWarningMessageFlags::EWMF_All );
	}

	if( 0 == (int)vecDpSensorErrorMsgs.size() )
	{
		lRow = AddStaticText( pclSheetDescription, ColumnDescription::Product, lRow, str );
	}
	else
	{
		// Add tooltip.
		AddTooltipMessage( CPoint( ColumnDescription::Product, lRow ), vecDpSensorErrorMsgs );

		// Add remark to put below.
		WriteTextWithFlags( pclSheetDescription, str, ColumnDescription::Product, lRow, CSheetDescription::RemarkFlags::ErrorMessage, vecDpSensorErrorMsgs, _RED );
		lRow++;
	}

	// Dpl range.
	str = TASApp.LoadLocalizedString( IDS_SSHEETSELPROD_DPSENSORDPLRANGE );
	str += _T(": ");
	str += pclDpSensor->GetFormatedDplRange( true ).c_str();

	// Check if dp to stabilize is in error.
	strErrorMessage = _T("");

	if( NULL != pclSSelSmartDpC )
	{
		if( true == pclSSelSmartDpC->IsCheckboxDpBranchChecked() && pclSSelSmartDpC->GetDpBranchValue() > 0.0 )
		{
			if( pclSSelSmartDpC->GetDpBranchValue() < pclDpSensor->GetMinMeasurableDp() )
			{
				// Dp to stabilize is below the minimum measurable pressure drop of the currenct selected Dp sensor (%1 < %2).
				CString strDpl = WriteCUDouble( _U_DIFFPRESS, pclSSelSmartDpC->GetDpBranchValue() );
				CString strMinMeasurableDpl = WriteCUDouble( _U_DIFFPRESS, pclDpSensor->GetMinMeasurableDp(), true );
				FormatString( strErrorMessage, IDS_HYDROMOD_SMARTDPCDPLTOOLOW, strDpl, strMinMeasurableDpl );	
			}
			else if( pclSSelSmartDpC->GetDpBranchValue() > pclDpSensor->GetMaxMeasurableDp() )
			{
				// Dp to stabilize is above the maximum measurable pressure drop of the current selected Dp sensor (%1 > %2).
				CString strDpl = WriteCUDouble( _U_DIFFPRESS, pclSSelSmartDpC->GetDpBranchValue() );
				CString strMaxMeasurableDpl = WriteCUDouble( _U_DIFFPRESS, pclDpSensor->GetMaxMeasurableDp(), true );
				FormatString( strErrorMessage, IDS_HYDROMOD_SMARTDPCDPLTOOHIGH, strDpl, strMaxMeasurableDpl );
			}
		}
	}
	else
	{
		// Update error messages if needed.
		if( -1 == iValidity )
		{
			iValidity = pclHM->GetpSmartDpC()->CheckValidity();
		}

		strErrorMessage = pclHM->GetpSmartDpC()->GetErrorMessageString( CDS_HydroMod::CSmartDpC::eErrorMessageObject_DpSensor,
				CDS_HydroMod::ErrorWarningMessageFlags::EWMF_Dpl );
	}

	if( true == strErrorMessage.IsEmpty() )
	{
		lRow = AddStaticText( pclSheetDescription, ColumnDescription::Product, lRow, str );
	}
	else
	{
		// Add tooltip.
		AddTooltipMessage( CPoint( ColumnDescription::Product, lRow ), strErrorMessage );

		// Add remark to put below.
		WriteTextWithFlags( pclSheetDescription, str, ColumnDescription::Product, lRow, CSheetDescription::RemarkFlags::ErrorMessage, strErrorMessage, _RED );
		lRow++;
	}

	// Set PN/Temp range.
	long lRowTemp = lRow;
	lRow = FillPNTminTmax( pclSheetDescription, ColumnDescription::Product, lRow, pclDpSensor );

	if( NULL != pclSSelSmartDpC )
	{
		double dFluidTemp = pclSSelSmartDpC->GetpSelectedInfos()->GetpWCData()->GetTemp();

		if( dFluidTemp < pclDpSensor->GetTmin() )
		{
			// The fluid temperature is lower that the minimum allowed for %1 (%2 < %3).
			CString strProductName = pclDpSensor->GetName();
			CString strTemp = WriteCUDouble( _U_TEMPERATURE, dFluidTemp );
			CString strMinTemp = WriteCUDouble( _U_TEMPERATURE, pclDpSensor->GetTmin(), true );
			FormatString( strErrorMessage, IDS_HYDROMOD_TEMPTOOLOWFORPRODUCT, strProductName, strTemp, strMinTemp );
		}
				
		if( dFluidTemp > pclDpSensor->GetTmax() )
		{
			// The fluid temperature is higher that the maximum allowed for %1 (%2 > %3).
			CString strProductName = pclDpSensor->GetName();
			CString strTemp = WriteCUDouble( _U_TEMPERATURE, dFluidTemp );
			CString strMaxTemp = WriteCUDouble( _U_TEMPERATURE, pclDpSensor->GetTmax(), true );
			FormatString( strErrorMessage, IDS_HYDROMOD_TEMPTOOHIGHFORPRODUCT, strProductName, strTemp, strMaxTemp );
		}
	}
	else
	{
		// Update error messages if needed.
		if( -1 == iValidity )
		{
			iValidity = pclHM->GetpSmartDpC()->CheckValidity();
		}

		strErrorMessage = pclHM->GetpSmartDpC()->GetErrorMessageString( CDS_HydroMod::CSmartDpC::eErrorMessageObject_DpSensor,
				CDS_HydroMod::ErrorWarningMessageFlags::EWMF_Temp );
	}

	if( false == strErrorMessage.IsEmpty() )
	{
		// Set the PN/temp text in red.
		pclSheetDescription->GetSSheetPointer()->SetForeColor( ColumnDescription::Product, lRowTemp, _RED );

		AddTooltipMessage( CPoint( ColumnDescription::Product, lRowTemp ), strErrorMessage );

		// Add remark to put below.
		CString strCellText = pclSheetDescription->GetSSheetPointer()->GetCellText( ColumnDescription::Product, lRowTemp );
		WriteTextWithFlags( pclSheetDescription, strCellText, ColumnDescription::Product, lRowTemp, CSheetDescription::RemarkFlags::ErrorMessage, strErrorMessage, _RED );
	}
		
	// Columns quantity and price.
	FillQtyPriceCol( pclSheetDescription, lFirstRow, pclDpSensor, iQuantity, !bIsSet );
	
	// Column 'Article number'.
	AddStaticText( pclSheetDescription, ColumnDescription::ArticleNumber, lFirstRow, _T("-") );

	if( false == bIsSet )
	{
		FillArtNumberCol( pclSheetDescription, ColumnDescription::ArticleNumber, lFirstRow, pclDpSensor, pclDpSensor->GetArtNum( true ) );
	}
	else
	{
		if( true == m_pTADSPageSetup->GetField( PageField_enum::epfARTICLE ) )
		{
			lRow = AddStaticText( pclSheetDescription, ColumnDescription::ArticleNumber, lFirstRow, _T("-") );
		}
	}

	return lRow;
}

long CSelProdPageSmartDpC::_FillRowDpSensorInfo( CSheetDescription *pclSheetDescription, long lRow, bool bIsSet, CDS_SSelSmartDpC *pclSSelSmartDpC, CDS_HydroMod *pclHM )
{
	if( NULL == pclSheetDescription || ( NULL == pclSSelSmartDpC && NULL == pclHM ) )
	{
		return lRow;
	}

	CDB_DpSensor *pclDpSensor = NULL;

	if( NULL != pclSSelSmartDpC )
	{
		pclDpSensor = dynamic_cast<CDB_DpSensor *>( pclSSelSmartDpC->GetDpSensorIDPtr().MP );
	}

	if( NULL == pclDpSensor )
	{
		if( NULL != pclSSelSmartDpC && true == bIsSet &&  NULL != dynamic_cast<CDB_Set *>( pclSSelSmartDpC->GetSetIDPtr().MP ) )
		{
			pclDpSensor = dynamic_cast<CDB_DpSensor *>( ( (CDB_Set *)( pclSSelSmartDpC->GetSetIDPtr().MP ) )->GetSecondIDPtr().MP );
		}
		else if( NULL != pclSSelSmartDpC && true == bIsSet && NULL != pclSSelSmartDpC->GetSetIDPtr().MP )
		{
			// HYS-1992: Fictif set Dp Sensor and Connection set.
			pclDpSensor = dynamic_cast<CDB_DpSensor*>(pclSSelSmartDpC->GetSetIDPtr().MP);
		}
	}

	if( NULL == pclDpSensor )
	{
		if( NULL != pclHM && NULL != pclHM->GetpSmartDpC() )
		{
			pclDpSensor = pclHM->GetpSmartDpC()->GetpDpSensor();
		}
	}

	if( NULL == pclDpSensor )
	{
		return lRow;
	}

	bool bIsDpToStabilizeSet = false;
	double dDpToStabilize = 0.0;

	if( NULL != pclSSelSmartDpC )
	{
		dDpToStabilize = pclSSelSmartDpC->GetDpBranchValue();
	}
	else
	{
		//pclDpSensor = pclHM->GetpSmartDpC()->GetpDpSensor();
		bIsDpToStabilizeSet = true;
		dDpToStabilize = pclHM->GetpSmartDpC()->GetDpToStabilize();
	}

	CString str1;
	int iValidity = -1;
	CString strErrorMessage = _T("");
	CString str;
	
	// Column Infos.
	AddStaticText( pclSheetDescription, ColumnDescription::TechnicalInfos, lRow, _T("-") );			// Default string

	// Dp to stabilize.
	if( true == bIsDpToStabilizeSet && dDpToStabilize > 0.0 )
	{
		CString str = TASApp.LoadLocalizedString( IDS_SSHEETSELPROD_DPSENSORDPL );
		str += _T(": ") + CString( WriteCUDouble( _U_DIFFPRESS, dDpToStabilize, true ) );
		
		// Check if dp to stabilize is in error.
		if( NULL != pclSSelSmartDpC )
		{
			if( dDpToStabilize > pclDpSensor->GetMaxMeasurableDp() )
			{
				// Dp to stabilize is above the maximum measurable pressure drop of the current selected Dp sensor (%1 > %2).
				CString strDpl = WriteCUDouble( _U_DIFFPRESS, pclSSelSmartDpC->GetDpBranchValue() );
				CString strMaxMeasurableDpl = WriteCUDouble( _U_DIFFPRESS, pclDpSensor->GetMaxMeasurableDp(), true );
				FormatString( strErrorMessage, IDS_HYDROMOD_SMARTDPCDPLTOOHIGH, strDpl, strMaxMeasurableDpl );
			}
			else if( dDpToStabilize < pclDpSensor->GetMinMeasurableDp() )
			{
				// Dp to stabilize is below the minimum measurable pressure drop of the currenct selected Dp sensor (%1 < %2).
				CString strDpl = WriteCUDouble( _U_DIFFPRESS, pclSSelSmartDpC->GetDpBranchValue() );
				CString strMinMeasurableDpl = WriteCUDouble( _U_DIFFPRESS, pclDpSensor->GetMinMeasurableDp(), true );
				FormatString( strErrorMessage, IDS_HYDROMOD_SMARTDPCDPLTOOLOW, strDpl, strMinMeasurableDpl );
			}
		}
		else
		{
			// Update error messages.
			iValidity = pclHM->GetpSmartDpC()->CheckValidity();

			if( CDS_HydroMod::eValidityFlags::evfDplTooLow == ( iValidity & CDS_HydroMod::eValidityFlags::evfDplTooLow )
					|| CDS_HydroMod::eValidityFlags::evfDplTooHigh == ( iValidity & CDS_HydroMod::eValidityFlags::evfDplTooHigh ) )
			{
				strErrorMessage = pclHM->GetpSmartDpC()->GetErrorMessageString( CDS_HydroMod::CSmartDpC::eErrorMessageObject_DpSensor, 
						CDS_HydroMod::ErrorWarningMessageFlags::EWMF_Dpl );
			}
		}

		if( true == strErrorMessage.IsEmpty() )
		{
			lRow = AddStaticText( pclSheetDescription, ColumnDescription::TechnicalInfos, lRow, str );
		}
		else
		{
			WriteTextWithFlags( pclSheetDescription, str, ColumnDescription::TechnicalInfos, lRow, CSheetDescription::RemarkFlags::ErrorMessage, strErrorMessage, _RED );
			AddTooltipMessage( CPoint( ColumnDescription::TechnicalInfos, lRow ), strErrorMessage );
			lRow++;
		}
	}
	else
	{
		CString str = TASApp.LoadLocalizedString( IDS_SSHEETSELPROD_DPSENSORDPL );
		str += _T(": ") + TASApp.LoadLocalizedString( IDS_UNKNOWN );
		lRow = AddStaticText( pclSheetDescription, ColumnDescription::TechnicalInfos, lRow, str );
	}

	return lRow;
}

long CSelProdPageSmartDpC::_FillDpSensorAccessoriesHM( CSheetDescription *pclSheetDescription, long lRow, 
		CDS_HydroMod::CSmartDpC *pclHMSmartDpC )

{
	if( NULL == pclSheetDescription || NULL == pclSheetDescription->GetSSheetPointer() || NULL == pclHMSmartDpC || NULL == pclHMSmartDpC->GetpDpSensor() 
			|| 0 == pclHMSmartDpC->GetDpSensorAccessoryCount() )
	{
		return lRow;
	}

	CDB_DpSensor *pclDpSensor = pclHMSmartDpC->GetpDpSensor();
	CDB_RuledTable *pclRuledTable = dynamic_cast<CDB_RuledTable*>( pclDpSensor->GetAccessoriesGroupIDPtr().MP );

	if( NULL != pclRuledTable )
	{
		for( int iLoopDpSensorAccessory = 0; iLoopDpSensorAccessory < pclHMSmartDpC->GetDpSensorAccessoryCount(); iLoopDpSensorAccessory++ )
		{
			CDB_Product *pclAccessory = (CDB_Product *)( pclHMSmartDpC->GetDpSensorAccessoryIDPtr( iLoopDpSensorAccessory ).MP );

			if( NULL == pclAccessory || false == pclAccessory->IsAnAccessory() )
			{
				continue;
			}
						
			bool bByPair = pclRuledTable->IsByPair( pclAccessory->GetIDPtr().ID );
			lRow = FillAccessory( pclSheetDescription, lRow, pclAccessory, false, ( false == bByPair ) ? 1 : 2 );
			lRow++;
		}
	}

	return pclSheetDescription->GetSSheetPointer()->GetMaxRows();
}

long CSelProdPageSmartDpC::_FillRowSet( CSheetDescription *pclSheetDescription, long lRow, CDS_SSelSmartDpC *pclSSelSmartDpC )
{
	if( NULL == pclSheetDescription || NULL == pclSheetDescription->GetSSheetPointer() || NULL == pclSSelSmartDpC )
	{
		return lRow;
	}

	CDB_Set *pclSet = dynamic_cast<CDB_Set *>( pclSSelSmartDpC->GetSetIDPtr().MP );
	
	if( NULL == pclSet )
	{
		return lRow;
	}

	CSSheet *pclSheet = pclSheetDescription->GetSSheetPointer();
	CString str1, str2;
	long lFirstRow = lRow;

	pclSheet->SetTextPattern( CSSheet::_SSTextPattern::UserStaticColumn );
	pclSheet->SetTextPatternProperty( CSSheet::_SSTextPatternProperty::TPP_FontBold, (LPARAM)FALSE );

	// Column TA Product.
	CDB_DpSensor *pclDpSensor = dynamic_cast<CDB_DpSensor *>( pclSet->GetSecondIDPtr().MP );

	if( NULL != pclDpSensor )
	{
		_FillRowDpSensorProduct( pclSheetDescription, lRow, true, pclSSelSmartDpC );
	}

	if( false == pclSSelSmartDpC->IsFromDirSel() )
	{
		// Column infos.
		if( false == pclSSelSmartDpC->IsFromDirSel() )
		{
			_FillRowDpSensorInfo( pclSheetDescription, lFirstRow, true, pclSSelSmartDpC );
		}
	}

	return pclSheet->GetMaxRows();
}

long CSelProdPageSmartDpC::_FillRowSet( CSheetDescription *pclSheetDescription, long lRow, CDS_HydroMod *pclHM )
{
	if( NULL == pclSheetDescription || NULL == pclSheetDescription->GetSSheetPointer() || NULL == pclHM || NULL == pclHM->GetpSmartDpC()
			|| NULL == pclHM->GetpSmartDpC()->GetpDpSensor() )
	{
		return lRow;
	}

	CSSheet *pclSheet = pclSheetDescription->GetSSheetPointer();
	CString str1, str2;
	long lFirstRow = lRow;

	CDB_DpSensor *pclDpSensor = pclHM->GetpSmartDpC()->GetpDpSensor();
	bool bIsSelectedAsAPackage = ( eBool3::eb3True == pclHM->GetpSmartDpC()->IsSelectedAsaPackage() ) ? true : false;

	pclSheet->SetTextPattern( CSSheet::_SSTextPattern::UserStaticColumn );
	pclSheet->SetTextPatternProperty( CSSheet::_SSTextPatternProperty::TPP_FontBold, (LPARAM)FALSE );

	if( NULL != pclDpSensor )
	{
		// Column TA Product.
		_FillRowDpSensorProduct( pclSheetDescription, lRow, bIsSelectedAsAPackage, NULL, pclHM );

		// Column infos.
		_FillRowDpSensorInfo( pclSheetDescription, lFirstRow, bIsSelectedAsAPackage, NULL, pclHM );
	}

	return pclSheet->GetMaxRows();
}

long CSelProdPageSmartDpC::_FillSetContent( CSheetDescription *pclSheetDescription, long lRow, CDS_SSelSmartDpC *pclSSelSmartDpC )
{
	if( NULL == pclSheetDescription || NULL == pclSheetDescription->GetSSheetPointer() || NULL == pclSSelSmartDpC 
			|| false == pclSSelSmartDpC->IsSetContentAccessoryExist() )
	{
		return lRow;
	}

	CAccessoryList *pclSetAccessoryList = pclSSelSmartDpC->GetSetContentAccessoryList();

	// HYS-1992: Display the first accessory with the set AN.
	CDB_Product* pclFictifSet = NULL;
	if( NULL == dynamic_cast<CDB_Set*>(pclSSelSmartDpC->GetSetIDPtr().MP) )
	{
		// We are with fictif sets Dp Sensor or Connection Set
		pclFictifSet = (CDB_Product*)(pclSSelSmartDpC->GetSetIDPtr().MP);
		// HYS-2007
		if( NULL != pclFictifSet )
		{
			long lRowToModify = lRow;
			lRow = FillAccessory( pclSheetDescription, lRow, pclFictifSet, true, pclSSelSmartDpC->GetpSelectedInfos()->GetQuantity(), 
								  NULL, (LPARAM)pclSSelSmartDpC );

			// Move DpSensor name from column reference to column product
			CSSheet* pclSheet = pclSheetDescription->GetSSheetPointer();
			CString DpSensorSetName = pclSheet->GetCellText( ColumnDescription::Reference1, lRowToModify );
			pclSheet->SetStaticText( ColumnDescription::Reference1, lRowToModify, _T("") );
			pclSheet->SetStaticText( ColumnDescription::Product, lRowToModify, DpSensorSetName );

			lRow++;
			// Draw dash line.
			pclSheetDescription->GetSSheetPointer()->SetCellBorder( ColumnDescription::Reference1, lRow, ColumnDescription::Footer - 1, lRow, true,
																	SS_BORDERTYPE_BOTTOM, SS_BORDERSTYLE_FINE_DASH );

		}
	}

	for( CAccessoryList::AccessoryItem rAccessoryItem = pclSetAccessoryList->GetFirst(); rAccessoryItem.IDPtr != _NULL_IDPTR; rAccessoryItem = pclSetAccessoryList->GetNext() )
	{
		CDB_Product *pclAccessory = dynamic_cast<CDB_Product *>( (CData *)( rAccessoryItem.IDPtr.MP ) );

		if( NULL == pclAccessory || false == pclAccessory->IsAnAccessory() )
		{
			continue;
		}

		bool bByPair = rAccessoryItem.fByPair;
		bool bDistributed = rAccessoryItem.fDistributed;
		int iTotalQuantity = pclSSelSmartDpC->GetpSelectedInfos()->GetQuantity();

		if( true == bByPair )
		{
			iTotalQuantity = iTotalQuantity * 2;
		}

		lRow = FillAccessory( pclSheetDescription, lRow, pclAccessory, false, iTotalQuantity, NULL, (LPARAM)pclSSelSmartDpC );

		lRow++;
	}

	return pclSheetDescription->GetSSheetPointer()->GetMaxRows();
}

long CSelProdPageSmartDpC::_FillSetContent( CSheetDescription *pclSheetDescription, long lRow, CDS_HydroMod::CSmartDpC *pclHMSmartDpC )
{
	if( NULL == pclSheetDescription || NULL == pclSheetDescription->GetSSheetPointer() || NULL == pclHMSmartDpC 
			|| 0 == pclHMSmartDpC->GetSetContentCount() )
	{
		return lRow;
	}

	CDB_Product *pclFictifSet = pclHMSmartDpC->GetpProductSet();

	// HYS-2007
	if( NULL != pclFictifSet )
	{
		long lRowToModify = lRow;
		lRow = FillAccessory( pclSheetDescription, lRow, pclFictifSet, true, 1, NULL, (LPARAM)pclHMSmartDpC );

		// Move DpSensor name from column reference to column product
		CSSheet* pclSheet = pclSheetDescription->GetSSheetPointer();
		CString DpSensorSetName = pclSheet->GetCellText( ColumnDescription::Reference1, lRowToModify );
		pclSheet->SetStaticText( ColumnDescription::Reference1, lRowToModify, _T( "" ) );
		pclSheet->SetStaticText( ColumnDescription::Product, lRowToModify, DpSensorSetName );

		lRow++;
		// Draw dash line.
		pclSheetDescription->GetSSheetPointer()->SetCellBorder( ColumnDescription::Reference1, lRow, ColumnDescription::Footer - 1, lRow, true,
																SS_BORDERTYPE_BOTTOM, SS_BORDERSTYLE_FINE_DASH );
	}
	// Smart differential pressure controler and Dp sensor accessories set.
	CDB_RuledTable *pclRuledTable = dynamic_cast<CDB_RuledTable *>(pclFictifSet->GetAccessoriesGroupIDPtr().MP );
	
	if( NULL != pclRuledTable )
	{
		for( int iLoopSmartDpCDpSensorAccSet = 0; iLoopSmartDpCDpSensorAccSet < pclHMSmartDpC->GetSetContentCount(); iLoopSmartDpCDpSensorAccSet++ )
		{
			CDB_Product *pclAccessory = (CDB_Product *)( pclHMSmartDpC->GetSetContentIDPtr( iLoopSmartDpCDpSensorAccSet ).MP );

			if( NULL == pclAccessory || false == pclAccessory->IsAnAccessory() )
			{
				continue;
			}
						
			bool bByPair = pclRuledTable->IsByPair( pclAccessory->GetIDPtr().ID );
			lRow = FillAccessory( pclSheetDescription, lRow, pclAccessory, false, ( false == bByPair ) ? 1 : 2, NULL, (LPARAM)pclHMSmartDpC );

			lRow++;
		}
	}

	return pclSheetDescription->GetSSheetPointer()->GetMaxRows();
}

void CSelProdPageSmartDpC::_AddArticleList( CDS_SSelSmartDpC *pclSSelSmartDpC )
{
	if( NULL == pclSSelSmartDpC || NULL == pclSSelSmartDpC->GetProductAs<CDB_SmartControlValve>() )
	{
		return;
	}

	CArticleGroup *pclArticleGroup = new CArticleGroup();

	if( NULL == pclArticleGroup )
	{
		return;
	}

	bool bIsSelectedAsAPackage = pclSSelSmartDpC->IsSelectedAsAPackage();

	////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
	// Smart differential pressure controller
	CDB_SmartControlValve *pclSmartDpC = pclSSelSmartDpC->GetProductAs<CDB_SmartControlValve>();
	int iTotalQty = pclSSelSmartDpC->GetpSelectedInfos()->GetQuantity();

	if( true == bIsSelectedAsAPackage && NULL != dynamic_cast<CDB_Set *>( pclSSelSmartDpC->GetSetIDPtr().MP ) )
	{
		CArticleItem *pclArticleItem = new CArticleItem();

		if( NULL == pclArticleItem )
		{
			delete pclArticleGroup;
			ASSERT_RETURN;
		}

		CDB_Set *pclSmartDpCSet = dynamic_cast<CDB_Set *>( pclSSelSmartDpC->GetSetIDPtr().MP );
		
		pclArticleItem->SetID( pclSmartDpCSet->GetIDPtr().ID );
	
		CString strArticleNumber = pclSmartDpCSet->GetReference();
		CString strLocArtNumber = LocArtNumTab.GetLocalArticleNumber( strArticleNumber );
		pclArticleItem->CheckThingAvailability( pclSmartDpCSet, strArticleNumber, strLocArtNumber );
			
		if( true == strLocArtNumber.IsEmpty() )
		{
			strLocArtNumber = _T("-");
		}

		pclArticleItem->SetArticle( strArticleNumber );
		pclArticleItem->SetLocArtNum( strLocArtNumber );
			
		CDB_SmartControlValve *pclSmartDpC = dynamic_cast<CDB_SmartControlValve *>( pclSmartDpCSet->GetFirstIDPtr().MP );
		ASSERT( NULL != pclSmartDpC );

		CDB_Product *pclProductSet = dynamic_cast<CDB_Product *>( pclSmartDpCSet->GetSecondIDPtr().MP );
		ASSERT( NULL != pclProductSet );

		if( NULL == pclSmartDpC || NULL == pclProductSet )
		{
			delete pclArticleGroup;
			ASSERT_RETURN;
		}

		CString str = pclSmartDpC->GetName();
		CString strProductSetName = pclProductSet->GetName();

		if( NULL != dynamic_cast<CDB_DpSensor *>( pclProductSet ) )
		{
			CDB_DpSensor *pclDpSensor = (CDB_DpSensor *)( pclProductSet );
			strProductSetName = pclDpSensor->GetFullName();
		}

		str += CString( _T(" + ") ) + strProductSetName + CString( _T(" ") ) + TASApp.LoadLocalizedString( IDS_SSHEETSELPROD_SET );
		pclArticleItem->SetDescription( str );
		pclArticleItem->SetQuantity( iTotalQty );

		pclArticleItem->SetPrice( TASApp.GetpTADB()->GetPrice( pclSmartDpCSet->GetArtNum( true ) ) );
		pclArticleItem->SetIsAvailable( pclSmartDpCSet->IsAvailable() );
		pclArticleItem->SetIsDeleted( pclSmartDpCSet->IsDeleted() );
			
		// Add smart differential pressure controller accessories.
		pclArticleGroup->AddArticle( pclArticleItem );
			
		// Remark: 'AddArticle' will call new CArticleContainer( pclArticleItem ) that copies content of 'pclArticleItem' and not saves the pointer.
		delete pclArticleItem;
	}
	else
	{
		pclArticleGroup->AddArticle( pclSmartDpC, iTotalQty, true );
	}

	AddAccessoriesInArticleContainer( pclSSelSmartDpC->GetAccessoryList(), NULL, iTotalQty, false );

	CSelProdPageBase::AddArticleInGroupList( pclArticleGroup );

	// HYS-1992: Fictif set article number.
	if( false == bIsSelectedAsAPackage && NULL != pclSSelSmartDpC->GetSetIDPtr().MP )
	{
		CDB_Product* pclFictifSet = (CDB_Product*)(pclSSelSmartDpC->GetSetIDPtr().MP);

		CArticleContainer* pclArticleContainer = NULL;
		CArticleGroup* pclArticleGroup = new CArticleGroup();

		if( NULL == pclArticleGroup )
		{
			ASSERT_RETURN;
		}

		CArticleItem* pclArticleItem = new CArticleItem();

		if( NULL == pclArticleItem )
		{
			delete pclArticleGroup;
			ASSERT_RETURN;
		}

		pclArticleItem->SetID( pclFictifSet->GetIDPtr().ID );

		// Check if article is available and not deleted.
		CString strArticleNumber = pclFictifSet->GetBodyArtNum();
		CString strLocArtNum = LocArtNumTab.GetLocalArticleNumber( strArticleNumber );
		pclArticleItem->CheckThingAvailability( pclFictifSet, strArticleNumber, strLocArtNum );

		if( true == strLocArtNum.IsEmpty() )
		{
			strLocArtNum = _T( "-" );
		}

		pclArticleItem->SetArticle( strArticleNumber );
		pclArticleItem->SetLocArtNum( strLocArtNum );

		pclArticleItem->SetQuantity( iTotalQty );
		// HYS-2007: Review name display.
		CString strDescription = CteEMPTY_STRING;
		if( NULL != dynamic_cast<CDB_DpSensor*>(pclFictifSet) )
		{
			strDescription = ((CDB_DpSensor*)pclFictifSet)->GetFullName();
		}
		else
		{
			strDescription = pclFictifSet->GetName();
		}

		pclArticleItem->SetDescription( strDescription );
		pclArticleItem->SetPrice( TASApp.GetpTADB()->GetPrice( pclFictifSet->GetArtNum( true ) ) );
		pclArticleItem->SetIsAvailable( pclFictifSet->IsAvailable() );
		pclArticleItem->SetIsDeleted( pclFictifSet->IsDeleted() );

		pclArticleContainer = pclArticleGroup->AddArticle( pclArticleItem );

		//CSelProdPageBase::AddArticleInGroupList( pclArticleGroup );

		// Remark: 'AddArticle' will call new CArticleContainer( pclArticleItem ) that copies content of 'pclArticleItem' and not saves the pointer.
		delete pclArticleItem;

		CSelProdPageBase::AddArticleInGroupList( pclArticleGroup );
	}
	// Remark: pay attention that 'CPage::AddArticleInGroupList' will increment quantity if article already exist or
	//         will create a new 'CArticleGroup' and will copy to it data passed as argument. This is the reason
	//         why you need to delete 'pclArticleGroup' after the call.
	delete pclArticleGroup;
	////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
	
	////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
	// Dp sensor.

	if( NULL != dynamic_cast<CDB_DpSensor*>(pclSSelSmartDpC->GetDpSensorIDPtr().MP) )
	{
		CDB_DpSensor* pclDpSensor = (CDB_DpSensor*)(pclSSelSmartDpC->GetDpSensorIDPtr().MP);

		if( false == bIsSelectedAsAPackage )
		{
			CArticleContainer* pclArticleContainer = NULL;
			CArticleGroup* pclArticleGroup = new CArticleGroup();

			if( NULL == pclArticleGroup )
			{
				ASSERT_RETURN;
			}

			CArticleItem* pclArticleItem = new CArticleItem();

			if( NULL == pclArticleItem )
			{
				delete pclArticleGroup;
				ASSERT_RETURN;
			}

			bool bIsDpToStabilizeSet = pclSSelSmartDpC->IsCheckboxDpBranchChecked();
			double dDpToStabilize = pclSSelSmartDpC->GetDpBranchValue();

			pclArticleItem->SetID( pclDpSensor->GetIDPtr().ID );

			// Check if article is available and not deleted.
			CString strArticleNumber = pclDpSensor->GetBodyArtNum();
			CString strLocArtNum = LocArtNumTab.GetLocalArticleNumber( strArticleNumber );
			pclArticleItem->CheckThingAvailability( pclDpSensor, strArticleNumber, strLocArtNum );

			if( true == strLocArtNum.IsEmpty() )
			{
				strLocArtNum = _T( "-" );
			}

			pclArticleItem->SetArticle( strArticleNumber );
			pclArticleItem->SetLocArtNum( strLocArtNum );

			pclArticleItem->SetQuantity( 1 );

			CString strDescription = pclDpSensor->GetFullName();

			if( true == bIsDpToStabilizeSet && dDpToStabilize > 0.0 )
			{
				strDescription += _T( "; " );
				strDescription += WriteCUDouble( _U_DIFFPRESS, dDpToStabilize, true );
			}

			pclArticleItem->SetDescription( strDescription );
			pclArticleItem->SetPrice( TASApp.GetpTADB()->GetPrice( pclDpSensor->GetArtNum( true ) ) );
			pclArticleItem->SetIsAvailable( pclDpSensor->IsAvailable() );
			pclArticleItem->SetIsDeleted( pclDpSensor->IsDeleted() );

			pclArticleGroup->AddArticle( pclArticleItem );

			CSelProdPageBase::AddArticleInGroupList( pclArticleGroup );

			// Remark: 'AddArticle' will call new CArticleContainer( pclArticleItem ) that copies content of 'pclArticleItem' and not saves the pointer.
			delete pclArticleItem;

			// Remark: pay attention that 'CPage::AddArticleInGroupList' will increment quantity if article already exist or
			//         will create a new 'CArticleGroup' and will copy to it data passed as argument. This is the reason
			//         why you need to delete 'pclArticleGroup' after the call.
			delete pclArticleGroup;
		}
	}
	////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
}

void CSelProdPageSmartDpC::_AddArticleList( CDS_HydroMod::CSmartDpC *pclHMSmartDpC )
{
	if( NULL == pclHMSmartDpC || NULL == pclHMSmartDpC->GetpSmartDpC() )
	{
		return;
	}

	CArticleGroup *pclArticleGroup = new CArticleGroup();

	if( NULL == pclArticleGroup )
	{
		return;
	}

	bool bIsSelectedAsAPackage = ( eBool3::eb3True == pclHMSmartDpC->IsSelectedAsaPackage() ) ? true : false;

	////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
	// Smart differential pressure controller
	CDB_SmartControlValve *pclSmartDpC = pclHMSmartDpC->GetpSmartDpC();
	int iTotalQty = 1;

	if( true == bIsSelectedAsAPackage && NULL != dynamic_cast<CDB_Product *>( pclHMSmartDpC->GetProductSetIDPtr().MP ) )
	{
		CArticleItem *pclArticleItem = new CArticleItem();

		if( NULL == pclArticleItem )
		{
			delete pclArticleGroup;
			ASSERT_RETURN;
		}

		CTableSet *pclTableSmartDpCSet = pclSmartDpC->GetTableSet();
				
		if( NULL == pclTableSmartDpCSet )
		{
			ASSERT_RETURN;
		}

		CDB_Product *pclProductSet = (CDB_Product *)( pclHMSmartDpC->GetProductSetIDPtr().MP );
		CDB_Set *pclSmartDpCSet = pclTableSmartDpCSet->FindCompatibleSet( pclSmartDpC->GetIDPtr().ID, pclProductSet->GetIDPtr().ID );
		
		pclArticleItem->SetID( pclSmartDpCSet->GetIDPtr().ID );
	
		CString strArticleNumber = pclSmartDpCSet->GetReference();
		CString strLocArtNumber = LocArtNumTab.GetLocalArticleNumber( strArticleNumber );
		pclArticleItem->CheckThingAvailability( pclSmartDpCSet, strArticleNumber, strLocArtNumber );
			
		if( true == strLocArtNumber.IsEmpty() )
		{
			strLocArtNumber = _T("-");
		}

		pclArticleItem->SetArticle( strArticleNumber );
		pclArticleItem->SetLocArtNum( strLocArtNumber );

		CString str = pclSmartDpC->GetName();
		CString strProductSetName = pclProductSet->GetName();

		if( NULL != dynamic_cast<CDB_DpSensor *>( pclProductSet ) )
		{
			CDB_DpSensor *pclDpSensor = (CDB_DpSensor *)( pclProductSet );
			strProductSetName = pclDpSensor->GetFullName();
		}

		str += CString( _T(" + ") ) + strProductSetName + CString( _T(" ") ) + TASApp.LoadLocalizedString( IDS_SSHEETSELPROD_SET );
		pclArticleItem->SetDescription( str );
		pclArticleItem->SetQuantity( iTotalQty );

		pclArticleItem->SetPrice( TASApp.GetpTADB()->GetPrice( pclSmartDpCSet->GetArtNum( true ) ) );
		pclArticleItem->SetIsAvailable( pclSmartDpCSet->IsAvailable() );
		pclArticleItem->SetIsDeleted( pclSmartDpCSet->IsDeleted() );
			
		// Add smart differential pressure controller accessories.
		pclArticleGroup->AddArticle( pclArticleItem );
			
		// Remark: 'AddArticle' will call new CArticleContainer( pclArticleItem ) that copies content of 'pclArticleItem' and not saves the pointer.
		delete pclArticleItem;
	}
	else
	{
		pclArticleGroup->AddArticle( pclSmartDpC, iTotalQty, true );
	}

	CSelProdPageBase::AddArticleInGroupList( pclArticleGroup );

	// Remark: pay attention that 'CPage::AddArticleInGroupList' will increment quantity if article already exist or
	//         will create a new 'CArticleGroup' and will copy to it data passed as argument. This is the reason
	//         why you need to delete 'pclArticleGroup' after the call.
	delete pclArticleGroup;

	CArray<IDPTR> *parAccessories = pclHMSmartDpC->GetpAccessoryIDPtrArray();
	CDB_RuledTableBase *pclRuledTable = dynamic_cast<CDB_RuledTableBase *>( pclSmartDpC->GetAccessoriesGroupIDPtr().MP );
	AddAccessoriesInArticleContainer( parAccessories, pclRuledTable, NULL, 1, false );

	////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
	

	////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
	// HYS-1992: Add article number for fictif set
	if( false == bIsSelectedAsAPackage && NULL != pclHMSmartDpC->GetProductSetIDPtr().MP )
	{
		CDB_Product* pclFictifSet = (CDB_Product*)(pclHMSmartDpC->GetProductSetIDPtr().MP);

		//if( false == bIsSelectedAsAPackage )
		//{
		CArticleContainer* pclArticleContainer = NULL;
		CArticleGroup* pclArticleGroup = new CArticleGroup();

		if( NULL == pclArticleGroup )
		{
			ASSERT_RETURN;
		}

		CArticleItem* pclArticleItem = new CArticleItem();

		if( NULL == pclArticleItem )
		{
			delete pclArticleGroup;
			ASSERT_RETURN;
		}

		pclArticleItem->SetID( pclFictifSet->GetIDPtr().ID );

		// Check if article is available and not deleted.
		CString strArticleNumber = pclFictifSet->GetBodyArtNum();
		CString strLocArtNum = LocArtNumTab.GetLocalArticleNumber( strArticleNumber );
		pclArticleItem->CheckThingAvailability( pclFictifSet, strArticleNumber, strLocArtNum );

		if( true == strLocArtNum.IsEmpty() )
		{
			strLocArtNum = _T( "-" );
		}

		pclArticleItem->SetArticle( strArticleNumber );
		pclArticleItem->SetLocArtNum( strLocArtNum );

		pclArticleItem->SetQuantity( iTotalQty );

		// HYS-2007: review name display.
		CString strDescription = CteEMPTY_STRING;
		if( NULL != dynamic_cast<CDB_DpSensor*>(pclFictifSet) )
		{
			strDescription = ((CDB_DpSensor*)pclFictifSet)->GetFullName();
		}
		else
		{
			strDescription = pclFictifSet->GetName();
		}

		pclArticleItem->SetDescription( strDescription );
		pclArticleItem->SetPrice( TASApp.GetpTADB()->GetPrice( pclFictifSet->GetArtNum( true ) ) );
		pclArticleItem->SetIsAvailable( pclFictifSet->IsAvailable() );
		pclArticleItem->SetIsDeleted( pclFictifSet->IsDeleted() );

		pclArticleContainer = pclArticleGroup->AddArticle( pclArticleItem );

		//CSelProdPageBase::AddArticleInGroupList( pclArticleGroup );

		// Remark: 'AddArticle' will call new CArticleContainer( pclArticleItem ) that copies content of 'pclArticleItem' and not saves the pointer.
		delete pclArticleItem;

		// Remark: pay attention that 'CPage::AddArticleInGroupList' will increment quantity if article already exist or
		//         will create a new 'CArticleGroup' and will copy to it data passed as argument. This is the reason
		//         why you need to delete 'pclArticleGroup' after the call.
		//delete pclArticleGroup;
	//}

		//AddAccessoriesInArticleContainer( pclSSelSmartDpC->GetSetContentAccessoryList(), pclArticleContainer, iTotalQty, true );

		CSelProdPageBase::AddArticleInGroupList( pclArticleGroup );

		//// Remark: pay attention that 'CPage::AddArticleInGroupList' will increment quantity if article already exist or
		////         will create a new 'CArticleGroup' and will copy to it data passed as argument. This is the reason
		////         why you need to delete 'pclArticleGroup' after the call.
		//delete pclArticleGroup;
	}
	// Dp sensor.
	if( NULL != pclHMSmartDpC->GetpDpSensor() )
	{
		CDB_DpSensor *pclDpSensor = pclHMSmartDpC->GetpDpSensor();

		if( false == bIsSelectedAsAPackage )
		{
			CArticleContainer *pclArticleContainer = NULL;
			CArticleGroup *pclArticleGroup = new CArticleGroup();

			if( NULL == pclArticleGroup )
			{
				return;
			}

			CArticleItem *pclArticleItem = new CArticleItem();

			if( NULL == pclArticleItem )
			{
				delete pclArticleGroup;
				ASSERT_RETURN;
			}

			bool bIsDpToStabilizeSet = true;
			double dDpToStabilize = pclHMSmartDpC->GetDpToStabilize();

			pclArticleItem->SetID( pclDpSensor->GetIDPtr().ID );
		
			// Check if article is available and not deleted.
			CString strArticleNumber = pclDpSensor->GetBodyArtNum();
			CString strLocArtNum = LocArtNumTab.GetLocalArticleNumber( strArticleNumber );
			pclArticleItem->CheckThingAvailability( pclDpSensor, strArticleNumber, strLocArtNum );
			
			if( true == strLocArtNum.IsEmpty() )
			{
				strLocArtNum = _T("-");
			}

			pclArticleItem->SetArticle( strArticleNumber );
			pclArticleItem->SetLocArtNum( strLocArtNum );

			pclArticleItem->SetQuantity( 1 );
		
			CString strDescription = pclDpSensor->GetFullName();
		
			if( true == bIsDpToStabilizeSet && dDpToStabilize > 0.0 )
			{
				strDescription += _T("; ");
				strDescription += WriteCUDouble( _U_DIFFPRESS, dDpToStabilize, true );
			}

			pclArticleItem->SetDescription( strDescription );
		
			pclArticleItem->SetPrice( TASApp.GetpTADB()->GetPrice( pclDpSensor->GetArtNum( true ) ) );

			pclArticleItem->SetIsAvailable( pclDpSensor->IsAvailable() );
			pclArticleItem->SetIsDeleted( pclDpSensor->IsDeleted() );

			pclArticleGroup->AddArticle( pclArticleItem );

			CSelProdPageBase::AddArticleInGroupList( pclArticleGroup );

			// Remark: 'AddArticle' will call new CArticleContainer( pclArticleItem ) that copies content of 'pclArticleItem' and not saves the pointer.
			delete pclArticleItem;

			// Remark: pay attention that 'CPage::AddArticleInGroupList' will increment quantity if article already exist or
			//         will create a new 'CArticleGroup' and will copy to it data passed as argument. This is the reason
			//         why you need to delete 'pclArticleGroup' after the call.
			delete pclArticleGroup;
		}

		// Dp sensor accessories.
		CArray<IDPTR> *parAccessories = pclHMSmartDpC->GetpDpSensorAccessoryIDPtrArray();
		CDB_RuledTableBase *pclRuledTable = dynamic_cast<CDB_RuledTableBase *>( pclDpSensor->GetAccessoriesGroupIDPtr().MP );
		AddAccessoriesInArticleContainer( parAccessories, pclRuledTable, NULL, 1, false );
	}
	////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
}
