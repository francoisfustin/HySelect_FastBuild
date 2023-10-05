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
#include "SelProdPageArtList.h"
#include "DlgTender.h"

CSelProdPageArtList::CSelProdPageArtList( CArticleGroupList *pclArticleGroupList ) 
	: CSelProdPageBase( CDB_PageSetup::enCheck::ARTLIST, pclArticleGroupList )
{
	m_pSelected = NULL;
	m_bTAArtNumberShown = true;
	m_bLocalArtNumberShown = false;
}

BEGIN_MESSAGE_MAP(CSelProdPageArtList, CSelProdPageBase)
	ON_MESSAGE(SSM_CLICK, &CSelProdPageArtList::OnSSClick)
END_MESSAGE_MAP()

LRESULT CSelProdPageArtList::OnSSClick(WPARAM wParam, LPARAM lParam)
{
	SS_CELLCOORD *pclCellCoord = (SS_CELLCOORD*)lParam;

	CSheetDescription *pclSDesc = m_ViewDescription.GetFromSSheetID(UINT(wParam));
	CSSheet *pclSSheet = pclSDesc->GetSSheetPointer();

	// intercept click on the first column
	if (pclCellCoord->Col != 1 || false == TASApp.GetTender().IsTenderNeeded())
	{
		return 0;
	}

	CTenderDlg Dlg;
	TCHAR artNum[256] = { 0 };
	pclSSheet->GetData(ColumnDescriptionPArtList::ArticleNumber, pclCellCoord->Row, artNum);

	// HYS-2072: For pressurisation accessories open the tender text dialog if it exists.
	if( 0 == StrCmp( artNum, L"" ) )
	{
		// Check for pressurisation accessory
		TCHAR strHeader[32] = { 0 };
		pclSSheet->GetData( ColumnDescriptionPArtList::Header, pclCellCoord->Row, strHeader );
		if( 0 != StrCmp( strHeader, L"" ) )
		{
			pclSSheet->GetData( ColumnDescriptionPArtList::AccArtNumber, pclCellCoord->Row, artNum );
		}
	}

	if (-1 != TASApp.GetTender().GetTenderID(artNum))
	{
		_string tenderText;

		TASApp.GetTender().GetTenderTxt(_string(artNum), tenderText);
		Dlg.SetTender(_T(""), _string(artNum), tenderText);

		INT_PTR result = Dlg.DoModal();
	}

	return 1; // event processed
}

void CSelProdPageArtList::Reset( void )
{
	CSelProdPageBase::Reset();
}

