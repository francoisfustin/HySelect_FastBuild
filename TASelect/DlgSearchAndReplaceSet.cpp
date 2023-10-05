#include "stdafx.h"
#include "afxdialogex.h"
#include "TASelect.h"
#include "MainFrm.h"
#include "HydroMod.h"
#include "SelProdPageBase.h"
#include "SelProdDockPane.h"
#include "RViewSelProd.h"
#include "DlgSearchReplace.h"
#include "DlgSearchAndReplaceSet.h"

IMPLEMENT_DYNAMIC( CDlgSearchAndReplaceSet, CDialogEx )

CDlgSearchAndReplaceSet::CDlgSearchAndReplaceSet( CWnd* pParent )
	: CDialogEx( CDlgSearchAndReplaceSet::IDD, pParent )
{
	m_pParent = pParent;
	m_eWorkingOn = WorkingOn::WO_HM;
	m_eWorkingMode = WorkingMode::WM_Group;
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
	m_vecCVActrSetSolution.clear();
	m_iCurrentCVActrSet = -1;
	m_pclAdapter = NULL;
	m_vecCVSetAccessoryList.clear();
	m_iTotalChange = 0;
	m_cTextForeColor = 0;
	m_cTextBackColor = 0;
}

CDlgSearchAndReplaceSet::~CDlgSearchAndReplaceSet()
{
	CTable *pTable = (CTable *)( TASApp.GetpTADS()->Get( _T("DLGSRSET_HMCOPY") ).MP );
	if( NULL != pTable )
	{
		TASApp.GetpTADS()->DeleteObjectRecursive( pTable, true );
	}

	pTable = (CTable *)( TASApp.GetpTADS()->Get( _T("DLGSRSET_SELCTRLCOPY") ).MP );
	
	if( NULL != pTable )
	{
		TASApp.GetpTADS()->DeleteObjectRecursive( pTable, true );
	}
}

bool CDlgSearchAndReplaceSet::SetParams( std::vector<LPARAM>* pvecParams )
{
	try
	{
		if( NULL == pvecParams || 2 != pvecParams->size() )
		{
			return false;
		}
	
		m_eWorkingOn = ( CDlgSearchReplace::RadioState::RS_HMCalc == (CDlgSearchReplace::RadioState)( pvecParams->at( 0 ) ) ) ? WorkingOn::WO_HM : WorkingOn::WO_IndSel;

		if( WorkingOn::WO_HM == m_eWorkingOn )
		{
			m_pvecHMList = (std::vector<CDS_HydroMod *>*)( pvecParams->at( 1 ) );
		
			// Verify if copy is not already exist.
			CTable *pTable = (CTable *)( TASApp.GetpTADS()->Get( _T("DLGSRSET_HMCOPY") ).MP );

			if( NULL != pTable )
			{
				pTable->MakeEmpty();
			}
			else
			{
				IDPTR IDPtr;
				TASApp.GetpTADS()->CreateObject( IDPtr, CLASS( CTable ), _T("DLGSRSET_HMCOPY") );
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

				// Initialize map.
				m_mapChangeStatus[i] = false;
			}
		}
		else
		{
			m_pvecSelCtrlList = (std::vector<CDS_SSelCtrl*>*)( pvecParams->at( 1 ) );

			// Verify if copy is not already exist.
			CTable *pTable = (CTable *)( TASApp.GetpTADS()->Get( _T("DLGSRSET_SELCTRLCOPY") ).MP );
		
			if( NULL != pTable )
			{
				pTable->MakeEmpty();
			}
			else
			{
				IDPTR IDPtr;
				TASApp.GetpTADS()->CreateObject( IDPtr, CLASS( CTable ), _T("DLGSRSET_SELCTRLCOPY") );
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
			
				// Initialize map.
				m_mapChangeStatus[i] = false;
			}
		}

		return true;
	}
	catch( CHySelectException &clHySelectException )
	{
		clHySelectException.AddMessage( _T("Error in 'CDlgSearchAndReplaceSet::SetParams'."), __LINE__, __FILE__ );
		throw;
	}
}

BEGIN_MESSAGE_MAP( CDlgSearchAndReplaceSet, CDialogEx )
	ON_WM_MOVE()
	ON_BN_CLICKED( IDC_BUTAPPLY, OnClickedButApply )
	ON_BN_CLICKED( IDC_BUTAPPLYALL, OnClickedButApplyAll )
	ON_BN_CLICKED( IDC_BUTPREV, OnClickedButPrevious )
	ON_BN_CLICKED( IDC_BUTNEXT, OnClickedButNext )
	ON_BN_CLICKED( IDC_BUTCLOSE, OnClickedButClose )
END_MESSAGE_MAP()

void CDlgSearchAndReplaceSet::DoDataExchange( CDataExchange* pDX )
{
	CDialogEx::DoDataExchange( pDX );
	DDX_Control( pDX, IDC_STATICCVSRC, m_StaticCVSrc );
	DDX_Control( pDX, IDC_STATICCVTRG, m_StaticCVTrg );
	DDX_Control( pDX, IDC_STATICNOSOLUTION, m_StaticNoSolution );
	DDX_Control( pDX, IDC_LISTCVSRC, m_ListCVSrc );
	DDX_Control( pDX, IDC_LISTCVTRG, m_ListCVTrg );
	DDX_Control( pDX, IDC_BUTAPPLY, m_ButApply );
	DDX_Control( pDX, IDC_BUTAPPLYALL, m_ButApplyAll );
	DDX_Control( pDX, IDC_BUTPREV, m_ButPrevious );
	DDX_Control( pDX, IDC_BUTNEXT, m_ButNext );
	DDX_Control( pDX, IDC_BUTCLOSE, m_ButClose );
	DDX_Control( pDX, IDC_LISTCHANGEDONE, m_ListChangeDone );
}

