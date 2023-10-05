#include "stdafx.h"

#include <float.h>
#include <math.h>
#include "TASelect.h"
#include "MainFrm.h"
#include "Global.h"
#include "Hydronic.h"
#include "Select.h"

#include "DlgNoDevFound.h"
#include "DlgConfSel.h"
#include "DlgNoDpCFound.h"
#include "DlgNoDevFound.h"
#include "DlgInfoSSelDpC.h"

#include "DlgLeftTabSelManager.h"

#include "RViewSSelSS.h"
#include "RViewSSelDpC.h"

#include "DlgSelectionBase.h"
#include "DlgIndSelBase.h"
#include "DlgIndSelDpC.h"

#ifdef _DEBUG
#define new DEBUG_NEW
#undef THIS_FILE
static char THIS_FILE[] = __FILE__;
#endif

CDlgIndSelDpC::CDlgIndSelDpC( CWnd* pParent )
	: CDlgIndSelBase( m_clIndSelDpCParams, CDlgIndSelDpC::IDD, pParent )
{
	m_clIndSelDpCParams.m_eProductSubCategory = ProductSubCategory::PSC_BC_DpController;
	m_clIndSelDpCParams.m_eProductSelectionMode = ProductSelectionMode::ProductSelectionMode_Individual;

	m_ComboKvRect.SetRectEmpty();
	m_pInfoDialogDpC = NULL;
}

CDlgIndSelDpC::~CDlgIndSelDpC()
{
	if( m_pInfoDialogDpC != NULL )
	{
		delete m_pInfoDialogDpC;
	}
}

void CDlgIndSelDpC::SetDpMax( double dDpMax )
{
	if( dDpMax > 0.0 )
	{
		m_clIndSelDpCParams.m_dDpMax = dDpMax;
		m_clExtEditDpMax.SetWindowText( WriteCUDouble( _U_DIFFPRESS, dDpMax ) );
	}
	else
	{
		m_clIndSelDpCParams.m_dDpMax = 0.0;
		m_clExtEditDpMax.SetWindowText( _T("") );
	}
}

void CDlgIndSelDpC::SaveSelectionParameters()
{
	CDlgIndSelBase::SaveSelectionParameters();
	CDS_IndSelParameter *pclIndSelParameter = m_clIndSelDpCParams.m_pTADS->GetpIndSelParameter();

	if( NULL == pclIndSelParameter )
	{
		return;
	}

	pclIndSelParameter->SetDpCRadioFlowPowerDT( (int)m_clIndSelDpCParams.m_eFlowOrPowerDTMode );
	pclIndSelParameter->SetDpCDpCheckBox( (int)m_clIndSelDpCParams.m_bIsGroupDpbranchOrKvChecked );
	pclIndSelParameter->SetDpCDpMaxCheckBox( (int)m_clIndSelDpCParams.m_bIsDpMaxChecked );
	pclIndSelParameter->SetDpCDpStab( (int)m_clIndSelDpCParams.m_eDpStab );
	pclIndSelParameter->SetDpCLoc( (int)m_clIndSelDpCParams.m_eDpCLoc );
	pclIndSelParameter->SetDpCMvLoc( (int)m_clIndSelDpCParams.m_eMvLoc );
	pclIndSelParameter->SetDpCTypeID( m_clIndSelDpCParams.m_strComboTypeID );
	pclIndSelParameter->SetDpCFamilyID( m_clIndSelDpCParams.m_strComboFamilyID );
	pclIndSelParameter->SetDpCMaterialID( m_clIndSelDpCParams.m_strComboMaterialID ); 
	pclIndSelParameter->SetDpCConnectID( m_clIndSelDpCParams.m_strComboConnectID );
	pclIndSelParameter->SetDpCVersionID( m_clIndSelDpCParams.m_strComboVersionID );
	pclIndSelParameter->SetDpCPNID( m_clIndSelDpCParams.m_strComboPNID );
	pclIndSelParameter->SetDpCPackageChecked( (int)m_clIndSelDpCParams.m_bOnlyForSet );
	pclIndSelParameter->SetDpCInfoWndChecked( (int)m_clIndSelDpCParams.m_bIsDpCInfoWndChecked );

	if( NULL != pRViewSSelDpC )
	{
		pRViewSSelDpC->SaveSelectionParameters( pclIndSelParameter );
	}
}

void CDlgIndSelDpC::LeaveLeftTabDialog()
{
	if( NULL != m_pInfoDialogDpC->GetSafeHwnd() )
	{
		m_pInfoDialogDpC->DestroyWindow();
	}

	CDlgIndSelBase::LeaveLeftTabDialog();
}

void CDlgIndSelDpC::ActivateLeftTabDialog()
{
	OnBnClickedCheckShowInfoWnd();
	CDlgIndSelBase::ActivateLeftTabDialog();

	// HYS-1263 : Added to update application type if it is necessary
	CDS_TechnicalParameter *pTechP = m_clIndSelDpCParams.m_pTADS->GetpTechParams();
	ASSERT( NULL != pTechP );

	ProjectType eCurrentPSApplicationType = pTechP->GetProductSelectionApplicationType();

	if( eCurrentPSApplicationType != m_clIndSelDpCParams.m_eApplicationType )
	{
		ChangeApplicationType( eCurrentPSApplicationType );
	}
}

void CDlgIndSelDpC::ChangeApplicationType( ProjectType eProductSelectionApplicationType )
{
	CDlgIndSelBase::ChangeApplicationType( eProductSelectionApplicationType );

	if( NULL != pRViewSSelDpC )
	{
		pRViewSSelDpC->Reset();
	}
}

//////////////////////////////////////////////////////////////////////////////////////////////////////////////
// PROTECTED METHODS
//////////////////////////////////////////////////////////////////////////////////////////////////////////////

BEGIN_MESSAGE_MAP( CDlgIndSelDpC, CDlgIndSelBase )
	ON_CBN_SELENDOK( IDC_COMBODPSTAB, OnCbnSelChangeDpStab )
	ON_CBN_SELENDOK( IDC_COMBOMVLOC, OnCbnSelChangeMvLocation )
	ON_CBN_SELENDOK( IDC_COMBODPCLOC, OnCbnSelChangeDpCLocation )
	ON_CBN_SELENDOK( IDC_COMBOTYPE, OnCbnSelChangeType )
	ON_CBN_SELENDOK( IDC_COMBOFAMILY, OnCbnSelChangeFamily )
	ON_CBN_SELENDOK( IDC_COMBOMATERIAL, OnCbnSelChangeMaterial )
	ON_CBN_SELENDOK( IDC_COMBOCONNECT, OnCbnSelChangeConnect )
	ON_CBN_SELENDOK( IDC_COMBOVERSION, OnCbnSelChangeVersion )
	ON_CBN_SELENDOK( IDC_COMBOPN, OnCbnSelChangePN )
	ON_CBN_SELENDOK( IDC_COMBOKV, OnCbnSelChangeKv )

	ON_CBN_EDITCHANGE( IDC_COMBOKV, OnEditChangeComboKv )
	ON_EN_CHANGE( IDC_EDITDPMAX, OnEnChangeDpMax )

	ON_EN_KILLFOCUS( IDC_EDITDPBRANCH, OnEnKillFocusDpBranch )
	ON_CBN_KILLFOCUS( IDC_COMBOKV, OnKillFocusComboKv )
	ON_EN_KILLFOCUS( IDC_EDITDPMAX, OnEnKillFocusDpMax )

	ON_BN_CLICKED( IDC_GROUPKVDP, OnBnClickedCheckGroupDpBranchOrKv )
	ON_BN_CLICKED( IDC_GROUPDPMAX, OnBnClickedCheckDpMax )
	ON_BN_CLICKED( IDC_CHECKSHOWINFOWND, OnBnClickedCheckShowInfoWnd )
	ON_BN_CLICKED( IDC_CHECKPACKAGE, OnBnClickedCheckboxSet )

	ON_NOTIFY( WM_USER_ENTERKEYPRESSED, IDC_COMBOKV, OnEditEnterChar )
	ON_NOTIFY( WM_USER_ENTERKEYPRESSED, IDC_EDITDPBRANCH, OnEditEnterChar )
	ON_NOTIFY( WM_USER_ENTERKEYPRESSED, IDC_EDITDPMAX, OnEditEnterChar )

	ON_MESSAGE( WM_USER_MODIFYSELECTEDDPC, OnModifySelectedDpC )
	ON_MESSAGE( WM_USER_DESTROYDIALOGINFOSSELDPC, OnDestroyInfoDialogDpC )
END_MESSAGE_MAP()

void CDlgIndSelDpC::DoDataExchange( CDataExchange* pDX )
{
	CDlgIndSelBase::DoDataExchange( pDX );
	DDX_Control( pDX, IDC_COMBODPSTAB, m_ComboDpStab );
	DDX_Control( pDX, IDC_COMBOMVLOC, m_ComboMvLoc );
	DDX_Control( pDX, IDC_COMBODPCLOC, m_ComboDpCLoc );
	DDX_Control( pDX, IDC_COMBOTYPE, m_ComboType );
	DDX_Control( pDX, IDC_COMBOFAMILY, m_ComboFamily );
	DDX_Control( pDX, IDC_COMBOMATERIAL, m_ComboBdyMat );
	DDX_Control( pDX, IDC_COMBOCONNECT, m_ComboConnect );
	DDX_Control( pDX, IDC_COMBOVERSION, m_ComboVersion );
	DDX_Control( pDX, IDC_COMBOPN, m_ComboPN );
	DDX_Control( pDX, IDC_COMBOKV, m_ComboKv );
	DDX_Control( pDX, IDC_GROUPDPC, m_GroupDpC );
	DDX_Control( pDX, IDC_GROUPKVDP, m_GroupDpOrKv );
	DDX_Control( pDX, IDC_GROUPDPMAX, m_GroupDpMax );
	DDX_Control( pDX, IDC_GROUPCONNSCHEME, m_GroupConnScheme );
	DDX_Control( pDX, IDC_CHECKPACKAGE, m_CheckboxSet );
	DDX_Control( pDX, IDC_CHECKSHOWINFOWND, m_CheckInfoWnd );
	DDX_Control( pDX, IDC_STATICKVRANGE, m_StaticKvRange );
	DDX_Control( pDX, IDC_STATICDPBRANCHUNIT, m_StaticDpBranchUnit );

	// 'CDlgIndSelBase' variables.
	DDX_Control( pDX, IDC_EDITFLOW, m_clExtEditFlow );
	DDX_Control( pDX, IDC_EDITPOWER, m_clExtEditPower );
	DDX_Control( pDX, IDC_EDITDT, m_clExtEditDT );
	DDX_Control( pDX, IDC_EDITDPBRANCH, m_clExtEditDpBranch );
	DDX_Control( pDX, IDC_EDITDPMAX, m_clExtEditDpMax );
	DDX_Control( pDX, IDC_GROUPQ, m_clGroupQ );
	DDX_Radio( pDX, IDC_RADIOFLOW, m_iRadioFlowPowerDT );
}

