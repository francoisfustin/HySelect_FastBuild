#include "stdafx.h"


#include "TASelect.h"
#include "DataBObj.h"
#include "HydroMod.h"
#include "HydronicPic.h"
#include "DlgHydronicSchView.h"
#include "DlgHydronicSchViewLarge.h"
#include "DlgHydronicSchView_BVTab.h"
#include "DlgHydronicSchView_CVTab.h"
#include "DlgHydronicSchView_DpCTab.h"
#include "DlgHydronicSchView_SmartTab.h"
#include "DlgHydronicSchView_SmartDpTab.h"
#include "SSheetPanelCirc1.h"

IMPLEMENT_DYNAMIC( CDlgHydronicSchView, CDialogEx )

CDlgHydronicSchView::CDlgHydronicSchView( CWnd* pParent )
	: CDialogEx( CDlgHydronicSchView::IDD, pParent )
{
	m_pTADB = TASApp.GetpTADB();
	ASSERT( NULL != m_pTADB );

	m_eProdType = ProdType::Undefined;
	m_pclCircuitSchemeCategory = NULL;
	m_eBalancingType = CDB_CircuitScheme::eBALTYPE::LastBALTYPE;
	m_eReturnType = CDS_HydroMod::ReturnType::Direct;
	m_bTermUnit = false;
	m_bShowDistributionPipe = false;
	m_eControlType = CDB_ControlProperties::CvCtrlType::eCvNU;
	m_eCvType = CDB_ControlProperties::eCVFUNC::NoControl;

	m_pProdPic = NULL;
	m_pCircuitScheme = NULL;

	// Load the tables for DpC schemes and circuit schemes
	m_pTabSch = (CTable *)( m_pTADB->Get( L"CIRCSCHEME_TAB" ).MP );
	ASSERT( NULL != m_pTabSch );

	m_pDpCSchemeTab = NULL;
}

CDlgHydronicSchView::~CDlgHydronicSchView()
{
	for( auto &iter : m_mapTabList )
	{
		delete iter.second;
	}

	m_mapTabList.clear();
}

void CDlgHydronicSchView::VerifyCircuitSheme()
{
	m_vecCircuitSchemeList.clear();
	m_pCircuitScheme = NULL;

	if( NULL == m_pclCircuitSchemeCategory )
	{
		return;
	}

	CSSheetPanelCirc1 clSheetPanelCirc1;
	CSSheetPanelCirc1::ePC1TU eTerminalUnit = ( true == m_bTermUnit ) ? CSSheetPanelCirc1::ePC1TU::ePC1TU_TermUnit : CSSheetPanelCirc1::ePC1TU::ePC1TU_None;
	CSSheetPanelCirc1::ePC1CVType eCVTypeFamily = clSheetPanelCirc1.ConvertCvFunc2PC1CVType( GetCvType() );

	clSheetPanelCirc1.Init_helper( &m_vecCircuitSchemeList, eTerminalUnit, m_bShowDistributionPipe, m_eReturnType, m_eBalancingType, GetCvType(),
			GetControlType(), (int)eCVTypeFamily );

	FillComboBoxCircuitScheme();
	VerifyTerminalUnit();
	VerifyShowDistributionPipe();
}

void CDlgHydronicSchView::RedrawSchemes()
{
	if( NULL != m_pCircuitScheme )
	{
		m_DynTASymbolBmp.DeleteObject();
		m_DynTASymbolBmp.CleanArrayAnchor();
		m_DynBmp.DeleteObject();
		m_DynBmp.CleanArrayAnchor();

		// Draw dynamic schemes.
		DrawDynScheme( true );
		DrawDynScheme( false );

		// Define the EnBitmap siz1 and put it into the corresponding window.
		if( NULL != m_DynTASymbolBmp.m_hObject )
		{
			GetGoodSizeRect( eDrawingType::DynTAScheme );
			m_stDynSchTA.SetBitmap( (HBITMAP)m_DynTASymbolBmp );
		}

		if( NULL != m_DynBmp.m_hObject )
		{
			GetGoodSizeRect( eDrawingType::DynScheme );
			m_stDynSch.SetBitmap( (HBITMAP)m_DynBmp );
		}
	}
	else
	{
		// Reset the object.
		if( NULL != m_DynTASymbolBmp.m_hObject )
		{
			CRect rect = m_DynTASymbolBmp.GetSizeImage();
			m_DynTASymbolBmp.DrawRect( rect, RGB( 255, 255, 255 ) );
			m_stDynSchTA.SetBitmap( (HBITMAP)m_DynTASymbolBmp );
		}

		if( NULL != m_DynBmp.m_hObject )
		{
			CRect rect = m_DynBmp.GetSizeImage();
			m_DynBmp.DrawRect( rect, RGB( 255, 255, 255 ) );
			m_stDynSch.SetBitmap( (HBITMAP)m_DynBmp );
		}
	}
	
	// Refresh the dialog.
	this->RedrawWindow();

	if( NULL != m_clHydronicSchViewCtrlTab.GetActiveWnd() )
	{
		m_clHydronicSchViewCtrlTab.GetActiveWnd()->Invalidate();
		m_clHydronicSchViewCtrlTab.GetActiveWnd()->RedrawWindow();
	}
}

