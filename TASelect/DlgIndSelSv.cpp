#include "stdafx.h"
#include <float.h>
#include <math.h>

#include "TASelect.h"
#include "MainFrm.h"
#include "Global.h"
#include "Select.h"

#include "DlgNoDevFound.h"
#include "DlgConfSel.h"
#include "DlgTechParam.h"

#include "DlgLeftTabSelManager.h"

#include "RViewSSelSS.h"
#include "RViewSSelSv.h"

#include "DlgSelectionBase.h"
#include "DlgIndSelBase.h"
#include "DlgIndSelSv.h"

#ifdef _DEBUG
#define new DEBUG_NEW
#undef THIS_FILE
static char THIS_FILE[] = __FILE__;
#endif

CDlgIndSelSv::CDlgIndSelSv( CWnd *pParent )
	: CDlgIndSelBase( m_clIndSelSVParams, CDlgIndSelSv::IDD, pParent )
{
	m_clIndSelSVParams.m_eProductSubCategory = ProductSubCategory::PSC_ESC_ShutoffValve;
	m_clIndSelSVParams.m_eProductSelectionMode = ProductSelectionMode::ProductSelectionMode_Individual;
}

void CDlgIndSelSv::SaveSelectionParameters()
{
	CDlgIndSelBase::SaveSelectionParameters();
	CDS_IndSelParameter *pclIndSelParameter = m_clIndSelSVParams.m_pTADS->GetpIndSelParameter();

	if( NULL == pclIndSelParameter )
	{
		return;
	}

	pclIndSelParameter->SetSvRadioFlowPowerDT( (int)m_clIndSelSVParams.m_eFlowOrPowerDTMode );
	pclIndSelParameter->SetSvTypeID( m_clIndSelSVParams.m_strComboTypeID );
	pclIndSelParameter->SetSvFamilyID( m_clIndSelSVParams.m_strComboFamilyID );
	pclIndSelParameter->SetSvMaterialID( m_clIndSelSVParams.m_strComboMaterialID );
	pclIndSelParameter->SetSvConnectID( m_clIndSelSVParams.m_strComboConnectID );
	pclIndSelParameter->SetSvVersionID( m_clIndSelSVParams.m_strComboVersionID );
	pclIndSelParameter->SetSvPNID( m_clIndSelSVParams.m_strComboPNID );

	if( NULL != pRViewSSelSv )
	{
		pRViewSSelSv->SaveSelectionParameters( pclIndSelParameter );
	}
}

void CDlgIndSelSv::ChangeApplicationType( ProjectType eProductSelectionApplicationType )
{
	CDlgIndSelBase::ChangeApplicationType( eProductSelectionApplicationType );

	if( NULL != pRViewSSelSv )
	{
		pRViewSSelSv->Reset();
	}
}

//////////////////////////////////////////////////////////////////////////////////////////////////////////////
// PROTECTED METHODS
//////////////////////////////////////////////////////////////////////////////////////////////////////////////

BEGIN_MESSAGE_MAP( CDlgIndSelSv, CDlgIndSelBase )
	ON_CBN_SELCHANGE( IDC_COMBOTYPE, OnCbnSelChangeType )
	ON_CBN_SELCHANGE( IDC_COMBOFAMILY, OnCbnSelChangeFamily )
	ON_CBN_SELCHANGE( IDC_COMBOMATERIAL, OnCbnSelChangeMaterial )
	ON_CBN_SELCHANGE( IDC_COMBOCONNECT, OnCbnSelChangeConnect )
	ON_CBN_SELCHANGE( IDC_COMBOVERSION, OnCbnSelChangeVersion )
	ON_CBN_SELCHANGE( IDC_COMBOPN, OnCbnSelChangePN )
	ON_MESSAGE( WM_USER_MODIFYSELECTEDSV, OnModifySelectedSv )
END_MESSAGE_MAP()

void CDlgIndSelSv::DoDataExchange( CDataExchange *pDX )
{
	CDlgIndSelBase::DoDataExchange( pDX );
	DDX_Control( pDX, IDC_COMBOTYPE, m_ComboType );
	DDX_Control( pDX, IDC_COMBOFAMILY, m_ComboFamily );
	DDX_Control( pDX, IDC_COMBOMATERIAL, m_ComboBdyMat );
	DDX_Control( pDX, IDC_COMBOCONNECT, m_ComboConnect );
	DDX_Control( pDX, IDC_COMBOVERSION, m_ComboVersion );
	DDX_Control( pDX, IDC_COMBOPN, m_ComboPN );
	DDX_Control( pDX, IDC_GROUPVALVE, m_GroupValve );

	// 'CDlgIndSelBase' variables.
	DDX_Control( pDX, IDC_EDITFLOW, m_clExtEditFlow );
	DDX_Control( pDX, IDC_EDITPOWER, m_clExtEditPower );
	DDX_Control( pDX, IDC_EDITDT, m_clExtEditDT );
	DDX_Control( pDX, IDC_GROUPQ, m_clGroupQ );
	DDX_Radio( pDX, IDC_RADIOFLOW, m_iRadioFlowPowerDT );
}

