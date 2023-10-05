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
#include "SelProdPageTender.h"
#include "DlgTender.h"
#include <map>

CSelProdPageTender::CSelProdPageTender( CArticleGroupList *pclArticleGroupList )
	: CSelProdPageBase( CDB_PageSetup::enCheck::TENDERTEXT, pclArticleGroupList )
{
	m_pSelected = NULL;
	m_fTAArtNumberShown = true;
	m_fLocalArtNumberShown = false;
}

void CSelProdPageTender::Reset( void )
{
	CSelProdPageBase::Reset();
}

bool CSelProdPageTender::PreInit( HMvector &vecHMList )
{
	// Something for individual or direct selection ?
	CTender tender;

	if( false == tender.IsTenderNeeded() )
	{
		return false;
	}

	if( false == HasSomethingToDisplay() )
	{
		return false;
	}

	return true;
}

bool CSelProdPageTender::Init( bool fResetOrder, bool fPrint )
{
	// Don't verify here if there is something to print. Because if there is no product to display, we show at least the article list.

	// Call 'MultiSpreadBase' method to get a new 'Sheet'.
	CSheetDescription *pclSDArticleList = CMultiSpreadBase::CreateSSheet( SD_TenderText );

	if( NULL == pclSDArticleList || NULL == pclSDArticleList->GetSSheetPointer() )
	{
		return false;
	}

	CSSheet *pclSheet = pclSDArticleList->GetSSheetPointer();

	if( false == CSelProdPageBase::PrepareSheet( pclSDArticleList, ColumnDescriptionPTender::Pointer, fPrint ) )
	{
		m_ViewDescription.RemoveOneSheetDescription( SD_TenderText );
		return false;
	}

	// Initialize column width for header and footer column.
	if( false == m_bForPrint )
	{
		SetColWidth( pclSDArticleList, ColumnDescriptionPTender::Header, DefaultColumnWidth::DCW_Header );
		SetColWidth( pclSDArticleList, ColumnDescriptionPTender::Footer, DefaultColumnWidth::DCW_Footer );
	}
	else
	{
		pclSheet->ShowCol( ColumnDescriptionPTender::Header, FALSE );
		pclSheet->ShowCol( ColumnDescriptionPTender::Footer, FALSE );
	}

	m_fTAArtNumberShown = !TASApp.IsTAArtNumberHidden();
	m_fLocalArtNumberShown = TASApp.IsLocalArtNumberUsed();

	if( true == m_fTAArtNumberShown )
	{
		SetColWidth( pclSDArticleList, ColumnDescriptionPTender::ArticleNumber, DefaultColumnWidth::DCW_ArticleNumber );
		// HYS-1304 : We have only 2 columns shown. If the article number column takes 10 then this colum can take 74
		SetColWidth( pclSDArticleList, ColumnDescriptionPTender::TenderText, 74 );
	}
	else
	{
		SetColWidth( pclSDArticleList, ColumnDescriptionPTender::ArticleNumber, 0 );
		pclSheet->ShowCol( ColumnDescriptionPTender::ArticleNumber, FALSE );
		SetColWidth( pclSDArticleList, ColumnDescriptionPTender::TenderText, 0 );
		pclSheet->ShowCol( ColumnDescriptionPTender::TenderText, FALSE );
	}

	if( true == m_fLocalArtNumberShown )
	{
		SetColWidth( pclSDArticleList, ColumnDescriptionPTender::LocalArticleNumber, DefaultColumnWidth::DCW_ArticleNumber );
	}
	else
	{
		SetColWidth( pclSDArticleList, ColumnDescriptionPTender::LocalArticleNumber, 0 );
		pclSheet->ShowCol( ColumnDescriptionPTender::LocalArticleNumber, FALSE );
	}

	SetColWidth( pclSDArticleList, ColumnDescriptionPTender::Description, 55 );
	SetColWidth( pclSDArticleList, ColumnDescriptionPTender::Quantity, DefaultColumnWidth::DCW_Quantity );
	SetColWidth( pclSDArticleList, ColumnDescriptionPTender::UnitPrice, 0 );
	SetColWidth( pclSDArticleList, ColumnDescriptionPTender::TotalPrice, 0 );
	SetColWidth( pclSDArticleList, ColumnDescriptionPTender::Pointer, DefaultColumnWidth::DCW_Pointer );
	pclSheet->ShowCol( ColumnDescriptionPTender::Description, FALSE );
	pclSheet->ShowCol( ColumnDescriptionPTender::Quantity, FALSE );
	pclSheet->ShowCol( ColumnDescriptionPTender::UnitPrice, FALSE );
	pclSheet->ShowCol( ColumnDescriptionPTender::TotalPrice, FALSE );
	pclSheet->ShowCol( ColumnDescriptionPTender::Pointer, FALSE );

	CRect rect = pclSheet->GetSheetSizeInPixels( false );
	m_mapSheetInfos[pclSDArticleList].m_dPageWidth = rect.Width();

	// Page title.
	SetPageTitle( pclSDArticleList, GetTabTitleID() );

	// Init column header.
	_InitColHeader( pclSDArticleList );

	// HYS-1571: We can optimize the column width now.
	// See Jira card to have more details about why we do that here.
	CRect rectClient;
	GetClientRect( &rectClient );
	OptimizePageSize( rectClient.Width(), rectClient.Height() );
	
	/////////////////////////////////////////////////////
	// Fill each line
	/////////////////////////////////////////////////////
	CRank rkList;
	CArticleGroup *pLoopArticleGroup = m_pclArticleGroupList->GetFirstArticleGroup();
	long lIndex = 0;

	while( NULL != pLoopArticleGroup )
	{
		rkList.AddStrSort( pLoopArticleGroup->GetFirstArticleContainer()->GetArticleItem()->GetArticle(), 0.0, ( LPARAM )lIndex, false );
		pLoopArticleGroup = m_pclArticleGroupList->GetNextArticleGroup();
		lIndex++;
	}

	long lRow = pclSheet->GetMaxRows();
	CString str;
	LPARAM lparam;
	// Use a map to avoid to display duplicate
	std::map<CString, CArticleItem *> mpArticleItem;

	// Fill the map with all article
	for( BOOL fContinue = rkList.GetFirst( str, lparam ); TRUE == fContinue; fContinue = rkList.GetNext( str, lparam ) )
	{
		lIndex = ( long )lparam;
		CArticleGroup *pLoopArticleGroup = m_pclArticleGroupList->GetArticleGroupAt( lIndex );
		CArticleContainer *pclArticleContainer = pLoopArticleGroup->GetFirstArticleContainer();
		CArticleItem *pclArticleItem = pclArticleContainer->GetArticleItem();

		mpArticleItem[pclArticleItem->GetArticle()] = pclArticleItem;

		// HYS-2072: Manage pressurisation accessories.
		bool bIsForPressMaint = false;
		CDB_Product *pclProduct = dynamic_cast<CDB_Product *>(TASApp.GetpTADB()->Get( pclArticleItem->GetID() ).MP);

		if( NULL != pclProduct && ( dynamic_cast<CDB_TecBox *>(pclProduct) || dynamic_cast<CDB_Vessel *>(pclProduct)
			|| dynamic_cast<CDB_TBPlenoVento *>(pclProduct) ) )
		{
			bIsForPressMaint = true;
		}
		
		if( NULL != pclArticleContainer->GetAccessoryList() && true == bIsForPressMaint )
		{
			for( CArticleItem *pclAccItem = pclArticleContainer->GetFirstAccessory(); NULL != pclAccItem; pclAccItem = pclArticleContainer->GetNextAccessory() )
			{
				mpArticleItem[pclAccItem->GetArticle()] = pclAccItem;
			}
		}

		while( pclArticleContainer = pLoopArticleGroup->GetNextArticleContainer() )
		{
			pclArticleItem = pclArticleContainer->GetArticleItem();
			mpArticleItem[pclArticleItem->GetArticle()] = pclArticleItem;

			// HYS-2072: For pressurisation accessories open the tender text dialog if it exists.
			if( NULL != pclArticleContainer->GetAccessoryList() && true == bIsForPressMaint )
			{
				for( CArticleItem *pclAccItem = pclArticleContainer->GetFirstAccessory(); NULL != pclAccItem; pclAccItem = pclArticleContainer->GetNextAccessory() )
				{
					mpArticleItem[pclAccItem->GetArticle()] = pclAccItem;
				}
			}
		}
	}

	// Fill the spread
	long lFirstRowTender = lRow;
	for (std::map<CString, CArticleItem *>::iterator it = mpArticleItem.begin(); it != mpArticleItem.end(); ++it)
	{
		CArticleItem *pclArticleItem = it->second;
		long lRowArticle = 0;

		lRowArticle = _FillRow( pclSDArticleList, lRow, pclArticleItem );
		lRow = ++lRowArticle;

		// Draw line below.
		pclSheet->SetCellBorder( ColumnDescriptionPTender::ArticleNumber, lRowArticle, ColumnDescriptionPTender::Footer - 1, lRowArticle, true,
								 SS_BORDERTYPE_BOTTOM );
	}

	// HYS-1304 : Adjust row height. We added the line one by without adapt the row height. For the very long string
	// only the first characters which can be in one line are shown.  
	for( int iCpt = lFirstRowTender; iCpt < pclSheet->GetMaxRows(); iCpt++ )
	{
		double dHeight, dMaxWidth, dMaxHeight;
		// Get the full line
		CString strColtext = pclSheet->GetCellText( ColumnDescriptionPTender::TenderText, iCpt );
		// Get the minimum height, width needed for the full text regarding the font size
		pclSheet->GetMaxTextCellSize( ColumnDescriptionPTender::TenderText, iCpt, &dMaxWidth, &dMaxHeight );
		// Get the actual row height
		pclSheet->GetRowHeight( iCpt, &dHeight );
		
		if( dHeight < dMaxHeight )
		{
			pclSheet->SetRowHeight( iCpt, dMaxHeight - 1);
		}
	}
	// Move sheet to correct position.
	SetSheetSize();
	Invalidate();
	UpdateWindow();

	return true;
}

