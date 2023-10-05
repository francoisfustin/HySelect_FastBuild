#include "stdafx.h"
#include "TASelect.h"
#include "MainFrm.h"
#include "ProductSelectionParameters.h"
#include "DlgCOWaterChar.h"


#ifdef _DEBUG
#define new DEBUG_NEW
#undef THIS_FILE
static char THIS_FILE[] = __FILE__;
#endif

CDlgCOWaterChar::CDlgCOWaterChar( CProductSelelectionParameters *pclProductSelParams, CDlgCOWaterChar::INotificationHandler *pfnVerifyFluidCharacteristics, CWnd *pParent )
	: CDialogEx( CDlgCOWaterChar::IDD, pParent )
{
	m_pclProductSelParams = pclProductSelParams;
	m_pfnVerifyFluidCharacteristics = pfnVerifyFluidCharacteristics;
	m_pTADB = NULL;
	m_pTADS = NULL;
	m_ptstrCaption = NULL;
	m_eOrigin = WMUserWaterCharWParam::WM_UWC_WP_ForProductSel;
	m_AdditFamID = _T( "" );
	m_AdditID = _T( "" );
	m_dCoolingTemp = 0.0;
	m_dHeatingTemp = 0.0;
	m_dPCWeight = 0.0;
	m_dCoolingDensity = 0.0;
	m_dCoolingKinVisc = 0.0;
	m_dCoolingSpecifHeat = 0.0;
	m_dCoolingTempfreez = 0.0;
	m_dDefaultCoolingDT = 0.0;
	m_dHeatingDensity = 0.0;
	m_dHeatingKinVisc = 0.0;
	m_dHeatingSpecifHeat = 0.0;
	m_dHeatingTempfreez = 0.0;
	m_dDefaultHeatingDT = 0.0;
	m_pAdditFamTab = NULL;
	m_pAdditChar = NULL;
	m_PrjType = Heating;
}

int CDlgCOWaterChar::Display( LPCTSTR lptstrCaption, WMUserWaterCharWParam eOrigin )
{
	m_ptstrCaption = lptstrCaption;
	m_eOrigin = eOrigin;

	if( NULL != m_pclProductSelParams && NULL != m_pclProductSelParams->m_pTADB )
	{
		m_pTADB = m_pclProductSelParams->m_pTADB;
	}
	else
	{
		m_pTADB = TASApp.GetpTADB();
	}

	if( NULL != m_pclProductSelParams && NULL != m_pclProductSelParams->m_pTADS )
	{
		m_pTADS = m_pclProductSelParams->m_pTADS;
	}
	else
	{
		m_pTADS = TASApp.GetpTADS();
	}

	return DoModal();
}

BEGIN_MESSAGE_MAP( CDlgCOWaterChar, CDialogEx )
	ON_WM_DESTROY()
	ON_EN_CHANGE( IDC_EDITTEMP_C, OnChangeEditCoolingTemperature )
	ON_EN_CHANGE( IDC_EDITTEMP_H, OnChangeEditHeatingTemperature )
	ON_EN_CHANGE( IDC_EDITDT_C, OnChangeEditCoolingDT )
	ON_EN_CHANGE( IDC_EDITDT_H, OnChangeEditHeatingDT )
	ON_CBN_SELCHANGE( IDC_COMBOADDITFAM, OnCbnSelChangeAdditiveFamily )
	ON_CBN_SELCHANGE( IDC_COMBOADDITNAME, OnCbnSelChangeAdditiveName )
	ON_EN_CHANGE( IDC_EDITPCWEIGHT, OnChangeEditPCWeight )
	ON_BN_CLICKED( IDC_BUTTONRECOMMENDATION, OnButtonRecommendation )
	ON_NOTIFY( WM_USER_ENTERKEYPRESSED, IDC_EDITTEMP_C, OnEditEnterCoolingTemperature )
	ON_NOTIFY( WM_USER_ENTERKEYPRESSED, IDC_EDITTEMP_H, OnEditEnterHeatingTemperature )
	ON_EN_KILLFOCUS( IDC_EDITTEMP_C, OnEnKillFocusCoolingTemperature )
	ON_EN_KILLFOCUS( IDC_EDITTEMP_H, OnEnKillFocusHeatingTemperature )
	ON_EN_KILLFOCUS( IDC_EDITDT_C, OnEnKillFocusCoolingDT )
	ON_EN_KILLFOCUS( IDC_EDITDT_H, OnEnKillFocusHeatingDT )
	ON_EN_KILLFOCUS( IDC_EDITPCWEIGHT, OnEnKillFocusPCWeight )
	ON_MESSAGE( WM_USER_UNITCHANGE, OnUnitChange )
END_MESSAGE_MAP()

void CDlgCOWaterChar::DoDataExchange( CDataExchange *pDX )
{
	CDialogEx::DoDataExchange( pDX );
	DDX_Control( pDX, IDC_STATICGROUP_C, m_GroupCooling );
	DDX_Control( pDX, IDC_STATICGROUP_H, m_GroupHeating );
	DDX_Control( pDX, IDC_GROUPADDIT, m_GroupAddit );
	DDX_Control( pDX, IDC_EDITTEMP_C, m_EditCoolingTemp );
	DDX_Control( pDX, IDC_EDITTEMP_H, m_EditHeatingTemp );
	DDX_Control( pDX, IDC_EDITDT_C, m_EditDefaultCoolingDT );
	DDX_Control( pDX, IDC_EDITDT_H, m_EditDefaultHeatingDT );
	DDX_Control( pDX, IDC_COMBOADDITFAM, m_ComboAdditFam );
	DDX_Control( pDX, IDC_COMBOADDITNAME, m_ComboAdditName );
	DDX_Control( pDX, IDC_BUTTONRECOMMENDATION, m_clButtonRecommendation );
	DDX_Control( pDX, IDC_EDITPCWEIGHT, m_EditPcWeight );
	DDX_Control( pDX, IDC_EDITDENSITY_C, m_EditCoolingDens );
	DDX_Control( pDX, IDC_EDITDENSITY_H, m_EditHeatingDens );
	DDX_Control( pDX, IDC_EDITSPECIFHEAT_C, m_EditCoolingSpecifHeat );
	DDX_Control( pDX, IDC_EDITSPECIFHEAT_H, m_EditHeatingSpecifHeat );
	DDX_Control( pDX, IDC_EDITKINVISC_C, m_EditCoolingKinVisc );
	DDX_Control( pDX, IDC_EDITKINVISC_H, m_EditHeatingKinVisc );
	DDX_Control( pDX, IDC_EDITFREEZ_C, m_EditCoolingFreezPoint );
	DDX_Control( pDX, IDC_EDITFREEZ_H, m_EditHeatingFreezPoint );
	DDX_Control( pDX, IDC_EDITWARNING_C, m_StaticCoolingWarning );
	DDX_Control( pDX, IDC_EDITWARNING_H, m_StaticHeatingWarning );
	DDX_Control( pDX, IDOK, m_clButtonOK );
}