BOOL CDlgIndSelDpC::OnInitDialog() 
{
	CDlgIndSelBase::OnInitDialog();

	// Initialization of flow and power static text is done in the 'CDlgSelectionBase' base class.
	CString str;
	str = TASApp.LoadLocalizedString( IDS_TABCDIALOGSSELDPC_STATICDPSTAB );
	GetDlgItem( IDC_STATICDPSTAB )->SetWindowText( str );
	
	str = TASApp.LoadLocalizedString( IDS_TABCDIALOGSSELDPC_STATICDPCLOC );
	GetDlgItem( IDC_STATICDPCLOC )->SetWindowText( str );
	
	str = TASApp.LoadLocalizedString( IDS_TABCDIALOGSSELDPC_STATICMVLOC );
	GetDlgItem( IDC_STATICMVLOC )->SetWindowText( str );
	
	str = TASApp.LoadLocalizedString( IDS_TABCDIALOGSSELDPC_STATICFAMILY );
	GetDlgItem( IDC_STATICFAMILY )->SetWindowText( str );
	
	str = TASApp.LoadLocalizedString( IDS_TABCDIALOGSSELDPC_STATICMATERIAL );
	GetDlgItem( IDC_STATICMATERIAL )->SetWindowText( str );

	str = TASApp.LoadLocalizedString( IDS_TABCDIALOGSSELDPC_STATICCONNECT );
	GetDlgItem( IDC_STATICCONNECT )->SetWindowText( str );

	str = TASApp.LoadLocalizedString( IDS_TABCDIALOGSSELDPC_STATICVERSION );
	GetDlgItem( IDC_STATICVERSION )->SetWindowText( str );

	str = TASApp.LoadLocalizedString( IDS_TABCDIALOGSSELDPC_STATICPN );
	GetDlgItem( IDC_STATICPN )->SetWindowText( str );

	str = TASApp.LoadLocalizedString( IDS_TABCDIALOGSSELDPC_CHECKPACKAGE );
	GetDlgItem( IDC_CHECKPACKAGE )->SetWindowText( str );

	str = TASApp.LoadLocalizedString( IDS_TABCDIALOGSSELDPC_BUTTONSUGGEST );
	GetDlgItem( IDC_BUTTONSUGGEST )->SetWindowText( str );

	str = TASApp.LoadLocalizedString( IDS_TABCDIALOGSSELDPC_BUTTONSELECT );
	GetDlgItem( IDC_BUTTONSELECT )->SetWindowText( str ); 

	str = TASApp.LoadLocalizedString( IDS_TABCDIALOGSSELDPC_BUTTONCANCEL );
	GetDlgItem( IDC_BUTTONCANCEL )->SetWindowText( str );

	str = TASApp.LoadLocalizedString( IDS_TABCDIALOGSSELDPC_DT );
	GetDlgItem( IDC_STATICDT )->SetWindowText( str );

	str = TASApp.LoadLocalizedString( IDS_TABCDIALOGSSELDPC_CHECKINFOWND );
	GetDlgItem( IDC_CHECKSHOWINFOWND )->SetWindowText( str );

 	str = TASApp.LoadLocalizedString( IDS_TABCDIALOGSSELDPC_STATICTYPE );
 	GetDlgItem( IDC_STATICTYPE )->SetWindowText( str );

	m_StaticKvRange.SetWindowText( _T("") );

	// Add a tooltip to DpStab and MvLoc Combo boxes.
	CString TTstr;
	TTstr = TASApp.LoadLocalizedString( IDS_BRANCH );
	m_ToolTip.AddToolWindow( &m_ComboDpStab, TTstr );
	
	TTstr = TASApp.LoadLocalizedString( IDS_MVEXCL );
	m_ToolTip.AddToolWindow( &m_ComboMvLoc, TTstr );
	
	TTstr = TASApp.LoadLocalizedString( IDS_DPCLOCDOWNSTREAMTOOLTIP );
	m_ToolTip.AddToolWindow( &m_ComboDpCLoc, TTstr );

 	// Add a tooltip to Combo Type.
 	TTstr = TASApp.LoadLocalizedString( IDS_MAXDPINVALVE );
 	m_ToolTip.AddToolWindow( &m_ComboType, TTstr );

	// Set proper style and add icons to Qkvdp and pipe groups.
	m_GroupConnScheme.SetInOffice2007Mainframe( true );

	CImageList *pclImgListGroupBox = TASApp.GetpRCImageManager()->GetImageList( CRCImageManager::ILN_GroupBox );
	ASSERT( NULL != pclImgListGroupBox );
	
	if( NULL != pclImgListGroupBox )
	{
		m_GroupDpC.SetImageList( pclImgListGroupBox, CRCImageManager::ILGB_DpC );
	}
	
	m_GroupDpC.SetInOffice2007Mainframe( true );

	// PAY ATTENTION: if it's feature is reactivated, there is some changes to do in 'OnLButtonDown' (ask to FF).
// 	m_GroupDpC.SetExpandCollapseMode( true );
// 	m_GroupDpC.SetNotificationHandler( this );

	m_GroupDpOrKv.SetCheckBoxStyle( BS_AUTOCHECKBOX );
	
	if( NULL != pclImgListGroupBox )
	{
		m_GroupDpOrKv.SetImageList( pclImgListGroupBox, CRCImageManager::ILGB_Pen );
	}

	CImageList *pclImgListGroupBoxGrayed = TASApp.GetpRCImageManager()->GetImageList( CRCImageManager::ILN_GroupBoxGrayed );
	ASSERT( NULL != pclImgListGroupBoxGrayed );

	if( NULL != pclImgListGroupBoxGrayed )
	{
		m_GroupDpOrKv.SetImageList( pclImgListGroupBoxGrayed, CRCImageManager::ILGB_Pen, true );
	}
	
	m_GroupDpOrKv.SetInOffice2007Mainframe( true );

	// PAY ATTENTION: if it's feature is reactivated, there is some changes to do in 'OnLButtonDown' (ask to FF).
// 	m_GroupKvDp.SetExpandCollapseMode( true );
// 	m_GroupKvDp.SetNotificationHandler( this );

	if( NULL != pclImgListGroupBox )
	{
		m_clGroupQ.SetImageList( pclImgListGroupBox, CRCImageManager::ILGB_Q );
	}

	m_clGroupQ.SetInOffice2007Mainframe( true );

	m_GroupDpMax.SetCheckBoxStyle( BS_AUTOCHECKBOX );

	if( NULL != pclImgListGroupBox )
	{
		m_GroupDpMax.SetImageList( pclImgListGroupBox, CRCImageManager::ILGB_Pen );
	}

	if( NULL != pclImgListGroupBoxGrayed )
	{
		m_GroupDpMax.SetImageList( pclImgListGroupBoxGrayed, CRCImageManager::ILGB_Pen, true );
	}

	m_GroupDpMax.SetInOffice2007Mainframe( true );

	str = TASApp.LoadLocalizedString( IDS_LVSSELB_STATDPMAX );
	m_GroupDpMax.SetWindowText( str );
	
  	m_clExtEditDpBranch.SetPhysicalType( _U_DIFFPRESS );
  	m_clExtEditDpBranch.SetEditType( CNumString::eDouble, CNumString::ePositive );
	
 	m_clExtEditDpMax.SetPhysicalType( _U_DIFFPRESS );
 	m_clExtEditDpMax.SetEditType( CNumString::eDouble, CNumString::ePositive );

	// Create flying DpC info.
	if( NULL == m_pInfoDialogDpC )
	{
		m_pInfoDialogDpC = new CDlgInfoSSelDpC( &m_clIndSelDpCParams, this );

		if( NULL == m_pInfoDialogDpC )
		{
			return FALSE;
		}
	}

	// Store the original rect of ComboKv.
	m_ComboKv.GetWindowRect( m_ComboKvRect );

	return TRUE;
}

void CDlgIndSelDpC::OnCbnSelChangeDpStab()
{
	// No need to call 'ClearAll()'. We call 'OnBnClickedCheckGroupDpOrKv' method and this one already calls 'ClearAll()'.
	_UpdateDpStabilizedOn();
	OnBnClickedCheckShowInfoWnd();
}

void CDlgIndSelDpC::OnCbnSelChangeDpCLocation() 
{
	_UpdateDpCLocation();
	OnBnClickedCheckShowInfoWnd();

 	_FillComboType();
 	OnCbnSelChangeType();
}

void CDlgIndSelDpC::OnCbnSelChangeMvLocation() 
{
	_UpdateMvLocation();
	OnBnClickedCheckShowInfoWnd();
	ClearAll();
}

void CDlgIndSelDpC::OnCbnSelChangeType() 
{
	m_clIndSelDpCParams.m_strComboTypeID = m_ComboType.GetCBCurSelIDPtr().ID;
	_FillComboFamily();
	OnCbnSelChangeFamily();
}

void CDlgIndSelDpC::OnCbnSelChangeFamily()
{
	m_clIndSelDpCParams.m_strComboFamilyID = m_ComboFamily.GetCBCurSelIDPtr().ID;
	_FillComboBodyMat();
	OnCbnSelChangeMaterial();
}

void CDlgIndSelDpC::OnCbnSelChangeMaterial()
{
	m_clIndSelDpCParams.m_strComboMaterialID = m_ComboBdyMat.GetCBCurSelIDPtr().ID;
	_FillComboConnect();
	OnCbnSelChangeConnect();
}

void CDlgIndSelDpC::OnCbnSelChangeConnect() 
{
	m_clIndSelDpCParams.m_strComboConnectID = m_ComboConnect.GetCBCurSelIDPtr().ID;
	_FillComboVersion();
	OnCbnSelChangeVersion();
}

void CDlgIndSelDpC::OnCbnSelChangeVersion()
{
	m_clIndSelDpCParams.m_strComboVersionID = m_ComboVersion.GetCBCurSelIDPtr().ID;
	_FillComboPN();
	OnCbnSelChangePN();
}

void CDlgIndSelDpC::OnCbnSelChangePN() 
{
	m_clIndSelDpCParams.m_strComboPNID = m_ComboPN.GetCBCurSelIDPtr().ID;
	_UpdateStaticKvRange();
	ClearAll();
}

void CDlgIndSelDpC::OnCbnSelChangeKv() 
{
	m_clIndSelDpCParams.m_dKv = 0.0;

	if( RD_OK == ReadDouble( m_ComboKv, &m_clIndSelDpCParams.m_dKv ) && m_clIndSelDpCParams.m_dKv > 0.0 )
	{
		m_clIndSelDpCParams.m_dKv = CDimValue::CUtoSI( _C_KVCVCOEFF, m_clIndSelDpCParams.m_dKv );
	}

	ClearAll();
}

void CDlgIndSelDpC::OnEditChangeComboKv() 
{
	// Each time you modify this combobox, it will clear your selection.
	m_clIndSelDpCParams.m_dKv = 0.0;

	if( RD_OK == ReadDouble( m_ComboKv, &m_clIndSelDpCParams.m_dKv ) && m_clIndSelDpCParams.m_dKv > 0.0 )
	{
		m_clIndSelDpCParams.m_dKv = CDimValue::CUtoSI( _C_KVCVCOEFF, m_clIndSelDpCParams.m_dKv );
	}

	ClearAll();
}

void CDlgIndSelDpC::OnKillFocusComboKv() 
{
	if( GetFocus() == &m_GroupDpOrKv || GetFocus() == &m_ComboDpStab )
	{
		return;
	}
	
	double dValue;

	if( RD_NOT_NUMBER == ReadDouble( m_ComboKv, &dValue ) || dValue < 0.0 )
	{
		TASApp.AfxLocalizeMessageBox( AFXMSG_NON_POSITIVE_VALUE );
		m_ComboKv.SetFocus();
	}
}

void CDlgIndSelDpC::OnBnClickedCheckGroupDpBranchOrKv() 
{
	_UpdateGroupDpBranchOrKvChecked();
	_UpdateDpBranchFieldState();
	ClearAll();
}

void CDlgIndSelDpC::OnBnClickedCheckDpMax()
{
	_UpdateDpMaxFieldState();
	ClearAll();
}