BOOL CDlgSearchAndReplaceSet::OnInitDialog()
{
	CDialogEx::OnInitDialog();
	SetWindowText( TASApp.LoadLocalizedString( ( WorkingMode::WM_Group == m_eWorkingMode ) ? IDS_DLGSEARCHREPLACE_SETGROUP : IDS_DLGSEARCHREPLACE_SETSPLIT ) );

	// Set the window position to the last stored position in registry. If window position is not yet stored in the registry, the 
	// window is centered by default in the mainframe rect.
 	CRect apprect, rect;
 	::AfxGetApp()->m_pMainWnd->GetWindowRect( &apprect );
 	GetWindowRect( &rect );

	// Set the window size to the last stored size in registry.
	int x = ::AfxGetApp()->GetProfileInt( _T("DlgSearchAndReplaceSet"), _T("ScreenXPos"), apprect.CenterPoint().x - rect.Width() / 2 );
	int y = ::AfxGetApp()->GetProfileInt( _T("DlgSearchAndReplaceSet"), _T("ScreenYPos"), apprect.CenterPoint().y - rect.Height() / 2 );
	SetWindowPos( NULL, x, y, 0, 0, SWP_NOZORDER | SWP_NOSIZE );

	// Title of the left list.
	m_iListSrcTitleID = ( WorkingOn::WO_HM == m_eWorkingOn ) ? IDS_DLGSRSET_HMTITLE : IDS_DLGSRSET_INDSELTITLE;

	// Title of the right list.
	m_iListTrgTitleID = ( WorkingMode::WM_Group == m_eWorkingMode ) ? IDS_DLGSRSET_CVACTRSETTITLE : IDS_DLGSRSET_CVACTRTITLE;

	// Prepare the left list.
	CString str;
	str.Format( TASApp.LoadLocalizedString( IDS_DLGSRSET_ARTICLENUMBER ), 0 );
	m_ListCVSrc.InsertColumn( 0, str, LVCFMT_LEFT, 93, 0 );
	str.Format( TASApp.LoadLocalizedString( IDS_DLGSRSET_ARTICLEDETAILS ), 1 );
	m_ListCVSrc.InsertColumn( 1, str, LVCFMT_LEFT, 236, 0 );
	m_ListCVSrc.ModifyStyle( 0, LVS_REPORT ); 
	m_ListCVSrc.SetExtendedStyle( m_ListCVSrc.GetExtendedStyle() | LVS_EX_FULLROWSELECT );
	m_ListCVSrc.SetClickListCtrlDisable( true );

	// Prepare the right list.
	str.Format( TASApp.LoadLocalizedString( IDS_DLGSRSET_ARTICLENUMBER ), 0 );
	m_ListCVTrg.InsertColumn( 0, str, LVCFMT_LEFT, 93, 0 );
	str.Format( TASApp.LoadLocalizedString( IDS_DLGSRSET_ARTICLEDETAILS ), 1 );
	m_ListCVTrg.InsertColumn( 1, str, LVCFMT_LEFT, 236, 0 );
	m_ListCVTrg.ModifyStyle( 0, LVS_REPORT ); 
	m_ListCVTrg.SetExtendedStyle( m_ListCVTrg.GetExtendedStyle() | LVS_EX_FULLROWSELECT );
	m_ListCVTrg.SetClickListCtrlDisable( true );

	// Prepare the no breakable text.
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

	m_mapChangeStatus.clear();

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

void CDlgSearchAndReplaceSet::OnCancel()
{
	TASApp.GetpTADS()->RefreshResults( true );

	if( NULL != pRViewSelProd )
	{
		pRViewSelProd->RedrawRightView();
	}

	CDialogEx::OnCancel();
}

void CDlgSearchAndReplaceSet::OnMove( int x, int y )
{
	CDialogEx::OnMove( x, y );

	if( true == m_bInitialized )
	{
		CRect rect;
		
		GetWindowRect( &rect );
		ScreenToClient( &rect );
		
		x += rect.left;
		y += rect.top;
		::AfxGetApp()->WriteProfileInt( _T("DlgSearchAndReplaceSet"), _T("ScreenXPos"), x );
		::AfxGetApp()->WriteProfileInt( _T("DlgSearchAndReplaceSet"), _T("ScreenYPos"), y );
	}
}

void CDlgSearchAndReplaceSet::OnClickedButApply()
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

void CDlgSearchAndReplaceSet::OnClickedButApplyAll()
{
	_ApplyAll();
	m_ButApply.EnableWindow( FALSE );
	m_ButApplyAll.EnableWindow( FALSE );
	m_ButNext.EnableWindow( FALSE );
}

void CDlgSearchAndReplaceSet::OnClickedButPrevious()
{
	m_ListCVSrc.DeleteAllItems();
	m_ListCVTrg.DeleteAllItems();
	m_StaticNoSolution.ShowWindow( SW_HIDE );
	m_ButApply.EnableWindow( TRUE );
	m_ButApplyAll.EnableWindow( TRUE );
	if( true == _GetPreviousSelection( ) )
	{
		_ShowSolution();
	}
}

void CDlgSearchAndReplaceSet::OnClickedButNext()
{
	m_ListCVSrc.DeleteAllItems();
	m_ListCVTrg.DeleteAllItems();
	m_StaticNoSolution.ShowWindow( SW_HIDE );
	m_ButApply.EnableWindow( TRUE );
	m_ButApplyAll.EnableWindow( TRUE );
	if( true == _GetNextSelection( ) )
	{
		_ShowSolution();
	}
}

void CDlgSearchAndReplaceSet::OnClickedButClose()
{
	CDlgSearchAndReplaceSet::OnCancel();
}

void CDlgSearchAndReplaceSet::_FillSrcList()
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
	CDB_Set* pCVActrSet = NULL;
	bool fForSet = false;
	if( WorkingOn::WO_HM == m_eWorkingOn )
	{
		iTotalSource = (int)m_pvecHMList->size();
		pHM = m_vecHMListCopy[m_iCurrentSourceSelected];
		if( NULL == pHM )
			return;

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

		if( WorkingMode::WM_Group == m_eWorkingMode )
		{
			pclActuator = (CDB_Actuator*)( pHM->GetpCV()->GetActrIDPtr().MP );
			if( NULL == pclActuator )
				return;
		}
		else
		{
			CTableSet* pCVActSetTab = pclControlValve->GetTableSet();				ASSERT( NULL != pCVActSetTab );
			if( NULL == pCVActSetTab )
				return;
			pclActuator = (CDB_Actuator*)( pHM->GetpCV()->GetActrIDPtr().MP );
			if( NULL == pclActuator )
				return;
			pCVActrSet = pCVActSetTab->FindCompatibleSet( pclControlValve->GetIDPtr().ID, pclActuator->GetIDPtr().ID );
			m_pHMCvActrSet = pCVActrSet;
			m_bHMSrcSelectedAsaPackage = ( NULL != pCVActrSet ) ? true : false;
			fForSet = m_bHMSrcSelectedAsaPackage;

			if( NULL != pCVActrSet )
			{
				// CV-Actuator accessories set.
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
	else
	{
		iTotalSource = (int)m_pvecSelCtrlList->size();
		pclCurrentSelCtrl = (CDS_SSelCtrl*)m_vecSelCtrlListCopy[m_iCurrentSourceSelected];
		if( NULL == pclCurrentSelCtrl )
			return;

		pclControlValve = (CDB_ControlValve*)( pclCurrentSelCtrl->GetProductIDPtr().MP );
		if( NULL == pclControlValve )
			return;

		if( WorkingMode::WM_Group == m_eWorkingMode )
		{
			pclActuator = (CDB_Actuator*)( pclCurrentSelCtrl->GetActrIDPtr().MP );
		}
		else
		{
			pCVActrSet = dynamic_cast<CDB_Set*>( (CData*)( pclCurrentSelCtrl->GetCvActrSetIDPtr().MP ) );
			if( NULL == pCVActrSet )
				return;

			fForSet = true;
			pclActuator = (CDB_Actuator*)( pCVActrSet->GetSecondIDPtr().MP );
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
	if( WorkingMode::WM_Group == m_eWorkingMode )
	{
		_FillCVArticleReferenceNoSet( pclControlValve, &m_ListCVSrc );
	}
	else
	{
		_FillCVArticleReferenceSet( pCVActrSet, &m_ListCVSrc );
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
	_FillActuatorPart( pclActuator, pclControlValve, fForSet, &m_ListCVSrc );

	// Fill actuator accessories if exist.
	if( NULL != pclActuatorAccessoryList && pclActuatorAccessoryList->GetCount( CAccessoryList::AccessoryType::_AT_Accessory ) > 0 )
	{
		CAccessoryList::AccessoryItem rAccessory = pclActuatorAccessoryList->GetFirst( CAccessoryList::AccessoryType::_AT_Accessory );
		while( _NULL_IDPTR != rAccessory.IDPtr )
		{
			_FillAccessoryPart( (CDB_Product*)( rAccessory.IDPtr.MP ), false, rAccessory.fByPair, &m_ListCVSrc );
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
	
	if( 0 == m_mapChangeStatus.count( m_iCurrentSourceSelected ) )
		m_mapChangeStatus[m_iCurrentSourceSelected] = false;

	if( WorkingOn::WO_HM == m_eWorkingOn )
		m_pParent->SendMessage( WM_USER_REPLACEPOPUPFINDNEXT, (WPARAM)m_pvecHMList->at( m_iCurrentSourceSelected ), NULL );
}

void CDlgSearchAndReplaceSet::_FillDstList()
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
	CAccessoryList rActuatorAccessoryList;
	CAccessoryList* pclActuatorAccessoryList = NULL;
	bool fIsSelectedAsPackage = false;
	CDB_Set* pCVActrSet = NULL;
	switch( m_eWorkingOn )
	{
		case WorkingOn::WO_HM:
			{
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

				if( WorkingMode::WM_Group == m_eWorkingMode )
				{
					pclActuator = (CDB_Actuator*)( pHM->GetpCV()->GetActrIDPtr().MP );
					if( true == m_mapChangeStatus[m_iCurrentSourceSelected] )
					{
						CTableSet* pCVActSetTab = pclControlValve->GetTableSet();				ASSERT( NULL != pCVActSetTab );
						if( NULL == pCVActSetTab )
							return;
						pCVActrSet = pCVActSetTab->FindCompatibleSet( pclControlValve->GetIDPtr().ID, pclActuator->GetIDPtr().ID );
						if( NULL == pCVActrSet )
							return;
					}
					else if( m_iCurrentCVActrSet < (int)m_vecCVActrSetSolution.size() )
					{
						pCVActrSet = m_vecCVActrSetSolution[m_iCurrentCVActrSet];
					}
				}
				else
				{
					pclActuator = (CDB_Actuator*)( pHM->GetpCV()->GetActrIDPtr().MP );
				}
			}
			break;

		case WorkingOn::WO_IndSel:
			{
				pclCurrentSelCtrl = (CDS_SSelCtrl*)m_pvecSelCtrlList->at( m_iCurrentSourceSelected );
				if( NULL == pclCurrentSelCtrl )
					return;

				fIsSelectedAsPackage = pclCurrentSelCtrl->IsSelectedAsAPackage();
				pclCVAccessoryList = pclCurrentSelCtrl->GetCvAccessoryList();
				pclActuatorAccessoryList = pclCurrentSelCtrl->GetActuatorAccessoryList();

				if( WorkingMode::WM_Group == m_eWorkingMode )
				{
					if( true == m_mapChangeStatus[m_iCurrentSourceSelected] )
					{
						pCVActrSet = (CDB_Set*)( pclCurrentSelCtrl->GetCvActrSetIDPtr().MP );
					}
					else if( m_iCurrentCVActrSet < (int)m_vecCVActrSetSolution.size() )
					{
						pCVActrSet = m_vecCVActrSetSolution[m_iCurrentCVActrSet];
					}
					if( NULL == pCVActrSet )
						return;
			
					pclControlValve = (CDB_ControlValve*)( pCVActrSet->GetFirstIDPtr().MP );
					pclActuator = (CDB_Actuator*)( pCVActrSet->GetSecondIDPtr().MP );
				}
				else
				{
					pclControlValve = (CDB_ControlValve*)( pclCurrentSelCtrl->GetProductIDPtr().MP );
					pclActuator =  (CDB_Actuator*)( pclCurrentSelCtrl->GetActrIDPtr().MP );
				}
			}
			break;
	}

	switch( m_eWorkingMode )
	{
		case WorkingMode::WM_Group:
			{
				// User try to group separated control valve and actuator to a set.

				if( true == m_mapChangeStatus[m_iCurrentSourceSelected] )
				{
					m_StaticCVTrg.SetWindowText( TASApp.LoadLocalizedString( IDS_DLGSRSET_ALREADYGROUPED ) );
				}
				else
				{
					m_StaticCVTrg.SetWindowText( TASApp.LoadLocalizedString( m_iListTrgTitleID ) );
				}

				if( NULL != pCVActrSet )
				{
					_FillCVArticleReferenceSet( pCVActrSet, &m_ListCVTrg );
					_FillCVCommonPart( pclControlValve, &m_ListCVTrg );
					
					// Fill control valve accessories if exist and allowed to be selected with the set.
					if( NULL != pclCVAccessoryList && pclCVAccessoryList->GetCount( CAccessoryList::AccessoryType::_AT_Accessory ) > 0 )
					{
						CAccessoryList::AccessoryItem rAccessory = pclCVAccessoryList->GetFirst( CAccessoryList::AccessoryType::_AT_Accessory );
						while( _NULL_IDPTR != rAccessory.IDPtr )
						{
							CDB_Product* pclAccessory = (CDB_Product*)( rAccessory.IDPtr.MP );
							if( false == pclAccessory->IsAttached() )
								_FillAccessoryPart( (CDB_Product*)( rAccessory.IDPtr.MP ), false, rAccessory.fByPair, &m_ListCVTrg );
							rAccessory = pclCVAccessoryList->GetNext( CAccessoryList::AccessoryType::_AT_Accessory );
						}
					}
					
					_FillAdapterPart( m_pclAdapter, true, &m_ListCVTrg );
					_FillActuatorPart( pclActuator, pclControlValve, true, &m_ListCVTrg );

					// Fill actuator accessories if exist and allowed to be selected with the set.
					if( NULL != pclActuatorAccessoryList && pclActuatorAccessoryList->GetCount( CAccessoryList::AccessoryType::_AT_Accessory ) > 0 )
					{
						CAccessoryList::AccessoryItem rAccessory = pclActuatorAccessoryList->GetFirst( CAccessoryList::AccessoryType::_AT_Accessory );
						while( _NULL_IDPTR != rAccessory.IDPtr )
						{
							CDB_Product* pclAccessory = (CDB_Product*)( rAccessory.IDPtr.MP );
							if( false == pclAccessory->IsAttached() )
								_FillAccessoryPart( (CDB_Product*)( rAccessory.IDPtr.MP ), rAccessory.fByPair, true, &m_ListCVTrg );
							rAccessory = pclActuatorAccessoryList->GetNext( CAccessoryList::AccessoryType::_AT_Accessory );
						}
					}

					// Check if there is some built-in accessory in the set.
					_FillCVActrAccSetPart( pCVActrSet, &m_ListCVTrg );
				}
			}
			break;

		case WorkingMode::WM_Split:
			{
				// User try to split control valve and actuator set to separated component.

				if( true == m_mapChangeStatus[m_iCurrentSourceSelected] )
				{
					m_StaticCVTrg.SetWindowText( TASApp.LoadLocalizedString( IDS_DLGSRSET_ALREADYSPLITTED ) );
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

				// Fill actuator part.
				_FillActuatorPart( pclActuator, pclControlValve, false, &m_ListCVTrg );

				// Fill actuator accessories if exist.
				if( NULL != pclActuatorAccessoryList && pclActuatorAccessoryList->GetCount( CAccessoryList::AccessoryType::_AT_Accessory ) > 0 )
				{
					CAccessoryList::AccessoryItem rAccessory = pclActuatorAccessoryList->GetFirst( CAccessoryList::AccessoryType::_AT_Accessory );
					while( _NULL_IDPTR != rAccessory.IDPtr )
					{
						_FillAccessoryPart( (CDB_Product*)( rAccessory.IDPtr.MP ), false, rAccessory.fByPair, &m_ListCVTrg );
						rAccessory = pclActuatorAccessoryList->GetNext( CAccessoryList::AccessoryType::_AT_Accessory );
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
		m_ButApply.EnableWindow( FALSE );
		m_ButApplyAll.EnableWindow( FALSE );
	}
}

void CDlgSearchAndReplaceSet::_FillDstListNotAvailableOrDeleted()
{
	m_StaticCVTrg.SetWindowText( _T("") );
	
	m_StaticNoSolution.ShowWindow( SW_SHOW );
	m_StaticNoSolution.SetTextColor( _RED );
	m_StaticNoSolution.SetWindowText( TASApp.LoadLocalizedString( IDS_DLGSRSET_NOTAVAILORDELETED ) );
	
	m_ButApply.EnableWindow( FALSE );
	m_ButApplyAll.EnableWindow( FALSE );
}

void CDlgSearchAndReplaceSet::_FillDstListSetNoBreakable()
{
	m_StaticCVTrg.SetWindowText( _T("") );

	m_StaticNoSolution.ShowWindow( SW_SHOW );
	m_StaticNoSolution.SetTextColor( _BLACK );
	m_StaticNoSolution.SetWindowText( TASApp.LoadLocalizedString( IDS_DLGSRSET_NOBREAKABLE ) );

	m_ButApply.EnableWindow( FALSE );
	m_ButApplyAll.EnableWindow( FALSE );
}

void CDlgSearchAndReplaceSet::_FillChangeDoneList()
{
	CDS_HydroMod *pHM = NULL;
	CDB_ControlValve *pclControlValve = NULL;
	CDB_Actuator *pclActuator = NULL;
	CDB_Set *pclSet = NULL;

	switch( m_eWorkingOn )
	{
		case WorkingOn::WO_HM:
			{
				pHM = m_vecHMListCopy[m_iCurrentSourceSelected];

				if( NULL == pHM )
				{
					return;
				}

				pclControlValve = pHM->GetpCV()->GetpCV();

				if( NULL == pclControlValve )
				{
					return;
				}

				pclActuator = (CDB_Actuator *)( pHM->GetpCV()->GetActrIDPtr().MP );

				if( WorkingMode::WM_Group == m_eWorkingMode )
				{
					pclSet = m_vecCVActrSetSolution[m_iCurrentCVActrSet];
				}
				else
				{
					CTableSet *pCVActSetTab = pclControlValve->GetTableSet();
					ASSERT( NULL != pCVActSetTab );

					if(	NULL == pCVActSetTab )
					{
						return;
					}

					pclSet = pCVActSetTab->FindCompatibleSet( pclControlValve->GetIDPtr().ID, pclActuator->GetIDPtr().ID );
				}
			}
			break;

		case WorkingOn::WO_IndSel:
			{
				CDS_SSelCtrl *pclCurrentSelCtrl = (CDS_SSelCtrl *)m_pvecSelCtrlList->at( m_iCurrentSourceSelected );

				if( NULL == pclCurrentSelCtrl )
				{
					return;
				}

				pclControlValve = (CDB_ControlValve *)( pclCurrentSelCtrl->GetProductIDPtr().MP );

				if( NULL == pclControlValve )
				{
					return;
				}

				pclActuator = (CDB_Actuator *)( pclCurrentSelCtrl->GetActrIDPtr().MP );

				if( WorkingMode::WM_Group == m_eWorkingMode )
				{
					pclSet = m_vecCVActrSetSolution[m_iCurrentCVActrSet];
				}
				else
				{
					pclSet = (CDB_Set*)( pclCurrentSelCtrl->GetCvActrSetIDPtr().MP );
				}
			}
			break;
	}

	
	// Prepare separated components.

	// Valve.
	CString strSeparated;
	CString strArticleNumber = pclControlValve->GetBodyArtNum();
	CString strLocArtNumber = LocArtNumTab.GetLocalArticleNumber( strArticleNumber );

	strSeparated = _T("[") + strArticleNumber;

	if( false == strLocArtNumber.IsEmpty() )
	{
		strSeparated += _T("(") + strLocArtNumber + _T(")]");
	}
	else
	{
		strSeparated += _T("]");
	}

	strSeparated += _T(" ") + CString( pclControlValve->GetName() );

	// Actuator
	strArticleNumber = pclActuator->GetArtNum();
	strLocArtNumber = LocArtNumTab.GetLocalArticleNumber( strArticleNumber );
	strSeparated += _T("; [") + strArticleNumber;

	if( false == strLocArtNumber.IsEmpty() )
	{
		strSeparated += _T("(") + strLocArtNumber + _T(")]");
	}
	else
	{
		strSeparated += _T("]");
	}

	strSeparated += _T(" ") + CString( pclActuator->GetName() );

	// Prepare set.
	CString strSet;
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

	CString strChangeDone;
	
	if( WorkingMode::WM_Group == m_eWorkingMode )
	{
		strChangeDone = strSeparated + _T(" -> ") + strSet;
	}
	else
	{
		strChangeDone = strSet + _T(" -> ") + strSeparated;
	}

	CString strValveNumber;
	
	if( WorkingOn::WO_HM == m_eWorkingOn )
	{
		strValveNumber = pHM->GetHMName();
	}
	else
	{
		CString strNumber;
		strNumber.Format( _T("%i"), m_iCurrentSourceSelected + 1 );
		FormatString( strValveNumber, TASApp.LoadLocalizedString( IDS_DLGSRSET_INDSELNUMBER ), strNumber );
	}

	strValveNumber += _T(" : ");
	strChangeDone.Insert( 0, strValveNumber );
	m_ListChangeDone.AddItemText( strChangeDone );
	m_ListChangeDone.EnsureVisible( m_ListChangeDone.GetItemCount() - 1, FALSE );
}

void CDlgSearchAndReplaceSet::_FillCVArticleReferenceNoSet( CDB_ControlValve* pclControlValve, CExtListCtrl* pList )
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
		m_cTextBackColor = GetSysColor( COLOR_3DFACE );
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
		pList->SetItemTextBold( m_iNbrRow + m_iLeftPos, DLGSRSET_LEFTCOLUMNINDEX, ( false == fOnePrint ) ? true : false );
		pList->AddItemTextColorVerify( str2, m_iNbrRow + m_iLeftPos++, DLGSRSET_LEFTCOLUMNINDEX, m_cTextForeColor, m_cTextForeColor, m_cTextBackColor );
		fOnePrint = true;
	}while( iPos >= 0 );

	// If something has been printed in left column and product is not available or deleted...
	if( true == fOnePrint && true == fCVNotAvailOrDeleted )
	{
		// Remarks: if product is flagged as 'not available' and 'deleted', we show only 'deleted'!
		CString strFlag = TASApp.LoadLocalizedString( ( true == pclControlValve->IsDeleted() ) ? IDS_TAPDELETED : IDS_TAPNOTAVAILABLE );
		pList->SetItemTextBold( m_iNbrRow + m_iLeftPos, DLGSRSET_LEFTCOLUMNINDEX, true );
		pList->AddItemTextColorVerify( strFlag, m_iNbrRow + m_iLeftPos++, DLGSRSET_LEFTCOLUMNINDEX, m_cTextForeColor, m_cTextForeColor, m_cTextBackColor );
	}
}

void CDlgSearchAndReplaceSet::_FillCVArticleReferenceSet( CDB_Set* pCVActSet, CExtListCtrl* pList )
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
			pList->SetItemTextBold( m_iNbrRow + m_iLeftPos, DLGSRSET_LEFTCOLUMNINDEX, true );
			pList->AddItemTextColorVerify( str, m_iNbrRow + m_iLeftPos++, DLGSRSET_LEFTCOLUMNINDEX, m_cTextForeColor, m_cTextForeColor, m_cTextBackColor );
			fArticleNumberShown = true;
		}
	}

	// Set local article number if allowed and exist.
	if( true == TASApp.IsLocalArtNumberUsed() && false == strLocalArticleNumber.IsEmpty() )
	{
		strLocalArticleNumber = (CString)TASApp.GetLocalArtNumberName() + LocArtNumTab.GetLocalArticleNumber( pCVActSet->GetReference() );
		strLocalArticleNumber.Trim();
		pList->SetItemTextBold( m_iNbrRow + m_iLeftPos, DLGSRSET_LEFTCOLUMNINDEX, ( true == fArticleNumberShown ) ? false : true );
		pList->AddItemTextColorVerify( strLocalArticleNumber, m_iNbrRow + m_iLeftPos++, DLGSRSET_LEFTCOLUMNINDEX, m_cTextForeColor, m_cTextForeColor, m_cTextBackColor );
	}

	if( true == m_bSetNotAvailableOrDeleted )
	{
		// Remarks: if product is flagged as 'not available' and 'deleted', we show only 'deleted'!
		CString strFlag = TASApp.LoadLocalizedString( ( true == pCVActSet->IsDeleted() ) ? IDS_TAPDELETED : IDS_TAPNOTAVAILABLE );
		pList->AddItemTextColorVerify( strFlag, m_iNbrRow + m_iLeftPos++, DLGSRSET_LEFTCOLUMNINDEX, m_cTextForeColor, m_cTextForeColor, m_cTextBackColor );
	}

	// Set the name.
	CString str = pCVActSet->GetName();
	pList->AddItemTextColorVerify( str, m_iNbrRow + m_iRightPos++, DLGSRSET_RIGHTCOLUMNINDEX, m_cTextForeColor, m_cTextForeColor, m_cTextBackColor );
	pList->AddItemTextColorVerify( TASApp.LoadLocalizedString( IDS_DLGSRSET_SET ), m_iNbrRow + m_iRightPos++, DLGSRSET_RIGHTCOLUMNINDEX, m_cTextForeColor, m_cTextForeColor, m_cTextBackColor );

	DLGSRSET_AJUSTCOORDS( m_iLeftPos, m_iRightPos );
	pList->InsertItem( LVIF_TEXT | LVIF_STATE, m_iNbrRow + m_iLeftPos, _T(""), 0, LVIS_SELECTED, 0, 0 );
	m_iLeftPos = ++m_iRightPos;

	pList->AddItemTextColorVerify( _T("-"), m_iNbrRow + m_iLeftPos++, DLGSRSET_LEFTCOLUMNINDEX, m_cTextForeColor, m_cTextForeColor, m_cTextBackColor );
}

void CDlgSearchAndReplaceSet::_FillCVCommonPart( CDB_ControlValve* pclControlValve, CExtListCtrl* pList )
{
	CString str = pclControlValve->GetName();
	pList->AddItemTextColorVerify( str, m_iNbrRow + m_iRightPos++, DLGSRSET_RIGHTCOLUMNINDEX, m_cTextForeColor, m_cTextForeColor, m_cTextBackColor );

	// Set the Kvs.
	CUnitDatabase *pUnitDB = CDimValue::AccessUDB();
	if( !pUnitDB->GetDefaultUnitIndex( _C_KVCVCOEFF ) )
	{
		str = TASApp.LoadLocalizedString( IDS_DLGSRSET_PRODUCT_KVS );
		str += CString( _T(" : ") ) + WriteCUDouble( _C_KVCVCOEFF, pclControlValve->GetKvs() );
	}
	else
	{
		str = TASApp.LoadLocalizedString( IDS_DLGSRSET_PRODUCT_CV );
		str += CString( _T(" : ") ) + WriteCUDouble( _C_KVCVCOEFF, pclControlValve->GetKvs() );
	}
	pList->AddItemTextColorVerify( str, m_iNbrRow + m_iRightPos++, DLGSRSET_RIGHTCOLUMNINDEX, m_cTextForeColor, m_cTextForeColor, m_cTextBackColor );
		
	// Set the rangeability.
	double dRangeability = pclControlValve->GetRangeability();
	if( dRangeability > 0.0 )
	{
		str = TASApp.LoadLocalizedString( IDS_DLGSRSET_RANGEABILITY );
		str += CString( _T(" : ") ) +  pclControlValve->GetStrRangeability().c_str();
		pList->AddItemTextColorVerify( str, m_iNbrRow + m_iRightPos++, DLGSRSET_RIGHTCOLUMNINDEX, m_cTextForeColor, m_cTextForeColor, m_cTextBackColor );
	}

	// Set the leakage rate.
	if( pclControlValve->GetLeakageRate() > 0 )
	{
		str = TASApp.LoadLocalizedString( IDS_DLGSRSET_CV_LEAKAGE );
		str += CString( _T(" : ") ) + WriteCUDouble( _C_KVCVCOEFF, pclControlValve->GetLeakageRate() * 100 );
		str += CString( _T(" ") ) + TASApp.LoadLocalizedString( IDS_DLGSRSET_CV_LEAKAGEUNIT );
		pList->AddItemTextColorVerify( str, m_iNbrRow + m_iRightPos++, DLGSRSET_RIGHTCOLUMNINDEX, m_cTextForeColor, m_cTextForeColor, m_cTextBackColor );
	}

	str = TASApp.LoadLocalizedString( IDS_DLGSRSET_PRODUCT_BDYMAT );
	str += CString( _T(" : ") ) + pclControlValve->GetBodyMaterial();
	pList->AddItemTextColorVerify( str, m_iNbrRow + m_iRightPos++, DLGSRSET_RIGHTCOLUMNINDEX, m_cTextForeColor, m_cTextForeColor, m_cTextBackColor );
	
	str = TASApp.LoadLocalizedString( IDS_DLGSRSET_PRODUCT_CONNECT );
	str += CString( _T(" : ") ) + pclControlValve->GetConnect();
	if( 0 == IDcmp( pclControlValve->GetConnTabID(), _T("COMPONENT_TAB") ) )
		str += CString( _T(" -> ") ) + pclControlValve->GetSize();
	pList->AddItemTextColorVerify( str, m_iNbrRow + m_iRightPos++, DLGSRSET_RIGHTCOLUMNINDEX, m_cTextForeColor, m_cTextForeColor, m_cTextBackColor );
	
	str = TASApp.LoadLocalizedString( IDS_DLGSRSET_PRODUCT_VERSION );
	str += CString( _T(" : ") ) + pclControlValve->GetVersion();
	pList->AddItemTextColorVerify( str, m_iNbrRow + m_iRightPos++, DLGSRSET_RIGHTCOLUMNINDEX, m_cTextForeColor, m_cTextForeColor, m_cTextBackColor );
	
	str = TASApp.LoadLocalizedString( IDS_DLGSRSET_PRODUCT_PN );
	str += CString( _T(" : ") ) + pclControlValve->GetPN().c_str();
	pList->AddItemTextColorVerify( str, m_iNbrRow + m_iRightPos++, DLGSRSET_RIGHTCOLUMNINDEX, m_cTextForeColor, m_cTextForeColor, m_cTextBackColor );

	// Insert a final blank line.
	DLGSRSET_AJUSTCOORDS( m_iLeftPos, m_iRightPos );
	pList->InsertItem( LVIF_TEXT | LVIF_STATE, m_iNbrRow + m_iLeftPos++, _T(""), 0, LVIS_SELECTED, 0, 0 );
	DLGSRSET_AJUSTCOORDS( m_iLeftPos, m_iRightPos );
}

void CDlgSearchAndReplaceSet::_FillAccessoryPart( CDB_Product* pclAccessory, bool fForSet, bool fByPair, CExtListCtrl* pList )
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
		pList->SetItemTextBold( m_iNbrRow + m_iLeftPos, DLGSRSET_LEFTCOLUMNINDEX, true );
		pList->AddItemTextColorVerify( _T("-"), m_iNbrRow + m_iLeftPos++, DLGSRSET_LEFTCOLUMNINDEX, m_cTextForeColor, m_cTextForeColor, m_cTextBackColor );
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
				pList->SetItemTextBold( m_iNbrRow + m_iLeftPos, DLGSRSET_LEFTCOLUMNINDEX, true );
				pList->AddItemTextColorVerify( str, m_iNbrRow + m_iLeftPos++, DLGSRSET_LEFTCOLUMNINDEX, m_cTextForeColor, m_cTextForeColor, m_cTextBackColor );
				fArticleNumberShown = true;
			}
		}

		// Set local article number if allowed and exist.
		if( true == TASApp.IsLocalArtNumberUsed() && false == strLocalArticleNumber.IsEmpty() )
		{
			strLocalArticleNumber = (CString)TASApp.GetLocalArtNumberName() + LocArtNumTab.GetLocalArticleNumber( pclAccessory->GetArtNum() );
			strLocalArticleNumber.Trim();
			pList->SetItemTextBold( m_iNbrRow + m_iLeftPos, DLGSRSET_LEFTCOLUMNINDEX, ( true == fArticleNumberShown ) ? false : true );
			pList->AddItemTextColorVerify( strLocalArticleNumber, m_iNbrRow + m_iLeftPos++, DLGSRSET_LEFTCOLUMNINDEX, m_cTextForeColor, m_cTextForeColor, m_cTextBackColor );
		}
	}

	// If something has been printed in left column and product is not available or deleted...
	if( true == fNotAvailOrDeleted && false == fForSet )
	{
		// Remarks: if product is flagged as 'not available' and 'deleted', we show only 'deleted'!
		CString strFlag = TASApp.LoadLocalizedString( ( true == pclAccessory->IsDeleted() ) ? IDS_TAPDELETED : IDS_TAPNOTAVAILABLE );
		pList->SetItemTextBold( m_iNbrRow + m_iLeftPos, DLGSRSET_LEFTCOLUMNINDEX, true );
		pList->AddItemTextColorVerify( strFlag, m_iNbrRow + m_iLeftPos++, DLGSRSET_LEFTCOLUMNINDEX, m_cTextForeColor, m_cTextForeColor, m_cTextBackColor );
	}
	
	str = ( true == fByPair ) ? _T("2x ") : _T("");
	str += pclAccessory->GetName();
	pList->SetItemTextBold( m_iNbrRow + m_iRightPos, DLGSRSET_RIGHTCOLUMNINDEX, false );
	pList->AddItemTextColorVerify( str, m_iNbrRow + m_iRightPos++, DLGSRSET_RIGHTCOLUMNINDEX, m_cTextForeColor, m_cTextForeColor, m_cTextBackColor );

	str = pclAccessory->GetComment();
	pList->AddItemTextColorVerify( str, m_iNbrRow + m_iRightPos++, DLGSRSET_RIGHTCOLUMNINDEX, m_cTextForeColor, m_cTextForeColor, m_cTextBackColor );

	// Insert a final blank line.
	DLGSRSET_AJUSTCOORDS( m_iLeftPos, m_iRightPos );
	pList->InsertItem( LVIF_TEXT | LVIF_STATE, m_iNbrRow + m_iLeftPos, _T(""), 0, LVIS_SELECTED, 0, 0 );
}

void CDlgSearchAndReplaceSet::_FillActuatorPart( CDB_Actuator* pclActuator, CDB_ControlValve* pclControlValve, bool fForSet, CExtListCtrl* pList )
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
		pList->SetItemTextBold( m_iNbrRow + m_iLeftPos, DLGSRSET_LEFTCOLUMNINDEX, true );
		pList->AddItemTextColorVerify( _T("-"), m_iNbrRow + m_iLeftPos++, DLGSRSET_LEFTCOLUMNINDEX, m_cTextForeColor, m_cTextForeColor, m_cTextBackColor );
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
				pList->SetItemTextBold( m_iNbrRow + m_iLeftPos, DLGSRSET_LEFTCOLUMNINDEX, true );
				pList->AddItemTextColorVerify( str, m_iNbrRow + m_iLeftPos++, DLGSRSET_LEFTCOLUMNINDEX, m_cTextForeColor, m_cTextForeColor, m_cTextBackColor );
				fArticleNumberShown = true;
			}
		}

		// Set local article number if allowed and exist.
		if( true == TASApp.IsLocalArtNumberUsed() && false == strLocalArticleNumber.IsEmpty() )
		{
			strLocalArticleNumber = (CString)TASApp.GetLocalArtNumberName() + LocArtNumTab.GetLocalArticleNumber( pclActuator->GetArtNum() );
			strLocalArticleNumber.Trim();
			pList->SetItemTextBold( m_iNbrRow + m_iLeftPos, DLGSRSET_LEFTCOLUMNINDEX, ( true == fArticleNumberShown ) ? false : true );
			pList->AddItemTextColorVerify( strLocalArticleNumber, m_iNbrRow + m_iLeftPos++, DLGSRSET_LEFTCOLUMNINDEX, m_cTextForeColor, m_cTextForeColor, m_cTextBackColor );
		}
	}

	// If something has been printed in left column and product is not available or deleted...
	if( true == fNotAvailOrDeleted && false == fForSet )
	{
		// Remarks: if product is flagged as 'not available' and 'deleted', we show only 'deleted'!
		CString strFlag = TASApp.LoadLocalizedString( ( true == pclActuator->IsDeleted() ) ? IDS_TAPDELETED : IDS_TAPNOTAVAILABLE );
		pList->SetItemTextBold( m_iNbrRow + m_iLeftPos, DLGSRSET_LEFTCOLUMNINDEX, true );
		pList->AddItemTextColorVerify( strFlag, m_iNbrRow + m_iLeftPos++, DLGSRSET_LEFTCOLUMNINDEX, m_cTextForeColor, m_cTextForeColor, m_cTextBackColor );
	}
	
	////////////////////////////////////////////////////////////////////////////////////////////////////////////////
	// Fill the right part.
	////////////////////////////////////////////////////////////////////////////////////////////////////////////////
	
	// Insert name.
	str = pclActuator->GetName();
	pList->SetItemTextBold( m_iNbrRow + m_iRightPos, DLGSRSET_RIGHTCOLUMNINDEX, false );
	pList->AddItemTextColorVerify( str, m_iNbrRow + m_iRightPos++, DLGSRSET_RIGHTCOLUMNINDEX, m_cTextForeColor, m_cTextForeColor, m_cTextBackColor );
	
	CString strDescription = pclActuator->GetComment();

	if( false == strDescription.IsEmpty() )
	{
		pList->AddItemTextColorVerify( pclActuator->GetComment(), m_iNbrRow + m_iRightPos++, DLGSRSET_RIGHTCOLUMNINDEX, m_cTextForeColor, m_cTextForeColor, m_cTextBackColor );
	}

	// Set the Close off value (Dp or inlet pressure).
	CDB_CloseOffChar *pCloseOffChar = (CDB_CloseOffChar *)( pclControlValve->GetCloseOffCharIDPtr().MP );

	if( NULL != pCloseOffChar )
	{
		if( CDB_CloseOffChar::CloseOffDp == pCloseOffChar->GetLimitType() )
		{
			str = TASApp.LoadLocalizedString( IDS_DLGSRSET_ACT_CLOSEOFFDP );
			double dCloseOffDp = pCloseOffChar->GetCloseOffDp( pclActuator->GetMaxForceTorque() );

			if( dCloseOffDp > 0.0 )
			{
				str += CString( _T(" : ") ) + WriteCUDouble( _U_DIFFPRESS, dCloseOffDp, true );
			}
		}
		else if( CDB_CloseOffChar::InletPressure == pCloseOffChar->GetLimitType() )
		{
			str = TASApp.LoadLocalizedString( IDS_DLGSRSET_ACT_MAXINLETPRESSURE );
			double dMaxInletPressure = pCloseOffChar->GetMaxInletPressure( pclActuator->GetMaxForceTorque() );

			if( dMaxInletPressure > 0.0 )
			{
				str += CString( _T(" : ") ) + WriteCUDouble( _U_PRESSURE, dMaxInletPressure, true );
			}
		}

		pList->AddItemTextColorVerify( str, m_iNbrRow + m_iRightPos++, DLGSRSET_RIGHTCOLUMNINDEX, m_cTextForeColor, m_cTextForeColor, m_cTextBackColor );
	}

	CDB_ElectroActuator *pclElectroActuator = dynamic_cast<CDB_ElectroActuator*>( pclActuator );

	if( NULL != pclElectroActuator )
	{
		// Compute actuating time in sec.
		str = TASApp.LoadLocalizedString( IDS_DLGSRSET_ACTUATING_TIME ) + CString( _T(" : ") ) + pclElectroActuator->GetActuatingTimesStr( pclControlValve->GetStroke(), true );
		pList->AddItemTextColorVerify( str, m_iNbrRow + m_iRightPos++, DLGSRSET_RIGHTCOLUMNINDEX, m_cTextForeColor, m_cTextForeColor, m_cTextBackColor );

		str = TASApp.LoadLocalizedString( IDS_DLGSRSET_ACT_IPXXAUTO );
		str += CString( _T(" : ") ) + pclElectroActuator->GetIPxxAuto();

		if( false == pclElectroActuator->GetIPxxManual().IsEmpty() )
		{
			str += _T("(") + pclElectroActuator->GetIPxxManual() + _T(")");
		}

		pList->AddItemTextColorVerify( str, m_iNbrRow + m_iRightPos++, DLGSRSET_RIGHTCOLUMNINDEX, m_cTextForeColor, m_cTextForeColor, m_cTextBackColor );

		str = TASApp.LoadLocalizedString( IDS_DLGSRSET_ACT_POWSUP );
		str += CString( _T(" : ") ) + pclElectroActuator->GetPowerSupplyStr();
		pList->AddItemTextColorVerify( str, m_iNbrRow + m_iRightPos++, DLGSRSET_RIGHTCOLUMNINDEX, m_cTextForeColor, m_cTextForeColor, m_cTextBackColor );

		str = TASApp.LoadLocalizedString( IDS_DLGSRSET_ACT_INPUTSIG );
		str += CString( _T(" : ")) + pclElectroActuator->GetInOutSignalsStr( true );
		pList->AddItemTextColorVerify( str, m_iNbrRow + m_iRightPos++, DLGSRSET_RIGHTCOLUMNINDEX, m_cTextForeColor, m_cTextForeColor, m_cTextBackColor );

		str = TASApp.LoadLocalizedString( IDS_DLGSRSET_ACT_OUTPUTSIG );

		if( CString( _T("") ) == pclElectroActuator->GetInOutSignalsStr( false ) )
		{
			str += CString( _T(" :    -") );
		}
		else
		{
			str += CString( _T(" : ") ) + pclElectroActuator->GetInOutSignalsStr( false );
		}

		pList->AddItemTextColorVerify( str, m_iNbrRow + m_iRightPos++, DLGSRSET_RIGHTCOLUMNINDEX, m_cTextForeColor, m_cTextForeColor, m_cTextBackColor );

		str = TASApp.LoadLocalizedString( IDS_DLGSRSET_ACT_FAILSAFE );
		
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
		
		pList->AddItemTextColorVerify( str, m_iNbrRow + m_iRightPos++, DLGSRSET_RIGHTCOLUMNINDEX, m_cTextForeColor, m_cTextForeColor, m_cTextBackColor );

		if( pclElectroActuator->GetFailSafe() >= CDB_ElectroActuator::FailSafeType::eFSTypeElectronic 
			&& pclElectroActuator->GetFailSafe() < CDB_ElectroActuator::FailSafeType::eFSTypeLast)
		{
			str = TASApp.LoadLocalizedString( IDS_DLGSRACTUATOR_ACT_FSP ) + _T( " " );
		}
		else
		{
			str = TASApp.LoadLocalizedString( IDS_DLGSRACTUATOR_ACT_DRP ) + _T( " " );
		}
		
		if( CString( _T("") ) == pclElectroActuator->GetDefaultReturnPosStr( pclElectroActuator->GetDefaultReturnPos() ).c_str() )
		{
			str += CString( _T("   -") );
		}
		else
		{
			str += pclElectroActuator->GetDefaultReturnPosStr( pclElectroActuator->GetDefaultReturnPos() ).c_str();
		}

		pList->AddItemTextColorVerify( str, m_iNbrRow + m_iRightPos++, DLGSRSET_RIGHTCOLUMNINDEX, m_cTextForeColor, m_cTextForeColor, m_cTextBackColor );
	}
	
	CDB_ThermostaticActuator *pclThermostaticActuator = dynamic_cast<CDB_ThermostaticActuator*>( pclActuator );

	if( NULL != pclThermostaticActuator )
	{
		// HYS-951: Min setting can be 0.
		if( pclThermostaticActuator->GetMinSetting() >= 0 && pclThermostaticActuator->GetMaxSetting() > 0 )
		{
			CString str1 = TASApp.LoadLocalizedString( IDS_DLGSRSET_ACT_SETTINGRANGE );
			CString strMin = WriteCUDouble( _U_TEMPERATURE, pclThermostaticActuator->GetMinSetting(), false );
			CString strMax =  WriteCUDouble( _U_TEMPERATURE, pclThermostaticActuator->GetMaxSetting(), true );
			str.Format( _T("%s: [%s-%s]"), str1, strMin, strMax );
			pList->AddItemTextColorVerify( str, m_iNbrRow + m_iRightPos++, DLGSRSET_RIGHTCOLUMNINDEX, m_cTextForeColor, m_cTextForeColor, m_cTextBackColor );
		}
		
		if( pclThermostaticActuator->GetCapillaryLength() > 0 )
		{
			str = TASApp.LoadLocalizedString( IDS_DLGSRSET_ACT_CAPILLENGTH );
			str += CString( _T(": ") ) + WriteCUDouble( _U_LENGTH, pclThermostaticActuator->GetCapillaryLength(), true );
			pList->AddItemTextColorVerify( str, m_iNbrRow + m_iRightPos++, DLGSRSET_RIGHTCOLUMNINDEX, m_cTextForeColor, m_cTextForeColor, m_cTextBackColor );
		}
		
		if( pclThermostaticActuator->GetHysteresis() > 0 )
		{
			str = TASApp.LoadLocalizedString( IDS_DLGSRSET_ACT_HYSTERESIS );
			str += CString( _T(": ") ) + WriteCUDouble( _U_DIFFTEMP, pclThermostaticActuator->GetHysteresis(), true );
			pList->AddItemTextColorVerify( str, m_iNbrRow + m_iRightPos++, DLGSRSET_RIGHTCOLUMNINDEX, m_cTextForeColor, m_cTextForeColor, m_cTextBackColor );
		}

		if( pclThermostaticActuator->GetFrostProtection() > 0 )
		{
			str = TASApp.LoadLocalizedString( IDS_DLGSRSET_ACT_FROSTPROTECT );
			str += CString( _T(": ") ) + WriteCUDouble( _U_TEMPERATURE, pclThermostaticActuator->GetFrostProtection(), true );
			pList->AddItemTextColorVerify( str, m_iNbrRow + m_iRightPos++, DLGSRSET_RIGHTCOLUMNINDEX, m_cTextForeColor, m_cTextForeColor, m_cTextBackColor );
		}
	}

	// Insert a final blank line.
	DLGSRSET_AJUSTCOORDS( m_iLeftPos, m_iRightPos );
	pList->InsertItem( LVIF_TEXT | LVIF_STATE, m_iNbrRow + m_iLeftPos, _T(""), 0, LVIS_SELECTED, 0, 0 );
}