BOOL CDlgCOWaterChar::OnInitDialog()
{
	CDialogEx::OnInitDialog();

	// Initialize dialog strings.

	CString str = CteEMPTY_STRING;
	str = TASApp.LoadLocalizedString( IDS_WATERCHAR_CHANGEOVERTITLE );
	SetWindowText( str );

	str = TASApp.LoadLocalizedString( IDS_DLGWATERCHAR_SUPPLYTEMP );

	GetDlgItem( IDC_STATICTEMP_C )->SetWindowText( str );
	GetDlgItem( IDC_STATICTEMP_H )->SetWindowText( str );

	str = TASApp.LoadLocalizedString( IDS_DLGWATERCHAR_DEFAULTDT );
	GetDlgItem( IDC_STATICDT_C )->SetWindowText( str );
	GetDlgItem( IDC_STATICDT_H )->SetWindowText( str );

	str = TASApp.LoadLocalizedString( IDS_DLGWATERCHAR_STATICADDITFAM );
	GetDlgItem( IDC_STATICADDITFAM )->SetWindowText( str );

	str = TASApp.LoadLocalizedString( IDS_DLGWATERCHAR_STATICADDITNAME );
	GetDlgItem( IDC_STATICADDITNAME )->SetWindowText( str );

	str = TASApp.LoadLocalizedString( IDS_DLGWATERCHAR_STATICPCWEIGHT );
	GetDlgItem( IDC_STATICPCWEIGHT )->SetWindowText( str );

	str = TASApp.LoadLocalizedString( IDS_DLGWATERCHAR_STATICDENSITY );
	GetDlgItem( IDC_STATICDENSITY_C )->SetWindowText( str );
	GetDlgItem( IDC_STATICDENSITY_H )->SetWindowText( str );

	str = TASApp.LoadLocalizedString( IDS_DLGWATERCHAR_STATICKINVISC );
	GetDlgItem( IDC_STATICKINVISC_C )->SetWindowText( str );
	GetDlgItem( IDC_STATICKINVISC_H )->SetWindowText( str );

	str = TASApp.LoadLocalizedString( IDS_DLGWATERCHAR_STATICSPECIFHEAT );
	GetDlgItem( IDC_STATICSPECIFHEAT_C )->SetWindowText( str );
	GetDlgItem( IDC_STATICSPECIFHEAT_H )->SetWindowText( str );

	str = TASApp.LoadLocalizedString( IDS_DLGWATERCHAR_STATICFREEZ );
	GetDlgItem( IDC_STATICFREEZ_C )->SetWindowText( str );
	GetDlgItem( IDC_STATICFREEZ_H )->SetWindowText( str );

	str = TASApp.LoadLocalizedString( IDS_OK );
	GetDlgItem( IDOK )->SetWindowText( str );

	str = TASApp.LoadLocalizedString( IDS_CANCEL );
	GetDlgItem( IDCANCEL )->SetWindowText( str );
	str.Empty();

	// Set font for static IDC_STATICWARNING.
	str = TASApp.LoadLocalizedString( IDS_WATERCHAR_WARNING );
	m_StaticCoolingWarning.SetWindowText( str );
	m_StaticCoolingWarning.SetBlockCursorChange( true );
	m_StaticCoolingWarning.SetBlockSelection( true );
	m_StaticCoolingWarning.SetTextColor( ::GetSysColor( COLOR_GRAYTEXT ) );
	m_StaticCoolingWarning.SetBackColor( ::GetSysColor( COLOR_3DFACE ) );
	
	str = TASApp.LoadLocalizedString( IDS_WATERCHAR_WARNING );
	m_StaticHeatingWarning.SetWindowText( str );
	m_StaticHeatingWarning.SetBlockCursorChange( true );
	m_StaticHeatingWarning.SetBlockSelection( true );
	m_StaticHeatingWarning.SetTextColor( ::GetSysColor( COLOR_GRAYTEXT ) );
	m_StaticHeatingWarning.SetBackColor( ::GetSysColor( COLOR_3DFACE ) );

	// Set caption in the title bar of the dialog.
	if( NULL != m_ptstrCaption )
	{
		SetWindowText( m_ptstrCaption );
	}

	// Add icons to PT, Addit and Dens groups.
	CImageList *pclImgListGroupBox = TASApp.GetpRCImageManager()->GetImageList( CRCImageManager::ILN_GroupBox );
	ASSERT( NULL != pclImgListGroupBox );

	if( NULL != pclImgListGroupBox )
	{
		m_GroupCooling.SetImageList( pclImgListGroupBox, CRCImageManager::ILGBG_Cooling );
		m_GroupHeating.SetImageList( pclImgListGroupBox, CRCImageManager::ILGBG_Heating );
		m_GroupAddit.SetImageList( pclImgListGroupBox, CRCImageManager::ILGB_H2OAddit );
	}

	// Load bitmaps for water phase.
	m_BitmapCoolingLiquid.LoadMappedBitmap( IDB_H2OLIQUID );
	m_BitmapCoolingIce.LoadMappedBitmap( IDB_H2OICE );
	m_BitmapCoolingUnknown.LoadMappedBitmap( IDB_H2OX );
	m_BitmapHeatingLiquid.LoadMappedBitmap( IDB_H2OLIQUID );
	m_BitmapHeatingIce.LoadMappedBitmap( IDB_H2OICE );
	m_BitmapHeatingUnknown.LoadMappedBitmap( IDB_H2OX );

	// Load the appropriate water characteristic depending on the 'm_eOrigin'.
	CDS_WaterCharacteristic *pWC = NULL;

	if( WMUserWaterCharWParam::WM_UWC_WP_ForProductSel == ( m_eOrigin & WMUserWaterCharWParam::WM_UWC_WP_ForProductSel ) )
	{
		pWC = m_pTADS->GetpWCForProductSelection();
	}
	else if( WMUserWaterCharWParam::WM_UWC_WP_ForProject == ( m_eOrigin & WMUserWaterCharWParam::WM_UWC_WP_ForProject ) )
	{
		pWC = m_pTADS->GetpWCForProject();
	}
	pWC->GetpWCData()->SetIsForChangeOver( true );
	m_AdditFamID = pWC->GetAdditFamID();
	m_AdditID = pWC->GetAdditID();
	m_dPCWeight = pWC->GetPcWeight();
	m_dCoolingTemp = pWC->GetTemp();
	m_dCoolingDensity = pWC->GetDens();
	m_dCoolingKinVisc = pWC->GetKinVisc();
	m_dCoolingSpecifHeat = pWC->GetSpecifHeat();


	CDS_TechnicalParameter *pTech = m_pTADS->GetpTechParams();

	if( WMUserWaterCharWParam::WM_UWC_WP_ForProductSel == ( m_eOrigin & WMUserWaterCharWParam::WM_UWC_WP_ForProductSel ) )
	{
		m_dDefaultCoolingDT = pTech->GetDefaultISChangeOverDT( CoolingSide );

		m_dDefaultHeatingDT = pTech->GetDefaultISChangeOverDT( HeatingSide );
		m_PrjType = pTech->GetProductSelectionApplicationType();
		m_dHeatingTemp = pTech->GetDefaultISChangeOverTps( HeatingSide );
		pWC->GetpWCData()->GetSecondWaterChar()->SetAdditFamID( m_AdditFamID );
		pWC->GetpWCData()->GetSecondWaterChar()->SetAdditID( m_AdditID );
		pWC->GetpWCData()->GetSecondWaterChar()->SetPcWeight( m_dPCWeight );
		pWC->GetpWCData()->GetSecondWaterChar()->SetTemp( m_dHeatingTemp );
		pWC->GetpWCData()->GetSecondWaterChar()->UpdateFluidData( m_dHeatingTemp );
		m_dHeatingDensity = pWC->GetpWCData()->GetSecondWaterChar()->GetDens();
		m_dHeatingKinVisc = pWC->GetpWCData()->GetSecondWaterChar()->GetKinVisc();
		m_dHeatingSpecifHeat = pWC->GetpWCData()->GetSecondWaterChar()->GetSpecifHeat();
	}
	else if( WMUserWaterCharWParam::WM_UWC_WP_ForProject == ( m_eOrigin & WMUserWaterCharWParam::WM_UWC_WP_ForProject ) )
	{
		//m_dDefaultCoolingDT = pTech->GetDefaultPrjDT();
		//m_PrjType = pTech->GetProjectType();
	}

	// Make a copy to check if there is a change made by user.
	m_clWCChangeOverCopy = *pWC->GetpWCData();

	// Fill all unit static controls.
	OnUnitChange();

	// Initialize the temperature, pressure, % weight, density, kin. visc. and specific heat edit boxes.
	m_EditCoolingTemp.SetPhysicalType( _U_TEMPERATURE );
	m_EditCoolingTemp.SetMinDblValue( -273.15 );
	m_EditCoolingTemp.SetEditSign( CNumString/*::eEditSign*/::eBoth );
	m_EditCoolingTemp.SetCurrentValSI( m_dCoolingTemp );
	m_EditCoolingTemp.Update();
	m_EditHeatingTemp.SetPhysicalType( _U_TEMPERATURE );
	m_EditHeatingTemp.SetMinDblValue( -273.15 );
	m_EditHeatingTemp.SetEditSign( CNumString/*::eEditSign*/::eBoth );
	m_EditHeatingTemp.SetCurrentValSI( m_dHeatingTemp );
	m_EditHeatingTemp.Update();

	m_EditPcWeight.SetPhysicalType( _U_PERCENT );
	m_EditPcWeight.SetMinDblValue( 0 );
	m_EditPcWeight.SetEditSign( CNumString/*::eEditSign*/::ePositive );
	m_EditPcWeight.SetCurrentValSI( m_dPCWeight );
	m_EditPcWeight.Update();

	m_EditCoolingDens.SetPhysicalType( _U_DENSITY );
	m_EditCoolingDens.SetMinDblValue( 0 );
	m_EditCoolingDens.SetEditSign( CNumString/*::eEditSign*/::ePositive );
	m_EditCoolingDens.SetCurrentValSI( m_dCoolingDensity );
	m_EditCoolingDens.Update();
	m_EditHeatingDens.SetPhysicalType( _U_DENSITY );
	m_EditHeatingDens.SetMinDblValue( 0 );
	m_EditHeatingDens.SetEditSign( CNumString/*::eEditSign*/::ePositive );
	m_EditHeatingDens.SetCurrentValSI( m_dHeatingDensity );
	m_EditHeatingDens.Update();

	m_EditCoolingKinVisc.SetPhysicalType( _U_KINVISCOSITY );
	m_EditCoolingKinVisc.SetMinDblValue( 0 );
	m_EditCoolingKinVisc.SetEditSign( CNumString/*::eEditSign*/::ePositive );
	m_EditCoolingKinVisc.SetCurrentValSI( m_dCoolingKinVisc );
	m_EditCoolingKinVisc.Update();
	m_EditHeatingKinVisc.SetPhysicalType( _U_KINVISCOSITY );
	m_EditHeatingKinVisc.SetMinDblValue( 0 );
	m_EditHeatingKinVisc.SetEditSign( CNumString/*::eEditSign*/::ePositive );
	m_EditHeatingKinVisc.SetCurrentValSI( m_dHeatingKinVisc );
	m_EditHeatingKinVisc.Update();

	m_EditCoolingSpecifHeat.SetPhysicalType( _U_SPECIFHEAT );
	m_EditCoolingSpecifHeat.SetMinDblValue( 0 );
	m_EditCoolingSpecifHeat.SetEditSign( CNumString/*::eEditSign*/::ePositive );
	m_EditCoolingSpecifHeat.SetCurrentValSI( m_dCoolingSpecifHeat );
	m_EditCoolingSpecifHeat.Update();
	m_EditHeatingSpecifHeat.SetPhysicalType( _U_SPECIFHEAT );
	m_EditHeatingSpecifHeat.SetMinDblValue( 0 );
	m_EditHeatingSpecifHeat.SetEditSign( CNumString/*::eEditSign*/::ePositive );
	m_EditHeatingSpecifHeat.SetCurrentValSI( m_dHeatingSpecifHeat );
	m_EditHeatingSpecifHeat.Update();

	m_EditDefaultCoolingDT.SetPhysicalType( _U_DIFFTEMP );
	m_EditDefaultCoolingDT.SetMinDblValue( 0 );
	m_EditDefaultCoolingDT.SetEditSign( CNumString/*::eEditSign*/::ePositive );
	m_EditDefaultCoolingDT.SetCurrentValSI( m_dDefaultCoolingDT );
	m_EditDefaultCoolingDT.Update();
	m_EditDefaultHeatingDT.SetPhysicalType( _U_DIFFTEMP );
	m_EditDefaultHeatingDT.SetMinDblValue( 0 );
	m_EditDefaultHeatingDT.SetEditSign( CNumString/*::eEditSign*/::ePositive );
	m_EditDefaultHeatingDT.SetCurrentValSI( m_dDefaultHeatingDT );
	m_EditDefaultHeatingDT.Update();

	m_EditCoolingFreezPoint.SetBlockSelection( true );
	m_EditCoolingFreezPoint.SetBlockCursorChange( true );
	m_EditCoolingFreezPoint.SetTextColor( ::GetSysColor( COLOR_GRAYTEXT ) );
	m_EditCoolingFreezPoint.SetBackColor( ::GetSysColor( COLOR_3DFACE ) );
	m_EditHeatingFreezPoint.SetBlockSelection( true );
	m_EditHeatingFreezPoint.SetBlockCursorChange( true );
	m_EditHeatingFreezPoint.SetTextColor( ::GetSysColor( COLOR_GRAYTEXT ) );
	m_EditHeatingFreezPoint.SetBackColor( ::GetSysColor( COLOR_3DFACE ) );

	// Fill the Additive combo boxes.
	CString pFam = ( (CTable *)m_pTADB->Get( m_AdditFamID ).MP )->GetName();
	CString pAddit = ( (CDB_StringID *)m_pTADB->Get( m_AdditID ).MP )->GetString();
	_FillComboAdditFam( (LPCTSTR)pFam );
	_FillComboAdditName( pAddit );
	OnCbnSelChangeAdditiveName();

	CImageList *pclImgListButton = TASApp.GetpRCImageManager()->GetImageList( CRCImageManager::ILN_Button );
	ASSERT( NULL != pclImgListButton );

	if( NULL != pclImgListButton )
	{
		m_clButtonRecommendation.SetIcon( (HICON)pclImgListButton->ExtractIcon( CRCImageManager::ILB_AdditRecomended ) );
	}

	// Create a ToolTipCtrl and add a tool tip to the Recommendation button.
	m_ToolTip.Create( this, TTS_NOPREFIX );

	// Following line to display a long tooltip string in several 'short' lines
	//	::SendMessage(m_ToolTip.GetSafeHwnd(),TTM_SETMAXTIPWIDTH,0,200);// increase height
	CString ttstr;
	ttstr = TASApp.LoadLocalizedString( IDS_WATERCHAR_WARNINGTT );
	m_ToolTip.AddToolWindow( &m_clButtonRecommendation, ttstr );

	return TRUE;
}

