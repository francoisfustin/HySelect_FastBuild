#include "stdafx.h"


#include "TASelect.h"
#include "TASelectDoc.h"
#include "MainFrm.h"
#include "DlgWizPMRightViewInputBase.h"
#include "DlgIndSelPMSysVolCooling.h"
#include "RViewWizardSelBase.h"
#include "RViewWizardSelPM.h"

#ifdef _DEBUG
#define new DEBUG_NEW
#undef THIS_FILE
static char THIS_FILE[] = __FILE__;
#endif

CDlgWizardPM_RightViewInput_Base::CDlgWizardPM_RightViewInput_Base( CWizardSelPMParams *pclWizardSelPMParams, int iRightViewInputID, UINT iDialogID, CWnd *pclParent )
	: CDialogEx( iDialogID, pclParent )
{
	m_pclWizardSelPMParams = pclWizardSelPMParams;
	m_pclTechParam = NULL;
	m_pclParent = pclParent;
	m_iRightViewInputID = iRightViewInputID;
	m_strMainTitle = _T("Undefined");
	m_bBackEnable = false;
	m_bNextEnable = false;
	m_strBackText = _T("");
	m_strNextText = _T("");
	m_rectBackClickableArea.SetRectEmpty();
	m_rectNextClickableArea.SetRectEmpty();
	m_bOnEnChangeEnabled = true;

	m_dSafetyTempLimiterSaved = 0.0;
	m_dSupplyTempSaved = 0.0;
	m_dReturnTempSaved = 0.0;
	m_dFillTempSaved = 0.0;
	m_dMinTempSaved = 0.0;
	m_dMaxTempSaved = 0.0;
	m_dWaterMakeUpTempSaved = 0.0;
	m_dStaticHeightSaved = 0.0;
	m_dPzSaved = 0.0;
	m_dSafetyVRPSaved = 0.0;
	m_dPumpHeadSaved = 0.0;
	m_dWaterMakeUpNetworkPNSaved = 0.0;
	m_dSystemVolumeSaved = 0.0;
	m_dStorageTankVolumeSaved = 0.0;
	m_dStorageTankMaxTempSaved = 0.0;
	m_dInstalledPowerSaved = 0.0;
	m_dMaxWidthSaved = 0.0;
	m_dMaxHeightSaved = 0.0;
	m_dpSVLocationSaved = 0.0;
}

CDlgWizardPM_RightViewInput_Base::~CDlgWizardPM_RightViewInput_Base()
{
	m_clBrushBackground.DeleteObject();
	m_clFontMaintTitle.DeleteObject();
	m_clGoTextFont.DeleteObject();
	m_clBackNextTextFont.DeleteObject();
}

void CDlgWizardPM_RightViewInput_Base::EnableBackButton( bool bEnable )
{
	if( m_bBackEnable != bEnable )
	{
		m_bBackEnable = bEnable;
		InvalidateRect( &m_rectBackClickableArea );
		UpdateWindow();
	}
}

void CDlgWizardPM_RightViewInput_Base::EnableNextButton( bool bEnable )
{
	if( m_bNextEnable != bEnable )
	{
		m_bNextEnable = bEnable;
		InvalidateRect( &m_rectNextClickableArea );
		UpdateWindow();
	}
}

void CDlgWizardPM_RightViewInput_Base::SetMainTitle( CString strText )
{
	m_strMainTitle = strText;
}

void CDlgWizardPM_RightViewInput_Base::OnSpecialValidation( CWnd *pWnd, int iVirtualKey )
{
	if( NULL == pWnd )
	{
		return;
	}

	if( VK_DOWN == iVirtualKey || VK_TAB == iVirtualKey )
	{
		CWnd *pWndNextFocus = GetNextDlgTabItem( pWnd, FALSE );

		if( NULL != pWndNextFocus && pWnd != pWndNextFocus )
		{
			CRect rectCurrent;
			pWnd->GetWindowRect( &rectCurrent );
			ScreenToClient( &rectCurrent );
			CRect rectNext;
			pWndNextFocus->GetWindowRect( &rectNext );
			ScreenToClient( &rectNext );

			if( rectNext.top > rectCurrent.top )
			{
				pWndNextFocus->SetFocus();
			}
		}
	}
	else if( VK_UP == iVirtualKey )
	{
		CWnd *pWndPrevFocus = GetNextDlgTabItem( pWnd, TRUE );

		if( NULL != pWndPrevFocus && pWnd != pWndPrevFocus )
		{
			CRect rectCurrent;
			pWnd->GetWindowRect( &rectCurrent );
			ScreenToClient( &rectCurrent );
			CRect rectPrev;
			pWndPrevFocus->GetWindowRect( &rectPrev );
			ScreenToClient( &rectPrev );

			if( rectPrev.top < rectCurrent.top )
			{
				pWndPrevFocus->SetFocus();
			}
		}
	}
	else if( VK_RETURN == iVirtualKey )
	{
		
	}
}

BEGIN_MESSAGE_MAP( CDlgWizardPM_RightViewInput_Base, CDialogEx )
	ON_WM_PAINT()
	ON_WM_ERASEBKGND()
    ON_WM_SIZE()
	ON_WM_LBUTTONDOWN()
	ON_MESSAGE( WM_USER_PIPECHANGE, OnPipeChange )
	ON_MESSAGE( WM_USER_TECHPARCHANGE, OnTechParChange )
	ON_MESSAGE( WM_USER_UNITCHANGE, OnUnitChange )
	ON_MESSAGE( WM_USER_WATERCHANGE, OnWaterChange )
END_MESSAGE_MAP()

void CDlgWizardPM_RightViewInput_Base::DoDataExchange( CDataExchange *pDX )
{
	CDialogEx::DoDataExchange( pDX );
}

void CDlgWizardPM_RightViewInput_Base::OnOK()
{
	PREVENT_ENTER_KEY
	CDialogEx::OnOK();
}

void CDlgWizardPM_RightViewInput_Base::OnCancel()
{
	// Do nothing
}

BOOL CDlgWizardPM_RightViewInput_Base::OnInitDialog()
{
	if( NULL == m_pclWizardSelPMParams || NULL == m_pclWizardSelPMParams->m_pTADS || NULL == m_pclWizardSelPMParams->m_pTADS->GetpTechParams() )
	{
		ASSERTA_RETURN( FALSE );
	}

	CDialogEx::OnInitDialog();

	m_pclTechParam = m_pclWizardSelPMParams->m_pTADS->GetpTechParams();

	CDC *pDC = GetDC();

	LOGFONT rLogFont;
	::GetObject( (HFONT)GetStockObject( DEFAULT_GUI_FONT ), sizeof( rLogFont ), &rLogFont );
	rLogFont.lfHeight = -MulDiv( 17, GetDeviceCaps( pDC->GetSafeHdc(), LOGPIXELSY ), 72 );
	m_clFontMaintTitle.CreateFontIndirect( &rLogFont );
	
	rLogFont.lfWeight = FW_BOLD;
	rLogFont.lfHeight = -MulDiv( 12, GetDeviceCaps( pDC->GetSafeHdc(), LOGPIXELSY ), 72 );
	m_clGoTextFont.CreateFontIndirect( &rLogFont );

	m_clBackNextTextFont.CreateFontIndirect( &rLogFont );
    m_strBackText = TASApp.LoadLocalizedString( IDS_BUTTON_BACK );
    m_strNextText = TASApp.LoadLocalizedString( IDS_BUTTON_NEXT );

	ReleaseDC( pDC );

	m_clBrushBackground.CreateSolidBrush( _WHITE );

	m_ToolTip.Create( this, TTS_NOPREFIX );
	InitToolTips();

	return TRUE;
}

void CDlgWizardPM_RightViewInput_Base::OnPaint()
{
	CDialogEx::OnPaint();

	CRect rect;
	GetClientRect( &rect );

	CDC *pDC = GetDC();

	COLORREF crBackgroundColor = ( false == ( (CRViewWizardSelPM*)m_pclParent )->IsEditingMode() ) ? _TAH_TITLE_MAIN : _TAH_TITLE_MAIN_REEDIT;
	CBrush brush( crBackgroundColor );
	CBrush *pOldBrush = pDC->SelectObject( &brush );

	// Draw background main title.
	CRect rectFill( 0, 10, rect.right + 1, 40 );
	pDC->FillRect( &rectFill, &brush );

	// Draw main title text.
	CFont* pOldFont = (CFont*)pDC->SelectObject( &m_clFontMaintTitle );
	pDC->SetBkMode( TRANSPARENT );
	UINT uiDTFlags = DT_END_ELLIPSIS | DT_CENTER | DT_VCENTER;

	// Center text.
	pDC->SetTextColor( RGB( 255, 255, 255 ) );
	pDC->DrawText( m_strMainTitle, rectFill, uiDTFlags );

	// Draw arrow back button.
	COLORREF crBackArrow = ( true == m_bBackEnable ) ? RGB( 255, 255, 255 ) : RGB( 192, 192, 192 );
	CPen penBackArrow;
	penBackArrow.CreatePen( PS_SOLID, 2, crBackArrow );
	CPen *pOldPen = pDC->SelectObject( &penBackArrow );
	pDC->MoveTo( rectFill.left + 10, rectFill.top + 10 );
	pDC->LineTo( rectFill.left + 5, rectFill.top + 15 );
	pDC->LineTo( rectFill.left + 10, rectFill.top + 20 );

	// Draw text back button.
	CRect rectBackText = rectFill;
	rectBackText.left += 13;
	rectBackText.top += 5;
	(CFont*)pDC->SelectObject( &m_clBackNextTextFont );
	pDC->SetBkMode( TRANSPARENT );
	pDC->SetTextColor( ( true == m_bBackEnable ) ? RGB( 255, 255, 255 ) : RGB( 192, 192, 192 ) );
	
	// Initialize the clickable area for the back button.
	// Remark: need to be done only one time for this button.
	if( TRUE == m_rectBackClickableArea.IsRectEmpty() )
	{
		pDC->DrawText( m_strBackText, rectBackText, DT_END_ELLIPSIS | DT_VCENTER | DT_LEFT | DT_CALCRECT );
		m_rectBackClickableArea.left = 0;
		m_rectBackClickableArea.top = rectFill.top;
		m_rectBackClickableArea.right = 13 + rectBackText.Width() + 5;
		m_rectBackClickableArea.bottom = rectFill.bottom;
	}

	pDC->DrawText( m_strBackText, rectBackText, DT_END_ELLIPSIS | DT_VCENTER | DT_LEFT );

	// Draw arrow next button.
	COLORREF crNextArrow = ( true == m_bNextEnable ) ? RGB( 255, 255, 255 ) : RGB( 192, 192, 192 );
	CPen penNextArrow;
	penNextArrow.CreatePen( PS_SOLID, 2, crNextArrow );
	pDC->SelectObject( &penNextArrow );
	pDC->MoveTo( rectFill.right - 12, rectFill.top + 10 );
	pDC->LineTo( rectFill.right - 7, rectFill.top + 15 );
	pDC->LineTo( rectFill.right - 12, rectFill.top + 20 );

	// Draw text next button.
	CRect rectNextText = rectFill;
	rectNextText.top += 5;
	(CFont*)pDC->SelectObject( &m_clBackNextTextFont );
	pDC->SetBkMode( TRANSPARENT );
	pDC->SetTextColor( ( true == m_bNextEnable ) ? RGB( 255, 255, 255 ) : RGB( 192, 192, 192 ) );

	// Initialize the clickable area for the next button.
	// Remark: need to be done each time for this button.
	pDC->DrawText( m_strNextText, rectNextText, DT_END_ELLIPSIS | DT_VCENTER | DT_LEFT | DT_CALCRECT );
	m_rectNextClickableArea.left = rectFill.right - 15 - rectNextText.Width() - 5;
	m_rectNextClickableArea.top = rectFill.top;
	m_rectNextClickableArea.right = rectFill.right;
	m_rectNextClickableArea.bottom = rectFill.bottom;

	rectNextText.right = m_rectNextClickableArea.left + rectNextText.Width() + 5;
	rectNextText.left = m_rectNextClickableArea.left + 5;
	pDC->DrawText( m_strNextText, rectNextText, DT_END_ELLIPSIS | DT_VCENTER | DT_LEFT );

	pDC->SelectObject( pOldFont );
	pDC->SelectObject( pOldBrush );

	brush.DeleteObject();
	penBackArrow.DeleteObject();
	penNextArrow.DeleteObject();
	ReleaseDC( pDC );
}

BOOL CDlgWizardPM_RightViewInput_Base::OnEraseBkgnd( CDC *pDC )
{
	CRect rect;
	pDC->GetClipBox( &rect );
	pDC->FillRect( &rect, &m_clBrushBackground );

	return TRUE;
}

void CDlgWizardPM_RightViewInput_Base::OnLButtonDown( UINT nFlags, CPoint point )
{
	if( TRUE == m_rectBackClickableArea.PtInRect( point ) )
	{
		m_pclParent->PostMessageW( WM_USER_RIGHTVIEWWIZPM_BACKNEXT, WM_RVWPMBN_Back, 0 );
	}
	else if( TRUE == m_rectNextClickableArea.PtInRect( point ) )
	{
		m_pclParent->PostMessageW( WM_USER_RIGHTVIEWWIZPM_BACKNEXT, WM_RVWPMBN_Next, 0 );
	}
	else
	{
		__super::OnLButtonDown( nFlags, point );
	}
}

void CDlgWizardPM_RightViewInput_Base::InitNumericalEdit( CExtNumEdit *pEdit, ePHYSTYPE phystype )
{
	if( NULL != pEdit )
	{
		pEdit->SetPhysicalType( phystype );
		pEdit->SetEditSign( CNumString::ePositive );
		pEdit->SetMinDblValue( 0.0 );

		if( _U_TEMPERATURE == phystype )
		{
			pEdit->SetEditSign( CNumString::eBoth );
			pEdit->SetMinDblValue( -273.15 );
		}

		pEdit->Update();
	}
}

void CDlgWizardPM_RightViewInput_Base::ResetToolTipAndErrorBorder( CExtNumEdit *pclExtEdit, bool bToolTips, bool bErrorBorder )
{
	if( NULL != m_ToolTip.GetSafeHwnd() )
	{
		m_arExtEditList.RemoveAll();
		
		if( NULL == pclExtEdit )
		{
			GetExtEditList( m_arExtEditList );
		}
		else
		{
			m_arExtEditList.Add( pclExtEdit );
		}

		if( 0 == m_arExtEditList.GetCount() )
		{
			return;
		}

		if( NULL != m_ToolTip.GetSafeHwnd() )
		{
			int iLoop = 0;

			CExtNumEdit *pNextEdit = ( NULL != pclExtEdit ) ? pclExtEdit : m_arExtEditList[iLoop];

			do
			{
				if( NULL == pclExtEdit || pclExtEdit == pNextEdit )
				{
					if( true == bToolTips )
					{
						m_ToolTip.DelTool( m_arExtEditList[iLoop] );
					}

					if( true == bErrorBorder )
					{
						pNextEdit->ResetDrawBorder();
					}
				}

				// Stop if this method has been called with a specific 'pWnd' otherwise continue with all controls in 'm_arExtEditList'.
				pNextEdit = ( NULL != pclExtEdit ) ? NULL : m_arExtEditList[++iLoop];
			}
			while( NULL != pNextEdit );
		}

		m_ToolTip.SetMaxTipWidth( 1000 );
	}
}