BOOL CDlgIndSelSv::OnInitDialog()
{
	CDlgIndSelBase::OnInitDialog();

	// Initialization of flow and power static text is done in the 'CDlgSelectionBase' base class.
	CString str;
	str = TASApp.LoadLocalizedString( IDS_TABCDIALOGSSELSV_DT );
	GetDlgItem( IDC_STATICDT )->SetWindowText( str );
	str = TASApp.LoadLocalizedString( IDS_TABCDIALOGSSELSV_BUTTONSUGGEST );
	GetDlgItem( IDC_BUTTONSUGGEST )->SetWindowText( str );
	str = TASApp.LoadLocalizedString( IDS_TABCDIALOGSSELSV_BUTTONSELECT );
	GetDlgItem( IDC_BUTTONSELECT )->SetWindowText( str );
	str = TASApp.LoadLocalizedString( IDS_TABCDIALOGSSELSV_BUTTONCANCEL );
	GetDlgItem( IDC_BUTTONCANCEL )->SetWindowText( str );
	str = TASApp.LoadLocalizedString( IDS_TABCDIALOGSSELSV_STATICTYPE );
	GetDlgItem( IDC_STATICTYPE )->SetWindowText( str );
	str = TASApp.LoadLocalizedString( IDS_TABCDIALOGSSELSV_STATICFAMILY );
	GetDlgItem( IDC_STATICFAMILY )->SetWindowText( str );
	str = TASApp.LoadLocalizedString( IDS_TABCDIALOGSSELSV_STATICBODYMAT );
	GetDlgItem( IDC_STATICMATERIAL )->SetWindowText( str );
	str = TASApp.LoadLocalizedString( IDS_TABCDIALOGSSELSV_STATICCONNECT );
	GetDlgItem( IDC_STATICCONNECT )->SetWindowText( str );
	str = TASApp.LoadLocalizedString( IDS_TABCDIALOGSSELSV_STATICVERSION );
	GetDlgItem( IDC_STATICVERSION )->SetWindowText( str );
	str = TASApp.LoadLocalizedString( IDS_TABCDIALOGSSELSV_STATICPN );
	GetDlgItem( IDC_STATICPN )->SetWindowText( str );

	str.Empty();

	// Set proper style and add icons to flow and valve groups.
	CImageList *pclImgListGroupBox = TASApp.GetpRCImageManager()->GetImageList( CRCImageManager::ILN_GroupBox );
	ASSERT( NULL != pclImgListGroupBox );

	if( NULL != pclImgListGroupBox )
	{
		m_GroupValve.SetImageList( pclImgListGroupBox, CRCImageManager::ILGB_Valve );
	}

	CImageList *pclImgListGroupBoxGrayed = TASApp.GetpRCImageManager()->GetImageList( CRCImageManager::ILN_GroupBoxGrayed );
	ASSERT( NULL != pclImgListGroupBoxGrayed );

	if( NULL != pclImgListGroupBoxGrayed )
	{
		m_GroupValve.SetImageList( pclImgListGroupBoxGrayed, CRCImageManager::ILGB_Valve, true );
	}

	m_GroupValve.SetInOffice2007Mainframe( true );

	// PAY ATTENTION: if it's feature is reactivated, there is some changes to do in 'OnLButtonDown' (ask to FF).
	// 	m_GroupValve.SetExpandCollapseMode( true );
	// 	m_GroupValve.SetNotificationHandler( this );

	if( NULL != pclImgListGroupBox )
	{
		GetpGroupQ()->SetImageList( pclImgListGroupBox, CRCImageManager::ILGB_Q );
	}

	if( NULL != pclImgListGroupBoxGrayed )
	{
		GetpGroupQ()->SetImageList( pclImgListGroupBoxGrayed, CRCImageManager::ILGBG_Q, true );
	}

	GetpGroupQ()->SetInOffice2007Mainframe( true );

	return TRUE;
}

void CDlgIndSelSv::OnCbnSelChangeType()
{
	m_clIndSelSVParams.m_strComboTypeID = m_ComboType.GetCBCurSelIDPtr().ID;
	_FillComboFamily();

	// The 'CComboBox::SetCurSel' method used to fill combos doesn't fire the 'ON_CBN_SELCHANGE'.
	// We have thus to do it manually.
	OnCbnSelChangeFamily();
}

void CDlgIndSelSv::OnCbnSelChangeFamily()
{
	m_clIndSelSVParams.m_strComboFamilyID = m_ComboFamily.GetCBCurSelIDPtr().ID;
	_FillComboBodyMat();

	// The 'CComboBox::SetCurSel' method used to fill combos doesn't fire the 'ON_CBN_SELCHANGE'.
	// We have thus to do it manually.
	OnCbnSelChangeMaterial();
}