BOOL CDlgCOWaterChar::PreTranslateMessage( MSG *pMsg )
{
	// Why not use 'WM_KEYDOWN' in the message map? Because if edit control has the focus in the dialog, WM_KEYDOWN is sent to this control
	// and not to this dialog.
	if( WM_KEYDOWN == pMsg->message && VK_ESCAPE == pMsg->wParam )
	{
		EndDialog( IDCANCEL );
	}

	return CDialogEx::PreTranslateMessage( pMsg );
}

void CDlgCOWaterChar::_DisplayWarningWithID( bool bActivate, eGroupApplication eGroup, CExtNumEdit *pExtEdit, UINT IDMsg, double dValue, int iUnit )
{
	CString strMsg = _T( "" );

	if( NULL == pExtEdit || false == bActivate )
	{
		// Restore default.
		strMsg = TASApp.LoadLocalizedString( IDS_WATERCHAR_WARNING );
	}
	else
	{
		if( iUnit != -1 )
		{
			FormatString( strMsg, IDMsg, WriteCUDouble( iUnit, dValue, true ) );
		}
		else
		{
			strMsg = TASApp.LoadLocalizedString( IDMsg );
		}
	}

	_DisplayWarningWithString( bActivate, eGroup, pExtEdit, strMsg, dValue, iUnit );
}

void CDlgCOWaterChar::_DisplayWarningWithString( bool bActivate, eGroupApplication eGroup, CExtNumEdit *pExtEdit, CString strMsg, double dValue, int iUnit )
{
	switch ( eGroup )
	{
		case eCooling:
			if( NULL == pExtEdit || false == bActivate )
			{
				// Restore default.
				m_StaticCoolingWarning.SetWindowText( strMsg );
				m_StaticCoolingWarning.SetTextColor( _BLACK );

			}
			else
			{
				m_StaticCoolingWarning.SetWindowText( strMsg );
				m_StaticCoolingWarning.SetTextColor( _RED );
			}
			break;

		case eHeating:
			if( NULL == pExtEdit || false == bActivate )
			{
				// Restore default.
				m_StaticHeatingWarning.SetWindowText( strMsg );
				m_StaticHeatingWarning.SetTextColor( _BLACK );
			}
			else
			{
				m_StaticHeatingWarning.SetWindowText( strMsg );
				m_StaticHeatingWarning.SetTextColor( _RED );
			}
			break;
	}

	if( NULL == pExtEdit || false == bActivate )
	{
		m_clButtonOK.EnableWindow( TRUE );
		m_clButtonOK.ModifyStyle( BS_OWNERDRAW, 0 );
		m_clButtonOK.SetTextColor( _BLACK );
		m_clButtonOK.ResetDrawBorder();
	}
	else
	{
		if( NULL != pExtEdit && iUnit != -1 )
		{
			pExtEdit->SetDrawBorder( true, _RED );
		}

		m_clButtonOK.EnableWindow( FALSE );
		m_clButtonOK.ModifyStyle( 0, BS_OWNERDRAW );
		m_clButtonOK.SetTextColor( _RED );
		m_clButtonOK.SetBackColor( RGB( 204, 204, 204 ) );
		m_clButtonOK.SetDrawBorder( true, _RED );
	}
}

