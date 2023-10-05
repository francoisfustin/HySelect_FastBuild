#include "stdafx.h"
#include "TASelect.h"
#include "MainFrm.h"
#include "Global.h"
#include "HydroMod.h"
#include "DrawSet.h"
#include "SelectPM.h"
#include "DlgLeftTabBase.h"
#include "DlgLeftTabSelP.h"
#include "RViewDescription.h"
#include "SelProdArticle.h"
#include "SelProdPageBase.h"
#include "SelProdPagePressMaint.h"

CSelProdPagePressMaint::CSelProdPagePressMaint( CArticleGroupList *pclArticleGroupList )
	: CSelProdPageBase( CDB_PageSetup::enCheck::PRESSMAINT, pclArticleGroupList, false )
{
	m_pSelected = NULL;
	m_vecComputedPressMaint.clear();
	m_vecDirSelPressMaint.clear();
	m_clCPMIOParam.Clear();
	m_clCPMPParam.Clear();
	m_clCPMGParam.Clear();
	m_clDSPMParam.Clear();
	m_clPVISParam.Clear();
	m_uiSDComputedIOCount = 0;
	m_uiSDComputedPCount = 0;
	m_uiSDGraphsCount = 0;
}

void CSelProdPagePressMaint::Reset( void )
{
	CSelProdPageBase::Reset();
	m_vecComputedPressMaint.clear();
	m_vecDirSelPressMaint.clear();
	m_clCPMIOParam.Clear();
	m_clCPMPParam.Clear();
	m_clCPMGParam.Clear();
	m_clDSPMParam.Clear();
	m_clPVISParam.Clear();
	m_uiSDComputedIOCount = 0;
	m_uiSDComputedPCount = 0;
	m_uiSDGraphsCount = 0;

	if( m_vecImageList.size() > 0 )
	{
		for( vecHandleIter iter = m_vecImageList.begin(); iter != m_vecImageList.end(); iter++ )
		{
			if( INVALID_HANDLE_VALUE != *iter )
			{
				DeleteObject( *iter );
			}
		}

		m_vecImageList.clear();
	}

	if( m_vecImageSelectedList.size() > 0 )
	{
		for( vecHandleIter iter = m_vecImageSelectedList.begin(); iter != m_vecImageSelectedList.end(); iter++ )
		{
			if( INVALID_HANDLE_VALUE != *iter )
			{
				DeleteObject( *iter );
			}
		}

		m_vecImageSelectedList.clear();
	}
}

bool CSelProdPagePressMaint::PreInit( HMvector &vecHMList )
{
	// Something for individual or direct selection ?
	m_vecComputedPressMaint.clear();
	m_vecDirSelPressMaint.clear();
	bool fPMExist = false;
	m_pclSelectionTable = NULL;
	CTable *pclTable = ( CTable * )( TASApp.GetpTADS()->Get( _T("PRESSMAINT_TAB") ).MP );
	if ( NULL == pclTable )
	{
		ASSERT( 0 );
		return false;
	}

	if( NULL != pclTable )
	{
		for( IDPTR IDPtr = pclTable->GetFirst( CLASS( CDS_SSelPMaint ) ); '\0' != *IDPtr.ID; IDPtr = pclTable->GetNext() )
		{
			CDS_SSelPMaint *pSel = dynamic_cast<CDS_SSelPMaint *>( IDPtr.MP );

			if( NULL == pSel )
			{
				continue;
			}

			if( NULL == pSel->GetIDPtr().MP )
			{
				continue;
			}

			fPMExist = true;
			m_pclSelectionTable = pclTable;

			if( false == pSel->IsFromDirSel() )
			{
				m_vecComputedPressMaint.push_back( pSel );
			}
			else
			{
				m_vecDirSelPressMaint.push_back( pSel );
			}
		}
	}

	// Something for hydronic calculation?
	bool fPMHMExist = ( vecHMList.size() > 0 );

	if( false == fPMExist && false == fPMHMExist )
	{
		return false;
	}

	m_vecHMList = vecHMList;
	SortTable();
	return true;
}

bool CSelProdPagePressMaint::Init( bool fResetOrder, bool fPrint )
{
	bool fPMExist = ( NULL != m_pclSelectionTable ) ? true : false;
	bool fPMHMExist = ( m_vecHMList.size() > 0 ) ? true : false;

	if( false == fPMExist && false == fPMHMExist )
	{
		return false;
	}

	m_uiSDComputedIOCount = 0;
	m_uiSDComputedPCount = 0;
	m_uiSDGraphsCount = 0;
	m_bForPrint = fPrint;

	// First of all: create and fill the page title.
	_FillPMTitle();

	if( true == fPMExist )
	{
		// Number of objects.
		int iPMCount = m_pclSelectionTable->GetItemCount( CLASS( CDS_SSelPMaint ) );

		if( 0 == iPMCount )
		{
			return false;
		}

		CDS_SSelPMaint **paSSelPM = new CDS_SSelPMaint*[iPMCount];

		if( NULL == paSSelPM )
		{
			return false;
		}

		// Fill 'paSSelPM' array with pointer on each 'CDS_SSelPMaint' object of the 'PRESSMAINT_TAB' table.
		int iMaxIndex = 0;
		int i = 0;

		for( IDPTR IDPtr = m_pclSelectionTable->GetFirst( CLASS( CDS_SSelPMaint ) ); '\0' != *IDPtr.ID; IDPtr = m_pclSelectionTable->GetNext() )
		{
			CDS_SSelPMaint *pclSSelPM = dynamic_cast<CDS_SSelPMaint *>( IDPtr.MP );

			if( NULL == pclSSelPM )
			{
				continue;
			}

			if( NULL == pclSSelPM->GetIDPtr().MP )
			{
				continue;
			}

			paSSelPM[i] = pclSSelPM;

			if( iMaxIndex < paSSelPM[i]->GetpSelectedInfos()->GetRowIndex() )
			{
				iMaxIndex = paSSelPM[i]->GetpSelectedInfos()->GetRowIndex();
			}

			++i;
		}

		// Sort 'paSSelPM' array.
		// Remark: 'fResetOrder' is set to 'true' only when the sorting combos are empty (No user choice) and the user clicks on the 'Apply sorting keys'.
		if( true == fResetOrder )
		{
			// Reset row index.
			for( int i = 0; i < iPMCount; i++ )
			{
				if( 0 == paSSelPM[i]->GetpSelectedInfos()->GetRowIndex() )
				{
					paSSelPM[i]->GetpSelectedInfos()->SetRowIndex( ++iMaxIndex );
				}
			}
		}
		else if( PageField_enum::epfNONE != m_pTADSPageSetup->GetKey( 0 ) || PageField_enum::epfNONE != m_pTADSPageSetup->GetKey( 1 )
				|| PageField_enum::epfNONE != m_pTADSPageSetup->GetKey( 2 ) )
		{
			if( NULL != pDlgLeftTabSelP )
			{
				pDlgLeftTabSelP->SortTable( (CData**)paSSelPM, iPMCount - 1 );
			}

			// Reset row index.
			for( int i = 0; i < iPMCount; i++ )
			{
				paSSelPM[i]->GetpSelectedInfos()->SetRowIndex( i + 1 );
			}
		}

		// Create a remark index.
		if( NULL != pDlgLeftTabSelP )
		{
			pDlgLeftTabSelP->SetRemarkIndex( CLASS( CDS_SSelPMaint ), (CData**)paSSelPM, iPMCount );
		}

		enum ePMSwitch
		{
			ComputedPM,
			PlenoVentoIndSel,
			DirSelPM,
			LastPMSwitch
		};

		for( int iLoopGroup = 0; iLoopGroup < ePMSwitch::LastPMSwitch; iLoopGroup++ )
		{
			std::vector<CDS_SSelPMaint *> *pvecSSelPressMaint = ( ePMSwitch::ComputedPM == iLoopGroup ) ? &m_vecComputedPressMaint : &m_vecDirSelPressMaint;

			CRank rkl;

			for( int i = 0; i < iPMCount; i++ )
			{
				CDS_SSelPMaint *pclSelPressMaint = dynamic_cast<CDS_SSelPMaint *>( paSSelPM[i] );

				// Verify that object well belongs to the current 'iLoopGroup'.
				bool fByPass = false;

				switch( iLoopGroup )
				{
					case ComputedPM:
						if( true == pclSelPressMaint->IsFromDirSel() )
						{
							fByPass = true;
						}
						else if( CDS_SSelPMaint::NoPressurization == pclSelPressMaint->GetSelectionType() )
						{
							fByPass = true;
						}

						break;

					case PlenoVentoIndSel:
						if( true == pclSelPressMaint->IsFromDirSel() )
						{
							fByPass = true;
						}
						else if( CDS_SSelPMaint::NoPressurization != pclSelPressMaint->GetSelectionType() )
						{
							fByPass = true;
						}

						break;

					case DirSelPM:
						if( false == pclSelPressMaint->IsFromDirSel() )
						{
							fByPass = true;
						}

						break;
				}

				if( true == fByPass )
				{
					continue;
				}

				// Verify that the object is well defined.
				fByPass = false;

				switch( iLoopGroup )
				{
					case ComputedPM:
						if( CDS_SSelPMaint::NoPressurization != pclSelPressMaint->GetSelectionType() )
						{
							if( NULL == dynamic_cast<CDB_Product *>( pclSelPressMaint->GetVesselIDPtr().MP ) )
							{
								ASSERT( 0 );
								fByPass = true;
							}
						}

						break;

					case PlenoVentoIndSel:
						if( NULL == dynamic_cast<CDB_Product *>( pclSelPressMaint->GetTecBoxPlenoIDPtr().MP ) &&
							NULL == dynamic_cast<CDB_Product *>( pclSelPressMaint->GetTecBoxVentoIDPtr().MP ) )
						{
							ASSERT( 0 );
							fByPass = true;
						}

						break;

					case DirSelPM:
						if( NULL == pclSelPressMaint->GetSelectedProduct() )
						{
							ASSERT( 0 );
							fByPass = true;
						}

						break;
				}

				if( true == fByPass )
				{
					continue;
				}

				rkl.Add( _T( "" ), paSSelPM[i]->GetpSelectedInfos()->GetRowIndex(), (LPARAM)paSSelPM[i] );
			}

			if( 0 == rkl.GetCount() )
			{
				continue;
			}

			CString str;
			LPARAM lparam;
			bool fFirstPass = true;

			for( BOOL fContinue = rkl.GetFirst( str, lparam ); TRUE == fContinue; fContinue = rkl.GetNext( str, lparam ) )
			{
				CData *pclData = ( CData * )lparam;
				CDS_SSelPMaint *pclSSelPressMaint = dynamic_cast<CDS_SSelPMaint *>( pclData );

				if( NULL == pclSSelPressMaint )
				{
					continue;
				}

				switch( iLoopGroup )
				{
					case ePMSwitch::ComputedPM:
						m_clCPMIOParam.Clear();
						m_clCPMPParam.Clear();
						m_clCPMGParam.Clear();
						_FillComputedInputOutput( pclSSelPressMaint );
						_FillComputedProduct( pclSSelPressMaint );
						_FillCurves( pclSSelPressMaint );

						// Save product.
						if( (LPARAM)0 != m_clCPMIOParam.m_lpSelectionContainer || (LPARAM)0 != m_clCPMPParam.m_lpSelectionContainer )
						{
							// We save the selectable row range and product in the 'SD_CPMInputOutput' sheet. If user clicks on this sheet, we must be able to
							// select also the rows in the 'SD_CPMProduct' and 'SD_Graphs' sheets. This is why we link the two 'CProductParam' together.
							m_clCPMIOParam.m_vecLinkedProductParam.push_back( m_clCPMPParam );
							m_clCPMIOParam.m_vecLinkedProductParam.push_back( m_clCPMGParam );
							SaveProduct( m_clCPMIOParam );

							m_clCPMIOParam.m_vecLinkedProductParam.clear();
							m_clCPMPParam.m_vecLinkedProductParam.push_back( m_clCPMIOParam );
							m_clCPMPParam.m_vecLinkedProductParam.push_back( m_clCPMGParam );
							SaveProduct( m_clCPMPParam );

							m_clCPMPParam.m_vecLinkedProductParam.clear();
							m_clCPMGParam.m_vecLinkedProductParam.push_back( m_clCPMIOParam );
							m_clCPMGParam.m_vecLinkedProductParam.push_back( m_clCPMPParam );
							SaveProduct( m_clCPMGParam );
						}

						break;

					case ePMSwitch::PlenoVentoIndSel:
						m_clPVISParam.Clear();

						if( true == fFirstPass )
						{
							_InitPlenoVentoIndSel();
						}

						_FillPlenoVentoIndSel( pclSSelPressMaint, fFirstPass );
						SaveProduct( m_clPVISParam );
						
						break;

					case ePMSwitch::DirSelPM:
						m_clDSPMParam.Clear();

						if( true == fFirstPass )
						{
							_InitDirectSel();
						}

						_FillDirectSel( pclSSelPressMaint, fFirstPass );
						SaveProduct( m_clDSPMParam );
						break;
				}

				fFirstPass = false;
			}
		}

		_FillRemarks();
		delete[] paSSelPM;
	}

	if( true == fPMHMExist )
	{
		// TODO: Not yet available.
	}

	// Move sheet to correct position.
	SetSheetSize();
	Invalidate();
	UpdateWindow();

	return true;
}

bool CSelProdPagePressMaint::HasSomethingToDisplay( void )
{
	bool fPMExist = ( NULL != m_pclSelectionTable ) ? true : false;
	bool fPMHMExist = ( m_vecHMList.size() > 0 ) ? true : false;

	if( false == fPMExist && false == fPMHMExist )
	{
		return false;
	}

	return true;
}

long CSelProdPagePressMaint::FillAccessory( CSheetDescription *pclSheetDescription, long lRow, CDB_Product *pclAccessory, bool fWithArticleNumber, int iQuantity, CProductParam *pclProductParam )
{
	if( NULL == pclSheetDescription || NULL == pclSheetDescription->GetSSheetPointer() )
	{
		return lRow;
	}
	
	CSSheet *pclSheet = pclSheetDescription->GetSSheetPointer();
	VERIFY( pclAccessory );

	if( NULL == pclAccessory || false == pclAccessory->IsAnAccessory() )
	{
		return pclSheet->GetMaxRows();
	}

	pclSheet->SetTextPattern( CSSheet::_SSTextPattern::UserStaticColumn );
	pclSheet->SetTextPatternProperty( CSSheet::_SSTextPatternProperty::TPP_StyleHorizontalAlign, (LPARAM)SSS_ALIGN_LEFT );
	pclSheet->SetTextPatternProperty( CSSheet::_SSTextPatternProperty::TPP_FontBold, (LPARAM)FALSE );
	pclSheet->SetTextPatternProperty( CSSheet::_SSTextPatternProperty::TPP_StyleEllipses, (LPARAM)TRUE );
	pclSheet->SetTextPatternProperty( CSSheet::_SSTextPatternProperty::TPP_StyleWordWrap, (LPARAM)TRUE );

	SetLastRow( pclSheetDescription, lRow );

	// Write name.
	pclSheet->SetStaticText( CD_CPMP_Reference1, lRow, pclAccessory->GetName() );

	// Check current height necessary to display all info.
	double dHeight, dMaxWidth, dMaxHeight;
	pclSheet->GetMaxTextCellSize( CD_CPMP_Reference1, lRow, &dMaxWidth, &dMaxHeight );

	// Write description.
	pclSheet->SetStaticText( CD_CPMP_Product, lRow, pclAccessory->GetComment() );

	pclSheet->GetMaxTextCellSize( CD_CPMP_Product, lRow, &dMaxWidth, &dHeight );
	if( dHeight > dMaxHeight )
		dMaxHeight = dHeight;

	// Set font color to red when accessory is not deleted or not available.
	if( true == pclAccessory->IsDeleted() || false == pclAccessory->IsAvailable() )
		pclSheet->SetTextPatternProperty( CSSheet::_SSTextPatternProperty::TPP_FontForeColor, (LPARAM)_RED );

	// If accessory is included into a set, mask his article number.
	if( false == fWithArticleNumber )
	{
		CString strArticleNumber = _T("-");
		if( true == pclAccessory->IsDeleted() )
			strArticleNumber += CString( _T("\r\n") ) + TASApp.LoadLocalizedString( IDS_TAPDELETED );
		else if( false == pclAccessory->IsAvailable() )
			strArticleNumber += CString( _T("\r\n") ) + TASApp.LoadLocalizedString( IDS_TAPNOTAVAILABLE );
		pclSheet->SetStaticText( CD_CPMP_ArticleNumber, lRow, strArticleNumber );
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
		CString strArticleNumber = _T("");
		// If we can show article number OR we can't show but there is no local article number...
		if( false == TASApp.IsTAArtNumberHidden() || ( true == TASApp.IsTAArtNumberHidden() && true == strLocalArticleNumber.IsEmpty() )  )
		{
			strArticleNumber = pclAccessory->GetArtNum();
		}

		// Set local article number if allowed and exist.
		CString strFullArticleNumber = strArticleNumber;
		if( true == TASApp.IsLocalArtNumberUsed() && false == strLocalArticleNumber.IsEmpty() )
		{
			strLocalArticleNumber = (CString)TASApp.GetLocalArtNumberName() + LocArtNumTab.GetLocalArticleNumber( pclAccessory->GetArtNum() );
			strLocalArticleNumber.Trim();

			if( false == strArticleNumber.IsEmpty() && false == strLocalArticleNumber.IsEmpty() )
				strFullArticleNumber = strArticleNumber + _T("\r\n") + strLocalArticleNumber;
			else
				strFullArticleNumber = ( true == strArticleNumber.IsEmpty() ) ? strLocalArticleNumber : strArticleNumber;
			strFullArticleNumber.Trim();
		}

		if( true == pclAccessory->IsDeleted() )
			strFullArticleNumber += CString( _T("\r\n") ) + TASApp.LoadLocalizedString( IDS_TAPDELETED );
		else if( false == pclAccessory->IsAvailable() )
			strFullArticleNumber += CString( _T("\r\n") ) + TASApp.LoadLocalizedString( IDS_TAPNOTAVAILABLE );
		// HYS-945: Center buffer vessel article number
		pclSheet->SetTextPatternProperty( CSSheet::_SSTextPatternProperty::TPP_StyleHorizontalAlign, (LPARAM)SSS_ALIGN_CENTER );
		pclSheet->SetStaticText( CD_CPMP_ArticleNumber, lRow, strFullArticleNumber );
		pclSheet->SetTextPatternProperty( CSSheet::_SSTextPatternProperty::TPP_StyleHorizontalAlign, (LPARAM)SSS_ALIGN_LEFT );
	}

	pclSheet->GetMaxTextCellSize( CD_CPMP_ArticleNumber, lRow, &dMaxWidth, &dHeight );
	if( dHeight > dMaxHeight )
		dMaxHeight = dHeight;

	// Adapt row height.
	pclSheet->SetRowHeight( lRow, ( dMaxHeight > m_dRowHeight ) ? dMaxHeight : m_dRowHeight );

	// Fill quantity and prices.
	_FillQtyPriceCol( pclSheetDescription, lRow, pclAccessory, iQuantity );

	pclSheet->SetTextPattern( CSSheet::_SSTextPattern::UserStaticColumn );
	pclSheet->SetTextPatternProperty( CSSheet::_SSTextPatternProperty::TPP_FontBold,(LPARAM) FALSE );

	if( NULL == pclProductParam )
		m_rProductParam.AddRange( lRow, lRow, pclAccessory );
	else
		pclProductParam->AddRange( lRow, lRow, pclAccessory );
	return pclSheet->GetMaxRows();
}

void CSelProdPagePressMaint::_FillPMTitle( void )
{
	// Call base class method to get a new 'Sheet'.
	CSheetDescription *pclSheetDescriptionTitle = CMultiSpreadBase::CreateSSheet( SD_PMPageTitle );

	if( NULL == pclSheetDescriptionTitle || NULL == pclSheetDescriptionTitle->GetSSheetPointer() )
	{
		return;
	}

	CSSheet *pclSheet = pclSheetDescriptionTitle->GetSSheetPointer();

	if( false == CSelProdPageBase::PrepareSheet( pclSheetDescriptionTitle, CD_PMPT_LastColumn, m_bForPrint, false ) )
	{
		m_ViewDescription.RemoveOneSheetDescription( SD_PMPageTitle );
		return;
	}

	pclSheet->SetMaxRows( RD_PMPT_PageName );
	pclSheet->SetRowHeight( RD_PMPT_FirstRow, m_dRowHeight * 0.5 );
	pclSheet->SetRowHeight( RD_PMPT_PageName, m_dRowHeight * 2 );

	// Initialize column width for header and footer column.
	if( false == m_bForPrint )
	{
		SetColWidth( pclSheetDescriptionTitle, CD_PMPT_Header, DefaultColumnWidth::DCW_Header );
		SetColWidth( pclSheetDescriptionTitle, CD_PMPT_Footer, DefaultColumnWidth::DCW_Footer );
	}
	else
	{
		pclSheet->ShowCol( CD_PMPT_Header, FALSE );
		pclSheet->ShowCol( CD_PMPT_Footer, FALSE );
	}

	int iHFColPixels = ( false == m_bForPrint ) ? pclSheet->ColWidthToLogUnits( 2.0 ) : 0;
	double dTitleWidth = pclSheet->LogUnitsToColWidthW( RIGHTVIEWWIDTH - 2 * iHFColPixels );
	SetColWidth( pclSheetDescriptionTitle, CD_PMPT_Title, dTitleWidth );
	SetColWidth( pclSheetDescriptionTitle, CD_PMPT_LastColumn, 0.0 );
	pclSheet->ShowCol( CD_PMPT_LastColumn, FALSE );
	CRect rect = pclSheet->GetSheetSizeInPixels( false );
	m_mapSheetInfos[pclSheetDescriptionTitle].m_dPageWidth = rect.Width();

	// Write empty static text to fix cell format
	pclSheet->FormatStaticText( -1, -1, -1, -1, _T( "" ) );

	CString str = TASApp.LoadLocalizedString( IDS_SSHEETSELPROD_TITLEPRESSMAINT );
	pclSheet->SetTextPattern( CSSheet::_SSTextPattern::TitlePage );
	pclSheet->SetStaticText( CD_PMPT_Title, RD_PMPT_PageName, str );
}

void CSelProdPagePressMaint::_FillComputedInputOutput( CDS_SSelPMaint *pclSSelPressMaint )
{
	// No computed data when Pleno and Vento are selected without pressurization.
	if( CDS_SSelPMaint::NoPressurization == pclSSelPressMaint->GetSelectionType() )
	{
		return;
	}

	// Call base class method to get a new 'Sheet'.
	CSheetDescription *pclSDInputOutput = CMultiSpreadBase::CreateSSheet( SD_CPMInputOutput + m_uiSDComputedIOCount );

	if( NULL == pclSDInputOutput || NULL == pclSDInputOutput->GetSSheetPointer() )
	{
		return;
	}

	CSSheet *pclSheet = pclSDInputOutput->GetSSheetPointer();

	if( false == CSelProdPageBase::PrepareSheet( pclSDInputOutput, CD_CPMIO_LastColumn, m_bForPrint, false ) )
	{
		m_ViewDescription.RemoveOneSheetDescription( SD_CPMInputOutput + m_uiSDComputedIOCount );
		return;
	}

	m_uiSDComputedIOCount++;

	// Initialize column width for header and footer column.
	if( false == m_bForPrint )
	{
		SetColWidth( pclSDInputOutput, CD_CPMIO_Header, DefaultColumnWidth::DCW_Header );
		SetColWidth( pclSDInputOutput, CD_CPMIO_Footer, DefaultColumnWidth::DCW_Footer );
	}
	else
	{
		pclSheet->ShowCol( CD_CPMIO_Header, FALSE );
		pclSheet->ShowCol( CD_CPMIO_Footer, FALSE );
	}

	pclSheet->SetMaxRows( RD_CPMIO_ColName );
	pclSheet->SetRowHeight( RD_CPMIO_FirstRow, m_dRowHeight * 0.5 );

	// Set the column width. To perfectly match the main title, we use proportionality.
	int iHFColPixels = ( false == m_bForPrint ) ? pclSheet->ColWidthToLogUnits( 2.0 ) : 0;
	double dAvailableWidth = pclSheet->LogUnitsToColWidthW( RIGHTVIEWWIDTH - 2 * iHFColPixels );

	double dTotalColWidth = 2 * ( DCW_CPMIO_Description1 + DCW_CPMIO_Description2 ) + 2 * DCW_CPMIO_Value;
	double dWidth = dAvailableWidth * DCW_CPMIO_Description1 / dTotalColWidth;
	double dTotalWidth = dWidth;
	SetColWidth( pclSDInputOutput, CD_CPMIO_IDescription1, dWidth );

	dWidth = dAvailableWidth * DCW_CPMIO_Description2 / dTotalColWidth;
	dTotalWidth += dWidth;
	SetColWidth( pclSDInputOutput, CD_CPMIO_IDescription2, dWidth );

	dWidth = dAvailableWidth * DCW_CPMIO_Value / dTotalColWidth;
	dTotalWidth += dWidth;
	SetColWidth( pclSDInputOutput, CD_CPMIO_IValue, dWidth );

	dWidth = dAvailableWidth * ( DCW_CPMIO_Description1 + DCW_CPMIO_Description2 ) / dTotalColWidth;
	dTotalWidth += dWidth;
	SetColWidth( pclSDInputOutput, CD_CPMIO_ODescription, dWidth );
	SetColWidth( pclSDInputOutput, CD_CPMIO_OValue, dAvailableWidth - dTotalWidth );

	pclSheet->ShowCol( CD_CPMIO_LastColumn, FALSE );

	// HYS-1571: We can optimize the column width now.
	// See Jira card to have more details about why we do that here.
	CRect rectClient;
	GetClientRect( &rectClient );
	OptimizePageSize( rectClient.Width(), rectClient.Height() );

	CRect rect = pclSheet->GetSheetSizeInPixels( false );
	m_mapSheetInfos[pclSDInputOutput].m_dPageWidth = rect.Width();

	// Set the subtitle.
	SetPageTitle( pclSDInputOutput, IDS_SSHEETSELPROD_SUBTITLEPRESSMAINTCOMPUTED, false, RD_CPMIO_GroupName, m_dRowHeight * 1.8 );

	_FillComputedInputData( pclSDInputOutput, pclSSelPressMaint );
	_FillComputedOutputData( pclSDInputOutput, pclSSelPressMaint );

	long lRow = pclSheet->GetMaxRows();
	pclSheet->SetCellBorder( CD_CPMIO_IDescription1, lRow, CD_CPMIO_OValue, lRow, true, SS_BORDERTYPE_BOTTOM, SS_BORDERSTYLE_SOLID );

	// Set the selection range.
	m_clCPMIOParam.m_pclSheetDescription = pclSDInputOutput;
	m_clCPMIOParam.m_lpSelectionContainer = (LPARAM)pclSSelPressMaint;
	m_clCPMIOParam.AddRange( RD_CPMIO_GroupName, lRow, NULL );

	// Set the scrolling range (for the scrolling, we include the title)
	m_clCPMIOParam.SetScrollRange( RD_CPMIO_GroupName, lRow );

	// Set the group as no breakable (for print).
	// Remark: include titles with the group.
	pclSheet->SetFlagRowNoBreakable( RD_CPMIO_GroupName, lRow, true );
}

