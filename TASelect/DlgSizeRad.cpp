#include "stdafx.h"
#include "float.h"
#include "math.h"

#include "TASelect.h"
#include "MainFrm.h"
#include "Hydronic.h"
#include "DlgAddNC.h"

#include "RViewSSelSS.h"

#include "DlgSelectionBase.h"
#include "DlgIndSelBase.h"
#include "DlgIndSelTrv.h"

#include "DlgSizeRad.h"

#ifdef _DEBUG
#define new DEBUG_NEW
#undef THIS_FILE
static char THIS_FILE[] = __FILE__;
#endif


#define _DEFAULT_N		1.3
#define _DEFAULT_PHI	0.25

const int PixelShift = 3;

CDlgSizeRad::CDlgSizeRad( CIndSelTRVParams *pclIndSelTrvParams, CWnd *pParent )
	: CDialogEx( CDlgSizeRad::IDD, pParent )
{
	m_pclIndSelTRVParams = pclIndSelTrvParams;
	_Init();
}

BEGIN_MESSAGE_MAP( CDlgSizeRad, CDialogEx )
	ON_WM_CLOSE()
	ON_WM_MOVE()
	ON_WM_PAINT()
	ON_MESSAGE( WM_USER_UNITCHANGE, OnUnitChange )
	ON_BN_CLICKED( IDC_BUTTONCALCRAD, OnBnClickedCalculateRecommendedPower )
	ON_BN_CLICKED( IDC_BUTTONCALCFLOW, OnBnClickedCalculateRequiredFlow )
	ON_BN_CLICKED( IDC_BUTTONADD, OnBnClickedAddNominalConditions )
	ON_BN_CLICKED( IDC_BUTTONDELETE, OnBnClickedDeleteNominalConditions )
	ON_BN_CLICKED( IDC_BUTTONQ2SHEET, OnBnClickedQDpReady )
	ON_EN_KILLFOCUS( IDC_EDITAVDP, OnKillFocusEditAvailableDp )
	ON_EN_KILLFOCUS( IDC_EDITHEATOUT, OnKillFocusEditRequiredHeatOut )
	ON_EN_KILLFOCUS( IDC_EDITTI, OnKillFocusEditRoomTemperature )
	ON_EN_KILLFOCUS( IDC_EDITTS, OnKillFocusEditSupplyTemperature )
	ON_EN_KILLFOCUS( IDC_EDITREQDT, OnKillfocusEditRequiredDT )
	ON_EN_KILLFOCUS( IDC_EDITINSTPOW, OnKillFocusEditInstalledPower )
	ON_CBN_SELCHANGE( IDC_COMBONOMCOND, OnCbnSelChangeNominalConditions )
	ON_EN_CHANGE( IDC_EDITAVDP, OnChangeEditAvailableDp )
	ON_EN_CHANGE( IDC_EDITHEATOUT, OnChangeEditRequiredHeatOut )
	ON_EN_CHANGE( IDC_EDITINSTPOW, OnChangeEditInstalledPower )
	ON_EN_CHANGE( IDC_EDITREQDT, OnChangeEditRequiredDT )
	ON_EN_CHANGE( IDC_EDITTI, OnChangeEditRoomTemperature )
	ON_EN_CHANGE( IDC_EDITTS, OnChangeEditSupplyTemperature )
	ON_BN_CLICKED( IDC_CHECKAVDP, OnBnCheckClickedAvailableDp )
END_MESSAGE_MAP()

void CDlgSizeRad::DoDataExchange( CDataExchange *pDX )
{
	CDialogEx::DoDataExchange( pDX );
	DDX_Control( pDX, IDC_CHECKAVDP, m_CheckAvDp );
	DDX_Control( pDX, IDC_COMBONOMCOND, m_ComboNomCond );
	DDX_Control( pDX, IDC_EDITAVDP, m_EditAvDp );
	DDX_Control( pDX, IDC_STATICRADINFOBMP, m_StaticRadInfoBmp );
	DDX_Control( pDX, IDSUMMARY, m_ButtonSummary );
	DDX_Control( pDX, IDC_GROUPINPUT, m_GroupNote );
	DDX_Control( pDX, IDC_GROUPNOMCOND, m_GroupRadCond );
	DDX_Control( pDX, IDC_GROUPRAD, m_GroupRad );
	DDX_Control( pDX, IDC_EDITTS, m_EditTs );
	DDX_Control( pDX, IDC_EDITTR, m_EditTr );
	DDX_Control( pDX, IDC_EDITTI, m_EditTi );
	DDX_Control( pDX, IDC_EDITREQDT, m_EditReqdT );
	DDX_Control( pDX, IDC_EDITRECPOW, m_EditRecPow );
	DDX_Control( pDX, IDC_EDITMINPOW, m_EditMinPow );
	DDX_Control( pDX, IDC_EDITMINDT, m_EditMindT );
	DDX_Control( pDX, IDC_EDITINSTPOW, m_EditInstPow );
	DDX_Control( pDX, IDC_EDITHEATOUT, m_EditHeatOut );
	DDX_Control( pDX, IDC_EDITFLOW, m_EditFlow );
	DDX_Control( pDX, IDC_BUTTONQ2SHEET, m_ButtonQ2Sheet );
	DDX_Control( pDX, IDC_BUTTONDELETE, m_ButtonDelete );
	DDX_Control( pDX, IDC_BUTTONCALCRAD, m_ButtonCalcRad );
	DDX_Control( pDX, IDC_BUTTONCALCFLOW, m_ButtonCalcFlow );
	DDX_Control( pDX, IDC_BUTTONADD, m_ButtonAdd );
}

