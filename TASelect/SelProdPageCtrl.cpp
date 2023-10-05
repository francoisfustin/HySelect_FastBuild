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
#include "SelProdPageCtrl.h"

CSelProdPageCtrl::CSelProdPageCtrl( CArticleGroupList *pclArticleGroupList ) 
	: CSelProdPageBase( CDB_PageSetup::enCheck::CV, pclArticleGroupList )
{
	m_pSelected = NULL;
}

void CSelProdPageCtrl::Reset( void )
{
	CSelProdPageBase::Reset();
}

bool CSelProdPageCtrl::PreInit( HMvector &vecHMList )
{
	// Something for individual or direct selection ?
	CTable *pclTable = (CTable*)( TASApp.GetpTADS()->Get( _T("CTRLVALVE_TAB") ).MP );

	if ( NULL == pclTable )
	{
		ASSERT( 0 );
		return false;
	}

	m_pclSelectionTable = ( '\0' != *pclTable->GetFirst().ID ) ? pclTable : NULL;
	bool fCvExist = ( NULL != m_pclSelectionTable );

	// Something for hydronic calculation?
	bool fCvHMExist = ( vecHMList.size() > 0 );

	if( false == fCvExist && false == fCvHMExist )
	{
		return false;
	}
	
	m_vecHMList = vecHMList;
	SortTable();
	
	return true;
}

