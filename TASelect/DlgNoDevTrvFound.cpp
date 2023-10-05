#include "stdafx.h"
#include <float.h>
#include <math.h>

#include "TASelect.h"
#include "MainFrm.h"
#include "Select.h"
#include "ProductSelectionParameters.h"
#include "DlgNoDevTrvFound.h"

// REMARK: for the moment, this dialog is only for the normal thermostatic valve and not the insert version.

CDlgNoDevTrvFoundParams::CDlgNoDevTrvFoundParams()
{
	m_pList = NULL;
	m_strFamilyID = _T("");
	m_strConnectID = _T("");
	m_strVersionID = _T("");
}

CDlgNoDevTrvFoundParams::CDlgNoDevTrvFoundParams( CRankEx *pList, CString strFamilyID, CString strConnectID, CString strVersionID )
{
	m_pList = pList;
	m_strFamilyID = strFamilyID;
	m_strConnectID = strConnectID;
	m_strVersionID = strVersionID;
}

CDlgNoDevTrvFoundParams &CDlgNoDevTrvFoundParams::operator=( CDlgNoDevTrvFoundParams &rNoDevTrvFoundParams )
{
	m_pList = rNoDevTrvFoundParams.m_pList;
	m_strFamilyID = rNoDevTrvFoundParams.m_strFamilyID;
	m_strConnectID = rNoDevTrvFoundParams.m_strConnectID;
	m_strVersionID = rNoDevTrvFoundParams.m_strVersionID;
	return *this;
}


CDlgNoDevTrvFound::CDlgNoDevTrvFound( CIndividualSelectionParameters *pclIndividualSelectionParams, CDlgNoDevTrvFoundParams *pclNoDevTrvFoundParams, CWnd *pParent )
	: CDialogEx( CDlgNoDevTrvFound::IDD, pParent )
{
	m_pclIndividualSelectionParams = pclIndividualSelectionParams;

	if( NULL != pclNoDevTrvFoundParams )
	{
		m_clNoDevTrvFoundParams = *pclNoDevTrvFoundParams;
	}
}

BEGIN_MESSAGE_MAP( CDlgNoDevTrvFound, CDialogEx )
	ON_CBN_SELCHANGE( IDC_COMBOFAMILY, OnCbnSelChangeFamily )
	ON_CBN_SELCHANGE( IDC_COMBOCONNECT, OnCbnSelChangeConnect )
	ON_CBN_SELCHANGE( IDC_COMBOVERSION, OnCbnSelChangeVersion )
END_MESSAGE_MAP()

void CDlgNoDevTrvFound::DoDataExchange( CDataExchange* pDX )
{
	CDialogEx::DoDataExchange( pDX );
	DDX_Control( pDX, IDC_COMBOFAMILY, m_ComboFamily );
	DDX_Control( pDX, IDC_COMBOCONNECT, m_ComboConnect );
	DDX_Control( pDX, IDC_COMBOVERSION, m_ComboVersion );
}

BOOL CDlgNoDevTrvFound::OnInitDialog() 
{
	CDialogEx::OnInitDialog();
	
	// Initialize dialog strings.
	CString str;
	str = TASApp.LoadLocalizedString( IDS_DLGNOTRVFOUND_CAPTION );
	SetWindowText( str );
	str = TASApp.LoadLocalizedString( IDS_DLGNOTRVFOUND_STATICNODEV );
	GetDlgItem( IDC_STATICNODEV )->SetWindowText( str );
	str = TASApp.LoadLocalizedString( IDS_DLGNOTRVFOUND_STATICDEVEXIST );
	GetDlgItem( IDC_STATICDEVEXIST )->SetWindowText( str );
	str = TASApp.LoadLocalizedString( IDS_DLGNOTRVFOUND_STATICFAMILY );
	GetDlgItem( IDC_STATICFAMILY )->SetWindowText( str );
	str = TASApp.LoadLocalizedString( IDS_DLGNOTRVFOUND_STATICCONNECT );
	GetDlgItem( IDC_STATICCONNECT )->SetWindowText( str );
	str = TASApp.LoadLocalizedString( IDS_DLGNOTRVFOUND_STATICVERSION );
	GetDlgItem( IDC_STATICVERSION )->SetWindowText( str );
	str = TASApp.LoadLocalizedString( IDS_OK );
	GetDlgItem(IDOK )->SetWindowText( str );
	str = TASApp.LoadLocalizedString( IDS_CANCEL);
	GetDlgItem(IDCANCEL )->SetWindowText( str );
	str.Empty();

	// Fill the combo boxes.
	_FillComboFamily( m_clNoDevTrvFoundParams.m_strFamilyID );
	_FillComboConnect( m_clNoDevTrvFoundParams.m_strConnectID );
	_FillComboVersion( m_clNoDevTrvFoundParams.m_strVersionID );

	return TRUE;
}

