#include "stdafx.h"
#include "TASelect.h"
#include "MainFrm.h"
#include "Global.h"
#include "HydroMod.h"
#include "EnBitmap.h"
#include "EnBitmapPatchWork.h"
#include "DlgLeftTabBase.h"
#include "DlgLeftTabSelP.h"
#include "RViewDescription.h"
#include "SelProdArticle.h"
#include "SelProdPageBase.h"
#include "SelProdPageBv.h"
#include "SelProdPageDpC.h"

CSelProdPageDpC::CSelProdPageDpC( CArticleGroupList *pclArticleGroupList ) 
	: CSelProdPageBase( CDB_PageSetup::enCheck::DPC, pclArticleGroupList )
{
	m_pSelected = NULL;
}

void CSelProdPageDpC::Reset( void )
{
	CSelProdPageBase::Reset();
}

bool CSelProdPageDpC::PreInit( HMvector &vecHMList )
{
	// Something for individual or direct selection ?
	CTable *pclTable = (CTable*)( TASApp.GetpTADS()->Get( _T("DPCONTR_TAB") ).MP );	
	
	if( NULL == pclTable )
	{
		ASSERT( 0 );
		return false;
	}

	m_pclSelectionTable = ( _T('\0') != *pclTable->GetFirst().ID ) ? pclTable : NULL;
	bool fDpCExist = ( NULL != m_pclSelectionTable );

	// Something for hydronic calculation?
	bool fDpCHMExist = ( vecHMList.size() > 0 );

	if( false == fDpCExist && false == fDpCHMExist )
	{
		return false;
	}
	
	m_vecHMList = vecHMList;
	SortTable();
	return true;
}