void CDlgCOWaterChar::OnOK()
{
	PREVENT_ENTER_KEY
	
	m_EditDefaultCoolingDT.ResetDrawBorder();
	m_EditDefaultHeatingDT.ResetDrawBorder();
	m_EditCoolingTemp.ResetDrawBorder();
	m_EditHeatingTemp.ResetDrawBorder();

	_DisplayWarningWithID( false, eCooling );
	_DisplayWarningWithID( false, eHeating );

	// Check valid data and not ice.
	if( _T("OTHER_ADDIT") != m_AdditFamID )
	{
		if( m_dCoolingTemp <= m_dCoolingTempfreez )
		{
			_DisplayWarningWithID( true, eCooling, &m_EditCoolingTemp, AFXMSG_BELOW_TFREEZ, m_dCoolingTempfreez, _U_TEMPERATURE );
			m_EditCoolingTemp.SetFocus();
			return;
		}
		if( m_dHeatingTemp <= m_dHeatingTempfreez )
		{
			_DisplayWarningWithID( true, eHeating, &m_EditHeatingTemp, AFXMSG_BELOW_TFREEZ, m_dHeatingTempfreez, _U_TEMPERATURE );
			m_EditHeatingTemp.SetFocus();
			return;
		}

		// Verify default DT.
		if( ( m_dHeatingTemp - m_dDefaultHeatingDT ) <= m_dHeatingTempfreez )
		{
			_DisplayWarningWithID( true, eHeating, &m_EditDefaultHeatingDT, AFXMSG_DTBELOW_TFREEZ, m_dHeatingTempfreez, _U_TEMPERATURE );
			m_EditDefaultHeatingDT.SetFocus();
			return;
		}
	}


	// Transfer water characteristics to appropriate place depending on the 'm_eOrigin'.
	CWaterChar *pWC = NULL;
	if( NULL != m_pclProductSelParams )
	{
		pWC = &m_pclProductSelParams->m_WC;
	}
	else if( WMUserWaterCharWParam::WM_UWC_WP_ForProductSel == ( m_eOrigin & WMUserWaterCharWParam::WM_UWC_WP_ForProductSel ) )
	{
		pWC = m_pTADS->GetpWCForProductSelection()->GetpWCData();
	}
	else if( WMUserWaterCharWParam::WM_UWC_WP_ForProject == ( m_eOrigin & WMUserWaterCharWParam::WM_UWC_WP_ForProject ) )
	{
		pWC = m_pTADS->GetpWCForProject()->GetpWCData();
	}

	pWC->SetAdditFamID( m_AdditFamID );
	pWC->SetAdditID( m_AdditID );
	pWC->SetTemp( m_dCoolingTemp );
	pWC->SetTfreez( m_dCoolingTempfreez );
	pWC->SetPcWeight( m_dPCWeight );
	pWC->SetDens( m_dCoolingDensity );
	pWC->SetKinVisc( m_dCoolingKinVisc );
	pWC->SetSpecifHeat( m_dCoolingSpecifHeat );

	pWC->GetSecondWaterChar()->SetAdditFamID( m_AdditFamID );
	pWC->GetSecondWaterChar()->SetAdditID( m_AdditID );
	pWC->GetSecondWaterChar()->SetPcWeight( m_dPCWeight );
	pWC->GetSecondWaterChar()->SetTemp( m_dHeatingTemp );
	pWC->GetSecondWaterChar()->UpdateFluidData( m_dHeatingTemp );
	pWC->GetSecondWaterChar()->SetTfreez( m_dHeatingTempfreez );
	pWC->GetSecondWaterChar()->SetDens( m_dHeatingDensity );
	pWC->GetSecondWaterChar()->SetKinVisc( m_dHeatingKinVisc );
	pWC->GetSecondWaterChar()->SetSpecifHeat( m_dHeatingSpecifHeat );

	bool bDTCoolingChanged = false;
	bool bDTHeatingChanged = false;

	CDS_TechnicalParameter *pTechParam = m_pTADS->GetpTechParams();

	if( WMUserWaterCharWParam::WM_UWC_WP_ForProductSel == ( m_eOrigin & WMUserWaterCharWParam::WM_UWC_WP_ForProductSel ) )
	{
		pTechParam->SetDefaultISChangeOverTps( m_dCoolingTemp, CoolingSide );
		pTechParam->SetDefaultISChangeOverAdditID( pWC->GetAdditID(), CoolingSide );
		pTechParam->SetDefaultISChangeOverPcWeight( m_dPCWeight, CoolingSide );
		if( pTechParam->GetDefaultISChangeOverDT( CoolingSide ) != m_dDefaultCoolingDT )
		{
			bDTCoolingChanged = true;
		}
		pTechParam->SetDefaultISChangeOverDT( m_dDefaultCoolingDT, CoolingSide );

		pTechParam->SetDefaultISChangeOverTps( m_dHeatingTemp, HeatingSide );
		pTechParam->SetDefaultISChangeOverAdditID( pWC->GetAdditID(), HeatingSide );
		if( pTechParam->GetDefaultISChangeOverDT( HeatingSide ) != m_dDefaultHeatingDT )
		{
			bDTHeatingChanged = true;
		}
		pTechParam->SetDefaultISChangeOverDT( m_dDefaultHeatingDT, HeatingSide );
		pTechParam->SetDefaultISChangeOverPcWeight( m_dPCWeight, HeatingSide );
	}
	else if( WMUserWaterCharWParam::WM_UWC_WP_ForProject == ( m_eOrigin & WMUserWaterCharWParam::WM_UWC_WP_ForProject ) )
	{
		//pTechParam->SetDefaultPrjTps( m_dTemp );
		//pTechParam->SetDefaultPrjDT( m_dDefaultDT );
		//pTechParam->SetDefaultPrjAddit( pWC->GetAdditID() );
		//pTechParam->SetDefaultPrjPcWeight( m_dPCWeight );
	}

	// Check if user has made change.
	LPARAM lpParam;
	if( true == m_clWCChangeOverCopy.Compare( *pWC, true ) )
	{
		lpParam = WMUserWaterCharLParam::WM_UWC_LWP_NoChangeForChangeOver;
	}
	else
	{
		lpParam = WMUserWaterCharLParam::WM_UWC_LWP_ChangeForChangeOver;
	}
	
	if( WMUserWaterCharLParam::WM_UWC_LWP_NoChangeForChangeOver != lpParam || true == bDTCoolingChanged || true == bDTHeatingChanged )
	{
		lpParam = WMUserWaterCharLParam::WM_UWC_LWP_ChangeForChangeOver; // For true == bDTCoolingChanged || true == bDTHeatingChanged
		// Send message to inform about modification of water characteristics.
		::AfxGetApp()->m_pMainWnd->SendMessage( WM_USER_WATERCHANGE, (WPARAM)m_eOrigin, lpParam );
		::AfxGetApp()->m_pMainWnd->SendMessageToDescendants( WM_USER_WATERCHANGE, (WPARAM)m_eOrigin, lpParam );
	}

	CDialogEx::OnOK();
}

void CDlgCOWaterChar::OnDestroy()
{
	CDialogEx::OnDestroy();

	//m_font.DeleteObject();
}

void CDlgCOWaterChar::OnChangeEditCoolingTemperature()
{
	if( GetFocus() != &m_EditCoolingTemp )
	{
		return;
	}

	switch( ReadDouble( m_EditCoolingTemp, &m_dCoolingTemp ) )
	{
		case RD_EMPTY:
			// Update fields to unknown state.
			SetUnknownState( eCooling );
			break;

		case RD_NOT_NUMBER:
			break;

		case RD_OK:
			m_dCoolingTemp = CDimValue::CUtoSI( _U_TEMPERATURE, m_dCoolingTemp );
			UpdateAll();
			break;
	}
}