void CDlgSearchAndReplaceSet::_FillAdapterPart( CDB_Product* pclAdapter, bool fForSet, CExtListCtrl* pList )
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
		pList->SetItemTextBold( m_iNbrRow + m_iLeftPos, DLGSRSET_LEFTCOLUMNINDEX, true );
		pList->AddItemTextColorVerify( _T("-"), m_iNbrRow + m_iLeftPos++, DLGSRSET_LEFTCOLUMNINDEX, m_cTextForeColor, m_cTextForeColor, m_cTextBackColor );
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
				pList->SetItemTextBold( m_iNbrRow + m_iLeftPos, DLGSRSET_LEFTCOLUMNINDEX, true );
				pList->AddItemTextColorVerify( str, m_iNbrRow + m_iLeftPos++, DLGSRSET_LEFTCOLUMNINDEX, m_cTextForeColor, m_cTextForeColor, m_cTextBackColor );
				fArticleNumberShown = true;
			}
		}

		// Set local article number if allowed and exist.
		if( true == TASApp.IsLocalArtNumberUsed() && false == strLocalArticleNumber.IsEmpty() )
		{
			strLocalArticleNumber = (CString)TASApp.GetLocalArtNumberName() + LocArtNumTab.GetLocalArticleNumber( pclAdapter->GetArtNum() );
			strLocalArticleNumber.Trim();
			pList->SetItemTextBold( m_iNbrRow + m_iLeftPos, DLGSRSET_LEFTCOLUMNINDEX, ( true == fArticleNumberShown ) ? false : true );
			pList->AddItemTextColorVerify( strLocalArticleNumber, m_iNbrRow + m_iLeftPos++, DLGSRSET_LEFTCOLUMNINDEX );
		}
	}

	// If something has been printed in left column and product is not available or deleted...
	if( true == fNotAvailOrDeleted && false == fForSet )
	{
		// Remarks: if product is flagged as 'not available' and 'deleted', we show only 'deleted'!
		CString strFlag = TASApp.LoadLocalizedString( ( true == pclAdapter->IsDeleted() ) ? IDS_TAPDELETED : IDS_TAPNOTAVAILABLE );
		pList->SetItemTextBold( m_iNbrRow + m_iLeftPos, DLGSRSET_LEFTCOLUMNINDEX, true );
		pList->AddItemTextColorVerify( strFlag, m_iNbrRow + m_iLeftPos++, DLGSRSET_LEFTCOLUMNINDEX, m_cTextForeColor, m_cTextForeColor, m_cTextBackColor );
	}

	str = pclAdapter->GetName();
	pList->SetItemTextBold( m_iNbrRow + m_iRightPos, DLGSRSET_RIGHTCOLUMNINDEX, false );
	pList->AddItemTextColorVerify( str, m_iNbrRow + m_iRightPos++, DLGSRSET_RIGHTCOLUMNINDEX, m_cTextForeColor, m_cTextForeColor, m_cTextBackColor );

	str = pclAdapter->GetComment();
	pList->AddItemTextColorVerify( str, m_iNbrRow + m_iRightPos++, DLGSRSET_RIGHTCOLUMNINDEX, m_cTextForeColor, m_cTextForeColor, m_cTextBackColor );

	// Insert a blank line.
	DLGSRSET_AJUSTCOORDS( m_iLeftPos, m_iRightPos );
	pList->InsertItem( LVIF_TEXT | LVIF_STATE, m_iNbrRow + m_iLeftPos++, _T(""), 0, LVIS_SELECTED, 0, 0 );
}