BOOL CDlgSizeRad::OnInitDialog() 
{
	if( NULL == m_pclIndSelTRVParams || NULL == m_pclIndSelTRVParams->m_pTADS || NULL == m_pclIndSelTRVParams->m_pTADS->GetpTechParams() )
	{
		ASSERTA_RETURN( FALSE );
	}

	CDialogEx::OnInitDialog();
	
	m_pUSERDB = TASApp.GetpUserDB();

	CString str;
	str = TASApp.LoadLocalizedString( IDS_DLGSIZERAD_CAPTION);
	SetWindowText( str );
	str = TASApp.LoadLocalizedString( IDS_DLGSIZERAD_STATICHEATOUT) ;
	GetDlgItem( IDC_STATICHEATOUT)->SetWindowText( str );
	str = TASApp.LoadLocalizedString( IDS_DLGSIZERAD_STATICTI );
	GetDlgItem( IDC_STATICTI)->SetWindowText( str );
	str = TASApp.LoadLocalizedString( IDS_DLGSIZERAD_STATICTS );
	GetDlgItem( IDC_STATICTS)->SetWindowText( str );
	str = TASApp.LoadLocalizedString( IDS_DLGSIZERAD_STATICNOMCOND );
	GetDlgItem( IDC_STATICNOMCOND)->SetWindowText( str );
	str = TASApp.LoadLocalizedString( IDS_DLGSIZERAD_STATICREQDT );
	GetDlgItem( IDC_STATICREQDT)->SetWindowText( str );
	str = TASApp.LoadLocalizedString( IDS_DLGSIZERAD_STATICMINDT );
	GetDlgItem( IDC_STATICMINDT)->SetWindowText( str );
	str = TASApp.LoadLocalizedString( IDS_DLGSIZERAD_STATICMINPOW );
	GetDlgItem( IDC_STATICMINPOW)->SetWindowText( str );
	str = TASApp.LoadLocalizedString( IDS_DLGSIZERAD_STATICRECPOW );
	GetDlgItem( IDC_STATICRECPOW)->SetWindowText( str );
	str = TASApp.LoadLocalizedString( IDS_DLGSIZERAD_STATICINSTPOW );
	GetDlgItem( IDC_STATICINSTPOW)->SetWindowText( str );
	str = TASApp.LoadLocalizedString( IDS_DLGSIZERAD_STATICFLOW );
	GetDlgItem( IDC_STATICFLOW)->SetWindowText( str );
	str = TASApp.LoadLocalizedString( IDS_DLGSIZERAD_STATICTR );
	GetDlgItem( IDC_STATICTR)->SetWindowText( str );
	str = TASApp.LoadLocalizedString( IDS_DLGSIZERAD_SUMMARY );
	GetDlgItem(IDSUMMARY)->SetWindowText( str );
	str = TASApp.LoadLocalizedString( IDS_DLGSIZERAD_CLOSE );
	GetDlgItem(IDCANCEL)->SetWindowText( str );
	str = TASApp.LoadLocalizedString( IDS_DLGSIZERAD_CHECKAVDP );
	m_CheckAvDp.SetWindowText( str );

	// Load Bitmaps.
	CImageList *pclImgListGroupBox = TASApp.GetpRCImageManager()->GetImageList( CRCImageManager::ILN_GroupBox );
	ASSERT ( NULL != pclImgListGroupBox );

	if( NULL != pclImgListGroupBox )
	{
		m_GroupNote.SetImageList( pclImgListGroupBox, CRCImageManager::ILGB_Note );
		m_GroupRadCond.SetImageList( pclImgListGroupBox, CRCImageManager::ILGB_RadCond );
		m_GroupRad.SetImageList( pclImgListGroupBox, CRCImageManager::ILGB_Rad );
	}
	
	CImageList *pclImgListButton = TASApp.GetpRCImageManager()->GetImageList( CRCImageManager::ILN_Button );
	ASSERT ( NULL != pclImgListButton );

	if( NULL != pclImgListButton )
	{
		m_ButtonAdd.SetIcon( (HICON)pclImgListButton->ExtractIcon( CRCImageManager::ILB_SaveTempCond ) );
		m_ButtonDelete.SetIcon( (HICON)pclImgListButton->ExtractIcon( CRCImageManager::ILB_TrashBin ) );
		m_ButtonCalcRad.SetIcon( (HICON)pclImgListButton->ExtractIcon( CRCImageManager::ILB_Calc1 ) );
		m_ButtonCalcFlow.SetIcon( (HICON)pclImgListButton->ExtractIcon( CRCImageManager::ILB_Calc2 ) );
		m_ButtonQ2Sheet.SetIcon( (HICON)pclImgListButton->ExtractIcon( CRCImageManager::ILB_CopyQDp ) );
	}

	// Create a ToolTipCtrl .
	m_ToolTip.Create( this, TTS_NOPREFIX );

	// Add a tooltip for each button.
	CString TTstr;
	TTstr = TASApp.LoadLocalizedString( IDS_TOOLTIP_CALCRECPOW );
	m_ToolTip.AddToolWindow( GetDlgItem( IDC_BUTTONCALCRAD ), TTstr );
	TTstr = TASApp.LoadLocalizedString( IDS_TOOLTIP_CALCREQFLOW );
	m_ToolTip.AddToolWindow( GetDlgItem( IDC_BUTTONCALCFLOW ), TTstr );
	TTstr = TASApp.LoadLocalizedString( IDS_TOOLTIP_COPYQDP );
	m_ToolTip.AddToolWindow( GetDlgItem( IDC_BUTTONQ2SHEET ), TTstr );
	TTstr = TASApp.LoadLocalizedString( IDS_TOOLTIP_ADDNOMCOND );
	m_ToolTip.AddToolWindow( GetDlgItem( IDC_BUTTONADD ), TTstr );
	TTstr = TASApp.LoadLocalizedString( IDS_TOOLTIP_DELNOMCOND );
	m_ToolTip.AddToolWindow( GetDlgItem( IDC_BUTTONDELETE ), TTstr );

	// Clear all Edit boxes and uncheck AvDp check box.
	ResetResults();
	m_CheckAvDp.SetCheck( 1 );
	m_pclIndSelTRVParams->m_RadInfos.bAvailableDpChecked = true;

	// Load default values in Dp, Ti and Ts Edit boxes.
	CDS_TechnicalParameter *pTechP = m_pclIndSelTRVParams->m_pTADS->GetpTechParams();
	
	// // HYS-1305 : Set unit parameters before call update() funtion
	m_EditAvDp.SetEditType( CNumString::eEditType::eDouble, CNumString::eEditSign::ePositive );
	m_EditHeatOut.SetEditType( CNumString::eEditType::eDouble, CNumString::eEditSign::ePositive );
	m_EditTi.SetEditType( CNumString::eEditType::eDouble, CNumString::eEditSign::ePositive );
	m_EditTs.SetEditType( CNumString::eEditType::eDouble, CNumString::eEditSign::ePositive );

	RefreshUnits();

	m_EditAvDp.SetCurrentValSI( pTechP->GetTrvDefDpTot() );
	m_EditAvDp.Update();

	m_EditTi.SetCurrentValSI( pTechP->GetTrvDefRoomT() );
	m_EditTi.Update();

	m_EditTs.SetCurrentValSI( pTechP->GetTrvDefSupplyT() );
	m_EditTs.Update();
	
	OnKillFocusEditAvailableDp();
	OnKillFocusEditRoomTemperature();
	OnKillFocusEditSupplyTemperature();


	// Set the window position to the last stored position in registry.
	// If window position is not yet stored in the registry, the window is centered by default in the mainframe rect.
	CRect apprect, rect;
	::AfxGetApp()->m_pMainWnd->GetWindowRect( &apprect );
	GetWindowRect( &rect );
	int x = ::AfxGetApp()->GetProfileInt( m_strSectionName, _T("ScreenXPos"), apprect.CenterPoint().x - rect.Width() / 2 );
	int y = ::AfxGetApp()->GetProfileInt( m_strSectionName, _T("ScreenYPos"), apprect.CenterPoint().y - rect.Height() / 2 );
	SetWindowPos( NULL, x, y, 0, 0, SWP_NOZORDER | SWP_NOSIZE );

	// If the window is placed on a screen that is currently deactivated, the windows is centered to the application.
	HMONITOR hMonitor = NULL;
	GetWindowRect( &rect );
	hMonitor = MonitorFromRect(&rect, MONITOR_DEFAULTTONULL);

	if( NULL == hMonitor )
	{
		CenterWindow();
	}

	// Initialize the content of NC combo box.
	m_pNC = NULL;
	InitComboNC();	

	// Initialize the 'Available Dp' checkbox.
	if( m_pclIndSelTRVParams->m_dDp <= 0.0 )
	{
		m_CheckAvDp.SetCheck( FALSE );
	}
	else
	{
		m_pclIndSelTRVParams->m_RadInfos.dAvailableDp = m_pclIndSelTRVParams->m_dDp;
		m_CheckAvDp.SetCheck( m_pclIndSelTRVParams->m_bDpEnabled );
	}

	OnBnCheckClickedAvailableDp();
	
	m_bInitialized = true;

	return TRUE;  
}