void CDlgCOWaterChar::OnChangeEditHeatingTemperature()
{
	if( GetFocus() != &m_EditHeatingTemp )
	{
		return;
	}

	switch( ReadDouble( m_EditHeatingTemp, &m_dHeatingTemp ) )
	{
		case RD_EMPTY:
			// Update fields to unknown state.
			SetUnknownState( eHeating );
			break;

		case RD_NOT_NUMBER:
			break;

		case RD_OK:
			m_dHeatingTemp = CDimValue::CUtoSI( _U_TEMPERATURE, m_dHeatingTemp );
			UpdateAll();
			break;
	}
}

void CDlgCOWaterChar::OnChangeEditCoolingDT()
{
	if( GetFocus() != &m_EditDefaultCoolingDT )
	{
		return;
	}

	switch( ReadDouble( m_EditDefaultCoolingDT, &m_dDefaultCoolingDT ) )
	{
		case RD_EMPTY:
			break;

		case RD_NOT_NUMBER:
			break;

		case RD_OK:
			m_dDefaultCoolingDT = CDimValue::CUtoSI( _U_DIFFTEMP, m_dDefaultCoolingDT );
			UpdateAll();
			break;
	}
}

void CDlgCOWaterChar::OnChangeEditHeatingDT()
{
	if( GetFocus() != &m_EditDefaultHeatingDT )
	{
		return;
	}

	switch( ReadDouble( m_EditDefaultHeatingDT, &m_dDefaultHeatingDT ) )
	{
		case RD_EMPTY:
			break;

		case RD_NOT_NUMBER:
			break;

		case RD_OK:
			m_dDefaultHeatingDT = CDimValue::CUtoSI( _U_DIFFTEMP, m_dDefaultHeatingDT );
			UpdateAll();
			break;
	}
}

void CDlgCOWaterChar::OnButtonRecommendation()
{
	// Look at Disabling the bell in the MSDN help for disabling the bip when the AfxMessageBox is displayed.
	TASApp.DisplayLocalizeMessageBox( IDS_WATERCHAR_WARNINGPH, MB_OK | MB_ICONINFORMATION );
}

void CDlgCOWaterChar::OnChangeEditPCWeight()
{
	if( GetFocus() != &m_EditPcWeight )
	{
		return;
	}

	switch( ReadDouble( m_EditPcWeight, &m_dPCWeight ) )
	{
		case RD_EMPTY:
			SetUnknownState( eCooling );
			SetUnknownState( eHeating );
			break;

		case RD_NOT_NUMBER:
			break;

		case RD_OK:
			// Update all fields.
			UpdateAll();
			break;
	}
}

void CDlgCOWaterChar::OnEditEnterCoolingTemperature( NMHDR *pNMHDR, LRESULT *pResult )
{
	if( IDC_EDITTEMP_C == pNMHDR->idFrom )
	{
		GetDlgItem( IDOK )->SetFocus();
		OnOK();
	}
}

void CDlgCOWaterChar::OnEditEnterHeatingTemperature( NMHDR *pNMHDR, LRESULT *pResult )
{
	if( IDC_EDITTEMP_H == pNMHDR->idFrom )
	{
		GetDlgItem( IDOK )->SetFocus();
		OnOK();
	}
}

void CDlgCOWaterChar::OnEnKillFocusCoolingTemperature()
{
	PREVENT_NOT_CANCEL_WITH_EMPTY_BOX

		double dValue = m_dCoolingTemp;

	switch( ReadDouble( m_EditCoolingTemp, &dValue ) )
	{
		case RD_EMPTY:
			// 			m_EditTemp.SetFocus();
			// 			TASApp.DisplayLocalizeMessageBox( AFXMSG_NON_NUMERICAL_VALUE );
			break;

		case RD_OK:
			dValue = CDimValue::CUtoSI( _U_TEMPERATURE, dValue );

			if( dValue < m_dCoolingTempfreez && m_AdditFamID != _T("OTHER_ADDIT") )
			{
				// DO NOTHING
			}
			else
			{
				m_EditCoolingTemp.SetCurrentValSI( m_dCoolingTemp );
				m_EditCoolingTemp.Update();
			}

			break;
	}
}

void CDlgCOWaterChar::OnEnKillFocusHeatingTemperature()
{
	PREVENT_NOT_CANCEL_WITH_EMPTY_BOX

		double dValue = m_dHeatingTemp;

	switch( ReadDouble( m_EditHeatingTemp, &dValue ) )
	{
		case RD_EMPTY:
			// 			m_EditTemp.SetFocus();
			// 			TASApp.DisplayLocalizeMessageBox( AFXMSG_NON_NUMERICAL_VALUE );
			break;

		case RD_OK:
			dValue = CDimValue::CUtoSI( _U_TEMPERATURE, dValue );

			if( dValue < m_dHeatingTempfreez && m_AdditFamID != _T("OTHER_ADDIT") )
			{
				// DO NOTHING
			}
			else
			{
				m_EditHeatingTemp.SetCurrentValSI( m_dHeatingTemp );
				m_EditHeatingTemp.Update();

				// Verify default DT.
				//if( Cooling != m_PrjType && m_AdditFamID != _T("OTHER_ADDIT") )
				{
					if( ( m_dHeatingTemp - m_dDefaultHeatingDT ) < m_dHeatingTempfreez )
					{
						m_dDefaultHeatingDT = 0;
						m_EditDefaultHeatingDT.SetWindowText( _T( "" ) );
					}
				}
			}

			break;
	}
}

void CDlgCOWaterChar::OnEnKillFocusCoolingDT()
{
	PREVENT_NOT_CANCEL_WITH_EMPTY_BOX

		if( GetFocus() == &m_EditDefaultCoolingDT )
		{
			return;
		}

	m_EditDefaultCoolingDT.SetWindowText( WriteCUDouble( _U_DIFFTEMP, m_dDefaultCoolingDT ) );
}

void CDlgCOWaterChar::OnEnKillFocusHeatingDT()
{
	PREVENT_NOT_CANCEL_WITH_EMPTY_BOX

		if( GetFocus() == &m_EditDefaultHeatingDT )
		{
			return;
		}

	m_EditDefaultHeatingDT.SetWindowText( WriteCUDouble( _U_DIFFTEMP, m_dDefaultHeatingDT ) );
}

void CDlgCOWaterChar::OnEnKillFocusPCWeight()
{
	PREVENT_NOT_CANCEL_WITH_EMPTY_BOX

		m_EditPcWeight.SetWindowText( WriteCUDouble( _U_PERCENT, m_dPCWeight ) );
}

LRESULT CDlgCOWaterChar::OnUnitChange( WPARAM wParam, LPARAM lParam )
{

	TCHAR tcName[_MAXCHARS];
	CUnitDatabase *pUnitDB = CDimValue::AccessUDB();

	// Set the text for the temperature units.
	GetNameOf( pUnitDB->GetDefaultUnit( _U_TEMPERATURE ), tcName );
	SetDlgItemText( IDC_STATICTEMPUNIT_C, tcName );
	SetDlgItemText( IDC_STATICTEMPUNIT_H, tcName );
	SetDlgItemText( IDC_STATICFREEZUNIT_C, tcName );
	SetDlgItemText( IDC_STATICFREEZUNIT_H, tcName );

	GetNameOf( pUnitDB->GetDefaultUnit( _U_DIFFTEMP ), tcName );
	SetDlgItemText( IDC_STATICUNITDT_C, tcName );
	SetDlgItemText( IDC_STATICUNITDT_H, tcName );

	// Set the text for the density units.
	GetNameOf( pUnitDB->GetDefaultUnit( _U_DENSITY ), tcName );
	SetDlgItemText( IDC_STATICDENSUNIT_C, tcName );
	SetDlgItemText( IDC_STATICDENSUNIT_H, tcName );

	// Set the text for the kinematic viscosity units.
	GetNameOf( pUnitDB->GetDefaultUnit( _U_KINVISCOSITY ), tcName );
	SetDlgItemText( IDC_STATICKINVISCUNIT_C, tcName );
	SetDlgItemText( IDC_STATICKINVISCUNIT_H, tcName );

	// Set the text for the specific heat units.
	GetNameOf( pUnitDB->GetDefaultUnit( _U_SPECIFHEAT ), tcName );
	SetDlgItemText( IDC_STATICSPECIFHEATUNIT_C, tcName );
	SetDlgItemText( IDC_STATICSPECIFHEATUNIT_C, tcName );

	return 0;
}

