#include "stdafx.h"

#include "TASelect.h"
#include "MainFrm.h"
#include "TASelectDoc.h"
#include "Select.h"
#include "DlgNoDevSafetyValveFound.h"
#include "ProductSelectionParameters.h"

CDlgNoDevSafetyValveFoundParams::CDlgNoDevSafetyValveFoundParams()
{
	m_pList = NULL;
	m_eSystemType = ProjectType::InvalidProjectType;
	m_strSystemHeatGeneratorTypeID = _T("");
	m_strNormID = _T( "" );
	m_strSafetyValveFamilyID = _T("");
	m_strSafetyValveConnectID = _T("");
	m_dSafetyValveSetPressure = 0.0;
}

CDlgNoDevSafetyValveFoundParams::CDlgNoDevSafetyValveFoundParams( CRankEx *pList, ProjectType eSystemType, CString strSystemHeatGeneratorTypeID, 
		CString strNormID, CString strSafetyValveFamilyID, CString strSafetyValveConnectID, double dSafetyValveSetPressure )
{
	m_pList = pList;
	m_eSystemType = eSystemType;
	m_strSystemHeatGeneratorTypeID = strSystemHeatGeneratorTypeID;
	m_strNormID = strNormID;
	m_strSafetyValveFamilyID = strSafetyValveFamilyID;
	m_strSafetyValveConnectID = strSafetyValveConnectID;
	m_dSafetyValveSetPressure =dSafetyValveSetPressure;
}

CDlgNoDevSafetyValveFoundParams &CDlgNoDevSafetyValveFoundParams::operator=( CDlgNoDevSafetyValveFoundParams &rNoDevSeparatorFoundParams )
{
	m_pList = rNoDevSeparatorFoundParams.m_pList;
	m_eSystemType = rNoDevSeparatorFoundParams.m_eSystemType;
	m_strSystemHeatGeneratorTypeID = rNoDevSeparatorFoundParams.m_strSystemHeatGeneratorTypeID;
	m_strNormID = rNoDevSeparatorFoundParams.m_strNormID;
	m_strSafetyValveFamilyID = rNoDevSeparatorFoundParams.m_strSafetyValveFamilyID;
	m_strSafetyValveConnectID = rNoDevSeparatorFoundParams.m_strSafetyValveConnectID;
	m_dSafetyValveSetPressure = rNoDevSeparatorFoundParams.m_dSafetyValveSetPressure;
	return *this;
}

CDlgNoDevSafetyValveFound::CDlgNoDevSafetyValveFound( CProductSelelectionParameters *pclProductSelParams, 
		CDlgNoDevSafetyValveFoundParams *pclNoDevSafetyValveFoundParams, CWnd *pParent )
	: CDialogEx( CDlgNoDevSafetyValveFound::IDD, pParent )
{
	m_pclProductSelParams = pclProductSelParams;

	if( NULL != pclNoDevSafetyValveFoundParams )
	{
		m_clNoDevSafetyValveFoundParams = *pclNoDevSafetyValveFoundParams;
	}
}

BEGIN_MESSAGE_MAP( CDlgNoDevSafetyValveFound, CDialogEx )
	ON_CBN_SELCHANGE( IDC_COMBO_SYSTEMTYPE, OnCbnSelChangeSystemType )
	ON_CBN_SELCHANGE( IDC_COMBO_SYSTEMHEATGENERATORTYPE, OnCbnSelChangeSystemHeatGeneratorTypeID )
	ON_CBN_SELCHANGE( IDC_COMBO_NORM, OnCbnSelChangeNormID )
	ON_CBN_SELCHANGE( IDC_COMBO_FAMILY, OnCbnSelChangeSafetyValveFamilyID )
	ON_CBN_SELCHANGE( IDC_COMBO_CONNECT, OnCbnSelChangeSafetyValveConnectID )
	ON_CBN_SELCHANGE( IDC_COMBO_SETPRESSURE, OnCbnSelChangeSafetyValveSetPressure )
END_MESSAGE_MAP()

