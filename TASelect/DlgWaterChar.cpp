#include "stdafx.h"

#include "TASelect.h"
#include "MainFrm.h"
#include "ProductSelectionParameters.h"
#include "DlgWaterChar.h"

#ifdef _DEBUG
#define new DEBUG_NEW
#undef THIS_FILE
static char THIS_FILE[] = __FILE__;
#endif

CDlgWaterChar::CDlgWaterChar( CDlgWaterChar::INotificationHandler *pfnVerifyFluidCharacteristics, CWnd *pParent )
	: CDialogEx( CDlgWaterChar::IDD, pParent )
{
	m_pfnVerifyFluidCharacteristics = pfnVerifyFluidCharacteristics;
	m_pTADB = NULL;
	m_pTADS = NULL;
	m_ptstrCaption = NULL;
	m_eOrigin = DlgWaterCharOrigin::DlgWaterChar_ForProductSelection;

	CWaterChar clWaterChar;
	clWaterChar.SetTemp( 0.0 );
	clWaterChar.SetPcWeight( 0.0 );
	clWaterChar.SetDens( 0.0 );
	clWaterChar.SetKinVisc( 0.0 );
	clWaterChar.SetCheckSpecifHeat( false );
	clWaterChar.SetSpecifHeat( 0.0 );
	clWaterChar.SetTfreez( 0.0 );
	clWaterChar.SetAdditFamID( _T("") );
	clWaterChar.SetAdditID( _T("") );

	m_mapWaterCharWorking.insert( std::pair<ProjectType, CWaterChar>( ProjectType::Heating, clWaterChar ) );
	m_mapWaterCharWorking.insert( std::pair<ProjectType, CWaterChar>( ProjectType::Cooling, clWaterChar ) );
	m_mapWaterCharWorking.insert( std::pair<ProjectType, CWaterChar>( ProjectType::Solar, clWaterChar ) );

	m_mapDTWorking.insert( std::pair<ProjectType, double>( ProjectType::Heating, 0.0 ) );
	m_mapDTWorking.insert( std::pair<ProjectType, double>( ProjectType::Cooling, 0.0 ) );
	m_mapDTWorking.insert( std::pair<ProjectType, double>( ProjectType::Solar, 0.0 ) );
	
	m_mapWaterCharCopies.insert( std::pair<ProjectType, CWaterChar>( ProjectType::Heating, clWaterChar ) );
	m_mapWaterCharCopies.insert( std::pair<ProjectType, CWaterChar>( ProjectType::Cooling, clWaterChar ) );
	m_mapWaterCharCopies.insert( std::pair<ProjectType, CWaterChar>( ProjectType::Solar, clWaterChar ) );

	m_mapDTCopies.insert( std::pair<ProjectType, double>( ProjectType::Heating, 0.0 ) );
	m_mapDTCopies.insert( std::pair<ProjectType, double>( ProjectType::Cooling, 0.0 ) );
	m_mapDTCopies.insert( std::pair<ProjectType, double>( ProjectType::Solar, 0.0 ) );

	m_mapIsSomethingChanged.insert( std::pair<ProjectType, bool>( ProjectType::Heating, false ) );
	m_mapIsSomethingChanged.insert( std::pair<ProjectType, bool>( ProjectType::Cooling, false ) );
	m_mapIsSomethingChanged.insert( std::pair<ProjectType, bool>( ProjectType::Solar, false ) );

	m_pAdditFamTab = NULL;
	m_pAdditChar = NULL;
	m_eGlobalProjectType = Heating;
	m_eCurrentProjectType = Heating;
	m_bModificationEnabled = true;
}

int CDlgWaterChar::Display( LPCTSTR lptstrCaption, DlgWaterCharOrigin eOrigin, bool bModificationEnabled, ProjectType eApplicationTypeToForce )
{
	m_ptstrCaption = lptstrCaption;
	m_eOrigin = eOrigin;

	m_pTADB = TASApp.GetpTADB();
	m_pTADS = TASApp.GetpTADS();

	if( false == bModificationEnabled )
	{
		m_bModificationEnabled = bModificationEnabled;
	}
	
	m_eForcedProjectType = eApplicationTypeToForce;

	return DoModal();
}

CString CDlgWaterChar::GetAdditiveFamilyID( ProjectType eProjectType )
{ 
	return m_mapWaterCharWorking.at( ( ProjectType::InvalidProjectType == eProjectType ) ? m_eForcedProjectType : eProjectType ).GetAdditFamID();
}

CString CDlgWaterChar::GetAdditiveID( ProjectType eProjectType )
{ 
	return m_mapWaterCharWorking.at( ( ProjectType::InvalidProjectType == eProjectType ) ? m_eForcedProjectType : eProjectType ).GetAdditID();
}

double CDlgWaterChar::GetTemp( ProjectType eProjectType )
{ 
	return m_mapWaterCharWorking.at( ( ProjectType::InvalidProjectType == eProjectType ) ? m_eForcedProjectType : eProjectType ).GetTemp();
}

double CDlgWaterChar::GetDT( ProjectType eProjectType )
{
	return m_mapDTWorking.at( ( ProjectType::InvalidProjectType == eProjectType ) ? m_eForcedProjectType : eProjectType );
}

double CDlgWaterChar::GetPCWeight( ProjectType eProjectType )
{ 
	return m_mapWaterCharWorking.at( ( ProjectType::InvalidProjectType == eProjectType ) ? m_eForcedProjectType : eProjectType ).GetPcWeight();
}

double CDlgWaterChar::GetDensity( ProjectType eProjectType )
{ 
	return m_mapWaterCharWorking.at( ( ProjectType::InvalidProjectType == eProjectType ) ? m_eForcedProjectType : eProjectType ).GetDens();
}

double CDlgWaterChar::GetKinematicViscosity( ProjectType eProjectType )
{ 
	return m_mapWaterCharWorking.at( ( ProjectType::InvalidProjectType == eProjectType ) ? m_eForcedProjectType : eProjectType ).GetKinVisc();
}

double CDlgWaterChar::GetSpecificHeat( ProjectType eProjectType )
{ 
	return m_mapWaterCharWorking.at( ( ProjectType::InvalidProjectType == eProjectType ) ? m_eForcedProjectType : eProjectType ).GetSpecifHeat();
}

double CDlgWaterChar::GetFrozenTemperature( ProjectType eProjectType )
{ 
	return m_mapWaterCharWorking.at( ( ProjectType::InvalidProjectType == eProjectType ) ? m_eForcedProjectType : eProjectType ).GetTfreez();
}

void CDlgWaterChar::GetWaterChar( CWaterChar &clWaterChar, double &dDT )
{
	clWaterChar = m_mapWaterCharWorking.at( m_eForcedProjectType );
	dDT = m_mapDTWorking.at( m_eForcedProjectType );
}
	
bool CDlgWaterChar::GetWaterChar( ProjectType eProjectType, CWaterChar &clWaterChar, double &dDT )
{
	if( 0 == m_mapWaterCharWorking.count( eProjectType ) )
	{
		return false;
	}

	clWaterChar = m_mapWaterCharWorking.at( eProjectType );
	dDT = m_mapDTCopies.at( eProjectType );

	return true;
}

bool CDlgWaterChar::IsSomethingChanged()
{
	return m_mapDTWorking.at( m_eForcedProjectType );
}

bool CDlgWaterChar::IsSomethingChanged( ProjectType eProjectType )
{
	if( 0 == m_mapDTWorking.count( eProjectType ) )
	{
		return false;
	}

	return m_mapDTWorking.at( eProjectType );
}

BEGIN_MESSAGE_MAP( CDlgWaterChar, CDialogEx )
	ON_WM_DESTROY()
	ON_EN_CHANGE( IDC_EDITTEMP, OnChangeEditTemperature )
	ON_EN_CHANGE( IDC_EDITDT, OnChangeEditDT )
	ON_CBN_SELCHANGE( IDC_COMBOAPPLICATIONTYPE, OnCbnSelChangeApplicationType )
	ON_CBN_SELCHANGE( IDC_COMBOADDITFAM, OnCbnSelChangeAdditiveFamily )
	ON_CBN_SELCHANGE( IDC_COMBOADDITNAME, OnCbnSelChangeAdditiveName )
	ON_BN_CLICKED( IDC_CHECKSPECIFHEAT, OnCheckSpecificHeat )
	ON_EN_CHANGE( IDC_EDITPCWEIGHT, OnChangeEditPCWeight )
	ON_BN_CLICKED( IDC_BUTTONRECOMMENDATION, OnButtonRecommendation )
	ON_EN_CHANGE( IDC_EDITDENSITY, OnChangeEditDensity )
	ON_EN_CHANGE( IDC_EDITKINVISC, OnChangeEditKinematicViscosity )
	ON_EN_CHANGE( IDC_EDITSPECIFHEAT, OnChangeEditSpecificHeat )
	ON_NOTIFY( WM_USER_ENTERKEYPRESSED, IDC_EDITTEMP, OnEditEnterTemperature )
	ON_EN_KILLFOCUS( IDC_EDITTEMP, OnEnKillFocusTemperature )
	ON_EN_KILLFOCUS( IDC_EDITDT, OnEnKillFocusDT )
	ON_EN_KILLFOCUS( IDC_EDITPCWEIGHT, OnEnKillFocusPCWeight )
	ON_EN_KILLFOCUS( IDC_EDITDENSITY, OnEnKillFocusDensity )
	ON_EN_KILLFOCUS( IDC_EDITKINVISC, OnEnKillFocusKinematicViscosity )
	ON_EN_KILLFOCUS( IDC_EDITSPECIFHEAT, OnEnKillFocusSpecificHeat )
	ON_MESSAGE( WM_USER_UNITCHANGE, OnUnitChange )
