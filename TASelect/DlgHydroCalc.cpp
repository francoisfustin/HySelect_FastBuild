#include "stdafx.h"

#include "TASelect.h"
#include "MainFrm.h"
#include "DlgWaterChar.h"
#include "DlgHydroCalcBase.h"
#include "DlgHydroCalcQKvDp.h"
#include "DlgHydroCalcQPDT.h"
#include "DlgHydroCalcRedDisk.h"
#include "DlgHydroCalc.h"
#include "afxdialogex.h"

//////////////////////////////////////////////////////////////////////////////////////////////////////////////
// CDlgHydroCalc dialog
//////////////////////////////////////////////////////////////////////////////////////////////////////////////

IMPLEMENT_DYNAMIC( CDlgHydroCalc, CDialogEx )

CDlgHydroCalc::CDlgHydroCalc( CWnd *pParent )
	: CDialogEx( CDlgHydroCalc::IDD, pParent )
{
	m_pTADS = NULL;
	m_pclDlgHydroCalcQKvDp = NULL;
	m_pclDlgHydroCalcQPDT = NULL;
	m_pclDlgHydroCalcRedDisk = NULL;
	m_clWaterChar = *( TASApp.GetpTADS()->GetpWCForProductSelection()->GetpWCData() );
	m_pValve = NULL;
	m_dKvValue = -1.0;
	m_dDpValue = -1.0;
	m_dFlowValue = -1.0;
	m_dDTValue = -1.0;
	m_dPowerValue = -1.0;
	m_dSettingValue = -1.0;

	m_iRadioState[TabIDs::QKvDp] = CDlgHydroCalcQKvDp::RadioState::Uninitialized;
	m_iRadioState[TabIDs::QPDT] = CDlgHydroCalcQPDT::RadioState::Uninitialized;
	m_iRadioState[TabIDs::QRedDisk] = CDlgHydroCalcRedDisk::RadioState::Uninitialized;
	m_ePreviousTabID = Undefined;
}

CDlgHydroCalc::~CDlgHydroCalc()
{
	if( NULL != m_pclDlgHydroCalcQKvDp )
	{
		if( INVALID_HANDLE_VALUE != m_pclDlgHydroCalcQKvDp->GetSafeHwnd() )
		{
			m_pclDlgHydroCalcQKvDp->DestroyWindow();
		}

		delete m_pclDlgHydroCalcQKvDp;
		m_pclDlgHydroCalcQKvDp = NULL;
	}

	if( NULL != m_pclDlgHydroCalcQPDT )
	{
		if( INVALID_HANDLE_VALUE != m_pclDlgHydroCalcQPDT->GetSafeHwnd() )
		{
			m_pclDlgHydroCalcQPDT->DestroyWindow();
		}

		delete m_pclDlgHydroCalcQPDT;
		m_pclDlgHydroCalcQPDT = NULL;
	}

	if( NULL != m_pclDlgHydroCalcRedDisk )
	{
		if( INVALID_HANDLE_VALUE != m_pclDlgHydroCalcRedDisk->GetSafeHwnd() )
		{
			m_pclDlgHydroCalcRedDisk->DestroyWindow();
		}

		delete m_pclDlgHydroCalcRedDisk;
		m_pclDlgHydroCalcRedDisk = NULL;
	}
}

BEGIN_MESSAGE_MAP( CDlgHydroCalc, CDialogEx )
	ON_WM_CREATE()
	ON_WM_SIZE()
	ON_BN_CLICKED( IDOK, OnBnClickedOk )
	ON_BN_CLICKED( IDC_BUTTONMODWATER, OnButtonModWater )
	ON_REGISTERED_MESSAGE( AFX_WM_CHANGE_ACTIVE_TAB, OnChangeActiveTab )
END_MESSAGE_MAP()

void CDlgHydroCalc::DoDataExchange( CDataExchange *pDX )
{
	CDialogEx::DoDataExchange(pDX);
	DDX_Control( pDX, IDC_GROUPHYDROCALC, m_GroupHydroCalc );
	DDX_Control( pDX, IDC_GROUPWATER, m_GroupWater);
	DDX_Control( pDX, IDC_BUTTONMODWATER, m_ButtonModWater );
	DDX_Control( pDX, IDOK, m_ButtonOK );
}

