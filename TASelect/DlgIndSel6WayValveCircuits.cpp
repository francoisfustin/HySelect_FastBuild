#include "stdafx.h"
#include "MainFrm.h"
#include "Global.h"
#include "Hydronic.h"
#include "Select.h"

#include "DlgLTtabctrl.h"
#include "DlgLeftTabSelManager.h"
#include "ProductSelectionParameters.h"
#include "DlgIndSel6WayValveCircuits.h"


CDlgIndSel6WayValveCircuits::CDlgIndSel6WayValveCircuits( bool *p_bIsModified, e6WayValveSelectionMode eSelectionMode, CWnd *pParent )
	: CDialogEx( CDlgIndSel6WayValveCircuits::IDD, pParent )
{
	m_pbIsModified = p_bIsModified;
	m_eCurrentSelectionMode = eSelectionMode;
	m_eSavedSelectionMode = eSelectionMode;
	// HYS-1429
	m_bCanEQMEnabled = false;
	m_bCanOnoffPIBCVEnabled = false;
	m_bCanOnoffBVEnabled = false;
	m_bCan6WayValveEnabled = false;
}

CDlgIndSel6WayValveCircuits::~CDlgIndSel6WayValveCircuits()
{
}

void CDlgIndSel6WayValveCircuits::DoDataExchange( CDataExchange *pDX )
{
	CDialogEx::DoDataExchange( pDX );
	DDX_Control( pDX, IDOK, m_ButtonOk );
	DDX_Control( pDX, IDCANCEL, m_ButtonCancel );
	DDX_Control( pDX, IDC_STATIC_TA, m_StaticTextA );
	DDX_Control( pDX, IDC_STATIC_TB, m_StaticTextB );
	DDX_Control( pDX, IDC_STATIC_TC, m_StaticTextC );
	DDX_Control( pDX, IDC_STATIC_TD, m_StaticTextD );
	DDX_Control( pDX, IDC_BUTTON_A, m_ButtonA );
	DDX_Control( pDX, IDC_BUTTON_B, m_ButtonB );
	DDX_Control( pDX, IDC_BUTTON_C, m_ButtonC );
	DDX_Control( pDX, IDC_BUTTON_D, m_ButtonD );
}