void CDlgIndSelDpC::OnBnClickedCheckShowInfoWnd()
{
	if( FALSE == IsWindowVisible() || false == m_bInitialised )
	{
		return;
	}
	
	m_clIndSelDpCParams.m_bIsDpCInfoWndChecked = ( BST_CHECKED == m_CheckInfoWnd.GetCheck() ) ? true : false;

	if( true == m_clIndSelDpCParams.m_bIsDpCInfoWndChecked )
	{
		// Creates the Info dialog if not already created.
		if( NULL == m_pInfoDialogDpC->GetSafeHwnd() )
		{
			if( FALSE == m_pInfoDialogDpC->Create() )	// display the dialog window
			{
				return;
			}
		}

		double dDpToStab = 0.0;

		if( true == m_clIndSelDpCParams.m_bIsGroupDpbranchOrKvChecked )
		{
			if( eDpStab::DpStabOnCV == m_clIndSelDpCParams.m_eDpStab )
			{
				CWaterChar *pWC = &m_clIndSelDpCParams.m_WC;
				
				// Dp stab on control valve.
				if( m_clIndSelDpCParams.m_dFlow > 0.0 )
				{
					m_clIndSelDpCParams.m_dDpBranch = CalcDp( m_clIndSelDpCParams.m_dFlow, m_clIndSelDpCParams.m_dKv, pWC->GetDens() );
				}
			}
			else
			{
				// Dp stab on branch.
				// The value is already in the 'm_clIndSelDpCParams.m_dDpBranch' variable.
			}
		}

		m_pInfoDialogDpC->SetSchemePict(	
				m_clIndSelDpCParams.m_eDpStab,							// eDpStab::DpStabOnBranch or eDpStab::DpStabOnCV
				m_clIndSelDpCParams.m_eMvLoc,							// eMvLoc::MvLocPrimary or eMvLoc::MvLocSecondary
				m_clIndSelDpCParams.m_eDpCLoc,
				m_clIndSelDpCParams.m_bIsGroupDpbranchOrKvChecked, 
				m_clIndSelDpCParams.m_dDpBranch,					// DP stabilized.
				m_clIndSelDpCParams.m_dKv,
				m_clIndSelDpCParams.m_bOnlyForSet );

		if( NULL != pRViewSSelDpC )
		{
			pRViewSSelDpC->UpdateBvInfos();
		}
	}
	else
	{
		if( NULL != m_pInfoDialogDpC->GetSafeHwnd() )
		{
			m_pInfoDialogDpC->DestroyWindow();
		}
	}
}

void CDlgIndSelDpC::OnBnClickedCheckboxSet()
{
	m_clIndSelDpCParams.m_bOnlyForSet = ( BST_CHECKED == m_CheckboxSet.GetCheck() ) ? true : false;
	ClearAll();
}

LRESULT CDlgIndSelDpC::OnModifySelectedDpC( WPARAM wParam, LPARAM lParam )
{
	if( NULL == TASApp.GetpTADS() || NULL == TASApp.GetpTADS()->GetpTechParams() )
	{
		ASSERTA_RETURN( -1 );
	}

	CDS_SSelDpC *pSelDpC = dynamic_cast<CDS_SSelDpC *>( (CData *)lParam );
	ASSERT( NULL != pSelDpC );

	if( NULL == pSelDpC || NULL == pDlgLTtabctrl )
	{
		return 0;
	}
	
	m_clIndSelDpCParams.m_SelIDPtr = pSelDpC->GetIDPtr();
	m_bInitialised = false;

	// Send message to notify the display of the SSel tab.
	pDlgLTtabctrl->SetActiveTab( CMyMFCTabCtrl::TabIndex::etiSSel );

	// If we are not in the good selection mode, we must first disable it.
	if( ProductSelectionMode_Individual != pMainFrame->GetCurrentProductSelectionMode() )
	{
		pMainFrame->ChangeProductSelectionMode( ProductSelectionMode_Individual );
	}

	// HYS-1242: SetRedraw to false to avoid too much refresh in ClickRibbonProductSelGallery. 
	pMainFrame->SetRedraw( FALSE );
	// Select the right item in the gallery box.
	pMainFrame->ClickRibbonProductSelGallery( ProductSubCategory::PSC_BC_DpController );

	// Select 'Heating, 'Cooling' or 'Solar'.
	m_clIndSelDpCParams.m_eApplicationType = pSelDpC->GetpSelectedInfos()->GetApplicationType();
	pMainFrame->ChangeRbnProductSelectionApplicationType( m_clIndSelDpCParams.m_eApplicationType );

	// Transfer water characteristics.
	m_clIndSelDpCParams.m_WC = *( pSelDpC->GetpSelectedInfos()->GetpWCData() );

	// Copy water characteristic and temperature data in the technical parameters.
	OnModifySelectedProductUpdateTechData( pSelDpC->GetpSelectedInfos() );

	// HYS-1242: To refresh the window in InitializeRibbonRichEditBoxesSSel
	pMainFrame->SetRedraw( TRUE );
	pMainFrame->InitializeRibbonRichEditBoxesSSel();

	// Remark: Corresponding combo variables in 'm_clIndSelDpCParams' are updated in each of this following methods.
	_FillComboDpStab( pSelDpC->GetDpStab() );
	_FillComboMvLoc( pSelDpC->GetMvLoc() );
	_FillComboDpCLoc( pSelDpC->GetDpCLoc() );
	
	// To adapt reset of control with user selection in Dp stabilization and Mv location.
	_UpdateDpStabilizedOn();
	_UpdateMvLocation();
	_UpdateDpCLocation();

	// Remark: 'm_clIndSelDpCParams.m_bIsGroupDpbranchOrKvChecked' is updated in the '_UpdateGroupDpBranchOrKvChecked' method.
	m_GroupDpOrKv.SetCheck( ( pSelDpC->GetDpL() > 0.0 || pSelDpC->GetKv() > 0.0 ) ? BST_CHECKED : BST_UNCHECKED );
	_UpdateGroupDpBranchOrKvChecked();
	_UpdateDpBranchFieldState();

	// HYS-1103 : m_eDpStab instead of m_eDpCLoc
	if( eDpStab::DpStabOnBranch == m_clIndSelDpCParams.m_eDpStab )
	{
		m_clIndSelDpCParams.m_dDpBranch = pSelDpC->GetDpL();
		_UpdateDpBranchFieldValue();
	}
	else
	{
		// Remark: 'm_clIndSelDpCParams.m_dKv' is updated in the following method.
		_FillComboKv( pSelDpC->GetKv() );
	}

	// Remark: 'm_clIndSelDpCParams.m_bIsDpMaxChecked' is updated in the '_UpdateDpMaxFieldState' method.
	m_GroupDpMax.SetCheck( ( pSelDpC->GetDpMax() > 0.0 ) ? BST_CHECKED : BST_UNCHECKED );
	_UpdateDpMaxFieldState();

	// Remark: 'm_clIndSelDpCParams.m_dDpMax' is updated in the following method.
	SetDpMax( pSelDpC->GetDpMax() );

	m_iRadioFlowPowerDT = ( CDS_SelProd::efdFlow == pSelDpC->GetFlowDef() ) ? Radio_Flow : Radio_PowerDT;
	
	// 'FALSE' to change the radio state in the dialog with the content of the 'm_iRadioFlowPowerDT' variable.	
	UpdateData( FALSE );

	// Remark: 'm_clIndSelDpCParams.m_eFlowOrPowerDTMode' is updated in the following method in regards to the 'm_iRadioFlowPowerDT' variable.
	CDlgIndSelBase::UpdateFlowOrPowerDTState();

	if( CDS_SelProd::efdFlow == m_clIndSelDpCParams.m_eFlowOrPowerDTMode )
	{
		m_clIndSelDpCParams.m_dFlow = pSelDpC->GetQ();
		UpdateFlowFieldValue();
	}
	else
	{
		m_clIndSelDpCParams.m_dPower = pSelDpC->GetPower();
		UpdatePowerFieldValue();

		m_clIndSelDpCParams.m_dDT = pSelDpC->GetDT();
		UpdateDTFieldValue();
	}

	m_clIndSelDpCParams.m_bOnlyForSet = pSelDpC->IsSelectedAsAPackage();
	m_CheckboxSet.SetCheck( ( true == m_clIndSelDpCParams.m_bOnlyForSet ) ? BST_CHECKED : BST_UNCHECKED );

	// Remark: Corresponding combo variables in 'm_clIndSelCVParams' are updated in each of this following methods.
	_FillComboType( pSelDpC->GetTypeID() );
	_FillComboFamily( pSelDpC->GetFamilyID() );
	_FillComboBodyMat( pSelDpC->GetMaterialID() );
	_FillComboConnect( pSelDpC->GetConnectID() );
	_FillComboVersion( pSelDpC->GetVersionID() );
	_FillComboPN( pSelDpC->GetPNID() );
	
	// Remark: The 'm_clIndSelDpCParams.m_strPipeSeriesID' and 'm_clIndSelDpCParams.m_strPipeID' are both updated
	//         by calling the 'SelectCurrentComboPipes' method.
	SelectCurrentComboPipes( pSelDpC->GetPipeSeriesID(), pSelDpC->GetPipeID() );

	m_bInitialised = true;

	OnBnClickedCheckShowInfoWnd();

	// By default now we automatically enable apply button.
	EnableSelectButton( true );

	// Remark: the 'm_pclIndSelParams->m_bEditModeRunning' variable is updated by calling the 'SetModificationMode' method.
	SetModificationMode( true );

	ClearAll();

	PostWMCommandToControl( GetpBtnSuggest() );

	return 0;
}

LRESULT CDlgIndSelDpC::OnDestroyInfoDialogDpC( WPARAM wParam, LPARAM lParam )
{
	if( NULL != m_pInfoDialogDpC && NULL != m_pInfoDialogDpC->GetSafeHwnd() )
	{
		m_pInfoDialogDpC->DestroyWindow();
	}

	m_clIndSelDpCParams.m_bIsDpCInfoWndChecked = false;
	m_CheckInfoWnd.SetCheck( BST_UNCHECKED );

	return 0L;
}

void CDlgIndSelDpC::OnBnClickedSuggest() 
{
	if( NULL == pRViewSSelDpC )
	{
		return;
	}

	ClearAll();

	// Build up suggestion (pre-selection) list
	if( true == _UpdateMembers() )
	{
		if( NULL != m_clIndSelDpCParams.m_pclSelectDpCList )
		{
			delete m_clIndSelDpCParams.m_pclSelectDpCList;
		}

		m_clIndSelDpCParams.m_pclSelectDpCList = new CSelectDpCList();

		if( NULL == m_clIndSelDpCParams.m_pclSelectDpCList || NULL == m_clIndSelDpCParams.m_pclSelectDpCList->GetSelectPipeList() )
		{
			ASSERT_RETURN;
		}

		m_clIndSelDpCParams.m_pclSelectDpCList->GetSelectPipeList()->SelectPipes( &m_clIndSelDpCParams, m_clIndSelDpCParams.m_dFlow );

		m_clIndSelDpCParams.m_DpCList.PurgeAll();

		int iValveCount = m_clIndSelDpCParams.m_pTADB->GetDpCList( 
				&m_clIndSelDpCParams.m_DpCList,
				m_clIndSelDpCParams.m_eDpCLoc,
				(LPCTSTR)m_clIndSelDpCParams.m_strComboTypeID,
				(LPCTSTR)m_clIndSelDpCParams.m_strComboFamilyID, 
				(LPCTSTR)m_clIndSelDpCParams.m_strComboMaterialID,
				(LPCTSTR)m_clIndSelDpCParams.m_strComboConnectID,
				(LPCTSTR)m_clIndSelDpCParams.m_strComboVersionID,
				(LPCTSTR)m_clIndSelDpCParams.m_strComboPNID,
				m_clIndSelDpCParams.m_eFilterSelection );

		// Preselect DpC.
		bool bValidFound = false;
		bool bTryAlternative = false;
		int iDevFound = 0;

		if( 0 == iValveCount )
		{
			bTryAlternative = true;
		}
		else
		{
			iDevFound = m_clIndSelDpCParams.m_pclSelectDpCList->SelectDpC( &m_clIndSelDpCParams, bValidFound );
		
			if( 0 != iDevFound && true == bValidFound )
			{
				VerifyModificationMode();
				pRViewSSelDpC->Suggest( &m_clIndSelDpCParams, (LPARAM)m_pInfoDialogDpC );
			}
			else
			{
				bTryAlternative = true;
			}
		}

		if( true == bTryAlternative )
		{
			_TryAlternativeSuggestion( iDevFound, bValidFound );
		}
	}
	else if( m_clIndSelDpCParams.m_dFlow <= 0.0 )
	{
		TASApp.AfxLocalizeMessageBox( AFXMSG_NO_SUGGESTION_AT_ZERO_FLOW );
		GetpEditFlow()->SetFocus();
	}
}

