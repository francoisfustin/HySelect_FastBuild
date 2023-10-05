#include "stdafx.h"


#include "afxctl.h"
#include "TASelect.h"
#include "global.h"
#include "utilities.h"
#include "database.h"
#include "wizard.h"
#include "DlgWizCircuit.h"
#include "TerminalUnit.h"
#include "HMPipes.h"
#include "picture.h"
#include "PanelCirc2.h"
#include "ssheetpanelcirc2.h"
#include "DlgHMTree.h"
#include "DlgHMCompilationOutput.h"
#include "DlgInjectionError.h"

CPanelCirc2::CPanelCirc2( CWnd *pParent )
	: CDlgWizard( CPanelCirc2::IDD, pParent )
{
	m_pHM = NULL;
	m_iPrevCompoParentSelection = -1;
	m_bEditEntry = false;
	m_pTADB = NULL;
	m_pTADS = NULL;
	m_pSch = NULL;
	m_pFPSheet = NULL;
	m_pTab = NULL;
	m_nTimer = (UINT_PTR)0;
	m_bAtLeastOneCircuitAdded = false;
}

CPanelCirc2::~CPanelCirc2()
{
	if( NULL != m_pFPSheet )
	{
		delete m_pFPSheet;
		m_pFPSheet = NULL;
	}
}

void CPanelCirc2::Print()
{
	m_pFPSheet->PrintThisSheet();
}

void CPanelCirc2::EnableButtonNext( bool bEnable )
{
	if( true == bEnable )
	{
		( (CDlgWizCircuit *)GetWizMan() )->EnableButtons( CWizardManager::WizButNext );
	}
	else
	{
		( (CDlgWizCircuit *)GetWizMan() )->DisableButtons( CWizardManager::WizButNext );
	}
}

void CPanelCirc2::DeleteCircuit( CDS_HydroMod *pHM )
{
	bool bClearHM = false;

	if( NULL == pHM || m_pHM == pHM )
	{
		pHM = m_pHM;
		bClearHM = true;
	}
	
	int iPosition = pHM->GetPos();
	IDPTR IDPtr = pHM->GetIDPtr();	
	m_pTab->Remove( IDPtr );
	m_pTADS->DeleteObject( IDPtr );

	for( IDPTR IDPtr = m_pTab->GetFirst(); NULL != IDPtr.MP; IDPtr = m_pTab->GetNext() )
	{
		CDS_HydroMod *pHM = (CDS_HydroMod *)( IDPtr.MP );

		if( NULL != pHM && pHM->GetPos() > iPosition )
		{
			pHM->SetPos( pHM->GetPos() - 1 );
		}
	}

	pHM = NULL;

	if( true == bClearHM )
	{
		m_pHM = NULL;
		( (CDlgWizCircuit *)GetWizMan() )->SetpCurrentHM( NULL );
	}
}

BEGIN_MESSAGE_MAP( CPanelCirc2, CDlgWizard )
	ON_CBN_SELCHANGE( IDC_COMBOPARENT, OnSelChangeComboParent )
	ON_EN_KILLFOCUS( IDC_EDITHMNAME, OnEnKillFocusEditHMName )
	ON_EN_KILLFOCUS( IDC_EDITHMDESCRIPTION, OnEnKillFocusEditHMDescription )
	ON_CBN_SELCHANGE( IDC_COMBOPOS, OnSelChangeComboPos )
	ON_WM_TIMER()
END_MESSAGE_MAP()

void CPanelCirc2::DoDataExchange( CDataExchange *pDX )
{
	CDlgWizard::DoDataExchange( pDX );
	DDX_Control( pDX, IDC_EDITHMNAME, m_EditHMName );
	DDX_Control( pDX, IDC_EDITHMDESCRIPTION, m_EditDescription );
	DDX_Control( pDX, IDC_COMBOPARENT, m_ComboParent );
	DDX_Control( pDX, IDC_COMBOPOS, m_ComboPos );
}

BOOL CPanelCirc2::OnInitDialog()
{
	CDlgWizard::OnInitDialog();
	m_pFPSheet = new CSSheetPanelCirc2();
	ASSERT( NULL != m_pFPSheet );

	if( NULL != m_pFPSheet )
	{
		CRect rect( 50, 50, 100, 100 );

		if( FALSE == m_pFPSheet->Create( (GetStyle() | WS_CHILD ) ^ WS_VISIBLE ^ WS_BORDER, rect, this, IDC_FPSPREAD ) )
		{
			return false;
		}
	}

	// Attach Panel to CSetDPI, in case of large font, prevent growing of dialog box.
	dpi.Attach( AfxFindResourceHandle( MAKEINTRESOURCE( IDD ), RT_DIALOG ), m_hWnd, IDD, 96.0 ); // 96 is the DPI
	m_EditHMName.LimitText( TASApp.GetModuleNameMaxChar() );

	// Added to force focus on Spread sheet
	m_nTimer = SetTimer( _TIMERID_PANELCIRC2, 200, 0 );
	return TRUE;
}