eMvLoc CDlgHydronicSchView::GetMvLoc()
{
	return m_mapTabList.at( CAnchorPt::eFunc::DpC )->m_eMvLoc;
}

eDpStab CDlgHydronicSchView::GetDpStab()
{
	return m_mapTabList.at( CAnchorPt::eFunc::DpC )->m_eDpStab;
}

bool CDlgHydronicSchView::GetBvIn3WBypass()
{
	return m_mapTabList.at( CAnchorPt::eFunc::ControlValve )->m_bBvIn3WBypass;
}

SmartValveLocalization CDlgHydronicSchView::GetSmartValveLocatization()
{
	SmartValveLocalization eSmartValveLocalization = SmartValveLocalization::SmartValveLocNone;

	if( NULL != m_pCircuitScheme )
	{
		if( true == m_pCircuitScheme->IsAnchorPtExist( CAnchorPt::eFunc::SmartControlValve ) )
		{
			eSmartValveLocalization =  m_mapTabList.at( CAnchorPt::eFunc::SmartControlValve )->m_eSmartValveLocation;
		}
	}

	return eSmartValveLocalization;
}

void CDlgHydronicSchView::DoDataExchange( CDataExchange *pDX )
{
	CDialogEx::DoDataExchange( pDX );
	DDX_Control( pDX, IDC_COMBOCIRCUITSCHEMECATEGORY, m_ComboCircuitSchemeCategory );
	DDX_Control( pDX, IDC_COMBOBALANCINGTYPE, m_ComboBalancingType );	
	DDX_Control( pDX, IDC_COMBOCIRCUITSCHEME, m_ComboCircuitScheme );
	DDX_Control( pDX, IDC_COMBORETURNTYPE, m_ComboReturnType );
	DDX_Control( pDX, IDC_CHECKTERMUNIT, m_CheckTerminalUnit );
	DDX_Control( pDX, IDC_CHECKSHOWDISTRPIPE, m_CheckShowDistributionPipe );
	DDX_Control( pDX, IDC_COMBOCONTROLTYPE, m_ComboControlType );
	DDX_Control( pDX, IDC_COMBOCVTYPE, m_ComboControlValveType );
	DDX_Control( pDX, IDC_STATICPICDYNSCHTA, m_stDynSchTA );
	DDX_Control( pDX, IDC_STATICPICDYNSCH, m_stDynSch );
}

BEGIN_MESSAGE_MAP( CDlgHydronicSchView, CDialogEx )
	ON_CBN_SELCHANGE( IDC_COMBOCIRCUITSCHEMECATEGORY, OnCbnSelChangeCircuitSchemeCategory )
	ON_CBN_SELCHANGE( IDC_COMBOBALANCINGTYPE, OnCbnSelChangeBalancingType )
	ON_CBN_SELCHANGE( IDC_COMBORETURNTYPE, OnCbnSelChangeReturnType )
	ON_CBN_SELCHANGE( IDC_COMBOCIRCUITSCHEME, OnCbnSelChangeCircuitScheme )
	ON_BN_CLICKED( IDC_CHECKTERMUNIT, OnBnClickedCheckTerminalUnit )
	ON_BN_CLICKED( IDC_CHECKSHOWDISTRPIPE, OnBnClickedCheckShowDistributionPipe )
	ON_CBN_SELCHANGE( IDC_COMBOCONTROLTYPE, OnCbnSelChangeControlType )
	ON_CBN_SELCHANGE( IDC_COMBOCVTYPE, OnCbnSelComboControlValveType )
	ON_BN_CLICKED( IDC_BTNLARGEVIEWSYMBOL, OnBnClickedLargeViewSymbol )
	ON_BN_CLICKED( IDC_BTNLARGEVIEWPICTURE, OnBnClickedLargeViewPicture )
	ON_BN_CLICKED( IDC_BUTTONDRAW, OnBnClickedButtonDraw )
END_MESSAGE_MAP()

