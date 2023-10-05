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
#include "SelProdPagePDef.h"

CSelProdPagePDef::CSelProdPagePDef( CArticleGroupList *pclArticleGroupList ) 
	: CSelProdPageBase( CDB_PageSetup::enCheck::PARTDEF, pclArticleGroupList )
{
}

void CSelProdPagePDef::Reset( void )
{
	CSelProdPageBase::Reset();
}

bool CSelProdPagePDef::PreInit( HMvector& vecHMList )
{
	// Something for individual or direct selection ?
	CTable* pclTable = (CTable*)( TASApp.GetpTADS()->Get( _T("PARTDEF_TAB") ).MP );
	if( NULL == pclTable )
	{
		ASSERT( 0 );
		return false;
	}

	m_pclSelectionTable = ( '\0' != *pclTable->GetFirst().ID ) ? pclTable : NULL;
	bool fPDExist = ( NULL != m_pclSelectionTable );

	// Something for hydronic calculation?
	bool fPDHMExist = ( vecHMList.size() > 0 );

	if( false == fPDExist && false == fPDHMExist )
		return false;
	
	m_vecHMList = vecHMList;
	SortTable();
	return true;
}

bool CSelProdPagePDef::Init( bool fResetOrder, bool fPrint )
{
	bool fPDExist = ( NULL != m_pclSelectionTable ) ? true : false;
	bool fPDHMExist = ( m_vecHMList.size() > 0 ) ? true : false;

	if( false == fPDExist && false == fPDHMExist )
	{
		return false;
	}

	// Call base class method to get a new 'Sheet'.
	CSheetDescription *pclSDPartiallyDefined = CMultiSpreadBase::CreateSSheet( SD_PartDef );

	if( NULL == pclSDPartiallyDefined || NULL == pclSDPartiallyDefined->GetSSheetPointer() )
	{
		return false;
	}
	
	CSSheet *pclSheet = pclSDPartiallyDefined->GetSSheetPointer();

	if( false == CSelProdPageBase::PrepareSheet( pclSDPartiallyDefined, ColumnDescription::Pointer, fPrint ) )
	{
		m_ViewDescription.RemoveOneSheetDescription( SD_PartDef );
		return false;
	}

	// Initialize column width for header and footer column.
	if( false == m_bForPrint )
	{
		SetColWidth( pclSDPartiallyDefined, ColumnDescription::Header, DefaultColumnWidth::DCW_Header );
		SetColWidth( pclSDPartiallyDefined, ColumnDescription::Footer, DefaultColumnWidth::DCW_Footer );
	}
	else
	{
		pclSheet->ShowCol( ColumnDescription::Header, FALSE );
		pclSheet->ShowCol( ColumnDescription::Footer, FALSE );
	}

	double dWidth = ( true == m_pTADSPageSetup->GetField( PageField_enum::epfFIRSTREF ) ) ? DefaultColumnWidth::DCW_Reference1 : 0.0;
	SetColWidth( pclSDPartiallyDefined, ColumnDescription::Reference1, dWidth );

	dWidth = ( true == m_pTADSPageSetup->GetField( PageField_enum::epfSECONDREF ) ) ? DefaultColumnWidth::DCW_Reference2 : 0.0;
	SetColWidth( pclSDPartiallyDefined, ColumnDescription::Reference2, dWidth );
	
	dWidth = ( true == m_pTADSPageSetup->GetField( PageField_enum::epfWATERINFO ) ) ? DefaultColumnWidth::DCW_Water : 0.0;
	SetColWidth( pclSDPartiallyDefined, ColumnDescription::Water, dWidth );
	
	dWidth = ( true == m_pTADSPageSetup->GetField( PageField_enum::epfPDINFOPRODUCT ) ) ? DefaultColumnWidth::DCW_Product : 0.0;
	SetColWidth( pclSDPartiallyDefined, ColumnDescription::Product, dWidth );
	
	SetColWidth( pclSDPartiallyDefined, ColumnDescription::TechnicalInfos, 0.0 );
	SetColWidth( pclSDPartiallyDefined, ColumnDescription::RadiatorInfos, 0.0 );
	SetColWidth( pclSDPartiallyDefined, ColumnDescription::ArticleNumber, 0.0 );
	
	dWidth = ( true == m_pTADSPageSetup->GetField( PageField_enum::epfPIPEINFO ) ) ? DefaultColumnWidth::DCW_Pipes : 0.0;
	SetColWidth( pclSDPartiallyDefined, ColumnDescription::Pipes, dWidth );
	
	dWidth = ( true == m_pTADSPageSetup->GetField( PageField_enum::epfQUANTITY ) ) ? DefaultColumnWidth::DCW_Quantity : 0.0;
	SetColWidth( pclSDPartiallyDefined, ColumnDescription::Quantity, dWidth );
	
	dWidth = 0.0;
	
	if( true == TASApp.IsPriceUsed() )
	{
		dWidth = ( true == m_pTADSPageSetup->GetField( PageField_enum::epfSALEUNITPRICE ) ) ? DefaultColumnWidth::DCW_UnitPrice : 0.0;
	}
	
	SetColWidth( pclSDPartiallyDefined, ColumnDescription::UnitPrice, dWidth );

	dWidth = 0.0;
	
	if( true == TASApp.IsPriceUsed() )
	{
		dWidth = ( true == m_pTADSPageSetup->GetField( PageField_enum::epfSALETOTALPRICE ) ) ? DefaultColumnWidth::DCW_TotalPrice : 0.0;
	}

	
	SetColWidth( pclSDPartiallyDefined, ColumnDescription::TotalPrice, dWidth );
	
	
	SetColWidth( pclSDPartiallyDefined, ColumnDescription::Remark, 0.0 );
	
	
	SetColWidth( pclSDPartiallyDefined, ColumnDescription::Pointer, DefaultColumnWidth::DCW_Pointer );
	
	pclSheet->ShowCol( ColumnDescription::TechnicalInfos, FALSE );
	pclSheet->ShowCol( ColumnDescription::RadiatorInfos, FALSE );
	pclSheet->ShowCol( ColumnDescription::ArticleNumber, FALSE );

	if( false == TASApp.IsPriceUsed() )
	{
		pclSheet->ShowCol( ColumnDescription::UnitPrice, FALSE );
		pclSheet->ShowCol( ColumnDescription::TotalPrice, FALSE );
	}

	pclSheet->ShowCol( ColumnDescription::Remark, FALSE );
	pclSheet->ShowCol( ColumnDescription::Pointer, FALSE );
	CRect rect = pclSheet->GetSheetSizeInPixels( false );
	m_mapSheetInfos[pclSDPartiallyDefined].m_dPageWidth = rect.Width();

	// Page title.
	SetPageTitle( pclSDPartiallyDefined, IDS_RVIEWSELP_TITLE_DIRSEL1 );
	
	// Init column header.
	_InitColHeader( pclSDPartiallyDefined );
	pclSheet->SetColumnAlwaysHidden( ColumnDescription::TechnicalInfos, true );
	pclSheet->SetColumnAlwaysHidden( ColumnDescription::RadiatorInfos, true );
	pclSheet->SetColumnAlwaysHidden( ColumnDescription::ArticleNumber, true );
	pclSheet->SetColumnAlwaysHidden( ColumnDescription::UnitPrice, true );
	pclSheet->SetColumnAlwaysHidden( ColumnDescription::TotalPrice, true );
	pclSheet->SetColumnAlwaysHidden( ColumnDescription::Remark, true );

	// HYS-1571: We can optimize the column width now.
	// See Jira card to have more details about why we do that here.
	CRect rectClient;
	GetClientRect( &rectClient );
	OptimizePageSize( rectClient.Width(), rectClient.Height() );

	long lSelectedRow = 0;

	if( true == fPDExist )
	{
		// Number of objects.
		int iPDCount = m_pclSelectionTable->GetItemCount( CLASS( CDS_DirectSel ) );

		if( 0 == iPDCount )
		{
			return false;
		}

		CDS_DirectSel **paPartDef = new CDS_DirectSel*[iPDCount];

		if( NULL == paPartDef )
		{
			return false;
		}
	
		// Fill 'paPartDef' array with pointer on each partially defined object of the 'PARTDEF_TAB' table.
		int iMaxIndex = 0;
		int i = 0;
		
		for( IDPTR IDPtr = m_pclSelectionTable->GetFirst( CLASS( CDS_DirectSel ) ); _T('\0') != *IDPtr.ID; IDPtr = m_pclSelectionTable->GetNext() )
		{
			CDS_DirectSel *pclPartDef = dynamic_cast<CDS_DirectSel *>( IDPtr.MP );

			if( NULL == pclPartDef )
			{
				continue;
			}

			// Skip object with ValveID.
			if( _T('\0') != *pclPartDef->GetID() )
			{
				continue;
			}
			
			paPartDef[i] = pclPartDef;

			if( iMaxIndex < paPartDef[i]->GetRowIndex() )
			{
				iMaxIndex = paPartDef[i]->GetRowIndex();
			}
			
			i++;
		}

		ASSERT( i == iPDCount );
		iPDCount = i;
		
		// Sort pDirectSel Array.
		// Remark: 'fResetOrder' is set to 'true' only when the sorting combos are empty (No user choice) and the user clicks on the 'Apply sorting keys'.
		if( true == fResetOrder )
		{
			// Reset row index.
			for( i = 0; i < iPDCount; i++ )
			{
				if( 0 == paPartDef[i]->GetRowIndex() )
				{
					paPartDef[i]->SetRowIndex( ++iMaxIndex );
				}
			}
		}
		else
		{
			if( NULL != pDlgLeftTabSelP )
			{
				pDlgLeftTabSelP->SortTable( (CData**)paPartDef, iPDCount - 1 );
			}

			// Reset row index.
			for( i = 0; i < iPDCount; i++ )
			{
				paPartDef[i]->SetRowIndex( i + 1 );
			}
		}

		// Create an Remark index. 
		if( NULL != pDlgLeftTabSelP )
		{
			pDlgLeftTabSelP->SetRemarkIndex( CLASS( CDS_DirectSel ), (CData**)paPartDef, iPDCount );
		}

		//====================================
		// Fill and format the table
		//====================================
		long lRow = pclSheet->GetMaxRows();
		LPARAM lparam;
		CRank rkl;

		for( i = 0; i < iPDCount; i++ )
		{
			rkl.Add( _T(""), paPartDef[i]->GetRowIndex(), (LPARAM)paPartDef[i] );
		}
		
		bool fFirstPass = true;
		CString str;

		for( BOOL fContinue = rkl.GetFirst( str, lparam ); TRUE == fContinue; fContinue = rkl.GetNext( str, lparam ) )
		{
			CDS_DirectSel *pSel = (CDS_DirectSel *)lparam;
			
			if( m_pSelected == (CDS_DirectSel *)pSel )
			{
				lSelectedRow = lRow;
			}

			m_rProductParam.Clear();
			m_rProductParam.SetSheetDescription( pclSDPartiallyDefined );
			m_rProductParam.SetSelectionContainer( (LPARAM)pSel );

			long lRowDSel = _FillRow( pclSDPartiallyDefined, lRow, pSel );

			// Save product.
			m_rProductParam.SetScrollRange( ( true == fFirstPass ) ? lRow - 1 : lRow, lRowDSel );
			SaveProduct( m_rProductParam );
	
			// Draw line below.
			pclSheet->SetCellBorder( ColumnDescription::Reference1, lRowDSel, ColumnDescription::Footer - 1, lRowDSel, true, SS_BORDERTYPE_BOTTOM );

			// Set all group as no breakable (for print).
			// Remark: include title with the group.
			pclSheet->SetFlagRowNoBreakable( ( true == fFirstPass ) ? lRow - 1 : lRow, lRowDSel, true );

			lRow = lRowDSel + 1;
			fFirstPass = false;
		}
		lRow++;
		
		delete[] paPartDef;
	}

	if( true == fPDHMExist )
	{
		if( m_vecHMList.size() > 0 )	
		{
			// Create a remark index.
			if( NULL != pDlgLeftTabSelP )
			{
				pDlgLeftTabSelP->SetRemarkIndex( &m_vecHMList );
			}

			long lRow = pclSheet->GetMaxRows();

			if( true == fPDExist )
			{
				lRow++;
			}

			SetLastRow( pclSDPartiallyDefined, lRow );
			
			// For each element.
			bool fFirstPass = true;
			HMvector::iterator It;
			
			for( It = m_vecHMList.begin(); It != m_vecHMList.end(); It++ )
			{
				CDS_HydroMod *pHM = NULL;
				
				if( eptHM == (*It).first )
				{
					pHM = dynamic_cast<CDS_HydroMod *>((CData*)( (*It).second ) );
				}
				
				if( NULL != pHM )
				{
					m_rProductParam.Clear();
					m_rProductParam.SetSheetDescription( pclSDPartiallyDefined );
					// Intentionally set to NULL to avoid clicking, double clicking or right clicking.
					m_rProductParam.SetSelectionContainer( (LPARAM)0 );

					long lRowBV = _FillRow( pclSDPartiallyDefined, lRow, pHM );
					long lRowGen = CSelProdPageBase::FillRowGen( pclSDPartiallyDefined, lRow, pHM );
					long lLastRow = max( lRowGen, lRowBV );

					// Save the product.
					m_rProductParam.SetScrollRange( ( true == fFirstPass ) ? lRow - 1 : lRow, lLastRow );
					SaveProduct( m_rProductParam );

					// Draw line below.
					pclSheet->SetCellBorder( ColumnDescription::Reference1, lLastRow, ColumnDescription::Footer - 1, lLastRow, true, SS_BORDERTYPE_BOTTOM );

					// Set all group as no breakable (for print).
					// Remark: include title with the group.
					if( true == fFirstPass )
					{
						pclSheet->SetFlagRowNoBreakable( lRow - 1, lLastRow, true );
					}
					else
					{
						pclSheet->SetFlagRowNoBreakable( lRow, lLastRow, true );
					}

					lRow = lLastRow + 1;
					fFirstPass = false;
				}
			}

			pclSheet->SetMaxRows( lRow );
			lRow = FillHMRemarks( &m_vecHMList, pclSDPartiallyDefined, lRow );
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

bool CSelProdPagePDef::HasSomethingToDisplay( void )
{
	bool fPDExist = ( NULL != m_pclSelectionTable ) ? true : false;
	bool fPDHMExist = ( m_vecHMList.size() > 0 ) ? true : false;
	if( false == fPDExist && false == fPDHMExist )
		return false;
	return true;
}

void CSelProdPagePDef::_InitColHeader( CSheetDescription* pclSheetDescription )
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

long CSelProdPagePDef::_FillRow( CSheetDescription* pclSheetDescription, long lRow, CDS_DirectSel* pclPartDef )
{
	if( NULL == pclSheetDescription || NULL == pclSheetDescription->GetSSheetPointer() || NULL == pclPartDef )
		return lRow;

	CSSheet* pclSheet = pclSheetDescription->GetSSheetPointer();
	CString str1, str2;
	pclSheet->SetTextPattern( CSSheet::_SSTextPattern::UserStaticColumn );
	long lFirstRow = lRow;

	pclSheet->SetTextPatternProperty( CSSheet::_SSTextPatternProperty::TPP_FontBold, (LPARAM)FALSE );
	CDB_TAProduct* pTAP = ( CDB_TAProduct *)( TASApp.GetpTADB()->Get( pclPartDef->GetID() ).MP );

	// Column Reference 1.
	pclSheet->SetRowHeight( lRow, m_dRowHeight );
	pclSheet->FormatEditText( ColumnDescription::Reference1, lRow, pclPartDef->GetRef1() );

	// Column Reference 2.
	pclSheet->FormatEditText( ColumnDescription::Reference2, lRow, pclPartDef->GetRef2() );

	// Column TA Product.
	AddStaticText( pclSheetDescription, ColumnDescription::Product, lRow, _T("-") );
	if( *pclPartDef->GetModuleID() ) 			// !!! Floating directSel without reference to the HydroMod
	{
		CDS_HydroMod* pHM = ( CDS_HydroMod*)( TASApp.GetpTADS()->Get( pclPartDef->GetModuleID() ).MP );
		if( NULL != pHM )
		{
			if( edt_KvCv != pHM->GetVDescrType() )
			{
				if( true == m_pTADSPageSetup->GetField( epfPDINFOPRODUCTNAME ) )	
					lRow = AddStaticText( pclSheetDescription, ColumnDescription::Product, lRow, pHM->GetCBIType() );
				
				if( true == m_pTADSPageSetup->GetField( epfPDINFOPRODUCTSIZE) )	
				{
					str1 += pHM->GetCBISize();
					if( edt_CBISizeInchValve == pHM->GetVDescrType() )
						str1 += CString( _T("\"") );
					lRow = AddStaticText( pclSheetDescription, ColumnDescription::Product, lRow, str1 );
				}
			}
			else
			{
				if( true == m_pTADSPageSetup->GetField( epfPDINFOPRODUCTNAME ) )	
				{
					if( 0 == m_pUnitDB->GetDefaultUnitIndex( _C_KVCVCOEFF ) )
						str1 = TASApp.LoadLocalizedString( IDS_KV );
					else
						str1 = TASApp.LoadLocalizedString( IDS_CV );
					str1 += (CString)_T(" = ") + WriteCUDouble( _C_KVCVCOEFF, pHM->GetKvCv() );
					lRow = AddStaticText( pclSheetDescription, ColumnDescription::Product, lRow, str1 );
				}
			}
		}
	}

	// Column article number.
	lRow = lFirstRow;
	AddStaticText( pclSheetDescription, ColumnDescription::ArticleNumber, lRow, _T("-") );

	// Column Tech Infos.
	if( true == m_pTADSPageSetup->GetField( epfDSTECHINFO ) && *pclPartDef->GetModuleID() )
	{
		// !!! Floating directSel without reference to the HydroMod
		CDS_HydroMod* pHM = ( CDS_HydroMod*)( TASApp.GetpTADS()->Get( pclPartDef->GetModuleID() ).MP );
		if( NULL == pHM )
			TRACE( _T("Broken cross reference %s -> %s\n"), pclPartDef->GetID(), pclPartDef->GetModuleID() );
		
		if( true == pclPartDef->IsTAProduct() && pHM->GetQDesign() > 0.0 )
			lRow = AddStaticText( pclSheetDescription, ColumnDescription::TechnicalInfos, lRow, WriteCUDouble( _U_FLOW, pHM->GetQDesign(), true ) );
	}

	// Column Quantity.
	lRow = lFirstRow;
	if( true == m_pTADSPageSetup->GetField( epfQUANTITY ) )
	{
		str1.Format( _T("%d"), pclPartDef->GetQuantity() );
		AddStaticText( pclSheetDescription, ColumnDescription::Quantity, lRow++, str1 );
	}
	
	// Column Remark.
	lRow = lFirstRow;
	if( true == m_pTADSPageSetup->GetField( epfREMARK ) )	
	{
		if( pclPartDef->GetRemarkIndex() )		// remark exist
		{
			str1.Format( _T("%d"), pclPartDef->GetRemarkIndex() );
			lRow = AddStaticText( pclSheetDescription, ColumnDescription::Remark, lRow, str1 );
		}
	}

	m_rProductParam.AddRange( lFirstRow, pclSheet->GetMaxRows(), pTAP );
	return pclSheet->GetMaxRows();
}

long CSelProdPagePDef::_FillRow( CSheetDescription* pclSheetDescription, long lRow, CDS_HydroMod* pHM )
{
	if( NULL == pclSheetDescription || NULL == pclSheetDescription->GetSSheetPointer() )
		return lRow;

	CSSheet* pclSheet = pclSheetDescription->GetSSheetPointer();
	CString str1, str2;
	long lFirstRow = lRow;
	pclSheet->SetTextPattern( CSSheet::_SSTextPattern::UserStaticColumn );
	pclSheet->SetTextPatternProperty( CSSheet::_SSTextPatternProperty::TPP_FontBold, (LPARAM)FALSE );

	// Column TA Product.
	AddStaticText( pclSheetDescription, ColumnDescription::Product, lRow, _T("-") );
	if( NULL != pHM )
	{
		if( edt_KvCv != pHM->GetVDescrType() )
		{
			if( true == m_pTADSPageSetup->GetField( epfPDINFOPRODUCTNAME ) )	
				lRow = AddStaticText( pclSheetDescription, ColumnDescription::Product, lRow, pHM->GetCBIType() );
			
			if( true == m_pTADSPageSetup->GetField( epfPDINFOPRODUCTSIZE) )	
			{
				str1 += pHM->GetCBISize();
				if( edt_CBISizeInchValve == pHM->GetVDescrType() )
					str1 += CString( _T("\"") );
				lRow = AddStaticText( pclSheetDescription, ColumnDescription::Product, lRow, str1 );
			}
		}
		else
		{
			if( true == m_pTADSPageSetup->GetField( epfPDINFOPRODUCTNAME ) )	
			{
				if( 0 == m_pUnitDB->GetDefaultUnitIndex( _C_KVCVCOEFF ) )
					str1 = TASApp.LoadLocalizedString( IDS_KV );
				else
					str1 = TASApp.LoadLocalizedString( IDS_CV );
				
				str1 += (CString)_T(" = ") + WriteCUDouble( _C_KVCVCOEFF, pHM->GetKvCv() );
				lRow = AddStaticText( pclSheetDescription, ColumnDescription::Product, lRow, str1 );
			}
		}
	}

	// Column Tech Infos.
	lRow = lFirstRow;
	AddStaticText( pclSheetDescription, ColumnDescription::ArticleNumber, lRow, _T("-") );
	str1=_T("");
	
	// Column Quantity.
	lRow = lFirstRow;
	if( true == m_pTADSPageSetup->GetField( epfQUANTITY ) )
		str1.Format( _T("%d"), 1 );
	AddStaticText( pclSheetDescription, ColumnDescription::Quantity, lRow++, str1 );
	
	return pclSheet->GetMaxRows();
}