void CDlgIndSelDpC::OnBnClickedSelect() 
{
	try
	{
		if( NULL == m_clIndSelDpCParams.m_pTADS )
		{
			HYSELECT_THROW( _T("Internal error: 'm_clIndSelDpCParams.m_pTADS' argument can't be NULL.") );
		}
		else if( NULL == m_clIndSelDpCParams.m_pTADS->GetpTechParams() )
		{
			HYSELECT_THROW( _T("Internal error: 'm_clIndSelDpCParams.m_pTADS->GetpTechParams()' argument can't be NULL.") );
		}

		// Verify the dialog is active.
		if( NULL == pRViewSSelDpC )
		{
			HYSELECT_THROW( _T("Internal error: The Dp controller valve right view is not created.") );
		}
		else if( FALSE == this->IsWindowVisible() )
		{
			HYSELECT_THROW( _T("Internal error: The Dp controller valve right view is not visible.") );
		}

		CDB_DpController *pclDpController = pRViewSSelDpC->GetCurrentDpControllerSelected();
	
		// This case should never occurs.
		if( NULL == pclDpController )
		{
			HYSELECT_THROW( _T("Internal error: Can't retrieve the Dp controller selected.") );
		}

		if( m_clIndSelDpCParams.m_dFlow <= 0.0 ) 
		{
			HYSELECT_THROW( _T("Internal error: Flow '%f' can't be lower or equal to 0."), m_clIndSelDpCParams.m_dFlow );
		}

		// Retrieve the respective 'CSelectedTAP' object with 'pclDpController'.
		CSelectedBase *pclSelectedProduct = NULL;

		for( CSelectedBase *pclLoopSelectedProduct = m_clIndSelDpCParams.m_pclSelectDpCList->GetFirst<CSelectedBase>(); NULL != pclLoopSelectedProduct; 
				pclLoopSelectedProduct = m_clIndSelDpCParams.m_pclSelectDpCList->GetNext<CSelectedBase>() )
		{
			CDB_TAProduct *pTAP = dynamic_cast<CDB_TAProduct *>( pclLoopSelectedProduct->GetpData() );
		
			if( pTAP == pclDpController )
			{
				pclSelectedProduct = pclLoopSelectedProduct;
				break;
			}
		}

		if( NULL == pclSelectedProduct )
		{
			return;
		}

		// When MV is in secondary, a measuring valve must be selected.  
		if( NULL == pRViewSSelDpC->GetCurrentBalancingValveSelected() && eMvLoc::MvLocSecondary == m_clIndSelDpCParams.m_eMvLoc )
		{
			TASApp.AfxLocalizeMessageBox( IDS_SELB_MVEXIST );
			return;
		}

		if( true == pclSelectedProduct->IsFlagSet( CSelectedBase::Flags::eValveDpToSmall ) ) 
		{
			TASApp.AfxLocalizeMessageBox( IDS_SELB_DPLTOOSMALL );
			return;
		}

		if( true == pclSelectedProduct->IsFlagSet( CSelectedBase::Flags::eValveDpToLarge ) ) 
		{
			TASApp.AfxLocalizeMessageBox( IDS_SELB_DPLTOOLARGE );
			return;
		}

		// Verify pointer validity; user can delete object after entering in edition.
		if( _T('\0') != *m_clIndSelDpCParams.m_SelIDPtr.ID )
		{
			m_clIndSelDpCParams.m_SelIDPtr = m_clIndSelDpCParams.m_pTADS->Get( m_clIndSelDpCParams.m_SelIDPtr.ID );
		}
	
		// Create selected object and initialize it.
		CDS_SSelDpC *pSelDpC = dynamic_cast<CDS_SSelDpC *>( m_clIndSelDpCParams.m_SelIDPtr.MP );
	
		if( NULL == pSelDpC )
		{
			IDPTR IDPtr;
			m_clIndSelDpCParams.m_pTADS->CreateObject( IDPtr, CLASS( CDS_SSelDpC ) );
			pSelDpC = (CDS_SSelDpC *)IDPtr.MP;
		}

		// Set DpStab (stabilization on branch or CV).
		pSelDpC->SetDpStab( m_clIndSelDpCParams.m_eDpStab );

		pSelDpC->SetDpCLoc( m_clIndSelDpCParams.m_eDpCLoc );

		// Set balancing valve location (primary or secondary).
		pSelDpC->SetMvLoc( m_clIndSelDpCParams.m_eMvLoc );

		// Set DpL or Kv (depending on DpStab).
		if( eDpStab::DpStabOnBranch == m_clIndSelDpCParams.m_eDpStab )
		{
			pSelDpC->SetDpL( m_clIndSelDpCParams.m_dDpBranch );
		}
		else
		{
			pSelDpC->SetKv( m_clIndSelDpCParams.m_dKv );
		}

		pSelDpC->SetDpMax( m_clIndSelDpCParams.m_dDpMax );
		pSelDpC->SetQ( m_clIndSelDpCParams.m_dFlow );

		if( CDS_SelProd::efdFlow == m_clIndSelDpCParams.m_eFlowOrPowerDTMode )
		{
			pSelDpC->SetFlowDef( CDS_SelProd::efdFlow );
			pSelDpC->SetPower( 0.0 );
			pSelDpC->SetDT( 0.0 );
		}
		else
		{
			pSelDpC->SetFlowDef( CDS_SelProd::efdPower );
			pSelDpC->SetPower( m_clIndSelDpCParams.m_dPower );
			pSelDpC->SetDT( m_clIndSelDpCParams.m_dDT );
		}

		pSelDpC->SetSelectedAsAPackage(	m_clIndSelDpCParams.m_bOnlyForSet );
		pSelDpC->ResetDpCMvPackageIDPtr();

		pSelDpC->SetTypeID( m_clIndSelDpCParams.m_strComboTypeID );
		pSelDpC->SetFamilyID( m_clIndSelDpCParams.m_strComboFamilyID );
		pSelDpC->SetMaterialID( m_clIndSelDpCParams.m_strComboMaterialID );
		pSelDpC->SetConnectID( m_clIndSelDpCParams.m_strComboConnectID );
		pSelDpC->SetVersionID( m_clIndSelDpCParams.m_strComboVersionID );
		pSelDpC->SetPNID( m_clIndSelDpCParams.m_strComboPNID );

		pSelDpC->SetPipeSeriesID( m_clIndSelDpCParams.m_strPipeSeriesID );
		pSelDpC->SetPipeID( m_clIndSelDpCParams.m_strPipeID );

		*pSelDpC->GetpSelectedInfos()->GetpWCData() = m_clIndSelDpCParams.m_WC;
	
		// We save here the real DT in the ribbon to be able to restore it when we edit a selection.
		// Remark: the DT in the ribbon is never used in the computing. It is the DT in the left panel that is used when
		//         user chooses the Power/DT mode input.
		pSelDpC->GetpSelectedInfos()->SetDT( m_clIndSelDpCParams.m_pTADS->GetpTechParams()->GetDefaultISDT() );

		// For the same reason, we also save the application type.
		pSelDpC->GetpSelectedInfos()->SetApplicationType( m_clIndSelDpCParams.m_eApplicationType );

		pRViewSSelDpC->FillInSelected( pSelDpC );

		CDlgConfSel dlg( &m_clIndSelDpCParams );
		IDPTR IDPtr = pSelDpC->GetIDPtr();
	
		dlg.Display( pSelDpC );

		if( IDOK == dlg.DoModal() )
		{
			CTable *pTab = dynamic_cast<CTable *>( m_clIndSelDpCParams.m_pTADS->Get( _T("DPCONTR_TAB") ).MP );
			
			if( NULL == pTab )
			{
				HYSELECT_THROW( _T("Internal error: Can't retrieve the 'DPCONTR_TAB' table from the datastruct.") );
			}
		
			if( _T('\0') == *m_clIndSelDpCParams.m_SelIDPtr.ID )
			{
				pTab->Insert( IDPtr );
				m_clIndSelDpCParams.m_pTADS->SetNewIndex( CLASS( CDS_SSelDpC ) );
			}

			// It's only in case user has clicked 'OK' button that we can set modification mode to 'false'. Otherwise if user
			// has clicked 'Cancel' button, we must stay in edition mode and not activate 'Select' button.
			if( true == m_clIndSelDpCParams.m_bEditModeRunning )
			{
				SetModificationMode( false );
				PostWMCommandToControl( GetpBtnSuggest() );
			}

			// Enable Select Button
			EnableSelectButton( true );
		}
		else
		{
			if( _T('\0') == *m_clIndSelDpCParams.m_SelIDPtr.ID )
			{
				m_clIndSelDpCParams.m_pTADS->DeleteObject( IDPtr );
			}
		}
	}
	catch( CHySelectException &clHySelectException )
	{
		clHySelectException.AddMessage( _T("Error in 'CDlgIndSelDpC::OnBnClickedSelect'."), __LINE__, __FILE__ );
		throw;
	}
}

void CDlgIndSelDpC::OnEnChangeFlow() 
{
	CDlgIndSelBase::OnEnChangeFlow(); 
	ReadCUDouble( _U_FLOW, *( ( CEdit * )GetpEditFlow() ), &m_clIndSelDpCParams.m_dFlow );
	_UpdateStaticKvRange();
}

void CDlgIndSelDpC::OnEnKillFocusFlow() 
{
	CDlgIndSelBase::OnEnKillFocusFlow();

	if( NULL != pRViewSSelDpC )
	{
		pRViewSSelDpC->UpdateBvInfos();
	}
}

void CDlgIndSelDpC::OnEnSetFocusFlow()
{
	GetpEditFlow()->SetSel( 0, -1 );
}

void CDlgIndSelDpC::OnEnSetFocusPower()
{
	GetpEditPower()->SetSel( 0, -1 );
}

void CDlgIndSelDpC::OnEnChangeDpBranch() 
{
	if( GetFocus() == &m_clExtEditDpBranch )
	{
		ClearAll();
	}
}

void CDlgIndSelDpC::OnEnKillFocusDpBranch()
{
	m_clIndSelDpCParams.m_dDpBranch = 0.0;

	if( RD_OK != ReadCUDouble( _U_DIFFPRESS, m_clExtEditDpBranch, &m_clIndSelDpCParams.m_dDpBranch ) 
			|| m_clIndSelDpCParams.m_dDpBranch < 0.0 )
	{
		m_clIndSelDpCParams.m_dDpBranch = 0.0;
	}
}

void CDlgIndSelDpC::OnEnChangeDpMax() 
{
	if( GetFocus() == &m_clExtEditDpMax )
	{
		ClearAll();
	}
}

void CDlgIndSelDpC::OnEnKillFocusDpMax() 
{
	m_clIndSelDpCParams.m_dDpMax = 0.0;

	if( RD_OK != ReadCUDouble( _U_DIFFPRESS, m_clExtEditDpMax, &m_clIndSelDpCParams.m_dDpMax ) 
			|| m_clIndSelDpCParams.m_dDpMax < 0.0 )
	{
		m_clIndSelDpCParams.m_dDpMax = 0.0;
	}
}

void CDlgIndSelDpC::OnDestroy() 
{
	CString str;
	CDlgIndSelBase::OnDestroy();

	if( NULL != m_pInfoDialogDpC )
	{
		m_pInfoDialogDpC->DestroyWindow();
	}
}

