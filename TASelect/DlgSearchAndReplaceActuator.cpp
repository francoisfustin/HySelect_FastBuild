#include "stdafx.h"
#include "afxdialogex.h"
#include "TASelect.h"
#include "MainFrm.h"
#include "HydroMod.h"
#include "SelProdPageBase.h"
#include "SelProdDockPane.h"
#include "RViewSelProd.h"
#include "DlgSearchReplace.h"
#include "DlgSearchAndReplaceActuator.h"


IMPLEMENT_DYNAMIC( CDlgSearchAndReplaceActuator, CDialogEx )

CDlgSearchAndReplaceActuator::CDlgSearchAndReplaceActuator( CWnd* pParent )
	: CDialogEx( CDlgSearchAndReplaceActuator::IDD, pParent )
{
	m_pParent = pParent;
	m_eWorkingOn = WorkingOn::WO_HM;
	m_eWorkingMode = WorkingMode::WM_Add;
	m_bInitialized = false;
	m_iListSrcTitleID = 0;
	m_iListTrgTitleID = 0;
	m_iNbrRow = 0;
	m_iLeftPos = 0;
	m_iRightPos = 0;
	m_bSetNotAvailableOrDeleted = false;
	m_iCurrentSourceSelected = -1;
	m_pvecHMList = NULL;
	m_vecHMListCopy.clear();
	m_bHMSrcSelectedAsaPackage = false;
	m_pHMCvActrSet = NULL;
	m_pvecSelCtrlList = NULL;
	m_vecSelCtrlListCopy.clear();
	m_mapChangeStatus.clear();
	m_pPowerSupplyIDPtr = NULL;
	m_pInputSignalIDPtr = NULL;
	m_bDowngradeFunctionalities = false;
	m_bForceSetWhenPossible = false;
	m_bFailSafeFct = false;
	m_vecActuatorSolution.clear();
	m_vecCVActrSetSolution.clear();
	m_cTextForeColor = 0;
	m_cTextBackColor = 0;
	m_iCurrentSolution = -1;
	m_bSetForced = false;
	m_mapDowngradeStatus.clear();
	m_pclAdapter = NULL;
	m_bAdapterBelongsToSet = false;
	m_vecCVSetAccessoryList.clear();
	m_iTotalChange = 0;
	m_eDefaultReturnPosFct = CDB_ControlValve::DRPFunction::drpfUndefined;
}

CDlgSearchAndReplaceActuator::~CDlgSearchAndReplaceActuator()
{
	CTable *pTable = (CTable *)( TASApp.GetpTADS()->Get( _T("DLGSRACTR_HMCOPY") ).MP );

	if( NULL != pTable )
	{
		TASApp.GetpTADS()->DeleteObjectRecursive( pTable, true );
	}

	pTable = (CTable *)( TASApp.GetpTADS()->Get( _T("DLGSRACTR_SELCTRLCOPY") ).MP );

	if( NULL != pTable )
	{
		TASApp.GetpTADS()->DeleteObjectRecursive( pTable, true );
	}
}


bool CDlgSearchAndReplaceActuator::SetParams( std::vector<LPARAM>* pvecParams )
{
	try
	{
		int iNbrParams = ( WorkingMode::WM_Remove == m_eWorkingMode ) ? 2 : 8;

		if( NULL == pvecParams || iNbrParams != pvecParams->size() )
		{
			return false;
		}
	
		m_eWorkingOn = ( CDlgSearchReplace::RadioState::RS_HMCalc == (CDlgSearchReplace::RadioState)( pvecParams->at( 0 ) ) ) ? WorkingOn::WO_HM : WorkingOn::WO_IndSel;
		
		if( WorkingOn::WO_HM == m_eWorkingOn )
		{
			m_pvecHMList = (std::vector<CDS_HydroMod *>*)( pvecParams->at( 1 ) );

			// Verify if copy is not already exist.
			CTable *pTable = (CTable *)( TASApp.GetpTADS()->Get( _T("DLGSRACTR_HMCOPY") ).MP );
			
			if( NULL != pTable )
			{
				pTable->MakeEmpty();
			}
			else
			{
				IDPTR IDPtr;
				TASApp.GetpTADS()->CreateObject( IDPtr, CLASS( CTable ), _T("DLGSRACTR_HMCOPY") );
				TASApp.GetpTADS()->Access().Insert( IDPtr );
				pTable = (CTable*)( IDPtr.MP );
			}
		
			for( int i = 0; i < (int)m_pvecHMList->size(); i++ )
			{
				CDS_HydroMod *pclHMSrc = m_pvecHMList->at( i );

				IDPTR IDPtr = TASApp.GetpTADS()->CreateCopyHM( pclHMSrc->GetIDPtr(), pTable, true );

				if( _NULL_IDPTR == IDPtr || NULL == IDPtr.MP )
				{
					continue;
				}

				m_vecHMListCopy.push_back( (CDS_HydroMod *)IDPtr.MP );

				// Initialize maps.
				m_mapChangeStatus[i] = false;
			}
		}
		else
		{
			m_pvecSelCtrlList = (std::vector<CDS_SSelCtrl*>*)( pvecParams->at( 1 ) );
	
			// Verify if copy is not already exist.
			CTable *pTable = (CTable *)( TASApp.GetpTADS()->Get( _T("DLGSRACTR_SELCTRLCOPY") ).MP );

			if( NULL != pTable )
			{
				pTable->MakeEmpty();
			}
			else
			{
				IDPTR IDPtr;
				TASApp.GetpTADS()->CreateObject( IDPtr, CLASS( CTable ), _T("DLGSRACTR_SELCTRLCOPY") );
				TASApp.GetpTADS()->Access().Insert( IDPtr );
				pTable = (CTable*)( IDPtr.MP );
			}
	
			for( int i = 0; i < (int)m_pvecSelCtrlList->size(); i++ )
			{
				CDS_SSelCtrl *pclSSelCtrlSrc = m_pvecSelCtrlList->at( i );

				IDPTR SSelCtrlIDPtr = _NULL_IDPTR;

				if( 0 == CString( pclSSelCtrlSrc->GetClassName() ).Compare( _T("CDS_SSelCv") ) )
				{
					TASApp.GetpTADS()->CreateObject( SSelCtrlIDPtr, CLASS( CDS_SSelCv ) );
				}
				else if( 0 == CString( pclSSelCtrlSrc->GetClassName() ).Compare( _T("CDS_SSelBCv") ) )
				{
					TASApp.GetpTADS()->CreateObject( SSelCtrlIDPtr, CLASS( CDS_SSelBCv ) );
				}
				else if( 0 == CString( pclSSelCtrlSrc->GetClassName() ).Compare( _T("CDS_SSelPICv") ) )
				{
					TASApp.GetpTADS()->CreateObject( SSelCtrlIDPtr, CLASS( CDS_SSelPICv ) );
				}
				else
				{
					return false;
				}
		
				pTable->Insert( SSelCtrlIDPtr );
			
				CDS_SSelCtrl *pclSSelCtrlTrg = (CDS_SSelCtrl *)( SSelCtrlIDPtr.MP );
				pclSSelCtrlSrc->Copy( pclSSelCtrlTrg );
				m_vecSelCtrlListCopy.push_back( pclSSelCtrlTrg );

				// Initialize maps.
				m_mapChangeStatus[i] = false;
			}
		}
	
		if( WorkingMode::WM_Add == m_eWorkingMode )
		{
			m_pPowerSupplyIDPtr = (IDPTR*)( pvecParams->at( 2 ) );
			m_pInputSignalIDPtr = (IDPTR*)( pvecParams->at( 3 ) );
			m_bFailSafeFct = (bool)( (LPARAM)0 == pvecParams->at( 4 ) ? false : true );
			m_eDefaultReturnPosFct = ( CDB_ControlValve::DRPFunction )( pvecParams->at( 5 ) );
			m_bDowngradeFunctionalities = (bool)( (LPARAM)0 == pvecParams->at(6 ) ? false : true );
			m_bForceSetWhenPossible = (bool)( (LPARAM)0 == pvecParams->at( 7 ) ? false : true );
		}

		return true;
	}
	catch( CHySelectException &clHySelectException )
	{
		clHySelectException.AddMessage( _T("Error in 'CDlgSearchAndReplaceActuator::SetParams'."), __LINE__, __FILE__ );
		throw;
	}
}

BEGIN_MESSAGE_MAP( CDlgSearchAndReplaceActuator, CDialogEx )
	ON_WM_MOVE()
	ON_WM_VSCROLL()
	ON_BN_CLICKED( IDC_BUTAPPLY, OnClickedButApply )
	ON_BN_CLICKED( IDC_BUTAPPLYALL, OnClickedButApplyAll )
	ON_BN_CLICKED( IDC_BUTPREV, OnClickedButPrevious )
	ON_BN_CLICKED( IDC_BUTNEXT, OnClickedButNext )
	ON_BN_CLICKED( IDC_BUTCLOSE, OnClickedButClose )
END_MESSAGE_MAP()

void CDlgSearchAndReplaceActuator::DoDataExchange( CDataExchange* pDX )
{
	CDialogEx::DoDataExchange( pDX );
	DDX_Control( pDX, IDC_STATICCVSRC, m_StaticCVSrc );
	DDX_Control( pDX, IDC_STATICCVTRG, m_StaticCVTrg );
	DDX_Control( pDX, IDC_LISTCVSRC, m_ListCVSrc );
	DDX_Control( pDX, IDC_LISTCVTRG, m_ListCVTrg );
	DDX_Control( pDX, IDC_STATICNOSOLUTION, m_StaticNoSolution );
	DDX_Control( pDX, IDC_BUTAPPLY, m_ButApply );
	DDX_Control( pDX, IDC_BUTAPPLYALL, m_ButApplyAll );
	DDX_Control( pDX, IDC_BUTPREV, m_ButPrevious );
	DDX_Control( pDX, IDC_BUTNEXT, m_ButNext );
	DDX_Control( pDX, IDC_BUTCLOSE, m_ButClose );
	DDX_Control( pDX, IDC_LISTCHANGEDONE, m_ListChangeDone );
	DDX_Control( pDX, IDC_SCROLLBAR, m_ScrollBar );
}

BOOL CDlgSearchAndReplaceActuator::OnInitDialog()
{
	CDialogEx::OnInitDialog();
	SetWindowText( TASApp.LoadLocalizedString( ( WorkingMode::WM_Remove == m_eWorkingMode ) ? IDS_DLGSEARCHREPLACE_ACTUATORREMOVE : IDS_DLGSEARCHREPLACE_ACTUATORADD ) );

	// Set the window position to the last stored position in registry. If window position is not yet stored in the registry, the 
	// window is centered by default in the mainframe rect.
 	CRect apprect, rect;
 	::AfxGetApp()->m_pMainWnd->GetWindowRect( &apprect );
 	GetWindowRect( &rect );

	// Set the window size to the last stored size in registry.
	int x = ::AfxGetApp()->GetProfileInt( _T("DlgSearchAndReplaceActuator"), _T("ScreenXPos"), apprect.CenterPoint().x - rect.Width() / 2 );
	int y = ::AfxGetApp()->GetProfileInt( _T("DlgSearchAndReplaceActuator"), _T("ScreenYPos"), apprect.CenterPoint().y - rect.Height() / 2 );
	SetWindowPos( NULL, x, y, 0, 0, SWP_NOZORDER | SWP_NOSIZE );

	// Title of the left list.
	m_iListSrcTitleID = ( WorkingOn::WO_HM == m_eWorkingOn ) ? IDS_DLGSRACTUATOR_HMTITLE : IDS_DLGSRACTUATOR_INDSELTITLE;

	// Title of the right list.
	m_iListTrgTitleID = ( WorkingMode::WM_Remove == m_eWorkingMode ) ? IDS_DLGSRACTUATOR_CVTITLE : IDS_DLGSRACTUATOR_CVACTRTITLE;

	// Prepare the left list.
	CString str;
	str.Format( TASApp.LoadLocalizedString( IDS_DLGSRACTUATOR_ARTICLENUMBER ), 0 );
	m_ListCVSrc.InsertColumn( 0, str, LVCFMT_LEFT, DLGSRACTR_LEFTCOLUMNWIDTH, 0 );
	str.Format( TASApp.LoadLocalizedString( IDS_DLGSRACTUATOR_ARTICLEDETAILS ), 1 );
	m_ListCVSrc.InsertColumn( 1, str, LVCFMT_LEFT, DLGSRACTR_RIGHTCOLUMNWIDTH, 0 );
	m_ListCVSrc.ModifyStyle( 0, LVS_REPORT ); 
	m_ListCVSrc.SetExtendedStyle( m_ListCVSrc.GetExtendedStyle() | LVS_EX_FULLROWSELECT );
	m_ListCVSrc.SetClickListCtrlDisable( true );

	// Prepare the right list.
	str.Format( TASApp.LoadLocalizedString( IDS_DLGSRACTUATOR_ARTICLENUMBER ), 0 );
	m_ListCVTrg.InsertColumn( 0, str, LVCFMT_LEFT, DLGSRACTR_LEFTCOLUMNWIDTH, 0 );
	str.Format( TASApp.LoadLocalizedString( IDS_DLGSRACTUATOR_ARTICLEDETAILS ), 1 );
	m_ListCVTrg.InsertColumn( 1, str, LVCFMT_LEFT, DLGSRACTR_RIGHTCOLUMNWIDTH, 0 );
	m_ListCVTrg.ModifyStyle( 0, LVS_REPORT ); 
	m_ListCVTrg.SetExtendedStyle( m_ListCVTrg.GetExtendedStyle() | LVS_EX_FULLROWSELECT );
	m_ListCVTrg.SetClickListCtrlDisable( true );

	// Prepare the no solution text.
	m_StaticNoSolution.ShowWindow( SW_HIDE );
	m_StaticNoSolution.SetBackColor( _WHITE );

	// Prepare the bottom list.
	m_ListChangeDone.GetClientRect( &rect );
	m_ListChangeDone.InsertColumn( 0, str, LVCFMT_LEFT, rect.Width(), 0 );
	m_ListChangeDone.ModifyStyle( 0, LVS_REPORT ); 
	m_ListChangeDone.SetExtendedStyle( m_ListCVTrg.GetExtendedStyle() | LVS_EX_FULLROWSELECT );
	m_ListChangeDone.SetClickListCtrlDisable( true );

	m_ButApply.SetWindowText( TASApp.LoadLocalizedString( IDS_BUTTON_APPLY ) );
	m_ButApplyAll.SetWindowText( TASApp.LoadLocalizedString( IDS_BUTTON_APPLYALL ) );
	m_ButPrevious.SetWindowText( TASApp.LoadLocalizedString( IDS_BUTTON_PREV ) );
	m_ButNext.SetWindowText( TASApp.LoadLocalizedString( IDS_BUTTON_NEXT ) );
	m_ButClose.SetWindowText( TASApp.LoadLocalizedString( IDS_CLOSE ) );
	
	// By default, disable the scrollbar.
	m_ScrollBar.EnableWindow( FALSE );

	if( ( NULL != m_pvecHMList && m_pvecHMList->size() > 0 )
			|| ( NULL != m_pvecSelCtrlList && m_pvecSelCtrlList->size() > 0 ) )
	{
		if( true == _GetFirstSelection( ) )
		{
			_ShowSolution();
		}
		else
		{
			// Show no control valve -> MUST NEVER APPEARS!
		}
	}

	// HYS-1336 : If the background color is not defined, the 'CExtStatic::CtlColor' considers the object as transparent.
	// So the text will overlapped with the previous each time it changes.
	m_StaticCVSrc.SetBackColor( ::GetSysColor( COLOR_3DFACE ) );
	m_StaticCVTrg.SetBackColor( ::GetSysColor( COLOR_3DFACE ) );

	m_bInitialized = true;
	return TRUE;
}

void CDlgSearchAndReplaceActuator::OnCancel()
{
	TASApp.GetpTADS()->RefreshResults( true );
	
	if( NULL != pRViewSelProd )
	{
		pRViewSelProd->RedrawRightView();
	}

	CDialogEx::OnCancel();
}