BOOL CDlgIndSel6WayValveCircuits::OnInitDialog()
{
	CDialogEx::OnInitDialog();
	// Dialog title
	// HYS-1429 : We get all available TA-6way-valve and look if all mode can have solution. For example is US version
	// only actuator M106-CO is available taht's why only EQM will have a solution.
	CRankEx List;
	_string strLoop = CteEMPTY_STRING;
	CDB_6WayValve* pclProduct = NULL;
	// Get the list of TA-6way-valve
	GetpTADB()->Get6WayValveList( &List );
	
	bool bAtLeastOneEQM = false;
	bool bAtLeastOneOnOffPibcv = false;
	bool bAtLeastOneOnOffbv = false;
	bool bAtLeastOne6WayValve = false;
	
	for( BOOL bContinue = List.GetFirstT<CDB_6WayValve*>( strLoop, pclProduct ); TRUE == bContinue;
		bContinue = List.GetNextT<CDB_6WayValve*>( strLoop, pclProduct ) )
	{
		if( NULL == pclProduct )
		{
			continue;
		}

		if( false == pclProduct->IsSelectable( true ) )
		{
			continue;
		}
		// At least one TA-6way valve could be selected
		bAtLeastOne6WayValve = true;

		// Has the 6-way valve a PIBCV table or BV table ?
		bool bIsAtLeastOnePIBCV = false;
		bool bIsAtLeastOneBV = false;
		CTable* pclPIBCVGroup = (CTable*)( pclProduct->GetPicvGroupTableIDPtr().MP );
		if( NULL != pclPIBCVGroup )
		{
			for( IDPTR idptr = pclPIBCVGroup->GetFirst(); NULL != idptr.MP; idptr = pclPIBCVGroup->GetNext( idptr.MP ) )
			{
				CDB_PIControlValve* pclPIControlValve = dynamic_cast<CDB_PIControlValve*>( idptr.MP );
				if( NULL == pclPIControlValve )
				{
					continue;
				}
				if( false == pclPIControlValve->IsSelectable( true ) )
				{
					continue;
				}

				if( NULL != pclPIControlValve )
				{
					bIsAtLeastOnePIBCV = true;
					break;
				}
			}
		}

		CTable* pclBVGroup = (CTable*)( pclProduct->GetBvFamiliesGroupTableIDPtr().MP );
		if( NULL != pclBVGroup )
		{
			for( IDPTR idptr = pclBVGroup->GetFirst(); NULL != idptr.MP; idptr = pclBVGroup->GetNext( idptr.MP ) )
			{
				CRankEx ValveList;
				// Get all bv with this family ID
				TASApp.GetpTADB()->GetBVList( &ValveList, _T( "" ), idptr.ID, _T( "" ), _T( "" ), _T( "" ) );

				if( 0 < ValveList.GetCount() )
				{
					bIsAtLeastOneBV = true;
					break;
				}
			}
		}

		// Retrieve the actuator group on the selected control valve.
		CTable* pclActuatorGroup = (CTable*)( pclProduct->GetActuatorGroupIDPtr().MP );

		if( NULL == pclActuatorGroup )
		{
			continue;
		}

		// Retrieve list of all actuators in this group.
		CRank ActList( false );

		for( IDPTR idptr = pclActuatorGroup->GetFirst(); NULL != idptr.MP; idptr = pclActuatorGroup->GetNext( idptr.MP ) )
		{
			ActList.Add( idptr.ID, 0, (LPARAM)idptr.MP );
		}

		if( 0 >= ActList.GetCount() )
		{
			continue;
		}
		
		// Look actuator availability	
		CDB_ElectroActuator* pclActuator = NULL;
		CString strLoopAct = CteEMPTY_STRING;
		for( BOOL bLoop = ActList.GetFirstT<CDB_ElectroActuator*>( strLoopAct, pclActuator ); TRUE == bLoop;
			bLoop = ActList.GetNextT<CDB_ElectroActuator*>( strLoopAct, pclActuator ) )
		{
			if( NULL == pclActuator )
			{
				continue;
			}

			if( false == pclActuator->IsSelectable(true) )
			{
				continue;
			}

			if( ( bIsAtLeastOnePIBCV == true ) && ( 0 == CString( pclActuator->GetVersionID() ).CompareNoCase( _T( "VERS_CO" ) ) ) )
			{
				bAtLeastOneEQM = true;
			}
			else if( bIsAtLeastOnePIBCV == true )
			{
				bAtLeastOneEQM = true;
				bAtLeastOneOnOffPibcv = true;
			}
			if( ( bIsAtLeastOneBV == true ) && ( 0 != CString( pclActuator->GetVersionID() ).CompareNoCase( _T( "VERS_CO" ) ) ) )
			{
				bAtLeastOneOnOffbv = true;
			}

			if( true == bAtLeastOneOnOffbv && true == bAtLeastOneOnOffPibcv && true == bAtLeastOneEQM )
			{
				break;
			}
		}
		if( true == bAtLeastOneOnOffbv && true == bAtLeastOneOnOffPibcv && true == bAtLeastOneEQM )
		{
			break;
		}
	}
	m_bCanEQMEnabled = bAtLeastOneEQM;
	m_bCanOnoffPIBCVEnabled = bAtLeastOneOnOffPibcv;
	m_bCanOnoffBVEnabled = bAtLeastOneOnOffbv;
	m_bCan6WayValveEnabled = bAtLeastOne6WayValve;

	CString str = CteEMPTY_STRING;
	str = TASApp.LoadLocalizedString( IDS_DLG6WAYVALVECIRCUIT_TITLE );
	SetWindowText( str );
	m_ButtonOk.EnableWindow();
	str = TASApp.LoadLocalizedString( IDS_DLG6WAYVALVECIRCUIT_OKBUTTON );
	m_ButtonOk.SetWindowText( str );
	m_ButtonCancel.EnableWindow();
	str = TASApp.LoadLocalizedString( IDS_DLG6WAYVALVECIRCUIT_CANCELBUTTON );
	m_ButtonCancel.SetWindowText( str );

	m_BmpA.LoadBitmap( IDB_CIRCUIT6W_A );
	m_ButtonA.SetBitmap( (HBITMAP)m_BmpA.GetSafeHandle() );

	m_BmpB.LoadBitmap( IDB_CIRCUIT6W_B );
	m_ButtonB.SetBitmap( (HBITMAP)m_BmpB.GetSafeHandle() );

	m_BmpC.LoadBitmap( IDB_CIRCUIT6W_C );
	m_ButtonC.SetBitmap( (HBITMAP)m_BmpC.GetSafeHandle() );

	m_BmpD.LoadBitmap( IDB_CIRCUIT6W_D );
	m_ButtonD.SetBitmap( (HBITMAP)m_BmpD.GetSafeHandle() );

	m_StaticTextA.SetTextAndToolTip( TASApp.LoadLocalizedString( IDS_DLGINDSEL6WAYVALVE_6WAYEQM ) );
	m_StaticTextB.SetTextAndToolTip( TASApp.LoadLocalizedString( IDS_DLGINDSEL6WAYVALVE_6WAYONOFFPICV ) );
	m_StaticTextC.SetTextAndToolTip( TASApp.LoadLocalizedString( IDS_DLGINDSEL6WAYVALVE_6WAYONOFFSTAD ) );
	m_StaticTextD.SetTextAndToolTip( TASApp.LoadLocalizedString( IDS_DLGINDSEL6WAYVALVE_6WAYALONE ) );

	m_ButtonA.ResetDrawBorder();
	m_ButtonB.ResetDrawBorder();
	m_ButtonC.ResetDrawBorder();
	m_ButtonD.ResetDrawBorder();


	if( NULL != GetDlgCircuitFocus() )
	{
		GetDlgCircuitFocus()->SetFocus();
		GetDlgCircuitFocus()->SetButtonStyle( BS_DEFPUSHBUTTON );
		GetDlgCircuitFocus()->SetDrawBorder( true, _SELECTEDBLUE );
		
	}
	else
	{
		m_ButtonOk.EnableWindow( false );
	}

	// HYS-1429 : If an image will not have a solution, disable it.
	if( false == m_bCanEQMEnabled )
	{
		m_ButtonA.EnableWindow( FALSE );
		m_StaticTextA.EnableWindow( FALSE );
	}
	if( false == m_bCanOnoffPIBCVEnabled )
	{
		m_ButtonB.EnableWindow( FALSE );
		m_StaticTextB.EnableWindow( FALSE );
	}
	if( false == m_bCanOnoffBVEnabled )
	{
		m_ButtonC.EnableWindow( FALSE );
		m_StaticTextC.EnableWindow( FALSE );
	}
	if( false == m_bCan6WayValveEnabled )
	{
		m_ButtonD.EnableWindow( FALSE );
		m_StaticTextD.EnableWindow( FALSE );
	}

	return FALSE;
}