void CSelProdPagePressMaint::_FillComputedInputData( CSheetDescription *pclSDInputOutput, CDS_SSelPMaint *pclSSelPressMaint )
{
	if( NULL == pclSDInputOutput || NULL == pclSDInputOutput->GetSSheetPointer() || NULL == pclSSelPressMaint )
	{
		return;
	}

	CSSheet *pclSheet = pclSDInputOutput->GetSSheetPointer();

	//////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
	// Set the 'Input data' title.
	// Remark: 'CPM' is for 'Computed Pressure Maintenance' and 'ID' is for 'Input Data'.
	CString str = TASApp.LoadLocalizedString( IDS_SSHEETSELPROD_CPM_ID_TITLE );
	pclSheet->SetTextPattern( CSSheet::_SSTextPattern::SubTitle );
	pclSheet->SetTextPatternProperty( CSSheet::_SSTextPatternProperty::TPP_StyleHorizontalAlign, (LPARAM)SSS_ALIGN_LEFT );
	pclSheet->SetTextPatternProperty( CSSheet::_SSTextPatternProperty::TPP_FontSize, (LPARAM)11 );
	pclSheet->SetTextPatternProperty( CSSheet::_SSTextPatternProperty::TPP_FontBackColor, (LPARAM)_IMI_TITLE_GROUP2 );
	pclSheet->SetTextPatternProperty( CSSheet::_SSTextPatternProperty::TPP_FontForeColor, (LPARAM)_BLACK );

	// Write title on all columns, title is not hidden when columns are hidden.
	AddStaticText( pclSDInputOutput, CD_CPMIO_IDescription1, RD_CPMIO_TitleIO, str );
	AddStaticText( pclSDInputOutput, CD_CPMIO_IDescription2, RD_CPMIO_TitleIO, str );
	AddStaticText( pclSDInputOutput, CD_CPMIO_IValue, RD_CPMIO_TitleIO, str );

	pclSheet->AddCellSpanW( CD_CPMIO_IDescription1, RD_CPMIO_TitleIO, 3, 1 );
	pclSheet->SetRowHeight( RD_CPMIO_TitleIO, m_dRowHeight * 1.7 );
	//////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////


	//////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
	// Init column header.
	pclSheet->SetTextPattern( CSSheet::_SSTextPattern::ColumnHeader );
	pclSheet->SetTextPatternProperty( CSSheet::_SSTextPatternProperty::TPP_FontBackColor, (LPARAM)_TAH_WHITE );

	AddCellSpanW( pclSDInputOutput, CD_CPMIO_IDescription1, RD_CPMIO_ColName, 2, 1 );
	AddStaticText( pclSDInputOutput, CD_CPMIO_IDescription1, RD_CPMIO_ColName, TASApp.LoadLocalizedString( IDS_SSHEETSELPROD_CPM_IDH_DESC ) );
	AddStaticText( pclSDInputOutput, CD_CPMIO_IValue, RD_CPMIO_ColName, TASApp.LoadLocalizedString( IDS_SSHEETSELPROD_CPM_IDH_VALUE ) );

	// Draw border.
	pclSheet->SetCellBorder( CD_CPMIO_IDescription1, RD_CPMIO_ColName, CD_CPMIO_IValue, RD_CPMIO_ColName, true, SS_BORDERTYPE_BOTTOM | SS_BORDERTYPE_TOP );
	//////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////


	//////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
	// Fill description columns.
	long lRow = RD_CPMIO_FirstAvailRow;
	pclSheet->SetTextPattern( CSSheet::_SSTextPattern::UserStaticColumn );
	pclSheet->SetTextPatternProperty( CSSheet::_SSTextPatternProperty::TPP_FontBold, (LPARAM)TRUE );
	pclSheet->SetTextPatternProperty( CSSheet::_SSTextPatternProperty::TPP_StyleHorizontalAlign, (LPARAM)SSS_ALIGN_LEFT );

	CPMInputUser *pclInputUser = pclSSelPressMaint->GetpInputUser();

	// Application type.
	AddCellSpanW( pclSDInputOutput, CD_CPMIO_IDescription1, lRow, 2, 1 );
	lRow = AddStaticText( pclSDInputOutput, CD_CPMIO_IDescription1, lRow, TASApp.LoadLocalizedString( IDS_SSHEETSELPROD_CPM_IDDESCR_APPTYPE ), true );
	
	// Fluid type.
	AddStaticText( pclSDInputOutput, CD_CPMIO_IDescription1, lRow, TASApp.LoadLocalizedString( IDS_SSHEETSELPROD_CPM_IDDESCR_WATERTYPE ), true );

	CString str1, str2;
	pclSSelPressMaint->GetpSelectedInfos()->GetpWCData()->BuildWaterStrings( str1, str2 );
	pclSheet->SetTextPatternProperty( CSSheet::_SSTextPatternProperty::TPP_FontBold, (LPARAM)FALSE );
	lRow = AddStaticText( pclSDInputOutput, CD_CPMIO_IDescription2, lRow, str1 );
	pclSheet->SetTextPatternProperty( CSSheet::_SSTextPatternProperty::TPP_FontBold, (LPARAM)TRUE );

	// Norm.
	AddCellSpanW( pclSDInputOutput, CD_CPMIO_IDescription1, lRow, 2, 1 );
	lRow = AddStaticText( pclSDInputOutput, CD_CPMIO_IDescription1, lRow, TASApp.LoadLocalizedString( IDS_SSHEETSELPROD_CPM_IDDESCR_NORM ), true );

	AddCellSpanW( pclSDInputOutput, CD_CPMIO_IDescription1, lRow, 2, 1 );
	lRow = AddStaticText( pclSDInputOutput, CD_CPMIO_IDescription1, lRow, TASApp.LoadLocalizedString( IDS_SSHEETSELPROD_CPM_IDDESCR_SYSTEMVOLUME ), true );

	if( ProjectType::Solar == pclInputUser->GetApplicationType() )
	{
		AddCellSpanW( pclSDInputOutput, CD_CPMIO_IDescription1, lRow, 2, 1 );
		lRow = AddStaticText( pclSDInputOutput, CD_CPMIO_IDescription1, lRow, TASApp.LoadLocalizedString( IDS_SSHEETSELPROD_CPM_IDDESCR_SOLARCOLLECTORVOLUME ), true );

		AddCellSpanW( pclSDInputOutput, CD_CPMIO_IDescription1, lRow, 2, 1 );
		lRow = AddStaticText( pclSDInputOutput, CD_CPMIO_IDescription1, lRow, TASApp.LoadLocalizedString( IDS_SSHEETSELPROD_CPM_IDDESCR_SOLARCOLLECTORMULTIPLIERFACTOR ), true );
	}

	if( true == pclInputUser->IsNorm( ProjectType::All, PressurisationNorm::PN_SWKIHE301_01 )
			&& pclInputUser->GetStorageTankVolume() > 0.0 )
	{
		AddCellSpanW( pclSDInputOutput, CD_CPMIO_IDescription1, lRow, 2, 1 );
		lRow = AddStaticText( pclSDInputOutput, CD_CPMIO_IDescription1, lRow, TASApp.LoadLocalizedString( IDS_SSHEETSELPROD_CPM_IDDESCR_STORAGETANKVOLUME ), true );

		AddCellSpanW( pclSDInputOutput, CD_CPMIO_IDescription1, lRow, 2, 1 );
		lRow = AddStaticText( pclSDInputOutput, CD_CPMIO_IDescription1, lRow, TASApp.LoadLocalizedString( IDS_SSHEETSELPROD_CPM_IDDESCR_STORAGETANKMAXTEMP ), true );
	}
	
	AddCellSpanW( pclSDInputOutput, CD_CPMIO_IDescription1, lRow, 2, 1 );
	lRow = AddStaticText( pclSDInputOutput, CD_CPMIO_IDescription1, lRow, TASApp.LoadLocalizedString( IDS_SSHEETSELPROD_CPM_IDDESCR_INSTPOWER ), true );
	
	AddCellSpanW( pclSDInputOutput, CD_CPMIO_IDescription1, lRow, 2, 1 );
	lRow = AddStaticText( pclSDInputOutput, CD_CPMIO_IDescription1, lRow, TASApp.LoadLocalizedString( IDS_SSHEETSELPROD_CPM_IDDESCR_STATHEIGHT ), true );
	
	AddCellSpanW( pclSDInputOutput, CD_CPMIO_IDescription1, lRow, 2, 1 );
	lRow = AddStaticText( pclSDInputOutput, CD_CPMIO_IDescription1, lRow, TASApp.LoadLocalizedString( IDS_SSHEETSELPROD_CPM_IDDESCR_PZ ), true );

	AddCellSpanW( pclSDInputOutput, CD_CPMIO_IDescription1, lRow, 2, 1 );
	lRow = AddStaticText( pclSDInputOutput, CD_CPMIO_IDescription1, lRow, TASApp.LoadLocalizedString( IDS_SSHEETSELPROD_CPM_IDDESCR_SAFVALRESPRESS ), true );

	AddCellSpanW( pclSDInputOutput, CD_CPMIO_IDescription1, lRow, 2, 1 );

	if( ProjectType::Heating == pclInputUser->GetApplicationType() || ProjectType::Solar == pclInputUser->GetApplicationType() )
	{
		lRow = AddStaticText( pclSDInputOutput, CD_CPMIO_IDescription1, lRow, TASApp.LoadLocalizedString( IDS_SSHEETSELPROD_CPM_IDDESCR_SAFTEMPLIM ), true );
	}
	else
	{
		lRow = AddStaticText( pclSDInputOutput, CD_CPMIO_IDescription1, lRow, TASApp.LoadLocalizedString( IDS_SSHEETSELPROD_CPM_IDDESCR_MAXTEMP ), true );
	}

	AddCellSpanW( pclSDInputOutput, CD_CPMIO_IDescription1, lRow, 2, 1 );
	lRow = AddStaticText( pclSDInputOutput, CD_CPMIO_IDescription1, lRow, TASApp.LoadLocalizedString( IDS_SSHEETSELPROD_CPM_IDDESCR_SUPPLYTEMP ), true );
	
	AddCellSpanW( pclSDInputOutput, CD_CPMIO_IDescription1, lRow, 2, 1 );
	lRow = AddStaticText( pclSDInputOutput, CD_CPMIO_IDescription1, lRow, TASApp.LoadLocalizedString( IDS_SSHEETSELPROD_CPM_IDDESCR_RETURNTEMP ), true );

	AddCellSpanW( pclSDInputOutput, CD_CPMIO_IDescription1, lRow, 2, 1 );
	lRow = AddStaticText( pclSDInputOutput, CD_CPMIO_IDescription1, lRow, TASApp.LoadLocalizedString( IDS_SSHEETSELPROD_CPM_IDDESCR_MINTEMP ), true );

	// HYS-1343 : We show the fill temperature only for Expansion vessel
	if( CDS_SSelPMaint::Statico == pclSSelPressMaint->GetSelectionType() )
	{
		AddCellSpanW( pclSDInputOutput, CD_CPMIO_IDescription1, lRow, 2, 1 );
		lRow = AddStaticText( pclSDInputOutput, CD_CPMIO_IDescription1, lRow, TASApp.LoadLocalizedString( IDS_SSHEETSELPROD_CPM_IDDESCR_FILLTEMP ), true );
	}

	AddCellSpanW( pclSDInputOutput, CD_CPMIO_IDescription1, lRow, 2, 1 );
	lRow = AddStaticText( pclSDInputOutput, CD_CPMIO_IDescription1, lRow, TASApp.LoadLocalizedString( IDS_SSHEETSELPROD_CPM_IDDESCR_PRESSON ), true );

	AddCellSpanW( pclSDInputOutput, CD_CPMIO_IDescription1, lRow, 2, 1 );
	lRow = AddStaticText( pclSDInputOutput, CD_CPMIO_IDescription1, lRow, TASApp.LoadLocalizedString( IDS_SSHEETSELPROD_CPM_IDDESCR_PUMPHEAD ), true );

	AddCellSpanW( pclSDInputOutput, CD_CPMIO_IDescription1, lRow, 2, 1 );
	lRow = AddStaticText( pclSDInputOutput, CD_CPMIO_IDescription1, lRow, TASApp.LoadLocalizedString( IDS_SSHEETSELPROD_CPM_IDDESCR_MAXWIDTH ), true );

	AddCellSpanW( pclSDInputOutput, CD_CPMIO_IDescription1, lRow, 2, 1 );
	lRow = AddStaticText( pclSDInputOutput, CD_CPMIO_IDescription1, lRow, TASApp.LoadLocalizedString( IDS_SSHEETSELPROD_CPM_IDDESCR_MAXHEIGHT ), true );
	//////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////


	//////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
	// Fill values and units.
	lRow = RD_CPMIO_FirstAvailRow;
	pclSheet->SetTextPatternProperty( CSSheet::_SSTextPatternProperty::TPP_FontBold, (LPARAM)FALSE );
	pclSheet->SetTextPatternProperty( CSSheet::_SSTextPatternProperty::TPP_StyleHorizontalAlign, (LPARAM)SSS_ALIGN_LEFT );
	lRow = AddStaticText( pclSDInputOutput, CD_CPMIO_IValue, lRow,
						  TASApp.LoadLocalizedString( m_pclTechParam->GetProductSelectionApplicationTypeIDS( pclInputUser->GetApplicationType() ) ) );

	// Bypass water type.
	lRow++;

	// Norm.
	IDPTR NormIDPtr = TASApp.GetpTADB()->Get( ( LPCTSTR )pclInputUser->GetNormID() );
	CDB_StringID *pStrNormID = dynamic_cast<CDB_StringID *>( NormIDPtr.MP );
	CString strNormID( _T( "" ) );

	if( NULL != pStrNormID )
	{
		strNormID = pStrNormID->GetString();
	}

	lRow = AddStaticText( pclSDInputOutput, CD_CPMIO_IValue, lRow, strNormID );
	lRow = AddStaticText( pclSDInputOutput, CD_CPMIO_IValue, lRow, WriteCUDouble( _U_VOLUME, pclInputUser->GetSystemVolume(), true ) );

	if( ProjectType::Solar == pclInputUser->GetApplicationType() )
	{
		lRow = AddStaticText( pclSDInputOutput, CD_CPMIO_IValue, lRow, WriteCUDouble( _U_VOLUME, pclInputUser->GetSolarCollectorVolume(), true ) );
		lRow = AddStaticText( pclSDInputOutput, CD_CPMIO_IValue, lRow, WriteDouble( pclInputUser->GetSolarCollectorMultiplierFactor(), 2, 1 ) );
	}

	if( true == pclInputUser->IsNorm( ProjectType::All, PressurisationNorm::PN_SWKIHE301_01 )
			&& pclInputUser->GetStorageTankVolume() > 0.0 )
	{
		lRow = AddStaticText( pclSDInputOutput, CD_CPMIO_IValue, lRow, WriteCUDouble( _U_VOLUME, pclInputUser->GetStorageTankVolume(), true ) );
		lRow = AddStaticText( pclSDInputOutput, CD_CPMIO_IValue, lRow, WriteCUDouble( _U_TEMPERATURE, pclInputUser->GetStorageTankMaxTemp(), true ) );
	}

	lRow = AddStaticText( pclSDInputOutput, CD_CPMIO_IValue, lRow, WriteCUDouble( _U_TH_POWER, pclInputUser->GetInstalledPower(), true ) );
	lRow = AddStaticText( pclSDInputOutput, CD_CPMIO_IValue, lRow, WriteCUDouble( _U_LENGTH, pclInputUser->GetStaticHeight(), true ) );
	
	if( BST_CHECKED == pclInputUser->GetPzChecked() )
	{
		lRow = AddStaticText( pclSDInputOutput, CD_CPMIO_IValue, lRow, WriteCUDouble( _U_PRESSURE, pclInputUser->GetPz(), true ) );
	}
	else
	{
		lRow = AddStaticText( pclSDInputOutput, CD_CPMIO_IValue, lRow, _T("-") );
	}
	
	lRow = AddStaticText( pclSDInputOutput, CD_CPMIO_IValue, lRow, WriteCUDouble( _U_PRESSURE, pclInputUser->GetSafetyValveResponsePressure(), true ) );

	if( ProjectType::Heating == pclInputUser->GetApplicationType() || ProjectType::Solar == pclInputUser->GetApplicationType() )
	{
		lRow = AddStaticText( pclSDInputOutput, CD_CPMIO_IValue, lRow, WriteCUDouble( _U_TEMPERATURE, pclInputUser->GetSafetyTempLimiter(), true ) );
	}
	else
	{
		lRow = AddStaticText( pclSDInputOutput, CD_CPMIO_IValue, lRow, WriteCUDouble( _U_TEMPERATURE, pclInputUser->GetMaxTemperature(), true ) );
	}

	lRow = AddStaticText( pclSDInputOutput, CD_CPMIO_IValue, lRow, WriteCUDouble( _U_TEMPERATURE, pclInputUser->GetSupplyTemperature(), true ) );
	lRow = AddStaticText( pclSDInputOutput, CD_CPMIO_IValue, lRow, WriteCUDouble( _U_TEMPERATURE, pclInputUser->GetReturnTemperature(), true ) );
	lRow = AddStaticText( pclSDInputOutput, CD_CPMIO_IValue, lRow, WriteCUDouble( _U_TEMPERATURE, pclInputUser->GetMinTemperature(), true ) );
	
	// HYS-1343 : We show the fill temperature only for Expansion vessel
	if( CDS_SSelPMaint::Statico == pclSSelPressMaint->GetSelectionType() )
	{
		lRow = AddStaticText( pclSDInputOutput, CD_CPMIO_IValue, lRow, WriteCUDouble( _U_TEMPERATURE, pclInputUser->GetFillTemperature(), true ) );
	}

	str = TASApp.LoadLocalizedString( m_pclTechParam->GetPressurONIDS( pclInputUser->GetPressOn() ) );
	lRow = AddStaticText( pclSDInputOutput, CD_CPMIO_IValue, lRow, str );

	if( PressurON::poPumpSuction == pclInputUser->GetPressOn() )
	{
		lRow = AddStaticText( pclSDInputOutput, CD_CPMIO_IValue, lRow, _T("-") );
	}
	else
	{
		lRow = AddStaticText( pclSDInputOutput, CD_CPMIO_IValue, lRow, WriteCUDouble( _U_PRESSURE, pclInputUser->GetPumpHead(), true ) );
	}

	lRow = AddStaticText( pclSDInputOutput, CD_CPMIO_IValue, lRow, WriteCUDouble( _U_LENGTH, pclInputUser->GetMaxWidth(), true ) );
	lRow = AddStaticText( pclSDInputOutput, CD_CPMIO_IValue, lRow, WriteCUDouble( _U_LENGTH, pclInputUser->GetMaxHeight(), true ) );
	//////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
}