void CDlgIndSelSv::OnCbnSelChangeMaterial()
{
	m_clIndSelSVParams.m_strComboMaterialID = m_ComboBdyMat.GetCBCurSelIDPtr().ID;
	_FillComboConnect();

	// The 'CComboBox::SetCurSel' method used to fill combos doesn't fire the 'ON_CBN_SELCHANGE'.
	// We have thus to do it manually.
	OnCbnSelChangeConnect();
}

void CDlgIndSelSv::OnCbnSelChangeConnect()
{
	m_clIndSelSVParams.m_strComboConnectID = m_ComboConnect.GetCBCurSelIDPtr().ID;
	_FillComboVersion();

	// The 'CComboBox::SetCurSel' method used to fill combos doesn't fire the 'ON_CBN_SELCHANGE'.
	// We have thus to do it manually.
	OnCbnSelChangeVersion();
}

void CDlgIndSelSv::OnCbnSelChangeVersion()
{
	m_clIndSelSVParams.m_strComboVersionID = m_ComboVersion.GetCBCurSelIDPtr().ID;
	_FillComboPN();

	// The 'CComboBox::SetCurSel' method used to fill combos doesn't fire the 'ON_CBN_SELCHANGE'.
	// We have thus to do it manually.
	OnCbnSelChangePN();
}

void CDlgIndSelSv::OnCbnSelChangePN()
{
	m_clIndSelSVParams.m_strComboPNID = m_ComboPN.GetCBCurSelIDPtr().ID;
	ClearAll();
}

LRESULT CDlgIndSelSv::OnModifySelectedSv( WPARAM wParam, LPARAM lParam )
{
	if( NULL == TASApp.GetpTADS() || NULL == TASApp.GetpTADS()->GetpTechParams() )
	{
		ASSERTA_RETURN( -1 );
	}

	CDS_SSelSv *pSelSv = dynamic_cast<CDS_SSelSv *>( ( CData * )lParam );
	ASSERT( NULL != pSelSv );

	if( NULL == pSelSv || NULL == pDlgLTtabctrl )
	{
		return 0;
	}

	m_clIndSelSVParams.m_SelIDPtr = pSelSv->GetIDPtr();
	m_bInitialised = false;

	pDlgLTtabctrl->SetActiveTab( CMyMFCTabCtrl::TabIndex::etiSSel );

	// If we are not in the good selection mode, we must first disable it.
	if( ProductSelectionMode_Individual != pMainFrame->GetCurrentProductSelectionMode() )
	{
		pMainFrame->ChangeProductSelectionMode( ProductSelectionMode_Individual );
	}

	// HYS-1242: SetRedraw to false to avoid too much refresh in ClickRibbonProductSelGallery. 
	pMainFrame->SetRedraw( FALSE );
	// Select the right item in the gallery box.
	pMainFrame->ClickRibbonProductSelGallery( ProductSubCategory::PSC_ESC_ShutoffValve );

	// Select 'Heating, 'Cooling' or 'Solar'.
	m_clIndSelSVParams.m_eApplicationType = pSelSv->GetpSelectedInfos()->GetApplicationType();
	pMainFrame->ChangeRbnProductSelectionApplicationType( m_clIndSelSVParams.m_eApplicationType );

	// Transfer water characteristics.
	m_clIndSelSVParams.m_WC = *( pSelSv->GetpSelectedInfos()->GetpWCData() );

	// Copy water characteristic and temperature data in the technical parameters.
	OnModifySelectedProductUpdateTechData( pSelSv->GetpSelectedInfos() );

	// HYS-1242: To refresh the window in InitializeRibbonRichEditBoxesSSel
	pMainFrame->SetRedraw( TRUE );
	pMainFrame->InitializeRibbonRichEditBoxesSSel();

	// Remark: The 'm_clIndSelSVParams.m_strPipeSeriesID' and 'm_clIndSelSVParams.m_strPipeID' are both updated
	//         by calling the 'SelectCurrentComboPipes' method.
	SelectCurrentComboPipes( pSelSv->GetPipeSeriesID(), pSelSv->GetPipeID() );

	// Remark: Corresponding combo variables in 'm_clIndSelSVParams' are updated in each of this following methods.
	_FillComboType( pSelSv->GetTypeID() );
	_FillComboFamily( pSelSv->GetFamilyID() );
	_FillComboBodyMat( pSelSv->GetMaterialID() );
	_FillComboConnect( pSelSv->GetConnectID() );
	_FillComboVersion( pSelSv->GetVersionID() );
	_FillComboPN( pSelSv->GetPNID() );

	m_iRadioFlowPowerDT = ( CDS_SelProd::efdFlow == pSelSv->GetFlowDef() ) ? Radio_Flow : Radio_PowerDT;
	
	// 'FALSE' to change the radio state in the dialog with the content of the 'm_iRadioFlowPowerDT' variable.	
	UpdateData( FALSE );
	
	// Remark: 'm_clIndSelSVParams.m_eFlowOrPowerDTMode' is updated in the following method in regards to the 'm_iRadioFlowPowerDT' variable.
	CDlgIndSelBase::UpdateFlowOrPowerDTState();

	if( CDS_SelProd::efdFlow == m_clIndSelSVParams.m_eFlowOrPowerDTMode )
	{
		m_clIndSelSVParams.m_dFlow = pSelSv->GetQ();
		UpdateFlowFieldValue();
	}
	else
	{
		m_clIndSelSVParams.m_dPower = pSelSv->GetPower();
		UpdatePowerFieldValue();

		m_clIndSelSVParams.m_dDT = pSelSv->GetDT();
		UpdateDTFieldValue();
	}

	m_bInitialised = true;

	// By default now we automatically enable apply button.
	EnableSelectButton( true );

	// Remark: the 'm_pclIndSelParams->m_bEditModeRunning' variable is updated by calling the 'SetModificationMode' method.
	SetModificationMode( true );
	PostWMCommandToControl( GetpBtnSuggest() );

	return 0;
}