void CDlgNoDevSafetyValveFound::DoDataExchange( CDataExchange *pDX )
{
	CDialogEx::DoDataExchange( pDX );
	DDX_Control( pDX, IDC_GROUPSYSTEM, m_clGroupSystem );
	DDX_Control( pDX, IDC_GROUPNORM, m_clGroupNorm );
	DDX_Control( pDX, IDC_GROUPSAFETYVALVE, m_clGroupSafetyValve );
	DDX_Control( pDX, IDC_COMBO_SYSTEMTYPE, m_ComboSystemType );
	DDX_Control( pDX, IDC_COMBO_SYSTEMHEATGENERATORTYPE, m_ComboSystemHeatGeneratorType );
	DDX_Control( pDX, IDC_COMBO_NORM, m_ComboNorm );
	DDX_Control( pDX, IDC_COMBO_FAMILY, m_ComboSafetyValveFamily );
	DDX_Control( pDX, IDC_COMBO_CONNECT, m_ComboSafetyValveConnect );
	DDX_Control( pDX, IDC_COMBO_SETPRESSURE, m_ComboSafetyValveSetPressure );
}

BOOL CDlgNoDevSafetyValveFound::OnInitDialog() 
{
	CDialogEx::OnInitDialog();
	
	// Initialize dialog strings.
	CString str;
	str = TASApp.LoadLocalizedString( IDS_DLGNODEVFOUND_CAPTION );
	SetWindowText( str );
	
	str = TASApp.LoadLocalizedString( IDS_DLGNODEVFOUND_STATICNODEV );
	GetDlgItem( IDC_STATICNODEV )->SetWindowText( str );
	
	str = TASApp.LoadLocalizedString( IDS_DLGNODEVFOUND_STATICDEVEXIST );
	GetDlgItem( IDC_STATICDEVEXIST )->SetWindowText( str );
	
	// 'System' group.
	str = TASApp.LoadLocalizedString( IDS_DLGNODEVSAFETYVALVEFOUND_STATICGROUPSYSTEM );
	GetDlgItem( IDC_GROUPSYSTEM )->SetWindowText( str );

	str = TASApp.LoadLocalizedString( IDS_DLGNODEVSAFETYVALVEFOUND_STATICSYSTEMTYPE );
	GetDlgItem( IDC_STATICSYSTEMTYPE )->SetWindowText( str );

	str = TASApp.LoadLocalizedString( IDS_DLGNODEVSAFETYVALVEFOUND_STATICSYSTEMHEATGENERATORTYPE );
	GetDlgItem( IDC_STATICHEATGENERATORTYPE )->SetWindowText( str );

	// 'Norm' group.
	str = TASApp.LoadLocalizedString( IDS_DLGNODEVSAFETYVALVEFOUND_STATICGROUPNORM );
	GetDlgItem( IDC_GROUPNORM )->SetWindowText( str );

	// 'Safety valve' group.
	str = TASApp.LoadLocalizedString( IDS_DLGNODEVSAFETYVALVEFOUND_STATICSAFETYVALVEFAMILY );
	GetDlgItem( IDC_STATICSAFETYVALVEFAMILY )->SetWindowText( str );

	str = TASApp.LoadLocalizedString( IDS_DLGNODEVSAFETYVALVEFOUND_STATICSAFETYVALVECONNECT );
	GetDlgItem( IDC_STATICSAFETYVALVECONNECT )->SetWindowText( str );

	str = TASApp.LoadLocalizedString( IDS_DLGNODEVSAFETYVALVEFOUND_STATICSAFETYVALVESETPRESSURE );
	GetDlgItem( IDC_STATICSAFETYVALVESETPRESSURE )->SetWindowText( str );

	str = TASApp.LoadLocalizedString( IDS_OK );
	GetDlgItem( IDOK )->SetWindowText( str );

	str = TASApp.LoadLocalizedString( IDS_CANCEL );
	GetDlgItem( IDCANCEL )->SetWindowText( str );
	str.Empty();

	CImageList *pclImgListGroupBox = TASApp.GetpRCImageManager()->GetImageList( CRCImageManager::ILN_GroupBox );
	ASSERT( NULL != pclImgListGroupBox );

	if( NULL != pclImgListGroupBox )
	{
		m_clGroupSafetyValve.SetImageList( pclImgListGroupBox, CRCImageManager::ILGB_SafetyValve );
	}

	CImageList *pclImgListGroupBoxGrayed = TASApp.GetpRCImageManager()->GetImageList( CRCImageManager::ILN_GroupBoxGrayed );
	ASSERT( NULL != pclImgListGroupBoxGrayed );

	if( NULL != pclImgListGroupBoxGrayed )
	{
		m_clGroupSafetyValve.SetImageList( pclImgListGroupBoxGrayed, CRCImageManager::ILGBG_SafetyValve, true );
	}

	// Update layout before to fill.
	_UpdateLayout();

	// Fill the combo boxes.
	_FillComboSystemType( m_clNoDevSafetyValveFoundParams.m_eSystemType );
	_FillComboSystemHeatGeneratorTypeID( m_clNoDevSafetyValveFoundParams.m_strSystemHeatGeneratorTypeID );
	_FillComboNormID( m_clNoDevSafetyValveFoundParams.m_strNormID );
	_FillComboSafetyValveFamilyID( m_clNoDevSafetyValveFoundParams.m_strSafetyValveFamilyID );
	_FillComboSafetyValveConnectID( m_clNoDevSafetyValveFoundParams.m_strSafetyValveConnectID );
	_FillComboSafetyValveSetPressure( m_clNoDevSafetyValveFoundParams.m_dSafetyValveSetPressure );
	
	return TRUE; 
}