void CDlgWizardPM_RightViewInput_Base::FillComboNorm( CExtNumEditComboBox *pclComboPressNorm, CString strNormID )
{
	if( NULL == m_pclWizardSelPMParams->m_pTADB || NULL == pclComboPressNorm )
	{
		ASSERT_RETURN;
	}

	CRankEx rkList;
	DWORD_PTR dwSelItem = 0;

	CString strComboNorm = _T("");
	
	switch( m_pclWizardSelPMParams->m_pclSelectPMList->GetpclPMInputUser()->GetApplicationType() )
	{
		case ProjectType::Heating:
			 strComboNorm = _T("PMCOMBONORM_HEATING");
			 break;

		case ProjectType::Cooling:
			strComboNorm = _T("PMCOMBONORM_COOLING");
			break;

		case ProjectType::Solar:
			strComboNorm = _T("PMCOMBONORM_SOLAR");
			break;
	}

	CDB_MultiString *pclMultiString = (CDB_MultiString *)( m_pclWizardSelPMParams->m_pTADB->Get( strComboNorm ).MP );
	ASSERT( NULL != pclMultiString );

	int i = 0;
	for( int i = 0; i < pclMultiString->GetNumofLines(); i++ )
	{
		CString strID = pclMultiString->GetString( i );
		
		CDB_StringID *pclNormID = (CDB_StringID *)( m_pclWizardSelPMParams->m_pTADB->Get( strID ).MP );
		ASSERT( NULL != pclNormID );

		if( strNormID == strID )
		{
			dwSelItem = ( LPARAM )pclNormID->GetIDPtr().MP;
		}

		wstring str = pclNormID->GetString();
		rkList.Add( str, i, ( LPARAM )pclNormID->GetIDPtr().MP );
	}

	rkList.Transfer( pclComboPressNorm );
	int iSel = pclComboPressNorm->FindItemDataPtr( dwSelItem );

	if( -1 == iSel )
	{
		iSel = 0;
	}

	pclComboPressNorm->SetCurSel( iSel );
}

void CDlgWizardPM_RightViewInput_Base::FillComboPressOn( CExtNumEditComboBox *pclComboPressOn, PressurON ePressOn )
{
	if( NULL == m_pclWizardSelPMParams || NULL == m_pclWizardSelPMParams->m_pTADS || NULL == m_pclWizardSelPMParams->m_pTADS->GetpTechParams() )
	{
		ASSERT_RETURN;
	}
	
	if( NULL == pclComboPressOn )
	{
		return;
	}

	CDS_TechnicalParameter *pclTechParam = m_pclWizardSelPMParams->m_pTADS->GetpTechParams();
	CRankEx PressurOnList;
	pclComboPressOn->ResetContent();
	CRankEx rkList;
	
	rkList.Add( ( LPCTSTR )TASApp.LoadLocalizedString( pclTechParam->GetPressurONIDS( PressurON::poPumpDischarge ) ), PressurON::poPumpDischarge,
				PressurON::poPumpDischarge );

	rkList.Add( ( LPCTSTR )TASApp.LoadLocalizedString( pclTechParam->GetPressurONIDS( PressurON::poPumpSuction ) ), PressurON::poPumpSuction,
				PressurON::poPumpSuction );

	rkList.Transfer( pclComboPressOn );

	int iSelPos = 0;

	for( int i = 0; i <pclComboPressOn->GetCount(); i++ )
	{
		if( ePressOn == (PressurON)(DWORD_PTR)pclComboPressOn->GetItemDataPtr( i ) )
		{
			iSelPos = i;
			break;
		}
	}

	pclComboPressOn->SetCurSel( iSelPos );

	if( pclComboPressOn->GetCount() <= 1 )
	{
		pclComboPressOn->EnableWindow( false );
	}
	else
	{
		pclComboPressOn->EnableWindow( true );
	}
}

void CDlgWizardPM_RightViewInput_Base::FillComboPressMaintType( CExtNumEditComboBox *pclComboPMType, CString strPressureTypeID, bool bNoNone )
{
	if( NULL == m_pclWizardSelPMParams || NULL == m_pclWizardSelPMParams->m_pTADB || NULL == pclComboPMType )
	{
		return;
	}

	pclComboPMType->ResetContent();
	IDPTR SelectedIDPtr;

	if( false == strPressureTypeID.IsEmpty() )
	{
		SelectedIDPtr = m_pclWizardSelPMParams->m_pTADB->Get( ( LPCTSTR )strPressureTypeID );
	}

	// Fill combo pressure maintenance type.
	std::map<int, CData *> MapLst;
	CTableOrdered *pTabOrd = ( CTableOrdered * )( m_pclWizardSelPMParams->m_pTADB->Get( _T("PRESSMAINTTYPECB_TAB") ).MP );

	if( NULL != pTabOrd )
	{
		pTabOrd->FillMapListOrdered( &MapLst );
	}
	else
	{
		ASSERT_RETURN;
	}

	// Extract all PM product tab from VSSLTYPE_TAB.
	// Remark: 'VSSLTYPE_TAB' contains 'Expansion vessel', 'Expansion vessel with membrane', 'Primary vessel', 'Secondary vessel',
	//         'Intermediate vessel' and 'Aqua vessel'.
	CTable *pTabType = ( CTable * )( m_pclWizardSelPMParams->m_pTADB->Get( _T("VSSLTYPE_TAB") ).MP );

	if( NULL == pTabType )
	{
		ASSERT_RETURN;
	}

	// Use ordered set to remove duplicated entry.
	std::set<CTable *> setVesselType;

	for( IDPTR idptr = pTabType->GetFirst(); NULL != idptr.MP; idptr = pTabType->GetNext( idptr.MP ) )
	{
		CDB_StringID *pStrID = dynamic_cast<CDB_StringID *>( idptr.MP );
		CTable *pTab = dynamic_cast<CTable *>( m_pclWizardSelPMParams->m_pTADB->Get( pStrID->GetIDstr2() ).MP );
		setVesselType.insert( pTab );
	}

	// For table present in the setPMPTabID verify product availability	and store corresponding type.
	std::set<CData *> setPMPTypeID;

	for each( CTable * pTab in setVesselType )
	{
		if( NULL == pTab )
		{
			ASSERT_CONTINUE;
		}

		for( IDPTR idptr = pTab->GetFirst(); NULL != idptr.MP; idptr = pTab->GetNext( idptr.MP ) )
		{
			CDB_Product *pPMP = dynamic_cast<CDB_Product *>( idptr.MP );

			if( NULL == pPMP )
			{
				ASSERT_CONTINUE;
			}

			if( false == pPMP->IsAvailable() )
			{
				continue;
			}

			setPMPTypeID.insert( pPMP->GetTypeIDPtr().MP );
		}
	}

	std::set<CData *> setNoExistingEntry;
	// Identify un-available CB entry.
	int iSelPos = 0;

	// Remark: 'MapList' contains list of all 'PRESSMAINTTYPECB_TAB' -> 'All', 'Expansion vessel', 'Expansion vessel (membrane)',
	//         'With compressor', 'With pump' and 'None'.
	for each( std::pair<int, CData *>PMDataType in MapLst )
	{
		CTable *pTab = dynamic_cast<CTable *>( PMDataType.second );

		if( NULL == pTab )
		{
			ASSERT_CONTINUE;
		}
		// HYS-1537 : Compresso and transfero are not available in Solar mode
		if( Solar == m_pclWizardSelPMParams->m_pclSelectPMList->GetpclPMInputUser()->GetApplicationType() )
		{
			if( true == ( (IDPTR)pTab->GetIDPtr() ).IDMatch( L"PMT_WITHCOMPRESS_TAB" ) )
			{
				InitToolTips();

				continue;
			}

			if( true == ( (IDPTR)pTab->GetIDPtr() ).IDMatch( L"PMT_WITHPUMP_TAB" ) )
			{
				continue;
			}
		}
		bool bFound = false;
		int iCount = 0;

		// Each table in 'MapList' contains one or more vessel type ID.
		// For example 'PMT_EXPVSSL_TAB' contains 'VSSLTYPE_STAT' and 'VSSLTYPE_AQUA'.
		for( IDPTR idptr = pTab->GetFirst(); NULL != idptr.MP && false == bFound; idptr = pTab->GetNext( idptr.MP ) )
		{
			iCount++;
			CDB_StringID *pStrID = dynamic_cast<CDB_StringID *>( idptr.MP );

			// All objects in a 'CTable' is in fact an ID on a 'CData' inherited object.
			// For the 'PRESSMAINTTYPECB_TAB' table, it's in fact an ID on a 'CDB_StringID' object.
			// Except 'PMT_ALL_TAB' table that has an ID on a 'CTable' object instead of 'CDB_StringID'. This is why
			// in this case 'pStrID' is NULL.

			if( NULL == pStrID )
			{
				if( NULL != dynamic_cast<CTable *>( idptr.MP ) )
				{
					bFound = true;
				}
				else
				{
					ASSERT( 0 );
				}

				continue;
			}

			if( setPMPTypeID.end() != setPMPTypeID.find( idptr.MP ) )
			{
				bFound = true;
			}
		}

		if( 0 == iCount && false == bNoNone )
		{
			// Specific treatment for entries 'PMT_NONE_TAB', the table has no 'CDB_StringID'.
			bFound = true;
		}

		// Add this entry in the CB.
		if( true == bFound )
		{
			int i = pclComboPMType->AddString( pTab->GetName() );
            pclComboPMType->SetItemData( i, (DWORD_PTR)pTab );

			if( SelectedIDPtr.MP == pTab )
			{
				iSelPos = i;
			}
		}
	}

	if( pclComboPMType->GetCount() >= 0 )
	{
		pclComboPMType->EnableWindow( TRUE );
		pclComboPMType->SetCurSel( iSelPos );
	}
	else
	{
		pclComboPMType->EnableWindow( FALSE );
	}
}

void CDlgWizardPM_RightViewInput_Base::FillComboWaterMakeUpType( CExtNumEditComboBox *pComboWaterMakeUpType, CString strWaterMakeUpID )
{
	if( NULL == m_pclWizardSelPMParams || NULL == m_pclWizardSelPMParams->m_pTADB || NULL == pComboWaterMakeUpType )
	{
		return;
	}

	pComboWaterMakeUpType->ResetContent();
	IDPTR SelectedIDPtr;

	if( false == strWaterMakeUpID.IsEmpty() )
	{
		SelectedIDPtr = m_pclWizardSelPMParams->m_pTADB->Get( ( LPCTSTR )strWaterMakeUpID );
	}

	// Fill combo pressure maintenance type.
	CRankEx rkList;
	_string str;
	DWORD_PTR dwSelItem = 0;
	CTable *pTab = ( CTable * )( m_pclWizardSelPMParams->m_pTADB->Get( _T("WATERMAKEUPTYPECB_TAB") ).MP );

	for( IDPTR idptr = pTab->GetFirst(); NULL != idptr.MP; idptr = pTab->GetNext( idptr.MP ) )
	{
		CDB_StringID *pStrID = dynamic_cast<CDB_StringID *>( idptr.MP );

		if( NULL == pStrID )
		{
			ASSERT_CONTINUE;
		}

		str = pStrID->GetString();
		int i;

		if( false == pStrID->GetIDstrAs<int>( 0, i ) )
		{
			ASSERT_CONTINUE;
		}

		if( strWaterMakeUpID == idptr.ID )
		{
			dwSelItem = ( LPARAM )idptr.MP;
		}

		rkList.Add( str, i, ( LPARAM )idptr.MP );
	}

	rkList.Transfer( pComboWaterMakeUpType );
	int iSelPos = pComboWaterMakeUpType->FindItemDataPtr( dwSelItem );

	if( -1 == iSelPos )
	{
		iSelPos = 0;
	}

	pComboWaterMakeUpType->SetCurSel( iSelPos );

	if( pComboWaterMakeUpType->GetCount() >= 0 )
	{
		pComboWaterMakeUpType->EnableWindow( TRUE );
		pComboWaterMakeUpType->SetCurSel( iSelPos );
	}
	else
	{
		pComboWaterMakeUpType->EnableWindow( FALSE );
	}
}

CString CDlgWizardPM_RightViewInput_Base::GetNormID( CExtNumEditComboBox *pComboNorm )
{
	if( NULL == pComboNorm )
	{
		return _T("");
	}

	CString strNormID = _T("PM_NONE");
	int iCur = pComboNorm->GetCurSel();

	if( iCur >= 0 )
	{
		CDB_StringID *pStrID = NULL;
		pStrID = dynamic_cast< CDB_StringID *>( (CData *)pComboNorm->GetItemDataPtr( iCur ) );

		if( NULL != pStrID )
		{
			strNormID = pStrID->GetIDPtr().ID;
		}
	}

	return strNormID;
}

void CDlgWizardPM_RightViewInput_Base::SetNormID( CExtNumEditComboBox *pclComboNorm, CString strNormID )
{
	if( NULL == pclComboNorm || true == strNormID.IsEmpty() )
	{
		return;
	}

	for( int iLoop = 0; iLoop < pclComboNorm->GetCount(); iLoop++ )
	{
		CDB_StringID *pStrID = dynamic_cast< CDB_StringID *>( ( CData * )pclComboNorm->GetItemDataPtr( iLoop ) );

		if( NULL != pStrID && strNormID == CString( pStrID->GetIDPtr().ID ) )
		{
			pclComboNorm->SetCurSel( iLoop );
			return;
		}
	}
}

PressurON CDlgWizardPM_RightViewInput_Base::GetPressurOn( CExtNumEditComboBox *pclComboPressOn )
{
	if( NULL == pclComboPressOn )
	{
		return PressurON::poPumpSuction;
	}

	PressurON ePressurON = PressurON::poPumpSuction;		// By default in case of error
	int iCur = pclComboPressOn->GetCurSel();

	if( iCur >= 0 )
	{
		ePressurON = (PressurON)(DWORD_PTR)pclComboPressOn->GetItemDataPtr( iCur );

		if( ePressurON < PressurON::poPumpSuction || ePressurON >= PressurON::poLast )
		{
			ASSERT( 0 );
			ePressurON = PressurON::poPumpSuction;
		}
	}

	return ePressurON;
}

void CDlgWizardPM_RightViewInput_Base::SetPressurOn( CExtNumEditComboBox *pclComboPressOn, PressurON ePressurOn )
{
	if( NULL == pclComboPressOn || 0 == pclComboPressOn->GetCount() )
	{
		return;
	}

	for( int iLoop = 0; iLoop < pclComboPressOn->GetCount(); iLoop++ )
	{
		PressurON eCurPressurON = ( PressurON )(DWORD_PTR)pclComboPressOn->GetItemDataPtr( iLoop );
		
		if( eCurPressurON == ePressurOn )
		{
			pclComboPressOn->SetCurSel( iLoop );
			return;
		}
	}
}