void CDlgIndSelSv::OnBnClickedSuggest()
{
	ClearAll();

	if( true == _VerifySubmittedValues() )
	{
		m_clIndSelSVParams.m_SVList.PurgeAll();

		m_clIndSelSVParams.m_pTADB->GetSvList( 
				&m_clIndSelSVParams.m_SVList,
				(LPCTSTR)m_clIndSelSVParams.m_strComboTypeID,
				(LPCTSTR)m_clIndSelSVParams.m_strComboFamilyID, 
				(LPCTSTR)m_clIndSelSVParams.m_strComboMaterialID, 
				(LPCTSTR)m_clIndSelSVParams.m_strComboConnectID, 
				(LPCTSTR)m_clIndSelSVParams.m_strComboVersionID,
				m_clIndSelSVParams.m_eFilterSelection );

		_SuggestIndividualSelection();
	}
}

void CDlgIndSelSv::OnBnClickedSelect()
{
	// Verify the dialog is active.
	if( FALSE == this->IsWindowVisible() )
	{
		return;
	}

	_SelectIndividualSelection();
}

void CDlgIndSelSv::ActivateLeftTabDialog()
{
	CDlgIndSelBase::ActivateLeftTabDialog();

	// HYS-1263 : Added to update application type if it is necessary
	CDS_TechnicalParameter *pTechP = m_clIndSelSVParams.m_pTADS->GetpTechParams();
	ASSERT( NULL != pTechP );

	ProjectType eCurrentPSApplicationType = pTechP->GetProductSelectionApplicationType();

	if( eCurrentPSApplicationType != m_clIndSelSVParams.m_eApplicationType )
	{
		ChangeApplicationType( eCurrentPSApplicationType );
	}
}

LRESULT CDlgIndSelSv::OnNewDocument( WPARAM wParam, LPARAM lParam )
{
	CDlgIndSelBase::OnNewDocument( wParam, lParam );

	// Get last selected parameters.
	CString str1, str2, str3;
	CDS_IndSelParameter *pclIndSelParameter = m_clIndSelSVParams.m_pTADS->GetpIndSelParameter();

	if( NULL == pclIndSelParameter )
	{
		return -1;
	}

	m_iRadioFlowPowerDT = ( CDS_SelProd::efdFlow == (CDS_SelProd::eFlowDef)pclIndSelParameter->GetSvRadioFlowPowerDT() ) ? Radio_Flow : Radio_PowerDT;

	// 'FALSE' to change the radio state in the dialog with the content of the 'm_iRadioFlowPowerDT' variable.
	UpdateData( FALSE );
	
	// Remark: 'm_clIndSelSVParams.m_eFlowOrPowerDTMode' is updated in the following method in regards to the 'm_iRadioFlowPowerDT' variable.
	CDlgIndSelBase::UpdateFlowOrPowerDTState();

	// Remark: Corresponding combo variables in 'm_clIndSelSVParams' are updated in each of this following methods.
	_FillComboType( pclIndSelParameter->GetSvTypeID() );
	_FillComboFamily( pclIndSelParameter->GetSvFamilyID() );
	_FillComboBodyMat( pclIndSelParameter->GetSvMaterialID() );
	_FillComboConnect( pclIndSelParameter->GetSvConnectID() );
	_FillComboVersion( pclIndSelParameter->GetSvVersionID() );
	_FillComboPN( pclIndSelParameter->GetSvPNID() );

	// Fill the flow static control and update water TCHAR. strings.
	OnUnitChange();
	m_bInitialised = true;

	if( NULL != pRViewSSelSv )
	{
		pRViewSSelSv->OnNewDocument( pclIndSelParameter );
	}

	return 0;
}