BOOL CDlgHydronicSchView::OnInitDialog()
{
	CDialogEx::OnInitDialog();

	CRect rectCtrlTabContainer;
	GetDlgItem( IDC_GROUPTABS )->GetClientRect( rectCtrlTabContainer );
	CRect rectCtrlTab = rectCtrlTabContainer;
	rectCtrlTab.DeflateRect( 1, 1 );

	if( FALSE == m_clHydronicSchViewCtrlTab.Create( CMFCTabCtrl::STYLE_3D, rectCtrlTab, this, 1, CMFCTabCtrl::LOCATION_TOP, FALSE ) )
	{
		return FALSE;
	}

	// Position the ctrl tab.
	GetDlgItem( IDC_GROUPTABS )->GetWindowRect( rectCtrlTabContainer );
	ScreenToClient( &rectCtrlTabContainer );
	m_clHydronicSchViewCtrlTab.SetWindowPos( NULL, rectCtrlTabContainer.left + 1, rectCtrlTabContainer.top + 7, -1, -1, SWP_NOSIZE | SWP_NOZORDER );

	// Some properties.
	m_clHydronicSchViewCtrlTab.SetDrawFrame( FALSE );
	m_clHydronicSchViewCtrlTab.SetTabBorderSize( 0 );
	m_clHydronicSchViewCtrlTab.AutoDestroyWindow( FALSE );

	// 'BV' tab creation.
	CDlgHydronicSchView_BVTab *pclDlgHydronicSchView_BVTab = new CDlgHydronicSchView_BVTab( this );

	if( NULL == pclDlgHydronicSchView_BVTab )
	{
		return FALSE;
	}

	if( FALSE == pclDlgHydronicSchView_BVTab->Create( CDlgHydronicSchView_BVTab::IDD, &m_clHydronicSchViewCtrlTab ) )
	{
		delete pclDlgHydronicSchView_BVTab;
		return FALSE;
	}

	m_clHydronicSchViewCtrlTab.AddTab( (CWnd *)pclDlgHydronicSchView_BVTab, _T("BV") );
	m_mapTabList.insert( std::pair<CAnchorPt::eFunc, CDlgHydronicSchView_BaseTab *>( CAnchorPt::eFunc::BV_P, pclDlgHydronicSchView_BVTab ) );

	// 'CV' tab creation.
	CDlgHydronicSchView_CVTab *pclDlgHydronicSchView_CVTab = new CDlgHydronicSchView_CVTab( this );

	if( NULL == pclDlgHydronicSchView_CVTab )
	{
		return FALSE;
	}

	if( FALSE == pclDlgHydronicSchView_CVTab->Create( CDlgHydronicSchView_CVTab::IDD, &m_clHydronicSchViewCtrlTab ) )
	{
		delete pclDlgHydronicSchView_CVTab;
		return FALSE;
	}

	m_clHydronicSchViewCtrlTab.AddTab( (CWnd *)pclDlgHydronicSchView_CVTab, _T("CV") );
	m_mapTabList.insert( std::pair<CAnchorPt::eFunc, CDlgHydronicSchView_BaseTab *>( CAnchorPt::eFunc::ControlValve, pclDlgHydronicSchView_CVTab ) );

	// 'DpC' tab creation.
	CDlgHydronicSchView_DpCTab *pclDlgHydronicSchView_DpCTab = new CDlgHydronicSchView_DpCTab( this );

	if( NULL == pclDlgHydronicSchView_DpCTab )
	{
		return FALSE;
	}

	if( FALSE == pclDlgHydronicSchView_DpCTab->Create( CDlgHydronicSchView_DpCTab::IDD, &m_clHydronicSchViewCtrlTab ) )
	{
		delete pclDlgHydronicSchView_DpCTab;
		return FALSE;
	}

	m_clHydronicSchViewCtrlTab.AddTab( (CWnd *)pclDlgHydronicSchView_DpCTab, _T("DpC") );
	m_mapTabList.insert( std::pair<CAnchorPt::eFunc, CDlgHydronicSchView_BaseTab *>( CAnchorPt::eFunc::DpC, pclDlgHydronicSchView_DpCTab ) );

	// 'Smart' tab creation.
	CDlgHydronicSchView_SmartTab *pclDlgHydronicSchView_SmartTab = new CDlgHydronicSchView_SmartTab( this );

	if( NULL == pclDlgHydronicSchView_SmartTab )
	{
		return FALSE;
	}

	if( FALSE == pclDlgHydronicSchView_SmartTab->Create( CDlgHydronicSchView_SmartTab::IDD, &m_clHydronicSchViewCtrlTab ) )
	{
		delete pclDlgHydronicSchView_SmartTab;
		return FALSE;
	}

	m_clHydronicSchViewCtrlTab.AddTab( (CWnd *)pclDlgHydronicSchView_SmartTab, _T("Smart") );
	m_mapTabList.insert( std::pair<CAnchorPt::eFunc, CDlgHydronicSchView_BaseTab *>( CAnchorPt::eFunc::SmartControlValve, pclDlgHydronicSchView_SmartTab ) );

	// 'Smart Dp' tab creation.
	CDlgHydronicSchView_SmartDpTab *pclDlgHydronicSchView_SmartDpTab = new CDlgHydronicSchView_SmartDpTab( this );

	if( NULL == pclDlgHydronicSchView_SmartDpTab )
	{
		return FALSE;
	}

	if( FALSE == pclDlgHydronicSchView_SmartDpTab->Create( CDlgHydronicSchView_SmartDpTab::IDD, &m_clHydronicSchViewCtrlTab ) )
	{
		delete pclDlgHydronicSchView_SmartDpTab;
		return FALSE;
	}

	m_clHydronicSchViewCtrlTab.AddTab( (CWnd *)pclDlgHydronicSchView_SmartDpTab, _T("Smart Dp") );
	m_mapTabList.insert( std::pair<CAnchorPt::eFunc, CDlgHydronicSchView_BaseTab *>( CAnchorPt::eFunc::SmartDpC, pclDlgHydronicSchView_SmartDpTab ) );

	m_clHydronicSchViewCtrlTab.SetActiveTab( 0 );

	// Initialize all ComboBoxes.
	InitComboBox();

	RedrawSchemes();

	return TRUE;
}