void CPanelCirc2::OnSelChangeComboParent()
{
	try
	{
		CTableHM *pclHydraulicNetwork = dynamic_cast<CTableHM *>( m_pTADS->GetpHydroModTable() );

		if( NULL == pclHydraulicNetwork )
		{
			ASSERT_RETURN;
		}

		// User changed parent module...
		if( NULL != m_pHM && m_pTab != (CTable *)m_ComboParent.GetItemDataPtr( m_ComboParent.GetCurSel() ) )
		{	
			IDPTR IDPtr = m_pHM->GetIDPtr();
			CTable *pclTableWhereToInsert = (CTable *)m_ComboParent.GetItemDataPtr( m_ComboParent.GetCurSel() );

			// First verify if there are injection circuits concernened in this object to insert.
			_VerifyInjectionTemperaturesBeforeInsert( (CDS_HydroMod *)( IDPtr.MP ), pclTableWhereToInsert );
			
			if( (int)CDlgInjectionError::ReturnCode::Cancel == m_iDlgInjectionErrorReturnCode )
			{
				// Reset the combo to its previous value and return.
				m_ComboParent.SetCurSel( m_iPrevCompoParentSelection );
				return;
			}
		
			// Disconnect m_pHM from his parent.
			m_pTab->Remove( IDPtr );
		
			// Remove gap in index numeration.
			m_pTADS->VerifyAndCorrectPosition( m_pTab );

			// Retrieve the new parent.
			m_pTab = (CTable *)m_ComboParent.GetItemDataPtr( m_ComboParent.GetCurSel() );
		
			// Connect m_pHM to his new parent.
			m_pTab->Insert( IDPtr );

			// Initialize water characteristics of the circuit just moved.
			// m_pHM->InitAllWaterCharWithParent( m_pTab );

			if( m_pTab->IsClass( CLASS( CTableHM ) ) )
			{
				m_pHM->SetLevel( 0 );
			}
			else
			{
				m_pHM->SetLevel( ( (CDS_HydroMod *)m_pTab )->GetLevel() + 1 );
			}

			// Verify return mode.
			CDS_HydroMod *pParentWhereToInsert = (CDS_HydroMod *)m_pTab;

			if( pParentWhereToInsert->GetCount() > 0 )
			{
				// Remark: If there is not yet any module (or circuit) child under 'pParentWhereToInsert', we have no need to
				//         verify the return mode.
				CDS_HydroMod *pChild = (CDS_HydroMod *)( pParentWhereToInsert->GetFirst().MP );
				CDS_HydroMod *pObjectToInsert = (CDS_HydroMod *)( IDPtr.MP );

				if( pObjectToInsert->GetReturnType() != pChild->GetReturnType() )
				{
					pObjectToInsert->SetReturnType( pChild->GetReturnType() );
				}
			}

			// Determine last index without current HM.
			m_pHM->SetPos( -1 );
			int iIndex = m_pTADS->GetLastBranchPos( m_pTab ) + 1;
		
			// Set index.
			m_pHM->SetPos( iIndex );
			FillComboPos( iIndex, iIndex );
		
			// Update its name.
			OnSelChangeComboPos();

			m_iPrevCompoParentSelection = m_ComboParent.GetCurSel();

			// Pump circuit cannot be installed everywhere.
			if( m_pHM->GetpSchcat() != NULL && true == m_pHM->GetpSchcat()->IsPump() && m_pHM->GetLevel() != 0 )
			{
				GetWizMan()->PostMessage( WM_USER_WIZARDMANAGER_DOBACK, 0, 0 );
			}
			else
			{
				// HYS-1716: Now we can correct injection temperatures that contain error.
				if( (int)CDlgInjectionError::ReturnCode::ApplyWithCorrection == m_iDlgInjectionErrorReturnCode )
				{
					_CorrectInjectionTemperaturesAfterInsert( m_pHM, m_pTab );
				}

				// HYS-1716: Now that the paste operation has been applied, we need to run all chidren injection circuit to update their
				// primary flows.
				// Remark: here we pass as argument the hydraulic circuit in which we have done the paste because we do not need to check all the network 
				//         but only the children of this hydraulic circuit.
				pclHydraulicNetwork->CorrectAllPrimaryFlow( m_pTab );

				m_pHM->ComputeAll();

				if( true == m_pFPSheet->IsReady() )
				{
					m_pFPSheet->RefreshDispatch();
					m_pFPSheet->Invalidate();
				}
			
				// Reset HMTREE.
				if( ( (CDlgWizCircuit *)GetWizMan() )->IsModeEdit() )
				{
					::AfxGetApp()->m_pMainWnd->SendMessageToDescendants( WM_USER_RESETHMTREEEDIT, (WPARAM)false, (LPARAM)m_pHM );
				}
				else
				{
					::AfxGetApp()->m_pMainWnd->SendMessageToDescendants( WM_USER_RESETHMTREECREATE, (WPARAM)false, (LPARAM)m_pHM );
				}
			}
		}
	}
	catch( CHySelectException &clHySelectException )
	{
		clHySelectException.AddMessage( _T("Error in 'CPanelCirc2::OnSelChangeComboParent'."), __LINE__, __FILE__ );
		throw;
	}
}

void CPanelCirc2::OnEnKillFocusEditHMName()
{
	CDS_ProjectParams *pPrjParam = m_pTADS->GetpProjectParams();
	ASSERT( NULL != pPrjParam );

	if( true == pPrjParam->IsFreezed() || NULL == m_pHM )
	{
		return;
	}

	CString str;
	m_EditHMName.GetWindowText( str );
	
	// Verify if the name is not already used.
	bool bFlag = false;
	
	CTable *pPipTab = m_pTADS->GetpHydroModTable();
	ASSERT( NULL != pPipTab );
	
	if( NULL != m_pTADS->FindHydroMod( str, pPipTab, m_pHM ) )
	{
		bFlag = true;
	}
	
	if( true == bFlag )
	{
		CString str1;
		::FormatString( str1, IDS_HYDROMODREF_ALREADYUSED, str );
		AfxMessageBox( str1 );
		m_EditHMName.SetFocus();
	}
	else
	{
		m_pHM->SetHMName( str );
		
		// Is automatic name rename it.
		if( true == m_pHM->GetHMName().IsEmpty() || _T('*') == m_pHM->GetHMName().GetAt( 0 ) )
		{
			m_pHM->SetHMName( _T("*") );

			if( 0 == m_pHM->GetLevel() )
			{
				m_pTADS->ComposeRMName( str, m_pHM, m_pHM->GetPos() );
				m_pHM->SetHMName( (LPCTSTR)str );
			}
			else
			{
				IDPTR IDPtr = m_pHM->GetIDPtr();
				m_pTADS->ComposeValveName( IDPtr );
			}
			m_EditHMName.SetWindowText( m_pHM->GetHMName() );
		}
		
		// Reset HMTREE.
		if( ( (CDlgWizCircuit *)GetWizMan() )->IsModeEdit() )
		{
			::AfxGetApp()->m_pMainWnd->SendMessageToDescendants( WM_USER_RESETHMTREEEDIT, (WPARAM)false, (LPARAM)m_pHM );
		}
		else
		{
			::AfxGetApp()->m_pMainWnd->SendMessageToDescendants( WM_USER_RESETHMTREECREATE, (WPARAM)false, (LPARAM)m_pHM );
		}
	}
}

void CPanelCirc2::OnEnKillFocusEditHMDescription()
{
	CDS_ProjectParams *pPrjParam = m_pTADS->GetpProjectParams();
	ASSERT( NULL != pPrjParam );

	if ( true == pPrjParam->IsFreezed() )
	{
		return;
	}

	CString str;
	m_EditDescription.GetWindowText( str );
	m_pHM->SetDescription( str );
}