bool CDlgWizardPM_RightViewInput_Base::IsFluidTemperatureOK( CExtNumEdit *pclCurrentEdit, CString strPrefix )
{
	if( NULL == m_pclWizardSelPMParams )
	{
		ASSERTA_RETURN( false );
	}

	if( NULL == pclCurrentEdit )
	{
		return true;
	}

	bool bFluidOK = true;
	double dCurrentTemp = pclCurrentEdit->GetCurrentValSI();
	double dSavedTemp = dCurrentTemp;
	CWaterChar::eFluidRetCode RetCode = m_pclWizardSelPMParams->m_WC.CheckFluidData( dCurrentTemp );

	if( CWaterChar::efrcFluidOk != RetCode )
	{
		CString strToolTip;

		if( CWaterChar::efrcTemperatureTooLow == RetCode )
		{
			CString strFreezePtTemp = WriteCUDouble( _U_TEMPERATURE, dCurrentTemp, true, -1, 2 );
			CString strFreezingPointMsg;
			FormatString( strFreezingPointMsg, IDS_DLGINDSELPMPANELS_TTERRORFREEZINGPOINT, strFreezePtTemp );
			CString strSecondStr = CString( WriteCUDouble( _U_TEMPERATURE, dSavedTemp, true, -1, 2 ) ) + _T(" <= ") + strFreezingPointMsg;
			FormatString( strToolTip, IDS_DLGINDSELPMPANELS_TTERRORFLUIDDATATEMPTOOLOW, strPrefix, strSecondStr );
		}
		else if( CWaterChar::efrcTempTooHigh == RetCode )
		{
			CString strTemp = CString( WriteCUDouble( _U_TEMPERATURE, dSavedTemp, true, -1, 2 ) ) + _T(" > ") + 
					CString( WriteCUDouble( _U_TEMPERATURE, dCurrentTemp, true, -1, 2 ) );

			FormatString( strToolTip, IDS_DLGINDSELPMPANELS_TTERRORFLUIDDATATEMPTOOHIGH, strPrefix, strTemp );
		}
		else if( CWaterChar::efrcAdditiveTooHigh == RetCode )
		{
			strToolTip = TASApp.LoadLocalizedString( IDS_DLGINDSELPMPANELS_TTERRORFLUIDDATATEMPADDTOOHIGH );
		}

		m_ToolTip.AddToolWindow( pclCurrentEdit, strToolTip );
		pclCurrentEdit->SetDrawBorder( true, _RED );

		bFluidOK = false;
	}

	return bFluidOK;
}

int CDlgWizardPM_RightViewInput_Base::IsWaterMakeUpNetworkPNOK( CExtNumEdit *pclCurrentEdit )
{
	if( NULL == pclCurrentEdit )
	{
		return 0;
	}

	// By default reset error.
	pclCurrentEdit->ResetDrawBorder();
	ResetToolTipAndErrorBorder( pclCurrentEdit );

	// After reset reload default values.
	InitToolTips( NULL );

	int iErrorCode = 0;

	if( pclCurrentEdit->GetCurrentValSI() <= 0.0 )
	{
		CString strToolTip = TASApp.LoadLocalizedString( IDS_DLGINDSELPMPANELS_TTERRORWATERMAKEUPPSN );
		m_ToolTip.AddToolWindow( pclCurrentEdit, strToolTip );
		pclCurrentEdit->SetDrawBorder( true, _RED );
		iErrorCode = Error_Pressure_WaterMakeUpNetworkPN;
	}

	return iErrorCode;
}

int CDlgWizardPM_RightViewInput_Base::IsPumpHeadOK( CExtNumEdit *pclCurrentEdit, CExtNumEditComboBox *pclComboPressOn )
{
	if( NULL == pclCurrentEdit || NULL == pclComboPressOn )
	{
		return 0;
	}

	// By default reset error.
	pclCurrentEdit->ResetDrawBorder();
	ResetToolTipAndErrorBorder( pclCurrentEdit );

	// After reset reload default values.
	InitToolTips( NULL );

	int iErrorCode = 0;

	if( PressurON::poPumpDischarge == GetPressurOn( pclComboPressOn ) && pclCurrentEdit->GetCurrentValSI() <= 0.0 )
	{
		CString strToolTip = TASApp.LoadLocalizedString( IDS_DLGINDSELPMPANELS_TTERRORPUMPHEAD );
		m_ToolTip.AddToolWindow( pclCurrentEdit, strToolTip );
		pclCurrentEdit->SetDrawBorder( true, _RED );
		iErrorCode = Error_Pressure_PumpHead;
	}

	return iErrorCode;
}

int CDlgWizardPM_RightViewInput_Base::IsHeatingCoolingSystemVolumeOK( CExtNumEdit *pclCurrentEdit )
{
	if( NULL == pclCurrentEdit )
	{
		return 0;
	}

	// By default reset error.
	pclCurrentEdit->ResetDrawBorder();
	ResetToolTipAndErrorBorder( pclCurrentEdit );

	// After reset reload default values.
	InitToolTips( NULL );

	int iErrorCode = 0;

	if( pclCurrentEdit->GetCurrentValSI() <= 0.0 )
	{
		CString strTooltip = TASApp.LoadLocalizedString( IDS_DLGINDSELPMPANELS_TTERRORSYSTEMVOLUME );
		m_ToolTip.AddToolWindow( pclCurrentEdit, strTooltip );
		pclCurrentEdit->SetDrawBorder( true, _RED );
		iErrorCode = Error_Installation_SystemVolume;
	}

	return iErrorCode;
}

int CDlgWizardPM_RightViewInput_Base::IsSolarSystemVolumeOK( CExtNumEdit *pclEditSystemVolume, CExtNumEdit *pclSolarContent )
{
	if( NULL == pclEditSystemVolume || NULL == pclSolarContent )
	{
		return 0;
	}

	ResetToolTipAndErrorBorder( pclEditSystemVolume );
	ResetToolTipAndErrorBorder( pclSolarContent );

	// After reset reload default values.
	InitToolTips( pclEditSystemVolume );
	InitToolTips( pclSolarContent );

	int iErrorCode = 0;

	if( pclEditSystemVolume->GetCurrentValSI() <= 0.0 )
	{
		CString strTooltip = TASApp.LoadLocalizedString( IDS_DLGINDSELPMPANELS_TTERRORSYSTEMVOLUME );
		m_ToolTip.AddToolWindow( pclEditSystemVolume, strTooltip );
		pclEditSystemVolume->SetDrawBorder( true, _RED );
		iErrorCode = Error_Installation_SystemVolume;
	}
	else if( pclEditSystemVolume->GetCurrentValSI() <= pclSolarContent->GetCurrentValSI() )
	{
		CString strToolTip;

		CString strTemp = CString( WriteCUDouble( _U_VOLUME, pclEditSystemVolume->GetCurrentValSI(), true, -1, 2 ) ) + _T(" <= ") 
				+ CString( WriteCUDouble( _U_VOLUME, pclSolarContent->GetCurrentValSI(), true, -1, 2 ) );

		FormatString( strToolTip, IDS_DLGINDSELPMPANELS_TTERRORSYSTEMVOLUME, strTemp );

		m_ToolTip.AddToolWindow( pclEditSystemVolume, strToolTip );
		pclEditSystemVolume->SetDrawBorder( true, _RED );

		strTemp = CString( WriteCUDouble( _U_VOLUME, pclSolarContent->GetCurrentValSI(), true, -1, 2 ) ) + _T(" >= ") 
				+ CString( WriteCUDouble( _U_VOLUME, pclEditSystemVolume->GetCurrentValSI(), true, -1, 2 ) );

		FormatString( strToolTip, IDS_DLGINDSELPMPANELS_TTERRORSOLARCONTENTTOHIGH, strTemp );

		m_ToolTip.AddToolWindow( pclSolarContent, strToolTip );
		pclSolarContent->SetDrawBorder( true, _RED );
		iErrorCode = Error_Installation_SystemVolume | Error_Installation_SolarContent;
	}

	return iErrorCode;
}

int CDlgWizardPM_RightViewInput_Base::IsSolarSolarContentOK( CExtNumEdit *pclSolarContent, CExtNumEdit *pclEditSystemVolume )
{
	if( NULL == pclSolarContent || NULL == pclEditSystemVolume )
	{
		return 0;
	}

	ResetToolTipAndErrorBorder( pclSolarContent );
	ResetToolTipAndErrorBorder( pclEditSystemVolume );

	// After reset reload default values.
	InitToolTips( pclSolarContent );
	InitToolTips( pclEditSystemVolume );

	int iErrorCode = 0;

	if( pclSolarContent->GetCurrentValSI() <= 0.0 )
	{
		CString strTooltip = TASApp.LoadLocalizedString( IDS_DLGINDSELPMPANELS_TTERRORSOLARCONTENT );
		m_ToolTip.AddToolWindow( pclSolarContent, strTooltip );
		pclSolarContent->SetDrawBorder( true, _RED );
		iErrorCode = Error_Installation_SolarContent;
	}
	else if( pclSolarContent->GetCurrentValSI() >= pclEditSystemVolume->GetCurrentValSI() )
	{
		CString strToolTip;

		CString strTemp = CString( WriteCUDouble( _U_VOLUME, pclSolarContent->GetCurrentValSI(), true, -1, 2 ) ) + _T(" >= ") 
				+ CString( WriteCUDouble( _U_VOLUME, pclEditSystemVolume->GetCurrentValSI(), true, -1, 2 ) );

		FormatString( strToolTip, IDS_DLGINDSELPMPANELS_TTERRORSOLARCONTENTTOHIGH, strTemp );

		m_ToolTip.AddToolWindow( pclSolarContent, strToolTip );
		pclSolarContent->SetDrawBorder( true, _RED );

		strTemp = CString( WriteCUDouble( _U_VOLUME, pclEditSystemVolume->GetCurrentValSI(), true, -1, 2 ) ) + _T(" <= ") 
				+ CString( WriteCUDouble( _U_VOLUME, pclSolarContent->GetCurrentValSI(), true, -1, 2 ) );

		FormatString( strToolTip, IDS_DLGINDSELPMPANELS_TTERRORSYSTEMVOLUME, strTemp );

		m_ToolTip.AddToolWindow( pclEditSystemVolume, strToolTip );
		pclEditSystemVolume->SetDrawBorder( true, _RED );
		iErrorCode = Error_Installation_SystemVolume | Error_Installation_SolarContent;
	}

	return iErrorCode;
}

int CDlgWizardPM_RightViewInput_Base::IsSolarSolarContentMultiplierFactorOK( CExtNumEdit *pclEditSolarContentMultiplierFactor )
{
	if( NULL == pclEditSolarContentMultiplierFactor )
	{
		return 0;
	}

	ResetToolTipAndErrorBorder( pclEditSolarContentMultiplierFactor );

	// After reset reload default values.
	InitToolTips( pclEditSolarContentMultiplierFactor );

	int iErrorCode = 0;

	if( pclEditSolarContentMultiplierFactor->GetCurrentValSI() < 1.1 || pclEditSolarContentMultiplierFactor->GetCurrentValSI() > 2.0 )
	{
		CString str;
		FormatString( str, IDS_SOLARCONTENTMULTIPLIERFACTOR_EDITLIMIT, _T("1.1"), _T("2") );
		m_ToolTip.AddToolWindow( pclEditSolarContentMultiplierFactor, str );
		pclEditSolarContentMultiplierFactor->SetDrawBorder( true, _RED );
		iErrorCode = Error_Installation_SolarContentFactor;
	}

	return iErrorCode;
}

int CDlgWizardPM_RightViewInput_Base::IsStorageTankMaxTempOK( CExtNumEdit *pclEditStorageTankMaxTemp )
{
	if( NULL == pclEditStorageTankMaxTemp )
	{
		return 0;
	}

	// By default reset error.
	pclEditStorageTankMaxTemp->ResetDrawBorder();
	ResetToolTipAndErrorBorder( pclEditStorageTankMaxTemp );

	// After reset reload default values.
	InitToolTips( NULL );

	int iErrorCode = 0;

	if( pclEditStorageTankMaxTemp->GetCurrentValSI() < m_pclWizardSelPMParams->m_pclSelectPMList->GetpclPMInputUser()->GetReturnTemperature() )
	{
		CString strTooltip;
		FormatString( strTooltip, IDS_DLGSTORAGETANKMAXTEMPERATURE_MAXTEMPCANTBELOWERTHANRETURNTEMP, WriteCUDouble( _U_TEMPERATURE, m_pclWizardSelPMParams->m_pclSelectPMList->GetpclPMInputUser()->GetReturnTemperature(), true ) );
		m_ToolTip.AddToolWindow( pclEditStorageTankMaxTemp, strTooltip );
		pclEditStorageTankMaxTemp->SetDrawBorder( true, _RED );
		iErrorCode = Error_Installation_StorageTankMaxTemp;
	}

	return iErrorCode;
}

int CDlgWizardPM_RightViewInput_Base::IsInstalledPowerOK( CExtNumEdit *pclInstalledPower )
{
	if( NULL == pclInstalledPower )
	{
		return 0;
	}

	// By default reset error.
	pclInstalledPower->ResetDrawBorder();
	ResetToolTipAndErrorBorder( pclInstalledPower );

	// After reset reload default values.
	InitToolTips( NULL );

	int iErrorCode = 0;

	if( true == m_pclWizardSelPMParams->m_pclSelectPMList->GetpclPMInputUser()->IsNorm( ProjectType::All, PressurisationNorm::PN_SWKIHE301_01 )
			&& pclInstalledPower->GetCurrentValSI() <= 0.0 )
	{
		m_ToolTip.AddToolWindow( pclInstalledPower, TASApp.LoadLocalizedString( IDS_DLGINDSELPMPANELS_TTERRORPOWER ) );
		pclInstalledPower->SetDrawBorder( true, _RED );
		iErrorCode = Error_Installation_InstalledPower;
	}

	return iErrorCode;
}

void CDlgWizardPM_RightViewInput_Base::UpdateOtherCoolingTemperatures( int iErrorCode, CExtNumEdit *pclMaxEdit, CExtNumEdit *pclSupplyEdit, CExtNumEdit *pclReturnEdit, 
		CExtNumEdit *pclFillEdit, CExtNumEdit *pclMinEdit, INT iCheckMinTemp )
{
	if( NULL == pclMaxEdit || NULL == pclSupplyEdit || NULL == pclReturnEdit || NULL == pclFillEdit )
	{
		ASSERT_RETURN;
	}


	if( NULL == m_pclWizardSelPMParams || NULL == m_pclWizardSelPMParams->m_pclSelectPMList || NULL == m_pclWizardSelPMParams->m_pclSelectPMList->GetpclPMInputUser() 
			|| NULL == m_pclWizardSelPMParams->m_pTADS || NULL == m_pclWizardSelPMParams->m_pTADS->GetpTechParams() )
	{
		ASSERT_RETURN;
	}

	// Remark: we update the 'CPMInputUser' class in all cases. But for technical parameters it is only if
	// there are no more error.
	CPMInputUser *pclPMInputUser = m_pclWizardSelPMParams->m_pclSelectPMList->GetpclPMInputUser();
	CDS_TechnicalParameter *pTechParam = m_pclWizardSelPMParams->m_pTADS->GetpTechParams();
	bool bUpdateRibbon = false;

	// HYS-1350: Do not save max. temperature if error.
	if( Error_Temp_Max != ( Error_Temp_Max & iErrorCode ) )
	{
		pclPMInputUser->SetMaxTemperature( pclMaxEdit->GetCurrentValSI() );
	}

	// HYS-1350: Do not save supply temperature if error.
	if( Error_Temp_Supply != ( Error_Temp_Supply & iErrorCode ) )
	{
		pclPMInputUser->SetSupplyTemperature( pclSupplyEdit->GetCurrentValSI() );

		m_pclWizardSelPMParams->m_WC.SetTemp( pclSupplyEdit->GetCurrentValSI() );
		m_pclWizardSelPMParams->m_WC.UpdateFluidData( m_pclWizardSelPMParams->m_WC.GetTemp() );
		*m_pclWizardSelPMParams->m_pclSelectPMList->GetpclPMInputUser()->GetpWC() = m_pclWizardSelPMParams->m_WC;

		// Copy also in the current water char object from the 'CTADataStruct' database. Because 'CMainFrame::OnWaterChange' will be called because
		// the 'WM_USER_WATERCHANGE' message sent just after. And this method will update the ribbon bar not with values from the technical 
		// parameters but well with the current water char object.
		*( TASApp.GetpTADS()->GetpWCForProductSelection()->GetpWCData() ) = m_pclWizardSelPMParams->m_WC;

		pTechParam->SetDefaultISTps( pclSupplyEdit->GetCurrentValSI() );
		bUpdateRibbon = true;
	}

	// HYS-1350: Do not save return temperature if error.
	if( Error_Temp_Return != ( Error_Temp_Return & iErrorCode ) )
	{
		pclPMInputUser->SetReturnTemperature( pclReturnEdit->GetCurrentValSI() );
	}

	if( Error_Temp_Supply != ( Error_Temp_Supply & iErrorCode ) && Error_Temp_Return != ( Error_Temp_Return & iErrorCode ) )
	{
		pTechParam->SetDefaultISDT( pclReturnEdit->GetCurrentValSI() - pclSupplyEdit->GetCurrentValSI() );
		bUpdateRibbon = true;
	}

	// HYS-1350: Do not save min. temperature when error.
	if( NULL != pclMinEdit && Error_Temp_Min != ( Error_Temp_Min & iErrorCode ) )
	{
		// HYS-1410: It's well 'pclMinEdit' and not 'pclFillEdit'.
		pclPMInputUser->SetMinTemperature( pclMinEdit->GetCurrentValSI() );
	}

	// HYS-1350: Do not save fill temperature when error.
	if( Error_Temp_Fill != ( Error_Temp_Fill & iErrorCode ) )
	{
		// HYS-1410: It's well 'pclFillEdit' and not 'pclMinEdit'.
		pclPMInputUser->SetFillTemperature( pclFillEdit->GetCurrentValSI() );
	}

	if( true == bUpdateRibbon )
	{
		// Message is sent to the mainframe to allow it to change the ribbon value.
		::AfxGetApp()->m_pMainWnd->SendMessage( WM_USER_WATERCHANGE, ( WPARAM )WMUserWaterCharWParam::WM_UWC_WP_ForProductSel );
	}
}