bool CSelProdPageArtList::Init( bool fResetOrder, bool fPrint )
{
	// Don't verify here if there is something to print. Because if there is no product to display, we show at least the article list.

	// Call 'MultiSpreadBase' method to get a new 'Sheet'.
	CSheetDescription* pclSDArticleList = CMultiSpreadBase::CreateSSheet( SD_ArticleList );
	
	if( NULL == pclSDArticleList || NULL == pclSDArticleList->GetSSheetPointer() )
	{
		return false;
	}
	
	CSSheet* pclSheet = pclSDArticleList->GetSSheetPointer();
	
	if( false == CSelProdPageBase::PrepareSheet( pclSDArticleList, ColumnDescriptionPArtList::Pointer, fPrint ) )
	{
		m_ViewDescription.RemoveOneSheetDescription( SD_ArticleList );
		return false;
	}
	
	// Initialize column width for header and footer column.
	if( false == m_bForPrint )
	{
		SetColWidth( pclSDArticleList, ColumnDescriptionPArtList::Header, DefaultColumnWidth::DCW_Header );
		SetColWidth( pclSDArticleList, ColumnDescriptionPArtList::Footer, DefaultColumnWidth::DCW_Footer );
	}
	else
	{
		pclSheet->ShowCol( ColumnDescriptionPArtList::Header, FALSE );
		pclSheet->ShowCol( ColumnDescriptionPArtList::Footer, FALSE );
	}

	m_bTAArtNumberShown = !TASApp.IsTAArtNumberHidden();
	m_bLocalArtNumberShown = TASApp.IsLocalArtNumberUsed();
	
	// HYS-1358 : Show TA article number when local is used and is empty
	if( false == m_bTAArtNumberShown )
	{
		// The TA article number column is hidden
		m_bTAArticleNumberIsEmpty = true;
	}
	else
	{
		// the TA article number column stays visible
		m_bTAArticleNumberIsEmpty = false;
	}

	// TA article number
	SetColWidth( pclSDArticleList, ColumnDescriptionPArtList::ArticleNumber, DefaultColumnWidth::DCW_ArticleNumber );
	SetColWidth( pclSDArticleList, ColumnDescriptionPArtList::AccArtNumber, 21 );

	if( true == m_bLocalArtNumberShown )
	{
		SetColWidth( pclSDArticleList, ColumnDescriptionPArtList::LocalArticleNumber, DefaultColumnWidth::DCW_ArticleNumber );
	}
	else
	{
		SetColWidth( pclSDArticleList, ColumnDescriptionPArtList::LocalArticleNumber, 0 ); 
		pclSheet->ShowCol( ColumnDescriptionPArtList::LocalArticleNumber, FALSE );
	}
	
	SetColWidth( pclSDArticleList, ColumnDescriptionPArtList::Description, 55 );
	SetColWidth( pclSDArticleList, ColumnDescriptionPArtList::Quantity, DefaultColumnWidth::DCW_Quantity );

	double dWidth = ( true == TASApp.IsPriceUsed() ) ? DefaultColumnWidth::DCW_UnitPrice : 0.0;
	SetColWidth( pclSDArticleList, ColumnDescriptionPArtList::UnitPrice, dWidth );

	dWidth = ( true == TASApp.IsPriceUsed() ) ? DefaultColumnWidth::DCW_TotalPrice : 0.0;
	SetColWidth( pclSDArticleList, ColumnDescriptionPArtList::TotalPrice, dWidth );

	SetColWidth( pclSDArticleList, ColumnDescriptionPArtList::Pointer, DefaultColumnWidth::DCW_Pointer );
	
	if( false == TASApp.IsPriceUsed() )
	{
		pclSheet->ShowCol( ColumnDescriptionPArtList::UnitPrice, FALSE );
		pclSheet->ShowCol( ColumnDescriptionPArtList::TotalPrice, FALSE );
	}

	pclSheet->ShowCol( ColumnDescriptionPArtList::Pointer, FALSE );
	
	CRect rect = pclSheet->GetSheetSizeInPixels( false );
	m_mapSheetInfos[pclSDArticleList].m_dPageWidth = rect.Width();

	// Page title.
	SetPageTitle( pclSDArticleList, IDS_RVIEWSELP_TITLE_RECAPITULATE );

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
		rkList.AddStrSort( pLoopArticleGroup->GetFirstArticleContainer()->GetArticleItem()->GetArticle(), 0.0, (LPARAM)lIndex, false );
		pLoopArticleGroup = m_pclArticleGroupList->GetNextArticleGroup();
		lIndex++;
	}

	long lRow = pclSheet->GetMaxRows();
	CString str;
	LPARAM lparam;

	for( BOOL bContinue = rkList.GetFirst( str, lparam ); TRUE == bContinue; bContinue = rkList.GetNext( str, lparam ) )
	{
		lIndex = (long)lparam;
		pLoopArticleGroup = m_pclArticleGroupList->GetArticleGroupAt( lIndex );
		long lRowArticle = _FillRow( pclSDArticleList, lRow, pLoopArticleGroup );

		// Draw line below.
		pclSheet->SetCellBorder( ColumnDescriptionPArtList::ArticleNumber, lRowArticle, ColumnDescriptionPArtList::Footer - 1, lRowArticle, true, SS_BORDERTYPE_BOTTOM );
		lRow = ++lRowArticle;
	}
	// HYS-1358 : All products have local article number
	if (true == m_bTAArticleNumberIsEmpty)
	{
		SetColWidth(pclSDArticleList, ColumnDescriptionPArtList::ArticleNumber, 0);
		pclSheet->ShowCol(ColumnDescriptionPArtList::ArticleNumber, FALSE);
		SetColWidth(pclSDArticleList, ColumnDescriptionPArtList::AccArtNumber, 0);
		pclSheet->ShowCol(ColumnDescriptionPArtList::AccArtNumber, FALSE);
	}
	// Move sheet to correct position.
	SetSheetSize();
	Invalidate();
	UpdateWindow();

	return true;
}