void CPanelCirc2::OnSelChangeComboPos()
{
	// Not yet initialized ?
	if( NULL == m_ComboPos.m_hWnd || NULL == m_pHM )
	{
		return;
	}
	
	int iCurPos = m_pHM->GetPos();
	int iNewIndex = m_ComboPos.GetItemData( m_ComboPos.GetCurSel() );

	m_pTADS->ChangePosition( m_pHM, iNewIndex );

	m_EditHMName.SetWindowText( m_pHM->GetHMName() );

	if( iCurPos != iNewIndex )
	{
		m_pHM->ComputeAll();
	}
	
	if( true == m_pFPSheet->IsReady() )
	{
		m_pFPSheet->Init( m_pHM, GetWizMan()->GetCurrentPanel()->m_ptInitialSize );
	}

	// Reset HMTREE.
	if( true == ( (CDlgWizCircuit *)GetWizMan() )->IsModeEdit() )
	{
		::AfxGetApp()->m_pMainWnd->SendMessageToDescendants( WM_USER_RESETHMTREEEDIT, (WPARAM)false, (LPARAM)m_pHM );
	}
	else
	{
		::AfxGetApp()->m_pMainWnd->SendMessageToDescendants( WM_USER_RESETHMTREECREATE, (WPARAM)false, (LPARAM)m_pHM );
	}
}

void CPanelCirc2::OnTimer( UINT_PTR nIDEvent )
{
	CDlgWizard::OnTimer( nIDEvent );

	if( (UINT_PTR)0 == m_nTimer || m_nTimer != nIDEvent )
	{
		return;
	}

	m_pFPSheet->SetFocus();
	KillTimer( m_nTimer );
	m_nTimer = (UINT_PTR)0;
}

BOOL CPanelCirc2::OnWizCancel()
{
	OnWizFinish();
	return TRUE;
}

// Button ADD in this panel
BOOL CPanelCirc2::OnWizNext()
{
	BeginWaitCursor();

	// Previous HM is copied, total Q increase, relaunch compute.
	// Add circuit.
	// Update Description members.
	// HYS-1469 : For each root module we directly go to construct the child instead of another root module
	bool bFirstAddUnderRootModule = false;
	int k = ( (CDlgWizCircuit *)GetWizMan() )->GetpSpin()->GetPos();

	for( int i = 0; i < k; i++ )
	{
		CString str;
		m_EditDescription.GetWindowText( str );
		m_pHM->SetDescription( str );
		
		// Create new circuit.
		CDS_HydroMod *pHMcopy = m_pHM;
		CreateNewCircuit( ( (CDlgWizCircuit *)GetWizMan() )->m_PanelCirc1.IsTerminalUnitChecked() );
		
		// Copy previous HM to the new created HydroMod.
		if( true == pHMcopy->IsClass( CLASS( CDS_Hm3W ) ) )
		{
			( (CDS_Hm3W *)pHMcopy )->Copy( (CDS_Hm3W *)m_pHM, false, true );
		}
		else if( true == pHMcopy->IsClass( CLASS( CDS_Hm2W ) ) )
		{
			( (CDS_Hm2W *)pHMcopy )->Copy( (CDS_Hm2W *)m_pHM, false, true );
		}
		else if( true == pHMcopy->IsClass( CLASS( CDS_Hm2WInj ) ) )
		{
			( (CDS_Hm2WInj *)pHMcopy )->Copy( (CDS_Hm2WInj *)m_pHM, false, true );
		}
		else if( true == pHMcopy->IsClass( CLASS( CDS_Hm3WInj ) ) )
		{
			( (CDS_Hm3WInj *)pHMcopy )->Copy( (CDS_Hm3WInj *)m_pHM, false, true );
		}
		else if( true == pHMcopy->IsClass( CLASS( CDS_HydroMod ) ) )
		{
			pHMcopy->Copy( m_pHM, false, true );
		}
		else
		{
			ASSERT( 0 );
		}
		
		m_pHM->SetHavail( 0.0, CAnchorPt::CircuitSide_Primary );
		
		// Set name and position.
		m_pHM->SetHMName( _T("*") );

		// HYS-1469.
		if( 0 == pHMcopy->GetLevel() && 1 == k )
		{
			bFirstAddUnderRootModule = true;
		}
		
		// Change position of following modules/circuits.
		int iPosition = pHMcopy->GetPos() + 1;

		for( IDPTR IDPtr = m_pTab->GetFirst(); NULL != IDPtr.MP; IDPtr = m_pTab->GetNext() )
		{
			CDS_HydroMod *pHM = (CDS_HydroMod *)( IDPtr.MP );

			if( NULL != pHM && pHM->GetPos() >= iPosition )
			{
				pHM->SetPos( pHM->GetPos() + 1 );
			}
		}
		
		m_pHM->SetPos( iPosition );
		
		if( 0 == m_pHM->GetLevel() )
		{
			m_pTADS->ComposeRMName( str, m_pHM, m_pHM->GetPos() );
			m_pHM->SetHMName( (LPCTSTR)str );
		}
		else
		{
			IDPTR IDPtr = m_pHM->GetIDPtr();
			m_pTADS->ComposeValveName( IDPtr );
		}
		
		// Resize current module needed, if user has changed scheme, circuit must be computed.
		m_pHM->ComputeHM( CDS_HydroMod::eComputeHMEvent::eceResize );
	}

	// HYS-1469 : Set current HM before using onSelChangeComboParent
	// Reinit HM pointer.
	( (CDlgWizCircuit *)GetWizMan() )->SetpCurrentHM( m_pHM );
	
	// Reset combo parent.
	ResetComboParent();

	if( true == bFirstAddUnderRootModule )
	{
		// HYS-1469 : SetCurSel the current root module
		m_ComboParent.SetCurSel( m_ComboParent.GetCount() - 1 );
		m_iPrevCompoParentSelection = m_ComboParent.GetCurSel();

		// This allow to insert into the root module.
		OnSelChangeComboParent();
	}
	else
	{
		int iIndex = m_pTADS->GetLastBranchPos( m_pTab );
		FillComboPos( iIndex, iIndex );

		// Update his name.
		OnSelChangeComboPos();
	}
	
	m_ComboParent.EnableWindow( ( (m_ComboParent.GetCount() > 1 ) && false == m_pHM->GetpPrjParam()->IsFreezed() ) ? TRUE : FALSE );

	// Resize current module needed, if user has changed scheme, circuit must be computed.
	m_pHM->ComputeHM( CDS_HydroMod::eComputeHMEvent::eceResize );
	m_pHM->ComputeAll();
	
	// Reinit spread sheet.
	m_pFPSheet->Init( m_pHM, GetWizMan()->GetCurrentPanel()->m_ptInitialSize );
	( (CDlgWizCircuit *)GetWizMan() )->GetpSpin()->SetPos( 1 );
	EndWaitCursor();
	
	// Must return false no existing panel after panel circ2; button next is redirected to ADD a circuit functionality.
	return FALSE;
}

BOOL CPanelCirc2::OnWizBack() 
{
	( (CDlgWizCircuit *)GetWizMan() )->SetpCurrentHM( m_pHM );
	return TRUE;
}

