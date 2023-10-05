#include "stdafx.h"
#include "TASelect.h"
#include "MainFrm.h"
#include "Global.h"
#include "Select.h"
#include "HMInclude.h"
#include "DlgWaterChar.h"

#include "DlgNoDevFound.h"
#include "DlgConfSel.h"

#include "DlgLTtabctrl.h"

#include "EnBitmapRow.h"
#include "RViewSSelSS.h"
#include "RViewSSelHub.h"
#include "HubHandler.h"

#include "DlgSelectionBase.h"
#include "DlgIndSelBase.h"
#include "DlgLeftTabSelManager.h"
#include "DlgIndSelHub.h"

#ifdef _DEBUG
#define new DEBUG_NEW
#undef THIS_FILE
static char THIS_FILE[] = __FILE__;
#endif

CDlgIndSelHub *pDlgIndSelHub = NULL;

CDlgIndSelHub::CDlgIndSelHub( CWnd *pParent )
	: CDlgIndSelBase( m_clIndSelHUBParams, CDlgIndSelHub::IDD, pParent )
{
	m_clIndSelHUBParams.m_eProductSubCategory = ProductSubCategory::PSC_BC_HUB;
	m_clIndSelHUBParams.m_eProductSelectionMode = ProductSelectionMode::ProductSelectionMode_Individual;

	// Initialize variables.
	m_bLocked = false;
	m_pHubHandler = new CHubHandler;
	m_fNewDoc = true;
}

CDlgIndSelHub::~CDlgIndSelHub()
{
	if( NULL != m_pHubHandler )
	{
		delete m_pHubHandler;
	}

	pDlgIndSelHub = NULL;
}

void CDlgIndSelHub::DoDataExchange( CDataExchange* pDX )
{
	CDlgIndSelBase::DoDataExchange( pDX );
	DDX_Control( pDX, IDC_BUTTONSELECT, m_ButtonSelect );
	DDX_Control( pDX, IDC_RADIOFLOW, m_radioFlow );
	DDX_Control( pDX, IDC_RADIOFLOWANDLOOPDP, m_radioFlowLoopDP );
	DDX_Control( pDX, IDC_RADIODIRECTSELECTION, m_radioDirectSelection );
	DDX_Control( pDX, IDC_RADIOFLOWANDLOOPDETAILS, m_radioFlowLoopDetails );
	DDX_Control( pDX, IDC_RADIOLEFT, m_radioLeft );
	DDX_Control( pDX, IDC_RADIORIGHT, m_radioRight );
	DDX_Control( pDX, IDC_COMBOPARTNERTYPE, m_ComboPartnerType );
	DDX_Control( pDX, IDC_GROUPTAHUB, m_GroupTAHub );
	DDX_Control( pDX, IDC_COMBONUMBEROFSTATIONS, m_ComboNumberOfStation );
	DDX_Control( pDX, IDC_GROUPSELECTIONMODE, m_GroupSelectionMode );
	DDX_Control( pDX, IDC_COMBOAPPLICATION, m_ComboAppli );
	DDX_Control( pDX, IDC_COMBOSTATIONCONNECT, m_ComboStationConnect );
	DDX_Control( pDX, IDC_COMBOMVLOC, m_ComboMvLoc );
}

BEGIN_MESSAGE_MAP( CDlgIndSelHub, CDlgIndSelBase )
	ON_WM_VSCROLL()
	ON_CBN_SELCHANGE( IDC_COMBOPARTNERTYPE, OnCbnSelchangeCombopartnertype )
	// ??? Why use syntax with '&CDlgSelectionBaseHub::'
// 	ON_CBN_SELCHANGE( IDC_COMBOAPPLICATION, &CDlgSelectionBaseHub::OnCbnSelchangeComboapplication )
// 	ON_CBN_SELCHANGE( IDC_COMBOSTATIONCONNECT, &CDlgSelectionBaseHub::OnCbnSelchangeCombostationconnect )
// 	ON_CBN_SELCHANGE( IDC_COMBOMVLOC, &CDlgSelectionBaseHub::OnCbnSelchangeCombomvloc )
	ON_CBN_SELCHANGE( IDC_COMBOAPPLICATION, OnCbnSelchangeComboapplication )
	ON_CBN_SELCHANGE( IDC_COMBOSTATIONCONNECT, OnCbnSelchangeCombostationconnect )
	ON_CBN_SELCHANGE( IDC_COMBOMVLOC, OnCbnSelchangeCombomvloc )
	ON_CBN_SELCHANGE( IDC_COMBONUMBEROFSTATIONS, OnCbnSelchangeCombonumberofstations )
	ON_BN_CLICKED( IDC_BUTTONMODWATER, OnButtonmodwater )
	ON_BN_CLICKED( IDC_BUTTONSELECT, OnButtonselect )
	ON_BN_CLICKED( IDC_RADIOLEFT, OnBnClickedRadioMainSupplyReturnPipe )
	ON_BN_CLICKED( IDC_RADIORIGHT, OnBnClickedRadioMainSupplyReturnPipe )
	ON_BN_CLICKED( IDC_RADIOFLOW, OnBnClickedRadioSelectionMode )
	ON_BN_CLICKED( IDC_RADIOFLOWANDLOOPDP, OnBnClickedRadioSelectionMode )
	ON_BN_CLICKED( IDC_RADIODIRECTSELECTION, OnBnClickedRadioSelectionMode )
	ON_BN_CLICKED( IDC_RADIOFLOWANDLOOPDETAILS, OnBnClickedRadioSelectionMode )
	ON_BN_CLICKED( IDC_BUTTONNEW, OnBnClickedButtonnew )
	ON_MESSAGE( WM_USER_CHANGEFOCUSRVIEWSSEL, OnChangefocusRViewssel )
	ON_MESSAGE( WM_USER_MODIFYSELECTEDHUB, OnModifySelectedHub )