void CDlgNoDevTrvFound::OnOK() 
{
	PREVENT_ENTER_KEY
	CDialogEx::OnOK();
}

void CDlgNoDevTrvFound::OnCbnSelChangeFamily()
{
	m_clNoDevTrvFoundParams.m_strFamilyID = m_ComboFamily.GetCBCurSelIDPtr().ID;

	_FillComboConnect();
	OnCbnSelChangeConnect();
}

void CDlgNoDevTrvFound::OnCbnSelChangeConnect() 
{
	m_clNoDevTrvFoundParams.m_strConnectID = m_ComboConnect.GetCBCurSelIDPtr().ID;

	_FillComboVersion();
	OnCbnSelChangeVersion();
}

void CDlgNoDevTrvFound::OnCbnSelChangeVersion() 
{
	m_clNoDevTrvFoundParams.m_strVersionID = m_ComboVersion.GetCBCurSelIDPtr().ID;
}

void CDlgNoDevTrvFound::_FillComboFamily( CString strFamilyID )
{
	if( NULL == m_pclIndividualSelectionParams || NULL == m_pclIndividualSelectionParams->m_pTADB )
	{
		ASSERT_RETURN;
	}

	CRankEx FamilyList;

	m_pclIndividualSelectionParams->m_pTADB->GetTrvFamilyList( m_clNoDevTrvFoundParams.m_pList, &FamilyList );

	m_ComboFamily.FillInCombo( &FamilyList, strFamilyID, IDS_COMBOTEXT_ALL_FAMILIES );

	if( false == m_pclIndividualSelectionParams->m_strComboFamilyID.IsEmpty() && m_ComboFamily.GetCount() > 1 && 0 == m_ComboFamily.GetCurSel() )
	{
		// If there is more than one item in the combo and the current selection is on the "*** All xxx ***" item, 
		// we force the selection to be on the second item.
		m_ComboFamily.SetCurSel( 1 );
	}

	m_clNoDevTrvFoundParams.m_strFamilyID = m_ComboFamily.GetCBCurSelIDPtr().ID;
}

void CDlgNoDevTrvFound::_FillComboConnect( CString strConnectID )
{
	if( NULL == m_pclIndividualSelectionParams || NULL == m_pclIndividualSelectionParams->m_pTADB )
	{
		ASSERT_RETURN;
	}

	CRankEx ConnectList;

	m_pclIndividualSelectionParams->m_pTADB->GetTrvConnectList( m_clNoDevTrvFoundParams.m_pList, &ConnectList, (LPCTSTR)m_clNoDevTrvFoundParams.m_strFamilyID );

	m_ComboConnect.FillInCombo( &ConnectList, strConnectID, IDS_COMBOTEXT_ALL_CONNECTIONS );

	if( false == m_pclIndividualSelectionParams->m_strComboConnectID.IsEmpty() && m_ComboConnect.GetCount() > 1 && 0 == m_ComboConnect.GetCurSel() )
	{
		// If there is more than one item in the combo and the current selection is on the "*** All xxx ***" item, 
		// we force the selection to be on the second item.
		m_ComboConnect.SetCurSel( 1 );
	}

	m_clNoDevTrvFoundParams.m_strConnectID = m_ComboConnect.GetCBCurSelIDPtr().ID;
}

void CDlgNoDevTrvFound::_FillComboVersion( CString strVersionID )
{
	if( NULL == m_pclIndividualSelectionParams || NULL == m_pclIndividualSelectionParams->m_pTADB )
	{
		ASSERT_RETURN;
	}

	CRankEx VersionList;

	m_pclIndividualSelectionParams->m_pTADB->GetTrvVersList( m_clNoDevTrvFoundParams.m_pList, &VersionList, (LPCTSTR)m_clNoDevTrvFoundParams.m_strFamilyID, 
			(LPCTSTR)m_clNoDevTrvFoundParams.m_strConnectID );

	m_ComboVersion.FillInCombo( &VersionList, strVersionID, IDS_COMBOTEXT_ALL_VERSIONS );

	if( false == m_pclIndividualSelectionParams->m_strComboVersionID.IsEmpty() && m_ComboVersion.GetCount() > 1 && 0 == m_ComboVersion.GetCurSel() )
	{
		// If there is more than one item in the combo and the current selection is on the "*** All xxx ***" item, 
		// we force the selection to be on the second item.
		m_ComboVersion.SetCurSel( 1 );
	}

	m_clNoDevTrvFoundParams.m_strVersionID = m_ComboVersion.GetCBCurSelIDPtr().ID;
}