BOOL CPanelCirc2::OnWizFinish()
{
	BeginWaitCursor();

	if( NULL == m_pHM )
	{
		EndWaitCursor();
		return TRUE;
	}

	CTable *pTab = (CTable *)( m_pHM->GetIDPtr().PP );

	if( NULL == pTab )
	{
		EndWaitCursor();
		return TRUE;
	}
	
	// Delete object.
	if( false == ( (CDlgWizCircuit *)GetWizMan() )->IsModeEdit() )
	{
		// HYS-1045: Clear the Output when canceling before adding on a root module
		if( true == m_pHM->GetIDPtr().PP->IsClass( CLASS( CTableHM ) ) || true == m_pHM->GetIDPtr().PP->IsClass( CLASS( CTable ) ) )
		{
			// On root module, delete output before deleting circuit
			CDlgHMCompilationOutput::CHMInterface clOutputInterface;

			if( false == TASApp.GetpTADS()->GetCheckAllModules() )
			{
				clOutputInterface.ClearOutput();
			}
		}
		
		::AfxGetApp()->m_pMainWnd->SendMessageToDescendants( WM_USER_RESETHMTREECREATE, (WPARAM)false, (LPARAM)m_pHM->GetParent() );
		DeleteCircuit();
		m_pTADS->VerifyAndCorrectPosition( pTab );
	}
	else
	{
		// save description, name.
		CString str;
		m_EditDescription.GetWindowText( str );
		m_pHM->SetDescription( str );
		m_EditHMName.GetWindowText( str );
		m_pHM->SetHMName( str );
	}

	// Force a refresh of Q for the module.
	if( false == pTab->IsClass( CLASS( CTableHM ) ) )
// Test for  PROFILING
//		for (int i =0; i<10; i++)
			( (CDS_HydroMod*)pTab )->ComputeAll();
	EndWaitCursor();
	return TRUE;
}

int CPanelCirc2::OnWizButFinishTextID()
{
	return ( false == ( (CDlgWizCircuit *)GetWizMan() )->IsModeEdit() ) ? IDS_CANCEL : IDS_CLOSE;
}

bool CPanelCirc2::OnInitialActivate()
{
	// Retrieve current position of the separator.
	CRect rectSeparator;
	GetDlgItem( IDC_SEPARATORUP )->GetWindowRect( &rectSeparator );
	ScreenToClient( &rectSeparator );

	// Retrieve current dialog client area.
	CRect rectDialog;
	GetClientRect( &rectDialog );

	m_pFPSheet->SetWindowPos( NULL, rectDialog.left, rectSeparator.bottom, rectDialog.Width(), rectDialog.Height() - rectSeparator.bottom - 2, SWP_NOZORDER );
	
	m_pTADS = TASApp.GetpTADS();
	m_pTADB = TASApp.GetpTADB();
	
	m_pTab = ( (CDlgWizCircuit *)GetWizMan() )->GetpInitialTab();
	m_pHM = ( (CDlgWizCircuit *)GetWizMan() )->GetpCurrentHM();
	m_bEditEntry = true;
	
	// Initialize static members.
	CString str;
	str = TASApp.LoadLocalizedString( IDS_PANELCIRC2_STATICNAME );
	GetDlgItem( IDC_STATICNAME )->SetWindowText( str );
	
	str = TASApp.LoadLocalizedString( IDS_PANELCIRC2_STATICDESC );
	GetDlgItem( IDC_STATICDESC )->SetWindowText( str );
	
	str = TASApp.LoadLocalizedString( IDS_PANELCIRC2_STATICINSERTIN );
	GetDlgItem( IDC_STATICINSERTIN )->SetWindowText( str );
	str = TASApp.LoadLocalizedString( IDS_PANELCIRC2_STATICPOS );
	GetDlgItem( IDC_STATICPOS )->SetWindowText( str );

	CDS_ProjectParams *pPrjParam = m_pTADS->GetpProjectParams();
	ASSERT( NULL != pPrjParam );

	bool bFreezed = (NULL != pPrjParam) ? pPrjParam->IsFreezed() : true;
	GetDlgItem( IDC_EDITHMNAME )->EnableWindow( ( false == bFreezed  ) ? TRUE : FALSE );
	GetDlgItem( IDC_EDITHMDESCRIPTION )->EnableWindow( ( false == bFreezed  ) ? TRUE : FALSE );
	GetDlgItem( IDC_COMBOPARENT )->EnableWindow( ( false == bFreezed  ) ? TRUE : FALSE );
	GetDlgItem( IDC_COMBOPOS )->EnableWindow( ( false == bFreezed  ) ? TRUE : FALSE );

	return OnActivate();
}