void CDlgSearchAndReplaceSet::_FillCVActrAccSetPart( CDB_Set* pCVActSet, CExtListCtrl* pList )
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

		// Insert a final blank line.
		DLGSRSET_AJUSTCOORDS( m_iLeftPos, m_iRightPos );
		pList->InsertItem( LVIF_TEXT | LVIF_STATE, m_iNbrRow + m_iLeftPos++, _T(""), 0, LVIS_SELECTED, 0, 0 );
		DLGSRSET_AJUSTCOORDS( m_iLeftPos, m_iRightPos );

		for( int i = 0; i < iCount; i++ )
		{
			bool fByPair = pclRuledTable->IsByPair( vecData[i]->GetIDPtr().ID );
			_FillAccessoryPart( (CDB_Product*)vecData[i], true, fByPair, pList );
			m_vecCVSetAccessoryList.push_back( (CDB_Product*)vecData[i] );
		}
	}
}

void CDlgSearchAndReplaceSet::_FillQuantity( int iQuantity, CExtListCtrl* pList )
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
	CString str = TASApp.LoadLocalizedString( IDS_DLGSRSET_QUANTITY );
	pList->SetItemTextBold( m_iNbrRow + m_iLeftPos, DLGSRSET_LEFTCOLUMNINDEX, true );
	pList->AddItemTextColorVerify( str, m_iNbrRow + m_iLeftPos++, DLGSRSET_LEFTCOLUMNINDEX, m_cTextForeColor, m_cTextForeColor, m_cTextBackColor );

	// Fill the right part.
	str.Format( _T("%i"), iQuantity );
	pList->AddItemTextColorVerify( str, m_iNbrRow + m_iRightPos++, DLGSRSET_RIGHTCOLUMNINDEX, m_cTextForeColor, m_cTextForeColor, m_cTextBackColor );

	// Insert a final blank line.
	DLGSRSET_AJUSTCOORDS( m_iLeftPos, m_iRightPos );
	pList->InsertItem( LVIF_TEXT | LVIF_STATE, m_iNbrRow + m_iLeftPos, _T(""), 0, LVIS_SELECTED, 0, 0 );
}