void CDlgSearchAndReplaceActuator::OnMove( int x, int y )
{
	CDialogEx::OnMove( x, y );

	if( true == m_bInitialized )
	{
		CRect rect;
		
		GetWindowRect( &rect );
		ScreenToClient( &rect );
		
		x += rect.left;
		y += rect.top;
		::AfxGetApp()->WriteProfileInt( _T("DlgSearchAndReplaceActuator"), _T("ScreenXPos"), x );
		::AfxGetApp()->WriteProfileInt( _T("DlgSearchAndReplaceActuator"), _T("ScreenYPos"), y );
	}
}

void CDlgSearchAndReplaceActuator::OnVScroll( UINT nSBCode, UINT nPos, CScrollBar* pScrollBar )
{
	if( pScrollBar != &m_ScrollBar )
		return CDialogEx::OnVScroll( nSBCode, nPos, &m_ScrollBar );

	SCROLLINFO rScrollInfo;
	m_ScrollBar.GetScrollInfo( &rScrollInfo, SIF_ALL );

	// Retrieve current position.
	int nPreviousPos = pScrollBar->GetScrollPos();

	switch( nSBCode )
	{
		// User has clicked on arrow down.
		case SB_PAGEDOWN:
		case SB_LINEDOWN:
			nPos = min( nPreviousPos + 1, rScrollInfo.nMax );
			break;

		// User has clicked on arrow up.
		case SB_PAGEUP:
		case SB_LINEUP:
			nPos = max( nPreviousPos - 1, rScrollInfo.nMin );
			break;
	}
	
	if( SB_ENDSCROLL != nSBCode )
	{
		m_ScrollBar.SetScrollPos( nPos );
		if( ( ( false == m_bSetForced && nPos < (UINT)m_vecActuatorSolution.size() ) ||
			  ( true == m_bSetForced && nPos < (UINT)m_vecCVActrSetSolution.size() ) ) &&
			nPreviousPos != nPos )
		{
			m_ListCVTrg.DeleteAllItems();
			_GetSolution( nPos );
			_FillDstList();
		}
	}
}

void CDlgSearchAndReplaceActuator::OnClickedButApply()
{
	switch( m_eWorkingOn )
	{
		case WorkingOn::WO_HM:
	 		_ApplyOnHM();
			break;

		case WorkingOn::WO_IndSel:
			_ApplyOnIndSel();
			break;
	}

	if( true == _IsNextSelectionExist() )
		OnClickedButNext();
	else
	{
		m_ButApply.EnableWindow( FALSE );
		m_ButApplyAll.EnableWindow( FALSE );
		m_ButNext.EnableWindow( FALSE );
		// Refresh the solution to display it in grayed.
		_ShowSolution();
	}
}

void CDlgSearchAndReplaceActuator::OnClickedButApplyAll()
{
	_ApplyAll();
	m_ButApply.EnableWindow( FALSE );
	m_ButApplyAll.EnableWindow( FALSE );
	m_ButNext.EnableWindow( FALSE );
}

void CDlgSearchAndReplaceActuator::OnClickedButPrevious()
{
	m_ListCVSrc.DeleteAllItems();
	m_ListCVTrg.DeleteAllItems();
	m_ButApply.EnableWindow( TRUE );
	m_ButApplyAll.EnableWindow( TRUE );
	m_StaticNoSolution.ShowWindow( SW_HIDE );

	if( true == _GetPreviousSelection( ) )
	{
		_ShowSolution();
	}
}

void CDlgSearchAndReplaceActuator::OnClickedButNext()
{
	m_ListCVSrc.DeleteAllItems();
	m_ListCVTrg.DeleteAllItems();
	m_ButApply.EnableWindow( TRUE );
	m_ButApplyAll.EnableWindow( TRUE );
	m_StaticNoSolution.ShowWindow( SW_HIDE );

	if( true == _GetNextSelection( ) )
	{
		_ShowSolution();
	}
}

void CDlgSearchAndReplaceActuator::OnClickedButClose()
{
	CDlgSearchAndReplaceActuator::OnCancel();
}

void CDlgSearchAndReplaceActuator::_FillSrcList()
{
	// Prepare variables.
	int iTotalSource = -1;
	CDS_HydroMod* pHM = NULL;
	CDS_SSelCtrl* pclCurrentSelCtrl = NULL;
	CDB_ControlValve* pclControlValve = NULL;
	CAccessoryList rCVAccessoryList;
	CAccessoryList* pclCVAccessoryList = NULL;
	CDB_Actuator* pclActuator = NULL;
	CAccessoryList rActuatorAccessoryList;
	CAccessoryList* pclActuatorAccessoryList = NULL;
	bool fIsSelectedAsPackage = false;
	m_bHMSrcSelectedAsaPackage = false;
	m_pHMCvActrSet = NULL;
	CDB_Set* pCVActrSet = NULL;
	bool fForSet = false;
	if( WorkingOn::WO_HM == m_eWorkingOn )
	{
		iTotalSource = (int)m_pvecHMList->size();
		pHM = m_vecHMListCopy[m_iCurrentSourceSelected];
		if( NULL == pHM )
			return;
		
		bool fUserWantedAPackage = (eBool3::eb3True == pHM->GetpCV()->GetActrSelectedAsaPackage() ) ? true : false;
		pclControlValve = pHM->GetpCV()->GetpCV();
		if( NULL == pclControlValve )
			return;

		// Cv accessories.
		if( pHM->GetpCV()->GetCVAccCount() > 0 )
		{
			CDB_RuledTable* pclRuledTable = dynamic_cast<CDB_RuledTable*>( pclControlValve->GetAccessoriesGroupIDPtr().MP );
			if( NULL != pclRuledTable )
			{
				for( int iLoopCVAcc = 0; iLoopCVAcc < pHM->GetpCV()->GetCVAccCount(); iLoopCVAcc++ )
				{
					CDB_Product* pclAccessory = (CDB_Product*)( pHM->GetpCV()->GetCVAccIDPtr( iLoopCVAcc ).MP );
					if( NULL == pclAccessory || false == pclAccessory->IsAnAccessory() )
						continue;

					rCVAccessoryList.Add( pclAccessory->GetIDPtr(), CAccessoryList::_AT_Accessory, pclRuledTable );
					pclCVAccessoryList = &rCVAccessoryList;
				}
			}
		}

		// Adapter.
		if( _NULL_IDPTR != pHM->GetpCV()->GetActrAdaptIDPtr() )
		{
			rCVAccessoryList.Add( pHM->GetpCV()->GetActrAdaptIDPtr(), CAccessoryList::_AT_Adapter );
			pclCVAccessoryList = &rCVAccessoryList;
		}

		// Actuator accessories.
		if( pHM->GetpCV()->GetActrAccCount() > 0 )
		{
			for( int iLoopActrAcc = 0; iLoopActrAcc < pHM->GetpCV()->GetActrAccCount(); iLoopActrAcc++ )
			{
				CDB_Product* pclAccessory = (CDB_Product*)( pHM->GetpCV()->GetActrAccIDPtr( iLoopActrAcc ).MP );
				if( NULL == pclAccessory || false == pclAccessory->IsAnAccessory() )
					continue;

				CDB_RuledTable* pclRuledTable = dynamic_cast<CDB_RuledTable*>( pclActuator->GetAccessoriesGroupIDPtr().MP );
				if( NULL == pclRuledTable )
					continue;

				rActuatorAccessoryList.Add( pclAccessory->GetIDPtr(), CAccessoryList::_AT_Accessory, pclRuledTable );
				pclActuatorAccessoryList = &rActuatorAccessoryList;
			}
		}
		
		if( WorkingMode::WM_Remove == m_eWorkingMode )
		{
			pclActuator = (CDB_Actuator*)( pHM->GetpCV()->GetActrIDPtr().MP );
			if( NULL == pclActuator )
				return;

			if( true == fUserWantedAPackage )
			{
				CTableSet* pCVActSetTab = pclControlValve->GetTableSet();				ASSERT( NULL != pCVActSetTab );
				if( NULL == pCVActSetTab )
					return;

				pCVActrSet = pCVActSetTab->FindCompatibleSet( pclControlValve->GetIDPtr().ID, pclActuator->GetIDPtr().ID );
				m_pHMCvActrSet = pCVActrSet;
				fIsSelectedAsPackage = ( NULL != pCVActrSet ) ? true : false;
				m_bHMSrcSelectedAsaPackage = fIsSelectedAsPackage;
				fForSet = fIsSelectedAsPackage;

				// CV-Actuator accessories set.
				if( NULL != pCVActrSet )
				{
					CDB_RuledTable* pclRuledTable = dynamic_cast<CDB_RuledTable*>( pCVActrSet->GetAccGroupIDPtr().MP );
					if( NULL != pclRuledTable && pHM->GetpCV()->GetCVActrAccSetCount() > 0 )
					{
						for( int iLoopCVActrAccSet = 0; iLoopCVActrAccSet < pHM->GetpCV()->GetCVActrAccSetCount(); iLoopCVActrAccSet++ )
						{
							CDB_Product* pclAccessory = (CDB_Product*)( pHM->GetpCV()->GetCVActrAccSetIDPtr( iLoopCVActrAccSet ).MP );
							if( NULL == pclAccessory || false == pclAccessory->IsAnAccessory() )
								continue;

							rCVAccessoryList.Add( pclAccessory->GetIDPtr(), CAccessoryList::_AT_SetAccessory, pclRuledTable );
							pclCVAccessoryList = &rCVAccessoryList;
						}
					}
				}
			}
		}
	}
	else
	{
		iTotalSource = (int)m_pvecSelCtrlList->size();
		pclCurrentSelCtrl = m_vecSelCtrlListCopy[m_iCurrentSourceSelected];
		if( NULL == pclCurrentSelCtrl )
			return;
		
		fIsSelectedAsPackage = pclCurrentSelCtrl->IsSelectedAsAPackage();
		pclControlValve = (CDB_ControlValve*)( pclCurrentSelCtrl->GetProductIDPtr().MP );
		if( NULL == pclControlValve )
			return;
		
		if( WorkingMode::WM_Remove == m_eWorkingMode )
		{
			if( true == fIsSelectedAsPackage )
			{
				pCVActrSet = dynamic_cast<CDB_Set*>( (CData*)( pclCurrentSelCtrl->GetCvActrSetIDPtr().MP ) );
				if( NULL == pCVActrSet )
					return;
				pclActuator = (CDB_Actuator*)( pCVActrSet->GetSecondIDPtr().MP );
				fForSet = true;
			}
			else
			{
				pclActuator = (CDB_Actuator*)( pclCurrentSelCtrl->GetActrIDPtr().MP );
			}
		}

		pclCVAccessoryList = pclCurrentSelCtrl->GetCvAccessoryList();
		pclActuatorAccessoryList = pclCurrentSelCtrl->GetActuatorAccessoryList();
	}

	CString strCurrent;
	strCurrent.Format( _T("%u/%u"), m_iCurrentSourceSelected + 1, iTotalSource );
	CString str;
	if( WorkingOn::WO_HM == m_eWorkingOn )
	{
		CString strName = pHM->GetHMName();
		FormatString( str, m_iListSrcTitleID, strName, strCurrent );
	}
	else
	{
		FormatString( str, m_iListSrcTitleID, strCurrent );
	}
	m_StaticCVSrc.SetWindowText( str );
	m_ListCVSrc.ResetAllFonts();

	m_iNbrRow = 0;
	m_iLeftPos = 0;
	m_iRightPos = 0;
	m_bSetNotAvailableOrDeleted = false;

	// Fill control valve part.
	if( WorkingMode::WM_Remove == m_eWorkingMode && true == fIsSelectedAsPackage )
	{
		_FillCVArticleReferenceSet( pCVActrSet, &m_ListCVSrc );
	}
	else
	{
		_FillCVArticleReferenceNoSet( pclControlValve, &m_ListCVSrc );
	}

	// Fill control valve part common to set or no set.
	_FillCVCommonPart( pclControlValve, &m_ListCVSrc );

	// Fill control valve accessories if exist.
	if( NULL != pclCVAccessoryList && pclCVAccessoryList->GetCount( CAccessoryList::AccessoryType::_AT_Accessory ) > 0 )
	{
		CAccessoryList::AccessoryItem rAccessory = pclCVAccessoryList->GetFirst( CAccessoryList::AccessoryType::_AT_Accessory );
		while( _NULL_IDPTR != rAccessory.IDPtr )
		{
			_FillAccessoryPart( (CDB_Product*)( rAccessory.IDPtr.MP ), false, rAccessory.fByPair, &m_ListCVSrc );
			rAccessory = pclCVAccessoryList->GetNext( CAccessoryList::AccessoryType::_AT_Accessory );
		}
	}

	// Fill adapter part if exist.
	if( NULL != pclCVAccessoryList )
	{
		CAccessoryList::AccessoryItem rAccessoryItem = pclCVAccessoryList->GetFirst( CAccessoryList::AccessoryType::_AT_Adapter );
		if( NULL != rAccessoryItem.IDPtr.MP )
		{
			_FillAdapterPart( (CDB_Product*)( rAccessoryItem.IDPtr.MP ), fForSet, &m_ListCVSrc );
		}
	}

	// Fill actuator part.
	if( WorkingMode::WM_Remove == m_eWorkingMode )
	{
		_FillActuatorPart( pclActuator, pclControlValve, fForSet, &m_ListCVSrc );
	}

	// Fill actuator accessories if exist.
	if( NULL != pclActuatorAccessoryList && pclActuatorAccessoryList->GetCount( CAccessoryList::AccessoryType::_AT_Accessory ) > 0 )
	{
		CAccessoryList::AccessoryItem rAccessory = pclActuatorAccessoryList->GetFirst( CAccessoryList::AccessoryType::_AT_Accessory );
		while( _NULL_IDPTR != rAccessory.IDPtr )
		{
			_FillAccessoryPart( (CDB_Product*)( rAccessory.IDPtr.MP ), fForSet, rAccessory.fByPair, &m_ListCVSrc );
			rAccessory = pclActuatorAccessoryList->GetNext( CAccessoryList::AccessoryType::_AT_Accessory );
		}
	}

	// Fill CV-actuator set accessories if exist.
	if( NULL != pclCVAccessoryList && true == fForSet && pclCVAccessoryList->GetCount( CAccessoryList::AccessoryType::_AT_SetAccessory ) > 0 )
	{
		CAccessoryList::AccessoryItem rAccessory = pclCVAccessoryList->GetFirst( CAccessoryList::AccessoryType::_AT_SetAccessory );
		while( _NULL_IDPTR != rAccessory.IDPtr )
		{
			_FillAccessoryPart( (CDB_Product*)( rAccessory.IDPtr.MP ), true, rAccessory.fByPair, &m_ListCVSrc );
			rAccessory = pclCVAccessoryList->GetNext( CAccessoryList::AccessoryType::_AT_SetAccessory );
		}
	}

	// Fill quantity for individual selection only.
	if( WorkingOn::WO_IndSel == m_eWorkingOn && NULL != pclCurrentSelCtrl && NULL != pclCurrentSelCtrl->GetpSelectedInfos() )
	{
		_FillQuantity( pclCurrentSelCtrl->GetpSelectedInfos()->GetQuantity(), &m_ListCVSrc );
	}

	m_ButNext.EnableWindow( ( true == _IsNextSelectionExist() ) ? TRUE : FALSE );
	m_ButPrevious.EnableWindow( ( true == _IsPreviousSelectionExist() ) ? TRUE : FALSE );
 	if( iTotalSource < 2 )
 		m_ButApplyAll.EnableWindow( FALSE );

	if( WorkingOn::WO_HM == m_eWorkingOn )
		m_pParent->SendMessage( WM_USER_REPLACEPOPUPFINDNEXT, (WPARAM)m_pvecHMList->at( m_iCurrentSourceSelected ), NULL );
}