void CDlgNoDevSafetyValveFound::OnOK() 
{
	PREVENT_ENTER_KEY
	CDialogEx::OnOK();
}

void CDlgNoDevSafetyValveFound::OnCbnSelChangeSystemType()
{
	m_clNoDevSafetyValveFoundParams.m_eSystemType = _GetSytemType();
	_UpdateLayout();

	if( Heating == m_clNoDevSafetyValveFoundParams.m_eSystemType )
	{
		_FillComboSystemHeatGeneratorTypeID();
		OnCbnSelChangeSystemHeatGeneratorTypeID();
	}
	else
	{
		_FillComboNormID();
		OnCbnSelChangeNormID();
	}
}

void CDlgNoDevSafetyValveFound::OnCbnSelChangeSystemHeatGeneratorTypeID()
{
	m_clNoDevSafetyValveFoundParams.m_strSystemHeatGeneratorTypeID = m_ComboSystemHeatGeneratorType.GetCBCurSelIDPtr().ID;

	_FillComboNormID();
	OnCbnSelChangeNormID();
}

void CDlgNoDevSafetyValveFound::OnCbnSelChangeNormID()
{
	m_clNoDevSafetyValveFoundParams.m_strNormID = _GetNormID();
	_FillComboSafetyValveFamilyID();
	OnCbnSelChangeSafetyValveFamilyID();
}

void CDlgNoDevSafetyValveFound::OnCbnSelChangeSafetyValveFamilyID()
{
	m_clNoDevSafetyValveFoundParams.m_strSafetyValveFamilyID = m_ComboSafetyValveFamily.GetCBCurSelIDPtr().ID;

	_FillComboSafetyValveConnectID();
	OnCbnSelChangeSafetyValveConnectID();
}

void CDlgNoDevSafetyValveFound::OnCbnSelChangeSafetyValveConnectID()
{
	m_clNoDevSafetyValveFoundParams.m_strSafetyValveConnectID = m_ComboSafetyValveConnect.GetCBCurSelIDPtr().ID;

	_FillComboSafetyValveSetPressure();
	OnCbnSelChangeSafetyValveSetPressure();
}

void CDlgNoDevSafetyValveFound::OnCbnSelChangeSafetyValveSetPressure()
{
	m_clNoDevSafetyValveFoundParams.m_dSafetyValveSetPressure = _GetSafetyValveSetPressure();
}

void CDlgNoDevSafetyValveFound::_FillComboSystemType( ProjectType eApplicationType )
{
	int iSel = 0;
	m_ComboSystemType.InsertString( 0, TASApp.LoadLocalizedString( IDS_RBN_APPLITYPE_HEATING ) );
	m_ComboSystemType.SetItemData( 0, ProjectType::Heating );

	m_ComboSystemType.InsertString( 1, TASApp.LoadLocalizedString( IDS_RBN_APPLITYPE_COOLING ) );
	m_ComboSystemType.SetItemData( 1, ProjectType::Cooling );

	if( ProjectType::Cooling == eApplicationType )
	{
		iSel = 1;
	}

	m_ComboSystemType.InsertString( 2, TASApp.LoadLocalizedString( IDS_RBN_APPLITYPE_SOLAR ) );
	m_ComboSystemType.SetItemData( 2, ProjectType::Solar );

	if( ProjectType::Solar == eApplicationType )
	{
		iSel = 2;
	}

	m_ComboSystemType.SetCurSel( iSel );
}