LRESULT CDlgIndSelSv::OnPipeChange( WPARAM wParam, LPARAM lParam )
{
	CDlgIndSelBase::OnPipeChange( wParam, lParam );

	if( NULL != pRViewSSelSv )
	{
		pRViewSSelSv->Reset();
	}

	return 0;
}

LRESULT CDlgIndSelSv::OnTechParChange( WPARAM wParam, LPARAM lParam )
{
	CDlgIndSelBase::OnTechParChange( wParam, lParam );

	if( NULL != pRViewSSelSv )
	{
		pRViewSSelSv->Reset();
	}

	return 0;
}

LRESULT CDlgIndSelSv::OnUnitChange( WPARAM wParam, LPARAM lParam )
{
	if( WMUserPipeChange::WM_UPC_ForProductSelection != ( WMUserPipeChange )wParam )
	{
		return 0;
	}

	CDlgIndSelBase::OnUnitChange( wParam, lParam );

	if( NULL != pRViewSSelSv )
	{
		pRViewSSelSv->Reset();
	}

	return 0;
}

LRESULT CDlgIndSelSv::OnWaterChange( WPARAM wParam, LPARAM lParam )
{
	// HYS-1693: Not for changeOver application type
	if( WMUserWaterCharWParam::WM_UWC_WP_ForProductSel != ( ( WMUserWaterCharWParam )wParam & WMUserWaterCharWParam::WM_UWC_WP_ForProductSel ) 
		|| WMUserWaterCharLParam::WM_UWC_LWP_ChangeForChangeOver == (WMUserWaterCharLParam)lParam
		|| WMUserWaterCharLParam::WM_UWC_LWP_NoChangeForChangeOver == (WMUserWaterCharLParam)lParam )
	{
		return 0;
	}

	CDlgIndSelBase::OnWaterChange( wParam, lParam );

	if( NULL != pRViewSSelSv )
	{
		pRViewSSelSv->Reset();
	}

	return 0;
}

CRViewSSelSS *CDlgIndSelSv::GetLinkedRightViewSSel( void )
{
	return pRViewSSelSv;
}

//////////////////////////////////////////////////////////////////////////////////////////////////////////////
// PRIVATE METHODS
//////////////////////////////////////////////////////////////////////////////////////////////////////////////

bool CDlgIndSelSv::_SuggestIndividualSelection()
{
	if( NULL == pRViewSSelSv )
	{
		return false;
	}

	if( NULL != m_clIndSelSVParams.m_pclSelectSVList )
	{
		delete m_clIndSelSVParams.m_pclSelectSVList;
	}

	m_clIndSelSVParams.m_pclSelectSVList = new CSelectShutoffList();

	if( NULL == m_clIndSelSVParams.m_pclSelectSVList || NULL == m_clIndSelSVParams.m_pclSelectSVList->GetSelectPipeList() )
	{
		return false;
	}

	m_clIndSelSVParams.m_pclSelectSVList->GetSelectPipeList()->SelectPipes( &m_clIndSelSVParams, m_clIndSelSVParams.m_dFlow );

	bool bSizeShiftProblem = false;
	bool bBestFound = false;

	int iDevFound = m_clIndSelSVParams.m_pclSelectSVList->SelectShutoffValve( &m_clIndSelSVParams, &bSizeShiftProblem, bBestFound );

	if( iDevFound != 0 && false == bSizeShiftProblem )
	{
		VerifyModificationMode();
		pRViewSSelSv->Suggest( &m_clIndSelSVParams );
	}
	else
	{
		if( false == bSizeShiftProblem )
		{
			m_clIndSelSVParams.m_SVList.PurgeAll();

			m_clIndSelSVParams.m_pTADB->GetSvList( &m_clIndSelSVParams.m_SVList, (LPCTSTR)m_clIndSelSVParams.m_strComboTypeID, _T("" ), _T("" ), _T( ""), _T( ""), 
					m_clIndSelSVParams.m_eFilterSelection );
			
			iDevFound = m_clIndSelSVParams.m_pclSelectSVList->SelectShutoffValve( &m_clIndSelSVParams, &bSizeShiftProblem, bBestFound );

			if( iDevFound > 0 )
			{
				// HYS-697: Temporary solution because it is not practical to pass through 2 lists to go in the 'CDlgNoDevFound' dialog.
				m_clIndSelSVParams.m_SVList.PurgeAll();
				double dKey = 0.0;

				for( CSelectedValve *pclSelectedValve = m_clIndSelSVParams.m_pclSelectSVList->GetFirst<CSelectedValve>(); NULL != pclSelectedValve;
					pclSelectedValve = m_clIndSelSVParams.m_pclSelectSVList->GetNext<CSelectedValve>() )
				{
					CDB_TAProduct *pTAP = dynamic_cast<CDB_TAProduct *>( pclSelectedValve->GetpData() );

					if( NULL == pTAP )
					{
						continue;
					}

					m_clIndSelSVParams.m_SVList.Add( pTAP->GetIDPtr().ID, dKey++, (LPARAM)pTAP->GetIDPtr().MP );
				}

				// Launch the no device found dialog box if something is found.
				// Remark: Combos in this dialog will be filled thanks to the new alternative shut-off valves found and saved in the 'm_SvList' variable.
				CDlgNoDevFoundParams clNoDevParams = CDlgNoDevFoundParams( &m_clIndSelSVParams.m_SVList, m_clIndSelSVParams.m_strComboTypeID, m_clIndSelSVParams.m_strComboFamilyID, 
						m_clIndSelSVParams.m_strComboMaterialID, m_clIndSelSVParams.m_strComboConnectID, m_clIndSelSVParams.m_strComboVersionID, m_clIndSelSVParams.m_strComboPNID );
				
				CDlgNoDevFound dlg( &m_clIndSelSVParams, &clNoDevParams );

				if( IDOK == dlg.DoModal() )
				{
					clNoDevParams = *dlg.GetNoDevFoundParams();

					_LaunchSuggestion( clNoDevParams.m_strTypeID, clNoDevParams.m_strFamilyID, clNoDevParams.m_strMaterialID, clNoDevParams.m_strConnectID, 
							clNoDevParams.m_strVersionID, clNoDevParams.m_strPNID );
				}
			}
			else
			{
				// No device found message.
				TASApp.AfxLocalizeMessageBox( AFXMSG_NO_DEV_FOUND );
			}
		}
		else
		{
			// In that case valves exist but not with the parameters specified.
			// Problem of SizeShift.
			if( IDYES == TASApp.AfxLocalizeMessageBox( AFXMSG_SIZE_SHIFT_PROB, MB_YESNO | MB_ICONQUESTION, 0 ) )
			{
				CDlgTechParam *pdlg = new CDlgTechParam( CDlgTechParam::PageGeneralDeviceSizes );

				if( NULL != pdlg && IDOK != pdlg->DoModal() )
				{
					// Set the focus on the first available edit control.
					SetFocusOnControl( CDlgSelectionBase::SetFocusWhere::First | CDlgSelectionBase::SetFocusWhere::Edit );
				}

				delete pdlg;
			}
			else
			{
				// Set the focus on the first available edit control.
				SetFocusOnControl( CDlgSelectionBase::SetFocusWhere::First | CDlgSelectionBase::SetFocusWhere::Edit );
			}
		}
	}

	return true;
}