void CDlgSearchAndReplaceActuator::_FillDstList()
{
	m_iNbrRow = 0;
	m_iLeftPos = 0;
	m_iRightPos = 0;
	m_vecCVSetAccessoryList.clear();

	// Prepare variables.
	int iTotalSource = -1;
	CDS_SSelCtrl* pclCurrentSelCtrl = NULL;
	CDB_ControlValve* pclControlValve = NULL;
	CAccessoryList rCVAccessoryList;
	CAccessoryList* pclCVAccessoryList = NULL;
	CDB_Actuator* pclActuator = NULL;
	bool fIsSelectedAsPackage = false;
	CDB_Set* pCVActSet = NULL;
	switch( m_eWorkingOn )
	{
		case WorkingOn::WO_HM:
			{
				iTotalSource = (int)m_pvecHMList->size();
				CDS_HydroMod* pHM = m_pvecHMList->at( m_iCurrentSourceSelected );
				if( NULL == pHM )
					return;

				fIsSelectedAsPackage = (eBool3::eb3True == pHM->GetpCV()->GetActrSelectedAsaPackage() ) ? true : false;
				pclControlValve = pHM->GetpCV()->GetpCV();
				if( NULL == pclControlValve )
					return;

				// Cv accessories.
				if( pHM->GetpCV()->GetCVAccCount() > 0 )
				{
					for( int iLoopCVAcc = 0; iLoopCVAcc < pHM->GetpCV()->GetCVAccCount(); iLoopCVAcc++ )
					{
						CDB_Product* pclAccessory = (CDB_Product*)( pHM->GetpCV()->GetCVAccIDPtr( iLoopCVAcc ).MP );
						if( NULL == pclAccessory || false == pclAccessory->IsAnAccessory() )
							continue;

						CDB_RuledTable* pclRuledTable = dynamic_cast<CDB_RuledTable*>( pclControlValve->GetAccessoriesGroupIDPtr().MP );
						if( NULL == pclRuledTable )
							continue;

						rCVAccessoryList.Add( pclAccessory->GetIDPtr(), CAccessoryList::_AT_Accessory, pclRuledTable );
						pclCVAccessoryList = &rCVAccessoryList;
					}
				}

				if( WorkingMode::WM_Add == m_eWorkingMode )
				{
					if( true == m_mapChangeStatus[m_iCurrentSourceSelected] )
					{
						pclActuator = (CDB_Actuator*)( pHM->GetpCV()->GetActrIDPtr().MP );
						if( true == m_bSetForced )
						{
							CTableSet* pCVActSetTab = pclControlValve->GetTableSet();				ASSERT( NULL != pCVActSetTab );
							if( NULL == pCVActSetTab )
								return;
							pCVActSet = pCVActSetTab->FindCompatibleSet( pclControlValve->GetIDPtr().ID, pclActuator->GetIDPtr().ID );
							if( NULL == pCVActSet )
								return;
						}
					}
					else if( false == m_bSetForced && m_iCurrentSolution < (int)m_vecActuatorSolution.size() )
					{
						pclActuator = m_vecActuatorSolution[m_iCurrentSolution];
					}
					else if( true == m_bSetForced && m_iCurrentSolution < (int)m_vecCVActrSetSolution.size() )
					{
						pCVActSet = m_vecCVActrSetSolution[m_iCurrentSolution];
					}
				}
			}
			break;

		case WorkingOn::WO_IndSel:
			{
				pclCurrentSelCtrl = (CDS_SSelCtrl*)m_pvecSelCtrlList->at( m_iCurrentSourceSelected );
				if( NULL == pclCurrentSelCtrl )
					return;

				fIsSelectedAsPackage = pclCurrentSelCtrl->IsSelectedAsAPackage();
				pclControlValve = (CDB_ControlValve*)( pclCurrentSelCtrl->GetProductIDPtr().MP );
				if( NULL == pclControlValve )
					return;

				pclCVAccessoryList = pclCurrentSelCtrl->GetCvAccessoryList();

				if( WorkingMode::WM_Add == m_eWorkingMode )
				{
					if( true == m_mapChangeStatus[m_iCurrentSourceSelected] )
					{
						if( false == m_bSetForced )
						{
							pclActuator = (CDB_Actuator*)( pclCurrentSelCtrl->GetActrIDPtr().MP );
						}
						else if( true == m_bSetForced )
						{
							pCVActSet = (CDB_Set*)( pclCurrentSelCtrl->GetCvActrSetIDPtr().MP );
						}
					}
					else if( false == m_bSetForced && m_iCurrentSolution < (int)m_vecActuatorSolution.size() )
					{
						pclActuator = m_vecActuatorSolution[m_iCurrentSolution];
					}
					else if( true == m_bSetForced && m_iCurrentSolution < (int)m_vecCVActrSetSolution.size() )
					{
						pCVActSet = m_vecCVActrSetSolution[m_iCurrentSolution];
					}
				}
			}
			break;
	}

	m_ListCVTrg.ResetAllFonts();
	switch( m_eWorkingMode )
	{
		case WorkingMode::WM_Remove:
			{
				if( true == m_mapChangeStatus[m_iCurrentSourceSelected] )
				{
					m_StaticCVTrg.SetWindowText( TASApp.LoadLocalizedString( IDS_DLGSRACTUATOR_ACTRREMOVED ) );
				}
				else
				{
					m_StaticCVTrg.SetWindowText( TASApp.LoadLocalizedString( m_iListTrgTitleID ) );
				}

				_FillCVArticleReferenceNoSet( pclControlValve, &m_ListCVTrg );
				_FillCVCommonPart( pclControlValve, &m_ListCVTrg );

				// Fill control valve accessories if exist.
				if( NULL != pclCVAccessoryList && pclCVAccessoryList->GetCount( CAccessoryList::AccessoryType::_AT_Accessory ) > 0 )
				{
					CAccessoryList::AccessoryItem rAccessory = pclCVAccessoryList->GetFirst( CAccessoryList::AccessoryType::_AT_Accessory );
					while( _NULL_IDPTR != rAccessory.IDPtr )
					{
						_FillAccessoryPart( (CDB_Product*)( rAccessory.IDPtr.MP ), false, rAccessory.fByPair, &m_ListCVTrg );
						rAccessory = pclCVAccessoryList->GetNext( CAccessoryList::AccessoryType::_AT_Accessory );
					}
				}

				// Fill adapter part if exist.
				if( NULL != pclCVAccessoryList )
				{
					CAccessoryList::AccessoryItem rAccessoryItem = pclCVAccessoryList->GetFirst( CAccessoryList::AccessoryType::_AT_Adapter );
					if( NULL != rAccessoryItem.IDPtr.MP )
					{
						_FillAdapterPart( (CDB_Product*)( rAccessoryItem.IDPtr.MP ), false, &m_ListCVTrg );
					}
				}
			}
			break;

		case WorkingMode::WM_Add:
			{
				m_StaticCVTrg.SetWindowText( TASApp.LoadLocalizedString( m_iListTrgTitleID ) );

				if( true == m_mapChangeStatus[m_iCurrentSourceSelected] )
				{
					m_StaticCVTrg.SetWindowText( TASApp.LoadLocalizedString( IDS_DLGSRACTUATOR_ACTRADDED ) );
				}
				else
				{
					CString strCounter;
					strCounter.Format( _T("%i/%i"), m_iCurrentSolution + 1, _GetNumberOfSolution() );
					CString strTitle;
					FormatString( strTitle, m_iListTrgTitleID, strCounter );
					m_StaticCVTrg.SetWindowText( strTitle );
				}
			
				if( NULL != pclActuator || NULL != pCVActSet )
				{
					if( false == m_bSetForced )
					{
						_FillCVArticleReferenceNoSet( pclControlValve, &m_ListCVTrg );
					}
					else
					{
						_FillCVArticleReferenceSet( pCVActSet, &m_ListCVTrg );
						pclActuator = (CDB_ElectroActuator*)( pCVActSet->GetSecondIDPtr().MP );
					}
					_FillCVCommonPart( pclControlValve, &m_ListCVTrg );

					// Fill control valve accessories if exist.
					if( NULL != pclCVAccessoryList && pclCVAccessoryList->GetCount( CAccessoryList::AccessoryType::_AT_Accessory ) > 0 )
					{
						CAccessoryList::AccessoryItem rAccessory = pclCVAccessoryList->GetFirst( CAccessoryList::AccessoryType::_AT_Accessory );
						while( _NULL_IDPTR != rAccessory.IDPtr )
						{
							_FillAccessoryPart( (CDB_Product*)( rAccessory.IDPtr.MP ), false, rAccessory.fByPair, &m_ListCVTrg );
							rAccessory = pclCVAccessoryList->GetNext( CAccessoryList::AccessoryType::_AT_Accessory );
						}
					}

					if( NULL != m_pclAdapter )
						_FillAdapterPart( m_pclAdapter, m_bSetForced, &m_ListCVTrg );

					_FillActuatorPart( pclActuator, pclControlValve, m_bSetForced, &m_ListCVTrg );
					if( true == m_bDowngradeFunctionalities && true == m_mapDowngradeStatus[pclActuator] )
					{
						// Add a blank line.
						DLGSRACTR_AJUSTCOORDS( m_iLeftPos, m_iRightPos );
						m_ListCVTrg.InsertItem( LVIF_TEXT | LVIF_STATE, m_iNbrRow + m_iLeftPos, _T(""), 0, LVIS_SELECTED, 0, 0 );
						m_iLeftPos = ++m_iRightPos;

						CString str = TASApp.LoadLocalizedString( IDS_DLGSRACTUATOR_DOWNGRADEFCT );
						m_ListCVTrg.SetItemTextBold( m_iNbrRow + m_iRightPos, DLGSRACTR_RIGHTCOLUMNINDEX, true );

						COLORREF crForeColor = ( false == m_mapChangeStatus[m_iCurrentSourceSelected] ) ? _RED : _LIGHTRED;
						COLORREF crBackColor = ( false == m_mapChangeStatus[m_iCurrentSourceSelected] ) ? _WHITE : GetSysColor( COLOR_3DFACE );
						m_ListCVTrg.SetItemTextColor( m_iNbrRow + m_iRightPos, DLGSRACTR_RIGHTCOLUMNINDEX, _RED, _RED );
						m_ListCVTrg.AddItemTextColorVerify( str, m_iNbrRow + m_iRightPos++, DLGSRACTR_RIGHTCOLUMNINDEX, crForeColor, crForeColor, crBackColor );

						// Add a blank line.
						DLGSRACTR_AJUSTCOORDS( m_iLeftPos, m_iRightPos );
						m_ListCVTrg.InsertItem( LVIF_TEXT | LVIF_STATE, m_iNbrRow + m_iLeftPos, _T(""), 0, LVIS_SELECTED, 0, 0 );
						m_iLeftPos = ++m_iRightPos;
					}

					// Fill CV-Actuator accessories set if exist.
					if( true == m_bSetForced )
					{
						_FillCVActrAccSetPart( pCVActSet, &m_ListCVTrg );
					}
				}
			}
			break;
	}

	// Fill quantity for individual selection only.
	if( WorkingOn::WO_IndSel == m_eWorkingOn && NULL != pclCurrentSelCtrl && NULL != pclCurrentSelCtrl->GetpSelectedInfos() )
	{
		_FillQuantity( pclCurrentSelCtrl->GetpSelectedInfos()->GetQuantity(), &m_ListCVTrg );
	}

	if( true == m_mapChangeStatus[m_iCurrentSourceSelected] )
	{
		m_ScrollBar.EnableWindow( FALSE );
		m_ButApply.EnableWindow( FALSE );
		m_ButApplyAll.EnableWindow( FALSE );
	}
}

void CDlgSearchAndReplaceActuator::_FillDstListNotAvailableOrDeleted()
{
	m_StaticCVTrg.SetWindowText( _T("") );
	
	m_StaticNoSolution.ShowWindow( SW_SHOW );
	m_StaticNoSolution.SetTextColor( _RED );
	m_StaticNoSolution.SetWindowText( TASApp.LoadLocalizedString( IDS_DLGSRACTUATOR_NOTAVAILORDELETED ) );
	
	m_ButApply.EnableWindow( FALSE );
	m_ButApplyAll.EnableWindow( FALSE );
	m_ScrollBar.EnableWindow( FALSE );
}

void CDlgSearchAndReplaceActuator::_FillDstListSetNoBreakable()
{
	m_StaticCVTrg.SetWindowText( _T("") );
	
	m_StaticNoSolution.ShowWindow( SW_SHOW );
	m_StaticNoSolution.SetTextColor( _BLACK );
	m_StaticNoSolution.SetWindowText( TASApp.LoadLocalizedString( IDS_DLGSRACTUATOR_NOBREAKABLE ) );
	
	m_ButApply.EnableWindow( FALSE );
	m_ButApplyAll.EnableWindow( FALSE );
	m_ScrollBar.EnableWindow( FALSE );
}

void CDlgSearchAndReplaceActuator::_FillDstListNoSolution( void )
{
	m_StaticCVTrg.SetWindowText( _T("") );
	
	m_StaticNoSolution.ShowWindow( SW_SHOW );
	m_StaticNoSolution.SetTextColor( _BLACK );
	m_StaticNoSolution.SetWindowText( TASApp.LoadLocalizedString( IDS_DLGSRACTUATOR_NOSOLUTION ) );

	m_ButApply.EnableWindow( FALSE );
	m_ButApplyAll.EnableWindow( FALSE );
	m_ScrollBar.EnableWindow( FALSE );
}

void CDlgSearchAndReplaceActuator::_FillCVArticleReferenceNoSet( CDB_ControlValve* pclControlValve, CExtListCtrl* pList )
{
	// Check if we must display reference in red.
	bool fCVNotAvailOrDeleted = ( false == pclControlValve->IsAvailable() || true == pclControlValve->IsDeleted( ) );
	if( false == m_mapChangeStatus[m_iCurrentSourceSelected] || pList == &m_ListCVSrc )
	{
		m_cTextForeColor =  ( false == fCVNotAvailOrDeleted ) ? _BLACK : _RED;
		m_cTextBackColor = _WHITE;
	}
	else
	{
		m_cTextForeColor = ( false == fCVNotAvailOrDeleted ) ? GetSysColor( COLOR_GRAYTEXT ) : _LIGHTRED;
		m_cTextBackColor = GetSysColor( CTLCOLOR_DLG );
	}

	// Check first if we have a local article number available.
	CString strLocalArticleNumber = _T("");
	if( true == TASApp.IsLocalArtNumberUsed() )
	{
		strLocalArticleNumber = LocArtNumTab.GetLocalArticleNumber( pclControlValve->GetArtNum( fCVNotAvailOrDeleted ) );
		strLocalArticleNumber.Trim();
	}

	CString strArticleNumber = _T("");
	// If we can show article number OR we can't show but there is no local article number...
	if( false == TASApp.IsTAArtNumberHidden() || ( true == TASApp.IsTAArtNumberHidden() && true == strLocalArticleNumber.IsEmpty() ) )
		strArticleNumber = pclControlValve->GetArtNum( fCVNotAvailOrDeleted );

	// Check for the local article number, and if it exist, change the name.
	if( true == TASApp.IsLocalArtNumberUsed() && false == strLocalArticleNumber.IsEmpty() )
	{
		strLocalArticleNumber = (CString)TASApp.GetLocalArtNumberName() + LocArtNumTab.GetLocalArticleNumber( pclControlValve->GetArtNum( fCVNotAvailOrDeleted ) );
		strLocalArticleNumber.Trim();
		if( false == strArticleNumber.IsEmpty() )
			strArticleNumber += (CString)_T("&") + strLocalArticleNumber;
		else
			strArticleNumber = strLocalArticleNumber;
	}

	bool fOnePrint = false;
	int iPos = 0;
	do
	{
		iPos = strArticleNumber.Find( _T("&") );
		CString str2 = strArticleNumber;
		if( iPos >= 0 )
		{
			str2 = strArticleNumber.Left( iPos );
			strArticleNumber.Delete( 0, iPos + 1 );
		}
		pList->SetItemTextBold( m_iNbrRow + m_iLeftPos, DLGSRACTR_LEFTCOLUMNINDEX, ( false == fOnePrint ) ? true : false );
		pList->AddItemTextColorVerify( str2, m_iNbrRow + m_iLeftPos++, DLGSRACTR_LEFTCOLUMNINDEX, m_cTextForeColor, m_cTextForeColor, m_cTextBackColor );
		fOnePrint = true;
	}while( iPos >= 0 );

	// If something has been printed in left column and product is not available or deleted...
	if( true == fOnePrint && true == fCVNotAvailOrDeleted )
	{
		// Remarks: if product is flagged as 'not available' and 'deleted', we show only 'deleted'!
		CString strFlag = TASApp.LoadLocalizedString( ( true == pclControlValve->IsDeleted() ) ? IDS_TAPDELETED : IDS_TAPNOTAVAILABLE );
		pList->SetItemTextBold( m_iNbrRow + m_iLeftPos, DLGSRACTR_LEFTCOLUMNINDEX, true );
		pList->AddItemTextColorVerify( strFlag, m_iNbrRow + m_iLeftPos++, DLGSRACTR_LEFTCOLUMNINDEX, m_cTextForeColor, m_cTextForeColor, m_cTextBackColor );
	}
}

