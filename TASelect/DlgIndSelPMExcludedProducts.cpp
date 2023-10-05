#include "stdafx.h"
#include <float.h>
#include <math.h>

#include "TASelect.h"
#include "TASelectDoc.h"
#include "MainFrm.h"
#include "DataStruct.h"
#include "Global.h"
#include "Utilities.h"
#include "ProductSelectionParameters.h"
#include "DlgIndSelPMExcludedProducts.h"


CDlgIndSelPMExcludedProducts::CDlgIndSelPMExcludedProducts( CSelectPMList *pclSelectPMList, CSelectPMList::ProductType eProductType, CRViewSSelPM *pRViewSSelPM )
	: CDialogExt( CDlgIndSelPMExcludedProducts::IDD, (CWnd*)pRViewSSelPM )
{
	m_pclSelectPMList = pclSelectPMList;
	m_eProductType = eProductType;
	m_pclSheetDescription = NULL;
	m_pclSheet = NULL;
	m_iTotalColWidth = 0;
	m_strSectionName = _T("DlgIndSelPMExludedProducts");
}

CDlgIndSelPMExcludedProducts::~CDlgIndSelPMExcludedProducts()
{
	if( NULL != m_pclSheet )
	{
		m_pclSheet->Detach();
		m_ViewDescription.RemoveAllSheetDescription( true );

		delete m_pclSheet;
	}
}

//////////////////////////////////////////////////////////////////////////////////////////////////////////////
// PROTECTED METHODS
//////////////////////////////////////////////////////////////////////////////////////////////////////////////

BEGIN_MESSAGE_MAP( CDlgIndSelPMExcludedProducts, CDialogExt )
	ON_BN_CLICKED( IDCLOSE, OnBnClickedClose )
	ON_WM_SIZE()
	ON_WM_GETMINMAXINFO()
	// Spread DLL messages.
	ON_MESSAGE( SSM_TEXTTIPFETCH, TextTipFetch )
END_MESSAGE_MAP()

void CDlgIndSelPMExcludedProducts::DoDataExchange( CDataExchange* pDX )
{
	CDialogExt::DoDataExchange( pDX );
	DDX_Control( pDX, IDCLOSE, m_ButtonClose );
}

BOOL CDlgIndSelPMExcludedProducts::OnInitDialog()
{
	if( NULL == m_pclSelectPMList || FALSE == CDialogExt::OnInitDialog() )
	{
		return FALSE;
	}

	// Initialize dialog strings.
	CString str;
	str = TASApp.LoadLocalizedString( IDS_DLGINDSELPMEXCLUDEDPRODUCTS_CAPTION );
	SetWindowText( str );
	str = TASApp.LoadLocalizedString( IDS_CLOSE );
	GetDlgItem( IDCLOSE )->SetWindowText( str );

	_InitializeSSheet();

	// Because in the base class 'CDialogExt', the dialog has been moved and resized to the position and size
	// saved in the registry, once the spread has been created we can resize it.
	CRect rectClient;
	GetClientRect( &rectClient );
	_UpdateLayout( rectClient.Width(), rectClient.Height() );

	// Adapt column width in regards to the new spread size.
	_ResizeColumns();

	return TRUE;
}

void CDlgIndSelPMExcludedProducts::OnBnClickedClose()
{
	OnOK();
}

void CDlgIndSelPMExcludedProducts::OnSize( UINT nType, int cx, int cy )
{
	CDialogExt::OnSize( nType, cx, cy );

	_UpdateLayout( cx, cy );
}

void CDlgIndSelPMExcludedProducts::OnGetMinMaxInfo( MINMAXINFO* lpMMI )
{
	lpMMI->ptMinTrackSize.x = 400;
	lpMMI->ptMinTrackSize.y = 400;
}