void CDlgHydronicSchView::InitComboBox()
{	
	FillComboBoxCircuitSchemeCategory();
	FillComboBoxBalancingType();
	FillComboBoxReturnType();
	FillComboBoxControlType();
	FillComboBoxControlValveType();
	
	VerifyControlValveType();
	VerifyCircuitSheme();

	// Call for all tabs.
	for( auto &iter : m_mapTabList )
	{
		iter.second->InitComboBox();
	}

	// Verify which tab we must activate.
	for( auto &iter: m_mapTabList )
	{
		if( true == iter.second->IsAvailable( m_pCircuitScheme ) )
		{
			iter.second->EnableWindow( TRUE );
			iter.second->FillComboBoxType();
		}
		else
		{
			iter.second->EnableWindow( FALSE );
		}
	}
}

void CDlgHydronicSchView::VerifyTerminalUnit()
{
	int iPrevState = m_CheckTerminalUnit.GetCheck();

	if( NULL != m_pCircuitScheme )
	{
		if( CDB_CircuitScheme::eTERMUNIT::NotAllowed == m_pCircuitScheme->GetTermUnit() )
		{
			m_CheckTerminalUnit.EnableWindow( FALSE );
			m_CheckTerminalUnit.SetCheck( BST_UNCHECKED );
		}
		else if( CDB_CircuitScheme::eTERMUNIT::Allowed == m_pCircuitScheme->GetTermUnit() )
		{
			m_CheckTerminalUnit.EnableWindow( TRUE );
			m_CheckTerminalUnit.SetCheck( iPrevState );
		}
		else if( CDB_CircuitScheme::eTERMUNIT::Compulsory == m_pCircuitScheme->GetTermUnit() )
		{
			m_CheckTerminalUnit.EnableWindow( TRUE );
			m_CheckTerminalUnit.SetCheck( BST_CHECKED );
		}
	}
	else
	{
		m_CheckTerminalUnit.EnableWindow( FALSE );
		m_CheckTerminalUnit.SetCheck( BST_UNCHECKED );
	}

	m_bTermUnit = ( BST_CHECKED == m_CheckTerminalUnit.GetCheck() ) ? true : false;
}

void CDlgHydronicSchView::VerifyShowDistributionPipe()
{
	int iPrevState = m_CheckShowDistributionPipe.GetCheck();

	if( NULL != m_pCircuitScheme )
	{
		if( ( 0 == IDcmp( _T("PUMP"), m_pCircuitScheme->GetIDPtr().ID ) ) || ( 0 == IDcmp( _T("PUMP_BV"), m_pCircuitScheme->GetIDPtr().ID ) ) )
		{
			m_CheckShowDistributionPipe.EnableWindow( FALSE );
			m_CheckShowDistributionPipe.SetCheck( BST_UNCHECKED );
		}
		else
		{
			m_CheckShowDistributionPipe.EnableWindow( TRUE );
			m_CheckShowDistributionPipe.SetCheck( iPrevState );
		}
	}
	else
	{
		m_CheckShowDistributionPipe.EnableWindow( FALSE );
		m_CheckShowDistributionPipe.SetCheck( BST_UNCHECKED );
	}
}

void CDlgHydronicSchView::VerifyControlValveType()
{
	if( CDB_ControlProperties::CvCtrlType::eCvNU == m_eControlType )
	{
		m_ComboControlValveType.EnableWindow( FALSE );
	}
	else
	{
		m_ComboControlValveType.EnableWindow( TRUE );
	}

	m_mapTabList.at( CAnchorPt::eFunc::ControlValve )->VerifyControlValveType();
}