void CDlgSearchAndReplaceActuator::_FillCVArticleReferenceSet( CDB_Set* pCVActSet, CExtListCtrl* pList )
{
	if( NULL == pCVActSet )
	{
		ASSERT( 0 );
		return;
	}

	// Check if we must display reference in red.
	m_bSetNotAvailableOrDeleted = ( false == pCVActSet->IsAvailable() || true == pCVActSet->IsDeleted( ) );
	if( false == m_mapChangeStatus[m_iCurrentSourceSelected] || pList == &m_ListCVSrc )
	{
		m_cTextForeColor =  ( false == m_bSetNotAvailableOrDeleted ) ? _BLACK : _RED;
		m_cTextBackColor = _WHITE;
	}
	else
	{
		m_cTextForeColor = ( false == m_bSetNotAvailableOrDeleted ) ? GetSysColor( COLOR_GRAYTEXT ) : _LIGHTRED;
		m_cTextBackColor = GetSysColor( COLOR_3DFACE );
	}

	// Check first if we have a local article number available.
	CString strLocalArticleNumber = _T("");
	if( true == TASApp.IsLocalArtNumberUsed() )
	{
		strLocalArticleNumber = LocArtNumTab.GetLocalArticleNumber( pCVActSet->GetReference() );
		strLocalArticleNumber.Trim();
	}

	// Set the reference.
	bool fArticleNumberShown = false;
	// If we can show article number OR we can't show but there is no local article number...
	if( false == TASApp.IsTAArtNumberHidden() || ( true == TASApp.IsTAArtNumberHidden() && true == strLocalArticleNumber.IsEmpty() ) )
	{
		CString str = pCVActSet->GetReference();
		if( false == str.IsEmpty() )
		{
			pList->SetItemTextBold( m_iNbrRow + m_iLeftPos, DLGSRACTR_LEFTCOLUMNINDEX, true );
			pList->AddItemTextColorVerify( str, m_iNbrRow + m_iLeftPos++, DLGSRACTR_LEFTCOLUMNINDEX, m_cTextForeColor, m_cTextForeColor, m_cTextBackColor );
			fArticleNumberShown = true;
		}
	}

	// Set local article number if allowed and exist.
	if( true == TASApp.IsLocalArtNumberUsed() && false == strLocalArticleNumber.IsEmpty() )
	{
		strLocalArticleNumber = (CString)TASApp.GetLocalArtNumberName() + LocArtNumTab.GetLocalArticleNumber( pCVActSet->GetReference() );
		strLocalArticleNumber.Trim();
		pList->SetItemTextBold( m_iNbrRow + m_iLeftPos, DLGSRACTR_LEFTCOLUMNINDEX, ( true == fArticleNumberShown ) ? false : true );
		pList->AddItemTextColorVerify( strLocalArticleNumber, m_iNbrRow + m_iLeftPos++, DLGSRACTR_LEFTCOLUMNINDEX, m_cTextForeColor, m_cTextForeColor, m_cTextBackColor );
	}

	if( true == m_bSetNotAvailableOrDeleted )
	{
		// Remarks: if product is flagged as 'not available' and 'deleted', we show only 'deleted'!
		CString strFlag = TASApp.LoadLocalizedString( ( true == pCVActSet->IsDeleted() ) ? IDS_TAPDELETED : IDS_TAPNOTAVAILABLE );
		pList->AddItemTextColorVerify( strFlag, m_iNbrRow + m_iLeftPos++, DLGSRACTR_LEFTCOLUMNINDEX, m_cTextForeColor, m_cTextForeColor, m_cTextBackColor );
	}

	// Set the name.
	CString str = pCVActSet->GetName();
	pList->AddItemTextColorVerify( str, m_iNbrRow + m_iRightPos++, DLGSRACTR_RIGHTCOLUMNINDEX, m_cTextForeColor, m_cTextForeColor, m_cTextBackColor );
	
	pList->AddItemTextColorVerify( TASApp.LoadLocalizedString( IDS_DLGSRACTUATOR_SET ), m_iNbrRow + m_iRightPos++, DLGSRACTR_RIGHTCOLUMNINDEX, m_cTextForeColor, m_cTextForeColor, m_cTextBackColor );

	DLGSRACTR_AJUSTCOORDS( m_iLeftPos, m_iRightPos );
	pList->InsertItem( LVIF_TEXT | LVIF_STATE, m_iNbrRow + m_iLeftPos, _T(""), 0, LVIS_SELECTED, 0, 0 );
	m_iLeftPos = ++m_iRightPos;

	pList->AddItemTextColorVerify( _T("-"), m_iNbrRow + m_iLeftPos++, DLGSRACTR_LEFTCOLUMNINDEX, m_cTextForeColor, m_cTextForeColor, m_cTextBackColor );
}

void CDlgSearchAndReplaceActuator::_FillCVCommonPart( CDB_ControlValve* pclControlValve, CExtListCtrl* pList )
{
	CString str = pclControlValve->GetName();
	pList->AddItemTextColorVerify( str, m_iNbrRow + m_iRightPos++, DLGSRACTR_RIGHTCOLUMNINDEX, m_cTextForeColor, m_cTextForeColor, m_cTextBackColor );

	// Set the Kvs.
	CUnitDatabase *pUnitDB = CDimValue::AccessUDB();
	if( !pUnitDB->GetDefaultUnitIndex( _C_KVCVCOEFF ) )
	{
		str = TASApp.LoadLocalizedString( IDS_DLGSRACTUATOR_PRODUCT_KVS );
		str += CString( _T(" : ") ) + WriteCUDouble( _C_KVCVCOEFF, pclControlValve->GetKvs() );
	}
	else
	{
		str = TASApp.LoadLocalizedString( IDS_DLGSRSET_PRODUCT_CV );
		str += CString( _T(" : ") ) + WriteCUDouble( _C_KVCVCOEFF, pclControlValve->GetKvs() );
	}
	pList->AddItemTextColorVerify( str, m_iNbrRow + m_iRightPos++, DLGSRACTR_RIGHTCOLUMNINDEX, m_cTextForeColor, m_cTextForeColor, m_cTextBackColor );
		
	// Set the rangeability.
	double dRangeability = pclControlValve->GetRangeability();
	if( dRangeability > 0.0 )
	{
		str = TASApp.LoadLocalizedString( IDS_DLGSRACTUATOR_RANGEABILITY );
		str += CString( _T(" : ") ) +  pclControlValve->GetStrRangeability().c_str();
		pList->AddItemTextColorVerify( str, m_iNbrRow + m_iRightPos++, DLGSRACTR_RIGHTCOLUMNINDEX, m_cTextForeColor, m_cTextForeColor, m_cTextBackColor );
	}

	// Set the leakage rate.
	if( pclControlValve->GetLeakageRate() > 0 )
	{
		str = TASApp.LoadLocalizedString( IDS_DLGSRACTUATOR_CV_LEAKAGE );
		str += CString( _T(" : ") ) + WriteCUDouble( _C_KVCVCOEFF, pclControlValve->GetLeakageRate() * 100 );
		str += CString( _T(" ") ) + TASApp.LoadLocalizedString( IDS_DLGSRACTUATOR_CV_LEAKAGEUNIT );
		pList->AddItemTextColorVerify( str, m_iNbrRow + m_iRightPos++, DLGSRACTR_RIGHTCOLUMNINDEX, m_cTextForeColor, m_cTextForeColor, m_cTextBackColor );
	}

	str = TASApp.LoadLocalizedString( IDS_DLGSRACTUATOR_PRODUCT_BDYMAT );
	str += CString( _T(" : ") ) + pclControlValve->GetBodyMaterial();
	pList->AddItemTextColorVerify( str, m_iNbrRow + m_iRightPos++, DLGSRACTR_RIGHTCOLUMNINDEX, m_cTextForeColor, m_cTextForeColor, m_cTextBackColor );
	
	str = TASApp.LoadLocalizedString( IDS_DLGSRACTUATOR_PRODUCT_CONNECT );
	str += CString( _T(" : ") ) + pclControlValve->GetConnect();
	if( 0 == IDcmp( pclControlValve->GetConnTabID(), _T("COMPONENT_TAB") ) )
		str += CString( _T(" -> ") ) + pclControlValve->GetSize();
	pList->AddItemTextColorVerify( str, m_iNbrRow + m_iRightPos++, DLGSRACTR_RIGHTCOLUMNINDEX, m_cTextForeColor, m_cTextForeColor, m_cTextBackColor );
	
	str = TASApp.LoadLocalizedString( IDS_DLGSRACTUATOR_PRODUCT_VERSION );
	str += CString( _T(" : ") ) + pclControlValve->GetVersion();
	pList->AddItemTextColorVerify( str, m_iNbrRow + m_iRightPos++, DLGSRACTR_RIGHTCOLUMNINDEX, m_cTextForeColor, m_cTextForeColor, m_cTextBackColor );
	
	str = TASApp.LoadLocalizedString( IDS_DLGSRACTUATOR_PRODUCT_PN );
	str += CString( _T(" : ") ) + pclControlValve->GetPN().c_str();
	pList->AddItemTextColorVerify( str, m_iNbrRow + m_iRightPos++, DLGSRACTR_RIGHTCOLUMNINDEX, m_cTextForeColor, m_cTextForeColor, m_cTextBackColor );

	str = TASApp.LoadLocalizedString( IDS_DLGSRACTUATOR_PRODUCT_CTRLTYPE );
	str += CString( _T(" : ") );
	CDB_ControlProperties* pclControlProperties = pclControlValve->GetCtrlProp();
	if( NULL != pclControlProperties )
	{
		str += pclControlProperties->GetCvCtrlTypeStr( pclControlProperties->GetCtrlType() ).c_str();
	}
	pList->AddItemTextColorVerify( str, m_iNbrRow + m_iRightPos++, DLGSRACTR_RIGHTCOLUMNINDEX, m_cTextForeColor, m_cTextForeColor, m_cTextBackColor );

	// Insert a final blank line.
	DLGSRACTR_AJUSTCOORDS( m_iLeftPos, m_iRightPos );
	pList->InsertItem( LVIF_TEXT | LVIF_STATE, m_iNbrRow + m_iLeftPos++, _T(""), 0, LVIS_SELECTED, 0, 0 );
	DLGSRACTR_AJUSTCOORDS( m_iLeftPos, m_iRightPos );
}

void CDlgSearchAndReplaceActuator::_FillAccessoryPart( CDB_Product* pclAccessory, bool fForSet, bool fByPair, CExtListCtrl* pList )
{
	if( NULL == pclAccessory || false == pclAccessory->IsAnAccessory() )
		return;

	// Variables.
	CString str; 
	m_iNbrRow = pList->GetItemCount();
	m_iLeftPos = 0;
	m_iRightPos = 0;

	// Check if we must display reference in red.
	bool fNotAvailOrDeleted = ( true == fForSet && true == m_bSetNotAvailableOrDeleted ) ||
							  ( false == pclAccessory->IsAvailable() || true == pclAccessory->IsDeleted( ) );
	if( false == m_mapChangeStatus[m_iCurrentSourceSelected] || pList == &m_ListCVSrc )
	{
		m_cTextForeColor =  ( false == fNotAvailOrDeleted ) ? _BLACK : _RED;
		m_cTextBackColor = _WHITE;
	}
	else
	{
		m_cTextForeColor = ( false == fNotAvailOrDeleted ) ? GetSysColor( COLOR_GRAYTEXT ) : _LIGHTRED;
		m_cTextBackColor = GetSysColor( COLOR_3DFACE );
	}

	if( true == fForSet )
	{
		// No article or local article number for set.
		pList->SetItemTextBold( m_iNbrRow + m_iLeftPos, DLGSRACTR_LEFTCOLUMNINDEX, true );
		pList->AddItemTextColorVerify( _T("-"), m_iNbrRow + m_iLeftPos++, DLGSRACTR_LEFTCOLUMNINDEX, m_cTextForeColor, m_cTextForeColor, m_cTextBackColor );
	}
	else
	{
		// Check first if we have a local article number available.
		CString strLocalArticleNumber = _T("");
		if( true == TASApp.IsLocalArtNumberUsed() )
		{
			strLocalArticleNumber = LocArtNumTab.GetLocalArticleNumber( pclAccessory->GetArtNum() );
			strLocalArticleNumber.Trim();
		}

		// Fill article number.
		bool fArticleNumberShown = false;
		// If we can show article number OR we can't show but there is no local article number...
		if( false == TASApp.IsTAArtNumberHidden() || ( true == TASApp.IsTAArtNumberHidden() && true == strLocalArticleNumber.IsEmpty() )  )
		{
			CString str = pclAccessory->GetArtNum();
			if( false == str.IsEmpty() )
			{
				pList->SetItemTextBold( m_iNbrRow + m_iLeftPos, DLGSRACTR_LEFTCOLUMNINDEX, true );
				pList->AddItemTextColorVerify( str, m_iNbrRow + m_iLeftPos++, DLGSRACTR_LEFTCOLUMNINDEX, m_cTextForeColor, m_cTextForeColor, m_cTextBackColor );
				fArticleNumberShown = true;
			}
		}

		// Set local article number if allowed and exist.
		if( true == TASApp.IsLocalArtNumberUsed() && false == strLocalArticleNumber.IsEmpty() )
		{
			strLocalArticleNumber = (CString)TASApp.GetLocalArtNumberName() + LocArtNumTab.GetLocalArticleNumber( pclAccessory->GetArtNum() );
			strLocalArticleNumber.Trim();
			pList->SetItemTextBold( m_iNbrRow + m_iLeftPos, DLGSRACTR_LEFTCOLUMNINDEX, ( true == fArticleNumberShown ) ? false : true );
			pList->AddItemTextColorVerify( strLocalArticleNumber, m_iNbrRow + m_iLeftPos++, DLGSRACTR_LEFTCOLUMNINDEX, m_cTextForeColor, m_cTextForeColor, m_cTextBackColor );
		}
	}

	// If something has been printed in left column and product is not available or deleted...
	if( true == fNotAvailOrDeleted && false == fForSet )
	{
		// Remarks: if product is flagged as 'not available' and 'deleted', we show only 'deleted'!
		CString strFlag = TASApp.LoadLocalizedString( ( true == pclAccessory->IsDeleted() ) ? IDS_TAPDELETED : IDS_TAPNOTAVAILABLE );
		pList->SetItemTextBold( m_iNbrRow + m_iLeftPos, DLGSRACTR_LEFTCOLUMNINDEX, true );
		pList->AddItemTextColorVerify( strFlag, m_iNbrRow + m_iLeftPos++, DLGSRACTR_LEFTCOLUMNINDEX, m_cTextForeColor, m_cTextForeColor, m_cTextBackColor );
	}
	
	str = ( true == fByPair ) ? _T("2x ") : _T("");
	str += pclAccessory->GetName();
	pList->SetItemTextBold( m_iNbrRow + m_iRightPos, DLGSRACTR_RIGHTCOLUMNINDEX, false );
	pList->AddItemTextColorVerify( str, m_iNbrRow + m_iRightPos++, DLGSRACTR_RIGHTCOLUMNINDEX, m_cTextForeColor, m_cTextForeColor, m_cTextBackColor );

	str = pclAccessory->GetComment();
	pList->AddItemTextColorVerify( str, m_iNbrRow + m_iRightPos++, DLGSRACTR_RIGHTCOLUMNINDEX, m_cTextForeColor, m_cTextForeColor, m_cTextBackColor );

	// Insert a final blank line.
	DLGSRACTR_AJUSTCOORDS( m_iLeftPos, m_iRightPos );
	pList->InsertItem( LVIF_TEXT | LVIF_STATE, m_iNbrRow + m_iLeftPos, _T(""), 0, LVIS_SELECTED, 0, 0 );
}