bool CSelProdPageCtrl::Init( bool fResetOrder, bool fPrint )
{
	try
	{
		bool fCvExist = ( NULL != m_pclSelectionTable ) ? true : false;
		bool fCvHMExist = ( m_vecHMList.size() > 0 ) ? true : false;

		if( false == fCvExist && false == fCvHMExist )
		{
			return false;
		}

		// Call base class method to get a new 'Sheet'.
		CSheetDescription *pclSheetDescriptionCv = CMultiSpreadBase::CreateSSheet( SD_ControlValve );

		if( NULL == pclSheetDescriptionCv || NULL == pclSheetDescriptionCv->GetSSheetPointer() )
		{
			return false;
		}
	
		CSSheet *pclSheet = pclSheetDescriptionCv->GetSSheetPointer();

		if( false == CSelProdPageBase::PrepareSheet( pclSheetDescriptionCv, ColumnDescription::Pointer, fPrint ) )
		{
			m_ViewDescription.RemoveOneSheetDescription( SD_ControlValve );
			return false;
		}

		// Initialize column width for header and footer columns.
		if( false == m_bForPrint )
		{
			SetColWidth( pclSheetDescriptionCv, ColumnDescription::Header, DefaultColumnWidth::DCW_Header );
			SetColWidth( pclSheetDescriptionCv, ColumnDescription::Footer, DefaultColumnWidth::DCW_Footer );
		}
		else
		{
			pclSheet->ShowCol( ColumnDescription::Header, FALSE );
			pclSheet->ShowCol( ColumnDescription::Footer, FALSE );
		}

		double dWidth = ( true == m_pTADSPageSetup->GetField( PageField_enum::epfFIRSTREF ) ) ? DefaultColumnWidth::DCW_Reference1 : 0.0;
		SetColWidth( pclSheetDescriptionCv, ColumnDescription::Reference1, dWidth );

		dWidth = ( true == m_pTADSPageSetup->GetField( PageField_enum::epfSECONDREF ) ) ? DefaultColumnWidth::DCW_Reference2 : 0.0;
		SetColWidth( pclSheetDescriptionCv, ColumnDescription::Reference2, dWidth );
	
		dWidth = ( true == m_pTADSPageSetup->GetField( PageField_enum::epfWATERINFO ) ) ? DefaultColumnWidth::DCW_Water : 0.0;
		SetColWidth( pclSheetDescriptionCv, ColumnDescription::Water, dWidth );
	
		dWidth = ( true == m_pTADSPageSetup->GetField( PageField_enum::epfCVINFOPRODUCT ) ) ? DefaultColumnWidth::DCW_Product : 0.0;
		SetColWidth( pclSheetDescriptionCv, ColumnDescription::Product, dWidth );
	
		dWidth = ( true == m_pTADSPageSetup->GetField( PageField_enum::epfCVTECHINFO ) ) ? DefaultColumnWidth::DCW_TechnicalInfos : 0.0;
		SetColWidth( pclSheetDescriptionCv, ColumnDescription::TechnicalInfos, dWidth );
	
		SetColWidth( pclSheetDescriptionCv, ColumnDescription::RadiatorInfos, 0.0 );
	
		dWidth = ( true == m_pTADSPageSetup->GetField( PageField_enum::epfARTICLE ) ) ? DefaultColumnWidth::DCW_ArticleNumber : 0.0;
		SetColWidth( pclSheetDescriptionCv, ColumnDescription::ArticleNumber, dWidth );
	
		dWidth = ( true == m_pTADSPageSetup->GetField( PageField_enum::epfPIPEINFO ) ) ? DefaultColumnWidth::DCW_Pipes : 0.0;
		SetColWidth( pclSheetDescriptionCv, ColumnDescription::Pipes, dWidth );
	
		dWidth = ( true == m_pTADSPageSetup->GetField( PageField_enum::epfQUANTITY ) ) ? DefaultColumnWidth::DCW_Quantity : 0.0;
		SetColWidth( pclSheetDescriptionCv, ColumnDescription::Quantity, dWidth );
	
		dWidth = 0.0;

		if( true == TASApp.IsPriceUsed() )
		{
			dWidth = ( true == m_pTADSPageSetup->GetField( PageField_enum::epfSALEUNITPRICE ) ) ? DefaultColumnWidth::DCW_UnitPrice : 0.0;
		}
	
		SetColWidth( pclSheetDescriptionCv, ColumnDescription::UnitPrice, dWidth );
	
		dWidth = 0.0;

		if( true == TASApp.IsPriceUsed() )
		{
			dWidth = ( true == m_pTADSPageSetup->GetField( PageField_enum::epfSALETOTALPRICE ) ) ? DefaultColumnWidth::DCW_TotalPrice : 0.0;
		}

		SetColWidth( pclSheetDescriptionCv, ColumnDescription::TotalPrice, dWidth );
	
		SetColWidth( pclSheetDescriptionCv, ColumnDescription::Remark, ( true == m_pTADSPageSetup->GetField( PageField_enum::epfREMARK ) ) ? DefaultColumnWidth::DCW_Remark : 0 );
		SetColWidth( pclSheetDescriptionCv, ColumnDescription::Pointer, DefaultColumnWidth::DCW_Pointer );
		pclSheet->ShowCol( ColumnDescription::RadiatorInfos, FALSE );
	
		if( false == TASApp.IsPriceUsed() )
		{
			pclSheet->ShowCol( ColumnDescription::UnitPrice, FALSE );
			pclSheet->ShowCol( ColumnDescription::TotalPrice, FALSE );
		}

		pclSheet->ShowCol( ColumnDescription::Pointer, FALSE );
		CRect rect = pclSheet->GetSheetSizeInPixels( false );
		m_mapSheetInfos[pclSheetDescriptionCv].m_dPageWidth = rect.Width();
	
		// Page title.
		SetPageTitle( pclSheetDescriptionCv, IDS_SSHEETSELPROD_TITLECV );

		// Init column header.
		_InitColHeader( pclSheetDescriptionCv );

		// HYS-1571: We can optimize the column width now.
		// See Jira card to have more details about why we do that here.
		CRect rectClient;
		GetClientRect( &rectClient );
		OptimizePageSize( rectClient.Width(), rectClient.Height() );

		// Current position.
		long lRow = pclSheet->GetMaxRows() + 1;

		if( true == fCvExist )
		{
			int iTotal = m_pclSelectionTable->GetItemCount();
			CDS_SSelCtrl **parSelectedCv = new CDS_SSelCtrl * [iTotal];
			memset( parSelectedCv, NULL, sizeof(CDS_SSelCtrl *) * iTotal );
		
			CDS_Accessory **parAccessory = new CDS_Accessory * [iTotal];
			memset( parAccessory, NULL, sizeof(CDS_Accessory *) * iTotal );

			CDS_Actuator **parActuator = new CDS_Actuator * [iTotal];
			memset( parActuator, NULL, sizeof(CDS_Actuator *) * iTotal );
		
			// Count items. Do it locally prevent to make loops several times.
			int i;
			int iAccessoryCount = 0;
			int iActuatorCount = 0;
			int iControlValveSelectedCount = 0;
			int iMaxISelCv, iMaxIActr, iMaxIAcc; 
			iMaxISelCv = iMaxIActr = iMaxIAcc = 0;
		
			// Fill 'parSelectedCv' array with pointer on each 'CDS_SSelCtrl' object.
			// Fill 'parAccessory' array with pointer on each 'CDS_Accessory' object.
			// Fill 'parActuator' array with pointer on each 'CDS_Actuator' object.
			for( IDPTR idptr = m_pclSelectionTable->GetFirst(); _T('\0') != *idptr.ID; idptr = m_pclSelectionTable->GetNext() )
			{
				CSelectedInfos *pInfos = idptr.MP->GetpSelectedInfos();

				if( NULL == pInfos )
				{
					ASSERT_CONTINUE;
				}
			
				int iIndex = pInfos->GetRowIndex();
			
				if( NULL != dynamic_cast<CDS_SSelCtrl *>( idptr.MP ) )
				{
					CDS_SSelCtrl *pclSSelCtrl = dynamic_cast<CDS_SSelCtrl *>( idptr.MP );
				
					if( NULL != pclSSelCtrl->GetProductIDPtr().MP )
					{
						parSelectedCv[iControlValveSelectedCount] = pclSSelCtrl;
						iControlValveSelectedCount++;
						iMaxISelCv = max( iMaxISelCv, iIndex );
					}
					else
					{
						// We are in a special case. Now in direct selection we can select adapter and actuator alone without the associated valve.
						// We use a 'CDS_SSelCv' for this selection.
						bool bRemoveObject = false;
					
						if( NULL != pclSSelCtrl->GetActrIDPtr().MP )
						{
							// Create the 'CDS_Actuator' object.
							IDPTR ActuatorIDPtr;
							TASApp.GetpTADS()->CreateObject( ActuatorIDPtr, CLASS( CDS_Actuator ) );
						
							CDS_Actuator *pSelectedActuator = (CDS_Actuator *)( ActuatorIDPtr.MP );
						
							if( NULL != pSelectedActuator )
							{
								pSelectedActuator->SetActuatorIDPtr( pclSSelCtrl->GetActrIDPtr() );

								CSelectedInfos *pclActuatorSelectedInfos = pSelectedActuator->GetpSelectedInfos();
								
								if( NULL == pclActuatorSelectedInfos )
								{
									HYSELECT_THROW( _T("Internal error: Can't retrieve the 'CSelectedInfos' object from the newly 'CDS_Actuator' created object.") );
								}

								CSelectedInfos *pclCVSelectedInfos = pclSSelCtrl->GetpSelectedInfos();
								
								if( NULL == pclCVSelectedInfos )
								{
									HYSELECT_THROW( _T("Internal error: Can't retrieve the 'CSelectedInfos' object from the 'CDS_SSelCtrl' object (%s)."), pclSSelCtrl->GetIDPtr().ID );
								}

								pclActuatorSelectedInfos->SetQuantity( pclCVSelectedInfos->GetQuantity() );
								pclActuatorSelectedInfos->SetReference( CSelectedInfos::eReferences::eRef1, pclCVSelectedInfos->GetReference( CSelectedInfos::eReferences::eRef1 ) );
								pclActuatorSelectedInfos->SetReference( CSelectedInfos::eReferences::eRef2, pclCVSelectedInfos->GetReference( CSelectedInfos::eReferences::eRef2 ) );
								pclActuatorSelectedInfos->SetRemark( pclCVSelectedInfos->GetRemark( ) );
								pclActuatorSelectedInfos->SetRemarkIndex( pclCVSelectedInfos->GetRemarkIndex() );

								CAccessoryList *pclAccessoryList = pclSSelCtrl->GetActuatorAccessoryList();

								if( NULL != pclAccessoryList && pclAccessoryList->GetCount() > 0 )
								{
									CAccessoryList::AccessoryItem rAccessoryItem = pclAccessoryList->GetFirst();
								
									while( _NULL_IDPTR != rAccessoryItem.IDPtr )
									{
										CDS_Actuator::AccessoryItem rActuatorAccessoryItem;
										rActuatorAccessoryItem.IDPtr = rAccessoryItem.IDPtr;
										rActuatorAccessoryItem.fByPair = rAccessoryItem.fByPair;
										pSelectedActuator->AddActuatorAccessory( &rActuatorAccessoryItem );
										rAccessoryItem = pclAccessoryList->GetNext();
									}
								}

								m_pclSelectionTable->Insert( ActuatorIDPtr );
								bRemoveObject = true;
							}
						}
					
						if( NULL != pclSSelCtrl->GetAccessoryList() )
						{
							CAccessoryList *pclAccessoryList = pclSSelCtrl->GetAccessoryList();
						
							if( pclAccessoryList->GetCount( CAccessoryList::_AT_Adapter ) > 0 )
							{
								// Create the 'CDS_Accessory' object.
								IDPTR AdapterIDPtr;
								TASApp.GetpTADS()->CreateObject( AdapterIDPtr, CLASS( CDS_Accessory ) );

								CDS_Accessory *pSelectedAdapter = (CDS_Accessory*)( AdapterIDPtr.MP );
						
								if( NULL != pSelectedAdapter )
								{
									CAccessoryList::AccessoryItem rAccessoryItem = pclAccessoryList->GetFirst( CAccessoryList::_AT_Adapter );
									pSelectedAdapter->SetAccessoryIDPtr( rAccessoryItem.IDPtr );
								
									CSelectedInfos *pclAdapterSelectedInfos = pSelectedAdapter->GetpSelectedInfos();
									
									if( NULL == pclAdapterSelectedInfos )
									{
										HYSELECT_THROW( _T("Internal error: Can't retrieve the 'CSelectedInfos' object from the newly 'CDS_Accessory' created object.") );
									}

									CSelectedInfos *pclCVSelectedInfos = pclSSelCtrl->GetpSelectedInfos();

									if( NULL == pclCVSelectedInfos )
									{
										HYSELECT_THROW( _T("Internal error: Can't retrieve the 'CSelectedInfos' object from the 'CDS_SSelCtrl' object (%s)."), pclSSelCtrl->GetIDPtr().ID );
									}

									pclAdapterSelectedInfos->SetQuantity( pclCVSelectedInfos->GetQuantity() );
									pclAdapterSelectedInfos->SetReference( CSelectedInfos::eReferences::eRef1, pclCVSelectedInfos->GetReference( CSelectedInfos::eReferences::eRef1 ) );
									pclAdapterSelectedInfos->SetReference( CSelectedInfos::eReferences::eRef2, pclCVSelectedInfos->GetReference( CSelectedInfos::eReferences::eRef2 ) );
									pclAdapterSelectedInfos->SetRemark( pclCVSelectedInfos->GetRemark( ) );
									pclAdapterSelectedInfos->SetRemarkIndex( pclCVSelectedInfos->GetRemarkIndex() );

									m_pclSelectionTable->Insert( AdapterIDPtr );
									bRemoveObject = true;
								}
							}
						}

						if( true == bRemoveObject )
						{
							m_pclSelectionTable->Remove( idptr );
							TASApp.GetpTADS()->DeleteObject( idptr );
							continue;
						}
					}
				}
			
				if( NULL != dynamic_cast<CDS_Accessory *>( idptr.MP ) )
				{
					parAccessory[iAccessoryCount] = (CDS_Accessory*)idptr.MP;
					iAccessoryCount++;
					iMaxIAcc = max( iMaxIAcc, iIndex );
				}
			
				if( NULL != dynamic_cast<CDS_Actuator *>( idptr.MP ) )
				{
					parActuator[iActuatorCount] = (CDS_Actuator*)idptr.MP;
					iActuatorCount++;
					iMaxIActr = max( iMaxIActr, iIndex );
				}
			}
		
			if( 0 == iControlValveSelectedCount && 0 == iAccessoryCount && 0 == iActuatorCount )
			{
				delete[] parSelectedCv;
				delete[] parActuator;
				delete[] parAccessory;
				return false;
			}

			// Initialize Remark in TADS.
			int iRemIndex = -1;

			if( NULL != pDlgLeftTabSelP )
			{
				iRemIndex = pDlgLeftTabSelP->SetRemarkIndex( CLASS( CDS_SSelCtrl ), (CData**)parSelectedCv, iControlValveSelectedCount );
			}
		
			// Sort SelCv Array.
			// Remark: 'fResetOrder' is set to 'true' only when the sorting combos are empty (No user choice) and the user clicks on the 'Apply sorting keys'.
			if( true == fResetOrder )
			{
				// Reset row index.
				for( i = 0; i < iControlValveSelectedCount; i++ )
				{
					if( 0 == parSelectedCv[i]->GetpSelectedInfos()->GetRowIndex() )
					{
						parSelectedCv[i]->GetpSelectedInfos()->SetRowIndex( ++iMaxISelCv );
					}
				}
			}
			else if( PageField_enum::epfNONE != m_pTADSPageSetup->GetKey( 0 ) || PageField_enum::epfNONE != m_pTADSPageSetup->GetKey( 1 )
					|| PageField_enum::epfNONE != m_pTADSPageSetup->GetKey( 2 ) )
			{
				if( NULL != pDlgLeftTabSelP )
				{
					pDlgLeftTabSelP->SortTable( (CData**)parSelectedCv, iControlValveSelectedCount - 1 );
				}

				// Reset row index.
				for( i = 0; i < iControlValveSelectedCount; i++ )
				{
					parSelectedCv[i]->GetpSelectedInfos()->SetRowIndex( i + 1 );
				}
			}

			// Fill and format the table.
			long lSelectedRow = 0;
		
			// By default, the items are deleted when read.
			CRank rkl;
		
			// Divided in 6 :
			// k == 0  -> Single selection of CDB_ControlValve 
			// k == 1  -> and for CDB_PIControlValve.
			enum eCVswitch
			{
				SingleSelBVC,
				DirSelBVC,
				SingleSelCV,
				DirSelCV,
				SingleSelPICV,
				DirSelPICV,
				LastCVswitch
			};
		
			bool fAtLeastOneBlockPrinted = false;
		
			for( int iLoopGroup = 0; iLoopGroup < LastCVswitch; iLoopGroup++ )
			{
				int iCount = 0;

				// For each selected control valve.
				for( i = 0; i < iControlValveSelectedCount; i++ )
				{	
					bool bShouldbeAdded = false;
					CDS_SSelCtrl *pSelCtrl = dynamic_cast<CDS_SSelCtrl *>( parSelectedCv[i] );
				
					if( NULL == pSelCtrl )
					{
						HYSELECT_THROW( _T("Internal error: The selected object at position %i is not a 'CDS_SSelCtrl' object."), i );
					}
					else if( NULL == pSelCtrl->GetProductIDPtr().MP )
					{
						HYSELECT_THROW( _T("Internal error: The selected object at position %i doesn't contain a control valve."), i );
					}
				
					// Skip product selected from Hydraulic calculator  .... should never occur.
					if( pSelCtrl->GetHMIDptr().MP != NULL ) 
					{
						HYSELECT_THROW( _T("Internal error: The selected object at position %i belongs to an hydraulic network."), i );
					}
				
					if( SingleSelBVC == iLoopGroup || SingleSelCV == iLoopGroup || SingleSelPICV == iLoopGroup )
					{
						if( true == pSelCtrl->IsFromDirSel() )
						{
							continue;
						}
					}
					else
					{
						if( false == pSelCtrl->IsFromDirSel() )
						{
							continue;
						}
					}

					// All control products are based on 'CDB_RegulatingValve'.
					CDB_RegulatingValve *pRegV = dynamic_cast<CDB_RegulatingValve *>( pSelCtrl->GetProductIDPtr().MP );

					if( NULL == pRegV )
					{
						HYSELECT_THROW( _T("Internal error: The selected object '%s' at position %i is not a 'CDB_RegulatingValve' object."), pSelCtrl->GetProductIDPtr().ID, i );
					}

					switch( iLoopGroup )
					{
						case SingleSelBVC:
						case DirSelBVC:

							if( true == pRegV->IsaBCV() )
							{
								bShouldbeAdded = true;
							}

							break;

						case SingleSelCV:
						case DirSelCV:

							if( true == pRegV->IsaCV() )
							{
								bShouldbeAdded = true;
							}

							break;

						case SingleSelPICV:
						case DirSelPICV:

							if( true == pRegV->IsaPICV() )
							{
								bShouldbeAdded = true;
							}

							break;

						default:
							ASSERT( 0 );
							break;
					}
				
					if( false == bShouldbeAdded )
					{
						continue;
					}
				
					rkl.Add( _T(""), parSelectedCv[i]->GetpSelectedInfos()->GetRowIndex(), (LPARAM)parSelectedCv[i] );
					iCount++;
				}

				if( 0 == iCount )
				{
					continue;
				}

				if( i > 0 && true == fAtLeastOneBlockPrinted )
				{
					lRow++;
				}

				// Set the subtitle.
				SetLastRow( pclSheetDescriptionCv, lRow );
			
				switch( iLoopGroup )
				{
					case SingleSelBVC:
						SetPageTitle( pclSheetDescriptionCv, IDS_SSHEETSELPROD_SUBTITLEBCVFROMSSEL, false, lRow );
						break;

					case DirSelBVC:
						SetPageTitle( pclSheetDescriptionCv, IDS_SSHEETSELPROD_SUBTITLEBCVFROMDIRSEL, false, lRow );
						break;

					case SingleSelCV:
						SetPageTitle( pclSheetDescriptionCv, IDS_SSHEETSELPROD_SUBTITLECVFROMSSEL, false, lRow );
						break;

					case DirSelCV:
						SetPageTitle( pclSheetDescriptionCv, IDS_SSHEETSELPROD_SUBTITLECVFROMDIRSEL, false, lRow );
						break;

					case SingleSelPICV:
						SetPageTitle( pclSheetDescriptionCv, IDS_SSHEETSELPROD_SUBTITLEPICVFROMSSEL, false, lRow );
						break;

					case DirSelPICV:
						SetPageTitle( pclSheetDescriptionCv, IDS_SSHEETSELPROD_SUBTITLEPICVFROMDIRSEL, false, lRow );
						break;

					default:
						ASSERT( 0 );
						break;
				}

				lRow = pclSheet->GetMaxRows();
				SetLastRow( pclSheetDescriptionCv, lRow );
				CString str;
				LPARAM lparam;
				bool bFirstPass = true;
			
				for( BOOL bContinue = rkl.GetFirst( str, lparam ); TRUE == bContinue; bContinue = rkl.GetNext( str, lparam ) )
				{
					CData *pclData = (CData *)lparam;
					CDS_SSelCtrl *pclSSelCtrl = dynamic_cast<CDS_SSelCtrl *>( pclData );

					if( NULL == pclSSelCtrl )
					{
						delete[] parSelectedCv;
						delete[] parActuator;
						delete[] parAccessory;
						return false;
					}

					if( m_pSelected == pclData )
					{
						lSelectedRow = lRow;
					}

					m_rProductParam.Clear();
					m_rProductParam.SetSheetDescription( pclSheetDescriptionCv );
					m_rProductParam.SetSelectionContainer( (LPARAM)pclSSelCtrl );

					long lFirstRow = lRow + 1;
				
					// Only if CV is present.
					if( pclSSelCtrl->GetProductAs<CDB_ControlValve>() )
					{
						lRow++;
						lRow = _FillControlValve( pclSheetDescriptionCv, lRow, ( CDS_SSelCtrl *)pclSSelCtrl );

						// Spanning must be done here because it's the only place where we know exactly number of lines to span!
						// Span reference #1 and #2.
						AddCellSpanW( pclSheetDescriptionCv, ColumnDescription::Reference1, lFirstRow, 1, pclSheet->GetMaxRows() - lFirstRow + 1 );
						AddCellSpanW( pclSheetDescriptionCv, ColumnDescription::Reference2, lFirstRow, 1, pclSheet->GetMaxRows() - lFirstRow + 1 );
	
						// Span water characteristic.
						AddCellSpanW( pclSheetDescriptionCv, ColumnDescription::Water, lFirstRow, 1, pclSheet->GetMaxRows() - lFirstRow + 1 );
					
						// Add accessories.
						if( true == pclSSelCtrl->IsAccessoryExist() )
						{
							// Draw dash line.
							pclSheet->SetCellBorder( ColumnDescription::Reference1, lRow, ColumnDescription::Footer - 1, lRow, true, SS_BORDERTYPE_BOTTOM, SS_BORDERSTYLE_FINE_DASH );
					
							lRow++;
							lRow = _FillAccessoriesCvOrActr( pclSheetDescriptionCv, lRow, pclSSelCtrl, true );
						}
					}

					// Fill actuator if exist.
					if( NULL != dynamic_cast<CDB_ElectroActuator *>( pclSSelCtrl->GetActrIDPtr().MP ) )
					{
						// Draw dash line.
						pclSheet->SetCellBorder( ColumnDescription::Reference1, lRow, ColumnDescription::Footer - 1, lRow, true, SS_BORDERTYPE_BOTTOM, SS_BORDERSTYLE_FINE_DASH );

						lRow++;
						lRow = _FillActuatorSelectedWithCv( pclSheetDescriptionCv, lRow, pclSSelCtrl );
					}
				
					if( true == pclSSelCtrl->IsActuatorAccessoryExist() )
					{
						// Draw dash line.
						pclSheet->SetCellBorder( ColumnDescription::Reference1, lRow, ColumnDescription::Footer - 1, lRow, true, SS_BORDERTYPE_BOTTOM, SS_BORDERSTYLE_FINE_DASH );

						lRow++;
						lRow = _FillAccessoriesCvOrActr( pclSheetDescriptionCv, lRow, pclSSelCtrl, false );
					}
				
					// Draw line below.
					pclSheet->SetCellBorder( ColumnDescription::Reference1, lRow, ColumnDescription::Footer - 1, lRow, true, SS_BORDERTYPE_BOTTOM, SS_BORDERSTYLE_SOLID );

					// Save object pointer.
					m_rProductParam.SetScrollRange( ( true == bFirstPass ) ? lFirstRow - 1 : lFirstRow, lRow );
					SaveProduct( m_rProductParam );

					// Set all group as no breakable (for print).
					// Remark: include title with the group.
					pclSheet->SetFlagRowNoBreakable( ( true == bFirstPass ) ? lFirstRow - 1 : lFirstRow, lRow, true );
				
					_AddArticleList( pclSSelCtrl );

					bFirstPass = false;
				}

				lRow++;
				fAtLeastOneBlockPrinted = true;
			}

			// Verify if a product is selected.
			if( 0 == lSelectedRow )
			{
				m_pSelected = NULL;
			}

			if( true == _InitDSActuator( pclSheetDescriptionCv, parActuator, iActuatorCount, iMaxIActr, fResetOrder, iRemIndex ) )
			{
				lRow = pclSheet->GetMaxRows();
				lRow++;
			}

			if( true == _InitDSAccessory( pclSheetDescriptionCv, parAccessory, iAccessoryCount, iMaxIAcc, fResetOrder, iRemIndex ) )
			{
				lRow = pclSheet->GetMaxRows();
				lRow++;
			}

			FillRemarks( pclSheetDescriptionCv, lRow );

			// We add by default a blank line.
			lRow = pclSheet->GetMaxRows() + 2;

			delete[] parSelectedCv;
			delete[] parActuator;
			delete[] parAccessory;
		}

		if( true == fCvHMExist )
		{
			// By default, the items are deleted when read.
			CRank rkl;

			enum HMCVSwitch
			{
				HMCV,
				HMBCV,
				HMPICV,
				LastHMCVSwitch
			};

			// For control valve, the 'CSelProdDockPane::_ExtractHMInfos' insert CV, BCV and PICV in the same vector.
			// To complete remark index, we need to separate CV, BCV and PICV. 
			HMvector vecHMList;

			bool bAtLeastOneBlockPrinted = false;

			for( int iLoopGroup = 0; iLoopGroup < LastHMCVSwitch; iLoopGroup++ )
			{
				vecHMList.clear();
				int iCount = 0;

				// For each hydraulic circuit.
				int i;

				for( i = 0; i < (int)m_vecHMList.size(); i++ )
				{
					CDS_HydroMod::CCv *pHMCv = static_cast<CDS_HydroMod::CCv *>( m_vecHMList[i].second );

					if( NULL == pHMCv || NULL == dynamic_cast<CDB_ControlValve *>( pHMCv->GetCvIDPtr().MP ) )
					{
						continue;
					}

					CDB_ControlValve *pclControlValve = dynamic_cast<CDB_ControlValve *>( pHMCv->GetCvIDPtr().MP );
				
					if( HMCV == iLoopGroup )
					{
						if( true == pclControlValve->IsaCV() )
						{
							vecHMList.push_back( CSelProdPageBase::PairPtr( CSelProdPageBase::ePointerType::eptCCV, ( void * )pHMCv ) );
						}
						else
						{
							continue;
						}
					}
					else if( HMBCV == iLoopGroup )
					{
						if( true == pclControlValve->IsaBCV() )
						{
							vecHMList.push_back( CSelProdPageBase::PairPtr( CSelProdPageBase::ePointerType::eptCCV, ( void * )pHMCv ) );
						}
						else
						{
							continue;
						}
					}
					else if( HMPICV == iLoopGroup )
					{
						if( true == pclControlValve->IsaPICV() )
						{
							vecHMList.push_back( CSelProdPageBase::PairPtr( CSelProdPageBase::ePointerType::eptCCV, ( void * )pHMCv ) );
						}
						else
						{
							continue;
						}
					}

					rkl.Add( _T(""), i, (LPARAM)pHMCv );
					iCount++;
				}

				if( 0 == iCount )
				{
					continue;
				}

				// Create a remark index for the current group (CV, BCV or PIBCV).
				if( NULL != pDlgLeftTabSelP )
				{
					pDlgLeftTabSelP->SetRemarkIndex( &vecHMList );
				}

				if( i > 0 && true == bAtLeastOneBlockPrinted )
				{
					lRow++;
				}

				// Set the subtitle.
				SetLastRow( pclSheetDescriptionCv, lRow );

				switch( iLoopGroup )
				{
					case HMCV:
						SetPageTitle( pclSheetDescriptionCv, IDS_SSHEETSELPROD_SUBTITLECVFROMHM, false, lRow );
						break;

					case HMBCV:
						SetPageTitle( pclSheetDescriptionCv, IDS_SSHEETSELPROD_SUBTITLEBCVFROMHM, false, lRow );
						break;

					case HMPICV:
						SetPageTitle( pclSheetDescriptionCv, IDS_SSHEETSELPROD_SUBTITLEPICVFROMHM, false, lRow );
						break;

					default:
						ASSERT( 0 );
						break;
				}
			
				lRow = pclSheet->GetMaxRows();
				SetLastRow( pclSheetDescriptionCv, lRow );
			
				CString str;
				LPARAM lparam;
				bool fFirstPass = true;

				for( BOOL fContinue = rkl.GetFirst( str, lparam ); TRUE == fContinue; fContinue = rkl.GetNext( str, lparam ) )
				{
					CDS_HydroMod::CCv *pHMCv = (CDS_HydroMod::CCv*)lparam;

					m_rProductParam.Clear();
					m_rProductParam.SetSheetDescription( pclSheetDescriptionCv );
					// Intentionally set to NULL to avoid clicking, double clicking or right clicking.
					m_rProductParam.SetSelectionContainer( (LPARAM)0 );

					long lFirstRow = ++lRow;
					lRow = _FillControlValve( pclSheetDescriptionCv, lRow, pHMCv );

					// HYS-721: Merge cells
					// Spanning must be done here because it's the only place where we know exactly number of lines to span!
					// Span reference #1 and #2.
					AddCellSpanW(pclSheetDescriptionCv, ColumnDescription::Reference1, lFirstRow, 1, pclSheet->GetMaxRows() - lFirstRow + 1);
					AddCellSpanW(pclSheetDescriptionCv, ColumnDescription::Reference2, lFirstRow, 1, pclSheet->GetMaxRows() - lFirstRow + 1);
					// Span water characteristic.
					AddCellSpanW(pclSheetDescriptionCv, ColumnDescription::Water, lFirstRow, 1, pclSheet->GetMaxRows() - lFirstRow + 1);

					CDB_TAProduct *pclTAProd = dynamic_cast<CDB_TAProduct *>(pHMCv->GetCvIDPtr().MP);
					lRow = FillAndAddBuiltInHMAccessories(pclSheet, pclSheetDescriptionCv, pclTAProd, lRow);

					// Fill set accessories if exist.
					if( pHMCv->GetCVActrAccSetCount() > 0 )
					{
						// Draw dash line.
						pclSheet->SetCellBorder( ColumnDescription::Reference1, lRow, ColumnDescription::Footer - 1, lRow, true, SS_BORDERTYPE_BOTTOM, SS_BORDERSTYLE_FINE_DASH );

						lRow++;
						lRow = _FillSetAccessoriesHM( pclSheetDescriptionCv, lRow, pHMCv );
					}

					// Fill adapter if exist.
					if( ( NULL != dynamic_cast<CDB_Product*>( (CData*)pHMCv->GetActrAdaptIDPtr().MP ) ) && 
						( (dynamic_cast<CDB_Product*>( (CData*)pHMCv->GetActrAdaptIDPtr().MP ) )->IsAnAccessory() ) )
					{
						// Draw dash line.
						pclSheet->SetCellBorder( ColumnDescription::Reference1, lRow, ColumnDescription::Footer - 1, lRow, true, SS_BORDERTYPE_BOTTOM, SS_BORDERSTYLE_FINE_DASH );

						// Add adapter if we can automatically select it.
						lRow++;
						lRow = _FillAdapterFromHM( pclSheetDescriptionCv, lRow, pHMCv );
					}

					// Fill actuator if exist.
					if( NULL != dynamic_cast<CDB_ElectroActuator *>( pHMCv->GetActrIDPtr().MP ) )
					{
						// Draw dash line.
						pclSheet->SetCellBorder( ColumnDescription::Reference1, lRow, ColumnDescription::Footer - 1, lRow, true, SS_BORDERTYPE_BOTTOM, SS_BORDERSTYLE_FINE_DASH );

						lRow++;
						lRow = _FillActuatorFromHM( pclSheetDescriptionCv, lRow, pHMCv );
					}

					// Draw line below.
					pclSheet->SetCellBorder( ColumnDescription::Reference1, lRow, ColumnDescription::Footer - 1, lRow, true, SS_BORDERTYPE_BOTTOM, SS_BORDERSTYLE_SOLID );

					// Save the product.
					m_rProductParam.SetScrollRange( ( true == fFirstPass ) ? lFirstRow - 1 : lFirstRow, lRow );
					SaveProduct( m_rProductParam );
					
					// Set all group as no breakable (for print).
					// Remark: include title with the group.
					pclSheet->SetFlagRowNoBreakable( ( true == fFirstPass ) ? lFirstRow - 1 : lFirstRow, lRow, true );
					
					_AddArticleList( pHMCv );

					fFirstPass = false;
				}

				lRow++;
				bAtLeastOneBlockPrinted = true;

				FillHMRemarks( &vecHMList, pclSheetDescriptionCv, lRow );

				// We add by default a blank line between two groups.
				lRow = pclSheet->GetMaxRows() + 1;
			}
		}

		// Move sheet to correct position.
		SetSheetSize();
		Invalidate();
		UpdateWindow();

		return true;
	}
	catch( CHySelectException &clHySelectException )
	{
		clHySelectException.AddMessage( _T("Error in 'CSelProdPageCtrl::Init'."), __LINE__, __FILE__ );
		throw;
	}
}