BOOL CDlgHydroCalc::OnInitDialog() 
{
	CDialogEx::OnInitDialog();
	
	m_pTADS = TASApp.GetpTADS();

	// Initialize dialog strings.
	CString str;
	str = TASApp.LoadLocalizedString( IDS_DLGHYDROCALC_CAPTION );
	SetWindowText( str );

	str = TASApp.LoadLocalizedString( IDS_OK );
	m_ButtonOK.SetWindowText( str );

	// Add icon to water group.
	CImageList *pclImgListGroupBox = TASApp.GetpRCImageManager()->GetImageList( CRCImageManager::ILN_GroupBox );
	ASSERT ( NULL != pclImgListGroupBox );

	if( NULL != pclImgListGroupBox )
	{
		m_GroupWater.SetImageList( pclImgListGroupBox, CRCImageManager::ILGB_Water );
	}

	// Add bitmap to "Water" button.
	CImageList *pclImgListButton = TASApp.GetpRCImageManager()->GetImageList( CRCImageManager::ILN_Button );
	ASSERT ( NULL != pclImgListButton );

	if( NULL != pclImgListButton )
	{
		m_ButtonModWater.SetIcon( (HICON)pclImgListButton->ExtractIcon( CRCImageManager::ILB_ModifyWater ) );
	}

	// Create a ToolTipCtrl and add a tool tip for "Water" button.
	m_ToolTip.Create( this, TTS_NOPREFIX );
	CString TTstr;
	TTstr = TASApp.LoadLocalizedString( IDS_LVTOOLTIP_BUTMODWAT );
	m_ToolTip.AddToolWindow( GetDlgItem( IDC_BUTTONMODWATER ), TTstr );
	
	// Save what is the current project type.
		// Can not calculate for Change-over
	if( ChangeOver == TASApp.GetpTADS()->GetpTechParams()->GetProductSelectionApplicationType() )
	{
		TASApp.GetpTADS()->GetpTechParams()->SetProductSelectionApplicationType( Heating );
		CWaterChar *pWC = TASApp.GetpTADS()->GetpWCForProductSelection()->GetpWCData();
		*pWC = *( TASApp.GetpTADS()->GetpTechParams()->GetDefaultISHeatingWC() );
	}
	m_eCurrentProjectType = m_pTADS->GetpTechParams()->GetProductSelectionApplicationType();

	// Build new water characteristic strings.
	CString str1;
	CString str2;
	m_clWaterChar.BuildWaterStrings( str1, str2 );

	// Set the text for the first static.
	SetDlgItemText( IDC_STATICFLUID1, str1 );
	
	// Set the text for the second static.
	SetDlgItemText( IDC_STATICFLUID2, str2 );

	// Create tabs window.
	if( FALSE == m_WndTabs.Create( CMFCTabCtrl::STYLE_3D, CRect( 0, 0, 0, 0 ), this, 1, CMFCTabCtrl::LOCATION_TOP ) )
	{
		TRACE0( "Failed to create output tab window\n" );
		return 0;      // fail to create
	}

	m_WndTabs.AutoDestroyWindow( TRUE );
	m_WndTabs.SetFlatFrame( FALSE, FALSE );
	m_WndTabs.SetDrawFrame( FALSE );
	m_WndTabs.SetTabBorderSize( 0 );

	// Build and attach the image list.
	m_TabCtrlImageList.Create( 16, 16, ILC_COLOR32, TabIndex::etiLast, 1 );
	CBitmap bm;
	bm.LoadBitmap( IDB_TABHYDROCALC );
	m_TabCtrlImageList.Add( &bm, CLR_NONE );

	m_WndTabs.SetImageList( m_TabCtrlImageList.GetSafeHandle() );
	
	// CDlgHydroCalcQKvDp creation.
	m_pclDlgHydroCalcQKvDp = new CDlgHydroCalcQKvDp( this );
	m_pclDlgHydroCalcQKvDp->Create( CDlgHydroCalcQKvDp::IDD, &m_WndTabs );
	str = TASApp.LoadLocalizedString( IDS_TABHYDROCALCQKVDP );
	m_WndTabs.AddTab( m_pclDlgHydroCalcQKvDp, str, 0 );

	// CDlgHydroCalcQPDT creation.
	m_pclDlgHydroCalcQPDT = new CDlgHydroCalcQPDT( this );
	m_pclDlgHydroCalcQPDT->Create( CDlgHydroCalcQPDT::IDD, &m_WndTabs );
	str = TASApp.LoadLocalizedString( IDS_TABHYDROCALCQPDT );
	m_WndTabs.AddTab( m_pclDlgHydroCalcQPDT, str, 1 );

	// CDlgHydroCalcRedDisk creation.
	m_pclDlgHydroCalcRedDisk = new CDlgHydroCalcRedDisk( this );
	m_pclDlgHydroCalcRedDisk->Create( CDlgHydroCalcRedDisk::IDD, &m_WndTabs );
	str = TASApp.LoadLocalizedString( IDS_TABHYDROCALCREDDISK );
	m_WndTabs.AddTab( m_pclDlgHydroCalcRedDisk, str, 2 );

	CRect rectWindow;
	m_GroupHydroCalc.ShowWindow( SW_HIDE );
	m_GroupHydroCalc.GetWindowRect( &rectWindow );
	ScreenToClient( &rectWindow );

	// Remark: because 'm_WndTabs' is dynamically created, tab order is not correct. With 'SetWindowPos' we can change z-order that is in fact 
	// also internally used by MFC for tab order.
	// Remark: Width() + 1 because in resource editor, I can't set the right side of 'IDC_GROUPHYDROCALC' group on the edge side of dialog.
	m_WndTabs.SetWindowPos( &CWnd::wndTop, rectWindow.left, rectWindow.top, rectWindow.Width() + 1, rectWindow.Height(), SWP_SHOWWINDOW );
	// Remark: be default, MFCTablCtrl is not created with 'WS_TABSTOP' style.
	m_WndTabs.ModifyStyle( 0, WS_TABSTOP );

	return TRUE;
}