END_MESSAGE_MAP()

void CDlgWaterChar::DoDataExchange( CDataExchange *pDX )
{
	CDialogEx::DoDataExchange( pDX );
	DDX_Control( pDX, IDC_GROUPAPPLICATIONTYPE, m_GroupApplicationType );
	DDX_Control( pDX, IDC_GROUPPT, m_GroupPT );
	DDX_Control( pDX, IDC_GROUPRHO, m_GroupRho );
	DDX_Control( pDX, IDC_GROUPADDIT, m_GroupAddit );
	DDX_Control( pDX, IDC_EDITTEMP, m_EditTemp );
	DDX_Control( pDX, IDC_EDITDT, m_EditDefaultDT );
	DDX_Control( pDX, IDC_COMBOAPPLICATIONTYPE, m_ComboApplicationType );
	DDX_Control( pDX, IDC_COMBOADDITFAM, m_ComboAdditFam );
	DDX_Control( pDX, IDC_COMBOADDITNAME, m_ComboAdditName );
	DDX_Control( pDX, IDC_CHECKSPECIFHEAT, m_clCheckSpecifHeat );
	DDX_Control( pDX, IDC_EDITSPECIFHEAT, m_EditSpecifHeat );
	DDX_Control( pDX, IDC_BUTTONRECOMMENDATION, m_clButtonRecommendation );
	DDX_Control( pDX, IDC_EDITDENSITY, m_EditDens );
	DDX_Control( pDX, IDC_EDITKINVISC, m_EditKinVisc );
	DDX_Control( pDX, IDC_EDITPCWEIGHT, m_EditPcWeight );
	DDX_Control( pDX, IDC_EDITFREEZ, m_EditFreezPoint );
	DDX_Control( pDX, IDC_EDITWARNING, m_StaticWarning );
	DDX_Control( pDX, IDOK, m_clButtonOK );
}