END_MESSAGE_MAP()

// CDlgSelectionBaseHub message handlers

void CDlgIndSelHub::RefreshNumberOfStations()
{
	int iNbS = ( (CDS_HmHub *)( m_pHubHandler->GetHubIDPtr().MP ) )->GetNumberOfStations();
	for( int iLoopStation = 0; iLoopStation < m_ComboNumberOfStation.GetCount(); iLoopStation++ )
	{
		if( iNbS == m_ComboNumberOfStation.GetItemData( iLoopStation ) )
		{
			m_ComboNumberOfStation.SetCurSel( iLoopStation );
			break;
		}
	}
}

void CDlgIndSelHub::ResetToDefault()
{
	m_radioFlow.SetCheck( true );
	m_radioFlowLoopDP.SetCheck( false );
	m_radioDirectSelection.SetCheck( false );
	m_radioFlowLoopDetails.SetCheck( false );
	m_radioLeft.SetCheck( true );
	m_radioRight.SetCheck( false );
	FillComboExPartner();
	FillComboNumberOfStation();
	FillComboConnect();
	FillComboApplication();
}

void CDlgIndSelHub::UpdateValues( bool fNew )
{
	if( false == m_bInitialised )
		return;

	CString BalTypeID = ( (CDB_StringID *)m_ComboPartnerType.GetItemDataPtr( m_ComboPartnerType.GetCurSel() ) )->GetIDstr2();

	CDS_HmHub::SelMode SelMode = CDS_HmHub::SelMode::Direct;
	if( m_radioFlow.GetCheck() )							// Selection mode : Flow ==> 0
		SelMode = CDS_HmHub::SelMode::Q;
	else if( m_radioFlowLoopDP.GetCheck() )					// Selection mode : Flow and loop Dp ==> 1
		SelMode = CDS_HmHub::SelMode::QDpLoop;
	else if( m_radioFlowLoopDetails.GetCheck() )			// Selection mode : Flow and loop details ==> 2
		SelMode = CDS_HmHub::SelMode::LoopDetails;
	else if (m_radioDirectSelection.GetCheck())			// Selection mode : direction selection ==> 3
		SelMode = CDS_HmHub::SelMode::Direct;
	else
		ASSERT(0);

	IDPTR StationsConnectIDPtr = ( (CData *)m_ComboStationConnect.GetItemData( m_ComboStationConnect.GetCurSel() ) )->GetIDPtr();

	CDS_HmHub::Application Appli = (CDS_HmHub::Application)m_ComboAppli.GetItemData( m_ComboAppli.GetCurSel() );
	int iNumofStations = m_ComboNumberOfStation.GetItemData( m_ComboNumberOfStation.GetCurSel() );
	CDS_HmHub::MainFeedPos MainFeedPos = m_radioLeft.GetCheck() ? CDS_HmHub::MainFeedPos::Left : CDS_HmHub::MainFeedPos::Right;
	
	// Create or modify existing Hub
	m_pHubHandler->CreateHub( _T("TMPHUB_TAB"),	Appli, iNumofStations, MainFeedPos, true, BalTypeID, StationsConnectIDPtr, SelMode, fNew );
	
	CDS_HmHub *pHub = (CDS_HmHub *)( m_pHubHandler->GetHubIDPtr().MP );

	pHub->ComputeHM( CDS_HydroMod::eComputeHMEvent::eceResize );
	pHub->ComputeAll();
	
	if( NULL != pRViewSSelHub )
	{
		pRViewSSelHub->SetRedraw( pHub );
	}
	
	// Update Text on New and Select Button
	CString str;
	str = TASApp.LoadLocalizedString( IDS_TABCDIALOGSSELH_SELECT );
	GetDlgItem( IDC_BUTTONSELECT )->SetWindowText( str );
	str = TASApp.LoadLocalizedString( IDS_TABCDIALOGSSELH_NEW );
	GetDlgItem( IDC_BUTTONNEW )->SetWindowText( str );
	if( NULL != pHub->GetpSelectedInfos() )
	{
		CRect rectButCancel, rectButApply;
		GetDlgItem( IDC_BUTTONNEW )->GetWindowRect( &rectButCancel );
		GetDlgItem( IDC_BUTTONSELECT )->GetWindowRect( &rectButApply );
		ScreenToClient( &rectButApply );
		double dWidth = rectButCancel.Width();
		if( NULL != *pHub->GetpSelectedInfos()->GetModifiedObjIDPtr().ID )
		{
			// Modification in course
			str = TASApp.LoadLocalizedString( IDS_TABCDIALOGSSELH_APPLYMODIFICATION );
			GetDlgItem( IDC_BUTTONSELECT )->SetWindowText( str );
			str = TASApp.LoadLocalizedString( IDS_CANCEL );
			GetDlgItem( IDC_BUTTONNEW )->SetWindowText( str );
			dWidth = dWidth * 1.6 + 0.5;
		}
		// Adapt button size
		GetDlgItem( IDC_BUTTONSELECT )->SetWindowPos( NULL, rectButApply.right - (int)dWidth, rectButApply.top, (int)dWidth, rectButCancel.Height(), SWP_NOOWNERZORDER );
	}	
	
	// Refresh measuring valve position
	FillComboMvLoc();
	CheckSelectButton( pHub );
}