bool CDlgSearchAndReplaceSet::_GetFirstSelection( void )
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

bool CDlgSearchAndReplaceSet::_GetPreviousSelection( void )
{
	bool fReturn = false;
	if( m_iCurrentSourceSelected > 0 )
	{
		m_iCurrentSourceSelected--;
		fReturn = true;
	}
	return fReturn;
}

bool CDlgSearchAndReplaceSet::_IsPreviousSelectionExist( void )
{
	return ( m_iCurrentSourceSelected > 0 );
}

bool CDlgSearchAndReplaceSet::_GetNextSelection( void )
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

bool CDlgSearchAndReplaceSet::_IsNextSelectionExist( void )
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

void CDlgSearchAndReplaceSet::_ShowSolution( void )
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
	else if( WorkingMode::WM_Split == m_eWorkingMode && true == fIsSelectedAsPackage && 
			 ePartOfaSet::ePartOfaSetYes == pclControlValve->GetPartOfaSet() )
	{
		// We can't remove actuator on a set that is not breakable!
		_FillDstListSetNoBreakable();
	}
	else if( true == _FindSolution() )
	{
		if( WorkingMode::WM_Group == m_eWorkingMode )
			_GetFirstCVActrSetSolution();
		_FillDstList();
	}
}

bool CDlgSearchAndReplaceSet::_GetFirstCVActrSetSolution()
{
	bool fReturn = false;
	m_iCurrentCVActrSet = -1;
	if( m_vecCVActrSetSolution.size() > 0 )
	{
		m_iCurrentCVActrSet = 0;
		_GetAdapter();
		fReturn = true;
	}
	return fReturn;
}