void CDlgHydronicSchView::AddHMChildren( CTable *pTab )
{
	if( NULL == pTab)
	{
		pTab = TASApp.GetpTADS()->GetpHydroModTable();
	}

	for( IDPTR IDPtr = pTab->GetFirst(); _T('\0') != *IDPtr.ID; IDPtr = pTab->GetNext() )
	{
		CDS_HydroMod *pHM = dynamic_cast<CDS_HydroMod*>( IDPtr.MP );
		int pos = m_ComboCircuitSchemeCategory.AddString( pHM->GetHMName() );
		m_ComboCircuitSchemeCategory.SetItemData( pos, (DWORD_PTR)IDPtr.MP );
		
		if( true == pHM->IsaModule() )
		{
			AddHMChildren( pHM );
		}
	}
}

void CDlgHydronicSchView::FillComboBoxCircuitSchemeCategory()
{
	m_ComboCircuitSchemeCategory.ResetContent();

	m_ComboCircuitSchemeCategory.InsertString( 0, _PDG );
	m_ComboCircuitSchemeCategory.InsertString( 1, _PUMP );
	m_ComboCircuitSchemeCategory.InsertString( 2, _SPipes );
	m_ComboCircuitSchemeCategory.InsertString( 3, _DC );
	m_ComboCircuitSchemeCategory.InsertString( 4, _2W );
	m_ComboCircuitSchemeCategory.InsertString( 5, _2WINJ );
	m_ComboCircuitSchemeCategory.InsertString( 6, _3W );
	m_ComboCircuitSchemeCategory.InsertString( 7, _3WINJMIX );
	m_ComboCircuitSchemeCategory.InsertString( 8, _3WINJDB );

	// Must correspond to the list above.
	m_arCDBSchemeCategory.Add( (CDB_CircSchemeCateg *)( TASApp.GetpTADB()->Get( _T("SCHCATEG_PDG") ).MP ) );
	m_arCDBSchemeCategory.Add( (CDB_CircSchemeCateg *)( TASApp.GetpTADB()->Get( _T("SCHCATEG_PUMP") ).MP ) );
	m_arCDBSchemeCategory.Add( (CDB_CircSchemeCateg *)( TASApp.GetpTADB()->Get( _T("SCHCATEG_SP") ).MP ) );
	m_arCDBSchemeCategory.Add( (CDB_CircSchemeCateg *)( TASApp.GetpTADB()->Get( _T("SCHCATEG_DC") ).MP ) );
	m_arCDBSchemeCategory.Add( (CDB_CircSchemeCateg *)( TASApp.GetpTADB()->Get( _T("SCHCATEG_2WD") ).MP ) );
	m_arCDBSchemeCategory.Add( (CDB_CircSchemeCateg *)( TASApp.GetpTADB()->Get( _T("SCHCATEG_2W_INJ") ).MP ) );
	m_arCDBSchemeCategory.Add( (CDB_CircSchemeCateg *)( TASApp.GetpTADB()->Get( _T("SCHCATEG_3WD") ).MP ) );
	m_arCDBSchemeCategory.Add( (CDB_CircSchemeCateg *)( TASApp.GetpTADB()->Get( _T("SCHCATEG_3WINJ_MIX") ).MP ) );
	m_arCDBSchemeCategory.Add( (CDB_CircSchemeCateg *)( TASApp.GetpTADB()->Get( _T("SCHCATEG_3WINJ_DB") ).MP ) );

	m_ComboCircuitSchemeCategory.SetCurSel( 0 );
	m_pclCircuitSchemeCategory = m_arCDBSchemeCategory[0];
}

void CDlgHydronicSchView::FillComboBoxBalancingType()
{
	m_ComboBalancingType.ResetContent();

	if( CB_ERR == m_ComboCircuitSchemeCategory.GetCurSel() )
	{
		return;
	}

	CString strCircuitSchemeCategory( m_arCDBSchemeCategory[m_ComboCircuitSchemeCategory.GetCurSel()]->GetIDPtr().ID );

	if( _T("SCHCATEG_PDG") == strCircuitSchemeCategory || _T("SCHCATEG_SP") == strCircuitSchemeCategory )
	{
		m_ComboBalancingType.InsertString( 0, _NoBalanc );
		m_ComboBalancingType.SetItemData( 0, (DWORD_PTR) CDB_CircuitScheme::eBALTYPE::SP );
	}
	else
	{
		m_ComboBalancingType.InsertString( 0, _NoBalanc );
		m_ComboBalancingType.SetItemData( 0, (DWORD_PTR) CDB_CircuitScheme::eBALTYPE::SP );

		m_ComboBalancingType.InsertString( 1, _ManBalanc );
		m_ComboBalancingType.SetItemData( 1, (DWORD_PTR) CDB_CircuitScheme::eBALTYPE::BV );

		m_ComboBalancingType.InsertString( 2, _DiffPresCtrl );
		m_ComboBalancingType.SetItemData( 2, (DWORD_PTR) CDB_CircuitScheme::eBALTYPE::DPC );

		m_ComboBalancingType.InsertString( 3, _Electronic );
		m_ComboBalancingType.SetItemData( 3, (DWORD_PTR) CDB_CircuitScheme::eBALTYPE::ELECTRONIC );

		m_eBalancingType = CDB_CircuitScheme::eBALTYPE::BV;
	}

	m_ComboBalancingType.SetCurSel( 0 );
	m_eBalancingType = (CDB_CircuitScheme::eBALTYPE)m_ComboBalancingType.GetItemData( 0 );
}