void CDlgIndSelHub::FillComboNumberOfStation()
{
	CString str;
	int nIndex;
	
	m_ComboNumberOfStation.ResetContent();
	for( int iLoopStation = HUB_MIN_NUMBEROFSTATION; iLoopStation <= HUB_MAX_NUMBEROFSTATION; iLoopStation++ )
	{
		str.Format( _T("%d"), iLoopStation );
		nIndex = m_ComboNumberOfStation.AddString( str );
		m_ComboNumberOfStation.SetItemData( nIndex, iLoopStation );
	}
	// Select the HUB_MAX_NUMBEROFSTATION
	m_ComboNumberOfStation.SetCurSel( nIndex );
}

void CDlgIndSelHub::FillComboExPartner()
{
	CTable *pTab = (CTable*)( m_clIndSelHUBParams.m_pTADB->Get( _T("HUBTYPE_TAB") ).MP );
	ASSERT( NULL != pTab );

	CTable *pTabRV = (CTable*)( m_clIndSelHUBParams.m_pTADB->Get( _T("HUBVALV_TAB") ).MP );
	ASSERT( NULL != pTabRV );

	CRank rkList;
	
	for( IDPTR Idptr = pTab->GetFirst(); NULL != *Idptr.ID; Idptr = pTab->GetNext() )
	{
		// Try to find in HubValvTab a return valve with matching type
		bool fFound = false;
		for( IDPTR IDptrHV = pTabRV->GetFirst(); NULL != *IDptrHV.ID && false == fFound; IDptrHV = pTabRV->GetNext() )
		{
			CDB_HubValv *pHV = (CDB_HubValv *)( IDptrHV.MP );
			if( pHV->GetSupplyOrReturn() != CDB_HubValv::eSupplyReturnValv::Return )
				continue;
			
			if( !( (CDB_TAProduct *)( pHV->GetValveIDPtr().MP ) )->IsSelectable( true, true ) )
				continue;
			
			CString str = ( (CDB_StringID *)( Idptr.MP ) )->GetIDstr2();
			if( 0 == str.CompareNoCase( ( (CDB_TAProduct *)( pHV->GetValveIDPtr().MP ) )->GetTypeID() ) )
				fFound = true;
		}
		
		if( false == fFound )
			continue;
		rkList.Add( ( (CDB_StringID *)( Idptr.MP ) )->GetString(), _ttoi( ( (CDB_StringID *)( Idptr.MP ) )->GetIDstr() ), (LPARAM)Idptr.MP );
	}
	
	m_ComboPartnerType.ResetContent();
	rkList.Transfer( &m_ComboPartnerType );

	// Select the first element.
	if( m_ComboPartnerType.GetCount() > 0 )
		m_ComboPartnerType.SetCurSel( 0 );
}

void CDlgIndSelHub::FillComboApplication()
{
	// Combo is filled with strings Select One..., Heating, Cooling.  
	// "Select one" is selected by default, and invalidate all other fields
	// When a selection is done "Select one" is removed and the user keep the choice between Heating and Cooling
	// Invalidate all fields
	m_bLocked = true;
	m_radioFlow.EnableWindow( false );				
	m_radioFlowLoopDP.EnableWindow( false );			
	m_radioDirectSelection.EnableWindow( false );		
	m_radioFlowLoopDetails.EnableWindow( false );		
	m_radioLeft.EnableWindow( false );				
	m_radioRight.EnableWindow( false );				
	m_ComboPartnerType.EnableWindow( false );		
	m_ComboStationConnect.EnableWindow( false );		
	m_ComboNumberOfStation.EnableWindow( false );

	GetDlgItem( IDC_NUMBEROFSTATIONS)->EnableWindow( false );
	GetDlgItem( IDC_SUPPLYRETURNSIDE)->EnableWindow( false );
	GetDlgItem( IDC_PARTNERVALVETYPE)->EnableWindow( false );
	GetDlgItem( IDC_SELECTIONMODE)->EnableWindow( false );
	GetDlgItem( IDC_STATIONCONNECT)->EnableWindow( false );
	GetDlgItem( IDC_STATICMVLOC)->EnableWindow( false );
//	GetDlgItem( IDC_GROUPSELECTIONMODE)->EnableWindow( false ); // JCC: Enable of XGroupBox must be made with its own methods

	// Fill Combo Application
	m_ComboAppli.ResetContent();
	CString str; 
	str = TASApp.LoadLocalizedString( IDS_SSELTAHUB_SELONE );
	short nIndex = m_ComboAppli.AddString( str );
	m_ComboAppli.SetItemData( nIndex, 0 );
	str = TASApp.LoadLocalizedString( IDS_SSELTAHUB_HEATING);
	nIndex = m_ComboAppli.AddString( str );
	m_ComboAppli.SetItemData( nIndex, CDS_HmHub::Application::Heating );
	str = TASApp.LoadLocalizedString( IDS_SSELTAHUB_COOLING );
	nIndex = m_ComboAppli.AddString( str );
	m_ComboAppli.SetItemData( nIndex, CDS_HmHub::Application::Cooling );
	// "Select One" item by default
	m_ComboAppli.SetCurSel( 0 );
}