bool CDlgSearchAndReplaceSet::_GetNextCVActrSetSolution()
{
	bool fReturn = false;
	m_iCurrentCVActrSet++;
	if( m_iCurrentCVActrSet < (int)m_vecCVActrSetSolution.size() )
	{
		_GetAdapter();
		fReturn = true;
	}
	return fReturn;
}

bool CDlgSearchAndReplaceSet::_IsNextCVActrSetSolutionExist( void )
{
	return ( m_iCurrentCVActrSet + 1 < (int)m_vecCVActrSetSolution.size() );
}

void CDlgSearchAndReplaceSet::_GetCVActrSetSolution( int iIndex )
{
	if( iIndex < 0 || iIndex > (int)m_vecCVActrSetSolution.size() - 1 )
		return;

	m_iCurrentCVActrSet = iIndex;
	_GetAdapter();
}

int CDlgSearchAndReplaceSet::_GetNumberOfSolution( void )
{
	return (int)m_vecCVActrSetSolution.size();
}

void CDlgSearchAndReplaceSet::_GetAdapter( void )
{
	// Try to find an adapter for the set.
	m_pclAdapter = NULL;
	if( WorkingMode::WM_Group == m_eWorkingMode && m_vecCVActrSetSolution.size() > 0 )
	{
		if( m_iCurrentCVActrSet < 0 || m_iCurrentCVActrSet >= (int)m_vecCVActrSetSolution.size() )
			return;

		CDB_Set* pclSet = m_vecCVActrSetSolution[m_iCurrentCVActrSet];
		if( NULL == pclSet )
			return;

		CDB_ControlValve* pclControlValve = dynamic_cast<CDB_ControlValve*>( pclSet->GetFirstIDPtr().MP );
		if( NULL == pclControlValve )
			return;

		CDB_Actuator* pclActuator = dynamic_cast<CDB_Actuator*>( pclSet->GetSecondIDPtr().MP );
		if( NULL == pclActuator )
			return;

		CRank clAdapterList;
		CDB_RuledTableBase* pclRuledTable = NULL;
		CDB_ControlProperties* pclControlProperties = pclControlValve->GetCtrlProp();
		CDB_ControlProperties::CvCtrlType CtrlType = CDB_ControlProperties::eCvNU;
		if( NULL != pclControlProperties )
			CtrlType = pclControlProperties->GetCtrlType();

		bool fDowngradeFct = true;
		CTADatabase::FilterSelection eFilterSelection = ( WorkingOn::WO_HM == m_eWorkingOn ) ? CTADatabase::FilterSelection::ForHMCalc : CTADatabase::FilterSelection::ForIndAndBatchSel;
		TASApp.GetpTADB()->GetAdapterList( pclControlValve, &clAdapterList, &pclRuledTable, true, _T(""), _T(""), -1,
											CDB_ControlValve::DRPFunction::drpfAll, CtrlType, true, eFilterSelection );

		if( 1 == clAdapterList.GetCount() )
		{
			CString str;
			LPARAM lparam;
			clAdapterList.GetFirst( str, lparam );
			m_pclAdapter = (CDB_Product *)lparam;
		}
	}
}