void CDlgCOWaterChar::SetUnknownState( eGroupApplication eGroup )
{
	// Load bitmap for unknown state and clean the edit boxes.
	switch( eGroup )
	{
		case eCooling:
			UpdateWaterBitmap( WP_UNKNOWN, eCooling );
			m_EditCoolingDens.SetWindowText( _T( "" ) );
			m_EditCoolingSpecifHeat.SetWindowText( _T( "" ) );
			m_EditCoolingKinVisc.SetWindowText( _T( "" ) );
			break;

		case eHeating:
			UpdateWaterBitmap( WP_UNKNOWN, eHeating );
			m_EditHeatingDens.SetWindowText( _T( "" ) );
			m_EditHeatingSpecifHeat.SetWindowText( _T( "" ) );
			m_EditHeatingKinVisc.SetWindowText( _T( "" ) );
			break;
	}
}

void CDlgCOWaterChar::UpdateWaterBitmap( enum WaterPhase_enum phase, eGroupApplication eGroup )
{
	HBITMAP hBitmap;

	switch( phase )
	{
		case WP_LIQUID:
			if( eCooling == eGroup )
			{
				hBitmap = (HBITMAP)m_BitmapCoolingLiquid.GetSafeHandle();
				( (CStatic *)GetDlgItem( IDC_STATICWATERPHASE_C ) )->SetBitmap( hBitmap );
			}
			else
			{
				hBitmap = (HBITMAP)m_BitmapHeatingLiquid.GetSafeHandle();
				( (CStatic *)GetDlgItem( IDC_STATICWATERPHASE_H ) )->SetBitmap( hBitmap );
			}
			break;

		case WP_ICE:
			if( eCooling == eGroup )
			{
				hBitmap = (HBITMAP)m_BitmapCoolingIce.GetSafeHandle();
				( (CStatic *)GetDlgItem( IDC_STATICWATERPHASE_C ) )->SetBitmap( hBitmap );
			}
			else
			{
				hBitmap = (HBITMAP)m_BitmapHeatingIce.GetSafeHandle();
				( (CStatic *)GetDlgItem( IDC_STATICWATERPHASE_H ) )->SetBitmap( hBitmap );
			}
			break;

		case WP_UNKNOWN:
			if( eCooling == eGroup )
			{
				hBitmap = (HBITMAP)m_BitmapCoolingUnknown.GetSafeHandle();
				( (CStatic *)GetDlgItem( IDC_STATICWATERPHASE_C ) )->SetBitmap( hBitmap );
			}
			else
			{
				hBitmap = (HBITMAP)m_BitmapHeatingUnknown.GetSafeHandle();
				( (CStatic *)GetDlgItem( IDC_STATICWATERPHASE_H ) )->SetBitmap( hBitmap );
			}
			break;

		default:
			ASSERT( TRUE );
			break;
	}
}

void CDlgCOWaterChar::UpdateAllCooling()
{
	// Check if the phase is liquid or solid and update accordingly.
	if( m_dCoolingTemp <= m_dCoolingTempfreez )
	{
		// Solid phase: ice.
		UpdateWaterBitmap( WP_ICE, eCooling );
		m_EditCoolingDens.SetWindowTextW( _T( "" ) );
		m_EditCoolingSpecifHeat.SetWindowTextW( _T( "" ) );
		m_EditCoolingKinVisc.SetWindowTextW( _T( "" ) );
		_DisplayWarningWithID( true, eCooling, &m_EditCoolingTemp, AFXMSG_BELOW_TFREEZ, m_dCoolingTempfreez, _U_TEMPERATURE );
		return;
	}
	else
	{
		m_EditCoolingTemp.ResetDrawBorder();
		_DisplayWarningWithID( false, eCooling );
	}

	// Verify default DT.
	double dMaxTemp = m_pAdditChar->GetMaxT( m_dPCWeight );

	if( ( m_dCoolingTemp + m_dDefaultCoolingDT ) > dMaxTemp )
	{
		_DisplayWarningWithID( true, eCooling, &m_EditDefaultCoolingDT, AFXMSG_DTABOVE_TMAX, dMaxTemp, _U_TEMPERATURE );
		return;
	}
	else
	{
		m_EditDefaultCoolingDT.ResetDrawBorder();
		_DisplayWarningWithID( false, eCooling );
	}

	// Liquid phase.
	UpdateWaterBitmap( WP_LIQUID, eCooling );

	double dVaporPressure = 0;
	enum SurfValReturn_enum SVReturnCooling = m_pAdditChar->GetAdditProp( m_dPCWeight, m_dCoolingTemp, &m_dCoolingDensity, &m_dCoolingKinVisc, &m_dCoolingSpecifHeat, &dVaporPressure );

	switch( SVReturnCooling )
	{
		case SV_OK:
			_DisplayWarningWithID( false, eCooling );
			m_EditPcWeight.ResetDrawBorder();
			m_EditCoolingDens.SetWindowText( WriteDouble( CDimValue::SItoCU( _U_DENSITY, m_dCoolingDensity ), 4, 1 ) );
			m_EditCoolingSpecifHeat.SetWindowText( WriteDouble( CDimValue::SItoCU( _U_SPECIFHEAT, m_dCoolingSpecifHeat ), 4, 1 ) );
			m_EditCoolingKinVisc.SetWindowText( WriteDouble( CDimValue::SItoCU( _U_KINVISCOSITY, m_dCoolingKinVisc ), 4, 1 ) );
			break;

		case SV_OUTOFXRANGE: // Additive percentage above max. value
			{
				// Check concentration min and max
				double dPcMin = m_pAdditChar->GetMinPc();
				double dPcMax = m_pAdditChar->GetMaxPc();

				if( m_dPCWeight < dPcMin )
				{
					_DisplayWarningWithID( true, eCooling, &m_EditPcWeight, AFXMSG_BELOW_MINPCWEIGHT, dPcMin, _U_PERCENT );
				}

				if( m_dPCWeight > dPcMax )
				{
					_DisplayWarningWithID( true, eCooling, &m_EditPcWeight, AFXMSG_ABOVE_MAXPCWEIGHT, dPcMax, _U_PERCENT );
				}

				SetUnknownState( eCooling );
			}
			break;

		case SV_ABOVEYMAX: // Temperature above max. value
			_DisplayWarningWithID( true, eCooling, &m_EditCoolingTemp, AFXMSG_ABOVE_MAXTEMP, m_pAdditChar->GetMaxT( m_dPCWeight ), _U_TEMPERATURE );
			SetUnknownState( eCooling );
			break;

		default:
			SetUnknownState( eCooling );
			break;
	}
}