int CDlgWizardPM_RightViewInput_Base::IsHeatingSolarSafetyTempLimiterOK( CExtNumEdit *pclSftyEdit, CExtNumEditComboBox *pComboNorm, bool bShowErrorMsg )
{
	if( NULL == m_pclWizardSelPMParams || NULL == m_pclWizardSelPMParams->m_pTADS || NULL == m_pclWizardSelPMParams->m_pTADS->GetpTechParams() 
			|| NULL == m_pclWizardSelPMParams->m_pclSelectPMList || NULL == m_pclWizardSelPMParams->m_pclSelectPMList->GetpclPMInputUser() )
	{
		ASSERTA_RETURN( 0 );
	}

	if( NULL == pclSftyEdit )
	{
		return 0;
	}

	// By default reset error.
	pclSftyEdit->ResetDrawBorder();
	ResetToolTipAndErrorBorder( pclSftyEdit );

	// After reset reload default values.
	InitToolTips( NULL );

	CString strSfty = TASApp.LoadLocalizedString( IDS_DLGINDSELPMPANELS_TTERRORSAFETYTEMP );
	CString strSftyLowerCase = strSfty;
	strSftyLowerCase.MakeLower();
	
	CString strSupply = TASApp.LoadLocalizedString( IDS_DLGINDSELPMPANELS_TTERRORSUPPLYTEMP );
	CString strSupplyLowerCase = strSupply;
	strSupplyLowerCase.MakeLower();

	int iError = 0;

	// We check first if the safety temperature limiter is valid.
	bool bSftyBadFluidError = false;

	if( false == IsFluidTemperatureOK( pclSftyEdit, strSfty ) )
	{
		iError |= Error_Temp_SafetyTempLimiter;
		bSftyBadFluidError = true;
	}

	// Check now all temperatures.
	double dSafetyTempLimiter = pclSftyEdit->GetCurrentValSI();
	double dSupplyTemperature = m_pclWizardSelPMParams->m_pclSelectPMList->GetpclPMInputUser()->GetSupplyTemperature();
	
	CString strTTSfty( _T("") );

	// In first step, check temperatures that must be not to high for EN12828 norm.
	if( NULL != pComboNorm && _T("PM_NORM_EN12828" ) == CString( GetNormID( pComboNorm ) ) )
	{
		double dTAZLimit = m_pclWizardSelPMParams->m_pTADS->GetpTechParams()->GetSafetyTempLimiterLimitEN12953();

		if( dSafetyTempLimiter > dTAZLimit )
		{
			if( true == bShowErrorMsg )
			{
				CString strTemp;
				CString strSafetyTempLimiter = TASApp.LoadLocalizedString( IDS_DLGINDSELPMPANELS_TTERRORSAFETYTEMP );
				CString strSafetyTempLimiterLimit = WriteCUDouble( _U_TEMPERATURE, dSafetyTempLimiter, true );
				FormatString( strTemp, IDS_DLGINDSELPMPANELS_TTERROREN12953, strSafetyTempLimiter, strSafetyTempLimiterLimit );
				strTemp += _T("\r\n\r\n") + TASApp.LoadLocalizedString( IDS_DLGINDSELPMPANELS_SWITCHEN12953 );
			
				if( IDYES == MessageBox( strTemp, _T(""), MB_YESNO | MB_ICONEXCLAMATION ) )
				{
					SetNormID( pComboNorm, _T("PM_NORM_EN12953") );
					m_pclWizardSelPMParams->m_pclSelectPMList->GetpclPMInputUser()->SetNormID( _T("PM_NORM_EN12953") );
				}
			}
			else
			{
				// If we already have an error message on the safety temperature limiter checked above when calling the
				// 'IsFluidTemperatureOK' method, we don't add a new tooltip.
				if( false == bSftyBadFluidError )
				{
					CString strTemp;
					CString strTAZLimit = WriteCUDouble( _U_TEMPERATURE, dTAZLimit, true );
					FormatString( strTemp, IDS_DLGINDSELPMPANELS_TTERROREN12953, strSfty, strTAZLimit );
					strTTSfty += strTemp;
				}

				// In all cases we add the error.
				iError |= Error_Temp_SafetyTempLimiter;
			}
		}
	}

	if( dSafetyTempLimiter < dSupplyTemperature )
	{
		// If we already have an error message on the safety temperature limiter checked above when calling the
		// 'IsFluidTemperatureOK' method, we don't add a new tooltip.
		if( false == bSftyBadFluidError )
		{
			// If already filled above with verifications done just before, we add a new line for this new error message.
			if( false == strTTSfty.IsEmpty() )
			{
				strTTSfty += _T("\r\n");
			}

			FormatString( strTTSfty, IDS_DLGINDSELPMPANELS_TTERRORTEMPLOWER, strSfty, strSupplyLowerCase );
		}

		// In all cases we add the error.
		iError |= ( Error_Temp_SafetyTempLimiter | Error_Temp_Supply );
	}

	if( false == strTTSfty.IsEmpty() )
	{
		ResetToolTipAndErrorBorder( pclSftyEdit );
		m_ToolTip.AddToolWindow( pclSftyEdit, strTTSfty );
		pclSftyEdit->SetDrawBorder( true, _RED );
	}

	return iError;
}