int CDlgHydroCalc::OnCreate( LPCREATESTRUCT lpCreateStruct )
{
	if( -1 == CDialogEx::OnCreate( lpCreateStruct ) )
	{
		return -1;
	}

	return 0;
}

void CDlgHydroCalc::OnSize( UINT nType, int cx, int cy )
{
	CDialogEx::OnSize( nType, cx, cy );
}

void CDlgHydroCalc::OnButtonModWater()
{
	// Display the dialog.
	CDlgWaterChar dlg;
	dlg.Display( NULL, CDlgWaterChar::DlgWaterChar_ForTools , true, m_eCurrentProjectType );

	// If the user has simply changed the application type, or has changed something in the current application type.
	if( m_eCurrentProjectType != dlg.GetCurrentApplicationType() || true == dlg.IsSomethingChanged( dlg.GetCurrentApplicationType() ) )
	{
		// Save the new user choice to force it if we call again the 'CDlgWaterChar' dialog.
		m_eCurrentProjectType = dlg.GetCurrentApplicationType();

		double dDT = 0.0;
		dlg.GetWaterChar( dlg.GetCurrentApplicationType(), m_clWaterChar, dDT );

		// Update the 2 water static text.
		CString str1;
		CString str2;
		m_clWaterChar.BuildWaterStrings( str1, str2 );

		// Set the text for the first static.
		SetDlgItemText( IDC_STATICFLUID1, str1 );
	
		// Set the text for the second static.
		SetDlgItemText( IDC_STATICFLUID2, str2 );

		CDlgHydroCalcBase *pclDlgHydroCalcBase = dynamic_cast<CDlgHydroCalcBase *>( m_WndTabs.GetActiveWnd() );
		
		if( NULL == pclDlgHydroCalcBase )
		{
			ASSERT_RETURN;
		}

		pclDlgHydroCalcBase->Calculate();
	}
}

void CDlgHydroCalc::OnBnClickedOk()
{
	CDialogEx::OnOK();
}

LRESULT CDlgHydroCalc::OnChangeActiveTab( WPARAM wp, LPARAM lp )
{
	CDlgHydroCalcBase *pDlgHydroCalcBase = dynamic_cast<CDlgHydroCalcBase *>( m_WndTabs.GetTabWnd( (int)wp ) );

	if( NULL != pDlgHydroCalcBase )
	{
		pDlgHydroCalcBase->OnChangeActiveTab( );
	}

	return 0;
}