void CDlgNoDevSafetyValveFound::_FillComboSystemHeatGeneratorTypeID( CString strSystemHeatGeneratorTypeID )
{
	if( NULL == m_pclProductSelParams || NULL == m_pclProductSelParams->m_pTADB )
	{
		ASSERT_RETURN;
	}

	CRankEx rkList;
	_string str;
	DWORD_PTR dwSelItem = 0;
	CTable *pTab = (CTable *)( m_pclProductSelParams->m_pTADB->Get( _T("SVHEATGENTYPE_TAB") ).MP );

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

		if( strSystemHeatGeneratorTypeID == idptr.ID )
		{
			dwSelItem = (LPARAM)idptr.MP;
		}

		rkList.Add( str, i, (LPARAM)idptr.MP );
	}

	rkList.Transfer( &m_ComboSystemHeatGeneratorType );
	int iSel = m_ComboSystemHeatGeneratorType.FindItemDataPtr( dwSelItem );

	if( -1 == iSel )
	{
		iSel = 0;
	}

	m_ComboSystemHeatGeneratorType.SetCurSel( iSel );
}

void CDlgNoDevSafetyValveFound::_FillComboNormID( CString strNormID )
{
	if( NULL == m_pclProductSelParams || NULL == m_pclProductSelParams->m_pTADB )
	{
		ASSERT_RETURN;
	}

	CRankEx rkList;
	_string str;
	DWORD_PTR dwSelItem = 0;
	CTable *pTab = (CTable *)( m_pclProductSelParams->m_pTADB->Get( _T("PRESSMAINTNORM_TAB") ).MP );

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

		// I found no way for the moment to not hard code the norm in regard to the system type.
		bool bAdd = false;

		if( 0 == CString( idptr.ID ).Compare( _T("PM_NORM_NONE") ) )
		{
			bAdd = true;
		}
		else if( 0 == CString( idptr.ID ).Compare( _T("PM_NORM_EN12828") )
			&& Cooling == m_clNoDevSafetyValveFoundParams.m_eSystemType )
		{
			bAdd = true;
		}

		if( false == bAdd )
		{
			continue;
		}

		if( strNormID == idptr.ID )
		{
			dwSelItem = (LPARAM)idptr.MP;
		}

		rkList.Add( str, i, (LPARAM)idptr.MP );
	}

	rkList.Transfer( &m_ComboNorm );
	int iSel = m_ComboNorm.FindItemDataPtr( dwSelItem );

	if( -1 == iSel )
	{
		iSel = 0;
	}

	m_ComboNorm.SetCurSel( iSel );

	if( m_ComboNorm.GetCount() <= 1 )
	{
		m_ComboNorm.EnableWindow( FALSE );
	}
	else
	{
		m_ComboNorm.EnableWindow( TRUE );
	}
}

void CDlgNoDevSafetyValveFound::_FillComboSafetyValveFamilyID( CString strSafetyValveFamilyID )
{
	if( NULL == m_pclProductSelParams || NULL == m_pclProductSelParams->m_pTADB )
	{
		ASSERT_RETURN;
	}

	CRankEx SafetyValveFamilyList;

	m_pclProductSelParams->m_pTADB->GetSafetyValveFamilyList( m_clNoDevSafetyValveFoundParams.m_pList, &SafetyValveFamilyList, 
			m_clNoDevSafetyValveFoundParams.m_eSystemType );

	m_ComboSafetyValveFamily.FillInCombo( &SafetyValveFamilyList, strSafetyValveFamilyID, IDS_COMBOTEXT_ALL_TYPES );
	m_clNoDevSafetyValveFoundParams.m_strSafetyValveFamilyID = m_ComboSafetyValveFamily.GetCBCurSelIDPtr().ID;
}

void CDlgNoDevSafetyValveFound::_FillComboSafetyValveConnectID( CString strSafetyValveConnectID )
{
	if( NULL == m_pclProductSelParams || NULL == m_pclProductSelParams->m_pTADB )
	{
		ASSERT_RETURN;
	}

	CRankEx SafetyValveConnectList;

	m_pclProductSelParams->m_pTADB->GetSafetyValveConnList( m_clNoDevSafetyValveFoundParams.m_pList, &SafetyValveConnectList, 
			m_clNoDevSafetyValveFoundParams.m_eSystemType, (LPCTSTR)m_clNoDevSafetyValveFoundParams.m_strSafetyValveFamilyID );

	m_ComboSafetyValveConnect.FillInCombo( &SafetyValveConnectList, strSafetyValveConnectID, IDS_COMBOTEXT_ALL_CONNECTIONS );
	m_clNoDevSafetyValveFoundParams.m_strSafetyValveConnectID = m_ComboSafetyValveConnect.GetCBCurSelIDPtr().ID;
}