int CDlgWizardPM_RightViewInput_Base::AreHeatingSolarTemperaturesOK( CExtNumEdit *pclSftyEdit, CExtNumEdit *pclSupplyEdit,
		CExtNumEdit *pclReturnEdit, CExtNumEdit *pclMinEdit, CExtNumEdit *pclFillEdit, CExtNumEditComboBox *pComboNorm, bool bShowErrorMsg )
{
	if( NULL == pclSftyEdit || NULL == pclSupplyEdit || NULL == pclReturnEdit || NULL == pclMinEdit || NULL == pclFillEdit 
			|| NULL == m_pclWizardSelPMParams || NULL == m_pclWizardSelPMParams->m_pTADS || NULL == m_pclWizardSelPMParams->m_pTADS->GetpTechParams()
			|| NULL == m_pclWizardSelPMParams->m_pclSelectPMList || NULL == m_pclWizardSelPMParams->m_pclSelectPMList->GetpclPMInputUser() )
	{
		ASSERTA_RETURN( 0 );
	}

	// By default reset all errors.
	pclSftyEdit->ResetDrawBorder();
	pclSupplyEdit->ResetDrawBorder();
	pclReturnEdit->ResetDrawBorder();
	pclMinEdit->ResetDrawBorder();
	// HYS-1054: Fill temperature is now also available in heating and solar system
	pclFillEdit->ResetDrawBorder();

	ResetToolTipAndErrorBorder( pclSftyEdit );
	ResetToolTipAndErrorBorder( pclSupplyEdit );
	ResetToolTipAndErrorBorder( pclReturnEdit );
	ResetToolTipAndErrorBorder( pclMinEdit );
	ResetToolTipAndErrorBorder( pclFillEdit );

	// After reset reload default values.
	InitToolTips( NULL );

	CString strSfty = TASApp.LoadLocalizedString( IDS_DLGINDSELPMPANELS_TTERRORSAFETYTEMP );
	CString strSupply = TASApp.LoadLocalizedString( IDS_DLGINDSELPMPANELS_TTERRORSUPPLYTEMP );
	CString strReturn = TASApp.LoadLocalizedString( IDS_DLGINDSELPMPANELS_TTERRORRETURNTEMP );
	CString strMin = TASApp.LoadLocalizedString( IDS_DLGINDSELPMPANELS_TTERRORMINTEMP );
	CString strFill = TASApp.LoadLocalizedString( IDS_DLGINDSELPMPANELS_TTERRORFILLTEMP );
	
	CString strSftyLowerCase = strSfty;
	strSftyLowerCase.MakeLower();
	
	CString strSupplyLowerCase = strSupply;
	strSupplyLowerCase.MakeLower();
	
	CString strReturnLowerCase = strReturn;
	strReturnLowerCase.MakeLower();
	
	CString strMinLowerCase = strMin;
	strMinLowerCase.MakeLower();

	CString strFillLowerCase = strFill;
	strFillLowerCase.MakeLower();

	int iError = 0;

	// Before check temperatures between each others, we check first if temperature are valid.
	bool bSftyBadFluidError = false;

	if( false == IsFluidTemperatureOK( pclSftyEdit, strSfty ) )
	{
		iError |= Error_Temp_SafetyTempLimiter;
		bSftyBadFluidError = true;
	}

	bool bSupplyBadfluidError = false;

	if( false == IsFluidTemperatureOK( pclSupplyEdit, strSupply ) )
	{
		iError |= Error_Temp_Supply;
		bSupplyBadfluidError = true;
	}

	bool bReturnBadFluidError = false;

	if( false == IsFluidTemperatureOK( pclReturnEdit, strReturn ) )
	{
		iError |= Error_Temp_Return;
		bReturnBadFluidError = true;
	}

	bool bMinBadFluidError = false;

	if( false == IsFluidTemperatureOK( pclMinEdit, strMin ) )
	{
		iError |= Error_Temp_Min;
		bMinBadFluidError = true;
	}

	bool bFillBadFluidError = false;

	if( false == IsFluidTemperatureOK( pclFillEdit, strMin ) )
	{
		iError |= Error_Temp_Fill;
		bFillBadFluidError = true;
	}

	// Check now all temperatures.
	double dSafetyTempLimiter = pclSftyEdit->GetCurrentValSI();
	double dSupplyTemperature = pclSupplyEdit->GetCurrentValSI();
	double dReturnTemperature = pclReturnEdit->GetCurrentValSI();
	double dMinTemperature = pclMinEdit->GetCurrentValSI();
	double dFillTemperature = pclFillEdit->GetCurrentValSI();
	double dWaterMakeUpTemperature = m_pclWizardSelPMParams->m_pclSelectPMList->GetpclPMInputUser()->GetWaterMakeUpWaterTemp();

	CString strTTSfty( _T("") );
	CString strTTSupply( _T("") );
	CString strTTReturn( _T("") );
	CString strTTMin( _T("") );
	CString strTTFill( _T("") );

	// In first step, check temperatures that must be not to high for EN12828 norm.
	if( NULL != pComboNorm && _T("PM_NORM_EN12828" ) == CString( GetNormID( pComboNorm ) ) )
	{
		double dTAZLimit = m_pclWizardSelPMParams->m_pTADS->GetpTechParams()->GetSafetyTempLimiterLimitEN12953();
		double dSupplyTempLimit = m_pclWizardSelPMParams->m_pTADS->GetpTechParams()->GetFluidTempLimitEN12953();
		double dReturnTempLimit = dSupplyTempLimit;
		bool bMsgAlreadyDisplayed = false;

		if( dSafetyTempLimiter > dTAZLimit )
		{
			// If we can display the error message...
			if( true == bShowErrorMsg )
			{
				CString strTemp;
				CString strSafetyTempLimiter = TASApp.LoadLocalizedString( IDS_DLGINDSELPMPANELS_TTERRORSAFETYTEMP );
				CString strSafetyTempLimiterLimit = WriteCUDouble( _U_TEMPERATURE, dSafetyTempLimiter, true );
				FormatString( strTemp, IDS_DLGINDSELPMPANELS_TTERROREN12953, strSafetyTempLimiter, strSafetyTempLimiterLimit );
				strTemp += _T("\r\n\r\n") + TASApp.LoadLocalizedString( IDS_DLGINDSELPMPANELS_SWITCHEN12953 );
				bMsgAlreadyDisplayed = true;
			
				if( IDYES == MessageBox( strTemp, _T(""), MB_YESNO | MB_ICONEXCLAMATION ) )
				{
					SetNormID( pComboNorm, _T("PM_NORM_EN12953") );
					m_pclWizardSelPMParams->m_pclSelectPMList->GetpclPMInputUser()->SetNormID( _T("PM_NORM_EN12953") );
				}
			}
			
			// If user has not changed the norm (Or the message can't be displayed)....
			if( _T("PM_NORM_EN12828" ) == CString( GetNormID( pComboNorm ) ) )
			{
				// If we already have an error message on the safety temperature limiter checked above when calling the
				// 'IsFluidTemperatureOK' method, we don't add a new tooltip.
				if( false == bSftyBadFluidError )
				{
					CString strTemp;
					CString strTAZLimit = WriteCUDouble( _U_TEMPERATURE, dTAZLimit, true );
					FormatString( strTemp, IDS_DLGINDSELPMPANELS_TTERROREN12953, strSfty, strTAZLimit );
					strTTSfty = strTemp;
				}

				// In all cases we add the error.
				iError |= Error_Temp_SafetyTempLimiter;
			}
		}

		// We recheck the norm because user can have changed it just above (By accepting to switch to the EN 12953 norm).
		if( _T("PM_NORM_EN12828" ) == CString( GetNormID( pComboNorm ) ) && dSupplyTemperature > dSupplyTempLimit )
		{
			// If message has not yet been displayed and we can display the error message...
			if( false == bMsgAlreadyDisplayed && true == bShowErrorMsg )
			{
				CString strTemp;
				CString strSupply = TASApp.LoadLocalizedString( IDS_DLGINDSELPMPANELS_TTERRORSUPPLYTEMP );
				CString strSupplyTempLimit = WriteCUDouble( _U_TEMPERATURE, dSupplyTempLimit, true );
				FormatString( strTemp, IDS_DLGINDSELPMPANELS_TTERROREN12953, strSupply, strSupplyTempLimit );
				strTemp += _T("\r\n\r\n") + TASApp.LoadLocalizedString( IDS_DLGINDSELPMPANELS_SWITCHEN12953 );
				bMsgAlreadyDisplayed = true;
			
				if( IDYES == MessageBox( strTemp, _T(""), MB_YESNO | MB_ICONEXCLAMATION ) )
				{
					SetNormID( pComboNorm, _T("PM_NORM_EN12953") );
					m_pclWizardSelPMParams->m_pclSelectPMList->GetpclPMInputUser()->SetNormID( _T("PM_NORM_EN12953") );
				}
			}

			// If user has not change the norm (Or the message can't be displayed), we are yet in error with the supply temperature.
			if( _T("PM_NORM_EN12828" ) == CString( GetNormID( pComboNorm ) ) )
			{
				// If we already have an error message on the supply temperature checked above when calling the
				// 'IsFluidTemperatureOK' method, we don't add a new tooltip.
				if( false == bSupplyBadfluidError )
				{
					CString strTemp;
					CString strSupplyTempLimit = WriteCUDouble( _U_TEMPERATURE, dSupplyTempLimit, true );
					FormatString( strTemp, IDS_DLGINDSELPMPANELS_TTERROREN12953, strSupply, strSupplyTempLimit );
					strTTSupply = strTemp;
				}

				// In all cases we add the error.
				iError |= Error_Temp_Supply;
			}
		}

		// We recheck the norm because user can have changed it just above (By accepting to switch to the EN 12953 norm).
		if( _T("PM_NORM_EN12828" ) == CString( GetNormID( pComboNorm ) ) && dReturnTemperature > dReturnTempLimit )
		{
			// If message has not yet been displayed and we can display the error message...
			if( false == bMsgAlreadyDisplayed && true == bShowErrorMsg )
			{
				CString strTemp;
				CString strReturn = TASApp.LoadLocalizedString( IDS_DLGINDSELPMPANELS_TTERRORRETURNTEMP );
				CString strReturnTempLimit = WriteCUDouble( _U_TEMPERATURE, dReturnTempLimit, true );
				FormatString( strTemp, IDS_DLGINDSELPMPANELS_TTERROREN12953, strReturn, strReturnTempLimit );
				strTemp += _T("\r\n\r\n") + TASApp.LoadLocalizedString( IDS_DLGINDSELPMPANELS_SWITCHEN12953 );
				bMsgAlreadyDisplayed =true;
			
				if( IDYES == MessageBox( strTemp, _T(""), MB_YESNO | MB_ICONEXCLAMATION ) )
				{
					SetNormID( pComboNorm, _T("PM_NORM_EN12953") );
					m_pclWizardSelPMParams->m_pclSelectPMList->GetpclPMInputUser()->SetNormID( _T("PM_NORM_EN12953") );
				}
			}

			// If user has not change the norm (Or the message can't be displayed), we are yet in error with the return temperature.
			if( _T("PM_NORM_EN12828" ) == CString( GetNormID( pComboNorm ) ) )
			{
				// If we already have an error message on the return temperature checked above when calling the
				// 'IsFluidTemperatureOK' method, we don't add a new tooltip.
				if( false == bReturnBadFluidError )
				{
					CString strTemp;
					CString strReturnTempLimit = WriteCUDouble( _U_TEMPERATURE, dReturnTempLimit, true );
					FormatString( strTemp, IDS_DLGINDSELPMPANELS_TTERROREN12953, strReturn, strReturnTempLimit );
					strTTReturn = strTemp;
				}

				// In all cases we add the error.
				iError |= Error_Temp_Return;
			}
		}
	}

	// In second step check the supply temperature in regards to the water make-up temperature.
	if( true == m_pclWizardSelPMParams->m_pclSelectPMList->GetpclPMInputUser()->IsWaterMakeUpExist()
			&& dSupplyTemperature < dWaterMakeUpTemperature )
	{
		// In this case we display a AFX message because water make-up temperature is in the 'Pressure maintenance type'
		// step.
		
		// A water make-up device is required and the water make-up temperature (%1) is above the supply temperature (%2).
		// Do you want to set the water make-up temperature to the same value of the supply temperature?
		CString strWaterMakeUpTemp = WriteCUDouble( _U_TEMPERATURE, dWaterMakeUpTemperature, true );
		CString strSupplyTemp = WriteCUDouble( _U_TEMPERATURE, dSupplyTemperature, true );
		CString strMessage;
		FormatString( strMessage, AFXMSG_PMWIZHEATSOLAR_SUPPLYANDWMUPTEMPINVALID, strWaterMakeUpTemp, strSupplyTemp );

		if( IDYES == AfxMessageBox( strMessage, MB_YESNO | MB_ICONSTOP ) )
		{
			m_pclWizardSelPMParams->m_pclSelectPMList->GetpclPMInputUser()->SetWaterMakeUpWaterTemp( dSupplyTemperature );
		}
		else
		{
			pclSupplyEdit->SetCurrentValSI( m_dSupplyTempSaved );
			pclSupplyEdit->Update();
			dSupplyTemperature = m_dSupplyTempSaved;
		}
	}
	
	if( dSafetyTempLimiter < dSupplyTemperature )
	{
		// If we already have an error message on the safety temperature limiter checked above when calling the
		// 'IsFluidTemperatureOK' method, we don't add a new tooltip.
		if( false == bSftyBadFluidError )
		{
			// If already filled above with verifications done just before, we add a new line for this new error message.
			if( false == strTTSfty.IsEmpty() )
			{
				strTTSfty += _T("\r\n");
			}

			FormatString( strTTSfty, IDS_DLGINDSELPMPANELS_TTERRORTEMPLOWER, strSfty, strSupplyLowerCase );
		}

		// If we already have an error message on the supply temperature checked above when calling the
		// 'IsFluidTemperatureOK' method, we don't add a new tooltip.
		if( false == bSupplyBadfluidError )
		{
			// If already filled above with verifications done just before, we add a new line for this new error message.
			if( false == strTTSupply.IsEmpty() )
			{
				strTTSupply += _T("\r\n");
			}

			FormatString( strTTSupply, IDS_DLGINDSELPMPANELS_TTERRORTEMPGREATER, strSupply, strSftyLowerCase );
		}

		// In all cases we add the error.
		iError |= ( Error_Temp_SafetyTempLimiter | Error_Temp_Supply );
	}

	if( dSupplyTemperature < dReturnTemperature )
	{
		// If we already have an error message on the supply temperature checked above when calling the
		// 'IsFluidTemperatureOK' method, we don't add a new tooltip.
		if( false == bSupplyBadfluidError )
		{
			// If already filled above with verifications done just before, we add a new line for this new error message.
			if( false == strTTSupply.IsEmpty() )
			{
				strTTSupply += _T("\r\n");
			}

			CString strTemp;
			FormatString( strTemp, IDS_DLGINDSELPMPANELS_TTERRORTEMPLOWER, strSupply, strReturnLowerCase );
			strTTSupply += strTemp;
		}

		// If we already have an error message on the return temperature checked above when calling the
		// 'IsFluidTemperatureOK' method, we don't add a new tooltip.
		if( false == bReturnBadFluidError )
		{
			// If already filled above with verifications done just before, we add a new line for this new error message.
			if( false == strTTReturn.IsEmpty() )
			{
				strTTReturn += _T("\r\n");
			}

			FormatString( strTTReturn, IDS_DLGINDSELPMPANELS_TTERRORTEMPGREATER, strReturn, strSupplyLowerCase );
		}

		// In all cases we add the error.
		iError |= ( Error_Temp_Supply | Error_Temp_Return );
	}

	if( dReturnTemperature < dMinTemperature )
	{
		// If we already have an error message on the return temperature checked above when calling the
		// 'IsFluidTemperatureOK' method, we don't add a new tooltip.
		if( false == bReturnBadFluidError )
		{
			// If already filled above with verifications done just before, we add a new line for this new error message.
			if( false == strTTReturn.IsEmpty() )
			{
				strTTReturn += _T("\r\n");
			}

			CString strTemp;
			FormatString( strTemp, IDS_DLGINDSELPMPANELS_TTERRORTEMPLOWER, strReturn, strMinLowerCase );
			strTTReturn += strTemp;
		}

		// If we already have an error message on the minimum temperature checked above when calling the
		// 'IsFluidTemperatureOK' method, we don't add a new tooltip.
		if( false == bMinBadFluidError )
		{
			// If already filled above with verifications done just before, we add a new line for this new error message.
			if( false == strTTMin.IsEmpty() )
			{
				strTTMin += _T("\r\n");
			}

			FormatString( strTTMin, IDS_DLGINDSELPMPANELS_TTERRORTEMPGREATER, strMin, strReturnLowerCase );
		}

		// In all cases we add the error.
		iError |= ( Error_Temp_Return | Error_Temp_Min );
	}

	if( dFillTemperature > dReturnTemperature )
	{
		// If we already have an error message on the fill temperature checked above when calling the
		// 'IsFluidTemperatureOK' method, we don't add a new tooltip.
		if( false == bFillBadFluidError )
		{
			// If already filled above with verifications done just before, we add a new line for this new error message.
			if( false == strTTFill.IsEmpty() )
			{
				strTTFill += _T("\n");
			}

			FormatString( strTTFill, IDS_DLGINDSELPMPANELS_TTERRORTEMPGREATER, strFill, strReturnLowerCase );
		}

		// If we already have an error message on the return temperature checked above when calling the
		// 'IsFluidTemperatureOK' method, we don't add a new tooltip.
		if( false == bReturnBadFluidError )
		{
			// If already filled above with verifications done just before, we add a new line for this new error message.
			if( false == strTTReturn.IsEmpty() )
			{
				strTTReturn += _T("\n");
			}

			CString strTemp;
			FormatString( strTemp, IDS_DLGINDSELPMPANELS_TTERRORTEMPLOWER, strReturn, strFillLowerCase );
			strTTReturn += strTemp;
		}

		// In all cases we add the error.
		iError |= ( Error_Temp_Return | Error_Temp_Fill );
	}

	if( dFillTemperature < dMinTemperature )
	{
		// If we already have an error message on the fill temperature checked above when calling the
		// 'IsFluidTemperatureOK' method, we don't add a new tooltip.
		if( false == bFillBadFluidError )
		{
			// If already filled above with verifications done just before, we add a new line for this new error message.
			if( false == strTTFill.IsEmpty() )
			{
				strTTFill += _T("\n");
			}

			FormatString( strTTFill, IDS_DLGINDSELPMPANELS_TTERRORTEMPLOWER, strFill, strMinLowerCase );
		}

		// If we already have an error message on the minimum temperature checked above when calling the
		// 'IsFluidTemperatureOK' method, we don't add a new tooltip.
		if( false == bMinBadFluidError )
		{
			// If already filled above with verifications done just before, we add a new line for this new error message.
			if( false == strTTMin.IsEmpty() )
			{
				strTTMin += _T("\n");
			}

			CString strTemp;
			FormatString( strTemp, IDS_DLGINDSELPMPANELS_TTERRORTEMPGREATER, strMin, strFillLowerCase );
			strTTMin += strTemp;
		}

		// In all cases we add the error.
		iError |= ( Error_Temp_Fill | Error_Temp_Min );
	}

	if( false == strTTSfty.IsEmpty() )
	{
		ResetToolTipAndErrorBorder( pclSftyEdit );
		m_ToolTip.AddToolWindow( pclSftyEdit, strTTSfty );
		pclSftyEdit->SetDrawBorder( true, _RED );
	}

	if( false == strTTSupply.IsEmpty() )
	{
		ResetToolTipAndErrorBorder( pclSupplyEdit );
		m_ToolTip.AddToolWindow( pclSupplyEdit, strTTSupply );
		pclSupplyEdit->SetDrawBorder( true, _RED );
	}

	if( false == strTTReturn.IsEmpty() )
	{
		ResetToolTipAndErrorBorder( pclReturnEdit );
		m_ToolTip.AddToolWindow( pclReturnEdit, strTTReturn );
		pclReturnEdit->SetDrawBorder( true, _RED );
	}

	if( false == strTTMin.IsEmpty() )
	{
		ResetToolTipAndErrorBorder( pclMinEdit );
		m_ToolTip.AddToolWindow( pclMinEdit, strTTMin );
		pclMinEdit->SetDrawBorder( true, _RED );
	}

	if( false == strTTFill.IsEmpty() )
	{
		ResetToolTipAndErrorBorder( pclFillEdit );
		m_ToolTip.AddToolWindow( pclFillEdit, strTTFill );
		pclFillEdit->SetDrawBorder( true, _RED );
	}

	return iError;
}

void CDlgWizardPM_RightViewInput_Base::UpdateOtherHeatingSolarTemperatures( int iErrorCode, CExtNumEdit *pclSftyEdit, CExtNumEdit *pclSupplyEdit, 
		CExtNumEdit *pclReturnEdit, CExtNumEdit *pclMinEdit, CExtNumEdit *pclFillEdit )
{
	if( NULL == pclSftyEdit )
	{
		ASSERT_RETURN;
	}

	if( NULL == m_pclWizardSelPMParams || NULL == m_pclWizardSelPMParams->m_pclSelectPMList || NULL == m_pclWizardSelPMParams->m_pclSelectPMList->GetpclPMInputUser() 
			|| NULL == m_pclWizardSelPMParams->m_pTADS || NULL == m_pclWizardSelPMParams->m_pTADS->GetpTechParams() )
	{
		ASSERT_RETURN;
	}

	// Remark: we update the 'CPMInputUser' class in all cases. But for technical parameters it is only if
	// there are no more error.
	CPMInputUser *pclPMInputUser = m_pclWizardSelPMParams->m_pclSelectPMList->GetpclPMInputUser();
	CDS_TechnicalParameter *pTechParam = m_pclWizardSelPMParams->m_pTADS->GetpTechParams();

	bool bUpdateRibbon = false;

	// HYS-1350: Do not save safety temperature limiter when error.
	if( Error_Temp_SafetyTempLimiter != ( Error_Temp_SafetyTempLimiter & iErrorCode ) )
	{
		pclPMInputUser->SetSafetyTempLimiter( pclSftyEdit->GetCurrentValSI() );
		pTechParam->SetDefaultTAZ( pclSftyEdit->GetCurrentValSI() );
	}

	if( NULL != pclSupplyEdit )
	{
		// HYS-1350: Do not save safety temperature limiter when error.
		if( Error_Temp_Supply != ( Error_Temp_Supply & iErrorCode ) )
		{
			m_pclWizardSelPMParams->m_WC.SetTemp( pclSupplyEdit->GetCurrentValSI() );
			m_pclWizardSelPMParams->m_WC.UpdateFluidData( m_pclWizardSelPMParams->m_WC.GetTemp() );
			*m_pclWizardSelPMParams->m_pclSelectPMList->GetpclPMInputUser()->GetpWC() = m_pclWizardSelPMParams->m_WC;

			// Copy also in the current water char object from the 'CTADataStruct' database. Because 'CMainFrame::OnWaterChange' will be called because
			// the 'WM_USER_WATERCHANGE' message sent just after. And this method will update the ribbon bar not with values from the technical 
			// parameters but well with the current water char object.
			*( TASApp.GetpTADS()->GetpWCForProductSelection()->GetpWCData() ) = m_pclWizardSelPMParams->m_WC;

			pTechParam->SetDefaultISTps( pclSupplyEdit->GetCurrentValSI() );
			bUpdateRibbon = true;
		}
	}

	if( NULL != pclReturnEdit )
	{
		// HYS-1350: Do not save return temperature when error.
		if( Error_Temp_Return != ( Error_Temp_Return & iErrorCode ) )
		{
			pclPMInputUser->SetReturnTemperature( pclReturnEdit->GetCurrentValSI() );
		}
	}

	if( NULL != pclSupplyEdit && Error_Temp_Supply != ( Error_Temp_Supply & iErrorCode ) 
			&& NULL != pclReturnEdit && Error_Temp_Return != ( Error_Temp_Return & iErrorCode ) )
	{
		pTechParam->SetDefaultISDT( pclSupplyEdit->GetCurrentValSI() - pclReturnEdit->GetCurrentValSI() );
		bUpdateRibbon = true;
	}

	if( NULL != pclMinEdit )
	{
		// HYS-1350: Do not save min. temperature when error.
		if( Error_Temp_Min != ( Error_Temp_Min & iErrorCode ) )
		{
			pclPMInputUser->SetMinTemperature( pclMinEdit->GetCurrentValSI() );
		}
	}

	if( NULL != pclFillEdit )
	{
		// HYS-1350: Do not save fill temperature when error.
		if( Error_Temp_Fill != ( Error_Temp_Fill & iErrorCode ) )
		{
			pclPMInputUser->SetFillTemperature( pclFillEdit->GetCurrentValSI() );
		}
	}

	if( true == bUpdateRibbon )
	{
		// Message is sent to the mainframe to allow it to change the ribbon value.
		::AfxGetApp()->m_pMainWnd->SendMessage( WM_USER_WATERCHANGE, ( WPARAM )WMUserWaterCharWParam::WM_UWC_WP_ForProductSel );
	}
}