void CSelProdPagePressMaint::_FillComputedOutputData( CSheetDescription *pclSDInputOutput, CDS_SSelPMaint *pclSSelPressMaint )
{
	if( NULL == pclSDInputOutput || NULL == pclSDInputOutput->GetSSheetPointer() || NULL == pclSSelPressMaint )
	{
		return;
	}

	CPMInputUser *pclPMInputUser = pclSSelPressMaint->GetpInputUser();
	
	if( NULL == pclPMInputUser || NULL == pclPMInputUser->GetpTADS() || NULL == pclPMInputUser->GetpTADS()->GetpTechParams() )
	{
		ASSERT_RETURN;
	}

	CDS_TechnicalParameter *pclTechnicalParameters = pclPMInputUser->GetpTADS()->GetpTechParams();
	CSSheet *pclSheet = pclSDInputOutput->GetSSheetPointer();

	//////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
	// Set the 'Calculated data' title.
	CString str = TASApp.LoadLocalizedString( IDS_SSHEETSELPROD_CPM_CD_TITLE );
	pclSheet->SetTextPattern( CSSheet::_SSTextPattern::SubTitle );
	pclSheet->SetTextPatternProperty( CSSheet::_SSTextPatternProperty::TPP_StyleHorizontalAlign, (LPARAM)SSS_ALIGN_LEFT );
	pclSheet->SetTextPatternProperty( CSSheet::_SSTextPatternProperty::TPP_FontSize, (LPARAM)11 );
	pclSheet->SetTextPatternProperty( CSSheet::_SSTextPatternProperty::TPP_FontBackColor, (LPARAM)_IMI_TITLE_GROUP2 );
	pclSheet->SetTextPatternProperty( CSSheet::_SSTextPatternProperty::TPP_FontForeColor, (LPARAM)_BLACK );

	// Write title on all columns, title is not hidden when columns are hidden.
	AddStaticText( pclSDInputOutput, CD_CPMIO_ODescription, RD_CPMIO_TitleIO, str );
	AddStaticText( pclSDInputOutput, CD_CPMIO_OValue, RD_CPMIO_TitleIO, str );

	pclSheet->AddCellSpanW( CD_CPMIO_ODescription, RD_CPMIO_TitleIO, 2, 1 );
	pclSheet->SetRowHeight( RD_CPMIO_TitleIO, m_dRowHeight * 1.7 );
	//////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////


	//////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
	// Init column header.
	pclSheet->SetTextPattern( CSSheet::_SSTextPattern::ColumnHeader );
	pclSheet->SetTextPatternProperty( CSSheet::_SSTextPatternProperty::TPP_FontBackColor, (LPARAM)_TAH_WHITE );

	AddStaticText( pclSDInputOutput, CD_CPMIO_ODescription, RD_CPMIO_ColName, TASApp.LoadLocalizedString( IDS_SSHEETSELPROD_CPM_IDH_DESC ) );
	AddStaticText( pclSDInputOutput, CD_CPMIO_OValue, RD_CPMIO_ColName, TASApp.LoadLocalizedString( IDS_SSHEETSELPROD_CPM_IDH_VALUE ) );

	// Draw border.
	pclSheet->SetCellBorder( CD_CPMIO_ODescription, RD_CPMIO_ColName, CD_CPMIO_OValue, RD_CPMIO_ColName, true, SS_BORDERTYPE_BOTTOM | SS_BORDERTYPE_TOP );
	//////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////


	//////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
	// Fill description columns.
	long lRow = RD_CPMIO_FirstAvailRow;
	pclSheet->SetTextPattern( CSSheet::_SSTextPattern::UserStaticColumn );
	pclSheet->SetTextPatternProperty( CSSheet::_SSTextPatternProperty::TPP_FontBold, (LPARAM)TRUE );
	pclSheet->SetTextPatternProperty( CSSheet::_SSTextPatternProperty::TPP_StyleHorizontalAlign, (LPARAM)SSS_ALIGN_LEFT );
	pclSheet->SetTextPatternProperty( CSSheet::_SSTextPatternProperty::TPP_StyleWordWrap, (LPARAM)TRUE );

	lRow = AddStaticText( pclSDInputOutput, CD_CPMIO_ODescription, lRow, TASApp.LoadLocalizedString( IDS_SSHEETSELPROD_CPM_CDDESCR_SYSTEMEXPCOEFF ), true );

	if( true == pclPMInputUser->IsNorm( ProjectType::All, PressurisationNorm::PN_SWKIHE301_01 ) )
	{
		// Factor [X].
		lRow = AddStaticText( pclSDInputOutput, CD_CPMIO_ODescription, lRow, TASApp.LoadLocalizedString( IDS_SSHEETSELPROD_CPM_CDDESCR_XFACTOR ), true );

		// System expansion volume [e.Vs.X].
		lRow = AddStaticText( pclSDInputOutput, CD_CPMIO_ODescription, lRow, TASApp.LoadLocalizedString( IDS_SSHEETSELPROD_CPM_CDDESCR_SYSTEMEXPVOLSWKI ), true );
	}
	else
	{
		// System expansion volume [Ve].
		lRow = AddStaticText( pclSDInputOutput, CD_CPMIO_ODescription, lRow, TASApp.LoadLocalizedString( IDS_SSHEETSELPROD_CPM_CDDESCR_SYSTEMEXPVOL ), true );
	}

	if( true == pclPMInputUser->IsNorm( ProjectType::All, PressurisationNorm::PN_SWKIHE301_01 ) 
			&& pclPMInputUser->GetStorageTankVolume() > 0.0 )
	{
		// Storage expansion coefficient [esto].
		lRow = AddStaticText( pclSDInputOutput, CD_CPMIO_ODescription, lRow, TASApp.LoadLocalizedString( IDS_SSHEETSELPROD_CPM_CDDESCR_STORAGETANKEXPCOEFF ), true );

		// Storage expansion volume [Vsto.esto].
		lRow = AddStaticText( pclSDInputOutput, CD_CPMIO_ODescription, lRow, TASApp.LoadLocalizedString( IDS_SSHEETSELPROD_CPM_CDDESCR_STORAGETANKEXPVOL ), true );
	}

	// Total expansion volume [Ve,tot].
	lRow = AddStaticText( pclSDInputOutput, CD_CPMIO_ODescription, lRow, TASApp.LoadLocalizedString( IDS_SSHEETSELPROD_CPM_CDDESCR_TOTALEXPVOL ), true );

	if( ProjectType::Solar == pclPMInputUser->GetApplicationType() )
	{
		// Solar collector security volume [VDK].
		lRow = AddStaticText( pclSDInputOutput, CD_CPMIO_ODescription, lRow, TASApp.LoadLocalizedString( IDS_SSHEETSELPROD_CPM_CDDESCR_SOLARCOLLECTORSECURITYVOLUME ), true );
	}

	// HYS-1022: 'Vwr' becomes 'Vwr,min'.
	// If we are not in the SWKI HE301-01 norm, we must show the minimal water reserve here.
	if( false == pclPMInputUser->IsNorm( ProjectType::All, PressurisationNorm::PN_SWKIHE301_01 ) )
	{
		lRow = AddStaticText( pclSDInputOutput, CD_CPMIO_ODescription, lRow, TASApp.LoadLocalizedString( IDS_SSHEETSELPROD_CPM_CDDESCR_MINWATRES ), true );
	}

	// Degassing water reserve [Vv].
	if( BST_CHECKED == pclPMInputUser->GetDegassingChecked() )
	{
		lRow = AddStaticText( pclSDInputOutput, CD_CPMIO_ODescription, lRow, TASApp.LoadLocalizedString( IDS_SSHEETSELPROD_CPM_CDDESCR_DEGASSINGWATERESERVE ), true );
	}

	// Vessel net volume [Vn].
	lRow = AddStaticText( pclSDInputOutput, CD_CPMIO_ODescription, lRow, TASApp.LoadLocalizedString( IDS_SSHEETSELPROD_CPM_CDDESCR_VESSELNETVOLUME ), true );

	// HYS-1022: 'Vwr' becomes 'Vwr,min'.
	// HYS-1534: We can show minimal water reserve for the SWKI HE301-01 norm -> minimal water reserve = Vs.e.(X - 1).
	// HYS-1534: If we are with the SWKI HE301-01, we show the 'Vwr,min' after the vessel net volume because this one is already in the expansion (Because X factor).
	if( true == pclPMInputUser->IsNorm( ProjectType::All, PressurisationNorm::PN_SWKIHE301_01 ) )
	{
		lRow = AddStaticText( pclSDInputOutput, CD_CPMIO_ODescription, lRow, TASApp.LoadLocalizedString( IDS_SSHEETSELPROD_CPM_CDDESCR_MINWATRES ), true );
	}

	// HYS-1534: We can show optimized water reserve for the SWKU 93-1 norm.
	if( CDS_SSelPMaint::SelectionType::Statico == pclSSelPressMaint->GetSelectionType() )
	{
		// HYS-1022: 'Vwr,opt' becomes 'Vwr'.
		lRow = AddStaticText( pclSDInputOutput, CD_CPMIO_ODescription, lRow, TASApp.LoadLocalizedString( IDS_SSHEETSELPROD_CPM_CDDESCR_WATRES ), true );
	}
	
	// HYS-1343 : We show the contraction volume only for expansion vessel in cooling.
	if( ProjectType::Cooling == pclSSelPressMaint->GetpInputUser()->GetApplicationType() 
			&& CDS_SSelPMaint::Statico == pclSSelPressMaint->GetSelectionType() )
	{
		lRow = AddStaticText( pclSDInputOutput, CD_CPMIO_ODescription, lRow, TASApp.LoadLocalizedString( IDS_SSHEETSELPROD_CPM_CDDESCR_CONTRVOL ), true );
	}
	
	lRow = AddStaticText( pclSDInputOutput, CD_CPMIO_ODescription, lRow, TASApp.LoadLocalizedString( IDS_SSHEETSELPROD_CPM_CDDESCR_VAPPRESS ), true );
	lRow = AddStaticText( pclSDInputOutput, CD_CPMIO_ODescription, lRow, TASApp.LoadLocalizedString( IDS_SSHEETSELPROD_CPM_CDDESCR_MINPRESS ), true );

	// HYS-1022: 'pa' becomes 'pa,min' (Minimum initial pressure).
	// HYS-1116: We show this value only if we are in EN12828 norm AND with a Statico.
	if( true == pclPMInputUser->IsNorm( ProjectType::All, PressurisationNorm::PN_EN12828 ) 
			&& CDS_SSelPMaint::Statico == pclSSelPressMaint->GetSelectionType() )
	{
		lRow = AddStaticText( pclSDInputOutput, CD_CPMIO_ODescription, lRow, TASApp.LoadLocalizedString( IDS_SSHEETSELPROD_CPM_CDDESCR_MININITPRESS ), true );
	}

	// HYS-1022: 'pa,opt' becomes 'pa' (Initial pressure).
	// HYS-1116: In all cases (Statico, Compresso or Transfero, EN12828 or not) we show now the initial pressure.
	// Remark: If we are in EN12828 with a Statico, we show the minimum initial pressure above and thus the initial pressure computed for this vessel.
	//         If we are in EN12828 with a Compresso/Transfero, we don't show the minimum initial pressure above but well the initial pressure (That is in fact
	//         also the p0 + 0.3 bar but we just don't want to call it 'minimum initial pressure' in this case).
	//         If we are not in EN12828 with a Statico/Compresso/Transfero, we don't show the minimum initial pressure but well only the initial pressure.
	lRow = AddStaticText( pclSDInputOutput, CD_CPMIO_ODescription, lRow, TASApp.LoadLocalizedString( IDS_SSHEETSELPROD_CPM_CDDESCR_INITPRESS ), true );

	// HYS-1343 : We show the filling pressure only for Expansion vessel
	if( CDS_SSelPMaint::Statico == pclSSelPressMaint->GetSelectionType() )
	{
		lRow = AddStaticText( pclSDInputOutput, CD_CPMIO_ODescription, lRow, TASApp.LoadLocalizedString( IDS_SSHEETSELPROD_CPM_CDDESCR_FILLINGPRESS ), true );
	}

	// HYS-1116: We show target pressure only for Compresso and Transfero.
	if( CDS_SSelPMaint::Statico != pclSSelPressMaint->GetSelectionType() )
	{
		lRow = AddStaticText( pclSDInputOutput, CD_CPMIO_ODescription, lRow, TASApp.LoadLocalizedString( IDS_SSHEETSELPROD_CPM_CDDESCR_TARGPRESS ), true );
	}

	lRow = AddStaticText( pclSDInputOutput, CD_CPMIO_ODescription, lRow, TASApp.LoadLocalizedString( IDS_SSHEETSELPROD_CPM_CDDESCR_FINALPRESS ), true );
	lRow = AddStaticText( pclSDInputOutput, CD_CPMIO_ODescription, lRow, TASApp.LoadLocalizedString( IDS_SSHEETSELPROD_CPM_CDDESCR_SPECQRATEEQVOL ), true );
	lRow = AddStaticText( pclSDInputOutput, CD_CPMIO_ODescription, lRow, TASApp.LoadLocalizedString( IDS_SSHEETSELPROD_CPM_CDDESCR_NEEDEDQRATEEQVOL ), true );
	lRow = AddStaticText( pclSDInputOutput, CD_CPMIO_ODescription, lRow, TASApp.LoadLocalizedString( IDS_SSHEETSELPROD_CPM_CDDESCR_PRESSFACTOR ), true );
	lRow = AddStaticText( pclSDInputOutput, CD_CPMIO_ODescription, lRow, TASApp.LoadLocalizedString( IDS_SSHEETSELPROD_CPM_CDDESCR_NOMVOL ), true );

	CString strLength = WriteCUDouble( _U_LENGTH, 10, true, 0 );
	FormatString( str, IDS_SSHEETSELPROD_CPM_CDDESCR_EXPPIPE, strLength );
	lRow = AddStaticText( pclSDInputOutput, CD_CPMIO_ODescription, lRow, str, true );

	strLength = WriteCUDouble( _U_LENGTH, 30, true, 0 );
	FormatString( str, IDS_SSHEETSELPROD_CPM_CDDESCR_EXPPIPE, strLength );
	lRow = AddStaticText( pclSDInputOutput, CD_CPMIO_ODescription, lRow, str, true );
	//////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////


	//////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
	// Fill values and units.
	lRow = RD_CPMIO_FirstAvailRow;
	pclSheet->SetTextPatternProperty( CSSheet::_SSTextPatternProperty::TPP_FontBold, (LPARAM)FALSE );
	pclSheet->SetTextPatternProperty( CSSheet::_SSTextPatternProperty::TPP_StyleHorizontalAlign, (LPARAM)SSS_ALIGN_LEFT );

	// System expansion coefficient [e].
	lRow = AddStaticText( pclSDInputOutput, CD_CPMIO_OValue, lRow, WriteDouble( pclPMInputUser->GetSystemExpansionCoefficient(), 4, 3 ) );
	
	// HYS-1534: for SWKI HE301-01 norm, if storage tank volume has been filled by user, we show data.
	if( true == pclPMInputUser->IsNorm( ProjectType::All, PressurisationNorm::PN_SWKIHE301_01 ) )
	{
		// Factor [X]
		lRow = AddStaticText( pclSDInputOutput, CD_CPMIO_OValue, lRow, WriteDouble( pclPMInputUser->GetXFactorSWKI(), 3, 1 ) );
	}

	// System expansion volume [Ve].
	lRow = AddStaticText( pclSDInputOutput, CD_CPMIO_OValue, lRow, WriteCUDouble( _U_VOLUME, pclPMInputUser->GetSystemExpansionVolume(), true ) );

	if( true == pclPMInputUser->IsNorm( ProjectType::All, PressurisationNorm::PN_SWKIHE301_01 ) && pclPMInputUser->GetStorageTankVolume() > 0.0 )
	{
		// Storage expansion coefficient [esto]
		lRow = AddStaticText( pclSDInputOutput, CD_CPMIO_OValue, lRow, WriteDouble( pclPMInputUser->GetStorageTankExpansionCoefficient(), 4, 3 ) );

		// Storage expansion volume [Vsto.esto]
		lRow = AddStaticText( pclSDInputOutput, CD_CPMIO_OValue, lRow, WriteCUDouble( _U_VOLUME, pclPMInputUser->GetStorageTankExpansionVolume(), true ) );
	}

	// Total expansion volume [Ve,tot].
	lRow = AddStaticText( pclSDInputOutput, CD_CPMIO_OValue, lRow, WriteCUDouble( _U_VOLUME, pclPMInputUser->GetTotalExpansionVolume(), true ) );

	if( ProjectType::Solar == pclPMInputUser->GetApplicationType() )
	{
		// Solar collector security volume.
		lRow = AddStaticText( pclSDInputOutput, CD_CPMIO_OValue, lRow, WriteCUDouble( _U_VOLUME, pclPMInputUser->GetSolarCollectorSecurityVolume(), true ) );
	}

	// HYS-1022: 'Vwr' becomes 'Vwr,min'.
	// If we are not in the SWKI HE301-01 norm, we must show the minimal water reserve here.
	if( false == pclPMInputUser->IsNorm( ProjectType::All, PressurisationNorm::PN_SWKIHE301_01 ) )
	{
		lRow = AddStaticText( pclSDInputOutput, CD_CPMIO_OValue, lRow, WriteCUDouble( _U_VOLUME, pclPMInputUser->GetMinimumWaterReserve(), true ) );
	}

	// Degassing water reserve [Vv].
	if( BST_CHECKED == pclPMInputUser->GetDegassingChecked() )
	{
		lRow = AddStaticText( pclSDInputOutput, CD_CPMIO_OValue, lRow, WriteCUDouble( _U_VOLUME, pclPMInputUser->GetDegassingWaterReserve(), true ) );
	}

	// Vessel net volume [Vn].
	lRow = AddStaticText( pclSDInputOutput, CD_CPMIO_OValue, lRow, WriteCUDouble( _U_VOLUME, pclPMInputUser->GetVesselNetVolume(), true ) );

	// HYS-1022: 'Vwr' becomes 'Vwr,min'.
	// HYS-1534: We can show minimal water reserve for the SWKI HE301-01 norm -> minimal water reserve = Vs.e.(X - 1).
	// HYS-1534: If we are with the SWKI HE301-01, we show the 'Vwr,min' after the vessel net volume because this one is already in the expansion (Because X factor).
	if( true == pclPMInputUser->IsNorm( ProjectType::All, PressurisationNorm::PN_SWKIHE301_01 ) )
	{
		lRow = AddStaticText( pclSDInputOutput, CD_CPMIO_OValue, lRow, WriteCUDouble( _U_VOLUME, pclPMInputUser->GetMinimumWaterReserve(), true ) );
	}

	// HYS-1534: We can show optimized water reserve for the SWKU 93-1 norm.
	if( CDS_SSelPMaint::Statico == pclSSelPressMaint->GetSelectionType() )
	{
		// HYS-1022: 'Vwr,opt' becomes 'Vwr'.
		lRow = AddStaticText( pclSDInputOutput, CD_CPMIO_OValue, lRow, WriteCUDouble( _U_VOLUME, pclSSelPressMaint->GetVesselWaterReserve(), true ) );
	}

	// HYS-1343 : We show the contraction volume only for Expansion vessel
	if( ProjectType::Cooling == pclSSelPressMaint->GetpInputUser()->GetApplicationType() 
		&& CDS_SSelPMaint::Statico == pclSSelPressMaint->GetSelectionType() )
	{
		lRow = AddStaticText( pclSDInputOutput, CD_CPMIO_OValue, lRow, WriteCUDouble( _U_VOLUME, pclSSelPressMaint->GetpInputUser()->ComputeContractionVolume(), true ) );
	}

	lRow = AddStaticText( pclSDInputOutput, CD_CPMIO_OValue, lRow, WriteCUDouble( _U_PRESSURE, pclPMInputUser->GetVaporPressure(), true ) );

	
	lRow = AddStaticText( pclSDInputOutput, CD_CPMIO_OValue, lRow, WriteCUDouble( _U_PRESSURE, pclPMInputUser->GetMinimumPressure(), true ) );

	// Minimal initial pressure.
	// HYS-1022: 'pa' becomes 'pa,min' (Minimum initial pressure).
	// HYS-1116: We show this value only if we are in EN12828 norm AND with a Statico.
	if( true == pclPMInputUser->IsNorm( ProjectType::All, PressurisationNorm::PN_EN12828 ) 
			&& CDS_SSelPMaint::Statico == pclSSelPressMaint->GetSelectionType() )
	{
		str = WriteCUDouble( _U_PRESSURE, pclPMInputUser->GetMinimumInitialPressure() , true );
		lRow = AddStaticText( pclSDInputOutput, CD_CPMIO_OValue, lRow, str );
	}

	// HYS-1022: 'pa,opt' becomes 'pa' (Initial pressure).
	// HYS-1116: In all cases (Statico, Compresso or Transfero, EN12828 or not) we show now the initial pressure.
	// Remark: If we are in EN12828 with a Statico, we show the minimum initial pressure above and thus the initial pressure computed for this vessel.
	//         If we are in EN12828 with a Compresso/Transfero, we don't show the minimum initial pressure above but well the initial pressure (That is in fact
	//         also the p0 + 0.3 bar but we just don't want to call it 'minimum initial pressure' in this case).
	//         If we are not in EN12828 with a Statico/Compresso/Transfero, we don't show the minimum initial pressure but well only the initial pressure.
	if( CDS_SSelPMaint::Statico == pclSSelPressMaint->GetSelectionType() )
	{
		// For Statico we write the real initial pressure computed.
		lRow = AddStaticText( pclSDInputOutput, CD_CPMIO_OValue, lRow, WriteCUDouble( _U_PRESSURE, pclSSelPressMaint->GetVesselInitialPressure(), true ) );
	}
	else
	{
		// For Compresso/Transfero it is always the minimum initial pressure.
		lRow = AddStaticText( pclSDInputOutput, CD_CPMIO_OValue, lRow, WriteCUDouble( _U_PRESSURE, pclPMInputUser->GetMinimumInitialPressure(), true ) );
	}

	// HYS-1054: pfill (filling pressure) is added in calculated data
	str = _T("-");
	// HYS-1343 : We show the filling pressure only for Expansion vessel
	if( ( CDS_SSelPMaint::Statico == pclSSelPressMaint->GetSelectionType() )
		&& ( NULL != dynamic_cast<CDB_Vessel *>(pclSSelPressMaint->GetVesselIDPtr().MP ) ) )
	{
		double dTotalVesselVolume = pclSSelPressMaint->GetVesselNumber() * (dynamic_cast<CDB_Vessel *>(pclSSelPressMaint->GetVesselIDPtr().MP)->GetNominalVolume() );
		double dFillingPressure = pclPMInputUser->GetIntermediatePressure( pclPMInputUser->GetFillTemperature(), pclSSelPressMaint->GetVesselWaterReserve(), dTotalVesselVolume );
		str = WriteCUDouble( _U_PRESSURE, dFillingPressure, true );
		lRow = AddStaticText( pclSDInputOutput, CD_CPMIO_OValue, lRow, str );
	}

	// Target pressure.
	if( CDS_SSelPMaint::Statico != pclSSelPressMaint->GetSelectionType() )
	{
		str = _T("-");
		CDB_TecBox *pclTecBox = dynamic_cast<CDB_TecBox *>( pclSSelPressMaint->GetTecBoxCompTransfIDPtr().MP );

		if( NULL != pclTecBox )
		{
			str = WriteCUDouble( _U_PRESSURE, pclPMInputUser->GetTargetPressureForTecBox( pclTecBox->GetTecBoxType() ) , true );
		}

		lRow = AddStaticText( pclSDInputOutput, CD_CPMIO_OValue, lRow, str );
	}

	// Final pressure.
	str = _T("-");

	if( CDS_SSelPMaint::Statico == pclSSelPressMaint->GetSelectionType() )
	{
		str = WriteCUDouble( _U_PRESSURE, pclPMInputUser->GetFinalPressure(), true );
	}
	else
	{
		CDB_TecBox *pclTecBox = dynamic_cast<CDB_TecBox *>( pclSSelPressMaint->GetTecBoxCompTransfIDPtr().MP );
		if( NULL != pclTecBox )
		{
			str = WriteCUDouble( _U_PRESSURE, pclPMInputUser->GetFinalPressure( pclTecBox->GetTecBoxType() ), true );
		}
	}

	lRow = AddStaticText( pclSDInputOutput, CD_CPMIO_OValue, lRow, str );

	// Specific flow rate of equalization volume.
	str = _T("-");

	if( CDS_SSelPMaint::Statico != pclSSelPressMaint->GetSelectionType() )
	{
		double dEqualizingVolumetricFlow = pclPMInputUser->GetEqualizingVolumetricFlow();
		double d1 = CDimValue::SItoCU( _U_FLOW, dEqualizingVolumetricFlow );
		double d2 = CDimValue::SItoCU( _U_TH_POWER, 1 / d1 );

		if( 0 != d2 )
		{
			d2 = 1 / d2;
			str = WriteDouble( d2, 4, 0 );
			CString strUnit = CString( CDimValue::AccessUDB()->GetNameOfDefaultUnit( _U_FLOW ).c_str() ) + CString( _T("/") )
							  + CString( CDimValue::AccessUDB()->GetNameOfDefaultUnit( _U_TH_POWER ).c_str() );
			str = str + _T(" ") + strUnit;
		}
	}

	lRow = AddStaticText( pclSDInputOutput, CD_CPMIO_OValue, lRow, str );

	// Needed flow rate of equalization volume.
	str = _T("-");

	if( CDS_SSelPMaint::Statico != pclSSelPressMaint->GetSelectionType() )
	{
		str = WriteCUDouble( _U_FLOW, pclPMInputUser->GetVD() , true );
	}

	lRow = AddStaticText( pclSDInputOutput, CD_CPMIO_OValue, lRow, str );

	if( CDS_SSelPMaint::Statico == pclSSelPressMaint->GetSelectionType() )
	{
		lRow = AddStaticText( pclSDInputOutput, CD_CPMIO_OValue, lRow, WriteDouble( pclPMInputUser->GetPressureFactor(), 3, 2 ) );
		lRow = AddStaticText( pclSDInputOutput, CD_CPMIO_OValue, lRow, WriteCUDouble( _U_VOLUME, pclPMInputUser->GetVesselNominalVolume(), true ) );
	}
	else
	{
		lRow = AddStaticText( pclSDInputOutput, CD_CPMIO_OValue, lRow, WriteDouble( pclPMInputUser->GetPressureFactor( true ), 3, 2 ) );
		lRow = AddStaticText( pclSDInputOutput, CD_CPMIO_OValue, lRow, WriteCUDouble( _U_VOLUME, pclPMInputUser->GetVesselNominalVolume( true ), true ) );
	}

	// Expansion pipe.
	// HYS-1340: Expansion pipe also exist for Statico.
	IDPTR PipeDNIDPTR = _NULL_IDPTR;

	if( CDS_SSelPMaint::Statico == pclSSelPressMaint->GetSelectionType() )
	{
		IDPTR PipeDNIDPTR = pclPMInputUser->GetExpansionPipeSizeIDPtr( (CDB_Product *)( pclSSelPressMaint->GetVesselIDPtr().MP ), 10.0 );
		CDB_StringID *pPipeDN = dynamic_cast<CDB_StringID *>( PipeDNIDPTR.MP );

		if( NULL != pPipeDN )
		{
			lRow = AddStaticText( pclSDInputOutput, CD_CPMIO_OValue, lRow, pPipeDN->GetString() );
			lRow = AddStaticText( pclSDInputOutput, CD_CPMIO_OValue, lRow, pPipeDN->GetString() );
		}
		else
		{
			lRow = AddStaticText( pclSDInputOutput, CD_CPMIO_OValue, lRow, _T("-") );
			lRow = AddStaticText( pclSDInputOutput, CD_CPMIO_OValue, lRow, _T("-") );
		}
	}
	else
	{
		IDPTR PipeDNIDPTR = pclPMInputUser->GetExpansionPipeSizeIDPtr( (CDB_Product *)( pclSSelPressMaint->GetTecBoxCompTransfIDPtr().MP ), 10.0 );
		CDB_StringID *pPipeDN = dynamic_cast<CDB_StringID *>( PipeDNIDPTR.MP );

		if( NULL != pPipeDN )
		{
			lRow = AddStaticText( pclSDInputOutput, CD_CPMIO_OValue, lRow, pPipeDN->GetString() );
		}
		else
		{
			lRow = AddStaticText( pclSDInputOutput, CD_CPMIO_OValue, lRow, _T("-") );
		}

		PipeDNIDPTR = pclPMInputUser->GetExpansionPipeSizeIDPtr( (CDB_Product *)( pclSSelPressMaint->GetTecBoxCompTransfIDPtr().MP ), 30.0 );
		pPipeDN = dynamic_cast<CDB_StringID *>( PipeDNIDPTR.MP );

		if( NULL != pPipeDN )
		{
			lRow = AddStaticText( pclSDInputOutput, CD_CPMIO_OValue, lRow, pPipeDN->GetString() );
		}
		else
		{
			lRow = AddStaticText( pclSDInputOutput, CD_CPMIO_OValue, lRow, _T("-") );
		}
	}

	//////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
}