bool CDlgIndSelSv::_SelectIndividualSelection()
{
	try
	{
		if( NULL == m_clIndSelSVParams.m_pTADS )
		{
			HYSELECT_THROW( _T("Internal error: 'm_clIndSelSVParams.m_pTADS' argument can't be NULL.") );
		}
		else if( NULL == m_clIndSelSVParams.m_pTADS->GetpTechParams() )
		{
			HYSELECT_THROW( _T("Internal error: 'm_clIndSelSVParams.m_pTADS->GetpTechParams()' argument can't be NULL.") );
		}

		// Verify the dialog is active.
		if( NULL == pRViewSSelSv )
		{
			HYSELECT_THROW( _T("Internal error: The shut-off valve right view is not created.") );
		}
		else if( FALSE == this->IsWindowVisible() )
		{
			HYSELECT_THROW( _T("Internal error: The shut-off valve right view is not visible.") );
		}

		// Verify pointer validity; user can delete object after entering in edition.
		if( _T('\0') != *m_clIndSelSVParams.m_SelIDPtr.ID )
		{
			m_clIndSelSVParams.m_SelIDPtr = m_clIndSelSVParams.m_pTADS->Get( m_clIndSelSVParams.m_SelIDPtr.ID );
		}

		// Create selected object and initialize it.
		CDS_SSelSv *pSelSv = dynamic_cast<CDS_SSelSv *>( m_clIndSelSVParams.m_SelIDPtr.MP );

		if( NULL == pSelSv )
		{
			IDPTR IDPtr;
			m_clIndSelSVParams.m_pTADS->CreateObject( IDPtr, CLASS( CDS_SSelSv ) );
			pSelSv = (CDS_SSelSv *)( IDPtr.MP );
		}

		if( m_clIndSelSVParams.m_dFlow <= 0.0 )
		{
			HYSELECT_THROW( _T("Internal error: Flow '%f' can't be lower or equal to 0."), m_clIndSelSVParams.m_dFlow );
		}

		pSelSv->SetQ( m_clIndSelSVParams.m_dFlow );

		if( CDS_SelProd::efdFlow == m_clIndSelSVParams.m_eFlowOrPowerDTMode )
		{
			pSelSv->SetFlowDef( CDS_SelProd::efdFlow );
			pSelSv->SetPower( 0.0 );
			pSelSv->SetDT( 0.0 );
		}
		else
		{
			pSelSv->SetFlowDef( CDS_SelProd::efdPower );
			pSelSv->SetPower( m_clIndSelSVParams.m_dPower );
			pSelSv->SetDT( m_clIndSelSVParams.m_dDT );	
		}
	
		pSelSv->SetTypeID( m_clIndSelSVParams.m_strComboTypeID );
		pSelSv->SetFamilyID( m_clIndSelSVParams.m_strComboFamilyID );
		pSelSv->SetMaterialID( m_clIndSelSVParams.m_strComboMaterialID );
		pSelSv->SetConnectID( m_clIndSelSVParams.m_strComboConnectID );
		pSelSv->SetVersionID( m_clIndSelSVParams.m_strComboVersionID );
		pSelSv->SetPNID( m_clIndSelSVParams.m_strComboPNID );
		pSelSv->SetPipeSeriesID( m_clIndSelSVParams.m_strPipeSeriesID );
		pSelSv->SetPipeID( m_clIndSelSVParams.m_strPipeID );

		*pSelSv->GetpSelectedInfos()->GetpWCData() = m_clIndSelSVParams.m_WC;

		// We save here the real DT in the ribbon to be able to restore it when we edit a selection.
		// Remark: the DT in the ribbon is never used in the computing. It is the DT in the left panel that is used when
		//         user chooses the Power/DT mode input.
		pSelSv->GetpSelectedInfos()->SetDT( m_clIndSelSVParams.m_pTADS->GetpTechParams()->GetDefaultISDT() );

		// For the same reason, we also save the application type.
		pSelSv->GetpSelectedInfos()->SetApplicationType( m_clIndSelSVParams.m_eApplicationType );

		pRViewSSelSv->FillInSelected( pSelSv );

		CDlgConfSel dlg( &m_clIndSelSVParams );
		IDPTR IDPtr = pSelSv->GetIDPtr();
		dlg.Display( pSelSv );

		if( IDOK == dlg.DoModal() )
		{
			CTable *pTab = dynamic_cast<CTable *>( m_clIndSelSVParams.m_pTADS->Get( _T("SHUTOFF_TAB") ).MP );
			
			if( NULL == pTab )
			{
				HYSELECT_THROW( _T("Internal error: Can't retrieve the 'SHUTOFF_TAB' table from the datastruct.") );
			}

			if( _T('\0') == *m_clIndSelSVParams.m_SelIDPtr.ID )
			{
				pTab->Insert( IDPtr );
				m_clIndSelSVParams.m_pTADS->SetNewIndex( CLASS( CDS_SSelSv ) );
			}

			// It's only in case user has clicked 'OK' button that we can set modification mode to 'false'. Otherwise if user
			// has clicked 'Cancel' button, we must stay in edition mode and not activate 'Select' button.
			if( true == m_clIndSelSVParams.m_bEditModeRunning )
			{
				SetModificationMode( false );
				PostWMCommandToControl( GetpBtnSuggest() );
			}

			// Enable 'Select' button.
			EnableSelectButton( true );
		}
		else
		{
			if( _T('\0') == *m_clIndSelSVParams.m_SelIDPtr.ID )
			{
				m_clIndSelSVParams.m_pTADS->DeleteObject( IDPtr );
			}
		}

		if( NULL != dlg.GetSafeHwnd() )
		{
			dlg.DestroyWindow();
		}

		return true;
	}
	catch( CHySelectException &clHySelectException )
	{
		clHySelectException.AddMessage( _T("Error in 'CDlgIndSelSv::_SelectIndividualSelection'."), __LINE__, __FILE__ );
		throw;
	}
}