void CDlgSizeRad::RefreshUnits()
{
	// Initialize All used units.
	CUnitDatabase *pclUnitDB = CDimValue::AccessUDB();

	if( NULL == pclUnitDB )
	{
		ASSERT_RETURN;
	}

	TCHAR tcName[_MAXCHARS];

	// Power.
	GetNameOf( pclUnitDB->GetUnit( _U_TH_POWER, pclUnitDB->GetDefaultUnitIndex( _U_TH_POWER ) ), tcName );
	GetDlgItem( IDC_STATICUNITHEATOUT )->SetWindowText( tcName );
	GetDlgItem( IDC_STATICUNITMINPOW )->SetWindowText( tcName );
	GetDlgItem( IDC_STATICUNITRECPOW )->SetWindowText( tcName );
	GetDlgItem( IDC_STATICUNITINSTPOW )->SetWindowText( tcName );

	// Temperature.
	GetNameOf( pclUnitDB->GetUnit( _U_TEMPERATURE, pclUnitDB->GetDefaultUnitIndex( _U_TEMPERATURE ) ), tcName );
	GetDlgItem( IDC_STATICUNITTI )->SetWindowText( tcName );
	GetDlgItem( IDC_STATICUNITTS )->SetWindowText( tcName );
	GetDlgItem( IDC_STATICUNITTR )->SetWindowText( tcName );
	GetDlgItem( IDC_STATICUNITREQDT )->SetWindowText( tcName );
	GetDlgItem( IDC_STATICUNITMINDT )->SetWindowText( tcName );
	GetDlgItem( IDC_STATICUNITNOMCOND )->SetWindowText( tcName );

	// Flow.
	GetNameOf( pclUnitDB->GetUnit( _U_FLOW, pclUnitDB->GetDefaultUnitIndex( _U_FLOW ) ), tcName );
	GetDlgItem( IDC_STATICUNITFLOW )->SetWindowText( tcName );

	// Pressure.
	GetNameOf( pclUnitDB->GetUnit( _U_DIFFPRESS, pclUnitDB->GetDefaultUnitIndex( _U_DIFFPRESS ) ), tcName );
	GetDlgItem( IDC_STATICUNITAVDP )->SetWindowText( tcName );

	m_EditAvDp.SetPhysicalType( _U_DIFFPRESS );
	m_EditAvDp.SetMinDblValue( 0 );
	m_EditAvDp.Update();

	m_EditHeatOut.SetPhysicalType( _U_TH_POWER );
	m_EditHeatOut.Update();

	m_EditTi.SetPhysicalType( _U_TEMPERATURE );
	m_EditTi.Update();

	m_EditTs.SetPhysicalType( _U_TEMPERATURE );
	m_EditTs.Update();
}

void CDlgSizeRad::OnMove( int x, int y ) 
{
	CDialogEx::OnMove( x, y );
	
	// Modeless case -- stores window position in registry.
	if( NULL != m_pParent && true == m_bInitialized )
	{
		CRect rect;
		GetWindowRect( &rect );
		::AfxGetApp()->WriteProfileInt( m_strSectionName, _T("ScreenXPos"), x );
		::AfxGetApp()->WriteProfileInt( m_strSectionName, _T("ScreenYPos"), y );
	}
}

void CDlgSizeRad::OnPaint() 
{
	CPaintDC dc( this );
	
	// Get client area of the dialog.
	CRect rect;
	GetClientRect( &rect );

	// User CStatic control like reference, to keep a trace of resizing control when the user select large fonts.
	m_StaticRadInfoBmp.GetWindowRect( &rect );
	ScreenToClient( &rect );

	// Deflate rect to adjust the painting area and paint a sunken border.
	rect.DeflateRect( PixelShift, 1, 1, 1 );
	dc.DrawEdge( rect, BDR_SUNKENOUTER, BF_TOPLEFT );
	dc.DrawEdge( rect, BDR_SUNKENOUTER, BF_BOTTOMRIGHT );
	
	// Deflate to have the painting area within the border.
	rect.DeflateRect( 1, 1, 1, 1 );

	// Fill rect with a white rectangle.
	dc.SelectStockObject( WHITE_BRUSH );
	dc.Rectangle( &rect );

	// Load bitmap and get dimensions of the bitmap.
	CBitmap bmp;
	bmp.LoadMappedBitmap( IDB_RADINFO );

	BITMAP bmpInfo;
	bmp.GetBitmap( &bmpInfo );

	// Draw bitmap in the center of CStatic control.
	CDC dcTmp;
	dcTmp.CreateCompatibleDC( &dc );
	dcTmp.SelectObject( &bmp );
	int iXOffset, iYOffset;
	iXOffset = ( rect.Width() - bmpInfo.bmWidth ) / 2;
	iYOffset = ( rect.Height() - bmpInfo.bmHeight ) / 2;
	dc.BitBlt( rect.left + iXOffset, rect.top + iYOffset, bmpInfo.bmWidth, bmpInfo.bmHeight, &dcTmp, 0, 0, SRCCOPY );
}

LRESULT CDlgSizeRad::OnUnitChange( WPARAM wParam, LPARAM lParam )
{
	RefreshUnits();

	if( CB_ERR != m_ComboNomCond.GetCurSel() )
	{
		CString SelectedItem( _T("") );
		m_ComboNomCond.GetLBText( m_ComboNomCond.GetCurSel(), SelectedItem.GetBuffer(256) );
		InitComboNC( SelectedItem );
	}

	ResetResults();

	return 0;
}