void CSelProdPagePressMaint::_FillComputedProduct( CDS_SSelPMaint *pclSSelPressMaint )
{
	if( NULL == pclSSelPressMaint )
	{
		return;
	}

	// Call base class method to get a new 'Sheet'.
	CSheetDescription *pclSDProduct = CMultiSpreadBase::CreateSSheet( SD_CPMProduct + m_uiSDComputedPCount );

	if( NULL == pclSDProduct || NULL == pclSDProduct->GetSSheetPointer() )
	{
		return;
	}

	CSSheet *pclSheet = pclSDProduct->GetSSheetPointer();

	if( false == CSelProdPageBase::PrepareSheet( pclSDProduct, CD_CPMP_Pointer, m_bForPrint, false ) )
	{
		m_ViewDescription.RemoveOneSheetDescription( SD_CPMProduct + m_uiSDComputedPCount );
		return;
	}

	m_uiSDComputedPCount++;

	// Initialize column width for header and footer column.
	if( false == m_bForPrint )
	{
		SetColWidth( pclSDProduct, CD_CPMP_Header, PMColumnWidth::PMCW_Header );
		SetColWidth( pclSDProduct, CD_CPMP_Footer, PMColumnWidth::PMCW_Footer );
	}
	else
	{
		pclSheet->ShowCol( CD_CPMP_Header, FALSE );
		pclSheet->ShowCol( CD_CPMP_Footer, FALSE );
	}

	// Set the column width. To perfectly match the main title, we use proportionality.
	int iHFColPixels = ( false == m_bForPrint ) ? pclSheet->ColWidthToLogUnits( 2.0 ) : 0;
	double dAvailableWidth = pclSheet->LogUnitsToColWidthW( RIGHTVIEWWIDTH - 2 * iHFColPixels );

	// To adapt column width, we take default defined width and do a proportionality.
	double dTotalColWidth = PMColumnWidth::PMCW_Reference1 + PMColumnWidth::PMCW_Reference2 + PMColumnWidth::PMCW_Product;
	dTotalColWidth += PMColumnWidth::PMCW_ArticleNumber + PMColumnWidth::PMCW_Quantity + PMColumnWidth::PMCW_Remark;

	if( true == TASApp.IsPriceUsed() )
	{
		dTotalColWidth += PMColumnWidth::PMCW_UnitPrice;
		dTotalColWidth += PMColumnWidth::PMCW_TotalPrice;
	}
	
	double dWidth = dAvailableWidth * PMColumnWidth::PMCW_Reference1 / dTotalColWidth;
	double dTotalWidth = dWidth;
	SetColWidth( pclSDProduct, CD_CPMP_Reference1, dWidth );

	dWidth = dAvailableWidth * PMColumnWidth::PMCW_Reference2 / dTotalColWidth;
	dTotalWidth += dWidth;
	SetColWidth( pclSDProduct, CD_CPMP_Reference2, dWidth );

	dWidth = dAvailableWidth * PMColumnWidth::PMCW_Product / dTotalColWidth;
	dTotalWidth += dWidth;
	SetColWidth( pclSDProduct, CD_CPMP_Product, dWidth );


	dWidth = dAvailableWidth * PMColumnWidth::PMCW_ArticleNumber / dTotalColWidth;
	dTotalWidth += dWidth;
	SetColWidth( pclSDProduct, CD_CPMP_ArticleNumber, dWidth );

	dWidth = dAvailableWidth * PMColumnWidth::PMCW_Quantity / dTotalColWidth;
	dTotalWidth += dWidth;
	SetColWidth( pclSDProduct, CD_CPMP_Quantity, dWidth );

	if( true == TASApp.IsPriceUsed() )
	{
		dWidth = dAvailableWidth * PMColumnWidth::PMCW_UnitPrice / dTotalColWidth;
		dTotalWidth += dWidth;
		SetColWidth( pclSDProduct, CD_CPMP_UnitPrice, dWidth );

		dWidth = dAvailableWidth * PMColumnWidth::PMCW_TotalPrice / dTotalColWidth;
		dTotalWidth += dWidth;
		SetColWidth( pclSDProduct, CD_CPMP_TotalPrice, dWidth );
	}

	SetColWidth( pclSDProduct, CD_CPMP_Remark, dAvailableWidth - dTotalWidth );

	if( false == TASApp.IsPriceUsed() )
	{
		pclSheet->ShowCol( CD_CPMP_UnitPrice, FALSE );
		pclSheet->ShowCol( CD_CPMP_TotalPrice, FALSE );
	}

	pclSheet->ShowCol( CD_CPMP_Pointer, FALSE );

	// HYS-1571: We can optimize the column width now.
	// See Jira card to have more details about why we do that here.
	CRect rectClient;
	GetClientRect( &rectClient );
	OptimizePageSize( rectClient.Width(), rectClient.Height() );

	CRect rect = pclSheet->GetSheetSizeInPixels( false );
	m_mapSheetInfos[pclSDProduct].m_dPageWidth = rect.Width();


	//////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
	// Set title.
	// Remark: 'CPM' is for 'Computed Pressure Maintenance' and 'P' is for 'Product'.
	CString str = TASApp.LoadLocalizedString( IDS_SSHEETSELPROD_CPM_P_TITLE );
	pclSheet->SetTextPattern( CSSheet::_SSTextPattern::SubTitle );
	pclSheet->SetTextPatternProperty( CSSheet::_SSTextPatternProperty::TPP_StyleHorizontalAlign, (LPARAM)SSS_ALIGN_LEFT );
	pclSheet->SetTextPatternProperty( CSSheet::_SSTextPatternProperty::TPP_FontSize, (LPARAM)11 );
	pclSheet->SetTextPatternProperty( CSSheet::_SSTextPatternProperty::TPP_FontBackColor, (LPARAM)_IMI_TITLE_GROUP2 );
	pclSheet->SetTextPatternProperty( CSSheet::_SSTextPatternProperty::TPP_FontForeColor, (LPARAM)_BLACK );

	// Write title on all columns, title is not hidden when columns are hidden.
	// We set the text from the column after header column (1) and before the footer column (m_iColNum-1).
	for( int iLoopColumn = CD_CPMP_Reference1; iLoopColumn <= CD_CPMP_Remark; iLoopColumn++ )
	{
		AddStaticText( pclSDProduct, iLoopColumn, RD_CPMP_Title, str );
	}

	pclSheet->AddCellSpanW( CD_CPMP_Reference1, RD_CPMP_Title, CD_CPMP_Remark - CD_CPMP_Reference1 + 1, 1 );
	pclSheet->SetRowHeight( RD_CPMP_Title, m_dRowHeight * 1.7 );
	//////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////


	//////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
	// Init column headers.
	pclSheet->SetTextPattern( CSSheet::_SSTextPattern::ColumnHeader );
	pclSheet->SetTextPatternProperty( CSSheet::_SSTextPatternProperty::TPP_FontBackColor, (LPARAM)_TAH_WHITE );
	pclSheet->SetTextPatternProperty( CSSheet::_SSTextPatternProperty::TPP_StyleWordWrap, (LPARAM)TRUE );

	// Remark: 'CPM' is for 'Computed Pressure Maintenance' and 'PH' is for 'Product Header'.
	AddStaticText( pclSDProduct, CD_CPMP_Reference1, RD_CPMP_ColName, TASApp.LoadLocalizedString( IDS_SELPHDR_REF1 ) );
	AddStaticText( pclSDProduct, CD_CPMP_Reference2, RD_CPMP_ColName, TASApp.LoadLocalizedString( IDS_SELPHDR_REF2 ) );
	AddStaticText( pclSDProduct, CD_CPMP_Product, RD_CPMP_ColName, TASApp.LoadLocalizedString( IDS_SELPHDR_PRODUCT ) );
	AddStaticText( pclSDProduct, CD_CPMP_ArticleNumber, RD_CPMP_ColName, TASApp.LoadLocalizedString( IDS_SELPHDR_ART ) );
	AddStaticText( pclSDProduct, CD_CPMP_Quantity, RD_CPMP_ColName, TASApp.LoadLocalizedString( IDS_SELPHDR_QTY ) );

	if( true == TASApp.IsPriceUsed() )
	{
		str = TASApp.LoadLocalizedString( IDS_SELPHDR_PRICE ) + _T("\r\n[");
		str += CString( m_pTADS->GetpTechParams()->GetCurrentCurrencyISO().c_str() ) + _T("]");
		AddStaticText( pclSDProduct, CD_CPMP_UnitPrice, RD_CPMP_ColName, str );

		str = TASApp.LoadLocalizedString( IDS_SELPHDR_TOTAL ) + _T("\r\n[");
		str += CString( m_pTADS->GetpTechParams()->GetCurrentCurrencyISO().c_str() ) + _T("]");
		AddStaticText( pclSDProduct, CD_CPMP_TotalPrice, RD_CPMP_ColName, str );
	}

	AddStaticText( pclSDProduct, CD_CPMP_Remark, RD_CPMP_ColName, TASApp.LoadLocalizedString( IDS_SELPHDR_REM ) );
	pclSheet->SetRowHeight( RD_CPMP_ColName, m_dRowHeight * 1.6 );

	// Draw border.
	pclSheet->SetCellBorder( CD_CPMP_Reference1, RD_CPMP_ColName, CD_CPMP_Remark, RD_CPMP_ColName, true,
							 SS_BORDERTYPE_BOTTOM | SS_BORDERTYPE_TOP );

	// HYS-1152: We now select with a border only and then we add these two rows in the selection.
	m_clCPMPParam.AddRange( RD_CPMP_Title, RD_CPMP_ColName, NULL );
	//////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////


	long lRow = RD_CPMP_FirstAvailRow;

	if( CDS_SSelPMaint::NoPressurization != pclSSelPressMaint->GetSelectionType() )
	{
		// Fill Statico.
		if( CDS_SSelPMaint::SelectionType::Statico == pclSSelPressMaint->GetSelectionType() )
		{
			// Fill Statico.
			int iQuantity = pclSSelPressMaint->GetVesselNumber() * pclSSelPressMaint->GetpSelectedInfos()->GetQuantity();
			CDB_Vessel *pclVessel = (CDB_Vessel *)( pclSSelPressMaint->GetVesselIDPtr().MP );

			long lFirstRow = lRow;
			lRow = _FillRowVesselProduct( pclSDProduct, lRow, pclVessel, pclSSelPressMaint, iQuantity );

			// Fill references and remarks.
			_FillRowGen( pclSDProduct, pclSSelPressMaint, RD_CPMP_FirstAvailRow, lRow - 1 );

			// Add accessories.
			if( NULL != pclSSelPressMaint->GetVesselAccessoryList() && pclSSelPressMaint->GetVesselAccessoryList()->GetCount() > 0 )
			{
				// Draw dash line.
				pclSheet->SetCellBorder( CD_CPMP_Reference1, lRow, CD_CPMP_Footer - 1, lRow, true, SS_BORDERTYPE_BOTTOM, SS_BORDERSTYLE_FINE_DASH );

				// Attention: this 'method' doesn't return the next row but the last row!!
				lRow++;
				int iDistributedQty = pclSSelPressMaint->GetVesselNumber() + pclSSelPressMaint->GetSecondaryVesselNumber();
				lRow = FillAccessories( pclSDProduct, lRow, pclSSelPressMaint->GetVesselAccessoryList(), pclSSelPressMaint->GetpSelectedInfos()->GetQuantity(), iDistributedQty,
										&m_clCPMPParam );
			}

			// Draw solid line.
			pclSheet->SetCellBorder( CD_CPMP_Reference1, lRow, CD_CPMP_Footer - 1, lRow, true, SS_BORDERTYPE_BOTTOM, SS_BORDERSTYLE_SOLID );

			// Set the group as no breakable (for print).
			// Remark: include titles with the group.
			pclSheet->SetFlagRowNoBreakable( RD_CPMP_Title, lRow, true );

			// Add intermediate vessel if exist.
			IDPTR IntermVesselIDPtr = pclSSelPressMaint->GetIntermediateVesselIDPtr();

			if( _NULL_IDPTR != IntermVesselIDPtr && NULL != dynamic_cast<CDB_Product *>( IntermVesselIDPtr.MP ) )
			{
				// Intermediate vessel is in fact a Statico.
				lRow++;
				long lRowFirstRowNoBreakable = lRow;
				int iQuantity = pclSSelPressMaint->GetIntermediateVesselNumber() * pclSSelPressMaint->GetpSelectedInfos()->GetQuantity();
				lRow = _FillRowVesselProduct( pclSDProduct, lRow, dynamic_cast<CDB_Vessel *>( IntermVesselIDPtr.MP ), pclSSelPressMaint, iQuantity );

				// Add accessory if exist.
				if( NULL != pclSSelPressMaint->GetIntermediateVesselAccessoryList() && pclSSelPressMaint->GetIntermediateVesselAccessoryList()->GetCount() > 0 )
				{
					// Draw dash line.
					pclSheet->SetCellBorder( CD_CPMP_Reference1, lRow, CD_CPMP_Footer - 1, lRow, true, SS_BORDERTYPE_BOTTOM, SS_BORDERSTYLE_FINE_DASH );

					// Attention: this 'method' doesn't return the next row but the last row!!
					lRow++;
					int iDistributedQty = pclSSelPressMaint->GetIntermediateVesselNumber();
					lRow = FillAccessories( pclSDProduct, lRow, pclSSelPressMaint->GetIntermediateVesselAccessoryList(), pclSSelPressMaint->GetpSelectedInfos()->GetQuantity(),
											iDistributedQty, &m_clCPMPParam );
				}

				pclSheet->SetCellBorder( CD_CPMP_Reference1, lRow, CD_CPMP_Remark, lRow, true, SS_BORDERTYPE_BOTTOM, SS_BORDERSTYLE_SOLID );

				// Set the group as no breakable (for print).
				pclSheet->SetFlagRowNoBreakable( lRowFirstRowNoBreakable, lRow, true );
			}

			// Fill Vento if exist.
			if( NULL != pclSSelPressMaint->GetTecBoxVentoIDPtr().MP )
			{
				lRow++;
				long lRowFirstRowNoBreakable = lRow;
				lRow = _FillRowTecBoxVentoProduct( pclSDProduct, lRow, pclSSelPressMaint );

				// Set the group as no breakable (for print).
				pclSheet->SetFlagRowNoBreakable( lRowFirstRowNoBreakable, lRow, true );
			}

			// Fill Pleno if exist.
			if( NULL != pclSSelPressMaint->GetTecBoxPlenoIDPtr().MP || NULL != pclSSelPressMaint->GetAdditionalTecBoxPlenoIDPtr().MP )
			{
				lRow++;
				long lRowFirstRowNoBreakable = lRow;
				lRow = _FillRowTecBoxPlenoProduct( pclSDProduct, lRow, pclSSelPressMaint );

				// Set the group as no breakable (for print).
				pclSheet->SetFlagRowNoBreakable( lRowFirstRowNoBreakable, lRow, true );
			}

			// Fill Pleno refill if exist.
			if( NULL != pclSSelPressMaint->GetPlenoRefillIDPtr().MP )
			{
				lRow++;
				long lRowFirstRowNoBreakable = lRow;
				lRow = _FillRowPlenoRefillProduct( pclSDProduct, lRow, pclSSelPressMaint, false, false );

				// Set the group as no breakable (for print).
				pclSheet->SetFlagRowNoBreakable( lRowFirstRowNoBreakable, lRow, true );
			}
		}
		else
		{
			// Fill Compresso or Transfero.
			lRow = _FillRowCprssoTrsfroProduct( pclSDProduct, lRow, pclSSelPressMaint, true );

			// Set the group as no breakable (for print).
			// Remark: include titles with the group.
			pclSheet->SetFlagRowNoBreakable( RD_CPMP_Title, lRow, true );
			
			bool bPrimaryVesselIntegrated = false;

			// Fill integrated vessel if needed.
			if( _NULL_IDPTR != pclSSelPressMaint->GetTecBoxIntegratedVesselIDPtr() )
			{
				// Draw a dash line.
				pclSheet->SetCellBorder( CD_CPMP_Reference1, lRow, CD_CPMP_Remark, lRow, true, SS_BORDERTYPE_BOTTOM, SS_BORDERSTYLE_FINE_DASH );

				lRow++;
				long lRowFirstRowNoBreakable = lRow;
				CDB_Vessel *pclIntegratedVessel = (CDB_Vessel *)( pclSSelPressMaint->GetTecBoxIntegratedVesselIDPtr().MP );
				int iQuantity = pclSSelPressMaint->GetTecBoxCompTransfNumber() * pclSSelPressMaint->GetpSelectedInfos()->GetQuantity();
				lRow = _FillRowVesselProduct( pclSDProduct, lRow, pclIntegratedVessel, pclSSelPressMaint, iQuantity, true );

				bPrimaryVesselIntegrated = pclIntegratedVessel->IsPrimaryVesselIntegrated();

				// HYS-872: Add accessories.
				if (NULL != pclSSelPressMaint->GetTecBoxIntegratedVesselAccessoryList() && pclSSelPressMaint->GetTecBoxIntegratedVesselAccessoryList()->GetCount() > 0)
				{
					// Draw dash line.
					pclSheet->SetCellBorder(CD_CPMP_Reference1, lRow, CD_CPMP_Footer - 1, lRow, true, SS_BORDERTYPE_BOTTOM, SS_BORDERSTYLE_FINE_DASH);

					// Attention: this 'method' doesn't return the next row but the last row!!
					lRow++;
					int iDistributedQty = pclSSelPressMaint->GetTecBoxCompTransfNumber();
					lRow = FillAccessories(pclSDProduct, lRow, pclSSelPressMaint->GetTecBoxIntegratedVesselAccessoryList(), pclSSelPressMaint->GetpSelectedInfos()->GetQuantity(),
						iDistributedQty, &m_clCPMPParam);
				}

				// Draw solid line.
				pclSheet->SetCellBorder(CD_CPMP_Reference1, lRow, CD_CPMP_Footer - 1, lRow, true, SS_BORDERTYPE_BOTTOM, SS_BORDERSTYLE_SOLID);
				// Set the group as no breakable (for print).
				pclSheet->SetFlagRowNoBreakable(lRowFirstRowNoBreakable, lRow, true);
			}

			// Fill primary vessel.
			long lRowFirstRowNoBreakable = lRow;
			int iQuantity = pclSSelPressMaint->GetVesselNumber() * pclSSelPressMaint->GetpSelectedInfos()->GetQuantity();
			CDB_Vessel *pclVessel = (CDB_Vessel *)( pclSSelPressMaint->GetVesselIDPtr().MP );

			// Prepare three accessory lists for primary and secondary vessel in case of an accessory in the primary vessel list is set as 'Distributed'.
			bool fSecondaryVesselExist = ( NULL != dynamic_cast<CDB_Vessel *>( ( CData * )pclSSelPressMaint->GetSecondaryVesselIDPtr().MP ) ) ? true : false;
			CAccessoryList clPrimOnlyVesselAccessoryList;
			CAccessoryList clSecOnlyVesselAccessoryList;
			CAccessoryList clPrimAndSecVesselAccessoryList;

			if( NULL != pclSSelPressMaint->GetVesselAccessoryList() )
			{
				CAccessoryList::AccessoryItem rAccessoryItem = pclSSelPressMaint->GetVesselAccessoryList()->GetFirst();

				while( _NULL_IDPTR != rAccessoryItem.IDPtr )
				{
					if( false == rAccessoryItem.fDistributed )
					{
						if( true == fSecondaryVesselExist )
						{
							clPrimAndSecVesselAccessoryList.Add( rAccessoryItem.eAccessoryType, &rAccessoryItem );
						}
						else
						{
							clPrimOnlyVesselAccessoryList.Add( rAccessoryItem.eAccessoryType, &rAccessoryItem );
						}
					}
					else
					{
						clPrimOnlyVesselAccessoryList.Add( rAccessoryItem.eAccessoryType, &rAccessoryItem );

						if( true == fSecondaryVesselExist )
						{
							clSecOnlyVesselAccessoryList.Add( rAccessoryItem.eAccessoryType, &rAccessoryItem );
						}
					}

					rAccessoryItem = pclSSelPressMaint->GetVesselAccessoryList()->GetNext();
				}
			}

			// We have two solutions about integrated vessel. Either it's a buffer vessel as it's the case for Transfero TV.
			// Or it's a real integrated vessel that is playing the role of the primary vessel (like the one in the Simply Compresso).
			// When it's a buffer vessel, we show primary and secondary just after the buffer vessel.
			// When it's a integrated vessel, we must show after only the secondary vessel.

			if( false == bPrimaryVesselIntegrated )
			{
				lRow++;
				lRowFirstRowNoBreakable = lRow;
				lRow = _FillRowVesselProduct( pclSDProduct, lRow, pclVessel, pclSSelPressMaint, iQuantity );

				// If primary vessel accessories exist...
				if( clPrimOnlyVesselAccessoryList.GetCount() > 0 )
				{
					// Draw dash line.
					pclSheet->SetCellBorder( CD_CPMP_Reference1, lRow, CD_CPMP_Footer - 1, lRow, true, SS_BORDERTYPE_BOTTOM, SS_BORDERSTYLE_FINE_DASH );

					// Attention: this 'method' doesn't return the next row but the last row!!
					lRow++;
					lRow = FillAccessories( pclSDProduct, lRow, &clPrimOnlyVesselAccessoryList, pclSSelPressMaint->GetpSelectedInfos()->GetQuantity(), 1, &m_clCPMPParam );
				}

				// Draw solid line.
				pclSheet->SetCellBorder( CD_CPMP_Reference1, lRow, CD_CPMP_Footer - 1, lRow, true, SS_BORDERTYPE_BOTTOM, SS_BORDERSTYLE_SOLID );

				// Set the group as no breakable (for print).
				pclSheet->SetFlagRowNoBreakable( lRowFirstRowNoBreakable, lRow, true );
			}

			// Fill secondary vessel if exist.
			if( _NULL_IDPTR != pclSSelPressMaint->GetSecondaryVesselIDPtr() )
			{
				lRow++;
				lRowFirstRowNoBreakable = lRow;
				pclVessel = (CDB_Vessel *)( pclSSelPressMaint->GetSecondaryVesselIDPtr().MP );
				iQuantity = pclSSelPressMaint->GetSecondaryVesselNumber() * pclSSelPressMaint->GetpSelectedInfos()->GetQuantity();
				lRow = _FillRowVesselProduct( pclSDProduct, lRow, pclVessel, pclSSelPressMaint, iQuantity );

				// If secondary vessel accessories exist...
				if( clSecOnlyVesselAccessoryList.GetCount() > 0 )
				{
					// Draw dash line.
					pclSheet->SetCellBorder( CD_CPMP_Reference1, lRow, CD_CPMP_Footer - 1, lRow, true, SS_BORDERTYPE_BOTTOM, SS_BORDERSTYLE_FINE_DASH );

					// Attention: this 'method' doesn't return the next row but the last row!!
					lRow++;
					lRow = FillAccessories( pclSDProduct, lRow, &clSecOnlyVesselAccessoryList, pclSSelPressMaint->GetpSelectedInfos()->GetQuantity(), 1, &m_clCPMPParam );
				}

				// Draw solid line.
				pclSheet->SetCellBorder( CD_CPMP_Reference1, lRow, CD_CPMP_Footer - 1, lRow, true, SS_BORDERTYPE_BOTTOM, SS_BORDERSTYLE_SOLID );

				// Add primary & secondary vessel accessories if exist.
				if( clPrimAndSecVesselAccessoryList.GetCount() > 0 )
				{
					lRow++;
					pclSheet->SetTextPatternProperty( CSSheet::_SSTextPatternProperty::TPP_FontBold, (LPARAM)TRUE );
					lRow = AddStaticText( pclSDProduct, CD_CPMP_Product, lRow, TASApp.LoadLocalizedString( IDS_SSHEETSSELPROD_CPM_PRIMSECVESSLACC ) );
					pclSheet->SetTextPatternProperty( CSSheet::_SSTextPatternProperty::TPP_FontBold, (LPARAM)FALSE );

					// Attention: this 'method' doesn't return the next row but the last row!!
					lRow = FillAccessories( pclSDProduct, lRow, &clPrimAndSecVesselAccessoryList, pclSSelPressMaint->GetpSelectedInfos()->GetQuantity(), 1, &m_clCPMPParam );
					m_clCPMPParam.UpdateRange( lRow, lRow, lRow - 1, lRow );

					// Draw solid line.
					pclSheet->SetCellBorder( CD_CPMP_Reference1, lRow, CD_CPMP_Footer - 1, lRow, true, SS_BORDERTYPE_BOTTOM, SS_BORDERSTYLE_SOLID );
				}

				// Set the group as no breakable (for print).
				pclSheet->SetFlagRowNoBreakable( lRowFirstRowNoBreakable, lRow, true );
			}

			// Add intermediate vessel if exist.
			IDPTR IntermVesselIDPtr = pclSSelPressMaint->GetIntermediateVesselIDPtr();

			if( _NULL_IDPTR != IntermVesselIDPtr && NULL != dynamic_cast<CDB_Product *>( IntermVesselIDPtr.MP ) )
			{
				// Intermediate vessel is in fact a Statico.
				lRow++;
				lRowFirstRowNoBreakable = lRow;
				int iQuantity = pclSSelPressMaint->GetIntermediateVesselNumber() * pclSSelPressMaint->GetpSelectedInfos()->GetQuantity();
				lRow = _FillRowVesselProduct( pclSDProduct, lRow, dynamic_cast<CDB_Vessel *>( IntermVesselIDPtr.MP ), pclSSelPressMaint, iQuantity );

				// Add accessory if exist.
				if( NULL != pclSSelPressMaint->GetIntermediateVesselAccessoryList() && pclSSelPressMaint->GetIntermediateVesselAccessoryList()->GetCount() > 0 )
				{
					// Draw dash line.
					pclSheet->SetCellBorder( CD_CPMP_Reference1, lRow, CD_CPMP_Footer - 1, lRow, true, SS_BORDERTYPE_BOTTOM, SS_BORDERSTYLE_FINE_DASH );

					// Attention: this 'method' doesn't return the next row but the last row!!
					lRow++;
					int iDistributedQty = pclSSelPressMaint->GetIntermediateVesselNumber();
					lRow = FillAccessories( pclSDProduct, lRow, pclSSelPressMaint->GetIntermediateVesselAccessoryList(), pclSSelPressMaint->GetpSelectedInfos()->GetQuantity(),
											iDistributedQty, &m_clCPMPParam );
				}

				pclSheet->SetCellBorder( CD_CPMP_Reference1, lRow, CD_CPMP_Remark, lRow, true, SS_BORDERTYPE_BOTTOM, SS_BORDERSTYLE_SOLID );

				// Set the group as no breakable (for print).
				pclSheet->SetFlagRowNoBreakable( lRowFirstRowNoBreakable, lRow, true );
			}

			// Fill Vento if exist.
			if( NULL != pclSSelPressMaint->GetTecBoxVentoIDPtr().MP )
			{
				lRow++;
				lRowFirstRowNoBreakable = lRow;
				lRow = _FillRowTecBoxVentoProduct( pclSDProduct, lRow, pclSSelPressMaint, true );

				// Set the group as no breakable (for print).
				pclSheet->SetFlagRowNoBreakable( lRowFirstRowNoBreakable, lRow, true );
			}

			// Fill Pleno if exist.
			if( NULL != pclSSelPressMaint->GetTecBoxPlenoIDPtr().MP || NULL != pclSSelPressMaint->GetAdditionalTecBoxPlenoIDPtr().MP )
			{
				lRow++;
				lRowFirstRowNoBreakable = lRow;
				lRow = _FillRowTecBoxPlenoProduct( pclSDProduct, lRow, pclSSelPressMaint, true );

				// Set the group as no breakable (for print).
				pclSheet->SetFlagRowNoBreakable( lRowFirstRowNoBreakable, lRow, true );
			}

			// Fill Pleno refill if exist.
			if( NULL != pclSSelPressMaint->GetPlenoRefillIDPtr().MP )
			{
				lRow++;
				lRowFirstRowNoBreakable = lRow;
				lRow = _FillRowPlenoRefillProduct( pclSDProduct, lRow, pclSSelPressMaint, true, false );

				// Set the group as no breakable (for print).
				pclSheet->SetFlagRowNoBreakable( lRowFirstRowNoBreakable, lRow, true );
			}

			lRow++;
		}
	}

	// Add the product in the 'Article list'.
	_AddArticleList( pclSSelPressMaint );

	// Set the selection range.
	lRow = pclSheet->GetMaxRows();
	m_clCPMPParam.m_pclSheetDescription = pclSDProduct;
	m_clCPMPParam.m_lpSelectionContainer = (LPARAM)pclSSelPressMaint;

	// Set the scrolling range (for the scrolling, we include the title)
	m_clCPMPParam.SetScrollRange( RD_CPMP_Title, lRow );
}