void CDlgNoDevSafetyValveFound::_FillComboSafetyValveSetPressure( double dSetPressure )
{
	if( NULL == m_pclProductSelParams || NULL == m_pclProductSelParams->m_pTADB )
	{
		ASSERT_RETURN;
	}

	CRankEx SafetyValveSetPressureList;

	m_pclProductSelParams->m_pTADB->GetSafetyValveSetPressureList( m_clNoDevSafetyValveFoundParams.m_pList, &SafetyValveSetPressureList, 
			m_clNoDevSafetyValveFoundParams.m_eSystemType, (LPCTSTR)m_clNoDevSafetyValveFoundParams.m_strSafetyValveFamilyID, 
			(LPCTSTR)m_clNoDevSafetyValveFoundParams.m_strSafetyValveConnectID );

	m_ComboSafetyValveSetPressure.ResetContent();

	std::wstring str;
	LPARAM lpParam = 0;
	int iSelPos = 0;
	int iCount = 0;

	for( bool bContinue = SafetyValveSetPressureList.GetFirst( str, lpParam ); true == bContinue; bContinue = SafetyValveSetPressureList.GetNext( str, lpParam ) )
	{
		int iItem = m_ComboSafetyValveSetPressure.AddString( str.c_str() );
		m_ComboSafetyValveSetPressure.SetItemData( iItem, lpParam );

		if( dSetPressure == (double)lpParam )
		{
			iSelPos = iCount;
		}

		iCount++;
	}

	m_ComboSafetyValveSetPressure.SetCurSel( iSelPos );

	if( m_ComboSafetyValveSetPressure.GetCount() <= 1 )
	{
		m_ComboSafetyValveSetPressure.EnableWindow( FALSE );
	}
	else
	{
		m_ComboSafetyValveSetPressure.EnableWindow( TRUE );
	}

	m_clNoDevSafetyValveFoundParams.m_dSafetyValveSetPressure = _GetSafetyValveSetPressure();
}

void CDlgNoDevSafetyValveFound::_UpdateLayout()
{
	if( Heating == m_clNoDevSafetyValveFoundParams.m_eSystemType )
	{
		GetDlgItem( IDC_STATICHEATGENERATORTYPE )->EnableWindow( TRUE );
		m_ComboSystemHeatGeneratorType.EnableWindow( TRUE );

		GetDlgItem( IDC_COMBO_NORM )->EnableWindow( FALSE );
	}
	else if( Cooling == m_clNoDevSafetyValveFoundParams.m_eSystemType )
	{
		GetDlgItem( IDC_STATICHEATGENERATORTYPE )->EnableWindow( FALSE );
		m_ComboSystemHeatGeneratorType.EnableWindow( FALSE );

		GetDlgItem( IDC_COMBO_NORM )->EnableWindow( TRUE );
	}
	else if( Solar == m_clNoDevSafetyValveFoundParams.m_eSystemType )
	{
		GetDlgItem( IDC_STATICHEATGENERATORTYPE )->EnableWindow( FALSE );
		m_ComboSystemHeatGeneratorType.EnableWindow( FALSE );

		GetDlgItem( IDC_COMBO_NORM )->EnableWindow( FALSE );
	}
}

ProjectType CDlgNoDevSafetyValveFound::_GetSytemType()
{
	ProjectType eProjectType = Heating;
	int iCur = m_ComboSystemType.GetCurSel();

	if( iCur >= 0 )
	{
        eProjectType = (ProjectType)m_ComboSystemType.GetItemData( iCur );
	}

	return eProjectType;
}

CString CDlgNoDevSafetyValveFound::_GetNormID()
{
	CString strNormID = _T("PM_NONE");
	int iCur = m_ComboNorm.GetCurSel();

	if( iCur >= 0 )
	{
		CDB_StringID *pStrID = NULL;
		pStrID = dynamic_cast<CDB_StringID *>( (CData *)m_ComboNorm.GetItemData( iCur ) );

		if( NULL != pStrID )
		{
			strNormID = pStrID->GetIDPtr().ID;
		}
	}

	return strNormID;
}

double CDlgNoDevSafetyValveFound::_GetSafetyValveSetPressure()
{
	double dSetPressure = 0.0;

	int iCur = m_ComboSafetyValveSetPressure.GetCurSel();

	if( iCur >= 0 )
	{
		dSetPressure = (double)m_ComboSafetyValveSetPressure.GetItemData( iCur );
	}

	return dSetPressure;
}