void CDlgSizeRad::OnBnClickedCalculateRecommendedPower() 
{
	if( NULL == m_pclIndSelTRVParams || NULL == m_pclIndSelTRVParams->m_pTADS || NULL == m_pclIndSelTRVParams->m_pTADS->GetpTechParams() )
	{
		ASSERT_RETURN;
	}

	CDS_TechnicalParameter *pclTechParams = m_pclIndSelTRVParams->m_pTADS->GetpTechParams();
	double dRho = m_pclIndSelTRVParams->m_WC.GetDens();
	double dCp = m_pclIndSelTRVParams->m_WC.GetSpecifHeat();
	
	// Calculate the maximum flow one can have.
	double dQmax = -1.0; 
	
	if( true == m_pclIndSelTRVParams->m_RadInfos.bAvailableDpChecked && ( m_pclIndSelTRVParams->m_RadInfos.dAvailableDp <= 0.0 ) )
	{
		return;
	}
	else
	{
		// Calculate the max. resulting Kvs for supply and return valves.
		double dHighestTrvKvs = 0.0;
		double dHighestRvKvs = 0.0;
		double dDummy = 0.0;
		double dKvsTot = 0.0;

		FindKvsBounds( &dDummy, &dHighestTrvKvs, &dDummy, &dHighestRvKvs );
		
		// 1 / KvTot^2 = 1/KvTrv^2 + 1/KvRv^2

		if( dHighestTrvKvs > 0.0 )
		{
			dKvsTot += 1.0 / ( dHighestTrvKvs * dHighestTrvKvs );
		}

		if( dHighestRvKvs > 0.0 )
		{
			dKvsTot += 1.0 / ( dHighestRvKvs * dHighestRvKvs );
		}

		if( dKvsTot > 0.0 )
		{
			dKvsTot = 1.0 / sqrt( dKvsTot );
		}

		// Compute the maximum flow.
		if( false == m_pclIndSelTRVParams->m_RadInfos.bAvailableDpChecked )
		{
			m_pclIndSelTRVParams->m_RadInfos.dAvailableDp = pclTechParams->GetTrvMaxDp();
		}

		dQmax = CalcqT( dKvsTot, m_pclIndSelTRVParams->m_RadInfos.dAvailableDp, dRho );

		// HYS-1305 : Add insert with automatic flow limiter
		if( ( RVT_WithFlowLimitation == m_pclIndSelTRVParams->m_eValveType )
			|| ( true == m_pclIndSelTRVParams->m_bIsFLCVInsert ) )
		{
			if( dQmax > 0.0 )
			{
				dQmax = min( dQmax, GetMaxQWithFlowLimitation() );
			}
			else
			{
				dQmax = GetMaxQWithFlowLimitation();
			}
		} 
	}

	// Compute the minimum power to be installed.
	if( ( m_pclIndSelTRVParams->m_RadInfos.dRequiredHeatOutput <= 0.0 ) || ( m_pclIndSelTRVParams->m_RadInfos.dRoomTemperature <= 0.0 ) 
			|| ( m_pclIndSelTRVParams->m_RadInfos.dSupplyTemperature <= 0.0 ) )
	{
		return;
	}
	else
	{
		// Check if the design power is deliverable in actual condition.
		// i.e. if required P is less than Pmax = Qmax*dTmax.
		double dP;
		double dTmax = m_pclIndSelTRVParams->m_RadInfos.dSupplyTemperature - m_pclIndSelTRVParams->m_RadInfos.dRoomTemperature;
		dP = CalcP( dQmax, dTmax, dRho, dCp );
		
		if( m_pclIndSelTRVParams->m_RadInfos.dRequiredHeatOutput > dP )
		{
			TASApp.AfxLocalizeMessageBox( AFXMSG_SUPPLYT_TOO_LOW );
			m_EditTs.SetSel( 0, -1 );
			m_EditTs.SetFocus();
			return;
		}
		
		// Compute Minimum dT.
		ASSERT( dQmax > 0.0 );
		m_pclIndSelTRVParams->m_RadInfos.dMinDT = CalcDT( m_pclIndSelTRVParams->m_RadInfos.dRequiredHeatOutput, dQmax, dRho, dCp );
		
		// Compute Min P.
		m_pclIndSelTRVParams->m_RadInfos.dMinPower = m_pclIndSelTRVParams->m_RadInfos.dRequiredHeatOutput * pow( ( m_pNC->GetTs() - m_pNC->GetTi() ) * ( m_pNC->GetTr() - m_pNC->GetTi() ) /
				( ( m_pclIndSelTRVParams->m_RadInfos.dSupplyTemperature - m_pclIndSelTRVParams->m_RadInfos.dRoomTemperature ) *
				( m_pclIndSelTRVParams->m_RadInfos.dSupplyTemperature - m_pclIndSelTRVParams->m_RadInfos.dMinDT - m_pclIndSelTRVParams->m_RadInfos.dRoomTemperature ) ), 0.5 * _DEFAULT_N );
	}

	// Set default requested dT.
	if( m_pclIndSelTRVParams->m_RadInfos.dRequiredDT <= 0.0 )
	{
		ASSERT( m_pclIndSelTRVParams->m_RadInfos.dSupplyTemperature > 0.0 && m_pclIndSelTRVParams->m_RadInfos.dRoomTemperature > 0.0 );
		m_pclIndSelTRVParams->m_RadInfos.dRequiredDT = _DEFAULT_PHI * ( m_pclIndSelTRVParams->m_RadInfos.dSupplyTemperature - m_pclIndSelTRVParams->m_RadInfos.dRoomTemperature );
	}
	
	ASSERT( m_pclIndSelTRVParams->m_RadInfos.dRequiredDT < ( m_pclIndSelTRVParams->m_RadInfos.dSupplyTemperature - m_pclIndSelTRVParams->m_RadInfos.dRoomTemperature ) );

	// Compute the minimum power according to the required dT.
	if( m_pclIndSelTRVParams->m_RadInfos.dRequiredDT > 0.0 )
	{
		// Check that the required dT is high enough.
		if( m_pclIndSelTRVParams->m_RadInfos.dRequiredDT < m_pclIndSelTRVParams->m_RadInfos.dMinDT )
		{
			m_pclIndSelTRVParams->m_RadInfos.dRequiredDT = m_pclIndSelTRVParams->m_RadInfos.dMinDT;
		}

		// Compute Min P.
		m_pclIndSelTRVParams->m_RadInfos.dMinPower = m_pclIndSelTRVParams->m_RadInfos.dRequiredHeatOutput * pow( ( m_pNC->GetTs() - m_pNC->GetTi() ) * ( m_pNC->GetTr() - m_pNC->GetTi() ) /
				( ( m_pclIndSelTRVParams->m_RadInfos.dSupplyTemperature - m_pclIndSelTRVParams->m_RadInfos.dRoomTemperature ) *
				( m_pclIndSelTRVParams->m_RadInfos.dSupplyTemperature - m_pclIndSelTRVParams->m_RadInfos.dRequiredDT - m_pclIndSelTRVParams->m_RadInfos.dRoomTemperature ) ), 0.5 * _DEFAULT_N );
	}

	// Compute the recommended power.
	ASSERT( m_pclIndSelTRVParams->m_RadInfos.dRequiredHeatOutput > 0.0 );
	ASSERT( ( m_pclIndSelTRVParams->m_RadInfos.dSupplyTemperature - m_pclIndSelTRVParams->m_RadInfos.dRoomTemperature ) > 0.0 );
	
	if( ( m_pclIndSelTRVParams->m_RadInfos.dRequiredHeatOutput / ( dRho * dCp * dQmax * ( m_pclIndSelTRVParams->m_RadInfos.dSupplyTemperature - m_pclIndSelTRVParams->m_RadInfos.dRoomTemperature ) ) ) >= _DEFAULT_PHI )
	{
		// The minimum power is already high enough
		m_pclIndSelTRVParams->m_RadInfos.dRecommendedPower = m_pclIndSelTRVParams->m_RadInfos.dMinPower;
	}
	else
	{
		m_pclIndSelTRVParams->m_RadInfos.dRecommendedPower = m_pclIndSelTRVParams->m_RadInfos.dRequiredHeatOutput * pow( ( m_pNC->GetTs() - m_pNC->GetTi() ) * ( m_pNC->GetTr() - m_pNC->GetTi() ) /
				( ( m_pclIndSelTRVParams->m_RadInfos.dSupplyTemperature - m_pclIndSelTRVParams->m_RadInfos.dRoomTemperature ) * 
				( m_pclIndSelTRVParams->m_RadInfos.dSupplyTemperature - m_pclIndSelTRVParams->m_RadInfos.dRoomTemperature) * ( 1.0 - _DEFAULT_PHI ) ), 0.5 * _DEFAULT_N );
		
		if( m_pclIndSelTRVParams->m_RadInfos.dRecommendedPower < m_pclIndSelTRVParams->m_RadInfos.dMinPower )
		{
			m_pclIndSelTRVParams->m_RadInfos.dRecommendedPower = m_pclIndSelTRVParams->m_RadInfos.dMinPower;
		}
	}

	// Display the calculated results.
	DisplayRadResults();
}