void CDlgCOWaterChar::UpdateAllHeating()
{
	// Check if the phase is liquid or solid and update accordingly.
	if( m_dHeatingTemp <= m_dHeatingTempfreez )
	{
		// Solid phase: ice.
		UpdateWaterBitmap( WP_ICE, eHeating );
		m_EditHeatingDens.SetWindowTextW( _T( "" ) );
		m_EditHeatingSpecifHeat.SetWindowTextW( _T( "" ) );
		m_EditHeatingKinVisc.SetWindowTextW( _T( "" ) );
		_DisplayWarningWithID( true, eHeating, &m_EditHeatingTemp, AFXMSG_BELOW_TFREEZ, m_dHeatingTempfreez, _U_TEMPERATURE );
		return;
	}
	else
	{
		m_EditHeatingTemp.ResetDrawBorder();
		_DisplayWarningWithID( false, eHeating );
	}


	if( ( m_dHeatingTemp - m_dDefaultHeatingDT ) <= m_dHeatingTempfreez )
	{
		_DisplayWarningWithID( true, eHeating, &m_EditDefaultHeatingDT, AFXMSG_DTBELOW_TFREEZ, m_dHeatingTempfreez, _U_TEMPERATURE );
		return;
	}
	else
	{
		m_EditDefaultHeatingDT.ResetDrawBorder();
		_DisplayWarningWithID( false, eHeating );
	}

	// Liquid phase.
	UpdateWaterBitmap( WP_LIQUID, eHeating );
	double dVaporPressure = 0;

	enum SurfValReturn_enum SVReturnHeating = m_pAdditChar->GetAdditProp( m_dPCWeight, m_dHeatingTemp, &m_dHeatingDensity, &m_dHeatingKinVisc, &m_dHeatingSpecifHeat, &dVaporPressure );

	switch( SVReturnHeating )
	{
		case SV_OK:
			_DisplayWarningWithID( false, eHeating );
			m_EditPcWeight.ResetDrawBorder();
			m_EditHeatingDens.SetWindowText( WriteDouble( CDimValue::SItoCU( _U_DENSITY, m_dHeatingDensity ), 4, 1 ) );
			m_EditHeatingSpecifHeat.SetWindowText( WriteDouble( CDimValue::SItoCU( _U_SPECIFHEAT, m_dHeatingSpecifHeat ), 4, 1 ) );
			m_EditHeatingKinVisc.SetWindowText( WriteDouble( CDimValue::SItoCU( _U_KINVISCOSITY, m_dHeatingKinVisc ), 4, 1 ) );
			break;

		case SV_OUTOFXRANGE: // Additive percentage above max. value
			{
				// Check concentration min and max
				double dPcMin = m_pAdditChar->GetMinPc();
				double dPcMax = m_pAdditChar->GetMaxPc();

				if( m_dPCWeight < dPcMin )
				{
					_DisplayWarningWithID( true, eHeating, &m_EditPcWeight, AFXMSG_BELOW_MINPCWEIGHT, dPcMin, _U_PERCENT );
				}

				if( m_dPCWeight > dPcMax )
				{
					_DisplayWarningWithID( true, eHeating, &m_EditPcWeight, AFXMSG_ABOVE_MAXPCWEIGHT, dPcMax, _U_PERCENT );
				}

				SetUnknownState( eHeating );
			}
			break;

		case SV_ABOVEYMAX: // Temperature above max. value
			_DisplayWarningWithID( true, eHeating, &m_EditHeatingTemp, AFXMSG_ABOVE_MAXTEMP, m_pAdditChar->GetMaxT( m_dPCWeight ), _U_TEMPERATURE );
			SetUnknownState( eHeating );
			break;

		default:
			SetUnknownState( eHeating );
			break;
	}
}

void CDlgCOWaterChar::UpdateAll()
{
	if( NULL != m_pfnVerifyFluidCharacteristics )
	{
		bool bEnable = true;
		CString strMsg = _T( "" );
		m_pfnVerifyFluidCharacteristics->OnVerifyFluidCharacteristics( bEnable, strMsg, this );

		if( false == bEnable )
		{
			_DisplayWarningWithString( true, eCooling, &m_EditCoolingTemp, strMsg );
			_DisplayWarningWithString( true, eHeating, &m_EditHeatingTemp, strMsg );
			return;
		}
	}

	if( NULL == m_pAdditChar )
	{
		return;
	}

	m_dCoolingTempfreez = m_pAdditChar->GetTfreez( m_dPCWeight );
	m_dHeatingTempfreez = m_pAdditChar->GetTfreez( m_dPCWeight );

	// Increase number of decimal of freezing temperature.
	CString strT = WriteDouble( CDimValue::SItoCU( _U_TEMPERATURE, m_dCoolingTempfreez ), 4, 1, 1 );
	m_EditCoolingFreezPoint.SetWindowText( strT );
	m_EditHeatingFreezPoint.SetWindowText( strT );

	UpdateAllCooling();
	UpdateAllHeating();
}

void CDlgCOWaterChar::_FillComboAdditFam( LPCTSTR ptstrFamily )
{
	// Fill the additive family Combo box.
	if( m_ComboAdditFam.GetCount() > 0 )
	{
		m_ComboAdditFam.ResetContent();
	}

	CTable *pTab = (CTable *)( m_pTADB->Get( _T("ADDITIVE_TAB") ).MP );
	ASSERT( NULL != pTab );
	IDPTR IDPtr = _NULL_IDPTR;

	for( IDPtr = pTab->GetFirst( CLASS( CTable ) ); _T( '\0' ) != *IDPtr.ID; IDPtr = pTab->GetNext() )
	{
		// Skip water & 'Aof' to introduce them at the top/bottom when all other fluids have been inserted.
		if( 0 == _tcscmp( IDPtr.ID, _T("WATER_ADDIT" ) ) || 0 == _tcscmp( IDPtr.ID, _T( "OTHER_ADDIT") ) )
		{
			continue;
		}

		if( m_ComboAdditFam.FindStringExact( -1, ( (CTable *)IDPtr.MP )->GetName() ) < 0 )
		{
			m_ComboAdditFam.AddString( ( (CTable *)IDPtr.MP )->GetName() );
		}
	}

	for( IDPtr = pTab->GetFirst( CLASS( CTable ) ); _T( '\0' ) != *IDPtr.ID; IDPtr = pTab->GetNext() )
	{
		if( 0 == _tcscmp( IDPtr.ID, _T("WATER_ADDIT") ) )
		{
			// Introduce water at the top.
			if( m_ComboAdditFam.FindStringExact( -1, ( (CTable *)IDPtr.MP )->GetName() ) < 0 )
			{
				m_ComboAdditFam.InsertString( 0, ( (CTable *)IDPtr.MP )->GetName() );
			}
		}
		//else if( 0 == _tcscmp( IDPtr.ID, _T("OTHER_ADDIT") ) )
		//{
		//	// Introduce 'Any other fluid' at the bottom.
		//	if( m_ComboAdditFam.FindStringExact( -1, ( (CTable *)IDPtr.MP )->GetName() ) < 0 )
		//	{
		//		m_ComboAdditFam.InsertString( -1, ( (CTable *)IDPtr.MP )->GetName() );
		//	}
		//}
	}

	// Set the additive family selection.
	if( NULL != ptstrFamily && CB_ERR != m_ComboAdditFam.FindStringExact( -1, ptstrFamily ) )
	{
		m_ComboAdditFam.SelectString( -1, ptstrFamily );
	}
	else
	{
		m_ComboAdditFam.SetCurSel( ( m_ComboAdditFam.GetCount() > 0 ) ? 0 : -1 );
	}

}