bool CPanelCirc2::OnActivate()
{
	( (CDlgWizCircuit *)GetWizMan() )->GetpSpin()->SetPos( 1 );

	// Retrieve selected scheme.
	m_pSch = ( (CDlgWizCircuit *)GetWizMan() )->m_PanelCirc1.GetSelectedSch();

	// Terminal unit needed ?
	bool bIsTerminalUnitUsed = ( (CDlgWizCircuit *)GetWizMan() )->m_PanelCirc1.IsTerminalUnitChecked();

	// Direct or reverse return mode ?
	CDS_HydroMod::ReturnType eReturnType = ( true == ( (CDlgWizCircuit *)GetWizMan() )->m_PanelCirc1.IsDirectReturn() ) ? CDS_HydroMod::ReturnType::Direct : CDS_HydroMod::ReturnType::Reverse;

	if( NULL == m_pHM || false == m_bEditEntry )
	{
		// HYS-1751: Is true when scheme changes and has an impact in PC2
		bool bSchemeChanged = false;

		// If 'm_pHM' already exist, copy what is possible.
		if( NULL != m_pHM )
		{
			// If the edited circuit is exactly the same keep it.
			// If it's different duplicate what it's possible.
			bool bKeepIt = true;

			// HYS-1698: We will keep the user choice concerning the position of the control valve in case of 3-way mixing circuit
			// with decoupling bypass.
			CvLocation eCvLocation = CvLocation::CvLocLast;
			eBool3 ePrevCVLocAuto = eBool3::eb3Undef;
			CvLocation ePrevCVLocation = CvLocation::CvLocLast;

			if( NULL != m_pHM->GetpSchcat() && CDB_CircSchemeCateg::e3WType::e3wTypeMixDecByp == m_pHM->GetpSchcat()->Get3WType()
					&& NULL != m_pSch->GetpSchCateg() && CDB_CircSchemeCateg::e3WType::e3wTypeMixDecByp == m_pSch->GetpSchCateg()->Get3WType() )
			{
				if( false == m_pHM->GetpCV()->IsCVLocAuto() && m_pHM->GetpSch()->GetCvLoc() == m_pSch->GetCvLoc() )
				{
					eCvLocation = m_pHM->GetpCV()->GetCvLocate();
				}

				ePrevCVLocAuto = ( false == m_pHM->GetpCV()->IsCVLocAuto() ) ? eBool3::eb3False : eBool3::eb3True;
				ePrevCVLocation = m_pHM->GetpSch()->GetCvLoc();
			}

			if( m_pHM->GetSchemeIDPtr() == m_pSch->GetIDPtr() )
			{
				// Test TU.
				if( bIsTerminalUnitUsed == m_pHM->IsaModule() )
				{
					bKeepIt = false;
				}
				
				m_pHM->SetFlagModule( !bIsTerminalUnitUsed );

				// If return mode becoming different...
				if( eReturnType != m_pHM->GetReturnType() )
				{
					m_pHM->SetReturnType( eReturnType );
				}

				// Cross verify hydromod with anchoring points on the selected circuit.
				CSSheetPanelCirc1 *pSheetPC1 = ( (CDlgWizCircuit *)GetWizMan() )->m_PanelCirc1.GetFPSpreadSheet();
				CDB_CircuitScheme *pSch = pSheetPC1->GetSelectedSch();
				
				bool bAnchorExist = pSch->IsAnchorPtExist( CAnchorPt::BV_P );

				if( NULL != m_pHM->GetpBv() )
				{
					if( false == bAnchorExist )
					{
						m_pHM->DeleteBv( m_pHM->GetppBv() );
					}
				}
				else
				{
					if( true == bAnchorExist && false == pSch->IsAnchorPtOptional( CAnchorPt::BV_P ) )
					{
						m_pHM->CreateBv( CDS_HydroMod::eHMObj::eBVprim );
					}
				}

				bAnchorExist =  pSheetPC1->IsAnchorPtExistInSelectedSCH( CAnchorPt::BV_Byp );
				
				if( NULL != m_pHM->GetpBypBv() )
				{
					if( false == bAnchorExist )
					{
						m_pHM->DeleteBv( m_pHM->GetppBypBv() );
					}
				}
				else
				{
					if( true == bAnchorExist && false == pSch->IsAnchorPtOptional( CAnchorPt::BV_Byp ) )
					{
						m_pHM->CreateBv( CDS_HydroMod::eHMObj::eBVbyp );
					}
				}

				bAnchorExist =  pSheetPC1->IsAnchorPtExistInSelectedSCH( CAnchorPt::BV_S );

				if( NULL != m_pHM->GetpSecBv() )
				{
					if( false == bAnchorExist )
					{
						m_pHM->DeleteBv( m_pHM->GetppSecBv() );
					}
				}
				else
				{
					if( true == bAnchorExist && false == pSch->IsAnchorPtOptional( CAnchorPt::BV_S ) )
					{
						m_pHM->CreateBv( CDS_HydroMod::eHMObj::eBVsec );
					}
				}

				bAnchorExist = pSheetPC1->IsAnchorPtExistInSelectedSCH( CAnchorPt::ControlValve ) || pSheetPC1->IsAnchorPtExistInSelectedSCH( CAnchorPt::PICV );
				bAnchorExist |= pSheetPC1->IsAnchorPtExistInSelectedSCH( CAnchorPt::DPCBCV );

				if( NULL != m_pHM->GetpCV() )
				{
					if( false == bAnchorExist )
					{
						m_pHM->DeleteCv();
					}
				}
				else
				{
					if( true == bAnchorExist )
					{
						m_pHM->CreateCv( pSch->GetCvLoc() );
					}
				}

				bAnchorExist = pSheetPC1->IsAnchorPtExistInSelectedSCH( CAnchorPt::DpC );

				if( NULL != m_pHM->GetpDpC() )
				{
					if( false == bAnchorExist )
					{
						m_pHM->DeleteDpC();
					}
				}
				else
				{
					if( true == bAnchorExist )
					{
						m_pHM->CreateDpC( );
					}
				}

				bAnchorExist = pSheetPC1->IsAnchorPtExistInSelectedSCH( CAnchorPt::ShutoffValve );

				if( ShutoffValveLocSupply == m_pSch->GetShutoffValveLoc() )
				{
					if( NULL != m_pHM->GetpShutoffValve( CDS_HydroMod::eShutoffValveSupply ) )
					{
						if( false == bAnchorExist )
						{
							m_pHM->DeleteSV( m_pHM->GetppSV( CDS_HydroMod::eShutoffValveSupply ) );
						}
					}
					else
					{
						if( true == bAnchorExist )
						{
							m_pHM->CreateSV( CDS_HydroMod::eShutoffValveSupply );
						}
					}
				}
				else if( ShutoffValveLocReturn == m_pSch->GetShutoffValveLoc() )
				{
					if( NULL != m_pHM->GetpShutoffValve( CDS_HydroMod::eShutoffValveReturn ) )
					{
						if( false == bAnchorExist )
						{
							m_pHM->DeleteSV( m_pHM->GetppSV( CDS_HydroMod::eShutoffValveReturn ) );
						}
					}
					else
					{
						if( true == bAnchorExist )
						{
							m_pHM->CreateSV( CDS_HydroMod::eShutoffValveReturn );
						}
					}
				}

				// Test control valve.
				if( true == m_pHM->IsCvExist() )
				{
					// TA CV ?
					if( m_pHM->GetpCV()->IsTaCV() != ( (CDlgWizCircuit *)GetWizMan() )->m_PanelCirc1.IsTACv() )
					{

						bKeepIt = false;
					}
					else if( m_pHM->GetpCV()->IsOn_Off() != ( (CDlgWizCircuit *)GetWizMan() )->m_PanelCirc1.IsCvOnOff() )
					{
						// On-off or proportional?
						bKeepIt = false;
					}
					else
					{
						// CV exist verify Func.
						CDB_ControlValve *pCv = (CDB_ControlValve *)( m_pHM->GetpCV()->GetCvIDPtr().MP );

						if( NULL != pCv && NULL != m_pHM->GetpSch() && m_pHM->GetpSch()->GetCvFunc() != pCv->GetCtrlProp()->GetCvFunc() )
						{
							bKeepIt = false;
						}
					}
				}

				// Smart control valve.
				bAnchorExist = pSheetPC1->IsAnchorPtExistInSelectedSCH( CAnchorPt::SmartControlValve );

				if( NULL != m_pHM->GetpSmartControlValve() )
				{
					if( false == bAnchorExist )
					{
						m_pHM->DeleteSmartControlValve();
					}
				}
				else
				{
					if( true == bAnchorExist && NULL != pSheetPC1->GetSelectedSch() )
					{
						CDB_CircuitScheme *pclCircuitScheme = pSheetPC1->GetSelectedSch();
						CAnchorPt::PipeLocation ePipeLocation = pclCircuitScheme->GetAnchorPtPipeLocation( CAnchorPt::SmartControlValve );

						if( CAnchorPt::PipeLocation_Supply == ePipeLocation )
						{
							m_pHM->CreateSmartControlValve( SmartValveLocalization::SmartValveLocSupply );
						}
						else if( CAnchorPt::PipeLocation_Return == ePipeLocation )
						{
							m_pHM->CreateSmartControlValve( SmartValveLocalization::SmartValveLocReturn );
						}
						else
						{
							// Other pipe location are not manager for smart control valve.
							ASSERT(0);
							bKeepIt = false;
						}
					}
				}

				// Smart differential pressure controller.
				bAnchorExist = pSheetPC1->IsAnchorPtExistInSelectedSCH( CAnchorPt::SmartDpC );

				if( NULL != m_pHM->GetpSmartDpC() )
				{
					if( false == bAnchorExist )
					{
						m_pHM->DeleteSmartDpC();
					}
				}
				else
				{
					if( true == bAnchorExist && NULL != pSheetPC1->GetSelectedSch() )
					{
						CDB_CircuitScheme *pclCircuitScheme = pSheetPC1->GetSelectedSch();
						CAnchorPt::PipeLocation ePipeLocation = pclCircuitScheme->GetAnchorPtPipeLocation( CAnchorPt::SmartControlValve );

						if( CAnchorPt::PipeLocation_Supply == ePipeLocation )
						{
							m_pHM->CreateSmartDpC( SmartValveLocalization::SmartValveLocSupply );
						}
						else if( CAnchorPt::PipeLocation_Return == ePipeLocation )
						{
							m_pHM->CreateSmartDpC( SmartValveLocalization::SmartValveLocReturn );
						}
						else
						{
							// Other pipe location are not manager for smart differential pressure controller.
							ASSERT(0);
							bKeepIt = false;
						}
					}
				}
			}
			else
			{
				bKeepIt = false;
			}
			
			if( false == bKeepIt )
			{
				// HYS-1698: save previous circuit scheme.
				CDB_CircSchemeCateg::e3WType e3WayTYpe = m_pHM->GetpSchcat()->Get3WType();

				DuplicateHM();
				
				( (CDlgWizCircuit *)GetWizMan() )->SetpCurrentHM( m_pHM );

				// HYS-1698: if the 'eCvLocation' is defined, it's because user change only the property of the control valve and
				// not the schematic. So we try to keep his choice concerning the localization of the control valve.
				if( CvLocation::CvLocLast != eCvLocation ) 
				{
					m_pHM->GetpCV()->SetCVLocAuto( false );
					m_pHM->GetpCV()->SetCvLocate( eCvLocation );
				}
				else if( eBool3::eb3Undef != ePrevCVLocAuto )
				{
					// If user was in automatic mode, we verify if he has not changed the schematic. If not, we let the automatic mode.
					// If yes, we remove the automatic mode.
					if( ePrevCVLocation != m_pSch->GetCvLoc() )
					{
						m_pHM->GetpCV()->SetCVLocAuto( false );
						m_pHM->GetpCV()->SetCvLocate( m_pSch->GetCvLoc() );
					}
				}

				// HYS-1698: If user was not on a 3-way mixing circuit with decoupling bypass but well now, we need to force the 
				// CV location automatic mode to false.
				if( CDB_CircSchemeCateg::e3WType::e3wTypeMixDecByp != e3WayTYpe
					&& NULL != m_pSch->GetpSchCateg() && CDB_CircSchemeCateg::e3WType::e3wTypeMixDecByp == m_pSch->GetpSchCateg()->Get3WType() )
				{
					m_pHM->GetpCV()->SetCVLocAuto( false );
				}
				
				// Resize current module needed, if user has changed scheme, circuit must be computed.
				// Use SetQ function to force the system to reload Q in each object, needed for circuit pipe in a pending circuit!
				// Flow in this pipe will be cleared when computing the installation.
				m_pHM->SetHMPrimAndSecFlow( m_pHM->GetpTermUnit()->GetQ() );

				// HYS-1751: Set to true to force computeHM when we are in edit mode.
				bSchemeChanged = true;
			}
		}
		else
		{
			CreateNewCircuit( bIsTerminalUnitUsed );

			// HYS-1968: When creating a 3-way mixing circuit with decoupling bypass, user can already choose with schematics where the control
			// valve is located. So we don't need to put the "Automatic" mode. It's up to the user to put "Automatic" if he wants in the 
			// SheetPanelCirc2.
			if( NULL != m_pHM->GetpSchcat() && CDB_CircSchemeCateg::e3WType::e3wTypeMixDecByp == m_pHM->GetpSchcat()->Get3WType()
					&& NULL != m_pSch->GetpSchCateg() && CDB_CircSchemeCateg::e3WType::e3wTypeMixDecByp == m_pSch->GetpSchCateg()->Get3WType() )
			{
				m_pHM->GetpCV()->SetCVLocAuto( false );
			}
		}

		// Specific treatment for DpC with PressetPT CV; force MV (CV in fact) on the secondary of DpC.
		if( NULL != m_pHM->GetpDpC() && NULL != m_pHM->GetpSch() )
		{
			if( CDB_ControlProperties::eCVFUNC::PresetPT == m_pHM->GetpSch()->GetCvFunc() )
			{
				m_pHM->GetpDpC()->SetMvLoc( MvLocSecondary );	
			}
		}

		// Resize current module needed, if user has changed scheme, circuit must be computed.

		// HYS-1751: ComputeHM when we are in edit mode and the scheme has changed
		if( false == ( (CDlgWizCircuit *)GetWizMan() )->IsModeEdit() || true == bSchemeChanged )
		{
			m_pHM->ComputeHM( CDS_HydroMod::eComputeHMEvent::eceResize );
		}
		
		// Modification must be spread on the installation.
		m_pHM->ComputeAll();
	}

	FillLocalControls();
	m_bEditEntry = false;
	return true;
}