void CDlgIndSelHub::FillComboConnect()
{
	CTable *pConnTab = (CTable *)( m_clIndSelHUBParams.m_pTADB->Get( _T("HUBCONNECT_TAB") ).MP );				ASSERT( pConnTab );
	if( NULL == pConnTab )
		return;
	
	CRank RKList;
	m_ComboStationConnect.ResetContent();
	for( IDPTR IDPtr = pConnTab->GetFirst(); NULL != *IDPtr.ID; IDPtr = pConnTab->GetNext() )
	{
		CDB_MultiStringExt *pMS = (CDB_MultiStringExt *)( IDPtr.MP );						ASSERT( true == IDPtr.MP->IsClass( CLASS( CDB_MultiStringExt ) ) );
		if( false == pMS->IsAvailable() )
			continue;
		
		CString str = pMS->GetString( CDS_HmHub::ExtConn::eHubPartner );
		if( 0 == str.CompareNoCase( _T("n") ) )
		{
			CString Name, str;
			double dKey;
			Name = pMS->GetString( CDS_HmHub::ExtConn::eConnectName );
			str = pMS->GetString( CDS_HmHub::ExtConn::eConnOrder );
			dKey = _tstof( (LPCTSTR)str );
			RKList.Add( Name, dKey, (LPARAM)IDPtr.MP );
		}
	}
	
	RKList.Transfer( &m_ComboStationConnect );
	m_ComboStationConnect.SetCurSel( 0 );
	if( 1 == m_ComboStationConnect.GetCount() )
		m_ComboStationConnect.EnableWindow( false );
}

void CDlgIndSelHub::FillComboMvLoc()
{
	m_ComboMvLoc.ResetContent();
	m_ComboMvLoc.EnableWindow( false );
	GetDlgItem( IDC_STATICMVLOC )->EnableWindow( false );
	CString BalTypeID =( (CDB_StringID *)m_ComboPartnerType.GetItemDataPtr( m_ComboPartnerType.GetCurSel() ) )->GetIDstr2();
	if( 0 == BalTypeID.CompareNoCase( _T("DPCTYPE_STD") ) )
	{
		CString str;
		str = TASApp.LoadLocalizedString( IDS_PRIMARY );
		m_ComboMvLoc.InsertString( 0, str );
		m_ComboMvLoc.SetItemData( 0, eMvLoc::MvLocPrimary );
		str = TASApp.LoadLocalizedString( IDS_SECONDARY );
		m_ComboMvLoc.InsertString( 1, str );
		m_ComboMvLoc.SetItemData( 1, eMvLoc::MvLocSecondary );
		
		// By default
		m_ComboMvLoc.SetCurSel( 0 );
		
		// Retrieve current measuring valve location
		CDS_HmHub *pHub = (CDS_HmHub *)( m_pHubHandler->GetHubIDPtr().MP );
		if( NULL != pHub && NULL != pHub->GetpDpC() )
		{
			for( int i = 0; i < m_ComboMvLoc.GetCount(); i++ )
			{
				if( m_ComboMvLoc.GetItemData( i ) == pHub->GetpDpC()->GetMvLoc() )
				{
					m_ComboMvLoc.SetCurSel( i );
					break;
				}
			}
		}
		
		// If Measuring valve is located on secondary side by default, user can't change to primary
		if( eMvLoc::MvLocSecondary == m_clIndSelHUBParams.m_pTADS->GetpTechParams()->GetDpCMvLoc() )
			return;
		
		//If measuring valve is located on secondary, and primary is not a correct selection disable combo
		if( eMvLoc::MvLocSecondary == pHub->GetpDpC()->GetMvLoc() )
		{
			CDB_DpController *pDpC = (CDB_DpController *)( pHub->GetpDpC()->GetIDPtr().MP );
			if( NULL != pDpC && ( pDpC->GetDplmin() > pHub->GetpDpC()->GetDpToStab( eMvLoc::MvLocPrimary ) ) )
				return;
		}
		m_ComboMvLoc.EnableWindow( true );
		GetDlgItem( IDC_STATICMVLOC )->EnableWindow( true );
	}
}