BEGIN_MESSAGE_MAP( CDlgIndSel6WayValveCircuits, CDialogEx )
	ON_BN_CLICKED( IDCANCEL, &OnBnClickedCancel )
	ON_BN_CLICKED( IDOK, &OnOK )
	ON_BN_CLICKED( IDC_BUTTON_A, &OnBnClickedImgA )
	ON_BN_CLICKED( IDC_BUTTON_B, &OnBnClickedImgB )
	ON_BN_CLICKED( IDC_BUTTON_C, &OnBnClickedImgC )
	ON_BN_CLICKED( IDC_BUTTON_D, &OnBnClickedImgD )
	ON_WM_LBUTTONDOWN()
END_MESSAGE_MAP()

void CDlgIndSel6WayValveCircuits::OnOK()
{
	CDialogEx::OnOK();
	// Reset the focus on the flow edit box in the 'CDlgXXXSel' if possible.
	CDlgSelectionBase *pCurrentDlgSel = NULL;

	if( m_eSavedSelectionMode != m_eCurrentSelectionMode )
	{
		*m_pbIsModified = true;
		m_eSavedSelectionMode = m_eCurrentSelectionMode;
	}
	
	if( NULL != pDlgLeftTabSelManager )
	{
		pCurrentDlgSel = pDlgLeftTabSelManager->GetCurrentLeftTabDialog();
	}

	if( NULL != pCurrentDlgSel )
	{
		pCurrentDlgSel->SetFocusOnControl( CDlgSelectionBase::SetFocusWhere::First | CDlgSelectionBase::SetFocusWhere::Edit );
	}
}