void CDlgCOWaterChar::_FillComboAdditName( CString strAdditName )
{
	m_ComboAdditName.ResetContent();

	if( 0 == m_ComboAdditFam.GetCount() || CB_ERR == m_ComboAdditFam.GetCurSel() )
	{
		return;
	}

	CString str( _T( "" ) );
	m_ComboAdditFam.GetLBText( m_ComboAdditFam.GetCurSel(), str );

	// Find the correct additive family table in table "ADDITIVE_TAB".
	CTable *pTab = (CTable *)( m_pTADB->Get( _T("ADDITIVE_TAB") ).MP );
	ASSERT( NULL != pTab );

	IDPTR IDPtr = pTab->GetFirst( CLASS( CTable ) );
	ASSERT( _T( '\0' ) != *IDPtr.ID );

	while( _T( '\0' ) != *IDPtr.ID && ( (CTable *)IDPtr.MP )->GetName() != str )
	{
		IDPtr = pTab->GetNext();
	}

	ASSERT( _T( '\0' ) != *IDPtr.ID );
	pTab = m_pAdditFamTab = (CTable *)( IDPtr.MP );
	m_AdditFamID = IDPtr.ID;
	CDB_StringID *pSelectedStrID = NULL;

	// Check which additive family is selected and update other dialog controls.
	if( 0 != _tcscmp( IDPtr.ID, _T("OTHER_ADDIT") ) )
	{
		if( 0 == _tcscmp( IDPtr.ID, _T("WATER_ADDIT") ) )
		{
			m_ComboAdditName.ShowWindow( SW_HIDE );
			GetDlgItem( IDC_STATICADDITNAME )->ShowWindow( SW_HIDE );
		}
		else
		{
			GetDlgItem( IDC_STATICADDITNAME )->ShowWindow( SW_SHOW );
			m_ComboAdditName.ShowWindow( SW_SHOW );
		}
		m_EditCoolingDens.SetReadOnly( TRUE );
		m_EditCoolingDens.SetBlockSelection( true );
		m_EditCoolingDens.SetBlockCursorChange( true );
		m_EditCoolingDens.SetTextColor( ::GetSysColor( COLOR_GRAYTEXT ) );
		m_EditCoolingDens.SetBackColor( ::GetSysColor( COLOR_3DFACE ) );
		m_EditHeatingDens.SetReadOnly( TRUE );
		m_EditHeatingDens.SetBlockSelection( true );
		m_EditHeatingDens.SetBlockCursorChange( true );
		m_EditHeatingDens.SetTextColor( ::GetSysColor( COLOR_GRAYTEXT ) );
		m_EditHeatingDens.SetBackColor( ::GetSysColor( COLOR_3DFACE ) );

		m_EditCoolingKinVisc.SetReadOnly( TRUE );
		m_EditCoolingKinVisc.SetBlockSelection( true );
		m_EditCoolingKinVisc.SetBlockCursorChange( true );
		m_EditCoolingKinVisc.SetTextColor( ::GetSysColor( COLOR_GRAYTEXT ) );
		m_EditCoolingKinVisc.SetBackColor( ::GetSysColor( COLOR_3DFACE ) );
		m_EditHeatingKinVisc.SetReadOnly( TRUE );
		m_EditHeatingKinVisc.SetBlockSelection( true );
		m_EditHeatingKinVisc.SetBlockCursorChange( true );
		m_EditHeatingKinVisc.SetTextColor( ::GetSysColor( COLOR_GRAYTEXT ) );
		m_EditHeatingKinVisc.SetBackColor( ::GetSysColor( COLOR_3DFACE ) );

		m_EditCoolingSpecifHeat.SetReadOnly( TRUE );
		m_EditCoolingSpecifHeat.SetBlockSelection( true );
		m_EditCoolingSpecifHeat.SetBlockCursorChange( true );
		m_EditCoolingSpecifHeat.SetTextColor( ::GetSysColor( COLOR_GRAYTEXT ) );
		m_EditCoolingSpecifHeat.SetBackColor( ::GetSysColor( COLOR_3DFACE ) );
		m_EditHeatingSpecifHeat.SetReadOnly( TRUE );
		m_EditHeatingSpecifHeat.SetBlockSelection( true );
		m_EditHeatingSpecifHeat.SetBlockCursorChange( true );
		m_EditHeatingSpecifHeat.SetTextColor( ::GetSysColor( COLOR_GRAYTEXT ) );
		m_EditHeatingSpecifHeat.SetBackColor( ::GetSysColor( COLOR_3DFACE ) );

		GetDlgItem( IDC_STATICSPECIFHEAT_C )->ShowWindow( SW_SHOW );
		GetDlgItem( IDC_STATICSPECIFHEAT_H )->ShowWindow( SW_SHOW );
		GetDlgItem( IDC_STATICSPECIFHEATUNIT_C )->ShowWindow( SW_SHOW );
		GetDlgItem( IDC_STATICSPECIFHEATUNIT_H )->ShowWindow( SW_SHOW );
		GetDlgItem( IDC_EDITSPECIFHEAT_C )->ShowWindow( SW_SHOW );
		GetDlgItem( IDC_EDITSPECIFHEAT_H )->ShowWindow( SW_SHOW );
		GetDlgItem( IDC_STATICFREEZ_C )->ShowWindow( SW_SHOW );
		GetDlgItem( IDC_STATICFREEZ_H )->ShowWindow( SW_SHOW );
		GetDlgItem( IDC_STATICFREEZUNIT_C )->ShowWindow( SW_SHOW );
		GetDlgItem( IDC_STATICFREEZUNIT_H )->ShowWindow( SW_SHOW );
		GetDlgItem( IDC_EDITFREEZ_C )->ShowWindow( SW_SHOW );
		GetDlgItem( IDC_EDITFREEZ_H )->ShowWindow( SW_SHOW );
		GetDlgItem( IDC_STATICWATERPHASE_C )->ShowWindow( SW_SHOW );
		GetDlgItem( IDC_STATICWATERPHASE_H )->ShowWindow( SW_SHOW );
	}
	std::map<int, CString> mapComboListSorted;
	// Fill the additive name combo box for preventing weird behaviour.
	for( IDPtr = pTab->GetFirst( CLASS( CDB_StringID ) ); _T( '\0' ) != *IDPtr.ID; IDPtr = pTab->GetNext() )
	{
		CDB_StringID *pID = dynamic_cast<CDB_StringID *>( IDPtr.MP );

		if( NULL == pID )
		{
			continue;
		}

		if( m_ComboAdditName.FindStringExact( -1, pID->GetString() ) < 0 )
		{
			//HYS-873: Fill the map to sort the combo list 
			int index = 0;
			if( pID->GetIDstrAsInt( 1, index ) && ( index > 0 ) )
			{
				mapComboListSorted.insert( std::pair<int, CString>( index, pID->GetString() ) );
			}
			else
			{
				m_ComboAdditName.InsertString( 0, pID->GetString() );
			}
			if( 0 == strAdditName.Compare( pID->GetString() ) )
			{
				pSelectedStrID = pID;
			}
		}
	}
	//HYS-873: 
	if( mapComboListSorted.size() > 0 )
	{
		for( std::map<int, CString>::iterator iter = mapComboListSorted.begin(); iter != mapComboListSorted.end(); ++iter )
		{
			m_ComboAdditName.InsertString( iter->first - 1, iter->second );
		}
	}
	mapComboListSorted.clear();

	m_ComboAdditName.SetCurSel( 0 );

	if( NULL == pSelectedStrID )
	{
		return;
	}

	if( false == strAdditName.IsEmpty() )
	{
		// Set the additive name selection.
		if( false == strAdditName.IsEmpty() && CB_ERR != m_ComboAdditName.FindStringExact( -1, pSelectedStrID->GetString() ) )
		{
			m_ComboAdditName.SelectString( -1, pSelectedStrID->GetString() );
		}
	}
}

void CDlgCOWaterChar::OnCbnSelChangeAdditiveFamily()
{
	_FillComboAdditName( _T( "" ) );
	m_ComboAdditName.SetCurSel( 0 );
	OnCbnSelChangeAdditiveName();
}

void CDlgCOWaterChar::OnCbnSelChangeAdditiveName()
{
	if( CB_ERR == m_ComboAdditName.GetCurSel() )
	{
		return;
	}

	// Find the additive characteristic ID for current selection.
	CString str( _T( "" ) );
	m_ComboAdditName.GetLBText( m_ComboAdditName.GetCurSel(), str );
	IDPTR IDPtr = m_pAdditFamTab->GetFirst( CLASS( CDB_StringID ) );
	ASSERT( _T( '\0' ) != *IDPtr.ID );

	while( _T( '\0' ) != *IDPtr.ID && ( (CDB_StringID *)IDPtr.MP )->GetString() != str )
	{
		IDPtr = m_pAdditFamTab->GetNext();
	}

	ASSERT( _T( '\0' ) != *IDPtr.ID );

	m_AdditID = IDPtr.ID;

	// Get and store the pointer on the additive characteristics.
	CString AdditCharID = ( (CDB_StringID *)IDPtr.MP )->GetIDstr();
	m_pAdditChar = (CDB_AdditCharacteristic *)( m_pTADB->Get( AdditCharID ).MP );
	ASSERT( NULL != m_pAdditChar );

	if( true == m_pAdditChar->IsaDilutedAddit() )
	{
		GetDlgItem( IDC_STATICPCWEIGHT )->ShowWindow( SW_SHOW );
		m_EditPcWeight.ShowWindow( SW_SHOW );
	}
	else
	{
		m_dPCWeight = m_pAdditChar->GetMinPc();
		m_EditPcWeight.SetWindowText( WriteDouble( m_dPCWeight, 1, 1 ) );
		m_EditPcWeight.ShowWindow( SW_HIDE );
		GetDlgItem( IDC_STATICPCWEIGHT )->ShowWindow( SW_HIDE );
	}

	UpdateAll();
}

