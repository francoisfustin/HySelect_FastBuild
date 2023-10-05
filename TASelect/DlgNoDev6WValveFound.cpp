#include "stdafx.h"
#include <float.h>
#include <math.h>

#include "TASelect.h"
#include "MainFrm.h"
#include "Select.h"
#include "ProductSelectionParameters.h"
#include "DlgNoDev6WValveFound.h"

// REMARK: for the moment, this dialog is only for the normal thermostatic valve and not the insert version.

CDlgNoDev6WValveFoundParams::CDlgNoDev6WValveFoundParams()
{
	m_pList = NULL;
	m_strConnectID = _T("");
	m_strVersionID = _T("");
}

CDlgNoDev6WValveFoundParams::CDlgNoDev6WValveFoundParams( CRankEx *pList, CString strConnectID, CString strVersionID )
{
	m_pList = pList;
	m_strConnectID = strConnectID;
	m_strVersionID = strVersionID;
}

CDlgNoDev6WValveFoundParams &CDlgNoDev6WValveFoundParams::operator=( CDlgNoDev6WValveFoundParams &rNoDev6WValveFoundParams )
{
	m_pList = rNoDev6WValveFoundParams.m_pList;
	m_strConnectID = rNoDev6WValveFoundParams.m_strConnectID;
	m_strVersionID = rNoDev6WValveFoundParams.m_strVersionID;
	return *this;
}


CDlgNoDev6WValveFound::CDlgNoDev6WValveFound( CIndividualSelectionParameters *pclIndividualSelectionParams, CDlgNoDev6WValveFoundParams *pclNoDev6WValveFoundParams, CWnd *pParent )
	: CDialogEx( CDlgNoDev6WValveFound::IDD, pParent )
{
	m_pclIndividualSelectionParams = pclIndividualSelectionParams;

	if( NULL != pclNoDev6WValveFoundParams )
	{
		m_clNoDev6WValveFoundParams = *pclNoDev6WValveFoundParams;
	}
}

BEGIN_MESSAGE_MAP( CDlgNoDev6WValveFound, CDialogEx )
	ON_CBN_SELCHANGE( IDC_COMBOCONNECT, OnCbnSelChangeConnect )
	ON_CBN_SELCHANGE( IDC_COMBOVERSION, OnCbnSelChangeVersion )
END_MESSAGE_MAP()

void CDlgNoDev6WValveFound::DoDataExchange( CDataExchange* pDX )
{
	CDialogEx::DoDataExchange( pDX );
	DDX_Control( pDX, IDC_COMBOCONNECT, m_ComboConnect );
	DDX_Control( pDX, IDC_COMBOVERSION, m_ComboVersion );
}

BOOL CDlgNoDev6WValveFound::OnInitDialog() 
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
	_FillComboConnect( m_clNoDev6WValveFoundParams.m_strConnectID );
	_FillComboVersion( m_clNoDev6WValveFoundParams.m_strVersionID );

	return TRUE;
}

void CDlgNoDev6WValveFound::OnOK() 
{
	PREVENT_ENTER_KEY
	CDialogEx::OnOK();
}

void CDlgNoDev6WValveFound::OnCbnSelChangeConnect() 
{
	m_clNoDev6WValveFoundParams.m_strConnectID = m_ComboConnect.GetCBCurSelIDPtr().ID;

	_FillComboVersion();
	OnCbnSelChangeVersion();
}

void CDlgNoDev6WValveFound::OnCbnSelChangeVersion() 
{
	m_clNoDev6WValveFoundParams.m_strVersionID = m_ComboVersion.GetCBCurSelIDPtr().ID;
}

void CDlgNoDev6WValveFound::_FillComboConnect( CString strConnectID )
{
	if( NULL == m_pclIndividualSelectionParams || NULL == m_pclIndividualSelectionParams->m_pTADB )
	{
		ASSERT_RETURN;
	}

	CRankEx ConnectList;

	m_pclIndividualSelectionParams->m_pTADB->Get6WayValveConnectionList( m_clNoDev6WValveFoundParams.m_pList, &ConnectList );

	m_ComboConnect.FillInCombo( &ConnectList, strConnectID, IDS_COMBOTEXT_ALL_CONNECTIONS );

	if( false == m_pclIndividualSelectionParams->m_strComboConnectID.IsEmpty() && m_ComboConnect.GetCount() > 1 && 0 == m_ComboConnect.GetCurSel() )
	{
		// If there is more than one item in the combo and the current selection is on the "*** All xxx ***" item, 
		// we force the selection to be on the second item.
		m_ComboConnect.SetCurSel( 1 );
	}

	m_clNoDev6WValveFoundParams.m_strConnectID = m_ComboConnect.GetCBCurSelIDPtr().ID;
}

void CDlgNoDev6WValveFound::_FillComboVersion( CString strVersionID )
{
	if( NULL == m_pclIndividualSelectionParams || NULL == m_pclIndividualSelectionParams->m_pTADB )
	{
		ASSERT_RETURN;
	}

	CRankEx VersionList;

	m_pclIndividualSelectionParams->m_pTADB->Get6WayValveVersList( m_clNoDev6WValveFoundParams.m_pList, &VersionList, (LPCTSTR)m_clNoDev6WValveFoundParams.m_strConnectID );

	m_ComboVersion.FillInCombo( &VersionList, strVersionID, IDS_COMBOTEXT_ALL_VERSIONS );

	if( false == m_pclIndividualSelectionParams->m_strComboVersionID.IsEmpty() && m_ComboVersion.GetCount() > 1 && 0 == m_ComboVersion.GetCurSel() )
	{
		// If there is more than one item in the combo and the current selection is on the "*** All xxx ***" item, 
		// we force the selection to be on the second item.
		m_ComboVersion.SetCurSel( 1 );
	}

	m_clNoDev6WValveFoundParams.m_strVersionID = m_ComboVersion.GetCBCurSelIDPtr().ID;
}