void CDlgSearchAndReplaceActuator::_FillAdapterPart( CDB_Product* pclAdapter, bool fForSet, CExtListCtrl* pList )
{
	if( NULL == pclAdapter || false == pclAdapter->IsAnAccessory() )
		return;

	// Variables.
	CString str; 
	m_iNbrRow = pList->GetItemCount();
	m_iLeftPos = 0;
	m_iRightPos = 0;

	// Check if we must display reference in red.
	bool fNotAvailOrDeleted = ( true == fForSet && true == m_bSetNotAvailableOrDeleted ) ||
							  ( false == pclAdapter->IsAvailable() || true == pclAdapter->IsDeleted( ) );
	if( false == m_mapChangeStatus[m_iCurrentSourceSelected] || pList == &m_ListCVSrc )
	{
		m_cTextForeColor =  ( false == fNotAvailOrDeleted ) ? _BLACK : _RED;
		m_cTextBackColor = _WHITE;
	}
	else
	{
		m_cTextForeColor = ( false == fNotAvailOrDeleted ) ? GetSysColor( COLOR_GRAYTEXT ) : _LIGHTRED;
		m_cTextBackColor = GetSysColor( COLOR_3DFACE );
	}
				
	if( true == fForSet )
	{
		// No article or local article number for set.
		pList->SetItemTextBold( m_iNbrRow + m_iLeftPos, DLGSRACTR_LEFTCOLUMNINDEX, true );
		pList->AddItemTextColorVerify( _T("-"), m_iNbrRow + m_iLeftPos++, DLGSRACTR_LEFTCOLUMNINDEX, m_cTextForeColor, m_cTextForeColor, m_cTextBackColor );
	}
	else
	{
		// Check first if we have a local article number available.
		CString strLocalArticleNumber = _T("");
		if( true == TASApp.IsLocalArtNumberUsed() )
		{
			strLocalArticleNumber = LocArtNumTab.GetLocalArticleNumber( pclAdapter->GetArtNum() );
			strLocalArticleNumber.Trim();
		}

		// Fill article number.
		bool fArticleNumberShown = false;
		// If we can show article number OR we can't show but there is no local article number...
		if( false == TASApp.IsTAArtNumberHidden() || ( true == TASApp.IsTAArtNumberHidden() && true == strLocalArticleNumber.IsEmpty() )  )
		{
			CString str = pclAdapter->GetArtNum();
			if( false == str.IsEmpty() )
			{
				pList->SetItemTextBold( m_iNbrRow + m_iLeftPos, DLGSRACTR_LEFTCOLUMNINDEX, true );
				pList->AddItemTextColorVerify( str, m_iNbrRow + m_iLeftPos++, DLGSRACTR_LEFTCOLUMNINDEX, m_cTextForeColor, m_cTextForeColor, m_cTextBackColor );
				fArticleNumberShown = true;
			}
		}

		// Set local article number if allowed and exist.
		if( true == TASApp.IsLocalArtNumberUsed() && false == strLocalArticleNumber.IsEmpty() )
		{
			strLocalArticleNumber = (CString)TASApp.GetLocalArtNumberName() + LocArtNumTab.GetLocalArticleNumber( pclAdapter->GetArtNum() );
			strLocalArticleNumber.Trim();
			pList->SetItemTextBold( m_iNbrRow + m_iLeftPos, DLGSRACTR_LEFTCOLUMNINDEX, ( true == fArticleNumberShown ) ? false : true );
			pList->AddItemTextColorVerify( strLocalArticleNumber, m_iNbrRow + m_iLeftPos++, DLGSRACTR_LEFTCOLUMNINDEX );
		}
	}

	// If something has been printed in left column and product is not available or deleted...
	if( true == fNotAvailOrDeleted && false == fForSet )
	{
		// Remarks: if product is flagged as 'not available' and 'deleted', we show only 'deleted'!
		CString strFlag = TASApp.LoadLocalizedString( ( true == pclAdapter->IsDeleted() ) ? IDS_TAPDELETED : IDS_TAPNOTAVAILABLE );
		pList->SetItemTextBold( m_iNbrRow + m_iLeftPos, DLGSRACTR_LEFTCOLUMNINDEX, true );
		pList->AddItemTextColorVerify( strFlag, m_iNbrRow + m_iLeftPos++, DLGSRACTR_LEFTCOLUMNINDEX, m_cTextForeColor, m_cTextForeColor, m_cTextBackColor );

	}
	
	str = pclAdapter->GetName();
	pList->SetItemTextBold( m_iNbrRow + m_iRightPos, DLGSRACTR_RIGHTCOLUMNINDEX, false );
	pList->AddItemTextColorVerify( str, m_iNbrRow + m_iRightPos++, DLGSRACTR_RIGHTCOLUMNINDEX, m_cTextForeColor, m_cTextForeColor, m_cTextBackColor );

	str = pclAdapter->GetComment();
	pList->AddItemTextColorVerify( str, m_iNbrRow + m_iRightPos++, DLGSRACTR_RIGHTCOLUMNINDEX, m_cTextForeColor, m_cTextForeColor, m_cTextBackColor );

	// Insert a blank line.
	DLGSRACTR_AJUSTCOORDS( m_iLeftPos, m_iRightPos );
	pList->InsertItem( LVIF_TEXT | LVIF_STATE, m_iNbrRow + m_iLeftPos++, _T(""), 0, LVIS_SELECTED, 0, 0 );
}

void CDlgSearchAndReplaceActuator::_FillActuatorPart( CDB_Actuator* pclActuator, CDB_ControlValve* pclControlValve, bool fForSet, CExtListCtrl* pList )
{
	if( NULL == pclActuator || NULL == pclControlValve || NULL == pList )
		return;

	// Variables.
	CString str; 
	m_iNbrRow = pList->GetItemCount();
	m_iLeftPos = 0;
	m_iRightPos = 0;

	////////////////////////////////////////////////////////////////////////////////////////////////////////////////
	// Fill the left part.
	////////////////////////////////////////////////////////////////////////////////////////////////////////////////

	// Check if we must display reference in red.
	bool fNotAvailOrDeleted = ( true == fForSet && true == m_bSetNotAvailableOrDeleted ) ||
							  ( false == pclActuator->IsAvailable() || true == pclActuator->IsDeleted( ) );
	if( false == m_mapChangeStatus[m_iCurrentSourceSelected] || pList == &m_ListCVSrc )
	{
		m_cTextForeColor =  ( false == fNotAvailOrDeleted ) ? _BLACK : _RED;
		m_cTextBackColor = _WHITE;
	}
	else
	{
		m_cTextForeColor = ( false == fNotAvailOrDeleted ) ? GetSysColor( COLOR_GRAYTEXT ) : _LIGHTRED;
		m_cTextBackColor = GetSysColor( COLOR_3DFACE );
	}

	// Insert reference.
	if( true == fForSet )
	{
		// No article or local article number for set.
		pList->SetItemTextBold( m_iNbrRow + m_iLeftPos, DLGSRACTR_LEFTCOLUMNINDEX, true );
		pList->AddItemTextColorVerify( _T("-"), m_iNbrRow + m_iLeftPos++, DLGSRACTR_LEFTCOLUMNINDEX, m_cTextForeColor, m_cTextForeColor, m_cTextBackColor );
	}
	else
	{
		// Check first if we have a local article number available.
		CString strLocalArticleNumber = _T("");
		if( true == TASApp.IsLocalArtNumberUsed() )
		{
			strLocalArticleNumber = LocArtNumTab.GetLocalArticleNumber( pclActuator->GetArtNum() );
			strLocalArticleNumber.Trim();
		}

		// Fill article number.
		bool fArticleNumberShown = false;
		// If we can show article number OR we can't show but there is no local article number...
		if( false == TASApp.IsTAArtNumberHidden() || ( true == TASApp.IsTAArtNumberHidden() && true == strLocalArticleNumber.IsEmpty() )   )
		{
			CString str = pclActuator->GetArtNum();
			if( false == str.IsEmpty() )
			{
				pList->SetItemTextBold( m_iNbrRow + m_iLeftPos, DLGSRACTR_LEFTCOLUMNINDEX, true );
				pList->AddItemTextColorVerify( str, m_iNbrRow + m_iLeftPos++, DLGSRACTR_LEFTCOLUMNINDEX, m_cTextForeColor, m_cTextForeColor, m_cTextBackColor );
				fArticleNumberShown = true;
			}
		}

		// Set local article number if allowed and exist.
		if( true == TASApp.IsLocalArtNumberUsed() && false == strLocalArticleNumber.IsEmpty() )
		{
			strLocalArticleNumber = (CString)TASApp.GetLocalArtNumberName() + LocArtNumTab.GetLocalArticleNumber( pclActuator->GetArtNum() );
			strLocalArticleNumber.Trim();
			pList->SetItemTextBold( m_iNbrRow + m_iLeftPos, DLGSRACTR_LEFTCOLUMNINDEX, ( true == fArticleNumberShown ) ? false : true );
			pList->AddItemTextColorVerify( strLocalArticleNumber, m_iNbrRow + m_iLeftPos++, DLGSRACTR_LEFTCOLUMNINDEX, m_cTextForeColor, m_cTextForeColor, m_cTextBackColor );
		}
	}

	// If something has been printed in left column and product is not available or deleted...
	if( true == fNotAvailOrDeleted && false == fForSet )
	{
		// Remarks: if product is flagged as 'not available' and 'deleted', we show only 'deleted'!
		CString strFlag = TASApp.LoadLocalizedString( ( true == pclActuator->IsDeleted() ) ? IDS_TAPDELETED : IDS_TAPNOTAVAILABLE );
		pList->SetItemTextBold( m_iNbrRow + m_iLeftPos, DLGSRACTR_LEFTCOLUMNINDEX, true );
		pList->AddItemTextColorVerify( strFlag, m_iNbrRow + m_iLeftPos++, DLGSRACTR_LEFTCOLUMNINDEX, m_cTextForeColor, m_cTextForeColor, m_cTextBackColor );
	}
	
	////////////////////////////////////////////////////////////////////////////////////////////////////////////////
	// Fill the right part.
	////////////////////////////////////////////////////////////////////////////////////////////////////////////////
	
	// Insert name.
	str = pclActuator->GetName();
	pList->SetItemTextBold( m_iNbrRow + m_iRightPos, DLGSRACTR_RIGHTCOLUMNINDEX, false );
	pList->AddItemTextColorVerify( str, m_iNbrRow + m_iRightPos++, DLGSRACTR_RIGHTCOLUMNINDEX, m_cTextForeColor, m_cTextForeColor, m_cTextBackColor );
	
	CString strDescription = pclActuator->GetComment();

	if( false == strDescription.IsEmpty() )
	{
		pList->AddItemTextColorVerify( pclActuator->GetComment(), m_iNbrRow + m_iRightPos++, DLGSRACTR_RIGHTCOLUMNINDEX, m_cTextForeColor, m_cTextForeColor, m_cTextBackColor );
	}

	// Set the close off value if exist.
	CDB_CloseOffChar *pCloseOffChar = (CDB_CloseOffChar *)( pclControlValve->GetCloseOffCharIDPtr().MP );

	if( NULL != pCloseOffChar )
	{
		if( CDB_CloseOffChar::CloseOffDp == pCloseOffChar->GetLimitType() )
		{
			str = TASApp.LoadLocalizedString( IDS_DLGSRACTUATOR_ACT_CLOSEOFFDP );
			double dCloseOffDp = pCloseOffChar->GetCloseOffDp( pclActuator->GetMaxForceTorque() );

			if( dCloseOffDp > 0.0 )
			{
				str += CString( _T(" : ") ) + WriteCUDouble( _U_DIFFPRESS, dCloseOffDp, true );
			}
		}
		else if( CDB_CloseOffChar::InletPressure == pCloseOffChar->GetLimitType() )
		{
			str = TASApp.LoadLocalizedString( IDS_DLGSRACTUATOR_ACT_MAXINLETPRESSURE );
			double dMaxInletPressure = pCloseOffChar->GetMaxInletPressure( pclActuator->GetMaxForceTorque() );

			if( dMaxInletPressure > 0.0 )
			{
				str += CString( _T(" : ") ) + WriteCUDouble( _U_PRESSURE, dMaxInletPressure, true );
			}
		}

		pList->AddItemTextColorVerify( str, m_iNbrRow + m_iRightPos++, DLGSRACTR_RIGHTCOLUMNINDEX, m_cTextForeColor, m_cTextForeColor, m_cTextBackColor );
	}

	CDB_ElectroActuator *pclElectroActuator = dynamic_cast<CDB_ElectroActuator*>( pclActuator );
	
	if( NULL != pclElectroActuator )
	{
		// Compute actuating time in sec.
		str = TASApp.LoadLocalizedString( IDS_DLGSRACTUATOR_ACTUATING_TIME ) + CString( _T(" : ") ) + pclElectroActuator->GetActuatingTimesStr( pclControlValve->GetStroke(), true );
		pList->AddItemTextColorVerify( str, m_iNbrRow + m_iRightPos++, DLGSRACTR_RIGHTCOLUMNINDEX, m_cTextForeColor, m_cTextForeColor, m_cTextBackColor );

		str = TASApp.LoadLocalizedString( IDS_DLGSRACTUATOR_ACT_IPXXAUTO );
		str += CString( _T(" : ") ) + pclElectroActuator->GetIPxxAuto();
		if( false == pclElectroActuator->GetIPxxManual().IsEmpty() )
			str += _T("(") + pclElectroActuator->GetIPxxManual() + _T(")");
		pList->AddItemTextColorVerify( str, m_iNbrRow + m_iRightPos++, DLGSRACTR_RIGHTCOLUMNINDEX, m_cTextForeColor, m_cTextForeColor, m_cTextBackColor );

		COLORREF cTextForeColor = m_cTextForeColor;
		str = TASApp.LoadLocalizedString( IDS_DLGSRACTUATOR_ACT_POWSUP );
		str += CString( _T(" : ") ) + pclElectroActuator->GetPowerSupplyStr();
		pList->AddItemTextColorVerify( str, m_iNbrRow + m_iRightPos++, DLGSRACTR_RIGHTCOLUMNINDEX, cTextForeColor, cTextForeColor, m_cTextBackColor );

		cTextForeColor = m_cTextForeColor;
		str = TASApp.LoadLocalizedString( IDS_DLGSRACTUATOR_ACT_INPUTSIG );
		str += CString( _T(" : ")) + pclElectroActuator->GetInOutSignalsStr( true );
		if( true == m_bDowngradeFunctionalities && true == m_mapDowngradeStatus[pclElectroActuator] && _NULL_IDPTR != *m_pInputSignalIDPtr )
		{
			// Check if current one is different for the user choice.
			if( false == pclElectroActuator->IsInputSignalAvailable( *m_pInputSignalIDPtr ) )
			{
				pList->SetItemTextBold( m_iNbrRow + m_iRightPos, DLGSRACTR_RIGHTCOLUMNINDEX, true );
				cTextForeColor = ( false == m_mapChangeStatus[m_iCurrentSourceSelected] ) ? _RED : _LIGHTRED;
			}
		}
		pList->AddItemTextColorVerify( str, m_iNbrRow + m_iRightPos++, DLGSRACTR_RIGHTCOLUMNINDEX, cTextForeColor, cTextForeColor, m_cTextBackColor );

		str = TASApp.LoadLocalizedString( IDS_DLGSRACTUATOR_ACT_OUTPUTSIG );
		if( CString( _T("") ) == pclElectroActuator->GetInOutSignalsStr( false ) )
			str += CString( _T(" :    -") );
		else
			str += CString( _T(" : ") ) + pclElectroActuator->GetInOutSignalsStr( false );
		pList->AddItemTextColorVerify( str, m_iNbrRow + m_iRightPos++, DLGSRACTR_RIGHTCOLUMNINDEX, m_cTextForeColor, m_cTextForeColor, m_cTextBackColor );

		cTextForeColor = m_cTextForeColor;
		str = TASApp.LoadLocalizedString( IDS_DLGSRACTUATOR_ACT_FAILSAFE );

		if( pclElectroActuator->GetFailSafe() == CDB_ElectroActuator::FailSafeType::eFSTypeElectronic )
		{
			str += CString( _T( " : " ) ) + TASApp.LoadLocalizedString( IDS_STRELECFAILSAFE );
		}
		else if( pclElectroActuator->GetFailSafe() == CDB_ElectroActuator::FailSafeType::eFSTypeSpringReturn )
		{
			str += CString( _T( " : " ) ) + TASApp.LoadLocalizedString( IDS_STRSPRINGFAILSAFE );
		}
		else
		{
			str += CString( _T( " : " ) ) + TASApp.LoadLocalizedString( IDS_NO );
		}

		pList->AddItemTextColorVerify( str, m_iNbrRow + m_iRightPos++, DLGSRACTR_RIGHTCOLUMNINDEX, cTextForeColor, cTextForeColor, m_cTextBackColor );

		cTextForeColor = m_cTextForeColor;

		if( pclElectroActuator->GetFailSafe() >= CDB_ElectroActuator::FailSafeType::eFSTypeElectronic
			&& pclElectroActuator->GetFailSafe() < CDB_ElectroActuator::FailSafeType::eFSTypeLast )
		{
			str = TASApp.LoadLocalizedString( IDS_DLGSRACTUATOR_ACT_FSP ) + _T( " " );
		}
		else
		{
			str = TASApp.LoadLocalizedString( IDS_DLGSRACTUATOR_ACT_DRP ) + _T( " " );
		}
		
		if( CString( _T("") ) == pclElectroActuator->GetDefaultReturnPosStr( pclElectroActuator->GetDefaultReturnPos() ).c_str() )
			str += CString( _T("   -") );
		else
			str += pclElectroActuator->GetDefaultReturnPosStr( pclElectroActuator->GetDefaultReturnPos() ).c_str();
		pList->AddItemTextColorVerify( str, m_iNbrRow + m_iRightPos++, DLGSRACTR_RIGHTCOLUMNINDEX, cTextForeColor, cTextForeColor, m_cTextBackColor );
	}
	
	CDB_ThermostaticActuator *pclThermostaticActuator = dynamic_cast<CDB_ThermostaticActuator*>( pclActuator );
	if( NULL != pclThermostaticActuator )
	{
		// HYS-951: Min setting can be 0.
		if( pclThermostaticActuator->GetMinSetting() >= 0 && pclThermostaticActuator->GetMaxSetting() > 0 )
		{
			CString str1 = TASApp.LoadLocalizedString( IDS_DLGSRACTUATOR_ACT_SETTINGRANGE );
			CString strMin = WriteCUDouble( _U_TEMPERATURE, pclThermostaticActuator->GetMinSetting(), false );
			CString strMax =  WriteCUDouble( _U_TEMPERATURE, pclThermostaticActuator->GetMaxSetting(), true );
			str.Format( _T("%s: [%s-%s]"), str1, strMin, strMax );
			pList->AddItemTextColorVerify( str, m_iNbrRow + m_iRightPos++, DLGSRACTR_RIGHTCOLUMNINDEX, m_cTextForeColor, m_cTextForeColor, m_cTextBackColor );
		}
		
		if( pclThermostaticActuator->GetCapillaryLength() > 0 )
		{
			str = TASApp.LoadLocalizedString( IDS_DLGSRACTUATOR_ACT_CAPILLENGTH );
			str += CString( _T(": ") ) + WriteCUDouble( _U_LENGTH, pclThermostaticActuator->GetCapillaryLength(), true );
			pList->AddItemTextColorVerify( str, m_iNbrRow + m_iRightPos++, DLGSRACTR_RIGHTCOLUMNINDEX, m_cTextForeColor, m_cTextForeColor, m_cTextBackColor );
		}
		
		if( pclThermostaticActuator->GetHysteresis() > 0 )
		{
			str = TASApp.LoadLocalizedString( IDS_DLGSRACTUATOR_ACT_HYSTERESIS );
			str += CString( _T(": ") ) + WriteCUDouble( _U_DIFFTEMP, pclThermostaticActuator->GetHysteresis(), true );
			pList->AddItemTextColorVerify( str, m_iNbrRow + m_iRightPos++, DLGSRACTR_RIGHTCOLUMNINDEX, m_cTextForeColor, m_cTextForeColor, m_cTextBackColor );
		}

		if( pclThermostaticActuator->GetFrostProtection() > 0 )
		{
			str = TASApp.LoadLocalizedString( IDS_DLGSRACTUATOR_ACT_FROSTPROTECT );
			str += CString( _T(": ") ) + WriteCUDouble( _U_TEMPERATURE, pclThermostaticActuator->GetFrostProtection(), true );
			pList->AddItemTextColorVerify( str, m_iNbrRow + m_iRightPos++, DLGSRACTR_RIGHTCOLUMNINDEX, m_cTextForeColor, m_cTextForeColor, m_cTextBackColor );
		}
	}

	// Insert a final blank line.
	DLGSRACTR_AJUSTCOORDS( m_iLeftPos, m_iRightPos );
	pList->InsertItem( LVIF_TEXT | LVIF_STATE, m_iNbrRow + m_iLeftPos, _T(""), 0, LVIS_SELECTED, 0, 0 );
}