LRESULT CDlgIndSelPMExcludedProducts::TextTipFetch( WPARAM wParam, LPARAM lParam )
{
	if( NULL == m_pclSheet || NULL == m_pclSheet->GetSafeHwnd() )
	{
		return  0;
	}
	
	SS_TEXTTIPFETCH *pTextTipFetch = (SS_TEXTTIPFETCH *)lParam;

	// If hText is not empty, spread takes hText in place of szText!
	pTextTipFetch->hText = NULL;
	
	CString str = _T("");
	LRESULT lReturn = 0;
	pTextTipFetch->fShow = false;
	pTextTipFetch->wMultiLine = SS_TT_MULTILINE_AUTO;

	CSelectedPMBase *pclRejectedPMBase = (CSelectedPMBase *)m_pclSheet->GetCellParam( 1, pTextTipFetch->Row );
	
	if( NULL == pclRejectedPMBase || false == pclRejectedPMBase->GetRejected() )
	{
		return 0;
	}

	// It exists now in the DB a Pleno that is in fact a combination of two modules "P BA4 P" + "P ABR5-R". We want to show this product as
	// one product but with specific data for each of both devices.
	bool fProductCombination = ( CSelectedPMBase::ST_WaterTreatmentCombination == pclRejectedPMBase->GetSelectedType() ) ? true : false;

	CSelectedPMBase *pclFirstSelected = NULL;
	CSelectedPMBase *pclSecondSelected = NULL;
	CSelectedWaterTreatmentCombination *pclSelectedProductCombination = NULL;
	CDB_Set *pProductCombination = NULL;

	if( false == fProductCombination )
	{
		// Only one device.
		pclFirstSelected = pclRejectedPMBase;
	}
	else
	{
		// Combination of two devices.
		pclSelectedProductCombination = dynamic_cast<CSelectedWaterTreatmentCombination*>( pclRejectedPMBase );

		if( NULL == pclSelectedProductCombination )
		{
			return 0;
		}

		pProductCombination = pclSelectedProductCombination->GetWTCombination();

		if( NULL == pProductCombination )
		{
			return 0;
		}

		if( NULL != pclSelectedProductCombination->GetSelectedFirst() )
		{
			pclFirstSelected = pclSelectedProductCombination->GetSelectedFirst();
		}

		if( NULL != pclSelectedProductCombination->GetSelectedSecond() )
		{
			pclSecondSelected = pclSelectedProductCombination->GetSelectedSecond();
		}
	}

	int iRejectReasonCount = 0;

	if( NULL != pclFirstSelected )
	{
		iRejectReasonCount += pclFirstSelected->GetRejectReasonCount();
	}

	if( NULL != pclSecondSelected )
	{
		iRejectReasonCount += pclSecondSelected->GetRejectReasonCount();
	}

	if( 0 == iRejectReasonCount )
	{
		return 0;
	}

	CSelectedPMBase *pclSelected;
	CString strHeader = ( 1 == iRejectReasonCount ) ? _T("") : _T(" - ");

	for( int iLoopProduct = 0; iLoopProduct < 2; iLoopProduct++ )
	{
		if( 0 == iLoopProduct )
		{
			pclSelected = pclFirstSelected;
		}
		else
		{
			pclSelected = pclSecondSelected;
		}

		if( NULL == pclSelected )
		{
			continue;
		}

		CString strRejectReason = pclSelected->GetFirstRejectedReason();

		while( false == strRejectReason.IsEmpty() )
		{
			if( false == str.IsEmpty() )
			{
				str += _T("\r\n");
			}

			str += ( strHeader + strRejectReason );
			strRejectReason = pclSelected->GetNextRejectedReason();
		}
	}

	if( false == str.IsEmpty() )
	{
		pTextTipFetch->nWidth = ( SHORT )m_pclSheet->GetTipTextWidth( str );
		wcsncpy_s( pTextTipFetch->szText, SS_TT_TEXTMAX, ( LPCTSTR )str, SS_TT_TEXTMAX );
		pTextTipFetch->fShow = true;
		lReturn = 1;
	}

	return lReturn;
}