bool CPanelCirc2::OnAfterActivate()
{
	if( true == ( (CDlgWizCircuit *)GetWizMan() )->IsModeEdit() )
	{
		( (CDlgWizCircuit *)GetWizMan() )->ShowButtons( CWizardManager::enum_WizBut::WizButNext, SW_HIDE );
		
		if( true == m_pHM->GetpPrjParam()->IsFreezed() )
		{
			( (CDlgWizCircuit *)GetWizMan() )->ShowButtons( CWizardManager::enum_WizBut::WizButBack, SW_HIDE );
		}
	}
	else
	{
		( (CDlgWizCircuit *)GetWizMan() )->m_ButWizNext.ShowWindow( SW_SHOW );
	}
	
	m_pFPSheet->Init( m_pHM, GetWizMan()->GetCurrentPanel()->m_ptInitialSize );
	
	// Just to have the focus on something and to be able to intercept the [ESCAPE] in the 'CDlgWizard' base class.
	m_pFPSheet->SetFocus();

	return true;
}

void CPanelCirc2::OnSize( CRect rectParentSize )
{
	// Retrieve current position of the separator.
	CRect rectSeparator;
	GetDlgItem( IDC_SEPARATORUP )->GetWindowRect( &rectSeparator );
	ScreenToClient( &rectSeparator );

	// Separator set in same place but take the all width.
	GetDlgItem( IDC_SEPARATORUP )->SetWindowPos( NULL, -1, -1, rectParentSize.Width(), rectSeparator.Height(), SWP_NOZORDER | SWP_NOMOVE );

	CRect rectPanelSize;
	GetClientRect( &rectPanelSize );

	m_pFPSheet->SetWindowPos( NULL, rectParentSize.left, rectSeparator.bottom, rectParentSize.Width(), rectPanelSize.Height() - rectSeparator.bottom, SWP_NOZORDER );
	m_pFPSheet->Invalidate();
	m_pFPSheet->UpdateWindow();
	m_pFPSheet->OptimizeColumnWidth();
}