bool CSelProdPageDpC::Init( bool bResetOrder, bool bPrint )
{
	bool bDpCExist = ( NULL != m_pclSelectionTable ) ? true : false;
	bool bDpCHMExist = ( m_vecHMList.size() > 0 ) ? true : false;

	if( false == bDpCExist && false == bDpCHMExist )
	{
		return false;
	}

	// Call base class method to get a new 'Sheet'.
	CSheetDescription *pclSheetDescriptionDpC = CMultiSpreadBase::CreateSSheet( SD_DpC );

	if( NULL == pclSheetDescriptionDpC || NULL == pclSheetDescriptionDpC->GetSSheetPointer() )
	{
		return false;
	}
	
	CSSheet *pclSheet = pclSheetDescriptionDpC->GetSSheetPointer();

	if( false == CSelProdPageBase::PrepareSheet( pclSheetDescriptionDpC, ColumnDescription::Pointer, bPrint ) )
	{
		m_ViewDescription.RemoveOneSheetDescription( SD_DpC );
		return false;
	}

	// Initialize column width for header and footer column.
	if( false == m_bForPrint )
	{
		SetColWidth( pclSheetDescriptionDpC, ColumnDescription::Header, DefaultColumnWidth::DCW_Header );
		SetColWidth( pclSheetDescriptionDpC, ColumnDescription::Footer, DefaultColumnWidth::DCW_Footer );
	}
	else
	{
		pclSheet->ShowCol( ColumnDescription::Header, FALSE );
		pclSheet->ShowCol( ColumnDescription::Footer, FALSE );
	}

	double dWidth = ( true == m_pTADSPageSetup->GetField( PageField_enum::epfFIRSTREF ) ) ? DefaultColumnWidth::DCW_Reference1 : 0.0;
	SetColWidth( pclSheetDescriptionDpC, ColumnDescription::Reference1, dWidth );

	dWidth = ( true == m_pTADSPageSetup->GetField( PageField_enum::epfSECONDREF ) ) ? DefaultColumnWidth::DCW_Reference2 : 0.0;
	SetColWidth( pclSheetDescriptionDpC, ColumnDescription::Reference2, dWidth );
	
	dWidth = ( true == m_pTADSPageSetup->GetField( PageField_enum::epfWATERINFO ) ) ? DefaultColumnWidth::DCW_Water : 0.0;
	SetColWidth( pclSheetDescriptionDpC, ColumnDescription::Water, dWidth );
	
	dWidth = ( true == m_pTADSPageSetup->GetField( PageField_enum::epfDPCINFOPRODUCT ) ) ? DefaultColumnWidth::DCW_Product : 0.0;
	SetColWidth( pclSheetDescriptionDpC, ColumnDescription::Product, dWidth );
	
	dWidth = ( true == m_pTADSPageSetup->GetField( PageField_enum::epfDPCTECHINFO ) ) ? DefaultColumnWidth::DCW_TechnicalInfos : 0.0;
	SetColWidth( pclSheetDescriptionDpC, ColumnDescription::TechnicalInfos, dWidth );
	
	SetColWidth( pclSheetDescriptionDpC, ColumnDescription::RadiatorInfos, 0.0 );
	
	dWidth = ( true == m_pTADSPageSetup->GetField( PageField_enum::epfARTICLE ) ) ? DefaultColumnWidth::DCW_ArticleNumber : 0.0;
	SetColWidth( pclSheetDescriptionDpC, ColumnDescription::ArticleNumber, dWidth );
	
	dWidth = ( true == m_pTADSPageSetup->GetField( PageField_enum::epfPIPEINFO ) ) ? DefaultColumnWidth::DCW_Pipes : 0.0;
	SetColWidth( pclSheetDescriptionDpC, ColumnDescription::Pipes, dWidth );
	
	dWidth = ( true == m_pTADSPageSetup->GetField( PageField_enum::epfQUANTITY ) ) ? DefaultColumnWidth::DCW_Quantity : 0.0;
	SetColWidth( pclSheetDescriptionDpC, ColumnDescription::Quantity, dWidth );
	
	dWidth = 0.0;
	
	if( true == TASApp.IsPriceUsed() )
	{
		dWidth = ( true == m_pTADSPageSetup->GetField( PageField_enum::epfSALEUNITPRICE ) ) ? DefaultColumnWidth::DCW_UnitPrice : 0.0;
	}

	SetColWidth( pclSheetDescriptionDpC, ColumnDescription::UnitPrice, dWidth );
	
	dWidth = 0.0;
	
	if( true == TASApp.IsPriceUsed() )
	{
		dWidth = ( true == m_pTADSPageSetup->GetField( PageField_enum::epfSALETOTALPRICE ) ) ? DefaultColumnWidth::DCW_TotalPrice : 0.0;
	}

	SetColWidth( pclSheetDescriptionDpC, ColumnDescription::TotalPrice, dWidth );
	
	SetColWidth( pclSheetDescriptionDpC, ColumnDescription::Remark, ( true == m_pTADSPageSetup->GetField( PageField_enum::epfREMARK ) ) ? DefaultColumnWidth::DCW_Remark : 0 );
	
	SetColWidth( pclSheetDescriptionDpC, ColumnDescription::Pointer, DefaultColumnWidth::DCW_Pointer );
	
	pclSheet->ShowCol( ColumnDescription::RadiatorInfos, FALSE );

	if( false == TASApp.IsPriceUsed() )
	{
		pclSheet->ShowCol( ColumnDescription::UnitPrice, FALSE );
		pclSheet->ShowCol( ColumnDescription::TotalPrice, FALSE );
	}

	pclSheet->ShowCol( ColumnDescription::Pointer, FALSE );
	CRect rect = pclSheet->GetSheetSizeInPixels( false );
	m_mapSheetInfos[pclSheetDescriptionDpC].m_dPageWidth = rect.Width();
	
	// Page title.
	SetPageTitle( pclSheetDescriptionDpC, IDS_RVIEWSELP_TITLE_DPC );
	
	// Init column header.
	_InitColHeader( pclSheetDescriptionDpC );
	pclSheet->SetColumnAlwaysHidden( ColumnDescription::RadiatorInfos, true );
	
	// HYS-1571: We can optimize the column width now.
	// See Jira card to have more details about why we do that here.
	CRect rectClient;
	GetClientRect( &rectClient );
	OptimizePageSize( rectClient.Width(), rectClient.Height() );

	// Current Position.
	long lRow = pclSheet->GetMaxRows() + 1;
	long lSelectedRow = 0;

	if( true == bDpCExist )
	{
		// Create an array of SelManDpc pointer.
		int iDpControllersSelectedCount = m_pclSelectionTable->GetItemCount();

		if( 0 == iDpControllersSelectedCount )
		{
			return false;
		}
		
		CDS_SSelDpC **paSSelDpController = new CDS_SSelDpC*[iDpControllersSelectedCount];

		if( NULL == paSSelDpController )
		{
			return false;
		}

		memset( paSSelDpController, NULL, sizeof(CDS_SSelDpC *) * iDpControllersSelectedCount );

		CDS_SSelDpReliefValve **parSelectedDpReliefValve = new CDS_SSelDpReliefValve * [iDpControllersSelectedCount];

		if( NULL == parSelectedDpReliefValve )
		{
			// HYS-1877: Memory leak
			delete[] paSSelDpController;
			return false;
		}

		memset( parSelectedDpReliefValve, NULL, sizeof(CDS_SSelDpReliefValve *) * iDpControllersSelectedCount );
		
		// HYS-2007: Add DpSensor selected alone
		CDS_SSelDpSensor **paSelectedDpSensor = new CDS_SSelDpSensor * [iDpControllersSelectedCount];

		if( NULL == paSelectedDpSensor )
		{
			delete[] paSSelDpController;
			delete[] parSelectedDpReliefValve;
			return false;
		}

		memset( paSelectedDpSensor, NULL, sizeof(CDS_SSelDpSensor *) * iDpControllersSelectedCount );
		
		// Fill 'paSSelDpController' and 'parSelectedDpReliefValve' arrays with pointer on each 'CDS_SSelDpC' and 'CDS_SSelDpReliefValve' 
		// objects of the 'DPCONTR_TAB' table.
		// Fill 'paSelectedDpSensor' array with pointer on each 'CDS_SSelDpSensor' objects of the 'DPCONTR_TAB' table.
		int iDpControllerCount = 0;
		int iDpReliefValveCount = 0;
		int iDpSensorCount = 0;
		int iMaxDpControllerIndex = 0;
		int iMaxDpReliefValveIndex = 0;
		int iMaxDpSensorIndex = 0;
		
		for( IDPTR IDPtr = m_pclSelectionTable->GetFirst(); _T('\0') != *IDPtr.ID; IDPtr = m_pclSelectionTable->GetNext() )
		{
			CSelectedInfos *pInfos = IDPtr.MP->GetpSelectedInfos();

			if( NULL == pInfos )
			{
				ASSERT_CONTINUE;
			}
			
			if( NULL != dynamic_cast<CDS_SSelDpC *>( IDPtr.MP ) )
			{
				paSSelDpController[iDpControllerCount++] = (CDS_SSelDpC *)( IDPtr.MP );
				iMaxDpControllerIndex = max( iMaxDpControllerIndex, pInfos->GetRowIndex() );
			}
			else if( NULL != dynamic_cast<CDS_SSelDpReliefValve*>(IDPtr.MP) )
			{
				parSelectedDpReliefValve[iDpReliefValveCount++] = (CDS_SSelDpReliefValve *)( IDPtr.MP );
				iMaxDpReliefValveIndex = max( iMaxDpReliefValveIndex, pInfos->GetRowIndex() );
			}
			else
			{
				paSelectedDpSensor[iDpSensorCount++] = (CDS_SSelDpSensor*)( IDPtr.MP );
				iMaxDpSensorIndex = max( iMaxDpSensorIndex, pInfos->GetRowIndex() );
			}
		}
		
		if( 0 == iDpControllerCount && 0 == iDpReliefValveCount && 0 == iDpSensorCount )
		{
			delete[] paSSelDpController;
			delete[] parSelectedDpReliefValve;
			delete[] paSelectedDpSensor;
			return false;
		}

		// Initialize remark and scheme index in TADS.
		if( NULL != pDlgLeftTabSelP )
		{
			if( iDpControllerCount > 0 )
			{
				pDlgLeftTabSelP->SetRemarkIndex( CLASS( CDS_SSelDpC ), (CData **)paSSelDpController, iDpControllerCount );		 
				pDlgLeftTabSelP->SetSchemeIndex( CLASS( CDS_SSelDpC ), (CData **)paSSelDpController, iDpControllerCount );
			}

			if( iDpReliefValveCount > 0 )
			{
				pDlgLeftTabSelP->SetRemarkIndex( CLASS( CDS_SSelDpReliefValve ), (CData **)parSelectedDpReliefValve, iDpReliefValveCount );		 
			}

			if( iDpSensorCount > 0 )
			{
				pDlgLeftTabSelP->SetRemarkIndex( CLASS( CDS_SSelDpSensor ), (CData **)paSelectedDpSensor, iDpSensorCount );		 
			}
		}

		// Sort 'paSSelDpController', 'parSelectedDpReliefValve' and paSelectedDpSensor arrays.
		// Remark: 'bResetOrder' is set to 'true' only when the sorting combos are empty (No user choice) and the user clicks on the 'Apply sorting keys'.
		if( true == bResetOrder ) 
		{	
			// Reset row index.
			for( int i = 0; i < iDpControllerCount; i++ )
			{
				if( 0 == paSSelDpController[i]->GetpSelectedInfos()->GetRowIndex() )
				{
					paSSelDpController[i]->GetpSelectedInfos()->SetRowIndex( ++iMaxDpControllerIndex );
				}
			}

			for( int i = 0; i < iDpReliefValveCount; i++ )
			{
				if( 0 == parSelectedDpReliefValve[i]->GetpSelectedInfos()->GetRowIndex() )
				{
					parSelectedDpReliefValve[i]->GetpSelectedInfos()->SetRowIndex( ++iMaxDpReliefValveIndex );
				}
			}

			for( int i = 0; i < iDpSensorCount; i++ )
			{
				if( 0 == paSelectedDpSensor[i]->GetpSelectedInfos()->GetRowIndex() )
				{
					paSelectedDpSensor[i]->GetpSelectedInfos()->SetRowIndex( ++iMaxDpSensorIndex );
				}
			}
		}
		else if( PageField_enum::epfNONE != m_pTADSPageSetup->GetKey( 0 ) || PageField_enum::epfNONE != m_pTADSPageSetup->GetKey( 1 )
				|| PageField_enum::epfNONE != m_pTADSPageSetup->GetKey( 2 ) )
		{
			if( NULL != pDlgLeftTabSelP )
			{
				pDlgLeftTabSelP->SortTable( (CData**)paSSelDpController, iDpControllerCount - 1 );
				pDlgLeftTabSelP->SortTable( (CData **)parSelectedDpReliefValve, iDpReliefValveCount - 1 );
				pDlgLeftTabSelP->SortTable( (CData **)paSelectedDpSensor, iDpSensorCount - 1 );
			}

			// Reset row index.
			for( int i = 0; i < iDpControllerCount; i++ )
			{
				paSSelDpController[i]->GetpSelectedInfos()->SetRowIndex( i + 1 );
			}

			for( int i = 0; i < iDpReliefValveCount; i++ )
			{
				parSelectedDpReliefValve[i]->GetpSelectedInfos()->SetRowIndex( i + 1 );
			}

			for( int i = 0; i < iDpSensorCount; i++ )
			{
				paSelectedDpSensor[i]->GetpSelectedInfos()->SetRowIndex( i + 1 );
			}
		}

		CRank rkl;
		enum eDpControllersSwitch
		{
			SingleSelDpController,
			DirSelDpController,
			DirSelDpReliefValve,
			DirSelDpSensor,
			LastDpCSwitch
		};
		
		bool bAtLeastOneBlockPrinted = false;

		for( int iLoopGroup = 0; iLoopGroup < LastDpCSwitch; iLoopGroup++ )
		{
			int i = 0;
			int iCount = 0;
			int iEndLoop = 0;

			if( SingleSelDpController == iLoopGroup || DirSelDpController == iLoopGroup )
			{
				iEndLoop = iDpControllerCount;
			}
			else if( DirSelDpReliefValve == iLoopGroup )
			{
				iEndLoop = iDpReliefValveCount;
			}
			else
			{
				iEndLoop = iDpSensorCount;
			}

			for( ; i < iEndLoop; i++ )
			{
				CDS_SSel *pclSSel = NULL;

				if( SingleSelDpController == iLoopGroup )
				{
					if( true == paSSelDpController[i]->IsFromDirSel() )
					{
						continue;
					}

					pclSSel = paSSelDpController[i];
				}
				else if( DirSelDpController == iLoopGroup )
				{
					if( false == paSSelDpController[i]->IsFromDirSel() )
					{
						continue;
					}

					pclSSel = paSSelDpController[i];
				}
				else
				{
					if( ( DirSelDpReliefValve == iLoopGroup && false == parSelectedDpReliefValve[i]->IsFromDirSel() )
					   && ( DirSelDpSensor == iLoopGroup && false == paSelectedDpSensor[i]->IsFromDirSel() ) )
					{
						// Differential pressure relief valves are only available for direct selection.
						// It is thus not normal to reach this point.
						ASSERT_CONTINUE;
					}

					if( DirSelDpReliefValve == iLoopGroup )
					{
						pclSSel = parSelectedDpReliefValve[i];
					}
					else
					{
						pclSSel = paSelectedDpSensor[i];
					}
				}

				rkl.Add( _T(""), pclSSel->GetpSelectedInfos()->GetRowIndex(), (LPARAM)pclSSel );
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
			SetLastRow( pclSheetDescriptionDpC, lRow );

			switch( iLoopGroup )
			{
				case SingleSelDpController:
					SetPageTitle( pclSheetDescriptionDpC, IDS_SSHEETSELPROD_SUBTITLEDPCONTROLLERFROMSSEL, false, lRow );
					break;

				case DirSelDpController:
					SetPageTitle( pclSheetDescriptionDpC, IDS_SSHEETSELPROD_SUBTITLEDPCONTROLLERFROMDIRSEL, false, lRow );
					break;

				case DirSelDpReliefValve:
					SetPageTitle( pclSheetDescriptionDpC, IDS_SSHEETSELPROD_SUBTITLEDPRELIEFVALVEFROMDIRSEL, false, lRow );
					break;

				case DirSelDpSensor:
					SetPageTitle( pclSheetDescriptionDpC, IDS_SSHEETSELPROD_SUBTITLEDPSENSORFROMDIRSEL, false, lRow );
					break;
			}

			lRow = pclSheet->GetMaxRows();
			SetLastRow( pclSheetDescriptionDpC, lRow );

			CString str;
			LPARAM lpParam;
			bool bFirstPass = true;
			
			for( BOOL bContinue = rkl.GetFirst( str, lpParam ); TRUE == bContinue; bContinue = rkl.GetNext( str, lpParam ) )
			{
				CData *pclData = (CData *)lpParam;
				CDS_SSel *pclSSel = dynamic_cast<CDS_SSel *>( pclData );

				if( NULL == pclSSel )
				{
					ASSERT_CONTINUE;
				}

				if( m_pSelected == pclData )
				{
					lSelectedRow = lRow;
				}

				// Verify the product exist in the DB. If not, don't show it.
				if( SingleSelDpController == iLoopGroup || DirSelDpController == iLoopGroup )
				{
					if( NULL == dynamic_cast<CDS_SSelDpC *>( pclSSel ) )
					{
						ASSERT_CONTINUE;
					}

					if( NULL == ( (CDS_SSelDpC *)( pclSSel ) )->GetDpCIDPtr().MP )
					{
						ASSERT_CONTINUE;
					}
				}
				else if( DirSelDpReliefValve == iLoopGroup )
				{
					if( NULL == dynamic_cast<CDS_SSelDpReliefValve *>( pclSSel ) )
					{
						ASSERT_CONTINUE;
					}

					if( NULL == pclSSel->GetProductAs<CDB_DpReliefValve>() )
					{
						ASSERT_CONTINUE;
					}
				}
				else if( DirSelDpSensor == iLoopGroup )
				{
					if( NULL == dynamic_cast<CDS_SSelDpSensor *>( pclSSel ) )
					{
						ASSERT_CONTINUE;
					}

					if( NULL == pclSSel->GetProductAs<CDB_DpSensor>() )
					{
						ASSERT_CONTINUE;
					}
				}

				m_rProductParam.Clear();
				m_rProductParam.SetSheetDescription( pclSheetDescriptionDpC );
				m_rProductParam.SetSelectionContainer( (LPARAM)pclSSel );
				
				++lRow;
				long lFirstRow = lRow;
				long lRowStartBv = 0;

				if( SingleSelDpController == iLoopGroup || DirSelDpController == iLoopGroup )
				{
					CDS_SSelDpC *pclSSelDpController = dynamic_cast<CDS_SSelDpC *>( pclSSel );

					// Add item into the full article list.
					_AddDpControllerArticleList( pclSSelDpController );

					long lRowDpC = _FillRowDpController( pclSheetDescriptionDpC, lRow, pclSSelDpController );
					long lRowGen = CSelProdPageBase::FillRowGen( pclSheetDescriptionDpC, lRow, (CDS_SelProd *)pclSSelDpController );
					lRowStartBv = max( lRowGen, lRowDpC );
				}
				else if( DirSelDpReliefValve == iLoopGroup )
				{
					CDS_SSelDpReliefValve *pclSSelDpReliefValve = dynamic_cast<CDS_SSelDpReliefValve *>( pclSSel );

					// Add item into the full article list.
					_AddDpReliefValveArticleList( pclSSelDpReliefValve->GetProductAs<CDB_DpReliefValve>(), pclSSelDpReliefValve->GetpSelectedInfos()->GetQuantity() );

					long lRowDpC = _FillRowDpReliefValve( pclSheetDescriptionDpC, lRow, pclSSelDpReliefValve );
					long lRowGen = CSelProdPageBase::FillRowGen( pclSheetDescriptionDpC, lRow, (CDS_SelProd *)pclSSelDpReliefValve );
					lRowStartBv = max( lRowGen, lRowDpC );
				}
				else
				{
					CDS_SSelDpSensor *pclSSelDpSensor = dynamic_cast<CDS_SSelDpSensor *>( pclSSel );

					// Add item into the full article list.
					_AddDpSensorArticleList( pclSSelDpSensor->GetProductAs<CDB_DpSensor>(), pclSSelDpSensor, pclSSelDpSensor->GetpSelectedInfos()->GetQuantity() );

					long lRowDpC = _FillRowDpSensor( pclSheetDescriptionDpC, lRow, pclSSelDpSensor );
					long lRowGen = CSelProdPageBase::FillRowGen( pclSheetDescriptionDpC, lRow, (CDS_SelProd *)pclSSelDpSensor );
					lRowStartBv = max( lRowGen, lRowDpC );
				}

				// Add the selectable row range.
				m_rProductParam.AddRange( lFirstRow, pclSheet->GetMaxRows(), pclSSel->GetProductAs<CData>() );

				// Spanning must be done here because it's the only place where we know exactly number of lines to span!
				// Span reference #1 and #2.
				AddCellSpanW( pclSheetDescriptionDpC, ColumnDescription::Reference1, lRow, 1, pclSheet->GetMaxRows() - lRow + 1 );
				AddCellSpanW( pclSheetDescriptionDpC, ColumnDescription::Reference2, lRow, 1, pclSheet->GetMaxRows() - lRow + 1 );

				// Span water characteristic.
				AddCellSpanW( pclSheetDescriptionDpC, ColumnDescription::Water, lRow, 1, pclSheet->GetMaxRows() - lRow + 1 );
				long lRowBV = lRowStartBv;

				if( SingleSelDpController == iLoopGroup || DirSelDpController == iLoopGroup )
				{
					CDS_SSelDpC *pclSSelDpController = dynamic_cast<CDS_SSelDpC *>( pclSSel );

					// Add DpC accessories.
					if( true == pclSSelDpController->IsDpCAccessoryExist() )
					{
						// Draw dash line .
						pclSheet->SetCellBorder( ColumnDescription::Reference1, lRowStartBv, ColumnDescription::Footer - 1, lRowStartBv, true, SS_BORDERTYPE_BOTTOM, SS_BORDERSTYLE_FINE_DASH );
	
						lRowStartBv++;
						lRowStartBv = CSelProdPageBase::FillAccessories( pclSheetDescriptionDpC, lRowStartBv, pclSSelDpController->GetDpCAccessoryList(), pclSSelDpController->GetpSelectedInfos()->GetQuantity() );
					}

					// Draw dash line.
					pclSheet->SetCellBorder( ColumnDescription::Reference1, lRowStartBv, ColumnDescription::Footer - 1, lRowStartBv, true, SS_BORDERTYPE_BOTTOM, SS_BORDERSTYLE_FINE_DASH );

					lRowBV = lRowStartBv;
					CDB_RegulatingValve *pMV = pclSSelDpController->GetProductAs<CDB_RegulatingValve>();
				
					if( NULL != pMV )
					{
						CSelProdPageBv clSelProdPageBv( m_pclArticleGroupList );

						// Add infos on the selected BV.
						clSelProdPageBv.FillRowBvProduct( pclSheetDescriptionDpC, lRowStartBv + 1, pMV, pclSSelDpController->GetpSelectedInfos()->GetQuantity(), pclSSelDpController->IsSelectedAsAPackage( true ) );

						if( false == pclSSelDpController->IsFromDirSel() ) 
						{
							double dQ = pclSSelDpController->GetQ();
							double dDp = pclSSelDpController->GetDp();
							double dRho = pclSSelDpController->GetpSelectedInfos()->GetpWCData()->GetDens();
							double dNu = pclSSelDpController->GetpSelectedInfos()->GetpWCData()->GetKinVisc();
							double dPresset = pclSSelDpController->GetOpening();

							// HYS-38: Show power dt info when their radio button is checked
							CString strPowerDt = _T( "" );

							if( CDS_SelProd::efdPower == pclSSelDpController->GetFlowDef() )
							{
								strPowerDt = WriteCUDouble( _U_TH_POWER, pclSSelDpController->GetPower(), true );
								strPowerDt += _T(" / ");
								strPowerDt += WriteCUDouble( _U_DIFFTEMP, pclSSelDpController->GetDT(), true );
							}
							
							clSelProdPageBv.FillRowBvInfo( pclSheetDescriptionDpC, lRowStartBv + 1, pMV, dQ, 0, dPresset, dRho, dNu, strPowerDt );
						}

						// Because information is saved in the 'CSelProdPageBv' class, we need to manually add it here.
						m_rProductParam.AddRange( lRowStartBv + 1, pclSheet->GetMaxRows(), pMV );

						lRowBV = pclSheet->GetMaxRows();
					
						if( true == pclSSelDpController->IsAccessoryExist() )
						{
							// Draw dash line.
							pclSheet->SetCellBorder( ColumnDescription::Reference1, lRowBV, ColumnDescription::Footer - 1, lRowBV, true, SS_BORDERTYPE_BOTTOM, SS_BORDERSTYLE_FINE_DASH );

							// Add Bv accessories.
							lRowBV++;
							lRowBV = CSelProdPageBase::FillAccessories( pclSheetDescriptionDpC, lRowBV, pclSSelDpController->GetAccessoryList(), pclSSelDpController->GetpSelectedInfos()->GetQuantity() );
						}

						// Add BV infos and accessories in the 'Article list' page only if valve has not been selected as a package.
						// Remark: if selected as a package, Bv accessories have been added in 'AddArticleList' above with DpC.
						if( false == pclSSelDpController->IsSelectedAsAPackage( true ) )
						{
							// We can send 'pclSSelDpController' because regulating valve information are saved in the 'CDS_SSel' base class and this is this one that 
							// 'AddArticleInGroupList' uses.
							clSelProdPageBv.AddArticleInGroupList( pclSSelDpController );
						}
					}
				}
				else if( DirSelDpReliefValve == iLoopGroup )
				{
					// Add accessories.
					CDS_SSelDpReliefValve *pclSSelDpReliefValve = dynamic_cast<CDS_SSelDpReliefValve *>( pclSSel );

					if( true == pclSSelDpReliefValve->IsAccessoryExist() )
					{
						// Draw dash line.
						pclSheet->SetCellBorder( ColumnDescription::Reference1, lRowBV, ColumnDescription::Footer - 1, lRowBV, true, SS_BORDERTYPE_BOTTOM, SS_BORDERSTYLE_FINE_DASH );

						lRowBV++;
						lRowBV = FillAccessories( pclSheetDescriptionDpC, lRowBV, pclSSelDpReliefValve, pclSSelDpReliefValve->GetpSelectedInfos()->GetQuantity() );
					}
				}
				else
				{
					// Add accessories.
					CDS_SSelDpSensor *pclSSelDpSensor = dynamic_cast<CDS_SSelDpSensor *>( pclSSel );

					if( true == pclSSelDpSensor->IsAccessoryExist() )
					{
						// Draw dash line.
						pclSheet->SetCellBorder( ColumnDescription::Reference1, lRowBV, ColumnDescription::Footer - 1, lRowBV, true, SS_BORDERTYPE_BOTTOM, SS_BORDERSTYLE_FINE_DASH );
						CDB_DpSensor* pclDpSensor = dynamic_cast<CDB_DpSensor*>(pclSSelDpSensor->GetProductAs<CDB_DpSensor>());
						lRowBV++;
						// Hide article number for the accessory 
						lRowBV = FillAccessories( pclSheetDescriptionDpC, lRowBV, pclSSelDpSensor->GetAccessoryList(), 
												  pclSSelDpSensor->GetpSelectedInfos()->GetQuantity(), 1, NULL, !pclDpSensor->IsPartOfaSet() );
					}
				}

				// Save product.
				m_rProductParam.SetScrollRange( ( true == bFirstPass ) ? lFirstRow - 1 : lFirstRow, lRowBV );
				SaveProduct( m_rProductParam );

				// Draw line below.
				pclSheet->SetCellBorder( ColumnDescription::Reference1, lRowBV, ColumnDescription::Footer - 1, lRowBV, true, SS_BORDERTYPE_BOTTOM );

				// Set all group as no breakable (for print).
				// Remark: include title with the group.
				pclSheet->SetFlagRowNoBreakable( ( true == bFirstPass ) ? lFirstRow - 1 : lFirstRow, lRowBV, true );

				bFirstPass = false;
				lRow = lRowBV;
			}

			lRow++;
			bAtLeastOneBlockPrinted = true;
		}
		
		long lRowScheme = lRow;
		lRow = _FillDpControllerSCheme( pclSheetDescriptionDpC, lRowScheme, NULL );

		lRow++;
		FillRemarks( pclSheetDescriptionDpC, lRow );

		// We add by default a blank line.
		lRow = pclSheet->GetMaxRows() + 2;
		
		delete[] paSSelDpController;
		// HYS-1877: Memory leak
		delete[] parSelectedDpReliefValve;
	}

	if( true == bDpCHMExist )
	{
		// Something inside ?
		if( m_vecHMList.size() > 0 )
		{
			// Create a remark index.
			if( NULL != pDlgLeftTabSelP )
			{
				pDlgLeftTabSelP->SetRemarkIndex( &m_vecHMList );
			}

			SetLastRow( pclSheetDescriptionDpC, lRow );
			SetPageTitle( pclSheetDescriptionDpC, IDS_SSHEETSELPROD_SUBTITLEDPCONTROLLERFROMHM, false, lRow );	
			
			lRow++;
			SetLastRow( pclSheetDescriptionDpC, lRow );
			
			// For each element.
			bool bFirstPass = true;
			HMvector::iterator It;
			
			for( It = m_vecHMList.begin(); It != m_vecHMList.end(); It++ )
			{
				CDS_HydroMod::CDpC *pDpController = static_cast<CDS_HydroMod::CDpC *>( (*It).second );
				
				// DpC exist and TA product exist into the DB.
				if( NULL != pDpController && NULL != pDpController->GetIDPtr().MP )
				{
					m_rProductParam.Clear();
					m_rProductParam.SetSheetDescription( pclSheetDescriptionDpC );
					// Intentionally set to NULL to avoid clicking, double clicking or right clicking.
					m_rProductParam.SetSelectionContainer( (LPARAM)0 );

					// Add item into the full Article List.
					_AddDpControllerArticleList( NULL, (CDB_TAProduct *)pDpController->GetIDPtr().MP, 1 );
					
					long lRowDpC = _FillRowDpController( pclSheetDescriptionDpC, lRow, pDpController );
					// HYS-1734: Add the valve to get temperature and pipe in FillRowGen.
					long lRowGen = CSelProdPageBase::FillRowGen( pclSheetDescriptionDpC, lRow, pDpController->GetpParentHM(), pDpController );
					long lRowBV = max( lRowGen, lRowDpC );
					
					// HYS-721: Merge cells
					long lFirstRow = lRow;
					
					// Spanning must be done here because it's the only place where we know exactly number of lines to span!
					// Span reference #1 and #2.
					AddCellSpanW(pclSheetDescriptionDpC, ColumnDescription::Reference1, lFirstRow, 1, pclSheet->GetMaxRows() - lFirstRow + 1);
					AddCellSpanW(pclSheetDescriptionDpC, ColumnDescription::Reference2, lFirstRow, 1, pclSheet->GetMaxRows() - lFirstRow + 1);
					
					// Span water characteristic.
					AddCellSpanW(pclSheetDescriptionDpC, ColumnDescription::Water, lFirstRow, 1, pclSheet->GetMaxRows() - lFirstRow + 1);

					CDB_TAProduct *pclTAProd = dynamic_cast<CDB_TAProduct *>(pDpController->GetIDPtr().MP);
					lRowBV = FillAndAddBuiltInHMAccessories(pclSheet, pclSheetDescriptionDpC, pclTAProd, lRowBV);

					// Draw dash line.
					pclSheet->SetCellBorder( ColumnDescription::Reference1, lRowBV, ColumnDescription::Footer - 1, lRowBV, true, SS_BORDERTYPE_BOTTOM, SS_BORDERSTYLE_FINE_DASH );
					
					CDS_HydroMod *pHM = pDpController->GetpParentHM();

					if( NULL != pHM->GetpBv() )
					{
						CSelProdPageBv clSelProdPageBv( m_pclArticleGroupList );
						CDB_TAProduct *pclMV = NULL;
						long lRowBVProduct = clSelProdPageBv.FillRowBv( pclSheetDescriptionDpC, lRowBV + 1, pHM, pHM->GetpBv(), &pclMV );
						// HYS-1734: Add Bv general info that is different from the DpC valve.
						long lRowGen = CSelProdPageBase::FillRowGenOthers( pclSheetDescriptionDpC, lRowBV + 1, pHM, pHM->GetpBv() );
						lRowBV = max( lRowGen, lRowBVProduct );

						if( NULL != pclMV )
						{
							lRowBV = FillAndAddBuiltInHMAccessories( pclSheet, pclSheetDescriptionDpC, pclMV, lRowBV );
						}
					}

					// Save the product.
					m_rProductParam.SetScrollRange( ( true == bFirstPass ) ? lRow - 1 : lRow, lRowBV );
					SaveProduct( m_rProductParam );

					// Draw line below.
					pclSheet->SetCellBorder( ColumnDescription::Reference1, lRowBV, ColumnDescription::Footer - 1, lRowBV, true, SS_BORDERTYPE_BOTTOM );

					// Set all group as no breakable (for print).
					// Remark: include title with the group.
					pclSheet->SetFlagRowNoBreakable( ( true == bFirstPass ) ? lRow - 1 : lRow, lRowBV, true );

					lRow = ++lRowBV;
					bFirstPass = false;
				}
			}

			FillHMRemarks( &m_vecHMList, pclSheetDescriptionDpC, lRow );
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

bool CSelProdPageDpC::HasSomethingToDisplay( void )
{
	bool fDpCExist = ( NULL != m_pclSelectionTable ) ? true : false;
	bool fDpCHMExist = ( m_vecHMList.size() > 0 ) ? true : false;
	if( false == fDpCExist && false == fDpCHMExist )
		return false;
	return true;
}

void CSelProdPageDpC::_InitColHeader( CSheetDescription* pclSheetDescription )
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
	
long CSelProdPageDpC::_FillRowDpController( CSheetDescription *pclSheetDescription, long lRow, CDS_SSelDpC *pclSSelDpC )
{
	if( NULL == pclSheetDescription || NULL == pclSheetDescription->GetSSheetPointer() || NULL == pclSSelDpC ) 
	{
		return lRow;
	}

	CDB_DpController *pDpController = dynamic_cast<CDB_DpController *>( pclSSelDpC->GetDpCIDPtr().MP );

	if( NULL == pDpController )
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

	if( true == pclSSelDpC->IsSelectedAsAPackage( true ) )
	{
		pSet = dynamic_cast<CDB_Set*>( pclSSelDpC->GetDpCMvPackageIDPtr().MP );

		if( NULL == pSet )
		{
			ASSERT( 0 );
			return lRow;
		}

		ArtNum += pSet->GetReference();
	}

	_FillRowDpControllerProduct( pclSheetDescription, lRow, pDpController, pclSSelDpC->GetpSelectedInfos()->GetQuantity(), pSet, ArtNum );

	if( false == pclSSelDpC->IsFromDirSel() )
	{
		// Column infos.
		lRow = lFirstRow;
		AddStaticText( pclSheetDescription, ColumnDescription::TechnicalInfos, lRow, _T("-") );			// Default string
		
		if( true == m_pTADSPageSetup->GetField( epfDPCTECHINFOFLOW ) )
		{
			double Q = pclSSelDpC->GetQ();
			str1 = WriteCUDouble( _U_FLOW, Q, true );
			// HYS-38: Show power dt info when their radio button is checked
			CString strPowerDt = _T( "" );
			if( CDS_SelProd::efdPower == pclSSelDpC->GetFlowDef() )
			{
				strPowerDt = WriteCUDouble( _U_TH_POWER, pclSSelDpC->GetPower(), true );
				strPowerDt += _T(" / ");
				strPowerDt += WriteCUDouble( _U_DIFFTEMP, pclSSelDpC->GetDT(), true );
			}
			if( strPowerDt != _T( "" ) )
			{
				CString strQ = _T("(");
				strQ += str1;
				strQ += _T(")");
				lRow = AddStaticText( pclSheetDescription, ColumnDescription::TechnicalInfos, lRow, strPowerDt );
				lRow = AddStaticText( pclSheetDescription, ColumnDescription::TechnicalInfos, lRow, strQ );
			}
			else
			{
				lRow = AddStaticText( pclSheetDescription, ColumnDescription::TechnicalInfos, lRow, str1 );
			}
		}

		if( true == m_pTADSPageSetup->GetField( epfDPCTECHINFO2NDDP ) )
		{
			double dDpL = pclSSelDpC->GetDpL();
			// Show Dpl only if different from -1
			str2 = TASApp.LoadLocalizedString( IDS_DPL );
			
			if( DpStabOnCV == pclSSelDpC->GetDpStab() )
			{
				str1 = str2 + (CString)_T(" = ");
				
				if( pclSSelDpC->GetKv() > 0.0 )
				{
					str1 += WriteCUDouble( _U_DIFFPRESS, pclSSelDpC->GetDpToStab(), true );
				}
				else
				{
					str1 += pDpController->GetFormatedDplRange(true).c_str();
				}
			}
			else
			{
				if( dDpL <= 0.0 )
				{
					CString str3;
					str3 = TASApp.LoadLocalizedString( IDS_UNKNOWN );
					str3.MakeLower();
					str1 = str2 + (CString)_T(" ") + str3;
				}
				else
				{
					str1 = str2 + (CString)_T(" = ");
					str1 += WriteCUDouble( _U_DIFFPRESS, pclSSelDpC->GetDpToStab(), true );
				}
			}
			// HYS-1102 : Display str1 out of the else.
			lRow = AddStaticText( pclSheetDescription, ColumnDescription::TechnicalInfos, lRow, str1 );
		}

		double Dpmin = 0.0;

		if( true == m_pTADSPageSetup->GetField( epfDPCTECHINFODPMIN ) )
		{
			// Show DPMin only if different from 0.
			if( pclSSelDpC->GetQ() > 0.0)
			{
				Dpmin = pclSSelDpC->GetDpMin();
				
				if (Dpmin>0.0)
				{
					str2 = TASApp.LoadLocalizedString( IDS_DPMIN );
					str1 = str2 + (CString)_T(" = ");
					str1 += WriteCUDouble( _U_DIFFPRESS, Dpmin, true );
					lRow = AddStaticText( pclSheetDescription, ColumnDescription::TechnicalInfos, lRow, str1 );
				}
			}
		}

		if( true == m_pTADSPageSetup->GetField( epfDPCTECHINFOSETTING ) )
		{
			if( pclSSelDpC->GetDpCSetting() > 0.0 )
			{
				str1 = _T("-");
				CDB_DpCCharacteristic *pDpCCharacteristic = pDpController->GetDpCCharacteristic();

				if( NULL != pDpCCharacteristic )
				{
					str1 = pDpCCharacteristic->GetSettingString( pclSSelDpC->GetDpCSetting(), true );
				}

				lRow = AddStaticText( pclSheetDescription, ColumnDescription::TechnicalInfos, lRow, str1 );
			}
		}

		if( true == m_pTADSPageSetup->GetField( epfDPCTECHINFOHMIN ) )
		{
			double dHMin = pclSSelDpC->GetHMin();
			
			// Show HMin only if different from 0.
			if( dHMin > 0 )
			{
				if ( (pclSSelDpC->GetDpStab() == DpStabOnBranch && pclSSelDpC->GetDpL() > 0.0) ||
					 pclSSelDpC->GetDpStab() == DpStabOnCV)
				{
					str2 = TASApp.LoadLocalizedString( IDS_HMIN );
					str1 = str2 + (CString)_T(" = ");

					str1 += WriteCUDouble( _U_DIFFPRESS, dHMin, true );
					lRow = AddStaticText( pclSheetDescription, ColumnDescription::TechnicalInfos, lRow, str1 );
				}
			}
		}

		if( true == m_pTADSPageSetup->GetField( epfDPCTECHINFOCONNECTSCHEME ) )
		{
			str2 = TASApp.LoadLocalizedString( IDS_CONNECT_SCHEME );
			
			if( eDpStab::DpStabOnCV == pclSSelDpC->GetDpStab() && pclSSelDpC->GetKv() > 0.0 )
			{
				str2 = TASApp.LoadLocalizedString( IDS_CONNECT_SCH );
			}

			CString strName = pclSSelDpC->GetSchemeName();
			
			if( FALSE == strName.IsEmpty() )
			{
				if( pclSSelDpC->GetSchemeIndex() > 0 )
				{
					str1 = str2 + _T(" ") + Base26Convert( pclSSelDpC->GetSchemeIndex() - 1 );
				}
			}

			if( eDpStab::DpStabOnCV == pclSSelDpC->GetDpStab() && pclSSelDpC->GetKv() > 0.0 )
			{
				if( !m_pUnitDB->GetDefaultUnitIndex( _C_KVCVCOEFF ) )
				{
					str2 = TASApp.LoadLocalizedString( IDS_KVSERIES_KV );
				}
				else
				{
					str2 = TASApp.LoadLocalizedString( IDS_KVSERIES_CV );
				}

				str1 += _T(" (") + str2 + _T("=") + (CString)WriteDouble( CDimValue::SItoCU( _C_KVCVCOEFF, pclSSelDpC->GetKv() ), 2, 0, true ) + _T(")");
			}
			lRow = AddStaticText( pclSheetDescription, ColumnDescription::TechnicalInfos, lRow, str1 );
		}

		if( true == m_pTADSPageSetup->GetField( epfDPCTECHINFOAUTHORITY ) )
		{
			// Authority.
			if( eDpStab::DpStabOnCV == pclSSelDpC->GetDpStab() && pclSSelDpC->GetKv() > 0.0 )						// Scheme C Or D
			{
				double dBeta = pclSSelDpC->GetAuthority();
				str2 = TASApp.LoadLocalizedString( IDS_AUTHORITY );
				str1 = str2 + _T(">") + (CString)WriteDouble( dBeta, 2, 0 );
				lRow = AddStaticText( pclSheetDescription, ColumnDescription::TechnicalInfos, lRow, str1 );
			}
		}
	}

	return pclSheet->GetMaxRows();
}

long CSelProdPageDpC::_FillRowDpController( CSheetDescription *pclSheetDescription, long lRow, CDS_HydroMod::CDpC *pclHMDpC )
{
	if( NULL == pclSheetDescription || NULL == pclSheetDescription->GetSSheetPointer() || NULL == pclHMDpC ) 
	{
		return lRow;
	}

	CSSheet *pclSheet = pclSheetDescription->GetSSheetPointer();
	CString str1, str2;
	long lFirstRow = lRow;

	CDB_DpController *pclDpController = dynamic_cast<CDB_DpController*>( pclHMDpC->GetIDPtr().MP );

	if( NULL == pclDpController )
	{
		return pclSheet->GetMaxRows();
	}

	pclSheet->SetTextPattern( CSSheet::_SSTextPattern::UserStaticColumn );
	pclSheet->SetTextPatternProperty( CSSheet::_SSTextPatternProperty::TPP_FontBold, (LPARAM)FALSE );

	// Column TA Product.
	_FillRowDpControllerProduct( pclSheetDescription, lRow, pclDpController, 1 );

	// Column Infos;
	lRow = lFirstRow;
	AddStaticText( pclSheetDescription, ColumnDescription::TechnicalInfos, lRow, _T("-") );			// Default string

	if( true == m_pTADSPageSetup->GetField( epfDPCTECHINFOFLOW ) )
	{
		str1 = WriteCUDouble( _U_FLOW, pclHMDpC->GetQ(), true );
		lRow = AddStaticText( pclSheetDescription, ColumnDescription::TechnicalInfos, lRow, str1 );
	}

	if( true == m_pTADSPageSetup->GetField( epfDPCTECHINFO2NDDP ) )
	{
		double dDpL = pclHMDpC->GetDpL();
		// Show Dpl only if different from -1
		str2 = TASApp.LoadLocalizedString( IDS_DPL );
		{
			if( dDpL <= 0 )
			{
				CString str3;
				str3 = TASApp.LoadLocalizedString( IDS_UNKNOWN );
				str3.MakeLower();
				str1 = str2 + (CString)_T(" ") + str3;
			}
			else
			{
				str1 = str2 + (CString)_T(" = ");
				str1 += WriteCUDouble( _U_DIFFPRESS, pclHMDpC->GetDpToStab(), true );
			}

			lRow = AddStaticText( pclSheetDescription, ColumnDescription::TechnicalInfos, lRow, str1 );
		}
	}

	if( true == m_pTADSPageSetup->GetField( epfDPCTECHINFODPMIN ) )
	{
		// Show DPMin only if different from 0.
		if( pclHMDpC->GetDpmin() > 0 )
		{
			str2 = TASApp.LoadLocalizedString( IDS_DPMIN );
			str1 = str2 + (CString)_T(" = ");
			str1 += WriteCUDouble( _U_DIFFPRESS, pclHMDpC->GetDpmin(), true );
			lRow = AddStaticText( pclSheetDescription, ColumnDescription::TechnicalInfos, lRow, str1 );
		}
	}

	if( true == m_pTADSPageSetup->GetField( epfDPCTECHINFOSETTING ) )
	{
		if( pclHMDpC->GetDpCSetting() > 0.0 )
		{
			str1 = _T("-");
			CDB_DpCCharacteristic *pclDpCCharacteristic = pclDpController->GetDpCCharacteristic();

			if( NULL != pclDpCCharacteristic )
			{
				str1 = pclDpCCharacteristic->GetSettingString( pclHMDpC->GetDpCSetting(), true );
			}
			
			lRow = AddStaticText( pclSheetDescription, ColumnDescription::TechnicalInfos, lRow, str1 );
		}
	}

	if( true == m_pTADSPageSetup->GetField( epfDPCTECHINFOHMIN ) )
	{
		// Show HMin only if different from 0.
		double dHMin = pclHMDpC->GetpParentHM()->GetDp(true);

		if( dHMin > 0.0 )
		{
			str2 = TASApp.LoadLocalizedString( IDS_HMIN );
			str1 = str2 + (CString)_T(" = ");

			str1 += WriteCUDouble( _U_DIFFPRESS, dHMin, true );
		}

		if( NULL != pclHMDpC->GetpParentHM()->GetpSch() && eDpStab::DpStabOnCV == pclHMDpC->GetpParentHM()->GetpSch()->GetDpStab() && true == pclHMDpC->GetpParentHM()->IsCvExist( true ) )
		{
			if( 0 == m_pUnitDB->GetDefaultUnitIndex( _C_KVCVCOEFF ) )
			{
				str2 = TASApp.LoadLocalizedString( IDS_KVSERIES_KV );
			}
			else
			{
				str2 = TASApp.LoadLocalizedString( IDS_KVSERIES_CV );
			}

			double dKvCv = pclHMDpC->GetpParentHM()->GetpCV()->GetKvs();
			str1 += _T(" (") + str2 + _T("=") + (CString)WriteDouble( CDimValue::SItoCU( _C_KVCVCOEFF, dKvCv ), 2, 0, true ) + _T(")");
			lRow = AddStaticText( pclSheetDescription, ColumnDescription::TechnicalInfos, lRow, str1 );
		}
		else
		{
			lRow = AddStaticText( pclSheetDescription, ColumnDescription::TechnicalInfos, lRow, str1 );
		}
	}

	if( true == m_pTADSPageSetup->GetField( epfDPCTECHINFOCONNECTSCHEME ) )
	{
		// No connection scheme for DpC selected from HydroCalc
	}

	if( true == m_pTADSPageSetup->GetField( epfDPCTECHINFOAUTHORITY ) )
	{
		if( NULL != pclHMDpC->GetpParentHM()->GetpCV() &&  pclHMDpC->GetpParentHM()->GetpCV()->GetAuth() > 0 )
		{
			if( NULL != pclHMDpC->GetpParentHM()->GetpSch() && eDpStab::DpStabOnCV == pclHMDpC->GetpParentHM()->GetpSch()->GetDpStab() )
			{
				str2 = TASApp.LoadLocalizedString( IDS_AUTHORITY );
				str1 = str2 + _T(">") + (CString)WriteDouble( pclHMDpC->GetpParentHM()->GetpCV()->GetAuth(), 2, 0 );
				lRow = AddStaticText( pclSheetDescription, ColumnDescription::TechnicalInfos, lRow, str1 );
			}
		}
	}

	return pclSheet->GetMaxRows();
}

void CSelProdPageDpC::_FillRowDpControllerProduct( CSheetDescription *pclSheetDescription, long lRow, CDB_TAProduct *pTAP, int iQuantity, CDB_Set *pclSet, CString strArtNumber )
{
	if( NULL == pclSheetDescription || NULL == pclSheetDescription->GetSSheetPointer() || NULL == pTAP ) 
	{
		return;
	}

	CString str1, str2;
	long lFirstRow = lRow;

	AddStaticText( pclSheetDescription, ColumnDescription::Product, lRow, _T("-") );
	
	if( true == m_pTADSPageSetup->GetField( epfDPCINFOPRODUCTNAME ) )	
	{
		lRow = AddStaticText( pclSheetDescription, ColumnDescription::Product, lRow, pTAP->GetName() );
	}

	if( true == m_pTADSPageSetup->GetField( epfDPCINFOPRODUCTSIZE ) )	
	{
		lRow = AddStaticText( pclSheetDescription, ColumnDescription::Product, lRow, pTAP->GetSize() );
	}
	
	if( true == m_pTADSPageSetup->GetField( epfDPCINFOPRODUCTCONNECTION ) )
	{
		str1 = ( (CDB_StringID *)pTAP->GetConnectIDPtr().MP )->GetString();
		if( 0 == IDcmp( pTAP->GetConnTabID(), _T("COMPONENT_TAB") ) )
			str1 += CString( _T(" -> ") ) + pTAP->GetSize();
		lRow = AddStaticText( pclSheetDescription, ColumnDescription::Product, lRow, str1 );
	}
	
	if( true == m_pTADSPageSetup->GetField( epfDPCINFOPRODUCTVERSION ) )
	{
		str1 = ( (CDB_StringID *)pTAP->GetVersionIDPtr().MP )->GetString();
		lRow = AddStaticText( pclSheetDescription, ColumnDescription::Product, lRow, str1 );
	}
	
	if( true == m_pTADSPageSetup->GetField( epfDPCINFOPRODUCTDPLRANGE) )	
		lRow = AddStaticText( pclSheetDescription, ColumnDescription::Product, lRow, ( (CDB_DpController*)pTAP )->GetFormatedDplRange( true ).c_str() );
	
	if( true == m_pTADSPageSetup->GetField( epfDPCINFOPRODUCTPN ) )	
	{
		lRow = FillPNTminTmax( pclSheetDescription, ColumnDescription::Product, lRow, pTAP );
	}

	// Column quantity and price.
	if( NULL != pclSet )
	{
		FillQtyPriceCol( pclSheetDescription, lFirstRow, pclSet, iQuantity );
	}
	else
	{
		FillQtyPriceCol( pclSheetDescription, lFirstRow, pTAP, iQuantity );
	}
	
	// Column article number.
	lRow = lFirstRow;
	AddStaticText( pclSheetDescription, ColumnDescription::ArticleNumber, lRow, _T("-") );			// Default string

	if( true == strArtNumber.IsEmpty() )
	{
		strArtNumber = pTAP->GetBodyArtNum();
	}
	
	if( NULL != pclSet )
	{
		FillArtNumberColForPackage( pclSheetDescription, ColumnDescription::ArticleNumber, lRow, pclSet, strArtNumber );
	}
	else
	{
		FillArtNumberCol( pclSheetDescription, ColumnDescription::ArticleNumber, lRow, pTAP, strArtNumber );
	}
}
		
long CSelProdPageDpC::_FillDpControllerSCheme( CSheetDescription *pclSheetDescription, long lRow, CDS_SSelDpC *pclSSelDpController )
{
	if( NULL == pclSheetDescription || NULL == pclSheetDescription->GetSSheetPointer() ) 
	{
		return lRow;
	}

	CSSheet *pclSheet = pclSheetDescription->GetSSheetPointer();
	CString strSchemeID( _T("") );
	CString str;
	long lFirstRow = lRow;
	pclSheet->SetTextPattern( CSSheet::_SSTextPattern::UserStaticColumn );
	pclSheet->SetTextPatternProperty( CSSheet::_SSTextPatternProperty::TPP_FontBold, (LPARAM)FALSE );

	int iStart, iStop;

	if( NULL == pclSSelDpController )
	{
		iStart = 1;
		iStop = 1000;	// 1000 schemes!
	}
	else
	{
		iStop = iStart = pclSSelDpController->GetSchemeIndex();
	}
	
	bool bAtLeastOneSchemeDisplayed = false;

	for( int i = iStart; i <= iStop; i++ )
	{
		CDS_SSelDpC *pclSSelDpController = NULL;

		if( NULL != pDlgLeftTabSelP )
		{
			strSchemeID = pDlgLeftTabSelP->GetScheme( i, &pclSSelDpController );
		}

		if( true == strSchemeID.IsEmpty() )
		{
			break;
		}
		
		if( NULL == pclSSelDpController )
		{
			break;
		}
		
		CDB_DpController *pclDpController = dynamic_cast<CDB_DpController *>( pclSSelDpController->GetDpCIDPtr().MP );

		if( NULL == pclDpController )
		{
			break;
		}

		if( true == bAtLeastOneSchemeDisplayed )
		{
			lRow++;
			SetLastRow( pclSheetDescription, lRow );
		}

		lFirstRow = lRow;
		
		CString SchemeName;

		if( i > 0 )
		{
			SchemeName = Base26Convert( i - 1 );
		}
		
		// Connection scheme.
		str = TASApp.LoadLocalizedString( IDS_CONNECT_SCHEME );
		str += CString( _T(" ") ) + SchemeName;
		pclSheet->SetTextPatternProperty( CSSheet::_SSTextPatternProperty::TPP_StyleVerticalAlign, (LPARAM)SSS_ALIGN_BOTTOM );
		AddStaticText( pclSheetDescription, ColumnDescription::Reference1, lRow, str );
		AddCellSpanW( pclSheetDescription, ColumnDescription::Reference1, lRow, 2, 1 );
	
		// DpC Family.
		CString FamID = pclDpController->GetFamilyID();
		str = _T("(");

		if( false == FamID.IsEmpty() )
		{
			str += ( (CDB_StringID *)TASApp.GetpTADB()->Get( (LPCTSTR)FamID ).MP )->GetString();
		}
		
		str += ( str.GetLength() > 1 ) ? _T(")") : _T("");

		pclSheet->SetTextPatternProperty( CSSheet::_SSTextPatternProperty::TPP_StyleVerticalAlign, (LPARAM)SSS_ALIGN_TOP );
		lRow++;
		AddStaticText( pclSheetDescription, ColumnDescription::Reference1, lRow, str );
		AddCellSpanW( pclSheetDescription, ColumnDescription::Reference1, lRow, 2, 1 );
		
		// Media characteristic.
		AddCellSpanW( pclSheetDescription, ColumnDescription::Water, lFirstRow, ColumnDescription::Footer - ColumnDescription::Water, 2 );

		pclSheet->SetRowHeight( lFirstRow, 60 );
		pclSheet->SetRowHeight( lRow, 60 );

		// Create the CEnBitmap.
		CEnBitmapPatchWork EnBmp;
		CDB_CircuitScheme* pCircSch = EnBmp.FindSSelDpCHydronicScheme( pclSSelDpController->GetMvLoc(), pclDpController->GetDpCLoc(), pclSSelDpController->GetDpStab(), pclSSelDpController->IsSelectedAsAPackage() );
		EnBmp.GetSSelDpCHydronicScheme( pCircSch, pclSSelDpController->GetProductIDPtr(), pclSSelDpController->GetDpCIDPtr() );
		EnBmp.AddSSelDpCArrows(pCircSch);
		EnBmp.RotateImage( 90 );

		if( false == m_bForPrint )
		{
			EnBmp.ResizeImage( 0.35 );
		}

		long lPictStyle = VPS_BMP | VPS_MAINTAINSCALE | SSS_ALIGN_CENTER | SSS_ALIGN_BOTTOM;

		if( true == m_bForPrint )
		{
			lPictStyle |= VPS_STRETCH;
		}
		
		pclSheet->SetPictureCellWithHandle( EnBmp, ColumnDescription::Water, lFirstRow, true, lPictStyle );
		EnBmp.DeleteObject();

		// Set all group as no breakable (for print).
		pclSheet->SetFlagRowNoBreakable( lFirstRow, lRow, true );

		bAtLeastOneSchemeDisplayed = true;
	}
	
	return pclSheet->GetMaxRows();
}

void CSelProdPageDpC::_AddDpControllerArticleList( CDS_SSelDpC *pclSSelDpController, CDB_TAProduct *pTAP, int iQuantity )
{
	if( NULL != pclSSelDpController )
	{
		pTAP = (CDB_TAProduct *)( pclSSelDpController->GetDpCIDPtr().MP );
		iQuantity = pclSSelDpController->GetpSelectedInfos()->GetQuantity();
	}

	if( NULL == pTAP )
	{
		return;
	}

	// If no 'pSel' (mean that comes from direct selection) or DpC is not selected as a package...
	if( NULL == pclSSelDpController || false == pclSSelDpController->IsSelectedAsAPackage( true ) )
	{
		CArticleGroup *pclArticleGroup = new CArticleGroup();
		
		if( NULL == pclArticleGroup )
		{
			return;
		}

		CArticleContainer *pclArticleContainer = pclArticleGroup->AddArticle( pTAP, iQuantity );

		if( NULL == pclArticleContainer || NULL == pclArticleContainer->GetArticleItem() )
		{
			delete pclArticleGroup;
			return;
		}

		CArticleItem *pclArticleItem = pclArticleContainer->GetArticleItem();

		// Remark: 'm_strID', 'm_strArticle', 'm_strLocArtNum', 'm_dPrice', 'm_iQuantity', 'm_fIsAvailable' and 'm_fIsDeleted'
		//         variables are already set when creating 'CArticleContainer'.

		pclArticleItem->SetDescription( ( (CDB_DpController *)pTAP )->GetName() + CString( _T("; ") ) );

		// Add Dpl range.
		pclArticleItem->AddDescription( ( (CDB_DpController*)pTAP )->GetFormatedDplRange().c_str() + CString( _T("; ") ) );

		// Add version.
		pclArticleItem->AddDescription( ( (CDB_StringID *)pTAP->GetVersionIDPtr().MP )->GetString()	+ CString( _T("; ") ) );

		// Add connection.
		CString str = ( (CDB_StringID *)pTAP->GetConnectIDPtr().MP )->GetString();
		str = str.Right( str.GetLength() - str.Find('/') - 1 );
		pclArticleItem->AddDescription( str + CString( _T("; ") ) );

		// Add PN.
		str = TASApp.LoadLocalizedString( IDS_SSHEETSELPROD_PN ) + CString( _T(" ") ) + pTAP->GetPN().c_str();
		pclArticleItem->AddDescription( str );

		if (NULL != pclSSelDpController)
		{
			CSelProdPageBase::AddAccessoriesInArticleContainer( pclSSelDpController->GetDpCAccessoryList(), pclArticleContainer, pclSSelDpController->GetpSelectedInfos()->GetQuantity(), pclSSelDpController->IsSelectedAsAPackage( true ) );
		}

		CSelProdPageBase::AddArticleInGroupList( pclArticleGroup );

		// Remark: pay attention that 'CPage::AddArticleInGroupList' will increment quantity if article already exist or
		//         will create a new 'CArticleGroup' and will copy to it data passed as argument. This is the reason
		//         why you need to delete 'pclArticleGroup' after the call.
		delete pclArticleGroup;

		if( true == pTAP->IsConnTabUsed() )
		{
			// Inlet.
			CSelProdPageBase::AddCompArtList( pTAP, iQuantity, true );
			// Outlet.
			CSelProdPageBase::AddCompArtList( pTAP, iQuantity, false );
		}
	}
	else if( NULL != pclSSelDpController )
	{
		CDB_DpController *pDpC = dynamic_cast<CDB_DpController *>(pclSSelDpController->GetDpCIDPtr().MP);
		CDB_TAProduct *pMV = pclSSelDpController->GetProductAs<CDB_TAProduct>();

		// Find associated package.
		IDPTR PackIDPtr = pclSSelDpController->GetDpCMvPackageIDPtr();
		CDB_Set *pSet = dynamic_cast<CDB_Set *>( PackIDPtr.MP );
		if( NULL != pSet )
		{
			// DpC is selected as a package.
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

			pclArticleItem->SetID( PackIDPtr.ID );
			pclArticleItem->SetPrice( TASApp.GetpTADB()->GetPrice( pSet->GetArtNum( true ) ) );
			pclArticleItem->SetQuantity( pclSSelDpController->GetpSelectedInfos()->GetQuantity() );
			pclArticleItem->SetLocArtNum( _T("-") );
			CString str = pSet->GetName() + CString( _T(" ") ) + TASApp.LoadLocalizedString( IDS_SSHEETSELPROD_SET );
			pclArticleItem->SetDescription( str );
			pclArticleItem->SetIsAvailable( pSet->IsAvailable() );
			pclArticleItem->SetIsDeleted( pSet->IsDeleted() );

			CString strArticleNumber = pSet->GetReference();
			CString strLocArtNumber = LocArtNumTab.GetLocalArticleNumber( strArticleNumber );
			pclArticleItem->CheckThingAvailability( pSet, strArticleNumber, strLocArtNumber );
			
			if( true == strLocArtNumber.IsEmpty() )
			{
				strLocArtNumber = _T("-");
			}

			pclArticleItem->SetArticle( strArticleNumber );
			pclArticleItem->SetLocArtNum( strLocArtNumber );

			CArticleContainer *pclArticleContainer = pclArticleGroup->AddArticle( pclArticleItem );

			// Remark: 'AddArticle' will call new CArticleContainer( pclArticleItem ) that copies content of 'pclArticleItem' and not saves the pointer.
			delete pclArticleItem;

			// Add accessories for Dpc AND Bv !!!
			CSelProdPageBase::AddAccessoriesInArticleContainer( pclSSelDpController->GetDpCAccessoryList(), pclArticleContainer, pclSSelDpController->GetpSelectedInfos()->GetQuantity(), true );
			CSelProdPageBase::AddAccessoriesInArticleContainer( pclSSelDpController->GetAccessoryList(), pclArticleContainer, pclSSelDpController->GetpSelectedInfos()->GetQuantity(), true );

			CSelProdPageBase::AddArticleInGroupList( pclArticleGroup );

			// Remark: pay attention that 'CPage::AddArticleInGroupList' will increment quantity if article already exist or
			//         will create a new 'CArticleGroup' and will copy to it data passed as argument. This is the reason
			//         why you need to delete 'pclArticleGroup' after the call.
			delete pclArticleGroup;
		}
	}
}

long CSelProdPageDpC::_FillRowDpReliefValve( CSheetDescription *pclSheetDescription, long lRow, CDS_SSelDpReliefValve *pclSSelReliefValve )
{
	if( NULL == pclSheetDescription || NULL == pclSheetDescription->GetSSheetPointer() || NULL == pclSSelReliefValve ) 
	{
		return lRow;
	}

	CDB_DpReliefValve *pclDpReliefValve = pclSSelReliefValve->GetProductAs<CDB_DpReliefValve>();

	if( NULL == pclDpReliefValve )
	{
		return lRow;
	}

	CSSheet *pclSheet = pclSheetDescription->GetSSheetPointer();
	CString str1, str2;
	long lFirstRow = lRow;
	pclSheet->SetTextPattern( CSSheet::_SSTextPattern::UserStaticColumn );
	pclSheet->SetTextPatternProperty( CSSheet::_SSTextPatternProperty::TPP_FontBold, (LPARAM)FALSE );

	// Column TA Product.
	CString ArtNum( _T("") );

	_FillRowDpReliefValveProduct( pclSheetDescription, lRow, pclDpReliefValve, pclSSelReliefValve->GetpSelectedInfos()->GetQuantity(), ArtNum );

	return pclSheet->GetMaxRows();
}

void CSelProdPageDpC::_FillRowDpReliefValveProduct( CSheetDescription *pclSheetDescription, long lRow, CDB_DpReliefValve *pclDpReliefValve, int iQuantity, CString strArtNumber )
{
	if( NULL == pclSheetDescription || NULL == pclSheetDescription->GetSSheetPointer() || NULL == pclDpReliefValve ) 
	{
		return;
	}

	CString str1, str2;
	long lFirstRow = lRow;

	AddStaticText( pclSheetDescription, ColumnDescription::Product, lRow, _T("-") );
	
	if( true == m_pTADSPageSetup->GetField( epfDPCINFOPRODUCTNAME ) )	
	{
		lRow = AddStaticText( pclSheetDescription, ColumnDescription::Product, lRow, pclDpReliefValve->GetName() );
	}

	if( true == m_pTADSPageSetup->GetField( epfDPCINFOPRODUCTSIZE ) )	
	{
		lRow = AddStaticText( pclSheetDescription, ColumnDescription::Product, lRow, pclDpReliefValve->GetSize() );
	}
	
	if( true == m_pTADSPageSetup->GetField( epfDPCINFOPRODUCTCONNECTION ) )
	{
		str1 = ( (CDB_StringID *)pclDpReliefValve->GetConnectIDPtr().MP )->GetString();

		if( 0 == IDcmp( pclDpReliefValve->GetConnTabID(), _T("COMPONENT_TAB") ) )
		{
			str1 += CString( _T(" -> ") ) + pclDpReliefValve->GetSize();
		}

		lRow = AddStaticText( pclSheetDescription, ColumnDescription::Product, lRow, str1 );
	}
	
	if( true == m_pTADSPageSetup->GetField( epfDPCINFOPRODUCTVERSION ) )
	{
		str1 = ( (CDB_StringID *)pclDpReliefValve->GetVersionIDPtr().MP )->GetString();
		lRow = AddStaticText( pclSheetDescription, ColumnDescription::Product, lRow, str1 );
	}

	// Show setting range.
	lRow = AddStaticText( pclSheetDescription, ColumnDescription::Product, lRow, pclDpReliefValve->GetAdjustableRange( true ) );
	
	if( true == m_pTADSPageSetup->GetField( epfDPCINFOPRODUCTPN ) )	
	{
		lRow = FillPNTminTmax( pclSheetDescription, ColumnDescription::Product, lRow, pclDpReliefValve );
	}

	// Column quantity and price.
	FillQtyPriceCol( pclSheetDescription, lFirstRow, pclDpReliefValve, iQuantity );
	
	// Column article number.
	lRow = lFirstRow;
	AddStaticText( pclSheetDescription, ColumnDescription::ArticleNumber, lRow, _T("-") );			// Default string

	if( true == strArtNumber.IsEmpty() )
	{
		strArtNumber = pclDpReliefValve->GetBodyArtNum();
	}
	
	FillArtNumberCol( pclSheetDescription, ColumnDescription::ArticleNumber, lRow, pclDpReliefValve, strArtNumber );
}

void CSelProdPageDpC::_AddDpReliefValveArticleList( CDB_TAProduct *pTAP, int iQuantity )
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
		delete pclArticleGroup;
		return;
	}

	CArticleItem *pclArticleItem = pclArticleContainer->GetArticleItem();

	// Remark: 'm_strID', 'm_strArticle', 'm_strLocArtNum', 'm_dPrice', 'm_iQuantity', 'm_fIsAvailable' and 'm_fIsDeleted'
	//         variables are already set when creating 'CArticleContainer'.

	pclArticleItem->SetDescription( pTAP->GetName() + CString( _T("; ") ) );

	// Add setting range.
	pclArticleItem->AddDescription( ( (CDB_DpReliefValve *)pTAP )->GetAdjustableRange() + CString( _T("; ") ) );

	// Add version.
	pclArticleItem->AddDescription( ( (CDB_StringID *)pTAP->GetVersionIDPtr().MP )->GetString()	+ CString( _T("; ") ) );

	// Add connection.
	CString str = ( (CDB_StringID *)pTAP->GetConnectIDPtr().MP )->GetString();
	str = str.Right( str.GetLength() - str.Find('/') - 1 );
	pclArticleItem->AddDescription( str + CString( _T("; ") ) );

	// Add PN.
	str = TASApp.LoadLocalizedString( IDS_SSHEETSELPROD_PN ) + CString( _T(" ") ) + pTAP->GetPN().c_str();
	pclArticleItem->AddDescription( str );

	CSelProdPageBase::AddArticleInGroupList( pclArticleGroup );

	// Remark: pay attention that 'CPage::AddArticleInGroupList' will increment quantity if article already exist or
	//         will create a new 'CArticleGroup' and will copy to it data passed as argument. This is the reason
	//         why you need to delete 'pclArticleGroup' after the call.
	delete pclArticleGroup;

	if( true == pTAP->IsConnTabUsed() )
	{
		// Inlet.
		CSelProdPageBase::AddCompArtList( pTAP, iQuantity, true );
		// Outlet.
		CSelProdPageBase::AddCompArtList( pTAP, iQuantity, false );
	}
}

long CSelProdPageDpC::_FillRowDpSensor( CSheetDescription* pclSheetDescription, long lRow, CDS_SSelDpSensor* pclSSelDpSensor )
{
	if( NULL == pclSheetDescription || NULL == pclSheetDescription->GetSSheetPointer() || NULL == pclSSelDpSensor )
	{
		return lRow;
	}

	CDB_DpSensor* pclDpSensor = pclSSelDpSensor->GetProductAs<CDB_DpSensor>();

	if( NULL == pclDpSensor )
	{
		return lRow;
	}

	CSSheet* pclSheet = pclSheetDescription->GetSSheetPointer();
	CString str1, str2;
	long lFirstRow = lRow;
	pclSheet->SetTextPattern( CSSheet::_SSTextPattern::UserStaticColumn );
	pclSheet->SetTextPatternProperty( CSSheet::_SSTextPatternProperty::TPP_FontBold, (LPARAM)FALSE );

	// Column TA Product.
	CString ArtNum( _T( "" ) );

	_FillRowDpSensorProduct( pclSheetDescription, lRow, pclDpSensor, pclSSelDpSensor->GetpSelectedInfos()->GetQuantity(), ArtNum );

	return pclSheet->GetMaxRows();
}

void CSelProdPageDpC::_FillRowDpSensorProduct( CSheetDescription* pclSheetDescription, long lRow, CDB_DpSensor* pclDpSensor, int iQuantity, CString strArtNumber )
{
	if( NULL == pclSheetDescription || NULL == pclSheetDescription->GetSSheetPointer() || NULL == pclDpSensor )
	{
		return;
	}

	CString str1, str2;
	long lFirstRow = lRow;

	AddStaticText( pclSheetDescription, ColumnDescription::Product, lRow, _T( "-" ) );

	// HYS-2059: For DpSensor set display only name with dpl in the same merged column.
	if( true == pclDpSensor->IsPartOfaSet() )
	{
		// Write description.
		pclSheetDescription->GetSSheetPointer()->AddCellSpanW( ColumnDescription::Product, lRow, 2, 1 );
		lRow = AddStaticText( pclSheetDescription, ColumnDescription::Product, lRow, pclDpSensor->GetFullName() );
	}
	else
	{
		if( true == m_pTADSPageSetup->GetField( epfDPCINFOPRODUCTNAME ) )
		{
			lRow = AddStaticText( pclSheetDescription, ColumnDescription::Product, lRow, pclDpSensor->GetName() );
		}

		if( true == m_pTADSPageSetup->GetField( epfDPCINFOPRODUCTDPLRANGE ) )
		{
			lRow = AddStaticText( pclSheetDescription, ColumnDescription::Product, lRow, pclDpSensor->GetFormatedDplRange( true ).c_str() );
		}

		// Show burst pressure.
		if( 0 < pclDpSensor->GetBurstPressure() )
		{
			str1 = WriteCUDouble( _U_DIFFPRESS, pclDpSensor->GetBurstPressure(), true );
			FormatString( str2, IDS_TALINK_BURSTPRESSURE, str1 );
			lRow = AddStaticText( pclSheetDescription, ColumnDescription::Product, lRow, str2 );
		}

		// Output signal
		lRow = AddStaticText( pclSheetDescription, ColumnDescription::Product, lRow, pclDpSensor->GetOutputSignalsStr() );
	}
	// Column quantity and price.
	FillQtyPriceCol( pclSheetDescription, lFirstRow, pclDpSensor, iQuantity );

	// Column article number.
	lRow = lFirstRow;
	AddStaticText( pclSheetDescription, ColumnDescription::ArticleNumber, lRow, _T( "-" ) );			// Default string

	if( true == strArtNumber.IsEmpty() )
	{
		strArtNumber = pclDpSensor->GetBodyArtNum();
	}

	FillArtNumberCol( pclSheetDescription, ColumnDescription::ArticleNumber, lRow, pclDpSensor, strArtNumber );
}

void CSelProdPageDpC::_AddDpSensorArticleList( CDB_Product *pclProduct, CDS_SSelDpSensor* pSSel, int iQuantity )
{
	if( NULL == pclProduct || NULL == pSSel )
	{
		return;
	}

	CArticleGroup *pclArticleGroup = new CArticleGroup();
		
	if( NULL == pclArticleGroup )
	{
		return;
	}

	CArticleContainer *pclArticleContainer = pclArticleGroup->AddArticle( pclProduct, iQuantity );

	if( NULL == pclArticleContainer || NULL == pclArticleContainer->GetArticleItem() )
	{
		delete pclArticleGroup;
		return;
	}

	CArticleItem *pclArticleItem = pclArticleContainer->GetArticleItem();

	// Remark: 'm_strID', 'm_strArticle', 'm_strLocArtNum', 'm_dPrice', 'm_iQuantity', 'm_fIsAvailable' and 'm_fIsDeleted'
	//         variables are already set when creating 'CArticleContainer'.

	pclArticleItem->SetDescription( ( (CDB_DpSensor*)pclProduct)->GetFullName() );

	// Add burst pressure.
	if( 0 < ((CDB_DpSensor*)pclProduct )->GetBurstPressure() )
	{
		pclArticleItem->AddDescription( CString( _T( "; " ) ) + WriteCUDouble( _U_DIFFPRESS, ((CDB_DpSensor*)pclProduct)->GetBurstPressure(), true ) );
	}

	if( NULL != pSSel && NULL != pSSel->GetAccessoryList() && false == pclProduct->IsPartOfaSet() )
	{
		CSelProdPageBase::AddAccessoriesInArticleContainer( pSSel->GetAccessoryList(), pclArticleContainer, pSSel->GetpSelectedInfos()->GetQuantity(), pclProduct->IsPartOfaSet() );
	}

	CSelProdPageBase::AddArticleInGroupList( pclArticleGroup );

	// Remark: pay attention that 'CPage::AddArticleInGroupList' will increment quantity if article already exist or
	//         will create a new 'CArticleGroup' and will copy to it data passed as argument. This is the reason
	//         why you need to delete 'pclArticleGroup' after the call.
	delete pclArticleGroup;
}