void CDlgSizeRad::OnBnClickedCalculateRequiredFlow() 
{
	if( NULL == m_pclIndSelTRVParams )
	{
		ASSERT_RETURN;
	}

	double dRho = m_pclIndSelTRVParams->m_WC.GetDens();
	double dCp = m_pclIndSelTRVParams->m_WC.GetSpecifHeat();
	
	// If a radiator power is available, compute the flow and all the temperatures.
	if( m_pclIndSelTRVParams->m_RadInfos.dInstalledPower > 0.0)
	{
		// Check if installed power is high enough.
		if( m_pclIndSelTRVParams->m_RadInfos.dInstalledPower < m_pclIndSelTRVParams->m_RadInfos.dMinPower )
		{
			// The installed power is not high enough.
			TASApp.AfxLocalizeMessageBox( AFXMSG_TOO_LOW_POWER );
			m_EditInstPow.SetSel( 0, -1 );
			m_EditInstPow.SetFocus();
			return;
		}
		
		// Compute radiator return temperature.
		m_pclIndSelTRVParams->m_RadInfos.dReturnTemperature = m_pclIndSelTRVParams->m_RadInfos.dRoomTemperature + pow( m_pclIndSelTRVParams->m_RadInfos.dRequiredHeatOutput / m_pclIndSelTRVParams->m_RadInfos.dInstalledPower, 2.0 / _DEFAULT_N ) * 
				( m_pNC->GetTs() - m_pNC->GetTi() ) * ( m_pNC->GetTr() - m_pNC->GetTi() ) / 
				( m_pclIndSelTRVParams->m_RadInfos.dSupplyTemperature - m_pclIndSelTRVParams->m_RadInfos.dRoomTemperature );

		// Calculate the flow to be adjusted.
		m_pclIndSelTRVParams->m_RadInfos.dFlow = m_pclIndSelTRVParams->m_RadInfos.dRequiredHeatOutput 
				/ ( dRho * dCp * ( m_pclIndSelTRVParams->m_RadInfos.dSupplyTemperature - m_pclIndSelTRVParams->m_RadInfos.dReturnTemperature ) );
	}
	else
	{
		return;
	}

	// Display the calculated results.
	DisplayFlowResults();
}

void CDlgSizeRad::OnBnClickedAddNominalConditions() 
{
	CDlgAddNC dlg;
	
	if( IDOK == dlg.Display() )
	{
		InitComboNC( dlg.GetRadNCID() );
	}
}

void CDlgSizeRad::OnBnClickedDeleteNominalConditions() 
{
	if( NULL == m_pNC )
	{
		return;				
	}
	
	// Warning message.
	CString str, str1;
	FormatString( str, AFXMSG_DELETEITEM, GetStringFromNC( m_pNC ) );

	if( IDYES != AfxMessageBox( str, MB_YESNO | MB_ICONQUESTION, 0 ) )
	{
		return;
	}
	
	// Delete object.
	CTable *pNCTab = (CTable*)( m_pUSERDB->Get( _T("RADNOMCOND_TAB") ).MP );
	ASSERT( NULL != pNCTab );

	IDPTR idptrNC = m_pNC->GetIDPtr();
	pNCTab->Remove( idptrNC );
	m_pUSERDB->DeleteObject( idptrNC );
	InitComboNC();
}

void CDlgSizeRad::OnBnClickedQDpReady() 
{
	OnOK();
}

void CDlgSizeRad::OnKillFocusEditAvailableDp() 
{
	double dValue;
	ReadDoubleReturn_enum rd = ReadDouble( m_EditAvDp, &dValue );
	
	if( RD_EMPTY == rd )
	{
		m_pclIndSelTRVParams->m_RadInfos.dAvailableDp = 0.0;
		return;
	}
	else if( RD_NOT_NUMBER == rd || ( RD_OK == rd && dValue < 0.0 ) )
	{
		TASApp.AfxLocalizeMessageBox( AFXMSG_NON_POSITIVE_VALUE );
		m_EditAvDp.SetSel( 0, -1 );
		m_EditAvDp.SetFocus();
		return;
	}
	
	dValue = CDimValue::CUtoSI( _U_DIFFPRESS, dValue );
	
	if( dValue != m_pclIndSelTRVParams->m_RadInfos.dAvailableDp )
	{
		ResetRadResults();
	}

	// Update member.
	m_pclIndSelTRVParams->m_RadInfos.dAvailableDp = dValue;
}

void CDlgSizeRad::OnKillFocusEditRequiredHeatOut() 
{
	double dValue;
	ReadDoubleReturn_enum rd = ReadDouble( m_EditHeatOut, &dValue );
	
	if( RD_EMPTY == rd )
	{
		m_pclIndSelTRVParams->m_RadInfos.dRequiredHeatOutput = 0.0;
		return;
	}
	else if( RD_NOT_NUMBER == rd || ( RD_OK == rd && dValue < 0.0 ) )
	{
		TASApp.AfxLocalizeMessageBox( AFXMSG_NON_POSITIVE_VALUE );
		m_EditHeatOut.SetSel( 0, -1 );
		m_EditHeatOut.SetFocus();
		return;
	}
	
	dValue = CDimValue::CUtoSI( _U_TH_POWER, dValue );
	
	if( dValue != m_pclIndSelTRVParams->m_RadInfos.dRequiredHeatOutput )
	{
		ResetRadResults();
	}
	
	// Update member.
	m_pclIndSelTRVParams->m_RadInfos.dRequiredHeatOutput = dValue;
}

void CDlgSizeRad::OnKillFocusEditRoomTemperature() 
{
	double dValue;
	ReadDoubleReturn_enum rd = ReadDouble( m_EditTi, &dValue );
	
	if( RD_EMPTY == rd )
	{
		m_pclIndSelTRVParams->m_RadInfos.dRoomTemperature = 0.0;
		return;
	}
	else if( RD_NOT_NUMBER == rd || ( RD_OK == rd && dValue < 0.0 ) )
	{
		TASApp.AfxLocalizeMessageBox( AFXMSG_NON_POSITIVE_VALUE );
		m_EditTi.SetSel( 0, -1 );
		m_EditTi.SetFocus();
		return;
	}
	
	dValue = CDimValue::CUtoSI( _U_TEMPERATURE, dValue );
	
	if( dValue != m_pclIndSelTRVParams->m_RadInfos.dRoomTemperature )
	{
		ResetRadResults();
	}
	
	// Update member.
	m_pclIndSelTRVParams->m_RadInfos.dRoomTemperature = dValue;
}

void CDlgSizeRad::OnKillFocusEditSupplyTemperature() 
{
	double dValue;
	ReadDoubleReturn_enum rd = ReadDouble( m_EditTs, &dValue );
	
	if( RD_EMPTY == rd )
	{
		m_pclIndSelTRVParams->m_RadInfos.dSupplyTemperature = 0.0;
		return;
	}
	else if( RD_NOT_NUMBER == rd || ( RD_OK == rd && dValue < 0.0 ) )
	{
		TASApp.AfxLocalizeMessageBox( AFXMSG_NON_POSITIVE_VALUE );
		m_EditTs.SetSel( 0, -1 );
		m_EditTs.SetFocus();
		return;
	}
	
	dValue = CDimValue::CUtoSI( _U_TEMPERATURE, dValue );
	
	if( dValue != m_pclIndSelTRVParams->m_RadInfos.dSupplyTemperature )
	{
		ResetRadResults();
	}
	
	// Update member.
	m_pclIndSelTRVParams->m_RadInfos.dSupplyTemperature = dValue;
}