void CDlgHydronicSchView::FillComboBoxReturnType()
{
	m_ComboReturnType.ResetContent();

	m_ComboReturnType.InsertString( 0, _ReturnDirect );
	m_ComboReturnType.SetItemData( 0, (DWORD_PTR)CDS_HydroMod::ReturnType::Direct );

	m_ComboReturnType.InsertString( 1, _ReturnReverse );
	m_ComboReturnType.SetItemData( 1, (DWORD_PTR)CDS_HydroMod::ReturnType::Reverse );

	m_ComboReturnType.SetCurSel( 0 );
	m_eReturnType = (CDS_HydroMod::ReturnType)m_ComboReturnType.GetItemData( 0 );
}

void CDlgHydronicSchView::FillComboBoxControlType()
{
	m_ComboControlType.ResetContent();

	m_ComboControlType.InsertString( 0, _None );
	m_ComboControlType.SetItemData( 0, (DWORD_PTR)CDB_ControlProperties::eCvNU );

	m_ComboControlType.InsertString( 1, _Proportional );
	m_ComboControlType.SetItemData( 1, (DWORD_PTR)CDB_ControlProperties::eCvProportional );

	m_ComboControlType.InsertString( 2, _OnOff );
	m_ComboControlType.SetItemData( 2, (DWORD_PTR)CDB_ControlProperties::eCvOnOff );

	m_ComboControlType.InsertString( 3, _3points );
	m_ComboControlType.SetItemData( 3, (DWORD_PTR)CDB_ControlProperties::eCv3point );
	
	m_ComboControlType.SetCurSel( 0 );
	m_eControlType = CDB_ControlProperties::eCvNU;
}

void CDlgHydronicSchView::FillComboBoxControlValveType()
{
	m_ComboControlValveType.ResetContent();

	if( CDB_ControlProperties::CvCtrlType::eCvNU != m_eControlType )
	{
		m_ComboControlValveType.InsertString( 0, _ControlOnly );
		m_ComboControlValveType.SetItemData( 0, (DWORD_PTR)CDB_ControlProperties::eCVFUNC::ControlOnly );

		m_ComboControlValveType.InsertString( 1, _Adjustable );
		m_ComboControlValveType.SetItemData( 1, (DWORD_PTR)CDB_ControlProperties::eCVFUNC::Presettable );

		m_ComboControlValveType.InsertString( 2, _AdjustableMeas );
		m_ComboControlValveType.SetItemData( 2, (DWORD_PTR)CDB_ControlProperties::eCVFUNC::PresetPT );

		m_ComboControlValveType.InsertString( 3, _PressIndep );
		m_ComboControlValveType.SetItemData( 3, (DWORD_PTR)( CDB_ControlProperties::eCVFUNC::Presettable | CDB_ControlProperties::eCVFUNC::PresetPT ) );

		m_ComboControlValveType.InsertString( 4, _Smart );
		m_ComboControlValveType.SetItemData( 4, (DWORD_PTR)CDB_ControlProperties::eCVFUNC::Electronic );

		m_ComboControlValveType.SetCurSel( 0 );
		m_eCvType = CDB_ControlProperties::eCVFUNC::ControlOnly;
	}
	else
	{
		m_ComboControlValveType.InsertString( 0, _NoControl );
		m_ComboControlValveType.SetItemData( 0, (DWORD_PTR)CDB_ControlProperties::eCVFUNC::NoControl );
		m_ComboControlValveType.SetCurSel( 0 );
		m_eCvType = CDB_ControlProperties::eCVFUNC::NoControl;
	}
}