void CPanelCirc2::OnGetMinMaxInfo( MINMAXINFO* lpMMI )
{
	if( GetDlgItem( IDC_STATICPOS )->GetSafeHwnd() != NULL )
	{
		// Retrieve dimension of the control located at the right edge of dialog (Here it's the 'IDC_STATICPOS').
		CRect rectStaticCtrlType;
		GetDlgItem( IDC_STATICPOS )->GetWindowRect( &rectStaticCtrlType );

		// Retrieve position of the parent dialog.
		CRect rectDialog;
		GetParent()->GetWindowRect( &rectDialog );

		// Take in count the border size.
		int iBorderSize = ::GetSystemMetrics( SM_CXSIZEFRAME );
		lpMMI->ptMinTrackSize.x = rectStaticCtrlType.right - rectDialog.left + iBorderSize;
	}
}

void CPanelCirc2::ResetComboParent()
{
	CRank rkl;
	CString str;
	LPARAM lparam;

	// Fill ComboParent.
	m_ComboParent.ResetContent();
	m_iPrevCompoParentSelection = -1;

	CTable *pTab = m_pTADS->GetpHydroModTable();
	ASSERT( NULL != pTab );

	for( IDPTR HMIDPtr = pTab->GetFirst(); _T('\0') != *HMIDPtr.ID; HMIDPtr = pTab->GetNext() )
	{
		rkl.Add( _T(""), ( (CDS_HydroMod *)HMIDPtr.MP )->GetPos(), (LPARAM)HMIDPtr.MP );
	}
	
	for( BOOL bContinue = rkl.GetFirst( str, lparam ); TRUE == bContinue; bContinue = rkl.GetNext( str, lparam ) )
	{
		FillComboParent( (CDS_HydroMod*)lparam );
	}

	int iIndex = 0;

	if( true == m_pHM->IsaModule() )
	{
		str = TASApp.LoadLocalizedString( IDS_HYDRAULIC_NETWORK );
		iIndex = m_ComboParent.InsertString( 0, str );
		m_ComboParent.SetItemDataPtr( iIndex, pTab );
	}
	
	if( NULL != m_pTab && false == m_pTab->IsClass( CLASS( CTableHM ) ) )
	{
		iIndex = m_ComboParent.FindStringExact( -1, ( (CDS_HydroMod*)m_pTab )->GetHMName() );
	}
	else
	{
		iIndex = m_ComboParent.FindStringExact( -1, str );
	}
	
	m_ComboParent.EnableWindow( ( m_ComboParent.GetCount() > 1 ) && false == m_pHM->GetpPrjParam()->IsFreezed() );
	m_ComboParent.SetCurSel( iIndex );
	m_iPrevCompoParentSelection = iIndex;
}

void CPanelCirc2::FillComboParent( CDS_HydroMod *pHM )
{
	if( false == pHM->IsaModule() || m_pHM == pHM )
	{
		return;
	}

	int iIndex = m_ComboParent.AddString( pHM->GetHMName() );
	m_ComboParent.SetItemDataPtr( iIndex, (void *)pHM );
	
	// Add children.
	CRank rkl;
	CString str;
	LPARAM lparam;

	for( IDPTR HMIDPtr = pHM->GetFirst(); _T('\0') != *HMIDPtr.ID; HMIDPtr = pHM->GetNext() )
	{
		rkl.Add( _T(""), ( (CDS_HydroMod *)HMIDPtr.MP )->GetPos(), (LPARAM)HMIDPtr.MP );
	}
	
	for( BOOL bContinue = rkl.GetFirst( str, lparam ); TRUE == bContinue; bContinue = rkl.GetNext( str, lparam ) )
	{
		FillComboParent( (CDS_HydroMod *)lparam );
	}
}

void CPanelCirc2::CreateNewCircuit( bool bWithTU )
{
	// HYS-1579: Use GetControlType.
	CDB_ControlProperties::CvCtrlType CtrlType = ( (CDlgWizCircuit *)GetWizMan() )->m_PanelCirc1.GetControlType();
	CDS_HydroMod::ReturnType eReturnType = ( true == ( (CDlgWizCircuit *)GetWizMan() )->m_PanelCirc1.IsDirectReturn() ) ? CDS_HydroMod::ReturnType::Direct : CDS_HydroMod::ReturnType::Reverse;
	
	// Pay attention: before we have only the possibility to create a circuit from the 'CTabCDialogProj' in the left tab tree view. And obviously
	//                it was only possible for module. Now, we can right click in 'CSheetHMCalc' and add a module (or circuit) everywhere, not only 
	//                on a module but also on a circuit. If it's on a circuit, we insert the new module (or circuit) before the selected circuit.
	CDS_HydroMod *pModuleClicked = dynamic_cast<CDS_HydroMod *>( m_pTab );
	bool bChangePosition = false;
	int iPosition = 0;

	if( NULL != pModuleClicked && false == pModuleClicked->IsaModule() )
	{
		m_pTab = (CTable *)( m_pTab->GetIDPtr().PP );
		iPosition = pModuleClicked->GetPos();
		bChangePosition = true;
	}

	m_pHM = m_pTADS->CreateNewHM( m_pTab, m_pSch, CtrlType, ( (CDlgWizCircuit *)GetWizMan() )->m_PanelCirc1.IsTACv(), bWithTU, eReturnType /*, paFunc*/ );

	if( NULL != m_pHM && true == bChangePosition )
	{
		// Change position of following modules/circuits.
		for( IDPTR IDPtr = m_pTab->GetFirst(); NULL != IDPtr.MP; IDPtr = m_pTab->GetNext() )
		{
			CDS_HydroMod *pHM = (CDS_HydroMod *)( IDPtr.MP );

			if( NULL != pHM && pHM->GetPos() >= iPosition )
			{
				pHM->SetPos( pHM->GetPos() + 1 );
			}
		}
		
		// Insert the new module/circuit in the position where user has clicked.
		m_pHM->SetPos( iPosition );
	}
	else
	{
		// Move module/circuit at the last available position.
		int iIndex = m_pTADS->GetLastBranchPos( m_pTab ) + 1;
		m_pHM->SetPos( iIndex );
	}

	m_bAtLeastOneCircuitAdded = true;
}