bool CDlgSearchAndReplaceSet::_FindSolution()
{
	if( true == m_mapChangeStatus[m_iCurrentSourceSelected] )
		return true;

	// Prepare variables.
	CDB_ControlValve* pclControlValve = NULL;
	CDB_Actuator* pclActuator = NULL;
	if( WorkingMode::WM_Group == m_eWorkingMode )
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

					CTableSet* pCVActSetTab = pclControlValve->GetTableSet();				ASSERT( NULL != pCVActSetTab );
					if( NULL == pCVActSetTab )
						return false;
					
					pclActuator = (CDB_Actuator*)( pHM->GetpCV()->GetActrIDPtr().MP );
					if( NULL == pclActuator )
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

					pclActuator = (CDB_Actuator*)( pclSelCtrl->GetActrIDPtr().MP );
					if( NULL == pclActuator )
						return false;
				}
				break;
		}
	}

	m_vecCVActrSetSolution.clear();
	bool fReturn = false;
	switch( m_eWorkingMode )
	{
		case WorkingMode::WM_Group:
			{
				if( NULL == pclControlValve || NULL == pclControlValve->GetTableSet() )
					break;

				CTableSet* pCVActTableSet = pclControlValve->GetTableSet();
				std::set<CDB_Set*> setCVActrSet;
				int iCDBSetCount = pCVActTableSet->FindCompatibleSet( &setCVActrSet, pclControlValve->GetIDPtr().ID, pclActuator->GetIDPtr().ID );
				if( iCDBSetCount > 0 )
				{
					// Move to a vector to facilitate code.
					for( std::set<CDB_Set*>::iterator iter = setCVActrSet.begin(); iter != setCVActrSet.end(); iter++ )
						m_vecCVActrSetSolution.push_back( *iter );
					fReturn = true;
				}
			}
			break;

		case WorkingMode::WM_Split:
			fReturn = true;
			break;
	}
	return fReturn;
}