void CDlgIndSelPMExcludedProducts::_InitializeSSheet( void )
{
	if( NULL == pRViewSSelPM )
	{
		return;
	}

	// Pay attention: because in this case I use TSpread directly as a custom control (see IDD_DlgIndSelPMExcludedProducts in the resource), if you want to
	// use 'CSSheet' possibility, we have to attach TSpread CWnd to CSSheet newly created. And the destruction of the dialog, don't forget 
	// to call 'Detach' before deleting 'CSSheet'.
	m_pclSheet = new CSSheet();
	
	if( NULL == m_pclSheet )
	{
		return;
	}

	m_pclSheet->Attach( GetDlgItem( IDC_SPREAD )->GetSafeHwnd() );
	m_pclSheet->SetBool( SSB_REDRAW, FALSE );
	m_pclSheet->Reset();

	m_pclSheetDescription = m_ViewDescription.AddSheetDescription( SD_ExcludedProduct, -1, m_pclSheet, CPoint( 0, 0 ) );
	
	if( NULL == m_pclSheetDescription )
	{
		delete m_pclSheet;
		m_pclSheet = NULL;
		return;
	}

	pRViewSSelPM->InitHeadersHelper( m_pclSheetDescription, m_eProductType );

	// Increase row height to allow "Exclude reason number" to be well displayed.
	double dRowHeight = 12.75;
	m_pclSheet->SetRowHeight( CRViewSSelPM::RD_ExpansionVessel_ColName, dRowHeight * 2.6 );
	
	m_pclSheet->SetBool( SSB_ALLOWDRAGDROP, FALSE );
	m_pclSheet->SetBool( SSB_HORZSCROLLBAR, TRUE );
	m_pclSheet->SetBool( SSB_VERTSCROLLBAR, TRUE );
	m_pclSheet->SetBool( SSB_MOVEACTIVEONFOCUS, FALSE );
	m_pclSheet->SetTextTip( SS_TT_STATUS_FLOATING, 250, NULL, _LIGHTYELLOW, _TAH_BLACK );

	// Add a column to write exclude reason number (that we move at the first position when all rows are filled). 
	long lTempReasonCol = m_pclSheet->GetMaxCols() + 1;
	m_pclSheet->SetMaxCols( lTempReasonCol );

	long lRow = CRViewSSelPM::RD_ExpansionVessel_FirstAvailRow;

	for( CSelectedPMBase *pclRejectedPMBase = m_pclSelectPMList->GetFirstProduct( m_eProductType, CSelectPMList::WL_Rejected ); 
			NULL != pclRejectedPMBase; pclRejectedPMBase = m_pclSelectPMList->GetNextProduct( m_eProductType, CSelectPMList::WL_Rejected ) )
	{
		long lReturn = pRViewSSelPM->FillRowHelper( m_pclSheetDescription, m_eProductType, lRow, pclRejectedPMBase, m_pclSelectPMList->GetpclPMInputUser() );

		if( -1 == lReturn )
		{
			continue;
		}
		else
		{
			// It exists now in the DB a Pleno that is in fact a combination of two modules "P BA4 P" + "P ABR5-R". We want to show this product as
			// one product but with specific data for each of both devices.
			bool bProductCombination = ( CSelectedPMBase::ST_WaterTreatmentCombination == pclRejectedPMBase->GetSelectedType() ) ? true : false;
			
			CSelectedPMBase *pclFirstSelected = NULL;
			CSelectedPMBase *pclSecondSelected = NULL;
			CSelectedWaterTreatmentCombination *pclSelectedWTCombination = NULL;
			CDB_Set *pProductCombination = NULL;

			if( false == bProductCombination )
			{
				// Only one device.
				pclFirstSelected = pclRejectedPMBase;
			}
			else
			{
				// Combination of two devices.
				pclSelectedWTCombination = dynamic_cast<CSelectedWaterTreatmentCombination*>( pclRejectedPMBase );

				if( NULL == pclSelectedWTCombination )
				{
					continue;
				}

				pProductCombination = pclSelectedWTCombination->GetWTCombination();

				if( NULL == pProductCombination )
				{
					continue;
				}

				if( NULL != pclSelectedWTCombination->GetSelectedFirst() )
				{
					pclFirstSelected = pclSelectedWTCombination->GetSelectedFirst();
				}

				if( NULL != pclSelectedWTCombination->GetSelectedSecond() )
				{
					pclSecondSelected = pclSelectedWTCombination->GetSelectedSecond();
				}
			}

			int iRejectReasonCount = 0;

			if( NULL != pclFirstSelected )
			{
				iRejectReasonCount += pclFirstSelected->GetRejectReasonCount();
			}

			if( NULL != pclSecondSelected )
			{
				iRejectReasonCount += pclSecondSelected->GetRejectReasonCount();
			}

			if( 0 == iRejectReasonCount )
			{
				continue;
			}

			CString str;
			str.Format( _T("%i"), iRejectReasonCount );
			m_pclSheet->SetStaticText( lTempReasonCol, lRow, str );
			m_pclSheet->SetCellParam( lTempReasonCol, lRow, (LPARAM)pclRejectedPMBase );

			lRow = lReturn;
		}

		++lRow;
	}

	--lRow;

	// Insert the column containing number of reject reasons.
	int iNbrReasonCol = 2;
	m_pclSheet->SetCellBorder( iNbrReasonCol, lRow, m_pclSheet->GetMaxCols(), lRow, true, SS_BORDERTYPE_BOTTOM, SS_BORDERSTYLE_SOLID, _BLACK );
	m_pclSheet->SetMaxCols( ++lTempReasonCol );
	m_pclSheet->InsCol( iNbrReasonCol );
	m_pclSheet->SetColWidth( iNbrReasonCol, 7 );
	m_pclSheet->CopyRange( iNbrReasonCol + 1, CRViewSSelPM::RD_ExpansionVessel_GroupName, iNbrReasonCol + 1, CRViewSSelPM::RD_ExpansionVessel_GroupName, iNbrReasonCol, CRViewSSelPM::RD_ExpansionVessel_GroupName );
	m_pclSheet->AddCellSpan( iNbrReasonCol, CRViewSSelPM::RD_ExpansionVessel_GroupName, m_pclSheet->GetMaxCols() - iNbrReasonCol - 1, 1 );
	m_pclSheet->SetTextPattern( CSSheet::_SSTextPattern::UserStaticColumn );
	m_pclSheet->SetTextPatternProperty( CSSheet::_SSTextPatternProperty::TPP_FontBold, (LPARAM)FALSE );
	m_pclSheet->SetTextPatternProperty( CSSheet::_SSTextPatternProperty::TPP_StyleWordWrap, ( LPARAM )TRUE );
	m_pclSheet->SetStaticText( iNbrReasonCol, CRViewSSelPM::RD_ExpansionVessel_ColName, IDS_DLGINDSELPMEXCLUDEDPRODUCTS_COLREASONNUMBER );
	m_pclSheet->SetCellBorder( iNbrReasonCol, CRViewSSelPM::RD_ExpansionVessel_FirstAvailRow - 1, iNbrReasonCol, CRViewSSelPM::RD_ExpansionVessel_FirstAvailRow - 1, true, SS_BORDERTYPE_BOTTOM, SS_BORDERSTYLE_SOLID, _BLACK );
	m_pclSheet->SetCellBorder( iNbrReasonCol, lRow, iNbrReasonCol, lRow, true, SS_BORDERTYPE_BOTTOM, SS_BORDERSTYLE_SOLID, _BLACK );

	for( long lLoopRow = CRViewSSelPM::RD_ExpansionVessel_FirstAvailRow; lLoopRow <= lRow; lLoopRow++ )
	{
		CString str = m_pclSheet->GetCellText( lTempReasonCol, lLoopRow );
		m_pclSheet->SetCellText( iNbrReasonCol, lLoopRow, str );
		LPARAM lpParam = m_pclSheet->GetCellParam( lTempReasonCol, lLoopRow );
		m_pclSheet->SetCellParam( iNbrReasonCol, lLoopRow, lpParam );
	}

	m_pclSheet->DelCol( lTempReasonCol );
	m_pclSheet->SetMaxCols( --lTempReasonCol );

	// Now need of the first and last column created by 'RViewSSelPM' because here we have already borders around the TSpread object in
	// the dialog.
	m_pclSheet->DelCol( lTempReasonCol );
	m_pclSheet->SetMaxCols( --lTempReasonCol );
	m_pclSheet->DelCol( 1 );
	m_pclSheet->SetMaxCols( --lTempReasonCol );

	m_pclSheet->SetBool( SSB_REDRAW, TRUE );

	m_iTotalColWidth = 0;
	for( int iLoop = 1; iLoop <= m_pclSheet->GetMaxCols(); ++iLoop )
	{
		m_mapDefaultColWidth[iLoop] = m_pclSheet->GetColWidthInPixelsW( iLoop );

		if( FALSE == m_pclSheet->IsColHidden( iLoop ) )
		{
			m_iTotalColWidth += m_mapDefaultColWidth[iLoop];
		}
	}
}