bool CSelProdPageTender::HasSomethingToDisplay( void )
{
	if( NULL == m_pclArticleGroupList || 0 == m_pclArticleGroupList->GetArticleGroupCount() )
	{
		return false;
	}

	return true;
}

void CSelProdPageTender::_InitColHeader( CSheetDescription *pclSheetDescription )
{
	if( NULL == pclSheetDescription || NULL == pclSheetDescription->GetSSheetPointer() )
	{
		return;
	}

	CSSheet *pclSheet = pclSheetDescription->GetSSheetPointer();
	CString str;

	// Format columns header.
	pclSheet->SetTextPattern( CSSheet::_SSTextPattern::ColumnHeader );
	pclSheet->SetTextPatternProperty( CSSheet::_SSTextPatternProperty::TPP_FontBackColor, ( LPARAM )_TAH_WHITE );
	pclSheet->SetTextPatternProperty( CSSheet::_SSTextPatternProperty::TPP_StyleHorizontalAlign, ( LPARAM )SSS_ALIGN_LEFT );

	if( true == m_fTAArtNumberShown )
	{
		str = TASApp.LoadLocalizedString( IDS_SELPHDR_ART );
		AddStaticText( pclSheetDescription, ColumnDescriptionPTender::ArticleNumber, SelProdHeaderRow::HR_RowHeader, str );
		//AddCellSpanW( pclSheetDescription, ColumnDescriptionPTender::ArticleNumber, SelProdHeaderRow::HR_RowHeader, 2, 1 );
	}

	str = TASApp.LoadLocalizedString( GetTabTitleID() );
	AddStaticText( pclSheetDescription, ColumnDescriptionPTender::TenderText, SelProdHeaderRow::HR_RowHeader, str );

	if( true == m_fLocalArtNumberShown )
	{
		str = TASApp.LoadLocalizedString( IDS_SELPHDR_LOCART );
		AddStaticText( pclSheetDescription, ColumnDescriptionPTender::LocalArticleNumber, SelProdHeaderRow::HR_RowHeader, str );
	}

	str = TASApp.LoadLocalizedString( IDS_SELPHDR_DESCRIPTION );
	AddStaticText( pclSheetDescription, ColumnDescriptionPTender::Description, SelProdHeaderRow::HR_RowHeader, str );

	pclSheet->SetTextPatternProperty( CSSheet::_SSTextPatternProperty::TPP_StyleHorizontalAlign, ( LPARAM )SSS_ALIGN_CENTER );
	str = TASApp.LoadLocalizedString( IDS_SELPHDR_QTY );
	AddStaticText( pclSheetDescription, ColumnDescriptionPTender::Quantity, SelProdHeaderRow::HR_RowHeader, str );

	// Draw border.
	pclSheet->SetCellBorder( ColumnDescriptionPTender::ArticleNumber, SelProdHeaderRow::HR_RowHeader, ColumnDescriptionPTender::Footer - 1,
							 SelProdHeaderRow::HR_RowHeader, true, SS_BORDERTYPE_BOTTOM | SS_BORDERTYPE_TOP );

	// Freeze row header.
	pclSheet->SetFreeze( 0, SelProdHeaderRow::HR_RowHeader );
}