void CSelProdPagePressMaint::_InitDirectSel( void )
{
	// Call base class method to get a new 'Sheet'.
	CSheetDescription *pclSDDirectSel = CMultiSpreadBase::CreateSSheet( SD_DirSelPressMaint );

	if( NULL == pclSDDirectSel || NULL == pclSDDirectSel->GetSSheetPointer() )
	{
		return;
	}

	CSSheet *pclSheet = pclSDDirectSel->GetSSheetPointer();

	if( false == CSelProdPageBase::PrepareSheet( pclSDDirectSel, CD_CPMP_Pointer, m_bForPrint, false ) )
	{
		m_ViewDescription.RemoveOneSheetDescription( SD_DirSelPressMaint );
		return;
	}

	// Initialize column width for header and footer column.
	if( false == m_bForPrint )
	{
		SetColWidth( pclSDDirectSel, CD_CPMP_Header, DefaultColumnWidth::DCW_Header );
		SetColWidth( pclSDDirectSel, CD_CPMP_Footer, DefaultColumnWidth::DCW_Footer );
	}
	else
	{
		pclSheet->ShowCol( CD_CPMP_Header, FALSE );
		pclSheet->ShowCol( CD_CPMP_Footer, FALSE );
	}

	pclSheet->SetMaxRows( RD_DSPM_ColName );
	pclSheet->SetRowHeight( RD_DSPM_ColName, m_dRowHeight * 1.6 );
	pclSheet->SetRowHeight( RD_DSPM_FirstRow, m_dRowHeight * 0.5 );

	// Set the column width. To perfectly match the main title, we use proportionality.
	int iHFColPixels = ( false == m_bForPrint ) ? pclSheet->ColWidthToLogUnits( 2.0 ) : 0;
	double dAvailableWidth = pclSheet->LogUnitsToColWidthW( RIGHTVIEWWIDTH - 2 * iHFColPixels );

	// To adapt column width, we take default defined width and do a proportionality.
	double dTotalColWidth = DefaultColumnWidth::DCW_Reference1 + DefaultColumnWidth::DCW_Reference2 + DefaultColumnWidth::DCW_Product;
	dTotalColWidth += DefaultColumnWidth::DCW_ArticleNumber + DefaultColumnWidth::DCW_Quantity + DefaultColumnWidth::DCW_Remark;

	if( true == TASApp.IsPriceUsed() )
	{
		dTotalColWidth += DefaultColumnWidth::DCW_UnitPrice;
		dTotalColWidth += DefaultColumnWidth::DCW_TotalPrice;
	}

	double dWidth = dAvailableWidth * DefaultColumnWidth::DCW_Reference1 / dTotalColWidth;
	double dTotalWidth = dWidth;
	SetColWidth( pclSDDirectSel, CD_CPMP_Reference1, dWidth );

	dWidth = dAvailableWidth * DefaultColumnWidth::DCW_Reference2 / dTotalColWidth;
	dTotalWidth += dWidth;
	SetColWidth( pclSDDirectSel, CD_CPMP_Reference2, dWidth );

	dWidth = dAvailableWidth * DefaultColumnWidth::DCW_Product / dTotalColWidth;
	dTotalWidth += dWidth;
	SetColWidth( pclSDDirectSel, CD_CPMP_Product, dWidth );

	dWidth = dAvailableWidth * DefaultColumnWidth::DCW_ArticleNumber / dTotalColWidth;
	dTotalWidth += dWidth;
	SetColWidth( pclSDDirectSel, CD_CPMP_ArticleNumber, dWidth );

	dWidth = dAvailableWidth * DefaultColumnWidth::DCW_Quantity / dTotalColWidth;
	dTotalWidth += dWidth;
	SetColWidth( pclSDDirectSel, CD_CPMP_Quantity, dWidth );

	if( true == TASApp.IsPriceUsed() )
	{
		dWidth = dAvailableWidth * DefaultColumnWidth::DCW_UnitPrice / dTotalColWidth;
		dTotalWidth += dWidth;
		SetColWidth( pclSDDirectSel, CD_CPMP_UnitPrice, dWidth );

		dWidth = dAvailableWidth * DefaultColumnWidth::DCW_TotalPrice / dTotalColWidth;
		dTotalWidth += dWidth;
		SetColWidth( pclSDDirectSel, CD_CPMP_TotalPrice, dWidth );
	}

	SetColWidth( pclSDDirectSel, CD_CPMP_Remark, dAvailableWidth - dTotalWidth );

	if( false == TASApp.IsPriceUsed() )
	{
		pclSheet->ShowCol( CD_CPMP_UnitPrice, FALSE );
		pclSheet->ShowCol( CD_CPMP_TotalPrice, FALSE );
	}

	pclSheet->ShowCol( CD_CPMP_Pointer, FALSE );

	// HYS-1571: We can optimize the column width now.
	// See Jira card to have more details about why we do that here.
	CRect rectClient;
	GetClientRect( &rectClient );
	OptimizePageSize( rectClient.Width(), rectClient.Height() );

	CRect rect = pclSheet->GetSheetSizeInPixels( false );
	m_mapSheetInfos[pclSDDirectSel].m_dPageWidth = rect.Width();

	// Set the subtitle.
	SetPageTitle( pclSDDirectSel, IDS_SSHEETSELPROD_SUBTITLEPRESSMAINTFROMDIRSEL, false, RD_DSPM_GroupName, m_dRowHeight * 1.8 );


	//////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
	// Init column headers.
	pclSheet->SetTextPattern( CSSheet::_SSTextPattern::ColumnHeader );
	pclSheet->SetTextPatternProperty( CSSheet::_SSTextPatternProperty::TPP_FontBackColor, (LPARAM)_TAH_WHITE );
	pclSheet->SetTextPatternProperty( CSSheet::_SSTextPatternProperty::TPP_StyleWordWrap, (LPARAM)TRUE );

	// Remark: 'CPM' is for 'Computed Pressure Maintenance' and 'PH' is for 'Product Header'.
	AddStaticText( pclSDDirectSel, CD_CPMP_Reference1, RD_DSPM_ColName, TASApp.LoadLocalizedString( IDS_SELPHDR_REF1 ) );
	AddStaticText( pclSDDirectSel, CD_CPMP_Reference2, RD_DSPM_ColName, TASApp.LoadLocalizedString( IDS_SELPHDR_REF2 ) );
	AddStaticText( pclSDDirectSel, CD_CPMP_Product, RD_DSPM_ColName, TASApp.LoadLocalizedString( IDS_SELPHDR_PRODUCT ) );
	AddStaticText( pclSDDirectSel, CD_CPMP_ArticleNumber, RD_DSPM_ColName, TASApp.LoadLocalizedString( IDS_SELPHDR_ART ) );
	AddStaticText( pclSDDirectSel, CD_CPMP_Quantity, RD_DSPM_ColName, TASApp.LoadLocalizedString( IDS_SELPHDR_QTY ) );

	if( true == TASApp.IsPriceUsed() )
	{
		CString str = TASApp.LoadLocalizedString( IDS_SELPHDR_PRICE ) + _T("\r\n[");
		str += CString( m_pTADS->GetpTechParams()->GetCurrentCurrencyISO().c_str() ) + _T("]");
		AddStaticText( pclSDDirectSel, CD_CPMP_UnitPrice, RD_DSPM_ColName, str );

		str = TASApp.LoadLocalizedString( IDS_SELPHDR_TOTAL ) + _T("\r\n[");
		str += CString( m_pTADS->GetpTechParams()->GetCurrentCurrencyISO().c_str() ) + _T("]");
		AddStaticText( pclSDDirectSel, CD_CPMP_TotalPrice, RD_DSPM_ColName, str );

		pclSheet->SetTextPatternProperty( CSSheet::_SSTextPatternProperty::TPP_StyleWordWrap, (LPARAM)FALSE );
	}

	AddStaticText( pclSDDirectSel, CD_CPMP_Remark, RD_DSPM_ColName, TASApp.LoadLocalizedString( IDS_SELPHDR_REM ) );

	// Draw border.
	pclSheet->SetCellBorder( CD_CPMP_Reference1, RD_DSPM_ColName, CD_CPMP_Remark, RD_DSPM_ColName, true,
							 SS_BORDERTYPE_BOTTOM | SS_BORDERTYPE_TOP );
	//////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
}

void CSelProdPagePressMaint::_FillDirectSel( CDS_SSelPMaint *pclSSelPressMaint, bool fFirstPass )
{
	if( NULL == pclSSelPressMaint )
	{
		return;
	}

	CSheetDescription *pclSheetDescription = m_ViewDescription.GetFromSheetDescriptionID( SD_DirSelPressMaint );

	if( NULL == pclSheetDescription || NULL == pclSheetDescription->GetSSheetPointer() )
	{
		return;
	}

	CSSheet *pclSheet = pclSheetDescription->GetSSheetPointer();
	long lFirstRow = pclSheet->GetMaxRows() + 1;

	//////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
	// Fill product details.
	CDB_Product *pclProductToSave = NULL;
	CAccessoryList *pclAccessoryList = NULL;
	IDPTR ProductIDPtr = _NULL_IDPTR;
	bool fCallFillRowGen = false;

	if( NULL != dynamic_cast<CDB_Vessel *>( pclSSelPressMaint->GetVesselIDPtr().MP ) ||
		NULL != dynamic_cast<CDB_Vessel *>( pclSSelPressMaint->GetSecondaryVesselIDPtr().MP ) ||
		NULL != dynamic_cast<CDB_Vessel *>( pclSSelPressMaint->GetIntermediateVesselIDPtr().MP ) )
	{
		if( NULL != pclSSelPressMaint->GetVesselIDPtr().MP )
		{
			pclProductToSave = dynamic_cast<CDB_Vessel *>( pclSSelPressMaint->GetVesselIDPtr().MP );
		}
		else if( NULL != pclSSelPressMaint->GetSecondaryVesselIDPtr().MP )
		{
			pclProductToSave = dynamic_cast<CDB_Vessel *>( pclSSelPressMaint->GetSecondaryVesselIDPtr().MP );
		}
		else
		{
			pclProductToSave = dynamic_cast<CDB_Vessel *>( pclSSelPressMaint->GetIntermediateVesselIDPtr().MP );
		}

		_FillRowVesselProduct( pclSheetDescription, lFirstRow, (CDB_Vessel *)pclProductToSave, pclSSelPressMaint, 
								pclSSelPressMaint->GetpSelectedInfos()->GetQuantity(), false, &m_clDSPMParam );
		
		pclAccessoryList = pclSSelPressMaint->GetVesselAccessoryList();
		fCallFillRowGen = true;
	}
	else if( NULL != dynamic_cast<CDB_TecBox *>( pclSSelPressMaint->GetTecBoxCompTransfIDPtr().MP ) )
	{
		pclProductToSave = dynamic_cast<CDB_TecBox *>( pclSSelPressMaint->GetTecBoxCompTransfIDPtr().MP );
		
		long lLastRow = _FillRowCprssoTrsfroProduct( pclSheetDescription, lFirstRow, pclSSelPressMaint, false, &m_clDSPMParam );
		// Accessories are filled in the '_FillRowCprssoTrsfroProduct' method.

		if( _NULL_IDPTR != pclSSelPressMaint->GetTecBoxIntegratedVesselIDPtr() )
		{
			pclSheet->SetCellBorder( CD_CPMP_Reference1, lLastRow, CD_CPMP_Remark, lLastRow, true, SS_BORDERTYPE_BOTTOM, SS_BORDERSTYLE_FINE_DASH );
			lLastRow++;
			// HYS-945: Buffer vessel quantity and accessories
			int iQuantity = pclSSelPressMaint->GetTecBoxCompTransfNumber() * pclSSelPressMaint->GetpSelectedInfos()->GetQuantity();
			long lRowFirstRowNoBreakable = lLastRow;
			CDB_Vessel *pclIntegratedVessel = (CDB_Vessel *)( pclSSelPressMaint->GetTecBoxIntegratedVesselIDPtr().MP );
			lLastRow = _FillRowVesselProduct( pclSheetDescription, lLastRow, pclIntegratedVessel, pclSSelPressMaint, iQuantity, true );
			pclAccessoryList = pclSSelPressMaint->GetTecBoxIntegratedVesselAccessoryList();
			// Set the group as no breakable (for print).
			pclSheet->SetFlagRowNoBreakable( lRowFirstRowNoBreakable, lLastRow, true );	
			m_clDSPMParam.AddRange( lRowFirstRowNoBreakable, lLastRow, pclIntegratedVessel );
		}
	}
	else if( NULL != dynamic_cast<CDB_TBPlenoVento *>( pclSSelPressMaint->GetTecBoxVentoIDPtr().MP ) )
	{
		pclProductToSave = dynamic_cast<CDB_TBPlenoVento *>( pclSSelPressMaint->GetTecBoxVentoIDPtr().MP );
		_FillRowTecBoxVentoProduct( pclSheetDescription, lFirstRow, pclSSelPressMaint, false, &m_clDSPMParam );
		// Accessories are filled in the '_FillRowCprssoTrsfroProduct' method.
		fCallFillRowGen = true;
	}
	else if( NULL != dynamic_cast<CDB_TBPlenoVento *>( pclSSelPressMaint->GetTecBoxPlenoIDPtr().MP ) )
	{
		pclProductToSave = dynamic_cast<CDB_TBPlenoVento *>( pclSSelPressMaint->GetTecBoxPlenoIDPtr().MP );
		_FillRowTecBoxPlenoProduct( pclSheetDescription, lFirstRow, pclSSelPressMaint, false, true, &m_clDSPMParam );
		// Accessories are filled in the '_FillRowCprssoTrsfroProduct' method.
	}
	else if( NULL != dynamic_cast<CDB_PlenoRefill *>( pclSSelPressMaint->GetPlenoRefillIDPtr().MP ) )
	{
		pclProductToSave = dynamic_cast<CDB_PlenoRefill *>( pclSSelPressMaint->GetPlenoRefillIDPtr().MP );
		_FillRowPlenoRefillProduct( pclSheetDescription, lFirstRow, pclSSelPressMaint, false, true, &m_clDSPMParam );
		// Accessories are filled in the '_FillRowCprssoTrsfroProduct' method.
	}

	//////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

	// Fill references and remarks.
	long lRow = pclSheet->GetMaxRows();

	if( true == fCallFillRowGen )
	{
		_FillRowGen( pclSheetDescription, pclSSelPressMaint, lFirstRow, lRow );
	}

	// Add accessories.
	lRow = pclSheet->GetMaxRows();

	if( NULL != pclAccessoryList )
	{
		// Draw dash line.
		pclSheet->SetCellBorder( CD_CPMP_Reference1, lRow, CD_CPMP_Footer - 1, lRow, true, SS_BORDERTYPE_BOTTOM, SS_BORDERSTYLE_FINE_DASH );

		lRow++;
		lRow = FillAccessories( pclSheetDescription, lRow, pclAccessoryList, pclSSelPressMaint->GetpSelectedInfos()->GetQuantity(), 1, &m_clDSPMParam );
	}

	pclSheet->SetCellBorder( CD_CPMP_Reference1, lRow, CD_CPMP_Remark, lRow, true, SS_BORDERTYPE_BOTTOM, SS_BORDERSTYLE_SOLID );

	// Add the product in the 'Article list'.
	_AddArticleList( pclSSelPressMaint );

	// Set the selection range.
	lRow = pclSheet->GetMaxRows();
	m_clDSPMParam.m_pclSheetDescription = pclSheetDescription;
	m_clDSPMParam.m_lpSelectionContainer = (LPARAM)pclSSelPressMaint;

	// Set the group as no breakable (for print).
	// Remark: include titles with the group if it's the first pass.
	if( true == fFirstPass )
	{
		pclSheet->SetFlagRowNoBreakable( RD_DSPM_GroupName, lRow, true );
	}
	else
	{
		pclSheet->SetFlagRowNoBreakable( lFirstRow, lRow, true );
	}
}

void CSelProdPagePressMaint::_InitPlenoVentoIndSel( void )
{
	// Call base class method to get a new 'Sheet'.
	CSheetDescription *pclSDPlenoVentoIndSel = CMultiSpreadBase::CreateSSheet( SD_PlenoVentoIndSel );

	if( NULL == pclSDPlenoVentoIndSel || NULL == pclSDPlenoVentoIndSel->GetSSheetPointer() )
	{
		return;
	}

	CSSheet *pclSheet = pclSDPlenoVentoIndSel->GetSSheetPointer();

	if( false == CSelProdPageBase::PrepareSheet( pclSDPlenoVentoIndSel, CD_CPMP_Pointer, m_bForPrint, false ) )
	{
		m_ViewDescription.RemoveOneSheetDescription( SD_PlenoVentoIndSel );
		return;
	}

	// Initialize column width for header and footer column.
	if( false == m_bForPrint )
	{
		SetColWidth( pclSDPlenoVentoIndSel, CD_CPMP_Header, DefaultColumnWidth::DCW_Header );
		SetColWidth( pclSDPlenoVentoIndSel, CD_CPMP_Footer, DefaultColumnWidth::DCW_Footer );
	}
	else
	{
		pclSheet->ShowCol( CD_CPMP_Header, FALSE );
		pclSheet->ShowCol( CD_CPMP_Footer, FALSE );
	}

	pclSheet->SetMaxRows( RD_DSPM_ColName );
	pclSheet->SetRowHeight( RD_DSPM_FirstRow, m_dRowHeight * 0.5 );

	// Set the column width. To perfectly match the main title, we use proportionality.
	int iHFColPixels = ( false == m_bForPrint ) ? pclSheet->ColWidthToLogUnits( 2.0 ) : 0;
	double dAvailableWidth = pclSheet->LogUnitsToColWidthW( RIGHTVIEWWIDTH - 2 * iHFColPixels );

	// To adapt column width, we take default defined width and do a proportionality.
	double dTotalColWidth = PMColumnWidth::PMCW_Reference1 + PMColumnWidth::PMCW_Reference2 + PMColumnWidth::PMCW_Product;
	dTotalColWidth += PMColumnWidth::PMCW_ArticleNumber + PMColumnWidth::PMCW_Quantity + PMColumnWidth::PMCW_Remark;

	if( true == TASApp.IsPriceUsed() )
	{
		dTotalColWidth += PMColumnWidth::PMCW_UnitPrice;
		dTotalColWidth += PMColumnWidth::PMCW_TotalPrice;
	}

	double dWidth = dAvailableWidth * PMColumnWidth::PMCW_Reference1 / dTotalColWidth;
	double dTotalWidth = dWidth;
	SetColWidth( pclSDPlenoVentoIndSel, CD_CPMP_Reference1, dWidth );

	dWidth = dAvailableWidth * PMColumnWidth::PMCW_Reference2 / dTotalColWidth;
	dTotalWidth += dWidth;
	SetColWidth( pclSDPlenoVentoIndSel, CD_CPMP_Reference2, dWidth );

	dWidth = dAvailableWidth * PMColumnWidth::PMCW_Product / dTotalColWidth;
	dTotalWidth += dWidth;
	SetColWidth( pclSDPlenoVentoIndSel, CD_CPMP_Product, dWidth );

	dWidth = dAvailableWidth * PMColumnWidth::PMCW_ArticleNumber / dTotalColWidth;
	dTotalWidth += dWidth;
	SetColWidth( pclSDPlenoVentoIndSel, CD_CPMP_ArticleNumber, dWidth );

	dWidth = dAvailableWidth * PMColumnWidth::PMCW_Quantity / dTotalColWidth;
	dTotalWidth += dWidth;
	SetColWidth( pclSDPlenoVentoIndSel, CD_CPMP_Quantity, dWidth );

	if( true == TASApp.IsPriceUsed() )
	{
		dWidth = dAvailableWidth * PMColumnWidth::PMCW_UnitPrice / dTotalColWidth;
		dTotalWidth += dWidth;
		SetColWidth( pclSDPlenoVentoIndSel, CD_CPMP_UnitPrice, dWidth );

		dWidth = dAvailableWidth * PMColumnWidth::PMCW_TotalPrice / dTotalColWidth;
		dTotalWidth += dWidth;
		SetColWidth( pclSDPlenoVentoIndSel, CD_CPMP_TotalPrice, dWidth );
	}

	SetColWidth( pclSDPlenoVentoIndSel, CD_CPMP_Remark, dAvailableWidth - dTotalWidth );

	if( false == TASApp.IsPriceUsed() )
	{
		pclSheet->ShowCol( CD_CPMP_UnitPrice, FALSE );
		pclSheet->ShowCol( CD_CPMP_TotalPrice, FALSE );
	}

	pclSheet->ShowCol( CD_CPMP_Pointer, FALSE );

	// HYS-1571: We can optimize the column width now.
	// See Jira card to have more details about why we do that here.
	CRect rectClient;
	GetClientRect( &rectClient );
	OptimizePageSize( rectClient.Width(), rectClient.Height() );

	CRect rect = pclSheet->GetSheetSizeInPixels( false );
	m_mapSheetInfos[pclSDPlenoVentoIndSel].m_dPageWidth = rect.Width();

	// Set the subtitle.
	// Remark: 'PV' is for Pleno/Vento'.
	SetPageTitle( pclSDPlenoVentoIndSel, IDS_SSHEETSELPROD_PV_INDSEL_TITLE, false, RD_DSPM_GroupName, m_dRowHeight * 1.8 );


	//////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
	// Init column headers.
	pclSheet->SetTextPattern( CSSheet::_SSTextPattern::ColumnHeader );
	pclSheet->SetTextPatternProperty( CSSheet::_SSTextPatternProperty::TPP_FontBackColor, (LPARAM)_TAH_WHITE );
	pclSheet->SetTextPatternProperty( CSSheet::_SSTextPatternProperty::TPP_StyleWordWrap, (LPARAM)TRUE );

	// Remark: 'DS' is for 'Direct selection' and 'PM' is for 'Product Maintenance'.
	AddStaticText( pclSDPlenoVentoIndSel, CD_CPMP_Reference1, RD_DSPM_ColName, TASApp.LoadLocalizedString( IDS_SELPHDR_REF1 ) );
	AddStaticText( pclSDPlenoVentoIndSel, CD_CPMP_Reference2, RD_DSPM_ColName, TASApp.LoadLocalizedString( IDS_SELPHDR_REF2 ) );
	AddStaticText( pclSDPlenoVentoIndSel, CD_CPMP_Product, RD_DSPM_ColName, TASApp.LoadLocalizedString( IDS_SELPHDR_PRODUCT ) );
	AddStaticText( pclSDPlenoVentoIndSel, CD_CPMP_ArticleNumber, RD_DSPM_ColName, TASApp.LoadLocalizedString( IDS_SELPHDR_ART ) );
	AddStaticText( pclSDPlenoVentoIndSel, CD_CPMP_Quantity, RD_DSPM_ColName, TASApp.LoadLocalizedString( IDS_SELPHDR_QTY ) );

	if( true == TASApp.IsPriceUsed() )
	{
		CString str = TASApp.LoadLocalizedString( IDS_SELPHDR_PRICE ) + _T("\r\n[");
		str += CString( m_pTADS->GetpTechParams()->GetCurrentCurrencyISO().c_str() ) + _T("]");
		AddStaticText( pclSDPlenoVentoIndSel, CD_CPMP_UnitPrice, RD_DSPM_ColName, str );

		str = TASApp.LoadLocalizedString( IDS_SELPHDR_TOTAL ) + _T("\r\n[");
		str += CString( m_pTADS->GetpTechParams()->GetCurrentCurrencyISO().c_str() ) + _T("]");
		AddStaticText( pclSDPlenoVentoIndSel, CD_CPMP_TotalPrice, RD_DSPM_ColName, str );
	}

	AddStaticText( pclSDPlenoVentoIndSel, CD_CPMP_Remark, RD_DSPM_ColName, TASApp.LoadLocalizedString( IDS_SELPHDR_REM ) );
	pclSheet->SetRowHeight( RD_DSPM_ColName, m_dRowHeight * 1.6 );

	// Draw border.
	pclSheet->SetCellBorder( CD_CPMP_Reference1, RD_DSPM_ColName, CD_CPMP_Remark, RD_DSPM_ColName, true,
							 SS_BORDERTYPE_BOTTOM | SS_BORDERTYPE_TOP );
	//////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
}