void CDlgSizeRad::OnKillfocusEditRequiredDT() 
{
	double dValue;
	ReadDoubleReturn_enum rd = ReadDouble( m_EditReqdT, &dValue );
	
	if( RD_EMPTY == rd )
	{
		m_pclIndSelTRVParams->m_RadInfos.dRequiredDT = 0.0;
		return;
	}
	else if( RD_NOT_NUMBER == rd || ( RD_OK == rd && dValue < 0.0 ) )
	{
		TASApp.AfxLocalizeMessageBox( AFXMSG_NON_POSITIVE_VALUE );
		m_EditReqdT.SetSel( 0, -1 );
		m_EditReqdT.SetFocus();
		return;
	}
	
	dValue = CDimValue::CUtoSI( _U_TEMPERATURE, dValue );
	
	if( dValue != m_pclIndSelTRVParams->m_RadInfos.dRequiredDT )
	{
		ResetRadResults( false );
	}
	
	// Update member.
	m_pclIndSelTRVParams->m_RadInfos.dRequiredDT = dValue;
}

void CDlgSizeRad::OnKillFocusEditInstalledPower()
{
	double dValue;
	ReadDoubleReturn_enum rd = ReadDouble( m_EditInstPow, &dValue );
	
	if( RD_EMPTY == rd )
	{
		m_pclIndSelTRVParams->m_RadInfos.dInstalledPower = 0.0;
		return;
	}
	else if( RD_NOT_NUMBER == rd || ( RD_OK == rd && dValue < 0.0 ) )
	{
		m_pclIndSelTRVParams->m_RadInfos.dInstalledPower = 0.0;
		TASApp.AfxLocalizeMessageBox( AFXMSG_NON_POSITIVE_VALUE );
		m_EditInstPow.SetSel( 0, -1 );
		m_EditInstPow.SetFocus();
		return;
	}
	
	dValue = CDimValue::CUtoSI( _U_TH_POWER, dValue );
	
	if( dValue != m_pclIndSelTRVParams->m_RadInfos.dInstalledPower )
	{
		ResetFlowResults();
	}
	
	// Update member.
	m_pclIndSelTRVParams->m_RadInfos.dInstalledPower = dValue;
}

void CDlgSizeRad::OnCbnSelChangeNominalConditions() 
{
	m_pNC = (CDB_RadNomCond *)m_ComboNomCond.GetItemDataPtr( m_ComboNomCond.GetCurSel() );
	
	if( true == m_pNC->IsFix() )
	{
		m_ButtonDelete.EnableWindow( FALSE );
	}
	else
	{
		m_ButtonDelete.EnableWindow( TRUE );
	}

	m_pclIndSelTRVParams->m_RadInfos.dNCRoomT = m_pNC->GetTi();
	m_pclIndSelTRVParams->m_RadInfos.dNCSupplyT = m_pNC->GetTs();
	m_pclIndSelTRVParams->m_RadInfos.dNCReturnT = m_pNC->GetTr();
	ResetFlowResults();
	ResetRadResults();
}

void CDlgSizeRad::OnChangeEditAvailableDp() 
{
	if( GetFocus() == &m_EditAvDp )
	{
		ResetFlowResults();
		ResetRadResults();
	}
}

void CDlgSizeRad::OnChangeEditRequiredHeatOut() 
{
	if( GetFocus() == &m_EditHeatOut )
	{
		ResetFlowResults();
		ResetRadResults();
	}
}

void CDlgSizeRad::OnChangeEditInstalledPower() 
{
	if( GetFocus() == &m_EditInstPow )
	{
		ResetFlowResults();
	}
}

void CDlgSizeRad::OnChangeEditRequiredDT() 
{
	if( GetFocus() == &m_EditReqdT )
	{
		ResetFlowResults();
		ResetRadResults( false );
	}
}

void CDlgSizeRad::OnChangeEditRoomTemperature()
{
	if( GetFocus() == &m_EditTi )
	{
		ResetFlowResults();
		ResetRadResults();
	}
}

void CDlgSizeRad::OnChangeEditSupplyTemperature() 
{
	if( GetFocus() == &m_EditTs )
	{
		ResetFlowResults();
		ResetRadResults();
	}
}

void CDlgSizeRad::OnBnCheckClickedAvailableDp() 
{
	ResetFlowResults();
	ResetRadResults();

	if( BST_CHECKED == m_CheckAvDp.GetCheck() )
	{
		// Dp edit box becomes active.
		m_pclIndSelTRVParams->m_RadInfos.bAvailableDpChecked = true;
		m_EditAvDp.SetReadOnly( FALSE );

		if( 0.0 == m_pclIndSelTRVParams->m_RadInfos.dAvailableDp )
		{
			// Take default value in the technical parameters.
			CDS_TechnicalParameter *pTechP = m_pclIndSelTRVParams->m_pTADS->GetpTechParams();
			m_pclIndSelTRVParams->m_RadInfos.dAvailableDp = pTechP->GetTrvDefDpTot();
		}
		
		m_EditAvDp.SetCurrentValSI( m_pclIndSelTRVParams->m_RadInfos.dAvailableDp );
		m_EditAvDp.Update();
		m_EditAvDp.EnableWindow( TRUE );
		GetDlgItem( IDC_STATICUNITAVDP )->ShowWindow( SW_SHOW );
	}
	else
	{
		// Dp edit box becomes inactive.
		m_pclIndSelTRVParams->m_RadInfos.bAvailableDpChecked = false;
		m_EditAvDp.SetReadOnly( TRUE );
		CString str;
		str = TASApp.LoadLocalizedString( IDS_UNKNOWN );
		m_EditAvDp.SetWindowText( str );
		m_EditAvDp.EnableWindow( FALSE );
		GetDlgItem( IDC_STATICUNITAVDP )->ShowWindow( SW_HIDE );
	}
}

void CDlgSizeRad::DisplayRadResults()
{
	m_EditMinPow.SetWindowText( (CString)WriteDouble( CDimValue::SItoCU( _U_TH_POWER, m_pclIndSelTRVParams->m_RadInfos.dMinPower ), 3, 0 ) );
	m_EditRecPow.SetWindowText( (CString)WriteDouble( CDimValue::SItoCU( _U_TH_POWER, m_pclIndSelTRVParams->m_RadInfos.dRecommendedPower ), 3, 0 ) );
	m_EditMindT.SetWindowText( (CString)WriteDouble( CDimValue::SItoCU( _U_TEMPERATURE, m_pclIndSelTRVParams->m_RadInfos.dMinDT ), 3, 0 ) );
	m_EditReqdT.SetWindowText( (CString)WriteDouble( CDimValue::SItoCU( _U_TEMPERATURE, m_pclIndSelTRVParams->m_RadInfos.dRequiredDT ), 3, 0 ) );
	m_ButtonCalcFlow.EnableWindow( TRUE );
}

void CDlgSizeRad::DisplayFlowResults()
{
	m_EditFlow.SetWindowText( (CString)WriteDouble( CDimValue::SItoCU( _U_FLOW, m_pclIndSelTRVParams->m_RadInfos.dFlow ), 3, 0 ) );
	m_EditTr.SetWindowText( (CString)WriteDouble( CDimValue::SItoCU( _U_TEMPERATURE, m_pclIndSelTRVParams->m_RadInfos.dReturnTemperature ), 3, 0 ) );
	m_ButtonQ2Sheet.EnableWindow( TRUE );
}