int CDlgWizardPM_RightViewInput_Base::AreCoolingMaxSupplyReturnFillTempOK( CExtNumEdit *pclMaxEdit, CExtNumEdit *pclSupplyEdit, CExtNumEdit *pclReturnEdit,
		CExtNumEdit *pclFillEdit )
{
	if( NULL == pclMaxEdit || NULL == pclSupplyEdit || NULL == pclReturnEdit || NULL == pclFillEdit || NULL == m_pclWizardSelPMParams 
			|| NULL == m_pclWizardSelPMParams->m_pclSelectPMList || NULL == m_pclWizardSelPMParams->m_pclSelectPMList->GetpclPMInputUser() )
	{
		ASSERTA_RETURN( false );
	}

	// By default, reset all error on temperature fields.
	pclMaxEdit->ResetDrawBorder();
	pclSupplyEdit->ResetDrawBorder();
	pclReturnEdit->ResetDrawBorder();
	pclFillEdit->ResetDrawBorder();

	ResetToolTipAndErrorBorder( pclMaxEdit );
	ResetToolTipAndErrorBorder( pclSupplyEdit );
	ResetToolTipAndErrorBorder( pclReturnEdit );
	ResetToolTipAndErrorBorder( pclFillEdit );

	// After reset reload default values.
	InitToolTips( NULL );

	CString strMax = TASApp.LoadLocalizedString( IDS_DLGINDSELPMPANELS_TTERRORMAXTEMP );
	CString strMaxLowerCase = strMax;
	strMaxLowerCase.MakeLower();
	
	CString strSupply = TASApp.LoadLocalizedString( IDS_DLGINDSELPMPANELS_TTERRORSUPPLYTEMP );
	CString strSupplyLowerCase = strSupply;
	strSupplyLowerCase.MakeLower();
	
	CString strReturn = TASApp.LoadLocalizedString( IDS_DLGINDSELPMPANELS_TTERRORRETURNTEMP );
	CString strReturnLowerCase = strReturn;
	strReturnLowerCase.MakeLower();

	CString strFill = TASApp.LoadLocalizedString( IDS_DLGINDSELPMPANELS_TTERRORFILLTEMP );
	CString strFillLowerCase = strFill;
	strFillLowerCase.MakeLower();

	CString strMin = TASApp.LoadLocalizedString( IDS_DLGINDSELPMPANELS_TTERRORMINTEMP );
	CString strMinLowerCase = strMin;
	strMinLowerCase.MakeLower();

	int iError = 0;

	// Before check temperatures between each others, we check first if temperature are valid.
	bool bMaxBadFluidError = false;

	if( false == IsFluidTemperatureOK( pclMaxEdit, strMax ) )
	{
		iError |= Error_Temp_Max;
		bMaxBadFluidError = true;
	}

	bool bSupplyBadFluidError = false;

	if( false == IsFluidTemperatureOK( pclSupplyEdit, strSupply ) )
	{
		iError |= Error_Temp_Supply;
		bSupplyBadFluidError = true;
	}

	bool bReturnBadFluidError = false;

	if( false == IsFluidTemperatureOK( pclReturnEdit, strReturn ) )
	{
		iError |= Error_Temp_Return;
		bReturnBadFluidError = true;
	}

	bool bFillBadFluidError = false;

	if( false == IsFluidTemperatureOK( pclFillEdit, strFill ) )
	{
		iError |= Error_Temp_Fill;
		bFillBadFluidError = true;
	}

	// Check now all temperatures.
	double dMaxTemperature = pclMaxEdit->GetCurrentValSI();
	double dSupplyTemperature = pclSupplyEdit->GetCurrentValSI();
	double dReturnTemperature = pclReturnEdit->GetCurrentValSI();
	double dFillTemperature = pclFillEdit->GetCurrentValSI();
	double dMinTemperature = m_pclWizardSelPMParams->m_pclSelectPMList->GetpclPMInputUser()->GetMinTemperature();
	double dWaterMakeUpTemperature = m_pclWizardSelPMParams->m_pclSelectPMList->GetpclPMInputUser()->GetWaterMakeUpWaterTemp();

	CString strTTMax( _T("") );
	CString strTTSupply( _T("") );
	CString strTTReturn( _T("") );
	CString strTTFill( _T("") );

	// In second step check the max. temperature in regards to the water make-up temperature.
	if( true == m_pclWizardSelPMParams->m_pclSelectPMList->GetpclPMInputUser()->IsWaterMakeUpExist() )
	{
		if(	dMaxTemperature < dWaterMakeUpTemperature )
		{
			// In this case we display a AFX message because water make-up temperature is in the 'Pressure maintenance type'
			// step.
		
			// A water make-up device is required and the water make-up temperature (%1) is above the maximum temperature (%2).
			// Do you want to set the water make-up temperature to the same value of the maximum temperature?
			CString strWaterMakeUpTemp = WriteCUDouble( _U_TEMPERATURE, dWaterMakeUpTemperature, true );
			CString strMaxTemp = WriteCUDouble( _U_TEMPERATURE, dMaxTemperature, true );
			CString strMessage;
			FormatString( strMessage, AFXMSG_PMWIZCOOLING_MAXANDWMUPTEMPINVALID, strWaterMakeUpTemp, strMaxTemp );

			if( IDYES == AfxMessageBox( strMessage, MB_YESNO | MB_ICONSTOP ) )
			{
				m_pclWizardSelPMParams->m_pclSelectPMList->GetpclPMInputUser()->SetWaterMakeUpWaterTemp( dMaxTemperature );	
			}
			else
			{
				pclMaxEdit->SetCurrentValSI( m_dMaxTempSaved );
				pclMaxEdit->Update();
				dMaxTemperature = m_dMaxTempSaved;
			}
		}
	}

	if( dSupplyTemperature > dMaxTemperature )
	{
		if( false == bMaxBadFluidError )
		{
			FormatString( strTTMax, IDS_DLGINDSELPMPANELS_TTERRORTEMPLOWER, strMax, strSupplyLowerCase );
		}

		if( false == bSupplyBadFluidError )
		{
			FormatString( strTTSupply, IDS_DLGINDSELPMPANELS_TTERRORTEMPGREATER, strSupply, strMaxLowerCase );
		}

		iError |= ( Error_Temp_Max | Error_Temp_Supply );
	}

	if( dReturnTemperature > dMaxTemperature )
	{
		if( false == bMaxBadFluidError )
		{
			if( false == strTTMax.IsEmpty() )
			{
				strTTMax += _T("\r\n");
			}
			
			CString strTemp;
			FormatString( strTemp, IDS_DLGINDSELPMPANELS_TTERRORTEMPLOWER, strMax, strReturnLowerCase );
			strTTMax += strTemp;
		}

		if( false == bReturnBadFluidError )
		{
			if( false == strTTReturn.IsEmpty() )
			{
				strTTReturn += _T("\r\n");
			}

			CString strTemp;
			FormatString( strTemp, IDS_DLGINDSELPMPANELS_TTERRORTEMPGREATER, strReturn, strMaxLowerCase );
			strTTReturn += strTemp;
		}

		iError |= ( Error_Temp_Max | Error_Temp_Return );
	}

	if( dReturnTemperature < dSupplyTemperature )
	{
		if( false == bReturnBadFluidError )
		{
			if( false == strTTReturn.IsEmpty() )
			{
				strTTReturn += _T("\r\n");
			}

			CString strTemp;
			FormatString( strTemp, IDS_DLGINDSELPMPANELS_TTERRORTEMPLOWER, strReturn, strSupplyLowerCase );
			strTTReturn += strTemp;
		}

		if( false == bSupplyBadFluidError )
		{
			if( false == strTTSupply.IsEmpty() )
			{
				strTTSupply += _T("\r\n");
			}
			
			CString strTemp;
			FormatString( strTemp, IDS_DLGINDSELPMPANELS_TTERRORTEMPGREATER, strSupply, strReturnLowerCase );
			strTTSupply += strTemp;
		}

		iError |= ( Error_Temp_Return | Error_Temp_Supply );
	}

	if( true == m_pclWizardSelPMParams->m_pclSelectPMList->GetpclPMInputUser()->GetCheckMinTemperature() 
			&& dMinTemperature > dSupplyTemperature )
	{
		if( false == bSupplyBadFluidError )
		{
			if( false == strTTSupply.IsEmpty() )
			{
				strTTSupply += _T("\r\n");
			}

			CString strTemp;
			FormatString( strTemp, IDS_DLGINDSELPMPANELS_TTERRORTEMPLOWER, strSupply, strMinLowerCase );
			strTTSupply += strTemp;
		}

		iError |= ( Error_Temp_Supply | Error_Temp_Min );
	}

	if( dFillTemperature > dMaxTemperature )
	{
		if( false == bFillBadFluidError )
		{
			if( false == strTTFill.IsEmpty() )
			{
				strTTFill += _T("\n");
			}

			CString strTemp;
			FormatString( strTemp, IDS_DLGINDSELPMPANELS_TTERRORTEMPGREATER, strFill, strMaxLowerCase );
			strTTFill += strTemp;
		}

		iError |= ( Error_Temp_Fill | Error_Temp_Max );
	}

	if( dFillTemperature < dMinTemperature )
	{
		if( false == bFillBadFluidError )
		{
			if( false == strTTFill.IsEmpty() )
			{
				strTTFill += _T("\n");
			}

			CString strTemp;
			FormatString( strTemp, IDS_DLGINDSELPMPANELS_TTERRORTEMPLOWER, strFill, strMinLowerCase );
			strTTFill += strTemp;
		}

		iError |= ( Error_Temp_Fill | Error_Temp_Min );
	}

	if( false == strTTMax.IsEmpty() )
	{
		ResetToolTipAndErrorBorder( pclMaxEdit );
		m_ToolTip.AddToolWindow( pclMaxEdit, strTTMax );
		pclMaxEdit->SetDrawBorder( true, _RED );
	}

	if( false == strTTSupply.IsEmpty() )
	{
		ResetToolTipAndErrorBorder( pclSupplyEdit );
		m_ToolTip.AddToolWindow( pclSupplyEdit, strTTSupply );
		pclSupplyEdit->SetDrawBorder( true, _RED );
	}

	if( false == strTTReturn.IsEmpty() )
	{
		ResetToolTipAndErrorBorder( pclReturnEdit );
		m_ToolTip.AddToolWindow( pclReturnEdit, strTTReturn );
		pclReturnEdit->SetDrawBorder( true, _RED );
	}

	if( false == strTTFill.IsEmpty() )
	{
		ResetToolTipAndErrorBorder( pclFillEdit );
		m_ToolTip.AddToolWindow( pclFillEdit, strTTFill );
		pclFillEdit->SetDrawBorder( true, _RED );
	}

	return iError;
}