bool CSelProdPageArtList::HasSomethingToDisplay( void )
{
	if( NULL == m_pclArticleGroupList || 0 == m_pclArticleGroupList->GetArticleGroupCount() )
	{
		return false;
	}

	return true;
}

void CSelProdPageArtList::_InitColHeader( CSheetDescription* pclSheetDescription )
{
	if( NULL == pclSheetDescription || NULL == pclSheetDescription->GetSSheetPointer() )
		return;

	CSSheet* pclSheet = pclSheetDescription->GetSSheetPointer();
	CString str;

	// Format columns header.
	pclSheet->SetTextPattern( CSSheet::_SSTextPattern::ColumnHeader );
	pclSheet->SetTextPatternProperty( CSSheet::_SSTextPatternProperty::TPP_FontBackColor, (LPARAM)_TAH_WHITE );
	pclSheet->SetTextPatternProperty( CSSheet::_SSTextPatternProperty::TPP_StyleHorizontalAlign, (LPARAM)SSS_ALIGN_LEFT );
	
	// TA article number header
	str = TASApp.LoadLocalizedString( IDS_SELPHDR_ART );
	AddStaticText( pclSheetDescription, ColumnDescriptionPArtList::ArticleNumber, SelProdHeaderRow::HR_RowHeader, str );
	AddCellSpanW( pclSheetDescription, ColumnDescriptionPArtList::ArticleNumber, SelProdHeaderRow::HR_RowHeader, 2, 1 );
	
	if( true == m_bLocalArtNumberShown )
	{
		str = TASApp.LoadLocalizedString( IDS_SELPHDR_LOCART );
		AddStaticText( pclSheetDescription, ColumnDescriptionPArtList::LocalArticleNumber, SelProdHeaderRow::HR_RowHeader, str );
	}
	
	str = TASApp.LoadLocalizedString( IDS_SELPHDR_DESCRIPTION );
	AddStaticText( pclSheetDescription, ColumnDescriptionPArtList::Description, SelProdHeaderRow::HR_RowHeader, str );
	
	pclSheet->SetTextPatternProperty( CSSheet::_SSTextPatternProperty::TPP_StyleHorizontalAlign, (LPARAM)SSS_ALIGN_CENTER );
	str = TASApp.LoadLocalizedString( IDS_SELPHDR_QTY );
	AddStaticText( pclSheetDescription, ColumnDescriptionPArtList::Quantity, SelProdHeaderRow::HR_RowHeader, str );

	if( true == TASApp.IsPriceUsed() )
	{
		pclSheet->SetTextPatternProperty( CSSheet::_SSTextPatternProperty::TPP_StyleWordWrap, (LPARAM)TRUE );

		str = TASApp.LoadLocalizedString( IDS_SELPHDR_PRICE ) + _T("\r\n[");
		str += CString( m_pTADS->GetpTechParams()->GetCurrentCurrencyISO().c_str() ) + _T("]");
		AddStaticText( pclSheetDescription, ColumnDescriptionPArtList::UnitPrice, SelProdHeaderRow::HR_RowHeader, str );

		str = TASApp.LoadLocalizedString( IDS_SELPHDR_TOTAL ) + _T("\r\n[");
		str += CString( m_pTADS->GetpTechParams()->GetCurrentCurrencyISO().c_str() ) + _T("]");
		AddStaticText( pclSheetDescription, ColumnDescriptionPArtList::TotalPrice, SelProdHeaderRow::HR_RowHeader, str );
	}

	// Draw border.
	pclSheet->SetCellBorder( ColumnDescriptionPArtList::ArticleNumber, SelProdHeaderRow::HR_RowHeader, ColumnDescriptionPArtList::Footer - 1, SelProdHeaderRow::HR_RowHeader, true, SS_BORDERTYPE_BOTTOM | SS_BORDERTYPE_TOP );

	// Freeze row header.
	pclSheet->SetFreeze( 0, SelProdHeaderRow::HR_RowHeader );
}