void CDlgSizeRad::ResetRadResults( bool bResetReqdT )
{
	m_EditMinPow.SetWindowText( _T("") );
	m_EditRecPow.SetWindowText( _T("") );

	if( true == bResetReqdT )
	{
		m_EditReqdT.SetWindowText( _T("") );
	}

	m_EditMindT.SetWindowText( _T("") );

	m_ButtonCalcRad.EnableWindow( TRUE );
	m_ButtonCalcFlow.EnableWindow( FALSE );
	m_ButtonQ2Sheet.EnableWindow( FALSE );
}

void CDlgSizeRad::ResetFlowResults()
{
	m_EditFlow.SetWindowText( _T("") );
	m_EditTr.SetWindowText( _T("") );

	m_ButtonCalcRad.EnableWindow( TRUE );
	m_ButtonCalcFlow.EnableWindow( TRUE );
	m_ButtonQ2Sheet.EnableWindow( FALSE );
}

void CDlgSizeRad::ResetResults()
{
	m_EditMinPow.SetWindowText( _T("") );
	m_EditRecPow.SetWindowText( _T("") );
	m_EditReqdT.SetWindowText( _T("") );
	m_EditMindT.SetWindowText( _T("") );
	m_EditInstPow.SetWindowText( _T("") );
	m_EditFlow.SetWindowText( _T("") );
	m_EditTr.SetWindowText( _T("") );

	m_ButtonCalcRad.EnableWindow( TRUE );
	m_ButtonCalcFlow.EnableWindow( FALSE );
	m_ButtonQ2Sheet.EnableWindow( FALSE );
}

void CDlgSizeRad::InitComboNC( CString strNCID )
{
	m_ComboNomCond.ResetContent();

	// Selected item by default.
	int i = 0;
	CString SelectedItem;
	CTable *pNCTab = (CTable *)( m_pUSERDB->Get( _T("RADNOMCOND_TAB") ).MP );
	ASSERT( NULL != pNCTab );
	
	// Loop on existing CDB_RadNomCond in USERDB radiator nom. Cond. table.
	for( IDPTR RNCIDPtr = pNCTab->GetFirst( CLASS( CDB_RadNomCond ) ); _T('\0') != *RNCIDPtr.ID; RNCIDPtr = pNCTab->GetNext() )
	{
		CString str = GetStringFromNC( (CDB_RadNomCond*)RNCIDPtr.MP );
		int iIndex = m_ComboNomCond.AddString( str );
		m_ComboNomCond.SetItemDataPtr( iIndex, RNCIDPtr.MP );

		if( false == strNCID.IsEmpty() )
		{
			if( 0 == _tcscmp( strNCID, RNCIDPtr.ID ) )
			{
				SelectedItem = str;
			}
		}
	}
	
	if( false == SelectedItem.IsEmpty() )
	{
		i = m_ComboNomCond.FindStringExact( -1, SelectedItem );
	}
	
	m_ComboNomCond.SetCurSel( i );
	OnCbnSelChangeNominalConditions();
}

CString CDlgSizeRad::GetStringFromNC( CDB_RadNomCond *pNC )
{
	CString str = (CString)WriteDouble( CDimValue::SItoCU( _U_TEMPERATURE, pNC->GetTs() ), 2, 0, true );
	str += _T("/")+(CString)WriteDouble( CDimValue::SItoCU( _U_TEMPERATURE, pNC->GetTr() ), 2, 0, true );
	str += _T("/")+(CString)WriteDouble( CDimValue::SItoCU( _U_TEMPERATURE, pNC->GetTi() ), 2, 0, true );
	return str;
}

double CDlgSizeRad::GetMaxQWithFlowLimitation()
{
	if( NULL == m_pclIndSelTRVParams || NULL == m_pclIndSelTRVParams->m_pTADB )
	{
		ASSERTA_RETURN( 0.0 );
	}

	double pdHighestQ = 0.0;
	
	// HYS-1305 : Add insert with automatic flow limiter
	if(( RVT_WithFlowLimitation != m_pclIndSelTRVParams->m_eValveType ) 
		&& ( false == m_pclIndSelTRVParams->m_bIsFLCVInsert ) )
	{
		return pdHighestQ;
	}

	CTable *pTab = (CTable *)m_pclIndSelTRVParams->m_pTADB->Get( _T("FLCTRLVALV_TAB") ).MP;
	ASSERT( NULL != pTab );

	if( NULL == pTab )
	{
		return pdHighestQ;
	}

	// Do a loop on supply valve finding these with correct family, connection and version.
	CString str = CLASS( CDB_FlowLimitedControlValve );
	IDPTR IDPtr = _NULL_IDPTR;

	for( IDPtr = pTab->GetFirst( str ); _T('\0') != *IDPtr.ID; IDPtr = pTab->GetNext() )
	{
		CDB_TAProduct *pProd = (CDB_TAProduct *)( IDPtr.MP );
		CDB_FlowLimitedControlValve *pFlowLimitValve = dynamic_cast<CDB_FlowLimitedControlValve *>( pProd );

		if( NULL == pFlowLimitValve )
		{
			continue;
		}

		CDB_FLCVCharacteristic *pFLCVChar = pFlowLimitValve->GetFLCVCharacteristic();

		if( NULL == pFLCVChar )
		{
			continue;
		}

		CString strSVFamilyID = ( RVT_Inserts != m_pclIndSelTRVParams->m_eValveType ) ? m_pclIndSelTRVParams->m_strComboSVFamilyID 
				: m_pclIndSelTRVParams->m_strComboSVInsertFamilyID;

		CString strSVTypeID = ( RVT_Inserts != m_pclIndSelTRVParams->m_eValveType ) ? m_pclIndSelTRVParams->m_strSVTypeID 
			: m_pclIndSelTRVParams->m_strComboSVInsertTypeID;

		CString strSVConnectID = ( RVT_Inserts != m_pclIndSelTRVParams->m_eValveType ) ? m_pclIndSelTRVParams->m_strComboSVConnectID : _T("");
		CString strSVVersionID = ( RVT_Inserts != m_pclIndSelTRVParams->m_eValveType ) ? m_pclIndSelTRVParams->m_strComboSVVersionID : _T("");

		if( true == pFlowLimitValve->IsSelectable( true ) &&
				( true == strSVFamilyID.IsEmpty() || strSVFamilyID == pProd->GetFamilyID() ) &&
				( true == strSVTypeID.IsEmpty() || strSVTypeID == pProd->GetTypeID() ) &&
				( true == strSVConnectID.IsEmpty() || strSVConnectID == pProd->GetConnectID() ) &&
				( true == strSVVersionID.IsEmpty() || strSVVersionID == pProd->GetVersionID() ) )
		{
			pdHighestQ = max( pdHighestQ, pFLCVChar->GetQNFmax() );
		}

	}

	return pdHighestQ;
}