void CDlgIndSelHub::CheckSelectButton( CDS_HmHub *pHmHub )
{
	m_ButtonSelect.EnableWindow( false );
	if( NULL == pHmHub )
		return;
	
	if( CDS_HmHub::SelMode::Direct == pHmHub->GetSelMode() )
	{
		m_ButtonSelect.EnableWindow( true );
		return;
	}

	if( true == pHmHub->IsCompletelyDefined() )
	{
		CString str = pHmHub->CheckMaxTotalFlow();
		if( true == str.IsEmpty() )
		{	
			m_ButtonSelect.EnableWindow( true );
			m_TTSelectBut.Activate( true );
		}
		else
			m_TTSelectBut.Activate( true );
	}
	else
		m_TTSelectBut.Activate( true );
}

BOOL CDlgIndSelHub::OnInitDialog() 
{
	CDlgIndSelBase::OnInitDialog();
	
	pDlgIndSelHub = this;

	// Set proper style and add icons to Qdp and valve groups.
	CImageList* pclImgListGroupBox = TASApp.GetpRCImageManager()->GetImageList( CRCImageManager::ILN_GroupBox );		ASSERT ( NULL != pclImgListGroupBox );
	if( NULL != pclImgListGroupBox )
		m_GroupTAHub.SetImageList( pclImgListGroupBox, CRCImageManager::ILGB_Hub );
	m_GroupTAHub.SetInOffice2007Mainframe( true );
	
	if( NULL != pclImgListGroupBox )
		m_GroupSelectionMode.SetImageList( pclImgListGroupBox, CRCImageManager::ILGB_Pen );
	m_GroupSelectionMode.SetInOffice2007Mainframe( true );

	CString str;
	str = TASApp.LoadLocalizedString( IDS_SSELTAHUB_NUMBEROFSTATIONS );
	GetDlgItem( IDC_NUMBEROFSTATIONS )->SetWindowText( str );
	str = TASApp.LoadLocalizedString( IDS_SSELTAHUB_SUPPLYRETURNSIDE );
	GetDlgItem( IDC_SUPPLYRETURNSIDE )->SetWindowText( str );
	str = TASApp.LoadLocalizedString( IDS_SSELTAHUB_RADIOLEFT );
	GetDlgItem( IDC_RADIOLEFT )->SetWindowText( str );
	str = TASApp.LoadLocalizedString( IDS_SSELTAHUB_RADIORIGHT );
	GetDlgItem( IDC_RADIORIGHT )->SetWindowText( str );
	str = TASApp.LoadLocalizedString( IDS_SSELTAHUB_PARTNERVALVETYPE );
	GetDlgItem( IDC_PARTNERVALVETYPE )->SetWindowText( str );	
	str = TASApp.LoadLocalizedString( IDS_SSELTAHUB_SELECTIONMODE );
	GetDlgItem( IDC_SELECTIONMODE )->SetWindowText( str );
	str = TASApp.LoadLocalizedString( IDS_SSELTAHUB_RADIOFLOW );
	GetDlgItem( IDC_RADIOFLOW )->SetWindowText( str );
	str = TASApp.LoadLocalizedString( IDS_SSELTAHUB_RADIOFLOWANDLOOPDP );
	GetDlgItem( IDC_RADIOFLOWANDLOOPDP )->SetWindowText( str );
	str = TASApp.LoadLocalizedString( IDS_SSELTAHUB_RADIOFLOWANDLOOPDETAILS );
	GetDlgItem( IDC_RADIOFLOWANDLOOPDETAILS )->SetWindowText( str );
	str = TASApp.LoadLocalizedString( IDS_SSELTAHUB_RADIODIRECTSELECTION );
	GetDlgItem( IDC_RADIODIRECTSELECTION )->SetWindowText( str );
	str = TASApp.LoadLocalizedString( IDS_SSELTAHUB_APPLICATION );
	GetDlgItem( IDC_APPLICATION )->SetWindowText( str );
	str = TASApp.LoadLocalizedString( IDS_SSELTAHUB_STATIONCONNECT );
	GetDlgItem( IDC_STATIONCONNECT )->SetWindowText( str );
	str = TASApp.LoadLocalizedString( IDS_TABCDIALOGSSELH_STATICMVLOC );
	GetDlgItem( IDC_STATICMVLOC )->SetWindowText( str );
	str = TASApp.LoadLocalizedString( IDS_TABCDIALOGSSELH_SELECT );
	GetDlgItem( IDC_BUTTONSELECT )->SetWindowText( str );
	str = TASApp.LoadLocalizedString( IDS_TABCDIALOGSSELH_NEW );
	GetDlgItem( IDC_BUTTONNEW )->SetWindowText( str );
	
	// Set font for static IDC_STATICWARNING
	str = TASApp.LoadLocalizedString( IDS_TABCDIALOGSSELH_KEEPUPTODATE );
	( (CStatic*)GetDlgItem( IDC_STATICWARNING) )->SetWindowText( str );	
	VERIFY( m_font.CreateFont(
			-9,								// nHeight
			0,								// nWidth
			0,								// nEscapement
			0,								// nOrientation
			FW_DONTCARE,					// nWeight
			FALSE,							// bItalic
			FALSE,							// bUnderline
			0,								// cStrikeOut
			ANSI_CHARSET,					// nCharSet
			OUT_DEFAULT_PRECIS,				// nOutPrecision
			CLIP_DEFAULT_PRECIS,			// nClipPrecision
			DEFAULT_QUALITY,				// nQuality
			DEFAULT_PITCH | FF_SWISS,		// nPitchAndFamily
			_T("Arial") ) );                 // lpszFacename
	
	( (CStatic*)GetDlgItem( IDC_STATICWARNING ) )->SetFont( &m_font );
	
	if( NULL != m_ToolTip.GetSafeHwnd() )
	{
		str = TASApp.LoadLocalizedString( IDS_MVEXCL );
		m_ToolTip.AddToolWindow( &m_ComboMvLoc, str );
	}

	m_TTSelectBut.Create( this, TTS_NOPREFIX | TTS_ALWAYSTIP );
	
	CRect rect;
	m_ButtonSelect.GetClientRect( &rect );
	
	// Set the default value for the elements.
	ResetToDefault();

	return TRUE;  // return TRUE unless you set the focus to a control
				  // EXCEPTION: OCX Property Pages should return FALSE
}