BOOL CDlgWaterChar::OnInitDialog()
{
	CDialogEx::OnInitDialog();

	// Initialize dialog strings.
	CString str;

	if( DlgWaterCharOrigin::DlgWaterChar_ForProductSelection == m_eOrigin )
	{
		str = TASApp.LoadLocalizedString( IDS_WATERCHAR_SSEL );
	}
	else if( DlgWaterCharOrigin::DlgWaterChar_ForHMCalc == m_eOrigin )
	{
		str = TASApp.LoadLocalizedString( IDS_WATERCHAR_PROJ );
	}
	else
	{
		str = TASApp.LoadLocalizedString( IDS_WATERCHAR_TOOLS );
	}

	SetWindowText( str );

	str = TASApp.LoadLocalizedString( IDS_DLGWATERCHAR_APPLICATION );
	GetDlgItem( IDC_STATICAPPLICATIONTYPE )->SetWindowTextW( str );

	if( DlgWaterCharOrigin::DlgWaterChar_ForProductSelection == m_eOrigin )
	{
		str = TASApp.LoadLocalizedString( IDS_DLGWATERCHAR_STATICTEMP );
	}
	else if( DlgWaterCharOrigin::DlgWaterChar_ForHMCalc == m_eOrigin )
	{
		str = TASApp.LoadLocalizedString( IDS_DLGWATERCHAR_SUPPLYTEMP );
	}
	else
	{
		str = TASApp.LoadLocalizedString( IDS_DLGWATERCHAR_STATICTEMP );
	}

	GetDlgItem( IDC_STATICTEMP )->SetWindowText( str );

	str = TASApp.LoadLocalizedString( IDS_DLGWATERCHAR_DEFAULTDT );
	GetDlgItem( IDC_STATICDT )->SetWindowText( str );

	str = TASApp.LoadLocalizedString( IDS_DLGWATERCHAR_STATICADDITFAM );
	GetDlgItem( IDC_STATICADDITFAM )->SetWindowText( str );
	
	str = TASApp.LoadLocalizedString( IDS_DLGWATERCHAR_STATICADDITNAME );
	GetDlgItem( IDC_STATICADDITNAME )->SetWindowText( str );
	
	str = TASApp.LoadLocalizedString( IDS_DLGWATERCHAR_CHECKSPECIFHEAT );
	GetDlgItem( IDC_CHECKSPECIFHEAT )->SetWindowText( str );
	
	str = TASApp.LoadLocalizedString( IDS_DLGWATERCHAR_STATICPCWEIGHT );
	GetDlgItem( IDC_STATICPCWEIGHT )->SetWindowText( str );
	
	str = TASApp.LoadLocalizedString( IDS_DLGWATERCHAR_STATICDENSITY );
	GetDlgItem( IDC_STATICDENSITY )->SetWindowText( str );
	
	str = TASApp.LoadLocalizedString( IDS_DLGWATERCHAR_STATICKINVISC );
	GetDlgItem( IDC_STATICKINVISC )->SetWindowText( str );
	
	str = TASApp.LoadLocalizedString( IDS_DLGWATERCHAR_STATICSPECIFHEAT );
	GetDlgItem( IDC_STATICSPECIFHEAT )->SetWindowText( str );
	
	str = TASApp.LoadLocalizedString( IDS_DLGWATERCHAR_STATICFREEZ );
	GetDlgItem( IDC_STATICFREEZ )->SetWindowText( str );

	str = TASApp.LoadLocalizedString( IDS_OK );
	GetDlgItem( IDOK )->SetWindowText( str );
	
	str = TASApp.LoadLocalizedString( IDS_CANCEL );
	GetDlgItem( IDCANCEL )->SetWindowText( str );
	str.Empty();

	// Set font for static IDC_STATICWARNING.
	str = TASApp.LoadLocalizedString( IDS_WATERCHAR_WARNING );
	m_StaticWarning.SetWindowText( str );
	m_StaticWarning.SetBlockCursorChange( true );
	m_StaticWarning.SetBlockSelection( true );
	m_StaticWarning.SetTextColor( ::GetSysColor( COLOR_GRAYTEXT ) );
	m_StaticWarning.SetBackColor( ::GetSysColor( COLOR_3DFACE ) );

	// Set caption in the title bar of the dialog.
	if( NULL != m_ptstrCaption )
	{
		SetWindowText( m_ptstrCaption );
	}

	// Add icon for application type.
	CImageList *pclImgListGroupBox = TASApp.GetpRCImageManager()->GetImageList( CRCImageManager::ILN_StripProject );
	ASSERT( NULL != pclImgListGroupBox );

	if( NULL != pclImgListGroupBox )
	{
		m_GroupApplicationType.SetImageList( pclImgListGroupBox, CRCImageManager::ImgListStripProject::ILSP_ProjectType );
	}

	// Add icons to PT, additive and density groups.
	pclImgListGroupBox = TASApp.GetpRCImageManager()->GetImageList( CRCImageManager::ILN_GroupBox );
	ASSERT( NULL != pclImgListGroupBox );

	if( NULL != pclImgListGroupBox )
	{
		m_GroupPT.SetImageList( pclImgListGroupBox, CRCImageManager::ILGB_H2OPt );
		m_GroupAddit.SetImageList( pclImgListGroupBox, CRCImageManager::ILGB_H2OAddit );
		m_GroupRho.SetImageList( pclImgListGroupBox, CRCImageManager::ILGB_Note );
	}

	// Load bitmaps for water phase.
	m_BitmapLiquid.LoadMappedBitmap( IDB_H2OLIQUID );
	m_BitmapIce.LoadMappedBitmap( IDB_H2OICE );
	m_BitmapUnknown.LoadMappedBitmap( IDB_H2OX );

	CDS_TechnicalParameter *pTech = m_pTADS->GetpTechParams();

	// Load the appropriate water characteristic depending on the 'm_eOrigin'.
	if( DlgWaterCharOrigin::DlgWaterChar_ForProductSelection == m_eOrigin || DlgWaterCharOrigin::DlgWaterChar_ForTools == m_eOrigin )
	{
		m_eGlobalProjectType = pTech->GetProductSelectionApplicationType();
		
		// Working.
		m_mapWaterCharWorking.at( ProjectType::Heating ) = *pTech->GetDefaultISHeatingWC();
		m_mapWaterCharWorking.at( ProjectType::Cooling ) = *pTech->GetDefaultISCoolingWC();
		m_mapWaterCharWorking.at( ProjectType::Solar ) = *pTech->GetDefaultISSolarWC();

		m_mapDTWorking.at( ProjectType::Heating ) = pTech->GetDefaultISHeatingDT();
		m_mapDTWorking.at( ProjectType::Cooling ) = pTech->GetDefaultISCoolingDT();
		m_mapDTWorking.at( ProjectType::Solar ) = pTech->GetDefaultISSolarDT();

		// Copies.
		m_mapWaterCharCopies = m_mapWaterCharWorking;
		m_mapDTCopies = m_mapDTWorking;
	}
	else
	{
		m_eGlobalProjectType = pTech->GetProjectApplicationType();

		// Heating.
		CWaterChar clWaterChar;
		clWaterChar.SetTemp( pTech->GetDefaultPrjHeatingTps() );
		clWaterChar.SetPcWeight( pTech->GetDefaultPrjHeatingPcWeight() );
		clWaterChar.SetAdditID( pTech->GetDefaultPrjHeatingAdditID() );
						
		IDPTR AdditIDPtr = m_pTADB->Get( clWaterChar.GetAdditID() );
		ASSERT( _T('\0') != AdditIDPtr.ID );
		clWaterChar.SetAdditFamID( AdditIDPtr.PP->GetIDPtr().ID );

		m_mapWaterCharWorking.at( ProjectType::Heating ) = clWaterChar;
		m_mapDTWorking.at( ProjectType::Heating ) = pTech->GetDefaultPrjHeatingDT();

		// Cooling.
		clWaterChar.SetTemp( pTech->GetDefaultPrjCoolingTps() );
		clWaterChar.SetPcWeight( pTech->GetDefaultPrjCoolingPcWeight() );
		clWaterChar.SetAdditID( pTech->GetDefaultPrjCoolingAdditID() );
						
		AdditIDPtr = m_pTADB->Get( clWaterChar.GetAdditID() );
		ASSERT( _T('\0') != AdditIDPtr.ID );
		clWaterChar.SetAdditFamID( AdditIDPtr.PP->GetIDPtr().ID );

		m_mapWaterCharWorking.at( ProjectType::Cooling ) = clWaterChar;
		m_mapDTWorking.at( ProjectType::Cooling ) = pTech->GetDefaultPrjCoolingDT();

		// Copies.
		m_mapWaterCharCopies = m_mapWaterCharWorking;
		m_mapDTCopies = m_mapDTWorking;
	}

	// Fill all unit static controls.
	OnUnitChange();

	if( ProjectType::InvalidProjectType == m_eForcedProjectType )
	{
		m_eForcedProjectType = m_eGlobalProjectType;
	}

	// Fill the application type combo.
	_FillComboApplicationType( m_eForcedProjectType );

	// Don't allow to change application type for product selection or HM calc.
	if( DlgWaterCharOrigin::DlgWaterChar_ForProductSelection == m_eOrigin || DlgWaterCharOrigin::DlgWaterChar_ForHMCalc == m_eOrigin )
	{
		m_ComboApplicationType.EnableWindow( FALSE );
	}

	// Initialize the temperature, pressure, % weight, density, kinematic viscosity and specific heat edit boxes.
	m_EditTemp.SetPhysicalType( _U_TEMPERATURE );
	m_EditTemp.SetMinDblValue( -273.15 );
	m_EditTemp.SetEditSign( CNumString::eBoth );
	m_EditTemp.SetCurrentValSI( m_mapWaterCharWorking.at( m_eForcedProjectType ).GetTemp() );
	m_EditTemp.Update();

	m_EditPcWeight.SetPhysicalType( _U_PERCENT );
	m_EditPcWeight.SetMinDblValue( 0 );
	m_EditPcWeight.SetEditSign( CNumString::ePositive );
	m_EditPcWeight.SetCurrentValSI( m_mapWaterCharWorking.at( m_eForcedProjectType ).GetPcWeight() );
	m_EditPcWeight.Update();

	m_EditDens.SetPhysicalType( _U_DENSITY );
	m_EditDens.SetMinDblValue( 0 );
	m_EditDens.SetEditSign( CNumString::ePositive );
	m_EditDens.SetCurrentValSI( m_mapWaterCharWorking.at( m_eForcedProjectType ).GetDens() );
	m_EditDens.Update();

	m_EditKinVisc.SetPhysicalType( _U_KINVISCOSITY );
	m_EditKinVisc.SetMinDblValue( 0 );
	m_EditKinVisc.SetEditSign( CNumString::ePositive );
	m_EditKinVisc.SetCurrentValSI( m_mapWaterCharWorking.at( m_eForcedProjectType ).GetKinVisc() );
	m_EditKinVisc.Update();

	m_EditSpecifHeat.SetPhysicalType( _U_SPECIFHEAT );
	m_EditSpecifHeat.SetMinDblValue( 0 );
	m_EditSpecifHeat.SetEditSign( CNumString::ePositive );
	m_EditSpecifHeat.SetCurrentValSI( m_mapWaterCharWorking.at( m_eForcedProjectType ).GetSpecifHeat() );
	m_EditSpecifHeat.Update();

	m_EditDefaultDT.SetPhysicalType( _U_DIFFTEMP );
	m_EditDefaultDT.SetMinDblValue( 0 );
	m_EditDefaultDT.SetEditSign( CNumString::ePositive );
	m_EditDefaultDT.SetCurrentValSI( m_mapDTWorking.at( m_eForcedProjectType ) );
	m_EditDefaultDT.Update();

	m_EditFreezPoint.SetBlockSelection( true );
	m_EditFreezPoint.SetBlockCursorChange( true );
	m_EditFreezPoint.SetTextColor( ::GetSysColor( COLOR_GRAYTEXT ) );
	m_EditFreezPoint.SetBackColor( ::GetSysColor( COLOR_3DFACE ) );

	// Fill the additive combo boxes.
	CString pFam = ( (CTable *)m_pTADB->Get( m_mapWaterCharWorking.at( m_eForcedProjectType ).GetAdditFamID() ).MP )->GetName();
	CString pAddit = ( (CDB_StringID *)m_pTADB->Get( m_mapWaterCharWorking.at( m_eForcedProjectType ).GetAdditID() ).MP )->GetString();
	_FillComboAdditFam( (LPCTSTR)pFam );
	_FillComboAdditName( pAddit );
	OnCbnSelChangeAdditiveName();

	CImageList *pclImgListButton = TASApp.GetpRCImageManager()->GetImageList( CRCImageManager::ILN_Button );
	ASSERT( NULL != pclImgListButton );

	if( NULL != pclImgListButton )
	{
		m_clButtonRecommendation.SetIcon( (HICON)pclImgListButton->ExtractIcon( CRCImageManager::ILB_AdditRecomended ) );
	}

	if( false == m_bModificationEnabled )
	{
		m_GroupApplicationType.SaveChildList();
		m_GroupPT.SaveChildList();
		m_GroupAddit.SaveChildList();
		m_GroupRho.SaveChildList();
		m_GroupApplicationType.EnableWindow( false, true );
		m_GroupPT.EnableWindow( false, true );
		m_GroupAddit.EnableWindow( false, true );
		m_GroupRho.EnableWindow( false, true );
		GetDlgItem( IDOK )->EnableWindow( FALSE );
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

BOOL CDlgWaterChar::PreTranslateMessage( MSG *pMsg )
{
	// Why not use 'WM_KEYDOWN' in the message map? Because if edit control has the focus in the dialog, WM_KEYDOWN is sent to this control
	// and not to this dialog.
	if( WM_KEYDOWN == pMsg->message && VK_ESCAPE == pMsg->wParam )
	{
		EndDialog( IDCANCEL );
	}

	return CDialogEx::PreTranslateMessage( pMsg );
}

void CDlgWaterChar::OnOK()
{
	PREVENT_ENTER_KEY

	m_EditDefaultDT.ResetDrawBorder();
	m_EditTemp.ResetDrawBorder();
	_DisplayWarningWithID( false );

	// Check valid data and not ice.
	// HYS-1194 : Do not enter in this condition if AdditFamID = OTHER_ADDIT
	if( 0 != _tcscmp( m_mapWaterCharWorking.at( m_eCurrentProjectType ).GetAdditFamID(), _T( "OTHER_ADDIT" ) ) )
	{
		if( m_mapWaterCharWorking.at( m_eCurrentProjectType ).GetTemp() <= m_mapWaterCharWorking.at( m_eCurrentProjectType ).GetTfreez() )
		{
			_DisplayWarningWithID( true, &m_EditTemp, AFXMSG_BELOW_TFREEZ, m_mapWaterCharWorking.at( m_eCurrentProjectType ).GetTfreez(), _U_TEMPERATURE );
			m_EditTemp.SetFocus();
			return;
		}

		// Verify default DT.
		if( ProjectType::Cooling != m_eCurrentProjectType )
		{
			if( m_mapWaterCharWorking.at( m_eCurrentProjectType ).GetTemp() - m_mapDTWorking.at( m_eCurrentProjectType ) <= m_mapWaterCharWorking.at( m_eCurrentProjectType ).GetTfreez() )
			{
				_DisplayWarningWithID( true, &m_EditDefaultDT, AFXMSG_DTBELOW_TFREEZ, m_mapWaterCharWorking.at( m_eCurrentProjectType ).GetTfreez(), _U_TEMPERATURE );
				m_EditDefaultDT.SetFocus();
				return;
			}
		}
	}

	// Transfer water characteristics to appropriate place depending on the 'm_eOrigin'.
	CDS_TechnicalParameter *pTechParam = m_pTADS->GetpTechParams();
	CWaterChar *pWC = NULL;

	if( DlgWaterCharOrigin::DlgWaterChar_ForProductSelection == m_eOrigin || DlgWaterCharOrigin::DlgWaterChar_ForTools == m_eOrigin )
	{
		// By default save all in the technical parameters.
		pTechParam->SetDefaultISHeatingWC( m_mapWaterCharWorking.at( ProjectType::Heating ) );
		pTechParam->SetDefaultISCoolingWC( m_mapWaterCharWorking.at( ProjectType::Cooling ) );
		pTechParam->SetDefaultISSolarWC( m_mapWaterCharWorking.at( ProjectType::Solar ) );

		pTechParam->SetDefaultISHeatingDT( m_mapDTWorking.at(  ProjectType::Heating ) );
		pTechParam->SetDefaultISCoolingDT( m_mapDTWorking.at( ProjectType::Cooling ) );
		pTechParam->SetDefaultISSolarDT( m_mapDTWorking.at( ProjectType::Solar ) );

		if( false == m_mapWaterCharCopies.at( ProjectType::Heating ).Compare( m_mapWaterCharWorking.at( ProjectType::Heating ), true ) 
				|| m_mapDTCopies.at( ProjectType::Heating ) != m_mapDTWorking.at( ProjectType::Heating ) )
		{
			m_mapIsSomethingChanged.at( ProjectType::Heating ) = true;
		}

		if( false == m_mapWaterCharCopies.at( ProjectType::Cooling ).Compare( m_mapWaterCharWorking.at( ProjectType::Cooling ), true ) 
				|| m_mapDTCopies.at( ProjectType::Cooling ) != m_mapDTWorking.at( ProjectType::Cooling ) )
		{
			m_mapIsSomethingChanged.at( ProjectType::Cooling ) = true;
		}

		if( false == m_mapWaterCharCopies.at( ProjectType::Solar ).Compare( m_mapWaterCharWorking.at( ProjectType::Solar ), true ) 
				|| m_mapDTCopies.at( ProjectType::Solar ) != m_mapDTWorking.at( ProjectType::Solar ) )
		{
			m_mapIsSomethingChanged.at( ProjectType::Solar ) = true;
		}

		// If user change water characteristic for the current application type, we save in water characteristic table (PARAM_WC).
		if( true == m_mapIsSomethingChanged.at( m_eGlobalProjectType ) )
		{
			*m_pTADS->GetpWCForProductSelection()->GetpWCData() = m_mapWaterCharWorking.at( m_eGlobalProjectType );
		}
	}
	else
	{
		// By default save all in the technical parameters.

		// Heating.
		pTechParam->SetDefaultPrjHeatingTps( m_mapWaterCharWorking.at( ProjectType::Heating ).GetTemp() );
		pTechParam->SetDefaultPrjHeatingAddit( m_mapWaterCharWorking.at( ProjectType::Heating ).GetAdditID() );
		pTechParam->SetDefaultPrjHeatingPcWeight( m_mapWaterCharWorking.at( ProjectType::Heating ).GetPcWeight() );
		pTechParam->SetDefaultPrjHeatingDT( m_mapDTWorking.at( ProjectType::Heating ) );

		pTechParam->SetDefaultPrjCoolingTps( m_mapWaterCharWorking.at( ProjectType::Cooling ).GetTemp() );
		pTechParam->SetDefaultPrjCoolingAddit( m_mapWaterCharWorking.at( ProjectType::Cooling ).GetAdditID() );
		pTechParam->SetDefaultPrjCoolingPcWeight( m_mapWaterCharWorking.at( ProjectType::Cooling ).GetPcWeight() );
		pTechParam->SetDefaultPrjCoolingDT( m_mapDTWorking.at( ProjectType::Cooling ) );

		if( false == m_mapWaterCharCopies.at( ProjectType::Heating ).Compare( m_mapWaterCharWorking.at( ProjectType::Heating ), true ) 
				|| m_mapDTCopies.at( ProjectType::Heating ) != m_mapDTWorking.at( ProjectType::Heating ) )
		{
			m_mapIsSomethingChanged.at( ProjectType::Heating ) = true;
		}

		if( false == m_mapWaterCharCopies.at( ProjectType::Cooling ).Compare( m_mapWaterCharWorking.at( ProjectType::Cooling ), true ) 
				|| m_mapDTCopies.at( ProjectType::Cooling ) != m_mapDTWorking.at( ProjectType::Cooling ) )
		{
			m_mapIsSomethingChanged.at( ProjectType::Cooling ) = true;
		}

		// If user change water characteristic for the current application type, we save in water characteristic table (PROJ_WC).
		if( true == m_mapIsSomethingChanged.at( m_eGlobalProjectType ) )
		{
			*m_pTADS->GetpWCForProject()->GetpWCData() = m_mapWaterCharWorking.at( m_eGlobalProjectType );
		}
	}

	// Check if user has made change.
	// We use 'm_eGlobalProjectType' to reset right view and so on only if user has changed something in the water characteristic
	// for the current global project type (The one that was when creating the dialog).
	if( true == m_mapIsSomethingChanged.at( m_eGlobalProjectType ) )
	{
		WPARAM wParam;

		if( DlgWaterCharOrigin::DlgWaterChar_ForProductSelection == m_eOrigin )
		{
			wParam = WMUserWaterCharWParam::WM_UWC_WP_ForProductSel;
		}
		else if( DlgWaterCharOrigin::DlgWaterChar_ForHMCalc == m_eOrigin )
		{
			wParam = WMUserWaterCharWParam::WM_UWC_WP_ForProject;
		}
		else
		{
			wParam = WMUserWaterCharWParam::WM_UWC_WP_ForTools;
		}

		// Send message to inform about modification of water characteristics.
		::AfxGetApp()->m_pMainWnd->SendMessage( WM_USER_WATERCHANGE, wParam, (LPARAM)WMUserWaterCharLParam::WM_UWC_LWP_Change );
		::AfxGetApp()->m_pMainWnd->SendMessageToDescendants( WM_USER_WATERCHANGE, wParam, (LPARAM)WMUserWaterCharLParam::WM_UWC_LWP_Change );
	}

	CDialogEx::OnOK();
}

void CDlgWaterChar::OnDestroy()
{
	CDialogEx::OnDestroy();

	m_font.DeleteObject();
}

void CDlgWaterChar::OnChangeEditTemperature()
{
	if( GetFocus() != &m_EditTemp )
	{
		return;
	}

	double dTemperature = 0.0;

	switch( ReadDouble( m_EditTemp, &dTemperature ) )
	{
		case RD_EMPTY:
			// Update fields to unknown state.
			SetUnknownState();
			break;

		case RD_NOT_NUMBER:
			break;

		case RD_OK:
			dTemperature = CDimValue::CUtoSI( _U_TEMPERATURE, dTemperature );
			m_mapWaterCharWorking.at( m_eCurrentProjectType ).SetTemp( dTemperature );
			UpdateAll();
			break;
	}
}

void CDlgWaterChar::OnChangeEditDT()
{
	if( GetFocus() != &m_EditDefaultDT )
	{
		return;
	}

	double dDefaultDT = 0.0;

	switch( ReadDouble( m_EditDefaultDT, &dDefaultDT ) )
	{
		case RD_EMPTY:
			break;

		case RD_NOT_NUMBER:
			break;

		case RD_OK:
			m_mapDTWorking.at( m_eCurrentProjectType ) = CDimValue::CUtoSI( _U_DIFFTEMP, dDefaultDT );
			UpdateAll();
			break;
	}
}

void CDlgWaterChar::OnButtonRecommendation()
{
	// Look at Disabling the bell in the MSDN help for disabling the beep when the AfxMessageBox is displayed.
	TASApp.DisplayLocalizeMessageBox( IDS_WATERCHAR_WARNINGPH, MB_OK | MB_ICONINFORMATION );
}

void CDlgWaterChar::OnCheckSpecificHeat()
{
	m_mapWaterCharWorking.at( m_eCurrentProjectType ).SetCheckSpecifHeat( ( BST_CHECKED == m_clCheckSpecifHeat.GetCheck() ) ? true : false );
	int iVisible = ( true == m_mapWaterCharWorking.at( m_eCurrentProjectType ).GetCheckSpecifHeat() ) ? SW_SHOW : SW_HIDE;
	GetDlgItem( IDC_STATICSPECIFHEAT )->ShowWindow( iVisible );
	GetDlgItem( IDC_STATICSPECIFHEATUNIT )->ShowWindow( iVisible );
	m_EditSpecifHeat.ShowWindow( iVisible );

	m_mapWaterCharWorking.at( m_eCurrentProjectType ).SetSpecifHeat( 0.0 );

	if( true == m_mapWaterCharWorking.at( m_eCurrentProjectType ).GetCheckSpecifHeat() )
	{
		m_EditSpecifHeat.SetWindowText( _T("") );
		m_EditSpecifHeat.SetFocus();
	}
}

void CDlgWaterChar::OnChangeEditPCWeight()
{
	if( GetFocus() != &m_EditPcWeight )
	{
		return;
	}

	double dPCWeight = 0.0;

	switch( ReadDouble( m_EditPcWeight, &dPCWeight ) )
	{
		case RD_EMPTY:
			SetUnknownState();
			break;

		case RD_NOT_NUMBER:
			break;

		case RD_OK:
			m_mapWaterCharWorking.at( m_eCurrentProjectType ).SetPcWeight( dPCWeight );
			UpdateAll() ;
			break;
	}
}

void CDlgWaterChar::OnChangeEditDensity()
{
	if( GetFocus() != &m_EditDens )
	{
		return;
	}

	double dDensity = 0.0;

	switch( ReadDouble( m_EditDens, &dDensity ) )
	{
		case RD_EMPTY:
			break;

		case RD_NOT_NUMBER:
			break;

		case RD_OK:
			dDensity = CDimValue::CUtoSI( _U_DENSITY, dDensity );
			break;
	}

	m_mapWaterCharWorking.at( m_eCurrentProjectType ).SetDens( dDensity );
}

void CDlgWaterChar::OnChangeEditKinematicViscosity()
{
	if( GetFocus() != &m_EditKinVisc )
	{
		return;
	}

	double dKinVisc = 0.0;

	switch( ReadDouble( m_EditKinVisc, &dKinVisc ) )
	{
		case RD_EMPTY:
			break;

		case RD_NOT_NUMBER:
			break;

		case RD_OK:
			dKinVisc = CDimValue::CUtoSI( _U_KINVISCOSITY, dKinVisc );
			break;
	}

	m_mapWaterCharWorking.at( m_eCurrentProjectType ).SetKinVisc( dKinVisc );
}

void CDlgWaterChar::OnChangeEditSpecificHeat()
{
	if( GetFocus() != &m_EditSpecifHeat )
	{
		return;
	}

	double dSpecifHeat = 0.0;

	switch( ReadDouble( m_EditSpecifHeat, &dSpecifHeat ) )
	{
		case RD_EMPTY:
			break;

		case RD_NOT_NUMBER:
			break;

		case RD_OK:
			dSpecifHeat = CDimValue::CUtoSI( _U_SPECIFHEAT, dSpecifHeat );
			break;
	}

	m_mapWaterCharWorking.at( m_eCurrentProjectType ).SetSpecifHeat( dSpecifHeat );
}

void CDlgWaterChar::OnEditEnterTemperature( NMHDR *pNMHDR, LRESULT *pResult )
{
	if( IDC_EDITTEMP == pNMHDR->idFrom )
	{
		GetDlgItem( IDOK )->SetFocus();
		OnOK();
	}
}

void CDlgWaterChar::OnEnKillFocusTemperature()
{
	PREVENT_NOT_CANCEL_WITH_EMPTY_BOX

	double dValue = m_mapWaterCharWorking.at( m_eCurrentProjectType ).GetTemp();

	switch( ReadDouble( m_EditTemp, &dValue ) )
	{
		case RD_EMPTY:
			// 			m_EditTemp.SetFocus();
			// 			TASApp.DisplayLocalizeMessageBox( AFXMSG_NON_NUMERICAL_VALUE );
			break;

		case RD_OK:
			dValue = CDimValue::CUtoSI( _U_TEMPERATURE, dValue );

			if( dValue < m_mapWaterCharWorking.at( m_eCurrentProjectType ).GetTfreez() && m_mapWaterCharWorking.at( m_eCurrentProjectType ).GetAdditFamID() != _T("OTHER_ADDIT") )
			{
				// DO NOTHING
			}
			else
			{
				m_EditTemp.SetCurrentValSI( m_mapWaterCharWorking.at( m_eCurrentProjectType ).GetTemp() );
				m_EditTemp.Update();

				// Verify default DT.
				if( ProjectType::Cooling != m_eCurrentProjectType && m_mapWaterCharWorking.at( m_eCurrentProjectType ).GetAdditFamID() != _T("OTHER_ADDIT") )
				{
					if( m_mapWaterCharWorking.at( m_eCurrentProjectType ).GetTemp() - m_mapDTWorking.at( m_eCurrentProjectType ) < m_mapWaterCharWorking.at( m_eCurrentProjectType ).GetTfreez() )
					{
						m_mapDTWorking.at( m_eCurrentProjectType ) = 0.0;
						m_EditDefaultDT.SetWindowText( _T("") );
					}
				}
			}

			break;
	}
}

void CDlgWaterChar::OnEnKillFocusDT()
{
	PREVENT_NOT_CANCEL_WITH_EMPTY_BOX

	if( GetFocus() == &m_EditDefaultDT )
	{
		return;
	}

	m_EditDefaultDT.SetWindowText( WriteCUDouble( _U_DIFFTEMP, m_mapDTWorking.at( m_eCurrentProjectType ) ) );
}

void CDlgWaterChar::OnEnKillFocusPCWeight()
{
	PREVENT_NOT_CANCEL_WITH_EMPTY_BOX

	m_EditPcWeight.SetWindowText( WriteCUDouble( _U_PERCENT, m_mapWaterCharWorking.at( m_eCurrentProjectType ).GetPcWeight() ) );
}

void CDlgWaterChar::OnEnKillFocusDensity()
{
	PREVENT_NOT_CANCEL_WITH_EMPTY_BOX

	m_EditDens.SetWindowText( WriteCUDouble( _U_DENSITY, m_mapWaterCharWorking.at( m_eCurrentProjectType ).GetDens() ) );
}

void CDlgWaterChar::OnEnKillFocusKinematicViscosity()
{
	PREVENT_NOT_CANCEL_WITH_EMPTY_BOX

	m_EditKinVisc.SetWindowText( WriteCUDouble( _U_KINVISCOSITY, m_mapWaterCharWorking.at( m_eCurrentProjectType ).GetKinVisc() ) );
}

void CDlgWaterChar::OnEnKillFocusSpecificHeat()
{
	PREVENT_NOT_CANCEL_WITH_EMPTY_BOX

	if( GetFocus() == GetDlgItem( IDC_CHECKSPECIFHEAT ) )
	{
		return;
	}

	m_EditSpecifHeat.SetWindowText( WriteCUDouble( _U_SPECIFHEAT, m_mapWaterCharWorking.at( m_eCurrentProjectType ).GetSpecifHeat() ) );
}

LRESULT CDlgWaterChar::OnUnitChange( WPARAM wParam, LPARAM lParam )
{
	TCHAR tcName[_MAXCHARS];
	CUnitDatabase *pUnitDB = CDimValue::AccessUDB();

	// Set the text for the temperature units.
	GetNameOf( pUnitDB->GetDefaultUnit( _U_TEMPERATURE ), tcName );
	SetDlgItemText( IDC_STATICTEMPUNIT, tcName );
	SetDlgItemText( IDC_STATICFREEZUNIT, tcName );

	GetNameOf( pUnitDB->GetDefaultUnit( _U_DIFFTEMP ), tcName );
	SetDlgItemText( IDC_STATICUNITDT, tcName );

	// Set the text for the density units.
	GetNameOf( pUnitDB->GetDefaultUnit( _U_DENSITY ), tcName );
	SetDlgItemText( IDC_STATICDENSUNIT, tcName );

	// Set the text for the kinematic viscosity units.
	GetNameOf( pUnitDB->GetDefaultUnit( _U_KINVISCOSITY ), tcName );
	SetDlgItemText( IDC_STATICKINVISCUNIT, tcName );

	// Set the text for the specific heat units.
	GetNameOf( pUnitDB->GetDefaultUnit( _U_SPECIFHEAT ), tcName );
	SetDlgItemText( IDC_STATICSPECIFHEATUNIT, tcName );

	return 0;
}

void CDlgWaterChar::SetUnknownState()
{
	// Load bitmap for unknown state and clean the edit boxes.
	UpdateWaterBitmap( WP_UNKNOWN );
	m_EditDens.SetWindowText( _T("") );
	m_EditKinVisc.SetWindowText( _T("") );
	m_EditSpecifHeat.SetWindowText( _T("") );
	m_EditFreezPoint.SetWindowText( _T("") );
}

void CDlgWaterChar::UpdateWaterBitmap( enum WaterPhase_enum phase )
{
	HBITMAP hBitmap;

	switch( phase )
	{
		case WP_LIQUID:
			hBitmap = ( HBITMAP )m_BitmapLiquid.GetSafeHandle();
			( ( CStatic * )GetDlgItem( IDC_STATICWATERPHASE ) )->SetBitmap( hBitmap );
			break;

		case WP_ICE:
			hBitmap = ( HBITMAP )m_BitmapIce.GetSafeHandle();
			( ( CStatic * )GetDlgItem( IDC_STATICWATERPHASE ) )->SetBitmap( hBitmap );
			break;

		case WP_UNKNOWN:
			hBitmap = ( HBITMAP )m_BitmapUnknown.GetSafeHandle();
			( ( CStatic * )GetDlgItem( IDC_STATICWATERPHASE ) )->SetBitmap( hBitmap );
			break;

		default:
			ASSERT( TRUE );
			break;
	}
}

void CDlgWaterChar::UpdateAll()
{
	if( NULL != m_pfnVerifyFluidCharacteristics )
	{
		bool bEnable = true;
		CString strMsg = _T("");
		m_pfnVerifyFluidCharacteristics->OnVerifyFluidCharacteristics( bEnable, strMsg, this );

		if( false == bEnable )
		{
			_DisplayWarningWithString( true, &m_EditTemp, strMsg );
			return;
		}
	}

	if( NULL == m_pAdditChar )
	{
		return;
	}

	if( 0 == _tcscmp( m_mapWaterCharWorking.at( m_eCurrentProjectType ).GetAdditFamID(), _T("OTHER_ADDIT") ) )
	{
		// HYS-1194 : No refresh but reset error and warning
		m_EditTemp.ResetDrawBorder();
		m_EditDefaultDT.ResetDrawBorder();
		_DisplayWarningWithID( false );
		// No refresh when we are with this family.
		return;
	}

	m_mapWaterCharWorking.at( m_eCurrentProjectType ).SetTfreez( m_pAdditChar->GetTfreez( m_mapWaterCharWorking.at( m_eCurrentProjectType ).GetPcWeight() ) );

	// Increase number of decimal of freezing temperature.
	CString strT = WriteDouble( CDimValue::SItoCU( _U_TEMPERATURE, m_mapWaterCharWorking.at( m_eCurrentProjectType ).GetTfreez() ), 4, 1, 1 );
	m_EditFreezPoint.SetWindowText( strT );

	// Check if the phase is liquid or solid and update accordingly.
	if( m_mapWaterCharWorking.at( m_eCurrentProjectType ).GetTemp() <= m_mapWaterCharWorking.at( m_eCurrentProjectType ).GetTfreez() )
	{
		// Solid phase: ice.
		UpdateWaterBitmap( WP_ICE );

		m_EditDens.SetWindowText( _T("") );
		m_EditKinVisc.SetWindowText( _T("") );
		m_EditSpecifHeat.SetWindowText( _T("") );
		
		_DisplayWarningWithID( true, &m_EditTemp, AFXMSG_BELOW_TFREEZ, m_mapWaterCharWorking.at( m_eCurrentProjectType ).GetTfreez(), _U_TEMPERATURE );
		return;
	}
	else
	{
		m_EditTemp.ResetDrawBorder();
		_DisplayWarningWithID( false );
	}

	// Verify default DT.
	if( ProjectType::Cooling == m_eCurrentProjectType )
	{
		double dMaxTemp = m_pAdditChar->GetMaxT( m_mapWaterCharWorking.at( m_eCurrentProjectType ).GetPcWeight() );

		if( m_mapWaterCharWorking.at( m_eCurrentProjectType ).GetTemp() + m_mapDTWorking.at( m_eCurrentProjectType ) > dMaxTemp )
		{
			_DisplayWarningWithID( true, &m_EditDefaultDT, AFXMSG_DTABOVE_TMAX, dMaxTemp, _U_TEMPERATURE );
			return;
		}
		else
		{
			m_EditDefaultDT.ResetDrawBorder();
			_DisplayWarningWithID( false );
		}
	}
	else
	{
		if( m_mapWaterCharWorking.at( m_eCurrentProjectType ).GetTemp() - m_mapDTWorking.at( m_eCurrentProjectType ) <= m_mapWaterCharWorking.at( m_eCurrentProjectType ).GetTfreez() )
		{
			_DisplayWarningWithID( true, &m_EditDefaultDT, AFXMSG_DTBELOW_TFREEZ, m_mapWaterCharWorking.at( m_eCurrentProjectType ).GetTfreez(), _U_TEMPERATURE );
			return;
		}
		else
		{
			m_EditDefaultDT.ResetDrawBorder();
			_DisplayWarningWithID( false );
		}
	}

	// Liquid phase.
	UpdateWaterBitmap( WP_LIQUID );
	double dDensity;
	double dKinVisc;
	double dSpecifHeat;
	double dVaporPressure = 0;
	enum SurfValReturn_enum SVReturn = m_pAdditChar->GetAdditProp( m_mapWaterCharWorking.at( m_eCurrentProjectType ).GetPcWeight(), m_mapWaterCharWorking.at( m_eCurrentProjectType ).GetTemp(), 
			&dDensity, &dKinVisc, &dSpecifHeat, &dVaporPressure );

	m_mapWaterCharWorking.at( m_eCurrentProjectType ).SetDens( dDensity );
	m_mapWaterCharWorking.at( m_eCurrentProjectType ).SetKinVisc( dKinVisc );
	m_mapWaterCharWorking.at( m_eCurrentProjectType ).SetSpecifHeat( dSpecifHeat );

	switch( SVReturn )
	{
		case SV_OK:
			_DisplayWarningWithID( false );

			m_EditPcWeight.ResetDrawBorder();
			m_EditDens.SetWindowText( WriteDouble( CDimValue::SItoCU( _U_DENSITY, m_mapWaterCharWorking.at( m_eCurrentProjectType ).GetDens() ), 4, 1 ) );
			m_EditKinVisc.SetWindowText( WriteDouble( CDimValue::SItoCU( _U_KINVISCOSITY, m_mapWaterCharWorking.at( m_eCurrentProjectType ).GetKinVisc() ), 4, 1 ) );
			m_EditSpecifHeat.SetWindowText( WriteDouble( CDimValue::SItoCU( _U_SPECIFHEAT, m_mapWaterCharWorking.at( m_eCurrentProjectType ).GetSpecifHeat() ), 4, 1 ) );

			break;

		case SV_OUTOFXRANGE: // Additive percentage above max. value
			{
				// Check concentration min and max
				double dPcMin = m_pAdditChar->GetMinPc();
				double dPcMax = m_pAdditChar->GetMaxPc();

				if( m_mapWaterCharWorking.at( m_eCurrentProjectType ).GetPcWeight() < dPcMin )
				{
					_DisplayWarningWithID( true, &m_EditPcWeight, AFXMSG_BELOW_MINPCWEIGHT, dPcMin, _U_PERCENT );
				}

				if( m_mapWaterCharWorking.at( m_eCurrentProjectType ).GetPcWeight() > dPcMax )
				{
					_DisplayWarningWithID( true, &m_EditPcWeight, AFXMSG_ABOVE_MAXPCWEIGHT, dPcMax, _U_PERCENT );
				}

				SetUnknownState();
			}
			break;

		case SV_ABOVEYMAX: // Temperature above max. value
			_DisplayWarningWithID( true, &m_EditTemp, AFXMSG_ABOVE_MAXTEMP, m_pAdditChar->GetMaxT( m_mapWaterCharWorking.at( m_eCurrentProjectType ).GetPcWeight() ), _U_TEMPERATURE );
			SetUnknownState();
			break;

		default:
			SetUnknownState();
			break;
	}
}

void CDlgWaterChar::_FillComboApplicationType( ProjectType eProjectType )
{
	CDS_TechnicalParameter *pclTechParams = m_pTADS->GetpTechParams();

	int iSelectedItem = 0;
	int iPos = m_ComboApplicationType.AddString( ( LPCTSTR )TASApp.LoadLocalizedString( pclTechParams->GetProductSelectionApplicationTypeIDS( ProjectType::Heating ) ) );
	m_ComboApplicationType.SetItemData( iPos, ProjectType::Heating );

	if( Heating == eProjectType )
	{
		iSelectedItem = iPos;
	}

	iPos = m_ComboApplicationType.AddString( ( LPCTSTR )TASApp.LoadLocalizedString( pclTechParams->GetProductSelectionApplicationTypeIDS( ProjectType::Cooling ) ) );
	m_ComboApplicationType.SetItemData( iPos, ProjectType::Cooling );

	if( Cooling == eProjectType )
	{
		iSelectedItem = iPos;
	}

	// Add solar only for product selection and tools.
	if( DlgWaterCharOrigin::DlgWaterChar_ForProductSelection == m_eOrigin || DlgWaterCharOrigin::DlgWaterChar_ForTools == m_eOrigin )
	{
		iPos = m_ComboApplicationType.AddString( ( LPCTSTR )TASApp.LoadLocalizedString( pclTechParams->GetProductSelectionApplicationTypeIDS( ProjectType::Solar ) ) );
		m_ComboApplicationType.SetItemData( iPos, ProjectType::Solar );

		if( Solar == eProjectType )
		{
			iSelectedItem = iPos;
		}
	}

	m_ComboApplicationType.SetCurSel( iSelectedItem );
	m_eCurrentProjectType = eProjectType;
}

void CDlgWaterChar::_FillComboAdditFam( LPCTSTR ptstrFamily )
{
	// Fill the additive family Combo box.
	if( m_ComboAdditFam.GetCount() > 0 )
	{
		m_ComboAdditFam.ResetContent();
	}

	CTable *pTab = (CTable *)( m_pTADB->Get( _T("ADDITIVE_TAB") ).MP );
	ASSERT( NULL != pTab );
	IDPTR IDPtr = _NULL_IDPTR;

	for( IDPtr = pTab->GetFirst( CLASS( CTable ) ); _T('\0') != *IDPtr.ID; IDPtr = pTab->GetNext() )
	{
		// Skip water & 'Any other fluid' to introduce them at the top/bottom when all other fluids have been inserted.
		if( 0 == _tcscmp( IDPtr.ID, _T("WATER_ADDIT" ) ) || 0 == _tcscmp( IDPtr.ID, _T( "OTHER_ADDIT") ) )
		{
			continue;
		}

		if( m_ComboAdditFam.FindStringExact( -1, ( (CTable *)IDPtr.MP )->GetName() ) < 0 )
		{
			m_ComboAdditFam.AddString( ( (CTable *)IDPtr.MP )->GetName() );
		}
	}

	for( IDPtr = pTab->GetFirst( CLASS( CTable ) ); _T('\0') != *IDPtr.ID; IDPtr = pTab->GetNext() )
	{
		if( 0 == _tcscmp( IDPtr.ID, _T("WATER_ADDIT") ) )
		{
			// Introduce water at the top.
			if( m_ComboAdditFam.FindStringExact( -1, ( (CTable *)IDPtr.MP )->GetName() ) < 0 )
			{
				m_ComboAdditFam.InsertString( 0, ( (CTable *)IDPtr.MP )->GetName() );
			}
		}
		else if( 0 == _tcscmp( IDPtr.ID, _T("OTHER_ADDIT") ) )
		{
			// Introduce 'Any other fluid' at the bottom.
			if( m_ComboAdditFam.FindStringExact( -1, ( (CTable *)IDPtr.MP )->GetName() ) < 0 )
			{
				m_ComboAdditFam.InsertString( -1, ( (CTable *)IDPtr.MP )->GetName() );
			}
		}
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

void CDlgWaterChar::_FillComboAdditName( CString strAdditName )
{
	m_ComboAdditName.ResetContent();

	if( 0 == m_ComboAdditFam.GetCount() || CB_ERR == m_ComboAdditFam.GetCurSel() )
	{
		return;
	}

	CString str( _T("") );
	m_ComboAdditFam.GetLBText( m_ComboAdditFam.GetCurSel(), str );

	// Find the correct additive family table in table "ADDITIVE_TAB".
	CTable *pTab = (CTable *)( m_pTADB->Get( _T("ADDITIVE_TAB") ).MP );
	ASSERT( NULL != pTab );

	IDPTR IDPtr = pTab->GetFirst( CLASS( CTable ) );
	ASSERT( _T('\0') != *IDPtr.ID );

	while( _T('\0') != *IDPtr.ID && ( (CTable *)IDPtr.MP )->GetName() != str )
	{
		IDPtr = pTab->GetNext();
	}

	ASSERT( _T('\0') != *IDPtr.ID );
	pTab = m_pAdditFamTab = (CTable *)( IDPtr.MP );
	m_mapWaterCharWorking.at( m_eCurrentProjectType ).SetAdditFamID( IDPtr.ID );
	CDB_StringID *pSelectedStrID = NULL;

	// Check which additive family is selected and update other dialog controls.
	if( 0 == _tcscmp( IDPtr.ID, _T("WATER_ADDIT") ) )
	{
		m_ComboAdditName.ShowWindow( SW_HIDE );
		GetDlgItem( IDC_STATICADDITNAME )->ShowWindow( SW_HIDE );

		m_EditDens.SetReadOnly( TRUE );
		m_EditDens.SetBlockSelection( true );
		m_EditDens.SetBlockCursorChange( true );
		m_EditDens.SetTextColor( ::GetSysColor( COLOR_GRAYTEXT ) );
		m_EditDens.SetBackColor( ::GetSysColor( COLOR_3DFACE ) );

		m_EditKinVisc.SetReadOnly( TRUE );
		m_EditKinVisc.SetBlockSelection( true );
		m_EditKinVisc.SetBlockCursorChange( true );
		m_EditKinVisc.SetTextColor( ::GetSysColor( COLOR_GRAYTEXT ) );
		m_EditKinVisc.SetBackColor( ::GetSysColor( COLOR_3DFACE ) );

		m_EditSpecifHeat.SetReadOnly( TRUE );
		m_EditSpecifHeat.SetBlockSelection( true );
		m_EditSpecifHeat.SetBlockCursorChange( true );
		m_EditSpecifHeat.SetTextColor( ::GetSysColor( COLOR_GRAYTEXT ) );
		m_EditSpecifHeat.SetBackColor( ::GetSysColor( COLOR_3DFACE ) );

		m_clCheckSpecifHeat.ShowWindow( SW_HIDE );
		m_mapWaterCharWorking.at( m_eCurrentProjectType ).SetCheckSpecifHeat( false );
		GetDlgItem( IDC_STATICSPECIFHEAT )->ShowWindow( SW_SHOW );
		GetDlgItem( IDC_STATICSPECIFHEATUNIT )->ShowWindow( SW_SHOW );
		GetDlgItem( IDC_EDITSPECIFHEAT )->ShowWindow( SW_SHOW );
		GetDlgItem( IDC_STATICFREEZ )->ShowWindow( SW_SHOW );
		GetDlgItem( IDC_STATICFREEZUNIT )->ShowWindow( SW_SHOW );
		GetDlgItem( IDC_EDITFREEZ )->ShowWindow( SW_SHOW );
		GetDlgItem( IDC_STATICWATERPHASE )->ShowWindow( SW_SHOW );
	}
	else if( 0 == _tcscmp( IDPtr.ID, _T("OTHER_ADDIT") ) )
	{
		m_ComboAdditName.ShowWindow( SW_HIDE );
		GetDlgItem( IDC_STATICADDITNAME )->ShowWindow( SW_HIDE );
		m_EditDens.SetReadOnly( FALSE );
		m_EditDens.SetBlockSelection( false );
		m_EditDens.SetBlockCursorChange( false );
		m_EditDens.SetTextColor( ::GetSysColor( COLOR_WINDOWTEXT ) );
		m_EditDens.SetBackColor( ::GetSysColor( COLOR_WINDOW ) );

		m_EditKinVisc.SetReadOnly( FALSE );
		m_EditKinVisc.SetBlockSelection( false );
		m_EditKinVisc.SetBlockCursorChange( false );
		m_EditKinVisc.SetTextColor( ::GetSysColor( COLOR_WINDOWTEXT ) );
		m_EditKinVisc.SetBackColor( ::GetSysColor( COLOR_WINDOW ) );

		m_EditSpecifHeat.SetReadOnly( FALSE );
		m_EditSpecifHeat.SetBlockSelection( false );
		m_EditSpecifHeat.SetBlockCursorChange( false );
		m_EditSpecifHeat.SetTextColor( ::GetSysColor( COLOR_WINDOWTEXT ) );
		m_EditSpecifHeat.SetBackColor( ::GetSysColor( COLOR_WINDOW ) );

		m_clCheckSpecifHeat.SetCheck( ( true == m_mapWaterCharWorking.at( m_eCurrentProjectType ).GetCheckSpecifHeat() ) ? BST_CHECKED : BST_UNCHECKED );
		m_clCheckSpecifHeat.ShowWindow( SW_SHOW );
		int visib = ( true == m_mapWaterCharWorking.at( m_eCurrentProjectType ).GetCheckSpecifHeat() ) ? SW_SHOW : SW_HIDE;
		GetDlgItem( IDC_STATICSPECIFHEAT )->ShowWindow( visib );
		GetDlgItem( IDC_STATICSPECIFHEATUNIT )->ShowWindow( visib );
		GetDlgItem( IDC_EDITSPECIFHEAT )->ShowWindow( visib );
		GetDlgItem( IDC_STATICFREEZ )->ShowWindow( SW_HIDE );
		GetDlgItem( IDC_STATICFREEZUNIT )->ShowWindow( SW_HIDE );
		GetDlgItem( IDC_EDITFREEZ )->ShowWindow( SW_HIDE );
		GetDlgItem( IDC_STATICWATERPHASE )->ShowWindow( SW_HIDE );
		UpdateWaterBitmap( WP_LIQUID );

	}
	else
	{
		GetDlgItem( IDC_STATICADDITNAME )->ShowWindow( SW_SHOW );
		m_ComboAdditName.ShowWindow( SW_SHOW );

		m_EditDens.SetReadOnly( TRUE );
		m_EditDens.SetBlockSelection( true );
		m_EditDens.SetBlockCursorChange( true );
		m_EditDens.SetTextColor( ::GetSysColor( COLOR_GRAYTEXT ) );
		m_EditDens.SetBackColor( ::GetSysColor( COLOR_3DFACE ) );

		m_EditKinVisc.SetReadOnly( TRUE );
		m_EditKinVisc.SetBlockSelection( true );
		m_EditKinVisc.SetBlockCursorChange( true );
		m_EditKinVisc.SetTextColor( ::GetSysColor( COLOR_GRAYTEXT ) );
		m_EditKinVisc.SetBackColor( ::GetSysColor( COLOR_3DFACE ) );

		m_EditSpecifHeat.SetReadOnly( TRUE );
		m_EditSpecifHeat.SetBlockSelection( true );
		m_EditSpecifHeat.SetBlockCursorChange( true );
		m_EditSpecifHeat.SetTextColor( ::GetSysColor( COLOR_GRAYTEXT ) );
		m_EditSpecifHeat.SetBackColor( ::GetSysColor( COLOR_3DFACE ) );

		m_clCheckSpecifHeat.ShowWindow( SW_HIDE );
		m_mapWaterCharWorking.at( m_eCurrentProjectType ).SetCheckSpecifHeat( false );
		GetDlgItem( IDC_STATICSPECIFHEAT )->ShowWindow( SW_SHOW );
		GetDlgItem( IDC_STATICSPECIFHEATUNIT )->ShowWindow( SW_SHOW );
		GetDlgItem( IDC_EDITSPECIFHEAT )->ShowWindow( SW_SHOW );
		GetDlgItem( IDC_STATICFREEZ )->ShowWindow( SW_SHOW );
		GetDlgItem( IDC_STATICFREEZUNIT )->ShowWindow( SW_SHOW );
		GetDlgItem( IDC_EDITFREEZ )->ShowWindow( SW_SHOW );
		GetDlgItem( IDC_STATICWATERPHASE )->ShowWindow( SW_SHOW );

	}

	std::map<int, CDB_StringID *> mapComboListSorted;
	int iIndex = 0;

	// Fill the additive name combo box for preventing weird behavior.
	for( IDPtr = pTab->GetFirst( CLASS( CDB_StringID ) ); _T('\0') != *IDPtr.ID; IDPtr = pTab->GetNext() )
	{
		CDB_StringID *pID = dynamic_cast<CDB_StringID *>( IDPtr.MP );

		if( NULL == pID )
		{
			continue;
		}

		int iIndexInsert = iIndex++;
		int iIndexFixed = 0;

		if( pID->GetIDstrAsInt( 1, iIndexFixed ) && iIndexFixed > 0 )
		{
			iIndexInsert = iIndexFixed;
		}

		mapComboListSorted.insert( std::pair<int, CDB_StringID *>( iIndexInsert, pID ) );
	}
	
	// HYS-873: 
	if( mapComboListSorted.size() > 0 )
	{
		for( std::map<int, CDB_StringID *>::iterator iter = mapComboListSorted.begin(); iter != mapComboListSorted.end(); ++iter )
		{
			if( m_ComboAdditName.FindStringExact( -1, iter->second->GetString() ) < 0 )
			{
				m_ComboAdditName.InsertString( iter->first -1, iter->second->GetString() );
			}

			if( 0 == strAdditName.Compare( iter->second->GetString() ) )
			{
				pSelectedStrID = iter->second;
			}
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

void CDlgWaterChar::OnCbnSelChangeApplicationType()
{
	CDS_TechnicalParameter *pclTechParams = m_pTADS->GetpTechParams();
	CWaterChar *pWC = NULL;

	m_eCurrentProjectType = (ProjectType)m_ComboApplicationType.GetItemData( m_ComboApplicationType.GetCurSel() );

	// Update temperature and DT.
	m_EditTemp.SetCurrentValSI( m_mapWaterCharWorking.at( m_eCurrentProjectType ).GetTemp() );
	m_EditTemp.Update();
	
	m_EditDefaultDT.SetCurrentValSI( m_mapDTWorking.at( m_eCurrentProjectType ) );
	m_EditDefaultDT.Update();

	// Fill the Additive combo boxes.
	CString pFam = ( (CTable *)m_pTADB->Get( m_mapWaterCharWorking.at( m_eCurrentProjectType ).GetAdditFamID() ).MP )->GetName();
	CString pAddit = ( (CDB_StringID *)m_pTADB->Get( m_mapWaterCharWorking.at( m_eCurrentProjectType ).GetAdditID() ).MP )->GetString();
	_FillComboAdditFam( (LPCTSTR)pFam );
	_FillComboAdditName( pAddit );
	OnCbnSelChangeAdditiveName();
}

void CDlgWaterChar::OnCbnSelChangeAdditiveFamily()
{
	_FillComboAdditName( _T("") );
	m_ComboAdditName.SetCurSel( 0 );
	OnCbnSelChangeAdditiveName();
}

void CDlgWaterChar::OnCbnSelChangeAdditiveName()
{
	if( CB_ERR == m_ComboAdditName.GetCurSel() )
	{
		return;
	}

	// Find the additive characteristic ID for current selection.
	CString str( _T("") );
	m_ComboAdditName.GetLBText( m_ComboAdditName.GetCurSel(), str );
	IDPTR IDPtr = m_pAdditFamTab->GetFirst( CLASS( CDB_StringID ) );
	ASSERT( _T('\0') != *IDPtr.ID );

	while( _T('\0') != *IDPtr.ID && ( (CDB_StringID *)IDPtr.MP )->GetString() != str )
	{
		IDPtr = m_pAdditFamTab->GetNext();
	}

	ASSERT( _T('\0') != *IDPtr.ID );

	m_mapWaterCharWorking.at( m_eCurrentProjectType ).SetAdditID( IDPtr.ID );

	// Get and store the pointer on the additive characteristics.
	CString AdditCharID = ( (CDB_StringID *)IDPtr.MP )->GetIDstr();
	m_pAdditChar = (CDB_AdditCharacteristic *)( m_pTADB->Get( AdditCharID ).MP );
	ASSERT( NULL != m_pAdditChar );

	if( true == m_pAdditChar->IsaDilutedAddit() )
	{
		GetDlgItem( IDC_STATICPCWEIGHT )->ShowWindow( SW_SHOW );
		m_EditPcWeight.SetWindowText( WriteDouble( m_mapWaterCharWorking.at( m_eCurrentProjectType ).GetPcWeight(), 1, 1 ) );
		m_EditPcWeight.ShowWindow( SW_SHOW );
	}
	else
	{
		m_mapWaterCharWorking.at( m_eCurrentProjectType ).SetPcWeight( m_pAdditChar->GetMinPc() );
		m_EditPcWeight.SetWindowText( WriteDouble( m_mapWaterCharWorking.at( m_eCurrentProjectType ).GetPcWeight(), 1, 1 ) );
		m_EditPcWeight.ShowWindow( SW_HIDE );
		GetDlgItem( IDC_STATICPCWEIGHT )->ShowWindow( SW_HIDE );
	}

	UpdateAll();
}

void CDlgWaterChar::_DisplayWarningWithID( bool bActivate, CExtNumEdit *pExtEdit, UINT IDMsg, double dValue, int iUnit )
{
	CString strMsg = _T("");

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

	_DisplayWarningWithString( bActivate, pExtEdit, strMsg, dValue, iUnit );
}

void CDlgWaterChar::_DisplayWarningWithString( bool bActivate, CExtNumEdit *pExtEdit, CString strMsg, double dValue, int iUnit )
{
	if( NULL == pExtEdit || false == bActivate )
	{
		// Restore default.
		m_StaticWarning.SetWindowText( strMsg );
		m_StaticWarning.SetTextColor( _BLACK );
		
		m_clButtonOK.EnableWindow( TRUE );
		m_clButtonOK.ModifyStyle( BS_OWNERDRAW, 0 );
		m_clButtonOK.SetTextColor( _BLACK );
		m_clButtonOK.ResetDrawBorder();

		// Do it only for tools.
		if( DlgWaterCharOrigin::DlgWaterChar_ForTools == m_eOrigin )
		{
			m_ComboApplicationType.EnableWindow( TRUE );
		}
	}
	else
	{
		m_StaticWarning.SetWindowText( strMsg );
		m_StaticWarning.SetTextColor( _RED );

		if( NULL != pExtEdit && iUnit != -1 )
		{
			pExtEdit->SetDrawBorder( true, _RED );
		}

		m_clButtonOK.EnableWindow( FALSE );
		m_clButtonOK.ModifyStyle( 0, BS_OWNERDRAW );
		m_clButtonOK.SetTextColor( _RED );
		m_clButtonOK.SetBackColor( RGB( 204, 204, 204 ) );
		m_clButtonOK.SetDrawBorder( true, _RED );

		// Do it only for tools.
		if( DlgWaterCharOrigin::DlgWaterChar_ForTools == m_eOrigin )
		{
			m_ComboApplicationType.EnableWindow( FALSE );
		}
	}
}