long CSelProdPageArtList::_FillRow( CSheetDescription *pclSheetDescription, long lRow, CArticleGroup *pclArticleGroup )
{
	if( NULL == pclSheetDescription || NULL == pclSheetDescription->GetSSheetPointer() || NULL == pclArticleGroup 
			|| NULL == pclArticleGroup->GetFirstArticleContainer()
			|| NULL == pclArticleGroup->GetFirstArticleContainer()->GetArticleItem() )
	{
		return lRow;
	}
	
	CSSheet *pclSheet = pclSheetDescription->GetSSheetPointer();

	bool bFirstPass = true;
	CArticleContainer *pclLoopArticleContainer = pclArticleGroup->GetFirstArticleContainer();
	
	while( NULL != pclLoopArticleContainer )
	{
		CArticleItem *pclArticleItem = pclLoopArticleContainer->GetArticleItem();

		if( NULL == pclArticleItem )
		{
			continue;
		}

		CString str;
		pclSheet->SetTextPattern( CSSheet::_SSTextPattern::UserStaticColumn );
		pclSheet->SetTextPatternProperty( CSSheet::_SSTextPatternProperty::TPP_FontBold, (LPARAM)FALSE );
		pclSheet->SetTextPatternProperty( CSSheet::_SSTextPatternProperty::TPP_StyleHorizontalAlign, (LPARAM)SSS_ALIGN_LEFT );

		if( false == pclArticleItem->GetIsAvailable() || true == pclArticleItem->GetIsDeleted() )
		{
			pclSheet->SetTextPatternProperty( CSSheet::_SSTextPatternProperty::TPP_FontForeColor, (LPARAM)_RED );
		}
	
		CString strArticlePrefix = ( true == bFirstPass ) ? _T("") : _T("    ");
		
		if( true == m_bTAArtNumberShown )
		{
			CString strArticle = strArticlePrefix + pclArticleItem->GetArticle();
			AddStaticText( pclSheetDescription, ColumnDescriptionPArtList::ArticleNumber, lRow, strArticle );
			AddCellSpanW( pclSheetDescription, ColumnDescriptionPArtList::ArticleNumber, lRow, 2, 1 );
		}

		if( true == TASApp.GetTender().IsTenderNeeded() )
		{
			if( -1 != TASApp.GetTender().GetTenderID( _string( pclArticleItem->GetArticle() ) ) )
			{
				HBITMAP hBitmap = (HBITMAP)LoadImage(AfxGetApp()->m_hInstance, MAKEINTRESOURCE( IDI_TENDER ), IMAGE_ICON, 16, 16, LR_SHARED );
				pclSheet->SetPictureCellWithHandle( hBitmap, ColumnDescriptionPArtList::Header, lRow, false, VPS_CENTER | VPS_ICON );
			}
		}

		// HYS-1358 : Show TA article number when local is used and is empty
		if( true == m_bLocalArtNumberShown && pclArticleItem->GetLocArtNum() != _T("-") )
		{
			AddStaticText( pclSheetDescription, ColumnDescriptionPArtList::LocalArticleNumber, lRow, pclArticleItem->GetLocArtNum() );
		}
		else if( false == m_bTAArtNumberShown )
		{
			// if we don't have a local article number the TA is used
			CString strArticle = strArticlePrefix + pclArticleItem->GetArticle();
			AddStaticText(pclSheetDescription, ColumnDescriptionPArtList::ArticleNumber, lRow, strArticle);
			AddCellSpanW(pclSheetDescription, ColumnDescriptionPArtList::ArticleNumber, lRow, 2, 1);
			m_bTAArticleNumberIsEmpty = false;
		}
		pclSheet->SetTextPatternProperty( CSSheet::_SSTextPatternProperty::TPP_FontForeColor, (LPARAM)_BLACK );

		// In screen mode we are displaying ellipses when needed, in print mode we are increasing row height if needed.
		if( false == m_bForPrint )
		{
			pclSheet->SetTextPatternProperty( CSSheet::_SSTextPatternProperty::TPP_StyleEllipses, (LPARAM)TRUE );
			AddStaticText( pclSheetDescription, ColumnDescriptionPArtList::Description, lRow, pclArticleItem->GetDescription() );
			pclSheet->SetTextPatternProperty( CSSheet::_SSTextPatternProperty::TPP_StyleEllipses, (LPARAM)FALSE );
		}
		else
		{
			pclSheet->SetTextPatternProperty( CSSheet::_SSTextPatternProperty::TPP_StyleWordWrap, (LPARAM)TRUE );
			pclSheet->SetStaticText( ColumnDescriptionPArtList::Description, lRow, pclArticleItem->GetDescription() );

			double dHeight, dMaxWidth, dMaxHeight;
			pclSheet->GetRowHeight( lRow, &dHeight );
			pclSheet->GetMaxTextCellSize( ColumnDescriptionPArtList::Description, lRow, &dMaxWidth, &dMaxHeight );
		
			// Adapt row height if needed...
			if( dHeight < dMaxHeight )
			{
				pclSheet->SetRowHeight( lRow, dMaxHeight );
			}

			pclSheet->SetTextPatternProperty( CSSheet::_SSTextPatternProperty::TPP_StyleWordWrap, (LPARAM)FALSE );
		}

		if( true == TASApp.IsPriceUsed() )
		{
			str = ( pclArticleItem->GetPrice() > 0.0 ) ? WriteDouble( pclArticleItem->GetPrice(), 2, 2, 0 ) : _T("-");
			pclSheet->SetStaticText( ColumnDescriptionPArtList::UnitPrice, lRow, str );

			str = ( pclArticleItem->GetPrice() > 0.0 ) ? WriteDouble( pclArticleItem->GetPrice() * pclArticleItem->GetQuantity(), 2, 2, 0 ) : _T("-");
			pclSheet->SetStaticText( ColumnDescriptionPArtList::TotalPrice, lRow, str );
		}

		pclSheet->SetTextPatternProperty( CSSheet::_SSTextPatternProperty::TPP_StyleHorizontalAlign, (LPARAM)SSS_ALIGN_CENTER );
		str = WriteDouble( pclArticleItem->GetQuantity(), 0, 0, true );
		pclSheet->SetStaticText( ColumnDescriptionPArtList::Quantity, lRow, str );

		pclSheet->SetTextPatternProperty( CSSheet::_SSTextPatternProperty::TPP_StyleHorizontalAlign, (LPARAM)SSS_ALIGN_LEFT );
	
		// Loop to fill all accessories.
		// Remark: 'GetSubArticleList' never returns NULL!
		CArticleItem *pclLoopAccessory = pclLoopArticleContainer->GetFirstAccessory(); 

		// HYS-2072: For pressurisation accessories display the tender image if it exists.
		bool bIsForPressMaint = false;
		CDB_Product *pclProduct = dynamic_cast<CDB_Product *>(TASApp.GetpTADB()->Get( pclArticleItem->GetID() ).MP);

		if( NULL != pclProduct && ( dynamic_cast<CDB_TecBox *>(pclProduct) || dynamic_cast<CDB_Vessel *>(pclProduct)
			|| dynamic_cast<CDB_TBPlenoVento *>(pclProduct) ) )
		{
			bIsForPressMaint = true;
		}

		int i = 1;

		while( NULL != pclLoopAccessory )
		{
			if( true == m_bTAArtNumberShown )
			{
				AddStaticText( pclSheetDescription, ColumnDescriptionPArtList::AccArtNumber, lRow + i, pclLoopAccessory->GetArticle() );
			}

			// HYS-2072
			if( true == TASApp.GetTender().IsTenderNeeded() && true == bIsForPressMaint )
			{
				if( -1 != TASApp.GetTender().GetTenderID( _string( pclLoopAccessory->GetArticle() ) ) )
				{
					HBITMAP hBitmap = (HBITMAP)LoadImage( AfxGetApp()->m_hInstance, MAKEINTRESOURCE( IDI_TENDER ), IMAGE_ICON, 16, 16, LR_SHARED );
					pclSheet->SetPictureCellWithHandle( hBitmap, ColumnDescriptionPArtList::Header, lRow + i, false, VPS_CENTER | VPS_ICON );
				}
			}

			// HYS-1358 : Show TA article number when local is used and is empty
			if( true == m_bLocalArtNumberShown && pclLoopAccessory->GetLocArtNum() != _T("-") )
			{
				AddStaticText( pclSheetDescription, ColumnDescriptionPArtList::LocalArticleNumber, lRow + i, pclLoopAccessory->GetLocArtNum() );
			}
			else if( false == m_bTAArtNumberShown )
			{
				AddStaticText(pclSheetDescription, ColumnDescriptionPArtList::AccArtNumber, lRow + i, pclLoopAccessory->GetArticle());
			}

			if( false == m_bForPrint )
			{
				pclSheet->SetTextPatternProperty( CSSheet::_SSTextPatternProperty::TPP_StyleEllipses, (LPARAM)TRUE );
				AddStaticText( pclSheetDescription, ColumnDescriptionPArtList::Description, lRow + i, pclLoopAccessory->GetDescription() );
				pclSheet->SetTextPatternProperty( CSSheet::_SSTextPatternProperty::TPP_StyleEllipses, (LPARAM)FALSE );
			}
			else
			{
				pclSheet->SetTextPatternProperty( CSSheet::_SSTextPatternProperty::TPP_StyleWordWrap, (LPARAM)TRUE );
				pclSheet->SetStaticText( ColumnDescriptionPArtList::Description, lRow + i, pclLoopAccessory->GetDescription() );
			
				double dHeight, dMaxWidth, dMaxHeight;
				pclSheet->GetRowHeight( lRow + i, &dHeight );
				pclSheet->GetMaxTextCellSize( ColumnDescriptionPArtList::Description, lRow + i, &dMaxWidth, &dMaxHeight );
		
				// Adapt row height if needed...
				if( dHeight < dMaxHeight )
				{
					pclSheet->SetRowHeight( lRow + i, dMaxHeight );
				}

				pclSheet->SetTextPatternProperty( CSSheet::_SSTextPatternProperty::TPP_StyleWordWrap, (LPARAM)FALSE );
			}

			// Quantity.
			// Remark: it is now possible to have an accessory that is shown as a sub-article. It's the case for example the Transfero TV
			//         for which we can select a DLV. Before, a sub-article was directly considered as a part of a set or as included with
			//         the article. Thus no quantity was written.
			if( false == pclLoopAccessory->GetIsAttached() && false == pclLoopAccessory->GetIsSelectedInSet() )
			{
				str = WriteDouble( pclLoopAccessory->GetQuantity(), 0, 0, true );

				pclSheet->SetTextPatternProperty( CSSheet::_SSTextPatternProperty::TPP_StyleHorizontalAlign, (LPARAM)SSS_ALIGN_CENTER );
				pclSheet->SetStaticText( ColumnDescriptionPArtList::Quantity, lRow + i, str );
				pclSheet->SetTextPatternProperty( CSSheet::_SSTextPatternProperty::TPP_StyleHorizontalAlign, (LPARAM)SSS_ALIGN_LEFT );
			}

			if( true == TASApp.IsPriceUsed() )
			{
				str = ( pclLoopAccessory->GetPrice() > 0.0 ) ? WriteDouble( pclLoopAccessory->GetPrice(), 2, 2, 0 ) : _T("-");
				pclSheet->SetStaticText( ColumnDescriptionPArtList::UnitPrice, lRow + i, str );

				str = ( pclLoopAccessory->GetPrice() > 0.0 ) ? WriteDouble( pclLoopAccessory->GetPrice() * pclLoopAccessory->GetQuantity(), 2, 2, 0 ) : _T("-");
				pclSheet->SetStaticText( ColumnDescriptionPArtList::TotalPrice, lRow + i, str );
			}

			pclLoopAccessory = pclLoopArticleContainer->GetNextAccessory();
			i++;
		}

		lRow = pclSheet->GetMaxRows() + 1;
		pclArticleItem = NULL;
		bFirstPass = false;
		
		pclLoopArticleContainer = pclArticleGroup->GetNextArticleContainer();
	}

	return pclSheet->GetMaxRows();
}

BOOL CSelProdPageArtList::PreTranslateMessage(MSG* pMsg)
{

	if (pMsg->message == WM_LBUTTONDOWN)
	{
		// PWU : The first click on the spread is ignored if the focus is elsewhere.
		// This code catch the first click and send it directly to the spread.
		CRect rectSpread;
		GetFocus()->GetClientRect(rectSpread);
		CRect rectWindow;
		this->GetWindowRect(rectWindow);

		CRect rectIntersect;
		rectIntersect.IntersectRect(rectSpread, rectWindow);

		if (rectWindow.PtInRect(pMsg->pt) && rectIntersect.IsRectNull())
		{
			CSheetDescription *pclSDesc = m_ViewDescription.GetFromSSheetID(IDC_FPSPREAD);
			CSSheet *plcSSheet = pclSDesc->GetSSheetPointer();
			plcSSheet->SendMessage(pMsg->message, pMsg->wParam, pMsg->lParam);
			SetFocus();
		}
	}
	return __super::PreTranslateMessage(pMsg); // allow default behavior (return TRUE if you want to discard message)
}