void CDlgIndSelHub::OnButtonselect() 
{
	if( NULL != m_pHubHandler )
	{
		CDlgConfSel dlg( &m_clIndSelHUBParams );
		CDS_HmHub *pHmHub = (CDS_HmHub*)( m_pHubHandler->GetHubIDPtr().MP );
	
		dlg.Display( (CDS_HmHub*)( m_pHubHandler->GetHubIDPtr().MP ) );

		if( IDOK == dlg.DoModal() )
		{
			CTable *pTab = m_clIndSelHUBParams.m_pTADS->GetpHUBSelectionTable();
			int iPos = pTab->GetItemCount();
			
			// Copy function created a new Hub into the TMPHUB_Tab, retrieve it
			CTable *pTmpTab = (CTable *)( m_clIndSelHUBParams.m_pTADS->Get( _T("TMPHUB_TAB") ).MP );
			IDPTR idptr = pTmpTab->GetFirst( CLASS( CDS_HmHub ) );
			
			if( NULL == *idptr.ID )	//Not Found!!!
			{
				ASSERT( 0 );
				OnBnClickedButtonnew();
			}
			else
			{
				m_pHubHandler->Attach( idptr.ID );
				pHmHub = (CDS_HmHub*)( idptr.MP );
				pHmHub->SetPos( iPos );
				CString str = pHmHub->RenameMe( false );
				if( NULL != m_clIndSelHUBParams.m_pTADS->FindHydroMod( str, pTab, pHmHub ) )
					m_clIndSelHUBParams.m_pTADS->DifferentiateHMName( pTab, pHmHub->GetHMName() );
				UpdateValues();
			}

			if( NULL != pRViewSSelHub )
			{
				pRViewSSelHub->Invalidate();
			}
		}
	}
}

void CDlgIndSelHub::OnButtonmodwater() 
{
	// Display the dialog
	CDlgWaterChar dlg;
	dlg.Display();
}

void CDlgIndSelHub::OnBnClickedRadioMainSupplyReturnPipe()
{
	UpdateValues();
}

void CDlgIndSelHub::OnBnClickedRadioSelectionMode()
{
	UpdateValues();

	// Force OnSize message, draw scroll bars if needed
	if( NULL != pRViewSSelHub )
	{
		pRViewSSelHub->SendMessage( WM_SIZE, SIZE_RESTORED, 0 );
	}
}

void CDlgIndSelHub::OnBnClickedButtonnew()
{
	ResetToDefault();
	UpdateValues( true );
}

void CDlgIndSelHub::OnCbnSelchangeCombooptions()
{
	UpdateValues();
}

void CDlgIndSelHub::OnCbnSelchangeCombonumberofstations()
{
	UpdateValues();
}

void CDlgIndSelHub::OnCbnSelchangeComboapplication()
{
	// Heating or Cooling selected
	if( m_ComboAppli.GetItemData( m_ComboAppli.GetCurSel() ) )
	{
		// Remove "Select One" item if exist ItemData = 0
		for( int i = 0; i < m_ComboAppli.GetCount(); i++ )
		{
			if( m_ComboAppli.GetItemData( i ) == 0 )
			{
				m_ComboAppli.DeleteString( i );
				break;
			}
		}
		
		// Re-enable user's fields
		m_bLocked = false;
		m_radioFlow.EnableWindow( true );				
		m_radioFlowLoopDP.EnableWindow( true );			
		m_radioDirectSelection.EnableWindow( true );		
		m_radioFlowLoopDetails.EnableWindow( true );		
		m_radioLeft.EnableWindow( true );				
		m_radioRight.EnableWindow( true );
		if (m_ComboStationConnect.GetCount()>1)
			m_ComboStationConnect.EnableWindow( true );
		m_ComboPartnerType.EnableWindow( true );		
		m_ComboNumberOfStation.EnableWindow( true );

		GetDlgItem( IDC_NUMBEROFSTATIONS)->EnableWindow( true );
		GetDlgItem( IDC_SUPPLYRETURNSIDE)->EnableWindow( true );
		GetDlgItem( IDC_PARTNERVALVETYPE)->EnableWindow( true );
		GetDlgItem( IDC_SELECTIONMODE)->EnableWindow( true );
		GetDlgItem( IDC_STATIONCONNECT)->EnableWindow( true );
		GetDlgItem( IDC_GROUPSELECTIONMODE)->EnableWindow( true );
		CDS_HmHub *pHub = dynamic_cast<CDS_HmHub *>( m_pHubHandler->GetHubIDPtr().MP );
		ASSERT( pHub );
		if( NULL != pHub )
			pHub->SetApplication( (CDS_HmHub::Application)m_ComboAppli.GetItemData( m_ComboAppli.GetCurSel() ) );
	}
}