void CDlgIndSelDpC::OnEditEnterChar( NMHDR *pNMHDR, LRESULT *pResult )
{
	if( IDC_EDITDPBRANCH == pNMHDR->idFrom || IDC_COMBOKV == pNMHDR->idFrom || IDC_EDITDPMAX == pNMHDR->idFrom )
	{
		GetpBtnSuggest()->SetFocus();	
		
		if( IDC_EDITDPBRANCH == pNMHDR->idFrom )
		{
			m_clExtEditDpBranch.SetFocus();
		}
		else if( IDC_EDITDPMAX == pNMHDR->idFrom )
		{
			m_ComboKv.SetFocus();
		}
		else if( IDC_EDITDPMAX == pNMHDR->idFrom )
		{
			m_clExtEditDpMax.SetFocus();
		}
		
		PostWMCommandToControl( GetpBtnSuggest() );
	}
	else
	{
		CDlgIndSelBase::OnEditEnterChar( pNMHDR, pResult );
	}
}

LRESULT CDlgIndSelDpC::OnNewDocument( WPARAM wParam, LPARAM lParam )
{
	CDlgIndSelBase::OnNewDocument( wParam, lParam );

	// Get last selected parameters.
	CDS_IndSelParameter *pclIndSelParameter = m_clIndSelDpCParams.m_pTADS->GetpIndSelParameter();

	if( NULL == pclIndSelParameter )
	{
		return -1;
	}

	m_iRadioFlowPowerDT = ( CDS_SelProd::efdFlow == (CDS_SelProd::eFlowDef)pclIndSelParameter->GetDpCRadioFlowPowerDT() ) ? Radio_Flow : Radio_PowerDT;

	// 'FALSE' to change the radio state in the dialog with the content of the 'm_iRadioFlowPowerDT' variable.
	UpdateData( FALSE );

	// Remark: 'm_clIndSelBCVParams.m_eFlowOrPowerDTMode' is updated in the following method in regards to the 'm_iRadioFlowPowerDT' variable.
	CDlgIndSelBase::UpdateFlowOrPowerDTState();

	m_clIndSelDpCParams.m_bIsDpCInfoWndChecked = ( ( 0 == pclIndSelParameter->GetDpCInfoWnd() ) ? false : true );
	m_CheckInfoWnd.SetCheck( ( true == m_clIndSelDpCParams.m_bIsDpCInfoWndChecked ) ? BST_CHECKED : BST_UNCHECKED );
	
	// Remark: Corresponding combo variables in 'm_clIndSelDpCParams' are updated in each of this following methods.
	_FillComboDpStab( (eDpStab)pclIndSelParameter->GetDpCDpStab() );
	_FillComboDpCLoc( (eDpCLoc)pclIndSelParameter->GetDpCLoc() );
	_FillComboMvLoc( (eMvLoc)pclIndSelParameter->GetDpCMvLoc() );

	// To adapt reset of control with user selection in Dp stabilization and Mv location.
	_UpdateDpStabilizedOn();
	_UpdateDpCLocation();
	_UpdateMvLocation();

	// Remark: 'm_clIndSelDpCParams.m_bIsGroupDpbranchOrKvChecked' is updated in the '_UpdateGroupDpBranchOrKvChecked' method.
	m_GroupDpOrKv.SetCheck( ( 0 == pclIndSelParameter->GetDpCDpCheckBox() ) ? BST_UNCHECKED : BST_CHECKED );
	_UpdateGroupDpBranchOrKvChecked();
	
	_UpdateDpBranchFieldState();
	_UpdateDpBranchFieldValue();

	// Remark: 'm_clIndSelDpCParams.m_bIsDpMaxChecked' is updated in the '_UpdateDpMaxFieldState' method.
	m_GroupDpMax.SetCheck( ( 0 == pclIndSelParameter->GetDpCDpMaxCheckBox() ) ? BST_UNCHECKED : BST_CHECKED );
	_UpdateDpMaxFieldState();

	_FillComboKv();

	// Remark: Corresponding combo variables in 'm_clIndSelDpCParams' are updated in each of this following methods.
	_FillComboType( pclIndSelParameter->GetDpCTypeID() );
	_FillComboFamily( pclIndSelParameter->GetDpCFamilyID() );
	_FillComboBodyMat( pclIndSelParameter->GetDpCMaterialID() );
	_FillComboConnect( pclIndSelParameter->GetDpCConnectID() );
	_FillComboVersion( pclIndSelParameter->GetDpCVersionID() );
	_FillComboPN( pclIndSelParameter->GetDpCPNID() );

	// Fill the flow and Dp unit static controls and update water TCHAR. strings.
	OnUnitChange();

	m_clIndSelDpCParams.m_bOnlyForSet = ( 0 == pclIndSelParameter->GetDpCPackageChecked() ) ? false : true;
	m_CheckboxSet.SetCheck( ( true == m_clIndSelDpCParams.m_bOnlyForSet ) ? BST_CHECKED : BST_UNCHECKED );

	m_bInitialised = true;

	EnableSelectButton( false );
	OnBnClickedCheckShowInfoWnd();

	if( NULL != pRViewSSelDpC )
	{
		pRViewSSelDpC->OnNewDocument( pclIndSelParameter );
	}

	return 0;
}

LRESULT CDlgIndSelDpC::OnPipeChange( WPARAM wParam, LPARAM lParam )
{
	if( WMUserPipeChange::WM_UPC_ForProductSelection != (WMUserPipeChange)wParam )
	{
		return 0;
	}

	CDlgIndSelBase::OnPipeChange( wParam, lParam );
	
	if( NULL != pRViewSSelDpC )
	{
		pRViewSSelDpC->Reset();
	}

	// Redraw info window.
	OnBnClickedCheckShowInfoWnd();
	
	return 0;
}

LRESULT CDlgIndSelDpC::OnTechParChange( WPARAM wParam, LPARAM lParam )
{
	CDlgIndSelBase::OnTechParChange( wParam, lParam );

	if( NULL != pRViewSSelDpC )
	{
		pRViewSSelDpC->Reset();
	}

	// Redraw info window.
	OnBnClickedCheckShowInfoWnd();

	return 0;
}

LRESULT CDlgIndSelDpC::OnUnitChange( WPARAM wParam, LPARAM lParam )
{
	CDlgIndSelBase::OnUnitChange( wParam, lParam );

	TCHAR name[_MAXCHARS];
	CUnitDatabase *pUnitDB = CDimValue::AccessUDB();

	GetNameOf( pUnitDB->GetDefaultUnit( _U_DIFFPRESS ), name );
	SetDlgItemText( IDC_STATICDPBRANCHUNIT, name );
	SetDlgItemText( IDC_STATICDPMAXUNIT, name );

	if( m_clIndSelDpCParams.m_dDpMax > 0.0 )
	{
		m_clExtEditDpMax.SetWindowText( WriteCUDouble( _U_DIFFPRESS, m_clIndSelDpCParams.m_dDpMax ) );
	}

	// Redraw info window.
	OnBnClickedCheckShowInfoWnd();

	if( NULL != pRViewSSelDpC )
	{
		pRViewSSelDpC->Reset();
	}

	return 0;
}

LRESULT CDlgIndSelDpC::OnWaterChange( WPARAM wParam, LPARAM lParam )
{
	// HYS-1693: Not for changeOver application type
	if( WMUserWaterCharWParam::WM_UWC_WP_ForProductSel != ( (WMUserWaterCharWParam)wParam & WMUserWaterCharWParam::WM_UWC_WP_ForProductSel )
		|| WMUserWaterCharLParam::WM_UWC_LWP_ChangeForChangeOver == (WMUserWaterCharLParam)lParam
		|| WMUserWaterCharLParam::WM_UWC_LWP_NoChangeForChangeOver == (WMUserWaterCharLParam)lParam )
	{
		return 0;
	}

	CDlgIndSelBase::OnWaterChange( wParam, lParam );
	_UpdateStaticKvRange();

	// Redraw info window.
	OnBnClickedCheckShowInfoWnd();

	if( NULL != pRViewSSelDpC )
	{
		pRViewSSelDpC->Reset();
	}

	return 0;
}

CRViewSSelSS* CDlgIndSelDpC::GetLinkedRightViewSSel( void )
{ 
	return pRViewSSelDpC;
}

void CDlgIndSelDpC::ClearAll( void )
{
	CDlgIndSelBase::ClearAll();

	// Redraw info window.
	OnBnClickedCheckShowInfoWnd();
}

//////////////////////////////////////////////////////////////////////////////////////////////////////////////
// PROTECTED METHODS
//////////////////////////////////////////////////////////////////////////////////////////////////////////////

void CDlgIndSelDpC::_FillComboDpStab( eDpStab eDpStabilizationOn )
{
	CRankEx DpStabList;
	m_clIndSelDpCParams.m_pTADB->GetDpCDpStabList( &DpStabList );

	// Intentionally do not use 'CExtNumEditComboBox::FillInCombo' method!!!
	m_ComboDpStab.ResetContent();
	DpStabList.Transfer( &m_ComboDpStab );

	m_ComboDpStab.SetCurSel( (int)eDpStabilizationOn );
	m_clIndSelDpCParams.m_eDpStab = (eDpStab)m_ComboDpStab.GetCurSel();

	if( NULL != m_ToolTip.GetSafeHwnd() )
	{
		CString str = ( eDpStab::DpStabOnBranch == eDpStabilizationOn ) ? TASApp.LoadLocalizedString( IDS_BRANCH ): TASApp.LoadLocalizedString( IDS_CONTROLVALVE );	
		m_ToolTip.UpdateTipText( (LPCTSTR) str, &m_ComboDpStab );
	}
}

void CDlgIndSelDpC::_FillComboDpCLoc( eDpCLoc eDpCLocation )
{
	CRankEx DpCLocList;
	m_clIndSelDpCParams.m_pTADB->GetDpCLocList( &DpCLocList, m_clIndSelDpCParams.m_eFilterSelection );

	// Intentionally do not use 'CExtNumEditComboBox::FillInCombo' method!!!
	m_ComboDpCLoc.ResetContent();
	DpCLocList.Transfer( &m_ComboDpCLoc );

	m_ComboDpCLoc.SetCurSel( (int)eDpCLocation );
	m_clIndSelDpCParams.m_eDpCLoc = (eDpCLoc)m_ComboDpCLoc.GetCurSel();
	
	if( NULL != m_ToolTip.GetSafeHwnd() )
	{
		CString str = ( eDpCLoc::DpCLocDownStream == eDpCLocation ) ? TASApp.LoadLocalizedString( IDS_DPCLOCDOWNSTREAMTOOLTIP ): TASApp.LoadLocalizedString( IDS_DPCLOCUPSTREAMTOOLTIP );	
		m_ToolTip.UpdateTipText( (LPCTSTR) str, &m_ComboDpCLoc );
	}
}

void CDlgIndSelDpC::_FillComboMvLoc( eMvLoc eMvLocation )
{
	CRankEx MvLocList;
	m_clIndSelDpCParams.m_pTADB->GetDpCMvLocList( &MvLocList );

	// Intentionally do not use 'CExtNumEditComboBox::FillInCombo' method!!!
	m_ComboMvLoc.ResetContent();
	MvLocList.Transfer( &m_ComboMvLoc );

	m_ComboMvLoc.SetCurSel( (int)eMvLocation );
	m_clIndSelDpCParams.m_eMvLoc = (eMvLoc)m_ComboMvLoc.GetCurSel();

	if( NULL != m_ToolTip.GetSafeHwnd() )
	{
		CString str = ( eMvLoc::MvLocPrimary == eMvLocation ) ? TASApp.LoadLocalizedString( IDS_MVEXCL ): TASApp.LoadLocalizedString( IDS_MVINCL );	
		m_ToolTip.UpdateTipText( (LPCTSTR) str, &m_ComboMvLoc );
	}
}