long CSelProdPageTender::_FillRow( CSheetDescription *pclSheetDescription, long lRow, CArticleItem *pclArticleGroup )
{
	if( NULL == pclSheetDescription
		|| NULL == pclSheetDescription->GetSSheetPointer()
		|| NULL == pclArticleGroup
		|| false == TASApp.GetTender().IsTenderNeeded()
		|| -1 == TASApp.GetTender().GetTenderID( _string( pclArticleGroup->GetArticle() ) ) )
	{
		return lRow - 1;
	}

	CSSheet *pclSheet = pclSheetDescription->GetSSheetPointer();
	CArticleItem *pclArticleItem = pclArticleGroup;

	CString str;
	pclSheet->SetTextPattern( CSSheet::_SSTextPattern::UserStaticColumn );
	pclSheet->SetTextPatternProperty( CSSheet::_SSTextPatternProperty::TPP_FontBold, ( LPARAM )FALSE );
	pclSheet->SetTextPatternProperty( CSSheet::_SSTextPatternProperty::TPP_StyleWordWrap, ( LPARAM )TRUE );
	pclSheet->SetTextPatternProperty( CSSheet::_SSTextPatternProperty::TPP_StyleVerticalAlign, ( LPARAM )SSS_ALIGN_TOP );
	pclSheet->SetTextPatternProperty( CSSheet::_SSTextPatternProperty::TPP_StyleHorizontalAlign, ( LPARAM )SSS_ALIGN_LEFT );

	if( false == pclArticleItem->GetIsAvailable() || true == pclArticleItem->GetIsDeleted() )
	{
		pclSheet->SetTextPatternProperty( CSSheet::_SSTextPatternProperty::TPP_FontForeColor, ( LPARAM )_RED );
	}


	if( true == m_fTAArtNumberShown )
	{
		AddStaticText( pclSheetDescription, ColumnDescriptionPTender::ArticleNumber, lRow, pclArticleItem->GetArticle() );
	}

	pclSheet->SetTextPatternProperty( CSSheet::_SSTextPatternProperty::TPP_FontForeColor, ( LPARAM )_BLACK );

	long lFirstRow = lRow;
	if( -1 != TASApp.GetTender().GetTenderID( _string( pclArticleItem->GetArticle() ) ) )
	{
		_string strTender;
		TASApp.GetTender().GetTenderTxt( _string( pclArticleItem->GetArticle() ), strTender );
		// HYS-1304 : Parse the text to obtain multiline when it has '\n'.
		TCHAR charDelimiter = '\n';
		int iNbLine = 0;
		CStringArray ArString;
		CString strTemp = strTender.c_str();
		strTemp.Remove( '\r' );
		ParseStringByChar( strTemp, charDelimiter, &iNbLine, &ArString );
		for( int iCpt = 0; iCpt < iNbLine; iCpt++ )
		{
			AddStaticText( pclSheetDescription, ColumnDescriptionPTender::TenderText, lRow, ArString.GetAt( iCpt ) );
			lRow++;
		}
		// Draw border : don't need border for the same article number.
		pclSheet->SetCellBorder( ColumnDescriptionPTender::ArticleNumber, lFirstRow, ColumnDescriptionPTender::Footer - 1,
			iNbLine, false, SS_BORDERTYPE_BOTTOM | SS_BORDERTYPE_TOP, SS_BORDERSTYLE_BLANK );
	}

	pclSheet->SetTextPatternProperty( CSSheet::_SSTextPatternProperty::TPP_StyleHorizontalAlign, ( LPARAM )SSS_ALIGN_LEFT );

	return pclSheet->GetMaxRows();
}

UINT CSelProdPageTender::GetTabTitleID( void )
{
	return IDS_DLGDIRSEL_TENDERTEXT;
}