void CDlgHydronicSchView::FillComboBoxCircuitScheme()
{
	m_ComboCircuitScheme.ResetContent();

	if( m_vecCircuitSchemeList.size() > 0 )
	{
		m_ComboCircuitScheme.EnableWindow( TRUE );

		for( auto &iter : m_vecCircuitSchemeList )
		{
			int pos = m_ComboCircuitScheme.AddString( iter->GetIDPtr().ID );
			m_ComboCircuitScheme.SetItemData( pos, (DWORD_PTR)iter->GetIDPtr().MP );
		}

		m_ComboCircuitScheme.SetCurSel( 0 );
		m_pCircuitScheme = (CDB_CircuitScheme *)m_ComboCircuitScheme.GetItemData( 0 );
	}
	else
	{
		m_ComboCircuitScheme.EnableWindow( FALSE );
	}
}

void CDlgHydronicSchView::OnCbnSelChangeCircuitSchemeCategory()
{
	if( CB_ERR == m_ComboCircuitSchemeCategory.GetCurSel() )
	{
		return;
	}

	m_pclCircuitSchemeCategory = m_arCDBSchemeCategory[m_ComboCircuitSchemeCategory.GetCurSel()];

	FillComboBoxBalancingType();
	OnCbnSelChangeBalancingType();
}

void CDlgHydronicSchView::OnCbnSelChangeBalancingType()
{
	if( CB_ERR == m_ComboBalancingType.GetCurSel() )
	{
		return;
	}

	m_eBalancingType = (CDB_CircuitScheme::eBALTYPE)m_ComboBalancingType.GetItemData( m_ComboBalancingType.GetCurSel() );
	
	FillComboBoxControlType();
	OnCbnSelChangeControlType();
	
	/*
	VerifyCircuitSheme();
	FillComboBoxCircuitScheme();

	// Verify which tab we must activate.
	for( auto &iter: m_mapTabList )
	{
		iter.second->ResetComboBoxStr();

		if( true == iter.second->IsAvailable( m_pCircuitScheme ) )
		{
			iter.second->EnableWindow( TRUE );
			iter.second->FillComboBoxType();
		}
		else
		{
			iter.second->EnableWindow( FALSE );
		}
	}

	OnCbnSelChangeCircuitScheme();
	*/
}

void CDlgHydronicSchView::OnCbnSelChangeCircuitScheme()
{
	if( m_ComboCircuitScheme.GetCount() > 0 )
	{
		m_pCircuitScheme = (CDB_CircuitScheme *)m_ComboCircuitScheme.GetItemData( m_ComboCircuitScheme.GetCurSel() );
	}
	
	VerifyTerminalUnit();
	VerifyShowDistributionPipe();
	RedrawSchemes();
}

void CDlgHydronicSchView::OnCbnSelChangeReturnType()
{
	m_eReturnType = (CDS_HydroMod::ReturnType)m_ComboReturnType.GetItemData( m_ComboReturnType.GetCurSel() );
	RedrawSchemes();
}

void CDlgHydronicSchView::OnBnClickedCheckTerminalUnit()
{
	m_bTermUnit = ( BST_CHECKED == m_CheckTerminalUnit.GetCheck() ) ? true : false;

	// Redraw the scheme.
	RedrawSchemes();
}

void CDlgHydronicSchView::OnBnClickedCheckShowDistributionPipe()
{
	m_bShowDistributionPipe = ( BST_CHECKED == m_CheckShowDistributionPipe.GetCheck()  ) ? true : false;

	// Redraw the scheme.
	RedrawSchemes();
}

void CDlgHydronicSchView::OnCbnSelChangeControlType()
{
	if( CB_ERR == m_ComboControlType.GetCurSel() )
	{
		return;
	}

	m_eControlType = (CDB_ControlProperties::CvCtrlType)m_ComboControlType.GetItemData( m_ComboControlType.GetCurSel() );

	FillComboBoxControlValveType();
	VerifyControlValveType();

	OnCbnSelComboControlValveType();
}

void CDlgHydronicSchView::OnCbnSelComboControlValveType()
{
	if( CB_ERR == m_ComboControlValveType.GetCurSel() )
	{
		return;
	}

	m_eCvType = (CDB_ControlProperties::eCVFUNC)m_ComboControlValveType.GetItemData( m_ComboControlValveType.GetCurSel() );
	
	VerifyCircuitSheme();
	FillComboBoxCircuitScheme();

	// Verify which tab we must activate.
	for( auto &iter: m_mapTabList )
	{
		iter.second->ResetComboBoxStr();

		if( true == iter.second->IsAvailable( m_pCircuitScheme ) )
		{
			iter.second->EnableWindow( TRUE );
			iter.second->FillComboBoxType();
		}
		else
		{
			iter.second->EnableWindow( FALSE );
		}
	}

	OnCbnSelChangeCircuitScheme();
}