void CDlgSearchAndReplaceActuator::_FillCVActrAccSetPart( CDB_Set* pCVActSet, CExtListCtrl* pList )
{
	if( NULL == pCVActSet || _NULL_IDPTR == pCVActSet->GetAccGroupIDPtr() )
		return;

	std::vector<CData *> vecData;
	CDB_RuledTable* pclRuledTable = (CDB_RuledTable*)( pCVActSet->GetAccGroupIDPtr().MP );
	if( NULL == pclRuledTable )
		return;

	int iCount = pclRuledTable->GetBuiltIn( &vecData );
	if( iCount > 0 )
	{
		m_iNbrRow = pList->GetItemCount();
		m_iLeftPos = 0;
		m_iRightPos = 0;

		// Insert a blank line.
		DLGSRACTR_AJUSTCOORDS( m_iLeftPos, m_iRightPos );
		pList->InsertItem( LVIF_TEXT | LVIF_STATE, m_iNbrRow + m_iLeftPos++, _T(""), 0, LVIS_SELECTED, 0, 0 );
		DLGSRACTR_AJUSTCOORDS( m_iLeftPos, m_iRightPos );

		for( int i = 0; i < iCount; i++ )
		{
			bool fByPair = pclRuledTable->IsByPair( vecData[i]->GetIDPtr().ID );
			_FillAccessoryPart( (CDB_Product*)vecData[i], true, fByPair, pList );
			m_vecCVSetAccessoryList.push_back( (CDB_Product*)vecData[i] );
		}
	}
}

void CDlgSearchAndReplaceActuator::_FillQuantity( int iQuantity, CExtListCtrl* pList )
{
	// Variables.
	m_iNbrRow = pList->GetItemCount();
	m_iLeftPos = 0;
	m_iRightPos = 0;

	if( false == m_mapChangeStatus[m_iCurrentSourceSelected] || pList == &m_ListCVSrc )
	{
		m_cTextForeColor = _BLACK;
		m_cTextBackColor = _WHITE;
	}
	else
	{
		m_cTextForeColor = GetSysColor( COLOR_GRAYTEXT );
		m_cTextBackColor = GetSysColor( COLOR_3DFACE );
	}

	// Fill the left part.
	CString str = TASApp.LoadLocalizedString( IDS_DLGSRACTUATOR_QUANTITY );
	pList->SetItemTextBold( m_iNbrRow + m_iLeftPos, DLGSRACTR_LEFTCOLUMNINDEX, true );
	pList->AddItemTextColorVerify( str, m_iNbrRow + m_iLeftPos++, DLGSRACTR_LEFTCOLUMNINDEX, m_cTextForeColor, m_cTextForeColor, m_cTextBackColor );

	// Fill the right part.
	str.Format( _T("%i"), iQuantity );
	pList->AddItemTextColorVerify( str, m_iNbrRow + m_iRightPos++, DLGSRACTR_RIGHTCOLUMNINDEX, m_cTextForeColor, m_cTextForeColor, m_cTextBackColor );

	// Insert a final blank line.
	DLGSRACTR_AJUSTCOORDS( m_iLeftPos, m_iRightPos );
	pList->InsertItem( LVIF_TEXT | LVIF_STATE, m_iNbrRow + m_iLeftPos, _T(""), 0, LVIS_SELECTED, 0, 0 );
}

bool CDlgSearchAndReplaceActuator::_GetFirstSelection( void )
{
	bool fReturn;
	m_iCurrentSourceSelected = 0;
	switch( m_eWorkingOn )
	{
		case WorkingOn::WO_HM:
			fReturn = ( m_iCurrentSourceSelected != m_pvecHMList->size() );
			break;

		case WorkingOn::WO_IndSel:
			m_iCurrentSourceSelected = 0;
			fReturn = ( m_iCurrentSourceSelected != m_pvecSelCtrlList->size() );
			break;
	}
	return fReturn;
}

bool CDlgSearchAndReplaceActuator::_GetPreviousSelection( void )
{
	bool fReturn = false;
	if( m_iCurrentSourceSelected > 0 )
	{
		m_iCurrentSourceSelected--;
		fReturn = true;
	}
	return fReturn;
}

bool CDlgSearchAndReplaceActuator::_IsPreviousSelectionExist( void )
{
	return ( m_iCurrentSourceSelected > 0 );
}

bool CDlgSearchAndReplaceActuator::_GetNextSelection( void )
{
	bool fReturn = true;
	switch( m_eWorkingOn )
	{
		case WorkingOn::WO_HM:
			if( m_iCurrentSourceSelected == (int)m_pvecHMList->size() - 1 )
				fReturn = false;
			else
				m_iCurrentSourceSelected++;
			break;

		case WorkingOn::WO_IndSel:
			if( m_iCurrentSourceSelected == (int)m_pvecSelCtrlList->size() - 1 )
				fReturn = false;
			else
				m_iCurrentSourceSelected++;
			break;
	}
	return fReturn;
}

bool CDlgSearchAndReplaceActuator::_IsNextSelectionExist( void )
{
	bool fReturn;
	switch( m_eWorkingOn )
	{
		case WorkingOn::WO_HM:
			fReturn = ( m_iCurrentSourceSelected + 1 < (int)m_pvecHMList->size() );
			break;

		case WorkingOn::WO_IndSel:
			fReturn = ( m_iCurrentSourceSelected + 1 < (int)m_pvecSelCtrlList->size() );
			break;
	}
	return fReturn;
}

void CDlgSearchAndReplaceActuator::_ShowSolution( void )
{
	m_ListCVSrc.DeleteAllItems();
	m_ListCVTrg.DeleteAllItems();

	// Fill the left part.
	_FillSrcList();

	CDB_ControlValve* pclControlValve = NULL;
	bool fIsSelectedAsPackage = false;
	if( WorkingOn::WO_HM == m_eWorkingOn )
	{
		CDS_HydroMod* pHM = m_pvecHMList->at( m_iCurrentSourceSelected );
		fIsSelectedAsPackage = ( eBool3::eb3True == pHM->GetpCV()->GetActrSelectedAsaPackage() ) ? true : false;
		pclControlValve = pHM->GetpCV()->GetpCV();
	}
	else
	{
		CDS_SSelCtrl* pclSSelCtrl = m_pvecSelCtrlList->at( m_iCurrentSourceSelected );
		fIsSelectedAsPackage = pclSSelCtrl->IsSelectedAsAPackage();
		pclControlValve = (CDB_ControlValve*)( pclSSelCtrl->GetProductIDPtr().MP );
	}
	
	// If user has already done his choice on this control valve...
	if( true == m_mapChangeStatus[m_iCurrentSourceSelected] )
	{
		_FillDstList();
	}
	else if( ( false == fIsSelectedAsPackage && false == pclControlValve->IsAvailable() ) ||
			 ( true == fIsSelectedAsPackage && true == m_bSetNotAvailableOrDeleted ) )
	{
		// Do nothing on a product not available or deleted.
		_FillDstListNotAvailableOrDeleted();
	}
	else if( WorkingMode::WM_Remove == m_eWorkingMode && true == fIsSelectedAsPackage && 
			 ePartOfaSet::ePartOfaSetYes == pclControlValve->GetPartOfaSet() )
	{
		// We can't remove actuator on a set that is not breakable!
		_FillDstListSetNoBreakable();
	}
	else if( true == _FindSolution() )
	{
		_GetFirstSolution();
		_FillDstList();
	}
	else
	{
		_FillDstListNoSolution();
	}
}

bool CDlgSearchAndReplaceActuator::_GetFirstSolution( void )
{
	bool fReturn = false;
	m_iCurrentSolution = -1;
	if( m_vecActuatorSolution.size() > 0 || m_vecCVActrSetSolution.size() > 0 )
	{
		m_iCurrentSolution = 0;
		_GetAdapter();
		fReturn = true;
	}
	return fReturn;
}

bool CDlgSearchAndReplaceActuator::_GetNextSolution( void )
{
	bool fReturn = false;
	m_iCurrentSolution++;
	if( ( false == m_bSetForced && m_iCurrentSolution < (int)m_vecActuatorSolution.size() ) ||
		( true == m_bSetForced && m_iCurrentSolution < (int)m_vecCVActrSetSolution.size() ) )
	{
		_GetAdapter();
		fReturn = true;
	}
	return fReturn;
}

bool CDlgSearchAndReplaceActuator::_IsNextSolutionExist( void )
{
	bool fReturn = false;
	if( ( false == m_bSetForced && m_iCurrentSolution + 1 < (int)m_vecActuatorSolution.size() ) ||
		( true == m_bSetForced && m_iCurrentSolution + 1 < (int)m_vecCVActrSetSolution.size() ) )
		fReturn = true;
	return fReturn;
}

void CDlgSearchAndReplaceActuator::_GetSolution( int iIndex )
{
	if( iIndex < 0 )
		return;

	if( false == m_bSetForced && iIndex > (int)m_vecActuatorSolution.size() - 1 )
		return;

	if( true == m_bSetForced && iIndex > (int)m_vecCVActrSetSolution.size() - 1 )
		return;

	m_iCurrentSolution = iIndex;
	_GetAdapter();
}

int CDlgSearchAndReplaceActuator::_GetNumberOfSolution( void )
{
	int iNumber;
	if( false == m_bSetForced )
		iNumber = (int)m_vecActuatorSolution.size();
	else
		iNumber = m_vecCVActrSetSolution.size();
	return iNumber;
}