void CDlgIndSelSv::_FillComboType( CString strTypeID )
{
	if( NULL == m_clIndSelSVParams.m_pTADB )
	{
		ASSERT_RETURN;
	}

	CRankEx TypeList;
	m_clIndSelSVParams.m_pTADB->GetSvTypeList( &TypeList, m_clIndSelSVParams.m_eFilterSelection );
	m_ComboType.FillInCombo( &TypeList, strTypeID );
	m_clIndSelSVParams.m_strComboTypeID = m_ComboType.GetCBCurSelIDPtr().ID;

	CString str( _T("") );

	if( CB_ERR != m_ComboType.GetCurSel() )
	{
		m_ComboType.GetLBText( m_ComboType.GetCurSel(), str );

		if( NULL != m_ToolTip.GetSafeHwnd() )
		{
			m_ToolTip.UpdateTipText( (LPCTSTR)str, &m_ComboType );
		}
	}

	OnCbnSelChangeType();
}

void CDlgIndSelSv::_FillComboFamily( CString strFamilyID )
{
	if( NULL == m_clIndSelSVParams.m_pTADB )
	{
		ASSERT_RETURN;
	}

	CRankEx FamilyList;
	m_clIndSelSVParams.m_pTADB->GetSvFamilyList( &FamilyList, (LPCTSTR)m_clIndSelSVParams.m_strComboTypeID, m_clIndSelSVParams.m_eFilterSelection );

	m_ComboFamily.FillInCombo( &FamilyList, strFamilyID, m_clIndSelSVParams.GetComboFamilyAllID() );
	m_clIndSelSVParams.m_strComboFamilyID = m_ComboFamily.GetCBCurSelIDPtr().ID;
}