void CSelProdPagePressMaint::_FillPlenoVentoIndSel( CDS_SSelPMaint *pclSSelPressMaint, bool fFirstPass )
{
	if( NULL == pclSSelPressMaint )
	{
		return;
	}

	CSheetDescription *pclSheetDescription = m_ViewDescription.GetFromSheetDescriptionID( SD_PlenoVentoIndSel );

	if( NULL == pclSheetDescription || NULL == pclSheetDescription->GetSSheetPointer() )
	{
		return;
	}

	CSSheet *pclSheet = pclSheetDescription->GetSSheetPointer();
	long lFirstRow = pclSheet->GetMaxRows() + 1;
	long lRow = lFirstRow;

	bool fSomethingDisplayed = false;

	// Show Vento if exist.
	if( NULL != dynamic_cast<CDB_TBPlenoVento *>( pclSSelPressMaint->GetTecBoxVentoIDPtr().MP ) )
	{
		CDB_TBPlenoVento *pclTecBoxVento = dynamic_cast<CDB_TBPlenoVento *>( pclSSelPressMaint->GetTecBoxVentoIDPtr().MP );
		// Pay attention: the 'lRow' returns is the last row written and not the next available row!!
		lRow = _FillRowTecBoxVentoProduct( pclSheetDescription, lRow, pclSSelPressMaint, false, &m_clPVISParam );
		fSomethingDisplayed = true;

		// Add Vento accessories if exist.
		CAccessoryList *pclAccessoryList = pclSSelPressMaint->GetTecBoxVentoAccessoryList();

		if( NULL != pclAccessoryList && pclAccessoryList->GetCount() > 0 )
		{
			// Draw dash line.
			pclSheet->SetCellBorder( CD_CPMP_Reference1, lRow, CD_CPMP_Footer - 1, lRow, true, SS_BORDERTYPE_BOTTOM, SS_BORDERSTYLE_FINE_DASH );

			lRow++;
			lRow = FillAccessories( pclSheetDescription, lRow, pclAccessoryList, pclSSelPressMaint->GetpSelectedInfos()->GetQuantity(), 1, &m_clPVISParam );
		}

		// Fill references and remarks.
		_FillRowGen( pclSheetDescription, pclSSelPressMaint, lFirstRow, lRow );
	}

	// Show Pleno if exist.
	if( NULL != dynamic_cast<CDB_TBPlenoVento *>( pclSSelPressMaint->GetTecBoxPlenoIDPtr().MP ) ||
		NULL != dynamic_cast<CDB_Set*>( pclSSelPressMaint->GetAdditionalTecBoxPlenoIDPtr().MP ) )
	{
		if( true == fSomethingDisplayed )
		{
			lRow++;
		}
		// HYS-1121: _FillRowGen is called in _FillRowTecBoxPlenoProduct
		bool bFillRowGen = false;
		if( false == fSomethingDisplayed )
		{
			// Fill references and remarks.
			bFillRowGen = true;
		}
		// Pay attention: the 'lRow' returns is the last row written and not the next available row!!
		lRow = _FillRowTecBoxPlenoProduct( pclSheetDescription, lRow, pclSSelPressMaint, false, bFillRowGen, &m_clPVISParam );
	}

	// Check if there is a Pleno Refill.
	if( NULL != dynamic_cast<CDB_PlenoRefill *>( pclSSelPressMaint->GetPlenoRefillIDPtr().MP ) )
	{
		lRow++;
		lRow = _FillRowPlenoRefillProduct( pclSheetDescription, lRow, pclSSelPressMaint, false, false, &m_clPVISParam );
	}

	pclSheet->SetCellBorder( CD_CPMP_Reference1, lRow, CD_CPMP_Remark, lRow, true, SS_BORDERTYPE_BOTTOM, SS_BORDERSTYLE_SOLID );

	// Add the product in the 'Article list'.
	_AddArticleList( pclSSelPressMaint );

	// Set the selection range.
	lRow = pclSheet->GetMaxRows();
	m_clPVISParam.m_pclSheetDescription = pclSheetDescription;
	m_clPVISParam.m_lpSelectionContainer = (LPARAM)pclSSelPressMaint;

	// Set the group as no breakable (for print).
	// Remark: include titles with the group if it's the first pass.
	if( true == fFirstPass )
	{
		pclSheet->SetFlagRowNoBreakable( RD_DSPM_FirstAvailRow, lRow, true );
	}
	else
	{
		pclSheet->SetFlagRowNoBreakable( lFirstRow, lRow, true );
	}
}

long CSelProdPagePressMaint::_FillRowVesselProduct( CSheetDescription *pclSheetDescription, long lRow, CDB_Vessel *pclVessel, CDS_SSelPMaint *pclSSelPressMaint, int iQuantity,
		bool bIntegratedVessel, CProductParam *pclProductParam )
{
	if( NULL == pclSheetDescription || NULL == pclSheetDescription->GetSSheetPointer() || NULL == pclVessel 
			|| NULL == pclSSelPressMaint || NULL == pclSSelPressMaint->GetpInputUser() || 0 == iQuantity )
	{
		return lRow;
	}

	if( NULL == pclProductParam )
	{
		pclProductParam = &m_clCPMPParam;
	}

	CSSheet *pclSheet = pclSheetDescription->GetSSheetPointer();

	long lFirstRow = lRow;
	pclSheet->SetTextPattern( CSSheet::_SSTextPattern::UserStaticColumn );

	CString str = _T( "" );

	if( false == bIntegratedVessel )
	{
		switch( pclVessel->GetVesselType() )
		{
			case CDB_Vessel::eVsslType_Statico:
				str = TASApp.LoadLocalizedString( IDS_SSHEETSSELPROD_CPM_PRODUCTSTATICO );
				break;

			case CDB_Vessel::eVsslType_StatMbr:
				str = TASApp.LoadLocalizedString( IDS_SSHEETSSELPROD_CPM_MEMBRANEVESSEL );
				break;

			case CDB_Vessel::eVsslType_Prim:
				str = TASApp.LoadLocalizedString( IDS_SSHEETSSELPROD_CPM_PRODUCTPVESSEL );
				break;

			case CDB_Vessel::eVsslType_Sec:
				str = TASApp.LoadLocalizedString( IDS_SSHEETSSELPROD_CPM_PRODUCTSVESSEL );
				break;

			case CDB_Vessel::eVsslType_Interm:
				str = TASApp.LoadLocalizedString( IDS_SSHEETSSELPROD_CPM_PRODUCTIVESSEL );
				break;

			case CDB_Vessel::eVsslType_Aqua:
				str = TASApp.LoadLocalizedString( IDS_CONFSEL_PRESSMAINT_PRODUCTAVESSEL );
				break;
		}
	}
	else
	{
		if( CDB_Product::eilIntegrated == pclVessel->GetInstallationLayout() )
		{
			str = TASApp.LoadLocalizedString( IDS_SSHEETSELPROD_CPM_INTEGRATEDBUFFERVESSEL );
		}
		else
		{
			str = TASApp.LoadLocalizedString( IDS_SSHEETSELPROD_CPM_BUFFERVESSEL );
		}
	}

	lRow = AddStaticText( pclSheetDescription, CD_CPMP_Product, lRow, str );
	pclSheet->SetTextPatternProperty( CSSheet::_SSTextPatternProperty::TPP_FontBold, (LPARAM)FALSE );

	// Column product.
	AddStaticText( pclSheetDescription, CD_CPMP_Product, lRow, _T("-") );

	if( true == m_pTADSPageSetup->GetField( epfSTATICOINFOPRODUCTNAME ) )
	{
		lRow = AddStaticText( pclSheetDescription, CD_CPMP_Product, lRow, pclVessel->GetName() );
	}

	if( true == m_pTADSPageSetup->GetField( epfSTATICOINFOPRODUCTCONNECTION ) )
	{
		str = TASApp.LoadLocalizedString( IDS_SSHEETSSELPROD_CPM_VESSELCON );
		str += CString( _T(" = ") ) + ( ( CDB_StringID * )( pclVessel->GetConnectIDPtr().MP ) )->GetString();
		lRow = AddStaticText( pclSheetDescription, CD_CPMP_Product, lRow, str );
	}

	if( true == m_pTADSPageSetup->GetField( epfSTATICOINFOPRODUCTNOMINALVOLUME ) )
	{
		str = TASApp.LoadLocalizedString( IDS_SSHEETSSELPROD_CPM_VESSELNOMVOL );
		str += CString( _T(" = ") ) + WriteCUDouble( _U_VOLUME, pclVessel->GetNominalVolume(), true );
		lRow = AddStaticText( pclSheetDescription, CD_CPMP_Product, lRow, str );
	}

	if( true == m_pTADSPageSetup->GetField( epfSTATICOINFOPRODUCTMAXPRESSURE ) )
	{
		bool bCHVersion = ( 0 == TASApp.GetTADBKey().CompareNoCase( _T("CH") ) ) ? true : false;
		int iID = ( true == bCHVersion ) ? IDS_SSHEETSSELPROD_CPM_VESSELMAXPRESSCH : IDS_SSHEETSSELPROD_CPM_VESSELMAXPRESS;
		str = TASApp.LoadLocalizedString( iID );

		double dPS = ( true == bCHVersion ) ? pclVessel->GetPSch() : pclVessel->GetPmaxmax();
		str += CString( _T(" = ") ) + WriteCUDouble( _U_PRESSURE, dPS, true );
		lRow = AddStaticText( pclSheetDescription, CD_CPMP_Product, lRow, str );
	}

	if( true == m_pTADSPageSetup->GetField( epfSTATICOINFOPRODUCTTEMPRANGE ) )
	{
		CUnitDatabase *pUnitDB = CDimValue::AccessUDB();
		str = TASApp.LoadLocalizedString( IDS_SSHEETSSELPROD_CPM_VESSELTEMPRANGE );
		str += CString( _T(" = " ) ) + pclVessel->GetTempRange() + _T( " ") + GetNameOf( pUnitDB->GetDefaultUnit( _U_TEMPERATURE ) ).c_str();
		lRow = AddStaticText( pclSheetDescription, CD_CPMP_Product, lRow, str );
	}

	// Weight.
	if( true == m_pTADSPageSetup->GetField( epfSTATICOINFOPRODUCTWEIGHT ) )
	{
		str = TASApp.LoadLocalizedString( IDS_SSHEETSSELPROD_CPM_VESSELWEIGHT );
		str += CString( _T(" = ") ) + WriteCUDouble( _U_MASS, pclVessel->GetWeight(), true );
		lRow = AddStaticText( pclSheetDescription, CD_CPMP_Product, lRow, str );
	}

	// Max weight.
	if( true == m_pTADSPageSetup->GetField( epfSTATICOINFOPRODUCTMAXWEIGHT ) )
	{
		str = TASApp.LoadLocalizedString( IDS_SSHEETSSELPROD_CPM_VESSELMAXWEIGHT );
		double dVesselMaxWeight = pclSSelPressMaint->GetpInputUser()->GetMaxWeight( pclVessel->GetWeight(), pclVessel->GetNominalVolume(),
			pclSSelPressMaint->GetpInputUser()->GetMinTemperature() );
		str += CString( _T(" = ") ) + WriteCUDouble( _U_MASS, dVesselMaxWeight, true );
		lRow = AddStaticText( pclSheetDescription, CD_CPMP_Product, lRow, str );
	}

	lRow--;

	// Column 'Quantity' and prices.
	_FillQtyPriceCol( pclSheetDescription, lFirstRow, pclVessel, iQuantity );

	// Column 'Article number'.
	AddStaticText( pclSheetDescription, CD_CPMP_ArticleNumber, lFirstRow, _T("-") );

	// Remark: 'true' to exclude any connection reference in the article number.
	FillArtNumberCol( pclSheetDescription, CD_CPMP_ArticleNumber, lFirstRow, pclVessel, pclVessel->GetArtNum( true ) );

	// Draw a solid line.
	pclSheet->SetCellBorder( CD_CPMP_Reference1, lRow, CD_CPMP_Remark, lRow, true, SS_BORDERTYPE_BOTTOM, SS_BORDERSTYLE_SOLID );

	pclProductParam->AddRange( lFirstRow, lRow, pclVessel );

	return pclSheet->GetMaxRows();
}

long CSelProdPagePressMaint::_FillRowCprssoTrsfroProduct( CSheetDescription *pclSheetDescription, long lRow, CDS_SSelPMaint *pclSSelPMaint,
		bool fSelIncludeColTitleIfNeeded, CProductParam *pclProductParam )
{
	if( NULL == pclSheetDescription || NULL == pclSheetDescription->GetSSheetPointer() || NULL == pclSSelPMaint
		|| _NULL_IDPTR == pclSSelPMaint->GetTecBoxCompTransfIDPtr() )
	{
		return lRow;
	}

	if( NULL == pclProductParam )
	{
		pclProductParam = &m_clCPMPParam;
	}

	long lFirstAvailRow = lRow;
	CSSheet *pclSheet = pclSheetDescription->GetSSheetPointer();
	CDB_TecBox *pclTechBox = (CDB_TecBox *)( pclSSelPMaint->GetTecBoxCompTransfIDPtr().MP );

	long lFirstRow = lRow;
	pclSheet->SetTextPattern( CSSheet::_SSTextPattern::UserStaticColumn );

	CString str;

	if( CDB_TecBox::etbtCompresso == pclTechBox->GetTecBoxType() )
	{
		str = TASApp.LoadLocalizedString( IDS_SSHEETSSELPROD_CPM_PRODUCTCPRSSO );
	}
	else
	{
		str = TASApp.LoadLocalizedString( IDS_SSHEETSSELPROD_CPM_PRODUCTTRSFRO );
	}

	str += _T(" (") + pclTechBox->GetFunctionsStr() + _T(")");
	lRow = AddStaticText( pclSheetDescription, CD_CPMP_Product, lRow, str );

	pclSheet->SetTextPatternProperty( CSSheet::_SSTextPatternProperty::TPP_FontBold, (LPARAM)FALSE );

	if( true == m_pTADSPageSetup->GetField( epfTECHBOXINFOPRODUCTNAME ) )
	{
		lRow = AddStaticText( pclSheetDescription, CD_CPMP_Product, lRow, pclTechBox->GetName() );
	}

	if( true == m_pTADSPageSetup->GetField( epfTECHBOXINFOPRODUCTPS ) )
	{
		str = TASApp.LoadLocalizedString( IDS_SSHEETSSELPROD_CPM_TECHBOXPS );
		str += CString( _T(" = ") ) + WriteCUDouble( _U_PRESSURE, pclTechBox->GetPmaxmax(), true );
		lRow = AddStaticText( pclSheetDescription, CD_CPMP_Product, lRow, str );
	}

	if( true == m_pTADSPageSetup->GetField( epfTECHBOXINFOPRODUCTPOWER ) )
	{
		str = TASApp.LoadLocalizedString( IDS_SSHEETSSELPROD_CPM_TECHBOXPOWER );
		str += CString( _T(" = ") ) + WriteCUDouble( _U_ELEC_POWER, pclTechBox->GetPower(), true );
		lRow = AddStaticText( pclSheetDescription, CD_CPMP_Product, lRow, str );
	}

	if( true == m_pTADSPageSetup->GetField( epfTECHBOXINFOPRODUCTSUPPLYVOLTAGE ) )
	{
		str = TASApp.LoadLocalizedString( IDS_SSHEETSSELPROD_CPM_TECHBOXSV );
		str += CString( _T(" = ") ) + pclTechBox->GetPowerSupplyStr();
		lRow = AddStaticText( pclSheetDescription, CD_CPMP_Product, lRow, str );
	}

	// Column 'Quantity' and prices.
	int iQuantity = pclSSelPMaint->GetTecBoxCompTransfNumber() * pclSSelPMaint->GetpSelectedInfos()->GetQuantity();
	_FillQtyPriceCol( pclSheetDescription, lFirstRow, pclTechBox, iQuantity );
	
	// Column 'Article number'.
	AddStaticText( pclSheetDescription, CD_CPMP_ArticleNumber, lFirstRow, _T("-") );

	// Remark: 'true' to exclude any connection reference in the article number.
	FillArtNumberCol( pclSheetDescription, CD_CPMP_ArticleNumber, lFirstRow, pclTechBox, pclTechBox->GetArtNum( true ) );

	// Fill references and remarks.
	lRow--;
	_FillRowGen( pclSheetDescription, pclSSelPMaint, lFirstAvailRow, lRow );

	// Add accessories.
	if( NULL != pclSSelPMaint->GetTecBoxCompTransfAccessoryList() && pclSSelPMaint->GetTecBoxCompTransfAccessoryList()->GetCount() > 0 )
	{
		// Draw dash line.
		pclSheet->SetCellBorder( CD_CPMP_Reference1, lRow, CD_CPMP_Footer - 1, lRow, true, SS_BORDERTYPE_BOTTOM, SS_BORDERSTYLE_FINE_DASH );

		// Attention: this 'method' doesn't return the next row but the last row!!
		lRow++;
		int iDistributedQty = pclSSelPMaint->GetTecBoxCompTransfNumber();
		lRow = FillAccessories( pclSheetDescription, lRow, pclSSelPMaint->GetTecBoxCompTransfAccessoryList(), pclSSelPMaint->GetpSelectedInfos()->GetQuantity(),
								iDistributedQty, pclProductParam );
	}

	// Draw a solid line.
	pclSheet->SetCellBorder( CD_CPMP_Reference1, lRow, CD_CPMP_Remark, lRow, true, SS_BORDERTYPE_BOTTOM, SS_BORDERSTYLE_SOLID );

	long lSelFirstRow = lFirstAvailRow;

	if( true == fSelIncludeColTitleIfNeeded )
	{
		lSelFirstRow = ( RD_CPMP_FirstAvailRow == lFirstAvailRow ) ? lFirstAvailRow - 1 : lFirstAvailRow;
	}

	pclProductParam->AddRange( lSelFirstRow, lRow, pclTechBox );
	return lRow;
}

long CSelProdPagePressMaint::_FillRowTecBoxPlenoProduct( CSheetDescription *pclSheetDescription, long lRow, CDS_SSelPMaint *pclSSelPMaint,
		bool fSelIncludeColTitleIfNeeded, bool fAddRemarks, CProductParam *pclProductParam )
{
	if( NULL == pclSheetDescription || NULL == pclSheetDescription->GetSSheetPointer() || NULL == pclSSelPMaint )
	{
		return lRow;
	}
	
	if( NULL == pclProductParam )
	{
		pclProductParam = &m_clCPMPParam;
	}

	CSSheet *pclSheet = pclSheetDescription->GetSSheetPointer();
	
	CDB_Set *pWTCombination = dynamic_cast<CDB_Set *>( (CData*)pclSSelPMaint->GetAdditionalTecBoxPlenoIDPtr().MP );
	int iLoopLimit = 1;

	if( NULL != pWTCombination )
	{
		iLoopLimit = 2;
	}

	long lFirstRow = lRow;
	long lSecondProductRow = lFirstRow;

	CDB_TBPlenoVento *pclFirstPleno = NULL;
	CDB_TBPlenoVento *pclSecondPleno = NULL;

	for( int iLoop = 0; iLoop < iLoopLimit; iLoop++ )
	{
		long lProductFirstRow = lRow;
		CDB_TBPlenoVento *pclTecBoxPleno = NULL;

		if( 0 == iLoop )
		{
			if( NULL == pWTCombination )
			{
				pclTecBoxPleno = dynamic_cast<CDB_TBPlenoVento *>( ( CData * )pclSSelPMaint->GetTecBoxPlenoIDPtr().MP );
			}
			else
			{
				pclTecBoxPleno = dynamic_cast<CDB_TBPlenoVento *>( ( CData * )pWTCombination->GetFirstIDPtr().MP );
			}

			pclFirstPleno = pclTecBoxPleno;
		}
		else
		{
			pclTecBoxPleno = dynamic_cast<CDB_TBPlenoVento *>( ( CData * )pWTCombination->GetSecondIDPtr().MP );
			pclSecondPleno = pclTecBoxPleno;
			lSecondProductRow = lRow;
		}

		if( NULL == pclTecBoxPleno )
		{
			continue;
		}

		pclSheet->SetTextPattern( CSSheet::_SSTextPattern::UserStaticColumn );

		CString str = TASApp.LoadLocalizedString( IDS_SSHEETSSELPROD_CPM_PRODUCTPLENO );
		str += _T(" (") + pclTecBoxPleno->GetFunctionsStr() + _T(")");
		lRow = AddStaticText( pclSheetDescription, CD_CPMP_Product, lRow, str );

		pclSheet->SetTextPatternProperty( CSSheet::_SSTextPatternProperty::TPP_FontBold, (LPARAM)FALSE );

		if( true == m_pTADSPageSetup->GetField( epfTECHBOXINFOPRODUCTNAME ) )
		{
			lRow = AddStaticText( pclSheetDescription, CD_CPMP_Product, lRow, pclTecBoxPleno->GetName() );
		}

		if( true == m_pTADSPageSetup->GetField( epfTECHBOXINFOPRODUCTPS ) )
		{
			str = TASApp.LoadLocalizedString( IDS_SSHEETSSELPROD_CPM_TECHBOXPS );
			str += CString( _T(" = ") ) + WriteCUDouble( _U_PRESSURE, pclTecBoxPleno->GetPmaxmax(), true );
			lRow = AddStaticText( pclSheetDescription, CD_CPMP_Product, lRow, str );
		}

		if( true == m_pTADSPageSetup->GetField( epfTECHBOXINFOPRODUCTPOWER ) )
		{
			str = TASApp.LoadLocalizedString( IDS_SSHEETSSELPROD_CPM_TECHBOXPOWER );
			str += CString( _T(" = ") );

			if( pclTecBoxPleno->GetPower() > 0 )
			{
				str += WriteCUDouble( _U_ELEC_POWER, pclTecBoxPleno->GetPower(), true );
			}
			else
			{
				str += CString( _T("-") );
			}

			lRow = AddStaticText( pclSheetDescription, CD_CPMP_Product, lRow, str );
		}

		if( true == m_pTADSPageSetup->GetField( epfTECHBOXINFOPRODUCTSUPPLYVOLTAGE ) )
		{
			str = TASApp.LoadLocalizedString( IDS_SSHEETSSELPROD_CPM_TECHBOXSV );
			str += CString( _T(" = ") ) + pclTecBoxPleno->GetPowerSupplyStr();
			lRow = AddStaticText( pclSheetDescription, CD_CPMP_Product, lRow, str );
		}

		// Column 'Quantity' and prices.
		int iQuantity = pclSSelPMaint->GetTecBoxPlenoNumber() * pclSSelPMaint->GetpSelectedInfos()->GetQuantity();
		_FillQtyPriceCol( pclSheetDescription, lProductFirstRow, pclTecBoxPleno, iQuantity );

		// Column 'Article number'.
		AddStaticText( pclSheetDescription, CD_CPMP_ArticleNumber, lProductFirstRow, _T("-") );

		// Remark: 'true' to exclude any connection reference in the article number.
		FillArtNumberCol( pclSheetDescription, CD_CPMP_ArticleNumber, lProductFirstRow, pclTecBoxPleno, pclTecBoxPleno->GetArtNum( true ) );

		if( NULL != pWTCombination && 0 == iLoop )
		{
			// Draw a solid line.
			pclSheet->SetCellBorder( CD_CPMP_Reference1, lRow, CD_CPMP_Remark, lRow, true, SS_BORDERTYPE_BOTTOM, SS_BORDERSTYLE_SOLID );

			// Save the first product range.
			long lSelFirstRow = lProductFirstRow;

			if( true == fSelIncludeColTitleIfNeeded )
			{
				lSelFirstRow = ( RD_CPMP_FirstAvailRow == lProductFirstRow ) ? lProductFirstRow - 1 : lProductFirstRow;
			}

			pclProductParam->AddRange( lSelFirstRow, lRow, pclTecBoxPleno );
		}
	}

	lRow--;

	// Fill references and remarks.
	if( true == fAddRemarks )
	{
		_FillRowGen( pclSheetDescription, pclSSelPMaint, lFirstRow, lRow );
	}
	// Add accessories.
	if( NULL != pclSSelPMaint->GetTecBoxPlenoAccessoryList() && pclSSelPMaint->GetTecBoxPlenoAccessoryList()->GetCount() > 0 )
	{
		// Draw dash line.
		pclSheet->SetCellBorder( CD_CPMP_Reference1, lRow, CD_CPMP_Footer - 1, lRow, true, SS_BORDERTYPE_BOTTOM, SS_BORDERSTYLE_FINE_DASH );

		// Attention: this 'method' doesn't return the next row but the last row!!
		lRow++;
		int iDistributedQty = pclSSelPMaint->GetTecBoxPlenoNumber();
		lRow = FillAccessories( pclSheetDescription, lRow, pclSSelPMaint->GetTecBoxPlenoAccessoryList(), pclSSelPMaint->GetpSelectedInfos()->GetQuantity(),
								iDistributedQty, pclProductParam );
	}

	if( NULL != pclSSelPMaint->GetTecBoxPlenoProtectionIDPtr().MP )
	{
		// Draw dash line.
		pclSheet->SetCellBorder( CD_CPMP_Reference1, lRow, CD_CPMP_Footer - 1, lRow, true, SS_BORDERTYPE_BOTTOM, SS_BORDERSTYLE_FINE_DASH );
		lRow++;
		lRow = _FillRowTBPlenoProtectionProduct( pclSheetDescription, lRow, pclSSelPMaint, fSelIncludeColTitleIfNeeded, pclProductParam );
	}
	// Draw a solid line.
	pclSheet->SetCellBorder( CD_CPMP_Reference1, lRow, CD_CPMP_Remark, lRow, true, SS_BORDERTYPE_BOTTOM, SS_BORDERSTYLE_SOLID );

	// Save the first product range.
	// Remark: if above we have no product combination, only one Pleno has been displayed and then 'lSecondProductRow' is always
	//         set to the first row
	long lSelFirstRow = lSecondProductRow;

	if( NULL == pWTCombination && true == fSelIncludeColTitleIfNeeded )
	{
		lSelFirstRow = ( RD_CPMP_FirstAvailRow == lSecondProductRow ) ? lSecondProductRow - 1 : lSecondProductRow;
	}

	if( NULL == pWTCombination )
	{
		// If the Pleno is not a combination, we save the Pleno here. In this case 'lSelFirstRow' is
		// always 'lFirstRow' (set in the entry of this method).
		pclProductParam->AddRange( lSelFirstRow, lRow, pclFirstPleno );
	}
	else if( NULL != pclSecondPleno )
	{
		// If the Pleno is a combination of two products, the first one has been already saved in the loop above. The second one is saved
		// now. And 'lSecondProductRow' is well the first line of the second product updated in the loop above.
		pclProductParam->AddRange( lSelFirstRow, lRow, pclSecondPleno );
	}

	return lRow;
}