void CDlgSearchAndReplaceActuator::_GetAdapter( void )
{
	m_pclAdapter = NULL;
	m_bAdapterBelongsToSet = false;

	// Check first if adapter is not already present in the source.
	CDS_HydroMod* pHM = NULL;
	CDS_SSelCtrl* pclSelCtrl = NULL;
	switch( m_eWorkingOn )
	{
		case WorkingOn::WO_HM:
			{
				pHM = m_vecHMListCopy[m_iCurrentSourceSelected];
				if( NULL == pHM )
					return;
				
				if( NULL != pHM->GetpCV() && _NULL_IDPTR != pHM->GetpCV()->GetActrAdaptIDPtr() )
				{
					m_pclAdapter = (CDB_Product*)( pHM->GetpCV()->GetActrAdaptIDPtr().MP );
					return;
				}
			}
			break;

		case WorkingOn::WO_IndSel:
			{
				pclSelCtrl = m_vecSelCtrlListCopy[m_iCurrentSourceSelected];
				if( NULL == pclSelCtrl )
					return;

				if( NULL != pclSelCtrl->GetCvAccessoryList() && pclSelCtrl->GetCvAccessoryList()->GetCount( CAccessoryList::AccessoryType::_AT_Adapter ) > 0 )
				{
					CAccessoryList::AccessoryItem rAccessoryItem = pclSelCtrl->GetCvAccessoryList()->GetFirst( CAccessoryList::AccessoryType::_AT_Adapter );
					if( _NULL_IDPTR == rAccessoryItem.IDPtr )
						return;
					m_pclAdapter = (CDB_Product*)( rAccessoryItem.IDPtr.MP );
					return;
				}
			}
			break;
	}
	
	// m_fSetForced | What to do?
	// --------------------------------------------------------
	//         true | Add adapter only if belonging to the set.
	//        false | Add adapter.

	if( m_vecActuatorSolution.size() > 0 || m_vecCVActrSetSolution.size() > 0 )
	{
		CDB_ControlValve* pclControlValve = NULL;
		CDB_Actuator* pclActuator = NULL;
		switch( m_eWorkingOn )
		{
			case WorkingOn::WO_HM:
				{
					pclControlValve = pHM->GetpCV()->GetpCV();
					if( NULL == pclControlValve )
						return;
				}
				break;

			case WorkingOn::WO_IndSel:
				{
					pclControlValve = (CDB_ControlValve*)( pclSelCtrl->GetProductIDPtr().MP );
					if( NULL == pclControlValve )
						return;
				}
				break;
		}

		if( m_vecActuatorSolution.size() > 0 && m_iCurrentSolution >= 0 && m_iCurrentSolution < (int)m_vecActuatorSolution.size() )
		{
			pclActuator = m_vecActuatorSolution[m_iCurrentSolution];
		}
		else if( m_vecCVActrSetSolution.size() > 0 && m_iCurrentSolution >= 0 && m_iCurrentSolution < (int)m_vecCVActrSetSolution.size() )
		{
			CDB_Set* pclSet = m_vecCVActrSetSolution[m_iCurrentSolution];
			pclActuator = (CDB_Actuator*)( pclSet->GetSecondIDPtr().MP );
		}
		if( NULL == pclActuator )
			return;

		CRank clAdapterList;
		CDB_RuledTableBase* pclRuledTable = NULL;
		CDB_ControlProperties* pclControlProperties = pclControlValve->GetCtrlProp();
		CDB_ControlProperties::CvCtrlType CtrlType = CDB_ControlProperties::eCvNU;
		if( NULL != pclControlProperties )
			CtrlType = pclControlProperties->GetCtrlType();

		CTADatabase::FilterSelection eFilterSelection = ( WorkingOn::WO_HM == m_eWorkingOn ) ? CTADatabase::FilterSelection::ForHMCalc : CTADatabase::FilterSelection::ForIndAndBatchSel;
		bool fDowngradeFct = ( m_mapDowngradeStatus.count( pclActuator ) > 0 ) ? m_mapDowngradeStatus[pclActuator] : false;

		int iFailSafeFct = -1;
		if( true == m_bFailSafeFct )
		{
			iFailSafeFct = 1;
		}
		else if( false == m_bFailSafeFct )
		{
			iFailSafeFct = 0;
		}
		TASApp.GetpTADB()->GetAdapterList( pclControlValve, &clAdapterList, &pclRuledTable, fDowngradeFct, m_pPowerSupplyIDPtr->ID,
											m_pInputSignalIDPtr->ID, iFailSafeFct, m_eDefaultReturnPosFct, CtrlType, m_bSetForced, eFilterSelection );
		
		// If not found for set ...
		if( 0 == clAdapterList.GetCount() && true == m_bSetForced )
		{
			// We can try to find adapter that not belongs to the set.
			TASApp.GetpTADB()->GetAdapterList( pclControlValve, &clAdapterList, &pclRuledTable, fDowngradeFct, m_pPowerSupplyIDPtr->ID,
											m_pInputSignalIDPtr->ID, iFailSafeFct, m_eDefaultReturnPosFct, CtrlType, false, eFilterSelection );
		}

		if( 1 == clAdapterList.GetCount() )
		{
			CString str;
			LPARAM lparam;
			clAdapterList.GetFirst( str, lparam );
			m_pclAdapter = (CDB_Product *)lparam;
		}
	}
}

bool CDlgSearchAndReplaceActuator::_FindSolution( bool fUseParams, IDPTR PowerSupplyIDPtr, IDPTR InputSignalIDPtr, 
												CDB_ControlValve::DRPFunction eFailSafeFunctionToCheck, bool fDowngradeFunctionalities )
{
	if( true == m_mapChangeStatus[m_iCurrentSourceSelected] )
		return true;

	// Prepare variables.
	CDB_ControlValve* pclControlValve = NULL;
	if( WorkingMode::WM_Add == m_eWorkingMode )
	{
		switch( m_eWorkingOn )
		{
			case WorkingOn::WO_HM:
				{
					CDS_HydroMod* pHM = m_vecHMListCopy[m_iCurrentSourceSelected];
					if( NULL == pHM )
						return false;

					pclControlValve = pHM->GetpCV()->GetpCV();
					if( NULL == pclControlValve )
						return false;
				}
				break;

			case WorkingOn::WO_IndSel:
				{
					CDS_SSelCtrl* pclSelCtrl = m_vecSelCtrlListCopy[m_iCurrentSourceSelected];
					if( NULL == pclSelCtrl )
						return false;

					pclControlValve = (CDB_ControlValve*)( pclSelCtrl->GetProductIDPtr().MP );
					if( NULL == pclControlValve )
						return false;
				}
				break;
		}
	}
	
	m_vecActuatorSolution.clear();
	m_vecCVActrSetSolution.clear();
	m_mapDowngradeStatus.clear();
	m_pclAdapter = NULL;
	m_bSetForced = false;
	m_ScrollBar.EnableWindow( FALSE );
	bool fReturn = false;
	switch( m_eWorkingMode )
	{
		case WorkingMode::WM_Remove:
			fReturn = true;
			break;

		case WorkingMode::WM_Add:
			{
				CDB_ElectroActuator* pclElectroActuator = NULL;
				CRank rList( false );
				if( true == m_bForceSetWhenPossible )
				{
					// User wants to force set when available. We have to list only actuators that belong to a package.
					std::set<CDB_Set *> CVActSetArray;
					CTableSet* pBCVActTableSet = pclControlValve->GetTableSet();				ASSERT( NULL != pBCVActTableSet );
					int iCDBSetCount = pBCVActTableSet->FindCompatibleSet( &CVActSetArray, pclControlValve->GetIDPtr().ID, _T("") );
					if( iCDBSetCount > 0 )
					{
						int iLoop;
						if( true == fUseParams )
							iLoop = ( true == fDowngradeFunctionalities ) ? 2 : 1;
						else
							iLoop = ( true == m_bDowngradeFunctionalities ) ? 2 : 1;
						
						int iPass = 0;
						do
						{
							// Do a first pass to select actuators that has the same input signal type as the control type that user wants.
							// (On/Off with On/Off, 3 points with 3 points and proportional with proportional).
							// If not found, we accept functionality of '3 points' and 'Proportional' actuators to work in the 'On/Off' and '3 points' modes.
							bool fDowngradeActuatorFunctionality = ( 0 == iPass++ ) ? false : true;

							for( std::set<CDB_Set *>::iterator iterCDBSet = CVActSetArray.begin(); iterCDBSet != CVActSetArray.end(); iterCDBSet++ )
							{
								CDB_Set* pCVActSet = *iterCDBSet;
								IDPTR ActuatorIDPtr = pCVActSet->GetSecondIDPtr();
								if( *ActuatorIDPtr.ID != '\0')
								{
									pclElectroActuator = dynamic_cast<CDB_ElectroActuator *>( ActuatorIDPtr.MP );
									if( NULL == pclElectroActuator || false == pclElectroActuator->IsSelectable( true ) )
										continue;

									CDB_ControlValve::DRPFunction DRPFct = pclControlValve->GetCompatibleDRPFunction( (int)pclElectroActuator->GetDefaultReturnPos() );
									
									CDB_ControlProperties::CvCtrlType eCtrlType = CDB_ControlProperties::CvCtrlType::eCvNU;
									CDB_ControlProperties* pclControlProperties = pclControlValve->GetCtrlProp();
									if( NULL != pclControlProperties )
										eCtrlType = pclControlProperties->GetCtrlType();

									bool fCompatible;
									int iFailSafe = -1;
									if( true == m_bFailSafeFct )
									{
										iFailSafe = 1;
									}
									else if( false == m_bFailSafeFct )
									{
										iFailSafe = 0;
									}
									if( true == fUseParams )
										fCompatible = pclElectroActuator->IsActuatorFit( PowerSupplyIDPtr.ID, InputSignalIDPtr.ID, iFailSafe, eFailSafeFunctionToCheck, DRPFct, eCtrlType, fDowngradeActuatorFunctionality );
									else
										fCompatible = pclElectroActuator->IsActuatorFit( m_pPowerSupplyIDPtr->ID, m_pInputSignalIDPtr->ID, iFailSafe, m_eDefaultReturnPosFct, DRPFct, eCtrlType, fDowngradeActuatorFunctionality );
			
									if( true == fCompatible )
									{
										m_vecCVActrSetSolution.push_back( pCVActSet );
										m_mapDowngradeStatus[pclElectroActuator] = fDowngradeActuatorFunctionality;
										fReturn = true;
										m_bSetForced = true;
									}
								}
							}
						}
						while( 0 == (int)m_vecCVActrSetSolution.size() && iPass < iLoop );
					}
				}

				// If set not found or user doesn't want force selection by set...
				if( false == m_bSetForced )
				{
					// Retrieve the actuator group on the selected control valve.
					CTable *pclActuatorGroup = (CTable *)( pclControlValve->GetActuatorGroupIDPtr().MP );
					if( NULL == pclActuatorGroup )
						break;

					// Retrieve list of all actuators in this group.
					CTADatabase::FilterSelection eFilterSelection = ( WorkingOn::WO_HM == m_eWorkingOn ) ? CTADatabase::FilterSelection::ForHMCalc : CTADatabase::FilterSelection::ForIndAndBatchSel;
					TASApp.GetpTADB()->GetActuator( &rList, pclActuatorGroup, L"", L"", ( true == m_bFailSafeFct)?1:0, CDB_ElectroActuator::DefaultReturnPosition::Undefined, eFilterSelection );

					int iLoop;
					if( true == fUseParams )
						iLoop = ( true == fDowngradeFunctionalities ) ? 2 : 1;
					else
						iLoop = ( true == m_bDowngradeFunctionalities ) ? 2 : 1;

					CString str;
					LPARAM lparam;
					int iPass = 0;
					do
					{
						// Do a first pass to select actuators that has the same input signal type as the control type that user wants.
						// (On/Off with On/Off, 3 points with 3 points and proportional with proportional).
						// If not found, we accept functionality of '3 points' and 'Proportional' actuators to work in the 'On/Off' and '3 points' modes.
						bool fDowngradeActuatorFunctionality = ( 0 == iPass++ ) ? false : true;

						for( BOOL fContinue = rList.GetFirst( str, lparam ); TRUE == fContinue; fContinue = rList.GetNext( str, lparam ) )
						{
							pclElectroActuator = dynamic_cast<CDB_ElectroActuator *>( (CData *)lparam );
							if( NULL == pclElectroActuator || false == pclElectroActuator->IsSelectable( true ) )
								continue;

							CDB_ControlValve::DRPFunction DRPFct = pclControlValve->GetCompatibleDRPFunction( (int)pclElectroActuator->GetDefaultReturnPos() );

							CDB_ControlProperties::CvCtrlType eCtrlType = CDB_ControlProperties::CvCtrlType::eCvNU;
							CDB_ControlProperties* pclControlProperties = pclControlValve->GetCtrlProp();
							if( NULL != pclControlProperties )
								eCtrlType = pclControlProperties->GetCtrlType();

							bool fCompatible;
							int iFailSafe = -1;
							if( true == m_bFailSafeFct )
							{
								iFailSafe = 1;
							}
							else if( false == m_bFailSafeFct )
							{
								iFailSafe = 0;
							}
							if( true == fUseParams )
								fCompatible = pclElectroActuator->IsActuatorFit( PowerSupplyIDPtr.ID, InputSignalIDPtr.ID, iFailSafe, eFailSafeFunctionToCheck, DRPFct, eCtrlType, fDowngradeActuatorFunctionality );
							else
								fCompatible = pclElectroActuator->IsActuatorFit( m_pPowerSupplyIDPtr->ID, m_pInputSignalIDPtr->ID, iFailSafe, m_eDefaultReturnPosFct, DRPFct, eCtrlType, fDowngradeActuatorFunctionality );
							if( true == fCompatible )
							{
								m_vecActuatorSolution.push_back( pclElectroActuator );
								m_mapDowngradeStatus[pclElectroActuator] = fDowngradeActuatorFunctionality;
								fReturn = true;
							}
						}
					}
					while( 0 == (int)m_vecActuatorSolution.size() && iPass < iLoop );
				}

				if( m_vecActuatorSolution.size() > 1 || m_vecCVActrSetSolution.size() > 1 )
				{
					m_ScrollBar.EnableWindow( TRUE );
					SCROLLINFO rScrollInfo;
					rScrollInfo.cbSize = sizeof( SCROLLINFO );
					rScrollInfo.fMask = SIF_ALL;
					rScrollInfo.nMin = 0;
					rScrollInfo.nMax = ( false == m_bSetForced ) ? m_vecActuatorSolution.size() - 1 : m_vecCVActrSetSolution.size() - 1;
					rScrollInfo.nPos = 0;
					rScrollInfo.nPage = 1;
					m_ScrollBar.SetScrollInfo( &rScrollInfo );
				}
			}
			break;
	}
	return fReturn;
}