void CDlgIndSelPMExcludedProducts::_ResizeColumns()
{
	if( NULL == m_pclSheet || NULL == m_pclSheet->GetSafeHwnd() )
	{
		return;
	}
	
	CRect rectDlgClient;
	GetClientRect( &rectDlgClient );

	// Resize spread taking in count the true height and width available.
	CRect rectSpreadClientAll;
	::GetClientRect( m_pclSheet->GetSafeHwnd(), &rectSpreadClientAll );
	CRect rectSpreadClientReal;
	m_pclSheet->GetClientRect( &rectSpreadClientReal );

	long lVScrollBarWidth = ( rectSpreadClientAll.right > rectSpreadClientReal.right ) ? GetSystemMetrics( SM_CXVSCROLL ) : 0;

	// If there is no column defined...
	if( 0 == m_pclSheet->GetMaxCols() )
	{
		return;
	}

	// Convert dialog unit in pixel.
	CRect rectDLG( 0, 0, 7, 7 );
	MapDialogRect( &rectDLG );
	int iBorder = rectDLG.Width();

	m_pclSheet->SetBool( SSB_REDRAW, FALSE );

	int iTotalWidth = 0;
	int iAvailableWidth = rectDlgClient.Width() - lVScrollBarWidth - ( 2 * iBorder );
	if( iAvailableWidth < m_iTotalColWidth )
	{
		iAvailableWidth = m_iTotalColWidth;
	}

	for( int iLoop = 1; iLoop < m_pclSheet->GetMaxCols(); ++iLoop )
	{
		if( FALSE == m_pclSheet->IsColHidden( iLoop ) )
		{
			int iWidth = iAvailableWidth * m_mapDefaultColWidth[iLoop] / m_iTotalColWidth;
			m_pclSheet->SetColWidthInPixels( iLoop, iWidth );
			iTotalWidth += iWidth;
		}
	}

	// Last column.
	m_pclSheet->SetColWidthInPixels( m_pclSheet->GetMaxCols(), iAvailableWidth - iTotalWidth );

	m_pclSheet->SetBool( SSB_REDRAW, TRUE );
}