void CDlgIndSel6WayValveCircuits::OnBnClickedImgA()
{
	if( false == m_bCanEQMEnabled )
	{
		return;
	}
	m_eCurrentSelectionMode = e6Way_EQMControl;

	m_ButtonA.ResetDrawBorder();
	m_ButtonB.ResetDrawBorder();
	m_ButtonC.ResetDrawBorder();
	m_ButtonD.ResetDrawBorder();
	m_ButtonA.SetDrawBorder( true, _SELECTEDBLUE );
}

void CDlgIndSel6WayValveCircuits::OnBnClickedImgB()
{
	if( false == m_bCanOnoffPIBCVEnabled )
	{
		return;
	}
	m_eCurrentSelectionMode = e6Way_OnOffControlWithPIBCV;

	m_ButtonA.ResetDrawBorder();
	m_ButtonB.ResetDrawBorder();
	m_ButtonC.ResetDrawBorder();
	m_ButtonD.ResetDrawBorder();
	m_ButtonB.SetDrawBorder( true, _SELECTEDBLUE );
}

void CDlgIndSel6WayValveCircuits::OnBnClickedImgC()
{
	if( false == m_bCanOnoffBVEnabled )
	{
		return;
	}
	m_eCurrentSelectionMode = e6Way_OnOffControlWithSTAD;

	m_ButtonA.ResetDrawBorder();
	m_ButtonB.ResetDrawBorder();
	m_ButtonC.ResetDrawBorder();
	m_ButtonD.ResetDrawBorder();
	m_ButtonC.SetDrawBorder( true, _SELECTEDBLUE );
}

void CDlgIndSel6WayValveCircuits::OnBnClickedImgD()
{
	if( false == m_bCan6WayValveEnabled )
	{
		return;
	}
	m_eCurrentSelectionMode = e6Way_Alone;

	m_ButtonA.ResetDrawBorder();
	m_ButtonB.ResetDrawBorder();
	m_ButtonC.ResetDrawBorder();
	m_ButtonD.ResetDrawBorder();
	m_ButtonD.SetDrawBorder( true, _SELECTEDBLUE );
}

CExtButton* CDlgIndSel6WayValveCircuits::GetDlgCircuitFocus()
{
	if( e6WayValveSelectionMode::e6Way_EQMControl == m_eCurrentSelectionMode )
	{
		return &m_ButtonA;
	}
	else if( e6WayValveSelectionMode::e6Way_OnOffControlWithPIBCV == m_eCurrentSelectionMode )
	{
		return &m_ButtonB;
	}
	else if( e6WayValveSelectionMode::e6Way_OnOffControlWithSTAD == m_eCurrentSelectionMode )
	{
		return &m_ButtonC;
	}
	else if( e6WayValveSelectionMode::e6Way_Alone == m_eCurrentSelectionMode )
	{
		return &m_ButtonD;
	}

	return NULL;
}

void CDlgIndSel6WayValveCircuits::OnBnClickedCancel()
{
	// TODO: Add your control notification handler code here
	CDialogEx::OnCancel();
}


void CDlgIndSel6WayValveCircuits::OnLButtonDown( UINT nFlags, CPoint point )
{
	// TODO: Add your message handler code here and/or call default

	DWORD dwStyleA = m_ButtonA.GetStyle();
	DWORD dwStyleB = m_ButtonB.GetStyle();
	DWORD dwStyleC = m_ButtonC.GetStyle();
	DWORD dwStyleD = m_ButtonD.GetStyle();
	CDialogEx::OnLButtonDown( nFlags, point );
}