void CDlgIndSelHub::OnCbnSelchangeCombostationconnect()
{
	UpdateValues( false );
}

void CDlgIndSelHub::OnCbnSelchangeCombopartnertype()
{
	UpdateValues();
}

void CDlgIndSelHub::OnCbnSelchangeCombomvloc()
{
	// update measuring valve location
	CDS_HmHub *pHub = (CDS_HmHub *)( m_pHubHandler->GetHubIDPtr().MP );
	if( NULL != pHub && NULL != pHub->GetpDpC() )
	{
		eMvLoc MvLoc = (eMvLoc)m_ComboMvLoc.GetItemData( m_ComboMvLoc.GetCurSel() );
		
		//TO BE DONE; set a variable to force MvLocalisation 
		pHub->GetpDpC()->SetMvLoc( MvLoc );
		pHub->GetpDpC()->SetMvLocLocked( true );
		pHub->ComputeHM( CDS_HydroMod::eComputeHMEvent::eceResize );
		pHub->ComputeAll();
	
		if( NULL != pRViewSSelHub )
		{
			pRViewSSelHub->SetRedraw( pHub );
		}
	
		// Refresh measuring valve position
		FillComboMvLoc();
		CheckSelectButton( pHub );
	}
}

LRESULT CDlgIndSelHub::OnUnitChange( WPARAM wParam, LPARAM lParam )
{
	// Verify the dialog is active
	if( FALSE == this->IsWindowVisible() )
		return 0;

	TCHAR name[_MAXCHARS];
	CUnitDatabase *pUnitDB = CDimValue::AccessUDB();

	if( false == TASApp.IsHubDisplayed() )
		return 0;
	
	// Set the text for the flow units
	GetNameOf( pUnitDB->GetDefaultUnit( _U_FLOW ), name );
	SetDlgItemText( IDC_STATICQUNIT, name );

	// Relaunch the suggestion procedure
	UpdateValues();
	
	return 0;
}

LRESULT CDlgIndSelHub::OnWaterChange( WPARAM wParam, LPARAM lParam )
{
	if( WMUserWaterCharWParam::WM_UWC_WP_ForProductSel != ( (WMUserWaterCharWParam)wParam & WMUserWaterCharWParam::WM_UWC_WP_ForProductSel ) )
	{
		return 0;
	}

	// HYS-1693: Not for changeOver application type
	if( WMUserWaterCharLParam::WM_UWC_LWP_ChangeForChangeOver == (WMUserWaterCharLParam)lParam
		|| WMUserWaterCharLParam::WM_UWC_LWP_NoChangeForChangeOver == (WMUserWaterCharLParam)lParam )
	{
		return 0;
	}

	// Verify the dialog is active
	if( FALSE == this->IsWindowVisible() )
		return 0;

	// Relaunch the suggestion procedure
	if( false == m_bRViewEmpty )	// RightView is not empty for SSelHub
	{
		UpdateValues();
	}

	return 0;
}

LRESULT CDlgIndSelHub::OnChangefocusRViewssel( WPARAM wParam, LPARAM lParam )
{
	// ??????????????
// 	if( wParam != 1 )
// 		return 0;
	return 0;
}

LRESULT CDlgIndSelHub::OnNewDocument( WPARAM wParam, LPARAM lParam )
{
	CDlgIndSelBase::OnNewDocument( wParam, lParam );

	if( false == TASApp.IsHubDisplayed() )
		return 0;
		
	m_pHubHandler->Detach();
	OnUnitChange();
	ResetToDefault();
	m_clIndSelHUBParams.m_pTADS->Modified( false );
	m_bInitialised = true;
	m_fNewDoc = true;

	// Verify the dialog is active
	if( FALSE == this->IsWindowVisible() )
		return 0;

	UpdateValues( true );
	m_fNewDoc = false;
	
	return 0;
}

LRESULT CDlgIndSelHub::OnTechParChange( WPARAM wParam, LPARAM lParam )
{
	if( false == TASApp.IsHubDisplayed() )
		return 0;
	
	// Relaunch the suggestion procedure
	if( false == m_bRViewEmpty )	// RightView is not empty for SSelHub
		UpdateValues();

	return 0;
}