void CDlgIndSelPMExcludedProducts::_UpdateLayout( int cx, int cy )
{
	if( NULL == m_pclSheet || NULL == m_pclSheet->GetSafeHwnd() )
		return;

	SetRedraw( FALSE );

	// Convert dialog unit in pixel.
	CRect rectDLG( 0, 0, 7, 7 );
	MapDialogRect( &rectDLG );
	int iBorder = rectDLG.Width();

	// Move the 'Close' button.
	CRect rectCloseButton;
	m_ButtonClose.GetWindowRect( &rectCloseButton );
	ScreenToClient( &rectCloseButton );
	m_ButtonClose.SetWindowPos( NULL, cx - iBorder - rectCloseButton.Width(), cy - iBorder - rectCloseButton.Height(), -1, -1, SWP_NOZORDER | SWP_NOSIZE );

	// Let 7 dialog unit for left and right border, 7 dialog unit for top  border, and 7 dialog unit between bottom border and close button.
	m_pclSheet->SetWindowPos( NULL, -1, -1, cx - ( 2 * iBorder ), cy - iBorder - rectCloseButton.Height() - ( 2 * iBorder ), SWP_NOZORDER | SWP_NOMOVE );
	m_pclSheet->EmptyMessageQueue();

	_ResizeColumns();

	SetRedraw( TRUE );
	Invalidate();
	UpdateWindow();
}