void CPanelCirc2::FillLocalControls()
{
	ResetComboParent();

	if( false == ( (CDlgWizCircuit *)GetWizMan() )->IsModeEdit() )
	{
		if( true == m_pTab->IsClass( CLASS( CTableHM ) ) )
		{
			m_pHM->SetLevel( 0 );
		}
		else
		{
			m_pHM->SetLevel( ( (CDS_HydroMod *)m_pTab )->GetLevel() + 1 );
		}

		// Position is well set in the 'CreateNewCircuit' method.
		FillComboPos( m_pHM->GetPos(), m_pHM->GetPos() );
		
		// Update his name.
		m_EditHMName.SetWindowText( m_pHM->GetHMName() );

	}
	else
	{
		int iIndex = m_pTADS->GetLastBranchPos( m_pTab );
		FillComboPos( iIndex, m_pHM->GetPos() );
		
		// Update his name.
		m_EditHMName.SetWindowText( m_pHM->GetHMName() );
	}

	// Description.
	m_EditDescription.SetWindowText( m_pHM->GetDescription() );
}

void CPanelCirc2::DuplicateHM()
{
	bool bIsTerminalUnitUsed = false;
	bIsTerminalUnitUsed = ( (CDlgWizCircuit *)GetWizMan() )->m_PanelCirc1.IsTerminalUnitChecked();

	CDS_HydroMod *pHMcopy = m_pHM;
	
	// Create a new circuit with new user parameters, m_pHM is updated with the new circuit *.
	CreateNewCircuit( bIsTerminalUnitUsed );

	m_pTADS->CopyCommonHMData( pHMcopy, m_pHM );
	
	// Update Terminal Unit.
	if( false == m_pHM->IsTermUnitExist() )
	{
		m_pHM->CreateTermUnit();
	}

	m_pHM->SetFlagModule( !bIsTerminalUnitUsed );
	m_pHM->GetpTermUnit()->SetVirtual( !bIsTerminalUnitUsed );
}

void CPanelCirc2::FillComboPos( int iMaxPos, int iCurPos )
{
	m_ComboPos.ResetContent();
	int iCurPosIndex = 0;

	for( int i = 1; i <= iMaxPos; i++ )
	{
		CString sPos; 
		sPos.Format( _T("%d"), i );
		int j = m_ComboPos.AddString( (LPCTSTR)sPos );
		m_ComboPos.SetItemData( j, i );

		if( iCurPos == i )
		{
			iCurPosIndex = j;
		}
	}

	m_ComboPos.SetCurSel( iCurPosIndex );
}

void CPanelCirc2::_VerifyInjectionTemperaturesBeforeInsert( CDS_HydroMod *pclObjectToInsert, CTable *pclTableWhereToInsert )
{
	if( NULL == pclObjectToInsert || NULL == pclTableWhereToInsert )
	{
		ASSERT_RETURN;
	}

	CTableHM *pclHydraulicNetwork = dynamic_cast<CTableHM *>( m_pTADS->GetpHydroModTable() );

	if( NULL == pclHydraulicNetwork )
	{
		ASSERT_RETURN;
	}

	m_mmapInjectionHydraulicCircuitWithTemperatureError.clear();
	m_vecAllInjectionCircuitsWithTemperatureError.clear();

	bool bIsInjectionCircuitTemperatureError = pclHydraulicNetwork->IsInjectionCircuitTemperatureErrorForPasteOperation( pclObjectToInsert, pclTableWhereToInsert, 
		&m_mmapInjectionHydraulicCircuitWithTemperatureError, &m_vecAllInjectionCircuitsWithTemperatureError );

	m_iDlgInjectionErrorReturnCode = (int)CDlgInjectionError::ReturnCode::Undefined;

	if( true == bIsInjectionCircuitTemperatureError )
	{
		CDlgInjectionError DlgInjectionError( &m_vecAllInjectionCircuitsWithTemperatureError );
		m_iDlgInjectionErrorReturnCode = (int)(CDlgInjectionError::ReturnCode)DlgInjectionError.DoModal();

		if( (int)CDlgInjectionError::ReturnCode::Cancel == m_iDlgInjectionErrorReturnCode )
		{
			return;
		}

		// The correction will be applied after the insertion.
	}
}

void CPanelCirc2::_CorrectInjectionTemperaturesAfterInsert( CDS_HydroMod *pclObjectInserted, CTable *pclTableWhereInserted )
{
	if( NULL == pclObjectInserted || NULL == pclTableWhereInserted )
	{
		ASSERT_RETURN;
	}

	CTableHM *pclHydraulicNetwork = dynamic_cast<CTableHM *>( m_pTADS->GetpHydroModTable() );

	if( NULL == pclHydraulicNetwork )
	{
		ASSERT_RETURN;
	}

	if( 0 != m_mmapInjectionHydraulicCircuitWithTemperatureError.count( pclObjectInserted ) )
	{
		// First we will extract the range in the multimap that is corresponding to the object we have pasted.
		// Remark: we can past one object in multi destinations. This is why we use a multimap.
		std::pair<std::multimap<CDS_HydroMod *, CTable *>::iterator, std::multimap<CDS_HydroMod *, CTable *>::iterator> ret;
		ret = m_mmapInjectionHydraulicCircuitWithTemperatureError.equal_range( (CDS_HydroMod *)( pclObjectInserted->GetIDPtr().MP ) );

		for( std::multimap<CDS_HydroMod *, CTable *>::iterator iter = ret.first; iter !=ret.second; iter++ )
		{
			// Now, for the object we check the one that is corresponding with the current selected line (or parent) with 'pDestTable'.
			if( iter->second == pclTableWhereInserted )
			{
				pclHydraulicNetwork->CorrectOneInjectionCircuit( pclObjectInserted, iter->second );
				m_mmapInjectionHydraulicCircuitWithTemperatureError.erase( iter );
				break;
			}
		}
	}
}