void CDlgIndSelDpC::_FillComboType( CString strTypeID )
{
	CRankEx TypeList;

	m_clIndSelDpCParams.m_pTADB->GetDpCTypeList( &TypeList, m_clIndSelDpCParams.m_eFilterSelection, m_clIndSelDpCParams.m_eDpCLoc );
	
	m_ComboType.FillInCombo( &TypeList, strTypeID, m_clIndSelDpCParams.GetComboTypeAllID() );
	m_clIndSelDpCParams.m_strComboTypeID = m_ComboType.GetCBCurSelIDPtr().ID;

	CString str( _T("") );

	if( CB_ERR != m_ComboType.GetCurSel() )
	{
		m_ComboType.GetLBText( m_ComboType.GetCurSel(), str );

		if( NULL != m_ToolTip.GetSafeHwnd() )
		{
			m_ToolTip.UpdateTipText( (LPCTSTR)str, &m_ComboType );
		}
	}
}

void CDlgIndSelDpC::_FillComboFamily( CString strFamilyID )
{
	CRankEx FamList;
	
	m_clIndSelDpCParams.m_pTADB->GetDpCFamilyList( &FamList, m_clIndSelDpCParams.m_eDpCLoc, (LPCTSTR)m_clIndSelDpCParams.m_strComboTypeID, 
			m_clIndSelDpCParams.m_eFilterSelection );
	
	m_ComboFamily.FillInCombo( &FamList, strFamilyID, m_clIndSelDpCParams.GetComboFamilyAllID() );
	m_clIndSelDpCParams.m_strComboFamilyID = m_ComboFamily.GetCBCurSelIDPtr().ID;
}

void CDlgIndSelDpC::_FillComboBodyMat( CString strBodyMatID )
{
	CRankEx BdyMatList;

	m_clIndSelDpCParams.m_pTADB->GetDpCBdyMatList( &BdyMatList, m_clIndSelDpCParams.m_eDpCLoc, (LPCTSTR)m_clIndSelDpCParams.m_strComboTypeID,
			(LPCTSTR)m_clIndSelDpCParams.m_strComboFamilyID, m_clIndSelDpCParams.m_eFilterSelection );
	
	m_ComboBdyMat.FillInCombo( &BdyMatList, strBodyMatID, m_clIndSelDpCParams.GetComboMaterialAllID() );
	m_clIndSelDpCParams.m_strComboMaterialID = m_ComboBdyMat.GetCBCurSelIDPtr().ID;
}

void CDlgIndSelDpC::_FillComboConnect( CString strConnectID )
{
	CRankEx ConnList;

	m_clIndSelDpCParams.m_pTADB->GetDpCConnList( &ConnList, m_clIndSelDpCParams.m_eDpCLoc, (LPCTSTR)m_clIndSelDpCParams.m_strComboTypeID, 
			(LPCTSTR)m_clIndSelDpCParams.m_strComboFamilyID, (LPCTSTR)m_clIndSelDpCParams.m_strComboMaterialID, m_clIndSelDpCParams.m_eFilterSelection );
	
	m_ComboConnect.FillInCombo( &ConnList, strConnectID, m_clIndSelDpCParams.GetComboConnectAllID() );
	m_clIndSelDpCParams.m_strComboConnectID = m_ComboConnect.GetCBCurSelIDPtr().ID;
}

void CDlgIndSelDpC::_FillComboVersion( CString strVersionID )
{
	CRankEx VerList;

	m_clIndSelDpCParams.m_pTADB->GetDpCVersList( &VerList, m_clIndSelDpCParams.m_eDpCLoc, (LPCTSTR)m_clIndSelDpCParams.m_strComboTypeID, 
			(LPCTSTR)m_clIndSelDpCParams.m_strComboFamilyID, (LPCTSTR)m_clIndSelDpCParams.m_strComboMaterialID, (LPCTSTR)m_clIndSelDpCParams.m_strComboConnectID, 
			m_clIndSelDpCParams.m_eFilterSelection );
	
	m_ComboVersion.FillInCombo( &VerList, strVersionID, m_clIndSelDpCParams.GetComboVersionAllID() );
	m_clIndSelDpCParams.m_strComboVersionID = m_ComboVersion.GetCBCurSelIDPtr().ID;
}

void CDlgIndSelDpC::_FillComboPN( CString strPNID )
{
	CRankEx PNList;

	m_clIndSelDpCParams.m_pTADB->GetDpCPNList( &PNList, m_clIndSelDpCParams.m_eDpCLoc, (LPCTSTR)m_clIndSelDpCParams.m_strComboTypeID, 
			(LPCTSTR)m_clIndSelDpCParams.m_strComboFamilyID, (LPCTSTR)m_clIndSelDpCParams.m_strComboMaterialID, (LPCTSTR)m_clIndSelDpCParams.m_strComboConnectID,
			(LPCTSTR)m_clIndSelDpCParams.m_strComboVersionID, m_clIndSelDpCParams.m_eFilterSelection );

	m_ComboPN.FillInCombo( &PNList, strPNID, m_clIndSelDpCParams.GetComboPNAllID() );
	m_clIndSelDpCParams.m_strComboPNID = m_ComboPN.GetCBCurSelIDPtr().ID;
}

void CDlgIndSelDpC::_FillComboKv( double dSelectKv )
{
	CUnitDatabase *pUnitDB = CDimValue::AccessUDB();

	CDB_MultiString *pSeries;

	if( 0 != pUnitDB->GetDefaultUnitIndex( _C_KVCVCOEFF ) )
	{
		pSeries = (CDB_MultiString*)( m_clIndSelDpCParams.m_pTADB->Get( _T("CV_SERIES") ).MP );
		ASSERT( NULL != pSeries );
	}
	else
	{	
		pSeries = (CDB_MultiString*)( m_clIndSelDpCParams.m_pTADB->Get( _T("REYNARD_SERIES") ).MP );
		ASSERT( NULL != pSeries );
	}

	m_ComboKv.ResetContent();
	CString str;
	int iSelect = -1;
	CString strText;
	
	for( int i = 0; i < pSeries->GetNumofLines(); i++ )
	{
		CString strKv = pSeries->GetString( i );
		double dKv = _wtof( (LPCTSTR)strKv );
		
		if( dSelectKv > 0.0 && dSelectKv == dKv )
		{
			iSelect = i;
			strText = pSeries->GetString( i );
		}

		m_ComboKv.InsertString( i, pSeries->GetString( i ) );
	}
	
	if( -1 != iSelect )
	{
		m_ComboKv.SetCurSel( iSelect );
		m_clIndSelDpCParams.m_dKv = dSelectKv;
	}
	else if( BST_UNCHECKED == m_GroupDpOrKv.GetCheck() )
	{
		strText = TASApp.LoadLocalizedString( IDS_UNKNOWN );
		m_clIndSelDpCParams.m_dKv = 0.0;
	}
	else
	{
		strText.Format( _T("%.2f"), dSelectKv );
		m_clIndSelDpCParams.m_dKv = dSelectKv;
	}

	m_ComboKv.SetWindowTextW( strText );
}

bool CDlgIndSelDpC::_UpdateMembers()
{
	if( NULL == GetSafeHwnd() )
	{
		return false;
	}
	
	bool bValid = ( m_clIndSelDpCParams.m_dFlow <= 0.0 ) ? false : true;
	bool bIsDpBranchOrKvValid;

	if( eDpStab::DpStabOnBranch == m_clIndSelDpCParams.m_eDpStab )
	{
		bIsDpBranchOrKvValid = ( m_clIndSelDpCParams.m_dDpBranch > 0.0 ) ? true : false;
	}
	else
	{
		bIsDpBranchOrKvValid = ( m_clIndSelDpCParams.m_dKv > 0.0 ) ? true : false;
	}

	if( false == bIsDpBranchOrKvValid && true == m_clIndSelDpCParams.m_bIsGroupDpbranchOrKvChecked && true == bValid )
	{
		// Flow is valid, checkbox of the 'Dp branch' of 'Kv' group is checked but the value is not defined.
		bValid = false;
		CString	str;
		CUnitDatabase *pUnitDB = CDimValue::AccessUDB();
				
		if( eDpStab::DpStabOnBranch == m_clIndSelDpCParams.m_eDpStab )
		{
			str = TASApp.LoadLocalizedString( AFXMSG_NODP_DPCHECKED );
		}
		else 
		{
			CString tmp;

			if( pUnitDB->GetDefaultUnitIndex( _C_KVCVCOEFF ) != 0 )
			{
				tmp = TASApp.LoadLocalizedString( IDS_KVSERIES_CV );
			}
			else
			{
				tmp = TASApp.LoadLocalizedString( IDS_KVS );
			}

			FormatString( str, AFXMSG_NOKV_KVCHECKED, tmp );
		}

		// Ask to the user if he wants to uncheck the group.
		if( IDYES == ::AfxMessageBox( str, MB_YESNO | MB_ICONQUESTION ) )
		{
			// Remark: 'm_clIndSelDpCParams.m_bIsGroupDpbranchOrKvChecked' is updated in the '_UpdateGroupDpBranchOrKvChecked' method.
			m_GroupDpOrKv.SetCheck( BST_UNCHECKED );
			_UpdateGroupDpBranchOrKvChecked();
			
			// HYS-1163: Update DpBranch field.
			_UpdateDpBranchFieldState();

			PostWMCommandToControl( GetpBtnSuggest() );
		}
	}

	if( m_clIndSelDpCParams.m_dDpMax <= 0.0 )
	{
		// Remark: 'm_clIndSelDpCParams.m_bIsDpMaxChecked' is updated in the '_UpdateDpMaxFieldState' method.
		m_GroupDpMax.SetCheck( BST_UNCHECKED );
		_UpdateDpMaxFieldState();
	}

	return bValid;
}

void CDlgIndSelDpC::_LaunchSuggestion( CString strFamilyID, CString strMaterialID, CString strConnectID, CString strVersionID, CString strPNID )
{
	_FillComboFamily( strFamilyID );
	_FillComboBodyMat( strMaterialID );
	_FillComboConnect( strConnectID );
	_FillComboVersion( strVersionID );
	_FillComboPN( strPNID );

	PostWMCommandToControl( GetpBtnSuggest() );
}

void CDlgIndSelDpC::_LaunchSuggestion( int iMvLoc )
{
	ClearAll();

	m_ComboMvLoc.SetCurSel( iMvLoc );
	OnCbnSelChangeMvLocation();

	PostWMCommandToControl( GetpBtnSuggest() );
}