void CDlgSearchAndReplaceSet::_ApplyOnHM( void )
{
	if( WS_DISABLED != ( m_ButApply.GetStyle() & WS_DISABLED ) )
	{
		CDS_HydroMod* pHM = m_pvecHMList->at( m_iCurrentSourceSelected );
		if( NULL == pHM || NULL == pHM->GetpCV() )
			return;

		CDS_HydroMod::CCv* pHMCv = pHM->GetpCV();

		// Call before doing the job.
		_FillChangeDoneList();
	
		switch( m_eWorkingMode )
		{
			case WorkingMode::WM_Group:
				{
					CDB_Set* pclSet = m_vecCVActrSetSolution[m_iCurrentCVActrSet];
					if( NULL == pclSet )
						return;

					CDB_ControlValve* pclControlValve = (CDB_ControlValve*)( pclSet->GetFirstIDPtr().MP );
					if( NULL == pclControlValve )
						return;

					CDB_Actuator* pclActuator = (CDB_Actuator*)( pclSet->GetSecondIDPtr().MP );
					if( NULL == pclActuator )
						return;
					
					CDB_RuledTable* pclRuledTable = dynamic_cast<CDB_RuledTable*>( pclControlValve->GetAccessoriesGroupIDPtr().MP );
					if( NULL == pclRuledTable )
						return;

					pHMCv->SetActrIDPtr( pclActuator->GetIDPtr() );
					pHMCv->SetActrSelectedAsaPackage( eBool3::eb3True );
					pHMCv->SetActrSelectionAllowed( eBool3::eb3True );

					// Review all accessories and remove those that can't be sold with the set.
					if( pHMCv->GetCVAccCount() > 0 )
					{
						CArray<IDPTR> rCVAcc;
						CArray<IDPTR>* pCVAcc = pHMCv->GetpCVAccIDPtrArray();
						for( int iLoopCVAcc = 0; iLoopCVAcc < pHMCv->GetCVAccCount(); iLoopCVAcc++ )
						{
							CDB_Product* pclAccessory = (CDB_Product*)( pHMCv->GetCVAccIDPtr( iLoopCVAcc ).MP );
							if( NULL != pclAccessory && true == pclAccessory->IsAnAccessory() && false == pclAccessory->IsAttached() )
							{
								rCVAcc.Add( pclAccessory->GetIDPtr() );
							}
						}
						pCVAcc->RemoveAll();
						pCVAcc->Copy( rCVAcc );
					}

					if( pHMCv->GetActrAccCount() > 0 )
					{
						CArray<IDPTR> rActrAcc;
						CArray<IDPTR>* pActrAcc = pHMCv->GetpActrAccIDPtrArray();
						for( int iLoopActrAcc = 0; iLoopActrAcc < pHMCv->GetActrAccCount(); iLoopActrAcc++ )
						{
							CDB_Product* pclAccessory = (CDB_Product*)( pHMCv->GetActrAccIDPtr( iLoopActrAcc ).MP );
							if( NULL != pclAccessory && true == pclAccessory->IsAnAccessory() && false == pclAccessory->IsAttached() )
							{
								rActrAcc.Add( pclAccessory->GetIDPtr() );
							}
						}
						pActrAcc->RemoveAll();
						pActrAcc->Copy( rActrAcc );
					}
				}
				break;

			case WorkingMode::WM_Split:
				{
					pHMCv->SetActrSelectedAsaPackage( eBool3::eb3False );
					pHMCv->SetActrSelectionAllowed( eBool3::eb3True );

					// Remove CV-actuator accessories set if exist.
					pHMCv->ClearCVActrAccSetIDPtrArray();
				}
				break;
		}

		m_mapChangeStatus[m_iCurrentSourceSelected] = true;
		m_iTotalChange++;
	}
}

void CDlgSearchAndReplaceSet::_ApplyOnIndSel( void )
{
	if( WS_DISABLED != ( m_ButApply.GetStyle() & WS_DISABLED ) )
	{
		CDS_SSelCtrl* pclSSelCtrl = m_pvecSelCtrlList->at( m_iCurrentSourceSelected );

		// Call before doing the job.
		_FillChangeDoneList();
	
		switch( m_eWorkingMode )
		{
			case WorkingMode::WM_Group:
				{
					pclSSelCtrl->SetSelectedAsAPackage( true );
				
					// Review all accessories and remove those that can't be sold with the set.
					// Remark: adapters have their 'Attached' property set to 'true' but we keep it.
					for( int iLoop = 0; iLoop < 2; iLoop++ )
					{
						CAccessoryList* pclAccessoryList = ( 0 == iLoop ) ? pclSSelCtrl->GetCvAccessoryList() : pclSSelCtrl->GetActuatorAccessoryList();
						if( NULL != pclAccessoryList )
						{
							CAccessoryList clAccessoryList; 
							CAccessoryList::AccessoryItem rAccessory = pclAccessoryList->GetFirst();
							while( _NULL_IDPTR != rAccessory.IDPtr )
							{
								CDB_Product* pclAccessory = dynamic_cast<CDB_Product*>( (CData*)rAccessory.IDPtr.MP );
								if( NULL != pclAccessory && true == pclAccessory->IsAnAccessory() && ( CAccessoryList::AccessoryType::_AT_Adapter == rAccessory.eAccessoryType || false == pclAccessory->IsAttached() ) )
									clAccessoryList.Add( rAccessory.eAccessoryType, &rAccessory );

								rAccessory = pclAccessoryList->GetNext();
							}
							pclAccessoryList->Clear();
							clAccessoryList.CopyTo( pclAccessoryList );
						}
					}

					// Add built-in if exist.
					if( m_vecCVSetAccessoryList.size() > 0 )
					{
						CAccessoryList* pclAccessoryList = pclSSelCtrl->GetCvAccessoryList();
						if( NULL != pclAccessoryList )
						{
							for( int iLoop = 0; iLoop < (int)m_vecCVSetAccessoryList.size(); iLoop++ )
							{
								pclAccessoryList->Add( m_vecCVSetAccessoryList[iLoop]->GetIDPtr(), CAccessoryList::AccessoryType::_AT_SetAccessory );
							}
						}
					}
				}
				break;

			case WorkingMode::WM_Split:
				{
					pclSSelCtrl->SetSelectedAsAPackage( false );
					pclSSelCtrl->ResetCVActrSetIDPtr();

					// Remove CV-actuator accessories if exist.
					CAccessoryList* pclAccessoryList = pclSSelCtrl->GetCvAccessoryList();
					if( NULL != pclAccessoryList )
					{
						CAccessoryList clAccessoryList;
						CAccessoryList::AccessoryItem rAccessoryItem = pclAccessoryList->GetFirst();
						while( _NULL_IDPTR != rAccessoryItem.IDPtr )
						{
							if( CAccessoryList::AccessoryType::_AT_SetAccessory != rAccessoryItem.eAccessoryType )
								clAccessoryList.Add( rAccessoryItem.eAccessoryType, &rAccessoryItem );
							rAccessoryItem = pclAccessoryList->GetNext();
						}
						pclAccessoryList->Clear();
						clAccessoryList.CopyTo( pclAccessoryList );
					}
				}
				break;
		}

		pclSSelCtrl->Modified();
		m_mapChangeStatus[m_iCurrentSourceSelected] = true;
		m_iTotalChange++;
	}
}

void CDlgSearchAndReplaceSet::_ApplyAll( void )
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
					int iReturn = MessageBox( TASApp.LoadLocalizedString( IDS_DLGSRSET_CONTINUE ), NULL, MB_YESNO | MB_ICONQUESTION );
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

void CDlgSearchAndReplaceSet::_ShowFinalMessage()
{
	CString str;
	CString strTotal;
	strTotal.Format( _T("%u"), m_iTotalChange );
	FormatString( str, ( WorkingMode::WM_Split == m_eWorkingMode ) ? IDS_DLGSRSET_FINISHSPLITMSG : IDS_DLGSRSET_FINISHGROUPMSG, strTotal );
	MessageBox( str, NULL, MB_OK );
}