int CDlgWizardPM_RightViewInput_Base::AreCoolingTemperaturesOK( CExtNumEdit *pclMaxEdit, CExtNumEdit *pclSupplyEdit, CExtNumEdit *pclReturnEdit, 
		CExtNumEdit *pclFillEdit, CExtNumEdit *pclMinEdit, INT iCheckMinTemp )
{
	if( NULL == pclMaxEdit || NULL == pclSupplyEdit || NULL == pclReturnEdit || NULL == pclFillEdit ||  NULL == pclMinEdit 
			|| NULL == m_pclWizardSelPMParams || NULL == m_pclWizardSelPMParams->m_pclSelectPMList 
			|| NULL == m_pclWizardSelPMParams->m_pclSelectPMList->GetpclPMInputUser() )
	{
		ASSERTA_RETURN( false );
	}

	// By default, reset all error on temperature fields.
	pclMaxEdit->ResetDrawBorder();
	pclSupplyEdit->ResetDrawBorder();
	pclReturnEdit->ResetDrawBorder();
	pclFillEdit->ResetDrawBorder();
	pclMinEdit->ResetDrawBorder();

	ResetToolTipAndErrorBorder( pclMaxEdit );
	ResetToolTipAndErrorBorder( pclSupplyEdit );
	ResetToolTipAndErrorBorder( pclReturnEdit );
	ResetToolTipAndErrorBorder( pclFillEdit );
	ResetToolTipAndErrorBorder( pclMinEdit );

	// After reset reload default values.
	InitToolTips( NULL );

	CString strMax = TASApp.LoadLocalizedString( IDS_DLGINDSELPMPANELS_TTERRORMAXTEMP );
	CString strMaxLowerCase = strMax;
	strMaxLowerCase.MakeLower();

	CString strSupply = TASApp.LoadLocalizedString( IDS_DLGINDSELPMPANELS_TTERRORSUPPLYTEMP );
	CString strSupplyLowerCase = strSupply;
	strSupplyLowerCase.MakeLower();

	CString strReturn = TASApp.LoadLocalizedString( IDS_DLGINDSELPMPANELS_TTERRORRETURNTEMP );
	CString strReturnLowerCase = strReturn;
	strReturnLowerCase.MakeLower();

	CString strFill = TASApp.LoadLocalizedString( IDS_DLGINDSELPMPANELS_TTERRORFILLTEMP );
	CString strFillLowerCase = strFill;
	strFillLowerCase.MakeLower();

	CString strMin = TASApp.LoadLocalizedString( IDS_DLGINDSELPMPANELS_TTERRORMINTEMP );
	CString strMinLowerCase = strMin;
	strMinLowerCase.MakeLower();

	int iError = 0;

	// Before check temperatures between each others, we check first if temperature are valid.
	bool bMaxBadFluidError = false;

	if( false == IsFluidTemperatureOK( pclMaxEdit, strMax ) )
	{
		iError |= Error_Temp_Max;
		bMaxBadFluidError = true;
	}

	bool bSupplyBadfluidError = false;

	if( false == IsFluidTemperatureOK( pclSupplyEdit, strSupply ) )
	{
		iError |= Error_Temp_Supply;
		bSupplyBadfluidError = true;
	}

	bool bReturnBadFluidError = false;

	if( false == IsFluidTemperatureOK( pclReturnEdit, strReturn ) )
	{
		iError |= Error_Temp_Return;
		bReturnBadFluidError = true;
	}

	bool bFillBadFluidError = false;

	if( false == IsFluidTemperatureOK( pclFillEdit, strFill ) )
	{
		iError |= Error_Temp_Fill;
		bFillBadFluidError = true;
	}

	bool bMinBadFluidError = false;

	if( BST_CHECKED == iCheckMinTemp && false == IsFluidTemperatureOK( pclMinEdit, strMin ) )
	{
		iError |= Error_Temp_Min;
		bMinBadFluidError = true;
	}

	// Check now all temperatures.
	double dMaxTemperature = pclMaxEdit->GetCurrentValSI();
	double dSupplyTemperature = pclSupplyEdit->GetCurrentValSI();
	double dReturnTemperature = pclReturnEdit->GetCurrentValSI();
	double dFillTemperature = pclFillEdit->GetCurrentValSI();
	double dMinTemperature = pclMinEdit->GetCurrentValSI();
	double dWaterMakeUpTemperature = m_pclWizardSelPMParams->m_pclSelectPMList->GetpclPMInputUser()->GetWaterMakeUpWaterTemp();

	CString strTTMax( _T("") );
	CString strTTSupply( _T("") );
	CString strTTReturn( _T("") );
	CString strTTFill( _T("") );
	CString strTTMin( _T("") );

	// In second step check the max. and min. temperature in regards to the water make-up temperature.
	if( true == m_pclWizardSelPMParams->m_pclSelectPMList->GetpclPMInputUser()->IsWaterMakeUpExist() )
	{
		if(	dMaxTemperature < dWaterMakeUpTemperature )
		{
			// In this case we display a AFX message because water make-up temperature is in the 'Pressure maintenance type'
			// step.
		
			// A water make-up device is required and the water make-up temperature (%1) is above the maximum temperature (%2).
			// Do you want to set the water make-up temperature to the same value of the maximum temperature?
			CString strWaterMaleUpTemp = WriteCUDouble( _U_TEMPERATURE, dWaterMakeUpTemperature, true );
			CString strMaxTemp = WriteCUDouble( _U_TEMPERATURE, dMaxTemperature, true );
			CString strMessage;
			FormatString( strMessage, AFXMSG_PMWIZCOOLING_MAXANDWMUPTEMPINVALID, strWaterMaleUpTemp, strMaxTemp );

			if( IDYES == AfxMessageBox( strMessage, MB_YESNO | MB_ICONSTOP ) )
			{
				m_pclWizardSelPMParams->m_pclSelectPMList->GetpclPMInputUser()->SetWaterMakeUpWaterTemp( dMaxTemperature );	
			}
			else
			{
				pclMaxEdit->SetCurrentValSI( m_dMaxTempSaved );
				pclMaxEdit->Update();
				dMaxTemperature = m_dMaxTempSaved;
			}
		}

		if(	dMinTemperature > dWaterMakeUpTemperature )
		{
			// In this case we display a AFX message because water make-up temperature is in the 'Pressure maintenance type'
			// step.
		
			// A water make-up device is required and the water make-up temperature (%1) is below the minimum temperature (%2).
			// Do you want to set the water make-up temperature to the same value of the minimum temperature?
			CString strWaterMaleUpTemp = WriteCUDouble( _U_TEMPERATURE, dWaterMakeUpTemperature, true );
			CString strMinTemp = WriteCUDouble( _U_TEMPERATURE, dMinTemperature, true );
			CString strMessage;
			FormatString( strMessage, AFXMSG_PMWIZCOOLING_MINANDWMUPTEMPINVALID, strWaterMaleUpTemp, strMinTemp );

			if( IDYES == AfxMessageBox( strMessage, MB_YESNO | MB_ICONSTOP ) )
			{
				m_pclWizardSelPMParams->m_pclSelectPMList->GetpclPMInputUser()->SetWaterMakeUpWaterTemp( dMinTemperature );	
			}
			else
			{
				pclMinEdit->SetCurrentValSI( m_dMinTempSaved );
				pclMinEdit->Update();
				dMinTemperature = m_dMinTempSaved;
			}
		}
	}

	if( dMaxTemperature < dReturnTemperature )
	{
		if( false == bMaxBadFluidError )
		{
			FormatString( strTTMax, IDS_DLGINDSELPMPANELS_TTERRORTEMPLOWER, strMax, strReturnLowerCase );
		}

		if( false == bReturnBadFluidError )
		{
			FormatString( strTTReturn, IDS_DLGINDSELPMPANELS_TTERRORTEMPGREATER, strReturn, strMaxLowerCase );
		}

		iError |= ( Error_Temp_Max | Error_Temp_Return );
	}

	if( dReturnTemperature < dSupplyTemperature )
	{
		if( false == bReturnBadFluidError )
		{
			if( false == strTTReturn.IsEmpty() )
			{
				strTTReturn += _T("\r\n");
			}

			CString strTemp;
			FormatString( strTemp, IDS_DLGINDSELPMPANELS_TTERRORTEMPLOWER, strReturn, strSupplyLowerCase );
			strTTReturn += strTemp;
		}

		if( false == bSupplyBadfluidError )
		{
			FormatString( strTTSupply, IDS_DLGINDSELPMPANELS_TTERRORTEMPGREATER, strSupply, strReturnLowerCase );
		}

		iError |= ( Error_Temp_Return | Error_Temp_Supply );
	}

	if( dFillTemperature > dMaxTemperature )
	{
		if( false == bFillBadFluidError )
		{
			if( false == strTTFill.IsEmpty() )
			{
				strTTFill += _T("\n");
			}

			CString strTemp;
			FormatString( strTemp, IDS_DLGINDSELPMPANELS_TTERRORTEMPGREATER, strFill, strMaxLowerCase );
			strTTFill += strTemp;
		}

		iError |= ( Error_Temp_Fill | Error_Temp_Max );
	}

	if( dFillTemperature < dMinTemperature )
	{
		if( false == bFillBadFluidError )
		{
			if( false == strTTFill.IsEmpty() )
			{
				strTTFill += _T("\n");
			}

			CString strTemp;
			FormatString( strTemp, IDS_DLGINDSELPMPANELS_TTERRORTEMPLOWER, strFill, strMinLowerCase );
			strTTFill += strTemp;
		}

		iError |= ( Error_Temp_Fill | Error_Temp_Min );
	}

	if( BST_CHECKED == iCheckMinTemp && dMinTemperature > dSupplyTemperature )
	{
		if( false == bMinBadFluidError )
		{
			FormatString( strTTMin, IDS_DLGINDSELPMPANELS_TTERRORTEMPGREATER, strMin, strSupplyLowerCase );
		}

		if( false == bSupplyBadfluidError )
		{
			if( false == strTTSupply.IsEmpty() )
			{
				strTTSupply += _T("\r\n");
			}

			CString strTemp;
			FormatString( strTemp, IDS_DLGINDSELPMPANELS_TTERRORTEMPLOWER, strSupply, strMinLowerCase );
			strTTSupply += strTemp;
		}

		iError |= ( Error_Temp_Min | Error_Temp_Supply );
	}

	if( false == strTTMax.IsEmpty() )
	{
		ResetToolTipAndErrorBorder( pclMaxEdit );
		m_ToolTip.AddToolWindow( pclMaxEdit, strTTMax );
		pclMaxEdit->SetDrawBorder( true, _RED );
	}

	if( false == strTTSupply.IsEmpty() )
	{
		ResetToolTipAndErrorBorder( pclSupplyEdit );
		m_ToolTip.AddToolWindow( pclSupplyEdit, strTTSupply );
		pclSupplyEdit->SetDrawBorder( true, _RED );
	}

	if( false == strTTReturn.IsEmpty() )
	{
		ResetToolTipAndErrorBorder( pclReturnEdit );
		m_ToolTip.AddToolWindow( pclReturnEdit, strTTReturn );
		pclReturnEdit->SetDrawBorder( true, _RED );
	}

	if( false == strTTFill.IsEmpty() )
	{
		ResetToolTipAndErrorBorder( pclFillEdit );
		m_ToolTip.AddToolWindow( pclFillEdit, strTTFill );
		pclFillEdit->SetDrawBorder( true, _RED );
	}

	if( false == strTTMin.IsEmpty() )
	{
		ResetToolTipAndErrorBorder( pclMinEdit );
		m_ToolTip.AddToolWindow( pclMinEdit, strTTMin );
		pclMinEdit->SetDrawBorder( true, _RED );
	}

	return iError;
}

int CDlgWizardPM_RightViewInput_Base::IsHeatingSolarSafetyTempLimiterOK()
{
	if( NULL == m_pclWizardSelPMParams || NULL == m_pclWizardSelPMParams->m_pTADS || NULL == m_pclWizardSelPMParams->m_pTADS->GetpTechParams() 
			|| NULL == m_pclWizardSelPMParams->m_pclSelectPMList || NULL == m_pclWizardSelPMParams->m_pclSelectPMList->GetpclPMInputUser() )
	{
		ASSERTA_RETURN( 0 );
	}
	
	// We check first if the safety temperature limiter is valid.
	double dSafetyTempLimiter = m_pclWizardSelPMParams->m_pclSelectPMList->GetpclPMInputUser()->GetSafetyTempLimiter();
	double dSupplyTemperature = m_pclWizardSelPMParams->m_pclSelectPMList->GetpclPMInputUser()->GetSupplyTemperature();
	int iError = 0;

	if( CWaterChar::efrcFluidOk != m_pclWizardSelPMParams->m_WC.CheckFluidData( dSafetyTempLimiter ) )
	{
		iError |= Error_Temp_SafetyTempLimiter;
	}

	// Check the EN12953 norm.
	if( _T("PM_NORM_EN12828" ) == CString( m_pclWizardSelPMParams->m_pclSelectPMList->GetpclPMInputUser()->GetNormID() ) )
	{
		double dTAZLimit = m_pclWizardSelPMParams->m_pTADS->GetpTechParams()->GetSafetyTempLimiterLimitEN12953();

		if( dSafetyTempLimiter > dTAZLimit )
		{
			iError |= Error_Temp_SafetyTempLimiter;
		}
	}

	// Check now all temperatures.
	if( dSafetyTempLimiter < dSupplyTemperature )
	{
		iError |= ( Error_Temp_SafetyTempLimiter | Error_Temp_Supply );
	}

	return iError;
}

int CDlgWizardPM_RightViewInput_Base::AreHeatingSolarTemperaturesOK()
{
	if( NULL == m_pclWizardSelPMParams || NULL == m_pclWizardSelPMParams->m_pTADS || NULL == m_pclWizardSelPMParams->m_pTADS->GetpTechParams()
			|| NULL == m_pclWizardSelPMParams->m_pclSelectPMList || NULL == m_pclWizardSelPMParams->m_pclSelectPMList->GetpclPMInputUser() )
	{
		ASSERTA_RETURN( 0 );
	}

	// Before check temperatures between each others, we check first if temperature are valid.
	double dSafetyTempLimiter = m_pclWizardSelPMParams->m_pclSelectPMList->GetpclPMInputUser()->GetSafetyTempLimiter();
	double dSupplyTemperature = m_pclWizardSelPMParams->m_pclSelectPMList->GetpclPMInputUser()->GetSupplyTemperature();
	double dReturnTemperature = m_pclWizardSelPMParams->m_pclSelectPMList->GetpclPMInputUser()->GetReturnTemperature();
	double dMinTemperature = m_pclWizardSelPMParams->m_pclSelectPMList->GetpclPMInputUser()->GetMinTemperature();
	double dFillTemperature = m_pclWizardSelPMParams->m_pclSelectPMList->GetpclPMInputUser()->GetFillTemperature();
	int iError = 0;
	
	if( CWaterChar::efrcFluidOk != m_pclWizardSelPMParams->m_WC.CheckFluidData( dSafetyTempLimiter ) )
	{
		iError |= Error_Temp_SafetyTempLimiter;
	}

	if( CWaterChar::efrcFluidOk != m_pclWizardSelPMParams->m_WC.CheckFluidData( dSupplyTemperature ) )
	{
		iError |= Error_Temp_Supply;
	}

	if( CWaterChar::efrcFluidOk != m_pclWizardSelPMParams->m_WC.CheckFluidData( dReturnTemperature ) )
	{
		iError |= Error_Temp_Return;
	}

	if( CWaterChar::efrcFluidOk != m_pclWizardSelPMParams->m_WC.CheckFluidData( dMinTemperature ) )
	{
		iError |= Error_Temp_Min;
	}

	if( CWaterChar::efrcFluidOk != m_pclWizardSelPMParams->m_WC.CheckFluidData( dFillTemperature ) )
	{
		iError |= Error_Temp_Fill;
	}

	// Check the EN12953 norm.
	if( _T("PM_NORM_EN12828" ) == CString( m_pclWizardSelPMParams->m_pclSelectPMList->GetpclPMInputUser()->GetNormID() ) )
	{
		double dTAZLimit = m_pclWizardSelPMParams->m_pTADS->GetpTechParams()->GetSafetyTempLimiterLimitEN12953();
		double dSupplyTempLimit = m_pclWizardSelPMParams->m_pTADS->GetpTechParams()->GetFluidTempLimitEN12953();
		double dReturnTempLimit = dSupplyTempLimit;

		if( dSafetyTempLimiter > dTAZLimit )
		{
			iError |= Error_Temp_SafetyTempLimiter;
		}

		if( dSupplyTemperature > dSupplyTempLimit )
		{
			iError |= Error_Temp_Supply;
		}

		if( dReturnTemperature > dReturnTempLimit )
		{
			iError |= Error_Temp_Return;
		}
	}

	// Check now all temperatures.
	if( dSafetyTempLimiter < dSupplyTemperature )
	{
		iError |= ( Error_Temp_SafetyTempLimiter | Error_Temp_Supply );
	}

	if( dSupplyTemperature < dReturnTemperature )
	{
		iError |= ( Error_Temp_Supply | Error_Temp_Return );
	}

	if( dReturnTemperature < dFillTemperature )
	{
		iError |= ( Error_Temp_Return | Error_Temp_Fill );
	}

	if( dFillTemperature < dMinTemperature )
	{
		iError |= ( Error_Temp_Fill | Error_Temp_Min );
	}

	return iError;
}