LRESULT CDlgIndSelHub::OnModifySelectedHub( WPARAM wParam, LPARAM lParam )
{
	if( false == TASApp.IsHubDisplayed() )
	{
		return 0;
	}
	
	CDS_HmHub *pHub = (CDS_HmHub *)lParam;
	ASSERT( NULL != pHub );
	
	if( NULL == pHub || NULL == pDlgLTtabctrl )
	{
		return 0;
	}
	
	CHubHandler HubHandler;
	HubHandler.Attach( pHub->GetIDPtr().ID );
	
	// Create a copy of hub that will be modified; warn the user if a modification is in course
	pHub = HubHandler.ModifySelectedProduct();
	
	if( NULL == pHub )
	{
		return 0;
	}
	
	m_pHubHandler->Attach( pHub->GetIDPtr().ID );
	m_bInitialised = false;

	// Send message to notify the display of the SSel tab.
	pDlgLTtabctrl->SetActiveTab( CMyMFCTabCtrl::etiSSel );

	// If we are not in the good selection mode, we must first disable it.
	if( ProductSelectionMode_Individual != pMainFrame->GetCurrentProductSelectionMode() )
	{
		pMainFrame->ChangeProductSelectionMode( ProductSelectionMode_Individual );
	}

	// Select the right item in the gallery box.
	pMainFrame->ClickRibbonProductSelGallery( ProductSubCategory::PSC_BC_HUB );

	// Select SSelH
	CString str; str = TASApp.LoadLocalizedString( IDS_COMBOSSELTAHUB );
	CString str_cur;

	// Refresh all controls according to the modified Hub,
	// a copy of this Hub is already stored into the temporary table
	ResetToDefault();
	
	// Application
	CDS_HmHub::Application Appli = pHub->GetApplication();
	for( int i = 0; i < m_ComboAppli.GetCount(); i++ )
	{
		if( m_ComboAppli.GetItemData( i ) == (DWORD_PTR)Appli )
		{
			m_ComboAppli.SetCurSel( i );
			break;
		}
	}
	OnCbnSelchangeComboapplication();
	
	// Number of Stations
	int iNbr = pHub->GetCount();
	for( int i = 0; i < m_ComboNumberOfStation.GetCount(); i++ )
	{
		if( m_ComboNumberOfStation.GetItemData( i ) == (DWORD_PTR)iNbr )
		{
			m_ComboNumberOfStation.SetCurSel( i );
			break;
		}
	}
	
	// Main supply side
	m_radioLeft.SetCheck( pHub->GetMainFeedPos() == CDS_HmHub::MainFeedPos::Left );
	m_radioRight.SetCheck( pHub->GetMainFeedPos() == CDS_HmHub::MainFeedPos::Right );
	
	// Partner Valve Type
	CString strBalTypeID = pHub->GetBalTypeID();
	for( int i = 0; i < m_ComboPartnerType.GetCount(); i++ )
	{
		if( 0 == strBalTypeID.CompareNoCase( ( (CDB_StringID *)m_ComboPartnerType.GetItemData( i ) )->GetIDstr2() ) )
		{
			m_ComboPartnerType.SetCurSel( i );
			break;
		}
	}
	
	// Measuring valve location
	if( true == pHub->IsDpCExist() )
	{
		for( int i = 0; i < m_ComboMvLoc.GetCount(); i++ )
		{
			if( m_ComboMvLoc.GetItemData( i ) == (DWORD_PTR)pHub->GetpDpC()->GetMvLoc() )
			{
				m_ComboMvLoc.SetCurSel( i );
				break;
			}
		}
	}
	
	// Connection
	CData *pData = pHub->GetStationsConnectIDPtr().MP;
	for( int i = 0; i < m_ComboStationConnect.GetCount(); i++ )
	{
		if( m_ComboStationConnect.GetItemData( i ) == (DWORD_PTR)pData )
		{
			m_ComboStationConnect.SetCurSel( i );
			break;
		}
	}

	// Selection Mode
	m_radioFlow.SetCheck( pHub->GetSelMode() == CDS_HmHub::SelMode::Q );
	m_radioFlowLoopDP.SetCheck( pHub->GetSelMode() == CDS_HmHub::SelMode::QDpLoop );
	m_radioFlowLoopDetails.SetCheck( pHub->GetSelMode() == CDS_HmHub::SelMode::LoopDetails );
	m_radioDirectSelection.SetCheck( pHub->GetSelMode() == CDS_HmHub::SelMode::Direct );
	
	// Water Characteristics
	m_clIndSelHUBParams.m_WC = *pHub->GetpWC();
	::AfxGetApp()->m_pMainWnd->SendMessageToDescendants( WM_USER_WATERCHANGE, (WPARAM)WMUserWaterCharWParam::WM_UWC_WP_ForProductSel );

	m_bInitialised = true;
	UpdateValues();
	TASApp.PumpMessages();

	if( NULL != pRViewSSelHub )
	{
		pRViewSSelHub->Invalidate();
	}

	return 0;
}

// Overrides CDlgIndSelBase
// PAY ATTENTION: CRViewSSelHub is not inherited from CRViewSSelSS !!!
CRViewSSelSS *CDlgIndSelHub::GetLinkedRightViewSSel( void ) { return NULL; }