void CDlgHydronicSchView::OnBnClickedLargeViewSymbol()
{
	m_DynTASymbolBmp.DeleteObject();
	m_DynTASymbolBmp.CleanArrayAnchor();

	DrawDynScheme( true );

	if( NULL != m_DynTASymbolBmp.m_hObject )
	{
		CDlgHydronicSchViewLarge clDlgHydronicSchViewLarge( &m_DynTASymbolBmp );
		clDlgHydronicSchViewLarge.DoModal();
	}
}

void CDlgHydronicSchView::OnBnClickedLargeViewPicture()
{
	m_DynBmp.DeleteObject();
	m_DynBmp.CleanArrayAnchor();

	DrawDynScheme( false );

	if( NULL != m_DynBmp.m_hObject )
	{
		CDlgHydronicSchViewLarge clDlgHydronicSchViewLarge( &m_DynBmp );
		clDlgHydronicSchViewLarge.DoModal();
	}
}

void CDlgHydronicSchView::OnBnClickedButtonDraw()
{
	RedrawSchemes();
}

void CDlgHydronicSchView::DrawDynScheme( bool bForDynSchTASymbol )
{
	if( NULL == m_pCircuitScheme )
	{
		return;
	}

	if( true == bForDynSchTASymbol )
	{
		CEnBitmapPatchWork *pBmp = &m_DynTASymbolBmp;
		pBmp->Reset();

		bool bCvModulating = ( CDB_ControlProperties::CvCtrlType::eCvOnOff == GetControlType() ) ? false : true;
		pBmp->GetHydronicScheme( m_pCircuitScheme, m_bTermUnit, bCvModulating, NULL, m_bShowDistributionPipe, m_eReturnType );
	}
	else
	{
		std::map<CAnchorPt::eFunc, CString> mapProductList;

		for( auto &iter : m_mapTabList )
		{
			iter.second->GetProducts( &mapProductList, m_pCircuitScheme );
		}

		CEnBitmapPatchWork *pBmp = &m_DynBmp;
		pBmp->Reset();

		bool bCvModulating = ( CDB_ControlProperties::CvCtrlType::eCvOnOff == GetControlType() ) ? false : true;
		pBmp->GetHydronicScheme( m_pCircuitScheme->GetIDPtr().ID, &mapProductList, bCvModulating, m_bTermUnit, (int)m_bShowDistributionPipe, m_eReturnType );
	}
}

CRect CDlgHydronicSchView::GetGoodSizeRect( eDrawingType eSchemeType )
{
	CRect rc,rect;
	CRect rectbmp;

	if( eDrawingType::DynScheme == eSchemeType )
	{
		GetDlgItem( IDC_STATICPICDYNSCH )->GetClientRect( &rect );
		rectbmp = m_DynBmp.GetSizeImage();
	}
	else if( eDrawingType::DynTAScheme == eSchemeType )
	{
		GetDlgItem(IDC_STATICPICDYNSCHTA)->GetClientRect( &rect );
		rectbmp = m_DynTASymbolBmp.GetSizeImage();
	}
		
	// Verify the "Gif" image has no higher dimension than the rectangle
	// If it's the case, stretch the image to fit in the rectangle.
	if( rectbmp.Height() > rect.Height() || rectbmp.Width() > rect.Width() )
	{
		double dHeight = INT_MAX;
		double dWidth = INT_MAX;

		if( rectbmp.Height() > rect.Height() )
		{
			dHeight = (double)rect.Height() / (double)rectbmp.Height();
		}

		if( rectbmp.Width() > rect.Width() )
		{
			dWidth = (double)rect.Width() / (double)rectbmp.Width();
		}

		if( eDrawingType::DynScheme == eSchemeType )
		{
			m_DynBmp.ResizeImage( dHeight < dWidth ? dHeight : dWidth );
			rectbmp = m_DynBmp.GetSizeImage();
		}
		else if( eDrawingType::DynTAScheme == eSchemeType )
		{
			m_DynTASymbolBmp.ResizeImage( dHeight < dWidth ? dHeight : dWidth );
			rectbmp = m_DynTASymbolBmp.GetSizeImage();
		}
	}

	rect.left = ( rect.Width() / 2 ) - ( rectbmp.Width() / 2 );
	rect.top = ( rect.Height() / 2 ) - ( rectbmp.Height() / 2 );

	if( eDrawingType::OldScheme == eSchemeType )
	{
		GetDlgItem( IDC_STATICPICOLDSCH )->GetWindowRect( &rc );
	}
	else if( eDrawingType::DynScheme == eSchemeType )
	{
		GetDlgItem( IDC_STATICPICDYNSCH )->GetWindowRect( &rc );
	}
	else if( eDrawingType::DynTAScheme == eSchemeType )
	{
		GetDlgItem( IDC_STATICPICDYNSCHTA )->GetWindowRect( &rc );
	}

	ScreenToClient( &rc );
	rect.left += rc.left;
	rect.top += rc.top;
	return rect;
}