long CSelProdPagePressMaint::_FillRowTBPlenoProtectionProduct( CSheetDescription *pclSheetDescription, long lRow, CDS_SSelPMaint *pclSSelPMaint,
		bool fSelIncludeColTitleIfNeeded, CProductParam *pclProductParam )
{
	if( NULL == pclSheetDescription || NULL == pclSheetDescription->GetSSheetPointer() || NULL == pclSSelPMaint )
	{
		return lRow;
	}
	
	if( NULL == pclProductParam )
	{
		pclProductParam = &m_clCPMPParam;
	}

	CSSheet *pclSheet = pclSheetDescription->GetSSheetPointer();
	
	long lFirstRow = lRow;
	long lSecondProductRow = lFirstRow;

	CDB_TBPlenoVento *pclFirstPleno = NULL;
	CDB_TBPlenoVento *pclSecondPleno = NULL;

	long lProductFirstRow = lRow;
	CDB_TBPlenoVento *pclTecBoxPleno = NULL;

	pclTecBoxPleno = dynamic_cast<CDB_TBPlenoVento *>( ( CData * )pclSSelPMaint->GetTecBoxPlenoProtectionIDPtr().MP );

	pclSheet->SetTextPattern( CSSheet::_SSTextPattern::UserStaticColumn );

	CString str = TASApp.LoadLocalizedString( IDS_SELECT_PLENO_WMPROTECTION_TITLE );
	str += _T(" (") + pclTecBoxPleno->GetFunctionsStr() + _T(")");
	lRow = AddStaticText( pclSheetDescription, CD_CPMP_Product, lRow, str );

	pclSheet->SetTextPatternProperty( CSSheet::_SSTextPatternProperty::TPP_FontBold, (LPARAM)FALSE );

	if( true == m_pTADSPageSetup->GetField( epfTECHBOXINFOPRODUCTNAME ) )
	{
		lRow = AddStaticText( pclSheetDescription, CD_CPMP_Product, lRow, pclTecBoxPleno->GetName() );
	}

	if( true == m_pTADSPageSetup->GetField( epfTECHBOXINFOPRODUCTPS ) )
	{
		str = TASApp.LoadLocalizedString( IDS_SSHEETSSELPROD_CPM_TECHBOXPS );
		str += CString( _T(" = ") ) + WriteCUDouble( _U_PRESSURE, pclTecBoxPleno->GetPmaxmax(), true );
		lRow = AddStaticText( pclSheetDescription, CD_CPMP_Product, lRow, str );
	}

	if( true == m_pTADSPageSetup->GetField( epfTECHBOXINFOPRODUCTPOWER ) )
	{
		str = TASApp.LoadLocalizedString( IDS_SSHEETSSELPROD_CPM_TECHBOXPOWER );
		str += CString( _T(" = ") );

		if( pclTecBoxPleno->GetPower() > 0 )
		{
			str += WriteCUDouble( _U_ELEC_POWER, pclTecBoxPleno->GetPower(), true );
		}
		else
		{
			str += CString( _T("-") );
		}

		lRow = AddStaticText( pclSheetDescription, CD_CPMP_Product, lRow, str );
	}

	if( true == m_pTADSPageSetup->GetField( epfTECHBOXINFOPRODUCTSUPPLYVOLTAGE ) )
	{
		str = TASApp.LoadLocalizedString( IDS_SSHEETSSELPROD_CPM_TECHBOXSV );
		str += CString( _T(" = ") ) + pclTecBoxPleno->GetPowerSupplyStr();
		lRow = AddStaticText( pclSheetDescription, CD_CPMP_Product, lRow, str );
	}

	// Column 'Quantity' and prices.
	int iQuantity = pclSSelPMaint->GetTecBoxPlenoNumber() * pclSSelPMaint->GetpSelectedInfos()->GetQuantity();
	_FillQtyPriceCol( pclSheetDescription, lProductFirstRow, pclTecBoxPleno, iQuantity );

	// Column 'Article number'.
	AddStaticText( pclSheetDescription, CD_CPMP_ArticleNumber, lProductFirstRow, _T("-") );

	// Remark: 'true' to exclude any connection reference in the article number.
	FillArtNumberCol( pclSheetDescription, CD_CPMP_ArticleNumber, lProductFirstRow, pclTecBoxPleno, pclTecBoxPleno->GetArtNum( true ) );

	lRow--;

	// Add accessories.
	if( NULL != pclSSelPMaint->GetTecBoxPlenoProtecAccessoryList() && pclSSelPMaint->GetTecBoxPlenoProtecAccessoryList()->GetCount() > 0 )
	{
		// Draw dash line.
		pclSheet->SetCellBorder( CD_CPMP_Reference1, lRow, CD_CPMP_Footer - 1, lRow, true, SS_BORDERTYPE_BOTTOM, SS_BORDERSTYLE_FINE_DASH );

		// Attention: this 'method' doesn't return the next row but the last row!!
		lRow++;
		int iDistributedQty = pclSSelPMaint->GetTecBoxPlenoNumber();
		lRow = FillAccessories( pclSheetDescription, lRow, pclSSelPMaint->GetTecBoxPlenoProtecAccessoryList(), pclSSelPMaint->GetpSelectedInfos()->GetQuantity(),
								iDistributedQty, pclProductParam );
	}

	// Draw a solid line.
	pclSheet->SetCellBorder( CD_CPMP_Reference1, lRow, CD_CPMP_Remark, lRow, true, SS_BORDERTYPE_BOTTOM, SS_BORDERSTYLE_SOLID );
	long lSelFirstRow = lFirstRow;

	if( true == fSelIncludeColTitleIfNeeded )
	{
		lSelFirstRow = ( RD_CPMP_FirstAvailRow == lFirstRow ) ? lFirstRow - 1 : lFirstRow;
	}

	pclProductParam->AddRange( lSelFirstRow, lRow, pclTecBoxPleno );

	return lRow;
}

long CSelProdPagePressMaint::_FillRowPlenoRefillProduct( CSheetDescription* pclSheetDescription, long lRow, CDS_SSelPMaint* pclSSelPMaint, 
		bool fSelIncludeColTitleIfNeeded, bool fAddRemarks, CProductParam *pclProductParam )
{
	if( NULL == pclSheetDescription || NULL == pclSheetDescription->GetSSheetPointer() || NULL == pclSSelPMaint
		|| _NULL_IDPTR == pclSSelPMaint->GetPlenoRefillIDPtr() )
	{
		return lRow;
	}

	if( NULL == pclProductParam )
	{
		pclProductParam = &m_clCPMPParam;
	}

	CSSheet *pclSheet = pclSheetDescription->GetSSheetPointer();
	CDB_PlenoRefill *pclPlenoRefill = ( CDB_PlenoRefill * )( pclSSelPMaint->GetPlenoRefillIDPtr().MP );

	long lFirstRow = lRow;
	pclSheet->SetTextPattern( CSSheet::_SSTextPattern::UserStaticColumn );

	CString str = TASApp.LoadLocalizedString( IDS_SSHEETSSELPROD_CPM_PRODUCTPLENOREFILL );
	str += _T(" (") + pclPlenoRefill->GetFunctionsStr() + _T(")");
	lRow = AddStaticText( pclSheetDescription, CD_CPMP_Product, lRow, str );

	pclSheet->SetTextPatternProperty( CSSheet::_SSTextPatternProperty::TPP_FontBold, (LPARAM)FALSE );

	if( true == m_pTADSPageSetup->GetField( epfWATERMAKEUPINFOPRODUCTNAME ) )
	{
		lRow = AddStaticText( pclSheetDescription, CD_CPMP_Product, lRow, pclPlenoRefill->GetName() );
	}

	if( true == m_pTADSPageSetup->GetField( epfWATERMAKEUPINFOPRODUCTFUNCTIONS ) )
	{
		lRow = AddStaticText( pclSheetDescription, CD_CPMP_Product, lRow, pclPlenoRefill->GetFunctionsStr() );
	}

	if( true == m_pTADSPageSetup->GetField( epfWATERMAKEUPINFOPRODUCTCAPACITY ) )
	{
		lRow = AddStaticText( pclSheetDescription, CD_CPMP_Product, lRow, pclPlenoRefill->GetCapacityStr( true ) );
	}

	if( true == m_pTADSPageSetup->GetField( epfWATERMAKEUPINFOPRODUCTHEIGHT ) )
	{
		lRow = AddStaticText( pclSheetDescription, CD_CPMP_Product, lRow, WriteCUDouble( _U_LENGTH, pclPlenoRefill->GetHeight(), true ) );
	}

	if( true == m_pTADSPageSetup->GetField( epfWATERMAKEUPINFOPRODUCTWEIGHT ) )
	{
		lRow = AddStaticText( pclSheetDescription, CD_CPMP_Product, lRow, WriteCUDouble( _U_MASS, pclPlenoRefill->GetWeight(), true ) );
	}

	lRow--;

	// Column 'Quantity' and prices.
	int iQuantity = pclSSelPMaint->GetPlenoRefillNumber() * pclSSelPMaint->GetpSelectedInfos()->GetQuantity();
	_FillQtyPriceCol( pclSheetDescription, lFirstRow, pclPlenoRefill, iQuantity );

	// Column 'Article number'.
	AddStaticText( pclSheetDescription, CD_CPMP_ArticleNumber, lFirstRow, _T("-") );

	// Remark: 'true' to exclude any connection reference in the article number.
	FillArtNumberCol( pclSheetDescription, CD_CPMP_ArticleNumber, lFirstRow, pclPlenoRefill, pclPlenoRefill->GetArtNum( true ) );

	// Fill references and remarks.
	if( true == fAddRemarks )
	{
		_FillRowGen( pclSheetDescription, pclSSelPMaint, lFirstRow, lRow );
	}

	// Add accessories.
	if( NULL != pclSSelPMaint->GetPlenoRefillAccessoryList() && pclSSelPMaint->GetPlenoRefillAccessoryList()->GetCount() > 0 )
	{
		// Draw dash line.
		pclSheet->SetCellBorder( CD_CPMP_Reference1, lRow, CD_CPMP_Footer - 1, lRow, true, SS_BORDERTYPE_BOTTOM, SS_BORDERSTYLE_FINE_DASH );

		// Attention: this 'method' doesn't return the next row but the last row!!
		lRow++;
		int iDistributedQty = pclSSelPMaint->GetPlenoRefillNumber();
		lRow = FillAccessories( pclSheetDescription, lRow, pclSSelPMaint->GetPlenoRefillAccessoryList(), pclSSelPMaint->GetpSelectedInfos()->GetQuantity(),
								iDistributedQty, pclProductParam );
	}

	// Draw a solid line.
	pclSheet->SetCellBorder( CD_CPMP_Reference1, lRow, CD_CPMP_Remark, lRow, true, SS_BORDERTYPE_BOTTOM, SS_BORDERSTYLE_SOLID );

	long lSelFirstRow = lFirstRow;

	if( true == fSelIncludeColTitleIfNeeded )
	{
		lSelFirstRow = ( RD_CPMP_FirstAvailRow == lFirstRow ) ? lFirstRow - 1 : lFirstRow;
	}

	pclProductParam->AddRange( lSelFirstRow, lRow, pclPlenoRefill );

	return lRow;
}

long CSelProdPagePressMaint::_FillRowTecBoxVentoProduct( CSheetDescription *pclSheetDescription, long lRow, CDS_SSelPMaint *pclSSelPMaint,
		bool fSelIncludeColTitleIfNeeded, CProductParam *pclProductParam )
{
	if( NULL == pclSheetDescription || NULL == pclSheetDescription->GetSSheetPointer() || NULL == pclSSelPMaint
		|| _NULL_IDPTR == pclSSelPMaint->GetTecBoxVentoIDPtr() )
	{
		return lRow;
	}

	if( NULL == pclProductParam )
	{
		pclProductParam = &m_clCPMPParam;
	}

	CSSheet *pclSheet = pclSheetDescription->GetSSheetPointer();
	CDB_TBPlenoVento *pclTechBoxVento = ( CDB_TBPlenoVento * )( pclSSelPMaint->GetTecBoxVentoIDPtr().MP );

	long lFirstRow = lRow;
	pclSheet->SetTextPattern( CSSheet::_SSTextPattern::UserStaticColumn );

	CString str = TASApp.LoadLocalizedString( IDS_SSHEETSSELPROD_CPM_PRODUCTVENTO );
	str += _T(" (") + pclTechBoxVento->GetFunctionsStr() + _T(")");
	lRow = AddStaticText( pclSheetDescription, CD_CPMP_Product, lRow, str );

	pclSheet->SetTextPatternProperty( CSSheet::_SSTextPatternProperty::TPP_FontBold, (LPARAM)FALSE );

	if( true == m_pTADSPageSetup->GetField( epfTECHBOXINFOPRODUCTNAME ) )
	{
		lRow = AddStaticText( pclSheetDescription, CD_CPMP_Product, lRow, pclTechBoxVento->GetName() );
	}

	if( true == m_pTADSPageSetup->GetField( epfTECHBOXINFOPRODUCTPS ) )
	{
		str = TASApp.LoadLocalizedString( IDS_SSHEETSSELPROD_CPM_TECHBOXPS );
		str += CString( _T(" = ") ) + WriteCUDouble( _U_PRESSURE, pclTechBoxVento->GetPmaxmax(), true );
		lRow = AddStaticText( pclSheetDescription, CD_CPMP_Product, lRow, str );
	}

	if( true == m_pTADSPageSetup->GetField( epfTECHBOXINFOPRODUCTPOWER ) )
	{
		str = TASApp.LoadLocalizedString( IDS_SSHEETSSELPROD_CPM_TECHBOXPOWER );
		str += CString( _T(" = ") ) + WriteCUDouble( _U_ELEC_POWER, pclTechBoxVento->GetPower(), true );
		lRow = AddStaticText( pclSheetDescription, CD_CPMP_Product, lRow, str );
	}

	if( true == m_pTADSPageSetup->GetField( epfTECHBOXINFOPRODUCTSUPPLYVOLTAGE ) )
	{
		str = TASApp.LoadLocalizedString( IDS_SSHEETSSELPROD_CPM_TECHBOXSV );
		str += CString( _T(" = ") ) + pclTechBoxVento->GetPowerSupplyStr();
		lRow = AddStaticText( pclSheetDescription, CD_CPMP_Product, lRow, str );
	}

	// Column 'Quantity' and prices.
	int iQuantity = pclSSelPMaint->GetTecBoxVentoNumber() * pclSSelPMaint->GetpSelectedInfos()->GetQuantity();
	_FillQtyPriceCol( pclSheetDescription, lFirstRow, pclTechBoxVento, iQuantity );

	// Column 'Article number'.
	AddStaticText( pclSheetDescription, CD_CPMP_ArticleNumber, lFirstRow, _T("-") );

	// Remark: 'true' to exclude any connection reference in the article number.
	FillArtNumberCol( pclSheetDescription, CD_CPMP_ArticleNumber, lFirstRow, pclTechBoxVento, pclTechBoxVento->GetArtNum( true ) );

	lRow--;

	// Add accessories.
	if( NULL != pclSSelPMaint->GetTecBoxVentoAccessoryList() && pclSSelPMaint->GetTecBoxVentoAccessoryList()->GetCount() > 0 )
	{
		// Draw dash line.
		pclSheet->SetCellBorder( CD_CPMP_Reference1, lRow, CD_CPMP_Footer - 1, lRow, true, SS_BORDERTYPE_BOTTOM, SS_BORDERSTYLE_FINE_DASH );

		// Attention: this 'method' doesn't return the next row but the last row!!
		lRow++;
		int iDistributedQty = pclSSelPMaint->GetTecBoxVentoNumber();
		lRow = FillAccessories( pclSheetDescription, lRow, pclSSelPMaint->GetTecBoxVentoAccessoryList(), pclSSelPMaint->GetpSelectedInfos()->GetQuantity(),
								iDistributedQty, pclProductParam );
	}

	// Draw a solid line.
	pclSheet->SetCellBorder( CD_CPMP_Reference1, lRow, CD_CPMP_Remark, lRow, true, SS_BORDERTYPE_BOTTOM, SS_BORDERSTYLE_SOLID );

	long lSelFirstRow = lFirstRow;

	if( true == fSelIncludeColTitleIfNeeded )
	{
		lSelFirstRow = ( RD_CPMP_FirstAvailRow == lFirstRow ) ? lFirstRow - 1 : lFirstRow;
	}

	pclProductParam->AddRange( lSelFirstRow, lRow, pclTechBoxVento );

	return lRow;
}

void CSelProdPagePressMaint::_FillRowGen( CSheetDescription *pclSheetDescription, CDS_SSelPMaint *pclSSelPMaint, long lFirstRow, long lLastRow )
{
	if( NULL == pclSheetDescription || NULL == pclSheetDescription->GetSSheetPointer() || NULL == pclSSelPMaint )
	{
		return;
	}

	CSSheet *pclSheet = pclSheetDescription->GetSSheetPointer();
	CString str1, str2;

	// Spanning must be done here because it's the only place where we know exactly number of lines to span!
	// Span reference #1 and #2.
	AddCellSpanW( pclSheetDescription, CD_CPMP_Reference1, lFirstRow, 1, lLastRow - lFirstRow + 1 );
	AddCellSpanW( pclSheetDescription, CD_CPMP_Reference2, lFirstRow, 1, lLastRow - lFirstRow + 1 );

	pclSheet->SetTextPattern( CSSheet::_SSTextPattern::UserStaticColumn );
	pclSheet->SetTextPatternProperty( CSSheet::_SSTextPatternProperty::TPP_FontBold, (LPARAM)FALSE );

	// Column 'Reference 1'.
	pclSheet->SetTextPatternProperty( CSSheet::_SSTextPatternProperty::TPP_StyleWordWrap, (LPARAM)TRUE );
	pclSheet->SetTextPatternProperty( CSSheet::_SSTextPatternProperty::TPP_StyleHorizontalAlign, (LPARAM)SSS_ALIGN_LEFT );
	AddStaticText( pclSheetDescription, CD_CPMP_Reference1, lFirstRow, pclSSelPMaint->GetpSelectedInfos()->GetReference( CSelectedInfos::eRef1 ) );

	// Column 'Reference 2'.
	AddStaticText( pclSheetDescription, CD_CPMP_Reference2, lFirstRow, pclSSelPMaint->GetpSelectedInfos()->GetReference( CSelectedInfos::eRef2 ) );
	pclSheet->SetTextPatternProperty( CSSheet::_SSTextPatternProperty::TPP_StyleWordWrap, (LPARAM)FALSE );
	pclSheet->SetTextPatternProperty( CSSheet::_SSTextPatternProperty::TPP_StyleHorizontalAlign, (LPARAM)SSS_ALIGN_CENTER );

	// Column 'Remark'.
	if( true == m_pTADSPageSetup->GetField( epfREMARK ) )
	{
		if( 0 != pclSSelPMaint->GetpSelectedInfos()->GetRemarkIndex() )		// remark exist
		{
			str1.Format( _T("%d"), pclSSelPMaint->GetpSelectedInfos()->GetRemarkIndex() );
			AddStaticText( pclSheetDescription, CD_CPMP_Remark, lFirstRow, str1 );
		}
	}
}

void CSelProdPagePressMaint::_FillQtyPriceCol( CSheetDescription *pclSheetDescription, long lFirstRow, CData *pclProduct, int iQuantity )
{
	if( NULL == pclSheetDescription || NULL == pclSheetDescription->GetSSheetPointer() || NULL == pclProduct )
	{
		return;
	}

	CString str;
	
	// Column 'Quantity'.
	CSSheet* pclSheet = pclSheetDescription->GetSSheetPointer();
	long lRow = lFirstRow;
	
	if( true == m_pTADSPageSetup->GetField( PageField_enum::epfQUANTITY ) )	
	{
		pclSheet->SetTextPatternProperty( CSSheet::_SSTextPatternProperty::TPP_StyleHorizontalAlign, (LPARAM)SSS_ALIGN_CENTER );

		str.Format( _T("%d"), iQuantity );
		AddStaticText( pclSheetDescription, CD_CPMP_Quantity, lRow++, str );
	}

	if( true == TASApp.IsPriceUsed() )
	{
		// Column 'Price'.
		lRow = lFirstRow;
		AddStaticText( pclSheetDescription, CD_CPMP_UnitPrice, lRow, _T("-") );

		double dPrice = TASApp.GetpTADB()->GetPrice( pclProduct->GetArtNum( true ) );

		if( true == m_pTADSPageSetup->GetField( PageField_enum::epfSALEUNITPRICE ) && dPrice > 0.0 )
		{
			str = (CString)WriteDouble( dPrice, 2, 1, 0 );
			AddStaticText( pclSheetDescription, CD_CPMP_UnitPrice, lRow++, str );
		}

		// Column 'Total Price'.
		lRow = lFirstRow;
		AddStaticText( pclSheetDescription, CD_CPMP_TotalPrice, lRow, _T("-") );			// Default string

		if( true == m_pTADSPageSetup->GetField( PageField_enum::epfSALETOTALPRICE ) && dPrice > 0.0 )
		{
			double dTotal = dPrice * iQuantity;
			str = (CString)WriteDouble( dTotal, 2, 1, 0 );
			AddStaticText( pclSheetDescription, CD_CPMP_TotalPrice, lRow++, str );
		}
	}
}

void CSelProdPagePressMaint::_FillRemarks()
{
	// Call base class method to get a new 'Sheet'.
	CSheetDescription *pclSDRemarks = CMultiSpreadBase::CreateSSheet( SD_Remarks );

	if( NULL == pclSDRemarks || NULL == pclSDRemarks->GetSSheetPointer() )
	{
		return;
	}

	CSSheet *pclSheet = pclSDRemarks->GetSSheetPointer();

	if( false == CSelProdPageBase::PrepareSheet( pclSDRemarks, CD_PMR_LastColumn, m_bForPrint, false ) )
	{
		m_ViewDescription.RemoveOneSheetDescription( SD_Remarks );
		return;
	}

	// Initialize column width for header and footer column.
	if( false == m_bForPrint )
	{
		SetColWidth( pclSDRemarks, CD_PMR_Header, DefaultColumnWidth::DCW_Header );
		SetColWidth( pclSDRemarks, CD_PMR_Footer, DefaultColumnWidth::DCW_Footer );
	}
	else
	{
		pclSheet->ShowCol( CD_PMR_Header, FALSE );
		pclSheet->ShowCol( CD_PMR_Footer, FALSE );
	}

	// Set the column width. To perfectly match the main title, we use proportionality.
	int iHFColPixels = ( false == m_bForPrint ) ? pclSheet->ColWidthToLogUnits( 2.0 ) : 0;
	double dAvailableWidth = pclSheet->LogUnitsToColWidthW( RIGHTVIEWWIDTH - 2 * iHFColPixels );
	SetColWidth( pclSDRemarks, CD_PMR_Title, dAvailableWidth );
	SetColWidth( pclSDRemarks, CD_PMR_LastColumn, 0.0 );

	pclSheet->ShowCol( CD_PMR_LastColumn, FALSE );
	CRect rect = pclSheet->GetSheetSizeInPixels( false );
	m_mapSheetInfos[pclSDRemarks].m_dPageWidth = rect.Width();

	bool bRemarkExist = false;
	long lRow = FillRemarks( pclSDRemarks, RD_PMR_FirstRow, &bRemarkExist );

	if( false == bRemarkExist )
	{
		m_ViewDescription.RemoveOneSheetDescription( SD_Remarks );
		m_mapSheetInfos.erase( m_mapSheetInfos.find( pclSDRemarks ) );
	}
}