void CDlgSearchAndReplaceActuator::_ApplyOnHM( void )
{
	if( WS_DISABLED != ( m_ButApply.GetStyle() & WS_DISABLED ) )
	{
		CString strControlValve;
		CString strChangeDone;
		CDS_HydroMod *pHM = m_pvecHMList->at( m_iCurrentSourceSelected );

		if( NULL == pHM || NULL == pHM->GetpCV() )
		{
			return;
		}

		CDS_HydroMod::CCv *pHMCv = pHM->GetpCV();

		switch( m_eWorkingMode )
		{
			case WorkingMode::WM_Remove:
				{
					// Control valve.
					CDB_ControlValve *pclControlValve = pHMCv->GetpCV();
					CString strArticleNumber = pclControlValve->GetBodyArtNum();
					CString strLocArtNumber = LocArtNumTab.GetLocalArticleNumber( strArticleNumber );
					strControlValve = _T("[") + strArticleNumber;
					
					if( false == strLocArtNumber.IsEmpty() )
					{
						strControlValve += _T("(") + strLocArtNumber + _T(")]");
					}
					else
					{
						strControlValve += _T("]");
					}

					strControlValve += _T(" ") + CString( pclControlValve->GetName() );

					if( false == m_bHMSrcSelectedAsaPackage )
					{
						// Actuator.
						CDB_Actuator *pclActuator = (CDB_Actuator *)( pHMCv->GetActrIDPtr().MP );
						strArticleNumber = pclActuator->GetArtNum();
						strLocArtNumber = LocArtNumTab.GetLocalArticleNumber( strArticleNumber );
						strChangeDone = strControlValve + _T("; [") + strArticleNumber;

						if( false == strLocArtNumber.IsEmpty() )
						{
							strChangeDone += _T("(") + strLocArtNumber + _T(")]");
						}
						else
						{
							strChangeDone += _T("]");
						}

						strChangeDone += _T(" ") + CString( pclActuator->GetName() );
					}
					else
					{
						strArticleNumber = m_pHMCvActrSet->GetReference();
						strLocArtNumber = LocArtNumTab.GetLocalArticleNumber( strArticleNumber );
						strChangeDone = _T("[") + strArticleNumber;
						
						if( false == strLocArtNumber.IsEmpty() )
						{
							strChangeDone += _T("(") + strLocArtNumber + _T(")]");
						}
						else
						{
							strChangeDone += _T("]");
						}

						strChangeDone += _T(" ") + m_pHMCvActrSet->GetName();
						strChangeDone += _T(" (") + TASApp.LoadLocalizedString( IDS_DLGSRACTUATOR_SET ) + _T(")");
					}

					strChangeDone += _T(" -> ") + strControlValve;

					// Clear actuator infos
					pHMCv->RemoveActuator();
					pHMCv->SetActrSelectedAsaPackage( eBool3::eb3False );
					pHMCv->SetActrSelectionAllowed( eBool3::eb3False );

					// Clear also control valve and actuator accessories set if exist.
					pHMCv->ClearCVActrAccSetIDPtrArray();
				}
				break;

			case WorkingMode::WM_Add:
				{
					// Control valve.
					CDB_ControlValve *pclControlValve = pHMCv->GetpCV();
					CString strArticleNumber = pclControlValve->GetBodyArtNum();
					CString strLocArtNumber = LocArtNumTab.GetLocalArticleNumber( strArticleNumber );
					strControlValve = _T("[") + strArticleNumber;

					if( false == strLocArtNumber.IsEmpty() )
					{
						strControlValve += _T("(") + strLocArtNumber + _T(")]");
					}
					else
					{
						strControlValve += _T("]");
					}

					strControlValve += _T(" ") + CString( pclControlValve->GetName() );

					CDB_Actuator *pclActuator = NULL;
					CString strActuator;
					CString strSet;

					if( false == m_bSetForced )
					{
						// Actuator
						pclActuator = m_vecActuatorSolution[m_iCurrentSolution];
						strArticleNumber = pclActuator->GetArtNum();
						strLocArtNumber = LocArtNumTab.GetLocalArticleNumber( strArticleNumber );
						strActuator = _T("[") + strArticleNumber;
						
						if( false == strLocArtNumber.IsEmpty() )
						{
							strActuator += _T("(") + strLocArtNumber + _T(")]");
						}
						else
						{
							strActuator += _T("]");
						}

						strActuator += _T(" ") + CString( pclActuator->GetName() );

						pHMCv->SetActrSelectedAsaPackage( eBool3::eb3False );
					}
					else
					{
						// Prepare set.
						CDB_Set *pclSet = m_vecCVActrSetSolution[m_iCurrentSolution];
						strArticleNumber = pclSet->GetReference();
						strLocArtNumber = LocArtNumTab.GetLocalArticleNumber( strArticleNumber );
						strSet = _T("[") + strArticleNumber;

						if( false == strLocArtNumber.IsEmpty() )
						{
							strSet += _T("(") + strLocArtNumber + _T(")]");
						}
						else
						{
							strSet += _T("]");
						}

						strSet += _T(" ") + pclSet->GetName();
						strSet += _T(" (") + TASApp.LoadLocalizedString( IDS_DLGSRSET_SET ) + _T(")");
						
						pclActuator = (CDB_Actuator *)( pclSet->GetSecondIDPtr().MP );
						pHMCv->SetActrSelectedAsaPackage( eBool3::eb3True );
					}
					
					if( NULL != m_pclAdapter && _NULL_IDPTR == pHMCv->GetActrAdaptIDPtr() )
					{
						pHMCv->SetActrAdaptIDPtr( m_pclAdapter->GetIDPtr() );
					}

					pHMCv->SetActrIDPtr( pclActuator->GetIDPtr() );
					pHMCv->SetActrSelectionAllowed( eBool3::eb3True );
					CDB_ElectroActuator *pclElectroActuator = dynamic_cast<CDB_ElectroActuator *>( pclActuator );

					if( NULL != pclElectroActuator )
					{
						pHMCv->SetActrSelectedVoltageIDPtr( *m_pPowerSupplyIDPtr );
						pHMCv->SetActrSelectedSignalIDPtr( *m_pInputSignalIDPtr );
					}

					if( false == m_bSetForced )
					{
						strChangeDone = strControlValve + _T(" -> ") + strControlValve + _T("; ") + strActuator;
					}
					else
					{
						strChangeDone = strControlValve + _T(" -> ") + strSet;
					}
				}

				break;
		}

		m_mapChangeStatus[m_iCurrentSourceSelected] = true;
		m_iTotalChange++;

		CString strValveNumber = pHM->GetHMName() + _T(" : ");
		strChangeDone.Insert( 0, strValveNumber );
		m_ListChangeDone.AddItemText( strChangeDone );
		m_ListChangeDone.EnsureVisible( m_ListChangeDone.GetItemCount() - 1, FALSE );
	}
}

void CDlgSearchAndReplaceActuator::_ApplyOnIndSel( void )
{
	if( WS_DISABLED != ( m_ButApply.GetStyle() & WS_DISABLED ) )
	{
		CString strControlValve;
		CString strChangeDone;
		CDS_SSelCtrl *pclSSelCtrl = m_pvecSelCtrlList->at( m_iCurrentSourceSelected );

		switch( m_eWorkingMode )
		{
			case WorkingMode::WM_Remove:
				{
					// Control valve.
					CDB_ControlValve *pclControlValve = (CDB_ControlValve *)( pclSSelCtrl->GetProductIDPtr().MP );
					CString strArticleNumber = pclControlValve->GetBodyArtNum();
					CString strLocArtNumber = LocArtNumTab.GetLocalArticleNumber( strArticleNumber );
					strControlValve = _T("[") + strArticleNumber;

					if( false == strLocArtNumber.IsEmpty() )
					{
						strControlValve += _T("(") + strLocArtNumber + _T(")]");
					}
					else
					{
						strControlValve += _T("]");
					}

					strControlValve += _T(" ") + CString( pclControlValve->GetName() );

					if( false == pclSSelCtrl->IsSelectedAsAPackage( ) )
					{
						// Actuator.
						CDB_Actuator *pclActuator = (CDB_Actuator *)( pclSSelCtrl->GetActrIDPtr().MP );
						strArticleNumber = pclActuator->GetArtNum();
						strLocArtNumber = LocArtNumTab.GetLocalArticleNumber( strArticleNumber );
						strChangeDone = strControlValve + _T("; [") + strArticleNumber;

						if( false == strLocArtNumber.IsEmpty() )
						{
							strChangeDone += _T("(") + strLocArtNumber + _T(")]");
						}
						else
						{
							strChangeDone += _T("]");
						}

						strChangeDone += _T(" ") + CString( pclActuator->GetName() );
					}
					else
					{
						CDB_Set *pclSet = (CDB_Set *)( pclSSelCtrl->GetCvActrSetIDPtr().MP );
						strArticleNumber = pclSet->GetReference();
						strLocArtNumber = LocArtNumTab.GetLocalArticleNumber( strArticleNumber );
						strChangeDone = _T("[") + strArticleNumber;

						if( false == strLocArtNumber.IsEmpty() )
						{
							strChangeDone += _T("(") + strLocArtNumber + _T(")]");
						}
						else
						{
							strChangeDone += _T("]");
						}

						strChangeDone += _T(" ") + pclSet->GetName();
						strChangeDone += _T(" (") + TASApp.LoadLocalizedString( IDS_DLGSRACTUATOR_SET ) + _T(")");
					}

					strChangeDone += _T(" -> ") + strControlValve;

					// Clear actuator infos.
					pclSSelCtrl->SetActrIDPtr( _NULL_IDPTR );
					pclSSelCtrl->SetActuatorPowerSupplyID( _T("") );
					pclSSelCtrl->SetActuatorInputSignalID( _T("") );
					pclSSelCtrl->SetFailSafeFunction( 0 );
					pclSSelCtrl->SetDRPFunction( CDB_ControlValve::DRPFunction::drpfUndefined );

					// Clear accessories linked to the actuator.
					CAccessoryList *pclAccessoryList = pclSSelCtrl->GetActuatorAccessoryList();

					if( NULL != pclAccessoryList )
					{
						pclAccessoryList->Clear();
					}

					pclAccessoryList = pclSSelCtrl->GetCvAccessoryList();

					if( NULL != pclAccessoryList && true == pclSSelCtrl->IsSelectedAsAPackage() )
					{
						// Clear also control valve and actuator accessories set if exist.
						pclAccessoryList->Clear( CAccessoryList::AccessoryType::_AT_SetAccessory );
					}

					pclSSelCtrl->ResetCVActrSetIDPtr();
					pclSSelCtrl->SetSelectedAsAPackage( false );
				}

				break;

			case WorkingMode::WM_Add:
				{
					// Control valve.
					CDB_ControlValve *pclControlValve = (CDB_ControlValve *)( pclSSelCtrl->GetProductIDPtr().MP );
					CString strArticleNumber = pclControlValve->GetBodyArtNum();
					CString strLocArtNumber = LocArtNumTab.GetLocalArticleNumber( strArticleNumber );
					strControlValve = _T("[") + strArticleNumber;

					if( false == strLocArtNumber.IsEmpty() )
					{
						strControlValve += _T("(") + strLocArtNumber + _T(")]");
					}
					else
					{
						strControlValve += _T("]");
					}

					strControlValve += _T(" ") + CString( pclControlValve->GetName() );

					CDB_Actuator *pclActuator = NULL;
					CString strActuator;
					CString strSet;

					if( false == m_bSetForced )
					{
						// Actuator.
						pclActuator = m_vecActuatorSolution[m_iCurrentSolution];
						strArticleNumber = pclActuator->GetArtNum();
						strLocArtNumber = LocArtNumTab.GetLocalArticleNumber( strArticleNumber );
						strActuator = _T("[") + strArticleNumber;

						if( false == strLocArtNumber.IsEmpty() )
						{
							strActuator += _T("(") + strLocArtNumber + _T(")]");
						}
						else
						{
							strActuator += _T("]");
						}

						strActuator += _T(" ") + CString( pclActuator->GetName() );

						pclSSelCtrl->SetActrIDPtr( pclActuator->GetIDPtr() );

						if( NULL != m_pclAdapter && 0 == pclSSelCtrl->GetCvAccessoryList()->GetCount( CAccessoryList::AccessoryType::_AT_Adapter ) )
						{
							pclSSelCtrl->GetCvAccessoryList()->Add( m_pclAdapter->GetIDPtr(), CAccessoryList::_AT_Adapter );
						}

						pclSSelCtrl->SetSelectedAsAPackage( false );
					}
					else
					{
						// Prepare set.
						CDB_Set *pclSet = m_vecCVActrSetSolution[m_iCurrentSolution];
						strArticleNumber = pclSet->GetReference();
						strLocArtNumber = LocArtNumTab.GetLocalArticleNumber( strArticleNumber );
						strSet = _T("[") + strArticleNumber;

						if( false == strLocArtNumber.IsEmpty() )
						{
							strSet += _T("(") + strLocArtNumber + _T(")]");
						}
						else
						{
							strSet += _T("]");
						}

						strSet += _T(" ") + pclSet->GetName();
						strSet += _T(" (") + TASApp.LoadLocalizedString( IDS_DLGSRSET_SET ) + _T(")");

						if( NULL != m_pclAdapter && 0 == pclSSelCtrl->GetCvAccessoryList()->GetCount( CAccessoryList::AccessoryType::_AT_Adapter ) )
						{
							pclSSelCtrl->GetCvAccessoryList()->Add( m_pclAdapter->GetIDPtr(), CAccessoryList::AccessoryType::_AT_Adapter );
						}

						pclActuator = (CDB_Actuator *)( pclSet->GetSecondIDPtr().MP );
						pclSSelCtrl->SetActrIDPtr( pclActuator->GetIDPtr() );
						CDB_ElectroActuator *pclElectroActuator = dynamic_cast<CDB_ElectroActuator *>( pclActuator );

						if( NULL != pclElectroActuator )
						{
							pclSSelCtrl->SetActuatorPowerSupplyID( m_pPowerSupplyIDPtr->ID );
							pclSSelCtrl->SetActuatorInputSignalID( m_pInputSignalIDPtr->ID );
							pclSSelCtrl->SetFailSafeFunction( ( false == m_bFailSafeFct ) ? 0 : 1 );
							pclSSelCtrl->SetDRPFunction( m_eDefaultReturnPosFct );
						}

						pclSSelCtrl->SetSelectedAsAPackage( true );

						// Add built-in if exist.
						if( m_vecCVSetAccessoryList.size() > 0 )
						{
							CAccessoryList *pclAccessoryList = pclSSelCtrl->GetCvAccessoryList();

							if( NULL != pclAccessoryList )
							{
								for( int iLoop = 0; iLoop < (int)m_vecCVSetAccessoryList.size(); iLoop++ )
								{
									pclAccessoryList->Add( m_vecCVSetAccessoryList[iLoop]->GetIDPtr(), CAccessoryList::AccessoryType::_AT_SetAccessory );
								}
							}
						}
					}

					if( false == m_bSetForced )
					{
						strChangeDone = strControlValve + _T(" -> ") + strControlValve + _T("; ") + strActuator;
					}
					else
					{
						strChangeDone = strControlValve + _T(" -> ") + strSet;
					}
				}

				break;
		}

		pclSSelCtrl->Modified();
		m_mapChangeStatus[m_iCurrentSourceSelected] = true;
		m_iTotalChange++;

		CString strValveNumber;
		CString strNumber;
		strNumber.Format( _T("%i"), m_iCurrentSourceSelected + 1 );
		FormatString( strValveNumber, TASApp.LoadLocalizedString( IDS_DLGSRACTUATOR_INDSELNUMBER ), strNumber );
		strValveNumber += _T(" : ");
		strChangeDone.Insert( 0, strValveNumber );
		m_ListChangeDone.AddItemText( strChangeDone );
		m_ListChangeDone.EnsureVisible( m_ListChangeDone.GetItemCount() - 1, FALSE );
	}
}

void CDlgSearchAndReplaceActuator::_ApplyAll( void )
{
	int iStartIndex = m_iCurrentSourceSelected;
	bool fStop = false;
	do 
	{
		_FindSolution();
		if( false == m_mapChangeStatus[m_iCurrentSourceSelected] )
		{
			switch( m_eWorkingOn )
			{
				case WorkingOn::WO_HM:
					_ApplyOnHM();
					break;

				case WorkingOn::WO_IndSel:
					_ApplyOnIndSel();
					break;
			}
		}

		// If we are on the last control valve...
		if( false == _IsNextSelectionExist() )
		{
			if( iStartIndex > 0 )
			{
				// Verify if the control valves before 'iStartIndex' are done or not.
				bool fAtLeastOneNotDone = false;
				for( int iLoop = 0; iLoop < iStartIndex && false == fAtLeastOneNotDone; iLoop++ )
				{
					if( false == m_mapChangeStatus[iLoop] )
						fAtLeastOneNotDone = true;
				}
				
				// If there are some that are not yet changed...
				if( true == fAtLeastOneNotDone )
				{
					// We propose to the user to start from the beginning.
					int iReturn = MessageBox( TASApp.LoadLocalizedString( IDS_DLGSRACTUATOR_CONTINUE ), NULL, MB_YESNO | MB_ICONQUESTION );
					if( IDNO == iReturn )
					{
						// User doesn't want to continue, we stop here.
						fStop = true;
					}
					else
					{
						// User wants to continue, we reset to the first position.
						m_iCurrentSourceSelected = 0;
					}
				}
				else
				{
					// No more change available.
					fStop = true;
				}
			}
			else
			{
				// User click 'Apply all' from the beginning. All valves have been checked.
				fStop = true;
			}
		}
		else if( iStartIndex > 0 && m_iCurrentSourceSelected == iStartIndex - 1 )
		{
			fStop = true;
		}
		else
		{
			m_iCurrentSourceSelected++;
		}
	}while( false == fStop );

	// Refresh and show the final message.
	_ShowSolution();
	_ShowFinalMessage();
}

void CDlgSearchAndReplaceActuator::_ShowFinalMessage()
{
	CString str;
	CString strTotal;
	strTotal.Format( _T("%u"), m_iTotalChange );
	FormatString( str, ( WorkingMode::WM_Remove == m_eWorkingMode ) ? IDS_DLGSRACTUATOR_FINISHREMOVEMSG : IDS_DLGSRACTUATOR_FINISHADDMSG, strTotal );
	MessageBox( str, NULL, MB_OK );
}