void CDlgSizeRad::FindKvsBounds( double *pdLowestTrvKvs, double *pdHighestTrvKvs, double *pdLowestRvKvs, double *pdHighestRvKvs )
{
	if( NULL == m_pclIndSelTRVParams || NULL == m_pclIndSelTRVParams->m_pTADB || NULL == pdLowestTrvKvs || NULL == pdHighestTrvKvs 
			|| NULL == pdLowestRvKvs || NULL == pdHighestRvKvs )
	{
		ASSERT_RETURN;
	}

	*pdLowestRvKvs = 0.0;
	*pdHighestRvKvs = 0.0;
	*pdLowestTrvKvs = 0.0;
	*pdHighestTrvKvs = 0.0;

	// HYS-1305 : Add insert with automatic flow limiter
	if( ( RVT_WithFlowLimitation != m_pclIndSelTRVParams->m_eValveType )
		&& ( false == m_pclIndSelTRVParams->m_bIsFLCVInsert ) )
	{
		// Get pointers on required database tables.
		CString strTable;

		switch( m_pclIndSelTRVParams->m_eValveType )
		{
			case RVT_Standard:
			case RVT_Presettable:
				strTable = _T("TRVALV_TAB");
				break;

			case RVT_Inserts:
				strTable = _T("TRINSERT_TAB");
				break;
		}

		CTable *pTab = (CTable *)( m_pclIndSelTRVParams->m_pTADB->Get( strTable ).MP );
		ASSERT( NULL != pTab );
		
		if( NULL == pTab )
		{
			return;
		}

		// Do a loop on supply valve finding these with correct family, connection and version. 
		CString str = CLASS( CDB_ThermostaticValve );
		bool bOneValidFound = false;
		*pdLowestTrvKvs = DBL_MAX;
		*pdHighestTrvKvs = 0.0;

		bool bInsert = ( RVT_Inserts == m_pclIndSelTRVParams->m_eValveType ) ? true : false;
		IDPTR IDPtr = _NULL_IDPTR;
		
		CString strSVFamilyID = ( RVT_Inserts != m_pclIndSelTRVParams->m_eValveType ) ? m_pclIndSelTRVParams->m_strComboSVFamilyID 
				: m_pclIndSelTRVParams->m_strComboSVInsertFamilyID;

		CString strSVTypeID = ( RVT_Inserts != m_pclIndSelTRVParams->m_eValveType ) ? m_pclIndSelTRVParams->m_strSVTypeID 
			: m_pclIndSelTRVParams->m_strComboSVInsertTypeID;

		CString strSVConnectID = ( RVT_Inserts != m_pclIndSelTRVParams->m_eValveType ) ? m_pclIndSelTRVParams->m_strComboSVConnectID : _T("");
		CString strSVVersionID = ( RVT_Inserts != m_pclIndSelTRVParams->m_eValveType ) ? m_pclIndSelTRVParams->m_strComboSVVersionID : _T("");

		for( IDPtr = pTab->GetFirst( str ); _T('\0') != *IDPtr.ID; IDPtr = pTab->GetNext() )
		{
			CDB_TAProduct *pProd = (CDB_TAProduct *)( IDPtr.MP );
			CDB_ThermostaticValve *pThermoValve = dynamic_cast<CDB_ThermostaticValve *>(pProd);

			if( NULL == pThermoValve )
			{
				continue;
			}

			if( true == pThermoValve->IsSelectable( true ) &&
					( true == strSVFamilyID.IsEmpty() || strSVFamilyID == pProd->GetFamilyID() ) &&
					( true == strSVTypeID.IsEmpty() || strSVTypeID == pProd->GetTypeID() ) &&
					( true == strSVConnectID.IsEmpty() || strSVConnectID == pProd->GetConnectID() ) &&
					( true == strSVVersionID.IsEmpty() || strSVVersionID == pProd->GetVersionID() ) )
			{
				bOneValidFound = true;

				// Get Kv characteristic of the valve at 2DT Kelvin.
				double dKvs = 0.0;
				CDB_ThermoCharacteristic *pThermoCharacteristic = pThermoValve->GetThermoCharacteristic();

				if( NULL != pThermoCharacteristic )
				{
					// If curve at 2DT Kelvin exists we take corresponding Kv, otherwise we take KvS.
					dKvs = pThermoCharacteristic->GetKvMax( 2 );

					if( -1.0 == dKvs )
					{
						dKvs = 0.0;
					}
				}

				// Check Kvs w.r.t. min and max Kvs bounds.
				*pdLowestTrvKvs = min( *pdLowestTrvKvs, dKvs );
				*pdHighestTrvKvs = max( *pdHighestTrvKvs, dKvs );
			}
		}

		if( false == bOneValidFound )
		{
			*pdLowestTrvKvs = *pdHighestTrvKvs = 0.0;
		}
	}

	CString strTable = ( RVT_Inserts != m_pclIndSelTRVParams->m_eValveType ) ? _T("REGVALV_TAB") : _T("RVINSERT_TAB");
	CTable *pTab = (CTable *)( m_pclIndSelTRVParams->m_pTADB->Get( strTable ).MP );
	ASSERT( NULL != pTab );
	
	// Do a loop on Rv's finding these with correct family, connection and version. 
	CString str = CLASS( CDB_RegulatingValve );
	bool bOneValidFound = false;
	*pdLowestRvKvs = DBL_MAX;
	*pdHighestRvKvs = 0.0;

	CString strRVFamilyID = ( RVT_Inserts != m_pclIndSelTRVParams->m_eValveType ) ? m_pclIndSelTRVParams->m_strComboRVFamilyID 
			: m_pclIndSelTRVParams->m_strComboRVInsertFamilyID;

	CString strRVConnectID = ( RVT_Inserts != m_pclIndSelTRVParams->m_eValveType ) ? m_pclIndSelTRVParams->m_strComboRVConnectID : _T("");
	CString strRVVersionID = ( RVT_Inserts != m_pclIndSelTRVParams->m_eValveType ) ? m_pclIndSelTRVParams->m_strComboSVVersionID : _T("");

	for( IDPTR IDPtr = pTab->GetFirst( str ); _T('\0') != *IDPtr.ID; IDPtr = pTab->GetNext() )
	{
		CDB_TAProduct *pProd = (CDB_TAProduct *)( IDPtr.MP );

		if( true == pProd->IsSelectable( true ) && -1.0 != ( (CDB_RegulatingValve*)pProd )->GetKvs() &&
				( true == strRVFamilyID.IsEmpty() || strRVFamilyID == pProd->GetFamilyIDPtr().ID ) &&
				( true == strRVConnectID.IsEmpty() || strRVConnectID == pProd->GetConnectIDPtr().ID ) &&
				( true == strRVVersionID.IsEmpty() || strRVVersionID == pProd->GetVersionIDPtr().ID ) )
		{
			bOneValidFound = true;

			// Get Kv characteristic of the valve.
			double dKvs = ( (CDB_RegulatingValve*)pProd )->GetKvs();

			// Check Kvs w.r.t. min and max Kvs bounds.
			*pdLowestRvKvs = min( *pdLowestRvKvs, dKvs );
			*pdHighestRvKvs = max( *pdHighestRvKvs, dKvs );
		}
	}
	
	if( false == bOneValidFound )
	{
		*pdLowestRvKvs = *pdHighestRvKvs = 0.0;
	}

	return;
}

void CDlgSizeRad::_Init( void )
{
	m_pParent = NULL;
	m_pUSERDB = NULL;
	m_strSectionName = _T("");
	m_bInitialized = false;
	m_pNC = NULL;
	m_pclIndSelTRVParams->m_RadInfos.Reset();
}