void CDlgIndSelSv::_FillComboBodyMat( CString strBodyMaterialID )
{
	if( NULL == m_clIndSelSVParams.m_pTADB )
	{
		ASSERT_RETURN;
	}

	CRankEx MaterialList;

	m_clIndSelSVParams.m_pTADB->GetSvBdyMatList( &MaterialList, (LPCTSTR)m_clIndSelSVParams.m_strComboTypeID, (LPCTSTR)m_clIndSelSVParams.m_strComboFamilyID, 
			m_clIndSelSVParams.m_eFilterSelection );

	m_ComboBdyMat.FillInCombo( &MaterialList, strBodyMaterialID, m_clIndSelSVParams.GetComboMaterialAllID() );
	m_clIndSelSVParams.m_strComboMaterialID = m_ComboBdyMat.GetCBCurSelIDPtr().ID;
}

void CDlgIndSelSv::_FillComboConnect( CString strConnectID )
{
	if( NULL == m_clIndSelSVParams.m_pTADB )
	{
		ASSERT_RETURN;
	}

	CRankEx ConnectList;

	m_clIndSelSVParams.m_pTADB->GetSvConnList( &ConnectList, (LPCTSTR)m_clIndSelSVParams.m_strComboTypeID, (LPCTSTR)m_clIndSelSVParams.m_strComboFamilyID, 
			(LPCTSTR)m_clIndSelSVParams.m_strComboMaterialID, m_clIndSelSVParams.m_eFilterSelection );

	m_ComboConnect.FillInCombo( &ConnectList, strConnectID, m_clIndSelSVParams.GetComboConnectAllID() );
	m_clIndSelSVParams.m_strComboConnectID = m_ComboConnect.GetCBCurSelIDPtr().ID;
}

void CDlgIndSelSv::_FillComboVersion( CString strVersionID )
{
	if( NULL == m_clIndSelSVParams.m_pTADB )
	{
		ASSERT_RETURN;
	}

	CRankEx VersionList;

	m_clIndSelSVParams.m_pTADB->GetSvVersList( &VersionList, (LPCTSTR)m_clIndSelSVParams.m_strComboTypeID, (LPCTSTR)m_clIndSelSVParams.m_strComboFamilyID, 
			(LPCTSTR)m_clIndSelSVParams.m_strComboMaterialID, (LPCTSTR)m_clIndSelSVParams.m_strComboConnectID, m_clIndSelSVParams.m_eFilterSelection );

	m_ComboVersion.FillInCombo( &VersionList, strVersionID, m_clIndSelSVParams.GetComboVersionAllID() );
	m_clIndSelSVParams.m_strComboVersionID = m_ComboVersion.GetCBCurSelIDPtr().ID;
}

void CDlgIndSelSv::_FillComboPN( CString strPNID )
{
	if( NULL == m_clIndSelSVParams.m_pTADB )
	{
		ASSERT_RETURN;
	}

	CRankEx PNList;

	m_clIndSelSVParams.m_pTADB->GetSvPNList( &PNList, (LPCTSTR)m_clIndSelSVParams.m_strComboTypeID, (LPCTSTR)m_clIndSelSVParams.m_strComboFamilyID, 
			(LPCTSTR)m_clIndSelSVParams.m_strComboMaterialID, (LPCTSTR)m_clIndSelSVParams.m_strComboConnectID, (LPCTSTR)m_clIndSelSVParams.m_strComboVersionID, 
			m_clIndSelSVParams.m_eFilterSelection );

	m_ComboPN.FillInCombo( &PNList, strPNID, m_clIndSelSVParams.GetComboPNAllID() );
	m_clIndSelSVParams.m_strComboPNID = m_ComboPN.GetCBCurSelIDPtr().ID;
}

void CDlgIndSelSv::_LaunchSuggestion( CString strTypeID, CString strFamilyID, CString strMaterialID, CString strConnectID, CString strVersionID, CString strPNID )
{
	_FillComboType( strTypeID );
	_FillComboFamily( strFamilyID );
	_FillComboBodyMat( strMaterialID );
	_FillComboConnect( strConnectID );
	_FillComboVersion( strVersionID );
	_FillComboPN( strPNID );

	PostWMCommandToControl( GetpBtnSuggest() );
}

bool CDlgIndSelSv::_VerifySubmittedValues()
{
	bool bValid = true;

	if( m_clIndSelSVParams.m_dFlow <= 0.0 )
	{
		TASApp.AfxLocalizeMessageBox( AFXMSG_NO_SUGGESTION_AT_ZERO_FLOW );
		GetDlgItem( IDC_EDITFLOW )->SetFocus();
		bValid = false;
	}

	return bValid;
}