int CDlgWizardPM_RightViewInput_Base::AreCoolingMaxSupplyReturnFillTempOK()
{
	if( NULL == m_pclWizardSelPMParams || NULL == m_pclWizardSelPMParams->m_pclSelectPMList 
			|| NULL == m_pclWizardSelPMParams->m_pclSelectPMList->GetpclPMInputUser() )
	{
		ASSERTA_RETURN( false );
	}

	// Before check temperatures between each others, we check first if temperature are valid.
	double dMaxTemperature = m_pclWizardSelPMParams->m_pclSelectPMList->GetpclPMInputUser()->GetMaxTemperature();
	double dSupplyTemperature = m_pclWizardSelPMParams->m_pclSelectPMList->GetpclPMInputUser()->GetSupplyTemperature();
	double dReturnTemperature = m_pclWizardSelPMParams->m_pclSelectPMList->GetpclPMInputUser()->GetReturnTemperature();
	double dFillTemperature = m_pclWizardSelPMParams->m_pclSelectPMList->GetpclPMInputUser()->GetFillTemperature();
	double dMinTemperature = m_pclWizardSelPMParams->m_pclSelectPMList->GetpclPMInputUser()->GetMinTemperature();

	int iError = 0;

	if( CWaterChar::efrcFluidOk != m_pclWizardSelPMParams->m_WC.CheckFluidData( dMaxTemperature ) )
	{
		iError |= Error_Temp_Max;
	}

	if( CWaterChar::efrcFluidOk != m_pclWizardSelPMParams->m_WC.CheckFluidData( dSupplyTemperature ) )
	{
		iError |= Error_Temp_Supply;
	}

	if( CWaterChar::efrcFluidOk != m_pclWizardSelPMParams->m_WC.CheckFluidData( dReturnTemperature ) )
	{
		iError |= Error_Temp_Return;
	}

	if( CWaterChar::efrcFluidOk != m_pclWizardSelPMParams->m_WC.CheckFluidData( dFillTemperature ) )
	{
		iError |= Error_Temp_Fill;
	}

	// Check now all temperatures.
	if( dSupplyTemperature > dMaxTemperature )
	{
		iError |= ( Error_Temp_Max | Error_Temp_Supply );
	}

	if( dReturnTemperature > dMaxTemperature )
	{
		iError |= ( Error_Temp_Max | Error_Temp_Return );
	}

	if( dReturnTemperature < dSupplyTemperature )
	{
		iError |= ( Error_Temp_Return | Error_Temp_Supply );
	}

	if( true == m_pclWizardSelPMParams->m_pclSelectPMList->GetpclPMInputUser()->GetCheckMinTemperature() 
			&& dMinTemperature > dSupplyTemperature )
	{
		iError |= ( Error_Temp_Min | Error_Temp_Supply );
	}

	if( dFillTemperature > dMaxTemperature )
	{
		iError |= ( Error_Temp_Fill | Error_Temp_Max );
	}

	if( dFillTemperature < dMinTemperature )
	{
		iError |= ( Error_Temp_Fill | Error_Temp_Min );
	}

	return iError;
}

int CDlgWizardPM_RightViewInput_Base::AreCoolingTemperaturesOK()
{
	if( NULL == m_pclWizardSelPMParams || NULL == m_pclWizardSelPMParams->m_pclSelectPMList 
			|| NULL == m_pclWizardSelPMParams->m_pclSelectPMList->GetpclPMInputUser() )
	{
		ASSERTA_RETURN( false );
	}

	// Before check temperatures between each others, we check first if temperature are valid.
	double dMaxTemperature = m_pclWizardSelPMParams->m_pclSelectPMList->GetpclPMInputUser()->GetMaxTemperature();
	double dSupplyTemperature = m_pclWizardSelPMParams->m_pclSelectPMList->GetpclPMInputUser()->GetSupplyTemperature();
	double dReturnTemperature = m_pclWizardSelPMParams->m_pclSelectPMList->GetpclPMInputUser()->GetReturnTemperature();
	double dFillTemperature = m_pclWizardSelPMParams->m_pclSelectPMList->GetpclPMInputUser()->GetReturnTemperature();
	double dMinTemperature = m_pclWizardSelPMParams->m_pclSelectPMList->GetpclPMInputUser()->GetMinTemperature();

	int iError = 0;

	if( CWaterChar::efrcFluidOk != m_pclWizardSelPMParams->m_WC.CheckFluidData( dMaxTemperature ) )
	{
		iError |= Error_Temp_Max;
	}

	if( CWaterChar::efrcFluidOk != m_pclWizardSelPMParams->m_WC.CheckFluidData( dSupplyTemperature ) )
	{
		iError |= Error_Temp_Supply;
	}

	if( CWaterChar::efrcFluidOk != m_pclWizardSelPMParams->m_WC.CheckFluidData( dReturnTemperature ) )
	{
		iError |= Error_Temp_Return;
	}

	if( CWaterChar::efrcFluidOk != m_pclWizardSelPMParams->m_WC.CheckFluidData( dFillTemperature ) )
	{
		iError |= Error_Temp_Fill;
	}

	if( true == m_pclWizardSelPMParams->m_pclSelectPMList->GetpclPMInputUser()->GetCheckMinTemperature() 
			&& CWaterChar::efrcFluidOk != m_pclWizardSelPMParams->m_WC.CheckFluidData( dMinTemperature ) )
	{
		iError |= Error_Temp_Min;
	}

	// Check now all temperatures.
	if( dMaxTemperature < dReturnTemperature )
	{
		iError |= ( Error_Temp_Max | Error_Temp_Return );
	}

	if( dReturnTemperature < dSupplyTemperature )
	{
		iError |= ( Error_Temp_Return | Error_Temp_Supply );
	}

	if( true == m_pclWizardSelPMParams->m_pclSelectPMList->GetpclPMInputUser()->GetCheckMinTemperature() 
			&& dMinTemperature > dSupplyTemperature )
	{
		iError |= ( Error_Temp_Min | Error_Temp_Supply );
	}

	if( dFillTemperature > dMaxTemperature )
	{
		iError |= ( Error_Temp_Fill | Error_Temp_Max );
	}

	if( dFillTemperature < dMinTemperature )
	{
		iError |= ( Error_Temp_Fill | Error_Temp_Min );
	}

	return iError;
}

bool CDlgWizardPM_RightViewInput_Base::CheckPz( CButton *pBtnCheckPz, CExtNumEdit *pclPzEdit )
{
	if( NULL == m_pclWizardSelPMParams || NULL == m_pclWizardSelPMParams->m_pclSelectPMList 
			|| NULL == m_pclWizardSelPMParams->m_pclSelectPMList->GetpclPMInputUser() )
	{
		ASSERTA_RETURN( false );
	}

	if( NULL == pBtnCheckPz || NULL == pclPzEdit )
	{
		return true;
	}

	if( BST_UNCHECKED == pBtnCheckPz->GetCheck() )
	{
		return true;
	}

	// By default reset error.
	pclPzEdit->ResetDrawBorder();
	ResetToolTipAndErrorBorder( pclPzEdit );

	// After reset reload default values.
	InitToolTips( NULL );

	bool bReturn = true;
	double dPz = pclPzEdit->GetCurrentValSI();
	double dP0 = m_pclWizardSelPMParams->m_pclSelectPMList->GetpclPMInputUser()->GetMinimumPressure( false );

	if( dPz < dP0 )
	{
		CString strPz = WriteCUDouble( _U_PRESSURE, dPz, false );
		CString strP0 = WriteCUDouble( _U_PRESSURE, dP0, false );

		CString msg;
		FormatString( msg, IDS_DLGINDSELPMPANELS_TTERRORPZ, strPz, strP0 );
		m_ToolTip.AddToolWindow( pclPzEdit, msg );
		pclPzEdit->SetDrawBorder( true, _RED );

		bReturn = false;
	}

	return bReturn;
}

bool CDlgWizardPM_RightViewInput_Base::CheckSafetyVRP( CExtNumEdit *pclEditSafetyPSV, CExtStatic *pclStaticSafetyPSVMinimum )
{
	if( NULL == m_pclWizardSelPMParams || NULL == m_pclWizardSelPMParams->m_pclSelectPMList 
			|| NULL == m_pclWizardSelPMParams->m_pclSelectPMList->GetpclPMInputUser() )
	{
		ASSERTA_RETURN( false );
	}

	if( NULL == pclEditSafetyPSV )
	{
		return true;
	}

	// By default reset error.
	pclEditSafetyPSV->ResetDrawBorder();
	ResetToolTipAndErrorBorder( pclEditSafetyPSV );

	// After reset reload default values.
	InitToolTips( NULL );
	
	bool bReturn = true;
	double dSafetyVRP = pclEditSafetyPSV->GetCurrentValSI();
	double dPSVLimit = 0.0;

	// Check what is the PSV limit.
	bool bPSVCorrect = m_pclWizardSelPMParams->m_pclSelectPMList->GetpclPMInputUser()->CheckPSV( &dPSVLimit, dSafetyVRP );
	CString strPSVLimit = WriteCUDouble( _U_PRESSURE, dPSVLimit, true, -1, 2 );

	// Update PSV limit static text.
	CString msg;
	FormatString( msg, IDS_DLGWIZPM_RVIEWINPUT_PSVMIN, strPSVLimit );
	pclStaticSafetyPSVMinimum->SetWindowTextW( msg );

	if( false == bPSVCorrect )
	{
		// We take what the user has written to avoid rounding this value with error.
		// Example: user input: 5,726; value computed: 5,73 -> If we use 'WriteCUDouble' for the use inpur, we will have 
		//          in the error message "5,73 < 5,73".
		TCHAR tcPSVInput[256];
		pclEditSafetyPSV->GetWindowText( tcPSVInput, 256 );

		CString strPSVInput = tcPSVInput;
		CUnitDatabase *pclUnitDB = CDimValue::AccessUDB();
		strPSVInput += _T(" ");
		strPSVInput += CString( GetNameOf( pclUnitDB->GetDefaultUnit( _U_PRESSURE ) ).c_str() );	
		
		CString msg;
		FormatString( msg, IDS_DLGINDSELPMPANELS_TTERRORPSV, strPSVInput, strPSVLimit );

		m_ToolTip.AddToolWindow( pclEditSafetyPSV, msg );
		pclEditSafetyPSV->SetDrawBorder( true, _RED );

		bReturn = false;
	}

	return bReturn;
}

bool CDlgWizardPM_RightViewInput_Base::IspSVLocationOK( CExtNumEdit* pclEditpSVLocation )
{
	if( NULL == m_pclWizardSelPMParams || NULL == m_pclWizardSelPMParams->m_pclSelectPMList
		|| NULL == m_pclWizardSelPMParams->m_pclSelectPMList->GetpclPMInputUser() )
	{
		ASSERTA_RETURN( false );
	}

	bool bReturn = true;

	if( NULL == pclEditpSVLocation )
	{
		return true;
	}

	// By default reset the error.
	pclEditpSVLocation->ResetDrawBorder();
	ResetToolTipAndErrorBorder( pclEditpSVLocation );

	// After reset reload default values.
	// Call InitToolTips with specific control to avoid rest of min. temp. control tooltip
	InitToolTips( pclEditpSVLocation );

	CPMInputUser* pclPMInputUser = m_pclWizardSelPMParams->m_pclSelectPMList->GetpclPMInputUser();
	double dpSVLocation = pclEditpSVLocation->GetCurrentValSI();

	CString strToolTip( _T( "" ) );

	if( dpSVLocation >= 0.0 && dpSVLocation > pclPMInputUser->GetStaticHeight() )
	{
		// The safety valve can't be placed in a higher point that the static height defined (%1 > %2).
		CString strpSVLocation = WriteCUDouble( _U_LENGTH, dpSVLocation, true );
		CString strStaticHeight = WriteCUDouble( _U_LENGTH, pclPMInputUser->GetStaticHeight(), true );
		FormatString( strToolTip, IDS_DLGINDSELPMPANELS_TTERRORPSVLOCATIONTOHIGH, strpSVLocation, strStaticHeight );
	}
	else if( dpSVLocation < 0.0 )
	{
		// See the 'PM - Max height between vessel and pSV.docx' documentation for explanation.
		double dHLimit = pclPMInputUser->GetSafetyValveResponsePressure() - pclPMInputUser->GetMinimumRequiredPSVRaw( pclPMInputUser->GetPressureMaintenanceType() );
		dHLimit /= ( pclPMInputUser->GetpWC()->GetDens() * 9.81 );

		if( abs( dpSVLocation ) > dHLimit )
		{
			// The safety valve is placed too low. The actual limit is %1.
			CString strHLimit = WriteCUDouble( _U_LENGTH, dHLimit, true );
			FormatString( strToolTip, IDS_DLGINDSELPMPANELS_TTERRORPSVLOCATIONTOLOW, strHLimit );
		}
	}

	if( false == strToolTip.IsEmpty() )
	{
		m_ToolTip.AddToolWindow( pclEditpSVLocation, strToolTip );
		pclEditpSVLocation->SetDrawBorder( true, _RED );
		bReturn = false;
	}

	return bReturn;
}

void CDlgWizardPM_RightViewInput_Base::DrawGoAdvancedNormalModeButtton( DrawWhichMode eDrawWhichMode, CRect &rectPos )
{
	CDC *pDC = GetDC();

	CRect rect;
	GetClientRect( &rect );

	CBrush brush;
	brush.CreateSolidBrush( RGB( 0, 120, 215 ) );

	CRect rectPaint( 20, rectPos.top, rect.Width() - 20, rectPos.top + 20 );

	pDC->FillRect( &rectPaint, &brush );

	rectPaint.DeflateRect( 2, 2 );
	brush.DeleteObject();
	brush.CreateSolidBrush( RGB( 147, 190, 232 ) );
	pDC->FillRect( &rectPaint, &brush );

	rectPaint.DeflateRect( 1, 1 );
	brush.DeleteObject();
	brush.CreateSolidBrush( RGB( 185, 208, 237 ) );
	pDC->FillRect( &rectPaint, &brush );
	
	brush.DeleteObject();

	CPen pen;
	pen.CreatePen( PS_SOLID, 2, RGB( 0, 120, 215 ) );
	CPen *pOldPen = pDC->SelectObject( &pen );

	CString strText;

	if( Draw_AdvancedMode == eDrawWhichMode )
	{
		pDC->MoveTo( 20 + rectPaint.Height(), rectPaint.bottom - 4 );
		pDC->LineTo( 20 + rectPaint.Height() - rectPaint.Height() + 8, rectPaint.top + 4 );
		pDC->MoveTo( 20 + rectPaint.Height(), rectPaint.bottom - 4 );
		pDC->LineTo( 20 + rectPaint.Height() + rectPaint.Height() - 8, rectPaint.top + 4 );
		strText = TASApp.LoadLocalizedString( IDS_DLGWIZPM_RVIEWINPUT_GOADVANCEDMODE );
	}
	else
	{
		pDC->MoveTo( 20 + rectPaint.Height(), rectPaint.top + 4 );
		pDC->LineTo( 20 + rectPaint.Height() - rectPaint.Height() + 8, rectPaint.bottom - 4 );
		pDC->MoveTo( 20 + rectPaint.Height(), rectPaint.top + 4 );
		pDC->LineTo( 20 + rectPaint.Height() + rectPaint.Height() - 8, rectPaint.bottom - 4 );
		strText = TASApp.LoadLocalizedString( IDS_DLGWIZPM_RVIEWINPUT_GONORMALMODE );
	}

	pDC->SelectObject( pOldPen );
	pen.DeleteObject();

	CRect rectText( rectPaint );
	rectText.left += ( rectPaint.Height() << 1 );
	rectText.top -= 1;

	pDC->SetTextColor( _WHITE );
	pDC->SetBkMode( TRANSPARENT );
	pDC->DrawText( strText, rectText, DT_LEFT | DT_SINGLELINE );

	ReleaseDC( pDC );
}

void CDlgWizardPM_RightViewInput_Base::VerifyFluidCharacteristic( CExtButton *pclButton )
{
	// Verify also fluid characteristic.
	// Remark: 'bEnable' is true if there is no error with the fluid characteristic.
	bool bEnable;
	CString strMsg;
	VerifyFluidCharacteristicHelper( bEnable, strMsg );

	if( NULL == pclButton )
	{
		return;
	}

	if( true == bEnable )
	{
		pclButton->ResetDrawBorder();
		m_ToolTip.DelTool( pclButton );
	}
	else
	{
		pclButton->SetDrawBorder( true, _RED );
		m_ToolTip.AddToolWindow( pclButton, strMsg );
	}
}

void CDlgWizardPM_RightViewInput_Base::VerifyFluidCharacteristicHelper( bool &bEnable, CString &strMsg )
{
	( (CRViewWizardSelPM*)m_pclParent )->VerifyFluidCharacteristic( bEnable, strMsg );
}