bool CSelProdPageCtrl::HasSomethingToDisplay( void )
{
	bool fCvExist = ( NULL != m_pclSelectionTable ) ? true : false;
	bool fCvHMExist = ( m_vecHMList.size() > 0 ) ? true : false;
	if( false == fCvExist && false == fCvHMExist )
		return false;
	return true;
}

void CSelProdPageCtrl::_InitColHeader( CSheetDescription* pclSheetDescription )
{
	if( NULL == pclSheetDescription || NULL == pclSheetDescription->GetSSheetPointer() )
		return;

	CSSheet* pclSheet = pclSheetDescription->GetSSheetPointer();
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

long CSelProdPageCtrl::_FillControlValve( CSheetDescription* pclSheetDescription, long lRow, CDS_HydroMod::CCv* pclHMCv )
{
	if( NULL == pclSheetDescription || NULL == pclSheetDescription->GetSSheetPointer() || NULL == pclHMCv )
		return lRow;

	CDB_ControlValve* pTApCV = dynamic_cast<CDB_ControlValve *>( pclHMCv->GetCvIDPtr().MP );
	if( NULL == pTApCV )
		return lRow;
	
	CSSheet* pclSheet = pclSheetDescription->GetSSheetPointer();
	CString str;
	long lMaxRow, lFirstRow;
	lMaxRow = lFirstRow = lRow;

	pclSheet->SetTextPattern( CSSheet::_SSTextPattern::UserStaticColumn );
	pclSheet->SetTextPatternProperty( CSSheet::_SSTextPatternProperty::TPP_FontBold, (LPARAM)FALSE );

	// Remark: 'true' to exclude any connection reference in the article number.
	CString strArticleNumber = pTApCV->GetArtNum( true );
	CDB_Set* pCvActSet = NULL;
	if( true == pTApCV->IsPartOfaSet() && eBool3::eb3True == pclHMCv->GetActrSelectedAsaPackage() )
	{
		CDB_Actuator* pclActuator = dynamic_cast<CDB_Actuator *>( pclHMCv->GetActrIDPtr().MP );
		if( NULL != pclActuator )
		{
			CTableSet* pTabSet = pTApCV->GetTableSet();		ASSERT( NULL != pTabSet );
			pCvActSet = pTabSet->FindCompatibleSet( pTApCV->GetIDPtr().ID, pclActuator->GetIDPtr().ID );
		}

		if( NULL != pCvActSet )
			strArticleNumber = pCvActSet->GetReference();	
	}
	
	if( NULL != pCvActSet )
	{
		FillArtNumberColForPackage( pclSheetDescription, ColumnDescription::ArticleNumber, lRow, pCvActSet, strArticleNumber );
		FillQtyPriceCol( pclSheetDescription, lRow, pCvActSet, 1 );
	}
	else
	{
		FillArtNumberCol( pclSheetDescription, ColumnDescription::ArticleNumber, lRow, pTApCV, strArticleNumber );
		FillQtyPriceCol( pclSheetDescription, lRow, pTApCV, 1 );
	}
	
	// HYS-1734: Add the valve to get temperature and pipe in FillRowGen.
	FillRowGen( pclSheetDescription, lRow, pclHMCv->GetpParentHM(), pclHMCv );

	// Column product.
	_FillCvTAP( pclSheetDescription, lRow, pTApCV );

	// Column infos.
	lMaxRow = max( lRow, lMaxRow );
	lRow = lFirstRow;
	AddStaticText( pclSheetDescription, ColumnDescription::TechnicalInfos, lRow, _T("-") );

	// Flow.
	if( true == m_pTADSPageSetup->GetField( epfCVTECHINFOFLOW ) )
	{
		double dQ = pclHMCv->GetQ();				ASSERT( dQ >0.0 );
		if( 0.0 != dQ )
		{
			str = WriteCUDouble( _U_FLOW, dQ, true );
			lRow = AddStaticText( pclSheetDescription, ColumnDescription::TechnicalInfos, lRow, str );
		}
	}

	// Dp.
	if( true == m_pTADSPageSetup->GetField( epfCVTECHINFODP ) )
	{
		double dDp = pclHMCv->GetDp();
		if( 0.0 != dDp )
		{
			if( true == pTApCV->IsaPICV() )
			{
				str = TASApp.LoadLocalizedString( IDS_DPMIN );
				str += (CString)_T(" = ");
				str += WriteCUDouble( _U_DIFFPRESS, dDp, true );
			}
			else
			{
				str = WriteCUDouble( _U_DIFFPRESS, dDp, true );
			}

			lRow = AddStaticText( pclSheetDescription, ColumnDescription::TechnicalInfos, lRow, str );
		}
	}

	// Presetting.
	if( true == m_pTADSPageSetup->GetField( epfCVTECHINFOSETTING ) )
	{
		str = _T("-");
		if ( pTApCV->IsaBCV() || pTApCV->IsaPICV() )		
		{
			str = pclHMCv->GetSettingStr( true );
			if( true == str.IsEmpty() )
				str = L"-";
		}
		lRow = AddStaticText( pclSheetDescription, ColumnDescription::TechnicalInfos, lRow, str );
	}
	
	m_rProductParam.AddRange( lFirstRow, pclSheet->GetMaxRows(), pTApCV );
	return pclSheet->GetMaxRows();
}

long CSelProdPageCtrl::_FillControlValve( CSheetDescription* pclSheetDescription, long lRow, CDS_SSelCtrl* pclSSelCtrl )
{
	if( NULL == pclSheetDescription || NULL == pclSheetDescription->GetSSheetPointer() || NULL == pclSSelCtrl )
		return lRow;

	CDB_ControlValve* pclControlValve = pclSSelCtrl->GetProductAs<CDB_ControlValve>(); 
	if( NULL == pclControlValve )
		return lRow;
	
	CSSheet* pclSheet = pclSheetDescription->GetSSheetPointer();
	CString str;
	long lFirstRow = lRow;

	pclSheet->SetTextPattern( CSSheet::_SSTextPattern::UserStaticColumn );
	pclSheet->SetTextPatternProperty( CSSheet::_SSTextPatternProperty::TPP_FontBold, (LPARAM)FALSE );

	CString strArticleNumber;
	if( true == pclSSelCtrl->IsSelectedAsAPackage( true ) )
	{
		CDB_Set* pCvActSet = dynamic_cast<CDB_Set *>( pclSSelCtrl->GetCvActrSetIDPtr().MP );
		strArticleNumber = pCvActSet->GetReference();
		FillArtNumberColForPackage( pclSheetDescription, ColumnDescription::ArticleNumber, lRow, pCvActSet, strArticleNumber );
		FillQtyPriceCol( pclSheetDescription, lRow, pCvActSet, pclSSelCtrl->GetpSelectedInfos()->GetQuantity() );
	}
	else
	{
		// Remark: 'true' to exclude any connection reference in the article number.
		strArticleNumber = pclControlValve->GetArtNum( true );
		FillArtNumberCol( pclSheetDescription, ColumnDescription::ArticleNumber, lRow, pclControlValve, strArticleNumber );
		FillQtyPriceCol( pclSheetDescription, lRow, pclControlValve, pclSSelCtrl->GetpSelectedInfos()->GetQuantity() );
	}

	FillRowGen( pclSheetDescription, lRow, pclSSelCtrl );
	
	// Column TA Product.
	long lRowProduct = _FillCvTAP( pclSheetDescription, lRow, pclControlValve );

	// Column Infos.
	lRow = lFirstRow;
	AddStaticText( pclSheetDescription, ColumnDescription::TechnicalInfos, lRow, _T("-") );

	CDB_ControlValve::ControlValveTable eControlValveTable = pclControlValve->GetCVParentTable();
	
	// Flow.
	// If Cv is directly selected don't fill Water, pipes.
	if( false == pclSSelCtrl->IsFromDirSel() )
	{
		double dQ = pclSSelCtrl->GetQ();															ASSERT( dQ > 0.0 );
		if( true == m_pTADSPageSetup->GetField( epfCVTECHINFOFLOW ) && 0.0 != dQ )
		{
			// HYS-38: Show power dt info when their radio button is checked
			CString strPowerDt = _T( "" );
			if( CDS_SelProd::efdPower == pclSSelCtrl->GetFlowDef() )
			{
				strPowerDt = WriteCUDouble( _U_TH_POWER, pclSSelCtrl->GetPower(), true );
				strPowerDt += _T(" / ");
				strPowerDt += WriteCUDouble( _U_DIFFTEMP, pclSSelCtrl->GetDT(), true );
			}
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

		// Dp.
		double dRho = pclSSelCtrl->GetpSelectedInfos()->GetpWCData()->GetDens();
		double dKinVisc = pclSSelCtrl->GetpSelectedInfos()->GetpWCData()->GetKinVisc();
		if( true == m_pTADSPageSetup->GetField( epfCVTECHINFODP ) )
		{
			switch( eControlValveTable )
			{
				case CDB_ControlValve::ControlValveTable::CV:
					{
						double dDp = CalcDp( dQ, pclControlValve->GetKvs(), dRho );
						str = WriteCUDouble( _U_DIFFPRESS, dDp, true );
					}
					break;

				case CDB_ControlValve::ControlValveTable::BCV:
					{
						CDS_SSelBCv *pSelBCv = dynamic_cast<CDS_SSelBCv*>( pclSSelCtrl );
						double dH = pSelBCv->GetOpening();
						if( dH > 0.0 )
						{
							// Compute Dp for current opening and current flow
							CDB_ValveCharacteristic *pValvChar = (CDB_ValveCharacteristic *)pclControlValve->GetValveCharDataPointer();
							
							if( NULL != pValvChar )
							{
								double dDp = 0.0;
								if( true == pValvChar->GetValveDp( dQ, &dDp, dH, dRho, dKinVisc ) )
								{
									str = WriteCUDouble( _U_DIFFPRESS, dDp, true );
								}
							}
						}
					}
					break;

				case CDB_ControlValve::ControlValveTable::PICV:
					{
						CDB_PIControlValve *pPICV = pclSSelCtrl->GetProductAs<CDB_PIControlValve>();
						double dDpMin = pPICV->GetDpmin( dQ, dRho );
						if( -1.0 != dDpMin )
						{
							str = TASApp.LoadLocalizedString( IDS_DPMIN );
							str += (CString)_T(" = ");
							str += WriteCUDouble( _U_DIFFPRESS, dDpMin, true );
						}
						else
							str = GetDashDotDash();
					}
					break;
			}

			lRow = AddStaticText( pclSheetDescription, ColumnDescription::TechnicalInfos, lRow, str );
		}

		// Presetting.
		if( true == m_pTADSPageSetup->GetField( epfCVTECHINFOSETTING ) )
		{
			str = _T("-");
			switch( eControlValveTable )
			{
				case CDB_ControlValve::ControlValveTable::CV:
					break;

				case CDB_ControlValve::ControlValveTable::BCV:
					{
						CDS_SSelBCv *pSelBCv = dynamic_cast<CDS_SSelBCv *>( pclSSelCtrl );
						double dH = pSelBCv->GetOpening();

						if( 0.0 != dH && NULL != pclControlValve->GetValveCharDataPointer() )
						{
							CDB_ValveCharacteristic *pValvChar = (CDB_ValveCharacteristic *)pclControlValve->GetValveCharDataPointer();
							str = pValvChar->GetSettingString( dH, true );
						}
					}
					break;

				case CDB_ControlValve::ControlValveTable::PICV:
					{
						CDB_PIControlValve *pPICV = pclSSelCtrl->GetProductAs<CDB_PIControlValve>();
						CDB_ValveCharacteristic *pValvChar = pPICV->GetValveCharacteristic();

						if( NULL != pValvChar && 0.0 != dQ )
						{
							double dPresetting = pPICV->GetPresetting( dQ, dRho, dKinVisc );

							if( -1.0 != dPresetting )
							{
								str = pValvChar->GetSettingString( dPresetting, true );
							}
						}
					}
					break;
			}
			
			lRow = AddStaticText( pclSheetDescription, ColumnDescription::TechnicalInfos, lRow, str );
		}
	}
	
	m_rProductParam.AddRange( lFirstRow, pclSheet->GetMaxRows(), pclControlValve );
	return pclSheet->GetMaxRows();
}

long CSelProdPageCtrl::_FillCvTAP( CSheetDescription* pclSheetDescription, long lRow, CDB_TAProduct* pTAP )
{
	if( NULL == pclSheetDescription || NULL == pclSheetDescription->GetSSheetPointer() || NULL == pTAP )
		return lRow;

	CString str;

	// Column 'Product'.
	AddStaticText( pclSheetDescription, ColumnDescription::Product, lRow, _T("-") );
	
	if( true == m_pTADSPageSetup->GetField( epfCVINFOPRODUCTNAME ) )
		lRow = AddStaticText( pclSheetDescription, ColumnDescription::Product, lRow, pTAP->GetName() );

	if( true == m_pTADSPageSetup->GetField( epfCVINFOPRODUCTKV ) )
	{
		CDB_ControlValve *pTACV = dynamic_cast<CDB_ControlValve *>( pTAP );

		if( NULL != pTACV )
		{
			if( NULL == pTACV->GetValveCharDataPointer() && -1.0 != pTACV->GetKvs() )
			{
				// BVC or pure CV.
				if( 0 == m_pUnitDB->GetDefaultUnitIndex( _C_KVCVCOEFF ) )
				{
					str = TASApp.LoadLocalizedString( IDS_KVS );
				}
				else
				{
					str = TASApp.LoadLocalizedString( IDS_CV );
				}

				str += CString( _T(" = ") ) + WriteCUDouble( _C_KVCVCOEFF, pTACV->GetKvs() );
				lRow = AddStaticText( pclSheetDescription, ColumnDescription::Product, lRow, str );
			}
		}
		else
		{
			ASSERT( 0 );
		}
	}

	// Body Material.
	if( true == m_pTADSPageSetup->GetField( epfCVINFOPRODUCTBDYMATERIAL ) )
	{
		CDB_StringID* pStrID = dynamic_cast <CDB_StringID *>( pTAP->GetBodyMaterialIDPtr().MP );
		if( NULL != pStrID )
		{
			lRow = AddStaticText( pclSheetDescription, ColumnDescription::Product, lRow,pStrID->GetString() );
		}
	}
	
	// Connection.
	if( true == m_pTADSPageSetup->GetField( epfCVINFOPRODUCTCONNECTION ) )
	{
		CDB_StringID* pStrID = dynamic_cast <CDB_StringID *>( pTAP->GetConnectIDPtr().MP );											
		if( NULL != pStrID )
		{
			str = pStrID->GetString();
			if( 0 == IDcmp( pTAP->GetConnTabID(), _T("COMPONENT_TAB") ) )
				str += CString( _T(" -> ") ) + pTAP->GetSize();	
			lRow = AddStaticText( pclSheetDescription, ColumnDescription::Product, lRow, str );
		}
	}

	// Version.
	if( true == m_pTADSPageSetup->GetField( epfCVINFOPRODUCTVERSION ) )
	{
		CDB_StringID* pStrID = dynamic_cast <CDB_StringID *>( pTAP->GetVersionIDPtr().MP );											
		if( NULL != pStrID )
		{
			lRow = AddStaticText( pclSheetDescription, ColumnDescription::Product, lRow, pStrID->GetString() );
		}
	}

	// PN.
	if( true == m_pTADSPageSetup->GetField( epfCVINFOPRODUCTPN ) )
	{
		lRow = FillPNTminTmax( pclSheetDescription, ColumnDescription::Product, lRow, pTAP );
	}

	return pclSheetDescription->GetSSheetPointer()->GetMaxRows();
}

long CSelProdPageCtrl::_FillAccessoryAlone( CSheetDescription* pclSheetDescription, long lRow, CDS_Accessory* pclSSelAccessory, bool fWithArticleNumber )
{
	if( NULL == pclSheetDescription || NULL == pclSheetDescription->GetSSheetPointer() || NULL == pclSSelAccessory )
		return lRow;
	
	CSSheet* pclSheet = pclSheetDescription->GetSSheetPointer();

	pclSheet->SetTextPattern( CSSheet::_SSTextPattern::UserStaticColumn );
	pclSheet->SetTextPatternProperty( CSSheet::_SSTextPatternProperty::TPP_FontBold, (LPARAM)FALSE );
	CDB_Product* pclAccessory = (CDB_Product *)pclSSelAccessory->GetAccessoryIDPtr().MP;
	AddStaticText( pclSheetDescription, ColumnDescription::Reference1, lRow, pclAccessory->GetComment() );

	AddCellSpanW( pclSheetDescription, ColumnDescription::Reference1, lRow, 3, 1 );
	AddStaticText( pclSheetDescription, ColumnDescription::Product, lRow, pclAccessory->GetName() );

	CString str;
	str.Format( _T("%d"), pclSSelAccessory->GetpSelectedInfos()->GetQuantity() );
	AddStaticText( pclSheetDescription, ColumnDescription::Quantity, lRow, str );

	// Set font color to red when accessory is not deleted or not available.
	if( true == pclAccessory->IsDeleted() || false == pclAccessory->IsAvailable() )
		pclSheet->SetTextPatternProperty( CSSheet::_SSTextPatternProperty::TPP_FontForeColor, (LPARAM)_RED );

	// If accessory is included into CvActSet, mask his article number.
	if( false == fWithArticleNumber )
	{
		AddStaticText( pclSheetDescription, ColumnDescription::ArticleNumber, lRow, _T("-") );
	}
	else
	{
		// Check first if we have a local article number available.
		CString strLocalArticleNumber = _T("");
		if( true == TASApp.IsLocalArtNumberUsed() )
		{
			strLocalArticleNumber = LocArtNumTab.GetLocalArticleNumber( pclAccessory->GetArtNum() );
			strLocalArticleNumber.Trim();
		}

		// Fill article number.
		bool fArticleNumberShown = false;
		// If we can show article number OR we can't show but there is no local article number...
		if( false == TASApp.IsTAArtNumberHidden() || ( true == TASApp.IsTAArtNumberHidden() && true == strLocalArticleNumber.IsEmpty() ) )
		{
			CString str = pclAccessory->GetArtNum();
			if( false == str.IsEmpty() )
			{
				AddStaticText( pclSheetDescription, ColumnDescription::ArticleNumber, lRow, pclAccessory->GetArtNum() );
				fArticleNumberShown = true;
			}
		}

		// Set local article number if allowed and exist.
		long lNewRow = lRow + ( ( true == fArticleNumberShown ) ? 1 : 0 );
		if( true == TASApp.IsLocalArtNumberUsed() && false == strLocalArticleNumber.IsEmpty() )
		{
			strLocalArticleNumber = (CString)TASApp.GetLocalArtNumberName() + LocArtNumTab.GetLocalArticleNumber( pclAccessory->GetArtNum() );
			strLocalArticleNumber.Trim();
			AddStaticText( pclSheetDescription, ColumnDescription::ArticleNumber, lNewRow, strLocalArticleNumber );
		}
	}

	long lFlagRow = pclSheet->GetMaxRows() + 1;
	if( true == pclAccessory->IsDeleted() )
		AddStaticText( pclSheetDescription, ColumnDescription::ArticleNumber, lFlagRow, TASApp.LoadLocalizedString( IDS_TAPDELETED ) );
	else if( false == pclAccessory->IsAvailable() )
		AddStaticText( pclSheetDescription, ColumnDescription::ArticleNumber, lFlagRow, TASApp.LoadLocalizedString( IDS_TAPNOTAVAILABLE ) );

	pclSheet->SetTextPatternProperty( CSSheet::_SSTextPatternProperty::TPP_FontForeColor, (LPARAM)_BLACK );

	// Column Remark.
	if( true == m_pTADSPageSetup->GetField( epfREMARK ) )	
	{
		if( 0 != pclSSelAccessory->GetpSelectedInfos()->GetRemarkIndex() )		// remark exist
		{
			str.Format( _T("%d"), pclSSelAccessory->GetpSelectedInfos()->GetRemarkIndex() );
			lRow = AddStaticText( pclSheetDescription, ColumnDescription::Remark, lRow, str );
		}
	}
	return pclSheet->GetMaxRows();
}

long CSelProdPageCtrl::_FillAccessoriesCvOrActr( CSheetDescription* pclSheetDescription, long lRow, CDS_SSelCtrl* pclSSelCtrl, bool fForCtrl )
{
	if( NULL == pclSheetDescription || NULL == pclSheetDescription->GetSSheetPointer() || NULL == pclSSelCtrl )
		return lRow;

	CAccessoryList* pclAccessoryList = ( true == fForCtrl ) ? pclSSelCtrl->GetCvAccessoryList() : pclSSelCtrl->GetActuatorAccessoryList();
	int iCount = pclAccessoryList->GetCount();
	if( iCount > 0 )
	{
		CRank rkList;
		CAccessoryList::AccessoryItem rAccessoryItem = pclAccessoryList->GetFirst();
		while( rAccessoryItem.IDPtr.MP != NULL )
		{
			CDB_Product* pclAccessory = dynamic_cast<CDB_Product *>( rAccessoryItem.IDPtr.MP );				
			VERIFY( pclAccessory != NULL );
			if( NULL == pclAccessory || false == pclAccessory->IsAnAccessory() )
				break;
			
			CAccessoryList::AccessoryItem* pclAccessoryItem = new CAccessoryList::AccessoryItem();
			if( NULL == pclAccessoryItem )
				break;
			*pclAccessoryItem = rAccessoryItem;
			rkList.AddStrSort( pclAccessory->GetName(), 0, (LPARAM)pclAccessoryItem, false, false );
			rAccessoryItem = pclAccessoryList->GetNext();
		}

		CString str;
		LPARAM lParam;

		CDB_Set* pCvActSet = dynamic_cast<CDB_Set *>( pclSSelCtrl->GetCvActrSetIDPtr().MP );
		CDB_RuledTable* pclAccessoryGroupInSet = NULL;
		std::set< CDB_Product* > setCVAccessorySetList;
		if( NULL != pCvActSet )
		{
			pclAccessoryGroupInSet = dynamic_cast<CDB_RuledTable *>( pCvActSet->GetAccGroupIDPtr().MP );
			if( NULL == pclAccessoryGroupInSet )
			{
				// Particular case for FUSION-C and FUSION-P in package selection mode. When we have no accessory group IDPTR defined in 'CDB_Set', 
				// we take an union between accessories belonging to valve and actuators (see for example code in 'CRViewSSelBCv::GetAdapterList').
				CDB_ControlValve* pTApCV = pclSSelCtrl->GetProductAs<CDB_ControlValve>();
				if( pTApCV != NULL && true == pclSSelCtrl->IsSelectedAsAPackage() &&
					( 0 == CString( pTApCV->GetFamilyID() ).Compare( _T("FAM_FUSION_C") ) ||
					  0 == CString( pTApCV->GetFamilyID() ).Compare( _T("FAM_FUSION_P") ) ) )
				{
					GetFUSIONAccessories( pTApCV, &setCVAccessorySetList );
				}
			}
		}

		for( BOOL fContinue = rkList.GetFirst( str, lParam ); TRUE == fContinue; fContinue = rkList.GetNext( str, lParam ) )
		{
			CAccessoryList::AccessoryItem* pclAccessoryItem = (CAccessoryList::AccessoryItem*)lParam;
			CDB_Product* pclAccessory = dynamic_cast<CDB_Product *>( (CData *)( pclAccessoryItem->IDPtr.MP ) );
			bool fByPair = pclAccessoryItem->fByPair;
			
			if( NULL == pclAccessory || false == pclAccessory->IsAnAccessory() )
				continue;
			
			// If accessory is included into CvActSet, mask his article number.
			bool fWithArticleNumber = true;
			if( true == fForCtrl )
			{
				// Don't show adapter article number if it belongs to a set.
				if( ( NULL != pclAccessoryGroupInSet && '\0' != *pclAccessoryGroupInSet->Get( pclAccessory->GetIDPtr().ID).ID ) ||
					( setCVAccessorySetList.size() > 0 && setCVAccessorySetList.count( pclAccessory ) > 0 ) )
					fWithArticleNumber = false;
			}

			int iQuantity = pclSSelCtrl->GetpSelectedInfos()->GetQuantity();
			// HYS-987: If the accessory quantity is changed take the new value
			if( -1 != pclAccessoryItem->lEditedQty )
			{
				iQuantity = pclAccessoryItem->lEditedQty;
			}
			else if( true == fByPair )
				iQuantity *= 2;

			delete pclAccessoryItem;

			lRow = FillAccessory( pclSheetDescription, lRow, pclAccessory, fWithArticleNumber, iQuantity );
			lRow++;
		}
	}
	return pclSheetDescription->GetSSheetPointer()->GetMaxRows();
}

long CSelProdPageCtrl::_FillSetAccessoriesHM( CSheetDescription* pclSheetDescription, long lRow, CDS_HydroMod::CCv* pHMCv )
{
	if( NULL == pclSheetDescription || NULL == pclSheetDescription->GetSSheetPointer() || NULL == pHMCv || NULL == pHMCv->GetpCV()
		|| _NULL_IDPTR == pHMCv->GetActrIDPtr() || 0 == pHMCv->GetCVActrAccSetCount() )
	{
		return lRow;
	}

	CDB_ControlValve* pclControlValve = pHMCv->GetpCV();
	CTableSet* pCVActSetTab = pclControlValve->GetTableSet();
	ASSERT( NULL != pCVActSetTab );

	if( NULL == pCVActSetTab )
	{
		return lRow;
	}

	CDB_Actuator* pclActuator = (CDB_Actuator*)( pHMCv->GetActrIDPtr().MP );
	CDB_Set* pCVActrSet = pCVActSetTab->FindCompatibleSet( pclControlValve->GetIDPtr().ID, pclActuator->GetIDPtr().ID );

	if( NULL == pCVActrSet )
	{
		return lRow;
	}

	// CV-Actuator accessories set.
	CDB_RuledTable* pclRuledTable = dynamic_cast<CDB_RuledTable*>( pCVActrSet->GetAccGroupIDPtr().MP );
	if( NULL != pclRuledTable )
	{
		// CV-Actuator accessories set.
		for( int iLoopCVActrAccSet = 0; iLoopCVActrAccSet < pHMCv->GetCVActrAccSetCount(); iLoopCVActrAccSet++ )
		{
			CDB_Product* pclAccessory = (CDB_Product*)( pHMCv->GetCVActrAccSetIDPtr( iLoopCVActrAccSet ).MP );
			if( NULL == pclAccessory || false == pclAccessory->IsAnAccessory() )
				continue;
						
			bool fByPair = pclRuledTable->IsByPair( pclAccessory->GetIDPtr().ID );
			lRow = FillAccessory( pclSheetDescription, lRow, pclAccessory, false, ( false == fByPair ) ? 1 : 2 );
			lRow++;
		}
	}

	return pclSheetDescription->GetSSheetPointer()->GetMaxRows();
}

long CSelProdPageCtrl::_FillActuatorSelectedWithCv( CSheetDescription *pclSheetDescription, long lRow, CDS_SSelCtrl *pclSSelCtrl )
{
	if( NULL == pclSheetDescription || NULL == pclSheetDescription->GetSSheetPointer() || NULL == pclSSelCtrl )
	{
		return lRow;
	}

	CSSheet *pclSheet = pclSheetDescription->GetSSheetPointer();
	
	CDB_ElectroActuator *pclElectroActuator = dynamic_cast<CDB_ElectroActuator *>( pclSSelCtrl->GetActrIDPtr().MP );

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
		if( true == pclSSelCtrl->IsSelectedAsAPackage( true ) )
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
		bool bShowPrice = !pclSSelCtrl->IsSelectedAsAPackage( true );
		FillQtyPriceCol( pclSheetDescription, lRow, pclElectroActuator, pclSSelCtrl->GetpSelectedInfos()->GetQuantity(), bShowPrice );
		
		// Write the actuator name.
		AddStaticText( pclSheetDescription, ColumnDescription::Product, lRow, pclElectroActuator->GetName() );
		AddCellSpanW( pclSheetDescription, ColumnDescription::Product, lRow++, 2, 1 );
		pclSheet->SetTextPatternProperty( CSSheet::_SSTextPatternProperty::TPP_StyleHorizontalAlign, (LPARAM)SSS_ALIGN_LEFT );

		CDB_ControlValve *pCv = pclSSelCtrl->GetProductAs<CDB_ControlValve>();
		
		if( NULL != pCv )
		{
			// Set the close off value if exist.
			CDB_CloseOffChar *pCloseOffChar = (CDB_CloseOffChar *)( pCv->GetCloseOffCharIDPtr().MP );

			if( NULL != pCloseOffChar )
			{
				if( CDB_CloseOffChar::CloseOffDp == pCloseOffChar->GetLimitType() )
				{
					str = TASApp.LoadLocalizedString( IDS_SSHEETSELPROD_CLOSEOFFDP );
					double dCloseOffDp = pCloseOffChar->GetCloseOffDp( pclElectroActuator->GetMaxForceTorque() );

					if( dCloseOffDp > 0.0 )
					{
						str += CString(_T(" : ")) + WriteCUDouble( _U_DIFFPRESS, dCloseOffDp, true );
					}
				} 
				else if( CDB_CloseOffChar::InletPressure == pCloseOffChar->GetLimitType() )
				{
					str = TASApp.LoadLocalizedString( IDS_SSHEETSELPROD_MAXINLETPRESSURE );
					double dMaxInletPressure = pCloseOffChar->GetMaxInletPressure( pclElectroActuator->GetMaxForceTorque() );

					if( dMaxInletPressure > 0.0 )
					{
						str += CString(_T(" : ")) + WriteCUDouble( _U_PRESSURE, dMaxInletPressure, true );
					}
				}

				AddStaticText( pclSheetDescription, ColumnDescription::Product, lRow, str );
				AddCellSpanW( pclSheetDescription, ColumnDescription::Product, lRow++, 2, 1 );
			}
				
			// Compute Actuating time in sec.
			str = TASApp.LoadLocalizedString( IDS_SSHEETSELPROD_ACTUATING_TIME ) + CString( _T(" : ") ) + pclElectroActuator->GetActuatingTimesStr( pCv->GetStroke(), true );
			AddStaticText( pclSheetDescription, ColumnDescription::Product, lRow, str );
			AddCellSpanW( pclSheetDescription, ColumnDescription::Product, lRow++, 2, 1 );
		}

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
		str = ( (int)pclElectroActuator->GetFailSafe() > 0 ) ? TASApp.LoadLocalizedString( IDS_SSHEETSSEL_ACTRFSP ) : TASApp.LoadLocalizedString( IDS_SSHEETSSEL_ACTRDRP );

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

long CSelProdPageCtrl::_FillActuatorSelectedAlone( CSheetDescription *pclSheetDescription, long lRow, CDS_Actuator *pclSSelActuator )
{
	if( NULL == pclSheetDescription || NULL == pclSheetDescription->GetSSheetPointer() || NULL == pclSSelActuator || NULL == pclSSelActuator->GetActuatorIDPtr().MP )
	{
		return lRow;
	}

	CDB_Actuator *pclActuator = dynamic_cast<CDB_Actuator *>( (CData *)( pclSSelActuator->GetActuatorIDPtr().MP ) );

	if( NULL == pclActuator )
	{
		return lRow;
	}

	// First: write the common part to all actuators.

	long lFirstRow = lRow;
	CSSheet *pclSheet = pclSheetDescription->GetSSheetPointer();
	CString str;
	pclSheet->SetTextPattern( CSSheet::_SSTextPattern::UserStaticColumn );
	pclSheet->SetTextPatternProperty( CSSheet::_SSTextPatternProperty::TPP_FontBold, (LPARAM)FALSE );
		
	// Set font color to red when accessory is not deleted or not available.
	if( true == pclActuator->IsDeleted() || false == pclActuator->IsAvailable() )
	{
		pclSheet->SetTextPatternProperty( CSSheet::_SSTextPatternProperty::TPP_FontForeColor, (LPARAM)_RED );
	}
	
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
	FillQtyPriceCol( pclSheetDescription, lRow, pclActuator, pclSSelActuator->GetpSelectedInfos()->GetQuantity() );
		
	// Write actuator name.
	AddStaticText( pclSheetDescription, ColumnDescription::Product, lRow, pclActuator->GetName() );
	pclSheet->SetTextPatternProperty( CSSheet::_SSTextPatternProperty::TPP_StyleHorizontalAlign, (LPARAM)SSS_ALIGN_LEFT );

	// Write first reference if exist.
	if( true == m_pTADSPageSetup->GetField( epfFIRSTREF ) )
	{
		pclSheet->SetRowHeight( lRow, m_dRowHeight );
		pclSheet->FormatEditText( ColumnDescription::Reference1, lRow, pclSSelActuator->GetpSelectedInfos()->GetReference( CSelectedInfos::eReferences::eRef1 ) );
	}

	// Write second reference if exist.
	if( true == m_pTADSPageSetup->GetField( epfSECONDREF ) )
	{
		pclSheet->SetRowHeight( lRow, m_dRowHeight );
		pclSheet->FormatEditText( ColumnDescription::Reference2, lRow, pclSSelActuator->GetpSelectedInfos()->GetReference( CSelectedInfos::eReferences::eRef2 ) );
	}

	// Write remark index if exist.
	if( true == m_pTADSPageSetup->GetField( epfREMARK ) )
	{
		if( 0 != pclSSelActuator->GetpSelectedInfos()->GetRemarkIndex() )
		{
			str.Format( _T("%d"), pclSSelActuator->GetpSelectedInfos()->GetRemarkIndex() );
			AddStaticText( pclSheetDescription, ColumnDescription::Remark, lRow, str );
		}
	}

	// Second: write part specific to each actuator.

	CDB_ElectroActuator *pclElectroActuator = dynamic_cast<CDB_ElectroActuator *>( (CData *)( pclSSelActuator->GetActuatorIDPtr().MP ) );
	CDB_ThermostaticActuator *pclThermostaticActuator = dynamic_cast<CDB_ThermostaticActuator *>( (CData *)( pclSSelActuator->GetActuatorIDPtr().MP ) );

	if( NULL != pclElectroActuator )
	{
		// Write max force (or torque).
		lRow++;

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
		str = ( (int)pclElectroActuator->GetFailSafe() > 0 ) ? TASApp.LoadLocalizedString( IDS_SSHEETSSEL_ACTRFSP ) : TASApp.LoadLocalizedString( IDS_SSHEETSSEL_ACTRDRP );
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
	}
	else if( pclThermostaticActuator != NULL )
	{
		// Write setting range.
		lRow++;
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
		
		if( iCapillaryLength != -1 && iCapillaryLength > 0 )
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
			str += CString( _T(" : ") ) + WriteDouble( dHysteresis, 3, 1, 1 ) + CString( _T(" K") );
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
		
		if( iFrostProtection != -1 && iFrostProtection > 0 )
		{
			str += CString( _T(" : ") ) + WriteCUDouble( _U_TEMPERATURE, iFrostProtection, true );
		}
		else
		{
			str += CString( _T(" : - ") );
		}

		AddStaticText( pclSheetDescription, ColumnDescription::Product, lRow, str );
		AddCellSpanW( pclSheetDescription, ColumnDescription::Product, lRow, 2, 1 );
	}

	// Center the text below.
	pclSheet->SetTextPatternProperty( CSSheet::_SSTextPatternProperty::TPP_StyleHorizontalAlign, (LPARAM)SSS_ALIGN_CENTER );

	m_rProductParam.AddRange( lFirstRow, pclSheet->GetMaxRows(), pclActuator );

	return pclSheet->GetMaxRows();
}

long CSelProdPageCtrl::_FillActuatorFromHM( CSheetDescription *pclSheetDescription, long lRow, CDS_HydroMod::CCv *pclHMCv )
{
	if( NULL == pclSheetDescription || NULL == pclSheetDescription->GetSSheetPointer() || NULL == pclHMCv )
	{
		return lRow;
	}
	
	CSSheet *pclSheet = pclSheetDescription->GetSSheetPointer();
	CString str;
	long lFirstRow = lRow;
	bool bfCvActrSet = false;
	
	// At now we accept only electric and thermoelectric actuator.
	CDB_ElectroActuator *pclElectroActuator = dynamic_cast<CDB_ElectroActuator *>( pclHMCv->GetActrIDPtr().MP );

	if( NULL == pclElectroActuator )
	{
		return pclSheet->GetMaxRows();
	}

	CDB_ControlValve *pclControlValve = dynamic_cast<CDB_ControlValve *>( pclHMCv->GetCvIDPtr().MP );

	if( NULL != pclElectroActuator && NULL != pclControlValve )
	{
		pclSheet->SetTextPattern( CSSheet::_SSTextPattern::UserStaticColumn );
		pclSheet->SetTextPatternProperty( CSSheet::_SSTextPatternProperty::TPP_FontBold, (LPARAM)FALSE );

		// Set font color to red when accessory is not deleted or not available.
		if( true == pclElectroActuator->IsDeleted() || false == pclElectroActuator->IsAvailable() )
		{
			pclSheet->SetTextPatternProperty( CSSheet::_SSTextPatternProperty::TPP_FontForeColor, (LPARAM)_RED );
		}
		
		// If selected as a CV-Actuator set don't show actuator article number.
		if( true == pclControlValve->IsPartOfaSet() && eBool3::eb3True == pclHMCv->GetActrSelectedAsaPackage() )
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
			
			// If local article number exist...
			long lNextRow = lRow + ( ( true == bArticleNumberShown ) ? 1 : 0 );

			if( true == TASApp.IsLocalArtNumberUsed() && false == strLocalArticleNumber.IsEmpty() )
			{
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

		// Write nothing for technical info.
		AddStaticText( pclSheetDescription, ColumnDescription::TechnicalInfos, lRow, _T("-") );
		
		// Write quantity and prices.
		bool bShowPrice = !( pclControlValve->IsPartOfaSet() && eBool3::eb3True == pclHMCv->GetActrSelectedAsaPackage() );
		FillQtyPriceCol( pclSheetDescription, lRow, pclElectroActuator, 1, bShowPrice );

		// Write the actuator name.
		lRow = AddStaticText( pclSheetDescription, ColumnDescription::Product, lRow, pclElectroActuator->GetName() );
		pclSheet->SetTextPatternProperty( CSSheet::_SSTextPatternProperty::TPP_StyleHorizontalAlign, SSS_ALIGN_LEFT );

		if( pclControlValve != NULL )
		{
			// Set the Close off value if exist.
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

			// Compute Actuating time in sec.
			str = TASApp.LoadLocalizedString( IDS_SSHEETSELPROD_ACTUATING_TIME ) + CString( _T(" : ") )	+ pclElectroActuator->GetActuatingTimesStr( pclControlValve->GetStroke(), true );
			lRow = AddStaticText( pclSheetDescription, ColumnDescription::Product, lRow, str );
			AddCellSpanW( pclSheetDescription, ColumnDescription::Product, lRow - 1, 2, 1 );
		}

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
		str = ( (int)pclElectroActuator->GetFailSafe() > 0 ) ? TASApp.LoadLocalizedString( IDS_SSHEETSSEL_ACTRFSP ) : TASApp.LoadLocalizedString( IDS_SSHEETSSEL_ACTRDRP );

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

		// Center the text below.
		pclSheet->SetTextPatternProperty( CSSheet::_SSTextPatternProperty::TPP_StyleHorizontalAlign, (LPARAM)SSS_ALIGN_CENTER );

		m_rProductParam.AddRange( lFirstRow, pclSheet->GetMaxRows(), pclElectroActuator );
	}

	return pclSheet->GetMaxRows();
}

long CSelProdPageCtrl::_FillAdapterFromHM( CSheetDescription* pclSheetDescription, long lRow, CDS_HydroMod::CCv* pclHMCV )
{
	if( NULL == pclSheetDescription || NULL == pclSheetDescription->GetSSheetPointer() || NULL == pclHMCV )
		return lRow;

	CDB_Product* pclAdapter = dynamic_cast<CDB_Product*>( (CData*)pclHMCV->GetActrAdaptIDPtr().MP );
	if( NULL == pclAdapter || false == pclAdapter->IsAnAccessory() )
		return lRow;

	bool fWithArticleNumber = ( eBool3::eb3True == pclHMCV->GetActrSelectedAsaPackage() ) ? false : true;
	lRow = FillAccessory( pclSheetDescription, lRow, pclAdapter, fWithArticleNumber, 1 );

	return pclSheetDescription->GetSSheetPointer()->GetMaxRows();
}

bool CSelProdPageCtrl::_InitDSActuator( CSheetDescription *pclSheetDescription, CDS_Actuator **paActr, int iCount, int iMax, bool fSort, int &iRemIndex )
{
	if( NULL == pclSheetDescription || NULL == pclSheetDescription->GetSSheetPointer() || 0 == iCount || NULL == paActr )
	{
		return false;
	}

	CSSheet *pclSheet = pclSheetDescription->GetSSheetPointer();

	//========================================
	// Sort Actuator Array
	//========================================
	if( true == fSort ) 
	{	
		if( NULL != pDlgLeftTabSelP )
		{
			pDlgLeftTabSelP->SortTable( (CData**)paActr, iCount - 1 );
		}

		// Reset row index.
		for( int i = 0; i < iCount; i++ )
		{
			paActr[i]->GetpSelectedInfos()->SetRowIndex( i + 1 );
		}
	}
	else
	{
		// Reset row index.
		for( int i = 0; i < iCount; i++ )
		{
			if( 0 == paActr[i]->GetpSelectedInfos()->GetRowIndex() )
			{
				paActr[i]->GetpSelectedInfos()->SetRowIndex( ++iMax );
			}
		}
	}

	// Initialize Remark in TADS.
	iRemIndex = -1;

	if( NULL != pDlgLeftTabSelP )
	{
		iRemIndex = pDlgLeftTabSelP->SetRemarkIndex( CLASS( CDS_Actuator ), (CData**)paActr, iCount, iRemIndex );
	}

	//====================================
	// Fill and format the table
	//====================================
	CRank rkl;

	for( int i = 0; i < iCount; i++ )
	{
		rkl.Add( _T(""), paActr[i]->GetpSelectedInfos()->GetRowIndex(), (LPARAM)paActr[i] );
	}

	// Set the sub title.
	long lSelectedRow = 0;
	long lRow = pclSheet->GetMaxRows() + 1;
	SetLastRow( pclSheetDescription, lRow );
	SetPageTitle( pclSheetDescription, IDS_SSHEETSELPROD_DIRSELACTUATOR, false, lRow );
	
	lRow++;
	
	CString str;
	LPARAM lParam;
	bool bFirstPass = true;

	for( BOOL bContinue = rkl.GetFirst( str, lParam ); TRUE == bContinue; bContinue = rkl.GetNext( str, lParam ) )
	{
		SetLastRow( pclSheetDescription, lRow );

		CData *pData = (CData *)lParam;
		CDS_Actuator *pclActuator = dynamic_cast<CDS_Actuator *>( pData );
		ASSERT( NULL != pclActuator );

		if( NULL == pclActuator )
		{
			return false;
		}
		
		if( m_pSelected == pData )
		{
			lSelectedRow = lRow;
		}

		m_rProductParam.Clear();
		m_rProductParam.SetSheetDescription( pclSheetDescription );
		m_rProductParam.SetSelectionContainer( (LPARAM)pclActuator );
		
		long lFirstRow = lRow;
		lRow = _FillActuatorSelectedAlone( pclSheetDescription, lRow, pclActuator );
		
		// Draw dash line.
		pclSheet->SetCellBorder( ColumnDescription::Reference1, lRow, ColumnDescription::Footer - 1, lRow, true, SS_BORDERTYPE_BOTTOM, SS_BORDERSTYLE_FINE_DASH );
		
		// Accessories.
		if( pclActuator->GetpActuatorAccessoryList()->size() > 0 )
		{
			lRow++;
			CRank rkList;

			for( int i = 0; i < (int)pclActuator->GetpActuatorAccessoryList()->size(); i++ )
			{
				CDS_Actuator::AccessoryItem *pclAccessoryItem = new CDS_Actuator::AccessoryItem();
				*pclAccessoryItem = pclActuator->GetpActuatorAccessoryList()->at( i );
				CDB_Product *pclAccessory = dynamic_cast<CDB_Product *>( pclAccessoryItem->IDPtr.MP );
				VERIFY( NULL != pclAccessory );

				if( NULL == pclAccessory || false == pclAccessory->IsAnAccessory() )
				{
					break;
				}

				rkList.AddStrSort( pclAccessory->GetName(), 0, (LPARAM)pclAccessoryItem, false, false );
			}

			CString str;

			for( BOOL bContinue = rkList.GetFirst( str, lParam ); TRUE == bContinue; bContinue = rkList.GetNext( str, lParam ) )
			{
				CDS_Actuator::AccessoryItem *pclAccessoryItem = (CDS_Actuator::AccessoryItem *)lParam;
				CDB_Product *pclAccessory = dynamic_cast<CDB_Product *>( (CData *)( pclAccessoryItem->IDPtr.MP ) );
				bool bByPair = pclAccessoryItem->fByPair;
				// HYS-987: Manage actuator accessories
				int ActAccQty = pclActuator->GetpSelectedInfos()->GetQuantity();
				if( -1 != pclAccessoryItem->lEditedQty )
				{
					ActAccQty = pclAccessoryItem->lEditedQty;
				}
				else if( true == bByPair )
				{
					ActAccQty *= 2;
				}
				delete pclAccessoryItem;

				lRow = FillAccessory( pclSheetDescription, lRow, pclAccessory, true, ActAccQty );
				lRow++;
			}
		}
		
		// Draw line below.
		pclSheet->SetCellBorder( ColumnDescription::Reference1, lRow, ColumnDescription::Footer - 1, lRow, true, SS_BORDERTYPE_BOTTOM, SS_BORDERSTYLE_SOLID );
		
		// Save product.
		m_rProductParam.SetScrollRange( ( true == bFirstPass ) ? lFirstRow - 1 : lFirstRow, lRow );
		SaveProduct( m_rProductParam );

		lRow++;
		
		_AddArticleList( (CDB_Actuator *)pclActuator->GetActuatorIDPtr().MP, pclActuator->GetpActuatorAccessoryList(), pclActuator->GetpSelectedInfos()->GetQuantity(), false );

		bFirstPass = false;
	}

	// Verify if a product is selected.
	if( 0 == lSelectedRow )
	{
		m_pSelected = 0;
	}

	return true;
}

bool CSelProdPageCtrl::_InitDSAccessory( CSheetDescription* pclSheetDescription, CDS_Accessory** paAcc, int iCount, int iMax, bool fSort, int &iRemIndex )
{
	if( NULL == pclSheetDescription || NULL == pclSheetDescription->GetSSheetPointer() || 0 == iCount || NULL == paAcc )
		return false;

	CSSheet* pclSheet = pclSheetDescription->GetSSheetPointer();

	//========================================
	// Sort Accessory Array
	//========================================
	if( true == fSort ) 
	{	
		if( NULL != pDlgLeftTabSelP )
		{
			pDlgLeftTabSelP->SortTable( (CData**)paAcc, iCount - 1 );
		}

		// Reset row index.
		for( int i = 0; i < iCount; i++ )
		{
			paAcc[i]->GetpSelectedInfos()->SetRowIndex( i + 1 );
		}
	}
	else
	{
		// Reset row index.
		for( int i = 0; i < iCount; i++ )
		{
			if( 0 == paAcc[i]->GetpSelectedInfos()->GetRowIndex() ) 
				paAcc[i]->GetpSelectedInfos()->SetRowIndex( ++iMax );
		}
	}

	// Initialize remark in TADS.
	iRemIndex = -1;

	if( NULL != pDlgLeftTabSelP )
	{
		iRemIndex = pDlgLeftTabSelP->SetRemarkIndex( CLASS( CDS_Accessory ), (CData**)paAcc, iCount, iRemIndex );
	}

	//====================================
	// Fill and format the table
	//====================================
	long lSelectedRow = 0;
	long lRow = pclSheet->GetMaxRows();
	SetLastRow( pclSheetDescription, lRow );
	LPARAM lparam;
	CRank rkl;
	for( int i = 0; i < iCount; i++ )
		rkl.Add( _T(""), paAcc[i]->GetpSelectedInfos()->GetRowIndex(), (LPARAM)paAcc[i] );

	// Set the sub title.
	SetLastRow( pclSheetDescription, lRow );
	SetPageTitle( pclSheetDescription, IDS_SSHEETSELPROD_DIRSELACCESSORIES, false, lRow );	
	
	lRow = pclSheet->GetMaxRows() + 1;
	
	CString str;
	bool fFirstPass = true;
	for( BOOL fContinue = rkl.GetFirst( str, lparam ); TRUE == fContinue; fContinue = rkl.GetNext( str, lparam ) )
	{
		SetLastRow( pclSheetDescription, lRow );

		CData* pData = (CData *)lparam;
		CDS_Accessory* pclAccessory = dynamic_cast<CDS_Accessory *>( pData );						ASSERT( NULL != pclAccessory );
		if( NULL == pclAccessory )
			return false;

		if( m_pSelected == pData )
			lSelectedRow = lRow;

		m_rProductParam.Clear();
		m_rProductParam.SetSheetDescription( pclSheetDescription );
		m_rProductParam.SetSelectionContainer( (LPARAM)pclAccessory );
		
		long lFirstRow = lRow;
		lRow = _FillAccessoryAlone( pclSheetDescription, lRow, pclAccessory, true );
		SetLastRow( pclSheetDescription, lRow );
		
		// Draw line below.
		pclSheet->SetCellBorder( ColumnDescription::Reference1, lRow, ColumnDescription::Footer - 1, lRow, true, SS_BORDERTYPE_BOTTOM, SS_BORDERSTYLE_SOLID );
		
		// Save product.
		m_rProductParam.SetScrollRange( ( true == fFirstPass ) ? lFirstRow - 1 : lFirstRow, lRow );
		SaveProduct( m_rProductParam );
		
		lRow++;

		AddAccessoryInArticleContainer( (CDB_Product *)pclAccessory->GetAccessoryIDPtr().MP, NULL, pclAccessory->GetpSelectedInfos()->GetQuantity(), false );
		fFirstPass = false;
	}
	
	// Verify if a product is selected.
	if( 0== lSelectedRow )
		m_pSelected = 0;

	return true;
}

void CSelProdPageCtrl::_AddArticleList( CDB_Actuator *pclActuator, CAccessoryList *pclAccessoryList, int iQuantity, bool bSelectionBySet )
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

void CSelProdPageCtrl::_AddArticleList( CDB_Actuator *pclActuator, std::vector<CDS_Actuator::AccessoryItem> *pvecAccessoryList, int iQuantity, bool bSelectionBySet )
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

	AddAccessoriesInArticleContainer( pvecAccessoryList, pclArticleContainer, iQuantity, bSelectionBySet );

	CSelProdPageBase::AddArticleInGroupList( pclArticleGroup );

	// Remark: pay attention that 'CPage::AddArticleInGroupList' will increment quantity if article already exist or
	//         will create a new 'CArticleGroup' and will copy to it data passed as argument. This is the reason
	//         why you need to delete 'pclArticleGroup' after the call.
	delete pclArticleGroup;
}

void CSelProdPageCtrl::_AddArticleList( CDB_Actuator* pclActuator, CArray<IDPTR>* parAccessory, int iQuantity, bool fSelectionBySet )
{
	if( NULL == pclActuator || NULL == parAccessory || 0 == iQuantity )
		return;

	std::vector<CDS_Actuator::AccessoryItem> vecAccessoryList;
	for( int iLoop = 0; iLoop < parAccessory->GetCount(); iLoop++ )
	{
		CDS_Actuator::AccessoryItem rAccessoryItem;
		rAccessoryItem.IDPtr = parAccessory->GetAt( iLoop );
		rAccessoryItem.fByPair = false;
	}
	
	_AddArticleList( pclActuator, &vecAccessoryList, iQuantity, fSelectionBySet );
}

void CSelProdPageCtrl::_AddArticleList( CDS_SSelCtrl* pclSSelCtrl )
{
	if( NULL == pclSSelCtrl )
		return;

	bool fAddActuator = true;

	// Add article number for control valve.
	CDB_ControlValve* pclControlValve = pclSSelCtrl->GetProductAs<CDB_ControlValve>();
	
	if( pclControlValve != NULL )
	{
		CArticleGroup *pclArticleGroup = new CArticleGroup();

		if( NULL == pclArticleGroup )
		{
			return;
		}

		CArticleItem* pclArticleItem = new CArticleItem();

		if( NULL == pclArticleItem )
		{
			delete pclArticleGroup;
			return;
		}

		CString str;
		
		// If user has done a selection by set and if there is a CV-Actuator set...
		if( true == pclSSelCtrl->IsSelectedAsAPackage( true ) )
		{
			CDB_Set *pCvActSet = dynamic_cast<CDB_Set*>( pclSSelCtrl->GetCvActrSetIDPtr().MP );
			
			pclArticleItem->SetID( pCvActSet->GetIDPtr().ID );
	
			CString strArticleNumber = pCvActSet->GetReference();
			CString strLocArtNumber = LocArtNumTab.GetLocalArticleNumber( strArticleNumber );
			pclArticleItem->CheckThingAvailability( pCvActSet, strArticleNumber, strLocArtNumber );
			
			if( true == strLocArtNumber.IsEmpty() )
			{
				strLocArtNumber = _T("-");
			}

			pclArticleItem->SetArticle( strArticleNumber );
			pclArticleItem->SetLocArtNum( strLocArtNumber );
			
			str = pCvActSet->GetName() + CString( _T(" ") ) + TASApp.LoadLocalizedString( IDS_SSHEETSELPROD_SET );
			pclArticleItem->SetDescription( str );
			pclArticleItem->SetQuantity( pclSSelCtrl->GetpSelectedInfos()->GetQuantity() );

			pclArticleItem->SetPrice( TASApp.GetpTADB()->GetPrice( pCvActSet->GetArtNum( true ) ) );
			pclArticleItem->SetIsAvailable( pCvActSet->IsAvailable() );
			pclArticleItem->SetIsDeleted( pCvActSet->IsDeleted() );
			
			// Add control valve accessories.
			CArticleContainer *pclArticleContainer = pclArticleGroup->AddArticle( pclArticleItem );
			
			// Remark: 'AddArticle' will call new CArticleContainer( pclArticleItem ) that copies content of 'pclArticleItem' and not saves the pointer.
			delete pclArticleItem;
			
			AddAccessoriesInArticleContainer( pclSSelCtrl->GetCvAccessoryList(), pclArticleContainer, pclSSelCtrl->GetpSelectedInfos()->GetQuantity(), true );
			
			// Add actuator accessories except for those that are already included into the set.
			CAccessoryList clActAccessoriesToAdd;
			CDB_RuledTable* pAccTab = dynamic_cast<CDB_RuledTable *>( pCvActSet->GetAccGroupIDPtr().MP );
			CAccessoryList* pclActuatorAccessoryList = pclSSelCtrl->GetActuatorAccessoryList();
			
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

			AddAccessoriesInArticleContainer( &clActAccessoriesToAdd, pclArticleContainer, pclSSelCtrl->GetpSelectedInfos()->GetQuantity(), true );

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
			pclArticleItem->SetID( pclControlValve->GetIDPtr().ID );
			pclArticleItem->SetArticle( pclControlValve->GetBodyArtNum() );
			
			// Check if article is available and not deleted.
			CString strArticleNumber = pclControlValve->GetBodyArtNum();
			CString strLocArtNum = LocArtNumTab.GetLocalArticleNumber( strArticleNumber );
			pclArticleItem->CheckThingAvailability( pclControlValve, strArticleNumber, strLocArtNum );

			if( true == strLocArtNum.IsEmpty() )
			{
				strLocArtNum = _T("-");
			}

			pclArticleItem->SetArticle( strArticleNumber );
			pclArticleItem->SetLocArtNum( strLocArtNum );
			
			pclArticleItem->SetQuantity( pclSSelCtrl->GetpSelectedInfos()->GetQuantity() );
			pclArticleItem->SetDescription( pclControlValve->GetName() + CString( _T("; ") ) );
			
			// Add version.
			pclArticleItem->AddDescription( ( (CDB_StringID *)pclControlValve->GetVersionIDPtr().MP )->GetString() + CString( _T("; ") ) );

			// Add connection.
			str = ( (CDB_StringID *)pclControlValve->GetConnectIDPtr().MP )->GetString();
			str = str.Right( str.GetLength() - str.Find('/') - 1 );
			pclArticleItem->AddDescription( str + CString( _T("; ") ) );

			// Add PN.
			pclArticleItem->AddDescription( TASApp.LoadLocalizedString( IDS_PN ) + _T(" ") + pclControlValve->GetPN().c_str() );
			
			// Add Kvs.
			if( false == pclControlValve->IsaPICV() && -1.0 != pclControlValve->GetKvs() )
			{
				str = CString( _T("; ") ) + GetKvCVString() + CString( _T(" = ") ) + WriteCUDouble( _C_KVCVCOEFF, pclControlValve->GetKvs() );
				pclArticleItem->AddDescription( str );
			}

			pclArticleItem->SetPrice( TASApp.GetpTADB()->GetPrice( pclControlValve->GetArtNum( true ) ) );
			pclArticleItem->SetIsAvailable( pclControlValve->IsAvailable() );
			pclArticleItem->SetIsDeleted( pclControlValve->IsDeleted() );

			// Add all Article Accessories.
			CArticleContainer *pclArticleContainer = pclArticleGroup->AddArticle( pclArticleItem );

			// Remark: 'AddArticle' will call new CArticleContainer( pclArticleItem ) that copies content of 'pclArticleItem' and not saves the pointer.
			delete pclArticleItem;

			AddAccessoriesInArticleContainer( pclSSelCtrl->GetCvAccessoryList(), pclArticleContainer, pclSSelCtrl->GetpSelectedInfos()->GetQuantity(), false );

			CSelProdPageBase::AddArticleInGroupList( pclArticleGroup );

			// Remark: pay attention that 'CPage::AddArticleInGroupList' will increment quantity if article already exist or
			//         will create a new 'CArticleGroup' and will copy to it data passed as argument. This is the reason
			//         why you need to delete 'pclArticlegroup' after the call.
			delete pclArticleGroup;

			// Add connection component if needed.
			if( true == pclControlValve->IsConnTabUsed() )
			{
				// Inlet.
				CSelProdPageBase::AddCompArtList( pclControlValve, pclSSelCtrl->GetpSelectedInfos()->GetQuantity(), true );
				// Outlet.
				CSelProdPageBase::AddCompArtList( pclControlValve, pclSSelCtrl->GetpSelectedInfos()->GetQuantity(), false );
			}
		}
	}

	if( true == fAddActuator )
	{
		CDB_Actuator *pclActuator = dynamic_cast<CDB_Actuator*>( pclSSelCtrl->GetActrIDPtr().MP );

		if( pclActuator != NULL )
		{
			_AddArticleList( pclActuator, pclSSelCtrl->GetActuatorAccessoryList(), pclSSelCtrl->GetpSelectedInfos()->GetQuantity(), pclSSelCtrl->IsSelectedAsAPackage( true ) );
		}
	}
}

void CSelProdPageCtrl::_AddArticleList( CDS_HydroMod::CCv* pclHMCV )
{
	if( NULL == pclHMCV )
		return;

	CDB_ControlValve *pclControlValve = dynamic_cast<CDB_ControlValve *>( pclHMCV->GetCvIDPtr().MP ); 
	
	if( NULL != pclControlValve )
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
		
		// If selection has been done by a set...
		if( true == pclControlValve->IsPartOfaSet() && eBool3::eb3True == pclHMCV->GetActrSelectedAsaPackage() )
		{
			CDB_Set *pCvActSet = NULL;
			CDB_Actuator* pclActuator = dynamic_cast<CDB_Actuator *>( pclHMCV->GetActrIDPtr().MP );

			if( NULL != pclActuator )
			{
				// Fill CvActSet if needed.
				CTableSet *pTabSet = pclControlValve->GetTableSet();			ASSERT( NULL != pTabSet );
				// HYS-1388 : Just find the Set, compatible or not.
				pCvActSet = pTabSet->FindSet( pclControlValve->GetIDPtr().ID, pclActuator->GetIDPtr().ID );
			}

			if( NULL == pCvActSet )
			{
				ASSERT( 0 );
				return;
			}
			
			pclArticleItem->SetID( pCvActSet->GetIDPtr().ID );
			
			CString strArticleNumber = pCvActSet->GetReference();
			CString strLocArtNumber = LocArtNumTab.GetLocalArticleNumber( strArticleNumber );
			pclArticleItem->CheckThingAvailability( pCvActSet, strArticleNumber, strLocArtNumber );
			
			if( true == strLocArtNumber.IsEmpty() )
			{
				strLocArtNumber = _T("-");
			}

			pclArticleItem->SetArticle( strArticleNumber );
			pclArticleItem->SetLocArtNum( strLocArtNumber );
			str = pCvActSet->GetName() + CString( _T(" ") ) + TASApp.LoadLocalizedString( IDS_SSHEETSELPROD_SET );
			pclArticleItem->SetDescription( str );
			pclArticleItem->SetQuantity( 1 );
			
			pclArticleItem->SetIsAvailable( pCvActSet->IsAvailable() );
			pclArticleItem->SetIsDeleted( pCvActSet->IsDeleted() );

			CArticleContainer *pclArticleContainer = pclArticleGroup->AddArticle( pclArticleItem );

			// Remark: 'AddArticle' will call new CArticleContainer( pclArticleItem ) that copies content of 'pclArticleItem' and not saves the pointer.
			delete pclArticleItem;

			// Add control valve and actuator accessories belonging to the set.
			CDB_RuledTable *pclRuledTable = dynamic_cast<CDB_RuledTable*>( pCvActSet->GetAccGroupIDPtr().MP );
			if( NULL != pclRuledTable && pclHMCV->GetCVActrAccSetCount() > 0 )
			{
				CAccessoryList clAccessoryList;
				for( int iLoop = 0; iLoop < pclHMCV->GetCVActrAccSetCount(); iLoop++ )
				{
					CDB_Product *pclAccessory = (CDB_Product*)( pclHMCV->GetCVActrAccSetIDPtr( iLoop ).MP );

					if( NULL == pclAccessory || false == pclAccessory->IsAnAccessory() )
					{
						continue;
					}

					clAccessoryList.Add( pclAccessory->GetIDPtr(), CAccessoryList::_AT_SetAccessory, pclRuledTable );
				}
				AddAccessoriesInArticleContainer( &clAccessoryList, pclArticleContainer, 1, true );
			}

			// Add control valve adapters.
			if( NULL != pclHMCV->GetActrAdaptIDPtr().MP )
			{
				CAccessoryList clAccessoryList;
				clAccessoryList.Add( pclHMCV->GetActrAdaptIDPtr(), CAccessoryList::_AT_Adapter );
				AddAccessoriesInArticleContainer( &clAccessoryList, pclArticleContainer, 1, true );
			}

			CSelProdPageBase::AddArticleInGroupList( pclArticleGroup );

			// Remark: pay attention that 'CPage::AddArticleInGroupList' will increment quantity if article already exist or
			//         will create a new 'CArticleGroup' and will copy to it data passed as argument. This is the reason
			//         why you need to delete 'pclArticleGroup' after the call.
			delete pclArticleGroup;
			
			// All has been already done for actuator and its accessories.
		}
		else
		{
			pclArticleItem->SetID( pclControlValve->GetIDPtr().ID );
			pclArticleItem->SetArticle( pclControlValve->GetBodyArtNum() );

			// Check if article is available and not deleted.
			CString strArticleNumber = pclControlValve->GetBodyArtNum();
			CString strLocArtNum = LocArtNumTab.GetLocalArticleNumber( strArticleNumber );
			pclArticleItem->CheckThingAvailability( pclControlValve, strArticleNumber, strLocArtNum );
			
			if( true == strLocArtNum.IsEmpty() )
			{
				strLocArtNum = _T("-");
			}

			pclArticleItem->SetArticle( strArticleNumber );
			pclArticleItem->SetLocArtNum( strLocArtNum );

			pclArticleItem->SetQuantity( 1 );
			pclArticleItem->SetDescription( pclControlValve->GetName() + CString( _T("; ") ) );
			
			if( false == pclControlValve->IsaPICV() && -1.0 != pclControlValve->GetKvs() )
			{
				str += GetKvCVString() + CString( _T(" = ") ) + WriteCUDouble( _C_KVCVCOEFF, pclControlValve->GetKvs() );
				pclArticleItem->AddDescription( str + CString( _T("; ") ) );
			}
			
			pclArticleItem->AddDescription( ( (CDB_StringID *)pclControlValve->GetVersionIDPtr().MP )->GetString() );

			str = ( (CDB_StringID *)pclControlValve->GetConnectIDPtr().MP )->GetString();
			str = str.Right( str.GetLength() - str.Find('/') - 1 );
			pclArticleItem->AddDescription( CString( _T("; ") ) + str );

			pclArticleItem->SetPrice( TASApp.GetpTADB()->GetPrice( pclControlValve->GetArtNum( true ) ) );
			pclArticleItem->SetIsAvailable( pclControlValve->IsAvailable() );
			pclArticleItem->SetIsDeleted( pclControlValve->IsDeleted() );

			CArticleContainer *pclArticleContainer = pclArticleGroup->AddArticle( pclArticleItem );

			// Remark: 'AddArticle' will call new CArticleContainer( pclArticleItem ) that copies content of 'pclArticleItem' and not saves the pointer.
			delete pclArticleItem;

			// Add all article accessories.
			CDB_RuledTable* pclRuledTable = dynamic_cast<CDB_RuledTable*>( pclControlValve->GetAccessoriesGroupIDPtr().MP );
			if( NULL != pclRuledTable && pclHMCV->GetCVAccCount() > 0 )
			{
				CAccessoryList clAccessoryList;
				for( int iLoop = 0; iLoop < pclHMCV->GetCVAccCount(); iLoop++ )
				{
					CDB_Product *pclAccessory = (CDB_Product*)( pclHMCV->GetCVAccIDPtr( iLoop ).MP );

					if( NULL == pclAccessory || false == pclAccessory->IsAnAccessory() )
					{
						continue;
					}

					clAccessoryList.Add( pclAccessory->GetIDPtr(), CAccessoryList::_AT_Accessory, pclRuledTable );
				}
				AddAccessoriesInArticleContainer( &clAccessoryList, pclArticleContainer, 1, false );
			}

			// Add control valve adapters.
			if( NULL != pclHMCV->GetActrAdaptIDPtr().MP )
			{
				CAccessoryList clAccessoryList;
				clAccessoryList.Add( pclHMCV->GetActrAdaptIDPtr(), CAccessoryList::_AT_Adapter );
				AddAccessoriesInArticleContainer( &clAccessoryList, pclArticleContainer, 1, false );
			}

			CSelProdPageBase::AddArticleInGroupList( pclArticleGroup );

			// Remark: pay attention that 'CPage::AddArticleInGroupList' will increment quantity if article already exist or
			//         will create a new 'CArticleGroup' and will copy to it data passed as argument. This is the reason
			//         why you need to delete 'pclArticleGroup' after the call.
			delete pclArticleGroup;
			
			// Add connection component if needed.
			if( true == pclControlValve->IsConnTabUsed() )
			{
				// Inlet
				CSelProdPageBase::AddCompArtList( pclControlValve, 1, true );
				// Outlet
				CSelProdPageBase::AddCompArtList( pclControlValve, 1, false );
			}

			CDB_Actuator *pclActuator = dynamic_cast<CDB_Actuator*>( pclHMCV->GetActrIDPtr().MP );

			if( pclActuator != NULL )
			{
				_AddArticleList( pclActuator, pclHMCV->GetpActrAccIDPtrArray(), 1, false );
			}
		}
	}
}

int CSelProdPageCtrl::GetFUSIONAccessories( CDB_ControlValve* pclControlValve, std::set< CDB_Product* >* pclAccessoryList )
{
	if( NULL == pclControlValve || NULL == pclAccessoryList )
		return 0;

	std::set<CDB_Set *> setValveActuatorSet;
	CTableSet* pclValveActuatorTableSet = pclControlValve->GetTableSet();
	
	if( NULL == pclValveActuatorTableSet )
		return 0;

	int iCDBSetCount = pclValveActuatorTableSet->FindCompatibleSet( &setValveActuatorSet, pclControlValve->GetIDPtr().ID, _T("") );
	for( std::set<CDB_Set *>::iterator iterCDBSet = setValveActuatorSet.begin(); iterCDBSet != setValveActuatorSet.end(); iterCDBSet++ )
	{
		CDB_Set* pValveActuatorSet = *iterCDBSet;
		IDPTR AdapterIDPtr = pValveActuatorSet->GetAccGroupIDPtr();
		if( NULL == AdapterIDPtr.MP )
		{
			// If no adapter in 'CDB_Set', we must choose adapters that are common with valve and actuators.
			CDB_ControlValve* pclCVInSet = (CDB_ControlValve*)( pValveActuatorSet->GetFirstIDPtr().MP );
			if( NULL == pclCVInSet )
				continue;
			CDB_RuledTable* pclCVAdapterGroup = (CDB_RuledTable*)( pclCVInSet->GetAdapterGroupIDPtr().MP );
			if( NULL == pclCVAdapterGroup )
				continue;

			CDB_ElectroActuator* pclActuatorInSet = (CDB_ElectroActuator*)( pValveActuatorSet->GetSecondIDPtr().MP );
			if( NULL == pclActuatorInSet )
				continue;
			CDB_RuledTable* pclActuatorAdapterTable = (CDB_RuledTable*)( pclActuatorInSet->GetActAdapterGroupIDPtr().MP );
			if( NULL == pclActuatorAdapterTable )
				continue;

			// Run all adapters linked to the valve.
			for( IDPTR ValveAdapterIDPtr = pclCVAdapterGroup->GetFirst(); ValveAdapterIDPtr.MP != NULL; ValveAdapterIDPtr = pclCVAdapterGroup->GetNext( ValveAdapterIDPtr.MP ) )
			{
				// Run all adapters linked to the current actuator.
				bool fStop = false;
				for( IDPTR ActuatorAdapterIDPtr = pclActuatorAdapterTable->GetFirst(); ActuatorAdapterIDPtr.MP != NULL && false == fStop; ActuatorAdapterIDPtr = pclActuatorAdapterTable->GetNext( ActuatorAdapterIDPtr.MP ) )
				{
					if( 0 == IDcmp( ValveAdapterIDPtr.ID, ActuatorAdapterIDPtr.ID ) )
					{
						CDB_Product* pclAdapter = dynamic_cast<CDB_Product *>( ValveAdapterIDPtr.MP );
						if( NULL == pclAdapter || false == pclAdapter->IsAnAccessory() )
							continue;
						if( false == pclAdapter->IsSelectable(true) )
							continue;
						if( 0 == pclAccessoryList->count( pclAdapter ) )
							pclAccessoryList->insert( pclAdapter );
						fStop = true;
					}
				}
			}
		}
		else
		{
			// Take this one.
			CDB_Product *pclAdapter = dynamic_cast<CDB_Product *>( AdapterIDPtr.MP );
			if( NULL == pclAdapter || false == pclAdapter->IsAnAccessory() )
				continue;
			if( false == pclAdapter->IsSelectable(true) )
				continue;
			if( 0 == pclAccessoryList->count( pclAdapter ) )
				pclAccessoryList->insert( pclAdapter );
		}
	}
	return pclAccessoryList->size();
}