void CDlgIndSelDpC::_TryAlternativeSuggestion( int iDevFound, bool bValidFound )
{
	if( 0 != iDevFound )
	{
		if( false == bValidFound )
		{
			// Verify DpLTooSmall and DplTooHigh.
			bool bDpLTooSmall = true;
			bool bDpLTooLarge = true;
			bool bAlternative = false;
				
			for( CSelectedBase *pclSelectedProduct = m_clIndSelDpCParams.m_pclSelectDpCList->GetFirst<CSelectedBase>(); 
					NULL != pclSelectedProduct; pclSelectedProduct = m_clIndSelDpCParams.m_pclSelectDpCList->GetNext<CSelectedBase>() )
			{
				if( true == pclSelectedProduct->IsFlagSet( CSelectedBase::eValveDpAlternative ) )
				{
					bAlternative = true;
				}

				bDpLTooSmall &= ( pclSelectedProduct->IsFlagSet( CSelectedBase::eValveDpToSmall ) );
				bDpLTooLarge &= ( pclSelectedProduct->IsFlagSet( CSelectedBase::eValveDpToLarge ) );
					
				if( false == bDpLTooSmall && false == bDpLTooLarge )
				{
					break;
				}
			}
				
			if( ( true == bDpLTooSmall || true == bDpLTooLarge ) && true == bAlternative )
			{
				CDlgNoDpCFound Dlg;

				if( IDOK == Dlg.Display( m_clIndSelDpCParams.m_pclSelectDpCList->GetDpToStab(), m_clIndSelDpCParams.m_eDpStab, m_clIndSelDpCParams.m_eMvLoc ) )
				{
					m_clIndSelDpCParams.m_eMvLoc = ( m_clIndSelDpCParams.m_eMvLoc == eMvLoc::MvLocPrimary ) ? eMvLoc::MvLocSecondary : eMvLoc::MvLocPrimary;
					_LaunchSuggestion( m_clIndSelDpCParams.m_eMvLoc );
				}
			}
			else
			{
				TASApp.AfxLocalizeMessageBox( AFXMSG_NO_DEV_FOUND );
			}
		}
	}
	else
	{
		if( false == m_clIndSelDpCParams.m_strComboTypeID.IsEmpty() || false == m_clIndSelDpCParams.m_strComboFamilyID.IsEmpty() ||
			false == m_clIndSelDpCParams.m_strComboMaterialID.IsEmpty() || false == m_clIndSelDpCParams.m_strComboConnectID.IsEmpty() ||
			false == m_clIndSelDpCParams.m_strComboVersionID.IsEmpty() || false == m_clIndSelDpCParams.m_strComboPNID.IsEmpty() )
		{
			m_clIndSelDpCParams.m_DpCList.PurgeAll();

			m_clIndSelDpCParams.m_pTADB->GetDpCList( &m_clIndSelDpCParams.m_DpCList, m_clIndSelDpCParams.m_eDpCLoc, _T(""), _T(""), _T(""), _T(""), 
					_T(""), _T(""), m_clIndSelDpCParams.m_eFilterSelection );
				
			bValidFound = false;

			iDevFound = m_clIndSelDpCParams.m_pclSelectDpCList->SelectDpC( &m_clIndSelDpCParams, bValidFound );
				
			if( 0 != iDevFound )
			{
				EndWaitCursor();
				
				// HYS-697: Temporary solution because it is not practical to pass through 2 lists to go in the 'CDlgNoDevFound' dialog.
				m_clIndSelDpCParams.m_DpCList.PurgeAll();
				double dKey = 0.0;

				for( CSelectedValve *pclSelectedValve = m_clIndSelDpCParams.m_pclSelectDpCList->GetFirst<CSelectedValve>(); NULL != pclSelectedValve;
					pclSelectedValve = m_clIndSelDpCParams.m_pclSelectDpCList->GetNext<CSelectedValve>() )
				{
					CDB_TAProduct *pTAP = dynamic_cast<CDB_TAProduct *>( pclSelectedValve->GetpData() );

					if( NULL == pTAP )
					{
						continue;
					}

					m_clIndSelDpCParams.m_DpCList.Add( pTAP->GetIDPtr().ID, dKey++, (LPARAM)pTAP->GetIDPtr().MP );
				}

				// Launch the no device found dialog box if something is found.
				// Remark: Combos in this dialog will be filled thanks to the new alternative shut-off valves found and saved in the 'm_SvList' variable.
				CDlgNoDevFoundParams clNoDevParams = CDlgNoDevFoundParams( &m_clIndSelDpCParams.m_DpCList, m_clIndSelDpCParams.m_strComboTypeID, 
						m_clIndSelDpCParams.m_strComboFamilyID, m_clIndSelDpCParams.m_strComboMaterialID, m_clIndSelDpCParams.m_strComboConnectID, 
						m_clIndSelDpCParams.m_strComboVersionID, m_clIndSelDpCParams.m_strComboPNID );

				CDlgNoDevFound dlg( &m_clIndSelDpCParams, &clNoDevParams );
					
				if( IDOK == dlg.DoModal() )
				{
					clNoDevParams = *dlg.GetNoDevFoundParams();

					_LaunchSuggestion( clNoDevParams.m_strFamilyID, clNoDevParams.m_strMaterialID, clNoDevParams.m_strConnectID, clNoDevParams.m_strVersionID, 
							clNoDevParams.m_strPNID );
				}
			}
			else
			{
				EndWaitCursor();
				TASApp.AfxLocalizeMessageBox( AFXMSG_NO_DEV_FOUND );
			}
		}
	}
}

void CDlgIndSelDpC::_GetSelSizeShift( int &iAbove, int &iBelow )
{
	iAbove = m_clIndSelDpCParams.m_pTADS->GetpTechParams()->GetSizeShiftAbove();
	iBelow = m_clIndSelDpCParams.m_pTADS->GetpTechParams()->GetSizeShiftBelow();
}

bool CDlgIndSelDpC::_FindKvsBounds( double dFlow, double *pdLowestKvs, double *pdHighestKvs, double *pdLowestDpl, double *pdHighestDpl )
{
	CWaterChar *pWC = &m_clIndSelDpCParams.m_WC;
	CSelectPipeList *pSelectPipeList = new CSelectPipeList();

	if( NULL == pSelectPipeList )
	{
		ASSERTA_RETURN( false );
	}

	CTableDN *pclTableDN = dynamic_cast<CTableDN *>( m_clIndSelDpCParams.m_pTADB->Get( _T("DN_TAB") ).MP );

	if( NULL == pclTableDN )
	{
		ASSERTA_RETURN( false );
	}

	// Get pointers on required database tables.
	CTable *pTab = (CTable*)( m_clIndSelDpCParams.m_pTADB->Get( _T("DPCONTR_TAB") ).MP );
	ASSERT( NULL != pTab );

	if( 0 == pSelectPipeList->SelectPipes( &m_clIndSelDpCParams, dFlow ) )
	{
		return false;
	}

	////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
	// Sweep over the STAP's according to SizeShift, check the flow wrt to qmax and take the min of lower Dpl bounds and max of higher Dpl bounds.
	////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

	// Highest pipe size to be considered is the size of first pipe plus the SizeShift, if a positive size shift has been asked.
	IDPTR SmallestPipeIDPtr = _NULL_IDPTR;
	IDPTR BiggestPipeIDPtr = _NULL_IDPTR;

	if( false == pSelectPipeList->GetPipeSizeBound( &SmallestPipeIDPtr, &BiggestPipeIDPtr ) )
	{
		delete pSelectPipeList;
		return false;
	}

	CDB_Pipe *pclSmallestPipe = dynamic_cast<CDB_Pipe *>( SmallestPipeIDPtr.MP );
	CDB_Pipe *pclBiggestPipe = dynamic_cast<CDB_Pipe *>( BiggestPipeIDPtr.MP );

	int iDN = pclTableDN->GetBiggestDN();
	
	int iSelSizeShiftAbove;
	int iSelSizeShiftBelow;
	_GetSelSizeShift( iSelSizeShiftAbove, iSelSizeShiftBelow );

	int iHighestSize = min( iDN, pclBiggestPipe->GetSizeKey( m_clIndSelDpCParams.m_pTADB ) + iSelSizeShiftAbove );
	int iLowestSize = max( 1, pclSmallestPipe->GetSizeKey( m_clIndSelDpCParams.m_pTADB ) + iSelSizeShiftBelow );

	// Do a loop on devices finding these with correct size and Qmin, Qmax taking the less restrictive Dpl bounds.
	CString str = CLASS(CDB_DpController);
	int iSizeKey;
	bool bOneValidFound = false;
	*pdLowestDpl = DBL_MAX;
	*pdHighestDpl = 0.0;
	double dBestBP = DBL_MAX;
	double dRho = pWC->GetDens();

	// Do a loop on all Dp controller.
	for( IDPTR IDPtr = pTab->GetFirst( str ); _T('\0') != *IDPtr.ID; IDPtr = pTab->GetNext() )
	{
		// Cast the pointer on device.
		CDB_DpController *pclDpController = (CDB_DpController *)( IDPtr.MP );

		if( NULL == pclDpController )
		{
			continue;
		}

		// If not selectable...
		if( false == pclDpController->IsSelectable( true ) )
		{
				continue;
		}

		// If user has defined a type and current Dp controller type in loop is not the same...
		if( ( false == m_clIndSelDpCParams.m_strComboTypeID.IsEmpty() && m_clIndSelDpCParams.m_strComboTypeID != pclDpController->GetTypeIDPtr().ID ) )
		{
			continue;
		}

		// If user has defined a family and current Dp controller family in loop is not the same...
		if( ( false == m_clIndSelDpCParams.m_strComboFamilyID.IsEmpty() && m_clIndSelDpCParams.m_strComboFamilyID != pclDpController->GetFamilyIDPtr().ID ) )
		{
			continue;
		}

		// If user has defined a body material and current Dp controller body material in loop is not the same...
		if( ( false == m_clIndSelDpCParams.m_strComboMaterialID.IsEmpty() && m_clIndSelDpCParams.m_strComboMaterialID != pclDpController->GetBodyMaterialIDPtr().ID ) )
		{
			continue;
		}
		
		// If user has defined a connection and current Dp controller connection in loop is not the same...
		if( ( false == m_clIndSelDpCParams.m_strComboConnectID.IsEmpty() && m_clIndSelDpCParams.m_strComboConnectID != pclDpController->GetConnectIDPtr().ID ) )
		{
			continue;
		}

		// If user has defined a version and current Dp controller version in loop is not the same...
		if( ( false == m_clIndSelDpCParams.m_strComboVersionID.IsEmpty() && m_clIndSelDpCParams.m_strComboVersionID != pclDpController->GetVersionIDPtr().ID ) )
		{
			continue;
		}
		
		// If user has defined a PN and current Dp controller PN in loop is not the same...
		if( ( false == m_clIndSelDpCParams.m_strComboPNID.IsEmpty() && m_clIndSelDpCParams.m_strComboPNID != pclDpController->GetPNIDPtr().ID ) )
		{
			continue;
		}

		// If current Dp controller is selectable and size is correct...
		iSizeKey = pclDpController->GetSizeKey();
		
		if( pclDpController->GetSizeKey() < iLowestSize || pclDpController->GetSizeKey() > iHighestSize )
		{
			continue;
		}

		// Get Kv characteristic of the valve.
		CDB_DpCCharacteristic *pDevChar = pclDpController->GetDpCCharacteristic();

		if( NULL == pDevChar )
		{
			continue;
		}

		// Retrieve Dpl min and max.
		double dDplmin = pclDpController->GetDplmin();
		double dDplmax = pclDpController->GetDplmax();
			
		if( -1.0 == dDplmin || -1.0 == dDplmax )
		{
			continue;
		}

		// Compute q min and max q.
		// Remark: does not take any Dp in consideration for qmin and qmax calculation here because the only given data is the flow.
		double dQmin = pDevChar->GetQmin( dRho );
		double dQmax = pDevChar->GetQmax( dRho );

		// Check the flow with respect to qmin and qmax.
		// Take the min of Dplmin and max of Dplmax (i.e. less limitative).
		if( dFlow >= dQmin && dFlow <= dQmax )
		{
			bOneValidFound = true;
			*pdHighestDpl = max( *pdHighestDpl, dDplmax );
				
			if( dDplmin < *pdLowestDpl )		// Reset Best BP
			{
				*pdLowestDpl = dDplmin;

				if( true == pDevChar->IsProportionalBandAbsolute() )
				{
					dBestBP = pDevChar->GetProportionalBand(dDplmin) / dDplmin;
				}
				else
				{
					dBestBP = pDevChar->GetProportionalBand(dDplmin);
				}
			}
			else if( dDplmin == *pdLowestDpl)	// Update Best BP
			{
				if( true == pDevChar->IsProportionalBandAbsolute() )
				{
					dBestBP = min( dBestBP, pDevChar->GetProportionalBand(dDplmin) / dDplmin );
				}
				else
				{
					dBestBP = min( dBestBP, pDevChar->GetProportionalBand(dDplmin) );
				}
			}
		}
	}

	if( false == bOneValidFound )
	{	
		delete pSelectPipeList;
		return false;
	}

	// Correct the Dp bounds when the Mv is in the secondary.
	if( eMvLoc::MvLocSecondary == m_clIndSelDpCParams.m_eMvLoc )
	{
		CDS_TechnicalParameter *pTech = m_clIndSelDpCParams.m_pTADS->GetpTechParams();
		*pdLowestDpl *= ( pTech->GetCVMinAuthor() * ( 1.0 + dBestBP ) );
		*pdHighestDpl -= pTech->GetValvMinDp();
	}

	// Calculate the Kvs bounds from the Dp bounds.
	*pdHighestKvs = CalcKv( dFlow, *pdLowestDpl, dRho );
	*pdLowestKvs = CalcKv( dFlow, *pdHighestDpl, dRho );

	// Free allocated memory and return.
	delete pSelectPipeList;
	return true;
}