void CSelProdPagePressMaint::_AddArticleList( CDS_SSelPMaint *pclSSelPressMaint )
{
	if( NULL == pclSSelPressMaint )
	{
		return;
	}

	CArticleGroup *pclArticleGroup = new CArticleGroup();

	if( NULL == pclArticleGroup )
	{
		return;
	}

	// See comments in the 'SelProdArticle.h'.
	pclArticleGroup->SetMergeSameArticleFlag( false );

	CArticleContainer *pclArticleContainer = NULL;

	////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
	// TECBOX - Compresso/Transfero
	CDB_TecBox *pclTecBox = dynamic_cast<CDB_TecBox *>( pclSSelPressMaint->GetTecBoxCompTransfIDPtr().MP );

	if( NULL != pclTecBox )
	{
		int iQuantity = pclSSelPressMaint->GetpSelectedInfos()->GetQuantity();
		int iDistributedQty = pclSSelPressMaint->GetTecBoxCompTransfNumber();
		int iTotalQty = iQuantity * iDistributedQty;

		CString strDefinitionAdditionalInfo = _T(" (") + pclTecBox->GetFunctionsStr() + _T(")");
		pclArticleContainer = pclArticleGroup->AddArticle( pclTecBox, iTotalQty, false, strDefinitionAdditionalInfo );
		
		AddAccessoriesInArticleContainer( pclSSelPressMaint->GetTecBoxCompTransfAccessoryList(), pclArticleContainer, iQuantity, 
				false, iDistributedQty, true );
	}

	////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////


	////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
	// STATICO OR PRIMARY VESSEL
	CDB_Vessel *pclVessel = dynamic_cast<CDB_Vessel *>( pclSSelPressMaint->GetVesselIDPtr().MP );

	if( NULL != pclVessel )
	{
		int iQuantity = pclSSelPressMaint->GetpSelectedInfos()->GetQuantity();
		int iTotalQty = pclSSelPressMaint->GetVesselNumber() * iQuantity;
		int iDistributedQty = pclSSelPressMaint->GetVesselNumber() + pclSSelPressMaint->GetSecondaryVesselNumber();

		CString strDefinitionAdditionalInfo = _T(" (") + pclVessel->GetVesselTypeStr() + _T(")");
		pclArticleContainer = pclArticleGroup->AddArticle( pclVessel, iTotalQty, false, strDefinitionAdditionalInfo );

		AddAccessoriesInArticleContainer( pclSSelPressMaint->GetVesselAccessoryList(), pclArticleContainer, iQuantity, 
				false, iDistributedQty, true );
	}
	////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////


	////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
	//// BUFFER VESSEL
	CDB_Vessel *pclBufferVessel = dynamic_cast<CDB_Vessel *>( pclSSelPressMaint->GetTecBoxIntegratedVesselIDPtr().MP );
	
	//HYS-970: Don't display integrated vessel again when it is already done above
	if( ( NULL != pclBufferVessel ) && ( NULL != pclTecBox ) && ( pclBufferVessel != pclVessel ) )
	{
		int iQuantity = pclSSelPressMaint->GetpSelectedInfos()->GetQuantity();
		
		// Buffer vessel is linked to the tecbox Transfero.
		int iTotalQty = pclSSelPressMaint->GetTecBoxCompTransfNumber() * iQuantity;

		// HYS-872: to put the accessory under the buffer vessel line.
		CString strDefinitionAdditionalInfo = _T(" (") + pclBufferVessel->GetVesselTypeStr( true ) + _T(")");
		pclArticleContainer = pclArticleGroup->AddArticle( pclBufferVessel, iTotalQty, false , strDefinitionAdditionalInfo );
		
		AddAccessoriesInArticleContainer( pclSSelPressMaint->GetTecBoxIntegratedVesselAccessoryList(), pclArticleContainer, iQuantity,
			false, pclSSelPressMaint->GetTecBoxCompTransfNumber(), true );
	}
	////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////


	////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
	// SECONDARY VESSEL
	pclVessel = dynamic_cast<CDB_Vessel *>( pclSSelPressMaint->GetSecondaryVesselIDPtr().MP );

	if( NULL != pclVessel )
	{
		int iQuantity = pclSSelPressMaint->GetSecondaryVesselNumber() * pclSSelPressMaint->GetpSelectedInfos()->GetQuantity();

		CString strDefinitionAdditionalInfo = _T(" (") + pclVessel->GetVesselTypeStr() + _T(")");
		pclArticleGroup->AddArticle( pclVessel, iQuantity, false, strDefinitionAdditionalInfo );
	}

	////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////


	////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
	// INTERMEDIATE VESSEL
	// Check know if we have an intermediate vessel and its accessories.
	CDB_Vessel *pclIntermediateVessel = dynamic_cast<CDB_Vessel *>( pclSSelPressMaint->GetIntermediateVesselIDPtr().MP );

	if( NULL != pclIntermediateVessel )
	{
		int iQuantity = pclSSelPressMaint->GetpSelectedInfos()->GetQuantity();
		int iDistributedQty = pclSSelPressMaint->GetIntermediateVesselNumber();
		int iTotalQty = iQuantity * iDistributedQty;

		CString strDefinitionAdditionalInfo = _T(" (") + pclIntermediateVessel->GetVesselTypeStr() + _T(")");
		pclArticleContainer = pclArticleGroup->AddArticle( pclIntermediateVessel, iTotalQty, false, strDefinitionAdditionalInfo );

		AddAccessoriesInArticleContainer( pclSSelPressMaint->GetIntermediateVesselAccessoryList(), pclArticleContainer, iQuantity, 
				false, iDistributedQty, true  );
	}

	////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////


	////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
	// TECBOX - Vento
	CDB_TBPlenoVento *pclTecBoxVento = dynamic_cast<CDB_TBPlenoVento *>( pclSSelPressMaint->GetTecBoxVentoIDPtr().MP );

	if( NULL != pclTecBoxVento )
	{
		int iQuantity = pclSSelPressMaint->GetpSelectedInfos()->GetQuantity();
		int iDistributedQty = pclSSelPressMaint->GetTecBoxVentoNumber();
		int iTotalQty = iQuantity * iDistributedQty;

		CString strDefinitionAdditionalInfo = _T(" (") + pclTecBoxVento->GetFunctionsStr() + _T(")");
		pclArticleContainer = pclArticleGroup->AddArticle( pclTecBoxVento, iTotalQty, false, strDefinitionAdditionalInfo );

		AddAccessoriesInArticleContainer( pclSSelPressMaint->GetTecBoxVentoAccessoryList(), pclArticleContainer, iQuantity, 
				false, iDistributedQty, true  );
	}

	////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////


	////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
	// TECBOX - Pleno
	CDB_TBPlenoVento *pclTecBoxPleno = dynamic_cast<CDB_TBPlenoVento *>( pclSSelPressMaint->GetTecBoxPlenoIDPtr().MP );
	CDB_TBPlenoVento *pclTecBoxPlenoWMProtection = dynamic_cast<CDB_TBPlenoVento *>( pclSSelPressMaint->GetTecBoxPlenoProtectionIDPtr().MP );
	CDB_Set *pclWTCombination = dynamic_cast<CDB_Set *>( pclSSelPressMaint->GetAdditionalTecBoxPlenoIDPtr().MP );

	if( NULL != pclTecBoxPleno || NULL != pclWTCombination )
	{
		int iQuantity = pclSSelPressMaint->GetpSelectedInfos()->GetQuantity();
		int iDistributedQty = pclSSelPressMaint->GetTecBoxPlenoNumber();
		int iTotalQty = iQuantity * iDistributedQty;

		if( NULL == pclWTCombination )
		{
			CString strDefinitionAdditionalInfo = _T(" (") + pclTecBoxPleno->GetFunctionsStr() + _T(")");
			pclArticleContainer = pclArticleGroup->AddArticle( pclTecBoxPleno, iTotalQty, false, strDefinitionAdditionalInfo );
		}
		else
		{
			pclTecBoxPleno = dynamic_cast<CDB_TBPlenoVento*>( (CData*)pclWTCombination->GetFirstIDPtr().MP );

			if( NULL != pclTecBoxPleno )
			{
				CString strDefinitionAdditionalInfo = _T(" (") + pclTecBoxPleno->GetFunctionsStr() + _T(")");
				pclArticleContainer = pclArticleGroup->AddArticle( pclTecBoxPleno, iTotalQty, false, strDefinitionAdditionalInfo );
			}

			pclTecBoxPleno = dynamic_cast<CDB_TBPlenoVento*>( (CData*)pclWTCombination->GetSecondIDPtr().MP );

			if( NULL != pclTecBoxPleno )
			{
				CString strDefinitionAdditionalInfo = _T(" (") + pclTecBoxPleno->GetFunctionsStr() + _T(")");
				pclArticleContainer = pclArticleGroup->AddArticle( pclTecBoxPleno, iTotalQty, false, strDefinitionAdditionalInfo );
			}
		}

		AddAccessoriesInArticleContainer( pclSSelPressMaint->GetTecBoxPlenoAccessoryList(), pclArticleContainer, iQuantity, 
				false, iDistributedQty, true  );
		
		// HYS-1121 : Add article number 
		if( NULL != pclTecBoxPlenoWMProtection )
		{
			pclArticleContainer = pclArticleGroup->AddArticle( pclTecBoxPlenoWMProtection, iTotalQty );
		}
		AddAccessoriesInArticleContainer( pclSSelPressMaint->GetTecBoxPlenoProtecAccessoryList(), pclArticleContainer, iQuantity, 
				false, iDistributedQty, true  );
	}

	////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////


	////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
	// Pleno refill
	CDB_PlenoRefill *pclPlenoRefill = dynamic_cast<CDB_PlenoRefill *>( pclSSelPressMaint->GetPlenoRefillIDPtr().MP );

	if( NULL != pclPlenoRefill )
	{
		int iQuantity = pclSSelPressMaint->GetpSelectedInfos()->GetQuantity();
		int iDistributedQty = pclSSelPressMaint->GetPlenoRefillNumber();
		int iTotalQty = iQuantity * iDistributedQty;

		CString strDefinitionAdditionalInfo = _T(" (") + pclPlenoRefill->GetFunctionsStr() + _T(")");
		pclArticleContainer = pclArticleGroup->AddArticle( pclPlenoRefill, iTotalQty, false, strDefinitionAdditionalInfo );

		// HYS-2072: Add the refill accessories in the group.
		AddAccessoriesInArticleContainer( pclSSelPressMaint->GetPlenoRefillAccessoryList(), pclArticleContainer, iQuantity, 
				false, iDistributedQty, true );
	}

	////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////


	CSelProdPageBase::AddArticleInGroupList( pclArticleGroup );

	// Remark: pay attention that 'CPage::AddArticleInGroupList' will increment quantity if article already exist or
	//         will create a new 'CArticleGroup' and will copy to it data passed as argument. This is the reason
	//         why you need to delete 'pclArticleGroup' after the call.
	delete pclArticleGroup;
}

long CSelProdPagePressMaint::_AddSpecialStaticText( CSheetDescription *pclSheetDescription, long lColumn, long lRow, CString strText, CString strIndex,
		CString strValue )
{
	if( NULL == pclSheetDescription || NULL == pclSheetDescription->GetSSheetPointer() )
	{
		return lRow;
	}

	// This method will create two bitmaps containing the text.
	// The 'clImage' will contain the normal image: background white and text in black.
	// The 'clImageSelected' will contain the selected image: background at the same default color for the 'CSSheet' selection and text in black.

	// Why not only one bitmap with transparent background? Because font used in the TSpread cells is anti-aliased. When writing in a DC with the
	// 'DrawText' method, if background is white, the anti-aliasing will soft edge with colors between black and white. If we set that the white color is
	// the transparent color, only the background will be transparent. The intermediate colors for the anti-aliasing stay non transparent. If picture is set
	// in a TSpread cell with white background, is not a problem. But if background has a different color (for the selection), we will see the white softening
	// around the text and it is ugly. Thus, we prepare two images: one with white and one with the 'CSSheet' default selection color. Internally, 'CSSheet'
	// will choose between these two images in regards to the current selection.
	// Keep in mind that if the color selection is changed in 'CSSheet', you must adapt the background color here for the selected image.

	CSSheet *pclSheet = pclSheetDescription->GetSSheetPointer();

	CDC *pDC = GetDC();
	CDC dcImage;
	dcImage.CreateCompatibleDC( pDC );
	CDC dcImageSelected;
	dcImageSelected.CreateCompatibleDC( pDC );

	int iLogPixelsY = pDC->GetDeviceCaps( LOGPIXELSY );
	int iWidth = pDC->GetDeviceCaps( HORZRES );
	int iHeight = pDC->GetDeviceCaps( VERTRES );

	CBitmap clImage;
	clImage.CreateCompatibleBitmap( pDC, iWidth, iHeight );

	CBitmap clImageSelected;
	clImageSelected.CreateCompatibleBitmap( pDC, iWidth, iHeight );

	ReleaseDC( pDC );

	// To get the good font.
	pclSheet->SetTextPattern( CSSheet::_SSTextPattern::UserStaticColumn );
	pclSheet->SetTextPatternProperty( CSSheet::_SSTextPatternProperty::TPP_FontBold, (LPARAM)FALSE );
	pclSheet->SetStaticText( lColumn, lRow, _T( "" ) );

	LOGFONT rLogFont;
	CFont *pclFontText = CFont::FromHandle( pclSheet->GetFont( lColumn, lRow ) );
	pclFontText->GetLogFont( &rLogFont );

	CFont clFontIndex;
	rLogFont.lfHeight = -MulDiv( 7, iLogPixelsY, 72 );
	clFontIndex.CreateFontIndirect( &rLogFont );

	dcImage.SetBkMode( TRANSPARENT );
	CBitmap *pclOldImage = dcImage.SelectObject( &clImage );
	dcImage.FillSolidRect( 0, 0, iWidth, iHeight, _WHITE );
	dcImage.SetTextColor( _BLACK );

	dcImageSelected.SetBkMode( TRANSPARENT );
	CBitmap *pclOldImageSelected = dcImageSelected.SelectObject( &clImageSelected );
	dcImageSelected.FillSolidRect( 0, 0, iWidth, iHeight, pclSheet->GetDefaultSelectionColor() );
	dcImageSelected.SetTextColor( _BLACK );

	dcImage.SelectObject( pclFontText );
	dcImageSelected.SelectObject( pclFontText );

	CRect rectText( 0, 0, 0, 0 );
	dcImage.DrawText( strText, &rectText, DT_CENTER | DT_CALCRECT | DT_NOPREFIX );
	dcImage.DrawText( strText, &rectText, DT_CENTER | DT_NOPREFIX );
	dcImageSelected.DrawText( strText, &rectText, DT_CENTER | DT_NOPREFIX );

	dcImage.SelectObject( &clFontIndex );
	dcImageSelected.SelectObject( &clFontIndex );

	CRect rectIndex( 0, 0, 0, 0 );
	dcImage.DrawText( strIndex, &rectIndex, DT_CENTER | DT_CALCRECT | DT_NOPREFIX );

	rectIndex.MoveToXY( rectText.right - 1, rectText.bottom - 5 );
	dcImage.DrawText( strIndex, &rectIndex, DT_CENTER | DT_NOPREFIX );
	dcImageSelected.DrawText( strIndex, &rectIndex, DT_CENTER | DT_NOPREFIX );

	CRect rectTextIndexSize;
	rectTextIndexSize.UnionRect( rectText, rectIndex );

	dcImage.SelectObject( pclFontText );
	dcImageSelected.SelectObject( pclFontText );

	CRect rectValue( 0, 0, 0, 0 );
	dcImage.DrawText( strValue, &rectValue, DT_CENTER | DT_CALCRECT | DT_NOPREFIX );

	rectValue.MoveToXY( rectIndex.right + 2, ( rectTextIndexSize.Height() - rectValue.Height() ) / 2 );
	dcImage.DrawText( strValue, &rectValue, DT_CENTER | DT_NOPREFIX );
	dcImageSelected.DrawText( strValue, &rectValue, DT_CENTER | DT_NOPREFIX );

	CRect rectAllTextSize;
	rectAllTextSize.UnionRect( rectTextIndexSize, rectValue );

	CBitmap *pclImageModify = dcImage.SelectObject( pclOldImage );
	CBitmap *pclImageSelectedModify = dcImageSelected.SelectObject( pclOldImageSelected );

	CImageList clImageList;
	clImageList.Create( rectAllTextSize.Width(), rectAllTextSize.Height(), ILC_COLOR32 | ILC_MASK, 0, 1 );
	clImageList.SetBkColor( CLR_NONE );
	int iImageIndex = clImageList.Add( pclImageModify, _WHITE );
	int iImageSelectedIndex = clImageList.Add( pclImageSelectedModify, pclSheet->GetDefaultSelectionColor() );

	HICON hImage = clImageList.ExtractIcon( iImageIndex );
	HICON hImageSelected = clImageList.ExtractIcon( iImageSelectedIndex );
	pclSheet->SetPictureCellWithHandle( ( HANDLE )hImage, lColumn, lRow, false, VPS_ICON | VPS_CENTER, ( HANDLE )hImageSelected );

	m_vecImageList.push_back( hImage );
	m_vecImageSelectedList.push_back( hImageSelected );

	clImageList.DeleteImageList();
	clFontIndex.DeleteObject();
	clImage.DeleteObject();
	clImageSelected.DeleteObject();
	dcImage.DeleteDC();
	dcImageSelected.DeleteDC();

	return ++lRow;
}

void CSelProdPagePressMaint::_FillCurves( CDS_SSelPMaint *pclSSelPressMaint )
{
	if( NULL == pclSSelPressMaint || NULL == pclSSelPressMaint->GetpInputUser() || NULL == pclSSelPressMaint->GetpInputUser()->GetPMWQSelectionPreferences() )
	{
		return;
	}

	CSelectPMList clSelectPMList;
	clSelectPMList.GetpclPMInputUser()->CopyFrom( pclSSelPressMaint->GetpInputUser() );
	clSelectPMList.SetpTADB( pclSSelPressMaint->GetpInputUser()->GetpTADB() );
	clSelectPMList.SetpTADS( pclSSelPressMaint->GetpInputUser()->GetpTADS() );

	// Call base class method to get a new 'Sheet'.
	CSheetDescription *pclSDGraphs = CMultiSpreadBase::CreateSSheet( SD_Graphs + m_uiSDGraphsCount );

	if( NULL == pclSDGraphs || NULL == pclSDGraphs->GetSSheetPointer() )
	{
		return;
	}

	CSSheet *pclSheet = pclSDGraphs->GetSSheetPointer();

	if( false == CSelProdPageBase::PrepareSheet( pclSDGraphs, CD_G_LastColumn, m_bForPrint, false ) )
	{
		m_ViewDescription.RemoveOneSheetDescription( SD_Graphs + m_uiSDGraphsCount );
		return;
	}

	m_uiSDGraphsCount++;

	// Initialize column width for header and footer column.
	if( false == m_bForPrint )
	{
		SetColWidth( pclSDGraphs, CD_G_Header, DefaultColumnWidth::DCW_Header );
		SetColWidth( pclSDGraphs, CD_G_Footer, DefaultColumnWidth::DCW_Footer );
	}
	else
	{
		pclSheet->ShowCol( CD_G_Header, FALSE );
		pclSheet->ShowCol( CD_G_Footer, FALSE );
	}

	pclSheet->SetMaxRows( RD_G_Graphs );
	pclSheet->SetRowHeight( RD_G_Header, m_dRowHeight * 0.5 );
	pclSheet->SetCellBorder( CD_G_Separator1, RD_G_Header, CD_G_Separator3, RD_G_Header, true, SS_BORDERTYPE_TOP, SS_BORDERSTYLE_SOLID );

	// Set the column width. To perfectly match the main title, we use proportionality.
	int iHFColPixels = ( false == m_bForPrint ) ? pclSheet->ColWidthToLogUnits( 2.0 ) : 0;
	double dAvailableWidth = pclSheet->LogUnitsToColWidthW( RIGHTVIEWWIDTH - 2 * iHFColPixels );

	// To adapt column width, we take default defined width and do a proportionality.
	double dTotalColWidth = GCW_Separator1 + GCW_Bitmap1 + GCW_Separator2 + GCW_Bitmap2 + GCW_Separator3;

	double dWidth = dAvailableWidth * GCW_Separator1 / dTotalColWidth;
	double dTotalWidth = dWidth;
	SetColWidth( pclSDGraphs, CD_G_Separator1, dWidth );

	dWidth = dAvailableWidth * GCW_Bitmap1 / dTotalColWidth;
	dTotalWidth += dWidth;
	SetColWidth( pclSDGraphs, CD_G_Bitmap1, dWidth );

	dWidth = dAvailableWidth * GCW_Separator2 / dTotalColWidth;
	dTotalWidth += dWidth;
	SetColWidth( pclSDGraphs, CD_G_Separator2, dWidth );

	dWidth = dAvailableWidth * GCW_Bitmap2 / dTotalColWidth;
	dTotalWidth += dWidth;
	SetColWidth( pclSDGraphs, CD_G_Bitmap2, dWidth );

	SetColWidth( pclSDGraphs, CD_G_Separator3, dAvailableWidth - dTotalWidth );

	pclSheet->ShowCol( CD_G_LastColumn, FALSE );

	// HYS-1571: We can optimize the column width now.
	// See Jira card to have more details about why we do that here.
	CRect rectClient;
	GetClientRect( &rectClient );
	OptimizePageSize( rectClient.Width(), rectClient.Height() );

	CRect rect = pclSheet->GetSheetSizeInPixels( false );
	m_mapSheetInfos[pclSDGraphs].m_dPageWidth = rect.Width();

	CDlgPMTChartHelper *pclPressureGraph = NULL;
	HBITMAP hPressureGauge = NULL;

	if( CDS_SSelPMaint::SelectionType::Statico == pclSSelPressMaint->GetSelectionType() )
	{
		CSelectedVssl clSelectedVessel;
		clSelectedVessel.SetMinimumInitialPressure( pclSSelPressMaint->GetpInputUser()->GetMinimumInitialPressure() );
		clSelectedVessel.SetInitialPressure( pclSSelPressMaint->GetVesselInitialPressure() );
		clSelectedVessel.SetWaterReserve( pclSSelPressMaint->GetVesselWaterReserve() );
		clSelectedVessel.SetProductIDPtr( pclSSelPressMaint->GetVesselIDPtr() );
		clSelectedVessel.SetNbreOfVsslNeeded( pclSSelPressMaint->GetVesselNumber() );
		pclPressureGraph = clSelectPMList.GetExpansionVesselCurves( &clSelectedVessel, (CWnd*)pclSheet );

		double dp0 = pclSSelPressMaint->GetpInputUser()->GetMinimumPressure();
		double dpa = pclSSelPressMaint->GetVesselInitialPressure();
		double dpe = pclSSelPressMaint->GetpInputUser()->GetFinalPressure();
		double dPSV = pclSSelPressMaint->GetpInputUser()->GetSafetyValveResponsePressure();
		hPressureGauge = clSelectPMList.GetPressureGauge( dp0, dpa, dpe, dPSV, (CWnd*)pclSheet );
	}
	else if( CDS_SSelPMaint::SelectionType::Compresso == pclSSelPressMaint->GetSelectionType()
			|| CDS_SSelPMaint::SelectionType::Transfero == pclSSelPressMaint->GetSelectionType() )
	{
		CDB_TecBox *pclTecBox = dynamic_cast<CDB_TecBox*>( pclSSelPressMaint->GetTecBoxCompTransfIDPtr().MP );

		if( NULL != pclTecBox )
		{
			double dPman = pclSSelPressMaint->GetpInputUser()->GetTargetPressureForTecBox( pclTecBox->GetTecBoxType() );
			double dVD = pclSSelPressMaint->GetpInputUser()->GetVD();

			if( CDS_SSelPMaint::SelectionType::Compresso == pclSSelPressMaint->GetSelectionType() )
			{
				// HYS-599: we can have more that on Compresso CX in parallel.
				if( true == pclTecBox->IsVariantExternalAir() )
				{
					int iNbrOfDevices = pclSSelPressMaint->GetTecBoxCompTransfNumber();
					// HYS-1537 : Look the disable state
					if( true == pclSSelPressMaint->GetpInputUser()->GetPMWQSelectionPreferences()->IsChecked( CPMWQPrefs::GeneralRedundancyTecBox ) 
						&& false == pclSSelPressMaint->GetpInputUser()->GetPMWQSelectionPreferences()->IsDisabled( CPMWQPrefs::GeneralRedundancyTecBox ) )
					{
						iNbrOfDevices >>= 1;
					}

					pclPressureGraph = clSelectPMList.GetCompressoCurves( pclTecBox, dPman, dVD, (CWnd*)pclSheet, iNbrOfDevices );
				}
				else
				{
					pclPressureGraph = clSelectPMList.GetCompressoCurves( pclTecBox, dPman, dVD, (CWnd*)pclSheet, -1 );
				}
			}
			else
			{
				pclPressureGraph = clSelectPMList.GetTransferoCurves( pclTecBox, dPman, dVD, (CWnd*)pclSheet );
			}

			double dp0 = pclSSelPressMaint->GetpInputUser()->GetMinimumPressure();
			double dpa = pclSSelPressMaint->GetpInputUser()->GetMinimumInitialPressure();
			double dpe = pclSSelPressMaint->GetpInputUser()->GetFinalPressure( pclTecBox->GetTecBoxType() );
			double dPSV = pclSSelPressMaint->GetpInputUser()->GetSafetyValveResponsePressure();
			hPressureGauge = clSelectPMList.GetPressureGauge( dp0, dpa, dpe, dPSV, (CWnd*)pclSheet );
		}
	}

	int iMaxHeight = 0;
	int iGraph1Height = 0;
	int iGraph2Height = 0;
	long lStyle = VPS_BMP | VPS_CENTER;

	if( true == m_bForPrint )
	{
		lStyle |= ( VPS_STRETCH | VPS_MAINTAINSCALE );
	}

	if( NULL != pclPressureGraph )
	{
		// Retrieve a BMP copy in the clipboard of the valves graph.
		pclPressureGraph->GetExport().CopyToClipboardBitmap();

		if( FALSE == IsClipboardFormatAvailable( CF_BITMAP ) )
		{
			return;
		}
		
		::OpenClipboard( NULL );
		HGLOBAL hBitmap = ::GetClipboardData( CF_BITMAP );
		
		if( NULL != hBitmap )
		{
			// Copy bitmap.
			CEnBitmap clEnBitmap;
			clEnBitmap.CopyImage( (HBITMAP)hBitmap );

			CRect rectGraph;
			pclPressureGraph->GetClientRect( &rectGraph );

			iGraph1Height = rectGraph.Height();
			iMaxHeight = iGraph1Height;

			// Set the bitmap in spread.
			pclSheet->SetPictureCellWithHandle( (HBITMAP)clEnBitmap, CD_G_Bitmap1, RD_G_Graphs, true, lStyle );
		}

		::CloseClipboard();
	}

	if( NULL != hPressureGauge )
	{
		int iCol = ( NULL != pclPressureGraph ) ? CD_G_Bitmap2 : CD_G_Bitmap1;

		CBitmap *pclBitmap = CBitmap::FromHandle( hPressureGauge );

		iGraph2Height = pclBitmap->GetBitmapDimension().cy;
		iMaxHeight = max( iMaxHeight, iGraph2Height );
		
		pclSheet->SetPictureCellWithHandle( hPressureGauge, iCol, RD_G_Graphs, true, lStyle );
	}

	if( NULL != hPressureGauge )
	{
		DeleteObject( hPressureGauge );
	}

	if( NULL != pclPressureGraph )
	{
		delete pclPressureGraph;
	}

	pclSheet->SetCellBorder( CD_G_Separator1, RD_G_Graphs, CD_G_Separator3, RD_G_Graphs, true, SS_BORDERTYPE_BOTTOM, SS_BORDERSTYLE_SOLID );
	pclSheet->SetRowHeightInPixels( RD_G_Graphs, iMaxHeight );

	// Set the selection range.
	m_clCPMGParam.m_pclSheetDescription = pclSDGraphs;
	m_clCPMGParam.m_lpSelectionContainer = (LPARAM)pclSSelPressMaint;
	m_clCPMGParam.AddRange( RD_G_Header, RD_G_Graphs, NULL );

	// Set the scrolling range (for the scrolling, we include the title)
	m_clCPMGParam.SetScrollRange( RD_G_Header, RD_G_Graphs );
}