CString CDlgIndSelDpC::_GetKvsBoundsString()
{
	CString str( _T("") );
	CString tmpstr;
	CUnitDatabase *pUnitDB = CDimValue::AccessUDB();

	// If Dp controller stabilized on control valve...
	if( eDpStab::DpStabOnCV == m_ComboDpStab.GetCurSel() )
	{
		// If flow is given and valid...
		if( m_clIndSelDpCParams.m_dFlow > 0.0 )
		{
			double dLowestDpl;
			double dHighestDpl;
			double dLowestKvs;
			double dHighestKvs;

			if( false == _FindKvsBounds( m_clIndSelDpCParams.m_dFlow, &dLowestKvs, &dHighestKvs, &dLowestDpl, &dHighestDpl ) )
			{
				return str;
			}

			double dKvLow = CDimValue::SItoCU( _C_KVCVCOEFF, dLowestKvs );
			double dKvHigh = CDimValue::SItoCU( _C_KVCVCOEFF, dHighestKvs );

			str = _T("[") + (CString)WriteDouble( dKvLow, ( dKvLow > 1.0 ) ? 3 : 2, 0 );
			str += _T(";") + (CString)WriteDouble( dKvHigh, ( dKvHigh > 1.0 ) ? 3 : 2, 0 ) + _T("]");
		}
	}

	return str;
}

void CDlgIndSelDpC::_UpdateDpStabilizedOn()
{
	m_clIndSelDpCParams.m_eDpStab = (eDpStab)m_ComboDpStab.GetCurSel();
	m_clIndSelDpCParams.m_dDpBranch = 0.0;
	m_clIndSelDpCParams.m_dKv = 0.0;

	if( eDpStab::DpStabOnBranch == m_clIndSelDpCParams.m_eDpStab )
	{
		// No more need of the 'Kv' combo.
		// 'Kv range' is done when calling the '_UpdateGroupDpBranchOrKvChecked' method.
		m_ComboKv.ShowWindow( SW_HIDE );
		m_ComboKv.EnableWindow( FALSE );

		// Update group text ("Dp branch").
 		CString str = TASApp.LoadLocalizedString( IDS_LVSSELB_STATKVDP2 );
 		m_GroupDpOrKv.SetWindowText( str );

		// 'Edit Dp' field and 'Unit Dp' static text become visible.
		m_clExtEditDpBranch.ShowWindow( SW_SHOW );
		m_clExtEditDpBranch.EnableWindow( TRUE );
		m_StaticDpBranchUnit.ShowWindow( SW_SHOW );

		// When changing this combo, it's better to not check Dp checkbox.
		m_GroupDpOrKv.SetCheck( BST_UNCHECKED );
		_UpdateGroupDpBranchOrKvChecked();
		_UpdateDpBranchFieldState();

		// Update the tooltip text for DpStab combo box
		if( NULL != m_ToolTip.GetSafeHwnd() )
		{
			str = TASApp.LoadLocalizedString( IDS_BRANCH );
			m_ToolTip.UpdateTipText( (LPCTSTR) str, &m_ComboDpStab );
		}
	}
	else // Dp Stab on control valve
	{
		// Update group text ("Kvs control valve" or "Cv control valve" depending of current unit).

		// No more need of 'Edit Dp' field and 'Unit Dp' static text.
		m_clExtEditDpBranch.ShowWindow( SW_HIDE );
		m_clExtEditDpBranch.EnableWindow( FALSE );
		m_StaticDpBranchUnit.ShowWindow( SW_HIDE );

		CString str;
		CUnitDatabase *pUnitDB = CDimValue::AccessUDB();
		
		if( 0 != pUnitDB->GetDefaultUnitIndex( _C_KVCVCOEFF ) )
		{
			str = TASApp.LoadLocalizedString( IDS_LVSSELB_STATKVDP3 );
		}
		else
		{
			str = TASApp.LoadLocalizedString( IDS_LVSSELB_STATKVDP1 );
		}
		
		m_GroupDpOrKv.SetWindowText( str );

		// The 'Kv' combo become visible.
		// 'Kv range' is done when calling the '_UpdateGroupDpBranchOrKvChecked' method.
		m_ComboKv.ShowWindow( SW_SHOW );
		m_ComboKv.EnableWindow( TRUE );
		
		// When changing this combo, it's better to not check Kv checkbox.
		m_GroupDpOrKv.SetCheck( BST_UNCHECKED );
		_UpdateGroupDpBranchOrKvChecked();
		_UpdateDpBranchFieldState();

		// Update the tooltip text for DpStab combo box.
		if( NULL != m_ToolTip.GetSafeHwnd() )
		{
			str = TASApp.LoadLocalizedString( IDS_CONTROLVALVE );	
			m_ToolTip.UpdateTipText( (LPCTSTR)str, &m_ComboDpStab );
		}
	}

	GetpEditFlow()->ShowWindow( SW_SHOW );
}

void CDlgIndSelDpC::_UpdateGroupDpBranchOrKvChecked()
{
	m_clIndSelDpCParams.m_bIsGroupDpbranchOrKvChecked = ( BST_CHECKED == m_GroupDpOrKv.GetCheck() ) ? true : false;

	if( eDpStab::DpStabOnCV == m_clIndSelDpCParams.m_eDpStab )
	{
		if( true == m_clIndSelDpCParams.m_bIsGroupDpbranchOrKvChecked )
		{
			m_ComboKv.SetWindowPos( &wndBottom, 0, 0, m_ComboKvRect.Width(), m_ComboKvRect.Height(), SWP_NOZORDER | SWP_NOMOVE );
			m_ComboKv.EnableWindow( TRUE );
			m_ComboKv.SetWindowText( _T("") );
			m_StaticKvRange.ShowWindow( SW_SHOW );
			m_GroupDpOrKv.SetForceShowDisable( false );
		}
		else
		{
			m_ComboKv.SetWindowPos( &wndBottom, 0, 0, m_ComboKvRect.Width() + 20, m_ComboKvRect.Height(), SWP_NOZORDER | SWP_NOMOVE );
			m_ComboKv.EnableWindow( FALSE );
			m_ComboKv.SetWindowText( TASApp.LoadLocalizedString( IDS_UNKNOWN ) );
			m_StaticKvRange.ShowWindow( SW_HIDE );
			m_GroupDpOrKv.SetForceShowDisable( true );
		}
	}
	else
	{
		m_StaticKvRange.ShowWindow( SW_HIDE );
	}
}

void CDlgIndSelDpC::_UpdateDpCLocation()
{
	m_clIndSelDpCParams.m_eDpCLoc = (eDpCLoc)m_ComboDpCLoc.GetCurSel();
}

void CDlgIndSelDpC::_UpdateMvLocation()
{
	m_clIndSelDpCParams.m_eMvLoc = (eMvLoc)m_ComboMvLoc.GetCurSel();

	// If balancing valves is located on the primary...
	if( eMvLoc::MvLocPrimary == m_clIndSelDpCParams.m_eMvLoc )
	{
		// Update the tooltip text for MvLoc combo box.
		if( NULL != m_ToolTip.GetSafeHwnd() )
		{
			CString str = TASApp.LoadLocalizedString( IDS_MVEXCL );	
			m_ToolTip.UpdateTipText( (LPCTSTR) str, &m_ComboMvLoc );
		}
	}
	else
	{
		// Secondary.
		if( NULL != m_ToolTip.GetSafeHwnd() )
		{
			CString str = TASApp.LoadLocalizedString( IDS_MVINCL );	
			m_ToolTip.UpdateTipText( (LPCTSTR) str, &m_ComboMvLoc );
		}
	}

	// Update static KvRange for Kvs suggestion.
	if( eDpStab::DpStabOnCV == m_ComboDpStab.GetCurSel() )
	{
		m_StaticKvRange.SetWindowText( _GetKvsBoundsString() );
	}
}

void CDlgIndSelDpC::_UpdateDpBranchFieldValue()
{
	if( eDpStab::DpStabOnBranch != m_clIndSelDpCParams.m_eDpStab )
	{
		return;
	}

	if( true == m_clIndSelDpCParams.m_bIsGroupDpbranchOrKvChecked )
	{
		if( m_clIndSelDpCParams.m_dDpBranch > 0.0 )
		{
			m_clExtEditDpBranch.SetWindowText( WriteCUDouble( _U_DIFFPRESS, m_clIndSelDpCParams.m_dDpBranch ) );
		}
		else
		{
			m_clExtEditDpBranch.SetWindowText( _T("") );
		}
	}
	else
	{
		m_clExtEditDpBranch.SetWindowText( TASApp.LoadLocalizedString( IDS_UNKNOWN ) );
	}
}

void CDlgIndSelDpC::_UpdateDpBranchFieldState()
{
	if( eDpStab::DpStabOnBranch != m_clIndSelDpCParams.m_eDpStab )
	{
		return;
	}

	if( true == m_clIndSelDpCParams.m_bIsGroupDpbranchOrKvChecked )
	{
		m_clExtEditDpBranch.SetReadOnly( FALSE );
		m_clExtEditDpBranch.EnableWindow( TRUE );
		m_clExtEditDpBranch.SetFocus();
		m_clExtEditDpBranch.SetWindowText( _T("") );

		m_StaticDpBranchUnit.ShowWindow( SW_SHOW );
		m_StaticDpBranchUnit.EnableWindow( TRUE );

		m_GroupDpOrKv.SetForceShowDisable( false );
	}
	else
	{
		m_clExtEditDpBranch.SetReadOnly( TRUE );
		m_clExtEditDpBranch.EnableWindow( FALSE );
		m_clExtEditDpBranch.SetWindowText( TASApp.LoadLocalizedString( IDS_UNKNOWN ) );

		m_StaticDpBranchUnit.EnableWindow( FALSE );
		
		m_GroupDpOrKv.SetForceShowDisable( true );

		m_clIndSelDpCParams.m_dDpBranch = 0.0;
	}
}

void CDlgIndSelDpC::_UpdateStaticKvRange()
{
	CString strKvRange = _T("");

	if( eDpStab::DpStabOnCV == m_clIndSelDpCParams.m_eDpStab && m_clIndSelDpCParams.m_dFlow > 0.0 )
	{
		// Update static KvRange for Kvs suggestion.
		strKvRange = _GetKvsBoundsString();
	}

	m_StaticKvRange.SetWindowText( strKvRange );
}

void CDlgIndSelDpC::_UpdateDpMaxFieldState()
{
	m_clIndSelDpCParams.m_bIsDpMaxChecked = ( BST_CHECKED == m_GroupDpMax.GetCheck() ) ? true : false;

	if( true == m_clIndSelDpCParams.m_bIsDpMaxChecked )
	{
		m_clExtEditDpMax.SetReadOnly( FALSE );
		m_clExtEditDpMax.EnableWindow( TRUE );
		m_clExtEditDpMax.SetFocus();
		m_clExtEditDpMax.SetWindowText( _T("") );
		GetDlgItem( IDC_STATICDPMAXUNIT )->EnableWindow( TRUE );
		m_GroupDpMax.SetForceShowDisable( false );
	}
	else
	{
		m_clIndSelDpCParams.m_dDpMax = 0.0;
		m_clExtEditDpMax.SetReadOnly( FALSE );
		m_clExtEditDpMax.EnableWindow( FALSE );
		m_clExtEditDpMax.SetWindowText( TASApp.LoadLocalizedString( IDS_UNKNOWN ) );
		GetDlgItem( IDC_STATICDPMAXUNIT )->EnableWindow( FALSE );
		m_GroupDpMax.SetForceShowDisable( true );
	}
}
